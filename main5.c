

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

#include "libavutil/avstring.h"
#include "libavutil/channel_layout.h"
#include "libavutil/eval.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "libavutil/fifo.h"
#include "libavutil/parseutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/bprint.h"
#include "libavformat/avformat.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libavcodec/avfft.h"
#include "libswresample/swresample.h"

#include "temp.h"

#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

/* Step size for volume control in dB */
#define SDL_VOLUME_STEP (0.75)

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
/* TODO: We assume that a decoded and resampled frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 65536)

#define CURSOR_HIDE_DELAY 1000000

#define USE_ONEPASS_SUBTITLE_RENDER 1

static unsigned sws_flags = SWS_BICUBIC;

typedef struct MyAVPacketList {
    AVPacket *pkt;
    int serial;
} MyAVPacketList;

typedef struct PacketQueue {
    AVFifoBuffer *pkt_list;
    int nb_packets;
    int size;
    int64_t duration;
    int abort_request;
    int serial;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

typedef struct AudioParams {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
} AudioParams;

typedef struct Clock {
    double pts;           /* clock base */
    double pts_drift;     /* clock base minus time at which we updated the clock */
    double last_updated;
    double speed;
    int serial;           /* clock is based on a packet with this serial */
    int paused;
    int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;

typedef struct Frame {
    AVFrame *frame;
    AVSubtitle sub;
    int serial;
    double pts;           /* presentation timestamp for the frame */
    double duration;      /* estimated duration of the frame */
    int64_t pos;          /* byte position of the frame in the input file */
    int width;
    int height;
    int format;
    AVRational sar;
    int uploaded;
    int flip_v;
} Frame;

typedef struct FrameQueue {
    Frame queue[FRAME_QUEUE_SIZE];
    int rindex;
    int windex;
    int size;
    int max_size;
    int keep_last;
    int rindex_shown;
    SDL_mutex *mutex;
    SDL_cond *cond;
    PacketQueue *pktq;
} FrameQueue;

enum {
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

typedef struct Decoder {
    AVPacket *pkt;
    PacketQueue *queue;
    AVCodecContext *avctx;
    int pkt_serial;
    int finished;
    int packet_pending;
    SDL_cond *empty_queue_cond;
    int64_t start_pts;
    AVRational start_pts_tb;
    int64_t next_pts;
    AVRational next_pts_tb;
    SDL_Thread *decoder_tid;
} Decoder;


typedef struct AudioState {
    SDL_Thread *read_tid;
    const AVInputFormat *iformat;
    int abort_request;
    int force_refresh;
    int paused;
    int last_paused;
    int queue_attachments_req;
    int seek_req;
    int seek_flags;
    int64_t seek_pos;
    int64_t seek_rel;
    int read_pause_return;
    AVFormatContext *ic;
    int realtime;

    Clock audclk;
    Clock vidclk;
    Clock extclk;

    FrameQueue pictq;
    FrameQueue subpq;
    FrameQueue sampq;

    Decoder auddec;
    Decoder viddec;
    Decoder subdec;

    int audio_stream;

    int av_sync_type;

    double audio_clock;
    int audio_clock_serial;
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;
    PacketQueue audioq;
    int audio_hw_buf_size;
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    unsigned int audio_buf_size; /* in bytes */
    unsigned int audio_buf1_size;
    int audio_buf_index; /* in bytes */
    int audio_write_buf_size;
    int audio_volume;
    int muted;
    struct AudioParams audio_src;

    struct AudioParams audio_tgt;
    struct SwrContext *swr_ctx;
    int frame_drops_early;
    int frame_drops_late;

    enum ShowMode {
        SHOW_MODE_NONE = -1, SHOW_MODE_VIDEO = 0, SHOW_MODE_WAVES, SHOW_MODE_RDFT, SHOW_MODE_NB
    } show_mode;
    int16_t sample_array[SAMPLE_ARRAY_SIZE];
    int sample_array_index;
    int last_i_start;
    RDFTContext *rdft;
    int rdft_bits;
    FFTSample *rdft_data;
    int xpos;
    double last_vis_time;
    SDL_Texture *vis_texture;
    SDL_Texture *sub_texture;
    SDL_Texture *vid_texture;

    int subtitle_stream;
    AVStream *subtitle_st;
    PacketQueue subtitleq;

    double frame_timer;
    double frame_last_returned_time;
    double frame_last_filter_delay;
    int video_stream;
    AVStream *video_st;
    PacketQueue videoq;
    double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    struct SwsContext *img_convert_ctx;
    struct SwsContext *sub_convert_ctx;
    int eof;

    char *filename;
    int width, height, xleft, ytop;
    int step;
    int last_video_stream, last_audio_stream, last_subtitle_stream;

    SDL_cond *continue_read_thread;
} AudioState;

SDL_AudioDeviceID audio_dev;
const char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
int infinite_buffer = -1;
int64_t audio_callback_time;

int packet_queue_init(PacketQueue *q){
    memset(q, 0, sizeof(PacketQueue));
    q->pkt_list = av_fifo_alloc(sizeof(MyAVPacketList));
    if(!q->pkt_list)
        return AVERROR(ENOMEM);
    q->mutex = SDL_CreateMutex();
    if (!q->mutex) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    q->cond = SDL_CreateCond();
    if (!q->cond) {
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    q->abort_request = 1;
    return 0;
}

void packet_queue_abort(PacketQueue *q){
    SDL_LockMutex(q->mutex);
    q->abort_request = 1;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
}

void packet_queue_flush(PacketQueue *q){
    MyAVPacketList pkt1;

    SDL_LockMutex(q->mutex);
    while (av_fifo_size(q->pkt_list) >= sizeof(pkt1)) {
        av_fifo_generic_read(q->pkt_list, &pkt1, sizeof(pkt1), NULL);
        av_packet_free(&pkt1.pkt);
    }
    q->nb_packets = 0;
    q->size = 0;
    q->duration = 0;
    q->serial++;
    SDL_UnlockMutex(q->mutex);
}

void packet_queue_destroy(PacketQueue *q){
    packet_queue_flush(q);
    av_fifo_freep(&q->pkt_list);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
}

void packet_queue_start(PacketQueue *q){
    SDL_LockMutex(q->mutex);
    q->abort_request = 0;
    q->serial++;
    SDL_UnlockMutex(q->mutex);
}

int packet_queue_put_private(PacketQueue *q, AVPacket *pkt)
{
    MyAVPacketList pkt1;

    if (q->abort_request)
        return -1;

    if (av_fifo_space(q->pkt_list) < sizeof(pkt1)) {
        if (av_fifo_grow(q->pkt_list, sizeof(pkt1)) < 0)
            return -1;
    }

    pkt1.pkt = pkt;
    pkt1.serial = q->serial;

    av_fifo_generic_write(q->pkt_list, &pkt1, sizeof(pkt1), NULL);
    q->nb_packets++;
    q->size += pkt1.pkt->size + sizeof(pkt1);
    q->duration += pkt1.pkt->duration;
    SDL_CondSignal(q->cond);
    return 0;
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt)
{
    AVPacket *pkt1;
    int ret;

    pkt1 = av_packet_alloc();
    if (!pkt1) {
        av_packet_unref(pkt);
        return -1;
    }
    av_packet_move_ref(pkt1, pkt);

    SDL_LockMutex(q->mutex);
    ret = packet_queue_put_private(q, pkt1);
    SDL_UnlockMutex(q->mutex);

    if (ret < 0)
        av_packet_free(&pkt1);

    return ret;
}

int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block, int *serial){
    MyAVPacketList pkt1;
    int ret;

    SDL_LockMutex(q->mutex);

    for (;;) {
        if (q->abort_request) {
            ret = -1;
            break;
        }

        if (av_fifo_size(q->pkt_list) >= sizeof(pkt1)) {
            av_fifo_generic_read(q->pkt_list, &pkt1, sizeof(pkt1), NULL);
            q->nb_packets--;
            q->size -= pkt1.pkt->size + sizeof(pkt1);
            q->duration -= pkt1.pkt->duration;
            av_packet_move_ref(pkt, pkt1.pkt);
            if (serial)
                *serial = pkt1.serial;
            av_packet_free(&pkt1.pkt);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}

int packet_queue_put_nullpacket(PacketQueue *q, AVPacket *pkt, int stream_index)
{
    pkt->stream_index = stream_index;
    return packet_queue_put(q, pkt);
}

void frame_queue_unref_item(Frame *vp){
    av_frame_unref(vp->frame);
}

int frame_queue_init(FrameQueue *f, PacketQueue *pktq, int max_size, int keep_last){
    int i;
    memset(f, 0, sizeof(FrameQueue));
    if(!(f->mutex = SDL_CreateMutex())){
        fprintf(stderr, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    if (!(f->cond = SDL_CreateCond())) {
        fprintf(stderr, "SDL_CreateCond(): %s\n", SDL_GetError());
        av_log(NULL, AV_LOG_FATAL, "SDL_CreateCond(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    f->pktq = pktq;
    f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
    f->keep_last = !!keep_last;
    for(i = 0; i < f->max_size; i++){
        if(!(f->queue[i].frame = av_frame_alloc()))
            return AVERROR(ENOMEM);
    }
    return 0;
}

void frame_queue_signal(FrameQueue *f){
    SDL_LockMutex(f->mutex);
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

void frame_queue_destroy(FrameQueue *f){
    for(int i = 0; i < f->max_size; i++){
        Frame* vp = &f->queue[i];
        frame_queue_unref_item(vp);
        av_frame_free(&vp->frame);
    }
    SDL_DestroyMutex(f->mutex);
    SDL_DestroyCond(f->cond);
}

Frame *frame_queue_peek_readable(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    while (f->size - f->rindex_shown <= 0 &&
           !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request)
        return NULL;

    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

void frame_queue_next(FrameQueue *f)
{
    if (f->keep_last && !f->rindex_shown) {
        f->rindex_shown = 1;
        return;
    }
    frame_queue_unref_item(&f->queue[f->rindex]);
    if (++f->rindex == f->max_size)
        f->rindex = 0;
    SDL_LockMutex(f->mutex);
    f->size--;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

Frame* frame_queue_peek_writable(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    while (f->size >= f->max_size &&
           !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request)
        return NULL;

    return &f->queue[f->windex];
}

void frame_queue_push(FrameQueue *f){
    if (++f->windex == f->max_size)
        f->windex = 0;
    SDL_LockMutex(f->mutex);
    f->size++;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

int decode_interrupt_cb(void* ctx){
    AudioState *is = ctx;
    return is->abort_request;
}

void decoder_abort(Decoder *d, FrameQueue *fq){
    packet_queue_abort(d->queue);
    frame_queue_signal(fq);
    SDL_WaitThread(d->decoder_tid, NULL);
    d->decoder_tid = NULL;
    packet_queue_flush(d->queue);
}

void decoder_destroy(Decoder *d){
    av_packet_free(&d->pkt);
    avcodec_free_context(&d->avctx);
}

int decoder_init(Decoder *d, AVCodecContext* avctx, PacketQueue* queue, SDL_cond* empty_queue_cond){
    memset(d, 0, sizeof(Decoder));
    d->pkt = av_packet_alloc();
    if (!d->pkt)
        return AVERROR(ENOMEM);
    d->avctx = avctx;
    d->queue = queue;
    d->empty_queue_cond = empty_queue_cond;
    d->start_pts = AV_NOPTS_VALUE;
    d->pkt_serial = -1;
    return 0;
}

int decoder_start(Decoder *d, int (*fn)(void *), const char *thread_name, void* arg){
    packet_queue_start(d->queue);
    d->decoder_tid = SDL_CreateThread(fn, thread_name, arg);
    if (!d->decoder_tid) {
        fprintf(stderr,"SDL_CreateThread(): %s\n", SDL_GetError());
        return AVERROR(ENOMEM);
    }
    return 0;
}

int decoder_decode_frame(Decoder *d, AVFrame *frame, AVSubtitle *sub){
    int ret = AVERROR(EAGAIN);

    for (;;) {
        if (d->queue->serial == d->pkt_serial) {
            do {
                if (d->queue->abort_request)
                    return -1;
                ret = avcodec_receive_frame(d->avctx, frame);
                if (ret >= 0) {
                    AVRational tb = (AVRational) {1, frame->sample_rate};
                    if (frame->pts != AV_NOPTS_VALUE)
                        frame->pts = av_rescale_q(frame->pts, d->avctx->pkt_timebase, tb);
                    else if (d->next_pts != AV_NOPTS_VALUE)
                        frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
                    if (frame->pts != AV_NOPTS_VALUE) {
                        d->next_pts = frame->pts + frame->nb_samples;
                        d->next_pts_tb = tb;
                    }
                }
                if (ret == AVERROR_EOF) {
                    d->finished = d->pkt_serial;
                    avcodec_flush_buffers(d->avctx);
                    return 0;
                }
                if (ret >= 0)
                    return 1;
            } while (ret != AVERROR(EAGAIN));
        }
        do {
            if (d->queue->nb_packets == 0)
                SDL_CondSignal(d->empty_queue_cond);
            if (d->packet_pending) {
                d->packet_pending = 0;
            } else {
                int old_serial = d->pkt_serial;
                if (packet_queue_get(d->queue, d->pkt, 1, &d->pkt_serial) < 0)
                    return -1;
                if (old_serial != d->pkt_serial) {
                    avcodec_flush_buffers(d->avctx);
                    d->finished = 0;
                    d->next_pts = d->start_pts;
                    d->next_pts_tb = d->start_pts_tb;
                }
            }
            if (d->queue->serial == d->pkt_serial)
                break;
            av_packet_unref(d->pkt);
        } while (1);
        if (avcodec_send_packet(d->avctx, d->pkt) == AVERROR(EAGAIN)) {
            av_log(d->avctx, AV_LOG_ERROR, "Receive_frame and send_packet both returned EAGAIN, which is an API violation.\n");
            d->packet_pending = 1;
        } else {
            av_packet_unref(d->pkt);
        }
    }
}

void set_clock_at(Clock *c, double pts, int serial, double time){
    c->pts = pts;
    c->last_updated = time;
    c->pts_drift = c->pts - time;
    c->serial = serial;
}

void set_clock(Clock* c, double pts, int serial){
    double time = av_gettime_relative() / 1000000.0;
    set_clock_at(c, pts, serial, time);
 }

void init_clock(Clock* c, int* queue_serial){
    c->speed = 1.0;
    c->paused = 0;
    c->queue_serial = queue_serial;
    set_clock(c, NAN, -1);
}

double get_clock(Clock *c){
    if (*c->queue_serial != c->serial)
        return NAN;
    if (c->paused) {
        return c->pts;
    } else {
        double time = av_gettime_relative() / 1000000.0;
        return c->pts_drift + time - (time - c->last_updated) * (1.0 - c->speed);
    }
}

void sync_clock_to_slave(Clock *c, Clock *slave){
    double clock = get_clock(c);
    double slave_clock = get_clock(slave);
    if (!isnan(slave_clock) && (isnan(clock) || fabs(clock - slave_clock) > AV_NOSYNC_THRESHOLD))
        set_clock(c, slave_clock, slave->serial);
}

int is_realtime(AVFormatContext* s){
    if(   !strcmp(s->iformat->name, "rtp")
          || !strcmp(s->iformat->name, "rtsp")
          || !strcmp(s->iformat->name, "sdp")
            )
        return 1;

    if(s->pb && (   !strncmp(s->url, "rtp:", 4)
                    || !strncmp(s->url, "udp:", 4)
    )
            )
        return 1;
    return 0;
}

int synchronize_audio(AudioState *is, int nb_samples){
    int wanted_nb_samples = nb_samples;
    return wanted_nb_samples;
}

int audio_decode_frame(AudioState* is){
    int data_size, resampled_data_size;
    int64_t dec_channel_layout;
    av_unused double audio_clock0;
    int wanted_nb_samples;
    Frame *af;

    do{
        if (!(af = frame_queue_peek_readable(&is->sampq)))
            return -1;
        frame_queue_next(&is->sampq);
    }while(af->serial != is->audioq.serial);

    data_size = av_samples_get_buffer_size(NULL, af->frame->channels,
                                           af->frame->nb_samples,
                                           af->frame->format, 1);

    dec_channel_layout =
            (af->frame->channel_layout && af->frame->channels == av_get_channel_layout_nb_channels(af->frame->channel_layout)) ?
            af->frame->channel_layout : av_get_default_channel_layout(af->frame->channels);
    wanted_nb_samples = synchronize_audio(is, af->frame->nb_samples);

    if (af->frame->format        != is->audio_src.fmt            ||
        dec_channel_layout       != is->audio_src.channel_layout ||
        af->frame->sample_rate   != is->audio_src.freq           ||
        (wanted_nb_samples       != af->frame->nb_samples && !is->swr_ctx)) {
        swr_free(&is->swr_ctx);
        is->swr_ctx = swr_alloc_set_opts(NULL,
                                         is->audio_tgt.channel_layout, is->audio_tgt.fmt, is->audio_tgt.freq,
                                         dec_channel_layout, af->frame->format, af->frame->sample_rate,
                                         0, NULL);
        if (!is->swr_ctx || swr_init(is->swr_ctx) < 0) {
            fprintf(stderr,
                    "Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
                    af->frame->sample_rate, av_get_sample_fmt_name(af->frame->format), af->frame->channels,
                    is->audio_tgt.freq, av_get_sample_fmt_name(is->audio_tgt.fmt), is->audio_tgt.channels);
            swr_free(&is->swr_ctx);
            return -1;
        }
        is->audio_src.channel_layout = dec_channel_layout;
        is->audio_src.channels = af->frame->channels;
        is->audio_src.freq = af->frame->sample_rate;
        is->audio_src.fmt = af->frame->format;
    }

    if (is->swr_ctx) {
        const uint8_t **in = (const uint8_t **) af->frame->extended_data;
        uint8_t **out = &is->audio_buf1;
        int out_count = (int64_t) wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate + 256;
        int out_size = av_samples_get_buffer_size(NULL, is->audio_tgt.channels, out_count, is->audio_tgt.fmt, 0);
        int len2;
        if (out_size < 0) {
            fprintf(stderr, "av_samples_get_buffer_size() failed\n");
            return -1;
        }
        if (wanted_nb_samples != af->frame->nb_samples) {
            if (swr_set_compensation(is->swr_ctx, (wanted_nb_samples - af->frame->nb_samples) * is->audio_tgt.freq /
                                                  af->frame->sample_rate,
                                     wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate) < 0) {
                fprintf(stderr, "swr_set_compensation() failed\n");
                return -1;
            }
        }
        av_fast_malloc(&is->audio_buf1, &is->audio_buf1_size, out_size);
        if (!is->audio_buf1)
            return AVERROR(ENOMEM);
        len2 = swr_convert(is->swr_ctx, out, out_count, in, af->frame->nb_samples);
        if (len2 < 0) {
            fprintf(stderr, "swr_convert() failed\n");
            return -1;
        }
        if (len2 == out_count) {
            fprintf(stderr, "audio buffer is probably too small\n");
            if (swr_init(is->swr_ctx) < 0)
                swr_free(&is->swr_ctx);
        }
        is->audio_buf = is->audio_buf1;
        resampled_data_size = len2 * is->audio_tgt.channels * av_get_bytes_per_sample(is->audio_tgt.fmt);
    }
    else {
        is->audio_buf = af->frame->data[0];
        resampled_data_size = data_size;
    }
    audio_clock0 = is->audio_clock;

    if (!isnan(af->pts))
        is->audio_clock = af->pts + (double) af->frame->nb_samples / af->frame->sample_rate;
    else
        is->audio_clock = NAN;
    is->audio_clock_serial = af->serial;

#ifdef DEBUG
    {
        static double last_clock;
        printf("audio: delay=%0.3f clock=%0.3f clock0=%0.3f\n",
               is->audio_clock - last_clock,
               is->audio_clock, audio_clock0);
        last_clock = is->audio_clock;
    }
#endif

    return resampled_data_size;
}

void sdl_audio_callback(void *opaque, Uint8 *stream, int len){
    AudioState *is = opaque;
    int audio_size, len1;

    audio_callback_time = av_gettime_relative();

    while(len > 0){
        if (is->audio_buf_index >= is->audio_buf_size) {
            audio_size = audio_decode_frame(is);
            if (audio_size < 0) {
                /* if error, just output silence */
                is->audio_buf = NULL;
                is->audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / is->audio_tgt.frame_size * is->audio_tgt.frame_size;
            }
            else{
                is->audio_buf_size = audio_size;
            }
            is->audio_buf_index = 0;
        }
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len)
            len1 = len;
        if (is->audio_buf && is->audio_volume == SDL_MIX_MAXVOLUME)
            memcpy(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, len1);
        else {
            memset(stream, 0, len1);
            if (is->audio_buf)
                SDL_MixAudioFormat(stream, (uint8_t *)is->audio_buf + is->audio_buf_index, AUDIO_S16SYS, len1, is->audio_volume);
        }
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
    is->audio_write_buf_size = is->audio_buf_size - is->audio_buf_index;
//    if (!isnan(is->audio_clock)) {
//        set_clock_at(&is->audclk, is->audio_clock - (double)(2 * is->audio_hw_buf_size + is->audio_write_buf_size) / is->audio_tgt.bytes_per_sec, is->audio_clock_serial, audio_callback_time / 1000000.0);
//        sync_clock_to_slave(&is->extclk, &is->audclk);
//    }
}

int audio_thread(void *arg){
    AudioState *is = arg;
    AVFrame *frame = av_frame_alloc();
    Frame *af;
    int got_frame = 0;
    AVRational tb;
    int ret = 0;

    if (!frame)
        return AVERROR(ENOMEM);

    do {
        if ((got_frame = decoder_decode_frame(&is->auddec, frame, NULL)) < 0)
            goto the_end;

        if (got_frame) {
            tb = (AVRational) {1, frame->sample_rate};

            if (!(af = frame_queue_peek_writable(&is->sampq)))
                goto the_end;

            af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
            af->pos = frame->pkt_pos;
            af->serial = is->auddec.pkt_serial;
            af->duration = av_q2d((AVRational) {frame->nb_samples, frame->sample_rate});

            av_frame_move_ref(af->frame, frame);
            frame_queue_push(&is->sampq);
        }
    } while (ret >= 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF);

    the_end:
        av_frame_free(&frame);
        return ret;
}

int audio_open(void *opaque, int64_t wanted_channel_layout, int wanted_nb_channels, int wanted_sample_rate, struct AudioParams* audio_hw_params){
    SDL_AudioSpec wanted_spec, spec;
    const char *env;
    const int next_nb_channels[] = {0, 0, 1, 6, 2, 6, 4, 6};
    const int next_sample_rates[] = {0, 44100, 48000, 96000, 192000};
    int next_sample_rate_idx = FF_ARRAY_ELEMS(next_sample_rates) -1;

    env = SDL_getenv("SDL_AUDIO_CHANNELS");
    if(env){
        wanted_nb_channels = atoi(env);
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
    }
    if(!wanted_channel_layout || wanted_nb_channels != av_get_channel_layout_nb_channels(wanted_channel_layout)){
        wanted_channel_layout = av_get_default_channel_layout(wanted_nb_channels);
        wanted_channel_layout &= ~AV_CH_LAYOUT_STEREO_DOWNMIX;
    }
    wanted_nb_channels = av_get_channel_layout_nb_channels(wanted_channel_layout);
    wanted_spec.channels = wanted_nb_channels;
    wanted_spec.freq = wanted_sample_rate;
    if (wanted_spec.freq <= 0 || wanted_spec.channels <= 0) {
        fprintf(stderr, "Invalid sample rate or channel count!\n");
        return -1;
    }
    while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq)
        next_sample_rate_idx--;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.silence = 0;
    wanted_spec.samples = FFMAX(SDL_AUDIO_MIN_BUFFER_SIZE, 2 << av_log2(wanted_spec.freq / SDL_AUDIO_MAX_CALLBACKS_PER_SEC));
    wanted_spec.callback = sdl_audio_callback;
    wanted_spec.userdata = opaque;

    while (!(audio_dev = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_CHANNELS_CHANGE))) {
        fprintf(stderr, "SDL_OpenAudio (%d channels, %d Hz): %s\n",wanted_spec.channels, wanted_spec.freq, SDL_GetError());
        wanted_spec.channels = next_nb_channels[FFMIN(7, wanted_spec.channels)];
        if (!wanted_spec.channels) {
            wanted_spec.freq = next_sample_rates[next_sample_rate_idx--];
            wanted_spec.channels = wanted_nb_channels;
            if (!wanted_spec.freq) {
                fprintf(stderr,"No more combinations to try, audio open failed\n");
                return -1;
            }
        }
        wanted_channel_layout = av_get_default_channel_layout(wanted_spec.channels);
    }
    if (spec.format != AUDIO_S16SYS) {
        fprintf(stderr,"SDL advised audio format %d is not supported!\n", spec.format);
        return -1;
    }
    if (spec.channels != wanted_spec.channels) {
        wanted_channel_layout = av_get_default_channel_layout(spec.channels);
        if (!wanted_channel_layout) {
            fprintf(stderr, "SDL advised channel count %d is not supported!\n", spec.channels);
            return -1;
        }
    }
    audio_hw_params->fmt = AV_SAMPLE_FMT_S16;
    audio_hw_params->freq = spec.freq;
    audio_hw_params->channel_layout = wanted_channel_layout;
    audio_hw_params->channels =  spec.channels;
    audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
    if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
        fprintf(stderr, "av_samples_get_buffer_size failed\n");
        return -1;
    }
    return spec.size;
}

int stream_has_enough_packets(AVStream *st, int stream_id, PacketQueue *queue) {
    return stream_id < 0 ||
           queue->abort_request ||
           (st->disposition & AV_DISPOSITION_ATTACHED_PIC) ||
           queue->nb_packets > MIN_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0);
}

int stream_component_open(AudioState* is, int stream_index){
    AVFormatContext *ic = is->ic;
    AVCodecContext *avctx;
    const AVCodec *codec;
    const char *forced_codec_name = NULL;
    AVDictionary *opts = NULL;
    const AVDictionaryEntry *t = NULL;
    int sample_rate, nb_channels;
    int64_t channel_layout;
    int ret = 0;

    if(stream_index < 0 || stream_index >= ic->nb_streams)
        return -1;

    avctx = avcodec_alloc_context3(NULL);
    if (!avctx)
        return AVERROR(ENOMEM);

    ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
    if(ret < 0)
        goto fail;
    avctx->pkt_timebase = ic->streams[stream_index]->time_base;

    codec = avcodec_find_decoder(avctx->codec_id);

    is->last_audio_stream = stream_index;
    if(!codec){
        fprintf(stderr, "No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
        ret = AVERROR(EINVAL);
        goto fail;
    }

    avctx->codec_id = codec->id;

    opts = filter_codec_opts(codec_opts, avctx->codec_id, ic, ic->streams[stream_index], codec);

    if (!av_dict_get(opts, "threads", NULL, 0))
        av_dict_set(&opts, "threads", "auto", 0);
    if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) {
        goto fail;
    }
    if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        ret =  AVERROR_OPTION_NOT_FOUND;
        goto fail;
    }

    is->eof = 0;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;

    sample_rate    = avctx->sample_rate;
    nb_channels    = avctx->channels;
    channel_layout = avctx->channel_layout;

    if ((ret = audio_open(is, channel_layout, nb_channels, sample_rate, &is->audio_tgt)) < 0)
        goto fail;
    is->audio_hw_buf_size = ret;
    is->audio_src = is->audio_tgt;
    is->audio_buf_size  = 0;
    is->audio_buf_index = 0;

    is->audio_diff_avg_coef  = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
    is->audio_diff_avg_count = 0;

    is->audio_diff_threshold = (double)(is->audio_hw_buf_size) / is->audio_tgt.bytes_per_sec;

    is->audio_stream = stream_index;
    is->audio_st = ic->streams[stream_index];

    if ((ret = decoder_init(&is->auddec, avctx, &is->audioq, is->continue_read_thread)) < 0)
        goto fail;
//    if ((is->ic->iformat->flags & (AVFMT_NOBINSEARCH | AVFMT_NOGENSEARCH | AVFMT_NO_BYTE_SEEK)) && !is->ic->iformat->read_seek) {
//        is->auddec.start_pts = is->audio_st->start_time;
//        is->auddec.start_pts_tb = is->audio_st->time_base;
//    }
    if ((ret = decoder_start(&is->auddec, audio_thread, "audio_decoder", is)) < 0)
        goto out;
    SDL_PauseAudioDevice(audio_dev, 0);
    goto out;

    fail:
        avcodec_free_context(&avctx);
    out:
        av_dict_free(&opts);

        return ret;
}

int read_thread(void *arg){
    AudioState *is = arg;
    AVFormatContext *ic = NULL;
    int err, i, ret;
    int audio_index = -1;
    AVPacket *pkt = NULL;
    int64_t stream_start_time;
    int pkt_in_play_range = 0;
    const AVDictionaryEntry *t;
    SDL_mutex *wait_mutex = SDL_CreateMutex();
    int scan_all_pmts_set = 0;
    int64_t pkt_ts;

    if(!wait_mutex){
        fprintf(stderr, "SDL_CreateMutex(): %s\n", SDL_GetError());
        ret = AVERROR(ENOMEM);
        goto fail;
    }

//    memset(st_index, -1, sizeof(st_index));
    is->eof = 0;

    pkt = av_packet_alloc();
    if(!pkt){
        fprintf(stderr, "Could not allocate packet\n");
        ret = AVERROR(ENOMEM);
        goto fail;
    }
    ic = avformat_alloc_context();
    if(!ic){
        fprintf(stderr, "COuld not allocate context");
        ret = AVERROR(ENOMEM);
        goto fail;
    }
    ic->interrupt_callback.callback = decode_interrupt_cb;
    ic->interrupt_callback.opaque = is;

    if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
        av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
        scan_all_pmts_set = 1;
    }

    err = avformat_open_input(&ic, is->filename, NULL, &format_opts);
    if(err < 0){
        fprintf(stderr, is->filename, err);
        ret = -1;
        goto fail;
    }

    if (scan_all_pmts_set)
        av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

    if ((t = av_dict_get(format_opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        av_log(NULL, AV_LOG_ERROR, "Option %s not found.\n", t->key);
        ret = AVERROR_OPTION_NOT_FOUND;
        goto fail;
    }

    is->ic = ic;

    av_format_inject_global_side_data(ic);

    AVDictionary **opts = setup_find_stream_info_opts(ic, codec_opts);
    int orig_nb_streams = ic->nb_streams;

    err = avformat_find_stream_info(ic, opts);

    for (i = 0; i < orig_nb_streams; i++)
        av_dict_free(&opts[i]);
    av_freep(&opts);

    if (err < 0) {
        av_log(NULL, AV_LOG_WARNING,
               "%s: could not find codec parameters\n", is->filename);
        ret = -1;
        goto fail;
    }

    if (ic->pb)
        ic->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end

    is->max_frame_duration = (ic->iformat->flags & AVFMT_TS_DISCONT) ? 10.0 : 3600.0;

    is->realtime = is_realtime(ic);
    fprintf(stderr, "t : %d\n", is->realtime);

//    av_dump_format(ic, 0, is->filename, 0);


    fprintf(stderr, "nb stream : %d\n", ic->nb_streams);
//    for (i = 0; i < ic->nb_streams; i++) {
//        AVStream *st = ic->streams[i];
//        enum AVMediaType type = st->codecpar->codec_type;
//        st->discard = AVDISCARD_ALL;
//        fprintf(stderr, "stindex : %d, type : %d, i : %d\n", st_index[type], type, i);
//        if (type >= 0 && wanted_stream_spec[type] && st_index[type] == -1)
//            if (avformat_match_stream_specifier(ic, st, wanted_stream_spec[type]) > 0){
//                fprintf(stderr, "stindex : %d, type : %d, i : %d\n", st_index[type], type, i);
//                st_index[type] = i;
//            }
//    }
//    for (i = 0; i < AVMEDIA_TYPE_NB; i++) {
//        if (wanted_stream_spec[i] && st_index[i] == -1) {
//            fprintf(stderr, "Stream specifier %s does not match any %s stream\n", wanted_stream_spec[i], av_get_media_type_string(i));
//            st_index[i] = INT_MAX;
//        }
//    }

    for(i = 0; i < ic->nb_streams; i++){
        if(ic->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audio_index < 0)
            audio_index = i;
    }
    if(audio_index < 0){
        fprintf(stderr, "could not find audio stream");
        ret -1;
        goto fail;
    }

    stream_component_open(is, audio_index);

    ret = -1;

    if(is->audio_stream < 0){
        fprintf(stderr, "Failed to open file %s", is->filename);
        ret = -1;
        goto fail;
    }

    if(infinite_buffer < 0 && is->realtime){
        infinite_buffer = 1;
    }

    for(;;){
        if(is->abort_request){
            break;
        }
        if(infinite_buffer < 1 && (is->audioq.size > MAX_QUEUE_SIZE || stream_has_enough_packets(is->audio_st, is->audio_stream, &is->audioq))){
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
        ret = av_read_frame(ic, pkt);
        if(ret < 0){
            if((ret == AVERROR_EOF || avio_feof(ic->pb)) && !is->eof){
                if(is->audio_stream >= 0){
                    packet_queue_put_nullpacket(&is->audioq, pkt, is->audio_stream);
                }
                is->eof = 1;
            }
            if (ic->pb && ic->pb->error) {
                break;
            }
            SDL_LockMutex(wait_mutex);
            SDL_CondWaitTimeout(is->continue_read_thread, wait_mutex, 10);
            SDL_UnlockMutex(wait_mutex);
            continue;
        }
        else {
            is->eof = 0;
        }
        if (pkt->stream_index == is->audio_stream) {
            packet_queue_put(&is->audioq, pkt);
        }
        else {
            av_packet_unref(pkt);
        }

    }

    ret = 0;

fail:
    if (ic && !is->ic)
        avformat_close_input(&ic);

    av_packet_free(&pkt);
    if (ret != 0) {
        SDL_Event event;

        event.type = FF_QUIT_EVENT;
        event.user.data1 = is;
        SDL_PushEvent(&event);
    }
    SDL_DestroyMutex(wait_mutex);
    return 0;
}

static void stream_component_close(AudioState* is, int stream_index){
    AVFormatContext  *ic = is->ic;

    if(stream_index < 0 || stream_index >= ic->nb_streams)
        return;

    decoder_abort(&is->auddec, &is->sampq);
    SDL_CloseAudioDevice(audio_dev);
    decoder_destroy(&is->auddec);
    swr_free(&is->swr_ctx);
    av_free(&is->audio_buf1);
    is->audio_buf1_size = 0;
    is->audio_buf = NULL;

    if(is->rdft){
        av_rdft_end(is->rdft);
        av_freep(&is->rdft_data);
        is->rdft = NULL;
        is->rdft_bits = 0;
    }

    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    is->audio_st = NULL;
    is->audio_stream = -1;
}

void stream_close(AudioState* is){
    is->abort_request = 1;
    SDL_WaitThread(is->read_tid, NULL);

    if(is->audio_stream >= 0)
        stream_component_close(is, is->audio_stream);

    avformat_close_input(&is->ic);

    packet_queue_destroy(&is->audioq);
    frame_queue_destroy(&is->sampq);

    SDL_DestroyCond(is->continue_read_thread);

    av_free(is);

}

void do_exit(AudioState *is)
{
    if (is) {
        stream_close(is);
    }

    uninit_opts();
    avformat_network_deinit();
    printf("\n");
    SDL_Quit();
    av_log(NULL, AV_LOG_QUIET, "%s", "");
    exit(0);
}
AudioState* stream_open(const char * filename){
    AudioState *is;

    is = av_mallocz(sizeof(AudioState));
    if(!is)
        return NULL;
    is->last_audio_stream = is->audio_stream = -1;
    is->filename = av_strdup(filename);
    if(!is->filename){
        stream_close(is);
        return NULL;
    }
    is->iformat = NULL;

    if(frame_queue_init(&is->sampq, &is->audioq, SAMPLE_QUEUE_SIZE, 1) < 0){
        stream_close(is);
        return NULL;
    }

    if(packet_queue_init(&is->audioq) < 0){
        stream_close(is);
        return NULL;
    }

    if(!(is->continue_read_thread = SDL_CreateCond())){
        fprintf(stderr, "SDL_CreateCond() : %s\n", SDL_GetError());
        stream_close(is);
        return NULL;
    }

    init_clock(&is->audclk, &is->audioq.serial);
    is->audio_clock_serial = -1;

    is->audio_volume = 128;
    is->av_sync_type = AV_SYNC_AUDIO_MASTER;
    is->read_tid = SDL_CreateThread(read_thread, "read_thread", is);
    if(!is->read_tid){
        fprintf(stderr, "SDL_CreateThread(): %s\n", SDL_GetError());
        stream_close(is);
        return NULL;
    }
    return is;
}



void event_loop(AudioState *cur_stream){
    SDL_Event event;
    double incr, pos, frac;

    for (;;) {
        double x;
        switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q) {
                    do_exit(cur_stream);
                    break;
                }
            case SDL_QUIT:
            case FF_QUIT_EVENT:
                do_exit(cur_stream);
                break;
            default:
                break;
        }
    }
}

int main(int argc, char** argv){

    if(argc < 2){
        fprintf(stderr, "not enough args\n");
        exit(1);
    }

    AudioState* is;

    avformat_network_init();

    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_TIMER)){
        fprintf(stderr, "Could not initialize SDL- %s\n", SDL_GetError());
        exit(1);
    }

    is = stream_open(argv[1]);
    if(!is){
        fprintf(stderr, "Failed to initialize AudioState\n");
        exit(1);
    }

    event_loop(is);

    return 0;
}
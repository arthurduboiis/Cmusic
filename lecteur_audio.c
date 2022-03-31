//
// Created by tanguy on 02/03/2022.
//

#include "common.h"
#include "flag_thread.h"

#include "lecteur_audio.h"

#include <libavutil/avstring.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mathematics.h>
#include <libavutil/dict.h>
#include <libavutil/fifo.h>
#include <libavutil/samplefmt.h>
#include <libavutil/opt.h>
#include <libavformat/avformat.h>
#include <libavcodec/avfft.h>
#include <libswresample/swresample.h>

#define FF_QUIT_EVENT    (SDL_USEREVENT + 2)
#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE 9

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

typedef struct AudioParams {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
} AudioParams;

typedef struct Frame {
    AVFrame *frame;
    int serial;
    double pts;           /* presentation timestamp for the frame */
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

typedef struct Decoder {
    AVPacket *pkt;
    PacketQueue *queue;
    AVCodecContext *avctx;
    int pkt_serial;
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
    int abort_request;
    AVFormatContext *ic;
    FrameQueue sampq;
    Decoder auddec;
    int audio_stream;
    AVStream *audio_st;
    PacketQueue audioq;
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    unsigned int audio_buf_size; /* in bytes */
    unsigned int audio_buf1_size;
    int audio_buf_index; /* in bytes */
    int audio_volume;
    struct AudioParams audio_src;
    struct AudioParams audio_tgt;
    struct SwrContext *swr_ctx;
    RDFTContext *rdft;
    FFTSample *rdft_data;
    int eof;
    char *filename;
    SDL_cond *continue_read_thread;
} AudioState;

SDL_AudioDeviceID audio_dev;


int packet_queue_init(PacketQueue *q){
    memset(q, 0, sizeof(PacketQueue));

    q->pkt_list = av_fifo_alloc(sizeof(MyAVPacketList));
    if(!q->pkt_list) {
        return -1;
    }
    q->mutex = SDL_CreateMutex();
    if (!q->mutex) {
        fprintf(stderr, "SDL_CreateMutex(): %s\n", SDL_GetError());
        return -1;
    }
    q->cond = SDL_CreateCond();
    if (!q->cond) {
        fprintf(stderr, "SDL_CreateCond(): %s\n", SDL_GetError());
        return -1;
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
            if (serial) {
                *serial = pkt1.serial;
            }
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
        return -1;
    }
    if (!(f->cond = SDL_CreateCond())) {
        fprintf(stderr, "SDL_CreateCond(): %s\n", SDL_GetError());
        return -1;
    }

    f->pktq = pktq;
    f->max_size = FFMIN(max_size, FRAME_QUEUE_SIZE);
    f->keep_last = keep_last;

    for(i = 0; i < f->max_size; i++){
        if(!(f->queue[i].frame = av_frame_alloc()))
            return -1;
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
    while (f->size - f->rindex_shown <= 0 && !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request) {
        return NULL;
    }

    return &f->queue[(f->rindex + f->rindex_shown) % f->max_size];
}

void frame_queue_next(FrameQueue *f)
{
    if (f->keep_last && !f->rindex_shown) {
        f->rindex_shown = 1;
        return;
    }
    frame_queue_unref_item(&f->queue[f->rindex]);
    if (++f->rindex == f->max_size) {
        f->rindex = 0;
    }
    SDL_LockMutex(f->mutex);
    f->size--;
    SDL_CondSignal(f->cond);
    SDL_UnlockMutex(f->mutex);
}

Frame* frame_queue_peek_writable(FrameQueue *f)
{
    SDL_LockMutex(f->mutex);
    while (f->size >= f->max_size && !f->pktq->abort_request) {
        SDL_CondWait(f->cond, f->mutex);
    }
    SDL_UnlockMutex(f->mutex);

    if (f->pktq->abort_request) {
        return NULL;
    }

    return &f->queue[f->windex];
}

void frame_queue_push(FrameQueue *f){
    if (++f->windex == f->max_size) {
        f->windex = 0;
    }
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
    if (!d->pkt) {
        return -1;
    }
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
        return -1;
    }
    return 0;
}

int decoder_decode_frame(Decoder *d, AVFrame *frame){
    int ret;

    for (;;) {
        if (d->queue->serial == d->pkt_serial) {
            do {
                if (d->queue->abort_request){
                    return -1;
                }
                ret = avcodec_receive_frame(d->avctx, frame);
                if (ret >= 0) {
                    AVRational tb = (AVRational) {1, frame->sample_rate};
                    if (frame->pts != AV_NOPTS_VALUE) {
                        frame->pts = av_rescale_q(frame->pts, d->avctx->pkt_timebase, tb);
                    }
                    else if (d->next_pts != AV_NOPTS_VALUE) {
                        frame->pts = av_rescale_q(d->next_pts, d->next_pts_tb, tb);
                    }
                    if (frame->pts != AV_NOPTS_VALUE) {
                        d->next_pts = frame->pts + frame->nb_samples;
                        d->next_pts_tb = tb;
                    }
                }
                if (ret == AVERROR_EOF) {
                    avcodec_flush_buffers(d->avctx);
                    return 0;
                }
                if (ret >= 0) {
                    return 1;
                }
            } while (ret != AVERROR(EAGAIN));
        }
        do {
            if (d->queue->nb_packets == 0) {
                SDL_CondSignal(d->empty_queue_cond);
            }
            if (d->packet_pending) {
                d->packet_pending = 0;
            } else {
                int old_serial = d->pkt_serial;
                if (packet_queue_get(d->queue, d->pkt, 1, &d->pkt_serial) < 0) {
                    return -1;
                }
                if (old_serial != d->pkt_serial) {
                    avcodec_flush_buffers(d->avctx);
                    d->next_pts = d->start_pts;
                    d->next_pts_tb = d->start_pts_tb;
                }
            }
            if (d->queue->serial == d->pkt_serial)
                break;
            av_packet_unref(d->pkt);
        } while (1);
        if (avcodec_send_packet(d->avctx, d->pkt) == AVERROR(EAGAIN)) {
            fprintf(stderr, "Receive_frame and send_packet both returned EAGAIN, which is an API violation\n");
            d->packet_pending = 1;
        } else {
            av_packet_unref(d->pkt);
        }
    }
}

int audio_decode_frame(AudioState* is){
    int data_size, resampled_data_size;
    int64_t dec_channel_layout;
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
    wanted_nb_samples = af->frame->nb_samples;

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
            fprintf(stderr,"Cannot create sample rate converter for conversion of %d Hz %s %d channels to %d Hz %s %d channels!\n",
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
            if (swr_set_compensation(is->swr_ctx, (wanted_nb_samples - af->frame->nb_samples) * is->audio_tgt.freq / af->frame->sample_rate,wanted_nb_samples * is->audio_tgt.freq / af->frame->sample_rate) < 0) {
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

    return resampled_data_size;
}

void sdl_audio_callback(void *opaque, Uint8 *stream, int len){
    AudioState *is = opaque;
    int audio_size, len1;


    while(len > 0){
        if (is->audio_buf_index >= is->audio_buf_size) {
            audio_size = audio_decode_frame(is);
            if (audio_size < 0) {
                is->audio_buf = NULL;
                is->audio_buf_size = SDL_AUDIO_MIN_BUFFER_SIZE / is->audio_tgt.frame_size * is->audio_tgt.frame_size;
            }
            else{
                is->audio_buf_size = audio_size;
            }
            is->audio_buf_index = 0;
        }
        len1 = is->audio_buf_size - is->audio_buf_index;
        if (len1 > len) {
            len1 = len;
        }
        if (is->audio_buf && is->audio_volume == SDL_MIX_MAXVOLUME) {
            memcpy(stream, (uint8_t *) is->audio_buf + is->audio_buf_index, len1);
        }
        else {
            memset(stream, 0, len1);
            if (is->audio_buf) {
                SDL_MixAudioFormat(stream, (uint8_t *) is->audio_buf + is->audio_buf_index, AUDIO_S16SYS, len1, is->audio_volume);
            }
        }
        len -= len1;
        stream += len1;
        is->audio_buf_index += len1;
    }
}

int audio_thread(void *arg){
    AudioState *is = arg;
    AVFrame *frame = av_frame_alloc();
    Frame *af;
    int got_frame;
    AVRational tb;
    int ret = 0;

    if (!frame) {
        return -1;
    }

    do {
        if ((got_frame = decoder_decode_frame(&is->auddec, frame)) < 0) {
            goto the_end;
        }

        if (got_frame) {
            tb = (AVRational) {1, frame->sample_rate};

            if (!(af = frame_queue_peek_writable(&is->sampq)))
                goto the_end;

            af->pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
            af->serial = is->auddec.pkt_serial;

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
    while (next_sample_rate_idx && next_sample_rates[next_sample_rate_idx] >= wanted_spec.freq) {
        next_sample_rate_idx--;
    }
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
    audio_hw_params->channels = spec.channels;
    audio_hw_params->frame_size = av_samples_get_buffer_size(NULL, audio_hw_params->channels, 1, audio_hw_params->fmt, 1);
    audio_hw_params->bytes_per_sec = av_samples_get_buffer_size(NULL, audio_hw_params->channels, audio_hw_params->freq, audio_hw_params->fmt, 1);
    if (audio_hw_params->bytes_per_sec <= 0 || audio_hw_params->frame_size <= 0) {
        fprintf(stderr, "av_samples_get_buffer_size failed\n");
        return -1;
    }
    return spec.size;
}

int stream_has_enough_packets(AVStream *st, int stream_id, PacketQueue *queue) {
    return stream_id < 0 || queue->abort_request || (st->disposition & AV_DISPOSITION_ATTACHED_PIC) || queue->nb_packets > MIN_FRAMES && (!queue->duration || av_q2d(st->time_base) * queue->duration > 1.0);
}

int check_stream_specifier(AVFormatContext *s, AVStream *st, const char *spec)
{
    int ret = avformat_match_stream_specifier(s, st, spec);
    if (ret < 0)
        av_log(s, AV_LOG_ERROR, "Invalid stream specifier: %s.\n", spec);
    return ret;
}

AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                AVFormatContext *s, AVStream *st, const AVCodec *codec)
{
    AVDictionary *ret = NULL;
    const AVDictionaryEntry *t = NULL;
    int flags = s->oformat ? AV_OPT_FLAG_ENCODING_PARAM
                                      : AV_OPT_FLAG_DECODING_PARAM;
    char prefix;
    const AVClass *cc = avcodec_get_class();

    if (!codec)
        codec            = s->oformat ? avcodec_find_encoder(codec_id)
                                      : avcodec_find_decoder(codec_id);


    prefix  = 'a';
    flags  |= AV_OPT_FLAG_AUDIO_PARAM;



    while (t = av_dict_get(opts, "", t, AV_DICT_IGNORE_SUFFIX)) {
        const AVClass *priv_class;
        char *p = strchr(t->key, ':');

        /* check stream specification in opt name */
        if (p)
            switch (check_stream_specifier(s, st, p + 1)) {
                case  1: *p = 0; break;
                case  0:         continue;
                default:         exit(1);
            }

        if (av_opt_find(&cc, t->key, NULL, flags, AV_OPT_SEARCH_FAKE_OBJ) ||
            !codec ||
            ((priv_class = codec->priv_class) &&
             av_opt_find(&priv_class, t->key, NULL, flags,
                         AV_OPT_SEARCH_FAKE_OBJ)))
            av_dict_set(&ret, t->key, t->value, 0);
        else if (t->key[0] == prefix &&
                 av_opt_find(&cc, t->key + 1, NULL, flags,
                             AV_OPT_SEARCH_FAKE_OBJ))
            av_dict_set(&ret, t->key + 1, t->value, 0);

        if (p)
            *p = ':';
    }
    return ret;
}

int stream_component_open(AudioState* is, int stream_index){
    AVFormatContext *ic = is->ic;
    AVCodecContext *avctx;
    const AVCodec *codec;
    AVDictionary *opts = NULL;
    const AVDictionaryEntry *t = NULL;
    int sample_rate, nb_channels;
    int64_t channel_layout;
    int ret;

    if(stream_index < 0 || stream_index >= ic->nb_streams) {
        return -1;
    }

    avctx = avcodec_alloc_context3(NULL);
    if (!avctx) {
        return -1;
    }

    ret = avcodec_parameters_to_context(avctx, ic->streams[stream_index]->codecpar);
    if(ret < 0) {
        goto fail;
    }
    avctx->pkt_timebase = ic->streams[stream_index]->time_base;

    codec = avcodec_find_decoder(avctx->codec_id);

    if(!codec){
        fprintf(stderr, "No decoder could be found for codec %s\n", avcodec_get_name(avctx->codec_id));
        ret = -1;
        goto fail;
    }

    avctx->codec_id = codec->id;

    opts = filter_codec_opts(NULL, avctx->codec_id, ic, ic->streams[stream_index], codec);

    if (!av_dict_get(opts, "threads", NULL, 0))
        av_dict_set(&opts, "threads", "auto", 0);
    if ((ret = avcodec_open2(avctx, codec, &opts)) < 0) {
        goto fail;
    }
    if ((t = av_dict_get(opts, "", NULL, AV_DICT_IGNORE_SUFFIX))) {
        fprintf(stderr, "Option %s not found\n", t->key);
        ret =  -1;
        goto fail;
    }

    is->eof = 0;
    ic->streams[stream_index]->discard = AVDISCARD_DEFAULT;

    sample_rate = avctx->sample_rate;
    nb_channels = avctx->channels;
    channel_layout = avctx->channel_layout;

    if ((ret = audio_open(is, channel_layout, nb_channels, sample_rate, &is->audio_tgt)) < 0) {
        goto fail;
    }
    is->audio_src = is->audio_tgt;
    is->audio_buf_size = 0;
    is->audio_buf_index = 0;

    is->audio_stream = stream_index;
    is->audio_st = ic->streams[stream_index];

    if ((ret = decoder_init(&is->auddec, avctx, &is->audioq, is->continue_read_thread)) < 0) {
        goto fail;
    }
    if ((ret = decoder_start(&is->auddec, audio_thread, "audio_decoder", is)) < 0) {
        goto out;
    }
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
    SDL_mutex *wait_mutex = SDL_CreateMutex();

    if(!wait_mutex){
        fprintf(stderr, "SDL_CreateMutex(): %s\n", SDL_GetError());
        ret = -1;
        goto fail;
    }

    is->eof = 0;

    pkt = av_packet_alloc();
    if(!pkt){
        fprintf(stderr, "Could not allocate packet\n");
        ret = -1;
        goto fail;
    }

    ic = avformat_alloc_context();
    if(!ic){
        fprintf(stderr, "Could not allocate context\n");
        ret = -1;
        goto fail;
    }
    ic->interrupt_callback.callback = decode_interrupt_cb;
    ic->interrupt_callback.opaque = is;

    err = avformat_open_input(&ic, is->filename, NULL, NULL);
    if(err < 0){
        fprintf(stderr, is->filename, err);
        ret = -1;
        goto fail;
    }
    is->ic = ic;

    av_format_inject_global_side_data(ic);

    err = avformat_find_stream_info(ic, NULL);

    if (err < 0) {
        fprintf(stderr, "%s: could not find codec parameters\n", is->filename);
        ret = -1;
        goto fail;
    }

    if (ic->pb) {
        ic->pb->eof_reached = 0;
    }

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

    if(is->audio_stream < 0){
        fprintf(stderr, "Failed to open file %s", is->filename);
        ret = -1;
        goto fail;
    }

    for(;;){

        if(quit)
            break;
        if(stopL){
            stopL = 0;
            break;
        }

        is->audio_volume = volume;

        if(is->abort_request){
            break;
        }
        if(is->audioq.size > MAX_QUEUE_SIZE || stream_has_enough_packets(is->audio_st, is->audio_stream, &is->audioq)){
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
    }

    ic->streams[stream_index]->discard = AVDISCARD_ALL;
    is->audio_st = NULL;
    is->audio_stream = -1;
}

void stream_close(AudioState* is){
    is->abort_request = 1;
    SDL_WaitThread(is->read_tid, NULL);

    if(is->audio_stream >= 0) {
        stream_component_close(is, is->audio_stream);
    }
    avformat_close_input(&is->ic);

    packet_queue_destroy(&is->audioq);
    frame_queue_destroy(&is->sampq);

    SDL_DestroyCond(is->continue_read_thread);

    av_free(is);
}

AudioState* stream_open(const char* filename){
    AudioState* is;

    is = av_mallocz(sizeof(AudioState));
    if(!is){
        fprintf(stderr, "malloc audiostate failed\n");
        return NULL;
    }
    is->audio_stream = -1;
    is->filename = av_strdup(filename);
    if(!is->filename)
    {
        stream_close(is);
        return NULL;
    }

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

    is->audio_volume = volume;

    is->read_tid = SDL_CreateThread(read_thread, "read_thread", is);
    if(!is->read_tid){
        fprintf(stderr, "SDL_CreateThread(): %s\n", SDL_GetError());
        stream_close(is);
        return NULL;
    }

    return is;
}

int thread_lecteur_audio(void *arg){
    AudioState* is;

    avformat_network_init();

    is = stream_open((char*) arg);
    if(!is){
        fprintf(stderr, "Failed to initialize AudioState\n");
        return -1;
    }
    return 0;
}
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

#include "libavutil/avstring.h"
#include "libavutil/channel_layout.h"
#include "libavutil/eval.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/imgutils.h"
#include "libavutil/dict.h"
#include "/home/tanguy/Documents/FFmpeg-master/libavutil/fifo.h"
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

extern AVDictionary *format_opts, *codec_opts;

AVDictionary **setup_find_stream_info_opts(AVFormatContext*, AVDictionary*);
AVDictionary *filter_codec_opts(AVDictionary*, enum AVCodecID, AVFormatContext*, AVStream*, const AVCodec*);
void uninit_opts(void);
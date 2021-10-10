/*
 * utils for libavcodec
 * Copyright (c) 2001 Fabrice Bellard
 * Copyright (c) 2002-2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * utils.
 */

#include "libavutil/avstring.h"
#include "libavutil/crc.h"
#include "libavutil/mathematics.h"
#include "libavutil/pixdesc.h"
#include "libavutil/audioconvert.h"
#include "libavutil/imgutils.h"
#include "libavutil/samplefmt.h"
#include "libavutil/dict.h"
#include "libavutil/avassert.h"
#include "libavutil/internal.h"
#include "avcodec.h"
#include "dsputil.h"
#include "libavutil/opt.h"
#include "imgconvert.h"
#include "thread.h"
#include "audioconvert.h"
#include "internal.h"
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>
#include <float.h>
#include "libavutil/internal.h"
#include "ite/audio.h"
#include <stdio.h>
#include <stddef.h>

/* ITE meddle */
#include "ite/itv.h"

#include "ite/ith.h"
#include "ite/itp.h"
#include "castor3player.h"

bool bPlayingBootVideo = false;
extern int64_t      global_video_pkt_pts;

static int volatile entangled_thread_counter = 0;
static int          (*ff_lockmgr_cb)(void **mutex, enum AVLockOp op);
static void         *codec_mutex;
static void         *avformat_mutex;

void updateMessageQueue();

#if defined(__OPENRTOS__) && defined(CFG_AUDIO_ENABLE)
    #define CONFIG_ITADRIVER
#endif
//#define CONFIG_GNASH_FLASH
//#define CONFIG_DUMP_AUDIO

#ifdef CONFIG_DUMP_AUDIO
char        inFileName[256]  = "D:\\Castor3_Alpha\\test_data\\dump_data.mp3";
static FILE *fin             = NULL;
char        inFileName1[256] = "D:\\Castor3_Alpha\\test_data\\dump_data1.mp3";
static FILE *fin1            = NULL;
char        inFileName2[256] = "D:\\Castor3_Alpha\\test_data\\dump_data2.mp3";
static FILE *fin2            = NULL;
#endif

#ifdef CONFIG_ITADRIVER
    #ifdef CONFIG_GNASH_FLASH
ITE_FlashInfo         gFlashInfo;
    #else
ITE_WaveInfo          gWaveInfo;
        int                   gWavEnable =0;
        #define SWITCH_WORKAROUND
    #endif
char                  gWaveHeader[48];
#endif
static struct timeval gStartT, gEndT;

#ifdef CONFIG_ITADRIVER
    #define AUDIO_FRAME_SIZE 2000
int getAudioInput(int nAVCtx)
{
    int i;
    int nInput = -1;
    #ifdef CONFIG_GNASH_FLASH
    // select input stream
    for (i = 0; i < FLASH_AUDIO_INPUT; i++)
    {
        if (gFlashInfo.nInputUsing[i] == nAVCtx)
        {
            return i;
        }
    }
    #endif
    return nInput;
}

void setWaveInfo(AVCodecContext *avctx, ITE_WaveInfo *wavInfo, int nSize, int nInput)
{
    int           wavHeaderSize;
    unsigned long bufSize = 0;

    int           nResult;
    wavInfo->sampRate  = avctx->sample_rate;
    wavInfo->nChans    = avctx->channels;
    wavInfo->nDataSize = nSize;

    switch (avctx->codec_id)
    {
    case CODEC_ID_PCM_S16LE:
    case CODEC_ID_PCM_S16BE:
    case CODEC_ID_PCM_U16LE:
    case CODEC_ID_PCM_U16BE:
        wavInfo->format        = ITE_WAVE_FORMAT_PCM;
        wavInfo->bitsPerSample = 16;
        break;

    case CODEC_ID_PCM_S8:
    case CODEC_ID_PCM_U8:
        wavInfo->format        = ITE_WAVE_FORMAT_PCM;
        wavInfo->bitsPerSample = 8;
        break;

    case CODEC_ID_PCM_ALAW:
        wavInfo->format = ITE_WAVE_FORMAT_ALAW;
        break;

    case CODEC_ID_PCM_MULAW:
        wavInfo->format = ITE_WAVE_FORMAT_MULAW;
        break;

    case CODEC_ID_ADPCM_IMA_WAV:
        wavInfo->format = ITE_WAVE_FORMAT_DVI_ADPCM;
        break;

    case CODEC_ID_ADPCM_SWF:
        wavInfo->format = ITE_WAVE_FORMAT_SWF_ADPCM;
        break;

    default:
        wavInfo->format = ITE_WAVE_FORMAT_PCM;
        break;
    }
    av_log(avctx, AV_LOG_ERROR, "[Utils] setWaveInfo nChans %d sampRate %d \n", wavInfo->nChans, wavInfo->sampRate);
    iteAudioGenWaveDecodeHeader(wavInfo, &gWaveHeader[0]);

    if (wavInfo->format == 3)
        wavHeaderSize = 48;
    else
        wavHeaderSize = 44;

    #ifdef CONFIG_GNASH_FLASH
    do
    {
        nResult = iteAudioGetFlashAvailableBufferLength(nInput, &bufSize);
        usleep(1000);
        updateMessageQueue();
    } while (bufSize < wavHeaderSize);
    iteAudioWriteFlashStream(nInput, &gWaveHeader[0], wavHeaderSize);
    #else
    do
    {
        nResult = iteAudioGetAvailableBufferLength(ITE_AUDIO_OUTPUT_BUFFER, &bufSize);
        usleep(1000);
        updateMessageQueue();
    } while (bufSize < wavHeaderSize);
    iteAudioWriteStream(&gWaveHeader[0], wavHeaderSize);

    #endif

    #ifdef CONFIG_DUMP_AUDIO
    if (nInput == 0)
        fwrite(&gWaveHeader[0], 1, wavHeaderSize, fin);
    else if (nInput == 1)
        fwrite(&gWaveHeader[0], 1, wavHeaderSize, fin1);
    else if (nInput == 2)
        fwrite(&gWaveHeader[0], 1, wavHeaderSize, fin2);
    #endif
}

static void
createADTSHeader(
    unsigned char   *adtsHeader,
    int sampleFre,
    int channels,
    int framelength)
{
    unsigned int dwSynWord        = 0xFFF;               // 12bit
    unsigned int dwID             = 0;                   // 1bit
    unsigned int dwLayer          = 0;                   // 2bit    '00'
    unsigned int dwPotectAbsent   = 1;                   // 1bit
    unsigned int dwProfile        = 1;                   // 2bit
    unsigned int sampleFreIdx     = 3;                   // 4bit
    unsigned int dwPrivateBit     = 0;                   // 1bit
    //unsigned int  channels = 2;                          // 3bit
    unsigned int dwOriCopy        = 0;                   // 1bit
    unsigned int dwHome           = 0;                   // 1bit
    unsigned int dwCRIDBit        = 0;                   // 1bit
    unsigned int dwCRIDStart      = 0;                   // 1bit
    unsigned int dwFrameLength    = framelength + 7;     // 13bit
    unsigned int dwBufferFullness = 0x7FF;               // 11bit
    unsigned int dwNoRawData      = 0;                   // 2bit

    switch (sampleFre)
    {
    case 9600:
        sampleFreIdx = 0;
        break;

    case 88200:
        sampleFreIdx = 1;
        break;

    case 64000:
        sampleFreIdx = 2;
        break;

    case 48000:
        sampleFreIdx = 3;
        break;

    case 44100:
        sampleFreIdx = 4;
        break;

    case 32000:
        sampleFreIdx = 5;
        break;

    case 24000:
        sampleFreIdx = 6;
        break;

    case 22050:
        sampleFreIdx = 7;
        break;

    case 16000:
        sampleFreIdx = 8;
        break;

    case 12000:
        sampleFreIdx = 9;
        break;

    case 11025:
        sampleFreIdx = 10;
        break;

    case 8000:
        sampleFreIdx = 11;
        break;

    case 7350:
        sampleFreIdx = 12;
        break;
    }

    adtsHeader[0] = (unsigned char)(dwSynWord >> 4);
    adtsHeader[1] = (unsigned char)(((dwSynWord & 0xF) << 4) | (dwID << 3) | (dwLayer << 1) | (dwPotectAbsent));
    adtsHeader[2] = (unsigned char)((dwProfile << 6) | (sampleFreIdx << 2) | (dwPrivateBit << 1) | (channels >> 2));
    adtsHeader[3] = (unsigned char)(((channels & 0x3) << 6) | (dwOriCopy << 5) | (dwHome << 4) | (dwCRIDBit << 3) | (dwCRIDStart << 2) | (dwFrameLength >> 11));
    adtsHeader[4] = (unsigned char)((dwFrameLength & 0x7F8) >> 3);
    adtsHeader[5] = (unsigned char)(((dwFrameLength & 0x7 ) << 5) | (dwBufferFullness >> 6));
    adtsHeader[6] = (unsigned char)(((dwBufferFullness & 0x3F) << 2) | (dwNoRawData));
}

void updateMessageQueue()
{
    #if 1
    if(bPlayingBootVideo)
    {
        uint16_t nAudioPluginRegister = 0;

        // check which audio processor cmd to process
        nAudioPluginRegister = ithReadRegH(0x16e8);
        if (((nAudioPluginRegister & 0xc000) >> 14) == 2)
        {
            // do audio api
            smtkMainProcessorExecuteAudioPluginCmd(nAudioPluginRegister);
        }
    }
    #endif
}
#endif

void *av_fast_realloc(void *ptr, unsigned int *size, size_t min_size)
{
    if (min_size < *size)
        return ptr;

    min_size = FFMAX(17 * min_size / 16 + 32, min_size);

    ptr      = av_realloc(ptr, min_size);
    if (!ptr) //we could set this to the unmodified min_size but this is safer if the user lost the ptr and uses NULL now
        min_size = 0;

    *size = min_size;

    return ptr;
}

void av_fast_malloc(void *ptr, unsigned int *size, size_t min_size)
{
    void **p = ptr;
    if (min_size < *size)
        return;
    min_size = FFMAX(17 * min_size / 16 + 32, min_size);
    av_free(*p);
    *p       = av_malloc(min_size);
    if (!*p)
        min_size = 0;
    *size    = min_size;
}

/* encoder management */
static AVCodec *first_avcodec = NULL;

AVCodec *av_codec_next(AVCodec *c)
{
    if (c)
        return c->next;
    else
        return first_avcodec;
}

#if !FF_API_AVCODEC_INIT
static
#endif
void avcodec_init(void)
{
    static int initialized = 0;

    if (initialized != 0)
        return;
    initialized = 1;

    dsputil_static_init();
#ifdef CONFIG_ITADRIVER
    #ifdef CONFIG_GNASH_FLASH
    memset(&gFlashInfo, 0, sizeof(gFlashInfo));
    #else
    memset(&gWaveInfo, 0, sizeof(gWaveInfo));
    #endif
#endif
}

void avcodec_register(AVCodec *codec)
{
    AVCodec **p;
    avcodec_init();
    p           = &first_avcodec;
    while (*p != NULL)
        p = &(*p)->next;
    *p          = codec;
    codec->next = NULL;

    if (codec->init_static_data)
        codec->init_static_data(codec);
}

unsigned avcodec_get_edge_width(void)
{
    return EDGE_WIDTH;
}

void avcodec_set_dimensions(AVCodecContext *s, int width, int height)
{
    s->coded_width  = width;
    s->coded_height = height;
    s->width        = -((-width ) >> s->lowres);
    s->height       = -((-height) >> s->lowres);
}

#define INTERNAL_BUFFER_SIZE (32 + 1)

void avcodec_align_dimensions2(AVCodecContext *s, int *width, int *height,
                               int linesize_align[AV_NUM_DATA_POINTERS])
{
    int i;
    int w_align = 1;
    int h_align = 1;

    switch (s->pix_fmt)
    {
    case PIX_FMT_YUV420P:
    case PIX_FMT_YUYV422:
    case PIX_FMT_UYVY422:
    case PIX_FMT_YUV422P:
    case PIX_FMT_YUV440P:
    case PIX_FMT_YUV444P:
    case PIX_FMT_GBRP:
    case PIX_FMT_GRAY8:
    case PIX_FMT_GRAY16BE:
    case PIX_FMT_GRAY16LE:
    case PIX_FMT_YUVJ420P:
    case PIX_FMT_YUVJ422P:
    case PIX_FMT_YUVJ440P:
    case PIX_FMT_YUVJ444P:
    case PIX_FMT_YUVA420P:
    case PIX_FMT_YUV420P9LE:
    case PIX_FMT_YUV420P9BE:
    case PIX_FMT_YUV420P10LE:
    case PIX_FMT_YUV420P10BE:
    case PIX_FMT_YUV422P9LE:
    case PIX_FMT_YUV422P9BE:
    case PIX_FMT_YUV422P10LE:
    case PIX_FMT_YUV422P10BE:
    case PIX_FMT_YUV444P9LE:
    case PIX_FMT_YUV444P9BE:
    case PIX_FMT_YUV444P10LE:
    case PIX_FMT_YUV444P10BE:
    case PIX_FMT_GBRP9LE:
    case PIX_FMT_GBRP9BE:
    case PIX_FMT_GBRP10LE:
    case PIX_FMT_GBRP10BE:
        w_align = 16;     //FIXME assume 16 pixel per macroblock
        h_align = 16 * 2; // interlaced needs 2 macroblocks height
        break;

    case PIX_FMT_YUV411P:
    case PIX_FMT_UYYVYY411:
        w_align = 32;
        h_align = 8;
        break;

    case PIX_FMT_YUV410P:
        if (s->codec_id == CODEC_ID_SVQ1)
        {
            w_align = 64;
            h_align = 64;
        }

    case PIX_FMT_RGB555:
        if (s->codec_id == CODEC_ID_RPZA)
        {
            w_align = 4;
            h_align = 4;
        }

    case PIX_FMT_PAL8:
    case PIX_FMT_BGR8:
    case PIX_FMT_RGB8:
        if (s->codec_id == CODEC_ID_SMC)
        {
            w_align = 4;
            h_align = 4;
        }
        break;

    case PIX_FMT_BGR24:
        if ((s->codec_id == CODEC_ID_MSZH) || (s->codec_id == CODEC_ID_ZLIB))
        {
            w_align = 4;
            h_align = 4;
        }
        break;

    default:
        w_align = 1;
        h_align = 1;
        break;
    }

    if (s->codec_id == CODEC_ID_IFF_ILBM || s->codec_id == CODEC_ID_IFF_BYTERUN1)
    {
        w_align = FFMAX(w_align, 8);
    }

    *width  = FFALIGN(*width, w_align);
    *height = FFALIGN(*height, h_align);
    if (s->codec_id == CODEC_ID_H264 || s->lowres)
        *height += 2; // some of the optimized chroma MC reads one line too much
                      // which is also done in mpeg decoders with lowres > 0

    for (i = 0; i < AV_NUM_DATA_POINTERS; i++)
        linesize_align[i] = STRIDE_ALIGN;
    //STRIDE_ALIGN is 8 for SSE* but this does not work for SVQ1 chroma planes
    //we could change STRIDE_ALIGN to 16 for x86/sse but it would increase the
    //picture size unneccessarily in some cases. The solution here is not
    //pretty and better ideas are welcome!
#if HAVE_MMX
    if (s->codec_id == CODEC_ID_SVQ1 || s->codec_id == CODEC_ID_VP5 ||
        s->codec_id == CODEC_ID_VP6 || s->codec_id == CODEC_ID_VP6F ||
        s->codec_id == CODEC_ID_VP6A || s->codec_id == CODEC_ID_DIRAC)
    {
        for (i = 0; i < AV_NUM_DATA_POINTERS; i++)
            linesize_align[i] = 16;
    }
#endif
}

void avcodec_align_dimensions(AVCodecContext *s, int *width, int *height)
{
    int chroma_shift = av_pix_fmt_descriptors[s->pix_fmt].log2_chroma_w;
    int linesize_align[AV_NUM_DATA_POINTERS];
    int align;
    avcodec_align_dimensions2(s, width, height, linesize_align);
    align               = FFMAX(linesize_align[0], linesize_align[3]);
    linesize_align[1] <<= chroma_shift;
    linesize_align[2] <<= chroma_shift;
    align               = FFMAX3(align, linesize_align[1], linesize_align[2]);
    *width              = FFALIGN(*width, align);
}

void ff_init_buffer_info(AVCodecContext *s, AVFrame *pic)
{
    if (s->pkt)
    {
        pic->pkt_pts = s->pkt->pts;
        pic->pkt_pos = s->pkt->pos;
    }
    else
    {
        pic->pkt_pts = AV_NOPTS_VALUE;
        pic->pkt_pos = -1;
    }
    pic->reordered_opaque    = s->reordered_opaque;
    pic->sample_aspect_ratio = s->sample_aspect_ratio;
    pic->width               = s->width;
    pic->height              = s->height;
    pic->format              = s->pix_fmt;
}

static int audio_get_buffer(AVCodecContext *avctx, AVFrame *frame)
{
    AVCodecInternal *avci = avctx->internal;
    InternalBuffer  *buf;
    int             buf_size, ret, i, needs_extended_data;

    buf_size = av_samples_get_buffer_size(NULL, avctx->channels,
                                          frame->nb_samples, avctx->sample_fmt,
                                          32);
    if (buf_size < 0)
        return AVERROR(EINVAL);

    needs_extended_data = av_sample_fmt_is_planar(avctx->sample_fmt) &&
                          avctx->channels > AV_NUM_DATA_POINTERS;

    /* allocate InternalBuffer if needed */
    if (!avci->buffer)
    {
        avci->buffer = av_mallocz(sizeof(InternalBuffer));
        if (!avci->buffer)
            return AVERROR(ENOMEM);
    }
    buf = avci->buffer;

    /* if there is a previously-used internal buffer, check its size and
       channel count to see if we can reuse it */
    if (buf->extended_data)
    {
        /* if current buffer is too small, free it */
        if (buf->extended_data[0] && buf_size > buf->audio_data_size)
        {
            av_free(buf->extended_data[0]);
            if (buf->extended_data != buf->data)
                av_free(&buf->extended_data);
            buf->extended_data = NULL;
            buf->data[0]       = NULL;
        }
        /* if number of channels has changed, reset and/or free extended data
           pointers but leave data buffer in buf->data[0] for reuse */
        if (buf->nb_channels != avctx->channels)
        {
            if (buf->extended_data != buf->data)
                av_free(buf->extended_data);
            buf->extended_data = NULL;
        }
    }

    /* if there is no previous buffer or the previous buffer cannot be used
       as-is, allocate a new buffer and/or rearrange the channel pointers */
    if (!buf->extended_data)
    {
        /* if the channel pointers will fit, just set extended_data to data,
           otherwise allocate the extended_data channel pointers */
        if (needs_extended_data)
        {
            buf->extended_data = av_mallocz(avctx->channels *
                                            sizeof(*buf->extended_data));
            if (!buf->extended_data)
                return AVERROR(ENOMEM);
        }
        else
        {
            buf->extended_data = buf->data;
        }

        /* if there is a previous buffer and it is large enough, reuse it and
           just fill-in new channel pointers and linesize, otherwise allocate
           a new buffer */
        if (buf->extended_data[0])
        {
            ret = av_samples_fill_arrays(buf->extended_data, &buf->linesize[0],
                                         buf->extended_data[0], avctx->channels,
                                         frame->nb_samples, avctx->sample_fmt,
                                         32);
        }
        else
        {
            ret = av_samples_alloc(buf->extended_data, &buf->linesize[0],
                                   avctx->channels, frame->nb_samples,
                                   avctx->sample_fmt, 32);
        }
        if (ret)
            return ret;

        /* if data was not used for extended_data, we need to copy as many of
           the extended_data channel pointers as will fit */
        if (needs_extended_data)
        {
            for (i = 0; i < AV_NUM_DATA_POINTERS; i++)
                buf->data[i] = buf->extended_data[i];
        }
        buf->audio_data_size = buf_size;
        buf->nb_channels     = avctx->channels;
    }

    /* copy InternalBuffer info to the AVFrame */
    frame->type          = FF_BUFFER_TYPE_INTERNAL;
    frame->extended_data = buf->extended_data;
    frame->linesize[0]   = buf->linesize[0];
    memcpy(frame->data, buf->data, sizeof(frame->data));

    if (avctx->pkt)
    {
        frame->pkt_pts = avctx->pkt->pts;
        frame->pkt_pos = avctx->pkt->pos;
    }
    else
    {
        frame->pkt_pts = AV_NOPTS_VALUE;
        frame->pkt_pos = -1;
    }

    frame->reordered_opaque = avctx->reordered_opaque;

    if (avctx->debug & FF_DEBUG_BUFFERS)
        av_log(avctx, AV_LOG_DEBUG, "default_get_buffer called on frame %p, "
               "internal audio buffer used\n", frame);

    return 0;
}

static int video_get_buffer(AVCodecContext *s, AVFrame *pic)
{
    int             i;
    int             w     = s->width;
    int             h     = s->height;
    InternalBuffer  *buf;
    int             *picture_number;
    AVCodecInternal *avci = s->internal;

    if (pic->data[0] != NULL)
    {
        av_log(s, AV_LOG_ERROR, "pic->data[0]!=NULL in avcodec_default_get_buffer\n");
        return -1;
    }
    if (avci->buffer_count >= INTERNAL_BUFFER_SIZE)
    {
        av_log(s, AV_LOG_ERROR, "buffer_count overflow (missing release_buffer?)\n");
        return -1;
    }

    if (av_image_check_size(w, h, 0, s))
        return -1;

    if (!avci->buffer)
    {
        avci->buffer = av_mallocz((INTERNAL_BUFFER_SIZE + 1) *
                                  sizeof(InternalBuffer));
    }

    buf            = &avci->buffer[avci->buffer_count];
    picture_number = &(avci->buffer[INTERNAL_BUFFER_SIZE]).last_pic_num; //FIXME ugly hack
    (*picture_number)++;

    if (buf->base[0] && (buf->width != w || buf->height != h || buf->pix_fmt != s->pix_fmt))
    {
        if (s->active_thread_type & FF_THREAD_FRAME)
        {
            av_log_missing_feature(s, "Width/height changing with frame threads is", 0);
            return -1;
        }

        for (i = 0; i < AV_NUM_DATA_POINTERS; i++)
        {
            av_freep(&buf->base[i]);
            buf->data[i] = NULL;
        }
    }

    if (buf->base[0])
    {
        pic->age          = *picture_number - buf->last_pic_num;
        buf->last_pic_num = *picture_number;
    }
    else
    {
        int       h_chroma_shift, v_chroma_shift;
        int       size[4] = {0};
        int       tmpsize;
        int       unaligned;
        AVPicture picture;
        int       stride_align[AV_NUM_DATA_POINTERS];
        const int pixel_size = av_pix_fmt_descriptors[s->pix_fmt].comp[0].step_minus1 + 1;

        avcodec_get_chroma_sub_sample(s->pix_fmt, &h_chroma_shift, &v_chroma_shift);

        avcodec_align_dimensions2(s, &w, &h, stride_align);

        if (!(s->flags & CODEC_FLAG_EMU_EDGE))
        {
            w += EDGE_WIDTH * 2;
            h += EDGE_WIDTH * 2;
        }

        do
        {
            // NOTE: do not align linesizes individually, this breaks e.g. assumptions
            // that linesize[0] == 2*linesize[1] in the MPEG-encoder for 4:2:2
            av_image_fill_linesizes(picture.linesize, s->pix_fmt, w);
            // increase alignment of w for next try (rhs gives the lowest bit set in w)
            w        += w & ~(w - 1);

            unaligned = 0;
            for (i = 0; i < 4; i++)
            {
                unaligned |= picture.linesize[i] % stride_align[i];
            }
        } while (unaligned);

        tmpsize = av_image_fill_pointers(picture.data, s->pix_fmt, h, NULL, picture.linesize);
        if (tmpsize < 0)
            return -1;

        for (i = 0; i < 3 && picture.data[i + 1]; i++)
            size[i] = picture.data[i + 1] - picture.data[i];
        size[i]           = tmpsize - (picture.data[i] - picture.data[0]);

        buf->last_pic_num = -256 * 256 * 256 * 64;
        memset(buf->base, 0, sizeof(buf->base));
        memset(buf->data, 0, sizeof(buf->data));

        for (i = 0; i < 4 && size[i]; i++)
        {
            const int h_shift = i == 0 ? 0 : h_chroma_shift;
            const int v_shift = i == 0 ? 0 : v_chroma_shift;

            buf->linesize[i] = picture.linesize[i];

            buf->base[i]     = av_malloc(size[i] + 16); //FIXME 16
            if (buf->base[i] == NULL)
                return -1;
            memset(buf->base[i], 128, size[i]);

            // no edge if EDGE EMU or not planar YUV
            if ((s->flags & CODEC_FLAG_EMU_EDGE) || !size[2])
                buf->data[i] = buf->base[i];
            else
                buf->data[i] = buf->base[i] + FFALIGN((buf->linesize[i] * EDGE_WIDTH >> v_shift) + (pixel_size * EDGE_WIDTH >> h_shift), stride_align[i]);
        }
        for (; i < AV_NUM_DATA_POINTERS; i++)
        {
            buf->base[i]     = buf->data[i] = NULL;
            buf->linesize[i] = 0;
        }
        if (size[1] && !size[2])
            ff_set_systematic_pal2((uint32_t *)buf->data[1], s->pix_fmt);
        buf->width   = s->width;
        buf->height  = s->height;
        buf->pix_fmt = s->pix_fmt;
        pic->age     = 256 * 256 * 256 * 64;
    }
    pic->type = FF_BUFFER_TYPE_INTERNAL;

    for (i = 0; i < AV_NUM_DATA_POINTERS; i++)
    {
        pic->base[i]     = buf->base[i];
        pic->data[i]     = buf->data[i];
        pic->linesize[i] = buf->linesize[i];
    }
    pic->extended_data = pic->data;
    avci->buffer_count++;

    if (s->pkt)
    {
        pic->pkt_pts = s->pkt->pts;
        pic->pkt_pos = s->pkt->pos;
    }
    else
    {
        pic->pkt_pts = AV_NOPTS_VALUE;
        pic->pkt_pos = -1;
    }
    pic->reordered_opaque    = s->reordered_opaque;
    pic->sample_aspect_ratio = s->sample_aspect_ratio;
    pic->width               = s->width;
    pic->height              = s->height;
    pic->format              = s->pix_fmt;

    if (s->debug & FF_DEBUG_BUFFERS)
        av_log(s, AV_LOG_DEBUG, "default_get_buffer called on pic %p, %d "
               "buffers used\n", pic, avci->buffer_count);

    return 0;
}

int our_get_buffer(struct AVCodecContext *c, AVFrame *pic)
{
    int     ret  = avcodec_default_get_buffer(c, pic);
    int64_t *pts = av_malloc(sizeof(int64_t));
    *pts         = global_video_pkt_pts;
    pic->opaque1 = pts;    // pts saved here
    return ret;
}
void our_release_buffer(struct AVCodecContext *c, AVFrame *pic)
{
    if (pic)
        av_freep(&pic->opaque1);
    avcodec_default_release_buffer(c, pic);
}

int avcodec_default_get_buffer(AVCodecContext *avctx, AVFrame *frame)
{
    switch (avctx->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        return video_get_buffer(avctx, frame);

    case AVMEDIA_TYPE_AUDIO:
        return audio_get_buffer(avctx, frame);

    default:
        return -1;
    }
}

void avcodec_default_release_buffer(AVCodecContext *s, AVFrame *pic)
{
    int             i;
    InternalBuffer  *buf, *last;
    AVCodecInternal *avci = s->internal;

    assert(s->codec_type == AVMEDIA_TYPE_VIDEO);

    assert(pic->type == FF_BUFFER_TYPE_INTERNAL);
    assert(avci->buffer_count);

    if (avci->buffer)
    {
        buf = NULL;                              /* avoids warning */
        for (i = 0; i < avci->buffer_count; i++) //just 3-5 checks so is not worth to optimize
        {
            buf = &avci->buffer[i];
            if (buf->data[0] == pic->data[0])
                break;
        }
        assert(i < avci->buffer_count);
        avci->buffer_count--;
        last = &avci->buffer[avci->buffer_count];

        FFSWAP(InternalBuffer, *buf, *last);
    }

    for (i = 0; i < AV_NUM_DATA_POINTERS; i++)
    {
        pic->data[i] = NULL;
        //        pic->base[i]=NULL;
    }
    //printf("R%X\n", pic->opaque);

    if (s->debug & FF_DEBUG_BUFFERS)
        av_log(s, AV_LOG_DEBUG, "default_release_buffer called on pic %p, %d "
               "buffers used\n", pic, avci->buffer_count);
}

int avcodec_default_reget_buffer(AVCodecContext *s, AVFrame *pic)
{
    AVFrame temp_pic;
    int     i;

    assert(s->codec_type == AVMEDIA_TYPE_VIDEO);

    /* If no picture return a new buffer */
    if (pic->data[0] == NULL)
    {
        /* We will copy from buffer, so must be readable */
        pic->buffer_hints |= FF_BUFFER_HINTS_READABLE;
        return s->get_buffer(s, pic);
    }

    /* If internal buffer type return the same buffer */
    if (pic->type == FF_BUFFER_TYPE_INTERNAL)
    {
        if (s->pkt)
            pic->pkt_pts = s->pkt->pts;
        else
            pic->pkt_pts = AV_NOPTS_VALUE;
        pic->reordered_opaque = s->reordered_opaque;
        return 0;
    }

    /*
     * Not internal type and reget_buffer not overridden, emulate cr buffer
     */
    temp_pic    = *pic;
    for (i = 0; i < AV_NUM_DATA_POINTERS; i++)
        pic->data[i] = pic->base[i] = NULL;
    pic->opaque = NULL;
    /* Allocate new frame */
    if (s->get_buffer(s, pic))
        return -1;
    /* Copy image data from old buffer to new buffer */
    av_picture_copy((AVPicture *)pic, (AVPicture *)&temp_pic, s->pix_fmt, s->width,
                    s->height);
    s->release_buffer(s, &temp_pic); // Release old frame
    return 0;
}

int avcodec_default_execute(AVCodecContext *c, int (*func)(AVCodecContext *c2, void *arg2), void *arg, int *ret, int count, int size)
{
    int i;

    for (i = 0; i < count; i++)
    {
        int r = func(c, (char *)arg + i * size);
        if (ret)
            ret[i] = r;
    }
    return 0;
}

int avcodec_default_execute2(AVCodecContext *c, int (*func)(AVCodecContext *c2, void *arg2, int jobnr, int threadnr), void *arg, int *ret, int count)
{
    int i;

    for (i = 0; i < count; i++)
    {
        int r = func(c, arg, i, 0);
        if (ret)
            ret[i] = r;
    }
    return 0;
}

enum PixelFormat avcodec_default_get_format(struct AVCodecContext *s, const enum PixelFormat *fmt)
{
    while (*fmt != PIX_FMT_NONE && ff_is_hwaccel_pix_fmt(*fmt))
        ++fmt;
    return fmt[0];
}

void avcodec_get_frame_defaults(AVFrame *pic)
{
    memset(pic, 0, sizeof(AVFrame));

    pic->pts                     = pic->best_effort_timestamp = AV_NOPTS_VALUE;
    pic->pkt_pos                 = -1;
    pic->key_frame               = 1;
#if defined(WIN32)
    pic->sample_aspect_ratio.num = 0;
    pic->sample_aspect_ratio.den = 1;
#else
    pic->sample_aspect_ratio     = (AVRational){0, 1};
#endif
    pic->format                  = -1; /* unknown */
}

AVFrame *avcodec_alloc_frame(void)
{
    AVFrame *pic = av_malloc(sizeof(AVFrame));

    if (pic == NULL)
        return NULL;

    avcodec_get_frame_defaults(pic);

    return pic;
}

static void avcodec_get_subtitle_defaults(AVSubtitle *sub)
{
    memset(sub, 0, sizeof(*sub));
    sub->pts = AV_NOPTS_VALUE;
}

#if FF_API_AVCODEC_OPEN
int attribute_align_arg avcodec_open(AVCodecContext *avctx, AVCodec *codec)
{
    return avcodec_open2(avctx, codec, NULL);
}
#endif

int attribute_align_arg avcodec_open2(AVCodecContext *avctx, AVCodec *codec, AVDictionary **options)
{
    int          ret          = 0;
    AVDictionary *tmp         = NULL;
    int          nAudioEngine = 0;
    int          i            = 0;

    if (options)
        av_dict_copy(&tmp, *options, 0);

    /* If there is a user-supplied mutex locking routine, call it. */
    if (ff_lockmgr_cb)
    {
        if ((*ff_lockmgr_cb)(&codec_mutex, AV_LOCK_OBTAIN))
            return -1;
    }

    entangled_thread_counter++;
    if (entangled_thread_counter != 1)
    {
        av_log(avctx, AV_LOG_ERROR, "insufficient thread locking around avcodec_open/close()\n");
        ret = -1;
        goto end;
    }

    if (avctx->codec || !codec)
    {
        ret = AVERROR(EINVAL);
        goto end;
    }

    avctx->internal = av_mallocz(sizeof(AVCodecInternal));
    if (!avctx->internal)
    {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    if (codec->priv_data_size > 0)
    {
        if (!avctx->priv_data)
        {
            avctx->priv_data = av_mallocz(codec->priv_data_size);
            if (!avctx->priv_data)
            {
                ret = AVERROR(ENOMEM);
                goto end;
            }
            if (codec->priv_class)
            {
                *(AVClass **)avctx->priv_data = codec->priv_class;
                av_opt_set_defaults(avctx->priv_data);
            }
        }
        if (codec->priv_class && (ret = av_opt_set_dict(avctx->priv_data, &tmp)) < 0)
            goto free_and_end;
    }
    else
    {
        avctx->priv_data = NULL;
    }
    if ((ret = av_opt_set_dict(avctx, &tmp)) < 0)
        goto free_and_end;

    //We only call avcodec_set_dimensions() for non h264 codecs so as not to overwrite previously setup dimensions
    if (!( avctx->coded_width && avctx->coded_height && avctx->width && avctx->height && avctx->codec_id == CODEC_ID_H264))
    {
        if (avctx->coded_width && avctx->coded_height)
            avcodec_set_dimensions(avctx, avctx->coded_width, avctx->coded_height);
        else if (avctx->width && avctx->height)
            avcodec_set_dimensions(avctx, avctx->width, avctx->height);
    }

    if ((avctx->coded_width || avctx->coded_height || avctx->width || avctx->height)
        && (  av_image_check_size(avctx->coded_width, avctx->coded_height, 0, avctx) < 0
              || av_image_check_size(avctx->width,       avctx->height,       0, avctx) < 0))
    {
        av_log(avctx, AV_LOG_WARNING, "ignoring invalid width/height values\n");
        avcodec_set_dimensions(avctx, 0, 0);
    }

    /* if the decoder init function was already called previously,
       free the already allocated subtitle_header before overwriting it */
    if (codec->decode)
        av_freep(&avctx->subtitle_header);

#define SANE_NB_CHANNELS 128U
    if (avctx->channels > SANE_NB_CHANNELS)
    {
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }

    avctx->codec = codec;
    if ((avctx->codec_type == AVMEDIA_TYPE_UNKNOWN || avctx->codec_type == codec->type) &&
        avctx->codec_id == CODEC_ID_NONE)
    {
        avctx->codec_type = codec->type;
        avctx->codec_id   = codec->id;
    }
    if (avctx->codec_id != codec->id || (avctx->codec_type != codec->type
                                         && avctx->codec_type != AVMEDIA_TYPE_ATTACHMENT))
    {
        av_log(avctx, AV_LOG_ERROR, "codec type or id mismatches\n");
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }
#ifdef CONFIG_ITADRIVER
    #ifdef CONFIG_DUMP_AUDIO
    if (fin == NULL && (fin = fopen(inFileName, "wb")) == NULL)
    {
        av_log(avctx, AV_LOG_WARNING, "Can not open file '%s' \n", inFileName);
    }
    if (fin1 == NULL && (fin1 = fopen(inFileName1, "wb")) == NULL)
    {
        av_log(avctx, AV_LOG_WARNING, "Can not open file '%s' \n", inFileName1);
    }
    if (fin2 == NULL && (fin2 = fopen(inFileName2, "wb")) == NULL)
    {
        av_log(avctx, AV_LOG_WARNING, "Can not open file '%s' \n", inFileName2);
    }
    #endif
    if (avctx->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        av_log(avctx, AV_LOG_WARNING, "iteAudioOpenEngine 0x%x codec_id %d \n", avctx, avctx->codec_id);
    #ifdef CONFIG_GNASH_FLASH
        switch (avctx->codec_id)
        {
        case CODEC_ID_MP2:
        case CODEC_ID_MP3:
            nAudioEngine            = ITE_MP3_DECODE;
            gFlashInfo.nFlashEnable = 1;
            break;

        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
        case CODEC_ID_PCM_S8:
        case CODEC_ID_PCM_U8:
        case CODEC_ID_PCM_ALAW:
        case CODEC_ID_PCM_MULAW:
        case CODEC_ID_ADPCM_IMA_WAV:
        case CODEC_ID_ADPCM_SWF:
            nAudioEngine            = ITE_WAV_DECODE;
            gFlashInfo.nFlashEnable = 1;
            break;

        case CODEC_ID_AAC:
            nAudioEngine = ITE_AAC_DECODE;
            break;

        case CODEC_ID_AC3:
        case CODEC_ID_EAC3:
            nAudioEngine = ITE_AC3_DECODE;
            break;

        case CODEC_ID_WMAV2:
            nAudioEngine = ITE_WMA_DECODE;
            break;

        case CODEC_ID_AMR_NB:
            nAudioEngine = ITE_AMR_CODEC;

        default:
            break;
        }
        av_log(avctx, AV_LOG_ERROR, "codec type %d %d \n", avctx->codec_id, nAudioEngine);

        if (gFlashInfo.nAudioEnable == 0)
        {
            iteAudioOpenEngine(nAudioEngine);
            gFlashInfo.nAudioEnable = 1;
        }

        // if mp3/wav, choose input stream
        if (gFlashInfo.nFlashEnable == 1)
        {
            // select input stream
            for (i = 0; i < FLASH_AUDIO_INPUT; i++)
            {
                if (gFlashInfo.nInputUsing[i] == 0)
                {
                    gFlashInfo.nInputUsing[i]  = avctx;
                    gFlashInfo.nAudioFormat[i] = nAudioEngine;
                    iteAudioSetFlashInputStatus(i, nAudioEngine, 1);
                    break;
                }
            }
            if (i > FLASH_AUDIO_INPUT)
            {
                av_log(avctx, AV_LOG_WARNING, "audio input stream is full 0x%x \n", avctx);
            }
            if (nAudioEngine == ITE_WAV_DECODE)
            {
                switch (avctx->codec_id)
                {
                case CODEC_ID_PCM_S16LE:
                    gFlashInfo.wavInfo[i].format        = 0;
                    gFlashInfo.wavInfo[i].bitsPerSample = 16;
                    break;

                case CODEC_ID_PCM_S8:
                    gFlashInfo.wavInfo[i].format        = 0;
                    gFlashInfo.wavInfo[i].bitsPerSample = 8;
                    break;

                case CODEC_ID_PCM_ALAW:
                    gFlashInfo.wavInfo[i].format        = 1;
                    gFlashInfo.wavInfo[i].bitsPerSample = 8;
                    break;

                case CODEC_ID_PCM_MULAW:
                    gFlashInfo.wavInfo[i].format        = 2;
                    gFlashInfo.wavInfo[i].bitsPerSample = 8;
                    break;

                case CODEC_ID_ADPCM_IMA_WAV:
                    gFlashInfo.wavInfo[i].format        = 3;
                    gFlashInfo.wavInfo[i].bitsPerSample = 4;
                    break;

                case CODEC_ID_ADPCM_SWF:
                    gFlashInfo.wavInfo[i].format        = 4;
                    gFlashInfo.wavInfo[i].bitsPerSample = 4;
                    break;

                default:
                    gFlashInfo.wavInfo[i].format        = 0;
                    gFlashInfo.wavInfo[i].bitsPerSample = 16;
                    break;
                } //switch (avctx->codec_id)
            }     //if (nAudioEngine == ITE_WAV_DECODE)
        }
    #else
        switch (avctx->codec_id)
        {
        case CODEC_ID_MP2:
        case CODEC_ID_MP3:
            nAudioEngine = ITE_MP3_DECODE;
            break;

        case CODEC_ID_PCM_S16LE:
        case CODEC_ID_PCM_S16BE:
        case CODEC_ID_PCM_U16LE:
        case CODEC_ID_PCM_U16BE:
        case CODEC_ID_PCM_S8:
        case CODEC_ID_PCM_U8:
        case CODEC_ID_PCM_ALAW:
        case CODEC_ID_PCM_MULAW:
        case CODEC_ID_ADPCM_IMA_WAV:
            // case CODEC_ID_ADPCM_SWF:
            nAudioEngine = ITE_WAV_DECODE;
            break;

        case CODEC_ID_AAC:
            nAudioEngine = ITE_AAC_DECODE;
            break;

        case CODEC_ID_AC3:
        case CODEC_ID_EAC3:
            nAudioEngine = ITE_AC3_DECODE;
            break;

        case CODEC_ID_WMAV2:
            nAudioEngine = ITE_WMA_DECODE;
            break;

        case CODEC_ID_AMR_NB:
            nAudioEngine = ITE_AMR_CODEC;

        default:
            break;
        }
        av_log(avctx, AV_LOG_ERROR, "codec type %d %d \n", avctx->codec_id, nAudioEngine);

        //iteAudioStop();

        iteAudioOpenEngine(nAudioEngine);

        if (nAudioEngine == ITE_WAV_DECODE)
        {
            switch (avctx->codec_id)
            {
            case CODEC_ID_PCM_S16LE:
                gWaveInfo.format        = 0;
                gWaveInfo.bitsPerSample = 16;
                break;

            case CODEC_ID_PCM_S8:
                gWaveInfo.format        = 0;
                gWaveInfo.bitsPerSample = 8;
                break;

            case CODEC_ID_PCM_ALAW:
                gWaveInfo.format        = 1;
                gWaveInfo.bitsPerSample = 8;
                break;

            case CODEC_ID_PCM_MULAW:
                gWaveInfo.format        = 2;
                gWaveInfo.bitsPerSample = 8;
                break;

            case CODEC_ID_ADPCM_IMA_WAV:
                gWaveInfo.format        = 3;
                gWaveInfo.bitsPerSample = 4;
                break;

            //case CODEC_ID_ADPCM_SWF:
            //   gWaveInfo.format = 4;
            //    gWaveInfo.bitsPerSample = 4;
            //    break;
            default:
                gWaveInfo.format        = 0;
                gWaveInfo.bitsPerSample = 16;
                break;
            }//switch (avctx->codec_id)
             // wave format, send wave header to driver
            #ifdef SWITCH_WORKAROUND
            gWavEnable = 1;
            #else
            setWaveInfo(avctx, &gWaveInfo, 0, 0);
            #endif

        }//if (nAudioEngine == ITE_WAV_DECODE)

    #endif
    }
#endif
    avctx->frame_number = 0;
#if FF_API_ER

    av_log(avctx, AV_LOG_DEBUG, "err{or,}_recognition separate: %d; %d\n",
           avctx->error_recognition, avctx->err_recognition);
    switch (avctx->error_recognition)
    {
    case FF_ER_EXPLODE: avctx->err_recognition |= AV_EF_EXPLODE | AV_EF_COMPLIANT | AV_EF_CAREFUL;
        break;

    case FF_ER_VERY_AGGRESSIVE:
    case FF_ER_AGGRESSIVE: avctx->err_recognition |= AV_EF_AGGRESSIVE;
    case FF_ER_COMPLIANT: avctx->err_recognition  |= AV_EF_COMPLIANT;
    case FF_ER_CAREFUL: avctx->err_recognition    |= AV_EF_CAREFUL;
    }

    av_log(avctx, AV_LOG_DEBUG, "err{or,}_recognition combined: %d; %d\n",
           avctx->error_recognition, avctx->err_recognition);
#endif

    if (!HAVE_THREADS)
        av_log(avctx, AV_LOG_WARNING, "Warning: not compiled with thread support, using thread emulation\n");

    if (HAVE_THREADS && !avctx->thread_opaque)
    {
        ret = ff_thread_init(avctx);
        if (ret < 0)
        {
            goto free_and_end;
        }
    }

    if (avctx->codec->max_lowres < avctx->lowres || avctx->lowres < 0)
    {
        av_log(avctx, AV_LOG_ERROR, "The maximum value for lowres supported by the decoder is %d\n",
               avctx->codec->max_lowres);
        ret = AVERROR(EINVAL);
        goto free_and_end;
    }
    if (avctx->codec->encode)
    {
        int i;
        if (avctx->codec->sample_fmts)
        {
            for (i = 0; avctx->codec->sample_fmts[i] != AV_SAMPLE_FMT_NONE; i++)
                if (avctx->sample_fmt == avctx->codec->sample_fmts[i])
                    break;
            if (avctx->codec->sample_fmts[i] == AV_SAMPLE_FMT_NONE)
            {
                av_log(avctx, AV_LOG_ERROR, "Specified sample_fmt is not supported.\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (avctx->codec->supported_samplerates)
        {
            for (i = 0; avctx->codec->supported_samplerates[i] != 0; i++)
                if (avctx->sample_rate == avctx->codec->supported_samplerates[i])
                    break;
            if (avctx->codec->supported_samplerates[i] == 0)
            {
                av_log(avctx, AV_LOG_ERROR, "Specified sample_rate is not supported\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        if (avctx->codec->channel_layouts)
        {
            if (!avctx->channel_layout)
            {
                av_log(avctx, AV_LOG_WARNING, "channel_layout not specified\n");
            }
            else
            {
                for (i = 0; avctx->codec->channel_layouts[i] != 0; i++)
                    if (avctx->channel_layout == avctx->codec->channel_layouts[i])
                        break;
                if (avctx->codec->channel_layouts[i] == 0)
                {
                    av_log(avctx, AV_LOG_ERROR, "Specified channel_layout is not supported\n");
                    ret = AVERROR(EINVAL);
                    goto free_and_end;
                }
            }
        }
        if (avctx->channel_layout && avctx->channels)
        {
            if (av_get_channel_layout_nb_channels(avctx->channel_layout) != avctx->channels)
            {
                av_log(avctx, AV_LOG_ERROR, "channel layout does not match number of channels\n");
                ret = AVERROR(EINVAL);
                goto free_and_end;
            }
        }
        else if (avctx->channel_layout)
        {
            avctx->channels = av_get_channel_layout_nb_channels(avctx->channel_layout);
        }
    }

    avctx->pts_correction_num_faulty_pts     =
        avctx->pts_correction_num_faulty_dts = 0;
    avctx->pts_correction_last_pts           =
        avctx->pts_correction_last_dts       = INT64_MIN;

    if (avctx->codec->init && !(avctx->active_thread_type & FF_THREAD_FRAME))
    {
        ret = avctx->codec->init(avctx);
        if (ret < 0)
        {
            goto free_and_end;
        }
    }

    ret = 0;
end:
    entangled_thread_counter--;

    /* Release any user-supplied mutex. */
    if (ff_lockmgr_cb)
    {
        (*ff_lockmgr_cb)(&codec_mutex, AV_LOCK_RELEASE);
    }
    if (options)
    {
        av_dict_free(options);
        *options = tmp;
    }

    return ret;
free_and_end:
    av_dict_free(&tmp);
    av_freep(&avctx->priv_data);
    av_freep(&avctx->internal);
    avctx->codec = NULL;
    goto end;
}

int attribute_align_arg avcodec_encode_audio(AVCodecContext *avctx, uint8_t *buf, int buf_size,
                                             const short *samples)
{
    if (buf_size < FF_MIN_BUFFER_SIZE && 0)
    {
        av_log(avctx, AV_LOG_ERROR, "buffer smaller than minimum size\n");
        return -1;
    }
    if ((avctx->codec->capabilities & CODEC_CAP_DELAY) || samples)
    {
        int ret = avctx->codec->encode(avctx, buf, buf_size, samples);
        avctx->frame_number++;
        return ret;
    }
    else
        return 0;
}

int attribute_align_arg avcodec_encode_video(AVCodecContext *avctx, uint8_t *buf, int buf_size,
                                             const AVFrame *pict)
{
    if (buf_size < FF_MIN_BUFFER_SIZE)
    {
        av_log(avctx, AV_LOG_ERROR, "buffer smaller than minimum size\n");
        return -1;
    }
    if (av_image_check_size(avctx->width, avctx->height, 0, avctx))
        return -1;
    if ((avctx->codec->capabilities & CODEC_CAP_DELAY) || pict)
    {
        int ret = avctx->codec->encode(avctx, buf, buf_size, pict);
        avctx->frame_number++;
        emms_c(); //needed to avoid an emms_c() call before every return;

        return ret;
    }
    else
        return 0;
}

int avcodec_encode_subtitle(AVCodecContext *avctx, uint8_t *buf, int buf_size,
                            const AVSubtitle *sub)
{
    int ret;
    if (sub->start_display_time)
    {
        av_log(avctx, AV_LOG_ERROR, "start_display_time must be 0.\n");
        return -1;
    }

    ret = avctx->codec->encode(avctx, buf, buf_size, sub);
    avctx->frame_number++;
    return ret;
}

/**
 * Attempt to guess proper monotonic timestamps for decoded video frames
 * which might have incorrect times. Input timestamps may wrap around, in
 * which case the output will as well.
 *
 * @param pts the pts field of the decoded AVPacket, as passed through
 * AVFrame.pkt_pts
 * @param dts the dts field of the decoded AVPacket
 * @return one of the input values, may be AV_NOPTS_VALUE
 */
static int64_t guess_correct_pts(AVCodecContext *ctx,
                                 int64_t reordered_pts, int64_t dts)
{
    int64_t pts = AV_NOPTS_VALUE;

    if (dts != AV_NOPTS_VALUE)
    {
        ctx->pts_correction_num_faulty_dts += dts <= ctx->pts_correction_last_dts;
        ctx->pts_correction_last_dts        = dts;
    }
    if (reordered_pts != AV_NOPTS_VALUE)
    {
        ctx->pts_correction_num_faulty_pts += reordered_pts <= ctx->pts_correction_last_pts;
        ctx->pts_correction_last_pts        = reordered_pts;
    }
    if ((ctx->pts_correction_num_faulty_pts <= ctx->pts_correction_num_faulty_dts || dts == AV_NOPTS_VALUE)
        && reordered_pts != AV_NOPTS_VALUE)
        pts = reordered_pts;
    else
        pts = dts;

    return pts;
}

#if 0 // David Debug
    #include <stdio.h>
extern unsigned int flag_try1;

    #define DMX_COUNT_MAX_V 749
    #define DMX_COUNT_MAX_A 5000

static unsigned long dec_cnt_v = 0;
static unsigned long dec_cnt_a = 0;

static FILE          *pVFile   = NULL;
static FILE          *pAFile   = NULL;
#endif

int attribute_align_arg avcodec_decode_video2(AVCodecContext *avctx, AVFrame *picture,
                                              int *got_picture_ptr,
                                              AVPacket *avpkt)
{
    int ret;

#if 0 // David Debug
    av_log(NULL, AV_LOG_INFO, "[Dav] dec_cnt_v=%ld\r\n", dec_cnt_v++);
    //return avpkt->size;
#endif
#if 0 // David Debug
    if (!flag_try1)
    {
        static int dec_vcnt    = 0;
        char       fname[1024] = {0};
        av_log(NULL, AV_LOG_INFO, "[Dav]%s() L#%ld: dec_vcnt=%ld\r\n", __FUNCTION__, __LINE__, dec_vcnt);
        snprintf(fname, sizeof(fname), "A:/v_frame-%d", dec_vcnt++);
        FILE       *pVF        = fopen(fname, "wb");
        fwrite(avpkt->data, 1, avpkt->size, pVF);
        fclose(pVF);
    }
#endif
#if 0 // David Debug
      // David debug
    if (flag_try1 == 1)
    {
        goto skip;
    }
    av_log(NULL, AV_LOG_INFO, "[Dav] dec_cnt_v=%ld\r\n", dec_cnt_v++);
    #if 1
    if (!pVFile && dec_cnt_v <= DMX_COUNT_MAX_V)
    {
        pVFile = fopen("A:/v_output", "wb");
        av_log(NULL, AV_LOG_INFO, "[Dav]%s() L#%ld: pVFile=%ld\r\n", __FUNCTION__, __LINE__, pVFile);
    }

    if (pVFile && dec_cnt_v <= DMX_COUNT_MAX_V)
    {
        fwrite(avpkt->data, 1, avpkt->size, pVFile);
    }

    if (pVFile && dec_cnt_v > DMX_COUNT_MAX_V)
    {
        fclose(pVFile);
        pVFile = 0;
    }
    #endif
skip:
    return avpkt->size;
#endif

    *got_picture_ptr = 0;
    if ((!avpkt) || (!avpkt->data) || (avpkt->size <= 0))
        return 0;
    if ((avctx->coded_width || avctx->coded_height) && av_image_check_size(avctx->coded_width, avctx->coded_height, 0, avctx))
        return -1;

    if ((avctx->codec->capabilities & CODEC_CAP_DELAY) || (avctx->active_thread_type & FF_THREAD_FRAME))
    {
        av_packet_split_side_data(avpkt);
        avctx->pkt = avpkt;
        if (HAVE_THREADS && avctx->active_thread_type & FF_THREAD_FRAME)
            ret = ff_thread_decode_frame(avctx, picture, got_picture_ptr,
                                         avpkt);
        else
        {
            ret              = avctx->codec->decode(avctx, picture, got_picture_ptr,
                                                    avpkt);
            picture->pkt_dts = avpkt->dts;

            if (!avctx->has_b_frames)
            {
                picture->pkt_pos = avpkt->pos;
            }
            //FIXME these should be under if(!avctx->has_b_frames)
            if (!picture->sample_aspect_ratio.num)
                picture->sample_aspect_ratio = avctx->sample_aspect_ratio;
            if (!picture->width)
                picture->width = avctx->width;
            if (!picture->height)
                picture->height = avctx->height;
            if (picture->format == PIX_FMT_NONE)
                picture->format = avctx->pix_fmt;
        }

        emms_c(); //needed to avoid an emms_c() call before every return;

        if (*got_picture_ptr)
        {
            avctx->frame_number++;
            picture->best_effort_timestamp = guess_correct_pts(avctx,
                                                               picture->pkt_pts,
                                                               picture->pkt_dts);
        }
    }
    else
        ret = 0;

    return ret;
}

#if FF_API_OLD_DECODE_AUDIO
int attribute_align_arg avcodec_decode_audio3(AVCodecContext *avctx, int16_t *samples,
                                              int *frame_size_ptr,
                                              AVPacket *avpkt)
{
    AVFrame frame;
    int     ret, got_frame = 0;

    if (avctx->get_buffer != avcodec_default_get_buffer)
    {
        av_log(avctx, AV_LOG_ERROR, "Overriding custom get_buffer() for "
               "avcodec_decode_audio3()\n");
        avctx->get_buffer     = avcodec_default_get_buffer;
        avctx->release_buffer = avcodec_default_release_buffer;
    }

    #if 0 // David debug
    av_log(NULL, AV_LOG_INFO, "[Dav] dec_cnt_a=%ld\r\n", dec_cnt_a++);
    //return avpkt->size;
    #endif
    #if 0
    if (!pAFile && dec_cnt_a <= DMX_COUNT_MAX_A)
    {
        pAFile = fopen("A:/a_output", "wb");
        av_log(NULL, AV_LOG_INFO, "[Dav]%s() L#%ld: pAFile=%ld\r\n", __FUNCTION__, __LINE__, pAFile);
    }

    if (pAFile && dec_cnt_a <= DMX_COUNT_MAX_A)
    {
        fwrite(avpkt->data, 1, avpkt->size, pAFile);
    }

    if (pAFile && dec_cnt_a > DMX_COUNT_MAX_A)
    {
        fclose(pAFile);
        pAFile = 0;
    }
    #endif

    ret = avcodec_decode_audio4(avctx, &frame, &got_frame, avpkt);

    if (ret >= 0 && got_frame)
    {
        int ch, plane_size;
        int planar    = av_sample_fmt_is_planar(avctx->sample_fmt);
        int data_size = av_samples_get_buffer_size(&plane_size, avctx->channels,
                                                   frame.nb_samples,
                                                   avctx->sample_fmt, 1);
        if (*frame_size_ptr < data_size)
        {
            av_log(avctx, AV_LOG_ERROR, "output buffer size is too small for "
                   "the current frame (%d < %d)\n", *frame_size_ptr, data_size);
            return AVERROR(EINVAL);
        }

        memcpy(samples, frame.extended_data[0], plane_size);

        if (planar && avctx->channels > 1)
        {
            uint8_t *out = ((uint8_t *)samples) + plane_size;
            for (ch = 1; ch < avctx->channels; ch++)
            {
                memcpy(out, frame.extended_data[ch], plane_size);
                out += plane_size;
            }
        }
        *frame_size_ptr = data_size;
    }
    else
    {
        *frame_size_ptr = 0;
    }
    return ret;
}
#endif

int attribute_align_arg avcodec_decode_audio4(AVCodecContext *avctx,
                                              AVFrame *frame,
                                              int *got_frame_ptr,
                                              AVPacket *avpkt)
{
    int            ret     = 0;
    unsigned long  bufSize = 0;
    int            nResult, nInput;
#ifdef CONFIG_ITADRIVER
    unsigned char  adtsHeader[7];
#endif
    unsigned int   time;
    unsigned short nData;
    unsigned short nDataE;
    uint16_t       nAudioPluginRegister = 0;
    int            nTemp;

    gettimeofday(&gEndT, NULL);
    if (itpTimevalDiff(&gStartT, &gEndT) > 5 * 1000)
        av_log(avctx, AV_LOG_ERROR, "[Utils] audio write duration %d \n", itpTimevalDiff(&gStartT, &gEndT));
    gettimeofday(&gStartT, NULL);

    nResult        = nInput = 0;
    *got_frame_ptr = 0;

    if (!avpkt->data && avpkt->size)
    {
        av_log(avctx, AV_LOG_ERROR, "invalid packet: NULL data, size != 0\n");
        return AVERROR(EINVAL);
    }

    if ((avctx->codec->capabilities & CODEC_CAP_DELAY) || avpkt->size)
    {
        av_packet_split_side_data(avpkt);
        avctx->pkt = avpkt;
#ifdef CONFIG_ITADRIVER
    #ifdef WIN32
        //ret = avctx->codec->decode(avctx, frame, got_frame_ptr, avpkt);
    #endif
        ret = avpkt->size;
    #ifdef CONFIG_GNASH_FLASH
        if (gFlashInfo.nFlashEnable == 1)
        {
            nInput = getAudioInput(avctx);

            // wave format, send wave header to driver
            if (avctx->frame_number == 0 && gFlashInfo.nAudioFormat[nInput] == ITE_WAV_DECODE)
            {
                setWaveInfo(avctx, &gFlashInfo.wavInfo[nInput], avpkt->size, nInput);
            }
            if (avpkt->size > AUDIO_FRAME_SIZE && gFlashInfo.nAudioFormat[nInput] != ITE_WAV_DECODE)
            {
                nTemp = 0;
                do
                {
                    nResult = iteAudioGetFlashAvailableBufferLength(nInput, &bufSize);
                    usleep(5000);
                    updateMessageQueue();
                    if (nTemp + bufSize < avpkt->size)
                        iteAudioWriteFlashStream(nInput, avpkt->data + nTemp, bufSize);
                    else
                        iteAudioWriteFlashStream(nInput, avpkt->data + nTemp, avpkt->size - bufSize);
                    nTemp += bufSize;
                } while (nTemp < avpkt->size);
            }
            else
            {
                do
                {
                    nResult = iteAudioGetFlashAvailableBufferLength(nInput, &bufSize);
                    usleep(1000);
                    updateMessageQueue();
                } while (bufSize < avpkt->size);
                iteAudioWriteFlashStream(nInput, avpkt->data, avpkt->size);
            }
        #ifdef CONFIG_DUMP_AUDIO
            if (nInput == 0)
                fwrite(avpkt->data, 1, avpkt->size, fin);
            else if (nInput == 1)
                fwrite(avpkt->data, 1, avpkt->size, fin1);
            else if (nInput == 2)
                fwrite(avpkt->data, 1, avpkt->size, fin2);
        #endif
        }
        else
        {
            if (0)/*(avpkt->size > AUDIO_FRAME_SIZE)*/
            {
                nTemp = 0;
                do
                {
                    nResult = iteAudioGetFlashAvailableBufferLength(nInput, &bufSize);
                    usleep(5000);
                    updateMessageQueue();
                    // AAC must write header
                    if (avctx->codec_id == CODEC_ID_AAC)
                    {}

                    if (nTemp + bufSize < avpkt->size)
                        iteAudioWriteFlashStream(nInput, avpkt->data + nTemp, bufSize);
                    else
                        iteAudioWriteFlashStream(nInput, avpkt->data + nTemp, avpkt->size - bufSize);
                    nTemp += bufSize;
                } while (nTemp < avpkt->size);
            }
            else
            {
                do
                {
                    nResult = iteAudioGetAvailableBufferLength(ITE_AUDIO_OUTPUT_BUFFER, &bufSize);
                    usleep(1000);
                    updateMessageQueue();
                } while (bufSize < avpkt->size);

                if (avctx->codec_id == CODEC_ID_AAC)
                    if (avpkt->data[0] != 0xff || avpkt->data[1] & 0xf0 != 0xf0)
                    {
                        createADTSHeader(adtsHeader, avctx->sample_rate, avctx->channels, avpkt->size);
                        iteAudioWriteStream(adtsHeader, 7);
        #ifdef CONFIG_DUMP_AUDIO
                        fwrite(adtsHeader, 1, 7, fin);
        #endif
                    }
                iteAudioWriteStream(avpkt->data, avpkt->size);
            }
        #ifdef CONFIG_DUMP_AUDIO
            fwrite(avpkt->data, 1, avpkt->size, fin);
        #endif
        }
    #else

        if (0)    /*(avpkt->size > AUDIO_FRAME_SIZE)*/
        {
            nTemp = 0;
            do
            {
                nResult = iteAudioGetAvailableBufferLength(ITE_AUDIO_OUTPUT_BUFFER, &bufSize);
                usleep(5000);
                updateMessageQueue();
                if (nTemp + bufSize < avpkt->size)
                    iteAudioWriteStream(avpkt->data + nTemp, bufSize);
                else
                    iteAudioWriteStream(avpkt->data + nTemp, avpkt->size - nTemp);
                nTemp += bufSize;
            } while (nTemp < avpkt->size);
        }
        else
        {
            do
            {
                nResult = iteAudioGetAvailableBufferLength(ITE_AUDIO_OUTPUT_BUFFER, &bufSize);
                usleep(1000);
                updateMessageQueue();
            } while (bufSize < avpkt->size);

            nTemp = 0;
            iteAudioGetAttrib(ITE_AUDIO_FFMPEG_PAUSE_AUDIO, &nTemp);
            if (nTemp)
            {
               // av_log(avctx, AV_LOG_ERROR, "video player,pause audio \n");
            
            }
            else 
            {
	            if (avctx->codec_id == CODEC_ID_AAC)
	                if (avpkt->data[0] != 0xff || avpkt->data[1] & 0xf0 != 0xf0)
	                {
	                    createADTSHeader(adtsHeader, avctx->sample_rate, avctx->channels, avpkt->size);
	                    iteAudioWriteStream(adtsHeader, 7);
	        #ifdef CONFIG_DUMP_AUDIO
	                    fwrite(adtsHeader, 1, 7, fin);
	        #endif
	                }
	                #ifdef SWITCH_WORKAROUND                    
	                if (gWavEnable==1){
	                    setWaveInfo(avctx,&gWaveInfo,0,0);
	                    gWavEnable = 0;
	                }
	                #endif
	            iteAudioWriteStream(avpkt->data, avpkt->size);
            }
        }
        #ifdef CONFIG_DUMP_AUDIO
        if (nInput == 0)
            fwrite(avpkt->data, 1, avpkt->size, fin);
        else if (nInput == 1)
            fwrite(avpkt->data, 1, avpkt->size, fin1);
        else if (nInput == 2)
            fwrite(avpkt->data, 1, avpkt->size, fin2);
        #endif

    #endif

    #ifndef WIN32
        //iteAudioGetDecodeTimeV2(&time);
    #endif
        nData = ithReadRegH(0x165E);
    #ifdef CFG_AUDIO_ENABLE
        if (avctx->frame_number % 50 == 0)
            av_log(avctx, AV_LOG_DEBUG, "frame_number %d rd ptr 0x%x status 0x%x time %d  \n", avctx->frame_number, I2S_DA32_GET_RP(), nData, time);
    #endif
        avctx->frame_number++;
#else
        ret = avctx->codec->decode(avctx, frame, got_frame_ptr, avpkt);
        if (ret >= 0 && *got_frame_ptr)
        {
            avctx->frame_number++;
            frame->pkt_dts = avpkt->dts;
        }
#endif
    }
    return ret;
}

int avcodec_decode_subtitle2(AVCodecContext *avctx, AVSubtitle *sub,
                             int *got_sub_ptr,
                             AVPacket *avpkt)
{
    int ret;

    avctx->pkt   = avpkt;
    *got_sub_ptr = 0;
    avcodec_get_subtitle_defaults(sub);
    ret          = avctx->codec->decode(avctx, sub, got_sub_ptr, avpkt);
    if (*got_sub_ptr)
        avctx->frame_number++;
    return ret;
}

void avsubtitle_free(AVSubtitle *sub)
{
    int i;

    for (i = 0; i < sub->num_rects; i++)
    {
        av_freep(&sub->rects[i]->pict.data[0]);
        av_freep(&sub->rects[i]->pict.data[1]);
        av_freep(&sub->rects[i]->pict.data[2]);
        av_freep(&sub->rects[i]->pict.data[3]);
        av_freep(&sub->rects[i]->text);
        av_freep(&sub->rects[i]->ass);
        av_freep(&sub->rects[i]);
    }

    av_freep(&sub->rects);

    memset(sub, 0, sizeof(AVSubtitle));
}

av_cold int avcodec_close(AVCodecContext *avctx)
{
    int i = 0;
    /* If there is a user-supplied mutex locking routine, call it. */
    if (ff_lockmgr_cb)
    {
        if ((*ff_lockmgr_cb)(&codec_mutex, AV_LOCK_OBTAIN))
            return -1;
    }
#ifdef CONFIG_ITADRIVER
    if (avctx->codec_type == AVMEDIA_TYPE_AUDIO)
    {
    #ifdef CONFIG_GNASH_FLASH
        // if mp3/wav, clear input stream
        if (gFlashInfo.nFlashEnable == 1)
        {
            // select input stream
            for (i = 0; i < FLASH_AUDIO_INPUT; i++)
            {
                if (gFlashInfo.nInputUsing[i] == avctx)
                {
                    gFlashInfo.nInputUsing[i] = 0;
                    iteAudioSetFlashInputStatus(i, 0, 0);
                    av_log(avctx, AV_LOG_WARNING, "avcodec_close input %d avctx 0x%x \n", i, avctx);
        #ifdef CONFIG_DUMP_AUDIO
                    if (i == 0)
                    {
                        fclose(fin);
                        fin = NULL;
                    }
                    else if (i == 1)
                    {
                        fclose(fin1);
                        fin1 = NULL;
                    }
                    else if (i == 2)
                    {
                        fclose(fin2);
                        fin2 = NULL;
                    }
        #endif
                    break;
                }
            }
            if (i > FLASH_AUDIO_INPUT)
            {
                av_log(avctx, AV_LOG_WARNING, "audio input stream can not found to clean 0x%x \n", avctx);
            }
        }
        else
        {
            av_log(avctx, AV_LOG_WARNING, "iteAudioStop 0x%x codec_id %d \n", avctx, avctx->codec_id);
            iteAudioStop();
        #ifdef CONFIG_DUMP_AUDIO
            if (fin != NULL)
            {
                fclose(fin);
                fin = NULL;
            }
        #endif
        }
    #else //#ifdef CONFIG_GNASH_FLASH

        av_log(avctx, AV_LOG_WARNING, "iteAudioStop 0x%x codec_id %d \n", avctx, avctx->codec_id);
        iteAudioStop();
        #ifdef CONFIG_DUMP_AUDIO
        if (fin != NULL)
        {
            fclose(fin);
            fin = NULL;
        }
        #endif

    #endif //#ifdef CONFIG_GNASH_FLASH
    }
#endif
    entangled_thread_counter++;
    if (entangled_thread_counter != 1)
    {
        av_log(avctx, AV_LOG_ERROR, "insufficient thread locking around avcodec_open/close()\n");
        entangled_thread_counter--;
        return -1;
    }

    if (HAVE_THREADS && avctx->thread_opaque)
        ff_thread_free(avctx);
    if (avctx->codec && avctx->codec->close)
        avctx->codec->close(avctx);
    avcodec_default_free_buffers(avctx);
    avctx->coded_frame = NULL;
    av_freep(&avctx->internal);
    if (avctx->codec && avctx->codec->priv_class)
        av_opt_free(avctx->priv_data);
    av_opt_free(avctx);
    av_freep(&avctx->priv_data);
    if (avctx->codec && avctx->codec->encode)
        av_freep(&avctx->extradata);
    avctx->codec              = NULL;
    avctx->active_thread_type = 0;
    entangled_thread_counter--;

    /* Release any user-supplied mutex. */
    if (ff_lockmgr_cb)
    {
        (*ff_lockmgr_cb)(&codec_mutex, AV_LOCK_RELEASE);
    }
    return 0;
}

static enum CodecID remap_deprecated_codec_id(enum CodecID id)
{
    switch (id)
    {
    case CODEC_ID_G723_1_DEPRECATED: return CODEC_ID_G723_1;
    case CODEC_ID_G729_DEPRECATED: return CODEC_ID_G729;
    case CODEC_ID_UTVIDEO_DEPRECATED: return CODEC_ID_UTVIDEO;
    default: return id;
    }
}

AVCodec *avcodec_find_encoder(enum CodecID id)
{
    AVCodec *p, *experimental = NULL;
    p  = first_avcodec;
    id = remap_deprecated_codec_id(id);
    while (p)
    {
        if (p->encode != NULL && p->id == id)
        {
            if (p->capabilities & CODEC_CAP_EXPERIMENTAL && !experimental)
            {
                experimental = p;
            }
            else
                return p;
        }
        p = p->next;
    }
    return experimental;
}

AVCodec *avcodec_find_encoder_by_name(const char *name)
{
    AVCodec *p;
    if (!name)
        return NULL;
    p = first_avcodec;
    while (p)
    {
        if (p->encode != NULL && strcmp(name, p->name) == 0)
            return p;
        p = p->next;
    }
    return NULL;
}

AVCodec *avcodec_find_decoder(enum CodecID id)
{
    AVCodec *p, *experimental = NULL;
    p  = first_avcodec;
    id = remap_deprecated_codec_id(id);
    while (p)
    {
        if (p->decode != NULL && p->id == id)
        {
            if (p->capabilities & CODEC_CAP_EXPERIMENTAL && !experimental)
            {
                experimental = p;
            }
            else
                return p;
        }
        p = p->next;
    }
    return experimental;
}

AVCodec *avcodec_find_decoder_by_name(const char *name)
{
    AVCodec *p;
    if (!name)
        return NULL;
    p = first_avcodec;
    while (p)
    {
        if (p->decode != NULL && strcmp(name, p->name) == 0)
            return p;
        p = p->next;
    }
    return NULL;
}

static int get_bit_rate(AVCodecContext *ctx)
{
    int bit_rate;
    int bits_per_sample;

    switch (ctx->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:
    case AVMEDIA_TYPE_DATA:
    case AVMEDIA_TYPE_SUBTITLE:
    case AVMEDIA_TYPE_ATTACHMENT:
        bit_rate = ctx->bit_rate;
        break;

    case AVMEDIA_TYPE_AUDIO:
        bits_per_sample = av_get_bits_per_sample(ctx->codec_id);
        bit_rate        = bits_per_sample ? ctx->sample_rate * ctx->channels * bits_per_sample : ctx->bit_rate;
        break;

    default:
        bit_rate = 0;
        break;
    }
    return bit_rate;
}

const char *avcodec_get_name(enum CodecID id)
{
    AVCodec *codec;

#if !CONFIG_SMALL
    switch (id)
    {
    #include "libavcodec/codec_names.h"
    }
    av_log(NULL, AV_LOG_WARNING, "Codec 0x%x is not in the full list.\n", id);
#endif
    codec = avcodec_find_decoder(id);
    if (codec)
        return codec->name;
    codec = avcodec_find_encoder(id);
    if (codec)
        return codec->name;
    return "unknown_codec";
}

size_t av_get_codec_tag_string(char *buf, size_t buf_size, unsigned int codec_tag)
{
    int i, len, ret = 0;

    for (i = 0; i < 4; i++)
    {
        len         = snprintf(buf, buf_size,
                               isprint(codec_tag & 0xFF) ? "%c" : "[%d]", codec_tag & 0xFF);
        buf        += len;
        buf_size    = buf_size > len ? buf_size - len : 0;
        ret        += len;
        codec_tag >>= 8;
    }
    return ret;
}

void avcodec_string(char *buf, int buf_size, AVCodecContext *enc, int encode)
{
    const char *codec_type;
    const char *codec_name;
    const char *profile = NULL;
    AVCodec    *p;
    int        bitrate;
    AVRational display_aspect_ratio;

    if (!buf || buf_size <= 0)
        return;
    codec_type = av_get_media_type_string(enc->codec_type);
    codec_name = avcodec_get_name(enc->codec_id);
    if (enc->profile != FF_PROFILE_UNKNOWN)
    {
        p = encode ? avcodec_find_encoder(enc->codec_id) :
            avcodec_find_decoder(enc->codec_id);
        if (p)
            profile = av_get_profile_name(p, enc->profile);
    }

    snprintf(buf, buf_size, "%s: %s%s", codec_type ? codec_type : "unknown",
             codec_name, enc->mb_decision ? " (hq)" : "");
    buf[0] ^= 'a' ^ 'A'; /* first letter in uppercase */
    if (profile)
        snprintf(buf + strlen(buf), buf_size - strlen(buf), " (%s)", profile);
    if (enc->codec_tag)
    {
        char tag_buf[32];
        av_get_codec_tag_string(tag_buf, sizeof(tag_buf), enc->codec_tag);
        snprintf(buf + strlen(buf), buf_size - strlen(buf),
                 " (%s / 0x%04X)", tag_buf, enc->codec_tag);
    }
    switch (enc->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        if (enc->pix_fmt != PIX_FMT_NONE)
        {
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", %s",
                     av_get_pix_fmt_name(enc->pix_fmt));
        }
        if (enc->width)
        {
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", %dx%d",
                     enc->width, enc->height);
            if (enc->sample_aspect_ratio.num)
            {
                av_reduce(&display_aspect_ratio.num, &display_aspect_ratio.den,
                          enc->width * enc->sample_aspect_ratio.num,
                          enc->height * enc->sample_aspect_ratio.den,
                          1024 * 1024);
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         " [SAR %d:%d DAR %d:%d]",
                         enc->sample_aspect_ratio.num, enc->sample_aspect_ratio.den,
                         display_aspect_ratio.num, display_aspect_ratio.den);
            }
            if (av_log_get_level() >= AV_LOG_DEBUG)
            {
                int g = av_gcd(enc->time_base.num, enc->time_base.den);
                snprintf(buf + strlen(buf), buf_size - strlen(buf),
                         ", %d/%d",
                         enc->time_base.num / g, enc->time_base.den / g);
            }
        }
        if (encode)
        {
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", q=%d-%d", enc->qmin, enc->qmax);
        }
        break;

    case AVMEDIA_TYPE_AUDIO:
        if (enc->sample_rate)
        {
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", %d Hz", enc->sample_rate);
        }
        av_strlcat(buf, ", ", buf_size);
        av_get_channel_layout_string(buf + strlen(buf), buf_size - strlen(buf), enc->channels, enc->channel_layout);
        if (enc->sample_fmt != AV_SAMPLE_FMT_NONE)
        {
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", %s", av_get_sample_fmt_name(enc->sample_fmt));
        }
        break;

    default:
        return;
    }
    if (encode)
    {
        if (enc->flags & CODEC_FLAG_PASS1)
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", pass 1");
        if (enc->flags & CODEC_FLAG_PASS2)
            snprintf(buf + strlen(buf), buf_size - strlen(buf),
                     ", pass 2");
    }
    bitrate = get_bit_rate(enc);
    if (bitrate != 0)
    {
        snprintf(buf + strlen(buf), buf_size - strlen(buf),
                 ", %d kb/s", bitrate / 1000);
    }
}

const char *av_get_profile_name(const AVCodec *codec, int profile)
{
    const AVProfile *p;
    if (profile == FF_PROFILE_UNKNOWN || !codec->profiles)
        return NULL;

    for (p = codec->profiles; p->profile != FF_PROFILE_UNKNOWN; p++)
        if (p->profile == profile)
            return p->name;

    return NULL;
}

unsigned avcodec_version( void )
{
    av_assert0(CODEC_ID_V410 == 164);
    av_assert0(CODEC_ID_PCM_S8_PLANAR == 65563);
    av_assert0(CODEC_ID_ADPCM_G722 == 69660);
    av_assert0(CODEC_ID_BMV_AUDIO == 86071);
    av_assert0(CODEC_ID_SRT == 94216);

    return LIBAVCODEC_VERSION_INT;
}

const char *avcodec_configuration(void)
{
    return FFMPEG_CONFIGURATION;
}

const char *avcodec_license(void)
{
#define LICENSE_PREFIX "libavcodec license: "
    return LICENSE_PREFIX FFMPEG_LICENSE + sizeof(LICENSE_PREFIX) - 1;
}

void avcodec_flush_buffers(AVCodecContext *avctx)
{
    if (HAVE_THREADS && avctx->active_thread_type & FF_THREAD_FRAME)
        ff_thread_flush(avctx);
    else if (avctx->codec->flush)
        avctx->codec->flush(avctx);
}

static void video_free_buffers(AVCodecContext *s)
{
    AVCodecInternal *avci = s->internal;
    int             i, j;

    if (!avci->buffer)
        return;

    if (avci->buffer_count)
        av_log(s, AV_LOG_WARNING, "Found %i unreleased buffers!\n",
               avci->buffer_count);
    for (i = 0; i < INTERNAL_BUFFER_SIZE; i++)
    {
        InternalBuffer *buf = &avci->buffer[i];
        for (j = 0; j < 4; j++)
        {
            av_freep(&buf->base[j]);
            buf->data[j] = NULL;
        }
    }
    av_freep(&avci->buffer);

    avci->buffer_count = 0;
}

static void audio_free_buffers(AVCodecContext *avctx)
{
    AVCodecInternal *avci = avctx->internal;
    InternalBuffer  *buf;

    if (!avci->buffer)
        return;
    buf = avci->buffer;

    if (buf->extended_data)
    {
        av_free(buf->extended_data[0]);
        if (buf->extended_data != buf->data)
            av_free(buf->extended_data);
    }
    av_freep(&avci->buffer);
}

void avcodec_default_free_buffers(AVCodecContext *avctx)
{
    switch (avctx->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        video_free_buffers(avctx);
        break;

    case AVMEDIA_TYPE_AUDIO:
        audio_free_buffers(avctx);
        break;

    default:
        break;
    }
}

#if FF_API_OLD_FF_PICT_TYPES
char av_get_pict_type_char(int pict_type)
{
    return av_get_picture_type_char(pict_type);
}
#endif

int av_get_bits_per_sample(enum CodecID codec_id)
{
    switch (codec_id)
    {
    case CODEC_ID_ADPCM_SBPRO_2:
        return 2;

    case CODEC_ID_ADPCM_SBPRO_3:
        return 3;

    case CODEC_ID_ADPCM_SBPRO_4:
    case CODEC_ID_ADPCM_CT:
    case CODEC_ID_ADPCM_IMA_WAV:
    case CODEC_ID_ADPCM_IMA_QT:
    case CODEC_ID_ADPCM_SWF:
    case CODEC_ID_ADPCM_MS:
    case CODEC_ID_ADPCM_YAMAHA:
    case CODEC_ID_ADPCM_G722:
        return 4;

    case CODEC_ID_PCM_ALAW:
    case CODEC_ID_PCM_MULAW:
    case CODEC_ID_PCM_S8:
    case CODEC_ID_PCM_U8:
    case CODEC_ID_PCM_ZORK:
        return 8;

    case CODEC_ID_PCM_S16BE:
    case CODEC_ID_PCM_S16LE:
    case CODEC_ID_PCM_S16LE_PLANAR:
    case CODEC_ID_PCM_U16BE:
    case CODEC_ID_PCM_U16LE:
        return 16;

    case CODEC_ID_PCM_S24DAUD:
    case CODEC_ID_PCM_S24BE:
    case CODEC_ID_PCM_S24LE:
    case CODEC_ID_PCM_U24BE:
    case CODEC_ID_PCM_U24LE:
        return 24;

    case CODEC_ID_PCM_S32BE:
    case CODEC_ID_PCM_S32LE:
    case CODEC_ID_PCM_U32BE:
    case CODEC_ID_PCM_U32LE:
    case CODEC_ID_PCM_F32BE:
    case CODEC_ID_PCM_F32LE:
        return 32;

    case CODEC_ID_PCM_F64BE:
    case CODEC_ID_PCM_F64LE:
        return 64;

    default:
        return 0;
    }
}

#if FF_API_OLD_SAMPLE_FMT
int av_get_bits_per_sample_format(enum AVSampleFormat sample_fmt)
{
    return av_get_bytes_per_sample(sample_fmt) << 3;
}
#endif

#if !HAVE_THREADS
int ff_thread_init(AVCodecContext *s)
{
    return -1;
}
#endif

unsigned int av_xiphlacing(unsigned char *s, unsigned int v)
{
    unsigned int n = 0;

    while (v >= 0xff)
    {
        *s++ = 0xff;
        v   -= 0xff;
        n++;
    }
    *s = v;
    n++;
    return n;
}

int ff_match_2uint16(const uint16_t(*tab)[2], int size, int a, int b)
{
    int i;
    for (i = 0; i < size && !(tab[i][0] == a && tab[i][1] == b); i++)
        ;
    return i;
}

void av_log_missing_feature(void *avc, const char *feature, int want_sample)
{
    av_log(avc, AV_LOG_WARNING, "%s not implemented. Update your FFmpeg "
           "version to the newest one from Git. If the problem still "
           "occurs, it means that your file has a feature which has not "
           "been implemented.\n", feature);
    if (want_sample)
        av_log_ask_for_sample(avc, NULL);
}

void av_log_ask_for_sample(void *avc, const char *msg, ...)
{
    va_list argument_list;

    va_start(argument_list, msg);

    if (msg)
        av_vlog(avc, AV_LOG_WARNING, msg, argument_list);
    av_log(avc, AV_LOG_WARNING, "If you want to help, upload a sample "
           "of this file to ftp://upload.ffmpeg.org/MPlayer/incoming/ "
           "and contact the ffmpeg-devel mailing list.\n");

    va_end(argument_list);
}

static AVHWAccel *first_hwaccel = NULL;

void av_register_hwaccel(AVHWAccel *hwaccel)
{
    AVHWAccel **p = &first_hwaccel;
    while (*p)
        p = &(*p)->next;
    *p            = hwaccel;
    hwaccel->next = NULL;
}

AVHWAccel *av_hwaccel_next(AVHWAccel *hwaccel)
{
    return hwaccel ? hwaccel->next : first_hwaccel;
}

AVHWAccel *ff_find_hwaccel(enum CodecID codec_id, enum PixelFormat pix_fmt)
{
    AVHWAccel *hwaccel = NULL;

    while ((hwaccel = av_hwaccel_next(hwaccel)))
    {
        if (hwaccel->id == codec_id
            && hwaccel->pix_fmt == pix_fmt)
            return hwaccel;
    }
    return NULL;
}

int av_lockmgr_register(int (*cb)(void **mutex, enum AVLockOp op))
{
    if (ff_lockmgr_cb)
    {
        if (ff_lockmgr_cb(&codec_mutex, AV_LOCK_DESTROY))
            return -1;
        if (ff_lockmgr_cb(&avformat_mutex, AV_LOCK_DESTROY))
            return -1;
    }

    ff_lockmgr_cb = cb;

    if (ff_lockmgr_cb)
    {
        if (ff_lockmgr_cb(&codec_mutex, AV_LOCK_CREATE))
            return -1;
        if (ff_lockmgr_cb(&avformat_mutex, AV_LOCK_CREATE))
            return -1;
    }
    return 0;
}

int avpriv_lock_avformat(void)
{
    if (ff_lockmgr_cb)
    {
        if ((*ff_lockmgr_cb)(&avformat_mutex, AV_LOCK_OBTAIN))
            return -1;
    }
    return 0;
}

int avpriv_unlock_avformat(void)
{
    if (ff_lockmgr_cb)
    {
        if ((*ff_lockmgr_cb)(&avformat_mutex, AV_LOCK_RELEASE))
            return -1;
    }
    return 0;
}

unsigned int avpriv_toupper4(unsigned int x)
{
    return toupper( x & 0xFF)
           + (toupper((x >> 8 ) & 0xFF) << 8 )
           + (toupper((x >> 16) & 0xFF) << 16)
           + (toupper((x >> 24) & 0xFF) << 24);
}

#if !HAVE_THREADS

int ff_thread_get_buffer(AVCodecContext *avctx, AVFrame *f)
{
    f->owner = avctx;

    ff_init_buffer_info(avctx, f);

    return avctx->get_buffer(avctx, f);
}

void ff_thread_release_buffer(AVCodecContext *avctx, AVFrame *f)
{
    f->owner->release_buffer(f->owner, f);
}

void ff_thread_finish_setup(AVCodecContext *avctx)
{}

void ff_thread_report_progress(AVFrame *f, int progress, int field)
{}

void ff_thread_await_progress(AVFrame *f, int progress, int field)
{}

#endif

#if FF_API_THREAD_INIT
int avcodec_thread_init(AVCodecContext *s, int thread_count)
{
    s->thread_count = thread_count;
    return ff_thread_init(s);
}
#endif

enum AVMediaType avcodec_get_type(enum CodecID codec_id)
{
    AVCodec *c = avcodec_find_decoder(codec_id);
    if (!c)
        c = avcodec_find_encoder(codec_id);
    if (c)
        return c->type;

    if (codec_id <= CODEC_ID_NONE)
        return AVMEDIA_TYPE_UNKNOWN;
    else if (codec_id < CODEC_ID_FIRST_AUDIO)
        return AVMEDIA_TYPE_VIDEO;
    else if (codec_id < CODEC_ID_FIRST_SUBTITLE)
        return AVMEDIA_TYPE_AUDIO;
    else if (codec_id < CODEC_ID_FIRST_UNKNOWN)
        return AVMEDIA_TYPE_SUBTITLE;

    return AVMEDIA_TYPE_UNKNOWN;
}
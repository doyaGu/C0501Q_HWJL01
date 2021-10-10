#include <pthread.h>
#include <sys/ioctl.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <malloc.h>
#include "ite/itp.h"
#include "filelist.h"
#include "ite/ith_video.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"

#include "isp/mmp_isp.h"
// parsing golomb
#include "golomb.h"
#include "ite/itv.h"

//#define printf(...) ((void)0);
/*
 * Global variables
 */
FILELIST files;
bool     stopTest;

#define DIRECT_FIRE_COMMAND 0
#define READ_WHOLE_FILE     0
#define SCALING             1
#define LOOP_FILE           1
#define GROUP_FILE          1
#define SINGLE_PLAY         0
#define INPUT_BUFFER_SIZE   512000    //307200

#if GROUP_FILE
    #ifdef LOOP_FILE
        #undef LOOP_FILE
        #define LOOP_FILE   0
    #endif
#endif

#if SINGLE_PLAY
    #ifdef LOOP_FILE
        #undef LOOP_FILE
        #define LOOP_FILE  0
    #endif
    #ifdef GROUP_FILE
        #undef GROUP_FILE
        #define GROUP_FILE 0
    #endif
#endif

#define MACRO_ERTN         { \
        printf("%f:%d, error returned!\n", __FILE__, __LINE__); \
        return (void *)-1; \
}

#define MACRO_NRTN         { \
        printf("%f:%d, error returned!\n", __FILE__, __LINE__); \
        return; \
}

void display_on_LCD(AVCodecContext *avctx, AVFrame *picture,
                    int *got_picture_ptr,
                    AVPacket *avpkt)
{
    ITV_DBUF_PROPERTY prop = {0};

    while (1)
    {
        uint8_t *dbuf = itv_get_dbuf_anchor();
        if (dbuf != NULL)
        {
            uint32_t rdIndex = picture->opaque ? *(uint32_t *)picture->opaque : 0;

            prop.src_w    = picture->width;
            prop.src_h    = picture->height;
            prop.ya       = picture->data[0];
            prop.ua       = picture->data[1];
            prop.va       = picture->data[2];
            prop.pitch_y  = picture->linesize[0];
            prop.pitch_uv = picture->linesize[1];
            prop.bidx     = rdIndex;
            prop.format   = MMP_ISP_IN_YUV420;

            itv_update_dbuf_anchor(&prop); // printf("display mark non-use %d\n", rdIndex);
            break;
        }
        else
        {
            ithDelay(1000);
        }
    }
}

static void
VideoInit(
    void)
{
    ithVideoInit(NULL);
    itv_init();
}

static void
VideoExit(
    void)
{
    /* release dbuf & itv */
    itv_flush_dbuf();
    itv_deinit();

    /* release decoder stuff */
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
    ithVideoExit();
    printf("%s(%d)\n", __FUNCTION__, __LINE__);
}

void play_file(const char     *filename,
               AVCodec        *codec,
               AVCodecContext *c,
               AVFrame        *picture,
               AVPacket       *avpkt)
{
    // buffer
    uint8_t       *inbuf          = NULL;
    unsigned int  inbuf_size      = INPUT_BUFFER_SIZE;

    unsigned char *pBufferStart   = NULL, *pBufferEnd = NULL;
    unsigned char *pCurFrameStart = NULL, *pLastFrameStart = NULL;
    bool          bReadFromFile   = true;
    int           max_frame_size  = 0;
    int           ret             = 0;
    bool          bEOS            = false;
    // file
    FILE          *f;
    unsigned int  file_size       = 0;
    int           frame, got_picture, len;

    // clock
    unsigned int  exe_clock = 0, dec_clock = 0, psr_clock = 0;

    if (!READ_WHOLE_FILE)
    {
        inbuf = malloc(sizeof(char) * INPUT_BUFFER_SIZE);
        if (!inbuf)
            MACRO_NRTN;
    }

    av_init_packet(avpkt);

    // init pointers
    pCurFrameStart = pLastFrameStart = pBufferStart = inbuf;

    // file open
    printf("\n[%s] open file %s\n", __FUNCTION__, filename);
    f = fopen(filename, "rb");

    assert(f);

    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    frame     = 0;
    exe_clock = PalGetClock();
    while (!bEOS)
    {
        unsigned int copy_size = 0;
        psr_clock = PalGetClock();

        while (1) // find frame
        {
            // Buffer error handle
            if (copy_size >= INPUT_BUFFER_SIZE) // temp solution, need modify
            {
                printf("[%s] Out of allocated parsing buffer size (%d)\n", __FUNCTION__, INPUT_BUFFER_SIZE);
                MACRO_NRTN;
            }

            // read pattern
            if (bReadFromFile)
            {
                inbuf_size = fread(pBufferStart + copy_size, 1, INPUT_BUFFER_SIZE - copy_size, f);
                if (inbuf_size == 0) // EOS
                {
                    if (LOOP_FILE)
                        fseek(f, 0, SEEK_SET);
                    else
                        bEOS = true;

                    pCurFrameStart = pBufferStart + copy_size; // output remain data

                    // statistic
                    {
                        unsigned long duration           = PalGetDuration(exe_clock);
                        float         AvgFramesPerSecond = frame / ((float)duration / 1000);
                        printf("[%s] frame = %d, duration = %u ms, AvgFramesPerSecond = %0.2f\n", __FUNCTION__, frame, duration, AvgFramesPerSecond);
                    }
                    break;
                }

                pBufferEnd    = pBufferStart + copy_size + inbuf_size;
                bReadFromFile = false;
#if defined(_WIN32) && defined(_DEBUG)
                if (pBufferEnd - pBufferStart != INPUT_BUFFER_SIZE)
                    break;
#endif
            }

            for (; pCurFrameStart <= (pBufferEnd - 8); pCurFrameStart++)
            {
                if (pCurFrameStart[0] == 0 && pCurFrameStart[1] == 0 && pCurFrameStart[2] == 1)
                {
                    unsigned char *pbuf             = (unsigned char *)pCurFrameStart + 3;
                    int           nal_unit_type     = *pbuf & 0x1F;
                    int           first_mb_in_slice = -1;
                    pbuf++;

                    if (nal_unit_type == 1 || nal_unit_type == 5)
                    {
                        GetBitContext gb;
                        init_get_bits(&gb, pbuf, 8 * 4);
                        first_mb_in_slice = get_ue_golomb(&gb);
                        if (first_mb_in_slice == 0)
                            break;
                    }
                    // else
                    //     printf("[%s] nal_unit_type %d\n", __FUNCTION__, nal_unit_type);
                }
            }

            // Buffer boundary, copy remain data to buffer start point
            if (pCurFrameStart > (pBufferEnd - 8))
            {
                unsigned int parsing_offset = pCurFrameStart - pLastFrameStart;

                copy_size = pBufferEnd - pLastFrameStart;

                memcpy(pBufferStart, pLastFrameStart, copy_size);

                pLastFrameStart = pBufferStart;
                pCurFrameStart  = pBufferStart + parsing_offset;

                bReadFromFile   = true;

                continue;
            }

            break;
        }

        avpkt->data     = pLastFrameStart;
        avpkt->size     = pCurFrameStart - pLastFrameStart;
        pLastFrameStart = pCurFrameStart;
        pCurFrameStart += 3; // next start code

        if (avpkt->size > max_frame_size)
            max_frame_size = avpkt->size;

        printf("\n[%s] #### start new slice, size = %d, parse = %u ms, max frame size = %u ####\n", __FUNCTION__, avpkt->size, PalGetDuration(psr_clock), max_frame_size);

        while (avpkt->size > 0)
        {
            dec_clock = PalGetClock();

            len       = avcodec_decode_video2(c, picture, &got_picture, avpkt);

            printf("[%s] decode = %u ms, frame %d\n", __FUNCTION__, PalGetDuration(dec_clock), c->frame_number);

#if defined(_WIN32) && defined(_DEBUG)
            if (c->frame_number == 1835)
            {
                printf("debug point\n");
            }
#endif

            if (len < 0) // decode error
                break;   //MACRO_ERTN;
            else if (len == 0)
                printf("[%s] warning return zero length\n", __FUNCTION__);

            if (got_picture)
            {
                //printf("[%s +] do display %p %p %p %p\n", __FUNCTION__, c, picture, &got_picture, &avpkt);
                display_on_LCD(c, picture, &got_picture, avpkt);
                //printf("[%s -] do display\n", __FUNCTION__);

                frame++;
                //usleep(30*1000);//ithDelay(30000); //sleep(1);
                //return 0;
            }
            avpkt->size -= len;
            avpkt->data += len;
        }
    }

    // release created buffer
    if (f)
        fclose(f);
    f              = NULL;
    if (inbuf)
        free(inbuf);
    inbuf          = NULL;
    pCurFrameStart = pLastFrameStart = pBufferStart = inbuf;
}

void *TestFunc(void *arg)
{
    // ffmpeg
    AVCodec        *codec;
    AVCodecContext *c       = NULL;
    AVFrame        *picture = NULL;
    AVDictionary   *dict    = NULL;
    // ffmpeg
    AVPacket       avpkt;
    int            frame, got_picture, len;

    int            file_idx = 0;

    // target board drivers init
    itpInit();
    ioctl(ITP_DEVICE_SCREEN, ITP_IOCTL_POST_RESET, NULL);
    ioctl(ITP_DEVICE_BACKLIGHT, ITP_IOCTL_ON, NULL);

    files = filelist_init();
    if (NULL == files)
        goto fail;
#ifdef WIN32
    filelist_add_files_from_all_drives(files, "264:h264;h264:h264;");
#else
    filelist_add(files, "a:/release.264");
#endif

    malloc_stats();
    printf("[%s] #### Begining test program #### \n", __FUNCTION__);
    VideoInit();

    sleep(8);

    avcodec_register_all(); // avcodec_init();
    codec = avcodec_find_decoder(CODEC_ID_H264);
    if (!codec)
    {
        MACRO_ERTN;
    }

    c               = avcodec_alloc_context3(codec); // avcodec_alloc_context();

#ifdef _DEBUG
    c->debug        = FF_DEBUG_PICT_INFO | FF_DEBUG_SKIP | FF_DEBUG_STARTCODE | FF_DEBUG_PTS | FF_DEBUG_MMCO | FF_DEBUG_BUGS | FF_DEBUG_BUFFERS | FF_DEBUG_THREADS;
#endif

    picture         = avcodec_alloc_frame();
    assert(picture);
    avcodec_get_frame_defaults(picture);
    picture->opaque = malloc(sizeof(uint32_t));

    if (codec->capabilities & CODEC_CAP_TRUNCATED)
        c->flags |= CODEC_FLAG_TRUNCATED;

    if (avcodec_open2(c, codec, &dict) /*avcodec_open(c, codec)*/ < 0)
    {
        MACRO_ERTN;
    }

#ifdef _DEBUG
    av_log_set_level(AV_LOG_DEBUG);
#endif

    printf("\nFFMPEG:\n");
    malloc_stats();

#if !SINGLE_PLAY
    while (1)
#endif
    {
        while (!files[file_idx])
        {
            file_idx++;
            file_idx %= MAX_FILE_INDEX;
        }
        play_file(files[file_idx], codec, c, picture, &avpkt);
    }

    /* maybe send a zero byte data to decoder? */
    avpkt.data = NULL;
    avpkt.size = 0;
    len        = avcodec_decode_video2(c, picture, &got_picture, &avpkt);
    if (got_picture)
    {
        display_on_LCD(c, picture, &got_picture, &avpkt);

        frame++;
        printf("[%s] #### got last frame ####\n", __FUNCTION__);
    }

    // ffmpeg de-initial
    avcodec_close(c);
    av_free(c);
    if (picture->opaque)
        free(picture->opaque);
    av_free(picture);
    if (files)
        filelist_deinit(files);
    //  printf("[%s] #### leaving test program, duration = %u ms ####\n", __FUNCTION__, PalGetDuration(exe_clock));

fail:
    VideoExit();
    itpExit();
    malloc_stats();
    return 0;
}
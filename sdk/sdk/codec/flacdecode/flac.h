/*
 * FLAC (Free Lossless Audio Codec) decoder/demuxer common functions
 * Copyright (c) 2008 Justin Ruggles
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
 * FLAC (Free Lossless Audio Codec) decoder/demuxer common functions
 */

#ifndef AVCODEC_FLAC_H
#define AVCODEC_FLAC_H

#include "get_bits.h"

#define FLAC_STREAMINFO_SIZE   34
#define FLAC_MAX_CHANNELS     2//  8
#define FLAC_MIN_BLOCKSIZE     16
#define FLAC_MAX_BLOCKSIZE  65535
#define FLAC_MIN_FRAME_SIZE    11
#define FLAC_MAX_FRAME_SIZE    FLAC_MAX_BLOCKSIZE
#define MKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((d) << 24))
#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((a) << 24))

#define FLAC_PERFORMANCE_TEST_BY_TICK

/**
 * all in native-endian format
 */
enum AVSampleFormat {
    AV_SAMPLE_FMT_NONE = -1,
    AV_SAMPLE_FMT_U8,          ///< unsigned 8 bits
    AV_SAMPLE_FMT_S16,         ///< signed 16 bits
    AV_SAMPLE_FMT_S32,         ///< signed 32 bits
    AV_SAMPLE_FMT_FLT,         ///< float
    AV_SAMPLE_FMT_DBL,         ///< double
    AV_SAMPLE_FMT_U8P,         ///< unsigned 8 bits, planar
    AV_SAMPLE_FMT_S16P,        ///< signed 16 bits, planar
    AV_SAMPLE_FMT_S32P,        ///< signed 32 bits, planar
    AV_SAMPLE_FMT_FLTP,        ///< float, planar
    AV_SAMPLE_FMT_DBLP,        ///< double, planar

    AV_SAMPLE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
};

enum {
    FLAC_CHMODE_INDEPENDENT =  0,
    FLAC_CHMODE_LEFT_SIDE   =  8,
    FLAC_CHMODE_RIGHT_SIDE  =  9,
    FLAC_CHMODE_MID_SIDE    = 10,
};

enum {
    FLAC_METADATA_TYPE_STREAMINFO = 0,
    FLAC_METADATA_TYPE_PADDING,
    FLAC_METADATA_TYPE_APPLICATION,
    FLAC_METADATA_TYPE_SEEKTABLE,
    FLAC_METADATA_TYPE_VORBIS_COMMENT,
    FLAC_METADATA_TYPE_CUESHEET,
    FLAC_METADATA_TYPE_PICTURE,
    FLAC_METADATA_TYPE_INVALID = 127
};

enum FLACExtradataFormat {
    FLAC_EXTRADATA_FORMAT_STREAMINFO  = 0,
    FLAC_EXTRADATA_FORMAT_FULL_HEADER = 1
};

/** largest possible size of flac header */
#define MAX_FRAME_HEADER_SIZE 16

#define FLACCOMMONINFO \
    int samplerate;         /**< sample rate                             */\
    int channels;           /**< number of channels                      */\
    int bps;                /**< bits-per-sample                         */\

/**
 * Data needed from the Streaminfo header for use by the raw FLAC demuxer
 * and/or the FLAC decoder.
 */
#define FLACSTREAMINFO \
    FLACCOMMONINFO \
    int max_blocksize;      /**< maximum block size, in samples          */\
    int max_framesize;      /**< maximum frame size, in bytes            */\
    int64_t samples;        /**< total number of samples                 */\

typedef struct FLACContext {
    FLACSTREAMINFO

    //AVCodecContext *avctx;                  ///< parent AVCodecContext
    //AVFrame frame;
    GetBitContext gb;                       ///< GetBitContext initialized to start at the current frame

    int blocksize;                          ///< number of samples in the current frame
    int curr_bps;                           ///< bps for current subframe, adjusted for channel correlation and wasted bits
    int sample_shift;                       ///< shift required to make output samples 16-bit or 32-bit
    int is32;                               ///< flag to indicate if output should be 32-bit instead of 16-bit
    int ch_mode;                            ///< channel decorrelation type in the current frame
    int got_streaminfo;                     ///< indicates if the STREAMINFO has been read
    int sample_fmt;
    int read_byte;
    int32_t decoded[FLAC_MAX_CHANNELS][FLAC_MAX_BLOCKSIZE];    ///< decoded samples
} FLACContext;

int av_log2(unsigned int v);

/**
 * Convert a UTF-8 character (up to 4 bytes) to its 32-bit UCS-4 encoded form.
 *
 * @param val      Output value, must be an lvalue of type uint32_t.
 * @param GET_BYTE Expression reading one byte from the input.
 *                 Evaluated up to 7 times (4 for the currently
 *                 assigned Unicode range).  With a memory buffer
 *                 input, this could be *ptr++.
 * @param ERROR    Expression to be evaluated on invalid input,
 *                 typically a goto statement.
 */
#define GET_UTF8(val, GET_BYTE, ERROR)\
    val= GET_BYTE;\
    {\
        int ones= 7 - av_log2(val ^ 255);\
        if(ones==1)\
            ERROR\
        val&= 127>>ones;\
        while(--ones > 0){\
            int tmp= GET_BYTE - 128;\
            if(tmp>>6)\
                ERROR\
            val= (val<<6) + tmp;\
        }\
    }

typedef struct FLACStreaminfo {
    FLACSTREAMINFO
} FLACStreaminfo;

typedef struct FLACFrameInfo {
    FLACCOMMONINFO
    int blocksize;          /**< block size of the frame                 */
    int ch_mode;            /**< channel decorrelation mode              */
    int64_t frame_or_sample_num;    /**< frame number or sample number   */
    int is_var_size;                /**< specifies if the stream uses variable
                                         block sizes or a fixed block size;
                                         also determines the meaning of
                                         frame_or_sample_num             */
} FLACFrameInfo;


int frame_header_is_valid(const uint8_t *buf,FLACFrameInfo *fi);

/**
 * Parse the Streaminfo metadata block
 * @param[out] avctx   codec context to set basic stream parameters
 * @param[out] s       where parsed information is stored
 * @param[in]  buffer  pointer to start of 34-byte streaminfo data
 */
void avpriv_flac_parse_streaminfo(struct FLACStreaminfo *s,const unsigned char *buffer);

/**
 * Validate the FLAC extradata.
 * @param[in]  avctx codec context containing the extradata.
 * @param[out] format extradata format.
 * @param[out] streaminfo_start pointer to start of 34-byte STREAMINFO data.
 * @return 1 if valid, 0 if not valid.
 */
int avpriv_flac_is_extradata_valid(//AVCodecContext *avctx,
                                   enum FLACExtradataFormat *format,
                                   unsigned char **streaminfo_start);

/**
 * Parse the metadata block parameters from the header.
 * @param[in]  block_header header data, at least 4 bytes
 * @param[out] last indicator for last metadata block
 * @param[out] type metadata block type
 * @param[out] size metadata block size
 */
void avpriv_flac_parse_block_header(const uint8_t *block_header,
                                    int *last, int *type, int *size);

/**
 * Calculate an estimate for the maximum frame size based on verbatim mode.
 * @param blocksize block size, in samples
 * @param ch number of channels
 * @param bps bits-per-sample
 */
int ff_flac_get_max_frame_size(int blocksize, int ch, int bps);

/**
 * Validate and decode a frame header.
 * @param      avctx AVCodecContext to use as av_log() context
 * @param      gb    GetBitContext from which to read frame header
 * @param[out] fi    frame information
 * @param      log_level_offset  log level offset. can be used to silence error messages.
 * @return non-zero on error, 0 if ok
 */
//int ff_flac_decode_frame_header(AVCodecContext *avctx, GetBitContext *gb,
//                                FLACFrameInfo *fi, int log_level_offset);
int ff_flac_decode_frame_header(GetBitContext *gb,
                                FLACFrameInfo *fi, int log_level_offset);

int flac_decode_frame(FLACContext *fctx, char *data,int size,
                             short* output);


#endif /* AVCODEC_FLAC_H */

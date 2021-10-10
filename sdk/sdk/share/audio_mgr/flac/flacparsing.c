/////////////////////////////////////////////////////////////////
//                      Include File
/////////////////////////////////////////////////////////////////
#include <stdio.h>

#include <string.h>


#include "config.h"
#include "get_bits.h"
//#include "flacmain.h"

/** largest possible size of flac header */
#define MAX_FRAME_HEADER_SIZE 16

#define FLAC_MAX_CHANNELS     2//  8

enum {
    FLAC_CHMODE_INDEPENDENT =  0,
    FLAC_CHMODE_LEFT_SIDE   =  8,
    FLAC_CHMODE_RIGHT_SIDE  =  9,
    FLAC_CHMODE_MID_SIDE    = 10,
};

#   define AV_RB32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
               (((const uint8_t*)(x))[1] << 16) |    \
               (((const uint8_t*)(x))[2] <<  8) |    \
                ((const uint8_t*)(x))[3])

#define MKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((a) << 24))


/////////////////////////////////////////////////////////////////
//                      Local Variable
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
//                      Global Variable
/////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////
//                      Global Function
/////////////////////////////////////////////////////////////////

int frame_header_is_valid(const uint8_t *buf)
{
    GetBitContext gb;
    init_get_bits(&gb, buf, MAX_FRAME_HEADER_SIZE * 8);
    return !flac_decode_frame_header(&gb, 127);
}

//int ff_flac_decode_frame_header(AVCodecContext *avctx, GetBitContext *gb,
//                                FLACFrameInfo *fi, int log_level_offset)
int flac_decode_frame_header(GetBitContext *gb,
                                int log_level_offset)
{
    int bs_code, sr_code, bps_code,is_var_size,ch_mode;
    int samplerate,channels,frame_or_sample_num,blocksize;
    

    /* frame sync code */
    if ((get_bits(gb, 15) & 0x7FFF) != 0x7FFC) {
        //printf("invalid sync code\n");
        return -1;
    }

    /* variable block size stream code */
    is_var_size = get_bits1(gb);

    /* block size and sample rate codes */
    bs_code = get_bits(gb, 4);
    sr_code = get_bits(gb, 4);

    /* channels and decorrelation */
    ch_mode = get_bits(gb, 4);
    if (ch_mode < FLAC_MAX_CHANNELS) {
        channels = ch_mode + 1;
        //if (fi->ch_mode <= 5)
        //    avctx->channel_layout = ff_vorbis_channel_layouts[fi->ch_mode];
        ch_mode = FLAC_CHMODE_INDEPENDENT;
    } else if (ch_mode <= FLAC_CHMODE_MID_SIDE) {
        channels = 2;
        //avctx->channel_layout = AV_CH_LAYOUT_STEREO;
    } else {
        printf("invalid channel mode: %d\n", ch_mode);
        return -1;
    }

    /* bits per sample */
    bps_code = get_bits(gb, 3);
    if (bps_code == 3 || bps_code == 7) {
        //printf("invalid sample size code (%d)\n",bps_code);
        return -1;
    }
    //fi->bps = sample_size_table[bps_code];

    /* reserved bit */
    if (get_bits1(gb)) {
       // printf("broken stream, invalid padding\n");
        return -1;
    }

    /* sample or frame count */
    /*frame_or_sample_num = get_utf8(gb);
    if (frame_or_sample_num < 0) {
        printf("sample/frame number invalid; utf8 fscked\n");
        return -1;
    }*/

    /* blocksize */
    if (bs_code == 0) {
        printf("reserved blocksize code: 0\n");
        return -1;
    } else if (bs_code == 6) {
        blocksize = get_bits(gb, 8) + 1;
    } else if (bs_code == 7) {
        blocksize = get_bits(gb, 16) + 1;
    } else {
        //fi->blocksize = ff_flac_blocksize_table[bs_code];
    }

    /* sample rate */
    if (sr_code < 12) {
        //fi->samplerate = ff_flac_sample_rate_table[sr_code];
    } else if (sr_code == 12) {
        samplerate = get_bits(gb, 8) * 1000;
    } else if (sr_code == 13) {
        samplerate = get_bits(gb, 16);
    } else if (sr_code == 14) {
        samplerate = get_bits(gb, 16) * 10;
    } else {
        printf("illegal sample rate code %d\n",sr_code);
        return -1;
    }

    /* header CRC-8 check */
    
    skip_bits(gb, 8);

    /*if (av_crc(av_crc_get_table(AV_CRC_8_ATM), 0, gb->buffer,
               get_bits_count(gb)/8)) {
        printf("header crc mismatch\n");
        return -1;
    }*/

    return 0;
}

int findFLACFrame(char* pBuf,int nLength,int nCheckFlac){
    int len = 0;
    unsigned int flacRIdx;
    int nTemp = 0;
    int frameLength;
    int keep;
    int duration =0;

    flacRIdx = 0;

    // check fLaC
    if (nCheckFlac){
        if (AV_RB32(pBuf) == MKBETAG('f','L','a','C') ){
            printf("fLaC \n");
        } else {
            // has not fLaC
            return -1;
        } 
    }   
    flacRIdx = 0;
    do {
        if (MAX_FRAME_HEADER_SIZE+flacRIdx<nLength) {

            if (((pBuf[flacRIdx]<<8|pBuf[flacRIdx+1]) & 0xFFFE) == 0xFFF8) {
                nTemp = frame_header_is_valid(&pBuf[flacRIdx]);
            }
        } else {
            // buffer has not valid header
            nTemp = 0;
            return -2;
        }
        flacRIdx++;

    } while(nTemp==0);

/*    duration = ((pBuf[22]<<24)&0xff000000)+ ((pBuf[23]<<16)&0x00ff0000)+((pBuf[24]<<8)&0x0000ff00)+((pBuf[25])&0x000000ff);
    if (tFlacInfo.samplerate)
        duration = duration/tFlacInfo.samplerate;
        
    smtkAudioMgrSetDuration(duration*1000);

    printf("[FLAC] ch %d , samplerate %d bits %d \n",tFlacInfo.channels,tFlacInfo.samplerate,tFlacInfo.bps);*/

    return flacRIdx--;
}




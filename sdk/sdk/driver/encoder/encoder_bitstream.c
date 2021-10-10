/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file encoder_bitstream.c
 *
 * @author
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "encoder/encoder_types.h"
#include "encoder/encoder_bitstream.h"
#include "encoder/config.h"
#include "encoder/encoder_types.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define Baseline_Profile_idc  0x42
#define AVC_Prefix_Hdr        0x00000001
#define SPS_NAL_TYPE          0x67
#define PPS_NAL_TYPE          0x68
#define AUD_NAL_TYPE          0x09
#define SEI_NAL_TYPE          0x06

//=============================================================================
//                              Macro Definition
//=============================================================================
#define BS_WAP(a) ((a) = ( ((a)&0xff)<<24) | (((a)&0xff00)<<8) | (((a)>>8)&0xff00) | (((a)>>24)&0xff))

//=============================================================================
//                              Structure Definition
//=============================================================================
typedef struct AVC_FRAME_RATE_PARA_TAG
{
    MMP_UINT32 FrameRateRes;
    MMP_UINT32 FrameRateDiv;
} AVC_FRAME_RATE_PARA;

static AVC_FRAME_RATE_PARA gFrameRate[16] =
{
    {    0,    0}, // 0: forbidden
    {   25,    1}, // 1: 25 fps
    {   50,    1}, // 2: 50 fps
    {   30,    1}, // 3: 30 fps
    {   60,    1}, // 4: 60 fps
    {30000, 1001}, // 5: 29.97 fps   
    {60000, 1001}, // 6: 59.94 fps
    {24000, 1001}, // 7: 23.976 fps
    {   24,    1}, // 8: 24 fps
    {    0,    0}, // 9: forbidden
    {    0,    0}, // 10: forbidden
    {    0,    0}, // 11: forbidden
    {    0,    0}, // 12: forbidden
    {    0,    0}, // 13: forbidden
    {   30,    1}, // 14: 30 fps
    {   60,    1}, // 15: 60 fps
     
};
//=============================================================================
//                              Global Data Definition
//=============================================================================

static const MMP_UINT32 STUFFING_CODES[8] =
{
	        /* nbits     stuffing code */
	1,		/* 1          1 */
	2,		/* 2          10 */
	4,		/* 3          100 */
	8,		/* 4          1000 */
	0x10,	/* 5          10000 */
	0x20,	/* 6          100000 */
	0x40,   /* 7          1000000 */
	0x80,	/* 8          10000000 */
};

static const MMP_UINT8 x264_ue_size_tab[256] =
{
     1, 1, 3, 3, 5, 5, 5, 5, 7, 7, 7, 7, 7, 7, 7, 7,
     9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
    11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
    15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
};

//=============================================================================
//                              Private Function Definition
//=============================================================================
static void BitstreamInit(BIT_STREAM *bs,
                   MMP_UINT32 *bitstream,
                   MMP_UINT32 dwStreamSize)
{
MMP_UINT32 tmp;
MMP_UINT32 streamAddr = 0;
MMP_UINT32 streamAddrAlign = 0;
MMP_UINT32 offset = 0;
    memset(bs, 0, sizeof(BIT_STREAM));
    streamAddr = (MMP_UINT32)bitstream;
    streamAddrAlign = ((streamAddr >> 2) << 2);
    offset = (streamAddr % 4);
	bs->start = bs->tail = (MMP_UINT32*)streamAddrAlign;
    if(offset != 0)
    {
        MMP_UINT8 stuff = 0;
        MMP_UINT32 i = 0;
        bs->bufa = 0;
        for(i = offset; i<4; i++)
        {
            stuff = (MMP_UINT8)(*((MMP_UINT8*)(streamAddrAlign + i)));
            bs->bufa |= (stuff << ((3 - i) * 8));
        }
        tmp = *(bs->start + 1);
	    bs->bufb = tmp;
    }
    else
    {
	    tmp = *bitstream;
	    bs->bufa = tmp;
	    tmp = *(bitstream + 1);
	    bs->bufb = tmp;
    }
	bs->buf = 0;
	bs->pos = offset * 8;
	bs->length = dwStreamSize + offset;
}

//-----< BitstreamForward >-----
static MMP_INLINE void BitstreamForward(BIT_STREAM *bs, 
                                        MMP_UINT32 bits)
{
    bs->pos += bits;
    
    if (bs->pos >= 32)
    {
		MMP_UINT32 b = bs->buf;
		
		//BS_WAP(b);
		*bs->tail++ = b;
		bs->buf = 0;
		bs->pos -= 32;
    }
}

//-----< BitstreamPutBits >-----
static MMP_INLINE void BitstreamPutBits(BIT_STREAM *bs, 
	                                    MMP_UINT32 value,
	                                    MMP_UINT32 size)
{
MMP_UINT32 shift = 32 - bs->pos - size;
    
	if (shift <= 32)
	{
		bs->buf |= value << shift;
		BitstreamForward(bs, size);
	}
	else
	{
		MMP_UINT32 remainder;
        
		shift = size - (32 - bs->pos);
		bs->buf |= value >> shift;
		BitstreamForward(bs, size - shift);
		remainder = shift;
        
		shift = 32 - shift;
		
		bs->buf |= value << shift;
		BitstreamForward(bs, remainder);
	}	
}

//-----< BitstreamPadAlways >-----
static MMP_INLINE void BitstreamPadAlways(BIT_STREAM *bs)
{
MMP_UINT32 bits = 8 - (bs->pos % 8);

	BitstreamPutBits(bs, STUFFING_CODES[bits - 1], bits);	
}

//-----< BitstreamPutBit >-----
static MMP_INLINE void BitstreamPutBit(BIT_STREAM *bs, 
                                       MMP_UINT32 bit)
{   
    if (bit)
    {
		bs->buf |= (0x80000000 >> bs->pos);
    }
    BitstreamForward(bs, 1);
}

//-----< BitstreamPos >-----
static MMP_INLINE MMP_UINT32 BitstreamPos(BIT_STREAM *bs)
{
    return (8 * ((MMP_UINT32)bs->tail - (MMP_UINT32)bs->start) + bs->pos);
}

//---------------< H.264 related function >-------------------------

static MMP_INLINE void bs_write_ue_big(BIT_STREAM *bs, 
                                       MMP_UINT32 value)
{
    MMP_UINT32 size = 0;
    MMP_UINT32 tmp = ++value;
    
    if( tmp >= 0x10000 )
    {
        size = 32;
        tmp >>= 16;
    }
    if( tmp >= 0x100 )
    {
        size += 16;
        tmp >>= 8;
    }
    size += x264_ue_size_tab[tmp];
    BitstreamPutBits( bs, 0, size>>1);
    BitstreamPutBits( bs, value, (size>>1)+1);
}

/* Only works on values under 255. */
static MMP_INLINE void bs_write_ue(BIT_STREAM *bs, 
                                   MMP_UINT32 value)
{
    BitstreamPutBits(bs, value+1, x264_ue_size_tab[value+1]);
}

static MMP_INLINE void bs_write_se(BIT_STREAM *bs, 
                                   MMP_INT32 value)
{
    MMP_UINT32 size = 0;
    /* Faster than (val <= 0 ? -value*2+1 : value*2) */
    /* 4 instructions on x86, 3 on ARM */
    MMP_INT32 tmp = 1 - value*2;
    
    if( tmp < 0 ) tmp = value*2;
    value = tmp;

    if( tmp >= 0x100 )
    {
        size = 16;
        tmp >>= 8;
    }
    size += x264_ue_size_tab[tmp];
    BitstreamPutBits(bs, value, size);
}

static MMP_INLINE void bs_rbsp_trailing(BIT_STREAM *bs)
{
    BitstreamPadAlways(bs);     
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
static void AddEmulationByte(BIT_STREAM *bs_src, BIT_STREAM *bs_dst)
{
MMP_UINT8*  buf = (MMP_UINT8 *) bs_src->start;
MMP_UINT32  size = BitstreamPos(bs_src) >> 3;
MMP_UINT32  i = 0;
MMP_UINT32  j = 0;  
MMP_UINT32  zero_count = 0;
MMP_UINT8   value;

    // flush source buffer data
    BitstreamPutBits(bs_src, 0x00, (4 -(size&0x3)) * 8);

    for (j = 0; j<size; j++)
    {
        value = buf[i++];
        
        if ((zero_count == 2) && (value == 0x00 || value == 0x01 || value == 0x02 || value == 0x03))
        {
            BitstreamPutBits(bs_dst, 0x03, 8);
            BitstreamPutBits(bs_dst, value, 8);
           
            zero_count = 0;
	        //BS_WAP(tmp);
        }
        else
        {
           BitstreamPutBits(bs_dst, value, 8);
	        //BS_WAP(tmp);
           ((value == 0x00) ? (zero_count++) : (zero_count = 0));          
	        //BS_WAP(tmp);
        }
    }   
}

void encoder_CreateIFrameHdr(AVC_ENCODER* ptEncoder,
                             MMP_UINT32 *bitstream,
                             MMP_UINT32 dwStreamSize)
{
    MMP_UINT32 tmp;
    MMP_UINT32 level_idc;
    BIT_STREAM bs, bs_sps, bs_pps, bs_sei;
    MMP_UINT32 bufSize = dwStreamSize >> 1;
    MMP_UINT32 *tmpbuf = (MMP_UINT32 *) ((MMP_UINT32) bitstream + bufSize);
    
    BitstreamInit(&bs, bitstream, bufSize);
    // Create AUD
    //BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    //BitstreamPutBits(&bs, AUD_NAL_TYPE, 8);
    //BitstreamPutBits(&bs, 0x50, 8);
    
    // Create SPS
    BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    BitstreamPutBits(&bs, SPS_NAL_TYPE, 8);
    BitstreamPutBits(&bs, Baseline_Profile_idc, 8);
    BitstreamInit(&bs_sps, tmpbuf, bufSize);
    BitstreamPutBit(&bs_sps, 0);        // constraint_set0
    BitstreamPutBit(&bs_sps, 1);        // constraint_set1
    BitstreamPutBit(&bs_sps, 0);        // constraint_set2
    BitstreamPutBit(&bs_sps, 0);        // constraint_set3
    BitstreamPutBits(&bs_sps, 0, 4);    // reserved
       
    tmp = (ptEncoder->frameWidth * ptEncoder->frameHeight) >> 8;      
    
    if (tmp <= 396)
    	level_idc = 20;
    else if (tmp <= 792)
    	level_idc = 21;
    else if (tmp <= 1620)
    	level_idc = 30;
    else if (tmp <= 3600)
    	level_idc = 31;	
    else if (tmp <= 5120)
    	level_idc = 32;
    else if (tmp <= 8192)
    	level_idc = 40;
    			
    BitstreamPutBits(&bs_sps, level_idc, 8);
    bs_write_ue(&bs_sps, 0);     // sps_id
       
    bs_write_ue(&bs_sps, 1);     // log2_max_frame_num
    bs_write_ue(&bs_sps, 2);     // poc_type
    
    bs_write_ue(&bs_sps, 1);     // num_ref_frames
    BitstreamPutBit(&bs_sps, 0); // gaps_in_frame_num_value_allowed   
    
    bs_write_ue(&bs_sps, ((ptEncoder->frameWidth+15) >> 4) - 1 ); // width_in_mbs_minus1    
    bs_write_ue(&bs_sps, ((ptEncoder->frameHeight+15) >> 4) - 1); // height_in_map_units_minus1
    
    BitstreamPutBit(&bs_sps, 1); // frame_mbs_only_flag    
    BitstreamPutBit(&bs_sps, 1); // direct8x8_inference_flag
    
    ptEncoder->frameCropLeft = 0;    
    ptEncoder->frameCropRight = (((ptEncoder->frameWidth+15)>>4)<<4) - ptEncoder->frameWidth;
    ptEncoder->frameCropTop = 0;
    ptEncoder->frameCropBottom = (((ptEncoder->frameHeight+15)>>4)<<4) - ptEncoder->frameHeight;
           
    if (ptEncoder->frameCropLeft > 0 || ptEncoder->frameCropRight > 0 ||
        ptEncoder->frameCropTop > 0 || ptEncoder->frameCropBottom > 0)
    {
        BitstreamPutBit(&bs_sps, 1); // frame_cropping_flag
        bs_write_ue(&bs_sps, ptEncoder->frameCropLeft>>1);
        bs_write_ue(&bs_sps, ptEncoder->frameCropRight>>1);
        bs_write_ue(&bs_sps, ptEncoder->frameCropTop>>1);
        bs_write_ue(&bs_sps, ptEncoder->frameCropBottom>>1);
        
    } 
    else 
    {
    	BitstreamPutBit(&bs_sps, 0); // frame_cropping_flag
    }
    
    // VUI parameters
#ifndef VUI_ON    
    BitstreamPutBit(&bs_sps, 0); // vui_parameters_present_flag
#else    
    BitstreamPutBit(&bs_sps, 1); // vui_parameters_present_flag
    BitstreamPutBit(&bs_sps, 0); // aspect_ratio_info_present_flag
    BitstreamPutBit(&bs_sps, 0); // overscan_info_present_flag
    BitstreamPutBit(&bs_sps, 0); // video_signal_type_present_flag
    BitstreamPutBit(&bs_sps, 0); // chroma_loc_info_present_flag
    BitstreamPutBit(&bs_sps, 1); // timing_info_present_flag       
           
    BitstreamPutBits(&bs_sps, gFrameRate[ptEncoder->frameRate].FrameRateDiv, 32);  // num_units_in_tick
    
    if (ptEncoder->frameRate == AVC_FRAME_RATE_29_97HZ ||
    	ptEncoder->frameRate == AVC_FRAME_RATE_59_94HZ ||
    	ptEncoder->frameRate == AVC_FRAME_RATE_23_97HZ)
    {    	
        gFrameRate[ptEncoder->frameRate].FrameRateRes = ptEncoder->EnFrameRate *1000;
    } else {    
        gFrameRate[ptEncoder->frameRate].FrameRateRes = ptEncoder->EnFrameRate;
    }
    
    BitstreamPutBits(&bs_sps, gFrameRate[ptEncoder->frameRate].FrameRateRes * 2, 32);           // time_scale        
    BitstreamPutBit(&bs_sps, 1); // fixed_frame_rate_flag
    
    BitstreamPutBit(&bs_sps, 1); // nal_hrd_parameters_present_flag
    //hrd_parameters() start
    bs_write_ue(&bs_sps, 0);         // cpb_cnt_minus1
    BitstreamPutBits(&bs_sps, 0, 4); // bit_rate_scale
    BitstreamPutBits(&bs_sps, 0, 4); // cpb_size_scale
    bs_write_ue(&bs_sps, 0);         // bit_rate_value_minus1
    bs_write_ue(&bs_sps, 0);         // cpb_size_value_minus1
    BitstreamPutBit(&bs_sps, 0);     // cbr_flag    
    BitstreamPutBits(&bs_sps, 0, 5); // initial_cpb_removal_delay_length_minus1
    BitstreamPutBits(&bs_sps, 0, 5); // cpb_removal_delay_length_minus1
    BitstreamPutBits(&bs_sps, 0, 5); // dpb_output_delay_length_minus1
    BitstreamPutBits(&bs_sps, 0, 5); // time_offset_length
    //hrd_parameters() end
    
    BitstreamPutBit(&bs_sps, 0); // vcl_hrd_parameters_present_flag    
    BitstreamPutBit(&bs_sps, 0); // low_delay_hrd_flag
    BitstreamPutBit(&bs_sps, 1); // pic_struct_present_flag
    BitstreamPutBit(&bs_sps, 0); // bitstream_restriction_flag
#endif    
    bs_rbsp_trailing(&bs_sps);
    
    AddEmulationByte(&bs_sps, &bs);
    // Create PPS        
    BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    BitstreamPutBits(&bs, PPS_NAL_TYPE, 8);
    BitstreamInit(&bs_pps, tmpbuf, bufSize);
    bs_write_ue(&bs_pps, 0);      // pic_parameter_set_id
    bs_write_ue(&bs_pps, 0);      // seq_parameter_set_id
    
    BitstreamPutBit(&bs_pps, 0);  // entropy_coding_mode_flag
    BitstreamPutBit(&bs_pps, 0);  // pic_order_present_flag
        
    bs_write_ue(&bs_pps, 0); // num_slice_groups_minus1
    bs_write_ue(&bs_pps, 0); // num_ref_idx_l0_active_minus1
    bs_write_ue(&bs_pps, 0); // num_ref_idx_l1_active_minus1
    
    BitstreamPutBit(&bs_pps, 0);  // weighted_pred_flag
    BitstreamPutBits(&bs_pps, 0, 2); // weighted_bipred_idc   

    bs_write_se(&bs_pps, 0); // pic_init_qp_minus26
    bs_write_se(&bs_pps, 0); // pic_init_qs_minus26
    
    tmp = ptEncoder->chromaQpOffset;
    if (tmp >= 16)
    	tmp = tmp - 32;
    bs_write_se(&bs_pps, tmp);
    
    if (ptEncoder->disableDeblk == 0 && (ptEncoder->deblkFilterOffsetBeta != 0 ||  
    	                                 ptEncoder->deblkFilterOffsetAlpha != 0))    
        BitstreamPutBit(&bs_pps, 1);  // deblocking_filter_control_present_flag
    else
    	BitstreamPutBit(&bs_pps, 0);  // deblocking_filter_control_present_flag
    
    if (ptEncoder->constrainedIntraPredFlag)
        BitstreamPutBit(&bs_pps, 1);  // constrained_intra_pred_flag
    else
    	BitstreamPutBit(&bs_pps, 0);  // constrained_intra_pred_flag
    	
    BitstreamPutBit(&bs_pps, 0);  // redundant_pic_cnt_present_flag

    bs_rbsp_trailing(&bs_pps);
    AddEmulationByte(&bs_pps, &bs);

#ifdef SEI_ON     
    // Create SEI
    BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    BitstreamPutBits(&bs, SEI_NAL_TYPE, 8);
    BitstreamInit(&bs_sei, tmpbuf, bufSize);
    BitstreamPutBits(&bs_sei, 1, 8); // pic_timing
    if (ptEncoder->interlaced_frame)
    {
        BitstreamPutBits(&bs_sei, 6, 8); // size
        BitstreamPutBit(&bs_sei, 0);     // cpb_removal_delay
        BitstreamPutBit(&bs_sei, 0);     // dpb_output_delay
        BitstreamPutBits(&bs_sei, 3, 4); // pic_struct  0: frame, 3 : top/bot field, in that order
        BitstreamPutBit(&bs_sei, 1);     // clock_timestamp_flag
        BitstreamPutBits(&bs_sei, 1, 2); // ct_type  0 : progressive  1 : interlaced
        BitstreamPutBit(&bs_sei, 0);     // nuit_field_based_flag
        BitstreamPutBits(&bs_sei, 0, 5); // counting_type
        BitstreamPutBit(&bs_sei, 0);     // full_timestamp_flag
        BitstreamPutBit(&bs_sei, 0);     // discontinuity_flag
        BitstreamPutBit(&bs_sei, 0);     // cnt_dropped_flag
        BitstreamPutBits(&bs_sei, 0, 8); // n_frames
        BitstreamPutBit(&bs_sei, 0);     // seconds_flag
        BitstreamPutBit(&bs_sei, 1);     // clock_timestamp_flag
        BitstreamPutBits(&bs_sei, 1, 2); // ct_type  0 : progressive  1 : interlaced
        BitstreamPutBit(&bs_sei, 0);     // nuit_field_based_flag
        BitstreamPutBits(&bs_sei, 0, 5); // counting_type
        BitstreamPutBit(&bs_sei, 0);     // full_timestamp_flag
        BitstreamPutBit(&bs_sei, 0);     // discontinuity_flag
        BitstreamPutBit(&bs_sei, 0);     // cnt_dropped_flag
        BitstreamPutBits(&bs_sei, 0, 8); // n_frames
        BitstreamPutBit(&bs_sei, 0);     // seconds_flag
    } 
    else
    {
    	BitstreamPutBits(&bs_sei, 4, 8); // size
    	BitstreamPutBit(&bs_sei, 0);     // cpb_removal_delay
        BitstreamPutBit(&bs_sei, 0);     // dpb_output_delay
        BitstreamPutBits(&bs_sei, 0, 4); // pic_struct  0: frame, 3 : top/bot field, in that order
        BitstreamPutBit(&bs_sei, 1);     // clock_timestamp_flag
        BitstreamPutBits(&bs_sei, 0, 2); // ct_type  0 : progressive  1 : interlaced
        BitstreamPutBit(&bs_sei, 0);     // nuit_field_based_flag
        BitstreamPutBits(&bs_sei, 0, 5); // counting_type
        BitstreamPutBit(&bs_sei, 0);     // full_timestamp_flag
        BitstreamPutBit(&bs_sei, 0);     // discontinuity_flag
        BitstreamPutBit(&bs_sei, 0);     // cnt_dropped_flag
        BitstreamPutBits(&bs_sei, 0, 8); // n_frames
        BitstreamPutBit(&bs_sei, 0);     // seconds_flag
        BitstreamPutBits(&bs_sei, 0, 5); // n_frames
    }
    bs_rbsp_trailing(&bs_sei);       
    
    AddEmulationByte(&bs_sei, &bs);
#endif    
    tmp = 8 - ((BitstreamPos(&bs) >> 3) & 0x7);  
    if (tmp != 8)
        BitstreamPutBits(&bs, 0, tmp*8);
    
    ptEncoder->ParaSetHdrSize[0] = BitstreamPos(&bs) >> 3;     
}

void encoder_CreatePFrameHdr(AVC_ENCODER* ptEncoder,
                             MMP_UINT32 *bitstream,
                             MMP_UINT32 dwStreamSize)                             
{
    MMP_UINT32 tmp;
    BIT_STREAM bs, bs_sei;
    MMP_UINT32 bufSize = dwStreamSize >> 1;
    MMP_UINT32 *tmpbuf = (MMP_UINT32 *) ((MMP_UINT32) bitstream + bufSize);
    
    BitstreamInit(&bs, bitstream, bufSize);
    // Create AUD
    //BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    //BitstreamPutBits(&bs, AUD_NAL_TYPE, 8);
    //BitstreamPutBits(&bs, 0x50, 8);

#ifdef SEI_ON             
    // Create SEI
    BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    BitstreamPutBits(&bs, SEI_NAL_TYPE, 8);
    BitstreamInit(&bs_sei, tmpbuf, bufSize);
    BitstreamPutBits(&bs_sei, 1, 8); // pic_timing
    if (ptEncoder->interlaced_frame)
    {
        BitstreamPutBits(&bs_sei, 6, 8); // size
        BitstreamPutBit(&bs_sei, 0);     // cpb_removal_delay
        BitstreamPutBit(&bs_sei, 0);     // dpb_output_delay
        BitstreamPutBits(&bs_sei, 3, 4); // pic_struct  0: frame, 3 : top/bot field, in that order
        BitstreamPutBit(&bs_sei, 1);     // clock_timestamp_flag
        BitstreamPutBits(&bs_sei, 1, 2); // ct_type  0 : progressive  1 : interlaced
        BitstreamPutBit(&bs_sei, 0);     // nuit_field_based_flag
        BitstreamPutBits(&bs_sei, 0, 5); // counting_type
        BitstreamPutBit(&bs_sei, 0);     // full_timestamp_flag
        BitstreamPutBit(&bs_sei, 0);     // discontinuity_flag
        BitstreamPutBit(&bs_sei, 0);     // cnt_dropped_flag
        BitstreamPutBits(&bs_sei, 0, 8); // n_frames
        BitstreamPutBit(&bs_sei, 0);     // seconds_flag
        BitstreamPutBit(&bs_sei, 1);     // clock_timestamp_flag
        BitstreamPutBits(&bs_sei, 1, 2); // ct_type  0 : progressive  1 : interlaced
        BitstreamPutBit(&bs_sei, 0);     // nuit_field_based_flag
        BitstreamPutBits(&bs_sei, 0, 5); // counting_type
        BitstreamPutBit(&bs_sei, 0);     // full_timestamp_flag
        BitstreamPutBit(&bs_sei, 0);     // discontinuity_flag
        BitstreamPutBit(&bs_sei, 0);     // cnt_dropped_flag
        BitstreamPutBits(&bs_sei, 0, 8); // n_frames
        BitstreamPutBit(&bs_sei, 0);     // seconds_flag
    } 
    else
    {
    	BitstreamPutBits(&bs_sei, 4, 8); // size
    	BitstreamPutBit(&bs_sei, 0);     // cpb_removal_delay
        BitstreamPutBit(&bs_sei, 0);     // dpb_output_delay
        BitstreamPutBits(&bs_sei, 0, 4); // pic_struct  0: frame, 3 : top/bot field, in that order
        BitstreamPutBit(&bs_sei, 1);     // clock_timestamp_flag
        BitstreamPutBits(&bs_sei, 0, 2); // ct_type  0 : progressive  1 : interlaced
        BitstreamPutBit(&bs_sei, 0);     // nuit_field_based_flag
        BitstreamPutBits(&bs_sei, 0, 5); // counting_type
        BitstreamPutBit(&bs_sei, 0);     // full_timestamp_flag
        BitstreamPutBit(&bs_sei, 0);     // discontinuity_flag
        BitstreamPutBit(&bs_sei, 0);     // cnt_dropped_flag
        BitstreamPutBits(&bs_sei, 0, 8); // n_frames
        BitstreamPutBit(&bs_sei, 0);     // seconds_flag
        BitstreamPutBits(&bs_sei, 0, 5); // n_frames
    }
    bs_rbsp_trailing(&bs_sei);       
    
    AddEmulationByte(&bs_sei, &bs);
#endif    
    tmp = 8 - ((BitstreamPos(&bs) >> 3) & 0x7);  
    if (tmp != 8)
        BitstreamPutBits(&bs, 0, tmp*8);
    
    ptEncoder->ParaSetHdrSize[1] = BitstreamPos(&bs) >> 3;  
}

void encoder_CreateMP4Config(AVC_ENCODER* ptEncoder,
                             MMP_UINT32 *bitstream,
                             MMP_UINT32 dwStreamSize)
{
    MMP_UINT32 tmp;
    MMP_UINT32 level_idc;
    BIT_STREAM bs, bs_sps, bs_pps, bs_sei;
    MMP_UINT32 bufSize = dwStreamSize >> 1;
    MMP_UINT32 *tmpbuf = (MMP_UINT32 *) ((MMP_UINT32) bitstream + bufSize);
    MMP_UINT8  *hdrbuf = (MMP_UINT8 *) bitstream;
    MMP_UINT32 SPS_Len, PPS_Len;
    
    //aligned(8) class AVCDecoderConfigurationRecord {
	//unsigned int(8) configurationVersion = 1;
	//unsigned int(8) AVCProfileIndication;
	//unsigned int(8) profile_compatibility;
	//unsigned int(8) AVCLevelIndication; 
	//bit(6) reserved = ．111111・b;
	//unsigned int(2) lengthSizeMinusOne; 
	//bit(3) reserved = ．111・b;
	//unsigned int(5) numOfSequenceParameterSets;
	//for (i=0; i< numOfSequenceParameterSets;  i++) {
	//	unsigned int(16) sequenceParameterSetLength ;
	//	bit(8*sequenceParameterSetLength) sequenceParameterSetNALUnit;
	//}
	//unsigned int(8) numOfPictureParameterSets;
	//for (i=0; i< numOfPictureParameterSets;  i++) {
	//	unsigned int(16) pictureParameterSetLength;
	//	bit(8*pictureParameterSetLength) pictureParameterSetNALUnit;
	//}
    //}

    BitstreamInit(&bs, bitstream, bufSize);
    BitstreamPutBits(&bs, 1, 8);                    // configurationVersion = 1;
    BitstreamPutBits(&bs, Baseline_Profile_idc, 8); // AVCProfileIndication
    BitstreamPutBits(&bs, 0x40, 8);                 // profile_compatibility
    
    tmp = (ptEncoder->frameWidth * ptEncoder->frameHeight) >> 8;      
    
    if (tmp <= 396)
    	level_idc = 20;
    else if (tmp <= 792)
    	level_idc = 21;
    else if (tmp <= 1620)
    	level_idc = 30;
    else if (tmp <= 3600)
    	level_idc = 31;	
    else if (tmp <= 5120)
    	level_idc = 32;
    else if (tmp <= 8192)
    	level_idc = 40;
    	
    BitstreamPutBits(&bs, level_idc, 8);         // AVCLevelIndication
    BitstreamPutBits(&bs, 63, 6);                // reserved = ．111111・b;
    BitstreamPutBits(&bs, 3, 2);                 // lengthSizeMinusOne = 4 - 1    
    
    BitstreamPutBits(&bs, 7, 3);                 // reserved = ．111・b;
    BitstreamPutBits(&bs, 1, 5);                 // numOfSequenceParameterSets
    
    BitstreamPutBits(&bs, 0, 16);                // SPS Len
    
    // Create SPS
    //BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    BitstreamPutBits(&bs, SPS_NAL_TYPE, 8);
    BitstreamPutBits(&bs, Baseline_Profile_idc, 8);
    BitstreamInit(&bs_sps, tmpbuf, bufSize);
    BitstreamPutBit(&bs_sps, 0);        // constraint_set0
    BitstreamPutBit(&bs_sps, 1);        // constraint_set1
    BitstreamPutBit(&bs_sps, 0);        // constraint_set2
    BitstreamPutBit(&bs_sps, 0);        // constraint_set3
    BitstreamPutBits(&bs_sps, 0, 4);    // reserved
       
    tmp = (ptEncoder->frameWidth * ptEncoder->frameHeight) >> 8;      
    
    if (tmp <= 396)
    	level_idc = 20;
    else if (tmp <= 792)
    	level_idc = 21;
    else if (tmp <= 1620)
    	level_idc = 30;
    else if (tmp <= 3600)
    	level_idc = 31;	
    else if (tmp <= 5120)
    	level_idc = 32;
    else if (tmp <= 8192)
    	level_idc = 40;
    			
    BitstreamPutBits(&bs_sps, level_idc, 8);
    bs_write_ue(&bs_sps, 0);     // sps_id
       
    bs_write_ue(&bs_sps, 1);     // log2_max_frame_num
    bs_write_ue(&bs_sps, 2);     // poc_type
    
    bs_write_ue(&bs_sps, 1);     // num_ref_frames
    BitstreamPutBit(&bs_sps, 0); // gaps_in_frame_num_value_allowed   
    
    bs_write_ue(&bs_sps, ((ptEncoder->frameWidth+15) >> 4) - 1 ); // width_in_mbs_minus1    
    bs_write_ue(&bs_sps, ((ptEncoder->frameHeight+15) >> 4) - 1); // height_in_map_units_minus1
    
    BitstreamPutBit(&bs_sps, 1); // frame_mbs_only_flag    
    BitstreamPutBit(&bs_sps, 1); // direct8x8_inference_flag
    
    ptEncoder->frameCropLeft = 0;    
    ptEncoder->frameCropRight = (((ptEncoder->frameWidth+15)>>4)<<4) - ptEncoder->frameWidth;
    ptEncoder->frameCropTop = 0;
    ptEncoder->frameCropBottom = (((ptEncoder->frameHeight+15)>>4)<<4) - ptEncoder->frameHeight;
           
    if (ptEncoder->frameCropLeft > 0 || ptEncoder->frameCropRight > 0 ||
        ptEncoder->frameCropTop > 0 || ptEncoder->frameCropBottom > 0)
    {
        BitstreamPutBit(&bs_sps, 1); // frame_cropping_flag
        bs_write_ue(&bs_sps, ptEncoder->frameCropLeft>>1);
        bs_write_ue(&bs_sps, ptEncoder->frameCropRight>>1);
        bs_write_ue(&bs_sps, ptEncoder->frameCropTop>>1);
        bs_write_ue(&bs_sps, ptEncoder->frameCropBottom>>1);
        
    } 
    else 
    {
    	BitstreamPutBit(&bs_sps, 0); // frame_cropping_flag
    }
    
    // VUI parameters
#ifndef VUI_ON    
    BitstreamPutBit(&bs_sps, 0); // vui_parameters_present_flag
#else    
    BitstreamPutBit(&bs_sps, 1); // vui_parameters_present_flag
    BitstreamPutBit(&bs_sps, 0); // aspect_ratio_info_present_flag
    BitstreamPutBit(&bs_sps, 0); // overscan_info_present_flag
    BitstreamPutBit(&bs_sps, 0); // video_signal_type_present_flag
    BitstreamPutBit(&bs_sps, 0); // chroma_loc_info_present_flag
    BitstreamPutBit(&bs_sps, 1); // timing_info_present_flag       

#ifdef ITURNE_TRICK
    if (ptEncoder->frameRate == 3)
    	ptEncoder->frameRate = 5;
#endif
       
    BitstreamPutBits(&bs_sps, gFrameRate[ptEncoder->frameRate].FrameRateDiv, 32);  // num_units_in_tick
    
    if (ptEncoder->frameRate == AVC_FRAME_RATE_29_97HZ ||
    	ptEncoder->frameRate == AVC_FRAME_RATE_59_94HZ ||
    	ptEncoder->frameRate == AVC_FRAME_RATE_23_97HZ)
    {    	
        gFrameRate[ptEncoder->frameRate].FrameRateRes = ptEncoder->EnFrameRate *1000;
    } else {    
        gFrameRate[ptEncoder->frameRate].FrameRateRes = ptEncoder->EnFrameRate;
    }
    
    BitstreamPutBits(&bs_sps, gFrameRate[ptEncoder->frameRate].FrameRateRes * 2, 32);           // time_scale        
    BitstreamPutBit(&bs_sps, 1); // fixed_frame_rate_flag
    
    BitstreamPutBit(&bs_sps, 1); // nal_hrd_parameters_present_flag
    //hrd_parameters() start
    bs_write_ue(&bs_sps, 0);         // cpb_cnt_minus1
    BitstreamPutBits(&bs_sps, 0, 4); // bit_rate_scale
    BitstreamPutBits(&bs_sps, 0, 4); // cpb_size_scale
    bs_write_ue(&bs_sps, 0);         // bit_rate_value_minus1
    bs_write_ue(&bs_sps, 0);         // cpb_size_value_minus1
    BitstreamPutBit(&bs_sps, 0);     // cbr_flag    
    BitstreamPutBits(&bs_sps, 0, 5); // initial_cpb_removal_delay_length_minus1
    BitstreamPutBits(&bs_sps, 0, 5); // cpb_removal_delay_length_minus1
    BitstreamPutBits(&bs_sps, 0, 5); // dpb_output_delay_length_minus1
    BitstreamPutBits(&bs_sps, 0, 5); // time_offset_length
    //hrd_parameters() end
    
    BitstreamPutBit(&bs_sps, 0); // vcl_hrd_parameters_present_flag    
    BitstreamPutBit(&bs_sps, 0); // low_delay_hrd_flag
    BitstreamPutBit(&bs_sps, 1); // pic_struct_present_flag
    BitstreamPutBit(&bs_sps, 0); // bitstream_restriction_flag
#endif    
    bs_rbsp_trailing(&bs_sps);
    
    AddEmulationByte(&bs_sps, &bs);
    
    // Len
    SPS_Len = BitstreamPos(&bs) >> 3;
   
    BitstreamPutBits(&bs, 1, 8);   // numOfPictureParameterSets
    BitstreamPutBits(&bs, 0, 16);  // PPS Len
    // Create PPS        
    //BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    BitstreamPutBits(&bs, PPS_NAL_TYPE, 8);
    BitstreamInit(&bs_pps, tmpbuf, bufSize);
    bs_write_ue(&bs_pps, 0);      // pic_parameter_set_id
    bs_write_ue(&bs_pps, 0);      // seq_parameter_set_id
    
    BitstreamPutBit(&bs_pps, 0);  // entropy_coding_mode_flag
    BitstreamPutBit(&bs_pps, 0);  // pic_order_present_flag
        
    bs_write_ue(&bs_pps, 0); // num_slice_groups_minus1
    bs_write_ue(&bs_pps, 0); // num_ref_idx_l0_active_minus1
    bs_write_ue(&bs_pps, 0); // num_ref_idx_l1_active_minus1
    
    BitstreamPutBit(&bs_pps, 0);  // weighted_pred_flag
    BitstreamPutBits(&bs_pps, 0, 2); // weighted_bipred_idc   

    bs_write_se(&bs_pps, 0); // pic_init_qp_minus26
    bs_write_se(&bs_pps, 0); // pic_init_qs_minus26
    
    tmp = ptEncoder->chromaQpOffset;
    if (tmp >= 16)
    	tmp = tmp - 32;
    bs_write_se(&bs_pps, tmp);
    
    if (ptEncoder->disableDeblk == 0 && (ptEncoder->deblkFilterOffsetBeta != 0 ||  
    	                                 ptEncoder->deblkFilterOffsetAlpha != 0))    
        BitstreamPutBit(&bs_pps, 1);  // deblocking_filter_control_present_flag
    else
    	BitstreamPutBit(&bs_pps, 0);  // deblocking_filter_control_present_flag
    
    if (ptEncoder->constrainedIntraPredFlag)
        BitstreamPutBit(&bs_pps, 1);  // constrained_intra_pred_flag
    else
    	BitstreamPutBit(&bs_pps, 0);  // constrained_intra_pred_flag
    	
    BitstreamPutBit(&bs_pps, 0);  // redundant_pic_cnt_present_flag

    bs_rbsp_trailing(&bs_pps);
    AddEmulationByte(&bs_pps, &bs);

    PPS_Len = BitstreamPos(&bs) >> 3;            
    
    //write SPS and SPS Len
    *(hdrbuf+6) = (SPS_Len - 8) >> 8;
    *(hdrbuf+7) = (SPS_Len - 8) & 0xFF;    
    *(hdrbuf+SPS_Len+1) = (PPS_Len - SPS_Len - 3) >> 8;
    *(hdrbuf+SPS_Len+2) = (PPS_Len - SPS_Len - 3) & 0xFF;
            
    ptEncoder->ParaSetHdrSize[0] = BitstreamPos(&bs) >> 3;
    BitstreamPutBits(&bs, 0x00, (4 -(ptEncoder->ParaSetHdrSize[0]&0x3)) * 8);
}

void encoder_CreateMP4SEI(AVC_ENCODER* ptEncoder,
                          MMP_UINT32 *bitstream,
                          MMP_UINT32 dwStreamSize)
{
    MMP_UINT32 tmp;
    MMP_UINT32 level_idc;
    BIT_STREAM bs, bs_sps, bs_pps, bs_sei;
    MMP_UINT32 bufSize = dwStreamSize >> 1;
    MMP_UINT32 *tmpbuf = (MMP_UINT32 *) ((MMP_UINT32) bitstream + bufSize);
    MMP_UINT8  *hdrbuf = (MMP_UINT8 *) bitstream;
    
    BitstreamInit(&bs, bitstream, bufSize);   
   
    // Create SEI
    //BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    BitstreamPutBits(&bs, 0, 16);
    BitstreamPutBits(&bs, SEI_NAL_TYPE, 8);
    BitstreamInit(&bs_sei, tmpbuf, bufSize);
    BitstreamPutBits(&bs_sei, 1, 8); // pic_timing
    if (ptEncoder->interlaced_frame)
    {
        BitstreamPutBits(&bs_sei, 6, 8); // size
        BitstreamPutBit(&bs_sei, 0);     // cpb_removal_delay
        BitstreamPutBit(&bs_sei, 0);     // dpb_output_delay
        BitstreamPutBits(&bs_sei, 3, 4); // pic_struct  0: frame, 3 : top/bot field, in that order
        BitstreamPutBit(&bs_sei, 1);     // clock_timestamp_flag
        BitstreamPutBits(&bs_sei, 1, 2); // ct_type  0 : progressive  1 : interlaced
        BitstreamPutBit(&bs_sei, 0);     // nuit_field_based_flag
        BitstreamPutBits(&bs_sei, 0, 5); // counting_type
        BitstreamPutBit(&bs_sei, 0);     // full_timestamp_flag
        BitstreamPutBit(&bs_sei, 0);     // discontinuity_flag
        BitstreamPutBit(&bs_sei, 0);     // cnt_dropped_flag
        BitstreamPutBits(&bs_sei, 0, 8); // n_frames
        BitstreamPutBit(&bs_sei, 0);     // seconds_flag
        BitstreamPutBit(&bs_sei, 1);     // clock_timestamp_flag
        BitstreamPutBits(&bs_sei, 1, 2); // ct_type  0 : progressive  1 : interlaced
        BitstreamPutBit(&bs_sei, 0);     // nuit_field_based_flag
        BitstreamPutBits(&bs_sei, 0, 5); // counting_type
        BitstreamPutBit(&bs_sei, 0);     // full_timestamp_flag
        BitstreamPutBit(&bs_sei, 0);     // discontinuity_flag
        BitstreamPutBit(&bs_sei, 0);     // cnt_dropped_flag
        BitstreamPutBits(&bs_sei, 0, 8); // n_frames
        BitstreamPutBit(&bs_sei, 0);     // seconds_flag
    } 
    else
    {
    	BitstreamPutBits(&bs_sei, 4, 8); // size
    	BitstreamPutBit(&bs_sei, 0);     // cpb_removal_delay
        BitstreamPutBit(&bs_sei, 0);     // dpb_output_delay
        BitstreamPutBits(&bs_sei, 0, 4); // pic_struct  0: frame, 3 : top/bot field, in that order
        BitstreamPutBit(&bs_sei, 1);     // clock_timestamp_flag
        BitstreamPutBits(&bs_sei, 0, 2); // ct_type  0 : progressive  1 : interlaced
        BitstreamPutBit(&bs_sei, 0);     // nuit_field_based_flag
        BitstreamPutBits(&bs_sei, 0, 5); // counting_type
        BitstreamPutBit(&bs_sei, 0);     // full_timestamp_flag
        BitstreamPutBit(&bs_sei, 0);     // discontinuity_flag
        BitstreamPutBit(&bs_sei, 0);     // cnt_dropped_flag
        BitstreamPutBits(&bs_sei, 0, 8); // n_frames
        BitstreamPutBit(&bs_sei, 0);     // seconds_flag
        BitstreamPutBits(&bs_sei, 0, 5); // n_frames
    }
    bs_rbsp_trailing(&bs_sei);
    
    AddEmulationByte(&bs_sei, &bs);
   
    //tmp = 8 - ((BitstreamPos(&bs) >> 3) & 0x7);
    //if (tmp != 8)
    //    BitstreamPutBits(&bs, 0, tmp*8);
    
    ptEncoder->ParaSetHdrSize[1] = BitstreamPos(&bs) >> 3;
    BitstreamPutBits(&bs, 0x00, (4 -(ptEncoder->ParaSetHdrSize[1]&0x3)) * 8);
    
    *(hdrbuf)   = (ptEncoder->ParaSetHdrSize[1] - 2) >> 8;
    *(hdrbuf+1) = (ptEncoder->ParaSetHdrSize[1] - 2) & 0xFF;
}

void encoder_CreateSPS(AVC_ENCODER* ptEncoder,
                       MMP_UINT32 *bitstream,
                       MMP_UINT32 dwStreamSize)
{
    MMP_UINT32 tmp;
    MMP_UINT32 level_idc;
    BIT_STREAM bs, bs_sps;
    MMP_UINT32 bufSize = dwStreamSize >> 1;
    MMP_UINT32 *tmpbuf = (MMP_UINT32 *) ((MMP_UINT32) bitstream + bufSize);
    
    BitstreamInit(&bs, bitstream, bufSize);
    // Create AUD
    //BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    //BitstreamPutBits(&bs, AUD_NAL_TYPE, 8);
    //BitstreamPutBits(&bs, 0x50, 8);
    
    // Create SPS
    BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    BitstreamPutBits(&bs, SPS_NAL_TYPE, 8);
    BitstreamPutBits(&bs, Baseline_Profile_idc, 8);
    BitstreamInit(&bs_sps, tmpbuf, bufSize);
    BitstreamPutBit(&bs_sps, 0);        // constraint_set0
    BitstreamPutBit(&bs_sps, 1);        // constraint_set1
    BitstreamPutBit(&bs_sps, 0);        // constraint_set2
    BitstreamPutBit(&bs_sps, 0);        // constraint_set3
    BitstreamPutBits(&bs_sps, 0, 4);    // reserved
       
    tmp = (ptEncoder->frameWidth * ptEncoder->frameHeight) >> 8;      
    
    if (tmp <= 396)
    	level_idc = 20;
    else if (tmp <= 792)
    	level_idc = 21;
    else if (tmp <= 1620)
    	level_idc = 30;
    else if (tmp <= 3600)
    	level_idc = 31;	
    else if (tmp <= 5120)
    	level_idc = 32;
    else if (tmp <= 8192)
    	level_idc = 40;
    			
    BitstreamPutBits(&bs_sps, level_idc, 8);
    bs_write_ue(&bs_sps, 0);     // sps_id
       
    bs_write_ue(&bs_sps, 1);     // log2_max_frame_num
    bs_write_ue(&bs_sps, 2);     // poc_type
    
    bs_write_ue(&bs_sps, 1);     // num_ref_frames
    BitstreamPutBit(&bs_sps, 0); // gaps_in_frame_num_value_allowed   
    
    bs_write_ue(&bs_sps, ((ptEncoder->frameWidth+15) >> 4) - 1 ); // width_in_mbs_minus1    
    bs_write_ue(&bs_sps, ((ptEncoder->frameHeight+15) >> 4) - 1); // height_in_map_units_minus1
    
    BitstreamPutBit(&bs_sps, 1); // frame_mbs_only_flag    
    BitstreamPutBit(&bs_sps, 1); // direct8x8_inference_flag
    
    ptEncoder->frameCropLeft = 0;    
    ptEncoder->frameCropRight = (((ptEncoder->frameWidth+15)>>4)<<4) - ptEncoder->frameWidth;
    ptEncoder->frameCropTop = 0;
    ptEncoder->frameCropBottom = (((ptEncoder->frameHeight+15)>>4)<<4) - ptEncoder->frameHeight;
           
    if (ptEncoder->frameCropLeft > 0 || ptEncoder->frameCropRight > 0 ||
        ptEncoder->frameCropTop > 0 || ptEncoder->frameCropBottom > 0)
    {
        BitstreamPutBit(&bs_sps, 1); // frame_cropping_flag
        bs_write_ue(&bs_sps, ptEncoder->frameCropLeft>>1);
        bs_write_ue(&bs_sps, ptEncoder->frameCropRight>>1);
        bs_write_ue(&bs_sps, ptEncoder->frameCropTop>>1);
        bs_write_ue(&bs_sps, ptEncoder->frameCropBottom>>1);
        
    } 
    else 
    {
    	BitstreamPutBit(&bs_sps, 0); // frame_cropping_flag
    }
    
    // VUI parameters
#ifndef VUI_ON    
    BitstreamPutBit(&bs_sps, 0); // vui_parameters_present_flag
#else    
    BitstreamPutBit(&bs_sps, 1); // vui_parameters_present_flag
    BitstreamPutBit(&bs_sps, 0); // aspect_ratio_info_present_flag
    BitstreamPutBit(&bs_sps, 0); // overscan_info_present_flag
    BitstreamPutBit(&bs_sps, 0); // video_signal_type_present_flag
    BitstreamPutBit(&bs_sps, 0); // chroma_loc_info_present_flag
    BitstreamPutBit(&bs_sps, 1); // timing_info_present_flag       
           
    BitstreamPutBits(&bs_sps, gFrameRate[ptEncoder->frameRate].FrameRateDiv, 32);  // num_units_in_tick
    
    if (ptEncoder->frameRate == AVC_FRAME_RATE_29_97HZ ||
    	ptEncoder->frameRate == AVC_FRAME_RATE_59_94HZ ||
    	ptEncoder->frameRate == AVC_FRAME_RATE_23_97HZ)
    {    	
        gFrameRate[ptEncoder->frameRate].FrameRateRes = ptEncoder->EnFrameRate *1000;
    } else {    
        gFrameRate[ptEncoder->frameRate].FrameRateRes = ptEncoder->EnFrameRate;
    }
    
    BitstreamPutBits(&bs_sps, gFrameRate[ptEncoder->frameRate].FrameRateRes * 2, 32);           // time_scale        
    BitstreamPutBit(&bs_sps, 1); // fixed_frame_rate_flag
    
    BitstreamPutBit(&bs_sps, 1); // nal_hrd_parameters_present_flag
    //hrd_parameters() start
    bs_write_ue(&bs_sps, 0);         // cpb_cnt_minus1
    BitstreamPutBits(&bs_sps, 0, 4); // bit_rate_scale
    BitstreamPutBits(&bs_sps, 0, 4); // cpb_size_scale
    bs_write_ue(&bs_sps, 0);         // bit_rate_value_minus1
    bs_write_ue(&bs_sps, 0);         // cpb_size_value_minus1
    BitstreamPutBit(&bs_sps, 0);     // cbr_flag    
    BitstreamPutBits(&bs_sps, 0, 5); // initial_cpb_removal_delay_length_minus1
    BitstreamPutBits(&bs_sps, 0, 5); // cpb_removal_delay_length_minus1
    BitstreamPutBits(&bs_sps, 0, 5); // dpb_output_delay_length_minus1
    BitstreamPutBits(&bs_sps, 0, 5); // time_offset_length
    //hrd_parameters() end
    
    BitstreamPutBit(&bs_sps, 0); // vcl_hrd_parameters_present_flag    
    BitstreamPutBit(&bs_sps, 0); // low_delay_hrd_flag
    BitstreamPutBit(&bs_sps, 1); // pic_struct_present_flag
    BitstreamPutBit(&bs_sps, 0); // bitstream_restriction_flag
#endif    
    bs_rbsp_trailing(&bs_sps);
    
    AddEmulationByte(&bs_sps, &bs);
    
    tmp = 8 - ((BitstreamPos(&bs) >> 3) & 0x7);  
    if (tmp != 8)
        BitstreamPutBits(&bs, 0, tmp*8);
    
    ptEncoder->ParaSetHdrSize[0] = BitstreamPos(&bs) >> 3;     
}

void encoder_CreatePPS(AVC_ENCODER* ptEncoder,
                       MMP_UINT32 *bitstream,
                       MMP_UINT32 dwStreamSize)
{
    MMP_UINT32 tmp;
    MMP_UINT32 level_idc;
    BIT_STREAM bs, bs_pps;
    MMP_UINT32 bufSize = dwStreamSize >> 1;
    MMP_UINT32 *tmpbuf = (MMP_UINT32 *) ((MMP_UINT32) bitstream + bufSize);
    
    BitstreamInit(&bs, bitstream, bufSize);
    // Create AUD
    //BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    //BitstreamPutBits(&bs, AUD_NAL_TYPE, 8);
    //BitstreamPutBits(&bs, 0x50, 8);
        
    // Create PPS        
    BitstreamPutBits(&bs, AVC_Prefix_Hdr, 32);
    BitstreamPutBits(&bs, PPS_NAL_TYPE, 8);
    BitstreamInit(&bs_pps, tmpbuf, bufSize);
    bs_write_ue(&bs_pps, 0);      // pic_parameter_set_id
    bs_write_ue(&bs_pps, 0);      // seq_parameter_set_id
    
    BitstreamPutBit(&bs_pps, 0);  // entropy_coding_mode_flag
    BitstreamPutBit(&bs_pps, 0);  // pic_order_present_flag
        
    bs_write_ue(&bs_pps, 0); // num_slice_groups_minus1
    bs_write_ue(&bs_pps, 0); // num_ref_idx_l0_active_minus1
    bs_write_ue(&bs_pps, 0); // num_ref_idx_l1_active_minus1
    
    BitstreamPutBit(&bs_pps, 0);  // weighted_pred_flag
    BitstreamPutBits(&bs_pps, 0, 2); // weighted_bipred_idc   

    bs_write_se(&bs_pps, 0); // pic_init_qp_minus26
    bs_write_se(&bs_pps, 0); // pic_init_qs_minus26
    
    tmp = ptEncoder->chromaQpOffset;
    if (tmp >= 16)
    	tmp = tmp - 32;
    bs_write_se(&bs_pps, tmp);
    
    if (ptEncoder->disableDeblk == 0 && (ptEncoder->deblkFilterOffsetBeta != 0 ||  
    	                                 ptEncoder->deblkFilterOffsetAlpha != 0))    
        BitstreamPutBit(&bs_pps, 1);  // deblocking_filter_control_present_flag
    else
    	BitstreamPutBit(&bs_pps, 0);  // deblocking_filter_control_present_flag
    
    if (ptEncoder->constrainedIntraPredFlag)
        BitstreamPutBit(&bs_pps, 1);  // constrained_intra_pred_flag
    else
    	BitstreamPutBit(&bs_pps, 0);  // constrained_intra_pred_flag
    	
    BitstreamPutBit(&bs_pps, 0);  // redundant_pic_cnt_present_flag

    bs_rbsp_trailing(&bs_pps);
    AddEmulationByte(&bs_pps, &bs);

    tmp = 8 - ((BitstreamPos(&bs) >> 3) & 0x7);  
    if (tmp != 8)
        BitstreamPutBits(&bs, 0, tmp*8);
    
    ptEncoder->ParaSetHdrSize[1] = BitstreamPos(&bs) >> 3;     
}
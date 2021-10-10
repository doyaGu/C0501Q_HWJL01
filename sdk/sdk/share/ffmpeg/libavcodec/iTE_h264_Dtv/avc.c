/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file avc.c
 *
 * @author
 */

//=============================================================================
//                              Include Files
//=============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "pal/pal.h"
#include "bitstream_kit.h"
#include "avc.h"
#include "ite/ith_video.h" //#include "mmp_video.h"
#include "video_decoder.h"
#include "avc_video_decoder.h"
//#include "dtv_dma.h"

#include "libavcodec/avcodec.h"
#include "libavcodec/h264.h"
#include "libavutil/opt.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//#define INT_MIN            -2147483647
//#define INT_MAX            2147483647
//#define NULL MMP_NULL

#define NALU_TYPE_SLICE    1
#define NALU_TYPE_DPA      2
#define NALU_TYPE_DPB      3
#define NALU_TYPE_DPC      4
#define NALU_TYPE_IDR      5
#define NALU_TYPE_SEI      6
#define NALU_TYPE_SPS      7
#define NALU_TYPE_PPS      8
#define NALU_TYPE_AUD      9
#define NALU_TYPE_EOSEQ    10
#define NALU_TYPE_EOSTREAM 11
#define NALU_TYPE_FILL     12

#define NALU_PRIORITY_HIGHEST     3
#define NALU_PRIORITY_HIGH        2
#define NALU_PRIRITY_LOW          1
#define NALU_PRIORITY_DISPOSABLE  0

#define YUV400 0
#define YUV420 1
#define YUV422 2
#define YUV444 3

#define MAX_NUM_SLICES 50
#define MAX_LIST_SIZE 33

#define MAX_MB_NUM_IN_SLICE     1944         // 864x576/(16x16)

#define NUM_SEQ_PARAM_SET       (32)
#define NUM_PIC_PARAM_SET       (256)

#define DUMMY_SEQ_PARAM_SET_ID          (NUM_SEQ_PARAM_SET)
#define DUMMY_PIC_PARAM_SET_ID          (NUM_PIC_PARAM_SET)

#define MAX_SLICE_HEADER_SIZE   (1024)
#define MAX_NUM_PREVENTION_BYTE (256 * 6)

#define MAX_CSB_SIZE                    (1024 * 1024)
#define SLICE_HEADER_SIZE               (8 * 15)
#define REFIDX_TO_PICIDX_TABLE_SIZE     (8 * 32)
#define PICIDX_TO_DIST_TABLE_SIZE       (8 * 160)
#define SCALING_TABLE_SIZE              (8 * 112)
#define WEIGHTING_TABLE_SIZE            (8 * 192)
#define CABAC_TABLE_SIZE                (8 * 64)

#define MAX_REFERENCE_FRAM_NUM          5
#define SLICE_HEADER_START              0
#define REFIDX_TO_PICIDX_TABLE_START    (SLICE_HEADER_START + SLICE_HEADER_SIZE)
#define PICIDX_TO_DIST_TABLE_START      (REFIDX_TO_PICIDX_TABLE_START + REFIDX_TO_PICIDX_TABLE_SIZE)
#define SCALING_TABLE_START             (PICIDX_TO_DIST_TABLE_START + PICIDX_TO_DIST_TABLE_SIZE)
#define WEIGHTING_TABLE_START           (SCALING_TABLE_START + SCALING_TABLE_SIZE)
#define CABAC_TABLE_START               (WEIGHTING_TABLE_START + WEIGHTING_TABLE_SIZE)
#define BITSTREAM_START                 (CABAC_TABLE_START + CABAC_TABLE_SIZE)

#define WAIT_REFERENCE_RELEASE 0x00000001
#define WAIT_FLIP_RELEASE      0x00010000
#define STORE_IN_DBP_BUF       0x00000100
#define STORE_IN_FLIP_QUEUE    0x01000000

//=============================================================================
//                              Macro Definition
//=============================================================================

#define max(a, b)      ((a) > (b) ? (a) : (b))  //!< Macro returning max value
#define min(a, b)      ((a) < (b) ? (a) : (b))  //!< Macro returning min value
#define Clip3(min,max,val) (((val)<(min))?(min):(((val)>(max))?(max):(val)))

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct EMULATION_PREVENTION_BYTE_TAG
{
    MMP_UINT32 number;
    MMP_UINT32 position[MAX_NUM_PREVENTION_BYTE];
} EMULATION_PREVENTION_BYTE;

typedef struct AVC_SEQUENCE_PARAMETER_SET_TAG
{
    MMP_UINT32 profile_idc;
    MMP_UINT32 constraint_set0_flag;
    MMP_UINT32 constraint_set1_flag;
    MMP_UINT32 constraint_set2_flag;
    MMP_UINT32 constraint_set3_flag;
    MMP_UINT32 reserved_zero_4bits;
    MMP_UINT32 level_idc;
    MMP_UINT32 seq_parameter_set_id;
    MMP_UINT32 chroma_format_idc;
    MMP_UINT32 residual_colour_transform_flag;
    MMP_UINT32 bit_depth_luma_minus8;
    MMP_UINT32 bit_depth_chroma_minus8;
    MMP_UINT32 qpprime_y_zero_transform_bypass_flag;
    MMP_UINT32 seq_scaling_matrix_present_flag;
    MMP_UINT32 seq_scaling_list_present_flag[8];
    MMP_UINT8  ScalingList4x4[6][16];
    MMP_UINT8  ScalingList8x8[2][64];
    MMP_UINT32 UseDefaultScalingMatrix4x4Flag[6];
    MMP_UINT32 UseDefaultScalingMatrix8x8Flag[2];
    MMP_UINT32 log2_max_frame_num_minus4;
    MMP_UINT32 pic_order_cnt_type;
    MMP_UINT32 log2_max_pic_order_cnt_lsb_minus4;
    MMP_UINT32 delta_pic_order_always_zero_flag;
    MMP_INT32 offset_for_non_ref_pic;
    MMP_INT32 offset_for_top_to_bottom_field;
    MMP_UINT32 num_ref_frames_in_pic_order_cnt_cycle;
    MMP_UINT32 offset_for_ref_frame[256];
    MMP_UINT32 num_ref_frames;
    MMP_UINT32 gaps_in_frame_num_value_allowed_flag;
    MMP_UINT32 pic_width_in_mbs_minus1;
    MMP_UINT32 pic_height_in_map_units_minus1;
    MMP_UINT32 frame_mbs_only_flag;
    MMP_UINT32 mb_adaptive_frame_field_flag;
    MMP_UINT32 direct_8x8_inference_flag;
    MMP_UINT32 frame_cropping_flag;
    MMP_UINT32 frame_crop_left_offset;
    MMP_UINT32 frame_crop_right_offset;
    MMP_UINT32 frame_crop_top_offset;
    MMP_UINT32 frame_crop_bottom_offset;
    MMP_UINT32 vui_parameters_present_flag;
    MMP_UINT32 aspect_ratio_info_present_flag;
    MMP_UINT32 aspect_ratio_idc;
    MMP_UINT32 timing_info_present_flag;
    MMP_UINT32 num_units_in_tick;
    MMP_UINT32 time_scale;
    MMP_UINT32 fixed_frame_rate_flag;
    MMP_UINT32 nal_hrd_parameters_present_flag;
    MMP_UINT32 vcl_hrd_parameters_present_flag;
    MMP_UINT32 nal_cpb_removal_delay_length_minus1;
    MMP_UINT32 nal_dpb_output_delay_length_minus1;
    MMP_UINT32 vcl_cpb_removal_delay_length_minus1;
    MMP_UINT32 vcl_dpb_output_delay_length_minus1;
    MMP_UINT32 pic_struct_present_flag;
} AVC_SEQUENCE_PARAMETER_SET;

typedef struct AVC_PICTURE_PARAMETER_SET_TAG
{
    MMP_UINT32 pic_parameter_set_id;
    MMP_UINT32 seq_parameter_set_id;
    MMP_UINT32 entropy_coding_mode_flag;
    MMP_UINT32 pic_order_present_flag;
    MMP_UINT32 num_slice_groups_minus1;
    MMP_UINT32 slice_group_map_type;
    MMP_UINT32 slice_group_change_direction_flag;
    MMP_UINT32 slice_group_change_rate_minus1;
    MMP_UINT32 pic_size_in_map_units_minus1;
    MMP_UINT32 num_ref_idx_l0_active_minus1;
    MMP_UINT32 num_ref_idx_l1_active_minus1;
    MMP_UINT32 weighted_pred_flag;
    MMP_UINT32 weighted_bipred_idc;
    MMP_INT32 pic_init_qp_minus26;
    MMP_INT32 pic_init_qs_minus26;
    MMP_INT32 chroma_qp_index_offset;
    MMP_UINT32 deblocking_filter_control_present_flag;
    MMP_UINT32 constrained_intra_pred_flag;
    MMP_UINT32 redundant_pic_cnt_present_flag;
    MMP_UINT32 transform_8x8_mode_flag;
    MMP_UINT32 pic_scaling_matrix_present_flag;
    MMP_UINT32 pic_scaling_list_present_flag[8];
    MMP_UINT8  ScalingList4x4[6][16];
    MMP_UINT8  ScalingList8x8[2][64];
    MMP_UINT32 UseDefaultScalingMatrix4x4Flag[6];
    MMP_UINT32 UseDefaultScalingMatrix8x8Flag[2];
    MMP_UINT32 second_chroma_qp_index_offset;
} AVC_PICTURE_PARAMETER_SET;

typedef struct AVC_REF_PIC_LIST_REORDERING_TAG
{
    MMP_UINT32 ref_pic_list_reordering_flag_l0;
    MMP_UINT32 ref_pic_list_reordering_flag_l1;
} AVC_REF_PIC_LIST_REORDERING;

typedef struct AVC_PRED_WEIGHT_TABLE_TAG
{
    MMP_UINT32 luma_log2_weight_denom;
    MMP_UINT32 chroma_log2_weight_denom;
} AVC_PRED_WEIGHT_TABLE;

typedef struct AVC_DEC_REF_PIC_MARKING_TAG
{
    MMP_UINT32 no_output_of_prior_pics_flag;
    MMP_UINT32 long_term_reference_flag;
    MMP_UINT32 adaptive_ref_pic_marking_mode_flag;
} AVC_DEC_REF_PIC_MARKING;

typedef struct AVC_SLICE_HEADER_TAG
{
    MMP_UINT32 first_mb_in_slice;
    MMP_UINT32 slice_type;
    MMP_UINT32 pic_parameter_set_id;
    MMP_UINT32 frame_num;
    MMP_UINT32 field_pic_flag;
    MMP_UINT32 bottom_field_flag;
    MMP_UINT32 idr_pic_id;
    MMP_UINT32 pic_order_cnt_lsb;
    MMP_UINT32 delta_pic_order_cnt_bottom;
    MMP_UINT32 delta_pic_order_cnt_0;
    MMP_UINT32 delta_pic_order_cnt_1;
    MMP_UINT32 redundant_pic_cnt;
    MMP_UINT32 direct_spatial_mv_pred_flag;
    MMP_UINT32 num_ref_idx_active_override_flag;
    MMP_UINT32 num_ref_idx_l0_active_minus1;
    MMP_UINT32 num_ref_idx_l1_active_minus1;
    MMP_UINT32 cabac_init_idc;
    MMP_UINT32 slice_qp_delta;
    MMP_UINT32 sp_for_switch_flag;
    /*
    The first bit to HWis from slice_qs_delta
    MMP_INT32 slice_qs_delta;
    MMP_UINT32 disable_deblocking_filter_idc;
    MMP_UINT32 slice_alpha_c0_offset_div2;
    MMP_INT32 slice_beta_offset_div2;
    MMP_INT32 slice_group_change_cycle;
    */
    AVC_REF_PIC_LIST_REORDERING ref_pic_list_reordering;
    AVC_PRED_WEIGHT_TABLE       pred_weight_table;
    AVC_DEC_REF_PIC_MARKING     dec_ref_pic_marking;
} AVC_SLICE_HEADER;

typedef struct AVC_SLICE_DATA_TAG
{
    MMP_UINT8 *pDataStart;              /**< Decode data start address  */
    MMP_UINT8 *pDataEnd;                /**< Decode data end address    */
    MMP_UINT32 dwStartBits;             /**< Decode data start bits     */
    MMP_UINT32 dwDataSize;              /**< Decode data size           */
} AVC_SLICE_DATA;

typedef struct AVC_DECODE_SLICE_TAG
{
    AVC_SEQUENCE_PARAMETER_SET seqParamSet;
    AVC_PICTURE_PARAMETER_SET  picParamSet;
    AVC_SLICE_HEADER           sliceHeader;
    AVC_SLICE_DATA             sliceData;
    MMP_UINT32                 last_mb_in_slice;
} AVC_DECODE_SLICE;

typedef struct SEQ_PARAM_SET_LIST_TAG
{
    MMP_UINT32                 activeParamSetId;
    AVC_SEQUENCE_PARAMETER_SET *pParamSet[NUM_SEQ_PARAM_SET];
} SEQ_PARAM_SET_LIST;

typedef struct PIC_PARAM_SET_LIST_TAG
{
    MMP_UINT32                activeParamSetId;
    AVC_PICTURE_PARAMETER_SET *pParamSet[NUM_PIC_PARAM_SET];
} PIC_PARAM_SET_LIST;

typedef struct PARAM_SET_LIST_TAG
{
    SEQ_PARAM_SET_LIST SeqParamSetList;
    PIC_PARAM_SET_LIST PicParamSetList;
} PARAMT_SET_LIS;

//! definition a picture (field or frame)
typedef struct storable_picture
{
    PictureStructure structure;
    int         poc;
    int         top_poc;
    int         bottom_poc;
    int         frame_poc;
    int         order_num;
    unsigned    frame_num;
    int         pic_num;
    int         long_term_pic_num;
    int         long_term_frame_idx;

    int         is_long_term;
    int         used_for_reference;
    int         is_output;
    int         non_existing;
    int         coded_frame;
    int         MbaffFrameFlag;

    struct storable_picture *top_field;     // for mb aff, if frame for referencing the top field
    struct storable_picture *bottom_field;  // for mb aff, if frame for referencing the bottom field
    struct storable_picture *frame;         // for mb aff, if field for referencing the combined frame

    int         slice_type;
    int         idr_flag;
    int         no_output_of_prior_pics_flag;
    int         long_term_reference_flag;
    int         adaptive_ref_pic_buffering_flag;

    DecRefPicMarking_t *dec_ref_pic_marking_buffer; //!< stores the memory management control operations

    // add for RefIdx2PicIdx, PicIdx2RefIdx table
    int         pic_idx;
    // add for decoding buffer & remapping buffer
    int         decoding_buf_idx;
    // add for col data buffer index
    int         col_data_buf_idx;
    int         pic_count;
} StorablePicture;

//! Frame Stores for Decoded Picture Buffer
typedef struct frame_store
{
    int       is_used;              //!< 0=empty; 1=top; 2=bottom; 3=both fields (or frame)
    int       is_reference;         //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
    int       is_long_term;         //!< 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used
    int       is_orig_reference;    //!< original marking by nal_ref_idc: 0=not used for ref; 1=top used; 2=bottom used; 3=both fields (or frame) used

    int       is_non_existent;

    unsigned  frame_num;
    int       frame_num_wrap;
    int       long_term_frame_idx;
    int       is_output;
    int       poc;

    int       decoding_buf_idx;
    int       pic_idx;
    int       pic_count;

    StorablePicture *frame;
    StorablePicture *top_field;
    StorablePicture *bottom_field;
} FrameStore;

//! Decoded Picture Buffer
typedef struct decoded_picture_buffer
{
    FrameStore  **fs;
    FrameStore  **fs_ref;
    FrameStore  **fs_ltref;
    unsigned      size;
    unsigned      used_size;
    unsigned      ref_frames_in_buffer;
    unsigned      ltref_frames_in_buffer;
    int           last_output_poc;
    int           max_long_term_pic_idx;

    int           init_done;
    int           num_ref_frames;

    FrameStore   *last_picture;
} DecodedPictureBuffer;

typedef struct {
    int col_data_placement; // 0 = a chunk, 1 = two chunk

    int col_data_use_which_chunk;
    int col_is_longterm;

    int ref_idx_to_pic_idx_mapping[2][32]; // for mapping the ref_idx to poc for colocated data
    int col_pic_idx_to_ref_idx_mapping[3][64]; // mapping the poc to DistScaleFactor
    int col_ref_idx_to_dist_scale_mapping[3][32];
    //  int col_data_bottom_offset;

    int luma_log2_weight_denom;
    int chroma_log2_weight_denom;

    int luma_weight[2][32]; // [2] for L0/L1
    int luma_offset[2][32];
    int chroma_weight_Cb[2][32];
    int chroma_offset_Cb[2][32];
    int chroma_weight_Cr[2][32];
    int chroma_offset_Cr[2][32];

    int remapping_ref_idx_l0[32];
    int remapping_ref_idx_l1[32];

    int poc;
    int top_poc;
    int bottom_poc;

    MMP_UINT32 dwStartBits;
    MMP_UINT8* pDataStart;
    MMP_UINT8* pDataEnd;
    MMP_UINT32 dwDataSize;
} sliceParametersDef;

typedef struct {
    unsigned char frameWidthMB;                            // A
    unsigned char frameHeightMB;                           // B
    unsigned char lumaPitch8_l;                            // C
    unsigned char lumaPitch8_h;                            // C
    unsigned char chromaPitch8_l;                          // D
    unsigned char chromaPitch8_h;                          // D
    unsigned char videoFormat;                             // E
    unsigned char pictureStructure;                        // F
    unsigned char direct_8x8_inference_flag;               // G
    unsigned char chroma_format_idc;                       // H
    unsigned char entropy_coding_mode_flag;                // I
    unsigned char weighted_pred_mode;                      // J
    unsigned char pic_init_qp_minus26;                     // K
    unsigned char chroma_qp_index_offset;                  // L
    unsigned char second_chroma_qp_index_offset;           // M
    unsigned char deblocking_filter_control_present_flag;  // N
    unsigned char constrained_intra_pred_flag;             // O
    unsigned char transform_8x8_mode_flag;                 // P
    unsigned char sliceType;                               // Q
    unsigned char direct_spatial_mv_pred_flag;             // R
    unsigned char num_ref_idx_l0_active_minus1;            // S
    unsigned char num_ref_idx_l1_active_minus1;            // T
    unsigned char luma_log2_weight_denom;                  // U
    unsigned char chroma_log2_weight_denom;                // V
    unsigned char colPicIsLongterm;                        // W
    unsigned char colDataPlacement;                        // X
    unsigned char colDataUseWhichChunk;                    // Y
    unsigned char first_MB_in_slice_X;                     // Z
    unsigned char first_MB_in_slice_Y;                     // AA
    unsigned char decodeBufferSelector;                    // AB
    unsigned char remapping_ref_idx_l0[32];                // AC000 ~ AC031
    unsigned char remapping_ref_idx_l1[32];                // AC100 ~ AC131
    unsigned char last_MB_in_slice_X;                      // AD
    unsigned char last_MB_in_slice_Y;                      // AE
    unsigned char discardBits;                             // AF
    unsigned char lastBytePosition;                        // AG
    unsigned char frame_poc_l;                             // AH
    unsigned char frame_poc_h;                             // AH
    unsigned char top_poc_l;                               // AI
    unsigned char top_poc_h;                               // AI
    unsigned char bottom_poc_l;                            // AJ
    unsigned char bottom_poc_h;                            // AJ
    unsigned char directWriteColDataBufferBase8_l;         // AK
    unsigned char directWriteColDataBufferBase8_m;         // AK
    unsigned char directWriteColDataBufferBase8_h;         // AK
    unsigned char stuff1;
    unsigned char directReadColDataBufferBase8_l;          // AL
    unsigned char directReadColDataBufferBase8_m;          // AL
    unsigned char directReadColDataBufferBase8_h;          // AL
    unsigned char stuff2;
    unsigned char nextSliceHeaderDataBase8_l;              // AM
    unsigned char nextSliceHeaderDataBase8_m;              // AM
    unsigned char nextSliceHeaderDataBase8_h;              // AM
    unsigned char stuff3;
    unsigned char tableInsertFlag;                         // AN
    unsigned char stuff4;
    unsigned char stuff5;
    unsigned char stuff6;
} SliceParameters;

MMP_UINT8 quant_intra_default[16] = {
     6,13,20,28,
    13,20,28,32,
    20,28,32,37,
    28,32,37,42
};

MMP_UINT8 quant_inter_default[16] = {
    10,14,20,24,
    14,20,24,27,
    20,24,27,30,
    24,27,30,34
};

MMP_UINT8 quant8_intra_default[64] = {
     6,10,13,16,18,23,25,27,
    10,11,16,18,23,25,27,29,
    13,16,18,23,25,27,29,31,
    16,18,23,25,27,29,31,33,
    18,23,25,27,29,31,33,36,
    23,25,27,29,31,33,36,38,
    25,27,29,31,33,36,38,40,
    27,29,31,33,36,38,40,42
};

MMP_UINT8 quant8_inter_default[64] = {
     9,13,15,17,19,21,22,24,
    13,13,17,19,21,22,24,25,
    15,17,19,21,22,24,25,27,
    17,19,21,22,24,25,27,28,
    19,21,22,24,25,27,28,30,
    21,22,24,25,27,28,30,32,
    22,24,25,27,28,30,32,33,
    24,25,27,28,30,32,33,35
};

MMP_UINT8 quant_org[16] = { //to be use if no q matrix is chosen
    16,16,16,16,
    16,16,16,16,
    16,16,16,16,
    16,16,16,16
};

MMP_UINT8 quant8_org[64] = { //to be use if no q matrix is chosen
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16,
    16,16,16,16,16,16,16,16
};

static MMP_UINT8 ZZ_SCAN[16]  =
{
    0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

static MMP_UINT8 ZZ_SCAN8[64] =
{   0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

short cabac_init_vars[460*8] = {
 -11,   87,   -6,   78,  -13,   91,  -12,   88,
  15,    9,    2,   40,   10,   28,    0,   42,
  -3,   77,   -1,   75,   -9,   89,   -5,   82,
   8,   25,    3,   44,   10,   31,    7,   34,
  -5,   71,   -7,   77,  -14,   92,   -3,   72,
  13,   18,    0,   49,   33,  -11,   11,   29,
  -4,   63,    2,   54,   -8,   76,   -4,   67,
  15,    9,    0,   46,   52,  -43,    8,   31,
  -4,   68,    5,   50,  -12,   87,   -8,   72,
  13,   19,    2,   44,   18,   15,    6,   37,
 -12,   84,   -3,   68,  -23,  110,  -16,   89,
  10,   37,    2,   51,   28,    0,    7,   42,
  -7,   62,    1,   50,  -24,  105,   -9,   69,
  12,   18,    0,   47,   35,  -22,    3,   40,
  -7,   65,    6,   42,  -10,   78,   -1,   59,
   6,   29,    4,   39,   38,  -25,    8,   33,
   8,   61,   -4,   81,  -20,  112,    5,   66,
  20,   33,    2,   62,   34,    0,   13,   43,
   5,   56,    1,   63,  -17,   99,    4,   57,
  15,   30,    6,   46,   39,  -18,   13,   36,
  -2,   66,   -4,   70,  -78,  127,   -4,   71,
   4,   45,    0,   54,   32,  -12,    4,   47,
   1,   64,    0,   67,  -70,  127,   -2,   71,
   1,   58,    3,   54,  102,  -94,    3,   55,
   0,   61,    2,   57,  -50,  127,    2,   58,
   0,   62,    2,   58,    0,    0,    2,   58,
  -2,   78,   -2,   76,  -46,  127,   -1,   74,
   7,   61,    4,   63,   56,  -15,    6,   60,
   0,   64,   12,   49,    9,   50,    6,   57,
   0,   64,   -4,   73,   -3,   70,  -17,   73,
   0,   64,   17,   50,   10,   54,   14,   57,
// 0,    0,    0,    0,    0,    0,    0,    0,
   7,   52,    4,   64,   -5,   78,   -1,   69,
  11,   45,    6,   57,   29,   10,   11,   44,
  10,   35,    1,   61,   -4,   71,    0,   62,
  15,   39,    7,   53,   37,   -5,   14,   42,
   0,   44,   11,   35,   -8,   72,   -7,   51,
  11,   42,    6,   52,   51,  -29,    7,   48,
  11,   38,   18,   25,    2,   59,   -4,   47,
  13,   44,    6,   55,   39,   -9,    4,   56,
   1,   45,   12,   24,   -1,   55,   -6,   42,
  16,   45,   11,   45,   52,  -34,    4,   52,
   0,   46,   13,   29,   -7,   70,   -3,   41,
  12,   41,   14,   36,   69,  -58,   13,   37,
   5,   44,   13,   36,   -6,   75,   -6,   53,
  10,   49,    8,   53,   67,  -63,    9,   49,
  31,   17,  -10,   93,   -8,   89,    8,   76,
  30,   34,   -1,   82,   44,   -5,   19,   58,
   1,   51,   -7,   73,  -34,  119,   -9,   78,
  18,   42,    7,   55,   32,    7,   10,   48,
   7,   50,   -2,   73,   -3,   75,  -11,   83,
  10,   55,   -3,   78,   55,  -29,   12,   45,
  28,   19,   13,   46,   32,   20,    9,   52,
  17,   51,   15,   46,   32,    1,    0,   69,
  16,   33,    9,   49,   30,   22,    0,   67,
  17,   46,   22,   31,    0,    0,   20,   33,
  14,   62,   -7,  100,  -44,  127,   -5,   90,
   0,   89,   -1,   84,   27,   36,    8,   63,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
   0,   64,   23,   33,   22,   25,   29,   16,
   0,   64,   23,    2,   34,    0,   25,    0,
   0,   64,   21,    0,   16,    0,   14,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
 -15,  100,    2,   53,   -5,   61,  -15,   72,
  22,  -17,   30,   -7,   34,  -30,   33,  -25,
 -13,  101,    5,   53,    0,   58,   -5,   75,
  26,  -17,   28,    3,   36,  -28,   28,   -3,
 -13,   91,   -2,   61,   -1,   60,   -8,   80,
  30,  -25,   28,    4,   38,  -28,   24,   10,
 -12,   94,    0,   56,   -3,   61,  -21,   83,
  28,  -20,   32,    0,   38,  -27,   27,    0,
 -10,   88,    0,   56,   -8,   67,  -21,   64,
  33,  -23,   34,   -1,   34,  -18,   34,  -14,
 -16,   84,  -13,   63,  -25,   84,  -13,   31,
  37,  -27,   30,    6,   35,  -16,   52,  -44,
 -10,   86,   -5,   60,  -14,   74,  -25,   64,
  33,  -23,   30,    6,   34,  -14,   39,  -24,
  -7,   83,   -1,   62,   -5,   65,  -29,   94,
  40,  -28,   32,    9,   32,   -8,   19,   17,
 -13,   87,    4,   57,    5,   52,    9,   75,
  38,  -17,   31,   19,   37,   -6,   31,   25,
 -19,   94,   -6,   69,    2,   57,   17,   63,
  33,  -11,   26,   27,   35,    0,   36,   29,
   1,   70,    4,   57,    0,   61,   -8,   74,
  40,  -15,   26,   30,   30,   10,   24,   33,
   0,   72,   14,   39,   -9,   69,   -5,   35,
  41,   -6,   37,   20,   28,   18,   34,   15,
  -5,   74,    4,   51,  -11,   70,   -2,   27,
  38,    1,   28,   34,   26,   25,   30,   20,
  18,   59,   13,   68,   18,   55,   13,   91,
  41,   17,   17,   70,   29,   41,   22,   73,
  13,   41,   13,   41,   13,   41,   13,   41,
// 0,    0,    0,    0,    0,    0,    0,    0,
   3,   62,    3,   62,    3,   62,    3,   62,
// 0,    0,    0,    0,    0,    0,    0,    0,
 -15,  100,    1,   61,    0,   58,   -7,   69,
  27,    3,    5,   59,    2,   72,   19,   31,
   0,   95,    9,   63,    7,   61,    8,   77,
  26,   22,    9,   67,    8,   77,   27,   44,
   0,   41,    0,   41,    0,   41,    0,   41,
   0,   63,    0,   63,    0,   63,    0,   63,
   0,   63,    0,   63,    0,   63,    0,   63,
   0,   63,    0,   63,    0,   63,    0,   63,
 -17,  123,   -7,   92,    0,   80,   11,   80,
 -12,  115,   -5,   89,   -5,   89,    5,   76,
 -16,  122,   -7,   96,   -7,   94,    2,   84,
 -11,  115,  -13,  108,   -4,   92,    5,   78,
  -7,   93,   -2,   85,  -13,  103,   -4,   86,
  24,    0,   11,   28,    4,   45,    4,   39,
  -6,   93,  -13,  106,  -21,  126,  -22,  127,
  15,    6,   14,   11,   19,   -6,   17,  -13,
 -12,   63,   -3,   46,    0,   39,   -6,   55,
  -2,   68,   -1,   65,    0,   65,    4,   61,
 -15,   84,   -1,   57,  -15,   84,  -14,   83,
 -13,  104,   -9,   93,  -35,  127,  -37,  127,
   1,   50,   11,   35,   -4,   66,   -4,   44,
  12,   38,    6,   51,   33,   -4,    8,   44,
  10,   44,   -9,   93,  -17,  111,  -13,  106,
  24,   17,   45,  -24,   69,  -71,   61,  -55,
   0,   64,   -3,   69,   -2,   69,  -11,   89,
   0,   64,   -6,   81,   -5,   82,  -15,  103,
   0,   64,  -11,   96,  -10,   96,  -21,  116,
   0,   64,    6,   55,    2,   59,   19,   57,
   0,   64,    7,   67,    2,   75,   20,   58,
   0,   64,   -5,   86,   -3,   87,    4,   84,
   0,   64,    2,   88,   -3,  100,    6,   96,
// 0,    0,    0,    0,    0,    0,    0,    0,
   2,   72,   16,   39,   18,   25,    3,   62,
  35,   -4,   18,   32,   18,   31,   15,   36,
 -11,   75,    5,   44,    9,   32,   -3,   68,
  38,   -8,   18,   35,   17,   35,   15,   36,
  -3,   71,    4,   52,    5,   43,  -20,   81,
  38,   -3,   22,   29,   21,   30,   21,   28,
  15,   46,   11,   48,    9,   47,    0,   30,
  37,    3,   24,   31,   17,   45,   25,   21,
 -13,   69,   -5,   60,    0,   44,    1,    7,
  38,    5,   23,   38,   20,   42,   30,   20,
   0,   62,   -1,   59,    0,   51,   -3,   23,
  42,    0,   18,   43,   18,   45,   31,   12,
   0,   65,    0,   59,    2,   46,  -21,   74,
  35,   16,   20,   41,   27,   26,   27,   16,
  21,   37,   22,   33,   19,   38,   16,   66,
  39,   22,   11,   63,   16,   54,   24,   42,
 -15,   72,    5,   44,   -4,   66,  -23,  124,
  14,   48,    9,   59,    7,   66,    0,   93,
   9,   57,   14,   43,   15,   38,   17,   37,
  27,   37,    9,   64,   16,   56,   14,   56,
  16,   54,   -1,   78,   12,   42,   44,  -18,
  21,   60,   -1,   94,   11,   73,   15,   57,
   0,   62,    0,   60,    9,   34,   50,  -34,
  12,   68,   -2,   89,   10,   67,   26,   38,
  12,   72,    9,   69,    0,   89,  -22,  127,
   2,   97,   -9,  108,  -10,  116,  -24,  127,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
  31,   21,   12,   40,   25,   32,   21,   33,
  31,   31,   11,   51,   21,   49,   19,   50,
  25,   50,   14,   59,   21,   54,   17,   61,
// 0,    0,    0,    0,    0,    0,    0,    0,
  40,  -15,   33,   -9,   42,   -9,   31,   -4,
 -26,   71,  -19,   73,   -4,   74,  -11,   63,
 -15,   81,  -12,   69,  -10,   83,   -5,   70,
 -14,   80,  -16,   70,   -9,   71,  -17,   75,
   0,   68,  -15,   67,   -9,   67,  -14,   72,
 -14,   70,  -20,   62,   -1,   61,  -16,   67,
 -24,   56,  -19,   70,   -8,   66,   -8,   53,
 -23,   68,  -16,   66,  -14,   66,  -14,   59,
  49,  -14,   39,   -7,   49,   -5,   33,   -1,
  44,    3,   41,   -2,   53,    0,   33,    7,
  45,    6,   45,    3,   64,    3,   31,   12,
  44,   34,   49,    9,   68,   10,   37,   23,
  33,   54,   45,   27,   66,   27,   31,   38,
  19,   82,   36,   59,   47,   57,   20,   64,
 -24,   50,  -22,   65,    0,   59,   -9,   52,
 -11,   74,  -20,   63,    2,   59,  -11,   68,
  23,  -13,    9,   -2,   17,  -10,    9,   -2,
  26,  -13,   26,   -9,   32,  -13,   30,  -10,
 -17,  120,   -4,   79,   -5,   85,   -3,   78,
 -20,  112,   -7,   71,   -6,   81,   -8,   74,
 -18,  114,   -5,   69,  -10,   77,   -9,   72,
 -11,   85,   -9,   70,   -7,   81,  -10,   72,
 -15,   92,   -8,   66,  -17,   80,  -18,   75,
 -14,   89,  -10,   68,  -18,   73,  -12,   71,
  20,  -15,   20,  -15,   20,  -15,   20,  -15,
   2,   54,    2,   54,    2,   54,    2,   54,
   3,   74,    3,   74,    3,   74,    3,   74,
 -28,  127,  -28,  127,  -28,  127,  -28,  127,
 -23,  104,  -23,  104,  -23,  104,  -23,  104,
  -6,   53,   -6,   53,   -6,   53,   -6,   53,
  -1,   54,   -1,   54,   -1,   54,   -1,   54,
   7,   51,    7,   51,    7,   51,    7,   51,
  -6,   42,   -2,   44,  -15,   71,  -22,   82,
  -5,   50,    0,   45,   -7,   61,   -9,   62,
  -3,   54,    0,   52,    0,   53,    0,   53,
  -2,   62,   -3,   64,   -5,   66,    0,   59,
   0,   58,   -2,   59,  -11,   77,  -14,   85,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
  -5,   27,    3,   24,   -3,   39,  -14,   57,
  -3,   39,    0,   42,   -5,   53,  -12,   67,
  -2,   44,    0,   48,   -7,   61,  -11,   71,
   0,   46,    0,   55,  -11,   75,  -10,   77,
 -16,   64,   -6,   59,  -15,   77,  -21,   85,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
 -15,   55,   -3,   29,  -10,   44,   -8,   48,
 -10,   60,   -1,   36,  -10,   52,   -8,   61,
  -6,   62,    1,   38,  -10,   57,   -8,   66,
  -4,   65,    2,   43,   -9,   58,   -7,   70,
 -12,   73,   -6,   55,  -16,   72,  -14,   75,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
 -20,   84,   -4,   29,   -9,   34,  -22,   69,
 -11,   79,    5,   31,    1,   32,  -16,   75,
  -6,   73,    7,   42,   11,   31,   -2,   58,
  -4,   74,    1,   59,    5,   52,    1,   58,
 -13,   86,   -2,   58,   -2,   55,  -13,   78,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
  -5,   33,    8,    5,    7,    4,   -6,   38,
  -4,   48,   10,   14,   10,    8,  -13,   62,
  -2,   53,   14,   18,   17,    8,   -6,   58,
  -3,   62,   13,   27,   16,   19,   -2,   59,
 -13,   71,    2,   40,    3,   37,  -16,   73,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
  -1,   23,   -7,   35,    0,   24,   -7,   37,
   1,   34,   -7,   42,   -1,   36,   -8,   44,
   1,   43,   -8,   45,   -2,   42,  -11,   49,
   0,   54,   -5,   48,   -2,   52,  -10,   56,
  -2,   55,  -12,   56,   -9,   57,  -12,   59,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
  -4,   56,   -1,   48,   -3,   53,   -6,   56,
  -5,   82,    0,   68,    0,   68,    3,   68,
  -7,   76,   -4,   69,   -7,   74,   -8,   71,
 -22,  125,   -8,   88,   -9,   88,  -13,   98,
  -4,   75,    7,   50,    9,   41,  -10,   66,
  37,  -16,   16,   30,   14,   35,   19,   16,
   0,   76,   10,   44,   16,   37,    5,   64,
  31,   -7,   21,   24,   22,   22,   24,   23,
 -21,  107,  -28,   82,  -35,   98,  -25,   86,
 -27,  127,  -20,   94,  -24,  102,  -12,   89,
 -31,  127,  -16,   83,  -23,   97,  -17,   91,
 -24,  127,  -22,  110,  -27,  119,  -31,  127,
 -18,   95,  -21,   91,  -24,   99,  -14,   76,
 -27,  127,  -18,  102,  -21,  110,  -18,  103,
 -21,  114,  -13,   93,  -18,  102,  -13,   90,
 -30,  127,  -29,  127,  -36,  127,  -37,  127,
  -6,   84,  -16,  106,  -23,  124,  -25,  127,
   6,   19,   11,   14,   18,   -6,   16,   -9,
  -8,   79,  -10,   87,  -20,  110,  -25,  120,
   7,   16,    9,   11,   14,    0,   17,  -12,
   0,   66,  -21,  114,  -26,  126,  -27,  127,
  12,   14,   18,   11,   26,  -12,   27,  -21,
  -1,   71,  -18,  110,  -25,  124,  -19,  114,
  18,   13,   21,    9,   31,  -16,   37,  -30,
   0,   62,  -14,   98,  -17,  105,  -23,  117,
  13,   11,   23,   -2,   33,  -25,   41,  -40,
  -2,   60,  -22,  110,  -27,  121,  -25,  118,
  13,   15,   32,  -15,   33,  -22,   42,  -41,
  -2,   59,  -21,  106,  -27,  117,  -26,  117,
  15,   16,   32,  -15,   37,  -28,   48,  -47,
  -5,   75,  -18,  103,  -17,  102,  -24,  113,
  12,   23,   34,  -21,   39,  -30,   39,  -32,
  -3,   62,  -21,  107,  -26,  117,  -28,  118,
  13,   23,   39,  -23,   42,  -30,   46,  -40,
  -4,   58,  -23,  108,  -27,  116,  -31,  120,
  15,   20,   42,  -33,   47,  -42,   52,  -51,
  -9,   66,  -26,  112,  -33,  122,  -37,  124,
  14,   26,   41,  -31,   45,  -36,   46,  -41,
  -1,   79,  -10,   96,  -10,   95,  -10,   94,
  14,   44,   46,  -28,   49,  -34,   52,  -39,
   0,   71,  -12,   95,  -14,  100,  -15,  102,
  17,   40,   38,  -12,   41,  -17,   43,  -19,
   3,   68,   -5,   91,   -8,   95,  -10,   99,
  17,   47,   21,   29,   32,    9,   32,   11,
   0,   64,   -6,   86,    6,   69,   -6,   93,
   0,   64,  -17,   95,  -13,   90,  -14,   88,
   0,   64,   -6,   61,    0,   52,   -6,   44,
   0,   64,    9,   45,    8,   43,    4,   55,
  -7,   62,  -22,   94,  -28,  114,  -50,  127,
  21,   21,   53,  -45,   63,  -63,   56,  -46,
  15,   36,   -5,   86,   -6,   89,   -5,   92,
  25,   22,   48,  -26,   66,  -64,   62,  -50,
  14,   40,    9,   67,   -2,   80,   17,   57,
  31,   27,   65,  -43,   77,  -74,   81,  -67,
  16,   27,   -4,   80,   -4,   82,   -5,   86,
  22,   29,   43,  -19,   54,  -39,   45,  -20,
  12,   29,  -10,   85,   -9,   85,  -13,   94,
  19,   35,   39,  -10,   52,  -35,   35,   -2,
   1,   44,   -1,   70,   -8,   81,  -12,   91,
  14,   50,   30,    9,   41,  -10,   28,   15,
  20,   36,    7,   60,   -1,   72,   -2,   77,
  10,   57,   18,   26,   36,    0,   34,    1,
  18,   32,    9,   58,    5,   64,    0,   71,
   7,   63,   20,   27,   40,   -1,   39,    1,
   5,   42,    5,   61,    1,   67,   -1,   73,
  -2,   77,    0,   57,   30,   14,   30,   17,
   1,   48,   12,   50,    9,   56,    4,   64,
  -4,   82,  -14,   82,   28,   26,   20,   38,
  10,   62,   15,   50,    0,   69,   -7,   81,
  -3,   94,   -5,   75,   23,   37,   18,   45,
  17,   46,   18,   49,    1,   69,    5,   64,
   9,   69,  -19,   97,   12,   55,   15,   54,
   9,   64,   17,   54,    7,   69,   15,   57,
 -12,  109,  -35,  125,   11,   65,    0,   79,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
   0,   64,   18,   64,   26,   34,   20,   40,
   0,   64,    9,   43,   19,   22,   20,   10,
   0,   64,   29,    0,   40,    0,   29,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
 -11,   97,    7,   46,   -6,   67,    0,   68,
  36,  -34,   28,    0,   39,  -36,   37,  -14,
 -16,   96,   -1,   51,  -16,   77,  -10,   67,
  32,  -26,   31,   -4,   40,  -37,   37,  -17,
  -7,   88,    7,   49,   -2,   64,    1,   68,
  37,  -30,   27,    6,   38,  -30,   32,    1,
  -8,   85,    8,   52,    2,   61,    0,   77,
  44,  -32,   34,    8,   46,  -33,   34,   15,
  -7,   85,    9,   41,   -6,   67,    2,   64,
  34,  -18,   30,   10,   42,  -30,   29,   15,
  -9,   85,    6,   47,   -3,   64,    0,   68,
  34,  -15,   24,   22,   40,  -24,   24,   25,
 -13,   88,    2,   55,    2,   57,   -5,   78,
  40,  -15,   33,   19,   49,  -29,   34,   22,
   4,   66,   13,   41,   -3,   65,    7,   55,
  33,   -7,   22,   32,   38,  -12,   31,   16,
  -3,   77,   10,   44,   -3,   66,    5,   59,
  35,   -5,   26,   31,   40,  -10,   35,   18,
  -3,   76,    6,   50,    0,   62,    2,   65,
  33,    0,   21,   41,   38,   -3,   31,   28,
  -6,   76,    5,   53,    9,   51,   14,   54,
  38,    2,   26,   44,   46,   -5,   33,   41,
  10,   58,   13,   49,   -1,   66,   15,   44,
  33,   13,   23,   47,   31,   20,   36,   28,
  -1,   76,    4,   63,   -2,   71,    5,   60,
  23,   35,   16,   65,   29,   30,   27,   47,
  -1,   83,    6,   64,   -2,   75,    2,   70,
  13,   58,   14,   71,   25,   44,   21,   62,
  -9,   83,   -9,   83,   -9,   83,   -9,   83,
   4,   86,    4,   86,    4,   86,    4,   86,
   0,   97,    0,   97,    0,   97,    0,   97,
  -7,   72,   -7,   72,   -7,   72,   -7,   72,
 -14,   95,   -2,   59,   -9,   72,  -18,   86,
  26,    0,    6,   63,   11,   49,   19,   26,
   2,   95,    6,   70,   14,   60,   12,   70,
  22,   30,   17,   65,   26,   45,   36,   24,
   0,   11,    0,   45,   13,   15,    7,   34,
   1,   55,   -4,   78,    7,   51,   -9,   88,
   0,   69,   -3,   96,    2,   80,  -20,  127,
// 0,    0,    0,    0,    0,    0,    0,    0,
  -3,   70,   -3,   74,   -2,   73,   -5,   79,
  -8,   93,   -9,   92,  -12,  104,  -11,  104,
 -10,   90,   -8,   87,   -9,   91,  -11,   91,
 -30,  127,  -23,  126,  -31,  127,  -30,  127,
 -13,  108,    9,   53,    0,   54,    1,   67,
  26,  -19,   25,    7,   33,  -25,   35,  -18,
 -12,  104,   10,   41,   -7,   69,    1,   67,
  36,  -35,   27,    0,   37,  -33,   36,  -16,
  -1,   74,    5,   54,    3,   55,    0,   65,
  -6,   97,    6,   60,    7,   56,   -2,   79,
  -7,   91,    6,   59,    7,   55,    0,   72,
 -20,  127,    6,   69,    8,   61,   -4,   92,
  -8,  102,    3,   64,   -4,   71,    3,   65,
  30,   -6,    1,   67,    0,   75,   20,   34,
  -7,   99,   -2,   69,   -1,   70,   -2,   76,
  29,   -3,    8,   60,   12,   48,   18,   31,
   0,   64,    0,   58,    1,   56,    1,   63,
   0,   64,   -3,   76,   -3,   74,   -5,   85,
   0,   64,  -10,   94,   -6,   85,  -13,  106,
   0,   64,    5,   54,    0,   59,    5,   63,
   0,   64,    4,   69,   -3,   81,    6,   75,
   0,   64,   -3,   81,   -7,   86,   -3,   90,
   0,   64,    0,   88,   -5,   95,   -1,  101,
// 0,    0,    0,    0,    0,    0,    0,    0,
  -5,   74,    9,   31,    0,   47,  -12,   70,
  35,  -15,   23,   20,   23,   22,   27,   16,
   0,   70,   12,   43,   18,   35,   11,   55,
  34,   -3,   26,   23,   27,   21,   24,   30,
 -11,   75,    3,   53,   11,   37,    5,   56,
  34,    3,   27,   32,   33,   20,   31,   29,
   1,   68,   14,   34,   12,   41,    0,   69,
  36,   -1,   28,   23,   26,   28,   22,   41,
   0,   65,   10,   38,   10,   41,    2,   65,
  34,    5,   28,   24,   30,   24,   22,   42,
 -14,   73,   -3,   52,    2,   48,   -6,   74,
  32,   11,   23,   40,   27,   34,   16,   60,
   3,   62,   13,   40,   12,   41,    5,   54,
  35,    5,   24,   32,   18,   42,   15,   52,
   4,   62,   17,   32,   13,   41,    7,   54,
  34,   12,   28,   29,   25,   39,   14,   60,
  -1,   68,    7,   44,    0,   59,   -6,   76,
  39,   11,   23,   42,   18,   50,    3,   78,
 -13,   75,    7,   38,    3,   50,  -11,   82,
  30,   29,   19,   57,   12,   70,  -16,  123,
  11,   55,   13,   50,   19,   40,   -2,   77,
  34,   26,   22,   53,   21,   54,   21,   53,
   5,   64,   10,   57,    3,   66,   -2,   77,
  29,   39,   22,   61,   14,   71,   22,   56,
  12,   70,   26,   43,   18,   50,   25,   42,
  19,   66,   11,   86,   11,   83,   25,   61,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
 -17,  127,  -27,  126,  -39,  127,  -36,  127,
 -13,  102,  -28,   98,  -18,   91,  -17,   91,
   0,   82,  -25,  101,  -17,   96,  -14,   95,
  -7,   74,  -23,   67,  -26,   81,  -25,   84,
  28,   -8,   39,   -7,   42,   -9,   31,   -4,
  -9,   79,   -4,   74,   -3,   70,  -11,   63,
 -14,   86,  -10,   83,   -6,   76,   -5,   70,
 -10,   73,   -9,   71,   -5,   66,  -17,   75,
 -10,   70,   -9,   67,   -5,   62,  -14,   72,
 -10,   69,   -1,   61,    0,   57,  -16,   67,
  -5,   66,   -8,   66,   -4,   61,   -8,   53,
  -9,   64,  -14,   66,   -9,   60,  -14,   59,
  28,   -1,   46,   -2,   49,   -5,   33,   -1,
  29,    3,   51,    2,   53,    0,   33,    7,
  29,    9,   60,    6,   64,    3,   31,   12,
  35,   20,   61,   17,   68,   10,   37,   23,
  29,   36,   55,   34,   66,   27,   31,   38,
  14,   67,   42,   62,   47,   57,   20,   64,
  -5,   58,    0,   59,    1,   54,   -9,   52,
   2,   59,    2,   59,    2,   58,  -11,   68,
  21,  -10,   21,  -13,   17,  -10,    9,   -2,
  24,  -11,   33,  -14,   32,  -13,   30,  -10,
 -14,  106,   -5,   85,   -3,   81,   -3,   78,
 -13,   97,   -6,   81,   -3,   76,   -8,   74,
 -15,   90,  -10,   77,   -7,   72,   -9,   72,
 -12,   90,   -7,   81,   -6,   78,  -10,   72,
 -18,   88,  -17,   80,  -12,   72,  -18,   75,
 -10,   73,  -18,   73,  -14,   68,  -12,   71,
   0,   64,    1,    9,   -2,    9,  -10,   51,
   0,   64,    0,   49,    4,   41,   -3,   62,
   0,   64,  -37,  118,  -29,  118,  -27,   99,
   0,   64,    5,   57,    2,   65,   26,   16,
   0,   64,  -13,   78,   -6,   71,   -4,   85,
   0,   64,  -11,   65,  -13,   79,  -24,  102,
   0,   64,    1,   62,    5,   52,    5,   57,
// 0,    0,    0,    0,    0,    0,    0,    0,
  -3,   71,   -6,   76,  -23,  112,  -24,  115,
   1,   63,   -4,   70,   -9,   80,  -13,   89,
  -2,   72,   -4,   75,   -9,   84,  -13,   94,
  -1,   74,   -8,   82,  -10,   87,  -11,   92,
  -9,   91,  -17,  102,  -34,  127,  -29,  127,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
  -5,   67,   -9,   77,  -21,  101,  -21,  100,
  -8,   68,   -7,   71,  -17,   91,  -16,   88,
 -10,   78,  -12,   83,  -25,  107,  -23,  104,
  -6,   77,  -11,   87,  -25,  111,  -15,   98,
 -10,   86,  -30,  119,  -28,  122,  -37,  127,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
 -12,   92,    1,   58,  -11,   76,  -10,   82,
  -8,   76,    0,   58,   -7,   69,  -10,   79,
  -7,   80,    0,   64,   -4,   69,   -9,   83,
  -9,   88,   -3,   74,   -5,   74,  -12,   92,
 -17,  110,  -10,   90,   -9,   86,  -18,  108,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
 -11,   97,    0,   70,    2,   66,   -4,   79,
 -13,   96,   -3,   72,   -2,   67,   -9,   83,
 -11,   97,   -3,   81,    0,   73,   -4,   81,
 -19,  117,  -11,   97,   -8,   89,  -13,   99,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
  -8,   78,    0,   58,    3,   52,  -13,   81,
 -10,   79,    0,   58,   -1,   61,  -10,   76,
 -12,   86,   -3,   70,   -5,   73,  -13,   86,
 -13,   90,   -6,   79,   -1,   70,   -9,   83,
 -14,   97,   -8,   85,   -4,   78,  -10,   87,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
  -3,   75,   -6,   66,   -5,   71,   -9,   71,
   0,   61,   -6,   60,   -6,   63,   -8,   63,
   1,   64,   -5,   62,   -4,   65,   -9,   67,
   0,   68,   -8,   66,   -4,   67,   -6,   68,
  -9,   92,   -8,   76,   -7,   82,  -10,   79,
   0,   64,  -13,   78,   -6,   71,   -4,   85,
   0,   64,  -11,   65,  -13,   79,  -24,  102,
   0,   64,    1,   62,    5,   52,    5,   57,
   0,   64,   -7,   67,   -1,   66,    3,   55,
   0,   64,   -5,   74,   -1,   77,   -4,   79,
   0,   64,   -4,   74,    1,   70,   -2,   75,
   0,   64,   -5,   80,   -2,   86,  -12,   97,
   0,   64,   -7,   72,   -5,   72,   -7,   50,
   0,   64,    1,   58,    0,   61,    1,   60,
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
   0,   64,   26,   67,   57,    2,   54,    0,
   0,   64,   16,   90,   41,   36,   37,   42,
   0,   64,    9,  104,   26,   69,   12,   97,
   0,   64,  -46,  127,  -45,  127,  -32,  127,
   0,   64,  -20,  104,  -15,  101,  -22,  117,
   0,   64,    1,   67,   -4,   76,   -2,   74
// 0,    0,    0,    0,    0,    0,    0,    0,
// 0,    0,    0,    0,    0,    0,    0,    0,
};

MMP_UINT8 cabac_output_map[512] = {
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,0,0,1,1,1,0,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,0,1,0,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,0,0,1,1,1,0,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,0,0,0,
    1,1,1,1,1,0,0,0,
    1,1,1,1,1,0,0,0,
    1,1,1,1,1,0,0,0,
    1,1,1,1,1,0,0,0,
    1,1,1,1,1,0,0,0,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,0,0,1,1,1,0,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,0,0,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,0,
    1,1,1,1,1,0,0,0,
    1,1,1,1,1,0,0,0,
    1,1,1,1,1,0,0,0,
    1,1,1,1,0,0,0,0,
    1,1,1,1,1,0,0,0,
    1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,0,0,
    1,1,1,1,1,1,0,0
};

//=============================================================================
//                              Global Data Definition
//=============================================================================

static PARAMT_SET_LIS*          gparamSetList = MMP_NULL;
static MMP_UINT8*               gpRBSP = MMP_NULL;
static MMP_BOOL                 gbSkipHwDecode = MMP_FALSE;

static MMP_BOOL                 gbSEIPicStrFlg = MMP_FALSE;
static MMP_UINT32               gSEIPicStructure = 0;


StorablePicture **listX[6];
int listXsize[6] = {0};
StorablePicture *dec_picture = MMP_NULL;
ImageParameters *img = MMP_NULL;
DecodedPictureBuffer dpb;
AVC_SEQUENCE_PARAMETER_SET *active_sps = MMP_NULL;
AVC_PICTURE_PARAMETER_SET *active_pps = MMP_NULL;

int gPocList[32] = {0};
int gRefIdxToPocListIdx[2][32] = {0};
int gPicIdxMode;
sliceParametersDef gSliceParameters;
MMP_UINT8  *qmatrix[8];
int weighted_pred_RF[192][3] = {0};

MMP_UINT32 PicIdxQueue[32] = {0};
MMP_UINT32 DecodingQueue[8] = {0};

BITSTREAM  gtAVCBitStream;
MMP_UINT32 gAVCCmdBuf;
MMP_UINT32 gOrgCmdBuf;
MMP_BOOL   gbCheckFrmGap = MMP_FALSE;
MMP_BOOL   gbLastPic = MMP_TRUE;
int gDePicCount;

extern MMP_BOOL   gbFrameEnd;
extern MMP_BOOL   gbFrameStart;
extern AVC_DECODER* gptAVCDecoder;
extern AVC_VIDEO_DECODER* gptAVCVideoDecoder;

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static void
nal_unit_to_RBSP(EMULATION_PREVENTION_BYTE *pPreventionByte,
                 MMP_UINT8 *pNalUnit,
                 MMP_UINT32 *pRBSPsize,
                 MMP_UINT32 numberBytesInNALunit);
static AVC_ERROR_CODE
nal_get_seq_parameter_set(AVC_SEQUENCE_PARAMETER_SET *pSeqParamSet,
                          MMP_UINT32 RBSPsize);
static AVC_ERROR_CODE
avc_InsertSequenceParameterSet(AVC_SEQUENCE_PARAMETER_SET *pSeqParamSet);

static AVC_ERROR_CODE
nal_get_pic_parameter_set(AVC_PICTURE_PARAMETER_SET *pPicParamSet,
                          MMP_UINT32 RBSPsize);

static AVC_ERROR_CODE
avc_InsertPictureParameterSet(AVC_PICTURE_PARAMETER_SET *pPicParamSet);
static MMP_UINT32 ue_v(MMP_UINT32 *useBits);
static MMP_INT32 se_v(MMP_UINT32 *useBits);
static MMP_UINT32 Log2BitNum(MMP_UINT32 value);
static MMP_BOOL more_rbsp_data(void);
static AVC_ERROR_CODE
avc_GetParameterSet(AVC_SEQUENCE_PARAMETER_SET **pSeqParamSet,
                    AVC_PICTURE_PARAMETER_SET **pPicParamSet,
                    MMP_UINT32 picParameSetId);
static void
scaling_list(MMP_UINT8 *scalingList, MMP_UINT32 sizeOfScalingList, MMP_UINT32 *UseDefaultScalingMatrix);
static MMP_UINT32
sps_is_equal(AVC_SEQUENCE_PARAMETER_SET *sps1, AVC_SEQUENCE_PARAMETER_SET *sps2);
static MMP_UINT32
pps_is_equal(AVC_PICTURE_PARAMETER_SET *pps1, AVC_PICTURE_PARAMETER_SET *pps2);

static void
free_ref_pic_list_reordering_buffer(Slice *currSlice);
static void
free_marking_buffer(void);

//=============================================================================
//                              Private Function Definition
//=============================================================================
static MMP_UINT32 ue_v(MMP_UINT32 *useBits)
{
MMP_INT leadingZeroBits = -1;
MMP_INT i = 0;

    for(i = 0; !i; leadingZeroBits++)
    {
        i = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    }
    *useBits = (leadingZeroBits << 1) + 1;
    if(leadingZeroBits == 0)
    {
        return 0;
    }
    else
    {
        return ((1 << leadingZeroBits) + BitStreamKit_GetBits(&gtAVCBitStream, leadingZeroBits) - 1); // codeNum = 2^leadingZeroBits -  1 + read_bits(leadingZeroBits)
    }
}

static MMP_INT32 se_v(MMP_UINT32 *useBits)
{
MMP_INT leadingZeroBits = -1;
MMP_INT i = 0;
MMP_INT value = 0;

    for(i = 0; !i; leadingZeroBits++)
    {
        i = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    }
    *useBits = (leadingZeroBits << 1) + 1;
    if(leadingZeroBits == 0)
    {
        return 0;
    }
    else
    {
        //  codeNum = 2^leadingZeroBits - 1 + read_bits(leadingZeroBits)
        i = (1 << leadingZeroBits) + BitStreamKit_GetBits(&gtAVCBitStream, leadingZeroBits) - 1;
        value = ((i+1) >> 1);
        if((i & 0x01)==0)           // lsb is signed bit
        {
            value = -value;
        }
        return value;
    }
}

//-----< Log2BitNum >-----
static MMP_UINT32 Log2BitNum(MMP_UINT32 value)
{
    MMP_UINT32 n = 0;

    while (value)
    {
        value >>= 1;
        n++;
    }
    return n;
}

static MMP_BOOL more_rbsp_data(void)
{
    MMP_UINT32 remainByte = gtAVCBitStream.remainByte;
    MMP_INT32 currBitPos = 7- gtAVCBitStream.bitPosition;
    MMP_UINT32 ctr_bit = 0;
    MMP_UINT32 cnt = 0;
    MMP_UINT32 i = 1;

    if (remainByte >= 2)
    {
        while (remainByte > 1)
        {
            if (*(gtAVCBitStream.pStartAddress+i) != 0)
                return MMP_TRUE;
            remainByte --;
            i++;
        }
    }

    // read one bit
    ctr_bit = (*gtAVCBitStream.pStartAddress) & (0x01<<currBitPos);

    // a stop bit has to be one
    if (ctr_bit==0) return MMP_TRUE;

    currBitPos--;

    while (currBitPos >=0)
    {
        ctr_bit = (*gtAVCBitStream.pStartAddress) & (0x01<<currBitPos);   // set up control bit
        if (ctr_bit>0) cnt++;
        currBitPos--;
    }

    return (0!=cnt);
}

// [out]pPreventionByte
// [in] pNalUnit
// [out]pRBSPsize
// [in] numberBytesInNALunit
static void
nal_unit_to_RBSP(EMULATION_PREVENTION_BYTE *pPreventionByte,
                 MMP_UINT8 *pNalUnit,
                 MMP_UINT32 *pRBSPsize,
                 MMP_UINT32 numberBytesInNALunit)
{
    MMP_UINT8*  pCurrentAddr = pNalUnit;
    MMP_UINT8*  pBufferStart = gptAVCVideoDecoder->pBufferStartAddr;
    MMP_UINT8*  pBufferEnd = gptAVCVideoDecoder->pBufferEndAddr;

    MMP_UINT32  NumBytesInRBSP = 0;
    MMP_UINT32  i = 0;
    MMP_UINT32  j = 0;
    MMP_UINT32  size = 0;
    MMP_UINT32  zero_count = 0;

    pPreventionByte->number = 0;
    *pRBSPsize = 0;

    i = 0;
    for (j = 0; j<numberBytesInNALunit; j++)
    {
        if ((zero_count == 2) && (pCurrentAddr[i] == 0x03))
        {
            pPreventionByte->position[pPreventionByte->number++] = i; // the (pNalUnit + (i + 1)) byte is (0x03)
            if (&pCurrentAddr[i] == (pBufferEnd - 1))
            {
                printf("0x3 wrap\n");
                pCurrentAddr = pBufferStart;
                i = 0;
            }
            else
                i++;

           zero_count = 0;

           if (pPreventionByte->number >= MAX_NUM_PREVENTION_BYTE)
               break;
           //printf("Find 0x03 %d\n", pPreventionByte->number);
        }
        else
        {
           gpRBSP[NumBytesInRBSP++] = pCurrentAddr[i];
           ((pCurrentAddr[i] == 0x00) ? (zero_count++) : (zero_count = 0));

           if (&pCurrentAddr[i] == (pBufferEnd - 1))    // reach the last byte
           {
               pCurrentAddr = pBufferStart;
               i = 0;
           }
           else
               i++;
        }
    }

    *pRBSPsize = NumBytesInRBSP;
    if (NumBytesInRBSP > MAX_RBSP_SIZE)
        printf("Error %d %x %x %x\n", NumBytesInRBSP, pNalUnit, pBufferStart, pBufferEnd);
}

void ReadNALHRDParameters(AVC_SEQUENCE_PARAMETER_SET *p)
{
    MMP_UINT32 i, j, tmp;
    MMP_UINT32 usedBits = 0;

    j   = ue_v(&usedBits);
    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 4);
    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 4);

    for ( i = 0; i <= j; i++ )
    {
        tmp = ue_v(&usedBits);
        tmp = ue_v(&usedBits);
        tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    }

    // initial_cpb_removal_delay_length_minus1
    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 5);
    p->nal_cpb_removal_delay_length_minus1 = BitStreamKit_GetBits(&gtAVCBitStream, 5);
    p->nal_dpb_output_delay_length_minus1 = BitStreamKit_GetBits(&gtAVCBitStream, 5);
    // time_offset_length
    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 5);
}

void ReadVCLHRDParameters(AVC_SEQUENCE_PARAMETER_SET *p)
{
    MMP_UINT32 i, j, tmp;
    MMP_UINT32 usedBits = 0;

    j   = ue_v(&usedBits);
    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 4);
    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 4);

    for ( i = 0; i <= j; i++ )
    {
        tmp = ue_v(&usedBits);
        tmp = ue_v(&usedBits);
        tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    }

    // initial_cpb_removal_delay_length_minus1
    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 5);
    p->vcl_cpb_removal_delay_length_minus1 = BitStreamKit_GetBits(&gtAVCBitStream, 5);
    p->vcl_dpb_output_delay_length_minus1 = BitStreamKit_GetBits(&gtAVCBitStream, 5);
    // time_offset_length
    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 5);
}

static AVC_ERROR_CODE
nal_get_seq_parameter_set(AVC_SEQUENCE_PARAMETER_SET *pSeqParamSet,
                          MMP_UINT32 RBSPsize)
{
    AVC_ERROR_CODE ret = AVC_ERROR_SUCCESS;
    MMP_UINT32 usedBits = 0;
    MMP_INT i = 0;

    memset((void*)pSeqParamSet, 0x00, sizeof(AVC_SEQUENCE_PARAMETER_SET));

    gtAVCBitStream.remainByte = RBSPsize;
    BitStreamKit_Init(&gtAVCBitStream, gpRBSP);
    pSeqParamSet->profile_idc          = BitStreamKit_GetBits(&gtAVCBitStream, 8);
    pSeqParamSet->constraint_set0_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pSeqParamSet->constraint_set1_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pSeqParamSet->constraint_set2_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pSeqParamSet->constraint_set3_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pSeqParamSet->reserved_zero_4bits  = BitStreamKit_GetBits(&gtAVCBitStream, 4);
    pSeqParamSet->level_idc            = BitStreamKit_GetBits(&gtAVCBitStream, 8);
    pSeqParamSet->seq_parameter_set_id = ue_v(&usedBits);

    //if (pSeqParamSet->level_idc > 30)
    //    return ERROR_RESOLUTION;

    pSeqParamSet->chroma_format_idc = 1;
    if((100 == pSeqParamSet->profile_idc) || (110 == pSeqParamSet->profile_idc) ||
       (122 == pSeqParamSet->profile_idc) || (144 == pSeqParamSet->profile_idc))
    {
        pSeqParamSet->chroma_format_idc = ue_v(&usedBits);
        if(3 == pSeqParamSet->chroma_format_idc)
        {
            pSeqParamSet->residual_colour_transform_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        }
        pSeqParamSet->bit_depth_luma_minus8                = ue_v(&usedBits);
        pSeqParamSet->bit_depth_chroma_minus8              = ue_v(&usedBits);
        pSeqParamSet->qpprime_y_zero_transform_bypass_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        pSeqParamSet->seq_scaling_matrix_present_flag      = BitStreamKit_GetBits(&gtAVCBitStream, 1);

        if(pSeqParamSet->seq_scaling_matrix_present_flag)
        {
            MMP_UINT32 seq_scaling_list_present_flag = 0;

            for(i=0; i<8; i++)
            {
                pSeqParamSet->seq_scaling_list_present_flag[i] = BitStreamKit_GetBits(&gtAVCBitStream, 1);
                if(pSeqParamSet->seq_scaling_list_present_flag[i])
                {
                    if (i<6)
                        scaling_list(pSeqParamSet->ScalingList4x4[i], 16, &pSeqParamSet->UseDefaultScalingMatrix4x4Flag[i]);
                    else
                        scaling_list(pSeqParamSet->ScalingList8x8[i-6], 64, &pSeqParamSet->UseDefaultScalingMatrix8x8Flag[i-6]);
                }
            }
        }
    }

    pSeqParamSet->log2_max_frame_num_minus4 = ue_v(&usedBits);
    pSeqParamSet->pic_order_cnt_type        = ue_v(&usedBits);
    if(0 == pSeqParamSet->pic_order_cnt_type)
    {
        pSeqParamSet->log2_max_pic_order_cnt_lsb_minus4 = ue_v(&usedBits);
    }
    else if(1 == pSeqParamSet->pic_order_cnt_type)
    {
        pSeqParamSet->delta_pic_order_always_zero_flag      = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        pSeqParamSet->offset_for_non_ref_pic                = se_v(&usedBits);
        pSeqParamSet->offset_for_top_to_bottom_field        = se_v(&usedBits);
        pSeqParamSet->num_ref_frames_in_pic_order_cnt_cycle = ue_v(&usedBits);
        for(i=0; i<pSeqParamSet->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            pSeqParamSet->offset_for_ref_frame[i] = se_v(&usedBits);
        }
    }

    pSeqParamSet->num_ref_frames                       = ue_v(&usedBits);
    pSeqParamSet->gaps_in_frame_num_value_allowed_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pSeqParamSet->pic_width_in_mbs_minus1              = ue_v(&usedBits);
    pSeqParamSet->pic_height_in_map_units_minus1       = ue_v(&usedBits);
    pSeqParamSet->frame_mbs_only_flag                  = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    if(!pSeqParamSet->frame_mbs_only_flag)
    {
        pSeqParamSet->mb_adaptive_frame_field_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    }
    pSeqParamSet->direct_8x8_inference_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pSeqParamSet->frame_cropping_flag       = BitStreamKit_GetBits(&gtAVCBitStream, 1);

    if(pSeqParamSet->frame_cropping_flag)
    {
        pSeqParamSet->frame_crop_left_offset   = ue_v(&usedBits);
        pSeqParamSet->frame_crop_right_offset  = ue_v(&usedBits);
        pSeqParamSet->frame_crop_top_offset    = ue_v(&usedBits);
        pSeqParamSet->frame_crop_bottom_offset = ue_v(&usedBits);
    }
    pSeqParamSet->vui_parameters_present_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);

    // printf("Seq More bits %d Pos %d\n", gtAVCBitStream.remainByte, gtAVCBitStream.bitPosition);

    if(pSeqParamSet->vui_parameters_present_flag)
    {
        MMP_UINT32 sar_width = 0;
        MMP_UINT32 sar_height = 0;
        MMP_UINT32 tmp;

        pSeqParamSet->aspect_ratio_info_present_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        if (pSeqParamSet->aspect_ratio_info_present_flag)
        {
            pSeqParamSet->aspect_ratio_idc = BitStreamKit_GetBits(&gtAVCBitStream, 8);

            if (255==pSeqParamSet->aspect_ratio_idc)
            {
                sar_width  = BitStreamKit_GetBits(&gtAVCBitStream, 16);
                sar_height = BitStreamKit_GetBits(&gtAVCBitStream, 16);
            }
        }

        // overscan_info_present_flag
        tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        if (tmp)
        {
            BitStreamKit_SkipBits(&gtAVCBitStream, 1);
        }

        // video_signal_type_present_flag
        tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        if (tmp)
        {
            BitStreamKit_SkipBits(&gtAVCBitStream, 3);
            BitStreamKit_SkipBits(&gtAVCBitStream, 1);

            // colour_description_present_flag
            tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
            if (tmp)
            {
                BitStreamKit_SkipBits(&gtAVCBitStream, 8);
                BitStreamKit_SkipBits(&gtAVCBitStream, 8);
                BitStreamKit_SkipBits(&gtAVCBitStream, 8);
            }
        }

        // chroma_location_info_present_flag
        tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        if (tmp)
        {
            tmp = ue_v(&usedBits);
            tmp = ue_v(&usedBits);
        }
        pSeqParamSet->timing_info_present_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        if (pSeqParamSet->timing_info_present_flag)
        {
            pSeqParamSet->num_units_in_tick = BitStreamKit_GetBits(&gtAVCBitStream, 32);
            pSeqParamSet->time_scale = BitStreamKit_GetBits(&gtAVCBitStream, 32);
            pSeqParamSet->fixed_frame_rate_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
            av_log(NULL, AV_LOG_INFO, "sps num_units_in_tick %d, time_scale %d fix_frame_rate_flag %d\n",
			    pSeqParamSet->num_units_in_tick, pSeqParamSet->time_scale, pSeqParamSet->fixed_frame_rate_flag);
        }

        pSeqParamSet->nal_hrd_parameters_present_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        if (pSeqParamSet->nal_hrd_parameters_present_flag)
        {
            ReadNALHRDParameters(pSeqParamSet);
        }

        pSeqParamSet->vcl_hrd_parameters_present_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        if (pSeqParamSet->vcl_hrd_parameters_present_flag)
        {
            ReadVCLHRDParameters(pSeqParamSet);
        }

        if (pSeqParamSet->nal_hrd_parameters_present_flag || pSeqParamSet->vcl_hrd_parameters_present_flag)
        {
            tmp =  BitStreamKit_GetBits(&gtAVCBitStream, 1);
        }
        pSeqParamSet->pic_struct_present_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);

        // bitstream_restriction_flag
        tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);

        if (tmp)
        {
            // motion_vectors_over_pic_boundaries_flag
            tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
            // max_bytes_per_pic_denom
            tmp = ue_v(&usedBits);
            // max_bits_per_mb_denom
            tmp = ue_v(&usedBits);
            // log2_max_mv_length_horizontal
            tmp = ue_v(&usedBits);
            // log2_max_mv_length_vertical
            tmp = ue_v(&usedBits);
            // num_reorder_frames
            tmp = ue_v(&usedBits);
            // max_dec_frame_buffering
            tmp = ue_v(&usedBits);
        }

        // now skip decoding
        // vui_parameters();
    }
    //rbsp_trailing_bits();

    if (gtAVCBitStream.remainByte < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "ERROR_SEQUENCE_SET_NOT_SUPPORT \n");
        return ERROR_SEQUENCE_SET_NOT_SUPPORT;
    }

    return AVC_ERROR_SUCCESS;
}

static AVC_ERROR_CODE
avc_InsertSequenceParameterSet(AVC_SEQUENCE_PARAMETER_SET *pSeqParamSet)
{
    MMP_UINT32 id = DUMMY_SEQ_PARAM_SET_ID;

    id = pSeqParamSet->seq_parameter_set_id;

    if(id >= NUM_SEQ_PARAM_SET)
    {
        av_log(NULL, AV_LOG_ERROR, "ERROR_SEQUENCE_SET_ID \n");
        return ERROR_SEQUENCE_SET_ID;
    }

    // find a unused set in list
    if(MMP_NULL == gparamSetList->SeqParamSetList.pParamSet[id])
    {
        gparamSetList->SeqParamSetList.pParamSet[id] = (AVC_SEQUENCE_PARAMETER_SET*)malloc(sizeof(AVC_SEQUENCE_PARAMETER_SET));
        if(MMP_NULL == gparamSetList->SeqParamSetList.pParamSet[id])
        {
            av_log(NULL, AV_LOG_ERROR, "ERROR_MALLOC_SEQUENCE_SET \n");
            return ERROR_MALLOC_SEQUENCE_SET;
        }
    }

    // insert sequence parameter set into list
    memcpy((void*)gparamSetList->SeqParamSetList.pParamSet[id], (void*)pSeqParamSet, sizeof(AVC_SEQUENCE_PARAMETER_SET));
    return AVC_ERROR_SUCCESS;
}

static AVC_ERROR_CODE
nal_get_pic_parameter_set(AVC_PICTURE_PARAMETER_SET *pPicParamSet,
                          MMP_UINT32 RBSPsize)
{
AVC_ERROR_CODE ret = AVC_ERROR_SUCCESS;
MMP_UINT32 usedBits = 0;
MMP_INT iGroup = 0;
MMP_INT i = 0;

    memset((void*)pPicParamSet, 0x00, sizeof(AVC_PICTURE_PARAMETER_SET));
    gtAVCBitStream.remainByte = RBSPsize;
    BitStreamKit_Init(&gtAVCBitStream, gpRBSP);

    pPicParamSet->pic_parameter_set_id     = ue_v(&usedBits);
    pPicParamSet->seq_parameter_set_id     = ue_v(&usedBits);
    pPicParamSet->entropy_coding_mode_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pPicParamSet->pic_order_present_flag   = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pPicParamSet->num_slice_groups_minus1  = ue_v(&usedBits);
    if(pPicParamSet->num_slice_groups_minus1 > 0)
    {
        // Hw not support
        av_log(NULL, AV_LOG_ERROR, "ERROR_PICTURE_SET_NOT_SUPPORT \n");
        return ERROR_PICTURE_SET_NOT_SUPPORT;

        pPicParamSet->slice_group_map_type = ue_v(&usedBits);
        if(0 == pPicParamSet->slice_group_map_type)
        {
            MMP_UINT32 run_length_minus1 = 0;

            for(iGroup = 0; iGroup <= pPicParamSet->num_slice_groups_minus1; iGroup++)
            {
                run_length_minus1 = ue_v(&usedBits);
            }
        }
        else if(2 == pPicParamSet->slice_group_map_type)
        {
            MMP_UINT32 top_left = 0;
            MMP_UINT32 bottom_right = 0;

            for(iGroup=0; iGroup<pPicParamSet->num_slice_groups_minus1; iGroup++)
            {
                top_left     = ue_v(&usedBits);
                bottom_right = ue_v(&usedBits);
            }
        }
        else if((3 == pPicParamSet->slice_group_map_type) ||
                (4 == pPicParamSet->slice_group_map_type) ||
                (5 == pPicParamSet->slice_group_map_type))
        {
            pPicParamSet->slice_group_change_direction_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
            pPicParamSet->slice_group_change_rate_minus1    = ue_v(&usedBits);
        }
        else if(6 == pPicParamSet->slice_group_map_type)
        {
            MMP_UINT32 slice_group_id = 0;

            pPicParamSet->pic_size_in_map_units_minus1 = ue_v(&usedBits);

            for(i = 0; i <= pPicParamSet->pic_size_in_map_units_minus1; i++)
            {
                slice_group_id = BitStreamKit_GetBits(&gtAVCBitStream, Log2BitNum(pPicParamSet->num_slice_groups_minus1 + 1));
            }
        }
    }
    pPicParamSet->num_ref_idx_l0_active_minus1           = ue_v(&usedBits);
    pPicParamSet->num_ref_idx_l1_active_minus1           = ue_v(&usedBits);
    pPicParamSet->weighted_pred_flag                     = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pPicParamSet->weighted_bipred_idc                    = BitStreamKit_GetBits(&gtAVCBitStream, 2);
    pPicParamSet->pic_init_qp_minus26                    = se_v(&usedBits);
    pPicParamSet->pic_init_qs_minus26                    = se_v(&usedBits);
    pPicParamSet->chroma_qp_index_offset                 = se_v(&usedBits);
    pPicParamSet->deblocking_filter_control_present_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pPicParamSet->constrained_intra_pred_flag            = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    pPicParamSet->redundant_pic_cnt_present_flag         = BitStreamKit_GetBits(&gtAVCBitStream, 1);

    pPicParamSet->transform_8x8_mode_flag = 0;

    if (more_rbsp_data())
    {
        pPicParamSet->transform_8x8_mode_flag            = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        pPicParamSet->pic_scaling_matrix_present_flag    = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        if (pPicParamSet->pic_scaling_matrix_present_flag)
        {
            for (i=0; i<6+2*pPicParamSet->transform_8x8_mode_flag; i++)
            {
                pPicParamSet->pic_scaling_list_present_flag[i] = BitStreamKit_GetBits(&gtAVCBitStream, 1);
                if (pPicParamSet->pic_scaling_list_present_flag[i])
                    if (i<6)
                        scaling_list(pPicParamSet->ScalingList4x4[i], 16, &pPicParamSet->UseDefaultScalingMatrix4x4Flag[i]);
                    else
                        scaling_list(pPicParamSet->ScalingList8x8[i-6], 64, &pPicParamSet->UseDefaultScalingMatrix8x8Flag[i-6]);
            }
        }
        pPicParamSet->second_chroma_qp_index_offset     = se_v(&usedBits);
    }
    else
    {
        pPicParamSet->second_chroma_qp_index_offset      = pPicParamSet->chroma_qp_index_offset;
    }
    //printf("1 Picture More bits %d Pos %d %x\n", gtAVCBitStream.remainByte, gtAVCBitStream.bitPosition, BitStreamKit_ShowBits(&gtAVCBitStream, 0, 8-gtAVCBitStream.bitPosition));
    // TODO:
    /*
    if(more_rbsp_data())
    {

    }
    rbsp_trailing_bits();
    */

    if (gtAVCBitStream.remainByte < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "ERROR_PICTURE_SET_NOT_SUPPORT \n");
        return ERROR_PICTURE_SET_NOT_SUPPORT;
    }

    return AVC_ERROR_SUCCESS;
}

static AVC_ERROR_CODE
avc_InsertPictureParameterSet(AVC_PICTURE_PARAMETER_SET *pPicParamSet)
{
    MMP_UINT32 id = DUMMY_PIC_PARAM_SET_ID;

    id = pPicParamSet->pic_parameter_set_id;

    if (id >= NUM_PIC_PARAM_SET)
    {
        return ERROR_PICTURE_SET_ID;
    }

    // find a unused set in list
    if(MMP_NULL == gparamSetList->PicParamSetList.pParamSet[id])
    {
        gparamSetList->PicParamSetList.pParamSet[id] = (AVC_PICTURE_PARAMETER_SET*)malloc(sizeof(AVC_PICTURE_PARAMETER_SET));
        if(MMP_NULL == gparamSetList->PicParamSetList.pParamSet[id])
        {
            av_log(NULL, AV_LOG_ERROR, "ERROR_MALLOC_PIC_SET \n");
            return (MMP_RESULT)ERROR_MALLOC_PIC_SET;
        }
    }

    // insert picture parameter set into list
    memcpy((void*)gparamSetList->PicParamSetList.pParamSet[id], (void*)pPicParamSet, sizeof(AVC_PICTURE_PARAMETER_SET));
    return AVC_ERROR_SUCCESS;
}

static AVC_ERROR_CODE
avc_GetParameterSet(AVC_SEQUENCE_PARAMETER_SET **pSeqParamSet,
                    AVC_PICTURE_PARAMETER_SET **pPicParamSet,
                    MMP_UINT32 picParameSetId)
{
    MMP_UINT32 seqParameSetId;

    if(picParameSetId >= NUM_PIC_PARAM_SET)
    {
        av_log(NULL, AV_LOG_ERROR, "ERROR_NO_ACTIVE_PICTURE_SET \n");
        return ERROR_NO_ACTIVE_PICTURE_SET;
    }

    // check error of null pointer
    if(MMP_NULL == gparamSetList->PicParamSetList.pParamSet[picParameSetId])
    {
        av_log(NULL, AV_LOG_ERROR, "picParameSetId = %d\n", picParameSetId);
        return ERROR_NO_ACTIVE_PICTURE_SET;
    }

    // set active id of picture and sequence parameter set
    gparamSetList->PicParamSetList.activeParamSetId = picParameSetId;

    seqParameSetId = gparamSetList->PicParamSetList.pParamSet[picParameSetId]->seq_parameter_set_id;

    if(seqParameSetId >= NUM_SEQ_PARAM_SET)
    {
        av_log(NULL, AV_LOG_ERROR, "%s:%d ERROR_NO_ACTIVE_SEQUENCE_SET \n", __func__, __LINE__);
        return ERROR_NO_ACTIVE_SEQUENCE_SET;
    }

    if(MMP_NULL == gparamSetList->SeqParamSetList.pParamSet[seqParameSetId])
    {
        av_log(NULL, AV_LOG_ERROR, "%s:%d ERROR_NO_ACTIVE_SEQUENCE_SET \n", __func__, __LINE__);
        return ERROR_NO_ACTIVE_SEQUENCE_SET;
    }
    gparamSetList->SeqParamSetList.activeParamSetId = seqParameSetId;

    *pPicParamSet = gparamSetList->PicParamSetList.pParamSet[picParameSetId];

    *pSeqParamSet = gparamSetList->SeqParamSetList.pParamSet[gparamSetList->PicParamSetList.pParamSet[picParameSetId]->seq_parameter_set_id];

    return AVC_ERROR_SUCCESS;
}

static MMP_UINT32
sps_is_equal(AVC_SEQUENCE_PARAMETER_SET *sps1, AVC_SEQUENCE_PARAMETER_SET *sps2)
{
#if 0 // Modified by wlHsu
    if( PalMemcmp((void *)sps1, (void *)sps2, sizeof(AVC_SEQUENCE_PARAMETER_SET)) == 0 )
        return 1;
    else
        return 0;
#else
    MMP_UINT32 i;
    MMP_UINT32 equal = 1;

    //if ((!sps1->Valid) || (!sps2->Valid))
    //    return 0;

    equal &= (sps1->profile_idc == sps2->profile_idc);
    //equal &= (sps1->constrained_set0_flag == sps2->constrained_set0_flag);
    //equal &= (sps1->constrained_set1_flag == sps2->constrained_set1_flag);
    //equal &= (sps1->constrained_set2_flag == sps2->constrained_set2_flag);
    equal &= (sps1->level_idc == sps2->level_idc);
    equal &= (sps1->seq_parameter_set_id == sps2->seq_parameter_set_id);
    equal &= (sps1->log2_max_frame_num_minus4 == sps2->log2_max_frame_num_minus4);
    equal &= (sps1->pic_order_cnt_type == sps2->pic_order_cnt_type);

    if (!equal) return equal;

    if( sps1->pic_order_cnt_type == 0 )
    {
        equal &= (sps1->log2_max_pic_order_cnt_lsb_minus4 == sps2->log2_max_pic_order_cnt_lsb_minus4);
    }
    else if( sps1->pic_order_cnt_type == 1 )
    {
        equal &= (sps1->delta_pic_order_always_zero_flag == sps2->delta_pic_order_always_zero_flag);
        equal &= (sps1->offset_for_non_ref_pic == sps2->offset_for_non_ref_pic);
        equal &= (sps1->offset_for_top_to_bottom_field == sps2->offset_for_top_to_bottom_field);
        equal &= (sps1->num_ref_frames_in_pic_order_cnt_cycle == sps2->num_ref_frames_in_pic_order_cnt_cycle);
        if (!equal) return equal;

        for ( i = 0 ; i< sps1->num_ref_frames_in_pic_order_cnt_cycle ;i ++)
            equal &= (sps1->offset_for_ref_frame[i] == sps2->offset_for_ref_frame[i]);
    }

    equal &= (sps1->num_ref_frames == sps2->num_ref_frames);
    equal &= (sps1->gaps_in_frame_num_value_allowed_flag == sps2->gaps_in_frame_num_value_allowed_flag);
    equal &= (sps1->pic_width_in_mbs_minus1 == sps2->pic_width_in_mbs_minus1);
    equal &= (sps1->pic_height_in_map_units_minus1 == sps2->pic_height_in_map_units_minus1);
    equal &= (sps1->frame_mbs_only_flag == sps2->frame_mbs_only_flag);

    if (!equal) return equal;
    if( !sps1->frame_mbs_only_flag )
        equal &= (sps1->mb_adaptive_frame_field_flag == sps2->mb_adaptive_frame_field_flag);

    equal &= (sps1->direct_8x8_inference_flag == sps2->direct_8x8_inference_flag);
    equal &= (sps1->frame_cropping_flag == sps2->frame_cropping_flag);
    if (!equal) return equal;
    //  if (sps1->frame_cropping_flag)
    //  {
    //    equal &= (sps1->frame_cropping_rect_left_offset == sps2->frame_cropping_rect_left_offset);
    //    equal &= (sps1->frame_cropping_rect_right_offset == sps2->frame_cropping_rect_right_offset);
    //    equal &= (sps1->frame_cropping_rect_top_offset == sps2->frame_cropping_rect_top_offset);
    //    equal &= (sps1->frame_cropping_rect_bottom_offset == sps2->frame_cropping_rect_bottom_offset);
    //  }
    equal &= (sps1->vui_parameters_present_flag == sps2->vui_parameters_present_flag);

    return equal;
#endif
}

static MMP_UINT32
pps_is_equal(AVC_PICTURE_PARAMETER_SET *pps1, AVC_PICTURE_PARAMETER_SET *pps2)
{
#if 0 // Modified by wlHsu
    if( PalMemcmp((void *)pps1, (void *)pps2, sizeof(AVC_PICTURE_PARAMETER_SET)) == 0 )
        return 1;
    else
        return 0;
#else
    MMP_UINT32 i;
    MMP_UINT32 equal = 1;

    //if ((!pps1->Valid) || (!pps2->Valid))
    //  return 0;

    equal &= (pps1->pic_parameter_set_id == pps2->pic_parameter_set_id);
    equal &= (pps1->seq_parameter_set_id == pps2->seq_parameter_set_id);
    equal &= (pps1->entropy_coding_mode_flag == pps2->entropy_coding_mode_flag);
    equal &= (pps1->pic_order_present_flag == pps2->pic_order_present_flag);
    equal &= (pps1->num_slice_groups_minus1 == pps2->num_slice_groups_minus1);

    if (!equal) return equal;

    //  if (pps1->num_slice_groups_minus1>0)
    //  {
    //      equal &= (pps1->slice_group_map_type == pps2->slice_group_map_type);
    //      if (!equal) return equal;
    //      if (pps1->slice_group_map_type == 0)
    //      {
    //        for (i=0; i<=pps1->num_slice_groups_minus1; i++)
    //          equal &= (pps1->run_length_minus1[i] == pps2->run_length_minus1[i]);
    //      }
    //      else if( pps1->slice_group_map_type == 2 )
    //      {
    //        for (i=0; i<pps1->num_slice_groups_minus1; i++)
    //        {
    //          equal &= (pps1->top_left[i] == pps2->top_left[i]);
    //          equal &= (pps1->bottom_right[i] == pps2->bottom_right[i]);
    //        }
    //      }
    //      else if( pps1->slice_group_map_type == 3 || pps1->slice_group_map_type==4 || pps1->slice_group_map_type==5 )
    //      {
    //        equal &= (pps1->slice_group_change_direction_flag == pps2->slice_group_change_direction_flag);
    //        equal &= (pps1->slice_group_change_rate_minus1 == pps2->slice_group_change_rate_minus1);
    //      }
    //      else if( pps1->slice_group_map_type == 6 )
    //      {
    //        equal &= (pps1->num_slice_group_map_units_minus1 == pps2->num_slice_group_map_units_minus1);
    //        if (!equal) return equal;
    //        for (i=0; i<=pps1->num_slice_group_map_units_minus1; i++)
    //          equal &= (pps1->slice_group_id[i] == pps2->slice_group_id[i]);
    //      }
    //  }

    equal &= (pps1->num_ref_idx_l0_active_minus1 == pps2->num_ref_idx_l0_active_minus1);
    equal &= (pps1->num_ref_idx_l1_active_minus1 == pps2->num_ref_idx_l1_active_minus1);
    equal &= (pps1->weighted_pred_flag == pps2->weighted_pred_flag);
    equal &= (pps1->weighted_bipred_idc == pps2->weighted_bipred_idc);
    equal &= (pps1->pic_init_qp_minus26 == pps2->pic_init_qp_minus26);
    equal &= (pps1->pic_init_qs_minus26 == pps2->pic_init_qs_minus26);
    equal &= (pps1->chroma_qp_index_offset == pps2->chroma_qp_index_offset);
    equal &= (pps1->deblocking_filter_control_present_flag == pps2->deblocking_filter_control_present_flag);
    equal &= (pps1->constrained_intra_pred_flag == pps2->constrained_intra_pred_flag);
    equal &= (pps1->redundant_pic_cnt_present_flag == pps2->redundant_pic_cnt_present_flag);

    return equal;
#endif
}

static void scaling_list(MMP_UINT8 *scalingList, MMP_UINT32 sizeOfScalingList, MMP_UINT32 *UseDefaultScalingMatrix)
{
    MMP_UINT32 usedBits = 0;
    MMP_UINT32 lastScale = 8;
    MMP_UINT32 nextScale = 8;
    MMP_UINT32 j = 0;
    MMP_UINT32 scanj = 0;
    MMP_INT32 delta_scale = 0;

    for(j=0; j<sizeOfScalingList; j++)
    {
        scanj = (sizeOfScalingList==16) ? ZZ_SCAN[j]:ZZ_SCAN8[j];
        if(0 != nextScale)
        {
            delta_scale = se_v(&usedBits);
            nextScale = (lastScale + delta_scale + 256) % 256;
            *UseDefaultScalingMatrix = (scanj==0 && nextScale==0);
        }
        scalingList[scanj] = (nextScale==0) ? lastScale:nextScale;
        lastScale = scalingList[scanj];
    }
}

//////////////////////////////////////////////////////////////////////////////////
// Move from C-Model
void free_marking_buffer(void)
{
    DecRefPicMarking_t *tmp_drpm;

    // free old buffer content
    if (dec_picture != MMP_NULL)
        while (dec_picture->dec_ref_pic_marking_buffer)
        {
            tmp_drpm=dec_picture->dec_ref_pic_marking_buffer;
            dec_picture->dec_ref_pic_marking_buffer=tmp_drpm->Next;
            free(tmp_drpm);
        }

    while (img->dec_ref_pic_marking_buffer)
    {
        tmp_drpm=img->dec_ref_pic_marking_buffer;
        img->dec_ref_pic_marking_buffer=tmp_drpm->Next;
        free(tmp_drpm);
    }
}

void free_storable_picture(StorablePicture* p)
{
    if (p)
    {
        free(p);
    }
}

void free_frame_store(FrameStore* f)
{
    if (f)
    {
        if (f->frame)
        {
            free_storable_picture(f->frame);
            f->frame=NULL;
        }
        if (f->top_field)
        {
            free_storable_picture(f->top_field);
            f->top_field=NULL;
        }
        if (f->bottom_field)
        {
            free_storable_picture(f->bottom_field);
            f->bottom_field=NULL;
        }

        //free(f);
    }
}

void reset_frame_store(FrameStore* f)
{
    if (MMP_NULL == f)
    {
        av_log(NULL, AV_LOG_INFO, "Fail reset frame store\n");
        return;
    }

    f->is_used      = 0;
    f->is_reference = 0;
    f->is_long_term = 0;
    f->is_orig_reference = 0;

    f->is_output = 0;

    f->frame        = NULL;;
    f->top_field    = NULL;
    f->bottom_field = NULL;

    return;
}

FrameStore* alloc_frame_store()
{
    FrameStore *f;

    f = (FrameStore *)calloc (1, sizeof(FrameStore));
    if (MMP_NULL == f)
    {
        av_log(NULL, AV_LOG_ERROR, "Fail allocate f\n");
        return MMP_NULL;
    }

    f->is_used      = 0;
    f->is_reference = 0;
    f->is_long_term = 0;
    f->is_orig_reference = 0;

    f->is_output = 0;

    f->frame        = NULL;;
    f->top_field    = NULL;
    f->bottom_field = NULL;

    return f;
}

static void unmark_for_reference(FrameStore* fs)
{
    if (fs->is_used & 1)
    {
        fs->top_field->used_for_reference = 0;
    }
    if (fs->is_used & 2)
    {
        fs->bottom_field->used_for_reference = 0;
    }
    if (fs->is_used == 3)
    {
        fs->top_field->used_for_reference = 0;
        fs->bottom_field->used_for_reference = 0;
        fs->frame->used_for_reference = 0;
    }

    fs->is_reference = 0;
}

static int is_used_for_reference(FrameStore* fs)
{
    if (fs->is_reference)
    {
        return 1;
    }

    if (fs->is_used==3) // frame
    {
        if (fs->frame->used_for_reference)
        {
            return 1;
        }
    }

    if (fs->is_used&1) // top field
    {
        if (fs->top_field->used_for_reference)
        {
            return 1;
        }
    }

    if (fs->is_used&2) // bottom field
    {
        if (fs->bottom_field->used_for_reference)
        {
            return 1;
        }
    }
    return 0;
}

static void remove_frame_from_dpb(int pos)
{
    FrameStore* fs = dpb.fs[pos];
    FrameStore* tmp;
    unsigned i;

    PicIdxQueue[fs->pic_idx>>1] = 0;
    DecodingQueue[fs->decoding_buf_idx] &= 0xFFFF0000;

    switch (fs->is_used)
    {
    case 3:
        free_storable_picture(fs->frame);
        free_storable_picture(fs->top_field);
        free_storable_picture(fs->bottom_field);
        fs->frame=NULL;
        fs->top_field=NULL;
        fs->bottom_field=NULL;
        break;
    case 2:
        free_storable_picture(fs->bottom_field);
        fs->bottom_field=NULL;
        break;
    case 1:
        free_storable_picture(fs->top_field);
        fs->top_field=NULL;
        break;
    case 0:
        break;
    default:
        //error("invalid frame store type",500);
        break;
    }
    fs->is_used = 0;
    fs->is_long_term = 0;
    fs->is_reference = 0;
    fs->is_orig_reference = 0;

    fs->pic_idx = 0;
    fs->decoding_buf_idx = 0;

    // move empty framestore to end of buffer
    tmp = dpb.fs[pos];

    for (i=pos; i<dpb.used_size-1;i++)
    {
        dpb.fs[i] = dpb.fs[i+1];
    }
    dpb.fs[dpb.used_size-1] = tmp;
    dpb.used_size--;
}

static int remove_unused_frame_from_dpb()
{
    unsigned i;

    // check for frames that were already output and no longer used for reference
    for (i=0; i<dpb.used_size; i++)
    {
        //if (dpb.fs[i]->is_output && (!is_used_for_reference(dpb.fs[i])))
        if ( !is_used_for_reference(dpb.fs[i]) )
        {
            remove_frame_from_dpb(i);
            return 1;
        }
    }
    return 0;
}

static void get_smallest_poc(int *poc,int * pos)
{
    unsigned i;

    if (dpb.used_size<1)
    {
        printf("Cannot determine smallest POC, DPB empty.\n");
    }

    *pos=-1;
    *poc = INT_MAX;
    for (i=0; i<dpb.used_size; i++)
    {
        if ((*poc>dpb.fs[i]->poc)&&(!dpb.fs[i]->is_output))
        {
            *poc = dpb.fs[i]->poc;
            *pos=i;
        }
    }
}

static void get_smallest_poc_ex(int *poc,int * pos)
{
    unsigned i;

    if (dpb.used_size<1)
    {
        printf("Cannot determine smallest POC, DPB empty.\n");
    }

    *pos=-1;
    *poc = INT_MAX;
    for (i=0; i<dpb.used_size; i++)
    {
        //if ((*poc>dpb.fs[i]->poc)&&(!dpb.fs[i]->is_output))
        if (*poc>dpb.fs[i]->poc)
        {
            *poc = dpb.fs[i]->poc;
            *pos=i;
        }
    }
}

static void output_one_frame_from_dpb()
{
    int poc, pos;

    //diagnostics
    if (dpb.used_size<1)
    {
        printf("Cannot output frame, DPB empty.\n");
    }

    // find smallest POC
    get_smallest_poc(&poc, &pos);

    if (pos==-1)
    {
        printf("no frames for output available : A\n");

        get_smallest_poc_ex(&poc, &pos);

        if (pos==-1)
        {
            printf("no frames for output available : B\n");
            return;
        }

        dpb.fs[pos]->is_output = 1;

        dpb.last_output_poc = poc;

        remove_frame_from_dpb(pos);

        return;
    }

    //write_stored_frame(dpb.fs[pos], p_out);
    //temp solution correct?
    dpb.fs[pos]->is_output = 1;

    //if (dpb.last_output_poc >= poc)
    //{
    //    printf("output POC must be in ascending order\n");
    //}
    dpb.last_output_poc = poc;
    // free frame store and move empty store to end of buffer
    if (!is_used_for_reference(dpb.fs[pos]))
    {
        remove_frame_from_dpb(pos);
    }
}

static int is_short_term_reference(FrameStore* fs)
{
    if (fs->is_used==3) // frame
    {
        if ((fs->frame->used_for_reference)&&(!fs->frame->is_long_term))
        {
            return 1;
        }
    }

    if (fs->is_used&1) // top field
    {
        if ((fs->top_field->used_for_reference)&&(!fs->top_field->is_long_term))
        {
            return 1;
        }
    }

    if (fs->is_used&2) // bottom field
    {
        if ((fs->bottom_field->used_for_reference)&&(!fs->bottom_field->is_long_term))
        {
            return 1;
        }
    }
    return 0;
}

void update_ref_list()
{
    unsigned i, j;
    int poc, pos;

    for (i=0, j=0; i<dpb.used_size; i++)
    {
        if (is_short_term_reference(dpb.fs[i]))
        {
            dpb.fs_ref[j++]=dpb.fs[i];
        }
    }

#if 1
    pos=-1;
    poc = INT_MAX;

    for (i=0; i<j; i++)
    {
        if (poc>dpb.fs_ref[i]->pic_count)
        {
            poc = dpb.fs_ref[i]->pic_count;
            pos=i;
        }
    }

    if (pos != -1)
    {
        if ((j > dpb.num_ref_frames) || ((gDePicCount - poc) > 200))
        {
            remove_frame_from_dpb(pos);
            for (i=0, j=0; i<dpb.used_size; i++)
            {
                if (is_short_term_reference(dpb.fs[i]))
                {
                    dpb.fs_ref[j++]=dpb.fs[i];
                }
            }
        }
    }
#endif

    dpb.ref_frames_in_buffer = j;

    while (j<dpb.size)
    {
        dpb.fs_ref[j++]=NULL;
    }
}

static int is_long_term_reference(FrameStore* fs)
{
    if (fs->is_used==3) // frame
    {
        if ((fs->frame->used_for_reference)&&(fs->frame->is_long_term))
        {
            return 1;
        }
    }

    if (fs->is_used&1) // top field
    {
        if ((fs->top_field->used_for_reference)&&(fs->top_field->is_long_term))
        {
            return 1;
        }
    }

    if (fs->is_used&2) // bottom field
    {
        if ((fs->bottom_field->used_for_reference)&&(fs->bottom_field->is_long_term))
        {
            return 1;
        }
    }
    return 0;
}

void update_ltref_list()
{
    unsigned i, j;

    for (i=0, j=0; i<dpb.used_size; i++)
    {
        if (is_long_term_reference(dpb.fs[i]))
        {
            dpb.fs_ltref[j++]=dpb.fs[i];
        }
    }

    dpb.ltref_frames_in_buffer=j;

    while (j<dpb.size)
    {
        dpb.fs_ltref[j++]=NULL;
    }
}

void flush_dpb()
{
    unsigned i;

    // mark all frames unused
    for (i=0; i<dpb.used_size; i++)
    {
        unmark_for_reference (dpb.fs[i]);
    }

    while (remove_unused_frame_from_dpb()) ;

    // output frames in POC order
    while (dpb.used_size)
    {
        output_one_frame_from_dpb();
    }

    dpb.last_output_poc = INT_MIN;
}

static void idr_memory_management(StorablePicture* p)
{
    unsigned i;

    if (p->no_output_of_prior_pics_flag)
    {
        // free all stored pictures
        for (i=0; i<dpb.used_size; i++)
        {
            // reset all reference settings
            free_frame_store(dpb.fs[i]);
            //dpb.fs[i] = alloc_frame_store();
            reset_frame_store(dpb.fs[i]);
        }
        for (i=0; i<dpb.ref_frames_in_buffer; i++)
        {
            dpb.fs_ref[i]=NULL;
        }
        for (i=0; i<dpb.ltref_frames_in_buffer; i++)
        {
            dpb.fs_ltref[i]=NULL;
        }
        dpb.used_size=0;
    }
    else
    {
        flush_dpb();
    }
    dpb.last_picture = NULL;

    update_ref_list();
    update_ltref_list();
    dpb.last_output_poc = INT_MIN;

    if (p->long_term_reference_flag)
    {
        dpb.max_long_term_pic_idx = 0;
        p->is_long_term           = 1;
        p->long_term_frame_idx    = 0;
    }
    else
    {
        dpb.max_long_term_pic_idx = -1;
        p->is_long_term           = 0;
    }
}

static int get_pic_num_x (StorablePicture *p, int difference_of_pic_nums_minus1)
{
    int currPicNum;

    if (p->structure == FRAME)
        currPicNum = p->frame_num;
    else
        currPicNum = 2 * p->frame_num + 1;

    return currPicNum - (difference_of_pic_nums_minus1 + 1);
}

static void mm_unmark_short_term_for_reference(StorablePicture *p, int difference_of_pic_nums_minus1)
{
    int picNumX;
    unsigned i;

    picNumX = get_pic_num_x(p, difference_of_pic_nums_minus1);

    for (i=0; i<dpb.ref_frames_in_buffer; i++)
    {
        if (p->structure == FRAME)
        {
            if ((dpb.fs_ref[i]->is_reference==3) && (dpb.fs_ref[i]->is_long_term==0))
            {
                if (dpb.fs_ref[i]->frame->pic_num == picNumX)
                {
                    unmark_for_reference(dpb.fs_ref[i]);
                    return;
                }
            }
        }
        else
        {
            if ((dpb.fs_ref[i]->is_reference & 1) && (!(dpb.fs_ref[i]->is_long_term & 1)))
            {
                if (dpb.fs_ref[i]->top_field->pic_num == picNumX)
                {
                    dpb.fs_ref[i]->top_field->used_for_reference = 0;
                    dpb.fs_ref[i]->is_reference &= 2;
                    if (dpb.fs_ref[i]->is_used == 3)
                    {
                        dpb.fs_ref[i]->frame->used_for_reference = 0;
                    }
                    return;
                }
            }
            if ((dpb.fs_ref[i]->is_reference & 2) && (!(dpb.fs_ref[i]->is_long_term & 2)))
            {
                if (dpb.fs_ref[i]->bottom_field->pic_num == picNumX)
                {
                    dpb.fs_ref[i]->bottom_field->used_for_reference = 0;
                    dpb.fs_ref[i]->is_reference &= 1;
                    if (dpb.fs_ref[i]->is_used == 3)
                    {
                        dpb.fs_ref[i]->frame->used_for_reference = 0;
                    }
                    return;
                }
            }
        }
    }
}

static void unmark_for_long_term_reference(FrameStore* fs)
{
    if (fs->is_used & 1)
    {
        fs->top_field->used_for_reference = 0;
        fs->top_field->is_long_term = 0;
    }
    if (fs->is_used & 2)
    {
        fs->bottom_field->used_for_reference = 0;
        fs->bottom_field->is_long_term = 0;
    }
    if (fs->is_used == 3)
    {
        fs->top_field->used_for_reference = 0;
        fs->top_field->is_long_term = 0;
        fs->bottom_field->used_for_reference = 0;
        fs->bottom_field->is_long_term = 0;
        fs->frame->used_for_reference = 0;
        fs->frame->is_long_term = 0;
    }

    fs->is_reference = 0;
    fs->is_long_term = 0;
}

static void mm_unmark_long_term_for_reference(StorablePicture *p, int long_term_pic_num)
{
    unsigned i;

    for (i=0; i<dpb.ltref_frames_in_buffer; i++)
    {
        if (p->structure == FRAME)
        {
            if ((dpb.fs_ltref[i]->is_reference==3) && (dpb.fs_ltref[i]->is_long_term==3))
            {
                if (dpb.fs_ltref[i]->frame->long_term_pic_num == long_term_pic_num)
                {
                  unmark_for_long_term_reference(dpb.fs_ltref[i]);
                }
            }
        }
        else
        {
            if ((dpb.fs_ltref[i]->is_reference & 1) && ((dpb.fs_ltref[i]->is_long_term & 1)))
            {
                if (dpb.fs_ltref[i]->top_field->long_term_pic_num == long_term_pic_num)
                {
                    dpb.fs_ltref[i]->top_field->used_for_reference = 0;
                    dpb.fs_ltref[i]->top_field->is_long_term = 0;
                    dpb.fs_ltref[i]->is_reference &= 2;
                    dpb.fs_ltref[i]->is_long_term &= 2;
                    if (dpb.fs_ltref[i]->is_used == 3)
                    {
                        dpb.fs_ltref[i]->frame->used_for_reference = 0;
                        dpb.fs_ltref[i]->frame->is_long_term = 0;
                    }
                    return;
                }
            }
            if ((dpb.fs_ltref[i]->is_reference & 2) && ((dpb.fs_ltref[i]->is_long_term & 2)))
            {
                if (dpb.fs_ltref[i]->bottom_field->long_term_pic_num == long_term_pic_num)
                {
                    dpb.fs_ltref[i]->bottom_field->used_for_reference = 0;
                    dpb.fs_ltref[i]->bottom_field->is_long_term = 0;
                    dpb.fs_ltref[i]->is_reference &= 1;
                    dpb.fs_ltref[i]->is_long_term &= 1;
                    if (dpb.fs_ltref[i]->is_used == 3)
                    {
                        dpb.fs_ltref[i]->frame->used_for_reference = 0;
                        dpb.fs_ltref[i]->frame->is_long_term = 0;
                    }
                    return;
                }
            }
        }
    }
}

static void unmark_long_term_frame_for_reference_by_frame_idx(int long_term_frame_idx)
{
    unsigned i;

    for (i=0; i<dpb.ltref_frames_in_buffer; i++)
    {
        if (dpb.fs_ltref[i]->long_term_frame_idx == long_term_frame_idx)
            unmark_for_long_term_reference(dpb.fs_ltref[i]);
    }
}

static void unmark_long_term_field_for_reference_by_frame_idx(PictureStructure structure, int long_term_frame_idx, int mark_current, unsigned curr_frame_num, int curr_pic_num)
{
    unsigned i;

    if (curr_pic_num<0)
        curr_pic_num += (img->MaxFrameNum << 1);

    for (i=0; i<dpb.ltref_frames_in_buffer; i++)
    {
        if (dpb.fs_ltref[i]->long_term_frame_idx == long_term_frame_idx)
        {
            if (structure == TOP_FIELD)
            {
                if ((dpb.fs_ltref[i]->is_long_term == 3))
                {
                    unmark_for_long_term_reference(dpb.fs_ltref[i]);
                }
                else
                {
                    if ((dpb.fs_ltref[i]->is_long_term == 1))
                    {
                        unmark_for_long_term_reference(dpb.fs_ltref[i]);
                    }
                    else
                    {
                        if (mark_current)
                        {
                            if (dpb.last_picture)
                            {
                                if ( ( dpb.last_picture != dpb.fs_ltref[i] )|| dpb.last_picture->frame_num != curr_frame_num)
                                    unmark_for_long_term_reference(dpb.fs_ltref[i]);
                            }
                            else
                            {
                                unmark_for_long_term_reference(dpb.fs_ltref[i]);
                            }
                        }
                        else
                        {
                            if ((dpb.fs_ltref[i]->frame_num) != (unsigned)(curr_pic_num >> 1))
                            {
                                unmark_for_long_term_reference(dpb.fs_ltref[i]);
                            }
                        }
                    }
                }
            }
            if (structure == BOTTOM_FIELD)
            {
                if ((dpb.fs_ltref[i]->is_long_term == 3))
                {
                    unmark_for_long_term_reference(dpb.fs_ltref[i]);
                }
                else
                {
                    if ((dpb.fs_ltref[i]->is_long_term == 2))
                    {
                        unmark_for_long_term_reference(dpb.fs_ltref[i]);
                    }
                    else
                    {
                        if (mark_current)
                        {
                            if (dpb.last_picture)
                            {
                                if ( ( dpb.last_picture != dpb.fs_ltref[i] )|| dpb.last_picture->frame_num != curr_frame_num)
                                    unmark_for_long_term_reference(dpb.fs_ltref[i]);
                            }
                            else
                            {
                                 unmark_for_long_term_reference(dpb.fs_ltref[i]);
                            }
                        }
                        else
                        {
                            if ((dpb.fs_ltref[i]->frame_num) != (unsigned)(curr_pic_num/2))
                            {
                                unmark_for_long_term_reference(dpb.fs_ltref[i]);
                            }
                        }
                    }
                }
            }
        }
    }
}

static void mark_pic_long_term(StorablePicture* p, int long_term_frame_idx, int picNumX)
{
    unsigned i;
    int add_top, add_bottom;

    if (p->structure == FRAME)
    {
        for (i=0; i<dpb.ref_frames_in_buffer; i++)
        {
            if (dpb.fs_ref[i]->is_reference == 3)
            {
                if ((!dpb.fs_ref[i]->frame->is_long_term)&&(dpb.fs_ref[i]->frame->pic_num == picNumX))
                {
                    dpb.fs_ref[i]->long_term_frame_idx = dpb.fs_ref[i]->frame->long_term_frame_idx
                                                       = dpb.fs_ref[i]->top_field->long_term_frame_idx
                                                       = dpb.fs_ref[i]->bottom_field->long_term_frame_idx
                                                       = long_term_frame_idx;
                    dpb.fs_ref[i]->frame->long_term_pic_num = dpb.fs_ref[i]->top_field->long_term_pic_num
                                                            = dpb.fs_ref[i]->bottom_field->long_term_pic_num
                                                            = long_term_frame_idx;
                    dpb.fs_ref[i]->frame->is_long_term = dpb.fs_ref[i]->top_field->is_long_term
                                                       = dpb.fs_ref[i]->bottom_field->is_long_term
                                                       = 1;
                    dpb.fs_ref[i]->is_long_term = 3;
                    return;
                }
            }
        }
        printf ("Warning: reference frame for long term marking not found\n");
    }
    else
    {
        if (p->structure == TOP_FIELD)
        {
            add_top    = 1;
            add_bottom = 0;
        }
        else
        {
            add_top    = 0;
            add_bottom = 1;
        }
        for (i=0; i<dpb.ref_frames_in_buffer; i++)
        {
            if (dpb.fs_ref[i]->is_reference & 1)
            {
                if ((!dpb.fs_ref[i]->top_field->is_long_term)&&(dpb.fs_ref[i]->top_field->pic_num == picNumX))
                {
                    if ((dpb.fs_ref[i]->is_long_term) && (dpb.fs_ref[i]->long_term_frame_idx != long_term_frame_idx))
                    {
                        printf ("Warning: assigning long_term_frame_idx different from other field\n");
                    }

                    dpb.fs_ref[i]->long_term_frame_idx = dpb.fs_ref[i]->top_field->long_term_frame_idx
                                                       = long_term_frame_idx;
                    dpb.fs_ref[i]->top_field->long_term_pic_num = 2 * long_term_frame_idx + add_top;
                    dpb.fs_ref[i]->top_field->is_long_term = 1;
                    dpb.fs_ref[i]->is_long_term |= 1;
                    if (dpb.fs_ref[i]->is_long_term == 3)
                    {
                        dpb.fs_ref[i]->frame->is_long_term = 1;
                        dpb.fs_ref[i]->frame->long_term_frame_idx = dpb.fs_ref[i]->frame->long_term_pic_num = long_term_frame_idx;
                    }
                    return;
                }
            }
            if (dpb.fs_ref[i]->is_reference & 2)
            {
                if ((!dpb.fs_ref[i]->bottom_field->is_long_term)&&(dpb.fs_ref[i]->bottom_field->pic_num == picNumX))
                {
                    if ((dpb.fs_ref[i]->is_long_term) && (dpb.fs_ref[i]->long_term_frame_idx != long_term_frame_idx))
                    {
                        printf ("Warning: assigning long_term_frame_idx different from other field\n");
                    }

                    dpb.fs_ref[i]->long_term_frame_idx = dpb.fs_ref[i]->bottom_field->long_term_frame_idx
                                                       = long_term_frame_idx;
                    dpb.fs_ref[i]->bottom_field->long_term_pic_num = 2 * long_term_frame_idx + add_top;
                    dpb.fs_ref[i]->bottom_field->is_long_term = 1;
                    dpb.fs_ref[i]->is_long_term |= 2;
                    if (dpb.fs_ref[i]->is_long_term == 3)
                    {
                        dpb.fs_ref[i]->frame->is_long_term = 1;
                        dpb.fs_ref[i]->frame->long_term_frame_idx = dpb.fs_ref[i]->frame->long_term_pic_num = long_term_frame_idx;
                    }
                    return;
                }
            }
        }
        printf ("Warning: reference field for long term marking not found\n");
    }
}

static void mm_assign_long_term_frame_idx(StorablePicture* p, int difference_of_pic_nums_minus1, int long_term_frame_idx)
{
    int picNumX;

    picNumX = get_pic_num_x(p, difference_of_pic_nums_minus1);

    // remove frames/fields with same long_term_frame_idx
    if (p->structure == FRAME)
    {
        unmark_long_term_frame_for_reference_by_frame_idx(long_term_frame_idx);
    }
    else
    {
        unsigned i;
        PictureStructure structure = FRAME;

        for (i=0; i<dpb.ref_frames_in_buffer; i++)
        {
            if (dpb.fs_ref[i]->is_reference & 1)
            {
                if (dpb.fs_ref[i]->top_field->pic_num == picNumX)
                {
                  structure = TOP_FIELD;
                  break;
                }
            }
            if (dpb.fs_ref[i]->is_reference & 2)
            {
                if (dpb.fs_ref[i]->bottom_field->pic_num == picNumX)
                {
                  structure = BOTTOM_FIELD;
                  break;
                }
            }
        }
        if (structure==FRAME)
        {
            printf("field for long term marking not found\n");
        }

        unmark_long_term_field_for_reference_by_frame_idx(structure, long_term_frame_idx, 0, 0, picNumX);
    }

    mark_pic_long_term(p, long_term_frame_idx, picNumX);
}

void mm_update_max_long_term_frame_idx(int max_long_term_frame_idx_plus1)
{
    unsigned i;

    dpb.max_long_term_pic_idx = max_long_term_frame_idx_plus1 - 1;

    // check for invalid frames
    for (i=0; i<dpb.ltref_frames_in_buffer; i++)
    {
        if (dpb.fs_ltref[i]->long_term_frame_idx > dpb.max_long_term_pic_idx)
        {
            unmark_for_long_term_reference(dpb.fs_ltref[i]);
        }
    }
}

static void mm_unmark_all_short_term_for_reference ()
{
    unsigned int i;

    for (i=0; i<dpb.ref_frames_in_buffer; i++)
    {
        unmark_for_reference(dpb.fs_ref[i]);
    }
    update_ref_list();
}

static void mm_unmark_all_long_term_for_reference ()
{
    mm_update_max_long_term_frame_idx(0);
}

static void mm_mark_current_picture_long_term(StorablePicture *p, int long_term_frame_idx)
{
    // remove long term pictures with same long_term_frame_idx
    if (p->structure == FRAME)
    {
        unmark_long_term_frame_for_reference_by_frame_idx(long_term_frame_idx);
    }
    else
    {
        unmark_long_term_field_for_reference_by_frame_idx(p->structure, long_term_frame_idx, 1, p->pic_num, 0);
    }

    p->is_long_term = 1;
    p->long_term_frame_idx = long_term_frame_idx;
}

static void adaptive_memory_management(StorablePicture* p)
{
    DecRefPicMarking_t *tmp_drpm;

    img->last_has_mmco_5 = 0;

    //assert (!p->idr_flag);
    //assert (p->adaptive_ref_pic_buffering_flag);

    while (p->dec_ref_pic_marking_buffer)
    {
        tmp_drpm = p->dec_ref_pic_marking_buffer;
        switch (tmp_drpm->memory_management_control_operation)
        {
        case 0:
            if (tmp_drpm->Next != NULL)
            {
                av_log(NULL, AV_LOG_ERROR, "\nmemory_management_control_operation = 0 not last operation in buffer !! \n");
                //error ("memory_management_control_operation = 0 not last operation in buffer", 500);
            }
            break;
        case 1:
            mm_unmark_short_term_for_reference(p, tmp_drpm->difference_of_pic_nums_minus1);
            update_ref_list();
            break;
        case 2:
            mm_unmark_long_term_for_reference(p, tmp_drpm->long_term_pic_num);
            update_ltref_list();
            break;
        case 3:
            mm_assign_long_term_frame_idx(p, tmp_drpm->difference_of_pic_nums_minus1, tmp_drpm->long_term_frame_idx);
            update_ref_list();
            update_ltref_list();
            break;
        case 4:
            mm_update_max_long_term_frame_idx (tmp_drpm->max_long_term_frame_idx_plus1);
            update_ltref_list();
            break;
        case 5:
            mm_unmark_all_short_term_for_reference();
            mm_unmark_all_long_term_for_reference();
            img->last_has_mmco_5 = 1;
            break;
        case 6:
            mm_mark_current_picture_long_term(p, tmp_drpm->long_term_frame_idx);
            break;
        default:
            //error ("invalid memory_management_control_operation in buffer", 500);
            break;
        }
        p->dec_ref_pic_marking_buffer = tmp_drpm->Next;
        free (tmp_drpm);
    }
    if ( img->last_has_mmco_5 )
    {
        p->pic_num = p->frame_num = 0;
        p->poc = 0;
        img->ThisPOC=0;

        switch (p->structure)
        {
        case TOP_FIELD:
            {
                img->toppoc=0;
                break;
            }
        case BOTTOM_FIELD:
            {
                img->bottompoc=0;
                break;
            }
        case FRAME:
            {
                img->framepoc=0;
                break;
            }
        }
        flush_dpb();
    }
}

StorablePicture* alloc_storable_picture(PictureStructure structure)
{
    StorablePicture *s;

    s = (StorablePicture *)calloc (1, sizeof(StorablePicture));
    if (s==MMP_NULL)
        av_log(NULL, AV_LOG_ERROR, "\nFail allocate s\n");

    s->pic_num=0;
    s->frame_num=0;
    s->long_term_frame_idx=0;
    s->long_term_pic_num=0;
    s->used_for_reference=0;
    s->is_long_term=0;
    s->non_existing=0;
    s->is_output = 0;
    //s->max_slice_id = 0;

    s->structure=structure;

    s->top_field    = NULL;
    s->bottom_field = NULL;
    s->frame        = NULL;

    s->dec_ref_pic_marking_buffer = NULL;

    s->coded_frame                   = 0;
    s->MbaffFrameFlag                = 0;

    return s;
}

void dpb_split_field(FrameStore *fs)
{
    fs->top_field    = alloc_storable_picture(TOP_FIELD);
    fs->bottom_field = alloc_storable_picture(BOTTOM_FIELD);
    fs->poc = fs->top_field->poc
            = fs->frame->poc;

    fs->top_field->frame_poc =  fs->frame->frame_poc;

    fs->bottom_field->poc =  fs->frame->bottom_poc;
    fs->top_field->bottom_poc =fs->bottom_field->bottom_poc =  fs->frame->bottom_poc;
    fs->top_field->top_poc =fs->bottom_field->top_poc =  fs->frame->top_poc;
    fs->bottom_field->frame_poc =  fs->frame->frame_poc;

    fs->top_field->used_for_reference = fs->bottom_field->used_for_reference
                                      = fs->frame->used_for_reference;
    fs->top_field->is_long_term = fs->bottom_field->is_long_term
                                = fs->frame->is_long_term;
    fs->long_term_frame_idx = fs->top_field->long_term_frame_idx
                            = fs->bottom_field->long_term_frame_idx
                            = fs->frame->long_term_frame_idx;

    fs->top_field->coded_frame = fs->bottom_field->coded_frame = 1;
    fs->top_field->MbaffFrameFlag = fs->bottom_field->MbaffFrameFlag
                                  = fs->frame->MbaffFrameFlag;

    fs->frame->top_field    = fs->top_field;
    fs->frame->bottom_field = fs->bottom_field;

    fs->top_field->bottom_field = fs->bottom_field;
    fs->top_field->frame        = fs->frame;
    fs->bottom_field->top_field = fs->top_field;
    fs->bottom_field->frame     = fs->frame;

    fs->top_field->decoding_buf_idx =  fs->frame->decoding_buf_idx;
    fs->bottom_field->decoding_buf_idx = fs->frame->decoding_buf_idx;
    fs->top_field->col_data_buf_idx =  fs->frame->col_data_buf_idx;
    fs->bottom_field->col_data_buf_idx = fs->frame->col_data_buf_idx;
    fs->top_field->pic_idx =  fs->frame->pic_idx;
    fs->bottom_field->pic_idx = fs->frame->pic_idx+1;
    fs->top_field->pic_count =  fs->frame->pic_count;
    fs->bottom_field->pic_count = fs->frame->pic_count;
}

void dpb_combine_field(FrameStore *fs)
{
    fs->frame = alloc_storable_picture(FRAME);

    fs->poc=fs->frame->poc =fs->frame->frame_poc = min (fs->top_field->poc, fs->bottom_field->poc);

    fs->bottom_field->frame_poc=fs->top_field->frame_poc=
    fs->bottom_field->top_poc=fs->frame->frame_poc=fs->frame->top_poc=fs->top_field->poc;
    fs->top_field->bottom_poc=fs->bottom_field->poc;

    fs->frame->bottom_poc=fs->bottom_field->poc;

    fs->frame->used_for_reference = (fs->top_field->used_for_reference && fs->bottom_field->used_for_reference );
    fs->frame->is_long_term = (fs->top_field->is_long_term && fs->bottom_field->is_long_term );

    if (fs->frame->is_long_term)
        fs->frame->long_term_frame_idx = fs->long_term_frame_idx;

    fs->frame->top_field    = fs->top_field;
    fs->frame->bottom_field = fs->bottom_field;

    fs->frame->coded_frame = 0;

    fs->top_field->frame = fs->bottom_field->frame = fs->frame;

    fs->frame->decoding_buf_idx = fs->top_field->decoding_buf_idx;
    fs->frame->col_data_buf_idx = fs->top_field->col_data_buf_idx;
    fs->frame->pic_idx = fs->top_field->pic_idx;

    fs->frame->pic_count = fs->top_field->pic_count;
}

static void insert_picture_in_dpb(FrameStore* fs, StorablePicture* p)
{
    switch (p->structure)
    {
    case FRAME:
        fs->frame = p;
        fs->is_used = 3;
        if (p->used_for_reference)
        {
            fs->is_reference = 3;
            fs->is_orig_reference = 3;
            if (p->is_long_term)
            {
                fs->is_long_term = 3;
            }
        }
        // generate field views
        dpb_split_field(fs);
        break;
    case TOP_FIELD:
        fs->top_field = p;
        fs->is_used |= 1;
        if (p->used_for_reference)
        {
            fs->is_reference |= 1;
            fs->is_orig_reference |= 1;
            if (p->is_long_term)
            {
                fs->is_long_term |= 1;
                fs->long_term_frame_idx = p->long_term_frame_idx;
            }
        }
        if (fs->is_used == 3)
        {
            // generate frame view
            dpb_combine_field(fs);
        }
        else
        {
            fs->poc = p->poc;
            //gen_field_ref_ids(p);
        }
        break;
    case BOTTOM_FIELD:
        fs->bottom_field = p;
        fs->is_used |= 2;
        if (p->used_for_reference)
        {
            fs->is_reference |= 2;
            fs->is_orig_reference |= 2;
            if (p->is_long_term)
            {
                fs->is_long_term |= 2;
                fs->long_term_frame_idx = p->long_term_frame_idx;
            }
        }
        if (fs->is_used == 3)
        {
            // generate frame view
            dpb_combine_field(fs);
        }
        else
        {
            fs->poc = p->poc;
            //gen_field_ref_ids(p);
        }
        break;
    }
    fs->frame_num = p->pic_num;
    fs->is_output = p->is_output;

    //if (fs->is_used==3)
    //{
    //  if (-1!=p_ref)
    //    find_snr(snr, fs->frame, p_ref);
    //}

    fs->decoding_buf_idx = p->decoding_buf_idx;
    fs->pic_count = p->pic_count;
    fs->pic_idx = p->pic_idx;
    DecodingQueue[p->decoding_buf_idx] |= STORE_IN_DBP_BUF;
}

static void sliding_window_memory_management(StorablePicture* p)
{
    unsigned i;

    //assert (!p->idr_flag);
    // if this is a reference pic with sliding sliding window, unmark first ref frame
    if (dpb.ref_frames_in_buffer==dpb.num_ref_frames - dpb.ltref_frames_in_buffer)
    {
        for (i=0; i<dpb.used_size;i++)
        {
            if (dpb.fs[i]->is_reference  && (!(dpb.fs[i]->is_long_term)))
            {
                unmark_for_reference(dpb.fs[i]);
                update_ref_list();
                break;
            }
        }
    }

    p->is_long_term = 0;
}

void store_picture_in_dpb(StorablePicture* p)
{
    unsigned i;
    int poc, pos;

    img->last_has_mmco_5=0;
    img->last_pic_bottom_field = (p->structure==BOTTOM_FIELD);

    if (p->idr_flag)
        idr_memory_management(p);
    else
    {
        // adaptive memory management
        if (p->used_for_reference && (p->adaptive_ref_pic_buffering_flag))
            adaptive_memory_management(p);
    }
    if ((p->structure==TOP_FIELD)||(p->structure==BOTTOM_FIELD))
    {
        // check for frame store with same pic_number
        if (dpb.last_picture)
        {
            if ((int)dpb.last_picture->frame_num == p->pic_num)
            {
                if (((p->structure==TOP_FIELD)&&(dpb.last_picture->is_used==2))||((p->structure==BOTTOM_FIELD)&&(dpb.last_picture->is_used==1)))
                {
                    if ((p->used_for_reference && (dpb.last_picture->is_orig_reference!=0))||
                        (!p->used_for_reference && (dpb.last_picture->is_orig_reference==0)))
                    {
                        insert_picture_in_dpb(dpb.last_picture, p);
                        update_ref_list();
                        update_ltref_list();
                        //dump_dpb();
                        dpb.last_picture = MMP_NULL;
                        return;
                    }
                }
            }
        }
    }

    // this is a frame or a field which has no stored complementatry field
    // sliding window, if necessary
    if ((!p->idr_flag)&&(p->used_for_reference && (!p->adaptive_ref_pic_buffering_flag)))
    {
        sliding_window_memory_management(p);
    }

    // first try to remove unused frames
    if (dpb.used_size==dpb.size)
    {
        remove_unused_frame_from_dpb();
    }

    // then output frames until one can be removed
    while (dpb.used_size==dpb.size)
    {
        // non-reference frames may be output directly
        if (!p->used_for_reference)
        {
            get_smallest_poc(&poc, &pos);
            if ((-1==pos) || (p->poc < poc))
            {
                //direct_output(p, p_out);
                free_marking_buffer();
                free_storable_picture(p);
                return;
            }
        }

        // flush a frame
        output_one_frame_from_dpb();

        if (dpb.used_size==dpb.size)
        {
            get_smallest_poc_ex(&poc, &pos);
            if (pos==-1)
            {
                flush_dpb();
            } else {
                dpb.fs[pos]->is_output = 1;
                dpb.last_output_poc = poc;
                remove_frame_from_dpb(pos);
            }
        }
    }

    // check for duplicate frame number in short term reference buffer
    if ((p->used_for_reference)&&(!p->is_long_term))
    {
        for (i=0; i<dpb.ref_frames_in_buffer; i++)
        {
            if (dpb.fs_ref[i]->frame_num == p->frame_num)
            {
                printf("duplicate frame_num im short-term reference picture buffer %d %d %d\n", i, dpb.fs_ref[i]->frame_num, p->frame_num);
                if (dpb.used_size > 0 && dpb.fs[i]->is_used !=0)
                    remove_frame_from_dpb(i);
            }
        }
    }
    // store at end of buffer
    insert_picture_in_dpb(dpb.fs[dpb.used_size],p);

    if (p->structure != FRAME)
    {
        dpb.last_picture = dpb.fs[dpb.used_size];
    }
    else
    {
        dpb.last_picture = NULL;
    }

    dpb.used_size++;

    update_ref_list();
    update_ltref_list();

    for (i=0; i<dpb.used_size; i++)
    {
        if (dpb.fs[i] == NULL)
            break;

        if ((!(dpb.fs[i]->is_reference)) && ((DecodingQueue[dpb.fs[i]->decoding_buf_idx]&0xFFFF0000) == 0))
        {
            remove_frame_from_dpb(i);
        }
    }

    update_ref_list();
    update_ltref_list();
    //dump_dpb();
}

static void exit_picture(void)
{
    // return if the last picture has already been finished
    if (dec_picture==MMP_NULL)
    {
        return;
    }

    store_picture_in_dpb(dec_picture);

    free_marking_buffer();

    dec_picture=MMP_NULL;

    if (img->last_has_mmco_5)
    {
        img->pre_frame_num = 0;
    }
}

void free_dpb_frame_store()
{
    unsigned i;

    if (dpb.fs)
    {
        for (i=0; i<dpb.size; i++)
        {
            free_frame_store(dpb.fs[i]);
        }
    }
    dpb.last_output_poc = INT_MIN;
    dpb.init_done = 0;
}


void free_dpb()
{
    unsigned i;

    if (dpb.fs)
    {
        for (i=0; i<dpb.size; i++)
        {
            free_frame_store(dpb.fs[i]);
            free (dpb.fs[i]);
            dpb.fs[i] = NULL;
        }
        free (dpb.fs);
        dpb.fs=NULL;
    }
    if (dpb.fs_ref)
    {
        free (dpb.fs_ref);
        dpb.fs_ref=NULL;
    }

    if (dpb.fs_ltref)
    {
        free (dpb.fs_ltref);
        dpb.fs_ltref = NULL;
    }
    dpb.last_output_poc = INT_MIN;

    for (i=0; i<6; i++)
    {
        if (listX[i])
        {
            free (listX[i]);
            listX[i] = NULL;
        }
    }
    dpb.init_done = 0;
}

int getDpbSize()
{
    int pic_size = (active_sps->pic_width_in_mbs_minus1 + 1) * (active_sps->pic_height_in_map_units_minus1 + 1) * (active_sps->frame_mbs_only_flag?1:2) * 384;
    int size = 0;

    switch (active_sps->level_idc)
    {
    case 10:
        size = 152064;
        break;
    case 11:
        size = 345600;
        break;
    case 12:
        size = 912384;
        break;
    case 13:
        size = 912384;
        break;
    case 20:
        size = 912384;
        break;
    case 21:
        size = 1824768;
        break;
    case 22:
        size = 3110400;
        break;
    case 30:
        size = 3110400;
        break;
        //    case 31:
        //      size = 6912000;
        //      break;
        //    case 32:
        //      size = 7864320;
        //      break;
        //    case 40:
        //      size = 12582912;
        //      break;
        //    case 41:
        //      size = 12582912;
        //      break;
        //    case 42:
        //      size = 12582912;
        //      break;
        //    case 50:
        //      size = 42393600;
        //      break;
        //    case 51:
        //      size = 70778880;
        //      break;
    default:
        //error ("undefined level", 500);
        size = 3110400;
        break;
    }

    size /= pic_size;
    //return min( size, 16);           
    
    return gptAVCDecoder->frameBufCount;
}

void init_dpb(void)
{
    unsigned i,j;

    if (dpb.init_done)
    {
        free_dpb_frame_store();
    }

    dpb.size      = getDpbSize();
    dpb.num_ref_frames = active_sps->num_ref_frames;

    if (0==dpb.size)
    {
        printf("warning: DPB size of zero frames at specified level / frame size. Decoding may fail.\n");
    }

    dpb.used_size = 0;
    dpb.last_picture = NULL;

    dpb.ref_frames_in_buffer = 0;
    dpb.ltref_frames_in_buffer = 0;

    //dpb.fs = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
    //if (NULL==dpb.fs)
    //  dbg_msg_ex(DBG_MSG_TYPE_ERROR, "init_dpb: dpb->fs fail \n")
    //
    //dpb.fs_ref = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
    //if (NULL==dpb.fs_ref)
    //  dbg_msg_ex(DBG_MSG_TYPE_ERROR, "init_dpb: dpb->fs_ref fail \n");
    //
    //dpb.fs_ltref = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
    //if (NULL==dpb.fs_ltref)
    //  dbg_msg_ex(DBG_MSG_TYPE_ERROR, "init_dpb: dpb->fs_ltref");

    for (i=0; i<dpb.size; i++)
    {
        //dpb.fs[i]       = alloc_frame_store();
        reset_frame_store(dpb.fs[i]);
        dpb.fs_ref[i]   = NULL;
        dpb.fs_ltref[i] = NULL;
    }

    //for (i=0; i<6; i++)
    //{
    //    listX[i] = (StorablePicture **)calloc(MAX_LIST_SIZE, sizeof (StorablePicture*)); // +1 for reordering
    //    if (NULL==listX[i])
    //      dbg_msg_ex(DBG_MSG_TYPE_ERROR, "init_dpb: listX[%d] fail\n", i);
    //}

    for (j=0;j<6;j++)
    {
        for (i=0; i<MAX_LIST_SIZE; i++)
        {
            listX[j][i] = NULL;
        }
        listXsize[j]=0;
    }

    dpb.last_output_poc = INT_MIN;
    img->last_has_mmco_5 = 0;
    dpb.init_done = 1;
}

static void activate_sps(AVC_SEQUENCE_PARAMETER_SET *sps)
{
    if (active_sps != sps)
    {
        if (dec_picture)
        {
            // this may only happen on slice loss
            exit_picture();
        }
        active_sps = sps;
        //img->bitdepth_chroma = 0;
        //img->width_cr        = 0;
        //img->height_cr       = 0;
        // Fidelity Range Extensions stuff (part 1)
        //img->bitdepth_luma   = sps->bit_depth_luma_minus8 + 8;
        //if (sps->chroma_format_idc != YUV400)
        //  img->bitdepth_chroma = sps->bit_depth_chroma_minus8 + 8;
        img->MaxFrameNum = 1<<(sps->log2_max_frame_num_minus4+4);
        img->PicWidthInMbs = (sps->pic_width_in_mbs_minus1 +1);
        img->PicHeightInMapUnits = (sps->pic_height_in_map_units_minus1 +1);
        img->FrameHeightInMbs = ( 2 - sps->frame_mbs_only_flag ) * img->PicHeightInMapUnits;
        img->FrameSizeInMbs = img->PicWidthInMbs * img->FrameHeightInMbs;

        //img->yuv_format=sps->chroma_format_idc;
        //img->width = img->PicWidthInMbs * 16;
        //img->height = img->FrameHeightInMbs * 16;

        //if (sps->chroma_format_idc == YUV420)
        //{
        //    img->width_cr = img->width /2;
        //    img->height_cr = img->height / 2;
        //}
        //else if (sps->chroma_format_idc == YUV422)
        //{
        //    img->width_cr = img->width /2;
        //    img->height_cr = img->height;
        //}
        //else if (sps->chroma_format_idc == YUV444)
        //{
        //    //YUV444
        //    img->width_cr = img->width;
        //    img->height_cr = img->height;
        //}
        //init_frext(img);
        //init_global_buffers();
        if (!img->no_output_of_prior_pics_flag)
        {
            flush_dpb();
        }
        init_dpb();
        //if (NULL!=Co_located)
        //{
        //  free_collocated(Co_located);
        //}
        //Co_located = alloc_colocated (img->width, img->height,sps->mb_adaptive_frame_field_flag);
        //ercInit(img->width, img->height, 1);
    }
}

static void activate_pps(AVC_PICTURE_PARAMETER_SET *pps)
{
    if (active_pps != pps)
    {
        if (dec_picture)
        {
            // this may only happen on slice loss
            exit_picture();
        }

        active_pps = pps;

        // Fidelity Range Extensions stuff (part 2)
        img->AllowTransform8x8 = pps->transform_8x8_mode_flag;
    }
}

static void reset_wp_params(struct img_par *img)
{
int i,comp;
int log_weight_denom;

    for (i=0; i<MAX_REFERENCE_PICTURES; i++)
    {
        for (comp=0; comp<3; comp++)
        {
            log_weight_denom = (comp == 0) ? img->luma_log2_weight_denom : img->chroma_log2_weight_denom;
            img->wp_weight[0][i][comp] = 1<<log_weight_denom;
            img->wp_weight[1][i][comp] = 1<<log_weight_denom;
        }
    }
}

void alloc_ref_pic_list_reordering_buffer(Slice *currSlice)
{
int size = img->num_ref_idx_l0_active+1;

    if (img->type!=I_SLICE && img->type!=SI_SLICE)
    {
        if ((currSlice->remapping_of_pic_nums_idc_l0 = (int *)calloc(size,sizeof(int)))==NULL)
        {
            av_log(NULL, AV_LOG_ERROR, "Fail Allocate reordering_buffer\n");
        }
        if ((currSlice->abs_diff_pic_num_minus1_l0 = (int *)calloc(size,sizeof(int)))==NULL)
        {
            av_log(NULL, AV_LOG_ERROR, "Fail Allocate reordering_buffer\n");
        }
        if ((currSlice->long_term_pic_idx_l0 = (int *)calloc(size,sizeof(int)))==NULL)
        {
            av_log(NULL, AV_LOG_ERROR, "Fail Allocate reordering_buffer\n");
        }
    }
    else
    {
        currSlice->remapping_of_pic_nums_idc_l0 = NULL;
        currSlice->abs_diff_pic_num_minus1_l0 = NULL;
        currSlice->long_term_pic_idx_l0 = NULL;
    }

    size = img->num_ref_idx_l1_active+1;

    if (img->type==B_SLICE)
    {
        if ((currSlice->remapping_of_pic_nums_idc_l1 = (int *)calloc(size,sizeof(int)))==NULL)
        {
            av_log(NULL, AV_LOG_ERROR, "Fail Allocate reordering_buffer\n");
        }
        if ((currSlice->abs_diff_pic_num_minus1_l1 = (int *)calloc(size,sizeof(int)))==NULL)
        {
            av_log(NULL, AV_LOG_ERROR, "Fail Allocate reordering_buffer\n");
        }
        if ((currSlice->long_term_pic_idx_l1 = (int *)calloc(size,sizeof(int)))==NULL)
        {
            av_log(NULL, AV_LOG_ERROR, "Fail Allocate reordering_buffer\n");
        }
    }
    else
    {
        currSlice->remapping_of_pic_nums_idc_l1 = NULL;
        currSlice->abs_diff_pic_num_minus1_l1 = NULL;
        currSlice->long_term_pic_idx_l1 = NULL;
    }
}

static AVC_ERROR_CODE ref_pic_list_reordering_new()
{
AVC_ERROR_CODE ret = AVC_ERROR_SUCCESS;
Slice *currSlice = img->currentSlice;
MMP_UINT32 usedBits = 0;
int i, val;

    alloc_ref_pic_list_reordering_buffer(currSlice);

    if (img->type!=I_SLICE && img->type!=SI_SLICE)
    {
        val = currSlice->ref_pic_list_reordering_flag_l0 = BitStreamKit_GetBits(&gtAVCBitStream, 1);

        if (val)
        {
            i=0;
            do
            {
#if 1 // modified by wlHsu
                // assert (i>img->num_ref_idx_l0_active);
                if (i > img->num_ref_idx_l0_active)
                {
                    av_log(NULL, AV_LOG_ERROR, "Error 0 !! index = %d, num_ref_idx_l0_active = %d\n",
                        i, img->num_ref_idx_l0_active);
                    //break;
                    return ERROR_REF_PARAMETER;
                }

                val = currSlice->remapping_of_pic_nums_idc_l0[i] = ue_v(&usedBits);
                switch( val )
                {
                case 0:
                case 1:
                    currSlice->abs_diff_pic_num_minus1_l0[i] = ue_v(&usedBits);
                    break;
                case 2:
                    currSlice->long_term_pic_idx_l0[i] = ue_v(&usedBits);
                    break;

                default:
                    break;
                }

                i++;
#else
                val = currSlice->remapping_of_pic_nums_idc_l0[i] = ue_v(&usedBits);

                if (val==0 || val==1)
                {
                    currSlice->abs_diff_pic_num_minus1_l0[i] = ue_v(&usedBits);
                }
                else
                {
                    if (val==2)
                    {
                        currSlice->long_term_pic_idx_l0[i] = ue_v(&usedBits);
                    }
                }
                i++;
                // assert (i>img->num_ref_idx_l0_active);
                if (i>img->num_ref_idx_l0_active)
                {
                    // printf("Error0 %d %d\n", i, img->num_ref_idx_l0_active);
                    break;
                }
#endif
            } while (val != 3);
        }
    }

    if (img->type==B_SLICE)
    {
        val = currSlice->ref_pic_list_reordering_flag_l1 = BitStreamKit_GetBits(&gtAVCBitStream, 1);

        if (val)
        {
            i=0;
            do
            {
#if 1 // modified by wlHsu
                // assert (i>img->num_ref_idx_l1_active);
                if (i > img->num_ref_idx_l1_active)
                {
                    av_log(NULL, AV_LOG_ERROR, "Error 1 !! index = %d, num_ref_idx_l0_active = %d\n",
                        i, img->num_ref_idx_l1_active);
                    //break;
                    return ERROR_REF_PARAMETER;
                }

                val = currSlice->remapping_of_pic_nums_idc_l1[i] = ue_v(&usedBits);
                switch( val )
                {
                case 0:
                case 1:
                    currSlice->abs_diff_pic_num_minus1_l1[i] = ue_v(&usedBits);
                    break;
                case 2:
                    currSlice->long_term_pic_idx_l1[i] = ue_v(&usedBits);
                    break;

                default:
                    break;
                }

                i++;
#else
                val = currSlice->remapping_of_pic_nums_idc_l1[i] = ue_v(&usedBits);

                if (val==0 || val==1)
                {
                    currSlice->abs_diff_pic_num_minus1_l1[i] = ue_v(&usedBits);
                }
                else
                {
                    if (val==2)
                    {
                        currSlice->long_term_pic_idx_l1[i] = ue_v(&usedBits);
                    }
                }
                i++;
                // assert (i>img->num_ref_idx_l1_active);

                if (i>img->num_ref_idx_l1_active)
                {
                    // printf("Error1 %d %d\n", i, img->num_ref_idx_l1_active);
                    break;
                }
#endif
            } while (val != 3);
        }
    }
    return ret;
}

static void pred_weight_table_new()
{
Slice *currSlice = img->currentSlice;
int luma_weight_flag_l0, luma_weight_flag_l1, chroma_weight_flag_l0, chroma_weight_flag_l1;
int i,j;
MMP_UINT32 usedBits = 0;
MMP_UINT32 i1, i2;

    img->luma_log2_weight_denom = ue_v(&usedBits);
    //img->wp_round_luma = img->luma_log2_weight_denom ? 1<<(img->luma_log2_weight_denom - 1): 0;

    if ( 0 != active_sps->chroma_format_idc)
    {
        img->chroma_log2_weight_denom = ue_v(&usedBits);
        //img->wp_round_chroma = img->chroma_log2_weight_denom ? 1<<(img->chroma_log2_weight_denom - 1): 0;
    }
    reset_wp_params(img);

    for (i1 = 0; i1 < 2; i1++)
        for (i2 = 0; i2 < 32; i2++)
        {
            gSliceParameters.luma_weight[i1][i2] = 1<<img->luma_log2_weight_denom;
            gSliceParameters.luma_offset[i1][i2] = 0;
            gSliceParameters.chroma_weight_Cb[i1][i2] = 1<<img->chroma_log2_weight_denom;
            gSliceParameters.chroma_offset_Cb[i1][i2] = 0;
            gSliceParameters.chroma_weight_Cr[i1][i2] = 1<<img->chroma_log2_weight_denom;
            gSliceParameters.chroma_offset_Cr[i1][i2] = 0;
        }

    for (i=0; i<img->num_ref_idx_l0_active; i++)
    {
        luma_weight_flag_l0 = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        if (luma_weight_flag_l0)
        {
            img->wp_weight[0][i][0] = se_v(&usedBits);
            img->wp_offset[0][i][0] = se_v(&usedBits);
        }
        else
        {
            img->wp_weight[0][i][0] = 1<<img->luma_log2_weight_denom;
            img->wp_offset[0][i][0] = 0;
        }

        gSliceParameters.luma_weight[0][i] = img->wp_weight[0][i][0];
        gSliceParameters.luma_offset[0][i] = img->wp_offset[0][i][0];

        if (active_sps->chroma_format_idc != 0)
        {
            chroma_weight_flag_l0 = BitStreamKit_GetBits(&gtAVCBitStream, 1);

            for (j=1; j<3; j++)
            {
                if (chroma_weight_flag_l0)
                {
                    img->wp_weight[0][i][j] = se_v(&usedBits);
                    img->wp_offset[0][i][j] = se_v(&usedBits);
                }
                else
                {
                    img->wp_weight[0][i][j] = 1<<img->chroma_log2_weight_denom;
                    img->wp_offset[0][i][j] = 0;
                }

                if (j == 1)
                {
                    gSliceParameters.chroma_weight_Cb[0][i] = img->wp_weight[0][i][j];
                    gSliceParameters.chroma_offset_Cb[0][i] = img->wp_offset[0][i][j];
                }
                else
                {
                    gSliceParameters.chroma_weight_Cr[0][i] = img->wp_weight[0][i][j];
                    gSliceParameters.chroma_offset_Cr[0][i] = img->wp_offset[0][i][j];
                }
            }
        }
    }
    if ((img->type == B_SLICE) && active_pps->weighted_bipred_idc == 1)
    {
        for (i=0; i<img->num_ref_idx_l1_active; i++)
        {
            luma_weight_flag_l1 = BitStreamKit_GetBits(&gtAVCBitStream, 1);

            if (luma_weight_flag_l1)
            {
                img->wp_weight[1][i][0] = se_v(&usedBits);
                img->wp_offset[1][i][0] = se_v(&usedBits);
            }
            else
            {
                img->wp_weight[1][i][0] = 1<<img->luma_log2_weight_denom;
                img->wp_offset[1][i][0] = 0;
            }

            if (active_sps->chroma_format_idc != 0)
            {
                chroma_weight_flag_l1 = BitStreamKit_GetBits(&gtAVCBitStream, 1);

                for (j=1; j<3; j++)
                {
                    if (chroma_weight_flag_l1)
                    {
                        img->wp_weight[1][i][j] = se_v(&usedBits);
                        img->wp_offset[1][i][j] = se_v(&usedBits);
                    }
                    else
                    {
                        img->wp_weight[1][i][j] = 1<<img->chroma_log2_weight_denom;
                        img->wp_offset[1][i][j] = 0;
                    }

                    if (j == 1)
                    {
                        gSliceParameters.chroma_weight_Cb[1][i] = img->wp_weight[1][i][j];
                        gSliceParameters.chroma_offset_Cb[1][i] = img->wp_offset[1][i][j];
                    }
                    else
                    {
                        gSliceParameters.chroma_weight_Cr[1][i] = img->wp_weight[1][i][j];
                        gSliceParameters.chroma_offset_Cr[1][i] = img->wp_offset[1][i][j];
                    }
                }
            }
        }
    }
    gSliceParameters.luma_log2_weight_denom = img->luma_log2_weight_denom;
    gSliceParameters.chroma_log2_weight_denom = img->chroma_log2_weight_denom;
}

void dec_ref_pic_marking_new()
{
MMP_UINT32 usedBits = 0;
int val;
DecRefPicMarking_t *tmp_drpm,*tmp_drpm2;

    // free old buffer content
    while (img->dec_ref_pic_marking_buffer)
    {
        tmp_drpm=img->dec_ref_pic_marking_buffer;
        img->dec_ref_pic_marking_buffer=tmp_drpm->Next;
        free(tmp_drpm);
    }

    if (img->idr_flag)
    {
        img->no_output_of_prior_pics_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
        img->long_term_reference_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    }
    else
    {
        img->adaptive_ref_pic_buffering_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);

        if (img->adaptive_ref_pic_buffering_flag)
        {
            // read Memory Management Control Operation
            do
            {
                tmp_drpm=(DecRefPicMarking_t*)calloc (1,sizeof (DecRefPicMarking_t));

                if (tmp_drpm == MMP_NULL)
                {
                    av_log(NULL, AV_LOG_ERROR, "dec_ref_pic_marking buffer allocate fail\n");
                }

                tmp_drpm->Next=MMP_NULL;

                val = tmp_drpm->memory_management_control_operation = ue_v(&usedBits);

                if ((val==1)||(val==3))
                {
                    tmp_drpm->difference_of_pic_nums_minus1 = ue_v(&usedBits);
                }
                if (val==2)
                {
                    tmp_drpm->long_term_pic_num = ue_v(&usedBits);
                }

                if ((val==3)||(val==6))
                {
                    tmp_drpm->long_term_frame_idx = ue_v(&usedBits);
                }
                if (val==4)
                {
                    tmp_drpm->max_long_term_frame_idx_plus1 = ue_v(&usedBits);
                }

                // add command
                if (img->dec_ref_pic_marking_buffer==MMP_NULL)
                {
                    img->dec_ref_pic_marking_buffer=tmp_drpm;
                }
                else
                {
                    tmp_drpm2=img->dec_ref_pic_marking_buffer;
                    while (tmp_drpm2->Next!=MMP_NULL) tmp_drpm2=tmp_drpm2->Next;
                    tmp_drpm2->Next=tmp_drpm;
                }
            }while (val != 0);
        }
    }
}

void fill_wp_params(struct img_par *img)
{
    int i1, i2, maxEntry, thisPOC, topPOC, bottomPOC, framePOCList[16];
    int bframe = (img->type==B_SLICE);

    if (active_pps->weighted_bipred_idc == 2 && bframe)
    {
        img->luma_log2_weight_denom = 5;
        img->chroma_log2_weight_denom = 5;
        //img->wp_round_luma = 16;
        //img->wp_round_chroma = 16;

        gSliceParameters.luma_log2_weight_denom = 5;
        gSliceParameters.chroma_log2_weight_denom = 5;
    }

    // [SmallY20100302] new implicit weighting method to support support num_ref_frames = 16
    if (img->field_pic_flag || !img->MbaffFrameFlag) // field pic or frame mb only pic
    {
        for (i1 = 0; i1 < img->num_ref_idx_l0_active; i1++)
        {
            gRefIdxToPocListIdx[0][i1] = i1;
            if (listX[LIST_0][i1]->is_long_term)
                gPocList[i1] = 0x08000;
            else
                gPocList[i1] = listX[LIST_0][i1]->poc & 0x0FFFF;
        }
        maxEntry = img->num_ref_idx_l0_active;
        for (i1 = 0; i1 < img->num_ref_idx_l1_active; i1++)
        {
            if (listX[LIST_1][i1]->is_long_term)
                thisPOC = 0x08000;
            else
                thisPOC = listX[LIST_1][i1]->poc & 0x0FFFF;
            for (i2 = 0; i2 < maxEntry; i2++)
            {
                if (gPocList[i2] == thisPOC)
                {
                    gRefIdxToPocListIdx[1][i1] = i2;
                    break;
                }
            }
            if (i2 == maxEntry) // ref pic with POC not in list 0
            {
                gPocList[maxEntry] = thisPOC;
                gRefIdxToPocListIdx[1][i1] = maxEntry;
                maxEntry++;
            }
        }
    } else { // MBAFF
        // fill the frame poc list first
        for (i1 = 0; i1 < img->num_ref_idx_l0_active; i1++)
        {
            framePOCList[i1] = listX[LIST_0][i1]->poc & 0x0FFFF;
        }
        maxEntry = img->num_ref_idx_l0_active;
        for (i1 = 0; i1 < img->num_ref_idx_l1_active; i1++)
        {
            thisPOC = listX[LIST_1][i1]->poc & 0x0FFFF;
            for (i2 = 0; i2 < maxEntry; i2++)
            {
                if (framePOCList[i2] == thisPOC)
                {
                    break;
                }
            }
            if (i2 == maxEntry) // ref pic with POC not in list 0
            {
                framePOCList[maxEntry] = thisPOC;
                maxEntry++;
            }
        }
        // place the frame/top/bottom field poc
        for (i1 = 0; i1 < img->num_ref_idx_l0_active; i1++)
        {
            thisPOC = listX[LIST_0][i1]->poc & 0x0FFFF;
            topPOC = listX[2 + LIST_0][i1 * 2]->poc & 0x0FFFF;
            bottomPOC = listX[2 + LIST_0][i1 * 2 + 1]->poc & 0x0FFFF;
            for (i2 = 0; i2 < maxEntry; i2++)
            {
                if (thisPOC == framePOCList[i2]) // shall be found
                {
                    gPocList[i2 * 2] = topPOC;
                    gPocList[i2 * 2 + 1] = bottomPOC;
                    if (thisPOC == topPOC)
                        gRefIdxToPocListIdx[0][i1] = i2 * 2;
                    else
                        gRefIdxToPocListIdx[0][i1] = i2 * 2 + 1;
                }
            }
        }
        for (i1 = 0; i1 < img->num_ref_idx_l1_active; i1++)
        {
            thisPOC = listX[LIST_1][i1]->poc & 0x0FFFF;
            topPOC = listX[2 + LIST_1][i1 * 2]->poc & 0x0FFFF;
            bottomPOC = listX[2 + LIST_1][i1 * 2 + 1]->poc & 0x0FFFF;
            for (i2 = 0; i2 < maxEntry; i2++)
            {
                if (thisPOC == framePOCList[i2]) // shall be found
                {
                    gPocList[i2 * 2] = topPOC;
                    gPocList[i2 * 2 + 1] = bottomPOC;
                    if (thisPOC == topPOC)
                        gRefIdxToPocListIdx[1][i1] = i2 * 2;
                    else
                        gRefIdxToPocListIdx[1][i1] = i2 * 2 + 1;
                }
            }
        }
    }
}

AVC_ERROR_CODE compute_collocated()
{
    AVC_ERROR_CODE ret = AVC_ERROR_SUCCESS;
    StorablePicture *fs, *fs_top, *fs_bottom;
    int i,j;
    int h, v, i1, i2, chunkNum, data;
    FrameStore *refFrame;
    int prescale, iTRb, iTRp;
    MMP_BOOL bFind = MMP_FALSE;

    fs_top=fs_bottom=fs = listX[LIST_1 ][0];

    // temp solution : TO DO
    if (fs == 0)
    {
        printf("\nlistX[LIST_1][0] == NULL !!");
        return ERROR_BITSTREAM_ERROR;
    }

    if (img->MbaffFrameFlag)
    {
        fs_top= listX[LIST_1 + 2][0];
        fs_bottom= listX[LIST_1 + 4][0];
    }
    else
    {
        if (img->field_pic_flag)
        {
            if ((img->structure != fs->structure) && (fs->coded_frame))
//            if ((img->structure != fs->structure) && ((fs->MbaffFrameFlag) || ((!fs->MbaffFrameFlag)&&(fs->coded_frame))))
            {

                if (img->structure==TOP_FIELD)
                {
                    fs_top=fs_bottom=fs = listX[LIST_1 ][0]->top_field;
                }
                else
                {
                    fs_top=fs_bottom=fs = listX[LIST_1 ][0]->bottom_field;
                }
            }
        }
    }

    if (fs->coded_frame)
        gSliceParameters.col_data_placement = 0; // one chunk of colocated data
    else
        gSliceParameters.col_data_placement = 1; // two chunk of colocated data

    gSliceParameters.col_is_longterm = fs->is_long_term;
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 32; j++) {
            gSliceParameters.col_pic_idx_to_ref_idx_mapping[i][j] = 0;
        }
        for (j = 0; j < 12; j++) {
            gSliceParameters.col_ref_idx_to_dist_scale_mapping[i][j] = 0;
        }
    }
    for (i = 0; i < listXsize[0]; i++)
    {
        gSliceParameters.col_pic_idx_to_ref_idx_mapping[0][listX[0][i]->pic_idx] = i;
    }
    if (img->MbaffFrameFlag)
    {
        for (i = 0; i < listXsize[2]; i++)
        {
            gSliceParameters.col_pic_idx_to_ref_idx_mapping[1][listX[2][i]->pic_idx] = i;
        }
        for (i = 0; i < listXsize[4]; i++)
        {
            gSliceParameters.col_pic_idx_to_ref_idx_mapping[2][listX[4][i]->pic_idx] = i;
        }
    }

    // find the frame with the same poc as RefList1[0] in the dpb buffer
    for (i1 = 0; i1 < dpb.ref_frames_in_buffer; i1++)
    {
        if (dpb.fs_ref[i1]->frame == NULL)
            continue;

        if (fs->structure == TOP_FIELD) // must have found
        {
            if (dpb.fs_ref[i1]->frame->top_poc == fs->poc)
            {
                bFind = MMP_TRUE;
                break;
            }
        }
        else if (fs->structure == BOTTOM_FIELD)
        {
            if (dpb.fs_ref[i1]->frame->bottom_poc == fs->poc) // must have found
            {
                bFind = MMP_TRUE;
                break;
            }
        }
        else
        {
            if (dpb.fs_ref[i1]->frame->frame_poc == fs->poc) // must have found
            {
                bFind = MMP_TRUE;
                break;
            }
        }
    }

    if (!bFind)
        return ERROR_BITSTREAM_ERROR;

    // [08/30] dkwang temp solution, i don't know why??
    //if (i1 == dpb.ref_frames_in_buffer)
    //    i1 = 0;

    refFrame = dpb.fs_ref[i1];
    if (refFrame->frame->coded_frame == 0) // complementary field pair, col_data_placement = 1
    {
        chunkNum = 2;
        if (img->structure == TOP_FIELD || img->structure == BOTTOM_FIELD) // currect pic is field
        {
            if (fs->structure == 1)
                gSliceParameters.col_data_use_which_chunk = 0; // top_field
            else
                gSliceParameters.col_data_use_which_chunk = 1; // bottom_field
        } else { // frame
            if (img->MbaffFrameFlag) // MBAFF
            {
                // decide to use which field as the colocated pic when a MB pair is coded in frame mode
                if (abs(dec_picture->poc - fs_bottom->poc) > abs(dec_picture->poc -fs_top->poc))
                    gSliceParameters.col_data_use_which_chunk = 0; // top_field
                else
                    gSliceParameters.col_data_use_which_chunk = 1; // bottom_field
            } else { // frame_mb_only
                // not used
                gSliceParameters.col_data_use_which_chunk = 0;
            }
        }
    } else {
        chunkNum = 1;
        // [SmallY20100121] update
        // In MBAFF mode, when curr is a frame MB pair and col is a field MB pair, use this flag to decide
        //   which MB is the col MB.
        if (img->MbaffFrameFlag) { // MBAFF
            if (abs(dec_picture->poc - fs_bottom->poc) > abs(dec_picture->poc -fs_top->poc))
                gSliceParameters.col_data_use_which_chunk = 0; // top_field
            else
                gSliceParameters.col_data_use_which_chunk = 1; // bottom_field
        }
    }

    //gSliceParameters.col_data_bottom_offset = img->width * img->height / 4 / 4 / 2 * 8 / 2;
    if (img->direct_spatial_mv_pred_flag ==0)
    {
        for (j=0; j<2 + (img->MbaffFrameFlag * 4);j+=2)
        {
            for (i=0; i<listXsize[j];i++)
            {
                if (j==0)
                {
                    iTRb = Clip3( -128, 127, dec_picture->poc - listX[LIST_0 + j][i]->poc );
                }
                else if (j == 2)
                {
                    iTRb = Clip3( -128, 127, dec_picture->top_poc - listX[LIST_0 + j][i]->poc );
                }
                else
                {
                    iTRb = Clip3( -128, 127, dec_picture->bottom_poc - listX[LIST_0 + j][i]->poc );
                }

                iTRp = Clip3( -128, 127,  listX[LIST_1 + j][0]->poc - listX[LIST_0 + j][i]->poc);

                if (iTRp!=0)
                {
                    prescale = ( 16384 + abs( iTRp / 2 ) ) / iTRp;
                    img->mvscale[j][i] = Clip3( -1024, 1023, ( iTRb * prescale + 32 ) >> 6 ) ;
                }
                else
                {
                    img->mvscale[j][i] = 9999;
                }
            }
        }
        // [SmallY20090102]
        for (i = 0; i < listXsize[0]; i++) {
            if (img->mvscale[0][i] == 9999 || img->mvscale[0][i] == 0)
                gSliceParameters.col_ref_idx_to_dist_scale_mapping[0][i] = 256;
            else
                gSliceParameters.col_ref_idx_to_dist_scale_mapping[0][i] = img->mvscale[0][i];
        }
        if (img->MbaffFrameFlag)
        {
            for (i = 0; i < listXsize[2]; i++) {
                if (img->mvscale[2][i] == 9999 || img->mvscale[2][i] == 0)
                    gSliceParameters.col_ref_idx_to_dist_scale_mapping[1][i] = 256;
                else
                    gSliceParameters.col_ref_idx_to_dist_scale_mapping[1][i] = img->mvscale[2][i];
            }
            for (i = 0; i < listXsize[4]; i++) {
                if (img->mvscale[4][i] == 9999 || img->mvscale[4][i] == 0)
                    gSliceParameters.col_ref_idx_to_dist_scale_mapping[2][i] = 256;
                else
                    gSliceParameters.col_ref_idx_to_dist_scale_mapping[2][i] = img->mvscale[4][i];
            }
        }
    }
    return ret;
}

static AVC_ERROR_CODE
read_new_slice(MMP_UINT32 RBSPsize)
{
AVC_ERROR_CODE ret = AVC_ERROR_SUCCESS;
MMP_UINT32 usedBits = 0;
AVC_SEQUENCE_PARAMETER_SET *pSeqParamSet;
AVC_PICTURE_PARAMETER_SET *pPicParamSet;
MMP_UINT32 tmp;
MMP_UINT32 cur_structure;
MMP_INT32 val;
MMP_UINT32 i1, i2;
MMP_UINT32 totalMBs;
MMP_UINT32 tmpMBAX, last_MB_in_slice_Y;
Slice *currSlice = img->currentSlice;

    gtAVCBitStream.remainByte = RBSPsize;
    BitStreamKit_Init(&gtAVCBitStream, gpRBSP);
    currSlice->start_mb_nr = ue_v(&usedBits);

    tmp = ue_v(&usedBits);

    if (tmp>4) tmp -=5;
    img->type = currSlice->picture_type = (SliceType) tmp;

    if (img->type > 4)
    {
        av_log(NULL, AV_LOG_ERROR, "Error Picture Type\n");
        return ERROR_PICTURE_TYPE;
    }

    currSlice->pic_parameter_set_id = ue_v(&usedBits);
    ret = avc_GetParameterSet(&pSeqParamSet, &pPicParamSet, currSlice->pic_parameter_set_id);

    if (ret!=AVC_ERROR_SUCCESS)
    {
        av_log(NULL, AV_LOG_ERROR, "Error set parameter set\n");
        return ret;
    }

    activate_sps(pSeqParamSet);
    activate_pps(pPicParamSet);

    // transfer some parameters
    // aspect ratio
    // width, height
    //gptAVCDecoder->frameWidth  = gptAVCVideoDecoder->horizontal_size = (img->PicWidthInMbs << 4);
    //gptAVCDecoder->frameHeight = gptAVCVideoDecoder->vertical_size   = (img->FrameHeightInMbs << 4);

    gptAVCDecoder->frameWidth  = (img->PicWidthInMbs << 4);
    gptAVCDecoder->frameHeight = (img->FrameHeightInMbs << 4);

    if (active_sps->aspect_ratio_idc == 1)
        gptAVCDecoder->aspect_ratio_information = 1; // 1:1 (square)
    else if ( (active_sps->aspect_ratio_idc == 2) ||
         (active_sps->aspect_ratio_idc == 3) ||
         (active_sps->aspect_ratio_idc == 4 && gptAVCDecoder->frameWidth == 528 && gptAVCDecoder->frameHeight == 576) ||
         (active_sps->aspect_ratio_idc == 5 && gptAVCDecoder->frameWidth == 528 && gptAVCDecoder->frameHeight == 480) ||
         (active_sps->aspect_ratio_idc == 6 && gptAVCDecoder->frameWidth == 352 && gptAVCDecoder->frameHeight == 576) ||
         (active_sps->aspect_ratio_idc == 7 && gptAVCDecoder->frameWidth == 352 && gptAVCDecoder->frameHeight == 480) ||
         (active_sps->aspect_ratio_idc == 10) ||
         (active_sps->aspect_ratio_idc == 11) )
        gptAVCDecoder->aspect_ratio_information = 2; // 4:3
    else if ( (active_sps->aspect_ratio_idc == 4 && gptAVCDecoder->frameWidth == 720 && gptAVCDecoder->frameHeight == 576) ||
         (active_sps->aspect_ratio_idc == 5 && gptAVCDecoder->frameWidth == 720 && gptAVCDecoder->frameHeight == 480) ||
         (active_sps->aspect_ratio_idc == 6 && gptAVCDecoder->frameWidth == 480 && gptAVCDecoder->frameHeight == 576) ||
         (active_sps->aspect_ratio_idc == 7 && gptAVCDecoder->frameWidth == 480 && gptAVCDecoder->frameHeight == 480) ||
         (active_sps->aspect_ratio_idc == 8) ||
         (active_sps->aspect_ratio_idc == 9) ||
         (active_sps->aspect_ratio_idc == 12) ||
         (active_sps->aspect_ratio_idc == 13) ||
         (active_sps->aspect_ratio_idc == 14) ||
         (active_sps->aspect_ratio_idc == 15) ||
         (active_sps->aspect_ratio_idc == 16) )
        gptAVCDecoder->aspect_ratio_information = 3; // 16:9
    else
        gptAVCDecoder->aspect_ratio_information = 2; // 4:3

    // frame rate
    if (active_sps->num_units_in_tick)
    {
#if 1 // modified by wlHsu
        switch ( active_sps->time_scale / active_sps->num_units_in_tick )
        {
        case 29: //14.985 fps
            gptAVCVideoDecoder->frame_rate_code = 9;
            break;
        case 30: //15 fps
            gptAVCVideoDecoder->frame_rate_code = 10;
            break;
        case 47: //23.976 fps
            gptAVCVideoDecoder->frame_rate_code = 1;
            break;
        case 48: //24 fps
            gptAVCVideoDecoder->frame_rate_code = 2;
            break;
        case 50: //25 fps
            gptAVCVideoDecoder->frame_rate_code = 3;
            break;
        case 59: //29.970 fps
            gptAVCVideoDecoder->frame_rate_code = 4;
            break;
        case 60: //30 fps
            gptAVCVideoDecoder->frame_rate_code = 5;
            break;
        case 100: //50 fps
            gptAVCVideoDecoder->frame_rate_code = 6;
            break;
        case 120: //60 fps
            gptAVCVideoDecoder->frame_rate_code = 8;
            break;
        default:
            gptAVCVideoDecoder->frame_rate_code = 3;
            break;
        }
#else
        if ((active_sps->time_scale / active_sps->num_units_in_tick) == 47)
            gptAVCVideoDecoder->frame_rate_code = 1;
        else if ((active_sps->time_scale / active_sps->num_units_in_tick) == 48)
            gptAVCVideoDecoder->frame_rate_code = 2;
        else if ((active_sps->time_scale / active_sps->num_units_in_tick) == 50)
            gptAVCVideoDecoder->frame_rate_code = 3;
        else if ((active_sps->time_scale / active_sps->num_units_in_tick) == 59)
            gptAVCVideoDecoder->frame_rate_code = 4;
        else if ((active_sps->time_scale / active_sps->num_units_in_tick) == 60)
            gptAVCVideoDecoder->frame_rate_code = 5;
        else if ((active_sps->time_scale / active_sps->num_units_in_tick) == 100)
            gptAVCVideoDecoder->frame_rate_code = 6;
        else if ((active_sps->time_scale / active_sps->num_units_in_tick) == 120)
            gptAVCVideoDecoder->frame_rate_code = 8;
        else
            gptAVCVideoDecoder->frame_rate_code = 3;
#endif
    } else {
        gptAVCVideoDecoder->frame_rate_code = 3;
    }

    if (img->PicWidthInMbs > MAX_FRAME_WIDTH_IN_MBs ||
        img->FrameHeightInMbs > MAX_FRAME_HEIGHTIN_MBs ||
        img->PicWidthInMbs < MIN_FRAME_WIDTH_IN_MBs ||
        img->FrameHeightInMbs < MIN_FRAME_HEIGHTIN_MBs)
    {
        av_log(NULL, AV_LOG_ERROR, "ERROR_RESOLUTION !!");
        return ERROR_RESOLUTION;
    }

    img->frame_num = BitStreamKit_GetBits(&gtAVCBitStream, (pSeqParamSet->log2_max_frame_num_minus4 + 4));

    if (img->frame_num >= img->MaxFrameNum)
        return ERROR_BITSTREAM_ERROR;

    if (MMP_FALSE == gbFrameStart &&  img->frame_num != img->pre_frame_num)
        return ERROR_BITSTREAM_ERROR;

    //printf("   %d\n", pSeqParamSet->log2_max_frame_num_minus4 + 4);
    if (img->idr_flag)
    {
        img->pre_frame_num = img->frame_num;
    //  assert(img->frame_num == 0);
    }

    if (active_sps->frame_mbs_only_flag)
    {
        img->structure = FRAME;
        img->field_pic_flag=0;
        gbLastPic = MMP_TRUE;
    }
    else
    {
        // field_pic_flag   u(1)
        img->field_pic_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);

        if (img->field_pic_flag)
        {
            // bottom_field_flag  u(1)
            img->bottom_field_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);

            cur_structure = img->bottom_field_flag ? BOTTOM_FIELD : TOP_FIELD;

            if (MMP_TRUE == gbFrameStart && img->structure == cur_structure)
            {
                //printf("Structure Error %d %d\n", img->structure, cur_structure);
                return ERROR_BITSTREAM_ERROR;
            }
            img->structure = cur_structure;

            if (cur_structure == BOTTOM_FIELD)
                gbLastPic = MMP_TRUE;
            else
                gbLastPic = MMP_FALSE;
        }
        else
        {
            img->structure = FRAME;
            img->bottom_field_flag=0;
            gbLastPic = MMP_TRUE;
        }
    }

    img->MbaffFrameFlag = (active_sps->mb_adaptive_frame_field_flag && (img->field_pic_flag==0));

    if (img->MbaffFrameFlag == 1)
        currSlice->start_mb_nr = currSlice->start_mb_nr * 2;

    if (MMP_FALSE == gbFrameStart)
    {
        if ((currSlice->structure != img->structure) || (img->preFrameSizeInMbs != img->FrameSizeInMbs))
            return ERROR_BITSTREAM_ERROR;
    }

    if (img->structure == TOP_FIELD || img->structure == BOTTOM_FIELD)
        totalMBs = img->FrameSizeInMbs >> 1;
    else
        totalMBs = img->FrameSizeInMbs;

    //if ((currSlice->start_mb_nr >= totalMBs) || ((totalMBs - currSlice->start_mb_nr) < 5))
    //{
    //    return ERROR_BITSTREAM_ERROR;
    //}

    img->preFrameSizeInMbs = img->FrameSizeInMbs;

    currSlice->structure = img->structure;

    // Error handle for MBAFF
    if (MMP_FALSE == gbFrameStart && img->MbaffFrameFlag == 1)
    {
        tmpMBAX = currSlice->start_mb_nr % img->PicWidthInMbs;

        if (tmpMBAX == 0)
        {
            last_MB_in_slice_Y = (tmpMBAX / img->PicWidthInMbs) - 1;
        }
        else
        {
            last_MB_in_slice_Y = (tmpMBAX / img->PicWidthInMbs);
        }

        if ((last_MB_in_slice_Y & 0x1) == 0)
            return ERROR_BITSTREAM_ERROR;
    }

    if (img->idr_flag)
    {
        img->idr_pic_id = ue_v(&usedBits);
    }

    if (active_sps->pic_order_cnt_type == 0)
    {
        img->pic_order_cnt_lsb = BitStreamKit_GetBits(&gtAVCBitStream, (active_sps->log2_max_pic_order_cnt_lsb_minus4 + 4));

        if (active_pps->pic_order_present_flag  ==  1 &&  !img->field_pic_flag)
        {
            img->delta_pic_order_cnt_bottom = se_v(&usedBits);
        }
        else
            img->delta_pic_order_cnt_bottom = 0;
    }
    if (active_sps->pic_order_cnt_type == 1 && !active_sps->delta_pic_order_always_zero_flag)
    {
        img->delta_pic_order_cnt[ 0 ] = se_v(&usedBits);

        if (active_pps->pic_order_present_flag  ==  1  &&  !img->field_pic_flag)
        {
            img->delta_pic_order_cnt[ 1 ] = se_v(&usedBits);
        }
    }
    else
    {
        if (active_sps->pic_order_cnt_type == 1)
        {
            img->delta_pic_order_cnt[ 0 ] = 0;
            img->delta_pic_order_cnt[ 1 ] = 0;
        }
    }

    //! redundant_pic_cnt is missing here
    if (active_pps->redundant_pic_cnt_present_flag)
    {
        img->redundant_pic_cnt = ue_v(&usedBits);
    }

    if (img->type==B_SLICE)
    {
        img->direct_spatial_mv_pred_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
    }

    img->num_ref_idx_l0_active = active_pps->num_ref_idx_l0_active_minus1 + 1;
    img->num_ref_idx_l1_active = active_pps->num_ref_idx_l1_active_minus1 + 1;

    if (img->type==P_SLICE || img->type == SP_SLICE || img->type==B_SLICE)
    {
        val = BitStreamKit_GetBits(&gtAVCBitStream, 1);

        if (val)
        {
            img->num_ref_idx_l0_active = 1 + ue_v(&usedBits);

            if (img->type==B_SLICE)
            {
                img->num_ref_idx_l1_active = 1 + ue_v(&usedBits);
            }
        }
    }
    if (img->type!=B_SLICE)
    {
        img->num_ref_idx_l1_active = 0;
    }

    if (img->num_ref_idx_l0_active > 16)
        img->num_ref_idx_l0_active = 16;

    if (img->num_ref_idx_l1_active > 16)
        img->num_ref_idx_l1_active = 16;

    //ref_pic_list_reordering_new();
    if ((ret = ref_pic_list_reordering_new()) != AVC_ERROR_SUCCESS)
        return ret;

    gSliceParameters.luma_log2_weight_denom = gSliceParameters.chroma_log2_weight_denom = 0;
    for (i1 = 0; i1 < 2; i1++)
        for (i2 = 0; i2 < 32; i2++)
        {
            gSliceParameters.luma_weight[i1][i2] = 1;
            gSliceParameters.luma_offset[i1][i2] = 0;
            gSliceParameters.chroma_weight_Cb[i1][i2] = 1;
            gSliceParameters.chroma_offset_Cb[i1][i2] = 0;
            gSliceParameters.chroma_weight_Cr[i1][i2] = 1;
            gSliceParameters.chroma_offset_Cr[i1][i2] = 0;
        }

    if ((active_pps->weighted_pred_flag&&(img->type==P_SLICE|| img->type == SP_SLICE))||
        (active_pps->weighted_bipred_idc==1 && (img->type==B_SLICE)))
    {
        pred_weight_table_new();
    }

    for (i1 = 0; i1 < 2; i1++)
    {
        for (i2 = 0; i2 < 32; i2++)
        {
            weighted_pred_RF[i1 * 96 + i2][0] = gSliceParameters.luma_weight[i1][i2] & 0x0FF;
            weighted_pred_RF[i1 * 96 + i2][1] = (gSliceParameters.luma_weight[i1][i2] >> 8) & 1;
            weighted_pred_RF[i1 * 96 + i2][2] = gSliceParameters.luma_offset[i1][i2] & 0x0FF;
            weighted_pred_RF[i1 * 96 + i2 + 32][0] = gSliceParameters.chroma_weight_Cb[i1][i2] & 0x0FF;
            weighted_pred_RF[i1 * 96 + i2 + 32][1] = (gSliceParameters.chroma_weight_Cb[i1][i2] >> 8) & 1;
            weighted_pred_RF[i1 * 96 + i2 + 32][2] = gSliceParameters.chroma_offset_Cb[i1][i2] & 0x0FF;
            weighted_pred_RF[i1 * 96 + i2 + 64][0] = gSliceParameters.chroma_weight_Cr[i1][i2] & 0x0FF;
            weighted_pred_RF[i1 * 96 + i2 + 64][1] = (gSliceParameters.chroma_weight_Cr[i1][i2] >> 8) & 1;
            weighted_pred_RF[i1 * 96 + i2 + 64][2] = gSliceParameters.chroma_offset_Cr[i1][i2] & 0x0FF;
        }
    }

    // mark for no B frame & recorder & marking
    if (img->nal_reference_idc)
        dec_ref_pic_marking_new();

    if (active_pps->entropy_coding_mode_flag && img->type!=I_SLICE && img->type!=SI_SLICE)
    {
        img->model_number = ue_v(&usedBits);
    }
    else
    {
        img->model_number = 0;
    }
    gSliceParameters.pDataStart  = gtAVCBitStream.pStartAddress;
    gSliceParameters.dwStartBits = gtAVCBitStream.bitPosition;

    currSlice->slice_qp_delta = se_v(&usedBits);
    currSlice->qp = img->qp = 26 + active_pps->pic_init_qp_minus26 + currSlice->slice_qp_delta;

    //printf("Flg %d SEIP %d str %d t %d %d f %d\n", gbSEIPicStrFlg, gSEIPicStructure, img->structure, active_sps->time_scale, active_sps->num_units_in_tick, gptAVCVideoDecoder->frame_rate_code);
    // Update for display structure
    if (gbSEIPicStrFlg)
    {
        img->picture_structure = gSEIPicStructure;
    } else {
        img->picture_structure = img->structure;
    }
    gbSEIPicStrFlg = MMP_FALSE;

    if (gtAVCBitStream.remainByte < 0)
        return ERROR_BITSTREAM_ERROR;
    //printf("Slice T %d  S %d R %d POC %d F_N %d\n", img->type, img->structure, img->nal_reference_idc, img->framepoc, img->frame_num);

    return ret;
}

static void decode_poc(void)
{
    int i;
    // for POC mode 0:
    unsigned int MaxPicOrderCntLsb = (1<<(active_sps->log2_max_pic_order_cnt_lsb_minus4+4));

    switch ( active_sps->pic_order_cnt_type )
    {
    case 0: // POC MODE 0
        // 1st
        if (img->idr_flag)
        {
            img->PrevPicOrderCntMsb = 0;
            img->PrevPicOrderCntLsb = 0;
        }
        else
        {
            if (img->last_has_mmco_5)
            {
                if (img->last_pic_bottom_field)
                {
                    img->PrevPicOrderCntMsb = 0;
                    img->PrevPicOrderCntLsb = 0;
                }
                else
                {
                    img->PrevPicOrderCntMsb = 0;
                    img->PrevPicOrderCntLsb = img->toppoc;
                }
            }
        }
        // Calculate the MSBs of current picture
        if( img->pic_order_cnt_lsb  <  img->PrevPicOrderCntLsb  &&
           (img->PrevPicOrderCntLsb - img->pic_order_cnt_lsb) >= (MaxPicOrderCntLsb >> 1) )
            img->PicOrderCntMsb = img->PrevPicOrderCntMsb + MaxPicOrderCntLsb;
        else if ( img->pic_order_cnt_lsb  >  img->PrevPicOrderCntLsb  &&
                 (img->pic_order_cnt_lsb - img->PrevPicOrderCntLsb) > (MaxPicOrderCntLsb >> 1) )
            img->PicOrderCntMsb = img->PrevPicOrderCntMsb - MaxPicOrderCntLsb;
        else
            img->PicOrderCntMsb = img->PrevPicOrderCntMsb;

        // 2nd
        if ((!img->field_pic_flag)||(!img->bottom_field_flag ))
        {
            img->ThisPOC= img->toppoc = img->PicOrderCntMsb + img->pic_order_cnt_lsb;
        }

        if (!img->field_pic_flag)
        {
            img->bottompoc = img->toppoc + img->delta_pic_order_cnt_bottom;
        }
        else
        {
            if ( img->bottom_field_flag )
            {
                img->ThisPOC= img->bottompoc = img->PicOrderCntMsb + img->pic_order_cnt_lsb;
            }
        }

        gSliceParameters.poc = img->ThisPOC;
        gSliceParameters.top_poc = img->toppoc;
        gSliceParameters.bottom_poc = img->bottompoc;

        // last: some post-processing.

        // if (img->newframe == 1)
        {
            if (!img->bottom_field_flag)
            {
                img->framepoc = img->toppoc;
            }
            else
            {
                img->framepoc = img->bottompoc;
            }
        }

        if ( img->frame_num!=img->PreviousFrameNum)
            img->PreviousFrameNum=img->frame_num;

        if(!img->disposable_flag)
        {
            img->PrevPicOrderCntLsb = img->pic_order_cnt_lsb;
            img->PrevPicOrderCntMsb = img->PicOrderCntMsb;
        }
        break;

    case 1: // POC MODE 1
        // 1st
        if (img->idr_flag)
        {
            img->FrameNumOffset=0;     //  first pix of IDRGOP,
            img->delta_pic_order_cnt[0]=0;                        //ignore first delta
            //if(img->frame_num)  error("frame_num != 0 in idr pix", -1020);
        }
        else
        {
            if (img->last_has_mmco_5)
            {
                img->PreviousFrameNumOffset = 0;
                img->PreviousFrameNum = 0;
            }
            if (img->frame_num<img->PreviousFrameNum)
            {             //not first pix of IDRGOP
                img->FrameNumOffset = img->PreviousFrameNumOffset + img->MaxFrameNum;
            }
            else
            {
                img->FrameNumOffset = img->PreviousFrameNumOffset;
            }
        }

        // 2nd
        if (active_sps->num_ref_frames_in_pic_order_cnt_cycle)
            img->AbsFrameNum = img->FrameNumOffset+img->frame_num;
        else
            img->AbsFrameNum=0;
        if (img->disposable_flag && img->AbsFrameNum>0)
            img->AbsFrameNum--;

        // 3rd
        img->ExpectedDeltaPerPicOrderCntCycle=0;

        if (active_sps->num_ref_frames_in_pic_order_cnt_cycle)
            for (i=0;i<(int) active_sps->num_ref_frames_in_pic_order_cnt_cycle;i++)
                img->ExpectedDeltaPerPicOrderCntCycle += active_sps->offset_for_ref_frame[i];

        if (img->AbsFrameNum)
        {
            img->PicOrderCntCycleCnt = (img->AbsFrameNum-1)/active_sps->num_ref_frames_in_pic_order_cnt_cycle;
            img->FrameNumInPicOrderCntCycle = (img->AbsFrameNum-1)%active_sps->num_ref_frames_in_pic_order_cnt_cycle;
            img->ExpectedPicOrderCnt = img->PicOrderCntCycleCnt*img->ExpectedDeltaPerPicOrderCntCycle;
            for (i=0;i<=(int)img->FrameNumInPicOrderCntCycle;i++)
                img->ExpectedPicOrderCnt += active_sps->offset_for_ref_frame[i];
        }
        else
            img->ExpectedPicOrderCnt=0;

        if (img->disposable_flag)
            img->ExpectedPicOrderCnt += active_sps->offset_for_non_ref_pic;

        if (img->field_pic_flag==0)
        {             //frame pix
            img->toppoc = img->ExpectedPicOrderCnt + img->delta_pic_order_cnt[0];
            img->bottompoc = img->toppoc + active_sps->offset_for_top_to_bottom_field + img->delta_pic_order_cnt[1];
            img->ThisPOC = img->framepoc = (img->toppoc < img->bottompoc)? img->toppoc : img->bottompoc; // POC200301
        }
        else if (img->bottom_field_flag==0)
        {   //top field
            img->ThisPOC = img->toppoc = img->ExpectedPicOrderCnt + img->delta_pic_order_cnt[0];
        }
        else
        {   //bottom field
            img->ThisPOC = img->bottompoc = img->ExpectedPicOrderCnt + active_sps->offset_for_top_to_bottom_field + img->delta_pic_order_cnt[0];
        }
        img->framepoc=img->ThisPOC;

        gSliceParameters.poc = img->ThisPOC;
        gSliceParameters.top_poc = img->toppoc;
        gSliceParameters.bottom_poc = img->bottompoc;

        img->PreviousFrameNum=img->frame_num;
        img->PreviousFrameNumOffset=img->FrameNumOffset;
        break;

    case 2: // POC MODE 2
        if (img->idr_flag) // IDR picture
        {
            img->FrameNumOffset=0;     //  first pix of IDRGOP,
            img->ThisPOC = img->framepoc = img->toppoc = img->bottompoc = 0;
            //if(img->frame_num)  error("frame_num != 0 in idr pix", -1020);
        }
        else
        {
            if (img->last_has_mmco_5)
            {
                img->PreviousFrameNum = 0;
                img->PreviousFrameNumOffset = 0;
            }
            if (img->frame_num<img->PreviousFrameNum)
                img->FrameNumOffset = img->PreviousFrameNumOffset + img->MaxFrameNum;
            else
                img->FrameNumOffset = img->PreviousFrameNumOffset;

            img->AbsFrameNum = img->FrameNumOffset+img->frame_num;
            if (img->disposable_flag)
                img->ThisPOC = (2*img->AbsFrameNum - 1);
            else
                img->ThisPOC = (2*img->AbsFrameNum);

            if (img->field_pic_flag==0)
                img->toppoc = img->bottompoc = img->framepoc = img->ThisPOC;
            else if (img->bottom_field_flag==0)
                img->toppoc = img->framepoc = img->ThisPOC;
            else
                img->bottompoc = img->framepoc = img->ThisPOC;
        }
        gSliceParameters.poc = img->ThisPOC;
        gSliceParameters.top_poc = img->toppoc;
        gSliceParameters.bottom_poc = img->bottompoc;
#if 0 // modified by wlHsu
        if (!img->disposable_flag)
#endif
            img->PreviousFrameNum=img->frame_num;
        img->PreviousFrameNumOffset=img->FrameNumOffset;
        break;

    default:
        //error must occurs
        //assert( 1==0 );
        av_log(NULL, AV_LOG_ERROR, "\npic_order_cnt_type error !! \n");
        break;
    }
}

int get_avaliable_pic_idx(void)
{
    MMP_UINT32 i;

    for (i=0; i < 32; i++)
    {
        if (PicIdxQueue[i] == 0)
        {
            PicIdxQueue[i] = 1;
            return i*2;
        }
    }
    return (i-1)*2;
}

int get_decoding_buf(void)
{
    MMP_UINT32 i, j;
    MMP_INT pos, poc;
   
    //clear abnormal condition
    #if 0
    for (i=0; i < gptAVCDecoder->frameBufCount; i++)
    {
        if (((DecodingQueue[i] & 0xFFFF) == WAIT_REFERENCE_RELEASE) ||
            ((DecodingQueue[i] & 0xFFFF0000) == WAIT_FLIP_RELEASE))
        {
            //printf("Clear0 buf %d f %x\n", i, DecodingQueue[i]);
             DecodingQueue[i] = 0;
        }
    }
    #endif

    j = (gptAVCDecoder->lastDisplayFrameBufIndex + 1) % gptAVCDecoder->frameBufCount;

    for (i=0; i < gptAVCDecoder->frameBufCount; i++)
    {
        if (DecodingQueue[j] == 0)
        {
            DecodingQueue[j] = WAIT_REFERENCE_RELEASE | WAIT_FLIP_RELEASE;
            return j;
        }
        j = (j+1) % gptAVCDecoder->frameBufCount;
    }

    pos=-1;
    poc = INT_MAX;
    for (i=0; i<dpb.used_size; i++)
    {
        if (poc>dpb.fs[i]->poc)
        {
            poc = dpb.fs[i]->poc;
            pos=i;
        }
    }
    if (pos == -1)
        remove_unused_frame_from_dpb();
    else
        remove_frame_from_dpb(pos);

    //remove_unused_frame_from_dpb();
    j = (gptAVCDecoder->lastDisplayFrameBufIndex + 1) % gptAVCDecoder->frameBufCount;

    for (i=0; i < gptAVCDecoder->frameBufCount; i++)
    {
        if (DecodingQueue[j] == 0)
        {
            DecodingQueue[j] = WAIT_REFERENCE_RELEASE | WAIT_FLIP_RELEASE;
            return j;
        }
        j = (j+1) % gptAVCDecoder->frameBufCount;
    }

    return j;
}

void fill_frame_num_gap(ImageParameters *img)
{
    int CurrFrameNum;
    int UnusedShortTermFrameNum;
    StorablePicture *picture = NULL;
    int tmp1 = img->delta_pic_order_cnt[0];
    int tmp2 = img->delta_pic_order_cnt[1];
    img->delta_pic_order_cnt[0] = img->delta_pic_order_cnt[1] = 0;

    UnusedShortTermFrameNum = (img->pre_frame_num + 1) % img->MaxFrameNum;
    CurrFrameNum = img->frame_num;

    //while (CurrFrameNum != UnusedShortTermFrameNum)
    ////while (dpb.ref_frames_in_buffer < img->num_ref_idx_l0_active)
    //{
    //      picture = alloc_storable_picture (FRAME);
    //      picture->coded_frame = 1;
    //      picture->pic_num = UnusedShortTermFrameNum;
    //      picture->frame_num = UnusedShortTermFrameNum;
    //      picture->non_existing = 1;
    //      picture->is_output = 1;
    //      picture->used_for_reference = 1;
    //
    //      picture->adaptive_ref_pic_buffering_flag = 0;
    //
    //      img->frame_num = UnusedShortTermFrameNum;
    //      if (active_sps->pic_order_cnt_type!=0)
    //      {
    //          decode_poc();
    //      }
    //      picture->top_poc=img->toppoc;
    //      picture->bottom_poc=img->bottompoc;
    //      picture->frame_poc=img->framepoc;
    //      picture->poc=img->framepoc;
    //
    //      // update buf index
    //      picture->decoding_buf_idx = 0;
    //      picture->col_data_buf_idx = 0;
    //      picture->pic_idx = 0;
    //
    //      //printf("Store one frame\n");
    //      store_picture_in_dpb(picture);
    //
    //      picture=NULL;
    //      img->pre_frame_num = UnusedShortTermFrameNum;
    //      UnusedShortTermFrameNum = (UnusedShortTermFrameNum + 1) % img->MaxFrameNum;
    //}
    img->pre_frame_num = CurrFrameNum - 1;
    img->delta_pic_order_cnt[0] = tmp1;
    img->delta_pic_order_cnt[1] = tmp2;
    img->frame_num = CurrFrameNum;
}

static AVC_ERROR_CODE init_picture(void)
{
    AVC_ERROR_CODE ret = AVC_ERROR_SUCCESS;

    if (dec_picture)
    {
        // this may only happen on slice loss
        exit_picture();
    }

    if (gbCheckFrmGap)
        if (img->frame_num != img->pre_frame_num && img->frame_num != (img->pre_frame_num + 1) % img->MaxFrameNum)
        {
            if (active_sps->gaps_in_frame_num_value_allowed_flag == 0)
            {
                //   /* Advanced Error Concealment would be called here to combat unintentional loss of pictures. */
                //   error("An unintentional loss of pictures occurs! Exit\n", 100);
                av_log(NULL, AV_LOG_ERROR, "Error frame num gap %d %d\n", img->frame_num, img->pre_frame_num);
            }
            fill_frame_num_gap(img);
        }
    img->pre_frame_num = img->frame_num;

    //calculate POC
    decode_poc();

    dec_picture = alloc_storable_picture (img->structure);
    dec_picture->top_poc=img->toppoc;
    dec_picture->bottom_poc=img->bottompoc;
    dec_picture->frame_poc=img->framepoc;

    switch (img->structure)
    {
    case TOP_FIELD:
        {
            dec_picture->poc=img->toppoc;
            break;
        }
    case BOTTOM_FIELD:
        {
            dec_picture->poc=img->bottompoc;
            break;
        }
    case FRAME:
        {
            dec_picture->poc=img->framepoc;
            break;
        }
    default:
        break;
    }

    dec_picture->slice_type = img->type;
    dec_picture->used_for_reference = (img->nal_reference_idc != 0);
    dec_picture->idr_flag = img->idr_flag;
    dec_picture->no_output_of_prior_pics_flag = img->no_output_of_prior_pics_flag;
    dec_picture->long_term_reference_flag = img->long_term_reference_flag;
    dec_picture->adaptive_ref_pic_buffering_flag = img->adaptive_ref_pic_buffering_flag;

    dec_picture->dec_ref_pic_marking_buffer = img->dec_ref_pic_marking_buffer;
    img->dec_ref_pic_marking_buffer = MMP_NULL;

    dec_picture->MbaffFrameFlag = img->MbaffFrameFlag;
    // dec_picture->PicWidthInMbs = img->PicWidthInMbs;
    dec_picture->pic_num = img->frame_num;
    dec_picture->frame_num = img->frame_num;
    dec_picture->coded_frame = (img->structure==FRAME);

    // update buf index
    if (dpb.last_picture)
    {
        if (((dec_picture->structure==TOP_FIELD)&&(dpb.last_picture->is_used==2))||((dec_picture->structure==BOTTOM_FIELD)&&(dpb.last_picture->is_used==1)))
        {
            if ((dec_picture->used_for_reference && (dpb.last_picture->is_orig_reference!=0))||
                (!dec_picture->used_for_reference && (dpb.last_picture->is_orig_reference==0)))
            {
                if (dec_picture->structure==TOP_FIELD)
                {
                    dec_picture->decoding_buf_idx = dpb.last_picture->bottom_field->decoding_buf_idx;
                    dec_picture->col_data_buf_idx = dec_picture->decoding_buf_idx*2;
                    dec_picture->pic_idx = dpb.last_picture->bottom_field->pic_idx - 1;
                    dec_picture->pic_count = dpb.last_picture->bottom_field->pic_count;
                } else {
                    dec_picture->decoding_buf_idx = dpb.last_picture->top_field->decoding_buf_idx;
                    dec_picture->col_data_buf_idx = dec_picture->decoding_buf_idx*2+1;
                    dec_picture->pic_idx = dpb.last_picture->top_field->pic_idx + 1;
                    dec_picture->pic_count = dpb.last_picture->top_field->pic_count;
                }
            } else
                return ERROR_BITSTREAM_ERROR;
        } else
            return ERROR_BITSTREAM_ERROR;
    } else {
        img->decoding_buf_idx = dec_picture->decoding_buf_idx = get_decoding_buf();
        dec_picture->pic_count = gDePicCount++;

        if (dec_picture->structure==BOTTOM_FIELD)
        {
            dec_picture->col_data_buf_idx = dec_picture->decoding_buf_idx*2+1;
            dec_picture->pic_idx = get_avaliable_pic_idx() + 1;
        } else {
            dec_picture->col_data_buf_idx = dec_picture->decoding_buf_idx*2;
            dec_picture->pic_idx = get_avaliable_pic_idx();
        }
    }

    return ret;
}

static StorablePicture*  get_short_term_pic(int picNum)
{
    unsigned i;

    for (i=0; i<dpb.ref_frames_in_buffer; i++)
    {
        if (img->structure==FRAME)
        {
            if (dpb.fs_ref[i]->is_reference == 3)
                if ((!dpb.fs_ref[i]->frame->is_long_term)&&(dpb.fs_ref[i]->frame->pic_num == picNum))
                    return dpb.fs_ref[i]->frame;
        }
        else
        {
            if (dpb.fs_ref[i]->is_reference & 1)
                if ((!dpb.fs_ref[i]->top_field->is_long_term)&&(dpb.fs_ref[i]->top_field->pic_num == picNum))
                    return dpb.fs_ref[i]->top_field;
            if (dpb.fs_ref[i]->is_reference & 2)
                if ((!dpb.fs_ref[i]->bottom_field->is_long_term)&&(dpb.fs_ref[i]->bottom_field->pic_num == picNum))
                    return dpb.fs_ref[i]->bottom_field;
        }
    }
    return MMP_NULL;
}

static void reorder_short_term(StorablePicture **RefPicListX, int num_ref_idx_lX_active_minus1, int picNumLX, int *refIdxLX)
{
    int cIdx, nIdx;
    StorablePicture *picLX;

    picLX = get_short_term_pic(picNumLX);

    for ( cIdx = num_ref_idx_lX_active_minus1+1; cIdx > *refIdxLX; cIdx-- )
        RefPicListX[ cIdx ] = RefPicListX[ cIdx - 1];

    RefPicListX[ (*refIdxLX)++ ] = picLX;

    nIdx = *refIdxLX;

    for ( cIdx = *refIdxLX; cIdx <= num_ref_idx_lX_active_minus1+1; cIdx++ )
        if (RefPicListX[ cIdx ])
            if ( (RefPicListX[ cIdx ]->is_long_term ) ||  (RefPicListX[ cIdx ]->pic_num != picNumLX ))
                RefPicListX[ nIdx++ ] = RefPicListX[ cIdx ];
}

static StorablePicture*  get_long_term_pic(int LongtermPicNum)
{
    unsigned i;

    for (i=0; i<dpb.ltref_frames_in_buffer; i++)
    {
        if (img->structure==FRAME)
        {
            if (dpb.fs_ltref[i]->is_reference == 3)
                if ((dpb.fs_ltref[i]->frame->is_long_term)&&(dpb.fs_ltref[i]->frame->long_term_pic_num == LongtermPicNum))
                    return dpb.fs_ltref[i]->frame;
        }
        else
        {
            if (dpb.fs_ltref[i]->is_reference & 1)
                if ((dpb.fs_ltref[i]->top_field->is_long_term)&&(dpb.fs_ltref[i]->top_field->long_term_pic_num == LongtermPicNum))
                    return dpb.fs_ltref[i]->top_field;
            if (dpb.fs_ltref[i]->is_reference & 2)
                if ((dpb.fs_ltref[i]->bottom_field->is_long_term)&&(dpb.fs_ltref[i]->bottom_field->long_term_pic_num == LongtermPicNum))
                    return dpb.fs_ltref[i]->bottom_field;
        }
    }
    return MMP_NULL;
}

static void reorder_long_term(StorablePicture **RefPicListX, int num_ref_idx_lX_active_minus1, int LongTermPicNum, int *refIdxLX)
{
    int cIdx, nIdx;
    StorablePicture *picLX;

    picLX = get_long_term_pic(LongTermPicNum);

    for ( cIdx = num_ref_idx_lX_active_minus1+1; cIdx > *refIdxLX; cIdx-- )
        RefPicListX[ cIdx ] = RefPicListX[ cIdx - 1];

    RefPicListX[ (*refIdxLX)++ ] = picLX;

    nIdx = *refIdxLX;

    for ( cIdx = *refIdxLX; cIdx <= num_ref_idx_lX_active_minus1+1; cIdx++ )
        if ( (!RefPicListX[ cIdx ]->is_long_term ) ||  (RefPicListX[ cIdx ]->long_term_pic_num != LongTermPicNum ))
            RefPicListX[ nIdx++ ] = RefPicListX[ cIdx ];
}

void reorder_ref_pic_list(StorablePicture **list, int *list_size, int num_ref_idx_lX_active_minus1, int *remapping_of_pic_nums_idc, int *abs_diff_pic_num_minus1, int *long_term_pic_idx)
{
    int i;
    int maxPicNum, currPicNum, picNumLXNoWrap, picNumLXPred, picNumLX;
    int refIdxLX = 0;

    if (img->structure==FRAME)
    {
        maxPicNum  = img->MaxFrameNum;
        currPicNum = img->frame_num;
    }
    else
    {
        maxPicNum  = 2 * img->MaxFrameNum;
        currPicNum = 2 * img->frame_num + 1;
    }

    picNumLXPred = currPicNum;

    for (i=0; remapping_of_pic_nums_idc[i]!=3; i++)
    {
        if (i > (num_ref_idx_lX_active_minus1+2))
        {
            //if (remapping_of_pic_nums_idc[i]>3)
            //{
            av_log(NULL, AV_LOG_ERROR, "Invalid remapping_of_pic_nums_idc command\n");
            break;
        }

        if (remapping_of_pic_nums_idc[i] < 2)
        {
            if (remapping_of_pic_nums_idc[i] == 0)
            {
                if ( picNumLXPred - ( abs_diff_pic_num_minus1[i] + 1 ) < 0 )
                    picNumLXNoWrap = picNumLXPred - ( abs_diff_pic_num_minus1[i] + 1 ) + maxPicNum;
                else
                    picNumLXNoWrap = picNumLXPred - ( abs_diff_pic_num_minus1[i] + 1 );
            }
            else // (remapping_of_pic_nums_idc[i] == 1)
            {
                if ( picNumLXPred + ( abs_diff_pic_num_minus1[i] + 1 )  >=  maxPicNum )
                    picNumLXNoWrap = picNumLXPred + ( abs_diff_pic_num_minus1[i] + 1 ) - maxPicNum;
                else
                    picNumLXNoWrap = picNumLXPred + ( abs_diff_pic_num_minus1[i] + 1 );
            }
            picNumLXPred = picNumLXNoWrap;

            if ( picNumLXNoWrap > currPicNum )
                picNumLX = picNumLXNoWrap - maxPicNum;
            else
                picNumLX = picNumLXNoWrap;

            reorder_short_term(list, num_ref_idx_lX_active_minus1, picNumLX, &refIdxLX);
        }
        else //(remapping_of_pic_nums_idc[i] == 2)
        {
            reorder_long_term(list, num_ref_idx_lX_active_minus1, long_term_pic_idx[i], &refIdxLX);
        }
    }
    // that's a definition
    *list_size = num_ref_idx_lX_active_minus1 + 1;
}

void free_ref_pic_list_reordering_buffer(Slice *currSlice)
{
    if (currSlice->remapping_of_pic_nums_idc_l0)
    {
        free(currSlice->remapping_of_pic_nums_idc_l0);
        currSlice->remapping_of_pic_nums_idc_l0 = MMP_NULL;
    }

    if (currSlice->abs_diff_pic_num_minus1_l0)
    {
        free(currSlice->abs_diff_pic_num_minus1_l0);
        currSlice->abs_diff_pic_num_minus1_l0 = MMP_NULL;
    }

    if (currSlice->long_term_pic_idx_l0)
    {
        free(currSlice->long_term_pic_idx_l0);
        currSlice->long_term_pic_idx_l0 = MMP_NULL;
    }

    if (currSlice->remapping_of_pic_nums_idc_l1)
    {
        free(currSlice->remapping_of_pic_nums_idc_l1);
        currSlice->remapping_of_pic_nums_idc_l1 = MMP_NULL;
    }

    if (currSlice->abs_diff_pic_num_minus1_l1)
    {
        free(currSlice->abs_diff_pic_num_minus1_l1);
        currSlice->abs_diff_pic_num_minus1_l1 = MMP_NULL;
    }

    if (currSlice->long_term_pic_idx_l1)
    {
        free(currSlice->long_term_pic_idx_l1);
        currSlice->long_term_pic_idx_l1 = MMP_NULL;
    }
}

void reorder_lists(int currSliceType, Slice * currSlice)
{
    if ((currSliceType != I_SLICE)&&(currSliceType != SI_SLICE))
    {
        if (currSlice->ref_pic_list_reordering_flag_l0)
        {
            reorder_ref_pic_list(listX[0], &listXsize[0],
                                 img->num_ref_idx_l0_active - 1,
                                 currSlice->remapping_of_pic_nums_idc_l0,
                                 currSlice->abs_diff_pic_num_minus1_l0,
                                 currSlice->long_term_pic_idx_l0);
        }

        //if (MMP_NULL == listX[0][img->num_ref_idx_l0_active-1])
        //{
        //  error("number of entries in list 0 smaller than num_ref_idx_l0_active_minus1",500);
        //}
        // that's a definition
        listXsize[0] = img->num_ref_idx_l0_active;
    }
    if (currSliceType == B_SLICE)
    {
        if (currSlice->ref_pic_list_reordering_flag_l1)
        {
            reorder_ref_pic_list(listX[1], &listXsize[1],
                                 img->num_ref_idx_l1_active - 1,
                                 currSlice->remapping_of_pic_nums_idc_l1,
                                 currSlice->abs_diff_pic_num_minus1_l1,
                                 currSlice->long_term_pic_idx_l1);
        }
        //if (MMP_NULL == listX[1][img->num_ref_idx_l1_active-1])
        //{
        //  error("number of entries in list 1 smaller than num_ref_idx_l1_active_minus1",500);
        //}
        // that's a definition
        listXsize[1] = img->num_ref_idx_l1_active;
    }

    free_ref_pic_list_reordering_buffer(currSlice);
}

static int compare_pic_by_pic_num_desc( const void *arg1, const void *arg2 )
{
    if ( (*(StorablePicture**)arg1)->pic_num < (*(StorablePicture**)arg2)->pic_num)
        return 1;
    if ( (*(StorablePicture**)arg1)->pic_num > (*(StorablePicture**)arg2)->pic_num)
        return -1;
    else
        return 0;
}

static int compare_pic_by_lt_pic_num_asc( const void *arg1, const void *arg2 )
{
    if ( (*(StorablePicture**)arg1)->long_term_pic_num < (*(StorablePicture**)arg2)->long_term_pic_num)
        return -1;
    if ( (*(StorablePicture**)arg1)->long_term_pic_num > (*(StorablePicture**)arg2)->long_term_pic_num)
        return 1;
    else
        return 0;
}

static int compare_fs_by_frame_num_desc( const void *arg1, const void *arg2 )
{
    if ( (*(FrameStore**)arg1)->frame_num_wrap < (*(FrameStore**)arg2)->frame_num_wrap)
        return 1;
    if ( (*(FrameStore**)arg1)->frame_num_wrap > (*(FrameStore**)arg2)->frame_num_wrap)
        return -1;
    else
        return 0;
}

static int compare_fs_by_lt_pic_idx_asc( const void *arg1, const void *arg2 )
{
    if ( (*(FrameStore**)arg1)->long_term_frame_idx < (*(FrameStore**)arg2)->long_term_frame_idx)
        return -1;
    if ( (*(FrameStore**)arg1)->long_term_frame_idx > (*(FrameStore**)arg2)->long_term_frame_idx)
        return 1;
    else
        return 0;
}

static int compare_pic_by_poc_asc( const void *arg1, const void *arg2 )
{
    if ( (*(StorablePicture**)arg1)->poc < (*(StorablePicture**)arg2)->poc)
        return -1;
    if ( (*(StorablePicture**)arg1)->poc > (*(StorablePicture**)arg2)->poc)
        return 1;
    else
        return 0;
}

static int compare_pic_by_poc_desc( const void *arg1, const void *arg2 )
{
    if ( (*(StorablePicture**)arg1)->poc < (*(StorablePicture**)arg2)->poc)
        return 1;
    if ( (*(StorablePicture**)arg1)->poc > (*(StorablePicture**)arg2)->poc)
        return -1;
    else
        return 0;
}

static int compare_fs_by_poc_asc( const void *arg1, const void *arg2 )
{
    if ( (*(FrameStore**)arg1)->poc < (*(FrameStore**)arg2)->poc)
        return -1;
    if ( (*(FrameStore**)arg1)->poc > (*(FrameStore**)arg2)->poc)
        return 1;
    else
        return 0;
}

static int compare_fs_by_poc_desc( const void *arg1, const void *arg2 )
{
    if ( (*(FrameStore**)arg1)->poc < (*(FrameStore**)arg2)->poc)
        return 1;
    if ( (*(FrameStore**)arg1)->poc > (*(FrameStore**)arg2)->poc)
        return -1;
    else
        return 0;
}

int is_short_ref(StorablePicture *s)
{
    return ((s->used_for_reference) && (!(s->is_long_term)));
}

int is_long_ref(StorablePicture *s)
{
    return ((s->used_for_reference) && (s->is_long_term));
}

static void gen_pic_list_from_frame_list(PictureStructure currStrcture, FrameStore **fs_list, int list_idx, StorablePicture **list, int *list_size, int long_term)
{
    int top_idx = 0;
    int bot_idx = 0;

    int (*is_ref)(StorablePicture *s);

    if (long_term)
        is_ref=is_long_ref;
    else
        is_ref=is_short_ref;

    if (currStrcture == TOP_FIELD)
    {
        while ((top_idx<list_idx)||(bot_idx<list_idx))
        {
            for ( ; top_idx<list_idx; top_idx++)
            {
                if (fs_list[top_idx]->is_used & 1)
                {
                    if (is_ref(fs_list[top_idx]->top_field))
                    {
                        // short term ref pic
                        list[*list_size] = fs_list[top_idx]->top_field;
                        (*list_size)++;
                        top_idx++;
                        break;
                    }
                }
            }
            for ( ; bot_idx<list_idx; bot_idx++)
            {
                if (fs_list[bot_idx]->is_used & 2)
                {
                    if (is_ref(fs_list[bot_idx]->bottom_field))
                    {
                        // short term ref pic
                        list[*list_size] = fs_list[bot_idx]->bottom_field;
                        (*list_size)++;
                        bot_idx++;
                        break;
                    }
                }
            }
        }
    }
    if (currStrcture == BOTTOM_FIELD)
    {
        while ((top_idx<list_idx)||(bot_idx<list_idx))
        {
            for ( ; bot_idx<list_idx; bot_idx++)
            {
                if (fs_list[bot_idx]->is_used & 2)
                {
                    if (is_ref(fs_list[bot_idx]->bottom_field))
                    {
                        // short term ref pic
                        list[*list_size] = fs_list[bot_idx]->bottom_field;
                        (*list_size)++;
                        bot_idx++;
                        break;
                    }
                }
            }
            for ( ; top_idx<list_idx; top_idx++)
            {
                if (fs_list[top_idx]->is_used & 1)
                {
                    if (is_ref(fs_list[top_idx]->top_field))
                    {
                        // short term ref pic
                        list[*list_size] = fs_list[top_idx]->top_field;
                        (*list_size)++;
                        top_idx++;
                        break;
                    }
                }
            }
        }
    }
}

void init_lists(int currSliceType, PictureStructure currPicStructure)
{
    int add_top = 0, add_bottom = 0;
    unsigned i;
    int j;
    int MaxFrameNum = 1 << (active_sps->log2_max_frame_num_minus4 + 4);
    int diff;
    int list0idx = 0;
    int list0idx_1 = 0;
    int listltidx = 0;
    FrameStore **fs_list0;
    FrameStore **fs_list1;
    FrameStore **fs_listlt;

    StorablePicture *tmp_s;

    if (currPicStructure == FRAME)
    {
        // frame case
        for (i=0; i<dpb.ref_frames_in_buffer; i++)
        {
            if (dpb.fs_ref[i]->is_used==3)
            {
                if ((dpb.fs_ref[i]->frame->used_for_reference)&&(!dpb.fs_ref[i]->frame->is_long_term))
                {
                    if( dpb.fs_ref[i]->frame_num > img->frame_num )
                    {
                        dpb.fs_ref[i]->frame_num_wrap = dpb.fs_ref[i]->frame_num - MaxFrameNum;
                    }
                    else
                    {
                        dpb.fs_ref[i]->frame_num_wrap = dpb.fs_ref[i]->frame_num;
                    }
                    dpb.fs_ref[i]->frame->pic_num = dpb.fs_ref[i]->frame_num_wrap;
                    // dpb.fs_ref[i]->frame->order_num = list0idx;
                }
            }
        }
    }
    else
    {
        if (currPicStructure == TOP_FIELD)
        {
            add_top    = 1;
            add_bottom = 0;
        }
        else
        {
            add_top    = 0;
            add_bottom = 1;
        }

        for (i=0; i<dpb.ref_frames_in_buffer; i++)
        {
            if (dpb.fs_ref[i]->is_reference)
            {
                if (dpb.fs_ref[i]->frame_num > img->frame_num)
                {
                    dpb.fs_ref[i]->frame_num_wrap = dpb.fs_ref[i]->frame_num - MaxFrameNum;
                }
                else
                {
                    dpb.fs_ref[i]->frame_num_wrap = dpb.fs_ref[i]->frame_num;
                }
                if (dpb.fs_ref[i]->is_reference & 1)
                {
                    dpb.fs_ref[i]->top_field->pic_num = (2 * dpb.fs_ref[i]->frame_num_wrap) + add_top;
                }
                if (dpb.fs_ref[i]->is_reference & 2)
                {
                    dpb.fs_ref[i]->bottom_field->pic_num = (2 * dpb.fs_ref[i]->frame_num_wrap) + add_bottom;
                }
            }
        }
    }

    if ((currSliceType == I_SLICE)||(currSliceType == SI_SLICE))
    {
        listXsize[0] = 0;
        listXsize[1] = 0;
        return;
    }

    if ((currSliceType == P_SLICE)||(currSliceType == SP_SLICE))
    {
        // Calculate FrameNumWrap and PicNum
        if (currPicStructure == FRAME)
        {
            // frame case => P type
            for (i=0; i<dpb.ref_frames_in_buffer; i++)
            {
                if (dpb.fs_ref[i]->is_used==3)
                {
                    if ((dpb.fs_ref[i]->frame->used_for_reference)&&(!dpb.fs_ref[i]->frame->is_long_term))
                    {
                        listX[0][list0idx++] = dpb.fs_ref[i]->frame;
                    }
                }
            }
            // order list 0 by PicNum
            qsort((void *)listX[0], list0idx, sizeof(StorablePicture*), compare_pic_by_pic_num_desc);
            listXsize[0] = list0idx;

            // long term handling
            for (i=0; i<dpb.ltref_frames_in_buffer; i++)
            {
                if (dpb.fs_ltref[i]->is_used==3)
                {
                    if (dpb.fs_ltref[i]->frame->is_long_term)
                    {
                        dpb.fs_ltref[i]->frame->long_term_pic_num = dpb.fs_ltref[i]->frame->long_term_frame_idx;
                        // dpb.fs_ltref[i]->frame->order_num = list0idx;
                        listX[0][list0idx++]=dpb.fs_ltref[i]->frame;
                    }
                }
            }
            qsort((void *)&listX[0][listXsize[0]], list0idx-listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
            listXsize[0] = list0idx;
        }
        else
        {
            // field case => P type
            fs_list0 = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
            if (NULL==fs_list0)
            {
                av_log(NULL, AV_LOG_ERROR, "Fail Allocate fs_list0\n");
            }

            fs_listlt = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
            if (NULL==fs_listlt)
            {
                av_log(NULL, AV_LOG_ERROR, "Fail Allocate fs_list0\n");
            }

            for (i=0; i<dpb.ref_frames_in_buffer; i++)
            {
                if (dpb.fs_ref[i]->is_reference)
                {
                    fs_list0[list0idx++] = dpb.fs_ref[i];
                }
            }

            qsort((void *)fs_list0, list0idx, sizeof(FrameStore*), compare_fs_by_frame_num_desc);

            listXsize[0] = 0;
            gen_pic_list_from_frame_list(currPicStructure, fs_list0, list0idx, listX[0], &listXsize[0], 0);

            // long term handling
            for (i=0; i<dpb.ltref_frames_in_buffer; i++)
            {
                fs_listlt[listltidx++]=dpb.fs_ltref[i];
                if (dpb.fs_ltref[i]->is_long_term & 1)
                {
                    dpb.fs_ltref[i]->top_field->long_term_pic_num = 2 * dpb.fs_ltref[i]->top_field->long_term_frame_idx + add_top;
                }
                if (dpb.fs_ltref[i]->is_long_term & 2)
                {
                    dpb.fs_ltref[i]->bottom_field->long_term_pic_num = 2 * dpb.fs_ltref[i]->bottom_field->long_term_frame_idx + add_bottom;
                }
            }

            qsort((void *)fs_listlt, listltidx, sizeof(FrameStore*), compare_fs_by_lt_pic_idx_asc);

            gen_pic_list_from_frame_list(currPicStructure, fs_listlt, listltidx, listX[0], &listXsize[0], 1);

            free(fs_list0);
            free(fs_listlt);
        }
        listXsize[1] = 0;
    }
    else
    {
        // B-Slice
        if (currPicStructure == FRAME)
        {
            // frame case => B type
            for (i=0; i<dpb.ref_frames_in_buffer; i++)
            {
                if (dpb.fs_ref[i]->is_used==3)
                {
                    if ((dpb.fs_ref[i]->frame->used_for_reference)&&(!dpb.fs_ref[i]->frame->is_long_term))
                    {
                        if (img->framepoc > dpb.fs_ref[i]->frame->poc)
                        {
                            // dpb.fs_ref[i]->frame->order_num = list0idx;
                            listX[0][list0idx++] = dpb.fs_ref[i]->frame;
                        }
                    }
                }
            }
            qsort((void *)listX[0], list0idx, sizeof(StorablePicture*), compare_pic_by_poc_desc);
            list0idx_1 = list0idx;
            for (i=0; i<dpb.ref_frames_in_buffer; i++)
            {
                if (dpb.fs_ref[i]->is_used==3)
                {
                    if ((dpb.fs_ref[i]->frame->used_for_reference)&&(!dpb.fs_ref[i]->frame->is_long_term))
                    {
                        if (img->framepoc < dpb.fs_ref[i]->frame->poc)
                        {
                            // dpb.fs_ref[i]->frame->order_num = list0idx;
                            listX[0][list0idx++] = dpb.fs_ref[i]->frame;
                        }
                    }
                }
            }
            qsort((void *)&listX[0][list0idx_1], list0idx-list0idx_1, sizeof(StorablePicture*), compare_pic_by_poc_asc);

            for (j=0; j<list0idx_1; j++)
            {
                listX[1][list0idx-list0idx_1+j]=listX[0][j];
            }
            for (j=list0idx_1; j<list0idx; j++)
            {
                listX[1][j-list0idx_1]=listX[0][j];
            }

            listXsize[0] = listXsize[1] = list0idx;

            // long term handling
            for (i=0; i<dpb.ltref_frames_in_buffer; i++)
            {
                if (dpb.fs_ltref[i]->is_used==3)
                {
                    if (dpb.fs_ltref[i]->frame->is_long_term)
                    {
                        dpb.fs_ltref[i]->frame->long_term_pic_num = dpb.fs_ltref[i]->frame->long_term_frame_idx;
//                      dpb.fs_ltref[i]->frame->order_num = list0idx;

                        listX[0][list0idx]  =dpb.fs_ltref[i]->frame;
                        listX[1][list0idx++]=dpb.fs_ltref[i]->frame;
                    }
                }
            }
            qsort((void *)&listX[0][listXsize[0]], list0idx-listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
            qsort((void *)&listX[1][listXsize[0]], list0idx-listXsize[0], sizeof(StorablePicture*), compare_pic_by_lt_pic_num_asc);
            listXsize[0] = listXsize[1] = list0idx;
        }
        else
        {
            // field case => B type
            fs_list0 = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
            if (NULL==fs_list0)
            {
                av_log(NULL, AV_LOG_ERROR, "Fail Allocate fs_list0\n");
            }

            fs_list1 = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
            if (NULL==fs_list1)
            {
                av_log(NULL, AV_LOG_ERROR, "Fail Allocate fs_list0\n");
            }

            fs_listlt = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
            if (NULL==fs_listlt)
            {
                av_log(NULL, AV_LOG_ERROR, "Fail Allocate fs_list0\n");
            }

            listXsize[0] = 0;
            listXsize[1] = 1;

            for (i=0; i<dpb.ref_frames_in_buffer; i++)
            {
                if (dpb.fs_ref[i]->is_used)
                {
                    if (img->ThisPOC >= dpb.fs_ref[i]->poc)
                    {
                        fs_list0[list0idx++] = dpb.fs_ref[i];
                    }
                }
            }
            qsort((void *)fs_list0, list0idx, sizeof(FrameStore*), compare_fs_by_poc_desc);
            list0idx_1 = list0idx;
            for (i=0; i<dpb.ref_frames_in_buffer; i++)
            {
                if (dpb.fs_ref[i]->is_used)
                {
                    if (img->ThisPOC < dpb.fs_ref[i]->poc)
                    {
                        fs_list0[list0idx++] = dpb.fs_ref[i];
                    }
                }
            }
            qsort((void *)&fs_list0[list0idx_1], list0idx-list0idx_1, sizeof(FrameStore*), compare_fs_by_poc_asc);

            for (j=0; j<list0idx_1; j++)
            {
                fs_list1[list0idx-list0idx_1+j]=fs_list0[j];
            }
            for (j=list0idx_1; j<list0idx; j++)
            {
                fs_list1[j-list0idx_1]=fs_list0[j];
            }

            listXsize[0] = 0;
            listXsize[1] = 0;
            gen_pic_list_from_frame_list(currPicStructure, fs_list0, list0idx, listX[0], &listXsize[0], 0);
            gen_pic_list_from_frame_list(currPicStructure, fs_list1, list0idx, listX[1], &listXsize[1], 0);

            // long term handling
            for (i=0; i<dpb.ltref_frames_in_buffer; i++)
            {
                fs_listlt[listltidx++]=dpb.fs_ltref[i];
                if (dpb.fs_ltref[i]->is_long_term & 1)
                {
                    dpb.fs_ltref[i]->top_field->long_term_pic_num = 2 * dpb.fs_ltref[i]->top_field->long_term_frame_idx + add_top;
                }
                if (dpb.fs_ltref[i]->is_long_term & 2)
                {
                    dpb.fs_ltref[i]->bottom_field->long_term_pic_num = 2 * dpb.fs_ltref[i]->bottom_field->long_term_frame_idx + add_bottom;
                }
            }

            qsort((void *)fs_listlt, listltidx, sizeof(FrameStore*), compare_fs_by_lt_pic_idx_asc);

            gen_pic_list_from_frame_list(currPicStructure, fs_listlt, listltidx, listX[0], &listXsize[0], 1);
            gen_pic_list_from_frame_list(currPicStructure, fs_listlt, listltidx, listX[1], &listXsize[1], 1);

            free(fs_list0);
            free(fs_list1);
            free(fs_listlt);
        }

#if 1 // modified by wlHsu
        if ((listXsize[0] == listXsize[1]) && (listXsize[0] > 1))
        {
            // check if lists are identical, if yes swap first two elements of listX[1]
            diff = 0;

            for (j = 0; j < listXsize[0]; j++)
            {
                if (listX[0][j] != listX[1][j])
                {
                    diff = 1;
                    break;
                }
            }

            if (!diff)
            {
                tmp_s = listX[1][0];
                listX[1][0] = listX[1][1];
                listX[1][1] = tmp_s;
            }
        }
    }
#else
    }

    if ((listXsize[0] == listXsize[1]) && (listXsize[0] > 1))
    {
        // check if lists are identical, if yes swap first two elements of listX[1]
        diff=0;
        for (j = 0; j< listXsize[0]; j++)
        {
            if (listX[0][j]!=listX[1][j])
                diff=1;
        }
        if (!diff)
        {
            tmp_s = listX[1][0];
            listX[1][0]=listX[1][1];
            listX[1][1]=tmp_s;
        }
    }
#endif
    // set max size
    listXsize[0] = min (listXsize[0], img->num_ref_idx_l0_active);
    listXsize[1] = min (listXsize[1], img->num_ref_idx_l1_active);

    // set the unused list entries to NULL
    for (i=listXsize[0]; i< (MAX_LIST_SIZE) ; i++)
    {
        listX[0][i] = NULL;
    }
    for (i=listXsize[1]; i< (MAX_LIST_SIZE) ; i++)
    {
        listX[1][i] = NULL;
    }
}

void init_mbaff_lists(void)
{
    unsigned j;
    int i;

    for (i=2;i<6;i++)
    {
        if( listX[i] )
        {
            for (j = 0; j < MAX_LIST_SIZE; j++)
            {
                listX[i][j] = NULL;
            }
        }

        listXsize[i]=0;
    }

    for (i=0; i<listXsize[0]; i++)
    {
        listX[2][2*i]  =listX[0][i]->top_field;
        listX[2][2*i+1]=listX[0][i]->bottom_field;
        listX[4][2*i]  =listX[0][i]->bottom_field;
        listX[4][2*i+1]=listX[0][i]->top_field;
    }
    listXsize[2]=listXsize[4]=listXsize[0] * 2;

    for (i=0; i<listXsize[1]; i++)
    {
        listX[3][2*i]  =listX[1][i]->top_field;
        listX[3][2*i+1]=listX[1][i]->bottom_field;
        listX[5][2*i]  =listX[1][i]->bottom_field;
        listX[5][2*i+1]=listX[1][i]->top_field;
    }
    listXsize[3]=listXsize[5]=listXsize[1] * 2;
}

void AssignQuantParam(AVC_PICTURE_PARAMETER_SET* pps, AVC_SEQUENCE_PARAMETER_SET* sps)
{
MMP_UINT32 i;

    if (!pps->pic_scaling_matrix_present_flag && !sps->seq_scaling_matrix_present_flag)
    {
        for (i=0; i<8; i++)
            qmatrix[i] = (i<6) ? quant_org:quant8_org;
    }
    else
    {
        if (sps->seq_scaling_matrix_present_flag) // check sps first
        {
            for (i=0; i<8; i++)
            {
                if (i<6)
                {
                    if (!sps->seq_scaling_list_present_flag[i]) // fall-back rule A
                    {
                        if ((i==0) || (i==3))
                            qmatrix[i] = (i==0) ? quant_intra_default:quant_inter_default;
                        else
                            qmatrix[i] = qmatrix[i-1];
                    }
                    else
                    {
                        if (sps->UseDefaultScalingMatrix4x4Flag[i])
                            qmatrix[i] = (i<3) ? quant_intra_default:quant_inter_default;
                        else
                            qmatrix[i] = sps->ScalingList4x4[i];
                    }
                }
                else
                {
                    if (!sps->seq_scaling_list_present_flag[i] || sps->UseDefaultScalingMatrix8x8Flag[i-6]) // fall-back rule A
                        qmatrix[i] = (i==6) ? quant8_intra_default:quant8_inter_default;
                    else
                        qmatrix[i] = sps->ScalingList8x8[i-6];
                }
            }
        }

        if (pps->pic_scaling_matrix_present_flag) // then check pps
        {
            for (i=0; i<8; i++)
            {
                if (i<6)
                {
                    if (!pps->pic_scaling_list_present_flag[i]) // fall-back rule B
                    {
                        if ((i==0) || (i==3))
                        {
                            if (!sps->seq_scaling_matrix_present_flag)
                                qmatrix[i] = (i==0) ? quant_intra_default:quant_inter_default;
                        }
                        else
                            qmatrix[i] = qmatrix[i-1];
                    }
                    else
                    {
                        if (pps->UseDefaultScalingMatrix4x4Flag[i])
                            qmatrix[i] = (i<3) ? quant_intra_default:quant_inter_default;
                        else
                            qmatrix[i] = pps->ScalingList4x4[i];
                    }
                }
                else
                {
                    if (!pps->pic_scaling_list_present_flag[i]) // fall-back rule B
                    {
                        if (!sps->seq_scaling_matrix_present_flag)
                            qmatrix[i] = (i==6) ? quant8_intra_default:quant8_inter_default;
                    }
                    else if (pps->UseDefaultScalingMatrix8x8Flag[i-6])
                        qmatrix[i] = (i==6) ? quant8_intra_default:quant8_inter_default;
                    else
                        qmatrix[i] = pps->ScalingList8x8[i-6];
                }
            }
        }
    }
}

void WriteHWData(void)
{
    Slice *currSlice = img->currentSlice;
    SliceParameters *ptSliceParam;
    MMP_UINT8*  pCmdBuf;
    MMP_UINT32  i1, i2, i3, num;
    MMP_UINT32  value, data;
    MMP_UINT32  index;
    MMP_UINT32  size, stuffbytes;
    MMP_INT32  sliceQP, pstate;
    MMP_UINT8*  pBufferStart = gptAVCVideoDecoder->pBufferStartAddr;
    MMP_UINT8*  pBufferEnd = gptAVCVideoDecoder->pBufferEndAddr;

    ptSliceParam = (SliceParameters *)gAVCCmdBuf;
    pCmdBuf = (MMP_UINT8*) gAVCCmdBuf;

    //Slice header
    ptSliceParam->frameWidthMB      = img->PicWidthInMbs;
    ptSliceParam->frameHeightMB     = img->FrameHeightInMbs;
    ptSliceParam->lumaPitch8_l      = (gptAVCDecoder->framePitchY % 2048) / 8; // Now Pitch is 2048 for tiling mode
    ptSliceParam->lumaPitch8_h      = gptAVCDecoder->framePitchY / 2048;
    ptSliceParam->chromaPitch8_l    = (gptAVCDecoder->framePitchY % 2048) / 8;
    ptSliceParam->chromaPitch8_h    = gptAVCDecoder->framePitchY / 2048;

    ptSliceParam->videoFormat = 0; // H.264
    if (img->MbaffFrameFlag == 1)
        ptSliceParam->pictureStructure = 3;
    else
        ptSliceParam->pictureStructure = (int)img->structure;

    ptSliceParam->direct_8x8_inference_flag = active_sps->direct_8x8_inference_flag;
    ptSliceParam->direct_8x8_inference_flag |= (!!img->nal_reference_idc) << 1;
    if (active_sps->chroma_format_idc == YUV400)
        ptSliceParam->chroma_format_idc = 0;
    else
        ptSliceParam->chroma_format_idc = 1;

    ptSliceParam->entropy_coding_mode_flag = active_pps->entropy_coding_mode_flag;

    ptSliceParam->weighted_pred_mode = 0; // P/B explicit, set to be default
    if (img->type == B_SLICE)
        if (active_pps->weighted_bipred_idc == 2)
            ptSliceParam->weighted_pred_mode = 1; // B implicit

    //if (ptSliceParam->weighted_pred_mode == 1)
    //    printf("ptSliceParam->weighted_pred_mode = 1\n");

    ptSliceParam->pic_init_qp_minus26 = active_pps->pic_init_qp_minus26;
    ptSliceParam->chroma_qp_index_offset = active_pps->chroma_qp_index_offset;
    ptSliceParam->second_chroma_qp_index_offset = active_pps->second_chroma_qp_index_offset;
    ptSliceParam->deblocking_filter_control_present_flag = active_pps->deblocking_filter_control_present_flag;

    ptSliceParam->constrained_intra_pred_flag = active_pps->constrained_intra_pred_flag;
    ptSliceParam->transform_8x8_mode_flag = img->AllowTransform8x8;
    if (currSlice->picture_type == 2)
        ptSliceParam->sliceType = 0;  // I, from slice header
    else if (currSlice->picture_type == 0)
        ptSliceParam->sliceType = 1;  // P, from slice header
    else if (currSlice->picture_type == 1)
        ptSliceParam->sliceType = 2;  // B, from slice header

    ptSliceParam->direct_spatial_mv_pred_flag = img->direct_spatial_mv_pred_flag;
    ptSliceParam->num_ref_idx_l0_active_minus1 = img->num_ref_idx_l0_active - 1;
    ptSliceParam->num_ref_idx_l1_active_minus1 = img->num_ref_idx_l1_active - 1;

    ptSliceParam->luma_log2_weight_denom = gSliceParameters.luma_log2_weight_denom;
    ptSliceParam->chroma_log2_weight_denom = gSliceParameters.chroma_log2_weight_denom;

    ptSliceParam->colPicIsLongterm = 0;
    ptSliceParam->colDataPlacement = gSliceParameters.col_data_placement;
    ptSliceParam->colDataUseWhichChunk = gSliceParameters.col_data_use_which_chunk;
    ptSliceParam->first_MB_in_slice_X = currSlice->start_mb_nr % img->PicWidthInMbs;
    ptSliceParam->first_MB_in_slice_Y = currSlice->start_mb_nr / img->PicWidthInMbs;

    ptSliceParam->decodeBufferSelector = dec_picture->decoding_buf_idx;
    for (i1 = 0; i1 < 32; i1++)
    {
        ptSliceParam->remapping_ref_idx_l0[i1] = gSliceParameters.remapping_ref_idx_l0[i1];
        ptSliceParam->remapping_ref_idx_l1[i1] = gSliceParameters.remapping_ref_idx_l1[i1];
    }

    // temp solution
    ptSliceParam->last_MB_in_slice_X = img->PicWidthInMbs - 1; //gSliceParameters.last_MB_in_slice_X;
    if (img->structure == TOP_FIELD || img->structure == BOTTOM_FIELD)
        ptSliceParam->last_MB_in_slice_Y = (img->FrameHeightInMbs>>1) - 1; //gSliceParameters.last_MB_in_slice_Y;
    else
        ptSliceParam->last_MB_in_slice_Y = img->FrameHeightInMbs - 1; //gSliceParameters.last_MB_in_slice_Y;

    //printf("MB slice Y %d\n", ptSliceParam->last_MB_in_slice_Y);

    ptSliceParam->discardBits = gSliceParameters.dwStartBits;
    ptSliceParam->lastBytePosition = 0;

    ptSliceParam->frame_poc_l = gSliceParameters.poc & 0x0FF;
    ptSliceParam->frame_poc_h = (gSliceParameters.poc & 0x0FF00) >> 8;
    ptSliceParam->top_poc_l = gSliceParameters.top_poc & 0x0FF;
    ptSliceParam->top_poc_h = (gSliceParameters.top_poc & 0x0FF00) >> 8;
    ptSliceParam->bottom_poc_l = gSliceParameters.bottom_poc & 0x0FF;
    ptSliceParam->bottom_poc_h = (gSliceParameters.bottom_poc & 0x0FF00) >> 8;

#ifdef MULTI_CHANNEL
    ptSliceParam->directWriteColDataBufferBase8_l = (gptAVCDecoder->colDataBufAdr[0] & 0xFF);
    ptSliceParam->directWriteColDataBufferBase8_m = (gptAVCDecoder->colDataBufAdr[0] & 0xFF00) >> 8;
    ptSliceParam->directWriteColDataBufferBase8_h = (gptAVCDecoder->colDataBufAdr[0] & 0xFF0000) >> 16;

    if (currSlice->picture_type == 1)
    {
        ptSliceParam->directReadColDataBufferBase8_l = (gptAVCDecoder->colDataBufAdr[0] & 0xFF);
        ptSliceParam->directReadColDataBufferBase8_m = (gptAVCDecoder->colDataBufAdr[0] & 0xFF00) >> 8;
        ptSliceParam->directReadColDataBufferBase8_h = (gptAVCDecoder->colDataBufAdr[0] & 0xFF0000) >> 16;
    }
#else
    ptSliceParam->directWriteColDataBufferBase8_l = (gptAVCDecoder->colDataBufAdr[dec_picture->col_data_buf_idx] & 0xFF);
    ptSliceParam->directWriteColDataBufferBase8_m = (gptAVCDecoder->colDataBufAdr[dec_picture->col_data_buf_idx] & 0xFF00) >> 8;
    ptSliceParam->directWriteColDataBufferBase8_h = (gptAVCDecoder->colDataBufAdr[dec_picture->col_data_buf_idx] & 0xFF0000) >> 16;

    if (currSlice->picture_type == 1)
    {
        ptSliceParam->directReadColDataBufferBase8_l = (gptAVCDecoder->colDataBufAdr[listX[1][0]->col_data_buf_idx] & 0xFF);
        ptSliceParam->directReadColDataBufferBase8_m = (gptAVCDecoder->colDataBufAdr[listX[1][0]->col_data_buf_idx] & 0xFF00) >> 8;
        ptSliceParam->directReadColDataBufferBase8_h = (gptAVCDecoder->colDataBufAdr[listX[1][0]->col_data_buf_idx] & 0xFF0000) >> 16;
    }
#endif

    //printf("Col data %d %d %d %d\n", dec_picture->col_data_buf_idx, listX[1][0]->col_data_buf_idx, gSliceParameters.col_data_placement, gSliceParameters.col_data_use_which_chunk);

    ptSliceParam->nextSliceHeaderDataBase8_l = 0;              // AM
    ptSliceParam->nextSliceHeaderDataBase8_m = 0;              // AM
    ptSliceParam->nextSliceHeaderDataBase8_h = 0;              // AM

    ptSliceParam->tableInsertFlag = 63;

    //if (img->structure == TOP_FIELD || img->structure == BOTTOM_FIELD)
    //for (i1 = 0; i1 < 120; i1+=2)
    //{
    //    printf("%x %x\n", *(pCmdBuf+i1), *(pCmdBuf+i1+1));
    //}
    //printf(" D_B %d P_I %d \n", dec_picture->decoding_buf_idx, dec_picture->pic_idx);

    if (gptAVCDecoder->cmdDataBufSize == 0)
    {
#ifdef MULTI_CHANNEL
#else
        // Table 1
        index = 0;
        for (i1 = 0; i1 < 32; i1++)
        {
            for (i2 = 0; i2 < 2; i2++)
            {
                *(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = gSliceParameters.ref_idx_to_pic_idx_mapping[i2][i1] & 0x03F;
            }

            *(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = gRefIdxToPocListIdx[0][i1];
            *(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = gRefIdxToPocListIdx[1][i1];
            *(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = gPocList[i1] & 0x0FF;
            *(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = gPocList[i1] >> 8;
            for (i2 = 0; i2 < 2; i2++)
            {
                *(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = 0;
            }
        }

        // Table 2
        index = 0;
        for (i1 = 0; i1 < 64; i1++)
        {
            for (i2 = 0; i2 < 3; i2++)
            {
                *(pCmdBuf + PICIDX_TO_DIST_TABLE_START + index++) =
                    gSliceParameters.col_pic_idx_to_ref_idx_mapping[i2][i1] & 0x1F;
            }
            for (i2 = 0; i2 < 5; i2++)
            {
                *(pCmdBuf + PICIDX_TO_DIST_TABLE_START + index++) = 0;
            }
        }
        for (i1 = 0; i1 < 3; i1++)
        {
            for (i2 = 0; i2 < 32; i2++)
            {
                data = gSliceParameters.col_ref_idx_to_dist_scale_mapping[i1][i2] & 0x07FF;
                *(pCmdBuf + PICIDX_TO_DIST_TABLE_START + index++) = data & 0x0FF;
                *(pCmdBuf + PICIDX_TO_DIST_TABLE_START + index++) = data >> 8;

                for (i3 = 0; i3 < 6; i3++)
                {
                    *(pCmdBuf + PICIDX_TO_DIST_TABLE_START + index++) = 0;
                }
            }
        }
#endif
        // Table 3
        index = 0;
        for (i1 = 0; i1 < 8; i1++)
        {
            if (i1 < 6)
            {
                num = 16;
            } else {
                num = 64;
            }
            for (i2 = 0; i2 < num; i2 += 2)
            {
                *(pCmdBuf + SCALING_TABLE_START + index++) = (MMP_UINT8)qmatrix[i1][i2];
                *(pCmdBuf + SCALING_TABLE_START + index++) = (MMP_UINT8)qmatrix[i1][i2 + 1];
                for (i3 = 0; i3 < 6; i3++) {
                    *(pCmdBuf + SCALING_TABLE_START + index++) = 0;
                }
            }
        }

        // Table 4
        index = 0;
        for (i1 = 0; i1 < 192; i1++)
        {
            for (i2 = 0; i2 < 3; i2++)
            {
                *(pCmdBuf + WEIGHTING_TABLE_START + index++) = (MMP_UINT8)weighted_pred_RF[i1][i2];
            }
            for (i2 = 0; i2 < 5; i2++)
            {
                *(pCmdBuf + WEIGHTING_TABLE_START + index++) = 0;
            }
        }
    }

    if (gptAVCDecoder->cmdDataBufSize == 0 || img->qp != img->preQP)
    {
        // Table 5
        sliceQP   = max(0,img->qp);

        if (img->type == I_SLICE)
            i1 = 0;
        else
            i1 = (1+ img->model_number)*2;
        index = 0;

        for (i2 = 0; i2 < 512; i2++) {
            if (cabac_output_map[i2])
            {
                //calculate
                if (i1 < 3680)
                    pstate = ((cabac_init_vars[i1]* sliceQP )>>4) + cabac_init_vars[i1+1];
                i1+=8;
                pstate = min (max ( 1, pstate), 126);

                if ( pstate >= 64 )
                    value = ((pstate - 64) & 0x03F) | 0x40;
                else
                    value = (63 - pstate) & 0x03F;
            }
            else
            {
                value = 0;
            }

            *(pCmdBuf + CABAC_TABLE_START + index++) = value;
        }
    }

    img->preQP = img->qp;

    gptAVCDecoder->cmdDataBufSize += BITSTREAM_START;

    // write slice data
    pCmdBuf = (MMP_UINT8*) ((MMP_UINT32)pCmdBuf+BITSTREAM_START);

    if ((MMP_UINT32) gSliceParameters.pDataEnd >= (MMP_UINT32) gSliceParameters.pDataStart)
    {
        memcpy(pCmdBuf, gSliceParameters.pDataStart, gSliceParameters.dwDataSize);

        stuffbytes = 8 - (gSliceParameters.dwDataSize&0x7);

        for (i2=0; i2<stuffbytes; i2++)
            *(pCmdBuf+gSliceParameters.dwDataSize+i2) = 0x00;

    } else {
        //printf("Wrap %x %x\n", pBufferEnd, gSliceParameters.pDataStart);
        size = (MMP_UINT32)pBufferEnd - (MMP_UINT32)gSliceParameters.pDataStart;

        memcpy(pCmdBuf, gSliceParameters.pDataStart, size);
        pCmdBuf = (MMP_UINT8*) ((MMP_UINT32)pCmdBuf+size);
        size = gSliceParameters.dwDataSize - size;
        memcpy(pCmdBuf, pBufferStart, size);

        stuffbytes = 8 - (gSliceParameters.dwDataSize&0x7);

        for (i2=0; i2<stuffbytes; i2++)
            *(pCmdBuf+gSliceParameters.dwDataSize+i2) = 0x00;
    }

    gptAVCDecoder->cmdDataBufSize += (gSliceParameters.dwDataSize + stuffbytes);

    //printf("[%s] BITSTREAM_START = %d, cmdDataBufSize = %d\n", __FUNCTION__, BITSTREAM_START, gptAVCDecoder->cmdDataBufSize);

#if defined(CFG_CPU_WB) && (!defined(WIN32))
    ithFlushDCacheRange(gAVCCmdBuf, gptAVCDecoder->cmdDataBufSize);
    ithFlushMemBuffer();
#elif defined(WIN32)
    ithFlushDCacheRange(gAVCCmdBuf, gptAVCDecoder->cmdDataBufSize);
#endif
#if 0
    if (1 && gptAVCDecoder->cmdDataBufSize > 0)
    {
        FILE* f;
        MMP_CHAR filename[255];
        static MMP_UINT i = 0;

        sprintf(filename, "C:/%05d.bin", i++);
        printf("%s(%d) - (%s)\n", __FUNCTION__, __LINE__, filename);
        f = fopen(filename, "wb");
        fwrite(gAVCCmdBuf, 1, gptAVCDecoder->cmdDataBufSize, f);
        fclose(f);
    }
#endif
    //printf("Ptr %x  : %x %x %x %x %x %x %x %x %d", pCmdBuf, *(pCmdBuf), *(pCmdBuf+1), *(pCmdBuf+2), *(pCmdBuf+3),
    //              *(pCmdBuf+4), *(pCmdBuf+5), *(pCmdBuf+6), *(pCmdBuf+7), gSliceParameters.dwDataSize);
    //printf("          %x %x %x %x %x %x %x %x\n", *(pCmdBuf+8), *(pCmdBuf+9), *(pCmdBuf+10), *(pCmdBuf+11),
    //              *(pCmdBuf+12), *(pCmdBuf+13), *(pCmdBuf+14), *(pCmdBuf+15));
    //printf("Start bits %d %x %x %x\n", ptSliceParam->discardBits, *(pCmdBuf+gSliceParameters.dwDataSize-1), *(pCmdBuf+gSliceParameters.dwDataSize), *(pCmdBuf+gSliceParameters.dwDataSize+1));
}

//=============================================================================
//                              Public Function Definition
//=============================================================================
void
avc_Nal_Get_Header(AVC_NAL_HEADER *pNalHeader, MMP_UINT8 *pNalUnit)
{
    pNalHeader->forbidden_zero_bit = (*(pNalUnit) & 0x80) >>7;
    pNalHeader->nal_ref_idc        = (*(pNalUnit) & 0x60) >>5;
    pNalHeader->nal_unit_type      = *(pNalUnit) & 0x1F;
}

AVC_ERROR_CODE
avc_Seq_Parameter(MMP_UINT8 *pNalUnit,
                  MMP_UINT32 numberBytesInNALunit)
{
    AVC_ERROR_CODE ret = AVC_ERROR_SUCCESS;
    AVC_SEQUENCE_PARAMETER_SET tSeqParamSet = {0};
    EMULATION_PREVENTION_BYTE PreventionByte = {0}; // record which bytes are prevention bytes and skip them when doing decode
    MMP_UINT32 RBSPsize = 0;

    nal_unit_to_RBSP(&PreventionByte, pNalUnit, &RBSPsize, numberBytesInNALunit);

    ret = nal_get_seq_parameter_set(&tSeqParamSet, RBSPsize);

    if (AVC_ERROR_SUCCESS != ret)
    {
        return ret;
    }

    //ret = avc_InsertSequenceParameterSet(&tSeqParamSet);
    //
    //if (AVC_ERROR_SUCCESS != ret)
    //{
    //    return ret;
    //}

    if (active_sps)
    {
        if (tSeqParamSet.seq_parameter_set_id == active_sps->seq_parameter_set_id)
        {
            if (!sps_is_equal(&tSeqParamSet, active_sps))
            {
                if (dec_picture)
                {
                    // this may only happen on slice loss
                    exit_picture();
                }
                active_sps=NULL;
            }
        }
    }

    ret = avc_InsertSequenceParameterSet(&tSeqParamSet);

    return ret;
}

void interpret_picture_timing_info(MMP_UINT8* payload, MMP_UINT32 size)
{
    MMP_UINT32 picture_structure_present_flag, picture_structure;
    MMP_UINT32 cpb_removal_len = 24;
    MMP_UINT32 dpb_output_len  = 24;
    MMP_BOOL   CpbDpbDelaysPresentFlag;
    MMP_UINT32 tmp;
    MMP_UINT32 usedBits = 0;

    gtAVCBitStream.remainByte = size;
    BitStreamKit_Init(&gtAVCBitStream, payload);

    if (NULL==active_sps)
    {
        //printf ("Warning: no active SPS, timing SEI cannot be parsed\n");
        return;
    }

    // CpbDpbDelaysPresentFlag can also be set "by some means not specified in this Recommendation | International Standard"
    CpbDpbDelaysPresentFlag =  (active_sps->vui_parameters_present_flag
        && ( (active_sps->nal_hrd_parameters_present_flag != 0)
        ||(active_sps->vcl_hrd_parameters_present_flag != 0)));

    if (CpbDpbDelaysPresentFlag)
    {
        if (active_sps->vui_parameters_present_flag)
        {
            if (active_sps->nal_hrd_parameters_present_flag)
            {
                cpb_removal_len = active_sps->nal_cpb_removal_delay_length_minus1 + 1;
                dpb_output_len  = active_sps->nal_dpb_output_delay_length_minus1  + 1;
            }
            else if (active_sps->vcl_hrd_parameters_present_flag)
            {
                cpb_removal_len = active_sps->vcl_cpb_removal_delay_length_minus1 + 1;
                dpb_output_len  = active_sps->vcl_dpb_output_delay_length_minus1  + 1;
            }
        }

        if ((active_sps->nal_hrd_parameters_present_flag)||
            (active_sps->vcl_hrd_parameters_present_flag))
        {
            cpb_removal_len = ue_v(&usedBits);
            dpb_output_len  = ue_v(&usedBits);
        }
    }

    if (!active_sps->vui_parameters_present_flag)
    {
        picture_structure_present_flag = 0;
    }
    else
    {
        picture_structure_present_flag  =  active_sps->pic_struct_present_flag;
    }

    if (picture_structure_present_flag)
    {
        MMP_UINT    i;
        MMP_UINT32  tmp;

        gbSEIPicStrFlg = MMP_TRUE;
        gSEIPicStructure = picture_structure = BitStreamKit_GetBits(&gtAVCBitStream, 4);

        //printf("SEI picture_structure %d\n", picture_structure);
        // not support this case
        //dbg_msg_ex(DBG_MSG_TYPE_ERROR, "This bitstream has some not-supported information !! ");
        return ;

#if 0 // Maybe support this case in the furtur.
        //picture_structure = BitStreamKit_GetBits(&gtAVCBitStream, 4);
        //printf("SEI picture_structure %d\n", picture_structure);
        switch (picture_structure)
        {
        case 0:
        case 1:
        case 2:
            NumClockTs = 1;
            break;
        case 3:
        case 4:
        case 7:
            NumClockTs = 2;
            break;
        case 5:
        case 6:
        case 8:
            NumClockTs = 3;
            break;
        default:
            dbg_msg(DBG_MSG_TYPE_ERROR, "reserved picture_structure used (can't determine NumClockTs)");
            break;
        }

        for (i = 0; i < NumClockTs; i++)
        {
            // clock_time_stamp_flag
            tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
            if( tmp )
            {
                MMP_UINT32    full_timestamp_flag;

                // ct_type
                tmp = BitStreamKit_GetBits(&gtAVCBitStream, 2);
                // nuit_field_based_flag
                tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
                // counting_type
                tmp = BitStreamKit_GetBits(&gtAVCBitStream, 5);
                // full_timestamp_flag
                full_timestamp_flag = BitStreamKit_GetBits(&gtAVCBitStream, 1);
                // discontinuity_flag
                tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
                // cnt_dropped_flag
                tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
                // nframes
                tmp = BitStreamKit_GetBits(&gtAVCBitStream, 8);

                if( full_timestamp_flag )
                {
                    // seconds_value
                    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 6);
                    // minutes_value
                    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 6);
                    // hours_value
                    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 5);
                }
                else
                {
                    MMP_INT     time_offset_length;

                    // seconds_flag
                    tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
                    if( tmp )
                    {
                        // seconds_value
                        tmp = BitStreamKit_GetBits(&gtAVCBitStream, 6);
                        // minutes_flag
                        tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
                        if( tmp )
                        {
                            // minutes_value
                            tmp = BitStreamKit_GetBits(&gtAVCBitStream, 6);
                            // hours_flag
                            tmp = BitStreamKit_GetBits(&gtAVCBitStream, 1);
                            if( tmp )
                            {
                                // hours_value
                                tmp = BitStreamKit_GetBits(&gtAVCBitStream, 5);
                            }
                        }
                    }

                    if (active_sps->vcl_hrd_parameters_present_flag)
                    {
                        // Fix Me
                        // ReadVCLHRDParameters() need to parse time_offset_length
                        // time_offset_length = active_sps->vcl_hrd_parameters.time_offset_length;
                        dbg_ms_ex(DBG_MSG_TYPE_ERROR, "not support vcl_hrd_parameters_present_flag !! ");
                        return;
                    }

                    else if (active_sps->nal_hrd_parameters_present_flag)
                    {
                        // Fix Me
                        // ReadNALHRDParameters() need to parse time_offset_length
                        // time_offset_length = active_sps->vui_seq_parameters.nal_hrd_parameters.time_offset_length;
                        dbg_ms_ex(DBG_MSG_TYPE_ERROR, "not support nal_hrd_parameters_present_flag !! ");
                        return;
                    }

                    else
                    {
                        time_offset_length = 24;
                    }

                    if (time_offset_length)
                    {
                        MMP_INT     time_offset;
                        time_offset = BitStreamKit_GetBits(&gtAVCBitStream, time_offset_length);
                    }
                }
            }
        }
#endif
    }
    //printf("SEI picture_structure %d\n", picture_structure);
}

void interpret_user_data_registered_itu_t_t35_info(MMP_UINT8* payload, MMP_UINT32 size)
{
    MMP_UINT32  user_identifier = 0;
    MMP_UINT32  active_format_flag = 0;
    MMP_UINT32  active_format = 0;

    gtAVCBitStream.remainByte = size;
    BitStreamKit_Init(&gtAVCBitStream, payload);

    user_identifier                 =    BitStreamKit_GetBits(&gtAVCBitStream, 32);

    if (user_identifier == 0x44544731)
    {
        // afd_data();
        /* '0'                      = */ BitStreamKit_SkipBits(&gtAVCBitStream, 1);
        active_format_flag          =    BitStreamKit_GetBits(&gtAVCBitStream, 1);
        /* reserved ('00 0001')     = */ BitStreamKit_SkipBits(&gtAVCBitStream, 6);

        if (active_format_flag==1)
        {
            /* reserved ('1111')    = */ BitStreamKit_SkipBits(&gtAVCBitStream, 4);
            active_format           =    BitStreamKit_GetBits(&gtAVCBitStream, 4);

            gptAVCDecoder->active_format = active_format;
        }
    }
    else if (user_identifier == 0x47413934)
    {
        // DVB1_data();
    }
}

void interpret_random_access_info(MMP_UINT8* payload, MMP_UINT32 size, MMP_INT32 *recovery_frame_cnt )
{
  MMP_UINT32 tmp;
  MMP_UINT32 usedBits = 0;

  gtAVCBitStream.remainByte = size;
  BitStreamKit_Init(&gtAVCBitStream, payload);

  *recovery_frame_cnt = ue_v(&usedBits);
  tmp  = BitStreamKit_GetBits(&gtAVCBitStream, 1);
  tmp  = BitStreamKit_GetBits(&gtAVCBitStream, 1);
  tmp  = BitStreamKit_GetBits(&gtAVCBitStream, 2);
}

void InterpretSEIMessage(MMP_UINT32 size, MMP_INT32 *recovery_frame_cnt)
{
MMP_UINT32 payload_type = 0;
MMP_UINT32 payload_size = 0;
MMP_UINT32 offset = 0;
MMP_UINT8  tmp_byte;
MMP_UINT8* msg;

    msg = gpRBSP;

    do
    {
        // sei_message();
        payload_type = 0;
        tmp_byte = msg[offset++];
        while (tmp_byte == 0xFF)
        {
            payload_type += 255;
            tmp_byte = msg[offset++];
        }
        payload_type += tmp_byte;   // this is the last byte

        payload_size = 0;
        tmp_byte = msg[offset++];
        while (tmp_byte == 0xFF)
        {
            payload_size += 255;
            tmp_byte = msg[offset++];
        }
        payload_size += tmp_byte;   // this is the last byte

        switch ( payload_type )     // sei_payload( type, size );
        {
            //case  SEI_BUFFERING_PERIOD:
            //  interpret_buffering_period_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_PIC_TIMING:
        case  1:
            interpret_picture_timing_info( msg+offset, payload_size);
            break;
            //case  SEI_PAN_SCAN_RECT:
            //  interpret_pan_scan_rect_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_FILLER_PAYLOAD:
            //  interpret_filler_payload_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_USER_DATA_REGISTERED_ITU_T_T35:
        case  4:
            interpret_user_data_registered_itu_t_t35_info( msg+offset, payload_size );
            break;
            //case  SEI_USER_DATA_UNREGISTERED:
            //  interpret_user_data_unregistered_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_RANDOM_ACCESS_POINT:
        case 6:
            interpret_random_access_info( msg+offset, payload_size, recovery_frame_cnt );
            break;
            //  interpret_random_access_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_DEC_REF_PIC_MARKING_REPETITION:
            //  interpret_dec_ref_pic_marking_repetition_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_SPARE_PIC:
            //  interpret_spare_pic( msg+offset, payload_size, img );
            //  break;
            //case  SEI_SCENE_INFO:
            //  interpret_scene_information( msg+offset, payload_size, img );
            //  break;
            //case  SEI_SUB_SEQ_INFO:
            //  interpret_subsequence_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_SUB_SEQ_LAYER_CHARACTERISTICS:
            //  interpret_subsequence_layer_characteristics_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_SUB_SEQ_CHARACTERISTICS:
            //  interpret_subsequence_characteristics_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_FULL_FRAME_FREEZE:
            //  interpret_full_frame_freeze_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_FULL_FRAME_FREEZE_RELEASE:
            //  interpret_full_frame_freeze_release_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_FULL_FRAME_SNAPSHOT:
            //  interpret_full_frame_snapshot_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_PROGRESSIVE_REFINEMENT_SEGMENT_START:
            //  interpret_progressive_refinement_end_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_PROGRESSIVE_REFINEMENT_SEGMENT_END:
            //  interpret_progressive_refinement_end_info( msg+offset, payload_size, img );
            //  break;
            //case  SEI_MOTION_CONSTRAINED_SLICE_GROUP_SET:
            //  interpret_motion_constrained_slice_group_set_info( msg+offset, payload_size, img );
            //  break;
        default:
            //interpret_reserved_info( msg+offset, payload_size, img );
            break;
        }
        offset += payload_size;

    } while( msg[offset] != 0x80 && offset < size);    // more_rbsp_data()  msg[offset] != 0x80
    // ignore the trailing bits rbsp_trailing_bits();
    //assert(msg[offset] == 0x80);      // this is the trailing bits
    //assert( offset+1 == size );
}

AVC_ERROR_CODE
avc_SEI(MMP_UINT8 *pNalUnit,
        MMP_UINT32 numberBytesInNALunit,
        MMP_BOOL *bRecovery_Point)
{
AVC_ERROR_CODE ret = AVC_ERROR_SUCCESS;
AVC_PICTURE_PARAMETER_SET  tPicParamSet = {0};
EMULATION_PREVENTION_BYTE PreventionByte = {0};
MMP_UINT32 RBSPsize = 0;
MMP_INT32 recovery_frame_cnt = -1;

    nal_unit_to_RBSP(&PreventionByte, pNalUnit, &RBSPsize, numberBytesInNALunit);
    //memcpy((void*)gpRBSP, (void*)pNalUnit, numberBytesInNALunit);
    //RBSPsize = numberBytesInNALunit;
    InterpretSEIMessage(RBSPsize, &recovery_frame_cnt);

    *bRecovery_Point = (recovery_frame_cnt >= 0);

    return ret;
}

AVC_ERROR_CODE
avc_Pic_Parameter(MMP_UINT8 *pNalUnit,
                  MMP_UINT32 numberBytesInNALunit)
{
    AVC_ERROR_CODE ret = AVC_ERROR_SUCCESS;
    AVC_PICTURE_PARAMETER_SET  tPicParamSet = {0};
    EMULATION_PREVENTION_BYTE PreventionByte = {0};
    MMP_UINT32 RBSPsize = 0;

    nal_unit_to_RBSP(&PreventionByte, pNalUnit, &RBSPsize, numberBytesInNALunit);

    ret = nal_get_pic_parameter_set(&tPicParamSet, RBSPsize);

    if (AVC_ERROR_SUCCESS != ret)
    {
        return ret;
    }

    //ret = avc_InsertPictureParameterSet(&tPicParamSet);
    //
    //if (AVC_ERROR_SUCCESS != ret)
    //{
    //    return ret;
    //}

    if (active_pps)
    {
        if (tPicParamSet.pic_parameter_set_id == active_pps->pic_parameter_set_id)
        {
            if (!pps_is_equal(&tPicParamSet, active_pps))
            {
                if (dec_picture)
                {
                    // this may only happen on slice loss
                    exit_picture();
                }
                active_pps = NULL;
            }
        }
    }

    ret = avc_InsertPictureParameterSet(&tPicParamSet);

    return ret;
}

int get_mem2Dint(int ***array2D, int rows, int columns)
{
    int i;

    if ((*array2D      = (int**)calloc(rows,        sizeof(int*))) == NULL)
        printf("get_mem2Dint fail\n");
    if (((*array2D)[0] = (int* )calloc(rows*columns,sizeof(int ))) == NULL)
        printf("get_mem2Dint fail\n");

    for(i=1 ; i<rows ; i++)
        (*array2D)[i] =  (*array2D)[i-1] + columns  ;

    return rows*columns*sizeof(int);
}

int get_mem3Dint(int ****array3D, int frames, int rows, int columns)
{
    int  j;

    if (((*array3D) = (int***)calloc(frames,sizeof(int**))) == NULL)
        printf("get_mem3Dint fail\n");

    for(j=0;j<frames;j++)
        get_mem2Dint( (*array3D)+j, rows, columns ) ;

    return frames*rows*columns*sizeof(int);
}

void free_mem2D(int ***array2D)
{        
    if (*array2D)
    {
        if ((*array2D)[0])
            free((*array2D)[0]);   
        else
        	  printf("free2D_0 error\n"); 
        	  
        if (*array2D)
            free(*array2D);
    }
    else
    	  printf("free2D_1 error\n");    	     
}

void free_mem3D(int ****array3D, int frames)
{
    int  j;   
    
    if (*array3D)
    {
        for(j=0;j<frames;j++)
            free_mem2D((*array3D)+j);
        
        if (*array3D)
            free(*array3D);
    } else
    	  printf("free3D error\n");
}

void
avc_ReleaseAllResource(void)
{
	if (gparamSetList)
	{
		free(gparamSetList);
		gparamSetList = NULL;
	}

	if (gpRBSP)
	{
		free(gpRBSP);
		gpRBSP = NULL;
	}

	if (img)
	{
		avc_frame_end();

		if (img->currentSlice)
		{
			free(img->currentSlice);
			img->currentSlice = NULL;
		}		
        
		//free_mem3D(&(img->wp_weight), 2);
		//free_mem3D(&(img->wp_offset), 6);
    
		free(img);
		img = NULL;
	}
}

void
avc_CreateParameterSetList(void)
{
    MMP_UINT32 i,j;

    if (MMP_NULL == gparamSetList)
        gparamSetList = (PARAMT_SET_LIS*)malloc(sizeof(PARAMT_SET_LIS));

    for (i=0; i<NUM_SEQ_PARAM_SET; i++)
    {
        gparamSetList->SeqParamSetList.pParamSet[i]     = MMP_NULL;
        gparamSetList->SeqParamSetList.activeParamSetId = DUMMY_SEQ_PARAM_SET_ID;
    }

    // Picture Parameter Set
    for(i=0; i<NUM_PIC_PARAM_SET; i++)
    {
        gparamSetList->PicParamSetList.pParamSet[i]     = MMP_NULL;
        gparamSetList->PicParamSetList.activeParamSetId = DUMMY_PIC_PARAM_SET_ID;
    }

    if (MMP_NULL == gpRBSP)
        gpRBSP = (MMP_UINT8*)malloc(MAX_RBSP_SIZE);

    gtAVCBitStream.pBufferStartAddr = gpRBSP;
    gtAVCBitStream.pBufferEndAddr = gpRBSP+MAX_RBSP_SIZE;

    BitStreamKit_Init(&gtAVCBitStream, gpRBSP);

    // tmp solution
    if (MMP_NULL == img)
    {
        if ((img   =  (struct img_par *)malloc(sizeof(struct img_par)))==MMP_NULL)
        {
            av_log(NULL, AV_LOG_ERROR, "Fail Allocate img structure\n");
        }

        memset((void*)img, 0x00, sizeof(struct img_par));
        img->currentSlice = (Slice *) malloc(sizeof(Slice));
        memset((void*)img->currentSlice, 0x00, sizeof(Slice));

        if (img->currentSlice == MMP_NULL)
        {
            av_log(NULL, AV_LOG_ERROR, "Fail Allocate currentSlice structure\n");
        }
        //get_mem3Dint(&(img->wp_weight), 2, MAX_REFERENCE_PICTURES, 3);
        //get_mem3Dint(&(img->wp_offset), 6, MAX_REFERENCE_PICTURES, 3);
    }

    if (MMP_NULL == dpb.fs)
    {
        dpb.size = gptAVCDecoder->frameBufCount;

        dpb.fs = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
        if (NULL==dpb.fs)
            av_log(NULL, AV_LOG_ERROR, "init_dpb: dpb->fs fail \n");

        dpb.fs_ref = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
        if (NULL==dpb.fs_ref)
            av_log(NULL, AV_LOG_ERROR, "init_dpb: dpb->fs_ref fail \n");

        dpb.fs_ltref = (FrameStore **)calloc(dpb.size, sizeof (FrameStore*));
        if (NULL==dpb.fs_ltref)
            av_log(NULL, AV_LOG_ERROR, "init_dpb: dpb->fs_ltref");

        for (i=0; i<dpb.size; i++)
        {
            dpb.fs[i]       = alloc_frame_store();
            dpb.fs_ref[i]   = NULL;
            dpb.fs_ltref[i] = NULL;
        }

        for (i=0; i<6; i++)
        {
            listX[i] = (StorablePicture **)calloc(MAX_LIST_SIZE, sizeof (StorablePicture*)); // +1 for reordering
            if (NULL==listX[i])
                av_log(NULL, AV_LOG_ERROR, "init_dpb: listX[%d] fail\n", i);
        }

        for (j=0;j<6;j++)
        {
            for (i=0; i<MAX_LIST_SIZE; i++)
            {
                listX[j][i] = NULL;
            }
            listXsize[j]=0;
        }
    }
}

void
avc_ReleaseParameterSetList(void)
{
    MMP_UINT32 i = 0;

    if (MMP_NULL == gparamSetList)
        return;

    for(i=0; i<NUM_SEQ_PARAM_SET; i++)
    {
        if(MMP_NULL != gparamSetList->SeqParamSetList.pParamSet[i])
        {
            free((void*)gparamSetList->SeqParamSetList.pParamSet[i]);
            gparamSetList->SeqParamSetList.pParamSet[i] = MMP_NULL;
        }
    }

    for(i=0; i<NUM_PIC_PARAM_SET; i++)
    {
        if(MMP_NULL != gparamSetList->PicParamSetList.pParamSet[i])
        {
            free((void*)gparamSetList->PicParamSetList.pParamSet[i]);
            gparamSetList->PicParamSetList.pParamSet[i] = MMP_NULL;
        }
    }

    free_dpb();
    img->dec_ref_pic_marking_buffer = MMP_NULL;

    active_sps = MMP_NULL;
    active_pps = MMP_NULL;

    gbCheckFrmGap = MMP_FALSE;
    gbSEIPicStrFlg = MMP_FALSE;

    for (i=0; i < 32; i++)
        PicIdxQueue[i] = 0;

    for (i=0; i < 8; i++)
        DecodingQueue[i] = 0;
}

void
avc_ResetParameterSetList(void)
{
    MMP_UINT32 i = 0;

    if (MMP_NULL == gparamSetList)
        return;

    for(i=0; i<NUM_SEQ_PARAM_SET; i++)
    {
        if(MMP_NULL != gparamSetList->SeqParamSetList.pParamSet[i])
        {
            free((void*)gparamSetList->SeqParamSetList.pParamSet[i]);
            gparamSetList->SeqParamSetList.pParamSet[i] = MMP_NULL;
        }
    }

    for(i=0; i<NUM_PIC_PARAM_SET; i++)
    {
        if(MMP_NULL != gparamSetList->PicParamSetList.pParamSet[i])
        {
            free((void*)gparamSetList->PicParamSetList.pParamSet[i]);
            gparamSetList->PicParamSetList.pParamSet[i] = MMP_NULL;
        }
    }

    free_dpb_frame_store();
    img->dec_ref_pic_marking_buffer = MMP_NULL;

    active_sps = MMP_NULL;
    active_pps = MMP_NULL;

    gbCheckFrmGap = MMP_FALSE;
    gbSEIPicStrFlg = MMP_FALSE;

    for (i=0; i < 32; i++)
        PicIdxQueue[i] = 0;

    for (i=0; i < 8; i++)
        DecodingQueue[i] = 0;
}

void
avc_ResetFrmGapCheck(void)
{
    gbCheckFrmGap = MMP_FALSE;
}

void
avc_ResetBuffer(void)
{
MMP_UINT32 i = 0;

    for (i=0; i < 32; i++)
        PicIdxQueue[i] = 0;

    for (i=0; i < 8; i++)
        DecodingQueue[i] = 0;

    flush_dpb();

    dpb.used_size = 0;
    dpb.last_picture = NULL;

    dpb.ref_frames_in_buffer = 0;
    dpb.ltref_frames_in_buffer = 0;

    if (dec_picture)
    {
        free_marking_buffer();
        free_storable_picture(dec_picture);
    }

    dec_picture = NULL;
    img->structure = FRAME;
    gbLastPic = MMP_TRUE;
    gDePicCount = 0;
}

void
avc_Get_First_MB_In_Slice(MMP_UINT32 *pFirstMBinSlice,
                          MMP_UINT8 *pNalUnit,
                          MMP_UINT32 numberBytesInNALunit)
{
    MMP_UINT32  usedBits = 0;

#if 0 // modified by wlHsu
    BitStreamKit_Init(&gtAVCBitStream, pNalUnit);
    *pFirstMBinSlice = ue_v(&usedBits);
    //printf("\tFirstMBinSlice = %d\n", *pFirstMBinSlice);
#else
    MMP_UINT32  size;
    MMP_UINT8*  pBufferStart = gptAVCVideoDecoder->pBufferStartAddr;
    MMP_UINT8*  pBufferEnd = gptAVCVideoDecoder->pBufferEndAddr;

    size = (MMP_UINT32) pBufferEnd - (MMP_UINT32) pNalUnit;
    if (size >= 8)
    {
        memcpy((void*)gpRBSP, (void*)pNalUnit, 8);
    } else {
        memcpy((void*)gpRBSP, (void*)pNalUnit, size);
        memcpy((void*)(gpRBSP+size), (void*)pBufferStart, 8-size);
    }

    BitStreamKit_Init(&gtAVCBitStream, gpRBSP);
    *pFirstMBinSlice = ue_v(&usedBits);
#endif
}

AVC_ERROR_CODE
avc_slice(AVC_NAL_HEADER *pNalHeader,
          MMP_UINT8 *pNalUnit,
          MMP_UINT32 numberBytesInNALunit)
{
AVC_ERROR_CODE ret = AVC_ERROR_SUCCESS;
MMP_UINT8*  pBufferStart = gptAVCVideoDecoder->pBufferStartAddr;
MMP_UINT8*  pBufferEnd = gptAVCVideoDecoder->pBufferEndAddr;
EMULATION_PREVENTION_BYTE PreventionByte = {0};
MMP_UINT32 workingBufSize = 0;
MMP_UINT32 RBSPsize = 0;
MMP_UINT32 position_slice_qp_delta = 0;
MMP_UINT32 startShift = 0;
MMP_UINT32 shiftByte = 0;
MMP_UINT32 i = 0;

    img->idr_flag = (pNalHeader->nal_unit_type == NALU_TYPE_IDR);
    img->nal_reference_idc = pNalHeader->nal_ref_idc;
    img->disposable_flag = (pNalHeader->nal_ref_idc == NALU_PRIORITY_DISPOSABLE);

    workingBufSize = min(MAX_SLICE_HEADER_SIZE, numberBytesInNALunit);

    PreventionByte.number = 0;
    nal_unit_to_RBSP(&PreventionByte, pNalUnit, &RBSPsize, workingBufSize);

    if ((ret = read_new_slice(RBSPsize)) != AVC_ERROR_SUCCESS)
    {
        free_ref_pic_list_reordering_buffer(img->currentSlice);
        free_marking_buffer();
        return ret;
    }

    if( active_sps->num_ref_frames > MAX_REFERENCE_FRAM_NUM )
    {
        av_log(NULL, AV_LOG_ERROR, "ERROR_OutT_Support_ref_range (ref = %d) !!\n", active_sps->num_ref_frames);
        free_ref_pic_list_reordering_buffer(img->currentSlice);
        free_marking_buffer();
        return ERROR_OUT_SUPPORT_REF_RANGE;
    }

    if (img->PicWidthInMbs > MAX_FRAME_WIDTH_IN_MBs ||
        img->FrameHeightInMbs > MAX_FRAME_HEIGHTIN_MBs ||
        img->PicWidthInMbs < MIN_FRAME_WIDTH_IN_MBs ||
        img->FrameHeightInMbs < MIN_FRAME_HEIGHTIN_MBs)
    {
        av_log(NULL, AV_LOG_ERROR, "ERROR_RESOLUTION !!");
        free_ref_pic_list_reordering_buffer(img->currentSlice);
        free_marking_buffer();
        return ERROR_RESOLUTION;
    }

    // print frame type
    if (img->type == I_SLICE)
        av_log(NULL, AV_LOG_DEBUG, "I SLICE\n");
    else if (img->type == P_SLICE)
        av_log(NULL, AV_LOG_DEBUG, "P SLICE\n");
    else if (img->type == B_SLICE)
        av_log(NULL, AV_LOG_DEBUG, "B SLICE\n");
    else
        av_log(NULL, AV_LOG_ERROR, "Unknow SLICE\n");

    // think more correct solution
    if (img->type == B_SLICE && dpb.ref_frames_in_buffer < 2 && !gbCheckFrmGap)// && img->nal_reference_idc == 0)
    {
        av_log(NULL, AV_LOG_ERROR, "ERROR_SKIP_B_FRAME !!");
        free_ref_pic_list_reordering_buffer(img->currentSlice);
        free_marking_buffer();
        return ERROR_SKIP_B_FRAME;
    }

    // temp solution
    if (img->type == B_SLICE)
        gbCheckFrmGap = MMP_TRUE;

    //if (gbCheckFrmGap)
    //    if (img->frame_num != img->pre_frame_num && img->frame_num != (img->pre_frame_num + 1) % img->MaxFrameNum)
    //    {
    //      dbg_msg_ex(DBG_MSG_TYPE_ERROR, "ERROR_TEMP_USE\n");
    //        return ERROR_TEMP_USE;
    //    }

    position_slice_qp_delta = (MMP_UINT32)gSliceParameters.pDataStart - (MMP_UINT32)gpRBSP;

    startShift = 0;

    if (PreventionByte.number > 0)
    {
        //printf("PreventionByte.number %d %d %d\n", PreventionByte.number, PreventionByte.position[startShift], position_slice_qp_delta);
        while((PreventionByte.position[startShift] <= position_slice_qp_delta) && (startShift < PreventionByte.number))
        {
            startShift++;
        }
    }

    gSliceParameters.pDataStart = (MMP_UINT8*)pNalUnit + position_slice_qp_delta + startShift;
    if (gSliceParameters.pDataStart >= pBufferEnd)
        gSliceParameters.pDataStart = (MMP_UINT8*)pBufferStart + (gSliceParameters.pDataStart - pBufferEnd);

    gSliceParameters.pDataEnd   = (MMP_UINT8*)pNalUnit + numberBytesInNALunit - 1;
    if (gSliceParameters.pDataEnd >= pBufferEnd)
        gSliceParameters.pDataEnd   = (MMP_UINT8*)pBufferStart + (gSliceParameters.pDataEnd - pBufferEnd);

    gSliceParameters.dwDataSize = numberBytesInNALunit - (position_slice_qp_delta + startShift);

    if (((gptAVCDecoder->cmdDataBufSize + gSliceParameters.dwDataSize +
        BITSTREAM_START) > gptAVCDecoder->cmdDataBufMaxSize) || (gSliceParameters.dwDataSize == 0))
    {
        av_log(NULL, AV_LOG_ERROR, "ERROR_BITSTREAM_ERROR\n");
        free_ref_pic_list_reordering_buffer(img->currentSlice);
        free_marking_buffer();
        return ERROR_BITSTREAM_ERROR;
    }

    shiftByte = ((MMP_UINT32)(gSliceParameters.pDataStart) & 0x3);
    if (shiftByte)
    {
        for (i = 1; i <= shiftByte; i++)
        {
            MMP_INT32 diff;

            diff = (gSliceParameters.pDataStart-i) - pBufferStart;

            if (diff < 0)
                *(pBufferEnd+diff) = 0xFF;
            else
                *(gSliceParameters.pDataStart-i) = 0xFF;
        }
        gSliceParameters.pDataStart  -= shiftByte;
        gSliceParameters.dwStartBits += (shiftByte * 8);
        gSliceParameters.dwDataSize  += shiftByte;
    }

    AssignQuantParam(active_pps, active_sps);

    if (gbFrameStart)
    {
        //init_picture();
        if ((ret = init_picture()) != AVC_ERROR_SUCCESS)
        {
            free_ref_pic_list_reordering_buffer(img->currentSlice);
            free_marking_buffer();
            free_storable_picture(dec_picture);
            dec_picture = MMP_NULL;
            dpb.last_picture = MMP_NULL;
            return ret;
        }

        gAVCCmdBuf = gOrgCmdBuf = (MMP_UINT32) gptAVCDecoder->ppCmdDataBufAddr[gptAVCDecoder->cmdDataBufWrSel];
        av_log(NULL, AV_LOG_DEBUG, "Init Decoding Buf %d POC %d Ref %d N_Ref %d\n", img->decoding_buf_idx, img->framepoc, img->nal_reference_idc, active_sps->num_ref_frames);
    }

    init_lists(img->type, img->currentSlice->structure);
    reorder_lists(img->type, img->currentSlice);

    //printf("list0 %d list1 %d list2 %d list4 %d\n", listXsize[0], listXsize[1], listXsize[2], listXsize[4]);

    if (img->type !=I_SLICE)
    {
        for (i = 0; i < 32; i++)
        {
            gSliceParameters.ref_idx_to_pic_idx_mapping[0][i] =
                gSliceParameters.ref_idx_to_pic_idx_mapping[1][i] = 0;
        }
        //printf("List0");
        for (i = 0; i < listXsize[0]; i++)
        {
            if (listX[0][i] == 0)
            {
                listXsize[0] = i;
                break;
            }
            gSliceParameters.ref_idx_to_pic_idx_mapping[0][i] = listX[0][i]->pic_idx;
            //printf(" %d", listX[0][i]->decoding_buf_idx);
        }
        //printf("\n");
        //printf("List1");
        for (i = 0; i < listXsize[1]; i++)
        {
            if (listX[1][i] == 0)
            {
                listXsize[1] = i;
                break;
            }
            gSliceParameters.ref_idx_to_pic_idx_mapping[1][i] = listX[1][i]->pic_idx;
            //printf(" %d", listX[1][i]->decoding_buf_idx);
        }
        //printf("\n");
        for (i = 0; i < 32; i++)
        {
            gSliceParameters.remapping_ref_idx_l0[i] = 0;
            gSliceParameters.remapping_ref_idx_l1[i] = 0;

            if (img->structure == FRAME)
            {
                if (i < listXsize[0])
                    gSliceParameters.remapping_ref_idx_l0[i] = listX[0][i]->decoding_buf_idx;
                if (i < listXsize[1])
                    gSliceParameters.remapping_ref_idx_l1[i] = listX[1][i]->decoding_buf_idx;
            }

            if (img->structure != FRAME)
            {
                if (i < listXsize[0])
                {
                    if (listX[0][i]->pic_idx & 1)
                        gSliceParameters.remapping_ref_idx_l0[i] = listX[0][i]->decoding_buf_idx*2+1;
                    else
                        gSliceParameters.remapping_ref_idx_l0[i] = listX[0][i]->decoding_buf_idx*2;
                }

                if (i < listXsize[1])
                {
                    if (listX[1][i]->pic_idx & 1)
                        gSliceParameters.remapping_ref_idx_l1[i] = listX[1][i]->decoding_buf_idx*2+1;
                    else
                        gSliceParameters.remapping_ref_idx_l1[i] = listX[1][i]->decoding_buf_idx*2;
                }
            }
        }
    }
    //printf("Remapping Buf %d %d %d %d\n", gSliceParameters.remapping_ref_idx_l0[0],
    //                                      gSliceParameters.remapping_ref_idx_l0[1],
    //                                      gSliceParameters.remapping_ref_idx_l0[2],
    //                                      gSliceParameters.remapping_ref_idx_l0[3]);

    if (img->structure==FRAME)
    {
        init_mbaff_lists();
    }

    if ((active_pps->weighted_bipred_idc > 0  && (img->type == B_SLICE)) || (active_pps->weighted_pred_flag && img->type !=I_SLICE))
    {
        fill_wp_params(img);
    }

    //set_ref_pic_num();

    if (img->type == B_SLICE)
        if ((ret = compute_collocated()) != AVC_ERROR_SUCCESS)
        {
            free_ref_pic_list_reordering_buffer(img->currentSlice);
            free_marking_buffer();
            free_storable_picture(dec_picture);
            dec_picture = MMP_NULL;
            return ret;
        }

    // wirte H/W data
    WriteHWData();
    return ret;
}

void
avc_frame_end(void)
{
    exit_picture();
}

void
avc_multi_slice_wr(MMP_UINT32 sliceMB)
{
    SliceParameters *ptSliceParam;
    MMP_UINT32 BufAddr;
    MMP_UINT32 nxtSliceMB;
    MMP_UINT32 tmpMBAX;
    MMP_UINT32 tmpMBA;
    MMP_UINT8  *PreCmdBuf, *CurCmdBuf;
    MMP_UINT32 totalMBs;

    // error handle
    if (img->structure == TOP_FIELD || img->structure == BOTTOM_FIELD)
        totalMBs = img->FrameSizeInMbs >> 1;
    else
        totalMBs = img->FrameSizeInMbs;

    if (img->MbaffFrameFlag == 1)
        nxtSliceMB = sliceMB * 2;
    else
        nxtSliceMB = sliceMB;

    if (nxtSliceMB >= totalMBs)
    {
        tmpMBA = (totalMBs -1) ;
    } else {
        tmpMBA = nxtSliceMB;
    }

    //if ((tmpMBA - img->currentSlice->start_mb_nr) < 5)
    //{
    //    av_log(NULL, AV_LOG_ERROR, "Error MultiSlice data F %d L %d W %d T %d\n", img->currentSlice->start_mb_nr, nxtSliceMB, img->PicWidthInMbs, totalMBs);
    //    gptAVCDecoder->cmdDataBufSize = 0;
    //    return;
    //}

    tmpMBAX = tmpMBA % img->PicWidthInMbs;

    // write last MB_X MB_Y
    ptSliceParam = (SliceParameters *)gAVCCmdBuf;

    PreCmdBuf = (MMP_UINT8*) (gAVCCmdBuf+SLICE_HEADER_SIZE);

    if (tmpMBAX == 0)
    {
        ptSliceParam->last_MB_in_slice_X = img->PicWidthInMbs - 1;
        ptSliceParam->last_MB_in_slice_Y = (tmpMBA / img->PicWidthInMbs) - 1;
    }
    else
    {
        ptSliceParam->last_MB_in_slice_X = tmpMBAX - 1;
        ptSliceParam->last_MB_in_slice_Y = (tmpMBA / img->PicWidthInMbs);
    }

    // write nextslice addr
    gAVCCmdBuf = gOrgCmdBuf + gptAVCDecoder->cmdDataBufSize;

    CurCmdBuf = (MMP_UINT8*) (gAVCCmdBuf+SLICE_HEADER_SIZE);

    BufAddr = (MMP_UINT32)gAVCCmdBuf >> 3;

    ptSliceParam->nextSliceHeaderDataBase8_l = (BufAddr & 0xFF);
    ptSliceParam->nextSliceHeaderDataBase8_m = (BufAddr & 0xFF00) >> 8;
    ptSliceParam->nextSliceHeaderDataBase8_h = (BufAddr & 0xFF0000) >> 16;

    //printf("X %d Y %d\n", ptSliceParam->last_MB_in_slice_X,
    //                      ptSliceParam->last_MB_in_slice_Y);

    memcpy(CurCmdBuf,
           PreCmdBuf,
           BITSTREAM_START - SLICE_HEADER_SIZE);
}

void
avc_store_flip_queue(MMP_UINT32 index)
{
    DecodingQueue[index] |= STORE_IN_FLIP_QUEUE;
}

void
avc_release_flip_buf(MMP_UINT32 index)
{
    DecodingQueue[index] &= 0x0000FFFF;
}

void
avc_release_decoding_buf(void)
{
    if (dec_picture==MMP_NULL)
        return;
    DecodingQueue[dec_picture->decoding_buf_idx] = 0;
    free_marking_buffer();

    if ((dec_picture->structure==TOP_FIELD)||(dec_picture->structure==BOTTOM_FIELD))
    {
        // check for frame store with same pic_number
        if (dpb.last_picture)
        {
            if ((int)dpb.last_picture->frame_num == dec_picture->pic_num)
            {
                if (((dec_picture->structure==TOP_FIELD)&&(dpb.last_picture->is_used==2))||((dec_picture->structure==BOTTOM_FIELD)&&(dpb.last_picture->is_used==1)))
                {
                    if (dpb.used_size > 0)
                        remove_frame_from_dpb(dpb.used_size-1);
                }
            }
        }
    }

    free_storable_picture(dec_picture);
    dec_picture = NULL;
}

MMP_BOOL get_dec_pic_info(SliceType* picType, MMP_BOOL* bFrame, MMP_BOOL* bLastPic)
{
    if (MMP_NULL == dec_picture)
        return MMP_FALSE;

    if (B_SLICE == dec_picture->slice_type
        && dec_picture->used_for_reference)
        *picType = P_SLICE;
    else
        *picType = dec_picture->slice_type;
    //if (dpb.last_picture)
    //    *bLastPic = MMP_TRUE;
    //else
    //    *bLastPic = MMP_FALSE;

    *bLastPic = gbLastPic;

    *bFrame = dec_picture->coded_frame;
    return MMP_TRUE;
}

void
avc_set_skip_dec(
    MMP_BOOL    bSkip)
{
    gbSkipHwDecode = bSkip;
}

void ChkSlice(void)
{
    MMP_UINT32 i = 0;
    SliceParameters *ptSliceParam;
    MMP_UINT32 BufAddr = MMP_NULL;

retry :
    if (i == 0)
        ptSliceParam = (SliceParameters *) gptAVCDecoder->ppCmdDataBufAddr[gptAVCDecoder->cmdDataBufSelect];
    else
        ptSliceParam = (SliceParameters *) BufAddr;

    if (ptSliceParam->last_MB_in_slice_X == 0)
    {
        ptSliceParam->last_MB_in_slice_X = ptSliceParam->frameWidthMB - 1;
        if (ptSliceParam->last_MB_in_slice_Y != 0)
            ptSliceParam->last_MB_in_slice_Y -= 1;
    }

    if (ptSliceParam->last_MB_in_slice_Y == 0)
        ptSliceParam->last_MB_in_slice_Y += 1;

    if (ptSliceParam->pictureStructure == 3 && (ptSliceParam->last_MB_in_slice_Y & 0x1) == 0)
        ptSliceParam->last_MB_in_slice_Y += 1;

    i++;
    BufAddr = ((ptSliceParam->nextSliceHeaderDataBase8_h << 16) |
              (ptSliceParam->nextSliceHeaderDataBase8_m << 8) |
              (ptSliceParam->nextSliceHeaderDataBase8_l)) << 3;

    if (BufAddr != 0)
    {
        goto retry;
    }
}

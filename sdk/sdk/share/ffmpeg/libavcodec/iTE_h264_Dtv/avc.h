/*
 * Copyright (c) 2010 ITE Technology Corp. All Rights Reserved.
 */
/** @file avc.h
 *
 * @author
 */

#ifndef _AVC_H_
#define _AVC_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

#include "ite/mmp_types.h"
#include "ite/itp.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MAX_STREAM_COUNT                    1
#define MAX_FRAME_WIDTH_IN_MBs              135//45
#define MAX_FRAME_HEIGHTIN_MBs              72//36

#define MIN_FRAME_WIDTH_IN_MBs              8
#define MIN_FRAME_HEIGHTIN_MBs              6

#define NAL_UNIT_TYPE_NON_IDR_PICTURE       (1)
#define NAL_UNIT_TYPE_IDR_PICTURE           (5)
#define NAL_UNIT_TYPE_SEI                   (6)
#define NAL_UNIT_TYPE_SEQ_PARAM_SET         (7)
#define NAL_UNIT_TYPE_PIC_PARAM_SET         (8)
#define NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER (9)
// AVC decoder error code
typedef MMP_UINT32 AVC_ERROR_CODE;

#define AVC_ERROR_SUCCESS                   0
#define ERROR_MALLOC_SEQUENCE_SET           (1 << 0)
#define ERROR_MALLOC_PIC_SET                (1 << 1)
#define ERROR_NO_ACTIVE_PICTURE_SET         (1 << 2)
#define ERROR_SEQUENCE_SET_ID               (1 << 3)
#define ERROR_PICTURE_SET_ID                (1 << 4)
#define ERROR_NO_ACTIVE_SEQUENCE_SET        (1 << 5)
#define ERROR_NAL_Parser_Bitstream_Error    (1 << 6)
#define ERROR_SKIP_B_FRAME                  (1 << 7)
#define ERROR_RESOLUTION                    (1 << 8)
#define ERROR_SEQUENCE_SET_NOT_SUPPORT      (1 << 9)
#define ERROR_PICTURE_SET_NOT_SUPPORT       (1 << 10)
#define ERROR_BITSTREAM_ERROR               (1 << 11)
#define ERROR_OUT_SUPPORT_REF_RANGE         (1 << 12)
#define ERROR_PICTURE_TYPE                  (1 << 13)
#define ERROR_SEI_NOT_SUPPORT               (1 << 14)
#define ERROR_REF_PARAMETER                 (1 << 15)

#define ERROR_TEMP_USE                      (1 << 31)

#define MAX_REFERENCE_PICTURES              5       //!< H264 allows 32 fields
#define MAX_RBSP_SIZE                       1024
#define MAX_NAL_SIZE                        MAX_RBSP_SIZE

typedef enum {
    P_SLICE = 0,
    B_SLICE,
    I_SLICE,
    SP_SLICE,
    SI_SLICE
} SliceType;

typedef enum {
    FRAME,
    TOP_FIELD,
    BOTTOM_FIELD
} PictureStructure;           //!< New enum for field processing

typedef enum {
    LIST_0=0,
    LIST_1=1
} Lists;

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct AVC_NAL_HEADER_TAG
{
    MMP_UINT32 forbidden_zero_bit;
    MMP_UINT32 nal_ref_idc;
    MMP_UINT32 nal_unit_type;
} AVC_NAL_HEADER;

typedef struct _Slice
{
    int                 qp;
    int                 slice_qp_delta;
    int                 picture_type;  //!< picture type
    PictureStructure    structure;     //!< Identify picture structure type
    int                 start_mb_nr;   //!< MUST be set by NAL even in case of ei_flag == 1
    int                 ref_pic_list_reordering_flag_l0;
    int                 *remapping_of_pic_nums_idc_l0;
    int                 *abs_diff_pic_num_minus1_l0;
    int                 *long_term_pic_idx_l0;
    int                 ref_pic_list_reordering_flag_l1;
    int                 *remapping_of_pic_nums_idc_l1;
    int                 *abs_diff_pic_num_minus1_l1;
    int                 *long_term_pic_idx_l1;
    int                 pic_parameter_set_id;   //!<the ID of the picture parameter set the slice is reffering to
} Slice;

typedef struct DecRefPicMarking_s
{
    int memory_management_control_operation;
    int difference_of_pic_nums_minus1;
    int long_term_pic_num;
    int long_term_frame_idx;
    int max_long_term_frame_idx_plus1;
    struct DecRefPicMarking_s *Next;
} DecRefPicMarking_t;

// image parameters
typedef struct img_par
{
    int qp;                                         //!< quant for the current frame
    int preQP;
    int direct_spatial_mv_pred_flag;                //!< 1 for Spatial Direct, 0 for Temporal
    int type;                                       //!< image type INTER/INTRA
    int mvscale[6][MAX_REFERENCE_PICTURES];
    int newframe;
    int structure;                                  //!< Identify picture structure type
    Slice       *currentSlice;                      //!< pointer to current Slice data struct
    int MbaffFrameFlag;
    DecRefPicMarking_t *dec_ref_pic_marking_buffer; //!< stores the memory management control operations

    int disposable_flag;                            //!< flag for disposable frame, 1:disposable
    int num_ref_idx_l0_active;                      //!< number of forward reference
    int num_ref_idx_l1_active;                      //!< number of backward reference

    int redundant_pic_cnt;

    unsigned int pre_frame_num;                     //!< store the frame_num in the last decoded slice. For detecting gap in frame_num.

    // End JVT-D101
    // POC200301: from unsigned int to int
             int toppoc;      //poc for this top field // POC200301
             int bottompoc;   //poc of bottom field of frame
             int framepoc;    //poc of this frame // POC200301
    unsigned int frame_num;   //frame_num for this frame
    unsigned int field_pic_flag;
    unsigned int bottom_field_flag;

    //the following is for slice header syntax elements of poc
    // for poc mode 0.
    unsigned int pic_order_cnt_lsb;
             int delta_pic_order_cnt_bottom;
    // for poc mode 1.
             int delta_pic_order_cnt[3];

    // ////////////////////////
    // for POC mode 0:
      signed int PrevPicOrderCntMsb;
    unsigned int PrevPicOrderCntLsb;
      signed int PicOrderCntMsb;

    // for POC mode 1:
    unsigned int AbsFrameNum;
      signed int ExpectedPicOrderCnt, PicOrderCntCycleCnt, FrameNumInPicOrderCntCycle;
    unsigned int PreviousFrameNum, FrameNumOffset;
             int ExpectedDeltaPerPicOrderCntCycle;
             int PreviousPOC, ThisPOC;
             int PreviousFrameNumOffset;
    // /////////////////////////

    //weighted prediction
    unsigned int luma_log2_weight_denom;
    unsigned int chroma_log2_weight_denom;
    //int ***wp_weight;  // weight in [list][index][component] order
    //int ***wp_offset;  // offset in [list][index][component] order
    
    int wp_weight[2][MAX_REFERENCE_PICTURES][3];  // weight in [list][index][component] order
    int wp_offset[6][MAX_REFERENCE_PICTURES][3];  // offset in [list][index][component] order
    
    int idr_flag;
    int nal_reference_idc;                       //!< nal_reference_idc from NAL unit

    int idr_pic_id;

    int MaxFrameNum;

    unsigned PicWidthInMbs;
    unsigned PicHeightInMapUnits;
    unsigned FrameHeightInMbs;
    unsigned PicHeightInMbs;
    unsigned PicSizeInMbs;
    unsigned FrameSizeInMbs;
    unsigned preFrameSizeInMbs;
    unsigned oldFrameSizeInMbs;

    int no_output_of_prior_pics_flag;
    int long_term_reference_flag;
    int adaptive_ref_pic_buffering_flag;

    int last_has_mmco_5;
    int last_pic_bottom_field;

    int model_number;

    int AllowTransform8x8;

    int decoding_buf_idx;

    int picture_structure;
} ImageParameters;

//=============================================================================
//                              Function Declaration
//=============================================================================

void
avc_Nal_Get_Header(AVC_NAL_HEADER *pNalHeader, MMP_UINT8 *pNalUnit);

AVC_ERROR_CODE
avc_Seq_Parameter(MMP_UINT8 *pNalUnit,
                  MMP_UINT32 numberBytesInNALunit);
                  
AVC_ERROR_CODE
avc_Pic_Parameter(MMP_UINT8 *pNalUnit,
                  MMP_UINT32 numberBytesInNALunit);
                  
AVC_ERROR_CODE
avc_SEI(MMP_UINT8 *pNalUnit,
        MMP_UINT32 numberBytesInNALunit,
        MMP_BOOL  *bRecovery_Point);
                  
void
avc_Get_First_MB_In_Slice(MMP_UINT32 *pFirstMBinSlice,
                          MMP_UINT8 *pNalUnit,
                          MMP_UINT32 numberBytesInNALunit);
                          
AVC_ERROR_CODE 
avc_slice(AVC_NAL_HEADER *pNalHeader,
          MMP_UINT8 *pNalUnit,
          MMP_UINT32 numberBytesInNALunit);
          
void 
avc_CreateParameterSetList(void);

void
avc_ReleaseParameterSetList(void);

void 
avc_ResetParameterSetList(void);                                                                        
                  
void
avc_ResetFrmGapCheck(void);

void
avc_frame_end(void);

void
avc_multi_slice_wr(MMP_UINT32 sliceMB);

void
avc_release_flip_buf(MMP_UINT32 index);

void
avc_release_decoding_buf(void);

MMP_BOOL
get_dec_pic_info(SliceType* picType, MMP_BOOL* bFrame, MMP_BOOL* bLastPic);

void
avc_set_skip_dec(
    MMP_BOOL    bSkip);
    
void 
avc_ResetBuffer(void);

void 
ChkSlice(void);

void
avc_store_flip_queue(MMP_UINT32 index);

#ifdef __cplusplus
}
#endif

#endif

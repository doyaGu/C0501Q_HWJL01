/***
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 *
 * @File		ite_h264.c
 * @Author	Evan Chang
 * @Version	1.0.0
 * @Chipset	MM9070
 *
 */

//#define USE_BUFFER_MANAGER

#include <stdio.h>
#include "h264.h"
#include "ite/ith_video.h"
#ifdef USE_BUFFER_MANAGER
#include "itv/fc_bm.h"
#endif

#define SLICE_HEADER_SIZE               (8 * 15)
#define REFIDX_TO_PICIDX_TABLE_SIZE     (8 * 32)
#define PICIDX_TO_DIST_TABLE_SIZE       (8 * 160)
#define SCALING_TABLE_SIZE              (8 * 112)
#define WEIGHTING_TABLE_SIZE            (8 * 192)
#define CABAC_TABLE_SIZE                (8 * 64)


#define SLICE_HEADER_START              0
#define REFIDX_TO_PICIDX_TABLE_START    (SLICE_HEADER_START + SLICE_HEADER_SIZE)
#define PICIDX_TO_DIST_TABLE_START      (REFIDX_TO_PICIDX_TABLE_START + REFIDX_TO_PICIDX_TABLE_SIZE)
#define SCALING_TABLE_START             (PICIDX_TO_DIST_TABLE_START + PICIDX_TO_DIST_TABLE_SIZE)
#define WEIGHTING_TABLE_START           (SCALING_TABLE_START + SCALING_TABLE_SIZE)
#define CABAC_TABLE_START               (WEIGHTING_TABLE_START + WEIGHTING_TABLE_SIZE)
#define BITSTREAM_START                 (CABAC_TABLE_START + CABAC_TABLE_SIZE)


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

unsigned char* debugSliceParameters[120] = {
	"frameWidthMB",                            // A
	"frameHeightMB",                           // B
	"lumaPitch8_l",                            // C
	"lumaPitch8_h",                            // C
	"chromaPitch8_l",                          // D
	"chromaPitch8_h",                          // D
	"videoFormat",                             // E
	"pictureStructure",                        // F
	"direct_8x8_inference_flag",               // G
	"chroma_format_idc",                       // H
	"entropy_coding_mode_flag",                // I
	"weighted_pred_mode",                      // J
	"pic_init_qp_minus26",                     // K
	"chroma_qp_index_offset",                  // L
	"second_chroma_qp_index_offset",           // M
	"deblocking_filter_control_present_flag",  // N
	"constrained_intra_pred_flag",             // O
	"transform_8x8_mode_flag",                 // P
	"sliceType",                               // Q
	"direct_spatial_mv_pred_flag",             // R
	"num_ref_idx_l0_active_minus1",            // S
	"num_ref_idx_l1_active_minus1",            // T
	"luma_log2_weight_denom",                  // U
	"chroma_log2_weight_denom",                // V
	"colPicIsLongterm",                        // W
	"colDataPlacement",                        // X
	"colDataUseWhichChunk",                    // Y
	"first_MB_in_slice_X",                     // Z
	"first_MB_in_slice_Y",                     // AA
	"decodeBufferSelector",                    // AB
	"remapping_ref_idx_l0[0]",                // AC000 ~ AC031
	"remapping_ref_idx_l0[1]",
	"remapping_ref_idx_l0[2]",
	"remapping_ref_idx_l0[3]",
	"remapping_ref_idx_l0[4]",
	"remapping_ref_idx_l0[5]",
	"remapping_ref_idx_l0[6]",
	"remapping_ref_idx_l0[7]",
	"remapping_ref_idx_l0[8]",
	"remapping_ref_idx_l0[9]",
	"remapping_ref_idx_l0[10]",
	"remapping_ref_idx_l0[11]",
	"remapping_ref_idx_l0[12]",
	"remapping_ref_idx_l0[13]",
	"remapping_ref_idx_l0[14]",
	"remapping_ref_idx_l0[15]",
	"remapping_ref_idx_l0[16]",
	"remapping_ref_idx_l0[17]",
	"remapping_ref_idx_l0[18]",
	"remapping_ref_idx_l0[19]",
	"remapping_ref_idx_l0[20]",
	"remapping_ref_idx_l0[21]",
	"remapping_ref_idx_l0[22]",
	"remapping_ref_idx_l0[23]",
	"remapping_ref_idx_l0[24]",
	"remapping_ref_idx_l0[25]",
	"remapping_ref_idx_l0[26]",
	"remapping_ref_idx_l0[27]",
	"remapping_ref_idx_l0[28]",
	"remapping_ref_idx_l0[29]",
	"remapping_ref_idx_l0[30]",
	"remapping_ref_idx_l0[31]",
	"remapping_ref_idx_l1[0]",                // AC100 ~ AC131
	"remapping_ref_idx_l1[1]",
	"remapping_ref_idx_l1[2]",
	"remapping_ref_idx_l1[3]",
	"remapping_ref_idx_l1[4]",
	"remapping_ref_idx_l1[5]",
	"remapping_ref_idx_l1[6]",
	"remapping_ref_idx_l1[7]",
	"remapping_ref_idx_l1[8]",
	"remapping_ref_idx_l1[9]",
	"remapping_ref_idx_l1[10]",
	"remapping_ref_idx_l1[11]",
	"remapping_ref_idx_l1[12]",
	"remapping_ref_idx_l1[13]",
	"remapping_ref_idx_l1[14]",
	"remapping_ref_idx_l1[15]",
	"remapping_ref_idx_l1[16]",
	"remapping_ref_idx_l1[17]",
	"remapping_ref_idx_l1[18]",
	"remapping_ref_idx_l1[19]",
	"remapping_ref_idx_l1[20]",
	"remapping_ref_idx_l1[21]",
	"remapping_ref_idx_l1[22]",
	"remapping_ref_idx_l1[23]",
	"remapping_ref_idx_l1[24]",
	"remapping_ref_idx_l1[25]",
	"remapping_ref_idx_l1[26]",
	"remapping_ref_idx_l1[27]",
	"remapping_ref_idx_l1[28]",
	"remapping_ref_idx_l1[29]",
	"remapping_ref_idx_l1[30]",
	"remapping_ref_idx_l1[31]",
	"last_MB_in_slice_X",                      // AD
	"last_MB_in_slice_Y",                      // AE
	"discardBits",                             // AF
	"lastBytePosition",                        // AG
	"frame_poc_l",                             // AH
	"frame_poc_h",                             // AH
	"top_poc_l",                               // AI
	"top_poc_h",                               // AI
	"bottom_poc_l",                            // AJ
	"bottom_poc_h",                            // AJ
	"directWriteColDataBufferBase8_l",         // AK
	"directWriteColDataBufferBase8_m",         // AK
	"directWriteColDataBufferBase8_h",         // AK
	"stuff1",
	"directReadColDataBufferBase8_l",          // AL
	"directReadColDataBufferBase8_m",          // AL
	"directReadColDataBufferBase8_h",          // AL
	"stuff2",
	"nextSliceHeaderDataBase8_l",              // AM
	"nextSliceHeaderDataBase8_m",              // AM
	"nextSliceHeaderDataBase8_h",              // AM
	"stuff3",
	"tableInsertFlag",                         // AN
	"stuff4",
	"stuff5",
	"stuff6"
};

typedef enum {
	FRAME			= 0x00,
	TOP_FIELD		= 0x01,
	BOTTOM_FIELD	= 0x10,
	MBAFF			= 0x11
} PictureStructure;

typedef enum {
	SLICE_TYPE_P	= 0,
	SLICE_TYPE_B,
	SLICE_TYPE_I,
	SLICE_TYPE_SP,
	SLICE_TYPE_SI
} SliceType;

static const unsigned char PicStruct[4] = {MBAFF, TOP_FIELD, BOTTOM_FIELD, FRAME};
static const char pict_type_to_golomb[8]= {-1, SLICE_TYPE_I, SLICE_TYPE_P, SLICE_TYPE_B, -1, SLICE_TYPE_SI, SLICE_TYPE_SP, -1};
static const char pict_type_to_ite[5]= {1, 2, 0, 1, 0};

unsigned short cabac_map_table[512] = {
	106, 167, 107, 168, 108, 169, 109, 170,
	110, 171, 111, 172, 112, 173, 113, 174,
	114, 175, 115, 176, 116, 177, 117, 178,
	118, 179, 119, 180, 21, 22, 23, 0,
	121, 182, 122, 183, 123, 184, 124, 185,
	125, 186, 126, 187, 127, 188, 128, 189,
	129, 190, 130, 191, 131, 192, 132, 193,
	133, 194, 0, 0, 11, 12, 13, 0,
	135, 196, 136, 197, 137, 198, 138, 199,
	139, 200, 140, 201, 141, 202, 142, 203,
	143, 204, 144, 205, 145, 206, 146, 207,
	147, 208, 148, 209, 68, 0, 69, 0,
	150, 211, 151, 212, 60, 61, 62, 63,
	85, 86, 87, 88, 105, 166, 277, 338,
	89, 90, 91, 92, 120, 181, 292, 353,
	40, 41, 42, 43, 44, 45, 46, 0,
	153, 214, 154, 215, 155, 216, 156, 217,
	157, 218, 158, 219, 159, 220, 160, 221,
	161, 222, 162, 223, 163, 224, 164, 225,
	165, 226, 0, 0, 399, 400, 401, 0,
	419, 408, 409, 410, 411, 412, 413, 414,
	420, 421, 422, 423, 424, 425, 415, 416,
	417, 418, 402, 403, 404, 405, 406, 407,
	3, 4, 5, 6, 7, 8, 9, 10,
	228, 229, 230, 231, 232, 0, 0, 0,
	238, 239, 240, 241, 242, 0, 0, 0,
	248, 249, 250, 251, 252, 0, 0, 0,
	258, 259, 260, 261, 262, 0, 0, 0,
	267, 268, 269, 270, 271, 0, 0, 0,
	427, 428, 429, 430, 431, 0, 0, 0,
	101, 102, 103, 104, 152, 213, 324, 385,
	77, 78, 79, 80, 81, 82, 83, 84,
	278, 339, 279, 340, 280, 341, 281, 342,
	282, 343, 283, 344, 284, 345, 285, 346,
	286, 347, 287, 348, 288, 349, 289, 350,
	290, 351, 291, 352, 36, 37, 38, 39,
	293, 354, 294, 355, 295, 356, 296, 357,
	297, 358, 298, 359, 299, 360, 300, 361,
	301, 362, 302, 363, 303, 364, 304, 365,
	305, 366, 0, 0, 24, 25, 26, 0,
	307, 368, 308, 369, 309, 370, 310, 371,
	311, 372, 312, 373, 313, 374, 314, 375,
	315, 376, 316, 377, 317, 378, 318, 379,
	319, 380, 320, 381, 64, 65, 66, 67,
	322, 383, 323, 384, 70, 71, 72, 0,
	93, 94, 95, 96, 134, 195, 306, 367,
	97, 98, 99, 100, 149, 210, 321, 382,
	47, 48, 49, 50, 51, 52, 53, 0,
	325, 386, 326, 387, 327, 388, 328, 389,
	329, 390, 330, 391, 331, 392, 332, 393,
	333, 394, 334, 395, 335, 396, 336, 397,
	337, 398, 0, 0, 73, 74, 75, 76,
	453, 442, 443, 444, 445, 446, 447, 448,
	454, 455, 456, 457, 458, 459, 449, 450,
	451, 452, 436, 437, 438, 439, 440, 441,
	14, 15, 16, 17, 18, 19, 20, 0,
	227, 233, 234, 235, 236, 0, 0, 0,
	237, 243, 244, 245, 246, 0, 0, 0,
	247, 253, 254, 255, 256, 0, 0, 0,
	257, 263, 264, 265, 0, 0, 0, 0,
	266, 272, 273, 274, 275, 0, 0, 0,
	426, 432, 433, 434, 435, 33, 34, 35,
	54, 55, 56, 57, 58, 59, 0, 0,
	27, 28, 29, 30, 31, 32, 0, 0
};

#ifdef USE_BUFFER_MANAGER
FC_STRC_BM *bm;

void ite_hw_init(void)
{
	bm = (struct FC_STRC_BM*) malloc(sizeof(struct FC_STRC_BM)));
	fc_bm_init(bm, 32, 2048);

	// assign buffer addr
}

void ite_hw_deinit(void)
{
	free(bm);
	bm = NULL;
}
#endif

void ite_hw_write_data(H264Context *h, AVC_DECODER* pAVCDecoder)
{
	int bm_idx = -1;
	uint32_t i1, i2, i3, index = 0, data, slice_data_buf_offset = 0;
	uint8_t *pCmdBuf = (uint8_t *) malloc(sizeof(uint8_t) * 204800); //NULL;
	MpegEncContext * const s = &h->s;
	SliceParameters *pSliceParam = (SliceParameters*) malloc(sizeof(SliceParameters)); // Command data buffer addr
	memset(pCmdBuf, 0x00, sizeof(uint8_t) * 204800);
	memset(pSliceParam, 0x00, sizeof(SliceParameters));

	/* iTE H.264 H/W decoder compatible data field
	 * Slice Header Data
	 */
	pSliceParam->frameWidthMB	= h->sps.mb_width;
	pSliceParam->frameHeightMB	= h->sps.mb_height;
	pSliceParam->lumaPitch8_l	= (h->sps.mb_width * 2) & 0xFF; // 192; // Now Pitch is 1536 for tiling mode
	pSliceParam->lumaPitch8_h	= (h->sps.mb_width * 2) >> 8; // 0;
	pSliceParam->chromaPitch8_l	= h->sps.mb_width & 0xFF; // 192;
	pSliceParam->chromaPitch8_h	= h->sps.mb_width >> 8; // 0;
	pSliceParam->videoFormat	= 0; // H.264
	pSliceParam->pictureStructure = PicStruct[h->mb_aff_frame ? 0 : s->picture_structure];
	pSliceParam->direct_8x8_inference_flag = h->sps.direct_8x8_inference_flag;
	pSliceParam->chroma_format_idc = (h->sps.chroma_format_idc & 0x11) ? 1 : 0;
	pSliceParam->entropy_coding_mode_flag = h->pps.cabac;
	pSliceParam->weighted_pred_mode = 0; // P/B explicit, set to be default
	if (pict_type_to_golomb[h->slice_type] == SLICE_TYPE_B)
		if (h->pps.weighted_bipred_idc == 2)
			pSliceParam->weighted_pred_mode = 1; // B implicit

	//if (pSliceParam->weighted_pred_mode == 1) printf("pSliceParam->weighted_pred_mode = 1\n");

	pSliceParam->pic_init_qp_minus26 = s->qscale - h->pps.init_qp; // h->pps.init_qp - 26 - 6*(h->sps.bit_depth_luma-8);
	pSliceParam->chroma_qp_index_offset = h->pps.chroma_qp_index_offset[0];
	pSliceParam->second_chroma_qp_index_offset = h->pps.chroma_qp_index_offset[1];
	pSliceParam->deblocking_filter_control_present_flag = h->pps.deblocking_filter_parameters_present;

	pSliceParam->constrained_intra_pred_flag = h->pps.constrained_intra_pred;
	pSliceParam->transform_8x8_mode_flag = h->pps.transform_8x8_mode;
	pSliceParam->sliceType = pict_type_to_ite[pict_type_to_golomb[h->slice_type]]; //h->slice_type - 1;  // I, P, B from slice header

	pSliceParam->direct_spatial_mv_pred_flag = h->direct_spatial_mv_pred;
	pSliceParam->num_ref_idx_l0_active_minus1 = (h->list_count) ? (h->ref_count[0] - 1) : (h->pps.ref_count[0] - 1);
	pSliceParam->num_ref_idx_l1_active_minus1 = (h->list_count) ? (h->ref_count[1] - 1) : (h->pps.ref_count[1] - 1);

	pSliceParam->luma_log2_weight_denom = h->luma_log2_weight_denom;
	pSliceParam->chroma_log2_weight_denom = h->chroma_log2_weight_denom;

	pSliceParam->colPicIsLongterm = 0;
	pSliceParam->colDataPlacement = !(pSliceParam->pictureStructure == FRAME);
	pSliceParam->colDataUseWhichChunk = (pSliceParam->pictureStructure == BOTTOM_FIELD);

	pSliceParam->first_MB_in_slice_X = s->resync_mb_x; // s->mb_x;
	pSliceParam->first_MB_in_slice_Y = s->resync_mb_y; // s->mb_y;

#ifdef USE_BUFFER_MANAGER
	do
	{
		bm_idx = fc_bm_trylock(bm);
	} while(bm_idx != -1);
	FC_BM_DEC_MARK_USE(bm, bm_idx);
#endif
	pSliceParam->decodeBufferSelector = bm_idx;
	for (i1 = 0; i1 < 32; i1++)
	{
		pSliceParam->remapping_ref_idx_l0[i1] = h->ref_list[0][i1].frame_num;
		pSliceParam->remapping_ref_idx_l1[i1] = h->ref_list[1][i1].frame_num;
		//ptSliceParam->remapping_ref_idx_l0[i1] = gSliceParameters.remapping_ref_idx_l0[i1];
		//ptSliceParam->remapping_ref_idx_l1[i1] = gSliceParameters.remapping_ref_idx_l1[i1];
	}

	// temp solution
	pSliceParam->last_MB_in_slice_X = h->sps.mb_width - 1; // gSliceParameters.last_MB_in_slice_X;
	if (pSliceParam->pictureStructure == TOP_FIELD || pSliceParam->pictureStructure == BOTTOM_FIELD)
		pSliceParam->last_MB_in_slice_Y = (h->sps.mb_height >> 1) - 1; // gSliceParameters.last_MB_in_slice_Y;
	else
		pSliceParam->last_MB_in_slice_Y = h->sps.mb_height - 1; // gSliceParameters.last_MB_in_slice_Y;

	//printf("MB slice Y %d\n", ptSliceParam->last_MB_in_slice_Y);

	slice_data_buf_offset = h->parse_history[h->parse_history_count-1] / 8; // s->gb.index / 8;
	pSliceParam->discardBits = h->parse_history[h->parse_history_count-1] & 0x07; // s->gb.index & 0x07; // (gb.index % 8)
	pSliceParam->lastBytePosition = (s->gb.buffer_end - (s->gb.buffer + slice_data_buf_offset)) & 0x07; // (bytes % 8)

	pSliceParam->directWriteColDataBufferBase8_l = 0; //(gptAVCDecoder->colDataBufAdr[0] & 0xFF);
	pSliceParam->directWriteColDataBufferBase8_m = 0; //(gptAVCDecoder->colDataBufAdr[0] & 0xFF00) >> 8;
	pSliceParam->directWriteColDataBufferBase8_h = 0; //(gptAVCDecoder->colDataBufAdr[0] & 0xFF0000) >> 16;
	pSliceParam->directReadColDataBufferBase8_l = 0; //(gptAVCDecoder->colDataBufAdr[0] & 0xFF);
	pSliceParam->directReadColDataBufferBase8_m = 0; //(gptAVCDecoder->colDataBufAdr[0] & 0xFF00) >> 8;
	pSliceParam->directReadColDataBufferBase8_h = 0; //(gptAVCDecoder->colDataBufAdr[0] & 0xFF0000) >> 16;

	pSliceParam->frame_poc_l = h->s.current_picture_ptr->poc & 0x0FF;
	pSliceParam->frame_poc_h = (h->s.current_picture_ptr->poc & 0x0FF00) >> 8;
	pSliceParam->top_poc_l = h->s.current_picture_ptr->field_poc[0] & 0x0FF;
	pSliceParam->top_poc_h = (h->s.current_picture_ptr->field_poc[0] & 0x0FF00) >> 8;
	pSliceParam->bottom_poc_l = h->s.current_picture_ptr->field_poc[1] & 0x0FF;
	pSliceParam->bottom_poc_h = (h->s.current_picture_ptr->field_poc[1] & 0x0FF00) >> 8;

#if defined(IT9063)
	ptSliceParam->directWriteColDataBufferBase8_l = (gptAVCDecoder->colDataBufAdr[0] & 0xFF);
	ptSliceParam->directWriteColDataBufferBase8_m = (gptAVCDecoder->colDataBufAdr[0] & 0xFF00) >> 8;
	ptSliceParam->directWriteColDataBufferBase8_h = (gptAVCDecoder->colDataBufAdr[0] & 0xFF0000) >> 16;

	if (currSlice->picture_type == 1)
	{
		ptSliceParam->directReadColDataBufferBase8_l = (gptAVCDecoder->colDataBufAdr[0] & 0xFF);
		ptSliceParam->directReadColDataBufferBase8_m = (gptAVCDecoder->colDataBufAdr[0] & 0xFF00) >> 8;
		ptSliceParam->directReadColDataBufferBase8_h = (gptAVCDecoder->colDataBufAdr[0] & 0xFF0000) >> 16;
	}
#endif
	//printf("Col data %d %d %d %d\n", dec_picture->col_data_buf_idx, listX[1][0]->col_data_buf_idx, gSliceParameters.col_data_placement, gSliceParameters.col_data_use_which_chunk);

	pSliceParam->nextSliceHeaderDataBase8_l = 0;              // AM
	pSliceParam->nextSliceHeaderDataBase8_m = 0;              // AM
	pSliceParam->nextSliceHeaderDataBase8_h = 0;              // AM

	pSliceParam->tableInsertFlag = 63;

	// Copy Slice Parameters to CommandDataBuffer
	memcpy(pCmdBuf, pSliceParam, sizeof(SliceParameters));

	/* iTE H.264 H/W decoder compatible data field
	 * Slice table Data
	 *
	 * 1 : RefIdx to PicIdx Table
	 *     Implicite Weighting Table
	 * 2 : PicIdx to Dist Table
	 *        PicIdx to RefIdx
	 *        RefIdx to Dist
	 * 3 : Scaling Table
	 * 4 : Explicite Weighting Table
	 * 5 : CABAC Table
	 *
	 * Table is 64bit alignment in CommandDataBuffer, so we need to assign some zero bytes in each table tail.
	 *
	 * Hint : ffmpeg usually use 48 element, 0..15 is for frame, 16..48 is for MBAFF field
	 */

	/*
	 * Table 1 : RefIdx to PicIdx Table
	 */

	for (i1 = 0; i1 < 32; i1++)
	{
		// RefIdx to PicIdx Table
		for (i2 = 0; i2 < 2; i2++)
			*(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = h->ref_list[i2][i1].pic_id & 0x03F;

		// Implicite weighting Table
		*(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = h->ref_list[0][i1].frame_num; //gRefIdxToPocListIdx[0][i1];
		*(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = h->ref_list[1][i1].frame_num; //gRefIdxToPocListIdx[1][i1];
		*(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = h->ref_list[0][i1].poc & 0x0FF; //gPocList[i1] & 0x0FF;
		*(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = h->ref_list[0][i1].poc >> 8; //gPocList[i1] >> 8;

		// Assign two Zero bytes
		for (i2 = 0; i2 < 2; i2++)
			*(pCmdBuf + REFIDX_TO_PICIDX_TABLE_START + index++) = 0;

	} index = 0;

	/*
	 * Table 2 : PicIdx to Dist Table
	 */

	// PicIdx to RefIdx
	for (i1 = 0; i1 < 64; i1++)
	{
		for (i2 = 0; i2 < 3; i2++)
			*(pCmdBuf + PICIDX_TO_DIST_TABLE_START + index++) = h->map_col_to_list0[i2][i1] & 0x1F;

		// Assign five Zero bytes
		for (i2 = 0; i2 < 5; i2++)
			*(pCmdBuf + PICIDX_TO_DIST_TABLE_START + index++) = 0;
	}

	// RefIdx to Dist
	// 0 : Frame/Field
	// 1 : MBAFF_Top
	// 2 : MBAFF_Bottom
	for (i1 = 0; i1 < 3; i1++)
	{
		for (i2 = 0; i2 < 32; i2++)
		{
			data = ((i1==0 && i2<16)?h->dist_scale_factor[i2] : 256) & 0x07FF; //gSliceParameters.col_ref_idx_to_dist_scale_mapping[i1][i2] & 0x07FF;
			if (i1 != 0)
				data = h->dist_scale_factor_field[i1-1][i2] & 0x07FF;
			*(pCmdBuf + PICIDX_TO_DIST_TABLE_START + index++) = data & 0x0FF;
			*(pCmdBuf + PICIDX_TO_DIST_TABLE_START + index++) = data >> 8 & 0x0FF;

			// Assign six Zero bytes
			for (i3 = 0; i3 < 6; i3++)
			{
				*(pCmdBuf + PICIDX_TO_DIST_TABLE_START + index++) = 0;
			}
		}
	} index = 0;

	/*
	 * Table 3 : Scaling Table
	 */

	// 0 : Y_Intra4x4
	// 1 : Cb_Intra4x4
	// 2 : Cr_Intra4x4
	// 3 : Y_Inter4x4
	// 4 : Cb_Inter4x4
	// 5 : Cr_Inter4x4
	for (i1 = 0; i1 < 6; i1++)
	{
		for (i2 = 0; i2 < 16; i2 +=2)
		{
			*(pCmdBuf + SCALING_TABLE_START + index++) = h->sps.scaling_matrix4[i1][i2];
			*(pCmdBuf + SCALING_TABLE_START + index++) = h->sps.scaling_matrix4[i1][i2 + 1];

			// Assign six Zero bytes
			for (i3 = 0; i3 < 6; i3++)
			{
				*(pCmdBuf + SCALING_TABLE_START + index++) = 0;
			}
		}
	}
	// 0 : Intra_8x8
	// 1 : Inter_8x8
	for (i1 = 0; i1 < 2; i1++)
	{
		for (i2 = 0; i2 < 64; i2 +=2)
		{
			*(pCmdBuf + SCALING_TABLE_START + index++) = h->sps.scaling_matrix8[i1][i2];
			*(pCmdBuf + SCALING_TABLE_START + index++) = h->sps.scaling_matrix8[i1][i2 + 1];

			// Assign six Zero bytes
			for (i3 = 0; i3 < 6; i3++)
			{
				*(pCmdBuf + SCALING_TABLE_START + index++) = 0;
			}
		}
	} index = 0;

	/*
	 * Table 4 : Explicite Weighting Table
	 */

	for (i1 = 0; i1 < 2; i1++)
	{
		// luma weight
		for (i2 = 0; i2 < 32; i2++)
		{
			// Y
			*(pCmdBuf + WEIGHTING_TABLE_START + index++) = (uint8_t)(h->luma_weight[i2][i1][0] & 0xFF);
			*(pCmdBuf + WEIGHTING_TABLE_START + index++) = (uint8_t)(h->luma_weight[i2][i1][0] >> 8 & 0xFF);
			*(pCmdBuf + WEIGHTING_TABLE_START + index++) = (uint8_t)(h->luma_weight[i2][i1][1] & 0xFF);

			// Assign fix Zero bytes
			for (i3 = 0; i3 < 5; i3++)
			{
				*(pCmdBuf + WEIGHTING_TABLE_START + index++) = 0;
			}
		}

		// chroma weight
		for (i2 = 0; i2 < 32; i2++)
		{
			// Cb
			*(pCmdBuf + WEIGHTING_TABLE_START + index++) = (uint8_t)(h->chroma_weight[i2][i1][0][0] & 0xFF);
			*(pCmdBuf + WEIGHTING_TABLE_START + index++) = (uint8_t)(h->chroma_weight[i2][i1][0][0] >> 8 & 0xFF);
			*(pCmdBuf + WEIGHTING_TABLE_START + index++) = (uint8_t)(h->chroma_weight[i2][i1][0][1] & 0xFF);

			// Assign fix Zero bytes
			for (i3 = 0; i3 < 5; i3++)
			{
				*(pCmdBuf + WEIGHTING_TABLE_START + index++) = 0;
			}
		}
		for (i2 = 0; i2 < 32; i2++)
		{
			// Cr
			*(pCmdBuf + WEIGHTING_TABLE_START + index++) = (uint8_t)(h->chroma_weight[i2][i1][1][0] & 0xFF);
			*(pCmdBuf + WEIGHTING_TABLE_START + index++) = (uint8_t)(h->chroma_weight[i2][i1][1][0] >> 8 & 0xFF);
			*(pCmdBuf + WEIGHTING_TABLE_START + index++) = (uint8_t)(h->chroma_weight[i2][i1][1][1] & 0xFF);

			// Assign fix Zero bytes
			for (i3 = 0; i3 < 5; i3++)
			{
				*(pCmdBuf + WEIGHTING_TABLE_START + index++) = 0;
			}
		}
	} index = 0;

	/*
	 * Table 5 : CABAC Table
	 */

	for (i2 = 0; i2 < 512; i2++)
	{
		*(pCmdBuf + CABAC_TABLE_START + index++) = h->cabac_state[cabac_map_table[i2]];
	} index = 0;

	// write slice data
	memcpy(pCmdBuf + BITSTREAM_START, s->gb.buffer + slice_data_buf_offset, s->gb.buffer_end - (s->gb.buffer + slice_data_buf_offset));

	free(pCmdBuf);
	free(pSliceParam);
}

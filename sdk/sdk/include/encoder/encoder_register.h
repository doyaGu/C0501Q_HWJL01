/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file encoder_register.h
 *
 * @author
 */

#ifndef _ENCODER_REGISTER_H_
#define _ENCODER_REGISTER_H_

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#if defined(__OPENRTOS__)
#define REG_ENCODER_BASE                  (0xD0A00000)    /* Base Register Address */
#else                                     
#define REG_ENCODER_BASE                  (0xB800)        /* Base Register Address */
#endif

//------------------------------------------------------------------------------
// HARDWARE REGISTER
//------------------------------------------------------------------------------
#define BIT_CODE_RUN                      (REG_ENCODER_BASE + 0x000)
#define BIT_CODE_DOWN                     (REG_ENCODER_BASE + 0x004)
#define BIT_INT_REQ                       (REG_ENCODER_BASE + 0x008)
#define BIT_INT_CLEAR                     (REG_ENCODER_BASE + 0x00C)
#define BIT_INT_STS                       (REG_ENCODER_BASE + 0x010)
#define BIT_CODE_RESET                    (REG_ENCODER_BASE + 0x014)
#define BIT_CUR_PC                        (REG_ENCODER_BASE + 0x018)
#define BIT_SW_RESET                      (REG_ENCODER_BASE + 0x024)
#define BIT_SW_RESET_STATUS               (REG_ENCODER_BASE + 0x034)

//------------------------------------------------------------------------------
// GLOBAL REGISTER
//------------------------------------------------------------------------------
#define BIT_CODE_BUF_ADDR                 (REG_ENCODER_BASE + 0x100)
#define BIT_WORK_BUF_ADDR                 (REG_ENCODER_BASE + 0x104)
#define BIT_PARA_BUF_ADDR                 (REG_ENCODER_BASE + 0x108)
#define BIT_BIT_STREAM_CTRL               (REG_ENCODER_BASE + 0x10C)
#define BIT_FRAME_MEM_CTRL                (REG_ENCODER_BASE + 0x110)
#define	BIT_BIT_STREAM_PARAM              (REG_ENCODER_BASE + 0x114)
#define	BIT_TEMP_BUF_ADDR                 (REG_ENCODER_BASE + 0x118)
#define BIT_RD_PTR                        (REG_ENCODER_BASE + 0x120)
#define BIT_WR_PTR                        (REG_ENCODER_BASE + 0x124)
#define BIT_AXI_SRAM_USE                  (REG_ENCODER_BASE + 0x140)
#define BIT_BYTE_POS_FRAME_START          (REG_ENCODER_BASE + 0x144)
#define BIT_BYTE_POS_FRAME_END            (REG_ENCODER_BASE + 0x148)
#define BIT_FRAME_CYCLE                   (REG_ENCODER_BASE + 0x14C)
#define	BIT_FRM_DIS_FLG                   (REG_ENCODER_BASE + 0x150)
#define BIT_BUSY_FLAG                     (REG_ENCODER_BASE + 0x160)
#define BIT_RUN_COMMAND                   (REG_ENCODER_BASE + 0x164)
#define BIT_RUN_INDEX                     (REG_ENCODER_BASE + 0x168)
#define BIT_RUN_COD_STD                   (REG_ENCODER_BASE + 0x16C)
#define BIT_INT_ENABLE                    (REG_ENCODER_BASE + 0x170)
#define BIT_INT_REASON                    (REG_ENCODER_BASE + 0x174)
#define BIT_RUN_AUX_STD                   (REG_ENCODER_BASE + 0x178)

// BIT_BIT_STREAM_CTRL
#define BS_CTRL_LITTLE_ENDIAN             (0x00000000)
#define BS_CTRL_BIG_ENDIAN                (0x00000001)
#define BS_CTRL_64BITS_ENDIAN             (0x00000000)
#define BS_CTRL_32BITS_ENDIAN             (0x00000002)
#define BS_CTRL_BUF_STS_CHK_DISABLE       (0x00000004)
#define BS_CTRL_BUF_PIC_RESET             (0x00000010)
#define BS_CTRL_DYN_BUF_ENABLE            (0x00000020)
                                          
// BIT_FRAME_MEM_CTRL                     
#define FRM_CTRL_LITTLE_ENDIAN            (0x00000000)
#define FRM_CTRL_BIG_ENDIAN               (0x00000001)
#define FRM_CTRL_64BITS_ENDIAN            (0x00000000)
#define FRM_CTRL_32BITS_ENDIAN            (0x00000002)
#define FRM_CTRL_CBCR_INTERLEAVE          (0x00000004)
#define FRM_CTRL_TILED_MAP_EN             (0x00000400)
#define FRM_CTRL_BURST_WR_BACK_EN         (0x00001000)
#define FRM_CTRL_ROT_BUF_EN               (0x00010000)
                                          
// BIT_AXI_SRAM_USE                       
#define AXI_SEC_EN                        (0x0000000F)
                                          
// BIT_INT_ENABLE                         
#define INT_BIT_BIT_BUF_FULL              (0x00008000)
#define INT_BIT_PIC_RUN                   (0x00000008)
                                          
// BIT_INT_CLEAR                          
#define INT_CLEAR                         (0x00000001)
                                          
// BIT_RUN_INDEX                          
#define RUN_CMD_SEQ_INIT                  (0x00000001)
#define RUN_CMD_SEQ_END                   (0x00000002)
#define RUN_CMD_PIC_RUN                   (0x00000003)
#define RUN_CMD_SET_FRM_BUF               (0x00000004)
#define RUN_CMD_ENOCDE_HEADER             (0x00000005)
#define RUN_CMD_ENCODE_PARA_SET           (0x00000006)
#define RUN_CMD_PARAM_CHANGE              (0x00000009)

// SW Reset command
#define VPU_SW_RESET_BPU_CORE             (0x00000008)
#define VPU_SW_RESET_BPU_BUS              (0x00000010)
#define VPU_SW_RESET_VCE_CORE             (0x00000020)
#define VPU_SW_RESET_VCE_BUS              (0x00000040)
#define VPU_SW_RESET_GDI_CORE             (0x00000080)
#define VPU_SW_RESET_GDI_BUS              (0x00000100)

//------------------------------------------------------------------------------
// [ENC SEQ INIT] COMMAND
//------------------------------------------------------------------------------
#define CMD_ENC_SEQ_BB_START              (REG_ENCODER_BASE + 0x180)
#define CMD_ENC_SEQ_BB_SIZE               (REG_ENCODER_BASE + 0x184)
#define CMD_ENC_SEQ_OPTION                (REG_ENCODER_BASE + 0x188)
#define CMD_ENC_SEQ_COD_STD               (REG_ENCODER_BASE + 0x18C)
#define CMD_ENC_SEQ_SRC_SIZE              (REG_ENCODER_BASE + 0x190)
#define CMD_ENC_SEQ_SRC_F_RATE            (REG_ENCODER_BASE + 0x194)
#define CMD_ENC_SEQ_MP4_PARA              (REG_ENCODER_BASE + 0x198)
#define CMD_ENC_SEQ_263_PARA              (REG_ENCODER_BASE + 0x19C)
#define CMD_ENC_SEQ_264_PARA              (REG_ENCODER_BASE + 0x1A0)
#define CMD_ENC_SEQ_SLICE_MODE            (REG_ENCODER_BASE + 0x1A4)
#define CMD_ENC_SEQ_GOP_NUM               (REG_ENCODER_BASE + 0x1A8)
#define CMD_ENC_SEQ_RC_PARA               (REG_ENCODER_BASE + 0x1AC)
#define CMD_ENC_SEQ_RC_BUF_SIZE           (REG_ENCODER_BASE + 0x1B0)
#define CMD_ENC_SEQ_INTRA_REFRESH         (REG_ENCODER_BASE + 0x1B4)
#define CMD_ENC_SEQ_INTRA_QP              (REG_ENCODER_BASE + 0x1C4)
#define CMD_ENC_SEQ_RC_QP_MAX             (REG_ENCODER_BASE + 0x1C8)		
#define CMD_ENC_SEQ_RC_GAMMA              (REG_ENCODER_BASE + 0x1CC)		
#define CMD_ENC_SEQ_RC_INTERVAL_MODE      (REG_ENCODER_BASE + 0x1D0)
#define CMD_ENC_SEQ_INTRA_WEIGHT          (REG_ENCODER_BASE + 0x1D4)
#define CMD_ENC_SEQ_ME_OPTION             (REG_ENCODER_BASE + 0x1D8)

#define RET_ENC_SEQ_ENC_SUCCESS           (REG_ENCODER_BASE + 0x1C0)

#define SEQ_INIT_AUD_ENABLE               (0x00000004)
#define SEQ_INIT_RC_CONST_INTRAQP_EN      (0x00000020)
#define SEQ_INIT_RC_QP_MAXSET_EN          (0x00000040)
#define SEQ_INIT_RC_GAMMASET_EN           (0x00000080)

#define SEQ_INIT_RC_ENABLE                (0x00000001)

//------------------------------------------------------------------------------
// [DEC SET FRAME BUF] COMMAND
//------------------------------------------------------------------------------
#define CMD_SET_FRAME_BUF_NUM             (REG_ENCODER_BASE + 0x180)
#define CMD_SET_FRAME_BUF_STRIDE          (REG_ENCODER_BASE + 0x184)
#define CMD_SET_FRAME_SUBSAMP_A           (REG_ENCODER_BASE + 0x188)
#define CMD_SET_FRAME_SUBSAMP_B           (REG_ENCODER_BASE + 0x18C)
#define CMD_SET_FRAME_AXI_BIT_ADDR        (REG_ENCODER_BASE + 0x190)
#define CMD_SET_FRAME_AXI_IPACDC_ADDR     (REG_ENCODER_BASE + 0x194)
#define CMD_SET_FRAME_AXI_DBKY_ADDR       (REG_ENCODER_BASE + 0x198)
#define CMD_SET_FRAME_AXI_DBKC_ADDR       (REG_ENCODER_BASE + 0x19C)
#define CMD_SET_FRAME_AXI_OVL_ADDR        (REG_ENCODER_BASE + 0x1A0)
#define CMD_SET_FRAME_AXI_BTP_ADDR        (REG_ENCODER_BASE + 0x1A4)
#define CMD_SET_FRAME_CACHE_SIZE          (REG_ENCODER_BASE + 0x1A8)
#define CMD_SET_FRAME_CACHE_CONFIG        (REG_ENCODER_BASE + 0x1AC)
#define CMD_SET_FRAME_MB_BUF_BASE         (REG_ENCODER_BASE + 0x1B0)

#define RET_SET_FRAME_SUCCESS             (REG_ENCODER_BASE + 0x1C0)

//------------------------------------------------------------------------------
// [ENC HEADER] COMMAND
//------------------------------------------------------------------------------
#define CMD_ENC_HEADER_CODE               (REG_ENCODER_BASE + 0x180)
#define CMD_ENC_HEADER_BB_START           (REG_ENCODER_BASE + 0x184)
#define CMD_ENC_HEADER_BB_SIZE            (REG_ENCODER_BASE + 0x188)
#define CMD_ENC_HEADER_FRAME_CROP_H       (REG_ENCODER_BASE + 0x18C)
#define CMD_ENC_HEADER_FRAME_CROP_V       (REG_ENCODER_BASE + 0x190)

#define RET_ENC_HEADER_SUCCESS            (REG_ENCODER_BASE + 0x1C0)

#define ENC_SPS_HEADER                    (0x00000000)
#define ENC_PPS_HEADER                    (0x00000001)
#define SPS_FRAME_CROPPING_FLAG           (0x00000004)

//------------------------------------------------------------------------------
// [ENC PARA CHANGE] COMMAND :
//------------------------------------------------------------------------------
#define CMD_ENC_SEQ_PARA_CHANGE_ENABLE	  (REG_ENCODER_BASE + 0x180)
#define CMD_ENC_SEQ_PARA_RC_FRAME_RATE    (REG_ENCODER_BASE + 0x190)

#define RET_ENC_SEQ_PARA_CHANGE_SECCESS	  (REG_ENCODER_BASE + 0x1C0)

#define FRAME_RATE_CHANGE_ENABLE          (0x00000008)

//------------------------------------------------------------------------------
// [ENC PIC RUN] COMMAND
//------------------------------------------------------------------------------
#define CMD_ENC_PIC_SRC_INDEX             (REG_ENCODER_BASE + 0x180)
#define CMD_ENC_PIC_SRC_STRIDE            (REG_ENCODER_BASE + 0x184)
#define CMD_ENC_PIC_SRC_ADDR_Y            (REG_ENCODER_BASE + 0x1A8)
#define CMD_ENC_PIC_SRC_ADDR_CB           (REG_ENCODER_BASE + 0x1AC)
#define CMD_ENC_PIC_SRC_ADDR_CR           (REG_ENCODER_BASE + 0x1B0)
#define CMD_ENC_PIC_QS                    (REG_ENCODER_BASE + 0x18C)
#define CMD_ENC_PIC_ROT_MODE              (REG_ENCODER_BASE + 0x190)
#define CMD_ENC_PIC_OPTION                (REG_ENCODER_BASE + 0x194)
#define CMD_ENC_PIC_BB_START              (REG_ENCODER_BASE + 0x198)
#define CMD_ENC_PIC_BB_SIZE        	      (REG_ENCODER_BASE + 0x19C)
#define CMD_ENC_PIC_PARA_BASE_ADDR	      (REG_ENCODER_BASE + 0x1A0)
#define CMD_ENC_PIC_SUB_FRAME_SYNC        (REG_ENCODER_BASE + 0x1A4)

#define RET_ENC_PIC_FRAME_NUM             (REG_ENCODER_BASE + 0x1C0)
#define RET_ENC_PIC_TYPE                  (REG_ENCODER_BASE + 0x1C4)
#define RET_ENC_PIC_FRAME_IDX             (REG_ENCODER_BASE + 0x1C8)
#define RET_ENC_PIC_SLICE_NUM             (REG_ENCODER_BASE + 0x1CC)
#define RET_ENC_PIC_FLAG                  (REG_ENCODER_BASE + 0x1D0)
#define RET_ENC_PIC_SUCCESS               (REG_ENCODER_BASE + 0x1D8)

//------------------------------------------------------------------------------
// GDI
//------------------------------------------------------------------------------
#define GDI_XY2_CAS_0                     (REG_ENCODER_BASE + 0x200) //0x1800)
#define GDI_XY2_CAS_F                     (REG_ENCODER_BASE + 0x23C) //0x183C)                                         
#define GDI_XY2_BA_0                      (REG_ENCODER_BASE + 0x240) //0x1840)
#define GDI_XY2_BA_1                      (REG_ENCODER_BASE + 0x244) //0x1844)
#define GDI_XY2_BA_2                      (REG_ENCODER_BASE + 0x248) //0x1848)
#define GDI_XY2_BA_3                      (REG_ENCODER_BASE + 0x24C) //0x184C)                                         
#define GDI_XY2_RAS_0                     (REG_ENCODER_BASE + 0x250) //0x1850)
#define GDI_XY2_RAS_F                     (REG_ENCODER_BASE + 0x28C) //0x188C)
#define GDI_XY2_RBC_CONFIG                (REG_ENCODER_BASE + 0x290) //0x1890)
#define GDI_RBC2_AXI_0                    (REG_ENCODER_BASE + 0x2A0) //0x18A0)
#define GDI_RBC2_AXI_1F                   (REG_ENCODER_BASE + 0x31C) //0x191C)
#define GDI_TILEDBUF_BASE                 (REG_ENCODER_BASE + 0x320) //0x1920)

#ifdef __cplusplus
}
#endif

#endif //_ENCODER_REGISTER_H_


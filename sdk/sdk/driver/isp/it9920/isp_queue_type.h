#ifndef __ISP_QUEUE_TYPE_H_9YFXLTXP_GN01_LY41_4DT0_9RHX43X7H6X3__
#define __ISP_QUEUE_TYPE_H_9YFXLTXP_GN01_LY41_4DT0_9RHX43X7H6X3__

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//				  Constant Definition
//=============================================================================
/*
 * COMMAND QUEUE Register
 */
#define ISPQ_REG_BASE_ADDR_LO     0x400
#define ISPQ_REG_BASE_ADDR_HI     0x402
#define ISPQ_REG_LENGTH           0x404
#define ISPQ_REG_WRITE_PTR_LO     0x406
#define ISPQ_REG_WRITE_PTR_HI     0x408
#define ISPQ_REG_FIRE_CMD         0x40A
#define ISPQ_REG_CONTROL          0x40C
#define ISPQ_REG_READ_PTR_LO      0x40E
#define ISPQ_REG_READ_PTR_HI      0x410
#define ISPQ_REG_STATUS           0x412

/*
 * Mask or Shift of Command Queue Register
 */
#define ISPQ_SHT_BASE_ADDR_LO     0
#define ISPQ_MSK_BASE_ADDR_LO     0xFFFF
#define ISPQ_SHT_BASE_ADDR_HI     16
#define ISPQ_MSK_BASE_ADDR_HI     0x03FF

#define ISPQ_SHT_READ_PTR_HI      16

#define ISPQ_SHT_WRITE_PTR_HI     16
#define ISPQ_MSK_WRITE_PTR_LO     0xFFFF

//=============================================================================
//				  Macro Definition
//=============================================================================
#define ISPQ_HQ_TH                0x8
#define ISPQ_UNIT_SIZE            0x400             // 1Kbyte
#define ISPQ_LENGTH               0x2               // Length with the unit of cmd_queue_unit_size
#define ISPQ_SIZE                 (ISPQ_UNIT_SIZE) * (ISPQ_LENGTH + 1)
#define ISPQ_BUFFER_SIZE          0x28
#define ISPQ_NULL_COMMAND         0x0000
#define ISPQ_DOUBLE_NULL_COMMANDS (ISPQ_NULL_COMMAND | (ISPQ_NULL_COMMAND << 16))

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================

//=============================================================================
//				  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
/*
 * Copyright (c) 2007 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as DMA error code header file.
 *
 * @author Irene Lin
 */

#ifndef IRDA_ERROR_H
#define IRDA_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

#define ERROR_IRDA_BASE            0x23000000                       //(MMP_MODULE_DNA << MMP_ERROR_OFFSET)

#define EEROR_FIR_ALLOC_DMA_BUFFER (ERROR_IRDA_BASE + 0x0001)
#define ERROR_FIR_RX_ERROR_OVERRUN (ERROR_IRDA_BASE + 0x0002)
#define ERROR_FIR_TX_UNDERRUN      (ERROR_IRDA_BASE + 0x0003)
#define ERROR_SIR_RX_TIMEPUT       (ERROR_IRDA_BASE + 0x0004)
#define ERROR_FMLSR_CRC            (ERROR_IRDA_BASE + 0x0005)
#define ERROR_FMLSR_PHY            (ERROR_IRDA_BASE + 0x0006)
#define ERROR_FMLSR_SIZE           (ERROR_IRDA_BASE + 0x0007)
#define ERROR_FMLSR_FULL           (ERROR_IRDA_BASE + 0x0008)
#define ERROR_IR_DATA_INVALID      (ERROR_IRDA_BASE + 0x0009)
#define ERROR_STFF_STS_SIZE        (ERROR_IRDA_BASE + 0x000A)
#define ERROR_STFF_STS_PHY         (ERROR_IRDA_BASE + 0x000B)
#define ERROR_STFF_STS_ORUN        (ERROR_IRDA_BASE + 0x000C)
#define ERROR_STFF_STS_CRC         (ERROR_IRDA_BASE + 0x000D)

#ifdef __cplusplus
}
#endif

#endif
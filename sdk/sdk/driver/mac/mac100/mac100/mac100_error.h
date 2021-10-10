/*
 * Copyright (c) 2010 ITE Corp. All Rights Reserved.
 */
/** @file
 *  Use as Ethernet error code header file.
 *
 * @author Irene Lin
 */

#ifndef MAC_ERROR_H
#define MAC_ERROR_H


#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Include Files
//=============================================================================

//=============================================================================
//                              Constant Definition
//=============================================================================

#define ERROR_MAC_BASE                                       0xC000

#define ERROR_MAC_PRIV_MEM_ALLOC_FAIL                        (ERROR_MAC_BASE + 0x0001)
#define ERROR_MAC_ALLOC_DESC_FAIL                            (ERROR_MAC_BASE + 0x0002)
#define ERROR_MAC_ALLOC_RX_BUF_FAIL                          (ERROR_MAC_BASE + 0x0003)
#define ERROR_MAC_RESET_TIMEOUT                              (ERROR_MAC_BASE + 0x0004)
#define ERROR_MAC_UNKNOWN_IOCTL                              (ERROR_MAC_BASE + 0x0005)
#define ERROR_MAC_INVALID_CMD                                (ERROR_MAC_BASE + 0x0006)
#define ERROR_MAC_NO_DEV                                     (ERROR_MAC_BASE + 0x0007)
#define ERROR_MAC_DEV_BUSY                                   (ERROR_MAC_BASE + 0x0008)


#ifdef __cplusplus
}
#endif

#endif

/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as MS related function header file.
 *
 * @author Irene Lin
 */
#ifndef MS_H
#define MS_H

#include "mspro/ms_type.h"
#include "mspro/ms_reg.h"

/**
 * ms_controller.c
 */
MMP_INT SMS_SetAccessAddress(MMP_UINT32 cmd, MMP_UINT32 blockAddr, MMP_UINT8 pageAddr);

MMP_INT SMS_SetAccessAddressE(MMP_UINT32 phyBlockAddr);

MMP_INT SMS_SetAccessAddressW(MMP_UINT32 cmd, MMP_UINT32 phyBlockAddr, MMP_UINT8 pageAddr, MMP_UINT32 logBlockAddr);

MMP_INT SMS_SetOverwriteFlag(MMP_UINT32 phyBlockAddr, MMP_UINT8 pageAddr, MMP_UINT8 method);

MMP_INT SMS_SetAccessCommand(MMP_UINT32 cmd);

MMP_INT SMS_ReadExtraRegister(MMP_UINT8 *overwriteFlag, MMP_UINT8 *managementFlag, MMP_UINT16 *logBlockAddr);

MMP_INT SMS_GetStatus1(MMP_UINT8 *status1);

/**
 * ms.c
 */
MMP_INT MS_SearchBootBlock(void);

MMP_INT MS_GetCapacity(void);

MMP_INT MS_BootAreaProtection(void);

MMP_INT MS_CreateLookupTable(void);

MMP_INT MS_ReadAttrib(MS_PRO_CARD_ATTRIB *attrib);

MMP_INT MS_ReadMultiSector(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8 *data);

MMP_INT MS_WriteMultiSector(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8 *data);

MMP_INT MS_WriteMultiSectorEx(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8 *data);

#endif
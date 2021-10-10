/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Used as MS pro related function header file.
 *
 * @author Irene Lin
 */
#ifndef MS_PRO_H
#define MS_PRO_H

#include "mspro/ms_type.h"
#include "mspro/ms_reg.h"

/**
 * ms_pro_controller.c
 */
MMP_INT
SMS_Pro_ExSetCmdPara(MMP_UINT32 cmd,
                     MMP_UINT16 size,
                     MMP_UINT32 addr);

/**
 * ms_pro.c
 */
MMP_INT MS_PRO_GetAttrib(MS_PRO_CARD_ATTRIB *attrib);

MMP_INT MS_PRO_ReadLongData(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8 *data);

MMP_INT MS_PRO_WriteLongData(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8 *data);

MMP_INT MS_Pro_ConfirmCpuStartup(void);

#endif
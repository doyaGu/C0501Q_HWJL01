/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  HOST controller communicate with Memory Stick PRO.
 *
 * @author Irene Lin
 */
#include "mspro/config.h"
#include "mspro/ms_hw.h"
#include "mspro/ms_common.h"

//=========================================================
/**
 * MS pro Memory Access Command.
 */
//=========================================================
MMP_INT
SMS_Pro_ExSetCmdPara(MMP_UINT32 cmd,MMP_UINT16 size,MMP_UINT32 addr)
{
    MMP_INT result = 0;

    MS_SetTpcCmdReg(MSP_TPC_EX_SET_CMD, Qualet_1_Byte, 0x7);
  
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
  
    MS_WriteDataReg(cmd);
    MS_WriteDataReg((size & 0xff00) >> 8);
    MS_WriteDataReg( size & 0xff);
    MS_WriteDataReg((addr & 0xff000000)	>> 24);
    MS_WriteDataReg((addr & 0xff0000)	>> 16);
    MS_WriteDataReg((addr & 0xff00)		>> 8);
    MS_WriteDataReg( addr & 0xff);
 
	result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMS_Pro_ExSetCmdPara() return error code 0x%08X \n", result LOG_END
    return result;
}



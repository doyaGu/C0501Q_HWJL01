/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  HOST controller communicate with Memory Stick Device.
 *
 * @author Irene Lin
 */
#include "mspro/config.h"
#include "mspro/ms_hw.h"
#include "mspro/ms_common.h"


//=========================================================
/**
 * Set the system operation command and block and page address that be accessed.
 */
//=========================================================
MMP_INT
SMS_SetAccessAddress(MMP_UINT32 cmd, MMP_UINT32 blockAddr, MMP_UINT8 pageAddr)
{
    MMP_INT result = 0;

    result = SMSC_SetRwRegAddress(0x5, 0x11, 0x04, 0x16);
    if(result)
        goto end;

    MS_SetTpcCmdReg(MS_TPC_WRITE_REG, Qualet_1_Byte, 0x5);
  
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
  
    //MS_WriteDataReg((MSCard.flags & MS_FLAGS_PARALLEL_INTERFACE) ? 0x88 : 0x80);
    MS_WriteDataReg((blockAddr & 0xFF0000) >> 16);
    MS_WriteDataReg((blockAddr & 0xFF00)   >> 8);
    MS_WriteDataReg( blockAddr & 0xFF);
    MS_WriteDataReg(cmd);
    MS_WriteDataReg((MMP_UINT32)pageAddr);
 
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMS_SetAccessAddress() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================
/**
 * For Erase Block use.
 */
//=========================================================
MMP_INT
SMS_SetAccessAddressE(MMP_UINT32 phyBlockAddr)
{
    MMP_INT result = 0;

    result = SMSC_SetRwRegAddress(0x3, 0x11, 0x04, 0x16);
    if(result)
        goto end;

    MS_SetTpcCmdReg(MS_TPC_WRITE_REG, Qualet_1_Byte, 0x3);
 
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
    
    //MS_WriteDataReg((MSCard.flags & MS_FLAGS_PARALLEL_INTERFACE) ? 0x88 : 0x80);
    MS_WriteDataReg((phyBlockAddr & 0xFF0000) >> 16);
    MS_WriteDataReg((phyBlockAddr & 0xFF00)   >> 8);
    MS_WriteDataReg( phyBlockAddr & 0xFF);
    
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMS_SetAccessAddressE() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================
/**
 * Set the system operation command and block and page address that be accessed. (For Write Access)
 */
//=========================================================
MMP_INT
SMS_SetAccessAddressW(MMP_UINT32 cmd, MMP_UINT32 phyBlockAddr, MMP_UINT8 pageAddr, MMP_UINT32 logBlockAddr)
{
    MMP_INT result = 0;
    MMP_UINT8 overwriteFlag = 0xF8;

    if(MSCard.flags & MS_FLAGS_WRITE_DATA_ERROR)
    {
        overwriteFlag = 0x98;
        MSCard.flags &= ~MS_FLAGS_WRITE_DATA_ERROR;
    }

    result = SMSC_SetRwRegAddress(0x9, 0x11, 0x04, 0x16);
    if(result)
        goto end;

    MS_SetTpcCmdReg(MS_TPC_WRITE_REG, Qualet_1_Byte, 0x9);
 
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
  
    //MS_WriteDataReg((MSCard.flags & MS_FLAGS_PARALLEL_INTERFACE) ? 0x88 : 0x80);
    MS_WriteDataReg((phyBlockAddr & 0xFF0000) >> 16);
    MS_WriteDataReg((phyBlockAddr & 0xFF00)   >> 8);
    MS_WriteDataReg( phyBlockAddr & 0xFF);
    MS_WriteDataReg(cmd);
    MS_WriteDataReg((MMP_UINT32)pageAddr);
    MS_WriteDataReg(overwriteFlag); // Overwrite flag
    MS_WriteDataReg(0xFF); // Management flag
    MS_WriteDataReg((logBlockAddr & 0xFF00)   >> 8);
    MS_WriteDataReg( logBlockAddr & 0xFF);
    
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMS_SetAccessAddressW() return error code 0x%08X \n", result LOG_END
    return result;
}

//=========================================================
/**
 * Update overwrite flag.
 */
//=========================================================
MMP_INT
SMS_SetOverwriteFlag(MMP_UINT32 phyBlockAddr, MMP_UINT8 pageAddr, MMP_UINT8 method)
{
    MMP_INT result = 0;
    MMP_UINT8 overwriteFlag = 0;

    switch(method)
    {
    case SMS_OWFLAG_BLOCK_NG:
        overwriteFlag = 0x78;  // D[7]: Block status set to 0
        LOG_INFO " Set Block %d NG!!!!!!!!!!!!!! \n", phyBlockAddr LOG_END
        break;
    case SMS_OWFLAG_PAGE_NG:
        overwriteFlag = 0xB8;  // D[6:5]: page status set to 01
        LOG_INFO " Set Block %d's Page %d NG!!!!!!!!!!!!!! \n", phyBlockAddr, pageAddr LOG_END
        break;
    case SMS_OWFLAG_DATA_ERROR:
        overwriteFlag = 0x98;  // D[6:5]: page status set to 00
        LOG_INFO " Set Block %d's Page %d Data Error!!!!!!!!!!!!!! \n", phyBlockAddr, pageAddr LOG_END
        break;
    case SMS_OWFLAG_USED_OR_UPDATING:
        overwriteFlag = 0xE8;  // D[4]: update status set to 0
        break;
    default:
        result = ERROR_MS_INVALID_OVERWRITE_METHOD;
        goto end;
    }

    result = SMSC_SetRwRegAddress(0x6, 0x11, 0x04, 0x16);
    if(result)
        goto end;

    MS_SetTpcCmdReg(MS_TPC_WRITE_REG, Qualet_1_Byte, 0x6);
 
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
  
    //MS_WriteDataReg((MSCard.flags & MS_FLAGS_PARALLEL_INTERFACE) ? 0x88 : 0x80);
    MS_WriteDataReg((phyBlockAddr & 0xFF0000) >> 16);
    MS_WriteDataReg((phyBlockAddr & 0xFF00)   >> 8);
    MS_WriteDataReg( phyBlockAddr & 0xFF);
    MS_WriteDataReg(SMS_CMD_OVERWRITE_ACCESS_MODE);
    MS_WriteDataReg((MMP_UINT32)pageAddr);
    MS_WriteDataReg(overwriteFlag);
    
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMS_SetOverwriteFlag() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================
/**
 * Set memory access command.
 */
//=========================================================
MMP_INT
SMS_SetAccessCommand(MMP_UINT32 cmd)
{
    MMP_INT result = 0;

    MS_SetTpcCmdReg(MS_TPC_SET_CMD, Qualet_1_Byte, 0x1);
    MS_WriteDataReg(cmd);
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
    
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMS_SetAccessCommand() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================
/**
 * Read extra register.
 */
//=========================================================
MMP_INT
SMS_ReadExtraRegister(MMP_UINT8* overwriteFlag, MMP_UINT8* managementFlag, MMP_UINT16* logBlockAddr)
{
    MMP_INT result = 0;
    MMP_UINT8 reg = 0;
    MMP_UINT16 logAddr = 0;

    result = SMSC_SetRwRegAddress(0x1, 0x10, 0x04, 0x16);
    if(result)
        goto end;

    MS_SetTpcCmdReg(MS_TPC_READ_REG, Qualet_1_Byte, 0x4);

    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
    
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;

    reg = (MMP_UINT8)MS_ReadDataReg();
    /** 0xF8: normal block, 0xC0: boot area, 0xE8: under updating */
    if( (reg != 0xF8) && (reg != 0xC0) && (reg != 0xE8) )
    {
        //result = ERROR_MS_OVERWRITE_FLAG_ERROR;
        LOG_ERROR "SMS_ReadExtraRegister() has error code 0x%08X, MS reg 0x16 = 0x%02X \n", result, reg LOG_END
    }
    if(overwriteFlag)
        (*overwriteFlag) = reg;

    reg = (MMP_UINT8)MS_ReadDataReg();
    if( (reg != 0xFF) && (reg != 0xFB) && (reg != 0xF7) )
    {
        //result = ERROR_MS_MANAGEMENT_FLAG_ERROR;
        LOG_ERROR "SMS_ReadExtraRegister() has error code 0x%08X, MS reg 0x17 = 0x%02X \n", result, reg LOG_END
    }
    if(managementFlag)
        (*managementFlag) = reg;

    logAddr  = (MMP_UINT16)((MS_ReadDataReg() & 0xFF) << 8);
    logAddr |= (MMP_UINT16)(MS_ReadDataReg() & 0xFF);
    if(logBlockAddr)
        (*logBlockAddr) = logAddr;

end:
    if(result)
        LOG_ERROR "SMS_ReadExtraRegister() return error code 0x%08X \n", result LOG_END
    return result;
}


//=========================================================
/**
 * MS get status register1.
 */
//=========================================================
MMP_INT
SMS_GetStatus1(MMP_UINT8* status)
{
    MMP_INT result = 0;

    result = SMSC_SetRwRegAddress(0x1, 0x10, 0x01, 0x3);
    if(result)
        goto end;

    MS_SetTpcCmdReg(MS_TPC_READ_REG, Qualet_1_Byte, 0x1);
    
    MS_SetControlReg(MS_MSK_GINT_WAIT_BS0_EN
                    |MS_MSK_INT_2STATE_EN
                    |MS_MSK_INT_CRC_CHECK_EN
                    |MS_MSK_TRANSFER_EN
                    |MS_MSK_MEMORY_STICK_EN);
    
    result = MS_WaitCmdCompleteReg();
    if(result)
        goto end;
    
    (*status) = (MMP_UINT8)MS_ReadDataReg();

end:
    if(result)
        LOG_ERROR "SMS_GetStatus1() return error code 0x%08X \n", result LOG_END
    return result;
}


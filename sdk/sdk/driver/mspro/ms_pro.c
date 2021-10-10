/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  MS pro read/write function.
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mspro/config.h"
#include "mspro/ms_pro.h"
#include "mspro/ms_common.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================

/**
 * Procedure to READ_ATTRIB.
 */
static MMP_INT MS_PRO_ReadAttrib(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8* data)
{
    MMP_INT    result = 0;
    MMP_UINT32 status = 0;
    MMP_UINT16 index = 0;
    
    result = SMS_Pro_ExSetCmdPara(SMSP_MEM_READ_ATTRIB, count, sector);
    if(result)
        goto end;

    MSC_ReadDmaPreset(data);

wait_INT:
    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_BREQ|INT_CMDNK), SMSP_TIMEOUT_READ_ATRB);
    if(result)
        goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_PRO_READ_ATTRIB_CMDNK;
        goto end;
    }
    else if(status == INT_BREQ)
    {
        // go ahead
    }
    else if(status == (INT_ERR|INT_BREQ))
    {
        LOG_ERROR " MS_PRO_ReadAttrib@ Read error occurs!! \n" LOG_END
        // go ahead
    }
    else if(status == INT_CED)
    {
        goto end;
    }
    else
    {
        LOG_ERROR " MS_PRO_ReadAttrib@ Unknown status 0x%02X \n", status LOG_END
        result = ERROR_MS_PRO_READ_UNKNOWN_STATUS;
        goto end;
    }

    result = SMSC_ReadPageData(data+(MS_PAGE_SIZE*index));
    if(result)
        goto end;
    index++;
    goto wait_INT;

end:
#if !defined(MS_WIN32_DMA)
    if(data)
    {
        MMP_INT i = 0;
        for(i=0; i<10; i++)
            LOG_DATA " %02X ", data[i] LOG_END
        LOG_DATA " \n" LOG_END

    }
#endif
    if(result)
        LOG_ERROR "MS_PRO_ReadAttrib() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

/**
 * Check all device information entrys are valid.
 */
static MMP_INT MS_PRO_CheckEntryValid(MMP_UINT8* data)
{
    MMP_INT    result = 0;
    MMP_UINT8  n = 0;
    MMP_UINT32 entryOffset= 0;
    MMP_UINT32 entryStart = 0;
    MMP_UINT32 entrySize = 0;
    MMP_UINT8  entryId = 0;

    /** check entry size */
    for(n=0; n<MSCard.entryCount; n++)
    {
        entryOffset = 16+n*12;
        entryStart = ((data[entryOffset] << 24) | 
                      (data[entryOffset+1] << 16) | 
                      (data[entryOffset+2] << 8) | 
                      data[entryOffset+3] );
        entrySize = ( (data[entryOffset+4] << 24) | 
                      (data[entryOffset+5] << 16) | 
                      (data[entryOffset+6] << 8) | 
                      data[entryOffset+7] );
        entryId = data[entryOffset+8];
        switch(entryId)
        {
        case 0x10:
            if(entrySize != 96)
            {
                result = ERROR_MS_PRO_ENTRY_SIZE_ERROR;
                goto end;
            }
            break;
        case 0x15:
            if(entrySize != 48)
            {
                result = ERROR_MS_PRO_ENTRY_SIZE_ERROR;
                goto end;
            }
            break;
        case 0x20:
            if(entrySize != 16)
            {
                result = ERROR_MS_PRO_ENTRY_SIZE_ERROR;
                goto end;
            }
            break;
        case 0x21:
            if(entrySize != 64)
            {
                result = ERROR_MS_PRO_ENTRY_SIZE_ERROR;
                goto end;
            }
            break;
        case 0x22:
            if(entrySize != 96)
            {
                result = ERROR_MS_PRO_ENTRY_SIZE_ERROR;
                goto end;
            }
            break;
        case 0x25:
            if(entrySize != 32)
            {
                result = ERROR_MS_PRO_ENTRY_SIZE_ERROR;
                goto end;
            }
            break;
        case 0x26:
            if(entrySize != 32)
            {
                result = ERROR_MS_PRO_ENTRY_SIZE_ERROR;
                goto end;
            }
            break;
        case 0x30:
            if(entrySize != 16)
            {
                result = ERROR_MS_PRO_ENTRY_SIZE_ERROR;
                goto end;
            }
            break;
        case 0x11:
            LOG_WARNING "Entry is Vender specification 1. \n" LOG_END
            break;
        case 0x12:
            LOG_WARNING "Entry is Vender specification 2. \n" LOG_END
            break;
        default:
            LOG_WARNING "Entry ID not match. \n" LOG_END
            break;
        }

        if(entryStart < 0x1A0)
        {
            result = ERROR_MS_PRO_ENTRY_START_ERROR;
            goto end;
        }
        if((entryStart+entrySize) > 0x8000)
        {
            result = ERROR_MS_PRO_ENTRY_START_ERROR;
            goto end;
        }
    }

end:
    if(result)
        LOG_ERROR "MS_PRO_CheckEntryValid() return error code 0x%08X, entry ID = 0x%02X, entryStart = 0x%08X, entrySize = %d  \n", result, entryId, entryStart, entrySize LOG_END
    return result;                                                                                           
}

/**
 * Procecure to get the device Information entry.
 */
static MMP_INT MS_PRO_GetEntry(
    MMP_UINT8*  data, 
    MMP_UINT8   deviceInfoId, 
    MMP_UINT32* start,
    MMP_UINT32* size)
{
    MMP_INT    result = 0;
    MMP_UINT8  n = 0;
    MMP_UINT32 entryOffset= 0;
    MMP_UINT32 entryStart = 0;
    MMP_UINT32 entrySize = 0;
    MMP_UINT8  entryId = 0;

read_entry:
    entryOffset = 16+n*12;
    entryStart = ((data[entryOffset] << 24) | 
                  (data[entryOffset+1] << 16) | 
                  (data[entryOffset+2] << 8) | 
                  data[entryOffset+3] );
    entrySize = ( (data[entryOffset+4] << 24) | 
                  (data[entryOffset+5] << 16) | 
                  (data[entryOffset+6] << 8) | 
                  data[entryOffset+7] );
    entryId = data[entryOffset+8];
    
    if(entryId != deviceInfoId)
    {
        n++;
        if(n < MSCard.entryCount)
            goto read_entry;
        else
        {
            result = ERROR_MS_PRO_NO_VALID_ENTRY;
            goto end;
        }
    }
    (*start) = entryStart;
    (*size) = entrySize;

end:
    if(result)
        LOG_ERROR "MS_PRO_GetEntry() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

/**
 * Procecure to Confirm Attribute Information. Big endian
 */
static MMP_INT MS_PRO_ConfirmSystemInfo(MMP_UINT8* data)
{
    MMP_INT    result = 0;
    MMP_UINT32 infoStart = 0;
    MMP_UINT32 size = 0;
    MMP_UINT8* infoAddr = MMP_NULL;
    MMP_UINT8* data1 = MMP_NULL;

    /** get system information entry address and size */
    result = MS_PRO_GetEntry(data, 0x10, &infoStart, &size);
    if(result)
    {
        result = ERROR_MS_PRO_NO_SYSTEM_INFO;
        goto end;
    }

    if((infoStart+size) >= 0x400)
    {
        MMP_UINT32 sector = (infoStart/MS_PAGE_SIZE);
        MMP_UINT32 count = ((infoStart+size)/MS_PAGE_SIZE) - sector + 1;

        LOG_WARNING " Need read data for confirm system information!!! (system information) \n" LOG_END
        LOG_WARNING " infoStart = 0x%08X, size = 0x%08X \n", infoStart, size LOG_END
        
        data1 = SYS_Malloc(MS_PAGE_SIZE*count);
        if(!data1)
        {
            result = ERROR_MS_ALLOC_SYS_INFO_MEM_FAIL;
            goto end;
        }
        LOG_INFO " MS_PRO_ConfirmSystemInfo@MS_PRO_ReadAttrib(%d,%d,0x%08X) \n", sector, count, data1 LOG_END
        result = MS_PRO_ReadAttrib(sector, (MMP_UINT16)count, data1);
        if(result)
            goto end;

        infoStart -= (sector*MS_PAGE_SIZE);
        data = data1;
        LOG_INFO " new infoStart = 0x%08X \n", infoStart LOG_END
    }

    infoAddr = data + infoStart;
    /** Memory stick sub class = 0x02 */
    if(infoAddr[0] != 0x02)
    {
        result = ERROR_MS_PRO_SYSTEM_INFO_CLASS_ERROR;
        goto end;
    }
    /** Device type */
    if(infoAddr[56] != 0x0)
    {
        if( (infoAddr[56] != 0x1) && (infoAddr[56] != 0x2) && (infoAddr[56] != 0x3) )
        {
            result = ERROR_MS_PRO_SYSTEM_INFO_DEVICE_TYPE_ERROR;
            goto end;
        }
        if(!(MSCard.flags & MS_FLAGS_WRITE_PROTECT))
        {
            LOG_WARNING " MS pro system information is read-only, but register is not read-only!! (Device Type) \n" LOG_END
            MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
        }
    }
    /** sub class => check protected status */
    if(infoAddr[46] & 0xC0)
    {
        LOG_INFO " infoAddr = 0x%08X, infoAddr[46] = 0x%02X \n", infoAddr, infoAddr[46] LOG_END
        LOG_INFO " Access Control Function ==> Read/Write-Protected Status!! \n" LOG_END
        MSCard.flags |= (MS_FLAGS_WRITE_PROTECT|MS_FLAGS_READ_PROTECT);
    }

end:
    if(data1)
        SYS_Free(data1);

    if(result)
        LOG_ERROR "MS_PRO_ConfirmSystemInfo() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

/**
 * Identify Devide informatin. Big endian
 */
static MMP_INT MS_PRO_IdentifyDeviceInfo(
    MMP_UINT8* data, 
    MS_PRO_CARD_ATTRIB* attrib)
{
    MMP_INT    result = 0;
    MMP_UINT32 start = 0;
    MMP_UINT32 size = 0;
    MMP_UINT8* addr = MMP_NULL;

    /** get system information entry address and size */
    result = MS_PRO_GetEntry(data, 0x30, &start, &size);
    if(result)
    {
        result = ERROR_MS_PRO_NO_IDENTIFY_DEVICE_INFO;
        goto end;
    }

    if((start+size) >= 0x400)
    {
        LOG_ERROR " Need read data for Identify Device Information!!! \n" LOG_END
        goto end;
    }

    addr = data + start;
    attrib->numCylinders = addr[0] << 8 | addr[1];
    attrib->numHeads     = addr[2] << 8 | addr[3];
    attrib->numSectorPerTrack = addr[8] << 8 | addr[9];

end:
    if(result)
        LOG_ERROR "MS_PRO_IdentifyDeviceInfo() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

/**
 * PBR values (FAT12/FAT16). Little endian
 */
static MMP_INT MS_PRO_GetPbrFat1216(
    MMP_UINT8* data, 
    MS_PRO_CARD_ATTRIB* attrib)
{
    MMP_INT    result = 0;
    MMP_UINT32 start = 0;
    MMP_UINT32 size = 0;
    MMP_UINT8* addr = MMP_NULL;

    /** get PBR values(FAT12/FAT16) entry address and size */
    result = MS_PRO_GetEntry(data, 0x21, &start, &size);
    if(result)
    {
        result = ERROR_MS_PRO_NO_PBR_FAT1216;
        goto end;
    }

    if((start+size) >= 0x400)
    {
        LOG_ERROR " Need read data for PBR (FAT12/FAT16)!!! \n" LOG_END
        goto end;
    }

    addr = data + start;
    attrib->mediaDescriptor = addr[21];
    attrib->numSectors = ( addr[32]         |
                           (addr[33] << 8)  |
                           (addr[34] << 16) |
                           (addr[35] << 24) );
    if(!attrib->numSectors)
    {
        attrib->numSectors = ( addr[19]  |
                               (addr[20] << 8) );
    }

end:
    if(result)
        LOG_ERROR "MS_PRO_GetPbrFat1216() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

/**
 * PBR values (FAT12/FAT16). Little endian
 */
static MMP_INT MS_PRO_GetPbrFat32(
    MMP_UINT8* data, 
    MS_PRO_CARD_ATTRIB* attrib)
{
    MMP_INT    result = 0;
    MMP_UINT32 start = 0;
    MMP_UINT32 size = 0;
    MMP_UINT8* addr = MMP_NULL;

    /** get PBR values(FAT12/FAT16) entry address and size */
    result = MS_PRO_GetEntry(data, 0x22, &start, &size);
    if(result)
    {
        result = ERROR_MS_PRO_NO_PBR_FAT32;
        goto end;
    }

    if((start+size) >= 0x400)
    {
        LOG_ERROR " Need read data for PBR (FAT32)!!! \n" LOG_END
        goto end;
    }

    addr = data + start;
    attrib->mediaDescriptor = addr[21];
    attrib->numSectors = ( addr[32]         |
                           (addr[33] << 8)  |
                           (addr[34] << 16) |
                           (addr[35] << 24) );

end:
    if(result)
        LOG_ERROR "MS_PRO_GetPbrFat32() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

#if defined(MS_SONY_FILE_SYSTEM)
/**
 * Procedure to recognize File System.
 */
static MMP_INT MS_PRO_RecognizeFileSystem(void)
{
    MMP_INT    result = 0;
    MMP_UINT8  data[MS_PAGE_SIZE] = {0};

    result = MS_PRO_ReadLongData(0, 1, data);
    if(result)
        goto end;

    if(0)
    {
        MMP_UINT i = 0;
        for(i=0; i<512; i+=16)
        {
            if(!(i%0x100))
                LOG_INFO " \n" LOG_END

            LOG_INFO "0x%04X  0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X \n",
                i,
                data[i], data[i+1], data[i+2], data[i+3], data[i+4],
                data[i+5], data[i+6], data[i+7], data[i+8], data[i+9],
                data[i+10], data[i+11], data[i+12], data[i+13], data[i+14], data[i+15] LOG_END
        }
    }
    if(data[0x1BE] != 0x80)
    {
        LOG_WARNING " MBR offset 0x1BEh = 0x%02X \n", data[0x1BE] LOG_END
        //result = ERROR_MS_FILE_SYSTEM_FAIL;
        goto check_PBR;
    }
    goto end;

check_PBR:
    if((data[0x00] != 0xE9) && (data[0x00] != 0xEB))
    {
        LOG_ERROR " PBR offset 0x00 = 0x%02X \n", data[0] LOG_END
        result = ERROR_MS_FILE_SYSTEM_FAIL;
        goto end;
    }
    if((data[0x1FE] != 0x55) || (data[0x1FF] != 0xAA))
    {
        LOG_ERROR " PBR offset 0x1FE = 0x%02X, 0x1FF = 0x%02X \n", data[0x1FE], data[0x1FF] LOG_END
        result = ERROR_MS_FILE_SYSTEM_FAIL;
        goto end;
    }

end:
    if(result)
        LOG_ERROR "MS_PRO_RecognizeFileSystem() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}
#endif

//=================================
/**
 * MS pro check and get attribute function.
 */
//=================================
MMP_INT
MS_PRO_GetAttrib(MS_PRO_CARD_ATTRIB* attrib)
{
    MMP_INT    result = 0;
    static MMP_UINT8 data[MS_PAGE_SIZE*2];
#if defined(MS_WIN32_DMA)
    MMP_UINT32 tmpAddr = 0;

    tmpAddr = itpVmemAlloc(MS_PAGE_SIZE*2);
    if(!tmpAddr)
    {
        result = ERROR_MS_ALLOC_TMP_VRAM_FAIL;
        goto end;
    }
    result = MS_PRO_ReadAttrib(0, 2, (uint8_t*)tmpAddr);
    if(result)
        goto end;

    ithReadVram(data, tmpAddr, MS_PAGE_SIZE*2);
    itpVmemFree(tmpAddr);
#else
    result = MS_PRO_ReadAttrib(0, 2, data);
    if(result)
        goto end;
#endif

    /**
     * Procedure to confirm Attribute Information.
     */
    /** check signature code */
    if((data[0] != 0xA5) || (data[1] != 0xC3))
    {
        result = ERROR_MS_PRO_SIGNATURE_CODE_ERROR;
        goto end;
    }
    /** get device information entry count */
    MSCard.entryCount = data[4]; 
    if((MSCard.entryCount < 1) || (MSCard.entryCount > 12))
    {
        result = ERROR_MS_PRO_ENTRY_COUNT_ERROR;
        goto end;
    }

    /** check all entry is valid */
    result = MS_PRO_CheckEntryValid(data);
    if(result)
        goto end;

    /** confirm system information */
    result = MS_PRO_ConfirmSystemInfo(data);
    if(result)
        goto end;

    /** get some attribute from Identify Device Information */
    result = MS_PRO_IdentifyDeviceInfo(data, attrib);
    if(result)
        goto end;

    /** get some attribute from PBR values  */
    result = MS_PRO_GetPbrFat32(data, attrib);
    if(result)
    {
        LOG_WARNING " MS_PRO_GetPbrFat32() return error code 0x%08X \n" LOG_END
        result = MS_PRO_GetPbrFat1216(data, attrib);
        if(result)
            goto end;
    }

    #if defined(MS_SONY_FILE_SYSTEM)
    if(!(MSCard.flags & MS_FLAGS_READ_PROTECT))
    {
        result = MS_PRO_RecognizeFileSystem();
        if(result)
            goto end;
    }
    #endif

end:
    if(data)
    {
        MMP_INT i = 0;
        for(i=0; i<10; i++)
            LOG_DATA " %02X ", data[i] LOG_END
        LOG_DATA " \n" LOG_END

        LOG_DATA " number of cylinders = %d \n", attrib->numCylinders LOG_END
        LOG_DATA " number of heads     = %d \n", attrib->numHeads LOG_END
        LOG_DATA " sector per track    = %d \n", attrib->numSectorPerTrack LOG_END
        LOG_DATA " number of sectors   = %d \n", attrib->numSectors LOG_END
        LOG_DATA " media descriptor    = %d \n", attrib->mediaDescriptor LOG_END
    }
    if(result)
        LOG_ERROR "MS_PRO_ReadAttrib() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

MMP_INT
MS_PRO_ReadLongData(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8* data)
{
    MMP_INT  result = 0;
    MMP_UINT32  status = 0;
    MMP_UINT16 tmpCount = count;

    result = SMS_Pro_ExSetCmdPara(SMSP_MEM_READ_DATA, count, sector);
    if(result)
        goto end;

    MSC_ReadDmaPreset(data);

wait_INT:
    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_BREQ|INT_CMDNK), SMSP_TIMEOUT_READ_DATA);
    if(result)
        goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_PRO_READ_CMDNK;
        goto end;
    }
    else if(status == INT_BREQ)
    {
        // go ahead
    }
    else if(status == (INT_ERR|INT_BREQ))
    {
        LOG_ERROR " MS_PRO_ReadLongData@ Read error occurs!! \n" LOG_END
        // go ahead
    }
    else if(status == INT_CED)
    {
        goto end;
    }
    else
    {
        LOG_ERROR " MS_PRO_ReadLongData@ Unknown status 0x%02X \n", status LOG_END
        result = ERROR_MS_PRO_READ_UNKNOWN_STATUS;
        goto end;
    }

    count--;
    result = SMSC_ReadPageData(data);
    if(result)
        goto end;
    data += MS_PAGE_SIZE;
    if(!((tmpCount-count) % 127))
        MMP_Sleep(0);
    goto wait_INT;

end:
    if(result)
        LOG_ERROR "MS_PRO_ReadLongData() return error code 0x%08X, count = %d \n", result, count LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS pro WRITE function.
 */
//=================================
MMP_INT
MS_PRO_WriteLongData(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8* data)
{
    MMP_INT result = 0;
    MMP_UINT32  status = 0;
    MMP_UINT16 tmpCount = count;
    
    result = SMS_Pro_ExSetCmdPara(SMSP_MEM_WRITE_DATA, count, sector);
    if(result)
        goto end;
    
    MSC_WriteDmaPreset(data);

wait_INT:
    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_BREQ|INT_CMDNK), SMSP_TIMEOUT_WRITE_DATA);
    if(result)
        goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_PRO_WRITE_CMDNK;
        goto end;
    }
    else if(status == INT_BREQ)
    {
        // go ahead
    }
    else if(status == INT_CED)
    {
        goto end;
    }
    else if(status == (INT_CED|INT_ERR))
    {
        result = ERROR_MS_PRO_WRITE_ERROR;
        goto end;
    }
    else if(status == (INT_CED|INT_ERR|INT_CMDNK))
    {
        LOG_WARNING " MS_PRO_WriteLongData@ status is (INT_CED|INT_ERR|INT_CMDNK), and then switch to Write-Disabled Status!! \n" LOG_END
        MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
        goto end;
        // go ahead
    }
    else
    {
        LOG_ERROR " MS_PRO_WriteLongData@ Unknown status 0x%02X \n", status LOG_END
        result = ERROR_MS_PRO_WRITE_UNKNOWN_STATUS;
        goto end;
    }

    result = SMSC_WritePageData(data);
    if(result)
        goto end;
    data += MS_PAGE_SIZE;
    goto wait_INT;

end:
    if(result)
        LOG_ERROR "MS_PRO_WriteLongData() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}


MMP_INT MS_Pro_ConfirmCpuStartup(void)
{
    MMP_INT    result = 0;
    MMP_UINT32  status = 0;
    struct timeval startT, endT;
    MMP_UINT   timeCount = 0;

    gettimeofday(&startT, NULL);
    result = SMSC_GetIntStatusEx(&status);
    if(result)
        goto end;
    while(!(status & SMS_INT_CED_S))
    {
        /** Under the normal operation, CED = 1b can be confirmed within 2 seconds,
         *  but the Memory Stick PRO can be judged as error after 10 seconds.
         */
        gettimeofday(&endT, NULL);
        if(itpTimevalDiff(&startT, &endT) > 1000)
        {
            timeCount++;
            gettimeofday(&startT, NULL);
        }
        if(timeCount>=10)
        {
            result = ERROR_MS_CONFIRM_CPU_STARTUP_FAIL1;
            LOG_ERROR " INT status = 0x%02X\n", status LOG_END
            goto end;
        }
        if(!(itpTimevalDiff(&startT, &endT) % 5))
        {
            result = SMSC_GetIntStatusEx(&status);
            if(result)
                goto end;
        }
    }

    result = SMSC_GetIntStatusEx(&status);
    if(result)
        goto end;
    if(status & SMS_INT_ERR_S)
    {
        if(status & SMS_INT_CMDNK_S)
        {
            MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
            LOG_ERROR "The media shifts to Write-disabled status even if the media can be writen and read.\n" LOG_END
            goto end;
        }
        else
        {
            result = ERROR_MS_CONFIRM_CPU_STARTUP_MEDIA_ERROR;
            goto end;
        }
    }

end:
    if(result)
        LOG_ERROR "MS_Pro_ConfirmCpuStartup() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

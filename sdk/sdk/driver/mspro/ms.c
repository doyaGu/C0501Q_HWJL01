/*
 * Copyright (c) 2007 SMedia Corp. All Rights Reserved.
 */
/** @file
 *  MS read/write function.
 *
 * @author Irene Lin
 */

//=============================================================================
//                              Include Files
//=============================================================================
#include "mspro/config.h"
#include "mspro/ms.h"
#include "mspro/ms_common.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
enum
{
    MS_PHY_START,
    MS_PHY_END,
    MS_LOG_START,
    MS_LOG_END
};

//=============================================================================
//                              Macro
//=============================================================================
#define MS_GetPageStatus(x)    ((x & SMS_MSK_PAGE_STATUS) >> 5)

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================
extern MEM_STICK_CARD_STRUCT MSCard;
static MMP_UINT16 lookupTable[8192];
static MMP_UINT16 blockConfig[16][4] =
{
    {   0,  511,    0,  493}, 
    { 512, 1023,  494,  989},
    {1024, 1535,  990, 1485},
    {1536, 2047, 1486, 1981},
    {2048, 2559, 1982, 2477},
    {2560, 3071, 2478, 2973},
    {3072, 3583, 2974, 3469},
    {3584, 4095, 3470, 3965},
    {4096, 4607, 3966, 4461},
    {4608, 5119, 4462, 4957},
    {5120, 5631, 4958, 5453},
    {5632, 6143, 5454, 5949},
    {6144, 6655, 5950, 6445},
    {6656, 7167, 6446, 6941},
    {7168, 7679, 6942, 7437},
    {7680, 8191, 7438, 7933}
};
static MMP_UINT16 freeTable[16][512];
static MMP_UINT16 freeBlockNum[16];
static MMP_UINT16 defectBlock[256];

//=============================================================================
//                              Public Function Definition
//=============================================================================


//=================================
/**
 * MS Overwrite Extra Data.
 */
//=================================
static MMP_INT
MS_OverwriteOverwriteFlag(MMP_UINT32 phyBlockAddr, MMP_UINT8 pageAddr, MMP_UINT8 method)
{
    MMP_INT  result = 0;
    MMP_UINT32 status = 0;

    result = SMS_SetOverwriteFlag(phyBlockAddr, pageAddr, method);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_BLOCK_WRITE);
    if(result)
        goto end;

    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_CMDNK|INT_ERR), SMS_TIMEOUT_BLOCK_WRITE);
    if(result)
        goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_OVERWRITE_CMDNK;
        goto end;
    }
    else if(status == INT_CED)
    {
        goto end;
    }
    else if(status == (INT_CED|INT_ERR))
    {
        MMP_UINT8 status1 = 0;
        LOG_ERROR " MS_OverwriteOverwriteFlag@ Flash Write Error occured!! \n" LOG_END
        result = ERROR_MS_OVERWRITE_ERROR;
        goto end;
    }
    else
    {
        LOG_ERROR " INT Status = 0x%02X \n", status LOG_END
        result = ERROR_MS_OVERWRITE_UNKNOWN_STATUS;
        goto end;
    }

end:
    if(result)
        LOG_ERROR "MS_OverwriteOverwriteFlag() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS get extra data.
 */
//=================================
static MMP_INT
MS_GetExtraData(MMP_UINT32 phyBlockAddr, MMP_UINT8* overwriteFlag, MMP_UINT8* managementFlag, MMP_UINT16* logBlockAddr)
{
    MMP_INT  result = 0;
    MMP_UINT32 status = 0;

    result = SMS_SetAccessAddress(SMS_CMD_EXTRA_DATA_ACCESS_MODE, phyBlockAddr, 0);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_BLOCK_READ);
    if(result)
        goto end;

    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_CMDNK|INT_ERR), SMS_TIMEOUT_BLOCK_READ);
    if(result)
        goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_GET_EXTRA_DATA_CMDNK;
        goto end;
    }
    else if(status == INT_CED)
    {
        result = SMS_ReadExtraRegister(overwriteFlag, managementFlag, logBlockAddr);
        if(result)
            goto end;
        goto end;
    }
    else if(status == (INT_CED|INT_ERR))
    {
        MMP_UINT8 status1 = 0;
        LOG_ERROR " MS_GetExtraData@ Flash Read Error occured!! \n" LOG_END
        result = SMS_GetStatus1(&status1);
        if(result)
            goto end;
        if(status1 & (SMS_MSK_UNABLE_CORRECT_EXTRA_DATA|SMS_MSK_UNABLE_CORRECT_FLAG))
        {
            LOG_ERROR " Status1 = 0x%02X, An uncorrectable error occurs. \n", status1 LOG_END
            result = ERROR_MS_UNABLE_CORRECT_DATA_FLAG;
            goto end;
        }

        result = SMS_ReadExtraRegister(overwriteFlag, managementFlag, logBlockAddr);
        if(result)
            goto end;
    }
    else
    {
        LOG_ERROR " INT Status = 0x%02X \n", status LOG_END
        result = ERROR_MS_GET_EXTRA_DATA_UNKNOWN_STATUS;
        goto end;
    }

end:
    if(result)
        LOG_ERROR "SMS_GetExtraData() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}


//=================================
/**
 * MS write extra data.
 */
//=================================
static MMP_INT
MS_WriteExtraData(MMP_UINT32 phyBlockAddr, MMP_UINT8 pageAddr, MMP_UINT32 logBlockAddr)
{
    MMP_INT  result = 0;
    MMP_UINT32 status = 0;

    result = SMS_SetAccessAddressW(SMS_CMD_EXTRA_DATA_ACCESS_MODE, phyBlockAddr, 0, logBlockAddr);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_BLOCK_WRITE);
    if(result)
        goto end;

    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_CMDNK|INT_ERR), SMS_TIMEOUT_BLOCK_WRITE);
    if(result)
        goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_WRITE_EXTRA_DATA_CMDNK;
        goto end;
    }
    else if(status == INT_CED)
    {
        goto end;
    }
    else if(status == (INT_CED|INT_ERR))
    {
        LOG_ERROR " MS_WriteExtraData@ Flash Write Error occured!! \n" LOG_END
        result = ERROR_MS_FLASH_WRITE_ERROR_OCCURRED;
        goto end;
    }
    else
    {
        LOG_ERROR " INT Status = 0x%02X \n", status LOG_END
        result = ERROR_MS_WRITE_EXTRA_DATA_UNKNOWN_STATUS;
        goto end;
    }

end:
    if(result)
        LOG_ERROR "MS_WriteExtraData() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}


//=================================
/**
 * Read page 0 of block for search boot block.
 * This function is same with MS_ReadPage(). Just the interface is difference.
 */
//=================================
static MMP_INT
MS_ReadPage0(MMP_UINT32 phyBlockAddr, MMP_UINT8* overwriteFlag, MMP_UINT8* managementFlag, MMP_UINT16* logBlockAddr, MMP_UINT8* data)
{
    MMP_INT    result = 0;
    MMP_UINT32  status = 0;

    result = SMS_SetAccessAddress(SMS_CMD_SINGLE_PAGE_ACCESS_MODE, phyBlockAddr, 0);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_BLOCK_READ);
    if(result)
        goto end;

wait_INT:
    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_BREQ|INT_CMDNK), SMS_TIMEOUT_BLOCK_READ);
        if(result)
            goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_READ_PAGE_CMDNK;
        goto end;
    }
    else if(status == INT_BREQ)
    {
        goto wait_INT;
    }
    else if(status == (INT_CED|INT_BREQ))
    {
        // goto step 7
    }
    else if(status == (INT_CED|INT_ERR|INT_BREQ))
    {
        MMP_UINT8 status1 = 0;
        LOG_ERROR " MS_ReadPage0@ Flash Read Error occured!! \n" LOG_END
        result = SMS_GetStatus1(&status1);
        if(result)
            goto end;
        if(status1 & (SMS_MSK_UNABLE_CORRECT_EXTRA_DATA|SMS_MSK_UNABLE_CORRECT_FLAG|SMS_MSK_UNABLE_CORRECT_DATA))
        {
            LOG_ERROR " Status1 = 0x%02X \n", status1 LOG_END

            /** set up page status to NG  */
            {
                result = SMS_ReadExtraRegister(overwriteFlag, managementFlag, logBlockAddr);
                if(result)
                {
                    LOG_ERROR "MS_ReadPage0@SMS_ReadExtraRegister() return error code 0x%08X\n", result LOG_END
                    goto end;
                }

                /** TEST SPEC: Check011-1, 2 */
                if( (MS_GetPageStatus(*overwriteFlag) == SMS_PAGE_STATUS_OK) && !(MSCard.flags & MS_FLAGS_WRITE_PROTECT) )
                {
                    result = MS_OverwriteOverwriteFlag(phyBlockAddr, 0, SMS_OWFLAG_PAGE_NG);
                    if(result)
                    {
                        LOG_ERROR "MS_ReadPage0@MS_OverwriteOverwriteFlag() return error code 0x%08X\n", result LOG_END
                        goto end;
                    }
                }
            }
            result = ERROR_MS_UNABLE_CORRECT_DATA_FLAG;
            goto end;
        }
        // goto step 7
    }
    else
    {
        LOG_ERROR " INT Status = 0x%02X \n", status LOG_END
        result = ERROR_MS_READ_PAGE_UNKNOWN_STATUS;
        goto end;
    }

    /** Step 7 */
    result = SMS_ReadExtraRegister(overwriteFlag, managementFlag, logBlockAddr);
    if(result)
    {
        //if( (result != ERROR_MS_OVERWRITE_FLAG_ERROR) && (result != ERROR_MS_MANAGEMENT_FLAG_ERROR) )
            goto end;
    }
    /** Step 8 */
    MSC_ReadDmaPreset(data);
    result = SMSC_ReadPageData(data);
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "MS_ReadPage0() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS read one page function.
 */
//=================================
static MMP_INT
MS_ReadPage(MMP_UINT32 phyBlockAddr, MMP_UINT8 pageAddr, MMP_UINT8* data)
{
    MMP_INT    result = 0;
    MMP_UINT32  status = 0;
    MMP_UINT16 extraData = 0;

    result = SMS_SetAccessAddress(SMS_CMD_SINGLE_PAGE_ACCESS_MODE, phyBlockAddr, pageAddr);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_BLOCK_READ);
    if(result)
        goto end;

wait_INT:
    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_BREQ|INT_CMDNK), SMS_TIMEOUT_BLOCK_READ);
        if(result)
            goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_READ_PAGE_CMDNK;
        goto end;
    }
    else if(status == INT_BREQ)
    {
        goto wait_INT;
    }
    else if(status == (INT_CED|INT_BREQ))
    {
        // goto step 7
    }
    else if(status == (INT_CED|INT_ERR|INT_BREQ))
    {
        MMP_UINT8 status1 = 0;
        LOG_ERROR " MS_ReadPage@ Flash Read Error occured!! \n" LOG_END
        result = SMS_GetStatus1(&status1);
        if(result)
            goto end;
        if(status1 & (SMS_MSK_UNABLE_CORRECT_EXTRA_DATA|SMS_MSK_UNABLE_CORRECT_FLAG|SMS_MSK_UNABLE_CORRECT_DATA))
        {
            LOG_ERROR " Status1 = 0x%02X \n", status1 LOG_END

            /** set up page status to NG  */
            {
                MMP_UINT8 overwriteFlag = 0;
                result = SMS_ReadExtraRegister(&overwriteFlag, MMP_NULL, MMP_NULL);
                if(result)
                {
                    LOG_ERROR "MS_ReadPage0@SMS_ReadExtraRegister() return error code 0x%08X\n", result LOG_END
                    goto end;
                }

                /** TEST SPEC: Check011-1, 2 */
                if( (MS_GetPageStatus(overwriteFlag) == SMS_PAGE_STATUS_OK) && !(MSCard.flags & MS_FLAGS_WRITE_PROTECT) )
                {
                    MMP_INT result1 = 0;
                    /** TEST SPEC: Check011-5 */
                    result1 = MS_OverwriteOverwriteFlag(phyBlockAddr, pageAddr, SMS_OWFLAG_PAGE_NG);
                    if(result1)
                    {
                        LOG_ERROR "MS_ReadPage@MS_OverwriteOverwriteFlag() return error code 0x%08X\n", result1 LOG_END
                        goto end;
                    }
                }
            }
            result = ERROR_MS_UNABLE_CORRECT_DATA_FLAG;
            goto end;
        }
        // goto step 7
    }
    else
    {
        LOG_ERROR " INT Status = 0x%02X \n", status LOG_END
        result = ERROR_MS_READ_PAGE_UNKNOWN_STATUS;
        goto end;
    }

    /** Step 7 */
    result = SMS_ReadExtraRegister(MMP_NULL, MMP_NULL, &extraData);
    if(result)
        goto end;
    /** Step 8 */
    MSC_ReadDmaPreset(data);
    result = SMSC_ReadPageData(data);
    if(result)
        goto end;

end:
    if(result)
        LOG_ERROR "SMS_ReadPage() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS read block function.
 */
//=================================
static MMP_INT
MS_ReadBlock(MMP_UINT32 phyBlockAddr, MMP_UINT8 pageAddr, MMP_UINT16 count, MMP_UINT8* data)
{
    MMP_INT    result = 0;
    MMP_UINT32  status = 0;
    MMP_UINT16 totalCount = count;
    MMP_UINT16 extraData = 0;
    MMP_UINT32 sTimeout = SMS_TIMEOUT_BLOCK_READ;
    LOG_ENTER "[MS_ReadBlock] phyBlockAddr = %d, pageAddr = %d, count = %d \n", phyBlockAddr, pageAddr, count LOG_END

    if(count == 1)
    {
        result = MS_ReadPage(phyBlockAddr, pageAddr, data);
        goto end;
    }

    result = SMS_SetAccessAddress(SMS_CMD_BLOCK_ACCESS_MODE, phyBlockAddr, pageAddr);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_BLOCK_READ);
    if(result)
        goto end;

    MSC_ReadDmaPreset(data);

wait_INT:
    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_BREQ|INT_CMDNK), sTimeout);
    if(result)
        goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_READ_BLOCK_CMDNK;
        goto end;
    }
    else if(status == INT_BREQ)
    {
        // go ahead
    }
    #if defined(WR_TMP_STATUS)
    else if(status == INT_CED)
    {
        printf(" @");
        goto wait_INT;
    }
    #endif
    else if(status == (INT_ERR|INT_BREQ))
    {
        LOG_ERROR " MS_ReadBlock@ Correctable Flash Read Error!! \n" LOG_END
        // go ahead
    }
    else if(status == (INT_CED|INT_BREQ))
    {
        goto last_page;
    }
    else if(status == (INT_CED|INT_ERR|INT_BREQ))
    {
        MMP_UINT8 status1 = 0;
        LOG_ERROR " MS_ReadBlock@ Flash Read Error occured. status 0x%02X \n", status LOG_END
        result = SMS_GetStatus1(&status1);
        if(result)
            goto end;
        if(status1 & (SMS_MSK_UNABLE_CORRECT_EXTRA_DATA|SMS_MSK_UNABLE_CORRECT_FLAG|SMS_MSK_UNABLE_CORRECT_DATA))
        {
            MMP_UINT8 overwriteFlag = 0;

            LOG_ERROR " Status1 = 0x%02X \n", status1 LOG_END

            /** set up page status to NG  */
            result = SMS_ReadExtraRegister(&overwriteFlag, MMP_NULL, MMP_NULL);
            if(result)
            {
                LOG_ERROR "MS_ReadBlock@SMS_ReadExtraRegister() return error code 0x%08X\n", result LOG_END
                goto end;
            }

            /** TEST SPEC: Check011-1, 2 */
            if( (MS_GetPageStatus(overwriteFlag) == SMS_PAGE_STATUS_OK) && !(MSCard.flags & MS_FLAGS_WRITE_PROTECT) )
            {
                /** TEST SPEC: Check011-5 */
                result = MS_OverwriteOverwriteFlag(phyBlockAddr, (pageAddr+totalCount-count), SMS_OWFLAG_PAGE_NG);
                if(result)
                {
                    LOG_ERROR "MS_ReadBlock@MS_OverwriteOverwriteFlag() return error code 0x%08X, pageAddr = %d, totalCount = %d, count = %d\n", result, pageAddr, totalCount, count LOG_END
                    goto end;
                }
            }

            result = ERROR_MS_UNABLE_CORRECT_DATA_FLAG;
        }
        goto last_page;
    }
    else
    {
        LOG_ERROR " MS_ReadBlock@ Unknown status 0x%02X \n", status LOG_END
        result = ERROR_MS_READ_BLOCK_UNKNOWN_STATUS;
        goto end;
    }

    count--;
    if(count > 0)
    {
        result = SMS_ReadExtraRegister(MMP_NULL, MMP_NULL, &extraData);
        if(result)
            goto end;
        result = SMSC_ReadPageData(data);
        if(result)
            goto end;
        data += MS_PAGE_SIZE;
        goto wait_INT;
    }
    else
    {
        result = SMS_SetAccessCommand(SMS_MEM_BLOCK_END);
        if(result)
            goto end;
        sTimeout = SMS_TIMEOUT_BLOCK_END_R;
        goto wait_INT;
    }

last_page:
    result = SMS_ReadExtraRegister(MMP_NULL, MMP_NULL, &extraData);
    if(result)
        goto end;
    result = SMSC_ReadPageData(data);
    if(result)
        goto end;

end:
    LOG_LEAVE "[MS_ReadBlock] Leave \n" LOG_END
    if(result)
        LOG_ERROR "MS_ReadBlock() return error code 0x%08X, count = %d, INT status = 0x%02X \n", result, count, status LOG_END
    return result;                                                                                           
}



//=================================
/**
 * MS Erase block function.
 */
//=================================
static MMP_INT
MS_EraseBlock(MMP_UINT32 phyBlockAddr)
{
    MMP_INT    result = 0;
    MMP_UINT32  status = 0;
    LOG_ENTER "[MS_EraseBlock] phyBlockAddr = %d \n", phyBlockAddr LOG_END

    if(MSCard.flags & MS_FLAGS_WRITE_PROTECT)
        goto end;

    result = SMS_SetAccessAddressE(phyBlockAddr);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_ERASE);
    if(result)
        goto end;

    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_CMDNK), SMS_TIMEOUT_ERASE);
        if(result)
            goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_ERASE_BLOCK_CMDNK;
        goto end;
    }
    else if(status == INT_CED)
    {
        goto end;
    }
    else if(status == (INT_CED|INT_ERR))
    {
        result = ERROR_MS_ERASE_BLOCK_FAIL;
        goto end;
    }
    else
    {
        result = ERROR_MS_ERASE_BLOCK_UNKNOWN_STATUS;
        goto end;
    }

end:
    LOG_LEAVE "[MS_EraseBlock] Leave \n" LOG_END
    if(result)
        LOG_ERROR "MS_EraseBlock() return error code 0x%08X, INT status = 0x%02X \n", result, status LOG_END
    return result;                                                                                           
}

//=================================
/**
 * Search the block to be updated and update the new mapping.
 */
//=================================
static MMP_INT
MS_RemoveLastFreeBlock(void)
{
    MMP_INT    result = 0;
    MMP_UINT32 i = 0;

    LOG_ENTER "[MS_RemoveLastFreeBlock] Enter \n" LOG_END

    LOG_INFO " Remove physical block addr %d from free Table. \n", freeTable[MSCard.lastUpdateSegment][MSCard.lastUpdateIndex] LOG_END
    LOG_INFO " freeBlockNum[%d] = %d (Before) \n", MSCard.lastUpdateSegment, freeBlockNum[MSCard.lastUpdateSegment] LOG_END
    for(i=MSCard.lastUpdateIndex; i<(freeBlockNum[MSCard.lastUpdateSegment]-2); i++)
        freeTable[MSCard.lastUpdateSegment][i] = freeTable[MSCard.lastUpdateSegment][i+1];

    freeBlockNum[MSCard.lastUpdateSegment]--;
    LOG_INFO " freeBlockNum[%d] = %d (After) \n", MSCard.lastUpdateSegment, freeBlockNum[MSCard.lastUpdateSegment] LOG_END

    if(MSCard.lastUpdateSegment==(MSCard.segmentNum-1))
    {
        if(freeBlockNum[MSCard.lastUpdateSegment] < 2)
        {
            MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
            goto end;
        }
    }

    if(freeBlockNum[MSCard.lastUpdateSegment] < 1)
    {
        MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
        goto end;
    }

end:
    LOG_LEAVE "[MS_RemoveLastFreeBlock] Leave \n" LOG_END
    if(result)
        LOG_ERROR "MS_RemoveLastFreeBlock() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS copy page function.
 */
//=================================
static MMP_INT
MS_CopyPage(MMP_UINT32 phySrcBlockAddr, MMP_UINT32 phyDstBlockAddr, MMP_UINT8 pageAddr)
{
    MMP_INT    result = 0;
    MMP_UINT32  status = 0;
    MMP_UINT16 logBlockAddr = 0;
    MMP_UINT8  overwriteFlag = 0;

    /**
     * Read source page data to device internal buffer.
     */
    result = SMS_SetAccessAddress(SMS_CMD_SINGLE_PAGE_ACCESS_MODE, phySrcBlockAddr, pageAddr);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_BLOCK_READ);
    if(result)
        goto end;

wait_INT:
    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_BREQ|INT_CMDNK), SMS_TIMEOUT_BLOCK_READ);
        if(result)
            goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_READ_PAGE_CMDNK;
        goto end;
    }
    else if(status == INT_BREQ)
    {
        goto wait_INT;
    }
    #if defined(WR_TMP_STATUS)
    else if(status == INT_CED)
    {
        printf(" @");
        goto wait_INT;
    }
    #endif
    else if(status == (INT_CED|INT_BREQ))
    {
        /** Step 6 */
        result = SMS_ReadExtraRegister(&overwriteFlag, MMP_NULL, &logBlockAddr);
        if(result)
            goto end;
    }
    else if(status == (INT_CED|INT_ERR|INT_BREQ))
    {
        /** Step 7 */
        MMP_UINT8 data[MS_PAGE_SIZE] = {0};
        MMP_UINT8 status1 = 0;
        LOG_WARNING " MS_CopyPage@ Flash Read Error occured!! \n" LOG_END

        result = SMS_ReadExtraRegister(&overwriteFlag, MMP_NULL, &logBlockAddr);
        if(result)
            goto end;

        result = SMS_GetStatus1(&status1);
        if(result)
            goto end;
        if(status1 & (SMS_MSK_UNABLE_CORRECT_EXTRA_DATA|SMS_MSK_UNABLE_CORRECT_FLAG|SMS_MSK_UNABLE_CORRECT_DATA))
        {
            /** Step 8 */
            MSC_ReadDmaPreset(data);
            result = SMSC_ReadPageData(data);
            if(result)
                goto end;

            LOG_WARNING " Status1 = 0x%02X, Uncorrectable error!! Set source block's BS to 0(NG) and PS to 1(NG).\n", status1 LOG_END

            /** TEST SPEC: Check011-6 */
            result = MS_OverwriteOverwriteFlag(phySrcBlockAddr, 0, SMS_OWFLAG_BLOCK_NG);
            if(result)
            {
                LOG_ERROR "MS_CopyPage@MS_OverwriteOverwriteFlag() return error code 0x%08X, set source block %d BS NG. \n", result, phySrcBlockAddr LOG_END
                goto end;
            }
            result = MS_RemoveLastFreeBlock();
            if(result)
                goto end;

            /** TEST SPEC: Check011-8 */
            result = MS_OverwriteOverwriteFlag(phySrcBlockAddr, pageAddr, SMS_OWFLAG_PAGE_NG);
            if(result)
            {
                LOG_ERROR "MS_CopyPage@MS_OverwriteOverwriteFlag() return error code 0x%08X, set source block %d PS NG. \n", result LOG_END
                goto end;
            }
            MSCard.flags |= MS_FLAGS_BLOCK_HAS_PAGE_NG;
            /** TEST SPEC: Check011-11 */
            MSCard.flags |= MS_FLAGS_WRITE_DATA_ERROR;
            logBlockAddr = (MMP_UINT16)MSCard.lastLogBlockAddr;

            MSC_WriteDmaPreset(data);
            result = SMSC_WritePageData(data);
            if(result)
                goto end;
        }
        else
        {
            /** Step 9 */
            MSC_ReadDmaPreset(data);
            result = SMSC_ReadPageData(data);
            if(result)
                goto end;
            /** Step 10 */
            MSC_WriteDmaPreset(data);
            result = SMSC_WritePageData(data);
            if(result)
                goto end;
            LOG_WARNING " MS_CopyPage() has corrected page, phySrcBlockAddr = %d, pageAddr = %d \n", phySrcBlockAddr , pageAddr LOG_END
        }
    }
    else
    {
        LOG_ERROR " ERROR_MS_READ_PAGE_UNKNOWN_STATUS, INT Status = 0x%02X \n", status LOG_END
        result = ERROR_MS_READ_PAGE_UNKNOWN_STATUS;
        goto end;
    }

    if(!(MSCard.flags & MS_FLAGS_WRITE_DATA_ERROR))
    {
        if(MS_GetPageStatus(overwriteFlag) == SMS_PAGE_STATUS_DATA_ERROR)
        {
            MSCard.flags |= MS_FLAGS_WRITE_DATA_ERROR;
            logBlockAddr = (MMP_UINT16)MSCard.lastLogBlockAddr;
        }
    }

    /**
     * Write device internal buffer data to destination page.
     */
    /** Step 11 ~ 14 */
    result = SMS_SetAccessAddressW(SMS_CMD_SINGLE_PAGE_ACCESS_MODE, phyDstBlockAddr, pageAddr, logBlockAddr);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_BLOCK_WRITE);
    if(result)
        goto end;

    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_CMDNK), SMS_TIMEOUT_BLOCK_WRITE);
        if(result)
            goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_WRITE_PAGE_CMDNK;
        goto end;
    }
    else if(status == INT_CED)
    {
        goto end;
    }
    else if(status == (INT_CED|INT_ERR))
    {
        result = ERROR_MS_WRITE_PAGE_ERROR;
        goto end;
    }
    else
    {
        LOG_ERROR " ERROR_MS_WRITE_PAGE_UNKNOWN_STATUS, INT Status = 0x%02X \n", status LOG_END
        result = ERROR_MS_WRITE_PAGE_UNKNOWN_STATUS;
        goto end;
    }

end:
    if(result)
        LOG_ERROR "MS_CopyPage() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS copy block function.
 */
//=================================
static MMP_INT
MS_CopyBlock(MMP_UINT32 phySrcBlockAddr, MMP_UINT32 phyDstBlockAddr, MMP_UINT8 pageAddr, MMP_UINT16 count)
{
    MMP_INT    result = 0;
    MMP_UINT8  i = 0;

    if( ((MMP_UINT32)pageAddr+(MMP_UINT32)count) > MSCard.pageNum)
    {
        result = ERROR_MS_COPY_BLOCK_ERROR_COUNT;
        goto end;
    }

    for(i=0; i<count; i++)
    {
        result = MS_CopyPage(phySrcBlockAddr, phyDstBlockAddr, (pageAddr+i));
        if(result)
            goto end;
    }

end:
    if(result)
        LOG_ERROR "MS_CopyBlock() return error code 0x%08X, i = %d \n", result, i LOG_END
    return result;                                                                                           
}


//=================================
/**
 * MS write one page function.
 */
//=================================
static MMP_INT
MS_WritePage(MMP_UINT32 logBlockAddr, MMP_UINT32 phyBlockAddr, MMP_UINT8 pageAddr, MMP_UINT8* data)
{
    MMP_INT    result = 0;
    MMP_UINT32  status = 0;
    MMP_UINT16 extraData = 0;

    result = SMS_SetAccessAddressW(SMS_CMD_SINGLE_PAGE_ACCESS_MODE, phyBlockAddr, pageAddr, logBlockAddr);
    if(result)
        goto end;

    MSC_WriteDmaPreset(data);
    result = SMSC_WritePageData(data);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_BLOCK_WRITE);
    if(result)
        goto end;

    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_BREQ|INT_CMDNK), SMS_TIMEOUT_BLOCK_WRITE);
    if(result)
        goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_WRITE_PAGE_CMDNK;
        goto end;
    }
    else if(status == INT_CED)
    {
        goto end;
    }
    else if(status == (INT_CED|INT_ERR))
    {
        result = ERROR_MS_WRITE_PAGE_ERROR;
        goto end;
    }
    else
    {
        LOG_ERROR " INT Status = 0x%02X \n", status LOG_END
        result = ERROR_MS_WRITE_PAGE_UNKNOWN_STATUS;
        goto end;
    }

end:
    if(result)
        LOG_ERROR "MS_WritePage() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS write block function.
 */
//=================================
static MMP_INT
MS_WriteBlock(MMP_UINT32 logBlockAddr, MMP_UINT8 pageAddr, MMP_UINT16 count, MMP_UINT8* data)
{
    MMP_INT    result = 0;
    MMP_UINT32 phyBlockAddr = lookupTable[logBlockAddr];
    MMP_UINT32  status = 0;
    MMP_UINT32 sTimeout = SMS_TIMEOUT_BLOCK_WRITE;
    LOG_ENTER "[MS_WriteBlock] logBlockAddr = %d, pageAddr = %d, count = %d, phyBlockAddr = %d \n", logBlockAddr, pageAddr, count, phyBlockAddr LOG_END

    if(count == 1)
    {
        result = MS_WritePage(logBlockAddr, phyBlockAddr, pageAddr, data);
        goto end;
    }

    result = SMS_SetAccessAddressW(SMS_CMD_BLOCK_ACCESS_MODE, phyBlockAddr, pageAddr, logBlockAddr);
    if(result)
        goto end;

    result = SMS_SetAccessCommand(SMS_MEM_BLOCK_WRITE);
    if(result)
        goto end;

    MSC_WriteDmaPreset(data);

wait_INT:
    result = SMSC_WaitIntStatus(&status, (INT_CED|INT_ERR|INT_BREQ|INT_CMDNK), sTimeout);
        if(result)
            goto end;

    if(status == INT_CMDNK)
    {
        result = ERROR_MS_WRITE_BLOCK_CMDNK;
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
        LOG_ERROR " MS_WriteBlock@ Flash Write Error occurred -> Error termination \n" LOG_END
        result = ERROR_MS_WRITE_BLOCK_ERROR_END;
        goto end;
    }
    else
    {
        result = ERROR_MS_WRITE_BLOCK_UNKNOWN_STATUS;
        goto end;
    }

    if(count > 0)
    {
        count--;
        result = SMSC_WritePageData(data);
        if(result)
            goto end;
        data += MS_PAGE_SIZE;
        goto wait_INT;
    }
    else
    {
        LOG_DEBUG " MS_WriteBlock() Set BLOCK_END \n" LOG_END
        result = SMS_SetAccessCommand(SMS_MEM_BLOCK_END);
        if(result)
            goto end;
        sTimeout = SMS_TIMEOUT_BLOCK_END_W;
        goto wait_INT;
    }

end:
    LOG_LEAVE "[MS_WriteBlock] Leave \n" LOG_END
    if(result)
        LOG_ERROR "MS_WriteBlock() return error code 0x%08X, count = %d \n", result, count LOG_END
    return result;                                                                                           
}

//=================================
/**
 * Search unused block for Boot Area Protection Process.
 */
//=================================
static MMP_INT
MS_FindUnusedBlock(void)
{
    MMP_INT    result = 0;
    MMP_UINT8  overwriteFlag = 0;
    MMP_UINT8  managementFlag = 0;
    MMP_UINT16 logBlockAddr = 0;
    MMP_UINT32 p = 0;
    MMP_UINT32 startBlock = (MSCard.unusedBlockIndex > MSCard.bootBlock1Index) ? (MSCard.unusedBlockIndex+1) : (MSCard.bootBlock1Index+1);
    MMP_UINT32 endBlock = (MMP_UINT32)blockConfig[0][MS_PHY_END];

    LOG_ENTER "[MS_FindUnusedBlock] Enter \n" LOG_END

    for(p=startBlock; p<=endBlock; p++)
    {
        result = MS_GetExtraData(p, &overwriteFlag, &managementFlag, &logBlockAddr);
        if(result)
            goto end;

        if(logBlockAddr == 0xFFFF)
        {
            if(!(overwriteFlag & SMS_MSK_BLOCK_STATUS))
                continue;

            LOG_INFO " Find unused block %d \n", p LOG_END
            MSCard.unusedBlockIndex = p;

            /** 
             * If erase status isn't checked, earse it! 
             */
            if(!(overwriteFlag & SMS_MSK_UPDATE_STATUS))
            {
                LOG_INFO " This block isn't reased, erase it. Overwrite Flag = 0x%02X \n", overwriteFlag LOG_END
                result = MS_EraseBlock(p);
                if(result)
                    goto end;
            }
            goto end;
        }
        else
            continue;
    }

    result = ERROR_MS_NOT_FIND_UNUSED_BLOCK;

end:
    LOG_LEAVE "[MS_FindUnusedBlock] Leave \n" LOG_END
    if(result)
        LOG_ERROR "MS_FindUnusedBlock() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=================================
/**
 * Search the block to be updated and update the new mapping.
 */
//=================================
static MMP_INT
MS_GetUpdateBlock(MMP_UINT32 logBlockAddr, MMP_UINT32* newPhyBlockAddr)
{
    MMP_INT    result = 0;
    MMP_UINT32 segmentIndex = (MMP_UINT32)lookupTable[logBlockAddr] / MS_SEGMENT_SIZE;
    MMP_UINT32 freeIndex = logBlockAddr % (MMP_UINT32)freeBlockNum[segmentIndex];
    MMP_UINT16 tmp = 0;

    LOG_ENTER "[MS_GetUpdateBlock] logBlockAddr = %d, oldPhyBlockAddr = %d \n", logBlockAddr, lookupTable[logBlockAddr] LOG_END

    tmp = lookupTable[logBlockAddr];
    lookupTable[logBlockAddr] = freeTable[segmentIndex][freeIndex];
    freeTable[segmentIndex][freeIndex] = tmp;
    (*newPhyBlockAddr) = (MMP_UINT32)lookupTable[logBlockAddr];
    MSCard.lastUpdateIndex = freeIndex;
    MSCard.lastUpdateSegment = segmentIndex;
    MSCard.lastLogBlockAddr = logBlockAddr;
    LOG_DEBUG " logBlockAddr = %d, Source physical %d to updated physical Block %d, \n", logBlockAddr, tmp, (*newPhyBlockAddr) LOG_END

    LOG_LEAVE "[MS_GetUpdateBlock] Leave, newPhyBlockAddr = %d \n", newPhyBlockAddr LOG_END
    if(result)
        LOG_ERROR "MS_GetUpdateBlock() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS Boot Block Search Process.
 */
//=================================
MMP_INT
MS_SearchBootBlock(void)
{
    MMP_INT    result = 0;
    MMP_UINT32 n = 0; /** block number */
    MMP_UINT8  data[MS_PAGE_SIZE] = {0};
    MMP_UINT8  overwiretFlag = 0;
    MMP_UINT8  managementFlag = 0;
    MMP_UINT16 logBlockAddr = 0;
    MMP_UINT16 bootBlockId = 0;

    for(n=0; n<12; n++)
    {
        result = MS_ReadPage0(n, &overwiretFlag, &managementFlag, &logBlockAddr, data);
        if(result)
        {
            if(result == ERROR_MS_UNABLE_CORRECT_DATA_FLAG)
            {
                LOG_WARNING " MS_SearchBootBlock@MS_ReadPage0() return error code 0x%08X \n", result LOG_END
                result = 0;
                continue;
            }
            else
                goto end;
        }

        if(!(overwiretFlag & SMS_MSK_BLOCK_STATUS))
        {
            LOG_WARNING " MS_SearchBootBlock@ Block Status NG! n = %d \n", n LOG_END
            continue;
        }

        if(managementFlag & SMS_MSK_SYSTEM_BIT)
        {
            LOG_WARNING " MS_SearchBootBlock@ Block %d is not BOOT BLOCK! \n", n LOG_END
            MSCard.flags |= MS_FLAGS_BOOT_AREA_PROTECTION_FLAG;
            continue;
        }
    
        bootBlockId = (MMP_UINT16)((data[0] << 8) | data[1]);
        if(bootBlockId != 0x0001)
        {
            LOG_WARNING " MS_SearchBootBlock@ Boot Block %d ID is not 0x0001! \n", n LOG_END
            MSCard.flags |= MS_FLAGS_BOOT_AREA_PROTECTION_FLAG;
            continue;
        }

        MSCard.bootBlockNum++;
        if(!(MSCard.flags & MS_FLAGS_FIND_FIRST_BOOT_BLOCK))
        {
            MSCard.bootBlock0Index = n;
            MSCard.flags |= MS_FLAGS_FIND_FIRST_BOOT_BLOCK;
            LOG_INFO " MS_SearchBootBlock@ Boot block Index is %d \n", MSCard.bootBlock0Index LOG_END
        }
        if(MSCard.bootBlockNum == 2)
        {
            MSCard.bootBlock1Index = n;
            break;
        }
    }

    if(n > 12)
    {
        if(!MSCard.bootBlockNum)
        {
            result = ERROR_MS_NOT_FOUND_BOOT_BLOCK;
            goto end;
        }
    }
    if(MSCard.bootBlockNum == 1)
        MSCard.bootBlock1Index = MSCard.bootBlock0Index;

end:
    LOG_INFO " Boot block number = %d \n", MSCard.bootBlockNum LOG_END
    if(result)
        LOG_ERROR "MS_SearchBootBlock() return error code 0x%08X, block number = %d \n", result, n LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS get boot and attribute information.
 */
//=================================
MMP_INT
MS_GetCapacity(void)
{
    MMP_INT    result = 0;
    MMP_UINT16 num = 0;
    MMP_UINT16 size = 0;
    MMP_UINT8  data[MS_PAGE_SIZE] = {0};
    MMP_UINT16 blockNum = 0;
    MMP_UINT16 tmpEffectBlockNum = 0;
    MMP_UINT16 effectBlockNum = 0;
    MMP_UINT16 blockSize = 0;

    result = MS_ReadPage(MSCard.bootBlock0Index, 0, data);
    if(result)
    {
        if(MSCard.bootBlockNum == 2)
        {
            result = MS_ReadPage(MSCard.bootBlock1Index, 0, data);
            if(result)
                goto end;
        }
        else
            goto end;
    }

    num = data[0x1B9];
    size = (num/16)*1000+(num%16)*100;
	num = data[0x1BA];
	size += (num/16)*10+num%16;
    MSCard.mediaSize = size;

	if(size > 8)
	{		
        MSCard.pageNum = 32;
        MSCard.physicalBlockNum = (size/8) * 512;
        MSCard.logicalBlockNum  = (size/8 - 1) * 496 + 494;		
        MSCard.blockSize = 16;
	}
	else
	{
        MSCard.pageNum = 16;
		if(size == 8)
		{			
            MSCard.physicalBlockNum = 1024;
            MSCard.logicalBlockNum  = 496 + 494;		
		}
		else
		{
            MSCard.physicalBlockNum = 512;
            MSCard.logicalBlockNum  = 494;		
		}
        MSCard.blockSize = 8;
	}	
    MSCard.segmentNum = MSCard.physicalBlockNum / MS_SEGMENT_SIZE;

    /**
     * Boot Block Contents Check Process.
     */
    /** Check Memory Stick Class = 1 */
    if(data[0x1A0] != 1)
    {
        result = ERROR_MS_BOOT_CONTENT_CLASS_ERROR;
        goto end;
    }
    /** Check Device Type */
    if(data[0x1D8] & 0xFC)
    {
        LOG_ERROR " Boot content's Device Type is 0x%02X \n", data[0x1D8] LOG_END
        result = ERROR_MS_BOOT_CONTENT_DEVICE_TYPE_ERROR;
        goto end;
    }
    if(data[0x1D8] & 0x03)
    {
        LOG_WARNING " This MS card is ROM 0x%02X \n", (data[0x1D8] & 0x03) LOG_END
        MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
        if( (data[0x1D8] & 0x03) == 0x01 )
            MSCard.flags |= MS_FLAGS_MS_ROM;
    }
    /** Check Parallel transfer support */
    if(!(MSCard.flags & MS_FLAGS_MS_ROM))
    {
        if(data[0x1D3] == 1)
        {
            LOG_INFO " Support Serial & Parallel **************************!!!!!!  \n" LOG_END
            MSCard.flags |= MS_FLAGS_MS_SUPPORT_PARALLEL;
        }
    }
    /** Check Format type = 1 */
    if(data[0x1D6] != 1)
    {
        result = ERROR_MS_BOOT_CONTENT_FORMAT_TYPE_ERROR;
        goto end;
    }
    /** Check block number */
    blockNum = (MMP_UINT16)((data[0x1A4] << 8) | data[0x1A5]);
    if(blockNum != MSCard.physicalBlockNum)
    {
        result = ERROR_MS_BOOT_CONTENT_BLOCK_NUM_ERROR;
        goto end;
    }
    /** Check effective block number */
    tmpEffectBlockNum = (MMP_UINT16)((data[0x1A6] << 8) | data[0x1A7]);
    switch(MSCard.mediaSize)
    {
    case 4:
        effectBlockNum = 0x01F0;
        break;
    case 8:
    case 16:
        effectBlockNum = 0x03E0;
        break;
    case 32:
        effectBlockNum = 0x07C0;
        break;
    case 64:
        effectBlockNum = 0x0F80;
        break;
    case 128:
        effectBlockNum = 0x1F00;
        break;
    }
    if(tmpEffectBlockNum != effectBlockNum)
    {
        result = ERROR_MS_BOOT_CONTENT_EFFECT_NUM_ERROR;
        goto end;
    }
    /** Check block size */
    blockSize = (MMP_UINT16)((data[0x1A2] << 8) | data[0x1A3]);
    if(blockSize != MSCard.blockSize)
    {
        LOG_ERROR "data[0x1A2] = 0x%02X, data[0x1A3] = 0x%02X, Block size = 0x%04X \n", data[0x1A2], data[0x1A3], MSCard.blockSize LOG_END
        result = ERROR_MS_BOOT_CONTENT_BLOCK_SIZE_ERROR;
        goto end;
    }
    /** Check controller function for MagicGate compliant media */
    if(!(MSCard.flags & MS_FLAGS_MS_ROM))
    {
        if((data[0x1C8] == 0x10) && (data[0x1C9] == 0x01))
        {
            LOG_INFO " This is Magic Gate compilant media. \n" LOG_END
        }
    }

    /**
     * System entry 48 bytes.
     */
    /** the offset value from page 1 */
    MSCard.disabledBlockOffset = ( (data[0x170]   << 24) |
                                   (data[0x170+1] << 16) |
                                   (data[0x170+2] <<  8) |
                                   (data[0x170+3] ) );
    MSCard.disabledBlockNum = ( (data[0x170+4] << 24) |
                                (data[0x170+5] << 16) |
                                (data[0x170+6] <<  8) |
                                (data[0x170+7] ) )/2;
    LOG_INFO " Disabled Block Data start location is 0x%08X \n", MSCard.disabledBlockOffset LOG_END
    LOG_INFO " Disabled number of blocks is 0x%08X \n", MSCard.disabledBlockNum LOG_END

    /**
     * Disabled Block Data.
     */
    {
        MMP_UINT i = 0;
        result = MS_ReadPage(MSCard.bootBlock0Index, 1, data);
        if(result)
        {
            if(MSCard.bootBlockNum == 2)
            {
                result = MS_ReadPage(MSCard.bootBlock1Index, 1, data);
                if(result)
                    goto end;
            }
            else
                goto end;
        }

        LOG_INFO "Disabled Block Data: \n" LOG_END
        MSCard.disabledBlockNum = 0;
        while( (data[i] != 0xFF) || (data[i+1] != 0xFF) )
        {
            LOG_INFO " 0x%02X 0x%02X \n", data[i], data[i+1] LOG_END
            defectBlock[MSCard.disabledBlockNum++] = ((data[i] << 8) | data[i+1]);
            i += 2;
        }
    }

end:
    {
        LOG_DATA " MS card size = %d Mb\n", MSCard.mediaSize LOG_END
        LOG_DATA " Page number  = %d \n", MSCard.pageNum LOG_END
        LOG_DATA " Physical Block number = %d \n", MSCard.physicalBlockNum LOG_END
        LOG_DATA " Logical Block number  = %d \n", MSCard.logicalBlockNum LOG_END
        LOG_DATA " Segment number = %d \n", MSCard.segmentNum LOG_END
    }
    if(result)
        LOG_ERROR "SMS_GetCapacity() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS Boot Area Protection Process.
 */
//=================================
MMP_INT
MS_BootAreaProtection(void) 
{
    MMP_INT    result = 0;
    MMP_UINT32 blockIndex = 0;
    MMP_UINT8  data[MS_PAGE_SIZE] = {0};
    MMP_UINT8  overwiretFlag = 0;
    MMP_UINT8  managementFlag = 0;
    MMP_UINT16 logBlockAddr = 0;
    
    if(MSCard.flags & MS_FLAGS_WRITE_PROTECT)
        goto end;
    
    for(blockIndex=0; blockIndex < MSCard.bootBlock1Index; blockIndex++)
    {
        result = MS_ReadPage0(blockIndex, &overwiretFlag, &managementFlag, &logBlockAddr, data);
        if(result)
        {
            if(result == ERROR_MS_UNABLE_CORRECT_DATA_FLAG)
            {
                result = 0;
                continue;
            }
            else
                goto end;
        }
        /** this block is boot block */
        if(!(managementFlag & SMS_MSK_SYSTEM_BIT))
            continue;

        /** this block status is 0: NG */
        if(!(overwiretFlag & SMS_MSK_BLOCK_STATUS))
            continue;

        if(logBlockAddr != 0xFFFF)
        {
            result = MS_FindUnusedBlock();
            if(result)
                goto end;

            result = MS_CopyBlock(blockIndex, MSCard.unusedBlockIndex, 0, (MMP_UINT16)MSCard.pageNum);
            if(result)
                goto end;
            LOG_INFO " Copy the contents of Block %d to User Area Block %d !! ~~~~~~ \n", blockIndex, MSCard.unusedBlockIndex LOG_END
        }
        result = MS_OverwriteOverwriteFlag(blockIndex, 0, SMS_OWFLAG_BLOCK_NG);
        if(result)
            goto end;
    }

end:
    if(result)
        LOG_ERROR "MS_BootAreaProtection() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

static MMP_BOOL
MS_IsDefectBlock(MMP_UINT16 blockNum)
{
    MMP_UINT16 i = 0;
    for(i=0; i<MSCard.disabledBlockNum; i++)
    {
        if(blockNum == defectBlock[i])
        {
            LOG_INFO " This is defect block. blockNum = %d, defectBlock[%d] = 0x%04X \n", blockNum, i, defectBlock[i] LOG_END
            return MMP_TRUE;
    }
    }
    return MMP_FALSE;
}

//=================================
/**
 * MS create Look Up Table.
 */
//=================================
MMP_INT
MS_CreateLookupTable(void)
{
    MMP_INT  result = 0;
    MMP_UINT16 p = 0;
    MMP_UINT8  overwriteFlag = 0;
    MMP_UINT8  managementFlag = 0;
    MMP_UINT16 logicalAddr = 0;
    MMP_UINT16 segmentIndex = 0;
    MMP_UINT8  overwriteFlag1 = 0;
    MMP_UINT8  pageStatus = 0;

    memset((void*)lookupTable, 0xFF, sizeof(lookupTable));
    memset((void*)freeTable, 0xFF, sizeof(freeTable));
    memset((void*)freeBlockNum, 0x0, sizeof(freeBlockNum));

    /**
     * Logical Address / Physical Block Number Corresponding Information Creation Process.
     */
    for(p=0; p<MSCard.physicalBlockNum; p++)
    {
        /** Is Defect Block */
        if(MS_IsDefectBlock(p))
            continue;

        segmentIndex = p / MS_SEGMENT_SIZE;
        result = MS_GetExtraData(p, &overwriteFlag, &managementFlag, &logicalAddr);
        if(result == ERROR_MS_UNABLE_CORRECT_DATA_FLAG)
        {
            LOG_ERROR " Get extra data %d fail! \n", p LOG_END
            result = MS_OverwriteOverwriteFlag(p, 0, SMS_OWFLAG_BLOCK_NG);
            if(result)
                goto end;

            result = 0;
            continue;
        }

        /** for last segment */
        if(p >= blockConfig[MSCard.segmentNum-1][MS_PHY_START])
        {
            MMP_UINT8 pageStatus = 0;
            /** Transformation Table bit of this block is 0. */
            if(!(managementFlag & SMS_MSK_CONVERSION_TABLE_BIT))
            {
                LOG_INFO " MS_CreateLookupTable@Physical block %d's Transformation Table bit is 0! flag = 0x%02X \n", p, managementFlag LOG_END
                result = MS_EraseBlock((MMP_UINT32)p);
                if(result)
                {
                    LOG_ERROR " (1) Erase physical block %d fail!!! \n", p LOG_END
                    goto end;
                }
            }
        }

        if(!(overwriteFlag & SMS_MSK_BLOCK_STATUS))
        {
            LOG_INFO " MS_CreateLookupTable@Physical block %d's Block Status is 0. \n", p LOG_END
            result = 0;
            continue;
        }

        if(overwriteFlag == 0xC0)
        {
            LOG_INFO " Block %d overwriteFlag = 0x%02X => Boot Area , logicalAddr = %d \n", p, overwriteFlag, logicalAddr LOG_END
            result = 0;
            continue;
        }

        pageStatus = (overwriteFlag & SMS_MSK_PAGE_STATUS) >> 5;
        if( (pageStatus != 0) && (pageStatus != 3) )
        {
            LOG_INFO " MS_CreateLookupTable@Physical block %d's Page Status is != 0(data error) && != 3(OK) \n", p LOG_END
            result = MS_OverwriteOverwriteFlag(p, 0, SMS_OWFLAG_BLOCK_NG);
            if(result)
                goto end;

            result = 0;
            continue;
        }

        if(logicalAddr == 0xFFFF)
        {
            //LOG_INFO " Physical block %d's Logical address is 0xFFFF \n", p LOG_END
            result = MS_EraseBlock((MMP_UINT32)p);
            if(result)
            {
                LOG_ERROR " (2) Erase physical block %d fail!!! \n", p LOG_END
                goto end;
            }
            freeTable[segmentIndex][freeBlockNum[segmentIndex]++] = p;
            continue;
        }

        if( (logicalAddr < blockConfig[segmentIndex][MS_LOG_START]) ||
            (logicalAddr > blockConfig[segmentIndex][MS_LOG_END])  )
        {
            LOG_INFO " MS_CreateLookupTable@Physical block %d's Logical address is %d and not in the predefined range of the segment!!\n", p, logicalAddr LOG_END
            result = MS_EraseBlock((MMP_UINT32)p);
            if(result)
            {
                LOG_ERROR " (3) Erase physical block %d fail!!! \n", p LOG_END
                goto end;
            }
            freeTable[segmentIndex][freeBlockNum[segmentIndex]++] = p;
        }

        if(lookupTable[logicalAddr] == 0xFFFF)
        {
            lookupTable[logicalAddr] = p;
            continue;
        }

        /** lookupTable[logicalAddr] is not 0xFFFF */
        LOG_WARNING " lookupTable[%d] is not 0xFFFF \n", logicalAddr LOG_END
        result = MS_GetExtraData(lookupTable[logicalAddr], &overwriteFlag1, MMP_NULL, MMP_NULL);
        if(result)
        {
            LOG_ERROR " Get overwrite flag 1 fail! \n" LOG_END
            goto end;
        }
        if( (overwriteFlag & SMS_MSK_UPDATE_STATUS) != (overwriteFlag1 & SMS_MSK_UPDATE_STATUS) )
        {
            if(!(overwriteFlag & SMS_MSK_UPDATE_STATUS))
            {
                result = MS_EraseBlock((MMP_UINT32)lookupTable[logicalAddr]);
                if(result)
                {
                    LOG_ERROR " (4) Erase physical block %d fail!!! \n", lookupTable[logicalAddr] LOG_END
                    goto end;
                }
                freeTable[segmentIndex][freeBlockNum[segmentIndex]++] = lookupTable[logicalAddr];
                lookupTable[logicalAddr] = p;
            }
            else
            {
                result = MS_EraseBlock((MMP_UINT32)p);
                if(result)
                {
                    LOG_ERROR " (5) Erase physical block %d fail!!! \n", p LOG_END
                    goto end;
                }
                freeTable[segmentIndex][freeBlockNum[segmentIndex]++] = (MMP_UINT16)p;
            }
        }
        else
        {
            if(p < lookupTable[logicalAddr])
            {
                result = MS_EraseBlock((MMP_UINT32)p);
                if(result)
                {
                    LOG_ERROR " (6) Erase physical block %d fail!!! \n", p LOG_END
                    goto end;
                }
                freeTable[segmentIndex][freeBlockNum[segmentIndex]++] = (MMP_UINT16)p;
            }
            else
            {
                result = MS_EraseBlock((MMP_UINT32)lookupTable[logicalAddr]);
                if(result)
                {
                    LOG_ERROR " (7) Erase physical block %d fail!!! \n", lookupTable[logicalAddr] LOG_END
                    goto end;
                }
                freeTable[segmentIndex][freeBlockNum[segmentIndex]++] = lookupTable[logicalAddr];
                lookupTable[logicalAddr] = p;
            }
        }
    }

    /**
     * Logical Address Confirmation Process.
     */
    for(segmentIndex=0; segmentIndex<MSCard.segmentNum; segmentIndex++)
    {
        MMP_UINT16 d = 0; /** logical address of each segment */

        /** last segment */
        if(segmentIndex==(MSCard.segmentNum-1))
        {
            if(freeBlockNum[segmentIndex] < 2)
            {
                MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
                goto end;
            }
        }

        if(freeBlockNum[segmentIndex] < 1)
        {
            MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
            goto end;
        }

        for(d=blockConfig[segmentIndex][MS_LOG_START]; d<=blockConfig[segmentIndex][MS_LOG_END]; d++)
        {
            if(lookupTable[d] != 0xFFFF)
                continue;

            result = MS_WriteExtraData((MMP_UINT32)freeTable[segmentIndex][freeBlockNum[segmentIndex]-1], 0, (MMP_UINT32)d);
            if(result)
                goto end;

            lookupTable[d] = freeTable[segmentIndex][freeBlockNum[segmentIndex]-1];
            freeTable[segmentIndex][freeBlockNum[segmentIndex]-1] = 0xFFFF;
            freeBlockNum[segmentIndex]--;

            /** last segment */
            if(segmentIndex==(MSCard.segmentNum-1))
            {
                if(freeBlockNum[segmentIndex] < 2)
                {
                    MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
                    goto end;
                }
            }
            else
            {
                if(freeBlockNum[segmentIndex] < 1)
                {
                    MSCard.flags |= MS_FLAGS_WRITE_PROTECT;
                    goto end;
                }
            }
        }
    }

end:
    LOG_INFO "Create Lookup Table Done!! \n\n" LOG_END
    if(result)
        LOG_ERROR "SMS_CreateLookupTable() return error code 0x%08X, p = %d \n", result, p LOG_END
    return result;     
}

//=================================
/**
 * MS get attrib
 */
//=================================
MMP_INT
MS_ReadAttrib(MS_PRO_CARD_ATTRIB* attrib)
{
    MMP_INT result = 0;

    attrib->numCylinders = 0;
    attrib->numHeads     = 63;
    attrib->numSectorPerTrack = 255;
    attrib->numSectors = (MMP_UINT32)(MSCard.logicalBlockNum * MSCard.pageNum);
    attrib->mediaDescriptor = 0xF8;

    {
        LOG_DATA " number of cylinders = %d \n", attrib->numCylinders LOG_END
        LOG_DATA " number of heads     = %d \n", attrib->numHeads LOG_END
        LOG_DATA " sector per track    = %d \n", attrib->numSectorPerTrack LOG_END
        LOG_DATA " number of sectors   = %d \n", attrib->numSectors LOG_END
        LOG_DATA " media descriptor    = %d \n", attrib->mediaDescriptor LOG_END
    }
    if(result)
        LOG_ERROR "MS_ReadAttrib() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}

//=================================
/**
 * MS card read multisector.
 */
//=================================
MMP_INT
MS_ReadMultiSector(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8* data)
{
    MMP_INT    result = 0;
    MMP_UINT32 logBlockAddr = 0;
    MMP_UINT32 phyBlockAddr = 0;
    MMP_UINT32 pageAddr = 0;
    MMP_UINT16 tmpCount = 0;

    logBlockAddr = sector / MSCard.pageNum;
    pageAddr     = sector % MSCard.pageNum;

	while(count > 0)
	{	
        phyBlockAddr = lookupTable[logBlockAddr];
        LOG_DEBUG " logBlockAddr = %d, phyBlockAddr = %d \n", logBlockAddr, phyBlockAddr LOG_END
        if(count > (MSCard.pageNum - pageAddr))
        {
            tmpCount = (MMP_UINT16)(MSCard.pageNum - pageAddr);
            result = MS_ReadBlock(phyBlockAddr, (MMP_UINT8)pageAddr, tmpCount, data);
            if(result)
                goto end;

            count  -= tmpCount;
            sector += tmpCount;
            data += (MS_PAGE_SIZE * tmpCount);
            logBlockAddr += 1;
            pageAddr = 0;
        }
        else
        {
            result = MS_ReadBlock(phyBlockAddr, (MMP_UINT8)pageAddr, count, data);
            if(result)
                goto end;

            count = 0;
        }

        if(!(MSCard.flags & MS_FLAGS_PARALLEL_INTERFACE))
        {
            MMP_Sleep(0);
        }
    }

end:
    if(result)
        LOG_ERROR "MS_ReadMultiSector() return error code 0x%08X \n", result LOG_END
    return result;                                                                                           
}


//=================================
/**
 * MS card write multisector with standard write procedure.
 */
//=================================
MMP_INT
MS_WriteMultiSectorEx(MMP_UINT32 sector, MMP_UINT16 count, MMP_UINT8* data)
{
    MMP_INT    result = 0;
    MMP_UINT32 logBlockAddr = 0;
    MMP_UINT32 oldPhyBlockAddr = 0;
    MMP_UINT32 newPhyBlockAddr = 0;
    MMP_UINT32 pageAddr = 0;
    MMP_UINT16 writeCount = 0;
    MMP_UINT16 copyCount1 = 0;
    MMP_UINT16 copyCount2 = 0;

    LOG_DEBUG " MS_WriteMultiSectorEx(%d, %d, xx) \n", sector, count LOG_END

    logBlockAddr = sector / MSCard.pageNum;
    pageAddr  = sector % MSCard.pageNum;

	while(count > 0)
	{	
        oldPhyBlockAddr = lookupTable[logBlockAddr];

        /** Change update status of block before updated to 0 */
        result = MS_OverwriteOverwriteFlag(oldPhyBlockAddr, 0, SMS_OWFLAG_USED_OR_UPDATING);
        if(result)
            goto end;

        /** Get un-used physical block address and update the lookup table with new mapping */
        result = MS_GetUpdateBlock(logBlockAddr, &newPhyBlockAddr);
        if(result)
            goto end;

        LOG_DEBUG " logBlockAddr = %d, oldPhyBlockAddr = %d, newPhyBlockAddr = %d \n", logBlockAddr, oldPhyBlockAddr, newPhyBlockAddr LOG_END

        /** start sector is not block start page */
        if(pageAddr)
        {
            if( (pageAddr+count) >= MSCard.pageNum )
            {
                LOG_DEBUG " Case 1 \n" LOG_END
                copyCount1 = (MMP_UINT16)pageAddr;
                writeCount = (MMP_UINT16)(MSCard.pageNum - pageAddr);
                copyCount2 = 0;

                LOG_DEBUG " MS_CopyBlock(oldPhyBlockAddr, newPhyBlockAddr, 0, copyCount1) = (%d, %d, 0, %d) \n", oldPhyBlockAddr ,newPhyBlockAddr, copyCount1 LOG_END
                result = MS_CopyBlock(oldPhyBlockAddr, newPhyBlockAddr, 0, copyCount1);
                if(result)
                    goto end;

                LOG_DEBUG " MS_WriteBlock(logBlockAddr, (MMP_UINT8)pageAddr, writeCount, data); = (%d, %d, %d, %d) \n", logBlockAddr, pageAddr, writeCount, data LOG_END
                result = MS_WriteBlock(logBlockAddr, (MMP_UINT8)pageAddr, writeCount, data);
                if(result)
                    goto end;

                logBlockAddr++;
                pageAddr = 0;
                data += (writeCount * MS_PAGE_SIZE);
                count -= writeCount;
            }
            else
            {
                LOG_DEBUG " Case 2 \n" LOG_END
                copyCount1 = (MMP_UINT16)pageAddr;
                writeCount = count;
                copyCount2 = (MMP_UINT16)MSCard.pageNum - (copyCount1 + writeCount);

                result = MS_CopyBlock(oldPhyBlockAddr, newPhyBlockAddr, 0, copyCount1);
                if(result)
                    goto end;

                result = MS_WriteBlock(logBlockAddr, (MMP_UINT8)pageAddr, writeCount, data);
                if(result)
                    goto end;

                result = MS_CopyBlock(oldPhyBlockAddr, newPhyBlockAddr, ((MMP_UINT8)pageAddr+(MMP_UINT8)writeCount), copyCount2);
                if(result)
                    goto end;
                count = 0;
            }
        }
        /** start sector is block start page */
        else
        {
            /** write all block */
            if(count >= MSCard.pageNum)
            {
                LOG_DEBUG " Case 3 \n" LOG_END
                writeCount = (MMP_UINT16)MSCard.pageNum;

                result = MS_WriteBlock(logBlockAddr, 0, writeCount, data);
                if(result)
                    goto end;

                logBlockAddr++;
                data += (writeCount * MS_PAGE_SIZE);
                count -= writeCount;
            }
            else
            {
                LOG_DEBUG " Case 4 \n" LOG_END
                copyCount1 = 0;
                writeCount = count;
                copyCount2 = (MMP_UINT16)MSCard.pageNum - writeCount;

                result = MS_WriteBlock(logBlockAddr, 0, writeCount, data);
                if(result)
                    goto end;

                result = MS_CopyBlock(oldPhyBlockAddr, newPhyBlockAddr, (MMP_UINT8)count, copyCount2);
                if(result)
                    goto end;

                count = 0;
            }
        }

        /** Erase block before updated */
        if(!(MSCard.flags & MS_FLAGS_BLOCK_HAS_PAGE_NG))
        {
            LOG_DEBUG " MS_EraseBlock(%d) \n", oldPhyBlockAddr LOG_END
            result = MS_EraseBlock(oldPhyBlockAddr);
            if(result)
                goto end;
        }
        else
        {
            MSCard.flags &= ~MS_FLAGS_BLOCK_HAS_PAGE_NG;
        }
    }

end:
    if(result)
        LOG_ERROR "MS_WriteMultiSectorEx() return error code 0x%08X, count = %d \n", result, count LOG_END
    return result;                                                                                           
}

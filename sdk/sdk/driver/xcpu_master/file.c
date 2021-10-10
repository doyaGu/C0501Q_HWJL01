/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file file.c
 *
 * @version 0.1
 */
#include "itx.h"
#if ITX_BOOT_TYPE == ITX_HOST_BOOT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "file.h"
#include "xcpu_io.h"
#include "sys_msgq.h"
#include "xcpu_msgq.h"
#include "pal.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define OFFSET_OFFSET         8

#define HEADER1 ((((int)'S')<<24)+(((int)'M')<<16)+(((int)'E')<<8)+(((int)'D')<<0))
#define HEADER2 ((((int)'I')<<24)+(((int)'A')<<16)+(((int)'0')<<8)+(((int)'2')<<0))

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static int parser_rom(unsigned int* buffer, int* startAddr, unsigned int* pBinSize);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Used to get the information (including the file size) of a specific file
 * identified by an unique id.
 *
 * @param id        The corresponding id for a specific file.
 * @param ptInfo    The output file information.
 * @return          0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
UserFileSize(
    MMP_UINT32* pFileSize)
{
    MMP_RESULT errCode = MMP_RESULT_SUCCESS;
    MMP_UINT32 dwFileSize = 0;
    FILE* file;

#if (ITX_BUS_TYPE == ITX_BUS_SPI)
    mmpSpiSetControlMode(SPI_CONTROL_NOR);
#endif
    file = fopen(BOOT_FILE_PATH, "rb");
    if (!file)
        printf("cannot open file: %s\n", BOOT_FILE_PATH);
    else
        printf("open file: %s\n", BOOT_FILE_PATH);


    if (file)
    {
        /*--- get file size ---*/
        errCode = fseek(file, 0L, SEEK_END);
        if (errCode < 0) {
            dwFileSize = 0;
        }
        dwFileSize = ftell(file);
        errCode = fseek(file, 0L, SEEK_SET);
        if (errCode < 0) {
            dwFileSize = 0;
        }
        errCode = fclose(file);
    }
    else
    {
        errCode = MMP_RESULT_ERROR;
    }

    *pFileSize  = dwFileSize;

#if (ITX_BUS_TYPE == ITX_BUS_SPI)
    mmpSpiResetControl();
#endif
    return errCode;
}

//=============================================================================
/**
 * Used to load the content of the specific file into buffer.
 *
 * @param id        The corresponding id for a specific file.
 * @param pBuffer   The output buffer.
 * @param size      Maximum size in bytes to be loaded.
 * @return          The number of bytes actually loaded.
 */
//=============================================================================
MMP_RESULT
UserFileLoad(
    MMP_UINT32 vRamAddress,
    MMP_UINT32 size)
{
    MMP_RESULT errCode = MMP_RESULT_SUCCESS;
    MMP_UINT32 loadSize = 0;
    MMP_UINT32 totalLoadSize = 0;
    MMP_UINT8* buffer = NULL;
    MMP_UINT8* dstbuffer = NULL;
    MMP_UINT32 binSize = 0;
    MMP_UINT32 index = 0;
    FILE* file;

#if (ITX_BUS_TYPE == ITX_BUS_SPI)
    mmpSpiSetControlMode(SPI_CONTROL_NOR);
#endif
    file = fopen(BOOT_FILE_PATH, "rb");

    if (file)
    {
        buffer = PalMalloc(size);

        if (buffer)
        {
            int startAddr;
			unsigned char* pStartBuffer = 0;
            loadSize = fread(buffer, 1, size, file);

            // skip script and decompress
            parser_rom((unsigned int*)buffer, &startAddr, &binSize);
            if (binSize)
            {
                dstbuffer = PalMalloc(binSize);
                PalMemset(dstbuffer, 0xFF, binSize);
            }
            //printf("[Get] startAddr:0x%x, bufstart: 0x%X, load:%u\n",startAddr, buffer, loadSize);
            {
                MMP_UINT32 inSize = loadSize - (startAddr - (unsigned int)buffer) - 4;
                pStartBuffer = ((unsigned char*) startAddr) + 4;
                printf("in: %u, outlen: %u\n", inSize, binSize);

                // decompress
                do_decompress(startAddr, dstbuffer);
            }

#if (ITX_BUS_TYPE == ITX_BUS_SPI)
            mmpSpiResetControl();
            mmpSpiSetControlMode(SPI_CONTROL_SLAVE);
#endif
            xCpuIO_WriteMemory(
                vRamAddress + totalLoadSize,
                (MMP_UINT32)dstbuffer,
                binSize);

#if (ITX_BUS_TYPE == ITX_BUS_SPI)
            mmpSpiResetControl();
            mmpSpiSetControlMode(SPI_CONTROL_NOR);
#endif
            //totalLoadSize += binSize;

            PalFree(buffer);
            PalFree(dstbuffer);
        }
        errCode = fclose(file);
    }

#if (ITX_BUS_TYPE == ITX_BUS_SPI)
    mmpSpiResetControl();
    mmpSpiSetControlMode(SPI_CONTROL_SLAVE);
#endif
    return errCode;
}

int parser_rom(unsigned int* buffer, int* startAddr, unsigned int* pBinSize)
{
    unsigned char* pCurPos = buffer;
    int scriptOffset = 0;
    int scriptLen = 0;
    int binSize = 0;
    int header1 = 0;
    int header2 = 0;
    header1 = (int) (pCurPos[0] << 24 | pCurPos[1] << 16 | pCurPos[2] << 8 | pCurPos[3]);
    header2 = (int) (pCurPos[4] << 24 | pCurPos[5] << 16 | pCurPos[6] << 8 | pCurPos[7]);

    if (header1 != HEADER1 && header2 != HEADER2)
    {
        return MMP_RESULT_ERROR;
    }

    pCurPos += OFFSET_OFFSET;
    scriptOffset = (int) (pCurPos[0] << 24 | pCurPos[1] << 16 | pCurPos[2] << 8 | pCurPos[3]);
    //printf("scrip offset: 0x%X\n", scriptOffset);
    pCurPos = ((unsigned char*) buffer + scriptOffset);
    scriptLen = (int) (pCurPos[0] << 24 | pCurPos[1] << 16 | pCurPos[2] << 8 | pCurPos[3]);
    //printf("scrip len: %u\n", scriptLen);
    pCurPos += (sizeof(int) + scriptLen * sizeof(int));
    // CRC 4bytes, decompress bin size 4bytes, SMAZ 4bytes
    binSize = (int) (pCurPos[4] << 24 | pCurPos[5] << 16 | pCurPos[6] << 8 | pCurPos[7]);
    //printf("bin Size: %u\n", binSize);
    pCurPos += 12;
    *startAddr = pCurPos;
    *pBinSize = binSize;
    return MMP_RESULT_SUCCESS;
}

#endif


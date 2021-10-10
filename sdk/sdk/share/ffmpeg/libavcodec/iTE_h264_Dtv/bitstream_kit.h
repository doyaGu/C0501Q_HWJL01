/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file bitstream_kit.h
 * Used to provide bistream operations
 * @author Steven Hsiao
 * @version 0.1
 */

#ifndef BITSTREAM_KIT_H
#define BITSTREAM_KIT_H

#include "ite/itp.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Macro Definition
//=============================================================================

#define _getPrevAddrInRingBuf(cur, bufStart, bufEnd, numSkipByte) \
            ((cur-numSkipByte) < bufStart) ? (cur-numSkipByte+(bufEnd-bufStart)) : (cur-numSkipByte)

#define _getNextAddrInRingBuf(cur, bufStart, bufEnd, numSkipByte) \
            ((cur+numSkipByte) <= bufEnd) ? (cur+numSkipByte) : (cur+numSkipByte-(bufEnd-bufStart))//[2012/04/11, Evan:prevent infinite loop]((cur+numSkipByte) < bufEnd) ? (cur+numSkipByte) : (cur+numSkipByte-(bufEnd-bufStart))

	/**
	 *  headerAddr         startAddr  currAddr   endAddr         tailAddr
	 *      |-----------------^----------i---------^---------------|
	 **/
#define _getRemainSizeInRingBuf(head, end, tail, cur)   \
            (end < cur) ? ((tail-head)+end-cur) : (end-cur)
                //(end > cur) ? (end-cur) : ((tail-head)+end-cur)

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct BITSTREAM_TAG
{
    uint32_t bitPosition;
    uint8_t* pStartAddress;

    uint32_t  remainByte;
    uint8_t* pBufferStartAddr;
    uint8_t* pBufferEndAddr;
} BITSTREAM;

//=============================================================================
//                              Function  Definition
//=============================================================================

//=============================================================================
/**
 * Init the Bitstream for TS parser usage.
 *
 * @param *ptBitstream     The TS bitstream is desired to initialization.
 * @param *pStreamStart    The start address to take as the stream start
 *                         poisition.
 * @return                 true if succeed, error codes of false
 *                         otherwise.
 */
//=============================================================================
static inline bool BitStreamKit_Init(BITSTREAM* ptBitstream, uint8_t* pStreamStart)
{
    if (NULL == ptBitstream || NULL == pStreamStart)
        return false;

    ptBitstream->bitPosition = 0;
    ptBitstream->pStartAddress = pStreamStart;

    if (ptBitstream->pBufferEndAddr == 0)
        ptBitstream->pBufferEndAddr = (uint8_t*) 0x7FFFFFFF;

    return true;
}


//=============================================================================
/**
 * Read the field value of a specific bitstream and tranlate it from bit format
 * to unsigned int (uint32_t) for reading purbitPositione
 *
 * @param *ptBitstream The TS bitstream is used to read. After read operation,
 *                     the bitPosition and the startAddress of ptBitstream is
 *                     probably moved/changed.
 * @param bitLength    The bitLength that we want to read. The limitation is
 *                     0 ~ 32 bits.
 * @return             The translated result in uint32_t form for the get
 *                     bits request.
 * @Note               Currently implementation is not doing any error check to
 *                     get better performance. For user who want to get bits
 *                     more than 32 bits can cancatneate one more function call
 *                     to perform same result.
 *                     For PTS case with length as 33 bits, we can get the
 *                     whole PTS field as the following example.
 *                     PTS_HIGH = TS_BitStreamGetBits(ptBitstream, 1);
 *                     PTS_LOW = TS_BitStreamGetBits(ptBitstream, 32);
 */
//=============================================================================
static inline uint32_t BitStreamKit_GetBits(BITSTREAM* ptBitstream, uint32_t bitLength)
{
    uint32_t oldPosition = ptBitstream->bitPosition;
    uint32_t next4ByteValue = 0x00000000;
    uint32_t remainByte = ptBitstream->pBufferEndAddr - ptBitstream->pStartAddress;
    uint32_t result = 0;
    uint32_t i = 0;
    uint32_t j = 0;

    if (bitLength > 32)
        return 0;

    remainByte = (remainByte < 4 ? remainByte : 4);
    for (i=1, j=24; i<=remainByte; i++, j-=8)
        next4ByteValue = next4ByteValue | ((uint32_t)ptBitstream->pStartAddress[i]<<j);
    for (i=0; i<4-remainByte; i++, j-=8)
        next4ByteValue = next4ByteValue | ((uint32_t)ptBitstream->pBufferStartAddr[i]<<j);

    result = (uint32_t)ptBitstream->pStartAddress[0] << (24+oldPosition);
    result = result | (next4ByteValue >> (8-oldPosition));
    result = result >> (32-bitLength);

    ptBitstream->pStartAddress += ((bitLength + oldPosition) >> 3);
    if (ptBitstream->pStartAddress >= ptBitstream->pBufferEndAddr)
        ptBitstream->pStartAddress = &ptBitstream->pBufferStartAddr[ptBitstream->pStartAddress - ptBitstream->pBufferEndAddr];

    ptBitstream->bitPosition = (bitLength + oldPosition) & 0x7;
    ptBitstream->remainByte -= ((bitLength + oldPosition) >> 3);

    return result;
}


//=============================================================================
/**
 * Skip the bitstream reading and shift the bitstream start address and current
 * bit bitPositionition.
 *
 * @param *ptBitstream  The TS bitstream is used to read. After read operation,
 *                      the bitPosition and the startAddress of ptBitstream is
 *                      probably moved/changed.
 * @param bitLength     The bitLength that we want to skip. No strict
 *                      limitation.
 * @No return value
 */
//=============================================================================
static inline void BitStreamKit_SkipBits(BITSTREAM* ptBitstream, uint32_t bitLength)
{
    uint32_t oldPosition = ptBitstream->bitPosition;

    ptBitstream->bitPosition = (bitLength + oldPosition) & 0x7;
    ptBitstream->pStartAddress += ((bitLength + oldPosition) >> 3);
    if (ptBitstream->pStartAddress >= ptBitstream->pBufferEndAddr)
        ptBitstream->pStartAddress = &ptBitstream->pBufferStartAddr[ptBitstream->pStartAddress - ptBitstream->pBufferEndAddr];

    ptBitstream->remainByte -= ((bitLength + oldPosition) >> 3);
}


//=============================================================================
/**
 * Previwe the field value of a specific bitstream and tranlate it from bit
 * format to unsigned int32 (uint32_t) for reading Purpose
 *
 * @param *ptBitstream      The TS bitstream is used to read. After read
 *                          operation, the bitPosition and the startAddress
 *                          of ptBitstream won't be moved/changed.
 * @param startBitOffset    the offset of bits from bitstream bitPositionition
 *                          that we want to start to read.
 * @param bitLength         The bitLength that we want to read. The limitation
 *                          is 0 ~ 32 bits
 * @return                  The translated result in uint32_t form for the
 *                          get bits request.
 * @Note                    Currently implementation is not doing any error
 *                          check to get better performance. For user who want
 *                          to get bits more than 32 bits can cancatneate one
 *                          more function call to perform same result.
 *                          For PTS case with length as 33 bits, we can get the
 *                          whole PTS field as the following example.
 *                          PTS_HIGH = TS_BitStreamShowBits(ptBitstream, 10, 1);
 *                          PTS_LOW = TS_BitStreamShowBits(ptBitstream, 11, 32);
 */
//=============================================================================
static inline uint32_t BitStreamKit_ShowBits(BITSTREAM *ptBitstream, uint32_t startBitOffset, uint32_t bitLength)
{
    uint32_t showStartPos = (ptBitstream->bitPosition + startBitOffset) & 0x7;
    uint32_t remainByte = 0;
    uint32_t next4ByteValue = 0x00000000;
    uint32_t result = 0;
    uint8_t* shiftStartAddress;
    uint32_t i = 0;
    uint32_t j = 0;

    if (bitLength > 32)
        return 0;

    shiftStartAddress = &ptBitstream->pStartAddress[(startBitOffset + ptBitstream->bitPosition) >> 3];

    remainByte = ptBitstream->pBufferEndAddr - shiftStartAddress;
    remainByte = (remainByte < 4 ? remainByte : 4);
    for (i=1, j=24; i<=remainByte; i++, j-=8)
        next4ByteValue = next4ByteValue | ((uint32_t)shiftStartAddress[i]<<j);
    for (i=0; i<4-remainByte; i++, j-=8)
        next4ByteValue = next4ByteValue | ((uint32_t)ptBitstream->pBufferStartAddr[i]<<j);

    result = (uint32_t)shiftStartAddress[0] << (24+showStartPos);
    result = result | (next4ByteValue >> (8-showStartPos));
    result = result >> (32-bitLength);

    return result;
}

#ifdef __cplusplus
}
#endif

#endif // End of #ifndef BITSTREAM_KIT_H


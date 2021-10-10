/*
 * Copyright (c) 2007 SMedia Technology Corp. All Rights Reserved.
 */
/** @file psi_packet.c
 * Used to process TS header decoding and then dispatch the decoded payload
 * section to privateDecoder for further decode.
 * @author Steven Hsiao
 * @version 0.1
 */

#ifdef USE_BITSTREAM_KIT
#include "bitstream_kit.h"
#endif
#if defined(USE_DMA_ENGINE) && defined(__FREERTOS__)
#include "dtv_dma.h"
#include "ite/ith.h"
#endif
#include "psi_packet.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

#define TS_PACKET_BYTE_NUMBER       (188)
#define CONTINUITY_COUNTER_MASK     (0xF)
#define VALID_SYNC_BYTE             (0x47)
#define CRC32_BYTE_NUMBER           (4)
#define STUFF_BYTE                  (0xFF)

// We need table_id (8) + section_syntax_indicator(1) + private_indicator(1)
// + Reserved (2) + private_section_length(12) = 3 Bytes to complete the
// header part of a private_section.
#define PRIVATE_SECTION_HEADER_SIZE (3)

// The offset is from the start of the section.
#define SECTION_LENGTH_OFFSET       (12)

typedef enum ADAPTATION_FIELD_CONTROL_TAG
{
    PAYLOAD_EXIST           = 0x01,
    ADAPTATION_FIELD_EXIST  = 0x02
} ADAPTATION_FIELD_CONTROL;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
//                              Structure Definition
//=============================================================================

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Global Data Declaration
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static void
_PSI_DecodeSection(
    PSI_DECODER*  ptDecoder,
    PSI_SECTION** pptSection);

//=============================================================================
//                              Public Function Definition
//=============================================================================

//=============================================================================
/**
 * Inject a TS packet into a PSI decoder for content decoding.
 *
 * @param ptDecoder         The decoder to deocde a incoming TS packet.
 * @param pPacketAddress    The pointer is pointing to the start of a 188 bytes
 *                          TS packet.
 * @return none
 */
//=============================================================================
void
psiPacket_DecodePacket(
    PSI_DECODER*    ptDecoder,
    MMP_UINT8*      pPacketAddress)
{
    MMP_UINT32 sync_byte = 0;
    MMP_UINT32 payload_unit_start_indicator = 0;
    MMP_UINT32 adaptation_field_control = 0;
    MMP_UINT32 continuity_counter = 0;
    MMP_UINT32 adaptation_field_length = 0;
    MMP_UINT32 pointer_field = 0;
    MMP_INT32 availableByte = 0;

    MMP_UINT8* pPayloadAddress = MMP_NULL;
    MMP_UINT8* pBeginSectionAddress = MMP_NULL;
    MMP_UINT8* pAdapationFieldAddress = MMP_NULL;
    MMP_UINT8* pDataAddress = MMP_NULL;

#ifdef USE_BITSTREAM_KIT
    BITSTREAM  tBitStream = { 0 };
#endif
    PSI_SECTION* ptSection = MMP_NULL;

#ifdef USE_BITSTREAM_KIT
    BitStreamKit_Init(&tBitStream, pPacketAddress);
    sync_byte =
        BitStreamKit_GetBits(&tBitStream, 8);

    // transport_error_indicator
        BitStreamKit_SkipBits(&tBitStream, 1);

    payload_unit_start_indicator =
        BitStreamKit_GetBits(&tBitStream, 1);

    // transport_priority
        BitStreamKit_SkipBits(&tBitStream, 1);

    // PID
        BitStreamKit_SkipBits(&tBitStream, 13);

    // transport_scrambling_control
        BitStreamKit_SkipBits(&tBitStream, 2);

    adaptation_field_control =
        BitStreamKit_GetBits(&tBitStream, 2);

    continuity_counter =
        BitStreamKit_GetBits(&tBitStream, 4);

    pDataAddress = tBitStream.pStartAddress;
#else
    sync_byte = pPacketAddress[0];

    payload_unit_start_indicator =
        (pPacketAddress[1] & 0x40) >> 6;

    adaptation_field_control =
        (pPacketAddress[3] & 0x30) >> 4;

    continuity_counter =
        pPacketAddress[3] & 0x0F;
    pDataAddress = &pPacketAddress[4];
#endif
    //
    // 1. Check sync_byte
    //
    if (sync_byte != VALID_SYNC_BYTE)
    {
        return;
    }

    //
    // 2. Check continuity_counter
    //
    if ((continuity_counter == ptDecoder->continuity_counter)
     && (ptDecoder->bDiscontinuity == MMP_FALSE))
    {
        // Duplicate
        return;
    }

    if (continuity_counter != ((ptDecoder->continuity_counter + 1) & CONTINUITY_COUNTER_MASK))
    {
        // Discontinuity
        ptDecoder->bDiscontinuity = MMP_TRUE;

        if (ptDecoder->ptCurrentSection)
        {
            SectionKit_DestroySection(ptDecoder->allocId, ptDecoder->pfFree, ptDecoder->ptCurrentSection);
            ptDecoder->ptCurrentSection = MMP_NULL;
        }
    }

    ptDecoder->continuity_counter = continuity_counter;

    //
    // 3. Check adaptation_field_control
    //
    if ((adaptation_field_control & PAYLOAD_EXIST) != PAYLOAD_EXIST)
    {
        // Payload doesn't exist. That is All stuffing bytes behide,
        // therefore, simply return.
        return;
    }

    if ((adaptation_field_control & ADAPTATION_FIELD_EXIST) == ADAPTATION_FIELD_EXIST)
    {
        pAdapationFieldAddress = pDataAddress;

#ifdef USE_BITSTREAM_KIT
        BitStreamKit_Init(&tBitStream, pAdapationFieldAddress);

        adaptation_field_length =
            BitStreamKit_GetBits(&tBitStream, 8);

        // Skip all adaptation field. The reason is that we only care about the
        // adapation field of TS packet with same PID as PCR_PID of a program.
        BitStreamKit_SkipBits(&tBitStream, adaptation_field_length << 3);

        pPayloadAddress = tBitStream.pStartAddress;
#else
        adaptation_field_length = pAdapationFieldAddress[0];
        pPayloadAddress = (pAdapationFieldAddress + 1 + adaptation_field_length);
#endif
    }
    else
    {
        pPayloadAddress = pDataAddress;
    }

    //
    // 4. Check payload_unit_start_indicator
    //
    if (payload_unit_start_indicator)
    {
#ifdef USE_BITSTREAM_KIT
        // pointer_field exist, get a new begin section address through it
        BitStreamKit_Init(&tBitStream, pPayloadAddress);

        pointer_field =
            BitStreamKit_GetBits(&tBitStream, 8);

        pPayloadAddress = tBitStream.pStartAddress;

        // Jump to the new begin section
        BitStreamKit_SkipBits(&tBitStream, pointer_field << 3);

        pBeginSectionAddress = tBitStream.pStartAddress;
#else
        // pointer_field exist, get a new begin section address through it
        pointer_field = pPayloadAddress[0];
        pPayloadAddress += 1;
        pBeginSectionAddress = (pPayloadAddress + pointer_field);
#endif
    }

    ptSection = ptDecoder->ptCurrentSection;

    //
    // 5. Check whether current section is under process. If no
    // Create a new section handle the packet.
    //
    if (ptSection == MMP_NULL)
    {
        if (pBeginSectionAddress)
        {
            ptDecoder->ptCurrentSection = ptSection
                                        = SectionKit_CreateSection(ptDecoder->allocId, ptDecoder->pfAlloc, ptDecoder->sectionMaxSize);

            if (ptSection)
            {
                pPayloadAddress = pBeginSectionAddress;

                pBeginSectionAddress = MMP_NULL;

                ptDecoder->needByte = PRIVATE_SECTION_HEADER_SIZE;
                ptDecoder->bCompleteHeader = MMP_FALSE;
            }
            else
                return;
        }
        else
        {
            return;
        }
    }

    availableByte = (pPacketAddress + TS_PACKET_BYTE_NUMBER) - pPayloadAddress;

    //
    // 6. Use loop to deal with the rest packet decode issue
    //
    while (availableByte > 0)
    {
        if (availableByte >= (MMP_INT32) ptDecoder->needByte)
        {
#if defined(USE_DMA_ENGINE) && defined(__FREERTOS__)
            DtvDma_Memcpy(DTV_DMA_TS_CH0, 
                          ptSection->pPayloadEndAddress, 
                          pPayloadAddress,
                          ptDecoder->needByte);
#if defined(__OPENRTOS__)
				ithInvalidateDCacheRange(ptSection->pPayloadEndAddress, ptDecoder->needByte);
#elif defined(__FREERTOS__)
				dc_invalidate();
#endif

            while (!DtvDma_IsIdle(DTV_DMA_TS_CH0)) { };
#else
            PalMemcpy(ptSection->pPayloadEndAddress, pPayloadAddress, ptDecoder->needByte);
#endif
            pPayloadAddress               += ptDecoder->needByte;
            ptSection->pPayloadEndAddress += ptDecoder->needByte;
            availableByte                 -= ptDecoder->needByte;

            if (ptDecoder->bCompleteHeader == MMP_FALSE)
            {
                ptDecoder->bCompleteHeader = MMP_TRUE;
#ifdef USE_BITSTREAM_KIT
                // Get section_length only
                BitStreamKit_Init(&tBitStream, ptSection->pData);

                ptDecoder->needByte = ptSection->section_length
                                    = BitStreamKit_ShowBits(&tBitStream,
                                                            SECTION_LENGTH_OFFSET,
                                                            12);
#else
                ptDecoder->needByte = ptSection->section_length
                                     = (MMP_UINT32) ((ptSection->pData[1] & 0xF) << 8 |
                                                      ptSection->pData[2]);
#endif
                if (ptDecoder->needByte > (ptDecoder->sectionMaxSize - PRIVATE_SECTION_HEADER_SIZE))
                {
                    // PSI section is too long
                    SectionKit_DestroySection(ptDecoder->allocId, ptDecoder->pfFree, ptSection);
                    ptDecoder->ptCurrentSection = MMP_NULL;

                    // If there is a new section not being handled then go forward
                    // in the packet
                    if (pBeginSectionAddress)
                    {
                        ptDecoder->ptCurrentSection = ptSection
                                                    = SectionKit_CreateSection(ptDecoder->allocId, ptDecoder->pfAlloc, ptDecoder->sectionMaxSize);

                        if (ptSection)
                        {
                            pPayloadAddress = pBeginSectionAddress;

                            pBeginSectionAddress = MMP_NULL;

                            ptDecoder->needByte = PRIVATE_SECTION_HEADER_SIZE;
                            ptDecoder->bCompleteHeader = MMP_FALSE;

                            availableByte = (pPacketAddress + TS_PACKET_BYTE_NUMBER) - pPayloadAddress;
                        }
                        else
                            availableByte = 0;
                    }
                    else
                    {
                        availableByte = 0;
                    }
                }
            }
            else
            {
                _PSI_DecodeSection(ptDecoder, &ptSection);

                if (ptSection)
                {
                    // Section is valid, callback to next step
                    ptDecoder->pfCallback(ptDecoder, ptSection);
                }

                ptDecoder->ptCurrentSection = MMP_NULL;

                // A TS packet may contain any number of sections, only the first
                // new one is flagged by the pointer_field. If the next payload
                // byte isn't 0xff then a new section starts.
                if ((pBeginSectionAddress == MMP_NULL)
                 && availableByte
                 && (*pPayloadAddress != STUFF_BYTE))
                {
                    pBeginSectionAddress = pPayloadAddress;
                }

                if (pBeginSectionAddress)
                {
                    ptDecoder->ptCurrentSection = ptSection
                                                = SectionKit_CreateSection(ptDecoder->allocId, ptDecoder->pfAlloc, ptDecoder->sectionMaxSize);

                    if (ptSection)
                    {
                        pPayloadAddress = pBeginSectionAddress;

                        pBeginSectionAddress = MMP_NULL;

                        ptDecoder->needByte = PRIVATE_SECTION_HEADER_SIZE;
                        ptDecoder->bCompleteHeader = MMP_FALSE;

                        availableByte = (pPacketAddress + TS_PACKET_BYTE_NUMBER) - pPayloadAddress;
                    }
                    else
                        availableByte = 0;
                }
                else
                {
                    availableByte = 0;
                }
            }
        }
        else
        {
#if defined(USE_DMA_ENGINE) && defined(__FREERTOS__)
            DtvDma_Memcpy(DTV_DMA_TS_CH0, 
                          ptSection->pPayloadEndAddress, 
                          pPayloadAddress,
                          availableByte);
#else
            PalMemcpy(ptSection->pPayloadEndAddress, pPayloadAddress, availableByte);
#endif
            ptSection->pPayloadEndAddress += availableByte;
            ptDecoder->needByte           -= availableByte;
            availableByte = 0;
        }
    }
}


//=============================================================================
//                              Private Function Definition
//=============================================================================

// TODO: return error code
//=============================================================================
/**
 * Deal with a complete section validation and private_section header decode
 *
 * @param ptDecoder the section decoder.
 * @param ptSection A complete section is ready for validation/decode
 * @return none
 */
//=============================================================================
static void
_PSI_DecodeSection(
    PSI_DECODER*  ptDecoder,
    PSI_SECTION** pptSection)
{
#ifdef USE_BITSTREAM_KIT
    BITSTREAM       tBitStream  = { 0 };
#else
    MMP_UINT8*      pCurrentAddr = MMP_NULL;
#endif
    PSI_SECTION*    ptSection   = *pptSection;

#if defined(USE_DMA_ENGINE) && defined(__FREERTOS__)
    while (!DtvDma_IsIdle(DTV_DMA_TS_CH0)) { };

#if defined(__OPENRTOS__)
				ithInvalidateDCache();
#elif defined(__FREERTOS__)
				dc_invalidate();
#endif
#endif

#ifdef USE_BITSTREAM_KIT
    BitStreamKit_Init(&tBitStream, ptSection->pData);

    ptSection->table_id =
        BitStreamKit_GetBits(&tBitStream, 8);

    ptSection->section_syntax_indicator =
        BitStreamKit_GetBits(&tBitStream, 1);

//    ptSection->private_indicator =
//        BitStreamKit_GetBits(&tBitStream, 1);
        BitStreamKit_SkipBits(&tBitStream, 1);

    // reserved
        BitStreamKit_SkipBits(&tBitStream, 2);

    // private_section_length
        BitStreamKit_SkipBits(&tBitStream, 12);
#else
    pCurrentAddr = ptSection->pData;
    ptSection->table_id = pCurrentAddr[0];
    ptSection->section_syntax_indicator = (pCurrentAddr[1] & 0x80) >> 7;
//    ptSection->private_indicator = (pCurrentAddr[1] & 0x40) >> 6;
    pCurrentAddr += 3;
#endif

    if (ptSection->section_syntax_indicator)
    {
        ptSection->pPayloadEndAddress -= CRC32_BYTE_NUMBER;

        if (SectionKit_VerifySection(ptSection))
        {
#ifdef USE_BITSTREAM_KIT
            ptSection->table_id_extension =
                BitStreamKit_GetBits(&tBitStream, 16);

            // reserved
                BitStreamKit_SkipBits(&tBitStream, 2);

            ptSection->version_number =
                BitStreamKit_GetBits(&tBitStream, 5);

            ptSection->current_next_indicator =
                BitStreamKit_GetBits(&tBitStream, 1);

            ptSection->section_number =
                BitStreamKit_GetBits(&tBitStream, 8);

            ptSection->last_section_number =
                BitStreamKit_GetBits(&tBitStream, 8);

            ptSection->pPayloadStartAddress = tBitStream.pStartAddress;
#else
            ptSection->table_id_extension = (MMP_UINT32) (pCurrentAddr[0] << 8 |
                                                          pCurrentAddr[1]);
            ptSection->version_number = (pCurrentAddr[2] & 0x3E) >> 1;
            ptSection->current_next_indicator = (pCurrentAddr[2] & 0x1);
            ptSection->section_number = pCurrentAddr[3];
            ptSection->last_section_number = pCurrentAddr[4];
            ptSection->pPayloadStartAddress = &pCurrentAddr[5];
#endif
        }
        else
        {
            SectionKit_DestroySection(ptDecoder->allocId, ptDecoder->pfFree, ptSection);
            *pptSection = MMP_NULL;
        }
    }
    else
    {
#ifdef USE_BITSTREAM_KIT
        ptSection->pPayloadStartAddress = tBitStream.pStartAddress;
#else
        ptSection->pPayloadStartAddress = pCurrentAddr;
#endif
    }
}

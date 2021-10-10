
#include <stdio.h>
#include <stdlib.h>
#include "jpg_parser.h"
#include "jpg_defs.h"
#include "jpg_reg.h"
#include "jpg_common.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define GetHighByte(bLEndian, pRPtr)     ((*(pRPtr) & 0xF0) >> 4)
#define GetLowByte(bLEndian, pRPtr)      (*(pRPtr) & 0x0F)
#define GetByte(bLEndian, pRPtr)         (*(pRPtr))
#define GetWord(bLEndian, pRPtr)         (bLEndian == true ? *(pRPtr+1)<<8 | *(pRPtr) : *(pRPtr)<<8 | *(pRPtr+1))
#define GetDWord(bLEndian, pRPtr)        (bLEndian == true ? *(pRPtr+3)<<24 | *(pRPtr+2)<<16 | *(pRPtr+1)<<8 | *(pRPtr) : *(pRPtr)<<24 | *(pRPtr+1)<<16 | *(pRPtr+2)<<8 | *(pRPtr+3))
#define GetValue(bLEndian, pRPtr, Count) (Count == 2 ? GetWord(bLEndian, pRPtr) : GetDWord(bLEndian, pRPtr))

#define APP2_CONTENTS_LIST          0x01
#define APP2_STREAM_DATA            0x02

#define APP2_OFFSET_TYPE            (0x0A-2)
#define APP2_OFFSET_LIST_INDEX      (0x0B-2)
#define APP2_OFFSET_STREAM_DATA     (0x11-2)

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static bool
_isLEndian(
    void)
{
    const uint32_t v = 0x12345678u;
    const uint8_t  *p = (const uint8_t*)&v;
    return (*p == (uint8_t)(0x12)) ? false : true;
}

static int
_GetCurFactorPos(
    JPG_STREAM_HANDLE   *pHJStream,
    int                 realSize,
    int                 offset)
{
    uint32_t            StreamPos = 0;
    JPG_STREAM_DESC     *pJStreamDesc = &pHJStream->jStreamDesc;

    /**
     *                    |---------- realSize ---------|
     *              prevStreamPos    offset       curStreamPos
     *     ---------------|-------------i---------------|----------|
     **/
    if( pJStreamDesc->jTell_stream )
        pJStreamDesc->jTell_stream(pHJStream, &StreamPos, 0);

    return (int)(StreamPos + offset - realSize);
}

static uint8_t*
_CompleteSection(
    JPG_STREAM_HANDLE   *pHJStream,
    uint8_t             *ptCurr,
    uint8_t             *ptEnd,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    int                 sectionLength = 0;
    JPG_STREAM_DESC     *pJStreamDesc = &pHJStream->jStreamDesc;
    JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;

    sectionLength = GetWord(pJPrsInfo->bLEndian, ptCurr);

    if( (ptCurr + sectionLength) >= ptEnd )
    {
        int   keepLength = 0;
        int   readLength = 0;

        keepLength = (ptEnd - ptCurr);
        readLength = sectionLength - keepLength;
        memcpy(pJPrsBsCtrl->bsPrsBuf, ptCurr, keepLength);

        if( pJStreamDesc->jFull_buf )
            pJStreamDesc->jFull_buf(pHJStream,
                                    pJPrsBsCtrl->bsPrsBuf + keepLength,
                                    readLength, &pJPrsBsCtrl->realSize, 0);
        // reset parameters
        ptCurr = pJPrsBsCtrl->bsPrsBuf;
        pJPrsInfo->remainSize -= pJPrsBsCtrl->realSize;
    }

    return ptCurr;
}

static uint8_t*
_ParseSOF00(
    JPG_STREAM_HANDLE   *pHJStream,
    uint8_t             *ptCurr,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    /**
     * SOF        16        0xffc0        Start Of Frame
     * Lf        16        3Nf+8        Frame header length
     * P        8        8            Sample precision
     * Y        16        0-65535        Number of lines
     * X        16        1-65535        Samples per line
     * Nf        8        1-255        Number of image components (e.g. Y, U and V).
     *
     * ---------Repeats for the number of components (e.g. Nf)-----------------
     * Ci        8        0-255        Component identifier
     * Hi        4        1-4            Horizontal Sampling Factor
     * Vi        4        1-4            Vertical Sampling Factor
     * Tqi        8        0-3            Quantization Table Selector.
     */
    uint16_t        length = 0;
    uint16_t        compCnt = 0;
    JPG_FRM_COMP    *jFrmComp = &pJPrsInfo->jFrmComp;
    int             i;

    length = GetWord(pJPrsInfo->bLEndian, ptCurr);

#if 0
    ptCurr += 2;
    // skip the sample precision field of scan segment
    ptCurr++;
#else
    ptCurr += 3;
#endif

    jFrmComp->imgHeight = GetWord(pJPrsInfo->bLEndian, ptCurr);
    ptCurr += 2;
    jFrmComp->imgWidth = GetWord(pJPrsInfo->bLEndian, ptCurr);
    ptCurr += 2;
    compCnt = GetByte(pJPrsInfo->bLEndian, ptCurr);
    ptCurr++;

    jFrmComp->bSingleChannel = (compCnt == 1) ? true : false;

    for(i = 0; i < compCnt; i++)
    {
        jFrmComp->jFrmInfo[i].compId = GetByte(pJPrsInfo->bLEndian, ptCurr);
        ptCurr++;
        jFrmComp->jFrmInfo[i].horizonSamp = GetHighByte(pJPrsInfo->bLEndian, ptCurr);
        jFrmComp->jFrmInfo[i].verticalSamp = GetLowByte(pJPrsInfo->bLEndian, ptCurr);
        ptCurr++;
        jFrmComp->jFrmInfo[i].qTableSel = GetByte(pJPrsInfo->bLEndian, ptCurr);
        ptCurr++;
    }

    return ptCurr;
}

static uint8_t*
_ParseSOS(
    JPG_STREAM_HANDLE   *pHJStream,
    uint8_t             *ptCurr,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    /**
     * SOS        16        0xffda            Start Of Scan
     * Ls        16        2Ns + 6            Scan header length
     * Ns        8        1-4                Number of image components
     * Csj        8        0-255            Scan Component Selector
     * Tdj        4        0-1                DC Coding Table Selector
     * Taj        4        0-1                AC Coding Table Selector
     * Ss        8        0                Start of spectral selection
     * Se        8        63                End of spectral selection
     * Ah        4        0                Successive Approximation Bit High
     * Ai        4        0                Successive Approximation Bit Low
     */
    uint16_t        length = 0;
    uint16_t        compCnt = 0;
    uint16_t        compIdx = 0;
    JPG_FRM_COMP    *jFrmComp = &pJPrsInfo->jFrmComp;
    JPG_FRM_INFO    *jFrmInfo = pJPrsInfo->jFrmComp.jFrmInfo;
    int             i, j;

    length = GetWord(pJPrsInfo->bLEndian, ptCurr);
    ptCurr += 2;

    compCnt = GetByte(pJPrsInfo->bLEndian, ptCurr);
    ptCurr++;

    jFrmComp->validComp = 0;
    for(i = 0; i < compCnt; i++)
    {
        compIdx = GetByte(pJPrsInfo->bLEndian, ptCurr);
        ptCurr++;

        for(j = 0; j < compCnt; j++)
        {
            if( jFrmInfo[j].compId == compIdx )
            {
                jFrmInfo[j].dcHuffTableSel = GetHighByte(pJPrsInfo->bLEndian, ptCurr);
                jFrmInfo[j].acHuffTableSel = GetLowByte(pJPrsInfo->bLEndian, ptCurr);
                ptCurr++;

                switch( j )
                {
                    case 0: jFrmComp->validComp |= JPG_MSK_DEC_COMPONENT_A_VALID;   break;
                    case 1: jFrmComp->validComp |= JPG_MSK_DEC_COMPONENT_B_VALID;   break;
                    case 2: jFrmComp->validComp |= JPG_MSK_DEC_COMPONENT_C_VALID;   break;
                    case 3: jFrmComp->validComp |= JPG_MSK_DEC_COMPONENT_D_VALID;   break;
                }
            }
        }
    }

    // skip : Ss, Se, Ah, Al
    ptCurr += 3;

    jFrmComp->bNonInterleaved = (jFrmComp->bSingleChannel && compCnt == 1) ? true : false;
    jFrmComp->compNum = compCnt;

    return ptCurr;
}

static uint8_t*
_ParseDHT(
    JPG_STREAM_HANDLE   *pHJStream,
    uint8_t             *ptCurr,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    /**
     * u16 0xffc4
     * u16 be length of segment
     * 4-bits class (0 is DC, 1 is AC, more on this later)
     * 4-bits table id
     * array of 16 u8 number of elements for each of 16 depths
     * array of u8 elements, in order of depth
     */
    uint16_t            length = 0;
    uint8_t             tabClass = 0; // 0:DC, 1:AC
    uint8_t             desId = 0;
    int16_t             index = 0;
    uint8_t             *ptSegEnd = NULL;
    JPG_STREAM_DESC     *pJStreamDesc = &pHJStream->jStreamDesc;
    JPG_H_TABLE         *huff_DC = pJPrsInfo->jFrmComp.huff_DC;  // 0->Y, 1->UV
    JPG_H_TABLE         *huff_AC = pJPrsInfo->jFrmComp.huff_AC;  // 0->Y, 1->UV
    int                 i;

    length = GetWord(pJPrsInfo->bLEndian, ptCurr);
    ptSegEnd = ptCurr + length;

    ptCurr += 2;

    while( ptCurr < ptSegEnd )
    {
        tabClass = GetHighByte(pJPrsInfo->bLEndian, ptCurr);
        desId = GetLowByte(pJPrsInfo->bLEndian, ptCurr);

        ptCurr++;

        index = ((desId == 0) ? desId : 1);
        switch( tabClass )
        {
            case 0:   // DC Huffman table
                huff_DC[index].totalCodeLenCnt = 0;
                for (i = 0; i < 16; i++)
                {
                    huff_DC[index].totalCodeLenCnt += *(ptCurr + i);
                }

                if( pJStreamDesc->jHeap_mem )
                {
                    uint32_t    realSize = 0;
                    huff_DC[index].pHuffmanTable = pJStreamDesc->jHeap_mem(pHJStream, JPG_HEAP_DEF,
                                                                           (huff_DC[index].totalCodeLenCnt + 16), &realSize);
                    if( huff_DC[index].pHuffmanTable &&
                        realSize == (huff_DC[index].totalCodeLenCnt + 16) )
                    {
                        memcpy(huff_DC[index].pHuffmanTable, ptCurr, (huff_DC[index].totalCodeLenCnt + 16));
                    }
                    else if( huff_DC[index].pHuffmanTable )
                    {
                        if( pJStreamDesc->jFree_mem )
                            pJStreamDesc->jFree_mem(pHJStream, JPG_HEAP_DEF, huff_DC[index].pHuffmanTable);
                        huff_DC[index].pHuffmanTable = 0;
                    }
                }

                ptCurr += 16;
                ptCurr += huff_DC[index].totalCodeLenCnt;
                break;

            case 1:   // AC Huffman table
                huff_AC[index].totalCodeLenCnt = 0;
                for (i = 0; i < 16; i++)
                {
                    huff_AC[index].totalCodeLenCnt += *(ptCurr + i);
                }

                if( pJStreamDesc->jHeap_mem )
                {
                    uint32_t    realSize = 0;
                    huff_AC[index].pHuffmanTable = pJStreamDesc->jHeap_mem(pHJStream, JPG_HEAP_DEF,
                                                                           (huff_AC[index].totalCodeLenCnt + 16), &realSize);
                    if( huff_AC[index].pHuffmanTable &&
                        realSize == (huff_AC[index].totalCodeLenCnt + 16) )
                    {
                        memcpy(huff_AC[index].pHuffmanTable, ptCurr, (huff_AC[index].totalCodeLenCnt + 16));
                    }
                    else if( huff_AC[index].pHuffmanTable )
                    {
                        if( pJStreamDesc->jFree_mem )
                            pJStreamDesc->jFree_mem(pHJStream, JPG_HEAP_DEF, huff_AC[index].pHuffmanTable);
                        huff_AC[index].pHuffmanTable = 0;
                    }
                }

                ptCurr += 16;
                ptCurr += huff_AC[index].totalCodeLenCnt;
                break;
        }
    }

    return ptCurr;
}

static uint8_t*
_ParseDQT(
    JPG_STREAM_HANDLE   *pHJStream,
    uint8_t             *ptCurr,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    /**
     * DQT        16        0xffdb            quantization table(s)
     * Lq        16                        quantization table length
     * Pq       4                       value "1" indicates 16-bit Qk values, value "0" indicates 8-bit Qk values
     * Tq       4
     * Qk       array of 16/u8 number of elements
     */

    int16_t         i;
    uint16_t        length = 0;
    int16_t         qTabCnt = 0;
    uint16_t        qTabIdx = 0;
    JPG_Q_TABLE     *qTable = &pJPrsInfo->jFrmComp.qTable;

    length = GetWord(pJPrsInfo->bLEndian, ptCurr);
    ptCurr += 2;

    qTabCnt = (length - 2) / (JPG_Q_TABLE_ELEMENT_NUM + 1);

    for(i = 0; i < qTabCnt; i++)
    {
        qTabIdx = GetLowByte(pJPrsInfo->bLEndian, ptCurr);

        // skip the precision & destination field identifier of Q table
        ptCurr++;
        memcpy(qTable->table[qTabIdx], ptCurr, JPG_Q_TABLE_ELEMENT_NUM);
        ptCurr += JPG_Q_TABLE_ELEMENT_NUM;
    }

    qTable->tableCnt += qTabCnt;
    return ptCurr;
}

static uint8_t*
_ParseDRI(
    JPG_STREAM_HANDLE   *pHJStream,
    uint8_t             *ptCurr,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    uint16_t        length = 0;
    uint16_t        restartInterval = 0;
    JPG_FRM_COMP    *jFrmComp = &pJPrsInfo->jFrmComp;

    length = GetWord(pJPrsInfo->bLEndian, ptCurr);
    ptCurr += 2;
    restartInterval = GetWord(pJPrsInfo->bLEndian, ptCurr);
    ptCurr += 2;

    jFrmComp->restartInterval = restartInterval;

    return ptCurr;
}

static void
_SetDefHTable(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    JPG_H_TABLE     *huff_DC = pJPrsInfo->jFrmComp.huff_DC;  // 0->Y, 1->UV
    JPG_H_TABLE     *huff_AC = pJPrsInfo->jFrmComp.huff_AC;  // 0->Y, 1->UV
    uint16_t        dstIdx = 0;

    for(dstIdx = 0; dstIdx < 2; dstIdx++)
    {
        huff_DC[dstIdx].totalCodeLenCnt = 12;
        huff_DC[dstIdx].pHuffmanTable = (uint8_t*)(&Def_DCHuffTable[dstIdx][0]);

        huff_AC[dstIdx].totalCodeLenCnt = 162;
        huff_AC[dstIdx].pHuffmanTable = (uint8_t*)(&Def_ACHuffTable[dstIdx][0]);
    }
}

static JPG_ERR
_AppSectionOverview(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    JPG_ERR         result = JPG_ERR_OK;
    JPG_STREAM_DESC *pJStreamDesc = &pHJStream->jStreamDesc;
    JPG_PRS_BS_CTRL *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;
    JPG_ATTRIB      *pJAttrib = &pJPrsInfo->jAttrib;
    uint8_t         *ptCur;
    uint8_t         *ptEnd;
    bool            bExistApp01 = false;
    bool            bSupportedApp = false;

    ptCur = (uint8_t*)(pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos);

    // check jpg file
    if( pJPrsBsCtrl->b1stSection == true )
    {
        pJPrsBsCtrl->b1stSection = false;

        if( (*ptCur == JPG_MARKER_START) &&
            (*(ptCur + 1) == JPG_START_OF_IMAGE_MARKER) )
        {
            ptCur += 2;
        }
        else
        {
            pJAttrib->flag |= ( JATT_STATUS_UNSUPPORT_PRIMARY |
                                JATT_STATUS_UNSUPPORT_SMALL_THUMB |
                                JATT_STATUS_UNSUPPORT_LARGE_THUMB );
            result = JPG_ERR_NOT_JPEG;
            goto end;
        }
    }

    ptEnd = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->realSize;

    // parsing process
    while( !((*ptCur == JPG_MARKER_START) && ((*(ptCur+1) & 0xE0) == 0xC0)) )
    {
        if( *ptCur == JPG_MARKER_START )
        {
            bSupportedApp = false;
            switch( *(ptCur + 1) )
            {
            case JPG_APP00_MARKER: // E0
                if( bExistApp01 == true )
                    break;

                ptCur += 2;
                bSupportedApp = true;
                pJAttrib->exifAppOffset[0] = _GetCurFactorPos(pHJStream, pJPrsBsCtrl->realSize, (ptCur - pJPrsBsCtrl->bsPrsBuf));
                pJAttrib->exifAppLength[0] = GetWord(pJPrsInfo->bLEndian, ptCur);
                pJAttrib->exifAppCnt = 1;
                break;

            case JPG_APP01_MARKER: // E1
                if( memcmp((ptCur + 4), "Exif", 4) == 0 )
                {
                    // Exif
                    ptCur += 2;
                    bSupportedApp = true;
                    pJAttrib->exifAppOffset[0] = _GetCurFactorPos(pHJStream, pJPrsBsCtrl->realSize, (ptCur - pJPrsBsCtrl->bsPrsBuf));
                    pJAttrib->exifAppLength[0] = GetWord(pJPrsInfo->bLEndian, ptCur);
                    pJAttrib->exifAppCnt = 1;
                    bExistApp01 = true;
                    pJAttrib->bApp1Find = true;
                }
                break;

            case JPG_APP02_MARKER: // E2
                ptCur += 2;
                bSupportedApp = true;

                // skip ICC_PROFILE
                if( memcmp((ptCur + 2), "ICC_PROFILE", 11) )
                {
                    pJAttrib->exifAppOffset[pJAttrib->exifAppCnt] = _GetCurFactorPos(pHJStream, pJPrsBsCtrl->realSize, (ptCur - pJPrsBsCtrl->bsPrsBuf));
                    pJAttrib->exifAppLength[pJAttrib->exifAppCnt] = GetWord(pJPrsInfo->bLEndian, ptCur);
                    pJAttrib->exifAppCnt++;
                }
                break;

            case JPG_APP13_MARKER: // ED
                if( bExistApp01 == true )
                    break;

                if( memcmp((ptCur + 4), "PHOTOSHOP", 9) == 0 )
                {
                    // PHOTOSHOP
                    break;
                }

                ptCur += 2;
                bSupportedApp = true;
                pJAttrib->exifAppOffset[0] = _GetCurFactorPos(pHJStream, pJPrsBsCtrl->realSize, (ptCur - pJPrsBsCtrl->bsPrsBuf));
                pJAttrib->exifAppLength[0] = GetWord(pJPrsInfo->bLEndian, ptCur);
                pJAttrib->exifAppCnt = 1;
                break;

            case JPG_APP14_MARKER: // EE
                if( memcmp((ptCur + 4), "Adobe", 5) == 0 && *(ptCur + 15) == 0x02 )
                {
                    // AppE for Adobe YCCK
                    pJAttrib->bCMYK = true;
                    ptCur += 2;
                    bSupportedApp = true;
                }
                break;
            }

            // check remain bs_buf size
            if( (bSupportedApp == true && (*(ptCur - 1) & 0xF0) == 0xE0) ||
                (bSupportedApp == false && (*(ptCur + 1) & 0xF0) == 0xE0) ||
                (bSupportedApp == false && (*(ptCur + 1) & 0xF0) == 0xF0) )
            {
                int   sectionLength = 0;

                if( bSupportedApp == false )
                    ptCur += 2;

                sectionLength = GetWord(pJPrsInfo->bLEndian, ptCur);

                if( (ptCur + sectionLength) < ptEnd )
                    ptCur += sectionLength;
                else
                {
                    int   skipLength = 0;

                    skipLength = sectionLength - (ptEnd - ptCur);

                    if( pJStreamDesc->jSeek_stream )
                        pJStreamDesc->jSeek_stream(pHJStream, skipLength, JPG_SEEK_CUR, 0);

                    pJPrsInfo->remainSize -= skipLength;

                    if( pJStreamDesc->jFull_buf )
                        pJStreamDesc->jFull_buf(pHJStream, pJPrsBsCtrl->bsPrsBuf,
                                                pJPrsBsCtrl->bsPrsBufMaxLeng,
                                                &pJPrsBsCtrl->realSize, 0);
                    // reset parameters
                    ptCur = pJPrsBsCtrl->bsPrsBuf;
                    pJPrsInfo->remainSize -= pJPrsBsCtrl->realSize;
                }

                continue;
            }
        }

        // check remain bs_buf size
        if( (ptCur + 4) < ptEnd )  // keep last 4 bytes for avoiding breaking Marker
            ptCur++;
        else
        {
            memcpy(pJPrsBsCtrl->bsPrsBuf, ptCur, 4);
            if( pJStreamDesc->jFull_buf )
                pJStreamDesc->jFull_buf(pHJStream, pJPrsBsCtrl->bsPrsBuf + 4,
                                        pJPrsBsCtrl->bsPrsBufMaxLeng - 4,
                                        &pJPrsBsCtrl->realSize, 0);
            // reset parameters
            ptCur = pJPrsBsCtrl->bsPrsBuf;
            pJPrsInfo->remainSize -= pJPrsBsCtrl->realSize;
        }
    }

    pJAttrib->primaryOffset = _GetCurFactorPos(pHJStream, pJPrsBsCtrl->realSize, (ptCur - pJPrsBsCtrl->bsPrsBuf));
    pJAttrib->primaryLength = pJPrsInfo->fileLength - pJAttrib->primaryOffset;
    pJPrsBsCtrl->curPos = ptCur - pJPrsBsCtrl->bsPrsBuf;

end:
    if( result != JPG_ERR_OK )
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() Err 0x%x ", __FUNCTION__, result);

    return result;
}


static JPG_ERR
_Parse_App01(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;
    JPG_ATTRIB          *pJAttrib = &pJPrsInfo->jAttrib;
    JPG_EXIF_INFO       *pJExifInfo = &pJAttrib->exifInfo;
    bool                bFindSmallJpeg = false;
    uint8_t             *ptCur = 0, *ptBasePos = 0;
    uint8_t             *ptEnd = 0;

    memset((void*)pJExifInfo, 0, sizeof(JPG_EXIF_INFO));

    if( pJAttrib->bApp1Find )
    {
        uint32_t    byteOrder, IFDOffset;
        bool        bLEndian, bFindSOI;
        uint32_t    IFDEntryCnt = 0;
        uint32_t    i, Tag;
        uint32_t    exifIFDoffset = 0;

        jPrs_Seek2SectionStart(pHJStream, pJPrsInfo, pJAttrib->exifAppOffset[0], pJAttrib->exifAppLength[0]);

        ptBasePos = ptCur = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos;
        ptEnd = ptCur + pJAttrib->exifAppLength[0];

        ptCur += 8;
        byteOrder = *(ptCur) << 8 | *(ptCur + 1);
        bLEndian = (byteOrder == 0x4949) ? true : false;

        ptCur += 4;
        IFDOffset = GetValue(bLEndian, ptCur, 4);

        // 0-th IFD
        ptCur       = ptBasePos + 8 + IFDOffset;
        IFDEntryCnt = GetValue(bLEndian, ptCur, 2);
        ptCur += 2;

        for(i = 0; i < IFDEntryCnt; i++)
        {
            if( ptCur > ptEnd )
                goto end;

            Tag = GetValue(bLEndian, ptCur, 2);

            switch( Tag )
            {
            case 0x0112: //Orientation Tag
                pJExifInfo->primaryOrientation = GetValue(bLEndian, ptCur + 8, 2);
                //jpg_msg(1, "Primary Orientation => Tag 0x%x Orientation %d\n", Tag, pJExifInfo->primaryOrientation);
                break;

            case 0x0100: //image Width Tag
                pJExifInfo->imageWidth = GetValue(bLEndian, ptCur + 8, 4);
                //jpg_msg(1, " Tag 0x%x imageWidth %d\n", Tag, pJExifInfo->imageWidth);
                break;

            case 0x0101: //image Height Tag
                pJExifInfo->imageHeight = GetValue(bLEndian, ptCur + 8, 4);
                //jpg_msg(1, " Tag 0x%x imageHeight %d\n", Tag, pJExifInfo->imageHeight);
                break;

            case 0x8769: // EXIF IFD Pointer
                exifIFDoffset = GetValue(bLEndian, ptCur + 8, 4);
                break;

            case 0x010F: // Camera Make Tag
                {
                    uint32_t   dateOffset = GetValue(bLEndian, ptCur + 8, 4);
                    uint32_t   count = GetValue(bLEndian, ptCur + 4, 4);
                    uint8_t    *value = ptBasePos + 8 + dateOffset;

                    if( count > 4 )
                    {
                        count = (count < 32) ? count : 31;
                        memcpy((void*)pJExifInfo->cameraMake, (void*)value, count);
                        pJExifInfo->cameraMake[31] = '\0';
                    }
                    else
                    {
                        if( bLEndian == true )
                        {
                            dateOffset = ((dateOffset & 0xff) << 24) |
                                         ((dateOffset & 0xff00) << 8) |
                                         ((dateOffset & 0xff0000) >> 8) |
                                         ((dateOffset & 0xff000000) >> 24);
                        }
                        memcpy((void*)pJExifInfo->cameraMake, (void*)&dateOffset, count);
                    }
                }
                break;

            case 0x0110: // Camera Model Tag
                {
                    uint32_t   dateOffset = GetValue(bLEndian, ptCur + 8, 4);
                    uint32_t   count = GetValue(bLEndian, ptCur + 4, 4);
                    uint8_t    *value = ptBasePos + 8 + dateOffset;

                    if( count > 4 )
                    {
                        count = (count < 32) ? count : 31;
                        memcpy((void*)pJExifInfo->cameraModel, (void*)value, count);
                        pJExifInfo->cameraModel[31] = '\0';
                    }
                    else
                    {
                        if( bLEndian == true )
                        {
                            dateOffset = ((dateOffset & 0xff) << 24) |
                                         ((dateOffset & 0xff00) << 8) |
                                         ((dateOffset & 0xff0000) >> 8) |
                                         ((dateOffset & 0xff000000) >> 24);
                        }
                        memcpy((void*)pJExifInfo->cameraModel, (void*)&dateOffset, count);
                    }
                }
                break;

            default:
                break;
            }

            ptCur += 12;
        }

        IFDOffset = GetValue(bLEndian, ptCur, 4);

        // exif IFD
        if( exifIFDoffset )
        {
            ptCur       = ptBasePos + 8 + exifIFDoffset;
            IFDEntryCnt = GetValue(bLEndian, ptCur, 2);

            ptCur += 2;
            for(i = 0; i < IFDEntryCnt; i++)
            {
                if( ptCur > ptEnd )
                    goto end;

                Tag = GetValue(bLEndian, ptCur, 2);

                switch (Tag)
                {
                case 0x9003:  // DateTimeOriginal
                    {
                        uint32_t   dateOffset = GetValue(bLEndian, ptCur + 8, 4);
                        uint32_t   count = GetValue(bLEndian, ptCur + 4, 4);
                        uint8_t    *dateValue = ptBasePos + 8 + dateOffset;

                        count = (count < 20) ? count : 19;
                        memcpy((void*)pJExifInfo->dateTime, (void*)dateValue, count);
                        pJExifInfo->dateTime[19] = '\0';
                    }
                    break;

                case 0xA002: //image Width Tag
                    pJExifInfo->imageWidth = GetValue(bLEndian, ptCur + 8, 4);
                    //jpg_msg(1, " Tag 0x%x imageWidth %d\n", Tag, pJExifInfo->imageWidth);
                    break;

                case 0xA003: //image Height Tag
                    pJExifInfo->imageHeight = GetValue(bLEndian, ptCur + 8, 4);
                    //jpg_msg(1, " Tag 0x%x imageHeight %d\n", Tag, pJExifInfo->imageHeight);
                    break;

                default:
                    break;
                }
                ptCur += 12;
            }
        }

        // 1-th IFD ( optional )
        ptCur       = ptBasePos + 8 + IFDOffset;
        IFDEntryCnt = GetValue(bLEndian, ptCur, 2);
        ptCur += 2;

        for (i = 0 ; i < IFDEntryCnt; i++)
        {
            if( ptCur > ptEnd )
                goto end;

            Tag = GetValue(bLEndian, ptCur, 2);

            switch (Tag)
            {
            case 0x0112: //Orientation Tag
                pJExifInfo->thumbOrientation = GetValue(bLEndian, ptCur + 8, 2);
                //jpg_msg(1, "thumbnail Orientation => Tag 0x%x Orientation %d\n", Tag, pJExifInfo->thumbOrientation);
                break;
            default:
                break;

            }
            ptCur += 12;
        }

        // Find thumbnail's offset and length
        bFindSOI = false;
        while( ptCur < ptEnd )
        {
            if( *(ptCur) != JPG_MARKER_START )
            {
                ptCur++;
                continue;
            }

            switch( *(ptCur + 1) )
            {
                case JPG_START_OF_IMAGE_MARKER:
                    if ( bFindSOI == false )
                    {
                        pJAttrib->jBaseLiteInfo[JPG_DEC_SMALL_THUMB].offset = (ptCur - ptBasePos) + pJAttrib->exifAppOffset[0];
                        bFindSOI = true;
                        ptCur += 2;
                    }
                    break;

                case JPG_END_OF_IMAGE_MARKER:
                    if( bFindSOI == true )
                    {
                        pJAttrib->jBaseLiteInfo[JPG_DEC_SMALL_THUMB].size = (ptCur - ptBasePos) + pJAttrib->exifAppOffset[0] - pJAttrib->jBaseLiteInfo[JPG_DEC_SMALL_THUMB].offset + 2;
                        bFindSmallJpeg = true;
                        goto end;
                    }
                    break;

                case JPG_PROGRESSIVE:
                    pJAttrib->jBaseLiteInfo[JPG_DEC_SMALL_THUMB].bJprog = true;
                case JPG_BASELINE_MARKER:
                    pJAttrib->jBaseLiteInfo[JPG_DEC_SMALL_THUMB].width  = (*(ptCur + 7) << 8) | *(ptCur + 8);
                    pJAttrib->jBaseLiteInfo[JPG_DEC_SMALL_THUMB].height = (*(ptCur + 5) << 8) | *(ptCur + 6);
                    ptCur += (((ptCur[2] << 8) | ptCur[3]) + 2);
                    break;

                default:
                    if( (*(ptCur + 1) & 0xC0) == 0xC0 && *(ptCur + 1) != 0xFF )
                    {
                        ptCur += (((ptCur[2] << 8) | ptCur[3]) + 2);
                    }
                    else
                        ptCur++;
                    break;
            }
        }
    }

end:
    if( bFindSmallJpeg == true )
        pJAttrib->flag |= JATT_STATUS_HAS_SMALL_THUMB;
    else
        pJAttrib->flag &= ~JATT_STATUS_HAS_SMALL_THUMB;

    return result;
}

static JPG_ERR
_Parse_App02(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    JPG_ERR             result = JPG_ERR_OK;
    JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;
    JPG_ATTRIB          *pJAttrib = &pJPrsInfo->jAttrib;
    bool                bFindLargeJpeg = false;
    bool                bFindCtntLst = false;
    bool                bFindNullT = false;
    bool                bFindSOI = false;
    uint8_t             *ptCur = 0, *ptEnd = 0;
    uint32_t            entityCnt = 0, entitySize = 0;
    uint8_t             Default_Value[4];
    uint32_t            stage;
    int                 jpegIdx;
    uint16_t            tmpValue = 0;
    uint32_t            i, j, k;

    if( pJAttrib->exifAppCnt < 2 )
        goto end;

    i = 0;
    // ----------------------------------------------
    // Find Contents list
    while( !bFindCtntLst && (i < pJAttrib->exifAppCnt) )
    {
        i++;
        // seek position
        jPrs_Seek2SectionStart(pHJStream, pJPrsInfo, pJAttrib->exifAppOffset[i], pJAttrib->exifAppLength[i]);

        ptCur = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos;
        ptEnd = ptCur + pJAttrib->exifAppLength[i];

        if( *(ptCur + APP2_OFFSET_TYPE) == APP2_CONTENTS_LIST &&
            memcmp((ptCur + 2), "FPXR", 4) == 0 ) // FPXR
        {
            bool          bStorageCase;

            ptCur += APP2_OFFSET_LIST_INDEX;

            entityCnt = *(ptCur) << 8 | *(ptCur + 1);
            //jpg_msg(1, "EntityCnt %d\n", EntityCnt);
            ptCur += 2;

            // Get the Default value
            for(j = 0, k = 0 ; j < entityCnt; j++)
            {
                entitySize = GetDWord(pJPrsInfo->bLEndian, ptCur);
                ptCur += 4;

                if( entitySize == 0xFFFFFFFF )
                {
                    bStorageCase = true;
                    ptCur++;
                }
                else
                {
                    bStorageCase = false;
                    Default_Value[k++] = *ptCur++;
                    //jpg_msg(1, "Default Value %d\n", Default_Value[k-1]);
                }

                // search NULL termination
                stage = 0;
                bFindNullT = false;

                while( ptCur < ptEnd && !bFindNullT )
                {
                    if( *ptCur++ == 0x00 )
                    {
                        stage++;
                        if (stage == 3)
                            bFindNullT = true;
                    }
                    else
                        stage = 0;
                }

                if( bStorageCase )
                    ptCur += 16;
            }

            bFindCtntLst = true;
        }
    }

    if( !bFindCtntLst )
        goto end;

    jpegIdx = -1;
    // ----------------------------------------------
    // Parse second jpeg
    while( i < pJAttrib->exifAppCnt )
    {
        i++;
        // seek position
        jPrs_Seek2SectionStart(pHJStream, pJPrsInfo, pJAttrib->exifAppOffset[i], pJAttrib->exifAppLength[i]);

        ptCur = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos;
        ptEnd = ptCur + pJAttrib->exifAppLength[i];

        if( (*(ptCur + APP2_OFFSET_TYPE)) == APP2_STREAM_DATA )
        {
            if(  bFindSOI == false )
                jpegIdx = *(ptCur + APP2_OFFSET_LIST_INDEX) << 8 | *(ptCur + APP2_OFFSET_LIST_INDEX + 1);
            else
            {
                pJAttrib->app2StreamCnt++;
                if( (*(ptCur + APP2_OFFSET_LIST_INDEX) << 8 | *(ptCur + APP2_OFFSET_LIST_INDEX + 1)) != jpegIdx )
                {
                    jpg_msg(0, "0x%02x, 0x%02x\n", *(ptCur + APP2_OFFSET_LIST_INDEX), *(ptCur + APP2_OFFSET_LIST_INDEX + 1));
                    pJAttrib->app2StreamCnt--;
                    continue;
                }

                ptCur += APP2_OFFSET_STREAM_DATA;

                // skip default value
                while( *ptCur == Default_Value[jpegIdx-1] )
                    ptCur++;

                pJAttrib->app2StreamOffset[pJAttrib->app2StreamCnt] = _GetCurFactorPos(pHJStream, pJPrsBsCtrl->realSize, (ptCur - pJPrsBsCtrl->bsPrsBuf));
                pJAttrib->app2StreamSize[pJAttrib->app2StreamCnt]   = ptEnd - ptCur;

            }

            while( ptCur < ptEnd )
            {
                tmpValue = GetWord(pJPrsInfo->bLEndian, ptCur);
                switch( tmpValue )
                {
                case 0xFFD8:
                    bFindSOI = true;
                    pJAttrib->app2StreamOffset[pJAttrib->app2StreamCnt] = _GetCurFactorPos(pHJStream, pJPrsBsCtrl->realSize, (ptCur - pJPrsBsCtrl->bsPrsBuf));
                    pJAttrib->app2StreamSize[pJAttrib->app2StreamCnt]   = ptEnd - ptCur;
                    ptCur += 2;
                    break;

                case 0xFFC2:
                    pJAttrib->jBaseLiteInfo[JPG_DEC_LARGE_THUMB].bJprog = true;
                case 0xFFC0:
                    pJAttrib->jBaseLiteInfo[JPG_DEC_LARGE_THUMB].width  = (*(ptCur + 7) << 8) | *(ptCur + 8);
                    pJAttrib->jBaseLiteInfo[JPG_DEC_LARGE_THUMB].height = (*(ptCur + 5) << 8) | *(ptCur + 6);
                    ptCur += (((ptCur[2] << 8) | ptCur[3]) + 2);
                    break;

                case 0xFFD9:
                    if( bFindSOI == true )
                    {
                        bFindLargeJpeg = true;
                        pJAttrib->app2StreamCnt++;
                        goto end;
                    }
                    break;

                default:
                    if( *ptCur == 0xFF && (*(ptCur + 1) & 0xC0) == 0xC0 && *(ptCur + 1) != 0xFF )
                    {
                        ptCur += (((ptCur[2] << 8) | ptCur[3]) + 2);
                    }
                    else
                        ptCur++;
                    break;
                }

            }
        }
    }


end:
    if( bFindLargeJpeg == true )
        pJAttrib->flag |= JATT_STATUS_HAS_LARGE_THUMB;
    else
        pJAttrib->flag &= ~JATT_STATUS_HAS_LARGE_THUMB;

    return result;
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
void
jPrs_Seek2SectionStart(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo,
    uint32_t            destPos,
    uint32_t            sectLength)
{
    /**
     *                    |---------- realSize ---------|
     *              prevStreamPos    destPos       curStreamPos
     *     ---------------|-------------i---------------|----------|
     *
     *      ps. MUST be "sectLength < bsPrsBufMaxLeng"
     **/
    uint32_t            curStreamPos = 0;
    uint32_t            prevStreamPos = 0;
    JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;
    JPG_STREAM_DESC     *pJStreamDesc = &pHJStream->jStreamDesc;

    if( pJStreamDesc->jTell_stream )
        pJStreamDesc->jTell_stream(pHJStream, &curStreamPos, 0);

    prevStreamPos = curStreamPos - pJPrsBsCtrl->realSize;

    if( prevStreamPos < destPos && (destPos + sectLength) < curStreamPos )
    {
        // one bs buffer case
        pJPrsBsCtrl->curPos = destPos - prevStreamPos;
    }
    else
    {
        // multi-bs buffer case
        if( pJStreamDesc->jSeek_stream )
            pJStreamDesc->jSeek_stream(pHJStream, destPos, JPG_SEEK_SET, 0);

        if( pJStreamDesc->jFull_buf )
            pJStreamDesc->jFull_buf(pHJStream, pJPrsBsCtrl->bsPrsBuf,
                                    pJPrsBsCtrl->bsPrsBufMaxLeng,
                                    &pJPrsBsCtrl->realSize, 0);
        pJPrsBsCtrl->curPos = 0;

        pJPrsInfo->remainSize = pJPrsInfo->fileLength - (destPos + pJPrsBsCtrl->realSize);
    }
}

// parsing base jpg info, ex. Q-table, H-thable, ...etc.
JPG_ERR
jPrs_BaseParser(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    JPG_ERR             result = JPG_ERR_OK;
    bool                bFindHTable = false;
    JPG_STREAM_DESC     *pJStreamDesc = &pHJStream->jStreamDesc;
    JPG_PRS_BS_CTRL     *pJPrsBsCtrl = &pJPrsInfo->jPrsBsCtrl;
    JPG_ATTRIB          *pJAttrib = &pJPrsInfo->jAttrib;
    uint8_t             *ptCur, *ptEnd;

    // parsing process
    ptCur = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->curPos;

    if( pJPrsBsCtrl->b1stSection == true )
    {
        pJPrsBsCtrl->b1stSection = false;

        // for the swf jpeg
        if( (*ptCur == JPG_MARKER_START) &&
            (*(ptCur + 1) == JPG_END_OF_IMAGE_MARKER) )
        {
            ptCur += 2;
        }

        if( (*ptCur == JPG_MARKER_START) &&
            (*(ptCur + 1) == JPG_START_OF_IMAGE_MARKER) )
        {
            ptCur += 2;
        }
        else
        {
            pJAttrib->flag |= ( JATT_STATUS_UNSUPPORT_PRIMARY |
                               JATT_STATUS_UNSUPPORT_SMALL_THUMB |
                               JATT_STATUS_UNSUPPORT_LARGE_THUMB );
            result = JPG_ERR_NOT_JPEG;
            goto end;
        }
    }

    ptEnd = pJPrsBsCtrl->bsPrsBuf + pJPrsBsCtrl->realSize;
    pJPrsInfo->jFrmComp.bFindHDT = false;

    // parsing process
    while( *ptCur == JPG_MARKER_START )
    {
        switch( *(ptCur + 1) )
        {
            default:
                if( (*(ptCur + 1) & 0xF0) == 0xE0 ||
                    (*(ptCur + 1) & 0xF0) == 0xF0 )
                {
                    int   sectionLength = 0;

                    ptCur += 2;
                    sectionLength = GetWord(pJPrsInfo->bLEndian, ptCur);

                    if( (ptCur + sectionLength) < ptEnd )
                        ptCur += sectionLength;
                    else
                    {
                        int   skipLength = 0;

                        skipLength = sectionLength - ((uint32_t)ptEnd - (uint32_t)ptCur);
                        if( pJStreamDesc->jSeek_stream )
                            pJStreamDesc->jSeek_stream(pHJStream, skipLength, JPG_SEEK_CUR, 0);

                        pJPrsInfo->remainSize -= skipLength;

                        if( pJStreamDesc->jFull_buf )
                            pJStreamDesc->jFull_buf(pHJStream, pJPrsBsCtrl->bsPrsBuf,
                                                    pJPrsBsCtrl->bsPrsBufMaxLeng,
                                                    &pJPrsBsCtrl->realSize, 0);
                        // reset parameters
                        ptCur = pJPrsBsCtrl->bsPrsBuf;
                        pJPrsInfo->remainSize -= pJPrsBsCtrl->realSize;
                    }
                }
                else
                    ptCur++;
                break;

            case JPG_START_OF_IMAGE_MARKER:
                ptCur += 2;
                break;
            case JPG_END_OF_IMAGE_MARKER:
                ptCur += 2;
                result = JPG_ERR_HDER_ONLY_TABLES;
                goto end;
                break;

			case JPG_NO_MEANING_MARKER:
				ptCur++;
				break;

            case JPG_DIFF_PROGRESSIVE:
            case JPG_DIFF_LOSSLESS:
            case JPG_DIFF_SEQUENTIAL:
            case JPG_EXTENDED_SEQUENTIAL:
            case JPG_LOSSLESS_SEQUENTIAL:
                result = JPG_ERR_HW_NOT_SUPPORT;
                jpg_msg(JPG_MSG_TYPE_ERR, "H/W not support !!\n");
                goto end;
                break;

            case JPG_PROGRESSIVE:
                result = JPG_ERR_JPROG_STREAM;
                //jpg_msg(JPG_MSG_TYPE_ERR, "Jprog data, please call jPrs_JprogBaseParser() !!\n");
                jpg_msg(JPG_MSG_TYPE_ERR, "Jprog data, not support !!\n");
                goto end;
                break;

            case JPG_BASELINE_MARKER:
                ptCur += 2;
                ptCur = _CompleteSection(pHJStream, ptCur, ptEnd, pJPrsInfo);

                ptCur = _ParseSOF00(pHJStream, ptCur, pJPrsInfo);
                break;

            case JPG_HUFFMAN_TABLE_MARKER:
                ptCur += 2;
                ptCur = _CompleteSection(pHJStream, ptCur, ptEnd, pJPrsInfo);

                if( pJPrsInfo->bSkipHDT == false )
                    ptCur = _ParseDHT(pHJStream, ptCur, pJPrsInfo);
                else
                {
                    ptCur += GetWord(pJPrsInfo->bLEndian, ptCur);
                }

                pJPrsInfo->jFrmComp.bFindHDT = true;
                bFindHTable = true;
                break;

            case JPG_Q_TABLE_MARKER:
                ptCur += 2;
                ptCur = _CompleteSection(pHJStream, ptCur, ptEnd, pJPrsInfo);

                ptCur = _ParseDQT(pHJStream, ptCur, pJPrsInfo);
                break;

            case JPG_DRI_MARKER:
                ptCur += 2;
                ptCur = _CompleteSection(pHJStream, ptCur, ptEnd, pJPrsInfo);

                ptCur = _ParseDRI(pHJStream, ptCur, pJPrsInfo);
                break;

            case JPG_START_OF_SCAN_MARKER:
                ptCur += 2;
                ptCur = _CompleteSection(pHJStream, ptCur, ptEnd, pJPrsInfo);

                ptCur = _ParseSOS(pHJStream, ptCur, pJPrsInfo);
                break;

            case 0x00: // the first entropy code may be "0xFF 0x00"
                goto end;
                break;
        }

        // check remain bs_buf size
    }

    if( bFindHTable == false )
    {
        // loading default H-Table
        _SetDefHTable(pHJStream, pJPrsInfo);
        jpg_msg(JPG_MSG_TYPE_ERR, "JPEG loss H-table section, use default table !!\n");
    }

end:
    pJPrsBsCtrl->curPos = ptCur - pJPrsBsCtrl->bsPrsBuf;

    if( result != JPG_ERR_OK && result != JPG_ERR_HDER_ONLY_TABLES && result != JPG_ERR_JPROG_STREAM)
        jpg_msg_ex(JPG_MSG_TYPE_ERR, "%s() Err 0x%x ", __FUNCTION__, result);

    return result;
}

// parsing app section info, ex. EXIF, 0xE0 ~ 0xEF, 0xF0 ~ 0xFF
JPG_ERR
jPrs_AppParser(
    JPG_STREAM_HANDLE   *pHJStream,
    JPG_PARSER_INFO     *pJPrsInfo)
{
    JPG_ERR         result = JPG_ERR_OK;

    // -----------------------------------
    // get App sections position
    result = _AppSectionOverview(pHJStream, pJPrsInfo);
    if( result != JPG_ERR_OK )
    {
        jpg_msg_ex(JPG_MSG_TYPE_ERR, " Jpg err 0x%x ! ", result);
        goto end;
    }

    // ---------------------------------
    // detail parse app and get thumbnail position
    // app1
    _Parse_App01(pHJStream, pJPrsInfo);

    // app2
    _Parse_App02(pHJStream, pJPrsInfo);

end:
    return result;
}

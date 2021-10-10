
#include "jpg_defs.h"
#include "jpg_hw.h"
#include "jpg_extern_link.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================
uint16_t jpgCompCtrl[5] =
{
    0,
    JPG_MSK_LINE_BUF_COMPONENT_1_VALID,
    (JPG_MSK_LINE_BUF_COMPONENT_1_VALID | JPG_MSK_LINE_BUF_COMPONENT_2_VALID),
    (JPG_MSK_LINE_BUF_COMPONENT_1_VALID | JPG_MSK_LINE_BUF_COMPONENT_2_VALID | JPG_MSK_LINE_BUF_COMPONENT_3_VALID),
    (JPG_MSK_LINE_BUF_COMPONENT_1_VALID | JPG_MSK_LINE_BUF_COMPONENT_2_VALID | JPG_MSK_LINE_BUF_COMPONENT_3_VALID | JPG_MSK_LINE_BUF_COMPONENT_4_VALID)
};

JPG_YUV_TO_RGB yuv2RgbMatrix =
{
    0x0100, // _11
    0x0167, // _13
    0x0100, // _21
    0x07a8, // _22
    0x0749, // _23
    0x0100, // _31
    0x01c6, // _32
    0x034d, // ConstR
    0x0089, // ConstG
    0x031e, // ConstB
    0,      // Reserved
};

uint8_t JPGTilingTable[5][32] =
{ 
  //{  0,  1,  2,  3, 4, 5, 6,  7,  8,  9, 10,  11, 12,  13,  14, 15,  // pitch = 512
  //   16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2,  9, 10, 11, 12,  3,  4,  5, 13,  6, 14,  7,  8, 15,  // pitch = 512
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 10, 11, 12, 13,  3,  4,  5, 14,  6, 15,  7,  8,  9,  // pitch = 1024
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 11, 12, 13, 14,  3,  4,  5, 15,  6, 16,  7,  8,  9,  // pitch = 2048
    10, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 12, 13, 14, 15,  3,  4,  5, 16,  6, 7,  8,  9,  10,  // pitch = 4096
     11, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  //{  0,  1,  2, 12, 13, 14, 15,  3,  4,  5, 16,  6, 17,  7,  8,  9,  // pitch = 4096
  //  10, 11, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 },
  {  0,  1,  2, 13, 14, 15, 16,  3,  4,  5, 17,  6, 18,  7,  8,  9,  // pitch = 8096
    10, 11, 12, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 } };
  
//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint8_t RemapTableSel(uint16_t pitch)
{
    uint8_t index;
    
    switch (pitch)
    {
    case 512:
        index = 0;
        break;
    case 1024:
    	  index = 1;
        break;
    case 2048:
    	  index = 2;
        break;
    case 4096:
        index = 3;
        break;
    case 8192:
        index = 4;
        break;
    default:
    	  index = 0;
        break;
    }
    
    return index;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
/////////////////////////////////////////////////
// Base Engine API
////////////////////////////////////////////////
void
_LogReg(
    bool    bMsgOn)
{
    JPG_REG   reg, p;
    uint32_t  i, j = 0, count = 0;

    if( bMsgOn == false )       return;

    reg     = REG_JPG_BASE;
    count   = (0x0B12 - REG_JPG_BASE) / sizeof(JPG_REG);
    p = reg;

    jpg_msg(1, "\n\t   0    2    4    6    8    A    C    E\r\n");
    for(i = 0; i < count; ++i)
    {
        JPG_REG   value = 0;

        jpgReadReg(p, &value);
        if( j == 0 )    jpg_msg(1, "0x%04X:", p);

        jpg_msg(1, " %04X", value);
        if( j >= 7 )
        {
            jpg_msg(1, "\r\n");
            j = 0;
        }
        else
            j++;

        p += 2;
    }

    if( j > 0 )
        jpg_msg(1, "\r\n");

    return;
}

void
_LogRemapReg(
    bool    bMsgOn)
{
    JPG_REG   reg, p;
    uint32_t  i, j = 0, count = 0;

    if( bMsgOn == false )       return;

    reg     = REG_JPG_BASE_EXT;
    count   = (0x0F44 - REG_JPG_BASE_EXT) / sizeof(JPG_REG);
    p = reg;

    jpg_msg(1, "\n\t   0    2    4    6    8    A    C    E\r\n");
    for(i = 0; i < count; ++i)
    {
        JPG_REG   value = 0;

        jpgReadReg(p, &value);
        if( j == 0 )    jpg_msg(1, "0x%04X:", p);

        jpg_msg(1, " %04X", value);
        if( j >= 7 )
        {
            jpg_msg(1, "\r\n");
            j = 0;
        }
        else
            j++;

        p += 2;
    }

    if( j > 0 )
        jpg_msg(1, "\r\n");

    return;
}


void
JPG_RegReset(
    void)
{
    jpgResetHwReg();
	if (ithGetDeviceId() == 0x9850 && ithGetRevisionId() == 0)
	{}else
	    jpgResetHwEngine();
}

void
JPG_PowerUp(
    void)
{
    // MM9070: JCLK max=133MHz,
    // and need to enable M7CLK (memory interface), XCLK (some module share with video)
    jpgEnableVideoClock();
    jpgEnableClock();

    JPG_RegReset();
}

//=============================================================================
/**
 * JPG encoder Power down (No isp)
 *
 * @param void
 * @return void
 */
//=============================================================================
void
JPG_EncPowerDown(
    void)
{
    volatile JPG_REG   hwStatus = 0;
    uint16_t           timeOut = 0;

    hwStatus = JPG_GetEngineStatusReg();
    if( (hwStatus&0xF800) == 0x3000 )
    {
        jpg_sleep(2);
        hwStatus = JPG_GetEngineStatusReg();
        if( (hwStatus&0xF800) == 0x3000 )
        {
            JPG_RegReset();
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "time out !!");
            goto end;
        }
    }

    hwStatus = JPG_GetProcStatusReg();
    while( !(hwStatus & JPG_STATUS_ENCODE_COMPLETE) )
    {
        jpg_sleep(10);
        timeOut++;
        if( timeOut > 100 )
        {
            // 1 sec timeOut
            JPG_RegReset();
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "time out (status= 0x%x)!!", hwStatus);
            break;
        }
        hwStatus = 0;
        hwStatus = JPG_GetProcStatusReg();
    }

end :
    // ------------------------------
    // for power consumption
    timeOut = 0;
    do
    {
        // Wait JPG streaming buffer empty, avoid cpu hang
        hwStatus = JPG_GetEngineStatusReg();
        if( (hwStatus & 0xc000) != 0 && timeOut < 1000 )
           jpg_sleep(1);
        else
            break;
    } while(timeOut++);
    JPG_RegReset();      // clear regist value
    jpgDisableClock();   // disable clock
    // -------------------------------------
    return;

}


//=============================================================================
/**
 * JPG decoder Power down
 *
 * @param void
 * @return void
 */
//=============================================================================
void
JPG_DecPowerDown(
    void)
{
    volatile JPG_REG   hwStatus = 0;
    uint16_t           timeOut = 0;

    hwStatus = JPG_GetEngineStatusReg();
    if( (hwStatus&0xF800) == 0x3000 )
    {
        jpg_sleep(1);
        hwStatus = JPG_GetEngineStatusReg();
        if( (hwStatus&0xF800) == 0x3000 )
        {
            JPG_RegReset();
            jpg_sleep(500);
            Jpg_Ext_Link_Ctrl(JEL_CTRL_ISP_HW_RESET, 0, 0);
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "time out !!");
            goto end;
        }
    }

    hwStatus = JPG_GetProcStatusReg();
    while( !(hwStatus & JPG_STATUS_DECODE_COMPLETE) )
    {
        jpg_sleep(1);
       
        timeOut++;
        if( timeOut > 50 )
        {
            // 1 sec timeOut
            JPG_RegReset();
            jpg_sleep(500);
            Jpg_Ext_Link_Ctrl(JEL_CTRL_ISP_HW_RESET, 0, 0);
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "time out (status[0xB00]= 0x%x)!!", hwStatus);
            break;
        }
        hwStatus = 0;
        hwStatus = JPG_GetProcStatusReg();
    }

end :
    // ------------------------------
    // for power consumption
    // maybe have side effect (When enter suspend, jpg no return ack and then isp busy ??)
    timeOut = 0;
    do
    {
        // Wait JPG streaming buffer empty, avoid cpu hang
        hwStatus = JPG_GetEngineStatusReg();
        if ((hwStatus & 0xc000) != 0 && timeOut < 1000)
           jpg_sleep(1);
        else
            break;
    } while(timeOut++);
    JPG_RegReset();      // clear regist value
    jpgDisableClock();   // disable clock
    // -------------------------------------
    return;
}

/////////////////////////////////////////////////
// Access Register API
////////////////////////////////////////////////
// 0x00
void
JPG_SetCodecCtrlReg(
    JPG_REG   data)
{
    jpgWriteRegMark(REG_JPG_CODEC_CTRL, (JPG_REG)data, (JPG_REG)(~JPG_MSK_BITSTREAM_READ_BYTE_POS));
}

void
JPG_SetBitstreamReadBytePosReg(
    JPG_REG   data)
{
    jpgWriteRegMark(REG_JPG_CODEC_CTRL,
                   (JPG_REG)(data << JPG_SHT_BITSTREAM_READ_BYTE_POS),
                   JPG_MSK_BITSTREAM_READ_BYTE_POS);
}

// 0x02
void
JPG_SetDriReg(
    JPG_REG   data)
{
    jpgWriteReg(REG_JPG_DRI_SETTING, (JPG_REG)data);
}

// 0x04
void
JPG_SetTableSpecifyReg(
    const JPG_FRM_COMP      *frmComp)
{
    JPG_REG     data = 0;
    // Hw issue
    uint16_t    qTableSel_Y  = 0x0;  // 00
    uint16_t    qTableSel_UV = 0x1;  // 01

    data = (frmComp->jFrmInfo[0].acHuffTableSel << JPG_SHT_COMPONENT_A_AC_HUFFMAN_TABLE) |
           (frmComp->jFrmInfo[0].dcHuffTableSel << JPG_SHT_COMPONENT_A_DC_HUFFMAN_TABLE) |
           (qTableSel_Y << JPG_SHT_COMPONENT_A_Q_TABLE) |
           (frmComp->jFrmInfo[1].acHuffTableSel << JPG_SHT_COMPONENT_B_AC_HUFFMAN_TABLE) |
           (frmComp->jFrmInfo[1].dcHuffTableSel << JPG_SHT_COMPONENT_B_DC_HUFFMAN_TABLE) |
           (qTableSel_UV << JPG_SHT_COMPONENT_B_Q_TABLE) |
           (frmComp->jFrmInfo[2].acHuffTableSel << JPG_SHT_COMPONENT_C_AC_HUFFMAN_TABLE) |
           (frmComp->jFrmInfo[2].dcHuffTableSel << JPG_SHT_COMPONENT_C_DC_HUFFMAN_TABLE) |
           (qTableSel_UV << JPG_SHT_COMPONENT_C_Q_TABLE) |
           (frmComp->jFrmInfo[3].acHuffTableSel << JPG_SHT_COMPONENT_D_AC_HUFFMAN_TABLE) |
           (frmComp->jFrmInfo[3].dcHuffTableSel << JPG_SHT_COMPONENT_D_DC_HUFFMAN_TABLE) |
           (frmComp->jFrmInfo[3].qTableSel << JPG_SHT_COMPONENT_D_Q_TABLE);

    jpgWriteReg(REG_JPG_TABLE_SPECIFY, data);
}

// 0x06, 0x08, 0xF0~0xFA
void
JPG_SetFrmSizeInfoReg(
    const JPG_FRM_SIZE_INFO     *sizeInfo)
{
    jpgWriteRegMark(REG_JPG_DISPLAY_MCU_WIDTH_Y, sizeInfo->mcuDispWidth, JPG_MSK_MCU);
    jpgWriteRegMark(REG_JPG_DISPLAY_MCU_HEIGHT_Y, sizeInfo->mcuDispHeight, JPG_MSK_MCU);

    jpgWriteRegMark(REG_JPG_ORIGINAL_MCU_WIDTH, sizeInfo->mcuRealWidth, JPG_MSK_MCU);
    jpgWriteRegMark(REG_JPG_ORIGINAL_MCU_HEIGHT, sizeInfo->mcuRealHeight, JPG_MSK_MCU);

    // set processed MCU range
    jpgWriteRegMark(REG_JPG_LEFT_MCU_OFFSET, (JPG_REG)(sizeInfo->mcuDispLeft + 1), JPG_MSK_MCU);
    jpgWriteRegMark(REG_JPG_RIGHT_MCU_OFFSET, sizeInfo->mcuDispRight, JPG_MSK_MCU);
    jpgWriteRegMark(REG_JPG_UP_MCU_OFFSET, (JPG_REG)(sizeInfo->mcuDispUp + 1), JPG_MSK_MCU);
    jpgWriteRegMark(REG_JPG_DOWN_MCU_OFFSET, sizeInfo->mcuDispDown, JPG_MSK_MCU);
}

// 0x0A~0x1A
void
JPG_SetLineBufInfoReg(
    const JPG_LINE_BUF_INFO     *lineBufInfo)
{
    uint32_t addr = 0;

    // component 1 starting address
    addr = (uint32_t)lineBufInfo->comp1Addr;
    addr >>= 2;

    jpgWriteRegMark(REG_JPG_LINE_BUF_ADDR_A_COMPONENT_L, (JPG_REG)(addr & JPG_MSK_LINE_BUF_ADDR_L), JPG_MSK_LINE_BUF_ADDR_L);
    jpgWriteRegMark(REG_JPG_LINE_BUF_ADDR_A_COMPONENT_H, (JPG_REG)((addr >> 16) & JPG_MSK_LINE_BUF_ADDR_H), JPG_MSK_LINE_BUF_ADDR_H);

    // component 2 starting address
    addr = (uint32_t)lineBufInfo->comp2Addr;
    addr >>= 2;

    jpgWriteRegMark(REG_JPG_LINE_BUF_ADDR_B_COMPONENT_L, (JPG_REG)(addr & JPG_MSK_LINE_BUF_ADDR_L), JPG_MSK_LINE_BUF_ADDR_L);
    jpgWriteRegMark(REG_JPG_LINE_BUF_ADDR_B_COMPONENT_H, (JPG_REG)((addr >> 16) & JPG_MSK_LINE_BUF_ADDR_H), JPG_MSK_LINE_BUF_ADDR_H);

    // component 3 starting address
    addr = (uint32_t)lineBufInfo->comp3Addr;
    addr >>= 2;

    jpgWriteRegMark(REG_JPG_LINE_BUF_ADDR_C_COMPONENT_L, (JPG_REG)(addr & JPG_MSK_LINE_BUF_ADDR_L), JPG_MSK_LINE_BUF_ADDR_L);
    jpgWriteRegMark(REG_JPG_LINE_BUF_ADDR_C_COMPONENT_H, (JPG_REG)((addr >> 16) & JPG_MSK_LINE_BUF_ADDR_H), JPG_MSK_LINE_BUF_ADDR_H);

    // component 1 pitch
    jpgWriteRegMark(REG_JPG_LINE_BUF_PITCH_COMPONENT_A,
                   (JPG_REG)((lineBufInfo->comp1Pitch >> 2) & JPG_MSK_LINE_BUF_PITCH),
                   JPG_MSK_LINE_BUF_PITCH);
    // component 2/3 pitch
    jpgWriteRegMark(REG_JPG_LINE_BUF_PITCH_COMPONENT_BC,
                   (JPG_REG)((lineBufInfo->comp23Pitch >> 2) & JPG_MSK_LINE_BUF_PITCH),
                   JPG_MSK_LINE_BUF_PITCH);
}

// 0x16
void
JPG_SetLineBufSliceUnitReg(
    JPG_REG   data,
    JPG_REG   yVerticalSamp)
{
    jpgWriteRegMark(REG_JPG_LINE_BUF_SLICE_NUM,
                   (JPG_REG)((data / yVerticalSamp) & JPG_MSK_LINE_BUF_SLICE_NUM),
                   JPG_MSK_LINE_BUF_SLICE_NUM);
}

// 0x1C
void
JPG_SetLineBufSliceWriteNumReg(
    JPG_REG   data)
{
    // for encode, set how many slice is ready to encode.
    jpgWriteRegMark(REG_JPG_LINE_BUF_SLICE_WRITE,
                   (JPG_REG)(data & JPG_MSK_LINE_BUF_SLICE_WRITE),
                   JPG_MSK_LINE_BUF_SLICE_WRITE);
}

// 0x1E, 0x20, 0x22, 0x24
void
JPG_SetBitStreamBufInfoReg(
    const JPG_HW_BS_CTRL   *bsBufInfo)
{
    uint32_t    addr = 0;
    uint32_t    size = 0;

    // Bit-Stream buffer starting address
    addr = (uint32_t)bsBufInfo->addr;
    addr >>= 2;

    jpgWriteRegMark(REG_JPG_BITSTREAM_BUF_ADDR_L, (JPG_REG)(addr & JPG_MSK_BITSTREAM_BUF_ADDR_L), JPG_MSK_BITSTREAM_BUF_ADDR_L);
    jpgWriteRegMark(REG_JPG_BITSTREAM_BUF_ADDR_H, (JPG_REG)((addr >> 16) & JPG_MSK_BITSTREAM_BUF_ADDR_H), JPG_MSK_BITSTREAM_BUF_ADDR_H);

    //Bit-Stream buffer size
    size = bsBufInfo->size >> 2;
    jpgWriteRegMark(REG_JPG_BITSTREAM_BUF_SIZE_L, (JPG_REG)(size & JPG_MSK_BITSTREAM_BUF_SIZE_L), JPG_MSK_BITSTREAM_BUF_SIZE_L);
    jpgWriteRegMark(REG_JPG_BITSTREAM_BUF_SIZE_H, (JPG_REG)((size >> 16) & JPG_MSK_BITSTREAM_BUF_SIZE_H), JPG_MSK_BITSTREAM_BUF_SIZE_H);
}

// 0x26, 0x28
void
JPG_SetBitstreamBufRwSizeReg(
    uint32_t    data)
{
    uint32_t    wrSize = data >> 2;

    // Use Ring buf, Read: H/W remain buf size (endoder), Write: S/W move out data length (decoder)
    jpgWriteRegMark(REG_JPG_BITSTREAM_RW_SIZE_L, (JPG_REG)(wrSize & JPG_MSK_BITSTREAM_RW_SIZE_L), JPG_MSK_BITSTREAM_RW_SIZE_L);
    jpgWriteRegMark(REG_JPG_BITSTREAM_RW_SIZE_H, (JPG_REG)((wrSize >> 16) & JPG_MSK_BITSTREAM_RW_SIZE_H), JPG_MSK_BITSTREAM_RW_SIZE_H);
}

// 0x2A, 0x2D
void
JPG_SetSamplingFactorReg(
    const JPG_FRM_COMP      *frmComp)
{
    JPG_REG   data = 0;

    data = (frmComp->jFrmInfo[0].horizonSamp  << JPG_SHT_SAMPLING_FACTOR_A_H) |
           (frmComp->jFrmInfo[0].verticalSamp << JPG_SHT_SAMPLING_FACTOR_A_V) |
           (frmComp->jFrmInfo[1].horizonSamp  << JPG_SHT_SAMPLING_FACTOR_B_H) |
           (frmComp->jFrmInfo[1].verticalSamp << JPG_SHT_SAMPLING_FACTOR_B_V);

    jpgWriteReg(REG_JPG_SAMPLING_FACTOR_AB, data);

    data = 0;
    data = (frmComp->jFrmInfo[2].horizonSamp  << JPG_SHT_SAMPLING_FACTOR_C_H) |
           (frmComp->jFrmInfo[2].verticalSamp << JPG_SHT_SAMPLING_FACTOR_C_V) |
           (frmComp->jFrmInfo[3].horizonSamp  << JPG_SHT_SAMPLING_FACTOR_D_H) |
           (frmComp->jFrmInfo[3].verticalSamp << JPG_SHT_SAMPLING_FACTOR_D_V);

    jpgWriteReg(REG_JPG_SAMPLING_FACTOR_CD, data);

    // Set MCU block height
    jpgWriteRegMark(REG_JPG_LEFT_MCU_OFFSET,
                   (JPG_REG)(frmComp->jFrmInfo[0].verticalSamp << JPG_SHT_MCU_HEIGHT_BLOCK),
                   JPG_MSK_MCU_HEIGHT_BLOCK);

    //Set MCU block number
    data = (frmComp->jFrmInfo[0].horizonSamp * frmComp->jFrmInfo[0].verticalSamp) +
           (frmComp->jFrmInfo[1].horizonSamp * frmComp->jFrmInfo[1].verticalSamp) +
           (frmComp->jFrmInfo[2].horizonSamp * frmComp->jFrmInfo[2].verticalSamp) - 1;

    jpgWriteRegMark(REG_JPG_RIGHT_MCU_OFFSET,
                   (JPG_REG)(data << JPG_SHT_BLOCK_MCU_NUM),
                   JPG_MSK_BLOCK_MCU_NUM);
}

// 0x2E~0xED
// input the Zig-zag order
void
JPG_SetQtableReg(
    JPG_FRM_COMP    *frmComp)
{
    JPG_Q_TABLE   *qTable = &frmComp->qTable;

    uint8_t       zz = 0, i = 0;
    uint8_t       *curTable = 0;
    JPG_REG       reg = REG_JPG_INDEX0_QTABLE;

    for(i = 0; i < qTable->tableCnt; i++)
    {
        curTable = qTable->table[frmComp->jFrmInfo[i].qTableSel];
        switch( i )
        {
            case 0:     reg = REG_JPG_INDEX0_QTABLE;    break;
            case 1:     reg = REG_JPG_INDEX1_QTABLE;    break;
            case 2:     reg = REG_JPG_INDEX2_QTABLE;    break;
        }

        for(zz = 0; zz < JPG_Q_TABLE_SIZE; zz += 2)
        {
            jpgWriteReg((JPG_REG)(reg + zz), (JPG_REG)(curTable[zz] | (curTable[zz+1] << 8)));
        }
    }
}

// 0xEE
void
JPG_DropHv(
    JPG_REG   data)
{
    // set jpg engine down/up sample output (ex. 444 in -> 422 out)
    jpgWriteReg(REG_JPG_DROP_DUPLICATE, (JPG_REG)(data & JPG_MSK_DUPLICATE_H_V));
}

// 0xFC
void
JPG_StartReg(
    void)
{
#if 0 // for write back case
    ithFlushDCache();
    ithInvalidateDCache();
#endif
    // fire jpg engine
    jpgWriteReg(REG_JPG_CODEC_FIRE, (JPG_REG)JPG_MSK_START_CODEC);
}

// 0xFE
JPG_REG
JPG_GetEngineStatusReg(
    void)
{
    JPG_REG value = 0;

    // H/W debug register
    jpgReadReg(REG_JPG_ENGINE_STATUS_0, &value);

    return value;
}

// 0x100
JPG_REG
JPG_GetProcStatusReg( //JPG_GetEngineStatus1Reg(
    void)
{
    JPG_REG value = 0;

    // decode/encode status (line buf and stream buf status)
    jpgReadReg(REG_JPG_ENGINE_STATUS_1,  &value);

    return value;
}

// 0x102
void
JPG_SetLineBufCtrlReg(
    JPG_REG   data)
{
    jpgWriteReg(REG_JPG_LINE_BUF_CTRL, (JPG_REG)(data & JPG_MSK_LINE_BUF_CTRL));
}

// 0x104
JPG_REG
JPG_GetLineBufValidSliceReg(
    void)
{
    JPG_REG value = 0;

    // H/W spec is not discrepted
    jpgReadReg(REG_JPG_LINE_BUF_VALID_SLICE, &value);

    return (JPG_REG)(value & JPG_MSK_LINE_BUF_VALID_SLICE);
}

// 0x106
void
JPG_SetBitstreamBufCtrlReg(
    JPG_REG   data)
{
    jpgWriteRegMark(REG_JPG_BITSTREAM_BUF_CTRL,
                   (JPG_REG)(data & JPG_MSK_BITSTREAM_BUF_CTRL),
                   JPG_MSK_BITSTREAM_BUF_CTRL);
}

// 0x108
uint32_t
JPG_GetBitStreamValidSizeReg(
    void)
{
    volatile JPG_REG  low = 0;
    volatile JPG_REG  high = 0;
    uint32_t          size = 0;

    jpgReadReg(REG_JPG_BITSTREAM_VALID_SIZE_L, (JPG_REG*)&low);
    jpgReadReg(REG_JPG_BITSTREAM_VALID_SIZE_H, (JPG_REG*)&high);
    size = ((((high & JPG_MSK_BITSTREAM_VALID_SIZE_H) << 16) | low) << 2);

    return (size);
}

// 0x10C
void
JPG_SetHuffmanCodeCtrlReg(
    JPG_HUFFMAN_TAB_SEL   tableSel,
    uint8_t               *pCodeLength)
{
    JPG_REG     Selection = 0x0000;
    JPG_REG     data = 0x0000;
    uint16_t    i = 0;

    switch( tableSel )
    {
        case JPG_HUUFFMAN_Y_DC:
            Selection = (JPG_HUFFMAN_DC_TABLE | JPG_HUFFMAN_LUMINANCE);
            break;

        case JPG_HUUFFMAN_UV_DC:
            Selection = (JPG_HUFFMAN_DC_TABLE | JPG_HUFFMAN_CHROMINANCE);
            break;

        case JPG_HUUFFMAN_Y_AC:
            Selection = (JPG_HUFFMAN_AC_TABLE | JPG_HUFFMAN_LUMINANCE);
            break;

        case JPG_HUUFFMAN_UV_AC:
            Selection = (JPG_HUFFMAN_AC_TABLE | JPG_HUFFMAN_CHROMINANCE);
            break;

        default:
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "\nWrong parameters !! ");
            return;
    }

    // Reset
    jpgWriteReg(REG_JPG_HUFFMAN_CTRL,  (JPG_REG)(Selection));

    // Write Length number
    for(i = 1; i < 16; i++)
    {
        data = (Selection | (i << 8) | (*(pCodeLength + i - 1)));
        jpgWriteReg(REG_JPG_HUFFMAN_CTRL, (JPG_REG)(data));
    }
}

// 0xB0E
void
JPG_SetDcHuffmanValueReg(
    JPG_HUFFMAN_TAB_SEL     tableSel,
    uint8_t                 *pCodeValue,
    uint16_t                totalLength)
{
    JPG_REG     data = 0x0000;
    uint16_t    wTotalHTNum = 0x0000;
    uint16_t    i = 0;

    switch( tableSel )
    {
        case JPG_HUUFFMAN_Y_DC:
            wTotalHTNum = (totalLength + 1) >> 1;

            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are the same
                if( (totalLength & 0x1) && (i == (wTotalHTNum - 1)) )
                {
                    data = (JPG_HUFFMAN_DC_LUMINANCE_TABLE | (i << 8) | (*(pCodeValue + 2 * i)));
                }
                else
                {
                    data = (JPG_HUFFMAN_DC_LUMINANCE_TABLE | (i << 8) | ((*(pCodeValue + 2 * i + 1)) << 4) | (*(pCodeValue + 2 * i)));
                }

                jpgWriteReg(REG_JPG_HUFFMAN_DC_CTRL,  (JPG_REG)data);

                /*{
                    if (!(i % 10) && i != 0)
                        jpg_msg(1, " \n");
                    jpg_msg(1, "0x%04X ", data);
                    if( i== (wTotalHTNum -1))
                        jpg_msg(1, "\n");
                }
                //*/
            }
            break;

        case JPG_HUUFFMAN_UV_DC:
            wTotalHTNum = ((totalLength + 1) >> 1);

            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are the same
                if ( (totalLength & 0x1) && (i == (wTotalHTNum - 1)) )
                {
                    data = (JPG_HUFFMAN_DC_CHROMINANCE_TABLE | (i << 8) | (*(pCodeValue + 2 * i)));
                }
                else
                {
                    data = (JPG_HUFFMAN_DC_CHROMINANCE_TABLE | (i << 8) | ((*(pCodeValue + 2 * i + 1)) << 4) | (*(pCodeValue + 2 * i)));
                }

                jpgWriteReg(REG_JPG_HUFFMAN_DC_CTRL,  (JPG_REG)data);

                /*{
                    if (!(i % 10) && i != 0)
                        jpg_msg(1, " \n");
                    jpg_msg(1, "0x%04X ", data);

                    if( i== (wTotalHTNum -1))
                        jpg_msg(1, "\n");
                }
                //*/
            }
            break;

        default:
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "\nWrong parameters !!");
            break;
    }
}

// 0x110
// 0x112
void
JPG_SetEncodeAcHuffmanValueReg(
    JPG_HUFFMAN_TAB_SEL     tableSel,
    uint8_t                 *pCodeValue,
    uint16_t                totalLength)
{
    JPG_REG     data = 0x0000;
    uint16_t    wTotalHTNum = 0x0000;
    uint16_t    i = 0;

    switch( tableSel )
    {
        case JPG_HUUFFMAN_Y_AC:
            wTotalHTNum = totalLength;

            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are different
                data = ((*(pCodeValue + i) << 8) | i);
                jpgWriteReg(REG_JPG_HUFFMAN_AC_LUMINANCE_CTRL, (JPG_REG)data);
            }
            break;

        case JPG_HUUFFMAN_UV_AC:
            wTotalHTNum = totalLength;

            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are different
                data = ((*(pCodeValue + i) << 8) | i );
                jpgWriteReg(REG_JPG_HUFFMAN_AC_CHROMINANCE_CTRL, (JPG_REG)data);
            }
            break;

        default:
            jpg_msg_ex(JPG_MSG_TYPE_ERR, "\nWrong parameters !!");
            break;
    }
}

// 0x110
// 0x112
void
JPG_SetDecodeAcHuffmanValueReg(
    JPG_HUFFMAN_TAB_SEL     tableSel,
    uint8_t                 *pCodeValue,
    uint16_t                totalLength)
{
    JPG_REG     data = 0x0000;
    uint16_t    wTotalHTNum = 0x0000;
    uint16_t    i = 0;

    switch( tableSel )
    {
        case JPG_HUUFFMAN_Y_AC:
            wTotalHTNum = totalLength;
            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are different
                data =  ((i << 8) | (*(pCodeValue + i)));
                jpgWriteReg(REG_JPG_HUFFMAN_AC_LUMINANCE_CTRL, (JPG_REG)data);
            }
            break;

        case JPG_HUUFFMAN_UV_AC:
            wTotalHTNum = totalLength;
            for(i = 0; i < wTotalHTNum; i++)
            {
                // Encode and Decode are different
                data =  ((i << 8) | (*(pCodeValue + i)));
                jpgWriteReg(REG_JPG_HUFFMAN_AC_CHROMINANCE_CTRL, (JPG_REG)data);
            }
            break;

        default:
            break;
    }
}

void
JPG_SetYuv2RgbMatrix(
    JPG_YUV_TO_RGB      *matrix)
{
    // MM9070 kill this feature
    jpgWriteReg(REG_JPG_YUV_TO_RGB_11, matrix->_11);
    jpgWriteReg(REG_JPG_YUV_TO_RGB_13, matrix->_13);
    jpgWriteReg(REG_JPG_YUV_TO_RGB_21, matrix->_21);
    jpgWriteReg(REG_JPG_YUV_TO_RGB_22, matrix->_22);
    jpgWriteReg(REG_JPG_YUV_TO_RGB_23, matrix->_23);
    jpgWriteReg(REG_JPG_YUV_TO_RGB_31, matrix->_31);
    jpgWriteReg(REG_JPG_YUV_TO_RGB_32, matrix->_32);
    jpgWriteReg(REG_JPG_YUV_TO_RGB_CONST_R, matrix->ConstR);
    jpgWriteReg(REG_JPG_YUV_TO_RGB_CONST_G, matrix->ConstG);
    jpgWriteReg(REG_JPG_YUV_TO_RGB_CONST_B, matrix->ConstB);
}

void
JPG_SetRgb565DitherKey(
    JPG_DITHER_KEY     *ditherKeyInfo)
{
    // MM9070 kill this feature
    if( ditherKeyInfo->bEnDitherKey == true )
    {
        jpgWriteReg(REG_JPG_EN_DITHER_KEY, 0x1);
        jpgWriteReg(REG_JPG_SET_DITHER_KEY, ditherKeyInfo->ditherKey);
        jpgWriteReg(REG_JPG_SET_MASK_DITHER_KEY, ditherKeyInfo->ditherKeyMask);
        jpgWriteReg(REG_JPG_SET_DITHER_KEY_BG, ditherKeyInfo->bgColor);
    }
    else
    {
        jpgWriteReg(REG_JPG_EN_DITHER_KEY, 0x0);
    }
}

void JPG_SetTilingMode(void)
{
	jpgWriteRegMark(REG_JPG_DISPLAY_MCU_WIDTH_Y, JPG_MSK_NV12_ENABLE, JPG_MSK_NV12_ENABLE);
	jpgWriteRegMark(REG_JPG_DISPLAY_MCU_HEIGHT_Y, JPG_MSK_NV12_ENABLE, JPG_MSK_NV12_ENABLE);
}

void JPG_SetTilingTable(const JPG_LINE_BUF_INFO     *lineBufInfo,
                        bool                         bEncodeMode,
                        bool                         bEnableTile)
{
    uint8_t tableYIdx , tableUVIdx;
if( ithIsTilingModeOn() == 0)
	return;
if (!(ithGetDeviceId() == 0x9850 && ithGetRevisionId() > 0))
    return;

#if (CFG_CHIP_FAMILY == 9850) 
   tableYIdx  = RemapTableSel(lineBufInfo->comp1Pitch);
   tableUVIdx = RemapTableSel(lineBufInfo->comp23Pitch);  
   if (bEnableTile)
   {
       if (bEncodeMode)
           jpgWriteRegMark(REG_JPG_LINE_BUF_ADDR_A_COMPONENT_H,  JPG_MSK_RD_TILING_ENABLE,  JPG_MSK_RD_TILING_ENABLE);
       else
           jpgWriteRegMark(REG_JPG_LINE_BUF_ADDR_A_COMPONENT_H,  JPG_MSK_WR_TILING_ENABLE,  JPG_MSK_WR_TILING_ENABLE);
       
       if (ithGetRevisionId() == 0)
       {
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_3_08H, (JPGTilingTable[tableYIdx][4] << 8)  | JPGTilingTable[tableYIdx][3]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_5_0AH, (JPGTilingTable[tableYIdx][6] << 8)  | JPGTilingTable[tableYIdx][5]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_7_0CH, (JPGTilingTable[tableYIdx][8] << 8)  | JPGTilingTable[tableYIdx][7]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_9_0EH, (JPGTilingTable[tableYIdx][10] << 8) | JPGTilingTable[tableYIdx][9]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_11_10H,(JPGTilingTable[tableYIdx][12] << 8) | JPGTilingTable[tableYIdx][11]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_13_12H,(JPGTilingTable[tableYIdx][14] << 8) | JPGTilingTable[tableYIdx][13]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_15_14H,(JPGTilingTable[tableYIdx][16] << 8) | JPGTilingTable[tableYIdx][15]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_17_16H,(JPGTilingTable[tableYIdx][18] << 8) | JPGTilingTable[tableYIdx][17]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_19_18H,(JPGTilingTable[tableYIdx][20] << 8) | JPGTilingTable[tableYIdx][19]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_21_1AH,(JPGTilingTable[tableYIdx][22] << 8) | JPGTilingTable[tableYIdx][21]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_23_1CH,(JPGTilingTable[tableYIdx][24] << 8) | JPGTilingTable[tableYIdx][23]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_25_1EH,(JPGTilingTable[tableYIdx][26] << 8) | JPGTilingTable[tableYIdx][25]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_27_20H,(JPGTilingTable[tableYIdx][28] << 8) | JPGTilingTable[tableYIdx][27]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_29_22H,(JPGTilingTable[tableYIdx][30] << 8) | JPGTilingTable[tableYIdx][29]);
           jpgWriteReg(REG_JPG_REMAP_ADR_LUM_31_24H,(JPGTilingTable[tableYIdx][32] << 8) | JPGTilingTable[tableYIdx][31]);
       }
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_3_26H, (JPGTilingTable[tableUVIdx][4] << 8)  | JPGTilingTable[tableUVIdx][3]); 
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_5_28H, (JPGTilingTable[tableUVIdx][6] << 8)  | JPGTilingTable[tableUVIdx][5]); 
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_7_2AH, (JPGTilingTable[tableUVIdx][8] << 8)  | JPGTilingTable[tableUVIdx][7]); 
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_9_2CH, (JPGTilingTable[tableUVIdx][10] << 8) | JPGTilingTable[tableUVIdx][9]); 
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_11_2EH,(JPGTilingTable[tableUVIdx][12] << 8) | JPGTilingTable[tableUVIdx][11]);
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_13_30H,(JPGTilingTable[tableUVIdx][14] << 8) | JPGTilingTable[tableUVIdx][13]);
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_15_32H,(JPGTilingTable[tableUVIdx][16] << 8) | JPGTilingTable[tableUVIdx][15]);
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_17_34H,(JPGTilingTable[tableUVIdx][18] << 8) | JPGTilingTable[tableUVIdx][17]);
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_19_36H,(JPGTilingTable[tableUVIdx][20] << 8) | JPGTilingTable[tableUVIdx][19]);
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_21_38H,(JPGTilingTable[tableUVIdx][22] << 8) | JPGTilingTable[tableUVIdx][21]);
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_23_3AH,(JPGTilingTable[tableUVIdx][24] << 8) | JPGTilingTable[tableUVIdx][23]);
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_25_3CH,(JPGTilingTable[tableUVIdx][26] << 8) | JPGTilingTable[tableUVIdx][25]);
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_27_3EH,(JPGTilingTable[tableUVIdx][28] << 8) | JPGTilingTable[tableUVIdx][27]);
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_29_40H,(JPGTilingTable[tableUVIdx][30] << 8) | JPGTilingTable[tableUVIdx][29]);
       jpgWriteReg(REG_JPG_REMAP_ADR_CHR_31_42H,(JPGTilingTable[tableUVIdx][32] << 8) | JPGTilingTable[tableUVIdx][31]);
	} else {
       jpgWriteRegMark(REG_JPG_LINE_BUF_ADDR_A_COMPONENT_H,  0,  JPG_MSK_RD_TILING_ENABLE);
       jpgWriteRegMark(REG_JPG_LINE_BUF_ADDR_A_COMPONENT_H,  0,  JPG_MSK_WR_TILING_ENABLE);
	}
#endif	
}

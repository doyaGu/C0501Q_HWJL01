/*
 * Copyright (c) 2012 ITE Technology Corp. All Rights Reserved.
 */
/** @file encoder_hardware.c
 *
 * @author
 */
 
//=============================================================================
//                              Include Files
//=============================================================================
#include "encoder/encoder_types.h"
#include "encoder/encoder_hardware.h"
#include "encoder/encoder_register.h"
#include "encoder/config.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
#define MMP_AVC_ENCODER_CLOCK_REG_34                0x0034
#define MMP_AVC_ENCODER_CLOCK_REG_36                0x0036
#define MMP_AVC_ENCODER_EN_DIV_XCLK                 0x8000  // [15]
#define MMP_AVC_ENCODER_ENCODER_RESET               0x1000  // [12]
#define MMP_AVC_ENCODER_EN_XCLK                     0x0002  // [ 2] AVC Encoder clock
#define MMP_AVC_ENCODER_EN_M7CLK                    0x0800  // [11] memory clock in AVC Encoder

//=============================================================================
//                              Global Data Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

//=============================================================================
//                              Public Function Definition
//=============================================================================


void
VPU_WriteRegister(
    MMP_UINT32 destAddress,
    MMP_UINT32 data)
{    
#if defined(__OPENRTOS__)

  #ifndef NULL_CMD
    AHB_WriteRegister(destAddress, data);
  #endif
    
#else                                     

  	HOST_WriteRegister(destAddress, (MMP_UINT16) (data & 0x0000FFFF));
    HOST_WriteRegister(destAddress+2, (MMP_UINT16) ((data & 0xFFFF0000) >> 16));
   
#endif
    //LOG_CMD "0x%08X 0x%08X\n", destAddress, data LOG_END
}

MMP_UINT32
VPU_ReadRegister(
    MMP_UINT32 destAddress)
{
#ifdef NULL_CMD
    return 1;
#endif

#if defined(__OPENRTOS__)
    MMP_UINT32 value;
    
    AHB_ReadRegister(destAddress, &value);
    
    return value;
#else                                     
    volatile MMP_UINT16 value0;
    volatile MMP_UINT16 value1;
    
    HOST_ReadRegister(destAddress, &value0);
    HOST_ReadRegister(destAddress+2, &value1);
    
  	return (value1 << 16 | value0);
#endif   
}
 
void 
BitLoadFirmware(
    MMP_UINT8* codeBase, 
    const MMP_UINT16 *codeWord, 
    MMP_UINT32 codeSize)
{
MMP_UINT32 i;
MMP_UINT32 data;
MMP_UINT8 code[8];
MMP_UINT32 addr = (MMP_UINT32)codeBase;

    //addr -= (MMP_UINT32)HOST_GetVramBaseAddress();   
#ifndef NULL_CMD
    for (i=0; i<codeSize; i+=4) {
        // 2byte little endian variable to 1byte big endian buffer
        code[0] = (MMP_UINT8)(codeWord[i+0]>>8);
        code[1] = (MMP_UINT8)codeWord[i+0];
        code[2] = (MMP_UINT8)(codeWord[i+1]>>8);
        code[3] = (MMP_UINT8)codeWord[i+1];
        code[4] = (MMP_UINT8)(codeWord[i+2]>>8);
        code[5] = (MMP_UINT8)codeWord[i+2];
        code[6] = (MMP_UINT8)(codeWord[i+3]>>8);
        code[7] = (MMP_UINT8)codeWord[i+3];     
        HOST_WriteBlockMemory((MMP_UINT32) addr+i*2, (MMP_UINT32) &code, 8);        
    }   
#endif    
    VPU_WriteRegister(BIT_INT_ENABLE, 0);
    VPU_WriteRegister(BIT_CODE_RUN, 0);
    
    for (i=0; i<2048; ++i) {
        data = codeWord[i];
        VPU_WriteRegister(BIT_CODE_DOWN, (i << 16) | data);
    }	
}

MMP_BOOL 
WaitIdle(
    MMP_UINT32 timeout)
{
VIDEO_ERROR_CODE error = VIDEO_ERROR_SUCCESS;

#ifdef NULL_CMD
    return MMP_TRUE;
#endif		
    // wait idle
    while (VPU_ReadRegister(BIT_BUSY_FLAG))
    {	 
       if (timeout == 0)
       {
           return MMP_FALSE;
       }
       PalSleep(1);
       timeout--;
    }
  	return MMP_TRUE;
}

void 
IssueCommand(
    MMP_UINT32 instanceNum, 
    MMP_UINT32 cmd)
{      
    VPU_WriteRegister(BIT_BUSY_FLAG, 1);
    VPU_WriteRegister(BIT_RUN_INDEX, instanceNum);
    VPU_WriteRegister(BIT_RUN_COD_STD, 0x8); // always H.264 encoder
    VPU_WriteRegister(BIT_RUN_AUX_STD, 0);
    VPU_WriteRegister(BIT_RUN_COMMAND, cmd);
}	  

void
encoder_hardware_SetBufAddr_Reg(
    MMP_UINT32 reg,
    MMP_UINT8* pAddr)
{
MMP_UINT32 addr = (MMP_UINT32)pAddr;
MMP_UINT16 regAddrLo = 0;
MMP_UINT16 regAddrHi = 0;
        
    //addr -= (MMP_UINT32)HOST_GetVramBaseAddress();
    
    VPU_WriteRegister(reg, (MMP_UINT32) addr);  
}

void
encoder_hardware_SetGDI_Reg(
    MMP_UINT32 baseAddr)
{
    VPU_WriteRegister(GDI_TILEDBUF_BASE, baseAddr);

#if defined(TILED_MODE_MAPPING)
    VPU_WriteRegister(REG_ENCODER_BASE+0x0200,0x00001010); // CA[0]  : Y[0]   Y[0]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0204,0x00001111); // CA[1]  : Y[1]   Y[1]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0208,0x00001212); // CA[2]  : Y[2]   Y[2]
    VPU_WriteRegister(REG_ENCODER_BASE+0x020C,0x00001313); // CA[3]  : Y[3]   Y[3]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0210,0x00000303); // CA[4]  : X[3]   X[3]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0214,0x00000404); // CA[5]  : X[4]   X[4]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0218,0x00000505); // CA[6]  : X[5]   X[5]
    VPU_WriteRegister(REG_ENCODER_BASE+0x021C,0x00001415); // CA[7]  : Y[4]   Y[5]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0220,0x00004040); // CA[8]  :        
    VPU_WriteRegister(REG_ENCODER_BASE+0x0224,0x00004040); // CA[9]  :        
    VPU_WriteRegister(REG_ENCODER_BASE+0x0228,0x00004040); // CA[10] :        
    VPU_WriteRegister(REG_ENCODER_BASE+0x022C,0x00004040); // CA[11] :        
    VPU_WriteRegister(REG_ENCODER_BASE+0x0230,0x00004040); // CA[12] :        
    VPU_WriteRegister(REG_ENCODER_BASE+0x0234,0x00004040); // CA[13] :        
    VPU_WriteRegister(REG_ENCODER_BASE+0x0238,0x00004040); // CA[14] :        
    VPU_WriteRegister(REG_ENCODER_BASE+0x023C,0x00004040); // CA[15] :        
    VPU_WriteRegister(REG_ENCODER_BASE+0x0240,0x00000686); // BA[0]  : X[6]   -X[6]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0244,0x00001594); // BA[1]  : Y[5]   -Y[4]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0248,0x00004040); // BA[2]  :        
    VPU_WriteRegister(REG_ENCODER_BASE+0x024C,0x00004040); // BA[3]  :        
    VPU_WriteRegister(REG_ENCODER_BASE+0x0250,0x00000707); // RA[0]  : X[7]   X[7]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0254,0x00000808); // RA[1]  : X[8]   X[8]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0258,0x00000909); // RA[2]  : X[9]   X[9]
    VPU_WriteRegister(REG_ENCODER_BASE+0x025C,0x00000A0A); // RA[3]  : X[10]  X[10]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0260,0x00001616); // RA[4]  : Y[6]   Y[6]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0264,0x00001717); // RA[5]  : Y[7]   Y[7]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0268,0x00001818); // RA[6]  : Y[8]   Y[8]
    VPU_WriteRegister(REG_ENCODER_BASE+0x026C,0x00001919); // RA[7]  : Y[9]   Y[9]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0270,0x00001A1A); // RA[8]  : Y[10]  Y[10]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0274,0x00001B1B); // RA[9]  : Y[11]  Y[11]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0278,0x00001C1C); // RA[10] : Y[12]  Y[12]
    VPU_WriteRegister(REG_ENCODER_BASE+0x027C,0x00001D1D); // RA[11] : Y[13]  Y[13]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0280,0x00001E1E); // RA[12] : Y[14]  Y[14]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0284,0x00001F1F); // RA[13] : Y[15]  Y[15]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0288,0x00004040); // RA[14] : 
    VPU_WriteRegister(REG_ENCODER_BASE+0x028C,0x00004040); // RA[15] : 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0290,0x0003F0F0); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02A0,0x00000C30); // AXI[0]  : 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02A4,0x00000C30); // AXI[1]  : 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02A8,0x00000C30); // AXI[2]  : 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02AC,0x00000000); // AXI[3]  : CA[0]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02B0,0x00000041); // AXI[4]  : CA[1]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02B4,0x00000082); // AXI[5]  : CA[2]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02B8,0x000000C3); // AXI[6]  : CA[3]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02BC,0x00000104); // AXI[7]  : CA[4]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02C0,0x00000145); // AXI[8]  : CA[5]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02C4,0x00000186); // AXI[9]  : CA[6]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02C8,0x000001C7); // AXI[10] : CA[7]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02CC,0x00000410); // AXI[11] : BA[0]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02D0,0x00000451); // AXI[12] : BA[1]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02D4,0x00000820); // AXI[13] : RA[0]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02D8,0x00000861); // AXI[14] : RA[1]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02DC,0x000008A2); // AXI[15] : RA[2]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02E0,0x000008E3); // AXI[16] : RA[3]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02E4,0x00000924); // AXI[17] : RA[4]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02E8,0x00000965); // AXI[18] : RA[5]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02EC,0x000009A6); // AXI[19] : RA[6]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02F0,0x000009E7); // AXI[20] : RA[7]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02F4,0x00000A28); // AXI[21] : RA[8]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02F8,0x00000A69); // AXI[22] : RA[9]
    VPU_WriteRegister(REG_ENCODER_BASE+0x02FC,0x00000AAA); // AXI[23] : RA[10]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0300,0x00000AEB); // AXI[24] : RA[11]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0304,0x00000B2C); // AXI[25] : RA[12]
    VPU_WriteRegister(REG_ENCODER_BASE+0x0308,0x00000B6D); // AXI[26] : RA[13]
    VPU_WriteRegister(REG_ENCODER_BASE+0x030C,0x00000C30); // AXI[27] : 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0310,0x00000C30); // AXI[28] : 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0314,0x00000C30); // AXI[29] : 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0318,0x00000C30); // AXI[30] : 
    VPU_WriteRegister(REG_ENCODER_BASE+0x031C,0x00000C30); // AXI[31] : 
#else
    VPU_WriteRegister(REG_ENCODER_BASE+0x0200,0x00004040);  
    VPU_WriteRegister(REG_ENCODER_BASE+0x0204,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0208,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x020C,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0210,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0214,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0218,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x021C,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0220,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0224,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0228,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x022C,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0230,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0234,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0238,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x023C,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0240,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0244,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0248,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x024C,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0250,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0254,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0258,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x025C,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0260,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0264,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0268,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x026C,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0270,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0274,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0278,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x027C,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0280,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0284,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0288,0x00004040); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x028C,0x00004040);  
    VPU_WriteRegister(REG_ENCODER_BASE+0x0290,0x00000000); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02A0,0x00000C30); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02A4,0x00000C30); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02A8,0x00000C30); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02AC,0x00000000); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02B0,0x00000041); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02B4,0x00000082); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02B8,0x000000C3); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02BC,0x00000104); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02C0,0x00000145); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02C4,0x00000186); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02C8,0x000001C7); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02CC,0x00000208); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02D0,0x00000410); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02D4,0x00000451); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02D8,0x00000820); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02DC,0x00000861); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02E0,0x000008A2); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02E4,0x000008E3); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02E8,0x00000924); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02EC,0x00000965); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02F0,0x000009A6); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02F4,0x000009E7); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02F8,0x00000A28); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x02FC,0x00000A69); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0300,0x00000AAA); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0304,0x00000AEB); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0308,0x00000B2C); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x030C,0x00000B6D); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0310,0x00000BAE); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0314,0x00000BEF); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x0318,0x00000C30); 
    VPU_WriteRegister(REG_ENCODER_BASE+0x031C,0x00000C30); 
#endif 
}

//=============================================================================
/**
 * Encoder engine clock related.
 */
//=============================================================================
void 
VPU_EnableClock(
    void)
{
    HOST_WriteRegisterMask(MMP_AVC_ENCODER_CLOCK_REG_36, 0xFFFF, MMP_AVC_ENCODER_EN_XCLK | MMP_AVC_ENCODER_EN_M7CLK);
    HOST_WriteRegisterMask(MMP_AVC_ENCODER_CLOCK_REG_34, 0xFFFF, MMP_AVC_ENCODER_EN_DIV_XCLK);
}

void 
VPU_DisableClock(
    void)
{
    HOST_WriteRegisterMask(MMP_AVC_ENCODER_CLOCK_REG_36, 0x0000, MMP_AVC_ENCODER_EN_XCLK | MMP_AVC_ENCODER_EN_M7CLK);
    HOST_WriteRegisterMask(MMP_AVC_ENCODER_CLOCK_REG_34, 0x0000, MMP_AVC_ENCODER_EN_DIV_XCLK);
}

void 
VPU_Reset(
    void)
{
	  HOST_WriteRegisterMask(MMP_AVC_ENCODER_CLOCK_REG_36, 0xFFFF, MMP_AVC_ENCODER_ENCODER_RESET);
	  MMP_Sleep(1);
	  HOST_WriteRegisterMask(MMP_AVC_ENCODER_CLOCK_REG_36, 0x0000, MMP_AVC_ENCODER_ENCODER_RESET);
	  MMP_Sleep(1);
}

/*
 * Copyright (c) 2013 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Caster operation API
 *
 * @version 0.1
 */
#include "bus.h"
#include "itx.h"
#include "xcpu_io.h"
#include "xcpu_msgq.h"
#if ITX_BOOT_TYPE == ITX_HOST_BOOT
#include "init9919_300_4M_1.h"
#include "init9919_300_4M_2.h"
#include "init9919_300_4M_3.h"
#include "file.h"
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================
typedef enum _ITX_BOOT_STATE
{
    _ITX_BOOT_NONE,
    _ITX_LOAD_FW_BOOT_OK,
    _ITX_FLAH_BOOT_OK,
} _ITX_BOOT_STATE;

//=============================================================================
//                              Macro Definition
//=============================================================================

//=============================================================================
/**
 * Counts the element number of an array.
 *
 * @param array The array.
 * @return The element number of array.
 */
//=============================================================================
#define COUNT_OF(array) (sizeof (array) / sizeof (array[0]))

#define MEM_SLAVE_WRITE         0x1016             

//=============================================================================
//                              Global Data Definition
//=============================================================================
static _ITX_BOOT_STATE gBootState    = _ITX_BOOT_NONE;

//static pthread_mutex_t    gAPICriticalSection   = 0;

//static MMP_UINT32 gBufferVRamAddress    = 0;
MMP_RESULT gResultBusOpen               = MMP_RESULT_ERROR;

//=============================================================================
//                              External Function Declaration
//=============================================================================

MMP_BOOL
xCpuMsgQ_RouteMessage(
    void);

MMP_UINT32
xCpuMsgQ_GetVersion(
    void);

MMP_UINT32
xCpuMsgQ_GetSubversion(
    void);

MMP_UINT32
xCpuMsgQ_GetCmdBufAddr(
    void);

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static
MMP_RESULT
_itxWaitForReturnMsg(
    SYS_MSGQ_ID  msgQID,
    MMP_UINT msgID,
    XCPU_MSG_OBJ* pStoMmsg);

static
MMP_RESULT
_TestBus();

static
MMP_RESULT
_itxCreateCriticalSection();

static
MMP_RESULT
_itxDeleteCriticalSection();

static
MMP_RESULT
_itxEnterCriticalSection();

static
MMP_RESULT
_itxleaveCriticalSection();

//=============================================================================
//                              Public Function Definition
//=============================================================================
//=============================================================================
/**
 * Boot-up ITX slave device
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
itxBootup(
    ITX_SDEVICE_BOOT_MODE boot_mode)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;
    MMP_UINT16 value = 0;

    if (gBootState ==_ITX_LOAD_FW_BOOT_OK)
        return result;

    //////////////////////////////////////////////////////////////////////////
    // 1.Init Bus
    //////////////////////////////////////////////////////////////////////////
    if (gResultBusOpen != MMP_RESULT_SUCCESS)
#if ITX_BUS_TYPE == ITX_BUS_SPI
        gResultBusOpen = busSPIOpen();
#elif ITX_BUS_TYPE == ITX_BUS_I2C
        gResultBusOpen = busI2COpen();
#endif

#if ITX_BOOT_TYPE == ITX_HOST_BOOT
    {
        if (_TestBus() == MMP_RESULT_ERROR)
            return MMP_RESULT_ERROR;
    }
#endif    
    //result = _itxCreateCriticalSection();
    //if (result) return MMP_RESULT_ERROR_CRITICAL_SECTION;

    //////////////////////////////////////////////////////////////////////////
    // 2. Load FW
    //////////////////////////////////////////////////////////////////////////
#if ITX_BOOT_TYPE == ITX_HOST_BOOT
    if (boot_mode == ITX_SPI_BOOT) 
    {   // 2.1 load firmware from file ///////////////////////////////////////

        if (gBootState == _ITX_BOOT_NONE)
        {
            do
            {
                // initialize hardware setting/////////////////////////
                MMP_UINT16          i           = 0;
                const MMP_UINT16*    pITXs_Reg  = 0;
                MMP_UINT16          totalSize   = 0;
                MMP_UINT32          fileSize    = 0;
                MMP_UINT32          mask        = 0; //0x16b2
   
                totalSize   = COUNT_OF(gJediReg_9919_300_4M_1);
                pITXs_Reg  = gJediReg_9919_300_4M_1;

                for (i = 0; i < totalSize; i += 2)
                {
                    if (pITXs_Reg[i] > 0)
                    {
                        xCpuIO_WriteRegister(pITXs_Reg[i], pITXs_Reg[i + 1]);
                    }
                    else
                    {
                        PalSleep(pITXs_Reg[i + 1]);
                    }
                }
                //##############################################
                //# /* FRange Selection for 440MHz */
                //##############################################
                xCpuIO_WriteRegister(0x16b2, 0x0000);    //clear reg[0x16b2]
                xCpuIO_WriteRegister(0x0342, 0x2a84);    //set FRange=4
                PalSleep(1*1000);//WAIT(7500);

                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 0);//MASK(0x16b2, 0x0001, 0x0001);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 1);//MASK(0x16b2, 0x0002, 0x0002);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 2);//MASK(0x16b2, 0x0004, 0x0004);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 3);//MASK(0x16b2, 0x0008, 0x0008);
                }

                xCpuIO_WriteRegister(0x0342, 0x0a85);    //dll reset
                xCpuIO_WriteRegister(0x0342, 0x2a85);    //set FRange=5
                PalSleep(1*1000);//WAIT(7500);

                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 4);//MASK(0x16b2, 0x0010, 0x0010);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 5);//MASK(0x16b2, 0x0020, 0x0020);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 6);//MASK(0x16b2, 0x0040, 0x0040);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 7);//MASK(0x16b2, 0x0080, 0x0080);
                }
                xCpuIO_WriteRegister(0x0342, 0x0a86);
                xCpuIO_WriteRegister(0x0342, 0x2a86);    //set FRange=6
                PalSleep(1*1000);//WAIT(7500);

                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 8);//MASK(0x16b2, 0x0100, 0x0100);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 9);//MASK(0x16b2, 0x0200, 0x0200);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 10);//MASK(0x16b2, 0x0400, 0x0400);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 11);//MASK(0x16b2, 0x0800, 0x0800);
                }
                xCpuIO_WriteRegister(0x0342, 0x0a87);
                xCpuIO_WriteRegister(0x0342, 0x2a87);    //set FRange=7
                PalSleep(1*1000);//WAIT(7500);

                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 12);//MASK(0x16b2, 0x1000, 0x1000);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 13);//MASK(0x16b2, 0x2000, 0x2000);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 14);//MASK(0x16b2, 0x4000, 0x4000);
                }
                if ( (xCpuIO_ReadRegister(0x0346)&0x000F)==0x0001 )
                {
                    mask = mask|(0x1 << 15);//MASK(0x16b2, 0x8000, 0x8000);
                }

                if ( (mask&0xF0FF)==0xF0FF )
                {
                    xCpuIO_WriteRegister( 0x0342, 0x0a85);
                    xCpuIO_WriteRegister( 0x0342, 0x2a85); //FRange=5
                }
                else
                {
                    if ( (mask&0xF000)==0xF000 )
                    {
                        xCpuIO_WriteRegister( 0x0342, 0x0a87);
                        xCpuIO_WriteRegister( 0x0342, 0x2a87); //FRange=7
                    }
                    else
                    {
                        if ( (mask&0x0F00)==0x0F00 )
                        {
                            xCpuIO_WriteRegister( 0x0342, 0x0a86);
                            xCpuIO_WriteRegister( 0x0342, 0x2a86); //FRange=6
                        }
                        else
                        {
                            if ( (mask&0x00F0)==0x00F0 )
                            {
                                xCpuIO_WriteRegister( 0x0342, 0x0a85);
                                xCpuIO_WriteRegister( 0x0342, 0x2a85); //FRange=5
                            }
                            else
                            {
                                if ( (mask&0x0000F)==0x000F )
                                {
                                    xCpuIO_WriteRegister( 0x0342, 0x0a84);
                                    xCpuIO_WriteRegister( 0x0342, 0x2a84); //FRange=4
                                }
                                else
                                {
                                    xCpuIO_WriteRegister( 0x0342, 0x0a87);
                                    xCpuIO_WriteRegister( 0x0342, 0x2a87);  //default
                                }
                            }
                        }
                    }
                }
                totalSize   = COUNT_OF(gJediReg_9919_300_4M_2);
                pITXs_Reg  = gJediReg_9919_300_4M_2;

                for (i = 0; i < totalSize; i += 2)
                {
                    if (pITXs_Reg[i] > 0)
                    {
                        xCpuIO_WriteRegister(pITXs_Reg[i], pITXs_Reg[i + 1]);
                    }
                    else
                    {
                        PalSleep(pITXs_Reg[i + 1]);
                    }
                }

                PalSleep(25*1000);//wait(200000);
                //##############################################
                //# /* MCLK_PHY Phase selection for 440MHz */
                //##############################################

                xCpuIO_WriteRegister(0x3a6, 0xffff);

                if( (xCpuIO_ReadRegister(0x034a)&0x000F)==0x0000 )
                {
                    xCpuIO_WriteRegister(0x0340, 0x2a44);
                    xCpuIO_WriteRegister(0x3a6, 0x0000);
                }

                if( (xCpuIO_ReadRegister(0x034a)&0x000F)==0x0008 )
                {
                    xCpuIO_WriteRegister(0x0340, 0x2a44);
                    xCpuIO_WriteRegister(0x3a6, 0x0008);
                }

                if( (xCpuIO_ReadRegister(0x034a)&0x000F)==0x0001 )
                {
                    xCpuIO_WriteRegister(0x0340, 0x2a44);
                    xCpuIO_WriteRegister(0x3a6, 0x0001);
                }

                totalSize   = COUNT_OF(gJediReg_9919_300_4M_3);
                pITXs_Reg  = gJediReg_9919_300_4M_3;

                for (i = 0; i < totalSize; i += 2)
                {
                    if (pITXs_Reg[i] > 0)
                    {
                        xCpuIO_WriteRegister(pITXs_Reg[i], pITXs_Reg[i + 1]);
                    }
                    else
                    {
                        PalSleep(pITXs_Reg[i + 1]);
                    }
                }                
                
                // warm up 9917 & check MCLK PHY
                {
                volatile MMP_UINT16 value;
                MMP_UINT32 cnt = 0;                                                                    
                
                // fire ISP
                xCpuIO_WriteRegister(0x0500, 0x0019);                                
                
                PalSleep(100 * 1000);
                
                // stop capture & isp
                xCpuIO_WriteRegister(0x2018, 0x0000);
                xCpuIO_WriteRegister(0x060a, 0x0000);
                
                // wait isp idle
                value = xCpuIO_ReadRegister(0x6fc);
                //HOST_ReadRegister(0x6fc, &value);
                
                while ((value & 0x1) != 0)
                {
                    value = xCpuIO_ReadRegister(0x6fc);
                    //HOST_ReadRegister(0x6fc, &value);                    
                }
                
                // wait capture idle
                value = xCpuIO_ReadRegister(0x1f22);
                //HOST_ReadRegister(0x1f22, &value);
                
                while ((value & 0x80c1) != 0x80c1)
                {
                    value = xCpuIO_ReadRegister(0x1f22);
                    //HOST_ReadRegister(0x1f22, &value);
                }
                
                // read dram status
                value = xCpuIO_ReadRegister(0x34a);
                //HOST_ReadRegister(0x34a, &value);
                
                while ((((value & 0xF) == 0x0) || ((value & 0xF) == 0x1) || ((value & 0xF) == 0x8)) &&
                        cnt++ < 5)
                {                    
                    MMP_UINT16 clockpos =  (xCpuIO_ReadRegister(0x340) & 0x10) ? 0x2a44 : 0x2a54;
                    	
                  	xCpuIO_WriteRegister(0x340, clockpos);
             
                    //for(i=0; i<30000; i++) asm("");
                    printf("Update(%d) 0x340 %x 0x342 = %x\n", cnt, clockpos, value);
                    PalSleep(100 * 1000);
                    value = xCpuIO_ReadRegister(0x34a);
                }
                
                value = xCpuIO_ReadRegister(0x3a6);
                //HOST_ReadRegister(0x3a6, &value);
                printf("Mem Addr (0x3a6) %x\n", value);
                
                value = xCpuIO_ReadRegister(0x340);
                //HOST_ReadRegister(0x340, &value);
                printf("Mem Addr (0x340) %x\n", value);
                
                value = xCpuIO_ReadRegister(0x342);
                //HOST_ReadRegister(0x342, &value);
                printf("Mem Addr (0x342) %x\n", value);
                
                value = xCpuIO_ReadRegister(0x344);
                //HOST_ReadRegister(0x344, &value);
                printf("Mem Addr (0x344) %x\n", value);
                
                value = xCpuIO_ReadRegister(0x346);
                //HOST_ReadRegister(0x346, &value);
                printf("Mem Addr (0x346) %x\n", value);
                
                value = xCpuIO_ReadRegister(0x348);
                //HOST_ReadRegister(0x348, &value);
                printf("Mem Addr (0x348) %x\n", value);
                
                value = xCpuIO_ReadRegister(0x34a);
                //HOST_ReadRegister(0x34a, &value);
                printf("Mem Addr (0x34a) %x\n", value);
                
                value = xCpuIO_ReadRegister(0x34e);
                //HOST_ReadRegister(0x34e, &value);
                printf("Mem Addr (0x34e) %x\n", value);                                
                }
                
                /* Bist test to select case for 320MHz */
                {
                MMP_UINT16 MBistResult = 0; //3b0
                MMP_UINT16 MPosResult = 0;  //3b4
                MMP_BOOL   bCaseSel = MMP_FALSE; //3b8
                	
                //xCpuIO_WriteRegister(0x3b0, 0x0000);
                //xCpuIO_WriteRegister(0x3b4, 0x0000);
                //xCpuIO_WriteRegister(0x3b8, 0x0000);
                //xCpuIO_WriteRegister(0x3a6, 0x0000);
                xCpuIO_WriteRegister(0x3aa, 0x00ff); // ?
                
                // Test Case 2 0x1444 a2
                xCpuIO_WriteRegister(0x344, 0x1444);
                
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0300);
                PalSleep(5 * 1000); //5ms
                
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0001);
                    MBistResult |= 0x0001;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0b10);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0002);
                    MBistResult |= 0x0002;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0500);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0004);
                    MBistResult |= 0x0004;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0310);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0008);
                    MBistResult |= 0x0008;
                }                
                printf("Case 2 0x1444 %x\n", xCpuIO_ReadRegister(0x034a));
                if((xCpuIO_ReadRegister(0x034a)&0xFFF0)==0x38E0) // EE
                {
                    //xCpuIO_WriteRegister(0x03b4, 0x0003);
                    MPosResult |= 0x0003;
                } else {
                    if((xCpuIO_ReadRegister(0x034a)&0xFC00)==0x3800) // EX
                    {
                        //xCpuIO_WriteRegister(0x03b4, 0x0002);
                        MPosResult |= 0x0002;
                    }
                    if((xCpuIO_ReadRegister(0x034a)&0x03F0)==0x00E0) // XE
                    {
                        //xCpuIO_WriteRegister(0x03b4, 0x0002);
                        MPosResult |= 0x0002;
                    }
                }
                
                // Test Case 2 0x1555 a2
                xCpuIO_WriteRegister(0x344, 0x1555);
                
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0300);
                PalSleep(5 * 1000); //5ms
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0010);
                    MBistResult |= 0x0010;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0b10);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0020);
                    MBistResult |= 0x0020;
                    
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0500);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0040);
                    MBistResult |= 0x0040;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0310);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0080);
                    MBistResult |= 0x0080;
                }
                printf("Case 2 0x1555 %x\n", xCpuIO_ReadRegister(0x034a));
                if((xCpuIO_ReadRegister(0x034a)&0xFFF0)==0x38E0) // EE
                {
                    //xCpuIO_WriteRegister(0x03b4, 0x0030);
                    MPosResult |= 0x0030;
                } else {
                    if((xCpuIO_ReadRegister(0x034a)&0xFC00)==0x3800) // EX
                    {
                        //xCpuIO_WriteRegister(0x03b4, 0x0020);
                        MPosResult |= 0x0020;
                    }
                    if((xCpuIO_ReadRegister(0x034a)&0x03F0)==0x00E0) // XE
                    {
                        //xCpuIO_WriteRegister(0x03b4, 0x0020);
                        MPosResult |= 0x0020;
                    }
                }
                
                // Test Case 2 0x1666 a2
                xCpuIO_WriteRegister(0x344, 0x1666);
                
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0300);
                PalSleep(5 * 1000); //5ms
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0100);
                    MBistResult |= 0x0100;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0b10);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0200);
                    MBistResult |= 0x0200;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0500);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0400);
                    MBistResult |= 0x0400;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0310);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x0800);
                    MBistResult |= 0x0800;
                }
                printf("Case 2 0x1666 %x\n", xCpuIO_ReadRegister(0x034a));
                if((xCpuIO_ReadRegister(0x034a)&0xFFF0)==0x38E0) // EE
                {
                    //xCpuIO_WriteRegister(0x03b4, 0x0300);
                    MPosResult |= 0x0300;
                } else {
                    if((xCpuIO_ReadRegister(0x034a)&0xFC00)==0x3800) // EX
                    {
                        //xCpuIO_WriteRegister(0x03b4, 0x0200);
                        MPosResult |= 0x0200;
                    }
                    if((xCpuIO_ReadRegister(0x034a)&0x03F0)==0x00E0) // XE
                    {
                        //xCpuIO_WriteRegister(0x03b4, 0x0200);
                        MPosResult |= 0x0200;
                    }
                }
                
                // Test Case 2 0x1777 a2
                xCpuIO_WriteRegister(0x344, 0x1777);
                
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0300);
                PalSleep(5 * 1000); //5ms
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x1000);
                    MBistResult |= 0x1000;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0b10);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x2000);
                    MBistResult |= 0x2000;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0500);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x4000);
                    MBistResult |= 0x4000;
                }
                xCpuIO_WriteRegister(0x3a8, 0x0000);
                xCpuIO_WriteRegister(0x3a8, 0x0310);
                PalSleep(5 * 1000);
                if((xCpuIO_ReadRegister(0x03a8)&0xF000)==0x3000)
                {
                    //xCpuIO_WriteRegister(0x03b0, 0x8000);
                    MBistResult |= 0x8000;
                }
                printf("Case 2 0x1777 %x\n", xCpuIO_ReadRegister(0x034a));
                if((xCpuIO_ReadRegister(0x034a)&0xFFF0)==0x38E0) // EE
                {
                    //xCpuIO_WriteRegister(0x03b4, 0x3000);
                    MPosResult |= 0x3000;
                } else {
                    if((xCpuIO_ReadRegister(0x034a)&0xFC00)==0x3800) // EX
                    {
                        //xCpuIO_WriteRegister(0x03b4, 0x2000);
                        MPosResult |= 0x2000;
                    }
                    if((xCpuIO_ReadRegister(0x034a)&0x03F0)==0x00E0) // XE
                    {
                        //xCpuIO_WriteRegister(0x03b4, 0x2000);
                        MPosResult |= 0x2000;
                    }
                }                
                                
                // case 2 0x1444, EE
                if(!bCaseSel)
                {
                   if((MBistResult&0x000F)==0x0000)
                   {
                      if((MPosResult&0x0003)==0x0003)
                      {
                         xCpuIO_WriteRegister(0x344, 0x1444);
                         xCpuIO_WriteRegister(0x348, 0x00a2);
                         //xCpuIO_WriteRegister(0x3b8, 0xFFFF);
                         bCaseSel = MMP_TRUE;
                      }
                   }
                }
                // case 2 0x1555, EE
                if(!bCaseSel)
                {
                   if((MBistResult&0x00F0)==0x0000)
                   {
                      if((MPosResult&0x0030)==0x0030)
                      {
                         xCpuIO_WriteRegister(0x344, 0x1555);
                         xCpuIO_WriteRegister(0x348, 0x00a2);
                         //xCpuIO_WriteRegister(0x3b8, 0xFFFF);
                         bCaseSel = MMP_TRUE;
                      }
                   }
                }
                // case 2 0x1666, EE
                if(!bCaseSel)
                {
                   if((MBistResult&0x0F00)==0x0000)
                   {
                      if((MPosResult&0x0300)==0x0300)
                      {
                         xCpuIO_WriteRegister(0x344, 0x1666);
                         xCpuIO_WriteRegister(0x348, 0x00a2);
                         //xCpuIO_WriteRegister(0x3b8, 0xFFFF);
                         bCaseSel = MMP_TRUE;
                      }
                   }
                }
                // case 2 0x1777, EE
                if(!bCaseSel)
                {
                   if((MBistResult&0xF000)==0x0000)
                   {
                      if((MPosResult&0x3000)==0x3000)
                      {
                         xCpuIO_WriteRegister(0x344, 0x1777);
                         xCpuIO_WriteRegister(0x348, 0x00a2);
                         //xCpuIO_WriteRegister(0x3b8, 0xFFFF);
                         bCaseSel = MMP_TRUE;
                      }
                   }
                }
                // case 2 0x1444, EX, XE
                if(!bCaseSel)
                {
                   if((MBistResult&0x000F)==0x0000)
                   {
                      if((MPosResult&0x0003)==0x0002)
                      {
                         xCpuIO_WriteRegister(0x344, 0x1444);
                         xCpuIO_WriteRegister(0x348, 0x00a2);
                         //xCpuIO_WriteRegister(0x3b8, 0xFFFF);
                         bCaseSel = MMP_TRUE;
                      }
                   }
                }
                // case 2 0x1555, EX, XE
                if(!bCaseSel)
                {
                   if((MBistResult&0x00F0)==0x0000)
                   {
                      if((MPosResult&0x0030)==0x0020)
                      {
                         xCpuIO_WriteRegister(0x344, 0x1555);
                         xCpuIO_WriteRegister(0x348, 0x00a2);
                         //xCpuIO_WriteRegister(0x3b8, 0xFFFF);
                         bCaseSel = MMP_TRUE;
                      }
                   }
                }
                // case 2 0x1666, EX, XE
                if(!bCaseSel)
                {
                   if((MBistResult&0x0F00)==0x0000)
                   {
                      if((MPosResult&0x0300)==0x0200)
                      {
                         xCpuIO_WriteRegister(0x344, 0x1666);
                         xCpuIO_WriteRegister(0x348, 0x00a2);
                         //xCpuIO_WriteRegister(0x3b8, 0xFFFF);
                         bCaseSel = MMP_TRUE;
                      }
                   }
                }
                // case 2 0x1777, EX, XE
                if(!bCaseSel)
                {
                   if((MBistResult&0xF000)==0x0000)
                   {
                      if((MPosResult&0x3000)==0x2000)
                      {
                         xCpuIO_WriteRegister(0x344, 0x1777);
                         xCpuIO_WriteRegister(0x348, 0x00a2);
                         //xCpuIO_WriteRegister(0x3b8, 0xFFFF);
                         bCaseSel = MMP_TRUE;
                      }
                   }
                }
                
                if(!bCaseSel)
                {
                   xCpuIO_WriteRegister(0x344, 0x1111);
                   xCpuIO_WriteRegister(0x348, 0x0092);
                }

                printf("DRAM window %4x %4x %4x %4x\n", xCpuIO_ReadRegister(0x344), xCpuIO_ReadRegister(0x348), MBistResult, MPosResult);
                }                                
                
                // load firmware/////////////////////////////////////////////////////////
                result = UserFileSize(&fileSize);
                if (result == MMP_RESULT_SUCCESS)
                {
                    if (UserFileLoad(0x0, fileSize) == MMP_RESULT_ERROR)
                    {
                    	printf("UserFileLoad fail\n");
                        result = MMP_RESULT_ERROR;
                        break;
                    }
                }
                else
                    break;                                
                    
                // enable CPU
                xCpuIO_WriteRegister(0x168C, 0x1001);

            } while(0);
        }
    }
    else
    {  //2.2 boot from NorFlash ///////////////////////////////////////////
        if (gBootState == _ITX_BOOT_NONE)
            result = MMP_RESULT_SUCCESS;
    }
#else    
    {  //2.2 boot from NorFlash ///////////////////////////////////////////
        if (gBootState == _ITX_BOOT_NONE)
            result = MMP_RESULT_SUCCESS;
    }
#endif    

    //////////////////////////////////////////////////////////////////////////
    //3. Init XCpu
    //////////////////////////////////////////////////////////////////////////

    if (result == MMP_RESULT_SUCCESS)
    {
        MMP_INT timeout = 10000;
        //sysMsgQ_Init(SYS_MSGQ_ID_FILE);
        sysMsgQ_Init(SYS_MSGQ_ID_CMD);

        result = xCpuMsgQ_Init();
        if (result != MMP_RESULT_SUCCESS)
            return result;

        //// Wait for ITX slave device BootOK//////////////////////////////////////////////
        if (xCpuMsgQ_GetVersion() != 0x0)
        {
        //    // If ITX slave device boots up successfully, it will update the
        //    // subversion number to a non-zero value. Otherwise, the
        //    // subversion number is zero.
            while (xCpuMsgQ_GetSubversion() == 0 && (--timeout > 0))
                PalSleep(1 * 1000);

            if (timeout <= 0)
                result = MMP_RESULT_TIMEOUT;
        }
		else
            result = MMP_RESULT_ERROR;  
    }

    //////////////////////////////////////////////////////////////////////////
    //4. Set BootState
    //////////////////////////////////////////////////////////////////////////
    if (result == MMP_RESULT_SUCCESS)
    {
        gBootState = (ITX_BOOT_TYPE == ITX_HOST_BOOT)
            ? _ITX_LOAD_FW_BOOT_OK
            : _ITX_FLAH_BOOT_OK;       
    }
    else
    {
        //_itxDeleteCriticalSection();
        //gAPICriticalSection = 0;
    }

    return result;
}

//=============================================================================
/**
 * Shutdown ITX slave device
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
itxShutdown(
    void)
{
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    if (gBootState != _ITX_BOOT_NONE)
    {
        // +++
        XCPU_MSG_OBJ tMtoSMsg = {0};
        XCPU_MSG_OBJ tStoMMsg = {0};
        tMtoSMsg.type   = SYS_MSG_TYPE_CMD;
        tMtoSMsg.id     = SYS_MSG_ID_SHUTDOWN;
        xCpuMsgQ_SendMessage(&tMtoSMsg);
        if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_SHUTDOWN_RET,&tStoMMsg) !=MMP_RESULT_SUCCESS)
            result = MMP_RESULT_ERROR;

        xCpuMsgQ_Terminate();
        sysMsgQ_Terminate(SYS_MSGQ_ID_CMD);
        //sysMsgQ_Terminate(SYS_MSGQ_ID_FILE);

#if ITX_BUS_TYPE == ITX_BUS_SPI
        busSPIClose();
#elif ITX_BUS_TYPE == ITX_BUS_I2C
        busI2CClose();
#endif

        gResultBusOpen=MMP_RESULT_ERROR;
        gBootState = _ITX_BOOT_NONE;
        //_itxDeleteCriticalSection();
        //gAPICriticalSection=0;
    }

    return result;
}

MMP_RESULT
itxTestBus(
    void)
{
    MMP_RESULT result = MMP_RESULT_ERROR;
    MMP_UINT16 value = 0x1234;

    // test register
    if (_TestBus() == MMP_RESULT_ERROR)
        return MMP_RESULT_ERROR;

    // test memory
    xCpuIO_WriteMemoryUInt16(
        (MMP_UINT32)MEM_SLAVE_WRITE,
        (MMP_UINT32)&value,
        2);

    value = 0;

    xCpuIO_ReadMemoryUInt16(
        (MMP_UINT32)&value,
        (MMP_UINT32)MEM_SLAVE_WRITE,
        2);

    if (value == 0x1234)
        return MMP_RESULT_SUCCESS;

    return result;
}

MMP_RESULT
itxEncoderParaUpdate(
    void)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_INIT_ENCODER;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_INIT_ENCODER_RET, &tStoMMsg) != MMP_RESULT_SUCCESS)
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxEncStart(
    void)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_START_ENCODER;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_START_ENCODER_RET, &tStoMMsg) != MMP_RESULT_SUCCESS)
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxEncSetStream(
    uint8_t streamId)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_ENCODER_SET_STREAM;
    tMtoSMsg.msg[0] = streamId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_ENCODER_SET_STREAM_RET, &tStoMMsg) != MMP_RESULT_SUCCESS)
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxEncStop(
    void)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_STOP_ENCODER;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_STOP_ENCODER_RET, &tStoMMsg) != MMP_RESULT_SUCCESS)
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxJPGRecord(
    uint32_t quality)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_JPEG_RECORD;
    tMtoSMsg.msg[0] = quality;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_JPEG_RECORD_RET, &tStoMMsg) != MMP_RESULT_SUCCESS)
        result = MMP_RESULT_ERROR;        
    //_itxleaveCriticalSection();
        
    return result;
}

MMP_RESULT
itxJPGStop(
    void)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_JPEG_STOP;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_JPEG_STOP_RET, &tStoMMsg) != MMP_RESULT_SUCCESS)
        result = MMP_RESULT_ERROR;        
    //_itxleaveCriticalSection();
        
    return result;
}

MMP_RESULT
itxCamPWon(
    uint8_t streamId)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GPIO_SET_CAM_PW_ON;  
    tMtoSMsg.msg[0] = streamId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_GPIO_SET_CAM_PW_ON_RET, &tStoMMsg) != MMP_RESULT_SUCCESS)
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxCamStandBy(
    void)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GPIO_SET_CAM_STANDBY;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_GPIO_SET_CAM_STANDBY_RET, &tStoMMsg) != MMP_RESULT_SUCCESS)
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxCamLedon(
    void)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GPIO_SET_CAM_LED_ON;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_GPIO_SET_CAM_LED_ON_RET, &tStoMMsg) != MMP_RESULT_SUCCESS)
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxCamLedoff(
    void)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GPIO_SET_CAM_LED_OFF;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_GPIO_SET_CAM_LED_OFF_RET, &tStoMMsg) != MMP_RESULT_SUCCESS)
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetSensorFlickMode(
    SENSOR_CTRL_FLICK_MODE_TYPE flickMode)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_SENSOR_ANTIFLICKER;
    tMtoSMsg.msg[0] = flickMode;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_SENSOR_ANTIFLICKER_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetSensorImageMirror(
    SENSOR_IMAGE_MIRROR pCtrl)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_SENSOR_MIRROR;
    tMtoSMsg.msg[0] = pCtrl.enHorMirror; 
    tMtoSMsg.msg[1] = pCtrl.enVerMirror;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_SENSOR_MIRROR_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetSensorImageEffect(
    SENSOR_IMAGE_EFFECT pCtrl)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_SENSOR_EFFECT;
    tMtoSMsg.msg[0] = pCtrl.brightness; 
    tMtoSMsg.msg[1] = pCtrl.contrast;
    tMtoSMsg.msg[2] = pCtrl.saturation;
    tMtoSMsg.msg[3] = pCtrl.edgeEnhancement;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_SENSOR_EFFECT_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}


MMP_RESULT
itxGetInputDevice(
    ITX_INPUT_DEVICE* pDevice)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_INPUT_SOURCE_DEVICE;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_INPUT_SOURCE_DEVICE_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        *pDevice = tStoMMsg.msg[0];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetInputDevice(
    ITX_INPUT_DEVICE  pdevice)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_INPUT_SOURCE_DEVICE;
    tMtoSMsg.msg[0] = pdevice;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_INPUT_SOURCE_DEVICE_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxGetInputVideoInfo(
    ITX_INPUT_VIDEO_INFO* pInfo)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_INPUT_VIDEO_INFO;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_INPUT_VIDEO_INFO_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        *pInfo = tStoMMsg.msg[0];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxGetOutputEncRes(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16*          enc_width,
    MMP_UINT16*          enc_height,
    MMP_UINT16*          enc_deinterlace)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_OUTPUT_ENCODER_RESOLUTION;
    tMtoSMsg.msg[0] = device;
    tMtoSMsg.msg[1] = queryId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_OUTPUT_ENCODER_RESOLUTION_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        *enc_width = tStoMMsg.msg[0];
        *enc_height = tStoMMsg.msg[1];
        *enc_deinterlace = tStoMMsg.msg[2];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetOutputEncRes(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16           enc_width,
    MMP_UINT16           enc_height,
    MMP_UINT16           streamId)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_OUTPUT_ENCODER_RESOLUTION;
    tMtoSMsg.msg[0] = device;
    tMtoSMsg.msg[1] = queryId;
    tMtoSMsg.msg[2] = enc_width;
    tMtoSMsg.msg[3] = enc_height;
    tMtoSMsg.msg[4] = streamId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_OUTPUT_ENCODER_RESOLUTION_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxGetOutputFrameRate(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16*          pFrmRate)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_OUTPUT_ENCODER_FRAME_RATE;
    tMtoSMsg.msg[0] = device;
    tMtoSMsg.msg[1] = queryId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_OUTPUT_ENCODER_FRAME_RATE_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        *pFrmRate = tStoMMsg.msg[0];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetOutputFrameRate(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO setId,
    MMP_UINT16           frameRate)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_OUTPUT_ENCODER_FRAME_RATE;
    tMtoSMsg.msg[0] = device;
    tMtoSMsg.msg[1] = setId;
    tMtoSMsg.msg[2] = frameRate;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_OUTPUT_ENCODER_FRAME_RATE_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxGetMaxEnFrameRate(
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16           enWidth,
    MMP_UINT16           enHeight,
    MMP_UINT16*          pFrmRate)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_MAX_OUTPUT_ENCODER_FRAME_RATE;
    tMtoSMsg.msg[0] = queryId;
    tMtoSMsg.msg[1] = enWidth;
    tMtoSMsg.msg[2] = enHeight;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_MAX_OUTPUT_ENCODER_FRAME_RATE_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        *pFrmRate = tStoMMsg.msg[0];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxGetOutputBitRate(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16*          bitrate)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_OUTPUT_ENCODER_BIT_RATE;
    tMtoSMsg.msg[0] = device;
    tMtoSMsg.msg[1] = queryId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_OUTPUT_ENCODER_BIT_RATE_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        *bitrate = tStoMMsg.msg[0];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetOutputBitRate(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO setId,
    MMP_UINT16           bitrate,
    MMP_UINT16           streamId)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_OUTPUT_ENCODER_BIT_RATE;
    tMtoSMsg.msg[0] = device;
    tMtoSMsg.msg[1] = setId;
    tMtoSMsg.msg[2] = bitrate;
    tMtoSMsg.msg[3] = streamId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_OUTPUT_ENCODER_BIT_RATE_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxGetIFramePeriod(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO queryId,
    MMP_UINT16*          Ifmperiod)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_OUTPUT_ENCODER_IFRAME_PERIOD;
    tMtoSMsg.msg[0] = device;
    tMtoSMsg.msg[1] = queryId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_OUTPUT_ENCODER_IFRAME_PERIOD_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        *Ifmperiod = tStoMMsg.msg[0];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetIFramePeriod(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO setId,
    MMP_UINT16           Ifmperiod)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_OUTPUT_ENCODER_IFRAME_PERIOD;
    tMtoSMsg.msg[0] = device;
    tMtoSMsg.msg[1] = setId;
    tMtoSMsg.msg[2] = Ifmperiod;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_OUTPUT_ENCODER_IFRAME_PERIOD_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetDeinterlaceOn(
    ITX_INPUT_DEVICE     device,
    ITX_INPUT_VIDEO_INFO setId,
    MMP_BOOL             bInterlaceOn)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_DEINTERLACE;
    tMtoSMsg.msg[0] = device;
    tMtoSMsg.msg[1] = setId;
    tMtoSMsg.msg[2] = bInterlaceOn;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_DEINTERLACE_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetOutputAudType(
    ITX_OUTPUT_AUD_TYPE   btype)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_AUDIO_CODEC_TYPE;
    tMtoSMsg.msg[0] = btype;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_AUDIO_CODEC_TYPE_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxGetOutputAudBitRate(
    MMP_UINT16*   bitrate)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_AUDIO_BIT_RATE;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_AUDIO_BIT_RATE_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        *bitrate = tStoMMsg.msg[0];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetOutputAudBitRate(
    MMP_UINT16   bitrate)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_AUDIO_BIT_RATE;
    tMtoSMsg.msg[0] = bitrate;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_AUDIO_BIT_RATE_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxGetTime(
    ITX_SYSTEM_TIME* time)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_TIME;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_TIME_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        time->year  = tStoMMsg.msg[0];
        time->month = tStoMMsg.msg[1];
        time->day   = tStoMMsg.msg[2];
        time->hour  = tStoMMsg.msg[3];
        time->min   = tStoMMsg.msg[4];
        time->sec   = tStoMMsg.msg[5];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxSetTime(
    ITX_SYSTEM_TIME time)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_TIME;
    tMtoSMsg.msg[0] = time.year;
    tMtoSMsg.msg[1] = time.month;
    tMtoSMsg.msg[2] = time.day;
    tMtoSMsg.msg[3] = time.hour;
    tMtoSMsg.msg[4] = time.min;
    tMtoSMsg.msg[5] = time.sec;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_TIME_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxGetVersion(
    ITX_VERSION_TYPE* version)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_FW_VERSION;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_FW_VERSION_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        version->customerCode = tStoMMsg.msg[0];
        version->projectCode = tStoMMsg.msg[1];
        version->sdkMajorVersion = tStoMMsg.msg[2];
        version->sdkMinorVersion = tStoMMsg.msg[3];
        version->buildNumber = tStoMMsg.msg[4];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxGetState(
    ITX_STATE* pState)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_STATE;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_STATE_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        if (tStoMMsg.msg[0])
            *pState = ITX_STATE_ENCODING;
        else
            *pState = ITX_STATE_STOP_ENCODING;
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxSendReturnChannel(
    MMP_UINT16 size)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_RETURN_CHANNEL_PARA;
    tMtoSMsg.msg[0] = size;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_RETURN_CHANNEL_PARA_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT
itxGetModulatorParameter(
    ITX_MODULATOR_PARA* pModulatorPara)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_MODULATOR_PARA;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_MODULATOR_PARA_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        pModulatorPara->frequency = (MMP_UINT32) ((tStoMMsg.msg[0] << 16) | tStoMMsg.msg[1]);
        pModulatorPara->bandwidth = tStoMMsg.msg[2];
        pModulatorPara->constellation = tStoMMsg.msg[3];
        pModulatorPara->codeRate = tStoMMsg.msg[4];
        pModulatorPara->guardInterval = tStoMMsg.msg[5];
        pModulatorPara->transmissionMode = tStoMMsg.msg[6];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxSetModulatorParameter(
    ITX_MODULATOR_PARA modulatorPara)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_SET_MODULATOR_PARA;
    tMtoSMsg.msg[0] = (MMP_UINT16) ((modulatorPara.frequency >> 16) & 0xFFFF);
    tMtoSMsg.msg[1] = (MMP_UINT16) (modulatorPara.frequency & 0xFFFF);
    tMtoSMsg.msg[2] = modulatorPara.bandwidth;
    tMtoSMsg.msg[3] = modulatorPara.constellation;
    tMtoSMsg.msg[4] = modulatorPara.codeRate;
    tMtoSMsg.msg[5] = modulatorPara.guardInterval;
    tMtoSMsg.msg[6] = modulatorPara.transmissionMode;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_SET_MODULATOR_PARA_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateTransportStreamId(
    MMP_UINT16 transportStreamId)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_TS_ID;
    tMtoSMsg.msg[0] = transportStreamId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_TS_ID_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateNetworkName(
    ITX_NETWORK_NAME_PARA* pNetworkName)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_UINT16      i = 0;
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_NETWORK_NAME;
    
    if ((((pNetworkName->nameLen + 1) / 2) + 1) > MAX_SYS_MSG_PARAM_COUNT)
    {
        return MMP_RESULT_ERROR;
    }
    
    tMtoSMsg.msg[0] = pNetworkName->nameLen;
    for (i = 0; i < ((pNetworkName->nameLen + 1) / 2); i++)
    {
        tMtoSMsg.msg[i + 1] = ((pNetworkName->networkName[i * 2] << 8) & 0xFF00);
        tMtoSMsg.msg[i + 1] |= pNetworkName->networkName[i * 2 + 1];
    }
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_NETWORK_NAME_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateNetworkId(
    MMP_UINT16 networkId)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_NETWORK_ID;
    tMtoSMsg.msg[0] = networkId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_NETWORK_ID_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateOriginalNetworkId(
    MMP_UINT16 originalNetworkId)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_ORIGINAL_NETWORK_ID;
    tMtoSMsg.msg[0] = originalNetworkId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_ORIGINAL_NETWORK_ID_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateServiceListDescriptor(
    ITX_SERVICE_LIST_PARA* pServiceList)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_SERVICE_LIST_DESCRIPTOR;
    tMtoSMsg.msg[0] = pServiceList->serviceId;
    tMtoSMsg.msg[1] = pServiceList->serviceType;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_SERVICE_LIST_DESCRIPTOR_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateCountryId(
    ITX_COUNTRY_ID_PARA countryId)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_COUNTRY_ID;
    tMtoSMsg.msg[0] = countryId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_COUNTRY_ID_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateLCN(
    ITX_SERVICE_LCN_PARA* pServiceLCN)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_LCN;
    tMtoSMsg.msg[0] = pServiceLCN->serviceId;
    tMtoSMsg.msg[1] = pServiceLCN->lcn;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_LCN_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateServiceName(
    ITX_SERVICE_NAME_PARA* pServiceName)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_UINT16      i = 0;
    MMP_UINT32      totalNameLen = 0;
    MMP_UINT32      offset = 3;
    MMP_RESULT result = MMP_RESULT_SUCCESS;
    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_SERVICE_NAME;

    if ((((pServiceName->serviceNameLen + 1) / 2) + ((pServiceName->serviceProviderNameLen + 1) / 2))  + 3 > MAX_SYS_MSG_PARAM_COUNT)
    {
        return MMP_RESULT_ERROR;;
    }

    tMtoSMsg.msg[0] = pServiceName->serviceId;
    tMtoSMsg.msg[1] = pServiceName->serviceNameLen;
    tMtoSMsg.msg[2] = pServiceName->serviceProviderNameLen;;

    for (i = 0; i < (pServiceName->serviceNameLen + 1) / 2; i++)
    {
        tMtoSMsg.msg[i+offset] = ((pServiceName->serviceName[i * 2] << 8) & 0xFF00);
        tMtoSMsg.msg[i+offset] |= pServiceName->serviceName[i * 2 + 1];
    }
    offset += ((pServiceName->serviceNameLen + 1) / 2);
    for (i = 0; i < (pServiceName->serviceProviderNameLen + 1) /2; i++)
    {
        tMtoSMsg.msg[i+offset] = ((pServiceName->serviceProviderName[i * 2] << 8) & 0xFF00);
        tMtoSMsg.msg[i+offset] |= pServiceName->serviceProviderName[i * 2 + 1];
    }   
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_SERVICE_NAME_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsGetServiceCount(
    MMP_UINT16* pServcieCount)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_GET_SERVICE_COUNT;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    if (_itxWaitForReturnMsg(SYS_MSGQ_ID_CMD,SYS_MSG_ID_GET_SERVICE_COUNT_RET,&tStoMMsg)==MMP_RESULT_SUCCESS)
    {
        *pServcieCount = tStoMMsg.msg[0];
    }
    else
        result = MMP_RESULT_ERROR;
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateServiceId(
    ITX_SERVICE_ID_PARA* pServiceId)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_SERVICE_ID;
    tMtoSMsg.msg[0] = pServiceId->serviceIndex;
    tMtoSMsg.msg[1] = pServiceId->serviceId;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_SERVICE_ID_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateServicePmtPid(
    ITX_SERVICE_PMT_PID_PARA* pServicePmtPid)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_SERVICE_PMT_PID;
    tMtoSMsg.msg[0] = pServicePmtPid->serviceId;
    tMtoSMsg.msg[1] = pServicePmtPid->pmtPid;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_SERVICE_PMT_PID_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

MMP_RESULT 
itxTsUpdateServiceEsPid(
    ITX_SERVICE_ES_INFO_PARA* pServiceEsInfo)
{
    XCPU_MSG_OBJ    tMtoSMsg = {0};
    XCPU_MSG_OBJ    tStoMMsg = {0};
    MMP_RESULT result = MMP_RESULT_SUCCESS;

    //_itxEnterCriticalSection();
    tMtoSMsg.type = SYS_MSG_TYPE_CMD;
    tMtoSMsg.id = SYS_MSG_ID_UPDATE_SERVICE_ES_PID;
    tMtoSMsg.msg[0] = pServiceEsInfo->serviceId;
    tMtoSMsg.msg[1] = pServiceEsInfo->videoPid;
    tMtoSMsg.msg[2] = pServiceEsInfo->audioPid;
    xCpuMsgQ_SendMessage(&tMtoSMsg);
    _itxWaitForReturnMsg(SYS_MSGQ_ID_CMD, SYS_MSG_ID_UPDATE_SERVICE_ES_PID_RET, &tStoMMsg);
    //_itxleaveCriticalSection();

    return result;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

//=============================================================================
/**
 * Wait for  the return message of CMD sent by Host
 *
 * @return MMP_RESULT_SUCCESS is OK. Return MMP_RESULT_TIMEOUT is timeout.
 */
//=============================================================================
static
MMP_RESULT
_itxWaitForReturnMsg(
    SYS_MSGQ_ID  msgQID,
    MMP_UINT msgID,
    XCPU_MSG_OBJ* pStoMmsg)
{
    MMP_RESULT mmp_Result=MMP_RESULT_SUCCESS;
    MMP_INT        timeout = 2000;
    do
    {
        // ToDo: timeout mechanism
        PalSleep(1 * 1000);
        if (xCpuMsgQ_RouteMessage())
        {
        if (QUEUE_NO_ERROR
            == sysMsgQ_CheckMessage(msgQID, pStoMmsg))
        {
            if (msgID == pStoMmsg->id)
            {
                break;
                }
            }
        }
    } while(--timeout > 0);

    if (timeout > 0 )
        mmp_Result =(MMP_RESULT)sysMsgQ_ReceiveMessage(msgQID,pStoMmsg);
    else
        mmp_Result=MMP_RESULT_ERROR;//MMP_RESULT_TIMEOUT
    return mmp_Result;
}

static
MMP_RESULT
_TestBus()
{
    MMP_RESULT result   = MMP_RESULT_ERROR;

    if (xCpuIO_ReadRegister(0x02) == 0x9910)
        result= MMP_RESULT_SUCCESS;
    else
        printf("no ID got: 0x%x!\n", xCpuIO_ReadRegister(0x02));
  
#if ITX_BOOT_TYPE == ITX_FLASH_BOOT
    // BOOT_CFG = Booting from NOR
    if ((xCpuIO_ReadRegister(0x0) & 0x3) == 0x01)
        result= MMP_RESULT_SUCCESS;
    else
        printf("Not Booting from NOR!\n");
#endif
    printf("boot_cfg 0x%x\n", xCpuIO_ReadRegister(0x0));
    result= MMP_RESULT_SUCCESS;

    return result;
}

static
MMP_RESULT
_itxCreateCriticalSection()
{
   /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     */
   //   if (gAPICriticalSection)
   //		return MMP_RESULT_SUCCESS;
   //
   //    pthread_mutex_init(&gAPICriticalSection, NULL);

   //if (0 == gAPICriticalSection)
   //		return MMP_RESULT_ERROR_CRITICAL_SECTION;

    return MMP_RESULT_SUCCESS;
}

static
MMP_RESULT
_itxDeleteCriticalSection()
{
    return MMP_RESULT_SUCCESS;
}

static
MMP_RESULT
_itxEnterCriticalSection()
{
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     */
   //  if (0 == gAPICriticalSection)
 	//{
 	//	printf((const char*)"%s(%d), The gAPICriticalSection is not created yet\n",
 	//		   (const char*) __FILE__,
 	//		   (const int) __LINE__);
 	//	return MMP_RESULT_ERROR_CRITICAL_SECTION;
 	//}

  //  pthread_mutex_lock(&gAPICriticalSection);

    return MMP_RESULT_SUCCESS;
}

static
MMP_RESULT
_itxleaveCriticalSection()
{
    /*
     *  ToDo:  Add code here
     *
     *  //Pseudo code
     */
   //  if (0 == gAPICriticalSection)
 	//{
 	//	printf((const char*)"%s(%d), The gAPICriticalSection is not created yet\n",
 	//		   (const char*) __FILE__,
 	//		   (const int) __LINE__);
 	//	return MMP_RESULT_ERROR_CRITICAL_SECTION;
 	//}

  //  pthread_mutex_unlock(&gAPICriticalSection);

    return MMP_RESULT_SUCCESS;
}



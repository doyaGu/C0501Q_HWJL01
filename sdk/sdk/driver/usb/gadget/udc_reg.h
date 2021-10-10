/** @file
 * usb device controller hardware configure header file.
 *
 * @author Irene Lin
 * @version 
 * @date
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
#ifndef UDC_REG_H
#define UDC_REG_H

#ifdef __cplusplus
extern "C" {
#endif


#if defined(CFG_USB_DEVICE_USB0)
    #define UDC_BASE	ITH_USB0_BASE
#elif defined(CFG_USB_DEVICE_USB1)
    #define UDC_BASE	ITH_USB1_BASE
#else
    #error " Not choose usb device hardware module!! "
#endif

#define BIT0	                 0x00000001
#define BIT1	                 0x00000002
#define BIT2	                 0x00000004
#define BIT3	                 0x00000008
#define BIT4	                 0x00000010
#define BIT5	                 0x00000020
#define BIT6	                 0x00000040
#define BIT7	                 0x00000080
#define BIT8	                 0x00000100
#define BIT9	                 0x00000200
#define BIT10	                 0x00000400
#define BIT11	                 0x00000800
#define BIT12	                 0x00001000
#define BIT13	                 0x00002000
#define BIT14	                 0x00004000
#define BIT15	                 0x00008000	
#define BIT16	                 0x00010000
#define BIT17	                 0x00020000
#define BIT18	                 0x00040000
#define BIT19	                 0x00080000
#define BIT20	                 0x00100000
#define BIT21	                 0x00200000
#define BIT22	                 0x00400000
#define BIT23	                 0x00800000	
#define BIT24	                 0x01000000
#define BIT25	                 0x02000000
#define BIT26	                 0x04000000
#define BIT27	                 0x08000000
#define BIT28	                 0x10000000
#define BIT29	                 0x20000000
#define BIT30	                 0x40000000
#define BIT31	                 0x80000000


#define FIFOCX			0xFF
#define DMA_CHANEL_FREE 0xFF

#define MAX_EP_NUM		8
#define MAX_FIFO_NUM	4

#define udc_reg(offset)			(UDC_BASE|(offset))

/*-------------------------------------------------------------------------*/
// OTG control status register (0x80)
#define mOtg_Ctrl_A_BUS_REQ_Rd()		(ithReadRegA(udc_reg(0x80)) & BIT4)
#define mOtg_Ctrl_A_BUS_REQ_Set()		ithWriteRegMaskA(udc_reg(0x80), BIT4, BIT4)
#define mOtg_Ctrl_A_BUS_REQ_Clr()		ithWriteRegMaskA(udc_reg(0x80), 0x0, BIT4)

#define mOtg_Ctrl_A_BUS_DROP_Rd()		(ithReadRegA(udc_reg(0x80)) & BIT5)
#define mOtg_Ctrl_A_BUS_DROP_Set()		ithWriteRegMaskA(udc_reg(0x80), BIT5, BIT5)
#define mOtg_Ctrl_A_BUS_DROP_Clr()		ithWriteRegMaskA(udc_reg(0x80), 0x0, BIT5)

#define mOtg_Ctrl_A_VBUS_VLD_Rd()		(ithReadRegA(udc_reg(0x80)) & BIT19)

#define mOtg_Ctrl_CROLE_Rd()			(ithReadRegA(udc_reg(0x80)) & BIT20)
#define mOtg_Ctrl_ID_Rd()				(ithReadRegA(udc_reg(0x80)) & BIT21)
#define mOtg_Ctrl_Rd()					ithReadRegA(udc_reg(0x80))

#define mOtgC_A_Bus_Drop()				do { mOtg_Ctrl_A_BUS_DROP_Clr(); mOtg_Ctrl_A_BUS_REQ_Clr(); } while(0)
#define mOtgC_A_Bus_Drive()				do { mOtg_Ctrl_A_BUS_DROP_Clr(); mOtg_Ctrl_A_BUS_REQ_Set(); } while(0)

#define OTG_ID_A_TYPE		0
#define OTG_ID_B_TYPE		BIT21

#define OTG_ROLE_HOST 		0
#define OTG_ROLE_DEVICE		BIT20


// OTG interrupt status register (0x84)
#define mOtg_INT_STS_Rd()				ithReadRegA(udc_reg(0x84))
#define mOtg_INT_STS_Clr(value)			ithWriteRegMaskA(udc_reg(0x84), value, value)

#if 0
#define OTGC_INT_BSRPDN				BIT0  
#define OTGC_INT_ASRPDET            BIT4
#define OTGC_INT_AVBUSERR           BIT5
#define OTGC_INT_RLCHG              BIT8
#define OTGC_INT_IDCHG              BIT9
#define OTGC_INT_OVC                BIT10
#define OTGC_INT_BPLGRMV            BIT11
#define OTGC_INT_APLGRMV            BIT12

#define OTGC_INT_A_TYPE   (OTGC_INT_ASRPDET|OTGC_INT_AVBUSERR|OTGC_INT_OVC|OTGC_INT_RLCHG|OTGC_INT_IDCHG|OTGC_INT_BPLGRMV|OTGC_INT_APLGRMV)
#define OTGC_INT_B_TYPE   (OTGC_INT_BSRPDN|OTGC_INT_AVBUSERR|OTGC_INT_OVC|OTGC_INT_RLCHG|OTGC_INT_IDCHG)
#endif

// OTG interrupt enable register (0x88)
#define mOtg_INT_Enable_Rd()			ithReadRegA(udc_reg(0x88))
#define mOtg_INT_Enable_Set(value)		ithWriteRegMaskA(udc_reg(0x88), value, value)
#define mOtg_INT_Enable_Clr(value)		ithWriteRegMaskA(udc_reg(0x88), 0x0, value)


/*-------------------------------------------------------------------------*/

// Main control register(0x100)
#define mUsbRmWkupST()				(ithReadRegA(udc_reg(0x100)) & BIT0)
#define mUsbRmWkupSet()				ithWriteRegMaskA(udc_reg(0x100), BIT0, BIT0)
#define mUsbRmWkupClr()				ithWriteRegMaskA(udc_reg(0x100),  0x0, BIT0)

#define mUsbTstHalfSpeedEn()		ithWriteRegMaskA(udc_reg(0x100), BIT1, BIT1)
#define mUsbTstHalfSpeedDis()		ithWriteRegMaskA(udc_reg(0x100),  0x0, BIT1)

#define mUsbGlobIntEnSet()			ithWriteRegMaskA(udc_reg(0x100), BIT2, BIT2)
#define mUsbGlobIntDis()			ithWriteRegMaskA(udc_reg(0x100),  0x0, BIT2)

#define mUsbGoSuspend()				ithWriteRegMaskA(udc_reg(0x100), BIT3, BIT3)
#define mUsbGoSuspendClr()			ithWriteRegMaskA(udc_reg(0x100), 0x0, BIT3)

#define mUsbSoftRstSet()			ithWriteRegMaskA(udc_reg(0x100), BIT4, BIT4)
#define mUsbSoftRstClr()			ithWriteRegMaskA(udc_reg(0x100),  0x0, BIT4)

#define mUsbChipEnSet()				ithWriteRegMaskA(udc_reg(0x100), BIT5, BIT5)
#define mUsbChipDis()				ithWriteRegMaskA(udc_reg(0x100), 0x0, BIT5)

#define mUsbHighSpeedST()			(ithReadRegA(udc_reg(0x100)) & BIT6)
#define mUsbDMAResetSet()			ithWriteRegMaskA(udc_reg(0x100), BIT8, BIT8)
#define mUsbForceFsEn()				ithWriteRegMaskA(udc_reg(0x100), BIT9, BIT9)


// Device address register(0x104)
#define mUsbDevAddrSet(value)		ithWriteRegA(udc_reg(0x104), value)
#define mUsbCfgST()					(ithReadRegA(udc_reg(0x104)) & BIT7)
#define mUsbCfgSet()				ithWriteRegMaskA(udc_reg(0x104), BIT7, BIT7)
#define mUsbCfgClr()				ithWriteRegMaskA(udc_reg(0x104),  0x0, BIT7)


// Test register(0x108)
#define mUsbClrAllFIFOSet()			ithWriteRegMaskA(udc_reg(0x108), BIT0, BIT0)
#define mUsbClrAllFIFOClr()			ithWriteRegMaskA(udc_reg(0x108),  0x0, BIT0)


// SOF Frame Number register(0x10C)
#define mUsbFrameNo()				(ithReadRegA(udc_reg(0x10C)) & 0x7FF)
#define mUsbMicroFrameNo()			((ithReadRegA(udc_reg(0x10C)) & 0x3800) >> 11)
#define mUsbFrameNoLow()			(mUsbFrameNo() & 0xFF)
#define mUsbFrameNoHigh()			(mUsbFrameNo() >> 8)


// SOF Mask register(0x110)
#define mUsbSofMskTimer(value)      ithWriteRegMaskA(udc_reg(0x110),  value, 0xFFFF)

// PHY Test Mode Selector register(0x114)
#define mUsbTsMdWr(item)			ithWriteRegA(udc_reg(0x114), item)
#define mUsbUnPLGClr()				ithWriteRegMaskA(udc_reg(0x114),  0x0, BIT0)
#define mUsbUnPLGSet()				ithWriteRegMaskA(udc_reg(0x114), BIT0, BIT0)

// Used for test pattern traffic
#define TEST_J					0x02
#define TEST_K					0x04
#define TEST_SE0_NAK			0x08
#define TEST_PKY				0x10


// Vendor Specific IO Control register(0x118)
// Cx configuration and status register(0x11C)

// Cx configuration and FIFO Empty Status register(0x120)
#define mUsbEP0DoneSet()			ithWriteRegMaskA(udc_reg(0x120), BIT0, BIT0)
#define mUsbTsPkDoneSet()			ithWriteRegMaskA(udc_reg(0x120), BIT1, BIT1)
#define mUsbEP0StallSet()			ithWriteRegMaskA(udc_reg(0x120), BIT2, BIT2)
#define mUsbCxFClr()				ithWriteRegMaskA(udc_reg(0x120), BIT3, BIT3)

#define mUsbCxFFull()				(ithReadRegA(udc_reg(0x120)) & BIT4)
#define mUsbCxFEmpty()				(ithReadRegA(udc_reg(0x120)) & BIT5)
#define mUsbCxFByteCnt()			((ithReadRegA(udc_reg(0x120)) & 0x7F000000) >> 24)


// IDLE Counter register(0x124)
#define mUsbIdleCnt(time)			ithWriteRegA(udc_reg(0x124), time)


// Mask of interrupt group(0x130)
#define mUsbIntGrpAllDis()			ithWriteRegMaskA(udc_reg(0x130), (BIT0|BIT1|BIT2), (BIT0|BIT1|BIT2))
#define mUsbIntGrpAllEn()			ithWriteRegMaskA(udc_reg(0x130), 0x0, (BIT0|BIT1|BIT2))
#define mUsbIntGroupMaskRd()		ithReadRegA(udc_reg(0x130))
#define mUsbIntGrp0En()			    ithWriteRegMaskA(udc_reg(0x130), 0x0, BIT0)
#define mUsbIntGrp1En()			    ithWriteRegMaskA(udc_reg(0x130), 0x0, BIT1)
#define mUsbIntGrp2En()			    ithWriteRegMaskA(udc_reg(0x130), 0x0, BIT2)
#define mUsbIntGrp0Dis()		    ithWriteRegMaskA(udc_reg(0x130), BIT0, BIT0)
#define mUsbIntGrp1Dis()		    ithWriteRegMaskA(udc_reg(0x130), BIT1, BIT1)
#define mUsbIntGrp2Dis()		    ithWriteRegMaskA(udc_reg(0x130), BIT2, BIT2)


// Mask of interrupt source group 0(0x134)
#define mUsbIntEP0SetupDis()		ithWriteRegMaskA(udc_reg(0x134), BIT0, BIT0)
#define mUsbIntEP0InDis()			ithWriteRegMaskA(udc_reg(0x134), BIT1, BIT1)
#define mUsbIntEP0OutDis()			ithWriteRegMaskA(udc_reg(0x134), BIT2, BIT2)
#define mUsbIntEP0EndDis()			ithWriteRegMaskA(udc_reg(0x134), BIT3, BIT3)
#define mUsbIntEP0FailDis()			ithWriteRegMaskA(udc_reg(0x134), BIT4, BIT4)

#define mUsbIntEP0SetupEn()			ithWriteRegMaskA(udc_reg(0x134), 0x0, BIT0)
#define mUsbIntEP0InEn()			ithWriteRegMaskA(udc_reg(0x134), 0x0, BIT1)
#define mUsbIntEP0OutEn()			ithWriteRegMaskA(udc_reg(0x134), 0x0, BIT2)
#define mUsbIntEP0EndEn()			ithWriteRegMaskA(udc_reg(0x134), 0x0, BIT3)
#define mUsbIntEP0FailEn()			ithWriteRegMaskA(udc_reg(0x134), 0x0, BIT4)

#define mUsbIntSrc0MaskRd()			ithReadRegA(udc_reg(0x134))


// Mask of interrupt source group 1(0x138)
#define mUsbIntFIFO0_3OUTDis()		ithWriteRegMaskA(udc_reg(0x138), 0xFF, 0xFF)
#define mUsbIntFIFO0_3INDis()		ithWriteRegMaskA(udc_reg(0x138), 0xF0000, 0xF0000)

#define mUsbIntFXOUTEn(bnum)		ithWriteRegMaskA(udc_reg(0x138), 0x0, ((BIT0<<((bnum)*2+1)) | (BIT0<<((bnum)*2))))
#define mUsbIntFXOUTDis(bnum)		ithWriteRegMaskA(udc_reg(0x138), ((BIT0<<((bnum)*2+1)) | (BIT0<<((bnum)*2))), ((BIT0<<((bnum)*2+1)) | (BIT0<<((bnum)*2))))

#define mUsbIntFXINEn(bnum)			ithWriteRegMaskA(udc_reg(0x138), 0x0, (BIT0<<((bnum)+16)))
#define mUsbIntFXINDis(bnum)		ithWriteRegMaskA(udc_reg(0x138), (BIT0<<((bnum)+16)), (BIT0<<((bnum)+16)))

#define mUsbIntSrc1MaskRd()			ithReadRegA(udc_reg(0x138))

#define mUsbIntSrc1MaskDis(msk)		ithWriteRegMaskA(udc_reg(0x138), msk, msk)


// Mask of interrupt source group 2(DMA int mask)(0x13C)
#define mUsbIntWakeupByVbusDis()	ithWriteRegMaskA(udc_reg(0x13C), BIT10, BIT10)
#define mUsbIntWakeupByVbusEn()		ithWriteRegMaskA(udc_reg(0x13C), 0x0, BIT10)

#define mUsbIntDevIdleDis()			ithWriteRegMaskA(udc_reg(0x13C), BIT9, BIT9)
#define mUsbIntDevIdleEn()			ithWriteRegMaskA(udc_reg(0x13C), 0x0, BIT9)

#define mUsbIntDmaFinishDis()		ithWriteRegMaskA(udc_reg(0x13C), BIT7, BIT7)
#define mUsbIntDmaFinishEn()		ithWriteRegMaskA(udc_reg(0x13C), 0x0, BIT7)

#define mUsbIntDmaErrDis()			ithWriteRegMaskA(udc_reg(0x13C), BIT8, BIT8)
#define mUsbIntDmaErrEn()			ithWriteRegMaskA(udc_reg(0x13C), 0x0, BIT8)

#define mUsbIntSrc2MaskRd()			ithReadRegA(udc_reg(0x13C))


// Interrupt group (0x140)
#define mUsbIntGroupRegRd()			ithReadRegA(udc_reg(0x140))


// Interrupt source group 0(0x144)
#define mUsbIntSrc0Rd()				ithReadRegA(udc_reg(0x144))
#define mUsbIntEP0AbortClr()		ithWriteRegMaskA(udc_reg(0x144), 0x0, BIT5)


// Interrupt source group 1(0x148)
#define mUsbIntSrc1Rd()				ithReadRegA(udc_reg(0x148))


// Interrupt source group 2(0x14C)
#define mUsbIntSrc2Rd()				ithReadRegA(udc_reg(0x14C))
#define mUsbIntBusRstClr()			ithWriteRegMaskA(udc_reg(0x14C), 0x0, BIT0)
#define mUsbIntSuspClr()			ithWriteRegMaskA(udc_reg(0x14C), 0x0, BIT1)
#define mUsbIntResmClr()			ithWriteRegMaskA(udc_reg(0x14C), 0x0, BIT2)
#define mUsbIntIsoSeqErrClr()		ithWriteRegMaskA(udc_reg(0x14C), 0x0, BIT3)
#define mUsbIntIsoSeqAbortClr()		ithWriteRegMaskA(udc_reg(0x14C), 0x0, BIT4)
#define mUsbIntTX0ByteClr()			ithWriteRegMaskA(udc_reg(0x14C), 0x0, BIT5)
#define mUsbIntRX0ByteClr()			ithWriteRegMaskA(udc_reg(0x14C), 0x0, BIT6)
#define mUsbIntDmaFinishClr()		ithWriteRegMaskA(udc_reg(0x14C), 0x0, BIT7)
#define mUsbIntDmaErrClr()			ithWriteRegMaskA(udc_reg(0x14C), 0x0, BIT8)

#define mUsbIntDmaFinishRd()		(ithReadRegA(udc_reg(0x14C)) & BIT7)
#define mUsbIntDmaErrRd()			(ithReadRegA(udc_reg(0x14C)) & BIT8)


// Rx 0 byte packet register(0x150)
#define mUsbIntRX0ByteRd()			(ithReadRegA(udc_reg(0x150)) & 0xFF)
#define mUsbIntRX0ByteSetClr(data)	ithWriteRegMaskA(udc_reg(0x150), 0x0, data)


// Tx 0 byte packet register(0x154)
#define mUsbIntTX0ByteRd()			(ithReadRegA(udc_reg(0x154)) & 0xFF)
#define mUsbIntTX0ByteSetClr(data)	ithWriteRegMaskA(udc_reg(0x154), 0x0, data)


// ISO sequential Error/Abort register(0x158)
#define mUsbIntIsoSeqErrRd()			((ithReadRegA(udc_reg(0x158)) & 0xFF0000) >> 16)
#define mUsbIntIsoSeqErrSetClr(data)	ithWriteRegMaskA(udc_reg(0x158), 0x0, (data<<16))

#define mUsbIntIsoSeqAbortRd()			(ithReadRegA(udc_reg(0x158)) & 0xFF)
#define mUsbIntIsoSeqAbortSetClr(data)	ithWriteRegMaskA(udc_reg(0x158), 0x0, data)


// IN Endpoint MaxPacketSize register(0x160,0x164,...,0x17C)
#define mUsbEPMxPtSz(EPn, dir, size)	ithWriteRegA(udc_reg(0x160 + ((1-dir) * 0x20) + ((EPn - 1) << 2)), size)
#define mUsbEPMxPtSzClr(EPn, dir)		ithWriteRegA(udc_reg(0x160 + ((1-dir) * 0x20) + ((EPn - 1) << 2)), 0x0)
#define mUsbEPMxPtSzRd(EPn, dir)		ithReadRegA(udc_reg(0x160 + ((1-dir) * 0x20) + ((EPn - 1) << 2)))

#define mUsbEPinHighBandSet(EPn, dir , size)	ithWriteRegMaskA(udc_reg(0x160 + ((EPn - 1) << 2)), ((((size >> 11)+1) << 13)* dir), (BIT14|BIT13))

#define mUsbEPinMxPtSz(EPn)			(ithReadRegA(udc_reg(0x160 + ((EPn - 1) << 2))) & 0x7FF)
#define mUsbEPinStallST(EPn)		((ithReadRegA(udc_reg(0x160 + ((EPn - 1) << 2))) & BIT11) >> 11)
#define mUsbEPinStallClr(EPn)		ithWriteRegMaskA(udc_reg(0x160 + ((EPn - 1) << 2)), 0x0, BIT11)
#define mUsbEPinStallSet(EPn)		ithWriteRegMaskA(udc_reg(0x160 + ((EPn - 1) << 2)), BIT11, BIT11)
#define mUsbEPinRsTgClr(EPn)		ithWriteRegMaskA(udc_reg(0x160 + ((EPn - 1) << 2)), 0x0, BIT12)
#define mUsbEPinRsTgSet(EPn)		ithWriteRegMaskA(udc_reg(0x160 + ((EPn - 1) << 2)), BIT12, BIT12)
#define mUsbEPinZeroSet(EPn)		ithWriteRegMaskA(udc_reg(0x160 + ((EPn - 1) << 2)), BIT15, BIT15)


// OUT Endpoint MaxPacketSize register(0x180,0x164,...,0x19C)
#define mUsbEPoutMxPtSz(EPn)		(ithReadRegA(udc_reg(0x180 + ((EPn - 1) << 2))) & 0x7FF)
#define mUsbEPoutStallST(EPn)		((ithReadRegA(udc_reg(0x180 + ((EPn - 1) << 2))) & BIT11) >> 11)
#define mUsbEPoutStallClr(EPn)		ithWriteRegMaskA(udc_reg(0x180 + ((EPn - 1) << 2)), 0x0, BIT11)
#define mUsbEPoutStallSet(EPn)		ithWriteRegMaskA(udc_reg(0x180 + ((EPn - 1) << 2)), BIT11, BIT11)
#define mUsbEPoutRsTgClr(EPn)		ithWriteRegMaskA(udc_reg(0x180 + ((EPn - 1) << 2)), 0x0, BIT12)
#define mUsbEPoutRsTgSet(EPn)		ithWriteRegMaskA(udc_reg(0x180 + ((EPn - 1) << 2)), BIT12, BIT12)


// Endpoint 1~4 Map register(0x1A0), Endpoint 5~8 Map register(0x1A4)
#define mUsbEPMap(EPn, MAP)	\
    do {\
        if(EPn <= 4)\
            ithWriteRegMaskA(udc_reg(0x1A0), (MAP << ((EPn-1)<<3)), (0xFF << ((EPn-1)<<3)));\
        else\
            ithWriteRegMaskA(udc_reg(0x1A4), (MAP << ((EPn-5)<<3)), (0xFF << ((EPn-5)<<3)));\
    } while(0) 											
#define mUsbEPMapRd(EPn)			(uint8_t)(ithReadRegA(udc_reg(0x1A0 + (((EPn-1)/4) << 2))) >> (((EPn-1)%4)<<3))
#define mUsbEPMapAllClr()			ithWriteRegA(udc_reg(0x1A0), 0x0);  ithWriteRegA(udc_reg(0x1A4), 0x0)
#define mUsbEPMap1_4Rd()			ithReadRegA(udc_reg(0x1A0))


// FIFO Map register(0x1A8)
#define mUsbFIFOMap(fifo_num, MAP)	ithWriteRegMaskA(udc_reg(0x1A8), ((MAP) << ((fifo_num) << 3)), (0xFF << ((fifo_num) << 3)))
#define mUsbFIFOMapRd(fifo_num)		((ithReadRegA(udc_reg(0x1A8)) >> ((fifo_num) << 3)) & 0xFF)
#define mUsbFIFOMapAllClr()			ithWriteRegA(udc_reg(0x1A8), 0x0)
#define mUsbFIFOMapAllRd()			ithReadRegA(udc_reg(0x1A8))


// FIFO Configuration register(0x1AC)
#define mUsbFIFOConfig(fifo_num, CONFIG)	ithWriteRegMaskA(udc_reg(0x1AC), ((CONFIG) << ((fifo_num) << 3)), (0xFF << ((fifo_num) << 3)))
#define mUsbFIFOConfigRd(fifo_num)	((ithReadRegA(udc_reg(0x1AC)) >> ((fifo_num) << 3)) & 0xFF)
#define mUsbFIFOConfigAllClr()		ithWriteRegA(udc_reg(0x1AC), 0x0)
#define FIFOEnBit					0x20
#define mUsbFIFOConfigAllRd()		ithReadRegA(udc_reg(0x1AC))


// FIFO byte count register(0x1B0)
#define mUsbFIFOOutByteCount(fifo_num)		(ithReadRegA(udc_reg(0x1B0 + ((fifo_num) << 2))) & 0x7FF)
#define mUsbFIFODone(fifo_num)		ithWriteRegMaskA(udc_reg(0x1B0 + ((fifo_num) << 2)), BIT11, BIT11)
#define mUsbFIFOReset(fifo_num)		ithWriteRegMaskA(udc_reg(0x1B0 + ((fifo_num) << 2)), BIT12, BIT12)
#define mUsbFIFOResetOK(fifo_num)	ithWriteRegMaskA(udc_reg(0x1B0 + ((fifo_num) << 2)), 0x0, BIT12)


// DMA target FIFO register(0x1C0)
#define UDC_DMA2FIFO_Non 	0
#define UDC_DMA2FIFO0 		BIT0
#define UDC_DMA2FIFO1 		BIT1
#define UDC_DMA2FIFO2 		BIT2
#define UDC_DMA2FIFO3 		BIT3
#define UDC_DMA2CxFIFO 		BIT4

#define mUsbDMA2FIFOSel(sel)		ithWriteRegA(udc_reg(0x1C0), sel)
#define mUsbDMA2FIFORd()			ithReadRegA(udc_reg(0x1C0))


// DMA parameter set 1 (0x1C8)
#define mUsbDmaConfig(len,dir)		ithWriteRegA(udc_reg(0x1C8), (((len)<<8) | ((dir)<<1)))
#define mUsbDmaLenRd()				((ithReadRegA(udc_reg(0x1C8)) & 0x1FFFF00) >> 8)
#define mUsbDmaConfigRd()			ithReadRegA(udc_reg(0x1C8))

#define mUsbDmaStart()				ithWriteRegMaskA(udc_reg(0x1C8), BIT0, BIT0)
#define mUsbDmaStop()				ithWriteRegMaskA(udc_reg(0x1C8), 0x0, BIT0)

#define mUsbDmaAbort()				ithWriteRegA(udc_reg(0x1C8), (mUsbDmaConfigRd()&~0x1)|0x18)

// DMA parameter set 2 (0x1CC)
#define mUsbDmaAddr(addr)			ithWriteRegA(udc_reg(0x1CC), addr)
#define mUsbDmaAddrRd()				ithReadRegA(udc_reg(0x1CC))


// 8 byte command data port(0x1D0)
#define mUsbEP0CmdDataRdDWord()		ithReadRegA(udc_reg(0x1D0))



#ifdef __cplusplus
}
#endif


#endif // UDC_REG_H

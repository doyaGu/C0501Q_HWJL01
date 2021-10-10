/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __RTL8195A_HAL_H__
#define __RTL8195A_HAL_H__

#ifdef CONFIG_SDIO_HCI
#define MAX_DLFW_PAGE_SIZE			2048	// firmware limit
#elif defined CONFIG_USB_HCI
	#define MAX_DLFW_PAGE_SIZE			1024	// firmware limit
#elif defined CONFIG_GSPI_HCI
	#define MAX_DLFW_PAGE_SIZE			2048	// firmware limit, same to SDIO
#endif
typedef enum _FIRMWARE_SOURCE {
	FW_SOURCE_IMG_FILE = 0,
	FW_SOURCE_HEADER_FILE = 1,		//from header file
} FIRMWARE_SOURCE, *PFIRMWARE_SOURCE;

#if 1 // all data need to be modified for rtl8195a, copy from rtl8189es
#define FW_8195A_SIZE				(1024*512) //512k
#define FW_8195A_START_ADDRESS	0x1000
#define FW_8195A_END_ADDRESS		0x1FFF //0x5FFF

#define IS_FW_HEADER_EXIST_95A(_pFwHdr)	((le16_to_cpu(_pFwHdr->Signature)&0xFFF0) == 0x88E0)

typedef struct _RT_FIRMWARE_8195A {
	FIRMWARE_SOURCE	eFWSource;
#ifdef CONFIG_EMBEDDED_FWIMG
	u8*		szFwBuffer;
#else
//	u8		szFwBuffer[FW_8195A_SIZE];
	u8*		szFwBuffer;
#endif
	u32		ulFwLength;
} RT_FIRMWARE_8195A , *PRT_FIRMWARE_8195A;

//
// This structure must be cared byte-ordering
//
typedef struct _RT_8195A_IMG1_HDR
{
	u8		CalData[16];    // Flash calibration data
	u32		Img1Size;
	u32		StartAddr;
	u16		Img2Offset;     // Offset = Img2Offset*1024
	u16		dummy1;
	u32		dummy2;
	u32		StartFunc;
	u32		WakeupFunc;
}RT_8195A_IMG1_HDR, *PRT_8195A_IMG1_HDR;

//
// This structure must be cared byte-ordering
//
typedef struct _RT_8195A_FIRMWARE_HDR
{
	u32		FwSize;
	u32		StartAddr;
	u8		Dummy[8];
	u32		StartFunc;
	u8		Signature[8];
	u16		Version;        // FW Version
	u16		Subversion; // FW Subversion, default 0x00
	u16		ChipId;
	u8		ChipVer;
	u8		BusType;    
	u8		Rsvd1;
	u8		Rsvd2;
	u8		Rsvd3;
	u8		Rsvd4;
}RT_8195A_FIRMWARE_HDR, *PRT_8195A_FIRMWARE_HDR;

#endif // download firmware related data structure

s32 rtl8195a_FirmwareDownload(PADAPTER padapter, BOOLEAN  bUsedWoWLANFw);
s32 rtl8195a_start_thread(_adapter *padapter);
void rtl8195a_stop_thread(_adapter *padapter);
#endif

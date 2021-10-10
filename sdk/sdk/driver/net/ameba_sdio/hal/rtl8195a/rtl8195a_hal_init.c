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
#define _HAL_INIT_C_

#include <linux/err.h> // Irene Lin
#include <drv_types.h>
#include <rtw_debug.h>
#include <rtl8195a_hal.h>
#include <8195_com_reg.h>
#include <rtw_io.h>
#include <8195_desc.h>


#ifdef CONFIG_SDIO_HCI 
	#include "sdio_ops.h"
	#include "8195_sdio_reg.h"
#elif defined(CONFIG_USB_HCI)
	#include "usb_ops_linux.h"
#elif  defined(CONFIG_GSPI_HCI)
	#include <8195_gspi_reg.h>
	#include <freertos/generic.h>
#endif


static int
_FwPageWrite(
	IN		PADAPTER	padapter,
	IN		u32			offset,
	IN		PVOID		pdata,
	IN		u32			size
	)
{
	u8 ret = _FAIL;
	u8 * tx_buff = NULL;
	u8 * rx_buff = NULL;
	TX_DESC_MW	*ptx_des;
#ifdef CONFIG_USB_HCI
	RX_DESC_MW	*prx_des;
#endif
	u32 buf_sz;
	
#if defined(CONFIG_GSPI_HCI) || defined(CONFIG_SDIO_HCI)
	u32 i;
	u16 old_hcpwm2;
	u16 new_hcpwm2;
#endif     
	buf_sz = (((size + SIZE_TX_DESC_8195a - 1) >> 9)+1) << 9;
	tx_buff = rtw_zmalloc(buf_sz);
	if(tx_buff == NULL)
		goto exit;

	ptx_des = (TX_DESC_MW *)tx_buff;
	ptx_des->txpktsize = cpu_to_le16(size);
	ptx_des->offset = SIZE_TX_DESC_8195a;
	ptx_des->bus_agg_num = 1;
	ptx_des->type = TX_MEM_WRITE;
#ifdef CONFIG_SDIO_HCI
	ptx_des->reply = 0;
	old_hcpwm2 = rtw_read16(padapter, SDIO_REG_HCPWM2); 
#elif defined(CONFIG_USB_HCI)
	ptx_des->reply = 1;
#elif defined(CONFIG_GSPI_HCI)
	ptx_des->reply = 0;
	old_hcpwm2 = rtw_read16(padapter, SPI_REG_HCPWM2); 
#endif
	ptx_des->start_addr = cpu_to_le32(offset);
	ptx_des->write_len = cpu_to_le16(size);

	rtw_memcpy(tx_buff + SIZE_TX_DESC_8195a, pdata, size);

	rx_buff = rtw_zmalloc(sizeof(RX_DESC_MW));
	if(rx_buff == NULL)
		goto exit;

#ifdef CONFIG_SDIO_HCI
	ret = rtw_write_port(padapter, WLAN_TX_FIFO_DEVICE_ID, (size + SIZE_TX_DESC_8195a), tx_buff);
	rtw_msleep_os(50); // wait memory write done
	for (i=0;i<100;i++) {
		new_hcpwm2 = rtw_read16(padapter, SDIO_REG_HCPWM2); 
		if ((new_hcpwm2 & BIT15) != (old_hcpwm2 & BIT15)) {
			// toggle bit(15)  is changed, it means the 8195a update its register value
			old_hcpwm2 = new_hcpwm2;
			if (new_hcpwm2 & SDIO_MEM_WR_DONE) {
				// 8195a memory write done
				ret = _SUCCESS;
				break;
			}
			rtw_msleep_os(10);
		}
		else {
			rtw_msleep_os(10);
		}        
	}
#elif defined(CONFIG_USB_HCI)
	ret = rtw_writeN(padapter, offset, (size + SIZE_TX_DESC_8195a), tx_buff);
	if (ret){
		// check firmware piece
		if(ptx_des->reply){
			rtw_readN(padapter, (u32)NULL, (SIZE_TX_DESC_8195a), rx_buff);
			prx_des = (RX_DESC_MW	*)rx_buff;
			// check write success
			if((prx_des->start_addr == offset)&& (prx_des->write_len == size) && (prx_des->result == 0))
				ret = _SUCCESS;
			else
				ret = _FAIL;
		}
	}
#elif defined(CONFIG_GSPI_HCI)
	ret = rtw_write_port(padapter, SPI_TXQ_FIFO_DEVICE_ID, (size + SIZE_TX_DESC_8195a), tx_buff);
	rtw_msleep_os(50); // wait memory write done
	for (i=0;i<100;i++) {
		new_hcpwm2 = rtw_read16(padapter, SPI_REG_HCPWM2); 
		if ((new_hcpwm2 & BIT15) != (old_hcpwm2 & BIT15)) {
			// toggle bit(15)  is changed, it means the 8195a update its register value
			old_hcpwm2 = new_hcpwm2;
			if (new_hcpwm2 & SPI_MEM_WR_DONE) {
				// 8195a memory write done
				ret = _SUCCESS;
				break;
			}
			rtw_msleep_os(10);
		}
		else {
			rtw_msleep_os(10);
		}        
	}
#endif


exit:	
	if(tx_buff)
		rtw_mfree(tx_buff, buf_sz);
	if(rx_buff)
		rtw_mfree(rx_buff, sizeof(RX_DESC_MW));
	return ret;
}


static int
_WriteFW(
	IN		PADAPTER		padapter,
	IN		PVOID			buffer,
	IN		u32         startaddr,
	IN		u32			size
	)
{
	int ret = _SUCCESS;
	u32 	pageNums,remainSize ;
	u32 	page, offset;
	u8	*bufferPtr = (u8*)buffer;
	u32  fw_startaddr;

	pageNums = size / MAX_DLFW_PAGE_SIZE ;
	remainSize = size % MAX_DLFW_PAGE_SIZE;

//	fw_startaddr = padapter->FirmwareStartAddr;
	fw_startaddr = startaddr;

	for (page = 0; page < pageNums; page++) {
		offset = page * MAX_DLFW_PAGE_SIZE;
		
		DBG_871X("%s: Write Mem: StartAddr=0x%08x Len=%d\n", __FUNCTION__
		    ,(fw_startaddr+offset), MAX_DLFW_PAGE_SIZE);

		ret = _FwPageWrite(padapter, (fw_startaddr+offset), bufferPtr+offset, MAX_DLFW_PAGE_SIZE);
		if(ret == _FAIL) {
			DBG_871X("%s: Error!", __FUNCTION__);
			goto exit;
		}
	}
	
	if (remainSize) {
		offset = pageNums * MAX_DLFW_PAGE_SIZE;
		page = pageNums;

	        DBG_871X("%s: Write Mem (Remain): StartAddr=0x%08x Len=%d\n", __FUNCTION__
	            ,(fw_startaddr+offset), remainSize);
			
		ret = _FwPageWrite(padapter, (fw_startaddr+offset), bufferPtr+offset, remainSize);
		
		if(ret == _FAIL)
			goto exit;

	}
	RT_TRACE(_module_hal_init_c_, _drv_info_, ("_WriteFW Done- for Normal chip.\n"));

exit:
	return ret;
}
#ifdef CONFIG_USB_HCI
int usb_firmware_isready(_adapter *padapter);
#endif 
static s32 check_firmware_status(_adapter *padapter){
	u8 fw_ready;
	s32 ret = _FAIL;
#ifdef CONFIG_SDIO_HCI
	fw_ready = rtw_read8(padapter, SDIO_REG_CPU_IND); 
	DBG_871X("%s: cpu_ind @ 0x%02x\n", __FUNCTION__, fw_ready);
	if (fw_ready&SDIO_SYSTEM_TRX_RDY_IND) {
		ret = _SUCCESS;
		DBG_871X("%s: firmware is already running!\n", __FUNCTION__);
	}
#elif defined CONFIG_USB_HCI
	fw_ready = usb_firmware_isready(padapter);
	if(fw_ready == 1){
		ret = _SUCCESS;
		DBG_871X("%s: firmware is already running!\n", __FUNCTION__);
	}
#elif defined CONFIG_GSPI_HCI
	fw_ready = rtw_read8(padapter, SPI_REG_CPU_IND); 
	DBG_871X("%s: cpu_ind @ 0x%02x\n", __FUNCTION__, fw_ready);
	if (fw_ready & SPI_CPU_RDY_IND) {
		ret = _SUCCESS;
		DBG_871X("%s: firmware is already running!\n", __FUNCTION__);
	}
#endif
	return ret;
}

#if 1
extern u8 g_fwdl_chksum_fail;
extern u8 g_fwdl_wintint_rdy_fail;
static s32 polling_fwdl_chksum(_adapter *adapter, u32 min_cnt, u32 timeout_ms)
{
	s32 ret = _FAIL;
	u32 value32 = 0;
	u32 start = rtw_get_current_time();
	u32 cnt = 0;
#if 0

	/* polling CheckSum report */
	do {
		cnt++;
		value32 = rtw_read32(adapter, REG_MCUFWDL);
		if (value32 & FWDL_ChkSum_rpt || adapter->bSurpriseRemoved || adapter->bDriverStopped)
			break;
		rtw_yield_os();
	} while (rtw_get_passing_time_ms(start) < timeout_ms || cnt < min_cnt);

	if (!(value32 & FWDL_ChkSum_rpt)) {
		goto exit;
	}

	if (g_fwdl_chksum_fail) {
		DBG_871X("%s: fwdl test case: fwdl_chksum_fail\n", __FUNCTION__);
		g_fwdl_chksum_fail--;
		goto exit;
	}
#endif
	ret = _SUCCESS;

//exit:
	DBG_871X("%s: Checksum report %s! (%u, %dms), REG_MCUFWDL:0x%08x\n", __FUNCTION__
	, (ret==_SUCCESS)?"OK":"Fail", cnt, rtw_get_passing_time_ms(start), value32);

	return ret;
}
#endif

static s32 _FWMemSet(_adapter *adapter, u32 Addr, u16 Size, u8 Data)
{
	s32 ret = _FAIL;
	TX_DESC_MS tx_des;
	u16 old_hcpwm2 = 0;
	u16 new_hcpwm2;
	u32 i;
    
	rtw_memset((void *)&tx_des, 0, sizeof(TX_DESC_MS));
	tx_des.txpktsize = 0;
	tx_des.offset = SIZE_TX_DESC_8195a;
	tx_des.bus_agg_num = 1;
	tx_des.type = TX_MEM_SET;
	tx_des.start_addr = cpu_to_le32(Addr);
	tx_des.write_len = cpu_to_le16(Size);
	tx_des.data = Data;

	DBG_871X("%s: Addr=0x%x Size=%u Data=0x%x\n", __FUNCTION__
	    ,Addr, Size, Data);
#ifdef CONFIG_SDIO_HCI
	ret = rtw_write_port(adapter, WLAN_TX_FIFO_DEVICE_ID, SIZE_TX_DESC_8195a, (u8 *)&tx_des);
	rtw_msleep_os(10); // wait memory write done
	for (i=0;i<1000;i++) {
		new_hcpwm2 = rtw_read16(adapter, SDIO_REG_HCPWM2); 
		if ((new_hcpwm2 & BIT15) != (old_hcpwm2 & BIT15)) {
			// toggle bit(15)  is changed, it means the 8195a update its register value
			old_hcpwm2 = new_hcpwm2;
			if (new_hcpwm2 & SDIO_MEM_ST_DONE) {
				// 8195a memory write done
				ret = _SUCCESS;
				break;
			}
			rtw_msleep_os(1);
		}
		else {
			rtw_msleep_os(1);
		}        
	}
#elif defined(CONFIG_USB_HCI)
	ret = rtw_writeN(adapter, USB_WRITE_ADD, SIZE_TX_DESC_8195a, (u8 *)&tx_des);
#elif defined(CONFIG_GSPI_HCI)
	ret = rtw_write_port(adapter, SPI_TXQ_FIFO_DEVICE_ID, SIZE_TX_DESC_8195a, (u8 *)&tx_des);
#endif

	return ret;
}

static s32 _FWFreeToGo(_adapter *adapter, u32 min_cnt, u32 timeout_ms)
{
	s32 ret = _FAIL;
	//u32 start = rtw_get_current_time();
	//u8 fw_ready;
	u32 i;

	TX_DESC_JS	tx_des;
	
	tx_des.txpktsize = 0;
	tx_des.offset = SIZE_TX_DESC_8195a;
	tx_des.bus_agg_num = 1;
	tx_des.type = TX_FM_FREETOGO;
	tx_des.start_fun = cpu_to_le32(adapter->FirmwareEntryFun);

	DBG_871X("%s: Jump to Entry Func @ 0x%08x\n", __FUNCTION__
	    ,adapter->FirmwareEntryFun);
#ifdef CONFIG_SDIO_HCI
	ret = rtw_write_port(adapter, WLAN_TX_FIFO_DEVICE_ID, SIZE_TX_DESC_8195a, (u8 *)&tx_des);
#elif defined(CONFIG_USB_HCI)
	ret = rtw_writeN(adapter, USB_WRITE_ADD, SIZE_TX_DESC_8195a, (u8 *)&tx_des);
#elif defined(CONFIG_GSPI_HCI)
	ret = rtw_write_port(adapter, SPI_TXQ_FIFO_DEVICE_ID, SIZE_TX_DESC_8195a, (u8 *)&tx_des);
#endif

#if 0
    // wait for the firmware going to re-load indication
    rtw_msleep_os(80);
    for (i=0;i<100;i++) {
        fw_ready = rtw_read8(adapter, SDIO_REG_CPU_IND); 
        if ((fw_ready & BIT0) == 0) {
            // it means the boot firmware aware the jump command
            break;
        }
        rtw_msleep_os(10);
    }

    // wait for the new downloaded firmware started
    rtw_msleep_os(500);
    for (i=0;i<100;i++) {
        fw_ready = rtw_read8(adapter, SDIO_REG_CPU_IND); 
        if (fw_ready & (BIT0)) {
            break;
        }
        rtw_msleep_os(10);
    }
    
    if (i==100) {
        DBG_871X("%s: Wait Firmware Start Timeout!!\n", __FUNCTION__);
	    ret = _FAIL;
    }
    else {
	ret = _SUCCESS;
    }
#endif
	// TODO: Pooling firmware ready here
	 for (i=0;i<100;i++) {
		if(check_firmware_status(adapter) == _SUCCESS)
			break;
		rtw_msleep_os(20);
	}
	if (i==100) {
		DBG_871X("%s: Wait Firmware Start Timeout!!\n", __FUNCTION__);
		ret = _FAIL;
	}
	else {
		ret = _SUCCESS;
	}
	return ret;
}

#ifdef CONFIG_FILE_FWIMG
extern char *rtw_fw_file_path;
extern char *rtw_fw_wow_file_path;
#endif //CONFIG_FILE_FWIMG


//
//	Description:
//		Download 8195a firmware code.
//
//
s32 rtl8195a_FirmwareDownload(PADAPTER padapter, BOOLEAN  bUsedWoWLANFw)
{
	s32	rtStatus = _SUCCESS;
	u8 write_fw = 0;
	u32 fwdl_start_time;
	
//#ifdef CONFIG_WOWLAN
//	u8			*FwImageWoWLAN;
//	u32			FwImageWoWLANLen;
//#endif

	PRT_FIRMWARE_8195A			pFirmware = NULL;
	PRT_8195A_IMG1_HDR			pImg1Hdr = NULL;
	PRT_8195A_FIRMWARE_HDR	pFwHdr = NULL;
	u8	*FwBuffer8195a=NULL;

	u8		*pFirmwareBuf;
	u32		FirmwareLen;
	u32		Img2StartAddr;
	u32		Img2Size;
#ifdef CONFIG_FILE_FWIMG
	u8 *fwfilepath;
#endif // CONFIG_FILE_FWIMG

	RT_TRACE(_module_hal_init_c_, _drv_info_, ("+%s\n", __FUNCTION__));

	//check if firmware is already running
	if(check_firmware_status(padapter) == _SUCCESS)
		goto exit;

	pFirmware = (PRT_FIRMWARE_8195A)rtw_zmalloc(sizeof(RT_FIRMWARE_8195A));
	if(!pFirmware)
	{
		DBG_871X("%s memory allocate failed 0\n", __FUNCTION__);
		rtStatus = _FAIL;
		goto exit;
	}

	FwBuffer8195a = rtw_zmalloc(FW_8195A_SIZE);
	if(!FwBuffer8195a)
	{
		DBG_871X("%s memory allocate failed 1\n", __FUNCTION__);
		rtStatus = _FAIL;
		goto exit;
	}


#ifdef CONFIG_FILE_FWIMG
#ifdef CONFIG_WOWLAN
		if (bUsedWoWLANFw)
		{
			//not used for Ameba
			fwfilepath = rtw_fw_wow_file_path;
		}
		else
#endif // CONFIG_WOWLAN
		{
			fwfilepath = rtw_fw_file_path;
		}
#endif // CONFIG_FILE_FWIMG


#ifdef CONFIG_FILE_FWIMG
	if(rtw_is_file_readable(fwfilepath) == _TRUE)
	{
		DBG_871X("%s accquire FW from file:%s\n", __FUNCTION__, fwfilepath);
		pFirmware->eFWSource = FW_SOURCE_IMG_FILE;
	}
	else
#endif //CONFIG_FILE_FWIMG
	{
		pFirmware->eFWSource = FW_SOURCE_HEADER_FILE;
	}

	switch(pFirmware->eFWSource)
	{
		case FW_SOURCE_IMG_FILE:
			#ifdef CONFIG_FILE_FWIMG
			rtStatus = rtw_retrive_from_file(fwfilepath, FwBuffer8195a, FW_8195A_SIZE);
			pFirmware->ulFwLength = rtStatus>=0?rtStatus:0;
			pFirmware->szFwBuffer = FwBuffer8195a;
			#endif //CONFIG_FILE_FWIMG
			break;
		case FW_SOURCE_HEADER_FILE:
			//not supported yet
			DBG_871X("%s accquire FW from file failed!\n", __FUNCTION__);
			rtStatus = _FAIL;
			goto exit;
			break;
	}


	if (pFirmware->ulFwLength > FW_8195A_SIZE) {
		rtStatus = _FAIL;
		DBG_871X_LEVEL(_drv_emerg_, "Firmware size:%u exceed %u\n", pFirmware->ulFwLength, FW_8195A_SIZE);
		goto exit;
	}
	
	pFirmwareBuf = pFirmware->szFwBuffer;
	FirmwareLen = pFirmware->ulFwLength;
#if 1
	// To Check Fw header. Added by tynli. 2009.12.04.
	pImg1Hdr = (PRT_8195A_IMG1_HDR)pFirmwareBuf;
	pImg1Hdr->Img1Size = le32_to_cpu(pImg1Hdr->Img1Size);
	pImg1Hdr->StartAddr = le32_to_cpu(pImg1Hdr->StartAddr);
	padapter->FirmwareEntryFun = le32_to_cpu(pImg1Hdr->StartFunc);  // Jump to Image1
	DBG_871X("%s: Image1 fw_startaddr=0x%08x fw_size=%d fw_entry=0x%08x\n",
	      __FUNCTION__, pImg1Hdr->StartAddr, pImg1Hdr->Img1Size, padapter->FirmwareEntryFun);

	pImg1Hdr->Img2Offset = le16_to_cpu(pImg1Hdr->Img2Offset);
	if (pImg1Hdr->Img2Offset != 0) {
		pFwHdr = (PRT_8195A_FIRMWARE_HDR)((u8*)pFirmwareBuf + (pImg1Hdr->Img2Offset*1024));
	} else {
		pFwHdr = (PRT_8195A_FIRMWARE_HDR)((u8*)pFirmwareBuf + pImg1Hdr->Img1Size);
	}

	padapter->FirmwareVersion =  le16_to_cpu(pFwHdr->Version);
	padapter->FirmwareSubVersion = le16_to_cpu(pFwHdr->Subversion);
//	padapter->FirmwareSignature = le16_to_cpu(pFwHdr->Signature);

//	padapter->FirmwareEntryFun = le32_to_cpu(pFwHdr->StartFunc);
//	padapter->FirmwareStartAddr = le32_to_cpu(pFwHdr->StartAddr);
	Img2StartAddr = le32_to_cpu(pFwHdr->StartAddr);
	Img2Size = le32_to_cpu(pFwHdr->FwSize);

	DBG_871X("%s: fw_ver=%04x fw_subver=%04x sig=%s\n",
		  __FUNCTION__, padapter->FirmwareVersion, padapter->FirmwareSubVersion, pFwHdr->Signature);

	DBG_871X("%s: Image2 fw_startaddr=0x%08x fw_size=%u\n",
	      __FUNCTION__, Img2StartAddr, Img2Size);

   
	// skip first 16 bytes
//    pFirmwareBuf = pFirmwareBuf + 32;
//    FirmwareLen = FirmwareLen - 32;
#else
	u32 image1_len;

	DBG_871X("pFirmwareBuf 0x%08x\n",pFirmwareBuf);

	rtw_memcpy(&image1_len, pFirmwareBuf + 0x10, 4);

	DBG_871X("image1_len 0x%08x\n",image1_len);
	rtw_memcpy(&padapter->FirmwareEntryFun, pFirmwareBuf + image1_len + 0x30, 4);
	rtw_memcpy(&padapter->FirmwareStartAddr, pFirmwareBuf+ image1_len + 0x24, 4);
	rtw_memcpy(&padapter->FirmwareSize, pFirmwareBuf+ image1_len + 0x20, 4);
 
	DBG_871X("image2_len 0x%08x\n",padapter->FirmwareSize);
	DBG_871X("image2_enterfuc 0x%08x\n",padapter->FirmwareEntryFun);
	DBG_871X("image2_startaddre 0x%08x\n",padapter->FirmwareStartAddr);

	    // skip first 16 bytes
    pFirmwareBuf = pFirmwareBuf + image1_len + 0x30 + 4;
    FirmwareLen = padapter->FirmwareSize;
    
#endif
	//_FWDownloadEnable_8195A(padapter, _TRUE);
	// Turn off FW debug message by write firmware variable ConfigDebugInfo=0
	_FWMemSet(padapter, 0x10000310, 4, 0);

	// Download Image1 first
	fwdl_start_time = rtw_get_current_time();
//    padapter->FirmwareStartAddr = pImg1Hdr->StartAddr;
	while(!padapter->bDriverStopped && !padapter->bSurpriseRemoved
	        && (write_fw++ < 3 || rtw_get_passing_time_ms(fwdl_start_time) < 500))
	{
		rtStatus = _WriteFW(padapter, pFirmwareBuf+32, pImg1Hdr->StartAddr, pImg1Hdr->Img1Size);
		if (rtStatus != _SUCCESS)
			continue;
#if 1
		rtStatus = polling_fwdl_chksum(padapter, 5, 50);
		if (rtStatus == _SUCCESS)
			break;
#endif
	}

	// Download Image2
	fwdl_start_time = rtw_get_current_time();
//    padapter->FirmwareStartAddr = Img2StartAddr;
	while(!padapter->bDriverStopped && !padapter->bSurpriseRemoved
			&& (write_fw++ < 3 || rtw_get_passing_time_ms(fwdl_start_time) < 500))
	{
		rtStatus = _WriteFW(padapter, ((u8*)pFwHdr+16), Img2StartAddr, Img2Size);
		if (rtStatus != _SUCCESS)
			continue;
#if 1
		rtStatus = polling_fwdl_chksum(padapter, 5, 50);
		if (rtStatus == _SUCCESS)
			break;
#endif
	}
	//_FWDownloadEnable_8195A(padapter, _FALSE);
	if(_SUCCESS != rtStatus)
		goto fwdl_stat;

	rtStatus = _FWFreeToGo(padapter, 10, 200);
	if (_SUCCESS != rtStatus)
		goto fwdl_stat;

fwdl_stat:
	DBG_871X("FWDL %s. write_fw:%u, %dms\n"
		, (rtStatus == _SUCCESS)?"success":"fail"
		, write_fw
		, rtw_get_passing_time_ms(fwdl_start_time)
	);

exit:
	if (pFirmware)
		rtw_mfree((u8*)pFirmware, sizeof(RT_FIRMWARE_8195A));

	if (FwBuffer8195a)
		rtw_mfree((u8*)FwBuffer8195a, FW_8195A_SIZE);

	return rtStatus;
}

#if defined(CONFIG_TX_AGGREGATION)|| defined(CONFIG_USB_TX_AGGREGATION)
extern thread_return rtl8195as_xmit_thread(thread_context context);
extern thread_return rtl8195au_xmit_thread(thread_context context);

#ifdef PLATFORM_ECOS
unsigned char rtl8195a_xmit_stack[24*1024];
cyg_handle_t rtl8195a_xmit_thread_handle;
cyg_thread rtl8195a_xmit_thread_obj;
extern int rtw_thread_pri;
static cyg_handle_t *kthread_run(thread_return (*_thread)(thread_context context), void *context, char *name){
	/* Create the thread */
	cyg_thread_create(rtw_thread_pri,
		      _thread,
		      (cyg_addrword_t)context,
		      name,
		      &rtl8195a_xmit_stack,
		      sizeof(rtl8195a_xmit_stack),
		      &rtl8195a_xmit_thread_handle,
		      &rtl8195a_xmit_thread_obj);
	/* Let the thread run when the scheduler starts */
	cyg_thread_resume(rtl8195a_xmit_thread_handle);
	return (&rtl8195a_xmit_thread_handle);
}
#ifndef IS_ERR
#define IS_ERR(thread) ((*thread == 0)?1:0)
#endif
#endif

s32 rtl8195a_start_thread(_adapter *padapter)
{
	struct xmit_priv *xmitpriv = &padapter->xmitpriv;
	_thread_hdl_ thread = NULL;

	thread_return	(*rtl8195a_xmit_thread)(thread_context context);
	
#ifdef CONFIG_SDIO_HCI
	rtl8195a_xmit_thread = rtl8195as_xmit_thread;
#elif defined(CONFIG_USB_HCI)
	rtl8195a_xmit_thread = rtl8195au_xmit_thread;
#endif

#if 1
	thread = kthread_run(rtl8195a_xmit_thread, padapter, "RTWHALXT");

	if (IS_ERR(thread))
	{
		RT_TRACE(_module_hal_xmit_c_, _drv_err_, ("%s: start rtl8195a_xmit_thread FAIL!!\n", __FUNCTION__));
		return _FAIL;
	}

#ifdef CONFIG_SDIO_HCI
	xmitpriv->SdioXmitThread = thread;
#elif defined(CONFIG_USB_HCI)
	xmitpriv->UsbXmitThread = thread;
#endif
	
	return _SUCCESS;
#else
	return rtw_create_task(xmitpriv->SdioXmitThread, "RTWHALXT", XMIT_STACKSIZE, TASK_PRORITY_HIGH, rtl8195as_xmit_thread, padapter);
#endif
}

void rtl8195a_stop_thread(_adapter *padapter)
{
	struct xmit_priv *xmitpriv = &padapter->xmitpriv;

#ifdef CONFIG_SDIO_HCI
	if (xmitpriv->SdioXmitThread){
		rtw_up_sema(&xmitpriv->SdioXmitSema);
		rtw_delete_task((struct task_struct *)xmitpriv->SdioXmitThread);
		rtw_down_sema(&xmitpriv->SdioXmitTerminateSema);
	}
#elif defined(CONFIG_USB_HCI)
	if (xmitpriv->UsbXmitThread){
		rtw_up_sema(&xmitpriv->UsbXmitSema);
		rtw_delete_task(xmitpriv->UsbXmitThread);
		rtw_down_sema(&xmitpriv->UsbXmitTerminateSema);
	}
#endif

}
#endif

#ifdef CONFIG_LOOPBACK_TEST
#define RTL8195A_LB_PKTSZ		2048
static void rtl8195a_fill_loopback_packet(u8 *pbuf, u16 sz){
	int i;
	for(i=0;i<sz;i++)
		rtw_memset(pbuf+i, i%256, 1);
}

#ifdef CONFIG_SDIO_HCI
extern s32 rtl8195as_loopback_send(PADAPTER padapter, u8 *pbuf, u16 sz);
#endif

static s32 rtl8195a_loopback_handler(PADAPTER padapter)
{
	PTXDESC_8195A ptxdesc;
	u8 *loopback_buf = NULL;
	s32 ret = _SUCCESS;
	
	loopback_buf = rtw_malloc(RTL8195A_LB_PKTSZ);
	if(loopback_buf == NULL){
		DBG_871X("memory allocate for loopback buffer failed!\n");
		rtw_msleep_os(10);
		return _SUCCESS;
	}
	ptxdesc = (PTXDESC_8195A)loopback_buf;
	ptxdesc->offset = SIZE_TX_DESC_8195a;
	ptxdesc->bus_agg_num = 1;
	ptxdesc->type = TX_PACKET_USER;
	ptxdesc->txpktsize = RTL8195A_LB_PKTSZ-ptxdesc->offset;

	rtl8195a_fill_loopback_packet(loopback_buf+ptxdesc->offset, ptxdesc->txpktsize);

#ifdef CONFIG_SDIO_HCI	
	ret = rtl8195as_loopback_send(padapter, loopback_buf, RTL8195A_LB_PKTSZ);
#endif

	return ret;
}

thread_return rtl8195a_loopback_thread(thread_context context)
{
	s32 ret;
	PADAPTER padapter= (PADAPTER)context;	
	
	ret = _SUCCESS;

	rtw_thread_enter("RTWHALXT");

	DBG_871X("start %s\n", __FUNCTION__);

	do {
#ifdef PLATFORM_LINUX
		if (kthread_should_stop())
			break;
#endif		
		ret = rtl8195a_loopback_handler(padapter);
		rtw_msleep_os(1000);
	} while (_SUCCESS == ret);

	RT_TRACE(_module_hal_xmit_c_, _drv_notice_, ("-%s\n", __FUNCTION__));
	DBG_871X("exit %s\n", __FUNCTION__);

	rtw_thread_exit();
}

void rtw_hal_start_loopback_test(_adapter *padapter){
	_thread_hdl_ thread = NULL;
	
	DBG_871X("%s\n", __FUNCTION__);
	
	thread = kthread_run(rtl8195a_loopback_thread, padapter, "RTWHALLB");

	if (IS_ERR(thread))
	{
		DBG_871X("%s: start rtl8195a_loopback_thread FAIL!!\n", __FUNCTION__);
		return;
	}
	
	padapter->LPThread = thread;
}

void rtw_hal_stop_loopback_test(_adapter *padapter){

	if (padapter->LPThread){
		rtw_delete_task(padapter->LPThread);
		padapter->LPThread = NULL;
	}
	
}
#endif

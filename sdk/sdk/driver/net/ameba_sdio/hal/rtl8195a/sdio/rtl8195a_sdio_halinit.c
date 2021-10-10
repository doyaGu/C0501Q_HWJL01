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
#define _SDIO_HALINIT_C_
#include "autoconf.h"
#ifndef CONFIG_SDIO_HCI
#error "CONFIG_SDIO_HCI shall be on!\n"
#endif
#include <stdlib.h> // Irene Lin: for strtoul()
#include "sdio_hal.h"
#include "sdio_ops.h"
#include "rtl8195a_xmit.h"
#include "rtl8195a_recv.h"
#include "rtl8195a_hal.h"
#include "8195_sdio_reg.h"
#include "rtl8195a_cmd.h"

static void _InitTxAvailBDThreshold(PADAPTER padapter)
{
	u32 freeBDNum;
	u16 txBDTh_l;
	u16 txBDTh_h;

	freeBDNum = SdioLocalCmd53Read4Byte(padapter, SDIO_REG_FREE_TXBD_NUM);
	//DBG_871X("%s: SDIO_REG_FREE_TXBD_NUM @ 0x%04x\n", __FUNCTION__, freeBDNum);
	
	//th_l = FreeNum-1, th_h = FreeNum/2	
	txBDTh_l = freeBDNum - 1;
	txBDTh_h = freeBDNum/2;
	
	rtw_write16(padapter, SDIO_REG_AVAI_BD_NUM_TH_L, txBDTh_l);
	rtw_write16(padapter, SDIO_REG_AVAI_BD_NUM_TH_H, txBDTh_h);
	//DBG_871X("%s: SDIO_REG_AVAI_BD_NUM_TH_L @ 0x%04x, SDIO_REG_AVAI_BD_NUM_TH_H @ 0x%04x\n", __FUNCTION__, 
	//	rtw_read16(padapter, SDIO_REG_AVAI_BD_NUM_TH_L),
	//	rtw_read16(padapter, SDIO_REG_AVAI_BD_NUM_TH_H));

}
static u32 HalRxAggr8195ASdio(PADAPTER padapter)
{
	// to confirm, for rx aggregation
#ifdef CONFIG_SDIO_RX_COPY
	u8	valueTimeout;
	u8	valueBDCount;
	u16	valueRXBDMax;

	//DMA timeout
	valueTimeout = 0x06;
	//AGG BD threshold
	valueBDCount = MAX_RX_AGG_NUM * 2; //SDIO needs 2 BD to send one rx packet

	//get rx bd max num
	valueRXBDMax = rtw_read16(padapter, SDIO_REG_RXBD_NUM);
#ifdef DBG_RX_AGG
	DBG_871X("%s: SDIO_REG_RX_AGG_CFG @ 0x%04x\n", __FUNCTION__, valueRXBDMax);
#endif

	if(valueBDCount > valueRXBDMax){
		DBG_871X("%s: HALINIT FAILED! RX_AGG_NUM(%d) > RXBD_NUM(%d)\n", __FUNCTION__, valueBDCount, valueRXBDMax);
		return _FAIL;
	}
	
	rtw_write8(padapter, SDIO_REG_RX_AGG_CFG+1, valueTimeout);
	rtw_write8(padapter, SDIO_REG_RX_AGG_CFG, valueBDCount);
	
	//enable rx aggregation
	rtw_write16(padapter, SDIO_REG_RX_AGG_CFG, rtw_read16(padapter, SDIO_REG_RX_AGG_CFG)|SDIO_RX_AGG_EN);
#ifdef DBG_RX_AGG
	DBG_871X("%s: SDIO_REG_RX_AGG_CFG @ 0x%04x\n", __FUNCTION__, 
		rtw_read16(padapter, SDIO_REG_RX_AGG_CFG));
#endif
#endif
	return _SUCCESS;
}

static u32 _initSdioAggregationSetting(PADAPTER padapter)
{
	u32 res = _SUCCESS;
	// Rx aggregation setting
	res = HalRxAggr8195ASdio(padapter);
	return res;
}


void _InitInterrupt(PADAPTER padapter)
{

	//HISR write one to clear
	rtw_write32(padapter, SDIO_REG_HISR, 0xFFFFFFFF);
	
	// HIMR - turn all off
	rtw_write32(padapter, SDIO_REG_HIMR, 0);

	//
	// Initialize and enable SDIO Host Interrupt.
	//
	InitInterrupt8195ASdio(padapter);
	
}

static u32 rtl8195as_hal_init(PADAPTER padapter){
	u8 res = _SUCCESS;
	u8 value8;
	//
	// Configure SDIO TxRx Control to enable Rx DMA timer masking.
	// 2010.02.24.
	//
	value8 = SdioLocalCmd52Read1Byte(padapter, SDIO_REG_TX_CTRL);
	DBG_871X("%s: SDIO_REG_TX_CTRL @ 0x%x\n", __FUNCTION__, value8);
	SdioLocalCmd52Write1Byte(padapter, SDIO_REG_TX_CTRL, value8 | 0x02);

	rtw_write8(padapter, SDIO_LOCAL_OFFSET|SDIO_REG_HRPWM, 0);

#ifdef CONFIG_FWDL
	res = rtl8195a_FirmwareDownload(padapter,_FAIL);
	if(res == _FAIL)
		goto exit;
#endif
	_InitTxAvailBDThreshold(padapter);
	res = _initSdioAggregationSetting(padapter);
	if(res == _FAIL)
		goto exit;
	_InitInterrupt(padapter);
	HalQueryTxBufferStatus8195ASdio(padapter);
	res = HalGetTxBufUnitSize8195ASdio(padapter);

exit:
	return res;
}

static u32 rtl8195as_hal_deinit(PADAPTER padapter){
	u8 res = _SUCCESS;

#ifdef CONFIG_POWER_SAVING
	DBG_871X("0x%x @ SDIO_HISR, 0x%x @ REQ_LEN\n", rtw_read32(padapter, SDIO_REG_HISR), 
	rtw_read32(padapter, SDIO_REG_RX0_REQ_LEN));
	rtw_hal_rpwm_notify(padapter, RPWM2_PWR_CG);
#endif

	return res;
}

static u32 rtl8195as_read_mac_addr(_adapter *padapter, u8 *mac){
	u8 res = _SUCCESS;
	char *cmd = "ATWZ=read_mac";
	u8 *pkt = NULL;
	u32 tot_len = SIZE_TX_DESC_8195a+strlen(cmd);
	TXDESC_8195A *txdesc;
	u32 hisr, time_out = 0, polling_num = 0, rx_len = 0;
	
	pkt = rtw_zmalloc(tot_len);
	if(pkt == NULL){
		DBG_871X("%s Failed\n", __FUNCTION__);
		return _FAIL;
	}
	txdesc = (PTXDESC_8195A)pkt;
	txdesc->txpktsize = cpu_to_le16(strlen(cmd));
	txdesc->offset = SIZE_TX_DESC_8195a;
	txdesc->bus_agg_num = 1;
	txdesc->type = TX_H2C_CMD;
	rtw_memcpy(pkt+SIZE_TX_DESC_8195a, cmd, strlen(cmd));
	sdio_write_port(padapter, WLAN_TX_FIFO_DEVICE_ID, tot_len, pkt);
	if(pkt)
		rtw_mfree(pkt, tot_len);

polling_rx_req:
	hisr = sdio_read32(padapter, SDIO_REG_HISR);
	polling_num++;
	if ((polling_num % 60) == 0) {
		time_out++;
		rtw_msleep_os(1);
	}
	if(time_out == 100)
		return _FAIL;
	if(hisr&SDIO_HISR_RX_REQUEST){
		u8 data[4];
		u8 rx_len_rdy, i;
		u8 *head = NULL, *end = NULL;
		//validate RX_LEN_RDY before reading RX0_REQ_LEN
		rx_len_rdy = sdio_read8(padapter, SDIO_REG_RX0_REQ_LEN+3);
		if(rx_len_rdy & BIT7){
			sdio_local_read(padapter, SDIO_REG_RX0_REQ_LEN, 4, data);
			rx_len = le16_to_cpu(*(u16*)data);
		}
		if(rx_len){
			pkt = rtw_zmalloc(rx_len);
			if(pkt == NULL){
				res = _FAIL;
				goto exit;
			}
			res = sdio_read_port(padapter, WLAN_RX_FIFO_DEVICE_ID, rx_len, pkt);
			if (res == _FAIL)
				goto exit;
			if(mac){
				rtw_memset(mac, 0, ETH_ALEN);
				head = end = pkt+SIZE_RX_DESC_8195a+11;
				for (i=0; i<ETH_ALEN; i++) {
					while (end && (*end != ':') && (*end != '\0'))
						end++;

					if (end && (*end == ':') )
						*end = '\0';

					//mac[i] = simple_strtoul(head, NULL, 16 );
					mac[i] = strtoul((const char *)head, NULL, 16 );

					if (end) {
						end++;
						head = end;
					}
				}
				head = end = NULL;
			}
		}
	}
	else
		goto polling_rx_req;
	DBG_871X("%s: "MAC_FMT"\n", __FUNCTION__, MAC_ARG(mac));
	if(MAC_VALIDATION(mac) == 0){
		res = _FAIL;
	}
exit:
	if(pkt)
		rtw_mfree(pkt, rx_len);
	return res;
}

void rtl8195as_set_hal_ops(PADAPTER padapter)
{
	struct hal_ops *pHalFunc = &padapter->HalFunc;
_func_enter_;
	pHalFunc->hal_init = &rtl8195as_hal_init;
	pHalFunc->hal_deinit = &rtl8195as_hal_deinit;
	
	pHalFunc->init_xmit_priv = &rtl8195as_init_xmit_priv;
	pHalFunc->free_xmit_priv = &rtl8195as_free_xmit_priv;

	pHalFunc->init_recv_priv = &rtl8195as_init_recv_priv;
	pHalFunc->free_recv_priv = &rtl8195as_free_recv_priv;
	
	pHalFunc->hal_xmit = &rtl8195as_hal_xmit;
	pHalFunc->hal_mgnt_xmit = &rtl8195as_hal_mgnt_xmit;
	pHalFunc->xmit_thread_handler = &rtl8195as_hal_xmit_handler;
#ifdef CONFIG_TX_AGGREGATION
	pHalFunc->run_thread= &rtl8195a_start_thread;
	pHalFunc->cancel_thread= &rtl8195a_stop_thread;
#endif	
	pHalFunc->enable_interrupt = &EnableInterrupt8195ASdio;
	pHalFunc->disable_interrupt = &DisableInterrupt8195ASdio;
#ifdef CONFIG_POWER_SAVING
	pHalFunc->hal_rpwm_notify = &rtl8195a_rpwm_notify;
#ifdef CONFIG_WOWLAN
	pHalFunc->hal_send_wowlan_cmd = &rtl8195a_send_wowlan_cmd;
#endif
#endif
	pHalFunc->hal_read_mac_addr = &rtl8195as_read_mac_addr;
_func_exit_;

}

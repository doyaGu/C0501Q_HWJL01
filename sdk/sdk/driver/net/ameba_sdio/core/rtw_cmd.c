#define _RTW_CMD_C_

#include <linux/kthread.h> // Irene Lin
#include "autoconf.h"
#include "rtw_debug.h"
#include "drv_types.h"
#include "rtw_xmit.h"
#include "rtw_ioctl.h"
#include "8195_desc.h"
#include "rtw_cmd.h"

u8 NULL_hdl(_adapter *padapter, u8 *pbuf)
{
	return H2C_SUCCESS;
}
/*
void rtw_sctx_init(struct completion *done)
{
	init_completion(done);
}
void rtw_sctx_complete(struct completion *done)
{
	complete(done);
}
s8 rtw_sctx_wait_timeout(struct completion *done, unsigned int timeout_ms)
{
	unsigned long expire;
	expire= timeout_ms ? msecs_to_jiffies(timeout_ms) : MAX_SCHEDULE_TIMEOUT;
	if(!wait_for_completion_timeout(done, expire))
	{	
		DBG_871X("%s timeout!\n", __FUNCTION__);
		return _FAIL;
	}
	return _SUCCESS;
}
*/
/*
Caller and the rtw_cmd_thread can protect cmd_q by spin_lock.
No irqsave is necessary.
*/
s32	rtw_init_cmd_priv (PADAPTER padapter)
{
	s32 res=_SUCCESS;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;	
_func_enter_;	

	rtw_init_sema(&(pcmdpriv->cmd_sema), 0);
	rtw_init_sema(&(pcmdpriv->CmdTerminateSema), 0);
	rtw_init_queue(&(pcmdpriv->cmd_queue));
	
	pcmdpriv->cmd_allocated_buf = rtw_zmalloc(MAX_CMDSZ + CMDBUFF_ALIGN_SZ);
	
	if (pcmdpriv->cmd_allocated_buf == NULL){
		res= _FAIL;
		goto exit;
	}
	
	pcmdpriv->cmd_buf = pcmdpriv->cmd_allocated_buf  +  CMDBUFF_ALIGN_SZ - ( (SIZE_PTR)(pcmdpriv->cmd_allocated_buf) & (CMDBUFF_ALIGN_SZ-1));

#ifdef CMD_RSP_BUF
	pcmdpriv->rsp_allocated_buf = rtw_zmalloc(MAX_RSPSZ + 4);
	
	if (pcmdpriv->rsp_allocated_buf == NULL){
		res= _FAIL;
		goto exit;
	}
	
	pcmdpriv->rsp_buf = pcmdpriv->rsp_allocated_buf  +  4 - ( (SIZE_PTR)(pcmdpriv->rsp_allocated_buf) & 3);
#endif

	rtw_mutex_init(&pcmdpriv->sctx_mutex);

exit:
	
_func_exit_;	  

	return res;
	
}	


void rtw_free_cmd_priv (PADAPTER padapter)
{
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
_func_enter_;

	if(pcmdpriv){
		rtw_spinlock_free(&(pcmdpriv->cmd_queue.lock));
		rtw_free_sema(&(pcmdpriv->cmd_sema));
		rtw_free_sema(&(pcmdpriv->CmdTerminateSema));

		if (pcmdpriv->cmd_allocated_buf)
			rtw_mfree(pcmdpriv->cmd_allocated_buf, MAX_CMDSZ + CMDBUFF_ALIGN_SZ);

#ifdef CMD_RSP_BUF
		if (pcmdpriv->rsp_allocated_buf)
			rtw_mfree(pcmdpriv->rsp_allocated_buf, MAX_RSPSZ + 4);
#endif	

		rtw_mutex_free(&pcmdpriv->sctx_mutex);
	}
_func_exit_;		
}


void rtw_free_cmd_obj(struct cmd_obj *pcmd)
{
_func_enter_;

	//free parmbuf in cmd_obj
	rtw_mfree((unsigned char*)pcmd->parmbuf, pcmd->cmdsz);
	
	if(pcmd->rsp!=NULL)
	{
		if(pcmd->rspsz!= 0)
		{
			//free rsp in cmd_obj
			rtw_mfree((unsigned char*)pcmd->rsp, pcmd->rspsz);
		}	
	}	

	//free cmd_obj
	rtw_mfree((unsigned char*)pcmd, sizeof(struct cmd_obj));
	
_func_exit_;		
}

int rtw_cmd_filter(struct cmd_priv *pcmdpriv, struct cmd_obj *cmd_obj)
{

#if 0	
	u8 bAllow = _FALSE; //set to _TRUE to allow enqueuing cmd when hw_init_completed is _FALSE

	#ifdef SUPPORT_HW_RFOFF_DETECTED
	//To decide allow or not
	if( (adapter_to_pwrctl(pcmdpriv->padapter)->bHWPwrPindetect)
		&&(!pcmdpriv->padapter->registrypriv.usbss_enable)
	)		
	{
		if(cmd_obj->cmdcode == GEN_CMD_CODE(_Set_Drv_Extra) ) 
		{
			struct drvextra_cmd_parm	*pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)cmd_obj->parmbuf; 
			if(pdrvextra_cmd_parm->ec_id == POWER_SAVING_CTRL_WK_CID)
			{	
				//DBG_871X("==>enqueue POWER_SAVING_CTRL_WK_CID\n");
				bAllow = _TRUE; 
			}
		}
	}
	#endif

#ifndef CONFIG_C2H_PACKET_EN
	/* C2H should be always allowed */
	if(cmd_obj->cmdcode == GEN_CMD_CODE(_Set_Drv_Extra)) {
		struct drvextra_cmd_parm *pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)cmd_obj->parmbuf;
		if(pdrvextra_cmd_parm->ec_id == C2H_WK_CID) {
			bAllow = _TRUE;
		}
	}
#endif

	if(cmd_obj->cmdcode == GEN_CMD_CODE(_SetChannelPlan))
		bAllow = _TRUE;

	if( (pcmdpriv->padapter->hw_init_completed ==_FALSE && bAllow == _FALSE)
		|| pcmdpriv->cmdthd_running== _FALSE	//com_thread not running
	)
	{
		//DBG_871X("%s:%s: drop cmdcode:%u, hw_init_completed:%u, cmdthd_running:%u\n", caller_func, __FUNCTION__,
		//	cmd_obj->cmdcode,
		//	pcmdpriv->padapter->hw_init_completed,
		//	pcmdpriv->cmdthd_running
		//);

		return _FAIL;
	}
#endif
	return _SUCCESS;
}


/*
Calling Context:

rtw_enqueue_cmd can only be called between kernel thread, 
since only spin_lock is used.

ISR/Call-Back functions can't call this sub-function.

*/

s32	_rtw_enqueue_cmd(_queue *queue, struct cmd_obj *obj)
{
	_irqL irqL;

_func_enter_;

	if (obj == NULL)
		goto exit;

	//rtw_enter_critical_bh(&queue->lock, &irqL);
	rtw_enter_critical(&queue->lock, &irqL);	

	rtw_list_insert_tail(&obj->list, &queue->queue);

	//rtw_exit_critical_bh(&queue->lock, &irqL);	
	rtw_exit_critical(&queue->lock, &irqL);

exit:	

_func_exit_;

	return _SUCCESS;
}

u32 rtw_enqueue_cmd(struct cmd_priv *pcmdpriv, struct cmd_obj *cmd_obj)
{
	int res = _FAIL;
	PADAPTER padapter = pcmdpriv->padapter;
	
_func_enter_;	
	
	if (cmd_obj == NULL) {
		goto exit;
	}

	cmd_obj->padapter = padapter;

	if( _FAIL == (res=rtw_cmd_filter(pcmdpriv, cmd_obj)) ) {
		rtw_free_cmd_obj(cmd_obj);
		goto exit;
	}

	res = _rtw_enqueue_cmd(&pcmdpriv->cmd_queue, cmd_obj);

	if(res == _SUCCESS)
		rtw_up_sema(&pcmdpriv->cmd_sema);
	
exit:	
	
_func_exit_;

	return res;
}


struct	cmd_obj	*_rtw_dequeue_cmd(_queue *queue)
{
	_irqL irqL;
	struct cmd_obj *obj;

_func_enter_;

	//_enter_critical_bh(&(queue->lock), &irqL);
	rtw_enter_critical(&queue->lock, &irqL);
	if (rtw_is_list_empty(&(queue->queue)))
		obj = NULL;
	else
	{
		obj = LIST_CONTAINOR(get_next(&(queue->queue)), struct cmd_obj, list);
		rtw_list_delete(&obj->list);
	}

	//_exit_critical_bh(&(queue->lock), &irqL);
	rtw_exit_critical(&queue->lock, &irqL);

_func_exit_;	

	return obj;
}

struct	cmd_obj	*rtw_dequeue_cmd(struct cmd_priv *pcmdpriv)
{
	struct cmd_obj *cmd_obj;
	
_func_enter_;		

	cmd_obj = _rtw_dequeue_cmd(&pcmdpriv->cmd_queue);
		
_func_exit_;			
	return cmd_obj;
}

s32 rtw_cmd_handler(PADAPTER padapter)
{
	s32 ret;
	struct cmd_obj *pcmd;
	u8 *pcmdbuf;
	u8 (*cmd_hdl)(_adapter *padapter, u8* pbuf);
	void (*pcmd_callback)(_adapter *dev, struct cmd_obj *pcmd);
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

_func_enter_;
	ret = rtw_down_sema(&pcmdpriv->cmd_sema);
	if(ret == _FAIL)
		return _FAIL;

	pcmdbuf = pcmdpriv->cmd_buf;
#ifdef CMD_RSP_BUF
	prspbuf = pcmdpriv->rsp_buf;
#endif

	if ((padapter->bDriverStopped == _TRUE)||(padapter->bSurpriseRemoved == _TRUE))
	{
		//DBG_871X("%s: DriverStopped(%d) SurpriseRemoved(%d) break at line %d\n",
		//	__FUNCTION__, padapter->bDriverStopped, padapter->bSurpriseRemoved, __LINE__);
		return _FAIL;
	}

_next:
	if ((padapter->bDriverStopped == _TRUE)||(padapter->bSurpriseRemoved == _TRUE))
	{
		//DBG_871X("%s: DriverStopped(%d) SurpriseRemoved(%d) break at line %d\n",
		//	__FUNCTION__, padapter->bDriverStopped, padapter->bSurpriseRemoved, __LINE__);
		return _FAIL;
	}
	
	pcmd = rtw_dequeue_cmd(pcmdpriv);
	if(!pcmd) {
		goto exit;
	}

	//DBG_871X("pcmd->cmdcode=%d\n", pcmd->cmdcode);
	if( _FAIL == rtw_cmd_filter(pcmdpriv, pcmd) )
	{
		pcmd->res = H2C_DROPPED;
		goto post_process;
	}

	pcmd->cmdsz = _RND4((pcmd->cmdsz));//_RND4

	rtw_memcpy(pcmdbuf, pcmd->parmbuf, pcmd->cmdsz);

	if(pcmd->cmdcode <= (sizeof(wlancmds) /sizeof(struct cmd_hdl)))
	{
		cmd_hdl = wlancmds[pcmd->cmdcode].h2cfuns;

		RT_TRACE(_module_rtl871x_cmd_c_,_drv_info_,("mlme_cmd_hdl(): cmd_hdl=%p, cmdcode=%d\n", (void *)(cmd_hdl), pcmd->cmdcode));	//cmd_hdl debug - Alex Fang

		if (cmd_hdl)
		{
			ret = cmd_hdl(pcmd->padapter, pcmdbuf);
			pcmd->res = ret;
		}
	}
	else
	{
		pcmd->res = H2C_PARAMETERS_ERROR;
	}

	cmd_hdl = NULL;

post_process:

	//call callback function for post-processed
	if(pcmd->cmdcode <= (sizeof(rtw_cmd_callback) /sizeof(struct _cmd_callback)))
	{
		pcmd_callback = rtw_cmd_callback[pcmd->cmdcode].callback;

		RT_TRACE(_module_rtl871x_cmd_c_,_drv_info_,("mlme_cmd_hdl(): pcmd_callback=%p, cmdcode=%d\n", (void *)(pcmd_callback), pcmd->cmdcode));	//cmd_callback debug - Alex Fang

		if(pcmd_callback == NULL)
		{
//TODO
//				RT_TRACE(_module_rtl871x_cmd_c_,_drv_info_,("mlme_cmd_hdl(): pcmd_callback=0x%p, cmdcode=%d\n", pcmd_callback, pcmd->cmdcode));
			rtw_free_cmd_obj(pcmd);
		}
		else
		{
			//todo: !!! fill rsp_buf to pcmd->rsp if (pcmd->rsp!=NULL)
			pcmd_callback(pcmd->padapter, pcmd);//need conider that free cmd_obj in rtw_cmd_callback
		}
	}
	else
	{
		RT_TRACE(_module_rtl871x_cmd_c_,_drv_err_,("%s: cmdcode=0x%x callback not defined!\n", __FUNCTION__, pcmd->cmdcode));
		rtw_free_cmd_obj(pcmd);
	}

	goto _next;
	
exit:
_func_exit_;
	return _SUCCESS;
}

thread_return rtw_cmd_thread(thread_context context)
{
	s32 err;
	PADAPTER padapter;
	struct cmd_priv *pcmdpriv;
	struct cmd_obj *pcmd;
	
	padapter = (PADAPTER)context;
	pcmdpriv = &padapter->cmdpriv;
	rtw_thread_enter("RTW_CMD_THREAD");
	DBG_871X("start %s\n", __FUNCTION__);

	do{
#ifdef PLATFORM_LINUX
		//if (kthread_should_stop())
        if (kthread_should_stop(pcmdpriv->cmdThread))  // Irene Lin
            break;
#endif
		err = rtw_cmd_handler(padapter);
		//rtw_flush_signals_thread();
	}while((_SUCCESS == err)/*&&(pcmdpriv->cmdThread)*/);

	// free all cmd_obj resources
	do{
		pcmd = rtw_dequeue_cmd(pcmdpriv);
		if(pcmd==NULL)
			break;

		//DBG_871X("%s: leaving... drop cmdcode:%u\n", __FUNCTION__, pcmd->cmdcode);

		rtw_free_cmd_obj(pcmd);	
	}while(1);
	
	rtw_up_sema(&(pcmdpriv->CmdTerminateSema));
	
	RT_TRACE(_module_hal_xmit_c_, _drv_notice_, ("-%s\n", __FUNCTION__));
	DBG_871X("exit %s\n", __FUNCTION__);	
	
	rtw_thread_exit();	
}

u8 rtw_disassoc_cmd(_adapter*padapter) /* for sta_mode */
{
	u8 ret = _SUCCESS;

	struct xmit_buf *pxmitbuf;
	PTXDESC_8195A ptxdesc;

	char *pcmd = "ATWD";
_func_enter_;
	pxmitbuf = rtw_alloc_xmitbuf(padapter);
	if(!pxmitbuf)
	{
		DBG_871X("%s(): pxmitbuf allocated failed!\n", __FUNCTION__);
		ret = _FAIL;
		goto exit;
	}
	
	pxmitbuf->pkt_len = strlen(pcmd) + SIZE_TX_DESC_8195a;
	ptxdesc = (PTXDESC_8195A)pxmitbuf->pbuf;
	ptxdesc->txpktsize = strlen(pcmd);
	ptxdesc->offset = SIZE_TX_DESC_8195a;
	ptxdesc->type = TX_H2C_CMD;//indicate transmittion of H2C packet
	ptxdesc->bus_agg_num = 1;//to do
	rtw_memcpy((pxmitbuf->pbuf + SIZE_TX_DESC_8195a), pcmd, strlen(pcmd));
	
	if(rtw_hal_mgnt_xmit(padapter, pxmitbuf) == _FALSE){
		ret = _FAIL;
		goto exit;
	}

exit:
_func_exit_;
	return ret;
}

u8 rtw_ATW0_cmd(_adapter*padapter) /* for sta_mode */
{
	u8 ret = _SUCCESS;

	struct xmit_buf *pxmitbuf;
	PTXDESC_8195A ptxdesc;

	char *pcmd = "ATW0=rtk";
_func_enter_;
	pxmitbuf = rtw_alloc_xmitbuf(padapter);
	if(!pxmitbuf)
	{
		DBG_871X("%s(): pxmitbuf allocated failed!\n", __FUNCTION__);
		ret = _FAIL;
		goto exit;
	}
	
	pxmitbuf->pkt_len = strlen(pcmd) + SIZE_TX_DESC_8195a;
	ptxdesc = (PTXDESC_8195A)pxmitbuf->pbuf;
	ptxdesc->txpktsize = strlen(pcmd);
	ptxdesc->offset = SIZE_TX_DESC_8195a;
	ptxdesc->type = TX_H2C_CMD;//indicate transmittion of H2C packet
	ptxdesc->bus_agg_num = 1;//to do
	rtw_memcpy((pxmitbuf->pbuf + SIZE_TX_DESC_8195a), pcmd, strlen(pcmd));
	
	if(rtw_hal_mgnt_xmit(padapter, pxmitbuf) == _FALSE){
		ret = _FAIL;
		goto exit;
	}

exit:
_func_exit_;
	return ret;
}
u8 rtw_ATW1_cmd(_adapter*padapter) /* for sta_mode */
{
	u8 ret = _SUCCESS;

	struct xmit_buf *pxmitbuf;
	PTXDESC_8195A ptxdesc;

	char *pcmd = "ATW1=12345abc";

_func_enter_;
	pxmitbuf = rtw_alloc_xmitbuf(padapter);
	if(!pxmitbuf)
	{
		DBG_871X("%s(): pxmitbuf allocated failed!\n", __FUNCTION__);
		ret = _FAIL;
		goto exit;
	}
	
	pxmitbuf->pkt_len = strlen(pcmd) + SIZE_TX_DESC_8195a;
	ptxdesc = (PTXDESC_8195A)pxmitbuf->pbuf;
	ptxdesc->txpktsize = strlen(pcmd);
	ptxdesc->offset = SIZE_TX_DESC_8195a;
	ptxdesc->type = TX_H2C_CMD;//indicate transmittion of H2C packet
	ptxdesc->bus_agg_num = 1;//to do
	rtw_memcpy((pxmitbuf->pbuf + SIZE_TX_DESC_8195a), pcmd, strlen(pcmd));
	
	if(rtw_hal_mgnt_xmit(padapter, pxmitbuf) == _FALSE){
		ret = _FAIL;
		goto exit;
	}

exit:
_func_exit_;
	return ret;
}

u8 rtw_assoc_cmd(_adapter*padapter) /* for sta_mode */
{
	u8 ret = _SUCCESS;

	struct xmit_buf *pxmitbuf;
	PTXDESC_8195A ptxdesc;

	char *pcmd = "ATWC";
	
_func_enter_;
	ret = rtw_ATW0_cmd(padapter);
	if(ret == _FAIL)
	{
		DBG_871X("%s(): set ssid failed!\n", __FUNCTION__);
		goto exit;
	}
	ret = rtw_ATW1_cmd(padapter);
	if(ret == _FAIL)
	{
		DBG_871X("%s(): set password failed!\n", __FUNCTION__);
		goto exit;
	}
	pxmitbuf = rtw_alloc_xmitbuf(padapter);
	if(!pxmitbuf)
	{
		DBG_871X("%s(): pxmitbuf allocated failed!\n", __FUNCTION__);
		ret = _FAIL;
		goto exit;
	}
	pxmitbuf->pkt_len = strlen(pcmd) + SIZE_TX_DESC_8195a;
	ptxdesc = (PTXDESC_8195A)pxmitbuf->pbuf;
	ptxdesc->txpktsize = strlen(pcmd);
	ptxdesc->offset = SIZE_TX_DESC_8195a;
	ptxdesc->type = TX_H2C_CMD;//indicate transmittion of H2C packet
	ptxdesc->bus_agg_num = 1;//to do
	rtw_memcpy((pxmitbuf->pbuf + SIZE_TX_DESC_8195a), pcmd, strlen(pcmd));
	if(rtw_hal_mgnt_xmit(padapter, pxmitbuf) == _FALSE){
		ret = _FAIL;
		goto exit;
	}

exit:
_func_exit_;
	return ret;
}

u8 rtw_scan_cmd(_adapter*padapter)
{
	u8 ret = _SUCCESS;

	struct xmit_buf *pxmitbuf;
	PTXDESC_8195A ptxdesc;

	char *pcmd = "ATWS";
_func_enter_;
	pxmitbuf = rtw_alloc_xmitbuf(padapter);
	if(!pxmitbuf)
	{
		DBG_871X("%s(): pxmitbuf allocated failed!\n", __FUNCTION__);
		ret = _FAIL;
		goto exit;
	}
	
	pxmitbuf->pkt_len = strlen(pcmd) + SIZE_TX_DESC_8195a;
	ptxdesc = (PTXDESC_8195A)pxmitbuf->pbuf;
	ptxdesc->txpktsize = strlen(pcmd);
	ptxdesc->offset = SIZE_TX_DESC_8195a;
	ptxdesc->type = TX_H2C_CMD;//indicate transmittion of H2C packet
	ptxdesc->bus_agg_num = 1;//to do
	rtw_memcpy((pxmitbuf->pbuf + SIZE_TX_DESC_8195a), pcmd, strlen(pcmd));
	
	if(rtw_hal_mgnt_xmit(padapter, pxmitbuf) == _FALSE){
		ret = _FAIL;
		goto exit;
	}

exit:
_func_exit_;
	return ret;
}

#if 0
u8 rtw_ps_cmd(_adapter*padapter)
{
	u8 ret = _SUCCESS;

	struct xmit_buf *pxmitbuf;
	PTXDESC_8195A ptxdesc;

	char *pcmd = "ATWP";
	DBG_871X("%s(): pxmitbuf allocated failed!\n", __FUNCTION__);
_func_enter_;
	pxmitbuf = rtw_alloc_xmitbuf(padapter);
	if(!pxmitbuf)
	{
		DBG_871X("%s(): pxmitbuf allocated failed!\n", __FUNCTION__);
		ret = _FAIL;
		goto exit;
	}
	
	pxmitbuf->pkt_len = strlen(pcmd) + SIZE_TX_DESC_8195a;
	ptxdesc = (PTXDESC_8195A)pxmitbuf->pbuf;
	ptxdesc->txpktsize = strlen(pcmd);
	ptxdesc->offset = SIZE_TX_DESC_8195a;
	ptxdesc->type = TX_H2C_CMD;//indicate transmittion of H2C packet
	ptxdesc->bus_agg_num = 1;//to do
	rtw_memcpy((pxmitbuf->pbuf + SIZE_TX_DESC_8195a), pcmd, strlen(pcmd));
	
	if(rtw_hal_mgnt_xmit(padapter, pxmitbuf) == _FALSE){
		ret = _FAIL;
		goto exit;
	}

exit:
_func_exit_;
	return ret;
}
#endif

void dynamic_chk_wk_hdl(_adapter *padapter, u8 *pbuf, int sz)
{
	
	padapter = (_adapter *)pbuf;

	// ToDo: traffic_status_watchdog() will idle 2sec in throughput test. 2014/11/25 mask by Cloud
	if (padapter) {
	#ifdef CONFIG_POWER_SAVING
		traffic_status_watchdog(padapter);
	#endif
	}

}

u8 rtw_drvextra_cmd_hdl(_adapter *padapter, unsigned char *pbuf)
{
	struct drvextra_cmd_parm *pdrvextra_cmd;

	if(!pbuf)
		return H2C_PARAMETERS_ERROR;

	pdrvextra_cmd = (struct drvextra_cmd_parm*)pbuf;
	
	//DBG_871X("pdrvextra_cmd->ec_id=%d\n", pdrvextra_cmd->ec_id);
	switch(pdrvextra_cmd->ec_id)
	{
		case DYNAMIC_CHK_WK_CID:
			dynamic_chk_wk_hdl(padapter, pdrvextra_cmd->pbuf, pdrvextra_cmd->type_size);
			break;

		default:
			break;
	}

	if (pdrvextra_cmd->pbuf && pdrvextra_cmd->type_size>0)
	{
		rtw_mfree(pdrvextra_cmd->pbuf, pdrvextra_cmd->type_size);
	}

	return H2C_SUCCESS;
}

#ifdef CONFIG_PS_DYNAMIC_CHK
u8 rtw_dynamic_chk_wk_cmd(_adapter*padapter)
{
	struct cmd_obj*		ph2c;
	struct drvextra_cmd_parm  *pdrvextra_cmd_parm;	
	struct cmd_priv	*pcmdpriv=&padapter->cmdpriv;
	u8	res=_SUCCESS;
	
_func_enter_;	
	ph2c = (struct cmd_obj*)rtw_zmalloc(sizeof(struct cmd_obj));	
	if(ph2c==NULL){
		res= _FAIL;
		goto exit;
	}
	
	pdrvextra_cmd_parm = (struct drvextra_cmd_parm*)rtw_zmalloc(sizeof(struct drvextra_cmd_parm)); 
	if(pdrvextra_cmd_parm==NULL){
		rtw_mfree((unsigned char *)ph2c, sizeof(struct cmd_obj));
		res= _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = DYNAMIC_CHK_WK_CID;
	pdrvextra_cmd_parm->type_size = 0;
	pdrvextra_cmd_parm->pbuf = (u8 *)padapter;

	init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));

	
	//rtw_enqueue_cmd(pcmdpriv, ph2c);	
	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
	
exit:
	
_func_exit_;

	return res;

}
#endif

#ifdef CONFIG_POWER_SAVING
u8 rtw_get_bBusyTraffic(_adapter *padapter){
	if(padapter == NULL)
		return _FALSE;	
	
	return padapter->LinkDetectInfo.bBusyTraffic;
}
static void collect_traffic_statistics(_adapter *padapter)
{
	struct dvobj_priv	*pdvobjpriv = adapter_to_dvobj(padapter);

	// Tx
	pdvobjpriv->traffic_stat.tx_bytes = padapter->xmitpriv.tx_bytes;
	pdvobjpriv->traffic_stat.tx_pkts = padapter->xmitpriv.tx_pkts;
	pdvobjpriv->traffic_stat.tx_drop = padapter->xmitpriv.tx_drop;

	// Rx
	pdvobjpriv->traffic_stat.rx_bytes = padapter->recvpriv.rx_bytes;
	pdvobjpriv->traffic_stat.rx_pkts = padapter->recvpriv.rx_pkts;
	pdvobjpriv->traffic_stat.rx_drop = padapter->recvpriv.rx_drop;

	// Calculate throughput in last interval
	pdvobjpriv->traffic_stat.cur_tx_bytes = pdvobjpriv->traffic_stat.tx_bytes - pdvobjpriv->traffic_stat.last_tx_bytes;
	pdvobjpriv->traffic_stat.cur_rx_bytes = pdvobjpriv->traffic_stat.rx_bytes - pdvobjpriv->traffic_stat.last_rx_bytes;
	pdvobjpriv->traffic_stat.last_tx_bytes = pdvobjpriv->traffic_stat.tx_bytes;
	pdvobjpriv->traffic_stat.last_rx_bytes = pdvobjpriv->traffic_stat.rx_bytes;

	pdvobjpriv->traffic_stat.cur_tx_tp = (u32)(pdvobjpriv->traffic_stat.cur_tx_bytes *8/2/1024/1024);
	pdvobjpriv->traffic_stat.cur_rx_tp = (u32)(pdvobjpriv->traffic_stat.cur_rx_bytes *8/2/1024/1024);
}


u8 traffic_status_watchdog(_adapter *padapter)
{
	u8	bEnterPS = _FALSE;
	u16	BusyThreshold = 100;
	u8	bBusyTraffic = _FALSE, bTxBusyTraffic = _FALSE, bRxBusyTraffic = _FALSE;
	u8	bHigherBusyTraffic = _FALSE, bHigherBusyRxTraffic = _FALSE, bHigherBusyTxTraffic = _FALSE;

//	RT_LINK_DETECT_T * link_detect = &padapter->LinkDetectInfo;

#ifdef DBG_POWER_SAVING
	DBG_871X("%s\n", __func__);
#endif

	collect_traffic_statistics(padapter);

	//
	// Determine if our traffic is busy now
	//
//	if(check_fwstate(padapter, _FW_LINKED) == _TRUE)
	{
		// if we raise bBusyTraffic in last watchdog, using lower threshold.
		if (padapter->LinkDetectInfo.bBusyTraffic)
			BusyThreshold = 75;
		if( padapter->LinkDetectInfo.NumRxOkInPeriod > BusyThreshold ||
			padapter->LinkDetectInfo.NumTxOkInPeriod > BusyThreshold )
		{
			bBusyTraffic = _TRUE;

			if (padapter->LinkDetectInfo.NumRxOkInPeriod > padapter->LinkDetectInfo.NumTxOkInPeriod)
				bRxBusyTraffic = _TRUE;
			else
				bTxBusyTraffic = _TRUE;
		}

		// Higher Tx/Rx data.
		if( padapter->LinkDetectInfo.NumRxOkInPeriod > 4000 ||
			padapter->LinkDetectInfo.NumTxOkInPeriod > 4000 )
		{
			bHigherBusyTraffic = _TRUE;

			if (padapter->LinkDetectInfo.NumRxOkInPeriod > padapter->LinkDetectInfo.NumTxOkInPeriod)
				bHigherBusyRxTraffic = _TRUE;
			else
				bHigherBusyTxTraffic = _TRUE;
		}

		// check traffic for  powersaving.
		if( ((padapter->LinkDetectInfo.NumRxUnicastOkInPeriod + padapter->LinkDetectInfo.NumTxOkInPeriod) > 8 ) ||
			(padapter->LinkDetectInfo.NumRxUnicastOkInPeriod > 2) )
		{
#ifdef DBG_POWER_SAVING
			DBG_871X("(-)Tx = %d, Rx = %d \n",padapter->LinkDetectInfo.NumTxOkInPeriod,padapter->LinkDetectInfo.NumRxUnicastOkInPeriod);
#endif
			bEnterPS= _FALSE;
		}
		else
		{
#ifdef DBG_POWER_SAVING
			DBG_871X("(+)Tx = %d, Rx = %d \n",padapter->LinkDetectInfo.NumTxOkInPeriod,padapter->LinkDetectInfo.NumRxUnicastOkInPeriod);
#endif
			bEnterPS= _TRUE;
		}

		// LeisurePS only work in infra mode.
		if(bEnterPS)
		{
			LPS_Enter(padapter);	
		}
		else
		{
			LPS_Leave(padapter);
		}
	}
#if 0
	else
	{
		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
		int n_assoc_iface = 0;
		int i;

		if (check_fwstate(dvobj->if1, WIFI_ASOC_STATE))
			n_assoc_iface++;

		if(!from_timer && n_assoc_iface == 0)
			LPS_Leave(padapter);
	}
#endif
	padapter->LinkDetectInfo.NumRxOkInPeriod = 0;
	padapter->LinkDetectInfo.NumTxOkInPeriod = 0;
	padapter->LinkDetectInfo.NumRxUnicastOkInPeriod = 0;
	padapter->LinkDetectInfo.bBusyTraffic = bBusyTraffic;
	padapter->LinkDetectInfo.bTxBusyTraffic = bTxBusyTraffic;
	padapter->LinkDetectInfo.bRxBusyTraffic = bRxBusyTraffic;
	padapter->LinkDetectInfo.bHigherBusyTraffic = bHigherBusyTraffic;
	padapter->LinkDetectInfo.bHigherBusyRxTraffic = bHigherBusyRxTraffic;
	padapter->LinkDetectInfo.bHigherBusyTxTraffic = bHigherBusyTxTraffic;

	return bEnterPS;
	
}
#endif

#ifndef __RTW_CMD_H__
#define __RTW_CMD_H__
#include "drv_types.h"

#define MAX_CMDSZ	(1024)
#define MAX_RSPSZ	(512)

#if defined(PLATFORM_ECOS) || defined(PLATFORM_FREERTOS)
	#define CMDBUFF_ALIGN_SZ 4
#else
	#define CMDBUFF_ALIGN_SZ 512
#endif

struct cmd_obj {
	_adapter *padapter;
	u16	cmdcode;
	u8	res;
	u8	*parmbuf;
	u32	cmdsz;
	u8	*rsp;
	u32	rspsz;
	//_sema 	cmd_sem;
	_list	list;
};
struct cmd_priv{
	_sema cmd_sema;
	_sema CmdTerminateSema;
	_queue cmd_queue;
	u8 *cmd_allocated_buf;
	u8 *cmd_buf;
#ifdef CMD_RSP_BUF
	u8	*rsp_buf;	//shall be non-paged, and 4 bytes aligned		
	u8	*rsp_allocated_buf;
	u32	rsp_cnt;
#endif
#if defined(PLATFORM_FREERTOS)
	struct task_struct	cmdThread;
#else
	_thread_hdl_ cmdThread;
#endif

	_adapter *padapter;
	_mutex sctx_mutex;
};

// CMD param Formart for driver extra cmd handler
struct drvextra_cmd_parm {
	int ec_id; //extra cmd id
	int type_size; // Can use this field as the type id or command size
	unsigned char *pbuf;
};

enum rtw_drvextra_cmd_id
{	
	NONE_WK_CID,
	DYNAMIC_CHK_WK_CID,
	DM_CTRL_WK_CID,
	PBC_POLLING_WK_CID,
	POWER_SAVING_CTRL_WK_CID,//IPS,AUTOSuspend
	LPS_CTRL_WK_CID,
	ANT_SELECT_WK_CID,
	P2P_PS_WK_CID,
	P2P_PROTO_WK_CID,
	CHECK_HIQ_WK_CID,//for softap mode, check hi queue if empty
	INTEl_WIDI_WK_CID,
	C2H_WK_CID,
	RTP_TIMER_CFG_WK_CID,
	RESET_SECURITYPRIV, // add for CONFIG_IEEE80211W, none 11w also can use
	FREE_ASSOC_RESOURCES, // add for CONFIG_IEEE80211W, none 11w also can use
	DM_IN_LPS_WK_CID,
	DM_RA_MSK_WK_CID, //add for STA update RAMask when bandwith change.
	BEAMFORMING_WK_CID,
	LPS_CHANGE_DTIM_CID,
	MAX_WK_CID
};

#define H2C_SUCCESS			0x00
#define H2C_SUCCESS_RSP			0x01
#define H2C_DUPLICATED			0x02
#define H2C_DROPPED			0x03
#define H2C_PARAMETERS_ERROR		0x04
#define H2C_REJECTED			0x05
#define H2C_CMD_OVERFLOW		0x06
#define H2C_RESERVED			0x07

#define init_h2fwcmd_w_parm_no_rsp(pcmd, pparm, code) \
do {\
	rtw_init_listhead(&pcmd->list);\
	pcmd->cmdcode = code;\
	pcmd->parmbuf = (u8 *)(pparm);\
	pcmd->cmdsz = sizeof (*pparm);\
	pcmd->rsp = NULL;\
	pcmd->rspsz = 0;\
} while(0)

#define init_h2fwcmd_w_parm_no_parm_rsp(pcmd, code) \
do {\
	rtw_init_listhead(&pcmd->list);\
	pcmd->cmdcode = code;\
	pcmd->parmbuf = NULL;\
	pcmd->cmdsz = 0;\
	pcmd->rsp = NULL;\
	pcmd->rspsz = 0;\
} while(0)

#define GEN_CMD_CODE(cmd)	cmd ## _CMD_

struct _cmd_callback {
	u32	cmd_code;
	void (*callback)(_adapter  *padapter, struct cmd_obj *cmd);
};

enum rtw_h2c_cmd
{
	GEN_CMD_CODE(_Set_Drv_Extra), /*0*/	
	MAX_H2CCMD
};

#ifdef _RTW_CMD_C_
const struct _cmd_callback 	rtw_cmd_callback[] = 
{
	{GEN_CMD_CODE(_Set_Drv_Extra), NULL},/*0*/
};
#endif

u8 NULL_hdl(_adapter *padapter, u8 *pbuf);
u8 rtw_drvextra_cmd_hdl(_adapter *padapter, unsigned char *pbuf);

#define GEN_DRV_CMD_HANDLER(size, cmd)	{size, &cmd ## _hdl},
#define GEN_MLME_EXT_HANDLER(size, cmd)	{size, cmd},

struct cmd_hdl {
	uint	parmsize;
	u8 (*h2cfuns)(struct _ADAPTER *padapter, u8 *pbuf);	
};

#ifdef _RTW_CMD_C_
const struct cmd_hdl wlancmds[] = 
{
	GEN_MLME_EXT_HANDLER(0, rtw_drvextra_cmd_hdl) /*0*/
};
#endif

#define MacAddr_isBcst(addr) \
( \
	( (addr[0] == 0xff) && (addr[1] == 0xff) && \
		(addr[2] == 0xff) && (addr[3] == 0xff) && \
		(addr[4] == 0xff) && (addr[5] == 0xff) )  ? _TRUE : _FALSE \
)

__inline static int IS_MCAST(unsigned char *da)
{
	if ((*da) & 0x01)
		return _TRUE;
	else
		return _FALSE;
}

s32 rtw_init_cmd_priv (PADAPTER padapter);
void rtw_free_cmd_priv (PADAPTER padapter);
void rtw_free_cmd_obj(struct cmd_obj *pcmd);
int rtw_cmd_filter(struct cmd_priv *pcmdpriv, struct cmd_obj *cmd_obj);
u32 rtw_enqueue_cmd(struct cmd_priv *pcmdpriv, struct cmd_obj *cmd_obj);
struct cmd_obj *rtw_dequeue_cmd(struct cmd_priv *pcmdpriv);
thread_return rtw_cmd_thread(thread_context context);
u8 rtw_disassoc_cmd(_adapter*padapter);/* for sta_mode */
u8 traffic_status_watchdog(_adapter *padapter);
u8 rtw_get_bBusyTraffic(_adapter *padapter);
#endif

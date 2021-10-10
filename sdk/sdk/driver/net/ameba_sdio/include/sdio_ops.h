#ifndef __SDIO_IO_H__
#define __SDIO_IO_H__
/***************************************************/
#include <basic_types.h>
#include <drv_types.h>
#include <sdio_drvio.h>
#ifdef PLATFORM_LINUX
#include <sdio_ops_linux.h>
#endif
#ifdef PLATFORM_ECOS
#include <ecos/sdio_ops_ecos.h>
#endif

typedef struct _SDIO_BUS_OPS{
// 0
	int (*bus_probe)(void);
	int (*bus_remove)(void);
	int (*enable_func)(struct sdio_func*func);	/*enables a SDIO function for usage*/
	int (*disable_func)(struct sdio_func *func); 
	int (*reg_driver)(struct sdio_driver*); /*register sdio function device driver callback*/
	void (*unreg_driver)(struct sdio_driver *); 
// 6
	int (*claim_irq)(struct sdio_func *func, void(*handler)(struct sdio_func *));
	int (*release_irq)(struct sdio_func*func);
	void (*claim_host)(struct sdio_func*func);	/*exclusively claim a bus for a certain SDIO function*/
	void (*release_host)(struct sdio_func *func); 
// 10
	unsigned char (*readb)(struct sdio_func *func, unsigned int addr, int *err_ret);/*read a single byte from a SDIO function*/
	unsigned short (*readw)(struct sdio_func *func, unsigned int addr, int *err_ret);	/*read a 16 bit integer from a SDIO function*/
	unsigned int (*readl)(struct sdio_func *func, unsigned int addr, int *err_ret); /*read a 32 bit integer from a SDIO function*/
// 13
	void (*writeb)(struct sdio_func *func, unsigned char b,unsigned int addr, int *err_ret);	/*write a single byte to a SDIO function*/
	void (*writew)(struct sdio_func *func, unsigned short b,unsigned int addr, int *err_ret);	/*write a 16 bit integer to a SDIO function*/
	void (*writel)(struct sdio_func *func, unsigned int b,unsigned int addr, int *err_ret); /*write a 32 bit integer to a SDIO function*/
//16
	int (*memcpy_fromio)(struct sdio_func *func, void *dst,unsigned int addr, int count);/*read a chunk of memory from a SDIO functio*/
	int (*memcpy_toio)(struct sdio_func *func, unsigned int addr,void *src, int count);  /*write a chunk of memory to a SDIO function*/ 
	unsigned char (*f0_readb)(struct sdio_func *func,	unsigned int addr, int *err_ret);
	void (*f0_writeb)(struct sdio_func *func, unsigned char b,unsigned int addr, int *err_ret);
}SDIO_BUS_OPS;
extern const SDIO_BUS_OPS rtw_sdio_bus_ops;

u8 sdio_read8(PADAPTER padapter, u32 addr);
u16 sdio_read16(PADAPTER padapter, u32 addr);
u32 sdio_read32(PADAPTER padapter, u32 addr);
void sdio_read_mem(PADAPTER padapter, u32 addr, u32 cnt, u8 *rmem);
u32 sdio_read_port(PADAPTER padapter, u32 addr, u32 cnt, u8 * mem);
u8 sdio_f0_read8(PADAPTER padapter, u32 addr);
s32 sdio_write8(PADAPTER padapter, u32 addr, u8 val);
s32 sdio_write16(PADAPTER padapter, u32 addr, u16 val);
s32 sdio_write32(PADAPTER padapter, u32 addr, u32 val);
s32 sdio_writeN(PADAPTER padapter, u32 addr, u32 cnt, u8* pbuf);
void sdio_write_mem(PADAPTER padapter, u32 addr, u32 cnt, u8 *wmem);
u32 sdio_write_port(PADAPTER padapter, u32 addr, u32 cnt, u8 *mem);
s32 _sdio_local_read(PADAPTER padapter, u32 addr, u32 cnt, u8 *pbuf);
s32 sdio_local_read(PADAPTER padapter, u32 addr, u32 cnt, u8 *pbuf);
s32 _sdio_local_write(PADAPTER padapter, u32 addr, u32 cnt, u8 *pbuf);
s32 sdio_local_write(PADAPTER padapter, u32 addr, u32 cnt, u8 *pbuf);
extern u8 SdioLocalCmd52Read1Byte(PADAPTER padapter, u32 addr);
extern u16 SdioLocalCmd52Read2Byte(PADAPTER padapter, u32 addr);
extern u32 SdioLocalCmd52Read4Byte(PADAPTER padapter, u32 addr);
extern u32 SdioLocalCmd53Read4Byte(PADAPTER padapter, u32 addr);
extern void SdioLocalCmd52Write1Byte(PADAPTER padapter, u32 addr, u8 v);
extern void SdioLocalCmd52Write2Byte(PADAPTER padapter, u32 addr, u16 v);
extern void SdioLocalCmd52Write4Byte(PADAPTER padapter, u32 addr, u32 v);
void sd_int_hdl(PADAPTER padapter);
u8 HalGetTxBufUnitSize8195ASdio(PADAPTER padapter);
u8 HalQueryTxBufferStatus8195ASdio(PADAPTER padapter);
void InitInterrupt8195ASdio(PADAPTER padapter);
void EnableInterrupt8195ASdio(PADAPTER padapter);
void DisableInterrupt8195ASdio(PADAPTER padapter);
#ifdef CONFIG_POWER_SAVING
u32 rtl8195a_rpwm_notify(_adapter *padapter, u8 rpwm_event);
#ifdef CONFIG_WOWLAN
u32	rtl8195a_send_wowlan_cmd(_adapter *padapter, u8 id, u8 *data, u16 len);
#endif
#endif
/**************************************************/
#endif

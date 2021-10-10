#ifndef _RTW_IO_H_
#define _RTW_IO_H_

struct _io_ops;
#include <drv_types.h>

struct _io_ops
{
	int (*init_io_priv)(PADAPTER padapter);
	int (*write8_endian)(PADAPTER padapter, u32 addr, u32 buf, u32 big);
	
	u8 (*_read8)(PADAPTER padapter, u32 addr);
	u16 (*_read16)(PADAPTER padapter, u32 addr);
	u32 (*_read32)(PADAPTER padapter, u32 addr);

	int (*_write8)(PADAPTER padapter, u32 addr, u8 val);
	int (*_write16)(PADAPTER padapter, u32 addr, u16 val);
	int (*_write32)(PADAPTER padapter, u32 addr, u32 val);
	int (*_writeN)(PADAPTER padapter, u32 addr, u32 length, u8 *pdata);
#ifdef CONFIG_USB_HCI
	int (*_readN)(PADAPTER padapter, u32 addr, u32 length, u8 *pdata);
#endif
	void (*_read_mem)(PADAPTER padapter, u32 addr, u32 cnt, u8 *pmem);
	void (*_write_mem)(PADAPTER padapter, u32 addr, u32 cnt, u8 *pmem);
#ifdef CONFIG_GSPI_HCI
	int(*read_rx_fifo)(PADAPTER pAdapter, u32 addr, u8 *buf, u32 len, struct spi_more_data *pmore_data);
	int (*write_tx_fifo)(PADAPTER padapter, u32 addr, u8 *buf, u32 len);	
#endif
	u32 (*_read_port)(PADAPTER padapter, u32 addr, u32 cnt, u8 *pmem);
	u32 (*_write_port)(PADAPTER padapter, u32 addr, u32 cnt, u8 *pmem);
	
	void (*_read_port_cancel)(PADAPTER padapter);
	void (*_write_port_cancel)(PADAPTER padapter);

#ifdef CONFIG_SDIO_HCI
	u8 (*_sd_f0_read8)(PADAPTER padapter, u32 addr);
#endif
};

#define SD_IO_TRY_CNT (8)
#define MAX_CONTINUAL_IO_ERR SD_IO_TRY_CNT

int rtw_inc_and_chk_continual_io_error(struct dvobj_priv *dvobj);
void rtw_reset_continual_io_error(struct dvobj_priv *dvobj);

u8 _rtw_read8(_adapter *adapter, u32 addr);
u16 _rtw_read16(_adapter *adapter, u32 addr);
u32 _rtw_read32(_adapter *adapter, u32 addr);
void _rtw_read_mem(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
u32 _rtw_read_port(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
void _rtw_read_port_cancel(_adapter *adapter);

int _rtw_write8(_adapter *adapter, u32 addr, u8 val);
int _rtw_write16(_adapter *adapter, u32 addr, u16 val);
int _rtw_write32(_adapter *adapter, u32 addr, u32 val);
int _rtw_writeN(_adapter *adapter, u32 addr, u32 length, u8 *pdata);
int _rtw_readN(_adapter *adapter, u32 addr, u32 length, u8 *pdata);
u8 _rtw_sd_f0_read8(_adapter *adapter, u32 addr);

u32 _rtw_write_port(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem);
void _rtw_write_port_cancel(_adapter *adapter);

#define rtw_read8(adapter, addr) _rtw_read8((adapter), (addr))
#define rtw_read16(adapter, addr) _rtw_read16((adapter), (addr))
#define rtw_read32(adapter, addr) _rtw_read32((adapter), (addr))
#define rtw_read_mem(adapter, addr, cnt, mem) _rtw_read_mem((adapter), (addr), (cnt), (mem))
#define rtw_read_port(adapter, addr, cnt, mem) _rtw_read_port((adapter), (addr), (cnt), (mem))
#define rtw_read_port_cancel(adapter) _rtw_read_port_cancel((adapter))

#define  rtw_write8(adapter, addr, val) _rtw_write8((adapter), (addr), (val))
#define  rtw_write16(adapter, addr, val) _rtw_write16((adapter), (addr), (val))
#define  rtw_write32(adapter, addr, val) _rtw_write32((adapter), (addr), (val))
#define  rtw_writeN(adapter, addr, length, data) _rtw_writeN((adapter), (addr), (length), (data))
#ifdef CONFIG_USB_HCI
#define  rtw_readN(adapter, addr, length, data) _rtw_readN((adapter), (addr), (length), (data))
#endif

#define rtw_write_port(adapter, addr, cnt, mem) _rtw_write_port((adapter), (addr), (cnt), (mem))
#define rtw_write_port_cancel(adapter) _rtw_write_port_cancel((adapter))

#ifdef CONFIG_GSPI_HCI
void rtw_set_chip_endian(PADAPTER padapter);
int rtw_get_chip_endian(PADAPTER padapter);
#endif

#endif  //_RTW_IO_H_
#define _RTW_IO_C_

#include "autoconf.h"
#include "rtw_debug.h"
#include "drv_types.h"
#include "osdep_service.h"
#include "rtw_io.h"
#ifdef PLATFORM_FREERTOS
#include <freertos/generic.h>
#endif //PLATFORM_FREERTOS
#ifdef CONFIG_GSPI_HCI
#include "8195_gspi_reg.h"
#endif  //CONFIG_GSPI_HCI

#if defined (PLATFORM_LINUX) && defined (PLATFORM_WINDOWS)
#error "Shall be Linux or Windows, but not both!\n"
#endif

#ifdef CONFIG_SDIO_HCI
#define rtw_le16_to_cpu(val) 		val
#define rtw_le32_to_cpu(val)		val
#define rtw_cpu_to_le16(val)		val
#define rtw_cpu_to_le32(val)		val
#else
#define rtw_le16_to_cpu(val) 		le16_to_cpu(val)
#define rtw_le32_to_cpu(val)		le32_to_cpu(val)
#define rtw_cpu_to_le16(val)		cpu_to_le16(val)
#define rtw_cpu_to_le32(val)		cpu_to_le32(val)
#endif


u8 _rtw_read8(_adapter *adapter, u32 addr)
{
	u8 r_val;
	u8 (*_read8)(_adapter *adapter, u32 addr);
	_func_enter_;
	_read8 = adapter->io_ops._read8;

	r_val = _read8(adapter, addr);
	_func_exit_;
	return r_val;
}

u16 _rtw_read16(_adapter *adapter, u32 addr)
{
	u16 r_val;
	u16 	(*_read16)(_adapter *adapter, u32 addr);
	_func_enter_;
	_read16 = adapter->io_ops._read16;

	r_val = _read16(adapter, addr);
	_func_exit_;
	return rtw_le16_to_cpu(r_val);
}

u32 _rtw_read32(_adapter *adapter, u32 addr)
{
	u32 r_val;
	u32 	(*_read32)(_adapter *adapter, u32 addr);
	_func_enter_;
	_read32 = adapter->io_ops._read32;

	r_val = _read32(adapter, addr);
	_func_exit_;
	return rtw_le32_to_cpu(r_val);

}

int _rtw_write8(_adapter *adapter, u32 addr, u8 val)
{
	int (*_write8)(_adapter *adapter, u32 addr, u8 val);
	int ret;
	_func_enter_;
	_write8 = adapter->io_ops._write8;

	ret = _write8(adapter, addr, val);
	_func_exit_;
	
	return RTW_STATUS_CODE(ret);
}
int _rtw_write16(_adapter *adapter, u32 addr, u16 val)
{
	int (*_write16)(_adapter *adapter, u32 addr, u16 val);
	int ret;
	_func_enter_;
	_write16 = adapter->io_ops._write16;

	val = rtw_cpu_to_le16(val);
	ret = _write16(adapter, addr, val);
	_func_exit_;

	return RTW_STATUS_CODE(ret);
}
int _rtw_write32(_adapter *adapter, u32 addr, u32 val)
{
	int (*_write32)(_adapter *adapter, u32 addr, u32 val);
	int ret;
	_func_enter_;
	_write32 = adapter->io_ops._write32;
	
	val = rtw_cpu_to_le32(val);
	ret = _write32(adapter, addr, val);
	_func_exit_;

	return RTW_STATUS_CODE(ret);
}

int _rtw_writeN(_adapter *adapter, u32 addr ,u32 length , u8 *pdata)
{
	int (*_writeN)(_adapter *adapter, u32 addr,u32 length, u8 *pdata);
	int ret;
	_func_enter_;
	_writeN = adapter->io_ops._writeN;

	ret = _writeN(adapter, addr,length,pdata);
	_func_exit_;

	return RTW_STATUS_CODE(ret);
}

#ifdef CONFIG_USB_HCI
int _rtw_readN(_adapter *adapter, u32 addr ,u32 length , u8 *pdata)
{
	int (*_readN)(_adapter *adapter, u32 addr,u32 length, u8 *pdata);
	int ret;
	_func_enter_;
	_readN = adapter->io_ops._readN;

	ret = _readN(adapter, addr,length,pdata);
	_func_exit_;

	return RTW_STATUS_CODE(ret);
}
#endif

#ifdef CONFIG_SDIO_HCI
u8 _rtw_sd_f0_read8(_adapter *adapter, u32 addr)
{
	u8 r_val = 0x00;
	u8 (*_sd_f0_read8)(_adapter *adapter, u32 addr);

	_func_enter_;
	_sd_f0_read8 = adapter->io_ops._sd_f0_read8;

	if (_sd_f0_read8)
		r_val = _sd_f0_read8(adapter, addr);
	else
		DBG_871X_LEVEL(_drv_warning_, FUNC_ADPT_FMT" _sd_f0_read8 callback is NULL\n", FUNC_ADPT_ARG(adapter));

	_func_exit_;
	return r_val;
}
#endif /* CONFIG_SDIO_HCI */
/*
* Increase and check if the continual_io_error of this @param dvobjprive is larger than MAX_CONTINUAL_IO_ERR
* @return _TRUE:
* @return _FALSE:
*/
int rtw_inc_and_chk_continual_io_error(struct dvobj_priv *dvobj)
{
	int ret = _FALSE;
	int value;
	if( (value=ATOMIC_INC_RETURN(&dvobj->continual_io_error)) > MAX_CONTINUAL_IO_ERR) {
		DBG_871X("[dvobj:%p][ERROR] continual_io_error:%d > %d\n", dvobj, value, MAX_CONTINUAL_IO_ERR);
		ret = _TRUE;
	} else {
		//DBG_871X("[dvobj:%p] continual_io_error:%d\n", dvobj, value);
	}
	return ret;
}

/*
* Set the continual_io_error of this @param dvobjprive to 0
*/
void rtw_reset_continual_io_error(struct dvobj_priv *dvobj)
{
	ATOMIC_SET(&dvobj->continual_io_error, 0);	
}

u32 _rtw_write_port(PADAPTER padapter, u32 addr, u32 cnt, u8 *pmem)
{
	u32 (*_write_port)(PADAPTER padapter, u32 addr, u32 cnt, u8 *pmem);

	u32 ret = _SUCCESS;

	_func_enter_;

	_write_port = padapter->io_ops._write_port;
	
	ret = _write_port(padapter, addr, cnt, pmem);

	 _func_exit_;

	return ret;
}

void _rtw_write_port_cancel(_adapter *adapter)
{
	void (*_write_port_cancel)(_adapter *adapter);
	
	_write_port_cancel = adapter->io_ops._write_port_cancel;

	if(_write_port_cancel)
		_write_port_cancel(adapter);
}


u32 _rtw_read_port(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem)
{
	u32 (*_read_port)(_adapter *adapter , u32 addr, u32 cnt, u8 *pmem);
	u32 ret = _SUCCESS;

	_func_enter_;

	if( (adapter->bDriverStopped ==_TRUE) || (adapter->bSurpriseRemoved == _TRUE))
	{
	     RT_TRACE(_module_rtl871x_io_c_, _drv_info_, ("rtw_read_port:bDriverStopped(%d) OR bSurpriseRemoved(%d)", adapter->bDriverStopped, adapter->bSurpriseRemoved));	    
	     return _FAIL;
	}

	_read_port = adapter->io_ops._read_port;

	ret = _read_port(adapter, addr, cnt, pmem);

	_func_exit_;
	return ret;
}
void _rtw_read_port_cancel(_adapter *adapter)
{
	void (*_read_port_cancel)(_adapter *adapter);

	_read_port_cancel = adapter->io_ops._read_port_cancel;

	if(_read_port_cancel)
		_read_port_cancel(adapter);
}

#ifdef CONFIG_GSPI_HCI
void rtw_set_chip_endian(PADAPTER padapter)
{
	
	DBG_871X("rtw_set_chip_endian!!\n");

	if(padapter->io_ops.write8_endian)
		padapter->io_ops.write8_endian(padapter, SPI_LOCAL_OFFSET | SPI_REG_SPI_CFG, 0x01, 1);
}

int rtw_get_chip_endian(PADAPTER padapter)
{
	u32	ret;

	ret = rtw_read32(padapter, SPI_LOCAL_OFFSET | SPI_REG_SPI_CFG);

	DBG_871X("rtw_get_chip_endian!! 0xf0 is 0x%08x\n", ret);

	return ret;
}
#endif


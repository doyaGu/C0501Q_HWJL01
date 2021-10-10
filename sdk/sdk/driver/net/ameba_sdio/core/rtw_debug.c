#define _RTW_DEBUG_C_

//#include <drv_types.h>   // Irene Lin: move to blow
#include <basic_types.h> // Irene Lin


u8 g_fwdl_chksum_fail = 0;
u8 g_fwdl_wintint_rdy_fail = 0;

#if 0 // Irene Lin
#define _drv_always_		1
#define _drv_emerg_		2
#define _drv_alert_		3
#define _drv_crit_		4
#define _drv_err_			5
#define	_drv_warning_	6
#define _drv_notice_		7
#define _drv_info_		8	
#define _drv_dump_		9
#define	_drv_debug_		10
#endif

#if 1  // Irene Lin: debug level
u32 GlobalDebugLevel = 10;
#else
u32 GlobalDebugLevel = _drv_debug_;
#endif

#include <drv_types.h>   // Irene Lin: move to here for build fail


#ifdef CONFIG_DEBUG_RTL871X

	u64 GlobalDebugComponents = \
			_module_rtl871x_xmit_c_ |
			_module_xmit_osdep_c_ |
			_module_rtl871x_recv_c_ |
			_module_recv_osdep_c_ |
			_module_rtl871x_mlme_c_ |
			_module_mlme_osdep_c_ |
			_module_rtl871x_sta_mgt_c_ |
			_module_rtl871x_cmd_c_ |
			_module_cmd_osdep_c_ |
			_module_rtl871x_io_c_ |
			_module_io_osdep_c_ |
			_module_os_intfs_c_|
			_module_rtl871x_security_c_|
			_module_rtl871x_eeprom_c_|
			_module_hal_init_c_|
			_module_hci_hal_init_c_|
			_module_rtl871x_ioctl_c_|
			_module_rtl871x_ioctl_set_c_|
			_module_rtl871x_ioctl_query_c_|
			_module_rtl871x_pwrctrl_c_|
			_module_hci_intfs_c_|
			_module_hci_ops_c_|
			_module_hci_ops_os_c_|
			_module_rtl871x_ioctl_os_c|
			_module_rtl8712_cmd_c_|
			_module_hal_xmit_c_|
			_module_rtl8712_recv_c_ |
			_module_mp_ |
			_module_efuse_;

#endif /* CONFIG_DEBUG_RTL871X */

#if defined(PLATFORM_LINUX) || defined(PLATFORM_ECOS)
void DumpForOneBytes(u8 *pData, u8 Len)
{
	u8 *pSbuf = pData;	
	s8 Length=Len;

	s8 LineIndex=0,BytesIndex,Offset;

	printk("\n.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .A .B .C .D .E .F\n" );
	while( LineIndex< Length)
	{		
			//DBG_871X("%08x: ", (int)(pSbuf+LineIndex) );

			if(LineIndex+16 < Length)
				Offset=16;
			else			
				Offset=Length-LineIndex;
			

			for(BytesIndex=0; BytesIndex<Offset; BytesIndex++)
				printk("%02x ", pSbuf[LineIndex+BytesIndex]);	

			for(BytesIndex=0;BytesIndex<16-Offset;BytesIndex++)	//a last line
    			printk("   ");


			printk("    ");		//between byte and char
			
			for(BytesIndex=0;  BytesIndex<Offset; BytesIndex++) {
                
				if( ' ' <= pSbuf[LineIndex+BytesIndex]  && pSbuf[LineIndex+BytesIndex] <= '~')
					printk("%c", pSbuf[LineIndex+BytesIndex]);
				else
					printk(".");
			}
            
			printk("\n");
			LineIndex += 16;
	}
	
}

void Dump_drv_version(){
	printk("%s %s\n", DRV_NAME, DRIVERVERSION);
	printk("build time: %s %s\n", __DATE__, __TIME__);
}
#endif


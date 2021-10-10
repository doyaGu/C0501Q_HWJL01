#ifndef __RTL8195A_CMD_
#define __RTL8195A_CMD_

#include <drv_types.h>
typedef enum _RTL8195A_H2C_CMD 
{
#if 0
	H2C_8192E_RSVDPAGE	= 0x00,
	H2C_8192E_MSRRPT	= 0x01,
	H2C_8192E_SCAN		= 0x02,
	H2C_8192E_KEEP_ALIVE_CTRL = 0x03,
	H2C_8192E_DISCONNECT_DECISION = 0x04,
	H2C_8192E_INIT_OFFLOAD = 0x06,
	H2C_8192E_AP_OFFLOAD = 0x08,
	H2C_8192E_BCN_RSVDPAGE = 0x09,
	H2C_8192E_PROBERSP_RSVDPAGE = 0x0a,

	H2C_8192E_AP_WOW_GPIO_CTRL = 0x13,
	
	H2C_8192E_SETPWRMODE = 0x20,		
	H2C_8192E_PS_TUNING_PARA = 0x21,
	H2C_8192E_PS_TUNING_PARA2 = 0x22,
	H2C_8192E_PS_LPS_PARA = 0x23,
	H2C_8192E_P2P_PS_OFFLOAD = 0x24,
	H2C_8192E_SAP_PS = 0x26,
	H2C_8192E_RA_MASK = 0x40,
	H2C_8192E_RSSI_REPORT = 0x42,

	H2C_8192E_WO_WLAN = 0x80,
	H2C_8192E_REMOTE_WAKE_CTRL = 0x81,
	H2C_8192E_AOAC_GLOBAL_INFO = 0x82,
	H2C_8192E_AOAC_RSVDPAGE = 0x83,

	//Not defined in new 88E H2C CMD Format
	H2C_8192E_SELECTIVE_SUSPEND_ROF_CMD,
	H2C_8192E_P2P_PS_MODE,
	H2C_8192E_PSD_RESULT,
	MAX_8192E_H2CCMD
#endif
	MAX_8195A_H2CCMD
}RTL8195A_H2C_CMD;

typedef enum _RTL8195A_C2H_EVT
{
#if 0
	C2H_8192E_DBG = 0,
	C2H_8192E_LB = 1,
	C2H_8192E_TXBF = 2,
	C2H_8192E_TX_REPORT = 3,
	C2H_8192E_BT_INFO = 9,
	C2H_8192E_FW_SWCHNL = 0x10,
	C2H_8192E_BT_MP = 11,
	C2H_8192E_RA_RPT=12,	
	
	MAX_8192E_C2HEVENT	
#endif
	C2H_8195A_
}RTL8195A_C2H_EVT;

/******************************************************************
**Network structures
******************************************************************/
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#  include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
typedef struct rtw_ssid {
    unsigned char len;     /**< SSID length */
    unsigned char val[33]; /**< SSID name (AP name)  */
} RTW_PACK_STRUCT_STRUCT rtw_ssid_t;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#  include "pack_end.h"
#endif

#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#  include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
typedef struct rtw_mac {
    unsigned char octet[6]; /**< Unique 6-byte MAC address */
} RTW_PACK_STRUCT_STRUCT rtw_mac_t;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#  include "pack_end.h"
#endif

#define WEP_ENABLED        0x0001
#define TKIP_ENABLED       0x0002
#define AES_ENABLED        0x0004
#define WSEC_SWFLAG        0x0008

#define SHARED_ENABLED  0x00008000
#define WPA_SECURITY    0x00200000
#define WPA2_SECURITY   0x00400000
#define WPS_ENABLED     0x10000000

typedef enum {
    RTW_SECURITY_OPEN           = 0,                                                /**< Open security                           */
    RTW_SECURITY_WEP_PSK        = WEP_ENABLED,                                      /**< WEP Security with open authentication   */
    RTW_SECURITY_WEP_SHARED     = ( WEP_ENABLED | SHARED_ENABLED ),                 /**< WEP Security with shared authentication */
    RTW_SECURITY_WPA_TKIP_PSK   = ( WPA_SECURITY  | TKIP_ENABLED ),                 /**< WPA Security with TKIP                  */
    RTW_SECURITY_WPA_AES_PSK    = ( WPA_SECURITY  | AES_ENABLED ),                  /**< WPA Security with AES                   */
    RTW_SECURITY_WPA2_AES_PSK   = ( WPA2_SECURITY | AES_ENABLED ),                  /**< WPA2 Security with AES                  */
    RTW_SECURITY_WPA2_TKIP_PSK  = ( WPA2_SECURITY | TKIP_ENABLED ),                 /**< WPA2 Security with TKIP                 */
    RTW_SECURITY_WPA2_MIXED_PSK = ( WPA2_SECURITY | AES_ENABLED | TKIP_ENABLED ),   /**< WPA2 Security with AES & TKIP           */
    RTW_SECURITY_WPA_WPA2_MIXED = ( WPA_SECURITY  | WPA2_SECURITY ),                /**< WPA/WPA2 Security                       */

    RTW_SECURITY_WPS_OPEN       = WPS_ENABLED,                                      /**< WPS with open security                  */
    RTW_SECURITY_WPS_SECURE     = (WPS_ENABLED | AES_ENABLED),                      /**< WPS with AES security                   */

    RTW_SECURITY_UNKNOWN        = -1,                                               /**< May be returned by scan function if security is unknown. Do not pass this to the join function! */

    RTW_SECURITY_FORCE_32_BIT   = 0x7fffffff                                        /**< Exists only to force rtw_security_t type to 32 bits */
} rtw_security_t;

typedef enum {
    RTW_BSS_TYPE_INFRASTRUCTURE = 0, /**< Denotes infrastructure network                  */
    RTW_BSS_TYPE_ADHOC          = 1, /**< Denotes an 802.11 ad-hoc IBSS network           */
    RTW_BSS_TYPE_ANY            = 2, /**< Denotes either infrastructure or ad-hoc network */

    RTW_BSS_TYPE_UNKNOWN        = -1, /**< May be returned by scan function if BSS type is unknown. Do not pass this to the Join function */
    RTW_BSS_TYPE_FORCE_32_BIT   = 0x7fffffff 
} rtw_bss_type_t;

typedef enum {
	RTW_WPS_TYPE_DEFAULT 		    	= 0x0000,
	RTW_WPS_TYPE_USER_SPECIFIED 		= 0x0001,
	RTW_WPS_TYPE_MACHINE_SPECIFIED   	= 0x0002,
	RTW_WPS_TYPE_REKEY 			        = 0x0003,
	RTW_WPS_TYPE_PUSHBUTTON 		    = 0x0004,
	RTW_WPS_TYPE_REGISTRAR_SPECIFIED 	= 0x0005,
    RTW_WPS_TYPE_NONE                   = 0x0006,
    RTW_WPS_TYPE_FORCE_32_BIT   = 0x7fffffff 
} rtw_wps_type_t;

typedef enum {
	RTW_802_11_BAND_5GHZ   = 0, /**< Denotes 5GHz radio band   */
	RTW_802_11_BAND_2_4GHZ = 1,  /**< Denotes 2.4GHz radio band */
	RTW_802_11_BAND_FORCE_32_BIT   = 0x7fffffff 
} rtw_802_11_band_t;

//todo: endian free
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#  include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
typedef struct rtw_scan_result {
    rtw_ssid_t              SSID;             /**< Service Set Identification (i.e. Name of Access Point)                    */
    rtw_mac_t               BSSID;            /**< Basic Service Set Identification (i.e. MAC address of Access Point)       */
    signed short		                  signal_strength;  /**< Receive Signal Strength Indication in dBm. <-90=Very poor, >-30=Excellent */
    rtw_bss_type_t          bss_type;         /**< Network type                                                              */
    rtw_security_t          security;         /**< Security type                                                             */
    rtw_wps_type_t          wps_type;         /**< WPS type                                                                  */
    unsigned int                      channel;          /**< Radio channel that the AP beacon was received on                          */
    rtw_802_11_band_t       band;             /**< Radio band                                                                */                                        
} RTW_PACK_STRUCT_STRUCT rtw_scan_result_t;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#  include "pack_end.h"
#endif

typedef enum {
	RTW_MODE_NONE = 0,
	RTW_MODE_STA,
	RTW_MODE_AP
}rtw_mode_t;

//todo: endian free
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#  include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
typedef struct rtw_wifi_setting {
	unsigned int 		mode;
	unsigned char 		ssid[33];
	unsigned char		channel;
	rtw_security_t		security_type;
	unsigned char 		password[65];
	unsigned char		key_idx;
} RTW_PACK_STRUCT_STRUCT rtw_wifi_setting_t;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#  include "pack_end.h"
#endif

#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
typedef struct rtw_eth_frame {
	unsigned char da[6];
	unsigned char sa[6];
	unsigned int len;
	unsigned char type;
} RTW_PACK_STRUCT_STRUCT rtw_eth_frame_t;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_end.h"
#endif

/******************************************************************
**WOWLAN structures
******************************************************************/
enum rtw_wowlan_wakeup_reason {
	RTW_WOWLAN_WAKEUP_BY_PATTERN				= BIT(0),
	RTW_WOWLAN_WAKEUP_BY_DISCONNECTION			= BIT(1),
	RTW_WOWLAN_WAKEUP_MAX 						= 0x7FFFFFFF
}; 

enum rtw_wowlan_cmd_id{
	RTW_WOWLAN_CMD_ENABLE = 0x01, // enable wowlan service
	RTW_WOWLAN_CMD_PATTERNS = 0x02, // wowlan pattern setting
	RTW_WOWLAN_CMD_PROT_OFFLOAD_CONFIG = 0x03, //ARP offload setting
	RTW_WOWLAN_CMD_GET_STATUS = 0x04, // get rtw_wowlan_status
	RTW_WOWLAN_CMD_CLEAR_ALL = 0x05, //clear wowlan content
	RTW_WOWLAN_CMD_MAX = 0xff
};

#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_begin.h"
#endif
RTW_PACK_STRUCT_BEGIN
struct rtw_wowlan_status {
	u32 wakeup_reasons;
} RTW_PACK_STRUCT_STRUCT ;
#ifdef RTW_PACK_STRUCT_USE_INCLUDES
#include "pack_end.h"
#endif

/******************************************************************
**Error Numbers
******************************************************************/
typedef enum
{
    RTW_SUCCESS                      = 0,    /**< Success */
    RTW_PENDING                      = 1,    /**< Pending */
    RTW_TIMEOUT                      = 2,    /**< Timeout */
    RTW_PARTIAL_RESULTS              = 3,    /**< Partial results */
    RTW_INVALID_KEY                  = 4,    /**< Invalid key */
    RTW_DOES_NOT_EXIST               = 5,    /**< Does not exist */
    RTW_NOT_AUTHENTICATED            = 6,    /**< Not authenticated */
    RTW_NOT_KEYED                    = 7,    /**< Not keyed */
    RTW_IOCTL_FAIL                   = 8,    /**< IOCTL fail */
    RTW_BUFFER_UNAVAILABLE_TEMPORARY = 9,    /**< Buffer unavailable temporarily */
    RTW_BUFFER_UNAVAILABLE_PERMANENT = 10,   /**< Buffer unavailable permanently */
    RTW_WPS_PBC_OVERLAP              = 11,   /**< WPS PBC overlap */
    RTW_CONNECTION_LOST              = 12,   /**< Connection lost */

    RTW_ERROR                        = -1,   /**< Generic Error */
    RTW_BADARG                       = -2,   /**< Bad Argument */
    RTW_BADOPTION                    = -3,   /**< Bad option */
    RTW_NOTUP                        = -4,   /**< Not up */
    RTW_NOTDOWN                      = -5,   /**< Not down */
    RTW_NOTAP                        = -6,   /**< Not AP */
    RTW_NOTSTA                       = -7,   /**< Not STA  */
    RTW_BADKEYIDX                    = -8,   /**< BAD Key Index */
    RTW_RADIOOFF                     = -9,   /**< Radio Off */
    RTW_NOTBANDLOCKED                = -10,  /**< Not  band locked */
    RTW_NOCLK                        = -11,  /**< No Clock */
    RTW_BADRATESET                   = -12,  /**< BAD Rate valueset */
    RTW_BADBAND                      = -13,  /**< BAD Band */
    RTW_BUFTOOSHORT                  = -14,  /**< Buffer too short */
    RTW_BUFTOOLONG                   = -15,  /**< Buffer too long */
    RTW_BUSY                         = -16,  /**< Busy */
    RTW_NOTASSOCIATED                = -17,  /**< Not Associated */
    RTW_BADSSIDLEN                   = -18,  /**< Bad SSID len */
    RTW_OUTOFRANGECHAN               = -19,  /**< Out of Range Channel */
    RTW_BADCHAN                      = -20,  /**< Bad Channel */
    RTW_BADADDR                      = -21,  /**< Bad Address */
    RTW_NORESOURCE                   = -22,  /**< Not Enough Resources */
    RTW_UNSUPPORTED                  = -23,  /**< Unsupported */
    RTW_BADLEN                       = -24,  /**< Bad length */
    RTW_NOTREADY                     = -25,  /**< Not Ready */
    RTW_EPERM                        = -26,  /**< Not Permitted */
    RTW_NOMEM                        = -27,  /**< No Memory */
    RTW_ASSOCIATED                   = -28,  /**< Associated */
    RTW_RANGE                        = -29,  /**< Not In Range */
    RTW_NOTFOUND                     = -30,  /**< Not Found */
    RTW_WME_NOT_ENABLED              = -31,  /**< WME Not Enabled */
    RTW_TSPEC_NOTFOUND               = -32,  /**< TSPEC Not Found */
    RTW_ACM_NOTSUPPORTED             = -33,  /**< ACM Not Supported */
    RTW_NOT_WME_ASSOCIATION          = -34,  /**< Not WME Association */
    RTW_SDIO_ERROR                   = -35,  /**< SDIO Bus Error */
    RTW_WLAN_DOWN                    = -36,  /**< WLAN Not Accessible */
    RTW_BAD_VERSION                  = -37,  /**< Incorrect version */
    RTW_TXFAIL                       = -38,  /**< TX failure */
    RTW_RXFAIL                       = -39,  /**< RX failure */
    RTW_NODEVICE                     = -40,  /**< Device not present */
    RTW_UNFINISHED                   = -41,  /**< To be finished */
    RTW_NONRESIDENT                  = -42,  /**< access to nonresident overlay */
    RTW_DISABLED                     = -43   /**< Disabled in this build */
} rtw_result_t;

/******************************************************************
**MACROs
******************************************************************/
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARG(x) ((u8*)(x))[0],((u8*)(x))[1],((u8*)(x))[2],((u8*)(x))[3],((u8*)(x))[4],((u8*)(x))[5]
#define MAC_VALIDATION(mac) ((!((mac[0]==0xff) &&(mac[1]==0xff) && (mac[2]==0xff) && \
		(mac[3]==0xff) && (mac[4]==0xff) &&(mac[5]==0xff)) && \
		!((mac[0]==0x0) && (mac[1]==0x0) && (mac[2]==0x0) && \
		(mac[3]==0x0) && (mac[4]==0x0) &&(mac[5]==0x0)))?1:0)

//#define _FW_UNLINKED_					0
//#define _FW_LINKED_					1
#define	_AT_WLAN_SET_SSID_			"ATW0"
#define	_AT_WLAN_SET_PASSPHRASE_	"ATW1"
#define	_AT_WLAN_SET_KEY_ID_		"ATW2"
#define	_AT_WLAN_AP_SET_SSID_		"ATW3"
#define	_AT_WLAN_AP_SET_SEC_KEY_	"ATW4"
#define	_AT_WLAN_AP_SET_CHANNEL_	"ATW5"
#define	_AT_WLAN_AP_ACTIVATE_		"ATWA"
#define	_AT_WLAN_AP_STA_ACTIVATE_	"ATWB"
#define	_AT_WLAN_JOIN_NET_			"ATWC"
#define	_AT_WLAN_DISC_NET_			"ATWD"
#define	_AT_WLAN_SIMPLE_CONFIG_		"ATWQ"
#define	_AT_WLAN_SCAN_				"ATWS"
#define	_AT_WLAN_IWPRIV_				"ATWZ"
#define	_AT_WLAN_INFO_				"ATW?"
#define	_AT_WLAN_AIRKISS_			"ATWX"
#define	_AT_WLAN_PROMISC_			"ATWM"
#define	_AT_WLAN_WOWLAN_			"ATWV"
#define	_AT_WLAN_WPS_				"ATWW"

s32 rtl8195a_c2h_cmd_handler(PADAPTER padapter, u8 *cmd_data, u16 sz);
#endif //__RTL8195A_CMD_

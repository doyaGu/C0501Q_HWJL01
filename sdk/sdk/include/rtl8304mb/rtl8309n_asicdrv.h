/*
* Copyright (C) 2012 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
* 
* $Revision: v1.0.1 $
* $Date: 2012-10-23 11:18:41 +0800 $
*
* Purpose : asic-level driver header file for RTL8309N switch
*
* Feature : This file consists of following modules:
*
*/


#ifndef _RTL8309N_ASICDRV_H_

#define _RTL8309N_ASICDRV_H_


/*save time of reading LUT*/
/*#define RTL8309N_LUT_CACHE */

/*if you need backup asic info in cpu memroy in order to 
 *accellerate CPU process, please define this macro. If 
 *support IGMP snooping, this macro is required.
 */

#define RTL8309N_PHY_NUMBER        8
#define RTL8309N_PAGE_NUMBER      18
#define RTL8309N_PORT_NUMBER      9
#define RTL8309N_MAX_PORT_ID	(RTL8309N_PORT_NUMBER - 1)
#define RTL8309N_MAX_PHY_ID		(RTL8309N_PORT_NUMBER - 2)
#define RTL8309N_VLAN_ENTRYS      16            /*Vlan entry number*/ 
#define RTL8309N_ACL_ENTRYNUM    16           /*ACL entry number*/
#define RTL8309N_MAX_LUT_ENTRYNUM   0X800       /*MAX LUT entry number*/     
#define RTL8309N_IDLE_TIMEOUT   (100)
#define RTL8309N_QOS_RATE_INPUT_MAX 0x640		/*100M bps*/
#define RTL8309N_QOS_RATE_INPUT_MAX_P0_P8 0xC80  /*200M bps for P0 or P8*/
#define RTL8309N_VIDMAX                       0XFFF
#define RTL8309N_MAX_VLANINDEX	15
#define RTL8309N_MAX_PORTMASK           0X1FF
#define RTL8309N_MAX_QUEUE_NUM 		4
#define RTL8309N_MAX_QUEUE_ID 		(RTL8309N_MAX_QUEUE_NUM - 1)
#define RTL8309N_MAX_1QTAG_PRIO_ID  7
#define RTL8309N_MAX_DSCP_VALUE 63
#define RTL8309N_NOCPUPORT  (RTL8309N_MAX_PORT_ID + 1)
#define RTL8309N_NOWANPORT  (RTL8309N_MAX_PORT_ID + 1)

enum RTL8309N_ENABLE_STATUS
{
	RTL8309N_DISABLED = 0,
	RTL8309N_ENABLED,
	RTL8309N_ENABLE_END
};

enum RTL8309N_REGPAGE
{
    RTL8309N_REGPAGE0 = 0,
    RTL8309N_REGPAGE1,
    RTL8309N_REGPAGE2,
    RTL8309N_REGPAGE3,
    RTL8309N_REGPAGE4, 
    RTL8309N_REGPAGE5, 
    RTL8309N_REGPAGE6, 
	RTL8309N_REGPAGE7, 
	RTL8309N_REGPAGE8, 
	RTL8309N_REGPAGE9, 
	RTL8309N_REGPAGE10, 
	RTL8309N_REGPAGE11, 
	RTL8309N_REGPAGE12, 
	RTL8309N_REGPAGE13, 
	RTL8309N_REGPAGE14, 
	RTL8309N_REGPAGE15, 
	RTL8309N_REGPAGE16, 
	RTL8309N_REGPAGE17, 
    RTL8309N_REGPAGE_END = 19
    
};

enum RTL8309N_PORTNUM
{
    RTL8309N_PORT0 = 0,
    RTL8309N_PORT1,        
    RTL8309N_PORT2,
    RTL8309N_PORT3,  
    RTL8309N_PORT4,
    RTL8309N_PORT5,
    RTL8309N_PORT6,  
    RTL8309N_PORT7,
    RTL8309N_PORT8,       
};

enum RTL8309N_PHYMODE
{
    RTL8309N_ETHER_AUTO_100FULL = 1,
    RTL8309N_ETHER_AUTO_100HALF,
    RTL8309N_ETHER_AUTO_10FULL,
    RTL8309N_ETHER_AUTO_10HALF
};

enum RTL8309N_PHYSPD
{
    RTL8309N_ETHER_SPEED_100 = 100,
    RTL8309N_ETHER_SPEED_10 = 10        
};

enum RTL8309N_PORT_MEDIA_TYPE
{
	RTL8309N_PORT_MEDIA_UTP = 0,
	RTL8309N_PORT_MEDIA_FIBER,
	RTL8309N_PORT_MEDIA_END
};

enum RTL8309N_PORT_LINKSTATUS
{
	RTL8309N_PORT_LINK_DOWN = 0,
	RTL8309N_PORT_LINK_UP,
	RTL8309N_PORT_LINK_END
};

enum RTL8309N_PORT_SPEEDSTATUS
{
	RTL8309N_PORT_SPEED_10M = 0,
	RTL8309N_PORT_SPEED_20M,
	RTL8309N_PORT_SPEED_100M,
	RTL8309N_PORT_SPEED_200M,
	RTL8309N_PORT_SPEED_END
};

enum RTL8309N_PORT_DUPLEX_MODE
{
	RTL8309N_PORT_HALF_DUPLEX = 0,
	RTL8309N_PORT_FULL_DUPLEX,
	RTL8309N_PORT_DUPLEX_END
};

enum RTL8309N_TAGMOD
{
    RTL8309N_VLAN_IRTAG = 0,             /*The switch will remove VLAN tags and add new tags */
    RTL8309N_VLAN_RTAG,                   /*The switch will remove VLAN tags */
    RTL8309N_VLAN_ITAG,                   /*The switch will  add new VLANtag */
    RTL8309N_VLAN_UNDOTAG            /*Do not insert or remove  VLAN tag */
};

enum RTL8309N_LEAKYVLAN
{
    RTL8309N_VLAN_LEAKY_UNICAST = 0,
    RTL8309N_VLAN_LEAKY_MULTICAST,
    RTL8309N_VLAN_LEAKY_BRDCAST,
    RTL8309N_VLAN_LEAKY_ARP,
    RTL8309N_VLAN_LEAKY_MIRROR,
    RTL8309N_VLAN_LEAKY_END
};

/*ACL Packet processing method*/
enum RTL8309N_ACTION
{
    RTL8309N_ACT_PERMIT = 0,      /*permit the packet*/ 
	RTL8309N_ACT_COPY2CPU,		/*copy to CPU*/
    RTL8309N_ACT_TRAP2CPU,   /*trap the packet to cpu*/	
    RTL8309N_ACT_DROP,    /*drop the packet*/    
    RTL8309N_ACT_MIRROR,     /*mirror the packet */
    RTL8309N_ACT_FLOOD,      /*flood the packet  */   
    RTL8309N_ACTION_END
};

/*
  * PHY control register field definitions 
  */
#define RTL8309N_SPEED_SELECT_100M                       (1 << 13)  
#define RTL8309N_ENABLE_AUTONEGO                         (1 << 12)
#define RTL8309N_RESTART_AUTONEGO                        (1 << 9)
#define RTL8309N_SELECT_FULL_DUPLEX                      (1 << 8)

/* 
  *PHY auto-negotiation advertisement and link partner 
  *ability registers field definitions
  */
#define RTL8309N_NEXT_PAGE_ENABLED                       (1 << 15)
#define RTL8309N_ACKNOWLEDGE                                 (1 << 14)
#define RTL8309N_REMOTE_FAULT                                 (1 << 13)
#define RTL8309N_CAPABLE_PAUSE                               (1 << 10)
#define RTL8309N_CAPABLE_100BASE_T4                      (1 << 9)
#define RTL8309N_CAPABLE_100BASE_TX_FD                (1 << 8)
#define RTL8309N_CAPABLE_100BASE_TX_HD                (1 << 7)
#define RTL8309N_CAPABLE_10BASE_TX_FD                  (1 << 6)
#define RTL8309N_CAPABLE_10BASE_TX_HD                  (1 << 5)
#define RTL8309N_SELECTOR_MASK                               0x1F
#define RTL8309N_SELECTOR_OFFSET                             0

enum RTL8309N_VLAN_TAGINSERTREMOVE_MODE
{
	RTL8309N_VLAN_TAG_REPALCE = 0,	/*replace VID as PVID for tagged frames, insert PVID for untagged frames*/
	RTL8309N_VLAN_TAG_REMOVE,	/*remove tag from tagged frames*/
	RTL8309N_VLAN_TAG_INSERT,	/*insert PVID to untagged frames*/
	RTL8309N_VLAN_TAG_DONTTOUCH,	/*don't touch tagged and untagged frames*/
	RTL8309N_VLAN_TAG_MODE_END
};

enum RTL8309N_IGMPCTL
{
    RTL8309N_IGMP = 0,
    RTL8309N_MLD, 
    RTL8309N_PPPOE_IPV4,
    RTL8309N_PPPOE_MLD,    
    RTL8309N_IGMPCTL_END
};

#define RTL8309N_PORT_RX  0
#define RTL8309N_PORT_TX  1

enum RTL8309N_QUENUM
{
    RTL8309N_QUEUE0 = 0,
    RTL8309N_QUEUE1,
    RTL8309N_QUEUE2,
    RTL8309N_QUEUE3,
    RTL8309N_QUEUE_END
};

enum RTL8309N_QOS_PRISRC
{
	RTL8309N_ACL_PRI = 0,
	RTL8309N_1Q_PRI,
	RTL8309N_PORT_PRI,
	RTL8309N_DSCP_PRI,
	RTL8309N_IP_PRI,
	RTL8309N_REASSIGNED_PRI,
	RTL8309N_RLDP_PRI,
	RTL8309N_CPUTAG_PRI,
	RTL8309N_QOS_PRISRC_END
};

enum RTL8309N_PRI
{
    RTL8309N_PRIO0 = 0,
    RTL8309N_PRIO1,
    RTL8309N_PRIO2,
    RTL8309N_PRIO3,    
    RTL8309N_PRI_END
};

enum RTL8309N_SCHSET
{
    RTL8309N_QOS_SET0 = 0, 
    RTL8309N_QOS_SET1        
};

#define RTL8309N_DSCP_USERA        0
#define RTL8309N_DSCP_USERB        1
#define RTL8309N_IPADD_A 0
#define RTL8309N_IPADD_B 1

#define RTL8309N_FCO_FULLTHR      0x0
#define RTL8309N_FCO_OVERTHR     0x1


#define RTL8309N_ACL_INVALID       0x9
#define RTL8309N_ACL_ANYPORT     0xA

enum RTL8309N_ACLPRO
{
    RTL8309N_ACL_ETHER = 0,
    RTL8309N_ACL_TCP,
    RTL8309N_ACL_UDP,
    RTL8309N_ACL_TCPUDP    
};

enum RTL8309N_MIBCNT
{
    RTL8309N_MIB_TXBYTECNT = 0,
    RTL8309N_MIB_RXBYTECNT = 2,
    
    RTL8309N_MIB_TXPKTCNT = 4,
    RTL8309N_MIB_RXPKTCNT,
    
    RTL8309N_MIB_RXDPCNT,
    RTL8309N_MIB_RXCRCCNT,
    RTL8309N_MIB_RXFRAGCNT,
    
    RTL8309N_MIB_TXBRDCNT,
    RTL8309N_MIB_RXBRDCNT,
    
    RTL8309N_MIB_TXMULTCNT,
    RTL8309N_MIB_RXMULTCNT,
    
    RTL8309N_MIB_TXUNICNT,
    RTL8309N_MIB_RXUNICNT,
    
    RTL8309N_MIB_RXSYMBLCNT = 15,
    RTL8309N_MIB_END
};

enum RTL8309N_MIBOP
{
    RTL8309N_MIB_RESET = 0, 
    RTL8309N_MIB_START        
};

#define RTL8309N_MIB_BYTE            0
#define RTL8309N_MIB_PKT              1

#define RTL8309N_MIR_INVALID       0x6

enum RTL8309N_LUT4WAY
{
    RTL8309N_LUT_ENTRY0 = 0,
    RTL8309N_LUT_ENTRY1,
    RTL8309N_LUT_ENTRY2,
    RTL8309N_LUT_ENTRY3    
}; 

enum RTL8309N_FID
{
	RTL8309N_FID0 = 0,
	RTL8309N_FID1,
	RTL8309N_FID2,
	RTL8309N_FID3
};

#define RTL8309N_LUT_FULL            -2  /*Four way of the same entry are all written by cpu*/
#define RTL8309N_LUT_NOTEXIST     -3
#define RTL8309N_LUT_NOADDR     10000

enum RTL8309N_LUTAGE
{
    RTL8309N_LUT_AGEOUT = 0,
    RTL8309N_LUT_AGE100 ,
    RTL8309N_LUT_AGE200 ,
    RTL8309N_LUT_AGE300    
};

#define RTL8309N_LUT_DYNAMIC      0
#define RTL8309N_LUT_STATIC         1
#define RTL8309N_LUT_UNAUTH       0
#define RTL8309N_LUT_AUTH           1
#define RTL8309N_ADDR_LEN 6

enum RTL8309N_SPAN_STATE
{
    RTL8309N_SPAN_DISABLE = 0,
    RTL8309N_SPAN_BLOCK,
    RTL8309N_SPAN_LEARN,
    RTL8309N_SPAN_FORWARD, 
    RTL8309N_SPAN_END
};

enum RTL8309N_RLDP_TIMER
{
	RTL8309N_RLDP_TIMER_3_5MIN = 0,
	RTL8309N_RLDP_TIMER_100S,
	RTL8309N_RLDP_TIMER_10S,
	RTL8309N_RLDP_TIMER_1S,
	RTL8309N_RLDP_TIMER_END
};

enum RTL8309N_DOT1X_STATE
{
    RTL8309N_PORT_UNAUTH = 0, 
    RTL8309N_PORT_AUTH
};

enum RTL8309N_DOT1X_PORT_DIR
{
    RTL8309N_PORT_BOTHDIR = 0,
    RTL8309N_PORT_INDIR      
};

enum RTL8309N_DOT1X_MAC_DIR
{
    RTL8309N_MAC_BOTHDIR = 0,
    RTL8309N_MAC_INDIR      
};

enum RTL8309N_MULTICAST_TYPE
{
	RTL8309N_MULCAST_IPV4,
	RTL8309N_MULCAST_IPV6,
	RTL8309N_MULCAST_END
};

enum RTL8309N_ABNORMAL_PKT
{
    RTL8309N_UNMATCHVID =0,
    RTL8309N_DOT1X_UNAUTH,
    RTL8309N_LUT_SABLOCK,
    RTL8309N_LUT_DABLOCK,
    RTL8309N_ABNORMAL_END
};

enum RTL8309N_RMA
{      
    RTL8309N_RMA00,      /*reserved address "01-80-c2-00-00-00"*/
    RTL8309N_RMA01,     /*reserved address "01-80-c2-00-00-01"*/
	RTL8309N_RMA02,		/*reserved address "01-80-c2-00-00-02"*/
	RTL8309N_RMA03,		/*reserved address "01-80-c2-00-00-03"*/
	RTL8309N_RMA04_0D0F,    /*reserved address "01-80-C2-00-00-04 --- 01-80-C2-00-00-0D" */
	                       /* "01-80-C2-00-00-0F" */
	RTL8309N_RMA0E,      /*reserved address "01-80-C2-00-00-0E"*/
	RTL8309N_RMA10,		 /*reserved address "01-80-c2-00-00-10"*/
	RTL8309N_RMA11_1F,    /*reserved address "01-80-C2-00-00-11 --- 01-80-C2-00-00-1F"*/
	RTL8309N_RMA20,		 /*reserved address "01-80-c2-00-00-20"*/
	RTL8309N_RMA21,		 /*reserved address "01-80-c2-00-00-21"*/ 
	RTL8309N_RMA22_2F,    /*reserved address "01-80-c2-00-00-22 --- 01-80-c2-00-00-2F"*/
	RTL8309N_RMA31_3F,    /*reserved address "01-80-c2-00-00-31 --- 01-80-c2-00-00-3F"*/ 
	RTL8309N_RMAIGMP,     /*IGMP packet without pppoe header*/
	RTL8309N_RMAMLD,      /*MLD packet without pppoe header*/
	RTL8309N_RMAPPPOE_IPV4,   /*IGMP packet with pppoe header*/
	RTL8309N_RMAPPPOE_MLD,    /*MLD packet with pppoe header*/
	RTL8309N_RMASWITCH_MAC,   /*RMA = switch MAC*/
	RTL8309N_RMAUNKNOWN_UNIDA,    /*reserved address "xxxxxxx0-xx-xx-xx-xx-xx"*/
	RTL8309N_RMAUNKNOWN_MULTDA,   /*reserved address "xxxxxxx1-xx-xx-xx-xx-xx"*/
	RTL8309N_RMAUSER         /*USER defined Reserved address*/
};

enum RTL8309N_PKT_TYPE
{
	RTL8309N_UNKOWN_UNIDAPKT,		/*storm filter for unknown unicast DA packet*/
	RTL8309N_UNKOWN_MULTDAPKT,   /*storm filter for unknown multicast DA packet*/
	RTL8309N_MULTICASTPKT,	  /*Multicast packet, include known and unknown*/
    RTL8309N_BROADCASTPKT,   /*Broadcast packet*/
	RTL8309N_PKT_TYPE_END
};


/*Max packet length*/
enum RTL8309N_PKT_LEN
{
    RTL8309N_MAX_PKTLEN_1518 = 0,  /*1518 bytes without any tag; 1522 bytes: with VLAN tag or CPU tag; 1526 bytes: with VLAN tag and CPU tag*/
    RTL8309N_MAX_PKTLEN_USER,       /*user defined, 64<len<2048*/
    RTL8309N_MAX_PKTLEN_2048,       /*2048 bytes*/
    RTL8309N_MAX_PKTLEN_16k       /*16k bytes*/
};

enum RTL8309N_FLC_QUEUETYPE
{
	RTL8309N_FLC_QUEUEDSCTHRLD_ON = 0,
	RTL8309N_FLC_QUEUEDSCTHRLD_OFF,
	RTL8309N_FLC_QUEUEPKTTHRLD_ON,
	RTL8309N_FLC_QUEUEPKTTHRLD_OFF,
	RTL8309N_FLC_QUEUETHRLD_END
};

enum RTL8309N_FLC_PORTTYPE
{
	RTL8309N_FLC_PORTDSCTHRLD_ON = 0,
	RTL8309N_FLC_PORTDSCTHRLD_OFF
};

enum RTL8309N_L2_HASH_METHOD
{
    RTL8309N_HASH_OPT0 = 0,
    RTL8309N_HASH_OPT1,
    RTL8309N_HASH_END
};

enum RTL8309N_LED_GROUP
{
	RTL8309N_LED_GROUP_0 = 0,
	RTL8309N_LED_GROUP_1,
	RTL8309N_LED_GROUP_END
};

enum RTL8309N_LED_MODE
{
	RTL8309N_LED_MODE_0 = 0,
	RTL8309N_LED_MODE_1,	
	RTL8309N_LED_MODE_2,	
	RTL8309N_LED_MODE_3,	
	RTL8309N_LED_MODE_4,
	RTL8309N_LED_MODE_5,	
	RTL8309N_LED_MODE_6,
	RTL8309N_LED_MODE_END	
};

enum RTL8309N_LED_RTCTMODE
{
	RTL8309N_LED_RTCTTEST_PARALLEL = 0,
	RTL8309N_LED_RTCTTEST_SCAN,
	RTL8309N_LED_RTCTTEST_END
};

enum RTL8309N_LED_BLINKRATE
{
	RTL8309N_LED_BLINKRATE_32MS = 0,
	RTL8309N_LED_BLINKRATE_128MS,
	RTL8309N_LED_BLINKRATE_400MS,
	RTL8309N_LED_BLINKRATE_800MS,
	RTL8309N_LED_BLINKRATE_END
};

enum RTL8309N_LED_TIME
{
	RTL8309N_LED_TIME_80MS = 0,
	RTL8309N_LED_TIME_128MS,
	RTL8309N_LED_TIME_256MS,
	RTL8309N_LED_TIME_512MS,
	RTL8309N_LED_TIME_2SEC,
	RTL8309N_LED_TIME_3SEC,
	RTL8309N_LED_TIME_4SEC,
	RTL8309N_LED_TIME_5SEC,
	RTL8309N_LED_TIME_END
};

enum RTL8309N_LED_BUZZER_MODE
{
    RTL8309N_LED_BUZZER_PASIVE = 0,
    RTL8309N_LED_BUZZER_ACTIVE,
    RTL8309N_LED_BUZZER_END
};

typedef enum rtl8309n_mode_ext_t
{
	RTL8309N_MODE_EXT_TMII_MAC = 0,
	RTL8309N_MODE_EXT_TMII_PHY,
	RTL8309N_MODE_EXT_MII_MAC,
	RTL8309N_MODE_EXT_MII_PHY,
	RTL8309N_MODE_EXT_RMII,
	RTL8309N_MODE_EXT_DISABLED,
	RTL8309N_MODE_EXT_END
}rtl8309n_mode_ext_t;

typedef enum rtl8309n_acceptFrameType_e
{
    RTL8309N_ACCEPT_UNTAG_ONLY = 1 ,    /* untagged and priority-tagged */
	RTL8309N_ACCEPT_TAG_ONLY = 2,		  /* tagged */
    RTL8309N_ACCEPT_ALL = 3,                   /* untagged, priority-tagged and tagged */
    RTL8309N_ACCEPT_TYPE_END
} rtl8309n_acceptFrameType_t;


typedef enum rtl8309n_priSrc_e
{
    RTL8309N_PRISRC_PPRI = 0,              /*port-based outer tag priority*/
    RTL8309N_PRISRC_1PRMK,                /* 1p remarking priority*/
    RTL8309N_PRISRC_END
} rtl8309n_priSrc_t;

typedef enum rtl8309n_cpu_trapPkt_e
{
    RTL8309N_CPU_TRAP_DOT1X = 0,
    RTL8309N_CPU_TRAP_VID,
    RTL8309N_CPU_TRAP_ISPMAC,
    RTL8309N_CPU_TRAP_ACL,
    RTL8309N_CPU_TRAP_IGMPMLD,
    RTL8309N_CPU_TRAP_RMA,
    RTL8309N_CPU_TRAP_EXMACLIMIT,
    RTL8309N_CPU_TRAP_RLPP,
    RTL8309N_CPU_TRAP_LUTMACBLOCK,
    RTL8309N_CPU_TRAP_ALL,
    RTL8309N_CPU_TRAP_END 
}rtl8309n_cpu_trapPkt_t;


typedef struct  rtl8309n_port_mac_ability_s
{
	uint32 speed;
    uint32 duplex;
    uint32 link;    
    uint32 nway;    
    uint32 txpause;
    uint32 rxpause;
	uint32 media;
}rtl8309n_port_mac_ability_t;

typedef struct rtl8309n_qos_priArbitPara_s 
{
    uint32 acl_pri_weight;
	uint32 dot1q_pri_weight;
	uint32 port_pri_weight;
    uint32 dscp_pri_weight;
} rtl8309n_qos_priArbitPara_t;

typedef struct rtl8309n_qos_schPara_s
{
    uint8   q0_wt;
    uint8   q1_wt;
    uint8   q2_wt;
    uint8   q3_wt;   
} rtl8309n_qos_schPara_t;

typedef struct rtl8309n_acl_entry_s
{
	uint32 priority;
	uint32 action;
	uint32 phyPort;
	uint32 protocol;
	uint32 data;
}rtl8309n_acl_entry_t;

typedef struct rtl8309n_rma_entry_s
{
	uint32 action;
	uint32 enable_rmapri;
	uint32 priority;
}rtl8309n_rma_entry_t;

typedef struct  rtl8309n_mac_s
{
    uint8 octet[RTL8309N_ADDR_LEN];
}rtl8309n_mac_t;

typedef struct rtl8309n_l2_ucastEntry_s
{
	uint32 isStatic;
	uint32 auth;
	uint32 da_block;
	uint32 sa_block;
	uint32 age;
	uint32 port;
	uint32 fid;
	rtl8309n_mac_t mac;
}rtl8309n_l2_ucastEntry_t;


typedef struct rtl8309n_l2_mcastEntry_s
{
	uint32 auth;
	uint32 fid;
	uint32 port_mask;
	rtl8309n_mac_t mac;
}rtl8309n_l2_mcastEntry_t;

typedef struct rtl8309n_storm_attr_s
{
	uint32 unit;
	uint32 rate;
	uint32 burst;
	uint32 ifg_include;
}rtl8309n_storm_attr_t;


typedef struct rtl8309_vlanConfigBakPara_s 
{
    uint8 enVlan;
    uint8 enUniLeakyVlan;
	uint8 enMulleakyVlan;
	uint8 enBrdleakyVlan;
    uint8 enArpleakyVlan;
    uint8 enMirLeakyVlan;
    uint8 enIngress;
} rtl8309_vlanConfigBakPara_t;

typedef struct rtl8309_vlanConfigPerPortBakPara_s 
{
    uint8 enVlanTagOnly;
    uint8 enTagAware;
    uint8 vlantagInserRm;
    uint8 en1PRemark;
    uint8 enNulPvidRep;         
} rtl8309_vlanConfigPerPortBakPara_t;

typedef struct  rtl8309_vlanTblBakPara_s 
{
    uint16 vid;
    uint8 memberPortMask;        
} rtl8309_vlanTblBakPara_t;

typedef struct rtl8309_aclTblBakPara_s 
{
    uint8 phy_port;
    uint8 proto;
    uint16 data;
    uint8 action;
    uint8 pri;
} rtl8309_aclTblBakPara_t;

typedef struct rtl8309_mirConfigBakPara_s
{
    uint8 mirPort;
    uint16 mirRxPortMask;
    uint16 mirTxPortMask;
    uint8 enMirself;
    uint8 enMirMac;
    uint8 mir_mac[6];
} rtl8309_mirConfigBakPara_t;

typedef struct rtl8309n_rldp_controlAttr_s
{
	uint32 timer;   /*period to transmit RLDP packet*/
	uint32 priority;    /*priority used to map packet into a tx queue*/
	uint32 ttl;     /*TTL timer value*/
	rtl8309n_mac_t mac;     /*switch mac address[0:47]*/
}rtl8309n_rldp_controlAttr_t;


typedef struct rtl8309_ConfigBakPara_s  
{
    rtl8309_vlanConfigBakPara_t vlanConfig;                    /*VLAN global configuration*/
    rtl8309_vlanConfigPerPortBakPara_t vlanConfig_perport[9];   /*VLAN per-port configuration*/
    rtl8309_vlanTblBakPara_t vlanTable[16]; /*It backups VLAN table in cpu memory*/
    uint8 vlanPvidIdx[9];   /*per-port PVID index*/                  
    uint8 En1PremarkPortMask; /*Enable/disable 802.1P remarking  port mask */
    uint8 dot1PremarkCtl[4]; /*802.1p remarking table*/
    uint8 dot1DportCtl[9]; /*Spanning tree port state*/
    rtl8309_aclTblBakPara_t aclTbl[16];         /*ACL table*/
    rtl8309_mirConfigBakPara_t mir; /*mirror configuration*/                                                                         
} rtl8309_ConfigBakPara_t;


extern rtl8309_ConfigBakPara_t rtl8309_TblBak; 

#endif






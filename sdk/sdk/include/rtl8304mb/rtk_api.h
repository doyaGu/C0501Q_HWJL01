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
* Purpose : realtek common API Header File
* 
* Feature :  This file consists of following modules:
*/

#ifndef _RTK_API_H_
#define _RTK_API_H_


/*
 * Include Files
 */
#include <rtl8309n_types.h>
#include <rtl8309n_asicdrv.h>
#include <rtk_error.h>

/*
 * Data Type Declaration
 */
#define ENABLE                                                    1
#define DISABLE                                                   0

#define PHY_CONTROL_REG                                   0
#define PHY_STATUS_REG                                      1
#define PHY_AN_ADVERTISEMENT_REG                    4
#define PHY_AN_LINKPARTNER_REG                        5
#define PHY_1000_BASET_CONTROL_REG                9
#define PHY_1000_BASET_STATUS_REG                   10
#define PHY_RESOLVED_REG                                   17

/*Qos related configuration define*/
#define QOS_DEFAULT_TICK_PERIOD                     (19-1)
#define QOS_DEFAULT_BYTE_PER_TOKEN                  34
#define QOS_DEFAULT_LK_THRESHOLD                    (34*3) /* Why use 0x400? */


#define QOS_DEFAULT_INGRESS_BANDWIDTH               0x3FFF /* 0x3FFF => unlimit */
#define QOS_DEFAULT_EGRESS_BANDWIDTH                0x3D08 /*( 0x3D08 + 1) * 64Kbps => 1Gbps*/
#define QOS_DEFAULT_PREIFP                          1
#define QOS_DEFAULT_PACKET_USED_PAGES_FC            0x60
#define QOS_DEFAULT_PACKET_USED_FC_EN               0
#define QOS_DEFAULT_QUEUE_BASED_FC_EN               1

#define QOS_DEFAULT_PRIORITY_SELECT_PORT            8
#define QOS_DEFAULT_PRIORITY_SELECT_1Q              0
#define QOS_DEFAULT_PRIORITY_SELECT_ACL             0
#define QOS_DEFAULT_PRIORITY_SELECT_DSCP            0

#define QOS_DEFAULT_DSCP_MAPPING_PRIORITY           0

#define QOS_DEFAULT_1Q_REMARKING_ABILITY             0
#define QOS_DEFAULT_DSCP_REMARKING_ABILITY          0
#define QOS_DEFAULT_QUEUE_GAP                                20
#define QOS_DEFAULT_QUEUE_NO_MAX                          6
#define QOS_DEFAULT_AVERAGE_PACKET_RATE             0x3FFF
#define QOS_DEFAULT_BURST_SIZE_IN_APR                   0x3F
#define QOS_DEFAULT_PEAK_PACKET_RATE                    2
#define QOS_DEFAULT_SCHEDULER_ABILITY_APR           1     /*disable*/
#define QOS_DEFAULT_SCHEDULER_ABILITY_PPR           1    /*disable*/
#define QOS_DEFAULT_SCHEDULER_ABILITY_WFQ          1    /*disable*/

#define QOS_WEIGHT_MAX                              128

#define LED_GROUP_MAX                               3

#define RTK_FILTER_RAW_FIELD_NUMBEER                8

#define ACL_DEFAULT_ABILITY                         0
#define ACL_DEFAULT_UNMATCH_PERMIT                  1

#define ACL_RULE_FREE                               0
#define ACL_RULE_INAVAILABLE                        1

#define FILTER_POLICING_MAX                         8
#define FILTER_LOGGING_MAX                          8
#define FILTER_PATTERN_MAX                          4

#define STORM_UNUC_INDEX                            39
#define STORM_UNMC_INDEX                            47
#define STORM_MC_INDEX                              55
#define STORM_BC_INDEX                              63

#define RTK_MAX_FID 4
#define RTK_MAX_NUM_OF_INTERRUPT_TYPE               1
#define RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST    1
#define RTK_MAX_NUM_OF_INTERNAL_PRIORITY            4		/*change at 2011-09-04*/
#define RTK_MAX_NUM_OF_DOT1Q_PRIORITY            8		/*add at 2011-09-04*/
#define RTK_MAX_NUM_OF_QUEUE                        4	/*change at 2011-09-04*/
#define RTK_MAX_NUM_OF_TRUNK_HASH_VAL               1
#define RTK_MAX_NUM_OF_PORT                         9		/*change at 2011-09-04*/
#define RTK_PORT_ID_MAX                            (RTK_MAX_NUM_OF_PORT - 1)/*change at 2011-09-04*/
#define RTK_ACL_INVALID_PORT						0x9 /*add at 2010-10-28*/
#define RTK_ACL_ANYPORT								0xA	/*add at 2010-10-28*/
#define RTK_PHY_ID_MAX                             (RTK_MAX_NUM_OF_PORT - 2)/*change at 2011-09-04*/
#define RTK_QUEUE_ID_MAX 						   (RTK_MAX_NUM_OF_QUEUE - 1)
#define RTK_MAX_NUM_OF_PROTO_TYPE                   0xFFFF
#define RTK_MAX_NUM_OF_MSTI                         0xF
#define RTK_MAX_NUM_OF_PORT_LEARN_LIMIT             0x1F
#define RTK_MAX_NUM_OF_SYSTEM_LEARN_LIMIT           0xFF
#define RTK_MAX_VID 								0XFFF		/*add at 2011-09-05*/


#define RTK_MAX_PORT_MASK                             0x1FF		/*change at 2011-09-04*/

#define RTK_FLOWCTRL_PAUSE_ALL                      1980
#define RTK_FLOWCTRL_DROP_ALL                       2012
#define RTK_FLOWCTRL_PAUSE_SYSTEM_ON                1200
#define RTK_FLOWCTRL_PAUSE_SYSTEM_OFF               1000
#define RTK_FLOWCTRL_DROP_SYSTEM_ON                 1200
#define RTK_FLOWCTRL_DROP_SYSTEM_OFF                1000
#define RTK_FLOWCTRL_PAUSE_SHARE_ON                 216
#define RTK_FLOWCTRL_PAUSE_SHARE_OFF                208
#define RTK_FLOWCTRL_DROP_SHARE_ON                  216
#define RTK_FLOWCTRL_DROP_SHARE_OFF                 208
#define RTK_FLOWCTRL_PAUSE_PORT_ON                  140
#define RTK_FLOWCTRL_PAUSE_PORT_OFF                 132
#define RTK_FLOWCTRL_DROP_PORT_ON                   140
#define RTK_FLOWCTRL_DROP_PORT_OFF                  132
#define RTK_FLOWCTRL_PAUSE_PORT_PRIVATE_ON          26
#define RTK_FLOWCTRL_PAUSE_PORT_PRIVATE_OFF         22
#define RTK_FLOWCTRL_DROP_PORT_PRIVATE_ON           26
#define RTK_FLOWCTRL_DROP_PORT_PRIVATE_OFF          22
#define RTK_FLOWCTRL_PORT_DROP_EGRESS               210
#define RTK_FLOWCTRL_QUEUE_DROP_EGRESS              2047
#define RTK_FLOWCTRL_PORT_GAP                       72
#define RTK_FLOWCTRL_QUEUE_GAP                      18

#define RTK_WHOLE_SYSTEM                            0xFF

#define RTK_EXT_0                                   0
#define RTK_EXT_1                                   1

#define RTK_EXT_0_MAC                               9
#define RTK_EXT_1_MAC                               8

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN                                6
#endif

#define IPV6_ADDR_LEN                               16
#define IPV4_ADDR_LEN                               4

#define RTK_DOT_1AS_TIMESTAMP_UNIT_IN_WORD_LENGTH   3UL

#define RTK_IPV6_ADDR_WORD_LENGTH                   4UL

#define ALLPORT 0xFF

typedef enum rtk_chip_mode_e
{
    CHIP_8367 = 0,
    CHIP_8370,
    CHIP_8370M,
    CHIP_8376, 
    CHIP_8218, 
    CHIP_8306E,
    CHIP_END
}rtk_chip_mode_t;

typedef enum rtk_cpu_insert_e
{
    CPU_INSERT_TO_ALL = 0,
    CPU_INSERT_TO_TRAPPING,
    CPU_INSERT_TO_NONE,
    CPU_INSERT_END
}rtk_cpu_insert_t;

typedef enum rtk_cpu_position_e
{
    CPU_POS_ATTER_DA = 0,
    CPU_POS_AFTER_CRC,
    CPU_POS_END
}rtk_cpu_position_t;


/* Type of port-based dot1x auth/unauth*/
typedef enum rtk_dot1x_auth_status_e
{
    UNAUTH = 0,
    AUTH,
    AUTH_STATUS_END
} rtk_dot1x_auth_status_t;

typedef enum rtk_dot1x_direction_e
{
    BOTH = 0,
    IN,
    DIRECTION_END
} rtk_dot1x_direction_t;

typedef enum rtk_mode_ext_e
{
    MODE_EXT_DISABLE = 0,
    MODE_EXT_RGMII,
    MODE_EXT_MII_MAC,
    MODE_EXT_MII_PHY, 
    MODE_EXT_TMII_MAC,
    MODE_EXT_TMII_PHY, 
    MODE_EXT_GMII,
    MODE_EXT_RMII,
    MODE_EXT_END
} rtk_mode_ext_t;

typedef struct
{
    uint32 value[RTK_DOT_1AS_TIMESTAMP_UNIT_IN_WORD_LENGTH];
} rtk_filter_dot1as_timestamp_t;

/* unauth pkt action */
typedef enum rtk_dot1x_unauth_action_e
{
    DOT1X_ACTION_DROP = 0,
    DOT1X_ACTION_TRAP2CPU,
    DOT1X_ACTION_GUESTVLAN,
    DOT1X_ACTION_END
} rtk_dot1x_unauth_action_t;

typedef uint32  rtk_dscp_t;         /* dscp vlaue */

typedef uint32 rtk_mib_cntValue_t;
typedef enum rtk_mib_cntType_e
{
    MIB_COUNTER_TYPE_BYTE = 0,
    MIB_COUNTER_TYPE_PKT,
    MIB_COUNTER_TYPE_END
}rtk_mib_cntType_t;

typedef enum rtk_mib_counter_e
{
    MIB_TXBYTECNT = 0,
    MIB_RXBYTECNT = 2,
    
    MIB_TXPKTCNT = 4,
    MIB_RXPKTCNT,
    
    MIB_RXDPCNT,
    MIB_RXCRCCNT,
    MIB_RXFRAGCNT,
    
    MIB_TXBRDCNT,
    MIB_RXBRDCNT,
    
    MIB_TXMULTCNT,
    MIB_RXMULTCNT,
    
    MIB_TXUNICNT,
    MIB_RXUNICNT,
    
    MIB_RXSYMBLCNT = 15,
    MIB_END

}rtk_mib_counter_t;


typedef enum rtk_enable_e
{
    DISABLED = 0,
    ENABLED,
    RTK_ENABLE_END
} rtk_enable_t;

typedef uint32  rtk_fid_t;        /* filter id type */

/* ethernet address type */
typedef struct  rtk_mac_s
{
    uint8 octet[ETHER_ADDR_LEN];
} rtk_mac_t;

typedef enum rtk_filter_act_enable_e
{
    /* CVLAN */
    FILTER_ENACT_INGRESS_CVLAN_INDEX = 0,
    FILTER_ENACT_INGRESS_CVLAN_VID,

    /* SVLAN */
    FILTER_ENACT_EGRESS_SVLAN_INDEX,

    /* Policing and Logging */
    FILTER_ENACT_POLICING_0,   

    /* Forward */
    FILTER_ENACT_TRAP_CPU,
    FILTER_ENACT_COPY_CPU,
    FILTER_ENACT_REDIRECT,
    FILTER_ENACT_DROP,    
    FILTER_ENACT_MIRROR,    
    FILTER_ENACT_ADD_DSTPORT,    

    /* QoS */
    FILTER_ENACT_PRIORITY,

    FILTER_ENACT_MAX
} rtk_filter_act_enable_t;
 
typedef struct rtk_filter_action_s
{
    rtk_filter_act_enable_t actEnable[FILTER_ENACT_MAX];

    /* CVLAN acton */
    uint32        filterIngressCvlanIdx;
    uint32        filterIngressCvlanVid;

    /* SVLAN action */
    uint32        filterEgressSvlanIdx; 

    /* Policing action */
    uint32        filterPolicingIdx[FILTER_POLICING_MAX];

    /* Forwarding action */
    uint32        filterRedirectPortmask;
    uint32        filterAddDstPortmask;  
    
    /* QOS action */
    uint32        filterPriority;    
} rtk_filter_action_t;

typedef struct rtk_filter_flag_s
{
    uint32 value;
    uint32 mask;
} rtk_filter_flag_t;

typedef enum rtk_filter_care_tag_index_e
{
    CARE_TAG_CTAG = 0,
    CARE_TAG_STAG,
    CARE_TAG_PPPOE,
    CARE_TAG_IPV4,
    CARE_TAG_IPV6,
    CARE_TAG_TCP,
    CARE_TAG_UDP,    
    CARE_TAG_ICMP,    
    CARE_TAG_IGMP,    
    CARE_TAG_MAX
} rtk_filter_care_tag_index_t;

typedef struct
{
    rtk_filter_flag_t tagType[CARE_TAG_MAX];
} rtk_filter_care_tag_t;

typedef struct rtk_filter_field rtk_filter_field_t;

typedef enum rtk_filter_field_data_type_e
{
    FILTER_FIELD_DATA_MASK = 0,
    FILTER_FIELD_DATA_RANGE,        
    FILTER_FIELD_DATA_MAX
} rtk_filter_field_data_type;

typedef struct
{
    uint32 dataType;
    uint32 rangeStart;
    uint32 rangeEnd;
    uint32 value;
    uint32 mask;
} rtk_filter_ip_t;

typedef struct
{
    uint32 dataType;
    rtk_mac_t value;
    rtk_mac_t mask;
    rtk_mac_t rangeStart;
    rtk_mac_t rangeEnd;
} rtk_filter_mac_t;

typedef uint32 rtk_filter_op_t;

typedef struct rtk_filter_value_s
{
    uint32 dataType;
    uint32 value;
    uint32 mask;
    uint32 rangeStart;
    uint32 rangeEnd;
} rtk_filter_value_t;

typedef struct rtk_filter_tag_s
{
    rtk_filter_value_t pri;
    rtk_filter_flag_t cfi;
    rtk_filter_value_t vid;
} rtk_filter_tag_t;

typedef struct
{
    rtk_filter_flag_t mf;
    rtk_filter_flag_t df;
} rtk_filter_ipFlag_t;

typedef struct
{
    uint32 addr[RTK_IPV6_ADDR_WORD_LENGTH];
} rtk_filter_ip6_addr_t;

typedef struct
{
    uint32 dataType;
    rtk_filter_ip6_addr_t value;
    rtk_filter_ip6_addr_t mask;
    rtk_filter_ip6_addr_t rangeStart;
    rtk_filter_ip6_addr_t rangeEnd;
} rtk_filter_ip6_t;

typedef uint32 rtk_filter_number_t;

typedef struct
{
    uint32 value[FILTER_PATTERN_MAX];
    uint32 mask[FILTER_PATTERN_MAX];
} rtk_filter_pattern_t;

typedef struct
{
    rtk_filter_flag_t urg;
    rtk_filter_flag_t ack;
    rtk_filter_flag_t psh;
    rtk_filter_flag_t rst;
    rtk_filter_flag_t syn;
    rtk_filter_flag_t fin;
    rtk_filter_flag_t ns;
    rtk_filter_flag_t cwr;
    rtk_filter_flag_t ece;    
} rtk_filter_tcpFlag_t;


typedef uint32 rtk_filter_field_raw_t;

struct rtk_filter_field
{
    uint32 fieldType;
    
    union
    {
        /* L2 struct */
        rtk_filter_mac_t       dmac;
        rtk_filter_mac_t       smac;
        rtk_filter_value_t     etherType;
        rtk_filter_tag_t       ctag;
        rtk_filter_tag_t       relayCtag;
        rtk_filter_tag_t       stag;
        rtk_filter_dot1as_timestamp_t dot1asTimeStamp;
        
        /* L3 struct */
        rtk_filter_ip_t      sip;
        rtk_filter_ip_t      dip;
        rtk_filter_value_t   protocol;
        rtk_filter_value_t   ipTos;
        rtk_filter_ipFlag_t  ipFlag;
        rtk_filter_value_t   ipOffset;
        rtk_filter_ip6_t     sipv6;
        rtk_filter_ip6_t     dipv6;
        rtk_filter_value_t   ipv6TrafficClass;        
        rtk_filter_value_t   ipv6NextHeader;
        rtk_filter_value_t   flowLabel;
        
        /* L4 struct */
        rtk_filter_value_t   tcpSrcPort;
        rtk_filter_value_t   tcpDstPort;
        rtk_filter_tcpFlag_t tcpFlag;
        rtk_filter_value_t   tcpSeqNumber;
        rtk_filter_value_t   tcpAckNumber;
        rtk_filter_value_t   udpSrcPort;
        rtk_filter_value_t   udpDatcPort;
        rtk_filter_value_t   icmpCode;
        rtk_filter_value_t   icmpType;
        rtk_filter_value_t   igmpType;

        /* pattern match */
        rtk_filter_pattern_t pattern;
    } filter_pattern_union;

    struct rtk_filter_field *next;
};

typedef enum rtk_filter_field_type_e
{
    FILTER_FIELD_DMAC = 0,
    FILTER_FIELD_SMAC,
    FILTER_FIELD_ETHERTYPE,
    FILTER_FIELD_CTAG,
    FILTER_FIELD_STAG, 
    
    FILTER_FIELD_IPV4_SIP,
    FILTER_FIELD_IPV4_DIP,
    FILTER_FIELD_IPV4_TOS,
    FILTER_FIELD_IPV4_PROTOCOL,
    FILTER_FIELD_IPV4_FLAG,
    FILTER_FIELD_IPV4_OFFSET,
    FILTER_FIELD_IPV6_SIPV6,
    FILTER_FIELD_IPV6_DIPV6,
    FILTER_FIELD_IPV6_TRAFFIC_CLASS,
    FILTER_FIELD_IPV6_NEXT_HEADER,
    
    FILTER_FIELD_TCP_SPORT,
    FILTER_FIELD_TCP_DPORT,
    FILTER_FIELD_TCP_FLAG,
    FILTER_FIELD_UDP_SPORT,
    FILTER_FIELD_UDP_DPORT,
    FILTER_FIELD_ICMP_CODE,
    FILTER_FIELD_ICMP_TYPE,
    FILTER_FIELD_IGMP_TYPE,
    FILTER_FIELD_MAX
} rtk_filter_field_type_t;

typedef enum rtk_filter_field_type_raw_e
{
    FILTER_FIELD_RAW_DMAC_47_32 = 0,
    FILTER_FIELD_RAW_DMAC_31_16,
    FILTER_FIELD_RAW_DMAC_15_0,    
    FILTER_FIELD_RAW_SMAC_47_32 = 0,
    FILTER_FIELD_RAW_SMAC_31_16,
    FILTER_FIELD_RAW_SMAC_15_0,    
    FILTER_FIELD_RAW_ETHERTYPE,
    FILTER_FIELD_RAW_CTAG,
    FILTER_FIELD_RAW_STAG, 
    
    FILTER_FIELD_RAW_IPV4_SIP_31_16,
    FILTER_FIELD_RAW_IPV4_SIP_15_0,    
    FILTER_FIELD_RAW_IPV4_DIP_31_16,
    FILTER_FIELD_RAW_IPV4_DIP_15_0,    
    FILTER_FIELD_RAW_IPV4_TOS_PROTOCOL,
    FILTER_FIELD_RAW_IPV4_FLAG_OFFSET,
    FILTER_FIELD_RAW_IPV6_SIP_127_112,
    FILTER_FIELD_RAW_IPV6_SIP_111_96,    
    FILTER_FIELD_RAW_IPV6_SIP_95_80,    
    FILTER_FIELD_RAW_IPV6_SIP_79_64,
    FILTER_FIELD_RAW_IPV6_SIP_63_48,
    FILTER_FIELD_RAW_IPV6_SIP_47_32,
    FILTER_FIELD_RAW_IPV6_SIP_31_16,
    FILTER_FIELD_RAW_IPV6_SIP_15_0,
    FILTER_FIELD_RAW_IPV6_DIP_127_112,
    FILTER_FIELD_RAW_IPV6_DIP_111_96,    
    FILTER_FIELD_RAW_IPV6_DIP_95_80,    
    FILTER_FIELD_RAW_IPV6_DIP_79_64,
    FILTER_FIELD_RAW_IPV6_DIP_63_48,
    FILTER_FIELD_RAW_IPV6_DIP_47_32,
    FILTER_FIELD_RAW_IPV6_DIP_31_16,
    FILTER_FIELD_RAW_IPV6_DIP_15_0,
    FILTER_FIELD_RAW_IPV6_TRAFFIC_CLASS_NEXT_HEADER,
    
    FILTER_FIELD_RAW_L4_SPORT,
    FILTER_FIELD_RAW_L4_DPORT,
    FILTER_FIELD_RAW_TCP_FLAG,
    FILTER_FIELD_RAW_ICMP_CODE_TYPE,
    FILTER_FIELD_RAW_IGMP_TYPE,
    FILTER_FIELD_RAW_MAX
} rtk_filter_field_type_raw_t;


typedef enum rtk_filter_flag_care_type_e
{
    FILTER_FLAG_CARE_DONT_CARE = 0,
    FILTER_FLAG_CARE_1,
    FILTER_FLAG_CARE_0,
    FILTER_FLAG_END
} rtk_filter_flag_care_type_t;

typedef uint32  rtk_filter_id_t;    /* filter id type */

typedef enum rtk_filter_invert_e
{
    FILTER_INVERT_DISABLE = 0,
    FILTER_INVERT_ENABLE,
    FILTER_INVERT_END
} rtk_filter_invert_t;

typedef uint32 rtk_filter_port_t;

typedef uint32 rtk_filter_state_t;

typedef uint32 rtk_filter_unmatch_action_t;

typedef enum rtk_filter_unmatch_action_e
{
    FILTER_UNMATCH_DROP = 0,
    FILTER_UNMATCH_PERMIT,
    FILTER_UNMATCH_END
} rtk_filter_unmatch_action_type_t;

typedef struct
{
    rtk_filter_field_t      *fieldHead;
    rtk_filter_care_tag_t   careTag;
    rtk_filter_value_t      activeport;

    rtk_filter_invert_t     invert;
} rtk_filter_cfg_t;

typedef struct
{
    rtk_filter_field_raw_t      dataFieldRaw[RTK_FILTER_RAW_FIELD_NUMBEER];     
    rtk_filter_field_raw_t      careFieldRaw[RTK_FILTER_RAW_FIELD_NUMBEER];
    rtk_filter_field_type_raw_t fieldRawType[RTK_FILTER_RAW_FIELD_NUMBEER];
    rtk_filter_care_tag_t       careTag;
    rtk_filter_value_t          activeport;
    rtk_filter_invert_t         invert;
    rtk_enable_t                valid;
} rtk_filter_cfg_raw_t;

typedef enum rtk_igmp_type_e
{
    IGMP_IPV4 = 0,
    IGMP_PPPOE_IPV4,
    IGMP_MLD,
    IGMP_PPPOE_MLD,
    IGMP_TYPE_END
} rtk_igmp_type_t;

typedef uint32 rtk_int_info_t;

typedef enum rtk_int_interrupt_type_e
{
    INT_TYPE_LINK_STATUS = 0,
    INT_TYPE_METER_EXCEED,
    INT_TYPE_LEARN_LIMIT,    
    INT_TYPE_LINK_SPEED,
    INT_TYPE_CONGEST,
    INT_TYPE_GREEN_FEATURE,
    INT_TYPE_LOOP_DETECT,
    INT_TYPE_8051,
    INT_TYPE_END
}rtk_int_interrupt_type_t;

typedef enum rtk_int_advType_e
{
    ADV_L2_LEARN_PORT_MASK = 0,
    ADV_SPEED_CHANGE_PORT_MASK,
    ADV_SPECIAL_CONGESTION_PORT_MASK,
    ADV_PORT_LINKDOWN_PORT_MASK,
    ADV_PORT_LINKUP_PORT_MASK,
    ADV_METER0_15_MASK,
    ADV_METER16_31_MASK,
    ADV_METER32_47,_MASK,
    ADV_METER48_63_MASK,
    ADV_END
} rtk_int_advType_t;

typedef enum rtk_int_polarity_e
{
    INT_POLAR_HIGH = 0,
    INT_POLAR_LOW,
    INT_POLAR_END
} rtk_int_polarity_t;

typedef struct  rtk_int_status_s
{
    uint8 value[RTK_MAX_NUM_OF_INTERRUPT_TYPE];
} rtk_int_status_t;

typedef enum rtk_int_type_e
{
    INT_LINK_CHANGE = 0,
    INT_SPEED_CHANGE,
    INT_RLDP,
    INT_METER,
    INT_LEARN_LIMIT,
    INT_END
} rtk_int_type_t;

typedef uint32 rtk_ipaddr_t;

typedef enum rtk_l2_age_time_e
{
    AGE_TIME_OUT = 0,
    AGE_TIME_100S = 100,
    AGE_TIME_200S = 200,
    AGE_TIME_300S = 300, 
    AGE_TIME_END
} rtk_l2_age_time_t;

typedef enum rtk_l2_flood_type_e
{
    FLOOD_UNKNOWNDA = 0,
    FLOOD_UNKNOWNMC,
    FLOOD_BC,
    FLOOD_END
} rtk_l2_flood_type_t;

typedef uint32 rtk_l2_flushItem_t;

typedef enum rtk_l2_flushType_e
{
    FLUSH_TYPE_BY_PORT = 0,       /* physical port       */
    FLUSH_TYPE_END
} rtk_l2_flushType_t;


typedef enum rtk_l2_hash_method_e
{
    HASH_OPT0 = 0,
    HASH_OPT1,
    HASH_END
} rtk_hash_method_t;

/* l2 limit learning count action */
typedef enum rtk_l2_limitLearnCntAction_e
{
    LIMIT_LEARN_CNT_ACTION_DROP = 0,
    LIMIT_LEARN_CNT_ACTION_FLOOD,
    LIMIT_LEARN_CNT_ACTION_TO_CPU,
    LIMIT_LEARN_CNT_ACTION_END
} rtk_l2_limitLearnCntAction_t;

typedef enum rtk_l2_lookup_type_e
{
    LOOKUP_MAC = 0,
    LOOKUP_SIP_DIP,
    LOOKUP_DIP,  
    LOOKUP_END
} rtk_l2_lookup_type_t;

/* l2 address table - unicast data structure */

typedef enum rtk_leaky_type_e
{
    LEAKY_BRG_GROUP = 0,
    LEAKY_FD_PAUSE,
    LEAKY_SP_MCAST,
    LEAKY_1X_PAE,
    LEAKY_UNDEF_BRG_04,
    LEAKY_UNDEF_BRG_05,
    LEAKY_UNDEF_BRG_06,
    LEAKY_UNDEF_BRG_07,    
    LEAKY_PROVIDER_BRIDGE_GROUP_ADDRESS,
    LEAKY_UNDEF_BRG_09,
    LEAKY_UNDEF_BRG_0A,
    LEAKY_UNDEF_BRG_0B,
    LEAKY_UNDEF_BRG_0C,    
    LEAKY_PROVIDER_BRIDGE_GVRP_ADDRESS,    
    LEAKY_8021AB,
    LEAKY_UNDEF_BRG_0F,    
    LEAKY_BRG_MNGEMENT,
    LEAKY_UNDEFINED_11,
    LEAKY_UNDEFINED_12,
    LEAKY_UNDEFINED_13,
    LEAKY_UNDEFINED_14,
    LEAKY_UNDEFINED_15,
    LEAKY_UNDEFINED_16,
    LEAKY_UNDEFINED_17,
    LEAKY_UNDEFINED_18,
    LEAKY_UNDEFINED_19,
    LEAKY_UNDEFINED_1A,
    LEAKY_UNDEFINED_1B,
    LEAKY_UNDEFINED_1C,
    LEAKY_UNDEFINED_1D,
    LEAKY_UNDEFINED_1E,
    LEAKY_UNDEFINED_1F,
    LEAKY_GMRP,
    LEAKY_GVRP,
    LEAKY_UNDEF_GARP_22,
    LEAKY_UNDEF_GARP_23,
    LEAKY_UNDEF_GARP_24,
    LEAKY_UNDEF_GARP_25,
    LEAKY_UNDEF_GARP_26,
    LEAKY_UNDEF_GARP_27,
    LEAKY_UNDEF_GARP_28,
    LEAKY_UNDEF_GARP_29,
    LEAKY_UNDEF_GARP_2A,
    LEAKY_UNDEF_GARP_2B,
    LEAKY_UNDEF_GARP_2C,
    LEAKY_UNDEF_GARP_2D,
    LEAKY_UNDEF_GARP_2E,
    LEAKY_UNDEF_GARP_2F,
    LEAKY_IGMP,
    LEAKY_IPMULTICAST,
    LEAKY_END
}rtk_leaky_type_t;

typedef enum rtk_led_blink_rate_e
{
    LED_BLINKRATE_32MS=0,         
    LED_BLINKRATE_64MS,        
    LED_BLINKRATE_128MS,
    LED_BLINKRATE_256MS,
    LED_BLINKRATE_512MS,
    LED_BLINKRATE_1024MS,
    LED_BLINKRATE_48MS,
    LED_BLINKRATE_96MS,
    LED_BLINKRATE_END
}rtk_led_blink_rate_t;

typedef enum rtk_led_group_e
{
    LED_GROUP_0 = 0,
    LED_GROUP_1,
    LED_GROUP_2,
    LED_GROUP_END
}rtk_led_group_t;    

typedef enum rtk_led_mode_e
{
    LED_MODE_0 = 0,
    LED_MODE_1,
    LED_MODE_2,
    LED_MODE_3,
    LED_MODE_END
}rtk_led_mode_t;    


typedef enum rtk_led_force_mode_e
{

    LED_FORCE_NORMAL=0,
    LED_FORCE_BLINK,
    LED_FORCE_OFF,
    LED_FORCE_ON,
    LED_FORCE_END
}rtk_led_force_mode_t;

typedef uint32  rtk_mac_cnt_t;     /* meter id type  */

typedef enum rtk_mcast_type_e
{
    MCAST_L2 = 0,
    MCAST_IPV4,
    MCAST_IPV6,
    MCAST_END
} rtk_mcast_type_t;

typedef uint32  rtk_meter_id_t;     /* meter id type  */

typedef uint32  rtk_mode_t; 

typedef uint32  rtk_port_t;        /* port is type */

typedef enum rtk_port_duplex_e
{
    PORT_HALF_DUPLEX = 0,
    PORT_FULL_DUPLEX,
    PORT_DUPLEX_END
} rtk_port_duplex_t;

typedef enum rtk_port_linkStatus_e
{
    PORT_LINKDOWN = 0,
    PORT_LINKUP,
    PORT_LINKSTATUS_END
} rtk_port_linkStatus_t;
	
typedef struct  rtk_port_mac_ability_s
{
    uint32 speed;
    uint32 duplex;
    uint32 link;    
    uint32 nway;    
    uint32 txpause;
    uint32 rxpause;      
}rtk_port_mac_ability_t;

typedef struct rtk_port_phy_ability_s
{   
    uint32    AutoNegotiation;  /*PHY register 0.12 setting for auto-negotiation process*/
    uint32    Half_10;          /*PHY register 4.5 setting for 10BASE-TX half duplex capable*/
    uint32    Full_10;          /*PHY register 4.6 setting for 10BASE-TX full duplex capable*/
    uint32    Half_100;         /*PHY register 4.7 setting for 100BASE-TX half duplex capable*/
    uint32    Full_100;         /*PHY register 4.8 setting for 100BASE-TX full duplex capable*/
    uint32    Full_1000;        /*PHY register 9.9 setting for 1000BASE-T full duplex capable*/
    uint32    FC;               /*PHY register 4.10 setting for flow control capability*/
    uint32    AsyFC;            /*PHY register 4.11 setting for  asymmetric flow control capability*/
} rtk_port_phy_ability_t;

typedef uint32  rtk_port_phy_data_t;     /* phy page  */

typedef uint32  rtk_port_phy_page_t;     /* phy page  */

typedef enum rtk_port_phy_reg_e  
{
    PHY_REG_CONTROL             = 0,
    PHY_REG_STATUS,
    PHY_REG_IDENTIFIER_1,
    PHY_REG_IDENTIFIER_2,
    PHY_REG_AN_ADVERTISEMENT,
    PHY_REG_AN_LINKPARTNER,
    PHY_REG_1000_BASET_CONTROL  = 9,
    PHY_REG_1000_BASET_STATUS,
    PHY_REG_END                 = 32
} rtk_port_phy_reg_t;

typedef enum rtk_port_phy_test_mode_e  
{
    PHY_TEST_MODE_NORMAL= 0,
    PHY_TEST_MODE_1,
    PHY_TEST_MODE_2,
    PHY_TEST_MODE_3,
    PHY_TEST_MODE_4,
    PHY_TEST_MODE_END           
} rtk_port_phy_test_mode_t;

typedef enum rtk_port_speed_e
{
    PORT_SPEED_10M = 0,
	PORT_SPEED_20M,	/*add for RTL8309N*/
    PORT_SPEED_100M,
    PORT_SPEED_200M,/*add for RTL8309N*/
    PORT_SPEED_1000M,
    PORT_SPEED_END
} rtk_port_speed_t;

typedef struct rtk_portmask_s
{
    uint32  bits[RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST];
} rtk_portmask_t;

typedef uint32  rtk_pri_t;         /*priority value */
typedef uint32  rtk_dei_t;        /*dei value*/

typedef uint32  rtk_qid_t;        /* queue id type */

typedef struct rtk_qos_pri2queue_s
{
    uint32 pri2queue[RTK_MAX_NUM_OF_INTERNAL_PRIORITY];
} rtk_qos_pri2queue_t;

typedef struct rtk_qos_queue_weights_s
{
    uint32 weights[RTK_MAX_NUM_OF_QUEUE];
} rtk_qos_queue_weights_t;

typedef enum rtk_qos_scheduling_type_e
{
    WFQ = 0,        /* Weighted-Fair-Queue */
    WRR,            /* Weighted-Round-Robin */
    SCHEDULING_TYPE_END
} rtk_qos_scheduling_type_t;

typedef uint32  rtk_queue_num_t;    /* queue number*/

typedef enum rtk_rate_storm_group_e
{
    STORM_GROUP_UNKNOWN_UNICAST = 0,
    STORM_GROUP_UNKNOWN_MULTICAST,
    STORM_GROUP_MULTICAST,
    STORM_GROUP_BROADCAST,
    STORM_GROUP_END
} rtk_rate_storm_group_t;

typedef uint32  rtk_rate_t;     /* rate type  */

typedef rtk_u_long_t rtk_stat_counter_t;

#ifndef EMBEDDED_SUPPORT
/* global statistic counter structure */
typedef struct rtk_stat_global_cntr_s
{
    uint64 dot1dTpLearnedEntryDiscards;
}rtk_stat_global_cntr_t;
#endif

typedef enum rtk_stat_global_type_e
{
    DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX = 36,
    MIB_GLOBAL_CNTR_END
}rtk_stat_global_type_t;

#ifndef EMBEDDED_SUPPORT
/* port statistic counter structure */
typedef struct rtk_stat_port_cntr_s
{
    uint64 ifInOctets;
    uint32 dot3StatsFCSErrors;
    uint32 dot3StatsSymbolErrors;
    uint32 dot3InPauseFrames;
    uint32 dot3ControlInUnknownOpcodes;
    uint32 etherStatsFragments;
    uint32 etherStatsJabbers;
    uint32 ifInUcastPkts;
    uint32 etherStatsDropEvents;
    uint64 etherStatsOctets;
    uint32 etherStatsUndersizePkts;
    uint32 etherStatsOversizePkts;
    uint32 etherStatsPkts64Octets;
    uint32 etherStatsPkts65to127Octets;
    uint32 etherStatsPkts128to255Octets;
    uint32 etherStatsPkts256to511Octets;
    uint32 etherStatsPkts512to1023Octets;
    uint32 etherStatsPkts1024toMaxOctets;
    uint32 etherStatsMcastPkts;
    uint32 etherStatsBcastPkts;
    uint64 ifOutOctets;
    uint32 dot3StatsSingleCollisionFrames;
    uint32 dot3StatsMultipleCollisionFrames;
    uint32 dot3StatsDeferredTransmissions;
    uint32 dot3StatsLateCollisions;
    uint32 etherStatsCollisions;
    uint32 dot3StatsExcessiveCollisions;
    uint32 dot3OutPauseFrames;
    uint32 dot1dBasePortDelayExceededDiscards;
    uint32 dot1dTpPortInDiscards;
    uint32 ifOutUcastPkts;
    uint32 ifOutMulticastPkts;
    uint32 ifOutBrocastPkts;
    uint32 outOampduPkts;
    uint32 inOampduPkts;
    uint32 pktgenPkts;
}rtk_stat_port_cntr_t;
#endif

/* port statistic counter index */
typedef enum rtk_stat_port_type_e
{
    STAT_IfInOctets = 0,
    STAT_Dot3StatsFCSErrors,
    STAT_Dot3StatsSymbolErrors,
    STAT_Dot3InPauseFrames,
    STAT_Dot3ControlInUnknownOpcodes,        
    STAT_EtherStatsFragments,
    STAT_EtherStatsJabbers,
    STAT_IfInUcastPkts,
    STAT_EtherStatsDropEvents,
    STAT_EtherStatsOctets,
    STAT_EtherStatsUnderSizePkts,
    STAT_EtherOversizeStats,
    STAT_EtherStatsPkts64Octets,
    STAT_EtherStatsPkts65to127Octets,
    STAT_EtherStatsPkts128to255Octets,
    STAT_EtherStatsPkts256to511Octets,
    STAT_EtherStatsPkts512to1023Octets,
    STAT_EtherStatsPkts1024to1518Octets,
    STAT_EtherStatsMulticastPkts,
    STAT_EtherStatsBroadcastPkts,    
    STAT_IfOutOctets,
    STAT_Dot3StatsSingleCollisionFrames,
    STAT_Dot3StatsMultipleCollisionFrames,
    STAT_Dot3StatsDeferredTransmissions,
    STAT_Dot3StatsLateCollisions,
    STAT_EtherStatsCollisions,
    STAT_Dot3StatsExcessiveCollisions,
    STAT_Dot3OutPauseFrames,
    STAT_Dot1dBasePortDelayExceededDiscards,
    STAT_Dot1dTpPortInDiscards,
    STAT_IfOutUcastPkts,
    STAT_IfOutMulticastPkts,
    STAT_IfOutBroadcastPkts,
    STAT_OutOampduPkts,
    STAT_InOampduPkts,
    STAT_PktgenPkts,
    STAT_PORT_CNTR_END
}rtk_stat_port_type_t;

typedef uint32  rtk_stg_t;        /* spanning tree instance id type */

typedef enum rtk_storm_bypass_e
{
    BYPASS_BRG_GROUP = 0,
    BYPASS_FD_PAUSE,
    BYPASS_SP_MCAST,
    BYPASS_1X_PAE,
    BYPASS_UNDEF_BRG_04,
    BYPASS_UNDEF_BRG_05,
    BYPASS_UNDEF_BRG_06,
    BYPASS_UNDEF_BRG_07,    
    BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
    BYPASS_UNDEF_BRG_09,
    BYPASS_UNDEF_BRG_0A,
    BYPASS_UNDEF_BRG_0B,
    BYPASS_UNDEF_BRG_0C,    
    BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,    
    BYPASS_8021AB,
    BYPASS_UNDEF_BRG_0F,    
    BYPASS_BRG_MNGEMENT,
    BYPASS_UNDEFINED_11,
    BYPASS_UNDEFINED_12,
    BYPASS_UNDEFINED_13,
    BYPASS_UNDEFINED_14,
    BYPASS_UNDEFINED_15,
    BYPASS_UNDEFINED_16,
    BYPASS_UNDEFINED_17,
    BYPASS_UNDEFINED_18,
    BYPASS_UNDEFINED_19,
    BYPASS_UNDEFINED_1A,
    BYPASS_UNDEFINED_1B,
    BYPASS_UNDEFINED_1C,
    BYPASS_UNDEFINED_1D,
    BYPASS_UNDEFINED_1E,
    BYPASS_UNDEFINED_1F,
    BYPASS_GMRP,
    BYPASS_GVRP,
    BYPASS_UNDEF_GARP_22,
    BYPASS_UNDEF_GARP_23,
    BYPASS_UNDEF_GARP_24,
    BYPASS_UNDEF_GARP_25,
    BYPASS_UNDEF_GARP_26,
    BYPASS_UNDEF_GARP_27,
    BYPASS_UNDEF_GARP_28,
    BYPASS_UNDEF_GARP_29,
    BYPASS_UNDEF_GARP_2A,
    BYPASS_UNDEF_GARP_2B,
    BYPASS_UNDEF_GARP_2C,
    BYPASS_UNDEF_GARP_2D,
    BYPASS_UNDEF_GARP_2E,
    BYPASS_UNDEF_GARP_2F,
    BYPASS_IGMP,
    BYPASS_END
}rtk_storm_bypass_t;

typedef uint32  rtk_stp_msti_id_t;     /* MSTI ID  */

typedef enum rtk_stp_state_e
{
    STP_STATE_DISABLED = 0,
    STP_STATE_BLOCKING,
    STP_STATE_LEARNING,
    STP_STATE_FORWARDING,
    STP_STATE_END
} rtk_stp_state_t;

typedef uint32 rtk_svlan_index_t;

typedef struct rtk_svlan_memberCfg_s{
    uint32 svid;   
    uint32 memberport;
    uint32 fid;
    uint32 priority;
    uint32 reserved1;     
    uint32 reserved2;    
    uint32 reserved3; 
    uint32 reserved4;   
}rtk_svlan_memberCfg_t;

typedef enum rtk_svlan_pri_ref_e
{
    REF_INTERNAL_PRI = 0,
    REF_CTAG_PRI,
    REF_SVLAN_PRI,
    REF_PRI_END
} rtk_svlan_pri_ref_t;


typedef uint32 rtk_svlan_tpid_t;
typedef uint32 rtk_switch_len_t;

typedef enum rtk_switch_maxPktLen_e
{
	MAX_PKTLEN_1518B = 0, /*add for RTL8309N*/
    MAX_PKTLEN_1522B,	
    MAX_PKTLEN_1526B,	/*add for RTL8309N*/
    MAX_PKTLEN_1536B,
    MAX_PKTLEN_1552B,
    MAX_PKTLEN_2000B,
    MAX_PKTLEN_2048B,	/*add for RTL8309N*/
    MAX_PKTLEN_16000B, 
    MAX_PKTLEN_USER,	/*add for RTL8309N*/
    MAX_PKTLEN_END   
} rtk_switch_maxPktLen_t;

typedef enum rtk_trap_igmp_action_e
{
    IGMP_ACTION_FORWARD = 0,
	IGMP_ACTION_COPY2CPU, /*add at 2011-10-28, for RTL8309N*/
    IGMP_ACTION_TRAP2CPU,
    IGMP_ACTION_DROP,
    IGMP_ACTION_FORWARD_EXCLUDE_CPU,
    IGMP_ACTION_END
} rtk_trap_igmp_action_t;

typedef enum rtk_trap_mcast_action_e
{
    MCAST_ACTION_FORWARD = 0,
    MCAST_ACTION_DROP,
    MCAST_ACTION_END
} rtk_trap_mcast_action_t;

typedef enum rtk_trap_reason_type_e
{
    TRAP_REASON_RMA = 0,
    TRAP_REASON_IPV4IGMP,
    TRAP_REASON_IPV6MLD,
    TRAP_REASON_1XEAPOL,
    TRAP_REASON_VLANERR,
    TRAP_REASON_SLPCHANGE,
    TRAP_REASON_MULTICASTDLF,
    TRAP_REASON_CFI,
    TRAP_REASON_1XUNAUTH,
    TRAP_REASON_END
} rtk_trap_reason_type_t;


typedef enum rtk_trap_rma_action_e
{
    RMA_ACTION_FORWARD = 0,
    RMA_ACTION_TRAP2CPU,
    RMA_ACTION_DROP,
    RMA_ACTION_FORWARD_EXCLUDE_CPU,
    RMA_ACTION_END
} rtk_trap_rma_action_t;

typedef enum rtk_trap_ucast_action_e
{
    UCAST_ACTION_FORWARD = 0,
    UCAST_ACTION_DROP,
    UCAST_ACTION_TRAP2CPU,
    UCAST_ACTION_END
} rtk_trap_ucast_action_t;

typedef enum rtk_trap_ucast_type_e
{
    UCAST_UNKNOWNDA = 0,
    UCAST_UNKNOWNSA,
    UCAST_UNMATCHSA,
    UCAST_END
} rtk_trap_ucast_type_t;

/*add for RTL8309N only*/
typedef enum rtk_acl_action_e
{
	ACL_ACT_DROP = 0,
	ACL_ACT_PERMIT,
	ACL_ACT_TRAP2CPU,
	ACL_ACT_MIRROR,
	ACL_ACT_END
}rtk_acl_action_t;

typedef enum rtk_acl_protocol_e
{
	ACL_PRO_ETHERTYPE = 0,
	ACL_PRO_TCP,
	ACL_PRO_UDP,
	ACL_PRO_TCPUDP,
	ACL_PRO_END
}rtk_acl_protocol_t;


typedef enum rtk_trunk_group_e
{
    TRUNK_GROUP0 = 0,
    TRUNK_GROUP1,
    TRUNK_GROUP2,
    TRUNK_GROUP3,
    TRUNK_GROUP_END
} rtk_trunk_group_t;

typedef enum rtk_cpu_tagTrapPkt_e
{
    CPU_TRAP_DOT1X = 0,
    CPU_TRAP_VID,
    CPU_TRAP_ISPMAC,
    CPU_TRAP_ACL,
    CPU_TRAP_IGMPMLD,
    CPU_TRAP_RMA,
    CPU_TRAP_EXMACLIMIT,
    CPU_TRAP_RLPP,
    CPU_TRAP_LUTMACBLOCK,
    CPU_TRAP_ALL,
    CPU_TRAP_END     
}rtk_cpu_tagTrapPkt_t;

typedef struct  rtk_trunk_hashVal2Port_s
{
    uint8 value[RTK_MAX_NUM_OF_TRUNK_HASH_VAL];
} rtk_trunk_hashVal2Port_t;

typedef uint32  rtk_vlan_proto_type_t;     /* protocol and port based VLAN protocol type  */


typedef enum rtk_vlan_acceptFrameType_e
{
    ACCEPT_FRAME_TYPE_ALL = 0,             /* untagged, priority-tagged and tagged */
    ACCEPT_FRAME_TYPE_TAG_ONLY,         /* tagged */
    ACCEPT_FRAME_TYPE_UNTAG_ONLY,     /* untagged and priority-tagged */
    ACCEPT_FRAME_TYPE_END
} rtk_vlan_acceptFrameType_t;


/* frame type of protocol vlan - reference 802.1v standard */
typedef enum rtk_vlan_protoVlan_frameType_e
{
    FRAME_TYPE_ETHERNET = 0,
    FRAME_TYPE_LLCOTHER,
    FRAME_TYPE_RFC1042,
    FRAME_TYPE_END
} rtk_vlan_protoVlan_frameType_t;

typedef uint32  rtk_vlan_t;        /* vlan id type */

/* Protocol-and-port-based Vlan structure */
typedef struct rtk_vlan_protoAndPortInfo_s
{
    uint32                         proto_type;
    rtk_vlan_protoVlan_frameType_t frame_type;
    rtk_vlan_t                     cvid;
    rtk_pri_t                     cpri;
}rtk_vlan_protoAndPortInfo_t;

/* tagged mode of VLAN - reference realtek private specification */
typedef enum rtk_vlan_tagMode_e
{
    VLAN_TAG_MODE_ORIGINAL = 0,
    VLAN_TAG_MODE_KEEP_FORMAT,
    VLAN_TAG_MODE_REAL_KEEP_FORMAT,
    VLAN_TAG_MODE_PRI,
    VLAN_TAG_MODE_END
} rtk_vlan_tagMode_t;


/*some type only for RTL8309N*/
#define RTK_MAX_DSCP_VALUE 64
typedef enum rtk_priSrc_s
{
	RTK_PRISRC_PPRI = 0,			   /*port-based outer tag priority*/
	RTK_PRISRC_1PRMK,				  /* 1p remarking priority*/
	RTK_PRISRC_END
}rtk_priSrc_t;		 /*add at 2011-09-05, only RTL8309N*/

typedef enum rtk_qosPriSrc_s
{
	RTK_CPUTAG_PRI = 0,
	RTK_IP_PRI,
	RTK_DSCP_PRI,
	RTK_1Q_PRI,
	RTK_PORT_PRI, 
	RTK_QOS_PRISRC_END
}rtk_qosPriSrc_t; 	/*add at 2011-09-05, only RTL8309N*/

typedef struct rtk_l2_ucastAddr_s
{
	uint8 auth;
	uint8 da_block;
	uint8 sa_block;
	uint8 isStatic;
	uint8 port;
	uint8 fid;
	uint32 age;
	rtk_mac_t mac;
}rtk_l2_ucastAddr_t;

typedef struct rtk_priority_select_s
{
    uint32 acl_pri_weight;
	uint32 dot1q_pri_weight;
	uint32 port_pri_weight;
    uint32 dscp_pri_weight;
}rtk_priority_select_t;/*add at 2011-09-05, only RTL8309N*/

typedef	enum rtk_qos_flctype_e
{
	RTK_EGR_PDSCFLC = 0,
	RTK_EGR_QFLC,
	RTK_EGR_PFLC,
	RTK_IGR_FLC,
	RTK_QOS_FLCTYPE_END
}rtk_qos_flctype_t;

typedef struct rtk_filter_rule_e
{
    uint32 phyport;   /*the physical port where packet received*/
    uint32 protocol;  /*Acl protocol which is in the packet*/
    uint32 data;       /*16bit ether type or TCP/UDP source port, destination port value*/
    uint32 priority;   /*Acl priority assigned to the packet*/
    uint32 action;     /*How to deal with the packet*/
} rtk_filter_rule_t;
    
typedef struct rtk_storm_attr_e /*add at 2011-09-04*/
{
	uint32 unit;
	uint32 rate;
	uint32 burst;
	uint32 ifg_include;
}rtk_storm_attr_t;

typedef enum rtk_port_phy_mdix_mode_e
{
    PHY_AUTO_CROSSOVER_MODE= 0,
    PHY_FORCE_MDI_MODE,
    PHY_FORCE_MDIX_MODE,
    PHY_FORCE_MODE_END
} rtk_port_phy_mdix_mode_t;

typedef enum rtk_port_phy_mdix_status_e
{
    PHY_STATUS_AUTO_MDI_MODE= 0,
    PHY_STATUS_AUTO_MDIX_MODE,
    PHY_STATUS_FORCE_MDI_MODE,
    PHY_STATUS_FORCE_MDIX_MODE,
    PHY_STATUS_FORCE_MODE_END
} rtk_port_phy_mdix_status_t;

#endif  /*__RTK_API_H__*/




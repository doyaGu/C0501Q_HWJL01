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
* Purpose : Definition the error number in the SDK
*
* Feature :  This file consists of following modules:
*                
*
*/

#ifndef _RTL_ERROR_H_
#define _RTL_ERROR_H_

/*
 * Data Type Declaration
 */
typedef enum rt_error_code_e
{
    RT_ERR_FAILED = -1,                             /* General Error                                                                    */

    /* 0x00xx for common error code */
    RT_ERR_OK = 0,                                    /* 0x0000, OK                                                                   */
    RT_ERR_INPUT,                                     /* 0x0001, invalid input parameter                                              */
    RT_ERR_UNIT_ID,                                  /* 0x0002, invalid unit id                                                      */
    RT_ERR_PORT_ID,                                 /* 0x0003, invalid port id                                                      */
    RT_ERR_PORT_MASK,                            /* 0x0004, invalid port mask                                                    */
    RT_ERR_PORT_LINKDOWN,                    /* 0x0005, link down port status                                                */
    RT_ERR_ENTRY_INDEX,                         /* 0x0006, invalid entry index                                                  */
    RT_ERR_NULL_POINTER,                       /* 0x0007, input parameter is null pointer                                      */
    RT_ERR_QUEUE_ID,                             /* 0x0008, invalid queue id                                                     */
    RT_ERR_QUEUE_NUM,                               /* 0x0009, invalid queue number                                                 */
    RT_ERR_BUSYWAIT_TIMEOUT,                        /* 0x000a, busy watting time out                                                */
    RT_ERR_MAC,                                     /* 0x000b, invalid mac address                                                  */
    RT_ERR_OUT_OF_RANGE,                            /* 0x000c, input parameter out of range                                         */
    RT_ERR_CHIP_NOT_SUPPORTED,                      /* 0x000d, functions not supported by this chip model                           */
    RT_ERR_SMI,                                     /* 0x000e, SMI error                                                            */
    RT_ERR_NOT_INIT,                                /* 0x000f, The module is not initial                                            */
    RT_ERR_CHIP_NOT_FOUND,                          /* 0x0010, The chip can not found                                               */
    RT_ERR_NOT_ALLOWED,                             /* 0x0011, actions not allowed by the function                                  */
    RT_ERR_DRIVER_NOT_FOUND,                        /* 0x0012, The driver can not found                                             */
    RT_ERR_SEM_LOCK_FAILED,                         /* 0x0013, Failed to lock semaphore                                             */
    RT_ERR_SEM_UNLOCK_FAILED,                       /* 0x0014, Failed to unlock semaphore                                           */
    RT_ERR_ENABLE,                                  /* 0x0015, invalid enable parameter                                             */
    RT_ERR_TBL_FULL,                                /* 0x0016, input table full                                                     */

    /* 0x01xx for vlan */
    RT_ERR_VLAN_VID = 0x0100,                       /* 0x0100, invalid vid                                                          */
    RT_ERR_VLAN_PRIORITY,                           /* 0x0101, invalid 1p priority                                                  */
    RT_ERR_VLAN_EMPTY_ENTRY,                        /* 0x0102, emtpy entry of vlan table                                            */
    RT_ERR_VLAN_ACCEPT_FRAME_TYPE,                  /* 0x0103, invalid accept frame type                                            */
    RT_ERR_VLAN_EXIST,                              /* 0x0104, vlan is exist                                                        */
    RT_ERR_VLAN_ENTRY_NOT_FOUND,                    /* 0x0105, specified vlan entry not found                                       */
    RT_ERR_VLAN_PORT_MBR_EXIST,                     /* 0x0106, member port exist in the specified vlan                              */
    RT_ERR_VLAN_PROTO_AND_PORT,                     /* 0x0108, invalid protocol and port based vlan                              */

    /* 0x02xx for svlan */
    RT_ERR_SVLAN_ENTRY_INDEX = 0x0200,              /* 0x0200, invalid svid entry no                                                */
    RT_ERR_SVLAN_ETHER_TYPE,                        /* 0x0201, invalid SVLAN ether type                                             */
    RT_ERR_SVLAN_TABLE_FULL,                        /* 0x0202, no empty entry in SVLAN table                                        */
    RT_ERR_SVLAN_ENTRY_NOT_FOUND,                   /* 0x0203, specified svlan entry not found                                      */
    RT_ERR_SVLAN_EXIST,                             /* 0x0204, SVLAN entry is exist                                                 */
    RT_ERR_SVLAN_VID,                               /* 0x0205, invalid svid                                                         */

    /* 0x03xx for MSTP */
    RT_ERR_MSTI = 0x0300,                           /* 0x0300, invalid msti                                                         */
    RT_ERR_MSTP_STATE,                              /* 0x0301, invalid spanning tree status                                         */
    RT_ERR_MSTI_EXIST,                              /* 0x0302, MSTI exist                                                           */
    RT_ERR_MSTI_NOT_EXIST,                          /* 0x0303, MSTI not exist                                                       */

    /* 0x04xx for BUCKET */
    RT_ERR_TIMESLOT = 0x0400,                       /* 0x0400, invalid time slot                                                    */
    RT_ERR_TOKEN,                                   /* 0x0401, invalid token amount                                                 */
    RT_ERR_RATE,                                    /* 0x0402, invalid rate                                                         */
    RT_ERR_TICK,                                    /* 0x0403, invalid tick                                                 */

    /* 0x05xx for RMA */
    RT_ERR_RMA_ADDR = 0x0500,                       /* 0x0500, invalid rma mac address                                              */
    RT_ERR_RMA_ACTION,                              /* 0x0501, invalid rma action                                                   */

    /* 0x06xx for L2 */
    RT_ERR_L2_HASH_KEY = 0x0600,                    /* 0x0600, invalid L2 Hash key                                                  */
    RT_ERR_L2_HASH_INDEX,                           /* 0x0601, invalid L2 Hash index                                                */
    RT_ERR_L2_CAM_INDEX,                            /* 0x0602, invalid L2 CAM index                                                 */
    RT_ERR_L2_ENRTYSEL,                             /* 0x0603, invalid EntrySel                                                     */
    RT_ERR_L2_INDEXTABLE_INDEX,                     /* 0x0604, invalid L2 index table(=portMask table) index                        */
    RT_ERR_LIMITED_L2ENTRY_NUM,                     /* 0x0605, invalid limited L2 entry number                                      */
    RT_ERR_L2_AGGREG_PORT,                          /* 0x0606, this aggregated port is not the lowest physical
                                                               port of its aggregation group                                        */
    RT_ERR_L2_FID,                                  /* 0x0607, invalid fid                                                          */
    RT_ERR_L2_RVID,                                 /* 0x0608, invalid cvid                                                         */
    RT_ERR_L2_NO_EMPTY_ENTRY,                       /* 0x0609, no empty entry in L2 table                                           */
    RT_ERR_L2_ENTRY_NOTFOUND,                       /* 0x060a, specified entry not found                                            */
    RT_ERR_L2_INDEXTBL_FULL,                        /* 0x060b, the L2 index table is full                                           */
    RT_ERR_L2_INVALID_FLOWTYPE,                     /* 0x060c, invalid L2 flow type                                                 */
    RT_ERR_L2_L2UNI_PARAM,                          /* 0x060d, invalid L2 unicast parameter                                         */
    RT_ERR_L2_L2MULTI_PARAM,                        /* 0x060e, invalid L2 multicast parameter                                       */
    RT_ERR_L2_IPMULTI_PARAM,                        /* 0x060f, invalid L2 ip multicast parameter                                    */
    RT_ERR_L2_PARTIAL_HASH_KEY,                     /* 0x0610, invalid L2 partial Hash key                                          */
    RT_ERR_L2_EMPTY_ENTRY,                          /* 0x0611, the entry is empty(invalid)                                          */
    RT_ERR_L2_FLUSH_TYPE,                           /* 0x0612, the flush type is invalid                                            */
    RT_ERR_L2_NO_CPU_PORT,                          /* 0x0613, CPU port not exist                                                   */

    /* 0x07xx for FILTER (PIE) */
    RT_ERR_FILTER_BLOCKNUM = 0x0700,                /* 0x0700, invalid block number                                                 */
    RT_ERR_FILTER_ENTRYIDX,                         /* 0x0701, invalid entry index                                                  */
    RT_ERR_FILTER_CUTLINE,                          /* 0x0702, invalid cutline value                                                */
    RT_ERR_FILTER_FLOWTBLBLOCK,                     /* 0x0703, block belongs to flow table                                          */
    RT_ERR_FILTER_INACLBLOCK,                       /* 0x0704, block belongs to ingress ACL                                         */
    RT_ERR_FILTER_ACTION,                           /* 0x0705, action doesn't consist to entry type                                 */
    RT_ERR_FILTER_INACL_RULENUM,                    /* 0x0706, invalid ACL rulenum                                                  */
    RT_ERR_FILTER_INACL_TYPE,                       /* 0x0707, entry type isn't an ingress ACL rule                                 */
    RT_ERR_FILTER_INACL_EXIST,                      /* 0x0708, ACL entry is already exit                                            */
    RT_ERR_FILTER_INACL_EMPTY,                      /* 0x0709, ACL entry is empty                                                   */
    RT_ERR_FILTER_FLOWTBL_TYPE,                     /* 0x070a, entry type isn't an flow table rule                                  */
    RT_ERR_FILTER_FLOWTBL_RULENUM,                  /* 0x070b, invalid flow table rulenum                                           */
    RT_ERR_FILTER_FLOWTBL_EMPTY,                    /* 0x070c, flow table entry is empty                                            */
    RT_ERR_FILTER_FLOWTBL_EXIST,                    /* 0x070d, flow table entry is already exist                                    */
    RT_ERR_FILTER_METER_ID,                         /* 0x070e, invalid metering id                                                  */
    RT_ERR_FILTER_LOG_ID,                           /* 0x070f, invalid log id                                                       */
    RT_ERR_FILTER_INACL_ACT_NOT_SUPPORT,            /* 0x0710, rule not support                                                     */        
    RT_ERR_FILTER_INACL_RULE_NOT_SUPPORT,           /* 0x0710, action not support                                                   */    

    /* 0x08xx for ACL Rate Limit */
    RT_ERR_ACLRL_HTHR = 0x0800,                     /* 0x0800, invalid high threshold                                               */
    RT_ERR_ACLRL_TIMESLOT,                          /* 0x0801, invalid time slot                                                    */
    RT_ERR_ACLRL_TOKEN,                             /* 0x0802, invalid token amount                                                 */
    RT_ERR_ACLRL_RATE,                              /* 0x0803, invalid rate                                                         */

    /* 0x09xx for Link aggregation */
    RT_ERR_LA_CPUPORT = 0x0900,                     /* 0x0900, CPU port can not be aggregated port                                  */
    RT_ERR_LA_TRUNK_ID,                             /* 0x0901, invalid trunk id                                                     */
    RT_ERR_LA_PORTMASK,                             /* 0x0902, invalid port mask                                                    */
    RT_ERR_LA_HASHMASK,                             /* 0x0903, invalid hash mask                                                    */
    RT_ERR_LA_DUMB,                                 /* 0x0904, this API should be used in 802.1ad dumb mode                         */
    RT_ERR_LA_PORTNUM_DUMB,                         /* 0x0905, it can only aggregate at most four ports when 802.1ad dumb mode      */
    RT_ERR_LA_PORTNUM_NORMAL,                       /* 0x0906, it can only aggregate at most eight ports when 802.1ad normal mode   */
    RT_ERR_LA_MEMBER_OVERLAP,                       /* 0x0907, the specified port mask is overlapped with other group               */
    RT_ERR_LA_NOT_MEMBER_PORT,                      /* 0x0908, the port is not a member port of the trunk                           */
    RT_ERR_LA_TRUNK_NOT_EXIST,                      /* 0x0909, the trunk doesn't exist                                              */


    /* 0x0axx for storm filter */
    RT_ERR_SFC_TICK_PERIOD = 0x0a00,                /* 0x0a00, invalid SFC tick period                                              */
    RT_ERR_SFC_UNKNOWN_GROUP,                       /* 0x0a01, Unknown Storm filter group                                           */

    /* 0x0bxx for pattern match */
    RT_ERR_PM_MASK = 0x0b00,                        /* 0x0b00, invalid pattern length. Pattern length should be 8                   */
    RT_ERR_PM_LENGTH,                               /* 0x0b01, invalid pattern match mask, first byte must care                     */
    RT_ERR_PM_MODE,                                 /* 0x0b02, invalid pattern match mode                                           */

    /* 0x0cxx for input bandwidth control */
    RT_ERR_INBW_TICK_PERIOD = 0x0c00,               /* 0x0c00, invalid tick period for input bandwidth control                      */
    RT_ERR_INBW_TOKEN_AMOUNT,                       /* 0x0c01, invalid amount of token for input bandwidth control                  */
    RT_ERR_INBW_FCON_VALUE,                         /* 0x0c02, invalid flow control ON threshold value for input bandwidth control  */
    RT_ERR_INBW_FCOFF_VALUE,                        /* 0x0c03, invalid flow control OFF threshold value for input bandwidth control */
    RT_ERR_INBW_FC_ALLOWANCE,                       /* 0x0c04, invalid allowance of incomming packet for input bandwidth control    */
    RT_ERR_INBW_RATE,                               /* 0x0c05, invalid input bandwidth                                              */

    /* 0x0dxx for QoS */
    RT_ERR_QOS_1P_PRIORITY = 0x0d00,                /* 0x0d00, invalid 802.1P priority                                              */
    RT_ERR_QOS_DSCP_VALUE,                          /* 0x0d01, invalid DSCP value                                                   */
    RT_ERR_QOS_INT_PRIORITY,                        /* 0x0d02, invalid internal priority                                            */
    RT_ERR_QOS_SEL_DSCP_PRI,                        /* 0x0d03, invalid DSCP selection priority                                      */
    RT_ERR_QOS_SEL_PORT_PRI,                        /* 0x0d04, invalid port selection priority                                      */
    RT_ERR_QOS_SEL_IN_ACL_PRI,                      /* 0x0d05, invalid ingress ACL selection priority                               */
    RT_ERR_QOS_SEL_CLASS_PRI,                       /* 0x0d06, invalid classifier selection priority                                */
    RT_ERR_QOS_EBW_RATE,                            /* 0x0d07, invalid egress bandwidth rate                                        */
    RT_ERR_QOS_SCHE_TYPE,                           /* 0x0d08, invalid QoS scheduling type                                          */
    RT_ERR_QOS_QUEUE_WEIGHT,                        /* 0x0d09, invalid Queue weight                                                 */
    RT_ERR_QOS_SEL_PRI_SOURCE,                      /* 0x0d0a, invalid selection of priority source                                                 */
    
    /* 0x0exx for port ability */
    RT_ERR_PHY_PAGE_ID = 0x0e00,                    /* 0x0e00, invalid PHY page id                                                  */
    RT_ERR_PHY_REG_ID,                              /* 0x0e01, invalid PHY reg id                                                   */
    RT_ERR_PHY_DATAMASK,                            /* 0x0e02, invalid PHY data mask                                                */
    RT_ERR_PHY_AUTO_NEGO_MODE,                      /* 0x0e03, invalid PHY auto-negotiation mode*/
    RT_ERR_PHY_SPEED,                               /* 0x0e04, invalid PHY speed setting                                            */
    RT_ERR_PHY_DUPLEX,                              /* 0x0e05, invalid PHY duplex setting                                           */
    RT_ERR_PHY_FORCE_ABILITY,                       /* 0x0e06, invalid PHY force mode ability parameter                             */
    RT_ERR_PHY_FORCE_1000,                          /* 0x0e07, invalid PHY force mode 1G speed setting                              */
    RT_ERR_PHY_TXRX,                                /* 0x0e08, invalid PHY tx/rx                                                    */

    /* 0x0fxx for mirror */
    RT_ERR_MIRROR_DIRECTION = 0x0f00,               /* 0x0f00, invalid error mirror direction                                       */
    RT_ERR_MIRROR_SESSION_FULL,                     /* 0x0f01, mirroring session is full                                            */
    RT_ERR_MIRROR_SESSION_NOEXIST,                  /* 0x0f02, mirroring session not exist                                          */
    RT_ERR_MIRROR_PORT_EXIST,                       /* 0x0f03, mirroring port already exists                                        */
    RT_ERR_MIRROR_PORT_NOT_EXIST,                   /* 0x0f04, mirroring port does not exists                                       */
    RT_ERR_MIRROR_PORT_FULL,                        /* 0x0f05, Exceeds maximum number of supported mirroring port                   */

    /* 0x10xx for stat */
    RT_ERR_STAT_INVALID_GLOBAL_CNTR = 0x1000,       /* 0x1000, Invalid Global Counter                                               */
    RT_ERR_STAT_INVALID_PORT_CNTR,                  /* 0x1001, Invalid Port Counter                                                 */
    RT_ERR_STAT_GLOBAL_CNTR_FAIL,                   /* 0x1002, Could not retrieve/reset Global Counter                              */
    RT_ERR_STAT_PORT_CNTR_FAIL,                     /* 0x1003, Could not retrieve/reset Port Counter                                */

    /* 0x1100 for dot1x */
    RT_ERR_DOT1X_INVALID_DIRECTION = 0x1100,        /* 0x1100, Invalid Authentication Direction                                     */
    RT_ERR_DOT1X_PORTBASEDPNEN,                     /* 0x1101, Port-based enable port error                                         */
    RT_ERR_DOT1X_PORTBASEDAUTH,                     /* 0x1102, Port-based auth port error                                           */
    RT_ERR_DOT1X_PORTBASEDOPDIR,                    /* 0x1103, Port-based opdir error                                               */
    RT_ERR_DOT1X_MACBASEDPNEN,                      /* 0x1104, MAC-based enable port error                                          */
    RT_ERR_DOT1X_MACBASEDOPDIR,                     /* 0x1105, MAC-based opdir error                                                */
    RT_ERR_DOT1X_PROC,                              /* 0x1106, unauthorized behavior error                                          */
    RT_ERR_DOT1X_GVLANIDX,                          /* 0x1107, guest vlan index error                                               */
    RT_ERR_DOT1X_GVLANTALK,                         /* 0x1108, guest vlan OPDIR error                                               */
    RT_ERR_DOT1X_MAC_PORT_MISMATCH,                 /* 0x1109, Auth MAC and port mismatch eror                                      */

    /* 0x1200 for jumbo */
    RT_ERR_JUMBO_FRAME_SIZE = 0x1200,               /* Jumbo frame size not supported */
    
    RT_ERR_END                                       /* The symbol is the latest symbol                                                  */
} rt_error_code_t;    


#endif /*__RTL_ERROR_H__*/





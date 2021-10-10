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
* Purpose : asic-level driver function list for RTL8309N switch
*
* Feature :  This file consists of following modules:
*               1)	Packet length
*				2)	Phy
*				3)	Port isolation
*				4)	VLAN
*				5)	CPU port
*				6)	Qos
*				7)	ACL
*				8)	MIB
*				9)	Mirror
*				10) Lookup table
*				11) Spanning tree
*				12) Dot1x
*				13) IGMP
*				14)	Trap
*				15) RMA
*				16)	Interrupt
*				17)	Storm filter
*				18)	RLDP/RLPP
*				19)	ISP
*				20) LED
*/

#ifndef _RTL8309N_ASICDRV_EXT_H
#define _RTL8309N_ASICDRV_EXT_H

#include <rtl8309n_types.h>
#include <rtl8309n_asicdrv.h>

/* Function Name:
 *      rtl8309n_reg_set
 * Description:
 *      Write Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~ 8)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~17)
 *      value    - Value to be write into the register
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable registers of RTL8309N, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
extern int32 rtl8309n_reg_set(uint32 phyad, uint32 regad, uint32 npage, uint32 value) ;

/* Function Name:
 *      rtl8309n_reg_get
 * Description:
 *      Read Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~ 8)
 *      regad   - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~ 17)
 * Output:
 *      pvalue    - The pointer of value read back from register
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable registers of RTL8309N, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
extern int32 rtl8309n_reg_get(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue);

/* Function Name:
 *      rtl8309n_regbit_set
 * Description:
 *      Write one bit of Asic Register
 * Input:
 *      phyad   - Specify Phy address (0 ~ 8)
 *      regad    - Specify register address (0 ~31)
 *      bit        - Specify bit position(0 ~ 15)
 *      npage   - Specify page number (0 ~ 17)
 *      value    - Value to be write(0, 1)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function  you could write each bit of  all configurable registers of RTL8309N.
 */
extern int32 rtl8309n_regbit_set(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 value) ;

/* Function Name:
 *      rtl8309n_regbit_get
 * Description:
 *      Read one bit of Asic  PHY Register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      bit        - Specify bit position(0 ~ 15)
 *      npage   - Specify page number (0 ~3)
 * Output:
 *      pvalue  - The pointer of value read back
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could read each bit of  all configurable registers of RTL8309N
 */
extern int32 rtl8309n_regbit_get(uint32 phyad, uint32 regad, uint32 bit, uint32 npage,  uint32 * pvalue) ;

/* Function Name:
 *      rtl8309n_phyReg_set
 * Description:
 *      Write PCS page register
 * Input:
 *      phyad   - Specify Phy address (0 ~ 7)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~ 17)
 *      value    - Value to be write into the register
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable pcs registers of RTL8309N, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
extern int32 rtl8309n_phyReg_set(uint32 phyad, uint32 regad, uint32 npage, uint32 value) ;

/* Function Name:
 *      rtl8309n_phyReg_get
 * Description:
 *      Read PCS page register
 * Input:
 *      phyad   - Specify Phy address (0 ~6)
 *      regad    - Specify register address (0 ~31)
 *      npage   - Specify page number (0 ~5)
 * Output:
 *      pvalue    - The pointer of value read back from register
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function you could write all configurable pcs registers of RTL8309N, 
 *      it is realized by calling functions smiRead and smiWrite which are switch
 *      MDC/MDIO interface access functions. Those two functions use two GPIO 
 *      pins to simulate MDC/MDIO timing,  and they are based on rtl8651b platform,
 *      to modify them,  you can port all asic API to other platform.
 */
extern int32 rtl8309n_phyReg_get(uint32 phyad, uint32 regad, uint32 npage, uint32 *pvalue);

extern int32 rtl8309n_phyRegBit_set(uint32 phyad, uint32 regad, uint32 bit, uint32 npage, uint32 value);

extern int32 rtl8309n_phyRegBit_get(uint32 phyad, uint32 regad, uint32 bit, uint32 npage, uint32 *pvalue);

extern int32 rtl8309n_asic_init(void);

#if 0
extern int32 rtl8309n_asic_init(void);

#endif
/* Function Name:
 *      rtl8309n_phy_reset
 * Description:
 *      Reset the phy
 * Input:
 *      phy   - Specify Phy address (0 ~ 7)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *		RTL8309N has 7 PHYs from PHY 0 - 7.
 */
extern int32 rtl8309n_phy_reset(uint32 phy);

/*
extern int32 rtl8309n_phy_reset1(uint32 phy);

*/
/* Function Name:
 *      rtl8309n_switch_maxPktLen_set
 * Description:
 *      set Max packet length which could be forwarded by
 * Input:
 *		type		-  max packet length type
 *      maxLen      -  user defined max packet length
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      maxLen could be set : 
 *      RTL8309N_MAX_PKT_LEN_1518 -1518 bytes without any tag; 1522 bytes: 
 *              with VLAN tag or CPU tag, 1526 bytes with CPU and VLAN tag;
 *      RTL8309N_MAX_PKT_LEN_2048 - 2048 bytes (all tags counted);
 *      RTL8309N_MAX_PKT_LEN_USER - user defined (64~2048 bytes); 
 *      RTL8309N_MAX_PKT_LEN_2000 - 16k bytes (all tags counted) 
 *              
 */ 
extern int32 rtl8309n_switch_maxPktLen_set(uint32 type, uint32 maxLen);

/* Function Name:
 *      rtl8309n_switch_maxPktLen_get
 * Description:
 *      set Max packet length which could be forwarded by
 * Input:
 *      none
 * Output:
 *		pType	-	max packet length type
 *      pMaxLen         -  max packet length
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      maxLen could be set : 
 *      RTL8309N_MAX_PKT_LEN_1518 -1518 bytes without any tag; 1522 bytes: 
 *              with VLAN tag or CPU tag, 1526 bytes with CPU and VLAN tag;
 *      RTL8309N_MAX_PKT_LEN_1536 - 1536 bytes (all tags counted);
 *      RTL8309N_MAX_PKT_LEN_1552 - 1552 bytes (all tags counted); 
 *      RTL8309N_MAX_PKT_LEN_2000 - 2000 bytes (all tags counted) 
 *              
 */ 
extern int32 rtl8309n_switch_maxPktLen_get(uint32 *pType, uint32 *pMaxLen);

/* Function Name:
 *      rtl8309n_port_etherPhy_set
 * Description:
 *      Configure PHY setting
 * Input:
 *      phy                    - Specify the phy to configure
 *      autoNegotiation    - Specify whether enable auto-negotiation
 *      advCapability       - When auto-negotiation is enabled, specify the advertised capability
 *      speed                 - When auto-negotiation is disabled, specify the force mode speed
 *      fullDuplex            - When auto-negotiatoin is disabled, specify the force mode duplex mode
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When auto-negotiation is enabled, the advertisement capability is used to handshaking with link partner.
 *      When auto-negotiation is disabled, the phy is configured into force mode and the speed and duplex mode 
 *      setting is based on speed and fullDuplex setting.Port number should be smaller than RTL8309N_PHY_NUMBER.
 *      AdverCapability should be ranged between RTL8309N_ETHER_AUTO_100FULL and RTL8309N_ETHER_AUTO_10HALF.
 *      Speed should be either RTL8309N_ETHER_SPEED_100 or RTL8309N_ETHER_SPEED_10.
 */
extern int32 rtl8309n_port_etherPhy_set(uint32 phy, uint32 autoNegotiation, uint32 advCapability, uint32 speed, uint32 fullDuplex) ;

/* Function Name:
 *      rtl8309n_port_etherPhy_get
 * Description:
 *       Get PHY setting
 * Input:
 *      phy                    - Specify the phy to configure
 * Output:
 *      pAutoNegotiation    - Get whether auto-negotiation is enabled
 *      pAdvCapability       - When auto-negotiation is enabled, Get the advertised capability
 *      pSpeed                 - When auto-negotiation is disabled, Get the force mode speed
 *      pFullDuplex            - When auto-negotiatoin is disabled, Get the force mode duplex mode
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When auto-negotiation is enabled, the advertisement capability is used to handshaking with link partner.
 *      When auto-negotiation is disabled, the phy is configured into force mode and the speed and duplex mode 
 *      setting is based on speed and fullDuplex setting.Port number should be smaller than RTL8309N_PHY_NUMBER.
 *      AdverCapability should be ranged between RTL8309N_ETHER_AUTO_100FULL and RTL8309N_ETHER_AUTO_10HALF.
 *      Speed should be either RTL8309N_ETHER_SPEED_100 or RTL8309N_ETHER_SPEED_10.
 */
extern int32 rtl8309n_port_etherPhy_get(uint32 phy, uint32 *pAutoNegotiation, uint32 *pAdvCapability, uint32 *pSpeed, uint32 *pFullDuplex);

/* Function Name:
 *      rtl8309n_port_phyLinkStatus_get
 * Description:
 *      Get PHY Link Status
 * Input:
*      phy        - Specify the phy 
 * Output:
*      plinkUp   - Describe whether link status is up or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *       Read the link status of PHY register 1
 */
extern int32 rtl8309n_port_phyLinkStatus_get(uint32 phy, uint32 *plinkUp) ;

extern int32 rtl8309n_port_macAbilityExt0_get(rtl8309n_mode_ext_t *pMode, rtl8309n_port_mac_ability_t *pPortAbility);

/* Function Name:
 *		rtl8309n_port_macForceLinkExt0_set
 * Description:
 *		Set MAC 8 force mode ability
 * Input:
 *		mode	-	MAC interface mode(TMII/MII/RMII, MAC/PHY)
 *		pPortAbility	-	pointer point to the struct describing MAC ability
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_port_macForceLinkExt0_set(rtl8309n_mode_ext_t mode, rtl8309n_port_mac_ability_t *pPortAbility);

/* Function Name:
 *		rtl8309n_port_macForceLinkExt0_get
 * Description:
 *		Get MAC 8 force mode ability configuration
 * Input:
 *		none
 * Output:
 *		pMode	-	MAC interface mode(TMII/MII/RMII, MAC/PHY)
 *		pPortAbility	-	pointer point to the struct describing MAC ability
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_port_macForceLinkExt0_get(rtl8309n_mode_ext_t *pMode, rtl8309n_port_mac_ability_t *pPortAbility);


/* Function Name:
 *      rtl8309n_port_phyAutoNegoDone_get
 * Description:
 *      Get PHY auto-negotiation result status
 * Input:
 *      phy      -	Specify the phy to get status
 * Output:
*       pDone	 -  Describe whether auto-negotiation is done or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Read the auto-negotiation complete of PHY register 1.
 */
extern int32 rtl8309n_port_phyAutoNegoDone_get(uint32 phy, uint32 *pDone) ;

/* Function Name:
 *      rtl8309n_port_phyLoopback_set
 * Description:
 *       Set PHY loopback
 * Input:
 *      phy         - 	Specify the phy to configure
 *      enabled   	- 	Enable phy loopback
 * Output:
 *      none      
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Only phy 0~7 could be configured the phy loopback
 */
extern int32 rtl8309n_port_phyLoopback_set(uint32 phy, uint32 enabled) ;

/* Function Name:
 *      rtl8309n_port_phyLoopback_get
 * Description:
 *      Get PHY loopback setting
 * Input:
 *      phy         - Specify the phy to get status
 * Output:
 *      pEnabled  -  phy loopback setting
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
extern int32 rtl8309n_port_phyLoopback_get(uint32 phy, uint32 *pEnabled) ;

/* Function Name:
 *      rtl8309n_port_MacLearnEnable_set
 * Description:
 *      Enable/disable physical port learning ability
 * Input:
 *      port        - Specify port number (0 ~ 8)
 * Output:
 *      enabled -  enable or disable
 * Return:
 *      SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_port_MacLearnEnable_set(uint32 port, uint32 enabled);

/* Function Name:
 *      rtl8309n_port_MacLearnEnable_get
 * Description:
 *      Enable/disable physical port learning ability
 * Input:
 *      port        -	Specify port number (0 ~ 8)
 * Output:
 *      pEnabled 	-	pointer point to the enabled status of port mac learning ability 
 * Return:
 *      SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_port_MacLearnEnable_get(uint32 port, uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_port_isolation_set
 * Description:
 *      set port isolation 
 * Input:
 *		port	-	port number(0 - 8)
 *      isomsk    - port isolation port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      In RTL8309N, there are 9 ports. Every port has its port isolation mask which has 9 bits.
 *		When the port isolation mask bit n of port m is 1, it means port m can
 *		forword packets to port n. When the bit n is 0 of port m, portm can not forward 
 *		packets to port n.
 */
extern int32 rtl8309n_port_isolation_set(uint32 port, uint32 isomsk);

/* Function Name:
 *      rtl8309n_port_isolation_get
 * Description:
 *      get the status of port isolation 
 * Input:
 *      port	-	port number
 * Output:
 *      pIsomsk    -  the pointer of port isolation port mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 * Note:
 *      In RTL8309N, there are 9 ports. Every port has its port isolation mask which has 9 bits.
 *		When the port isolation mask bit n of port m is 1, it means port m can
 *		forword packets to port n. Otherwise when the bit n is 0 of port m, portm can not forward 
 *		packets to port n.
 */
extern int32 rtl8309n_port_isolation_get(uint32 port, uint32 *pIsomsk);

/* Function Name:
 *		rtl8309n_vlan_init
 * Description:
 *		Init vlan
 * Input:
 *		none
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *      VLAN is disabled by default. User has to call this API to enable VLAN before
 *      using it. And It will set a default VLAN(vid 1) including all ports and set 
 *      all ports PVID to the default VLAN.
 */
extern int32 rtl8309n_vlan_init(void);

/* Function Name:
 *		rtl8309n_vlan_enable_set
 * Description:
 *		Enable vlan
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		
 */
extern int32 rtl8309n_vlan_enable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_vlan_enable_get
 * Description:
 *		Enable vlan
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 *		
 */
extern int32 rtl8309n_vlan_enable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_vlan_discardNonPvidPktEnable_set
 * Description:
 *		Enable discarding tagged frame whose VID does not match PVID 
 * Input:
 *		port	-	port number(0 - 8)
 *		discard_enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		When tagged frame's VID doesn't equal to PVID, this function can be called to discard
 *		or normal forward the frame.
 */
extern int32 rtl8309n_vlan_discardNonPvidPktEnable_set(uint32 port, uint32 discard_enabled);

/* Function Name:
 *		rtl8309n_vlan_discardNonPvidPktEnable_get
 * Description:
 *		Get discarding status of tagged frame whose VID does not match PVID 
 * Input:
 *		port	-	port number(0 - 8)
 * Output:
 *		pDiscard_enabled	-	enable or disable
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		When tagged frame's VID doesn't equal to PVID, this function can be called to discard
 *		or normal forward the frame.
 */
extern int32 rtl8309n_vlan_discardNonPvidPktEnable_get(uint32 port, uint32 *pDiscard_enabled);

/* Function Name:
 *      rtl8309n_vlan_tagAwareEnable_set
 * Description:
 *      Enable ASIC tag aware
 * Input:
 *		port     - port number(0 - 8)
 *      enabled  - Configure RTL8309N VLAN tag awared
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *		Enable ASIC to parse the tag when it receives a frame.
 */
extern int32 rtl8309n_vlan_tagAwareEnable_set(uint32 port, uint32 enabled);

/* Function Name:
 *      rtl8309n_vlan_tagAwareEnable_get
 * Description:
 *      Get tag aware enabled status
 * Input:
 *      port      - port number
 * Output:
 *      pEnabled  - the pointer of RTL8309N VLAN tag awared status
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *		Enable ASIC to parse the tag when it receives a frame.
 */
extern int32 rtl8309n_vlan_tagAwareEnable_get(uint32 port, uint32 * pEnabled) ;

/* Function Name:
 *      rtl8309n_vlan_igrFilterEnable_set
 * Description:
 *      Enable VLAN ingress filter
 * Input:
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
extern int32 rtl8309n_vlan_igrFilterEnable_set(uint32 enabled) ;

/* Function Name:
 *      rtl8309n_vlan_igrFilterEnable_get
 * Description:
 *      Get VLAN ingress filter enabled status
 * Input:
 *      none
 * Output:
 *      pEnabled  - enable or disable
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
extern int32 rtl8309n_vlan_igrFilterEnable_get(uint32 *pEnabled) ;

/* Function Name:
 *      rtl8309n_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN support frame type
 * Input:
 *      port                          - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    	RTL8309N_ACCEPT_ALL             -   accept vlan tagged, prority tagged(vid=0) and untagged packets
 *    	RTL8309N_ACCEPT_TAG_ONLY        -   accpet vlan tagged packets
 *    	RTL8309N_ACCEPT_UNTAG_ONLY      -   accept prority tagged and untagged packets
 */
extern int32 rtl8309n_vlan_portAcceptFrameType_set(uint32 port, rtl8309n_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtl8309n_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN support frame type
 * Input:
 *      port                                 - Port id
 * Output:
 *      pAccept_frame_type             - accept frame type pointer
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    	RTL8309N_ACCEPT_ALL             -   accept vlan tagged, prority tagged(vid=0) and untagged packets
 *    	RTL8309N_ACCEPT_TAG_ONLY        -   accpet vlan tagged packets
 *    	RTL8309N_ACCEPT_UNTAG_ONLY      -   accept prority tagged and untagged packets
 */
extern int32 rtl8309n_vlan_portAcceptFrameType_get(uint32 port, rtl8309n_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      rtl8309n_vlan_portPvidIndex_set
 * Description:
 *      Configure port PVID index 
 * Input:
 *      port           -   Specify the port(port 0 ~ port 8) to configure VLAN index
 *      vlanIndex      -   Specify the VLAN index
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 16 vlan entry, VID of vlan entry pointed by Port index is port's PVID 
 */
extern int32 rtl8309n_vlan_portPvidIndex_set(uint32 port, uint32 vlanIndex);

/* Function Name:
 *      rtl8309n_vlan_portPvidIndex_get
 * Description:
 *      Get port PVID index 
 * Input:
 *      port            -   Specify the port(port 0 ~ port 8) to configure VLAN index
 * Output:
 *      pVlanIndex   -   pointer of VLAN index number
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 16 vlan entry, VID of vlan entry pointed by PVID index  is the PVID 
 */
extern int32 rtl8309n_vlan_portPvidIndex_get(uint32 port, uint32 *pVlanIndex) ;

/* Function Name:
 *      rtl8309n_vlan_leakyEnable_set
 * Description:
 *      Configure switch to forward frames to other VLANs ignoring the egress rule.
 * Input:
 *      type   -  vlan leaky type
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    (1)type coulde be:
 *          RTL8309N_VLAN_LEAKY_UNICAST - Vlan leaky for unicast pkt
 *          RTL8309N_VLAN_LEAKY_MULTICAST - Vlan leaky for multicast pkt
 *          RTL8309N_VLAN_LEAKY_BRDCAST   - VLan leaky for broadcast pkt
 *          RTL8309N_VLAN_LEAKY_ARP - Vlan leaky for ARP brodcast pkt 
 *          RTL8309N_VLAN_LEAKY_MIRROR - Vlan leaky for mirror function
 *    (2)When the Vlan leaky for unicast pkt is enabled, it enables the inter-VLANs unicast packet forwarding. 
 *       That is, if the L2 look up MAC table search hit, then the unicast packet will be forwarded
 *       to the egress port ignoring the egress rule.
 *    (3)When Vlan leaky for multicast pkt is enabled, multicast packet may be flood to all multicast address
 *       group member set, ignoring the VLAN member set domain limitation.
 *    (4)when Vlan leaky for broadcast pkt is enabled, broadcast packet may be flood to all ports, ignoring the 
 *       VLAN memeber set domain limitation.
 *    (5)When Vlan leaky for ARP pkt is enabled, the ARP broadcast packets will be forward to all the other
 *       ports ignoring the egress rule.
 *    (6)When Vlan leaky for mirror function is enabled, it enables the inter-VLANs mirror function, 
 *       ignoring the VLAN member set domain limitation.
 */
extern int32 rtl8309n_vlan_leakyEnable_set(uint32 type, uint32 enabled);

/* Function Name:
 *      rtl8309n_vlan_leakyEnable_get
 * Description:
 *      Get switch whether forwards unicast frames to other VLANs
 * Input:
 *      type   -  vlan leaky type
 * Output:
 *      pEnabled  - the pointer of Vlan Leaky status(Dsiabled or Enabled) 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *    (1)type coulde be:
 *          RTL8309N_VLAN_LEAKY_UNICAST - Vlan leaky for unicast pkt
 *          RTL8309N_VLAN_LEAKY_MULTICAST - Vlan leaky for multicast pkt
 *          RTL8309N_VLAN_LEAKY_BRDCAST   - VLan leaky for broadcast pkt
 *          RTL8309N_VLAN_LEAKY_ARP - Vlan leaky for ARP brodcast pkt 
 *          RTL8309N_VLAN_LEAKY_MIRROR - Vlan leaky for mirror function
 *    (2)When the Vlan leaky for unicast pkt is enabled, it enables the inter-VLANs unicast packet forwarding. 
 *       That is, if the L2 look up MAC table search hit, then the unicast packet will be forwarded
 *       to the egress port ignoring the egress rule.
 *    (3)When Vlan leaky for multicast pkt is enabled, multicast packet may be flood to all multicast address
 *       group member set, ignoring the VLAN member set domain limitation.
 *    (4)when Vlan leaky for broadcast pkt is enabled, broadcast packet may be flood to all ports, ignoring the 
 *       VLAN memeber set domain limitation.
 *    (5)When Vlan leaky for ARP pkt is enabled, the ARP broadcast packets will be forward to all the other
 *       ports ignoring the egress rule.
 *    (6)When Vlan leaky for mirror function is enabled, it enables the inter-VLANs mirror function, 
 *       ignoring the VLAN member set domain limitation.
 */
extern int32 rtl8309n_vlan_leakyEnable_get(uint32 type, uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_vlan_nullVidReplaceEnable_set
 * Description:
 *      Configure switch to replace Null VID tagged frame by PVID if it is tag aware
 * Input:
 *      port   -  port number
 *      enabled  - enable or disable
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.When Null VID replacement is enabled, 8309n only captures tagged packet with VID=0,
 *      then replace VID with input port's PVID. If switch received a packet that is not tagged, 
 *      it will not insert a tag with PVID to this packet.
 *      2. When Null VID replacement is disabled, switch will drop or deal the null VID tagged 
 *      frame depends on the configuration.
 */
extern int32 rtl8309n_vlan_nullVidReplaceEnable_set(uint32 port, uint32 enabled);

/* Function Name:
 *      rtl8309n_vlan_nullVidReplaceEnable_get
 * Description:
 *      Configure switch to forward frames to other VLANs ignoring the egress rule.
 * Input:
 *      port   -  port number
 * Output:
 *      pEnabled  - the pointer of Null VID replacement ability(Dsiabled or Enabled) 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1.When Null VID replacement is enabled, 8306E only captures tagged packet with VID=0,
 *      then replace VID with input port's PVID. If switch received a packet that is not tagged, 
 *      it will not insert a tag with PVID to this packet.
 *      2. When Null VID replacement is disabled, switch will drop or deal the null VID tagged 
 *      frame depends on the configuration.
 */
extern int32 rtl8309n_vlan_nullVidReplaceEnable_get(uint32 port, uint32 *pEnabled);

/* Function Name:
 * 		rtl8309n_vlan_tagInsertRemove_set
 * Description:
 *		configure switch to insert or remove tag when TX frames out from a port
 * Input:
 *		port	-	port number
 *		mode	-	replace, insert, remove, or don't touch
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      This API could be called to set per egress port action for vlan tag.
 *      The action could be:
 *          RTL8309N_VLAN_TAGINSERTREMOVE_REPALCE   -   replace, which means that remove tag from tagged packets
 *                  add new tag(vid=pvid) to it, and just add new tag(vid=pvid) to untagged packets
 *          RTL8309N_VLAN_TAGINSERTREMOVE_REMOVE    -   remove, which means that remove tag from tagged pacets
 *                  and don't touch untagged packets
 *          RTL8309N_VLAN_TAGINSERTREMOVE_INSERT    -   insert, which means that add new tag(vid=pvid) to untagged packets
 *          RTL8309N_VLAN_TAGINSERTREMOVE_DONTTOUCH     -   do nothing for frames' tag
 */
extern int32 rtl8309n_vlan_tagInsertRemove_set(uint32 port, uint32 mode);

/* Function Name:
 * 		rtl8309n_vlan_tagInsertRemove_get
 * Description:
 *		Get switch status whether to insert or remove tag when TX frames out from a port
 * Input:
 *		port	-	port number
 * Output:
 *		pMode	-	replace, insert, remove, or don't touch
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      This API could be called to set per egress port action for vlan tag.
 *      The action could be:
 *          RTL8309N_VLAN_TAGINSERTREMOVE_REPALCE   -   replace, which means that remove tag from tagged packets
 *                  add new tag(vid=pvid) to it, and just add new tag(vid=pvid) to untagged packets
 *          RTL8309N_VLAN_TAGINSERTREMOVE_REMOVE    -   remove, which means that remove tag from tagged pacets
 *                  and don't touch untagged packets
 *          RTL8309N_VLAN_TAGINSERTREMOVE_INSERT    -   insert, which means that add new tag(vid=pvid) to untagged packets
 *          RTL8309N_VLAN_TAGINSERTREMOVE_DONTTOUCH     -   do nothing for frames' tag
 */
extern int32 rtl8309n_vlan_tagInsertRemove_get(uint32 port, uint32 *pMode);

/* Function Name:
 *		rtl8309n_vlan_igrPortTagInsertEnable_set
 * Description:
 *		Enable switch to insert vlan tag based on ingress port
 * Input:
 *		port	-	port number	(0 - 8)
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
extern int32 rtl8309n_vlan_igrPortTagInsertEnable_set(uint32 port, uint32 enabled);

/* Function Name:
 *		rtl8309n_vlan_igrPortTagInsertEnable_get
 * Description:
 *		Enable switch to insert vlan tag based on ingress port
 * Input:
 *		port	-	port number	(0 - 8)
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
extern int32 rtl8309n_vlan_igrPortTagInsertEnable_get(uint32 port, uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_vlan_tagInsert_set
 * Description:
 *      Insert VLAN tag to untag packet by ingress port
 * Input:
 *      egPort              - egress port number 0~8
 *      igPortMsk           - ingress port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      egPort is packet egress port, if the packet is untagged and its igress port
 *      is in the rxPortMsk, it will be inserted with an VLAN tag.
 */
extern int32 rtl8309n_vlan_igrPortTagInsert_set(uint32 egPort, uint32 rxPortMsk);

/* Function Name:
 *      rtl8309n_vlan_tagInsert_get
 * Description:
 *      get  ingress port mask of VLAN tag insertion for untagged packet
 * Input:
 *      egPort               - egress port number 0~5
 * Output:
 *      pRxPortMsk           - ingress port mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      egPort is packet egress port, if the packet is untagged and its igress port
 *      is in the igPortMsk, it will be inserted with an VLAN tag.
 */
extern int32 rtl8309n_vlan_igrPortTagInsert_get(uint32 egPort, uint32 * pRxPortMsk);

/* Function Name:
 *      rtk_vlan_set
 * Description:
 *      Set a VLAN entry
 * Input:
 *      vlanIndex     - VLAN entry index
 *      vid           - VLAN ID to configure
 *      mbrmsk        - VLAN member set portmask
 *      untagmsk      - VLAN untag set portmask
 *      fid           - VLAN IVL ID
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     fid can be any value from 0 - 3.It means that RTL8309N maxismly supports 4 IVLs.
 */
extern int32 rtl8309n_vlan_entry_set(uint32 vlanIndex, uint32 vid, uint32 mbrmsk, uint32 untagmsk, uint32 fid);

/* Function Name:
 *      rtl8309n_vlan_entry_get
 * Description:
 *      Get a VLAN entry
 * Input:
 *      vlanIndex  - VLAN entry index
 * Output:
 *      pVid           -  the pointer of VLAN ID 
 *      pMbrmsk     -  the pointer of VLAN member set portmask
 *      pUntagmsk  -  the pointer of VLAN untag set portmask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 */
extern int32 rtl8309n_vlan_entry_get(uint32 vlanIndex, uint32 *pVid, uint32 *pMbrmsk, uint32 *pUntagmsk, uint32 *pFid);

/* Function Name:
 *		rtl8309n_vlan_fixMcastFidEnable_set
 * Description:
 *		Enable fixing fid for multicast packets
 * Input:
 *		enabled		-	Enable or Disable
 * Output:
 *		none
 * Return:
 *		SUCCESS		-	success
 * Note:
 *		
 */
extern int32 rtl8309n_vlan_fixMcastFidEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_vlan_fixMcastFidEnable_get
 * Description:
 *		Get Enabled status of fixing fid for multicast packets
 * Input:
 *		none
 * Output:
 *		pEnabled		-	Enable or Disable
 * Return:
 *		SUCCESS		-	Success
 *		FAILED		-	Failure
 * Note:
 *		
 */
extern int32 rtl8309n_vlan_fixMcastFidEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_vlan_fixMcastFid_set
 * Description:
 *		Set fixed fid for multicast packets
 * Input:
 *		fid
 * Output:
 *		none
 * Return:
 *		SUCCESS		-	Success
 *		FAILED		-	Failure
 * Note:
 *		
 */
extern int32 rtl8309n_vlan_fixMcastFid_set(uint32 fid);

/* Function Name:
 *		rtl8309n_vlan_fixMcastFid_get
 * Description:
 *		Get fixed fid for multicast packets
 * Input:
 *		none
 * Output:
 *		pFid	-	pointer point to fid
 * Return:
 *		SUCCESS		-	success
 *		FAILED		-	failure
 */
extern int32 rtl8309n_vlan_fixMcastFid_get(uint32 *pFid);

/* Function Name:
 *      rtl8309n_vlan_unmatchVidAction_set
 * Description:
 *      set action for unmatch vid packets
 * Input:
 *      action
 * Output:
 *      none
 * Return:
 *      FAILED
 *      SUCCESS
 * Note:
 *      This API can be called to set action for vlan tagged packets whose
 *      tag vid can't be found in vlan table.
*/
extern int32 rtl8309n_vlan_unmatchVidAction_set(uint32 action);

/* Function Name:
 *      rtl8309n_vlan_unmatchVidAction_get
 * Description:
 *      get action for unmatch vid packets
 * Input:
 *      none
 * Output:
 *      pAction
 * Return:
 *      FAILED
 *      SUCCESS
 * Note:
 *      This API can be called to get action for vlan tagged packets whose
 *      tag vid can't be found in vlan table.
*/
extern int32 rtl8309n_vlan_unmatchVidAction_get(uint32 *pAction);

/* Function Name:
 *		rtl8309n_cpu_enable_set
 * Description:
 *		Enable CPU port ability
 * Input:
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS		-	success
 * Note:
 */
extern int32 rtl8309n_cpu_enable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_cpu_enable_set
 * Description:
 *		Get status of CPU port ability
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS		-	success
 * Note:
 */
extern int32 rtl8309n_cpu_enable_get(uint32 *pEnabled);

/* Function Name:
 * 		rtl8309n_cpu_portNumber_set
 * Description:
 *		Set cpu port number
 * Input:
 *		port	-	port id(0 - 8), 9 means no port is set as cpu port
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 */
extern int32 rtl8309n_cpu_portNumber_set(uint32 port);

/* Function Name:
 * 		rtl8309n_cpu_portNumber_set
 * Description:
 *		Get cpu port number
 * Input:
 *		none
 * Output:
 *		pPort	-	port id(0 - 8)
 * Return:
 *		FAILED
 *		SUCCESS
 */
extern int32 rtl8309n_cpu_portNumber_get(uint32 *pPort);

/* Function Name:
 *		rtl8309n_cpu_tagInsertEnable_set
 * Description:
 *		Enable insert cpu tag to packets trapped to CPU
 * Input:
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_cpu_tagInsertEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_cpu_tagInsertEnable_get
 * Description:
 *		Get enbaled status of CPU tag insert ability
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
extern int32 rtl8309n_cpu_tagInsertEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_cpu_tagRemoveEnable_set
 * Description:
 *		Enable remove CPU tag when receive a packet from cpu port
 * Input:
 *		enabled
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_cpu_tagRemoveEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_cpu_tagRemoveEnable_get
 * Description:
 *		Get status of removing cpu tag
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_cpu_tagRemoveEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_cpu_cpuTagAwareEnable_set
 * Description:
 *		Enable CPU tag aware ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_cpu_cpuTagAwareEnable_set(uint32 enabled);

/* Function Name:
 * 		rtl8309n_cpu_cpuTagAwareEnable_get
 * Description:
 *		Get status of CPU tag aware ability
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 *		FAILED
 */
extern int32 rtl8309n_cpu_cpuTagAwareEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_cpu_learnEnable_set
 * Description:
 *		Enable CPU port learning ability
 * Input:
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_cpu_learnEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_cpu_learnEnable_set
 * Description:
 *		Enable CPU port learning ability
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_cpu_learnEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_cpu_checkCrcEnable_set
 * Description:
 *		Enable checking CRC for CPU tagged packets
 * Input:
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_cpu_checkCrcEnable_set(uint32 enabled);

/* Function:
 *		rtl8309n_cpu_checkCrcEnable_get
 * Description:
 *		Get status of checking CRC ability
 * Input:
 *		none
 * Output:
 *		pEnabled
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_cpu_checkCrcEnable_get(uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_cpu_port_set
 * Description:
 *      Specify Asic CPU port 
 * Input:
 *      port       -   Specify the port
 *      enTag      -   CPU tag insert or not
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If the port is specified RTL8309N_NOCPUPORT, it means
 *      that no port is assigned as cpu port
 */
extern int32 rtl8309n_cpu_port_set(uint32 port, uint32 enTag) ;

/* Function Name:
 *      rtl8309n_cpu_port_get
 * Description:
 *       Get Asic CPU port number
 * Input:
 *      none
 * Output:
 *      pPort     - the pointer of CPU port number
 *      pEnTag    - the pointer of CPU tag insert or not
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      If the port is specified RTL8309N_NOCPUPORT, it means
 *      that no port is assigned as cpu port
 */
extern int32 rtl8309n_cpu_port_get(uint32 *pPort, uint32 *pEnTag);

/* Function Name:
 *		rtl8309n_cpu_tagInsertMask_set
 * Description:
 *		Enable inserting CPU tag for special packets
 * Input:
 *		type    -   special packet type
 *      enabled -   enable,disable
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_cpu_tagInsertMask_set(rtl8309n_cpu_trapPkt_t type, uint32 enabled);

/* Function Name:
 *		rtl8309n_cpu_tagInsertMask_get
 * Description:
 *		Get enabled status of inserting CPU tag for special packets
 * Input:
 *		type    -   special packet type
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_cpu_tagInsertMask_get(rtl8309n_cpu_trapPkt_t type, uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_cpu_nonCpuPortRxCpuTag_set
 * Description:
 *		Set action for none cpu port receive a cpu tagged frame
 * Input:
 *		action	-	action, drop or normal forward
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This Api can be called to set the action when port which is not cpu 
 *      port receives a cpu tagged packets.
 *      The action could be:
 *          RTL8309N_ACT_DROP   -   drop
 *          RTL8309N_ACT_PERMIT     -  forward as normal
 */
extern int32 rtl8309n_cpu_nonCpuPortRxCpuTag_set(uint32 action);

/* Function Name:
 *		rtl8309n_cpu_nonCpuPortRxCpuTag_get
 * Description:
 *		Get action for none cpu port receive a cpu tagged frame
 * Input:
 *		none
 * Output:
 *		pAction	-	action, drop or normal forward
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This Api can be called to get the action when port which is not cpu 
 *      port receives a cpu tagged packets.
 *      The action could be:
 *          RTL8309N_ACT_DROP   -   drop
 *          RTL8309N_ACT_PERMIT     -  forward as normal
 */
extern int32 rtl8309n_cpu_nonCpuPortRxCpuTag_get(uint32 *pAction);

/* Function Name:
 *      rtl8309n_qos_softReset_set
 * Description:
 *      Software reset the asic
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Set switch to software reset.
 */
 
extern int32 rtl8309n_qos_softReset_set(void) ;

/* Function Name:
 *      rtl8309n_qos_queueNum_set
 * Description:
 *      Set egress port queue number 
 * Input:
 *		num		-	queue number(1 ~4)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Queue number is global configuration for switch. It's value is from 1 to 4.
 */
extern int32 rtl8309n_qos_queueNum_set(uint32 num);

/* Function Name:
 *      rtl8309n_qos_queueNum_get
 * Description:
 *      Get egress port queue number 
 * Input:
 *      pNum	-	pointer point to queue number(1 ~4)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Queue number is global configuration for switch.It's value is from 1 to 4.
 */
extern int32 rtl8309n_qos_queueNum_get(uint32 *pNum) ;

/* Function Name:
 *      rtl8309n_qos_priToQueMap_set
 * Description:
 *     Set priority to Queue ID mapping
 * Input:
 *      port       -  port id (0 - 8)
 *      priority   -  priority value (0 ~ 3)
 *      qid        -  Queue id (0 ~ 3)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *		Set 2 bit user priority to queue id mapping. It's set for per port.
 */
extern int32 rtl8309n_qos_priToQueMap_set(uint32 port, uint32 priority, uint32 qid);

/* Function Name:
 *      rtl8309n_qos_priToQueMap_get
 * Description:
 *      Get priority to Queue ID mapping
 * Input:
 *      port       -  port id (0 - 8)
 *      priority   -  priority value (0 ~ 3)
 * Output:
 *      pQid      -  pointer of Queue id (0~3)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 * Note:
 *		Set 2 bit user priority to queue id mapping. It's set for per port.
 */
extern int32 rtl8309n_qos_priToQueMap_get(uint32 port, uint32 priority, uint32 *pQid) ;

/* Function Name:
 *      rtl8309n_qos_portRate_set
 * Description:
 *      Set port bandwidth control
 * Input:
 *      port            -  port number (0 ~ 8)
 *      n64Kbps         -  Port rate, unit 64Kbps ;
 *						   0 - 0x640, port 0 - 7,  100Mps, UTP mode; 
 *                         0 - 0xc80, port 0, 200Mbps, Fiber mode;				   
 *      direction       -  Ingress or Egress bandwidth control
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For each port, both input and output bandwidth could be configured.
 *      The direction could be:
 *          RTL8309N_PORT_RX    -  port input bandwidth control. 
 *          RTL8309N_PORT_TX    -  port output bandwidth control.
 *      port rate unit is 64Kbps. 
 */
extern int32 rtl8309n_qos_portRate_set(uint32 port, uint32 n64Kbps, uint32 direction);

/* Function Name:
 *      rtl8309n_qos_portRate_get
 * Description:
 *      Get port bandwidth control rate
 * Input:
 *      port                 -  Port number (0~5)
 * Output:
 *      *pN64Kbps        -  Port rate (0~1526), unit 64Kbps
 *      direction           -  Input or output bandwidth control
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For each port, both input and output bandwidth could be configured.
 *      The direction could be:
 *          RTL8309N_PORT_RX    -  port input bandwidth control. 
 *          RTL8309N_PORT_TX    -  port output bandwidth control.
 *      port rate unit is 64Kbps. 
 */
extern int32 rtl8309n_qos_portRate_get(uint32 port, uint32 *pN64Kbps, uint32 direction) ;

/* Function Name:
 *		rtl8309n_qos_queueRate_set
 * Description:
 *		Set queue rate
 * Input:
 *		port	-	port number(0 - 8)
 *		queue	-	queue number(2, 3)
 *		n64Kbps	-	rate value
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)For RTL8309N, only queue 2 and 3 per port have leaky bucket. So only queue 2 and 3's rate can be set.
 *		    Port rate unit is 64Kbps.
 *      (2)The queue could be:
 *          RTL8309N_QUEUE2     -   queue 2
 *          RTL8309N_QUEUE3     -   queue 3
 */
extern int32 rtl8309n_qos_queueRate_set(uint32 port, uint32 queue, uint32 n64Kbps);

/* Function Name:
 *		rtl8309n_qos_queueRate_set
 * Description:
 *		Set queue rate
 * Input:
 *		port	-	port number(0 - 8)
 *		queue	-	queue number(2, 3)
 * Output:
 *		pN64Kbps	-	rate value
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)For RTL8309N, only queue 2 and 3 per port have leaky bucket. So only queue 2 and 3's rate can be set.
 *		    Port rate unit is 64Kbps.
 *      (2)The queue could be:
 *          RTL8309N_QUEUE2     -   queue 2
 *          RTL8309N_QUEUE3     -   queue 3
 */
extern int32 rtl8309n_qos_queueRate_get(uint32 port, uint32 queue, uint32 *pN64Kbps);

/* Function Name:
 *		rtl8309n_qos_portRate_IfgIncludeEnable_set
 * Description:
 *		Enable including IFG when set port and queue rate 
 * Input:
 *		direction	-	direction of port, rx or tx
 *		port	-	port number (0 - 8)
 *		enabled	-	enable or disable	
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      The direction could be:
 *          RTL8309N_PORT_RX    -   input
 *          RTL8309N_PORT_TX    -   output
 */
extern int32 rtl8309n_qos_portRate_IfgIncludeEnable_set(uint32 direction, uint32 port, uint32 enabled);

/* Function Name:
 *		rtl8309n_qos_portRate_IfgIncludeEnable_get
 * Description:
 *		Enable including IFG when set port and queue rate 
 * Input:
 *		direction	-	direction of port, rx or tx
 *		port	-	port number (0 - 8)
 * Output:
 *		pEnabled	-	enable or disable	
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      The direction could be:
 *          RTL8309N_PORT_RX    -   input
 *          RTL8309N_PORT_TX    -   output
 */
extern int32 rtl8309n_qos_portRate_IfgIncludeEnable_get(uint32 direction, uint32 port, uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_qos_1pRemarkEnable_set
 * Description:
 *      Set 802.1P remarking ability
 * Input:
 *      port       -  port number (0~8)
 *      enabled    -  enable or disable
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For RTL8309N, it could use 1P remark ability to map 2 bit internal 
 *		priority to 3 bit 1Q tag priority.
 */
extern int32 rtl8309n_qos_1pRemarkEnable_set(uint32 port, uint32 enabled);

/* Function Name:
 *      rtl8309n_qos_1pRemarkEnable_set
 * Description:
 *      Get 802.1P remarking ability status
 * Input:
 *      port        -  port number (0~8)
 * Output:
 *      pEnabled  -  pointer of the ability status, enabled or disabled
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      For RTL8309N, it could use 1P remark ability to map 2 bit internal 
 *		priority to 3 bit 1Q tag priority.
 */
extern int32 rtl8309n_qos_1pRemarkEnable_get(uint32 port, uint32 *pEnabled) ;

/* Function Name:
 *      rtl8309n_qos_1pRemark_set
 * Description:
 *      Set 802.1P remarking priority
 * Input:
 *      priority       -  Packet priority(0~4)
 *      priority1p     -  802.1P priority(0~7)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *		
 */
extern int32 rtl8309n_qos_1pRemark_set(uint32 priority, uint32 priority1p);

/* Function Name:
 *      rtl8309n_qos_1pRemark_get
 * Description:
 *      Get 802.1P remarking priority
 * Input:
 *      priority       -  Packet priority(0~4)
 * Output:
 *      pPriority1p  -  the pointer of 802.1P priority(0~7)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *
 */
extern int32 rtl8309n_qos_1pRemark_get(uint32 priority, uint32 *pPriority1p) ;

/* Function Name:
 *      rtl8309n_qos_portPri_set
 * Description:
 *      Set port-based priority
 * Input:
 *      port          -  port number (0~8)
 *      priority      -  Packet port-based priority(0~3)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      packet will be assigned a port-based priority correspond to the ingress port.
 */
extern int32 rtl8309n_qos_portPri_set(uint32 port, uint32 priority);

/* Function Name:
 *      rtl8309n_qos_portPri_get
 * Description:
 *      Get port-based priority
 * Input:
 *      port          -  port number (0~5)
 * Output:
 *      pPriority    -   pointer of packet port-based priority(0~4)
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      packet will be assigned a port-based priority correspond to the ingress port.
 */ 
extern int32 rtl8309n_qos_portPri_get(uint32 port, uint32 *pPriority) ;

/* Function Name:
 *      rtl8309n_qos_1QPortPri_set
 * Description:
 *      Set 1Q port-based priority
 * Input:
 *      port          -  port number (0~8)
 *      priority      -  Packet port-based priority(0~3)
 * Output:
 *       none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When 1Q based priority source is enabled, a untag packet will be assigned a 
 *		1Q port-based priority correspond to the 1Q port based priority. A tagged packet will be 
 *		assigned the priority from its VLAN tag or priority tag.
 */
extern int32 rtl8309n_qos_1QPortPri_set(uint32 port, uint32 priority);

/* Function Name:
 *      rtl8309n_qos_1QPortPri_get
 * Description:
 *      Set 1Q port-based priority
 * Input:
 *      port          -  port number (0~8)
 * Output:
 *      pPriority	  -  pointer point to the 1Q port based priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      When 1Q based priority source is enabled, a untag packet will be assigned a 
 *		1Q port-based priority correspond to the 1Q port based priority. A tagged packet will be 
 *		assigned the priority from its VLAN tag or priority tag.
 */
extern int32 rtl8309n_qos_1QPortPri_get(uint32 port, uint32 *pPriority);

/* Function Name:
 *      rtl8309n_qos_1pPriRemap_set
 * Description:
 *      Set Asic 1Q-tag priority mapping to 2-bit priority
 * Input:
 *      tagprio   -  1Q-tag proirty (0~7, 3 bit value)
 *      prio      -   internal use 2-bit priority
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      1Q tag priority has 3 bits, but switch internal use 2-bit priority. So it should map 3-bit 1Q-tag priority
 *      to 2-bit priority
 */ 
extern int32 rtl8309n_qos_1pPriRemap_set(uint32 tagprio, uint32 prio) ;

/* Function Name:
 *      rtl8309n_qos_1pPriRemap_get
 * Description:
 *      Get Asic 1Q-tag priority mapping to 2-bit priority
 * Input:
 *      tagprio   -  1Q-tag proirty (0~7, 3 bit value)
 * Output:
 *      pPrio     -  pointer of  internal use 2-bit priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch internal use 2-bit priority, so it should map 3-bit 1Q-tag priority
 *      to 2-bit priority
 */ 
extern int32 rtl8309n_qos_1pPriRemap_get(uint32 tagprio, uint32 *pPrio) ;

/* Function Name:
 *      rtl8309n_dscpPriRemap_set
 * Description:
 *      Set DSCP-based priority
 * Input:
 *      value      -  dscp code
 *      priority   -  dscp-based priority
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:  
 *		DSCP has 64 value from 0 - 63.
 */ 
extern int32 rtl8309n_qos_dscpPriRemap_set(uint32 value, uint32 priority) ;

/* Function Name:
 *      rtl8309n_dscpPriRemap_set
 * Description:
 *      Get DSCP-based priority
 * Input:
 *      value      -  dscp code
 * Output:
 *      pPriority  -  the pointer of dscp-based priority
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     
 */ 
extern int32 rtl8309n_qos_dscpPriRemap_get(uint32 value, uint32 *pPriority) ;

/* Function Name:
 *      rtl8309n_qos_priSrcArbit_set
 * Description:
 *      Set priority source arbitration weight
 * Input:
 *      pPriArbit  - pointer point to the structure describe weight of 4 priority sources
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch could recognize 8 types of priority source at most, 
 *      and a packet properly has all of them. Among them, there 
 *      are 4 type priorities could be set priority weight, they are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority,
 *      Port-based priority. Each one could be set weight from 0 to 4, 
 *      arbitration module will decide their sequece to take based on 
 *      their weight, the highest weight priority will be adopted at first, 
 *      then  priority type of the sencond highest weight. 
 */
extern int32 rtl8309n_qos_priSrcArbit_set(rtl8309n_qos_priArbitPara_t *pPriArbit) ;

/* Function Name:
 *      rtl8309n_qos_priSrcArbit_set
 * Description:
 *      Set priority source arbitration weight
 * Input:
 *      none
 * Output:
 *      pPriArbit  - The poniter point to the structure describe weight of 4 priority sources
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch could recognize 8 types of priority source at most, 
 *      and a packet properly has all of them. Among them, there 
 *      are 4 type priorities could be set priority weight, they are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority,
 *      Port-based priority. Each one could be set weight from 0 to 4, 
 *      arbitration module will decide their sequece to take based on 
 *      their weight, the highest weight priority will be adopted at first, 
 *      then  priority type of the sencond highest weight. 
 */
extern int32 rtl8309n_qos_priSrcArbit_get(rtl8309n_qos_priArbitPara_t *pPriArbit);

/* Function Name:
 *      rtl8309n_qos_priSrcEnable_set
 * Description:
 *      enable/disable Qos priority source for ingress port
 * Input:
 *      port      -  Specify port number (0 ~ 8)
 *      priSrc    -  Specify priority source  
 *      enabled   -  DISABLED or ENABLED 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 5 kind of priority source for each port which could
 *     be enabled or disabled:
 *		RTL8309N_CPUTAG_PRI     -   cpu tag based priority
 *		RTL8309N_IP_PRI        -   IP address based priority
 *		RTL8309N_DSCP_PRI      -   DSCP based priority
 *		RTL8309N_1Q_PRI        -   DOT1Q based priority
 *		RTL8309N_PORT_PRI      -   port based priority
 */
extern int32 rtl8309n_qos_priSrcEnable_set(uint32 port, uint32 priSrc, uint32 enabled) ;

/* Function Name:
 *      rtl8309n_qos_priSrcEnable_set
 * Description:
 *      enable/disable Qos priority source for  ingress port
 * Input:
 *      port       -  Specify port number (0 ~5)
 *      priSrc     -  Specify priority source  
 * Output:
 *      pEnabled   -  the pointer of priority source status  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 5 kind of priority source for each port which could
 *     be enabled or disabled:
 *		RTL8309N_CPUTAG_PRI     -   cpu tag based priority
 *		RTL8309N_IP_PRI        -   IP address based priority
 *		RTL8309N_DSCP_PRI      -   DSCP based priority
 *		RTL8309N_1Q_PRI        -   DOT1Q based priority
 *		RTL8309N_PORT_PRI      -   port based priority
 */
extern int32 rtl8309n_qos_priSrcEnable_get(uint32 port, uint32 priSrc, uint32 *pEnabled) ;

/* Function Name:
 *		rtl8309n_qos_ipAddrPriEnable_set
 * Description:
 *		Enable IP address based priority
 * Input:
 *		entry	-	entry type
 *		enabled	-	enable or disable
 * Output:
 * 		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
extern int32 rtl8309n_qos_ipAddrPriEnable_set(uint32 entry, uint32 enabled);

/* Function Name:
 *		rtl8309n_qos_ipAddrPriEnable_get
 * Description:
 *		Get IP address based priority ability status
 * Input:
 *		entry	-	entry type
 * Output:
 * 		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
extern int32 rtl8309n_qos_ipAddrPriEnable_get(uint32 entry, uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_qos_ipAddrPri_set
 * Description:
 *      Set IP address priority
 * Input:
 *		entry	-	indicate the ip address A or B
 *      priority  -  internal use 2-bit priority value (0 ~ 3)  
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
extern int32 rtl8309n_qos_ipAddrPri_set(uint32 entry, int32 priority) ;

/* Function Name:
 *      rtl8309n_qos_ipAddrPri_get
 * Description:
 *      Get IP address priority
 * Input:
 *		entry	-	indicate the ip address A or B
 * Output:
 *      pPriority  -  internal use 2-bit priority value (0~3)  
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
extern int32 rtl8309n_qos_ipAddrPri_get(uint32 entry, uint32 *pPriority);

/* Function Name:
 *      rtl8309n_qos_ipAddr_set
 * Description:
 *      Set IP address
 * Input:
 *      entry        -   specify entry
 *      ip           -   ip address
 *      mask         -  ip mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)There are two entries RTL8309N_IPADD_A and RTL8309N_IPADD_B
 *          for user setting ip address, if ip address of packet matches
 *          the entry, the packet will be assign the priority of ip address
 *          priority which is configured by rtl8309n_qos_ipAddrPri_set.
 *      (2)The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
extern int32 rtl8309n_qos_ipAddr_set(uint32 entry, uint32 ip, uint32 mask) ;

/* Function Name:
 *      rtl8309n_qos_ipAddr_get
 * Description:
 *      Get IP address user seting
 * Input:
 *      entry       -   specify entry
 * Output:
 *      pIp            -   ip address
 *      pMask        -   ip mask
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)There are two entries RTL8309N_IPADD_A and RTL8309N_IPADD_B
 *          for user setting ip address, if ip address of packet matches
 *          the entry, the packet will be assign the priority of ip address
 *          priority which is configured by rtl8309n_qos_ipAddrPri_set.
 *      (2)The entry could be :
 *          RTL8309N_IPADD_A    -   entry for IP address A
 *          RTL8309N_IPADD_B    -   entry for IP address B
 */
extern int32 rtl8309n_qos_ipAddr_get(uint32 entry, uint32 *pIp, uint32 *pMask) ;

/* Function Name:
 *      rtl8309n_qos_queueWrr_set
 * Description:
 *      Set qos scheduling parameter(WRR)
 * Input:
 *      port         -  port id
 *      sch_para     -  The structure describe queue scheduling parameter
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch supports 4 queue per egress port, scheduling algorithm could be WRR(Weight Round Robin) or 
 *      SP(Strict Priority). Either WRR or SP can be used for each of 4 queues. SP has higher priority than WRR,
 *		so queues with SP will be scanned firt when 4 queues are scheduled.I n rtl8309n_qos_schPara_t, q0_wt 
 *		and q1_wt, q2_wt and  q3_wt could be 0~127. 
 */
extern int32 rtl8309n_qos_queueWrr_set(uint32 port, rtl8309n_qos_schPara_t sch_para);

/* Function Name:
 *      rtl8309n_qos_queueWrr_get
 * Description:
 *      Set qos scheduling parameter(WRR)
 * Input:
 *      port           -  port id
 * Output:
 *      pSch_para      - the pointer of struct describing the schedule parameter
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      switch supports 4 queue per egress port, scheduling algorithm could be WRR(Weight Round Robin) or 
 *      SP(Strict Priority). Either WRR or SP can be used for each of 4 queues. SP has higher priority than WRR,
 *		so queues with SP will be scanned firt when 4 queues are scheduled.In rtl8309n_qos_schPara_t, q0_wt 
 *		and q1_wt, q2_wt and  q3_wt could be 0~127. 
 */
extern int32 rtl8309n_qos_queueWrr_get(uint32 port, rtl8309n_qos_schPara_t *pSch_para);

/*Function Name:
 *		rtl8309n_qos_queueStrictPriEnable_set
 *Description:
 *      Enable one queue's stric priority
 *Input:
 *      port    -   port number
 *      queue   -   queue number
 *      enabled   -  ENABLED or DISABLED
 *Output:
 *      none
 *Return:
 *      SUCCESS
 *      FAILED
 *Note:
 *      (1)Every port of RTL8309N have 4 queue. Strict priority ability can be enable or disable for each queue.
 *          They can be enable or disable independently.
 *      (2)The queue could be:
 *              RTL8309N_QUEUE0     -   queue 0
 *              RTL8309N_QUEUE1     -   queue 1
 *              RTL8309N_QUEUE2     -   queue 2
 *              RTL8309N_QUEUE3     -   queue 3
 */
extern int32 rtl8309n_qos_queueStrictPriEnable_set(uint32 port, uint32 queue, uint32 enabled);

/*Function Name:
 *		rtl8309n_qos_queueStrictPriEnable_get
 *Description:
 *      Enable one queue's stric priority
 *Input:
 *      port      -  port number
 *      queue     -  queue number
 *Output:
 *      pEnabled   -  pointer point to ENABLED or DISABLED
 *Return:
 *      SUCCESS
 *      FAILED
 *Note:
 *      (1)Every port of RTL8309N have 4 queue. Strict priority ability can be enable or disable for each queue.
 *          They can be enable or disable independently.
 *      (2)The queue could be:
 *              RTL8309N_QUEUE0     -   queue 0
 *              RTL8309N_QUEUE1     -   queue 1
 *              RTL8309N_QUEUE2     -   queue 2
 *              RTL8309N_QUEUE3     -   queue 3
 */
extern int32 rtl8309n_qos_queueStrictPriEnable_get(uint32 port, uint32 queue, uint32 *pEnabled);

/*Function Name:
 *     rtl8309n_qos_portLeakyBktEnable_set
 *Description:
 *     Enable port Tx or Rx leaky bucket
 *Input:
 *     port        -  port number(0 - 8)
 *     direction   -  direction, rx or tx
 *     enabled     -  enable or disable the leaky bucket
 *Output:
 *     none
 *Return:
 *	   SUCCESS
 *	   FAILED
 *Note:
 *	   (1)Every port of RTL8309N have rx or tx leaky bucket. Each port's Tx or Rx leaky bucket can be enabled 
 *	        or disabled independently.
 *     (2)The direction could be:
 *          RTL8309N_PORT_TX    -   ouput
 *          RTL8309N_PORT_RX    -   input        
 */
extern int32 rtl8309n_qos_portLeakyBktEnable_set(uint32 port, uint32 direction, uint32 enabled);

/*Function Name:
 *     rtl8309n_qos_portLeakyBktEnable_set
 *Description:
 *     Enable port Tx or Rx leaky bucket
 *Input:
 *     port         -  port number(0 - 8)
 *     direction    -  direction, rx or tx
 *Output:
 *     pEnabled     -  pointer point to the enabling status of leaky bucket
 *Return:
 *	   SUCCESS
 *	   FAILED
 *Note:
 *	   (1)Every port of RTL8309N have rx or tx leaky bucket. Each port's Tx or Rx leaky bucket can be enabled 
 *	        or disabled independently.
 *     (2)The direction could be:
 *          RTL8309N_PORT_TX    -   ouput
 *          RTL8309N_PORT_RX    -   input         
 */
extern int32 rtl8309n_qos_portLeakyBktEnable_get(uint32 port, uint32 direction, uint32 *pEnabled);

/*Function Name:
 *     rtl8309n_qos_queueLeakyBktEnable_set
 *Description:
 *     Enable output leaky buckt of a queue in a port
 *Input:
 *     port    -  port number(0 - 8)
 *     queue   -  queue number(2 - 3)
 *     enabled   -  ENABLED or DISABLED
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     (1)Every output port of RTL8309N have 4 queues. Only queue 3 and queue 2 have leaky bucket.
 *          They can be enable or disable independently.
 *     (2)queue could be:
 *          RTL8309N_QUEUE2     -   queue 2
 *          RTL8309N_QUEUE3     -   queue 3
 */
extern int32 rtl8309n_qos_queueLeakyBktEnable_set(uint32 port, uint32 queue, uint32 enabled);

/*Function Name:
 *     rtl8309n_qos_queueLeakyBktEnable_get
 *Description:
 *     Enable output leaky buckt of a queue in a port
 *Input:
 *     port    -  port number(0 - 8)
 *     queue   -  queue number(2 - 3)
 *Output:
 *     pEnabled   -  pointer point to ENABLED or DISABLED
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     Every output port of RTL8309N have 4 queues. Only queue 3 and queue 2 have leaky bucket.
 *     They can be enable or disable independently.
 */
extern int32 rtl8309n_qos_queueLeakyBktEnable_get(uint32 port, uint32 queue, uint32 *pEnabled);

/*Function Name:
 *     rtl8309n_qos_queueFlcEnable_set
 *Description:
 *     Enable flow control ability of one queue
 *Input:
 *     port      -   port number (0 - 8)
 *     queue     -   queue number (0 - 3)
 *     enabled   -   ENABLED or DISABLED
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *	   (1)Enble one queue's flow control ability. every port have 4 queues for RTL8309N.
 *        Each queue's flow control ability can be enabled or disabled independently.	
 *     (2)The port could be:
 *          RTL8309N_PORT0, RTL8309N_PORT1, RTL8309N_PORT2, RTL8309N_PORT3, 
 *          RTL8309N_PORT4, RTL8309N_PORT5, RTL8309N_PORT5, RTL8309N_PORT6, 
 *          RTL8309N_PORT7, RTL8309N_PORT8,
 */
extern int32 rtl8309n_qos_queueFlcEnable_set(uint32 port, uint32 queue, uint32 enabled) ;

/*Function Name:
 *     rtl8309n_qos_queueFlcEnable_get
 *Description:
 *     Enable flow control ability of one queue
 *Input:
 *     port      -   port number (0 - 8)
 *     queue     -   queue number (0 - 3)
 *Output:
 *     pEnabled   -   pointer point to ENABLED or DISABLED
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *	   (1)Enble one queue's flow control ability. every port have 4 queues for RTL8309N.
 *        Each queue's flow control ability can be enabled or disabled independently.	
 *     (2)The port could be:
 *          RTL8309N_PORT0, RTL8309N_PORT1, RTL8309N_PORT2, RTL8309N_PORT3, 
 *          RTL8309N_PORT4, RTL8309N_PORT5, RTL8309N_PORT5, RTL8309N_PORT6, 
 *          RTL8309N_PORT7, RTL8309N_PORT8,
 */
extern int32 rtl8309n_qos_queueFlcEnable_get(uint32 port, uint32 queue, uint32 *pEnabled) ;

/*Function Name:
 *     rtl8309n_qos_queueFlcThr_set
 *Description:
 *     Set one queue's flow control threshold, include packet based and descriptor based types.
 *Input:
 *     port     -    port number (0 - 8)
 *     queue    -    queue number (0 - 3)
 *     type     -    indicate the queue flow controling threshold type
 *     value    -    value for the dedicated threshold
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     (1)There are two flow controling types for queue. One is descriptor based and the other is packet based.
 *      Each type have flow control on and off threshold. The on and off threshold can be set independently 
 *      for each queue.
 *     (2)The queue flow control threshold type could be:
 *          RTL8309N_FLC_QUEUEDSCTHRLD_ON      -    queue descriptor flow control on threshold
 *          RTL8309N_FLC_QUEUEDSCTHRLD_OFF     -    queue descriptor flow control off threshold
 *          RTL8309N_FLC_QUEUEPKTTHRLD_ON      -    queue packet flow control on threshold 
 *          RTL8309N_FLC_QUEUEPKTTHRLD_OFF     -    queue packet flow control off threshold   
 *
 */
extern int32 rtl8309n_qos_queueFlcThr_set(uint32 port, uint32 queue, uint32 type, uint32 value);

/*Function Name:
 *     rtl8309n_qos_queueFlcThr_get
 *Description:
 *     Get one queue's flow control threshold, include packet based and descriptor based types.
 *Input:
 *     port     -   port number (0 - 8)
 *     queue    -   queue number (0 - 3)
 *     type     -   indicate the queue flow controling threshold type
 *Output:
 *     pValue   -   pointer point to the value for the dedicated threshold
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     (1)There are two flow controling types for queue. One is descriptor based and the other is packet based.
 *      Each type have flow control on and off threshold. The on and off threshold can be set independently 
 *      for each queue.
 *     (2)The queue flow control threshold type could be:
 *          RTL8309N_FLC_QUEUEDSCTHRLD_ON      -    queue descriptor flow control on threshold
 *          RTL8309N_FLC_QUEUEDSCTHRLD_OFF     -    queue descriptor flow control off threshold
 *          RTL8309N_FLC_QUEUEPKTTHRLD_ON      -    queue packet flow control on threshold 
 *          RTL8309N_FLC_QUEUEPKTTHRLD_OFF     -    queue packet flow control off threshold   
 *
 */
extern int32 rtl8309n_qos_queueFlcThr_get(uint32 port, uint32 queue, uint32 type, uint32* pValue) ;

/*Function Name:
 *     rtl8309n_qos_portDscFlcEnable_set
 *Description
 *     Enable port descriptor flow control ability
 *Input:
 *     port    -  port number(0 - 8)
 *     enabled -  ENABLED or DISABLED
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     The port descriptor based flow control will not be turn on untill the system descriptor based flow control 
 *     is turn on. And at this time port descriptor flow control ability must be enabled.
 */
extern int32 rtl8309n_qos_portDscFlcEnable_set(uint32 port, uint32 enabled);

/*Function Name:
 *     rtl8309n_qos_portDscFlcEnable_get
 *Description
 *     Enable port descriptor flow control ability
 *Input:
 *     port    -  port number(0 - 8)
 *Output:
 *     pEnabled  -  pointer point to ENABLED or DISABLED
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     The port descriptor based flow control will not be turn on untill the system descriptor based flow control 
 *     is turn on. And at this time port descriptor flow control ability must be enabled.
 */
extern int32 rtl8309n_qos_portDscFlcEnable_get(uint32 port, uint32 *pEnabled);

/*Function Name:
 *     rtl8309n_qos_egrFlcEnable_get
 *Description
 *     Enable output flow control ability , include port and queue based flow control.
 *Input:
 *     port     -  port number(0 - 8)
 *     enabled  -  ENABLED or DISABLED
 *Output:
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     The output flow control ability include port descriptor based, queue descriptor
 *     based and queue packet based type. This API can be called to enable the whole output flow control
 *     ability. Only after this ability is enabled, RTL8309N can really start to output flow control.
 */
extern int32 rtl8309n_qos_egrFlcEnable_set(uint32 port, uint32 enabled);

/*Function Name:
 *     rtl8309n_qos_egrFlcEnable_get
 *Description
 *     Enable output flow control ability , include port and queue based flow control.
 *Input:
 *     port    -  port number(0 - 8)
 *Output:
 *     pEnabled  -  pointer point to ENABLED or DISABLED
 *Return:
 *     SUCCESS
 *     FAILED
 *Note:
 *     The output flow control ability include port descriptor based, queue descriptor
 *     based and queue packet based type. This API can be called to enable the whole output flow control
 *     ability. Only after this ability is enabled, RTL8309N can really start to output flow control.
 */
extern int32 rtl8309n_qos_egrFlcEnable_get(uint32 port, uint32 *pEnabled);

/*Function Name:
 *     rtl8309n_qos_portFlcThr_set
 *Description
 *     Set the port descriptor based flow control turn on and turn off threshold, include RX and TX.
 *Input:
 *     port   -  port number
 *     onthr   -  turn on threshold
 *     offthr  -  turn off threshold
 *     direction  -  input or output
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *Note:
 *     (1)Set the port descriptor base flow control turn on and turn off threshold. There are input and output 
 *        port descriptor based flow control, so this API can be called to set the port descriptor based flow control
 *        threshold both for rx and tx, turn on and turn off.
 *     (2)The direction could be:
 *          RTL8309N_PORT_RX    -   input
 *          RTL8309N_PORT_TX    -   output
 */
extern int32 rtl8309n_qos_portFlcThr_set(uint32 port, uint32 onthr, uint32 offthr, uint32 direction ) ;

/*Function Name:
 *     rtl8309n_qos_portFlcThr_get
 *Description
 *     Set the port descriptor based flow control turn on and turn off threshold, include RX and TX.
 *Input:
 *     port   -  port number
 *     direction  -  input or output
 *Output:
 *     pOnthr   -  pointer point to turn on threshold
 *     pOffthr  -  pointer point to turn off threshold
 *Return:
 *     SUCCESS
 *Note:
 *     (1)Get the port descriptor base flow control turn on and turn off threshold. There are input and output 
 *        port descriptor based flow control, so this API can be called to set the port descriptor based flow control
 *        threshold both for rx and tx, turn on and turn off.
 *     (2)The direction could be:
 *          RTL8309N_PORT_RX    -   input
 *          RTL8309N_PORT_TX    -   output
 */
extern int32 rtl8309n_qos_portFlcThr_get(uint32 port, uint32 *pOnthr, uint32 *pOffthr, uint32 direction) ;

/*Function Name:
 *		rtl8309n_qos_igrFlcEnable_set
 *Description:
 *      Enable input flow control ability
 *Input:
 *      enabled
 *Output:
 *      none
 *Return:
 *      SUCCESS
 *      FAILED
 *Note:
 *      none
 */
extern int32 rtl8309n_qos_igrFlcEnable_set(uint32 enabled);

/*Function Name:
 *		rtl8309n_qos_igrFlcEnable_set
 *Description:
 *      Enable input flow control ability
 *Input:
 *      none
 *Output:
 *      pEnabled  - enable or disable
 *Return:
 *      SUCCESS
 *      FAILED
 *Note:
 *      none
 */
extern int32 rtl8309n_qos_igrFlcEnable_get(uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_acl_entry_set
 * Description:
 *      Set Asic ACL table
 * Input:
 *      entryAdd    -  Acl entry address (0~15)
 *      pAclData     -  struct describes the acl entry data
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      phyport could be 
 *          0~8: port number, 
 *          RTL8309N_ACL_INVALID: invalid entry,
 *          RTL8309N_ACL_ANYPORT: any port.
 *      Acl action could be
 *          RTL8309N_ACT_DROP,
 *          RTL8309N_ACT_PERMIT, 
 *          RTL8309N_ACT_TRAP2CPU, 
 *          RTL8309N_ACT_MIRROR
 *      Acl protocol could be
 *          RTL8309N_ACL_ETHER(ether type), 
 *          RTL8309N_ACL_TCP(TCP), 
 *          RTL8309N_ACL_UDP(UDP),
 *          RTL8309N_ACL_TCPUDP(TCP or UDP)
 *      Acl priority:
 *          RTL8309N_PRIO0 ~ RTL8309N_PRIO3     
 */
extern int32 rtl8309n_acl_entry_set(uint32 entryAdd, rtl8309n_acl_entry_t *pAclData) ;

/* Function Name:
 *      rtl8309n_acl_entry_get
 * Description:
 *      Get Asic ACL entry
 * Input:
 *      entryAddr   -  Acl entry address (0 ~ 15)
 * Output:
 *      pAclData   -  pointer point to struct decribing the acl entry data
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      phyport could be 
 *          0~8:       port number, 
 *          RTL8309N_ACL_INVALID: invalid entry,
 *          RTL8309N_ACL_ANYPORT: any port.
 *      Acl action could be
 *          RTL8309N_ACT_DROP,
 *          RTL8309N_ACT_PERMIT, 
 *          RTL8309N_ACT_TRAP2CPU, 
 *          RTL8309N_ACT_MIRROR
 *      Acl protocol could be
 *          RTL8309N_ACL_ETHER(ether type), 
 *          RTL8309N_ACL_TCP(TCP), 
 *          RTL8309N_ACL_UDP(UDP),
 *          RTL8309N_ACL_TCPUDP(TCP or UDP)
 *      Acl priority:
 *          RTL8309N_PRIO0~RTL8309N_PRIO3    
 */
extern int32 rtl8309n_acl_entry_get(uint32 entryAddr, rtl8309n_acl_entry_t *pAclData);

extern int32 rtl8309n_mib_enable_set(uint32 enabled);

extern int32 rtl8309n_mib_enable_get(uint32 *pEnabled);

extern int32 rtl8309n_mib_byteCnt_get(uint32 port, uint32 counter, uint32 *pValue);

extern int32 rtl8309n_mib_pktCnt_get(uint32 port, uint32 counter, uint32 *pValue); 

extern int32 rtl8309n_mib_start(uint32 port);
extern int32 rtl8309n_mib_stop(uint32 port);

/*Function Name:
 *		rtl8309n_mib_overFlowFlag_set
 *Description;
 *      Clear a port's MIB counter overflow flag 
 *Input:
 *      port     -  port number
 *      enaled   -  ENABLED or DISABLED
 *Output:
 *      none
 *Return:
 *		SUCCESS
 *      FAILED
 *Note:
 *      Write 1 to clear dedicate port's MIB counter overflow flag. When "enabled = ENABLED", overflow flag will be cleared.
 *      Otherwise, this API will do nothing.
 */
extern int32 rtl8309n_mib_overFlowFlag_set(uint32 port, uint32 overflow);

/*Function Name:
 *		rtl8309n_mib_overFlowFlag_get
 *Description;
 *      Get a port's MIB counter overflow flag 
 *Input:
 *      none
 *Output:
 *      pValue  -  pointer point to the overflow flag.
 *Return:
 *		SUCCESS
 *      FAILED
 *Note:
 *      none
 */
extern int32 rtl8309n_mib_overFlowFlag_get(uint32 port, uint32 *pOverflow);

/* Function Name:
 *      rtl8309n_mib_reset
 * Description:
 *      reset MIB counter
 * Input:
 *      port         -  port number (0 ~ 5)
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
extern int32 rtl8309n_mib_reset(uint32 port) ;

/* Function Name:
 *		rtl8309n_mirror_selfFilterEnable_set
 * Description:
 *		Enable mirror port self filter ability
 * Input:
 *		enabled 	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		Enable mirror port to filter packet sent from itself.
 */
extern int32 rtl8309n_mirror_selfFilterEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_mirror_selfFilterEnable_get
 * Description:
 *		Get status of  mirror port self filter ability
 * Input:
 *		enabled 	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		Enable mirror port to filter packet sent from itself.
 */
extern int32 rtl8309n_mirror_selfFilterEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_mirror_pauseFrameEnable_set
 * Description:
 *		Enable mirror pause frame ability
 * Input:
 *		enabled 	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		Enable mirror pause frame.
 */
extern int32 rtl8309n_mirror_pauseFrameEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_mirror_pauseFrameEnable_get
 * Description:
 *		Get status of mirror pause frame ability
 * Input:
 *		enabled 	-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		Enable mirror pause frame.
 */
extern int32 rtl8309n_mirror_pauseFrameEnable_get(uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_mirror_portBased_set
 * Description:
 *      Set asic Mirror port
 * Input:
 *      mirport         -  Specify mirror port 
 *      rxmbr           -  Specify Rx mirror port mask
 *      txmbr           -  Specify Tx mirror port mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      mirport could be 0 ~ 5, represent physical port number, 
 *      7 means that no port has mirror ability. rxmbr and txmbr
 *      is 6 bit value, each bit corresponds one port. ingress packet
 *      of port in rxmbr will be mirrored to mirport, egress packet 
 *      of port in txmbr will be mirrored to mirport.
 */
extern int32 rtl8309n_mirror_portBased_set(uint32 mirport, uint32 rxmbr, uint32 txmbr) ;

/* Function Name:
 *      rtl8309n_mirror_portBased_get
 * Description:
 *      Get asic Mirror port
 * Input:
 *      none 
 * Output:
 *      pMirport     -  the pointer of mirror port
 *      pRxmbr       -  the pointer of  Rx mirror port mask
 *      pTxmbr       -  the pointer of Tx mirror port mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      mirport could be 0 ~ 5, represent physical port number, 
 *      7 means that no port has mirror ability. rxmbr and txmbr
 *      is 6 bit value, each bit corresponds one port. ingress packet
 *      of port in rxmbr will be mirrored to mirport, egress packet 
 *      of port in txmbr will be mirrored to mirport.
 */
extern int32 rtl8309n_mirror_portBased_get(uint32 *pMirport, uint32 *pRxmbr, uint32* pTxmbr) ;

/* Function Name:
 *      rtl8309n_mirror_macBased_set
 * Description:
 *      Set Mac address for mirror packet
 * Input:
 *      macAddr   - mirrored mac address, it could be SA or DA of the packet 
 *      enabled   - enable mirror packet by mac address
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */
extern int32 rtl8309n_mirror_macBased_set(uint8 *macAddr, uint32 enabled) ;

/* Function Name:
 *      rtl8309n_mirror_macBased_set
 * Description:
 *      Set Mac address for mirror packet
 * Input:
 *      none 
 * Output:
 *      macAddr   - mirrored mac address, it could be SA or DA of the packet 
 *      pEnabled   - the pointer of enable mirror packet by mac address 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 * 
 */
extern int32 rtl8309n_mirror_macBased_get(uint8 *macAddr, uint32 *pEnabled) ;

/*Function Name:
 *		rtl8309n_l2_hashmode_set
 *Description:
 *		Set hash algorithm 
 *Input:
 *		mode
 *Output:
 *		none
 *Return:
 *		FAILED
 *		SUCCESS
 *Note:
 *      The hash algorithm could be:
 *          RTL8309N_HASH_OPT0  -   Type I hash algorithm 
 *          RTL8309N_HASH_OPT1  -   Type II hash algorithm
 */
extern int32 rtl8309n_l2_hashmode_set(uint32 mode);

/*Function Name:
 *		rtl8309n_l2_hashmode_get
 *Description:
 *		Set hash algorithm 
 *Input:
 *		none
 *Output:
 *		pMode	-	pointer point to the hash algorithm mode
 *Return:
 *		FAILED
 *		SUCCESS
 *Note:
 *      The hash algorithm could be:
 *          RTL8309N_HASH_OPT0  -   Type I hash algorithm 
 *          RTL8309N_HASH_OPT1  -   Type II hash algorithm
 */
extern int32 rtl8309n_l2_hashmode_get(uint32 *pMode);

/* Function Name:
 *      rtl8309n_l2_MacToIdx_get
 * Description:
 *      get L2 table hash value from mac address
 * Input:
 *      macAddr        -  mac address
 *		fid			   -  fid value
 * Output:
 *      pIndex         -  mac address table index   
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      when a mac address is learned into mac address table, 
 *      9 bit index value is got from the mac address by hashing 
 *      algorithm, each index corresponds to 4 entry, it means
 *      the table could save 4 mac addresses at the same time
 *      whose index value is equal, so switch mac address table 
 *      has 2048 entry. the API could get hash index from 
 *      a specified mac address and its  fid.
 */
extern int32 rtl8309n_l2_MacToIdx_get(uint8 *macAddr, uint32 fid, uint32* pIndex);

/* Function Name:
 *		rtl8309n_l2_macAddr_get
 * Description:
 *		Get mac address from l2 table entry
 * Input:
 *		entryAddr	-	entry address
 * Output:
 *		macAddr		-	pointer point to array of mac address
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *		In a RTL8309N l2 table entry, 48 bits mac address are set in the same order and place from data[47:0], regardless
 *		of whether it's a unicast entry or a multicast entry. So this API can be called to get 48 bits mac address from l2 
 *		table entry, and then whether the entry is a unicast entry or a multicast can be decided based on the mac address bit[47]
 */
extern int32 rtl8309n_l2_macAddr_get(uint32 entryAddr, uint8 *macAddr);

/* Function Name:
 *      rtl8309n_l2_ucastEntry_set
 * Description:
 *      write an unicast mac address into L2 table
 * Input: 
 *      entryAddr           -  the entry address
 *      pL2_data            -  pointer point to the struct describing the unicast entry
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :
 *          RTL8309N_LUT_AGEOUT     -   age out 
 *          RTL8309N_LUT_AGE100     -   100s
 *          RTL8309N_LUT_AGE200     -   200s
 *          RTL8309N_LUT_AGE300     -   300s
 */ 
extern int32 rtl8309n_l2_ucastEntry_set(uint32 entryAddr, rtl8309n_l2_ucastEntry_t *pL2_data);

/* Function Name:
 *      rtl8309n_l2_ucastEntry_get
 * Description:
 *      read an unicast entry from L2 table
 * Input:
 *      entryaddr       -  Specify the entry address to be read (0 ~ 2047)
 * Output:
 *      pL2Data         -  pointer point to the struct describes the unicast entry data
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Age time has 4 value :
 *          RTL8309N_LUT_AGEOUT     -   age out 
 *          RTL8309N_LUT_AGE100     -   100s
 *          RTL8309N_LUT_AGE200     -   200s
 *          RTL8309N_LUT_AGE300     -   300s
 */ 
extern int32 rtl8309n_l2_ucastEntry_get(uint32 entryAddr, rtl8309n_l2_ucastEntry_t *pL2_data);

#ifdef RTL8309N_LUT_CACHE
 
extern int32 rtl8309n_fastGetAsicLUTucastEntry(uint8 *macAddress, uint32 entryAddr, uint32 *pAge, uint32 *pIsStatic, uint32 *pIsAuth, uint32 *pPort) ;

extern int32 rtl8309n_fastGetAsicLUTmcastEntry(uint8 *macAddress, uint32 entryAddr, uint32 *pIsAuth, uint32 *pPortMask) ;

#endif
/* Function Name:
 *      rtl8309n_l2_mcastEntry_set
 * Description:
 *      write an multicast mac address into L2 table
 * Input:
 *      entryAddr  -  L2 table entry address
 *      pL2_data   -  pointer point to the multicast entry
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The auth for the look up table could be:
 *          RTL8309N_LUT_AUTH       -   authorizated
 *          RTL8309N_LUT_UNAUTH     -   unauthorizated
 */ 
extern int32 rtl8309n_l2_mcastEntry_set(uint32 entryAddr, rtl8309n_l2_mcastEntry_t *pL2_data);

/* Function Name:
 *      rtl8309n_l2_mcastEntry_get
 * Description:
 *      Get LUT multicast entry
 * Input:
 *      entryAddr         -  Specify the LUT entry address(0~2047)
 * Output:
 *      pL2_data          -  pointer point to the struct describing the multicast entry 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      The auth for the look up table could be:
 *          RTL8309N_LUT_AUTH       -   authorizated
 *          RTL8309N_LUT_UNAUTH     -   unauthorizated
 */  
extern int32 rtl8309n_l2_mcastEntry_get(uint32 entryAddr, rtl8309n_l2_mcastEntry_t *pL2_data);

/* Function Name:
 *      rtl8309n_l2_ucastAddr_add
 * Description:
 *     Add an unicast mac address, software will detect empty entry
 *Description:
 *		add a unicast entry into l2 table
 *Input:
 *      pMac    -   pointer point to MAC address
 *      fid     -   VLAN IVL ID
 *		pL2_data		-  pointer point to the struct describing the unicast entry data
 *Output:
 *		pEntryAddr   -  pointer point to the entry address
 *Return:
 *		SUCCESS
 *      FAILED
 *      RTL8309N_LUT_FULL
 * Note:
 *      (1)Age time has 4 value :
 *          RTL8309N_LUT_AGEOUT     -   age out
 *          RTL8309N_LUT_AGE100     -   100s 
 *          RTL8309N_LUT_AGE200     -   200s
 *          RTL8309N_LUT_AGE300     -   300s
 *      (2)The lut has a 4-way entry of an index. If the macAddress has existed in the lut, it will update the entry,
 *          otherwise the function will find an empty entry to put it.
 *          When the index is full, it will find a dynamic & unauth unicast macAddress entry to replace with it. 
 *          If the mac address has been written into LUT, function return value is SUCCESS,  *pEntryaddr is recorded the 
 *          entry address of the Mac address stored.
 *          If all the four entries can not be replaced, it will return a  RTL8309N_LUT_FULL error, you can delete one of them 
 *          and rewrite the unicast address.  
 */ 
extern int32 rtl8309n_l2_ucastAddr_add(uint8 *pMac, uint32 fid, rtl8309n_l2_ucastEntry_t* pL2_data, uint32* pEntryaddr);

/* Function Name:
 *      rtl8309n_l2_ucastAddr_get
 * Description:
 *      Get an unicast mac address information from l2 table
 * Input:
 *      pMac    -   pointer point to mac address
 *      fid     -   VLAN IVL ID
 * Output:
 *      pL2_data        -  pointer point to the struct describing the unicast entry
 *      pEntryAddr      -  pointer point to the unicast entry address
 * Return:
 *      SUCCESS
 *      FAILED
 *      RTL8309N_LUT_NOTEXIST
 * Note:
 *      (1)Age time has 4 value :
 *          RTL8309N_LUT_AGEOUT     -   age out
 *          RTL8309N_LUT_AGE100     -   100s 
 *          RTL8309N_LUT_AGE200     -   200s
 *          RTL8309N_LUT_AGE300     -   300s
 *      (2)The lut has a 4-way entry of an index. If the macAddress has existed in the lut, it will update the entry,
 *          otherwise the function will find an empty entry to put it.
 *          When the index is full, it will find a dynamic & unauth unicast macAddress entry to replace with it. 
 *          If the mac address has been written into LUT, function return value is SUCCESS,  *pEntryaddr is recorded the 
 *          entry address of the Mac address stored.
 *          If all the four entries can not be replaced, it will return a  RTL8309N_LUT_FULL error, you can delete one of them 
 *          and rewrite the unicast address.  
 */ 
extern int32 rtl8309n_l2_ucastAddr_get(uint8 *pMac, uint32 fid, rtl8309n_l2_ucastEntry_t *pL2_data, uint32 *pEntryAddr);

/* Function Name:
 *      rtl8309n_l2_ucastAddr_del
 * Description:
 *      Delete the specified Mac address, could be both unicast and multicast 
 * Input:
 *      pMac    -   pointer point to mac address
 *      fid     -   VLAN IVL ID
 * Output:
 *      pEntryAddr      -  pointer point to the entry address
 * Return:
 *      SUCCESS
 *      FAILED
 *      RTL8309N_LUT_NOTEXIST
 * Note:
 *      Use this function to delete a unicastMac address, it does not require to specify the 
 *      entry address. MAC and fid should be given as input parameter, which are needed to caculate 
 *      entery address for the entry to be deleted.if the Mac has existed in the LUT, it will be 
 *      deleted and function return value is SUCCESS.If the Mac is not existed in the LUT, the return 
 *      value is RTL8309N_LUT_NOTEXIST.
 */ 
extern int32 rtl8309n_l2_ucastAddr_del(uint8 *pMac, uint32 fid, uint32 *pEntryAddr);

/*Function Name:
 *		rtl8309n_l2_mcastAddr_add
 *Description:
 *		add a multicast entry into l2 table
 *Input:
 *		pMac    -   pointer point to mac address
 *      fid     -   VLAN IVL ID
 *      portmask    -   port mask of multicast address entry
 *Output:
 *		pEntryAddr    -  pointer point to the entry address
 *Return:
 *		SUCCESS
 *      FAILED
 *      RTL8309N_LUT_FULL
 *Note:
 *      (1)RTL8309N use mac and fid in hash alogrim to calculate 4 way index. After index is got, mac address
 *          within the 4 way entries can be capared with the input mac address one by one from entry 3 to entry 0. 
 *      (2)If the input mac address is already in the table, the corresponded entry will be updated. 
 *         If the input mac address is not in the table, a dynamic and unauthorised unicast entry will be found out
 *         and replaced by the input mac address entry.
 *      (3)The multicast address wrote by CPU is always authorizated.
 *      (4)If no dynamic and unauthorised unicast entry is found out, it means that the 4 way entries indexed by the
 *         index calculated are full. So RTL8309N_LUT_FULL will be returned.
 */
extern int32 rtl8309n_l2_mcastAddr_add(uint8 *pMac, uint32 fid, uint32 portmask, uint32 *pEntryaddr);

/* Function Name:
 *      rtl8309n_l2_multicastMac_get
 * Description:
 *      Get an multicast mac address information from l2 table
 * Input:
 *      pMac	-	pointer point to mac address
 *		fid		-	VLAN IVL ID
 * Output:
 *		pPortMask		-	port membership of a multicast address
 *      pEntryAddr      -   the pointer point to the multicast entry address                             
 * Return:
 *      SUCCESS
 *      FAILED
 *      RTL8309N_LUT_NOTEXIST
 *Note:
 *      (1)RTL8309N use mac and fid in hash alogrim to calculate 4 way index. After index is got, mac address
 *          within the 4 way entries can be capared with the input mac address one by one from entry 3 to entry 0. 
 *      (2)If the input mac address is already in the table, the corresponded entry will be updated. 
 *         If the input mac address is not in the table, a dynamic and unauthorised unicast entry will be found out
 *         and replaced by the input mac address entry.
 *      (3)The multicast address wrote by CPU is always authorizated.
 *      (4)If no dynamic and unauthorised unicast entry is found out, it means that the 4 way entries indexed by the
 *         index calculated are full. So RTL8309N_LUT_FULL will be returned.
 */ 
extern int32 rtl8309n_l2_mcastAddr_get(uint8* pMac, uint32 fid, uint32 *pPortmask, uint32 *pEntryAddr);

/* Function Name:
 *      rtl8309n_l2_multicastMac_del
 * Description:
 *      Delete the specified Mac address, could be both unicast and multicast 
 * Input:
 *      pMac	-	pointer point to mac address
 *		fid		-	VLAN IVL ID
 * Output:
 *		pEntryAddr	-	pointer point to the entry address of the deleted multicast entry 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Use this function to delete a multicastMac address, it does not require to specify the 
 *      entry address, if the Mac has existed in the LUT, it will be deleted and function
 *      return value is SUCCESS.If the Mac is not existed in the LUT, the return value is FAILED.
 */
extern int32 rtl8309n_l2_mcastAddr_del(uint8 *pMac, uint32 fid, uint32 *pEntryAddr);

/*Function Name:
 *		rtl8309n_l2_portMacLimitEnable_set
 *Description:
 *		Enable port mac learning limit function
 *Input:
 *		port	-	port number
 *		enabled	-	enable or disable
 *Output:
 *		none
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *
 */
extern int32 rtl8309n_l2_portMacLimitEnable_set(uint32 port, uint32 enabled);

/*Function Name:
 *		rtl8309n_l2_portMacLimitEnable_get
 *Description:
 *		Get status of port mac learning limit function
 *Input:
 *		port	-	port number
 *Output:
 *		pEnabled	-	pointer point to enable or disable
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *
 */
extern int32 rtl8309n_l2_portMacLimitEnable_get(uint32 port, uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_l2_portMacLimit_set
 * Description:
 *      Set per port mac limit counter max value
 * Input:
 *      port         -  port number (0 ~ 8)  
 *      maxVal    - auto learning MAC limit number
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
extern int32 rtl8309n_l2_portMacLimit_set(uint32 port, uint32 maxVal) ;

/* Function Name:
 *      rtl8309n_l2_portMacLimit_get
 * Description:
 *      Get per port mac limit counter max value
 * Input:
 *      port         -  port number (0 ~ 8)  
 * Output:
 *      pMacCnt     -  auto learning MAC limit number
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
extern int32 rtl8309n_l2_portMacLimit_get(uint32 port, uint32 *pMacCnt) ;

/*Function Name:
 *		rtl8309n_l2_systemMacLimitEnable_set
 *Description:
 *		Enable system mac learning limit function
 *Input:
 *		enabled		-	enable or disable
 *Output:
 *		none
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 */
extern int32 rtl8309n_l2_systemMacLimitEnable_set(uint32 enabled);

/*Function Name:
 *		rtl8309n_l2_systemMacLimitEnable_set
 *Description:
 *		Get the status of system mac learning limit function
 *Input:
 *		none
 *Output:
 *		pEnabled		-	pointer point to enable or disable
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 */
extern int32 rtl8309n_l2_systemMacLimitEnable_get(uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_l2_systemMacLimit_set
 * Description:
 *      Set the system mac limit counter max value
 * Input:
 *      maxVal    - system auto learning MAC limit number
 *      mergMask  -  port mask for the ports merged to system mac limit
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Besides per port mac limit function, 8306E also supports system MAC limit function.
 *      mergMask is to decide whitch ports are limited by system MAC limit function.
 *      For example, when system mac limit is enabled, and mergMask is 0x15(0b010101),
 *      that means the auto learning MAC number of port 0, port 2 and port 4 will also be
 *      influenced by system MAC limit.    
 */ 
extern int32 rtl8309n_l2_systemMacLimit_set(uint32 maxVal, uint32 mergMask) ;

/* Function Name:
 *      rtl8309n_l2_systemMacLimit_get
 * Description:
 *      Get the system mac limit counter value
 * Input:
 *      none
 * Output:
 *      pMacCnt    -   system auto learning MAC limit number
 *      pMergMask  -  port mask for the ports merged to system mac limit
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
extern int32 rtl8309n_l2_systemMacLimit_get(uint32 *pMacCnt, uint32 *pMergMask) ;

/* Function Name:
 *      rtl8309n_l2_macLimitAction_set
 * Description:
 *      Set the action taken by switch when auto learning MAC reach to the limit number
 * Input:
 *      action      -  the action taken when auto learning MAC reach to the max value 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      This API can be called to set action when mac limit counter exceed the max value
 *          RTL8309N_ACT_TRAP2CPU   -   trap to cpu
 *          RTL8309N_ACT_DROP       -   drop
 */ 
extern int32 rtl8309n_l2_macLimitAction_set(uint32 action) ;

/* Function Name:
 *      rtl8309n_l2_macLimitAction_get
 * Description:
 *      Get the action taken by switch when auto learning MAC reach to the limit number
 * Input:
 *      pAction      -  the action taken when auto learning MAC reach to the max value
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      This API can be called to get action when mac limit counter exceed the max value
 *          RTL8309N_ACT_TRAP2CPU   -   trap to cpu
 *          RTL8309N_ACT_DROP       -   drop
 */ 
extern int32 rtl8309n_l2_macLimitAction_get(uint32 *pAction) ;

/* Function Name:
 *      rtl8309n_l2_systemLearningCnt_get
 * Description:
 *      Get the current value of  system mac auto learning counter
 * Input:
 *      none
 * Output:
 *      pMacCnt		-	pointer point to value of system mac auto learning counter
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
extern int32 rtl8309n_l2_systemLearningCnt_get(uint32 *pMacCnt);

/* Function Name:
 *      rtl8309n_l2_systemLearningCnt_get
 * Description:
 *      Get the current value of  port mac auto learning counter
 * Input:
 *      port		-	port id
 * Output:
 *      pMacCnt		-	pointer point to value of port mac auto learning counter
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 */ 
extern int32 rtl8309n_l2_portlearningCnt_get(uint32 port, uint32 *pMacCnt);

/* Function Name:
 *		rtl8309n_l2_lruEnable_set
 * Description:
 *		Enable LRU ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_lruEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_l2_lruEnable_get
 * Description:
 *		Get enabled status of LRU ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_lruEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_l2_camEnable_set
 * Description:
 *		Enable CAM ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_camEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_l2_camEnable_get
 * Description:
 *		Get enabled status of CAM ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_camEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_l2_flushEnable_set
 * Description:
 *		Enable flushing l2 table ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_flushEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_l2_flushEnable_get
 * Description:
 *		Get enabled status of flushing l2 table ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_flushEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_l2_agingEnable_set
 * Description:
 *		Enable aging l2 table ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_agingEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_l2_agingEnable_get
 * Description:
 *		Get enabled status of aging l2 table ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_agingEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_l2_fastAgingEnable_set
 * Description:
 *		Enable fast aging l2 table ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_fastAgingEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_l2_fastAgingEnable_get
 * Description:
 *		Get enabled status of fast aging l2 table ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_fastAgingEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_l2_dpMskEnable_set
 * Description:
 *		Enable DP mask ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		When DP mask ability is enabled, one frame rx from port n can't tx from port n.
 */
extern int32 rtl8309n_l2_dpMskEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_l2_dpMskEnable_get
 * Description:
 *		Get enabled status of DP mark ability
 * Input:
 *		none
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_l2_dpMskEnable_get(uint32 *pEnabled);

/* Function Name:
 * 		rtl8309n_l2_unkownDaDropEnable_set
 * Description:
 *		Enable drop unkown Da unicast packets
 * Input:
 *		enabled		-	Enable or Disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
extern int32 rtl8309n_l2_unkownDaDropEnable_set(uint32 enabled);

/* Function Name:
 * 		rtl8309n_l2_unkownDaDropEnable_get
 * Description:
 *		Get enabled status of dropping unkown Da unicast packets
 * Input:
 *		none
 * Output:
 *		pEnabled		-	Enable or Disable
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
extern int32 rtl8309n_l2_unkownDaDropEnable_get(uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_l2_sablockAction_set
 * Description:
 *      Set action for packets blocked by sa block module
 * Input:
 *      action -  drop or trap to cpu
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
*/
extern int32 rtl8309n_l2_sablockAction_set(uint32 action);

/* Function Name:
 *      rtl8309n_l2_sablockAction_get
 * Description:
 *      get action for packets blocked by sa block module
 * Input:
 *      none
 * Output:
 *      pAction -  drop or trap to cpu
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
*/
extern int32 rtl8309n_l2_sablockAction_get(uint32 *pAction);

/* Function Name:
 *      rtl8309n_l2_dablockAction_set
 * Description:
 *      Set action for packets blocked by da block module
 * Input:
 *      action -  drop or trap to cpu
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
*/
extern int32 rtl8309n_l2_dablockAction_set(uint32 action);

/* Function Name:
 *      rtl8309n_l2_dablockAction_get
 * Description:
 *      get action for packets blocked by da block module
 * Input:
 *      none
 * Output:
 *      pAction -  drop or trap to cpu
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
*/
extern int32 rtl8309n_l2_dablockAction_get(uint32 *pAction);

/* Function Name:
 *      rtl8309n_stp_set
 * Description:
 *      Set IEEE 802.1d port state
 * Input:
 *      port   -  Specify port number (0 ~ 8)
 *      state -   Specify port state
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 port state:
 *         RTL8309N_SPAN_DISABLE    - Disable state
 *         RTL8309N_SPAN_BLOCK      - Blocking state
 *         RTL8309N_SPAN_LEARN      - Learning state
 *         RTL8309N_SPAN_FORWARD    - Forwarding state        
 */ 
extern int32 rtl8309n_stp_set(uint32 port, uint32 state) ;

/* Function Name:
 *      rtl8309n_stp_get
 * Description:
 *      Get IEEE 802.1d port state
 * Input:
 *      port    -  Specify port number (0 ~ 5)
 * Output:
 *      pState -  get port state
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are 4 port state:
 *         RTL8309N_SPAN_DISABLE   - Disable state
 *         RTL8309N_SPAN_BLOCK      - Blocking state
 *         RTL8309N_SPAN_LEARN      - Learning state
 *         RTL8309N_SPAN_FORWARD - Forwarding state
 */ 
extern int32 rtl8309n_stp_get(uint32 port, uint32 *pState) ;

/* Function Name:
 *		rtl8309n_dot1x_portBasedEnable_set
 * Description:
 *		Enable DOT1X port based access control
 * Input:
 *		port	-	port number(0~8)
 *		enabled -	enable or disable
 * Output:
 *		none
 * Return:
 *		FAILED
 *      SUCCESS
 * Note:
 */
extern int32 rtl8309n_dot1x_portBasedEnable_set(uint32 port, uint32 enabled);

/* Function Name:
 *		rtl8309n_dot1x_portBasedEnable_get
 * Description:
 *		Get enabled status of DOT1X port based access control ability
 * Input:
 *		port	-	port number(0~8)
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		FAILED
 *      SUCCESS
 * Note:
 */
extern int32 rtl8309n_dot1x_portBasedEnable_get(uint32 port, uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_dot1x_macBasedEnable_set
 * Description:
 *		Enable DOT1X mac based access control
 * Input:
 *		port	-	port number(0~8)
 *		enabled -	enable or disable
 * Output:
 *		none
 * Return:
 *		FAILED
 *      SUCCESS
 * Note:
 */
extern int32 rtl8309n_dot1x_macBasedEnable_set(uint32 port, uint32 enabled);

/* Function Name:
 *		rtl8309n_dot1x_macBasedEnable_get
 * Description:
 *		Get enabled status of DOT1X mac based access control
 * Input:
 *		port	-	port number(0~8)
 * Output:
 *		pEnabled -	enable or disable
 * Return:
 *		FAILED
 *      SUCCESS
 * Note:
 */
extern int32 rtl8309n_dot1x_macBasedEnable_get(uint32 port, uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_dot1x_portBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 8)
 *      isAuth      -   Authorized or unauthorized state 
 *      direction   -    set IEEE802.1x port-based control direction
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are two IEEE802.1x port state:
 *         RTL8309N_PORT_AUTH      - authorized
 *         RTL8309N_PORT_UNAUTH  - unauthorized
 *
 *     There are also two 802.1x port-based control direction:
 *         RTL8309N_PORT_BOTHDIR - if port-base access control is enabled, 
 *                                              forbid forwarding this port's traffic to unauthorized port
 *         RTL8309N_PORT_INDIR     - if port-base access control is enabled, permit forwarding this
 *                                              port's traffic to unauthorized port
 */ 
extern int32 rtl8309n_dot1x_portBased_set(uint32 port, uint32 isAuth, uint32 direction);

/* Function Name:
 *      rtl8309n_dot1x_portBased_set
 * Description:
 *      Set IEEE802.1x port-based access control
 * Input:
 *      port         -  Specify port number (0 ~ 8)
 * Output:
 *      pIsAuth      - the pointer of authorized or unauthorized state 
 *      pDirection   - the pointer of IEEE802.1x port-based control direction
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     There are two IEEE802.1x port state:
 *         RTL8309N_PORT_AUTH      - authorized
 *         RTL8309N_PORT_UNAUTH  - unauthorized
 *     There are also two 802.1x port-based control direction:
 *         RTL8309N_PORT_BOTHDIR - if port-base access control is enabled, 
 *                                              forbid forwarding this port's traffic to unauthorized port
 *         RTL8309N_PORT_INDIR     - if port-base access control is enabled, permit forwarding this
 *                                              port's traffic to unauthorized port
 */ 
extern int32 rtl8309n_dot1x_portBased_get(uint32 port, uint32 *pIsAuth, uint32 *pDirection) ;

/* Function Name:
 *      rtl8309n_dot1x_macBased_set
 * Description:
 *      Set IEEE802.1x mac-based access control
 * Input:
 *      direction    -  IEEE802.1x mac-based access control direction
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)The mac address authentication status is saved in L2 table entry,
 *         it should be set by software.
 *      (2)there are also two mac-based control directions which are not per 
 *         port but global configurtion:
 *              RTL8309N_MAC_BOTHDIR - if Mac-based access control is enabled, packet with 
 *                 unauthorized DA will be dropped.
 *              RTL8309N_MAC_INDIR   - if Mac-based access control is enabled, packet with 
 *                 unauthorized DA will pass mac-based access control igress rule.
 */ 
extern int32 rtl8309n_dot1x_macBased_set(uint32 direction);

/* Function Name:
 *      rtl8309n_dot1x_macBased_get
 * Description:
 *      Get IEEE802.1x port-based access control ability status
 * Input:
 *		none
 * Output:
 *      pDirection   -  pointer point to IEEE802.1x mac-based access control direction
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)The mac address authentication status is saved in L2 table entry,
 *         it should be set by software.
 *      (2)there are also two mac-based control directions which are not per 
 *         port but global configurtion:
 *              RTL8309N_MAC_BOTHDIR - if Mac-based access control is enabled, packet with 
 *                 unauthorized DA will be dropped.
 *              RTL8309N_MAC_INDIR   - if Mac-based access control is enabled, packet with 
 *                 unauthorized DA will pass mac-based access control igress rule.
 */
extern int32 rtl8309n_dot1x_macBased_get(uint32 *pDirection) ;

/* Funtion Name:
 *      rtl8309n_dot1x_unauthPktAction_set
 * Description:
 *      Set action for dot1x unauthorised packets
 * Input:
 *      action  -   drop or trap to cpu
 * Output:
 *      none
 * Return:
 *      FAILED
 *      SUCCESS
 * Note:
 */
extern int32 rtl8309n_dot1x_unauthPktAction_set(uint32 action);

/* Funtion Name:
 *      rtl8309n_dot1x_unauthPktAction_get
 * Description:
 *      Set action for dot1x unauthorised packets
 * Input:
 *      none
 * Output:
 *      pAction     -   pointer pointed to action for unauthorised packets
 * Return:
 *      FAILED
 *      SUCCESS
 * Note:
 */
extern int32 rtl8309n_dot1x_unauthPktAction_get(uint32 *pAction);


/* Function Name:
 *		rtl8309n_igmp_enable_set
 * Description:
 *		Enable IGMP/MLD snooping and PPPOE bypass ability
 * Input:
 *		type	-	IGMP/MLD, PPPOE
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)Enable the ASIC to parse IGMP/MLD or PPPOE packets. The action to take when the packets 
 *		   are received and detected is set by RMA function.
 *      (2)type could be:
 *              RTL8309N_IGMP   -   IGMP packets
 *              RTL8309N_MLD    -   MLD packets
 *              RTL8309N_PPPOE_IPV4     -   PPPOE IGMP packets
 *              RTL8309N_PPPOE_MLD      -   PPPOE MLD packets
 */
extern int32 rtl8309n_igmp_enable_set(uint32 type, uint32 enabled);

/* Function Name:
 *		rtl8309n_igmp_enable_get
 * Description:
 *		Get Enabled status of IGMP/MLD snooping and PPPOE bypass ability
 * Input:
 *		type	-	IGMP/MLD, PPPOE
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)Enable the ASIC to parse IGMP/MLD or PPPOE packets. The action to take when the packets 
 *		   are received and detected is set by RMA function.
 *      (2)type could be:
 *              RTL8309N_IGMP   -   IGMP packets
 *              RTL8309N_MLD    -   MLD packets
 *              RTL8309N_PPPOE_IPV4     -   PPPOE IGMP packets
 *              RTL8309N_PPPOE_MLD      -   PPPOE MLD packets
 */
extern int32 rtl8309n_igmp_enable_get(uint32 type, uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_trap_igmpCtrlPktAction_set
 * Description:
 *      Set IGMP/MLD trap function
 * Input:
 *      type         -  Specify IGMP/MLD or PPPOE
 *      action       -  Action could be normal forward or trap
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     type could be:
 *              RTL8309N_IGMP   -   IGMP packets
 *              RTL8309N_MLD    -   MLD packets
 *              RTL8309N_PPPOE_IPV4     -   PPPOE IGMP packets
 *              RTL8309N_PPPOE_MLD      -   PPPOE MLD packets
 *     action could be:
 *          RTL8309N_ACT_PERMIT    - normal forward
 *          RTL8309N_ACT_TRAP2CPU  - trap to cpu
 *			RTL8309N_ACT_COPY2CPU  - copy to cpu
 *			RTL8309N_ACT_DROP	   - drop
 */ 
extern int32 rtl8309n_trap_igmpCtrlPktAction_set(uint32 type, uint32 action);

/* Function Name:
 *      rtl8309n_trap_igmpCtrlPktAction_get
 * Description:
 *      Get IGMP/MLD trap function status
 * Input:
 *      type         -  Specify IGMP/MLD or PPPOE
 * Output:
 *      pAction       -  Action could be normal forward or trap
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *     type could be:
 *              RTL8309N_IGMP   -   IGMP packets
 *              RTL8309N_MLD    -   MLD packets
 *              RTL8309N_PPPOE_IPV4     -   PPPOE IGMP packets
 *              RTL8309N_PPPOE_MLD      -   PPPOE MLD packets
 *     action could be:
 *          RTL8309N_ACT_PERMIT    - normal forward
 *          RTL8309N_ACT_TRAP2CPU  - trap to cpu
 *			RTL8309N_ACT_COPY2CPU  - copy to cpu
 *			RTL8309N_ACT_DROP	   - drop
 */ 
extern int32 rtl8309n_trap_igmpCtrlPktAction_get(uint32 type, uint32 *pAction);

/* Function Name:
 *      rtl8309n_trap_unknownIPMcastPktAction_set
 * Description:
 *      Set unknown ip multicast drop or normal forward
 * Input:
 *      type         -  Specify ipv4 or ipv6 unkown multicast
 *      action       -  drop or normal forward
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8309N_MULCAST_IPV4 - ipv4 unknown multicast
 *          RTL8309N_MULCAST_IPV6 - ipv6 unknown multicast
 *      action could be:
 *          RTL8309N_ACT_DROP      - trap to cpu 
 *          RTL8309N_ACT_PERMIT   - normal forward
 */  
extern int32 rtl8309n_trap_unknownIPMcastPktAction_set(uint32 type, uint32 action);

/* Function Name:
 *      rtl8309n_trap_unknownIPMcastPktAction_set
 * Description:
 *      Set unknown ip multicast drop or normal forward
 * Input:
 *      type         -  Specify ipv4 or ipv6 unkown multicast
 * Output:
 *      pAction		 -  drop or normal forward
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8309N_MULCAST_IPV4 - ipv4 unknown multicast
 *          RTL8309N_MULCAST_IPV6 - ipv6 unknown multicast
 *      action could be:
 *          RTL8309N_ACT_DROP      - trap to cpu 
 *          RTL8309N_ACT_PERMIT   - normal forward
 */  
extern int32 rtl8309n_trap_unknownIPMcastPktAction_get(uint32 type, uint32 *pAction);

/* Function Name:
 *      rtl8309n_trap_abnormalPktAction_set
 * Description:
 *      set abnormal packet action 
 * Input:
 *      type         -  abnormal packet type
 *      action       -  drop or trap to cpu
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8309N_UNMATCHVID   - vlan-tagged packet, vid dismatch vlan table 
 *          RTL8309N_DOT1XUNAUTH  - 802.1x authentication fail packet
 *          RTL8309N_LUTSABLOCK   - macth look up table SMAC block entry
 *   		RTL8309N_LUTDABLOCK   - match look up table DMAC block entry
 *      action could be:
 *          RTL8309N_ACT_DROP       - drop 
 *          RTL8309N_ACT_TRAP2CPU    - trap to cpu
 *			RTL8309N_ACT_PERMIT		-	normal forward
 */ 
extern int32 rtl8309n_trap_abnormalPktAction_set(uint32 type,  uint32 action);

/* Function Name:
 *      rtl8309n_trap_abnormalPktAction_get
 * Description:
 *      get abnormal packet action 
 * Input:
 *      type         -  abnormal packet type
 * Output:
 *      pAction     -  the pointer of action
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      type coulde be:
 *          RTL8309N_UNMATCHVID   - vlan-tagged packet, vid dismatch vlan table 
 *          RTL8309N_DOT1XUNAUTH  - 802.1x authentication fail packet
 *          RTL8309N_LUTSABLOCK   - macth look up table SMAC block entry
 *   		RTL8309N_LUTDABLOCK   - match look up table DMAC block entry
 *      action could be:
 *          RTL8309N_ACT_DROP       - drop 
 *          RTL8309N_ACT_TRAP2CPU    - trap to cpu
 */ 
extern int32 rtl8309n_trap_abnormalPktAction_get(uint32 type,  uint32 *pAction);

/* Function Name:
 *      rtl8309n_rma_entry_set
 * Description:
 *      Set reserved multicast Mac address forwarding behavior
 * Input:
 *      type         -  reserved Mac address type
 *      pRmaentry    -  pointer point to struct describes the RMA entry 
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)There are eight types  reserved addresses which user can set asic to determine 
 *      how to forwarding them:
 *      RTL8309N_RMA00, 	 reserved address "01-80-c2-00-00-00"
 *      RTL8309N_RMA01,	   reserved address "01-80-c2-00-00-01"
 *      RTL8309N_RMA02, 	 reserved address "01-80-c2-00-00-02"
 *      RTL8309N_RMA03, 	 reserved address "01-80-c2-00-00-03"
 *		RTL8309N_RMA04_0D0F,	 reserved address "01-80-C2-00-00-04 --- 01-80-C2-00-00-0D" 
 *						 "01-80-C2-00-00-0F" 
 *		RTL8309N_RMA0E, 	 reserved address "01-80-C2-00-00-0E"
 *		RTL8309N_RMA10, 	  reserved address "01-80-c2-00-00-10"
 *		RTL8309N_RMA11_1F,	  reserved address "01-80-C2-00-00-11 --- 01-80-C2-00-00-1F"
 *		RTL8309N_RMA20, 	  reserved address "01-80-c2-00-00-20"
 *		RTL8309N_RMA21, 	  reserved address "01-80-c2-00-00-21"
 *		RTL8309N_RMA22_2F,	  reserved address "01-80-c2-00-00-22 --- 01-80-c2-00-00-2F"
 *		RTL8309N_RMA31_3F,	  reserved address "01-80-c2-00-00-31 --- 01-80-c2-00-00-3F" 
 *		RTL8309N_RMAIGMP,	  IGMP packet without pppoe header
 *		RTL8309N_RMAMLD,	  MLD packet without pppoe header
 *		RTL8309N_RMAPPPOE_IPV4,   IGMP packet with pppoe header
 *		RTL8309N_RAMPPPOE_MLD,	  MLD packet with pppoe header
 *		RTL8309N_RMASWITCH_MAC,   RMA = switch MAC
 *		RTL8309N_RMAUNKNOWN_UNIDA,	  reserved address "xxxxxxx0-xx-xx-xx-xx-xx"
 *		RTL8309N_RMAUNKNOWN_MULTDA,   reserved address "xxxxxxx1-xx-xx-xx-xx-xx"
 *
 *      (2) data of rmaentry are :
 *          action:	
 *            RTL8306_ACT_FORWORD      - Drop the packet
 *            RTL8306_ACT_COPY         - Copy the packet to cpu
 *			  RTL8309N_ACT_TRAP2CPU      - Trap the packet to CPU
 *            RTL8309N_ACT_FLOOD        - Flood the packet
 *          enable_rmapri:  enable or disable the predefined RMA priority
 *          priority:  the predefined priority
 */ 
extern int32 rtl8309n_rma_entry_set(uint32 type, rtl8309n_rma_entry_t *pRmaentry);

/* Function Name:
 *      rtl8309n_rma_entry_get
 * Description:
 *      Get reserved multicast Mac address forwarding behavior
 * Input:
 *      type         -  reserved Mac address type
 * Output:
 *      pAction     -  the pointer of action
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      (1)There are eight types  reserved addresses which user can set asic to determine 
 *      how to forwarding them:
 *      RTL8309N_RMA00, 	 reserved address "01-80-c2-00-00-00"
 *      RTL8309N_RMA01	   reserved address "01-80-c2-00-00-01"
 *      RTL8309N_RMA02, 	 reserved address "01-80-c2-00-00-02"
 *      RTL8309N_RMA03, 	 reserved address "01-80-c2-00-00-03"
 *		RTL8309N_RMA04_0D0F,	 reserved address "01-80-C2-00-00-04 --- 01-80-C2-00-00-0D" 
 *						 "01-80-C2-00-00-0F" 
 *		RTL8309N_RMA0E, 	 reserved address "01-80-C2-00-00-0E"
 *		RTL8309N_RMA10, 	  reserved address "01-80-c2-00-00-10"
 *		RTL8309N_RMA11_1F,	  reserved address "01-80-C2-00-00-11 --- 01-80-C2-00-00-1F"
 *		RTL8309N_RMA20, 	  reserved address "01-80-c2-00-00-20"
 *		RTL8309N_RMA21, 	  reserved address "01-80-c2-00-00-21"
 *		RTL8309N_RMA22_2F,	  reserved address "01-80-c2-00-00-22 --- 01-80-c2-00-00-2F"
 *		RTL8309N_RMA31_3F,	  reserved address "01-80-c2-00-00-31 --- 01-80-c2-00-00-3F" 
 *		RTL8309N_RMAIGMP,	  IGMP packet without pppoe header
 *		RTL8309N_RMAMLD,	  MLD packet without pppoe header
 *		RTL8309N_RMAPPPOE_IPV4,   IGMP packet with pppoe header
 *		RTL8309N_RAMPPPOE_MLD,	  MLD packet with pppoe header
 *		RTL8309N_RMASWITCH_MAC,   RMA = switch MAC
 *		RTL8309N_RMAUNKNOWN_UNIDA,	  reserved address "xxxxxxx0-xx-xx-xx-xx-xx"
 *		RTL8309N_RMAUNKNOWN_MULTDA,   reserved address "xxxxxxx1-xx-xx-xx-xx-xx"
 *
 *      (2) data of rmaentry are :
 *          action:	
 *            RTL8306_ACT_FORWORD      - Drop the packet
 *            RTL8306_ACT_COPY         - Copy the packet to cpu
 *			  RTL8309N_ACT_TRAP2CPU      - Trap the packet to CPU
 *            RTL8309N_ACT_FLOOD        - Flood the packet
 *          enable_rmapri:  enable or disable the predefined RMA priority
 *          priority:  the predefined priority
 */ 
extern int32 rtl8309n_rma_entry_get(uint32 type, rtl8309n_rma_entry_t *pRmaentry);

/*Function Name:
 *     rtl8309n_rma_userentry_set
 *Description:
 *     set user defined RMA entry
 *Input:
 *     type       -  reserved MAC address type
 *     rmaentry   -  struct describes the RMA entry
 *     macAddr    -  pointer point to MAC address
 *Output:
 *     none
 *Return:
 *     SUCCESS
 *     FAILED
 *Note
 *     User can set a user defined RMA entry. A user defined RMA entry includes MAC address, RMA entry data.
 *     This API can be called to write a user defined entry to RMA entry table.
 */
extern int32 rtl8309n_rma_userentry_set(rtl8309n_rma_entry_t *pRmaentry, uint8 *macAddr);

/*Function Name:
 *     rtl8309n_rma_userentry_set
 *Description:
 *     Get user defined RMA entry
 *Input:
 *     none
 *Output:
 *     pRmaentry   -  pointer point to struct describes the RMA entry
 *     macAddr     -  pointer point to MAC address
 *Return:
 *     SUCCESS
 *     FAILED
 *Note
 *     User can set a user defined RMA entry. A user defined RMA entry includes MAC address, RMA entry data.
 *     This API can be called to write a user defined entry to RMA entry table.
 */
extern int32 rtl8309n_rma_userentry_get(rtl8309n_rma_entry_t *pRmaentry, uint8 *macAddr);

/* Function Name:
 *      rtl8309n_intr_enable__set
 * Description:
 *      Enable asic interrupt 
 * Input:
 *      enInt        -  Enable interrupt cpu
 *      intmask      -  interrupt event  mask
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Interrupt mask register:
 *         bit15: enable interrupt CPU
 *         bit13: enable tx meter interrupt
 *         bit12: enable unmatched sa interrupt
 *         bit11: enable wakenup frame interrupt
 *         bit10: enable loop detection interrupt
 *         bit9:  enable storm filter interrupt
 *         bit8:  enable port8 link change interrupt
 *         bit7:  enable port7 link change interrupt
 *         bit6:  enable port6 link change interrupt
 *         bit5:  enable port5 link change interrupt
 *         bit4:  enable port4 link change interrupt
 *         bit3:  enable port3 link change interrupt
 *         bit2:  enable port2 link change interrupt
 *         bit1:  enable port1 link change interrupt
 *         bit0:  enable port0 link change interrupt
 */ 
extern int32 rtl8309n_intr_enable_set(uint32 enInt, uint32 intmask);

/* Function Name:
 *      rtl8309n_intr_enable_get
 * Description:
 *      Get Asic interrupt enabling status
 * Input:
 *      none 
 * Output:
 *      pEnInt       -  the pointer of  interrupt global enable bit
 *      pIntmask    -  the pointer of interrupt event  mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Interrupt mask register:
 *         bit15: enable interrupt CPU
 *         bit13: enable tx meter interrupt
 *         bit12: enable unmatched sa interrupt
 *         bit11: enable wakenup frame interrupt
 *         bit10: enable loop detection interrupt
 *         bit9:  enable storm filter interrupt
 *         bit8:  enable port8 link change interrupt
 *         bit7:  enable port7 link change interrupt
 *         bit6:  enable port6 link change interrupt
 *         bit5:  enable port5 link change interrupt
 *         bit4:  enable port4 link change interrupt
 *         bit3:  enable port3 link change interrupt
 *         bit2:  enable port2 link change interrupt
 *         bit1:  enable port1 link change interrupt
 *         bit0:  enable port0 link change interrupt
 */ 
extern int32 rtl8309n_intr_enable_get(uint32 *pEnInt, uint32 *pIntmask) ;

/* Function Name:
 *      rtl8309n_intr_control_set
 * Description:
 *      set Asic interrupt
 * Input:
 *      intrmask    -   interrupt mask 
 * Output:
 *      none 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Interrupt status register:
 *         bit12: unmatched sa interrupt
 *         bit11: wakenup frame interrupt
 *         bit10: loop detection interrupt
 *         bit9:  storm filter interrupt
 *         bit8:  port8 link change interrupt
 *         bit7:  port7 link change interrupt
 *         bit6:  port6 link change interrupt
 *         bit5:  port5 link change interrupt
 *         bit4:  port4 link change interrupt
 *         bit3:  port3 link change interrupt
 *         bit2:  port2 link change interrupt
 *         bit1:  port1 link change interrupt
 *         bit0:  port0 link change interrupt
 */ 
extern int32 rtl8309n_intr_status_set(uint32 intrmask);

/* Function Name:
 *      rtl8309n_intr_control_get
 * Description:
 *      Get Asic interrupt
 * Input:
 *      none 
 * Output:
 *      pEnInt       -  the pointer of  interrupt global enable bit
 *      pIntmask    -  the pointer of interrupt event  mask 
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      Interrupt status register:
 *         bit13: tx meter interrupt
 *         bit12: unmatched sa interrupt
 *         bit11: wakenup frame interrupt
 *         bit10: loop detection interrupt
 *         bit9:  storm filter interrupt
 *         bit8:  port8 link change interrupt
 *         bit7:  port7 link change interrupt
 *         bit6:  port6 link change interrupt
 *         bit5:  port5 link change interrupt
 *         bit4:  port4 link change interrupt
 *         bit3:  port3 link change interrupt
 *         bit2:  port2 link change interrupt
 *         bit1:  port1 link change interrupt
 *         bit0:  port0 link change interrupt
 */ 
extern int32 rtl8309n_intr_status_get(uint32 *pStatusMask) ;

/* Function Name:
 *      rtl8309n_storm_filterEnable_set
 * Description:
 *      Enable Asic storm filter 
 * Input:
 *      port      -  port number(0-8)
 *      type      -  specify storm filter type
 *      enabled   -  TRUE or FALSE
 * Output:
 *      none
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 4 kinds of storm filter:
 *          RTL8309N_BROADCASTPKT       -   storm filter for broadcast packet
 *          RTL8309N_MULTICASTPKT       -   storm filter for all multicast packet, include known and unknown
 *          RTL8309N_UNKOWN_UNIDAPKT    -   storm filter for unknown unicast DA packet
 *          RTL8309N_UNKOWN_MULTDAPKT   -   storm filter for unknown multicast DA packet
 */ 
extern int32 rtl8309n_storm_filterEnable_set(uint32 port, uint32 type, uint32 enabled);

/* Function Name:
 *      rtl8309n_storm_filterEnable_get
 * Description:
 *      Get Asic storm filter enabled or disabled 
 * Input:
 *      port      -  port number(0-8)
 *      type      -  specify storm filter type
 * Output:
 *      pEnabled  -  the pointer of enabled or disabled
 * Return:
 *      SUCCESS
 *      FAILED
 * Note:
 *      There are 4 kinds of storm filter:
 *          RTL8309N_BROADCASTPKT       -   storm filter for broadcast packet
 *          RTL8309N_MULTICASTPKT       -   storm filter for all multicast packet, include known and unknown
 *          RTL8309N_UNKOWN_UNIDAPKT    -   storm filter for unknown unicast DA packet
 *          RTL8309N_UNKOWN_MULTDAPKT   -   storm filter for unknown multicast DA packet
 */
extern int32 rtl8309n_storm_filterEnable_get(uint32 port, uint32 type, uint32 *pEnabled);

/*Function Name:
 *		rtl8309n_storm_filterAttr_set
 *Description:
 *		set storm filter attributes
 *Input:
 *		port	-	port number (0 - 8)
 *		storm	-	storm filter traffic type
 *		pStorm_data		-	pointer point to the struct describing the storm filter attributes
 *Output:
 *		none
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *		(1)There are 4 types of traffic that could be filtered by the RTL8309N storm filetr. So storm_type
 *			could be any values as follows:
 *			RTL8309N_BROADCASTPKT	-	broadcast tarffic
 *			RTL8309N_MULTICASTPKT	-	both known and unknown multicast traffic
 *			RTL8309N_UNKOWN_UNIDAPKT		-	unicast traffic
 *			RTL8309N_UNKOWN_MULTDAPKT	-	unknown multicast traffic
 *		(2)This API can be called to set storm filter counter unit, rate, burst size and whether inclue ifg when calculate
 *			rate.
 */
extern int32 rtl8309n_storm_filterAttr_set(uint32 port, uint32 storm_type, rtl8309n_storm_attr_t *pStorm_data);

/*Function Name:
 *		rtl8309n_storm_filterAttr_get
 *Description:
 *		Get storm filter attributes
 *Input:
 *		port	-	port number (0 - 8)
 *		storm	-	storm filter traffic type
 *Output:
 *		pStorm_data		-	pointer point to the struct describing the storm filter attributes
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *		(1)There are 4 types of traffic that could be filtered by the RTL8309N storm filetr. So storm_type
 *			could be any values as follows:
 *			RTL8309N_BROADCASTPKT	-	broadcast tarffic
 *			RTL8309N_MULTICASTPKT	-	both known and unknown multicast traffic
 *			RTL8309N_UNKOWN_UNIDAPKT		-	unicast traffic
 *			RTL8309N_UNKOWN_MULTDAPKT	-	unknown multicast traffic
 *		(2)This API can be called to get storm filter counter unit, rate, burst size and whether inclue ifg when calculate
 *			rate.
 */
extern int32 rtl8309n_storm_filterAttr_get(uint32 port, uint32 storm_type, rtl8309n_storm_attr_t *pStorm_data);

/*Function Name:
 *		rtl8309n_storm_filterStatus_set
 *Description:
 *		clear storm counter exceeding status
 *Input:
 *		port	-	port number (0 - 8)
 *		storm_type	-	storm traffic type
 *		enabled		-	enable clearing storm counter exceeding status
 *Output:
 *		none
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *		(1)When the dedicated traffic exceed the storm filter rate for one port, it means that the 
 *		    the dedicated traffic storm has happened for this port. And the exceed status will be set to 1
 *		    until it is cleared by software. When software write 1 to the exceed bit, the exceeding status 
 *		    will be cleared.
 *		(2)There are 4 types of storm_type, and they could be any values as follows:
 *			RTL8309N_BROADCASTPKT	-	broadcast tarffic
 *			RTL8309N_MULTICASTPKT	-	both known and unknown multicast traffic
 *			RTL8309N_UNKOWN_UNIDAPKT		-	unicast traffic
 *			RTL8309N_UNKOWN_MULTDAPKT	-	unknown multicast traffic
 */
extern int32 rtl8309n_storm_filterStatus_set(uint32 port, uint32 storm_type, uint32 enabled);

/*Function Name:
 *		rtl8309n_storm_filterStatus_get
 *Description:
 *		Get storm counter exceeding status
 *Input:
 *		port	-	port number (0 - 8)
 *		storm_type	-	storm traffic type
 *		pStatus	-	storm counter exceeding status
 *Output:
 *		none
 *Return:
 *		SUCCESS
 *		FAILED
 *Note:
 *		(1)When the dedicated traffic exceed the storm filter rate for one port, it means that the 
 *		    the dedicated traffic storm has happened for this port. And the exceed status will be set to 1
 *		    until it is cleared by software. When software write 1 to the exceed bit, the exceeding status 
 *		    will be cleared.
 *		(2)There are 4 types of storm_type, and they could be any values as follows:
 *			RTL8309N_BROADCASTPKT	-	broadcast tarffic
 *			RTL8309N_MULTICASTPKT	-	both known and unknown multicast traffic
 *			RTL8309N_UNKOWN_UNIDAPKT		-	unicast traffic
 *			RTL8309N_UNKOWN_MULTDAPKT	-	unknown multicast traffic
 */
extern int32 rtl8309n_storm_filterStatus_get(uint32 port, uint32 storm_type, uint32 *pStatus);

/* RLDP/RLPP */
/* Function Name:
 *		rtl8309n_rldp_enable_set
 * Description:
 *		Enable RLDP ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_enable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_rldp_enable_set
 * Description:
 *		Get RLDP ability status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_enable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_rldp_priEnable_set
 * Description:
 *		Enable RLDP priority ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_priEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_rldp_priEnable_set
 * Description:
 *		Get RLDP priority ability status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_priEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_rldp_ttlEnable_set
 * Description:
 *		Enable RLDP ttl timer ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_ttlEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_rldp_ttlEnable_get
 * Description:
 *		Get RLDP ttl timer ability status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_ttlEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_rldp_stormFilterEnable_set
 * Description:
 *		Enable RLDP storm filter ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_stormFilterEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_rldp_stormfilterEnable_get
 * Description:
 *		Get RLDP storm filter ability status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_stormFilterEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_rldp_vlanTgaEnable_set
 * Description:
 *		Enable RLDP vlan tag ability
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_vlanTagEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_rldp_vlanTagEnable_get
 * Description:
 *		Get RLDP vlan tag ability status
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_vlanTagEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_rldp_controlAttr_set
 * Description:
 * 		Set RLDP/RLPP attributes
 * Input:
 *		pRldp_data		-	pointer point to the struct describing the RLDP/RLPP attributes
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *		(1)Before RLDP/RLPP function work properly, RLDP/RLPP frame transmit timer, priority, TTL ,
 *		    switch mac address have to set correctly.
 *      (2)The timer(RLDP Frame Transmit Time Period) could be:
 *              RTL8309N_RLDP_TIMER_3_5MIN
 *              RTL8309N_RLDP_TIMER_100S
 *              RTL8309N_RLDP_TIMER_10S
 *              RTL8309N_RLDP_TIMER_1S              
 */
extern int32 rtl8309n_rldp_controlAttr_set(rtl8309n_rldp_controlAttr_t *pRldp_data);

/* Function Name:
 *		rtl8309n_rldp_controlAttr_Get
 * Description:
 * 		Get RLDP/RLPP attributes
 * Input:
 *		none
 * Output:
 *		pRldp_data		-	pointer point to the struct describing the RLDP/RLPP attributes
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *		(1)Before RLDP/RLPP function work properly, RLDP/RLPP frame transmit timer, priority, TTL ,
 *		    switch mac address have to set correctly.
 *      (2)The timer(RLDP Frame Transmit Time Period) could be:
 *              RTL8309N_RLDP_TIMER_3_5MIN
 *              RTL8309N_RLDP_TIMER_100S
 *              RTL8309N_RLDP_TIMER_10S
 *              RTL8309N_RLDP_TIMER_1S              
 */
extern int32 rtl8309n_rldp_controlAttr_get(rtl8309n_rldp_controlAttr_t *pRldp_data);

/* Function Name:
 *		rtl8309n_rlpp_action_set
 * Description:
 *		Set RLPP frame action
 * Input:
 *		action	-	action on RLPP frame
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)Action for RLPP frame could only be one of the value as follows:
 *		      RTL8309N_ACT_TRAP2CPU   -   trap to cpu
 *		      RTL8309N_ACT_PERMIT     -   forward as normal
 *		(2)When RTL8309N receive the RLPP frame, it will either trap the frame to CPU or forward it
 *		normally.
 */
extern int32 rtl8309n_rlpp_action_set(uint32 action);

/* Function Name:
 *		rtl8309n_rlpp_action_get
 * Description:
 *		Get RLPP frame action
 * Input:
 *		none
 * Output:
 *		pAction		-	action on RLPP frame
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		Action for RLPP frame could only be one of the value as follows:
 *		      RTL8309N_ACT_TRAP2CPU   -   trap to cpu
 *		      RTL8309N_ACT_PERMIT     -   forward as normal
 *		When RTL8309N receive the RLPP frame, it will either trap the frame to CPU or forward it
 *		normally.
 */
extern int32 rtl8309n_rlpp_action_get(uint32 *pAction);

/* Function Name:
 *      rtl8309n_rldp_identifierEnable_set
 * Description:
 *      enable calculate and put identifier into RLDPP packet
 * Input:
 *      enabled     -   enable or disable
 * Output:
 *      none
 * Return:
 *      SUCCESS
 * Note:
 *      When identifier is enabled, ASIC will calculate identifier and put it into RLDP 
 *      packet. When ASIC receive a RLDP packet, it will check identifier besides checking
 *      source mac address and protocol.
 */
extern int32 rtl8309n_rldp_identifierEnable_set(uint32 enabled);


/* Function Name:
 *      rtl8309n_rldp_identifierEnable_get
 * Description:
 *      enable calculate and put identifier into RLDPP packet
 * Input:
 *      none
 * Output:
 *      pEnabled    -   pointer pointed to enabled status
 * Return:
 *      SUCCESS
 * Note:
 *      When identifier is enabled, ASIC will calculate identifier and put it into RLDP 
 *      packet. When ASIC receive a RLDP packet, it will check identifier besides checking
 *      source mac address and protocol.
 */
extern int32 rtl8309n_rldp_identifierEnable_get(uint32 *pEnabled);

/* Function Name:
 *      rtl8309n_rldp_protocol_set
 * Description:
 *      set protocol value for RLDP packet
 * Input:
 *      protocol    -   protocol value
 * Output:
 *      none
 * Return:
 *      SUCCESS
 */
extern int32 rtl8309n_rldp_protocol_set(uint32 protocol);

/* Function Name:
 *      rtl8309n_rldp_protocol_get
 * Description:
 *      get protocol value for RLDP packet
 * Input:
 *      protocol    -   protocol value
 * Output:
 *      none
 * Return:
 *      SUCCESS
 */
extern int32 rtl8309n_rldp_protocol_get(uint32 *pProtocol);

/* Function Name:
 *      rtl8309n_rldp_loopStatus_get
 * Description:
 *      get loop status port mask 
 * Input:
 *      none
 * Output:
 *      pStatus -   pointer pointed to loop status port mask
 * Return:
 *      SUCCESS
 * Note:
 */
extern int32 rtl8309n_rldp_loopStatus_get(uint32 *pStatus);

/* ISP MAC*/

/* Function Name:
 *		rtl8309n_isp_enable_set
 * Description:
 * 		Enable ISP MAC function
 * Input:
 *		enabled
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *
 */
extern int32 rtl8309n_isp_enable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_isp_enable_get
 * Description:
 * 		Get enabled status of ISP MAC function
 * Input:
 *		none
 * Output:
 *		pEnabled	-	pointer point to enabled status 
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *
 */
extern int32 rtl8309n_isp_enable_get(uint32 *pEnabled);

/* Function Name:
 * 		rtl8309n_isp_mac_set
 * Description:
 *		Set ISP MAC address
 * Input:
 *		pMac		-	pointer point to ISP MAC address
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
extern int32 rtl8309n_isp_mac_set(uint8 *pMac);

/* Function Name:
 * 		rtl8309n_isp_mac_get
 * Description:
 *		Get ISP MAC address
 * Input:
 *		none
 * Output:
 *		pMac		-	pointer point to ISP MAC address
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
extern int32 rtl8309n_isp_mac_get(uint8 *pMac);

/* Function Name:
 * 		rtl8309n_isp_wanPort_set
 * Description:
 *		Set WAN port number
 * Input:
 *		wanport -   wan port number
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *      The value of wanport should be 0-8, and 9 means no port is set as wan port. 
 */
extern int32 rtl8309n_isp_wanPort_set(uint32 wanport);

/* Function Name:
 * 		rtl8309n_isp_wanPort_get
 * Description:
 *		Get WAN port number
 * Input:
 *		none
 * Output:
 *		pWanport		-	pointer point to WAN port number
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 */
extern int32 rtl8309n_isp_wanPort_get(uint32 *pWanport);

/* LED */
/* Function Name:
 *		rtl8309n_led_rstBlinkEnable_set
 * Description:
 *		Enable reset blink, rldp  ability and led1 indicator 
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *		Before led function is used, reset blink ability, rldp ability and led1 indicator have to be enabled
 *		
 */
extern int32 rtl8309n_led_rstBlinkEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_led_rstBlinkEnable_get
 * Description:
 *		Get ASIC reset blink ability status
 * Input:
 *		none
 * Output:
 *		*pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 *		Before led function is used, reset blink ability, rldp ability and led1 indicator have to be enabled
 *		
 */
extern int32 rtl8309n_led_rstBlinkEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_led_rldpEnable_set
 * Description:
 *		Enable LED RLDP display function
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_led_rldpEnable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_led_rldpEnable_get
 * Description:
 *		Get LED RLDP display ability status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_led_rldpEnable_get(uint32 *pEnabled);

/* Function Name:
 *		rtl8309n_led_led1Enable_set
 * Description:
 *		Enable LED1 indicator 1
 * Input:
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_led_led1Enable_set(uint32 enabled);

/* Function Name:
 *		rtl8309n_led_led1Enable_get
 * Description:
 *		Get LED indicator 1 status
 * Input:
 *		none
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		SUCCESS
 * Note:
 *
 */
extern int32 rtl8309n_led_led1Enable_get(uint32 *pEnabled);

/* Function Name:
 * 		rtl8309n_led_modeBlinkRate_set
 * Description:
 *		Set LED mode and LED blink rate
 * Input:
 *		mode	-	LED mode
 *		rate	-	LED blink rate
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)RTL8309N have 2 group of LED indicators for per port: LED indicator 0 and LED indicator 1.
 *		   Each group have 6 LED modes set by the same global register. LED blink rate could be 32ms and 128 ms.
 *		   So mode could be any value as follows:
 *		        RTL8309N_LED_MODE_0
 *		        RTL8309N_LED_MODE_1
 *		        RTL8309N_LED_MODE_2
 *		        RTL8309N_LED_MODE_3
 *		        RTL8309N_LED_MODE_4
 *		        RTL8309N_LED_MODE_5
 *		 (2)rate could be any value as follows:
 *		        RTL8309N_LED_BLINKRATE_32MS		    -	32ms blink period
 *		        RTL8309N_LED_BLINKRATE_128MS		-	128ms blink period
 */
extern int32 rtl8309n_led_modeBlinkRate_set(uint32 mode, uint32 rate);

/* Function Name:
 * 		rtl8309n_led_modeBlinkRate_get
 * Description:
 *		Get LED mode and LED blink rate
 * Input:
 *		none
 * Output:
 *		pMode	-	pointer point to mode
 *		pRate	-	pointer point to blink rate
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)RTL8309N have 2 group of LED indicators for per port: LED indicator 0 and LED indicator 1.
 *		   Each group have 6 LED modes set by the same global register. LED blink rate could be 32ms and 128 ms.
 *		   So mode could be any value as follows:
 *		        RTL8309N_LED_MODE_0
 *		        RTL8309N_LED_MODE_1
 *		        RTL8309N_LED_MODE_2
 *		        RTL8309N_LED_MODE_3
 *		        RTL8309N_LED_MODE_4
 *		        RTL8309N_LED_MODE_5
 *		 (2)rate could be any value as follows:
 *		        RTL8309N_LED_BLINKRATE_32MS		    -	32ms blink period
 *		        RTL8309N_LED_BLINKRATE_128MS		-	128ms blink period
 */
extern int32 rtl8309n_led_modeBlinkRate_get(uint32 *pMode, uint32 *pRate);

extern int32 rtl8309n_led_rlppRldp_blinkRate_set(uint32 rate);

extern int32 rtl8309n_led_rlppRldp_blinkRate_get(uint32 *pRate);

extern int32 rtl8309n_led_rlppLedMsk_set(uint32 ledmsk);

extern int32 rtl8309n_led_rlppLedMsk_get(uint32 *pLedmsk);

extern int32 rtl8309n_led_buzzerForm_set(uint32 waveform);

extern int32 rtl8309n_led_buzzerForm_get(uint32 *pWaveform);

/* Function Name:
 * 		rtl8309n_led_rtct_set
 * Description:
 *		Set RTCT LED display mode, test time, result time
 * Input:
 *		ledMode		-	led mode, parallel, scan mode
 *		test_time	-	LED time for test
 *		result time	-	LED time for result display
 * Output:
 *		none
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *		(1) RTCT LED display mode has 2 type: parallel mode and scan mode. The LED dispaly time for RTCT test
 *		process can be set seperately for each type. The dispaly time for displaying RTCT result will be the same
 *		for these 2 types.
 *		(2) ledMode could be any value as follows:
 *		    RTL8309N_LED_RTCTTEST_PARALLEL	-	parallel test mode
 *		    RTL8309N_LED_RTCTTEST_SCAN		-	scan test mode
 *		(3) test_time for parallel test mode could be :	
 *		    RTL8309N_LED_TIME_2SEC 	-	2 seconds
 *		    RTL8309N_LED_TIME_3SEC		-	3 seconds
 *		    RTL8309N_LED_TIME_4SEC 	-	4 seconds
 *		    RTL8309N_LED_TIME_5SEC 	-	5 seconds
 *		(4) test_time for scan test mode could be :	
 *		    RTL8309N_LED_TIME_80MS	-	80 ms
 *		    RTL8309N_LED_TIME_128MS	-	128 ms
 *		    RTL8309N_LED_TIME_256MS	-	256 ms
 *		    RTL8309N_LED_TIME_512MS	-	512 ms
 *		(5) result_time for result diplay:
 *		    RTL8309N_LED_TIME_3SEC	-	3 seconds	
 *		    RTL8309N_LED_TIME_5SEC	-	5 seconds
 */
extern int32 rtl8309n_led_rtct_set(uint32 led_mode, uint32 test_time, uint32 result_time);

/* Function Name:
 * 		rtl8309n_led_rtct_get
 * Description:
 *		Get RTCT LED display mode, test time, result time
 * Input:
 *		none
 * Output:
 *		pLedMode		-	led mode, parallel, scan mode
 *		pTest_time	-	LED time for test
 *		pResult_time	-	LED time for result display
 * Return:
 *		SUCCESS
 *		FAILED
 * Note:
 *		(1) RTCT LED display mode has 2 type: parallel mode and scan mode. The LED dispaly time for RTCT test
 *		process can be set seperately for each type. The dispaly time for displaying RTCT result will be the same
 *		for these 2 types.
 *		(2) ledMode could be any value as follows:
 *		    RTL8309N_LED_RTCTTEST_PARALLEL	-	parallel test mode
 *		    RTL8309N_LED_RTCTTEST_SCAN		-	scan test mode
 *		(3) test_time for parallel test mode could be :	
 *		    RTL8309N_LED_TIME_2SEC 	-	2 seconds
 *		    RTL8309N_LED_TIME_3SEC		-	3 seconds
 *		    RTL8309N_LED_TIME_4SEC 	-	4 seconds
 *		    RTL8309N_LED_TIME_5SEC 	-	5 seconds
 *		(4) test_time for scan test mode could be :	
 *		    RTL8309N_LED_TIME_80MS	-	80 ms
 *		    RTL8309N_LED_TIME_128MS	-	128 ms
 *		    RTL8309N_LED_TIME_256MS	-	256 ms
 *		    RTL8309N_LED_TIME_512MS	-	512 ms
 *		(5) result_time for result diplay:
 *		    RTL8309N_LED_TIME_3SEC	-	3 seconds	
 *		    RTL8309N_LED_TIME_5SEC	-	5 seconds
 */
extern int32 rtl8309n_led_rtct_get(uint32 *pLedMode, uint32 *pTest_time, uint32 *pResult_time);

/* Function Name:
 *		rtl8309n_led_cpu_set
 * Description:
 *		Set CPU control LED ability
 * Input:
 *		ledGroup	-	select LED indicator, LED indicator 0 or 1
 *		cpuCtrlMsk	-	select CPU control portmask, ASIC control or CPU control
 *		ledStatus	-	per port LED status, on or off
 * Output:
 *		none
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)RTL8309N has 2 groups of indicator per port, LED indicator 0 and 1. Each of them can be 
 *		   choosed to controlled by ASIC or CPU. When per port's LED indicator are controlled by CPU,
 *		   the LED can be turned on or off by CPU. RTL8309N only 8 PHY from 0 - 7, so portmask or ledStatus can 
 *		   not exceed 0XFF.
 *		(2)ledGroup could be any value as follows:
 *			RTL8309N_LED_GROUP_0		-	LED indicator 0
 *			RTL8309N_LED_GROUP_1		-	LED indicator 1
 */
extern int32 rtl8309n_led_cpu_set(uint32 ledGroup, uint32 cpuCtrlMsk, uint32 ledStatus);

/* Function Name:
 *		rtl8309n_led_cpu_get
 * Description:
 *		Get CPU control LED ability
 * Input:
 *		ledGroup	-	select LED indicator, LED indicator 0 or 1
 * Output:
 *		pCpuCtrlMsk	-	select CPU control portmask, ASIC control or CPU control
 *		pLedStatus	-	per port LED status, on or off
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)RTL8309N has 2 groups of indicator per port, LED indicator 0 and 1. Each of them can be 
 *		   choosed to controlled by ASIC or CPU. When per port's LED indicator are controlled by CPU,
 *		   the LED can be turned on or off by CPU. RTL8309N only 8 PHY from 0 - 7, so portmask or ledStatus can 
 *		   not exceed 0XFF.
 *		(2)ledGroup could be any value as follows:
 *			RTL8309N_LED_GROUP_0		-	LED indicator 0
 *			RTL8309N_LED_GROUP_1		-	LED indicator 1
 */
extern int32 rtl8309n_led_cpu_get(uint32 ledGroup, uint32 *pCpuCtrlMsk, uint32 *pLedStatus);

/* Function Name:
 *		rtl8309n_wkp_enable_set
 * Description:
 *		Set cpu wake up ability
 * Input:
 *		enable	    -	enable or disable wake up ability
 *      pinLevel   -   SLP_WKP pin status, high or low
 * Output:
 *		None
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)enable could be any value as follows:
 *			RTL8309N_ENABLED		-	Enable CPU sleep. The pin SLP_WKP is in SLEEP status
 *			RTL8309N_DISABLED		-	Disable CPU sleep. The pin SLP_WKP is in WAKEUP status.
 *      (2)pinLevel could be any value as follows:
 *          0   -   High level indicates SLEEP status. Low level indicates WAKEUP status 
 *          1   -   Low level indicates SLEEP status. High level indicates WAKEUP status.
 */
int32 rtl8309n_wkp_enable_set(uint32 enabled, uint32 pinLevel);



/* Function Name:
 *		rtl8309n_wkp_enable_get
 * Description:
 *		Get cpu wake up ability
 * Input:
 *      none
 * Output:
 *		pEnable	    -	enable or disable wake up ability
 *      pPinLevel   -   SLP_WKP pin status, high or low
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)enable could be any value as follows:
 *			RTL8309N_ENABLED		-	Enable CPU sleep. The pin SLP_WKP is in SLEEP status
 *			RTL8309N_DISABLED		-	Disable CPU sleep. The pin SLP_WKP is in WAKEUP status.
 *      (2)pinStatus could be any value as follows:
 *          0   -   High level indicates SLEEP status. Low level indicates WAKEUP status 
 *          1   -   Low level indicates SLEEP status. High level indicates WAKEUP status.
 */
int32 rtl8309n_wkp_enable_get(uint32 *pEnable, uint32 *pPinLevel);


/* Function Name:
 *		rtl8309n_wkp_linkchangeEnable_set
 * Description:
 *		Enable link change to wakeup cpu
 * Input:
 *		enable	   -    enable or disable link change to wake up cpu
 *      pmsk       -    the link change status of port in "pmsk" will wake up cpu
 * Output:
 *		None
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)enable could be any value as follows:
 *			RTL8309N_ENABLED		-	Enable CPU sleep. The pin SLP_WKP is in SLEEP status
 *			RTL8309N_DISABLED		-	Disable CPU sleep. The pin SLP_WKP is in WAKEUP status.
 */
int32 rtl8309n_wkp_linkchngEnable_set(uint32 enabled, uint32 pmsk);


/* Function Name:
 *		rtl8309n_wkp_linkchangeEnable_set
 * Description:
 *		Enable link change to wakeup cpu
 * Input:
 *      none
 * Output:
 *		pEnable	   -    enable or disable link change to wake up cpu
 *      pPmsk       -    the link change status of port in "pmsk" will wake up cpu
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *		(1)enable could be any value as follows:
 *			RTL8309N_ENABLED		-	Enable CPU sleep. The pin SLP_WKP is in SLEEP status
 *			RTL8309N_DISABLED		-	Disable CPU sleep. The pin SLP_WKP is in WAKEUP status.
 */
int32 rtl8309n_wkp_linkchngEnable_get(uint32 *pEnable, uint32 *pPmsk);


/* Function Name:
 *		rtl8309n_wkp_linkchangeStatus_get
 * Description:
 *		Get link change status
 * Input:
 *      none
 * Output:
 *      pPmsk   -   the link change status of port
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This api is called to get the link change status of port. The link status bit
 *      is read clear.
 */
int32 rtl8309n_wkp_linkchangeStatus_get(uint32 *pPmsk);



/* Function Name:
 *		rtl8309n_wkp_frameEnable_set
 * Description:
 *		Enable wake up frame to wakeup cpu
 * Input:
 *      enable    -    enable frame to wake up cpu
 *		igrpmsk	  -    packet's ingress port mask
 *      dpm       -    packet's destination port mask
 *      enbrdmul  -    0 : disable the broadcast and multicast packet to wakeup CPU
 *                     1 : enable the broadcast and multicast packet to wakeup CPU
 *      enuni     -    0 : all unicast can wakeup the CPU.;
 *                     1 : not all unicast but only the packet with DA = Switch MAC can wakeup the CPU
 * Output:
 *		None
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This api is called to enable frame to wake up cpu.
 */
int32 rtl8309n_wkp_frameEnable_set(uint32 enable, uint32 igrpmsk, uint32 dpm, uint32 enbrdmul, uint32 enuni);


/* Function Name:
 *		rtl8309n_wkp_frameEnable_get
 * Description:
 *		Enable wake up frame to wakeup cpu
 * Input:
 *      none
 * Output:
 *      pEnable    -    enable frame to wake up cpu
 *		pIgrpmsk	  -    packet's ingress port mask
 *      pDpm       -    packet's destination port mask
 *      pEnbrdmul  -    0 : disable the broadcast and multicast packet to wakeup CPU
 *                     1 : enable the broadcast and multicast packet to wakeup CPU
 *      pEnuni     -    0 : all unicast can wakeup the CPU.;
 *                     1 : not all unicast but only the packet with DA = Switch MAC can wakeup the CPU
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This api is called to enable frame to wake up cpu.
 */
int32 rtl8309n_wkp_frameEnable_get(uint32 *pEnable, uint32 *pIgrpmsk, uint32 *pDpm, uint32 *pEnbrdmul, uint32 *pEnuni);


/* Function Name:
 *		rtl8309n_wkp_frameStatus_get
 * Description:
 *		Get frame status which indicate the valid wakeup frame occurs
 * Input:
 *      none
 * Output:
 *      pStatus   -   the status indicate the valid wakeup frame occurs
 * Return:
 *		FAILED
 *		SUCCESS
 * Note:
 *      This api is called to get the status indicate the valid wakeup frame occurs
 */
int32 rtl8309n_wkp_frameStatus_get(uint32 *pStatus);



#endif

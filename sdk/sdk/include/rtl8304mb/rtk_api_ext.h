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
* Purpose : realtek common API
*
* Feature :  This file consists of following modules:
*     		1)	Packet length
*			2)	Phy
*			3)    Port isolation
*			4)	Rate
*			5)	Qos
*			6)	IGMP/MLD
*			7)	IPV4/IPV6 unknown multicast 
*			8)	Vlan
*			9)	Stp
*			10)	Lookup table
*			11)	CPU
*			12)	Mirror
*			13)	Dot1x
*			14)	ACL
*			15)  Storm filter
*/
/* Function Name:
 *      rtk_switch_init
 * Description:
 *       Set chip to default configuration enviroment
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED               -   Failure
 * Note:
 *      The API can set chip registers to default configuration for
 *      different release chip model.
 */
extern rtk_api_ret_t rtk_switch_init(void);

/* Function Name:
 *      rtk_switch_maxPktLen_set
 * Description:
 *      Set the max packet length
 * Input:
 *	  type	-	  max packet length type
 *      len       -       max packet length
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                    -  Success
 *      RT_ERR_FAILED               -  Failure
 *      RT_ERR_INPUT                -  Invalid input parameter
 * Note:
 *      The API can set max packet length. The len would be values as follows:
 *          	- MAX_PKTLEN_1522B
 *		- MAX_PKTLEN_1526B
 *          	- MAX_PKTLEN_2048B
 *          	- MAX_PKTLEN_16000B
 *          	- MAX_PKTLEN_USER
 */
extern rtk_api_ret_t rtk_switch_maxPktLen_set(rtk_switch_maxPktLen_t type, rtk_switch_len_t len);

/* Function Name:
 *      rtk_switch_maxPktLen_get
 * Description:
 *      Get the max packet length
 * Input:
 *      none
 * Output:
 *	  pType	  -	 the pointer of max packet length type	
 *      pLen       -    the pointer of max packet length
 * Return: 
 *      RT_ERR_OK                    -  Success
 *      RT_ERR_FAILED               -  Failure
 *      RT_ERR_NULL_POINTER     -  Input parameter is null pointer 
 * Note:
 *      The API can get max packet length.The len would be values as follows:
 *          	- MAX_PKTLEN_1522B
 *		- MAX_PKTLEN_1526B
 *          	- MAX_PKTLEN_2048B
 *          	- MAX_PKTLEN_16000B
 *          	- MAX_PKTLEN_USER
 */
extern rtk_api_ret_t rtk_switch_maxPktLen_get(rtk_switch_maxPktLen_t *pType, rtk_switch_len_t *pLen);

/* Function Name:
 *      rtk_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      phy               - phy ID(0 ~ 7) 
 *      reg                - Register id
 *      regData         - Register data
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK         -  Success
 *      RT_ERR_FAILED   -   Failure
 *	  RT_ERR_PORT_ID	-	invalid port id	
 * Note:
 *      This API can be called to write a phy register provided by IEEE standard.
 *      RTL8309N switch has 8 PHYs(PHY 0-7). 
 */
extern rtk_api_ret_t rtk_port_phyReg_set(rtk_port_t phy, rtk_port_phy_reg_t reg, rtk_port_phy_data_t regData);

/* Function Name:
 *      rtk_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      phy                  - phy number, 0 ~ 7
 *      reg                  - Register id
 * Output:
 *      pData               - the pointer of Register data
 * Return:
 *     RT_ERR_OK        	-  Success
 *     RT_ERR_FAILED   	-  Failure
 *	 RT_ERR_PORT_ID		-	invalid port id
 *	 RT_ERR_NULL_POINTER		-	input parameter is null pointer 
 * Note:
 *      This API can be called to read a PHY register data provided by IEEE standard.
 *      RTL8309N switch has 8 PHYs(PHY 0-7).  
 */
extern rtk_api_ret_t rtk_port_phyReg_get(rtk_port_t phy, rtk_port_phy_reg_t reg, rtk_port_phy_data_t *pData) ;

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set ethernet PHY auto-negotiation desired ability
 * Input:
 *      port       -  phy id,0~7
 *      pAbility   -  pointer point to Ability structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       			-  	Success
 *      RT_ERR_FAILED   		-  	Failure
 *	  RT_ERR_PORT_ID			-  	invalid port id
 *	  RT_ERR_NULL_POINTER	-  	input parameter is null pointer 
 *	  RT_ERR_INPUT			-  	invalid input parameter 
 * Note:
 *      (1) RTL8309N switch only has 8 phy, so the input phy id should be 0 ~ 7.
 *      (2) In auto-negotiation mode, phy autoNegotiation ability must be enabled
 */ 
extern rtk_api_ret_t rtk_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get ethernet PHY auto-negotiation ability configurations
 * Input:
 *      port       -  phy id,0~7
 * Output:
 *      pAbility   -  pointer point to Ability structure
 * Return:
 *      RT_ERR_OK        		-  Success
 *      RT_ERR_FAILED   	-  Failure
 *	  RT_ERR_PORT_ID		-  invalid port id
 *	  RT_ERR_NULL_POINTER			-	input parameter is null pointer
 *	  RT_ERR_PHY_AUTO_NEGO_MODE	-	invalid PHY auto-negotiation mode
 * Note:
 *      (1) RTL8309N switch only has 8 phy, so the input phy id should be 0~7.
 *      (2) In auto-negotiation mode, phy autoNegotiation ability must be enabled.
 */   
extern rtk_api_ret_t rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *     rtk_port_phyForceModeAbility_set
 * Description:
 *      Set ethernet PHY force mode desired ability
 * Input:
 *      port       -  Port id
 *      pAbility   -  pointer point to Ability structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK        				-  Success
 *      RT_ERR_FAILED  	 			-  Failure
 *	  RT_ERR_PORT_ID				-  invalid port id
 *	  RT_ERR_NULL_POINTER		-  input parameter is null pointer 
 *	  RT_ERR_INPUT				-  invalid input parameter 
 * Note:
 *      (1) RTL8309N switch only has 8 phy, so the input phy id should be 0~7.
 *      (2) In force mode, phy autoNegotiation ability must be disabled.
 */
extern rtk_api_ret_t rtk_port_phyForceModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyForceModeAbility_get
 * Description:
 *      Get ethernet PHY force mode ability configuration
 * Input:
 *      port       -  Port id
 * Output:
 *      pAbility   -  pointer point to Ability structure
 * Return:
 *      RT_ERR_OK        					-   Success
 *	  RT_ERR_PORT_ID					-   invalid port id
 *	  RT_ERR_NULL_POINTER			-   input parameter is null pointer  
 *	  RT_ERR_PHY_AUTO_NEGO_MODE	-   invalid PHY auto-negotiation mode
 * Note:
 *      (1) RTL8309N switch only has 8 phy, so the input phy id should be 0~7.
 *      (2) In force mode, phy autoNegotiation ability must be disabled.
 */
extern rtk_api_ret_t rtk_port_phyForceModeAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

 /* Function Name:
  * 	 rtk_port_phyLinkStatus_get
  * Description:
  * 	 Get ethernet PHY link status
  * Input:
  * 	 phy		-  phy id,0-7
  * Output:
  * 	 pLinkStatus	-  pointer point to phy link status 
  * Return:
  * 	 RT_ERR_OK		  		-  	Success
  * 	 RT_ERR_PORT_ID  		-	 invalid port id
  * 	 RT_ERR_NULL_POINTER	 -	 input parameter is null pointer  
  * Note:
  * 	 (1) The link status of phy would be values as follows:
  *		-	PORT_LINKUP
  *		-	PORT_LINKDOWN
  *	 (2) The speed could be values as  follows:
  *		-	PORT_SPEED_10M
  *		-	PORT_SPEED_100M
  *	 (3) The duplex could be values as follows:
  *		-	PORT_HALF_DUPLEX
  *		-	PORT_FULL_DUPLEX
  */
extern rtk_api_ret_t rtk_port_phyStatus_get(rtk_port_t phy, rtk_port_linkStatus_t *pLinkStatus, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex);


/* Function Name:
 *      rtk_port_isolation_set
 * Description:
 *      Set permitted port isolation portmask
 * Input:
 *      port                - 		port id
 *      portmask         - 		Permit port mask
 * Output:
 *      none
 * Return:
 *      RT_ERR_FAILED         -   Failure
 *      RT_ERR_OK               -   Success
 *      RT_ERR_PORT_ID       -   Invalid port number
 *      RT_ERR_PORT_MASK 	-   Invalid portmask
 * Note:
 *      This API can be called to set port isolation mask for port 0~8.
 */
extern rtk_api_ret_t rtk_port_isolation_set(rtk_port_t port, rtk_portmask_t portmask);

/* Function Name:
 *      rtk_port_isolation_get
 * Description:
 *      Get permitted port isolation portmask
 * Input:
 *      port                - port id
 * Output:
 *      pPortmask       - the pointer of permit port mask
 * Return:
 *      RT_ERR_OK                  -   Success
 *      RT_ERR_FAILED             -   Failure
 *      RT_ERR_PORT_ID          -   Invalid port number
 *      RT_ERR_NULL_POINTER  -   Input parameter is a null pointer
 * Note:
 *      This API can be called to get port isolation mask.
 */
extern rtk_api_ret_t rtk_port_isolation_get(rtk_port_t port, rtk_portmask_t *pPortmask);

/* Function Name:
 *	  rtk_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set port ingress bandwidth control.
 * Input:
 *      port            		-  Port id
 *      rate            		-  Rate of share meter
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK              		-  Success
 *      RT_ERR_FAILED         		-  Failure
 *      RT_ERR_PORT_ID         	-  Invalid port id
 *      RT_ERR_ENABLE          	-  invalid enable parameter
 *      RT_ERR_INBW_RATE       	-  invalid input bandwidth 
 * Note:
 *	  For RTL8309N, port0 and port 8's max speed could be 100Mbps, 
 *	  and max speed could only be 100Mbps for port1 to port 7.
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps. 
 *	  The granularity of rate is 64Kbps. 
 *	  interframe gap and preamble. 
 */
extern rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate, rtk_enable_t ifg_include);

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control
 * Input:
 *      port               -  Port id, 0~8
 * Output:
 *      pRate             -  the pointer of rate of share meter
 * Return:
 *      RT_ERR_OK                      	-  Success
 *      RT_ERR_FAILED                  	-  Failure
 *      RT_ERR_PORT_ID                 	-  Invalid port id
 *      RT_ERR_NULL_POINTER      	-  input parameter is null pointer
 * Note:
 *	  For RTL8309N, port0 and port 8's max speed could be 100Mbps, 
 *	  and max speed could only be 100Mbps for port1 to port 7.
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps. 
 *	  The granularity of rate is 64Kbps. 
 *	  interframe gap and preamble. 
 */
extern rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);


/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set port egress bandwidth control
 * Input:
 *      port            -  Port id
 *      rate            -  Rate of bandwidth control
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                   -  Success
 *      RT_ERR_FAILED             -  Failure
 *      RT_ERR_PORT_ID           -  invalid port id  
 *      RT_ERR_ENABLE             -  invalid enable parameter  
 *      RT_ERR_QOS_EBW_RATE       -  invalid egress bandwidth rate
 * Note:
 *	  For RTL8309N, port0 and port 8's max speed could be 100Mbps, 
 *	  and max speed could only be 100Mbps for port from port 1 to port 7.
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps.
 *	  for rate calculation with or without interframe gap and preamble. 
 */
extern rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate, rtk_enable_t ifg_include);

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_get
 * Description:
 *      Get port egress bandwidth control
 * Input:
 *      port             -  Port id
 * Output:
 *      pRate           -  the pointer of rate of bandwidth control
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_PORT_ID             -  Invalid port number
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_NULL_POINTER      -  null pointer
 * Note:
 *	  For RTL8309N, port0 and port 8's max speed could be 100Mbps, 
 *	  and max speed could only be 100Mbps for port from port 1 to port 7.
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps.
 *	  for rate calculation with or without interframe gap and preamble. 
 */
extern rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);

/* Function Name:
 *      rtk_qos_init
 * Description:
 *      Configure Qos with default settings
 * Input:
 *      queueNum     -  Queue number of each port(from 1 to 4)
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                -  Success
 *      RT_ERR_FAILED            -  Failure
 *      RT_ERR_QUEUE_NUM         -  invalid queue number 
 * Note:
 *    This API will initialize related Qos function. First it will set the ASIC's queue number 
 *	  globally for all port. Then it will set priority to queue mapping table based on the queue number
 *	  for all ports. And it will enable output and input flow control abilities.
 */
extern rtk_api_ret_t rtk_qos_init(rtk_queue_num_t queueNum);


/* Function Name:
 *      rtk_qos_priSrcEnable_set
 * Description:
 *      Enable/disable Qos priority source for ingress port
 * Input:
 *      port     -  port id
 *	  priSrc	 -  priority source id
 *	  enable   -  DISABLED or ENABLED
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID				-  invalid port id
 *	  RT_ERR_INPUT			   -  Invalid input parameter
 * Note:
 *    This API will enable Qos priority source for ingress port.
 *    The port id is from 0 to 8. priSrc are Port, 1Q, DSCP, IP adress, and CPU tag basded priority. 
 */
extern rtk_api_ret_t rtk_qos_priSrcEnable_set(rtk_port_t port, rtk_qosPriSrc_t priSrc, rtk_enable_t enabled);

/* Function Name:
 *      rtk_qos_priSrcEnable_set
 * Description:
 *      Enable/disable Qos priority source for ingress port
 * Input:
 *      port     -  port id
 *		priSrc	 -  priority source id
 * Output:
 *      pEnable  -  Point to the status of qos priority source
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID				-  error port id
 *		RT_ERR_INPUT			   -  Invalid input parameter
 * Note:
 *    This API will get the status of Qos priority source for ingress port.
 *    The port id is from 0 to 8. priSrc are Port, 1Q, DSCP, IP adress, and CPU tag basded priority. 
 */
extern rtk_api_ret_t rtk_qos_priSrcEnable_get(rtk_port_t port, rtk_qosPriSrc_t priSrc, rtk_enable_t *pEnabled);

/* Function Name:
 *      rtk_qos_priSel_set
 * Description:
 *      Configure the priority order among different priority mechanisms.
 * Input:
 *      pPriDec		- pointer point to priority level structure. 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - 	success
 *      RT_ERR_FAILED           - 	failure 
 *      RT_ERR_NULL_POINTER  	-   Input parameter is null pointer
 *      RT_ERR_INPUT 			- 	Invalid input parameter.
 * Note: 
 *      (1)For 8309N, there are 4 types of priority source that could be set arbitration level, which are 
 *      	ACL-based, DSCP-based, 1Q-based, Port-based priority. Each one could be set to level from 0 to 4. 
 *      (2)ASIC will follow user's arbitration level setting to select internal priority for receiving frame. 
 *      	If two priority mechanisms are the same level, the ASIC will choose the higher priority to assign for the receiving frame.
 */
extern rtk_api_ret_t rtk_qos_priSel_set(rtk_priority_select_t *pPriDec);

/* Function Name:
 *      rtk_qos_priSel_get
 * Description:
 *      Get the priority order configuration among different priority mechanism.
 * Input:
 *      None
 * Output:
 *      pPriDec		- pointer point to priority level structure. 
 * Return:
 *      RT_ERR_OK              		- success
 *      RT_ERR_FAILED          	- failure 
 *      RT_ERR_NULL_POINTER  	- Input parameter is null pointer
 * Note: 
 *      (1)For 8309N, there are 4 types of priority mechanisms that could be set arbitration level, which are 
 *      	ACL-based, DSCP-based, 1Q-based, Port-based priority. Each one could be set to level from 1 to 4. 
 *      (2)ASIC will follow user's arbitration level setting to select internal priority for receiving frame. 
 *      	If two priority mechanisms are the same level, the ASIC will choose the higher priority to assign for the receiving frame.
 */
extern rtk_api_ret_t rtk_qos_priSel_get(rtk_priority_select_t *pPriDec);

/* Function Name:
 *      rtk_qos_1pPriRemap_set
 * Description:
 *      Configure 1Q priority mapping to internal absolute priority
 * Input:
 *      dot1p_pri     -  802.1p priority value, 0~7
 *      int_pri       	-  internal priority value, 0~3
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_VLAN_PRIORITY        -  Invalid 1p priority
 *      RT_ERR_QOS_INT_PRIORITY   -  invalid internal priority  
 * Note:
 *	  When DOT1Q tagged packet has been received, 1Q tag priority has 3 bits, and RTL8309N only support 2 bit priority internally.
 *	  So 3 bit 1Q tag priority has to be mapped to a 2 bit internal priority for further QOS operations.	
 */
extern rtk_api_ret_t rtk_qos_1pPriRemap_set(rtk_pri_t dot1p_pri, rtk_pri_t int_pri);

/* Function Name:
 *      rtk_qos_1pPriRemap_get
 * Description:
 *      Get 1Q priorities mapping to internal absolute priority
 * Input:
 *      dot1p_pri    -  802.1p priority value
 * Output:
 *      pInt_pri      -  the pointer of internal priority value
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_VLAN_PRIORITY        -  Invalid 1p priority
 *      RT_ERR_NULL_POINTER         -  Input parameter is null pointer
 * Note:
 *      Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage 
 *      and packet scheduling.
 */
extern rtk_api_ret_t rtk_qos_1pPriRemap_get(rtk_pri_t dot1p_pri, rtk_pri_t *pInt_pri);

/* Function Name:
 *      rtk_qos_dscpPriRemap_set
 * Description:
 *      Set DSCP-based priority
 * Input:
 *      dscp_value      -  dscp value(0~63)
 *      int_pri    		-  internal priority value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                     -  Failure
 *      RT_ERR_QOS_INT_PRIORITY   -   invalid internal priority
 *      RT_ERR_QOS_DSCP_VALUE     -   invalid DSCP value  
 * Note:  
 *	  This API can be called to configure a dscp value to a 2-bit internal priority value.
 *	  RTL8309N support 64 DSCP values and 2-bit internal priority.
 */ 
extern rtk_api_ret_t rtk_qos_dscpPriRemap_set(rtk_dscp_t dscp_value, rtk_pri_t int_pri);

/* Function Name:
 *      rtk_qos_dscpPriRemap_get
 * Description:
 *      Get DSCP-based priority
 * Input:
 *      dscp_value      	-  dscp code
 * Output:
 *      pInt_pri  		-  the pointer of internal priority value
 * Return:
 *      RT_ERR_OK                           -  Success
 *      RT_ERR_FAILED                     -  Failure
 *      RT_ERR_QOS_DSCP_VALUE      -  Invalid DSCP value
 *      RT_ERR_NULL_POINTER           -  Input parameter is null pointer
 * Note:  
 *	  This API can be called to get a 2-bit internal priority value for a specified dscp value.
 *	  RTL8309N support 64 DSCP values and 2-bit internal priority.
 */ 
extern rtk_api_ret_t rtk_qos_dscpPriRemap_get(rtk_dscp_t dscp_value, rtk_pri_t *pInt_pri);

/* Function Name:
 *      rtk_qos_portPri_set
 * Description:
 *      Configure priority usage to each port
 * Input:
 *      port                - 		Port id.                
 *      int_pri             -  	internal priority value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *      RT_ERR_PORT_ID                -   invalid port id
 *      RT_ERR_QOS_INT_PRIORITY  -   invalid internal priority
 * Note:
 *     The API can set port-based priority.
 */
extern rtk_api_ret_t rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t int_pri);

/* Function Name:
 *      rtk_qos_portPri_get
 * Description:
 *      Get priority usage to each port
 * Input:
 *      port                  - Port id.                
 * Output:
 *      pInt_pri             -  the pointer of internal priority value
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *      RT_ERR_PORT_ID                -   invalid port id
 *      RT_ERR_NULL_POINTER        -   input parameter is null pointer
 * Note:
 *     This API can be called to get port-based priority
 */
extern rtk_api_ret_t rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pInt_pri);


/* Function Name:
 *      rtk_qos_priMap_set
 * Description:
 *      Set internal priority mapping to queue ID for different queue number
 * Input:
 *	port				-	port id
 *    queue_num       	- 	Queue number usage
 *    pPri2qid            - 	pointer point to Priority and queue ID mapping table               
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *		RT_ERR_PORT_ID				-	invalid port id
 *      RT_ERR_QUEUE_NUM              -  invalid queue number
 *      RT_ERR_NULL_POINTER        -   input parameter is null pointer
 * Note:
 *      ASIC supports priority mapping to queue with different queue number from 1 to 4.
 *      For different queue numbers usage, ASIC supports different internal available queue IDs.
 *	  pPri2qid has 4 members, which is from queue id for priority 0 to queue id for priority 3.
 */
extern rtk_api_ret_t rtk_qos_priMap_set(rtk_port_t port, rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid);

/* Function Name:
 *      rtk_qos_priMap_get
 * Description:
 *      Get priority to queue ID mapping table parameters
 * Input:
 *	  port			-  port id
 *      queue_num       	- queue number usage          
 * Output:
 *      pPri2qid            - pointer point to Priority and queue ID mapping table
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *      RT_ERR_PORT_ID              -   invalid port id
 *	  RT_ERR_QUEUE_NUM			-	invalid queue number
 *      RT_ERR_NULL_POINTER        -  input parameter is null pointer
 * Note:
 *      ASIC supports priority mapping to queue with different queue number from 1 to 4.
 *      For different queue numbers usage, ASIC supports different internal available queue IDs.
 *	  pPri2qid has 4 members, which is from queue id for priority 0 to queue id for priority 3.
 */
extern rtk_api_ret_t rtk_qos_priMap_get(rtk_port_t port, rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid);

/* Function Name:
 *      rtk_qos_1pRemarkEnable_set
 * Description:
 *      Enable 802.1P remarking ability
 * Input:
 *      port       -  port number
 *      enabled    -  DISABLED or ENABLED
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK              -  Success
 *      RT_ERR_FAILED        -   Failure
 *      RT_ERR_PORT_ID      -   Invalid port id
 * Note:
 *      This API can be called to enable or disable 802.1P remarking ability for a port of RTL8309N.
 *	  The 802.1P remarking function here is used to assign a new 3-bit priority for a tx packet instead of
 *	  its old 2-bit priority. The assignment is based on the user's definition.
 */
extern rtk_api_ret_t rtk_qos_1pRemarkEnable_set(rtk_port_t port, rtk_enable_t enabled);

/* Function Name:
 *      rtk_qos_1pRemarkEnable_get
 * Description:
 *      Get enabled status of 802.1P remarking ability
 * Input:
 *      port        -  port number
 * Output:
 *      pEnabled  	-  pointer point to the ability status
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED               -   Failure
 *      RT_ERR_PORT_ID             -   Invalid port id
 *      RT_ERR_NULL_POINTER     -   Input parameter is null pointer
 * Note:
 *      This API can be called to get the enabled status of  802.1P remarking ability for a port of RTL8309N.
 *	  The 802.1P remarking function here is used to assign a new 3-bit priority for a tx packet instead of
 *	  its old 2-bit priority. The assignment is based on the user's definition.
 */
extern rtk_api_ret_t rtk_qos_1pRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_qos_1pRemark_set
 * Description:
 *      Set 802.1P remarking priority
 * Input:
 *      int_pri        -  Packet internal priority(0~4)
 *      dot1p_pri      -  802.1P priority(0~7)
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                           - Success
 *      RT_ERR_FAILED                      - Failure
 *      RT_ERR_VLAN_PRIORITY          - Invalid 1p priority
 *      RT_ERR_QOS_INT_PRIORITY     - Invalid internal priority 
 * Note:
 *      RTL8309N support 2-bit internal priority and 3-bit dot1p priotiy. User can use this API to map 
 *	  a 2-bit internal priority to a 3-bit dot1p priority.
 */
extern rtk_api_ret_t rtk_qos_1pRemark_set(rtk_pri_t int_pri, rtk_pri_t dot1p_pri);

/* Function Name:
 *      rtk_qos_1pRemark_get
 * Description:
 *      Get 802.1P remarking priority
 * Input:
 *      int_pri        	-  Packet priority(0~4)
 * Output:
 *      pDot1p_pri  	-  the pointer of 802.1P priority(0~7)
 * Return:
 *      RT_ERR_OK                           -  Success
 *      RT_ERR_FAILED                      -  Failure
 *      RT_ERR_NULL_POINTER            -  Input parameter is null pointer
 *      RT_ERR_QOS_INT_PRIORITY     -  Invalid internal priority 
 * Note:
 *      This API can be called to get a 2-bit internal priority and a 3-bit dot1p priority mapping. 
 */
extern rtk_api_ret_t rtk_qos_1pRemark_get(rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri);


/* Function Name:
 *      rtk_trap_unknownMcastPktAction_set
 * Description:
 *      Set behavior of unknown multicast
 * Input:
 *      port                -   port id
 *      type               -   unknown multicast packet type
 *      mcast_action    -  unknown multicast action
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID              -  Invalid port id    
 *      RT_ERR_INPUT                 -  Invalid input parameter 
 * Note:
 *      When receives an unknown multicast packet, switch may forward, drop this packet
 *      The unknown multicast packet type is as following:
 *               - MCAST_IPV4
 *               - MCAST_IPV6
 *      The unknown multicast action is as following:
 *               - MCAST_ACTION_FORWARD
 *               - MCAST_ACTION_DROP
 */
extern rtk_api_ret_t rtk_trap_unknownMcastPktAction_set(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t mcast_action);

/* Function Name:
 *      rtk_trap_unknownMcastPktAction_get
 * Description:
 *      Get behavior of unknown multicast
 * Input:
 *      port                  -   port id
 *      type                 -   unknown multicast packet type
 * Output:
 *      pMcast_action    -   the pointer of unknown multicast action
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID              -  Invalid port id 
 *	  RT_ERR_INPUT			-	invalid input parameter
 *      RT_ERR_NULL_POINTER      -  Input parameter is null pointer
 * Note:
 *      When receives an unknown multicast packet, switch may forward, drop this packet.
 *      The unknown multicast packet type is as following:
 *               - MCAST_IPV4
 *               - MCAST_IPV6
 *      The unknown multicast action is as following:
 *               - MCAST_ACTION_FORWARD
 *               - MCAST_ACTION_DROP
 */
extern rtk_api_ret_t rtk_trap_unknownMcastPktAction_get(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t *pMcast_action);

/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_set
 * Description:
 *      Set IGMP/MLD trap function
 * Input:
 *      type                -   IGMP/MLD packet type
 *      igmp_action         -   IGMP/MLD action
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_INPUT                 -  Invalid input parameter
 *      RT_ERR_NOT_ALLOWED     -  Actions not allowed by the function
 * Note:
 *      This API can set both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
 *      All 4 kinds of IGMP/MLD function can be set separately.
 *      The IGMP/MLD packet type is as following:
 *          - IGMP_IPV4
 *          - IGMP_MLD
 *          - IGMP_PPPOE_IPV4
 *          - IGMP_PPPOE_MLD
 *      The IGMP/MLD action is as following:
 *          - IGMP_ACTION_FORWARD
 *	      - IGMP_ACTION_COPY2CPU
 *          - IGMP_ACTION_TRAP2CPU
 *	      - IGMP_ACTION_DROP
 */ 
extern rtk_api_ret_t rtk_trap_igmpCtrlPktAction_set(rtk_igmp_type_t type, rtk_trap_igmp_action_t igmp_action);

/* Function Name:
 *      rtk_trap_igmpCtrlPktAction_get
 * Description:
 *      Get IGMP/MLD trap function
 * Input:
 *      type                -   IGMP/MLD packet type
 * Output:
 *      pIgmp_action    -   the pointer of IGMP/MLD action
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_INPUT                 -  Invalid input parameter
 *      RT_ERR_NULL_POINTER      -  Input parameter is null pointer
 * Note:
 *      This API can get both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
 *      All 4 kinds of IGMP/MLD function can be set seperately.
 *      The IGMP/MLD packet type is as following:
 *          - IGMP_IPV4
 *          - IGMP_MLD
 *          - IGMP_PPPOE_IPV4
 *          - IGMP_PPPOE_MLD
 *      The IGMP/MLD action is as following:
 *          - IGMP_ACTION_FORWARD
 *          - IGMP_ACTION_TRAP2CPU
 */
extern rtk_api_ret_t rtk_trap_igmpCtrlPktAction_get(rtk_igmp_type_t type, rtk_trap_igmp_action_t *pIgmp_action);

/* Function Name:
 *      rtk_vlan_init
 * Description:
 *      Initialize VLAN
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_FAILED                -  Failure
 * Note:
 *      VLAN function is disabled for ASIC after reset by default. User has to call this API to enable VLAN before
 *      using it. And It will set a default VLAN(vid 1) including all ports and set all ports's vlan index pointed 
 *	  to the default VLAN. So all port's PVID are 1.
 */
extern rtk_api_ret_t rtk_vlan_init(void);

/* Function Name:
 *      rtk_vlan_set
 * Description:
 *      Set a VLAN entry
 * Input:
 *      vid              - VLAN ID to configure, should be 1~4094
 *      mbrmsk        - VLAN member set portmask
 *      untagmsk     - VLAN untag set portmask
 *      fid          - filtering database id, should be 0 - 3 
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED                 -  Failure
 *      RT_ERR_VLAN_VID               -  Invalid vid
 *		RT_ERR_PORT_MASK			  -	 invalid port mask
 *      RT_ERR_INPUT                  -  Invalid input parameter 
 *      RT_ERR_TBL_FULL               -  Input table full 
 * Note:
 *     There are 16 VLAN entry supported for RTL8309N. User could configure the member port set and untag member port set for
 *     specified vid through this API. The vid is from 0 to 4095. The vid 0 is used for priority tagged frames which is treated as untagged frames.
 *	 The vid 4095 is reserved for further usage. 
 *	 The portmask's bit N means port N. For example, mbrmask 0x17 = 010111 means that port 0,1,2,4 are in the vlan's member port set.
 *     FID is for SVL/IVL usage, and the range is from 0 to 3. RTL8309N can only support 4 filtering database with the use of FID. 
 */
extern rtk_api_ret_t rtk_vlan_set(rtk_vlan_t vid, rtk_portmask_t mbrmsk, rtk_portmask_t untagmsk, rtk_fid_t fid);

/* Function Name:
 *      rtk_vlan_get
 * Description:
 *      Get a VLAN entry
 * Input:
 *      vid             - VLAN ID to configure
 * Output:
 *      pMbrmsk     - VLAN member set portmask
 *      pUntagmsk  - VLAN untag set portmask
 *      pFid           -  filtering database id
 * Return:
 *      RT_ERR_OK                                   -  Success
 *      RT_ERR_FAILED                              -  Failure
 *      RT_ERR_VLAN_VID                          -  Invalid vid
 *      RT_ERR_NULL_POINTER                    -  Input parameter is null pointer
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND    -   specified vlan entry not found 
 * Note:
 *     There are 16 VLAN entry supported for RTL8309N. User could configure the member port set and untag member port set for
 *     specified vid through this API. The vid is from 0 to 4095. The vid 0 is used for priority tagged frames which is treated as untagged frames.
 *	 The vid 4095 is reserved for further usage. 
 *	 The portmask's bit N means port N. For example, mbrmask 0x17 = 010111 means that port 0,1,2,4 are in the vlan's member port set.
 *     FID is for SVL/IVL usage, and the range is from 0 to 3. RTL8309N can only support 4 filtering database with the use of FID. 
 */
extern rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_portmask_t *pMbrmsk, rtk_portmask_t *pUntagmsk, rtk_fid_t *pFid);

/* Function Name:
 *      rtk_vlan_destroy
 * Description:
 *      delete a vlan entry from vlan table with specified vid
 * Input:
 *      vid             - VLAN ID to configure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                   -  Success
 *      RT_ERR_VLAN_VID                          -  Invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND    			 -  Specified vlan entry not found
 * Note:
 * 	This API can be called to delet a vlan entry from vlan table with specified vid. After it is called,
 *	the content of vlan entry will set to zero.
 */
extern rtk_api_ret_t rtk_vlan_destroy(rtk_vlan_t vid);

    
/* Function Name:
 *      rtk_vlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID)
 * Input:
 *      port             - Port id
 *      pvid             - Specified VLAN ID
 *      priority         - 802.1p priority for the PVID, 0~3 for RTL8309N 
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                   -  Success
 *      RT_ERR_FAILED                              -  Failure
 *	  RT_ERR_PORT_ID							-	invalid port id
 *      RT_ERR_VLAN_VID                          -  Invalid vid
 *      RT_ERR_VLAN_PRIORITY                  -  Invalid 1p priority 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND    -  Specified vlan entry not found
 * Note:
 *      The API is used for Port-based VLAN. The untagged frame received from the
 *      port will be classified to the specified port-based VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority);

/* Function Name:
 *      rtk_vlan_portPvid_get
 * Description:
 *      Get VLAN ID(PVID) on specified port
 * Input:
 *      port             - Port id
 * Output:
 *      pPvid            - Specified VLAN ID
 *      pPriority        - 802.1p priority for the PVID
 * Return:
 *      RT_ERR_OK                                   - Success
 *      RT_ERR_FAILED                             -  Failure
 *      RT_ERR_PORT_ID                           -  Invalid port id
 *      RT_ERR_NULL_POINTER                   -  Input parameter is null pointer
 * Note:
 *      The API is used for Port-based VLAN. The untagged frame received from the
 *      port will be classified to the specified port-based VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority);

/* Function Name:
 *      rtk_vlan_portIFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port
 * Input:
 *      port             - Port id, no use for RTL8309N
 *      igr_filter       - VLAN ingress function enable status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        - Success
 *      RT_ERR_FAILED                  -  Failure
 *	  RT_ERR_PORT_ID				-	invalid port id
 * Note:
 *      RTL8309N use one vlan ingress filter configuration for whole system, not for each port, so 
 *      any port you set will affect all ports's ingress filter setting.
 *      While VLAN function is enabled, ASIC will decide VLAN ID for each received frame 
 *      and get member ports for this vlan from VLAN table. If packets ingress port is in VLAN, 
 *	  ASIC will drop the received frame if VLAN ingress filter function is enabled.
 */
extern rtk_api_ret_t rtk_vlan_portIFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter);

/* Function Name:
 *      rtk_vlan_portIFilterEnable_get
 * Description:
 *      get VLAN ingress for each port
 * Input:
 *      port             - Port id, no use for RTL8309N
 * Output:
 *      pIgr_filter     - the pointer of VLAN ingress function enable status
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_FAILED           -  Failure
 *	  RT_ERR_PORT_ID			-	invalid port id
 *	  RT_ERR_NULL_POINTER		-	input parameter is null pointer
 * Note:
 *      RTL8309N use one ingress filter configuration for whole system, not for each port, so 
 *      any port you set will affect all ports ingress filter setting.
 *      While VLAN function is enabled, ASIC will decide VLAN ID for each received frame 
 *      and get belonged member ports from VLAN table. If received port is not belonged 
 *      to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
extern rtk_api_ret_t rtk_vlan_portIFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN support frame type
 * Input:
 *      port                          - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                  - Success
 *      RT_ERR_FAILED            -  Failure
 *      RT_ERR_PORT_ID          -  Invalid port id
 *	  RT_ERR_VLAN_ACCEPT_FRAME_TYPE	-	invalid accept frame type 
 * Note:
 *    The API is used for ingress port to check 802.1Q tagged frames.
 *    The ingress ports's accept frame type could be set to values as follows:
 *          -	ACCEPT_FRAME_TYPE_AL
 *          -	ACCEPT_FRAME_TYPE_TAG_ONLY
 *          -	ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN support frame type
 * Input:
 *      port                          - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                   - Success
 *      RT_ERR_FAILED                             -  Failure
 *      RT_ERR_PORT_ID                           -  Invalid port id
 *	  RT_ERR_NULL_POINTER				-	input parameter is null pointer
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE -  Invalid accept frame type 
 * Note:
 *    The API is used for ingress port to check 802.1Q tagged frames.
 *    The ingress ports's accept frame type could be set to values as follows:
 *          -	ACCEPT_FRAME_TYPE_AL
 *          -	ACCEPT_FRAME_TYPE_TAG_ONLY
 *          -	ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      rtk_stp_mstpState_set
 * Description:
 *      Configure spanning tree state per port
 * Input:
 *      msti              - Multiple spanning tree instance, no use for RTL8309N
 *      port              - Port id
 *      stp_state         - Spanning tree state
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID              -  Invalid port id
 *      RT_ERR_MSTP_STATE        -  Invalid spanning tree status
 * Note:
 *      Because RTL8309N does not support multiple spanning tree, so msti is no use. 
 *      There are four states supported by ASIC.
 *          -	STP_STATE_DISABLED
 *          -	STP_STATE_BLOCKING
 *          -	STP_STATE_LEARNING
 *          -	STP_STATE_FORWARDING
 */
extern rtk_api_ret_t rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stp_state);

/* Function Name:
 *      rtk_stp_mstpState_get
 * Description:
 *      Get Configuration of spanning tree state per port
 * Input:
 *      msti              - Multiple spanning tree instance, no use for RTL8309N
 *      port              - Port id
 * Output:
 *      pStp_state     - the pointer of Spanning tree state
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID              -  Invalid port id
 *      RT_ERR_NULL_POINTER      -  Input parameter is null pointer
 * Note:
 *      Because RTL8309N does not support multiple spanning tree, so msti is no use. 
 *      There are four states supported by ASIC.
 *          -	STP_STATE_DISABLED
 *          -	STP_STATE_BLOCKING
 *          -	STP_STATE_LEARNING
 *          -	STP_STATE_FORWARDING
 */
extern rtk_api_ret_t rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t *pStp_state);

/* Function Name:
 *      rtk_l2_addr_add
 * Description:
 *      Add a unicast entry into LUT table
 * Input:
 *		pMac			-	pointer point to structure of unicastmac address
 *		fid				-	fid value(0~3)
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                       -  Success
 *      RT_ERR_FAILED                 -   Failure
 *      RT_ERR_INPUT                  -   Invalid input parameter
 *      RT_ERR_MAC                    -   invalid mac address      
 *      RT_ERR_NULL_POINTER       -  Input parameter is null pointer    
 *      RT_ERR_L2_INDEXTBL_FULL -  The L2 index table is full
 * Note:
 *      (1)The lut has a 4-way entry due to an index. If the macAddress has existed in the lut, it will update the entry 
 *		with the user's defined entry content, otherwise the function will find an empty entry to put it.
 *          When the index is full, it will find a dynamic & unauth unicast macAddress entry to replace with it. 
 *      (2)If the mac address has been added into LUT, function return value is SUCCESS,  *pEntryaddr is recorded the 
 *          entry address of the Mac address stored.
 *          If all the four entries can not be replaced, it will return a  RTL8309N_LUT_FULL error, you can delete one of them 
 *          manually and rewrite the unicast address. 
 *      (3) The age of the look up table entry could be:
 *              -	AGE_TIME_OUT
 *              -	AGE_TIME_100S
 *              -	AGE_TIME_200S
 *              -	AGE_TIME_300S
 */
extern rtk_api_ret_t rtk_l2_addr_add(rtk_mac_t *pMac, rtk_fid_t fid, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      rtk_l2_addr_get
 * Description:
 *      Get a unicast entry from LUT table
 * Input:
 *      pMac               -   6 bytes unicast(I/G bit is 0) mac address to be gotten
 *      fid                   -   filtering database id, could be any value for RTL8309N switch
 * Output:
 *      pL2_data          -   the mac address attributes
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *	  RT_ERR_L2_FID						-	invalid fid
 *      RT_ERR_MAC                            -   invalid mac address        
 *      RT_ERR_NULL_POINTER              -   Input parameter is null pointer    
 *      RT_ERR_L2_ENTRY_NOTFOUND    -   Specified entry not found
 * Note:
 *      (1)The lut has a 4-way entry due to an index. If the macAddress has existed in the lut, 
 *		This API will return the entry content.		
 */
extern rtk_api_ret_t rtk_l2_addr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      rtk_l2_addr_del
 * Description:
 *      Delete a LUT unicast entry
 * Input:
 *      pMac               -   6 bytes unicast mac address to be deleted
 *      fid                   -   filtering database id, could be any value for RTL8309N switch
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac 
 *	  RT_ERR_L2_FID						-	invalid fid
 *      RT_ERR_NULL_POINTER              -   Input parameter is null pointer    
 *      RT_ERR_L2_ENTRY_NOTFOUND    -   Specified entry not found 
 * Note:
 *      If the mac has existed in the LUT, it will be deleted.
 *      Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
extern rtk_api_ret_t rtk_l2_addr_del(rtk_mac_t *pMac, rtk_fid_t fid);

/* Function Name:
 *      rtk_l2_mcastAddr_add
 * Description:
 *      Add a LUT multicast entry
 * Input:
 *      pMac               -   6 bytes unicast mac address to be deleted
 *      fid                   -   filtering database id, could be any value for RTL8309N switch
 *      portmask          -   Port mask to be forwarded to 
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_PORT_MASK                  -   Invalid port mask
 *      RT_ERR_MAC                            -   invalid mac address 
 *	  RT_ERR_NULL_POINTER			-	input parameter is null pointer
 *      RT_ERR_L2_INDEXTBL_FULL         -   the L2 index table is full
 * Note:
 *      If the multicast mac address already existed in the LUT, it will udpate the
 *      port mask of the entry. Otherwise, it will find an empty or asic auto learned
 *      entry to write. If all the entries with the same hash value can't be replaced, 
 *      ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 */
extern rtk_api_ret_t rtk_l2_mcastAddr_add(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t portmask);

/* Function Name:
 *      rtk_l2_mcastAddr_get
 * Description:
 *      Get a LUT multicast entry
 * Input:
 *      pMac               -   6 bytes multicast(I/G bit is 0) mac address to be gotten
 *      fid                   -   filtering database id, could be any value for RTL8309N switch
 * Output:
 *      pPortmask         -   the pointer of port mask      
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_NULL_POINTER              -   Input parameter is null pointer    
 *      RT_ERR_MAC                            -   invalid mac address        
 *      RT_ERR_L2_ENTRY_NOTFOUND         -   specified entry not found
 * Note:
 *      If the multicast mac address existed in LUT, it will return the port mask where
 *      the packet should be forwarded to, Otherwise, it will return a 
 *      RT_ERR_L2_ENTRY_NOTFOUND error.
 */
extern rtk_api_ret_t rtk_l2_mcastAddr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_l2_mcastAddr_del
 * Description:
 *      Delete a LUT unicast entry
 * Input:
 *      pMac               -   6 bytes multicast(I/G bit is 1) mac address to be gotten
 *      fid                   -   filtering database id, could be any value for RTL8309N switch
 * Output:
*       none
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_MAC                            -   invalid mac address
 *	  RT_ERR_L2_FID				-	invalid fid
 *      RT_ERR_L2_ENTRY_NOTFOUND     -   specified entry not found
 * Note:
 *      If the mac has existed in the LUT, it will be deleted.
 *      Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
extern rtk_api_ret_t rtk_l2_mcastAddr_del(rtk_mac_t *pMac, rtk_fid_t fid);


/* Function Name:
 *		rtk_l2_limitLearningSysCntEnable_set
 * Description:
 *		Enable system mac learning limit function
 * Input:
 *		enabled		-	ENABLED or DISABLED
 * Output:
 *		none
 * Return:
 *		RT_ERR_FAILED	-	failure
 *		RT_ERR_OK		-	success
 * Note:
 *		For RTL8309N, mac learning limit function can be enabled or disabled for a whole system.
 */
extern rtk_api_ret_t rtk_l2_limitLearningSysCntEnable_set(rtk_enable_t enabled);

/* Function Name:
 *		rtk_l2_limitLearningSysCntEnable_set
 * Description:
 *		Get enabled status of system mac learning limit function
 * Input:
 *		none
 * Output:
 *		pEnabled	-	pointer point to enabled status of system mac learning limit function
 * Return:
 *		RT_ERR_FAILED	-	failure
 *		RT_ERR_OK		-	success
 * Note:
 *		For RTL8309N, mac learning limit function can be enabled or disabled for a whole system.
 */
extern rtk_api_ret_t rtk_l2_limitLearningSysCntEnable_get(rtk_enable_t *pEnabled);

/* Function Name:
 *		rtk_l2_limitLearningSysCnt_set
 * Description:
 *      Set system mac limitting max value and port merge mask
 * Input:
 *		mac_cnt		-	system mac limitting value
 *		mergeMask	-	a set describing the ports whose port mac limitting value are counted into system mac limitting value
 * Output:
 *      none
 * Return:
 *      RT_ERR_FAILED	-	failure
 *		RT_ERR_OK		-	success
 *		RT_ERR_INPUT	-	invalid input parameter 
 * Note:
 *		(1) This API can be called to set system mac limitting max value and port merge mask.
 *		(2)	mac_cnt: the whole system mac limitting max value, it's value is from 0 - 0xFF;
 *		(3) mergeMask: the ports whose mac limitting counter value are counted into the system mac limitting counter value,
 *			it's value is from 0 - 0x1FF. If bit n is 1, it means that port n is counted.
 */
extern rtk_api_ret_t rtk_l2_limitLearningSysCnt_set(rtk_mac_cnt_t mac_cnt, rtk_portmask_t mergeMask);

/* Fcuntion Name:
 *		rtk_l2_limitLearningSysCnt_get
 * Description:
 *		Get system mac limitting max value and merge mask.
 * Input:
 *		none
 * Output:
 *		pMac_cnt	-	pointer point to system mac limitting max value
 *		pMergeMask	-	pointer point to merge mask
 * Return:
 *		RT_ERR_FAILED	-	failure
 *		RT_ERR_OK		-	success
 *		RT_ERR_NULL_POINTER		-	pointer is NULL
 * Note:
 *		(1) This API can be called to get system mac limitting max value and port merge mask.
 *		(2)	mac_cnt: the whole system mac limitting max value, it's value is from 0 - 0xFF;
 *		(3) mergeMask: the ports whose mac limitting counter value are counted into the system mac limitting counter value,
 *			it's value is from 0 - 0x1FF. If bit n is 1, it means that port n is counted.
 */
extern rtk_api_ret_t rtk_l2_limitLearningSysCnt_get(rtk_mac_cnt_t *pMac_cnt, rtk_portmask_t *pMergeMask);

/* Function Nmae:
 *		rtk_l2_limitLearningCntEnable_set
 * Description:
 *		Enable port mac limit learning function
 * Input:
 *		port	-	port id
 *		enabled	-	enable or disable
 * Output:
 *		none
 * Return:
 *		RT_ERR_PORT_ID	-	invalid port id
 *		RT_ERR_FAILED	-	failure
 *		RT_ERR_OK		-	success
 * Note:
 *		This API can be called to enable or disable mac limit learning function for a port.
 */
extern rtk_api_ret_t rtk_l2_limitLearningCntEnable_set(rtk_port_t port, rtk_enable_t enabled);

/* Function Nmae:
 *		rtk_l2_limitLearningCntEnable_get
 * Description:
 *		Get enabled status of port mac limit learning function
 * Input:
 *		port	-	port id
 * Output:
 *		pEnabled	-	enable or disable
 * Return:
 *		RT_ERR_PORT_ID			-	invalid port id
 *		RT_ERR_NULL_POINTER		-	input parameter is null pointer
 *		RT_ERR_FAILED			-	failure
 *		RT_ERR_OK				-	success
 * Note:
 *		This API can be called to get the enabled status of mac limit learning function for a port.
 */
extern rtk_api_ret_t rtk_l2_limitLearningCntEnable_get(rtk_port_t port, rtk_enable_t *pEnabled);

/* Function Name:
 *      rtk_l2_limitLearningCnt_set
 * Description:
 *      Set per-Port auto learning limit counter max value
 * Input:
 *      port 		- 	Port id	
 *      mac_cnt		- 	mac limit counter value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - 	set shared meter successfully
 *      RT_ERR_FAILED          -	 FAILED to iset shared meter
 *      RT_ERR_PORT_ID 		- 	Invalid port id.
 *      RT_ERR_LIMITED_L2ENTRY_NUM 	- 	invalid limited L2 entry number 
 * Note:
 *      (1) Per port mac learning limit function can be enabled or disabled independently;
 *      (2) mac_cnt: port mac learning limit max value, it's value is from 0 - 0x1F;
 */
extern rtk_api_ret_t rtk_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt);

/* Function Name:
 *      rtk_l2_limitLearningCnt_get
 * Description:
 *      Get per-Port auto learning limit counter max value
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt 	- 	mac limit counter value
 * Return:
 *      RT_ERR_OK              - 	Success
 *      RT_ERR_FAILED          - 	Failure 
 *      RT_ERR_PORT_ID 		   -    Invalid port id. 
 *      RT_ERR_NULL_POINTER    - 	input parameter is NULL pointer
 * Note:
 *      (1) Per port mac learning limit function can be enabled or disabled independently;
 *      (2) mac_cnt: port mac learning limit max value, it's value is from 0 - 0x1F;
 */
extern rtk_api_ret_t rtk_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);

/* Function Name:
 *      rtk_l2_limitLearningCntAction_set
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *		port	-	port id, no usage for RTL8309N
 *      action 	- Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure 
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_NOT_ALLOWED - Invalid learn over action
 * Note:
 *      (1)The API can set SA unknown packet action while auto learn limit number is over. 
 *      The action symbol as following:
 *          -	LIMIT_LEARN_CNT_ACTION_DROP
 *          -	LIMIT_LEARN_CNT_ACTION_TO_CPU
 */
extern rtk_api_ret_t rtk_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action);

/* Function Name:
 *      rtk_l2_limitLearningCntAction_get
 * Description:
 *      Get auto learn over limit number action.
 * Input:
 *		port	-	port id, no usage for RTL8309N
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure 
 *      RT_ERR_PORT_ID 	- Invalid port id. 
 *	  RT_ERR_NULL_POINTER	-	null pointer
 *	  
 * Note:
 *      (1)The API can get SA unknown packet action while auto learn limit number is over. 
 *      The action symbol as following:
 *          -	LIMIT_LEARN_CNT_ACTION_DROP  
 *          -	LIMIT_LEARN_CNT_ACTION_TO_CPU 
 */
extern rtk_api_ret_t rtk_l2_limitLearningCntAction_get(rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction);

/* Function Name:
 *		rtk_l2_learningSysCnt_get
 * Description:
 *		Get current value of system auto learning mac counter
 * Input:
 *		none
 * Output:
 *		pMac_cnt	-	mac counter
 * Return:
 *		RT_ERR_FAILED
 *		RT_ERR_OK
 *		RT_ERR_NULL_POINTER
 * Note:
 */
extern rtk_api_ret_t rtk_l2_learningSysCnt_get(rtk_mac_cnt_t *pMac_cnt);

/* Function Name:
 *      rtk_l2_learningCnt_get
 * Description:
 *      Get current value of per-Port auto learning counter
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - ASIC auto learning entries number
 * Return:
 *      RT_ERR_OK              - 	Success
 *	  RT_ERR_FAILED		-	Failure
 *      RT_ERR_PORT_ID 	- 	Invalid port number. 
 *      RT_ERR_NULL_POINTER   -   Input parameter is null pointer  
 * Note:
 *      The API can get per-port ASIC auto learning number
 */
extern rtk_api_ret_t rtk_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);

/* Function Name:
 *		rtk_cpu_enable_set
 * Description:
 *		Enable cpu port ability
 * Input:
 *		enabled	-	enable or disable	
 * Output:
 *		none
 * Return:
 *		RT_ERR_FAILED	-	failure
 *		RT_ERR_OK		-	success
 * Note:
 *		
 */
extern rtk_api_ret_t rtk_cpu_enable_set(rtk_enable_t enabled);

/* Function Name:
 *		rtk_cpu_enable_get
 * Description:
 *		Enable cpu port ability
 * Input:
 *		none	
 * Output:
 *		pEnabled	-	enable or disable	
 * Return:
 *		RT_ERR_FAILED	-	failure
 *		RT_ERR_OK		-	success
 *		RT_ERR_NULL_POINTER	-	input parameter is null pointer
 * Note:
 *			
 */
extern rtk_api_ret_t rtk_cpu_enable_get(rtk_enable_t *pEnabled);

/* Function Name:
 *		rtk_cpu_tagPort_set
 * Description:
 *		Set cpu port and insert cpu tag
 * Input:
 *		port		-	port id
 *		enTag	-	enable insert cpu tag, enable or disable	
 * Output:
 *		none
 * Return:
 *		RT_ERR_PORT_ID	-	invalid port id
 *		RT_ERR_FAILED	-	failure
 *		RT_ERR_OK		-	success
 * Note:
 *		
 */
extern rtk_api_ret_t rtk_cpu_tagPort_set(rtk_port_t port, rtk_enable_t enTag);

/* Function Name:
 *		rtk_cpu_tagPort_get
 * Description:
 *		Get cpu port and insert cpu tag status
 * Input:
 *		port	-	port id(0 - 8)
 *		enTag	-	enable insert cpu tag, enable or disable	
 * Output:
 *		none
 * Return:
 *		RT_ERR_PORT_ID	-	invalid port id
 *		RT_ERR_FAILED	-	failure
 *		RT_ERR_OK		-	success
 * Note:
 *		
 */
extern rtk_api_ret_t rtk_cpu_tagPort_get(rtk_port_t *pPort, rtk_enable_t *pEnTag);


/* Function Name:
 *      rtk_mirror_portBased_set
 * Description:
 *      Set port mirror function parameters
 * Input:
 *      mirroring_port             - Monitor port, 7 means no monitor port
 *      pMirrored_rx_portmask      - the pointer of Rx mirror port mask
 *      pMirrored_tx_portmask      - the pointer of Tx mirror port mask
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_PORT_MASK         -   Invalid port mask
 *	  RT_ERR_PORT_ID			-	invalid port id
 *	  RT_ERR_NULL_POINTER		-	input parameter is null pointer
 * Note:
 *      The API is called to set mirroring port and mirrored rx and tx port mask. 
 */
extern rtk_api_ret_t rtk_mirror_portBased_set(rtk_port_t mirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);

/* Function Name:
 *      rtk_mirror_portBased_get
 * Description:
 *      Get port mirror function parameters
 * Input:
 *      none 
 * Output:
 *      pMirroring_port             - the pointer Monitor port, 7 means no monitor port
 *      pMirrored_rx_portmask   - the pointer of Rx mirror port mask
 *      pMirrored_tx_portmask   - the pointer of Tx mirror port mask 
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_NULL_POINTER      -   Input parameter is null pointer 
 * Note:
 *      The API is to set mirror function of source port and mirror port. 
 */
extern rtk_api_ret_t rtk_mirror_portBased_get(rtk_port_t *pMirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);

/* Function Name:
 *      rtk_mirror_macBased_set
 * Description:
 *      Set Mac address for mirror packet
 * Input:
 *      macAddr   - mirrored mac address, it could be SA or DA of the packet 
 *      enabled   - enable mirror packet by mac address
 * Output:
 *      none
 * Return:
 *      RT_ERR_FAILED	-	failure
 *      RT_ERR_OK		-	success
 * Note:
 */
extern rtk_api_ret_t rtk_mirror_macBased_set(rtk_mac_t *macAddr, rtk_enable_t enabled);

/* Function Name:
 *      rtk_mirror_macBased_get
 * Description:
 *      get Mac address for mirror packet
 * Input:
 *      none 
 * Output:
 *      macAddr    - mirrored mac address, it could be SA or DA of the packet 
 *      pEnabled   - the pointer of enable mirror packet by mac address 
 * Return:
 *      RT_ERR_FAILED	-	failure
 *      RT_ERR_OK		-	success
 * Note:
 */
extern rtk_api_ret_t rtk_mirror_macBased_get(rtk_mac_t *macAddr, uint32 *pEnabled);

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_set
 * Description:
 *      Set 802.1x unauth action configuration
 * Input:
 *      port                 -   Port id, no use for RTL8309N switch
 *      unauth_action        -   802.1X unauth action    
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_DOT1X_PROC        -   Unauthorized behavior error
 * Note:
 *      This API can set 802.1x unauth action configuration, 
 *      for RTL8309N switch, the action is by whole system,
 *      so port could be any value of 0~8.
 *      The unauth action is as following:
 *          -	DOT1X_ACTION_DROP
 *          -	DOT1X_ACTION_TRAP2CPU
 */
extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action);

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_get
 * Description:
 *      Get 802.1x unauth action configuration
 * Input:
 *      port                  -   Port id, no use for RTL8309N switch
 * Output:
 *      pUnauth_action   -  the pointer of 802.1X unauth action    
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_NULL_POINTER      -   Input parameter is null pointer
 * Note:
 *      This API can set 802.1x unauth action configuration, for RTL8309N switch, the action is by whole system,
 *      so port could be any value of 0~8.
 *      The unauth action is as following:
 *          -	DOT1X_ACTION_DROP
 *          -	DOT1X_ACTION_TRAP2CPU
 */
extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_get(rtk_port_t port, rtk_dot1x_unauth_action_t *pUnauth_action);

/* Function Name:
 *      rtk_dot1x_portBasedEnable_set
 * Description:
 *      Set 802.1x port-based enable configuration
 * Input:
 *      port                  -   Port id
 *      enable               -   enable or disable
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_PORT_ID              -   Invalid port id
 * Note:
 *      The API can update the port-based port enable register content. If a port is 802.1x 
 *      port based network access control "enabled", it should be authenticated so packets 
 *      from that port won't be dropped or trapped to CPU. 
 *      The status of 802.1x port-based network access control is as following:
 *          - DISABLED
 *          - ENABLED
 */    
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_set(rtk_port_t port, rtk_enable_t enabled);

/* Function Name:
 *      rtk_dot1x_portBasedEnable_get
 * Description:
 *      Get 802.1x port-based enable configuration
 * Input:
 *      port                  -   Port id
 * Output:
 *      pEnable             -   the pointer of enable or disable
 * Return:
 *      RT_ERR_OK                -  Success
 *      RT_ERR_FAILED          -   Failure
 *      RT_ERR_PORT_ID        -   Invalid port id
 * Note:
 *      The API can update the port-based port enable register content. If a port is 802.1x 
 *      port based network access control "enabled", it should be authenticated so packets 
 *      from that port won't be dropped or trapped to CPU. 
 *      The status of 802.1x port-based network access control is as following:
 *          - DISABLED
 *          - ENABLED
 */    
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set 802.1x port-based enable configuration
 * Input:
 *      port                  -   Port id
 *      port_auth          -  The status of 802.1x port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                    -  Success
 *      RT_ERR_FAILED                              -   Failure
 *      RT_ERR_PORT_ID                            -   Invalid port id
 *      RT_ERR_DOT1X_PORTBASEDAUTH      -   Port-based auth port error
 * Note:
 *      The authenticated status of 802.1x port-based network access control is as following:
 *          -	UNAUTH
 *          -	AUTH
 */    
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_set(rtk_port_t port, rtk_dot1x_auth_status_t port_auth);

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_set
 * Description:
 *      Set 802.1x port-based enable configuration
 * Input:
 *      port                  -   Port id
 *      port_auth          -  The status of 802.1x port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                    -  Success
 *      RT_ERR_FAILED                              -   Failure
 *      RT_ERR_PORT_ID                            -   Invalid port id
 *      RT_ERR_DOT1X_PORTBASEDAUTH      -   Port-based auth port error
 * Note:
 *      The authenticated status of 802.1x port-based network access control is as following:
 *          -	UNAUTH
 *          -	AUTH
 */   
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_get(rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth);

/* Function Name:
 *      rtk_dot1x_portBasedDirection_set
 * Description:
 *      Set 802.1x port-based operational direction configuration
 * Input:
 *      port                -   Port id
 *      port_direction    	-   Operation direction
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                 -  Success
 *      RT_ERR_FAILED                           -  Failure
 *      RT_ERR_PORT_ID                         -  Invalid port id
 *      RT_ERR_DOT1X_PORTBASEDOPDIR  -  Port-based opdir error
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *          -	BOTH 
 *          -	IN
 */   
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_set(rtk_port_t port, rtk_dot1x_direction_t port_direction);

/* Function Name:
 *      rtk_dot1x_portBasedDirection_get
 * Description:
 *      Get 802.1x port-based operational direction configuration
 * Input:
 *      port                  -   Port id
 * Output:
 *      pPort_direction    -   the pointer of Operation direction
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_PORT_ID                  -  Invalid port id
 *      RT_ERR_NULL_POINTER          -  Input parameter is null pointer
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *          -	BOTH
 *          -	IN
 */   
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_get(rtk_port_t port, rtk_dot1x_direction_t *pPort_direction);

/* Function Name:
 *      rtk_dot1x_macBasedEnable_set
 * Description:
 *      Set 802.1x mac-based port enable configuration
 * Input:
 *      port                  -   Port id
 *      enable                -   The status of 802.1x mac-base funtion
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_PORT_ID                  -  Invalid port id
 * Note:
 *     If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *     be authenticated so packets from that port won't be dropped or trapped to CPU.
 */    
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_set(rtk_port_t port, rtk_enable_t enabled);

/* Function Name:
 *      rtk_dot1x_macBasedEnable_get
 * Description:
 *      Get 802.1x mac-based port enable configuration
 * Input:
 *      port                -   Port id
 * Output:
 *      pEnable             -   the pointer of the status of 802.1x mac-base funtion
 * Return:
 *      RT_ERR_OK               -  Success
 *      RT_ERR_FAILED         -   Failure
 *		RT_ERR_PORT_ID		-	invalid port id
 *		RT_ERR_NULL_POINTER		-	input parameter is null pointer
 * Note:
 *     If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *     be authenticated so packets from that port won't be dropped or trapped to CPU.
 */    
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_dot1x_macBasedDirection_set
 * Description:
 *      Set 802.1x mac-based operational direction configuration
 * Input:
 *      mac_direction    -   Operation direction
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                    -  Success
 *      RT_ERR_FAILED                              -  Failure
 *      RT_ERR_DOT1X_MACBASEDOPDIR      -  MAC-based opdir error
 * Note:
 *      The operate controlled direction of 802.1x mac-based network access control is as following:
 *          -	BOTH
 *          -	IN
 */    
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_set(rtk_dot1x_direction_t mac_direction);

    
/* Function Name:
 *      rtk_dot1x_macBasedDirection_get
 * Description:
 *      Get 802.1x mac-based operational direction configuration
 * Input:
 *      none
 * Output:
 *      pMac_direction    -   the pointer of Operation direction
 * Return:
 *      RT_ERR_OK                                    -  Success
 *      RT_ERR_FAILED                              -  Failure
 *      RT_ERR_NULL_POINTER                    -  Input parameter is null pointer
 *		RT_ERR_PORT_ID				-	Wrong port ID
 * Note:
 *      The operate controlled direction of 802.1x mac-based network access control is as following:
 *          -	BOTH
 *          -	IN
 */
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_get(rtk_dot1x_direction_t *pMac_direction);

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      pAuth_mac       - The authenticated MAC
 *      fid             - no use for RTL8309N   
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                       - Success
 *      RT_ERR_FAILED                                 -  Failure
 *      RT_ERR_L2_ENTRY_NOTFOUND             -  Specified entry not found
 *	  RT_ERR_L2_FID						-	invalid	fid
 *	  RT_ERR_MAC							-	invalid mac address
 *      RT_ERR_DOT1X_MAC_PORT_MISMATCH 		 - Auth MAC and port mismatch eror 
 *	  RT_ERR_PORT_ID						-	invalid port id
 * Note:
 *     The API can add a 802.1x authorised MAC address to port. If the MAC does not exist in LUT, 
 *     user can't add this MAC with authorised status.
 */    
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_add(rtk_mac_t *pAuth_mac, rtk_fid_t fid);

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_del
 * Description:
 *      Delete an authenticated MAC to ASIC
 * Input:
 *      pAuth_mac       - The authenticated MAC
 *      fid             -  no use for RTL8309N   
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                       - Success
 *      RT_ERR_FAILED                                 -  Failure
 *      RT_ERR_L2_ENTRY_NOTFOUND             -  Specified entry not found
 *      RT_ERR_DOT1X_MAC_PORT_MISMATCH  - Auth MAC and port mismatch eror 
 *	  RT_ERR_L2_FID		-	invalid fid 
 * Note:
 *     The API can delete a 802.1x authenticated MAC address to port. It only change the auth status of
 *     the MAC and won't delete it from LUT.
 */
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_del(rtk_mac_t *pAuth_mac, rtk_fid_t fid);

/* Function Name:
 *      rtk_filter_igrAcl_init
 * Description:
 *      Initialize ACL 
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK		-	success
 *      RT_ERR_FAILED	-	failure
 * Note:
 *      The API init ACL module.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_init(void);

/* Function Name:
 *      rtk_filter_igrAcl_rule_add
 * Description:
 *      Add an acl rule into acl table
 * Input:
 *      pRule    -  the pointer of rule structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK	-	success
 *      RT_ERR_FAILED	-	failure
 *      RT_ERR_TBL_FULL		-	input table full  	
 *      RT_ERR_NULL_POINTER		-	input parameter is null pointer
 * Note:
 *      (1)The API add an  ACL rule. 
 *          phyport could be port 0~8, RTK_ACL_INVALID_PORT, RTK_ACL_ANYPORT;
 *      (2)protocol could be :
 *           -	ACL_PRO_ETHER
 *           -	ACL_PRO_TCP
 *           -	ACL_PRO_UDP
 *           -	ACL_PRO_TCPUDP
 *      (3)prority could be 0-3;
 *      (4)action could be :
 *           -	ACL_ACT_DROP
 *           -	ACL_ACT_PERMIT
 *           -	ACL_ACT_TRAP2CPU
 *           -	ACL_ACT_MIRROR
 */
extern rtk_api_ret_t rtk_filter_igrAcl_rule_add(rtk_filter_rule_t *pRule);

/* Function Name:
 *      rtk_filter_igrAcl_rule_get
 * Description:
 *      Get ACL rule priority and action 
 * Input:
 *      pRule    -  the pointer of rule structure
 * Output:
 *      pRule    -  the pointer of rule structure
 * Return:
 *      RT_ERR_OK			-	success
 *      RT_ERR_FAILED			-	failure
 *		RT_ERR_NULL_POINTER		-	input parameter is null pointer
 * Note:
 *      (1)The API add an  ACL rule. 
 *          phyport could be port 0~8, RTK_ACL_INVALID_PORT and  RTK_ACL_ANYPORT;
 *      (2)protocol could be :
 *           -	ACL_PRO_ETHER
 *           -	ACL_PRO_TCP
 *           -	ACL_PRO_UDP
 *           -	ACL_PRO_TCPUDP
 *      (3)prority could be 0-3;
 *      (4)action could be :
 *           -	ACL_ACT_DROP
 *           -	ACL_ACT_PERMIT
 *           -	ACL_ACT_TRAP2CPU
 *           -	ACL_ACT_MIRROR
 */
extern rtk_api_ret_t rtk_filter_igrAcl_rule_get(rtk_filter_rule_t *pRule);

/* Function Name:
 *      rtk_filter_igrAcl_rule_del
 * Description:
 *      Delete an acl rule from acl table
 * Input:
 *      pRule    -  the pointer of rule structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK				-	success
 *      RT_ERR_FAILED			-	failure
 *      RT_ERR_INPUT			-	invalid input parameter
 *      RT_ERR_NULL_POINTER		-	input parameter is null pointer
 * Note:
 *      (1)The API delet an ACL rule. 
 *          phyport could be port 0~8, RTK_ACL_INVALID_PORT and  RTK_ACL_ANYPORT;
 *      (2)protocol could be :
 *           -	ACL_PRO_ETHER
 *           -	ACL_PRO_TCP
 *           -	ACL_PRO_UDP
 *           -	ACL_PRO_TCPUDP
 *      (3)prority could be 0-3;
 *      (4)action could be :
 *           -	ACL_ACT_DROP
 *           -	ACL_ACT_PERMIT
 *           -	ACL_ACT_TRAP2CPU
 *           -	ACL_ACT_MIRROR
 */
extern rtk_api_ret_t rtk_filter_igrAcl_rule_del(rtk_filter_rule_t *pRule);

/* Function Name:
 *		rtk_storm_filterEnable_set
 * Description:
 *		Enable storm filter
 * Input:
 *		port			-	port id
 *		storm_type	-	storm filter type
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		RT_ERR_FAILED		-	failure
 *		RT_ERR_OK			-	success
 *		RT_ERR_PORT_ID		-	invalid port id
 *		RT_ERR_INPUT		-	invalid input parameter
 * Note:
 */
extern rtk_api_ret_t rtk_storm_filterEnable_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_enable_t enabled);

/* Function Name:
 *		rtk_storm_filterEnable_get
 * Description:
 *		Enable storm filter
 * Input:
 *		port			-	port id
 *		storm_type	-	storm filter type
 * Output:
 *		pEnabled		-	enable or disable
 * Return:
 *		RT_ERR_FAILED		-	failure
 *		RT_ERR_OK			-	success
 *		RT_ERR_PORT_ID		-	invalid port id
 *		RT_ERR_INPUT		-	invalid input parameter
 * Note:
 */
extern rtk_api_ret_t rtk_storm_filterEnable_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_enable_t *pEnabled);

/* Function Name:
 *		rtk_storm_filterAttr_set
 * Description:
 *		Set storm filter attributes
 * Input:
 *		port	-	port id(0 - 8)
 *		storm_type	-	storm filter type
 *		pStorm_data	-	storm filter data
 * Output:
 *		none
 * Return:
 *		RT_ERR_PORT_ID		-	invalid port id
 *		RT_ERR_NULL_POINTER		-	input parameter is null pointer
 *		RT_ERR_FAILED		-	failure
 *		RT_ERR_OK		-	success
 * Note:
 */
extern rtk_api_ret_t rtk_storm_filterAttr_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_storm_attr_t *pStorm_data);

/* Function Name:
 *		rtk_storm_filterAttr_get
 * Description:
 *		Get storm filter attributes
 * Input:
 *		port	-	port id(0 - 8)
 *		storm_type	-	storm filter type
 * Output:
 *		pStorm_data	-	pointer point to structure describing storm filter data
 * Return:
 *		RT_ERR_PORT_ID		-	invalid port id
 *		RT_ERR_NULL_POINTER		-	input parameter is null pointer
 *		RT_ERR_INPUT			-	invalid input parameter
 *		RT_ERR_FAILED		-	failure
 *		RT_ERR_OK		-	success
 */
extern rtk_api_ret_t rtk_storm_filterAttr_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_storm_attr_t *pStorm_data);

/* Function Name:
 *		rtk_storm_filterStatus_set
 * Description:
 *		Clearing storm filter flag
 * Input:
 *		port			-	port id
 *		storm_type	-	storm filter type
 *		enabled		-	enable or disable
 * Output:
 *		none
 * Return:
 *		RT_ERR_PORT_ID		-	invalid port id
 *		RT_ERR_INPUT		-	invalid input parameter
 *		RT_ERR_FAILED		-	failure
 *		RT_ERR_OK			-	success
 * Note:
 */
extern rtk_api_ret_t rtk_storm_filterStatus_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_enable_t enabled);

/* Function Name:
 *		rtk_storm_filterStatus_get
 * Description:
 *		Get storm filter flag status
 * Input:
 *		port	-	port id
 *		storm_type	-	storm filter type
 * Output:
 *		pExceed	-	exceed storm filter, exceed or not
 * Return:
 *		RT_ERR_PORT_ID		-	invalid port id
 *		RT_ERR_INPUT		-	invalid input parameter
 *		RT_ERR_NULL_POINTER		-	input parameter is null pointer
 *		RT_ERR_FAILED		-	failure
 *		RT_ERR_OK			-	success
 */
extern rtk_api_ret_t rtk_storm_filterStatus_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, uint32 *pExceed);

/* Function Name:
 *		rtk_mib_get
 * Description:
 *		Get mib counter value
 * Input:
 *		port	-	port id
 *		counter	-	mib counter type
 * Output:
 *		pValue	-	pointer point to mib counter value
 * Return:
 *		RT_ERR_PORT_ID		-	invalid port id
 *		RT_ERR_INPUT		-	invalid input parameter
 *		RT_ERR_NULL_POINTER		-	input parameter is null pointer
 *		RT_ERR_FAILED		-	failure
 *		RT_ERR_OK			-	success
 *Note:
 *		mib counter named MIB_TXBYTECNT and MIB_RXBYTECNT are counted by unit of byte. 
 *		And the counter values are 64bits long. So when these mib counter value are needed to 
 *		read out,  pValue should be pointed to a array with 2 unsigned 32bits data elements.
 *		To read out other mib counter, the unit is packet and pValue is pointed to a unsigned 32
 *		bits value.
 */
extern rtk_api_ret_t rtk_mib_get(rtk_port_t port, rtk_mib_counter_t counter, rtk_mib_cntValue_t *pValue);

/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *      Reset per port MIB counter by port, and enable mib counter start to count.
 * Input:
 *      port 	- 	port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   					-	success           
 *      RT_ERR_PORT_ID					-	invalid port id          
 *	  RT_ERR_STAT_PORT_CNTR_FAIL 	- 	Could not retrieve/reset Port Counter
 * Note:
 *	  This API can be called to enable mib counter, and reset port's mib counter to run.
 */
extern rtk_api_ret_t rtk_stat_port_reset(rtk_port_t port);

/* Function Name:
 *      rtk_eee_portEnable_set
 * Description:
 *      Set enable status of EEE function.
 * Input:
 *      port - port id.
 *      enable - enable EEE status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
extern rtk_api_ret_t rtk_eee_portEnable_set(rtk_port_t port, rtk_enable_t enable);
/* Function Name:
 *		rtk_eee_portEnable_get
 * Description:
 *		Get enable status of EEE function
 * Input:
 *		port - Port id.
 * Output:
 *		pEnable - Back pressure status.
 * Return:
 *		RT_ERR_OK			   - OK
 *		RT_ERR_FAILED		   - Failed
 *		RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *		This API can get EEE function to the specific port.
 *		The configuration of the port is as following:
 *		- DISABLE
 *		- ENABLE
 */
extern rtk_api_ret_t rtk_eee_portEnable_get(rtk_port_t port, rtk_enable_t *pEnable);


extern rtk_api_ret_t rtk_port_phyMdx_set(rtk_port_t port, rtk_port_phy_mdix_mode_t mode);

rtk_api_ret_t rtk_port_phyMdx_get(rtk_port_t port, rtk_port_phy_mdix_mode_t* pmode);

extern rtk_api_ret_t rtk_port_phyMdxStatus_get(rtk_port_t port, rtk_port_phy_mdix_status_t *pStatus);

extern rtk_api_ret_t rtk_port_adminEnable_set(rtk_port_t port, rtk_enable_t enable);

extern rtk_api_ret_t rtk_port_adminEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

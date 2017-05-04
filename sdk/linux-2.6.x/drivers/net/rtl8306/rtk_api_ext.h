/*
* Copyright (C) 2010 Realtek Semiconductor Corp.
* All Rights Reserved.
*
* This program is the proprietary software of Realtek Semiconductor
* Corporation and/or its licensors, and only be used, duplicated,
* modified or distributed under the authorized license from Realtek.
*
* ANY USE OF THE SOFTWARE OTEHR THAN AS AUTHORIZED UNDER 
* THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
* 
* $Revision: 1.1 $ 
* $Date: 2011/03/03 08:28:10 $
*
* Purpose : Realtek common API function list supported by RTL8306E switch, 
*               they could also be applied to chips 
*               RTL8306M/RTL8305N/RTL8303/RTL8304E/RTL8306N
* 
* Feature : This file consists of following modules:
*              (1)   Port Configuration              
*              (2)   Rate Limit
*              (3)   QOS
*              (4)   TRAP
*              (5)   VLAN
*              (6)   Spanning Tree
*              (7)   Address Table
*              (8)   CPU Port
*              (9)   Mirror
*              (10)  IEEE 802.1x
*              (11)  SVLAN
*/


#ifndef __RTK_API_EXT_H__
#define __RTK_API_EXT_H__
#if 0 //shlee, one armed router
#include <rtl8306e_types.h>
#include <rtk_api.h>
#else
#include "Rtl8306_types.h"
#include "rtk_api.h"

#endif
/*
 * Function Declaration
 */
 
/*Misc*/

/* Function Name:
 *      rtk_switch_init
 * Description:
 *      Set chip to default configuration enviroment
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED               -  Failure
 * Note:
 *     The API can set chip registers to default configuration for
 *     different release chip model.
 */ 
extern rtk_api_ret_t rtk_switch_init(void);

/* Function Name:
 *      rtk_switch_maxPktLen_set
 * Description:
 *      Set the max packet length of the specific unit
 * Input:
 *      len       -       max packet length
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                    -  Success
 *      RT_ERR_FAILED               -  Failure
 *      RT_ERR_INPUT                -  Invalid input parameter
 * Note:
 *      The API can set max packet length of the specific unit to
 *          MAXPKTLEN_1522B,
 *          MAXPKTLEN_1536B,
 *          MAXPKTLEN_1552B,
 *          MAXPKTLEN_2000B, 
 */ 
extern rtk_api_ret_t rtk_switch_maxPktLen_set(rtk_switch_maxPktLen_t len); 

/* Function Name:
 *      rtk_switch_maxPktLen_get
 * Description:
 *      Get the max packet length of the specific unit
 * Input:
 *      none
 * Output:
 *      pLen                             -    the pointer of max packet length
 * Return: 
 *      RT_ERR_OK                    -  Success
 *      RT_ERR_FAILED               -  Failure
 *      RT_ERR_NULL_POINTER     -  Input parameter is null pointer 
 * Note:
 *      The API can set max packet length of the specific unit to
 *          MAXPKTLEN_1522B,
 *          MAXPKTLEN_1536B,
 *          MAXPKTLEN_1552B,
 *          MAXPKTLEN_2000B, 
 */
 extern rtk_api_ret_t rtk_switch_maxPktLen_get(rtk_switch_maxPktLen_t *pLen);

/* Function Name:
 *      rtk_port_phyReg_set
 * Description:
 *      Set PHY register data of the specific port
 * Input:
 *      phy                - phy number, 0 ~ 6 
 *      reg                 - Register id
 *      regData           - Register data
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK         -  Success
 *      RT_ERR_FAILED   -   Failure
 * Note:
 *      This API can set PHY register data of the specific port.
 *      RTL8306E switch has 5 FE port, and port 4 could be set as
 *      phy mode or mac mode, port 5 is mac mode which connect
 *      with mii interface. so parameter phy 0 ~4 means port 0 ~4
 *      phy register, 5 means port 4 mac mode, 6 means port 5.
 *      
 */
extern rtk_api_ret_t rtk_port_phyReg_set(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t value);

/* Function Name:
 *      rtk_port_phyReg_get
 * Description:
 *      Get PHY register data of the specific port
 * Input:
 *      phy                  - phy number, 0 ~ 6
 *      reg                   - Register id
 * Output:
 *      pData               - the pointer of Register data
 * Return:
 *      RT_ERR_OK        -  Success
 *      RT_ERR_FAILED   -  Failure
 * Note:
 *      This API can set PHY register data of the specific port.
 *      RTL8306E switch has 5 FE port, and port 4 could be set as
 *      phy mode or mac mode, port 5 is mac mode which connect
 *      with mii interface. so parameter phy 0 ~4 means port 0 ~4
 *      phy register, 5 means port 4 mac mode, 6 means port 5.
 *      
 */
extern rtk_api_ret_t rtk_port_phyReg_get(rtk_port_t port, rtk_port_phy_reg_t reg, rtk_port_phy_data_t *pData); 

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_set
 * Description:
 *      Set ethernet PHY auto-negotiation desired ability
 * Input:
 *      port       -  Port id
 *      pAbility   -  Ability structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      (1) RTL8306E switch only has 5 phy, so the input port id should be 0~4.
 *      (2) In auto-negotiation mode, phy autoNegotiation ability must be enabled
 */ 
extern rtk_api_ret_t rtk_port_phyAutoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyAutoNegoAbility_get
 * Description:
 *      Get ethernet PHY auto-negotiation desired ability
 * Input:
 *      port       -  Port id
 * Output:
 *      pAbility   -  Ability structure
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_PHY_AUTO_NEGO_MODE
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      (1) RTL8306E switch only has 5 phy, so the input port id should be 0~4.
 *      (2) In auto-negotiation mode, phy autoNegotiation ability must be enabled
 */  
extern rtk_api_ret_t rtk_port_phyAutoNegoAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *     rtk_port_phyForceModeAbility_set
 * Description:
 *      Set the port PHY force mode, config its speed/duplex mode/pause/asy_pause 
 * Input:
 *      port       -  Port id
 *      pAbility   -  Ability structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_PHY_FORCE_1000
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      (1) RTL8306E switch only has 5 phy, so the input port id should be 0~4.
 *      (2) In force mode,  phy autoNegotiation ability must be disabled.
 */
extern rtk_api_ret_t rtk_port_phyForceModeAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyForceModeAbility_get
 * Description:
 *      Get the port PHY speed/duplex mode/pause/asy_pause in force mode
 * Input:
 *      port       -  Port id
 * Output:
 *      pAbility   -  Ability structure
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_PHY_REG_ID
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_BUSYWAIT_TIMEOUT
 *     
 * Note:
 *      (1) RTL8306E switch only has 5 phy, so the input port id should be 0~4.
 *      (2) In force mode, phy autoNegotiation ability must be disabled.
 */
extern rtk_api_ret_t rtk_port_phyForceModeAbility_get(rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      rtk_port_phyStatus_get
 * Description:
 *      Get ethernet PHY linking status
 * Input:
 *      port             -  Port id
 * Output:
 *      pLinkStatus   -  the pointer of PHY link status 
 *      pSpeed         -  the pointer of PHY link speed
 *      pDuplex        -  the pointer of PHY duplex 
 * Return:
 *      RT_ERR_OK                        
 *      RT_ERR_FAILED                  
 *      RT_ERR_SMI       
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER
 *     
 * Note:
 *      RTL8306E switch only has 5 phy, so the input port id should be 0~4.
 */
extern rtk_api_ret_t rtk_port_phyStatus_get(rtk_port_t port, rtk_port_linkStatus_t *pLinkStatus, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex);

/* Function Name:
 *      rtk_port_macForceLinkExt0_set
 * Description:
 *      Set external interface 0(MAC 5) force linking configuration.
 * Input:
 *      mode - external interface mode
 *      pPortability - port ability configuration
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure
 *      RT_ERR_INPUT       - Invalid input parameters.
 * Note:
 *      (1) This API can set external interface 0(MAC 5) force mode properties,
 *      including link status,speed,duplex,and tx pause and tx pause ability.
 *      In force mode, autoNegotiation ability must be disabled.
 *      (2) For 8306E, the external interface 0(MAC 5) operating mode can be
 *      MAC Mode (T)MII, PHY Mode (T)MII and RMII. The operating mode is 
 *      determined by strapping pin upon reset, and can not be configured 
 *      by software, except the selection of MII or TMII. 
 *      (3)The input parament mode here is only used to select MII or TMII.
 *      When operating mode is configured as MAC Mode (T)MII or PHY Mode (T)MII
 *      via strapping pin, then the selection of MII or TMII can be done via software.
 *      For example, set mode MODE_EXT_TMII_MAC to select TMII, and set mode
 *      MODE_EXT_MII_MAC to select MII.
 */
extern rtk_api_ret_t rtk_port_macForceLinkExt0_set(rtk_mode_ext_t mode, rtk_port_mac_ability_t *pPortability);

/* Function Name:
 *      rtk_port_macForceLinkExt0_get
 * Description:
 *      Get external interface 0(MAC 5) force linking configuration.
 * Input:
 *      None
 * Output:
 *      pMode - external interface mode
 *      pPortability - port ability configuration
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure 
 *      RT_ERR_INPUT - Invalid input parameters.
 * Note:
 *      This API can get external interface 0 (MAC 5) force mode properties.
 *      The external interface 0 operating mode can be:
 *      - MODE_EXT_MII_MAC,
 *      - MODE_EXT_MII_PHY, 
 *      - MODE_EXT_TMII_MAC,
 *      - MODE_EXT_TMII_PHY, 
 *      - MODE_EXT_RMII, 
 */
extern rtk_api_ret_t rtk_port_macForceLinkExt0_get(rtk_mode_ext_t *pMode, rtk_port_mac_ability_t *pPortability);

/* Function Name:
 *      rtk_port_isolation_set
 * Description:
 *      Set permitted port isolation portmask
 * Input:
 *      port                - port id, 0 ~ 5 
 *      portmask         - Permit port mask
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK             -   Success
 *      RT_ERR_PORT_ID     -   Invalid port number
 *      RT_ERR_PORT_MASK -   Invalid portmask
 *      RT_ERR_FAILED        -   Failure
 * Note:
 *      This API set the port mask that a port can trasmit packet to of each port.
 *      A port can only transmit packet to ports included in permitted portmask  
 */
extern rtk_api_ret_t rtk_port_isolation_set(rtk_port_t port, rtk_portmask_t portmask);

/* Function Name:
 *      rtk_port_isolation_get
 * Description:
 *      Get permitted port isolation portmask
 * Input:
 *      port                - port id, 0 ~ 5 
 * Output:
 *      pPortmask       - the pointer of permit port mask
 * Return:
 *      RT_ERR_OK                  -   Success
 *      RT_ERR_PORT_ID          -   Invalid port number
 *      RT_ERR_NULL_POINTER  -   Input parameter is null pointer
 *      RT_ERR_FAILED             -   Failure
 * Note:
 *      This API get the port mask that a port can trasmit packet to of each port.
 *      A port can only transmit packet to ports included in permitted portmask  
 */
 
extern rtk_api_ret_t rtk_port_isolation_get(rtk_port_t port, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *      Reset per port MIB counter by port.
 * Input:
 *      port - port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - set shared meter successfully
 *      RT_ERR_FAILED          - FAILED to iset shared meter
 * Note:
 */
extern rtk_api_ret_t rtk_stat_port_reset(rtk_port_t port);


/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set port ingress bandwidth control
 * Input:
 *      port            -  Port id
 *      rate            -  Rate of share meter
 *      ifg_include   -  Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK              -  Success
 *      RT_ERR_PORT_ID      -  Invalid port number
 *      RT_ERR_FAILED        -  Failure
 *      RT_ERR_ENABLE       -  Invalid IFG parameter
 *      RT_ERR_INBW_RATE -  Invalid ingress rate parameter
 * Note:
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps. The granularity of rate is 64Kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */ 
extern rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_set( rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include);

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control
 * Input:
 *      port             -  Port id
 * Output:
 *      pRate           -  the pointer of rate of share meter
 *      pIfg_include   -  Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_PORT_ID              -  Invalid port number
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_ENABLE               -  Invalid IFG parameter
 *      RT_ERR_NULL_POINTER      -  null pointer
 * Note:
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps. The granularity of rate is 64Kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */
extern rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set port egress bandwidth control
 * Input:
 *      port            -  Port id
 *      rate            -  Rate of bandwidth control
 *      ifg_include   -  Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                   -  Success
 *      RT_ERR_PORT_ID           -  Invalid port number
 *      RT_ERR_FAILED             -  Failure
 *      RT_ERR_ENABLE             -  Invalid IFG parameter
 *      RT_ERR_QOS_EBW_RATE -  Invalid egress rate parameter
 * Note:
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps. The granularity of rate is 64Kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */
extern rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include);

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control
 * Input:
 *      port             -  Port id
 * Output:
 *      pRate           -  the pointer of rate of bandwidth control
 *      pIfg_include   -  Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_PORT_ID             -  Invalid port number
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_NULL_POINTER      -  null pointer
 * Note:
 *      The rate unit is 64Kbps and the range is from 64Kbps to 100Mbps. The granularity of rate is 64Kbps. 
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble. 
 */
extern rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include);

/* QoS */

/* Function Name:
 *      rtk_qos_init
 * Description:
 *      Configure Qos default settings with queue number assigment to each port
 * Input:
 *      queueNum     -  Queue number of each port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_QUEUE_NUM         -  Error queue number
 * Note:
 *    This API will initialize related Qos setting with queue number assigment.
 *    The queue number is from 1 to 4.
 */ 
extern rtk_api_ret_t rtk_qos_init(rtk_queue_num_t queueNum);

/* Function Name:
 *      rtk_qos_1pPriRemap_set
 * Description:
 *      Configure 1Q priorities mapping to internal absolute priority
 * Input:
 *      dot1p_pri   -  802.1p priority value
 *      int_pri       -  internal priority value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_VLAN_PRIORITY        -  Invalid 1p priority
 *      RT_ERR_QOS_INT_PRIORITY   -  Invalid priority
 * Note:
 *      Priority of 802.1Q assignment for internal asic priority, and it is used for queue usage 
 *      and packet scheduling.
 */ 
extern rtk_api_ret_t rtk_qos_1pPriRemap_set(rtk_pri_t dot1p_pri, rtk_pri_t int_pri);

/* Function Name:
 *      rtk_qos_priSel_set
 * Description:
 *      Configure the priority order among different priority mechanisms.
 * Input:
 *      pPriDec - priority level for port, dscp, 802.1Q, ACL and VID based priority decision.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - success
 *      RT_ERR_NULL_POINTER  -   Input parameter is null pointer
 *      RT_ERR_FAILED          - failure 
 *      RT_ERR_QOS_SEL_PRI_SOURCE - Invalid priority decision source parameter.
 * Note: 
 *      (1)For 8306E, there are 5 types of priority mechanisms that could be set arbitration level, which are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority, Port-based priority, VID-based priority.
 *      Each one could be set level from 1 to 5.
 *      (2)ASIC will follow user's arbitration level setting of priority mechanisms to select internal priority for receiving frame. 
 *      If two priority mechanisms are the same level, the ASIC will chose the priority mechanism with highest level to 
 *      assign internal priority to receiving frame.
 */
extern rtk_api_ret_t rtk_qos_priSel_set(rtk_priority_select_t *pPriDec);

/* Function Name:
 *      rtk_qos_priSel_get
 * Description:
 *      Get the priority order configuration among different priority mechanism.
 * Input:
 *      None
 * Output:
 *      pPriDec - priority level for port, dscp, 802.1Q, ACL and VID based priority decision.
 * Return:
 *      RT_ERR_OK              - success
 *      RT_ERR_NULL_POINTER  -   Input parameter is null pointer
 *      RT_ERR_FAILED          - failure 
 * Note:
 *      (1)For 8306E, there are 5 types of priority mechanisms that could be set arbitration level, which are 
 *      ACL-based  priority, DSCP-based priority, 1Q-based priority,Port-based priority, VID-based priority.
 *      Each one could be set level from 1 to 5.
 *      (2)ASIC will follow user's arbitration level setting of priority mechanisms to select internal priority for receiving frame. 
 *      If two priority mechanisms are the same level, the ASIC will chose the priority mechanism with the highest level to 
 *      assign internal priority to receiving frame.
 */
extern rtk_api_ret_t rtk_qos_priSel_get(rtk_priority_select_t *pPriDec);

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
 *      RT_ERR_NULL_POINTER         -   null pointer
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
 *      code      -  dscp code
 *      int_pri    -  internal priority value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_QOS_INT_PRIORITY   -  invalid internal priority
 *      RT_ERR_QOS_DSCP_VALUE    -   invalid DSCP value  
 * Note:
 *      switch support 16 kinds of dscp code: <nl>
 *          RTL8306_DSCP_EF          
 *                 - DSCP for the Expedited forwarding PHB, 101110  <nl>   
 *          RTL8306_DSCP_AFL1         
 *                 - DSCP for AF PHB Class 1 low drop, 001010 <nl>
 *          RTL8306_DSCP_AFM1     
 *                 - DSCP for AF PHB Class 1 medium drop, 001100 <nl>
 *          RTL8306_DSCP_AFH1      
 *                 - DSCP for AF PHB Class 1 high drop, 001110 <nl>
 *          RTL8306_DSCP_AFL2       
 *                 - DSCP for AF PHB Class 2 low drop, 01001 <nl>
 *          RTL8306_DSCP_AFM2       
 *                 - DSCP for AF PHB Class 2 medium drop, 010100 <nl>
 *          RTL8306_DSCP_AFH2   
 *                 - DSCP for AF PHB Class 2 high drop, 010110 <nl>
 *          RTL8306_DSCP_AFL3    
 *                 - DSCP for AF PHB Class 3 low drop, 011010 <nl>
 *          RTL8306_DSCP_AFM3      
 *                 - DSCP for AF PHB Class 3 medium drop, 011100 <nl>
 *          RTL8306_DSCP_AFH3    
 *                 - DSCP for AF PHB Class 3 high drop, 0111 <nl>
 *          RTL8306_DSCP_AFL4     
 *                 - DSCP for AF PHB Class 4 low drop, 100010 <nl>
 *          RTL8306_DSCP_AFM4    
 *                 - DSCP for AF PHB Class 4 medium drop, 100100 <nl>
 *          RTL8306_DSCP_AFH4     
 *                 - DSCP for AF PHB Class 4 high drop, 100110 <nl>
 *          RTL8306_DSCP_NC        
 *                 - DSCP for network control, 110000 or 111000 <nl>
 *          RTL8306_DSCP_REG_PRI 
 *                 - DSCP Register match priority, user could define two dscp code <nl>
 *          RTL8306_DSCP_BF        
 *                 - DSCP Default PHB <nl>
 *
 *     The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically 
 *     greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of 
 *     DSCP are carefully chosen then backward compatibility can be achieved.         
 */ 
extern rtk_api_ret_t rtk_qos_dscpPriRemap_set(rtk_dscp_t dscp, rtk_pri_t int_pri);

/* Function Name:
 *      rtk_qos_dscpPriRemap_get
 * Description:
 *      Get DSCP-based priority
 * Input:
 *      dscp      -  dscp code
 * Output:
 *      pInt_pri  -  the pointer of internal priority value
 * Return:
 *      RT_ERR_OK                           -  Success
 *      RT_ERR_FAILED                     -  Failure
 *      RT_ERR_QOS_DSCP_VALUE      -  Invalid DSCP value
 *      RT_ERR_NULL_POINTER           -  Input parameter is null pointer
 * Note:
 *      switch support 16 kinds of dscp code: <nl>
 *          RTL8306_DSCP_EF          
 *                 - DSCP for the Expedited forwarding PHB, 101110  <nl>   
 *          RTL8306_DSCP_AFL1         
 *                 - DSCP for AF PHB Class 1 low drop, 001010 <nl>
 *          RTL8306_DSCP_AFM1     
 *                 - DSCP for AF PHB Class 1 medium drop, 001100 <nl>
 *          RTL8306_DSCP_AFH1      
 *                 - DSCP for AF PHB Class 1 high drop, 001110 <nl>
 *          RTL8306_DSCP_AFL2       
 *                 - DSCP for AF PHB Class 2 low drop, 01001 <nl>
 *          RTL8306_DSCP_AFM2       
 *                 - DSCP for AF PHB Class 2 medium drop, 010100 <nl>
 *          RTL8306_DSCP_AFH2   
 *                 - DSCP for AF PHB Class 2 high drop, 010110 <nl>
 *          RTL8306_DSCP_AFL3    
 *                 - DSCP for AF PHB Class 3 low drop, 011010 <nl>
 *          RTL8306_DSCP_AFM3      
 *                 - DSCP for AF PHB Class 3 medium drop, 011100 <nl>
 *          RTL8306_DSCP_AFH3    
 *                 - DSCP for AF PHB Class 3 high drop, 0111 <nl>
 *          RTL8306_DSCP_AFL4     
 *                 - DSCP for AF PHB Class 4 low drop, 100010 <nl>
 *          RTL8306_DSCP_AFM4    
 *                 - DSCP for AF PHB Class 4 medium drop, 100100 <nl>
 *          RTL8306_DSCP_AFH4     
 *                 - DSCP for AF PHB Class 4 high drop, 100110 <nl>
 *          RTL8306_DSCP_NC        
 *                 - DSCP for network control, 110000 or 111000 <nl>
 *          RTL8306_DSCP_REG_PRI 
 *                 - DSCP Register match priority, user could define two dscp code <nl>
 *          RTL8306_DSCP_BF        
 *                 - DSCP Default PHB <nl>
 *
 *     The Differentiated Service Code Point is a selector for router's per-hop behaviors. As a selector, there is no implication that a numerically 
 *     greater DSCP implies a better network service. As can be seen, the DSCP totally overlaps the old precedence field of TOS. So if values of 
 *     DSCP are carefully chosen then backward compatibility can be achieved.         
 */ 
extern rtk_api_ret_t rtk_qos_dscpPriRemap_get(rtk_dscp_t dscp, rtk_pri_t *pInt_pri);

/* Function Name:
 *      rtk_qos_portPri_set
 * Description:
 *      Configure priority usage to each port
 * Input:
 *      port                - Port id.                
 *      int_pri             -  internal priority value
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *      RT_ERR_PORT_ID                -   Error port id
 *      RT_ERR_QOS_INT_PRIORITY  -   Error internal priority value
 * Note:
 *     The API can set priority of port assignments for queue usage and packet scheduling.
 */
 
extern rtk_api_ret_t rtk_qos_portPri_set(rtk_port_t port, rtk_pri_t int_pri) ;

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
 *      RT_ERR_PORT_ID                -   Error port id
 *      RT_ERR_NULL_POINTER        -   null pointer
 * Note:
 *      The API can get priority of port assignments for queue usage and packet scheduling.
 */
 
extern rtk_api_ret_t rtk_qos_portPri_get(rtk_port_t port, rtk_pri_t *pInt_pri) ;

/* Function Name:
 *      rtk_qos_priMap_set
 * Description:
 *      Set internal priority mapping to queue ID for different queue number
 * Input:
 *      queue_num       - Queue number usage
 *      pPri2qid            - Priority mapping to queue ID               
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *      RT_ERR_QUEUE_ID              -   Error queue id
 *      RT_ERR_NULL_POINTER        -   null pointer
 * Note:
 *      ASIC supports priority mapping to queue with different queue number from 1 to 4.
 *      For different queue numbers usage, ASIC supports different internal available queue IDs.
 */

extern rtk_api_ret_t rtk_qos_priMap_set(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid);

/* Function Name:
 *      rtk_qos_priMap_get
 * Description:
 *      Get priority to queue ID mapping table parameters
 * Input:
 *      queue_num       - Queue number usage
 *      pPri2qid            - Priority mapping to queue ID               
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 *      RT_ERR_QUEUE_ID              -   Error queue id
 *      RT_ERR_NULL_POINTER        -   null pointer
 * Note:
 *      ASIC supports priority mapping to queue with different queue number from 1 to 4.
 *      For different queue numbers usage, ASIC supports different internal available queue IDs.
 */
 
extern rtk_api_ret_t rtk_qos_priMap_get(rtk_queue_num_t queue_num, rtk_qos_pri2queue_t *pPri2qid);

/* Function Name:
 *      rtk_qos_1pRemarkEnable_set
 * Description:
 *      Set 802.1P remarking ability
 * Input:
 *      port       -  port number (0~5)
 *      enabled  -  TRUE or FALSE
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK              -  Success
 *      RT_ERR_FAILED        -   Failure
 *      RT_ERR_PORT_ID      -   Invalid port id
 * Note:
 *      
 */
extern rtk_api_ret_t rtk_qos_1pRemarkEnable_set(rtk_port_t port, rtk_enable_t enable); 

/* Function Name:
 *      rtk_qos_1pRemarkEnable_get
 * Description:
 *      Get 802.1P remarking ability
 * Input:
 *      port        -  port number (0~5)
 * Output:
 *      pEnabled  -  pointer of the ability status
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED               -   Failure
 *      RT_ERR_PORT_ID             -   Invalid port id
 *      RT_ERR_NULL_POINTER     -   Input parameter is null pointer
 * Note:
 *      
 */
extern rtk_api_ret_t rtk_qos_1pRemarkEnable_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_qos_1pRemark_set
 * Description:
 *      Set 802.1P remarking priority
 * Input:
 *      int_pri        -  Packet priority(0~4)
 *      dot1p_pri    -  802.1P priority(0~7)
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                           - Success
 *      RT_ERR_FAILED                      - Failure
 *      RT_ERR_VLAN_PRIORITY          - Invalid 1p priority
 *      RT_ERR_QOS_INT_PRIORITY     - Invalid internal priority 
 * Note:
 *      switch determines packet priority, the priority souce could
 *      be port-based, 1Q-based, dscp-based, vid-based, ip address,
 *      cpu tag.
 */ 
extern rtk_api_ret_t rtk_qos_1pRemark_set(rtk_pri_t int_pri, rtk_pri_t dot1p_pri);

/* Function Name:
 *      rtk_qos_1pRemark_get
 * Description:
 *      Get 802.1P remarking priority
 * Input:
 *      int_pri        -  Packet priority(0~4)
 * Output:
 *      pDot1p_pri  -  the pointer of 802.1P priority(0~7)
 * Return:
 *      RT_ERR_OK                           -  Success
 *      RT_ERR_FAILED                      -  Failure
 *      RT_ERR_NULL_POINTER            -  Input parameter is null pointer
 *      RT_ERR_QOS_INT_PRIORITY     -  Invalid internal priority 
 * Note:
 *      switch determines packet priority, the priority souce could
 *      be port-based, 1Q-based, dscp-based, vid-based, ip address,
 *      cpu tag.
 */
extern rtk_api_ret_t rtk_qos_1pRemark_get(rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri);

/* Trap & Reserved Multicast Address (More Action like leaky, bypass storm not define) */

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
 *      When receives an unknown multicast packet, switch may trap, drop this packet
 *      The unknown multicast packet type is as following:
 *               - MCAST_IPV4
 *               - MCAST_IPV6 <nl>
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
 *      RT_ERR_NULL_POINTER      -  Input parameter is null pointer
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop this packet.
 *      The unknown multicast packet type is as following:
 *               - MCAST_IPV4
 *               - MCAST_IPV6 <nl>
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
 *      igmp_action      -   IGMP/MLD action
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_INPUT                 -  Invalid input parameter
 *      RT_ERR_NOT_ALLOWED     -  Actions not allowed by the function
 * Note:
 *      This API can set both IPv4 IGMP/IPv6 MLD with/without PPPoE header trapping function.
 *      All 4 kinds of IGMP/MLD function can be set separately.<nl>
 *      The IGMP/MLD packet type is as following:
 *          - IGMP_IPV4
 *          - IGMP_MLD
 *          - IGMP_PPPOE_IPV4
 *          - IGMP_PPPOE_MLD <nl>
 *      The IGMP/MLD action is as following:
 *          - IGMP_ACTION_FORWARD
 *          - IGMP_ACTION_TRAP2CPU
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
 *      All 4 kinds of IGMP/MLD function can be set separately.<nl>
 *      The IGMP/MLD packet type is as following:
 *          - IGMP_IPV4
 *          - IGMP_MLD
 *          - IGMP_PPPOE_IPV4
 *          - IGMP_PPPOE_MLD <nl>
 *      The IGMP/MLD action is as following:
 *          - IGMP_ACTION_FORWARD
 *          - IGMP_ACTION_TRAP2CPU
 */
extern rtk_api_ret_t rtk_trap_igmpCtrlPktAction_get(rtk_igmp_type_t type, rtk_trap_igmp_action_t *pIgmp_action);


/* CVLAN */

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
 * Note:
 *      VLAN is disabled by default. User has to call this API to enable VLAN before
 *      using it. And It will set a default VLAN(vid 1) including all ports and set 
 *      all ports PVID to the default VLAN.
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
 *      fid              -  filtering database id, could be any value for RTL8306E
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                     -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_VLAN_VID            -  Invalid vid
 *      RT_ERR_INPUT                 -  Invalid input parameter 
 *      RT_ERR_TBL_FULL            -  Input table full 
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     FID is for SVL/IVL usage, and the range is 0~4095, rtl8306E only supports SVL, 
 *     so fid is no useage.
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
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND    -   Specified vlan entry not found
 * Note:
 *     There are 16 VLAN entry supported. User could configure the member set and untag set
 *     for specified vid through this API. The portmask's bit N means port N.
 *     For example, mbrmask 23=0x17=010111 means port 0,1,2,4 in the member set.
 *     FID is for SVL/IVL usage, and the range is 0~4095, rtl8306E only supports SVL, 
 *     so fid is no useage.
 */
extern rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_portmask_t *pMbrmsk, rtk_portmask_t *pUntagmsk, rtk_fid_t *pFid);

/* Function Name:
 *      rtk_vlan_destroy
 * Description:
 *      delete vid from vlan table
 * Input:
 *      vid             - VLAN ID to configure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                   -  Success
 *      RT_ERR_FAILED                              -  Failure
 *      RT_ERR_VLAN_VID                          -  Invalid vid
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND    -  Specified vlan entry not found
 * Note:
 * 
 */ 
extern rtk_api_ret_t rtk_vlan_destroy(rtk_vlan_t vid);

/* Function Name:
 *      rtk_vlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID)
 * Input:
 *      port             - Port id
 *      pvid             - Specified VLAN ID
 *      priority         - 802.1p priority for the PVID, 0~3 for RTL8306E 
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                   -  Success
 *      RT_ERR_FAILED                              -  Failure
 *      RT_ERR_VLAN_VID                          -  Invalid vid
 *      RT_ERR_VLAN_PRIORITY                  -  Invalid 1p priority 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND    -  Specified vlan entry not found
 * Note:
 *      The API is used for Port-based VLAN. The untagged frame received from the
 *      port will be classified to the specified VLAN and assigned to the specified priority.
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
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port
 * Input:
 *      port             - Port id
 *      igr_filter        - VLAN ingress function enable status
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        - Success
 *      RT_ERR_FAILED                  -  Failure
 * Note:
 *      RTL8306E use one ingress filter for whole system, not for each port, so 
 *      any port you set will affect all ports ingress filter setting.
 *      While VLAN function is enabled, ASIC will decide VLAN ID for each received frame 
 *      and get belonged member ports from VLAN table. If received port is not belonged 
 *      to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      get VLAN ingress for each port
 * Input:
 *      port             - Port id
 * Output:
 *      pIgr_filter     - the pointer of VLAN ingress function enable status
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_FAILED           -  Failure
 * Note:
 *      RTL8306E use one ingress filter for whole system, not for each port, so 
 *      any port you set will affect all ports ingress filter setting.
 *      While VLAN function is enabled, ASIC will decide VLAN ID for each received frame 
 *      and get belonged member ports from VLAN table. If received port is not belonged 
 *      to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN support frame type
 * Input:
 *      port                                 - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                  - Success
 *      RT_ERR_FAILED            -  Failure
 *      RT_ERR_PORT_ID          -  Invalid port id
 * Note:
 *      The API is used for checking 802.1Q tagged frames.
 *      The accept frame type as following:
 *          - ACCEPT_FRAME_TYPE_ALL
 *          - ACCEPT_FRAME_TYPE_TAG_ONLY
 *          - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN support frame type
 * Input:
 *      port                                 - Port id
 *      accept_frame_type             - accept frame type
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                   - Success
 *      RT_ERR_FAILED                             -  Failure
 *      RT_ERR_PORT_ID                           -  Invalid port id
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE -  Invalid accept frame type 
 * Note:
 *    The API is used for checking 802.1Q tagged frames.
 *    The accept frame type as following:
 *    - ACCEPT_FRAME_TYPE_ALL
 *    - ACCEPT_FRAME_TYPE_TAG_ONLY
 *    - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      rtk_vlan_vlanBasedPriority_set
 * Description:
 *      Set VLAN priority for each CVLAN
 * Input:
 *      vid                -Specified VLAN ID
 *      priority           -priority for the VID
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_VLAN_VID                       -  Invalid vid 
 *      RT_ERR_VLAN_PRIORITY               -  Invalid 1p priority
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND -  Specified vlan entry not found
 * Note:
 *      This API is used to set priority per VLAN.
 */ 
extern rtk_api_ret_t rtk_vlan_vlanBasedPriority_set(rtk_vlan_t vid, rtk_pri_t priority);

/* Function Name:
 *      rtk_vlan_vlanBasedPriority_get
 * Description:
 *      Get VLAN priority for each CVLAN
 * Input:
 *      vid                -Specified VLAN ID
 * Output:
 *      pPriority         -the pointer of priority for the VID
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_VLAN_VID                       -  Invalid vid 
 *      RT_ERR_NULL_POINTER                -   Input parameter is null pointer
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND -   Specified vlan entry not found 
 * Note:
 *      This API is used to set priority per VLAN.
 */ 
extern rtk_api_ret_t rtk_vlan_vlanBasedPriority_get(rtk_vlan_t vid, rtk_pri_t *pPriority);

/* Function Name:
 *      rtk_vlan_vidTranslation_set
 * Description:
 *      Set vid translated to new vid
 * Input:
 *      vid       -  old vid
 *      nvid     -   new vid
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_VLAN_VID                       -  Invalid vid 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND -   Specified vlan entry not found 
 * Note:
 *      This API is used to translate a vid to a new vid, the new vid could be 
 *      used by Q-in-Q or vlan translation function.
 */
extern rtk_api_ret_t rtk_vlan_vidTranslation_set(rtk_vlan_t vid, rtk_vlan_t nvid);

/* Function Name:
 *      rtk_vlan_vidTranslation_get
 * Description:
 *      Get vid translation new vid
 * Input:
 *      vid        -  old vid
 * Output:
 *      pNvid     -  the pointer of new vid
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_VLAN_VID                       -  Invalid vid 
 *      RT_ERR_NULL_POINTER                -   NULL pointer
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND -   Specified vlan entry not found 
 * Note:
 *      This API is used to translate a vid to a new vid, the new vid could be 
 *      used by Q-in-Q or vlan translation function.
 */
extern rtk_api_ret_t rtk_vlan_vidTranslation_get(rtk_vlan_t vid, rtk_vlan_t *pNvid);


/* Function Name:
 *      rtk_vlan_vidTranslationEnable_set
 * Description:
 *      Set vlan translation function enabled or disabled 
 * Input:
 *      enable        -  enable or disable
 *      nniMask      -   NNI port mask
 * Output:
 *      pNvid     -  the pointer of new vid
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_PORT_MASK                   -   Error port mask
 * Note:
 *      VLAN translation only happens between UNI and NNI port, 
 *      in nniMask, 1 means the port is NNI port, 0 means the port
 *      is UNI port
 */
extern rtk_api_ret_t rtk_vlan_vidTranslationEnable_set(rtk_enable_t enable, rtk_portmask_t nniMask);

/* Function Name:
 *      rtk_vlan_vidTranslationEnable_get
 * Description:
 *      Get vlan translation function enabled or disabled 
 * Input:
 * Output:
 *      pEnable      -   the pointer of enable or disable
 *      pNniMask    -   the pointer of NNI port mask
 * Return:
 *      RT_ERR_OK                                -  Success
 *      RT_ERR_FAILED                          -   Failure
 *      RT_ERR_NULL_POINTER                -   NULL pointer
 * Note:
 *      VLAN translation only happens between UNI and NNI port, 
 *      in nniMask, 1 means the port is NNI port, 0 means the port
 *      is UNI port
 */

extern rtk_api_ret_t rtk_vlan_vidTranslationEnable_get(rtk_enable_t *pEnable, rtk_portmask_t *pNniMask);


/* Function Name:
 *      rtk_vlan_tagSrc_set
 * Description:
 *      Set tag vid and priority source for Q-in-Q and VLAN translation
 * Input:
 *      port          -    port id
 *      vidSrc       -    vid source
 *      priSrc        -    priority source
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                 -  Success
 *      RT_ERR_FAILED            -   Failure
 *      RT_ERR_PORT_ID          -   error port id
 * Note:
 *     Q-in-Q(SVLAN) and VLAN translation will modify tag, 
 *      the API could set outer tag or translated VLAN tag
 *      vid/priority source.<nl>
 *      vidSrc: 
 *                - RTL8306E_VIDSRC_POVID - port-based otag vid,     
 *                - RTL8306E_VIDSRC_NVID   - new vid(translated vid) <nl>
 *       priSrc:
 *                - RTL8306E_PRISRC_PPRI    - port-based otag priority, 
 *                - RTL8306E_PRISRC_1PRMK - 1p remarking priority
 */
extern rtk_api_ret_t rtk_vlan_tagSrc_set(rtk_port_t port, rtk_vidSrc_t vidSrc, rtk_priSrc_t priSrc);

/* Function Name:
 *      rtk_vlan_tagSrc_get
 * Description:
 *      Get tag vid and priority source for Q-in-Q and VLAN translation
 * Input:
 *      port          -    port id
 * Output:
 *      pVidSrc     -    the pointer of vid source
 *      pPriSrc      -    the pointer of priority source
 * Return:
 *      RT_ERR_OK                 -  Success
 *      RT_ERR_FAILED            -   Failure
 *      RT_ERR_PORT_ID          -   error port id
 * Note:
 *     Q-in-Q(SVLAN) and VLAN translation will modify tag, 
 *      the API could set outer tag or translated VLAN tag
 *      vid/priority source.<nl>
 *      vidSrc: 
 *                - RTL8306E_VIDSRC_POVID - port-based otag vid,     
 *                - RTL8306E_VIDSRC_NVID   - new vid(translated vid) <nl>
 *       priSrc:
 *                - RTL8306E_PRISRC_PPRI    - port-based otag priority, 
 *                - RTL8306E_PRISRC_1PRMK - 1p remarking priority
 */
extern rtk_api_ret_t rtk_vlan_tagSrc_get(rtk_port_t port, rtk_vidSrc_t *pVidSrc, rtk_priSrc_t *pPriSrc);    



/*Spanning Tree*/

/* Function Name:
 *      rtk_stp_mstpState_set
 * Description:
 *      Configure spanning tree state per port
 * Input:
 *      msti              - Multiple spanning tree instance, no use for RTL8306E
 *      port              - Port id
 *      stp_state       - Spanning tree state
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID              -  Invalid port id
 *      RT_ERR_MSTP_STATE        -  Invalid spanning tree status
 * Note:
 *      Because RTL8306E does not support multiple spanning tree, so msti is no use. 
 *      There are four states supported by ASIC.
 *          - STP_STATE_DISABLED
 *          - STP_STATE_BLOCKING
 *          - STP_STATE_LEARNING
 *          - STP_STATE_FORWARDING
 */
extern rtk_api_ret_t rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stp_state);

/* Function Name:
 *      rtk_stp_mstpState_get
 * Description:
 *      Get Configuration of spanning tree state per port
 * Input:
 *      msti              - Multiple spanning tree instance, no use for RTL8306E
 *      port              - Port id
 * Output:
 *      pStp_state     - the pointer of Spanning tree state
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -  Failure
 *      RT_ERR_PORT_ID              -  Invalid port id
 *      RT_ERR_NULL_POINTER      -  Input parameter is null pointer
 * Note:
 *      Because RTL8306E does not support multiple spanning tree, so msti is no use. 
 *      There are four states supported by ASIC.
 *          - STP_STATE_DISABLED
 *          - STP_STATE_BLOCKING
 *          - STP_STATE_LEARNING
 *          - STP_STATE_FORWARDING
 */
extern rtk_api_ret_t rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t *pStp_state);

/* LUT */

/* Function Name:
 *      rtk_l2_addr_add
 * Description:
 *      Set LUT unicast entry
 * Input:
 *      pMac               -   6 bytes unicast(I/G bit is 0) mac address to be written into LUT
 *      pL2_data          -   the mac address attributes
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                       -  Success
 *      RT_ERR_FAILED                 -   Failure
 *      RT_ERR_INPUT                  -   Invalid input parameter
 *      RT_ERR_MAC                    -   Wrong mac address, must be unicast mac        
 *      RT_ERR_NULL_POINTER       -  Input parameter is null pointer    
 *      RT_ERR_L2_INDEXTBL_FULL -  The L2 index table is full
 * Note:
 *      If the unicast mac address already existed in LUT, it will udpate the status of the entry. 
 *      Otherwise, it will find an empty or asic auto learned entry to write. If all the entries 
 *      with the same hash value can't be replaced, ASIC will return a RT_ERR_L2_INDEXTBL_FULL error.
 *      for RTL8306E, pL2_data member fid and sa_block is no use, so it can be chosen any value.
 */
extern rtk_api_ret_t rtk_l2_addr_add(rtk_mac_t *pMac, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      rtk_l2_addr_get
 * Description:
 *      Get LUT unicast entry
 * Input:
 *      pMac               -   6 bytes unicast(I/G bit is 0) mac address to be gotten
 *      fid                   -   filtering database id, could be any value for RTL8306E switch
 * Output:
 *      pL2_data          -   the mac address attributes
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_INPUT                          -   Invalid input parameter
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac        
 *      RT_ERR_NULL_POINTER              -   Input parameter is null pointer    
 *      RT_ERR_L2_ENTRY_NOTFOUND    -   Specified entry not found
 * Note:
 *      If the unicast mac address existed in LUT, it will return the port where
 *      the mac is learned, 802.1x authorized status and dynamic/static entry,
 *      Otherwise, it will return a RT_ERR_L2_ENTRY_NOTFOUND error.
 */
extern rtk_api_ret_t rtk_l2_addr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_l2_ucastAddr_t *pL2_data);

/* Function Name:
 *      rtk_l2_addr_del
 * Description:
 *      Delete LUT unicast entry
 * Input:
 *      pMac               -   6 bytes unicast mac address to be deleted
 *      fid                   -   filtering database id, could be any value for RTL8306E switch
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_INPUT                          -   Invalid input parameter
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac        
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
 *      Add LUT multicast entry
 * Input:
 *      pMac               -   6 bytes unicast mac address to be deleted
 *      fid                   -   filtering database id, could be any value for RTL8306E switch
 *      portmask          -   Port mask to be forwarded to 
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_INPUT                          -   Invalid input parameter
 *      RT_ERR_PORT_MASK                  -   Invalid port mask
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac        
 *      RT_ERR_L2_INDEXTBL_FULL         -   Hashed index is full of entries
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
 *      Get LUT multicast entry
 * Input:
 *      pMac               -   6 bytes multicast(I/G bit is 0) mac address to be gotten
 *      fid                   -   filtering database id, could be any value for RTL8306E switch
 * Output:
 *      pPortmask         -   the pointer of port mask      
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_INPUT                          -   Invalid input parameter
 *      RT_ERR_NULL_POINTER              -   Input parameter is null pointer    
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac        
 *      RT_ERR_L2_INDEXTBL_FULL         -   Hashed index is full of entries 
 * Note:
 *      If the multicast mac address existed in LUT, it will return the port mask where
 *      the packet should be forwarded to, Otherwise, it will return a 
 *      RT_ERR_L2_ENTRY_NOTFOUND error.
 */
extern rtk_api_ret_t rtk_l2_mcastAddr_get(rtk_mac_t *pMac, rtk_fid_t fid, rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_l2_mcastAddr_del
 * Description:
 *      Delete LUT unicast entry
 * Input:
 *      pMac               -   6 bytes multicast(I/G bit is 1) mac address to be gotten
 *      fid                   -   filtering database id, could be any value for RTL8306E switch
 * Output:
*       none
 * Return:
 *      RT_ERR_OK                               -  Success
 *      RT_ERR_FAILED                         -   Failure
 *      RT_ERR_INPUT                          -   Invalid input parameter
 *      RT_ERR_MAC                            -   Wrong mac address, must be unicast mac        
 *      RT_ERR_L2_ENTRY_NOTFOUND     -   No such LUT entry
 * Note:
 *      If the mac has existed in the LUT, it will be deleted.
 *      Otherwise, it will return RT_ERR_L2_ENTRY_NOTFOUND.
 */
extern rtk_api_ret_t rtk_l2_mcastAddr_del(rtk_mac_t *pMac, rtk_fid_t fid);    

/* Function Name:
 *      rtk_l2_limitLearningCnt_set
 * Description:
 *      Set per-Port auto learning limit number
 * Input:
 *      port - Port id.
 *      mac_cnt - Auto learning entries limit number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - set shared meter successfully
 *      RT_ERR_FAILED          - FAILED to iset shared meter
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_LIMITED_L2ENTRY_NUM - Invalid auto learning limit number
 * Note:
 *      (1)The API can set per-port ASIC auto learning limit number from 0(disable learning) 
 *      to 0x1F(31). 
 *      (2)If mac_cnt is set from 0 to 0x1F, per-port ASIC auto learning limit will be enabled;
 *      if mac_cnt is set 0xFF, per-port ASIC auto learning limit will be disabled.
 */
extern rtk_api_ret_t rtk_l2_limitLearningCnt_set(rtk_port_t port, rtk_mac_cnt_t mac_cnt);

/* Function Name:
 *      rtk_l2_limitLearningCnt_get
 * Description:
 *      Get per-Port auto learning limit number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - Auto learning entries limit number
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure 
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      The API can get per-port ASIC auto learning limit number.
 */
extern rtk_api_ret_t rtk_l2_limitLearningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);

/* Function Name:
 *      rtk_l2_limitLearningCntAction_set
 * Description:
 *      Configure auto learn over limit number action.
 * Input:
 *      port - Port id (must be RTK_WHOLE_SYSTEM)
 *      action - Auto learning entries limit number
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
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 *      (2)The action is global, so the port must be set as RTK_WHOLE_SYSTEM
 */
extern rtk_api_ret_t rtk_l2_limitLearningCntAction_set(rtk_port_t port, rtk_l2_limitLearnCntAction_t action);

/* Function Name:
 *      rtk_l2_limitLearningCntAction_get
 * Description:
 *      Get auto learn over limit number action.
 * Input:
 *      port - Port id (must be RTK_WHOLE_SYSTEM)
 * Output:
 *      pAction - Learn over action
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_FAILED          - Failure 
 *      RT_ERR_PORT_ID - Invalid port number. 
 * Note:
 *      (1)The API can get SA unknown packet action while auto learn limit number is over. 
 *      The action symbol as following:
 *      - LIMIT_LEARN_CNT_ACTION_DROP,
 *      - LIMIT_LEARN_CNT_ACTION_TO_CPU,
 *      (2)The action is global, so the port must be set as RTK_WHOLE_SYSTEM
 */
extern rtk_api_ret_t rtk_l2_limitLearningCntAction_get(rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction);

/* Function Name:
 *      rtk_l2_learningCnt_get
 * Description:
 *      Get per-Port current auto learning number
 * Input:
 *      port - Port id.
 * Output:
 *      pMac_cnt - ASIC auto learning entries number
 * Return:
 *      RT_ERR_OK              - Success
 *      RT_ERR_PORT_ID - Invalid port number. 
 *      RT_ERR_NULL_POINTER   -   Input parameter is null pointer  
 * Note:
 *      The API can get per-port ASIC auto learning number
 */
extern rtk_api_ret_t rtk_l2_learningCnt_get(rtk_port_t port, rtk_mac_cnt_t *pMac_cnt);

/* CPU Port */

/* Function Name:
 *      rtk_cpu_enable_set
 * Description:
 *      Set cpu port function enable or disable
 * Input:
 *      enable          - enable or disable
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                        -  Success
 *      RT_ERR_FAILED                  -   Failure
 * Note:
 *      The API can set CPU port function enable/disable
 *      default port 4 is cpu port.
 */
extern rtk_api_ret_t rtk_cpu_enable_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_cpu_enable_get
 * Description:
 *      Get cpu port function enable or disable
 * Input:
 *      none
 * Output:
 *      pEnable          -  the pointer of enable or disable cpu port function
 * Return:
 *      RT_ERR_OK                   -  Success
 *      RT_ERR_FAILED             -   Failure
 * Note:
 *      The API can set CPU port function enable/disable
 */
extern rtk_api_ret_t rtk_cpu_enable_get(rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_cpu_tagPort_set
 * Description:
 *      Set CPU port and CPU tag insert mode
 * Input:
 *      port          -  Port id
 *      mode        -  CPU tag insert for packets egress from CPU port
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                   -  Success
 *      RT_ERR_FAILED             -   Failure
 *      RT_ERR_PORT_ID           -   Invalid port id
 * Note:
 *      The API can set CPU port and inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *          - CPU_INSERT_TO_ALL
 *          - CPU_INSERT_TO_TRAPPING
 *          - CPU_INSERT_TO_NONE   
 */ 
extern rtk_api_ret_t rtk_cpu_tagPort_set(rtk_port_t port, rtk_cpu_insert_t mode);

/* Function Name:
 *      rtk_cpu_tagPort_get
 * Description:
 *      Get CPU port and CPU tag insert mode
 * Input:
 *      port          -  Port id
 * Output:
 *      pMode       -  the pointer of CPU tag insert for packets egress from CPU port
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_NULL_POINTER      -   Input parameter is null pointer
 * Note:
 *      The API can set CPU port and inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *          - CPU_INSERT_TO_ALL
 *          - CPU_INSERT_TO_TRAPPING
 *          - CPU_INSERT_TO_NONE    
 */

extern rtk_api_ret_t rtk_cpu_tagPort_get(rtk_port_t *pPort, rtk_cpu_insert_t *pMode);


/*Port Mirror */

/* Function Name:
 *      rtk_mirror_portBased_set
 * Description:
 *      Set port mirror function
 * Input:
 *      mirroring_port                  - Monitor port, 7 means no monitor port
 *      pMirrored_rx_portmask      - the pointer of Rx mirror port mask
 *      pMirrored_tx_portmask      - the pointer of Tx mirror port mask
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_PORT_MASK         -   Invalid port mask
 * Note:
 *      The API is to set mirror function of source port and mirror port. 
 */
extern rtk_api_ret_t rtk_mirror_portBased_set(rtk_port_t mirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);

/* Function Name:
 *      rtk_mirror_portBased_get
 * Description:
 *      Get port mirror function
 * Input:
 *      none 
 * Output:
 *      pMirroring_port             - the pointer Monitor port, 7 means no monitor port
 *      pMirrored_rx_portmask   - the pointer of Rx mirror port mask
 *      pMirrored_tx_portmask   - the pointer of Tx mirror port mask 
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_PORT_MASK         -   Invalid port mask
 *      RT_ERR_NULL_POINTER      -   Input parameter is null pointer 
 * Note:
 *      The API is to set mirror function of source port and mirror port. 
 */
extern rtk_api_ret_t rtk_mirror_portBased_get(rtk_port_t* pMirroring_port, rtk_portmask_t *pMirrored_rx_portmask, rtk_portmask_t *pMirrored_tx_portmask);

/* 802.1X */

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_set
 * Description:
 *      Set 802.1x unauth action configuration
 * Input:
 *      port                 -   Port id, no use for RTL8306E switch
 *      unauth_action   -   802.1X unauth action    
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_DOT1X_PROC
 * Note:
 *      This API can set 802.1x unauth action configuration, 
 *      for RTL8306E switch, the action is by whole system,
 *      so port could be any value of 0~6.
 *      The unauth action is as following:
 *          - DOT1X_ACTION_DROP
 *          - DOT1X_ACTION_TRAP2CPU
 */

extern rtk_api_ret_t rtk_dot1x_unauthPacketOper_set(rtk_port_t port, rtk_dot1x_unauth_action_t unauth_action);

/* Function Name:
 *      rtk_dot1x_unauthPacketOper_get
 * Description:
 *      Get 802.1x unauth action configuration
 * Input:
 *      port                  -   Port id, no use for RTL8306E switch
 * Output:
 *      pUnauth_action   -  the pointer of 802.1X unauth action    
 * Return:
 *      RT_ERR_OK                      -  Success
 *      RT_ERR_FAILED                -   Failure
 *      RT_ERR_NULL_POINTER      -   Input parameter is null pointer
 * Note:
 *      This API can set 802.1x unauth action configuration, 
 *      for RTL8306E switch, the action is by whole system,
 *      so port could be any value of 0~6.
 *      The unauth action is as following:
 *          - DOT1X_ACTION_DROP
 *          - DOT1X_ACTION_TRAP2CPU
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
extern rtk_api_ret_t rtk_dot1x_portBasedEnable_set(rtk_port_t port, rtk_enable_t enable);

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
 *          - UNAUTH
 *          - AUTH
 */    
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_set(rtk_port_t port, rtk_dot1x_auth_status_t port_auth);

/* Function Name:
 *      rtk_dot1x_portBasedAuthStatus_get
 * Description:
 *      Get 802.1x port-based enable configuration
 * Input:
 *      port                  -   Port id
 * Output:
 *      pPort_auth         -   the pointer of the status of 802.1x port
 * Return:
 *      RT_ERR_OK                          -  Success
 *      RT_ERR_FAILED                     -  Failure
 *      RT_ERR_PORT_ID                   -  Invalid port id
 *      RT_ERR_NULL_POINTER           -  Input parameter is null pointer
 * Note:
 *      The authenticated status of 802.1x port-based network access control is as following:
 *          - UNAUTH
 *          - AUTH
 */    
extern rtk_api_ret_t rtk_dot1x_portBasedAuthStatus_get(rtk_port_t port, rtk_dot1x_auth_status_t *pPort_auth);

/* Function Name:
 *      rtk_dot1x_portBasedDirection_set
 * Description:
 *      Set 802.1x port-based operational direction configuration
 * Input:
 *      port                  -   Port id
 *      port_direction     -   Operation direction
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                 -  Success
 *      RT_ERR_FAILED                           -  Failure
 *      RT_ERR_PORT_ID                         -  Invalid port id
 *      RT_ERR_DOT1X_PORTBASEDOPDIR  -  Port-based opdir error
 * Note:
 *      The operate controlled direction of 802.1x port-based network access control is as following:
 *          - BOTH
 *          - IN
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
 *          - BOTH
 *          - IN
 */    
extern rtk_api_ret_t rtk_dot1x_portBasedDirection_get(rtk_port_t port, rtk_dot1x_direction_t *pPort_direction);

/* Function Name:
 *      rtk_dot1x_macBasedEnable_set
 * Description:
 *      Set 802.1x mac-based port enable configuration
 * Input:
 *      port                  -   Port id
 *      enable               -   The status of 802.1x mac-base funtion
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                         -  Success
 *      RT_ERR_FAILED                    -  Failure
 *      RT_ERR_PORT_ID                  -  Invalid port id
 * Note:
 *     If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *     be authenticated so packets from that port won't be dropped or trapped to CPU.
 *     The status of 802.1x MAC-based network access control is as following:
 *         - DISABLED
 *         - ENABLED
 */    
extern rtk_api_ret_t rtk_dot1x_macBasedEnable_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_dot1x_macBasedEnable_get
 * Description:
 *      Get 802.1x mac-based port enable configuration
 * Input:
 *      port                  -   Port id
 * Output:
 *      pEnable             -   the pointer of the status of 802.1x mac-base funtion
 * Return:
 *      RT_ERR_OK               -  Success
 *      RT_ERR_FAILED         -   Failure
 * Note:
 *     If a port is 802.1x MAC based network access control "enabled", the incoming packets should 
 *     be authenticated so packets from that port won't be dropped or trapped to CPU.
 *     The status of 802.1x MAC-based network access control is as following:
 *         - DISABLED
 *         - ENABLED
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
 *          - BOTH
 *          - IN
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
 * Note:
 *      The operate controlled direction of 802.1x mac-based network access control is as following:
 *          - BOTH
 *          - IN
 */  
extern rtk_api_ret_t rtk_dot1x_macBasedDirection_get(rtk_dot1x_direction_t *pMac_direction);

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_add
 * Description:
 *      Add an authenticated MAC to ASIC
 * Input:
 *      port            -  Port id
 *      pAuth_mac   - The authenticated MAC
 *      fid              -  no use for RTL8306E   
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                       - Success
 *      RT_ERR_FAILED                                 -  Failure
 *      RT_ERR_L2_ENTRY_NOTFOUND             -  Specified entry not found
 *      RT_ERR_DOT1X_MAC_PORT_MISMATCH  - Auth MAC and port mismatch eror 
 * Note:
 *     The API can add a 802.1x authenticated MAC address to port. If the MAC does not exist in LUT, 
 *     user can't add this MAC to auth status.
 */    
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_add(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid);

/* Function Name:
 *      rtk_dot1x_macBasedAuthMac_del
 * Description:
 *      Delete an authenticated MAC to ASIC
 * Input:
 *      port            -  Port id
 *      pAuth_mac   - The authenticated MAC
 *      fid              -  no use for RTL8306E   
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK                                       - Success
 *      RT_ERR_FAILED                                 -  Failure
 *      RT_ERR_L2_ENTRY_NOTFOUND             -  Specified entry not found
 *      RT_ERR_DOT1X_MAC_PORT_MISMATCH  - Auth MAC and port mismatch eror 
 * Note:
 *     The API can delete a 802.1x authenticated MAC address to port. It only change the auth status of
 *     the MAC and won't delete it from LUT.
 */    
extern rtk_api_ret_t rtk_dot1x_macBasedAuthMac_del(rtk_port_t port, rtk_mac_t *pAuth_mac, rtk_fid_t fid);


/* Function Name:
 *      rtk_svlan_init
 * Description:
 *      Initialize SVLAN Configuration
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_INPUT
 * Note:
 *    Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 
 *    0x9100 and 0x9200 for Q-in-Q SLAN design. User can set mathced ether 
 *    type as service provider supported protocol. After call this API, all ports are 
 *    set as CVLAN port. you can use rtk_svlan_servicePort_add to add SVLAN port. 
 */
extern rtk_api_ret_t rtk_svlan_init(void);

/* Function Name:
 *      rtk_svlan_servicePort_add
 * Description:
 *      Enable one service port in the specified device
 * Input:
 *      port     -  Port id
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *    This API is setting which port is connected to provider switch. All frames receiving from this port
 *    will recognize Service Vlan Tag.
 *    
 */
extern rtk_api_ret_t rtk_svlan_servicePort_add(rtk_port_t port);


/* Function Name:
 *      rtk_svlan_servicePort_del
 * Description:
 *      Disable one service port in the specified device
 * Input:
 *      none
 * Output:
 *      pSvlan_portmask  - svlan ports mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type
 *      0x9100 and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_servicePort_del(rtk_port_t port);

/* Function Name:
 *      rtk_svlan_servicePort_get
 * Description:
 *      Disable one service port in the specified device
 * Input:
 *      none
 * Output:
 *      pSvlan_portmask  - svlan ports mask
 * Return:
 *      RT_ERR_OK                 - success
 *      RT_ERR_FAILED            -  fail
 *      RT_ERR_NULL_POINTER  -  null pointer
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_servicePort_get(rtk_portmask_t *pSvlan_portmask);

/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      svlan_tag_id  - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_tpidEntry_set(rtk_svlan_tpid_t svlan_tag_id);


/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type. The default ether type of S-VLAN is 0x88a8
 * Input:
 *      pSvlan_tag_id       - Ether type of S-tag frame parsing in uplink ports
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 
 *      and 0x9200 for Q-in-Q SLAN design. User can set mathced ether type as service 
 *      provider supported protocol. 
 */
extern rtk_api_ret_t rtk_svlan_tpidEntry_get(rtk_svlan_tpid_t *pSvlan_tag_id);


/* Function Name:
 *      rtk_svlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID) for Service Provider Port
 * Input:
 *      port             - Port id
 *      pvid             - Specified Service VLAN ID
 *      priority         - 802.1p priority for the PVID
 *      dei               - Service VLAN tag DEI bit
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_VLAN_VID
 *      RT_ERR_SMI 
 *      RT_ERR_VLAN_PRIORITY 
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_svlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority, rtk_dei_t dei);

/* Function Name:
 *      rtk_svlan_portPvid_get
 * Description:
 *      Get Service VLAN ID(PVID) on specified port
 * Input:
 *      port             - Port id
 *      pPvid            - Specified VLAN ID
 *      pPriority        - 802.1p priority for the PVID
 *      pDei             - DEI bit
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SMI
 *      RT_ERR_PORT_ID
 *      RT_ERR_NULL_POINTER 
 * Note:
 *    The API is used for Port-based VLAN. The untagged frame received from the
 *    port will be classified to the specified VLAN and assigned to the specified priority.
 */
extern rtk_api_ret_t rtk_svlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t* pPriority, rtk_dei_t *pDei);

/* Function Name:
 *      rtk_filter_igrAcl_init
 * Description:
 *       Initialize ACL 
 * Input:
 *      none
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
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
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_TBL_FULL
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The API add an  ACL rule. <nl>
 *      phyport could be  0~5: port number,  RTL8306_ACL_ANYPORT: any port;<nl>
 *      protocol could be RTL8306_ACL_ETHER(ether type), RTL8306_ACL_TCP(TCP), RTL8306_ACL_UDP(UDP), RTL8306_ACL_TCPUDP(TCP or UDP);<nl>
 *      prority could be RTL8306_PRIO0~RTL8306_PRIO3;<nl>
 *      action could be RTL8306_ACT_DROP/RTL8306_ACT_PERMIT/RTL8306_ACT_TRAPCPU/RTL8306_ACT_MIRROR;<nl>
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
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      The API add an  ACL rule. <nl>
 *      phyport could be  0~5: port number,  RTL8306_ACL_ANYPORT: any port;<nl>
 *      protocol could be RTL8306_ACL_ETHER(ether type), RTL8306_ACL_TCP(TCP), RTL8306_ACL_UDP(UDP), RTL8306_ACL_TCPUDP(TCP or UDP);<nl>
 *      prority could be RTL8306_PRIO0~RTL8306_PRIO3;<nl>
 *      action could be RTL8306_ACT_DROP/RTL8306_ACT_PERMIT/RTL8306_ACT_TRAPCPU/RTL8306_ACT_MIRROR;<nl>
 */
extern rtk_api_ret_t rtk_filter_igrAcl_rule_get(rtk_filter_rule_t *pRule);



/* Function Name:
 *      rtk_filter_igrAcl_rule_del
 * Description:
 *      Delete an acl rule into acl table
 * Input:
 *      pRule    -  the pointer of rule structure
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 * Note:
 *      The API delete an  ACL rule. <nl>
 *      only phyport/protocol/data field in pRule needs to be specified.
 */
extern rtk_api_ret_t rtk_filter_igrAcl_rule_del(rtk_filter_rule_t *pRule);




#endif   /*__RTK_API_EXT_H__*/




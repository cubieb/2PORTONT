#ifndef _APOLLO_RAW_L34_H_
#define _APOLLO_RAW_L34_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>
#include <rtk/l34.h>
/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/

/****************************************************************/
/* Type Definition                                              */
/****************************************************************/

typedef enum apollo_raw_l34_hsb_type_e
{
    RAW_L34_HSB_ETHERNET      = 0,
    RAW_L34_HSB_RESERVED_PPTP = 1,
    RAW_L34_HSB_IPV4          = 2,
    RAW_L34_HSB_ICMP          = 3,
    RAW_L34_HSB_IGMP          = 4,
    RAW_L34_HSB_TCP           = 5,
    RAW_L34_HSB_UDP           = 6,
    RAW_L34_HSB_IPV6          = 7
} apollo_raw_l34_hsb_type_t; 


typedef enum apollo_raw_l34_hsa_pppoe_act_e
{
    RAW_L34_HSA_PPPOE_INACT      = 0,
    RAW_L34_HSA_PPPOE_TAGGING    = 1,
    RAW_L34_HSA_PPPOE_REMOVE     = 2,
    RAW_L34_HSA_PPPOE_MODIFY     = 3
} apollo_raw_l34_hsa_pppoe_act_t; 


typedef enum apollo_raw_l34_hsa_act_e
{
    RAW_L34_HSA_ACTION_INACT      = 0,
    RAW_L34_HSA_ACTION_TAGGING    = 1,
    RAW_L34_HSA_ACTION_REMOVE     = 2,
    RAW_L34_HSA_ACTION_MODIFY     = 3
} apollo_raw_l34_hsa_act_t; 



typedef enum apollo_raw_l34_hsba_mode_e
{
    RAW_L34_HSBA_BOTH_LOG         = 0,
    RAW_L34_HSBA_NO_LOG           = 1,
    RAW_L34_HSBA_LOG_ALL          = 2,
    RAW_L34_HSBA_LOG_FIRST_DROP   = 3,
    RAW_L34_HSBA_LOG_FIRST_PASS   = 4,
    RAW_L34_HSBA_LOG_FIRST_TO_CPU = 5    
} apollo_raw_l34_hsba_mode_t; 






typedef enum apollo_raw_l34_limbc_e
{
    RAW_L34_LIMBC_VLAN_BASED  = 0,
    RAW_L34_LIMBC_PORT_BASED  = 1,
    RAW_L34_LIMBC_MAC_BASED   = 2,
    RAW_L34_LIMBC_RESERVED    = 3
} apollo_raw_l34_limbc_t; 




typedef enum apollo_raw_l34_mode_e
{
    RAW_L34_L3_DISABLE_L4_DISABLE  = 0,
    RAW_L34_L3_ENABLE_L4_DISABLE  =  1,
    RAW_L34_L3_DISABLE_L4_ENABLE  =  2,
    RAW_L34_L3_ENABLE_L4_ENABLE   =  3,
    RAW_L34_L3_MODE_END
} apollo_raw_l34_mode_t; 


typedef enum apollo_raw_l34_l4_trf_e
{
    RAW_L34_L4TRF_TABLE0  = 0,
    RAW_L34_L4TRF_TABLE1  = 1
} apollo_raw_l34_l4_trf_t; 


typedef enum apollo_raw_l34_arp_trf_e
{
    RAW_L34_ARPTRF_TABLE0  = 0,
    RAW_L34_ARPTRF_TABLE1  = 1
} apollo_raw_l34_arp_trf_t; 





typedef struct apollo_raw_l34_hsb_s
{
    uint32  l2bridge;
    uint32  ipfrag_s;
    uint32  ipmf;
    uint32  l4_chksum_ok;
    uint32  l3_chksum_ok;
    uint32  cpu_direct_tx;
    uint32  udp_no_chksum;
    uint32  parse_fail;
    uint32  pppoe_if;
    uint32  svlan_if;
    uint32  ttls;   /*0b00 TTL=0, 0b01 TTL=1, 0b10 TTL>1 */
    apollo_raw_l34_hsb_type_t  type;   
    uint32   tcp_flag;
    uint32  cvlan_if;
    uint32   spa;
    uint32  cvid;
    uint32  len;
    uint32  dport_l4chksum;
    uint32  pppoe_id;
    ipaddr_t dip;
    ipaddr_t sip;
    uint32  sport_icmpid_chksum;
    rtk_mac_t  dmac;
}apollo_raw_l34_hsb_t;


typedef struct apollo_raw_l34_hsa_s
{
    uint32  l34trans;
    uint32  l2trans;
    uint32  inter_if;
    uint32  l4_pri_valid;
    uint32  frag;
    apollo_raw_l34_hsa_pppoe_act_t  pppoe_if;
    apollo_raw_l34_hsa_act_t  action;
    uint32  pppid_idx;
    uint32  difid;
    uint32  l4_pri_sel;
    uint32  dvid;
    uint32  reason;
    uint32  nexthop_mac_idx;
    uint32  l4_chksum;
    uint32  l3_chksum;
    uint32  port;
    ipaddr_t ip;
}apollo_raw_l34_hsa_t;



extern int32 apollo_raw_l34_hsba_mode_get(apollo_raw_l34_hsba_mode_t *mode);
extern int32 apollo_raw_l34_hsba_mode_set(apollo_raw_l34_hsba_mode_t mode);
extern int32 apollo_raw_l34_hsbaBusyFlag_get(uint32 *flag);
extern int32 apollo_raw_l34_hsbaActiveFlag_set(uint32 flag);
extern int32 apollo_raw_l34_hsbaActiveFlag_get(uint32 *flag);

extern int32 apollo_raw_l34_hsaData_set(apollo_raw_l34_hsa_t *hsaData);
extern int32 apollo_raw_l34_hsaData_get(apollo_raw_l34_hsa_t *hsaData);
extern int32 apollo_raw_l34_hsbData_set(apollo_raw_l34_hsb_t *hsbData);
extern int32 apollo_raw_l34_hsbData_get(apollo_raw_l34_hsb_t *hsbData);

extern int32 apollo_raw_l34_pppoeTrf_get(uint8 *pppoeTrf);

extern int32 apollo_raw_l34_portIntfIdx_get(rtk_port_t port,uint8 *idx);
extern int32 apollo_raw_l34_portIntfIdx_set(rtk_port_t port,uint8 idx);
extern int32 apollo_raw_l34_extPortIntfIdx_get(rtk_port_t port,uint8 *idx);
extern int32 apollo_raw_l34_extPortIntfIdx_set(rtk_port_t port,uint8 idx);
extern int32 apollo_raw_l34_vcPortIntfIdx_get(rtk_port_t port,uint8 *idx);
extern int32 apollo_raw_l34_vcPortIntfIdx_set(rtk_port_t port,uint8 idx);

extern int32 apollo_raw_l34_debugMonitor_set(rtk_enable_t enable);
extern int32 apollo_raw_l34_debugMonitor_get(rtk_enable_t *enable);
extern int32 apollo_raw_l34_natAttack2CPU_set(rtk_enable_t enable);
extern int32 apollo_raw_l34_natAttack2CPU_get(rtk_enable_t *enable);
extern int32 apollo_raw_l34_wanRouteAct_set(rtk_l34_wanroute_act_t act);
extern int32 apollo_raw_l34_wanRouteAct_get(rtk_l34_wanroute_act_t *act);
extern int32 apollo_raw_l34_LanIntfMDBC_set(apollo_raw_l34_limbc_t mode);
extern int32 apollo_raw_l34_LanIntfMDBC_get(apollo_raw_l34_limbc_t *mode);
extern int32 apollo_raw_l34_mode_set(apollo_raw_l34_mode_t mode);
extern int32 apollo_raw_l34_mode_get(apollo_raw_l34_mode_t *mode);
extern int32 apollo_raw_l34_TtlOperationMode_set(rtk_enable_t mode);
extern int32 apollo_raw_l34_TtlOperationMode_get(rtk_enable_t *mode);
extern int32 apollo_raw_l34_L3chksumErrAllow_set(rtk_enable_t mode);
extern int32 apollo_raw_l34_L3chksumErrAllow_get(rtk_enable_t *mode);
extern int32 apollo_raw_l34_L4chksumErrAllow_set(rtk_enable_t mode);
extern int32 apollo_raw_l34_L4chksumErrAllow_get(rtk_enable_t *mode);
extern int32 apollo_raw_l34_FragPkt2Cpu_set(rtk_enable_t mode);
extern int32 apollo_raw_l34_FragPkt2Cpu_get(rtk_enable_t *mode);

extern int32 apollo_raw_l34_hwL4TrfWrkTbl_set(apollo_raw_l34_l4_trf_t table);
extern int32 apollo_raw_l34_hwL4TrfWrkTbl_get(apollo_raw_l34_l4_trf_t *table);
extern int32 apollo_raw_l34_hwArpTrfWrkTbl_set(apollo_raw_l34_arp_trf_t table);
extern int32 apollo_raw_l34_hwArpTrfWrkTbl_get(apollo_raw_l34_arp_trf_t *table);
extern int32 apollo_raw_l34_hwArpTrfWrkTbl_Clear(apollo_raw_l34_arp_trf_t table);
extern int32 apollo_raw_l34_hwL4TrfWrkTbl_Clear(apollo_raw_l34_l4_trf_t table);



/* Function Name:
 *      apollo_raw_l34_portToWanAction_set
 * Description:
 *      Set L34 packet action from port to WAN action
 * Input:
 *      port   - port id
 *      wanIdx - index to WAN interface
 *      action - packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      the action support:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32 
apollo_raw_l34_portToWanAction_set(rtk_port_t port, uint32 wanIdx, rtk_action_t action);


/* Function Name:
 *      apollo_raw_l34_portToWanAction_get
 * Description:
 *      Get L34 packet action from port to WAN action
 * Input:
 *      port   - port id
 *      wanIdx - index to WAN interface
 *      action - packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      the action support:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32 
apollo_raw_l34_portToWanAction_get(rtk_port_t port, uint32 wanIdx, rtk_action_t *action);


/* Function Name:
 *      apollo_raw_l34_extPortToWanAction_set
 * Description:
 *      Set L34 packet action from port to WAN action
 * Input:
 *      extPort- extention port id
 *      wanIdx - index to WAN interface
 *      action - packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      the action support:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32 
apollo_raw_l34_extPortToWanAction_set(rtk_port_t extPort, uint32 wanIdx, rtk_action_t action);


/* Function Name:
 *      apollo_raw_l34_extPortToWanAction_get
 * Description:
 *      Get L34 packet action from port to WAN action
 * Input:
 *      extPort- extention port id
 *      wanIdx - index to WAN interface
 *      action - packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      the action support:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32 
apollo_raw_l34_extPortToWanAction_get(rtk_port_t extPort, uint32 wanIdx, rtk_action_t *action);


/* Function Name:
 *      apollo_raw_l34_wanToPortAction_set
 * Description:
 *      Set L34 packet action from WAN to port action
 * Input:
 *      wanIdx - index to WAN interface
 *      port   - port id
 *      action - packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      the action support:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32 
apollo_raw_l34_wanToPortAction_set(uint32 wanIdx, rtk_port_t port, rtk_action_t action);


/* Function Name:
 *      apollo_raw_l34_wanToPortAction_get
 * Description:
 *      Get L34 packet action from WAN to port action
 * Input:
 *      wanIdx - index to WAN interface
 *      port   - port id
 *      action - packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      the action support:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32 
apollo_raw_l34_wanToPortAction_get(uint32 wanIdx, rtk_port_t port, rtk_action_t *action);



/* Function Name:
 *      apollo_raw_l34_wanToExtPortAction_set
 * Description:
 *      Set L34 packet action from WAN to extention port action
 * Input:
 *      wanIdx - index to WAN interface
 *      extPort- extention port id
 *      action - packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      the action support:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32 
apollo_raw_l34_wanToExtPortAction_set(uint32 wanIdx, rtk_port_t extPort, rtk_action_t action);


/* Function Name:
 *      apollo_raw_l34_wanToExtPortAction_get
 * Description:
 *      Get L34 packet action from WAN to extention port action
 * Input:
 *      extPort- extention port id
 *      wanIdx - index to WAN interface
 *      action - packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      the action support:
 *      - ACTION_FORWARD
 *      - ACTION_DROP
 */
extern int32 
apollo_raw_l34_wanToExtPortAction_get(uint32 wanIdx, rtk_port_t extPort, rtk_action_t *action);

extern int32
apollo_raw_l34_globalFunction_get(rtk_enable_t *enable);

extern int32
apollo_raw_l34_globalFunction_set(rtk_enable_t enable);

extern int32
apollo_raw_l34_l4TrfTb_get(apollo_raw_l34_l4_trf_t table,uint32 index,rtk_enable_t *pIndicator);

extern int32 
apollo_raw_l34_arpTrfTb_get(apollo_raw_l34_arp_trf_t table,uint32 index,rtk_enable_t *pIndicator);


#endif /*#ifndef _APOLLO_RAW_L34_H_*/


/*
 * Copyright (C) 2011 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 */
#ifndef __DAL_APOLLO_L34_H_
#define __DAL_APOLLO_L34_H_
/*
 * Include Files
 */
#include <rtk/l34.h>

/*
 * Symbol Definition
 */
#define APOLLO_L34_HSB_WORD 7
#define APOLLO_L34_HSA_WORD 4

/*
 * Data Declaration
 */

typedef struct apollo_l34_hsb_param_s
{
    uint32 hsbWords[APOLLO_L34_HSB_WORD];
}apollo_l34_hsb_param_t;

typedef struct apollo_l34_hsa_param_s
{
    uint32 hsaWords[APOLLO_L34_HSA_WORD];
}apollo_l34_hsa_param_t;


typedef enum apollo_l34_hsab_mode_s
{
    APOLLO_L34_HSBA_TEST_MODE  = 0,
    APOLLO_L34_HSBA_NO_LOG = 1,
    APOLLO_L34_HSBA_LOG_ALL = 2,
    APOLLO_L34_HSBA_LOG_FIRST_DROP = 3,
    APOLLO_L34_HSBA_LOG_FIRS_PASS = 4,
    APOLLO_L34_HSBA_LOG_FIRS_TO_CPU = 5
}apollo_l34_hsab_mode_t;

extern int32
dal_apollo_l34_init(void);

/*NETIF table access*/
extern int32
dal_apollo_l34_netifTable_set(uint32 idx, rtk_l34_netif_entry_t *entry);

extern int32
dal_apollo_l34_netifTable_get(uint32 idx, rtk_l34_netif_entry_t *entry);

/*ARP table access*/
extern int32
dal_apollo_l34_arpTable_set(uint32 idx, rtk_l34_arp_entry_t *entry);

extern int32
dal_apollo_l34_arpTable_get(uint32 idx, rtk_l34_arp_entry_t *entry);

extern int32
dal_apollo_l34_arpTable_del(uint32 idx);

/*PPPoE table access*/
extern int32
dal_apollo_l34_pppoeTable_set(uint32 idx, rtk_l34_pppoe_entry_t *entry);

extern int32
dal_apollo_l34_pppoeTable_get(uint32 idx, rtk_l34_pppoe_entry_t *entry);

/*L3 Routing table access*/
extern int32
dal_apollo_l34_routingTable_set(uint32 idx, rtk_l34_routing_entry_t *entry);

extern int32
dal_apollo_l34_routingTable_get(uint32 idx, rtk_l34_routing_entry_t *entry);

extern int32
dal_apollo_l34_routingTable_del(uint32 idx);

/*NEXT Hop Table access*/
extern int32
dal_apollo_l34_nexthopTable_set(uint32 idx, rtk_l34_nexthop_entry_t *entry);

extern int32
dal_apollo_l34_nexthopTable_get(uint32 idx, rtk_l34_nexthop_entry_t *entry);

/*External IP Table access*/
extern int32
dal_apollo_l34_extIntIPTable_set(uint32 idx, rtk_l34_ext_intip_entry_t *entry);

extern int32
dal_apollo_l34_extIntIPTable_get(uint32 idx, rtk_l34_ext_intip_entry_t *entry);

extern int32
dal_apollo_l34_extIntIPTable_del(uint32 idx);

/*NAPTR Inbound table access*/
extern uint32
dal_apollo_l34_naptRemHash_get(uint32 sip, uint32 sport);

extern int32
dal_apollo_l34_naptInboundTable_set(int8 forced, uint32 idx,rtk_l34_naptInbound_entry_t *entry);

extern int32
dal_apollo_l34_naptInboundTable_get(uint32 idx,rtk_l34_naptInbound_entry_t *entry);

extern int32
dal_apollo_l34_naptOutboundTable_get(uint32 idx,rtk_l34_naptOutbound_entry_t *entry);


extern uint32 
dal_apollo_l34_naptInboundHashidx_get(uint32 dip, uint16 dport, uint16 isTCP);

/*NAPTR Outbound table access*/
extern int32
dal_apollo_l34_naptOutboundTable_set(int8 forced, uint32 idx,rtk_l34_naptOutbound_entry_t *entry);

extern int32
dal_apollo_l34_naptOutboundTable_get(uint32 idx,rtk_l34_naptOutbound_entry_t *entry);

extern uint32 
dal_apollo_l34_naptOutboundHashidx_get(int8 isTCP, uint32 sip, uint16 sport, uint32 dip, uint16 dport);


/*Table reset*/
extern int32 
dal_apollo_l34_table_reset(rtk_l34_table_type_t type);


/*HSA/HSB access*/
int32 
dal_apollo_l34_hsb_set(apollo_l34_hsb_param_t *hsb);

int32 
dal_apollo_l34_hsb_get(apollo_l34_hsb_param_t *hsb);


int32 
dal_apollo_l34_hsa_set(apollo_l34_hsa_param_t *hsa);

int32 
dal_apollo_l34_hsa_get(apollo_l34_hsa_param_t *hsa);

int32 
dal_apollo_l34_hsabCtrMode_set(apollo_l34_hsab_mode_t mode);


/* Function Name:
 *      dal_apollo_l34_globalState_set
 * Description:
 *      get l34 global status
 * Input:
 * 	  stateType	-status type
 *	  state		- status of state type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_globalState_set(rtk_l34_globalStateType_t stateType,rtk_enable_t state);




/* Function Name:
 *      dal_apollo_l34_globalState_get
 * Description:
 *      set l34 global status
 * Input:
 * 	  stateType	-status type
 * Output:
 *	  pState		- status of state type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_globalState_get(rtk_l34_globalStateType_t stateType,rtk_enable_t *pState);



/* Function Name:
 *      dal_apollo_l34_lookupMode_set
 * Description:
 *      configure l34 lookup mode selection
 * Input:
 * 	  lookupMode	- mode of l34 lookup method
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_lookupMode_set(rtk_l34_lookupMode_t lookupMode);


/* Function Name:
 *      dal_apollo_l34_lookupMode_get
 * Description:
 *      get l34 lookup mode selection
 * Input:
 *      None
 * Output:
 * 	  pLookupMode	- point of mode of l34 lookup method
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_lookupMode_get(rtk_l34_lookupMode_t *pLookupMode);



/* Function Name:
 *      dal_apollo_l34_lookupPortMap_set
 * Description:
 *      configure l34 port base mapping 
 * Input:
 * 	 portType	- port type, mac port/ext port/vc port
 *     portId	- port identity
 *	 wanIdx	- port based to wanIdx
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_lookupPortMap_set(rtk_l34_portType_t portType, uint32 portId, uint32 wanIdx);


/* Function Name:
 *      dal_apollo_l34_lookupPortMap_get
 * Description:
 *      configure l34 port base mapping 
 * Input:
 * 	 portType	- port type, mac port/ext port/vc port
 *     portId	- port identity
 * Output:
 *	 *pWanIdx	- port based to wanIdx
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_lookupPortMap_get(rtk_l34_portType_t portType, uint32 portId, uint32 *pWanIdx);


/* Function Name:
 *      dal_apollo_l34_wanRoutMode_set
 * Description:
 *      set wan route mode 
 * Input:
 * 	  wanRouteMode	- mode of wan routed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_wanRoutMode_set(rtk_l34_wanRouteMode_t wanRouteMode);


/* Function Name:
 *      dal_apollo_l34_wanRoutMode_get
 * Description:
 *      get  wan route mode 
 * Input:
 *      None
 * Output:
 * 	  *pWanRouteMode	- point of mode of wan routed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_wanRoutMode_get(rtk_l34_wanRouteMode_t *pWanRouteMode);


/* Function Name:
 *      dal_apollo_l34_arpTrfIndicator_get
 * Description:
 *      get  arp entry traffic indicator by index
 * Input:
 *      index: traffic table index
 * Output:
 * 	  *pArpIndicator	- point of traffic indicator for arp 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_arpTrfIndicator_get(uint32 index, rtk_enable_t *pArpIndicator);


/* Function Name:
 *      dal_apollo_l34_naptTrfIndicator_get
 * Description:
 *      get  napt entry traffic indicator by index
 * Input:
 *      index: traffic table index
 * Output:
 * 	  *pNaptIndicator	- point of traffic indicator for arp 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_naptTrfIndicator_get(uint32 index, rtk_enable_t *pNaptIndicator);



/* Function Name:
 *      dal_apollo_l34_pppTrfIndicator_get
 * Description:
 *      get  ppp entry traffic indicator by index
 * Input:
 *      index: traffic table index
 * Output:
 * 	  *pPppIndicator	- point of traffic indicator for pppoe table 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_pppTrfIndicator_get(uint32 index, rtk_enable_t *pPppIndicator);


/* Function Name:
 *      dal_apollo_l34_hsabMode_set
 * Description:
 *      Set L34 hsab mode
 * Input:
 *      hsabMode - L34 hsab 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_hsabMode_set(rtk_l34_hsba_mode_t hsabMode);


/* Function Name:
 *      dal_apollo_l34_hsabMode_get
 * Description:
 *      Get L34 hsab mode
 * Input:
 *      None
 * Output:
 *      *pHsabMode - point of L34 hsab 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_hsabMode_get(rtk_l34_hsba_mode_t *pHsabMode);



/* Function Name:
 *      dal_apollo_l34_hsaData_get
 * Description:
 *      Get L34 hsa data
 * Input:
 *      None
 * Output:
 *      pHsaData - point of hsa data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_hsaData_get(rtk_l34_hsa_t *pHsaData);


/* Function Name:
 *      dal_apollo_l34_hsbData_get
 * Description:
 *      Get L34 hsab mode
 * Input:
 *      None
 * Output:
 *      pHsaData - point of hsa data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_hsbData_get(rtk_l34_hsb_t *pHsbData);

/* Function Name:
 *      dal_apollo_l34_hsdState_set
 * Description:
 *      Set L34 hsd state
 * Input:
 *      hsdState - L34 hsd state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_hsdState_set(rtk_enable_t hsdState);


/* Function Name:
 *      dal_apollo_l34_hsdState_get
 * Description:
 *      Get L34 hsab mode
 * Input:
 *      None
 * Output:
 *      phsdState - point of hsd state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_hsdState_get(rtk_enable_t *phsdState);


/* Function Name:
 *      dal_apollo_l34_hwL4TrfWrkTbl_set
 * Description:
 *      Set HW working table id for L4 trf.
 * Input:
 *      dal_apollo_l34_l4_trf_t l4TrfTable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
extern int32 
dal_apollo_l34_hwL4TrfWrkTbl_set(rtk_l34_l4_trf_t l4TrfTable);


/* Function Name:
 *      dal_apollo_l34_hwL4TrfWrkTbl_get
 * Description:
 *      Get HW working table id for L4 trf.
 * Input:
 *      None
 * Output:
 *      dal_apollo_l34_l4_trf_t *pl4TrfTable
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
extern int32 
dal_apollo_l34_hwL4TrfWrkTbl_get(rtk_l34_l4_trf_t *pl4TrfTable);

/* Function Name:
 *      dal_apollo_l34_l4TrfTb_get
 * Description:
 *      Get HW working table id for L4 trf.
 * Input:
 *      l4TrfTable - table index
 *      l4EntryIndex - index of l4 table that went to get
 * Output:
 *      pIndicator - indicator for result of state
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
extern int32 
dal_apollo_l34_l4TrfTb_get(rtk_l34_l4_trf_t l4TrfTable,uint32 l4EntryIndex,rtk_enable_t *pIndicator);


/* Function Name:
 *      dal_apollo_l34_hwL4TrfWrkTbl_Clear
 * Description:
 *      Clear HW working table id for ARP trf.
 * Input:
 *      l4TrfTable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
extern int32 
dal_apollo_l34_hwL4TrfWrkTbl_Clear(rtk_l34_l4_trf_t l4TrfTable);


/* Function Name:
 *      dal_apollo_l34_hwArpTrfWrkTbl_set
 * Description:
 *      Set HW working table id for ARP trf.
 * Input:
 *      dal_apollo_l34_arp_trf_t arpTrfTable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
extern int32 
dal_apollo_l34_hwArpTrfWrkTbl_set(rtk_l34_arp_trf_t arpTrfTable);


/* Function Name:
 *      dal_apollo_l34_hwArpTrfWrkTbl_get
 * Description:
 *      Get HW working table id for ARP trf.
 * Input:
 *      None
 * Output:
 *      dal_apollo_l34_l4_trf_t *pArpTrfTable
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
extern int32 
dal_apollo_l34_hwArpTrfWrkTbl_get(rtk_l34_arp_trf_t *pArpTrfTable);

/* Function Name:
 *      dal_apollo_l34_arpTrfTb_get
 * Description:
 *      Get HW working table id for ARP trf.
 * Input:
 *      arpTrfTable - table index
 *      arpEntryIndex - index of l4 table that went to get
 * Output:
 *      pIndicator - indicator for result of state
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
extern int32 
dal_apollo_l34_arpTrfTb_get(rtk_l34_arp_trf_t arpTrfTable,uint32 arpEntryIndex,rtk_enable_t *pIndicator);


/* Function Name:
 *      dal_apollo_l34_hwArpTrfWrkTbl_Clear
 * Description:
 *      Clear HW working table id for ARP trf.
 * Input:
 *      l4TrfTable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
extern int32 
dal_apollo_l34_hwArpTrfWrkTbl_Clear(rtk_l34_arp_trf_t arpTrfTable);

/* Function Name:
 *      dal_apollo_l34_naptTrfIndicator_get_all
 * Description:
 *      get  napt entry traffic indicator
 * Input:
 *      None
 * Output:
 * 	  *pNaptMaps	- point of traffic indicator for mask (64*32bit mask)
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_naptTrfIndicator_get_all(uint32 *pNaptMaps);


/* Function Name:
 *      dal_apollo_l34_arpTrfIndicator_get_all
 * Description:
 *      get  all arp entry traffic indicator
 * Input:
 *      None
 * Output:
 * 	  *pArpMaps	- point of traffic indicator for arp 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
dal_apollo_l34_arpTrfIndicator_get_all(uint32 *pArpMaps);



#endif /*#ifndef __DAL_APOLLO_L34_H_*/


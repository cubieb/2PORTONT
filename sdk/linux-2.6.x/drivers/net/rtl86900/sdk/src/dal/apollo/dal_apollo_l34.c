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
 */ 

/*  
 * Include Files 
 */
#include <dal/apollo/dal_apollo.h>

#include <rtk/l34.h>
#include <rtk/qos.h>
#include <dal/apollo/dal_apollo_l34.h>
#include <dal/apollo/raw/apollo_raw.h>
#include <dal/apollo/raw/apollo_raw_l34.h>
#include <ioal/mem32.h>


/* 
 * Symbol Definition 
 */

static int32 
_dal_apollo_l34_hsabMode_cfg2raw(rtk_l34_hsba_mode_t mode,apollo_raw_l34_hsba_mode_t *pMode)
{
	switch(mode){
	case L34_HSBA_BOTH_LOG:
		*pMode =  RAW_L34_HSBA_BOTH_LOG;
	break;
	case L34_HSBA_NO_LOG:
		*pMode =  RAW_L34_HSBA_NO_LOG;
	break;
	case L34_HSBA_LOG_ALL:
		*pMode =  RAW_L34_HSBA_LOG_ALL;
	break;
	case L34_HSBA_LOG_FIRST_DROP:
		*pMode =  RAW_L34_HSBA_LOG_FIRST_DROP;
	break;
	case L34_HSBA_LOG_FIRST_PASS:
		*pMode =  RAW_L34_HSBA_LOG_FIRST_PASS;
	break;
	case L34_HSBA_LOG_FIRST_TO_CPU:
		*pMode =  RAW_L34_HSBA_LOG_FIRST_TO_CPU;
	break;
	default:
		return RT_ERR_FAILED;
	break;
	}
	return RT_ERR_OK;
}

static int32 
_dal_apollo_l34_hsabMode_raw2cfg(apollo_raw_l34_hsba_mode_t mode,rtk_l34_hsba_mode_t *pMode)
{
	switch(mode){
	case  RAW_L34_HSBA_BOTH_LOG:
		*pMode = L34_HSBA_BOTH_LOG;
	break;
	case  RAW_L34_HSBA_NO_LOG:
		*pMode = L34_HSBA_NO_LOG;
	break;
	case  RAW_L34_HSBA_LOG_ALL:
		*pMode = L34_HSBA_LOG_ALL;
	break;
	case  RAW_L34_HSBA_LOG_FIRST_DROP:
		*pMode = L34_HSBA_LOG_FIRST_DROP;
	break;
	case  RAW_L34_HSBA_LOG_FIRST_PASS:
		*pMode = L34_HSBA_LOG_FIRST_PASS;
	break;
	case  RAW_L34_HSBA_LOG_FIRST_TO_CPU:
		*pMode = L34_HSBA_LOG_FIRST_TO_CPU;
	break;
	default:
		return RT_ERR_FAILED;
	break;
	}
	return RT_ERR_OK;
}



static int32
l34_portMapNetif_get(rtk_l34_portType_t type,uint32 *map,uint32 index)
{

	uint32 size;
	uint8 value;
	int32 ret;
	int32 (*fun)(rtk_port_t,uint8*);

	switch(type){
	case L34_PORT_MAC:
		fun = apollo_raw_l34_portIntfIdx_get;
		size = L34_MAC_PORT_MAX;
	break;
	case L34_PORT_EXTENSION:
		fun = apollo_raw_l34_extPortIntfIdx_get;
		size = L34_EXT_PORT_MAX;
	break;
	case L34_PORT_DSLVC:
		fun = apollo_raw_l34_vcPortIntfIdx_get;
		size = L34_DSLVC_PORT_MAX;
	break;
	default:
		return RT_ERR_FAILED;
	break;
	}

	if((ret =fun(index,&value))!=RT_ERR_OK)
	{
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return RT_ERR_FAILED;
	}
	*map = value;
	return RT_ERR_OK;
}



static int32
l34_mode_l3_set(rtk_enable_t state)
{
	int32 ret;
	apollo_raw_l34_mode_t l34mode;

	if((ret=apollo_raw_l34_mode_get(&l34mode))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	l34mode = state | (l34mode & 2);
	
	if((ret=apollo_raw_l34_mode_set(l34mode))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	return RT_ERR_OK;
}

static int32
l34_mode_l4_set(rtk_enable_t state)
{
	int32 ret;
	apollo_raw_l34_mode_t l34mode;

	if((ret=apollo_raw_l34_mode_get(&l34mode))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	l34mode = (state << 1) | (l34mode & 1);
	
	if((ret=apollo_raw_l34_mode_set(l34mode))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	return RT_ERR_OK;
}


static int32
l34_lookupMode_cfg2raw(rtk_l34_lookupMode_t type,uint32 *raw)
{

	switch(type){
	case L34_LOOKUP_VLAN_BASE:
		*raw = RAW_L34_LIMBC_VLAN_BASED;
	break;
	case L34_LOOKUP_MAC_BASE:
		*raw = RAW_L34_LIMBC_MAC_BASED;
	break;
	case L34_LOOKUP_PORT_BASE:
		*raw = RAW_L34_LIMBC_PORT_BASED;
	break;
	default:
		return  RT_ERR_INPUT;
	break;
	}
	return RT_ERR_OK;
}

static int32
l34_lookupMode_raw2cfg(uint32 raw, rtk_l34_lookupMode_t *mode)
{

	switch(raw){
	case RAW_L34_LIMBC_VLAN_BASED:
		*mode = L34_LOOKUP_VLAN_BASE;
	break;
	case RAW_L34_LIMBC_MAC_BASED:
		*mode = L34_LOOKUP_MAC_BASE;
	break;
	case RAW_L34_LIMBC_PORT_BASED:
		*mode = L34_LOOKUP_PORT_BASE;
	break;
	default:
		return  RT_ERR_INPUT;
	break;
	}
	return RT_ERR_OK;
}

static int32
l34_wanroute_cfg2raw(rtk_l34_wanRouteMode_t type,uint32 *raw)
{
	switch(type){
	case L34_WANROUTE_FORWARD:
		*raw = L34_WAN_ROUTE_FWD;
	break;
	case L34_WANROUTE_FORWARD2CPU:
		*raw = L34_WAN_ROUTE_FWD_TO_CPU;
	break;
	case L34_WANROUTE_DROP:
		*raw = L34_WAN_ROUTE_DROP;
	break;
	default:
		return  RT_ERR_INPUT;
	break;
	}
	return RT_ERR_OK;
}

static int32
l34_wanroute_raw2cfg(uint32 raw,rtk_l34_wanRouteMode_t *type)
{

	switch(raw){
	case L34_WAN_ROUTE_FWD:
		*type = L34_WANROUTE_FORWARD;
	break;
	case L34_WAN_ROUTE_FWD_TO_CPU:
		*type = L34_WANROUTE_FORWARD2CPU;
	break;
	case L34_WAN_ROUTE_DROP:
		*type = L34_WANROUTE_DROP;
	break;
	default:
		return RT_ERR_INPUT;
	break;
	}

	return RT_ERR_OK;
}

static void 
l34_l4Trf_cfg2raw(rtk_l34_l4_trf_t cfg,apollo_raw_l34_l4_trf_t *table)
{
	switch(cfg){
	case RTK_L34_L4TRF_TABLE0:
		*table = RAW_L34_L4TRF_TABLE0;
	break;
	case RTK_L34_L4TRF_TABLE1:
		*table =RAW_L34_L4TRF_TABLE1;
	break;
	default:
	break;
	}
}

static void 
l34_arpTrf_cfg2raw(rtk_l34_arp_trf_t cfg,apollo_raw_l34_arp_trf_t *table)
{
	switch(cfg){
	case RTK_L34_ARPTRF_TABLE0:
		*table = RAW_L34_ARPTRF_TABLE0;
	break;
	case RTK_L34_ARPTRF_TABLE1:
		*table =RAW_L34_ARPTRF_TABLE1;
	break;
	default:
	break;
	}
}

 static void 
 l34_l4Trf_raw2cfg(apollo_raw_l34_l4_trf_t table,rtk_l34_l4_trf_t *cfg)
{
	switch(table){
	case RAW_L34_L4TRF_TABLE0:
		*cfg = RTK_L34_L4TRF_TABLE0;
	break;
	case RAW_L34_L4TRF_TABLE1:
		*cfg =RTK_L34_L4TRF_TABLE1;
	break;
	default:
	break;
	}
}

 static void 
 l34_arpTrf_raw2cfg(apollo_raw_l34_arp_trf_t table,rtk_l34_arp_trf_t *cfg)
{
	switch(table){
	case RAW_L34_ARPTRF_TABLE0:
		*cfg = RTK_L34_ARPTRF_TABLE0;
	break;
	case RAW_L34_ARPTRF_TABLE1:
		*cfg =RTK_L34_ARPTRF_TABLE1;
	break;
	default:
	break;
	}
}


static int
l34_patch(void)
{
	int32 ret;
	int i;
	rtk_port_t port;
	
	HAL_SCAN_ALL_PORT(port)
    	{
		if((ret = rtk_qos_portDscpRemarkSrcSel_set(port,DSCP_RMK_SRC_DSCP))!=RT_ERR_OK)
		{
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        		return RT_ERR_FAILED;
		}
		
	}
	
	for(i=0;i<64;i++){
		if((ret = rtk_qos_dscp2DscpRemarkGroup_set(0,i,i))!=RT_ERR_OK)
		{
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        		return RT_ERR_FAILED;
		}
	}

	return RT_ERR_OK;
}


/* 
 * Data Declaration 
 */
static uint32               l34_init = {INIT_NOT_COMPLETED};


/*
 * Macro Definition
 */


/* 
 * Function Declaration 
 */

unsigned int _dal_apollo_l34_log2(unsigned int n);
uint8 _dal_apollo_l34_routingTable_nhNum_to_asicVal(uint8 nh_num);
int8 _dal_apollo_l34_routingTable_asicVal_to_nhNum(uint8 asic_val);
/* Function Name:
 *      dal_apollo_l34_init
 * Description:
 *      Initialize l34 module of the specified device.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize l34 module before calling any l34 APIs.
 */
int32
dal_apollo_l34_init(void)
{
	int32 ret;
	int32 i;
	rtk_l34_globalStateType_t cfg;

	l34_init = INIT_COMPLETED;

	/*reset all table*/
	for(i=0;i<L34_TABLE_END;i++)
	{
		if((ret = dal_apollo_l34_table_reset(i))!=RT_ERR_OK)
		{
			if(ret==RT_ERR_CHIP_NOT_SUPPORTED){
				continue;
			}
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			l34_init = INIT_NOT_COMPLETED ;
	        	return ret;
		}
	}
	/*enable l34 routing */
	cfg = L34_GLOBAL_L34_STATE;
	if((ret = dal_apollo_l34_globalState_set(cfg, ENABLED))!=RT_ERR_OK)
	{
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		l34_init = INIT_NOT_COMPLETED ;
	   	return ret;
	}
	/*patch for RTL86907 A1 version*/
	if((ret = l34_patch())!=RT_ERR_OK)
	{
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		l34_init = INIT_NOT_COMPLETED ;
	   	return ret;
	}
	
	return RT_ERR_OK;
} /* end of dal_apollo_l34_init */





/* Function Name:
 *      dal_apollo_l34_netifTable_set
 * Description:
 *      Set netif entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    
 *      RT_ERR_NULL_POINTER 
 *      RT_ERR_ENTRY_INDEX
 *      RT_ERR_VLAN_VID
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32
dal_apollo_l34_netifTable_set(uint32 idx, rtk_l34_netif_entry_t *entry)
{
    int32 ret;
    uint32 tempVal;
    
    l34_netif_entry_t netif_entry;
    
    /*input error check*/
    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_NETIF_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((RTK_VLAN_ID_MAX < entry->vlan_id), RT_ERR_VLAN_VID);
    RT_PARAM_CHK((entry->valid != 0 && entry->valid != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->enable_rounting != 0 && entry->enable_rounting != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->mtu >= 16384), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->mac_mask != 0 && entry->mac_mask != 4 && entry->mac_mask != 6 && entry->mac_mask != 7), RT_ERR_INPUT);
    
    /* check Init status */
    RT_INIT_CHK(l34_init);

    osal_memset(&netif_entry, 0x0, sizeof(netif_entry));

    
    tempVal = entry->valid;
    if ((ret = table_field_set(NETIFt, NETIF_VALIDtf, &tempVal, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    tempVal = entry->mtu;
    if ((ret = table_field_set(NETIFt, NETIF_MTUtf, &tempVal, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    tempVal = entry->enable_rounting;
    if ((ret = table_field_set(NETIFt, NETIF_EN_ROUNTINGtf, &tempVal, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    tempVal = entry->mac_mask;
    if ((ret = table_field_set(NETIFt, NETIF_MAC_MASKtf, &tempVal, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    if ((ret = table_field_mac_set(NETIFt, NETIF_GATEWAY_MACtf, (uint8 *)&entry->gateway_mac, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    tempVal = entry->vlan_id;
    if ((ret = table_field_set(NETIFt, NETIF_VLAN_IDtf, &tempVal, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }


    if ((ret = table_write(NETIFt, idx, (uint32 *)&netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    return RT_ERR_OK;
} /* end of dal_apollo_l34_netifTable_set */



/* Function Name:
 *      dal_apollo_l34_netifTable_get
 * Description:
 *      Get a arp entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 
dal_apollo_l34_netifTable_get(uint32 idx, rtk_l34_netif_entry_t *entry)
{
    int32 ret;
    l34_netif_entry_t netif_entry;
    uint32 tmpVal;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_NETIF_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);

    /* check Init status */
    RT_INIT_CHK(l34_init);
    
    osal_memset(&netif_entry, 0x0, sizeof(netif_entry));
    
    if ((ret = table_read(NETIFt, idx, (uint32 *)&netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_field_get(NETIFt, NETIF_MTUtf, (uint32 *)&tmpVal, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->mtu = tmpVal;

    if ((ret = table_field_get(NETIFt, NETIF_EN_ROUNTINGtf, (uint32 *)&tmpVal, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->enable_rounting = tmpVal;

    if ((ret = table_field_get(NETIFt, NETIF_MAC_MASKtf, (uint32 *)&tmpVal, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->mac_mask = tmpVal;

    if ((ret = table_field_mac_get(NETIFt, NETIF_GATEWAY_MACtf, (uint8 *)&(entry->gateway_mac), (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_field_get(NETIFt, NETIF_VLAN_IDtf, (uint32 *)&tmpVal, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->vlan_id = tmpVal;

    if ((ret = table_field_get(NETIFt, NETIF_VALIDtf, (uint32 *)&tmpVal, (uint32 *) &netif_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->valid = tmpVal;

    return RT_ERR_OK;
} /* end of dal_apollo_l34_netifTable_get */


/* Function Name:
 *      dal_apollo_l34_arpTable_set
 * Description:
 *      Set ARP entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT    
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32
dal_apollo_l34_arpTable_set(uint32 idx, rtk_l34_arp_entry_t *entry)
{
    int32 ret;
    l34_arp_entry_t arp_entry;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_ARP_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((entry->valid != 0 && entry->valid != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_L2_LEARN_LIMIT_CNT_MAX()<=entry->nhIdx), RT_ERR_ENTRY_INDEX);


    /* check Init status */
    RT_INIT_CHK(l34_init);

    
    osal_memset(&arp_entry, 0x0, sizeof(arp_entry));
    
    if ((ret = table_field_set(ARP_TABLEt, ARP_TABLE_NH_IDXtf, (uint32 *)&entry->nhIdx, (uint32 *) &arp_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_field_set(ARP_TABLEt, ARP_TABLE_VALIDtf, (uint32 *)&entry->valid, (uint32 *) &arp_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_write(ARP_TABLEt, idx, (uint32 *)&arp_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_apollo_l34_arpTable_set */



/* Function Name:
 *      dal_apollo_l34_arpTable_get
 * Description:
 *      Get a arp entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_arpTable_get(uint32 idx, rtk_l34_arp_entry_t *entry)
{
    int32 ret;
    l34_arp_entry_t arp_entry;
    uint32 tmp_val;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_ARP_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    
    /* check Init status */
    RT_INIT_CHK(l34_init);

    
    osal_memset(&arp_entry, 0x0, sizeof(arp_entry));
    
    if ((ret = table_read(ARP_TABLEt, idx, (uint32 *)&arp_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_field_get(ARP_TABLEt, ARP_TABLE_NH_IDXtf, (uint32 *)&(tmp_val), (uint32 *) &arp_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->nhIdx = tmp_val;

    if ((ret = table_field_get(ARP_TABLEt, ARP_TABLE_VALIDtf, (uint32 *)&(tmp_val), (uint32 *) &arp_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->valid = tmp_val;


    return RT_ERR_OK;
} /* end of dal_apollo_l34_arpTable_get */

/* Function Name:
 *      dal_apollo_l34_arpTable_del
 * Description:
 *      delete a arp entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_arpTable_del(uint32 idx)
{
    int32 ret;
    l34_arp_entry_t arp_entry;
    uint32 is_valid=0;
    RT_PARAM_CHK((HAL_L34_ARP_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);

    /* check Init status */
    RT_INIT_CHK(l34_init);


    osal_memset(&arp_entry, 0x0, sizeof(arp_entry));

    if ((ret = table_field_set(ARP_TABLEt, ARP_TABLE_VALIDtf, (uint32 *)&is_valid, (uint32 *) &arp_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_write(ARP_TABLEt, idx, (uint32 *)&arp_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;

}/* end of dal_apollo_l34_arpTable_del */



/* Function Name:
 *      dal_apollo_l34_pppoeTable_set
 * Description:
 *      Set ARP entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_pppoeTable_set(uint32 idx, rtk_l34_pppoe_entry_t *entry)
{
    int32 ret;
    l34_pppoe_entry_t pppoe_entry;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_PPPOE_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((entry->sessionID >= 0x10000), RT_ERR_INPUT);

    /* check Init status */
    RT_INIT_CHK(l34_init);


    if(idx > APOLLO_PPPOE_ENTRY)
        return RT_ERR_FAILED;

    osal_memset(&pppoe_entry, 0x0, sizeof(pppoe_entry));
    
    if ((ret = table_field_set(PPPOE_TABLEt, PPPOE_TABLE_SESSION_IDtf, (uint32 *)&entry->sessionID, (uint32 *) &pppoe_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_write(PPPOE_TABLEt, idx, (uint32 *)&pppoe_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_apollo_l34_pppoeTable_set */




/* Function Name:
 *      dal_apollo_l34_pppoeTable_get
 * Description:
 *      Get a PPPOE entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_pppoeTable_get(uint32 idx, rtk_l34_pppoe_entry_t *entry)
{
    int32 ret;
    l34_pppoe_entry_t pppoe_entry;
    uint32 tmp_val;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_PPPOE_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    
    /* check Init status */
    RT_INIT_CHK(l34_init);
   
    osal_memset(&pppoe_entry, 0x0, sizeof(pppoe_entry));
    
    if ((ret = table_read(PPPOE_TABLEt, idx, (uint32 *)&pppoe_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_field_get(PPPOE_TABLEt, PPPOE_TABLE_SESSION_IDtf, (uint32 *)&(tmp_val), (uint32 *) &pppoe_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->sessionID = tmp_val;


    return RT_ERR_OK;
} /* end of dal_apollo_l34_pppoeTable_get */



/* 
    Number of entries allocate for Nextp hop routing entry.
    asic_val  entry number
    --------  ------------
    0          1 
    1          2
    2          4
    3          8
    4         16
 */

unsigned int _dal_apollo_l34_log2(unsigned int n) 
{ 
    return n < 2 ? 0 : 1 + _dal_apollo_l34_log2(n/2); 
}


uint8
_dal_apollo_l34_routingTable_nhNum_to_asicVal(uint8 nh_num)
{
    return _dal_apollo_l34_log2(nh_num);
}

int8
_dal_apollo_l34_routingTable_asicVal_to_nhNum(uint8 asic_val)
{
   	uint8 ret=1;
    int i;
    for(i=1;i<asic_val;i++)
    {
        ret=ret*2;
    }


    return ret;
}

/* Function Name:
 *      dal_apollo_l34_routingTable_set
 * Description:
 *      Set ARP entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_routingTable_set(uint32 idx, rtk_l34_routing_entry_t *entry)
{
    int32 ret;
    l34_routing_entry_t routing_entry;
    rtk_table_list_t table_type;
    uint32 tmpVal;
    uint32 nh_num_asic_val;
    
    /*input error check*/
    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_ROUTING_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((31 < entry->ipMask), RT_ERR_INPUT);
    RT_PARAM_CHK((L34_PROCESS_END <= entry->process), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->valid != 0 && entry->valid != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->internal != 0 && entry->internal != 1), RT_ERR_INPUT);

    RT_PARAM_CHK((HAL_L34_NETIF_ENTRY_MAX() <= entry->netifIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((128 <= entry->arpStart), RT_ERR_INPUT);
    RT_PARAM_CHK((128 <= entry->arpEnd), RT_ERR_INPUT);

    RT_PARAM_CHK((HAL_L34_NH_ENTRY_MAX() <= entry->nhStart), RT_ERR_INPUT);
    RT_PARAM_CHK((17 <= entry->nhNum), RT_ERR_INPUT);
    RT_PARAM_CHK((3 <= entry->nhAlgo), RT_ERR_INPUT);
    RT_PARAM_CHK((7 <= entry->ipDomain), RT_ERR_INPUT);

    /* check Init status */
    RT_INIT_CHK(l34_init);
    
    osal_memset(&routing_entry, 0x0, sizeof(routing_entry));


    /*general part*/
    tmpVal = entry->ipAddr;
    if ((ret = table_field_set(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_IP_ADDRtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    tmpVal = entry->ipMask;
    if ((ret = table_field_set(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_IP_MASKtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }


    tmpVal = entry->process;
    if ((ret = table_field_set(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_PROCESS_TYPEtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }


    tmpVal = entry->valid;
    if ((ret = table_field_set(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_VALIDtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }


    tmpVal = entry->internal;
    if ((ret = table_field_set(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_IS_INTERNALtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if(entry->process == L34_PROCESS_CPU || entry->process == L34_PROCESS_DROP)
        table_type = L3_ROUTING_DROP_TRAPt;
    else if(entry->process == L34_PROCESS_ARP)
        table_type = L3_ROUTING_LOCAL_ROUTEt;
    else if(entry->process == L34_PROCESS_NH)
        table_type = L3_ROUTING_GLOBAL_ROUTEt;
    else
        return RT_ERR_FAILED;

    /*process == arp*/
    if(table_type == L3_ROUTING_LOCAL_ROUTEt)
    {
        tmpVal = entry->netifIdx;
        if ((ret = table_field_set(table_type, L3_ROUTING_LOCAL_ROUTE_DEST_NETIF_IDXtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }

        tmpVal = entry->arpStart;
        if ((ret = table_field_set(table_type, L3_ROUTING_LOCAL_ROUTE_ARP_ADDR_STARTtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        
        tmpVal = entry->arpEnd;
        if ((ret = table_field_set(table_type, L3_ROUTING_LOCAL_ROUTE_ARP_ADDR_ENDtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
    }
    /*process == nexthop*/
    else if(table_type == L3_ROUTING_GLOBAL_ROUTEt)
    {
        tmpVal = entry->ipDomain;
        if ((ret = table_field_set(table_type, L3_ROUTING_GLOBAL_ROUTE_IP_DOMAIN_NUMtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        tmpVal = entry->nhAlgo;
        if ((ret = table_field_set(table_type, L3_ROUTING_GLOBAL_ROUTE_NH_ALGOtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        #if 0
        tmpVal = entry->nhNxt;
        if ((ret = table_field_set(table_type, L3_ROUTING_GLOBAL_ROUTE_NH_IDXtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        #endif
        tmpVal = entry->nhStart;
        if ((ret = table_field_set(table_type, L3_ROUTING_GLOBAL_ROUTE_NH_ADDR_STARTtf, (uint32 *)&tmpVal, (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }

        /*translate nhNum to ASIC setting*/
        nh_num_asic_val = _dal_apollo_l34_routingTable_nhNum_to_asicVal(entry->nhNum);

        if ((ret = table_field_set(table_type, L3_ROUTING_GLOBAL_ROUTE_NH_NUMtf, (uint32 *)&nh_num_asic_val, (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }


    }


    if ((ret = table_write(table_type, idx, (uint32 *)&routing_entry)) != RT_ERR_OK)
    { 
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);   
        return RT_ERR_FAILED;
    }
   
    return RT_ERR_OK;
} /* end of dal_apollo_l34_routingTable_set */

/* Function Name:
 *      dal_apollo_l34_routingTable_get
 * Description:
 *      Get a routing table entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_routingTable_get(uint32 idx, rtk_l34_routing_entry_t *entry)
{
    int32 ret;
    l34_routing_entry_t routing_entry;
    rtk_table_list_t table_type;
    uint32 tmp_val;
    
    /*input error check*/
    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_ROUTING_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    
    /* check Init status */
    RT_INIT_CHK(l34_init);

    
    osal_memset(&routing_entry, 0x0, sizeof(routing_entry));
    
    if ((ret = table_read(L3_ROUTING_GLOBAL_ROUTEt, idx, (uint32 *)&routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    if ((ret = table_field_get(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_PROCESS_TYPEtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->process = tmp_val;

    if ((ret = table_field_get(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_IP_ADDRtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    entry->ipAddr = tmp_val;

    if ((ret = table_field_get(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_IP_MASKtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->ipMask = tmp_val;


    if ((ret = table_field_get(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_VALIDtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->valid = tmp_val;

    if ((ret = table_field_get(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_IS_INTERNALtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->internal = tmp_val;



    /*get entry process type*/
    if(entry->process == L34_PROCESS_CPU || entry->process == L34_PROCESS_DROP)
        table_type = L3_ROUTING_DROP_TRAPt;
    else if(entry->process == L34_PROCESS_ARP)
        table_type = L3_ROUTING_LOCAL_ROUTEt;
    else if(entry->process == L34_PROCESS_NH)
        table_type = L3_ROUTING_GLOBAL_ROUTEt;
    else
        return RT_ERR_FAILED;


    /*process == arp*/
    if(table_type == L3_ROUTING_LOCAL_ROUTEt)
    {
        if ((ret = table_field_get(table_type, L3_ROUTING_LOCAL_ROUTE_DEST_NETIF_IDXtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        entry->netifIdx = tmp_val;

        if ((ret = table_field_get(table_type, L3_ROUTING_LOCAL_ROUTE_ARP_ADDR_STARTtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        entry->arpStart = tmp_val;    

        if ((ret = table_field_get(table_type, L3_ROUTING_LOCAL_ROUTE_ARP_ADDR_ENDtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        entry->arpEnd = tmp_val;    


    }
    /*process == nexthop*/
    else if(table_type == L3_ROUTING_GLOBAL_ROUTEt)
    {
        if ((ret = table_field_get(table_type, L3_ROUTING_GLOBAL_ROUTE_IP_DOMAIN_NUMtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        entry->ipDomain = tmp_val;    

        if ((ret = table_field_get(table_type, L3_ROUTING_GLOBAL_ROUTE_NH_ALGOtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        entry->nhAlgo = tmp_val;  

        if ((ret = table_field_get(table_type, L3_ROUTING_GLOBAL_ROUTE_NH_IDXtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        entry->nhNxt = tmp_val;  

        if ((ret = table_field_get(table_type, L3_ROUTING_GLOBAL_ROUTE_NH_ADDR_STARTtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        entry->nhStart = tmp_val;  



        if ((ret = table_field_get(table_type, L3_ROUTING_GLOBAL_ROUTE_NH_NUMtf, (uint32 *)&(tmp_val), (uint32 *) &routing_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        /*translate nhNum to ASIC setting*/
        entry->nhNum = _dal_apollo_l34_routingTable_asicVal_to_nhNum(tmp_val);

    }
    return RT_ERR_OK;
} /* end of dal_apollo_l34_routingTable_get */




/* Function Name:
 *      dal_apollo_l34_routingTable_del
 * Description:
 *      delete a routing entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_routingTable_del(uint32 idx)
{
    int32 ret;
    l34_routing_entry_t routing_entry;
    uint32 is_valid=0;

    /* check Init status */
    RT_INIT_CHK(l34_init);


    osal_memset(&routing_entry, 0x0, sizeof(routing_entry));

    if ((ret = table_field_set(L3_ROUTING_GLOBAL_ROUTEt, L3_ROUTING_GLOBAL_ROUTE_VALIDtf, (uint32 *)&is_valid, (uint32 *) &routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_write(L3_ROUTING_GLOBAL_ROUTEt, idx, (uint32 *)&routing_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;

}/* end of dal_apollo_l34_routingTable_del */


/* Function Name:
 *      dal_apollo_l34_nexthopTable_set
 * Description:
 *      Set nexthop entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_nexthopTable_set(uint32 idx, rtk_l34_nexthop_entry_t *entry)
{
    int32 ret;
    l34_nexthop_entry_t nh_entry;
    uint32 tmp_val;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_NH_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((2048 <= entry->nhIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_L34_NETIF_ENTRY_MAX() <= entry->ifIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_L34_PPPOE_ENTRY_MAX() <= entry->pppoeIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((L34_NH_END <= entry->type), RT_ERR_INPUT);

    
    /* check Init status */
    RT_INIT_CHK(l34_init);

    
    osal_memset(&nh_entry, 0x0, sizeof(nh_entry));
    
    
    tmp_val = (uint32)entry->nhIdx;
    if ((ret = table_field_set(NEXT_HOP_TABLEt, NEXT_HOP_TABLE_NH_L2IDXtf, &tmp_val, (uint32 *) &nh_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    tmp_val = (uint32)entry->pppoeIdx;
    if ((ret = table_field_set(NEXT_HOP_TABLEt, NEXT_HOP_TABLE_PPPIDXtf, &tmp_val, (uint32 *) &nh_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    tmp_val = (uint32)entry->type;
    if ((ret = table_field_set(NEXT_HOP_TABLEt, NEXT_HOP_TABLE_TYPEtf, &tmp_val, (uint32 *) &nh_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    tmp_val = (uint32)entry->ifIdx;
    if ((ret = table_field_set(NEXT_HOP_TABLEt, NEXT_HOP_TABLE_IFIDXtf, &tmp_val, (uint32 *) &nh_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_write(NEXT_HOP_TABLEt, idx, (uint32 *)&nh_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;

}/* end of dal_apollo_l34_nexthopTable_set */



/* Function Name:
 *      dal_apollo_l34_nexthopTable_get
 * Description:
 *      Get a nexthop entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_nexthopTable_get(uint32 idx, rtk_l34_nexthop_entry_t *entry)
{
    int32 ret;
    l34_nexthop_entry_t nh_entry;
    uint32 tmp_val;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_NH_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    
    osal_memset(&nh_entry, 0x0, sizeof(nh_entry));
    
    if ((ret = table_read(NEXT_HOP_TABLEt, idx, (uint32 *)&nh_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_field_get(NEXT_HOP_TABLEt, NEXT_HOP_TABLE_NH_L2IDXtf, (uint32 *)&(tmp_val), (uint32 *) &nh_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->nhIdx = tmp_val;

    if ((ret = table_field_get(NEXT_HOP_TABLEt, NEXT_HOP_TABLE_PPPIDXtf, (uint32 *)&(tmp_val), (uint32 *) &nh_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->pppoeIdx = tmp_val;

    if ((ret = table_field_get(NEXT_HOP_TABLEt, NEXT_HOP_TABLE_IFIDXtf, (uint32 *)&(tmp_val), (uint32 *) &nh_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->ifIdx = tmp_val;

    if ((ret = table_field_get(NEXT_HOP_TABLEt, NEXT_HOP_TABLE_TYPEtf, (uint32 *)&(tmp_val), (uint32 *) &nh_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->type = tmp_val;

    return RT_ERR_OK;
} /* end of dal_apollo_l34_nexthopTable_get */




/* Function Name:
 *      dal_apollo_l34_extIntIPTable_set
 * Description:
 *      Set external internal IP entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_extIntIPTable_set(uint32 idx, rtk_l34_ext_intip_entry_t *entry)
{
    int32 ret;
    l34_extip_entry_t extip_entry;
    uint32 tmp_val;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_EXTIP_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((entry->valid != 0 && entry->valid != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_L34_NH_ENTRY_MAX()<=entry->nhIdx), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((entry->prival != 0 && entry->prival != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->type >= L34_EXTIP_TYPE_END ), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->pri >= RTK_MAX_NUM_OF_PRIORITY ), RT_ERR_INPUT);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    tmp_val = (uint32)entry->pri;
    osal_memset(&extip_entry, 0x0, sizeof(extip_entry));
    
    if ((ret = table_field_set(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_PRIORITYtf, &tmp_val, (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    tmp_val = (uint32)entry->prival;
    if ((ret = table_field_set(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_PRI_VALIDtf, &tmp_val, (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    tmp_val = (uint32)entry->nhIdx;
    if ((ret = table_field_set(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_NH_IDXtf, &tmp_val, (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    tmp_val = (uint32)entry->type;
    if ((ret = table_field_set(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_TYPEtf, &tmp_val, (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    tmp_val = (uint32)entry->valid;
    if ((ret = table_field_set(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_VALIDtf, &tmp_val, (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    tmp_val = (uint32)entry->extIpAddr;
    if ((ret = table_field_set(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_EXT_IPtf, &tmp_val, (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    tmp_val = (uint32)entry->intIpAddr;
    if ((ret = table_field_set(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_INT_IPtf, &tmp_val, (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_write(EXTERNAL_IP_TABLEt, idx, (uint32 *)&extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
} /* end of dal_apollo_l34_extIntIPTable_set */



/* Function Name:
 *      dal_apollo_l34_extIntIPTable_get
 * Description:
 *      Get a external internal IP entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT 
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32
dal_apollo_l34_extIntIPTable_get(uint32 idx, rtk_l34_ext_intip_entry_t *entry)
{
    int32 ret;
    l34_extip_entry_t extip_entry;
    uint32 tmp_val;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_EXTIP_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    
    osal_memset(&extip_entry, 0x0, sizeof(extip_entry));
    
    if ((ret = table_read(EXTERNAL_IP_TABLEt, idx, (uint32 *)&extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_field_get(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_PRIORITYtf, (uint32 *)&(tmp_val), (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->pri = tmp_val;

    if ((ret = table_field_get(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_PRI_VALIDtf, (uint32 *)&(tmp_val), (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->prival = tmp_val;

    if ((ret = table_field_get(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_NH_IDXtf, (uint32 *)&(tmp_val), (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->nhIdx = tmp_val;

    if ((ret = table_field_get(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_TYPEtf, (uint32 *)&(tmp_val), (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->type = tmp_val;

    if ((ret = table_field_get(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_VALIDtf, (uint32 *)&(tmp_val), (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->valid = tmp_val;

    if ((ret = table_field_get(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_EXT_IPtf, (uint32 *)&(tmp_val), (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->extIpAddr = tmp_val;

    if ((ret = table_field_get(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_INT_IPtf, (uint32 *)&(tmp_val), (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->intIpAddr = tmp_val;

    return RT_ERR_OK;
} /* end of dal_apollo_l34_extIntIPTable_get */



/* Function Name:
 *      dal_apollo_l34_extIntIPTable_del
 * Description:
 *      delete a external internal IP entry by idx.
 * Input:
 *      None
 * Output:
 *      idx 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_extIntIPTable_del(uint32 idx)
{
    int32 ret;
    l34_extip_entry_t extip_entry;
    uint32 is_valid=0;
    RT_PARAM_CHK((HAL_L34_EXTIP_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    /* check Init status */
    RT_INIT_CHK(l34_init);


    osal_memset(&extip_entry, 0x0, sizeof(extip_entry));

    if ((ret = table_field_set(EXTERNAL_IP_TABLEt, EXTERNAL_IP_TABLE_VALIDtf, (uint32 *)&is_valid, (uint32 *) &extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    if ((ret = table_write(EXTERNAL_IP_TABLEt, idx, (uint32 *)&extip_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;

}/* end of dal_apollo_l34_extIntIPTable_del */


/* Function Name:
 *      dal_apollo_l34_naptRemHash_get
 * Description:
 *      Get a hash index by source IP and source port.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      hash_value 
 * Note:
 *      None
 */
uint32
dal_apollo_l34_naptRemHash_get(uint32 sip, uint32 sport)
{
	uint16 hash_value=0;

	hash_value = ((sip&0xffff) ^ ((sip>>16)&0xffff) ^ (sport));

	return hash_value;
}/* end of dal_apollo_l34_naptRemHash_get */



/* Function Name:
 *      dal_apollo_l34_naptInboundHashidx_get
 * Description:
 *      Get Inband NAPT hash index.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      hash idex
 * Note:
 *      None
 */
uint32 
dal_apollo_l34_naptInboundHashidx_get(uint32 dip, uint16 dport, uint16 isTCP)
{
    /* Hashing Algorithm: 
        DIP[7:0] ^ DIP[15:8] ^ DIP[23:16] ^ DIP[31:24] ^ DPOR[7:0] ^ DPORT[15:8] ^ ( TCP << 7)
    */
    uint32 eidx=0;

    eidx = ((dip&0xff) ^ ((dip>>8)&0xff) ^ ((dip>>16)&0xff) ^ ((dip>>24)&0xff) ^ (dport&0xff) ^ ((dport>>8)&0xff) ^ (isTCP << 7));

    return eidx;

}


/* Function Name:
 *      dal_apollo_l34_naptInboundTable_set
 * Description:
 *      Set Inband NAPT table by idx.
 * Input:
 *      None
 * Output:
 *      forced : 0:do not add this entry if collection  1:farce overwrite this entry
 *      idx    : entry index
 *      entry  : entry content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_naptInboundTable_set(int8 forced, uint32 idx,rtk_l34_naptInbound_entry_t *entry)
{
    int32 ret;
    l34_napt_inband_entry_t naptInband_entry;
    uint32 tmp_val;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_NAPTR_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((forced != 0 && forced != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->isTcp != 0 && entry->isTcp != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->valid >=4), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->priValid != 0 && entry->priValid != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->priId >= RTK_MAX_NUM_OF_PRIORITY), RT_ERR_PRIORITY);
    RT_PARAM_CHK((entry->extIpIdx >= HAL_L34_EXTIP_ENTRY_MAX()), RT_ERR_INPUT);


    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* check if the index is valid*/
    if(forced == 0)
    {
        osal_memset(&naptInband_entry, 0x0, sizeof(naptInband_entry));
        if ((ret = table_read(NAPTR_TABLEt, idx, (uint32 *)&naptInband_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        /*get valid field*/
        if ((ret = table_field_get(NAPTR_TABLEt, NAPTR_TABLE_VALIDtf, (uint32 *)&(tmp_val), (uint32 *) &naptInband_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        if(tmp_val == 1)
        {
            return RT_ERR_FAILED;
        }
    }


    osal_memset(&naptInband_entry, 0x0, sizeof(naptInband_entry));
    
	tmp_val = entry->priId;
    if ((ret = table_field_set(NAPTR_TABLEt, NAPTR_TABLE_PRIORITYtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

	tmp_val = entry->priValid;
    if ((ret = table_field_set(NAPTR_TABLEt, NAPTR_TABLE_PRI_VALIDtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

	tmp_val = entry->valid;
    if ((ret = table_field_set(NAPTR_TABLEt, NAPTR_TABLE_VALIDtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

	tmp_val = entry->isTcp;
    if ((ret = table_field_set(NAPTR_TABLEt, NAPTR_TABLE_TCPtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

	tmp_val = entry->extPortLSB;
    if ((ret = table_field_set(NAPTR_TABLEt, NAPTR_TABLE_EXTPORT_LSBtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

	tmp_val = entry->extIpIdx;
    if ((ret = table_field_set(NAPTR_TABLEt, NAPTR_TABLE_EXT_IP_IDXtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

	tmp_val = entry->remHash;
    if ((ret = table_field_set(NAPTR_TABLEt, NAPTR_TABLE_REMOTE_HASHtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

	tmp_val = entry->intPort;
    if ((ret = table_field_set(NAPTR_TABLEt, NAPTR_TABLE_INT_PORTtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

	tmp_val = entry->intIp;
    if ((ret = table_field_set(NAPTR_TABLEt, NAPTR_TABLE_INT_IPtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }


    if ((ret = table_write(NAPTR_TABLEt, idx, (uint32 *)&naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
}/* end of dal_apollo_l34_naptInboundTable_set */






/* Function Name:
 *      dal_apollo_l34_naptInboundTable_get
 * Description:
 *      Get Inband NAPT table by idx.
 * Input:
 *      None
 * Output:
 *      idx    : entry index
 *      entry  : entry content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_naptInboundTable_get(uint32 idx,rtk_l34_naptInbound_entry_t *entry)
{
    int32 ret;
    l34_napt_inband_entry_t naptInband_entry;
    uint32 tmp_val;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_NAPTR_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
 
    /* check Init status */
    RT_INIT_CHK(l34_init);


    osal_memset(&naptInband_entry, 0x0, sizeof(naptInband_entry));

    if ((ret = table_read(NAPTR_TABLEt, idx, (uint32 *)&naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    if ((ret = table_field_get(NAPTR_TABLEt, NAPTR_TABLE_PRIORITYtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->priId = tmp_val; 

    if ((ret = table_field_get(NAPTR_TABLEt, NAPTR_TABLE_PRI_VALIDtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->priValid = tmp_val; 

    if ((ret = table_field_get(NAPTR_TABLEt, NAPTR_TABLE_VALIDtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->valid = tmp_val; 
    
    if ((ret = table_field_get(NAPTR_TABLEt, NAPTR_TABLE_TCPtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->isTcp = tmp_val; 

    if ((ret = table_field_get(NAPTR_TABLEt, NAPTR_TABLE_EXTPORT_LSBtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->extPortLSB = tmp_val; 

    if ((ret = table_field_get(NAPTR_TABLEt, NAPTR_TABLE_EXT_IP_IDXtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->extIpIdx = tmp_val; 

    if ((ret = table_field_get(NAPTR_TABLEt, NAPTR_TABLE_REMOTE_HASHtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->remHash= tmp_val; 

    if ((ret = table_field_get(NAPTR_TABLEt, NAPTR_TABLE_INT_PORTtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->intPort= tmp_val; 

    if ((ret = table_field_get(NAPTR_TABLEt, NAPTR_TABLE_INT_IPtf, (uint32 *)&tmp_val, (uint32 *) &naptInband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->intIp = tmp_val; 

    return RT_ERR_OK;
}/* end of dal_apollo_l34_naptInboundTable_get */






/* Function Name:
 *      dal_apollo_l34_naptOutboundHashidx_get
 * Description:
 *      Get Outband NAPT hash index.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      hash idex
 * Note:
 *      None
 */
uint32 
dal_apollo_l34_naptOutboundHashidx_get(int8 isTCP, uint32 sip, uint16 sport, uint32 dip, uint16 dport)
{
    /* Hashing Algorithm: 
        X[15:0] = SIP[15:0] ^ SIP[31:16] ^ SPORT[15:0] ^ DIP[15:0] ^ DIP[31:16] ^ DPORT[15:0]
        NAPT 5-tuple hash ID[8:0] = X[8:0] ^ { 2'b0, X[15:9] } ^ ( TCP << 8)
    */
    uint16 eidx_16;
    uint32 eidx_9;

    eidx_16 = (sip&0xff) ^ ((sip&0xff00)>>16) ^ (sport) ^ (dip&0xff) ^ ((dip&0xff00)>>16)^dport;
    eidx_9 = (eidx_16 & 0x1ff) ^ ((eidx_16 & 0xfe)>>9) ^(isTCP<<8) ;

    return eidx_9;


}


/* Function Name:
 *      dal_apollo_l34_naptOutboundTable_set
 * Description:
 *      Set Outband NAPT table by idx.
 * Input:
 *      None
 * Output:
 *      forced : 0:do not add this entry if collection  1:farce overwrite this entry
 *      idx    : entry index
 *      entry  : entry content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_naptOutboundTable_set(int8 forced, uint32 idx,rtk_l34_naptOutbound_entry_t *entry)
{
    int32 ret;
    l34_napt_inband_entry_t naptOnband_entry;
    uint32 tmp_val;

    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_NAPT_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);
    RT_PARAM_CHK((forced != 0 && forced != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->valid != 0 && entry->valid != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->priValid != 0 && entry->priValid != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((entry->priValue >= RTK_MAX_NUM_OF_PRIORITY), RT_ERR_PRIORITY);
    RT_PARAM_CHK((entry->hashIdx >=1024), RT_ERR_INPUT);



    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* check if the index is valid*/
    if(forced == 0)
    {
        osal_memset(&naptOnband_entry, 0x0, sizeof(naptOnband_entry));
        if ((ret = table_read(NAPT_TABLEt, idx, (uint32 *)&naptOnband_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        /*get valid field from napt table*/
        if ((ret = table_field_get(NAPT_TABLEt, NAPT_TABLE_VALIDtf, (uint32 *)&(tmp_val), (uint32 *) &naptOnband_entry)) != RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return RT_ERR_FAILED;
        }
        if(tmp_val == 1)
        {
            return RT_ERR_FAILED;
        }
    }


    osal_memset(&naptOnband_entry, 0x0, sizeof(naptOnband_entry));
    
    tmp_val = (uint32)entry->valid;
    if ((ret = table_field_set(NAPT_TABLEt, NAPT_TABLE_VALIDtf, &tmp_val, (uint32 *) &naptOnband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    tmp_val = (uint32)entry->hashIdx;
    if ((ret = table_field_set(NAPT_TABLEt, NAPT_TABLE_HASH_IDXtf, &tmp_val, (uint32 *) &naptOnband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }

    tmp_val = (uint32)entry->priValid;
    if ((ret = table_field_set(NAPT_TABLEt, NAPT_TABLE_PRI_VALIDtf, &tmp_val, (uint32 *) &naptOnband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    tmp_val = (uint32)entry->priValue;
    if ((ret = table_field_set(NAPT_TABLEt, NAPT_TABLE_PRItf, &tmp_val, (uint32 *) &naptOnband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }    
 
    if ((ret = table_write(NAPT_TABLEt, idx, (uint32 *)&naptOnband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    return RT_ERR_OK;
}/* end of dal_apollo_l34_naptOutboundTable_set */

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
int32
dal_apollo_l34_hsabMode_set(rtk_l34_hsba_mode_t hsabMode)
{
	int32 ret;
	apollo_raw_l34_hsba_mode_t mode;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "hsabMode=%d",hsabMode);

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((L34_HSBA_END <=hsabMode), RT_ERR_INPUT);

	_dal_apollo_l34_hsabMode_cfg2raw(hsabMode,&mode);

	/* function body */
	if((ret = apollo_raw_l34_hsba_mode_set(mode))!=RT_ERR_OK)
	{
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x", ret);
	    	return ret;
	}


	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_hsabMode_set */


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
int32
dal_apollo_l34_hsabMode_get(rtk_l34_hsba_mode_t *pHsabMode)
{
	int32 ret;
	apollo_raw_l34_hsba_mode_t mode;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "");

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == pHsabMode), RT_ERR_NULL_POINTER);

	/* function body */
	if((ret = apollo_raw_l34_hsba_mode_get(&mode))!=RT_ERR_OK)
	{
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x", ret);
	    	return ret;
	}
	_dal_apollo_l34_hsabMode_raw2cfg(mode,pHsabMode);


	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_hsabMode_get */




/* Function Name:
 *      dal_apollo_l34_naptOutboundTable_get
 * Description:
 *      Set Outband NAPT table by idx.
 * Input:
 *      None
 * Output:
 *      idx    : entry index
 *      entry  : entry content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_apollo_l34_naptOutboundTable_get(uint32 idx,rtk_l34_naptOutbound_entry_t *entry)
{
    int32 ret;
    l34_napt_inband_entry_t naptOnband_entry;
    uint32 tmp_val;
    RT_PARAM_CHK((entry==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_L34_NAPT_ENTRY_MAX()<=idx), RT_ERR_ENTRY_INDEX);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    osal_memset(&naptOnband_entry, 0x0, sizeof(naptOnband_entry));

    if ((ret = table_read(NAPT_TABLEt, idx, (uint32 *)&naptOnband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    
    if ((ret = table_field_get(NAPT_TABLEt, NAPT_TABLE_VALIDtf, &tmp_val, (uint32 *) &naptOnband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->valid = tmp_val;
    

    if ((ret = table_field_get(NAPT_TABLEt, NAPT_TABLE_HASH_IDXtf, &tmp_val, (uint32 *) &naptOnband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->hashIdx = tmp_val;
    
    if ((ret = table_field_get(NAPT_TABLEt, NAPT_TABLE_PRI_VALIDtf, &tmp_val, (uint32 *) &naptOnband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }
    entry->priValid = tmp_val;
    

    if ((ret = table_field_get(NAPT_TABLEt, NAPT_TABLE_PRItf, &tmp_val, (uint32 *) &naptOnband_entry)) != RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return RT_ERR_FAILED;
    }    
    entry->priValue = tmp_val;
     

    
    return RT_ERR_OK;
}/* end of dal_apollo_l34_naptOutboundTable_get */



/* Function Name:
 *      dal_apollo_l34_table_reset
 * Description:
 *      Reset specific L34 table.
 * Input:
 *      None
 * Output:
 *      type : L34 table type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 
dal_apollo_l34_table_reset(rtk_l34_table_type_t type)
{
    int32 ret;
    uint32 reg_field;
    uint32 reset_status,busy;
    /*get reset table type*/
    switch(type)
    {
        case L34_ROUTING_TABLE:
            reg_field = RST_L3f;    
            break;
        case L34_PPPOE_TABLE:
            reg_field = RST_PPf;    
            break;
        case L34_NEXTHOP_TABLE:
            reg_field = RST_NHf;    
            break;
        case L34_NETIF_TABLE:
            reg_field = RST_IFf;    
            break;
        case L34_INTIP_TABLE:
            reg_field = RST_IPf;    
            break;
        case L34_ARP_TABLE:
            reg_field = RST_ARPf;    
            break;
        case L34_NAPTR_TABLE:
            reg_field = RST_NAPTRf;    
            break;        
        case L34_NAPT_TABLE:
            reg_field = RST_NAPTf;    
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }
    /*get register*/
    if ((ret = reg_field_read(NAT_TBL_ACCESS_CLRr,reg_field,&reset_status))!=RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return ret;
    }
    if(reset_status == 0)
    {/*table is not perform reset, we reset it*/
        reset_status = 1;
        if ((ret = reg_field_write(NAT_TBL_ACCESS_CLRr,reg_field,&reset_status))!=RT_ERR_OK)
        {
            RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
            return ret;
        }
    }
    
    /* Wait operation completed */
    do
    {
        if ((ret = reg_field_read(NAT_TBL_ACCESS_CLRr, reg_field, &busy)) != RT_ERR_OK)
        {
            return ret;
        }
    } while (busy);

    return RT_ERR_OK;
}/* end of dal_apollo_l34_table_reset */




/* Function Name:
 *      dal_apollo_l34_hsb_set
 * Description:
 *      set hsb value.
 * Input:
 *      hsb : hsb content
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
int32 
dal_apollo_l34_hsb_set(apollo_l34_hsb_param_t *hsb)
{
    int32 ret;
    if ((ret = reg_write(HSB_DESCr, (uint32 *)hsb->hsbWords))!=RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return ret;
    }
    return RT_ERR_OK;   

}/* end of dal_apollo_l34_hsb_set */



/* Function Name:
 *      dal_apollo_l34_hsb_get
 * Description:
 *      get data from hsb.
 * Input:
 *      None
 * Output:
 *      hsb : hsb content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 
dal_apollo_l34_hsb_get(apollo_l34_hsb_param_t *hsb)
{
    int32 ret;

    if ((ret = reg_read(HSB_DESCr, (uint32 *)hsb->hsbWords))!=RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return ret;
    }
    return RT_ERR_OK;   


}/* end of dal_apollo_l34_hsb_get */





/* Function Name:
 *      dal_apollo_l34_hsa_set
 * Description:
 *      set hsa value.
 * Input:
 *      hsa : hsa content
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
int32 
dal_apollo_l34_hsa_set(apollo_l34_hsa_param_t *hsa)
{
    int32 ret;
    if ((ret = reg_write(HSA_DESCr, (uint32 *)hsa->hsaWords))!=RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return ret;
    }
    return RT_ERR_OK;   

}/* end of dal_apollo_l34_hsa_set */



/* Function Name:
 *      dal_apollo_l34_hsa_get
 * Description:
 *      get data from hsa.
 * Input:
 *      None
 * Output:
 *      hsb : hsa content
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 
dal_apollo_l34_hsa_get(apollo_l34_hsa_param_t *hsa)
{
    int32 ret;

    if ((ret = reg_read(HSA_DESCr, (uint32 *)hsa->hsaWords))!=RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return ret;
    }
    return RT_ERR_OK;   


}/* end of dal_apollo_l34_hsa_get */

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
int32
dal_apollo_l34_hsaData_get(rtk_l34_hsa_t *pHsaData)
{
	int32 ret;
	apollo_raw_l34_hsa_t entry;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "");

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == pHsaData), RT_ERR_NULL_POINTER);

	/* function body */
	if((ret = apollo_raw_l34_hsaData_get(&entry))!=RT_ERR_OK)
	{
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x", ret);
	    	return ret;
	}

	pHsaData->action = entry.action;
	pHsaData->pppid_idx= entry.pppid_idx;
	pHsaData->pppoe_if= entry.pppoe_if;
	pHsaData->reason= entry.reason;
	pHsaData->difid= entry.difid;
	pHsaData->dvid= entry.dvid;
	pHsaData->frag = entry.frag;
	pHsaData->ip = entry.ip;
	pHsaData->l2trans = entry.l2trans;
	pHsaData->l34trans = entry.l34trans;
	pHsaData->l3_chksum = entry.l3_chksum;
	pHsaData->l4_chksum = entry.l4_chksum;
	pHsaData->l4_pri_sel = entry.l4_pri_sel;
	pHsaData->l4_pri_valid = entry.l4_pri_valid;
	pHsaData->nexthop_mac_idx = entry.nexthop_mac_idx;
	pHsaData->port = entry.port;

	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_hsaData_get */


/* Function Name:
 *      dal_apollomp_l34_hsbData_get
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
int32
dal_apollo_l34_hsbData_get(rtk_l34_hsb_t *pHsbData)
{
    	int32 ret;
	apollo_raw_l34_hsb_t entry;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "");

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == pHsbData), RT_ERR_NULL_POINTER);

	/* function body */
	if((ret = apollo_raw_l34_hsbData_get(&entry))!=RT_ERR_OK)
	{
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x", ret);
	    	return ret;
	}

	pHsbData->cpu_direct_tx = entry.cpu_direct_tx;
	pHsbData->cvid= entry.cvid;
	pHsbData->cvlan_if= entry.cvlan_if;
	pHsbData->dmac= entry.dmac;
	pHsbData->dport_l4chksum= entry.dport_l4chksum;
	pHsbData->ipfrag_s= entry.ipfrag_s;
	pHsbData->ipmf= entry.ipmf;
	pHsbData->l2bridge = entry.l2bridge;
	pHsbData->l3_chksum_ok = entry.l3_chksum_ok;
	pHsbData->l4_chksum_ok = entry.l4_chksum_ok;
	pHsbData->len = entry.len;
	pHsbData->parse_fail = entry.parse_fail;
	pHsbData->pppoe_id = entry.pppoe_id;
	pHsbData->pppoe_if = entry.pppoe_if;
	memcpy(&(pHsbData->sip.ipv6_addr[12]),&entry.sip,4);
	pHsbData->sport_icmpid_chksum = entry.sport_icmpid_chksum;
	pHsbData->svlan_if = entry.svlan_if;
	pHsbData->tcp_flag = entry.tcp_flag;
	pHsbData->ttls = entry.ttls;
	pHsbData->type = entry.type;
	pHsbData->udp_no_chksum = entry.udp_no_chksum;



    return RT_ERR_OK;
}   /* end of dal_apollomp_l34_hsbData_get */



/* Function Name:
 *      dal_apollo_l34_hsabCtrMode_set
 * Description:
 *      Set L34 HSAB log mode.
 * Input:
 *      None
 * Output:
 *      mode : L34 HSAB log mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 
dal_apollo_l34_hsabCtrMode_set(apollo_l34_hsab_mode_t mode)
{
    int32 ret;
    if ((ret = reg_field_write(HSBA_CTRLr,TST_LOG_MDf,(uint32 *)&mode))!=RT_ERR_OK)
    {
        RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);  
        return ret;
    }
    return RT_ERR_OK;
}/* end of dal_apollo_l34_hsabCtrMode_set */


/* Sub-module Name: System configuration */

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
int32
dal_apollo_l34_globalState_set(rtk_l34_globalStateType_t stateType,rtk_enable_t state)
{

    	int32 ret;
 	
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "stateType=%d,state=%d",stateType, state);

   	/* check Init status */
    	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((L34_GLOBAL_STATE_END <=stateType), RT_ERR_INPUT);
	RT_PARAM_CHK((RTK_ENABLE_END <= state), RT_ERR_INPUT);

	switch(stateType){
	case L34_GLOBAL_L34_STATE:
		if((ret=apollo_raw_l34_globalFunction_set(state))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;
	case L34_GLOBAL_L3NAT_STATE:
		if((ret= l34_mode_l3_set(state))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;
	case L34_GLOBAL_L4NAT_STATE:
		if((ret= l34_mode_l4_set(state))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;
	case L34_GLOBAL_L3CHKSERRALLOW_STATE:
		if((ret=apollo_raw_l34_L3chksumErrAllow_set(state))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;
	case L34_GLOBAL_L4CHKSERRALLOW_STATE:
		if((ret=apollo_raw_l34_L4chksumErrAllow_set(state))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;
	case L34_GLOBAL_TTLMINUS_STATE:
		if((ret=apollo_raw_l34_TtlOperationMode_set(state))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;
	case L34_GLOBAL_NAT2LOG_STATE:
		if((ret=apollo_raw_l34_natAttack2CPU_set(state))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;
	case L34_GLOBAL_FRAG2CPU_STATE:
		if((ret=apollo_raw_l34_FragPkt2Cpu_set(state))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;  
	default:
		return RT_ERR_INPUT;
	break;
	}

	return RT_ERR_OK;

}   /* end of dal_apollomp_l34_globalState_set */


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
int32
dal_apollo_l34_globalState_get(rtk_l34_globalStateType_t stateType,rtk_enable_t *pState)
{
	
    	int32 ret;
	apollo_raw_l34_mode_t l34mode;
 
    	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "stateType=%d",stateType);

    	/* check Init status */
    	RT_INIT_CHK(l34_init);

    	/* parameter check */
    	RT_PARAM_CHK((L34_GLOBAL_STATE_END <=stateType), RT_ERR_INPUT);
    	RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

	switch(stateType){
	case L34_GLOBAL_L34_STATE:
	if((ret=apollo_raw_l34_globalFunction_get(pState)) != RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	break;
	case L34_GLOBAL_L3NAT_STATE:
	if((ret=apollo_raw_l34_mode_get(&l34mode))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	*pState = l34mode & 1;
	break;
	case L34_GLOBAL_L4NAT_STATE:
	if((ret=apollo_raw_l34_mode_get(&l34mode))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	*pState = (l34mode >> 1) & 1;
	break;
	case L34_GLOBAL_L3CHKSERRALLOW_STATE:
	if((ret=apollo_raw_l34_L3chksumErrAllow_get(pState))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	break;
	case L34_GLOBAL_L4CHKSERRALLOW_STATE:
	if((ret=apollo_raw_l34_L4chksumErrAllow_get(pState))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	break;
	case L34_GLOBAL_NAT2LOG_STATE:
	if((ret=apollo_raw_l34_natAttack2CPU_get(pState))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	break;
	case L34_GLOBAL_FRAG2CPU_STATE:
	if((ret=apollo_raw_l34_FragPkt2Cpu_get(pState))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	break;
	case L34_GLOBAL_TTLMINUS_STATE:
	if((ret=apollo_raw_l34_TtlOperationMode_get(pState))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	break;  
	default:
		return RT_ERR_INPUT;
	break;
	}

    return RT_ERR_OK;
}   /* end of dal_apollomp_l34_globalState_get */



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
int32
dal_apollo_l34_lookupMode_set(rtk_l34_lookupMode_t lookupMode)
{
    	int32 ret;

	apollo_raw_l34_limbc_t mode;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "lookupMode=%d",lookupMode);

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((L34_LOOKUP_MODE_END <=lookupMode), RT_ERR_INPUT);

	if((ret= l34_lookupMode_cfg2raw(lookupMode,&mode)) != RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	if((ret = apollo_raw_l34_LanIntfMDBC_set(mode)) != RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	return RT_ERR_OK;

}   /* end of dal_apollomp_l34_lookupMode_set */


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
int32
dal_apollo_l34_lookupMode_get(rtk_l34_lookupMode_t *pLookupMode)
{
	int32 ret;
	apollo_raw_l34_limbc_t mode;

	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "");

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == pLookupMode), RT_ERR_NULL_POINTER);

	if((ret = apollo_raw_l34_LanIntfMDBC_get(&mode)) != RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	if((ret = l34_lookupMode_raw2cfg(mode,pLookupMode)) != RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
    return RT_ERR_OK;
}   /* end of dal_apollomp_l34_lookupMode_get */



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
int32
dal_apollo_l34_lookupPortMap_set(rtk_l34_portType_t portType, uint32 portId, uint32 wanIdx)
{
      int32 ret;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "portType=%d,portId=%d,wanIdx=%d",portType, portId, wanIdx);

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((L34_PORT_TYPE_END <=portType), RT_ERR_INPUT);
	switch(portType){
	case L34_PORT_MAC:
		RT_PARAM_CHK((L34_MAC_PORT_MAX <=portId), RT_ERR_INPUT);
	break;
	case L34_PORT_EXTENSION:
		RT_PARAM_CHK((L34_EXT_PORT_MAX<=portId), RT_ERR_INPUT);
	break;
	case L34_PORT_DSLVC:
		RT_PARAM_CHK((L34_DSLVC_PORT_MAX <=portId), RT_ERR_INPUT);
	default:
	break;
	}
	RT_PARAM_CHK((HAL_L34_NETIF_ENTRY_MAX() <=wanIdx), RT_ERR_INPUT);

	/* function body */
	switch(portType){
	case L34_PORT_MAC:
		if((ret =apollo_raw_l34_portIntfIdx_set(portId,wanIdx))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;
	case L34_PORT_EXTENSION:
		if((ret =apollo_raw_l34_extPortIntfIdx_set(portId,wanIdx))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;
	case L34_PORT_DSLVC:
		if((ret =apollo_raw_l34_vcPortIntfIdx_set(portId,wanIdx))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	break;
	default:
		return RT_ERR_FAILED;
	break;
	}
	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_lookupPortMap_set */


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
int32
dal_apollo_l34_lookupPortMap_get(rtk_l34_portType_t portType, uint32 portId, uint32 *pWanIdx)
{
	int32 ret;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "portType=%d,portId=%d",portType, portId);

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((L34_PORT_TYPE_END <=portType), RT_ERR_INPUT);
	RT_PARAM_CHK((NULL == pWanIdx), RT_ERR_NULL_POINTER);

      if((ret = l34_portMapNetif_get(portType,pWanIdx,portId)) != RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	
    return RT_ERR_OK;
}   /* end of dal_apollomp_l34_lookupPortMap_get */


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
int32
dal_apollo_l34_wanRoutMode_set(rtk_l34_wanRouteMode_t wanRouteMode)
{
    	int32 ret;
	rtk_l34_wanroute_act_t act;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "wanRouteMode=%d",wanRouteMode);

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((L34_WANROUTE_END <=wanRouteMode), RT_ERR_INPUT);

	/* function body */
	if((ret = l34_wanroute_cfg2raw(wanRouteMode,&act)) != RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	if((ret = apollo_raw_l34_wanRouteAct_set(act)) != RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_wanRoutMode_set */


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
int32
dal_apollo_l34_wanRoutMode_get(rtk_l34_wanRouteMode_t *pWanRouteMode)
{
	int32 ret;
	rtk_l34_wanroute_act_t act;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "");

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == pWanRouteMode), RT_ERR_NULL_POINTER);

	if((ret = apollo_raw_l34_wanRouteAct_get(&act)) != RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	if((ret = l34_wanroute_cfg2raw(act,pWanRouteMode)) != RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_wanRoutMode_get */


/* Function Name:
 *      dal_apollomp_l34_arpTrfIndicator_get
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
int32
dal_apollo_l34_arpTrfIndicator_get(uint32 index, rtk_enable_t *pArpIndicator)
{
	int32 ret;
	int i;
	apollo_raw_l34_arp_trf_t newTable,oldTable;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "index=%d",index);

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((HAL_L34_ARP_ENTRY_MAX()+1<index), RT_ERR_INPUT);
	RT_PARAM_CHK((NULL == pArpIndicator), RT_ERR_NULL_POINTER);

	/*get current work table*/
	if((ret=apollo_raw_l34_hwArpTrfWrkTbl_get(&oldTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	newTable = !oldTable;
	/*clear new table first*/
	if((ret=apollo_raw_l34_hwArpTrfWrkTbl_Clear(newTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	/*swap*/
	if((ret=apollo_raw_l34_hwArpTrfWrkTbl_set(newTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	/*get*/
	if(index==L34_TF_ARP_ALL){
		for(i=0;i<HAL_L34_ARP_ENTRY_MAX();i++){
			if((ret=apollo_raw_l34_arpTrfTb_get( oldTable, i, &pArpIndicator[i]))!=RT_ERR_OK){
				RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
				return ret;
			}
		}
	}else{
		if((ret=apollo_raw_l34_arpTrfTb_get(oldTable, index, pArpIndicator))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	}

	/*clear old table*/
	if((ret=apollo_raw_l34_hwArpTrfWrkTbl_Clear(oldTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_arpTrfIndicator_get */


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
int32
dal_apollo_l34_naptTrfIndicator_get(uint32 index, rtk_enable_t *pNaptIndicator)
{
	
	int32 ret,i;
	apollo_raw_l34_l4_trf_t newTable,oldTable;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "index=%d",index);

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((HAL_L34_NAPT_ENTRY_MAX()+1<index), RT_ERR_INPUT);
	RT_PARAM_CHK((NULL == pNaptIndicator), RT_ERR_NULL_POINTER);

	/*get current work table*/
	if((ret=apollo_raw_l34_hwL4TrfWrkTbl_get(&oldTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	newTable = !oldTable;
	/*clear new table first*/
	if((ret=apollo_raw_l34_hwL4TrfWrkTbl_Clear(newTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	/*swap*/
	if((ret=apollo_raw_l34_hwL4TrfWrkTbl_set(newTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	/*get*/
	if(index==L34_TF_NAT_ALL){
		for(i=0;i<HAL_L34_NAPT_ENTRY_MAX();i++){
			if((ret=apollo_raw_l34_l4TrfTb_get(oldTable,i,&pNaptIndicator[i]))!=RT_ERR_OK){
				RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
				return ret;
			}
		}

	}else{
		if((ret=apollo_raw_l34_l4TrfTb_get(oldTable,index,pNaptIndicator))!=RT_ERR_OK){
			RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
			return ret;
		}
	}

	/*clear old table*/
	if((ret=apollo_raw_l34_hwL4TrfWrkTbl_Clear(oldTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_naptTrfIndicator_get */



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
int32
dal_apollo_l34_pppTrfIndicator_get(uint32 index, rtk_enable_t *pPppIndicator)
{
	int32 ret;
	uint8 pppInd;
	int i;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "index=%d",index);

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((HAL_L34_PPPOE_ENTRY_MAX()+1<index), RT_ERR_INPUT);
	RT_PARAM_CHK((NULL == pPppIndicator), RT_ERR_NULL_POINTER);

	if((ret = apollo_raw_l34_pppoeTrf_get(&pppInd))!=RT_ERR_OK)
	{
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	if(index == L34_TF_PPP_ALL){
		for(i=0;i<HAL_L34_PPPOE_ENTRY_MAX();i++){
			pPppIndicator[i] = (pppInd & (1 << i)) ? ENABLED: DISABLED;
		}
	}else
	{
		*pPppIndicator = (pppInd & (1 << index)) ? ENABLED: DISABLED;
	}
	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_pppTrfIndicator_get */


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
int32
dal_apollo_l34_hsdState_set(rtk_enable_t hsdState)
{
    int32 ret;
    uint32 pValue;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "hsdState=%d",hsdState);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_ENABLE_END <= hsdState), RT_ERR_INPUT);

    /* function body */
    if(hsdState == ENABLED)
		pValue = 0xf5;
    else
		pValue = 0;

    if((ret = reg_field_write(RSVD_ALE_HSAr, RSVD_MEMf, &pValue))!=RT_ERR_OK)
    {
    		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
    }

    return RT_ERR_OK;
}   /* end of dal_apollo_l34_hsdState_set */


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
int32
dal_apollo_l34_hsdState_get(rtk_enable_t *phsdState)
{
    int32 ret;
    uint32 pValue;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "");

    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == phsdState), RT_ERR_NULL_POINTER);

    /* function body */
  if((ret = reg_field_read(RSVD_ALE_HSAr, RSVD_MEMf, &pValue))!=RT_ERR_OK)
    {
    		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
    }

    if(pValue == 0x5f)
		*phsdState = ENABLED;
    else
		*phsdState = DISABLED;
	
    return RT_ERR_OK;
}   /* end of dal_apollo_l34_hsdState_get */

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
 

int32 
dal_apollo_l34_hwL4TrfWrkTbl_set(rtk_l34_l4_trf_t l4TrfTable)
{
    int32 ret;
    apollo_raw_l34_l4_trf_t table;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "l4TrfTable=%d",l4TrfTable);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_L34_L4TRF_TABLE_END <=l4TrfTable), RT_ERR_INPUT);

    /* function body */
    l34_l4Trf_cfg2raw(l4TrfTable,&table);
    if((ret = apollo_raw_l34_hwL4TrfWrkTbl_set(table))!=RT_ERR_OK)
    {
    		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
    }
    return RT_ERR_OK;
}   /* end of dal_apollo_l34_hwL4TrfWrkTbl_set */


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
int32 
dal_apollo_l34_hwL4TrfWrkTbl_get(rtk_l34_l4_trf_t *pl4TrfTable)
{

    int32 ret;
    apollo_raw_l34_l4_trf_t table;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "");

    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pl4TrfTable), RT_ERR_NULL_POINTER);

    /* function body */
   if((ret = apollo_raw_l34_hwL4TrfWrkTbl_get(&table))!=RT_ERR_OK)
    {
    		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
    }
    l34_l4Trf_raw2cfg(table,pl4TrfTable);
    
    return RT_ERR_OK;
}   /* end of dal_apollo_l34_hwL4TrfWrkTbl_get */

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
int32 
dal_apollo_l34_l4TrfTb_get(rtk_l34_l4_trf_t l4TrfTable,uint32 l4EntryIndex,rtk_enable_t *pIndicator)
{

    int32 ret;
    apollo_raw_l34_l4_trf_t table;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "l4TrfTable=%d,l4EntryIndex=%d",l4TrfTable, l4EntryIndex);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_L34_L4TRF_TABLE_END <=l4TrfTable), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_L34_NAPT_ENTRY_MAX()<=l4EntryIndex), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pIndicator), RT_ERR_NULL_POINTER);

    /* function body */
    l34_l4Trf_cfg2raw(l4TrfTable,&table);
   if((ret = apollo_raw_l34_l4TrfTb_get(table,l4EntryIndex,pIndicator))!=RT_ERR_OK)
    {
    		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
    }
    return RT_ERR_OK;
}   /* end of dal_apollo_l34_l4TrfTb_get */


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
int32 
dal_apollo_l34_hwL4TrfWrkTbl_Clear(rtk_l34_l4_trf_t l4TrfTable)
{
    int32 ret;
    apollo_raw_l34_l4_trf_t table;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "l4TrfTable=%d",l4TrfTable);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_L34_L4TRF_TABLE_END <=l4TrfTable), RT_ERR_INPUT);

    /* function body */
   l34_l4Trf_cfg2raw(l4TrfTable,&table);
  if((ret = apollo_raw_l34_hwL4TrfWrkTbl_Clear(table))!=RT_ERR_OK)
    {
    		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
    }
    return RT_ERR_OK;
}   /* end of dal_apollo_l34_hwL4TrfWrkTbl_Clear */


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
int32 
dal_apollo_l34_hwArpTrfWrkTbl_set(rtk_l34_arp_trf_t arpTrfTable)
{

    int32 ret;
    apollo_raw_l34_arp_trf_t table;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "arpTrfTable=%d",arpTrfTable);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_L34_ARPTRF_TABLE_END <=arpTrfTable), RT_ERR_INPUT);

  /* function body */
    l34_arpTrf_cfg2raw(arpTrfTable,&table);
    if((ret = apollo_raw_l34_hwArpTrfWrkTbl_set(table))!=RT_ERR_OK)
    {
    		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
    }
    return RT_ERR_OK;
}   /* end of dal_apollo_l34_hwArpTrfWrkTbl_set */


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
int32 
dal_apollo_l34_hwArpTrfWrkTbl_get(rtk_l34_arp_trf_t *pArpTrfTable)
{

    int32 ret;
    apollo_raw_l34_arp_trf_t table;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "");

    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pArpTrfTable), RT_ERR_NULL_POINTER);

    /* function body */
  if((ret = apollo_raw_l34_hwArpTrfWrkTbl_get(&table))!=RT_ERR_OK)
    {
    		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
    }
    l34_arpTrf_raw2cfg(table,pArpTrfTable);
    return RT_ERR_OK;
}   /* end of dal_apollo_l34_hwArpTrfWrkTbl_get */

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
int32 
dal_apollo_l34_arpTrfTb_get(rtk_l34_arp_trf_t arpTrfTable,uint32 arpEntryIndex,rtk_enable_t *pIndicator)
{
    int32 ret;
    apollo_raw_l34_arp_trf_t table;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "arpTrfTable=%d,arpEntryIndex=%d",arpTrfTable, arpEntryIndex);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_L34_ARPTRF_TABLE_END <=arpTrfTable), RT_ERR_INPUT);
    RT_PARAM_CHK((HAL_L34_ARP_ENTRY_MAX()<=arpEntryIndex), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pIndicator), RT_ERR_NULL_POINTER);

    /* function body */
    l34_arpTrf_cfg2raw(arpTrfTable,&table);
   if((ret = apollo_raw_l34_arpTrfTb_get(table,arpEntryIndex,pIndicator))!=RT_ERR_OK)
    {
    		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
    }

    return RT_ERR_OK;
}   /* end of dal_apollo_l34_arpTrfTb_get */


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
int32 
dal_apollo_l34_hwArpTrfWrkTbl_Clear(rtk_l34_arp_trf_t arpTrfTable)
{
    int32 ret;
    apollo_raw_l34_arp_trf_t table;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "arpTrfTable=%d",arpTrfTable);

    /* check Init status */
    RT_INIT_CHK(l34_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_L34_ARPTRF_TABLE_END <=arpTrfTable), RT_ERR_INPUT);

    /* function body */
   l34_arpTrf_cfg2raw(arpTrfTable,&table);
   if((ret = apollo_raw_l34_hwArpTrfWrkTbl_Clear(table))!=RT_ERR_OK)
    {
    		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
    }
    return RT_ERR_OK;
}   /* end of dal_apollo_l34_hwArpTrfWrkTbl_Clear */


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
int32
dal_apollo_l34_naptTrfIndicator_get_all(uint32 *pNaptMaps)
{
	int32 ret,i;
	uint32 base,addr;
	apollo_raw_l34_l4_trf_t newTable,oldTable;

	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L34), "");

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == pNaptMaps), RT_ERR_NULL_POINTER);

	/*get current work table*/
	if((ret=apollo_raw_l34_hwL4TrfWrkTbl_get(&oldTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	newTable = !oldTable;
	/*clear new table first*/
	if((ret=apollo_raw_l34_hwL4TrfWrkTbl_Clear(newTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	/*swap*/
	if((ret=apollo_raw_l34_hwL4TrfWrkTbl_set(newTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	/*get*/
	if(oldTable == RAW_L34_L4TRF_TABLE0){
		base = HAL_GET_REG_ADDR(L4_TRF0r);
	}else{
		base = HAL_GET_REG_ADDR(L4_TRF1r);
	}
	/*get data, 64 word 32 bits traffic indicator*/
	for(i=0;i<(HAL_L34_NAPT_ENTRY_MAX()/32);i++){
		addr = base + i*4;
		if (ioal_mem32_read(addr, &pNaptMaps[i]) != RT_ERR_OK)
	    	{
	        	return RT_ERR_FAILED;
	    	}
	}

	/*clear old table*/
	if((ret=apollo_raw_l34_hwL4TrfWrkTbl_Clear(oldTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}


	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_naptTrfIndicator_get_all */


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
int32
dal_apollo_l34_arpTrfIndicator_get_all(uint32 *pArpMaps)
{
	int32 ret,i;
	uint32 base,addr;
	apollo_raw_l34_arp_trf_t newTable,oldTable;

	/* check Init status */
	RT_INIT_CHK(l34_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == pArpMaps), RT_ERR_NULL_POINTER);

	/*get current work table*/
	if((ret=apollo_raw_l34_hwArpTrfWrkTbl_get(&oldTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}
	newTable = !oldTable;
	/*clear new table first*/
	if((ret=apollo_raw_l34_hwArpTrfWrkTbl_Clear(newTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	/*swap*/
	if((ret=apollo_raw_l34_hwArpTrfWrkTbl_set(newTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	/*get*/
	if(oldTable == RAW_L34_ARPTRF_TABLE0){
		base = HAL_GET_REG_ADDR(ARP_TRF0r);
	}else{
		base = HAL_GET_REG_ADDR(ARP_TRF1r);
	}
	/*get data, 16 word 32 bits traffic indicator*/
	for(i=0;i<(HAL_L34_ARP_ENTRY_MAX()/32);i++){
		addr = base + i*4;
		if (ioal_mem32_read(addr, &pArpMaps[i]) != RT_ERR_OK)
	    	{
	        	return RT_ERR_FAILED;
	    	}
	}

	/*clear old table*/
	if((ret=apollo_raw_l34_hwArpTrfWrkTbl_Clear(oldTable))!=RT_ERR_OK){
		RT_LOG(LOG_DEBUG, (MOD_DAL|MOD_L34), "return failed ret value = %x",ret);
		return ret;
	}

	return RT_ERR_OK;
}   /* end of dal_apollomp_l34_arpTrfIndicator_get_all */



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
 * Purpose : switch asic-level SVLAN API 
 * Feature : The file have include the following module and sub-modules
 *           1) QinQ/SVLAN
 *
 */

#include <dal/apollo/raw/apollo_raw_svlan.h>


/* Function Name:
 *      apollo_raw_svlan_portUplinkEnable_set
 * Description:
 *      Set the uplink port status of the specific port
 * Input:
 *      port   		- port id
 *      enable 		- enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_PORT_ID       
 * Note:
 *      None
 */
int32 apollo_raw_svlan_portUplinkEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
	RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_array_field_write(SVLAN_UPLINK_PMSKr, port, REG_ARRAY_INDEX_NONE, ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_portUplinkEnable_set */


/* Function Name:
 *      apollo_raw_svlan_portUplinkEnable_get
 * Description:
 *      Get the uplink port status of the specific port
 * Input:
 *      port   			- port id
 * Output:
 *      pEnable 		- enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       
 *      RT_ERR_NULL_POINTER       
 * Note:
 *      None
 */
int32 apollo_raw_svlan_portUplinkEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;

    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(SVLAN_UPLINK_PMSKr, port, REG_ARRAY_INDEX_NONE, ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_portUplinkEnable_get */

/* Function Name:
 *      apollo_raw_svlan_port1tonVlanEnable_set
 * Description:
 *      Set the downstrean 1:N vlan status of the specific port
 * Input:
 *      port   		- port id
 *      enable 		- enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT       
 *      RT_ERR_PORT_ID       
 * Note:
 *      None
 */
int32 apollo_raw_svlan_port1tonVlanEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);
    
    if ((ret = reg_array_field_write(SVLAN_EP_DMAC_CTRLr, port , REG_ARRAY_INDEX_NONE, ENf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_port1tonVlanEnable_get */

/* Function Name:
 *      apollo_raw_svlan_port1tonVlanEnable_get
 * Description:
 *      Get the downstrean 1:N vlan status of the specific port
 * Input:
 *      port   		- port id
 * Output:
 *      pEnable 		- enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       
 *      RT_ERR_NULL_POINTER       
 * Note:
 *      None
 */
int32 apollo_raw_svlan_port1tonVlanEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);
    
    if ((ret = reg_array_field_read(SVLAN_EP_DMAC_CTRLr, port , REG_ARRAY_INDEX_NONE, ENf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_port1tonVlanEnable_get */

/* Function Name:
 *      apollo_raw_svlan_portSvlan_set
 * Description:
 *      Set port-based svlan configiration of the specific port
 * Input:
 *      port   		- port id
 *      svidx		- SVLAN member configuration index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       
 *      RT_ERR_SVLAN_ENTRY_INDEX       
 * Note:
 *      None
 */
int32 apollo_raw_svlan_portSvlan_set(rtk_port_t port, apollo_raw_svidx_t svidx)
{
    int32 ret;
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((APOLLO_SVIDXMAX < svidx), RT_ERR_SVLAN_ENTRY_INDEX);
    
    if ((ret = reg_array_field_write(SVLAN_P_SVIDXr, port , REG_ARRAY_INDEX_NONE, SVIDXf, &svidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_port1tonVlanEnable_get */

/* Function Name:
 *      apollo_raw_svlan_portSvlan_get
 * Description:
 *      Get port-based svlan configiration of the specific port
 * Input:
 *      port   		- port id
 * Output:
 *      pSvidx		- SVLAN member configuration index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       
 *      RT_ERR_NULL_POINTER       
 * Note:
 *      None
 */
int32 apollo_raw_svlan_portSvlan_get(rtk_port_t port, apollo_raw_svidx_t *pSvidx)
{
    int32 ret;
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((pSvidx==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(SVLAN_P_SVIDXr, port , REG_ARRAY_INDEX_NONE, SVIDXf, pSvidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_portSvlan_get */



/* Function Name:
 *      apollo_raw_svlan_lookupType_set
 * Description:
 *      Set lookup type of SVLAN
 * Input:
 *      type 		- lookup type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 apollo_raw_svlan_lookupType_set(rtk_svlan_lookupType_t type)
{
    int32 ret;
    apollo_raw_svlan_lookupType_t lookupType;
    
    switch(type)
    {
        case SVLAN_LOOKUP_S64MBRCGF: 
            lookupType = RAW_SVLAN_LOOKUP_S64MBRCGF;

            break;
        case SVLAN_LOOKUP_C4KVLAN: 
            lookupType = RAW_SVLAN_LOOKUP_C4KVLAN;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }
    
    if ((ret = reg_field_write(SVLAN_LOOK_UP_TYPEr, TYPEf, &type)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_lookupType_set */

/* Function Name:
 *      apollo_raw_svlan_lookupType_get
 * Description:
 *      Get lookup type of SVLAN
 * Input:
 *      None
 * Output:
 *      pType 		- lookup type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_CHIP_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 apollo_raw_svlan_lookupType_get(rtk_svlan_lookupType_t *pType)
{
    int32 ret;
    apollo_raw_svlan_lookupType_t lookupType;

    RT_PARAM_CHK((pType==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SVLAN_LOOK_UP_TYPEr, TYPEf, &lookupType)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    switch(lookupType)
    {
        case RAW_SVLAN_LOOKUP_S64MBRCGF: 
            *pType = SVLAN_LOOKUP_S64MBRCGF;

            break;
        case RAW_SVLAN_LOOKUP_C4KVLAN: 
            *pType = SVLAN_LOOKUP_C4KVLAN;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }


    return RT_ERR_OK;
} /* end of apollo_raw_svlan_lookupType_get */

/* Function Name:
 *      apollo_raw_svlan_tpid_set
 * Description:
 *      Set the svlan TPID 
 * Input:
 *      svlanTpid 		- svlan TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SVLAN_ETHER_TYPE
 * Note:
 *      none
 */
int32 apollo_raw_svlan_tpid_set(apollo_raw_ethertype_t svlanTpid)
{
    int32 ret;

    /* parameter check */
    RT_PARAM_CHK(svlanTpid > RTK_ETHERTYPE_MAX, RT_ERR_SVLAN_ETHER_TYPE);

    if ((ret = reg_field_write(SVLAN_CFGr, VS_TPIDf, &svlanTpid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return ret;
} /* end of apollo_raw_svlan_tpid_set */

/* Function Name:
 *      apollo_raw_svlan_tpid_get
 * Description:
 *      Get the svlan TPID 
 * Input:
 *      None
 * Output:
 *      pSvlanTpid 		- svlan TPID
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      none
 */
int32 apollo_raw_svlan_tpid_get(apollo_raw_ethertype_t *pSvlanTpid)
{
    int32 ret;

    RT_PARAM_CHK((pSvlanTpid==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SVLAN_CFGr, VS_TPIDf, pSvlanTpid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return ret;
} /* end of apollo_raw_svlan_tpid_get */

/* Function Name:
 *      apollo_raw_svlan_cfiKeepEnable_set
 * Description:
 *      Set keeping igress SVLAN CFI status  
 * Input:
 *      enable 		- enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_svlan_cfiKeepEnable_set(rtk_enable_t enable)
{
    int32 ret;

	RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if ((ret = reg_field_write(SVLAN_CTRLr, VS_CFI_KEEPf, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_cfiKeepEnable_get */

/* Function Name:
 *      apollo_raw_svlan_cfiKeepEnable_get
 * Description:
 *      Get keeping igress SVLAN CFI status   
 * Input:
 *      None
 * Output:
 *      pEnable 		- enable status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_svlan_cfiKeepEnable_get(rtk_enable_t *pEnable)
{
    int32 ret;

    RT_PARAM_CHK((pEnable==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SVLAN_CTRLr, VS_CFI_KEEPf, pEnable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_cfiKeepEnable_set */


/* Function Name:
 *      apollo_raw_svlan_trapPri_set
 * Description:
 *      Set SVLAN trapping priority  
 * Input:
 *      priority 		- SVLAN trapping priority
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PRIORITY
 * Note:
 *      None
 */
int32 apollo_raw_svlan_trapPri_set(rtk_pri_t priority)
{
    int32 ret;
    
    RT_PARAM_CHK((RTK_DOT1P_PRIORITY_MAX < priority), RT_ERR_PRIORITY);

    if ((ret = reg_field_write(SVLAN_CTRLr, VS_PRIf, &priority)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_trapPri_set */

/* Function Name:
 *      apollo_raw_svlan_trapPri_get
 * Description:
 *      Get SVLAN trapping priority  
 * Input:
 *      None
 * Output:
 *      None
 *      pPriority 		- SVLAN trapping priority
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_svlan_trapPri_get(rtk_pri_t *pPriority)
{
    int32 ret;

    RT_PARAM_CHK((pPriority==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SVLAN_CTRLr, VS_PRIf, pPriority)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_trapPri_get */

/* Function Name:
 *      apollo_raw_svlan_egrPriSel_set
 * Description:
 *      Set SVLAN egress tag priority selection
 * Input:
 *      mode 		- egress priority selection mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 apollo_raw_svlan_egrPriSel_set(rtk_svlan_priSel_t mode)
{
    int32 ret;
    apollo_raw_svlan_priSel_t selModel;

    switch(mode)
    {
        case SVLAN_PRISEL_INTERNAL_PRI:
            selModel = RAW_SVLAN_PRISEL_INTERNAL_PRI;

            break;
        case SVLAN_PRISEL_1QTAG_PRI:
            selModel = RAW_SVLAN_PRISEL_1QTAG_PRI;

            break;
        case SVLAN_PRISEL_VSPRI:
            selModel = RAW_SVLAN_PRISEL_VSPRI;

            break;
        case SVLAN_PRISEL_PBPRI:
            selModel = RAW_SVLAN_PRISEL_PBPRI;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    if ((ret = reg_field_write(SVLAN_CTRLr, VS_SPRISELf, &selModel)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_egrPriSel_set */

/* Function Name:
 *      apollo_raw_svlan_egrPriSel_get
 * Description:
 *      Get SVLAN egress tag priority selection
 * Input:
 *      None
 * Output:
 *      pMode 		- egress priority selection mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_CHIP_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 apollo_raw_svlan_egrPriSel_get(rtk_svlan_priSel_t *pMode)
{
    int32 ret;
    apollo_raw_svlan_priSel_t selModel;
    
    RT_PARAM_CHK((pMode==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SVLAN_CTRLr, VS_SPRISELf, &selModel)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    switch(selModel)
    {
        case RAW_SVLAN_PRISEL_INTERNAL_PRI:
            *pMode = SVLAN_PRISEL_INTERNAL_PRI;

            break;
        case RAW_SVLAN_PRISEL_1QTAG_PRI:
            *pMode = SVLAN_PRISEL_1QTAG_PRI;

            break;
        case RAW_SVLAN_PRISEL_VSPRI:
            *pMode = SVLAN_PRISEL_VSPRI;

            break;
        case RAW_SVLAN_PRISEL_PBPRI:
            *pMode = SVLAN_PRISEL_PBPRI;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_egrPriSel_get */

/* Function Name:
 *      apollo_raw_svlan_untagAction_set
 * Description:
 *      Set action of uplink port ingress un-Stag packet
 * Input:
 *      action		- SVLAN action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 apollo_raw_svlan_untagAction_set(rtk_svlan_action_t action)
{
    int32 ret;
    apollo_raw_svlan_action_t untagAct;
        
    switch(action)
    {
        case SVLAN_ACTION_DROP:
            untagAct = RAW_SVLAN_ACTION_DROP;

            break;
        case SVLAN_ACTION_TRAP:
            untagAct = RAW_SVLAN_ACTION_TRAP;

            break;
        case SVLAN_ACTION_SVLAN:
            untagAct = RAW_SVLAN_ACTION_SVLAN;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }
    
    if ((ret = reg_field_write(SVLAN_CTRLr, VS_UNTAGf, &untagAct)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_untagAction_set */

/* Function Name:
 *      apollo_raw_svlan_untagAction_get
 * Description:
 *      Get action of uplink port ingress un-Stag packet
 * Input:
 *      None
 * Output:
 *      pAction		- SVLAN action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_CHIP_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 apollo_raw_svlan_untagAction_get(rtk_svlan_action_t *pAction)
{
    int32 ret;
    apollo_raw_svlan_action_t untagAct;

    RT_PARAM_CHK((pAction==NULL), RT_ERR_NULL_POINTER);
        
    if ((ret = reg_field_read(SVLAN_CTRLr, VS_UNTAGf, &untagAct)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    switch(untagAct)
    {
        case RAW_SVLAN_ACTION_DROP:
            *pAction = SVLAN_ACTION_DROP;

            break;
        case RAW_SVLAN_ACTION_TRAP:
            *pAction = SVLAN_ACTION_TRAP;

            break;
        case RAW_SVLAN_ACTION_SVLAN:
            *pAction = SVLAN_ACTION_SVLAN;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_untagAction_get */

/* Function Name:
 *      apollo_raw_svlan_unmatchAction_set
 * Description:
 *      Set action of uplink port ingress un-match packet
 * Input:
 *      action		- SVLAN action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_CHIP_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 apollo_raw_svlan_unmatchAction_set(rtk_svlan_action_t action)
{
    int32 ret;
    apollo_raw_svlan_action_t unmatchAct;

    switch(action)
    {
        case SVLAN_ACTION_DROP:
            unmatchAct = RAW_SVLAN_ACTION_DROP;

            break;
        case SVLAN_ACTION_TRAP:
            unmatchAct = RAW_SVLAN_ACTION_TRAP;

            break;
        case SVLAN_ACTION_SVLAN:
            unmatchAct = RAW_SVLAN_ACTION_SVLAN;

            break;
        case SVLAN_ACTION_SVLAN_AND_KEEP:
            unmatchAct = RAW_SVLAN_ACTION_SVLAN_AND_KEEP;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    if ((ret = reg_field_write(SVLAN_CTRLr, VS_UNMATf, &action)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_unmatchAction_set */

/* Function Name:
 *      apollo_raw_svlan_unmatchAction_get
 * Description:
 *      Get action of uplink port ingress un-match packet
 * Input:
 *      None
 * Output:
 *      pAction		- SVLAN action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_CHIP_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 apollo_raw_svlan_unmatchAction_get(rtk_svlan_action_t *pAction)
{
    int32 ret;
    apollo_raw_svlan_action_t unmatchAct;

    RT_PARAM_CHK((pAction==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SVLAN_CTRLr, VS_UNMATf, &unmatchAct)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    switch(unmatchAct)
    {
        case RAW_SVLAN_ACTION_DROP:
            *pAction = SVLAN_ACTION_DROP;

            break;
        case RAW_SVLAN_ACTION_TRAP:
            *pAction = SVLAN_ACTION_TRAP;

            break;
        case RAW_SVLAN_ACTION_SVLAN:
            *pAction = SVLAN_ACTION_SVLAN;

            break;
        case RAW_SVLAN_ACTION_SVLAN_AND_KEEP:
            *pAction = SVLAN_ACTION_SVLAN_AND_KEEP;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return RT_ERR_OK;
} /* end of apollo_raw_svlan_unmatchAction_get */

/* Function Name:
 *      apollo_raw_svlan_untagSvidx_set
 * Description:
 *      Set svidx of uplink port ingress un-tag packet
 * Input:
 *      svidx		- SVLAN member configuration index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SVLAN_ENTRY_INDEX
 * Note:
 *      None
 */
int32 apollo_raw_svlan_untagSvidx_set(uint32 svidx)
{
    int32 ret;

    RT_PARAM_CHK((APOLLO_SVIDXMAX < svidx), RT_ERR_SVLAN_ENTRY_INDEX);

    if ((ret = reg_field_write(SVLAN_CTRLr, VS_UNTAG_SVIDXf, &svidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_untagSvidx_set */

/* Function Name:
 *      apollo_raw_svlan_untagSvidx_get
 * Description:
 *      Get svidx of uplink port ingress un-tag packet
 * Input:
 *      None
 * Output:
 *      pSvidx		- SVLAN member configuration index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_svlan_untagSvidx_get(uint32 *pSvidx)
{
    int32 ret;
    
    RT_PARAM_CHK((pSvidx==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SVLAN_CTRLr, VS_UNTAG_SVIDXf, pSvidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_untagSvidx_get */


/* Function Name:
 *      apollo_raw_svlan_unmatchSvidx_set
 * Description:
 *      Set svidx of uplink port ingress un-match packet
 * Input:
 *      svidx		- SVLAN member configuration index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_SVLAN_ENTRY_INDEX
 * Note:
 *      None
 */
int32 apollo_raw_svlan_unmatchSvidx_set(apollo_raw_svidx_t svidx)
{
    int32 ret;

    RT_PARAM_CHK((APOLLO_SVIDXMAX < svidx), RT_ERR_SVLAN_ENTRY_INDEX);

    if ((ret = reg_field_write(SVLAN_CTRLr, VS_UNMAT_SVIDXf, &svidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_unmatchSvidx_set */

/* Function Name:
 *      apollo_raw_svlan_unmatchSvidx_get
 * Description:
 *      Get svidx of uplink port ingress un-match packet
 * Input:
 *      None
 * Output:
 *      pSvidx		- SVLAN member configuration index
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32 apollo_raw_svlan_unmatchSvidx_get(apollo_raw_svidx_t *pSvidx)
{
    int32 ret;

    RT_PARAM_CHK((pSvidx==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SVLAN_CTRLr, VS_UNMAT_SVIDXf, pSvidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_unmatchSvidx_get */


/* Function Name:
 *      apollo_raw_svlan_mrbCfg_set
 * Description:
 *      Set SLAN member configuration entry
 * Input:
 *      pMbrCfg		- SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_PORT_MASK
 *      RT_ERR_PRIORITY
 *      RT_ERR_FID
 *      RT_ERR_SVLAN_ENTRY_INDEX
 *      RT_ERR_SVLAN_VID
 *      RT_ERR_SVLAN_EFID
 * Note:
 *      None
 */
int32 apollo_raw_svlan_mrbCfg_set(apollo_raw_svlan_mbrCfg_t *pMbrCfg)
{
    int32 ret;

	/*check*/
    RT_PARAM_CHK((pMbrCfg==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((APOLLO_SVIDXMAX < pMbrCfg->idx ), RT_ERR_SVLAN_ENTRY_INDEX);
	RT_PARAM_CHK((APOLLO_VIDMAX < pMbrCfg->svid), RT_ERR_SVLAN_VID);
	RT_PARAM_CHK(!HAL_IS_PORTMASK_VALID(pMbrCfg->mbr), RT_ERR_PORT_MASK);
	RT_PARAM_CHK(!HAL_IS_PORTMASK_VALID(pMbrCfg->untagset), RT_ERR_PORT_MASK);
	RT_PARAM_CHK((APOLLO_PRIMAX < pMbrCfg->spri), RT_ERR_PRIORITY);
	RT_PARAM_CHK((APOLLO_FIDMAX < pMbrCfg->fid_msti), RT_ERR_FID);
	RT_PARAM_CHK((APOLLO_EFIDMAX < pMbrCfg->efid), RT_ERR_SVLAN_EFID);
	RT_PARAM_CHK((RTK_ENABLE_END <= pMbrCfg->efid_en), RT_ERR_INPUT);
	RT_PARAM_CHK((RTK_ENABLE_END <= pMbrCfg->fid_en), RT_ERR_INPUT);
	
    if ((ret = reg_array_field_write(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, SVIDf, &pMbrCfg->svid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, MBRf, &pMbrCfg->mbr.bits[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, UNTAGSETf, &pMbrCfg->untagset.bits[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, SPRf, &pMbrCfg->spri)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, FIDENf, &pMbrCfg->fid_en)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, FID_MSTIf, &pMbrCfg->fid_msti)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, EFIDENf, &pMbrCfg->efid_en)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, EFIDf, &pMbrCfg->efid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of apollo_raw_svlan_mrbCfg_set */

/* Function Name:
 *      apollo_raw_svlan_mrbCfg_get
 * Description:
 *      Get SLAN member configuration entry
 * Input:
 *      None
 * Output:
 *      pMbrCfg		- SVLAN member configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_SVLAN_ENTRY_INDEX
 * Note:
 *      None
 */
int32 apollo_raw_svlan_mrbCfg_get(apollo_raw_svlan_mbrCfg_t *pMbrCfg)
{
    int32 ret;

    RT_PARAM_CHK((pMbrCfg==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((APOLLO_SVIDXMAX < pMbrCfg->idx ), RT_ERR_SVLAN_ENTRY_INDEX);

    if ((ret = reg_array_field_read(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, SVIDf, &pMbrCfg->svid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, MBRf, &pMbrCfg->mbr.bits[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, UNTAGSETf, &pMbrCfg->untagset.bits[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, SPRf, &pMbrCfg->spri)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, FIDENf, &pMbrCfg->fid_en)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, FID_MSTIf, &pMbrCfg->fid_msti)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, EFIDENf, &pMbrCfg->efid_en)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_MBRCFGr, REG_ARRAY_INDEX_NONE, pMbrCfg->idx, EFIDf, &pMbrCfg->efid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
	
    return RT_ERR_OK;
} /* end of apollo_raw_svlan_mrbCfg_get */

/* Function Name:
 *      apollo_raw_svlan_mc2sCfg_set
 * Description:
 *      Set multicast to SVLAN configuration entry
 * Input:
 *      pMc2sCfg		- multicast to SVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 *      RT_ERR_SVLAN_ENTRY_INDEX
 *      RT_ERR_CHIP_NOT_SUPPORTED
 * Note:
 *      None
 */
int32 apollo_raw_svlan_mc2sCfg_set(apollo_raw_svlan_mc2sCfg_t *pMc2sCfg)
{
    int32 ret;
	uint32 valid;

	/*check*/
    RT_PARAM_CHK((pMc2sCfg==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((APOLLO_SVLAN_MC2S_INDEX_MAX < pMc2sCfg->idx ), RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK((APOLLO_SVIDXMAX < pMc2sCfg->svidx), RT_ERR_SVLAN_ENTRY_INDEX);
	RT_PARAM_CHK((RAW_SVLAN_MC2S_FMT_END <= pMc2sCfg->format), RT_ERR_CHIP_NOT_SUPPORTED);
	RT_PARAM_CHK((RTK_ENABLE_END <= pMc2sCfg->valid), RT_ERR_INPUT);

	/*Set entry in-valid at the first*/
	/*to make sure will not have invalid action happen while changing entry data and mask*/
	valid = 0;
    if ((ret = reg_array_field_write(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, VALIDf, &valid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
	/*normal entry setting*/
    if ((ret = reg_array_field_write(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, SVIDXf, &pMc2sCfg->svidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, FORMATf, &pMc2sCfg->format)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }


    if ((ret = reg_array_field_write(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, DATAf, &pMc2sCfg->data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, MASKf, &pMc2sCfg->mask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, VALIDf, &pMc2sCfg->valid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
	
    return RT_ERR_OK;
}/* end of apollo_raw_svlan_mc2sCfg_set */

/* Function Name:
 *      apollo_raw_svlan_mc2sCfg_get
 * Description:
 *      Get multicast to SVLAN configuration entry
 * Input:
 *      None
 * Output:
 *      pMc2sCfg		- multicast to SVLAN configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 apollo_raw_svlan_mc2sCfg_get(apollo_raw_svlan_mc2sCfg_t *pMc2sCfg)
{
    int32 ret;

	/*check*/
    RT_PARAM_CHK((pMc2sCfg==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((APOLLO_SVLAN_MC2S_INDEX_MAX < pMc2sCfg->idx ), RT_ERR_OUT_OF_RANGE);

    if ((ret = reg_array_field_read(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, SVIDXf, &pMc2sCfg->svidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, FORMATf, &pMc2sCfg->format)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }


    if ((ret = reg_array_field_read(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, DATAf, &pMc2sCfg->data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, MASKf, &pMc2sCfg->mask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_MC2Sr, REG_ARRAY_INDEX_NONE, pMc2sCfg->idx, VALIDf, &pMc2sCfg->valid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
	
    return RT_ERR_OK;
}/* end of apollo_raw_svlan_mc2sCfg_get */

/* Function Name:
 *      apollo_raw_svlan_c2sCfg_set
 * Description:
 *      Set CVLAN to SVLAN configuration entry
 * Input:
 *      pC2sCfg		- CVLAN to SVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_SVLAN_EVID
 *      RT_ERR_SVLAN_ENTRY_INDEX
 *      RT_ERR_PORT_MASK
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 apollo_raw_svlan_c2sCfg_set(apollo_raw_svlan_c2sCfg_t *pC2sCfg)
{
    int32 ret;

	/*check*/
    RT_PARAM_CHK((pC2sCfg==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((APOLLO_SVLAN_C2S_INDEX_MAX < pC2sCfg->idx ), RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK((APOLLO_EVIDMAX < pC2sCfg->evid), RT_ERR_SVLAN_EVID);
	RT_PARAM_CHK((APOLLO_SVIDXMAX < pC2sCfg->svidx), RT_ERR_SVLAN_ENTRY_INDEX);
	RT_PARAM_CHK(!HAL_IS_PORTMASK_VALID(pC2sCfg->pmsk), RT_ERR_PORT_MASK);

    if ((ret = reg_array_field_write(SVLAN_C2Sr, REG_ARRAY_INDEX_NONE, pC2sCfg->idx, SVIDXf, &pC2sCfg->svidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_C2Sr, REG_ARRAY_INDEX_NONE, pC2sCfg->idx, EVIDf, &pC2sCfg->evid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_C2Sr, REG_ARRAY_INDEX_NONE, pC2sCfg->idx, C2SENPMSKf, &pC2sCfg->pmsk.bits[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
	
    return RT_ERR_OK;
}/* end of apollo_raw_svlan_c2sCfg_set */

/* Function Name:
 *      apollo_raw_svlan_c2sCfg_get
 * Description:
 *      Get CVLAN to SVLAN configuration entry
 * Input:
 *      None
 * Output:
 *      pC2sCfg		- CVLAN to SVLAN configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 apollo_raw_svlan_c2sCfg_get(apollo_raw_svlan_c2sCfg_t *pC2sCfg)
{
    int32 ret;

	/*check*/
    RT_PARAM_CHK((pC2sCfg==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((APOLLO_SVLAN_C2S_INDEX_MAX < pC2sCfg->idx ), RT_ERR_OUT_OF_RANGE);

    if ((ret = reg_array_field_read(SVLAN_C2Sr, REG_ARRAY_INDEX_NONE, pC2sCfg->idx, SVIDXf, &pC2sCfg->svidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_C2Sr, REG_ARRAY_INDEX_NONE, pC2sCfg->idx, EVIDf, &pC2sCfg->evid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_C2Sr, REG_ARRAY_INDEX_NONE, pC2sCfg->idx, C2SENPMSKf, &pC2sCfg->pmsk.bits[0])) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
	
    return RT_ERR_OK;
}/* end of apollo_raw_svlan_c2sCfg_get */

/* Function Name:
 *      apollo_raw_svlan_sp2cCfg_set
 * Description:
 *      Set downstream SVLAN and egress port to CVLAN configuration entry
 * Input:
 *      pSp2cCfg		- downstream SVLAN and egress port to CVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_INPUT
 *      RT_ERR_SVLAN_VID
 *      RT_ERR_SVLAN_ENTRY_INDEX
 *      RT_ERR_PORT_ID
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 apollo_raw_svlan_sp2cCfg_set(apollo_raw_svlan_sp2cCfg_t *pSp2cCfg)
{
    int32 ret;
    int32 valid;

	/*check*/
    RT_PARAM_CHK((pSp2cCfg==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((APOLLO_SVLAN_SP2C_INDEX_MAX < pSp2cCfg->idx), RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK((APOLLO_VIDMAX < pSp2cCfg->vid), RT_ERR_SVLAN_VID);
	RT_PARAM_CHK((APOLLO_SVIDXMAX < pSp2cCfg->svidx), RT_ERR_SVLAN_ENTRY_INDEX);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(pSp2cCfg->port), RT_ERR_PORT_ID);
	RT_PARAM_CHK((RTK_ENABLE_END <= pSp2cCfg->valid), RT_ERR_INPUT);

	/*Set entry in-valid at the first*/
	/*to make sure will not have invalid action happen while changing entry data and mask*/
	valid = 0;
    if ((ret = reg_array_field_write(SVLAN_SP2Cr, REG_ARRAY_INDEX_NONE, pSp2cCfg->idx, VALIDf, &valid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }


    if ((ret = reg_array_field_write(SVLAN_SP2Cr, REG_ARRAY_INDEX_NONE, pSp2cCfg->idx, SVIDXf, &pSp2cCfg->svidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_SP2Cr, REG_ARRAY_INDEX_NONE, pSp2cCfg->idx, VIDf, &pSp2cCfg->vid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_SP2Cr, REG_ARRAY_INDEX_NONE, pSp2cCfg->idx, DST_PORTf, &pSp2cCfg->port)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_write(SVLAN_SP2Cr, REG_ARRAY_INDEX_NONE, pSp2cCfg->idx, VALIDf, &pSp2cCfg->valid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
	
    return RT_ERR_OK;
}/* end of apollo_raw_svlan_sp2cCfg_set */

/* Function Name:
 *      apollo_raw_svlan_sp2cCfg_get
 * Description:
 *      Get downstream SVLAN and egress port to CVLAN configuration entry
 * Input:
 *      None
 * Output:
 *      pSp2cCfg		- downstream SVLAN and egress port to CVLAN configuration
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None
 */
int32 apollo_raw_svlan_sp2cCfg_get(apollo_raw_svlan_sp2cCfg_t *pSp2cCfg)
{
    int32 ret;

	/*check*/
    RT_PARAM_CHK((pSp2cCfg==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((APOLLO_SVLAN_SP2C_INDEX_MAX < pSp2cCfg->idx), RT_ERR_OUT_OF_RANGE);

    if ((ret = reg_array_field_read(SVLAN_SP2Cr, REG_ARRAY_INDEX_NONE, pSp2cCfg->idx, SVIDXf, &pSp2cCfg->svidx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_SP2Cr, REG_ARRAY_INDEX_NONE, pSp2cCfg->idx, VIDf, &pSp2cCfg->vid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_SP2Cr, REG_ARRAY_INDEX_NONE, pSp2cCfg->idx, DST_PORTf, &pSp2cCfg->port)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }

    if ((ret = reg_array_field_read(SVLAN_SP2Cr, REG_ARRAY_INDEX_NONE, pSp2cCfg->idx, VALIDf, &pSp2cCfg->valid)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_SVLAN), "");
        return ret;
    }
	
    return RT_ERR_OK;
}/* end of apollo_raw_svlan_sp2cCfg_get */




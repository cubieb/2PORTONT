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
 * Purpose : switch asic-level L34 API
 * Feature : L34 related functions
 *
 */

#include <dal/apollo/raw/apollo_raw_l34.h>



static apollo_raw_l34_arp_trf_t arp_trf_select_table = RAW_L34_ARPTRF_TABLE0;
static apollo_raw_l34_l4_trf_t l34_trf_select_table = RAW_L34_L4TRF_TABLE0;


/* Function Name:
 *      apollo_raw_l34_hsba_mode_get
 * Description:
 *      Get L34 hsba moe
 * Input:
 *      None
 * Output:
 *      mode
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hsba_mode_get(apollo_raw_l34_hsba_mode_t *mode)
{
    int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((mode==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(HSBA_CTRLr, TST_LOG_MDf, &tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    *mode = (apollo_raw_l34_hsba_mode_t)tmp_val;
	return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_l34_hsba_mode_set
 * Description:
 *      Set L34 hsba moe
 * Input:
 *      mode
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hsba_mode_set(apollo_raw_l34_hsba_mode_t mode)
{
    int32 ret;

    RT_PARAM_CHK((mode>RAW_L34_HSBA_LOG_FIRST_TO_CPU), RT_ERR_INPUT);

    if ((ret = reg_field_write(HSBA_CTRLr, TST_LOG_MDf, (uint32 *)&mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;
}




/* Function Name:
 *      apollo_raw_l34_hsbaBusyFlag_get
 * Description:
 *      Get L34 hsba busy flag
 * Input:
 *      None
 * Output:
 *      flag
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hsbaBusyFlag_get(uint32 *flag)
{
    int32 ret;

    RT_PARAM_CHK((flag==NULL), RT_ERR_NULL_POINTER);
    if ((ret = reg_field_read(HSBA_CTRLr, ALE34_BZf, flag)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;
}





/* Function Name:
 *      apollo_raw_l34_hsbaBusyFlag_get
 * Description:
 *      Get L34 hsba active flag
 * Input:
 *      None
 * Output:
 *      flag
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hsbaActiveFlag_get(uint32 *flag)
{
    int32 ret;

    RT_PARAM_CHK((flag==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(HSBA_CTRLr, HSB_ATVf, flag)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_l34_hsbaBusyFlag_get
 * Description:
 *      Get L34 hsba active flag
 * Input:
 *      flag
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hsbaActiveFlag_set(uint32 flag)
{
    int32 ret;

    if ((ret = reg_field_write(HSBA_CTRLr, HSB_ATVf, (uint32 *)&flag)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

	return RT_ERR_OK;
}
/* Function Name:
 *      apollo_raw_l34_hsaData_set
 * Description:
 *      Set L34 HSA data
 * Input:
 *      hsaData
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hsaData_set(apollo_raw_l34_hsa_t *hsaData)
{
	int32 ret,index;
    uint32 tmp_val,*tmp_val_ptr;
    ipaddr_t tmp_ip;
    l34_hsa_entry_t hsba_entry;

    RT_PARAM_CHK((hsaData==NULL), RT_ERR_NULL_POINTER);

    tmp_val = hsaData->l34trans;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_L34TRANStf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->l2trans;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_L2TRANStf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }


    tmp_val = hsaData->inter_if;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_INTER_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->l4_pri_valid;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_L4_PRI_VALIDtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->frag;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_FRAGtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->pppoe_if;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_PPPOE_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->action;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_ACTIONtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->pppid_idx;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_PPPID_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->difid;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_DIFIDtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->l4_pri_sel;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_L4_PRI_SELtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->dvid;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_DVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->reason;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_REASONtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->nexthop_mac_idx;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_NEXTHOP_MAC_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->l4_chksum;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_L4_CHKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->l3_chksum;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_L3_CHKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsaData->port;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_PORTtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_ip = hsaData->ip;
    if ((ret = table_field_set(L34_HSAt, L34_HSA_IPtf, (uint32 *)&tmp_ip, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    return RT_ERR_OK;

    /*write data to register*/
    tmp_val_ptr = (uint32 *) &hsba_entry;
    for(index=0 ; index<(sizeof(l34_hsa_entry_t)/4) ; index++)
    {
        if ((ret = reg_array_write(HSA_DESCr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_L34|MOD_DAL), "");
            return ret;
        }
        tmp_val_ptr ++;
    }
}




/* Function Name:
 *      apollo_raw_l34_hsaData_get
 * Description:
 *      Get L34 HSA data
 * Input:
 *      None
 * Output:
 *      hsaData
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hsaData_get(apollo_raw_l34_hsa_t *hsaData)
{
	int32 ret,index;
    uint32 tmp_val,*tmp_val_ptr;
    ipaddr_t tmp_ip;
    l34_hsa_entry_t hsba_entry;

    /* parameter check */
    RT_PARAM_CHK((hsaData==NULL), RT_ERR_NULL_POINTER);


    /*read data from register*/
    tmp_val_ptr = (uint32 *) &hsba_entry;
    for(index=0 ; index<(sizeof(l34_hsa_entry_t)/4) ; index++)
    {
        if ((ret = reg_array_read(HSA_DESCr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_L34|MOD_DAL), "");
            return ret;
        }
        tmp_val_ptr ++;
    }


    /*get field data from hsba buffer*/

    if ((ret = table_field_get(L34_HSAt, L34_HSA_L34TRANStf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->l34trans = tmp_val ;


    if ((ret = table_field_get(L34_HSAt, L34_HSA_L2TRANStf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->l2trans = tmp_val ;


    if ((ret = table_field_get(L34_HSAt, L34_HSA_INTER_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->inter_if = tmp_val ;


    if ((ret = table_field_get(L34_HSAt, L34_HSA_L4_PRI_VALIDtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->l4_pri_valid = tmp_val ;

    if ((ret = table_field_get(L34_HSAt, L34_HSA_FRAGtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->frag = tmp_val ;


    if ((ret = table_field_get(L34_HSAt, L34_HSA_PPPOE_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->pppoe_if = tmp_val ;


    if ((ret = table_field_get(L34_HSAt, L34_HSA_ACTIONtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->action = tmp_val ;


    if ((ret = table_field_get(L34_HSAt, L34_HSA_PPPID_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->pppid_idx = tmp_val ;


    if ((ret = table_field_get(L34_HSAt, L34_HSA_DIFIDtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->difid = tmp_val ;

    if ((ret = table_field_get(L34_HSAt, L34_HSA_L4_PRI_SELtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->l4_pri_sel = tmp_val ;

    if ((ret = table_field_get(L34_HSAt, L34_HSA_DVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->dvid = tmp_val ;


    if ((ret = table_field_get(L34_HSAt, L34_HSA_REASONtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->reason = tmp_val ;

    if ((ret = table_field_get(L34_HSAt, L34_HSA_NEXTHOP_MAC_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->nexthop_mac_idx = tmp_val ;

    if ((ret = table_field_get(L34_HSAt, L34_HSA_L4_CHKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->l4_chksum = tmp_val ;


    if ((ret = table_field_get(L34_HSAt, L34_HSA_L3_CHKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->l3_chksum = tmp_val ;


    if ((ret = table_field_get(L34_HSAt, L34_HSA_PORTtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->port = tmp_val ;

    if ((ret = table_field_get(L34_HSAt, L34_HSA_IPtf, (uint32 *)&tmp_ip, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->ip = tmp_ip ;

    return RT_ERR_OK;




}





/* Function Name:
 *      apollo_raw_l34_hsbData_set
 * Description:
 *      Set L34 HSB data
 * Input:
 *      hsbData
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hsbData_set(apollo_raw_l34_hsb_t *hsbData)
{
	int32 ret,index;
    uint32 tmp_val,*tmp_val_ptr;
    ipaddr_t tmp_ip;
    l34_hsb_entry_t hsba_entry;

    RT_PARAM_CHK((hsbData==NULL), RT_ERR_NULL_POINTER);

    tmp_val = hsbData->l2bridge;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_L2BRIDGEtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ipfrag_s;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_IPFRAG_Stf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ipmf;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_IPMFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->l4_chksum_ok;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_L4_CHKSUM_OKtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->l3_chksum_ok;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_L3_CHKSUM_OKtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cpu_direct_tx;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_CPU_DIRECT_TXtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->udp_no_chksum;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_UDP_NO_CHKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->parse_fail;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_PARSE_FAILtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->pppoe_if;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_PPPOE_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->svlan_if;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_SVLAN_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ttls;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_TTLStf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->type;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->tcp_flag;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_TCP_FLAGtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cvlan_if;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_CVLAN_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->spa;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_SPAtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cvid;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_CVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->len;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_LENtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->dport_l4chksum;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_DPORT_L4CHKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->pppoe_id;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_PPPOE_IDtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }


    tmp_ip = hsbData->dip;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_DIPtf, (uint32 *)&tmp_ip, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_ip = hsbData->sip;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_SIPtf, (uint32 *)&tmp_ip, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->sport_icmpid_chksum;
    if ((ret = table_field_set(L34_HSBt, L34_HSB_SPORT_ICMPID_CHKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    if ((ret = table_field_mac_set(L34_HSBt, L34_HSB_DMACtf, (uint8 *)&hsbData->dmac, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }


    /*write data to register*/
    tmp_val_ptr = (uint32 *) &hsba_entry;
    for(index=0 ; index<(sizeof(l34_hsb_entry_t)/4) ; index++)
    {
        if ((ret = reg_array_write(HSB_DESCr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_L34|MOD_DAL), "");
            return ret;
        }
        tmp_val_ptr ++;
    }


    /*set active flag*/
    if ((ret = apollo_raw_l34_hsbaActiveFlag_set(1)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }


    /*check busy flag*/
    index=0;
    for(;;)
    {
        if ((ret = apollo_raw_l34_hsbaBusyFlag_get(&tmp_val)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_L34|MOD_DAL), "");
            return ret;
        }
        if(tmp_val == 0)
            break;
        index++;
        if(index > 10000)
        {
            osal_printf("HSB access timeout!!");
            RT_ERR(ret, (MOD_L34|MOD_DAL), "");
            return ret;
        }

    }

    return RT_ERR_OK;
}




/* Function Name:
 *      apollo_raw_l34_hsbData_get
 * Description:
 *      Get L34 HSB data
 * Input:
 *      None
 * Output:
 *      hsbData
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hsbData_get(apollo_raw_l34_hsb_t *hsbData)
{
	int32 ret,index;
    uint32 tmp_val,*tmp_val_ptr;
    ipaddr_t tmp_ip;
    l34_hsb_entry_t hsba_entry;

    RT_PARAM_CHK((hsbData==NULL), RT_ERR_NULL_POINTER);

    /*read data from register*/
    tmp_val_ptr = (uint32 *) &hsba_entry;
    for(index=0 ; index<(sizeof(l34_hsb_entry_t)/4) ; index++)
    {
        if ((ret = reg_array_read(HSB_DESCr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_L34|MOD_DAL), "");
            return ret;
        }
        tmp_val_ptr ++;
    }


    if ((ret = table_field_get(L34_HSBt, L34_HSB_L2BRIDGEtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->l2bridge = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_IPFRAG_Stf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ipfrag_s = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_IPMFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ipmf = tmp_val ;


    if ((ret = table_field_get(L34_HSBt, L34_HSB_L4_CHKSUM_OKtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->l4_chksum_ok = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_L3_CHKSUM_OKtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->l3_chksum_ok = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_CPU_DIRECT_TXtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cpu_direct_tx = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_UDP_NO_CHKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->udp_no_chksum = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_PARSE_FAILtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->parse_fail = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_PPPOE_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->pppoe_if = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_SVLAN_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->svlan_if = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_TTLStf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ttls = tmp_val ;


    if ((ret = table_field_get(L34_HSBt, L34_HSB_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->type = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_TCP_FLAGtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->tcp_flag = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_CVLAN_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cvlan_if = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_SPAtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->spa = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_CVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cvid = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_LENtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->len = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_DPORT_L4CHKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->dport_l4chksum = tmp_val ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_PPPOE_IDtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->pppoe_id = tmp_val ;


    if ((ret = table_field_get(L34_HSBt, L34_HSB_DIPtf, (uint32 *)&tmp_ip, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->dip = tmp_ip ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_SIPtf, (uint32 *)&tmp_ip, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->sip = tmp_ip ;

    if ((ret = table_field_get(L34_HSBt, L34_HSB_SPORT_ICMPID_CHKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->sport_icmpid_chksum = tmp_val ;

    if ((ret = table_field_mac_get(L34_HSBt, L34_HSB_DMACtf, (uint8 *)&hsbData->dmac, (uint32 *) &hsba_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;

}



/* Function Name:
 *      apollo_raw_l34_pppoeTrf_get
 * Description:
 *      Get L34 PPPoE traffic indicator
 * Input:
 *      None
 * Output:
 *      pppoeTrf
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_pppoeTrf_get(uint8 *pppoeTrf)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((pppoeTrf==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(PP_AGEr, PPPOE_TRF_BMPf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    *pppoeTrf = (uint8)tmp_val;

    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_l34_portIntfIdx_get
 * Description:
 *      Get port net interface index
 * Input:
 *      None
 * Output:
 *      hsbData
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_portIntfIdx_get(rtk_port_t port,uint8 *idx)
{
	int32 ret;
    uint32 tmp_val;
    uint32 field_name;

    RT_PARAM_CHK((port > 5), RT_ERR_PORT_ID);
    RT_PARAM_CHK((idx==NULL), RT_ERR_NULL_POINTER);


    field_name = INTP0f + port;

    if ((ret = reg_field_read(NIFPr, field_name, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    *idx = (uint8)tmp_val;

    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_l34_portIntfIdx_set
 * Description:
 *      Set port net interface index
 * Input:
 *      None
 * Output:
 *      hsbData
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_portIntfIdx_set(rtk_port_t port,uint8 idx)
{
	int32 ret;
    uint32 tmp_val;
    uint32 field_name;

    RT_PARAM_CHK((port > 5), RT_ERR_PORT_ID);
    RT_PARAM_CHK((idx >= L34_NETIF_TABLE_MAX_IDX), RT_ERR_INPUT);


    field_name = INTP0f + port;
    tmp_val = idx;
    if ((ret = reg_field_write(NIFPr, field_name, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}




/* Function Name:
 *      apollo_raw_l34_extPortIntfIdx_get
 * Description:
 *      Get extention port net interface index
 * Input:
 *      None
 * Output:
 *      hsbData
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_extPortIntfIdx_get(rtk_port_t port,uint8 *idx)
{
	int32 ret;
    uint32 tmp_val;
    uint32 field_name;

    RT_PARAM_CHK((port > APOLLO_EXTPORTNO), RT_ERR_PORT_ID);
    RT_PARAM_CHK((idx==NULL), RT_ERR_NULL_POINTER);


    field_name = INTEXTP0f + port - 1;

    if ((ret = reg_field_read(NIFEPr, field_name, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    *idx = (uint8)tmp_val;

    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_l34_extPortIntfIdx_set
 * Description:
 *      Set extension port net interface index
 * Input:
 *      None
 * Output:
 *      hsbData
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_extPortIntfIdx_set(rtk_port_t port,uint8 idx)
{
	int32 ret;
    uint32 tmp_val;
    uint32 field_name;


    RT_PARAM_CHK((port > APOLLO_EXTPORTNO), RT_ERR_PORT_ID);
    RT_PARAM_CHK((idx >= L34_NETIF_TABLE_MAX_IDX), RT_ERR_PORT_ID);

    field_name = INTEXTP0f + port - 1;
    tmp_val = idx;
    if ((ret = reg_field_write(NIFEPr, field_name, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      apollo_raw_l34_vcPortIntfIdx_get
 * Description:
 *      Get vc port net interface index
 * Input:
 *      None
 * Output:
 *      hsbData
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_vcPortIntfIdx_get(rtk_port_t port,uint8 *idx)
{
	int32 ret;
    uint32 tmp_val;
    uint32 field_name;
    uint32 register_name;

    RT_PARAM_CHK((port >= APOLLO_VCPORTNO), RT_ERR_PORT_ID);
    RT_PARAM_CHK((idx==NULL), RT_ERR_NULL_POINTER);

    switch(port)
    {
        case 0:
            field_name = VC0f;
            break;
        case 1:
            field_name = VC1f;
            break;
        case 2:
            field_name = VC2f;
            break;
        case 3:
            field_name = VC3f;
            break;
        case 4:
            field_name = VC4f;
            break;
        case 5:
            field_name = VC5f;
            break;
        case 6:
            field_name = VC6f;
            break;
        case 7:
            field_name = VC7f;
            break;
        case 8:
            field_name = VC8f;
            break;
        case 9:
            field_name = VC9f;
            break;
        case 10:
            field_name = VC10f;
            break;
        case 112:
            field_name = VC11f;
            break;
        case 12:
            field_name = VC12f;
            break;
        case 13:
            field_name = VC13f;
            break;
        case 14:
            field_name = VC14f;
            break;
        case 15:
            field_name = VC15f;
            break;
        default:
            RT_PARAM_CHK((port > 15), RT_ERR_PORT_ID);
            break;
    }


    if(port > 7)
        register_name = NIFVCHr;
    else
        register_name = NIFVCLr;

    if ((ret = reg_field_read(register_name, field_name, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    *idx = (uint8)tmp_val;

    return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_l34_vcPortIntfIdx_set
 * Description:
 *      Set vc port net interface index
 * Input:
 *      None
 * Output:
 *      hsbData
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_vcPortIntfIdx_set(rtk_port_t port,uint8 idx)
{
	int32 ret;
    uint32 tmp_val;
    uint32 field_name;
    uint32 register_name;

    RT_PARAM_CHK((port >= APOLLO_VCPORTNO), RT_ERR_PORT_ID);
    RT_PARAM_CHK((idx >= L34_NETIF_TABLE_MAX_IDX), RT_ERR_INPUT);

    switch(port)
    {
        case 0:
            field_name = VC0f;
            break;
        case 1:
            field_name = VC1f;
            break;
        case 2:
            field_name = VC2f;
            break;
        case 3:
            field_name = VC3f;
            break;
        case 4:
            field_name = VC4f;
            break;
        case 5:
            field_name = VC5f;
            break;
        case 6:
            field_name = VC6f;
            break;
        case 7:
            field_name = VC7f;
            break;
        case 8:
            field_name = VC8f;
            break;
        case 9:
            field_name = VC9f;
            break;
        case 10:
            field_name = VC10f;
            break;
        case 112:
            field_name = VC11f;
            break;
        case 12:
            field_name = VC12f;
            break;
        case 13:
            field_name = VC13f;
            break;
        case 14:
            field_name = VC14f;
            break;
        case 15:
            field_name = VC15f;
            break;
        default:
            RT_PARAM_CHK((port > 15), RT_ERR_PORT_ID);
            break;
    }


    if(port > 7)
        register_name = NIFVCHr;
    else
        register_name = NIFVCLr;

    tmp_val = idx;
    if ((ret = reg_field_write(register_name, field_name, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}




/* Function Name:
 *      apollo_raw_l34_debugMonitor_set
 * Description:
 *      Set debug/monitor signal select
 * Input:
 *      None
 * Output:
 *      hsbData
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_INPUT
 * Note:
 *      None
 */
int32 apollo_raw_l34_debugMonitor_set(rtk_enable_t enable)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if(enable==DISABLED)
        tmp_val = 0;
    else
        tmp_val = 1;

    if ((ret = reg_field_write(SWTCR0r, MONSELf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_debugMonitor_get
 * Description:
 *      Get debug/monitor signal select
 * Input:
 *      None
 * Output:
 *      hsbData
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_debugMonitor_get(rtk_enable_t *enable)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((enable==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SWTCR0r, MONSELf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    if(tmp_val==0)
        *enable = DISABLED;
    else
        *enable = ENABLED;

    return RT_ERR_OK;

}



/* Function Name:
 *      apollo_raw_l34_natAttack2CPU_set
 * Description:
 *      Set nat attack packet to CPU setting
 * Input:
 *      rtk_enable_t enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_natAttack2CPU_set(rtk_enable_t enable)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if(enable==DISABLED)
        tmp_val = 0;
    else
        tmp_val = 1;

    if ((ret = reg_field_write(SWTCR0r, ENNATT2LOGf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_natAttack2CPU_get
 * Description:
 *      Get debug/monitor signal select
 * Input:
 *      None
 * Output:
 *      rtk_enable_t *enable
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_natAttack2CPU_get(rtk_enable_t *enable)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((enable==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SWTCR0r, ENNATT2LOGf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    if(tmp_val==0)
        *enable = DISABLED;
    else
        *enable = ENABLED;

    return RT_ERR_OK;

}


/* Function Name:
 *      apollo_raw_l34_wanRouteAct_set
 * Description:
 *      Set nat WAN route mode
 * Input:
 *      rtk_l34_wanroute_act_t mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_wanRouteAct_set(rtk_l34_wanroute_act_t act)
{
	int32 ret;
    uint32 tmp_val;

    switch(act)
    {
        case L34_WAN_ROUTE_FWD:
            tmp_val = 0;
            break;

        case L34_WAN_ROUTE_FWD_TO_CPU:
            tmp_val = 1;
            break;

        case L34_WAN_ROUTE_DROP:
            tmp_val = 2;
            break;
        default:
            return RT_ERR_INPUT;
    }


    if ((ret = reg_field_write(SWTCR0r, WANROUTEMODEf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_wanRouteAct_get
 * Description:
 *      Get nat WAN route mode
 * Input:
 *      None
 * Output:
 *      rtk_enable_t *enable
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_wanRouteAct_get(rtk_l34_wanroute_act_t *act)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((act==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SWTCR0r, WANROUTEMODEf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }


    switch(tmp_val)
    {
        case 0:
            *act = L34_WAN_ROUTE_FWD;
            break;

        case 1:
            *act = L34_WAN_ROUTE_FWD_TO_CPU;
            break;

        case 2:
            *act = L34_WAN_ROUTE_DROP ;

            break;

        default:
            return RT_ERR_OUT_OF_RANGE;
    }


    return RT_ERR_OK;

}


/* Function Name:
 *      apollo_raw_l34_LanIntfMDBC_set
 * Description:
 *      Set NAT LAN interface Mutilayer-Decision-Base Control mode.
 * Input:
 *      apollo_raw_l34_limbc_t mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_LanIntfMDBC_set(apollo_raw_l34_limbc_t mode)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((mode >= RAW_L34_LIMBC_RESERVED), RT_ERR_INPUT);

    tmp_val = mode;

    if ((ret = reg_field_write(SWTCR0r, LIMDBCf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_LanIntfMDBC_get
 * Description:
 *      Get NAT LAN interface Mutilayer-Decision-Base Control mode.
 * Input:
 *      None
 * Output:
 *      apollo_raw_l34_limbc_t *mode
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_LanIntfMDBC_get(apollo_raw_l34_limbc_t *mode)
{
	int32 ret;
    uint32 tmp_val;

     RT_PARAM_CHK((mode == NULL), RT_ERR_NULL_POINTER);


    if ((ret = reg_field_read(SWTCR0r, LIMDBCf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    *mode = tmp_val;

    return RT_ERR_OK;

}



/* Function Name:
 *      apollo_raw_l34_mode_set
 * Description:
 *      Set L34 mode.
 * Input:
 *      apollo_raw_l34_mode_t mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_mode_set(apollo_raw_l34_mode_t mode)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((mode > RAW_L34_L3_ENABLE_L4_ENABLE), RT_ERR_INPUT);

    tmp_val = mode;

    if ((ret = reg_field_write(SWTCR0r, NATMODEf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_mode_get
 * Description:
 *      Get L34 mode.
 * Input:
 *      None
 * Output:
 *      apollo_raw_l34_mode_t *mode
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_mode_get(apollo_raw_l34_mode_t *mode)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((mode==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SWTCR0r, NATMODEf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    *mode = tmp_val;

    return RT_ERR_OK;

}


/* Function Name:
 *      apollo_raw_l34_TtlOperationMode_set
 * Description:
 *      Set TTL-1 operation mode.
 * Input:
 *      rtk_enable_t mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_TtlOperationMode_set(rtk_enable_t mode)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((RTK_ENABLE_END <= mode), RT_ERR_INPUT);

    if(mode == DISABLED)
        tmp_val = 0;
    else
        tmp_val = 1;

    if ((ret = reg_field_write(SWTCR0r, TTL_1ENABLEf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_TtlOperationMode_get
 * Description:
 *      Get TTL-1 operation mode.
 * Input:
 *      None
 * Output:
 *      rtk_enable_t *mode
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_TtlOperationMode_get(rtk_enable_t *mode)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((mode==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SWTCR0r, TTL_1ENABLEf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    
    if(0x0 == tmp_val)
        *mode = DISABLED;
    else
        *mode = ENABLED;

    return RT_ERR_OK;

}


/* Function Name:
 *      apollo_raw_l34_L3chksumErrAllow_set
 * Description:
 *      L3 Checksum Error Allow mode set.
 * Input:
 *      rtk_enable_t mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_L3chksumErrAllow_set(rtk_enable_t mode)
{
	int32 ret;
    uint32 tmp_val;
    RT_PARAM_CHK((RTK_ENABLE_END <= mode), RT_ERR_INPUT);

    if(DISABLED == mode)
        tmp_val = 0;
    else
        tmp_val = 1;

    if ((ret = reg_field_write(SWTCR0r, L3CHKSERRALLOWf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_L3chksumErrAllow_get
 * Description:
 *      L3 Checksum Error Allow mode get.
 * Input:
 *      None
 * Output:
 *      rtk_enable_t *mode
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_L3chksumErrAllow_get(rtk_enable_t *mode)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((mode==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SWTCR0r, L3CHKSERRALLOWf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    
    if(0x0 == tmp_val)
        *mode = DISABLED;
    else
        *mode = ENABLED;

    return RT_ERR_OK;

}


/* Function Name:
 *      apollo_raw_l34_L4chksumErrAllow_set
 * Description:
 *      L3 Checksum Error Allow mode set.
 * Input:
 *      rtk_enable_t mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_L4chksumErrAllow_set(rtk_enable_t mode)
{
	int32 ret;
    uint32 tmp_val;
    RT_PARAM_CHK((RTK_ENABLE_END <= mode), RT_ERR_INPUT);

    if(mode == DISABLED)
        tmp_val = 0;
    else
        tmp_val = 1;

    if ((ret = reg_field_write(SWTCR0r, L4CHKSERRALLOWf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_L4chksumErrAllow_get
 * Description:
 *      L4 Checksum Error Allow mode get.
 * Input:
 *      None
 * Output:
 *      rtk_enable_t *mode
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_L4chksumErrAllow_get(rtk_enable_t *mode)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((mode==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SWTCR0r, L4CHKSERRALLOWf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    
    if(0x0 == tmp_val)
        *mode = DISABLED;
    else
        *mode = ENABLED;

    return RT_ERR_OK;

}



/* Function Name:
 *      apollo_raw_l34_FragPkt2Cpu_set
 * Description:
 *      Set fragment packet trap to CPU.
 * Input:
 *      rtk_enable_t mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_FragPkt2Cpu_set(rtk_enable_t mode)
{
	int32 ret;
    uint32 tmp_val;
    RT_PARAM_CHK((RTK_ENABLE_END <= mode), RT_ERR_INPUT);

    if(mode == DISABLED)
        tmp_val = 0;
    else
        tmp_val = 1;

    if ((ret = reg_field_write(SWTCR0r, FRAGMENT2CPUf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_FragPkt2Cpu_get
 * Description:
 *      Get fragment packet trap to CPU setting.
 * Input:
 *      None
 * Output:
 *      rtk_enable_t *mode
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_FragPkt2Cpu_get(rtk_enable_t *mode)
{
	int32 ret;
    uint32 tmp_val;

    RT_PARAM_CHK((mode==NULL), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(SWTCR0r, FRAGMENT2CPUf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    
    if(0x0 == tmp_val)
        *mode = DISABLED;
    else
        *mode = ENABLED;

    return RT_ERR_OK;

}



/* Function Name:
 *      apollo_raw_l34_hwL4TrfWrkTbl_set
 * Description:
 *      Set HW working table id for L4 trf.
 * Input:
 *      apollo_raw_l34_l4_trf_t table
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hwL4TrfWrkTbl_set(apollo_raw_l34_l4_trf_t table)
{
	int32 ret;
    uint32 tmp_val;

    tmp_val = table;

    if ((ret = reg_field_write(SWTCR0r, L4_TRF_HWWRK_SELf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    
    /*check HW really change complete*/
    while(1)
    {
        if ((ret = reg_field_read(SWTCR0r, L4_TRF_CHGf, (uint32 *)&tmp_val)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_L34|MOD_DAL), "");
            return ret;
        }
        if(0x0 == tmp_val)
            break;
    
    }

    l34_trf_select_table = table;
    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_hwL4TrfWrkTbl_get
 * Description:
 *      Get HW working table id for L4 trf.
 * Input:
 *      None
 * Output:
 *      apollo_raw_l34_l4_trf_t *table
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hwL4TrfWrkTbl_get(apollo_raw_l34_l4_trf_t *table)
{
	int32 ret;
    uint32 tmp_val;


    if ((ret = reg_field_read(SWTCR0r, L4_TRF_HWWRK_SELf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    *table = tmp_val;

    l34_trf_select_table = tmp_val;

    return RT_ERR_OK;

}


/* Function Name:
 *      apollo_raw_l34_l4TrfTb_get
 * Description:
 *      Get HW working table id for L4 trf.
 * Input:
 *      None
 * Output:
 *      apollomp_raw_l34_l4_trf_t *table
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_l4TrfTb_get(apollo_raw_l34_l4_trf_t table,uint32 index,rtk_enable_t *pIndicator)
{
	int32 ret;
	uint32 reg;

	if(table == RAW_L34_L4TRF_TABLE0){
		reg =  L4_TRF0r;
	}else{
		reg =  L4_TRF1r;
	}
	if ((ret = reg_array_field_read(reg,REG_ARRAY_INDEX_NONE,index,  TRFf, pIndicator)) != RT_ERR_OK){
	   	 RT_ERR(ret, (MOD_L34|MOD_DAL), "");
		 return ret;
	}

    	return RT_ERR_OK;

}

/* Function Name:
 *      apollo_raw_l34_hwArpTrfWrkTbl_set
 * Description:
 *      Set HW working table id for ARP trf.
 * Input:
 *      apollo_raw_l34_arp_trf_t table
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hwArpTrfWrkTbl_set(apollo_raw_l34_arp_trf_t table)
{
	int32 ret;
    uint32 tmp_val;

    tmp_val = table;

    if ((ret = reg_field_write(SWTCR0r, ARP_TRF_HWWRK_SELf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    
    /*check HW really change complete*/
    while(1)
    {
        if ((ret = reg_field_read(SWTCR0r, ARP_TRF_CHGf, (uint32 *)&tmp_val)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_L34|MOD_DAL), "");
            return ret;
        }
        if(0x0 == tmp_val)
            break;
    
    }

    arp_trf_select_table = table;

    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_hwArpTrfWrkTbl_get
 * Description:
 *      Get HW working table id for ARP trf.
 * Input:
 *      None
 * Output:
 *      apollo_raw_l34_arp_trf_t *table
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hwArpTrfWrkTbl_get(apollo_raw_l34_arp_trf_t *table)
{
	int32 ret;
    uint32 tmp_val;


    if ((ret = reg_field_read(SWTCR0r, ARP_TRF_HWWRK_SELf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    *table = tmp_val;

    arp_trf_select_table = tmp_val;

    return RT_ERR_OK;

}


/* Function Name:
 *      apollo_raw_l34_l4TrfTb_get
 * Description:
 *      Get HW working table id for L4 trf.
 * Input:
 *      None
 * Output:
 *      apollomp_raw_l34_l4_trf_t *table
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_arpTrfTb_get(apollo_raw_l34_arp_trf_t table,uint32 index,rtk_enable_t *pIndicator)
{
	int32 ret;
	uint32 reg;

	if(table == RAW_L34_ARPTRF_TABLE0){
		reg =  ARP_TRF0r;
	}else{
		reg =  ARP_TRF1r;
	}
	if ((ret = reg_array_field_read(reg,REG_ARRAY_INDEX_NONE,index,  TRFf, pIndicator)) != RT_ERR_OK){
	   	 RT_ERR(ret, (MOD_L34|MOD_DAL), "");
		 return ret;
	}

    	return RT_ERR_OK;

}



/* Function Name:
 *      apollo_raw_l34_hwArpTrfWrkTbl_Clear
 * Description:
 *      Clear HW working table id for ARP trf.
 * Input:
 *      apollo_raw_l34_arp_trf_t table
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hwArpTrfWrkTbl_Clear(apollo_raw_l34_arp_trf_t table)
{
	int32 ret;
    uint32 tmp_val;

    if(table == RAW_L34_ARPTRF_TABLE0)
        tmp_val = 1;
    else
        tmp_val = 2;
        

    if ((ret = reg_field_write(SWTCR0r, ARP_TRF_EXEC_CLRf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    
    /*check HW really clear complete*/
    while(1)
    {
        if ((ret = reg_field_read(SWTCR0r, ARP_TRF_EXEC_CLRf, (uint32 *)&tmp_val)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_L34|MOD_DAL), "");
            return ret;
        }
        if(0x0 == tmp_val)
            break;
    
    }


    return RT_ERR_OK;

}




/* Function Name:
 *      apollo_raw_l34_hwL4TrfWrkTbl_Clear
 * Description:
 *      Clear HW working table id for ARP trf.
 * Input:
 *      apollo_raw_l34_l4_trf_t table
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_l34_hwL4TrfWrkTbl_Clear(apollo_raw_l34_l4_trf_t table)
{
	int32 ret;
    uint32 tmp_val;

    if(table == RAW_L34_L4TRF_TABLE0)
        tmp_val = 1;
    else
        tmp_val = 2;
        

    if ((ret = reg_field_write(SWTCR0r, L4_TRF_EXEC_CLRf, (uint32 *)&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }
    
    /*check HW really clear complete*/
    while(1)
    {
        if ((ret = reg_field_read(SWTCR0r, L4_TRF_EXEC_CLRf, (uint32 *)&tmp_val)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_L34|MOD_DAL), "");
            return ret;
        }
        if( 0x0 == tmp_val)
            break;
    
    }

    return RT_ERR_OK;

}

int32 
apollo_raw_l34_portToWanAction_set(rtk_port_t port, uint32 wanIdx, rtk_action_t action)
{
    if(port);
    if(wanIdx);
    if(action);
    return RT_ERR_OK;
}

int32 
apollo_raw_l34_portToWanAction_get(rtk_port_t port, uint32 wanIdx, rtk_action_t *pAction)
{
    if(port);
    if(wanIdx);
    if(pAction);
    return RT_ERR_OK;
}

int32 
apollo_raw_l34_extPortToWanAction_set(rtk_port_t extPort, uint32 wanIdx, rtk_action_t action)
{
    if(extPort);
    if(wanIdx);
    if(action);
    return RT_ERR_OK;
}

int32 
apollo_raw_l34_extPortToWanAction_get(rtk_port_t extPort, uint32 wanIdx, rtk_action_t *pAction)
{
    if(extPort);
    if(wanIdx);
    if(pAction);
    return RT_ERR_OK;
}

int32 
apollo_raw_l34_wanToPortAction_set(uint32 wanIdx, rtk_port_t port, rtk_action_t action)
{
    if(port);
    if(wanIdx);
    if(action);
    return RT_ERR_OK;
}

int32 
apollo_raw_l34_wanToPortAction_get(uint32 wanIdx, rtk_port_t port, rtk_action_t *pAction)
{
    if(port);
    if(wanIdx);
    if(pAction);
    return RT_ERR_OK;
}

int32 
apollo_raw_l34_wanToExtPortAction_set(uint32 wanIdx, rtk_port_t extPort, rtk_action_t action)
{
    if(extPort);
    if(wanIdx);
    if(action);
    return RT_ERR_OK;
}

int32 
apollo_raw_l34_wanToExtPortAction_get(uint32 wanIdx, rtk_port_t extPort, rtk_action_t *pAction)
{
    if(extPort);
    if(wanIdx);
    if(pAction);
    return RT_ERR_OK;
}


int32
apollo_raw_l34_globalFunction_set(rtk_enable_t enable)
{
	int32 ret;
    uint32 tmpVal;

    RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_INPUT);

    if(enable == DISABLED)
        tmpVal = 0;
    else
        tmpVal = 1;

    if ((ret = reg_field_write(L34_GLB_CFGr, L34_GLOBAL_CFGf, (uint32 *)&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}



int32
apollo_raw_l34_globalFunction_get(rtk_enable_t *enable)
{
	int32 ret;
    uint32 tmpVal;

    RT_PARAM_CHK((NULL == enable), RT_ERR_NULL_POINTER);


    if ((ret = reg_field_read(L34_GLB_CFGr, L34_GLOBAL_CFGf, (uint32 *)&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_L34|MOD_DAL), "");
        return ret;
    }

    if(0 == tmpVal)
        *enable = DISABLED;
    else
        *enable = ENABLED;


    return RT_ERR_OK;
}


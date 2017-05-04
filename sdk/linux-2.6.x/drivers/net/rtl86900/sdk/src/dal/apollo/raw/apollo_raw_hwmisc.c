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
 * Purpose : switch asic-level HW misc API
 * Feature : HW misc related functions
 *
 */
#include <rtk/debug.h>
#include <dal/apollo/raw/apollo_raw_hwmisc.h>

#ifdef CONFIG_SOC_MODE

    #ifdef CONFIG_SDK_KERNEL_LINUX
        #include <linux/delay.h>
    #else
        #include <unistd.h>
    #endif
    int msec = 1;
#endif

/* Function Name:
 *      apollo_raw_virtualMacMode_Set(
 * Description:
 *      set virtual MAC mode
 * Input:
 *      port     - port number
 *      enable   - enable virtual mac on this port
 * Output:
 *      mode
 * Return:
 *      RT_ERR_OK 				- Success
 * Note:
 *      None
 */
int32 apollo_raw_virtualMacMode_Set(rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;
    uint32 tmpVal;
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable>=RTK_ENABLE_END, RT_ERR_INPUT);


    /* enable/disable VM mac*/
    tmpVal = enable;
    if ((ret = reg_array_field_write(PORT_VM_ENr,port,REG_ARRAY_INDEX_NONE,PORT_VM_ENf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return ret;
    }
    
	return RT_ERR_OK;    
}


/* Function Name:
 *      apollo_raw_virtualMac_Input(
 * Description:
 *      virtual MAC input
 * Input:
 *      fromPort - input packet to indicate port number
 *      pPkt     - input packet buffer, this buffer must include FCS
 *      len      - input packet length, include FCS length
 * Output:
 *      mode
 * Return:
 *      RT_ERR_OK 				- Success
 * Note:
 *      None
 */
int32 apollo_raw_virtualMac_Input(rtk_port_t fromPort, uint8 *pPkt, uint32 len)
{
    int32 ret;
    uint32 tmpVal;
    uint32 regData;
    uint32 index;
    uint8 *pPktPtr;
    
    RT_PARAM_CHK((pPkt==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(fromPort), RT_ERR_PORT_ID);
    
    /* general dumy data*/
    regData = 0;
    for(index =0; index < APOLLO_VIR_MAC_DUMY_CYCLE ;index++)    
    {
#ifdef CONFIG_SOC_MODE
    #ifdef CONFIG_SDK_KERNEL_LINUX
        mdelay(msec);
    #else
        usleep(msec*1000);
    #endif    
#endif
        if ((ret = reg_array_write(PORT_VM_RXr,fromPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
    }

    /*start send packet contant*/
    /*preamble*/
    for(index =0; index < 8 ;index++)    
    {
        tmpVal = 1;
        regData = 0;
#ifdef CONFIG_SOC_MODE
    #ifdef CONFIG_SDK_KERNEL_LINUX
        mdelay(msec);
    #else
        usleep(msec*1000);
    #endif    
#endif
        if ((ret = reg_field_set(PORT_VM_RXr,PORT_VM_RXDVf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        if(7 == index)
            tmpVal = 0xd5;
        else
            tmpVal = 0x55;
        if ((ret = reg_field_set(PORT_VM_RXr,PORT_VM_RXDf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if ((ret = reg_array_write(PORT_VM_RXr,fromPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
    }
    
    /*data*/
    pPktPtr = pPkt;
    for(index =0; index < len ;index++)    
    {
#ifdef CONFIG_SOC_MODE
    #ifdef CONFIG_SDK_KERNEL_LINUX
        mdelay(msec);
    #else
        usleep(msec*1000);
    #endif    
#endif
        tmpVal = 1;
        if ((ret = reg_field_set(PORT_VM_RXr,PORT_VM_RXDVf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        tmpVal = *pPktPtr;
        if ((ret = reg_field_set(PORT_VM_RXr,PORT_VM_RXDf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        if ((ret = reg_array_write(PORT_VM_RXr,fromPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        pPktPtr++;    
    }

   
    /*send dumy*/
    /* general dumy data*/
    regData = 0;

    for(index =0; index < APOLLO_VIR_MAC_DUMY_CYCLE ;index++)    
    {
#ifdef CONFIG_SOC_MODE
    #ifdef CONFIG_SDK_KERNEL_LINUX
        mdelay(msec);
    #else
        usleep(msec*1000);
    #endif    
#endif
        if ((ret = reg_array_write(PORT_VM_RXr,fromPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
    }
    

	return RT_ERR_OK;
}



/* Function Name:
 *      apollo_raw_virtualMac_Output(
 * Description:
 *      virtual MAC outpit
 * Input:
 *      toPort   - Get output packet from indicate port number
 * Output:
 *      pPkt     - Get output packet buffer, this buffer include FCS
 *      len      - input packet length, include FCS length
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_TIMEOUT
 * Note:
 *      None
 */
int32 apollo_raw_virtualMac_Output(rtk_port_t toPort, uint8 *pPkt, uint32 *plen)
{
    int32 ret;
    uint32 tmpVal;
    uint32 regData;
    uint32 index;
    uint8 *pPktPtr;
    uint32 isGetPkt;
    uint32 pktlen;
    RT_PARAM_CHK((pPkt==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((plen==NULL), RT_ERR_NULL_POINTER);    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(toPort), RT_ERR_PORT_ID);

    
    /* get dumy data and preamble*/
    regData = 0;
    isGetPkt = FALSE;
    for(index =0; index < APOLLO_VIR_MAC_TX_MAX_CNT ;index++)    
    {
#ifdef CONFIG_SOC_MODE
    #ifdef CONFIG_SDK_KERNEL_LINUX
        mdelay(msec);
    #else
        usleep(msec*1000);
    #endif    
#endif
        if ((ret = reg_array_read(PORT_VM_TXr,toPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if ((ret = reg_field_get(PORT_VM_TXr,PORT_VM_TXENf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        if(0 == tmpVal)
            continue;

        if ((ret = reg_field_get(PORT_VM_TXr,PORT_VM_TXDf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        } 
        /*get start of frame*/       
        if(0xd5 == tmpVal)
        {
            isGetPkt = TRUE;
            break;
        }
    }

    if(isGetPkt != TRUE)
        return RT_ERR_TIMEOUT;
    
    pktlen=0;
    pPktPtr = pPkt;
    /*start get packet contant*/
    do   
    {
#ifdef CONFIG_SOC_MODE
    #ifdef CONFIG_SDK_KERNEL_LINUX
        mdelay(msec);
    #else
        usleep(msec*1000);
    #endif    
#endif
        if ((ret = reg_array_read(PORT_VM_TXr,toPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if ((ret = reg_field_get(PORT_VM_TXr,PORT_VM_TXENf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if(0 == tmpVal)
            break;

        if ((ret = reg_field_get(PORT_VM_TXr,PORT_VM_TXDf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        
        *pPktPtr=tmpVal;
        pPktPtr++;
        pktlen++;
    }while(1);
    
    *plen = pktlen;
    
    /*dummy read*/
     for(index =0; index < APOLLO_VIR_MAC_DUMY_CYCLE ;index++)    
    {
#ifdef CONFIG_SOC_MODE
    #ifdef CONFIG_SDK_KERNEL_LINUX
        mdelay(msec);
    #else
        usleep(msec*1000);
    #endif    
#endif
        if ((ret = reg_array_read(PORT_VM_TXr,toPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if ((ret = reg_field_get(PORT_VM_TXr,PORT_VM_TXENf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        /*get next packt stop dummy read*/
        if(1 == tmpVal)
            break;
    }

 
	return RT_ERR_OK;
}

/* Function Name:
 *      apollo_raw_hsbData_get
 * Description:
 *      Get HSB data
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
int32 apollo_raw_hsbData_get(rtk_hsb_t *hsbData)
{
	int32 ret,index;
    uint32 tmp_val,*tmp_val_ptr;
    rtk_mac_t tmp_da;
    rtk_mac_t tmp_sa;
    hsb_entry_t hsb_entry2;
    hsb_entry_t hsb_entry;

    RT_PARAM_CHK((hsbData==NULL), RT_ERR_NULL_POINTER);

    /*read data from register*/
    tmp_val_ptr = (uint32 *) &hsb_entry2;
    for(index=0 ; index< (sizeof(hsb_entry_t)/4) ; index++)
    {
        if ((ret = reg_array_read(HSB_DATAr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        tmp_val_ptr ++;
    }

    for(index=0 ; index< (sizeof(hsb_entry_t)/4) ; index++)
    {
        hsb_entry.entry_data[sizeof(hsb_entry_t)/4 - 1 - index] = hsb_entry2.entry_data[index];
    }

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_SPAtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->spa = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_15tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_15 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_15tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_15 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_15tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_15 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_14tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_14 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_13tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_13 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_12tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_12 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_11tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_11 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_10tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_10 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_9tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_9 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_8tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_8 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_7tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_7 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_6tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_6 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_5tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_5 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_4tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_4 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_3tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_3 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_2tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_2 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_1tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_1 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_FIELD_0tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_0 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_USER_VALIDtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_valid = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CKS_OK_L4tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cks_ok_l4 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CKS_OK_L3tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cks_ok_l3 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_TTL_GT1tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ttl_gt1 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_TTL_GT5tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ttl_gt5 = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_GRE_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->gre_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_ICMP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->icmp_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_UDP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->udp_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_TCP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->tcp_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_PPPOE_SESSIONtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->pppoe_session = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_DIPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->dip = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_SIPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->sip = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_TOS_DSCPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->tos_dscp = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_IP_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ip_type = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_PTP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ptp_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_OMAPDUtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->omapdu = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_RLPP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->rlpp_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_RLDP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->rldp_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_LLC_OTHERtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->llc_other = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_PPPOE_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->pppoe_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_SNAP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->snap_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_ETHER_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ether_type = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CTAGtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ctag = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CTAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ctag_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_STAGtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->stag = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_STAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->stag_if = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_DSL_VCMSKtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_dsl_vcmsk = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_PON_SIDtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_pon_sid = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_L2BRtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_l2br = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_PPPOE_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_pppoe_idx = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_PPPOE_ACTtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_pppoe_act = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_EXTSPAtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_extspa = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_PSELtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_psel = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_DISLRNtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_dislrn = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_VSELtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_vsel = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_KEEPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_keep = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_PRItf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_pri = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_PRISELtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_prisel = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_EFIDtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_efid = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_EFID_ENtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_efid_en = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_TXPMSKtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_txpmsk = tmp_val ;
    
    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_L4Ctf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_l4c = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_L3Ctf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_l3c = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_CPUTAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_if = tmp_val ;

    if ((ret = table_field_byte_get(HSB_DATAt, HSB_DATA_SAtf, (uint8 *)&tmp_sa.octet[0], (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsbData->sa.octet[0],&tmp_sa.octet[0],sizeof(rtk_mac_t));

    if ((ret = table_field_byte_get(HSB_DATAt, HSB_DATA_DAtf, (uint8 *)&tmp_da.octet[0], (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsbData->da.octet[0],&tmp_da.octet[0],sizeof(rtk_mac_t));

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_PON_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->pon_idx = tmp_val ;

    if ((ret = table_field_get(HSB_DATAt, HSB_DATA_PKT_LENtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->pkt_len = tmp_val ;

    return RT_ERR_OK;
} /* end of apollo_raw_hsbData_get */


/* Function Name:
 *      apollo_raw_hsaData_get
 * Description:
 *      Get HSA data
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
int32 apollo_raw_hsaData_get(rtk_hsa_t *hsaData)
{
	int32 ret,index;
    uint32 tmp_val,*tmp_val_ptr;
    uint8 tmp_buf[9];
    hsa_entry_t hsa_entry;
    hsa_entry_t hsa_entry2;
    rtk_mac_t tmp_mac;

    RT_PARAM_CHK((hsaData==NULL), RT_ERR_NULL_POINTER);

    /*read data from register*/
    tmp_val_ptr = (uint32 *) &hsa_entry2;
    for(index=0 ; index<(sizeof(hsa_entry_t)/4) ; index++)
    {
        if ((ret = reg_array_read(HSA_DATAr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        tmp_val_ptr ++;
    }

    for(index=0 ; index< (sizeof(hsa_entry_t)/4) ; index++)
    {
        hsa_entry.entry_data[sizeof(hsa_entry_t)/4 - 1 - index] = hsa_entry2.entry_data[index];

    }

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_ENDSCtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_endsc = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_BGDSCtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_bgdsc = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_QIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_qid = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_RESERVEtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_reserve = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_CPUPRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_cpupri = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_FWDRSNtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_fwdrsn = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_PON_SIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_pon_sid = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_VC_SPAtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_vc_spa = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_VC_MASKtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_vc_mask = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_EXT_MASKtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_ext_mask = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_DPMtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_dpm = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_SPAtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_spa = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAB_PKTLENtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_pktlen = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAC_UNTAGSETtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_untagset = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAC_CTAG_ACTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_ctag_act = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAC_VIDZEROtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_vidzero = tmp_val ;
    
    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAC_PRITAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_pritag_if = tmp_val ;


    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAC_CTAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_ctag_if = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAC_VIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_vid = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAC_CFItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_cfi = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAC_PRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_pri = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_STAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_stag_if = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_STAG_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_stag_type = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_SP2Stf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_sp2s = tmp_val ;


    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_DEItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_dei = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_VIDSELtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_vidsel = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_FRCTAGtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_frctag = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_FRCTAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_frctag_if = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_SVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_svid = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_SVIDXtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_svidx = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_PKT_SPRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_pkt_spri = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAS_SPRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_spri = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAM_USER_PRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsam_user_pri = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAM_1P_REMtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsam_1p_rem = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAM_1P_REM_ENtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsam_1p_rem_en = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAM_DSCP_REMtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsam_dscp_rem = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAM_DSCP_REM_ENtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsam_dscp_rem_en = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAF_REGEN_CRCtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_regen_crc = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAF_KEEPtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_keep = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAF_PTPtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_ptp = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAF_IPV4tf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_ipv4 = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAF_IPV6tf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_ipv6 = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAF_RFC1042tf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_rfc1042 = tmp_val ;

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAF_PPPOE_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_pppoe_if = tmp_val ;

    if ((ret = table_field_byte_get(HSA_DATAt, HSA_DATA_RNG_NHSAP_PTP_RESVtf, (uint8 *)&tmp_buf[0], (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsaData->rng_nhsap_ptp_resv[0],&tmp_buf[0],9);

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAP_PTP_ACTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsap_ptp_act = tmp_val ;
    
    if ((ret = table_field_byte_get(HSA_DATAt, HSA_DATA_RNG_NHSAP_PTP_SECtf, (uint8 *)&tmp_buf[0], (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsaData->rng_nhsap_ptp_sec[0],&tmp_buf[0],6);

    if ((ret = table_field_get(HSA_DATAt, HSA_DATA_RNG_NHSAP_PTP_NSECtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    
    hsaData->rng_nhsap_ptp_nsec = tmp_val ;



    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_L3Rtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_l3 = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_ORGtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_org = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_L2TRANStf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_l2trans = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_L34TRANStf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_l34trans = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_SRC_MODtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_src_mode = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_PPPOE_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_pppoe_idx = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_PPPOE_ACTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_pppoe_act = tmp_val ;
    
    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_SMAC_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_smac_idx = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_L3CKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_l3chsum = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_L4CKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_l4chsum = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_NEWIPtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_newip = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_NEWPORTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_newport = tmp_val ;

    if ((ret = table_field_byte_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_NEWDMACtf, (uint8 *)&tmp_mac.octet[0], (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsaData->rng_nhsan_newmac.octet[0],&tmp_mac.octet[0],sizeof(rtk_mac_t));

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_TTLM1_EXTMASKtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_ttlm1_extmask = tmp_val ;

    if ((ret = table_field_get(HSA_DATA_NATt, HSA_DATA_NAT_RNG_NHSAN_TTLM1_PMASKtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_ttlm1_pmask = tmp_val ;
  
    return RT_ERR_OK;
} /* end of apollo_raw_hsaData_get */


/* Function Name:
 *      apollo_raw_hsdData_get
 * Description:
 *      Get HSA debug data  
 * Input:
 *      None
 * Output:
 *      hsdData
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 apollo_raw_hsdData_get(rtk_hsa_debug_t *hsdData)
{
	int32 ret,index;
    uint32 tmp_val,*tmp_val_ptr;
    uint8 tmp_buf[9];
    hsd_entry_t hsd_entry;
    hsd_entry_t hsd_entry2;
    rtk_mac_t tmp_mac;

    RT_PARAM_CHK((hsdData==NULL), RT_ERR_NULL_POINTER);

    /*read data from register*/
    tmp_val_ptr = (uint32 *) &hsd_entry2;
    for(index=0 ; index<(sizeof(hsd_entry_t)/4) ; index++)
    {
        if ((ret = reg_array_read(HSA_TX_DBGr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        tmp_val_ptr ++;
    }

    for(index=0 ; index< (sizeof(hsd_entry_t)/4) ; index++)
    {
        hsd_entry.entry_data[sizeof(hsd_entry_t)/4 - 1 - index] = hsd_entry2.entry_data[index];

    }

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_EPtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ep = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_DSL_VCtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_dsl_vc = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_34PPPOEtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_34pppoe = tmp_val ;


    if ((ret = table_field_byte_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_34SMACtf, (uint8 *)&tmp_mac.octet[0], (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsdData->hsa_debug_34smac.octet[0],&tmp_mac.octet[0],sizeof(rtk_mac_t));

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_TTLPMSKtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ttlpmsk = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_TTLEXMSKtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ttlexmsk = tmp_val ;

    if ((ret = table_field_byte_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_NEWMACtf, (uint8 *)&tmp_mac.octet[0], (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsdData->hsa_debug_newmac.octet[0],&tmp_mac.octet[0],sizeof(rtk_mac_t));

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_NEWPRTtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_newprt = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_NEWIPtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_newip = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_L4CKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l4cksum = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_L3CKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l3cksum = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_PPPOEACTtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_pppoeact = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_SRC_MODtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_src_mod = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_L34TRANStf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l34trans = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_L2TRANStf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l2trans = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_ORGtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_org = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA1_L3Rtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l3r = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_SV_DEItf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_sv_dei = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_STYPtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_styp = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_PKTLEN_ORItf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_pktlen_ori = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_QIDtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_qid = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_STDSCtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_stdsc = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_CPUPRItf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_cpupri = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_SPRItf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_spri = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_CORItf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_cori = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_CMDYtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_cmdy = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_CRMStf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_crms = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_CINStf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_cins = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_CVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_cvid = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_CFItf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_cfi = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_PTPNSECtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ptpnsec = tmp_val ;

    if ((ret = table_field_byte_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_PTPSECtf, (uint8 *)&tmp_buf[0], (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsdData->hsa_debug_ptpsec[0],&tmp_buf[0],6);

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_PTPACTtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ptpact = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_REGENCRCtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_regencrc = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_PPPOEtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_pppoe = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_RFC1042tf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_rfc1042 = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_IPV6tf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ipv6 = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_IPV4tf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ipv4 = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_PTPtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ptp = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_REMDSCP_PRItf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_remdscp_pri = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_REM1Q_PRItf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_rem1q_pri = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_REMDSCP_ENtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_remdscp_en = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_REM1Q_ENtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_rem1q_en = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_SVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_svid = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_INSTAGtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_instag = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_INCTAGtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_inctag = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_PKTLENtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_pktlen = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_SPAtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_spa = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_DPCtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_dpc = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_EXTMSKtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_extmsk = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_VCMSKtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_vcmsk = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_PONSIDtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ponsid = tmp_val ;

    if ((ret = table_field_get(HSA_DEBUG_DATAt, HSA_DEBUG_DATA_TXHSA0_TRPRSNtf, (uint32 *)&tmp_val, (uint32 *) &hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_trprsn = tmp_val ;

    return RT_ERR_OK;
}


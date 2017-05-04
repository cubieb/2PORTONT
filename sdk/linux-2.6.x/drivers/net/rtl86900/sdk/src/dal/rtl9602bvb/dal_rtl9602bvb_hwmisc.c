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
#include <dal/rtl9602bvb/dal_rtl9602bvb.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_hwmisc.h>
#include <rtk/switch.h>
#include <ioal/mem32.h>




/* Function Name:
 *      rtl9602bvb_virtualMacMode_Set(
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
int32 rtl9602bvb_virtualMacMode_Set(rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;
    uint32 tmpVal;
    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(enable>=RTK_ENABLE_END, RT_ERR_INPUT);


    /* enable/disable VM mac*/
    tmpVal = enable;
    if ((ret = reg_array_field_write(RTL9602BVB_PORT_VM_ENr,port,REG_ARRAY_INDEX_NONE,RTL9602BVB_PORT_VM_ENf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return ret;
    }
    
	return RT_ERR_OK;    
}


/* Function Name:
 *      rtl9602bvb_virtualMac_Input(
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
int32 rtl9602bvb_virtualMac_Input(rtk_port_t fromPort, uint8 *pPkt, uint32 len)
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
    for(index =0; index < RTL9602BVB_VIR_MAC_DUMY_CYCLE ;index++)    
    {
        if ((ret = reg_array_write(RTL9602BVB_PORT_VM_RXr,fromPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
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
        if ((ret = reg_field_set(RTL9602BVB_PORT_VM_RXr, RTL9602BVB_PORT_VM_RXDVf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        if(7 == index)
            tmpVal = 0xd5;
        else
            tmpVal = 0x55;
        if ((ret = reg_field_set(RTL9602BVB_PORT_VM_RXr, RTL9602BVB_PORT_VM_RXDf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if ((ret = reg_array_write(RTL9602BVB_PORT_VM_RXr,fromPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        
            
    }
    
    /*data*/
    pPktPtr = pPkt;
    for(index =0; index < len ;index++)    
    {
        tmpVal = 1;
        if ((ret = reg_field_set(RTL9602BVB_PORT_VM_RXr, RTL9602BVB_PORT_VM_RXDVf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        tmpVal = *pPktPtr;
        if ((ret = reg_field_set(RTL9602BVB_PORT_VM_RXr,RTL9602BVB_PORT_VM_RXDf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        if ((ret = reg_array_write(RTL9602BVB_PORT_VM_RXr,fromPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        pPktPtr++;    
    }

   
    /*send dumy*/
    /* general dumy data*/
    regData = 0;

    for(index =0; index < RTL9602BVB_VIR_MAC_DUMY_CYCLE ;index++)    
    {
        if ((ret = reg_array_write(RTL9602BVB_PORT_VM_RXr,fromPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
    }
    

	return RT_ERR_OK;
}



/* Function Name:
 *      rtl9602bvb_virtualMac_Output(
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
int32 rtl9602bvb_virtualMac_Output(rtk_port_t toPort, uint8 *pPkt, uint32 *plen)
{
    int32 ret;
    uint32 tmpVal;
    uint32 regData;
    uint32 index;
    uint8 *pPktPtr;
    uint32 isGetPkt;
    uint32 pktlen;
    uint32 bufLen;
    
    RT_PARAM_CHK((pPkt==NULL), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((plen==NULL), RT_ERR_NULL_POINTER);    
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(toPort), RT_ERR_PORT_ID);

    /*init packet length*/
    bufLen = *plen;
    *plen = 0;
    
    /* get dumy data and preamble*/
    regData = 0;
    isGetPkt = FALSE;
    for(index =0; index < RTL9602BVB_VIR_MAC_TX_MAX_CNT ;index++)    
    {
        if ((ret = reg_array_read(RTL9602BVB_PORT_VM_TXr,toPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if ((ret = reg_field_get(RTL9602BVB_PORT_VM_TXr, RTL9602BVB_PORT_VM_TXENf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        if(0 == tmpVal)
            continue;

        if ((ret = reg_field_get(RTL9602BVB_PORT_VM_TXr, RTL9602BVB_PORT_VM_TXDf,&tmpVal,&regData)) != RT_ERR_OK)
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
        if ((ret = reg_array_read(RTL9602BVB_PORT_VM_TXr,toPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if ((ret = reg_field_get(RTL9602BVB_PORT_VM_TXr, RTL9602BVB_PORT_VM_TXENf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if(0 == tmpVal)
            break;

        if ((ret = reg_field_get(RTL9602BVB_PORT_VM_TXr, RTL9602BVB_PORT_VM_TXDf,&tmpVal,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if(pktlen < bufLen)
        {
            *pPktPtr=tmpVal;
            pPktPtr++;
        }
        pktlen++;
            
        
    }while(1);
    
    *plen = pktlen;
    
    /*dummy read*/
     for(index =0; index < RTL9602BVB_VIR_MAC_DUMY_CYCLE ;index++)    
    {
        if ((ret = reg_array_read(RTL9602BVB_PORT_VM_TXr,toPort,REG_ARRAY_INDEX_NONE,&regData)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        if ((ret = reg_field_get(RTL9602BVB_PORT_VM_TXr, RTL9602BVB_PORT_VM_TXENf,&tmpVal,&regData)) != RT_ERR_OK)
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
 *      rtl9602bvb_hsbData_get
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
int32 rtl9602bvb_hsbData_get(rtk_hsb_t *hsbData)
{
	int32 ret,index;
    uint32 tmp_val,*tmp_val_ptr;
    rtk_mac_t tmp_da;
    rtk_mac_t tmp_sa;
	rtk_ipv6_addr_t tmp_ip6;
    rtl9602bvb_hsb_entry_t hsb_entry2;
    rtl9602bvb_hsb_entry_t hsb_entry;

    RT_PARAM_CHK((hsbData==NULL), RT_ERR_NULL_POINTER);

    /*read data from register*/
    tmp_val_ptr = (uint32 *) &hsb_entry2;
    for(index=0 ; index< (sizeof(rtl9602bvb_hsb_entry_t)/4) ; index++)
    {
        if ((ret = reg_array_read(RTL9602BVB_HSB_DATAr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }

        tmp_val_ptr ++;
    }

    for(index=0 ; index< (sizeof(rtl9602bvb_hsb_entry_t)/4) ; index++)
    {
        hsb_entry.entry_data[sizeof(rtl9602bvb_hsb_entry_t)/4 - 1 - index] = hsb_entry2.entry_data[index];
    }

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_SPAtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->spa = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_15tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_15 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_15tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_15 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_15tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_15 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_14tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_14 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_13tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_13 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_12tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_12 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_11tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_11 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_10tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_10 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_9tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_9 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_8tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_8 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_7tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_7 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_6tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_6 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_5tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_5 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_4tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_4 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_3tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_3 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_2tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_2 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_1tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_1 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_0tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_field_0 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_VALIDtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->user_valid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_LEN_OF_NHStf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->len_of_nhs = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_L3PROTOCOL_NHtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->l3proto_nh = tmp_val;

    if ((ret = table_field_byte_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_DIP6tf, (uint8 *)&tmp_ip6.ipv6_addr[0], (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsbData->dip6.ipv6_addr[0],&tmp_ip6.ipv6_addr[0],sizeof(rtk_ipv6_addr_t));

    if ((ret = table_field_byte_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_SIP6tf, (uint8 *)&tmp_ip6.ipv6_addr[0], (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsbData->sip6.ipv6_addr[0],&tmp_ip6.ipv6_addr[0],sizeof(rtk_ipv6_addr_t));


    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_IP6_HN_RGtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ip6_nh_rg = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CKS_OK_L4tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cks_ok_l4 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CKS_OK_L3tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cks_ok_l3 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_TTL_GT1tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ttl_gt1 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_TTL_GT5tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ttl_gt5 = tmp_val;


    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_L4_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->l4_type = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_PPPOE_SESSIONtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->pppoe_session = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_DIPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->dip = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_SIPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->sip = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_TCtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->tc = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_TOStf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->tos_dscp = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_IP6_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ip6_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_IP4_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ip4_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_PTP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ptp_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_OAMPDUtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->oampdu = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_RLPP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->rlpp_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_RLDP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->rldp_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_LLC_OTHERtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->llc_other = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_PPPOE_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->pppoe_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_SNAP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->snap_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_ETHER_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ether_type = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CTAGtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ctag = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CTAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->ctag_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_STAGtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->stag = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_STAG_TPIDtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->stag_tpid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_STAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->stag_if = tmp_val;


    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PON_SIDtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_pon_sid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_L2BRtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_l2br = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PPPOE_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_pppoe_idx = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PPPOE_ACTtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_pppoe_act = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_L34KEEPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_l34keep = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_SBtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_sb = tmp_val;
    
    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PSELtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_psel = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_DISLRNtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_dislrn = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_KEEPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_keep = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PRItf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_pri = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PRISELtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_prisel = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_TXPMSKtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_txpmsk = tmp_val;
    
    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_L4Ctf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_l4c = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_L3Ctf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_l3c = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->cputag_if = tmp_val;

    if ((ret = table_field_byte_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_SAtf, (uint8 *)&tmp_sa.octet[0], (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsbData->sa.octet[0],&tmp_sa.octet[0],sizeof(rtk_mac_t));

    if ((ret = table_field_byte_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_DAtf, (uint8 *)&tmp_da.octet[0], (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsbData->da.octet[0],&tmp_da.octet[0],sizeof(rtk_mac_t));

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_PON_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->pon_idx = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_PKT_LENtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->pkt_len = tmp_val;

    return RT_ERR_OK;
} /* end of rtl9602bvb_hsbData_get */


/* Function Name:
 *      rtl9602bvb_hsaData_get
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
int32 rtl9602bvb_hsaData_get(rtk_hsa_t *hsaData)
{
	int32 ret,index;
    uint32 tmp_val,*tmp_val_ptr;
    uint8 tmp_buf[9];
    rtl9602bvb_hsa_entry_t hsa_entry;
    rtl9602bvb_hsa_entry_t hsa_entry2;
    rtk_mac_t tmp_mac;

    RT_PARAM_CHK((hsaData==NULL), RT_ERR_NULL_POINTER);

    /*read data from register*/
    tmp_val_ptr = (uint32 *) &hsa_entry2;
    for(index=0 ; index<(sizeof(rtl9602bvb_hsa_entry_t)/4) ; index++)
    {
        if ((ret = reg_array_read(RTL9602BVB_HSA_DATAr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        tmp_val_ptr ++;
    }

    for(index=0 ; index< (sizeof(rtl9602bvb_hsa_entry_t)/4) ; index++)
    {
        hsa_entry.entry_data[sizeof(rtl9602bvb_hsa_entry_t)/4 - 1 - index] = hsa_entry2.entry_data[index];

    }

//DATA_TABLE
    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_ENDSCtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_endsc = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_BGDSCtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_bgdsc = tmp_val;

	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_FLOODPKTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_floodpkt = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_QIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_qid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_ORGtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_org = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_L3Rtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_l3r = tmp_val;


    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_EXTMASKtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_ext_mask = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_CPUPRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_cpupri = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_ISSBtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_issb = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_IPMCtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_ipmc = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_FWDRSNtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_fwdrsn = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_PON_SIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_pon_sid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_DPMtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_dpm = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_DMA_SPAtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_dma_spa = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_SPAtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_spa = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAB_PKTLENtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsab_pktlen = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_IG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_ctag_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_UNTAGSETtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_untagset = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_PONACTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_ctag_ponact = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_ACTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_ctag_act = tmp_val;


    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_PRI_REM_PON_ENtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_1p_rem_pon_en = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_PRI_REM_ENtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_1p_rem_en = tmp_val;

	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_PONPRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
	{
	   RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
	   return RT_ERR_FAILED;
	}
	hsaData->rng_nhsac_ponpri = tmp_val;
	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_PONVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
	{
	    RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
	    return RT_ERR_FAILED;
	}
	hsaData->rng_nhsac_ponvid = tmp_val;
	
  	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_PRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_pri = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_VIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_vid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_TAGPRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_tagpri = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_TAGCFItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_tagcfi = tmp_val;
	
  	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_TAGPRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_tagpri = tmp_val;

  	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAC_TAGVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsac_tagvid = tmp_val;
    
    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_IG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_stag_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_UNTAGSETtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_untagset = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_PONACTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_ponact = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_PONPRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_ponspri = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_PONVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_ponsvid = tmp_val;


    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_PRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_spri = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_VIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_svid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_SP2CVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_sp2cvid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_SP2CACTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_sp2cact = tmp_val;

	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_TPID_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_tpid_type = tmp_val;

	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_TAGPRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_tagpri = tmp_val;

	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_TAGDEItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_tagdei = tmp_val;

	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAS_TAGVIDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsas_tagvid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAM_INTPRItf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsam_intpri = tmp_val;

	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAM_DSCP_REM_PONtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsam_dscp_rem_pon = tmp_val;

	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAM_DSCP_REM_PON_ENtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsam_dscp_rem_en_pon = tmp_val;


    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAM_DSCP_REMtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsam_dscp_rem = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAM_DSCP_REM_ENtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsam_dscp_rem_en = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAF_REGEN_CRCtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_regen_crc = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAF_CPUKEEPtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_cpukeep = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAF_OTHERKEEPtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_keep = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAF_PTPtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_ptp = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAF_TCPtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_tcp = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAF_UDPtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_udp = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAF_IPV4tf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_ipv4 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAF_IPV6tf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_ipv6 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAF_RFC1042tf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_rfc1042 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAF_PPPOE_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsaf_pppoe_if = tmp_val;


	if ((ret = table_field_get(RTL9602BVB_HSA_DATA_PTPt, RTL9602BVB_HSA_DATA_PTP_RNG_HSAP_PTP_IDtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
		return RT_ERR_FAILED;
	}
	hsaData->rng_nhsap_ptp_id = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_PTPt, RTL9602BVB_HSA_DATA_PTP_RNG_HSAP_PTP_ACTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsap_ptp_act = tmp_val;
    
    if ((ret = table_field_byte_get(RTL9602BVB_HSA_DATA_PTPt, RTL9602BVB_HSA_DATA_PTP_RNG_HSAP_PTP_SECtf, (uint8 *)&tmp_buf[0], (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsaData->rng_nhsap_ptp_sec[0],&tmp_buf[0],6);

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_PTPt, RTL9602BVB_HSA_DATA_PTP_RNG_HSAP_PTP_NSECtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }    
    hsaData->rng_nhsap_ptp_nsec = tmp_val;

//NAT
    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_DSLITE_ACTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_dslite_act= tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_DSLITE_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_dslite_idx = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_WANSAtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_wansa = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_L2TRANStf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_l2trans = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_L34TRANStf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_l34trans = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_SRC_MODtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_src_mode = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_PPPOE_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_pppoe_idx = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_PPPOE_ACTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_pppoe_act = tmp_val;
    
    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_SMAC_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_smac_idx = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_L3CKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_l3chsum = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_L4CKSUMtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_l4chsum = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_NATMCt, RTL9602BVB_HSA_DATA_NATMC_RNG_HSAN_L3TRIDXtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_l3tridx = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_NEWIPtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_newip = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_NEWPORTtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_newport = tmp_val;

    if ((ret = table_field_byte_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_NEWDMACtf, (uint8 *)&tmp_mac.octet[0], (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsaData->rng_nhsan_newmac.octet[0],&tmp_mac.octet[0],sizeof(rtk_mac_t));

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_TABLEt, RTL9602BVB_HSA_DATA_TABLE_RNG_HSAN_TTLM1_PMASKtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_ttlm1_pmask = tmp_val;


    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_OMCIt, RTL9602BVB_HSA_DATA_OMCI_RNG_HSAN_OMCI_MSG_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_omci_msg_type  = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DATA_OMCIt, RTL9602BVB_HSA_DATA_OMCI_RNG_HSAN_OMCI_CONT_LENtf, (uint32 *)&tmp_val, (uint32 *) &hsa_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsaData->rng_nhsan_omci_cont_len = tmp_val;

  
    return RT_ERR_OK;
} /* end of apollo_raw_hsaData_get */


/* Function Name:
 *      rtl9602bvb_hsdData_get
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
int32 rtl9602bvb_hsdData_get(rtk_hsa_debug_t *hsdData)
{
	int32 ret,index;
	uint32 tmp_val;
    uint8 tmp_buf[9];
    rtk_mac_t tmp_mac;
	uint8 sizeTb;
	uint32 *hsd_entry;
	uint32 *hsd_entry2;
	uint32 *tmp_val_ptr;

    RT_PARAM_CHK((hsdData==NULL), RT_ERR_NULL_POINTER);
	
	sizeTb = rtk_rtl9602bvb_table_list[INT_RTL9602BVB_HSA_DEBUG_DATA_RTL9602BVB].datareg_num;

	hsd_entry = osal_alloc(sizeTb*4);
	hsd_entry2 = osal_alloc(sizeTb*4);

    /*read data from register*/
    tmp_val_ptr = hsd_entry2;
	
	for(index=0; index < sizeTb; index++)
    {
        if ((ret = reg_array_read(RTL9602BVB_HSD_DATAr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        tmp_val_ptr ++;
    }

	for(index=0; index < sizeTb; index++)
    {

		*(hsd_entry + sizeTb - 1 - index) = *(hsd_entry2 + index);		
	}

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_TXPADtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_txpad = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_STDSCtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_stdsc = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_DMA_SPAtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_dmaspa = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_SPAtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_spa = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_DSCP_REM_PRItf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_dscp_rem_pri = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_DSCP_REM_ENtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_dscp_rem_en = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_REGENCRCtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_regencrc = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_PKTLEN_PLAtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_pktlen_pla = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_PKTLEN_DMAtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_pktlen_dma = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_DPCtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_dpc = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_ISSBtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_issb = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_TTLPMSKtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ttlpmsk = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_TTLEXMSKtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ttlexmsk = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_TRPRSNtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_trprsn = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_EXTMSKtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_extmsk = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_PONSIDtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ponsid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_L3Rtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l3r = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_CPUPRItf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_cpupri = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_QIDtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_qid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_ORGtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_org = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_PTPNSECtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ptpnsec = tmp_val;

    if ((ret = table_field_byte_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_PTPSECtf, (uint8 *)&tmp_buf[0], (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsdData->hsa_debug_ptpsec[0],&tmp_buf[0],6);

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_PTPIDtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ptpid = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_PTPACTtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ptpact = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_PTPtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ptp = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_CTAGtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ctag = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_CINStf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_cins = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_STAGtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_stag = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_STYPtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_styp = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD0_SINStf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_sins = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_EGR_PORTtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_egr_port = tmp_val;


    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_DSLITE_IDXtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_dslite_idx = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_DSLITE_ACTtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_dslite_act = tmp_val;





    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_SRC_MODtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_src_mod = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_PPPOE_ACTtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_pppoe_act = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_NEWPRTtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_newprt = tmp_val;

    if ((ret = table_field_byte_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_NEWDMACtf, (uint8 *)&tmp_mac.octet[0], (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsdData->hsa_debug_newdmac.octet[0],&tmp_mac.octet[0],sizeof(rtk_mac_t));


    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_NEWIPtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_newip = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_L4CKSUMtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l4cksum = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_L3CKSUMtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l3cksum = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_L34TRANStf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l34trans = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_L2TRANStf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l2trans = tmp_val;

    if ((ret = table_field_byte_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_NEWSMACtf, (uint8 *)&tmp_mac.octet[0], (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    osal_memcpy(&hsdData->hsa_debug_newsmac.octet[0],&tmp_mac.octet[0],sizeof(rtk_mac_t));

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_34PPPOEtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_l34pppoe = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_UDPtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_udp = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_IPV6tf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ipv6= tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_IPV4tf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_ipv4= tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_TCPtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_tcp = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_RFC1042tf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_rfc1042 = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_PPPOEtf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_pppoe = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSA_DEBUG_DATAt, RTL9602BVB_HSA_DEBUG_DATA_RNG_HSD1_CPUINStf, (uint32 *)&tmp_val, (uint32 *) hsd_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsdData->hsa_debug_cpuins = tmp_val;

	osal_free(hsd_entry);
	osal_free(hsd_entry2);	

    return RT_ERR_OK;
}


/* Function Name:
 *      rtl9602bvb_hsbData_set
 * Description:
 *      Get HSB data
 * Input:
 *      hsbData
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK 				- Success
 *      RT_ERR_SMI  			- SMI access error
 * Note:
 *      None
 */
int32 rtl9602bvb_hsbData_set(rtk_hsb_t *hsbData)
{
	int32 ret,index;
    uint32 tmp_val,*tmp_val_ptr;
    rtk_mac_t tmp_da;
    rtk_mac_t tmp_sa;
	rtk_ipv6_addr_t tmp_ip6;
    rtl9602bvb_hsb_entry_t hsb_entry2;
    rtl9602bvb_hsb_entry_t hsb_entry;

    RT_PARAM_CHK((hsbData==NULL), RT_ERR_NULL_POINTER);

	tmp_val = hsbData->spa;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_SPAtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

	tmp_val = hsbData->user_field_15;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_15tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

	tmp_val = hsbData->user_field_14;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_14tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

	tmp_val = hsbData->user_field_13;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_13tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_12;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_12tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_11;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_11tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_10;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_10tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_0;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_9tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_8;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_8tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_7;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_7tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_6;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_6tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_5;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_5tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_4;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_4tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_3;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_3tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_2;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_2tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_1;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_1tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_field_0;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_FIELD_0tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->user_valid;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_USER_VALIDtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->len_of_nhs;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_LEN_OF_NHStf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->l3proto_nh;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_L3PROTOCOL_NHtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    osal_memcpy(&tmp_ip6.ipv6_addr[0],&hsbData->dip6.ipv6_addr[0],sizeof(rtk_ipv6_addr_t));
    if ((ret = table_field_byte_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_DIP6tf, (uint8 *)&tmp_ip6.ipv6_addr[0], (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    osal_memcpy(&tmp_ip6.ipv6_addr[0],&hsbData->sip6.ipv6_addr[0],sizeof(rtk_ipv6_addr_t));
    if ((ret = table_field_byte_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_SIP6tf, (uint8 *)&tmp_ip6.ipv6_addr[0], (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ip6_nh_rg;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_IP6_HN_RGtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cks_ok_l4;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CKS_OK_L4tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cks_ok_l3;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CKS_OK_L3tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ttl_gt1;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_TTL_GT1tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ttl_gt5;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_TTL_GT5tf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->l4_type;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_L4_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->pppoe_session;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_PPPOE_SESSIONtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->dip;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_DIPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->sip;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_SIPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->tos_dscp;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_TCtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ip6_if;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_IP6_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ip4_if;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_IP4_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ptp_if;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_PTP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->oampdu;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_OAMPDUtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->rlpp_if;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_RLPP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->rldp_if;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_RLDP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->llc_other;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_LLC_OTHERtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->pppoe_if;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_PPPOE_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->snap_if;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_SNAP_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ether_type;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_ETHER_TYPEtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ctag;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CTAGtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->ctag_if;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CTAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->stag;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_STAGtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->stag_tpid;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_STAG_TPIDtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->stag_if;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_STAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_pon_sid;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PON_SIDtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_l2br;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_L2BRtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_pppoe_idx;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PPPOE_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_pppoe_act;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PPPOE_ACTtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_l34keep;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_L34KEEPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_sb;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_SBtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    
    tmp_val = hsbData->cputag_psel;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PSELtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_dislrn;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_DISLRNtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_keep;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_KEEPtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_pri;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PRItf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_prisel;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_PRISELtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_txpmsk;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_TXPMSKtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    
    tmp_val = hsbData->cputag_l4c;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_L4Ctf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_l3c;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_L3Ctf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->cputag_if;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_CPUTAG_IFtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    osal_memcpy(&tmp_sa.octet[0],&hsbData->sa.octet[0],sizeof(rtk_mac_t));
    if ((ret = table_field_byte_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_SAtf, (uint8 *)&tmp_sa.octet[0], (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    osal_memcpy(&tmp_da.octet[0],&hsbData->da.octet[0],sizeof(rtk_mac_t));
    if ((ret = table_field_byte_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_DAtf, (uint8 *)&tmp_da.octet[0], (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->pon_idx;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_PON_IDXtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

    tmp_val = hsbData->pkt_len;
    if ((ret = table_field_set(RTL9602BVB_HSB_DATA_TABLEt, RTL9602BVB_HSB_DATA_TABLE_PKT_LENtf, (uint32 *)&tmp_val, (uint32 *) &hsb_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }

	for(index=0 ; index< (sizeof(rtl9602bvb_hsb_entry_t)/4) ; index++)
	{
		hsb_entry2.entry_data[sizeof(rtl9602bvb_hsb_entry_t)/4 - 1 - index] = hsb_entry.entry_data[index];
	}

	/*write data from register*/
	tmp_val_ptr = (uint32 *) &hsb_entry2;
	for(index=0 ; index< (sizeof(rtl9602bvb_hsb_entry_t)/4) ; index++)
	{
		if ((ret = reg_array_write(RTL9602BVB_HSB_DATA_DBGr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
		{
			RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
			return ret;
		}

		tmp_val_ptr ++;
	}
	
    return RT_ERR_OK;
} /* end of rtl9602bvb_hsbData_set */



/* Function Name:
 *      rtl9602bvb_hsdPar_get
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
int32 rtl9602bvb_hsbPar_get(rtk_hsb_t *hsbData)
{
	int32 ret,index;
	uint32 tmp_val;
	uint8 sizeTb;
	uint32 *bug_entry;
	uint32 *bug_entry2;
	uint32 *tmp_val_ptr;

    RT_PARAM_CHK((hsbData==NULL), RT_ERR_NULL_POINTER);
	
	sizeTb = rtk_rtl9602bvb_table_list[INT_RTL9602BVB_HSB_PAR_TABLE_RTL9602BVB].datareg_num;

	bug_entry = osal_alloc(sizeTb*4);
	bug_entry2 = osal_alloc(sizeTb*4);

    /*read data from register*/
    tmp_val_ptr = bug_entry2;
	
	for(index=0; index < sizeTb; index++)
    {
        if ((ret = reg_array_read(RTL9602BVB_HSB_PARSERr,REG_ARRAY_INDEX_NONE,index,tmp_val_ptr)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
            return ret;
        }
        tmp_val_ptr ++;
    }

	for(index=0; index < sizeTb; index++)
    {

		*(bug_entry + sizeTb - 1 - index) = *(bug_entry2 + index);		
	}

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_UNItf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_uni = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_MLTtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_mlt = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_BRDtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_brd = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_L2MLTtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_l2mlt = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_IPV4MLTtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_ipv4mlt = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_IPV6MLTtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_ipv6mlt = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_FRM_PROVIDERtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_frm_provider = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_FRM_WANtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_frm_wan = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_TCP_IFtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_tcp_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_UDP_IFtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_udp_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_IGMP_IFtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_igmp_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_MLD_IFtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_mld_if = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_L34PKTtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_l34pkt = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_RMA_TYPEtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_rma_type = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_FLD00_VLDtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_fld00_vld = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_SW_PKTtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_sw_pkt = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_UTP_PMSKtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_utp_pmsk = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_EXT_PMSKtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_ext_pmsk = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_CPU_PSELtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_cpu_sel = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_IPV6_RESV_MCtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_ipv6_resv_mc = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_IPV4_RESV_MCtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_ipv4_resv_mc = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_PRI_CTAGtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_pri_ctag = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_DSLITE_MATCH_IDXtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_dslite_match_idx = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_MYRLDPtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_myrldp = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_PONIDXtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_ponidx = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_MPCP_OMCItf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_mpcp_omci = tmp_val;

    if ((ret = table_field_get(RTL9602BVB_HSB_PAR_TABLEt, RTL9602BVB_HSB_PAR_TABLE_RNG_DBGPARSOR_RX_PKTLENtf, (uint32 *)&tmp_val, (uint32 *) bug_entry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HWMISC|MOD_DAL), "");
        return RT_ERR_FAILED;
    }
    hsbData->par_rx_pktlen = tmp_val;

	
    return RT_ERR_OK;
}




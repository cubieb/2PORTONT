/*
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 *
 * *
 * $Revision:  $
 * $Date: 2013-10-16 $
 *
 * Purpose : EPON MAC register access APIs
 *
 * Feature : Provide the APIs to access EPON MAC
 *
 */

#include <common/rt_error.h>
#include <common/rt_type.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_ponmac.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_epon.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_pbo.h>

#include <ioal/mem32.h>
#include <rtk/ponmac.h>
#include <rtk/switch.h>
#include <rtk/oam.h>
#include <rtk/irq.h>
#include <rtk/intr.h>
#include <rtk/led.h>

#include <dal/rtl9602bvb/dal_rtl9602bvb_intr.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_port.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_oam.h>

#include "../../module/pkt_redirect/pkt_redirect.h"


/***********************************************************
    EPON Queue mapping
    dying gasp queue = 31
    
    =======================================
    RTL9602BVB_EPON_QUEUE_NUM_PER_LLID = 8 (only 4 LLID would be used)
    =======================================
    
    LLID idx  Data_queue  oam_queue  mpcp_queue
    --------  ----------  ---------  ---------- 
     0        0~7         8          9  
     1        N/A         10         11  
     2        16~23       24         25  
     3        N/A         26         27  
     4        32~39       40         41  
     5        N/A         42         43  
     6        48~55       56         57  
     7        N/A         58         59  
    
    =======================================
    RTL9602BVB_EPON_QUEUE_NUM_PER_LLID = 4
    =======================================
    LLID idx  Data_queue  oam_queue  mpcp_queue
    --------  ----------  ---------  ---------- 
     0        0~3         8          9  
     1        4~7         10         11  
     2        16~19       24         25  
     3        20~23       26         27  
     4        32~35       40         41  
     5        36~39       42         43  
     6        48~51       56         57  
     7        52~55       58         59  

************************************************************/



static uint32 epon_init = {INIT_NOT_COMPLETED};

static rtk_epon_laser_status_t forceLaserState = RTK_EPON_LASER_STATUS_NORMAL;
static rtk_epon_polarity_t oe_polarity = EPON_POLARITY_HIGH;


typedef struct rtl9602bvb_raw_epon_llid_table_s
{
    uint16  llid;
    uint8   valid;
    uint8   report_timer;
    uint8   is_report_timeout; /*read only*/

}rtl9602bvb_raw_epon_llid_table_t;



static uint32 _eponRegIntCnt=0;
static uint32 _eponTimeDriftIntCnt=0;
static uint32 _eponMpcpTimeoutIntCnt=0;
static uint32 _eponLosIntCnt=0;
static uint32 _eponMpcpTimeoutLLidCnt[RTL9602BVB_MAX_LLID_ENTRY];
static uint32 _eponRegSuccessCnt=0;
static uint32 _eponRegFailIntCnt=0;
static uint32 _epon1ppsIntCnt=0;
static uint32 _eponfecIntCnt=0;

static uint32 dal_rtl9602bvb_epon_isr_init(void);
static void dal_rtl9602bvb_epon_isr_entry(void);
static int32 _rtl9602bvb_epon_initPonQueue(uint8  llidIdx, uint32 queueId);
static int32 rtl9602bvb_raw_epon_llidTable_get(uint32 llid_idx,rtl9602bvb_raw_epon_llid_table_t *entry);
static int32 rtl9602bvb_raw_epon_anyLlidEntryVaild(void);


#if defined(CONFIG_SDK_KERNEL_LINUX)
static uint32 _dal_rtl9602bvb_epon_losIntrHandle(void);

uint32 _dal_rtl9602bvb_epon_losIntrHandle(void)
{
    int32  ret,i;    
    rtk_enable_t state;
    rtk_epon_llid_entry_t llidEntry;

    unsigned char losMsg[] = {
        0x15, 0x68, /* Magic key */
        0xde, 0xad  /* Message body */
    };    
    
    if((ret = dal_rtl9602bvb_epon_losState_get(&state)) != RT_ERR_OK)
    {
    	RT_ERR(ret,(MOD_EPON | MOD_DAL), "");
		return ret;
    }
    if(ENABLED==state)
    {/*detect fiber pull off*/
        /* Trigger lost of link */
        for(i=0;i<HAL_MAX_LLID_ENTRY();i++)
        {
            llidEntry.llidIdx = i;
            ret = rtk_epon_llid_entry_get(&llidEntry);
            if(RT_ERR_OK != ret)
            {
                return ret;
            }
            
            llidEntry.valid = DISABLED;
            llidEntry.llid = 0x7fff;
            ret = rtk_epon_llid_entry_set(&llidEntry);
            if(RT_ERR_OK != ret)
            {
                return ret;
            }
        }
        pkt_redirect_kernelApp_sendPkt(PR_USER_UID_EPONOAM, 1, sizeof(losMsg), losMsg);
        //osal_printf("\n los laser off\n");        
    }
    return RT_ERR_OK;
}
#endif



void dal_rtl9602bvb_epon_isr_entry(void)
{
    int32  ret,i;
    uint32 data;
    rtk_enable_t state;

#if defined(CONFIG_SDK_KERNEL_LINUX)
    if((ret = dal_rtl9602bvb_intr_imr_set(INTR_TYPE_EPON,DISABLED)) != RT_ERR_OK)
    {
    	RT_ERR(ret,(MOD_EPON | MOD_DAL), "");
		goto epon_intr_exit; 
    }
#endif    
    /*LLID register success interrupt check*/
    if ((ret = dal_rtl9602bvb_epon_intr_get(EPON_INTR_REG_FIN,&state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        goto epon_intr_exit; 
    }
    
    if(ENABLED==state)
    {
        /*epon register success handle*/
        _eponRegIntCnt++;
        /*disable register success interrupt*/
        data = 0;
        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_REG_FIN, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            
        
        if ((ret = reg_field_read(RTL9602BVB_EPON_INTRr,RTL9602BVB_REG_RESULTf,&data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
            goto epon_intr_exit; 
        } 
        if(data == 1)       
        {
            _eponRegSuccessCnt++;
            /*add sid valid to registered LLID idx*/
            if ((ret = reg_field_write(RTL9602BVB_EPON_RGSTR1r,RTL9602BVB_REG_LLID_IDXf,&data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                goto epon_intr_exit; 
            }


            /*stop sync local time*/
            /*set stop sync local time to enable, only sync local time from llid 1*/
            data=2;
            if ((ret = reg_field_write(RTL9602BVB_DBG_LCTM_DRFr,RTL9602BVB_UPD_LCL_TYPEf,&data)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                goto epon_intr_exit; 
            }            

        }
        else
        {
            _eponRegFailIntCnt++;
        }

    }

    /*Time drift interrupt check*/
    if ((ret = dal_rtl9602bvb_epon_intr_get(EPON_INTR_TIMEDRIFT,&state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        goto epon_intr_exit; 
    }
    
    if(ENABLED==state)
    {
        /*EPON_INTR_TIMEDRIFT handle*/
        _eponTimeDriftIntCnt++;
        /*disable register success interrupt*/
        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_TIMEDRIFT, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            

        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_TIMEDRIFT, ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            

    }

    /*mpcp timeout interrupt check*/
    if ((ret = dal_rtl9602bvb_epon_intr_get(EPON_INTR_MPCPTIMEOUT,&state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        goto epon_intr_exit; 
    }
    
    if(ENABLED==state)
    {
        /*epon mpcp timeout handle*/
        _eponMpcpTimeoutIntCnt++;

        /*disable register success interrupt*/
        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_MPCPTIMEOUT, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            

        /*check which llid mpcp timeout*/
        if((ret = reg_field_read(RTL9602BVB_EPON_INTRr, RTL9602BVB_MPCP_TIMEOUT_LLIDIDXf, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }
        for(i=0;i<HAL_MAX_LLID_ENTRY();i++)
        {
            if(data & (1<<i))    
                _eponMpcpTimeoutLLidCnt[i]++;
        }
        
        /*clear interrupt status*/
        data = 0;
        if((ret = reg_field_write(RTL9602BVB_EPON_INTRr, RTL9602BVB_MPCP_TIMEOUT_LLIDIDXf, &data)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }

        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_MPCPTIMEOUT, ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            
    }


    /*los interrupt check*/
    if ((ret = dal_rtl9602bvb_epon_intr_get(EPON_INTR_LOS,&state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        goto epon_intr_exit; 
    }
    
    if(ENABLED==state)
    {
        /*epon EPON_INTR_LOS handle*/
        _eponLosIntCnt++;
        /*disable register success interrupt*/
        data = 0;
        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_LOS, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            

#if defined(CONFIG_SDK_KERNEL_LINUX)
        if((ret = _dal_rtl9602bvb_epon_losIntrHandle()) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            
            
#endif
        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_LOS, ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }
    }

    /*1pps interrupt check*/
    if ((ret = dal_rtl9602bvb_epon_intr_get(EPON_INTR_1PPS,&state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        goto epon_intr_exit; 
    }
    
    if(ENABLED==state)
    {

        /*disable register success interrupt*/
        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_1PPS, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            

        /*1pps interrupt handle*/
        _epon1ppsIntCnt++;

        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_1PPS, ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            
    }

    /*fec interrupt check*/
    if ((ret = dal_rtl9602bvb_epon_intr_get(EPON_INTR_FEC,&state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        goto epon_intr_exit; 
    }
    
    if(ENABLED==state)
    {
        /*disable register success interrupt*/
        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_FEC, DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            
        /*fec interrupt handle*/
        _eponfecIntCnt++;

        if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_FEC, ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            goto epon_intr_exit; 
        }            
    }


epon_intr_exit:
#if defined(CONFIG_SDK_KERNEL_LINUX)
    /* switch interrupt clear EPON state */
    if((ret=dal_rtl9602bvb_intr_ims_clear(INTR_TYPE_EPON)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ;
	}
    if((ret = dal_rtl9602bvb_intr_imr_set(INTR_TYPE_EPON,ENABLED)) != RT_ERR_OK)
    {
    	RT_ERR(ret,(MOD_EPON | MOD_DAL), "");
		return ;
    }
#endif
    return ; 
}

uint32 dal_rtl9602bvb_epon_isr_init(void)
{
    int32  ret;
    uint32 data;

    /*diable all EPON interrupt mask*/
    data = 0;
    if((ret = dal_rtl9602bvb_epon_intr_disableAll()) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }    


    /*enable LoS interrupt*/
    if((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_LOS, ENABLED)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }  
    
#if defined(CONFIG_SDK_KERNEL_LINUX)
	/*register EPON isr*/
	if((ret = rtk_irq_isr_register(INTR_TYPE_EPON,dal_rtl9602bvb_epon_isr_entry)) != RT_ERR_OK)
	{
		RT_ERR(ret,(MOD_EPON | MOD_DAL), "");
		return ret;
	}

    if((ret = dal_rtl9602bvb_intr_imr_set(INTR_TYPE_EPON,ENABLED)) != RT_ERR_OK)
    {
    	RT_ERR(ret,(MOD_EPON | MOD_DAL), "");
		return ret;
    }

#endif

    return RT_ERR_OK;    
}    



/* Function Name:
 *      rtl9602bvb_raw_epon_regLlidIdx_set
 * Description:
 *      Set EPON registeration LLID index
 * Input:
 *      idx 		- registeration LLID index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
static int32 rtl9602bvb_raw_epon_regLlidIdx_set(uint32 idx)
{
 	int32   ret;
	uint32 tmp_val;   

    RT_PARAM_CHK((idx > HAL_MAX_LLID_ENTRY()), RT_ERR_INPUT);

	
	tmp_val = idx;	
    if ((ret = reg_field_write(RTL9602BVB_EPON_RGSTR1r,RTL9602BVB_REG_LLID_IDXf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      rtl9602bvb_raw_epon_regLlidIdx_get
 * Description:
 *      Get EPON registeration LLID index
 * Input:
 *      None
 * Output:
 *      idx 		- registeration LLID index
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
static int32 rtl9602bvb_raw_epon_regLlidIdx_get(uint32 *idx)
{
 	int32   ret;
	uint32 tmp_val;   
	
    if ((ret = reg_field_read(RTL9602BVB_EPON_RGSTR1r,RTL9602BVB_REG_LLID_IDXf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	*idx = tmp_val;	

    return RT_ERR_OK;   
}



/* Function Name:
 *      rtl9602bvb_raw_epon_llidIdxMac_set
 * Description:
 *      Set EPON registeration MAC address
 * Input:
 *      idx 		- registeration LLID index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
static int32 rtl9602bvb_raw_epon_llidIdxMac_set(uint32 llidIdx, rtk_mac_t *mac)
{
 	int32   ret;
 	uint32 tmp_val[3],fieldVal;   
 	
    RT_PARAM_CHK((llidIdx >= HAL_MAX_LLID_ENTRY()), RT_ERR_INPUT);
 	
    if ((ret = reg_array_read(RTL9602BVB_LLID_TABLEr,REG_ARRAY_INDEX_NONE,llidIdx,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

	fieldVal = mac->octet[0];
    if ((ret = reg_field_set(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC5f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  

	fieldVal = mac->octet[1];
    if ((ret = reg_field_set(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC4f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  

	fieldVal = mac->octet[2];
    if ((ret = reg_field_set(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC3f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  

	fieldVal = mac->octet[3];
    if ((ret = reg_field_set(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC2f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  

	fieldVal = mac->octet[4];
    if ((ret = reg_field_set(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC1f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  

	fieldVal = mac->octet[5];
    if ((ret = reg_field_set(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC0f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  

    if ((ret = reg_array_write(RTL9602BVB_LLID_TABLEr,REG_ARRAY_INDEX_NONE,llidIdx,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;   
}



/* Function Name:
 *      rtl9602bvb_raw_epon_llidIdxMac_get
 * Description:
 *      Get EPON registeration MAC address
 * Input:
 *      rtk_mac_t *mac 		- registeration MAC address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
static int32 rtl9602bvb_raw_epon_llidIdxMac_get(uint32 llidIdx,rtk_mac_t *mac)
{
 	int32   ret;
 	uint32 tmp_val[3],fieldVal;   

    RT_PARAM_CHK((llidIdx >= HAL_MAX_LLID_ENTRY()), RT_ERR_INPUT);
  	
    if ((ret = reg_array_read(RTL9602BVB_LLID_TABLEr,REG_ARRAY_INDEX_NONE,llidIdx,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

    if ((ret = reg_field_get(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC5f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
	mac->octet[0] = fieldVal;
		
    if ((ret = reg_field_get(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC4f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
	mac->octet[1] = fieldVal;


    if ((ret = reg_field_get(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC3f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
	mac->octet[2] = fieldVal;


    if ((ret = reg_field_get(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC2f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
	mac->octet[3] = fieldVal;



    if ((ret = reg_field_get(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC1f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
	mac->octet[4] = fieldVal;



    if ((ret = reg_field_get(RTL9602BVB_LLID_TABLEr,RTL9602BVB_ONU_MAC0f,&fieldVal,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
	mac->octet[5] = fieldVal;

    return RT_ERR_OK;   
}

static int32 rtl9602bvb_raw_epon_anyLlidEntryVaild(void)
{
    uint32 llidIdx;
    int32   ret;
    int32   anyLlidEntryVaild = 0;
    rtl9602bvb_raw_epon_llid_table_t  rawLlidEntry;

    /*check all llid entry*/
    for(llidIdx=0;llidIdx<HAL_MAX_LLID_ENTRY();llidIdx++)
    {
        if ((ret = rtl9602bvb_raw_epon_llidTable_get(llidIdx,&rawLlidEntry)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            return ret;
        }
        if(1 == rawLlidEntry.valid)
        {
            anyLlidEntryVaild=1;
            break;    
        }
    }
    return anyLlidEntryVaild;
}    

/* Function Name:
 *      rtl9602bvb_raw_epon_regReguest_set
 * Description:
 *      Set EPON registeration request mode
 * Input:
 *      mode 		- registeration request enable/disable 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
static int32 rtl9602bvb_raw_epon_regReguest_set(rtk_enable_t mode)
{
 	int32   ret;
	uint32 tmp_val;   
	
    if(mode==DISABLED)
    {
        tmp_val = 0;

    }
    else
    {
        /*check if all LLID entry invalid*/
        if(rtl9602bvb_raw_epon_anyLlidEntryVaild()==0)
        {
            /*only all LLID invalid need drain out all gate entry*/
            {
                /*reset EPON_REG_BAK0[1], draint out latest gate entry*/
                if ((ret = reg_field_read(RTL9602BVB_EPON_REG_BAK0r,RTL9602BVB_EPON_REG_BAK0f,&tmp_val)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                    return ret;
                }
                tmp_val = tmp_val| 0x00000002;
                if ((ret = reg_field_write(RTL9602BVB_EPON_REG_BAK0r,RTL9602BVB_EPON_REG_BAK0f,&tmp_val)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                    return ret;
                }
                if ((ret = reg_field_read(RTL9602BVB_EPON_REG_BAK0r,RTL9602BVB_EPON_REG_BAK0f,&tmp_val)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                    return ret;
                }
                tmp_val = tmp_val & 0xfffffffd;
                if ((ret = reg_field_write(RTL9602BVB_EPON_REG_BAK0r,RTL9602BVB_EPON_REG_BAK0f,&tmp_val)) != RT_ERR_OK)
                {
                    RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                    return ret;
                }
            }        
        
            /*set stop sync local time to disable, sycn local tome from all mpcp packet*/
            tmp_val=0;
            if ((ret = reg_field_write(RTL9602BVB_DBG_LCTM_DRFr,RTL9602BVB_UPD_LCL_TYPEf,&tmp_val)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                return ret;
            }
        }
        /*set register request to 1*/       
        tmp_val = 1;
    }   
        
    if ((ret = reg_field_write(RTL9602BVB_EPON_RGSTR2r,RTL9602BVB_REGISTER_REQUESTf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      rtl9602bvb_raw_epon_regReguest_get
 * Description:
 *      Get EPON registeration request mode
 * Input:
 *      mode 		- registeration request enable/disable 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
static int32 rtl9602bvb_raw_epon_regReguest_get(rtk_enable_t *mode)
{
 	int32   ret;
	uint32 tmp_val;   
	
    if ((ret = reg_field_read(RTL9602BVB_EPON_RGSTR2r,RTL9602BVB_REGISTER_REQUESTf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    if(tmp_val==0)
        *mode = DISABLED;    
    else
        *mode = ENABLED;

    return RT_ERR_OK;   
}




/* Function Name:
 *      rtl9602bvb_raw_epon_regPendingGrantNum_set
 * Description:
 *      Set EPON registeration pendding grant number
 * Input:
 *      num		- pendding grant number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
static int32 rtl9602bvb_raw_epon_regPendingGrantNum_set(uint8 num)
{
 	int32   ret;
	uint32 tmp_val;   

    RT_PARAM_CHK((num > 8), RT_ERR_INPUT);

	
	tmp_val = num;	
    if ((ret = reg_field_write(RTL9602BVB_EPON_RGSTR2r,RTL9602BVB_REG_PENDDING_GRANTf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	 
    return RT_ERR_OK;   
}



/* Function Name:
 *      rtl9602bvb_raw_epon_regPendingGrantNum_get
 * Description:
 *      Get EPON registeration pendding grant number
 * Input:
 *      None 
 * Output:
 *      num		- pendding grant number * Return:
 * Return:
 *      RT_ERR_OK 					- Success
 *      RT_ERR_SMI  				- SMI access error
 * Note:
 *      None
 */
static int32 rtl9602bvb_raw_epon_regPendingGrantNum_get(uint32 *num)
{
 	int32   ret;
	uint32 tmp_val;   
	
    if ((ret = reg_field_read(RTL9602BVB_EPON_RGSTR2r,RTL9602BVB_REG_PENDDING_GRANTf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	*num = tmp_val;	

    return RT_ERR_OK;   
}



static int32 rtl9602bvb_raw_epon_llidTable_set(uint32 llid_idx, rtl9602bvb_raw_epon_llid_table_t *entry)
{
 	int32   ret;
	uint32  tmp_val[3],tmp_field_val;   

    RT_PARAM_CHK((llid_idx >= HAL_MAX_LLID_ENTRY()), RT_ERR_INPUT);

    
    if ((ret = reg_array_read(RTL9602BVB_LLID_TABLEr,REG_ARRAY_INDEX_NONE,llid_idx,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    tmp_field_val = entry->llid;
    
    if ((ret = reg_field_set(RTL9602BVB_LLID_TABLEr,RTL9602BVB_LLIDf,&tmp_field_val,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
	
    tmp_field_val = entry->valid;
    
    if ((ret = reg_field_set(RTL9602BVB_LLID_TABLEr,RTL9602BVB_VALIDf,&tmp_field_val,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  

    tmp_field_val = entry->report_timer;
    
    if ((ret = reg_field_set(RTL9602BVB_LLID_TABLEr,RTL9602BVB_REPORT_TIMERf,&tmp_field_val,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  

    if ((ret = reg_array_write(RTL9602BVB_LLID_TABLEr,REG_ARRAY_INDEX_NONE,llid_idx,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }        
    return RT_ERR_OK;   
}


static int32 rtl9602bvb_raw_epon_llidTable_get(uint32 llid_idx,rtl9602bvb_raw_epon_llid_table_t *entry)
{
 	int32   ret;
	uint32  tmp_val[3],tmp_field_val;   

    RT_PARAM_CHK((llid_idx >= HAL_MAX_LLID_ENTRY()), RT_ERR_INPUT);
    
    if ((ret = reg_array_read(RTL9602BVB_LLID_TABLEr,REG_ARRAY_INDEX_NONE,llid_idx,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
  
    
    if ((ret = reg_field_get(RTL9602BVB_LLID_TABLEr,RTL9602BVB_LLIDf,&tmp_field_val,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    entry->llid = tmp_field_val;


    if ((ret = reg_field_get(RTL9602BVB_LLID_TABLEr,RTL9602BVB_VALIDf,&tmp_field_val,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
    entry->valid = tmp_field_val;

	
    if ((ret = reg_field_get(RTL9602BVB_LLID_TABLEr,RTL9602BVB_REPORT_TIMERf,&tmp_field_val,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
    entry->report_timer = tmp_field_val;

   if ((ret = reg_field_get(RTL9602BVB_LLID_TABLEr,RTL9602BVB_REPORT_TIMEOUTf,&tmp_field_val,tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }  
    entry->is_report_timeout = tmp_field_val;
   
    return RT_ERR_OK;   
}



int32 rtl9602bvb_raw_epon_forceLaserOn_set(uint32 force)
{
 	int32   ret;
	
    if ((ret = reg_field_write(RTL9602BVB_EPON_TX_CTRLr,RTL9602BVB_FORCE_LASER_ONf,&force)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
       
    return RT_ERR_OK;   
}


int32 rtl9602bvb_raw_epon_forceLaserOn_get(uint32 *pForce)
{
 	int32   ret;
	uint32  tmp_val;   
	
    if ((ret = reg_field_read(RTL9602BVB_EPON_TX_CTRLr,RTL9602BVB_FORCE_LASER_ONf,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    *pForce = tmp_val;    
        
    return RT_ERR_OK;   
}



static int32 _rtl9602bvb_epon_initPonQueue(uint8  llidIdx, uint32 queueId)
{
 	int32   ret;
    rtk_ponmac_queueCfg_t   queueCfg;
    rtk_ponmac_queue_t logicalQueue;	
    uint32  flowId;
    
    memset(&queueCfg, 0, sizeof(rtk_ponmac_queueCfg_t));
    logicalQueue.schedulerId = llidIdx;
    logicalQueue.queueId = queueId;   

    queueCfg.cir       = 0x0;
    queueCfg.pir       = HAL_PONMAC_PIR_CIR_RATE_MAX();
    queueCfg.type      = STRICT_PRIORITY;
    queueCfg.egrssDrop = DISABLED;
    
    
    if((ret= dal_rtl9602bvb_ponmac_queue_add(&logicalQueue, &queueCfg)) != RT_ERR_OK)
    {
        return ret;
    }
     
    /*mapping flow to queue*/
    flowId = RTL9602BVB_TCONT_QUEUE_MAX *(llidIdx/4) + queueId;
    if((ret= dal_rtl9602bvb_ponmac_flow2Queue_set(flowId, &logicalQueue)) != RT_ERR_OK)
    {
        return ret;
    }
        
    return RT_ERR_OK;   
}




/* Function Name:
 *      dal_rtl9602bvb_epon_init
 * Description:
 *      epon register level initial function
 * Input:
 *
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_init(void)  
{
    uint32 data;
    int32  ret;
    rtk_port_macAbility_t mac_ability;
    rtk_epon_report_threshold_t thRpt;
    uint8  llidIdx;
   
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    
    
    if ((ret = dal_rtl9602bvb_ponmac_mode_set(PONMAC_MODE_EPON)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    } 


    /* PON port set as force mode */
    {
        osal_memset(&mac_ability, 0x00, sizeof(rtk_port_macAbility_t));
        mac_ability.speed           = PORT_SPEED_1000M;
        mac_ability.duplex          = PORT_FULL_DUPLEX;
        mac_ability.linkFib1g       = DISABLED;
        mac_ability.linkStatus      = PORT_LINKUP;
        mac_ability.txFc            = DISABLED;
        mac_ability.rxFc            = DISABLED;
        mac_ability.nwayAbility     = DISABLED;
        mac_ability.masterMod       = DISABLED;
        mac_ability.nwayFault       = DISABLED;
        mac_ability.lpi_100m        = DISABLED;
        mac_ability.lpi_giga        = DISABLED;
        if((ret = dal_rtl9602bvb_port_macForceAbility_set(HAL_GET_PON_PORT(), mac_ability)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            return ret;
        }

        if((ret = dal_rtl9602bvb_port_macForceAbilityState_set(HAL_GET_PON_PORT(), ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            return ret;
        }
    }

    /*when EPON not registered to OLT only OAM packet can send out from pon port*/
    if((ret = dal_rtl9602bvb_oam_multiplexerAction_set(HAL_GET_PON_PORT(),OAM_MULTIPLEXER_ACTION_DISCARD)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    
    if ((ret = dal_rtl9602bvb_epon_fecOverhead_set(0xc)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }

    if ((ret = dal_rtl9602bvb_epon_churningKeyMode_set(RTK_EPON_CHURNING_BL)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
 
    if ((ret = dal_rtl9602bvb_epon_reportMode_set(RTK_EPON_REPORT_NORMAL)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    

    /*report include ifg*/
    data = 1;
    if ((ret = reg_field_write(RTL9602BVB_PON_DBA_IFGr,RTL9602BVB_DBA_IFGf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

    /*init EPON register*/
    
    /*reset EPON*/
    data = 0;
    if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_00r,RTL9602BVB_CFG_SFT_RSTB_EPONf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
        
    /*set laser parameter*/
    data = 0x24;
    if ((ret = reg_field_write(RTL9602BVB_LASER_ON_OFF_TIMEr,RTL9602BVB_LASER_ON_TIMEf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
    data = 5;
    if ((ret = reg_field_write(RTL9602BVB_LASER_ON_OFF_TIMEr,RTL9602BVB_LASER_OFF_TIMEf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

    data = 0xa;
    if ((ret = reg_field_write(RTL9602BVB_EPON_ASIC_TIMING_ADJUST2r,RTL9602BVB_LSR_ON_SHIFTf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
    data = 0xe;
    if ((ret = reg_field_write(RTL9602BVB_EPON_ASIC_TIMING_ADJUST2r,RTL9602BVB_LSR_OFF_SHIFTf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

    data = 0x3; /*ADJ_BC*/
    if ((ret = reg_field_write(RTL9602BVB_EPON_ASIC_TIMING_ADJUST2r,RTL9602BVB_ADJ_BCf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

    data = 0x4; /*ADJ RPT_TMG*/
    if ((ret = reg_field_write(RTL9602BVB_EPON_ASIC_TIMING_ADJUST1r,RTL9602BVB_RPT_TMGf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

    data = 0x1; /*bypass DS FEC*/
    if ((ret = reg_field_write(RTL9602BVB_EPON_FEC_CONFIGr,RTL9602BVB_BYPASS_FECf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

    /*
    RSVD_EGR_SCH[2:0]
    signal name:      gate_len_cmps
    description        adjusting gate length from gate packet:
                        3'd1 : gate length - 1*8
                        3'd2 : gate length - 2*8
                        3'd3 : gate length - 3*8
                        3'd4 : gate length - 4*8
                        3'd5 : gate length - 5*8
                        3'd6 : gate length - 6*8
                        3'd7 : gate length - 7*8
    */
    /*ioal_mem32_write(0x2de54, 0x0);*/
    data = 0;
    if((ret = reg_field_write(RTL9602BVB_EPON_GATE_CTRLr, RTL9602BVB_GATE_LENGTHf, &data))!=RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
    data = 0x1;
    if ((ret = reg_field_write(RTL9602BVB_EP_MISCr,RTL9602BVB_SRT_GNf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
    data = 0x1;
    if ((ret = reg_field_write(RTL9602BVB_EP_MISCr,RTL9602BVB_ALWAYS_SVYf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
     

    data = 1; /*release EPON reset*/
    if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_00r,RTL9602BVB_CFG_SFT_RSTB_EPONf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }


    /*register mode setting*/
 
        /*register pennding grant set to 4*/
    data = 4;
    if ((ret = reg_field_write(RTL9602BVB_EPON_RGSTR2r,RTL9602BVB_REG_PENDDING_GRANTf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
  
        /*register enable random delay*/
    data = 1;
    if ((ret = reg_field_write(RTL9602BVB_EPON_DEBUG1r,RTL9602BVB_DIS_RANDOM_DELAY_ENf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

        /*register ack flag fields set to 1*/
    data = 0x1;
    if ((ret = reg_field_write(RTL9602BVB_EPON_REG_ACKr,RTL9602BVB_ACK_FLAGf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

  

    /*invalid frame handle set to drop*/
    data = 0;
    if ((ret = reg_field_write(RTL9602BVB_EPON_DEBUG1r,RTL9602BVB_MODE0_INVALID_HDLf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

    data = 0;
    if ((ret = reg_field_write(RTL9602BVB_EPON_DEBUG1r,RTL9602BVB_MODE1_INVALID_HDLf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
#if 0  
    /*MPCP Gate frame handle set to asic handle*/
    data = 0x0;
    if ((ret = reg_field_write(RTL9602BVB_EPON_MPCP_CTRr,RTL9602BVB_GATE_TRAP_TYPEf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
    data = 0x0;
    if ((ret = reg_field_write(RTL9602BVB_EPON_MPCP_CTRr,RTL9602BVB_GATE_HANDLEf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
#endif

    
    /*invalid mpcp packet handle set to drop*/
    data = 0x0;
    if ((ret = reg_field_write(RTL9602BVB_EPON_MPCP_CTRr,RTL9602BVB_OTHER_HANDLEf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON| MOD_DAL), "");
        return ret;
    }

    data = 0x0;
    if ((ret = reg_field_write(RTL9602BVB_EPON_MPCP_CTRr,RTL9602BVB_INVALID_LEN_HANDLEf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
    
    /*decryption mode set to churnning key*/
    data = 0x1;
    if ((ret = reg_field_write(RTL9602BVB_EPON_DECRYP_CFGr,RTL9602BVB_EPON_DECRYPf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

    /*enable pbo dying gasp*/
    data = 0x1;
    if ((ret = reg_field_write(RTL9602BVB_PONIP_CTL_USr,RTL9602BVB_CFG_EPON_DYINGGASP_ENf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }    
 
    /*pon mac init*/
    if ((ret = rtk_ponmac_init()) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    
    data = 500;
    if ((ret = reg_field_write(RTL9602BVB_EPON_SCH_TIMINGr,RTL9602BVB_CFG_EPON_SCH_LATENCYf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }    
    
 
 
    thRpt.levelNum = 1;
    thRpt.th1 = 0;
    thRpt.th2 = 0;
    thRpt.th3 = 0;

    for(llidIdx=0;llidIdx<HAL_MAX_LLID_ENTRY();llidIdx++)
    {
        if ((ret = dal_rtl9602bvb_epon_thresholdReport_set(llidIdx, &thRpt)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            return ret;
        }
    }    
    

    /*
    =======================================
    RTL9602BVB_EPON_QUEUE_NUM_PER_LLID = 4
    =======================================
    LLID idx  logical_queue  oam_queue  mpcp_queue
    --------  -------------  ---------  ---------- 
     0        0~3            8          9  
     1        4~7            10         11  
     2        16~19          24         25  
     3        20~23          26         27  
     4        0~3            8          9   
     5        4~7            10         11  
     6        16~19          24         25  
     7        20~23          26         27  
    */
    for(llidIdx=0;llidIdx<HAL_MAX_LLID_ENTRY();llidIdx++)
    {
        uint32 queueId;
        uint32 logicalLlididx,queueIdx;

        logicalLlididx = llidIdx%4;
        /*mapping queue oam queue*/
        queueId = (logicalLlididx/2)*16 + 8 + (logicalLlididx%2)*2;                
        if((ret= _rtl9602bvb_epon_initPonQueue(llidIdx, queueId)) != RT_ERR_OK)
        {
            return ret;
        }

        if((ret= dal_rtl9602bvb_ponmac_sidValid_set(queueId, ENABLED)) != RT_ERR_OK)
        {
            return ret;
        }
        
        /*mapping queue mpcp queue*/
        queueId = (logicalLlididx/2)*16 + 9 + (logicalLlididx%2)*2;                
        if((ret= _rtl9602bvb_epon_initPonQueue(llidIdx, queueId)) != RT_ERR_OK)
        {
            return ret;
        }
        if((ret= dal_rtl9602bvb_ponmac_sidValid_set(queueId, ENABLED)) != RT_ERR_OK)
        {
            return ret;
        }

        /*mapping data queue*/
        
        if(RTL9602BVB_EPON_QUEUE_NUM_PER_LLID==4)
        {
            for(queueIdx = 0 ; queueIdx < RTL9602BVB_EPON_QUEUE_NUM_PER_LLID ; queueIdx++ )
            { 
                queueId = queueIdx + (logicalLlididx/2)*16 + (logicalLlididx%2)*4;
                if((ret= _rtl9602bvb_epon_initPonQueue(llidIdx, queueId)) != RT_ERR_OK)
                {
                    return ret;
                }
                /*TBD*/
                /*must remove when epon interrupt ready, data queue sid vaild must enable when llid enable*/
                if((ret= dal_rtl9602bvb_ponmac_sidValid_set(queueId, ENABLED)) != RT_ERR_OK)
                {
                    return ret;
                }

            }                   
        }
        else
        {
            /*for 8 queue support only LLID 0/2/4/6 would be used */
            if(llidIdx%2==1)
                continue;
                
            for(queueIdx = 0 ; queueIdx < RTL9602BVB_EPON_QUEUE_NUM_PER_LLID ; queueIdx++ )
            { 
                queueId = queueIdx + (logicalLlididx/2)*16 + (logicalLlididx%2)*4;
                if((ret= _rtl9602bvb_epon_initPonQueue(llidIdx, queueId)) != RT_ERR_OK)
                {
                    return ret;
                }
                /*TBD*/
                /*must remove when epon interrupt ready, data queue sid vaild must enable when llid enable*/
                if((ret= dal_rtl9602bvb_ponmac_sidValid_set(queueId, ENABLED)) != RT_ERR_OK)
                {
                    return ret;
                }

            }                   
        }
    }
  
     if ((ret = dal_rtl9602bvb_epon_isr_init()) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    } 

    /*EPON_MISC_CFG.GMII_RXER_EN = 0*/
    data = 0x0;
    if ((ret = reg_field_write(RTL9602BVB_EPON_MISC_CFGr,RTL9602BVB_GMII_RXER_ENf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

    /*CLR_INVLD_LID_GN.INVLD_LID_GN = 1*/
    data = 0x1;
    if ((ret = reg_field_write(RTL9602BVB_CLR_INVLD_LID_GNr,RTL9602BVB_INVLD_LID_GNf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }

    /*EPON_REG_BAK0[8]=0,  EPON_REG_BAK0[4]=0*/
    if ((ret = reg_field_read(RTL9602BVB_EPON_REG_BAK0r,RTL9602BVB_EPON_REG_BAK0f,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    data = data & 0xfffffeef;
    if ((ret = reg_field_write(RTL9602BVB_EPON_REG_BAK0r,RTL9602BVB_EPON_REG_BAK0f,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

    /*init discovery random seed*/
    data = 0;
    if ((ret = reg_field_write(RTL9602BVB_EPON_RDM_SEEDr,RTL9602BVB_EPON_RDM_SEEDf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    data = 0;
    if ((ret = reg_field_write(RTL9602BVB_EPON_DEBUG1r,RTL9602BVB_DIS_RANDOM_DELAY_ENf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    data = 1;
    if ((ret = reg_field_write(RTL9602BVB_EPON_DEBUG1r,RTL9602BVB_DIS_RANDOM_DELAY_ENf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

   
    /*for test chip disable INVLD_LID_GN option*/
    data = 0;
    if ((ret = reg_field_write(RTL9602BVB_CLR_INVLD_LID_GNr,RTL9602BVB_INVLD_LID_GNf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    


    /*for epon always 8 queue enable, must set pbo threshold*/
    if ((ret = rtl9602bvb_raw_pbo_globalThreshold_set(6200,6100)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    } 

#ifdef FPGA_DEFINED
    /*only fpga need set this field to 0*/
    data  = 0;
    if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_02r,RTL9602BVB_REG_BEN_SYNC_SELf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
#endif

    epon_init = INIT_COMPLETED;
    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_init */




/* Function Name:
 *      dal_rtl9602bvb_epon_intrMask_get
 * Description:
 *      Get EPON interrupt mask
 * Input:
 *	  intrType: type of interrupt
 * Output:
 *      pState: point of get interrupt state
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_intrMask_get(rtk_epon_intrType_t intrType, rtk_enable_t *pState)
{
	uint32  tmpVal;   
 	int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "intrType=%d",intrType);

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((EPON_INTR_END <=intrType), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    /* function body */
    switch(intrType)
    {
        case EPON_INTR_TIMEDRIFT:
            if ((ret = reg_field_read(RTL9602BVB_EPON_INTRr,RTL9602BVB_TIME_DRIFT_IMRf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
                return ret;
            }
            break;    
        case EPON_INTR_MPCPTIMEOUT:
            if ((ret = reg_field_read(RTL9602BVB_EPON_INTRr,RTL9602BVB_MPCP_TIMEOUT_IMRf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
                return ret;
            }
            break;    
        case EPON_INTR_REG_FIN:
            if ((ret = reg_field_read(RTL9602BVB_EPON_INTRr,RTL9602BVB_REG_COMPLETE_IMSf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
                return ret;
            }
            break; 
        case EPON_INTR_LOS:
            if ((ret = reg_field_read(RTL9602BVB_EPON_INTRr,RTL9602BVB_LOS_IMSf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
                return ret;
            }
            break; 

        default:
            return RT_ERR_INPUT;       
        
    }
    
    if(tmpVal==0)
        *pState = DISABLED;    
    else
        *pState = ENABLED;

       
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_intrMask_get */


/* Function Name:
 *      dal_rtl9602bvb_epon_intrMask_set
 * Description:
 *      Set EPON interrupt mask
 * Input:
 * 	    intrType: type of top interrupt
 *      state: set interrupt mask state
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_intrMask_set(rtk_epon_intrType_t intrType, rtk_enable_t state)
{
	uint32  tmpVal;   
 	int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "intrType=%d,state=%d",intrType, state);

    /* parameter check */
    RT_PARAM_CHK((EPON_INTR_END <=intrType), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_ENABLE_END <= state), RT_ERR_INPUT);

    /* function body */
    if(state==ENABLED)
        tmpVal = 1;    
    else
        tmpVal = 0;
        
    switch(intrType)
    {
        case EPON_INTR_TIMEDRIFT:
            if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_TIME_DRIFT_IMRf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
                return ret;
            }
            break;    
        case EPON_INTR_MPCPTIMEOUT:
            if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_MPCP_TIMEOUT_IMRf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
                return ret;
            }
            break;    
        case EPON_INTR_REG_FIN:
            if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_REG_COMPLETE_IMRf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
                return ret;
            }
            break; 
        case EPON_INTR_LOS:
            if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_LOS_IMRf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
                return ret;
            }
            break;
        case EPON_INTR_1PPS:
            if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_EPON_1PPS_IMRf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
                return ret;
            }
            break;
        case EPON_INTR_FEC:
            if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_RCOV_RXFEC_IMRf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
                return ret;
            }
            break;
        default:
            return RT_ERR_INPUT;       
        
    }
     
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_intrMask_set */




/* Function Name:
 *      dal_rtl9602bvb_epon_intr_get
 * Description:
 *      Set EPON interrupt state
 * Input:
 * 	  intrType: type of interrupt
 * Output:
 *      pState: point for get  interrupt state
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_intr_get(rtk_epon_intrType_t intrType, rtk_enable_t *pState)
{
 	int32   ret;
    uint32  field,tmp_val;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "intrType=%d",intrType);

    /* parameter check */
    RT_PARAM_CHK((EPON_INTR_END <=intrType), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    /* function body */
    switch(intrType)
    {
        case EPON_INTR_TIMEDRIFT:
            field = RTL9602BVB_TIME_DRIFT_IMSf; 
            break;    
        case EPON_INTR_MPCPTIMEOUT:
            field = RTL9602BVB_MPCP_TIMEOUT_IMSf; 
            break;    
        case EPON_INTR_REG_FIN:
            field = RTL9602BVB_REG_COMPLETE_IMSf; 
            break; 
        case EPON_INTR_LOS:
            field = RTL9602BVB_LOS_IMSf; 
            break; 
        case EPON_INTR_1PPS:
            field = RTL9602BVB_EPON_1PPS_IMSf; 
            break;
        case EPON_INTR_FEC:
            field = RTL9602BVB_RCOV_RXFEC_IMSf; 
            break;
            
        default:
            return RT_ERR_INPUT;       
        
    }


    if ((ret = reg_field_read(RTL9602BVB_EPON_INTRr,field,&tmp_val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    if(tmp_val == 0)
    {
        *pState = ENABLED;
        tmp_val = 1;
        if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,field,&tmp_val)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
            return ret;
        }
        
    }
    else
    {
        *pState = DISABLED;
    }


    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_gpon_intr_get */


/* Function Name:
 *      dal_rtl9602bvb_epon_intr_disableAll
 * Description:
 *      Disable all of top interrupt for EPON
 * Input:
 *
 * Output:
 *      
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_intr_disableAll(void)  
{
	uint32  tmpVal;   
 	int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /*clear all interrupt status*/
    tmpVal = 1;
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_TIME_DRIFT_IMSf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_MPCP_TIMEOUT_IMSf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_REG_COMPLETE_IMSf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_LOS_IMSf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_EPON_1PPS_IMSf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_RCOV_RXFEC_IMSf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    
    /* function body */
    /*disable all interrupt*/
    tmpVal = 0;
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_TIME_DRIFT_IMRf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_MPCP_TIMEOUT_IMRf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_REG_COMPLETE_IMRf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_LOS_IMRf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_RCOV_RXFEC_IMRf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }    
    if ((ret = reg_field_write(RTL9602BVB_EPON_INTRr,RTL9602BVB_EPON_1PPS_IMRf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_intr_disableAll */



/* Function Name:
 *      dal_rtl9602bvb_epon_llid_entry_set
 * Description:
 *      Set llid entry
 * Input:
 *      llidIdx: LLID table index
 * Output:
 *      pLlidEntry: LLID entry 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_llid_entry_set(rtk_epon_llid_entry_t *pLlidEntry)  
{
    rtl9602bvb_raw_epon_llid_table_t  rawLlidEntry;
 	int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pLlidEntry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((RTK_EPON_MAX_LLID <= pLlidEntry->llidIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_ENABLE_END <= pLlidEntry->valid), RT_ERR_INPUT);

    /* function body */
    rawLlidEntry.llid                = pLlidEntry->llid;
    rawLlidEntry.report_timer        = pLlidEntry->reportTimer;
    if(ENABLED == pLlidEntry->valid )
        rawLlidEntry.valid = 1;
    else
        rawLlidEntry.valid = 0;
#if 0
    /*not llid valid must link-down pon port*/
    if((pLlidEntry->llidIdx==0) && (DISABLED == pLlidEntry->valid))
    {
        rtk_port_macAbility_t mac_ability;
        uint32 queueId;
        rtk_ponmac_queue_t pon_queue;

        /*drain out all epon packet*/
        
        /*dying gasp queue*/
        pon_queue.schedulerId = 0;
        pon_queue.queueId = 31;
    	if((ret = rtk_ponmac_queueDrainOut_set(&pon_queue))!=RT_ERR_OK)
    	{
    		 RT_ERR(ret, (MOD_DAL|MOD_GPON), "");
    	    	return ret;
    	}
    	for(queueId=0;queueId<9;queueId++)
    	{
            pon_queue.schedulerId = 0;
            pon_queue.queueId = queueId;

        	if((ret = rtk_ponmac_queueDrainOut_set(&pon_queue))!=RT_ERR_OK)
        	{
        		 RT_ERR(ret, (MOD_DAL|MOD_GPON), "");
        	    	return ret;
        	}
        }            
    }
#endif

    if ((ret = rtl9602bvb_raw_epon_llidTable_set(pLlidEntry->llidIdx,&rawLlidEntry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }

    if ((ret = rtl9602bvb_raw_epon_llidIdxMac_set(pLlidEntry->llidIdx, &(pLlidEntry->mac))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_llid_entry_set */

/* Function Name:
 *      dal_rtl9602bvb_epon_llid_entry_get
 * Description:
 *      Get llid entry
 * Input:
 *      llidIdx: LLID table index
 * Output:
 *      pLlidEntry: LLID entry 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_llid_entry_get(rtk_epon_llid_entry_t *pLlidEntry)  
{
    rtl9602bvb_raw_epon_llid_table_t  rawLlidEntry;
 	int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pLlidEntry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((RTK_EPON_MAX_LLID <= pLlidEntry->llidIdx), RT_ERR_INPUT);
    
    
    /* function body */
    if ((ret = rtl9602bvb_raw_epon_llidTable_get(pLlidEntry->llidIdx,&rawLlidEntry)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }

    pLlidEntry->llid        = rawLlidEntry.llid;
    pLlidEntry->reportTimer = rawLlidEntry.report_timer;

    if(1 == rawLlidEntry.valid)
        pLlidEntry->valid = ENABLED;
    else
        pLlidEntry->valid = DISABLED;
    
    if(1 == rawLlidEntry.is_report_timeout)
        pLlidEntry->isReportTimeout = ENABLED;
    else
        pLlidEntry->isReportTimeout = DISABLED;


    if ((ret = rtl9602bvb_raw_epon_llidIdxMac_get(pLlidEntry->llidIdx, &(pLlidEntry->mac))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_llid_entry_get */



/* Function Name:
 *      dal_rtl9602bvb_epon_opticalPolarity_get
 * Description:
 *      Set OE module polarity.
 * Input:
 *      pPolarity - pointer of OE module polarity
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_epon_opticalPolarity_get(rtk_epon_polarity_t *pPolarity)
{

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPolarity), RT_ERR_NULL_POINTER);

    /* function body */
    *pPolarity = oe_polarity;
        
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_opticalPolarity_get */



/* Function Name:
 *      dal_9602bvb_epon_opticalPolarity_set
 * Description:
 *      Set OE module polarity.
 * Input:
 *      polarity - OE module polarity
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_epon_opticalPolarity_set(rtk_epon_polarity_t polarity)
{
 	int32   ret;
	uint32  tmpVal;   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "polarity=%d",polarity);

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((EPON_POLARITY_END <=polarity), RT_ERR_INPUT);

    /* function body */
    oe_polarity = polarity;

    if(EPON_POLARITY_HIGH == polarity)
    {
        tmpVal = 0;
    }
    else
    {
        tmpVal = 1;
    }
    if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_12r,RTL9602BVB_CFG_FRC_BEN_INVf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
     
    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_opticalPolarity_set */



/* Function Name:
 *      dal_rtl9602bvb_epon_forceLaserState_set
 * Description:
 *      Set Force Laser status
 * Input:
 *      laserStatus: Force laser status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32  dal_rtl9602bvb_epon_forceLaserState_set(rtk_epon_laser_status_t laserStatus)
{
 	int32   ret;
    rtk_epon_polarity_t tempPolarity;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "laserStatus=%d",laserStatus);

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_EPON_LASER_STATUS_END <=laserStatus), RT_ERR_INPUT);

    /* function body */
    if(RTK_EPON_LASER_STATUS_NORMAL==laserStatus)
    {
        /*set OE polarity to original mode*/
        if ((ret = dal_rtl9602bvb_epon_opticalPolarity_set(oe_polarity)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            return ret;
        }

        /*set force laser on disable*/
        if ((ret = rtl9602bvb_raw_epon_forceLaserOn_set(DISABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            return ret;
        }
    }
    if(RTK_EPON_LASER_STATUS_FORCE_ON==laserStatus)
    {
        /*set OE polarity to original mode*/
        if ((ret = dal_rtl9602bvb_epon_opticalPolarity_set(oe_polarity)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            return ret;
        }
        /*set force laser on enable*/
        if ((ret = rtl9602bvb_raw_epon_forceLaserOn_set(ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            return ret;
        }
    }
    if(RTK_EPON_LASER_STATUS_FORCE_OFF==laserStatus)
    {
        if(EPON_POLARITY_LOW==oe_polarity)
            tempPolarity=EPON_POLARITY_HIGH;
        else
            tempPolarity=EPON_POLARITY_LOW;
        
        /*set OE polarity to original mode*/
        if ((ret = dal_rtl9602bvb_epon_opticalPolarity_set(tempPolarity)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            return ret;
        }

        /*set force laser on enable*/
        if ((ret = rtl9602bvb_raw_epon_forceLaserOn_set(ENABLED)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
            return ret;
        }
    }

    forceLaserState = laserStatus;

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_forceLaserState_set */

/* Function Name:
 *      dal_rtl9602bvb_epon_forceLaserState_get
 * Description:
 *      Get Force Laser status
 * Input:
 *      None
 * Output:
 *      pLaserStatus: Force laser status
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_forceLaserState_get(rtk_epon_laser_status_t *pLaserStatus)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pLaserStatus), RT_ERR_NULL_POINTER);

    /* function body */
    *pLaserStatus=forceLaserState;

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_forceLaserState_get */


/* Function Name:
 *      dal_rtl9602bvb_epon_laserTime_set
 * Description:
 *      Set laserTime value
 * Input:
 *      laserOnTime:  OE module laser on time
 *      laserOffTime: OE module laser off time
 * Output:
 *      None 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32  dal_rtl9602bvb_epon_laserTime_set(uint8 laserOnTime, uint8 laserOffTime)
{
 	int32   ret;
    uint32  val;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "laserOnTime=%d,laserOffTime=%d",laserOnTime, laserOffTime);

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_EPON_MAX_LASER_ON_TIME <=laserOnTime), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_EPON_MAX_LASER_OFF_TIME <=laserOffTime), RT_ERR_INPUT);
    val = laserOnTime;
    /* function body */
    if ((ret = reg_field_write(RTL9602BVB_LASER_ON_OFF_TIMEr,RTL9602BVB_LASER_ON_TIMEf,&val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    val = laserOffTime;
    if ((ret = reg_field_write(RTL9602BVB_LASER_ON_OFF_TIMEr,RTL9602BVB_LASER_OFF_TIMEf,&val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }


    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_laserTime_set */

/* Function Name:
 *      dal_rtl9602bvb_epon_laserTime_get
 * Description:
 *      Get laser Time value
 * Input:
 *      None
 * Output:
 *      pLasetOnTime:  OE module laser on time
 *      pLasetOffTime: OE module laser off time
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_laserTime_get(uint8 *pLaserOnTime, uint8 *pLaserOffTime)
{
	uint32  tmpVal;   
 	int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON),"");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pLaserOnTime), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pLaserOffTime), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_field_read(RTL9602BVB_LASER_ON_OFF_TIMEr,RTL9602BVB_LASER_ON_TIMEf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    *pLaserOnTime = tmpVal;
    
    if ((ret = reg_field_read(RTL9602BVB_LASER_ON_OFF_TIMEr,RTL9602BVB_LASER_OFF_TIMEf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    *pLaserOffTime = tmpVal;

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_laserTime_get */

/* Function Name:
 *      dal_rtl9602bvb_epon_syncTime_get
 * Description:
 *      Get sync Time value
 * Input:
 *      None
 * Output:
 *      pSyncTime  : olt assigned sync time
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_syncTime_get(uint8 *pSyncTime)
{
	uint32  tmpVal;   
 	int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pSyncTime), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_field_read(RTL9602BVB_SYNC_TIMEr,RTL9602BVB_NORMAL_SYNC_TIMEf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    *pSyncTime = tmpVal;
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_syncTime_get */


/* Function Name:
 *      dal_rtl9602bvb_epon_registerReq_get
 * Description:
 *      Get register request relative parameter
 * Input:
 *      pRegEntry : register request relative parament
 * Output:
 *       None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_registerReq_get(rtk_epon_regReq_t *pRegEntry)
{
	uint32  tmpVal;   
 	int32   ret;
    rtk_enable_t enable;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pRegEntry), RT_ERR_NULL_POINTER);

    /* function body */

    if ((ret = rtl9602bvb_raw_epon_regLlidIdx_get(&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    pRegEntry->llidIdx = tmpVal;

#if 0    
    if ((ret = rtl9602bvb_raw_epon_llidIdxMac_get(pRegEntry->llidIdx,&(pRegEntry->mac))) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
#endif
    
    if ((ret = rtl9602bvb_raw_epon_regPendingGrantNum_get(&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    pRegEntry->pendGrantNum = tmpVal;

    if ((ret = rtl9602bvb_raw_epon_regReguest_get(&enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
    pRegEntry->doRequest = enable;

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_registerReq_get */

/* Function Name:
 *      dal_rtl9602bvb_epon_registerReq_set
 * Description:
 *      Set register request relative parameter
 * Input:
 *       None
 * Output:
 *       pRegEntry : register request relative parament 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_registerReq_set(rtk_epon_regReq_t *pRegEntry)
{
 	int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pRegEntry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((RTK_EPON_MAX_LLID <= pRegEntry->llidIdx), RT_ERR_INPUT);

    /* function body */
    if ((ret = rtl9602bvb_raw_epon_regLlidIdx_set(pRegEntry->llidIdx)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }

    if ((ret = rtl9602bvb_raw_epon_regPendingGrantNum_set(pRegEntry->pendGrantNum)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }

    /*enable EPON register interrupt*/
    if ((ret = dal_rtl9602bvb_epon_intrMask_set(EPON_INTR_REG_FIN,ENABLED)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }
        
    if ((ret = rtl9602bvb_raw_epon_regReguest_set(pRegEntry->doRequest)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_EPON), "");
        return ret;
    }

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_registerReq_set */



/* Function Name:
 *      dal_rtl9602bvb_epon_churningKey_set
 * Description:
 *      Set churning key entry
 * Input:
 *       pEntry : churning key relative parameter 
 * Output:
 *       None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_churningKey_set(rtk_epon_churningKeyEntry_t *pEntry)
{
 	int32   ret;
    uint8   pKeyTmp[4];

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEntry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((2 <= pEntry->keyIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_EPON_MAX_LLID <= pEntry->llidIdx), RT_ERR_INPUT);

    pKeyTmp[0] = 0;
    pKeyTmp[1] = pEntry->churningKey[0];
    pKeyTmp[2] = pEntry->churningKey[1];
    pKeyTmp[3] = pEntry->churningKey[2];
    
    if(0==pEntry->keyIdx)
    {
        if ((ret = reg_array_field_write(RTL9602BVB_EPON_DECRYP_KEY0r, REG_ARRAY_INDEX_NONE, pEntry->llidIdx, RTL9602BVB_EPON_DECRYP_KEY0f, (uint32 *)pKeyTmp)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_QOS|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_array_field_write(RTL9602BVB_EPON_DECRYP_KEY1r, REG_ARRAY_INDEX_NONE, pEntry->llidIdx, RTL9602BVB_EPON_DECRYP_KEY1f, (uint32 *)pKeyTmp)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_QOS|MOD_DAL), "");
            return ret;
        }
    }

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_churningKey_set */


/* Function Name:
 *      dal_rtl9602bvb_epon_churningKey_get
 * Description:
 *      Get churning key entry
 * Input:
 *       None
 * Output:
 *       pEntry : churning key relative parameter 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_churningKey_get(rtk_epon_churningKeyEntry_t *pEntry)
{
	uint32  tmpVal;   
    uint8   *pTmpPtr;
 	int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pEntry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((2 <= pEntry->keyIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_EPON_MAX_LLID <= pEntry->llidIdx), RT_ERR_INPUT);

    if(0==pEntry->keyIdx)
    {
        if ((ret = reg_array_field_read(RTL9602BVB_EPON_DECRYP_KEY0r, pEntry->llidIdx, REG_ARRAY_INDEX_NONE, RTL9602BVB_EPON_DECRYP_KEY0f, (uint32 *)&tmpVal)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_QOS|MOD_DAL), "");
            return ret;
        }
    }
    else
    {
        if ((ret = reg_array_field_read(RTL9602BVB_EPON_DECRYP_KEY1r, pEntry->llidIdx, REG_ARRAY_INDEX_NONE, RTL9602BVB_EPON_DECRYP_KEY1f, (uint32 *)&tmpVal)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_QOS|MOD_DAL), "");
            return ret;
        }
    }
    pTmpPtr = (uint8 *)&tmpVal;
    pEntry->churningKey[0] = pTmpPtr[0];
    pEntry->churningKey[1] = pTmpPtr[1];
    pEntry->churningKey[2] = pTmpPtr[2];
    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_churningKey_set */


/* Function Name:
 *      dal_rtl9602bvb_epon_usFecState_get
 * Description:
 *      Get upstream fec state
 * Input:
 *       None
 * Output:
 *       *pState : upstream FEC state 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_usFecState_get(rtk_enable_t *pState)
{
 	int32   ret;
	uint32  tmpVal;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* parameter check */
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_field_read(RTL9602BVB_EPON_FEC_CONFIGr,RTL9602BVB_FEC_US_ENf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    if(1==tmpVal)
        *pState = ENABLED;
    else
        *pState = DISABLED;
   
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_usFecState_get */


/* Function Name:
 *      dal_rtl9602bvb_epon_usFecState_set
 * Description:
 *      Set upstream fec state
 * Input:
 *       state : upstream FEC state
 * Output:
 *       None 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_usFecState_set(rtk_enable_t state)
{
 	int32   ret;
	uint32  tmpVal;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "state=%d",state);

    /* parameter check */
    RT_PARAM_CHK((RTK_ENABLE_END <= state), RT_ERR_INPUT);

    /* function body */
    if(ENABLED==state)
    {
        tmpVal =1;
        /*for US FEC enable must enable BPO first*/
        if ((ret = reg_field_write(RTL9602BVB_PONIP_CTL_USr,RTL9602BVB_CFG_FEC_ONf,&tmpVal)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
            return ret;
        }

        if ((ret = reg_field_write(RTL9602BVB_EPON_FEC_CONFIGr,RTL9602BVB_FEC_US_ENf,&tmpVal)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
            return ret;
        }

    }
    else
    {
        tmpVal =0;
        /*for US FEC enable must disable EPON mac us-fec first*/
        if ((ret = reg_field_write(RTL9602BVB_EPON_FEC_CONFIGr,RTL9602BVB_FEC_US_ENf,&tmpVal)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
            return ret;
        }

        if ((ret = reg_field_write(RTL9602BVB_PONIP_CTL_USr,RTL9602BVB_CFG_FEC_ONf,&tmpVal)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
            return ret;
        }
    }    

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_usFecState_set */


/* Function Name:
 *      dal_rtl9602bvb_epon_dsFecState_get
 * Description:
 *      Get down-stream fec state
 * Input:
 *       None
 * Output:
 *       *pState : down-stream FEC state 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_dsFecState_get(rtk_enable_t *pState)
{
 	int32   ret;
	uint32  tmpVal;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* parameter check */
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_field_read(RTL9602BVB_EPON_FEC_CONFIGr,RTL9602BVB_BYPASS_FECf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    if(0==tmpVal)
        *pState = ENABLED;
    else
        *pState = DISABLED;
    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_dsFecState_get */


/* Function Name:
 *      dal_rtl9602bvb_epon_dsFecState_set
 * Description:
 *      Set down-stream fec state
 * Input:
 *       state : down-stream FEC state
 * Output:
 *       None 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_dsFecState_set(rtk_enable_t state)
{
 	int32   ret;
	uint32  tmpVal,recoverFec,data;
	
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "state=%d",state);

    /* parameter check */
    RT_PARAM_CHK((RTK_ENABLE_END <= state), RT_ERR_INPUT);

    /* function body */
    if(ENABLED==state)
    {
        tmpVal =0;
        recoverFec = 1;
    }
    else
    {
        tmpVal =1;
        recoverFec = 0;
    }
    
    /*reset EPON*/
    data = 0;
    if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_00r,RTL9602BVB_CFG_SFT_RSTB_EPONf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
    
    if ((ret = reg_field_write(RTL9602BVB_EPON_FEC_CONFIGr,RTL9602BVB_BYPASS_FECf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

#if 0
    if ((ret = reg_field_write(RTL9602BVB_EPON_FEC_CONFIGr,RTL9602BVB_FEC_RECOVERf,&recoverFec)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
#endif
    
    data = 1;
    if ((ret = reg_field_write(RTL9602BVB_WSDS_DIG_00r,RTL9602BVB_CFG_SFT_RSTB_EPONf,&data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON | MOD_DAL), "");
        return ret;
    }
    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_dsFecState_set */


/* Function Name:
 *      dal_rtl9602bvb_epon_dsFecState_set
 * Description:
 *      Set down-stream fec state
 * Input:
 *       None
 * Output:
 *       pCounter : EPON mib counter 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_mibCounter_get(rtk_epon_counter_t *pCounter)
{
 	int32   ret,i;
	uint32  tmpVal;
    uint32 queueId,queueBase;
  

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");


    /* parameter check */
    RT_PARAM_CHK((NULL == pCounter), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((RTK_EPON_MAX_LLID <= pCounter->llidIdx), RT_ERR_INPUT);

    /* function body */
    /*get global counter*/
    if ((ret = reg_field_read(RTL9602BVB_DOT3_MPCP_RX_DISCr,RTL9602BVB_DOT3MPCPRXDISCOVERYGATEf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->mpcpRxDiscGate=tmpVal;

    if ((ret = reg_field_read(RTL9602BVB_DOT3_EPON_FEC_UNCORRECTED_BLOCKSr,RTL9602BVB_DOT3EPONFECUNCORRECTABLEBLOCKSf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->fecUncorrectedBlocks=tmpVal;

    if ((ret = reg_field_read(RTL9602BVB_DOT3_EPON_FEC_CORRECTED_BLOCKSr,RTL9602BVB_DOT3EPONFECCORRECTEDBLOCKSf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->fecCorrectedBlocks=tmpVal;    
#if 0
    if ((ret = reg_field_read(RTL9602BVB_DOT3_NOT_BROADCAST_LLID_7FFFr,RTL9602BVB_NOTBROADCASTLLID7FFFf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->notBcstBitLlid7fff=tmpVal;
#endif
    if ((ret = reg_field_read(RTL9602BVB_DOT3_NOT_BROADCAST_BIT_NOT_ONU_LLIDr,RTL9602BVB_NOTBROADCASTBITNOTONULLIDf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->notBcstBitNotOnuLlid=tmpVal;

    if ((ret = reg_field_read(RTL9602BVB_DOT3_BROADCAST_BIT_PLUS_ONU_LLIDr,RTL9602BVB_BROADCASTBITPLUSONULLIDf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->bcstBitPlusOnuLLid=tmpVal;

    if ((ret = reg_field_read(RTL9602BVB_DOT3_BROADCAST_NOT_ONUIDr,RTL9602BVB_BROADCASTBITNOTONULLIDf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->bcstNotOnuLLid=tmpVal;

    if ((ret = reg_field_read(RTL9602BVB_DOT3_CRC8_ERRORSr,RTL9602BVB_CRC8ERRORSf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->crc8Err=tmpVal;

    if ((ret = reg_field_read(RTL9602BVB_DOT3_MPCP_TX_REG_REQr,RTL9602BVB_DOT3MPCPTXREGREQUESTf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->mpcpTxRegRequest=tmpVal;
    if ((ret = reg_field_read(RTL9602BVB_DOT3_MPCP_TX_REG_ACKr,RTL9602BVB_MPCPTXREGACKf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->mpcpTxRegAck=tmpVal;
    /*get llid counter*/
    if ((ret = reg_array_field_read(RTL9602BVB_DOT3_MPCP_TX_REPORTr, REG_ARRAY_INDEX_NONE, pCounter->llidIdx, RTL9602BVB_DOT3MPCPTXREPORTf, &tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    pCounter->llidIdxCnt.mpcpTxReport=tmpVal;

    if ((ret = reg_array_field_read(RTL9602BVB_DOT3_MPCP_RX_GATEr, REG_ARRAY_INDEX_NONE, pCounter->llidIdx, RTL9602BVB_DOT3MPCPRXGATEf, &tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    pCounter->llidIdxCnt.mpcpRxGate=tmpVal;

    if ((ret = reg_array_field_read(RTL9602BVB_DOT3_ONUID_NOT_BROADCASTr, REG_ARRAY_INDEX_NONE, pCounter->llidIdx, RTL9602BVB_ONULLIDNOTBROADCASTf, &tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pCounter->llidIdxCnt.onuLlidNotBcst=tmpVal;

    /*LLID Queue basic counter*/
    queueBase=(pCounter->llidIdx/2)*16;

 
    for(i=0;i<10;i++)
    {
        if(8==i)
        {
            if(pCounter->llidIdx%2 == 0)
                queueId = queueBase+8;
            else
                queueId = queueBase+10;
        }    
        else if(9==i)
        {
            if(pCounter->llidIdx%2 == 0)
                queueId = queueBase+9;
            else
                queueId = queueBase+11;
        }    
        else
        {
            queueId = queueBase+i;
        }
        
        if ((ret = reg_array_field_read(RTL9602BVB_DOT3_Q_TX_FRAMESr, REG_ARRAY_INDEX_NONE, queueId, RTL9602BVB_DOT3EXTPKGSTATTXFRAMESQUEUEf, &tmpVal)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
            return ret;
        }    
        pCounter->llidIdxCnt.queueTxFrames[i]=tmpVal;        
    }

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_mibCounter_get */



/* Function Name:
 *      dal_rtl9602bvb_epon_mibGlobal_reset
 * Description:
 *      Reset EPON global counters.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not reset Global Counter
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_epon_mibGlobal_reset(void)
{
 	int32   ret;
	uint32  tmpVal;   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* parameter check */
    
    /* function body */
	tmpVal = 1;
    if ((ret = reg_field_write(RTL9602BVB_STAT_RST_CFGr,RTL9602BVB_RST_EPON_MIBf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    /*trigger mib reset*/
	tmpVal = 1;
    if ((ret = reg_field_write(RTL9602BVB_STAT_RST_CFGr,RTL9602BVB_RST_CMDf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

    /*check busy flag*/
    do{
        if ((ret = reg_field_write(RTL9602BVB_STAT_RSTr, RTL9602BVB_RST_STATf, &tmpVal)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
            return ret;
        }
    }while(tmpVal == 0);
    
    /*set to default*/
	tmpVal = 0;
    if ((ret = reg_field_write(RTL9602BVB_STAT_RST_CFGr,RTL9602BVB_RST_EPON_MIBf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_mibGlobal_reset */


/* Function Name:
 *      dal_rtl9602bvb_epon_mibLlidIdx_reset
 * Description:
 *      Reset the specified LLID index counters.
 * Input:
 *      llidIdx - LLID table index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_epon_mibLlidIdx_reset(uint8 llidIdx)
{
 	int32   ret;
	uint32  tmpVal;   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "llidIdx=%d",llidIdx);

    /* parameter check */
    RT_PARAM_CHK((RTK_EPON_MAX_LLID <=llidIdx), RT_ERR_INPUT);

    /* function body */
	tmpVal = 1;
    if ((ret = reg_field_write(RTL9602BVB_STAT_RST_CFGr,RTL9602BVB_RST_LLIDf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

	tmpVal = llidIdx;
    if ((ret = reg_field_write(RTL9602BVB_STAT_RST_CFGr,RTL9602BVB_RST_LLID_IDXf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    /*trigger mib reset*/
	tmpVal = 1;
    if ((ret = reg_field_write(RTL9602BVB_STAT_RST_CFGr,RTL9602BVB_RST_CMDf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    /*check busy flag*/
    do{
        if ((ret = reg_field_write(RTL9602BVB_STAT_RSTr, RTL9602BVB_RST_STATf, &tmpVal)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
            return ret;
        }
    }while(tmpVal == 0);
    
    /*set to default*/
	tmpVal = 0;
    if ((ret = reg_field_write(RTL9602BVB_STAT_RST_CFGr,RTL9602BVB_RST_EPON_MIBf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_mibLlidIdx_reset */


int32
dal_rtl9602bvb_epon_churningKeyMode_set(rtk_epon_churning_mode_t mode)
{
 	int32   ret;
	uint32  tmpVal;   

    /* parameter check */
    RT_PARAM_CHK((RTK_EPON_CHURNING_END <=mode), RT_ERR_INPUT);

    /* function body */
	if(RTK_EPON_CHURNING_BL == mode)
	    tmpVal = 1;
    else
	    tmpVal = 0;
    
    if ((ret = reg_field_write(RTL9602BVB_EPON_DEBUG1r,RTL9602BVB_CHURN_MODEf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
    
}    



int32
dal_rtl9602bvb_epon_fecOverhead_set(uint32 val)
{
 	int32   ret;
	uint32  tmpVal;   

    /* parameter check */
    RT_PARAM_CHK((64 <= val), RT_ERR_INPUT);

    /* function body */
    tmpVal = val;
    if ((ret = reg_field_write(RTL9602BVB_EPON_FEC_CONFIGr,RTL9602BVB_FEC_OVER_TXf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    return RT_ERR_OK;
}    




int32
dal_rtl9602bvb_epon_reportMode_set(rtk_epon_report_mode_t mode)
{
 	int32   ret; 
	uint32  tmpVal; 

    /* parameter check */
    RT_PARAM_CHK((RTK_EPON_REPORT_END <= mode), RT_ERR_INPUT);

    switch(mode)
    {
        case RTK_EPON_REPORT_NORMAL:
            tmpVal = 0;
            if ((ret = reg_field_write(RTL9602BVB_EPON_REPORT_MODEr,RTL9602BVB_REPORT_MODEf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                return ret;
            }
            break;
        case RTK_EPON_REPORT_0_F:
            tmpVal = 4;
            if ((ret = reg_field_write(RTL9602BVB_EPON_REPORT_MODEr,RTL9602BVB_REPORT_MODEf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                return ret;
            }
            break;
        case RTK_EPON_REPORT_FORCE_0:
            tmpVal = 2;
            if ((ret = reg_field_write(RTL9602BVB_EPON_REPORT_MODEr,RTL9602BVB_REPORT_MODEf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                return ret;
            }
            break;
        case RTK_EPON_REPORT_FORCE_F:
            tmpVal = 3;
            if ((ret = reg_field_write(RTL9602BVB_EPON_REPORT_MODEr,RTL9602BVB_REPORT_MODEf,&tmpVal)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
                return ret;
            }
            break;

        default:
            return RT_ERR_INPUT;         
    }

    return RT_ERR_OK;
}    





int32
dal_rtl9602bvb_epon_reportMode_get(rtk_epon_report_mode_t *pMode)
{
 	int32   ret;
	uint32  tmpVal; 

    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);
    *pMode = RTK_EPON_REPORT_END;

    /* function body */
    if ((ret = reg_field_read(RTL9602BVB_EPON_REPORT_MODEr,RTL9602BVB_REPORT_MODEf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }

    switch(tmpVal)
    {
        case 0:
            *pMode = RTK_EPON_REPORT_NORMAL;
            break;    
        case 2:
            *pMode = RTK_EPON_REPORT_FORCE_0;
            break;    
        case 3:
            *pMode = RTK_EPON_REPORT_FORCE_F;
            break;    
        case 4:
            *pMode = RTK_EPON_REPORT_0_F;
            break;    
        
        default:
            *pMode = RTK_EPON_REPORT_END;
            return RT_ERR_FAILED;
            break;
    }
    return RT_ERR_OK;
}    


/* Function Name:
 *      dal_rtl9602bvb_epon_mpcpTimeoutVal_get
 * Description:
 *      Get mpcp time out value.
 * Input:
 *      pTimeVal - pointer of mpcp tomeout value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_epon_mpcpTimeoutVal_get(uint32 *pTimeVal)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pTimeVal), RT_ERR_NULL_POINTER);

    /* function body */

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_mpcpTimeoutVal_get */



/* Function Name:
 *      dal_rtl9602bvb_epon_mpcpTimeoutVal_set
 * Description:
 *      Set mpcp time out value.
 * Input:
 *      timeVal - mpcp tomeout value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_epon_mpcpTimeoutVal_set(uint32 timeVal)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "timeVal=%d",timeVal);

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((0x80 <=timeVal), RT_ERR_INPUT);

    /* function body */

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_mpcpTimeoutVal_set */


#if 0
/* Function Name:
 *      dal_rtl9602bvb_epon_fecState_get
 * Description:
 *      Get EPON global fec state
 * Input:
 *       None
 * Output:
 *       *pState : glogal FEC state 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_fecState_get(rtk_enable_t *pState)
{
 	//int32   ret; /* MODIFICATION needed */
	uint32  tmpVal;   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);
    

#if 0 /* MODIFICATION needed */
    /* function body */
    tmpVal=0;
    if ((ret = reg_field_read(RTL9602BVB_SDS_REG25r,RTL9602BVB_CFG_FIB2G_TXCFG_NP_P1f,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
#endif
    
    /*bit 1 is PCS FEC mode*/
    if(tmpVal & 0x2)
    {
        *pState=ENABLED;    
    }
    else
    {
        *pState=DISABLED;    
    }

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_fecState_get */


/* Function Name:
 *      dal_rtl9602bvb_epon_fecState_set
 * Description:
 *      Set EPON global fec state
 * Input:
 *       state : global FEC state
 * Output:
 *       None 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_fecState_set(rtk_enable_t state)
{
 	//int32   ret; /* MODIFICATION needed */
	uint32  tmpVal;   

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "state=%d",state);

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_ENABLE_END <= state), RT_ERR_INPUT);

#if 0 /* MODIFICATION needed */
    /* function body */
    tmpVal=0;
    if ((ret = reg_field_read(RTL9602BVB_SDS_REG25r,RTL9602BVB_CFG_FIB2G_TXCFG_NP_P1f,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
#endif

    if(ENABLED == state)
    {
        tmpVal = tmpVal | 0x2;        
    }
    else
    {
        tmpVal = tmpVal & (uint32)(~0x00000002);        
    }
    
#if 0 /* MODIFICATION needed */
    if ((ret = reg_field_write(RTL9602BVB_SDS_REG25r,RTL9602BVB_CFG_FIB2G_TXCFG_NP_P1f,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
#endif

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_fecState_set */
#endif


/* Function Name:
 *      dal_rtl9602bvb_epon_llidEntryNum_get
 * Description:
 *      Get EPON support LLID entry number
 * Input:
 *       None
 * Output:
 *       *pState : glogal FEC state 
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_llidEntryNum_get(uint32 *num)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* parameter check */
    RT_PARAM_CHK((NULL == num), RT_ERR_NULL_POINTER);

    /* function body */
    *num = RTL9602BVB_MAX_LLID_ENTRY;
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_llidEntryNum_get */


/* Function Name:
 *      dal_rtl9602bvb_epon_thresholdReport_set
 * Description:
 *      Set epon threshold report
 * Input:
 *      llidIdx       - llid index
 *      pThresholdRpt - threshole report setting
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT            - Error Input
 * Note:
 *      The the report level 1 for normal report type, max value is 4.
 */
int32
dal_rtl9602bvb_epon_thresholdReport_set(uint8 llidIdx, rtk_epon_report_threshold_t *pThresholdRpt)
{
 	int32  ret;
    int32  level;
    int32 th1;
    int32 th2;
    int32 th3;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "llidIdx=%d",llidIdx);


    /* parameter check */
    RT_PARAM_CHK((RTL9602BVB_MAX_LLID_ENTRY <= llidIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pThresholdRpt), RT_ERR_NULL_POINTER);
    level = pThresholdRpt->levelNum;
    if(level ==0 || level>4)
        return RT_ERR_INPUT;
    
    /* function body */
    if ((ret = reg_array_field_write(RTL9602BVB_EPON_RPT_QSET_NUM1r, REG_ARRAY_INDEX_NONE, llidIdx, RTL9602BVB_CFG_LLID_QSET_NUMf, (uint32 *)&level)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    /*set level*/
    th1 = pThresholdRpt->th1;
    th2 = pThresholdRpt->th2;
    th3 = pThresholdRpt->th3;
    
    if ((ret = reg_array_field_write(RTL9602BVB_EPON_RPT_LVL1r, REG_ARRAY_INDEX_NONE, 0, RTL9602BVB_CFG_LLID_RPT_LVLf, (uint32 *)&th1)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    if ((ret = reg_array_field_write(RTL9602BVB_EPON_RPT_LVL1r, REG_ARRAY_INDEX_NONE, 1, RTL9602BVB_CFG_LLID_RPT_LVLf, (uint32 *)&th2)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    if ((ret = reg_array_field_write(RTL9602BVB_EPON_RPT_LVL1r, REG_ARRAY_INDEX_NONE, 2, RTL9602BVB_CFG_LLID_RPT_LVLf, (uint32 *)&th3)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    } 
    
    if ((ret = reg_array_field_write(RTL9602BVB_EPON_RPT_LVLr, REG_ARRAY_INDEX_NONE, 0, RTL9602BVB_CFG_LLID_RPT_LVLf, (uint32 *)&th1)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    if ((ret = reg_array_field_write(RTL9602BVB_EPON_RPT_LVLr, REG_ARRAY_INDEX_NONE, 1, RTL9602BVB_CFG_LLID_RPT_LVLf, (uint32 *)&th2)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    if ((ret = reg_array_field_write(RTL9602BVB_EPON_RPT_LVLr, REG_ARRAY_INDEX_NONE, 2, RTL9602BVB_CFG_LLID_RPT_LVLf, (uint32 *)&th3)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }     
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_thresholdReport_set */

/* Function Name:
 *      dal_rtl9602bvb_epon_thresholdReport_get
 * Description:
 *      Get epon threshold report setting
 * Input:
 *      llidIdx       - llid index
 *      pThresholdRpt - threshole report setting
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT            - Error Input
 * Note:
 */
int32
dal_rtl9602bvb_epon_thresholdReport_get(uint8 llidIdx, rtk_epon_report_threshold_t *pThresholdRpt)
{
 	int32   ret;
	uint32  tmpVal; 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "llidIdx=%d",llidIdx);

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((2 <=llidIdx), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pThresholdRpt), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_array_field_read(RTL9602BVB_EPON_RPT_QSET_NUM1r, REG_ARRAY_INDEX_NONE, llidIdx, RTL9602BVB_CFG_LLID_QSET_NUMf, (uint32 *)&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    pThresholdRpt->levelNum = tmpVal;

    if ((ret = reg_array_field_read(RTL9602BVB_EPON_RPT_LVL1r, REG_ARRAY_INDEX_NONE, 0, RTL9602BVB_CFG_LLID_RPT_LVLf, (uint32 *)&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pThresholdRpt->th1 = tmpVal;

    if ((ret = reg_array_field_read(RTL9602BVB_EPON_RPT_LVL1r, REG_ARRAY_INDEX_NONE, 1, RTL9602BVB_CFG_LLID_RPT_LVLf, (uint32 *)&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pThresholdRpt->th2 = tmpVal;
    
    if ((ret = reg_array_field_read(RTL9602BVB_EPON_RPT_LVL1r, REG_ARRAY_INDEX_NONE, 2, RTL9602BVB_CFG_LLID_RPT_LVLf, (uint32 *)&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }    
    pThresholdRpt->th3 = tmpVal;
        
    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_thresholdReport_get */


/* Function Name:
 *      dal_rtl9602bvb_epon_dbgInfo_get
 * Description:
 *      Get EPON debug information
 * Input:
 *	  intrType: type of interrupt
 * Output:
 *      pDbgCnt: point of relative debug counter
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 * Note:
 */
int32 dal_rtl9602bvb_epon_dbgInfo_get(rtk_epon_dbgCnt_t *pDbgCnt)
{
	uint32  tmpVal,i; 
 	int32   ret;

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pDbgCnt), RT_ERR_NULL_POINTER);

    /* function body */
    pDbgCnt->losIntCnt       =_eponLosIntCnt;
    pDbgCnt->mpcpIntCnt      =_eponMpcpTimeoutIntCnt;
    pDbgCnt->timeDriftIntCnt =_eponTimeDriftIntCnt;
    pDbgCnt->regFinIntCnt    =_eponRegIntCnt;

    pDbgCnt->regSuccessCnt   =_eponRegSuccessCnt;
    pDbgCnt->regFailCnt      =_eponRegFailIntCnt;
    pDbgCnt->tod1ppsIntCnt   =_epon1ppsIntCnt;
    pDbgCnt->fecIntCnt       =_eponfecIntCnt;

    for(i=0;i<RTL9602BVB_MAX_LLID_ENTRY;i++)
    {
        pDbgCnt->mpcpTimeoutCnt[i]   =_eponMpcpTimeoutLLidCnt[i];
        _eponMpcpTimeoutLLidCnt[i]=0;
    }


    /*reset counter*/
    _eponRegIntCnt=0;
    _eponTimeDriftIntCnt=0;
    _eponMpcpTimeoutIntCnt=0;
    _eponLosIntCnt=0;
    _eponRegSuccessCnt=0;
    _eponRegFailIntCnt=0;
    _epon1ppsIntCnt=0;
    _eponfecIntCnt=0;


    if ((ret = reg_field_read(RTL9602BVB_EPON_GATE_ANA_CNTr,RTL9602BVB_NOT_FORCE_RPTf, (uint32 *)&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    } 
    pDbgCnt->gatenoForceReport=tmpVal;

    if ((ret = reg_field_read(RTL9602BVB_EPON_GATE_ANA_CNTr,RTL9602BVB_HIDDEN_GNT_CNTf, (uint32 *)&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }     
    pDbgCnt->gateHidden       =tmpVal;

    if ((ret = reg_field_read(RTL9602BVB_EPON_GATE_ANA_CNTr,RTL9602BVB_BTB_GNT_CNTf, (uint32 *)&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    } 
    pDbgCnt->gateBackToBack   =tmpVal;

    return RT_ERR_OK;
}   /* end of dal_rtl9602bvb_epon_dbgInfo_get */



/* Function Name:
 *      dal_rtl9602bvb_epon_losState_get
 * Description:
 *      Get laser lose of signal state.
 * Input:
 *      pState LOS state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32
dal_rtl9602bvb_epon_losState_get(rtk_enable_t *pState)
{
	uint32  tmpVal;   
 	int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_EPON), "");

    /* check Init status */
    RT_INIT_CHK(epon_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    /* function body */
    if ((ret = reg_field_read(RTL9602BVB_SDS_EXT_REG30r,RTL9602BVB_SEP_LINKOK_LATCHf,&tmpVal)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_EPON|MOD_DAL), "");
        return ret;
    }
    
    if(1==tmpVal)
        *pState = DISABLED;
    else
        *pState = ENABLED;
    
    return RT_ERR_OK;
}   /* end of dal_apollomp_epon_losState_get */


/*
 * Copyright (C) 2012 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision:
 * $Date: 2013-05-03 17:35:27 +0800 (週五, 03 五月 2013) $
 *
 * Purpose : Definition those public Interrupt APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Interrupt parameter settings
 */


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb.h>
#include <dal/rtl9602bvb/dal_rtl9602bvb_intr.h>
#include <rtk/intr.h>


/*
 * Symbol Definition
 */
#define IS_RTL9602BVB_VALID_INTERRUPT_TYPE(type) \
    (((INTR_TYPE_LINK_CHANGE != type) && \
     (INTR_TYPE_METER_EXCEED != type) && \
     (INTR_TYPE_LEARN_OVER != type) && \
     (INTR_TYPE_SPEED_CHANGE != type) && \
     (INTR_TYPE_SPECIAL_CONGEST != type) && \
     (INTR_TYPE_LOOP_DETECTION != type) && \
     (INTR_TYPE_DEBUG != type) && \
     (INTR_TYPE_ACL_ACTION != type) && \
     (INTR_TYPE_GPHY != type) && \
     (INTR_TYPE_SERDES != type) && \
     (INTR_TYPE_GPON != type) && \
     (INTR_TYPE_EPON != type) && \
     (INTR_TYPE_PTP != type) && \
     (INTR_TYPE_DYING_GASP != type) && \
     (INTR_TYPE_THERMAL != type) && \
     (INTR_TYPE_SMARTCARD != type) && \
     (INTR_TYPE_SWITCH_ERROR != type) && \
     (INTR_TYPE_ALL != type)) ? 0 : 1)

/*
 * Data Declaration
 */
static uint32	  intr_init = {INIT_NOT_COMPLETED};

/* The order of ims/imr here should be the same as in intr.h
 * There should be no array indexing to dummy entries
 * which should be blocked by IS_RTL9602BVB_VALID_INTERRUPT_TYPE
 */
static uint32 	intr_reg_ims_field[] = {
    RTL9602BVB_IMS_LINK_CHGf,
    RTL9602BVB_IMS_METER_EXCEEDf,
    RTL9602BVB_IMS_L2_LRN_OVERf,
    RTL9602BVB_IMS_SPE_CHGf,
    RTL9602BVB_IMS_SPE_CONGESTf,
    RTL9602BVB_IMS_LOOPf,
    RTL9602BVB_IMS_LOOPf, /* Dummy entry */
    RTL9602BVB_IMS_ACLf,
    RTL9602BVB_IMS_GPHYf,
    RTL9602BVB_IMS_SERDESf,
    RTL9602BVB_IMS_GPONf,
    RTL9602BVB_IMS_EPONf,
    RTL9602BVB_IMS_PTP_1_SECf,
    RTL9602BVB_IMS_DYING_GASPf,
    RTL9602BVB_IMS_THERMAL_ALARMf,
    RTL9602BVB_IMS_THERMAL_ALARMf, /* Dummy entry */
    RTL9602BVB_IMS_THERMAL_ALARMf, /* Dummy entry */
    RTL9602BVB_IMS_THERMAL_ALARMf, /* Dummy entry */
    RTL9602BVB_IMS_THERMAL_ALARMf, /* Dummy entry */
    RTL9602BVB_IMS_DBGOf,
    RTL9602BVB_IMS_SCf,
    RTL9602BVB_IMS_CRASHf,
};

static uint32 	intr_reg_ims_bitPos[] = {
    0,  /* RTL9602BVB_IMS_LINK_CHGf */
    1,  /* RTL9602BVB_IMS_METER_EXCEEDf */
    2,  /* RTL9602BVB_IMS_L2_LRN_OVERf */
    3,  /* RTL9602BVB_IMS_SPE_CHGf */
    4,  /* RTL9602BVB_IMS_SPE_CONGESTf */
    5,  /* RTL9602BVB_IMS_LOOPf */
    5,  /* RTL9602BVB_IMS_LOOPf */ /* Dummy entry */
    7,  /* RTL9602BVB_IMS_ACLf */
    8,  /* RTL9602BVB_IMS_GPHYf */
    9,  /* RTL9602BVB_IMS_SERDESf */
    10, /* RTL9602BVB_IMS_GPONf */
    11, /* RTL9602BVB_IMS_EPONf */
    12, /* RTL9602BVB_IMR_PTP_1_SECf */
    13, /* RTL9602BVB_IMS_DYING_GASPf */
    14, /* RTL9602BVB_IMS_THERMAL_ALARMf */
    14, /* RTL9602BVB_IMS_THERMAL_ALARMf */ /* Dummy entry */
    14, /* RTL9602BVB_IMS_THERMAL_ALARMf */ /* Dummy entry */
    14, /* RTL9602BVB_IMS_THERMAL_ALARMf */ /* Dummy entry */
    14, /* RTL9602BVB_IMS_THERMAL_ALARMf */ /* Dummy entry */
    6,  /* RTL9602BVB_IMS_DBGOf */
    15, /* RTL9602BVB_IMS_SCf */
    16, /* RTL9602BVB_IMS_CRASHf */
};

static uint32 	intr_reg_imr_field[] = {
    RTL9602BVB_IMR_LINK_CHGf,
    RTL9602BVB_IMR_METER_EXCEEDf,
    RTL9602BVB_IMR_L2_LRN_OVERf,
    RTL9602BVB_IMR_SPE_CHGf,
    RTL9602BVB_IMR_SPE_CONGESTf,
    RTL9602BVB_IMR_LOOPf,
    RTL9602BVB_IMR_LOOPf, /* Dummy entry */
    RTL9602BVB_IMR_ACLf,
    RTL9602BVB_IMR_GPHYf,
    RTL9602BVB_IMR_SERDESf,
    RTL9602BVB_IMR_GPONf,
    RTL9602BVB_IMR_EPONf,
    RTL9602BVB_IMR_PTP_1_SECf,
    RTL9602BVB_IMR_DYING_GASPf,
    RTL9602BVB_IMR_THERMAL_ALARMf,
    RTL9602BVB_IMR_THERMAL_ALARMf, /* Dummy entry */
    RTL9602BVB_IMR_THERMAL_ALARMf, /* Dummy entry */
    RTL9602BVB_IMR_THERMAL_ALARMf, /* Dummy entry */
    RTL9602BVB_IMR_THERMAL_ALARMf, /* Dummy entry */
    RTL9602BVB_IMR_DBGOf,
    RTL9602BVB_IMR_SCf,
    RTL9602BVB_IMR_CRASHf,
};

/*
 * Function Declaration
 */

/* Function Name:
 *      dal_rtl9602bvb_intr_init
 * Description:
 *      Initialize interrupt module.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize interrupt module before calling any interrupt APIs.
 */
int32
dal_rtl9602bvb_intr_init(void)
{
	int32   ret;

	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);
	intr_init = INIT_COMPLETED;

	/*reset interrupt mask register*/
	if((ret=dal_rtl9602bvb_intr_imr_set(INTR_TYPE_ALL,DISABLED)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		intr_init = INIT_NOT_COMPLETED;
		return ret;
	}

	/*clear all of interrupt status*/
	if((ret=dal_rtl9602bvb_intr_ims_clear(INTR_TYPE_ALL)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		intr_init = INIT_NOT_COMPLETED;
		return ret;
	}

	/*clear port speed shange status*/
	if((ret=dal_rtl9602bvb_intr_speedChangeStatus_clear()) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		intr_init = INIT_NOT_COMPLETED;
		return ret;
	}

	/*clear port linkup status*/
	if((ret=dal_rtl9602bvb_intr_linkupStatus_clear()) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		intr_init = INIT_NOT_COMPLETED;
		return ret;
	}

	/*clear port linkdown status*/
	if((ret=dal_rtl9602bvb_intr_linkdownStatus_clear()) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		intr_init = INIT_NOT_COMPLETED;
		return ret;
	}

	/*clear gphy status*/
	if((ret=dal_rtl9602bvb_intr_gphyStatus_clear()) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		intr_init = INIT_NOT_COMPLETED;
		return ret;
	}
	return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_init */


/* Function Name:
 *      dal_rtl9602bvb_intr_polarity_set
 * Description:
 *      Set interrupt polarity mode
 * Input:
 *      mode - Interrupt polarity mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK		- OK
 *      RT_ERR_FAILED	- Failed
 *      RT_ERR_INPUT	- Invalid input parameters.
 * Note:
 *      The API can set Interrupt polarity mode.
 *      The modes that can be set as following:
 *      - INTR_POLAR_HIGH
 *      - INTR_POLAR_LOW
 */
int32
dal_rtl9602bvb_intr_polarity_set(rtk_intr_polarity_t polar)
{
	int32   ret;
	uint32 ipolar;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

	/* check Init status */
	RT_INIT_CHK(intr_init);

	/* parameter check */
	RT_PARAM_CHK((INTR_POLAR_END <= polar), RT_ERR_OUT_OF_RANGE);

	ipolar = polar;

	if((ret = reg_field_write(RTL9602BVB_INTR_CTRLr,RTL9602BVB_INTR_POLARITYf, &ipolar) ) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		return ret;
	}

	return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_polarity_set */


/* Function Name:
 *      dal_rtl9602bvb_intr_polarity_mode_get
 * Description:
 *      Get Interrupt polarity mode
 * Input:
 *      None
 * Output:
 *      pMode - Interrupt polarity mode
 * Return:
 *      RT_ERR_OK      	- OK
 *      RT_ERR_FAILED	- Failed
 *      RT_ERR_INPUT 	- Invalid input parameters.
 * Note:
 *      The API can get Interrupt polarity mode.
 *	  The modes that can be got as following:
 *      - INTR_POLAR_HIGH
 *      - INTR_POLAR_LOW
 */
int32
dal_rtl9602bvb_intr_polarity_get(rtk_intr_polarity_t *pPolar)
{
	int32   ret;
	uint32 ipolar;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

	/* check Init status */
	RT_INIT_CHK(intr_init);

	/* parameter check */
	RT_PARAM_CHK((NULL == pPolar), RT_ERR_NULL_POINTER);

	if ((ret = reg_field_read(RTL9602BVB_INTR_CTRLr, RTL9602BVB_INTR_POLARITYf, &ipolar)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
		return ret;
	}

	*pPolar = ipolar;

	return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_polarity_get */


/* Function Name:
 *      dal_rtl9602bvb_intr_imr_set
 * Description:
 *      Set interrupt mask.
 * Input:
 *      intr          - interrupt type
 *      enable      - interrupt state
 *
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *	   None.
 */
int32
dal_rtl9602bvb_intr_imr_set(rtk_intr_type_t intr, rtk_enable_t enable)
{
	int32   ret,i;
	uint32	regValue;

	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);
	RT_INIT_CHK(intr_init);

	/* parameter check */
	RT_PARAM_CHK((INTR_TYPE_END <= intr), RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK(!IS_RTL9602BVB_VALID_INTERRUPT_TYPE(intr), RT_ERR_INPUT);
	RT_PARAM_CHK((RTK_ENABLE_END <= enable), RT_ERR_OUT_OF_RANGE);


	if(INTR_TYPE_ALL == intr)
    {
        regValue = 0;
        for(i = 0; i < INTR_TYPE_ALL; i++)
        {
            if(!IS_RTL9602BVB_VALID_INTERRUPT_TYPE(i))
                continue;

            regValue |= ((enable&0x1)<<i);
        }
        if ((ret = reg_write(RTL9602BVB_INTR_IMRr, &regValue)) != RT_ERR_OK)
		{
            RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
			return ret;
		}
    }
    else
    {
        regValue = enable;
        if ((ret = reg_field_write(RTL9602BVB_INTR_IMRr, intr_reg_imr_field[intr], &regValue)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
			return ret;
		}
    }

	return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_imr_set */


/* Function Name:
 *      dal_rtl9602bvb_intr_imr_get
 * Description:
 *      Get interrupt mask.
 * Input:
 *      intr            - interrupt type
 *      pEnable      - pointer of return status
 *
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_INPUT
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
 extern int32
dal_rtl9602bvb_intr_imr_get(rtk_intr_type_t intr, rtk_enable_t *pEnable){

	uint32 ret;
	uint32 regValue;

	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

	/* check Init status */
	RT_INIT_CHK(intr_init);

	/* parameter check */
	RT_PARAM_CHK((INTR_TYPE_END <= intr), RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK(!IS_RTL9602BVB_VALID_INTERRUPT_TYPE(intr), RT_ERR_INPUT);
	RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

	if(INTR_TYPE_ALL == intr)
	{
		 if ((ret = reg_read(RTL9602BVB_INTR_IMRr, &regValue)) != RT_ERR_OK)
		 {
			 RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
			 return ret;
		 }
	}
	else
	{
		if ((ret = reg_field_read(RTL9602BVB_INTR_IMRr, intr_reg_imr_field[intr], &regValue)) != RT_ERR_OK)
		{
			RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
			return ret;
		}
	}

	*pEnable = regValue;

	return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl9602bvb_intr_ims_get
 * Description:
 *      Get interrupt status.
 * Input:
 *      intr            - interrupt type
 *      pState        - pointer of return status
 *
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_INPUT
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_ims_get(rtk_intr_type_t intr, rtk_enable_t *pState)
{
	int32   ret;
	uint32	regValue;
	RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

	/* check Init status */
	RT_INIT_CHK(intr_init);

	/* parameter check */
	RT_PARAM_CHK((INTR_TYPE_END <= intr), RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK(!IS_RTL9602BVB_VALID_INTERRUPT_TYPE(intr), RT_ERR_INPUT);
	RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

	if(INTR_TYPE_ALL == intr)
	{
		if ((ret = reg_read(RTL9602BVB_INTR_IMSr, &regValue)) != RT_ERR_OK)
		{
		    RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
			return ret;
		}
	}
	else
	{
		if ((ret = reg_field_read(RTL9602BVB_INTR_IMSr, intr_reg_ims_field[intr], &regValue)) != RT_ERR_OK)
	    {
			RT_ERR(ret, (MOD_DAL|MOD_INTR), "intr=%d",intr);
			return ret;
		}
	}

	*pState = regValue;

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_ims_get */


/* Function Name:
 *      dal_rtl9602bvb_intr_ims_clear
 * Description:
 *      Clear interrupt status.
 * Input:
 *      intr            - interrupt type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_ims_clear(rtk_intr_type_t intr)
{
    int32   ret,i;
	uint32	regValue;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(intr_init);

	/* parameter check */
	RT_PARAM_CHK((INTR_TYPE_END <= intr), RT_ERR_OUT_OF_RANGE);
	RT_PARAM_CHK(!IS_RTL9602BVB_VALID_INTERRUPT_TYPE(intr), RT_ERR_INPUT);

    if(INTR_TYPE_ALL == intr)
    {
        regValue = 0;
        for(i = 0; i < INTR_TYPE_ALL; i++)
        {
            if(!IS_RTL9602BVB_VALID_INTERRUPT_TYPE(i))
                continue;

            regValue |= (1<<intr_reg_ims_bitPos[intr]);
        }
    }
    else
    {
        regValue = (1<<intr_reg_ims_bitPos[intr]);
    }

    if ((ret = reg_write(RTL9602BVB_INTR_IMSr, &regValue)) != RT_ERR_OK)
    {
    	RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
		return ret;
    }

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_ims_clear */


/* Function Name:
 *      dal_rtl9602bvb_intr_speedChangeStatus_get
 * Description:
 *      Get interrupt status of speed change.
 * Input:
 *      pPortMask          - pointer of return status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_speedChangeStatus_get(rtk_portmask_t *pPortMask)
{
    int32   ret;
	uint32	regValue;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(intr_init);

    /* parameter check */
    RT_PARAM_CHK((NULL==pPortMask), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(RTL9602BVB_INTR_STATr, RTL9602BVB_INTR_STAT_PORT_CHANGEf, &regValue)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
		return ret;
    }

	RTK_PORTMASK_FROM_UINT_PORTMASK(pPortMask,&regValue);

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_speedChangeStatus_get */


/* Function Name:
 *      dal_rtl9602bvb_intr_speedChangeStatus_clear
 * Description:
 *      Clear interrupt status of port speed change.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_speedChangeStatus_clear(void)
{
    int32   ret;
	uint32	regValue;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(intr_init);

 	regValue = INTR_PORT_MASK;

    if ((ret = reg_field_write(RTL9602BVB_INTR_STATr,RTL9602BVB_INTR_STAT_PORT_CHANGEf,&regValue)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
		return ret;
    }


    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_speedChangeStatus_clear */


/* Function Name:
 *      dal_rtl9602bvb_intr_linkupStatus_get
 * Description:
 *      Get interrupt status of linkup.
 * Input:
 *      pPortMask          - pointer of return status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_linkupStatus_get(rtk_portmask_t *pPortMask)
{
    int32   ret;
	uint32	regValue;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(intr_init);


    /* parameter check */
    RT_PARAM_CHK((NULL == pPortMask), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(RTL9602BVB_INTR_STATr, RTL9602BVB_INTR_STAT_PORT_LINKUPf, &regValue)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
		return ret;
    }
	RTK_PORTMASK_FROM_UINT_PORTMASK(pPortMask,&regValue);

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_linkupStatus_get */


/* Function Name:
 *      dal_rtl9602bvb_intr_linkupStatus_clear
 * Description:
 *      Clear interrupt status of linkup.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_linkupStatus_clear(void)
{
    int32   ret;
	uint32	regValue;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(intr_init);

	regValue = INTR_PORT_MASK;

	if((ret=reg_field_write(RTL9602BVB_INTR_STATr,RTL9602BVB_INTR_STAT_PORT_LINKUPf,&regValue)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		return ret;
	}


    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_linkupStatus_clear */


/* Function Name:
 *      dal_rtl9602bvb_intr_linkdownStatus_get
 * Description:
 *      Get interrupt status of linkdown.
 * Input:
 *      pPortMask          - pointer of return status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_linkdownStatus_get(rtk_portmask_t *pPortMask)
{
    int32   ret;
	uint32	regValue;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(intr_init);


    /* parameter check */
    RT_PARAM_CHK((NULL == pPortMask), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(RTL9602BVB_INTR_STATr, RTL9602BVB_INTR_STAT_PORT_LINKDOWNf, &regValue)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
		return ret;
    }
	RTK_PORTMASK_FROM_UINT_PORTMASK(pPortMask,&regValue);

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_linkdownStatus_get */


/* Function Name:
 *      dal_rtl9602bvb_intr_linkdownStatus_clear
 * Description:
 *      Clear interrupt status of linkdown.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_linkdownStatus_clear(void)
{
    int32   ret;
	uint32	regValue;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(intr_init);

	regValue = INTR_PORT_MASK;

	if((ret=reg_field_write(RTL9602BVB_INTR_STATr,RTL9602BVB_INTR_STAT_PORT_LINKDOWNf,&regValue)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		return ret;
	}

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_linkdownStatus_clear */


/* Function Name:
 *      dal_rtl9602bvb_intr_gphyStatus_get
 * Description:
 *      Get interrupt status of GPHY.
 * Input:
 *      pPortMask          - pointer of return status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_gphyStatus_get(rtk_portmask_t *pPortMask)
{
    int32   ret;
	uint32 	regValue;
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(intr_init);


    /* parameter check */
    RT_PARAM_CHK((NULL == pPortMask), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(RTL9602BVB_INTR_STATr, RTL9602BVB_INTR_STAT_GPHYf, &regValue)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
		return ret;
    }
	RTK_PORTMASK_FROM_UINT_PORTMASK(pPortMask,&regValue);

    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_gphyStatus_get */


/* Function Name:
 *      dal_rtl9602bvb_intr_sts_gphy_clear
 * Description:
 *      Clear interrupt status of GPHY.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_gphyStatus_clear(void)
{
    int32   ret;
	uint32	regValue;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(intr_init);

	regValue = INTR_PORT_MASK;

	if((ret=reg_field_write(RTL9602BVB_INTR_STATr,RTL9602BVB_INTR_STAT_GPHYf,&regValue)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		return ret;
	}


    return RT_ERR_OK;
} /* end of dal_rtl9602bvb_intr_gphyStatus_clear */


/* Function Name:
 *      dal_rtl9602bvb_intr_imr_restore
 * Description:
 *     set imr mask from input value
 * Input:
 *      imrValue: imr value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None.
 */
int32
dal_rtl9602bvb_intr_imr_restore(uint32 imrValue)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_INTR),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(intr_init);

	/* check input value*/
	RT_PARAM_CHK((INTR_IMR_MASK < imrValue), RT_ERR_OUT_OF_RANGE);

	if((ret=reg_write(RTL9602BVB_INTR_IMRr,&imrValue)) != RT_ERR_OK)
	{
		RT_ERR(ret, (MOD_DAL|MOD_INTR),"");
		return ret;
	}


    return RT_ERR_OK;

}


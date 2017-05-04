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
 * Purpose : Definition those public Trap APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) interrupt.
 *
 */

#include <dal/apollo/raw/apollo_raw_intr.h>
#include <hal/chipdef/apollo/apollo_reg_definition.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
uint32 intr_imr_reg[] = {
    IMR_LINK_CHGf,
    IMR_METER_EXCEEDf,
    IMR_L2_LRN_OVERf,
    IMR_SPE_CHGf,
    IMR_SPE_CONGESTf,
    IMR_LOOPf,
    IMR_RTCTf,
    IMR_ACLf,
    IMR_GPHYf,
    IMR_SERDESf,
    IMR_GPONf,
    IMR_EPONf,
    IMR_PTPf,
    IMR_DYING_GASPf,
    IMR_THERMAL_ALARMf,
    IMR_ADC_ALARMf
};

uint32 intr_ims_reg[] = {
    IMS_LINK_CHGf,
    IMS_METER_EXCEEDf,
    IMS_L2_LRN_OVERf,
    IMS_SPE_CHGf,
    IMS_SPE_CONGESTf,
    IMS_LOOPf,
    IMS_RTCTf,
    IMS_ACLf,
    IMS_GPHYf,
    IMS_SERDESf,
    IMS_GPONf,
    IMS_EPONf,
    IMS_PTPf,
    IMS_DYING_GASPf,
    IMS_THERMAL_ALARMf,
    IMS_ADC_ALARMf
};

/* Function Name:
 *      apollo_raw_intr_imr_set
 * Description:
 *      Set interrupt polarity.
 * Input:
 *      polar         - interrupt polarity
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
int32 apollo_raw_intr_polarity_set(rtk_intr_polarity_t polar)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((polar>=INTR_POLAR_END), RT_ERR_OUT_OF_RANGE);

    data = polar;
    if ((ret = reg_field_write(INTR_CTRLr, INTR_POLARITYf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }

    return ret;
} /* end of apollo_raw_intr_polarity_set */

/* Function Name:
 *      apollo_raw_intr_polarity_get
 * Description:
 *      Get interrupt polarity.
 * Input:
 *      pPolar        - the pointer of return interrupt polarity
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32 apollo_raw_intr_polarity_get(rtk_intr_polarity_t *pPolar)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((NULL==pPolar), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(INTR_CTRLr, INTR_POLARITYf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }
    *pPolar = data;

    return ret;
} /* end of apollo_raw_intr_polarity_get */

/* Function Name:
 *      apollo_raw_intr_imr_set
 * Description:
 *      Set interrupt mask.
 * Input:
 *      intr          - interrupt type
 *      enable        - interrupt mask state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
int32 apollo_raw_intr_imr_set(rtk_intr_type_t intr, rtk_enable_t enable)
{
    int32 ret, i;
    uint32 data=0;

    /* parameter check */
    RT_PARAM_CHK((intr>=INTR_TYPE_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((enable>=RTK_ENABLE_END), RT_ERR_OUT_OF_RANGE);

    if(INTR_TYPE_ALL == intr)
    {
        data = 0;
        for(i = 0; i < INTR_TYPE_ALL; i++)
        {
            data |= ((enable&0x1)<<i);
        }
        if ((ret = reg_write(INTR_IMRr, &data)) != RT_ERR_OK)
            RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }
    else
    {
        data = enable;
        if ((ret = reg_field_write(INTR_IMRr, intr_imr_reg[intr], &data)) != RT_ERR_OK)
            RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }

    return ret;
} /* end of apollo_raw_intr_imr_set */

/* Function Name:
 *      apollo_raw_intr_imr_get
 * Description:
 *      Get interrupt mask.
 * Input:
 *      intr            - interrupt type
 *      pMask           - pointer of return mask state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
int32 apollo_raw_intr_imr_get(rtk_intr_type_t intr, uint32 *pMask)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((intr>=INTR_TYPE_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL==pMask), RT_ERR_NULL_POINTER);

    if(INTR_TYPE_ALL == intr)
    {
        if ((ret = reg_read(INTR_IMRr, &data)) != RT_ERR_OK)
            RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }
    else
    {
        if ((ret = reg_field_read(INTR_IMRr, intr_imr_reg[intr], &data)) != RT_ERR_OK)
            RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }

    *pMask = data;

    return ret;
} /* end of apollo_raw_intr_imr_get */

/* Function Name:
 *      apollo_raw_intr_ims_get
 * Description:
 *      Get interrupt status.
 * Input:
 *      intr            - interrupt type
 *      pState          - pointer of return status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
int32 apollo_raw_intr_ims_get(rtk_intr_type_t intr, uint32 *pState)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((intr>=INTR_TYPE_END), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL==pState), RT_ERR_NULL_POINTER);

    if(INTR_TYPE_ALL == intr)
    {
        if ((ret = reg_read(INTR_IMSr, &data)) != RT_ERR_OK)
            RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }
    else
    {
        if ((ret = reg_field_read(INTR_IMSr, intr_ims_reg[intr], &data)) != RT_ERR_OK)
            RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }

    *pState = data;

    return ret;
} /* end of apollo_raw_intr_ims_get */

/* Function Name:
 *      apollo_raw_intr_ims_clear
 * Description:
 *      Clear interrupt status.
 * Input:
 *      intr            - interrupt type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      None.
 */
int32 apollo_raw_intr_ims_clear(rtk_intr_type_t intr)
{
    int32 ret, i;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((intr>=INTR_TYPE_END), RT_ERR_OUT_OF_RANGE);

    if(INTR_TYPE_ALL == intr)
    {
        data = 0;
        for(i = 0; i < INTR_TYPE_ALL; i++)
        {
            data |= (1<<i);
        }
    }
    else
    {
        data = (1<<intr);
    }

    if ((ret = reg_write(INTR_IMSr, &data)) != RT_ERR_OK)
            RT_ERR(ret, (MOD_DAL|MOD_INTR), "");

    return ret;
} /* end of apollo_raw_intr_ims_clear */


/* Function Name:
 *      apollo_raw_intr_sts_speed_change_get
 * Description:
 *      Get interrupt status of speed change.
 * Input:
 *      pState          - pointer of return status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32 apollo_raw_intr_sts_speed_change_get(uint32 *pState)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((NULL==pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(INTR_STATr, INTR_STAT_PORT_CHANGEf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }
    *pState = data;

    return ret;
} /* end of apollo_raw_intr_sts_speed_change_get */

/* Function Name:
 *      apollo_raw_intr_sts_linkup_get
 * Description:
 *      Get interrupt status of linkup.
 * Input:
 *      pState          - pointer of return status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32 apollo_raw_intr_sts_linkup_get(uint32 *pState)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((NULL==pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(INTR_STATr, INTR_STAT_PORT_LINKUPf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }
    *pState = data;

    return ret;
} /* end of apollo_raw_intr_sts_linkup_get */

/* Function Name:
 *      apollo_raw_intr_sts_linkdown_get
 * Description:
 *      Get interrupt status of linkdown.
 * Input:
 *      pState          - pointer of return status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32 apollo_raw_intr_sts_linkdown_get(uint32 *pState)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((NULL==pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(INTR_STATr, INTR_STAT_PORT_LINKDOWNf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }
    *pState = data;

    return ret;
} /* end of apollo_raw_intr_sts_linkdown_get */

/* Function Name:
 *      apollo_raw_intr_sts_gphy_get
 * Description:
 *      Get interrupt status of GPHY.
 * Input:
 *      pState          - pointer of return status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None.
 */
int32 apollo_raw_intr_sts_gphy_get(uint32 *pState)
{
    int32 ret;
    uint32 data;

    /* parameter check */
    RT_PARAM_CHK((NULL==pState), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(INTR_STATr, INTR_STAT_GPHYf, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }
    *pState = data;

    return ret;
} /* end of apollo_raw_intr_sts_gphy_get */

/* Function Name:
 *      apollo_raw_intr_sts_speed_change_clear
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
int32 apollo_raw_intr_sts_speed_change_clear(void)
{
    int32 ret;
    uint32 data;

    data = APOLLO_INTR_STAT_INTR_STAT_PORT_CHANGE_MASK;

    if ((ret = reg_write(INTR_STATr, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }

    return ret;
} /* end of apollo_raw_intr_sts_speed_change_clear */

/* Function Name:
 *      apollo_raw_intr_sts_linkup_clear
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
int32 apollo_raw_intr_sts_linkup_clear(void)
{
    int32 ret;
    uint32 data;

    data = APOLLO_INTR_STAT_INTR_STAT_PORT_LINKUP_MASK;

    if ((ret = reg_write(INTR_STATr, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }

    return ret;
} /* end of apollo_raw_intr_sts_linkup_clear */

/* Function Name:
 *      apollo_raw_intr_sts_linkdown_clear
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
int32 apollo_raw_intr_sts_linkdown_clear(void)
{
    int32 ret;
    uint32 data;

    data = APOLLO_INTR_STAT_INTR_STAT_PORT_LINKDOWN_MASK;

    if ((ret = reg_write(INTR_STATr, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }

    return ret;
} /* end of apollo_raw_intr_sts_linkdown_clear */

/* Function Name:
 *      apollo_raw_intr_sts_gphy_clear
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
int32 apollo_raw_intr_sts_gphy_clear(void)
{
    int32 ret;
    uint32 data;

    data = APOLLO_INTR_STAT_INTR_STAT_GPHY_MASK;

    if ((ret = reg_write(INTR_STATr, &data)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_INTR), "");
    }

    return ret;
} /* end of apollo_raw_intr_sts_gphy_clear */




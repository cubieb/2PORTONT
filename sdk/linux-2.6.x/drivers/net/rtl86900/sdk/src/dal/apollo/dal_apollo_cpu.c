/*-----------------<<Programmer Note>>---------------------------------
 Please check this note before coding-- Programmer "Must" remove this Note

	1. declear [your_module]_init
		ex:
          static uint32    l2_init = {INIT_NOT_COMPLETED};
	2. replace all xxx_init to [your_module]_init
	3. replace all MOD_XXX to MOD_[your_module]
	4. chage all RTK_RANGE_DEFINED_XXX to correct defination
	5. implement function body
	6. rename dal.c to dal_[project_name]_[your_module].c
	7. copy this file to sdk/src/dal/[project_name]/

-------------------<<End of Note>>-------------------------------*/

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
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Definition of Classifyication API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) CPU tag functions set/get
 */



/*
 * Include Files
 */
#include <common/rt_type.h>
#include <dal/apollo/dal_apollo.h>
#include <rtk/cpu.h>
#include <dal/apollo/raw/apollo_raw_cpu.h>
#include <dal/apollo/dal_apollo_cpu.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
static uint32    cpu_init = INIT_NOT_COMPLETED;

/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */
/* Function Name:
 *      dal_apollo_cpu_init
 * Description:
 *      Initialize cpu tag module.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize classification module before calling any classification APIs.
 */
int32
dal_apollo_cpu_init(void)
{
    rtk_portmask_t port_mask;
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CPU),"%s",__FUNCTION__);

    cpu_init = INIT_COMPLETED;

    if ((ret = dal_apollo_cpu_trapInsertTag_set(ENABLED)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_CPU|MOD_DAL), "");
        return ret;
    }

    if ((ret = dal_apollo_cpu_tagFormat_set(CPU_TAG_FMT_APOLLO)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_CPU|MOD_DAL), "");
        return ret;
    }

    osal_memset(&port_mask, 0, sizeof(rtk_portmask_t));
    RTK_PORTMASK_PORT_SET(port_mask, HAL_GET_CPU_PORT());
    if ((ret = dal_apollo_cpu_awarePortMask_set(port_mask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_CPU|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_cpu_init */

/* Function Name:
 *      dal_apollo_cpu_awarePortMask_set
 * Description:
 *      Set CPU awared port mask.
 * Input:
 *      port_mask   - CPU awared port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT
 * Note:
 *      Must initialize cpu tag module before calling any cpu tag APIs.
 */
int32
dal_apollo_cpu_awarePortMask_set(rtk_portmask_t port_mask)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CPU),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(cpu_init);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORTMASK_VALID(port_mask), RT_ERR_INPUT);

    if((ret = apollo_raw_cpu_aware_port_mask_set(port_mask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_CPU|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_cpu_awarePortMask_set */

/* Function Name:
 *      dal_apollo_cpu_awarePortMask_get
 * Description:
 *      Get CPU awared port mask.
 * Input:
 *      pPort_mask   - the pointer of CPU awared port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      Must initialize cpu tag module before calling any cpu tag APIs.
 */
int32
dal_apollo_cpu_awarePortMask_get(rtk_portmask_t *pPort_mask)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CPU),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(cpu_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPort_mask), RT_ERR_NULL_POINTER);

    if((ret = apollo_raw_cpu_aware_port_mask_get(pPort_mask)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_CPU|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_cpu_awarePortMask_get */

/* Function Name:
 *      dal_apollo_cpu_tagFormat_set
 * Description:
 *      Set CPU tag format.
 * Input:
 *      mode    - CPU tag format mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      Must initialize cpu tag module before calling any cpu tag APIs.
 */
int32
dal_apollo_cpu_tagFormat_set(rtk_cpu_tag_fmt_t mode)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CPU),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(cpu_init);

    /* parameter check */
    RT_PARAM_CHK((CPU_TAG_FMT_END <= mode), RT_ERR_INPUT);

    if((ret = apollo_raw_cpu_tag_format_set(mode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_CPU|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_cpu_tagFormat_set */

/* Function Name:
 *      dal_apollo_cpu_tagFormat_get
 * Description:
 *      Get CPU tag format.
 * Input:
 *      pMode    - the pointer of CPU tag format mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      Must initialize cpu tag module before calling any cpu tag APIs.
 */
int32
dal_apollo_cpu_tagFormat_get(rtk_cpu_tag_fmt_t *pMode)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CPU),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(cpu_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);

    if((ret = apollo_raw_cpu_tag_format_get(pMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_CPU|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_cpu_tagFormat_get */

/* Function Name:
 *      dal_apollo_cpu_trapInsertTag_set
 * Description:
 *      Set trap CPU insert tag state.
 * Input:
 *      state    - insert CPU tag state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_OUT_OF_RANGE
 * Note:
 *      Must initialize cpu tag module before calling any cpu tag APIs.
 */
int32
dal_apollo_cpu_trapInsertTag_set(rtk_enable_t state)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CPU),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(cpu_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_ENABLE_END <= state), RT_ERR_INPUT);

    if((ret = apollo_raw_cpu_trap_insert_tag_set(state)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_CPU|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_cpu_trapInsertTag_set */

/* Function Name:
 *      dal_apollo_cpu_trapInsertTag_get
 * Description:
 *      Get trap CPU insert tag state.
 * Input:
 *      pState    - the pointer of insert CPU tag state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER
 * Note:
 *      Must initialize cpu tag module before calling any cpu tag APIs.
 */
int32
dal_apollo_cpu_trapInsertTag_get(rtk_enable_t *pState)
{
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CPU),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(cpu_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pState), RT_ERR_NULL_POINTER);

    if((ret = apollo_raw_cpu_trap_insert_tag_get(pState)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_CPU|MOD_DAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_cpu_trapInsertTag_get */




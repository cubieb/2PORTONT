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
 * Purpose : Declaration of PTP
 *
 * Feature : Fake PTP init function for init process
 *
 */


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <dal/apollo/dal_apollo.h>
#include <rtk/time.h>
#include <dal/apollo/dal_apollo_time.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
static uint32    apollo_time_init = {INIT_NOT_COMPLETED}; 


/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Module Name : time */

/* Function Name:
 *      dal_apollomp_time_init
 * Description:
 *      Fake initialize time module.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Fake the init function to allow init process go through
 */
int32
dal_apollo_time_init(void)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PTP),"%s",__FUNCTION__);

    apollo_time_init = INIT_COMPLETED;

    return RT_ERR_OK;
} /* end of dal_apollomp_time_init */



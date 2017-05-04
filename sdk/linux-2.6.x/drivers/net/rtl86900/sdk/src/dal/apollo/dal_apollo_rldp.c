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
 * Purpose : Declaration of RLDP
 *
 * Feature : Fake RLDP init function for init process
 *
 */


/*
 * Include Files
 */
#include <common/rt_type.h>
#include <dal/apollo/dal_apollo.h>
#include <rtk/rldp.h>
#include <dal/apollo/dal_apollo_rldp.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
static uint32    apollo_rldp_init = {INIT_NOT_COMPLETED}; 


/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Module Name : RLDP */

/* Function Name:
 *      dal_apollomp_rldp_init
 * Description:
 *      Fake initialize rldp module.
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
dal_apollo_rldp_init(void)
{
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_RLDP),"%s",__FUNCTION__);

    apollo_rldp_init = INIT_COMPLETED;

    return RT_ERR_OK;
} /* end of dal_apollomp_rldp_init */



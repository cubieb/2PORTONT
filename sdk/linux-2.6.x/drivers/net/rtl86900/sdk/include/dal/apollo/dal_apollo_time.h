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


#ifndef __DAL_APOLLO_TIME_H__
#define __DAL_APOLLO_TIME_H__


/*
 * Include Files
 */
#include <common/rt_type.h>


/*
 * Data Declaration
 */


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
extern int32
dal_apollo_time_init(void);

#endif /* __DAL_APOLLO_TIME_H__ */


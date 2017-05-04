/*
 * Copyright (C) 2014 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : Definition of OMCI Customize define
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI Customize define
 */

#ifndef _OMCI_CUSTOMIZE_H_
#define _OMCI_CUSTOMIZE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	unsigned int        bridgeDP;            /*For bridge data path related feature*/
	unsigned int        routeDP;             /*For route data path related feature*/
	unsigned int        multicast;           /*For multicast related feature*/
	unsigned int        me;                  /*For proprietary ME which NOT related to data path and multicast feature*/
} omci_customer_feature_flag_t;


#ifdef __cplusplus
}
#endif

#endif

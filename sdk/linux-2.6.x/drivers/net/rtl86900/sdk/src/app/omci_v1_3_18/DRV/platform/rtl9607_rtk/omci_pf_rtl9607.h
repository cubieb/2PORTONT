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
 * $Date: 2013-06-24 04:35:27 -0500 (Fri, 03 May 2013) $
 *
 * Purpose : OMCI driver layer module defination
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) OMCI (G.984.4)
 *
 */

#ifndef __OMCI_RF_RTL9607_H__
#define __OMCI_RF_RTL9607_H__

#include <linux/list.h>
#include <rtk/classify.h>
#include <module/gpon/gpon.h>
#include <module/gpon/gpon_defs.h>
#include <DRV/omci_drv.h>
#include "omci_drv_ext.h"
#include <module/intr_bcaster/intr_bcaster.h>



/*
MACRO
*/



/*
DEFINE
*/

#define PF_VEIP_PROT 	4
#define PF_VEIP_SUPPORT 1

// refers to mib_PriQ.h
enum {
    PQ_DROP_COLOUR_NO_MARKING,
    PQ_DROP_COLOUR_INTERNAL_MARKING,
    PQ_DROP_COLOUR_DEI_MARKING,
    PQ_DROP_COLOUR_PCP_8P0D_MARKING,
    PQ_DROP_COLOUR_PCP_7P1D_MARKING,
    PQ_DROP_COLOUR_PCP_6P2D_MARKING,
    PQ_DROP_COLOUR_PCP_5P3D_MARKING,
    PQ_DROP_COLOUR_DSCP_AF_CLASS_MARKING,
};


/*
CF rule mapping table
0~63	: VEIP fast path (64)
64~127	: L2 Service for EtherType filter (64)
128~511 : L2 Serivice for common (384)
*/

enum {
PF_CF_TYPE_VEIP_FAST = 0,
PF_CF_TYPE_L2_ETH_FILTER,
PF_CF_TYPE_L2_COMM
};

enum {
PF_VEIP_FAST_START 			= 0,
PF_VEIP_FAST_STOP  			= 63,
PF_L2_ETHTYPE_FILTER_START 	= 64,
PF_L2_ETHTYPE_FILTER_STOP  	= 127,
PF_L2_COMM_START 			= 128,
PF_L2_COMM_STOP 			= 511
};

enum {
VID = 0,
PRI
};

#define GET_ACT(type, outNum, vlanAct, act) ({ 												\
	unsigned int __ret = 0; 																\
	if (1 == outNum)																		\
	{																						\
		__ret = (type == VID ? VID_ACT_COPY_OUTER : PRI_ACT_COPY_OUTER); 					\
	} 																						\
	else if (1 < outNum) 																	\
	{																						\
		if(PRI == type)																		\
		{																					\
			__ret = ((PRI_ACT_COPY_INNER == act || VLAN_ACT_NON == vlanAct ||				\
			VLAN_ACT_TRANSPARENT == vlanAct) ? PRI_ACT_COPY_INNER : PRI_ACT_COPY_OUTER);	\
		}																					\
		else if(VID == type)																\
		{																					\
			__ret = ((VID_ACT_COPY_INNER == act || VLAN_ACT_NON == vlanAct ||				\
			VLAN_ACT_TRANSPARENT == vlanAct) ? VID_ACT_COPY_INNER : VID_ACT_COPY_OUTER);	\
		}																					\
	}																						\
	__ret;																					\
})

/*for handle ioctl handler*/
typedef struct omci_work_s {
	rtk_gpon_omci_msg_t omci;
	struct work_struct work;
}omci_work_t;



#endif

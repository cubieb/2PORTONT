#ifndef __RTK_API_L2_H__
#define __RTK_API_L2_H__

#include "rtk_api_common.h"
#include "rtk_types_common.h"
#include "rtk_error.h"

#define RTK_MAX_NUM_OF_LEARN_LIMIT                  0x840


typedef struct rtk_l2_addr_table_s
{
	rtk_int32 	index;
	ipaddr_t    sip;
	ipaddr_t    dip;
	rtk_mac_t   mac;
	rtk_uint32 	sa_block;
	rtk_uint32 	auth;
	rtk_uint32 	portmask;
	rtk_uint32 	age;
    rtk_uint32 	ivl;
    rtk_uint32 	cvid;
	rtk_uint32 	fid;
	rtk_uint32 	efid;
	rtk_uint32 	is_ipmul;
	rtk_uint32 	is_static;
}rtk_l2_addr_table_t;

rtk_api_ret_t rtk_l2_entry_get(rtk_l2_addr_table_t *pL2_entry);

#endif

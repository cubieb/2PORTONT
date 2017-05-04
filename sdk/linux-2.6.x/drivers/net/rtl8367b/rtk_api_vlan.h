#ifndef __RTK_API_VLAN_H__
#define __RTK_API_VLAN_H__

#include "rtk_api_common.h"
#include "rtk_types_common.h"
#include "rtk_error.h"

#define RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST    1


typedef struct rtk_portmask_s
{
    rtk_uint32  bits[RTK_TOTAL_NUM_OF_WORD_FOR_1BIT_PORT_LIST];
} rtk_portmask_t;

typedef struct rtk_svlan_memberCfg_s{
    rtk_uint32 svid;
    rtk_uint32 memberport;
    rtk_uint32 untagport;
    rtk_uint32 fiden;
    rtk_uint32 fid;
    rtk_uint32 priority;
    rtk_uint32 efiden;
    rtk_uint32 efid;
}rtk_svlan_memberCfg_t;



typedef rtk_uint32  rtk_fid_t;        /* filter id type */
typedef rtk_uint32  rtk_vlan_t;        /* vlan id type */

rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_portmask_t *pMbrmsk, rtk_portmask_t *pUntagmsk, rtk_fid_t *pFid);
rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority);

rtk_api_ret_t rtk_svlan_memberPortEntry_get(rtk_vlan_t svid, rtk_svlan_memberCfg_t *pSvlan_cfg);
rtk_api_ret_t rtk_svlan_defaultSvlan_get(rtk_port_t port, rtk_vlan_t *pSvid);

#endif

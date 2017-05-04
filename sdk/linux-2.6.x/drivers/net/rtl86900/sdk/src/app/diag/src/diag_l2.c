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
 * $Revision: 43034 $
 * $Date: 2013-09-24 18:58:25 +0800 (Tue, 24 Sep 2013) $
 *
 * Purpose : Definition those Layer2 command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *
 */

/*
 * Include Files
 */
#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <diag_str.h>
#include <parser/cparser_priv.h>

#include <dal/apollo/raw/apollo_raw_l2.h>
#include <rtk/l2.h>


apollo_lut_table_t diag_lut;



void _diag_lutDisplay(apollo_lut_table_t *diag_lut)
{
	uint8 strBuffer[64];
	uint8 strDsl[64];
	uint8 strExt[64];
	uint8 strSip[64];
	uint8 strDip[64];

    apollo_raw_l2_ipMcHashType_t ipMcHashType;



	if(diag_lut->lookup_busy)
	{
		diag_util_mprintf("\nLUT Access Busy\n");
	}
    else if(!diag_lut->lookup_hit)
    {
        diag_util_mprintf("\nLUT Access Error\n");
    }
    else
    {
        diag_util_mprintf("\nLUT address: 0x%4.4x %s\n",diag_lut->address,(diag_lut->address > (APOLLO_LUT_4WAY_NO-1))?"(64 BCAM)":"(2K LUT)");

        if(diag_lut->l3lookup)
        {
            apollo_raw_l2_ipmcHashType_get(&ipMcHashType);



            diag_lut->gip = diag_lut->gip | 0xE0000000;
            diag_util_ip2str(strSip,diag_lut->sip_vid);
            diag_util_ip2str(strDip,diag_lut->gip);
            diag_util_mask2str(strDsl, diag_lut->dsl_mbr);
            diag_util_mask2str(strExt, diag_lut->ext_mbr);

    		diag_util_mprintf("IP Multicast table:\n");

            if(ipMcHashType == RAW_LUT_IPMCHASH_TYPE_GIPVID)
            {
                diag_util_mprintf("DestinationIP   Vid  Member Fwd Pri State  Ext   Dsl   DipOnly\n");

        		diag_util_mprintf("%-15s %-4d %-6s %-3s %-3d %-6s %-5s %-5s %s\n",
                                    strDip,
                                    diag_lut->sip_vid,
                                    diag_util_mask32tostr(diag_lut->mbr),
                                    diagStr_enDisplay[diag_lut->fwdpri_en],
                                    diag_lut->lut_pri,
                                    diagStr_l2LutStaticOrAutoStr[diag_lut->nosalearn],
                                    strExt,
                                    strDsl,
                                    diagStr_enDisplay[diag_lut->gip_only]);
            }
            else
            {
                diag_util_mprintf("DestinationIP   SourceIP        Member Fwd Pri State  Ext   Dsl\n");

        		diag_util_mprintf("%-15s %-15s %-6s %-3s %-3d %-6s %-5s %-5s %s\n",
                                    strDip,
                                    strSip,
                                    diag_util_mask32tostr(diag_lut->mbr),
                                    diagStr_enDisplay[diag_lut->fwdpri_en],
                                    diag_lut->lut_pri,
                                    diagStr_l2LutStaticOrAutoStr[diag_lut->nosalearn],
                                    strExt,
                                    strDsl,
                                    diagStr_enDisplay[diag_lut->gip_only]);
            }
        }
        else if(diag_lut->mac.octet[0]&0x01)
        {
            diag_util_mask2str(strDsl, diag_lut->dsl_mbr);
            diag_util_mask2str(strExt, diag_lut->ext_mbr);

            diag_util_mprintf("L2 Multicast table:\n");
			diag_util_mprintf("MACAddress         Member Fid_Vid FwdPriEn Pri State  Hash Ext   Dsl\n");

			diag_util_mprintf("%-18s %-6s %-7d %-8s %-3d %-6s %-4s %-5s %-s\n",
			                    diag_util_inet_mactoa(&diag_lut->mac.octet[0]),
			                    diag_util_mask32tostr(diag_lut->mbr),
			                    diag_lut->cvid_fid,
			                    diagStr_enDisplay[diag_lut->fwdpri_en],
			                    diag_lut->lut_pri,
			                    diagStr_l2LutStaticOrAutoStr[diag_lut->nosalearn],
			                    diagStr_l2HashMethodStr[diag_lut->ivl_svl],
                                strExt,
                                strDsl);
        }
        else
        {
            diag_util_mprintf("L2 Unicast table:\n");
			diag_util_mprintf("MACAddress        Spa Fid Efid Age Vid  State  Hash\n");
 			diag_util_mprintf("%-17s %-3d %-3d %-4d %-3d %-4d %-6s %s\n",
			                    diag_util_inet_mactoa(&diag_lut->mac.octet[0]),
			                    diag_lut->spa,
			                    diag_lut->fid,
			                    diag_lut->efid,
			                    diag_lut->age,
			                    diag_lut->cvid_fid,
			                    diagStr_l2LutStaticOrAutoStr[diag_lut->nosalearn],
			                    diagStr_l2HashMethodStr[diag_lut->ivl_svl]);

			diag_util_mprintf("SaPriEn FwdPriEn Pri Auth DaBlock SaBlock Arp ExtDsl\n");
 			diag_util_mprintf("%-7s %-8s %-3d %-4s %-7s %-7s %-3s %d\n",
			                    diagStr_enDisplay[diag_lut->sapri_en],
                                diagStr_enDisplay[diag_lut->fwdpri_en],
                                diag_lut->lut_pri,
                                diagStr_enDisplay[diag_lut->auth],
                                diagStr_enDisplay[diag_lut->da_block],
                                diagStr_enDisplay[diag_lut->sa_block],
                                diagStr_enDisplay[diag_lut->arp_used],
                                diag_lut->ext_dsl_spa);
		}
    }
}

/*
 * l2-table init
 */
cparser_result_t
cparser_cmd_l2_table_init(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(rtk_l2_init(), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_init */



/*
 * l2-table del all { include-static }
 */
cparser_result_t
cparser_cmd_l2_table_del_all_include_static(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_flush_ctrl_t flushCtrl;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();

    if (4 == TOKEN_NUM())
    {
        flushCtrl.flushType = RAW_FLUSH_TYPE_BOTH;
    }
    else
    {
        flushCtrl.flushType = RAW_FLUSH_TYPE_DYNAMIC;
    }

    flushCtrl.flushMode = RAW_FLUSH_MODE_PORT;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushCtrl_set(&flushCtrl), ret);

    HAL_GET_ALL_PORTMASK(portlist.portmask);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushEn_set(port,ENABLED), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_del_all_include_static */

/*
 * l2-table del ip-mcast dip <IPV4ADDR:dip>
 */
cparser_result_t
cparser_cmd_l2_table_del_ip_mcast_dip_dip(
    cparser_context_t *context,
    uint32_t  *dip_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = DISABLED;
    diag_lut.l3lookup = ENABLED;
    diag_lut.gip_only = ENABLED;
    diag_lut.gip = *dip_ptr;


    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_del_ip_mcast_dip_dip */

/*
 * l2-table del ip-mcast sip <IPV4ADDR:sip> dip <IPV4ADDR:dip>
 */
cparser_result_t
cparser_cmd_l2_table_del_ip_mcast_sip_sip_dip_dip(
    cparser_context_t *context,
    uint32_t  *sip_ptr,
    uint32_t  *dip_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = DISABLED;
    diag_lut.l3lookup = ENABLED;
    diag_lut.gip = *dip_ptr;
    diag_lut.sip_vid = *sip_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_del_ip_mcast_sip_sip_dip_dip */

/*
 * l2-table del ip-mcast vid <UINT:vid> dip <IPV4ADDR:dip>
 */
cparser_result_t
cparser_cmd_l2_table_del_ip_mcast_vid_vid_dip_dip(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *dip_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = DISABLED;
    diag_lut.l3lookup = ENABLED;
    diag_lut.gip = *dip_ptr;
    diag_lut.sip_vid = *vid_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_del_ip_mcast_vid_vid_dip_dip */

/*
 * l2-table del mac-mcast fid <UINT:fid> mac_addr <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_l2_table_del_mac_mcast_fid_fid_mac_addr_mac(
    cparser_context_t *context,
    uint32_t  *fid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2MC_DSL;
    diag_lut.ivl_svl = RAW_L2_HASH_SVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.cvid_fid = *fid_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_del_mac_mcast_fid_fid_mac_addr_mac */

/*
 * l2-table del mac-mcast vid <UINT:vid> mac_addr <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_l2_table_del_mac_mcast_vid_vid_mac_addr_mac(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2MC_DSL;
    diag_lut.ivl_svl = RAW_L2_HASH_IVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.cvid_fid = *vid_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_del_mac_mcast_vid_vid_mac_addr_mac */


/*
 * l2-table del mac-ucast vid <UINT:vid> mac_addr <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_l2_table_del_mac_ucast_vid_vid_mac_addr_mac(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2UC;
    diag_lut.ivl_svl = RAW_L2_HASH_IVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.cvid_fid = *vid_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_del_mac_ucast_vid_vid_mac_addr_mac */

/*
 * l2-table del mac-ucast fid <UINT:fid> mac_addr <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_l2_table_del_mac_ucast_fid_fid_mac_addr_mac(
    cparser_context_t *context,
    uint32_t  *fid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2UC;
    diag_lut.ivl_svl = RAW_L2_HASH_SVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.fid = *fid_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_del_mac_ucast_fid_fid_mac_addr_mac */

/*
 * l2-table add ip-mcast dip <IPV4ADDR:dip> port ( <PORT_LIST:port> | all | none )
 */
cparser_result_t
cparser_cmd_l2_table_add_ip_mcast_dip_dip_port_port_all_none(
    cparser_context_t *context,
    uint32_t  *dip_ptr,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = ENABLED;

    diag_lut.gip = *dip_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];
    diag_lut.l3lookup = ENABLED;
    diag_lut.gip_only = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_ip_mcast_dip_dip_port_port_all_none */

/*
 * l2-table add ip-mcast dip <IPV4ADDR:dip> port ( <PORT_LIST:port> | all | none ) fwdpri ( disable | enable ) priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_l2_table_add_ip_mcast_dip_dip_port_port_all_none_fwdpri_disable_enable_priority_pri(
    cparser_context_t *context,
    uint32_t  *dip_ptr,
    char * *port_ptr,
    uint32_t  *pri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = ENABLED;

    diag_lut.gip = *dip_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];

    if ('d' == TOKEN_CHAR(8,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(8,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }

    diag_lut.lut_pri = *pri_ptr;
    diag_lut.l3lookup = ENABLED;
    diag_lut.gip_only = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_ip_mcast_dip_dip_port_port_all_none_fwdpri_disable_enable_priority_pri */

/*
 * l2-table add ip-mcast dip <IPV4ADDR:dip> port ( <PORT_LIST:port> | all | none ) fwdpri ( disable | enable ) priority <UINT:pri> dsl ( <PORT_LIST:dsl> | dsl_all ) ext ( <PORT_LIST:ext> | ext_all ) l3routing ( disable | enable ) p0_idx <UINT:l3idxp0> p1_idx <UINT:l3idxp1> p2_idx <UINT:l3idxp2> p3_idx <UINT:l3idxp3> p4_idx <UINT:l3idxp4> p5_idx <UINT:l3idxp5> p6_idx <UINT:l3idxp6> forcedl3routing ( disable | enable )
 */
cparser_result_t
cparser_cmd_l2_table_add_ip_mcast_dip_dip_port_port_all_none_fwdpri_disable_enable_priority_pri_dsl_dsl_dsl_all_ext_ext_ext_all_l3routing_disable_enable_p0_idx_l3idxp0_p1_idx_l3idxp1_p2_idx_l3idxp2_p3_idx_l3idxp3_p4_idx_l3idxp4_p5_idx_l3idxp5_p6_idx_l3idxp6_forcedl3routing_disable_enable(
    cparser_context_t *context,
    uint32_t  *dip_ptr,
    char * *port_ptr,
    uint32_t  *pri_ptr,
    char * *dsl_ptr,
    char * *ext_ptr,
    uint32_t  *l3idxp0_ptr,
    uint32_t  *l3idxp1_ptr,
    uint32_t  *l3idxp2_ptr,
    uint32_t  *l3idxp3_ptr,
    uint32_t  *l3idxp4_ptr,
    uint32_t  *l3idxp5_ptr,
    uint32_t  *l3idxp6_ptr)
{

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_ip_mcast_dip_dip_port_port_all_none_fwdpri_disable_enable_priority_pri_dsl_dsl_dsl_all_ext_ext_ext_all_l3routing_disable_enable_p0_idx_l3idxp0_p1_idx_l3idxp1_p2_idx_l3idxp2_p3_idx_l3idxp3_p4_idx_l3idxp4_p5_idx_l3idxp5_p6_idx_l3idxp6_forcedl3routing_disable_enable */

/*
 * l2-table add ip-mcast sip <IPV4ADDR:sip> dip <IPV4ADDR:dip> port ( <PORT_LIST:port> | all | none )
 */
cparser_result_t
cparser_cmd_l2_table_add_ip_mcast_sip_sip_dip_dip_port_port_all_none(
    cparser_context_t *context,
    uint32_t  *sip_ptr,
    uint32_t  *dip_ptr,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = ENABLED;

    diag_lut.gip = *dip_ptr;
    diag_lut.sip_vid = *sip_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];

    diag_lut.l3lookup = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_ip_mcast_sip_sip_dip_dip_port_port_all_none */

/*
 * l2-table add ip-mcast sip <IPV4ADDR:sip> dip <IPV4ADDR:dip> port ( <PORT_LIST:port> | all | none ) fwdpri ( disable | enable ) priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_l2_table_add_ip_mcast_sip_sip_dip_dip_port_port_all_none_fwdpri_disable_enable_priority_pri(
    cparser_context_t *context,
    uint32_t  *sip_ptr,
    uint32_t  *dip_ptr,
    char * *port_ptr,
    uint32_t  *pri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = ENABLED;

    diag_lut.gip = *dip_ptr;
    diag_lut.sip_vid = *sip_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];

    if ('d' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }
    diag_lut.lut_pri = *pri_ptr;

    diag_lut.l3lookup = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_ip_mcast_sip_sip_dip_dip_port_port_all_none_fwdpri_disable_enable_priority_pri */

/*
 * l2-table add ip-mcast sip <IPV4ADDR:sip> dip <IPV4ADDR:dip> port ( <PORT_LIST:port> | all | none ) fwdpri ( disable | enable ) priority <UINT:pri> dsl ( <PORT_LIST:dsl> | dsl_all ) ext ( <PORT_LIST:ext> | ext_all )
 */
cparser_result_t
cparser_cmd_l2_table_add_ip_mcast_sip_sip_dip_dip_port_port_all_none_fwdpri_disable_enable_priority_pri_dsl_dsl_dsl_all_ext_ext_ext_all(
    cparser_context_t *context,
    uint32_t  *sip_ptr,
    uint32_t  *dip_ptr,
    char * *port_ptr,
    uint32_t  *pri_ptr,
    char * *dsl_ptr,
    char * *ext_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    diag_portlist_t extlist;
    diag_portlist_t dsllist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(dsllist, 14), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(extlist, 16), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = ENABLED;

    diag_lut.gip = *dip_ptr;
    diag_lut.sip_vid = *sip_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];
    diag_lut.dsl_mbr = dsllist.portmask.bits[0];
    diag_lut.ext_mbr = extlist.portmask.bits[0];

    if ('d' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }
    diag_lut.lut_pri = *pri_ptr;

    diag_lut.l3lookup = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_ip_mcast_sip_sip_dip_dip_port_port_all_none_fwdpri_disable_enable_priority_pri_dsl_dsl_dsl_all_ext_ext_ext_all */

/*
 * l2-table add ip-mcast vid <UINT:vid> dip <IPV4ADDR:dip> port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_l2_table_add_ip_mcast_vid_vid_dip_dip_port_port_all(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *dip_ptr,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = ENABLED;

    diag_lut.gip = *dip_ptr;
    diag_lut.sip_vid = *vid_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];

    diag_lut.l3lookup = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_ip_mcast_vid_vid_dip_dip_port_port_all */

/*
 * l2-table add ip-mcast vid <UINT:vid> dip <IPV4ADDR:dip> port ( <PORT_LIST:port> | all ) fwdpri ( disable | enable ) priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_l2_table_add_ip_mcast_vid_vid_dip_dip_port_port_all_fwdpri_disable_enable_priority_pri(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *dip_ptr,
    char * *port_ptr,
    uint32_t  *pri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = ENABLED;

    diag_lut.gip = *dip_ptr;
    diag_lut.sip_vid = *vid_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];

    if ('d' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }

    diag_lut.lut_pri = *pri_ptr;
    diag_lut.l3lookup = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_ip_mcast_vid_vid_dip_dip_port_port_all_fwdpri_disable_enable_priority_pri */

/*
 * l2-table add ip-mcast vid <UINT:vid> dip <IPV4ADDR:dip> port ( <PORT_LIST:port> | all ) fwdpri ( disable | enable ) priority <UINT:pri> dsl ( <PORT_LIST:dsl> | dsl_all ) ext ( <PORT_LIST:ext> | ext_all )
 */
cparser_result_t
cparser_cmd_l2_table_add_ip_mcast_vid_vid_dip_dip_port_port_all_fwdpri_disable_enable_priority_pri_dsl_dsl_dsl_all_ext_ext_ext_all(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *dip_ptr,
    char * *port_ptr,
    uint32_t  *pri_ptr,
    char * *dsl_ptr,
    char * *ext_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    diag_portlist_t extlist;
    diag_portlist_t dsllist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(dsllist, 14), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(extlist, 16), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.nosalearn = ENABLED;

    diag_lut.gip = *dip_ptr;
    diag_lut.sip_vid = *vid_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];
    diag_lut.dsl_mbr = dsllist.portmask.bits[0];
    diag_lut.ext_mbr = extlist.portmask.bits[0];

    if ('d' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }
    diag_lut.lut_pri = *pri_ptr;

    diag_lut.l3lookup = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_ip_mcast_vid_vid_dip_dip_port_port_all_fwdpri_disable_enable_priority_pri_dsl_dsl_dsl_all_ext_ext_ext_all */

/*
 * l2-table add mac-mcast fid <UINT:fid> mac_addr <MACADDR:mac> port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_l2_table_add_mac_mcast_fid_fid_mac_addr_mac_port_port_all(
    cparser_context_t *context,
    uint32_t  *fid_ptr,
    cparser_macaddr_t  *mac_ptr,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2MC_DSL;
    diag_lut.ivl_svl = RAW_L2_HASH_SVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.cvid_fid = *fid_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];

    diag_lut.nosalearn = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_mac_mcast_fid_fid_mac_addr_mac_port_port_all */

/*
 * l2-table add mac-mcast fid <UINT:fid> mac_addr <MACADDR:mac> port ( <PORT_LIST:port> | all ) fwdpri ( disable | enable ) priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_l2_table_add_mac_mcast_fid_fid_mac_addr_mac_port_port_all_fwdpri_disable_enable_priority_pri(
    cparser_context_t *context,
    uint32_t  *fid_ptr,
    cparser_macaddr_t  *mac_ptr,
    char * *port_ptr,
    uint32_t  *pri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2MC_DSL;
    diag_lut.ivl_svl = RAW_L2_HASH_SVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.cvid_fid = *fid_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];

    if ('d' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }
    diag_lut.lut_pri = *pri_ptr;

    diag_lut.nosalearn = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_mac_mcast_fid_fid_mac_addr_mac_port_port_all_fwdpri_disable_enable_priority_pri */

/*
 * l2-table add mac-mcast fid <UINT:fid> mac_addr <MACADDR:mac> port ( <PORT_LIST:port> | all ) fwdpri ( disable | enable ) priority <UINT:pri> dsl ( <PORT_LIST:dsl> | dsl_all ) ext ( <PORT_LIST:ext> | ext_all )
 */
cparser_result_t
cparser_cmd_l2_table_add_mac_mcast_fid_fid_mac_addr_mac_port_port_all_fwdpri_disable_enable_priority_pri_dsl_dsl_dsl_all_ext_ext_ext_all(
    cparser_context_t *context,
    uint32_t  *fid_ptr,
    cparser_macaddr_t  *mac_ptr,
    char * *port_ptr,
    uint32_t  *pri_ptr,
    char * *dsl_ptr,
    char * *ext_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    diag_portlist_t extlist;
    diag_portlist_t dsllist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(dsllist, 14), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(extlist, 16), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2MC_DSL;
    diag_lut.ivl_svl = RAW_L2_HASH_SVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.cvid_fid = *fid_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];
    diag_lut.dsl_mbr = dsllist.portmask.bits[0];
    diag_lut.ext_mbr = extlist.portmask.bits[0];

    if ('d' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }
    diag_lut.lut_pri = *pri_ptr;

    diag_lut.nosalearn = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_mac_mcast_fid_fid_mac_addr_mac_port_port_all_fwdpri_disable_enable_priority_pri_dsl_dsl_dsl_all_ext_ext_ext_all */

/*
 * l2-table add mac-mcast vid <UINT:vid> mac_addr <MACADDR:mac> port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_l2_table_add_mac_mcast_vid_vid_mac_addr_mac_port_port_all(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2MC_DSL;
    diag_lut.ivl_svl = RAW_L2_HASH_IVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.cvid_fid = *vid_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];

    diag_lut.nosalearn = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_mac_mcast_vid_vid_mac_addr_mac_port_port_all */

/*
 * l2-table add mac-mcast vid <UINT:vid> mac_addr <MACADDR:mac> port ( <PORT_LIST:port> | all ) fwdpri ( disable | enable ) priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_l2_table_add_mac_mcast_vid_vid_mac_addr_mac_port_port_all_fwdpri_disable_enable_priority_pri(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr,
    char * *port_ptr,
    uint32_t  *pri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2MC_DSL;
    diag_lut.ivl_svl = RAW_L2_HASH_IVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.cvid_fid = *vid_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];

    if ('d' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }
    diag_lut.lut_pri = *pri_ptr;

    diag_lut.nosalearn = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_mac_mcast_vid_vid_mac_addr_mac_port_port_all_fwdpri_disable_enable_priority_pri */

/*
 * l2-table add mac-mcast vid <UINT:vid> mac_addr <MACADDR:mac> port ( <PORT_LIST:port> | all ) fwdpri { disable | enable } priority <UINT:pri> dsl ( <PORT_LIST:dsl> | dsl_all ) ext ( <PORT_LIST:ext> | ext_all )
 */
cparser_result_t
cparser_cmd_l2_table_add_mac_mcast_vid_vid_mac_addr_mac_port_port_all_fwdpri_disable_enable_priority_pri_dsl_dsl_dsl_all_ext_ext_ext_all(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr,
    char * *port_ptr,
    uint32_t  *pri_ptr,
    char * *dsl_ptr,
    char * *ext_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    diag_portlist_t extlist;
    diag_portlist_t dsllist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(dsllist, 14), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(extlist, 16), ret);

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2MC_DSL;
    diag_lut.ivl_svl = RAW_L2_HASH_IVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.cvid_fid = *vid_ptr;
    diag_lut.mbr = portlist.portmask.bits[0];
    diag_lut.dsl_mbr = dsllist.portmask.bits[0];
    diag_lut.ext_mbr = extlist.portmask.bits[0];

    if ('d' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(10,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }
    diag_lut.lut_pri = *pri_ptr;

    diag_lut.nosalearn = ENABLED;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_mac_mcast_vid_vid_mac_addr_mac_port_port_all_fwdpri_disable_enable_priority_pri_dsl_dsl_dsl_all_ext_ext_ext_all */


/*
 * l2-table add mac-ucast vid <UINT:vid> mac_addr <MACADDR:mac> spa <UINT:spa>
 */
cparser_result_t
cparser_cmd_l2_table_add_mac_ucast_vid_vid_mac_addr_mac_spa_spa(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *spa_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2UC;
    diag_lut.ivl_svl = RAW_L2_HASH_IVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.spa = *spa_ptr;
    diag_lut.cvid_fid = *vid_ptr;

    diag_lut.nosalearn = ENABLED;
    /*diag_lut.age = 7;*/

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_mac_ucast_vid_vid_mac_addr_mac_spa_spa */

/*
 * l2-table add mac-ucast vid <UINT:vid> mac_addr <MACADDR:mac> spa <UINT:spa> ( svl | ivl ) fid <UINT:fid>
 */
cparser_result_t
cparser_cmd_l2_table_add_mac_ucast_vid_vid_mac_addr_mac_spa_spa_svl_ivl_fid_fid(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *spa_ptr,
    uint32_t  *fid_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2UC;
    if ('i' == TOKEN_CHAR(9,0))
    {
        diag_lut.ivl_svl = RAW_L2_HASH_IVL;
    }
    else if ('s' == TOKEN_CHAR(9,0))
    {
        diag_lut.ivl_svl = RAW_L2_HASH_SVL;
    }

    diag_lut.cvid_fid = *vid_ptr;
    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.spa = *spa_ptr;
    diag_lut.fid = *fid_ptr;

    diag_lut.nosalearn = ENABLED;
    /*diag_lut.age = 7;*/


    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_mac_ucast_vid_vid_mac_addr_mac_spa_spa_svl_ivl_fid_fid */


/*
 * l2-table add mac-ucast vid <UINT:vid> mac_addr <MACADDR:mac> spa <UINT:spa> ( svl | ivl ) fid <UINT:fid> age <UINT:age> efid <UINT:efid> fwdpri ( disable | enable ) sapri ( disable | enable ) priority <UINT:pri> { auth } { da-block  } { sa-block  } { static }
 */
cparser_result_t
cparser_cmd_l2_table_add_mac_ucast_vid_vid_mac_addr_mac_spa_spa_svl_ivl_fid_fid_age_age_efid_efid_fwdpri_disable_enable_sapri_disable_enable_priority_pri_auth_da_block_sa_block_static(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *spa_ptr,
    uint32_t  *fid_ptr,
    uint32_t  *age_ptr,
    uint32_t  *efid_ptr,
    uint32_t  *pri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 tokenIdx;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2UC;
    if ('i' == TOKEN_CHAR(9,0))
    {
        diag_lut.ivl_svl = RAW_L2_HASH_IVL;
    }
    else if ('s' == TOKEN_CHAR(9,0))
    {
        diag_lut.ivl_svl = RAW_L2_HASH_SVL;
    }

    diag_lut.cvid_fid = *vid_ptr;
    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.spa = *spa_ptr;
    diag_lut.fid = *fid_ptr;
    diag_lut.age = *age_ptr;
    diag_lut.efid = *efid_ptr;

    if ('d' == TOKEN_CHAR(17,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(17,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }

    if ('d' == TOKEN_CHAR(19,0))
    {
        diag_lut.sapri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(19,0))
    {
        diag_lut.sapri_en = ENABLED;
    }

    diag_lut.lut_pri = *pri_ptr;


    for(tokenIdx = 22; tokenIdx <= TOKEN_NUM();tokenIdx ++)
    {
        if(!osal_strcmp(TOKEN_STR(tokenIdx),"auth"))
            diag_lut.auth = ENABLED;
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"sa-block"))
            diag_lut.sa_block = ENABLED;
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"da-block"))
            diag_lut.da_block = ENABLED;
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"static"))
        {
            diag_lut.nosalearn = ENABLED;
            /*diag_lut.age = 7;*/
        }
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_mac_ucast_vid_vid_mac_addr_mac_spa_spa_svl_ivl_fid_fid_age_age_efid_efid_fwdpri_disable_enable_sapri_disable_enable_priority_pri_auth_da_block_sa_block_static */

/*
 * l2-table add mac-ucast vid <UINT:vid> mac_addr <MACADDR:mac> spa <UINT:spa> ( svl | ivl ) fid <UINT:fid> age <UINT:age> efid <UINT:efid> fwdpri ( disable | enable ) sapri ( disable | enable ) priority <UINT:pri> ext_vc <UINT:ext_vc_spa> { arp-usage } { auth } { da-block  } { sa-block  } { static }
 */
cparser_result_t
cparser_cmd_l2_table_add_mac_ucast_vid_vid_mac_addr_mac_spa_spa_svl_ivl_fid_fid_age_age_efid_efid_fwdpri_disable_enable_sapri_disable_enable_priority_pri_ext_vc_ext_vc_spa_arp_usage_auth_da_block_sa_block_static(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *spa_ptr,
    uint32_t  *fid_ptr,
    uint32_t  *age_ptr,
    uint32_t  *efid_ptr,
    uint32_t  *pri_ptr,
    uint32_t  *ext_vc_spa_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 tokenIdx;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2UC;
    if ('i' == TOKEN_CHAR(9,0))
    {
        diag_lut.ivl_svl = RAW_L2_HASH_IVL;
    }
    else if ('s' == TOKEN_CHAR(9,0))
    {
        diag_lut.ivl_svl = RAW_L2_HASH_SVL;
    }

    diag_lut.cvid_fid = *vid_ptr;
    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.spa = *spa_ptr;
    diag_lut.fid = *fid_ptr;
    diag_lut.age = *age_ptr;
    diag_lut.efid = *efid_ptr;
    diag_lut.ext_dsl_spa = *ext_vc_spa_ptr;


    if ('d' == TOKEN_CHAR(17,0))
    {
        diag_lut.fwdpri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(17,0))
    {
        diag_lut.fwdpri_en = ENABLED;
    }

    if ('d' == TOKEN_CHAR(19,0))
    {
        diag_lut.sapri_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(19,0))
    {
        diag_lut.sapri_en = ENABLED;
    }

    diag_lut.lut_pri = *pri_ptr;


    for(tokenIdx = 24; tokenIdx <= TOKEN_NUM();tokenIdx ++)
    {
        if(!osal_strcmp(TOKEN_STR(tokenIdx),"auth"))
            diag_lut.auth = ENABLED;
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"sa-block"))
            diag_lut.sa_block = ENABLED;
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"da-block"))
            diag_lut.da_block = ENABLED;
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"static"))
        {
            diag_lut.nosalearn = ENABLED;
            /*diag_lut.age = 7;*/
        }
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"arp-usage"))
            diag_lut.arp_used = ENABLED;
    }


    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_set(&diag_lut), ret);


    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_add_mac_ucast_vid_vid_mac_addr_mac_spa_spa_svl_ivl_fid_fid_age_age_efid_efid_ext_vc_ext_vc_spa_fwdpri_disable_enable_sapri_disable_enable_priority_auth_sa_block_da_block_static_arp_usage */



/*
 * l2-table get aging-out port ( <PORT_LIST:port> | all ) state
 */
cparser_result_t
cparser_cmd_l2_table_get_aging_out_port_port_all_state(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port State\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_agingEnable_get(port, &enable), ret);
        diag_util_mprintf("%-4d %s\n", port, diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_aging_out_port_port_all_state */

/*
 * l2-table get aging-time
 */
cparser_result_t
cparser_cmd_l2_table_get_aging_time(
    cparser_context_t *context)
{

    int32 ret = RT_ERR_FAILED;
    uint32  agingTime;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();


    DIAG_UTIL_ERR_CHK(apollo_raw_l2_agingTime_get(&agingTime), ret);

    diag_util_mprintf("L2 aging timer %ld (unit 0.1 second)\n", agingTime);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_aging_time */

/*
 * l2-table get cam state
 */
cparser_result_t
cparser_cmd_l2_table_get_cam_state(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_camEnable_get(&enable), ret);

    diag_util_mprintf("L2 CAM usage: %s\n",diagStr_enable[enable]);


    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_cam_state */

/*
 * l2-table get ip-mcast dip <IPV4ADDR:dip>
 */
cparser_result_t
cparser_cmd_l2_table_get_ip_mcast_dip_dip(
    cparser_context_t *context,
    uint32_t  *dip_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.l3lookup = ENABLED;
    diag_lut.gip_only = ENABLED;
    diag_lut.gip = *dip_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_get(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_ip_mcast_dip_dip */

/*
 * l2-table get ip-mcast sip <IPV4ADDR:sip> dip <IPV4ADDR:dip>
 */
cparser_result_t
cparser_cmd_l2_table_get_ip_mcast_sip_sip_dip_dip(
    cparser_context_t *context,
    uint32_t  *sip_ptr,
    uint32_t  *dip_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.l3lookup = ENABLED;
    diag_lut.gip = *dip_ptr;
    diag_lut.sip_vid = *sip_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_get(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_ip_mcast_sip_sip_dip_dip */

/*
 * l2-table get ip-mcast vid <UINT:vid> dip <IPV4ADDR:dip>
 */
cparser_result_t
cparser_cmd_l2_table_get_ip_mcast_vid_vid_dip_dip(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *dip_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L3MC_DSL;
    diag_lut.l3lookup = ENABLED;
    diag_lut.gip = *dip_ptr;
    diag_lut.sip_vid = *vid_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_get(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_ip_mcast_vid_vid_dip_dip */

/*
 * l2-table get ipmc-lookup-op
 */
cparser_result_t
cparser_cmd_l2_table_get_ipmc_lookup_op(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    apollo_raw_l2_ipMcLookupOp_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_ipmcLookupOp_get(&type), ret);

    diag_util_mprintf("Mathced hash type: %s\n",diagStr_l2IpMcHashOpStr[type]);


    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_ipmc_lookup_op */

/*
 * l2-table get ipmc-mode
 */
cparser_result_t
cparser_cmd_l2_table_get_ipmc_mode(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    apollo_raw_l2_ipMcHashType_t type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_ipmcHashType_get(&type), ret);

    diag_util_mprintf("IPv4 mulitcast hash method: %s\n",diagStr_l2IpMcHashMethodStr[type]);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_ipmc_mode */

/*
 * l2-table get limit-learning action
 */
cparser_result_t
cparser_cmd_l2_table_get_limit_learning_action(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_action_t action;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_sysLrnOverAct_get(&action), ret);

    diag_util_mprintf("System: %s\n", diagStr_actionStr[action]);

    for(port=0;port <=APOLLO_PORTIDMAX;port++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_learnOverAct_get(port,&action), ret);
        diag_util_mprintf("Port%2d: %s\n", port, diagStr_actionStr[action]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_limit_learning_action */

/*
 * l2-table get limit-learning count
 */
cparser_result_t
cparser_cmd_l2_table_get_limit_learning_count(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 num;;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("         Current Limit\n");
    DIAG_UTIL_ERR_CHK(apollo_raw_l2_SysLrnCnt_get(&num), ret);
    diag_util_printf("System:  %-7d ", num);

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_sysLrnLimitNo_get(&num), ret);
    diag_util_mprintf("%d\n", num);

    for(port = 0; port <= APOLLO_PORTIDMAX; port++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_lrnCnt_get(port, &num), ret);
        diag_util_printf("Port %d:  %-7d ", port, num);

        DIAG_UTIL_ERR_CHK(apollo_raw_l2_lrnLimitNo_get(port, &num), ret);
        diag_util_mprintf("%d\n", num);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_limit_learning_count */

/*
 * l2-table get limit-learning port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_l2_table_get_limit_learning_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_action_t action;
    uint32 num;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Current Limit Action\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_lrnCnt_get(port, &num), ret);
        diag_util_printf("%-4d %-7d ", port, num);

        DIAG_UTIL_ERR_CHK(apollo_raw_l2_lrnLimitNo_get(port, &num), ret);
        diag_util_mprintf("%-5d", num);

        DIAG_UTIL_ERR_CHK(apollo_raw_l2_learnOverAct_get(port, &action), ret);
        diag_util_mprintf("%s\n", diagStr_actionStr[action]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_limit_learning_port_port_all */

/*
 * l2-table get limit-learning port ( <PORT_LIST:port> | all ) count
 */
cparser_result_t
cparser_cmd_l2_table_get_limit_learning_port_port_all_count(
    cparser_context_t *context,
    char * *port_ptr)
{

    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint32 num;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Current Limit\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_lrnCnt_get(port, &num), ret);
        diag_util_printf("%-4d %-7d ", port, num);

        DIAG_UTIL_ERR_CHK(apollo_raw_l2_lrnLimitNo_get(port, &num), ret);
        diag_util_mprintf("%d\n", num);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_limit_learning_port_port_all_count */

/*
 * l2-table get limit-learning over-state ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_l2_table_get_limit_learning_over_state_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint32 status;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Current Limit Over State\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_lrnOverSts_get(port, &status), ret);
        diag_util_printf("%-4d %-7d ", port, (status ? "occur" : "clear"));
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_limit_learning_over_state_port_all */

/*
 * l2-table get link-down-flush state
 */
cparser_result_t
cparser_cmd_l2_table_get_link_down_flush_state(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushLinkDownPortAddrEnable_get(&enable), ret);

    diag_util_mprintf("Lut linkdown flush function: %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_link_down_flush_state */

/*
 * l2-table get lookup-miss ( bcast | unicast | multicast ) flood-ports
 */
cparser_result_t
cparser_cmd_l2_table_get_lookup_miss_bcast_unicast_multicast_flood_ports(
    cparser_context_t *context)
{
    int32                       ret;
    diag_portlist_t             portlist;
    rtk_portmask_t              target_mask;
    rtk_action_t                action;
    rtk_port_t                  port = 0;
    rtk_enable_t                state;
    uint8                       portStr[20];

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    /* All ports mask */
    portlist.portmask.bits[0] = APOLLO_PORTMASK;
    portlist.min = 0;
    portlist.max = APOLLO_PORTIDMAX;

    /* reset target portmask */
    target_mask.bits[0] = 0x00;

    if ('b' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_bcFlood_get(port, &state), ret);

            if(ENABLED == state)
                target_mask.bits[0] |= (0x01 << port);
        }

        DIAG_UTIL_ERR_CHK(diag_util_lPortMask2str(portStr, &target_mask), ret);
        diag_util_printf("\n Broadcast Flooding Portmask: %s", portStr);
    }
    else if ('u' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unknUcFlood_get(port, &state), ret);

            if(ENABLED == state)
                target_mask.bits[0] |= (0x01 << port);
        }

        DIAG_UTIL_ERR_CHK(diag_util_lPortMask2str(portStr, &target_mask), ret);
        diag_util_printf("\n Unknown Unicast Flooding Portmask: %s", portStr);
    }
    else if ('m' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unknMcFlood_get(port, &state), ret);

            if(ENABLED == state)
                target_mask.bits[0] |= (0x01 << port);
        }

        DIAG_UTIL_ERR_CHK(diag_util_lPortMask2str(portStr, &target_mask), ret);
        diag_util_printf("\n Unknown Multicast Flooding Portmask: %s", portStr);
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    diag_util_printf("\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_lookup_miss_bcast_unicast_multicast_flood_ports */

/*
 * l2-table get lookup-miss multicast trap-priority
 */
cparser_result_t
cparser_cmd_l2_table_get_lookup_miss_multicast_trap_priority(
    cparser_context_t *context)
{
    int32       ret;
    uint32      priority;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_unkn_mcPri_get(&priority), ret);
    diag_util_printf("\n Unknown Multicast Trap Priority: %d", priority);
    diag_util_printf("\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_lookup_miss_multicast_trap_priority */

/*
 * l2-table get lookup-miss port ( <PORT_LIST:ports> | all ) ( l2mc | ipmc | ip6mc | unicast )
 */
cparser_result_t
cparser_cmd_l2_table_get_lookup_miss_port_ports_all_l2mc_ipmc_ip6mc_unicast(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32                       ret;
    diag_portlist_t             portlist;
    rtk_action_t                action;
    rtk_port_t                  port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('l' == TOKEN_CHAR(5,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_unkn_l2Mc_get(port, &action), ret);
            diag_util_printf("\n Port %d Unknown L2 Multicast Action: %s", port, diagStr_actionStr[action]);
        }
    }
    else if ('m' == TOKEN_CHAR(5,2))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_unkn_ip4Mc_get(port, &action), ret);
            diag_util_printf("\n Port %d Unknown IPv4 Multicast Action: %s", port, diagStr_actionStr[action]);
        }
    }
    else if ('6' == TOKEN_CHAR(5,2))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_unkn_ip6Mc_get(port, &action), ret);
            diag_util_printf("\n Port %d Unknown IPv6 Multicast Action: %s", port, diagStr_actionStr[action]);
        }
    }
    else if ('u' == TOKEN_CHAR(5,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unkn_ucDaCtl_get(port, &action), ret);
            diag_util_printf("\n Port %d Unknown Unicast Action: %s", port, diagStr_actionStr[action]);
        }
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    diag_util_printf("\n");
    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_lookup_miss_port_port_all_l2mc_ipmc_ip6mc_unicast */

/*
 * l2-table get mac-ucast vid <UINT:vid> mac_addr <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_l2_table_get_mac_ucast_vid_vid_mac_addr_mac(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_MAC;
    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2UC;
    diag_lut.ivl_svl = RAW_L2_HASH_IVL;

    diag_lut.cvid_fid = *vid_ptr;
    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);

    apollo_raw_l2_lookUpTb_get(&diag_lut);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_mac_ucast_vid_vid_mac_addr_mac_ivl */

/*
 * l2-table get mac-ucast vid <UINT:vid> mac_addr <MACADDR:mac> efid <UINT:efid>
 */
cparser_result_t
cparser_cmd_l2_table_get_mac_ucast_vid_vid_mac_addr_mac_efid_efid(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *efid_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_MAC;
    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2UC;
    diag_lut.ivl_svl = RAW_L2_HASH_IVL;

    diag_lut.cvid_fid = *vid_ptr;
    diag_lut.efid = *efid_ptr;
    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);

    apollo_raw_l2_lookUpTb_get(&diag_lut);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_mac_ucast_vid_vid_mac_addr_mac_efid_efid */


/*
 * l2-table get mac-ucast filter-id <UINT:fid> mac_addr <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_l2_table_get_mac_ucast_filter_id_fid_mac_addr_mac(
    cparser_context_t *context,
    uint32_t  *fid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_MAC;
    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2UC;
    diag_lut.ivl_svl = RAW_L2_HASH_SVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.fid = *fid_ptr;

    apollo_raw_l2_lookUpTb_get(&diag_lut);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_mac_ucast_filter_id_fid_mac_addr_mac */

/*
 * l2-table get mac-ucast filter-id <UINT:fid> mac_addr <MACADDR:mac> efid <UINT:efid>
 */
cparser_result_t
cparser_cmd_l2_table_get_mac_ucast_filter_id_fid_mac_addr_mac_efid_efid(
    cparser_context_t *context,
    uint32_t  *fid_ptr,
    cparser_macaddr_t  *mac_ptr,
    uint32_t  *efid_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_MAC;
    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2UC;
    diag_lut.ivl_svl = RAW_L2_HASH_SVL;

    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);
    diag_lut.fid = *fid_ptr;
    diag_lut.efid = *efid_ptr;

    apollo_raw_l2_lookUpTb_get(&diag_lut);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_mac_ucast_filter_id_fid_mac_addr_mac_efid_efid */

/*
 * l2-table get mac-mcast vid <UINT:vid> mac_addr <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_l2_table_get_mac_mcast_vid_vid_mac_addr_mac(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_MAC;
    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2MC_DSL;
    diag_lut.ivl_svl = RAW_L2_HASH_IVL;

    diag_lut.cvid_fid = *vid_ptr;
    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);

    apollo_raw_l2_lookUpTb_get(&diag_lut);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_mac_mcast_vid_vid_mac_addr_mac */

/*
 * l2-table get mac-mcast filter-id <UINT:fid> mac_addr <MACADDR:mac>
 */
cparser_result_t
cparser_cmd_l2_table_get_mac_mcast_filter_id_fid_mac_addr_mac(
    cparser_context_t *context,
    uint32_t  *fid_ptr,
    cparser_macaddr_t  *mac_ptr)
{
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_MAC;
    diag_lut.table_type = RAW_LUT_ENTRY_TYPE_L2MC_DSL;
    diag_lut.ivl_svl = RAW_L2_HASH_SVL;

    diag_lut.cvid_fid = *fid_ptr;
    osal_memcpy(&diag_lut.mac.octet, mac_ptr->octet, ETHER_ADDR_LEN);

    apollo_raw_l2_lookUpTb_get(&diag_lut);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_mac_mcast_filter_id_fid_mac_addr_mac */

/*
 * l2-table get ( port-move | unknown-sa ) port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_l2_table_get_port_move_unknown_sa_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Action\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ('u' == TOKEN_CHAR(2,0))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unkn_saCtl_get(port, &action), ret);

            diag_util_mprintf("%-4d %s\n", port, diagStr_actionStr[action]);
        }
        else if ('p' == TOKEN_CHAR(2,0))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unMatched_saCtl_get(port, &action), ret);

            diag_util_mprintf("%-4d %s\n", port, diagStr_actionStr[action]);
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_port_move_unknown_sa_port_port_all */

/*
 * l2-table get src-mac port ( <PORT_LIST:port> | all ) asic-learn
 */
cparser_result_t
cparser_cmd_l2_table_get_src_mac_port_port_all_asic_learn(
    cparser_context_t *context,
    char * *port_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_src_mac_port_port_all_asic_learn */

#if defined(CMD_L2_TABLE_GET_VLAN_LEARNING_METHOD_VID_VID)
/*
 * l2-table get vlan-learning-method vid <UINT:vid>
 */
cparser_result_t
cparser_cmd_l2_table_get_vlan_learning_method_vid_vid(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_vlan_learning_method_vid_vid */
#endif
/*
 * l2-table set aging-out port ( <PORT_LIST:port> | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_l2_table_set_aging_out_port_port_all_state_disable_enable(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('d' == TOKEN_CHAR(6,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(6,0))
    {
        enable = ENABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_agingEnable_set(port, enable), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_aging_out_port_port_all_state_disable_enable */

/*
 * l2-table set aging-time <UINT:time>
 */
cparser_result_t
cparser_cmd_l2_table_set_aging_time_time(
    cparser_context_t *context,
    uint32_t  *time_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32  agingTime;

    DIAG_UTIL_PARAM_CHK();

    agingTime =  *time_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_agingTime_set(agingTime), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_aging_time_time */

/*
 * l2-table set cam state ( disable | enable )
 */
cparser_result_t
cparser_cmd_l2_table_set_cam_state_disable_enable(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_camEnable_set(enable), ret);


    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_cam_state_disable_enable */

/*
 * l2-table set flush mode ( dynamic | static | both )
 */
cparser_result_t
cparser_cmd_l2_table_set_flush_mode_dynamic_static_both(
    cparser_context_t *context)
{
    int32 ret;
    apollo_raw_flush_ctrl_t flushCtrl;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushCtrl_get(&flushCtrl), ret);

    if ('d' == TOKEN_CHAR(4,0))
    {
        flushCtrl.flushType = RAW_FLUSH_TYPE_DYNAMIC;
    }
    else if ('s' == TOKEN_CHAR(4,0))
    {
        flushCtrl.flushType = RAW_FLUSH_TYPE_STATIC;
    }
    else if ('b' == TOKEN_CHAR(4,0))
    {
        flushCtrl.flushType = RAW_FLUSH_TYPE_BOTH;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushCtrl_set(&flushCtrl), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_flush_mode_dynamic_static_both */

/*
 * l2-table get flush mode
 */
cparser_result_t
cparser_cmd_l2_table_get_flush_mode(
    cparser_context_t *context)
{
    int32 ret;
    apollo_raw_flush_ctrl_t flushCtrl;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushCtrl_get(&flushCtrl), ret);

    diag_util_mprintf("mode: %s\n", diagStr_l2flushMode[flushCtrl.flushType]);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_flush_mode */


/*
 * l2-table set flush mac-ucast port ( <PORT_LIST:port> | all ) filter-id <UINT:fid> { include-static }
 */
cparser_result_t
cparser_cmd_l2_table_set_flush_mac_ucast_port_port_all_filter_id_fid_include_static(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *fid_ptr)
{
    int32 ret;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_flush_ctrl_t flushCtrl;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    osal_memset(&flushCtrl,0x0,sizeof(apollo_raw_flush_ctrl_t));

    flushCtrl.flushMode = RAW_FLUSH_MODE_FID;
    if(TOKEN_NUM() == 9)
        flushCtrl.flushType = RAW_FLUSH_TYPE_BOTH;
    else
        flushCtrl.flushType = RAW_FLUSH_TYPE_DYNAMIC;

    flushCtrl.fid = *fid_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushCtrl_set(&flushCtrl), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushEn_set(port, ENABLED), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_flush_mac_ucast_port_port_all_filter_id_fid_include_static */

/*
 * l2-table set flush mac-ucast port ( <PORT_LIST:port> | all ) include-static
 */
cparser_result_t
cparser_cmd_l2_table_set_flush_mac_ucast_port_port_all_include_static(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_flush_ctrl_t flushCtrl;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    osal_memset(&flushCtrl,0x0,sizeof(apollo_raw_flush_ctrl_t));

    flushCtrl.flushMode = RAW_FLUSH_MODE_PORT;
    flushCtrl.flushType = RAW_FLUSH_TYPE_BOTH;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushCtrl_set(&flushCtrl), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushEn_set(port, ENABLED), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_flush_mac_ucast_port_port_all_include_static */

/*
 * l2-table set flush mac-ucast port ( <PORT_LIST:port> | all ) vid <UINT:vid> { include-static }
 */
cparser_result_t
cparser_cmd_l2_table_set_flush_mac_ucast_port_port_all_vid_vid_include_static(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *vid_ptr)
{
    int32 ret;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_flush_ctrl_t flushCtrl;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    osal_memset(&flushCtrl,0x0,sizeof(apollo_raw_flush_ctrl_t));

    flushCtrl.flushMode = RAW_FLUSH_MODE_VID;
    if(TOKEN_NUM() == 9)
        flushCtrl.flushType = RAW_FLUSH_TYPE_BOTH;
    else
        flushCtrl.flushType = RAW_FLUSH_TYPE_DYNAMIC;

    flushCtrl.vid = *vid_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushCtrl_set(&flushCtrl), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushEn_set(port, ENABLED), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_flush_mac_ucast_port_port_all_vid_vid_include_static */

/*
 * l2-table set flush mac-ucast port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_l2_table_set_flush_mac_ucast_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret;
    rtk_l2_flushCfg_t cfg;
    rtk_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        osal_memset(&cfg, 0x00, sizeof(rtk_l2_flushCfg_t));
        cfg.flushByPort = 1;
        cfg.port = port;
        cfg.flushDynamicAddr = 1;
        cfg.flushStaticAddr = 0;
        DIAG_UTIL_ERR_CHK(rtk_l2_ucastAddr_flush(&cfg), ret);
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_flush_mac_ucast_port_port_all */

/*
 * l2-table set ipmc-table index <UINT:index> group-ip <IPV4ADDR:gip> port ( <PORT_LIST:port> | all | none )
 */
cparser_result_t
cparser_cmd_l2_table_set_ipmc_table_index_index_group_ip_gip_port_port_all_none(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *gip_ptr,
    char * *port_ptr)
{
    int32                       ret;
    diag_portlist_t             portlist;
    uint32                      index, portmask;
    rtk_ip_addr_t               gip;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    index = *index_ptr;
    gip = (rtk_ip_addr_t)(*gip_ptr);
    portmask = portlist.portmask.bits[0];

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_igmp_Mc_table_set(index, gip, portmask), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_ipmc_table_index_index_group_ip_gip_port_port_all_none */

/*
 * l2-table get ipmc-table index <UINT:index>
 */
cparser_result_t
cparser_cmd_l2_table_get_ipmc_table_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32                       ret;
    uint32                      index;
    rtk_portmask_t              portmask;
    rtk_ip_addr_t               gip;
    uint8                       portStr[20];


    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    index = *index_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_l2_igmp_Mc_table_get(index, &gip, &portmask.bits[0]), ret);
    DIAG_UTIL_ERR_CHK(diag_util_lPortMask2str(portStr, &portmask), ret);
    diag_util_printf("\n index   : %d", index);
    diag_util_printf("\n Grout IP: %s", diag_util_inet_ntoa(gip));
    diag_util_printf("\n Portmask: %s\n", portStr);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_ipmc_table_index_index */

/*
 * l2-table set ipmc-lookup-op ( dip-and-sip | dip-only )
 */
cparser_result_t
cparser_cmd_l2_table_set_ipmc_lookup_op_dip_and_sip_dip_only(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    apollo_raw_l2_ipMcLookupOp_t type;

    DIAG_UTIL_PARAM_CHK();

    if ('o' == TOKEN_CHAR(3,4))
    {
        type = RAW_LUT_IPMCLOOKUP_TYPE_DIP;
    }
    else if ('a' == TOKEN_CHAR(3,4))
    {
        type = RAW_LUT_IPMCLOOKUP_TYPE_DIPSIP;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_ipmcLookupOp_set(type), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_ipmc_lookup_op_dip_and_sip_dip_only */

/*
 * l2-table set ipmc-mode ( dip-and-sip | dip-and-vid | vid-and-mac )
 */
cparser_result_t
cparser_cmd_l2_table_set_ipmc_mode_dip_and_sip_dip_and_vid_vid_and_mac(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    apollo_raw_l2_ipMcHashType_t type;

    DIAG_UTIL_PARAM_CHK();

    if ('m' == TOKEN_CHAR(3,8))
    {
        type = RAW_LUT_IPMCHASH_TYPE_DMACFID;
    }
    else if ('v' == TOKEN_CHAR(3,8))
    {
        type = RAW_LUT_IPMCHASH_TYPE_GIPVID;
    }
    else if ('s' == TOKEN_CHAR(3,8))
    {
        type = RAW_LUT_IPMCHASH_TYPE_DIPSIP;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_ipmcHashType_set(type), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_ipmc_mode_dip_and_sip_dip_and_vid_vid_and_mac */

/*
 * l2-table set limit-learning action ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_l2_table_set_limit_learning_action_copy_to_cpu_drop_forward_trap_to_cpu(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(4,0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(4,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('f' == TOKEN_CHAR(4,0))
    {
        action = ACTION_FORWARD;
    }
    else if ('c' == TOKEN_CHAR(4,0))
    {
        action = ACTION_COPY2CPU;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_sysLrnOverAct_set(action), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_limit_learning_action_copy_to_cpu_drop_forward_trap_to_cpu */


/*
 * l2-table set limit-learning count <UINT:max_count>
 */
cparser_result_t
cparser_cmd_l2_table_set_limit_learning_count_max_count(
    cparser_context_t *context,
    uint32_t  *max_count_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32 num;

    DIAG_UTIL_PARAM_CHK();

    num = *max_count_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_sysLrnLimitNo_set(num), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_limit_learning_count_max_count */

/*
 * l2-table set limit-learning count unlimited
 */
cparser_result_t
cparser_cmd_l2_table_set_limit_learning_count_unlimited(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 num;

    DIAG_UTIL_PARAM_CHK();

    num = APOLLO_LUT_TBL_NO;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_sysLrnLimitNo_set(num), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_limit_learning_count_unlimited */

/*
 * l2-table set limit-learning port ( <PORT_LIST:port> | all ) ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_l2_table_set_limit_learning_port_port_all_copy_to_cpu_drop_forward_trap_to_cpu(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('d' == TOKEN_CHAR(5,0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('f' == TOKEN_CHAR(5,0))
    {
        action = ACTION_FORWARD;
    }
    else if ('c' == TOKEN_CHAR(5,0))
    {
        action = ACTION_COPY2CPU;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_learnOverAct_set(port,action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_limit_learning_port_port_all_copy_to_cpu_drop_forward_trap_to_cpu */


/*
 * l2-table set limit-learning port ( <PORT_LIST:port> | all ) count <UINT:max_count>
 */
cparser_result_t
cparser_cmd_l2_table_set_limit_learning_port_port_all_count_max_count(
    cparser_context_t *context,
    char * *port_ptr,
    uint32_t  *max_count_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint32 num;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    num = *max_count_ptr;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_lrnLimitNo_set(port, num), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_limit_learning_port_port_all_count_max_count */

/*
 * l2-table set limit-learning port ( <PORT_LIST:port> | all ) count unlimited
 */
cparser_result_t
cparser_cmd_l2_table_set_limit_learning_port_port_all_count_unlimited(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint32 num;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    num = APOLLO_LUT_TBL_NO;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_lrnLimitNo_set(port, num), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_limit_learning_port_port_all_count_unlimited */

/*
 * l2-table set link-down-flush state ( disable | enable )
 */
cparser_result_t
cparser_cmd_l2_table_set_link_down_flush_state_disable_enable(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(4,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(4,0))
    {
        enable = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_flushLinkDownPortAddrEnable_set(enable), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_link_down_flush_state_disable_enable */

/*
 * l2-table set lookup-miss ( bcast | unicast | multicast ) flood-ports ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_l2_table_set_lookup_miss_bcast_unicast_multicast_flood_ports_ports_all(
    cparser_context_t *context,
    char * *flood_port_ptr)
{
    int32                       ret;
    diag_portlist_t             portlist;
    rtk_action_t                action;
    rtk_port_t                  port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    if ('b' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_bcFlood_set(port, ENABLED), ret);
        }

        DIAG_UTIL_PORTMASK_UNSETSCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_bcFlood_set(port, DISABLED), ret);
        }
    }
    else if ('u' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unknUcFlood_set(port, ENABLED), ret);
        }

        DIAG_UTIL_PORTMASK_UNSETSCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unknUcFlood_set(port, DISABLED), ret);
        }
    }
    else if ('m' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unknMcFlood_set(port, ENABLED), ret);
        }

        DIAG_UTIL_PORTMASK_UNSETSCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unknMcFlood_set(port, DISABLED), ret);
        }
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_lookup_miss_bcast_unicast_multicast_flood_ports_ports_all */

/*
 * l2-table set lookup-miss multicast trap-priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_l2_table_set_lookup_miss_multicast_trap_priority_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    int32           ret;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(apollo_raw_unkn_mcPri_set(*priority_ptr), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_lookup_miss_multicast_trap_priority_priority */

/*
 * l2-table set lookup-miss port ( <PORT_LIST:ports> | all ) ( ipmc | ip6mc ) ( drop | flood-in-vlan | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_l2_table_set_lookup_miss_port_ports_all_ipmc_ip6mc_drop_flood_in_vlan_trap_to_cpu(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret;
    diag_portlist_t portlist;
    rtk_action_t action;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('d' == TOKEN_CHAR(6,0))
        action = ACTION_DROP;
    else if ('f' == TOKEN_CHAR(6,0))
        action = ACTION_FORWARD;
    else if ('t' == TOKEN_CHAR(6,0))
        action = ACTION_TRAP2CPU;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    if ('m' == TOKEN_CHAR(5,2))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_unkn_ip4Mc_set(port, action), ret);
        }
    }
    else if ('6' == TOKEN_CHAR(5,2))
    {
        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_unkn_ip6Mc_set(port, action), ret);
        }
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_lookup_miss_port_ports_all_ipmc_ip6mc_drop_flood_in_vlan_trap_to_cpu */

/*
 * l2-table set lookup-miss port ( <PORT_LIST:ports> | all ) l2mc ( drop | drop-exclude-rma | flood-in-vlan | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_l2_table_set_lookup_miss_port_ports_all_l2mc_drop_drop_exclude_rma_flood_in_vlan_trap_to_cpu(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32                       ret;
    diag_portlist_t             portlist;
    rtk_action_t                action;
    rtk_port_t                  port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('f' == TOKEN_CHAR(6,0))
        action = ACTION_FORWARD;
    else if ('t' == TOKEN_CHAR(6,0))
        action = ACTION_TRAP2CPU;
    else if ('d' == TOKEN_CHAR(6,0))
    {
        if ('-' == TOKEN_CHAR(6,4))
            action = ACTION_DROP_EXCLUDE_RMA;
        else
            action = ACTION_DROP;
    }
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_unkn_l2Mc_set(port, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_lookup_miss_port_ports_all_l2mc_drop_drop_exclude_rma_flood_in_vlan_trap_to_cpu */

/*
 * l2-table set lookup-miss port ( <PORT_LIST:ports> | all ) unicast ( drop | flood-in-vlan | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_l2_table_set_lookup_miss_port_ports_all_unicast_drop_flood_in_vlan_trap_to_cpu(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32                       ret;
    diag_portlist_t             portlist;
    rtk_action_t                action;
    rtk_port_t                  port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('f' == TOKEN_CHAR(6,0))
        action = ACTION_FORWARD;
    else if ('t' == TOKEN_CHAR(6,0))
        action = ACTION_TRAP2CPU;
    else if ('d' == TOKEN_CHAR(6,0))
        action = ACTION_DROP;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_unkn_ucDaCtl_set(port, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_lookup_miss_port_ports_all_unicast_drop_flood_in_vlan_trap_to_cpu */

/*
 * l2-table set ( port-move | unknown-sa ) port ( <PORT_LIST:port> | all ) ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_l2_table_set_port_move_unknown_sa_port_port_all_copy_to_cpu_drop_forward_trap_to_cpu(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('d' == TOKEN_CHAR(5,0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('f' == TOKEN_CHAR(5,0))
    {
        action = ACTION_FORWARD;
    }
    else if ('c' == TOKEN_CHAR(5,0))
    {
        action = ACTION_COPY2CPU;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if ('u' == TOKEN_CHAR(2,0))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unkn_saCtl_set(port, action), ret);
        }
        else if ('p' == TOKEN_CHAR(2,0))
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_l2_unMatched_saCtl_set(port, action), ret);
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_port_move_unknown_sa_port_port_all_copy_to_cpu_drop_forward_trap_to_cpu */

/*
 * l2-table set src-mac port ( <PORT_LIST:port> | all ) asic-learn ( copy-to-cpu | drop | forward | trap-to-cpu )
 */
cparser_result_t
cparser_cmd_l2_table_set_src_mac_port_port_all_asic_learn_copy_to_cpu_drop_forward_trap_to_cpu(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_action_t action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('d' == TOKEN_CHAR(6,0))
    {
        action = ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(6,0))
    {
        action = ACTION_TRAP2CPU;
    }
    else if ('f' == TOKEN_CHAR(6,0))
    {
        action = ACTION_FORWARD;
    }
    else if ('c' == TOKEN_CHAR(6,0))
    {
        action = ACTION_COPY2CPU;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_learnOverAct_set(port,action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_src_mac_port_port_all_asic_learn_copy_to_cpu_drop_forward_trap_to_cpu */

#if defined(CMD_L2_TABLE_SET_VLAN_LEARNING_METHOD_IVL_VID_VID)
/*
 * l2-table set vlan-learning-method ivl vid <UINT:vid>
 */
cparser_result_t
cparser_cmd_l2_table_set_vlan_learning_method_ivl_vid_vid(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_vlan_learning_method_ivl_vid_vid */
#endif

#if defined(CMD_L2_TABLE_SET_VLAN_LEARNING_METHOD_SVL_VID_VID_FILTER_ID_FID_MSTI)
/*
 * l2-table set vlan-learning-method svl vid <UINT:vid> filter-id <UINT:fid_msti>
 */
cparser_result_t
cparser_cmd_l2_table_set_vlan_learning_method_svl_vid_vid_filter_id_fid_msti(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *fid_msti_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_vlan_learning_method_svl_vid_vid_filter_id_fid_msti */
#endif

/*
 * l2-table set ipmc-data port ( <PORT_LIST:port> | all ) ( forward | drop )
 */
cparser_result_t
cparser_cmd_l2_table_set_ipmc_data_port_port_all_forward_drop(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32                       ret;
    diag_portlist_t             portlist;
    rtk_port_t                  port = 0;
    rtk_action_t                action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if ('f' == TOKEN_CHAR(5,0))
        action = ACTION_FORWARD;
    else if ('d' == TOKEN_CHAR(5,0))
        action = ACTION_DROP;
    else
        return CPARSER_ERR_INVALID_PARAMS;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_ipmcAction_set(port, action), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_set_ipmc_data_port_port_all_forward_drop */

/*
 * l2-table get ipmc-data port ( <PORT_LIST:port> | all )
 */
cparser_result_t
cparser_cmd_l2_table_get_ipmc_data_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32                       ret;
    diag_portlist_t             portlist;
    rtk_port_t                  port = 0;
    rtk_action_t                action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Action\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_l2_ipmcAction_get(port, &action), ret);
        diag_util_mprintf("%-4d %s\n", port, (action == ACTION_FORWARD) ? DIAG_STR_FORWARD : DIAG_STR_DROP);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_ipmc_data_port_port_all */

/*
 * l2-table get entry address <UINT:address>
 */
cparser_result_t
cparser_cmd_l2_table_get_entry_address_address(
    cparser_context_t *context,
    uint32_t  *address_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_ADDRESS;
    diag_lut.address = *address_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_get(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_entry_address_address */

/*
 * l2-table get entry next-mode address <UINT:address>
 */
cparser_result_t
cparser_cmd_l2_table_get_entry_next_mode_address_address(
    cparser_context_t *context,
    uint32_t  *address_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_NEXT_ADDRESS;
    diag_lut.address = *address_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_get(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_entry_next_mode_address_address */

/*
 * l2-table get entry next-l2-uc address <UINT:address>
 */
cparser_result_t
cparser_cmd_l2_table_get_entry_next_l2_uc_address_address(
    cparser_context_t *context,
    uint32_t  *address_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_NEXT_L2UC;
    diag_lut.address = *address_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_get(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_entry_next_l2_uc_address_address */

/*
 * l2-table get entry next-l2-spa address <UINT:address> spa <UINT:spa>
 */
cparser_result_t
cparser_cmd_l2_table_get_entry_next_l2_spa_address_address_spa_spa(
    cparser_context_t *context,
    uint32_t  *address_ptr,
    uint32_t  *spa_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_NEXT_L2UCSPA;
    diag_lut.address = *address_ptr;
    diag_lut.spa = *spa_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_get(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_entry_next_l2_spa_address_address */

/*
 * l2-table get entry next-l2-mc address <UINT:address>
 */
cparser_result_t
cparser_cmd_l2_table_get_entry_next_l2_mc_address_address(
    cparser_context_t *context,
    uint32_t  *address_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_NEXT_L2MC;
    diag_lut.address = *address_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_get(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_entry_next_l2_mc_address_address */

/*
 * l2-table get entry next-l3-mc address <UINT:address>
 */
cparser_result_t
cparser_cmd_l2_table_get_entry_next_l3_mc_address_address(
    cparser_context_t *context,
    uint32_t  *address_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_NEXT_L3MC;
    diag_lut.address = *address_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_get(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_entry_next_l3_mc_address_address */

/*
 * l2-table get entry next-mc address <UINT:address>
 */
cparser_result_t
cparser_cmd_l2_table_get_entry_next_mc_address_address(
    cparser_context_t *context,
    uint32_t  *address_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    osal_memset(&diag_lut, 0x0, sizeof(apollo_lut_table_t));

    diag_lut.method = RAW_LUT_READ_METHOD_NEXT_L2L3MC;
    diag_lut.address = *address_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_l2_lookUpTb_get(&diag_lut), ret);

    _diag_lutDisplay(&diag_lut);

    return CPARSER_OK;
}    /* end of cparser_cmd_l2_table_get_entry_next_mc_address_address */





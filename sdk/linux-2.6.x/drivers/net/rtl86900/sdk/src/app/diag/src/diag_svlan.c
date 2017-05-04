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
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Definition those SVLAN command and APIs in the SDK diagnostic shell.
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
#include <parser/cparser_priv.h>
#include <dal/apollo/raw/apollo_raw_svlan.h>
#include <diag_str.h>


void _diag_svlan_entry_display(apollo_raw_svlan_mbrCfg_t *pMbrCfg)
{
    diag_util_printf("%-6d", pMbrCfg->idx);
    diag_util_printf("%-5d", pMbrCfg->svid);

    diag_util_printf("%-8s", diag_util_mask32tostr(pMbrCfg->mbr.bits[0]));

    diag_util_printf("%-8s", diag_util_mask32tostr((~pMbrCfg->untagset.bits[0])&APOLLO_PORTMASK));

    diag_util_printf("%-5d", pMbrCfg->spri);

    diag_util_printf("%-8s", diagStr_enable[pMbrCfg->fid_en]);
    diag_util_printf("%-4d",pMbrCfg->fid_msti);  

    diag_util_printf("%-8s", diagStr_enable[pMbrCfg->efid_en]);
    diag_util_mprintf("%d\n",pMbrCfg->efid);  

}

void _diag_svlan_c2s_display(apollo_raw_svlan_c2sCfg_t *pC2sCfg)
{
    diag_util_printf("%-6d", pC2sCfg->idx);
    diag_util_printf("%-6d", pC2sCfg->svidx);
    diag_util_printf("%-5d", pC2sCfg->evid);
    diag_util_mprintf("%s\n", diag_util_mask32tostr(pC2sCfg->pmsk.bits[0]));
}

void _diag_svlan_mc2s_display(apollo_raw_svlan_mc2sCfg_t *pMc2sCfg)
{
    uint8  macStr[19];
    rtk_mac_t mac;
    
    diag_util_printf("%-6d", pMc2sCfg->idx);
    diag_util_printf("%-6d", pMc2sCfg->svidx);
    diag_util_printf("%-10s", diagStr_svlanFmtStr[pMc2sCfg->format]);

    if(RAW_SVLAN_MC2S_FMT_MAC == pMc2sCfg->format)
    {
        mac.octet[0] = 0;
        mac.octet[1] = 0;
        mac.octet[2] = (pMc2sCfg->data>>24)&0xFF;
        mac.octet[3] = (pMc2sCfg->data>>16)&0xFF;
        mac.octet[4] = (pMc2sCfg->data>>8)&0xFF;
        mac.octet[5] = pMc2sCfg->data&0xFF;
        
        diag_util_mac2str(macStr,mac.octet);
        macStr[0] = 'x';
        macStr[1] = 'x';
        macStr[3] = 'x';
        macStr[4] = 'x';        
        diag_util_printf("%-18s", macStr);

        mac.octet[2] = (pMc2sCfg->mask>>24)&0xFF;
        mac.octet[3] = (pMc2sCfg->mask>>16)&0xFF;
        mac.octet[4] = (pMc2sCfg->mask>>8)&0xFF;
        mac.octet[5] = pMc2sCfg->mask&0xFF;
        diag_util_mac2str(macStr,mac.octet);
        macStr[0] = 'x';
        macStr[1] = 'x';
        macStr[3] = 'x';
        macStr[4] = 'x';        
        diag_util_printf("%-18s", macStr);
    }
    else
    {
        diag_util_printf("%-18s", diag_util_inet_ntoa(pMc2sCfg->data));
        diag_util_printf("%-18s", diag_util_inet_ntoa(pMc2sCfg->mask));
    }
    
    diag_util_mprintf("%s\n", diagStr_enable[pMc2sCfg->valid]);
}
#if defined(CMD_SVLAN_CREATE_SVID_SVID)
/*
 * svlan create svid <UINT:svid>
 */
cparser_result_t
cparser_cmd_svlan_create_svid_svid(
    cparser_context_t *context,
    uint32_t  *svid_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    diag_util_mprintf("not implement");

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_create_svid_svid */
#endif
/*
 * svlan destroy all
 */
cparser_result_t
cparser_cmd_svlan_destroy_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32_t vidx;
    apollo_raw_svlan_mbrCfg_t mbrCfgW;
    
    DIAG_UTIL_PARAM_CHK();

    osal_memset(&mbrCfgW, 0x0, sizeof(apollo_raw_svlan_mbrCfg_t));
    
    for(mbrCfgW.idx = 0; mbrCfgW.idx <= APOLLO_SVIDXMAX; mbrCfgW.idx++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_set(&mbrCfgW), ret);   
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_destroy_all */

/*
 * svlan destroy svid <UINT:svid>
 */
cparser_result_t
cparser_cmd_svlan_destroy_svid_svid(
    cparser_context_t *context,
    uint32_t  *svid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_svlan_mbrCfg_t mbrCfg, mbrCfgW;
    
    DIAG_UTIL_PARAM_CHK();

    osal_memset(&mbrCfgW, 0x0, sizeof(apollo_raw_svlan_mbrCfg_t));
    
    for(mbrCfg.idx = 0; mbrCfg.idx <= APOLLO_SVIDXMAX; mbrCfg.idx++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_get(&mbrCfg), ret);   

        if(mbrCfg.svid == *svid_ptr)
        {
            mbrCfgW.idx = mbrCfg.idx;
            DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_set(&mbrCfgW), ret);   
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_destroy_svid_svid */

/*
 * svlan get c2s entry <UINT:index> 
 */
cparser_result_t
cparser_cmd_svlan_get_c2s_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_svlan_c2sCfg_t c2sCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    c2sCfg.idx = *index_ptr;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_c2sCfg_get(&c2sCfg), ret);

    diag_util_mprintf("Index Svidx Evid Port\n");
    _diag_svlan_c2s_display(&c2sCfg);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_c2s_entry_index */

/*
 * svlan get c2s entry all
 */
cparser_result_t
cparser_cmd_svlan_get_c2s_entry_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_svlan_c2sCfg_t c2sCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    diag_util_mprintf("Index Svidx Evid Port\n");
    for(c2sCfg.idx = 0; c2sCfg.idx <= APOLLO_SVLAN_C2S_INDEX_MAX; c2sCfg.idx++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_c2sCfg_get(&c2sCfg), ret);

        _diag_svlan_c2s_display(&c2sCfg);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_c2s_entry_all */

/*
 * svlan get entry <UINT:index>
 */
cparser_result_t
cparser_cmd_svlan_get_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;

    apollo_raw_svlan_mbrCfg_t mbrCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    mbrCfg.idx = *index_ptr;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_get(&mbrCfg), ret);

    diag_util_mprintf("Index SVID Member  TagSet  Spri FidEn   FID EfidEn  Efid\n");
    _diag_svlan_entry_display(&mbrCfg);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_entry_index */

/*
 * svlan get entry all
 */
cparser_result_t
cparser_cmd_svlan_get_entry_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_svlan_mbrCfg_t mbrCfg;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Index SVID Member  TagSet  Spri FidEn   FID EfidEn  Efid\n");

    for(mbrCfg.idx=0;mbrCfg.idx<=APOLLO_SVIDXMAX;mbrCfg.idx++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_get(&mbrCfg), ret);
         _diag_svlan_entry_display(&mbrCfg);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_entry_all */


/*
 * svlan get mc2s entry <UINT:index>
 */
cparser_result_t
cparser_cmd_svlan_get_mc2s_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_svlan_mc2sCfg_t mc2sCfg;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    mc2sCfg.idx = *index_ptr;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mc2sCfg_get(&mc2sCfg), ret);

    diag_util_mprintf("Index SVIDX Format    Data              Mask              Valid\n");
    _diag_svlan_mc2s_display(&mc2sCfg);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_mc2s_entry_index */

/*
 * svlan get mc2s entry all
 */
cparser_result_t
cparser_cmd_svlan_get_mc2s_entry_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;

    apollo_raw_svlan_mc2sCfg_t mc2sCfg;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
        
    diag_util_mprintf("Index SVIDX Format    Data              Mask              Valid\n");
    for(mc2sCfg.idx = 0; mc2sCfg.idx <= APOLLO_SVLAN_MC2S_INDEX_MAX; mc2sCfg.idx ++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mc2sCfg_get(&mc2sCfg), ret);
        _diag_svlan_mc2s_display(&mc2sCfg);

    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_mc2s_entry_all */


/*
 * svlan get port ( <PORT_LIST:portlist> | all ) svid
 */
cparser_result_t
cparser_cmd_svlan_get_port_portlist_all_svid(
    cparser_context_t *context,
    char * *portlist_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_svidx_t svidx;
    apollo_raw_svlan_mbrCfg_t mbrCfg;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    diag_util_mprintf("Port SVID\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_portSvlan_get(port, &svidx), ret);

        mbrCfg.idx = svidx;
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_get(&mbrCfg), ret);
        
        diag_util_mprintf("%-5d%d\n", port, mbrCfg.svid);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_port_portlist_all_svid */
/*
 * svlan get port ( <PORT_LIST:portlist> | all ) svidx
 */
cparser_result_t
cparser_cmd_svlan_get_port_portlist_all_svidx(
    cparser_context_t *context,
    char * *portlist_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_svidx_t svidx;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);


    diag_util_mprintf("Port SVIDX\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_portSvlan_get(port, &svidx), ret);

        diag_util_mprintf("%-5d%d\n",port, svidx);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_port_portlist_all_svidx */

/*
 * svlan get priority-source 
 */
cparser_result_t
cparser_cmd_svlan_get_priority_source(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_svlan_priSel_t mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_egrPriSel_get(&mode), ret);

    diag_util_mprintf("Priority source: %s\n", diagStr_svlanSpriSrc[mode]);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_priority_source */

/*
 * svlan get service-port
 */
cparser_result_t
cparser_cmd_svlan_get_service_port(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_port_t port = 0;
    rtk_enable_t enable;
    rtk_portmask_t lPortMask;
    uint8  port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    lPortMask.bits[0] = 0;
    for(port = 0; port <= APOLLO_PORTIDMAX; port ++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_portUplinkEnable_get(port, &enable), ret);
        if( ENABLED == enable)
            lPortMask.bits[0] |= (1 << port);
    }

    diag_util_lPortMask2str(port_list, &lPortMask);
    diag_util_mprintf("Server Ports: %s\n",port_list);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_service_port */

/*
 * svlan get sp2c entry <UINT:index> 
 */
cparser_result_t
cparser_cmd_svlan_get_sp2c_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint8  port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_portmask_t lPortMask;
    apollo_raw_svlan_sp2cCfg_t sp2cCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    sp2cCfg.idx = *index_ptr;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_sp2cCfg_get(&sp2cCfg), ret);

    diag_util_mprintf("Index VID EgPort Svidx Valid\n");
    diag_util_mprintf("%-6d%-5d%-7d%-6d%s\n", 
                            sp2cCfg.idx,
                            sp2cCfg.vid,
                            sp2cCfg.port,
                            sp2cCfg.svidx,
                            diagStr_enable[sp2cCfg.valid]);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_sp2c_entry_index */

/*
 * svlan get sp2c entry all
 */
cparser_result_t
cparser_cmd_svlan_get_sp2c_entry_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint8  port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_portmask_t lPortMask;
    apollo_raw_svlan_sp2cCfg_t sp2cCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Index VID EgPort Svidx Valid\n");
    for(sp2cCfg.idx = 0; sp2cCfg.idx <= APOLLO_SVLAN_SP2C_INDEX_MAX; sp2cCfg.idx ++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_sp2cCfg_get(&sp2cCfg), ret);

        diag_util_mprintf("%-6d%-5d%-7d%-6d%s\n", 
                                sp2cCfg.idx,
                                sp2cCfg.vid,
                                sp2cCfg.port,
                                sp2cCfg.svidx,
                                diagStr_enable[sp2cCfg.valid]);
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_sp2c_entry_all */


/*
 * svlan get svid <UINT:svid>
 */
cparser_result_t
cparser_cmd_svlan_get_svid_svid(
    cparser_context_t *context,
    uint32_t  *svid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint32_t vidx;
    apollo_raw_svlan_mbrCfg_t mbrCfg;
    rtk_portmask_t lPortMask;
    uint8  port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
  
    diag_util_mprintf("Index SVID Member  TagSet  Spri FidEn   FID EfidEn  Efid\n");
    for(vidx=0;vidx<APOLLO_SVIDXMAX;vidx++)
    {
        mbrCfg.idx = vidx;
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_get(&mbrCfg), ret);   

        if(mbrCfg.svid == *svid_ptr)
        {
            diag_util_printf("%-6d", mbrCfg.idx);
            diag_util_printf("%-5d", mbrCfg.svid);

            diag_util_lPortMask2str(port_list, &mbrCfg.mbr);
            diag_util_printf("%-8s", port_list);

            lPortMask.bits[0] = (~(mbrCfg.untagset.bits[0])) & APOLLO_PORTMASK;
            diag_util_lPortMask2str(port_list, &lPortMask);
            diag_util_printf("%-8s", port_list);

            diag_util_printf("%-5d", mbrCfg.spri);

            diag_util_printf("%-8s", diagStr_enable[mbrCfg.fid_en]);
            diag_util_printf("%-4d",mbrCfg.fid_msti);  

            diag_util_printf("%-8s", diagStr_enable[mbrCfg.efid_en]);
            diag_util_mprintf("%d\n",mbrCfg.efid);  
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_svid_svid */

/*
 * svlan get tpid
 */
cparser_result_t
cparser_cmd_svlan_get_tpid(
    cparser_context_t *context)
{
    apollo_raw_ethertype_t  svlan_tag_id = 0;
    int32                   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    
    if ((ret = apollo_raw_svlan_tpid_get(&svlan_tag_id)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("TPID: 0x%4.4x\n", svlan_tag_id);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_tpid */

/*
 * svlan get trap-priority
 */
cparser_result_t
cparser_cmd_svlan_get_trap_priority(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_pri_t priority;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    if ((ret = apollo_raw_svlan_trapPri_get(&priority)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    
    diag_util_mprintf("Trap priority: %d\n", priority);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_trap_priority */

/*
 * svlan get unmatch
 */
cparser_result_t
cparser_cmd_svlan_get_unmatch(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_svlan_action_t action = SVLAN_ACTION_END;
    apollo_raw_svidx_t svidx;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();


    if ((ret = apollo_raw_svlan_unmatchAction_get(&action)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if ((ret = apollo_raw_svlan_unmatchSvidx_get(&svidx)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Action: %s\n", diagStr_svlanAct[action]);
    if(action >= SVLAN_ACTION_SVLAN)
        diag_util_mprintf("Svidx: %d\n", svidx);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_unmatch */

/*
 * svlan get untag
 */
cparser_result_t
cparser_cmd_svlan_get_untag(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_svlan_action_t action = SVLAN_ACTION_END;
    apollo_raw_svidx_t svidx;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();


    if ((ret = apollo_raw_svlan_untagAction_get(&action)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if ((ret = apollo_raw_svlan_untagSvidx_get(&svidx)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Action: %s\n", diagStr_svlanAct[action]);
    if(action >= SVLAN_ACTION_SVLAN)
        diag_util_mprintf("Svidx: %d\n", svidx);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_untag */

/*
 * svlan get vlan-aggregation port ( <PORT_LIST:portlist> | all ) state 
 */
cparser_result_t
cparser_cmd_svlan_get_vlan_aggregation_port_portlist_all_state(
    cparser_context_t *context,
    char * *portlist_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Status\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_port1tonVlanEnable_get(port, &enable), ret);
        diag_util_mprintf("%-5d%s\n", port, diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_vlan_aggregation_port_portlist_all_state */

/*
 * svlan set c2s entry <UINT:index> evid <UINT:evid> member ( <PORT_LIST:portlist> | all | none ) svidx <UINT:svidx>
 */
cparser_result_t
cparser_cmd_svlan_set_c2s_entry_index_evid_evid_member_portlist_all_none_svidx_svidx(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *evid_ptr,
    char * *portlist_ptr,
    uint32_t  *svidx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    uint8  port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    diag_portlist_t portlist;

    apollo_raw_svlan_c2sCfg_t c2sCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);
    
    c2sCfg.idx = *index_ptr;
    c2sCfg.evid =  *evid_ptr;
    c2sCfg.svidx = *svidx_ptr;
    c2sCfg.pmsk.bits[0] = portlist.portmask.bits[0];
    
    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_c2sCfg_set(&c2sCfg), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_c2s_entry_index_evid_evid_member_portlist_all_none_svidx_svidx */

/*
 * svlan set entry <UINT:index> svid <UINT:svid> member ( <PORT_LIST:portlist> | all | none ) 
 */
cparser_result_t
cparser_cmd_svlan_set_entry_index_svid_svid_member_portlist_all_none(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *svid_ptr,
    char * *portlist_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_svlan_mbrCfg_t mbrCfg;

    osal_memset(&mbrCfg, 0, sizeof(apollo_raw_svlan_mbrCfg_t));

    DIAG_UTIL_PARAM_CHK();

    mbrCfg.idx = *index_ptr;
    mbrCfg.svid = *svid_ptr;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 7), ret);
    mbrCfg.mbr.bits[0] = portlist.portmask.bits[0];

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_set(&mbrCfg), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_entry_index_svid_svid_member_portlist_all_none */

/*
 * svlan set entry <UINT:index> svid <UINT:svid> member ( <PORT_LIST:portlist> | all | none ) tag-mbr ( <PORT_LIST:tagged_ports> | all | none )
 */
cparser_result_t
cparser_cmd_svlan_set_entry_index_svid_svid_member_portlist_all_none_tag_mbr_tagged_ports_all_none(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *svid_ptr,
    char * *portlist_ptr,
    char * *tagged_ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_svlan_mbrCfg_t mbrCfg;

    osal_memset(&mbrCfg, 0, sizeof(apollo_raw_svlan_mbrCfg_t));

    DIAG_UTIL_PARAM_CHK();

    mbrCfg.idx = *index_ptr;
    mbrCfg.svid = *svid_ptr;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 7), ret);
    mbrCfg.mbr.bits[0] = portlist.portmask.bits[0];

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 9), ret);
    mbrCfg.untagset.bits[0] = (~portlist.portmask.bits[0]) & APOLLO_PORTMASK;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_set(&mbrCfg), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_entry_index_svid_svid_member_portlist_all_none_tag_mbr_tagged_ports_all_none */

/*
 * svlan set entry <UINT:index> svid <UINT:svid> member ( <PORT_LIST:portlist> | all | none ) tag-mbr ( <PORT_LIST:tagged_ports> | all | none ) s-pri <UINT:spriority> fid-state ( disable | enable ) fid <UINT:fid> efid-state ( disable | enable ) efid <UINT:efid>
 */
cparser_result_t
cparser_cmd_svlan_set_entry_index_svid_svid_member_portlist_all_none_tag_mbr_tagged_ports_all_none_s_pri_spriority_fid_state_disable_enable_fid_fid_efid_state_disable_enable_efid_efid(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *svid_ptr,
    char * *portlist_ptr,
    char * *tagged_ports_ptr,
    uint32_t  *spriority_ptr,
    uint32_t  *fid_ptr,
    uint32_t  *efid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_svlan_mbrCfg_t mbrCfg;

    osal_memset(&mbrCfg, 0, sizeof(apollo_raw_svlan_mbrCfg_t));

    DIAG_UTIL_PARAM_CHK();

    mbrCfg.idx = *index_ptr;
    mbrCfg.svid = *svid_ptr;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 7), ret);
    mbrCfg.mbr.bits[0] = portlist.portmask.bits[0];

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 9), ret);
    mbrCfg.untagset.bits[0]= (~portlist.portmask.bits[0]) & APOLLO_PORTMASK;
    
    mbrCfg.spri = *spriority_ptr;
    mbrCfg.efid = *efid_ptr;
    mbrCfg.fid_msti = *fid_ptr;

    if ('d' == TOKEN_CHAR(13,0))
    {
        mbrCfg.fid_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(13,0))
    {
        mbrCfg.fid_en = ENABLED;
    }

    if ('d' == TOKEN_CHAR(17,0))
    {
        mbrCfg.efid_en = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(17,0))
    {
        mbrCfg.efid_en = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_set(&mbrCfg), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_entry_index_svid_svid_member_portlist_all_none_tag_mbr_tagged_ports_all_none_s_pri_spriority_fid_state_disable_enable_fid_fid_efid_state_disable_enable_efid_efid */


/*
 * svlan set mc2s entry <UINT:index> state invalid
 */
cparser_result_t
cparser_cmd_svlan_set_mc2s_entry_index_state_invalid(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{

    int32     ret = RT_ERR_FAILED;
    apollo_raw_svlan_mc2sCfg_t mc2sCfg;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&mc2sCfg, 0x00, sizeof(apollo_raw_svlan_mc2sCfg_t));

    mc2sCfg.idx = *index_ptr;
    mc2sCfg.valid = DISABLED;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mc2sCfg_set(&mc2sCfg), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_mc2s_entry_index_state_invalid */

/*
 * svlan set mc2s entry <UINT:index> state valid format dip ip_addr <IPV4ADDR:ip_addr> ip_mask <IPV4ADDR:ip_mask> svidx <UINT:svidx>
 */
cparser_result_t
cparser_cmd_svlan_set_mc2s_entry_index_state_valid_format_dip_ip_addr_ip_addr_ip_mask_ip_mask_svidx_svidx(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *ip_addr_ptr,
    uint32_t  *ip_mask_ptr,
    uint32_t  *svidx_ptr)
{
    int32     ret = RT_ERR_FAILED;
    apollo_raw_svlan_mc2sCfg_t mc2sCfg;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == ip_addr_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == ip_mask_ptr), CPARSER_ERR_INVALID_PARAMS);

    mc2sCfg.idx = *index_ptr;
    mc2sCfg.valid = ENABLED;
    mc2sCfg.format = RAW_SVLAN_MC2S_FMT_IP;
    mc2sCfg.svidx = *svidx_ptr;

    mc2sCfg.data = *ip_addr_ptr;
    mc2sCfg.mask = *ip_mask_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mc2sCfg_set(&mc2sCfg), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_mc2s_entry_index_state_valid_format_dip_ip_addr_ip_addr_ip_mask_ip_mask_svidx_svidx */

/*
 * svlan set mc2s entry <UINT:index> state valid format dmac mac_addr <MACADDR:mac> mac_mask <MACADDR:mac_mask> svidx <UINT:svidx>
 */
cparser_result_t
cparser_cmd_svlan_set_mc2s_entry_index_state_valid_format_dmac_mac_addr_mac_mac_mask_mac_mask_svidx_svidx(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    cparser_macaddr_t  *mac_ptr,
    cparser_macaddr_t  *mac_mask_ptr,
    uint32_t  *svidx_ptr)
{
    int32     ret = RT_ERR_FAILED;
    apollo_raw_svlan_mc2sCfg_t mc2sCfg;

    DIAG_UTIL_PARAM_CHK();
    RT_PARAM_CHK((NULL == mac_ptr), CPARSER_ERR_INVALID_PARAMS);
    RT_PARAM_CHK((NULL == mac_mask_ptr), CPARSER_ERR_INVALID_PARAMS);

    mc2sCfg.idx = *index_ptr;
    mc2sCfg.valid = ENABLED;
    mc2sCfg.format = RAW_SVLAN_MC2S_FMT_MAC;
    mc2sCfg.svidx = *svidx_ptr;
    mc2sCfg.data = mac_ptr->octet[2];
    mc2sCfg.data = (mc2sCfg.data << 8) | mac_ptr->octet[3];
    mc2sCfg.data = (mc2sCfg.data << 8) | mac_ptr->octet[4];
    mc2sCfg.data = (mc2sCfg.data << 8) | mac_ptr->octet[5];

    mc2sCfg.mask = mac_mask_ptr->octet[2];
    mc2sCfg.mask = (mc2sCfg.mask << 8) | mac_mask_ptr->octet[3];
    mc2sCfg.mask = (mc2sCfg.mask << 8) | mac_mask_ptr->octet[4];
    mc2sCfg.mask = (mc2sCfg.mask << 8) | mac_mask_ptr->octet[5];

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mc2sCfg_set(&mc2sCfg), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_mc2s_entry_index_state_valid_format_dmac_mac_addr_mac_mac_mask_mac_mask_svidx_svidx */

/*
 * svlan set port ( <PORT_LIST:portlist> | all ) svid <UINT:svid>
 */
cparser_result_t
cparser_cmd_svlan_set_port_portlist_all_svid_svid(
    cparser_context_t *context,
    char * *portlist_ptr,
    uint32_t  *svid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_svidx_t svidx;
    apollo_raw_svlan_mbrCfg_t mbrCfg;
    uint32 svidx_ok = FALSE;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    for(mbrCfg.idx=0;mbrCfg.idx<APOLLO_SVIDXMAX;mbrCfg.idx++)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_get(&mbrCfg), ret);
        if(mbrCfg.svid == *svid_ptr)
        {
            svidx_ok = TRUE;   
        }
    }   

    /*check empty svlan member configuration*/
    if(svidx_ok == FALSE)
    {
        for(mbrCfg.idx=0;mbrCfg.idx<APOLLO_SVIDXMAX;mbrCfg.idx++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_get(&mbrCfg), ret);

            if(mbrCfg.svid == 0 && mbrCfg.mbr.bits[0] == 0 && mbrCfg.untagset.bits[0] == 0)   
            {
                svidx_ok =  TRUE;
                /*write PSVID to SVIDX*/
                mbrCfg.svid = *svid_ptr;
                DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_set(&mbrCfg), ret);
                break;
            }
        }
    }


    if(svidx_ok == TRUE)
    {
        svidx = mbrCfg.idx;

        DIAG_UTIL_PORTMASK_SCAN(portlist, port)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_svlan_portSvlan_set(port, svidx), ret);
        }
    }
    else
        return RT_ERR_FAILED;
    
    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_port_portlist_all_svid_svid */

/*
 * svlan set port ( <PORT_LIST:portlist> | all ) svidx <UINT:svidx>
 */
cparser_result_t
cparser_cmd_svlan_set_port_portlist_all_svidx_svidx(
    cparser_context_t *context,
    char **portlist_ptr,
    uint32_t *svidx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    apollo_raw_svidx_t svidx;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    svidx = *svidx_ptr;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_portSvlan_set(port, svidx), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_port_portlist_all_svidx_svidx */

/*
 * svlan set priority-source ( internal-priority | dot1q-priority | svlan_mbr_cfg | pb-priority ) 
 */
cparser_result_t
cparser_cmd_svlan_set_priority_source_internal_priority_dot1q_priority_svlan_mbr_cfg_pb_priority(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_svlan_priSel_t mode;

    DIAG_UTIL_PARAM_CHK();

    if ('i' == TOKEN_CHAR(3,0))
    {
        mode = SVLAN_PRISEL_INTERNAL_PRI;
    }
    else if ('d' == TOKEN_CHAR(3,0))
    {
        mode = SVLAN_PRISEL_1QTAG_PRI;
    }
    else if ('s' == TOKEN_CHAR(3,0))
    {
        mode = SVLAN_PRISEL_VSPRI;
    }
    else if ('p' == TOKEN_CHAR(3,0))
    {
        mode = SVLAN_PRISEL_PBPRI;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_egrPriSel_set(mode), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_priority_source_internal_priority_dot1q_priority_svlan_mbr_cfg_pb_priority */

/*
 * svlan set service-port ( <PORT_LIST:portlist> | all | none )
 */
cparser_result_t
cparser_cmd_svlan_set_service_port_portlist_all_none(
    cparser_context_t *context,
    char * *portlist_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_portUplinkEnable_set(port, ENABLED), ret);
    }

    DIAG_UTIL_PORTMASK_UNSETSCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_portUplinkEnable_set(port, DISABLED), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_service_port_portlist_all_none */

/*
 * svlan set sp2c entry <UINT:index> state invalid
 */
cparser_result_t
cparser_cmd_svlan_set_sp2c_entry_index_state_invalid(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    int32     ret = RT_ERR_FAILED;
    apollo_raw_svlan_sp2cCfg_t sp2cCfg;
    
    DIAG_UTIL_PARAM_CHK();

    osal_memset(&sp2cCfg, 0x00, sizeof(apollo_raw_svlan_sp2cCfg_t));

    sp2cCfg.idx = *index_ptr;
    sp2cCfg.valid = DISABLED;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_sp2cCfg_set(&sp2cCfg), ret);


    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_sp2c_entry_index_state_invalid */

/*
 * svlan set sp2c entry <UINT:index> state valid svidx <UINT:svidx> egress_port <UINT:port> cvid <UINT:vid>
 */
cparser_result_t
cparser_cmd_svlan_set_sp2c_entry_index_state_valid_svidx_svidx_egress_port_port_cvid_vid(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *svidx_ptr,
    uint32_t  *port_ptr,
    uint32_t  *vid_ptr)
{

    DIAG_UTIL_PARAM_CHK();

    int32     ret = RT_ERR_FAILED;
    apollo_raw_svlan_sp2cCfg_t sp2cCfg;
    
    DIAG_UTIL_PARAM_CHK();

    osal_memset(&sp2cCfg, 0x00, sizeof(apollo_raw_svlan_sp2cCfg_t));

    sp2cCfg.idx = *index_ptr;
    sp2cCfg.valid = ENABLED;
    sp2cCfg.svidx = *svidx_ptr;
    sp2cCfg.port = *port_ptr;
    sp2cCfg.vid = *vid_ptr;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_sp2cCfg_set(&sp2cCfg), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_sp2c_entry_index_state_valid_svidx_svidx_egress_port_port_cvid_vid */

/*
 * svlan set svid <UINT:svid> member ( <PORT_LIST:portlist> | all | none )
 */
cparser_result_t
cparser_cmd_svlan_set_svid_svid_member_portlist_all_none(
    cparser_context_t *context,
    uint32_t  *svid_ptr,
    char * *portlist_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint32 svidx_ok = FALSE;
    apollo_raw_svlan_mbrCfg_t mbrCfg;


    DIAG_UTIL_PARAM_CHK();

    for(mbrCfg.idx = 0;mbrCfg.idx < RT_ERR_SVLAN_ENTRY_INDEX;mbrCfg.idx++)
    {
        if(mbrCfg.svid == *svid_ptr)
            break;

    }

    if(svidx_ok == FALSE)
    {
        for(mbrCfg.idx=0;mbrCfg.idx<APOLLO_SVIDXMAX;mbrCfg.idx++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_get(&mbrCfg), ret);

            if(mbrCfg.svid == 0 && mbrCfg.mbr.bits[0] == 0 && mbrCfg.untagset.bits[0] == 0)   
            {
                svidx_ok =  TRUE;
                break;
            }
        }
    }


    if(svidx_ok == TRUE)
    {
        mbrCfg.svid = *svid_ptr;
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
        mbrCfg.mbr.bits[0] = portlist.portmask.bits[0];
        
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_set(&mbrCfg), ret);

    }
    else
        return RT_ERR_FAILED;

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_svid_svid_member_portlist_all_none */

/*
 * svlan set svid <UINT:svid> member ( <PORT_LIST:portlist> | all | none ) tag_mbr ( <PORT_LIST:tagged_ports> | all | none )
 */
cparser_result_t
cparser_cmd_svlan_set_svid_svid_member_portlist_all_none_tag_mbr_tagged_ports_all_none(
    cparser_context_t *context,
    uint32_t  *svid_ptr,
    char * *portlist_ptr,
    char * *tagged_ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    uint32 svidx_ok = FALSE;
    apollo_raw_svlan_mbrCfg_t mbrCfg;


    DIAG_UTIL_PARAM_CHK();

    for(mbrCfg.idx = 0;mbrCfg.idx < RT_ERR_SVLAN_ENTRY_INDEX;mbrCfg.idx++)
    {
        if(mbrCfg.svid == *svid_ptr)
            break;

    }

    if(svidx_ok == FALSE)
    {
        for(mbrCfg.idx=0;mbrCfg.idx<APOLLO_SVIDXMAX;mbrCfg.idx++)
        {
            DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_get(&mbrCfg), ret);

            if(mbrCfg.svid == 0 && mbrCfg.mbr.bits[0] == 0 && mbrCfg.untagset.bits[0] == 0)   
            {
                svidx_ok =  TRUE;
                break;
            }
        }
    }


    if(svidx_ok == TRUE)
    {
        mbrCfg.svid = *svid_ptr;
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);
        mbrCfg.mbr.bits[0] = portlist.portmask.bits[0];
        
        DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 7), ret);
        mbrCfg.untagset.bits[0] = (~(portlist.portmask.bits[0]))& APOLLO_PORTMASK;
        
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_mrbCfg_set(&mbrCfg), ret);

    }
    else
        return RT_ERR_FAILED;

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_svid_svid_member_portlist_all_none_tag_mbr_tagged_ports_all_none */

/*
 * svlan set tpid <UINT:tpid_value>
 */
cparser_result_t
cparser_cmd_svlan_set_tpid_tpid_value(
    cparser_context_t *context,
    uint32_t  *tpid_value_ptr)
{
    apollo_raw_ethertype_t  svlan_tag_id = 0;
    int32                   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    svlan_tag_id = *tpid_value_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_tpid_set(svlan_tag_id), ret);    

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_tpid_tpid_value */

/*
 * svlan set trap-priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_svlan_set_trap_priority_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    int32     ret = RT_ERR_FAILED;
    rtk_pri_t priority;

    DIAG_UTIL_PARAM_CHK();

    priority = *priority_ptr;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_trapPri_set(priority), ret);    

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_trap_priority_priority */

/*
 * svlan set unmatch ( drop | trap-to-cpu ) 
 */
cparser_result_t
cparser_cmd_svlan_set_unmatch_drop_trap_to_cpu(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_svlan_action_t action = SVLAN_ACTION_END;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(3,0))
    {
        action = SVLAN_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(3,0))
    {
        action = SVLAN_ACTION_TRAP;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_unmatchAction_set(action), ret);    

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_unmatch_drop_trap_to_cpu */

/*
 * svlan set unmatch ( svlan-mbr | svlan-mbr-and-keep ) svidx <UINT:svidx> 
 */
cparser_result_t
cparser_cmd_svlan_set_unmatch_svlan_mbr_svlan_mbr_and_keep_svidx_svidx(
    cparser_context_t *context,
    uint32_t  *svidx_ptr)
{
    int32     ret = RT_ERR_FAILED;
    rtk_svlan_action_t action = SVLAN_ACTION_END;
    apollo_raw_svidx_t svidx;

    DIAG_UTIL_PARAM_CHK();

    if ('-' == TOKEN_CHAR(3,9))
    {
        action = SVLAN_ACTION_SVLAN_AND_KEEP;
    }
    else
    {
        action = SVLAN_ACTION_SVLAN;
    }
    svidx = *svidx_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_unmatchAction_set(action), ret);    

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_unmatchSvidx_set(svidx), ret);    

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_unmatch_svlan_mbr_svlan_mbr_and_keep_svidx_svidx */

/*
 * svlan set untag ( drop | trap-to-cpu ) 
 */
cparser_result_t
cparser_cmd_svlan_set_untag_drop_trap_to_cpu(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_svlan_action_t action = SVLAN_ACTION_END;

    DIAG_UTIL_PARAM_CHK();

    if ('d' == TOKEN_CHAR(3,0))
    {
        action = SVLAN_ACTION_DROP;
    }
    else if ('t' == TOKEN_CHAR(3,0))
    {
        action = SVLAN_ACTION_TRAP;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_untagAction_set(action), ret);    

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_untag_drop_trap_to_cpu */

/*
 * svlan set untag svlan-mbr svidx <UINT:svidx>
 */
cparser_result_t
cparser_cmd_svlan_set_untag_svlan_mbr_svidx_svidx(
    cparser_context_t *context,
    uint32_t  *svidx_ptr)
{
    int32     ret = RT_ERR_FAILED;
    rtk_svlan_action_t action = SVLAN_ACTION_END;

    DIAG_UTIL_PARAM_CHK();

    action = SVLAN_ACTION_SVLAN;

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_untagAction_set(action), ret);    

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_untagSvidx_set(*svidx_ptr), ret);    

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_untag_svlan_mbr_svidx_svidx */

/*
 * svlan set vlan-aggregation port ( <PORT_LIST:portlist> | all ) state ( disable | enable ) 
 */
cparser_result_t
cparser_cmd_svlan_set_vlan_aggregation_port_portlist_all_state_disable_enable(
    cparser_context_t *context,
    char * *portlist_ptr)
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
        DIAG_UTIL_ERR_CHK(apollo_raw_svlan_port1tonVlanEnable_set(port, enable), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_vlan_aggregation_port_portlist_all_state_disable_enable */

/*
 * svlan set dei-keep state ( disable | enable ) 
 */
cparser_result_t
cparser_cmd_svlan_set_dei_keep_state_disable_enable(
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

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_cfiKeepEnable_set(enable), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_cfi_keep_state_disable_enable */

/*
 * svlan get dei-keep state 
 */
cparser_result_t
cparser_cmd_svlan_get_dei_keep_state(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_cfiKeepEnable_get(&enable), ret);

    diag_util_mprintf("Keep DEI function: %s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_cfi_keep_state */

/*
 * svlan set lookup-type ( svlan64 | vlan4k ) 
 */
cparser_result_t
cparser_cmd_svlan_set_lookup_type_svlan64_vlan4k(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_svlan_lookupType_t type;
    
    DIAG_UTIL_PARAM_CHK();

    if ('s' == TOKEN_CHAR(3,0))
    {
        type = SVLAN_LOOKUP_S64MBRCGF;
    }
    else if ('v' == TOKEN_CHAR(3,0))
    {
        type = SVLAN_LOOKUP_C4KVLAN;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_lookupType_set(type), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_set_lookup_type_svlan64_vlan4k */

/*
 * svlan get lookup-type 
 */
cparser_result_t
cparser_cmd_svlan_get_lookup_type(
    cparser_context_t *context)
{
    int32     ret = RT_ERR_FAILED;
    rtk_svlan_lookupType_t type;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_svlan_lookupType_get(&type), ret);

    diag_util_mprintf("Lookup type: %s\n", diagStr_svlanLookup[type]);

    return CPARSER_OK;
}    /* end of cparser_cmd_svlan_get_lookup_type */





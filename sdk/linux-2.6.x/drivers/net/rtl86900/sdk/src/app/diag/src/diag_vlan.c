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
 * $Revision: 41742 $
 * $Date: 2013-08-01 21:31:49 +0800 (Thu, 01 Aug 2013) $
 *
 * Purpose : Definition those VLAN command and APIs in the SDK diagnostic shell.
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
#include <dal/apollo/raw/apollo_raw_vlan.h>
#include <diag_str.h>


#define VLAN_MAX_MEMBER_CFG_IDX       (32)
#define VLAN_MAX_PROTO_VLAN_IDX       (4)
#define VLAN_MAX_4K_VLAN_IDX          (4095)

/*
 * vlan destroy all
 */
cparser_result_t
cparser_cmd_vlan_destroy_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;
    uint32 vid;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

#ifdef RTL_RLX_IO
    if((ret = table_clear(VLANt,0,(VLAN_MAX_4K_VLAN_IDX-1))) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#else
    for(vid = 0;vid <VLAN_MAX_4K_VLAN_IDX ;vid++)
    {
        vlanCfg.vid = vid;
        if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_destroy_all */


/*
 * vlan destroy all untag
 */
cparser_result_t
cparser_cmd_vlan_destroy_all_untag(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;
    uint32 vid;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_extract_portlist("0-6", DIAG_PORTTYPE_ALL, &portlist);

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));
    vlanCfg.untag.bits[0] = portlist.portmask.bits[0];

    for(vid = 0;vid <VLAN_MAX_4K_VLAN_IDX ;vid++)
    {
        vlanCfg.vid = vid;
        if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_destroy_all_untag */



/*
 * vlan destroy entry all
 */
cparser_result_t
cparser_cmd_vlan_destroy_entry_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;
    uint32 idx;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));
    for(idx = 0;idx <VLAN_MAX_MEMBER_CFG_IDX -1 ;idx++)
    {
        vlanCfg.index = idx;
        if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_destroy_mbrcfg_all */

/*
 * vlan destroy entry <UINT:index>
 */
cparser_result_t
cparser_cmd_vlan_destroy_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_destroy_mbrcfg_vidx_vidx */

/*
 * vlan destroy vid <UINT:vid>
 */
cparser_result_t
cparser_cmd_vlan_destroy_vid_vid(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;

    if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_destroy_vid_vid */

/*
 * vlan get accept-frame-type port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_vlan_get_accept_frame_type_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanAcpFrameType_t type;
    rtk_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Accept Frame Type\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        /*call API*/
        if((ret = apollo_raw_vlan_acceptFrameType_get(port ,&type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("%-4d ",port);

        switch(type)
        {
            case RAW_VLAN_ACCEPT_ALL:
                diag_util_mprintf("%s\n","all");
                break;
            case RAW_VLAN_ACCEPT_TAGGED_ONLY:
                diag_util_mprintf("%s\n","tagged-only");
                break;
            case RAW_VLAN_ACCEPT_UNTAGGED_ONLY:
                diag_util_mprintf("%s\n","untagged-only");
                break;
            case RAW_VLAN_ACCEPT_1Q_1P_TAGGED_ONLY:
                diag_util_mprintf("%s\n","tagged-1p-1q-only");
                break;
            default:
                diag_util_mprintf("%s\n",DIAG_STR_INVALID);
                break;

        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_accept_frame_type_port_ports_all */

/*
 * vlan get egress-keep port ( <PORT_LIST:egr_ports> | all ) ingress-port ( <PORT_LIST:igr_ports> | all ) state
 */
cparser_result_t
cparser_cmd_vlan_get_egress_keep_port_egr_ports_all_ingress_port_igr_ports_all_state(
    cparser_context_t *context,
    char * *egr_ports_ptr,
    char * *igr_ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_port_t port;
    rtk_port_t igrPort;
    diag_portlist_t igrPortlist;
    diag_portlist_t egrPortlist;
    uint32 portMbr;
    diag_portlist_t asicPortMbr;
    rtk_portmask_t pmsk;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(egrPortlist, 4), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(igrPortlist, 6), ret);

    DIAG_UTIL_PORTMASK_SCAN(egrPortlist, port)
    {
        /*call API*/
        if((ret = apollo_raw_vlan_egrKeepPmsk_get(port ,&asicPortMbr.portmask)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        pmsk.bits[0] = asicPortMbr.portmask.bits[0];

        DIAG_UTIL_PORTMASK_SCAN(igrPortlist, igrPort)
        {
            if(RTK_PORTMASK_IS_PORT_SET(pmsk, igrPort))
                diag_util_mprintf("Port: %2d Ingress-port: %2d egress-keep:%-6s \n",port,igrPort,DIAG_STR_ENABLE);
            else
                diag_util_mprintf("Port: %2d Ingress-port: %2d egress-keep:%-6s \n",port,igrPort,DIAG_STR_DISABLE);
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_egress_keep_port_egr_ports_all_ingress_port_igr_ports_all_state */

/*
 * vlan get ingress-filter port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_vlan_get_ingress_filter_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    rtk_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Ingress-filter\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        /*call API*/
        if((ret = apollo_raw_vlan_igrFlterEnable_get(port ,&enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("%-4d %s\n",port,diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_ingress_filter_port_ports_all_state */


/*
 * vlan get entry <UINT:index>
 */
cparser_result_t
cparser_cmd_vlan_get_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;
    uint8   buf[UTIL_PORT_MASK_BUFFER_LENGTH];

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("vlan id:%d\n",vlanCfg.index);
    diag_util_mprintf("%4s %5s %6s %5s %3s %3s\n",
                        "Evid",
                        "PlyEn",
                        "MtrIdx",
                        "PriEn",
                        "pri",
                        "Fid");

    diag_util_mprintf("%4d %5s %6d %5s %3d %3d\n",
                        vlanCfg.evid,
                        diagStr_enDisplay[vlanCfg.envlanpol],
                        vlanCfg.meteridx,
                        diagStr_enDisplay[vlanCfg.vbpen],
                        vlanCfg.vbpri,
                        vlanCfg.fid_msti);

    diag_util_lPortMask2str(buf,&vlanCfg.mbr);
    diag_util_mprintf("member port          : %s\n",buf);
    diag_util_lPortMask2str(buf,&vlanCfg.exMbr);
    diag_util_mprintf("extention member port: %s\n",buf);
    diag_util_lPortMask2str(buf,&vlanCfg.dslMbr);
    diag_util_mprintf("dsl member port      : %s\n\n",buf);
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_mbrcfg_vidx_vidx */



/*
 * vlan get entry all
 */
cparser_result_t
cparser_cmd_vlan_get_entry_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;
    int32 index;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    uint8   buf[UTIL_PORT_MASK_BUFFER_LENGTH];

    for(index = 0;index < VLAN_MAX_MEMBER_CFG_IDX;index++)
    {
        memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));
        vlanCfg.index = index;

        if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("vlan id:%d\n\n",vlanCfg.index);
        diag_util_mprintf("%4s %5s %6s %5s %3s %3s\n",
                            "Evid",
                            "PlyEn",
                            "MtrIdx",
                            "PriEn",
                            "pri",
                            "Fid");

        diag_util_mprintf("%4d %5s %6d %5s %3d %3d\n\n",
                            vlanCfg.evid,
                            diagStr_enDisplay[vlanCfg.envlanpol],
                            vlanCfg.meteridx,
                            diagStr_enDisplay[vlanCfg.vbpen],
                            vlanCfg.vbpri,
                            vlanCfg.fid_msti);
        diag_util_lPortMask2str(buf,&vlanCfg.mbr);

        diag_util_mprintf("member port          : %s\n",buf);
        diag_util_lPortMask2str(buf,&vlanCfg.exMbr);
        diag_util_mprintf("extention member port: %s\n",buf);
        diag_util_lPortMask2str(buf,&vlanCfg.dslMbr);
        diag_util_mprintf("dsl member port      : %s\n\n",buf);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_mbrcfg_all */



/*
 * vlan get protocol-vlan group <UINT:index>
 */
cparser_result_t
cparser_cmd_vlan_get_protocol_vlan_group_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_protoValnEntryCfg_t  cfg;

    memset(&cfg,0x0,sizeof(apollo_raw_protoValnEntryCfg_t));

    cfg.index = *index_ptr;

    if((ret = apollo_raw_vlanProtoAndPortBasedEntry_get(&cfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Group  FRAME-TYPE  ETHER-TYPE\n");
    diag_util_mprintf("%-5d  %-10s  0x%-4.4x\n",*index_ptr,diagStr_frameType[cfg.frameType],cfg.etherType);

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_protocol_vlan_group_index */

/*
 * vlan get pvid port ( <PORT_LIST:ports> | all ) vlan_index
 */
cparser_result_t
cparser_cmd_vlan_get_pvid_port_ports_all_vlan_index(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    uint32 vidx;;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port  Vidx\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_vlan_portToMbrCfgIdx_get(port,&vidx)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("%-4d  %-4d\n",port,vidx);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_pvid_port_ports_all_vlan_index */



/*
 * vlan get vc_pvid port ( <PORT_LIST:ports> | dsl_all ) vlan_index
 */
cparser_result_t
cparser_cmd_vlan_get_vc_pvid_port_ports_dsl_all_vlan_index(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;
    uint32 vidx;;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(portlist, 4), ret);

    diag_util_mprintf("VCPort  Vidx\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_vlan_vcPortToMbrCfgIdx_get(port,&vidx)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("%-6d  %-4d\n",port,vidx);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_vc_pvid_port_ports_dsl_all_vlan_index */

/*
 * vlan get ext_pvid port <PORT_LIST:ports> vlan_index
 */
cparser_result_t
cparser_cmd_vlan_get_ext_pvid_port_ports_vlan_index(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;

    rtk_port_t port;
    uint32 vidx;;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(portlist, 4), ret);


    diag_util_mprintf("ExtPort  Vidx\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if(0 == port)
        {
            DIAG_ERR_PRINT(RT_ERR_PORT_ID);
            return CPARSER_NOT_OK;
        }
        if((ret = apollo_raw_vlan_extPortToMbrCfgIdx_get(port-1,&vidx)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("%-7d  %-4d\n",port,vidx);
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_ext_pvid_port_ports_ext_all_vlan_index */



/*
 * vlan get state
 */
cparser_result_t
cparser_cmd_vlan_get_state(
    cparser_context_t *context)
{
    rtk_enable_t enable;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();


    /*call API*/
    if((ret = apollo_raw_vlan_egrIgrFilterEnable_get(&enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    diag_util_mprintf("Valn filter state:%s\n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_state */

/*
 * vlan get tag-mode port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_vlan_get_tag_mode_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan_egrTagMode_t mode;
    rtk_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port Mode\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        /*call API*/
        if((ret = apollo_raw_vlan_egrTagMode_get(port ,&mode)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("%-4d ",port);

        switch(mode)
        {
            case RAW_VLAN_EGR_TAG_MODE_ORI:
                diag_util_mprintf("%s\n","original");
                break;
            case RAW_VLAN_EGR_TAG_MODE_KEEP:
                diag_util_mprintf("%s\n","keep-format");
                break;
            case RAW_VLAN_EGR_TAG_MODE_PRI_TAG:
                diag_util_mprintf("%s\n","priority-tag");
                break;
            default:
                diag_util_mprintf("%s\n",DIAG_STR_INVALID);
                break;

        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_tag_mode_port_ports_all */

/*
 * vlan get transparent state
 */
cparser_result_t
cparser_cmd_vlan_get_transparent_state(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    /*call API*/
    if((ret = apollo_raw_vlan_transparentEnable_get(&enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    diag_util_printf("Vlan Transparent:%s \n",diagStr_enable[enable]);

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_transparent_state */

/*
 * vlan get vlan-treat vid ( 0 | 4095 ) type
 */
cparser_result_t
cparser_cmd_vlan_get_vlan_treat_vid_0_4095_type(
    cparser_context_t *context)
{
    apollo_raw_vlanTag_t tagType;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    /*call API*/
    if('0'==TOKEN_CHAR(4,0))
    {
        if((ret = apollo_raw_vlan_vid0TagType_get(&tagType)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("vlan 0 treat %s\n",diagStr_vlanTagType[tagType]);

    }
    else
    {
        if((ret = apollo_raw_vlan_vid4095TagType_get(&tagType)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("vlan 4095 treat %s\n",diagStr_vlanTagType[tagType]);

    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_vlan_treat_vid_0_4095_type */

/*
 * vlan get vlan vid <UINT:vid>
 */
cparser_result_t
cparser_cmd_vlan_get_vlan_vid_vid(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;
    rtk_portmask_t tagPlst;
    uint8   buf[UTIL_PORT_MASK_BUFFER_LENGTH];

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;

    if((ret = apollo_raw_vlan_4kEntry_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_reverse_portlist(DIAG_PORTTYPE_ALL,&(vlanCfg.untag),&tagPlst);

    diag_util_mprintf("vlan id:%d\n\n",vlanCfg.vid);
    diag_util_mprintf("%4s %5s %6s %5s %3s\n\n",
                      "hash",
                      "PlyEn",
                      "MtrIdx",
                      "PriEn",
                      "Fid");

    diag_util_mprintf("%-4s %-5s %-6d %-5s %-3d\n",
                        diagStr_l2HashMethodStr[vlanCfg.ivl_svl],
                        diagStr_enDisplay[vlanCfg.envlanpol],
                        vlanCfg.meteridx,
                        diagStr_enDisplay[vlanCfg.vbpen],
                        vlanCfg.vbpri,
                        vlanCfg.fid_msti);

    diag_util_lPortMask2str(buf,&vlanCfg.mbr);
    diag_util_mprintf("member port          : %s\n",buf);
    diag_util_lPortMask2str(buf,&tagPlst);
    diag_util_mprintf("tag member port      : %s\n",buf);
    diag_util_lPortMask2str(buf,&vlanCfg.exMbr);
    diag_util_mprintf("extention member port: %s\n",buf);
    diag_util_lPortMask2str(buf,&vlanCfg.dslMbr);
    diag_util_mprintf("dsl member port      : %s\n\n",buf);

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_vlan_vid_vid */

/*
 * vlan set accept-frame-type port ( <PORT_LIST:ports> | all ) ( all | tagged-only | untagged-only | tagged-1p-1q-only )
 */
cparser_result_t
cparser_cmd_vlan_set_accept_frame_type_port_ports_all_all_tagged_only_untagged_only_tagged_1p_1q_only(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanAcpFrameType_t type;
    rtk_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    switch(TOKEN_CHAR(5,0))
    {
        case 'a':
            type = RAW_VLAN_ACCEPT_ALL;
            break;

        case 'u':
            type = RAW_VLAN_ACCEPT_UNTAGGED_ONLY;
            break;

        case 't':
            if('o'==TOKEN_CHAR(5,7))
                type = RAW_VLAN_ACCEPT_TAGGED_ONLY;
            else
                type = RAW_VLAN_ACCEPT_1Q_1P_TAGGED_ONLY;
            break;

        default:
            return CPARSER_NOT_OK;
    }



    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        /*call API*/
        if((ret = apollo_raw_vlan_acceptFrameType_set(port ,type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_accept_frame_type_port_ports_all_all_tagged_only_untagged_only_tagged_1p_1q_only */

/*
 * vlan set egress-keep port ( <PORT_LIST:egr_ports> | all ) ingress-port ( <PORT_LIST:igr_ports> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_vlan_set_egress_keep_port_egr_ports_all_ingress_port_igr_ports_all_state_enable_disable(
    cparser_context_t *context,
    char * *egr_ports_ptr,
    char * *igr_ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    rtk_port_t port;
    diag_portlist_t setPortlist;
    diag_portlist_t egrPortlist;
    uint32 portMbr;
    diag_portlist_t asicPortMbr;

    DIAG_UTIL_PARAM_CHK();

    if('e'==(TOKEN_CHAR(8,0)))
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(setPortlist, 4), ret);
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(egrPortlist, 6), ret);
    portMbr = egrPortlist.portmask.bits[0];

    DIAG_UTIL_PORTMASK_SCAN(setPortlist, port)
    {
        /*call API*/
        if((ret = apollo_raw_vlan_egrKeepPmsk_get(port ,&(asicPortMbr.portmask))) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        if(enable == ENABLED)
            asicPortMbr.portmask.bits[0] = asicPortMbr.portmask.bits[0] | portMbr;
        else
            asicPortMbr.portmask.bits[0] = asicPortMbr.portmask.bits[0] & (~portMbr);

        if((ret = apollo_raw_vlan_egrKeepPmsk_set(port ,asicPortMbr.portmask)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_egress_keep_port_egr_ports_all_ingress_port_igr_ports_all_state_enable_disable */

/*
 * vlan set ingress-filter port ( <PORT_LIST:ports> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_vlan_set_ingress_filter_port_ports_all_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    rtk_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e' == TOKEN_CHAR(6,0))
        enable = ENABLED;
    else
        enable = DISABLED;


    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        /*call API*/
        if((ret = apollo_raw_vlan_igrFlterEnable_set(port ,enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_ingress_filter_port_ports_all_state_enable_disable */

/*
 * vlan set entry <UINT:index> evid <UINT:evid>
 */
cparser_result_t
cparser_cmd_vlan_set_entry_index_evid_evid(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *evid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.evid = *evid_ptr;
    if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_mbrcfg_vidx_vidx_evid_evid */

/*
 * vlan set entry <UINT:index> filter-id <UINT:fid>
 */
cparser_result_t
cparser_cmd_vlan_set_entry_index_filter_id_fid(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *fid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.fid_msti = *fid_ptr;
    if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_mbrcfg_vidx_vidx_filter_id_fid */

/*
 * vlan set mbrcfg vidx <UINT:vidx> member <PORT_LIST:member_ports>
 */
/*
 * vlan set entry <UINT:index> member <PORT_LIST:member_ports>
 */
cparser_result_t
cparser_cmd_vlan_set_entry_index_member_member_ports(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    char * *member_ports_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    RTK_PORTMASK_ASSIGN(vlanCfg.mbr,portlist.portmask);

    if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_mbrcfg_vidx_vidx_member_member_ports */


/*
 * vlan set entry <UINT:index> dsl_member <PORT_LIST:dls_member_ports>
 */
cparser_result_t
cparser_cmd_vlan_set_entry_index_dsl_member_dls_member_ports(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    char * *dls_member_ports_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(portlist, 5), ret);

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    RTK_PORTMASK_ASSIGN(vlanCfg.dslMbr,portlist.portmask);
    if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_mbrcfg_vidx_vidx_dsl_member_dls_member_ports */

/*
 * vlan set entry <UINT:index> ext_member <PORT_LIST:ext_member_ports>
 */
cparser_result_t
cparser_cmd_vlan_set_entry_index_ext_member_ext_member_ports(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    char * *ext_member_ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(portlist, 5), ret);

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    RTK_PORTMASK_ASSIGN(vlanCfg.exMbr,portlist.portmask);
    if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_mbrcfg_vidx_vidx_ext_member_ext_member_ports */



/*
 * vlan set entry <UINT:index> policing disable
 */
cparser_result_t
cparser_cmd_vlan_set_entry_index_policing_disable(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.envlanpol = DISABLED;
    if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_mbrcfg_vidx_vidx_policing_disable */

/*
 * vlan set entry <UINT:index> policing enable meter_index <UINT:meter_index>
 */
cparser_result_t
cparser_cmd_vlan_set_entry_index_policing_enable_meter_index_meter_index(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *meter_index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.envlanpol = ENABLED;
    vlanCfg.meteridx = *meter_index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_mbrcfg_vidx_vidx_policing_enable_meter_index_index */

/*
 * vlan set entry <UINT:index> vlan-based-pri disable
 */
cparser_result_t
cparser_cmd_vlan_set_entry_index_vlan_based_pri_disable(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.vbpen = DISABLED;
    if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_mbrcfg_vidx_vidx_vlan_based_pri_disable */

/*
 * vlan set entry <UINT:index> vlan-based-pri enable priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_vlan_set_entry_index_vlan_based_pri_enable_priority_pri(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *pri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlanconfig_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlanconfig_t));

    vlanCfg.index = *index_ptr;

    if((ret = apollo_raw_vlan_memberConfig_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.vbpen = ENABLED;
    vlanCfg.vbpri = *pri_ptr;

    if((ret = apollo_raw_vlan_memberConfig_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_mbrcfg_vidx_vidx_vlan_based_pri_enable_priority_pri */

/*
 * vlan set protocol-vlan group <UINT:index> frame-type ( ethernet | snap | llc-other ) <UINT:frame_type>
 */
cparser_result_t
cparser_cmd_vlan_set_protocol_vlan_group_index_frame_type_ethernet_snap_llc_other_frame_type(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *frame_type_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_protoValnEntryCfg_t  cfg;

    memset(&cfg,0x0,sizeof(apollo_raw_protoValnEntryCfg_t));

    switch(TOKEN_CHAR(6,0))
    {
        case 'e':
            cfg.frameType = RAW_PPVLAN_FRAME_TYPE_ETHERNET;
            break;

        case 's':
            cfg.frameType = RAW_PPVLAN_FRAME_TYPE_RFC1042;
            break;

        case 'l':
            cfg.frameType = RAW_PPVLAN_FRAME_TYPE_LLC;
            break;

        default:
            return CPARSER_NOT_OK;
    }
    cfg.index = *index_ptr;
    cfg.etherType = *frame_type_ptr;

    if((ret = apollo_raw_vlanProtoAndPortBasedEntry_set(&cfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_protocol_vlan_group_index_frame_type_ethernet_snap_llc_other_frame_type */

/*
 * vlan get protocol-vlan port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_vlan_get_protocol_vlan_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 protoId;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_protoVlanCfg_t cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port  ProtoIdx    Valid  VlanIDX  Pri\n");

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        memset(&cfg,0x0,sizeof(apollo_raw_protoVlanCfg_t));

        for(protoId = 0;protoId < VLAN_MAX_PROTO_VLAN_IDX;protoId++)
        {
            cfg.index = protoId;
            if((ret = apollo_raw_vlan_portAndProtoBasedEntry_get(port,&cfg)) != RT_ERR_OK)
            {
                DIAG_ERR_PRINT(ret);
                return CPARSER_NOT_OK;
            }
            diag_util_mprintf("%-4d  %-8d  %-7s  %-9d  %-3d\n",port,protoId,diagStr_valid[cfg.valid],cfg.vlan_idx,cfg.priority);

        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_protocol_vlan_port_ports_all */

/*
 * vlan set protocol-vlan port ( <PORT_LIST:ports> | all ) group <UINT:index> state ( enable | disable )
 */
cparser_result_t
cparser_cmd_vlan_set_protocol_vlan_port_ports_all_group_index_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 protoId;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_protoVlanCfg_t cfg;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    if('e' == TOKEN_CHAR(8,0))
        enable = ENABLED;
    else
        enable = DISABLED;

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        memset(&cfg,0x0,sizeof(apollo_raw_protoVlanCfg_t));

        cfg.index = *index_ptr;
        if((ret = apollo_raw_vlan_portAndProtoBasedEntry_get(port,&cfg)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if(ENABLED == enable)
            cfg.valid = 1;
        else
            cfg.valid = 0;

        if((ret = apollo_raw_vlan_protoAndPortBasedEntry_set(port,&cfg)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_protocol_vlan_port_ports_all_group_index_state_enable_disable */

/*
 * vlan set protocol-vlan port ( <PORT_LIST:ports> | all ) group <UINT:index> vlan_index <UINT:vidx> priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_vlan_set_protocol_vlan_port_ports_all_group_index_vlan_index_vidx_priority_pri(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *index_ptr,
    uint32_t  *vidx_ptr,
    uint32_t  *pri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    int32 protoId;
    diag_portlist_t portlist;
    rtk_port_t port;
    apollo_raw_protoVlanCfg_t cfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        memset(&cfg,0x0,sizeof(apollo_raw_protoVlanCfg_t));

        cfg.index = *index_ptr;
        cfg.valid = 1;
        cfg.vlan_idx = *vidx_ptr;
        cfg.priority = *pri_ptr;

        if((ret = apollo_raw_vlan_protoAndPortBasedEntry_set(port,&cfg)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_protocol_vlan_port_ports_all_group_index_vlan_index_vidx_priority_pri */

/*
 * vlan set pvid port ( <PORT_LIST:ports> | all ) vlan_index <UINT:vidx>
 */
cparser_result_t
cparser_cmd_vlan_set_pvid_port_ports_all_vlan_index_vidx(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *vidx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_vlan_portToMbrCfgIdx_set(port,*vidx_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_pvid_port_ports_all_vlan_index_vidx */


/*
 * vlan set vc_pvid port ( <PORT_LIST:ports> | dsl_all ) vlan_index <UINT:vidx>
 */
cparser_result_t
cparser_cmd_vlan_set_vc_pvid_port_ports_dsl_all_vlan_index_vidx(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *vidx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_vlan_vcPortToMbrCfgIdx_set(port,*vidx_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_dsl_pvid_port_ports_dsl_all_vlan_index_vidx */


/*
 * vlan set ext_pvid port <PORT_LIST:ports> vlan_index <UINT:vidx>
 */
cparser_result_t
cparser_cmd_vlan_set_ext_pvid_port_ports_vlan_index_vidx(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *vidx_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if(0 == port)
        {
            DIAG_ERR_PRINT(RT_ERR_PORT_ID);
            return CPARSER_NOT_OK;
        }
        if((ret = apollo_raw_vlan_extPortToMbrCfgIdx_set(port-1,*vidx_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_ext_pvid_port_ports_ext_all_vlan_index_vidx */



/*
 * vlan set state ( enable | disable )
 */
cparser_result_t
cparser_cmd_vlan_set_state_enable_disable(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(TOKEN_CHAR(3,0)=='e')
        enable = ENABLED;
    else
        enable = DISABLED;

    /*call API*/
    if((ret = apollo_raw_vlan_egrIgrFilterEnable_set(enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_state_enable_disable */

/*
 * vlan set tag-mode port ( <PORT_LIST:ports> | all ) ( original | keep-format | priority-tag )
 */
cparser_result_t
cparser_cmd_vlan_set_tag_mode_port_ports_all_original_keep_format_priority_tag(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan_egrTagMode_t type;
    rtk_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    switch(TOKEN_CHAR(5,0))
    {
        case 'o':
            type = RAW_VLAN_EGR_TAG_MODE_ORI;
            break;

        case 'k':
            type = RAW_VLAN_EGR_TAG_MODE_KEEP;
            break;

        case 'p':
            type = RAW_VLAN_EGR_TAG_MODE_PRI_TAG;
            break;

        default:
            return CPARSER_NOT_OK;
    }



    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        /*call API*/
        if((ret = apollo_raw_vlan_egrTagMode_set(port ,type)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_tag_mode_port_ports_all_original_keep_format_priority_tag */

/*
 * vlan set transparent state ( enable | disable )
 */
cparser_result_t
cparser_cmd_vlan_set_transparent_state_enable_disable(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;

    if('e' == TOKEN_CHAR(4,0))
        enable = ENABLED;
    else
        enable = DISABLED;

    /*call API*/
    if((ret = apollo_raw_vlan_transparentEnable_set(enable)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_transparent_state_enable_disable */

/*
 * vlan set vlan-treat vid ( 0 | 4095 ) type ( tagging | un-tagging )
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_treat_vid_0_4095_type_tagging_un_tagging(
    cparser_context_t *context)
{
    apollo_raw_vlanTag_t tagType;
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();

    if('u'==TOKEN_CHAR(6,0))
        tagType = RAW_VLAN_UNTAG;
    else
        tagType = RAW_VLAN_TAG;

    /*call API*/
    if('0'==TOKEN_CHAR(4,0))
    {
        if((ret = apollo_raw_vlan_vid0TagType_set(tagType)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    else
    {
        if((ret = apollo_raw_vlan_vid4095TagType_set(tagType)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_vlan_treat_vid_0_4095_type_tagging_un_tagging */

/*
 * vlan set vlan vid <UINT:vid> filter-id <UINT:fid>
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_vid_vid_filter_id_fid(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *fid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;
    if((ret = apollo_raw_vlan_4kEntry_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.fid_msti = *fid_ptr;
    if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_vlan_vid_vid_filter_id_fid */

/*
 * vlan set vlan vid <UINT:vid> hash-mode ( ivl | svl )
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_vid_vid_hash_mode_ivl_svl(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;
    if((ret = apollo_raw_vlan_4kEntry_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('i' == TOKEN_CHAR(6,0))
        vlanCfg.ivl_svl = RAW_VLAN_HASH_IVL;
    else
        vlanCfg.ivl_svl = RAW_VLAN_HASH_SVL;

    if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_vlan_vid_vid_hash_mode_ivl_svl */

/*
 * vlan set vlan vid <UINT:vid> member <PORT_LIST:member_ports> dsl ( <PORT_LIST:dsl> | dsl_all ) ext ( <PORT_LIST:ext> | ext_all )
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_vid_vid_member_member_ports_dsl_dsl_dsl_all_ext_ext_ext_all(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    char * *member_ports_ptr,
    char * *dsl_ptr,
    char * *ext_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;
    if((ret = apollo_raw_vlan_4kEntry_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);
    RTK_PORTMASK_ASSIGN(vlanCfg.mbr, portlist.portmask);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(portlist, 8), ret);
    RTK_PORTMASK_ASSIGN(vlanCfg.dslMbr, portlist.portmask);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(portlist, 10), ret);
    RTK_PORTMASK_ASSIGN(vlanCfg.exMbr, portlist.portmask);

    if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_vlan_vid_vid_member_member_ports_dsl_dsl_dsl_all_ext_ext_ext_all */

/*
 * vlan set vlan vid <UINT:vid> member <PORT_LIST:member_ports> tag_mbr ( <PORT_LIST:tagged_ports> | none )
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_vid_vid_member_member_ports_tag_mbr_tagged_ports_none(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    char * *member_ports_ptr,
    char * *tagged_ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;
    diag_portlist_t portlist;
    rtk_portmask_t tagPortlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;
    if((ret = apollo_raw_vlan_4kEntry_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);
    RTK_PORTMASK_ASSIGN(vlanCfg.mbr, portlist.portmask);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    diag_util_reverse_portlist(DIAG_PORTTYPE_ALL,&(portlist.portmask),&tagPortlist);

    vlanCfg.untag.bits[0] = tagPortlist.bits[0];


    if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_vlan_vid_vid_member_member_ports_tag_mbr_tagged_ports_none */

/*
 * vlan set vlan vid <UINT:vid> member <PORT_LIST:member_ports> tag_mbr ( <PORT_LIST:tagged_ports> | none ) dsl ( <PORT_LIST:dsl> | dsl_all ) ext ( <PORT_LIST:ext> | ext_all )
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_vid_vid_member_member_ports_tag_mbr_tagged_ports_none_dsl_dsl_dsl_all_ext_ext_ext_all(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    char * *member_ports_ptr,
    char * *tagged_ports_ptr,
    char * *dsl_ptr,
    char * *ext_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;
    diag_portlist_t portlist;
    rtk_portmask_t tagPortlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;
    if((ret = apollo_raw_vlan_4kEntry_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);
    RTK_PORTMASK_ASSIGN(vlanCfg.mbr, portlist.portmask);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 8), ret);

    diag_util_reverse_portlist(DIAG_PORTTYPE_ALL,&(portlist.portmask),&tagPortlist);

    vlanCfg.untag.bits[0] = tagPortlist.bits[0];

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_DSLLIST(portlist, 10), ret);
    RTK_PORTMASK_ASSIGN(vlanCfg.dslMbr, portlist.portmask);

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_EXTLIST(portlist, 12), ret);
    RTK_PORTMASK_ASSIGN(vlanCfg.exMbr, portlist.portmask);


    if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_vlan_vid_vid_member_member_ports_tag_mbr_tagged_ports_none_dsl_dsl_dsl_all_ext_ext_ext_all */

/*
 * vlan set vlan vid <UINT:vid> policing disable
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_vid_vid_policing_disable(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;
    if((ret = apollo_raw_vlan_4kEntry_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.envlanpol = DISABLED;
    if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_vlan_vid_vid_policing_disable */

/*
 * vlan set vlan vid <UINT:vid> policing enable meter_index <UINT:index>
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_vid_vid_policing_enable_meter_index_index(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;
    if((ret = apollo_raw_vlan_4kEntry_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.envlanpol = ENABLED;
    vlanCfg.meteridx = *index_ptr;
    if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_vlan_vid_vid_policing_enable_meter_index_index */

/*
 * vlan set vlan vid <UINT:vid> vlan-based-pri disable
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_vid_vid_vlan_based_pri_disable(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;
    if((ret = apollo_raw_vlan_4kEntry_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.vbpen = DISABLED;
    if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_vlan_vid_vid_vlan_based_pri_disable */

/*
 * vlan set vlan vid <UINT:vid> vlan-based-pri enable priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_vlan_set_vlan_vid_vid_vlan_based_pri_enable_priority_pri(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *pri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan4kentry_t vlanCfg;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vlanCfg,0x0,sizeof(apollo_raw_vlan4kentry_t));

    vlanCfg.vid = *vid_ptr;
    if((ret = apollo_raw_vlan_4kEntry_get(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    vlanCfg.vbpen = ENABLED;
    vlanCfg.vbpri = *pri_ptr;
    if((ret = apollo_raw_vlan_4kEntry_set(&vlanCfg)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_vlan_vid_vid_vlan_based_pri_enable_priority_pri */



/*
 * vlan get cfi-keep
 */
cparser_result_t
cparser_cmd_vlan_get_cfi_keep(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan_cfiKeepMode_t mode;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    /*call API*/
    if((ret = apollo_raw_vlan_cfiKeepMode_get(&mode)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if(mode == 0)
        diag_util_printf("cfi-keep mode:%S \n","egress tag CFI always 0");
    else
        diag_util_printf("cfi-keep mode:%S \n","egress tag keep ingress tag value");

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_cfi_keep */

/*
 * vlan set cfi-keep ( cfi_to_0 | keep_cfi )
 */
cparser_result_t
cparser_cmd_vlan_set_cfi_keep_cfi_to_0_keep_cfi(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_vlan_cfiKeepMode_t mode;
    DIAG_UTIL_PARAM_CHK();

    if('c' == TOKEN_CHAR(3,0))
        mode = RAW_CFI_ALWAYS_0;
    else
        mode = RAW_CFI_KEEP_INGRESS;
    /*call API*/
    if((ret = apollo_raw_vlan_cfiKeepMode_set(mode)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_cfi_keep_cfi_to_0_keep_cfi */

/*
 * vlan set port-based-fid port ( <PORT_LIST:ports> | all ) enable fid <UINT:fid>
 */
cparser_result_t
cparser_cmd_vlan_set_port_based_fid_port_ports_all_enable_fid_fid(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *fid_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_vlan_portBasedFidEnable_set(port, ENABLED)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if((ret = apollo_raw_vlan_portBasedFid_set(port, *fid_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_port_based_fid_port_ports_all_enable_fid_fid */

/*
 * vlan set port-based-fid port ( <PORT_LIST:ports> | all ) disable
 */
cparser_result_t
cparser_cmd_vlan_set_port_based_fid_port_ports_all_disable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_vlan_portBasedFidEnable_set(port ,DISABLED)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_port_based_fid_port_ports_all_disable */

/*
 * vlan get port-based-fid port ( <PORT_LIST:ports> | all )
 */
cparser_result_t
cparser_cmd_vlan_get_port_based_fid_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_port_t port;
    diag_portlist_t portlist;
    uint32 fid;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port  FID  State\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_vlan_portBasedFidEnable_get(port ,&enable)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        if((ret = apollo_raw_vlan_portBasedFid_get(port ,&fid)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("%-4d  %-3d  %s\n",port,fid,diagStr_enable[enable]);

    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_port_based_fid_port_ports_all */

/*
 * vlan set port-based-pri port ( <PORT_LIST:ports> | all ) priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_vlan_set_port_based_pri_port_ports_all_priority_pri(
    cparser_context_t *context,
    char * *ports_ptr,
    uint32_t  *pri_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_port_t port;
    diag_portlist_t portlist;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_vlan_portBasedPri_set(port ,*pri_ptr)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_port_based_pri_port_ports_all_enable_priority_pri */


/*
 * vlan get port-based-pri port ( <PORT_LIST:ports> | all ) */
cparser_result_t
cparser_cmd_vlan_get_port_based_pri_port_ports_all(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_port_t port;
    diag_portlist_t portlist;
    uint32 pri;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_util_mprintf("Port  Priority\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        if((ret = apollo_raw_vlan_portBasedPri_get(port ,&pri)) != RT_ERR_OK)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        diag_util_mprintf("%-4d  %-2d\n",port,pri);

    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_port_based_pri_port_ports_all */

/*
 * vlan set leaky ip-mcast port ( <PORT_LIST:ports> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_vlan_set_leaky_ip_mcast_port_ports_all_state_enable_disable(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    if ('d' == TOKEN_CHAR(7,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(7,0))
    {
        enable = ENABLED;
    }


    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_vlan_ipmcastLeaky_set(port, enable), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_set_leaky_ip_mcast_port_ports_all_state_enable_disable */

/*
 * vlan get leaky ip-mcast port ( <PORT_LIST:ports> | all ) state
 */
cparser_result_t
cparser_cmd_vlan_get_leaky_ip_mcast_port_ports_all_state(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_util_mprintf("Port State\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_vlan_ipmcastLeaky_get(port, &enable), ret);

        diag_util_mprintf("%-5d%s\n", port, diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_vlan_get_leaky_ip_mcast_port_ports_all_state */





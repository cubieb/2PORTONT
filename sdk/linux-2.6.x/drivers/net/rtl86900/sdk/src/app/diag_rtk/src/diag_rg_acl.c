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
 * $Revision: 65434 $
 * $Date: 2016-01-22 14:02:05 +0800 (Fri, 22 Jan 2016) $
 *
 * Purpose : Definition those XXX command and APIs in the SDK diagnostic shell.
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
#include <hal/common/halctrl.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <parser/cparser_priv.h>
#include <diag_str.h>

#include <rtk_rg_struct.h>

//#include "../../../../uClinux-dist/linux-2.6.33.2/drivers/net/apollo/romeDriver/rtk_rg_define.h"
//#include "../../../../uClinux-dist/linux-2.6.33.2/drivers/net/apollo/romeDriver/rtk_rg_struct.h"


/* variable */
static rtk_rg_aclFilterAndQos_t acl_parameter;
static rtk_rg_classifyEntry_t classifyFilter;


/* local function */
static uint32 _diag_rg_aclFilter_show_field(rtk_rg_aclFilterAndQos_t *aclPara)
{		
	diag_util_printf("Current Temp ACL-filter entry:\n");
	diag_util_printf("Patterns: \n");	
	
	diag_util_printf("filter_fields:0x%llx\n", aclPara->filter_fields);
	diag_util_printf("ingress_port_mask:0x%x\n", aclPara->ingress_port_mask);	
	diag_util_printf("ingress_dscp:     %d\n", aclPara->ingress_dscp);		
	diag_util_printf("ingress_ipv6_dscp:	%d\n", aclPara->ingress_ipv6_dscp);	
	diag_util_printf("ingress_intf_idx: %d\n", aclPara->ingress_intf_idx);	
	diag_util_printf("egress_intf_idx:  %d\n", aclPara->egress_intf_idx);	
	diag_util_printf("ingress_stream_id:  %d\n", aclPara->ingress_stream_id);	
	diag_util_printf("ingress_ethertype:0x%x\n", aclPara->ingress_ethertype);			
	diag_util_printf("ingress_ctag_vid: %d\n", aclPara->ingress_ctag_vid);	
	diag_util_printf("ingress_ctag_pri: %d\n", aclPara->ingress_ctag_pri);	
	diag_util_printf("egress_ctag_vid: %d\n", aclPara->egress_ctag_vid);	
	diag_util_printf("egress_ctag_vid_mask: %d\n", aclPara->egress_ctag_vid_mask);	
	diag_util_printf("egress_ctag_pri: %d\n", aclPara->egress_ctag_pri);	
	diag_util_printf("egress_ip4mc_if: %d\n", aclPara->egress_ip4mc_if);
	diag_util_printf("egress_ip6mc_if: %d\n", aclPara->egress_ip6mc_if);

	diag_util_printf("ingress_smac: %02X:%02X:%02X:%02X:%02X:%02X\n", 
			aclPara->ingress_smac.octet[0],
			aclPara->ingress_smac.octet[1],
			aclPara->ingress_smac.octet[2],
			aclPara->ingress_smac.octet[3],
			aclPara->ingress_smac.octet[4],
			aclPara->ingress_smac.octet[5]);			
	diag_util_printf("ingress_dmac: %02X:%02X:%02X:%02X:%02X:%02X\n",
			aclPara->ingress_dmac.octet[0],
			aclPara->ingress_dmac.octet[1],
			aclPara->ingress_dmac.octet[2],
			aclPara->ingress_dmac.octet[3],
			aclPara->ingress_dmac.octet[4],
			aclPara->ingress_dmac.octet[5]);

	diag_util_printf("ingress_sip_low_bound: %s  ", diag_util_inet_ntoa(acl_parameter.ingress_src_ipv4_addr_start));
	diag_util_printf("ingress_sip_up_bound:  %s \n", diag_util_inet_ntoa(acl_parameter.ingress_src_ipv4_addr_end));

	diag_util_printf("ingress_dip_low_bound: %s  ", diag_util_inet_ntoa(acl_parameter.ingress_dest_ipv4_addr_start));
	diag_util_printf("ingress_dip_up_bound:  %s \n", diag_util_inet_ntoa(acl_parameter.ingress_dest_ipv4_addr_end));


	diag_util_printf("ingress_src_ipv6_addr_start: %02X%02X:",acl_parameter.ingress_src_ipv6_addr_start[0],acl_parameter.ingress_src_ipv6_addr_start[1]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_start[2],acl_parameter.ingress_src_ipv6_addr_start[3]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_start[4],acl_parameter.ingress_src_ipv6_addr_start[5]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_start[6],acl_parameter.ingress_src_ipv6_addr_start[7]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_start[8],acl_parameter.ingress_src_ipv6_addr_start[9]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_start[10],acl_parameter.ingress_src_ipv6_addr_start[11]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_start[12],acl_parameter.ingress_src_ipv6_addr_start[13]);
	diag_util_printf(":%02X%02X: \n",acl_parameter.ingress_src_ipv6_addr_start[14],acl_parameter.ingress_src_ipv6_addr_start[15]);

	diag_util_printf("ingress_src_ipv6_addr_end: %02X%02X:",acl_parameter.ingress_src_ipv6_addr_end[0],acl_parameter.ingress_src_ipv6_addr_end[1]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_end[2],acl_parameter.ingress_src_ipv6_addr_end[3]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_end[4],acl_parameter.ingress_src_ipv6_addr_end[5]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_end[6],acl_parameter.ingress_src_ipv6_addr_end[7]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_end[8],acl_parameter.ingress_src_ipv6_addr_end[9]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_end[10],acl_parameter.ingress_src_ipv6_addr_end[11]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_src_ipv6_addr_end[12],acl_parameter.ingress_src_ipv6_addr_end[13]);
	diag_util_printf(":%02X%02X: \n",acl_parameter.ingress_src_ipv6_addr_end[14],acl_parameter.ingress_src_ipv6_addr_end[15]);

	diag_util_printf("ingress_dest_ipv6_addr_start: %02X%02X:",acl_parameter.ingress_dest_ipv6_addr_start[0],acl_parameter.ingress_dest_ipv6_addr_start[1]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_start[2],acl_parameter.ingress_dest_ipv6_addr_start[3]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_start[4],acl_parameter.ingress_dest_ipv6_addr_start[5]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_start[6],acl_parameter.ingress_dest_ipv6_addr_start[7]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_start[8],acl_parameter.ingress_dest_ipv6_addr_start[9]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_start[10],acl_parameter.ingress_dest_ipv6_addr_start[11]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_start[12],acl_parameter.ingress_dest_ipv6_addr_start[13]);
	diag_util_printf(":%02X%02X: \n",acl_parameter.ingress_dest_ipv6_addr_start[14],acl_parameter.ingress_dest_ipv6_addr_start[15]);

	diag_util_printf("ingress_dest_ipv6_addr_end: %02X%02X:",acl_parameter.ingress_dest_ipv6_addr_end[0],acl_parameter.ingress_dest_ipv6_addr_end[1]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_end[2],acl_parameter.ingress_dest_ipv6_addr_end[3]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_end[4],acl_parameter.ingress_dest_ipv6_addr_end[5]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_end[6],acl_parameter.ingress_dest_ipv6_addr_end[7]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_end[8],acl_parameter.ingress_dest_ipv6_addr_end[9]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_end[10],acl_parameter.ingress_dest_ipv6_addr_end[11]);
	diag_util_printf(":%02X%02X:",acl_parameter.ingress_dest_ipv6_addr_end[12],acl_parameter.ingress_dest_ipv6_addr_end[13]);
	diag_util_printf(":%02X%02X: \n",acl_parameter.ingress_dest_ipv6_addr_end[14],acl_parameter.ingress_dest_ipv6_addr_end[15]);


	diag_util_printf("ingress_src_l4_port_low_bound: %d  ", acl_parameter.ingress_src_l4_port_start);
	diag_util_printf("ingress_src_l4_port_up_bound:  %d \n", acl_parameter.ingress_src_l4_port_end);

	diag_util_printf("ingress_dest_l4_port_low_bound: %d  ", acl_parameter.ingress_dest_l4_port_start);
	diag_util_printf("ingress_dest_l4_port_up_bound:  %d \n", acl_parameter.ingress_dest_l4_port_end);


	diag_util_printf("egress_sip_low_bound: %s  ", diag_util_inet_ntoa(acl_parameter.egress_src_ipv4_addr_start));
	diag_util_printf("egress_sip_up_bound:  %s \n", diag_util_inet_ntoa(acl_parameter.egress_src_ipv4_addr_end));

	diag_util_printf("egress_dip_low_bound: %s  ", diag_util_inet_ntoa(acl_parameter.egress_dest_ipv4_addr_start));
	diag_util_printf("egress_dip_up_bound:  %s \n", diag_util_inet_ntoa(acl_parameter.egress_dest_ipv4_addr_end));

	diag_util_printf("egress_src_l4_port_low_bound: %d  ", acl_parameter.egress_src_l4_port_start);
	diag_util_printf("egress_src_l4_port_up_bound:  %d \n", acl_parameter.egress_src_l4_port_end);

	diag_util_printf("egress_dest_l4_port_low_bound: %d  ", acl_parameter.egress_dest_l4_port_start);
	diag_util_printf("egress_dest_l4_port_up_bound:  %d \n", acl_parameter.egress_dest_l4_port_end);

	if(acl_parameter.filter_fields & INGRESS_L4_TCP_BIT){
		diag_util_printf("l4-protocal: tcp \n");
	}else if(acl_parameter.filter_fields & INGRESS_L4_UDP_BIT){
		diag_util_printf("l4-protocal: udp \n");
	}else if(acl_parameter.filter_fields & INGRESS_L4_ICMP_BIT){
		diag_util_printf("l4-protocal: icmp \n");
	}else{
		diag_util_printf("l4-protocal: not care \n");
	}

	diag_util_printf("ingress_wlanDevIdx:  %d \n", acl_parameter.ingress_wlanDevMask);

	diag_util_printf("\n");
	diag_util_printf("Actions: \n");
	switch(acl_parameter.action_type){
		case ACL_ACTION_TYPE_DROP:
			diag_util_printf("action_type: ACL_ACTION_TYPE_DROP \n");
			break;
		case ACL_ACTION_TYPE_PERMIT:
			diag_util_printf("action type: ACL_ACTION_TYPE_PERMIT \n");
			break;
		case ACL_ACTION_TYPE_TRAP:
			diag_util_printf("action type: ACL_ACTION_TYPE_TRAP \n");
			break;
		case ACL_ACTION_TYPE_TRAP_TO_PS:
			diag_util_printf("action type: ACL_ACTION_TYPE_TRAP_TO_PS \n");
			break;
		case ACL_ACTION_TYPE_QOS:
			diag_util_printf("action type: ACL_ACTION_TYPE_QOS \n");
			diag_util_printf("qos_actions_bits: 0x%x\n",acl_parameter.qos_actions);
			diag_util_printf("dot1p_remarking_pri: %d \n",acl_parameter.action_dot1p_remarking_pri);
			diag_util_printf("ip_precedence_remarking_pri: %d\n",acl_parameter.action_ip_precedence_remarking_pri);
			diag_util_printf("dot1p_remarking_pri: %d\n",acl_parameter.action_dscp_remarking_pri);
			diag_util_printf("queue_id: %d\n",acl_parameter.action_queue_id);
			diag_util_printf("share_meter: %d\n",acl_parameter.action_share_meter);	
			diag_util_printf("acl_priority: %d\n",acl_parameter.action_acl_priority);	
			diag_util_printf("ingress_cvid: %d\n",acl_parameter.action_acl_ingress_vid);
			diag_util_mprintf("downstream_uni_portmask: 0x%x\n",acl_parameter.downstream_uni_portmask);
			diag_util_mprintf("redirect_portmask: 0x%x\n",acl_parameter.redirect_portmask);
			diag_util_mprintf("egress_internal_priority: 0x%x\n",acl_parameter.egress_internal_priority);
			break;
		case ACL_ACTION_TYPE_POLICY_ROUTE:
			diag_util_printf("action type: ACL_ACTION_TYPE_POLICY_ROUTE \n");
			diag_util_printf("policy_route_wan: 0x%x\n",acl_parameter.action_policy_route_wan);
	}
	
	return CPARSER_OK;
	
}


static uint32 _diag_rg_classifyFilter_show_field(rtk_rg_classifyEntry_t *classifyFilter){
	
#if defined(CONFIG_RTL9602C_SERIES)
#else
	diag_util_printf("Current Temp classify-filter entry:\n");
	diag_util_printf("Index: %d \n",classifyFilter->index);	
	diag_util_printf("Direction: %s \n",classifyFilter->direction?"downstream":"upstream");	
	diag_util_printf("Patterns: \n");
	if(classifyFilter->filter_fields & EGRESS_ETHERTYPR_BIT)
		diag_util_printf("etherType:0x%x\n", classifyFilter->etherType);	
	if(classifyFilter->filter_fields & EGRESS_GEMIDX_BIT)
		diag_util_printf("gemidx:%d\n", classifyFilter->gemidx);	
	if(classifyFilter->filter_fields & EGRESS_LLID_BIT)
		diag_util_printf("llid:%d\n", classifyFilter->llid);	
	if(classifyFilter->filter_fields & EGRESS_TAGVID_BIT)
		diag_util_printf("outterTagVid:%d\n", classifyFilter->outterTagVid);	
	if(classifyFilter->filter_fields & EGRESS_TAGPRI_BIT)
		diag_util_printf("outterTagPri:%d\n", classifyFilter->outterTagPri);	
	if(classifyFilter->filter_fields & EGRESS_INTERNALPRI_BIT)
		diag_util_printf("internalPri:%d\n", classifyFilter->internalPri);	
	if(classifyFilter->filter_fields & EGRESS_STAGIF_BIT)
		diag_util_printf("stagIf:%s\n", classifyFilter->stagIf?"Must Tag":"Must unTag");
	if(classifyFilter->filter_fields & EGRESS_CTAGIF_BIT)
		diag_util_printf("ctagIf:%s\n", classifyFilter->ctagIf?"Must Tag":"Must unTag");
	if(classifyFilter->filter_fields & EGRESS_UNI_BIT)
		diag_util_printf("uni:%d\n", classifyFilter->uni);

	diag_util_printf("Action: \n");
	if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_UPSTREAM){
		diag_util_printf("US_ACTION_MASKBIT=0x%x \n",classifyFilter->us_action_field);
		
		if(classifyFilter->us_action_field&CF_US_ACTION_DROP_BIT)
			diag_util_printf("US_ACTION_DROP \n");
		if(classifyFilter->us_action_field&CF_US_ACTION_SID_BIT)
			diag_util_printf("US_ACTION_ASSIGN_SID: sid=%d \n",classifyFilter->action_sid_or_llid.assignedSid_or_llid);	

	}else if(classifyFilter->direction==RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM){
		diag_util_printf("DS_ACTION_MASKBIT=0x%x \n",classifyFilter->ds_action_field);
		
		if(classifyFilter->ds_action_field&CF_DS_ACTION_DROP_BIT)
			diag_util_printf("DS_ACTION_DROP \n");	
	}
#endif
	return CPARSER_OK;
}


/*
 * rg show acl-filter
 */
cparser_result_t
cparser_cmd_rg_show_acl_filter(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	_diag_rg_aclFilter_show_field(&acl_parameter);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_acl_filter */

/*
 * rg clear acl-filter
 */
cparser_result_t
cparser_cmd_rg_clear_acl_filter(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&acl_parameter, 0x0, sizeof(rtk_rg_aclFilterAndQos_t));
	diag_util_mprintf("acl-filter entry clear!\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_acl_filter */

/*
 * rg add acl-filter entry
 */
cparser_result_t
cparser_cmd_rg_add_acl_filter_entry(
    cparser_context_t *context)
{
	int ret=0;
	int acl_filter_idx=0;
	
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_aclFilterAndQos_add(&acl_parameter, &acl_filter_idx);
	if(ret == RT_ERR_RG_OK )
		diag_util_mprintf("add acl-filter entry[%d] success!\n",acl_filter_idx);
	else
		diag_util_mprintf("add acl-filter entry failed! (ret=%d) \n",ret);
	

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_acl_filter_entry */

/*
 * rg get acl-filter entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_get_acl_filter_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret=0;
	int search_start_idx = *entry_index_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	
	osal_memset(&acl_parameter, 0x0, sizeof(rtk_rg_aclFilterAndQos_t));
	ret = rtk_rg_aclFilterAndQos_find(&acl_parameter,&search_start_idx);
	if(ret == RT_ERR_RG_OK){
		if(search_start_idx == (*entry_index_ptr)){
			_diag_rg_aclFilter_show_field(&acl_parameter);
			diag_util_mprintf("get acl-filter entry[%d] success!\n",*entry_index_ptr);
		}else{
			diag_util_mprintf("get acl-filter entry[%d] failed! acl-filter entry[%d] is empty!\n",*entry_index_ptr);
		}
		
	}else{
		diag_util_mprintf("find acl-filter entry[%d] failed!\n",*entry_index_ptr);
	}

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_acl_filter_entry_entry_index */

/*
 * rg del acl-filter entry <UINT:entry_index>
 */
cparser_result_t
cparser_cmd_rg_del_acl_filter_entry_entry_index(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_aclFilterAndQos_del(*entry_index_ptr);

	if(ret == RT_ERR_RG_OK){
		diag_util_mprintf("del acl-filter entry[%d] success!\n",*entry_index_ptr);
	}else{
		diag_util_mprintf("del acl-filter entry[%d] failed!\n",*entry_index_ptr);
	}

	

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_acl_filter_entry_entry_index */

/*
 * rg set acl-filter fwding_type_and_direction <UINT:fwding_type_and_direction>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_fwding_type_and_direction_fwding_type_and_direction(
    cparser_context_t *context,
    uint32_t  *fwding_type_and_direction_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.fwding_type_and_direction=*fwding_type_and_direction_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_fwding_type_and_direction_fwding_type_and_direction */


/*
 * rg set acl-filter acl_weight <UINT:acl_weight>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_acl_weight_acl_weight(
    cparser_context_t *context,
    uint32_t  *acl_weight_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.acl_weight= *acl_weight_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_acl_weight_acl_weight */



/*
 * rg set acl-filter pattern ingress_port_mask <HEX:portmask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_port_mask_portmask(
    cparser_context_t *context,
    uint32_t  *portmask_ptr)
{
    DIAG_UTIL_PARAM_CHK();   
	acl_parameter.filter_fields |= INGRESS_PORT_BIT;
	acl_parameter.ingress_port_mask.portmask= *portmask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_port_mask_portmask */


/*
 * rg set acl-filter pattern ingress_dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscp_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_PARAM_RANGE_CHK((*dscp_ptr > RTK_VALUE_OF_DSCP_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= INGRESS_DSCP_BIT;	
	acl_parameter.ingress_dscp= *dscp_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_dscp_dscp */

/*
 * rg set acl-filter pattern ingress_ipv6_dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_ipv6_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscp_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*dscp_ptr > RTK_VALUE_OF_DSCP_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= INGRESS_IPV6_DSCP_BIT;	
	acl_parameter.ingress_ipv6_dscp= *dscp_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_ipv6_dscp_dscp */





/*
 * rg set acl-filter pattern ingress_intf_idx <UINT:igr_intf>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_intf_idx_igr_intf(
    cparser_context_t *context,
    uint32_t  *igr_intf_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_INTF_BIT;	
	acl_parameter.ingress_intf_idx= *igr_intf_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_intf_idx_igr_intf */

/*
 * rg set acl-filter pattern egress_intf_idx <UINT:egr_intf>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_intf_idx_egr_intf(
    cparser_context_t *context,
    uint32_t  *egr_intf_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.egress_intf_idx= *egr_intf_ptr;
	acl_parameter.filter_fields |= EGRESS_INTF_BIT;	

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_intf_idx_egr_intf */


/*
 * rg set acl-filter pattern ingress_stream_id <UINT:stream_id>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_stream_id_stream_id(
    cparser_context_t *context,
    uint32_t  *stream_id_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.ingress_stream_id= *stream_id_ptr;
	acl_parameter.filter_fields |= INGRESS_STREAM_ID_BIT;	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_stream_id_stream_id */

/*
 * rg set acl-filter pattern ingress_ipv4_tagif <UINT:ingress_ipv4_tagif>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_ipv4_tagif_ingress_ipv4_tagif(
    cparser_context_t *context,
    uint32_t  *ingress_ipv4_tagif_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.ingress_ipv4_tagif= *ingress_ipv4_tagif_ptr;
	acl_parameter.filter_fields |= INGRESS_IPV4_TAGIF_BIT;	

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_ipv4_tagif_ingress_ipv4_tagif */

/*
 * rg set acl-filter pattern ingress_ipv6_tagif <UINT:ingress_ipv6_tagif>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_ipv6_tagif_ingress_ipv6_tagif(
    cparser_context_t *context,
    uint32_t  *ingress_ipv6_tagif_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.ingress_ipv6_tagif= *ingress_ipv6_tagif_ptr;
	acl_parameter.filter_fields |= INGRESS_IPV6_TAGIF_BIT;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_ipv6_tagif_ingress_ipv6_tagif */


/*
 * rg set acl-filter pattern ingress_ethertype <HEX:ethtype>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_ethertype_ethtype(
    cparser_context_t *context,
    uint32_t  *ethtype_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*ethtype_ptr > RTK_ETHERTYPE_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= INGRESS_ETHERTYPE_BIT;	
	acl_parameter.ingress_ethertype= *ethtype_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_ethertype_ethtype */

/*
 * rg set acl-filter pattern ingress_ctag_vid <UINT:vid>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_ctag_vid_vid(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > RTK_VLAN_ID_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= INGRESS_CTAG_VID_BIT;
	acl_parameter.ingress_ctag_vid= *vid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_ctag_vid_vid */

/*
 * rg set acl-filter pattern ingress_ctag_pri <UINT:pri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_ctag_pri_pri(
    cparser_context_t *context,
    uint32_t  *pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > RTK_DOT1P_PRIORITY_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= INGRESS_CTAG_PRI_BIT;
	acl_parameter.ingress_ctag_pri= *pri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_ctag_pri_pri */

/*
 * rg set acl-filter pattern ingress_ctag_cfi <UINT:cfi>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_ctag_cfi_cfi(
    cparser_context_t *context,
    uint32_t  *cfi_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    acl_parameter.filter_fields |= INGRESS_CTAG_CFI_BIT;
	acl_parameter.ingress_ctag_cfi= *cfi_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_ctag_cfi_cfi */


/*
 * rg set acl-filter pattern egress_ctag_vid <UINT:vid>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_ctag_vid_vid(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > RTK_VLAN_ID_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= EGRESS_CTAG_VID_BIT;
	acl_parameter.egress_ctag_vid= *vid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_ctag_vid_vid */

/*
 * rg set acl-filter pattern egress_ctag_vid_mask <HEX:vid_mask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_ctag_vid_mask_vid_mask(
    cparser_context_t *context,
    uint32_t  *vid_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.egress_ctag_vid_mask= *vid_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_ctag_vid_mask_vid_mask */


/*
 * rg set acl-filter pattern egress_ctag_pri <UINT:pri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_ctag_pri_pri(
    cparser_context_t *context,
    uint32_t  *pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > RTK_DOT1P_PRIORITY_MAX), CPARSER_ERR_INVALID_PARAMS);
	acl_parameter.filter_fields |= EGRESS_CTAG_PRI_BIT;
	acl_parameter.egress_ctag_pri= *pri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_ctag_pri_pri */

/*
 * rg set acl-filter pattern egress_ip4mc_if <UINT:egress_ip4mc_if>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_ip4mc_if_egress_ip4mc_if(
    cparser_context_t *context,
    uint32_t  *egress_ip4mc_if_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= EGRESS_IP4MC_IF;
	acl_parameter.egress_ip4mc_if= *egress_ip4mc_if_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_ip4mc_if_egress_ip4mc_if */

/*
 * rg set acl-filter pattern egress_ip6mc_if <UINT:egress_ip6mc_if>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_ip6mc_if_egress_ip6mc_if(
    cparser_context_t *context,
    uint32_t  *egress_ip6mc_if_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= EGRESS_IP6MC_IF;
	acl_parameter.egress_ip6mc_if= *egress_ip6mc_if_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_ip6mc_if_egress_ip6mc_if */


/*
 * rg set acl-filter pattern ingress_smac <MACADDR:smac>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_smac_smac(
    cparser_context_t *context,
    cparser_macaddr_t  *smac_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_SMAC_BIT;
	osal_memcpy(&acl_parameter.ingress_smac.octet, smac_ptr->octet, ETHER_ADDR_LEN);	

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_smac_smac */

/*
 * rg set acl-filter pattern ingress_dmac <MACADDR:dmac>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_dmac_dmac(
    cparser_context_t *context,
    cparser_macaddr_t  *dmac_ptr)
{
    DIAG_UTIL_PARAM_CHK();	
	acl_parameter.filter_fields |= INGRESS_DMAC_BIT;
	osal_memcpy(&acl_parameter.ingress_dmac.octet, dmac_ptr->octet, ETHER_ADDR_LEN);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_dmac_dmac */

/*
 * rg set acl-filter pattern ingress_src_ipv4_addr_start <IPV4ADDR:igr_sip_low_bound> ingress_src_ipv4_addr_end <IPV4ADDR:igr_sip_up_bound>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_src_ipv4_addr_start_igr_sip_low_bound_ingress_src_ipv4_addr_end_igr_sip_up_bound(
    cparser_context_t *context,
    uint32_t  *igr_sip_low_bound_ptr,
    uint32_t  *igr_sip_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_IPV4_SIP_RANGE_BIT;
    acl_parameter.ingress_src_ipv4_addr_start = (ipaddr_t)*igr_sip_low_bound_ptr;
    acl_parameter.ingress_src_ipv4_addr_end = (ipaddr_t)*igr_sip_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_src_ipv4_addr_start_igr_sip_low_bound_ingress_src_ipv4_addr_end_igr_sip_up_bound */

/*
 * rg set acl-filter pattern ingress_dest_ipv4_addr_start  <IPV4ADDR:igr_dip_low_bound> ingress_dest_ipv4_addr_end  <IPV4ADDR:igr_dip_up_bound>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_dest_ipv4_addr_start_igr_dip_low_bound_ingress_dest_ipv4_addr_end_igr_dip_up_bound(
    cparser_context_t *context,
    uint32_t  *igr_dip_low_bound_ptr,
    uint32_t  *igr_dip_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_IPV4_DIP_RANGE_BIT;
	acl_parameter.ingress_dest_ipv4_addr_start = (ipaddr_t)*igr_dip_low_bound_ptr;
    acl_parameter.ingress_dest_ipv4_addr_end = (ipaddr_t)*igr_dip_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_dest_ipv4_addr_start_igr_dip_low_bound_ingress_dest_ipv4_addr_end_igr_dip_up_bound */

/*
 * rg set acl-filter pattern ingress_src_ipv6_addr_start  <IPV6ADDR:igr_sip6_low_bound> ingress_src_ipv6_addr_end <IPV6ADDR:igr_sip6_up_bound> 
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_src_ipv6_addr_start_igr_sip6_low_bound_ingress_src_ipv6_addr_end_igr_sip6_up_bound(
    cparser_context_t *context,
    char * *igr_sip6_low_bound_ptr,
    char * *igr_sip6_up_bound_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

	//FIX ME
	acl_parameter.filter_fields |= INGRESS_IPV6_SIP_RANGE_BIT;

 
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_src_ipv6_addr_start[0], TOKEN_STR(5)), ret);
  	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_src_ipv6_addr_end[0], TOKEN_STR(7)), ret);

	//osal_memcpy(&acl_parameter.ingress_src_ipv6_addr_start, *igr_sip6_low_bound_ptr, sizeof(uint8)*16);
	//osal_memcpy(&acl_parameter.ingress_src_ipv6_addr_end, *igr_sip6_up_bound_ptr, sizeof(uint8)*16);
	//diag_util_printf("[fix me]ipv6 address parse error in ipv6. \n");
	//return CPARSER_ERR_INVALID_PARAMS;


    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_src_ipv6_addr_start_igr_sip6_low_bound_ingress_src_ipv6_addr_end_igr_sip6_up_bound */


/*
 * rg set acl-filter pattern ingress_src_ipv6_addr  <IPV6ADDR:ingress_src_ipv6_addr>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_src_ipv6_addr_ingress_src_ipv6_addr(
    cparser_context_t *context,
    char * *ingress_src_ipv6_addr_ptr)
{
		int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_IPV6_SIP_BIT;

    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_src_ipv6_addr[0], TOKEN_STR(5)), ret);
 

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_src_ipv6_addr_ingress_src_ipv6_addr */

/*
 * rg set acl-filter pattern ingress_src_ipv6_addr_mask  <IPV6ADDR:ingress_src_ipv6_addr_mask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_src_ipv6_addr_mask_ingress_src_ipv6_addr_mask(
    cparser_context_t *context,
    char * *ingress_src_ipv6_addr_mask_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_src_ipv6_addr_mask[0], TOKEN_STR(5)), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_src_ipv6_addr_mask_ingress_src_ipv6_addr_mask */

/*
 * rg set acl-filter pattern ingress_dest_ipv6_addr_start <IPV6ADDR:igr_dip6_low_bound> ingress_dest_ipv6_addr_end <IPV6ADDR:igr_sip6_up_bound> 
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_dest_ipv6_addr_start_igr_dip6_low_bound_ingress_dest_ipv6_addr_end_igr_sip6_up_bound(
    cparser_context_t *context,
    char * *igr_dip6_low_bound_ptr,
    char * *igr_sip6_up_bound_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

	//FIX ME
	acl_parameter.filter_fields |= INGRESS_IPV6_DIP_RANGE_BIT;

	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_dest_ipv6_addr_start[0], TOKEN_STR(5)), ret);
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_dest_ipv6_addr_end[0], TOKEN_STR(7)), ret);

	//osal_memcpy(&acl_parameter.ingress_dest_ipv6_addr_start, *igr_dip6_low_bound_ptr, sizeof(uint8)*16);
	//osal_memcpy(&acl_parameter.ingress_dest_ipv6_addr_end, *igr_sip6_up_bound_ptr, sizeof(uint8)*16);
	//diag_util_printf("[fix me]ipv6 address parse error in ipv6. \n");
	//return CPARSER_ERR_INVALID_PARAMS;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_dest_ipv6_addr_start_igr_dip6_low_bound_ingress_dest_ipv6_addr_end_igr_sip6_up_bound */

/*
 * rg set acl-filter pattern ingress_dest_ipv6_addr  <IPV6ADDR:ingress_dest_ipv6_addr>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_dest_ipv6_addr_ingress_dest_ipv6_addr(
    cparser_context_t *context,
    char * *ingress_dest_ipv6_addr_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_IPV6_DIP_BIT;
	
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_dest_ipv6_addr[0], TOKEN_STR(5)), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_dest_ipv6_addr_ingress_dest_ipv6_addr */


/*
 * rg set acl-filter pattern ingress_dest_ipv6_addr_mask  <IPV6ADDR:ingress_dest_ipv6_addr_mask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_dest_ipv6_addr_mask_ingress_dest_ipv6_addr_mask(
    cparser_context_t *context,
    char * *ingress_dest_ipv6_addr_mask_ptr)
{
	int32  ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();
	DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&acl_parameter.ingress_dest_ipv6_addr_mask[0], TOKEN_STR(5)), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_dest_ipv6_addr_mask_ingress_dest_ipv6_addr_mask */




/*
 * rg set acl-filter pattern ingress_src_l4_port_start <UINT:igr_l4sport_low_bound> ingress_src_l4_port_end <UINT:igr_l4sport_up_bound>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_src_l4_port_start_igr_l4sport_low_bound_ingress_src_l4_port_end_igr_l4sport_up_bound(
    cparser_context_t *context,
    uint32_t  *igr_l4sport_low_bound_ptr,
    uint32_t  *igr_l4sport_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_L4_SPORT_RANGE_BIT;
	acl_parameter.ingress_src_l4_port_start = *igr_l4sport_low_bound_ptr;
	acl_parameter.ingress_src_l4_port_end = *igr_l4sport_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_src_l4_port_start_igr_l4sport_low_bound_ingress_src_l4_port_end_igr_l4sport_up_bound */

/*
 * rg set acl-filter pattern ingress_dest_l4_port_start <UINT:igr_l4dport_low_bound> ingress_dest_l4_port_end <UINT:igr_l4dport_up_bound>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_dest_l4_port_start_igr_l4dport_low_bound_ingress_dest_l4_port_end_igr_l4dport_up_bound(
    cparser_context_t *context,
    uint32_t  *igr_l4dport_low_bound_ptr,
    uint32_t  *igr_l4dport_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	acl_parameter.filter_fields |= INGRESS_L4_DPORT_RANGE_BIT;
	acl_parameter.ingress_dest_l4_port_start = *igr_l4dport_low_bound_ptr;
	acl_parameter.ingress_dest_l4_port_end = *igr_l4dport_up_bound_ptr;


    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_dest_l4_port_start_igr_l4dport_low_bound_ingress_dest_l4_port_end_igr_l4dport_up_bound */

/*
 * rg set acl-filter pattern egress_src_ipv4_addr_start <IPV4ADDR:egr_sip_low_bound> egress_src_ipv4_addr_end <IPV4ADDR:egr_sip_up_bound>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_src_ipv4_addr_start_egr_sip_low_bound_egress_src_ipv4_addr_end_egr_sip_up_bound(
    cparser_context_t *context,
    uint32_t  *egr_sip_low_bound_ptr,
    uint32_t  *egr_sip_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= EGRESS_IPV4_SIP_RANGE_BIT;
    acl_parameter.egress_src_ipv4_addr_start = (ipaddr_t)*egr_sip_low_bound_ptr;
    acl_parameter.egress_src_ipv4_addr_end = (ipaddr_t)*egr_sip_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_src_ipv4_addr_start_egr_sip_low_bound_egress_src_ipv4_addr_end_egr_sip_up_bound */

/*
 * rg set acl-filter pattern egress_dest_ipv4_addr_start <IPV4ADDR:egr_dip_low_bound> egress_dest_ipv4_addr_end <IPV4ADDR:egr_dip_up_bound>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_dest_ipv4_addr_start_egr_dip_low_bound_egress_dest_ipv4_addr_end_egr_dip_up_bound(
    cparser_context_t *context,
    uint32_t  *egr_dip_low_bound_ptr,
    uint32_t  *egr_dip_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= EGRESS_IPV4_DIP_RANGE_BIT;
	acl_parameter.egress_dest_ipv4_addr_start = (ipaddr_t)*egr_dip_low_bound_ptr;
    acl_parameter.egress_dest_ipv4_addr_end = (ipaddr_t)*egr_dip_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_dest_ipv4_addr_start_egr_dip_low_bound_egress_dest_ipv4_addr_end_egr_dip_up_bound */

/*
 * rg set acl-filter pattern egress_src_l4_port_start <UINT:egr_l4sport_low_bound> egress_src_l4_port_end <UINT:egr_l4sport_up_bound>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_src_l4_port_start_egr_l4sport_low_bound_egress_src_l4_port_end_egr_l4sport_up_bound(
    cparser_context_t *context,
    uint32_t  *egr_l4sport_low_bound_ptr,
    uint32_t  *egr_l4sport_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= EGRESS_L4_SPORT_RANGE_BIT;
	acl_parameter.egress_src_l4_port_start = *egr_l4sport_low_bound_ptr;
	acl_parameter.egress_src_l4_port_end = *egr_l4sport_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_src_l4_port_start_egr_l4sport_low_bound_egress_src_l4_port_end_egr_l4sport_up_bound */

/*
 * rg set acl-filter pattern egress_dest_l4_port_start <UINT:egr_l4dport_low_bound> egress_dest_l4_port_end <UINT:igr_l4dport_up_bound>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_dest_l4_port_start_egr_l4dport_low_bound_egress_dest_l4_port_end_igr_l4dport_up_bound(
    cparser_context_t *context,
    uint32_t  *egr_l4dport_low_bound_ptr,
    uint32_t  *igr_l4dport_up_bound_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= EGRESS_L4_DPORT_RANGE_BIT;
	acl_parameter.egress_dest_l4_port_start = *egr_l4dport_low_bound_ptr;
	acl_parameter.egress_dest_l4_port_end = *igr_l4dport_up_bound_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_dest_l4_port_start_egr_l4dport_low_bound_egress_dest_l4_port_end_igr_l4dport_up_bound */

/*
 * rg set acl-filter pattern ingress_l4_protocal <UINT:protocal>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_l4_protocal_protocal(
    cparser_context_t *context,
    uint32_t  *protocal_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(*protocal_ptr==0){
		acl_parameter.filter_fields |= INGRESS_L4_UDP_BIT;
	}else if(*protocal_ptr==1){
		acl_parameter.filter_fields |= INGRESS_L4_TCP_BIT;
	}else if(*protocal_ptr==2){
		acl_parameter.filter_fields |= INGRESS_L4_ICMP_BIT;
	}else{
		diag_util_printf("no such l4-protocal type!");
		return CPARSER_ERR_INVALID_PARAMS;
	}

	return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_l4_protocal_protocal */

/*
 * rg set acl-filter pattern ingress_l4_protocal_value <HEX:protocal>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_l4_protocal_value_protocal(
    cparser_context_t *context,
    uint32_t  *protocal_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_L4_POROTCAL_VALUE_BIT;
	acl_parameter.ingress_l4_protocal = *protocal_ptr;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_l4_protocal_value_protocal */


/*
 * rg set acl-filter pattern ingress_port_idx <UINT:ingress_port_idx>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_port_idx_ingress_port_idx(
    cparser_context_t *context,
    uint32_t  *ingress_port_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if( acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET ||
    	acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP || 
    	acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
    	)
	{
		diag_util_printf("[ERROR!]acl fwding_type_and_direction type must be ACL_ACTION_TYPE_L34_UP! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	acl_parameter.filter_fields |= INGRESS_EGRESS_PORTIDX_BIT;
	acl_parameter.ingress_port_idx= *ingress_port_idx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_port_idx_ingress_port_idx */


/*
 * rg set acl-filter pattern egress_port_idx <UINT:egress_port_idx>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_port_idx_egress_port_idx(
    cparser_context_t *context,
    uint32_t  *egress_port_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if( acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET ||
    	acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || 
    	acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN
    	)
	{
		diag_util_printf("[ERROR!]acl fwding_type_and_direction type must be ACL_ACTION_TYPE_L34_DOWN! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	acl_parameter.filter_fields |= INGRESS_EGRESS_PORTIDX_BIT;
	acl_parameter.egress_port_idx= *egress_port_idx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_port_idx_egress_port_idx */

/*
 * rg set acl-filter pattern internal_pri <UINT:internal_pri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_internal_pri_internal_pri(
    cparser_context_t *context,
    uint32_t  *internal_pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INTERNAL_PRI_BIT;
	acl_parameter.internal_pri= *internal_pri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_internal_pri_internal_pri */

/*
 * rg set acl-filter pattern ingress_stag_vid <UINT:ingress_stag_vid>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_stag_vid_ingress_stag_vid(
    cparser_context_t *context,
    uint32_t  *ingress_stag_vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_STAG_VID_BIT;
	acl_parameter.ingress_stag_vid= *ingress_stag_vid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_stag_vid_ingress_stag_vid */

/*
 * rg set acl-filter pattern ingress_stag_pri <UINT:ingress_stag_pri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_stag_pri_ingress_stag_pri(
    cparser_context_t *context,
    uint32_t  *ingress_stag_pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_STAG_PRI_BIT;
	acl_parameter.ingress_stag_pri= *ingress_stag_pri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_stag_pri_ingress_stag_pri */

/*
 * rg set acl-filter pattern ingress_stag_dei <UINT:ingress_stag_dei>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_stag_dei_ingress_stag_dei(
    cparser_context_t *context,
    uint32_t  *ingress_stag_dei_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_STAG_DEI_BIT;
	acl_parameter.ingress_stag_dei= *ingress_stag_dei_ptr;
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_stag_dei_ingress_stag_dei */

/*
 * rg set acl-filter pattern ingress_stagIf <UINT:ingress_stagIf>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_stagIf_ingress_stagIf(
    cparser_context_t *context,
    uint32_t  *ingress_stagIf_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_STAGIF_BIT;
	acl_parameter.ingress_stagIf= *ingress_stagIf_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_stagif_ingress_stagif */

/*
 * rg set acl-filter pattern ingress_ctagIf <UINT:ingress_ctagIf>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_ctagIf_ingress_ctagIf(
    cparser_context_t *context,
    uint32_t  *ingress_ctagIf_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_CTAGIF_BIT;
	acl_parameter.ingress_ctagIf= *ingress_ctagIf_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_ctagif_ingress_ctagif */

/*
 * rg set acl-filter pattern ingress_wlanDevMask <UINT:ingress_wlanDevMask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_wlanDevMask_ingress_wlanDevMask(
    cparser_context_t *context,
    uint32_t  *ingress_wlanDevMask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.filter_fields |= INGRESS_WLANDEV_BIT;
	acl_parameter.ingress_wlanDevMask= *ingress_wlanDevMask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_wlandevmask_ingress_wlandevmask */

/*
 * rg set acl-filter pattern ingress_smac_mask <MACADDR:smac_mask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_smac_mask_smac_mask(
    cparser_context_t *context,
    cparser_macaddr_t  *smac_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memcpy(&acl_parameter.ingress_smac_mask.octet, smac_mask_ptr->octet, ETHER_ADDR_LEN);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_smac_mask_smac_mask */

/*
 * rg set acl-filter pattern ingress_dmac_mask <MACADDR:dmac_mask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_dmac_mask_dmac_mask(
    cparser_context_t *context,
    cparser_macaddr_t  *dmac_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memcpy(&acl_parameter.ingress_dmac_mask.octet, dmac_mask_ptr->octet, ETHER_ADDR_LEN);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_dmac_mask_dmac_mask */

/*
 * rg set acl-filter pattern ingress_ethertype_mask <HEX:ethtype_mask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_ethertype_mask_ethtype_mask(
    cparser_context_t *context,
    uint32_t  *ethtype_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(*ethtype_mask_ptr > 0xffff)
		diag_util_printf("ingress_ethertype_mask have 16-bits only \n");
	acl_parameter.ingress_ethertype_mask = *ethtype_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_ethertype_mask_ethtype

/*
 * rg set acl-filter pattern ingress_stream_id_mask <HEX:stream_id_mask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_stream_id_mask_stream_id_mask(
    cparser_context_t *context,
    uint32_t  *stream_id_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(*stream_id_mask_ptr > 0xffff)
		diag_util_printf("egress_stream_id_mask have 16-bits only \n");
	acl_parameter.ingress_stream_id_mask = *stream_id_mask_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_stream_id_mask_stream_

/*
 * rg set acl-filter pattern ingress_port_idx_mask <HEX:ingress_port_idx_mask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_ingress_port_idx_mask_ingress_port_idx_mask(
    cparser_context_t *context,
    uint32_t  *ingress_port_idx_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
     if( acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET ||
    	acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_DROP || 
    	acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN
    	)
	{
		diag_util_printf("[ERROR!]acl fwding_type_and_direction type must be ACL_ACTION_TYPE_L34_UP! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	if(*ingress_port_idx_mask_ptr > 0x7)
		diag_util_printf("ingress_port_idx_mask have 3-bits only \n");
	acl_parameter.ingress_port_idx_mask= *ingress_port_idx_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_ingress_port_idx_mask_ingress_port_idx_mask */

/*
 * rg set acl-filter pattern egress_port_idx_mask <HEX:egress_port_idx_mask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_pattern_egress_port_idx_mask_egress_port_idx_mask(
    cparser_context_t *context,
    uint32_t  *egress_port_idx_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if( acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_ALL_PACKET ||
    	acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_DROP || 
    	acl_parameter.fwding_type_and_direction == ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_UP_STREAMID_CVLAN_SVLAN
    	)
	{
		diag_util_printf("[ERROR!]acl fwding_type_and_direction type must be ACL_ACTION_TYPE_L34_DOWN! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	if(*egress_port_idx_mask_ptr > 0x7)
		diag_util_printf("egress_port_idx_mask have 3-bits only \n");
	acl_parameter.ingress_port_idx_mask= *egress_port_idx_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_pattern_egress_port_idx_mask_egress_port_idx_mask */

/*
 * rg set acl-filter action action_type <UINT:actiontype>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_action_type_actiontype(
    cparser_context_t *context,
    uint32_t  *actiontype_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	acl_parameter.action_type = *actiontype_ptr;
    return CPARSER_OK;
	
}    /* end of cparser_cmd_rg_set_acl_filter_action_action_type_actiontype */

/*
 * rg set acl-filter action policy-route egress_intf_idx <UINT:egr_intf>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_policy_route_egress_intf_idx_egr_intf(
    cparser_context_t *context,
    uint32_t  *egr_intf_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_POLICY_ROUTE)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_POLICY_ROUTE! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
		acl_parameter.action_policy_route_wan = *egr_intf_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_policy_route_egress_intf_idx_egr_intf */

/*
 * rg set acl-filter action qos action_dot1p_remarking_pri <UINT:dot1p_remarking_pri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_dot1p_remarking_pri_dot1p_remarking_pri(
    cparser_context_t *context,
    uint32_t  *dot1p_remarking_pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		DIAG_UTIL_PARAM_RANGE_CHK((*dot1p_remarking_pri_ptr > RTK_DOT1P_PRIORITY_MAX), CPARSER_ERR_INVALID_PARAMS);
		acl_parameter.qos_actions |= ACL_ACTION_1P_REMARKING_BIT;
		acl_parameter.action_dot1p_remarking_pri = *dot1p_remarking_pri_ptr;
	}
		
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_dot1p_remarking_pri_dot1p_remarking_pri */

/*
 * rg set acl-filter action qos action_ip_precedence_remarking_pri  <UINT:ip_precedence_remarking_pri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_ip_precedence_remarking_pri_ip_precedence_remarking_pri(
    cparser_context_t *context,
    uint32_t  *ip_precedence_remarking_pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();

	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		DIAG_UTIL_PARAM_RANGE_CHK((*ip_precedence_remarking_pri_ptr > (RTK_VALUE_OF_DSCP_MAX>>3)), CPARSER_ERR_INVALID_PARAMS);
		acl_parameter.qos_actions |= ACL_ACTION_IP_PRECEDENCE_REMARKING_BIT;
		acl_parameter.action_ip_precedence_remarking_pri = *ip_precedence_remarking_pri_ptr;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_ip_precedence_remarking_pri_ip_precedence_remarking_pri */

/*
 * rg set acl-filter action qos action_dscp_remarking_pri <UINT:dscp_remarking_pri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_dscp_remarking_pri_dscp_remarking_pri(
    cparser_context_t *context,
    uint32_t  *dscp_remarking_pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		DIAG_UTIL_PARAM_RANGE_CHK((*dscp_remarking_pri_ptr > RTK_VALUE_OF_DSCP_MAX), CPARSER_ERR_INVALID_PARAMS);
		acl_parameter.qos_actions |= ACL_ACTION_DSCP_REMARKING_BIT;
		acl_parameter.action_dscp_remarking_pri = *dscp_remarking_pri_ptr;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_dscp_remarking_pri_dscp_remarking_pri */

/*
 * rg set acl-filter action qos action_queue_id <UINT:queue_id>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_queue_id_queue_id(
    cparser_context_t *context,
    uint32_t  *queue_id_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		DIAG_UTIL_PARAM_RANGE_CHK((*queue_id_ptr > 7), CPARSER_ERR_INVALID_PARAMS);
		acl_parameter.qos_actions |= ACL_ACTION_QUEUE_ID_BIT;
		acl_parameter.action_queue_id = *queue_id_ptr;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_queue_id_queue_id */

/*
 * rg set acl-filter action qos action_share_meter <UINT:share_meter>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_share_meter_share_meter(
    cparser_context_t *context,
    uint32_t  *share_meter_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		DIAG_UTIL_PARAM_RANGE_CHK((*share_meter_ptr >= HAL_MAX_NUM_OF_METERING()), CPARSER_ERR_INVALID_PARAMS);
		acl_parameter.qos_actions |= ACL_ACTION_SHARE_METER_BIT;
		acl_parameter.action_share_meter = *share_meter_ptr;
	}
	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_share_meter_share_meter */


/*
 * rg set acl-filter action qos action_egress_internal_priority <UINT:egress_internal_priority>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_egress_internal_priority_egress_internal_priority(
    cparser_context_t *context,
    uint32_t  *egress_internal_priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_EGRESS_INTERNAL_PRIORITY_BIT;
		acl_parameter.egress_internal_priority = *egress_internal_priority_ptr;
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_egress_internal_priority_egress_internal_priority */


/*
 * rg set acl-filter action qos action_stream_id <UINT:stream_id>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_stream_id_stream_id(
    cparser_context_t *context,
    uint32_t  *stream_id_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		DIAG_UTIL_PARAM_RANGE_CHK((*stream_id_ptr >= HAL_CLASSIFY_SID_NUM()), CPARSER_ERR_INVALID_PARAMS);
		acl_parameter.qos_actions |= ACL_ACTION_STREAM_ID_OR_LLID_BIT;
		acl_parameter.action_stream_id_or_llid = *stream_id_ptr;
	}
	
    return CPARSER_OK;

}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_stream_id_stream_id */

/*
 * rg set acl-filter action qos action_acl_priority <UINT:priority> */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_acl_priority_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		DIAG_UTIL_PARAM_RANGE_CHK((*priority_ptr > RTK_DOT1P_PRIORITY_MAX), CPARSER_ERR_INVALID_PARAMS);
		acl_parameter.qos_actions |= ACL_ACTION_ACL_PRIORITY_BIT;
		acl_parameter.action_acl_priority = *priority_ptr;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_acl_priority_priority */


/*
 * rg set acl-filter action qos action_redirect <UINT:redirect_portmask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_redirect_redirect_portmask(
    cparser_context_t *context,
    uint32_t  *redirect_portmask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_REDIRECT_BIT;
		acl_parameter.redirect_portmask = *redirect_portmask_ptr;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_redirect_redirect_portmask */

#if 1  //RTL9602C SERIES CF ACTIONS

/*
 * rg set acl-filter action qos action_stag tagDecision <UINT:tagDecision> svidDecision <UINT:svidDecision> spriDecision <UINT:spriDecision> svid <UINT:svid> spri <UINT:spri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_stag_tagDecision_tagDecision_svidDecision_svidDecision_spriDecision_spriDecision_svid_svid_spri_spri(
    cparser_context_t *context,
    uint32_t  *tagDecision_ptr,
    uint32_t  *svidDecision_ptr,
    uint32_t  *spriDecision_ptr,
    uint32_t  *svid_ptr,
    uint32_t  *spri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_SVLANTAG_BIT;
		switch(*tagDecision_ptr){
			case 0: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_NOP; break;
			case 1: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID; break;
			case 2: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2; break;
			case 3: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_UNTAG; break;
			case 4: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TRANSPARENT; break;
			case 5: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID; break;
			default:
				diag_util_printf("[ERROR!]invalid tagDecision parameter.\n");
				return CPARSER_ERR_INVALID_PARAMS;
				break;
		}

		switch(*svidDecision_ptr){
			case 0: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_NOP; break;
			case 1: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_ASSIGN; break;
			case 2: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_COPY_FROM_1ST_TAG; break;
			case 3: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_COPY_FROM_2ND_TAG; break;
			default:
				diag_util_printf("[ERROR!]invalid svidDecision parameter.\n");
				return CPARSER_ERR_INVALID_PARAMS;
				break;	
		}
		switch(*spriDecision_ptr){
			case 0: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_NOP; break;
			case 1: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_ASSIGN; break;
			case 2: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG; break;
			case 3: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG; break;
			case 4: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI; break;
			default:
				diag_util_printf("[ERROR!]invalid spriDecision parameter.\n");
				return CPARSER_ERR_INVALID_PARAMS;
				break;				
		}
	
		acl_parameter.action_acl_svlan.assignedSvid = *svid_ptr;
		acl_parameter.action_acl_svlan.assignedSpri = *spri_ptr;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_stag_tagdecision_tagdecision_sviddecision_sviddecision_spridecision_spridecision_svid_svid_spri_spri */

/*
 * rg set acl-filter action qos action_ctag tagDecision <UINT:tagDecision> cvidDecision <UINT:cvidDecision> cpriDecision <UINT:cpriDecision> cvid <UINT:cvid> cpri <UINT:cpri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_tagDecision_tagDecision_cvidDecision_cvidDecision_cpriDecision_cpriDecision_cvid_cvid_cpri_cpri(
    cparser_context_t *context,
    uint32_t  *tagDecision_ptr,
    uint32_t  *cvidDecision_ptr,
    uint32_t  *cpriDecision_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *cpri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	
	acl_parameter.qos_actions |= ACL_ACTION_ACL_CVLANTAG_BIT;

	switch(*tagDecision_ptr){
		case 0: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_NOP; break;
		case 1: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_TAGGING; break;
		case 2: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_UNTAG; break;		
		case 3: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_TRANSPARENT; break;
		default:
			diag_util_printf("[ERROR!]invalid tagDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;	
	}
	
	switch(*cvidDecision_ptr){
		case 0: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_NOP; break;
		case 1: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_ASSIGN; break;
		case 2: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_COPY_FROM_1ST_TAG; break;		
		case 3: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_COPY_FROM_2ND_TAG; break;
		case 4: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID; break;
		default:
			diag_util_printf("[ERROR!]invalid cvidDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;
	}
	
	switch(*cpriDecision_ptr){
		case 0: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_NOP; break;
		case 1: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_ASSIGN; break;
		case 2: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG; break;
		case 3: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG; break;
		case 4: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI; break;
		default:
			diag_util_printf("[ERROR!]invalid cvidDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;
	}

	acl_parameter.action_acl_cvlan.assignedCvid = *cvid_ptr;
	acl_parameter.action_acl_cvlan.assignedCpri = *cpri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_tagdecision_tagdecision_cviddecision_cviddecision_cpridecision_cpridecision_cvid_cvid_cpri_cpri */

#endif
#if 1 //RTL9600 SERIES CF ACTIONS
/*
 * rg set acl-filter action qos action_stag tagWithTPID svidDecision <UINT:svidDecision> spriDecision <UINT:spriDecision> svid <UINT:svid> spri <UINT:spri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_stag_tagWithTPID_svidDecision_svidDecision_spriDecision_spriDecision_svid_svid_spri_spri(
    cparser_context_t *context,
    uint32_t  *svidDecision_ptr,
    uint32_t  *spriDecision_ptr,
    uint32_t  *svid_ptr,
    uint32_t  *spri_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_SVLANTAG_BIT;
		acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID;
		acl_parameter.action_acl_svlan.svlanSvidDecision = *svidDecision_ptr;
		acl_parameter.action_acl_svlan.svlanSpriDecision = *spriDecision_ptr;
		acl_parameter.action_acl_svlan.assignedSvid = *svid_ptr;
		acl_parameter.action_acl_svlan.assignedSpri = *spri_ptr;
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_stag_tagwithtpid_sviddecision_sviddecision_spridecision_spridecision_svid_svid_spri_spri */

/*
 * rg set acl-filter action qos action_stag tagWith8100 svidDecision <UINT:svidDecision> spriDecision <UINT:spriDecision> svid <UINT:svid> spri <UINT:spri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_stag_tagWith8100_svidDecision_svidDecision_spriDecision_spriDecision_svid_svid_spri_spri(
    cparser_context_t *context,
    uint32_t  *svidDecision_ptr,
    uint32_t  *spriDecision_ptr,
    uint32_t  *svid_ptr,
    uint32_t  *spri_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
	diag_util_printf("[NOT SUPPORT] please use:  rg set acl-filter action qos action_stag tagDecision XXX \n");
#else
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_SVLANTAG_BIT;
		acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_8100;
		acl_parameter.action_acl_svlan.svlanSvidDecision = *svidDecision_ptr;
		acl_parameter.action_acl_svlan.svlanSpriDecision = *spriDecision_ptr;
		acl_parameter.action_acl_svlan.assignedSvid = *svid_ptr;
		acl_parameter.action_acl_svlan.assignedSpri = *spri_ptr;
	}
#endif
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_stag_tagwith8100_sviddecision_sviddecision_spridecision_spridecision_svid_svid_spri_spri */

/*
 * rg set acl-filter action qos action_stag tagWithSP2C
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_stag_tagWithSP2C(
    cparser_context_t *context)
{
#if defined(CONFIG_RTL9602C_SERIES)
	diag_util_printf("[WARNING!] please use:  rg set acl-filter action qos action_stag tagDecision XXX \n");
#else
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_SVLANTAG_BIT;
		acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C;
	}
#endif
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_stag_tagwithsp2c */

/*
 * rg set acl-filter action qos action_stag untag
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_stag_untag(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_SVLANTAG_BIT;
		acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_UNTAG;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_stag_untag */

/*
 * rg set acl-filter action qos action_stag transparent
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_stag_transparent(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_SVLANTAG_BIT;
		acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TRANSPARENT;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_stag_transparent */

/*
 * rg set acl-filter action qos action_stag nop
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_stag_nop(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_SVLANTAG_BIT;
		acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_NOP;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_stag_nop */


/*
 * rg set acl-filter action qos action_ctag tagging cvidDecision <UINT:cvidDecision> cpriDecision <UINT:cpriDecision> cvid <UINT:cvid> cpri <UINT:cpri>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_tagging_cvidDecision_cvidDecision_cpriDecision_cpriDecision_cvid_cvid_cpri_cpri(
    cparser_context_t *context,
    uint32_t  *cvidDecision_ptr,
    uint32_t  *cpriDecision_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *cpri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_CVLANTAG_BIT;
		acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_TAGGING;
		acl_parameter.action_acl_cvlan.cvlanCvidDecision = *cvidDecision_ptr;
		acl_parameter.action_acl_cvlan.cvlanCpriDecision = *cpriDecision_ptr;
		acl_parameter.action_acl_cvlan.assignedCvid = *cvid_ptr;
		acl_parameter.action_acl_cvlan.assignedCpri = *cpri_ptr;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_tagging_cviddecision_cviddecision_cpridecision_cpridecision_cvid_cvid_cpri_cpri */

/*
 * rg set acl-filter action qos action_ctag tagWithC2S
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_tagWithC2S(
    cparser_context_t *context)
{
#if defined(CONFIG_RTL9602C_SERIES)
	diag_util_printf("[NOT SUPPORT!] please use:  rg set acl-filter action qos action_ctag tagDecision XXX \n");
#else
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_CVLANTAG_BIT;
		acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_TAGGING_WITH_C2S;
	}
#endif
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_tagwithc2s */

/*
 * rg set acl-filter action qos action_ctag tagWithSP2C
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_tagWithSP2C(
    cparser_context_t *context)
{
#if defined(CONFIG_RTL9602C_SERIES)
	diag_util_printf("[NOT SUPPORT!] please use:  rg set acl-filter action qos action_ctag tagDecision XXX \n");
#else
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_CVLANTAG_BIT;
		acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C;
	}
#endif
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_tagwithsp2c */

/*
 * rg set acl-filter action qos action_ctag untag
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_untag(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_CVLANTAG_BIT;
		acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_UNTAG;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_untag */

/*
 * rg set acl-filter action qos action_ctag transparent
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_transparent(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_CVLANTAG_BIT;
		acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_TRANSPARENT;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_transparent */

/*
 * rg set acl-filter action qos action_ctag nop
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_nop(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_CVLANTAG_BIT;
		acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_NOP;
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_ctag_nop */

#endif

/*
 * rg set acl-filter action qos action_ingress_vid <UINT:ingress_vid>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_ingress_vid_ingress_vid(
    cparser_context_t *context,
    uint32_t  *ingress_vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.action_type != ACL_ACTION_TYPE_QOS)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_ACTION_TYPE_QOS! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	else
	{
		acl_parameter.qos_actions |= ACL_ACTION_ACL_INGRESS_VID_BIT;
		acl_parameter.action_acl_ingress_vid=*ingress_vid_ptr;
	}
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_action_ingress_vid_ingress_vi


/*
 * rg set acl-filter action qos action_downstream_uni_portmask portmask <HEX:portmask>
 */
cparser_result_t
cparser_cmd_rg_set_acl_filter_action_qos_action_downstream_uni_portmask_portmask_portmask(
    cparser_context_t *context,
    uint32_t  *portmask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	if(acl_parameter.fwding_type_and_direction != ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN)
	{
		diag_util_printf("[ERROR!]acl action type must be ACL_FWD_TYPE_DIR_INGRESS_OR_EGRESS_L34_DOWN_CVLAN_SVLAN! \n");
		return CPARSER_ERR_INVALID_PARAMS;
	}
	if(*portmask_ptr!=0)
	{
		acl_parameter.qos_actions |= ACL_ACTION_DS_UNIMASK_BIT;
		acl_parameter.downstream_uni_portmask=(*portmask_ptr & 0xf);	//only allow in LAN
	}	
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_acl_filter_action_qos_downstream_uni_portmask_portmask_portmask */


//
//classify-filter
//

/*
 * rg show classify-filter
 */
cparser_result_t
cparser_cmd_rg_show_classify_filter(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	_diag_rg_classifyFilter_show_field(&classifyFilter);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_show_classify_filter */

/*
 * rg clear classify-filter
 */
cparser_result_t
cparser_cmd_rg_clear_classify_filter(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	osal_memset(&classifyFilter, 0x0, sizeof(rtk_rg_classifyEntry_t));
	diag_util_mprintf("classify-filter entry clear!\n");

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_clear_classify_filter */

/*
 * rg add classify-filter entry
 */
cparser_result_t
cparser_cmd_rg_add_classify_filter_entry(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
	int ret=0;
	int classify_filter_idx=0;
	
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_classifyEntry_add(&classifyFilter);
	if(ret == RT_ERR_RG_OK )
		diag_util_mprintf("add classify-filter entry[%d] success!\n",classifyFilter.index);
	else
		diag_util_mprintf("add classify-filter entry failed! (ret=%d) \n",ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_add_classify_filter_entry */


/*
 * rg get classify-filter entry <UINT:index>
 */
cparser_result_t
cparser_cmd_rg_get_classify_filter_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int ret=0;
	int search_start_idx = *index_ptr;
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

	osal_memset(&classifyFilter, 0x0, sizeof(rtk_rg_classifyEntry_t));
	ret = rtk_rg_classifyEntry_find(search_start_idx,&classifyFilter);
	if(ret == RT_ERR_RG_OK){
		if(search_start_idx == (*index_ptr)){
			_diag_rg_classifyFilter_show_field(&classifyFilter);			
			diag_util_mprintf("get classify-filter entry[%d] success!\n",*index_ptr);
		}else{
			diag_util_mprintf("get classify-filter entry[%d] failed! acl-filter entry[%d] is empty!\n",*index_ptr);
		}
		
	}else{
		diag_util_mprintf("find classify-filter entry[%d] failed!\n",*index_ptr);
	}

    diag_util_mprintf("");

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_get_classify_filter_entry_index */

/*
 * rg del classify-filter entry <UINT:index>
 */
cparser_result_t
cparser_cmd_rg_del_classify_filter_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
	int ret;
    DIAG_UTIL_PARAM_CHK();
	ret = rtk_rg_classifyEntry_del(*index_ptr);

	if(ret == RT_ERR_RG_OK){
		diag_util_mprintf("del classify-filter entry[%d] success!\n",*index_ptr);
	}else{
		diag_util_mprintf("del classify-filter entry[%d] failed!\n",*index_ptr);
	}

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_del_classify_filter_entry_index */

/*
 * rg set classify-filter entry <UINT:index>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.index=*index_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_entry_index */


/*
 * rg set classify-filter pattern etherType <HEX:etherType>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_etherType_etherType(
    cparser_context_t *context,
    uint32_t  *etherType_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.filter_fields |= EGRESS_ETHERTYPR_BIT;
	classifyFilter.etherType=*etherType_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_ethertype_ethertype */

/*
 * rg set classify-filter pattern gemidx <UINT:gemidx>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_gemidx_gemidx(
    cparser_context_t *context,
    uint32_t  *gemidx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.filter_fields |= EGRESS_GEMIDX_BIT;
	classifyFilter.gemidx=*gemidx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_gemidx_gemidx */

/*
 * rg set classify-filter pattern llid <UINT:llid>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_llid_llid(
    cparser_context_t *context,
    uint32_t  *llid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.filter_fields |= EGRESS_LLID_BIT;
	classifyFilter.llid=*llid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_llid_llid */

/*
 * rg set classify-filter pattern outterTagVid <UINT:outterTagVid>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_outterTagVid_outterTagVid(
    cparser_context_t *context,
    uint32_t  *outterTagVid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.filter_fields |= EGRESS_TAGVID_BIT;
	classifyFilter.outterTagVid=*outterTagVid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_outtertagvid_outtertagvid */

/*
 * rg set classify-filter pattern outterTagPri <UINT:outterTagPri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_outterTagPri_outterTagPri(
    cparser_context_t *context,
    uint32_t  *outterTagPri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.filter_fields |= EGRESS_TAGPRI_BIT;
	classifyFilter.outterTagPri=*outterTagPri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_outtertagpri_outtertagpri */

/*
 * rg set classify-filter pattern internalPri <UINT:internalPri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_internalPri_internalPri(
    cparser_context_t *context,
    uint32_t  *internalPri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.filter_fields |= EGRESS_INTERNALPRI_BIT;
	classifyFilter.internalPri=*internalPri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_internalpri_internalpri */

/*
 * rg set classify-filter pattern stagIf <UINT:stagIf>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_stagIf_stagIf(
    cparser_context_t *context,
    uint32_t  *stagIf_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.filter_fields |= EGRESS_STAGIF_BIT;
	classifyFilter.stagIf=*stagIf_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_stagif_stagif */

/*
 * rg set classify-filter pattern ctagIf <UINT:ctagIf>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_ctagIf_ctagIf(
    cparser_context_t *context,
    uint32_t  *ctagIf_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.filter_fields |= EGRESS_CTAGIF_BIT;
	classifyFilter.ctagIf=*ctagIf_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_ctagif_ctagif */

/*
 * rg set classify-filter pattern uni <UINT:uni>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_uni_uni(
    cparser_context_t *context,
    uint32_t  *uni_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.filter_fields |= EGRESS_UNI_BIT;
	classifyFilter.uni=*uni_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_uni_uni */


/*
 * rg set classify-filter pattern etherType_mask <HEX:etherType_mask>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_etherType_mask_etherType_mask(
    cparser_context_t *context,
    uint32_t  *etherType_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.etherType_mask=*etherType_mask_ptr;
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_ethertype_mask_ethertype_

/*
 * rg set classify-filter pattern gemidx_mask <HEX:gemidx_mask>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_gemidx_mask_gemidx_mask(
    cparser_context_t *context,
    uint32_t  *gemidx_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.gemidx_mask=*gemidx_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_gemidx_mask_gemidx_mask *

/*
 * rg set classify-filter pattern uni_mask <UINT:uni_mask>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_pattern_uni_mask_uni_mask(
    cparser_context_t *context,
    uint32_t  *uni_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.uni_mask=*uni_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_pattern_uni_mask_uni_mask */


#if 1  //RTL9602C SERIES CF ACTIONS
/*
 * rg set classify-filter action direction upstream action_stag tagDecision <UINT:ta
gDecision> svidDecision <UINT:svidDecision> spriDecision <UINT:spriDecision> svid <UINT:svid> spri <UINT:spri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_stag_tagDecision_tagDecision_svidDecision_svidDecision_spriDecision_spriDecision_svid_svid_spri_spri(
    cparser_context_t *context,
    uint32_t  *tagDecision_ptr,
    uint32_t  *svidDecision_ptr,
    uint32_t  *spriDecision_ptr,
    uint32_t  *svid_ptr,
    uint32_t  *spri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_STAG_BIT;

	switch(*tagDecision_ptr){
		case 0: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_NOP; break;
		case 1: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID; break;
		case 2: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2; break;
		case 3: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_UNTAG; break;
		case 4: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TRANSPARENT; break;
		case 5: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID; break;
		default:
			diag_util_printf("[ERROR!]invalid tagDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;
	}
	
	switch(*svidDecision_ptr){
		case 0: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_NOP; break;
		case 1: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_ASSIGN; break;
		case 2: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_COPY_FROM_1ST_TAG; break;
		case 3: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_COPY_FROM_2ND_TAG; break;
		default:
			diag_util_printf("[ERROR!]invalid svidDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;	
	}
	switch(*spriDecision_ptr){
		case 0: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_NOP; break;
		case 1: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_ASSIGN; break;
		case 2: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG; break;
		case 3: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG; break;
		case 4: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI; break;
		default:
			diag_util_printf("[ERROR!]invalid spriDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;				
	}

	classifyFilter.action_svlan.assignedSvid = *svid_ptr;
	classifyFilter.action_svlan.assignedSpri = *spri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_stag_tagdecision_tagdecision_sviddecision_sviddecision_spridecision_spridecision_svid_svid_spri_spri */

/*
 * rg set classify-filter action direction upstream action_ctag tagDecision <UINT:tagDecision> cvidDecision <UINT:cvidDecision> cpriDecision <UINT:cpriDecision> cvid <UINT:cvid> cpri <UINT:cpri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_ctag_tagDecision_tagDecision_cvidDecision_cvidDecision_cpriDecision_cpriDecision_cvid_cvid_cpri_cpri(
    cparser_context_t *context,
    uint32_t  *tagDecision_ptr,
    uint32_t  *cvidDecision_ptr,
    uint32_t  *cpriDecision_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *cpri_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_CTAG_BIT;
	switch(*tagDecision_ptr){
		case 0: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_NOP; break;
		case 1: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_TAGGING; break;
		case 2: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_UNTAG; break;		
		case 3: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_TRANSPARENT; break;
		default:
			diag_util_printf("[ERROR!]invalid tagDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;	
	}
	
	switch(*cvidDecision_ptr){
		case 0: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_NOP; break;
		case 1: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_ASSIGN; break;
		case 2: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_COPY_FROM_1ST_TAG; break;		
		case 3: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_COPY_FROM_2ND_TAG; break;
		case 4: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_COPY_FROM_INTERNAL_VID; break;
		default:
			diag_util_printf("[ERROR!]invalid cvidDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;
	}
	
	switch(*cpriDecision_ptr){
		case 0: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_NOP; break;
		case 1: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_ASSIGN; break;
		case 2: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG; break;
		case 3: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG; break;
		case 4: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI; break;
		default:
			diag_util_printf("[ERROR!]invalid cvidDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;
	}
	classifyFilter.action_cvlan.assignedCvid = *cvid_ptr;
	classifyFilter.action_cvlan.assignedCpri = *cpri_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_ctag_tagdecision_tagdecision_cviddecision_cviddecision_cpridecision_cpridecision_cvid_cvid_cpri_cpri */

#if 0
/*
 * rg set classify-filter action direction upstream action_fwd <UINT:fwdDecision>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_fwd_fwdDecision(
    cparser_context_t *context,
    uint32_t  *fwdDecision_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_FWD_BIT;
	classifyFilter.action_fwd.fwdDecision= *fwdDecision_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_fwd_fwddecision */

/*
 * rg set classify-filter action direction upstream action_cfpri <UINT:cfpriDecision> cfpri <UINT:cfpri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_cfpri_cfpriDecision_cfpri_cfpri(
    cparser_context_t *context,
    uint32_t  *cfpriDecision_ptr,
    uint32_t  *cfpri_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_CFPRI_BIT;
	classifyFilter.action_cfpri.cfPriDecision = *cfpriDecision_ptr;
	classifyFilter.action_cfpri.assignedCfPri = *cfpri_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_cfpri_cfpridecision_cfpri_cfpri */

/*
 * rg set classify-filter action direction upstream action_dscp <UINT:dscpDecision> dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_dscp_dscpDecision_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscpDecision_ptr,
    uint32_t  *dscp_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.ds_action_field |= CF_US_ACTION_DSCP_BIT;
	classifyFilter.action_dscp.dscpDecision = *dscpDecision_ptr;
	classifyFilter.action_dscp.assignedDscp = *dscp_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_dscp_dscpdecision_dscp_dscp */

/*
 * rg set classify-filter action direction upstream action_sid <UINT:sidDecision> sid <UINT:sid>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_sid_sidDecision_sid_sid(
    cparser_context_t *context,
    uint32_t  *sidDecision_ptr,
    uint32_t  *sid_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_SID_BIT;
	classifyFilter.action_sid_or_llid.sidDecision = *sidDecision_ptr;
	classifyFilter.action_sid_or_llid.assignedSid_or_llid= *sid_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_sid_siddecision_sid_sid */
#endif

/*
 * rg set classify-filter action direction upstream action_fwd fwdDecision <UINT:fwdDecision>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_fwd_fwdDecision_fwdDecision(
    cparser_context_t *context,
    uint32_t  *fwdDecision_ptr)
{
    DIAG_UTIL_PARAM_CHK();
#if defined(CONFIG_RTL9602C_SERIES)
	DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_FWD_BIT;
	classifyFilter.action_fwd.fwdDecision= *fwdDecision_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_fwd_fwddecision_fwddecision */

/*
 * rg set classify-filter action direction upstream action_cfpri cfpriDecision <UINT:cfpriDecision> cfpri <UINT:cfpri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_cfpri_cfpriDecision_cfpriDecision_cfpri_cfpri(
    cparser_context_t *context,
    uint32_t  *cfpriDecision_ptr,
    uint32_t  *cfpri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
#if defined(CONFIG_RTL9602C_SERIES)
	DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_CFPRI_BIT;
	classifyFilter.action_cfpri.cfPriDecision = *cfpriDecision_ptr;
	classifyFilter.action_cfpri.assignedCfPri = *cfpri_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_cfpri_cfpridecision_cfpridecision_cfpri_cfpri */

/*
 * rg set classify-filter action direction upstream action_dscp dscpDecision <UINT:dscpDecision> dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_dscp_dscpDecision_dscpDecision_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscpDecision_ptr,
    uint32_t  *dscp_ptr)
{
	DIAG_UTIL_PARAM_CHK();
#if defined(CONFIG_RTL9602C_SERIES)
	DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.ds_action_field |= CF_US_ACTION_DSCP_BIT;
	classifyFilter.action_dscp.dscpDecision = *dscpDecision_ptr;
	classifyFilter.action_dscp.assignedDscp = *dscp_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_dscp_dscpdecision_dscpdecision_dscp_dscp */

/*
 * rg set classify-filter action direction upstream action_sid sidDecision <UINT:sidDecision> sid <UINT:sid>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_sid_sidDecision_sidDecision_sid_sid(
    cparser_context_t *context,
    uint32_t  *sidDecision_ptr,
    uint32_t  *sid_ptr)
{
	DIAG_UTIL_PARAM_CHK();
#if defined(CONFIG_RTL9602C_SERIES)
	DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_SID_BIT;
	classifyFilter.action_sid_or_llid.sidDecision = *sidDecision_ptr;
	classifyFilter.action_sid_or_llid.assignedSid_or_llid= *sid_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_sid_siddecision_siddecision_sid_sid */

/*
 * rg set classify-filter action direction downstream action_stag tagDecision <UINT:tagDecision> svidDecision <UINT:svidDecision> spriDecision <UINT:spriDecision> svid <UINT:svid> spri <UINT:spri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_tagDecision_tagDecision_svidDecision_svidDecision_spriDecision_spriDecision_svid_svid_spri_spri(
    cparser_context_t *context,
    uint32_t  *tagDecision_ptr,
    uint32_t  *svidDecision_ptr,
    uint32_t  *spriDecision_ptr,
    uint32_t  *svid_ptr,
    uint32_t  *spri_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_STAG_BIT;

	switch(*tagDecision_ptr){
		case 0: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_NOP; break;
		case 1: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID; break;
		case 2: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID2; break;
		case 3: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_UNTAG; break;
		case 4: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TRANSPARENT; break;
		case 5: acl_parameter.action_acl_svlan.svlanTagIfDecision=ACL_SVLAN_TAGIF_TAGGING_WITH_ORIGINAL_STAG_TPID; break;
		default:
			diag_util_printf("[ERROR!]invalid tagDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;
	}
	
	switch(*svidDecision_ptr){
		case 0: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_NOP; break;
		case 1: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_ASSIGN; break;
		case 2: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_COPY_FROM_1ST_TAG; break;
		case 3: acl_parameter.action_acl_svlan.svlanSvidDecision=ACL_SVLAN_SVID_COPY_FROM_2ND_TAG; break;
		default:
			diag_util_printf("[ERROR!]invalid svidDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;	
	}
	switch(*spriDecision_ptr){
		case 0: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_NOP; break;
		case 1: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_ASSIGN; break;
		case 2: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_COPY_FROM_1ST_TAG; break;
		case 3: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_COPY_FROM_2ND_TAG; break;
		case 4: acl_parameter.action_acl_svlan.svlanSpriDecision=ACL_SVLAN_SPRI_COPY_FROM_INTERNAL_PRI; break;
		default:
			diag_util_printf("[ERROR!]invalid spriDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;				
	}
	
	classifyFilter.action_svlan.assignedSvid = *svid_ptr;
	classifyFilter.action_svlan.assignedSpri = *spri_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_tagdecision_tagdecision_sviddecision_sviddecision_spridecision_spridecision_svid_svid_spri_spri */

/*
 * rg set classify-filter action direction downstream action_ctag tagDecision <UINT:tagDecision> cvidDecision <UINT:cvidDecision> cpriDecision <UINT:cpriDecision> cvid <UINT:cvid> cpri <UINT:cpri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_ctag_tagDecision_tagDecision_cvidDecision_cvidDecision_cpriDecision_cpriDecision_cvid_cvid_cpri_cpri(
    cparser_context_t *context,
    uint32_t  *tagDecision_ptr,
    uint32_t  *cvidDecision_ptr,
    uint32_t  *cpriDecision_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *cpri_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_CTAG_BIT;
	switch(*tagDecision_ptr){
		case 0: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_NOP; break;
		case 1: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_TAGGING; break;
		case 2: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_UNTAG; break;		
		case 3: acl_parameter.action_acl_cvlan.cvlanTagIfDecision=ACL_CVLAN_TAGIF_TRANSPARENT; break;
		default:
			diag_util_printf("[ERROR!]invalid tagDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;	
	}
	
	switch(*cvidDecision_ptr){
		case 0: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_NOP; break;
		case 1: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_ASSIGN; break;
		case 2: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_COPY_FROM_1ST_TAG; break;		
		case 3: acl_parameter.action_acl_cvlan.cvlanCvidDecision=ACL_CVLAN_CVID_COPY_FROM_2ND_TAG; break;
		default:
			diag_util_printf("[ERROR!]invalid cvidDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;
	}
	
	switch(*cpriDecision_ptr){
		case 0: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_NOP; break;
		case 1: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_ASSIGN; break;
		case 2: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_COPY_FROM_1ST_TAG; break;
		case 3: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_COPY_FROM_2ND_TAG; break;
		case 4: acl_parameter.action_acl_cvlan.cvlanCpriDecision=ACL_CVLAN_CPRI_COPY_FROM_INTERNAL_PRI; break;
		default:
			diag_util_printf("[ERROR!]invalid cvidDecision parameter.\n");
			return CPARSER_ERR_INVALID_PARAMS;
			break;
	}
	classifyFilter.action_cvlan.assignedCvid = *cvid_ptr;
	classifyFilter.action_cvlan.assignedCpri = *cpri_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_ctag_tagdecision_tagdecision_cviddecision_cviddecision_cpridecision_cpridecision_cvid_cvid_cpri_cpri */
#if 0
/*
 * rg set classify-filter action direction downstream action_fwd <UINT:fwdDecision> fwdportmask <HEX:fwdportmask>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_fwd_fwdDecision_fwdportmask_fwdportmask(
    cparser_context_t *context,
    uint32_t  *fwdDecision_ptr,
    uint32_t  *fwdportmask_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_UNI_MASK_BIT;
	classifyFilter.action_uni.uniActionDecision = *fwdDecision_ptr;
	classifyFilter.action_uni.assignedUniPortMask = *fwdportmask_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_fwd_fwddecision_fwdportmask_fwdportmask */


/*
 * rg set classify-filter action direction downstream action_cfpri <UINT:cfpriDecision> cfpri <UINT:cfpri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_cfpri_cfpriDecision_cfpri_cfpri(
    cparser_context_t *context,
    uint32_t  *cfpriDecision_ptr,
    uint32_t  *cfpri_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_CFPRI_BIT;
	classifyFilter.action_cfpri.cfPriDecision = *cfpriDecision_ptr;
	classifyFilter.action_cfpri.assignedCfPri = *cfpri_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_cfpri_cfpridecision_cfpri_cfpri */

/*
 * rg set classify-filter action direction downstream action_dscp <UINT:dscpDecision> dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_dscp_dscpDecision_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscpDecision_ptr,
    uint32_t  *dscp_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_DSCP_BIT;
	classifyFilter.action_dscp.dscpDecision = *dscpDecision_ptr;
	classifyFilter.action_dscp.assignedDscp = *dscp_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_dscp_dscpdecision_dscp_dscp */
#endif

/*
 * rg set classify-filter action direction downstream action_fwd fwdDecision <UINT:fwdDecision> fwdportmask <HEX:fwdportmask>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_fwd_fwdDecision_fwdDecision_fwdportmask_fwdportmask(
    cparser_context_t *context,
    uint32_t  *fwdDecision_ptr,
    uint32_t  *fwdportmask_ptr)
{

#if defined(CONFIG_RTL9602C_SERIES)
	DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_UNI_MASK_BIT;
	classifyFilter.action_uni.uniActionDecision = *fwdDecision_ptr;
	classifyFilter.action_uni.assignedUniPortMask = *fwdportmask_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_fwd_fwddecision_fwddecision_fwdportmask_fwdportmask */

/*
 * rg set classify-filter action direction downstream action_cfpri cfpriDecision <UINT:cfpriDecision> cfpri <UINT:cfpri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_cfpri_cfpriDecision_cfpriDecision_cfpri_cfpri(
    cparser_context_t *context,
    uint32_t  *cfpriDecision_ptr,
    uint32_t  *cfpri_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
	DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_CFPRI_BIT;
	classifyFilter.action_cfpri.cfPriDecision = *cfpriDecision_ptr;
	classifyFilter.action_cfpri.assignedCfPri = *cfpri_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_cfpri_cfpridecision_cfpridecision_cfpri_cfpri */

/*
 * rg set classify-filter action direction downstream action_dscp dscpDecision <UINT:dscpDecision> dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_dscp_dscpDecision_dscpDecision_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscpDecision_ptr,
    uint32_t  *dscp_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
	DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_DSCP_BIT;
	classifyFilter.action_dscp.dscpDecision = *dscpDecision_ptr;
	classifyFilter.action_dscp.assignedDscp = *dscp_ptr;
#else
	diag_util_printf("[NOT SUPPORT!]\n");
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_dscp_dscpdecision_dscpdecision_dscp_dscp */

#endif
#if 1 //RTL9600 SERIES CF ACTIONS

/*
 * rg set classify-filter action direction upstream action_drop
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_drop(
    cparser_context_t *context)
{
#if defined(CONFIG_RTL9602C_SERIES)
	diag_util_printf("[NOT SUPPORT!] please use: rg set classify-filter action direction upstream action_fwd\n");
#else
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_DROP_BIT;
#endif
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_drop */

/*
 * rg set classify-filter action direction upstream action_sid <UINT:sid>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_sid_sid(
    cparser_context_t *context,
    uint32_t  *sid_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_SID_BIT;
	classifyFilter.action_sid_or_llid.assignedSid_or_llid= *sid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_sid_sid */



/*
 * rg set classify-filter action direction downstream action_drop
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_drop(
    cparser_context_t *context)
{
#if defined(CONFIG_RTL9602C_SERIES)
	diag_util_printf("[NOT SUPPORT!] please use: rg set classify-filter action direction downstream action_fwd\n");
#else
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_DROP_BIT;
#endif
    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_drop */

/*
 * rg set classify-filter action direction upstream action_stag tagWithTPID svidDecision <UINT:svidDecision> spriDecision <UINT:spriDecision> svid <UINT:svid> spri <UINT:spri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_stag_tagWithTPID_svidDecision_svidDecision_spriDecision_spriDecision_svid_svid_spri_spri(
    cparser_context_t *context,
    uint32_t  *svidDecision_ptr,
    uint32_t  *spriDecision_ptr,
    uint32_t  *svid_ptr,
    uint32_t  *spri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_STAG_BIT;
	classifyFilter.action_svlan.svlanTagIfDecision = ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID;
	classifyFilter.action_svlan.svlanSvidDecision = *svidDecision_ptr;
	classifyFilter.action_svlan.svlanSpriDecision = *spriDecision_ptr;
	classifyFilter.action_svlan.assignedSvid = *svid_ptr;
	classifyFilter.action_svlan.assignedSpri = *spri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_stag_tagwithtpid_sviddecision_sviddecision_spridecision_spridecision_svid_svid_spri_spri */

/*
 * rg set classify-filter action direction upstream action_stag tagWith8100 svidDecision <UINT:svidDecision> spriDecision <UINT:spriDecision> svid <UINT:svid> spri <UINT:spri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_stag_tagWith8100_svidDecision_svidDecision_spriDecision_spriDecision_svid_svid_spri_spri(
    cparser_context_t *context,
    uint32_t  *svidDecision_ptr,
    uint32_t  *spriDecision_ptr,
    uint32_t  *svid_ptr,
    uint32_t  *spri_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_STAG_BIT;
	classifyFilter.action_svlan.svlanTagIfDecision = ACL_SVLAN_TAGIF_TAGGING_WITH_8100;
	classifyFilter.action_svlan.svlanSvidDecision = *svidDecision_ptr;
	classifyFilter.action_svlan.svlanSpriDecision = *spriDecision_ptr;
	classifyFilter.action_svlan.assignedSvid = *svid_ptr;
	classifyFilter.action_svlan.assignedSpri = *spri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_stag_tagwith8100_sviddecision_sviddecision_spridecision_spridecision_svid_svid_spri_spri */

/*
 * rg set classify-filter action direction upstream action_stag unTag
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_stag_unTag(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_STAG_BIT;
	classifyFilter.action_svlan.svlanTagIfDecision = ACL_SVLAN_TAGIF_UNTAG;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_stag_untag */

/*
 * rg set classify-filter action direction upstream action_stag transparent
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_stag_transparent(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_STAG_BIT;
	classifyFilter.action_svlan.svlanTagIfDecision = ACL_SVLAN_TAGIF_TRANSPARENT;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_stag_transparent */

/*
 * rg set classify-filter action direction upstream action_ctag tagging cvidDecision <UINT:cvidDecision> cpriDecision <UINT:cpriDecision> cvid <UINT:cvid> cpri <UINT:cpri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_ctag_tagging_cvidDecision_cvidDecision_cpriDecision_cpriDecision_cvid_cvid_cpri_cpri(
    cparser_context_t *context,
    uint32_t  *cvidDecision_ptr,
    uint32_t  *cpriDecision_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *cpri_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_CTAG_BIT;
	classifyFilter.action_cvlan.cvlanTagIfDecision = ACL_CVLAN_TAGIF_TAGGING;
	classifyFilter.action_cvlan.cvlanCvidDecision = *cvidDecision_ptr;
	classifyFilter.action_cvlan.cvlanCpriDecision = *cpriDecision_ptr;
	classifyFilter.action_cvlan.assignedCvid = *cvid_ptr;
	classifyFilter.action_cvlan.assignedCpri = *cpri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_ctag_tagging_cviddecision_cviddecision_cpridecision_cpridecision_cvid_cvid_cpri_cpri */

/*
 * rg set classify-filter action direction upstream action_ctag tagWithC2S
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_ctag_tagWithC2S(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_CTAG_BIT;
	classifyFilter.action_cvlan.cvlanTagIfDecision = ACL_CVLAN_TAGIF_TAGGING_WITH_C2S;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_ctag_tagwithc2s */

/*
 * rg set classify-filter action direction upstream action_ctag unTag
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_ctag_unTag(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_CTAG_BIT;
	classifyFilter.action_cvlan.cvlanTagIfDecision = ACL_CVLAN_TAGIF_UNTAG;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_ctag_untag */

/*
 * rg set classify-filter action direction upstream action_ctag transparent
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_ctag_transparent(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_CTAG_BIT;
	classifyFilter.action_cvlan.cvlanTagIfDecision = ACL_CVLAN_TAGIF_TRANSPARENT;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_ctag_transparent */

/*
 * rg set classify-filter action direction upstream action_cfpri <UINT:cfpri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_cfpri_cfpri(
    cparser_context_t *context,
    uint32_t  *cfpri_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_CFPRI_BIT;
	classifyFilter.action_cfpri.assignedCfPri= *cfpri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_cfpri_cfpri */

/*
 * rg set classify-filter action direction upstream action_dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscp_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_DSCP_BIT;
	classifyFilter.action_dscp.assignedDscp= *dscp_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_dscp_dscp */

/*
 * rg set classify-filter action direction upstream action_log <UINT:cntIdx>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_log_cntIdx(
    cparser_context_t *context,
    uint32_t  *cntIdx_ptr)
{
#if defined(CONFIG_RTL9602C_SERIES)
	diag_util_printf("[NOT SUPPORT!] apolloFE does not have this action.");
#else
	DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_UPSTREAM;
	classifyFilter.us_action_field |= CF_US_ACTION_LOG_BIT;
	classifyFilter.action_log.assignedCounterIdx= *cntIdx_ptr;
#endif

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_upstream_action_log_cntidx */

/*
 * rg set classify-filter action direction downstream action_stag tagWithTPID svidDecision <UINT:svidDecision> spriDecision <UINT:spriDecision> svid <UINT:svid> spri <UINT:spri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_tagWithTPID_svidDecision_svidDecision_spriDecision_spriDecision_svid_svid_spri_spri(
    cparser_context_t *context,
    uint32_t  *svidDecision_ptr,
    uint32_t  *spriDecision_ptr,
    uint32_t  *svid_ptr,
    uint32_t  *spri_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_STAG_BIT;
	classifyFilter.action_svlan.svlanTagIfDecision = ACL_SVLAN_TAGIF_TAGGING_WITH_VSTPID;
	classifyFilter.action_svlan.svlanSvidDecision = *svidDecision_ptr;
	classifyFilter.action_svlan.svlanSpriDecision = *spriDecision_ptr;
	classifyFilter.action_svlan.assignedSvid = *svid_ptr;
	classifyFilter.action_svlan.assignedSpri = *spri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_tagwithtpid_sviddecision_sviddecision_spridecision_spridecision_svid_svid_spri_spri */

/*
 * rg set classify-filter action direction downstream action_stag tagWith8100 svidDecision <UINT:svidDecision> spriDecision <UINT:spriDecision> svid <UINT:svid> spri <UINT:spri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_tagWith8100_svidDecision_svidDecision_spriDecision_spriDecision_svid_svid_spri_spri(
    cparser_context_t *context,
    uint32_t  *svidDecision_ptr,
    uint32_t  *spriDecision_ptr,
    uint32_t  *svid_ptr,
    uint32_t  *spri_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_STAG_BIT;
	classifyFilter.action_svlan.svlanTagIfDecision = ACL_SVLAN_TAGIF_TAGGING_WITH_8100;
	classifyFilter.action_svlan.svlanSvidDecision = *svidDecision_ptr;
	classifyFilter.action_svlan.svlanSpriDecision = *spriDecision_ptr;
	classifyFilter.action_svlan.assignedSvid = *svid_ptr;
	classifyFilter.action_svlan.assignedSpri = *spri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_tagwith8100_sviddecision_sviddecision_spridecision_spridecision_svid_svid_spri_spri */

/*
 * rg set classify-filter action direction downstream action_stag tagWithSP2C
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_tagWithSP2C(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_STAG_BIT;
	classifyFilter.action_svlan.svlanTagIfDecision = ACL_SVLAN_TAGIF_TAGGING_WITH_SP2C;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_tagwithsp2c */

/*
 * rg set classify-filter action direction downstream action_stag unTag
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_unTag(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_STAG_BIT;
	classifyFilter.action_svlan.svlanTagIfDecision = ACL_SVLAN_TAGIF_UNTAG;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_untag */

/*
 * rg set classify-filter action direction downstream action_stag transparent
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_transparent(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_STAG_BIT;
	classifyFilter.action_svlan.svlanTagIfDecision = ACL_SVLAN_TAGIF_TRANSPARENT;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_stag_transparent */

/*
 * rg set classify-filter action direction downstream action_ctag tagging cvidDecision <UINT:cvidDecision> cpriDecision <UINT:cpriDecision> cvid <UINT:cvid> cpri <UINT:cpri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_ctag_tagging_cvidDecision_cvidDecision_cpriDecision_cpriDecision_cvid_cvid_cpri_cpri(
    cparser_context_t *context,
    uint32_t  *cvidDecision_ptr,
    uint32_t  *cpriDecision_ptr,
    uint32_t  *cvid_ptr,
    uint32_t  *cpri_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_CTAG_BIT;
	classifyFilter.action_cvlan.cvlanTagIfDecision = ACL_CVLAN_TAGIF_TAGGING;
	classifyFilter.action_cvlan.cvlanCvidDecision = *cvidDecision_ptr;
	classifyFilter.action_cvlan.cvlanCpriDecision = *cpriDecision_ptr;
	classifyFilter.action_cvlan.assignedCvid = *cvid_ptr;
	classifyFilter.action_cvlan.assignedCpri = *cpri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_ctag_tagging_cviddecision_cviddecision_cpridecision_cpridecision_cvid_cvid_cpri_cpri */

/*
 * rg set classify-filter action direction downstream action_ctag tagWithSP2C
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_ctag_tagWithSP2C(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_CTAG_BIT;
	classifyFilter.action_cvlan.cvlanTagIfDecision = ACL_CVLAN_TAGIF_TAGGING_WITH_SP2C;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_ctag_tagwithsp2c */

/*
 * rg set classify-filter action direction downstream action_ctag unTag
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_ctag_unTag(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_CTAG_BIT;
	classifyFilter.action_cvlan.cvlanTagIfDecision = ACL_CVLAN_TAGIF_UNTAG;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_ctag_untag */

/*
 * rg set classify-filter action direction downstream action_ctag transparent
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_ctag_transparent(
    cparser_context_t *context)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_CTAG_BIT;
	classifyFilter.action_cvlan.cvlanTagIfDecision = ACL_CVLAN_TAGIF_TRANSPARENT;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_ctag_transparent */

/*
 * rg set classify-filter action direction downstream action_cfpri <UINT:cfpri>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_cfpri_cfpri(
    cparser_context_t *context,
    uint32_t  *cfpri_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_CFPRI_BIT;
	classifyFilter.action_cfpri.assignedCfPri= *cfpri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_cfpri_cfpri */

/*
 * rg set classify-filter action direction downstream action_dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscp_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_DSCP_BIT;
	classifyFilter.action_dscp.assignedDscp= *dscp_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_dscp_dscp */

/*
 * rg set classify-filter action direction downstream action_uni uniDecision <UINT:uniDecision> portmask <HEX:portmask>
 */
cparser_result_t
cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_uni_uniDecision_uniDecision_portmask_portmask(
    cparser_context_t *context,
    uint32_t  *uniDecision_ptr,
    uint32_t  *portmask_ptr)
{

    DIAG_UTIL_PARAM_CHK();
	classifyFilter.direction = RTK_RG_CLASSIFY_DIRECTION_DOWNSTREAM;
	classifyFilter.ds_action_field |= CF_DS_ACTION_UNI_MASK_BIT;
	classifyFilter.action_uni.uniActionDecision = *uniDecision_ptr;
	classifyFilter.action_uni.assignedUniPortMask = *portmask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_rg_set_classify_filter_action_direction_downstream_action_uni_unidecision_unidecision_portmask_portmask */


#endif




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
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <parser/cparser_priv.h>
#include <dal/apollo/raw/apollo_raw_acl.h>
#include <diag_str.h>


typedef struct diag_acl_ruleCfg_s
{
    rtk_enable_t dmac_en;
    rtk_mac_t dmac;
    rtk_mac_t dmac_mask;
    rtk_enable_t smac_en;
    rtk_mac_t smac;
    rtk_mac_t smac_mask;

    rtk_enable_t sip_en;
    ipaddr_t sip;
    ipaddr_t sip_mask;
    rtk_enable_t dip_en;
    ipaddr_t dip;
    ipaddr_t dip_mask;

    rtk_enable_t sip6_en;
    rtk_ipv6_addr_t sip6;
    rtk_ipv6_addr_t sip6_mask;
    rtk_enable_t dip6_en;
    rtk_ipv6_addr_t dip6;
    rtk_ipv6_addr_t dip6_mask;

    rtk_enable_t ethertype_en;
    uint32 ethertype;
    uint32 ethertype_mask;
    rtk_enable_t ctag_en;
    uint32 ctag;
    uint32 ctag_mask;
    rtk_enable_t stag_en;
    uint32 stag;
    uint32 stag_mask;
    rtk_enable_t gemid_en;
    uint32 gemid;
    uint32 gemid_mask;

    rtk_enable_t nh_en;
    uint32 nh;
    uint32 nh_mask;

    rtk_enable_t rngVid_en;
    uint32 rngVid;
    uint32 rngVid_mask;
    rtk_enable_t rngIp_en;
    uint32 rngIp;
    uint32 rngIp_mask;
    rtk_enable_t rngL4Port_en;
    uint32 rngL4Port;
    uint32 rngL4Port_mask;
    rtk_enable_t rngPktLen_en;
    uint32 rngPktLen;
    uint32 rngPktLen_mask;

    uint32 fieldValid_en;
    uint32 fieldValid;
    uint32 fieldValid_mask;
    
    rtk_enable_t field_en[APOLLO_RAW_ACL_FIELD_SELECTOR_NO];
    uint32 field[APOLLO_RAW_ACL_FIELD_SELECTOR_NO];
    uint32 field_mask[APOLLO_RAW_ACL_FIELD_SELECTOR_NO];

    rtk_portmask_t Port;
    rtk_portmask_t Port_mask;

    rtk_enable_t tags[APOLLO_ACL_RULETAG_END];
    rtk_enable_t tags_mask[APOLLO_ACL_RULETAG_END];

    apollo_raw_acl_modeTypes_t mode;
    uint32 tempIdx;
    uint32 ruleIdx;
    
} diag_acl_ruleCfg_t;

diag_acl_ruleCfg_t diag_aclRule;
apollo_raw_acl_template_t diag_aclTemplate;
apollo_raw_acl_actCtrl_t diag_aclActCtrl;
apollo_raw_acl_act_t diag_aclAct;

int8 * _diag_aclGetTagName(apollo_raw_acl_ruleTag_t tag)
{
    static int8 name[20];
	switch(tag)
	{
        case APOLLO_ACL_RULETAG_PPPOE: osal_strcpy(name,"PPPoE");break;
        case APOLLO_ACL_RULETAG_CTAG: osal_strcpy(name,"Ctag");break;
        case APOLLO_ACL_RULETAG_STAG: osal_strcpy(name,"Stag");break;
        case APOLLO_ACL_RULETAG_IP4: osal_strcpy(name,"IP4");break;
        case APOLLO_ACL_RULETAG_IP6: osal_strcpy(name,"IP6");break;
        case APOLLO_ACL_RULETAG_TCP: osal_strcpy(name,"Tcp");break;
        case APOLLO_ACL_RULETAG_UDP: osal_strcpy(name,"Udp");break;
        default: osal_strcpy(name,"Known");break;
	}
    return name;
}

int8 * _diag_aclGetTypeName(apollo_raw_acl_fieldTypes_t type)
{
    static int8 name[20];
	switch(type)
	{
		case ACL_DMAC0: osal_strcpy(name,"DMAC0");break;
		case ACL_DMAC1: osal_strcpy(name,"DMAC1");break;
		case ACL_DMAC2: osal_strcpy(name,"DMAC2");break;
		case ACL_SMAC0: osal_strcpy(name,"SMAC0");break;
		case ACL_SMAC1: osal_strcpy(name,"SMAC1");break;
		case ACL_SMAC2: osal_strcpy(name,"SMAC2");break;
		case ACL_ETHERTYPE: osal_strcpy(name,"ETHERTYPE");break;
		case ACL_STAG: osal_strcpy(name,"STAG");break;
		case ACL_CTAG: osal_strcpy(name,"CTAG");break;
		case ACL_GEMIDXLLIDX: osal_strcpy(name,"GEM/LLID");break;

		case ACL_IP4SIP0: osal_strcpy(name,"IP4SIP0");break;
		case ACL_IP4SIP1: osal_strcpy(name,"IP4SIP1");break;
		case ACL_IP4DIP0: osal_strcpy(name,"IP4DIP0");break;
		case ACL_IP4DIP1: osal_strcpy(name,"IP4DIP1");break;

		case ACL_IP6SIP0WITHIPV4: osal_strcpy(name,"IP6SIP0");break;
		case ACL_IP6SIP1WITHIPV4: osal_strcpy(name,"IP6SIP1");break;
		case ACL_IP6DIP0WITHIPV4: osal_strcpy(name,"IP6DIP0");break;
		case ACL_IP6DIP1WITHIPV4: osal_strcpy(name,"IP6DIP1");break;
		case ACL_IP6NH: osal_strcpy(name,"NEXTHEADER");break;

 		case ACL_VIDRANGE: osal_strcpy(name,"VIDRANGE");break;
 		case ACL_IPRANGE: osal_strcpy(name,"IPRANGE");break;
 		case ACL_PORTRANGE: osal_strcpy(name,"PORTRANGE");break;
 		case ACL_PKTLENRANGE: osal_strcpy(name,"PKTELNRANGE");break;
 		case ACL_FIELDVALID: osal_strcpy(name,"FIELDVALID");break;

		case ACL_FIELD_SELECT00: osal_strcpy(name,"Field_Sel0");break;
		case ACL_FIELD_SELECT01: osal_strcpy(name,"Field_Sel1");break;
		case ACL_FIELD_SELECT02: osal_strcpy(name,"Field_Sel2");break;
		case ACL_FIELD_SELECT03: osal_strcpy(name,"Field_Sel3");break;
		case ACL_FIELD_SELECT04: osal_strcpy(name,"Field_Sel4");break;
		case ACL_FIELD_SELECT05: osal_strcpy(name,"Field_Sel5");break;
		case ACL_FIELD_SELECT06: osal_strcpy(name,"Field_Sel6");break;
		case ACL_FIELD_SELECT07: osal_strcpy(name,"Field_Sel7");break;
		case ACL_FIELD_SELECT08: osal_strcpy(name,"Field_Sel8");break;
		case ACL_FIELD_SELECT09: osal_strcpy(name,"Field_Sel9");break;
		case ACL_FIELD_SELECT10: osal_strcpy(name,"Field_Sel10");break;
		case ACL_FIELD_SELECT11: osal_strcpy(name,"Field_Sel11");break;
		case ACL_FIELD_SELECT12: osal_strcpy(name,"Field_Sel12");break;
		case ACL_FIELD_SELECT13: osal_strcpy(name,"Field_Sel13");break;
		case ACL_FIELD_SELECT14: osal_strcpy(name,"Field_Sel14");break;
		case ACL_FIELD_SELECT15: osal_strcpy(name,"Field_Sel15");break;
		default: osal_strcpy(name,"unknown");
	}
    return name;
}



void _diag_aclTemplateShow(apollo_raw_acl_template_t *pAclTmeplate)
{
    DIAG_UTIL_OUTPUT_INIT();
    diag_util_mprintf("Template: %d ",pAclTmeplate->idx);
    diag_util_mprintf("[0] %s\n",_diag_aclGetTypeName(pAclTmeplate->field[0]));
    diag_util_mprintf("            [1] %s\n",_diag_aclGetTypeName(pAclTmeplate->field[1]));
    diag_util_mprintf("            [2] %s\n",_diag_aclGetTypeName(pAclTmeplate->field[2]));
    diag_util_mprintf("            [3] %s\n",_diag_aclGetTypeName(pAclTmeplate->field[3]));
    diag_util_mprintf("            [4] %s\n",_diag_aclGetTypeName(pAclTmeplate->field[4]));
    diag_util_mprintf("            [5] %s\n",_diag_aclGetTypeName(pAclTmeplate->field[5]));
    diag_util_mprintf("            [6] %s\n",_diag_aclGetTypeName(pAclTmeplate->field[6]));
    diag_util_mprintf("            [7] %s\n",_diag_aclGetTypeName(pAclTmeplate->field[7]));
}


void _diag_user2rawField(uint32 fieldIdx, apollo_raw_acl_template_t *pTemplate, diag_acl_ruleCfg_t *pDiagRule, apollo_raw_acl_ruleEntry_t *pRawRule)
{
	switch(pTemplate->field[fieldIdx])
	{
		case ACL_DMAC0: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->dmac.octet[4];
            pRawRule->data_bits.field[fieldIdx] = (pRawRule->data_bits.field[fieldIdx] << 8) | pDiagRule->dmac.octet[5];
                
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->dmac_mask.octet[4];
            pRawRule->care_bits.field[fieldIdx] = (pRawRule->care_bits.field[fieldIdx] << 8) | pDiagRule->dmac_mask.octet[5];
            break;

		case ACL_DMAC1: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->dmac.octet[2];
            pRawRule->data_bits.field[fieldIdx] = (pRawRule->data_bits.field[fieldIdx] << 8) | pDiagRule->dmac.octet[3];
                
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->dmac_mask.octet[2];
            pRawRule->care_bits.field[fieldIdx] = (pRawRule->care_bits.field[fieldIdx] << 8) | pDiagRule->dmac_mask.octet[3];
            break;
            
		case ACL_DMAC2: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->dmac.octet[0];
            pRawRule->data_bits.field[fieldIdx] = (pRawRule->data_bits.field[fieldIdx] << 8) | pDiagRule->dmac.octet[1];
                
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->dmac_mask.octet[0];
            pRawRule->care_bits.field[fieldIdx] = (pRawRule->care_bits.field[fieldIdx] << 8) | pDiagRule->dmac_mask.octet[1];
            break;
		case ACL_SMAC0: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->smac.octet[4];
            pRawRule->data_bits.field[fieldIdx] = (pRawRule->data_bits.field[fieldIdx] << 8) | pDiagRule->smac.octet[5];
                
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->smac_mask.octet[4];
            pRawRule->care_bits.field[fieldIdx] = (pRawRule->care_bits.field[fieldIdx] << 8) | pDiagRule->smac_mask.octet[5];
            break;

		case ACL_SMAC1: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->smac.octet[2];
            pRawRule->data_bits.field[fieldIdx] = (pRawRule->data_bits.field[fieldIdx] << 8) | pDiagRule->smac.octet[3];
                
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->smac_mask.octet[2];
            pRawRule->care_bits.field[fieldIdx] = (pRawRule->care_bits.field[fieldIdx] << 8) | pDiagRule->smac_mask.octet[3];
            break;
            
		case ACL_SMAC2: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->smac.octet[0];
            pRawRule->data_bits.field[fieldIdx] = (pRawRule->data_bits.field[fieldIdx] << 8) | pDiagRule->smac.octet[1];
                
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->smac_mask.octet[0];
            pRawRule->care_bits.field[fieldIdx] = (pRawRule->care_bits.field[fieldIdx] << 8) | pDiagRule->smac_mask.octet[1];
            break;

		case ACL_ETHERTYPE: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->ethertype;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->ethertype_mask;
            break;

		case ACL_STAG: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->stag;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->stag_mask;
            break;
          
		case ACL_CTAG: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->ctag;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->ctag_mask;
            break;

		case ACL_GEMIDXLLIDX: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->gemid;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->gemid_mask;
            break;

		case ACL_IP4SIP0: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->sip&0xFFFF;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->sip_mask&0xFFFF;
            break;

		case ACL_IP4SIP1: 
            pRawRule->data_bits.field[fieldIdx] = (pDiagRule->sip)>>16;
            pRawRule->care_bits.field[fieldIdx] = (pDiagRule->sip_mask)>>16;
            break;

		case ACL_IP4DIP0: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->dip&0xFFFF;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->dip_mask&0xFFFF;
            break;

		case ACL_IP4DIP1: 
            pRawRule->data_bits.field[fieldIdx] = (pDiagRule->dip)>>16;
            pRawRule->care_bits.field[fieldIdx] = (pDiagRule->dip_mask)>>16;
            break;

		case ACL_IP6SIP0WITHIPV4: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->sip6.ipv6_addr[14];
            pRawRule->data_bits.field[fieldIdx] = (pRawRule->data_bits.field[fieldIdx] << 8) | pDiagRule->sip6.ipv6_addr[15];
                
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->sip6_mask.ipv6_addr[14];
            pRawRule->care_bits.field[fieldIdx] = (pRawRule->care_bits.field[fieldIdx] << 8) | pDiagRule->sip6_mask.ipv6_addr[15];
            break;
          
		case ACL_IP6SIP1WITHIPV4: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->sip6.ipv6_addr[12];
            pRawRule->data_bits.field[fieldIdx] = (pRawRule->data_bits.field[fieldIdx] << 8) | pDiagRule->sip6.ipv6_addr[13];
                
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->sip6_mask.ipv6_addr[12];
            pRawRule->care_bits.field[fieldIdx] = (pRawRule->care_bits.field[fieldIdx] << 8) | pDiagRule->sip6_mask.ipv6_addr[13];
            break;

		case ACL_IP6DIP0WITHIPV4: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->dip6.ipv6_addr[14];
            pRawRule->data_bits.field[fieldIdx] = (pRawRule->data_bits.field[fieldIdx] << 8) | pDiagRule->dip6.ipv6_addr[15];
                
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->dip6_mask.ipv6_addr[14];
            pRawRule->care_bits.field[fieldIdx] = (pRawRule->care_bits.field[fieldIdx] << 8) | pDiagRule->dip6_mask.ipv6_addr[15];
            break;
          
		case ACL_IP6DIP1WITHIPV4: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->dip6.ipv6_addr[12];
            pRawRule->data_bits.field[fieldIdx] = (pRawRule->data_bits.field[fieldIdx] << 8) | pDiagRule->dip6.ipv6_addr[13];
                
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->dip6_mask.ipv6_addr[12];
            pRawRule->care_bits.field[fieldIdx] = (pRawRule->care_bits.field[fieldIdx] << 8) | pDiagRule->dip6_mask.ipv6_addr[13];
            break;

		case ACL_IP6NH: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->nh;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->nh_mask;
            break;

		case ACL_VIDRANGE: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->rngVid;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->rngVid_mask;
            break;

		case ACL_IPRANGE: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->rngIp;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->rngIp_mask;
            break;

		case ACL_PORTRANGE: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->rngL4Port;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->rngL4Port_mask;
            break;

		case ACL_PKTLENRANGE: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->rngPktLen;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->rngPktLen_mask;
            break;

		case ACL_FIELDVALID: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->fieldValid;
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->fieldValid_mask;
            break;

		case ACL_FIELD_SELECT00: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[0];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[0];
            break;

		case ACL_FIELD_SELECT01: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[1];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[1];
            break;

		case ACL_FIELD_SELECT02: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[2];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[2];
            break;

		case ACL_FIELD_SELECT03: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[3];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[3];
            break;

		case ACL_FIELD_SELECT04: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[4];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[4];
            break;

		case ACL_FIELD_SELECT05: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[5];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[5];
            break;

		case ACL_FIELD_SELECT06: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[6];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[6];
            break;

		case ACL_FIELD_SELECT07: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[7];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[7];
            break;

		case ACL_FIELD_SELECT08: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[8];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[8];
            break;

		case ACL_FIELD_SELECT09: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[9];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[9];
            break;

		case ACL_FIELD_SELECT10: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[10];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[10];
            break;

		case ACL_FIELD_SELECT11: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[11];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[11];
            break;

		case ACL_FIELD_SELECT12: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[12];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[12];
            break;

		case ACL_FIELD_SELECT13: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[13];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[13];
            break;

		case ACL_FIELD_SELECT14: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[14];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[14];
            break;

		case ACL_FIELD_SELECT15: 
            pRawRule->data_bits.field[fieldIdx] = pDiagRule->field[15];
            pRawRule->care_bits.field[fieldIdx] = pDiagRule->field_mask[15];
            break;

		default: 
            break;
	}
}

void _diag_raw2userField(uint32 fieldIdx, apollo_raw_acl_template_t *pTemplate, diag_acl_ruleCfg_t *pDiagRule, apollo_raw_acl_ruleEntry_t *pRawRule)
{
	switch(pTemplate->field[fieldIdx])
	{
		case ACL_DMAC0: 
            pDiagRule->dmac_en = ENABLED;
            pDiagRule->dmac.octet[4] = (pRawRule->data_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->dmac.octet[5] = pRawRule->data_bits.field[fieldIdx] & 0xFF;
            
            pDiagRule->dmac_mask.octet[4] = (pRawRule->care_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->dmac_mask.octet[5] = pRawRule->care_bits.field[fieldIdx] & 0xFF;
            break;

		case ACL_DMAC1: 
            pDiagRule->dmac_en = ENABLED;
            pDiagRule->dmac.octet[2] = (pRawRule->data_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->dmac.octet[3] = pRawRule->data_bits.field[fieldIdx] & 0xFF;
            
            pDiagRule->dmac_mask.octet[2] = (pRawRule->care_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->dmac_mask.octet[3] = pRawRule->care_bits.field[fieldIdx] & 0xFF;
            break;
            
		case ACL_DMAC2: 
            pDiagRule->dmac_en = ENABLED;
            pDiagRule->dmac.octet[0] = (pRawRule->data_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->dmac.octet[1] = pRawRule->data_bits.field[fieldIdx] & 0xFF;
            
            pDiagRule->dmac_mask.octet[0] = (pRawRule->care_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->dmac_mask.octet[1] = pRawRule->care_bits.field[fieldIdx] & 0xFF;
            break;

		case ACL_SMAC0: 
            pDiagRule->smac_en = ENABLED;
            pDiagRule->smac.octet[4] = (pRawRule->data_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->smac.octet[5] = pRawRule->data_bits.field[fieldIdx] & 0xFF;
            
            pDiagRule->smac_mask.octet[4] = (pRawRule->care_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->smac_mask.octet[5] = pRawRule->care_bits.field[fieldIdx] & 0xFF;
            break;

		case ACL_SMAC1: 
            pDiagRule->smac_en = ENABLED;
            pDiagRule->smac.octet[2] = (pRawRule->data_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->smac.octet[3] = pRawRule->data_bits.field[fieldIdx] & 0xFF;
            
            pDiagRule->smac_mask.octet[2] = (pRawRule->care_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->smac_mask.octet[3] = pRawRule->care_bits.field[fieldIdx] & 0xFF;
            break;
            
		case ACL_SMAC2: 
            pDiagRule->smac_en = ENABLED;
            pDiagRule->smac.octet[0] = (pRawRule->data_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->smac.octet[1] = pRawRule->data_bits.field[fieldIdx] & 0xFF;
            
            pDiagRule->smac_mask.octet[0] = (pRawRule->care_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->smac_mask.octet[1] = pRawRule->care_bits.field[fieldIdx] & 0xFF;
            break;

		case ACL_ETHERTYPE: 
            pDiagRule->ethertype_en = ENABLED;
            pDiagRule->ethertype = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->ethertype_mask = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_STAG: 
            pDiagRule->stag_en = ENABLED;
            pDiagRule->stag = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->stag_mask = pRawRule->care_bits.field[fieldIdx];
            break;
          
		case ACL_CTAG: 
            pDiagRule->ctag_en = ENABLED;
            pDiagRule->ctag = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->ctag_mask = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_GEMIDXLLIDX: 
            pDiagRule->gemid_en = ENABLED;
            pDiagRule->gemid = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->gemid_mask = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_IP4SIP0: 
            pDiagRule->sip_en = ENABLED;
            pDiagRule->sip = (pDiagRule->sip & 0xFFFF0000) | pRawRule->data_bits.field[fieldIdx];
            pDiagRule->sip_mask = (pDiagRule->sip_mask & 0xFFFF0000) | pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_IP4SIP1: 
            pDiagRule->sip_en = ENABLED;
            pDiagRule->sip = (pDiagRule->sip & 0xFFFF) | (pRawRule->data_bits.field[fieldIdx]<<16);
            pDiagRule->sip_mask = (pDiagRule->sip_mask & 0xFFFF) | (pRawRule->care_bits.field[fieldIdx]<<16);
            break;

		case ACL_IP4DIP0: 
            pDiagRule->dip_en = ENABLED;
            pDiagRule->dip = (pDiagRule->dip & 0xFFFF0000) | pRawRule->data_bits.field[fieldIdx];
            pDiagRule->dip_mask = (pDiagRule->dip_mask & 0xFFFF0000) | pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_IP4DIP1: 
            pDiagRule->dip_en = ENABLED;
            pDiagRule->dip = (pDiagRule->dip & 0xFFFF) | (pRawRule->data_bits.field[fieldIdx]<<16);
            pDiagRule->dip_mask = (pDiagRule->dip_mask & 0xFFFF) | (pRawRule->care_bits.field[fieldIdx]<<16);
            break;

		case ACL_IP6SIP0WITHIPV4: 
            pDiagRule->sip6_en = ENABLED;
            pDiagRule->sip6.ipv6_addr[14] = (pRawRule->data_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->sip6.ipv6_addr[15] = pRawRule->data_bits.field[fieldIdx] & 0xFF;

            pDiagRule->sip6_mask.ipv6_addr[14] = (pRawRule->care_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->sip6_mask.ipv6_addr[15] = pRawRule->care_bits.field[fieldIdx] & 0xFF;
            break;
          
		case ACL_IP6SIP1WITHIPV4: 
            pDiagRule->sip6_en = ENABLED;
            pDiagRule->sip6.ipv6_addr[12] = (pRawRule->data_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->sip6.ipv6_addr[13] = pRawRule->data_bits.field[fieldIdx] & 0xFF;

            pDiagRule->sip6_mask.ipv6_addr[12] = (pRawRule->care_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->sip6_mask.ipv6_addr[13] = pRawRule->care_bits.field[fieldIdx] & 0xFF;
            break;

		case ACL_IP6DIP0WITHIPV4: 
            pDiagRule->dip6_en = ENABLED;
            pDiagRule->dip6.ipv6_addr[14] = (pRawRule->data_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->dip6.ipv6_addr[15] = pRawRule->data_bits.field[fieldIdx] & 0xFF;

            pDiagRule->dip6_mask.ipv6_addr[14] = (pRawRule->care_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->dip6_mask.ipv6_addr[15] = pRawRule->care_bits.field[fieldIdx] & 0xFF;
            break;
          
		case ACL_IP6DIP1WITHIPV4: 
            pDiagRule->dip6_en = ENABLED;
            pDiagRule->dip6.ipv6_addr[12] = (pRawRule->data_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->dip6.ipv6_addr[13] = pRawRule->data_bits.field[fieldIdx] & 0xFF;

            pDiagRule->dip6_mask.ipv6_addr[12] = (pRawRule->care_bits.field[fieldIdx] >> 8) & 0xFF;
            pDiagRule->dip6_mask.ipv6_addr[13] = pRawRule->care_bits.field[fieldIdx] & 0xFF;
            break;

		case ACL_IP6NH: 
            pDiagRule->nh_en = ENABLED;
            pDiagRule->nh = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->nh_mask = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_VIDRANGE: 
            pDiagRule->rngVid_en = ENABLED;
            pDiagRule->rngVid = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->rngVid_mask = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_IPRANGE: 
            pDiagRule->rngIp_en = ENABLED;
            pDiagRule->rngIp = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->rngIp_mask = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_PORTRANGE: 
            pDiagRule->rngL4Port_en = ENABLED;
            pDiagRule->rngL4Port = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->rngL4Port_mask = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_PKTLENRANGE: 
            pDiagRule->rngPktLen_en = ENABLED;
            pDiagRule->rngPktLen = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->rngPktLen_mask = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELDVALID: 
            pDiagRule->fieldValid_en = ENABLED;
            pDiagRule->fieldValid = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->fieldValid_mask = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT00: 
            pDiagRule->field_en[0] = ENABLED;
            pDiagRule->field[0] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[0] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT01: 
            pDiagRule->field_en[1] = ENABLED;
            pDiagRule->field[1] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[1] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT02: 
            pDiagRule->field_en[2] = ENABLED;
            pDiagRule->field[2] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[2] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT03: 
            pDiagRule->field_en[3] = ENABLED;
            pDiagRule->field[3] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[3] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT04: 
            pDiagRule->field_en[4] = ENABLED;
            pDiagRule->field[4] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[4] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT05: 
            pDiagRule->field_en[5] = ENABLED;
            pDiagRule->field[5] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[5] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT06: 
            pDiagRule->field_en[6] = ENABLED;
            pDiagRule->field[6] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[6] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT07: 
            pDiagRule->field_en[7] = ENABLED;
            pDiagRule->field[7] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[7] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT08: 
            pDiagRule->field_en[8] = ENABLED;
            pDiagRule->field[8] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[8] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT09: 
            pDiagRule->field_en[9] = ENABLED;
            pDiagRule->field[9] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[9] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT10: 
            pDiagRule->field_en[10] = ENABLED;
            pDiagRule->field[10] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[10] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT11: 
            pDiagRule->field_en[11] = ENABLED;
            pDiagRule->field[11] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[11] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT12: 
            pDiagRule->field_en[12] = ENABLED;
            pDiagRule->field[12] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[12] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT13: 
            pDiagRule->field_en[13] = ENABLED;
            pDiagRule->field[13] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[13] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT14: 
            pDiagRule->field_en[14] = ENABLED;
            pDiagRule->field[14] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[14] = pRawRule->care_bits.field[fieldIdx];
            break;

		case ACL_FIELD_SELECT15: 
            pDiagRule->field_en[15] = ENABLED;
            pDiagRule->field[15] = pRawRule->data_bits.field[fieldIdx];
            pDiagRule->field_mask[15] = pRawRule->care_bits.field[fieldIdx];
            break;

		default: 
            break;
	}
}

void _diag_aclRuleDiag2Raw(apollo_raw_acl_template_t *pDiagTmplate, diag_acl_ruleCfg_t *pDiagRule,  apollo_raw_acl_ruleEntry_t *pRawRule)
{
    uint16 tagIdx;
    uint16 fieldIdx;

    osal_memset(pRawRule,0x00,sizeof(apollo_raw_acl_ruleEntry_t));
    
    pRawRule->data_bits.active_portmsk = pDiagRule->Port.bits[0];
    pRawRule->data_bits.type = pDiagRule->tempIdx;

    pRawRule->data_bits.tag_exist = 0;
    for(tagIdx = 0; tagIdx < APOLLO_ACL_RULETAG_END; tagIdx++)
    {
        if(pDiagRule->tags[tagIdx] == ENABLED)
        {
            pRawRule->data_bits.tag_exist = pRawRule->data_bits.tag_exist | (1 << tagIdx);
        }           
    }
    
    pRawRule->care_bits.active_portmsk = pDiagRule->Port_mask.bits[0];
    pRawRule->care_bits.type = 0x3;
    pRawRule->care_bits.tag_exist = 0;
    for(tagIdx = 0; tagIdx < APOLLO_ACL_RULETAG_END; tagIdx++)
    {
        if(pDiagRule->tags_mask[tagIdx] == ENABLED)
        {
            pRawRule->care_bits.tag_exist = pRawRule->care_bits.tag_exist | (1 << tagIdx);
        }           
    }

    /*_diag_aclTemplateShow(pDiagTmplate);*/

    if(pDiagRule->mode == APOLLO_ACL_MODE_64ENTRIES)
    {
        for(fieldIdx = 0; fieldIdx < APOLLO_ACLRULEFIELDNO; fieldIdx++)
        {
             _diag_user2rawField(fieldIdx, pDiagTmplate, pDiagRule, pRawRule);
        }
    }
    else
    {
        if(pDiagRule->ruleIdx <= 63)
        {
            for(fieldIdx = 4; fieldIdx < 8; fieldIdx++)
            {
                 _diag_user2rawField(fieldIdx, pDiagTmplate, pDiagRule, pRawRule);
            }
            
            pRawRule->data_bits.field[0] = pRawRule->data_bits.field[4];
            pRawRule->care_bits.field[0] = pRawRule->care_bits.field[4];
            pRawRule->data_bits.field[1] = pRawRule->data_bits.field[5];
            pRawRule->care_bits.field[1] = pRawRule->care_bits.field[5];
            pRawRule->data_bits.field[2] = pRawRule->data_bits.field[6];
            pRawRule->care_bits.field[2] = pRawRule->care_bits.field[6];
            pRawRule->data_bits.field[3] = pRawRule->data_bits.field[7];
            pRawRule->care_bits.field[4] = pRawRule->care_bits.field[7];
            
        }
        else
        {
            for(fieldIdx = 0; fieldIdx < 3; fieldIdx++)
            {
                 _diag_user2rawField(fieldIdx, pDiagTmplate, pDiagRule, pRawRule);
            }
        }
    }    
    /*
    osal_printf("data ");
    for(fieldIdx = 0; fieldIdx < APOLLO_ACLRULEFIELDNO; fieldIdx++)
    {
        osal_printf("%4.4x ", pRawRule->data_bits.field[fieldIdx]);
    }
    osal_printf("\n");
    osal_printf("care ");
    for(fieldIdx = 0; fieldIdx < APOLLO_ACLRULEFIELDNO; fieldIdx++)
    {
        osal_printf("%4.4x ", pRawRule->care_bits.field[fieldIdx]);
    }
    osal_printf("\n");
    */
}

void _diag_aclRuleRaw2Diag(apollo_raw_acl_template_t *pDiagTmplate, diag_acl_ruleCfg_t *pDiagRule,  apollo_raw_acl_ruleEntry_t *pRawRule)
{
    uint16 tagIdx;
    uint16 fieldIdx;
    
    pDiagRule->Port.bits[0] = pRawRule->data_bits.active_portmsk;
    pDiagRule->tempIdx = pRawRule->data_bits.type;

    for(tagIdx = 0; tagIdx < APOLLO_ACL_RULETAG_END; tagIdx++)
    {
        if(pRawRule->data_bits.tag_exist & (1 << tagIdx))
        {
            pDiagRule->tags[tagIdx] = ENABLED;
        }           
        else
        {
            pDiagRule->tags[tagIdx] = DISABLED;
        }     
    }
    
    pDiagRule->Port_mask.bits[0] = pRawRule->care_bits.active_portmsk;
    for(tagIdx = 0; tagIdx < APOLLO_ACL_RULETAG_END; tagIdx++)
    {
        if(pRawRule->care_bits.tag_exist & (1 << tagIdx))
        {
            pDiagRule->tags_mask[tagIdx] = ENABLED;
        }           
        else
        {
            pDiagRule->tags_mask[tagIdx] = DISABLED;
        }     
    }
    osal_printf("pDiagRule->mode %d\n",pDiagRule->mode);
    if(pDiagRule->mode == APOLLO_ACL_MODE_64ENTRIES)
    {
        for(fieldIdx = 0; fieldIdx < APOLLO_ACLRULEFIELDNO; fieldIdx++)
        {
             _diag_raw2userField(fieldIdx, pDiagTmplate, pDiagRule, pRawRule);

        }
    }
    else
    {
        if(pDiagRule->ruleIdx <= 63)
        {
            pRawRule->data_bits.field[4] = pRawRule->data_bits.field[0];
            pRawRule->care_bits.field[4] = pRawRule->care_bits.field[0];
            pRawRule->data_bits.field[5] = pRawRule->data_bits.field[1];
            pRawRule->care_bits.field[5] = pRawRule->care_bits.field[1];
            pRawRule->data_bits.field[6] = pRawRule->data_bits.field[2];
            pRawRule->care_bits.field[6] = pRawRule->care_bits.field[2];
            pRawRule->data_bits.field[7] = pRawRule->data_bits.field[3];
            pRawRule->care_bits.field[7] = pRawRule->care_bits.field[3];
            
            for(fieldIdx = 4; fieldIdx < 8; fieldIdx++)
            {
                 _diag_raw2userField(fieldIdx, pDiagTmplate, pDiagRule, pRawRule);

            }    
        }
        else
        {
            for(fieldIdx = 0; fieldIdx < 3; fieldIdx++)
            {
                 _diag_raw2userField(fieldIdx, pDiagTmplate, pDiagRule, pRawRule);

            }    
        }
    }
    /*
    osal_printf("data ");
    for(fieldIdx = 0; fieldIdx < APOLLO_ACLRULEFIELDNO; fieldIdx++)
    {
        osal_printf("%4.4x ", pRawRule->data_bits.field[fieldIdx]);
    }
    osal_printf("\n");
    osal_printf("care ");
    for(fieldIdx = 0; fieldIdx < APOLLO_ACLRULEFIELDNO; fieldIdx++)
    {
        osal_printf("%4.4x ", pRawRule->care_bits.field[fieldIdx]);
    }
    osal_printf("\n");    
    */
}


int32 _diag_aclTemplateAdd(apollo_raw_acl_fieldTypes_t type)
{
    uint32 fieldIdx;

    for(fieldIdx = 0; fieldIdx < APOLLO_ACLRULEFIELDNO; fieldIdx ++ )
    {
        if(diag_aclTemplate.field[fieldIdx] == ACL_UNUSED)
        {
            diag_aclTemplate.field[fieldIdx] = type;
            return RT_ERR_OK;
        }    
        else if(diag_aclTemplate.field[fieldIdx] == type)
        {
            return RT_ERR_OK;
        }    
    }

    return RT_ERR_FAILED;
}


void _diag_aclRuleShow(diag_acl_ruleCfg_t *pAclRule)
{
    uint32 fieldIdx;
    apollo_raw_acl_ruleTag_t tag;
    uint8 buf[64];
    uint8 buf2[64];
    
    diag_util_lPortMask2str(buf, &pAclRule->Port);
    diag_util_lPortMask2str(buf2, &pAclRule->Port_mask);

    
    /*diag_util_mprintf("\nMode: %s\n", diagStr_aclModeStr[pAclRule->mode]);*/
    diag_util_mprintf("Active Port: %s Mask: %s\n",buf, buf2);

    diag_util_printf("Tags: ");
    for(tag = 0; tag < APOLLO_ACL_RULETAG_END; tag++)
    {
        if(pAclRule->tags[tag])
            diag_util_printf("%s ",_diag_aclGetTagName(tag));
    }
    diag_util_printf("Mask: ");
    for(tag = 0; tag < APOLLO_ACL_RULETAG_END; tag++)
    {
        if(pAclRule->tags_mask[tag])
            diag_util_printf("%s ",_diag_aclGetTagName(tag));
    }
    diag_util_mprintf("\n");
    
    if(pAclRule->dmac_en)
    {
        diag_util_mprintf("dmac data: %s\n",diag_util_inet_mactoa(&pAclRule->dmac.octet[0]));
        diag_util_mprintf("     mask: %s\n",diag_util_inet_mactoa(&pAclRule->dmac_mask.octet[0]));
    }
    
    if(pAclRule->smac_en)
    {
        diag_util_mprintf("smac data: %s\n",diag_util_inet_mactoa(&pAclRule->smac.octet[0]));
        diag_util_mprintf("     mask: %s\n",diag_util_inet_mactoa(&pAclRule->smac_mask.octet[0]));
    }

    if(pAclRule->sip_en)
    {
        diag_util_mprintf("sip data: %s\n",diag_util_inet_ntoa(pAclRule->sip));
        diag_util_mprintf("    mask: %s\n",diag_util_inet_ntoa(pAclRule->sip_mask));
        
    }

    if(pAclRule->dip_en)
    {
        diag_util_mprintf("dip data: %s\n",diag_util_inet_ntoa(pAclRule->dip));
        diag_util_mprintf("    mask: %s\n",diag_util_inet_ntoa(pAclRule->dip_mask));
        
    }

    if(pAclRule->sip6_en)
    {
        diag_util_mprintf("sipv6 data: %s\n",diag_util_inet_n6toa( &pAclRule->sip6.ipv6_addr[0]));
        diag_util_mprintf("      mask: %s\n",diag_util_inet_n6toa( &pAclRule->sip6_mask.ipv6_addr[0]));
        
    }

    if(pAclRule->dip6_en)
    {
        diag_util_mprintf("dipv6 data: %s\n",diag_util_inet_n6toa( &pAclRule->dip6.ipv6_addr[0]));
        diag_util_mprintf("      mask: %s\n",diag_util_inet_n6toa( &pAclRule->dip6_mask.ipv6_addr[0]));
        
    }

    if(pAclRule->ethertype_en)
    {
        diag_util_mprintf("ethertype data: 0x%4.4X\n",pAclRule->ethertype);
        diag_util_mprintf("          mask: 0x%4.4X\n",pAclRule->ethertype_mask);
    }

    if(pAclRule->gemid_en)
    {
        diag_util_mprintf("gemport/llid data: 0x%4.4X\n",pAclRule->gemid);
        diag_util_mprintf("             mask: 0x%4.4X\n",pAclRule->gemid_mask);
    }

    if(pAclRule->nh_en)
    {
        diag_util_mprintf("next header data: 0x%4.4X\n",pAclRule->nh);
        diag_util_mprintf("            mask: 0x%4.4X\n",pAclRule->nh_mask);
    }

    if(pAclRule->ctag_en)
    {
        diag_util_mprintf("ctag data: priority: %d cfi: %d vid: %d\n",
                                                (pAclRule->ctag>>13)&0x7,
                                                (pAclRule->ctag>>12)&0x1,
                                                pAclRule->ctag&0xFFF);
        diag_util_mprintf("     mask: priority: %d cfi: %d vid: %d\n",
                                                (pAclRule->ctag_mask>>13)&0x7,
                                                (pAclRule->ctag_mask>>12)&0x1,
                                                pAclRule->ctag_mask&0xFFF);
    }

    if(pAclRule->stag_en)
    {
        diag_util_mprintf("stag data: priority: %d dei: %d vid: %d\n",
                                                (pAclRule->stag>>13)&0x7,
                                                (pAclRule->stag>>12)&0x1,
                                                pAclRule->stag&0xFFF);
        diag_util_mprintf("     mask: priority: %d dei: %d vid: %d\n",
                                                (pAclRule->stag_mask>>13)&0x7,
                                                (pAclRule->stag_mask>>12)&0x1,
                                                pAclRule->stag_mask&0xFFF);
    }
    
    if(pAclRule->rngVid_en)
    {
        diag_util_mprintf("vid range data: %s\n",diag_util_mask32tostr(pAclRule->rngVid));
        diag_util_mprintf("          mask: %s\n",diag_util_mask32tostr(pAclRule->rngVid_mask));
    }

    if(pAclRule->rngIp_en)
    {
        diag_util_mprintf("ip range data: %s\n",diag_util_mask32tostr(pAclRule->rngIp));
        diag_util_mprintf("         mask: %s\n",diag_util_mask32tostr(pAclRule->rngIp_mask));
    }

    if(pAclRule->rngL4Port_en)
    {
        diag_util_mprintf("l4 port range data: %s\n",diag_util_mask32tostr(pAclRule->rngL4Port));
        diag_util_mprintf("              mask: %s\n",diag_util_mask32tostr(pAclRule->rngL4Port_mask));
    }

    if(pAclRule->rngPktLen_en)
    {
        diag_util_mprintf("pkt length range data: %s\n",diag_util_mask32tostr(pAclRule->rngPktLen));
        diag_util_mprintf("                 mask: %s\n",diag_util_mask32tostr(pAclRule->rngPktLen_mask));
    }

    for( fieldIdx = 0; fieldIdx < APOLLO_RAW_ACL_FIELD_SELECTOR_NO; fieldIdx ++)
    {
        if(pAclRule->field_en[fieldIdx])
        {
            diag_util_mprintf("filed[%2d] data: 0x%4.4X mask: 0x%4.4X\n", 
                                                fieldIdx,
                                                pAclRule->field[fieldIdx],
                                                pAclRule->field_mask[fieldIdx]);
        }
    }
}

void _diag_aclActCtrlShow(apollo_raw_acl_actCtrl_t *pActCtrl)
{
    diag_util_mprintf("Valid: %s\n", diagStr_valid[pActCtrl->valid]);
    diag_util_mprintf("Operation: %s\n", diagStr_aclOper[pActCtrl->not]);
    diag_util_mprintf("Cvlan Action: %s\n", diagStr_enable[pActCtrl->cvlan_en]);
    diag_util_mprintf("Svaln Action: %s\n", diagStr_enable[pActCtrl->svlan_en]);
    diag_util_mprintf("Forward: %s\n", diagStr_enable[pActCtrl->fwd_en]);
    diag_util_mprintf("Policing: %s\n", diagStr_enable[pActCtrl->polic_en]);
    diag_util_mprintf("Pri-Remark: %s\n", diagStr_enable[pActCtrl->pri_remark_en]);
    diag_util_mprintf("Interrupt or Classification: %s\n", diagStr_enable[pActCtrl->int_cf_en]);
}

void _diag_aclActShow(apollo_raw_acl_actCtrl_t *pActCtrl, apollo_raw_acl_act_t *pAclAct)
{
    uint8 buf[16];
    diag_util_mprintf("Valid: %s\n", diagStr_valid[pActCtrl->valid]);
    diag_util_mprintf("Not: %s\n", diagStr_enable[pActCtrl->not]);
        
    if(pActCtrl->cvlan_en)
    {
        diag_util_printf("Cvlan Ation: %s ",diagStr_aclActCvlanStr[pAclAct->cact]);
        switch(pAclAct->cact)
        {
            case APOLLO_ACL_CACT_INGRESS:
            case APOLLO_ACL_CACT_EGRESS:    
                diag_util_mprintf("vidx: %d\n",pAclAct->cvidx_cact);
                break;
            case APOLLO_ACL_CACT_POLICE:
                diag_util_mprintf("meter: %d\n",pAclAct->cvidx_cact);
                break;
            case APOLLO_ACL_CACT_LOGGING:    
                diag_util_mprintf("counter: %d\n",pAclAct->cvidx_cact);
                break;
            case APOLLO_ACL_CACT_1PREMARK:    
                diag_util_mprintf("1p-pri: %d\n",pAclAct->cvidx_cact);
                break;
            default:
                diag_util_mprintf("\n");
                break;
        }
    }
    else
    {
        diag_util_mprintf("Cvlan Ation: %s\n", diagStr_enable[pActCtrl->cvlan_en]);
    }   

    if(pActCtrl->svlan_en)
    {
        diag_util_printf("Svlan Ation: %s ",diagStr_aclActSvlanStr[pAclAct->sact]);

        switch(pAclAct->sact)
        {
            case APOLLO_ACL_SACT_INGRESS:
            case APOLLO_ACL_SACT_EGRESS:    
                diag_util_mprintf("svidx: %d\n",pAclAct->svidx_sact);
                break;
            case APOLLO_ACL_SACT_POLICE:
                diag_util_mprintf("meter: %d\n",pAclAct->svidx_sact);
                break;
            case APOLLO_ACL_SACT_LOGGING:    
                diag_util_mprintf("counter: %d\n",pAclAct->svidx_sact);
                break;
            case APOLLO_ACL_SACT_1PREMARK:    
                diag_util_mprintf("1p-pri: %d\n",pAclAct->svidx_sact);
                break;
            case APOLLO_ACL_SACT_DSCPREMARK:    
                diag_util_mprintf("dscp: %d\n",pAclAct->svidx_sact);
                break;
            default:
                diag_util_mprintf("\n");
                break;
        }
    }
    else
    {
        diag_util_mprintf("Svlan Ation: %s\n", diagStr_enable[pActCtrl->svlan_en]);
    }   

    if(pActCtrl->fwd_en)
    {
        if(pAclAct->fwdact == APOLLO_ACL_FWDACT_TRAP)
        {
            diag_util_mprintf("Forward: %s \n",diagStr_aclActFwdStr[pAclAct->fwdact]);

        }
        else
        {
            diag_util_mask2str(buf,pAclAct->fwdpmask);
            diag_util_mprintf("Forward: %s Ports: %s\n", 
                                    diagStr_aclActFwdStr[pAclAct->fwdact],
                                    buf);
                                    /*diag_util_mask32tostr(pAclAct->fwdpmask));*/

        }
    }
    else
    {
        diag_util_mprintf("Forward: %s\n", diagStr_enable[pActCtrl->fwd_en]);
    }   

    if(pActCtrl->polic_en)
    {
        diag_util_printf("Policing: %s ",diagStr_aclActPoliceStr[pAclAct->policact],
                                    pAclAct->meteridx);
        switch(pAclAct->policact)
        {
            case APOLLO_ACL_PACT_POLICE:
                diag_util_mprintf("meter: %d\n",pAclAct->meteridx);
                break;
            case APOLLO_ACL_PACT_LOGGING:    
                diag_util_mprintf("counter: %d\n",pAclAct->meteridx);
                break;
            default:
                diag_util_mprintf("\n");
                break;
        }

    }
    else
    {
        diag_util_mprintf("Policing: %s\n", diagStr_enable[pActCtrl->polic_en]);
    }   

    if(pActCtrl->pri_remark_en)
    {
        diag_util_printf("Pri-Remark: %s ",diagStr_aclActPriStr[pAclAct->priact]);
        
        switch(pAclAct->priact)
        {
            case APOLLO_ACL_PRIACT_PRI:    
                diag_util_mprintf("acl-pri: %d\n",pAclAct->pridx);
                break;
            case APOLLO_ACL_PRIACT_1PREMARK:    
                diag_util_mprintf("1p-pri: %d\n",pAclAct->pridx);
                break;
            case APOLLO_ACL_PRIACT_DSCPREMARK:    
                diag_util_mprintf("dscp: %d\n",pAclAct->pridx);
                break;
            case APOLLO_ACL_PRIACT_POLICE:
                diag_util_mprintf("meter: %d\n",pAclAct->pridx);
                break;
            case APOLLO_ACL_PRIACT_LOGGING:    
                diag_util_mprintf("counter: %d\n",pAclAct->pridx);
                break;
            default:
                diag_util_mprintf("\n");
                break;
        }
    }
    else
    {
        diag_util_mprintf("Pri-Remark: %s\n", diagStr_enable[pActCtrl->pri_remark_en]);
    }   

    if(pActCtrl->int_cf_en)
    {
        diag_util_mprintf("Interrupt: %s\n",diagStr_enable[pAclAct->aclint]);
        if(pAclAct->cfact == APOLLO_ACL_CFACT_NONE)
        {
            diag_util_mprintf("Classification: %s\n",diagStr_aclActCfStr[pAclAct->cfact]);
        }
        else if(pAclAct->cfact == APOLLO_ACL_CFACT_VCEXT)
        {
            diag_util_mprintf("Classification: %s VcExtMask: %d\n",
                                            diagStr_aclActCfStr[pAclAct->cfact],
                                            pAclAct->cfidx);
        }
        else
        {
            diag_util_mprintf("Classification: %s index: %d\n",
                                            diagStr_aclActCfStr[pAclAct->cfact],
                                            pAclAct->cfidx);
        }
    }
    else
    {
        diag_util_mprintf("Interrupt or Classification: %s\n", diagStr_enable[pActCtrl->int_cf_en]);
    }
}

cparser_result_t _diag_aclEntryDel(uint32 index, apollo_raw_acl_modeTypes_t mode)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_ruleEntry_t rawRule;
    apollo_raw_acl_template_t aclTemplate;
    apollo_raw_acl_actCtrl_t aclActCtrl;
    apollo_raw_acl_act_t aclAct;

    osal_memset(&rawRule,0x0,sizeof(apollo_raw_acl_ruleEntry_t));
    rawRule.idx = index;
    rawRule.mode = mode;

    if(mode == APOLLO_ACL_MODE_64ENTRIES)
        rawRule.valid = DISABLED;
    else
        rawRule.valid = ENABLED;
        
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_rule_set(&rawRule), ret);;  

    osal_memset(&aclActCtrl,0x0,sizeof(apollo_raw_acl_actCtrl_t));
    aclActCtrl.idx = index;
    aclActCtrl.valid = DISABLED;
    aclActCtrl.cvlan_en = ENABLED;
    aclActCtrl.svlan_en = ENABLED;
    aclActCtrl.fwd_en = ENABLED;
    aclActCtrl.polic_en = ENABLED;
    aclActCtrl.pri_remark_en= ENABLED;
    aclActCtrl.int_cf_en= ENABLED;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_actCtrl_set(&aclActCtrl), ret);

    osal_memset(&aclAct,0x0,sizeof(apollo_raw_acl_act_t));
    aclAct.idx = index;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_act_set(&aclAct), ret);  

    return CPARSER_OK;
}


/*
 * acl show 
 */
cparser_result_t
cparser_cmd_acl_show(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_template_t aclTemplate;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    _diag_aclActShow(&diag_aclActCtrl,&diag_aclAct);

    aclTemplate.idx = diag_aclRule.tempIdx; 
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_get(&aclTemplate), ret);
    
    _diag_aclTemplateShow(&aclTemplate);
    _diag_aclRuleShow(&diag_aclRule);
       
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_show */

/*
 * acl clear 
 */
cparser_result_t
cparser_cmd_acl_clear(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_aclRule,0x0,sizeof(diag_acl_ruleCfg_t));
    osal_memset(&diag_aclActCtrl,0x0,sizeof(apollo_raw_acl_actCtrl_t));
    osal_memset(&diag_aclAct,0x0,sizeof(apollo_raw_acl_act_t));
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_clear */

/*
 * acl add entry <UINT:entry> ( rule | action-data | action-control ) 
 */
cparser_result_t
cparser_cmd_acl_add_entry_entry_rule_action_data_action_control(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_ruleEntry_t rawRule;
    apollo_raw_acl_template_t aclTemplate;
    apollo_raw_acl_actCtrl_t aclActCtrl;
    apollo_raw_acl_act_t aclAct;

    DIAG_UTIL_PARAM_CHK();

    aclTemplate.idx = diag_aclRule.tempIdx;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_get(&aclTemplate), ret);

    diag_aclRule.ruleIdx = *entry_ptr;
    _diag_aclRuleDiag2Raw(&aclTemplate, &diag_aclRule, &rawRule);

    if(!osal_strcmp(TOKEN_STR(4),"rule"))
    {
        rawRule.idx = *entry_ptr;
        rawRule.valid = ENABLED;
        rawRule.mode = APOLLO_ACL_MODE_64ENTRIES;
        DIAG_UTIL_ERR_CHK(apollo_raw_acl_rule_set(&rawRule), ret);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"action-data"))
    {
        diag_aclAct.idx = *entry_ptr;
        DIAG_UTIL_ERR_CHK(apollo_raw_acl_act_set(&diag_aclAct), ret);
    }
    else if(!osal_strcmp(TOKEN_STR(4),"action-control"))
    {
        diag_aclActCtrl.idx = *entry_ptr;
        DIAG_UTIL_ERR_CHK(apollo_raw_acl_actCtrl_set(&diag_aclActCtrl), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_add_entry_entry_rule_action_data_action_control */



/*
 * acl add entry <UINT:entry> 
 */
cparser_result_t
cparser_cmd_acl_add_entry_entry(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_ruleEntry_t rawRule;
    apollo_raw_acl_template_t aclTemplate;
    apollo_raw_acl_actCtrl_t aclActCtrl;
    apollo_raw_acl_act_t aclAct;

    DIAG_UTIL_PARAM_CHK();

    aclTemplate.idx = diag_aclRule.tempIdx;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_get(&aclTemplate), ret);
    
    diag_aclRule.ruleIdx = *entry_ptr;
    _diag_aclRuleDiag2Raw(&aclTemplate, &diag_aclRule, &rawRule);
        
    rawRule.idx = *entry_ptr;
    rawRule.valid = ENABLED;
    rawRule.mode = diag_aclRule.mode;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_rule_set(&rawRule), ret);;  
    /*
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_set(&diag_aclTemplate), ret);
    */
    diag_aclActCtrl.idx = *entry_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_actCtrl_set(&diag_aclActCtrl), ret);

    diag_aclAct.idx = *entry_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_act_set(&diag_aclAct), ret);


    return CPARSER_OK;
}    /* end of cparser_cmd_acl_add_entry_entry */

/*
 * acl del entry <UINT:entry> 
 */
cparser_result_t
cparser_cmd_acl_del_entry_entry(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(_diag_aclEntryDel(*entry_ptr, diag_aclRule.mode), ret);  

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_del_entry_entry */

/*
 * acl del entry all 
 */
cparser_result_t
cparser_cmd_acl_del_entry_all(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    uint32 idx;

    DIAG_UTIL_PARAM_CHK();

    if(diag_aclRule.mode == APOLLO_ACL_MODE_64ENTRIES)
    {
        for(idx = 0; idx <= APOLLO_ACLRULEMAX; idx ++)
        {
            DIAG_UTIL_ERR_CHK(_diag_aclEntryDel(idx,diag_aclRule.mode), ret);  
        }
    }
    else
    {
        for(idx = 0; idx <= APOLLO_ACLACTIONMAX; idx ++)
        {
            DIAG_UTIL_ERR_CHK(_diag_aclEntryDel(idx,diag_aclRule.mode), ret);  
        }
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_del_entry_all */


/*
 * acl set rule mode ( 64-entries | 128-entries )
 */
cparser_result_t
cparser_cmd_acl_set_rule_mode_64_entries_128_entries(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_modeTypes_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    
    if ('6' == TOKEN_CHAR(4,0))
    {
        mode = APOLLO_ACL_MODE_64ENTRIES;
    }
    else if ('1' == TOKEN_CHAR(4,0))
    {
        mode = APOLLO_ACL_MODE_128ENTRIES;
    }

    diag_aclRule.mode = mode;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_mode_64_entries_128_entries */


/*
 * acl get entry <UINT:entry> 
 */
cparser_result_t
cparser_cmd_acl_get_entry_entry(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_ruleEntry_t rawRule;
    diag_acl_ruleCfg_t diagRule;
    apollo_raw_acl_template_t aclTemplate;
    uint32 tmpData;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    rawRule.idx = *entry_ptr;
    rawRule.mode = diag_aclRule.mode;
    /*osal_printf("rawRule.idx %d, rawRule.mode %d\n",rawRule.idx,rawRule.mode);*/
    
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_rule_get(&rawRule), ret);
    /*need to fixed for 1st dummy read from table access to ACL_CTRL_REG*/
    diag_aclTemplate.idx = rawRule.data_bits.type;
    
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_get(&diag_aclTemplate), ret);

    diag_aclTemplate.idx = rawRule.data_bits.type;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_get(&diag_aclTemplate), ret);

    diag_aclActCtrl.idx = *entry_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_actCtrl_get(&diag_aclActCtrl), ret);

    diag_aclAct.idx = *entry_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_act_get(&diag_aclAct), ret);

    osal_memset(&diagRule, 0x0, sizeof(diag_acl_ruleCfg_t));

    diagRule.mode = diag_aclRule.mode;
    diagRule.ruleIdx = *entry_ptr;
    _diag_aclRuleRaw2Diag(&diag_aclTemplate, &diagRule, &rawRule);
   
    _diag_aclActShow(&diag_aclActCtrl,&diag_aclAct);
    _diag_aclTemplateShow(&diag_aclTemplate);
    _diag_aclRuleShow(&diagRule);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_get_entry_entry */

/*
 * acl set rule ( dmac | smac ) data <MACADDR:mac_data> mask <MACADDR:mac_mask>
 */
cparser_result_t
cparser_cmd_acl_set_rule_dmac_smac_data_mac_data_mask_mac_mask(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_data_ptr,
    cparser_macaddr_t  *mac_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    
    if ('s' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.smac_en = ENABLED;
        osal_memcpy(&diag_aclRule.smac.octet, mac_data_ptr->octet, ETHER_ADDR_LEN);    
        osal_memcpy(&diag_aclRule.smac_mask.octet, mac_mask_ptr->octet, ETHER_ADDR_LEN);    
    }
    else if ('d' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.dmac_en = ENABLED;
        osal_memcpy(&diag_aclRule.dmac.octet, mac_data_ptr->octet, ETHER_ADDR_LEN);    
        osal_memcpy(&diag_aclRule.dmac_mask.octet, mac_mask_ptr->octet, ETHER_ADDR_LEN);    
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_dmac_smac_data_mac_data_mask_mac_mask */

/*
 * acl set rule ( dmac | smac ) data <MACADDR:mac_data> 
 */
cparser_result_t
cparser_cmd_acl_set_rule_dmac_smac_data_mac_data(
    cparser_context_t *context,
    cparser_macaddr_t  *mac_data_ptr)
{

    if ('s' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.smac_en = ENABLED;
        osal_memcpy(&diag_aclRule.smac.octet, mac_data_ptr->octet, ETHER_ADDR_LEN);    
        osal_memset(&diag_aclRule.smac_mask, 0xFF, sizeof(rtk_mac_t));    
    }
    else if ('d' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.dmac_en = ENABLED;
        osal_memcpy(&diag_aclRule.dmac.octet, mac_data_ptr->octet, ETHER_ADDR_LEN);    
        osal_memset(&diag_aclRule.dmac_mask, 0xFF, sizeof(rtk_mac_t));    
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_dmac_smac_data_mac_data */

/*
 * acl set rule ( sip | dip ) data <IPV4ADDR:ip_data> mask <IPV4ADDR:ip_mask>
 */
cparser_result_t
cparser_cmd_acl_set_rule_sip_dip_data_ip_data_mask_ip_mask(
    cparser_context_t *context,
    uint32_t  *ip_data_ptr,
    uint32_t  *ip_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    if ('s' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.sip_en = ENABLED;
        diag_aclRule.sip = *ip_data_ptr;
        diag_aclRule.sip_mask = *ip_mask_ptr;
    }
    else if ('d' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.dip_en = ENABLED;
        diag_aclRule.dip = *ip_data_ptr;
        diag_aclRule.dip_mask = *ip_mask_ptr;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_sip_dip_data_ip_data_mask_ip_mask */

/*
 * acl set rule ( sip | dip ) data <IPV4ADDR:ip_data>
 */
cparser_result_t
cparser_cmd_acl_set_rule_sip_dip_data_ip_data(
    cparser_context_t *context,
    uint32_t  *ip_data_ptr)
{
    DIAG_UTIL_PARAM_CHK();

    if ('s' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.sip_en = ENABLED;
        diag_aclRule.sip = *ip_data_ptr;
        diag_aclRule.sip_mask = 0xFFFFFFFF;
    }
    else if ('d' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.dip_en = ENABLED;
        diag_aclRule.dip = *ip_data_ptr;
        diag_aclRule.dip_mask = 0xFFFFFFFF;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_sip_dip_data_ip_data */

/*
 * acl set rule ( sipv6 | dipv6 ) data <IPV6ADDR:ip_data> mask <IPV6ADDR:ip_mask>
 */
cparser_result_t
cparser_cmd_acl_set_rule_sipv6_dipv6_data_ip_data_mask_ip_mask(
    cparser_context_t *context,
    char * *ip_data_ptr,
    char * *ip_mask_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_ipv6_addr_t ip;
    
    DIAG_UTIL_PARAM_CHK();

    if ('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&diag_aclRule.sip6.ipv6_addr[0], TOKEN_STR(5)), ret);
        DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&diag_aclRule.sip6_mask.ipv6_addr[0], TOKEN_STR(7)), ret);
        diag_aclRule.sip6_en = ENABLED;
    }
    else if ('d' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&diag_aclRule.dip6.ipv6_addr[0], TOKEN_STR(5)), ret);
        DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&diag_aclRule.dip6_mask.ipv6_addr[0], TOKEN_STR(7)), ret);
        diag_aclRule.dip6_en = ENABLED;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_sipv6_dipv6_data_ip_data_mask_ip_mask */

/*
 * acl set rule ( sipv6 | dipv6 ) data <IPV6ADDR:ip_data> 
 */
cparser_result_t
cparser_cmd_acl_set_rule_sipv6_dipv6_data_ip_data(
    cparser_context_t *context,
    char * *ip_data_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_ipv6_addr_t ip;

    if ('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&diag_aclRule.sip6.ipv6_addr[0], TOKEN_STR(5)), ret);
        diag_aclRule.sip6_en = ENABLED;
        osal_memset(&diag_aclRule.sip6_mask,0xFF,sizeof(rtk_ipv6_addr_t));
    }
    else if ('d' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(diag_util_str2ipv6(&diag_aclRule.dip6.ipv6_addr[0], TOKEN_STR(5)), ret);
        diag_aclRule.dip6_en = ENABLED;
        osal_memset(&diag_aclRule.dip6_mask,0xFF,sizeof(rtk_ipv6_addr_t));
    }
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_sipv6_dipv6_data_ip_data */

/*
 * acl set rule ( ethertype | gemidx-llidx | ip6-nh ) data <UINT:data> mask <UINT:mask>
 */
cparser_result_t
cparser_cmd_acl_set_rule_ethertype_gemidx_llidx_ip6_nh_data_data_mask_mask(
    cparser_context_t *context,
    uint32_t  *data_ptr,
    uint32_t  *mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*data_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);

    if ('e' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.ethertype_en =  ENABLED;
        diag_aclRule.ethertype = *data_ptr;
        diag_aclRule.ethertype_mask = *mask_ptr;
    }
    else if ('g' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.gemid_en=  ENABLED;
        diag_aclRule.gemid = *data_ptr;
        diag_aclRule.gemid_mask = *mask_ptr;
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.nh_en =  ENABLED;
        diag_aclRule.nh = *data_ptr;
        diag_aclRule.nh_mask = *mask_ptr;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_ethertype_gemidx_llidx_ip6_nh_data_data_mask_mask */

/*
 * acl set rule ( ethertype | gemidx-llidx | ip6-nh ) data <UINT:data> 
 */
cparser_result_t
cparser_cmd_acl_set_rule_ethertype_gemidx_llidx_ip6_nh_data_data(
    cparser_context_t *context,
    uint32_t  *data_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*data_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);

    if ('e' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.ethertype_en =  ENABLED;
        diag_aclRule.ethertype = *data_ptr;
        diag_aclRule.ethertype_mask = 0xFFFF;
    }
    else if ('g' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.gemid_en=  ENABLED;
        diag_aclRule.gemid = *data_ptr;
        diag_aclRule.gemid_mask = 0xFFFF;
    }
    else if ('i' == TOKEN_CHAR(3,0))
    {
        diag_aclRule.nh_en =  ENABLED;
        diag_aclRule.nh = *data_ptr;
        diag_aclRule.nh_mask = 0xFFFF;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_ethertype_gemidx_llidx_ip6_nh_data_data */

/*
 * acl set rule range-l4port care-range <MASK_LIST:list> 
 */
cparser_result_t
cparser_cmd_acl_set_rule_range_l4port_care_range_list(
    cparser_context_t *context,
    char * *list_ptr)
{
    int32  ret = RT_ERR_FAILED;
    diag_mask_t mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK8(mask, 5), ret);


    diag_aclRule.rngL4Port_en = ENABLED;
    diag_aclRule.rngL4Port = mask.mask.bits[0];
    diag_aclRule.rngL4Port_mask = 0xFFFF;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_range_l4port_care_range_list */

/*
 * acl set rule range-l4port data <UINT:data> 
 */
cparser_result_t
cparser_cmd_acl_set_rule_range_l4port_data_data(
    cparser_context_t *context,
    uint32_t  *data_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*data_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.rngL4Port_en=  ENABLED;
    diag_aclRule.rngL4Port = *data_ptr;            
    diag_aclRule.rngL4Port_mask = 0xFFFF;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_range_l4port_data_data */

/*
 * acl set rule range-l4port data <UINT:data> mask <UINT:mask>
 */
cparser_result_t
cparser_cmd_acl_set_rule_range_l4port_data_data_mask_mask(
    cparser_context_t *context,
    uint32_t  *data_ptr,
    uint32_t  *mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*data_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.rngL4Port_en=  ENABLED;
    diag_aclRule.rngL4Port = *data_ptr;            
    diag_aclRule.rngL4Port_mask = *mask_ptr;


    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_range_l4port_data_data_mask_mask */

/*
 * acl set rule ( range-vid | range-ip | range-len ) care-range <MASK_LIST:list> 
 */
cparser_result_t
cparser_cmd_acl_set_rule_range_vid_range_ip_range_len_care_range_list(
    cparser_context_t *context,
    char * *list_ptr)
{
    int32  ret = RT_ERR_FAILED;
    diag_mask_t mask;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_MASK8(mask, 5), ret);


    if ('v' == TOKEN_CHAR(3,6))    
    {
        diag_aclRule.rngVid_en = ENABLED;
        diag_aclRule.rngVid = mask.mask.bits[0];
        diag_aclRule.rngVid_mask = 0xFFFF;
    }
    else if ('i' == TOKEN_CHAR(3,6))    
    {
        diag_aclRule.rngIp_en = ENABLED;
        diag_aclRule.rngIp = mask.mask.bits[0];
        diag_aclRule.rngIp_mask = 0xFFFF;
    }
    else if ('l' == TOKEN_CHAR(3,6))    
    {
        diag_aclRule.rngPktLen_en = ENABLED;
        diag_aclRule.rngPktLen = mask.mask.bits[0];
        diag_aclRule.rngPktLen_mask = 0xFFFF;
    }
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_range_vid_range_ip_range_len_care_range_list */

/*
 * acl set rule ( range-vid | range-ip | range-len ) data <UINT:data> 
 */
cparser_result_t
cparser_cmd_acl_set_rule_range_vid_range_ip_range_len_data_data(
    cparser_context_t *context,
    uint32_t  *data_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*data_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);

    if('v' == TOKEN_CHAR(3,6))
    {
        diag_aclRule.rngVid_en=  ENABLED;
        diag_aclRule.rngVid = *data_ptr;            
        diag_aclRule.rngVid_mask = 0xFFFF;
    }    
    else if('l' == TOKEN_CHAR(3,6))
    {

        diag_aclRule.rngPktLen_en=  ENABLED;
        diag_aclRule.rngPktLen = *data_ptr;            
        diag_aclRule.rngPktLen_mask = 0xFFFF;
        
    }    
    else if('i' == TOKEN_CHAR(3,6))
    {
        diag_aclRule.rngIp_en=  ENABLED;
        diag_aclRule.rngIp= *data_ptr;            
        diag_aclRule.rngIp_mask = 0xFFFF;
    }    


    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_range_vid_range_ip_range_len_data_data */

/*
 * acl set rule ( range-vid | range-ip | range-len ) data <UINT:data> mask <UINT:mask>
 */
cparser_result_t
cparser_cmd_acl_set_rule_range_vid_range_ip_range_len_data_data_mask_mask(
    cparser_context_t *context,
    uint32_t  *data_ptr,
    uint32_t  *mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*data_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);

    if('v' == TOKEN_CHAR(3,6))
    {
        diag_aclRule.rngVid_en=  ENABLED;
        diag_aclRule.rngVid = *data_ptr;            
        diag_aclRule.rngVid_mask = *mask_ptr;
    }    
    else if('l' == TOKEN_CHAR(3,6))
    {
        diag_aclRule.rngPktLen_en=  ENABLED;
        diag_aclRule.rngPktLen = *data_ptr;            
        diag_aclRule.rngPktLen_mask = *mask_ptr;
    }    
    else if('i' == TOKEN_CHAR(3,6))
    {
        diag_aclRule.rngIp_en=  ENABLED;
        diag_aclRule.rngIp= *data_ptr;            
        diag_aclRule.rngIp_mask = *mask_ptr;
    }    


    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_range_vid_range_ip_range_len_data_data_mask_mask */



/*
 * acl set rule ctag data <UINT:data> 
 */
cparser_result_t
cparser_cmd_acl_set_rule_ctag_data_data(
    cparser_context_t *context,
    uint32_t  *data_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*data_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.ctag_en =  ENABLED;
    diag_aclRule.ctag = *data_ptr;
    diag_aclRule.ctag_mask = 0xFFFF;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_ctag_data_data */

/*
 * acl set rule ctag data <UINT:data> mask <UINT:mask>
 */
cparser_result_t
cparser_cmd_acl_set_rule_ctag_data_data_mask_mask(
    cparser_context_t *context,
    uint32_t  *data_ptr,
    uint32_t  *mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*data_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.ctag_en =  ENABLED;
    diag_aclRule.ctag = *data_ptr;
    diag_aclRule.ctag_mask = *mask_ptr;


    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_ctag_data_data_mask_mask */

/*
 * acl set rule ctag data vid <UINT:vid> priority <UINT:priority> cfi ( 0 | 1 ) mask vid <UINT:vid_mask> priority <UINT:priority_mask> cfi ( 0 | 1 )
 */
cparser_result_t
cparser_cmd_acl_set_rule_ctag_data_vid_vid_priority_priority_cfi_0_1_mask_vid_vid_mask_priority_priority_mask_cfi_0_1(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *priority_ptr,
    uint32_t  *vid_mask_ptr,
    uint32_t  *priority_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_mask_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_mask_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.ctag_en = ENABLED;
    
    diag_aclRule.ctag = (*vid_ptr) & 0xFFF;
    diag_aclRule.ctag = diag_aclRule.ctag | ((*priority_ptr<<13) & 0xE000);
    if ('1' == TOKEN_CHAR(10,0))    
    {
        diag_aclRule.ctag = diag_aclRule.ctag | (1<<12);
    }

    diag_aclRule.ctag_mask = (*vid_mask_ptr) & 0xFFF;
    diag_aclRule.ctag_mask = diag_aclRule.ctag_mask | ((*priority_mask_ptr<<13) & 0xE000);
    if ('1' == TOKEN_CHAR(17,0))    
    {
        diag_aclRule.ctag_mask = diag_aclRule.ctag_mask | (1<<12);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_ctag_data_vid_vid_priority_priority_cfi_0_1_mask_vid_vid_mask_priority_priority_mask_cfi_0_1 */

/*
 * acl set rule ctag data vid <UINT:vid> priority <UINT:priority> cfi ( 0 | 1 ) 
 */
cparser_result_t
cparser_cmd_acl_set_rule_ctag_data_vid_vid_priority_priority_cfi_0_1(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.ctag_en = ENABLED;
    
    diag_aclRule.ctag = (*vid_ptr) & 0xFFF;
    diag_aclRule.ctag = diag_aclRule.ctag | ((*priority_ptr<<13) & 0xE000);
    if ('1' == TOKEN_CHAR(10,0))    
    {
        diag_aclRule.ctag = diag_aclRule.ctag | (1<<12);
    }

    diag_aclRule.ctag_mask = 0xFFFF;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_ctag_data_vid_vid_priority_priority_cfi_0_1 */

/*
 * acl set rule stag data <UINT:data> 
 */
cparser_result_t
cparser_cmd_acl_set_rule_stag_data_data(
    cparser_context_t *context,
    uint32_t  *data_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*data_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.stag_en =  ENABLED;
    diag_aclRule.stag = *data_ptr;
    diag_aclRule.stag_mask = 0xFFFF;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_stag_data_data */

/*
 * acl set rule stag data <UINT:data> mask <UINT:mask>
 */
cparser_result_t
cparser_cmd_acl_set_rule_stag_data_data_mask_mask(
    cparser_context_t *context,
    uint32_t  *data_ptr,
    uint32_t  *mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*data_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*mask_ptr > 0xFFFF), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.stag_en =  ENABLED;
    diag_aclRule.stag = *data_ptr;
    diag_aclRule.stag_mask = *mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_stag_data_data_mask_mask */




/*
 * acl set rule stag data vid <UINT:vid> priority <UINT:priority> dei ( 0 | 1 ) mask vid <UINT:vid_mask> priority <UINT:priority_mask> dei ( 0 | 1 )
 */
cparser_result_t
cparser_cmd_acl_set_rule_stag_data_vid_vid_priority_priority_dei_0_1_mask_vid_vid_mask_priority_priority_mask_dei_0_1(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *priority_ptr,
    uint32_t  *vid_mask_ptr,
    uint32_t  *priority_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_mask_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_mask_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.stag_en = ENABLED;
    
    diag_aclRule.stag = (*vid_ptr) & 0xFFF;
    diag_aclRule.stag = diag_aclRule.stag | ((*priority_ptr<<13) & 0xE000);
    if ('1' == TOKEN_CHAR(10,0))    
    {
        diag_aclRule.stag = diag_aclRule.stag | (1<<12);
    }

    diag_aclRule.stag_mask = (*vid_mask_ptr) & 0xFFF;
    diag_aclRule.stag_mask = diag_aclRule.stag_mask | ((*priority_mask_ptr<<13) & 0xE000);
    if ('1' == TOKEN_CHAR(17,0))    
    {
        diag_aclRule.stag_mask = diag_aclRule.stag_mask | (1<<12);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_stag_data_vid_vid_priority_priority_dei_0_1_mask_vid_vid_mask_priority_priority_mask_dei_0_1 */

/*
 * acl set rule stag data vid <UINT:vid> priority <UINT:priority> dei ( 0 | 1 ) 
 */
cparser_result_t
cparser_cmd_acl_set_rule_stag_data_vid_vid_priority_priority_dei_0_1(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.stag_en = ENABLED;
    
    diag_aclRule.stag = (*vid_ptr) & 0xFFF;
    diag_aclRule.stag = diag_aclRule.stag | ((*priority_ptr<<13) & 0xE000);
    if ('1' == TOKEN_CHAR(10,0))    
    {
        diag_aclRule.stag = diag_aclRule.stag | (1<<12);
    }

    diag_aclRule.stag_mask = 0xFFFF;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_stag_data_vid_vid_priority_priority_dei_0_1 */

/*
 * acl set rule user-field <UINT:index> data <UINT:data> mask <UINT:mask>
 */
cparser_result_t
cparser_cmd_acl_set_rule_user_field_index_data_data_mask_mask(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *data_ptr,
    uint32_t  *mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr>APOLLO_RAW_ACL_FIELD_SELECTOR_MAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.field_en[*index_ptr] = ENABLED;
    diag_aclRule.field[*index_ptr] = *data_ptr;
    diag_aclRule.field_mask[*index_ptr] = *mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_user_field_index_data_data_mask_mask */

/*
 * acl set rule user-field <UINT:index> data <UINT:data>
 */
cparser_result_t
cparser_cmd_acl_set_rule_user_field_index_data_data(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *data_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr>APOLLO_RAW_ACL_FIELD_SELECTOR_MAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.field_en[*index_ptr] = ENABLED;
    diag_aclRule.field[*index_ptr] = *data_ptr;
    diag_aclRule.field_mask[*index_ptr] = 0xFFFF;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_user_field_index_data_data */

/*
 * acl set rule care-tags { ctag } { ipv4 } { ipv6 } { pppoe } { stag } { tcp } { udp } 
 */
cparser_result_t
cparser_cmd_acl_set_rule_care_tags_ctag_ipv4_ipv6_pppoe_stag_tcp_udp(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    int32 tokenIdx;
    int32 tagsIdx;
    DIAG_UTIL_PARAM_CHK();

    for(tagsIdx = 0; tagsIdx < APOLLO_ACL_RULETAG_END;tagsIdx ++)    
    {
        diag_aclRule.tags[tagsIdx] = DISABLED;
        diag_aclRule.tags_mask[tagsIdx] = DISABLED;
    }
    
    for(tokenIdx = 4; tokenIdx <= TOKEN_NUM();tokenIdx ++)    
    {
        if(!osal_strcmp(TOKEN_STR(tokenIdx),"ctag"))
        {
            diag_aclRule.tags[APOLLO_ACL_RULETAG_CTAG] = ENABLED;
            diag_aclRule.tags_mask[APOLLO_ACL_RULETAG_CTAG] = ENABLED;
        }
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"ipv4"))
        {
            diag_aclRule.tags[APOLLO_ACL_RULETAG_IP4] = ENABLED;
            diag_aclRule.tags_mask[APOLLO_ACL_RULETAG_IP4] = ENABLED;
        }           
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"ipv6"))
        {
            diag_aclRule.tags[APOLLO_ACL_RULETAG_IP6] = ENABLED;
            diag_aclRule.tags_mask[APOLLO_ACL_RULETAG_IP6] = ENABLED;
        }           
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"pppoe"))
        {
            diag_aclRule.tags[APOLLO_ACL_RULETAG_PPPOE] = ENABLED;
            diag_aclRule.tags_mask[APOLLO_ACL_RULETAG_PPPOE] = ENABLED;
        }           
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"stag"))
        {
            diag_aclRule.tags[APOLLO_ACL_RULETAG_STAG] = ENABLED;
            diag_aclRule.tags_mask[APOLLO_ACL_RULETAG_STAG] = ENABLED;
        }           
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"tcp"))
        {
            diag_aclRule.tags[APOLLO_ACL_RULETAG_TCP] = ENABLED;
            diag_aclRule.tags_mask[APOLLO_ACL_RULETAG_TCP] = ENABLED;
        }           
        else if(!osal_strcmp(TOKEN_STR(tokenIdx),"udp"))
        {
            diag_aclRule.tags[APOLLO_ACL_RULETAG_UDP] = ENABLED;
            diag_aclRule.tags_mask[APOLLO_ACL_RULETAG_UDP] = ENABLED;
        }           
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_care_tags_ctag_ipv4_ipv6_pppoe_stag_tcp_udp */

/*
 * acl set rule port ( <PORT_LIST:port> | all | none )
 */
cparser_result_t
cparser_cmd_acl_set_rule_port_port_all_none(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 4), ret);

    diag_aclRule.Port.bits[0] = portlist.portmask.bits[0];
    diag_aclRule.Port_mask.bits[0] = APOLLO_PORTMASK;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_port_port_all */

/*
 * acl set rule template index <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_set_rule_template_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > APOLLO_ACLTEMPLATEMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclRule.tempIdx = *index_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_template_index_index */

/*
 * acl set rule operation ( hit | not ) 
 */
cparser_result_t
cparser_cmd_acl_set_rule_operation_hit_not(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    if ('h' == TOKEN_CHAR(4,0))
    {
        diag_aclActCtrl.not = DISABLED;
    }
    else if ('n' == TOKEN_CHAR(4,0))
    {
        diag_aclActCtrl.not = ENABLED;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_operation_hit_not */

/*
 * acl set rule ( valid | invalid ) 
 */
cparser_result_t
cparser_cmd_acl_set_rule_valid_invalid(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    if ('i' == TOKEN_CHAR(3,0))
    {
        diag_aclActCtrl.valid = DISABLED;
    }
    else if ('v' == TOKEN_CHAR(3,0))
    {
        diag_aclActCtrl.valid = ENABLED;
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_rule_valid */

/*
 * acl set entry <UINT:entry> operation ( hit | not )
 */
cparser_result_t
cparser_cmd_acl_set_entry_entry_operation_hit_not(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_actCtrl_t aclActCtrl;

    DIAG_UTIL_PARAM_CHK();

    aclActCtrl.idx = *entry_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_actCtrl_get(&aclActCtrl), ret);

    if ('h' == TOKEN_CHAR(5,0))
    {
        aclActCtrl.not = DISABLED;
    }
    else if ('n' == TOKEN_CHAR(5,0))
    {
        aclActCtrl.not = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_actCtrl_set(&aclActCtrl), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_entry_entry_operation_hit_not */

/*
 * acl get entry <UINT:entry> operation 
 */
cparser_result_t
cparser_cmd_acl_get_entry_entry_operation(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_actCtrl_t aclActCtrl;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    aclActCtrl.idx = *entry_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_actCtrl_get(&aclActCtrl), ret);
   

    diag_util_mprintf("Operation: %s\n", diagStr_aclOper[aclActCtrl.not]);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_get_entry_entry_operation */

/*
 * acl set entry <UINT:entry> ( valid | invalid )
 */
cparser_result_t
cparser_cmd_acl_set_entry_entry_valid_invalid(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    apollo_raw_acl_actCtrl_t aclActCtrl;

    DIAG_UTIL_PARAM_CHK();

    aclActCtrl.idx = *entry_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_actCtrl_get(&aclActCtrl), ret);
    
    if ('v' == TOKEN_CHAR(4,0))
    {
        aclActCtrl.valid = ENABLED;
    }
    else if ('i' == TOKEN_CHAR(4,0))
    {
        aclActCtrl.valid = DISABLED;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_actCtrl_set(&aclActCtrl), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_entry_entry_valid_invalid */

/*
 * acl get entry <UINT:entry> action
 */
cparser_result_t
cparser_cmd_acl_get_entry_entry_action(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_ruleEntry_t rawRule;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    diag_aclActCtrl.idx = *entry_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_actCtrl_get(&diag_aclActCtrl), ret);

    diag_aclAct.idx = *entry_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_act_get(&diag_aclAct), ret);
  
    _diag_aclActShow(&diag_aclActCtrl,&diag_aclAct);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_get_entry_entry_action */


/*
 * acl get range-vid index <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_get_range_vid_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_vidRange_t vidRange;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    vidRange.idx = *index_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_vidRange_get(&vidRange), ret);

    diag_util_mprintf("Range check of VID\n");
    diag_util_mprintf("Index: %d Upper: %d Lower: %d Type: %s\n",
                                                    vidRange.idx, 
                                                    vidRange.upperVid, 
                                                    vidRange.lowerVid,
                                                    diagStr_aclRangeCheckVidTypeStr[vidRange.type]);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_get_range_vid_index_index */

/*
 * acl set range-vid index <UINT:index> invalid
 */
cparser_result_t
cparser_cmd_acl_set_range_vid_index_index_invalid(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_vidRange_t vidRange;

    osal_memset(&vidRange, 0x0,sizeof(apollo_raw_acl_vidRange_t));

    vidRange.idx = *index_ptr;
    vidRange.type = APOLLO_ACL_RNGVIDTYPE_NONVALID;

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_vidRange_set(&vidRange), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_range_vid_index_index_invalid */

/*
 * acl set range-vid index <UINT:index> ( svid | cvid ) low-bound <UINT:vid_lower> up-bound <UINT:vid_upper>
 */
cparser_result_t
cparser_cmd_acl_set_range_vid_index_index_svid_cvid_low_bound_vid_lower_up_bound_vid_upper(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *vid_lower_ptr,
    uint32_t  *vid_upper_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_vidRange_t vidRange;

    osal_memset(&vidRange, 0x0,sizeof(apollo_raw_acl_vidRange_t));

    vidRange.idx = *index_ptr;

    if ('s' == TOKEN_CHAR(5,0))
    {
        vidRange.type = APOLLO_ACL_RNGVIDTYPE_SVLAN;
    }
    else if ('c' == TOKEN_CHAR(5,0))
    {
        vidRange.type = APOLLO_ACL_RNGVIDTYPE_CVLAN;
    }

    vidRange.upperVid = *vid_upper_ptr;
    vidRange.lowerVid = *vid_lower_ptr;        
    
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_vidRange_set(&vidRange), ret);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_range_vid_index_index_svid_cvid_low_bound_vid_lower_up_bound_vid_upper */

/*
 * acl get range-ip index <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_get_range_ip_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_ipRange_t ipRange;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    ipRange.idx = *index_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_ipRange_get(&ipRange), ret);

    diag_util_mprintf("Range check of IP address\n");
    diag_util_mprintf("Index: %d Upper: %s ",
                                                    ipRange.idx, 
                                                    diag_util_inet_ntoa(ipRange.upperIp)); 
    diag_util_mprintf("Lower: %s Type: %s\n",
                                                    diag_util_inet_ntoa(ipRange.lowerIp),
                                                    diagStr_aclRangeCheckIpTypeStr[ipRange.type]);
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_get_range_ip_index_index */

/*
 * acl set range-ip index <UINT:index> invalid
 */
cparser_result_t
cparser_cmd_acl_set_range_ip_index_index_invalid(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_ipRange_t ipRange;

    osal_memset(&ipRange, 0x0,sizeof(apollo_raw_acl_ipRange_t));

    ipRange.idx = *index_ptr;
    ipRange.type = APOLLO_ACL_RNGIPTYPE_NONVALID;

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_ipRange_set(&ipRange), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_range_ip_index_index_invalid */

/*
 * acl set range-ip index <UINT:index> ( sip | dip | sipv6 | dipv6 ) low-bound <IPV4ADDR:ip_lower> up-bound <IPV4ADDR:ip_upper>
 */
cparser_result_t
cparser_cmd_acl_set_range_ip_index_index_sip_dip_sipv6_dipv6_low_bound_ip_lower_up_bound_ip_upper(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *ip_lower_ptr,
    uint32_t  *ip_upper_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_ipRange_t ipRange;

    osal_memset(&ipRange, 0x0,sizeof(apollo_raw_acl_ipRange_t));

    ipRange.idx = *index_ptr;

    ipRange.lowerIp = *ip_lower_ptr;
    ipRange.upperIp = *ip_upper_ptr;

    if(!osal_strcmp(TOKEN_STR(5),"sipv6"))
    {
        ipRange.type = APOLLO_ACL_RNGIPTYPE_V6SIP;
    }
    else if(!osal_strcmp(TOKEN_STR(5),"dipv6"))
    {
        ipRange.type = APOLLO_ACL_RNGIPTYPE_V6DIP;
    }
    else if(!osal_strcmp(TOKEN_STR(5),"sip"))
    {
        ipRange.type = APOLLO_ACL_RNGIPTYPE_V4SIP;
    }
    else if(!osal_strcmp(TOKEN_STR(5),"dip"))
    {
        ipRange.type = APOLLO_ACL_RNGIPTYPE_V4DIP;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_ipRange_set(&ipRange), ret);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_range_ip_index_index_sip_dip_sipv6_dipv6_low_bound_ip_lower_up_bound_ip_upper */

/*
 * acl get range-l4port index <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_get_range_l4port_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_portRange_t portRange;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    portRange.idx = *index_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_portRange_get(&portRange), ret);

    diag_util_mprintf("Range check of L4 port\n");
    diag_util_mprintf("Index: %d Upper: %d Lower: %d Type: %s\n",
                                                    portRange.idx, 
                                                    portRange.upperPort, 
                                                    portRange.lowerPort,
                                                    diagStr_aclRangeCheckPortTypeStr[portRange.type]);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_get_range_l4port_index_index */

/*
 * acl set range-l4port index <UINT:index> invalid
 */
cparser_result_t
cparser_cmd_acl_set_range_l4port_index_index_invalid(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_portRange_t portRange;

    osal_memset(&portRange, 0x0,sizeof(apollo_raw_acl_portRange_t));

    portRange.idx = *index_ptr;
    portRange.type = APOLLO_ACL_RNGL4PORTTYPE_NONVALID;

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_portRange_set(&portRange), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_range_l4port_index_index_invalid */

/*
 * acl set range-l4port index <UINT:index> ( src-port | dest-port ) low-bound <UINT:port_lower> up-bound <UINT:port_upper>
 */
cparser_result_t
cparser_cmd_acl_set_range_l4port_index_index_src_port_dest_port_low_bound_port_lower_up_bound_port_upper(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *port_lower_ptr,
    uint32_t  *port_upper_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_portRange_t portRange;

    osal_memset(&portRange, 0x0,sizeof(apollo_raw_acl_portRange_t));

    portRange.idx = *index_ptr;
    portRange.lowerPort = *port_lower_ptr;
    portRange.upperPort = *port_upper_ptr;

    if ('s' == TOKEN_CHAR(5,0))
    {
        portRange.type = APOLLO_ACL_RNGL4PORTTYPE_SRC;
    }
    else if ('d' == TOKEN_CHAR(5,0))
    {
        portRange.type = APOLLO_ACL_RNGL4PORTTYPE_DEST;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_portRange_set(&portRange), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_range_l4port_index_index_src_port_dest_port_low_bound_port_lower_up_bound_port_upper */

/*
 * acl get range-len index <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_get_range_len_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_pktLenRange_t pktLenRange;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    pktLenRange.idx = *index_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_pktLenRange_get(&pktLenRange), ret);

    diag_util_mprintf("Range check of packet length\n");
    diag_util_mprintf("Index: %d Upper: %d Lower: %d Type: %s\n",
                                                    pktLenRange.idx, 
                                                    pktLenRange.upperPktLen, 
                                                    pktLenRange.lowerPktLen,
                                                    diagStr_aclRangeCheckLenTypeStr[pktLenRange.type]);
   
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_get_range_len_index_index */

/*
 * acl set range-len index <UINT:index> low-bound <UINT:len_lower> up-bound <UINT:len_upper> { reverse }
 */
cparser_result_t
cparser_cmd_acl_set_range_len_index_index_low_bound_len_lower_up_bound_len_upper_reverse(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *len_lower_ptr,
    uint32_t  *len_upper_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_pktLenRange_t pktLenRange;

    osal_memset(&pktLenRange, 0x0,sizeof(apollo_raw_acl_pktLenRange_t));

    pktLenRange.idx = *index_ptr;
    pktLenRange.lowerPktLen = *len_lower_ptr;
    pktLenRange.upperPktLen = *len_upper_ptr;

    if (9 == TOKEN_NUM())
    {
        pktLenRange.type = APOLLO_ACL_RNGPKTLENTYPE_NOTREVISE;
    }
    else 
    {
        pktLenRange.type = APOLLO_ACL_RNGPKTLENTYPE_REVISE;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_pktLenRange_set(&pktLenRange), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_range_len_index_index_low_bound_len_lower_up_bound_len_upper_reverse */

/*
 * acl set range-len index <UINT:index> invalid
 */
cparser_result_t
cparser_cmd_acl_set_range_len_index_index_invalid(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_pktLenRange_t pktLenRange;

    osal_memset(&pktLenRange, 0x0,sizeof(apollo_raw_acl_pktLenRange_t));

    pktLenRange.idx = *index_ptr;
    pktLenRange.type = APOLLO_ACL_RNGPKTLENTYPE_NOTREVISE;

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_pktLenRange_set(&pktLenRange), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_range_len_index_index_invalid */

/*
 * acl get port ( <PORT_LIST:port> | all ) state
 */
cparser_result_t
cparser_cmd_acl_get_port_port_all_state(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);


    diag_util_mprintf("Port State\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_acl_state_get(port, &enable), ret);
        diag_util_mprintf("%-5d%s\n", port, diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_get_port_port_all_state */

/*
 * acl set port ( <PORT_LIST:port> | all ) state ( enable | disable )
 */
cparser_result_t
cparser_cmd_acl_set_port_port_all_state_enable_disable(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_acl_state_set(port, enable), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_port_port_all_state_enable_disable */

/*
 * acl get port ( <PORT_LIST:port> | all ) permit
 */
cparser_result_t
cparser_cmd_acl_get_port_port_all_permit(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    diag_util_mprintf("Port State\n");
    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
       DIAG_UTIL_ERR_CHK(apollo_raw_acl_permit_get(port, &enable), ret);

       diag_util_mprintf("%-5d%s\n", port, diagStr_enable[enable]);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_get_port_port_all_permit */

/*
 * acl set port ( <PORT_LIST:port> | all ) permit ( enable | disable )
 */
cparser_result_t
cparser_cmd_acl_set_port_port_all_permit_enable_disable(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    rtk_port_t port = 0;
    rtk_enable_t enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 3), ret);

    if ('d' == TOKEN_CHAR(5,0))
    {
        enable = DISABLED;
    }
    else if ('e' == TOKEN_CHAR(5,0))
    {
        enable = ENABLED;
    }

    DIAG_UTIL_PORTMASK_SCAN(portlist, port)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_acl_permit_set(port, enable), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_port_port_all_permit_enable_disable */

/*
 * acl set action cvlan ingress cvidx <UINT:cvidx>
 */
cparser_result_t
cparser_cmd_acl_set_action_cvlan_ingress_cvidx_cvidx(
    cparser_context_t *context,
    uint32_t  *cvidx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*cvidx_ptr > APOLLO_CVIDXMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.cvlan_en = ENABLED;
    diag_aclAct.cact = APOLLO_ACL_CACT_INGRESS;
    diag_aclAct.cvidx_cact = *cvidx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_cvlan_ingress_cvidx_cvidx */

/*
 * acl set action cvlan egress cvidx <UINT:cvidx>
 */
cparser_result_t
cparser_cmd_acl_set_action_cvlan_egress_cvidx_cvidx(
    cparser_context_t *context,
    uint32_t  *cvidx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*cvidx_ptr > APOLLO_CVIDXMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.cvlan_en = ENABLED;
    diag_aclAct.cact = APOLLO_ACL_CACT_EGRESS;
    diag_aclAct.cvidx_cact = *cvidx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_cvlan_egress_cvidx_cvidx */

/*
 * acl set action cvlan using-svid
 */
cparser_result_t
cparser_cmd_acl_set_action_cvlan_using_svid(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_aclActCtrl.cvlan_en = ENABLED;
    diag_aclAct.cact = APOLLO_ACL_CACT_SVID;
    diag_aclAct.cvidx_cact = 0;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_cvlan_using_svid */

/*
 * acl set action cvlan policing meter <UINT:meter_idx> 
 */
cparser_result_t
cparser_cmd_acl_set_action_cvlan_policing_meter_meter_idx(
    cparser_context_t *context,
    uint32_t  *meter_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*meter_idx_ptr >= HAL_MAX_NUM_OF_METERING()), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.cvlan_en = ENABLED;
    diag_aclAct.cact = APOLLO_ACL_CACT_POLICE;
    diag_aclAct.cvidx_cact = *meter_idx_ptr;


    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_cvlan_policing_meter_meter_idx */

/*
 * acl set action cvlan logging acl-mib <UINT:mib_idx> 
 */
cparser_result_t
cparser_cmd_acl_set_action_cvlan_logging_acl_mib_mib_idx(
    cparser_context_t *context,
    uint32_t  *mib_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*mib_idx_ptr > APOLLO_ACL_MIB_MAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.cvlan_en = ENABLED;
    diag_aclAct.cact = APOLLO_ACL_CACT_LOGGING;
    diag_aclAct.cvidx_cact = *mib_idx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_cvlan_logging_acl_mib_mib_idx */

/*
 * acl set action cvlan dot1p-remark priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_acl_set_action_cvlan_dot1p_remark_priority_pri(
    cparser_context_t *context,
    uint32_t  *pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.cvlan_en = ENABLED;
    diag_aclAct.cact = APOLLO_ACL_CACT_1PREMARK;
    diag_aclAct.cvidx_cact = *pri_ptr;
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_cvlan_dot1p_remark_priority_pri */

/*
 * acl set action svlan ingress svidx <UINT:svidx>
 */
cparser_result_t
cparser_cmd_acl_set_action_svlan_ingress_svidx_svidx(
    cparser_context_t *context,
    uint32_t  *svidx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*svidx_ptr > APOLLO_SVIDXMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.svlan_en = ENABLED;
    diag_aclAct.sact = APOLLO_ACL_SACT_INGRESS;
    diag_aclAct.svidx_sact = *svidx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_svlan_ingress_svidx_svidx */

/*
 * acl set action svlan egress svidx <UINT:svidx>
 */
cparser_result_t
cparser_cmd_acl_set_action_svlan_egress_svidx_svidx(
    cparser_context_t *context,
    uint32_t  *svidx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*svidx_ptr > APOLLO_SVIDXMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.svlan_en = ENABLED;
    diag_aclAct.sact = APOLLO_ACL_SACT_EGRESS;
    diag_aclAct.svidx_sact = *svidx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_svlan_egress_svidx_svidx */

/*
 * acl set action svlan using-cvid
 */
cparser_result_t
cparser_cmd_acl_set_action_svlan_using_cvid(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_aclActCtrl.svlan_en = ENABLED;
    diag_aclAct.sact = APOLLO_ACL_SACT_CVID;
    diag_aclAct.svidx_sact = 0;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_svlan_using_cvid */

/*
 * acl set action svlan policing meter <UINT:meter_idx> 
 */
cparser_result_t
cparser_cmd_acl_set_action_svlan_policing_meter_meter_idx(
    cparser_context_t *context,
    uint32_t  *meter_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*meter_idx_ptr >= HAL_MAX_NUM_OF_METERING()), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.svlan_en = ENABLED;
    diag_aclAct.sact = APOLLO_ACL_SACT_POLICE;
    diag_aclAct.svidx_sact = *meter_idx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_svlan_policing_meter_meter_idx */

/*
 * acl set action svlan logging acl-mib <UINT:mib_idx> 
 */
cparser_result_t
cparser_cmd_acl_set_action_svlan_logging_acl_mib_mib_idx(
    cparser_context_t *context,
    uint32_t  *mib_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*mib_idx_ptr > APOLLO_ACL_MIB_MAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.svlan_en = ENABLED;
    diag_aclAct.sact = APOLLO_ACL_SACT_LOGGING;
    diag_aclAct.svidx_sact = *mib_idx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_svlan_logging_acl_mib_mib_idx */

/*
 * acl set action svlan dscp-remark dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_acl_set_action_svlan_dscp_remark_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscp_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*dscp_ptr > APOLLO_DSCPMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.svlan_en = ENABLED;
    diag_aclAct.sact = APOLLO_ACL_SACT_DSCPREMARK;
    diag_aclAct.svidx_sact = *dscp_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_svlan_dscp_remark_dscp_dscp */

/*
 * acl set action svlan dot1p-remark priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_acl_set_action_svlan_dot1p_remark_priority_pri(
    cparser_context_t *context,
    uint32_t  *pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.svlan_en = ENABLED;
    diag_aclAct.sact = APOLLO_ACL_SACT_1PREMARK;
    diag_aclAct.svidx_sact = *pri_ptr;
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_svlan_dot1p_remark_priority_pri */

/*
 * acl set action priority pri-assign priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_acl_set_action_priority_pri_assign_priority_pri(
    cparser_context_t *context,
    uint32_t  *pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.pri_remark_en = ENABLED;
    diag_aclAct.priact = APOLLO_ACL_PRIACT_PRI;
    diag_aclAct.pridx = *pri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_priority_pri_assign_priority_pri */

/*
 * acl set action priority dscp-remark dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_acl_set_action_priority_dscp_remark_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *dscp_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*dscp_ptr > APOLLO_DSCPMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.pri_remark_en = ENABLED;
    diag_aclAct.priact = APOLLO_ACL_PRIACT_DSCPREMARK;
    diag_aclAct.pridx = *dscp_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_priority_dscp_remark_dscp_dscp */

/*
 * acl set action priority dot1p-remark priority <UINT:pri>
 */
cparser_result_t
cparser_cmd_acl_set_action_priority_dot1p_remark_priority_pri(
    cparser_context_t *context,
    uint32_t  *pri_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*pri_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.pri_remark_en = ENABLED;
    diag_aclAct.priact = APOLLO_ACL_PRIACT_1PREMARK;
    diag_aclAct.pridx = *pri_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_priority_dot1p_remark_priority_pri */

/*
 * acl set action priority policing meter <UINT:meter_idx> 
 */
cparser_result_t
cparser_cmd_acl_set_action_priority_policing_meter_meter_idx(
    cparser_context_t *context,
    uint32_t  *meter_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*meter_idx_ptr >= HAL_MAX_NUM_OF_METERING()), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.pri_remark_en = ENABLED;
    diag_aclAct.priact = APOLLO_ACL_PRIACT_POLICE;
    diag_aclAct.pridx = *meter_idx_ptr;
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_priority_policing_meter_meter_idx */

/*
 * acl set action priority logging acl-mib <UINT:mib_idx> 
 */
cparser_result_t
cparser_cmd_acl_set_action_priority_logging_acl_mib_mib_idx(
    cparser_context_t *context,
    uint32_t  *mib_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*mib_idx_ptr > APOLLO_ACL_MIB_MAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.pri_remark_en = ENABLED;
    diag_aclAct.priact = APOLLO_ACL_PRIACT_LOGGING;
    diag_aclAct.pridx = *mib_idx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_priority_logging_acl_mib_mib_idx */

/*
 * acl set action policing meter <UINT:meter_idx> 
 */
cparser_result_t
cparser_cmd_acl_set_action_policing_meter_meter_idx(
    cparser_context_t *context,
    uint32_t  *meter_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*meter_idx_ptr >= HAL_MAX_NUM_OF_METERING()), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.polic_en = ENABLED;
    diag_aclAct.policact = APOLLO_ACL_PACT_POLICE;
    diag_aclAct.meteridx = *meter_idx_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_policing_meter_meter_idx */

/*
 * acl set action logging acl-mib <UINT:mib_idx> 
 */
cparser_result_t
cparser_cmd_acl_set_action_logging_acl_mib_mib_idx(
    cparser_context_t *context,
    uint32_t  *mib_idx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*mib_idx_ptr > APOLLO_ACL_MIB_MAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.polic_en = ENABLED;
    diag_aclAct.policact = APOLLO_ACL_PACT_LOGGING;
    diag_aclAct.meteridx = *mib_idx_ptr;
        
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_logging_acl_mib_mib_idx */

/*
 * acl set action trap-to-cpu
 */
cparser_result_t
cparser_cmd_acl_set_action_trap_to_cpu(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_aclActCtrl.fwd_en = ENABLED;
    diag_aclAct.fwdact = APOLLO_ACL_FWDACT_TRAP;
    diag_aclAct.fwdpmask = 0x0;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_trap_to_cpu */

/*
 * acl set action copy port ( <PORT_LIST:port> | all ) 
 */
cparser_result_t
cparser_cmd_acl_set_action_copy_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_aclActCtrl.fwd_en = ENABLED;
    diag_aclAct.fwdact = APOLLO_ACL_FWDACT_COPY;
    diag_aclAct.fwdpmask = portlist.portmask.bits[0];

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_copy_port_port_all */

/*
 * acl set action redirect port ( <PORT_LIST:port> | all | none ) 
 */
cparser_result_t
cparser_cmd_acl_set_action_redirect_port_port_all_none(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_aclActCtrl.fwd_en = ENABLED;
    diag_aclAct.fwdact = APOLLO_ACL_FWDACT_REDIRECT;
    diag_aclAct.fwdpmask = portlist.portmask.bits[0];

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_redirect_port_port_all */

/*
 * acl set action mirror port ( <PORT_LIST:port> | all ) 
 */
cparser_result_t
cparser_cmd_acl_set_action_mirror_port_port_all(
    cparser_context_t *context,
    char * *port_ptr)
{
    int32 ret = RT_ERR_FAILED;
    diag_portlist_t portlist;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 5), ret);

    diag_aclActCtrl.fwd_en = ENABLED;
    diag_aclAct.fwdact = APOLLO_ACL_FWDACT_MIRROR;
    diag_aclAct.fwdpmask = portlist.portmask.bits[0];

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_mirror_port_port_all */


/*
 * acl set action interrupt
 */
cparser_result_t
cparser_cmd_acl_set_action_interrupt(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_aclActCtrl.int_cf_en = ENABLED;
    diag_aclAct.aclint = ENABLED;

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_interrupt */

/*
 * acl set action classify none 
 */
cparser_result_t
cparser_cmd_acl_set_action_classify_none(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_aclActCtrl.int_cf_en = ENABLED;
    diag_aclAct.cfact = APOLLO_ACL_CFACT_NONE;
    diag_aclAct.cfidx = 0;        

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_classify_none */

/*
 * acl set action classify sid <UINT:sid> 
 */
cparser_result_t
cparser_cmd_acl_set_action_classify_sid_sid(
    cparser_context_t *context,
    uint32_t  *sid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*sid_ptr > APOLLO_SID_MAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.int_cf_en = ENABLED;
    diag_aclAct.cfact = APOLLO_ACL_CFACT_SID;
    diag_aclAct.cfidx = *sid_ptr;        

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_classify_sid_sid */

/*
 * acl set action classify llid <UINT:llid> 
 */
cparser_result_t
cparser_cmd_acl_set_action_classify_llid_llid(
    cparser_context_t *context,
    uint32_t  *llid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*llid_ptr > APOLLO_LLID_MAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.int_cf_en = ENABLED;
    diag_aclAct.cfact = APOLLO_ACL_CFACT_LLID;
    diag_aclAct.cfidx = *llid_ptr;        

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_classify_llid_llid */

/*
 * acl set action classify ext-dsl <UINT:vidx> 
 */
cparser_result_t
cparser_cmd_acl_set_action_classify_ext_dsl_vidx(
    cparser_context_t *context,
    uint32_t  *vidx_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*vidx_ptr > APOLLO_CVIDXMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_aclActCtrl.int_cf_en = ENABLED;
    diag_aclAct.cfact = APOLLO_ACL_CFACT_VCEXT;
    diag_aclAct.cfidx = *vidx_ptr;        

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_classify_ext_dsl_vidx */

/*
 * acl set action none
 */
cparser_result_t
cparser_cmd_acl_set_action_none(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_aclActCtrl,0x0,sizeof(apollo_raw_acl_actCtrl_t));
     
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_action_none */

/*
 * acl show action 
 */
cparser_result_t
cparser_cmd_acl_show_action(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    _diag_aclActShow(&diag_aclActCtrl,&diag_aclAct);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_show_action */

/*
 * acl clear action 
 */
cparser_result_t
cparser_cmd_acl_clear_action(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_aclActCtrl, 0x0, sizeof(apollo_raw_acl_actCtrl_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_clear_action */


/*
 * acl show template
 */
cparser_result_t
cparser_cmd_acl_show_template(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_OUTPUT_INIT();
    _diag_aclTemplateShow(&diag_aclTemplate);
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_show_template */

/*
 * acl clear template
 */
cparser_result_t
cparser_cmd_acl_clear_template(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_aclTemplate, 0x0, sizeof(apollo_raw_acl_template_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_clear_template */

/*
 * acl set template ( stag | ctag | ethertype | gemidx-llidx | ip6-nh | unknown | range-ip )
 */
cparser_result_t
cparser_cmd_acl_set_template_stag_ctag_ethertype_gemidx_llidx_ip6_nh_unknown_range_ip(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    if('c' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_CTAG), ret);            
    }
    else if('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_STAG), ret);            
    }
    else if('e' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_ETHERTYPE), ret);            
    }
    else if('g' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_GEMIDXLLIDX), ret);            
    }
    else if('i' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_IP6NH), ret);            
    }
    else if('u' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_TYPE_END), ret);            
    }
    else if('r' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_IPRANGE), ret);            
    }


    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_template_stag_ctag_ethertype_gemidx_llidx_ip6_nh_unknown_range_ip */

/*
 * acl set template ( dip | sip | range-len )
 */
cparser_result_t
cparser_cmd_acl_set_template_dip_sip_range_len(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    if('d' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_IP4DIP0), ret);            
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_IP4DIP1), ret);            
    }
    else if('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_IP4SIP0), ret);            
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_IP4SIP1), ret);            
    }
    else if('r' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_PKTLENRANGE), ret);            
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_template_dip_sip_range_len */

/*
 * acl set template ( smac | dmac | range-l4port )
 */
cparser_result_t
cparser_cmd_acl_set_template_smac_dmac_range_l4port(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    if('d' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_DMAC0), ret);            
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_DMAC1), ret);            
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_DMAC2), ret);            
    }
    else if('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_SMAC0), ret);            
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_SMAC1), ret);            
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_SMAC2), ret);            
    }
    else if('r' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_PORTRANGE), ret);            
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_template_smac_dmac_range_l4port */

/*
 * acl set template ( sipv6 | dipv6 | range-vid )
 */
cparser_result_t
cparser_cmd_acl_set_template_sipv6_dipv6_range_vid(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    DIAG_UTIL_PARAM_CHK();

    if('d' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_IP6DIP0WITHIPV4), ret);            
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_IP6DIP1WITHIPV4), ret);            
    }
    else if('s' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_IP6SIP0WITHIPV4), ret);            
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_IP6SIP1WITHIPV4), ret);            
    }
    else if('r' == TOKEN_CHAR(3,0))
    {
        DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_VIDRANGE), ret);            
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_template_sipv6_dipv6_range_vid */

/*
 * acl set template user-field <UINT:index> 
 */
cparser_result_t
cparser_cmd_acl_set_template_user_field_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr > APOLLO_RAW_ACL_FIELD_SELECTOR_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(_diag_aclTemplateAdd(ACL_FIELD_SELECT00 + *index_ptr), ret);            

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_template_user_field_index */


/*
 * acl ( add | del | get ) template index <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_add_del_get_template_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_template_t aclTmeplate;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr >= APOLLO_ACLTEMPLATENO), CPARSER_ERR_INVALID_PARAMS);

    if('a' == TOKEN_CHAR(1,0))
    {
        diag_aclTemplate.idx = *index_ptr;
        DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_set(&diag_aclTemplate), ret);
    }
    else if('d' == TOKEN_CHAR(1,0))
    {
        osal_memset(&aclTmeplate, 0x0, sizeof(apollo_raw_acl_template_t));
        aclTmeplate.idx = *index_ptr;
        DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_set(&aclTmeplate), ret);
    }
    else if('g' == TOKEN_CHAR(1,0))
    {
        aclTmeplate.idx = *index_ptr;
        DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_get(&aclTmeplate), ret);
        _diag_aclTemplateShow(&aclTmeplate);

    }
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_add_del_get_template_index_index */

/*
 * acl add template index <UINT:index> ( msb-rule | lsb-rule )
 */
cparser_result_t
cparser_cmd_acl_add_template_index_index_msb_rule_lsb_rule(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_template_t aclTmeplate;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*index_ptr >= APOLLO_ACLTEMPLATENO), CPARSER_ERR_INVALID_PARAMS);

    osal_memset(&aclTmeplate, 0x0, sizeof(apollo_raw_acl_template_t));
    aclTmeplate.idx = *index_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_get(&aclTmeplate), ret);


    if('m' == TOKEN_CHAR(5,0))
    {
        aclTmeplate.field[4] = diag_aclTemplate.field[0]; 
        aclTmeplate.field[5] = diag_aclTemplate.field[1]; 
        aclTmeplate.field[6] = diag_aclTemplate.field[2]; 
        aclTmeplate.field[7] = diag_aclTemplate.field[3]; 
    }
    else if('l' == TOKEN_CHAR(5,0))
    {
        aclTmeplate.field[0] = diag_aclTemplate.field[0]; 
        aclTmeplate.field[1] = diag_aclTemplate.field[1]; 
        aclTmeplate.field[2] = diag_aclTemplate.field[2]; 
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_template_set(&aclTmeplate), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_add_template_index_index_msb_rule_lsb_rule */


/*
 * acl set mode ( 64-entries | 128-entries )
 */
cparser_result_t
cparser_cmd_acl_set_mode_64_entries_128_entries(
    cparser_context_t *context)
{
    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_modeTypes_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    
    if ('6' == TOKEN_CHAR(3,0))
    {
        mode = APOLLO_ACL_MODE_64ENTRIES;
    }
    else if ('1' == TOKEN_CHAR(3,0))
    {
        mode = APOLLO_ACL_MODE_128ENTRIES;
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_acl_mode_set(mode), ret);

    diag_aclRule.mode = mode;
    
    return CPARSER_OK;
}    /* end of cparser_cmd_acl_set_mode_64_entries_128_entries */

/*
 * acl get mode 
 */
cparser_result_t
cparser_cmd_acl_get_mode(
    cparser_context_t *context)
{

    int32 ret = RT_ERR_FAILED;
    apollo_raw_acl_modeTypes_t mode;
    
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
   
    DIAG_UTIL_ERR_CHK(apollo_raw_acl_mode_get(&mode), ret);
    diag_aclRule.mode = mode;

    diag_util_mprintf("ACL mode: %s\n", diagStr_aclModeStr[mode]);

    return CPARSER_OK;
}    /* end of cparser_cmd_acl_get_mode */


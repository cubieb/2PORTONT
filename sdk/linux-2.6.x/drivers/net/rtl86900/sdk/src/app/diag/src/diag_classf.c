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
#include <common/type.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <diag_util.h>
#include <parser/cparser_priv.h>
#include <diag_str.h>
#include <dal/apollo/raw/apollo_raw_cf.h>

/* symbol declare */
typedef struct diag_classf_ruleCfg_s
{
    rtk_enable_t direct_en;
    rtk_direct_t direct;

    rtk_enable_t ethertype_en;
    uint32 ethertype;
    uint32 ethertype_mask;

    rtk_enable_t tos_sid_en;
    uint32 tos_sid;
    uint32 tos_sid_mask;

    rtk_enable_t tag_vid_en;
    uint32 tag_vid;
    uint32 tag_vid_mask;

    rtk_enable_t tag_pri_en;
    uint32 tag_pri;
    uint32 tag_pri_mask;

    rtk_enable_t inter_pri_en;
    uint32 inter_pri;
    uint32 inter_pri_mask;

    rtk_enable_t if_stag_en;
    uint32 if_stag;
    uint32 if_stag_mask;

    rtk_enable_t if_ctag_en;
    uint32 if_ctag;
    uint32 if_ctag_mask;

    rtk_enable_t uni_en;
    uint32 uni;
    uint32 uni_mask;

} diag_classf_ruleCfg_t;

typedef struct diag_classf_usAction_s
{
    rtk_enable_t csact_en;
    rtk_us_csact_t csact;

    rtk_enable_t cact_en;
    uint32 cact;

    rtk_enable_t csvid_en;
    rtk_us_act_t csvid_act;
    uint32 tag_vid;

    rtk_enable_t cspri_en;
    rtk_us_act_t cspri_act;
    uint32 tag_pri;

    rtk_enable_t sid_en;
    rtk_us_id_act_t sid_act;
    uint32 assign_idx;

} diag_classf_usAction_t;

typedef struct diag_classf_dsAction_s
{
    rtk_enable_t csact_en;
	rtk_ds_csact_t csact;

    rtk_enable_t cact_en;
    uint32 cact;

    rtk_enable_t cvid_en;
    rtk_ds_act_t cvid_act;
    uint32 tag_vid;

    rtk_enable_t cpri_en;
    rtk_ds_act_t cpri_act;
    uint32 tag_pri;

    rtk_enable_t cfpri_en;
    rtk_ds_cfpri_act_t cfpri_act;
    uint32 cfpri;

    rtk_enable_t uni_en;
    rtk_ds_uni_act_t uni_act;
    uint32 uni_mask;

} diag_classf_dsAction_t;

/* variable */
static diag_classf_ruleCfg_t diag_classfRule;
static diag_classf_usAction_t diag_classfUsAction;
static diag_classf_dsAction_t diag_classfDsAction;


/* function */
void _diag_classfRuleDiag2Raw(diag_classf_ruleCfg_t *pDiagRule,  apollo_raw_cf_RuleEntry_t *pRawRule)
{
    /* data bits */
    if(DIRECT_UPSTREAM == pDiagRule->direct)
        pRawRule->data_bits.direction = 0;
    else
        pRawRule->data_bits.direction = 1;
    pRawRule->data_bits.etherType = pDiagRule->ethertype;
    pRawRule->data_bits.tos_gemidx = pDiagRule->tos_sid;
    pRawRule->data_bits.tagVid = pDiagRule->tag_vid;
    pRawRule->data_bits.tagPri = pDiagRule->tag_pri;
    pRawRule->data_bits.interPri = pDiagRule->inter_pri;
    pRawRule->data_bits.ifStag = pDiagRule->if_stag;
    pRawRule->data_bits.ifCtag = pDiagRule->if_ctag;
    pRawRule->data_bits.uni = pDiagRule->uni;

    /* mask bits */
    if(pDiagRule->direct_en)
        pRawRule->care_bits.direction = 0x1;
    pRawRule->care_bits.etherType = pDiagRule->ethertype_mask;
    pRawRule->care_bits.tos_gemidx = pDiagRule->tos_sid_mask;
    pRawRule->care_bits.tagVid = pDiagRule->tag_vid_mask;
    pRawRule->care_bits.tagPri = pDiagRule->tag_pri_mask;
    pRawRule->care_bits.interPri = pDiagRule->inter_pri_mask;
    pRawRule->care_bits.ifStag = pDiagRule->if_stag_mask;
    pRawRule->care_bits.ifCtag = pDiagRule->if_ctag_mask;
    pRawRule->care_bits.uni = pDiagRule->uni_mask;
}

void _diag_classfUsActionDiag2Raw(diag_classf_usAction_t *pDiagAction,  apollo_raw_cf_usAct_t *pRawAction)
{
    pRawAction->csact      = pDiagAction->csact;
    pRawAction->cact       = pDiagAction->cact;
    pRawAction->csvid_act  = pDiagAction->csvid_act;
    pRawAction->tag_vid    = pDiagAction->tag_vid;
    pRawAction->cspri_act  = pDiagAction->cspri_act;
    pRawAction->tag_pri    = pDiagAction->tag_pri;
    pRawAction->sid_act    = pDiagAction->sid_act;
    pRawAction->assign_idx = pDiagAction->assign_idx;
}

void _diag_classfDsActionDiag2Raw(diag_classf_dsAction_t *pDiagAction,  apollo_raw_cf_dsAct_t *pRawAction)
{
    pRawAction->csact     = pDiagAction->csact;
    pRawAction->cact      = pDiagAction->cact;
    pRawAction->cvid_act  = pDiagAction->cvid_act;
    pRawAction->tag_vid   = pDiagAction->tag_vid;
    pRawAction->cpri_act  = pDiagAction->cpri_act;
    pRawAction->tag_pri   = pDiagAction->tag_pri;
    pRawAction->cfpri_act = pDiagAction->cfpri_en;
    pRawAction->cfpri     = pDiagAction->cfpri;
    pRawAction->uni_act   = pDiagAction->uni_act;
    pRawAction->uni_mask  = pDiagAction->uni_mask;
}

/*
 * classify show rule
 */
cparser_result_t
cparser_cmd_classify_show_rule(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    if(diag_classfRule.direct_en)
        diag_util_printf("direction: %s\n",diagStr_direction[diag_classfRule.direct]);

    if(diag_classfRule.ethertype_en)
    {
        diag_util_printf("ether type data: 0x%04x\n",diag_classfRule.ethertype);
        diag_util_printf("           mask: 0x%x\n",diag_classfRule.ethertype_mask);
    }

    if(diag_classfRule.tos_sid_en)
    {
        if(0 == diag_classfRule.direct)
        {
            diag_util_printf("tos data: 0x%x\n",diag_classfRule.tos_sid);
            diag_util_printf("    mask: 0x%x\n",diag_classfRule.tos_sid_mask);
        }
        else
        {
            diag_util_printf("sid data: %d\n",diag_classfRule.tos_sid);
            diag_util_printf("    mask: 0x%x\n",diag_classfRule.tos_sid_mask);
        }
    }

    if(diag_classfRule.tag_vid_en)
    {
        diag_util_printf("tag vid data: %d\n", diag_classfRule.tag_vid);
        diag_util_printf("        mask: 0x%x\n", diag_classfRule.tag_vid_mask);
    }

    if(diag_classfRule.tag_pri_en)
    {
        diag_util_printf("tag priority data: %d\n", diag_classfRule.tag_pri);
        diag_util_printf("             mask: 0x%x\n", diag_classfRule.tag_pri_mask);
    }

    if(diag_classfRule.inter_pri_en)
    {
        diag_util_printf("internal priority data: %d\n", diag_classfRule.inter_pri);
        diag_util_printf("                  mask: 0x%x\n", diag_classfRule.inter_pri_mask);
    }

    if(diag_classfRule.if_stag_en)
    {
        diag_util_printf("s-bit data: %d\n", diag_classfRule.if_stag);
        diag_util_printf("      mask: 0x%x\n", diag_classfRule.if_stag_mask);
    }

    if(diag_classfRule.if_ctag_en)
    {
        diag_util_printf("c-bit data: %d\n", diag_classfRule.if_ctag);
        diag_util_printf("      mask: 0x%x\n", diag_classfRule.if_ctag_mask);
    }

    if(diag_classfRule.uni_en)
    {
        diag_util_printf("UNI data: %d\n", diag_classfRule.uni);
        diag_util_printf("    mask: 0x%x\n", diag_classfRule.uni_mask);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_show_rule */

/*
 * classify show us-action
 */
cparser_result_t
cparser_cmd_classify_show_us_action(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    if(diag_classfUsAction.csact_en)
        diag_util_printf("CStag action: %s\n", diagStr_usCStagAction[diag_classfUsAction.csact]);

    if(diag_classfUsAction.cact_en)
        diag_util_printf("Ctag action: %s\n", diagStr_usCtagAction[diag_classfUsAction.cact]);

    if(diag_classfUsAction.csvid_en)
    {
        diag_util_printf("CSVID action: %s\n", diagStr_usSvidAction[diag_classfUsAction.csvid_act]);
        if(US_SACT_ASSIGN == diag_classfUsAction.csvid_act)
            diag_util_printf("VID: %d\n", diag_classfUsAction.tag_vid);
    }
    if(diag_classfUsAction.cspri_en)
    {
        diag_util_printf("CSPRI action: %s\n", diagStr_usSvidAction[diag_classfUsAction.cspri_act]);
        if(0 == diag_classfUsAction.cspri_act)
            diag_util_printf("PRI: %d\n", diag_classfUsAction.tag_pri);
    }

    if(diag_classfUsAction.sid_en)
    {
        diag_util_printf("SID action: %s\n", diagStr_usSidAction[diag_classfUsAction.sid_act]);
        diag_util_printf("Assign ID: %d\n", diag_classfUsAction.assign_idx);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_show_us_action */

/*
 * classify show ds-action
 */
cparser_result_t
cparser_cmd_classify_show_ds_action(
    cparser_context_t *context)
{
    rtk_portmask_t portlist;
    uint8   buf[UTIL_PORT_MASK_BUFFER_LENGTH];

    DIAG_UTIL_PARAM_CHK();

    if(diag_classfDsAction.csact_en)
    {
        diag_util_printf("CStag action: %s\n", diagStr_dsCStagAction[diag_classfDsAction.csact]);
        if((DS_CSACT_VS_TPID == diag_classfDsAction.csact) ||
           (DS_CSACT_8100 == diag_classfDsAction.csact))
        {
            diag_util_printf("Tag VID: %d\n", diag_classfDsAction.tag_vid);
            diag_util_printf("Tag RPI: %d\n", diag_classfDsAction.tag_pri);
        }
    }

    if(diag_classfDsAction.cact_en)
        diag_util_printf("Ctag action: %s\n", diagStr_dsCtagAction[diag_classfDsAction.cact]);

    if(diag_classfDsAction.cvid_en)
    {
        diag_util_printf("CVID action: %s\n", diagStr_dsCvidAction[diag_classfDsAction.cvid_act]);
        if(DS_ACT_ASSIGN == diag_classfDsAction.cvid_act)
            diag_util_printf("CVID: %d\n", diag_classfDsAction.tag_vid);
    }

    if(diag_classfDsAction.cpri_en)
    {
        diag_util_printf("CPRI action: %s\n", diagStr_dsCvidAction[diag_classfDsAction.cpri_act]);
        if(DS_ACT_ASSIGN == diag_classfDsAction.cpri_act)
            diag_util_printf("CPRI: %d\n", diag_classfDsAction.tag_pri);
    }

    if(diag_classfDsAction.cfpri_en)
    {
        diag_util_printf("Classification PRI action: %s\n", diagStr_dsCspriAction[diag_classfDsAction.cfpri_act]);
        if(DS_CFPRI_ASSIGN == diag_classfDsAction.cfpri_act)
            diag_util_printf("CF PRI: %d\n", diag_classfDsAction.cfpri);
    }

    if(diag_classfDsAction.uni_en)
    {
        diag_util_printf("UNI action: %s\n", diagStr_dsUniAction[diag_classfDsAction.uni_act]);
        portlist.bits[0] = diag_classfDsAction.uni_mask;
        diag_util_lPortMask2str(buf,&portlist);
        diag_util_printf("UNI ports: %s\n", buf);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_show_ds_action */

/*
 * classify clear
 */
cparser_result_t
cparser_cmd_classify_clear(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    osal_memset(&diag_classfRule, 0x0, sizeof(diag_classf_ruleCfg_t));
    osal_memset(&diag_classfUsAction, 0x0, sizeof(diag_classf_usAction_t));
    osal_memset(&diag_classfDsAction, 0x0, sizeof(diag_classf_dsAction_t));

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_clear */

/*
 * classify add entry <UINT:entry>
 */
cparser_result_t
cparser_cmd_classify_add_entry_entry(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    apollo_raw_cf_RuleEntry_t entry;
    apollo_raw_cf_dsAct_t ds_act;
    apollo_raw_cf_usAct_t us_act;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*entry_ptr > APOLLO_CF_PATTERN_MAX), CPARSER_ERR_INVALID_PARAMS);

    osal_memset(&entry, 0x0, sizeof(apollo_raw_cf_RuleEntry_t));
    osal_memset(&ds_act, 0x0, sizeof(apollo_raw_cf_dsAct_t));
    osal_memset(&us_act, 0x0, sizeof(apollo_raw_cf_usAct_t));

    _diag_classfRuleDiag2Raw(&diag_classfRule, &entry);

    if(diag_classfRule.direct_en == ENABLED)
    {
    if(DIRECT_UPSTREAM == diag_classfRule.direct)
        _diag_classfUsActionDiag2Raw(&diag_classfUsAction, &us_act);
    else if(DIRECT_DOWNSTREAM == diag_classfRule.direct)
        _diag_classfDsActionDiag2Raw(&diag_classfDsAction, &ds_act);
    }
    else
    {
        diag_util_printf("Error : rule direction not assigned\n");
        return CPARSER_NOT_OK;
    }

    entry.idx  = *entry_ptr;
    entry.valid = 1; /* always set to 1 for tcam valid bit */
    ds_act.idx = *entry_ptr;
    us_act.idx = *entry_ptr;

    DIAG_UTIL_ERR_CHK(apollo_raw_cf_valid_set(entry.idx, DISABLED), ret);

    DIAG_UTIL_ERR_CHK(apollo_raw_cf_rule_set(&entry), ret);
    if(diag_classfRule.direct == DIRECT_DOWNSTREAM)
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_dsAct_set(&ds_act), ret);
    else
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_usAct_set(&us_act), ret);

    DIAG_UTIL_ERR_CHK(apollo_raw_cf_valid_set(entry.idx, ENABLED), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_add_entry_entry */

/*
 * classify del entry <UINT:entry>
 */
cparser_result_t
cparser_cmd_classify_del_entry_entry(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    apollo_raw_cf_RuleEntry_t entry;
    apollo_raw_cf_dsAct_t ds_act;
    apollo_raw_cf_usAct_t us_act;
    int32 idx, start_entry, total_entry;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry, 0x0, sizeof(apollo_raw_cf_RuleEntry_t));
    osal_memset(&ds_act, 0x0, sizeof(apollo_raw_cf_dsAct_t));
    osal_memset(&us_act, 0x0, sizeof(apollo_raw_cf_usAct_t));

    start_entry = *entry_ptr;
    total_entry = 1;

    for(idx=start_entry; idx < (start_entry + total_entry); idx++)
    {
        entry.idx  = idx;
        entry.valid = 1; /* always set to 1 for tcam valid bit */
        ds_act.idx = idx;
        us_act.idx = idx;

        DIAG_UTIL_ERR_CHK(apollo_raw_cf_valid_set(entry.idx, DISABLED), ret);

        DIAG_UTIL_ERR_CHK(apollo_raw_cf_rule_set(&entry), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_dsAct_set(&ds_act), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_usAct_set(&us_act), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_del_entry_entry */

/*
 * classify del entry all
 */
cparser_result_t
cparser_cmd_classify_del_entry_all(
    cparser_context_t *context)
{
    apollo_raw_cf_RuleEntry_t entry;
    apollo_raw_cf_dsAct_t ds_act;
    apollo_raw_cf_usAct_t us_act;
    int32 idx, start_entry, total_entry;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    osal_memset(&entry, 0x0, sizeof(apollo_raw_cf_RuleEntry_t));
    osal_memset(&ds_act, 0x0, sizeof(apollo_raw_cf_dsAct_t));
    osal_memset(&us_act, 0x0, sizeof(apollo_raw_cf_usAct_t));

    start_entry = 0;
    total_entry = APOLLO_CF_PATTERN_MAX;

    for(idx=start_entry; idx < (start_entry + total_entry); idx++)
    {
        entry.idx  = idx;
        entry.valid = 1; /* always set to 1 for tcam valid bit */
        ds_act.idx = idx;
        us_act.idx = idx;

        DIAG_UTIL_ERR_CHK(apollo_raw_cf_valid_set(entry.idx, DISABLED), ret);

        DIAG_UTIL_ERR_CHK(apollo_raw_cf_rule_set(&entry), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_dsAct_set(&ds_act), ret);
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_usAct_set(&us_act), ret);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_del_entry_all */

/*
 * classify get entry <UINT:entry>
 */
cparser_result_t
cparser_cmd_classify_get_entry_entry(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    apollo_raw_cf_RuleEntry_t entry;
    apollo_raw_cf_dsAct_t       ds_act;
    apollo_raw_cf_usAct_t       us_act;
    rtk_enable_t                not_hit;
    rtk_enable_t                valid;
    int32                       ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    entry.idx = *entry_ptr;
    DIAG_UTIL_ERR_CHK(apollo_raw_cf_rule_get(&entry), ret);
    diag_util_printf("Classification Rule entry %d\n\r", entry.idx);
    diag_util_printf("  Direction data: 0x%x, mask 0x%x\n\r", entry.data_bits.direction, entry.care_bits.direction);
    diag_util_printf("  Ethertype data: 0x%x, mask 0x%x\n\r", entry.data_bits.etherType, entry.care_bits.etherType);
    diag_util_printf("  Tos_gemidx data: 0x%x, mask 0x%x\n\r", entry.data_bits.tos_gemidx, entry.care_bits.tos_gemidx);
    diag_util_printf("  TagVid data: 0x%x, mask 0x%x\n\r", entry.data_bits.tagVid, entry.care_bits.tagVid);
    diag_util_printf("  TagPri data: 0x%x, mask 0x%x\n\r", entry.data_bits.tagPri, entry.care_bits.tagPri);
    diag_util_printf("  InterPri data: 0x%x, mask 0x%x\n\r", entry.data_bits.interPri, entry.care_bits.interPri);
    diag_util_printf("  IfStag data: 0x%x, mask 0x%x\n\r", entry.data_bits.ifStag, entry.care_bits.ifStag);
    diag_util_printf("  IfCtag data: 0x%x, mask 0x%x\n\r", entry.data_bits.ifCtag, entry.care_bits.ifCtag);
    diag_util_printf("  UNI data: 0x%x, mask 0x%x\n\r", entry.data_bits.uni, entry.care_bits.uni);

    if(DIRECT_DOWNSTREAM == entry.data_bits.direction)
    {
        ds_act.idx = *entry_ptr;
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_dsAct_get(&ds_act), ret);
        diag_util_printf("Classification downstream action\n\r");
        diag_util_printf("  Stag action: 0x%x\n\r", ds_act.csact);
        diag_util_printf("  Ctag action: 0x%x\n\r", ds_act.cact);
        diag_util_printf("  CVID action: 0x%x\n\r", ds_act.cvid_act);
        diag_util_printf("  Tag VID: 0x%x\n\r", ds_act.tag_vid);
        diag_util_printf("  CPRI action: 0x%x\n\r", ds_act.cpri_act);
        diag_util_printf("  Tag PRI: 0x%x\n\r", ds_act.tag_pri);
        diag_util_printf("  ClassPRI action: 0x%x\n\r", ds_act.cfpri_act);
        diag_util_printf("  Class PRI: 0x%x\n\r", ds_act.cfpri);
        diag_util_printf("  UNI action: 0x%x\n\r", ds_act.uni_act);
        diag_util_printf("  UNI mask: 0x%x\n\r", ds_act.uni_mask);
    }
    else if(DIRECT_UPSTREAM == entry.data_bits.direction)
    {
        us_act.idx = *entry_ptr;
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_usAct_get(&us_act), ret);
        diag_util_printf("Classification upstream action\n\r");
        diag_util_printf("  STPID action: 0x%x\n\r", us_act.csact);
        diag_util_printf("  Ctag action: 0x%x\n\r", us_act.cact);
        diag_util_printf("  SVID action: 0x%x\n\r", us_act.csvid_act);
        diag_util_printf("  Tag VID: 0x%x\n\r", us_act.tag_vid);
        diag_util_printf("  SPRI action: 0x%x\n\r", us_act.cspri_act);
        diag_util_printf("  Tag PRI: 0x%x\n\r", us_act.tag_pri);
        diag_util_printf("  SID action: 0x%x\n\r", us_act.sid_act);
        diag_util_printf("  Assign ID: 0x%x\n\r", us_act.assign_idx);
    }

    DIAG_UTIL_ERR_CHK(apollo_raw_cf_valid_get(*entry_ptr, &valid), ret);
    diag_util_printf("Entry valid: 0x%x\n\r", valid);

    if(DIRECT_DOWNSTREAM == entry.data_bits.direction)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_dsOp_get(*entry_ptr, &not_hit), ret);
        diag_util_printf("Rule operation: 0x%x\n\r", not_hit);
    }
    else if(DIRECT_UPSTREAM == entry.data_bits.direction)
    {
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_usOp_get(*entry_ptr, &not_hit), ret);
        diag_util_printf("Rule operation: 0x%x\n\r", not_hit);
    }

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_get_entry_entry */

/*
 * classify set rule direction ( upstream | downstream )
 */
cparser_result_t
cparser_cmd_classify_set_rule_direction_upstream_downstream(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_classfRule.direct_en = ENABLED;

    if ('u' == TOKEN_CHAR(4,0))
        diag_classfRule.direct = DIRECT_UPSTREAM;
    else
        diag_classfRule.direct = DIRECT_DOWNSTREAM;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_rule_direction_data_upstream_downstream_mask_direct_mask */

/*
 * classify set rule tos-sid data <UINT:tos_sid_data> mask <UINT:tos_sid_mask>
 */
cparser_result_t
cparser_cmd_classify_set_rule_tos_sid_data_tos_sid_data_mask_tos_sid_mask(
    cparser_context_t *context,
    uint32_t  *tos_sid_data_ptr,
    uint32_t  *tos_sid_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    if(DIRECT_DOWNSTREAM == diag_classfRule.direct)
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*tos_sid_data_ptr > APOLLO_SID_MAX), CPARSER_ERR_INVALID_PARAMS);
        DIAG_UTIL_PARAM_RANGE_CHK((*tos_sid_mask_ptr > APOLLO_SID_MAX), CPARSER_ERR_INVALID_PARAMS);
    }
    else
    {
        DIAG_UTIL_PARAM_RANGE_CHK((*tos_sid_data_ptr > 0xFF), CPARSER_ERR_INVALID_PARAMS);
        DIAG_UTIL_PARAM_RANGE_CHK((*tos_sid_mask_ptr > 0xFF), CPARSER_ERR_INVALID_PARAMS);
    }

    diag_classfRule.tos_sid_en = ENABLED;

    diag_classfRule.tos_sid = *tos_sid_data_ptr;
    diag_classfRule.tos_sid_mask = *tos_sid_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_rule_tos_sid_data_tos_sid_data_mask_tos_sid_mask */

/*
 * classify set rule tag-vid data <UINT:vid_data> mask <UINT:vid_mask>
 */
cparser_result_t
cparser_cmd_classify_set_rule_tag_vid_data_vid_data_mask_vid_mask(
    cparser_context_t *context,
    uint32_t  *vid_data_ptr,
    uint32_t  *vid_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_data_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_mask_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfRule.tag_vid_en = ENABLED;

    diag_classfRule.tag_vid = *vid_data_ptr;
    diag_classfRule.tag_vid_mask = *vid_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_rule_tag_vid_data_vid_data_mask_vid_mask */

/*
 * classify set rule tag-pri data <UINT:priority_data> mask <UINT:priority_mask>
 */
cparser_result_t
cparser_cmd_classify_set_rule_tag_priority_data_priority_data_mask_priority_mask(
    cparser_context_t *context,
    uint32_t  *priority_data_ptr,
    uint32_t  *priority_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_data_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_mask_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfRule.tag_pri_en = ENABLED;

    diag_classfRule.tag_pri = *priority_data_ptr;
    diag_classfRule.tag_pri_mask = *priority_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_rule_tag_priority_data_priority_data_mask_priority_mask */

/*
 * classify set rule internal-priority data <UINT:priority_data> mask <UINT:priority_mask>
 */
cparser_result_t
cparser_cmd_classify_set_rule_internal_priority_data_priority_data_mask_priority_mask(
    cparser_context_t *context,
    uint32_t  *priority_data_ptr,
    uint32_t  *priority_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_data_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_mask_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfRule.inter_pri_en = ENABLED;

    diag_classfRule.inter_pri = *priority_data_ptr;
    diag_classfRule.inter_pri_mask = *priority_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_rule_internal_priority_data_priority_data_mask_priority_mask */

/*
 * classify set rule s-bit data <UINT:s_data> mask <UINT:s_mask>
 */
cparser_result_t
cparser_cmd_classify_set_rule_s_bit_data_s_data_mask_s_mask(
    cparser_context_t *context,
    uint32_t  *s_data_ptr,
    uint32_t  *s_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*s_data_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*s_mask_ptr > 1), CPARSER_ERR_INVALID_PARAMS);

    diag_classfRule.if_stag_en = ENABLED;

    diag_classfRule.if_stag = *s_data_ptr;
    diag_classfRule.if_stag_mask = *s_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_rule_s_bit_data_s_data_mask_s_mask */

/*
 * classify set rule c-bit data <UINT:c_data> mask <UINT:c_mask>
 */
cparser_result_t
cparser_cmd_classify_set_rule_c_bit_data_c_data_mask_c_mask(
    cparser_context_t *context,
    uint32_t  *c_data_ptr,
    uint32_t  *c_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*c_data_ptr > 1), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*c_mask_ptr > 1), CPARSER_ERR_INVALID_PARAMS);

    diag_classfRule.if_ctag_en = ENABLED;

    diag_classfRule.if_ctag = *c_data_ptr;
    diag_classfRule.if_ctag_mask = *c_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_rule_c_bit_data_c_data_mask_c_mask */

/*
 * classify set rule uni data <UINT:uni_data> mask <UINT:uni_mask>
 */
cparser_result_t
cparser_cmd_classify_set_rule_uni_data_uni_data_mask_uni_mask(
    cparser_context_t *context,
    uint32_t  *uni_data_ptr,
    uint32_t  *uni_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*uni_data_ptr > APOLLO_PORTMASK), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*uni_mask_ptr > APOLLO_PORTMASK), CPARSER_ERR_INVALID_PARAMS);

    diag_classfRule.uni_en = ENABLED;

    diag_classfRule.uni = *uni_data_ptr;
    diag_classfRule.uni_mask = *uni_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_rule_uni_data_uni_data_mask_uni_mask */

/*
 * classify set rule ether-type data <UINT:type_data> mask <UINT:type_mask>
 */
cparser_result_t
cparser_cmd_classify_set_rule_ether_type_data_type_data_mask_type_mask(
    cparser_context_t *context,
    uint32_t  *type_data_ptr,
    uint32_t  *type_mask_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*type_data_ptr > APOLLO_ETHTYPEMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*type_mask_ptr > APOLLO_ETHTYPEMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfRule.ethertype_en = ENABLED;

    diag_classfRule.ethertype = *type_data_ptr;
    diag_classfRule.ethertype_mask = *type_mask_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_rule_ether_type_data_type_data_mask_type_mask */

/*
 * classify set operation entry <UINT:entry> ( upstream | downstream ) ( hit | not )
 */
cparser_result_t
cparser_cmd_classify_set_operation_entry_entry_upstream_downstream_hit_not(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t enable;
    rtk_direct_t direct;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*entry_ptr > APOLLO_CF_PATTERN_MAX), CPARSER_ERR_INVALID_PARAMS);

    if('u' == TOKEN_CHAR(5,0))
        direct = DIRECT_UPSTREAM;
    else if('d' == TOKEN_CHAR(5,0))
        direct = DIRECT_DOWNSTREAM;

    if ('h' == TOKEN_CHAR(6,0))
        enable = DISABLED;
    else if ('n' == TOKEN_CHAR(6,0))
        enable = ENABLED;

    if(DIRECT_UPSTREAM == direct)
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_usOp_set(*entry_ptr, enable), ret);
    else
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_dsOp_set(*entry_ptr, enable), ret);
    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_operation_entry_entry_upstream_downstream_hit_not */

/*
 * classify get operation entry <UINT:entry> ( upstream | downstream )
 */
cparser_result_t
cparser_cmd_classify_get_operation_entry_entry_upstream_downstream(
    cparser_context_t *context,
    uint32_t  *entry_ptr)
{
    int32 ret = RT_ERR_FAILED;
    rtk_enable_t operation;
    rtk_direct_t direct;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();
    DIAG_UTIL_PARAM_RANGE_CHK((*entry_ptr > APOLLO_CF_PATTERN_MAX), CPARSER_ERR_INVALID_PARAMS);

    if('u' == TOKEN_CHAR(5,0))
        direct = DIRECT_UPSTREAM;
    else if('d' == TOKEN_CHAR(5,0))
        direct = DIRECT_DOWNSTREAM;

    if(DIRECT_UPSTREAM == direct)
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_usOp_get(*entry_ptr, &operation), ret);
    else
        DIAG_UTIL_ERR_CHK(apollo_raw_cf_dsOp_get(*entry_ptr, &operation), ret);

    diag_util_printf("Operation: %s\n\r",diagStr_aclOper[operation]);

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_get_operation_entry_entry_upstream_downstream */

/*
 * classify set us-action cs-act ( nop | vs-tpid | c-tpid )
 */
cparser_result_t
cparser_cmd_classify_set_us_action_cs_act_nop_vs_tpid_c_tpid(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_classfUsAction.csact_en = ENABLED;

    if('n' == TOKEN_CHAR(4,0))
        diag_classfUsAction.csact = US_CSACT_NOP;
    else if('v' == TOKEN_CHAR(4,0))
        diag_classfUsAction.csact = US_CSACT_VS_TPID;
    else if('c' == TOKEN_CHAR(4,0))
        diag_classfUsAction.csact = US_CSACT_8100;
    else
        diag_classfUsAction.csact_en = DISABLED;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_us_action_cs_act_nop_vs_tpid_8100 */

/*
 * classify set us-action c-act ( nop | untag | c2s | transparent )
 */
cparser_result_t
cparser_cmd_classify_set_us_action_c_act_nop_untag_c2s_transparent(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_classfUsAction.cact_en = ENABLED;

    if('n' == TOKEN_CHAR(4,0))
        diag_classfUsAction.cact = US_CACT_NOP;
    else if('u' == TOKEN_CHAR(4,0))
        diag_classfUsAction.cact = US_CACT_UNTAG;
    else if('c' == TOKEN_CHAR(4,0))
        diag_classfUsAction.cact = US_CACT_C2S;
    else if('t' == TOKEN_CHAR(4,0))
        diag_classfUsAction.cact = US_CACT_TRANSPARENT;
    else
        diag_classfUsAction.cact_en = DISABLED;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_us_action_c_act_nop_untag_c2s_transparent */

/*
 * classify set us-action cs-vid-act assign <UINT:vid>
 */
cparser_result_t
cparser_cmd_classify_set_us_action_cs_vid_act_assign_vid(
    cparser_context_t *context,
    uint32_t  *vid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfUsAction.csvid_en = ENABLED;
    diag_classfUsAction.csvid_act = US_SACT_ASSIGN;
    diag_classfUsAction.tag_vid = *vid_ptr;


    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_us_action_cs_vid_act_assign_vid */

/*
 * classify set us-action cs-vid-act copy
 */
cparser_result_t
cparser_cmd_classify_set_us_action_cs_vid_act_copy(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_classfUsAction.csvid_en = ENABLED;
    diag_classfUsAction.csvid_act = US_SACT_COPY_C;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_us_action_cs_vid_act_copy */

/*
 * classify set us-action cs-priority-act assign <UINT:priority>
 */
cparser_result_t
cparser_cmd_classify_set_us_action_cs_priority_act_assign_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfUsAction.cspri_en = ENABLED;
    diag_classfUsAction.cspri_act = US_SACT_ASSIGN;
    diag_classfUsAction.tag_pri = *priority_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_us_action_cs_priority_act_assign_priority */

/*
 * classify set us-action cs-priority-act ( copy | internal_priority )
 */
cparser_result_t
cparser_cmd_classify_set_us_action_cs_priority_act_copy_internal_priority(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_classfUsAction.cspri_en = ENABLED;
    if('c' == TOKEN_CHAR(4,0))
        diag_classfUsAction.cspri_act = US_SACT_COPY_C;
    else if('i' == TOKEN_CHAR(4,0))
        diag_classfUsAction.cspri_act = US_SACT_INTERNAL;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_us_action_cs_priority_act_copy_internal_priority */

/*
 * classify set us-action sid-act ( sid | qid ) <UINT:id>
 */
cparser_result_t
cparser_cmd_classify_set_us_action_sid_act_sid_qid_id(
    cparser_context_t *context,
    uint32_t  *id_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*id_ptr > APOLLO_SID_MAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfUsAction.sid_en = ENABLED;

    if('s' == TOKEN_CHAR(4,0))
        diag_classfUsAction.sid_act = US_ID_SID;
    else if('q' == TOKEN_CHAR(4,0))
        diag_classfUsAction.sid_act = US_ID_QID;

    diag_classfUsAction.assign_idx = *id_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_us_action_sid_act_sid_qid_id */

/*
 * classify set ds-action cs-act ( nop | del )
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_cs_act_nop_del(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_classfDsAction.csact_en = ENABLED;

    if('n' == TOKEN_CHAR(4,0))
        diag_classfDsAction.csact = DS_CSACT_NOP;
    else if('d' == TOKEN_CHAR(4,0))
        diag_classfDsAction.csact = DS_CSACT_DEL;
    else
        diag_classfDsAction.csact_en = DISABLED;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_cs_act_nop_del */

/*
 * classify set ds-action cs-act vs-tpid tag-vid <UINT:vid> tag-priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_cs_act_vs_tpid_tag_vid_vid_tag_priority_priority(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfDsAction.csact_en = ENABLED;
    diag_classfDsAction.csact = DS_CSACT_VS_TPID;
    diag_classfDsAction.tag_vid = *vid_ptr;
    diag_classfDsAction.tag_pri = *priority_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_cs_act_vs_tpid_tag_vid_vid_tag_priority_priority */

/*
 * classify set ds-action cs-act c-tpid tag-vid <UINT:vid> tag-priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_cs_act_c_tpid_tag_vid_vid_tag_priority_priority(
    cparser_context_t *context,
    uint32_t  *vid_ptr,
    uint32_t  *priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*vid_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfDsAction.csact_en = ENABLED;
    diag_classfDsAction.csact = DS_CSACT_8100;
    diag_classfDsAction.tag_vid = *vid_ptr;
    diag_classfDsAction.tag_pri = *priority_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_cs_act_c_tpid_tag_vid_vid_tag_priority_priority */

/*
 * classify set ds-action c-act ( nop | c-tag | sp2c | transparent )
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_c_act_nop_c_tag_sp2c_transparent(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_classfDsAction.cact_en = ENABLED;

    if('n' == TOKEN_CHAR(4,0))
        diag_classfDsAction.cact = DS_CACT_NOP;
    else if('c' == TOKEN_CHAR(4,0))
        diag_classfDsAction.cact = DS_CACT_TAG;
    else if('s' == TOKEN_CHAR(4,0))
        diag_classfDsAction.cact = DS_CACT_SP2C;
    else if('t' == TOKEN_CHAR(4,0))
        diag_classfDsAction.cact = DS_CACT_TRANSPARENT;
    else
        diag_classfDsAction.cact_en = DISABLED;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_c_act_nop_c_tag_sp2c_transparent */

/*
 * classify set ds-action c-vid-act ( swcore | copy | lut )
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_c_vid_act_swcore_copy_lut(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_classfDsAction.cvid_en = ENABLED;

    if('s' == TOKEN_CHAR(4,0))
        diag_classfDsAction.cvid_act = DS_ACT_SWCORE;
    else if('c' == TOKEN_CHAR(4,0))
        diag_classfDsAction.cvid_act = DS_ACT_COPY_S;
    else if('l' == TOKEN_CHAR(4,0))
        diag_classfDsAction.cvid_act = DS_ACT_LUT;
    else
        diag_classfDsAction.cvid_en = DISABLED;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_c_vid_act_swcore_copy_s_lut */

/*
 * classify set ds-action c-vid-act assign <UINT:cvid>
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_c_vid_act_assign_cvid(
    cparser_context_t *context,
    uint32_t  *cvid_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*cvid_ptr > APOLLO_VIDMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfDsAction.cvid_en = ENABLED;
    diag_classfDsAction.cvid_act = DS_ACT_ASSIGN;
    diag_classfDsAction.tag_vid = *cvid_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_c_vid_act_assign_cvid */

/*
 * classify set ds-action c-priority-act ( swcore | copy )
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_c_priority_act_swcore_copy(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_classfDsAction.cpri_en = ENABLED;

    if('s' == TOKEN_CHAR(4,0))
        diag_classfDsAction.cpri_act = DS_ACT_SWCORE;
    else if('c' == TOKEN_CHAR(4,0))
        diag_classfDsAction.cpri_act = DS_ACT_COPY_S;
    else
        diag_classfDsAction.cpri_en = DISABLED;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_c_priority_act_swcore_copy */

/*
 * classify set ds-action c-priority-act assign <UINT:priority>
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_c_priority_act_assign_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfDsAction.cpri_en = ENABLED;
    diag_classfDsAction.cpri_act = DS_ACT_ASSIGN;
    diag_classfDsAction.tag_pri = *priority_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_c_priority_act_assign_priority */

/*
 * classify set ds-action cf-priority-act swcore
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_cf_priority_act_swcore(
    cparser_context_t *context)
{
    DIAG_UTIL_PARAM_CHK();

    diag_classfDsAction.cfpri_en = ENABLED;
    diag_classfDsAction.cfpri_act = DS_CFPRI_SWCORE;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_cf_priority_act_swcore */

/*
 * classify set ds-action cf-priority-act assign <UINT:priority>
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_cf_priority_act_assign_priority(
    cparser_context_t *context,
    uint32_t  *priority_ptr)
{
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((*priority_ptr > APOLLO_PRIMAX), CPARSER_ERR_INVALID_PARAMS);

    diag_classfDsAction.cfpri_en = ENABLED;
    diag_classfDsAction.cfpri_act = DS_CFPRI_ASSIGN;
    diag_classfDsAction.cfpri = *priority_ptr;

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_cf_priority_act_assign_priority */

/*
 * classify set ds-action uni-act ( flood | forced ) port ( <PORT_LIST:ports> | all | none )
 */
cparser_result_t
cparser_cmd_classify_set_ds_action_uni_act_flood_forced_port_ports_all_none(
    cparser_context_t *context,
    char * *ports_ptr)
{
    int32                       ret;
    diag_portlist_t             portlist;

    DIAG_UTIL_PARAM_CHK();

    DIAG_UTIL_ERR_CHK(DIAG_UTIL_EXTRACT_PORTLIST(portlist, 6), ret);

    diag_classfDsAction.uni_en = ENABLED;

    if(0 == osal_strcmp(TOKEN_STR(4),"flood"))
    {
        diag_classfDsAction.uni_act = DS_UNI_FWD;
    }
    else if(0 == osal_strcmp(TOKEN_STR(4),"forced"))
    {
        diag_classfDsAction.uni_act = DS_UNI_FS_FWD;
    }
    else
    {
        DIAG_ERR_PRINT(RT_ERR_INPUT);
        return CPARSER_NOT_OK;
    }

    /* bit 0-2:MAC0-2, bit 3-4:MAC4-5  bit5:MAC6(aka CPU) */
    diag_classfDsAction.uni_mask = ( (portlist.portmask.bits[0] & 0x7) |
                                     ((portlist.portmask.bits[0] & 0x70) >> 1) );

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_ds_action_uni_act_flood_forced_port_ports_all_none */

/*
 * classify set us-unmatch-act ( drop | permit | permit-without-pon )
 */
cparser_result_t
cparser_cmd_classify_set_us_unmatch_act_drop_permit_permit_without_pon(
    cparser_context_t *context)
{
    apollo_raw_cf_usPermit_t action;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();

    if(0 == osal_strcmp(TOKEN_STR(3),"drop"))
        action = APOLLO_RAW_CF_US_PERMIT_DROP;
    else if(0 == osal_strcmp(TOKEN_STR(3),"permit"))
        action = APOLLO_RAW_CF_US_PERMIT_NORMAL;
    else if(0 == osal_strcmp(TOKEN_STR(3),"permit-without-pon"))
        action = APOLLO_RAW_CF_US_PERMIT_NOPON;

    DIAG_UTIL_ERR_CHK(apollo_raw_cf_usPermit_set(action), ret);

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_set_us_unmatch_act_drop_permit_permit_without_pon */

/*
 * classify get us-unmatch-act
 */
cparser_result_t
cparser_cmd_classify_get_us_unmatch_act(
    cparser_context_t *context)
{
    apollo_raw_cf_usPermit_t action;
    int32 ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(apollo_raw_cf_usPermit_get(&action), ret);

    diag_util_printf("Upstream un-match action: %s\n\r",diagStr_cfUnmatchAct[action]);

    return CPARSER_OK;
}    /* end of cparser_cmd_classify_get_us_unmatch_act */


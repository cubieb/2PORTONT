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
 * $Revision: 50477 $
 * $Date: 2014-08-21 11:23:27 +0800 (Thu, 21 Aug 2014) $
 *
 * Purpose : Definition of Classifyication API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) classfication rule add/delete/get
 */



/*
 * Include Files
 */
#include <dal/apollo/dal_apollo.h>
#include <dal/apollo/raw/apollo_raw_cf.h>
#include <dal/apollo/dal_apollo_classify.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
static uint32    classify_init = INIT_NOT_COMPLETED;

/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */
/* Function Name:
 *      dal_apollo_classify_init
 * Description:
 *      Initialize classification module.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize classification module before calling any classification APIs.
 */
int32
dal_apollo_classify_init(void)
{
    apollo_raw_cf_RuleEntry_t entry;
    apollo_raw_cf_dsAct_t ds_act;
    apollo_raw_cf_usAct_t us_act;
    uint32  idx;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* clear table */
    osal_memset(&entry, 0x0, sizeof(apollo_raw_cf_RuleEntry_t));
    osal_memset(&ds_act, 0x0, sizeof(apollo_raw_cf_dsAct_t));
    osal_memset(&us_act, 0x0, sizeof(apollo_raw_cf_usAct_t));

    for(idx = 0; idx <= APOLLO_CF_PATTERN_MAX; idx++)
    {
        entry.idx  = idx;
        entry.valid = 1; /* always set to 1 for Tcam valid bit */
        ds_act.idx = idx;
        us_act.idx = idx;

        RT_ERR_CHK(apollo_raw_cf_valid_set(entry.idx, DISABLED), ret);
        RT_ERR_CHK(apollo_raw_cf_rule_set(&entry), ret);
        RT_ERR_CHK(apollo_raw_cf_dsAct_set(&ds_act), ret);
        RT_ERR_CHK(apollo_raw_cf_usAct_set(&us_act), ret);
        RT_ERR_CHK(apollo_raw_cf_dsOp_set(entry.idx, CLASSIFY_INVERT_DISABLE), ret);
        RT_ERR_CHK(apollo_raw_cf_usOp_set(entry.idx, CLASSIFY_INVERT_DISABLE), ret);
    }

    classify_init = INIT_COMPLETED;
    return RT_ERR_OK;
} /* end of dal_apollo_classify_init */

static int
dal_apollo_classify_cfg2raw_us_csAct(rtk_classify_us_csact_t cfgAct, uint32 *pRawAct)
{
    int32 ret=RT_ERR_OK;

    switch(cfgAct)
    {
        case CLASSIFY_US_CSACT_NOP:
            *pRawAct = 0;
            break;
        case CLASSIFY_US_CSACT_ADD_TAG_VS_TPID:
            *pRawAct = 1;
            break;
        case CLASSIFY_US_CSACT_ADD_TAG_8100:
            *pRawAct = 2;
            break;
        case CLASSIFY_US_CSACT_DEL_STAG:
        case CLASSIFY_US_CSACT_TRANSPARENT:
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_cfg2raw_us_cAct(rtk_classify_us_cact_t cfgAct, uint32 *pRawAct)
{
    int32 ret=RT_ERR_OK;

    switch(cfgAct)
    {
        case CLASSIFY_US_CACT_NOP:
            *pRawAct = 0;
            break;
        case CLASSIFY_US_CACT_TRANSLATION_C2S:
            *pRawAct = 2;
            break;
        case CLASSIFY_US_CACT_DEL_CTAG:
            *pRawAct = 1;
            break;
        case CLASSIFY_US_CACT_TRANSPARENT:
            *pRawAct = 3;
            break;
        case CLASSIFY_US_CACT_ADD_CTAG_8100:
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_cfg2raw_us_csVidAct(rtk_classify_us_vid_act_t cfgAct, uint32 *pRawAct)
{
    int32 ret=RT_ERR_OK;

    switch(cfgAct)
    {
        case CLASSIFY_US_VID_ACT_ASSIGN:
            *pRawAct = 0;
            break;
        case CLASSIFY_US_VID_ACT_FROM_1ST_TAG:
            *pRawAct = 1;
            break;
        case CLASSIFY_US_VID_ACT_FROM_2ND_TAG:
        case CLASSIFY_US_VID_ACT_FROM_INTERNAL:
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_cfg2raw_us_csPriAct(rtk_classify_us_pri_act_t cfgAct, uint32 *pRawAct)
{
    int32 ret=RT_ERR_OK;

    switch(cfgAct)
    {
        case CLASSIFY_US_PRI_ACT_ASSIGN:
            *pRawAct = 0;
            break;
        case CLASSIFY_US_PRI_ACT_FROM_1ST_TAG:
            *pRawAct = 1;
            break;
        case CLASSIFY_US_PRI_ACT_FROM_INTERNAL:
            *pRawAct = 2;
            break;
        case CLASSIFY_US_PRI_ACT_FROM_2ND_TAG:
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_cfg2raw_sidAct(rtk_classify_us_sqid_act_t sqidAct, uint32 *pRawAct)
{
       int32 ret=RT_ERR_OK;

	 /*assign sid act, since new register designed, 2012.05.28*/
	 switch(sqidAct){
	 case CLASSIFY_US_SQID_ACT_ASSIGN_SID:
	 	*pRawAct  =0;
	 break;
	 case CLASSIFY_US_SQID_ACT_ASSIGN_QID:
	 	*pRawAct = 1;
	 default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
	 break;
	 }
	 
	 return ret;
}


static int
dal_apollo_classify_cfg2raw_uniAct(rtk_classify_ds_uni_act_t cfgAct, uint32 *pRawAct)
{
    int32 ret=RT_ERR_OK;

    /*assign uni act, since new register designed, 2012.05.28*/
	 switch(cfgAct){ 
	 case CLASSIFY_DS_UNI_ACT_MASK_BY_UNIMASK:
		 *pRawAct   = 0;
 	 break;
	 case CLASSIFY_DS_UNI_ACT_FORCE_FORWARD:
	 	*pRawAct   = 1 ;
	 break;
	 default:
	 	ret = RT_ERR_CHIP_NOT_SUPPORTED;
	 break;
	 }
	 return ret;
}


static int
dal_apollo_classify_cfg2raw_ds_csAct(rtk_classify_ds_csact_t cfgAct, uint32 *pRawAct)
{
    int32 ret=RT_ERR_OK;

    switch(cfgAct)
    {
        case CLASSIFY_DS_CSACT_NOP:
            *pRawAct = 0;
            break;
        case CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID:
            *pRawAct = 1;
            break;
        case CLASSIFY_DS_CSACT_ADD_TAG_8100:
            *pRawAct = 2;
            break;
        case CLASSIFY_DS_CSACT_DEL_STAG:
            *pRawAct = 3;
            break;
        case CLASSIFY_DS_CSACT_TRANSPARENT:
        case CLASSIFY_DS_CSACT_SP2C:
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_cfg2raw_ds_cAct(rtk_classify_ds_cact_t cfgAct, uint32 *pRawAct)
{
    int32 ret=RT_ERR_OK;

    switch(cfgAct)
    {
        case CLASSIFY_DS_CACT_NOP:
            *pRawAct = 0;
            break;
        case CLASSIFY_DS_CACT_ADD_CTAG_8100:
            *pRawAct = 1;
            break;
        case CLASSIFY_DS_CACT_TRANSLATION_SP2C:
            *pRawAct = 2;
            break;
        case CLASSIFY_DS_CACT_TRANSPARENT:
            *pRawAct = 3;
            break;
        case CLASSIFY_DS_CACT_DEL_CTAG:
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_cfg2raw_ds_cVidAct(rtk_classify_ds_vid_act_t cfgAct, uint32 *pRawAct)
{
    int32 ret=RT_ERR_OK;

    switch(cfgAct)
    {
        case CLASSIFY_DS_VID_ACT_NOP:
            *pRawAct = 0;
            break;
        case CLASSIFY_DS_VID_ACT_ASSIGN:
            *pRawAct = 2;
            break;
        case CLASSIFY_DS_VID_ACT_FROM_1ST_TAG:
            *pRawAct = 1;
            break;
        case CLASSIFY_DS_VID_ACT_FROM_LUT:
            *pRawAct = 3;
            break;
        case CLASSIFY_DS_VID_ACT_FROM_2ND_TAG:
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_cfg2raw_ds_cPriAct(rtk_classify_ds_pri_act_t cfgAct, uint32 *pRawAct)
{
    int32 ret=RT_ERR_OK;

    switch(cfgAct)
    {
        case CLASSIFY_DS_PRI_ACT_NOP:
            *pRawAct = 0;
            break;
        case CLASSIFY_DS_PRI_ACT_ASSIGN:
            *pRawAct = 2;
            break;
        case CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG:
            *pRawAct = 1;
            break;
        case CLASSIFY_DS_PRI_ACT_FROM_INTERNAL:
            *pRawAct = 3;
            break;
        case CLASSIFY_DS_PRI_ACT_FROM_2ND_TAG:
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_raw2cfg_us_csAct(uint32 rawAct, rtk_classify_us_csact_t *pCcfgAct)
{
    int32 ret=RT_ERR_OK;

    switch(rawAct)
    {
        case 0:
            *pCcfgAct = CLASSIFY_US_CSACT_NOP;
            break;
        case 1:
            *pCcfgAct = CLASSIFY_US_CSACT_ADD_TAG_VS_TPID;
            break;
        case 2:
            *pCcfgAct = CLASSIFY_US_CSACT_ADD_TAG_8100;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_raw2cfg_us_cAct(uint32 rawAct, rtk_classify_us_cact_t *pCfgAct)
{
    int32 ret=RT_ERR_OK;

    switch(rawAct)
    {
        case 0:
            *pCfgAct = CLASSIFY_US_CACT_NOP;
            break;
        case 1:
            *pCfgAct = CLASSIFY_US_CACT_DEL_CTAG;
            break;
        case 2:
            *pCfgAct = CLASSIFY_US_CACT_TRANSLATION_C2S;
            break;
        case 3:
            *pCfgAct = CLASSIFY_US_CACT_TRANSPARENT;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_raw2cfg_us_csVidAct(uint32 rawAct, rtk_classify_us_vid_act_t *pCfgAct)
{
    int32 ret=RT_ERR_OK;

    switch(rawAct)
    {
        case 0:
            *pCfgAct = CLASSIFY_US_VID_ACT_ASSIGN;
            break;
        case 1:
            *pCfgAct = CLASSIFY_US_VID_ACT_FROM_1ST_TAG;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_raw2cfg_us_csPriAct(uint32 rawAct, rtk_classify_us_pri_act_t *pCfgAct)
{
    int32 ret=RT_ERR_OK;

    switch(rawAct)
    {
        case 0:
            *pCfgAct = CLASSIFY_US_PRI_ACT_ASSIGN;
            break;
        case 1:
            *pCfgAct = CLASSIFY_US_PRI_ACT_FROM_1ST_TAG;
            break;
        case 2:
            *pCfgAct = CLASSIFY_US_PRI_ACT_FROM_INTERNAL;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}


static int
dal_apollo_classify_raw2cfg_sidAct(uint32 rawAct, rtk_classify_us_sqid_act_t *psidAct)
{
	int32 ret=RT_ERR_OK;

	/*assign sid act, since new register designed, 2012.05.28*/
	switch(rawAct){
	case 0:
		*psidAct = CLASSIFY_US_SQID_ACT_ASSIGN_SID;
	      break;
	case 1:
		*psidAct = CLASSIFY_US_SQID_ACT_ASSIGN_QID;
	      break;
	default:
		ret = RT_ERR_CHIP_NOT_SUPPORTED;
	     break;
	}
	return ret;
}

static int
dal_apollo_classify_raw2cfg_uniAct(uint32 rawAct, rtk_classify_ds_uni_act_t *puniAct)
{
	int32 ret=RT_ERR_OK;

	/*assign uni act, since new register designed, 2012.05.28*/
	switch(rawAct) { 
	case 0:
		*puniAct  = CLASSIFY_DS_UNI_ACT_MASK_BY_UNIMASK;
 	break;
	case 1:
		*puniAct   = CLASSIFY_DS_UNI_ACT_FORCE_FORWARD ;
	break;
	default:
		ret = RT_ERR_CHIP_NOT_SUPPORTED;
	break;
	}
return ret;
}



static int
dal_apollo_classify_raw2cfg_ds_csAct(uint32 rawAct, rtk_classify_ds_csact_t *pCfgAct)
{
    int32 ret=RT_ERR_OK;

    switch(rawAct)
    {
        case 0:
            *pCfgAct = CLASSIFY_DS_CSACT_NOP;
            break;
        case 1:
            *pCfgAct = CLASSIFY_DS_CSACT_ADD_TAG_VS_TPID;
            break;
        case 2:
            *pCfgAct = CLASSIFY_DS_CSACT_ADD_TAG_8100;
            break;
        case 3:
            *pCfgAct = CLASSIFY_DS_CSACT_DEL_STAG;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_raw2cfg_ds_cAct(uint32 rawAct, rtk_classify_ds_cact_t *pCfgAct)
{
    int32 ret=RT_ERR_OK;

    switch(rawAct)
    {
        case 0:
            *pCfgAct = CLASSIFY_DS_CACT_NOP;
            break;
        case 1:
            *pCfgAct = CLASSIFY_DS_CACT_ADD_CTAG_8100;
            break;
        case 2:
            *pCfgAct = CLASSIFY_DS_CACT_TRANSLATION_SP2C;
            break;
        case 3:
            *pCfgAct = CLASSIFY_DS_CACT_TRANSPARENT;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_raw2cfg_ds_cVidAct(uint32 rawAct, rtk_classify_ds_vid_act_t *pCfgAct)
{
    int32 ret=RT_ERR_OK;

    switch(rawAct)
    {
        case 0:
            *pCfgAct = CLASSIFY_DS_VID_ACT_NOP;
            break;
        case 2:
            *pCfgAct = CLASSIFY_DS_VID_ACT_ASSIGN;
            break;
        case 1:
            *pCfgAct = CLASSIFY_DS_VID_ACT_FROM_1ST_TAG;
            break;
        case 3:
            *pCfgAct = CLASSIFY_DS_VID_ACT_FROM_LUT;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_raw2cfg_ds_cPriAct(uint32 rawAct, rtk_classify_ds_pri_act_t *pCfgAct)
{
    int32 ret=RT_ERR_OK;

    switch(rawAct)
    {
        case 0:
            *pCfgAct = CLASSIFY_DS_PRI_ACT_NOP;
            break;
        case 2:
            *pCfgAct = CLASSIFY_DS_PRI_ACT_ASSIGN;
            break;
        case 1:
            *pCfgAct = CLASSIFY_DS_PRI_ACT_FROM_1ST_TAG;
            break;
        case 3:
            *pCfgAct = CLASSIFY_DS_PRI_ACT_FROM_INTERNAL;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }
    return ret;
}

static int
dal_apollo_classify_cfg2entry(rtk_classify_cfg_t        *pClassifyCfg,
                              apollo_raw_cf_RuleEntry_t *pRule,
                              apollo_raw_cf_usAct_t     *pUsAction,
                              apollo_raw_cf_dsAct_t     *pDsAction)
{
    rtk_classify_field_t *tmp_field;
    int32 ret=RT_ERR_OK;

    osal_memset(pRule, 0, sizeof(apollo_raw_cf_RuleEntry_t));
    osal_memset(pUsAction, 0, sizeof(apollo_raw_cf_usAct_t));
    osal_memset(pDsAction, 0, sizeof(apollo_raw_cf_dsAct_t));

    /* rule */
    pRule->idx   = pClassifyCfg->index;
    pRule->valid = 1;
    pRule->data_bits.direction  = pClassifyCfg->direction;
    pRule->care_bits.direction  = 0x1;
    tmp_field = pClassifyCfg->field.pFieldHead;
    while(tmp_field != NULL)
    {
        switch(tmp_field->fieldType)
        {
            case CLASSIFY_FIELD_ETHERTYPE:
                pRule->data_bits.etherType = tmp_field->classify_pattern.etherType.value;
                pRule->care_bits.etherType = tmp_field->classify_pattern.etherType.mask;
                break;

            case CLASSIFY_FIELD_TOS_DSIDX:
                pRule->data_bits.tos_gemidx = tmp_field->classify_pattern.tosDsidx.value;
                pRule->care_bits.tos_gemidx = tmp_field->classify_pattern.tosDsidx.mask;
                break;

            case CLASSIFY_FIELD_TAG_VID:
                pRule->data_bits.tagVid = tmp_field->classify_pattern.tagVid.value;
                pRule->care_bits.tagVid = tmp_field->classify_pattern.tagVid.mask;
                break;

            case CLASSIFY_FIELD_TAG_PRI:
                pRule->data_bits.tagPri = tmp_field->classify_pattern.tagPri.value;
                pRule->care_bits.tagPri = tmp_field->classify_pattern.tagPri.mask;
                break;

            case CLASSIFY_FIELD_INTER_PRI:
                pRule->data_bits.interPri = tmp_field->classify_pattern.interPri.value;
                pRule->care_bits.interPri = tmp_field->classify_pattern.interPri.mask;
                break;

            case CLASSIFY_FIELD_IS_CTAG:
                pRule->data_bits.ifCtag = tmp_field->classify_pattern.isCtag.value;
                pRule->care_bits.ifCtag = tmp_field->classify_pattern.isCtag.mask;
                break;

            case CLASSIFY_FIELD_IS_STAG:
                pRule->data_bits.ifStag = tmp_field->classify_pattern.isStag.value;
                pRule->care_bits.ifStag = tmp_field->classify_pattern.isStag.mask;
                break;

            case CLASSIFY_FIELD_UNI:
                pRule->data_bits.uni = tmp_field->classify_pattern.uni.value;
                pRule->care_bits.uni = tmp_field->classify_pattern.uni.mask;
                break;

            default:
                return RT_ERR_CHIP_NOT_SUPPORTED;
        }

        tmp_field = tmp_field->next;
    }

    if(CLASSIFY_DIRECTION_US == pClassifyCfg->direction)
    {   /* us action */
        pUsAction->idx = pClassifyCfg->index;
        RT_ERR_CHK(dal_apollo_classify_cfg2raw_us_csAct(pClassifyCfg->act.usAct.csAct, &pUsAction->csact), ret);
        RT_ERR_CHK(dal_apollo_classify_cfg2raw_us_cAct(pClassifyCfg->act.usAct.cAct, &pUsAction->cact), ret);
        RT_ERR_CHK(dal_apollo_classify_cfg2raw_us_csVidAct(pClassifyCfg->act.usAct.csVidAct, &pUsAction->csvid_act), ret);
        RT_ERR_CHK(dal_apollo_classify_cfg2raw_us_csPriAct(pClassifyCfg->act.usAct.csPriAct, &pUsAction->cspri_act), ret);
	  RT_ERR_CHK(dal_apollo_classify_cfg2raw_sidAct(pClassifyCfg->act.usAct.sidQidAct, &pUsAction->sid_act), ret);
        pUsAction->tag_vid    = pClassifyCfg->act.usAct.sTagVid;
        pUsAction->tag_pri    = pClassifyCfg->act.usAct.sTagPri;
        pUsAction->assign_idx = pClassifyCfg->act.usAct.sidQid;
    }
    else /* ds action */
    {
        pDsAction->idx = pClassifyCfg->index;
        RT_ERR_CHK(dal_apollo_classify_cfg2raw_ds_csAct(pClassifyCfg->act.dsAct.csAct, &pDsAction->csact), ret);
        RT_ERR_CHK(dal_apollo_classify_cfg2raw_ds_cAct(pClassifyCfg->act.dsAct.cAct, &pDsAction->cact), ret);
        RT_ERR_CHK(dal_apollo_classify_cfg2raw_ds_cVidAct(pClassifyCfg->act.dsAct.cVidAct, &pDsAction->cvid_act), ret);
        RT_ERR_CHK(dal_apollo_classify_cfg2raw_ds_cPriAct(pClassifyCfg->act.dsAct.cPriAct, &pDsAction->cpri_act), ret);
	  RT_ERR_CHK(dal_apollo_classify_cfg2raw_uniAct(pClassifyCfg->act.dsAct.uniAct, &pDsAction->uni_act), ret);
        pDsAction->tag_vid   = pClassifyCfg->act.dsAct.cTagVid;
        pDsAction->tag_pri   = pClassifyCfg->act.dsAct.cTagPri;
        pDsAction->cfpri_act = pClassifyCfg->act.dsAct.interPriAct;
        pDsAction->cfpri     = pClassifyCfg->act.dsAct.cfPri;
        pDsAction->uni_mask  = ((pClassifyCfg->act.dsAct.uniMask.bits[0] & 0x7) |
                               ((pClassifyCfg->act.dsAct.uniMask.bits[0] & 0x70) >> 1) );
    }
    return ret;
}

static int
dal_apollo_classify_entry2cfg(apollo_raw_cf_usAct_t     *pUsAction,
                              apollo_raw_cf_dsAct_t     *pDsAction,
                              rtk_classify_cfg_t        *pClassifyCfg)
{
    int32 ret=RT_ERR_OK;

    if(CLASSIFY_DIRECTION_US == pClassifyCfg->direction)
    {   /* us action */
        RT_ERR_CHK(dal_apollo_classify_raw2cfg_us_csAct(pUsAction->csact, &pClassifyCfg->act.usAct.csAct), ret);
        RT_ERR_CHK(dal_apollo_classify_raw2cfg_us_cAct(pUsAction->cact, &pClassifyCfg->act.usAct.cAct), ret);
        RT_ERR_CHK(dal_apollo_classify_raw2cfg_us_csVidAct(pUsAction->csvid_act, &pClassifyCfg->act.usAct.csVidAct), ret);
        RT_ERR_CHK(dal_apollo_classify_raw2cfg_us_csPriAct(pUsAction->cspri_act, &pClassifyCfg->act.usAct.csPriAct), ret);
	  RT_ERR_CHK(dal_apollo_classify_raw2cfg_sidAct( pUsAction->sid_act, &pClassifyCfg->act.usAct.sidQidAct), ret);
        pClassifyCfg->act.usAct.sTagVid     = pUsAction->tag_vid;
        pClassifyCfg->act.usAct.sTagPri     = pUsAction->tag_pri;
        pClassifyCfg->act.usAct.sidQid      = pUsAction->assign_idx;
    }
    else /* ds action */
    {
        RT_ERR_CHK(dal_apollo_classify_raw2cfg_ds_csAct(pDsAction->csact, &pClassifyCfg->act.dsAct.csAct), ret);
        RT_ERR_CHK(dal_apollo_classify_raw2cfg_ds_cAct(pDsAction->cact, &pClassifyCfg->act.dsAct.cAct), ret);
        RT_ERR_CHK(dal_apollo_classify_raw2cfg_ds_cVidAct(pDsAction->cvid_act, &pClassifyCfg->act.dsAct.cVidAct), ret);
        RT_ERR_CHK(dal_apollo_classify_raw2cfg_ds_cPriAct(pDsAction->cpri_act, &pClassifyCfg->act.dsAct.cPriAct), ret);
	 RT_ERR_CHK(dal_apollo_classify_raw2cfg_uniAct(pDsAction->uni_act, &pClassifyCfg->act.dsAct.uniAct), ret);
        pClassifyCfg->act.dsAct.cTagVid     = pDsAction->tag_vid;
        pClassifyCfg->act.dsAct.cTagPri     = pDsAction->tag_pri;
        pClassifyCfg->act.dsAct.interPriAct = pDsAction->cfpri_act;
        pClassifyCfg->act.dsAct.cfPri       = pDsAction->cfpri;
        pClassifyCfg->act.dsAct.uniMask.bits[0] = ((pDsAction->uni_mask & 0x7) |
                                                  ((pDsAction->uni_mask & 0x38) << 1)) & 0x77;
    }
    return ret;
}

/* Function Name:
 *      dal_apollo_classify_cfgEntry_add
 * Description:
 *      Add an classification entry to ASIC
 * Input:
 *      entryIdx       - index of classification entry.
 *      pClassifyCfg     - The classification configuration that this function will add comparison rule
 *      pClassifyAct     - Action(s) of classification configuration.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pClassifyCfg point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None.
 */
int32
dal_apollo_classify_cfgEntry_add(rtk_classify_cfg_t *pClassifyCfg)
{
    apollo_raw_cf_RuleEntry_t entry;
    apollo_raw_cf_dsAct_t ds_act;
    apollo_raw_cf_usAct_t us_act;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pClassifyCfg), RT_ERR_NULL_POINTER);

    /* translate data structure */
    RT_ERR_CHK(dal_apollo_classify_cfg2entry(pClassifyCfg, &entry, &us_act, &ds_act), ret);

    /* set entry invalid */
    RT_ERR_CHK(apollo_raw_cf_valid_set(entry.idx, DISABLED), ret);

    /* set entry rule */
    RT_ERR_CHK(apollo_raw_cf_rule_set(&entry), ret);

    /* set entry action and set not operation */
    if(CLASSIFY_DIRECTION_US == pClassifyCfg->direction)
    {
        RT_ERR_CHK(apollo_raw_cf_usAct_set(&us_act), ret);
        RT_ERR_CHK(apollo_raw_cf_usOp_set(pClassifyCfg->index, pClassifyCfg->invert), ret);
    }
    else
    {
        RT_ERR_CHK(apollo_raw_cf_dsAct_set(&ds_act), ret);
        RT_ERR_CHK(apollo_raw_cf_dsOp_set(pClassifyCfg->index, pClassifyCfg->invert), ret);
    }

    /* set entry valid */
    RT_ERR_CHK(apollo_raw_cf_valid_set(entry.idx, pClassifyCfg->valid), ret);

    return RT_ERR_OK;
} /* end of dal_apollo_classify_cfgEntry_add */


/* Function Name:
 *      dal_apollo_classify_cfgEntry_get
 * Description:
 *      Gdd an classification entry from ASIC
 * Input:
 *      None.
 * Output:
 *      pClassifyCfg     - The classification configuration that this function will add comparison rule
 * Return:
 *      RT_ERR_OK              					- OK
 *      RT_ERR_FAILED          					- Failed
 *      RT_ERR_NULL_POINTER    					- Pointer pClassifyCfg point to NULL.
 *      RT_ERR_INPUT 							- Invalid input parameters.
 * Note:
 *      None.
 */
int32
dal_apollo_classify_cfgEntry_get(rtk_classify_cfg_t *pClassifyCfg)
{
    uint32 entryIdx;
    uint32 patternIdx;
    uint32 fieldData[APOLLO_RAW_CF_ENTRYLEN];
    uint32 fieldMask[APOLLO_RAW_CF_ENTRYLEN];
    apollo_raw_cf_dsAct_t ds_act;
    apollo_raw_cf_usAct_t us_act;
    rtk_enable_t not_op;
    int32 ret;
    uint32 idx;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pClassifyCfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((HAL_CLASSIFY_ENTRY_MAX() <= pClassifyCfg->index), RT_ERR_NULL_POINTER);

    /* get entry valid */
    RT_ERR_CHK(apollo_raw_cf_valid_get(pClassifyCfg->index, &pClassifyCfg->valid), ret);

    /* get entry rule raw data */
    entryIdx = pClassifyCfg->index&0x7f;
    patternIdx = pClassifyCfg->index>>7;
    RT_ERR_CHK(table_read(CF_RULEt, entryIdx, fieldData), ret);
    RT_ERR_CHK(table_read(CF_MASKt, entryIdx, fieldMask), ret);
    for(idx=0; idx<APOLLO_RAW_CF_ENTRYLEN; idx++)
        fieldMask[idx] = fieldMask[idx]^fieldData[idx];

    osal_memset(&pClassifyCfg->field.readField, 0, sizeof(rtk_classify_raw_field_t));
    pClassifyCfg->field.readField.dataFieldRaw[0] = (fieldData[patternIdx+1]&0xFFFF);
    pClassifyCfg->field.readField.dataFieldRaw[1] = ((fieldData[patternIdx+1]>>16)&0xFFFF);
    pClassifyCfg->field.readField.careFieldRaw[0] = (fieldMask[patternIdx+1]&0xFFFF);
    pClassifyCfg->field.readField.careFieldRaw[1] = ((fieldMask[patternIdx+1]>>16)&0xFFFF);
    if(0 == patternIdx)
    {
        pClassifyCfg->field.readField.dataFieldRaw[2] = (fieldData[patternIdx]&0xFFFF);
        pClassifyCfg->field.readField.careFieldRaw[2] = (fieldMask[patternIdx]&0xFFFF);
    }

    /* direction: bit 31 */
    if(((pClassifyCfg->field.readField.dataFieldRaw[1] & 0x8000) &
         (pClassifyCfg->field.readField.careFieldRaw[1] & 0x8000)) == 0)
         pClassifyCfg->direction = CLASSIFY_DIRECTION_US;
     else
         pClassifyCfg->direction = CLASSIFY_DIRECTION_DS;

    osal_memset(&ds_act, 0, sizeof(apollo_raw_cf_dsAct_t));
    osal_memset(&us_act, 0, sizeof(apollo_raw_cf_usAct_t));
    ds_act.idx = pClassifyCfg->index;
    us_act.idx = pClassifyCfg->index;
    /* get entry action and get not operation */
    if(CLASSIFY_DIRECTION_US == pClassifyCfg->direction)
    {
        RT_ERR_CHK(apollo_raw_cf_usAct_get(&us_act), ret);
        RT_ERR_CHK(apollo_raw_cf_usOp_get(pClassifyCfg->index, &not_op), ret);
    }
    else
    {
        RT_ERR_CHK(apollo_raw_cf_dsAct_get(&ds_act), ret);
        RT_ERR_CHK(apollo_raw_cf_dsOp_get(pClassifyCfg->index, &not_op), ret);
    }

    /* translate data structure */
    RT_ERR_CHK(dal_apollo_classify_entry2cfg(&us_act, &ds_act, pClassifyCfg), ret);
    if(DISABLED == not_op)
        pClassifyCfg->invert = CLASSIFY_INVERT_DISABLE;
    else
        pClassifyCfg->invert = CLASSIFY_INVERT_ENABLE;

    return RT_ERR_OK;
} /* end of dal_apollo_classify_cfgEntry_get */


/* Function Name:
 *      dal_apollo_classify_cfgEntry_del
 * Description:
 *      Delete an classification configuration from ASIC
 * Input:
 *      entryIdx    - index of classification entry.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_ENTRY_INDEX 		- Invalid classification index .
 * Note:
 *      None.
 */
int32
dal_apollo_classify_cfgEntry_del(uint32 entryIdx)
{
    apollo_raw_cf_RuleEntry_t entry;
    apollo_raw_cf_dsAct_t ds_act;
    apollo_raw_cf_usAct_t us_act;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((HAL_CLASSIFY_ENTRY_MAX() <= entryIdx), RT_ERR_INPUT);

    osal_memset(&entry, 0x0, sizeof(apollo_raw_cf_RuleEntry_t));
    osal_memset(&ds_act, 0x0, sizeof(apollo_raw_cf_dsAct_t));
    osal_memset(&us_act, 0x0, sizeof(apollo_raw_cf_usAct_t));

    entry.idx  = entryIdx;
    entry.valid = 1; /* always set to 1 for tcam valid bit */
    ds_act.idx = entryIdx;
    us_act.idx = entryIdx;

    RT_ERR_CHK(apollo_raw_cf_valid_set(entryIdx, DISABLED), ret);

    RT_ERR_CHK(apollo_raw_cf_rule_set(&entry), ret);
    RT_ERR_CHK(apollo_raw_cf_dsAct_set(&ds_act), ret);
    RT_ERR_CHK(apollo_raw_cf_usAct_set(&us_act), ret);

    RT_ERR_CHK(apollo_raw_cf_dsOp_set(entryIdx, CLASSIFY_INVERT_DISABLE), ret);
    RT_ERR_CHK(apollo_raw_cf_usOp_set(entryIdx, CLASSIFY_INVERT_DISABLE), ret);

    return RT_ERR_OK;
} /* end of dal_apollo_classify_cfgEntry_del */


/* Function Name:
 *      dal_apollo_classify_field_add
 * Description:
 *      Add comparison field to an classfication configuration
 * Input:
 *      pClassifyEntry     - The classfication configuration that this function will add comparison rule
 *      pClassifyField     - The comparison rule that will be added.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              	- OK
 *      RT_ERR_FAILED          	- Failed
 *      RT_ERR_NULL_POINTER    	- Pointer pFilter_field or pFilter_cfg point to NULL.
 *      RT_ERR_INPUT 			- Invalid input parameters.
 * Note:
 *      This function add a comparison rule (*pClassifyField) to an ACL configuration (*pClassifyEntry).
 *      Pointer pFieldHead points to an Classification configuration structure, this structure keeps multiple
 *      comparison rules by means of linked list. Pointer pClassifyField will be added to linked
 *      list keeped by structure that pClassifyEntry points to.
 *      - caller should not free (*pClassifyField) before dal_apollo_classify_cfgEntry_add is called
 */
int32
dal_apollo_classify_field_add(rtk_classify_cfg_t *pClassifyEntry, rtk_classify_field_t *pClassifyField)
{
    rtk_classify_field_t *tmp_field;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pClassifyEntry), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pClassifyField), RT_ERR_NULL_POINTER);

    switch(pClassifyField->fieldType)
    {
        case CLASSIFY_FIELD_ETHERTYPE:
        case CLASSIFY_FIELD_TOS_DSIDX:
        case CLASSIFY_FIELD_TAG_VID:
        case CLASSIFY_FIELD_TAG_PRI:
        case CLASSIFY_FIELD_INTER_PRI:
        case CLASSIFY_FIELD_IS_CTAG:
        case CLASSIFY_FIELD_IS_STAG:
        case CLASSIFY_FIELD_UNI:
        break;

        case CLASSIFY_FIELD_PORT_RANGE:
        case CLASSIFY_FIELD_IP_RANGE:
        case CLASSIFY_FIELD_ACL_HIT:
        case CLASSIFY_FIELD_WAN_IF:
        case CLASSIFY_FIELD_IP6_MC:
        case CLASSIFY_FIELD_IP4_MC:
        case CLASSIFY_FIELD_MLD:
        case CLASSIFY_FIELD_IGMP:
        case CLASSIFY_FIELD_DEI:
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
        break;

    }

    if(NULL == pClassifyEntry->field.pFieldHead)
    {
        pClassifyEntry->field.pFieldHead = pClassifyField;
        pClassifyEntry->field.pFieldHead->next = NULL;
    }
    else
    {
        tmp_field = pClassifyEntry->field.pFieldHead;
        while(tmp_field->next != NULL)
            tmp_field = tmp_field->next;

        pClassifyField->next = NULL;
        tmp_field->next = pClassifyField;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_classify_field_add */


/* Function Name:
 *      dal_apollo_classify_unmatchAction_set
 * Description:
 *      Apply action to packets when no classfication configuration match
 * Input:
 *      None
 * Output:
 *      action - unmatch action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no classfication configruation matches.
 */
int32
dal_apollo_classify_unmatchAction_set(rtk_classify_unmatch_action_t action)
{
    apollo_raw_cf_usPermit_t raw_act;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((CLASSIFY_UNMATCH_END <= action), RT_ERR_INPUT);

    switch(action)
    {
        case CLASSIFY_UNMATCH_DROP:
            raw_act = APOLLO_RAW_CF_US_PERMIT_DROP;
            break;
        case CLASSIFY_UNMATCH_PERMIT_WITHOUT_PON:
            raw_act = APOLLO_RAW_CF_US_PERMIT_NOPON;
            break;
        case CLASSIFY_UNMATCH_PERMIT:
            raw_act = APOLLO_RAW_CF_US_PERMIT_NORMAL;
            break;
        default:
            return RT_ERR_INPUT;
    }
    RT_ERR_CHK(apollo_raw_cf_usPermit_set(raw_act), ret);

    return RT_ERR_OK;
} /* end of dal_apollo_classify_unmatchAction_set */


/* Function Name:
 *      dal_apollo_classify_unmatchAction_get
 * Description:
 *      Get action to packets when no classfication configuration match
 * Input:
 *      None
 * Output:
 *      pAction - Action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - Pointer pAction point to NULL.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function gets action of packets when no classfication configruation matches.
 */
int32
dal_apollo_classify_unmatchAction_get(rtk_classify_unmatch_action_t *pAction)
{
    apollo_raw_cf_usPermit_t raw_act;
    int32   ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pAction), RT_ERR_NULL_POINTER);

    RT_ERR_CHK(apollo_raw_cf_usPermit_get(&raw_act), ret);
    switch(raw_act)
    {
        case APOLLO_RAW_CF_US_PERMIT_NORMAL:
            *pAction = CLASSIFY_UNMATCH_PERMIT;
            break;
        case APOLLO_RAW_CF_US_PERMIT_NOPON:
            *pAction = CLASSIFY_UNMATCH_PERMIT_WITHOUT_PON;
            break;
        case APOLLO_RAW_CF_US_PERMIT_DROP:
            *pAction = CLASSIFY_UNMATCH_DROP;
            break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of dal_apollo_classify_unmatchAction_get */


/* Function Name:
 *      dal_apollo_classify_portRange_set
 * Description:
 *      Set Port Range check
 * Input:
 *      pRangeEntry - L4 Port Range entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      UpperPort must be larger or equal than lowerPort.
 *      This function is not supported in Test chip.
 */
int32
dal_apollo_classify_portRange_set(rtk_classify_rangeCheck_l4Port_t *pRangeEntry)
{
#if 0
    int32   ret;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pRangeEntry), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
} /* end of dal_apollo_classify_portRange_set */


/* Function Name:
 *      dal_apollo_classify_portRange_get
 * Description:
 *      Set Port Range check
 * Input:
 *      None
 * Output:
 *      pRangeEntry - L4 Port Range entry
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      This function is not supported in Test chip.
 */
int32
dal_apollo_classify_portRange_get(rtk_classify_rangeCheck_l4Port_t *pRangeEntry)
{
#if 0
    int32   ret;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pRangeEntry), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
} /* end of dal_apollo_classify_portRange_get */


/* Function Name:
 *      dal_apollo_classify_ipRange_set
 * Description:
 *      Set IP Range check
 * Input:
 *      pRangeEntry - IP Range entry
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_INPUT           - Input error
 * Note:
 *      UpperIp must be larger or equal than lowerIp.
 *      This function is not supported in Test chip.
 */
int32
dal_apollo_classify_ipRange_set(rtk_classify_rangeCheck_ip_t *pRangeEntry)
{
#if 0
    int32   ret;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pRangeEntry), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
} /* end of dal_apollo_classify_ipRange_set */


/* Function Name:
 *      dal_apollo_classify_ipRange_get
 * Description:
 *      Set IP Range check
 * Input:
 *      None.
 * Output:
 *      pRangeEntry - IP Range entry
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      This function is not supported in Test chip.
 */
int32
dal_apollo_classify_ipRange_get(rtk_classify_rangeCheck_ip_t *pRangeEntry)
{
#if 0
    int32   ret;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((NULL == pRangeEntry), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
} /* end of dal_apollo_classify_ipRange_get */

/* Function Name:
 *      dal_apollo_classify_cf_sel_set
 * Description:
 *      Set CF port selection, only pon port and RGMII port can be set
 * Input:
 *      port    - port id, only pon port and RGMII port can be set.
 *      cfSel   - CF port selection.
 * Output:
 *      pRangeEntry - IP Range entry
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      Only accept pon port and RGMII port.
 *      This function is not supported in Test chip.
 */
int32
dal_apollo_classify_cf_sel_set(rtk_port_t port, rtk_classify_cf_sel_t cfSel)
{
#if 0
    int32   ret;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((HAL_GET_PORTNUM() <= port), RT_ERR_INPUT);
    RT_PARAM_CHK((CLASSIFY_CF_SEL_END <= cfSel), RT_ERR_INPUT);

    return RT_ERR_OK;
} /* end of dal_apollo_classify_cf_sel_set */

/* Function Name:
 *      dal_apollo_classify_cf_sel_get
 * Description:
 *      Get CF port selection, only pon port and RGMII port can be get
 * Input:
 *      port    - port id, only pon port and RGMII port can be get.
 *      pCfSel  - pointer of CF port selection.
 * Output:
 *      pRangeEntry - IP Range entry
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      Only accept pon port and RGMII port.
 *      This function is not supported in Test chip.
 */
int32
dal_apollo_classify_cf_sel_get(rtk_port_t port, rtk_classify_cf_sel_t *pCfSel)
{
#if 0
    int32   ret;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((HAL_GET_PORTNUM() <= port), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pCfSel), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
} /* end of dal_apollo_classify_cf_sel_get */

/* Function Name:
 *      dal_apollo_classify_cfPri2Dscp_set
 * Description:
 *      Set CF priority to DSCP value mapping
 * Input:
 *      pri    - priority value
 *      dscp   - DSCP value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 * Note:
 *      This function is not supported in Test chip.
 */
extern int32
dal_apollo_classify_cfPri2Dscp_set(rtk_pri_t pri, rtk_dscp_t dscp)
{
#if 0
    int32   ret;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_DOT1P_PRIORITY_MAX < pri), RT_ERR_INPUT);
    RT_PARAM_CHK((RTK_VALUE_OF_DSCP_MAX < dscp), RT_ERR_INPUT);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_apollo_classify_cfPri2Dscp_get
 * Description:
 *      Get CF priority to DSCP value mapping
 * Input:
 *      pri    - priority value
 * Output:
 *      pDscp  - pointer of DSCP value.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_OUT_OF_RANGE    - The parameter is out of range
 *      RT_ERR_NULL_POINTER    - Pointer pClassifyCfg point to NULL.
 * Note:
 *      This function is not supported in Test chip.
 */
extern int32
dal_apollo_classify_cfPri2Dscp_get(rtk_pri_t pri, rtk_dscp_t *pDscp)
{
    #if 0
    int32   ret;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_CLASSIFY),"%s",__FUNCTION__);

    /* check Init status */
    RT_INIT_CHK(classify_init);

    /* parameter check */
    RT_PARAM_CHK((RTK_DOT1P_PRIORITY_MAX < pri), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pDscp), RT_ERR_NULL_POINTER);

    return RT_ERR_OK;
}

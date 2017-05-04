#ifndef _APOLLO_RAW_CLASSIFICATION_H_
#define _APOLLO_RAW_CLASSIFICATION_H_

/****************************************************************/
/* Header File inclusion                                        */
/****************************************************************/
#include <dal/apollo/raw/apollo_raw.h>

/****************************************************************/
/* Constant Definition                                          */
/****************************************************************/
#define APOLLO_RAW_CF_ETHERTYPE_NONE			0XFFFF
#define APOLLO_RAW_CF_ENTRYLEN	    			5
#define APOLLO_RAW_CF_UPACTTBLEN                1
#define APOLLO_RAW_CF_DSACTTBLEN                2



/****************************************************************/
/* Type Definition                                              */
/****************************************************************/
typedef enum apollo_raw_cf_patternNum_e{
    APOLLO_RAW_CF_PATTERN_0 = 0,
    APOLLO_RAW_CF_PATTERN_1,
    APOLLO_RAW_CF_PATTERN_2,
    APOLLO_RAW_CF_PATTERN_3,
    APOLLO_RAW_CF_PATTERN_END,
}apollo_raw_cf_patternNum_t;

typedef enum apollo_raw_cf_usPermit_e{
    APOLLO_RAW_CF_US_PERMIT_NORMAL = 0,
    APOLLO_RAW_CF_US_PERMIT_NOPON,
    APOLLO_RAW_CF_US_PERMIT_DROP,
    APOLLO_RAW_CF_US_PERMIT_END,
}apollo_raw_cf_usPermit_t;

typedef struct apollo_raw_cf_pattern_s{
    uint32 etherType;
    uint32 direction;     /* upstraeam or downstream configuration*/
    uint32 tos_gemidx;    /* TOS/TC for upstream,GEMIDX/LLIDX for downstream*/
    uint32 tagVid;
    uint32 tagPri;
    uint32 interPri;
    uint32 ifStag;
    uint32 ifCtag;
    uint32 uni;
}apollo_raw_cf_pattern_t;

typedef struct apollo_raw_cf_RuleEntry_s{
    uint32 idx;
    uint32 valid;
    apollo_raw_cf_pattern_t data_bits;
    apollo_raw_cf_pattern_t care_bits;
}apollo_raw_cf_RuleEntry_t;


typedef struct apollo_raw_cf_dsAct_s{
    uint32 idx;
	uint32 csact;
    uint32 cact;
    uint32 tag_vid;
    uint32 tag_pri;
    uint32 cvid_act;
    uint32 cpri_act;
    uint32 cfpri_act;
    uint32 cfpri;
    uint32 uni_act;
    uint32 uni_mask;

}apollo_raw_cf_dsAct_t;

typedef struct apollo_raw_cf_usAct_s{
    uint32 idx;
    uint32 csact;
    uint32 cact;
    uint32 tag_vid;
    uint32 tag_pri;
    uint32 csvid_act;
    uint32 cspri_act;
    uint32 sid_act;
    uint32 assign_idx;
}apollo_raw_cf_usAct_t;

extern int32 apollo_raw_cf_valid_set(uint32 index, rtk_enable_t enabled);
extern int32 apollo_raw_cf_valid_get(uint32 index, rtk_enable_t *pEnabled);
extern int32 apollo_raw_cf_rule_set(apollo_raw_cf_RuleEntry_t *pattern);
extern int32 apollo_raw_cf_rule_get(apollo_raw_cf_RuleEntry_t *pattern);
extern int32 apollo_raw_cf_dsAct_set(apollo_raw_cf_dsAct_t *pCfDsAct);
extern int32 apollo_raw_cf_dsAct_get(apollo_raw_cf_dsAct_t *pCfDsAct);
extern int32 apollo_raw_cf_usAct_set(apollo_raw_cf_usAct_t *pCfUsAct);
extern int32 apollo_raw_cf_usAct_get(apollo_raw_cf_usAct_t *pCfUsAct);
extern int32 apollo_raw_cf_dsOp_set(uint32 index, rtk_enable_t notDs);
extern int32 apollo_raw_cf_dsOp_get(uint32 index, rtk_enable_t *notDs);
extern int32 apollo_raw_cf_usOp_set(uint32 index, rtk_enable_t notUs);
extern int32 apollo_raw_cf_usOp_get(uint32 index, rtk_enable_t *notUs);
extern int32 apollo_raw_cf_usPermit_set(apollo_raw_cf_usPermit_t rule);
extern int32 apollo_raw_cf_usPermit_get(apollo_raw_cf_usPermit_t *pRule);
#endif /*_APOLLO_RAW_CLASSIFICATION_H_*/



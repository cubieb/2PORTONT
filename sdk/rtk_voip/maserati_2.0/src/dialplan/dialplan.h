#ifndef __DIALPLAN_H__
#define __DIALPLAN_H__

#include <regex.h>

#define REG_MAX_LENGTH MAX_DIALPLAN_LENGTH*5		//the max length of Regular Expression
#define MAX_REPLACE_TARGET_LENGTH 10

/* ======================================================
 * Dial plan internal context structure  
 * ====================================================== */

typedef struct replace_list_s {	
	regex_t				regReplace;
	unsigned char 		p_replace_target[MAX_REPLACE_TARGET_LENGTH];	
	unsigned int 			p_replace_soure_length;
	unsigned int 			p_replace_start;
	struct replace_list_s *	next;
} replace_list_t;


typedef struct dialplan_vars_s {
	unsigned int		bUseReplaceRule;
	unsigned char		bUseDialplan;
	unsigned char		bUsePrefixUnset;
    	unsigned char		bUsePMatchDialPlan;				//Flag For Partial Match
	//regex_t		regReplaceRule;
	replace_list_t *	replace;
	regex_t			regDialPlan;
	regex_t			regPrefixUnsetPlan;
    	regex_t			regPMatchDialPlan;				//For Processing Partial Match
	unsigned int		max_elements_dialplan;
	unsigned char *	p_replace_rule_target;
} dialplan_vars_t;

/* ======================================================
 * Dial plan parameter structure  
 * ====================================================== */
typedef struct {
	//unsigned char		replace_rule_option;									///< replace rule option
	unsigned char		dial_plan_enable;
	unsigned char *	p_replace_rule_source;	//[ MAX_REPLACE_RULE_SOURCE ];	///< replace rule source plan
	unsigned char *	p_replace_rule_target;	//[ MAX_REPLACE_RULE_TARGET ];	///< replace rule target
	unsigned char *	p_dialplan;				//[MAX_DIALPLAN_LENGTH];		///< dialplan
	unsigned char *	p_auto_prefix;			//[ MAX_AUTO_PREFIX ];			///< auto prefix
	unsigned char *	p_prefix_unset_plan;	//[ MAX_PREFIX_UNSET_PLAN ];	///< prefix unset plan
} dpInitParam_t;

typedef struct {
	unsigned char *	pDialData;
	unsigned int	nDialDataLen;
} dpMatchParam_t;

/* ======================================================
 * Dial plan regular expression parser functions.
 * (implement in dialplan.c)
 * ====================================================== */
/* Initialize dial plan module */
extern void InitializeDialPlan( dialplan_vars_t *me, const dpInitParam_t *pdpInitParam);

/* Uninitialize dial plan module */
extern void UninitializeDialPlan( dialplan_vars_t *me );

/* test whether user's input match dial plan */
extern int MatchDialProcess( dialplan_vars_t *me, const dpMatchParam_t *pdpMatchParam );

/* test whether user's input match prefix unset */
extern int MatchPrefixUnsetProcess( dialplan_vars_t *me, 
							 const unsigned char *pszDialString  );

/* test whether user's input match replace rule */
extern int MatchReplaceRuleProcess( dialplan_vars_t *me, 
							 const unsigned char *pszDialString,
							 int *idx_start,
							 int *idx_end ,unsigned char *target);

#endif /* __DIALPLAN_H__ */


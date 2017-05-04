#ifndef RTL867x_8367b_ACL_H
#define RTL867x_8367b_ACL_H


#include <linux/list.h>
#include "../../rtl8367b/rtk_api_acl.h"


#define RL6000_ACL_CHAIN_QOS			-30000


//#define DBG_8367B_ACL
#ifdef DBG_8367B_ACL
#define DBG_8367B_ACL_PRK printk
#else
#define DBG_8367B_ACL_PRK(format, args...)
#endif

typedef struct rtl867x_8367b_acl_chain_s
{
	int ruleCnt;
	int priority;
	struct list_head chain_list;
	struct list_head aclrule_list_head;
	int force;
}rtl867x_8367b_acl_chain_t;

typedef struct rtl867x_8367b_acl_chain_rule_s
{
	rtk_filter_cfg_t*	cfg;
	rtk_filter_action_t*	act;
	int 						asic_used_rulenum;
	struct list_head aclrule_list;
}rtl867x_8367b_acl_chain_rule_t;

int rtl867x_rtl8367b_register_aclchain(int priority, int force);
int rtl867x_rtl8367b_unregister_aclchain(int priority);
int rtl867x_rtl8367b_addacl(rtk_filter_cfg_t* cfg,rtk_filter_action_t* act, int chain_priority);
int rtl867x_rtl8367b_flushacl(int chain_priority);
void rtl867x_rtl8367b_show_aclrule_Datafiled(rtk_filter_cfg_t* pFilter_cfg);
void rtl867x_rtl8367b_show_aclrule_raw_Datafiled(rtk_filter_cfg_raw_t* pFilter_cfg);
void rtl867x_rtl8367b_show_aclrule(rtk_filter_cfg_t* pFilter_cfg,rtk_filter_action_t* pAction);
void rtl867x_rtl8367b_show_aclrule_raw(rtk_filter_cfg_raw_t* pFilter_cfg, rtk_filter_action_t* pAction);
void rtl867x_rtl8367b_show_aclchain(void);


/* APIs for manipulating cfg's field_data */
void rtl867x_rtl8367b_acl_fieldData_free(rtk_filter_field_t **fieldDataHead);

#endif

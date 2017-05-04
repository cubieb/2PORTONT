#ifndef RTL867x_8367b_QOS_H
#define RTL867x_8367b_QOS_H

#include <net/rtl/rtl867x_hwnat_qosrule.h>

#include "../../rtl8367b/rtk_api_qos.h"
#include "../../rtl8367b/rtk_api_acl.h"


//#define DBG_8367B_QOS
#ifdef DBG_8367B_QOS
#define DBG_8367B_QOS_PRK printk
#else
#define DBG_8367B_QOS_PRK(format, args...)
#endif

typedef struct rtl867x_8367b_qos_rule{

	/* matching data field*/
	rtk_filter_cfg_t* rule_cfg;

	/* qos policy */
	uint32			priority;
	uint32			swQid;	

	/* for linking list */
	struct list_head qos_rule_list;
} rtl867x_8367b_qos_rule_t;

int32 rtl867x_8367b_enableQos(int sp_queue_num, int wrr_queue_num, int ceil[],int rate[] ,int32 default_sw_qid);
int32 rtl867x_8367b_closeQos(void);
int	rtl867x_8367b_add_qosrule(rtl867x_hwnat_qos_rule_t* qos_rule,int priority);
void rtl867x_8367b_flush_qosrule(void);
void rtl867x_8367b_qosShowDebugInfo(void);
void rtl867x_8367b_flush_qosrule(void);
void rtl867x_8367b_qosShowSwRule(void);

#endif


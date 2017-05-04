#ifndef	RTL865X_OUTPUTQUEUE_H
#define	RTL865X_OUTPUTQUEUE_H
#include <linux/if.h>
#include <linux/list.h>
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl867x_hwnat_api.h>

//#define DBG_OutputQueue
#ifdef DBG_OutputQueue
#define DBG_OutputQueue_PRK printk
#else
#define DBG_OutputQueue_PRK(format, args...)
#endif

//#define DBG_QosRule
#ifdef DBG_QosRule
#define DBG_QosRule_PRK printk
#else
#define DBG_QosRule_PRK(format, args...)
#endif



#if	!defined(CONFIG_RTL_LAYERED_ASIC_DRIVER)
#define	RTL865XC_MNQUEUE_OUTPUTQUEUE		1
#define	RTL865XC_QOS_OUTPUTQUEUE				1

typedef struct rtl865xC_outputQueuePara_s {

	uint32	ifg;							/* default: Bandwidth Control Include/exclude Preamble & IFG */
	uint32	gap;							/* default: Per Queue Physical Length Gap = 20 */
	uint32	drop;						/* default: Descriptor Run Out Threshold = 500 */

	uint32	systemSBFCOFF;				/*System shared buffer flow control turn off threshold*/
	uint32	systemSBFCON;				/*System shared buffer flow control turn on threshold*/

	uint32	systemFCOFF;				/* system flow control turn off threshold */
	uint32	systemFCON;					/* system flow control turn on threshold */

	uint32	portFCOFF;					/* port base flow control turn off threshold */
	uint32	portFCON;					/* port base flow control turn on threshold */	

	uint32	queueDescFCOFF;				/* Queue-Descriptor=Based Flow Control turn off Threshold  */
	uint32	queueDescFCON;				/* Queue-Descriptor=Based Flow Control turn on Threshold  */

	uint32	queuePktFCOFF;				/* Queue-Packet=Based Flow Control turn off Threshold  */
	uint32	queuePktFCON;				/* Queue-Packet=Based Flow Control turn on Threshold  */
}	rtl865xC_outputQueuePara_t;
#endif

#define	MAX_QOS_RULE_NUM		10
#define	IPPROTO_ANY		256
#define	IPPROTO_BOTH		257

#define	QOS_DEF_QUEUE		0x4
#define	QOS_VALID_MASK		0x2
#define	QOS_TYPE_MASK		0x1
#define	QOS_TYPE_STR		0x0	/*0x0|QOS_VALID_MASK*/
#define	QOS_TYPE_WFQ		0x1	/*0x1|QOS_VALID_MASK*/

#define	EGRESS_BANDWIDTH_GRANULARITY			0x10000	/* 	64*1024	 */
#define	EGRESS_BANDWIDTH_GRANULARITY_BITMASK	0xffff
#define	EGRESS_BANDWIDTH_GRANULARITY_BITLEN	16

#define	INGRESS_BANDWIDTH_GRANULARITY_BITLEN	14

#define	EGRESS_WFQ_MAX_RATIO			0x80

#if 0
#define QOS_DEBUGP		printk
#else
#define QOS_DEBUGP(format, args...)
#endif

#if 0
#define QOS_RULE_DEBUGP		printk
#else
#define QOS_RULE_DEBUGP(format, args...)
#endif

/* priority decision array index */
enum PriDecIdx
{
	PORT_BASE	= 0,
	D1P_BASE, 
	DSCP_BASE, 
	ACL_BASE, 
	NAT_BASE,
	PRI_TYPE_NUM,
};


typedef struct {
	/*	classify	*/
	unsigned int protocol;
	ipaddr_t	local_ip_start;
	ipaddr_t 	local_ip_end;
	ipaddr_t 	remote_ip_start;
	ipaddr_t 	remote_ip_end;
	unsigned short lo_port_start;
	unsigned short lo_port_end;
	unsigned short re_port_start;
	unsigned short re_port_end;

	/*	tc	*/
	uint32		mark;
	unsigned char	prio;
	unsigned char	rate;
} rtl865x_qos_entry_t, *rtl865x_qos_entry_p;


typedef	rtl865x_qos_entry_t		QOS_T;
typedef	rtl865x_qos_entry_p		QOS_Tp;

typedef struct rtl865x_qos_rule{
	//char			inIfname[IFNAMSIZ];
	//char			outIfname[IFNAMSIZ];
	//rtl865x_AclRule_t *rule;

	/* matching data field*/
	rtl865x_AclRuleData_t	acl_rule_data;
	int						acl_rule_data_format;  /* RTL865X_ACL_XXXX */

	/* qos policy */
	uint32			priority;
	uint32			swQid;	
	uint32			remark_8021p;
	uint32			remark_dscp;

	/* for linking list */
	struct list_head qos_rule_list;
	char outIfname[IFNAMSIZ];
} rtl865x_qos_rule_t;


int32 rtl865x_qosSetBandwidth(uint32 memberPort, uint32 bps);
int32 rtl865x_qosFlushBandwidth(uint32 memberPort);

int   rtl865x_qosPriorityMappingGet(uint32 sw_Qidx,uint32 remark_8021p,uint32 remark_dscp);
void  rtl865x_qosPriorityMappingDeRef(uint32 priority);

int32 rtl865x_qosAddRule(rtl867x_hwnat_qos_rule_t *input_qos_rule ,int q_index ,int remark_8021p ,int remark_dscp);
int32 rtl865x_qosFlushRule(void);
int32 rtl865x_enableQos(int sp_queue_num, int wrr_queue_num, int ceil[],int rate[],int32 default_sw_qid);
int32 rtl865x_closeQos(void);
int32 rtl865x_Qos_SetRemarking(int enable_8021p,int enable_dscp);


int __init rtl865x_initOutputQueue(void);
void __exit rtl865x_exitOutputQueue(void);

#if defined(CONFIG_RTL_PROC_DEBUG)
int32 rtl865x_show_allQosAcl(void);
void rtl865x_qosShowDebugInfo(void);
#endif

#endif

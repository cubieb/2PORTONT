#include "rtl867x_8367b_qos.h"
#include "rtl867x_8367b_acl.h"
#include "../../rtl8367b/port.h"
#include "../../../../net/bridge/br_private.h"
#include "../../rtl8367b/rtl8367b_asicdrv_acl_para.h"
#include <net/rtl/rtl_nic.h>

static int rtl8367b_iprange_uesd_num 	= 0;
static int rtl8367b_portrange_uesd_num = 0;

#define rtl8367b_UPSTREAM_Quene_NUM		4
#define rtl8367b_Queue_NOT_Create		-1


struct list_head rtl8367b_qosRuleHead;

typedef struct rtl8367b_aclpriority_mapping_data_s
{
	uint8 valid;
	uint8 Qid;
	uint32 ref_count;
} rtl8367b_aclpriority_mapping_data_t;

static rtl8367b_aclpriority_mapping_data_t 	rtl8367b_priority_mapping[RTK_MAX_NUM_OF_PRIORITY];
static int32 							rtl8367b_priority_default =-1;
static uint32 						rtl8367b_hwQid2swQid_Mapping[RTK_MAX_NUM_OF_QUEUE] = {0};

/* 
	input : 
		is_dest  :    0: ipv4 source ip    1 : ipv4 destnation ip
	return :
		ip_range's index ,  if -1 : error 
*/
static int _rtl867x_8367b_add_iprange(uint32 ip_start,uint32 ip_end,int is_dest)
{	
	rtk_api_ret_t	ret_val;

	if(ip_end<ip_start)
	{			
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return -1;
	}	

	ret_val = rtk_filter_iprange_set(rtl8367b_iprange_uesd_num, is_dest? IPRANGE_IPV4_DIP : IPRANGE_IPV4_SIP, ip_end, ip_start);
	if(ret_val!=RT_ERR_OK)
    {
		DBG_8367B_QOS_PRK("Leave %s @ %d rtk_filter_iprange_set failed!\n",__func__,__LINE__);	
		return -1;
    }
	else
	{
		int result_index = rtl8367b_iprange_uesd_num;
		rtl8367b_iprange_uesd_num++;
		return result_index;
	}
}

static void _rtl867x_8367b_flush_iprange(void)
{	
	int i;

	for(i=0;i<RL6000_ACLRANGERULENUM;i++)
		rtk_filter_iprange_set(i, IPRANGE_UNUSED, 0, 0);	

	rtl8367b_iprange_uesd_num = 0;
}

/* 
	input : 
		is_dest  :    0: L4 source port    1 : L4 destnation port
	return :
		ip_range's index ,  if -1 : error 
*/
static int _rtl867x_8367b_add_portrange(uint16 port_start,uint16 port_end,int is_dest)
{	
	rtk_api_ret_t	ret_val;

	if(port_end<port_start)
	{			
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return -1;
	}	

	ret_val = rtk_filter_portrange_set(rtl8367b_portrange_uesd_num, is_dest? PORTRANGE_DPORT : PORTRANGE_SPORT, port_end, port_start);
	if(ret_val!=RT_ERR_OK)
    {
		DBG_8367B_QOS_PRK("Leave %s @ %d rtk_filter_portrange_set failed!\n",__func__,__LINE__);	
		return -1;
    }
	else
	{
		int result_index = rtl8367b_portrange_uesd_num;
		rtl8367b_portrange_uesd_num++;
		return result_index;
	}
}

static void _rtl867x_8367b_flush_portrange(void)
{	
	int i;

	for(i=0;i<RL6000_ACLRANGERULENUM;i++)	
		rtk_filter_portrange_set(i, PORTRANGE_UNUSED, 0, 0);	

	rtl8367b_portrange_uesd_num = 0;
}

int _rtl867x_8367b_set_port_priority(int port, int priority)
{
	if(rtk_qos_portPri_set(port, priority)!=RT_ERR_OK)
	{
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}
	return SUCCESS;
}



static int my_gcd(int numA, int numB)
{
	int	tmp;
	int	divisor;

	if (numA<numB)
	{
		tmp = numA;
		numA = numB;
		numB = tmp;
	}

	divisor = numA%numB;
	while(divisor)
	{
		numA = numB;
		numB = divisor;
		divisor = numA%numB;
	}

	return numB;
}


static void _rtl867x_8367b_qosArrangeQueue(int ceil[],int rate[],uint32 bands_num)
{	
	int	i; 		
	

	#ifdef DBG_8367B_QOS
	DBG_8367B_QOS_PRK("Enter %s\n",__func__);		
	DBG_8367B_QOS_PRK("(%s)------------Before--------------------\n",__func__);
	for(i=0; i<bands_num; i++)
		DBG_8367B_QOS_PRK("(%s) [Queue %d] bandwidth:%d(bits)  ceil:%d\n",__func__,i,rate[i],ceil[i]);		
	DBG_8367B_QOS_PRK("-------------------------------------------------------------\n");
	#endif	

	
	/* bandwidth
		only can be set as 1~128 ,  if the maxBandwidth>128 ..... 

		 ex. bandwidth_0 = 3  , bandwidth_1 = 254 , bandwidth_2 = 257
		 we shift right these value until  the maxBandwidth/divisor <= 128   */
	
	do
	{
		int	divisor;
		int	maxBandwidth;
		
		for(i=0; i<bands_num; i++)
		{		
			if(rate[i]==0)
				rate[i] = 1;			
		}

		/* calculate gcd */
		divisor = rate[0];
		for(i=1; i<bands_num; i++)				
			divisor = my_gcd(rate[i], divisor);		
		
		for(i=0; i<bands_num; i++)				
			rate[i] = rate[i]/divisor;		

		/* maxBandwidth has to <= 128*/
		maxBandwidth = 0;
		for(i=0; i<bands_num; i++)
		{		
			if(maxBandwidth<rate[i])
				maxBandwidth = rate[i];		
		}
		if(maxBandwidth<=QOS_WEIGHT_MAX)
			break;

		/* start right shift .. */
		for(i=0; i<bands_num; i++)				
			rate[i] = rate[i]>>1 ;		
		
	}while(1);



	#ifdef DBG_8367B_QOS	
	DBG_8367B_QOS_PRK("(%s)------------After--------------------\n",__func__);
	for(i=0; i<bands_num; i++)
		DBG_8367B_QOS_PRK("(%s) [Queue %d] bandwidth:%d(bits)  ceil:%d\n",__func__,i,rate[i],ceil[i]);		
	DBG_8367B_QOS_PRK("-------------------------------------------------------------\n");	
	DBG_8367B_QOS_PRK("Leave %s\n",__func__);
	#endif


	
	
}

static int _rtl867x_8367b_qos_setQueueInfo(int sp_queue_num, int wrr_queue_num,int queue_number,int ceil[],int rate[])
{
	/*   Start  to set Queue scheduling ..  
		8676 embeded switch 	: set queue's number /  set queue's scheduling info / enable remark

		ex. if sp_queue_num=1 , wrr_queue_num=2   queue_number=4

			hw_qid : 0   1   2   3   4   5 
			sw_qid:  1   2   -1   0  -1   -1

			we only use the first 4 hw Queues 
			set hw_Queue 0 & 1 as WRR type
			set the remainging hw_Queue 2 & 3 as SP type (we will not use hw_Queue 2)
	*/		
	rtk_qos_queue_weights_t qweights;
	int queue_idx;
	memset(&qweights,0,sizeof(rtk_qos_queue_weights_t));

	if(sp_queue_num+wrr_queue_num >queue_number)
		return FAILED;
	
	/* 2.1	set queue's number	*/
	if(rtk_qos_init(queue_number)!=RT_ERR_OK) /* all ports uses 4 queues */
		return FAILED;

	/* 2.2	set queue's scheduling info  */ 			
	for (queue_idx=0;queue_idx<queue_number;queue_idx++)
	{
		if(queue_idx < wrr_queue_num) /* wrr */ 
			qweights.weights[queue_idx] = rate[queue_idx]; 		
		else /* sp */
			qweights.weights[queue_idx] = 0;									
	}	
	
	if(rtk_qos_schedulingQueue_set(RL6000_CPU_PORT,&qweights)!=RT_ERR_OK)	
		return FAILED;	


	return SUCCESS;
}

/* Get hw_Qid */
static int32 _rtl867x_8367b_qosQIDMappingGet( uint32 sw_Qidx)
{
	int	i;

	for(i=0; i < RTK_MAX_NUM_OF_QUEUE; i++)
	{
		if (rtl8367b_hwQid2swQid_Mapping[i] == sw_Qidx)
		{
			return i;
		}
	}

	return -1;
}


static int _rtl867x_8367b_qosQIDMappingSet(int32 Qidmapping[RTK_MAX_NUM_OF_QUEUE])
{		
	memcpy(rtl8367b_hwQid2swQid_Mapping,Qidmapping,RTK_MAX_NUM_OF_QUEUE*sizeof(int));	
	return SUCCESS;
}


static void _rtl867x_8367b_qosQIDMappingClear(void)
{	
	int i;		
	for(i=0;i<RTK_MAX_NUM_OF_QUEUE;i++)
		rtl8367b_hwQid2swQid_Mapping[i] = rtl8367b_Queue_NOT_Create;	
	
}

static int _rtl867x_8367b_qosPriorityMappingTakeEffect(void)
{	
	int32  i;	

	rtk_qos_pri2queue_t pri2qid;
	memset(&pri2qid,0,sizeof(rtk_qos_pri2queue_t));

	for(i=0;i<RTK_MAX_NUM_OF_PRIORITY;i++)		
	{
		pri2qid.pri2queue[i] = rtl8367b_priority_mapping[i].Qid;
	}
	
	if(rtk_qos_priMap_set(rtl8367b_UPSTREAM_Quene_NUM,&pri2qid)!=RT_ERR_OK)
		return FAILED;	

	return SUCCESS;	
}


static void _rtl867x_8367b_qosPriorityMappingDeRef(uint32 priority)	
{

	if(priority<0 || priority>=RTK_MAX_NUM_OF_PRIORITY)
		return;

	rtl8367b_priority_mapping[priority].ref_count--;
	if(rtl8367b_priority_mapping[priority].ref_count==0)
	{
		memset(&rtl8367b_priority_mapping[priority], 0,sizeof(rtl8367b_aclpriority_mapping_data_t));
		if(_rtl867x_8367b_qosPriorityMappingTakeEffect()!=SUCCESS)
		{
			printk("%s@ %d(return FAILED)\n",__func__,__LINE__);
			DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);		
		}
	}		

}

/* return ...     -1:   no avaliable acl priority mapping   others: acl priority 
	Note. At the same time ,ref_count ++ */
static int _rtl867x_8367b_qosPriorityMappingGet(uint32 sw_Qidx)
{
	int32  i,empty_idx,hw_queue_idx;	

	DBG_8367B_QOS_PRK("Enter %s (sw_Qidx:%d)\n"	,__func__,sw_Qidx); 
	
	hw_queue_idx = _rtl867x_8367b_qosQIDMappingGet(sw_Qidx);
	if(hw_queue_idx==-1)
	{
		DBG_8367B_QOS_PRK("Leave %s @ %d\n",__func__,__LINE__);	
		return -1;	
	}

	DBG_8367B_QOS_PRK("(%s)  hw_queue_idx=%d \n",__func__,hw_queue_idx);	
	

	for(i=0;i<RTK_MAX_NUM_OF_PRIORITY;i++)
	{
		if( rtl8367b_priority_mapping[i].valid 
			&& rtl8367b_priority_mapping[i].Qid == hw_queue_idx)
		{

			rtl8367b_priority_mapping[i].ref_count++;		
			DBG_8367B_QOS_PRK("Leave %s (return existed priority %d, ref:%d)\n",__func__,i,rtl8367b_priority_mapping[i].ref_count);
			return i;
		}
	}

	/* cannot find out matching entry, create the new one*/
	empty_idx = -1;
	for(i=0;i<RTK_MAX_NUM_OF_PRIORITY;i++)
	{
		if(rtl8367b_priority_mapping[i].valid==0)
		{
			empty_idx = i;
			break;
		}
	}
	if(empty_idx==-1)
	{
		DBG_8367B_QOS_PRK("Leave %s (return %d)\n",__func__,-1);
		return -1; /* no empty entry */
	}

	rtl8367b_priority_mapping[empty_idx].valid 			= 1	;
	rtl8367b_priority_mapping[empty_idx].Qid 			= hw_queue_idx;
	rtl8367b_priority_mapping[empty_idx].ref_count		= 1	;


	/*  let new acl_priority mapping take effect */
	if(_rtl867x_8367b_qosPriorityMappingTakeEffect()!=SUCCESS)
	{
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}	

	DBG_8367B_QOS_PRK("Leave %s (return new priority %d, ref:%d)\n",__func__,i,rtl8367b_priority_mapping[i].ref_count);
	return empty_idx;
	
}


int32 rtl867x_8367b_enableQos(int sp_queue_num, int wrr_queue_num, int ceil[],int rate[] ,int32 default_sw_qid)
{

	int total_queue_num = sp_queue_num + wrr_queue_num;
	int Qidmapping[RTK_MAX_NUM_OF_QUEUE];
	rtk_priority_select_t priDec;	
	int i;

	DBG_8367B_QOS_PRK("Enter %s (sp_queue_num:%d   wrr_queue_num:%d   default_sw_qid:%d)\n"
		,__func__,sp_queue_num,wrr_queue_num,default_sw_qid);

	/* 1. check queue number */
	if (sp_queue_num<0 || wrr_queue_num<0 )
	{
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED) \n",__func__,__LINE__);
		return FAILED;	
	}
	
	if( total_queue_num<1 || total_queue_num>rtl8367b_UPSTREAM_Quene_NUM)
	{
		printk("(%s)Warning!! rtl8367b switch only support 1~%d outputqueues in upstream (Your input is %d) \n"
			,__func__,rtl8367b_UPSTREAM_Quene_NUM,total_queue_num);
		return FAILED;	
	}
	
	if (default_sw_qid<0 || default_sw_qid>=total_queue_num)
	{
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED) \n",__func__,__LINE__);
		return FAILED;	
	}

	/* 2.  Re-adjust qosinfo's para  */
	if(wrr_queue_num>0)
	{
		if(ceil==NULL || rate==NULL)
		{
			DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED) \n",__func__,__LINE__);
			return FAILED;	

		}

		_rtl867x_8367b_qosArrangeQueue(ceil,rate,wrr_queue_num);
	}


	/* 3.  set QueueID mapping
		ex. if we create 4 queues (2 is priority , 2 is wrr )
			sw_qid ==>	0 : the highest priority 
						1 : the second priority 
						2 : 1st wrr queue
						3 : 2nd wrr queue
			
			hw_qid : 0   1   2   3    4     5   6    7    8
			sw_qid:  2   3    1   0  -1   -1  -1   -1   -1 

		ex. if we create 3 queues (1 is priority , 2 is wrr )
			sw_qid ==>	0 : the highest priority 					
						1 : 1st wrr queue
						2 : 2nd wrr queue
			
			hw_qid : 0   1   2   3   4     5    6    7    8
			sw_qid:  1   2   -1   0  -1   -1  -1  -1   -1 
	*/	
	for(i=0;i<RTK_MAX_NUM_OF_QUEUE;i++)
		Qidmapping[i] = rtl8367b_Queue_NOT_Create;

	for(i=0;i<sp_queue_num;i++)		
		Qidmapping[rtl8367b_UPSTREAM_Quene_NUM-1-i] = i;
	
	for(i=0;i<wrr_queue_num;i++)	
		Qidmapping[i] = sp_queue_num+i;
	
	if(_rtl867x_8367b_qosQIDMappingSet(Qidmapping)!=SUCCESS)
	{			
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}

	/*  4. Start  to set Queue scheduling ..  */
	if(_rtl867x_8367b_qos_setQueueInfo(sp_queue_num,wrr_queue_num,rtl8367b_UPSTREAM_Quene_NUM,ceil,rate)!=SUCCESS)
	{
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}

	/*  5. set default acl priority mapping  */
	rtl8367b_priority_default = _rtl867x_8367b_qosPriorityMappingGet(default_sw_qid);
	if(rtl8367b_priority_default==-1)
	{			
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}


	/*  6. init priority setting except fot ACL based in hw 
		RL6000	: port-based (for default priority)
	*/	
	for(i=0; i<RTK_MAX_NUM_OF_PORT; i++)
		_rtl867x_8367b_set_port_priority(i,rtl8367b_priority_default);
	
	
	/*  7.  set  priority  between different qos based 
		RL6000	: ACL-based > port based
	*/	
	memset(&priDec,0,sizeof(rtk_priority_select_t));
	priDec.acl_pri   	= 7;
	priDec.port_pri 	= 6;		
	priDec.dot1q_pri 	= 0;	
	priDec.dscp_pri 	= 0;		
	priDec.cvlan_pri 	= 0;
	priDec.svlan_pri 	= 0;
	priDec.dmac_pri 	= 0;
	priDec.smac_pri 	= 0;
	if(rtk_qos_priSel_set(&priDec)!=RT_ERR_OK )
		return FAILED;


	DBG_8367B_QOS_PRK("Leave %s\n",__func__);
	return SUCCESS;
	
}


int32 rtl867x_8367b_closeQos(void)
{	
	uint32	i;

	DBG_8367B_QOS_PRK("Enter %s\n",__func__);

	/* 1.  clear Qid  mapping and acl priotiy mapping */
	_rtl867x_8367b_qosQIDMappingClear();	

	/*  2. flush all sw qos rules */	
	rtl867x_8367b_flush_qosrule();

	/*  3. clear default acl priority */
	_rtl867x_8367b_qosPriorityMappingDeRef(rtl8367b_priority_default);
	
	/* 4. check whether all ref count reaches zero ? */
	for(i=0;i<RTK_MAX_NUM_OF_PRIORITY;i++)
	{
		if( rtl8367b_priority_mapping[i].valid)
		{			
			printk("(%s)!!!!BUG!!!!  @%d (return %d, ref:%d)\n",__func__,__LINE__,i,rtl8367b_priority_mapping[i].ref_count);		
		}
	}
	
	/*  5. Start  to set Queue scheduling .. (back to number 1)  */
	if(_rtl867x_8367b_qos_setQueueInfo(1,0,1,NULL,NULL)!=SUCCESS)
	{
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}

	DBG_8367B_QOS_PRK("Leave %s\n",__func__);
	return SUCCESS;
}



/* Create a complete 8367b cfg & act  from 8367b SW QoS Rule */
static rtk_filter_action_t* __rtl867x_8367b_qosCreateACLrule(rtl867x_8367b_qos_rule_t* input_rule)
{

	rtk_filter_action_t* act;
	
	act = kmalloc(sizeof(rtk_filter_action_t),GFP_KERNEL);
	if(!act)
		return NULL;

   	memset(act, 0x00, sizeof(rtk_filter_action_t));

	
	act->actEnable[FILTER_ENACT_PRIORITY] = TRUE;
	act->filterPriority 			= input_rule->priority;
	
	return act;

}


/*  	
	FUNC 	: _rtl865x_qosAddRule()
	USAGE	: Add a new sw_qos entry into sw_qos_list , and add asic acl rule 
		
	Note.  
		1. It is caller's respnsibility to promise that all fileds  (except for linking list) in input_rule has been assigned correctly
		2. You have to get priority first. (Ref priority)	
*/

static int _rtl867x_8367b_qosAddRule(rtl867x_8367b_qos_rule_t* input_rule)
{
	rtk_filter_action_t*			rule_act;

	rule_act = __rtl867x_8367b_qosCreateACLrule(input_rule);
	if(!rule_act)
		return FAILED;	

	if (rtl867x_rtl8367b_addacl(input_rule->rule_cfg, rule_act,RL6000_ACL_CHAIN_QOS) != RT_ERR_OK)
	{
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		kfree(rule_act);
		return FAILED;
	}	

	kfree(rule_act); /* add_acl will memcpy */
	INIT_LIST_HEAD(&input_rule->qos_rule_list);
	list_add_tail(&input_rule->qos_rule_list , &rtl8367b_qosRuleHead);
	
	return SUCCESS;
}

/* Convert RTL867x IPQoS Format to rtl8367b acl format 
	Note. It only write un_ty and ruleType_
*/
static rtk_filter_cfg_t* _rtl867x_8367b_qosConvertDataForamt(rtl867x_hwnat_qos_rule_t* input_rule)
{
	rtk_filter_cfg_t* 	output_cfg=NULL;
	rtk_filter_field_t*	field_smac=NULL;
	rtk_filter_field_t*	field_dmac=NULL;
	rtk_filter_field_t*	field_ctag=NULL;
	rtk_filter_field_t*	field_ethtype=NULL;
	rtk_filter_field_t*	field_scrIP=NULL;
	rtk_filter_field_t*	field_dstIP=NULL;
	rtk_filter_field_t*	field_ipRange=NULL;
	rtk_filter_field_t*	field_tos=NULL;
	rtk_filter_field_t*	field_ipProto=NULL;
	rtk_filter_field_t*	field_portRange=NULL;
	rtk_filter_field_t*	field_always_match=NULL;
	int physical_port=-1;
	int add_field = 0;

	output_cfg = kmalloc(sizeof(rtk_filter_cfg_t),GFP_KERNEL);
	if(!output_cfg)
		goto convert_fail;

	memset(output_cfg, 0x00, sizeof(rtk_filter_cfg_t));


	/*   Physical port  */
	if(input_rule->rule_type != RTL867x_IPQos_Format_srcPort_setMask && input_rule->match_field.PHY._lan_netifname[0]!='\0')
	{	
		struct net_device *dev = re865x_get_netdev_by_name(input_rule->match_field.PHY._lan_netifname);
		
		/*  check whether this interface is  under br0 */
		if(dev)
		{
			struct net_bridge_port *br_port = dev->br_port;
			if(br_port && !strcmp(dev->br_port->br->dev->name,RTL_DRV_LAN_NETIF_NAME))
			{
				int port_idx =0 ;	

				for(port_idx=0;port_idx<RL6000_CPU_PORT;port_idx++)
				{
					if((1<<port_idx)&((struct dev_priv *)dev->priv)->portmask)

					{
						physical_port=(1<<port_idx);
						break;
					}
				}					
			}
		}
	}
	else if(input_rule->rule_type == RTL867x_IPQos_Format_srcPort_setMask && input_rule->match_field.PHY._lan_portmask!=0)
	{
		physical_port=input_rule->match_field.PHY._lan_portmask;
	}
	
	/* Source MAC */
	if(input_rule->match_field.L2._srcMacMask.octet[0]	|| input_rule->match_field.L2._srcMacMask.octet[1]
		|| input_rule->match_field.L2._srcMacMask.octet[2] || input_rule->match_field.L2._srcMacMask.octet[3]
		|| input_rule->match_field.L2._srcMacMask.octet[4] || input_rule->match_field.L2._srcMacMask.octet[5])
	{	
		field_smac = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
   		memset(field_smac, 0x00, sizeof(rtk_filter_field_t));	

		field_smac->fieldType = FILTER_FIELD_SMAC;
		field_smac->filter_pattern_union.smac.dataType = FILTER_FIELD_DATA_MASK;
		memcpy(field_smac->filter_pattern_union.smac.value.octet,input_rule->match_field.L2._srcMac.octet,ETHER_ADDR_LEN);
		memcpy(field_smac->filter_pattern_union.smac.mask.octet,input_rule->match_field.L2._srcMacMask.octet,ETHER_ADDR_LEN);

		if( rtk_filter_igrAcl_field_add(output_cfg, field_smac) != RT_ERR_OK)
			goto convert_fail;		

		add_field = 1;
	}

	/* Destination  MAC */
	if(input_rule->match_field.L2._dstMacMask.octet[0]	|| input_rule->match_field.L2._dstMacMask.octet[1]
		|| input_rule->match_field.L2._dstMacMask.octet[2] || input_rule->match_field.L2._dstMacMask.octet[3]
		|| input_rule->match_field.L2._dstMacMask.octet[4] || input_rule->match_field.L2._dstMacMask.octet[5])
	{	
		field_dmac = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
   		memset(field_dmac, 0x00, sizeof(rtk_filter_field_t));	

		field_dmac->fieldType = FILTER_FIELD_DMAC;
		field_dmac->filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;
		memcpy(field_dmac->filter_pattern_union.dmac.value.octet,input_rule->match_field.L2._dstMac.octet,ETHER_ADDR_LEN);
		memcpy(field_dmac->filter_pattern_union.dmac.mask.octet,input_rule->match_field.L2._dstMacMask.octet,ETHER_ADDR_LEN);

		if( rtk_filter_igrAcl_field_add(output_cfg, field_dmac) != RT_ERR_OK)
			goto convert_fail;	

		add_field = 1;
	}

	/* VLAN ID , 802.1p  */
	if(input_rule->match_field.L2._vlanidMask || input_rule->match_field.L2._8021pMask)
	{
		field_ctag = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
		memset(field_ctag, 0x00, sizeof(rtk_filter_field_t));

		field_ctag->fieldType = FILTER_FIELD_CTAG;
		
		if(input_rule->match_field.L2._vlanidMask)
		{
			field_ctag->filter_pattern_union.ctag.vid.dataType = FILTER_FIELD_DATA_MASK;
			field_ctag->filter_pattern_union.ctag.vid.value = input_rule->match_field.L2._vlanid;
			field_ctag->filter_pattern_union.ctag.vid.mask = input_rule->match_field.L2._vlanidMask;
		}

		if(input_rule->match_field.L2._8021pMask)
		{
			field_ctag->filter_pattern_union.ctag.pri.dataType = FILTER_FIELD_DATA_MASK;
			field_ctag->filter_pattern_union.ctag.pri.value = input_rule->match_field.L2._8021p;
			field_ctag->filter_pattern_union.ctag.pri.mask = input_rule->match_field.L2._8021pMask;
		}

		if( rtk_filter_igrAcl_field_add(output_cfg, field_ctag) != RT_ERR_OK)
			goto convert_fail;	

		add_field = 1;
	}

	/* Ether Type  */
	if(input_rule->match_field.L2._ethTypeMask)
	{
		field_ethtype = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
		memset(field_ethtype, 0x00, sizeof(rtk_filter_field_t));

		field_ethtype->fieldType = FILTER_FIELD_ETHERTYPE;
		field_ethtype->filter_pattern_union.etherType.dataType = FILTER_FIELD_DATA_MASK;
		field_ethtype->filter_pattern_union.etherType.value = input_rule->match_field.L2._ethType;
		field_ethtype->filter_pattern_union.etherType.mask = input_rule->match_field.L2._ethTypeMask;

		if( rtk_filter_igrAcl_field_add(output_cfg, field_ethtype) != RT_ERR_OK)
			goto convert_fail;	

		add_field = 1;
	}

	/* Source IP  */	
	if(	(	input_rule->rule_type == RTL867x_IPQos_Format_IP 
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_TCP
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_UDP
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_srcFilter
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_8367Ext_IPMask	)
	
		&& (input_rule->match_field.L3.ip.mask._sipMask))
	{
		field_scrIP = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
		memset(field_scrIP, 0x00, sizeof(rtk_filter_field_t));

		field_scrIP->fieldType = FILTER_FIELD_IPV4_SIP;
		field_scrIP->filter_pattern_union.sip.dataType = FILTER_FIELD_DATA_MASK;
		field_scrIP->filter_pattern_union.sip.value = input_rule->match_field.L3.ip.mask._sip;
		field_scrIP->filter_pattern_union.sip.mask = input_rule->match_field.L3.ip.mask._sipMask;

		if( rtk_filter_igrAcl_field_add(output_cfg, field_scrIP) != RT_ERR_OK)
			goto convert_fail;	

		add_field = 1;
	}

	/* Destination  IP  */
	if(	(	input_rule->rule_type == RTL867x_IPQos_Format_IP 
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_TCP
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_UDP
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_srcFilter
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_8367Ext_IPMask	)
	
		&& (input_rule->match_field.L3.ip.mask._dipMask))
	{
		field_dstIP = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
		memset(field_dstIP, 0x00, sizeof(rtk_filter_field_t));

		field_dstIP->fieldType = FILTER_FIELD_IPV4_DIP;
		field_dstIP->filter_pattern_union.dip.dataType = FILTER_FIELD_DATA_MASK;
		field_dstIP->filter_pattern_union.dip.value = input_rule->match_field.L3.ip.mask._dip;
		field_dstIP->filter_pattern_union.dip.mask = input_rule->match_field.L3.ip.mask._dipMask;

		if( rtk_filter_igrAcl_field_add(output_cfg, field_dstIP) != RT_ERR_OK)
			goto convert_fail;	

		add_field = 1;
	}	
	
	/* Source IP (Range) , Destination  IP(Range) */
	if(	(	input_rule->rule_type == RTL867x_IPQos_Format_IP_Range 
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_TCP_Range
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_UDP_Range
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_srcFilter_Range
		|| 	input_rule->rule_type == RTL867x_IPQos_Format_8367Ext_IPRange	)
		&& ((input_rule->match_field.L3.ip.range._sip_start!=0 && input_rule->match_field.L3.ip.range._sip_end!=0)
			||(input_rule->match_field.L3.ip.range._dip_start!=0 && input_rule->match_field.L3.ip.range._dip_end!=0)))
	{
		int ip_range_bitmask = 0;
		field_ipRange = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
		memset(field_ipRange, 0x00, sizeof(rtk_filter_field_t));
		
		if(input_rule->match_field.L3.ip.range._sip_start!=0 && input_rule->match_field.L3.ip.range._sip_end!=0)
		{
			int sip_range_idx = _rtl867x_8367b_add_iprange(input_rule->match_field.L3.ip.range._sip_start
														,input_rule->match_field.L3.ip.range._sip_end,0);

			if(sip_range_idx!=-1)
				ip_range_bitmask |= (1<<sip_range_idx);
			else
				goto convert_fail;
		}

		if(input_rule->match_field.L3.ip.range._dip_start!=0 && input_rule->match_field.L3.ip.range._dip_end!=0)
		{
			int dip_range_idx = _rtl867x_8367b_add_iprange(input_rule->match_field.L3.ip.range._dip_start
														,input_rule->match_field.L3.ip.range._dip_end,1);

			if(dip_range_idx!=-1)
				ip_range_bitmask |= (1<<dip_range_idx);
			else
				goto convert_fail;
		}		

		field_ipRange->fieldType = FILTER_FIELD_IP_RANGE;
		field_ipRange->filter_pattern_union.inData.dataType = FILTER_FIELD_DATA_MASK; 
		field_ipRange->filter_pattern_union.inData.value = ip_range_bitmask;
		field_ipRange->filter_pattern_union.inData.mask = 0xFFFFFFFF;

		if( rtk_filter_igrAcl_field_add(output_cfg, field_ipRange) != RT_ERR_OK)
			goto convert_fail;	

		add_field = 1;
	}

	/* TOS  */
	if(input_rule->match_field.L3._tosMask)
	{
		field_tos = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
		memset(field_tos, 0x00, sizeof(rtk_filter_field_t));

		field_tos->fieldType = FILTER_FIELD_IPV4_TOS;
		field_tos->filter_pattern_union.ipTos.dataType = FILTER_FIELD_DATA_MASK;
		field_tos->filter_pattern_union.ipTos.value = input_rule->match_field.L3._tos;
		field_tos->filter_pattern_union.ipTos.mask = input_rule->match_field.L3._tosMask;

		if( rtk_filter_igrAcl_field_add(output_cfg, field_tos) != RT_ERR_OK)
			goto convert_fail;	

		add_field = 1;
	}

	/*  L4 Protocol  */
	if(input_rule->match_field.L3._ipProtoMask)
	{
		field_ipProto = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
		memset(field_ipProto, 0x00, sizeof(rtk_filter_field_t));

		field_ipProto->fieldType = FILTER_FIELD_IPV4_PROTOCOL;
		field_ipProto->filter_pattern_union.protocol.dataType = FILTER_FIELD_DATA_MASK;
		field_ipProto->filter_pattern_union.protocol.value = input_rule->match_field.L3._ipProto;
		field_ipProto->filter_pattern_union.protocol.mask = input_rule->match_field.L3._ipProtoMask;

		if( rtk_filter_igrAcl_field_add(output_cfg, field_ipProto) != RT_ERR_OK)
			goto convert_fail;	

		add_field = 1;
	}

	/*  TCP/UDP Port  */
	if((input_rule->match_field.L4._sport_start!=0 && input_rule->match_field.L4._sport_end!=0) 
		|| (input_rule->match_field.L4._dport_start!=0 && input_rule->match_field.L4._dport_end!=0))
	{	
		int port_range_bitmask = 0;
		field_portRange = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
		memset(field_portRange, 0x00, sizeof(rtk_filter_field_t));

		
		if(input_rule->match_field.L4._sport_start!=0 && input_rule->match_field.L4._sport_end!=0)
		{
			int sport_range_idx = _rtl867x_8367b_add_portrange(input_rule->match_field.L4._sport_start
														,input_rule->match_field.L4._sport_end,0);

			if(sport_range_idx!=-1)
				port_range_bitmask |= (1<<sport_range_idx);
		}

		if(input_rule->match_field.L4._dport_start!=0 && input_rule->match_field.L4._dport_end!=0)
		{
			int dport_range_idx = _rtl867x_8367b_add_portrange(input_rule->match_field.L4._dport_start
														,input_rule->match_field.L4._dport_end,1);

			if(dport_range_idx!=-1)
				port_range_bitmask |= (1<<dport_range_idx);
		}			

		field_portRange->fieldType = FILTER_FIELD_PORT_RANGE;
		field_portRange->filter_pattern_union.inData.dataType = FILTER_FIELD_DATA_MASK; 
		field_portRange->filter_pattern_union.inData.value = port_range_bitmask;
		field_portRange->filter_pattern_union.inData.mask = 0xFFFFFFFF;

		if( rtk_filter_igrAcl_field_add(output_cfg, field_portRange) != RT_ERR_OK)
			goto convert_fail;

		add_field = 1;
	}


	/*  If there's no field added into cfg, create a dump always match field */
	if(add_field == 0)
	{	
		DBG_8367B_QOS_PRK("(%s)no field added into cfg, create a dump always match field\n",__func__);
	
		field_always_match = kmalloc(sizeof(rtk_filter_field_t),GFP_KERNEL);
   		memset(field_always_match, 0x00, sizeof(rtk_filter_field_t));	

		field_always_match->fieldType = FILTER_FIELD_SMAC;
		field_always_match->filter_pattern_union.smac.dataType = FILTER_FIELD_DATA_MASK;	

		if( rtk_filter_igrAcl_field_add(output_cfg, field_always_match) != RT_ERR_OK)
			goto convert_fail;	

		add_field = 1;
	}


	output_cfg->activeport.dataType = FILTER_FIELD_DATA_MASK;

  	if(physical_port!=-1) 
   		output_cfg->activeport.value    = (physical_port); /* active on specific lan port  */
	else
		output_cfg->activeport.value    = RL6000_LAN_PORT_BITMAP; /* active on all LAN ports  */

   	output_cfg->activeport.mask	= 0xFF;
   	output_cfg->invert 			= FALSE;

	return output_cfg;
convert_fail:
	if(field_smac)
		kfree(field_smac);
	if(field_dmac)
		kfree(field_dmac);
	if(field_ctag)
		kfree(field_ctag);
	if(field_ethtype)
		kfree(field_ethtype);
	if(field_scrIP)
		kfree(field_scrIP);
	if(field_dstIP)
		kfree(field_dstIP);
	if(field_ipRange)
		kfree(field_ipRange);
	if(field_tos)
		kfree(field_tos);
	if(field_ipProto)
		kfree(field_ipProto);
	if(field_portRange)
		kfree(field_portRange);
	if(field_always_match)
		kfree(field_always_match);
	if(output_cfg)
		kfree(output_cfg);
	
	return NULL;
		
}


/* Note.
	1. This function will copy the data in input_qos_rule , the caller can free input_qos_rule after calling function

	TODO : I do not do duplicate check in this fuction 
*/
int rtl867x_8367b_add_qosrule(rtl867x_hwnat_qos_rule_t *input_qos_rule ,int q_index)
{
	rtl867x_8367b_qos_rule_t*	new_qosRule = NULL;
	rtk_filter_cfg_t* 			new_qosRule_cfg = NULL;
	int priority;
	

	DBG_8367B_QOS_PRK("Enter %s\n",__func__);	

	
	/* Step1. Create a new sw ipqos rule entry  */
	new_qosRule = kmalloc(sizeof(rtl867x_8367b_qos_rule_t), GFP_ATOMIC);
	if(!new_qosRule)
	{
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		goto add_fail;	
	}
	memset(new_qosRule,0,sizeof(rtl867x_8367b_qos_rule_t));	


	/* Step2. convert input rule to 8676 acl rule */
	new_qosRule_cfg = _rtl867x_8367b_qosConvertDataForamt(input_qos_rule);
	if(!new_qosRule_cfg)
	{
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		goto add_fail;
	}	
	new_qosRule->rule_cfg = new_qosRule_cfg;			
	

	/* Step3. Setup IPQoS policy  (At the same time, if we found the same qos rule, delete older one) */
	new_qosRule->swQid	= ((q_index>=0)?	q_index: rtl8367b_priority_mapping[rtl8367b_priority_default].Qid);


	/* 	Step3.2  Get acl priority */
	priority = _rtl867x_8367b_qosPriorityMappingGet(new_qosRule->swQid);
	if(priority==-1)
	{
		DBG_8367B_QOS_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		goto add_fail;
	}	
	else
		new_qosRule->priority 	= priority;		

	
	/* Step 4. Finally, start to add acl rule*/	
	if(_rtl867x_8367b_qosAddRule(new_qosRule)!=SUCCESS)	
		goto add_fail_deref_prio;

	
	DBG_8367B_QOS_PRK("Leave %s\n",__func__);	
	return SUCCESS;

add_fail_deref_prio:
	_rtl867x_8367b_qosPriorityMappingDeRef(priority);
add_fail:
	if(new_qosRule_cfg && new_qosRule_cfg->fieldHead)
		rtl867x_rtl8367b_acl_fieldData_free(&new_qosRule_cfg->fieldHead);
	if(new_qosRule_cfg)
		kfree(new_qosRule_cfg);
	if(new_qosRule)
		kfree(new_qosRule);
	DBG_8367B_QOS_PRK("Leave %s @ %d\n",__func__,__LINE__);	
	return FAILED;
}

void rtl867x_8367b_flush_qosrule(void)
{
	rtl867x_8367b_qos_rule_t	*qosRule_entry;
	rtl867x_8367b_qos_rule_t	*qosRule_nxt;
	
	DBG_8367B_QOS_PRK(" Enter %s\n",__func__);
	

	list_for_each_entry_safe(qosRule_entry, qosRule_nxt,&rtl8367b_qosRuleHead,qos_rule_list) {		 
		_rtl867x_8367b_qosPriorityMappingDeRef(qosRule_entry->priority);
		list_del(&qosRule_entry->qos_rule_list);
		rtl867x_rtl8367b_acl_fieldData_free(&qosRule_entry->rule_cfg->fieldHead);
		kfree(qosRule_entry->rule_cfg);	
		kfree(qosRule_entry);
	}
	
	rtl867x_rtl8367b_flushacl(RL6000_ACL_CHAIN_QOS);
	_rtl867x_8367b_flush_iprange();
	_rtl867x_8367b_flush_portrange();	

}

static int __init rtl867x_8367b_qos_init(void)
{
	int i;	

	/* Init some sw_data (sw_priority_mapping , sw_qosrule .. ) */
	rtl867x_rtl8367b_register_aclchain(RL6000_ACL_CHAIN_QOS,0);
	rtl8367b_priority_default = -1;
	memset(rtl8367b_priority_mapping, 0, RTK_MAX_NUM_OF_PRIORITY*sizeof(rtl8367b_aclpriority_mapping_data_t));
	
	for(i=0;i<RTK_MAX_NUM_OF_QUEUE;i++)
	{			
		rtl8367b_hwQid2swQid_Mapping[i] = rtl8367b_Queue_NOT_Create;			
	}	

	INIT_LIST_HEAD(&rtl8367b_qosRuleHead);

	
	return SUCCESS;
}

void rtl867x_8367b_qosShowDebugInfo(void)
{
	int i;
	
	printk("Queue hwIdx <----> swIdx : ");
	for(i=0;i<RTK_MAX_NUM_OF_QUEUE;i++)
		printk("[HwQid_%d]0x%08X  ",i,rtl8367b_hwQid2swQid_Mapping[i]);
	printk("\n");

	printk("acl priority <----> \tQueue hwIdx  \tref_count\n");
	for(i=0;i<TOTAL_VLAN_PRIORITY_NUM;i++)
		printk("%d[%s] \t\t\t%d \t\t%d\n"
		,i,rtl8367b_priority_mapping[i].valid?"Y":"N"
		,rtl8367b_priority_mapping[i].Qid
		,rtl8367b_priority_mapping[i].ref_count);

	printk("def acl priority : %d\n",rtl8367b_priority_default);
}


void rtl867x_8367b_qosShowSwRule(void)
{
	rtl867x_8367b_qos_rule_t	*qosRule_entry;	
	int index = 0;

	list_for_each_entry(qosRule_entry,&rtl8367b_qosRuleHead,qos_rule_list) {	
		printk("==   Qos Rule %d == \n",index);
		
		rtl867x_rtl8367b_show_aclrule_Datafiled(qosRule_entry->rule_cfg);	
		printk("  Priority: %d	sw_Qid:0x%08X\n\n",  qosRule_entry->priority, qosRule_entry->swQid);
		index++;
	}
}


static void __exit rtl867x_8367b_qos_exit(void)
{
	rtl867x_rtl8367b_unregister_aclchain(RL6000_ACL_CHAIN_QOS);
}
module_init(rtl867x_8367b_qos_init);
module_exit(rtl867x_8367b_qos_exit);


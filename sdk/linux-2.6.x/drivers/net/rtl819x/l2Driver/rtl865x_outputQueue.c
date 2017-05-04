
/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_outputQueue.c - RTL865x Home gateway controller Layered driver API documentation       |
        This document explains the API interface of the table driver module. Functions with rtl865x prefix
        are external functions.
        @normal Hyking Liu (Hyking_liu@realsil.com.cn) <date>

        Copyright <cp>2008 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

        @head3 List of Symbols |
        Here is a list of all functions and variables in this module.
        
        @index | RTL_LAYEREDDRV_API
*/
#include <linux/config.h>
#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <common/rtl8651_tblDrvProto.h>
#include <common/rtl865x_eventMgr.h>
#include <common/rtl865x_vlan.h>
#include <net/rtl/rtl865x_netif.h>
#include <common/rtl865x_netif_local.h>
#include <net/rtl/rtl865x_outputQueue.h>
//#include "assert.h"
//#include "rtl_utils.h"
#include <common/rtl_errno.h>
#if defined (CONFIG_RTL_LOCAL_PUBLIC)
#include <l3Driver/rtl865x_localPublic.h>
#endif

#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER
#include <AsicDriver/asicRegs.h>
#include <AsicDriver/rtl865x_asicCom.h>
#include <AsicDriver/rtl865x_asicL2.h>
#else
#include <AsicDriver/asicRegs.h>
#include <AsicDriver/rtl8651_tblAsicDrv.h>
#endif

#include "../../../../net/bridge/br_private.h"



/* Warning ! If you want to create more than 4 output queues , you have to decrese Per-Queue physical length gap Register (0xBB80-4500)
     In order to let pkts enqueue into higher queues (Queue4 , 5) */
#define UPSTREAM_Quene_NUM		4


#define Queue_NOT_Create			-1


struct list_head rtl865x_qosRuleHead;


static uint8    priorityDecisionArray[] = {	1,		/* port base */
									2,		/*         802.1p base */ 
									1,		/*         dscp base */                   
									4,		/*         acl base */    
									8		/* nat base */
								};


typedef struct aclpriority_mapping_data_s
{
	uint8 valid;
	uint8 Qid;
	uint32 remark_8021p;
	uint32 remark_dscp;
	uint32 ref_count;
} aclpriority_mapping_data_t;

static aclpriority_mapping_data_t 	upstream_priority_mapping[TOTAL_VLAN_PRIORITY_NUM];
static int32 						upstream_priority_default 	=-1;
static int32 						upstream_default_swQid 		=-1;
static uint32 						upstream_hwQid2swQid_Mapping[RTL8651_OUTPUTQUEUE_SIZE] = {0};

#if defined(CONFIG_RTL_PROC_DEBUG)
static void rtl865x_show_QosAcl(rtl865x_qos_rule_t	*qosRule)
{
	rtl865x_showACL_DataField(&qosRule->acl_rule_data,qosRule->acl_rule_data_format);
	
	printk("\tPriority: %d	sw_Qid:0x%08X    Remark_8021p: 0x%08X   Remark_DSCP: 0x%08X \n"
		,  qosRule->priority, qosRule->swQid, qosRule->remark_8021p, qosRule->remark_dscp);

}
int32 rtl865x_show_allQosAcl(void)
{
	int index=0;
	rtl865x_qos_rule_t	*qosRule;
	list_for_each_entry(qosRule,&rtl865x_qosRuleHead,qos_rule_list)
	{
		printk("[%d] ",index);
		rtl865x_show_QosAcl(qosRule);
		printk("\n");
		index++;
	}	
	return SUCCESS;

}
void rtl865x_qosShowDebugInfo(void)
{
	int i;
	
	printk("Queue hwIdx <----> swIdx : ");
	for(i=0;i<RTL8651_OUTPUTQUEUE_SIZE;i++)
		printk("[HwQid_%d]0x%08X  ",i,upstream_hwQid2swQid_Mapping[i]);
	printk("\n");

	printk("acl priority <----> \tQueue hwIdx  \tRemark_802.1p \tRemark_DSCP  \tref_count\n");
	for(i=0;i<TOTAL_VLAN_PRIORITY_NUM;i++)
		printk("%d[%s] \t\t\t%d \t\t0x%X \t\t0x%X \t\t%d\n"
		,i,upstream_priority_mapping[i].valid?"Y":"N"
		,upstream_priority_mapping[i].Qid
		,upstream_priority_mapping[i].remark_8021p
		,upstream_priority_mapping[i].remark_dscp
		,upstream_priority_mapping[i].ref_count);

	printk("def acl priority : %d\n",upstream_priority_default);
	
}

#endif


/* Set 8676 switch's port egress bandwidth limit */
int32 rtl865x_qosSetBandwidth(uint32 memberPort, uint32 bps)
{

	uint32	asicBandwidth;
	uint32	port;
	DBG_OutputQueue_PRK("Enter %s (memberPort=0x%X , bps = %d)\n",__func__,memberPort,bps);

	///////////////////////////////////////////////
	/*	Egress bandwidth granularity was 64Kbps	*/
	asicBandwidth = bps>>EGRESS_BANDWIDTH_GRANULARITY_BITLEN;
	if (asicBandwidth>0 && (bps&(1<<(EGRESS_BANDWIDTH_GRANULARITY_BITLEN-1)))==1)
	{
		asicBandwidth++;
	}

//#if defined(CONFIG_RTL_PUBLIC_SSID)
	//if(strcmp(netIf->name,RTL_GW_WAN_DEVICE_NAME) == 0)
//#else
	//if(strcmp(netIf->name,RTL_DRV_WAN0_NETIF_NAME)==0)
//#endif
	//{
		//Adjust for wan port egress asic bandwidth
		asicBandwidth+=3;
	//}
	//if(strcmp(netIf->name,RTL_DRV_LAN_NETIF_NAME)==0)
	//{
		//Adjust for lan port egress asic bandwidth
		//asicBandwidth++;
	//}
	////////////////////////////////////////////////////////////////

	#if 0 /* Kevin, ignore ingress bandwidth setting */
	///////////////////////////////////////////////
	/*	Ingress bandwidth granularity was 16Kbps	*/
	wanPortAsicBandwidth=bps>>INGRESS_BANDWIDTH_GRANULARITY_BITLEN;
	if (wanPortAsicBandwidth>0 && (bps&(1<<(INGRESS_BANDWIDTH_GRANULARITY_BITLEN-1)))==1)
	{
		wanPortAsicBandwidth++;
	}

	//Adjust for wan port ingress asic bandwidth
	wanPortAsicBandwidth+=5;

	if(strcmp(netIf->name,RTL_DRV_LAN_NETIF_NAME)==0)
	{
		//To set wan port ingress asic bandwidth
#if defined(CONFIG_RTL_PUBLIC_SSID)
		wanNetIf= _rtl865x_getNetifByName(RTL_GW_WAN_DEVICE_NAME);
#else
		wanNetIf= _rtl865x_getNetifByName(RTL_DRV_WAN0_NETIF_NAME);
#endif
		if(wanNetIf != NULL)
		{
			
			wanMemberPort=rtl865x_getVlanPortMask(wanNetIf->vid);		
			DBG_OutputQueue_PRK("(%s) wanMemberPort : 0x%X \n",__func__,wanMemberPort);


			for(port=0;port<=CPU;port++)
			{
				if(((1<<port)&wanMemberPort)==0)
					continue;
				rtl8651_setAsicPortIngressBandwidth(port,wanPortAsicBandwidth);
			}
		}
	}
	////////////////////////////////////////////////////////////////
	#endif
	
	rtl865xC_lockSWCore();
	for(port=0;port<=CPU;port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;
		rtl8651_setAsicPortEgressBandwidth(port, asicBandwidth);
	}
	rtl865xC_waitForOutputQueueEmpty();
	rtl8651_resetAsicOutputQueue();
	rtl865xC_unLockSWCore();
	return SUCCESS;
}

int32 rtl865x_qosFlushBandwidth(uint32 memberPort)
{

	uint32	port;	
	DBG_OutputQueue_PRK("Enter %s(memberPort:0x%X)\n",__func__,memberPort);

	rtl865xC_lockSWCore();
	for(port=0;port<=CPU;port++)
	{
		if(((1<<port)&memberPort)==0)
			continue;
		rtl8651_setAsicPortEgressBandwidth(port, APR_MASK>>APR_OFFSET);

		//To flush wan port Ingress bandwidth limit
//		if(strcmp(netIf->name,"eth1")==0)
		{
			rtl8651_setAsicPortIngressBandwidth(port,0);
		}
	}


	
	rtl865xC_waitForOutputQueueEmpty();
	rtl8651_resetAsicOutputQueue();
	rtl865xC_unLockSWCore();
	return SUCCESS;
}

/* Get hw_Qid */
static int32 _rtl865x_qosQIDMappingGet( uint32 sw_Qidx)
{
	int	i;

	for(i=0; i < RTL8651_OUTPUTQUEUE_SIZE; i++)
	{
		if (upstream_hwQid2swQid_Mapping[i] == sw_Qidx)
		{
			return i;
		}
	}

	return -1;
}


static int _rtl865x_qosQIDMappingSet(int32 Qidmapping[RTL8651_OUTPUTQUEUE_SIZE])
{		
	memcpy(upstream_hwQid2swQid_Mapping,Qidmapping,RTL8651_OUTPUTQUEUE_SIZE*sizeof(int));	
	return SUCCESS;
}


static void _rtl865x_qosQIDMappingClear(void)
{
	
	int i;		
	for(i=0;i<RTL8651_OUTPUTQUEUE_SIZE;i++)
		upstream_hwQid2swQid_Mapping[i] = Queue_NOT_Create;	
	
}

/* Note. it need not to lock 0412 switch core before use this funcion */
static int _rtl865x_qosPriorityMappingTakeEffect(void)
{	
	int32  i;	

	/* 1. set priotiy  <---> queue_id  */		
	for(i=0;i<TOTAL_VLAN_PRIORITY_NUM;i++)		
	{		
		if(rtl8651_setAsicPriorityToQIDMappingTable(UPSTREAM_Quene_NUM, i, upstream_priority_mapping[i].Qid)==FAILED)
		{
			DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
			return FAILED;
		}			
	}	

	
	/* 2. set remark info */
	for(i=0;i<TOTAL_VLAN_PRIORITY_NUM;i++)		
	{
		if(rtl8651_SetPortRemark(i,upstream_priority_mapping[i].remark_8021p,upstream_priority_mapping[i].remark_dscp)==FAILED)
		{
			DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
			return FAILED;
		}
	}

	return SUCCESS;
	
}


void rtl865x_qosPriorityMappingDeRef(uint32 priority)	
{

	#ifdef DBG_OutputQueue
	DBG_OutputQueue_PRK("Enter %s(priority:%d)\n",__func__,priority);	
	DBG_OutputQueue_PRK("--  dump stack --\n");
	dump_stack();
	DBG_OutputQueue_PRK("------------------------ --\n");
	#endif
	

	if(priority<0 || priority>=TOTAL_VLAN_PRIORITY_NUM)
	{
		DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return;	
	}
		
	upstream_priority_mapping[priority].ref_count--;
	if(upstream_priority_mapping[priority].ref_count==0)
	{
		memset(&upstream_priority_mapping[priority], 0,sizeof(aclpriority_mapping_data_t));
		if(_rtl865x_qosPriorityMappingTakeEffect()!=SUCCESS)
		{
			printk("%s@ %d(return FAILED)\n",__func__,__LINE__);		
			DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);		
		}
	}		

}

/* return ...     -1:   no avaliable acl priority mapping   others: acl priority 
	Note. At the same time ,ref_count ++ */
int rtl865x_qosPriorityMappingGet(uint32 sw_Qidx,uint32 remark_8021p,uint32 remark_dscp)
{
	int32  i,empty_idx,hw_queue_idx;	

	DBG_OutputQueue_PRK("Enter %s (sw_Qidx:%d   remark_8021p:%d  remark_dscp:0x%X )\n"
		,__func__,sw_Qidx,remark_8021p,remark_dscp); 
	
	hw_queue_idx = _rtl865x_qosQIDMappingGet(sw_Qidx);
	if(hw_queue_idx==-1)
	{
		DBG_OutputQueue_PRK("Leave %s @ %d\n",__func__,__LINE__);	
		return -1;	
	}
	
	DBG_OutputQueue_PRK("(%s)  hw_queue_idx=%d \n",__func__,hw_queue_idx);	
	

	for(i=0;i<TOTAL_VLAN_PRIORITY_NUM;i++)
	{
		if( upstream_priority_mapping[i].valid 
			&& upstream_priority_mapping[i].Qid 			== hw_queue_idx
			&& upstream_priority_mapping[i].remark_8021p	== remark_8021p
			&& upstream_priority_mapping[i].remark_dscp	== remark_dscp)
		{

			upstream_priority_mapping[i].ref_count++;			
			DBG_OutputQueue_PRK("Leave %s (return existed priority %d, ref:%d)\n",__func__,i,upstream_priority_mapping[i].ref_count);
			return i;
		}
	}

	/* cannot find out matching entry, create the new one*/
	empty_idx = -1;
	for(i=0;i<TOTAL_VLAN_PRIORITY_NUM;i++)
	{
		if(upstream_priority_mapping[i].valid==0)
		{
			empty_idx = i;
			break;
		}
	}
	if(empty_idx==-1)
	{
		DBG_OutputQueue_PRK("Leave %s (return %d)\n",__func__,-1);
		return -1; /* no empty entry */
	}

	upstream_priority_mapping[empty_idx].valid 			= 1	;
	upstream_priority_mapping[empty_idx].Qid 			= hw_queue_idx;
	upstream_priority_mapping[empty_idx].remark_8021p 	= remark_8021p;
	upstream_priority_mapping[empty_idx].remark_dscp 	= remark_dscp;
	upstream_priority_mapping[empty_idx].ref_count		= 1	;


	/*  let new acl_priority mapping take effect */
	/*  mapping between priority and Qid need not lock hw. (change queue number or bandwith need it ! ) */
	if(_rtl865x_qosPriorityMappingTakeEffect()!=SUCCESS)
	{
		DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;	
	}	
	DBG_OutputQueue_PRK("Leave %s (return new priority %d, ref:%d)\n",__func__,i,upstream_priority_mapping[i].ref_count);
	return empty_idx;
	
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

static void _rtl865x_qosArrangeQueue(int ceil[],int rate[],uint32 bands_num)
{	
	int	i; 	

	#ifdef DBG_OutputQueue
	DBG_OutputQueue_PRK("Enter %s\n",__func__);		
	DBG_OutputQueue_PRK("(%s)------------Before--------------------\n",__func__);
	for(i=0; i<bands_num; i++)
		DBG_OutputQueue_PRK("(%s) [Queue %d] bandwidth:%d(bits)  ceil:%d\n",__func__,i,ceil[i],rate[i]);		
	DBG_OutputQueue_PRK("-------------------------------------------------------------\n");
	#endif


	
	/* bandwidth
		only cna be set as 0~255 ,  if the maxBandwidth>255 ..... 

		 ex. bandwidth_0 = 3  , bandwidth_1 = 254 , bandwidth_2 = 257
		 we shift right these value until  the maxBandwidth/divisor <= 255   */
	
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
		if(maxBandwidth<EGRESS_WFQ_MAX_RATIO)
			break;

		/* start right shift .. */
		for(i=0; i<bands_num; i++)				
			rate[i] = rate[i]>>1 ;		
		
	}while(1);	

	#ifdef DBG_OutputQueue	
	DBG_OutputQueue_PRK("(%s)------------After--------------------\n",__func__);
	for(i=0; i<bands_num; i++)
		DBG_OutputQueue_PRK("(%s) [Queue %d] bandwidth:%d(bits) ceil:%d\n",__func__,i,ceil[i],rate[i]);		
	DBG_OutputQueue_PRK("-------------------------------------------------------------\n");
	DBG_OutputQueue_PRK("Leave %s\n",__func__);
	#endif

	
}

static int _rtl865x_qos_remark(int enable_8021p,int enable_dscp)
{
	int ipqos_8676_port_mask=0;
	int port_idx;

#ifdef CONFIG_RTL_8367B
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B) 
		ipqos_8676_port_mask = 0x1; /* p0 router mode => port 0*/
	else
#endif
	{
		int i;
		for(i=PHY0;i<=CPU;i++)
		{			
			if(((1<<i)&RTL_WANPORT_MASK)==0)
				continue;
			ipqos_8676_port_mask |= (1<<i);			
		}
	}

	for(port_idx=PHY0;port_idx<=CPU;port_idx++)
	{
		if( (1<<port_idx)&ipqos_8676_port_mask )
		{
			if(enable_8021p)
				rtl8651_EnablePortRemark_8021p(port_idx);
			else
				rtl8651_DisablePortRemark_8021p(port_idx);

			if(enable_dscp)
				rtl8651_EnablePortRemark_dscp(port_idx);
			else
				rtl8651_DisablePortRemark_dscp(port_idx);
		}
	}

	return SUCCESS;		
}

static int _rtl865x_qos_setQueueInfo(int sp_queue_num, int wrr_queue_num,int queue_number,int ceil[],int rate[])
{

	int ipqos_8676_port_mask=0;
	int port_idx,queue_idx;

	if(sp_queue_num+wrr_queue_num >queue_number)
		return FAILED;
		
	/*   Start  to set Queue scheduling ..  
		8676 embeded switch 	: set queue's number /  set queue's scheduling info / enable remark

		ex. if sp_queue_num=1 , wrr_queue_num=2   queue_number=4

			hw_qid : 0   1   2   3   4   5 
			sw_qid:  1   2   -1   0  -1   -1

			we only use the first 4 hw Queues 
			set hw_Queue 0 & 1 as WRR type
			set the remainging hw_Queue 2 & 3 as SP type (we will not use hw_Queue 2)
	*/		

#ifdef CONFIG_RTL_8367B
	if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B) 
		ipqos_8676_port_mask = 0x1; /* p0 router mode => port 0*/
	else
#endif
	{
		int i;
		for(i=PHY0;i<=CPU;i++)
		{			
			if(((1<<i)&RTL_WANPORT_MASK)==0)
				continue;
			ipqos_8676_port_mask |= (1<<i);			
		}
	}
	DBG_OutputQueue_PRK("(%s) ipqos_8676_port_mask : 0x%X \n",__func__,ipqos_8676_port_mask);

	rtl865xC_lockSWCore(); 
	for(port_idx=PHY0;port_idx<=CPU;port_idx++)
	{
		if( (1<<port_idx)&ipqos_8676_port_mask )
		{
			/* 1.1  set queue's number  */				
			rtl8651_setAsicOutputQueueNumber(port_idx, queue_number);					

			/* 1.2  set queue's scheduling info  */
			for (queue_idx=0;queue_idx<queue_number;queue_idx++)
			{
				if(queue_idx < wrr_queue_num) /* wrr */
				{
					int asicBandwidth;					
					/*	Egress bandwidth granularity was 64Kbps	*/
					asicBandwidth = ((ceil[queue_idx])>>(EGRESS_BANDWIDTH_GRANULARITY_BITLEN)) - 1;
					if ((ceil[queue_idx])&(1<<(EGRESS_BANDWIDTH_GRANULARITY_BITLEN-1)))
						asicBandwidth += 1;	
				
					/*  HTB's ceil==>rtl8651_setAsicQueueRate ,    HTB's rate==>rtl8651_setAsicQueueWeight  */		
					rtl8651_setAsicQueueRate(port_idx, queue_idx, 
						0>>PPR_OFFSET, 
						L1_MASK>>L1_OFFSET, 
						asicBandwidth);			
					rtl8651_setAsicQueueWeight(port_idx, queue_idx, WFQ_PRIO, rate[queue_idx]-1);					
				}
				else /* sp */
				{					
					rtl8651_setAsicQueueRate(port_idx, queue_idx, 
						PPR_MASK>>PPR_OFFSET, 
						L1_MASK>>L1_OFFSET, 
						APR_MASK>>APR_OFFSET);
					rtl8651_setAsicQueueWeight(port_idx, queue_idx, STR_PRIO, 0);					
				}				
			}	
		}		
	}
	rtl865xC_waitForOutputQueueEmpty();
	rtl8651_resetAsicOutputQueue();
	rtl865xC_unLockSWCore();
	return SUCCESS;
}



int32 rtl865x_enableQos(int sp_queue_num, int wrr_queue_num, int ceil[],int rate[],int32 default_sw_qid)
{

	int total_queue_num = sp_queue_num + wrr_queue_num;
	int Qidmapping[RTL8651_OUTPUTQUEUE_SIZE];
	int i;

	DBG_OutputQueue_PRK("Enter %s (sp_queue_num:%d   wrr_queue_num:%d   default_sw_qid:%d)\n"
		,__func__,sp_queue_num,wrr_queue_num,default_sw_qid);

	/* 1. check queue number */
	if (sp_queue_num<0 || wrr_queue_num<0 )
	{
		DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED) \n",__func__,__LINE__);
		return FAILED;	
	}
	
	if( total_queue_num<1 || total_queue_num>UPSTREAM_Quene_NUM)
	{
		printk("(%s)Warning!! 0412 switch only support 1~%d outputqueues in upstream (Your input is %d) \n"
			,__func__,UPSTREAM_Quene_NUM,total_queue_num);
		return FAILED;	
	}
	
	if (default_sw_qid<0 || default_sw_qid>=total_queue_num)
	{
		DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED) \n",__func__,__LINE__);
		return FAILED;	
	}

	/* 2.  Re-adjust qosinfo's para  */
	if(wrr_queue_num>0)
	{
		if(ceil==NULL || rate==NULL)
		{
			DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED) \n",__func__,__LINE__);
		return FAILED;	
		}

		_rtl865x_qosArrangeQueue(ceil,rate,wrr_queue_num);
	}


	/* 3.  set QueueID mapping
		ex. if we create 4 queues (2 is priority , 2 is wrr )
			sw_qid ==>	0 : the highest priority 
						1 : the second priority 
						2 : 1st wrr queue
						3 : 2nd wrr queue
			
			hw_qid : 0   1   2   3   4   5 
			sw_qid:  2   3    1   0  -1   -1

		ex. if we create 3 queues (1 is priority , 2 is wrr )
			sw_qid ==>	0 : the highest priority 					
						1 : 1st wrr queue
						2 : 2nd wrr queue
			
			hw_qid : 0   1   2   3   4   5 
			sw_qid:  1   2   -1   0  -1   -1
	*/	
	for(i=0;i<RTL8651_OUTPUTQUEUE_SIZE;i++)
		Qidmapping[i] = Queue_NOT_Create;

	for(i=0;i<sp_queue_num;i++)		
		Qidmapping[UPSTREAM_Quene_NUM-1-i] = i;
	
	for(i=0;i<wrr_queue_num;i++)	
		Qidmapping[i] = sp_queue_num+i;
	
	if(_rtl865x_qosQIDMappingSet(Qidmapping)!=SUCCESS)
	{			
		DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}

	/*  4. Start  to set Queue scheduling ..  */
	if(_rtl865x_qos_setQueueInfo(sp_queue_num,wrr_queue_num,UPSTREAM_Quene_NUM,ceil,rate)!=SUCCESS)
	{
		DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}

	/*  5. Set 802.1p/DSCP remarking by default */
	if(_rtl865x_qos_remark(1,1)!=SUCCESS)
	{
		DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}

	/*  6. set default acl priority mapping  */
	upstream_default_swQid = default_sw_qid;
	upstream_priority_default = rtl865x_qosPriorityMappingGet(default_sw_qid,0,0);
	if(upstream_priority_default==-1)
	{			
		DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}

	/*  7. init priority setting except fot ACL based in hw 
		8676 :	(1) 802.1p based (for 802.1p rules)
				(2) port based	(for default priority)
	*/	
	rtl8651_flushAsicDot1qAbsolutelyPriority();	
	for(i=0; i<RTL8651_PORT_NUMBER + 3; i++)
		rtl8651_setAsicPortDefaultPriority(i,upstream_priority_default);

	
	/*  8.  set  priority  between different qos based 
		8676 	: NAPT-based > ACL-based >802.1p based >port based
	*/
	rtl8651_setAsicPriorityDecision(priorityDecisionArray[PORT_BASE], 
	priorityDecisionArray[D1P_BASE], priorityDecisionArray[DSCP_BASE], 
	priorityDecisionArray[ACL_BASE], priorityDecisionArray[NAT_BASE]);	

	DBG_OutputQueue_PRK("Leave %s\n",__func__);
	

	return SUCCESS;	
}


/* Create a complete 8676 ACL Rule from 8676 SW QoS Rule */
static rtl865x_AclRule_t* __rtl865x_qosCreateACLrule(rtl865x_qos_rule_t* input_rule)
{
	rtl865x_AclRule_t*	AclRule;	
	AclRule = kmalloc(sizeof(rtl865x_AclRule_t), GFP_ATOMIC);
	if(!AclRule)
		return NULL;
	
	memset(AclRule,0,sizeof(rtl865x_AclRule_t));	
	AclRule->actionType_ 	= RTL865X_ACL_PRIORITY;
	AclRule->pktOpApp_  = RTL865X_ACL_ALL_LAYER;
	AclRule->direction_ 	= RTL865X_ACL_INGRESS; 
	AclRule->priority_ 	= input_rule->priority;
	AclRule->ruleType_	= input_rule->acl_rule_data_format;
	memcpy(&AclRule->un_ty,&input_rule->acl_rule_data,sizeof(rtl865x_AclRuleData_t));

	return AclRule;
}

/* It inspects acl rule
	(NOT including  q_index and reamaking value ) 
*/
static int8 __rtl865x_sameQosRule(rtl865x_qos_rule_t *rule1, rtl865x_qos_rule_t *rule2)
{
	if(rtl865x_sameAclRuleDataField(&rule1->acl_rule_data,&rule2->acl_rule_data,rule1->acl_rule_data_format,rule2->acl_rule_data_format)==FALSE)		
		return FALSE;	
	else
	{
		if((rule1->priority != rule2->priority) || (rule1->swQid != rule2->swQid) || (rule1->remark_8021p != rule2->remark_8021p) || (rule1->remark_dscp != rule2->remark_dscp))		
			return FALSE;		
		else
			return TRUE;
	}		
}



/*  	
	FUNC 	: _rtl865x_qosAddRule()
	USAGE	: Add a new sw_qos entry into sw_qos_list , and add asic acl rule 
		
	Note.  
		1. It is caller's respnsibility to promise that all fileds  (except for linking list) in input_rule has been assigned correctly
		2. You have to get priority first. (Ref priority)	
*/

static int _rtl865x_qosAddRule(rtl865x_qos_rule_t* input_rule)
{
	rtl865x_qos_rule_t* rule_entry;	
	int rtn=0;

	/* dupliate check */
	list_for_each_entry(rule_entry,&rtl865x_qosRuleHead,qos_rule_list)
	{       
	    if(__rtl865x_sameQosRule(rule_entry,input_rule)==TRUE)
	    	return FAILED;	    
	}		

	if(input_rule->acl_rule_data_format!=RTL865X_ACL_802D1P)				
	{
		rtl865x_AclRule_t*	aclRule;
		aclRule = __rtl865x_qosCreateACLrule(input_rule);
		if(!aclRule)
			return FAILED;

		if(input_rule->outIfname[0]!='\0')											
			rtn=rtl865x_add_acl(aclRule, input_rule->outIfname, RTL865X_ACL_QOS_USED0,1,1);
		else
			rtn=rtl865x_add_acl(aclRule, NULL, RTL865X_ACL_QOS_USED0,1,1);
		if(rtn!=SUCCESS)	
		{
			DBG_QosRule_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
			kfree(aclRule);
			return FAILED;
		}
		kfree(aclRule); /* rtl865x_add_acl() will memcpy the rule */
	}
	else	
	{
		if(rtl8651_setAsicDot1qAbsolutelyPriority(input_rule->acl_rule_data.VLANTAG.vlanTagPri, input_rule->priority)!=SUCCESS)
		{
			DBG_QosRule_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
			return FAILED;
		}
	}

	INIT_LIST_HEAD(&input_rule->qos_rule_list);
	list_add_tail(&input_rule->qos_rule_list , &rtl865x_qosRuleHead);
	
	return SUCCESS;
}
/*  	
	FUNC 	: _rtl865x_qosDelRule()
	USAGE	: Remove some specific sw_qos entry from sw_qos_list , and delete asic acl rule 
		
	Note. I will help you deref the prioity
*/
static void _rtl865x_qosDelRule(rtl865x_qos_rule_t* input_rule)
{		

	#ifdef DBG_QosRule
	DBG_QosRule_PRK("Enter %s\n",__func__);	
	DBG_QosRule_PRK("-------    deleted rule----------\n");
	rtl865x_show_QosAcl(input_rule);
	DBG_QosRule_PRK("----------------------------------\n");
	#endif	

	if(input_rule->acl_rule_data_format!=RTL865X_ACL_802D1P)	
	{
		rtl865x_AclRule_t*	aclRule;
		aclRule = __rtl865x_qosCreateACLrule(input_rule);
		if(!aclRule)
			return;
		rtl865x_del_acl(aclRule, RTL_DRV_LAN_NETIF_NAME, RTL865X_ACL_QOS_USED0);
		kfree(aclRule);
	}
	else
		rtl8651_setAsicDot1qAbsolutelyPriority(input_rule->acl_rule_data.VLANTAG.vlanTagPri, 0);

	rtl865x_qosPriorityMappingDeRef(input_rule->priority);
	list_del(&input_rule->qos_rule_list);
	kfree(input_rule);

	DBG_QosRule_PRK("Leave %s\n",__func__);	
}

/* Convert RTL867x IPQoS Format to 6239 acl format 
	Note. It only write un_ty and ruleType_
*/
static int _rtl865x_qosConvertDataForamt
	(rtl867x_hwnat_qos_rule_t* input_rule,rtl865x_AclRuleData_t* output_ruleData, int* output_ruleDataFormat)
{

	switch(input_rule->rule_type)
	{
		case RTL867x_IPQos_Format_Ethernet:
			*output_ruleDataFormat	= RTL865X_ACL_MAC;
			break;
		case RTL867x_IPQos_Format_IP:
			*output_ruleDataFormat	= RTL865X_ACL_IP;
			break;
		case RTL867x_IPQos_Format_IP_Range:
			*output_ruleDataFormat	= RTL865X_ACL_IP_RANGE;
			break;
		case RTL867x_IPQos_Format_TCP:
			*output_ruleDataFormat	= RTL865X_ACL_TCP;
			break;
		case RTL867x_IPQos_Format_TCP_Range:
			*output_ruleDataFormat	= RTL865X_ACL_TCP_IPRANGE;
			break;
		case RTL867x_IPQos_Format_UDP:
			*output_ruleDataFormat	= RTL865X_ACL_UDP;
			break;
		case RTL867x_IPQos_Format_UDP_Range:
			*output_ruleDataFormat	= RTL865X_ACL_UDP_IPRANGE;
			break;
		case RTL867x_IPQos_Format_8021p:
			*output_ruleDataFormat	= RTL865X_ACL_802D1P;
			break;
		case RTL867x_IPQos_Format_srcPort_setMask:
		case RTL867x_IPQos_Format_srcPort:
		case RTL867x_IPQos_Format_srcFilter:
			*output_ruleDataFormat	= RTL865X_ACL_SRCFILTER;
			break;
		case RTL867x_IPQos_Format_srcFilter_Range:
			*output_ruleDataFormat	= RTL865X_ACL_SRCFILTER_IPRANGE;
			break;
		default: /* unknow type or not supported */
			DBG_QosRule_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
			return FAILED;
	}


	switch(input_rule->rule_type)
	{
		case RTL867x_IPQos_Format_Ethernet:			
			output_ruleData->MAC._dstMac		= input_rule->match_field.L2._dstMac;
			output_ruleData->MAC._dstMacMask	= input_rule->match_field.L2._dstMacMask;
			output_ruleData->MAC._srcMac			= input_rule->match_field.L2._srcMac;
			output_ruleData->MAC._srcMacMask	= input_rule->match_field.L2._srcMacMask;
			output_ruleData->MAC._typeLen		= input_rule->match_field.L2._ethType;
			output_ruleData->MAC._typeLenMask	= input_rule->match_field.L2._ethTypeMask;
			break;


		case RTL867x_IPQos_Format_TCP:
		case RTL867x_IPQos_Format_TCP_Range:
			if(input_rule->match_field.L4._sport_start==0
				&& input_rule->match_field.L4._sport_end==0)
			{
				output_ruleData->L3L4.is.tcp._srcPortUpperBound = 0xFFFF;
				output_ruleData->L3L4.is.tcp._srcPortLowerBound = 0x0000;
			}
			else
			{
				output_ruleData->L3L4.is.tcp._srcPortUpperBound = input_rule->match_field.L4._sport_end;
				output_ruleData->L3L4.is.tcp._srcPortLowerBound = input_rule->match_field.L4._sport_start;
			}
			
			if(input_rule->match_field.L4._dport_start==0
				&& input_rule->match_field.L4._dport_end==0)
			{
				output_ruleData->L3L4.is.tcp._dstPortUpperBound = 0xFFFF;
				output_ruleData->L3L4.is.tcp._dstPortLowerBound = 0x0000;
			}
			else
			{
				output_ruleData->L3L4.is.tcp._dstPortUpperBound = input_rule->match_field.L4._dport_end;
				output_ruleData->L3L4.is.tcp._dstPortLowerBound = input_rule->match_field.L4._dport_start;
			}
			goto finish_port;
		case RTL867x_IPQos_Format_UDP:			
		case RTL867x_IPQos_Format_UDP_Range:
			if(input_rule->match_field.L4._sport_start==0
				&& input_rule->match_field.L4._sport_end==0)
			{
				output_ruleData->L3L4.is.udp._srcPortUpperBound = 0xFFFF;
				output_ruleData->L3L4.is.udp._srcPortLowerBound = 0x0000;
			}
			else
			{
				output_ruleData->L3L4.is.udp._srcPortUpperBound = input_rule->match_field.L4._sport_end;
				output_ruleData->L3L4.is.udp._srcPortLowerBound = input_rule->match_field.L4._sport_start;
			}
			
			if(input_rule->match_field.L4._dport_start==0
				&& input_rule->match_field.L4._dport_end==0)
			{
				output_ruleData->L3L4.is.udp._dstPortUpperBound = 0xFFFF;
				output_ruleData->L3L4.is.udp._dstPortLowerBound = 0x0000;
			}
			else
			{
				output_ruleData->L3L4.is.udp._dstPortUpperBound = input_rule->match_field.L4._dport_end;
				output_ruleData->L3L4.is.udp._dstPortLowerBound = input_rule->match_field.L4._dport_start;
			}	
			goto finish_port;
		case RTL867x_IPQos_Format_IP:
		case RTL867x_IPQos_Format_IP_Range:
		finish_port:			
		if(input_rule->rule_type == RTL867x_IPQos_Format_IP 
			|| input_rule->rule_type == RTL867x_IPQos_Format_TCP
			|| input_rule->rule_type == RTL867x_IPQos_Format_UDP)
		{
			output_ruleData->L3L4._srcIpAddr		= input_rule->match_field.L3.ip.mask._sip;
			output_ruleData->L3L4._srcIpAddrMask	= input_rule->match_field.L3.ip.mask._sipMask;
			output_ruleData->L3L4._dstIpAddr		= input_rule->match_field.L3.ip.mask._dip;
			output_ruleData->L3L4._dstIpAddrMask	= input_rule->match_field.L3.ip.mask._dipMask;
		}
		else //RTL867x_IPQos_Format_IP_Range , RTL867x_IPQos_Format_TCP_RANGE , RTL867x_IPQos_Format_UDP_Range
		{
			/*Hyking:Asic use Addr to srore Upper address
				and use Mask to store Lower address */
			if(input_rule->match_field.L3.ip.range._sip_start==0
				&& input_rule->match_field.L3.ip.range._sip_end==0)
			{
				output_ruleData->L3L4._srcIpAddr		= 0xFFFFFFFF;
				output_ruleData->L3L4._srcIpAddrMask	= 0x00000000;
			}
			else
			{
				output_ruleData->L3L4._srcIpAddr		= input_rule->match_field.L3.ip.range._sip_end;
				output_ruleData->L3L4._srcIpAddrMask	= input_rule->match_field.L3.ip.range._sip_start;
			}

			if(input_rule->match_field.L3.ip.range._dip_start==0
				&& input_rule->match_field.L3.ip.range._dip_end==0)
			{
				output_ruleData->L3L4._dstIpAddr		= 0xFFFFFFFF;
				output_ruleData->L3L4._dstIpAddrMask	= 0x00000000;
			}
			else
			{
				output_ruleData->L3L4._dstIpAddr		= input_rule->match_field.L3.ip.range._dip_end;
				output_ruleData->L3L4._dstIpAddrMask	= input_rule->match_field.L3.ip.range._dip_start;
			}
		}	

			output_ruleData->L3L4._tos			= input_rule->match_field.L3._tos;
			output_ruleData->L3L4._tosMask		= input_rule->match_field.L3._tosMask;	
		
		if(input_rule->rule_type == RTL867x_IPQos_Format_IP 
			|| input_rule->rule_type == RTL867x_IPQos_Format_IP_Range)
		{
			output_ruleData->L3L4.is.ip._proto		= input_rule->match_field.L3._ipProto;
			output_ruleData->L3L4.is.ip._protoMask	= input_rule->match_field.L3._ipProtoMask;
		}
			break;

		case RTL867x_IPQos_Format_8021p:
			output_ruleData->VLANTAG.vlanTagPri = input_rule->match_field.L2._8021p;
			break;

		case RTL867x_IPQos_Format_srcPort:
		{	
			struct net_device *dev = re865x_get_netdev_by_name(input_rule->match_field.PHY._lan_netifname);
			DBG_QosRule_PRK("(%s) %s\n",__func__,input_rule->match_field.PHY._lan_netifname);
			
			/*  check whether this interface is  under br0 */
			if(dev)
			{
				struct net_bridge_port *br_port = dev->br_port;
				if(br_port && !strcmp(dev->br_port->br->dev->name,RTL_DRV_LAN_NETIF_NAME))
				{
					output_ruleData->SRCFILTER._srcPort = ((struct dev_priv *)dev->priv)->portmask;  
					output_ruleData->SRCFILTER._srcPortUpperBound	= 0xFFFF;
					output_ruleData->SRCFILTER._srcPortLowerBound	= 0x0000;
					output_ruleData->SRCFILTER._ignoreL4=1;
				}
				else	
				{
					DBG_QosRule_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
					return FAILED;
				}		
			}
			else		
			{
				DBG_QosRule_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
				return FAILED;
			}
			
			break;
		}

		case RTL867x_IPQos_Format_srcFilter:	
		case RTL867x_IPQos_Format_srcFilter_Range:
		{
			output_ruleData->SRCFILTER._srcMac				= input_rule->match_field.L2._srcMac;
			output_ruleData->SRCFILTER._srcMacMask			= input_rule->match_field.L2._srcMacMask;
			output_ruleData->SRCFILTER._srcVlanIdx				= input_rule->match_field.L2._vlanid;
			output_ruleData->SRCFILTER._srcVlanIdxMask			= input_rule->match_field.L2._vlanidMask;

			if(input_rule->rule_type == RTL867x_IPQos_Format_srcFilter)
			{
				output_ruleData->SRCFILTER._srcIpAddr			= input_rule->match_field.L3.ip.mask._sip;
				output_ruleData->SRCFILTER._srcIpAddrMask		= input_rule->match_field.L3.ip.mask._sipMask;
			}
			else //RTL867x_IPQos_Format_srcFilter_Range
			{
				if(input_rule->match_field.L3.ip.range._sip_start==0
					&& input_rule->match_field.L3.ip.range._sip_end==0)
				{
					output_ruleData->SRCFILTER._srcIpAddr			= 0xFFFFFFFF;
					output_ruleData->SRCFILTER._srcIpAddrMask		= 0x00000000;
				}
				else
				{
					output_ruleData->SRCFILTER._srcIpAddr			= input_rule->match_field.L3.ip.range._sip_end;
					output_ruleData->SRCFILTER._srcIpAddrMask		= input_rule->match_field.L3.ip.range._sip_start;
				}
			}

			if(input_rule->match_field.L4._sport_start==0
				&& input_rule->match_field.L4._sport_end==0)
			{
				output_ruleData->SRCFILTER._srcPortUpperBound	= 0xFFFF;
				output_ruleData->SRCFILTER._srcPortLowerBound	= 0x0000;
			}
			else
			{
				output_ruleData->SRCFILTER._srcPortUpperBound	= input_rule->match_field.L4._sport_end;
				output_ruleData->SRCFILTER._srcPortLowerBound	= input_rule->match_field.L4._sport_start;
			}

			if(input_rule->match_field.L4._sport_start==0 && input_rule->match_field.L4._sport_end==0
				&& input_rule->match_field.L3.ip.mask._sip ==0 && input_rule->match_field.L3.ip.mask._sipMask ==0
				&& input_rule->match_field.L3.ip.range._sip_end ==0 && input_rule->match_field.L3.ip.range._sip_start ==0)
				output_ruleData->SRCFILTER._ignoreL3L4 = 1;

			else if(input_rule->match_field.L4._sport_start==0 && input_rule->match_field.L4._sport_end==0)
				output_ruleData->SRCFILTER._ignoreL4 = 1;

			break;	
		}
		case RTL867x_IPQos_Format_srcPort_setMask:
		{	
			output_ruleData->SRCFILTER._srcPort = input_rule->match_field.PHY._lan_portmask;  
			output_ruleData->SRCFILTER._srcPortUpperBound	= 0xFFFF;
			output_ruleData->SRCFILTER._srcPortLowerBound	= 0x0000;		
			output_ruleData->SRCFILTER._ignoreL4=1;
			break;
		}	
		default: // unknow type
			DBG_QosRule_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
			return FAILED;
	}	
	return SUCCESS;
}
	

/* Note.
	1. if q_index , remark_8021p , remark_dscp <0 :	it means that ...
		(1) using the default setting (q_index = default_qid  ,  remark = 0 ) if it is a new qos rule
		(2) keep the original setting if there exists the same qos rule
	2. This function will copy the data in input_qos_rule , the caller can free input_qos_rule after calling function
*/
int32 rtl865x_qosAddRule(rtl867x_hwnat_qos_rule_t *input_qos_rule ,int q_index ,int remark_8021p ,int remark_dscp)
{	
	rtl865x_qos_rule_t*	qosRule_entry;
	rtl865x_qos_rule_t*	qosRule_same = NULL ;
	rtl865x_qos_rule_t*	new_qosRule = NULL;
	int priority;

	DBG_QosRule_PRK("Enter %s\n",__func__);	


	/* Step1. Create a new sw ipqos rule entry  */
	new_qosRule = kmalloc(sizeof(rtl865x_qos_rule_t), GFP_ATOMIC);
	if(!new_qosRule)
		goto add_fail;
	memset(new_qosRule,0,sizeof(rtl865x_qos_rule_t));	

	
	/* Step2. convert input rule to 8676 acl rule */
	if(_rtl865x_qosConvertDataForamt(input_qos_rule,&new_qosRule->acl_rule_data,&new_qosRule->acl_rule_data_format)!=SUCCESS)
	{
		DBG_QosRule_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		goto add_fail;
	}	
	
	#ifdef DBG_QosRule
	DBG_QosRule_PRK("(%s)----------- After converting-----------------\n",__func__);
	rtl865x_showACL_DataField(&new_qosRule->acl_rule_data,new_qosRule->acl_rule_data_format);
	#endif	


	/* Step3. Setup IPQoS policy  (At the same time, if we found the same qos rule, delete older one) */	
	
	/* 	Step3.1  Qid , 802.1p / DSCP remark 
			
			if user input q_index , remark_8021p , remark_dscp <0 :	it means that ...
			(1) using the default setting (q_index = default_qid  ,  remark = 0 ) if it is a new qos rule
			(2) keep the original setting if there exists the same qos rule
	*/	
	list_for_each_entry(qosRule_entry,&rtl865x_qosRuleHead,qos_rule_list)
	{		
		if(rtl865x_sameAclRuleDataField(&new_qosRule->acl_rule_data,&qosRule_entry->acl_rule_data
										,new_qosRule->acl_rule_data_format,qosRule_entry->acl_rule_data_format)==TRUE)
		{
			qosRule_same = qosRule_entry;
			break;
		}
	}	

	if(qosRule_same) /* there exists the same qos rule */
	{
		DBG_QosRule_PRK("(%s %d)there exists the same qos rule \n",__func__,__LINE__);	

		/* Update Qid , 802.1p / DSCP remark  */
		new_qosRule->swQid 			= ((q_index>=0)?			q_index: 		qosRule_same->swQid);
		new_qosRule->remark_8021p 	= ((remark_8021p>=0)?	remark_8021p: 	qosRule_same->remark_8021p);
		new_qosRule->remark_dscp 	= ((remark_dscp>=0)?		remark_dscp: 	qosRule_same->remark_dscp);

		/* Delete the older rule */
		_rtl865x_qosDelRule(qosRule_same);		
	
	}
	else /* new qos rule */
	{
		/* New  Qid , 802.1p / DSCP remark  */
		new_qosRule->swQid			= ((q_index>=0)?			q_index: 		upstream_default_swQid);
		new_qosRule->remark_8021p	= ((remark_8021p>=0)?	remark_8021p: 	0);
		new_qosRule->remark_dscp		= ((remark_dscp>=0)?		remark_dscp: 	0);
		
	}

	/* 	Step3.2  Get acl priority */
	priority = rtl865x_qosPriorityMappingGet(new_qosRule->swQid,new_qosRule->remark_8021p,new_qosRule->remark_dscp);
	if(priority==-1)
	{
		DBG_QosRule_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		goto add_fail;
	}	
	else
		new_qosRule->priority 	= priority;		
	memcpy(new_qosRule->outIfname,input_qos_rule->outIfname,sizeof(input_qos_rule->outIfname));
	/* Step 4. Finally, start to add acl rule*/	
	if(_rtl865x_qosAddRule(new_qosRule)!=SUCCESS)	
		goto add_fail_deref_prio;

	
	DBG_QosRule_PRK("Leave %s\n",__func__);	
	return SUCCESS;

add_fail_deref_prio:
	rtl865x_qosPriorityMappingDeRef(priority);
add_fail:
	if(new_qosRule)
		kfree(new_qosRule);
	DBG_QosRule_PRK("Leave %s @ %d\n",__func__,__LINE__);	
	return FAILED;
}




int32 rtl865x_qosFlushRule(void)
{
	rtl865x_qos_rule_t	*qosRule_entry;
	rtl865x_qos_rule_t	*qosRule_nxt;

	DBG_QosRule_PRK("Enter %s\n",__func__);
	
	list_for_each_entry_safe(qosRule_entry, qosRule_nxt,&rtl865x_qosRuleHead,qos_rule_list) {		 
		_rtl865x_qosDelRule(qosRule_entry);
	}

	
	return SUCCESS;
}

int32 rtl865x_Qos_SetRemarking(int enable_8021p,int enable_dscp)
{
	DBG_OutputQueue_PRK("Enter %s  (enable_8021p:%d  enable_dscp:%d)\n",__func__,enable_8021p,enable_dscp);
	return _rtl865x_qos_remark(enable_8021p,enable_dscp);
}


int32 rtl865x_closeQos(void)
{
	
	uint32	i;

	DBG_OutputQueue_PRK("Enter %s\n",__func__);
	/* 1.  clear Qid  mapping and acl priotiy mapping */
	_rtl865x_qosQIDMappingClear();
	upstream_default_swQid = -1;

	/*  2. flush all sw qos rules */	
	rtl865x_qosFlushRule();
	rtl865x_raiseEvent(EVENT_FLUSH_QOSRULE, NULL); /* inform napt table  to cancel its IPQos*/
	/*  3. clear default acl priority */
	rtl865x_qosPriorityMappingDeRef(upstream_priority_default);
	
	/* 4. check whether all ref count reaches zero ? */
	for(i=0;i<TOTAL_VLAN_PRIORITY_NUM;i++)
	{


		if( upstream_priority_mapping[i].valid)
		{			
			printk("(%s)!!!!BUG!!!!  @%d (return %d, ref:%d)\n",__func__,__LINE__,i,upstream_priority_mapping[i].ref_count);		
		}
	}

	
	/*  5. Start  to set Queue scheduling .. (back to number 1)  */
	if(_rtl865x_qos_setQueueInfo(1,0,1,NULL,NULL)!=SUCCESS)
	{
		DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}

	/*  6. Disable 802.1p/DSCP remarking */
	if(_rtl865x_qos_remark(0,0)!=SUCCESS)
	{
		DBG_OutputQueue_PRK("Leave %s@ %d(return FAILED)\n",__func__,__LINE__);
		return FAILED;
	}

	DBG_OutputQueue_PRK("Leave %s\n",__func__);
	return SUCCESS;
}

/* When enable hwQos  ....*/
int __init rtl865x_initOutputQueue(void)
{
	int	i;

	DBG_OutputQueue_PRK("Enter %s\n",__func__);	

	#if 0
	/* register Qos chain in ACL */
	{
		ForEachMasterNetif_Declaration
		ForEachMasterNetif_Start
			rtl865x_regist_aclChain(netif->name, RTL865X_ACL_QOS_USED0 ,8);
			rtl865x_regist_aclChain(netif->name, RTL865X_ACL_QOS_USED1 ,0);
		ForEachMasterNetif_End		
	}	
	#endif

	/* Init some sw_data (sw_priority_mapping , sw_qosrule .. ) */
	upstream_priority_default = -1;
	memset(upstream_priority_mapping, 0, TOTAL_VLAN_PRIORITY_NUM*sizeof(aclpriority_mapping_data_t));

	
	for(i=0;i<RTL8651_OUTPUTQUEUE_SIZE;i++)
	{			
		upstream_hwQid2swQid_Mapping[i] = Queue_NOT_Create;			
	}	

	INIT_LIST_HEAD(&rtl865x_qosRuleHead);

	DBG_OutputQueue_PRK("Leave %s\n",__func__);
	
	return SUCCESS;
}

void __exit rtl865x_exitOutputQueue(void)
{
	ForEachMasterNetif_Declaration
	ForEachMasterNetif_Start
		rtl865x_unRegist_aclChain(netif->name, RTL865X_ACL_QOS_USED0);
		//rtl865x_unRegist_aclChain(netif->name, RTL865X_ACL_QOS_USED1);
	ForEachMasterNetif_End

	rtl865x_qosFlushBandwidth(0xFF);	
	rtl865x_closeQos();
}






/*
* Copyright c                  Realtek Semiconductor Corporation, 2008  
* All rights reserved.
* 
* Program : napt table driver
* Abstract : 
* Author : hyking (hyking_liu@realsil.com.cn)  
*/

/*      @doc RTL_LAYEREDDRV_API

        @module rtl865x_nat.c - RTL865x Home gateway controller Layered driver API documentation       |
        This document explains the API interface of the table driver module. Functions with rtl865x prefix
        are external functions.
        @normal Hyking Liu (Hyking_liu@realsil.com.cn) <date>

        Copyright <cp>2008 Realtek<tm> Semiconductor Cooperation, All Rights Reserved.

        @head3 List of Symbols |
        Here is a list of all functions and variables in this module.
        
        @index | RTL_LAYEREDDRV_API
*/

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include <net/rtl/rtl865x_netif.h>
#include "common/mbuf.h"
#include "AsicDriver/rtl865x_asicCom.h"




//#include "assert.h"

//#include "rtl865xc_swNic.h"
//#include <common/types.h>
#include "AsicDriver/rtl865x_hwPatch.h"		/* define for chip related spec */
#ifdef CONFIG_RTL_LAYERED_ASIC_DRIVER

#include "AsicDriver/rtl865x_asicL4.h"
#else
#include "common/rtl8651_aclLocal.h"
#include "rtl865xC_tblAsicDrv.h"
#endif

#include "common/rtl_errno.h"
//#include <net/rtl/rtl_queue.h>
#include "AsicDriver/rtl865xc_asicregs.h"
#include "common/rtl865x_eventMgr.h"
#include "l3Driver/rtl865x_ip.h"

#include <net/rtl/rtl865x_nat.h>
#include "rtl865x_nat_local.h"

//#include "rtl865x_ppp.h"
#include "common/rtl865x_netif_local.h"



#ifdef CONFIG_RTL_PROC_DEBUG
#include <linux/seq_file.h>
#endif

#include <linux/jiffies.h>

#ifdef CONFIG_RTL_HW_QOS_SUPPORT
#include <net/rtl/rtl865x_outputQueue.h>
#endif


struct nat_table nat_tbl;
static int32 rtl865x_enableNaptFourWay=FALSE;



static int32 _rtl865x_nat_init(void)
{
	rtl865x_tblAsicDrv_naptTcpUdpParam_t naptTcpUdp;
	uint32 flowTblIdx;
	
	memset(nat_tbl.nat_bucket, 0, 
	sizeof(struct nat_entry)*RTL8676_TCPUDPTBL_SIZE_HW);

	nat_tbl.tcp_timeout = TCP_TIMEOUT; //24*60*60;
	nat_tbl.udp_timeout = UDP_TIMEOUT; //60*15;
	nat_tbl.freeHWEntryNum=RTL8676_TCPUDPTBL_SIZE_HW;

	/* Set ASIC timeout value */
	rtl8651_setAsicNaptTcpLongTimeout(TCP_TIMEOUT);
	rtl8651_setAsicNaptTcpMediumTimeout(TCP_TIMEOUT);
	rtl8651_setAsicNaptTcpFastTimeout(TCP_TIMEOUT);
	rtl8651_setAsicNaptUdpTimeout(UDP_TIMEOUT);

	/*enable 865xC enhanced hash1*/
	_rtl8651_enableEnhancedHash1();
	
	/* Initial ASIC NAT Table */
	memset( &naptTcpUdp, 0, sizeof(naptTcpUdp) );
	naptTcpUdp.isCollision = 1;
	naptTcpUdp.isCollision2 = 1;
	for(flowTblIdx=0; flowTblIdx<RTL8676_TCPUDPTBL_SIZE_HW; flowTblIdx++)
		rtl8651_setAsicNaptTcpUdpTable(TRUE, flowTblIdx, &naptTcpUdp );

	//rtl865x_nat_register_event();
		
	return SUCCESS;
}


static struct nat_entry * _rtl865x_nat_outbound_lookup(struct nat_tuple *nat_tuple)
{
	struct nat_entry *nat_out;
	uint32 i,hash;


	hash = rtl8651_naptTcpUdpTableIndex((uint8)nat_tuple->proto, nat_tuple->int_host.ip, nat_tuple->int_host.port, 
											nat_tuple->rem_host.ip, nat_tuple->rem_host.port);
	assert(hash<RTL8676_TCPUDPTBL_SIZE_HW);


	
	
	if(rtl865x_enableNaptFourWay==TRUE)
	{
		for(i=0; i<4; i++)
		{
			nat_out = &nat_tbl.nat_bucket[hash];
			if (!memcmp(nat_out, nat_tuple, sizeof(*nat_tuple)) &&
				(nat_out->flags&NAT_OUTBOUND))
			{
				return nat_out;
			}
			hash=(hash&0xFFFFFFFC)+(hash+1)%4;		
		}
	}
	else
	{
		nat_out = &nat_tbl.nat_bucket[hash];
		if (!memcmp(nat_out, nat_tuple, sizeof(*nat_tuple)) &&
			(nat_out->flags&NAT_OUTBOUND))
			return nat_out;
	}
	return NULL;
}


static struct nat_entry * _rtl865x_nat_inbound_lookup(struct nat_tuple *nat_tuple)
{
	struct nat_entry *nat_in;
	uint32 hash;

	hash = rtl8651_naptTcpUdpTableIndex((uint8)nat_tuple->proto, nat_tuple->rem_host.ip, nat_tuple->rem_host.port, 
											nat_tuple->ext_host.ip, nat_tuple->ext_host.port);
	assert(hash<RTL8676_TCPUDPTBL_SIZE_HW);


	
	nat_in = &nat_tbl.nat_bucket[hash];
	if (!memcmp(nat_in, nat_tuple, sizeof(*nat_tuple)) && 
		(nat_in->flags&NAT_INBOUND))
	{
			return nat_in;
	}
	
	return NULL;
}

#if defined (CONFIG_RTL_INBOUND_COLLISION_AVOIDANCE)
static int _rtl865x_isEntryPreReserved(uint32 index)
{
	struct nat_entry *natEntry;
	if(index>=RTL8676_TCPUDPTBL_SIZE_HW)
	{
		return FALSE;
	}
	
	natEntry= &nat_tbl.nat_bucket[index];

	if((natEntry->flags & NAT_PRE_RESERVED))
	{
		if(jiffies>=natEntry->reserveTime)
		{
			if(jiffies>(natEntry->reserveTime+RESERVE_EXPIRE_TIME*HZ))
			{
				/*pre-reserve become invalid now*/
				natEntry->flags &= (~(NAT_PRE_RESERVED));
				natEntry->reserveTime=0;
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
		else
		{
			/*timer overflow*/
			if(((0xFFFFFFFF-natEntry->reserveTime)+jiffies+1)>(RESERVE_EXPIRE_TIME*HZ))
			{
				natEntry->flags &= (~(NAT_PRE_RESERVED));
				natEntry->reserveTime=0;
				return FALSE;
			}
			else
			{
				return TRUE;
			}
		}
		return TRUE;
	}

	return FALSE;
}

static int _rtl865x_PreReserveEntry(uint32 index)
{
	struct nat_entry *natEntry;
	if(index>=RTL8676_TCPUDPTBL_SIZE_HW)
	{
		return FAILED;
	}
	natEntry= &nat_tbl.nat_bucket[index];
	
	if(NAT_INUSE(natEntry))
	{
		/*already used by other napt connection, can not reserve it*/
		natEntry->flags &= (~(NAT_PRE_RESERVED));
		natEntry->reserveTime=0;
	}
	else
	{
		natEntry->flags|=NAT_PRE_RESERVED;
		natEntry->reserveTime=jiffies;
	}
	return SUCCESS;
}


static int _rtl865x_getNaptHashInfo( uint32 protocol, ipaddr_t intIp, uint32 intPort,
                        ipaddr_t extIp, uint32 extPort,
                        ipaddr_t remIp, uint32 remPort, 
                        rtl865x_naptHashInfo_t *naptHashInfo)
{
	
	uint32 in, out;
	uint32  i,index;
	struct nat_entry *nat_in, *nat_out;
	struct nat_entry *natEntry;
	
	if(naptHashInfo==NULL)
	{
		return FAILED;
	}
	
	memset(naptHashInfo, 0, sizeof(rtl865x_naptHashInfo_t));
	
	in = rtl8651_naptTcpUdpTableIndex((uint8)protocol, remIp, remPort, extIp, extPort);
	out = rtl8651_naptTcpUdpTableIndex((uint8)protocol, intIp, intPort, remIp, remPort);

	if(rtl865x_enableNaptFourWay==TRUE)
	{
		uint32 outAvailIdx=0xFFFFFFFF;
		index=out;
		for(i=0;i<4;i++)
		{
			natEntry = &nat_tbl.nat_bucket[index];
			if (NAT_INUSE(natEntry) || _rtl865x_isEntryPreReserved(index))
			{
			
			}
			else
			{
				if(index==in)
				{
					/*collide with inbound*/
				}
				else
				{
					out=index;
					break;
				}
			}
			index=(index&0xFFFFFFFC)+(index+1)%4;
			assert(index<=RTL8676_TCPUDPTBL_SIZE_HW);
				
		}

		if(i>=4)
		{
			/*only one empty entry, but collide with its own inbound*/
			if(outAvailIdx!=0xFFFFFFFF)
			{
				out=outAvailIdx;
			}
		}
		else
		{
			/*proper empty entry has been found*/
		}
	}
	
	naptHashInfo->outIndex=out;
	naptHashInfo->inIndex=in;

	if((in&0xFFFFFFFC)==(out&0xFFFFFFFC))
	{
		naptHashInfo->sameFourWay=1;
	}	

	if(in==out)
	{
		naptHashInfo->sameLocation=1;
		
		nat_out = &nat_tbl.nat_bucket[out];
		if(NAT_INUSE(nat_out)|| _rtl865x_isEntryPreReserved(out))
		{
			naptHashInfo->outCollision=1;
		}

		naptHashInfo->inCollision=1;
	}
	else
	{
		nat_out = &nat_tbl.nat_bucket[out];
		nat_in = &nat_tbl.nat_bucket[in];
		
		if(NAT_INUSE(nat_out) || _rtl865x_isEntryPreReserved(out))
		{
			naptHashInfo->outCollision=1;
		}
		
		if (NAT_INUSE(nat_in) ||  _rtl865x_isEntryPreReserved(in))
		{
			naptHashInfo->inCollision=1;
		}
	}
	

	index=in;
	naptHashInfo->inFreeCnt=0;
	for(i=0;i<4;i++)
	{
		natEntry = &nat_tbl.nat_bucket[index];
		if (NAT_INUSE(natEntry) || _rtl865x_isEntryPreReserved(index))
		{
		
		}
		else
		{
			naptHashInfo->inFreeCnt++;
		}
		index=(index&0xFFFFFFFC)+(index+1)%4;
		assert(index<=RTL8676_TCPUDPTBL_SIZE_HW);
	}
	#if 0
	printk("%s:%d:%s (%u.%u.%u.%u:%u -> %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u) ,out is %d,in is %d\n",
			__FUNCTION__,__LINE__,protocol?"tcp":"udp", 
			NIPQUAD(intIp), intPort, NIPQUAD(extIp), extPort, NIPQUAD(remIp), remPort, out, in);	
	#endif
	return SUCCESS;
}

int rtl865x_optimizeExtPort(unsigned short origDelta, unsigned int rangeSize, unsigned short *newDelta)
{
	int i;
	int msb;
	unsigned int bitShift;

	msb=0;
	for(i=0;i<16;i++)
	{
		if((1<<i) & rangeSize)
		{
			msb=i;
		}
	}

	if(((1<<msb)+1)>rangeSize)
	{
		if(msb>1)
		{
			msb--;
		}
	}
		
 	*newDelta=0;
	if(msb<10)
	{
		bitShift=0x01;
		for(i=0;i<=msb;i++)
		{
			if(i==0)/*bit0 keep the same*/
			{
				if(origDelta&bitShift)
				{
					 *newDelta|=bitShift;
				}
			}
			else /*original bit1~ bit_maxPower mapped to bit_maxPower~bit1*/
			{
				if(origDelta&bitShift) 
				{
					 *newDelta |=(0x1<<(msb+1-i));
				}
			}

			bitShift=bitShift<<1;
		}
	}
	else
	{
		bitShift=0x01;
		
		for(i=0;i<=msb;i++)
		{
			if(i==0)	/*bit0 keep the same*/
			{
				if(origDelta&bitShift) 
				{
					*newDelta |=bitShift;
				}
			}
			else if (i<10) /*bit1~ bit9 mapped to bit 9~bit1*/
			{
				if(origDelta&bitShift) 
				{
					*newDelta  |=(0x1<<(10-i));
				}
			}
			else/*other bits keep the same*/
			{
				if(origDelta&bitShift) 
				{
					*newDelta  |=bitShift;
				}
			}

			bitShift=bitShift<<1;
		}


	}
	return SUCCESS;
}

int rtl865x_getAsicNaptHashScore( uint32 protocol, ipaddr_t intIp, uint32 intPort,
					                        ipaddr_t extIp, uint32 extPort,
					                        ipaddr_t remIp, uint32 remPort, 
					                        uint32 *naptHashScore)
{
	 rtl865x_naptHashInfo_t naptHashInfo;
	_rtl865x_getNaptHashInfo( protocol, intIp, intPort, extIp, extPort, remIp, remPort, &naptHashInfo);
	/*initialize napt hash score*/
	*naptHashScore=100;

	/*note:we can not change outbound index*/
	
	if(naptHashInfo.inCollision==FALSE)
	{
		if(naptHashInfo.inFreeCnt==4)
		{
			if(!naptHashInfo.sameFourWay)
			{
				*naptHashScore=100;
			}
			else
			{
				if(!naptHashInfo.sameLocation)
				{
					*naptHashScore=80;
				}
				else
				{
					*naptHashScore=0;
				}
			}
		}
		else if (naptHashInfo.inFreeCnt==3)
		{
			if(!naptHashInfo.sameFourWay)
			{
				*naptHashScore=80;
			}
			else
			{
				if(!naptHashInfo.sameLocation)
				{
					*naptHashScore=70;
				}
				else
				{
					*naptHashScore=0;
				}
			}
		}
		else if (naptHashInfo.inFreeCnt==2)
		{
			if(!naptHashInfo.sameFourWay)
			{
				*naptHashScore=70;
			}
			else
			{
				if(!naptHashInfo.sameLocation==FALSE)
				{
					*naptHashScore=60;
				}
				else
				{
					*naptHashScore=0;
				}
			}
		}
		else if (naptHashInfo.inFreeCnt==1)
		{
			if(naptHashInfo.sameFourWay==FALSE)
			{
				*naptHashScore=60;
			}
			else
			{
				*naptHashScore=0;
					
			}
		}
		else
		{
			*naptHashScore=0;
		}
		

	}
	else
	{
		/*worst case:inbound is collision*/
		*naptHashScore=0;
	}
	
	return SUCCESS;

}

int32 rtl865x_preReserveConn( uint32 protocol, ipaddr_t intIp, uint32 intPort,
					                        ipaddr_t extIp, uint32 extPort,
					                        ipaddr_t remIp, uint32 remPort)
{

	 rtl865x_naptHashInfo_t naptHashInfo;
	_rtl865x_getNaptHashInfo( protocol, intIp, intPort, extIp, extPort, remIp, remPort, &naptHashInfo);

	if(naptHashInfo.outCollision==FALSE)
	{
		_rtl865x_PreReserveEntry(naptHashInfo.outIndex);
	}

	if(naptHashInfo.inCollision==FALSE)
	{
		_rtl865x_PreReserveEntry(naptHashInfo.inIndex);
	}
	
	return SUCCESS;
}	

#endif

static int32 _rtl865x_lookupNaptConnection
		(ipaddr_t intIp, uint16 intPort,ipaddr_t extIp, uint16 extPort,ipaddr_t remIp, uint16 remPort,uint8 protocol,uint8 isUpstream)
{
	struct nat_tuple  nat_tuple;	
	struct nat_entry* nat_entry;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_nat;
	int index,rc;

	memset(&nat_tuple, 0, sizeof(struct nat_tuple));
	nat_tuple.int_host.ip			= intIp;
	nat_tuple.int_host.port			= intPort;
	nat_tuple.ext_host.ip			= extIp;
	nat_tuple.ext_host.port		= extPort;
	nat_tuple.rem_host.ip			= remIp;
	nat_tuple.rem_host.port		= remPort;
	nat_tuple.proto				= protocol;	

	if(isUpstream)
		nat_entry = _rtl865x_nat_outbound_lookup(&nat_tuple);
	else
		nat_entry = _rtl865x_nat_inbound_lookup(&nat_tuple);

	
	if (nat_entry==NULL)	
		return -1;	
	
	if(isUpstream)
		index = nat_entry->out;
	else
		index = nat_entry->in;
	
	

	memset(&asic_nat ,0 ,sizeof(rtl865x_tblAsicDrv_naptTcpUdpParam_t));
	rc = rtl8651_getAsicNaptTcpUdpTable(index, &asic_nat);
	assert(rc==SUCCESS);
	assert(asic_nat.ageSec>=0);
	
	return asic_nat.ageSec;
}


static int32 _rtl865x_addNaptConnection
		(ipaddr_t intIp, uint16 intPort,ipaddr_t extIp, uint16 extPort,ipaddr_t remIp, uint16 remPort,uint8 protocol,uint8 pri_valid,uint8 pri_value,uint8 isUpstream)
{
	uint32 in, out , result_idx;
	rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_nat;
	int ret;


	/* 0. santity check */
	if(protocol!=RTL865X_PROTOCOL_TCP && protocol!=RTL865X_PROTOCOL_UDP)	
		return RTL_EINVALIDINPUT;
	if(pri_valid && (pri_value<0||pri_value>=TOTAL_VLAN_PRIORITY_NUM))
		return RTL_EINVALIDINPUT;
		

	/* 1. duplicate check */
	ret = _rtl865x_lookupNaptConnection(intIp, intPort, extIp, extPort, remIp, remPort,protocol,isUpstream);
	if(ret!=-1)
		return RTL_EENTRYALREADYEXIST;
		

	/* 2.  we have to check both inbound and outbound index at the same time (we have to promise that out != in) */
	out = rtl8651_naptTcpUdpTableIndex((uint8)protocol, intIp, intPort, remIp, remPort);
	in  = rtl8651_naptTcpUdpTableIndex((uint8)protocol, remIp, remPort, extIp, extPort);


	if(rtl865x_enableNaptFourWay==FALSE)	
	{		
		if(out==in) /*we don't support this case at present, otherwise, when delete napt connection must be very very careful*/	
			return RTL_EINVALIDINPUT;


		if(isUpstream)
		{
			if ( NAT_INUSE(&(nat_tbl.nat_bucket[out])) )
			{
				/* the index has been occupied*/			
				return RTL_EINVALIDINPUT;
			}
			else
				result_idx = out;
		}
		else /* downstream */
		{
			if ( NAT_INUSE(&(nat_tbl.nat_bucket[in])) )
			{
				/* the index has been occupied*/			
				return RTL_EINVALIDINPUT;
			}
			else
				result_idx = in;
		}
	}
	else
	{
		/* we have 4 chances to find one outbound hash index which is not equal to inbound index  */
		int i;
		result_idx=RTL8676_TCPUDPTBL_SIZE_HW;
		for(i=0;i<4;i++)
		{
			uint32 fourway_hash=(out&0xFFFFFFFC)+(out+i)%4;

			if(fourway_hash==in)
				continue;

			if(isUpstream)
			{
				if ( NAT_INUSE(&(nat_tbl.nat_bucket[fourway_hash])) )
					continue;
			}
			else /* downstream */
			{
				if ( NAT_INUSE(&(nat_tbl.nat_bucket[in])) )
					break; //exit for loop directly
			}

			/*   we find it if reaches here */
			if(isUpstream)
				result_idx = fourway_hash;
			else
				result_idx = in;
			break;
		}		

		if(result_idx==RTL8676_TCPUDPTBL_SIZE_HW)
			return RTL_EINVALIDINPUT;	
		
	}

	/* 3. set sw_napt */
	memset(&(nat_tbl.nat_bucket[result_idx]), 0, sizeof(struct nat_entry));	
	nat_tbl.nat_bucket[result_idx].tuple_info.int_host.ip 	= intIp;
	nat_tbl.nat_bucket[result_idx].tuple_info.int_host.port	= intPort;
	nat_tbl.nat_bucket[result_idx].tuple_info.ext_host.ip 	= extIp;
	nat_tbl.nat_bucket[result_idx].tuple_info.ext_host.port	= extPort;
	nat_tbl.nat_bucket[result_idx].tuple_info.rem_host.ip 	= remIp;
	nat_tbl.nat_bucket[result_idx].tuple_info.rem_host.port	= remPort;
	nat_tbl.nat_bucket[result_idx].tuple_info.proto		= protocol;
	if(isUpstream)
	{
		nat_tbl.nat_bucket[result_idx].out	=result_idx;
		SET_NAT_FLAGS(&(nat_tbl.nat_bucket[result_idx]), NAT_OUTBOUND);
	}
	else
	{
		nat_tbl.nat_bucket[result_idx].in	=result_idx;
		SET_NAT_FLAGS(&(nat_tbl.nat_bucket[result_idx]), NAT_INBOUND);
	}
	
	#ifdef CONFIG_RTL_HW_QOS_SUPPORT
	if(pri_valid)
	{
		SET_NAT_FLAGS(&(nat_tbl.nat_bucket[result_idx]), NAT_PRIOTIY_VALID);
		nat_tbl.nat_bucket[result_idx].priority = pri_value;
	}
	#endif	

	/*  4. finally, set asic napt (hw_napt) */
	memset(&asic_nat, 0, sizeof(asic_nat));		
	asic_nat.insideLocalIpAddr	= intIp;
	asic_nat.insideLocalPort	= intPort;
	asic_nat.isCollision		= 0;
	asic_nat.isCollision2		= 0;
	asic_nat.isDedicated		= 0;
	asic_nat.isStatic			= 1;
	asic_nat.isTcp			= (protocol==RTL865X_PROTOCOL_TCP)? 1: 0;
	asic_nat.isValid			= 1;

	if(isUpstream)
	{
	asic_nat.offset			= (extPort&0x0000ffff)>>10;
	asic_nat.selEIdx			= (extPort&0x3ff);	
	asic_nat.selExtIPIdx		= 0; 		/* for upstream(outbound), the selExtIPIdx is useless.....*/													
	asic_nat.tcpFlag			= 0x3;		/*  bit0 :  0:inbound   				1 : outbound 	
										     bit1 :  0:the napt flow use 1 entry  	1 : the napt flow use 2 entries  
									  	     bit2 :  0:trap SYN/FIN/RST   		1 : do not trap 
										    (enhanced hash1 doesn't support outbound/inbound share one connection) */
	}
	else
	{
	uint16 very = rtl8651_naptTcpUdpTableIndex(((uint8)protocol) |HASH_FOR_VERI , remIp, remPort, 0, 0);
	asic_nat.offset			= (extPort & 0x3f);
	asic_nat.selEIdx			= very &0x3ff;	
	asic_nat.selExtIPIdx		= (extPort & 0x3ff) >> 6; 												
	asic_nat.tcpFlag			= 0x2;	
	}

	asic_nat.ageSec			= (protocol==RTL865X_PROTOCOL_TCP)?(nat_tbl.tcp_timeout):(nat_tbl.udp_timeout);
	#ifdef CONFIG_RTL_HW_QOS_SUPPORT
	if(pri_valid)
	{
	asic_nat.priValid			= TRUE;
	asic_nat.priority			= pri_value;
	}
	else
	#endif
	asic_nat.priValid			= FALSE; /* we use acl to set flow priority , so we do not need napt base ipqos */
	rtl8651_setAsicNaptTcpUdpTable(1, result_idx, &asic_nat);
	nat_tbl.freeHWEntryNum--;
	
	return SUCCESS;
}

 
static int32 _rtl865x_delNaptConnection
		(ipaddr_t intIp, uint16 intPort,ipaddr_t extIp, uint16 extPort,ipaddr_t remIp, uint16 remPort,uint8 protocol,uint8 isUpstream)
{
	struct nat_tuple nat_tuple;
	struct nat_entry *nat_entry;
	int index=-1;

	if(protocol!=RTL865X_PROTOCOL_TCP && protocol!=RTL865X_PROTOCOL_UDP)
		return RTL_EINVALIDINPUT;	
	
	memset(&nat_tuple, 0, sizeof(struct nat_tuple));
	nat_tuple.int_host.ip			= intIp;
	nat_tuple.int_host.port			= intPort;
	nat_tuple.ext_host.ip			= extIp;
	nat_tuple.ext_host.port		= extPort;
	nat_tuple.rem_host.ip			= remIp;
	nat_tuple.rem_host.port		= remPort;
	nat_tuple.proto				= protocol;	
	
	if(isUpstream)
	{
		nat_entry = _rtl865x_nat_outbound_lookup(&nat_tuple);
		if(nat_entry)
			index = nat_entry->out;
	}
	else
	{
		nat_entry = _rtl865x_nat_inbound_lookup(&nat_tuple);
		if(nat_entry)
			index = nat_entry->in;
	}

	if(index!=-1)
	{
	#ifdef CONFIG_RTL_HW_QOS_SUPPORT
		if(nat_entry->flags&NAT_PRIOTIY_VALID)
			rtl865x_qosPriorityMappingDeRef(nat_entry->priority);			
	#endif
	
		
	
		rtl8651_delAsicNaptTcpUdpTable(index, index);
		nat_tbl.freeHWEntryNum++;

		memset(nat_entry, 0, sizeof(struct nat_entry));
	
	}	
	
	return SUCCESS;
}

/*  isUpstream=1   =>  Upstream		 :  Src NAPT or Pure routing ,  naptIp/naptPort = extIp/extPort
      isUpstream=0   =>  Downstream	 :  Dst NAPT or Pure routing ,  naptIp/naptPort = privateIp/privatePort

      We record Src/Dst NAPT in both sw and hw_napt table ,  Pure routing only in sw_napt

      <Note.> napt table is NOT correlated with pure routing 
      		     (pure routing only use arp,routing table , acl / nexthop table for redirecting)
      		     Here, we just record pure routing in sw_napt table only for caller to trace whether the specified conntrack is maintaind by ourselves

      		     For example, in dynamical case 
      		     1.  We use rtl865x_addNaptConnection to add a pure routing connection
      		     2.  Then, use dynamic acl to permit this connection.
      		     3.  After a while , when kernel's nf_conntrack is going to timeout , 
      		          use rtl865x_lookupNaptConnection to assure that this connection is maintained by 8676 hw-acc .
      		          We have to check  whether this connection is still alive in hw. (see _rtl8676_query_L34Unicast_hwacc)
      		          If it is also timeout in hw, use  rtl865x_delNaptConnection to cancel this maintain relationship.
      */
int32 rtl865x_addNaptConnection
		(ipaddr_t scrIp, uint16 scrPort,ipaddr_t dstIp, uint16 dstPort,ipaddr_t naptIp, uint16 naptPort,uint8 protocol,uint8 pri_valid,uint8 pri_value,uint8 isUpstream)
{
	int32 retval = FAILED;
	if(isUpstream)
		retval = _rtl865x_addNaptConnection(scrIp,scrPort,naptIp,naptPort,dstIp,dstPort,protocol,pri_valid,pri_value,1);
	else
		retval = _rtl865x_addNaptConnection(naptIp,naptPort,dstIp,dstPort,scrIp,scrPort,protocol,pri_valid,pri_value,0);
	return retval;
}

int32 rtl865x_delNaptConnection
		(ipaddr_t scrIp, uint16 scrPort,ipaddr_t dstIp, uint16 dstPort,ipaddr_t naptIp, uint16 naptPort,uint8 protocol,uint8 isUpstream)
{
	int32 retval = FAILED;
	if(isUpstream)
		retval = _rtl865x_delNaptConnection(scrIp,scrPort,naptIp,naptPort,dstIp,dstPort,protocol,1);
	else
		retval = _rtl865x_delNaptConnection(naptIp,naptPort,dstIp,dstPort,scrIp,scrPort,protocol,0);
	return retval;
}

/* return 	-1   	: the entry does not exist 
		 0    	: aging out  
		 >0 	:  its aging time left                */
int32 rtl865x_lookupNaptConnection
		(ipaddr_t scrIp, uint16 scrPort,ipaddr_t dstIp, uint16 dstPort,ipaddr_t naptIp, uint16 naptPort,uint8 protocol,uint8 isUpstream)
{
	if(isUpstream)
		return _rtl865x_lookupNaptConnection(scrIp,scrPort,naptIp,naptPort,dstIp,dstPort,protocol,1);
	else
		return _rtl865x_lookupNaptConnection(naptIp,naptPort,dstIp,dstPort,scrIp,scrPort,protocol,0);
}

int32 rtl865x_flushNapt(void)
{
	uint32 i;
	
	for(i=0;i<RTL8676_TCPUDPTBL_SIZE_HW;i++)
	{
		if(NAT_INUSE(&nat_tbl.nat_bucket[i]))
		{
		#ifdef CONFIG_RTL_HW_QOS_SUPPORT
			if(nat_tbl.nat_bucket[i].flags&NAT_PRIOTIY_VALID)					
				rtl865x_qosPriorityMappingDeRef(nat_tbl.nat_bucket[i].priority);					
		#endif
		

		
			rtl8651_delAsicNaptTcpUdpTable(i, i);
			nat_tbl.freeHWEntryNum++;
					
			memset(&nat_tbl.nat_bucket[i], 0, sizeof(struct nat_entry));
			
		}
	}	
	return SUCCESS;
}
#ifdef CONFIG_RTL_HW_QOS_SUPPORT
static int32 rtl865x_naptCallbackFn_for_closeIPQos(void *param)
{
	uint32 i;
	
	for(i=0;i<RTL8676_TCPUDPTBL_SIZE_HW;i++)
	{
		if(NAT_INUSE(&nat_tbl.nat_bucket[i]))
		{			
			struct nat_entry* entry = &nat_tbl.nat_bucket[i];

			if(entry->flags&NAT_PRIOTIY_VALID)
			{
				rtl865x_tblAsicDrv_naptTcpUdpParam_t asic_nat;				
				int32		idx= (entry->flags&NAT_INBOUND)?entry->in:entry->out;			
				
				rtl8651_getAsicNaptTcpUdpTable(idx, &asic_nat);
				asic_nat.priority = 0;
				asic_nat.priValid = FALSE;
				rtl8651_setAsicNaptTcpUdpTable(1, idx, &asic_nat);

				rtl865x_qosPriorityMappingDeRef(entry->priority);
				CLR_NAT_FLAGS(entry, NAT_PRIOTIY_VALID);
			}
		}
	}		
	return EVENT_CONTINUE_EXECUTE;
}
#endif

/*
@func int32 | rtl865x_setNatFourWay |enable 4way hash algorithm.
@parm int32 | enable | enable or disable.
@rvalue SUCCESS | success.
@comm
	default is enable in system.
*/
int32 rtl865x_setNatFourWay(int32 enable)
{
	 _set4WayHash(enable);
	rtl865x_enableNaptFourWay=enable;
	return SUCCESS;
}

/*
@func int32 | rtl865x_nat_init |initialize napt table.
@rvalue SUCCESS | success.
@comm	
*/
int32 rtl865x_nat_init(void)
{
	int32 retval = FAILED;
#ifdef CONFIG_RTL_HW_QOS_SUPPORT
	rtl865x_event_Param_t eventParam;
#endif

	retval = _rtl865x_nat_init();
	rtl865x_setNatFourWay(TRUE);
#ifdef CONFIG_RTL_HW_QOS_SUPPORT	
	eventParam.eventLayerId=DEFAULT_LAYER2_EVENT_LIST_ID;
	eventParam.eventId=EVENT_FLUSH_QOSRULE;
	eventParam.eventPriority=0;
	eventParam.event_action_fn=rtl865x_naptCallbackFn_for_closeIPQos;
	rtl865x_registerEvent(&eventParam);
#endif

	return retval;
}

int32 rtl865x_nat_reinit(void)
{
	return rtl865x_nat_init();

}
/*
return 	0: dst ip is in use
	     -1: dst ip is in use by aging out napt connection or not in use by any napt connection
*/
int32 rtl865x_checkNaptConnection(ipaddr_t ip) //check if dst ip is in use by napt connection
{
	int i;
	struct nat_entry *natEntryPtr;

	for(i=0; i<RTL8676_TCPUDPTBL_SIZE_HW; i++)
	{
		natEntryPtr= &nat_tbl.nat_bucket[i];
		if(NAT_INUSE(natEntryPtr))
		{
			
			if(natEntryPtr->flags&NAT_OUTBOUND)
			{		
				if((ip ^ natEntryPtr->rem_ip_)==0){
					if(_rtl865x_lookupNaptConnection(natEntryPtr->int_ip_, natEntryPtr->int_port_, natEntryPtr->ext_ip_, 
					natEntryPtr->ext_port_, natEntryPtr->rem_ip_, natEntryPtr->rem_port_, natEntryPtr->proto_, 1) > 0)
					return 0;
				}
			}

			if(natEntryPtr->flags&NAT_INBOUND)
			{
				if((ip ^ natEntryPtr->int_ip_)==0){
					if(_rtl865x_lookupNaptConnection(natEntryPtr->int_ip_, natEntryPtr->int_port_, natEntryPtr->ext_ip_, 
					natEntryPtr->ext_port_, natEntryPtr->rem_ip_, natEntryPtr->rem_port_, natEntryPtr->proto_, 0) > 0)
					return 0;
				}
			}
		}
	}
	return -1;
	
}

#ifdef CONFIG_RTL_PROC_DEBUG
int32 rtl865x_sw_napt_seq_read(struct seq_file *s, void *v)
{

	int i;
	struct nat_entry *natEntryPtr;
	int len=0;
	
	len = seq_printf(s, "%s\n", "sw napt table:");
	
	for(i=0; i<RTL8676_TCPUDPTBL_SIZE_HW; i++)
	{
		natEntryPtr= &nat_tbl.nat_bucket[i];
		if(NAT_INUSE(natEntryPtr))
		{
			if(natEntryPtr->flags&NAT_OUTBOUND)
			{
				len += seq_printf(s, "[%4d]%s:%d.%d.%d.%d:%d---->%d.%d.%d.%d:%d---->%d.%d.%d.%d:%d\n      flags:0x%x,outbound:(%d),inbound:(%d)",
				i,natEntryPtr->proto_==1?"tcp":"udp" ,NIPQUAD(natEntryPtr->int_ip_),natEntryPtr->int_port_,
				NIPQUAD(natEntryPtr->ext_ip_),natEntryPtr->ext_port_,NIPQUAD(natEntryPtr->rem_ip_),natEntryPtr->rem_port_,natEntryPtr->flags,natEntryPtr->out, natEntryPtr->in);
			}

			if(natEntryPtr->flags&NAT_INBOUND)
			{
				len += seq_printf(s, "[%4d]%s:%d.%d.%d.%d:%d<----%d.%d.%d.%d:%d<----%d.%d.%d.%d:%d\n      flags:0x%x, outbound:(%d), inbound:(%d)",
				i,natEntryPtr->proto_==1?"tcp":"udp" ,NIPQUAD(natEntryPtr->int_ip_),natEntryPtr->int_port_,
				NIPQUAD(natEntryPtr->ext_ip_),natEntryPtr->ext_port_,NIPQUAD(natEntryPtr->rem_ip_),natEntryPtr->rem_port_,natEntryPtr->flags,natEntryPtr->out,natEntryPtr->in);
			}
			#ifdef CONFIG_RTL_HW_QOS_SUPPORT
			if(natEntryPtr->flags & NAT_PRIOTIY_VALID)
				len += seq_printf(s, " priority:%d\n",natEntryPtr->priority);
			else
			#endif
				len += seq_printf(s, "\n");
		}
	
	}
	
	len += seq_printf(s, "total free HW entry number is %d\n",nat_tbl.freeHWEntryNum);
	return 0;
}

int32  rtl865x_sw_napt_seq_write( struct file *filp, const char *buff,unsigned long len, loff_t *off )
{
	char 	tmpbuf[64];
	uint32	delIndex;
	char		*strptr, *cmd_addr;
	char		*tokptr;

	
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		cmd_addr = strsep(&strptr," ");
		if (cmd_addr==NULL)
		{
			goto errout;
		}

		if (!memcmp(cmd_addr, "flush", 5))
		{
			rtl865x_flushNapt();
		}
		else if (!memcmp(cmd_addr, "del", 3))
		{
			tokptr = strsep(&strptr," ");
			if (tokptr==NULL)
			{
				goto errout;
			}

			delIndex=simple_strtol(tokptr, NULL, 0);
			if(delIndex>RTL8676_TCPUDPTBL_SIZE_HW)
			{
				printk("error input!\n");
				return len;
			}		

			if(NAT_INUSE(&nat_tbl.nat_bucket[delIndex]))
			{			
				
				rtl8651_delAsicNaptTcpUdpTable(delIndex, delIndex);
				nat_tbl.freeHWEntryNum++;
								
				memset(&nat_tbl.nat_bucket[delIndex], 0, sizeof(struct nat_entry));
						
			}
			
		}
		else
		{
			goto errout;
		}
	}
	else
	{
errout:
		return len;
	}

	return len;
}

#endif

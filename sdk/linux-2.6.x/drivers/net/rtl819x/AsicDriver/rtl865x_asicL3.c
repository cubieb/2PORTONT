/*
* Copyright c                  Realtek Semiconductor Corporation, 2009  
* All rights reserved.
* 
* Program : Switch table Layer3 route driver,following features are included:
*	Route/Multicast
* Abstract :
* Author : hyking (hyking_liu@realsil.com.cn)  
*/
#include <net/rtl/rtl_types.h>
#include "asicRegs.h"
#include "rtl865x_asicCom.h"
#include "rtl865x_asicBasic.h"
#include "rtl865x_asicL3.h"
//#include "rtl_utils.h"
#include "rtl865x_hwPatch.h"


int32 rtl8651_setAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp) 
{
	rtl8651_tblAsic_extIpTable_t   entry;
	
	if((index >= RTL8651_IPTABLE_SIZE) || (extIntIpp == NULL) || 
	((extIntIpp->localPublic == TRUE) && (extIntIpp->nat == TRUE))) //Local public IP and NAT property cannot co-exist
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.externalIP = extIntIpp->extIpAddr;
	entry.internalIP = extIntIpp->intIpAddr;
	entry.isLocalPublic = extIntIpp->localPublic==TRUE? 1: 0;
	entry.isOne2One = extIntIpp->nat==TRUE? 1: 0;
    	entry.nextHop = extIntIpp->nhIndex;
	entry.valid = 1;
	return _rtl8651_forceAddAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);

}

int32 rtl8651_delAsicExtIntIpTable(uint32 index) 
{
    	rtl8651_tblAsic_extIpTable_t   entry;

	if(index >= RTL8651_IPTABLE_SIZE)
		return FAILED;
        
	memset(&entry,0,sizeof(entry));
	entry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
}

int32 rtl8651_getAsicExtIntIpTable(uint32 index, rtl865x_tblAsicDrv_extIntIpParam_t *extIntIpp)
{
    	rtl8651_tblAsic_extIpTable_t   entry;
    
	if((index>=RTL8651_IPTABLE_SIZE) || (extIntIpp == NULL))
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_EXT_INT_IP_TABLE, index, &entry);
	if(entry.valid == 0)
		return FAILED;//Entry not found
	extIntIpp->extIpAddr = entry.externalIP;
	extIntIpp->intIpAddr = entry.internalIP;
	extIntIpp->localPublic = entry.isLocalPublic==1? TRUE: FALSE;
	extIntIpp->nat = entry.isOne2One==1? TRUE: FALSE;
    	extIntIpp->nhIndex = entry.nextHop;
    	return SUCCESS;
}

int32 rtl8651_setAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep) 
{
	rtl8651_tblAsic_pppoeTable_t entry;

	if((index >= RTL8651_PPPOETBL_SIZE) || (pppoep == NULL) || (pppoep->sessionId == 0xffff))
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.sessionID = pppoep->sessionId;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	//entry.ageTime = pppoep->age;
	entry.ageTime = (pppoep->age>0)?0x07:0;
#endif /*CONFIG_RTL865XB*/
	
	return _rtl8651_forceAddAsicEntry(TYPE_PPPOE_TABLE, index, &entry);
}

int32 rtl8651_getAsicPppoe(uint32 index, rtl865x_tblAsicDrv_pppoeParam_t *pppoep) {
	rtl8651_tblAsic_pppoeTable_t entry;

	if((index >= RTL8651_PPPOETBL_SIZE) || (pppoep == NULL))
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_PPPOE_TABLE, index, &entry);
	pppoep->sessionId = entry.sessionID;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	pppoep->age = entry.ageTime;
#endif /*CONFIG_RTL865XB*/

	return SUCCESS;
}



int32 rtl8651_setAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp)
{
    rtl8651_tblAsic_nextHopTable_t  entry;
    if((index >= RTL8651_NEXTHOPTBL_SIZE ) || (nextHopp == NULL))
        return FAILED;

	/* for debug
	rtlglue_printf("[%s:%d] rtl8651_setAsicNextHopTable(idx=%d,Row=%d,Col=%d,PPPIdx=%d,dvid=%d,IPIdx=%d,type=%d)\n",
		__FILE__,__LINE__,index, nextHopp->nextHopRow,nextHopp->nextHopColumn,nextHopp->pppoeIdx,
		nextHopp->dvid,nextHopp->extIntIpIdx,nextHopp->isPppoe);
	*/
    memset(&entry,0,sizeof(entry));
    entry.nextHop = (nextHopp->nextHopRow <<2) | nextHopp->nextHopColumn;
    entry.PPPoEIndex = nextHopp->pppoeIdx;
    entry.dstVid = nextHopp->dvid;
    entry.IPIndex = nextHopp->extIntIpIdx;
    entry.type = nextHopp->isPppoe==TRUE? 1: 0;
    return _rtl8651_forceAddAsicEntry(TYPE_NEXT_HOP_TABLE, index, &entry);
}

int32 rtl8651_getAsicNextHopTable(uint32 index, rtl865x_tblAsicDrv_nextHopParam_t *nextHopp) 
{
    rtl8651_tblAsic_nextHopTable_t  entry;
    if((index>=RTL8651_NEXTHOPTBL_SIZE) || (nextHopp == NULL))
        return FAILED;

    _rtl8651_readAsicEntry(TYPE_NEXT_HOP_TABLE, index, &entry);
    nextHopp->nextHopRow = entry.nextHop>>2;
    nextHopp->nextHopColumn = entry.nextHop&0x3;
    nextHopp->pppoeIdx = entry.PPPoEIndex;
    nextHopp->dvid = entry.dstVid;
    nextHopp->extIntIpIdx = entry.IPIndex;
    nextHopp->isPppoe = entry.type==1? TRUE: FALSE;
    return SUCCESS;
}


int32 rtl8651_setAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp) 
{
	uint32 i, asicMask;
	rtl865xc_tblAsic_l3RouteTable_t entry;
#ifdef FPGA
	if (index==2) index=6;
	if (index==3) index=7;
	if (index>=4 && index <=5) 
		rtlglue_printf("Out of range\n");
#endif	
	if((index >= RTL8651_ROUTINGTBL_SIZE) || (routingp == NULL))
		return FAILED;

	if (routingp->ipMask) {
		for(i=0; i<32; i++)
			if(routingp->ipMask & (1<<i)) break;
		asicMask = 31 - i;
	} else asicMask = 0;
    
	memset(&entry,0,sizeof(entry));
	entry.IPAddr = routingp->ipAddr;
	switch(routingp->process) {
	case 0://PPPoE
		entry.linkTo.PPPoEEntry.PPPoEIndex = routingp->pppoeIdx;
		entry.linkTo.PPPoEEntry.nextHop = (routingp->nextHopRow <<2) | routingp->nextHopColumn;
		entry.linkTo.PPPoEEntry.IPMask = asicMask;
		entry.linkTo.PPPoEEntry.netif = routingp->vidx;
		entry.linkTo.PPPoEEntry.internal=routingp->internal;
		entry.linkTo.PPPoEEntry.isDMZ=routingp->DMZFlag;
		entry.linkTo.PPPoEEntry.process = routingp->process;
		entry.linkTo.PPPoEEntry.valid = 1;

		break;
	case 1://Direct
		entry.linkTo.L2Entry.nextHop = (routingp->nextHopRow <<2) | routingp->nextHopColumn;
		entry.linkTo.L2Entry.IPMask = asicMask;
		entry.linkTo.L2Entry.netif = routingp->vidx;
		entry.linkTo.L2Entry.internal=routingp->internal;
		entry.linkTo.L2Entry.isDMZ=routingp->DMZFlag;
		entry.linkTo.L2Entry.process = routingp->process;
		entry.linkTo.L2Entry.valid = 1;		
		break;
	case 2://arp
		entry.linkTo.ARPEntry.ARPEnd = routingp->arpEnd >> 3;
		entry.linkTo.ARPEntry.ARPStart = routingp->arpStart >> 3;
		entry.linkTo.ARPEntry.IPMask = asicMask;
		entry.linkTo.ARPEntry.netif = routingp->vidx;
		entry.linkTo.ARPEntry.internal=routingp->internal;
		entry.linkTo.ARPEntry.isDMZ = routingp->DMZFlag;
		entry.linkTo.ARPEntry.process = routingp->process;
		entry.linkTo.ARPEntry.valid = 1;

		entry.linkTo.ARPEntry.ARPIpIdx = routingp->arpIpIdx; /* for RTL8650B C Version Only */
		break;
	case 4://CPU
	case 6://DROP
		/*
		  *   Note:  although the process of this routing entry is CPU/DROP,
		  *             we still have to assign "vid" field for packet decision process use.
		  *                                                                                          - 2005.3.23 -
		  */
		entry.linkTo.ARPEntry.netif = routingp->vidx;
		entry.linkTo.ARPEntry.IPMask = asicMask;
		entry.linkTo.ARPEntry.process = routingp->process;
		entry.linkTo.ARPEntry.valid = 1;
		entry.linkTo.ARPEntry.internal=routingp->internal;
		break;
	case 5://NAPT NextHop
		entry.linkTo.NxtHopEntry.nhStart = routingp->nhStart >> 1;
		switch (routingp->nhNum)
		{
		    case 2: entry.linkTo.NxtHopEntry.nhNum = 0; break;
		    case 4: entry.linkTo.NxtHopEntry.nhNum = 1; break;
		    case 8: entry.linkTo.NxtHopEntry.nhNum = 2; break;
		    case 16: entry.linkTo.NxtHopEntry.nhNum = 3; break;
		    case 32: entry.linkTo.NxtHopEntry.nhNum = 4; break;
		    default: return FAILED;
		}
		entry.linkTo.NxtHopEntry.nhNxt = routingp->nhNxt;
		entry.linkTo.NxtHopEntry.nhAlgo = routingp->nhAlgo;
		entry.linkTo.NxtHopEntry.IPMask = asicMask;
		entry.linkTo.NxtHopEntry.process = routingp->process;
		entry.linkTo.NxtHopEntry.valid = 1;
		entry.linkTo.NxtHopEntry.IPDomain = routingp->ipDomain;
		entry.linkTo.NxtHopEntry.internal = routingp->internal;
		entry.linkTo.NxtHopEntry.isDMZ = routingp->DMZFlag;
		break;
		
	default: 
		return FAILED;
	}
    return _rtl8651_forceAddAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
}

int32 rtl8651_delAsicRouting(uint32 index) 
{
	rtl865xc_tblAsic_l3RouteTable_t entry;

	if(index >= RTL8651_ROUTINGTBL_SIZE)
		return FAILED;
	memset(&entry,0,sizeof(entry));
	entry.linkTo.ARPEntry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
}

int32 rtl8651_getAsicRouting(uint32 index, rtl865x_tblAsicDrv_routingParam_t *routingp) 
{
	uint32 i;
	rtl865xc_tblAsic_l3RouteTable_t entry;
    
	if((index >= RTL8651_ROUTINGTBL_SIZE) || (routingp == NULL))
		return FAILED;

	_rtl8651_readAsicEntry(TYPE_L3_ROUTING_TABLE, index, &entry);
	if(entry.linkTo.ARPEntry.valid == 0)
		return FAILED;

	routingp->ipAddr = entry.IPAddr;
	routingp->process = entry.linkTo.ARPEntry.process;
	for(i=0, routingp->ipMask = 0; i<=entry.linkTo.ARPEntry.IPMask; i++)
		routingp->ipMask |= 1<<(31-i);
    
    	routingp->vidx = entry.linkTo.ARPEntry.netif;
	routingp->internal= entry.linkTo.PPPoEEntry.internal;
	switch(routingp->process) {
	case 0://PPPoE
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = entry.linkTo.PPPoEEntry.PPPoEIndex;
		routingp->nextHopRow = entry.linkTo.PPPoEEntry.nextHop>>2;
		routingp->nextHopColumn = entry.linkTo.PPPoEEntry.nextHop & 0x3;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
	case 1://Direct
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = entry.linkTo.L2Entry.nextHop>>2;
		routingp->nextHopColumn = entry.linkTo.L2Entry.nextHop&0x3;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
	case 2://Indirect
		routingp->arpEnd = entry.linkTo.ARPEntry.ARPEnd;
		routingp->arpStart = entry.linkTo.ARPEntry.ARPStart;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = 0;
		routingp->nextHopColumn = 0;
		routingp->arpIpIdx = entry.linkTo.ARPEntry.ARPIpIdx;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
	case 4: /* CPU */
	case 6: /* Drop */
		routingp->arpStart = 0;
		routingp->arpEnd = 0;
		routingp->pppoeIdx = 0;
		routingp->nextHopRow = 0;
		routingp->nextHopColumn = 0;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		break;
#if 1 //chhuang: #ifdef CONFIG_RTL865XB
	case 5:
		routingp->nhStart = (entry.linkTo.NxtHopEntry.nhStart) << 1;
		switch (entry.linkTo.NxtHopEntry.nhNum)
		{
		case 0: routingp->nhNum = 2; break;
		case 1: routingp->nhNum = 4; break;
		case 2: routingp->nhNum = 8; break;
		case 3: routingp->nhNum = 16; break;
		case 4: routingp->nhNum = 32; break;
		default: return FAILED;
		}
		routingp->nhNxt = entry.linkTo.NxtHopEntry.nhNxt;
		routingp->nhAlgo = entry.linkTo.NxtHopEntry.nhAlgo;
		routingp->ipDomain = entry.linkTo.NxtHopEntry.IPDomain;
		routingp->internal= entry.linkTo.NxtHopEntry.internal;
		routingp->DMZFlag= entry.linkTo.NxtHopEntry.isDMZ;
		 
		break;
#endif
	default: return FAILED;
	}
    return SUCCESS;
}


int32 rtl8651_setAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp) 
{
	rtl865xc_tblAsic_arpTable_t   entry;
	if((index >= RTL8651_ARPTBL_SIZE) || (arpp == NULL))
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.nextHop = (arpp->nextHopRow<<2) | (arpp->nextHopColumn&0x3);
	entry.valid = 1;
	entry.aging=arpp->aging;
	return _rtl8651_forceAddAsicEntry(TYPE_ARP_TABLE, index, &entry);
}

int32 rtl8651_delAsicArp(uint32 index) 
{
	rtl865xc_tblAsic_arpTable_t   entry;
	if(index >= RTL8651_ARPTBL_SIZE)
		return FAILED;

	memset(&entry,0,sizeof(entry));
	entry.valid = 0;
	return _rtl8651_forceAddAsicEntry(TYPE_ARP_TABLE, index, &entry);
}

int32 rtl8651_getAsicArp(uint32 index, rtl865x_tblAsicDrv_arpParam_t *arpp) 
{
	rtl865xc_tblAsic_arpTable_t   entry;
	if((index >= RTL8651_ARPTBL_SIZE) || (arpp == NULL))
		return FAILED;
	_rtl8651_readAsicEntry(TYPE_ARP_TABLE, index, &entry);
	/*for 8196B patch,read arp table and ip multicast table should stop TLU*/
	//_rtl8651_readAsicEntryStopTLU(TYPE_ARP_TABLE, index, &entry); //No need to Stop_Table_Lookup process 

	if(entry.valid == 0)
		return FAILED;
	arpp->nextHopRow = entry.nextHop>>2;
	arpp->nextHopColumn = entry.nextHop&0x3;
	arpp->aging=entry.aging&0x1f;
	return SUCCESS;
}

/*=========================================
  * ASIC DRIVER API: Multicast Table
  *=========================================*/

uint32 rtl8651_ipMulticastTableIndex(ipaddr_t srcAddr, ipaddr_t dstAddr)
{
	uint32 idx;
	#if defined(CONFIG_RTL_819X) || defined(CONFIG_RTL_8676HWNAT)
#if defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) 
	uint32 sip[32],dip[32];
	uint32 hash[8];
	uint32 i;

	for(i=0; i<8; i++) {
		hash[i]=0;
	}

	for(i=0; i<32; i++)	{
		if((srcAddr & (1<<i))!=0) {
			sip[i]=1;
		}
		else 	{
			sip[i]=0;
		}

		if((dstAddr & (1<<i))!=0) {
			dip[i]=1;
		}
		else {
			dip[i]=0;
		}			
	}

	hash[7] = sip[0] ^ sip[8]   ^ sip[16] ^ sip[24] ^ dip[7] ^ dip[15] ^ dip[23] ^ dip[31];
	hash[6] = sip[1] ^ sip[9]   ^ sip[17] ^ sip[25] ^ dip[6] ^ dip[14] ^ dip[22] ^ dip[30];
	hash[5] = sip[2] ^ sip[10] ^ sip[18] ^ sip[26] ^ dip[5] ^ dip[13] ^ dip[21] ^ dip[29];
	hash[4] = sip[3] ^ sip[11] ^ sip[19] ^ sip[27] ^ dip[4] ^ dip[12] ^ dip[20] ^ dip[28];
	hash[3] = sip[4] ^ sip[12] ^ sip[20] ^ sip[28] ^ dip[3] ^ dip[11] ^ dip[19] ^ dip[27];
	hash[2] = sip[5] ^ sip[13] ^ sip[21] ^ sip[29] ^ dip[2] ^ dip[10] ^ dip[18] ^ dip[26];
	hash[1] = sip[6] ^ sip[14] ^ sip[22] ^ sip[30] ^ dip[1] ^ dip[9]  ^ dip[17] ^ dip[25];
	hash[0] = sip[7] ^ sip[15] ^ sip[23] ^ sip[31] ^ dip[0] ^ dip[8]  ^ dip[16] ^ dip[24];

	for(i=0; i<8; i++) {
		hash[i]=hash[i] & (0x01);
	}

	idx=0;
	for(i=0; i<8; i++) {
		idx=idx+(hash[i]<<i);
	}
	
	return idx;
#else
	{
		#if 0
		uint32 sip[32],dip[32];
		uint32 hash[6];
		uint32 i;
		uint32 bitmask;

		for(i=0; i<32; i++)
		{
			bitmask=1<<i;
			if((srcAddr & bitmask)!=0)
			{
				sip[i]=1;
			}
			else
			{
				sip[i]=0;
			}

			if((dstAddr & bitmask)!=0)
			{
				dip[i]=1;
			}
			else
			{
				dip[i]=0;
			}
			
		}

		hash[0] = dip[0]^dip[6]^dip[12]^dip[18]^dip[24]^dip[26]^dip[28]^dip[30]^
	         		sip[23]^sip[5]^sip[11]^sip[17]^sip[31]^sip[25]^sip[27]^sip[29];
		hash[1] = dip[1]^dip[7]^dip[13]^dip[19]^dip[25]^dip[27]^dip[29]^dip[31]^
		         	sip[0]^sip[6]^sip[12]^sip[18]^sip[24]^sip[26]^sip[28]^sip[30];
		hash[2] = dip[2]^dip[8]^dip[14]^dip[20]^sip[1]^sip[7]^sip[13]^sip[19];
		hash[3] = dip[3]^dip[9]^dip[15]^dip[21]^sip[2]^sip[8]^sip[14]^sip[20];
		hash[4] = dip[4]^dip[10]^dip[16]^dip[22]^sip[3]^sip[9]^sip[15]^sip[21];
		hash[5] = dip[5]^dip[11]^dip[17]^dip[23]^sip[4]^sip[10]^sip[16]^sip[22];

		idx=0;
		for(i=0; i<6; i++)
		{
			hash[i]=hash[i] & (0x01);
			idx=idx+(hash[i]<<i);
		}
		#else
		uint32 hashSrcAddr;
		uint32 hashDstAddr;		
		hashDstAddr=((dstAddr ^ (dstAddr>>6) ^ (dstAddr>>12) ^ (dstAddr>>18)) ) ^ 
					(((dstAddr>>24) ^ (dstAddr>>26) ^ (dstAddr>>28) ^ (dstAddr>>30) ) & 0x03);
		
		hashSrcAddr=(	( 	(((srcAddr>>23) ^(srcAddr>>31)) & 0x01) 	^ 
							((srcAddr ^ (srcAddr >>24))<<1) 			^
							(srcAddr>>5) 	^
							(srcAddr>>11) 	^ 
							(srcAddr>>17) 	^ 
							(srcAddr>>25) 	^ 
							(srcAddr>>27) 	^ 
							(srcAddr>>29)	)	& 0x03	
					) 	|
					 (	(	(srcAddr>>1) 	^
					 		(srcAddr>>7) 	^
					 		(srcAddr>>13) 	^
					 		(srcAddr>>19)	)	<<2
					 );
					
					
		idx= (hashDstAddr ^ hashSrcAddr) & 0x3F; 
		#endif
	}
#endif
	#else
	#error "wrong compile flag, not supported IC reversion"
	{
		idx = srcAddr ^ (srcAddr>>8) ^ (srcAddr>>16) ^ (srcAddr>>24) ^ dstAddr ^ (dstAddr>>8) ^ (dstAddr>>16) ^ (dstAddr>>24);
		idx = ((idx >> 2) ^ (idx & 0x3)) & (RTL8651_IPMULTICASTTBL_SIZE-1);
	}
	#endif
	return idx;
}

int32 rtl8651_setAsicIpMulticastTable(rtl865x_tblAsicDrv_multiCastParam_t *mCast_t) {
	uint32 idx;
 	rtl865xc_tblAsic_ipMulticastTable_t entry;
	int16 age;

	if(mCast_t->dip >>28 != 0xe || mCast_t->port >= RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum)
		return FAILED;//Non-IP multicast destination address
	memset(&entry,0,sizeof(entry));
	entry.srcIPAddr 		= mCast_t->sip;
	entry.destIPAddrLsbs 	= mCast_t->dip & 0xfffffff;

	idx = rtl8651_ipMulticastTableIndex(mCast_t->sip, mCast_t->dip);

#if defined (CONFIG_RTL8196C_REVISION_B)  || defined (CONFIG_RTL8198_REVISION_B) 
	entry.srcPort 			= mCast_t->port;
	entry.portList 			= mCast_t->mbr;

#else

	if (mCast_t->port >= RTL8651_PORT_NUMBER) {
		/* extension port */
		entry.srcPortExt = 1;
		entry.srcPort 			= (mCast_t->port-RTL8651_PORT_NUMBER);

	} else {
		entry.srcPortExt = 0;
		entry.srcPort 			= mCast_t->port;

	}

	entry.extPortList 		= mCast_t->mbr >> RTL8651_PORT_NUMBER;
	entry.srcVidH 			= ((mCast_t->svid)>>4) &0xff;
	entry.srcVidL 			= (mCast_t->svid)&0xf;
	entry.portList 			= mCast_t->mbr & (RTL8651_PHYSICALPORTMASK);
#endif
	entry.toCPU 			= 0;
	entry.valid 			= 1;
	entry.extIPIndex 		= mCast_t->extIdx;
	
	entry.ageTime			= 0;
	age = (int16)mCast_t->age;
	while ( age > 0 ) {
		if ( (++entry.ageTime) == 7)
			break;
		age -= 5;
	}
	#ifdef CONFIG_RTL_HARDWARE_MULTICAST
	entry.ageTime = 7;
	#endif
	
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, idx, &entry);
}

int32 rtl8651_delAsicIpMulticastTable(uint32 index) {
	rtl865xc_tblAsic_ipMulticastTable_t entry;
	memset(&entry,0,sizeof(entry));
	return _rtl8651_forceAddAsicEntry(TYPE_MULTICAST_TABLE, index, &entry);
}

int32 rtl8651_getAsicIpMulticastTable(uint32 index, rtl865x_tblAsicDrv_multiCastParam_t *mCast_t) 
{	
	rtl865xc_tblAsic_ipMulticastTable_t entry;
	
	if (mCast_t == NULL)
		return FAILED;
   	_rtl8651_readAsicEntry(TYPE_MULTICAST_TABLE, index, &entry);
	//_rtl8651_readAsicEntryStopTLU(TYPE_MULTICAST_TABLE, index, &entry); //No need to Stop_Table_Lookup process 

 	mCast_t->sip	= entry.srcIPAddr;
	if(entry.valid)
	{
 		mCast_t->dip	= entry.destIPAddrLsbs | 0xe0000000;
	}
	else
	{
		if(entry.destIPAddrLsbs==0)
		{
			mCast_t->dip	= entry.destIPAddrLsbs;
		}
		else
		{
			mCast_t->dip	= entry.destIPAddrLsbs | 0xe0000000;
		}
	}

#if defined (CONFIG_RTL8196C_REVISION_B) || defined (CONFIG_RTL8198_REVISION_B) 
	mCast_t->svid = 0;
	mCast_t->port = entry.srcPort;
	mCast_t->mbr = entry.portList;
#else
	mCast_t->svid = (entry.srcVidH<<4) | entry.srcVidL;
#if 1
//#ifdef CONFIG_RTL8650BBASIC
#if 1
	if (entry.srcPortExt) {
		mCast_t->port = entry.srcPort + RTL8651_PORT_NUMBER;
	} else {
		mCast_t->port = entry.srcPort;
	}
#else
	mCast_t->port = entry.srcPortExt<<3 | entry.srcPortH<<1 | entry.srcPortL;
#endif
	mCast_t->mbr = entry.extPortList<<RTL8651_PORT_NUMBER | entry.portList;
#else
	mCast_t->port = entry.srcPortH<<1 | entry.srcPortL;
	mCast_t->mbr = entry.portList;
#endif
#endif
	mCast_t->extIdx = entry.extIPIndex ;
	mCast_t->age	= entry.ageTime * 5;
	mCast_t->cpu = entry.toCPU;
	
	if(entry.valid == 0)
	{
		return FAILED;

	}

	return SUCCESS;
	
}


/*
@func int32		|	rtl8651_setAsicMulticastEnable 	| Enable / disable ASIC IP multicast support.
@parm uint32		|	enable	| TRUE to indicate ASIC IP multicast process is enabled; FALSE to indicate ASIC IP multicast process is disabled.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
We would use this API to enable/disable ASIC IP multicast process.
If it's disabled here, Hardware IP multicast table would be ignored.
If it's enabled here, IP multicast table is used to forwarding IP multicast packets.
 */
int32 rtl8651_setAsicMulticastEnable(uint32 enable)
{
	if (enable == TRUE)
	{
		//0xBB804428, Frame Forwarding Configuration Register
		WRITE_MEM32(FFCR, READ_MEM32(FFCR)|EN_MCAST);
	} else
	{
		WRITE_MEM32(FFCR, READ_MEM32(FFCR) & ~EN_MCAST);
	}

	return SUCCESS;
}

/*
@func int32		|	rtl8651_getAsicMulticastEnable 	| Get the state about ASIC IP multicast support.
@parm uint32*		|	enable	| Pointer to store the state about ASIC IP multicast support.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
We would use this API to get the status of ASIC IP multicast process.
TRUE to indicate ASIC IP multicast process is enabled; FALSE to indicate ASIC IP multicast process is disabled.
*/
int32 rtl8651_getAsicMulticastEnable(uint32 *enable)
{
	if (enable == NULL)
	{
		return FAILED;
	}

	*enable = (READ_MEM32(FFCR) & EN_MCAST) ? TRUE : FALSE;

	return SUCCESS;
}

/*
@func int32		|	rtl8651_setAsicMulticastPortInternal 	| Configure internal/external state for each port
@parm uint32		|	port		| Port to set its state.
@parm int8		|	isInternal	| set TRUE to indicate <p port> is internal port; set FALSE to indicate <p port> is external port
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
In RTL865x platform,
user would need to config the Internal/External state for each port to support HW multicast NAPT.
If packet is sent from internal port to external port, and source VLAN member port checking indicates that L34 is needed.
Source IP modification would be applied.
 */
int32 rtl8651_setAsicMulticastPortInternal(uint32 port, int8 isInternal)
{
	if (port >= RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum)
	{	/* Invalid port */
		return FAILED;
	}

	/*
		RTL865XC : All multicast mode are stored in [SWTCR0 / Switch Table Control Register 0]
	*/
	if (isInternal == FALSE)
	{//external
		//0xBB804418, Switch Table Control Register 0
		WRITE_MEM32(SWTCR0, READ_MEM32(SWTCR0) | (((1 << port) & MCAST_PORT_EXT_MODE_MASK) << MCAST_PORT_EXT_MODE_OFFSET));
	} else
	{//internal
		WRITE_MEM32(SWTCR0, READ_MEM32(SWTCR0) & ~(((1 << port) & MCAST_PORT_EXT_MODE_MASK) << MCAST_PORT_EXT_MODE_OFFSET));
	}

	return SUCCESS;
}

/*
@func int32		|	rtl8651_getAsicMulticastPortInternal 	| Get internal/external state for each port
@parm uint32		|	port		| Port to set its state.
@parm int8*		|	isInternal	| Pointer to get port state of <p port>.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
To get the port internal / external state for <p port>:
TRUE to indicate <p port> is internal port; FALSE to indicate <p port> is external port
 */
int32 rtl8651_getAsicMulticastPortInternal(uint32 port, int8 *isInternal)
{
	if (isInternal == NULL)
	{
		return FAILED;
	}

	if (port >= RTL8651_PORT_NUMBER + rtl8651_totalExtPortNum)
	{	/* Invalid port */
		return FAILED;
	}

	if (READ_MEM32(SWTCR0) & (((1 << port) & MCAST_PORT_EXT_MODE_MASK) << MCAST_PORT_EXT_MODE_OFFSET))
	{
		*isInternal = TRUE;
	} else
	{
		*isInternal = FALSE;
	}

	return SUCCESS;
}

/*
@func int32		|	rtl8651_setAsicMulticastMTU 	| Set MTU for ASIC IP multicast forwarding
@parm uint32		|	mcastMTU	| MTU used by HW IP multicast forwarding.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
To set the MTU for ASIC IP multicast forwarding.
Its independent from other packet forwarding because IP multicast would include L2/L3/L4 at one time.
*/
int32 rtl8651_setAsicMulticastMTU(uint32 mcastMTU)
{
	if (mcastMTU & ~(MultiCastMTU_MASK) )
	{	/* multicast MTU overflow */
		return FAILED;
	}

	//0xBB80440C, ALE Control Register
	UPDATE_MEM32(ALECR, mcastMTU, MultiCastMTU_MASK, MultiCastMTU_OFFSET);

	return SUCCESS;
}

/*
@func int32		|	rtl8651_setAsicMulticastMTU 	| Get MTU for ASIC IP multicast forwarding
@parm uint32*	|	mcastMTU	| Pointer to get MTU used by HW IP multicast forwarding.
@rvalue FAILED	|	Failed
@rvalue SUCCESS	|	Success
@comm
To get the MTU value for ASIC IP multicast forwarding.
Its independent from other packet forwarding because IP multicast would include L2/L3/L4 at one time.
*/
int32 rtl8651_getAsicMulticastMTU(uint32 *mcastMTU)
{
	if (mcastMTU == NULL)
	{
		return FAILED;
	}

	*mcastMTU = GET_MEM32_VAL(ALECR, MultiCastMTU_MASK, MultiCastMTU_OFFSET);

	return SUCCESS;
}

int32 rtl865x_setAsicMulticastAging(uint32 enable)
{
	if (enable == TRUE)
	{
		//0xBB804400, multicast entry aging.
		WRITE_MEM32(TEACR, READ_MEM32(TEACR) & ~IPMcastAgingDisable);
		
	} else
	{
		WRITE_MEM32(TEACR, READ_MEM32(TEACR)|IPMcastAgingDisable);
	}

	return SUCCESS;
}

int32 rtl865x_initAsicL3(void)
{
	int32 index;
	rtl865x_tblAsicDrv_routingParam_t rth;
	
	/*clear asic table*/
	rtl8651_clearSpecifiedAsicTable(TYPE_EXT_INT_IP_TABLE, RTL8651_IPTABLE_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_ARP_TABLE, RTL8651_ARPTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_PPPOE_TABLE, RTL8651_PPPOETBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_NEXT_HOP_TABLE, RTL8651_NEXTHOPTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_L3_ROUTING_TABLE, RTL8651_ROUTINGTBL_SIZE);
	rtl8651_clearSpecifiedAsicTable(TYPE_MULTICAST_TABLE, RTL8651_IPMULTICASTTBL_SIZE);

	rtl8651_setAsicMulticastEnable(TRUE); /* Enable multicast table */
	//0xBB80440C, Enable PPPoE auto encap.
	WRITE_MEM32(ALECR, READ_MEM32(ALECR)|(uint32)EN_PPPOE);//enable PPPoE auto encapsulation
	rtl8651_setAsicMulticastMTU(1522);

	//0xBB80440C, Enable TTL-1
	WRITE_MEM32(TTLCR,READ_MEM32(TTLCR)|(uint32)EN_TTL1);//Don't hide this router. enable TTL-1 when routing on this gateway.

	for (index=0; index<RTL8651_PORT_NUMBER+rtl8651_totalExtPortNum; index++) 
	{		
		if( rtl8651_setAsicMulticastPortInternal(index, TRUE) )
			return FAILED;		
	}

	{
		memset(&rth, 0, sizeof(rtl865x_tblAsicDrv_routingParam_t));
		rth.process = 0x4; /* CPU */
		rth.ipAddr = 0;
		rth.ipMask = 0;
		rth.vidx = 0;
		rth.internal = 0;
		rtl8651_setAsicRouting(7, &rth);
	}

	return SUCCESS;
	
}


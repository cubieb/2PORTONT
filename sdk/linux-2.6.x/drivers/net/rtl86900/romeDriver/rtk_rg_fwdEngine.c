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
*/



/*
 * Include Files
 */

#if defined(CONFIG_APOLLO_ROMEDRIVER) || defined(CONFIG_XDSL_ROMEDRIVER)

//MOVE TO RTK_RG_DEFINE_H
//#define CONFIG_ROME_NAPT_SHORTCUT

#ifdef CONFIG_APOLLO_MODEL
#else
#include <linux/slab.h> //for kmalloc/kfree
#endif

#ifdef __KERNEL__

#include <linux/interrupt.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/in.h>
#include <linux/if_vlan.h>

#else
unsigned long int jiffies=0;
#endif





#ifdef CONFIG_APOLLO_MODEL

#include "utility.h"
#include "rtl_types.h"
#include <rtl_glue.h>
#include <rtl_utils.h>
//#include <re8686.h>
#include <time.h>		//for time in fragment queuing
#endif

#include <rtk_rg_fwdEngine.h>

#if defined(CONFIG_APOLLO)
#if 0//def CONFIG_RG_LAYER2_SOFTWARE_LEARN
#include <rtk/intr.h>		//for get and clear link-down indicator register
#endif
#include <common/error.h>
#include <rtk/init.h>
#include <rtk/l34.h>
#elif defined(CONFIG_XDSL_ROMEDRIVER)
#include <rtk_rg_xdsl_extAPI.h>
#include <net/rtl/rtl_nic.h>
#define CONFIG_RG_ACCESSWAN_TIMER_DELAY 100

#endif


//#ifdef CONFIG_RG_DEBUG
#include <rtk_rg_debug.h>
//#endif

/*siyuan add for alg function*/
#include <rtk_rg_ipsec.h>
#include <rtk_rg_sip.h>
#include <rtk_rg_ftp.h>
#include <rtk_rg_pptp.h>
#include <rtk_rg_alg_tool.h>


#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
#include <rtk_rg_igmpsnooping.h>
extern struct rtl_mCastTimerParameters rtl_mCastTimerParas; 
extern struct rtl_multicastModule rtl_mCastModuleArray[MAX_MCAST_MODULE_NUM]; 
extern int32 rtl_compareMacAddr(uint8* macAddr1, uint8* macAddr2);
extern int32 rtl_compareIpv6Addr(uint32* ipv6Addr1, uint32* ipv6Addr2);
extern int32 rtl_checkMCastAddrMapping(uint32 ipVersion, uint32 *ipAddr, uint8* macAddr);
#endif
static void _rtk_rg_igmpPacketParser(int off, u8 *pData, rtk_rg_pktHdr_t *pPktHdr);
static void _rtk_rg_mospfPacketParser(int off, u8 *pData, rtk_rg_pktHdr_t *pPktHdr);
static void _rtk_rg_pimfPacketParser(int off, u8 *pData, rtk_rg_pktHdr_t *pPktHdr);
static void _rtk_rg_mldPacketParser(int off, u8 *pData, rtk_rg_pktHdr_t *pPktHdr);

#ifdef CONFIG_RG_WMUX_SUPPORT
#include <rtk_rg_wmux.h>
#endif

#ifdef __KERNEL__
#else
int model_nic_rx_skb (struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo)
{
	printf("FIXME %s %d\n",__FUNCTION__,__LINE__);
	return 0;
}

#endif

#ifdef CONFIG_DUALBAND_CONCURRENT
static rtk_mac_t master_ipc_macAddr;
//static rtk_mac_t slave_ipc_macAddr;
#endif

/*void _rtk_rg_fwdEngineTxDescSetting(void *pTxInfoPtr,void *ptxPtr,void *ptxMaskPtr)
{
	rtk_rg_txdesc_t *pTxInfo=(rtk_rg_txdesc_t *)pTxInfoPtr;
	rtk_rg_txdesc_t *ptx=(rtk_rg_txdesc_t *)ptxPtr;
	rtk_rg_txdesc_t *ptxMask=(rtk_rg_txdesc_t *)ptxMaskPtr;	

	if(ptxMask == NULL)
	{
		pTxInfo->opts1.bit.ipcs = ptx->opts1.bit.ipcs;
		pTxInfo->opts1.bit.l4cs = ptx->opts1.bit.l4cs;
		pTxInfo->opts1.bit.cputag_ipcs = ptx->opts1.bit.cputag_ipcs;
		pTxInfo->opts1.bit.cputag_l4cs = ptx->opts1.bit.cputag_l4cs;
		pTxInfo->opts2.dw = ptx->opts2.dw;
		pTxInfo->opts3.dw = ptx->opts3.dw;
		pTxInfo->opts4.dw = ptx->opts4.dw;
	}
	else
	{
		if(ptxMask->opts1.dw)
		{
			pTxInfo->opts1.dw &= (~ptxMask->opts1.dw);
			pTxInfo->opts1.dw |= (ptx->opts1.dw & ptxMask->opts1.dw);
		}
		if(ptxMask->opts2.dw)
		{
			pTxInfo->opts2.dw &= (~ptxMask->opts2.dw);
			pTxInfo->opts2.dw |= (ptx->opts2.dw & ptxMask->opts2.dw);
		}
		if(ptxMask->opts3.dw)
		{
			pTxInfo->opts3.dw &= (~ptxMask->opts3.dw);
			pTxInfo->opts3.dw |= (ptx->opts3.dw & ptxMask->opts3.dw);
		}
		if(ptxMask->opts4.dw)
		{
			pTxInfo->opts4.dw &= (~ptxMask->opts4.dw);
			pTxInfo->opts4.dw |= (ptx->opts4.dw & ptxMask->opts4.dw);
		}
	}
}*/
rtk_rg_fwdEngineReturn_t _rtk_rg_dslite_multicast_unmatch_check(void)
{
	//Check unmatch action
	switch(rg_db.systemGlobal.dsliteControlSet[L34_DSLITE_CTRL_MC_PREFIX_UNMATCH]){
		case RTK_L34_DSLITE_UNMATCH_ACT_DROP:
			TRACE("dslite multicast unmatch...DROP!");
			return RG_FWDENGINE_RET_DROP;
		case RTK_L34_DSLITE_UNMATCH_ACT_TRAP:
			TRACE("dslite multicast unmatch...TRAP!");
			return RG_FWDENGINE_RET_TO_PS;
		default:
			WARNING("dslite multicast unmatch action error...%d",rg_db.systemGlobal.dsliteControlSet[L34_DSLITE_CTRL_MC_PREFIX_UNMATCH]);
			return RG_FWDENGINE_RET_DROP;
	}
}

rtk_rg_fwdEngineReturn_t _rtk_rg_dslite_multicast_v4Check(rtk_rg_pktHdr_t *pPktHdr)
{
	if(*((unsigned int *)(pPktHdr->pIpv6Dip+12))==pPktHdr->ipv4Dip && 
		*((unsigned int *)(pPktHdr->pIpv6Sip+12))==pPktHdr->ipv4Sip){
		TRACE("IPv4 address also match!!");
		return RG_FWDENGINE_RET_CONTINUE;
	}
	return _rtk_rg_dslite_multicast_unmatch_check();
}

rtk_rg_fwdEngineReturn_t _rtk_rg_dslite_multicast_v6Check(rtk_rg_pktHdr_t *pPktHdr)
{
	int i,j;

	for(i=0;i<MAX_DSLITEMC_SW_TABLE_SIZE;i++){
		for(j=0;j<IPV6_ADDR_LEN;j++){
			if(rg_db.dsliteMc[i].ipMPrefix64_AND_mask.ipv6_addr[j]!=(rg_db.dsliteMc[i].rtk_dsliteMc.ipMPrefix64Mask.ipv6_addr[j]&pPktHdr->pIpv6Dip[j]))break;
			if(rg_db.dsliteMc[i].ipUPrefix64_AND_mask.ipv6_addr[j]!=(rg_db.dsliteMc[i].rtk_dsliteMc.ipUPrefix64Mask.ipv6_addr[j]&pPktHdr->pIpv6Sip[j]))break;
		}
		if(j==IPV6_ADDR_LEN){
			//Match!!
			TRACE("Match dsliteMc[%d]!",i);
			pPktHdr->tagif&=(~IPV6_TAGIF);
			pPktHdr->tagif|=DSLITEMC_INNER_TAGIF;
			return RG_FWDENGINE_RET_CONTINUE;
		}
	}
	return _rtk_rg_dslite_multicast_unmatch_check();
}

__IRAM_FWDENG
rtk_rg_fwdEngineReturn_t _rtk_rg_packetParser(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	u8 *pData=skb->data;
	u32 len=skb->len;

    int i,off,v6HeaderOff,tmpOff;
    u8 protocol=0;
	rtk_rg_pptpMsgHead_t *pPPTPHdr;
	unsigned int pptpMagic;
	rtk_rg_wanIntfInfo_t *pWan_intf;
	uint32 ret = RG_FWDENGINE_RET_CONTINUE;

	if(len==0)
	{
		rtlglue_printf("FIXME at %s %d\n",__FUNCTION__,__LINE__);
		return RG_FWDENGINE_RET_DROP;
	}	

	//init
	pPktHdr->tagif=0;
	pPktHdr->egressTagif=0;
	pPktHdr->overMTU=0;
	pPktHdr->shortcutStatus=RG_SC_NORMAL_PATH;
	
	pPktHdr->pDmac=&pData[0]; //DA
#if defined(CONFIG_RTL9602C_SERIES)
	memcpy(pPktHdr->dmac,pData,ETHER_ADDR_LEN);
#endif
	pPktHdr->pSmac=&pData[6]; //SA
	if(rg_db.systemGlobal.fwdStatistic)
	{
		int reason=pPktHdr->pRxDesc->opts3.bit.reason;
		if((pData[0]&0x1)!=0)
		{
			if(pData[0]==0xff)
				rg_db.systemGlobal.statistic.perPortCnt_broadcast[pPktHdr->ingressPort]++;
			else
				rg_db.systemGlobal.statistic.perPortCnt_multicast[pPktHdr->ingressPort]++;			
		}
		else
		{
			rg_db.systemGlobal.statistic.perPortCnt_unicast[pPktHdr->ingressPort]++;
		}

		if((reason<256)&&(pPktHdr->ingressPort<RTK_RG_EXT_PORT2))
			rg_db.systemGlobal.statistic.perPortCnt_Reason[reason][pPktHdr->ingressPort]++;

	}
	
    off=12;

#if 0 //cpu tag will parse by GMAC	
    if((pData[off]==0x88)&&(pData[off+1]==0x99)) //CPU TAG
    {

        if(((pData[off+8]==0x88)&&(pData[off+9]==0xa8))||((pData[off+8]==0x81)&&(pData[off+9]==0x00))||((pData[off+8]==0x88)&&((pData[off+9]==0x63)||(pData[off+9]==0x64)))||
                ((pData[off+8]==0x86)&&(pData[off+9]==0xdd))||((pData[off]==0x08)&&(pData[off+1]==0x00)))
        {
            //TO CPU
            off+=8;
        }
        else
        {
            //FROM CPU
            off+=12;
        }
    }
#endif	
#if defined(CONFIG_RTL9600_SERIES) 
	//patch for ingress unStag but CF add Stag case: del Stag(svid=0, spri=0) : chuck
	if(((*(u16*)(pData+off))==htons(rg_db.systemGlobal.tpid)) && ((*(u16*)(pData+off+2))==0x0)){
		//DEBUG("####packetLen(before del Stag)=%d####\n",len);
		//memDump(skb->data,len,"before pkt remove stag:");

		if(!_vlan_remove_tag(pPktHdr,skb,rg_db.systemGlobal.tpid)){
				rtlglue_printf("FIXME at %s %d\n",__FUNCTION__,__LINE__);
				return RG_FWDENGINE_RET_DROP;
		}

		
		len -= 4; //sync this local varibal
//		off+=4;	//skip parsing the tag into pktHdr

		pData=skb->data; //sync this local varibal
		pPktHdr->pDmac=&skb->data[0]; //DA, fix the pointer
		pPktHdr->pSmac=&skb->data[6]; //SA, fix the pointer
		
		//DEBUG("####packetLen(after del Stag)=%d####\n",len);
		//memDump(skb->data,len,"after pkt remove stag:");
	}
	
#endif

#if defined(CONFIG_APOLLO)
#if 1 //CVLAN/SVLAN Tag is removed by GMAC, so it will parsed by _rtk_rg_ingressVlanDecision(). the value should comes from CPUTag.
	//if((*(u16*)(pData+off))==htons(0x88a8))//STAG
    //if((pData[off]==0x88)&&(pData[off+1]==0xa8)) //STAG
    //if((*(u16*)(pData+off))==htons(rg_db.systemGlobal.tpid) && (*(u16*)(pData+off+4))==htons(0x8100))//STAG "MUST" outside of CTAG
    //20140502: if tpid is 8100 & only one tag, this tag is STAG.(follow HW behavior)
#if defined(CONFIG_RTL9600_SERIES)
    if((rg_db.systemGlobal.internalSupportMask & RTK_RG_INTERNAL_SUPPORT_BIT1))
#endif
	{
	    if((pPktHdr->ingressPort<=RTK_RG_PORT_CPU)&&(rg_db.systemGlobal.service_pmsk.portmask&(1<<pPktHdr->ingressPort)))
		{
		    if((*(u16*)(pData+off))==htons(rg_db.systemGlobal.tpid))
		    {
				pPktHdr->tagif|=SVLAN_TAGIF;
		    	pPktHdr->pSVlanTag=&pData[off];
				pPktHdr->stagPri=pData[off+2]>>5;
				pPktHdr->stagDei=(pData[off+2]>>4)&1;
				pPktHdr->stagVid=((pData[off+2]&0xf)<<8)|(pData[off+3]);
				pPktHdr->stagTpid= ntohs(*(u16*)(pData+off));
		        off+=4;
		    }
#if defined(CONFIG_RTL9602C_SERIES)
			else if( (rg_db.systemGlobal.tpid2_en==1)&&(*(u16*)(pData+off))==htons(rg_db.systemGlobal.tpid2))
			{
				pPktHdr->tagif|=SVLAN_TAGIF;
				pPktHdr->pSVlanTag=&pData[off];
				pPktHdr->stagPri=pData[off+2]>>5;
				pPktHdr->stagDei=(pData[off+2]>>4)&1;
				pPktHdr->stagVid=((pData[off+2]&0xf)<<8)|(pData[off+3]);
				pPktHdr->stagTpid= ntohs(*(u16*)(pData+off));
				off+=4;
			}
#endif			
	    }	
    }
#endif
#endif

#if 1
	if((*(u16*)(pData+off))==htons(0x8100))//CTAG
	//if((pData[off]==0x81)&&(pData[off+1]==0x00)) //CTAG
	{	
		pPktHdr->tagif|=CVLAN_TAGIF;        
		pPktHdr->pCVlanTag=&pData[off];
		pPktHdr->ctagVid=((pData[off+2]&0xf)<<8)|(pData[off+3]);		
		pPktHdr->ctagPri=pData[off+2]>>5;
		pPktHdr->ctagCfi=(pData[off+2]>>4)&1;
		off+=4;
	}
#endif

	pPktHdr->etherType=ntohs(*(u16 *)&pData[off]);
	if(((*(u16*)(pData+off))==htons(0x8863))||((*(u16*)(pData+off))==htons(0x8864)))//PPPoE
	//if((pData[off]==0x88)&&((pData[off+1]==0x63)||(pData[off+1]==0x64))) //PPPoE
	{
   		pPktHdr->tagif|=PPPOE_TAGIF;    	
		pPktHdr->sessionId=ntohs(*(u16*)&pData[off+4]);
		pPktHdr->pPppoeLength=(u16*)&pData[off+6];
		off+=8;
		pPktHdr->pppProtocal = (*(u16*)(pData+off));	//IPv4:0x0021 or 0xC021, IPv6:0x0057 or 0xC057 
	}
	
TUUNEL_INNER:
	if(((*(u16*)(pData+off))==htons(0x0800))||((*(u16*)(pData+off))==htons(0x0021))||(pData[off]==0x21))//IPv4 or IPv4 with PPPoE
	//if(((pData[off]==0x08)&&(pData[off+1]==0x00))||((pData[off]==0x00)&&(pData[off+1]==0x21))) //IPv4 or IPv4 with PPPoE
	{
		//TRACE("parsing IPv4");
#if 0	
        rtlglue_printf("IPv4:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "HLen" COLOR_NM "=%d][" COLOR_Y "TOS" COLOR_NM "=%d(DSCP=%d)][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "ID" COLOR_NM "=%d][" COLOR_Y "R" COLOR_NM "=%d," COLOR_Y "DF" COLOR_NM "=%d," COLOR_Y "MF" COLOR_NM "=%d]\n"
                       ,pData[off+2]>>4,(pData[off+2]&0xf)*4,pData[off+3],pData[off+3]>>2,(pData[off+4]<<8)|pData[off+5],(pData[off+6]<<8)|pData[off+7]
                       ,(pData[off+8]>>7)&1,(pData[off+8]>>6)&1,(pData[off+8]>>5)&1);
        rtlglue_printf("     [" COLOR_Y "FrgOff" COLOR_NM "=%d][" COLOR_Y "TTL" COLOR_NM "=%d][" COLOR_Y "PROTO" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n"
                       ,((pData[off+8]&0x1f)<<8)|pData[off+9],pData[off+10],pData[off+11],(pData[off+12]<<8)|pData[off+13]);
#endif
		//20150420LUKE: ppp protocol field could be 8bits or 16bits
		if(pData[off]==0x21)off-=1;
DSLITE_INNER:
		pPktHdr->l3Offset=off+2;

		pPktHdr->ipv4HeaderLen = (pData[off+2]&0xf)<<2;		//IHL multiply 32 equals how many bits, so multiply 4 equals to bytes!
		pPktHdr->tos = pData[off+3];
		pPktHdr->pTos = (u8*)&pData[off+3];

		if((pData[off+8]>>6)&1)
			pPktHdr->ipv4DontFragment=1;	//DF=1
		else
			pPktHdr->ipv4DontFragment=0;
		if((pData[off+8]>>5)&1)
			pPktHdr->ipv4MoreFragment=1; //MF=1
		else 
			pPktHdr->ipv4MoreFragment=0;
		pPktHdr->ipv4FragmentOffset=((pData[off+8]&0x1f)<<8)|pData[off+9];	//Fragment Offset
		if((pPktHdr->ipv4MoreFragment!=0)||(pPktHdr->ipv4FragmentOffset!=0)) 
			pPktHdr->ipv4FragPacket=1;
		else
			pPktHdr->ipv4FragPacket=0;
		
		pPktHdr->l3Len=(pData[off+4]<<8)|pData[off+5];
		pPktHdr->pL3Len=(u16*)&pData[off+4];

		//if(pPktHdr->ipv4FragPacket) 
			pPktHdr->pIpv4Identification=(u16*)&pData[off+6];
		pPktHdr->ipv4TTL=pData[off+10];
		pPktHdr->pIpv4TTL=(u8*)&pData[off+10];
		pPktHdr->tagif|=IPV4_TAGIF;
		pPktHdr->ipv4Sip=ntohl(*(u32*)&pData[off+14]);
		pPktHdr->ipv4Dip=ntohl(*(u32*)&pData[off+18]);
		pPktHdr->pIpv4Sip=(u32*)&pData[off+14];
		pPktHdr->pIpv4Dip=(u32*)&pData[off+18];		
		pPktHdr->ipv4Checksum=ntohs(*(u16*)&pData[off+12]);
		pPktHdr->pIpv4Checksum=(u16*)&pData[off+12];

		protocol=pData[off+11];

		//proc/rg/igmp_trap_to_PS : trap igmp packet original to PS 
		if((rg_db.systemGlobal.igmp_Trap_to_PS_enable)&&(pPktHdr->pRxDesc->opts3.bit.dst_port_mask!=0x20)){
			if(protocol==0x2){//igmp
				TRACE("TRAP Igmp to PS");
				ret = RG_FWDENGINE_RET_ACL_TO_PS;
			}
		}
		
		//20151007LUKE: check for dslite multicast's IPv4 address!
		if(pPktHdr->tagif&DSLITEMC_INNER_TAGIF){
			int dsliteMc_ret=_rtk_rg_dslite_multicast_v4Check(pPktHdr);
			if(dsliteMc_ret!=RG_FWDENGINE_RET_CONTINUE) return dsliteMc_ret;
		}

		off+=((pData[off+2]&0xf)<<2)+2;
	}
	else if(((*(u16*)(pData+off))==htons(0x86dd))||((*(u16*)(pData+off))==htons(0x0057))||(pData[off]==0x57))//IPv6 or IPv6 with PPPoE
    //else if(((pData[off]==0x86)&&(pData[off+1]==0xdd)) || ((pData[off]==0x00)&&(pData[off+1]==0x57)))		//IPv6 or IPv6 with PPPoE
	{
		//TRACE("parsing IPv6");
		//20150420LUKE: ppp protocol field could be 8bits or 16bits
		if(pData[off]==0x57)off-=1;
#if 0		
        rtlglue_printf("IPv6:[" COLOR_Y "Ver" COLOR_NM "=%d][" COLOR_Y "TC" COLOR_NM "=%02x][" COLOR_Y "FL" COLOR_NM "=%02x%02x%x][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "NxHdr" COLOR_NM "=%d][" COLOR_Y "HopLimit" COLOR_NM "=%d]\n"
                       ,pData[off+2]>>4, (pData[off+2]&0xf)+(pData[off+3]>>4), (pData[off+3]&0xf)+(pData[off+4]>>4), (pData[off+4]&0xf)+(pData[off+5]>>4), (pData[off+5]&0xf), pData[off+6]+pData[off+7], pData[off+8], pData[off+9]);
#endif
		pPktHdr->tagif|=IPV6_TAGIF;
		
		pPktHdr->l3Offset=off+2;
		pPktHdr->pTos=(u8*)&pData[off+2];		//IPv6's traffic class is between version and flow label
		
		pPktHdr->ipv6PayloadLen=(pData[off+6]<<8)|pData[off+7];
		pPktHdr->pIPv6HopLimit=&pData[off+9];
		pPktHdr->pIpv6Sip=&pData[off+10];
		pPktHdr->pIpv6Dip=&pData[off+26];
		
	        protocol=pData[off+8];
		pPktHdr->l3Len=42;		//header length plus ether type
		
		off+=pPktHdr->l3Len;
		pPktHdr->l3Len+=pPktHdr->ipv6PayloadLen-2;
		
MORE_IPV6_EXT:
		//process extension headers
		switch(protocol)
		{
			case 4:		//IP in IP
				//20150108LUKE: check for dslite WAN
				if((pPktHdr->tagif&DSLITE_INNER_TAGIF)==0){
					if(pPktHdr->pIpv6Dip[0]==0xff){
						//20151007LUKE: check for dslite multicast's IPv6 address!
						int dsliteMc_ret=_rtk_rg_dslite_multicast_v6Check(pPktHdr);
						if(dsliteMc_ret!=RG_FWDENGINE_RET_CONTINUE) return dsliteMc_ret;
						off-=2; //minus ethertype
						goto DSLITE_INNER;
					}else{
						//Check if match DSlite WAN's IP and gateway IP
						for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
						{
							pWan_intf=&rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf;
							if(pWan_intf->wan_intf_conf.wan_type==RTK_RG_DSLITE &&
								(memcmp(pPktHdr->pIpv6Dip,pWan_intf->dslite_info.rtk_dslite.ipB4.ipv6_addr,IPV6_ADDR_LEN)==0) &&
								(memcmp(pPktHdr->pIpv6Sip,pWan_intf->dslite_info.rtk_dslite.ipAftr.ipv6_addr,IPV6_ADDR_LEN)==0)){
								TRACE("Match DSLITE[%d]!",i);
								if(pPktHdr->tagif&V6FRAG_TAGIF)return RG_FWDENGINE_RET_TO_PS;   //frag to PS
								pPktHdr->tagif&=(~IPV6_TAGIF);
								pPktHdr->tagif|=DSLITE_INNER_TAGIF;
								off-=2; //minus ethertype
								break;
							}
							else if((pPktHdr->tagif&PPPOE_TAGIF) && pWan_intf->wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE &&
								(memcmp(pPktHdr->pIpv6Dip,pWan_intf->pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr,IPV6_ADDR_LEN)==0) &&
								(memcmp(pPktHdr->pIpv6Sip,pWan_intf->pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr,IPV6_ADDR_LEN)==0)){
								TRACE("Match PPPoE_DSLITE[%d]!",i);
								if(pPktHdr->tagif&V6FRAG_TAGIF)return RG_FWDENGINE_RET_TO_PS;   //frag to PS
								pPktHdr->tagif&=(~IPV6_TAGIF);
								pPktHdr->tagif|=DSLITE_INNER_TAGIF;
								off-=2; //minus ethertype
								break;
							}
						}
						if(pPktHdr->tagif&DSLITE_INNER_TAGIF)goto DSLITE_INNER;
					}
				}
				break;
			case 0:		//Hop-by-hop
			{
				//uint8 optionType;
				//uint8 optionDataLen;
				TRACE("IPv6 Hop-by-hop. Trap to protocol stack if L3/L4");
				pPktHdr->tagif|=V6TRAP_TAGIF;
				
				protocol=pData[off];
				v6HeaderOff=(pData[off+1]+1)<<3;	//length didn't count the first 8 octets, so we have to add 1 here
				//pPktHdr->l3Len+=(v6HeaderOff+2);	//pPktHdr->l3Len+=2;
				//off+=(v6HeaderOff+2);	//off+=2
				pPktHdr->l3Len+=v6HeaderOff;	//pPktHdr->l3Len+=2;
				off+=v6HeaderOff;	//off+=2
				#if 0
				/* check all the type/length of hop-by-hop options are correct */
				while (off - pPktHdr->l3Offset < v6HeaderOff)
				{
					//optionType=pData[off]; switch (optionType)
					switch (pData[off])
					{
						case 0: /*pad1 option*/
							off++;
							continue;
						#if 1
						case 1: /* padN option*/
							optionDataLen=pData[off+1];
							off=off+optionDataLen+2;
							continue;
						#endif
						default:
							/*router alter option*/
							if (ntohl(*(uint32 *)(&pData[off]))==IPV6_ROUTER_ALTER_OPTION)
							{
								//ipv6RAO=IPV6_ROUTER_ALTER_OPTION;
								off+=4;
								continue;
							}
						//case 1: /* padN option*/
							/* other TLV option*/
							optionDataLen=pData[off+1];
							off=off+optionDataLen+2;
							continue;
					}
				}
				if (off - pPktHdr->l3Offset -40 != v6HeaderOff)
					rtlglue_printf("ipv6 ext hop-by-hop packet parse error\n");
				#endif
				pPktHdr->ipv6PayloadLen-=v6HeaderOff;
				goto MORE_IPV6_EXT;
			}
			case 43:	//Routing Header
				protocol=pData[off];
				v6HeaderOff=(pData[off+1]+1)<<3;	//length didn't count the first 8 octets, so we have to add 1 here
				off+=v6HeaderOff;
				pPktHdr->l3Len+=v6HeaderOff;
				/*
				if (pData[off+3]>0)
				{
					//Routing Head
				}
				*/
				pPktHdr->ipv6PayloadLen-=v6HeaderOff;
				goto MORE_IPV6_EXT;
			case 60:	//Destionation Options Header
				protocol=pData[off];
				v6HeaderOff=(pData[off+1]+1)<<3;	//length didn't count the first 8 octets, so we have to add 1 here
				
				pPktHdr->l3Len+=v6HeaderOff;
				off+=v6HeaderOff;
				pPktHdr->ipv6PayloadLen-=v6HeaderOff;
				goto MORE_IPV6_EXT;
			case 44:	//Fragment Header
				//TRACE("protocol is frag, next is %d",pData[off]);
				pPktHdr->tagif|=V6FRAG_TAGIF;
				protocol=pData[off];
				v6HeaderOff=8;
				
				pPktHdr->ipv6MoreFragment=pData[off+3]&0x1;
				pPktHdr->ipv6FragmentOffset=(pData[off+2]<<5)|((pData[off+3]&0xf8)>>3);	//Fragment Offset
				pPktHdr->ipv6FragId_First=ntohs(*(u16*)&pData[off+4]);
				pPktHdr->ipv6FragId_Second=ntohs(*(u16*)&pData[off+6]);
				if((pPktHdr->ipv6MoreFragment!=0)||(pPktHdr->ipv6FragmentOffset!=0)) 
					pPktHdr->ipv6FragPacket=1;
				else
					pPktHdr->ipv6FragPacket=0;
				//TRACE("IPv6 frag:M=%d, Off=%d, Id=%04x%04x",pPktHdr->ipv6MoreFragment,
					//pPktHdr->ipv6FragmentOffset,pPktHdr->ipv6FragId_First,pPktHdr->ipv6FragId_Second);
				pPktHdr->l3Len+=v6HeaderOff;
				off+=v6HeaderOff;
				pPktHdr->ipv6PayloadLen-=v6HeaderOff;
				goto MORE_IPV6_EXT;
			case 51:	//Authentication Header, [RFC4302]
				protocol=pData[off];
				v6HeaderOff=(pData[off+1]+2)<<2;	//This field specifies the length of AH in 32-bit words (4-byte units), minus "2" 
				pPktHdr->l3Len+=v6HeaderOff;
				off+=v6HeaderOff;
				pPktHdr->ipv6PayloadLen-=v6HeaderOff;
				goto MORE_IPV6_EXT;				
			case 103:	//PIM
				//if (protocol==103) TRACE("got IPv6 Packet with Next Header={103 | PIM}");
					break;
			case 89:	//OSPF or MOSPF
				//if (protocol==89) TRACE("got IPv6 Packet with Next Header={89 | MOSPF}");
					break;
			case 6:		//TCP
			case 9:		//IGP(used in Cisco IGRP)
			case 17:	//UDP				
			case 41:	//IPv6 Header, for Tunnel
			case 45:	//Inerdomain Routing Protocol(IDRP)
			case 46:	//Resource Reservation Protocol(RSVP)
			case 50:	//Encapsulating Security Payload, [RFC4303]				
				//if (protocol==50) TRACE("got IPv6 Packet with Next Header={50 | Encapsulating Security Payload}");
			case 58:	//ICMPv6
				//if (protocol==58) TRACE("got IPv6 Packet with Next Header={58 | ICMPv6}");
			//case 130:	//MLDv1/MLDv2 Query
				//if (protocol==130) TRACE("got IPv6 Packet with Next Header={130 | MLDv1/MLDv2 Query}");
			//case 131:	//MLDv1 Report
				//if (protocol==131) TRACE("got IPv6 Packet with Next Header={131 | MLDv1 Report}");
			//case 143:	//MLDv2 Report
				//if (protocol==143) TRACE("got IPv6 Packet with Next Header={131 | MLDv2 Report}");
			case 59:	//No Next Header
			case 88:	//EIGRP
			case 108:	//IP Payload Compression Protocol
			case 115:	//Layer2 Tunneling Protocol(L2TP)
			case 132:	//Stream Control Transmission Protocol(SCTP) ???????
				//if (protocol==89) TRACE("got IPv6 Packet with Next Header={132 | MLD Done}");
			case 135:	//Mobility Header, [RFC6275]
			case 139:	//Experimental use, Host Identity Protocol [RFC5201]
			case 140:	//Shim6 Protocol, [RFC5533]
			case 253:	//Use for experimentation and testing, [RFC3692], [RFC4727]
			case 254:	//Use for experimentation and testing, [RFC3692], [RFC4727]
				TRACE("v6 nextHeader value %d. Continue",protocol);
				break;
			default:
				TRACE("v6 unknown nextHeader value %d. Trap to protocol stack if L3/L4");
				pPktHdr->tagif|=V6TRAP_TAGIF;
				break;
		}
	}
	else if((*(u16*)(pData+off))==htons(0x0806)) //ARP
	//else if((pData[off]==0x08)&&(pData[off+1]==0x06))
	{
		pPktHdr->tagif|=ARP_TAGIF;
		pPktHdr->arpOpCode=ntohs(*(u16*)&pData[off+8]);
		pPktHdr->ipv4Sip=ntohl(*(u32*)&pData[off+16]);
		pPktHdr->ipv4Dip=ntohl(*(u32*)&pData[off+26]);
		pPktHdr->pIpv4Sip=(u32*)&pData[off+16];
		pPktHdr->pIpv4Dip=(u32*)&pData[off+26];	

		if(rg_db.systemGlobal.fwdStatistic)
		{
			if(pPktHdr->arpOpCode==1)
				rg_db.systemGlobal.statistic.perPortCnt_ARP_request[pPktHdr->ingressPort]++;
			else if(pPktHdr->arpOpCode==2)
				rg_db.systemGlobal.statistic.perPortCnt_ARP_reply[pPktHdr->ingressPort]++;
		}
	}

	pPktHdr->ipProtocol=protocol;

	if(((pPktHdr->tagif&IPV4_TAGIF)&&(pPktHdr->ipv4FragmentOffset>0))||((pPktHdr->tagif&IPV6_TAGIF)&&(pPktHdr->ipv6FragmentOffset>0)))
		return RG_FWDENGINE_RET_CONTINUE;

	if(protocol==0x6) //TCP
	{
    	//dump_packet(pData,len,"par");
#if 0    
        rtlglue_printf("TCP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Seq" COLOR_NM "=0x%x][" COLOR_Y "Ack" COLOR_NM "=0x%x][" COLOR_Y "HLen" COLOR_NM "=%d]\n"
                       ,(pData[off]<<8)|(pData[off+1]),(pData[off+2]<<8)|(pData[off+3]),(pData[off+4]<<24)|(pData[off+5]<<16)|(pData[off+6]<<8)|(pData[off+7]<<0)
                       ,(pData[off+8]<<24)|(pData[off+9]<<16)|(pData[off+10]<<8)|(pData[off+11]<<0),pData[off+12]>>4<<2);
        rtlglue_printf("    [" COLOR_Y "URG" COLOR_NM "=%d][" COLOR_Y "ACK" COLOR_NM "=%d][" COLOR_Y "PSH" COLOR_NM "=%d][" COLOR_Y "RST" COLOR_NM "=%d][" COLOR_Y "SYN" COLOR_NM "=%d][" COLOR_Y "FIN" COLOR_NM "=%d][" COLOR_Y "Win" COLOR_NM "=%d]\n"
                       ,(pData[off+13]>>5)&1,(pData[off+13]>>4)&1,(pData[off+13]>>3)&1,(pData[off+13]>>2)&1,(pData[off+13]>>1)&1,(pData[off+13]>>0)&1
                       ,(pData[off+14]<<8)|pData[off+15]);
        rtlglue_printf("    [" COLOR_Y "CHM" COLOR_NM "=0x%x][" COLOR_Y "Urg" COLOR_NM "=0x%x]\n",(pData[off+16]<<8)|(pData[off+17]<<0),(pData[off+18]<<8)|(pData[off+19]<<0));
#endif
		pPktHdr->l4Offset=off;
		pPktHdr->tagif|=TCP_TAGIF;		
		pPktHdr->sport=ntohs(*(u16*)&pData[off]);
		pPktHdr->pSport=(u16*)&pData[off];
		pPktHdr->dport=ntohs(*(u16*)&pData[off+2]);
		pPktHdr->pDport=(u16*)&pData[off+2];
		pPktHdr->tcpSeq=ntohl(*(u32*)&pData[off+4]);
		pPktHdr->pTcpSeq=(u32*)&pData[off+4];
		pPktHdr->tcpAck=ntohl(*(u32*)&pData[off+8]);		
		pPktHdr->pTcpAck=(u32*)&pData[off+8];
		*((u8*)(&pPktHdr->tcpFlags))=pData[off+13];

		if(rg_db.systemGlobal.fwdStatistic)
		{
			rg_db.systemGlobal.statistic.perPortCnt_TCP[pPktHdr->ingressPort]++;
			if(pPktHdr->tcpFlags.ack==0)
			{
				if(*((u8*)(&pPktHdr->tcpFlags))==0x2) rg_db.systemGlobal.statistic.perPortCnt_SYN[pPktHdr->ingressPort]++;
				else if(*((u8*)(&pPktHdr->tcpFlags))==0x1) rg_db.systemGlobal.statistic.perPortCnt_FIN[pPktHdr->ingressPort]++;
				else if(*((u8*)(&pPktHdr->tcpFlags))==0x4) rg_db.systemGlobal.statistic.perPortCnt_RST[pPktHdr->ingressPort]++;
			}
			else
			{
				if(*((u8*)(&pPktHdr->tcpFlags))==0x10) rg_db.systemGlobal.statistic.perPortCnt_ACK[pPktHdr->ingressPort]++;
				else if(*((u8*)(&pPktHdr->tcpFlags))==0x12) rg_db.systemGlobal.statistic.perPortCnt_SYN_ACK[pPktHdr->ingressPort]++;
				else if(*((u8*)(&pPktHdr->tcpFlags))==0x11) rg_db.systemGlobal.statistic.perPortCnt_FIN_ACK[pPktHdr->ingressPort]++;
				else if(*((u8*)(&pPktHdr->tcpFlags))==0x14) rg_db.systemGlobal.statistic.perPortCnt_RST_ACK[pPktHdr->ingressPort]++;
				else if(*((u8*)(&pPktHdr->tcpFlags))==0x19) rg_db.systemGlobal.statistic.perPortCnt_FIN_PSH_ACK[pPktHdr->ingressPort]++;
			}
		}

		pPktHdr->headerLen=pData[off+12]>>4<<2;
		pPktHdr->tcpWindow=ntohs(*(u16*)&pData[off+14]);
		pPktHdr->l4Checksum=ntohs(*(u16*)&pData[off+16]);
		pPktHdr->pL4Checksum=(u16*)&pData[off+16];
		//DEBUG("sip=%x dip=%x sport=0x%x dport=0x%x ip_protocol=%d syn=%d ack=%d fin=%d rst=%d\n",pPktHdr->ipv4Sip,pPktHdr->ipv4Dip,pPktHdr->sport,pPktHdr->dport,pPktHdr->ipProtocol,pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);			

		if(pPktHdr->tcpFlags.syn==1)
		{
			//parsing mss optins
			if(pPktHdr->headerLen>20) //tcp header len > 20 (have options)
			{
				int i;
				//memDump(&pData[off+20],pPktHdr->headerLen-20,"options");
				for(i=20;i<pPktHdr->headerLen;i++)
				{
					if(pData[off+i]==1) //No-Operation
					{
						//skip NOP					
					}
					else if(pData[off+i]==2) //Maximum Segment Size
					{
						pPktHdr->tagif|=MSS_TAGIF;
						pPktHdr->pMssLength=(uint16 *)&pData[off+i+2];
						//printk("mss=%d\n",*pPktHdr->pMssLength);
						break;
					}
					else if(pData[off+i]==0) //End of Option List	
					{
						break;
					}
					else
					{
						if(pData[off+i+1]>=2)
							i+=(pData[off+i+1]-1);	
						else //maybe packet format is error.
							break;
					}					
				}					
			}		
		}
		off+=pPktHdr->headerLen;

		//check HTTP request first packet & assign payloadbuff
		pPktHdr->httpFirstPacket=0;
		pPktHdr->pL4Payload=NULL;
		if((off < skb->len)&&((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF))){//only TCP or UDP have l4Payload.
			pPktHdr->pL4Payload=&pData[off];
		}
		if(rg_db.systemGlobal.urlFilter_totalNum>0 || !list_empty(&rg_db.redirectHttpURLListHead) || rg_db.systemGlobal.gatherLanNetInfo)
		{		
			if(off+2<=len)
			{
				if((pData[off]=='G')&&(pData[off+1]=='E')){
					pPktHdr->httpFirstPacket=1;
					pPktHdr->pL4Payload=&pData[off];
				}
				else if((pData[off]=='P')&&(pData[off+1]=='O')){
					pPktHdr->httpFirstPacket=1;
					pPktHdr->pL4Payload=&pData[off];
				}
			}
		}

		

		//Check for PPTP packet
		//Verify data length, if the length is unable to analyze outgoing-call-request or outgoing-call-reply, this should be invalid PPTP control packet
		if (off + sizeof(rtk_rg_pptpMsgHead_t) + sizeof(rtk_rg_pptpCallIds_t) <= len)	//pptpMsgHead and pptpCallIds are just enough to get session ID and Peer's call ID
		{
			// Move up to PPTP message header
			pPPTPHdr = (rtk_rg_pptpMsgHead_t *) (((u8 *) pData) + off);
			
			// Verify PPTP Control Message  
			pptpMagic = ntohl(pPPTPHdr->magic);
			if ((ntohs(pPPTPHdr->msgType) == PPTP_CTRL_MSG_TYPE) && (pptpMagic == PPTP_MAGIC))
			{
				// When packet is reply type, must have result code and error code to decide whether ID field is valid
				if ((ntohs(pPPTPHdr->type) == PPTP_OutCallReply || ntohs(pPPTPHdr->type) == PPTP_InCallReply))
				{
					// Verify data length:
					if((off + sizeof(rtk_rg_pptpMsgHead_t) + sizeof(rtk_rg_pptpCallIds_t) + sizeof(rtk_rg_pptpCodes_t)) <= len)
					{
						pPktHdr->tagif|=PPTP_TAGIF;
						pPktHdr->pptpCtrlType = ntohs(pPPTPHdr->type);
						pPktHdr->pPptpCallId = (rtk_rg_pptpCallIds_t *) (pPPTPHdr + 1);
						pPktHdr->pptpCodes = *(rtk_rg_pptpCodes_t *) (pPktHdr->pPptpCallId + 1);
						//DEBUG("PPTP CTRL TYPE is %d, CID1 is %04x, CID2 is %04x, resCode is %d, errCode is %d",pPktHdr->pptpCtrlType,pPktHdr->pPptpCallId->cid1,pPktHdr->pPptpCallId->cid2,
							//pPktHdr->pptpCodes.resCode,pPktHdr->pptpCodes.errCode);
					}
				}
				else
				{
					pPktHdr->tagif|=PPTP_TAGIF;
					pPktHdr->pptpCtrlType = ntohs(pPPTPHdr->type);
					pPktHdr->pPptpCallId = (rtk_rg_pptpCallIds_t *) (pPPTPHdr + 1);
					//DEBUG("PPTP CTRL TYPE is %d, CID1 is %04x, CID2 is %04x",pPktHdr->pptpCtrlType,pPktHdr->pPptpCallId->cid1,pPktHdr->pPptpCallId->cid2);
				}
			}
		}
	}
	else if(protocol==0x11) //UDP
	{
#if 0    
        rtlglue_printf("UDP:[" COLOR_Y "SPort" COLOR_NM "=%d][" COLOR_Y "DPort" COLOR_NM "=%d][" COLOR_Y "Len" COLOR_NM "=%d][" COLOR_Y "CHM" COLOR_NM "=0x%x]\n",(pData[off]<<8)|(pData[off+1]),(pData[off+2]<<8)|(pData[off+3])
                       ,(pData[off+4]<<8)|(pData[off+5]),(pData[off+6]<<8)|(pData[off+7]));
#endif
		if(rg_db.systemGlobal.fwdStatistic)
		{
			rg_db.systemGlobal.statistic.perPortCnt_UDP[pPktHdr->ingressPort]++;
		}

		pPktHdr->l4Offset=off;		
		pPktHdr->tagif|=UDP_TAGIF;
		pPktHdr->sport=ntohs(*(u16*)&pData[off]);
		pPktHdr->pSport=(u16*)&pData[off];
		pPktHdr->dport=ntohs(*(u16*)&pData[off+2]);
		pPktHdr->pDport=(u16*)&pData[off+2];
		pPktHdr->l4Checksum=ntohs(*(u16*)&pData[off+6]);
		pPktHdr->pL4Checksum=(u16*)&pData[off+6];
		//siyuan add for alg function which may change the udp data length
		pPktHdr->pL4Len = (u16*)&pData[off+4];
		pPktHdr->l4Len = ntohs(*(u16*)&pData[off+4]);
		off+=8;

		if((pPktHdr->tagif&L2TP_INNER_TAGIF)==0)
		{
			//Check if match L2TP WAN's IP and port number
			for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
			{
				pWan_intf=&rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf;
				if(pWan_intf->wan_intf_conf.wan_type==RTK_RG_L2TP &&
					pPktHdr->dport==pWan_intf->l2tp_info.after_dial.outer_port &&
					pPktHdr->sport==pWan_intf->l2tp_info.after_dial.gateway_outer_port &&
					pPktHdr->ipv4Sip==pWan_intf->l2tp_info.before_dial.l2tp_ipv4_addr)
				{
					//Match IP, check tunnel ID and session ID
					pPktHdr->l2tpFlagVersion = ntohs(*(u16*)&pData[off]);
					if(pPktHdr->l2tpFlagVersion&0x8000)break;		//skip control packet parsing
					if((pPktHdr->l2tpFlagVersion&0xf)!=0x2)break;	//only support version 2 now

					//L2tp version 2, need to parse each bit for Length, Ns, Nr, Offset.
					tmpOff=2;
					if(pPktHdr->l2tpFlagVersion&0x4000)	//Length bit
						tmpOff=4;
					if(ntohs(*(u16*)&pData[off+tmpOff])==pWan_intf->l2tp_info.after_dial.tunnelId &&
						ntohs(*(u16*)&pData[off+tmpOff+2])==pWan_intf->l2tp_info.after_dial.sessionId)
					{
						TRACE("Match L2TP!!");
						tmpOff+=4;
						if(pPktHdr->l2tpFlagVersion&0x0800)	//Sequence bit
							tmpOff+=4;
						if(pPktHdr->l2tpFlagVersion&0x0200)	//Offset bit
							tmpOff+=ntohs(*(u16*)&pData[off+tmpOff])+2;
						
						pPktHdr->tagif&=(~UDP_TAGIF);
						pPktHdr->tagif&=(~IPV4_TAGIF);
						//pPktHdr->tagif&=(~IPV6_TAGIF);

						//20151201LUKE: check if the Address and Control field be omitted
						if(*(u16*)(pData+off+tmpOff)==htons(0xff03))
							off+=tmpOff+2;		//PPP
						else
							off+=tmpOff;
						pPktHdr->tagif|=L2TP_INNER_TAGIF;
						break;
					}
				}	
			}
			if(pPktHdr->tagif&L2TP_INNER_TAGIF)goto TUUNEL_INNER;
		}
	}
	else if(protocol==0x1) //ICMP
	{
		//TRACE("parsing ICMP");
		pPktHdr->tagif|=ICMP_TAGIF;
		pPktHdr->l4Offset=off;		
		pPktHdr->ICMPType=(ntohs(*(u16*)&pData[off])&0xff00)>>8;
		pPktHdr->ICMPCode=ntohs(*(u16*)&pData[off])&0xff;
		pPktHdr->ICMPIdentifier=ntohs(*(u16*)&pData[off+4]);
		pPktHdr->ICMPSeqNum=ntohs(*(u16*)&pData[off+6]);
		pPktHdr->l4Checksum=ntohs(*(u16*)&pData[off+2]);
		pPktHdr->pL4Checksum=(u16*)&pData[off+2];
	}
	else if(protocol==0x2) //IGMP
	{
		_rtk_rg_igmpPacketParser(off, pData, pPktHdr);
	#if 0
		pPktHdr->IGMPType = (ntohs(*(u16*)&pData[off])&0xff00)>>8;
		
	#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
		//TRACE("parsing IGMP");
		//pPktHdr->tagif|=IGMP_TAGIF;
		/*check DVMRP*/
		if (pPktHdr->IGMPType==DVMRP_TYPE && (pPktHdr->ipv4Dip==htonl(DVMRP_ADDR)))
		{
			pPktHdr->tagif|=DVMRP_TAGIF;
		}
		else
		{
			/*means unicast*/
			if((pData[0]&0x1)==0)
			{
				if(rtl_compareMacAddr(pData, rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayMac)==TRUE)
				{
					if (pPktHdr->ipv4Dip==htonl(rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayIpv4Addr)) {
						pPktHdr->tagif|=IGMP_TAGIF;
					}
				}
			}
			else/*means multicast*/
			{
				if(rtl_checkMCastAddrMapping(IP_VERSION4,(uint32*)(&pPktHdr->ipv4Dip),pData)==TRUE) {
					pPktHdr->tagif|=IGMP_TAGIF;
				}
			}
		}
	#else
		//TRACE("not parsing IGMP");
	#endif
	#endif
	}
	else if (protocol==89) //MOSPF
	{
		_rtk_rg_mospfPacketParser(off, pData, pPktHdr);
	#if 0
	#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
		if (pPktHdr->tagif&IPV4_TAGIF)
		{
			//TRACE("parsing MOSPFv4");
			if (pPktHdr->ipv4Dip==htonl(IPV4_MOSPF_ADDR1) || pPktHdr->ipv4Dip==htonl(IPV4_MOSPF_ADDR2))
			{
				pPktHdr->tagif|=MOSPF_TAGIF;
			}
		}
		else if (pPktHdr->tagif&IPV6_TAGIF)
		{
			//TRACE("parsing MOSPFv6");
			//if (IS_IPV6_MOSPF_ADDR1(ipAddr) || IS_IPV6_MOSPF_ADDR2(ipAddr))
			if ((memcmp(pPktHdr->pIpv6Dip, IPV6_MOSPF_ADDR1, 16))
			||  (memcmp(pPktHdr->pIpv6Dip, IPV6_MOSPF_ADDR2, 16)))
			{
				pPktHdr->tagif|=MOSPF_TAGIF;
			}
		}
	#else
		//TRACE("not parsing MOSPF");
	#endif 
	#endif
	}
	else if (protocol==103) //PIM
	{
		_rtk_rg_pimfPacketParser(off, pData, pPktHdr);
	#if 0
	#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
		if (pPktHdr->tagif&IPV4_TAGIF)
		{
			//TRACE("parsing PIMv4");
			if (pPktHdr->ipv4Dip==htonl(IPV4_PIM_ADDR))
			{
				pPktHdr->tagif|=PIM_TAGIF;
			}
		}
		else if (pPktHdr->tagif&IPV6_TAGIF)
		{
			//TRACE("parsing PIMv6");
			//if (IS_IPV6_PIM_ADDR(ipAddr))
			if (memcmp(pPktHdr->pIpv6Dip,IPV6_PIM_ADDR,16))
			{
				pPktHdr->tagif|=PIM_TAGIF;
			}
		}
	#else
		//TRACE("not parsing PIM");
	#endif
	#endif
	}
	else if(protocol==0x3a)	//ICMPv6
	{
		pPktHdr->tagif|=ICMPV6_TAGIF;
		//Leo: Fix Parsing Error
		//off=off-2;
		
		pPktHdr->l4Offset=off;
		pPktHdr->ICMPv6Type=(ntohs(*(u16*)&pData[off])&0xff00)>>8;
		TRACE("got ICMPv6 Type=%d", pPktHdr->ICMPv6Type);
		if(pPktHdr->ICMPv6Type==0x88)	//136, Neighbor Advertisement
			pPktHdr->ICMPv6Flag=(ntohl(*(u32*)&pData[off+4])&0xf0000000)>>28;

		if(rg_db.systemGlobal.fwdStatistic)
		{
			if(pPktHdr->ICMPv6Type==0x87)
			{
				TRACE("parsing ICMPv6/fwdStatistic/solicitation(87)");
				rg_db.systemGlobal.statistic.perPortCnt_NB_solicitation[pPktHdr->ingressPort]++;
			}
			else if(pPktHdr->ICMPv6Type==0x88)
			{
				TRACE("parsing ICMPv6/fwdStatistic/adverticsement(88)");
				rg_db.systemGlobal.statistic.perPortCnt_NB_advertisement[pPktHdr->ingressPort]++;
			}
		}

		//next header start from 8 byte later
		//check mld
		if(pPktHdr->ICMPv6Type==130/*Multicast Listener Query*/
		|| pPktHdr->ICMPv6Type==131/*Multicast Listener Report*/
		|| pPktHdr->ICMPv6Type==132/*Multicast Listener Done*/
		|| pPktHdr->ICMPv6Type==143/*Multicast Listener Discovery (MLDv2) reports */)
		{
			_rtk_rg_mldPacketParser(off, pData, pPktHdr);
		#if 0
			pPktHdr->tagif|=IPV6_MLD_TAGIF;
			pPktHdr->IGMPv6Type = pPktHdr->ICMPv6Type;
			switch (pPktHdr->IGMPv6Type)
			{
			case 130:
				TRACE("got ICMPv6/MLDv1-Query");
			break;
			case 131:
				TRACE("got ICMPv6/MLDv1-Report");
			break;
			case 132:
				TRACE("got ICMPv6/MLDv1-Done");
			break;
			case 143:
				TRACE("got ICMPv6/MLDv2-Report");
			break;
			default :
				TRACE("Unknown Error for MLD type");
				break;
			}

		#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
			if (pPktHdr->pDmac[0]==0x33 && pPktHdr->pDmac[1]==0x33) {
				uint32 ipAddr[4]={0,0,0,0};
				ipAddr[0]=htonl((uint32*)(&pPktHdr->pIpv6Dip[0]));
				ipAddr[1]=htonl((uint32*)(&pPktHdr->pIpv6Dip[4]));
				ipAddr[2]=htonl((uint32*)(&pPktHdr->pIpv6Dip[8]));
				ipAddr[3]=htonl((uint32*)(&pPktHdr->pIpv6Dip[12]));
				if (rtl_checkMCastAddrMapping(IP_VERSION6, ipAddr, pData)==TRUE)
				{
				//	pPktHdr->tagif|=ICMPV6_TAGIF;
				//	pPktHdr->tagif|=IPV6_MLD_TAGIF;
				}
			}else{
				uint32 ipAddr1[4]={0,0,0,0};
				uint32 ipAddr2[4]={0,0,0,0};
				ipAddr1[0]=htonl((uint32*)(&pPktHdr->pIpv6Dip[0]));
				ipAddr1[1]=htonl((uint32*)(&pPktHdr->pIpv6Dip[4]));
				ipAddr1[2]=htonl((uint32*)(&pPktHdr->pIpv6Dip[8]));
				ipAddr1[3]=htonl((uint32*)(&pPktHdr->pIpv6Dip[12]));
				
				ipAddr2[0]=htonl(rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayIpv6Addr[0]);
				ipAddr2[1]=htonl(rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayIpv6Addr[1]);
				ipAddr2[2]=htonl(rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayIpv6Addr[2]);
				ipAddr2[3]=htonl(rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayIpv6Addr[3]);

				if ((rtl_compareMacAddr(pPktHdr->pDmac, rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayMac)==TRUE)
				&& (rtl_compareIpv6Addr(ipAddr1, ipAddr2)==TRUE))
				//&& (rtl_compareIpv6Addr(pPktHdr->pIpv6Dip, ipAddr2)==TRUE))
				{
				//	pPktHdr->tagif|=ICMPV6_TAGIF;
				//	pPktHdr->tagif|=IPV6_MLD_TAGIF;
				}
			}
		#endif
		#endif
		}
		//proc/rg/mld_trap_to_PS : trap mld packet original to PS 
		if((rg_db.systemGlobal.mld_Trap_to_PS_enable)&&(pPktHdr->pRxDesc->opts3.bit.dst_port_mask!=0x20)){
			if(pPktHdr->tagif&IPV6_MLD_TAGIF){//mld
				TRACE("TRAP mld to PS");
				ret = RG_FWDENGINE_RET_ACL_TO_PS;
			}
		}
	}
	else if(protocol==0x2f)	//GRE
	{
		tmpOff=8;	//GRE
		pPktHdr->tagif|=GRE_TAGIF;
		pPktHdr->pGRECallID=(u16*)&pData[off+6];
		//DEBUG("flags and version %x, protocol type is %x, len is %x, callID is %x",ntohs(*(u16*)&pData[off]),ntohs(*(u16*)&pData[off+2]),ntohs(*(u16*)&pData[off+4]),ntohs(*pPktHdr->pGRECallID));
		if(ntohs(*(u16*)&pData[off])&0x1000)
		{
			pPktHdr->tagif|=GRE_SEQ_TAGIF;
			pPktHdr->pGRESequence=(u32*)&pData[off+tmpOff];
			pPktHdr->GRESequence=ntohl(*pPktHdr->pGRESequence);
			//DEBUG("sequence is %d",pPktHdr->GRESequence);
			tmpOff+=4;
		}
		if(ntohs(*(u16*)&pData[off])&0x80)
		{
			pPktHdr->tagif|=GRE_ACK_TAGIF;
			pPktHdr->pGREAcknowledgment=(u32*)&pData[off+tmpOff];
			pPktHdr->GREAcknowledgment=ntohl(*pPktHdr->pGREAcknowledgment);
			//DEBUG("ack is %d",pPktHdr->GREAcknowledgment);
			tmpOff+=4;
		}
		//20150420LUKE: check if the Address and Control field be omitted
		if(*(u16*)(pData+off+tmpOff)==htons(0xff03))
			tmpOff+=2;

		if((pPktHdr->tagif&PPTP_INNER_TAGIF)==0)
		{
			for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
			{
				pWan_intf=&rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf;
				if(pWan_intf->wan_intf_conf.wan_type==RTK_RG_PPTP &&
					pPktHdr->ipv4Sip==pWan_intf->pptp_info.before_dial.pptp_ipv4_addr &&
					*pPktHdr->pGRECallID==pWan_intf->pptp_info.after_dial.callId)
				{
					TRACE("Match PPTP WAN[%d]!! update acknowledgment to %d",rg_db.systemGlobal.wanIntfGroup[i].index,pPktHdr->GRESequence);
					pPktHdr->tagif&=(~GRE_TAGIF);
					if(pPktHdr->tagif&GRE_SEQ_TAGIF)pWan_intf->pptp_info.gre_header_acknowledgment=pPktHdr->GRESequence;
					//20150617LUKE: from WAN may send to protocol stack, we should change ack to seq stored when protocol stack send packet!
					if(pPktHdr->ingressLocation!=RG_IGR_PROTOCOL_STACK){
						if(pPktHdr->tagif&GRE_ACK_TAGIF)
							*pPktHdr->pGREAcknowledgment=htonl(pWan_intf->pptp_info.sw_gre_header_sequence);
						if(pPktHdr->tagif&GRE_SEQ_TAGIF)
							pPktHdr->pServerGRESequence=&pWan_intf->pptp_info.sw_gre_header_server_sequence;
						if(!pWan_intf->pptp_info.sw_gre_header_server_sequence_started){
							pWan_intf->pptp_info.sw_gre_header_server_sequence=pPktHdr->GRESequence;
							pWan_intf->pptp_info.sw_gre_header_server_sequence_started=1;
						}
					}
					off+=tmpOff;
					pPktHdr->tagif|=PPTP_INNER_TAGIF;
					break;
				}
			}
			if(pPktHdr->tagif&PPTP_INNER_TAGIF)goto TUUNEL_INNER;
		}
	}
	else if(protocol==0x32) //ESP
	{
		pPktHdr->tagif|=ESP_TAGIF;  /*siyuan add for alg IPsec passthrough*/
		pPktHdr->l4Offset=off;
	}

	return ret;
}

static void _rtk_rg_igmpPacketParser(int off, u8 *pData, rtk_rg_pktHdr_t *pPktHdr)
{
	pPktHdr->IGMPType = (ntohs(*(u16*)&pData[off])&0xff00)>>8;
#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
	TRACE("parsing IGMP");
	//pPktHdr->tagif|=IGMP_TAGIF;
	
	/*check DVMRP*/
	if (pPktHdr->IGMPType==DVMRP_TYPE && (pPktHdr->ipv4Dip==htonl(DVMRP_ADDR)))
	{
		pPktHdr->tagif|=DVMRP_TAGIF;
	}
	else
	{
		/*means unicast*/
		if((pData[0]&0x1)==0)
		{
			if(rtl_compareMacAddr(pData, rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayMac)==TRUE)
			{
				if (pPktHdr->ipv4Dip==htonl(rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayIpv4Addr))
				{
					pPktHdr->tagif|=IGMP_TAGIF;
				}
			}
		}
		else/*means multicast*/
		{
			if(rtl_checkMCastAddrMapping(IP_VERSION4,(uint32*)(&pPktHdr->ipv4Dip),pData)==TRUE) {
				pPktHdr->tagif|=IGMP_TAGIF;
			}
		}
	}
#else
	TRACE("not parsing IGMP");
#endif

}

static void _rtk_rg_mospfPacketParser(int off, u8 *pData, rtk_rg_pktHdr_t *pPktHdr)
{
#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
	if (pPktHdr->tagif&IPV4_TAGIF)
	{
		TRACE("parsing MOSPFv4");
		if (pPktHdr->ipv4Dip==htonl(IPV4_MOSPF_ADDR1) || pPktHdr->ipv4Dip==htonl(IPV4_MOSPF_ADDR2))
		{
			pPktHdr->tagif|=MOSPF_TAGIF;
		}
	}
	else if (pPktHdr->tagif&IPV6_TAGIF)
	{
		TRACE("parsing MOSPFv6");
		//if (IS_IPV6_MOSPF_ADDR1(ipAddr) || IS_IPV6_MOSPF_ADDR2(ipAddr))
		if ((memcmp(pPktHdr->pIpv6Dip, IPV6_MOSPF_ADDR1, 16))
		||  (memcmp(pPktHdr->pIpv6Dip, IPV6_MOSPF_ADDR2, 16)))
		{
			pPktHdr->tagif|=MOSPF_TAGIF;
		}
	}
#else
	TRACE("not parsing MOSPF");
#endif 
}

static void _rtk_rg_pimfPacketParser(int off, u8 *pData, rtk_rg_pktHdr_t *pPktHdr)
{
#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
	if (pPktHdr->tagif&IPV4_TAGIF)
	{
		TRACE("parsing PIMv4");
		if (pPktHdr->ipv4Dip==htonl(IPV4_PIM_ADDR))
		{
			pPktHdr->tagif|=PIM_TAGIF;
		}
	}
	else if (pPktHdr->tagif&IPV6_TAGIF)
	{
		TRACE("parsing PIMv6");
		//if (IS_IPV6_PIM_ADDR(ipAddr))
		if (memcmp(pPktHdr->pIpv6Dip,IPV6_PIM_ADDR,16))
		{
			pPktHdr->tagif|=PIM_TAGIF;
		}
	}
#else
	TRACE("not parsing PIM");
#endif

}

void _rtk_rg_mldPacketParser(int off, u8 *pData, rtk_rg_pktHdr_t *pPktHdr)
{
	pPktHdr->tagif|=IPV6_MLD_TAGIF;
	pPktHdr->IGMPv6Type = pPktHdr->ICMPv6Type;
	switch (pPktHdr->IGMPv6Type)
	{
	case 130:
		TRACE("got ICMPv6/MLDv1-Query");
		break;
	case 131:
		TRACE("got ICMPv6/MLDv1-Report");
		break;
	case 132:
		TRACE("got ICMPv6/MLDv1-Done");
		break;
	case 143:
		TRACE("got ICMPv6/MLDv2-Report");
		break;
	default :
		TRACE("Unknown Error for MLD type");
		break;
	}

#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
	if (pPktHdr->pDmac[0]==0x33 && pPktHdr->pDmac[1]==0x33) {
		uint32 ipAddr[4]={0,0,0,0};
		ipAddr[0]=htonl((uint32*)(&pPktHdr->pIpv6Dip[0]));
		ipAddr[1]=htonl((uint32*)(&pPktHdr->pIpv6Dip[4]));
		ipAddr[2]=htonl((uint32*)(&pPktHdr->pIpv6Dip[8]));
		ipAddr[3]=htonl((uint32*)(&pPktHdr->pIpv6Dip[12]));
		if (rtl_checkMCastAddrMapping(IP_VERSION6, ipAddr, pData)==TRUE)
		{
		//	pPktHdr->tagif|=ICMPV6_TAGIF;
		//	pPktHdr->tagif|=IPV6_MLD_TAGIF;
		}
	}else{
		uint32 ipAddr1[4]={0,0,0,0};
		uint32 ipAddr2[4]={0,0,0,0};
		ipAddr1[0]=htonl((uint32*)(&pPktHdr->pIpv6Dip[0]));
		ipAddr1[1]=htonl((uint32*)(&pPktHdr->pIpv6Dip[4]));
		ipAddr1[2]=htonl((uint32*)(&pPktHdr->pIpv6Dip[8]));
		ipAddr1[3]=htonl((uint32*)(&pPktHdr->pIpv6Dip[12]));
		
		ipAddr2[0]=htonl(rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayIpv6Addr[0]);
		ipAddr2[1]=htonl(rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayIpv6Addr[1]);
		ipAddr2[2]=htonl(rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayIpv6Addr[2]);
		ipAddr2[3]=htonl(rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayIpv6Addr[3]);

		if ((rtl_compareMacAddr(pPktHdr->pDmac, rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].rtl_gatewayMac)==TRUE)
		&& (rtl_compareIpv6Addr(ipAddr1, ipAddr2)==TRUE))
		//&& (rtl_compareIpv6Addr(pPktHdr->pIpv6Dip, ipAddr2)==TRUE))
		{
		//	pPktHdr->tagif|=ICMPV6_TAGIF;
		//	pPktHdr->tagif|=IPV6_MLD_TAGIF;
		}
	}
#else
	TRACE("not parsing MLD");
#endif

}

rtk_rg_successFailReturn_t _rtk_rg_algCheckEnable(unsigned char isTCP, unsigned short checkPort)
{
	int algIdx;
	unsigned int algBitValue;

	algIdx=checkPort>>5;
	algBitValue=0x1<<(checkPort&0x1f);

	//DEBUG("the algIdx is %d, TCPPortEnable is %x, algbitvalue is %x",
		//algIdx,rg_db.algTcpExternPortEnabled[algIdx],algBitValue);
	
	if(isTCP)
	{
		if(rg_db.algTcpExternPortEnabled[algIdx]&algBitValue)
		{
			//DEBUG("the TCP port %d has enabled ALG!!",checkPort);
			return RG_RET_SUCCESS;
		}
	}
	else
	{
		if(rg_db.algUdpExternPortEnabled[algIdx]&algBitValue)
		{
			//DEBUG("the UDP port %d is ALG enabled!!",checkPort);
			return RG_RET_SUCCESS;
		}
	}

	return RG_RET_FAIL;
}

rtk_rg_successFailReturn_t _rtk_rg_algSrvInLanCheckEnable(unsigned char isTCP, unsigned short checkPort)
{
	int algIdx;
	unsigned int algBitValue;

	algIdx=checkPort>>5;
	algBitValue=0x1<<(checkPort&0x1f);

	//DEBUG("the algIdx is %d, TCPPortEnable is %x, algbitvalue is %x",
		//algIdx,rg_db.algTcpExternPortEnabled[algIdx],algBitValue);
	
	if(isTCP)
	{
		if(rg_db.algTcpExternPortEnabled_SrvInLan[algIdx]&algBitValue)
		{
			//DEBUG("the TCP port %d has enabled ALG when Server In LAN!!",checkPort);
			return RG_RET_SUCCESS;
		}
	}
	else
	{
		if(rg_db.algUdpExternPortEnabled_SrvInLan[algIdx]&algBitValue)
		{
			//DEBUG("the UDP port %d has enabled ALG when Server In LAN!!",checkPort);
			return RG_RET_SUCCESS;
		}
	}

	return RG_RET_FAIL;
}

void _rtk_rg_algDynamicPortCheck(rtk_rg_pktHdr_t *pPktHdr, int checkPort, int isTCP, int serverInLan)
{
	rtk_rg_alg_dynamicPort_t *pList;

	//Check Dynamic Port first!!
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.algDynamicLock);
	if(!list_empty(&rg_db.algDynamicCheckListHead))
	{
		list_for_each_entry(pList,&rg_db.algDynamicCheckListHead,alg_list)
		{
			if(pList->portNum==checkPort && pList->isTCP==isTCP && pList->serverInLan==serverInLan)
			{
				DEBUG("Hit %s %d for serverIn%s dynamic ALG!! funtion is %p",isTCP?"TCP":"UDP",checkPort,serverInLan==1?"LAN":"WAN",pList->algFun);
				pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
				pPktHdr->algRegFun=pList->algFun;
				break;
			}
		}
	}
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.algDynamicLock);

}

rtk_rg_fwdEngineReturn_t _rtk_rg_algFunctionCheck(int direct, rtk_rg_pktHdr_t *pPktHdr)
{
	int i,checkPort,checkPort_SrvInLan;
	unsigned int checkSetting;

	if(direct == NAPT_DIRECTION_OUTBOUND)
	{
		checkPort_SrvInLan=pPktHdr->sport;
		checkPort=pPktHdr->dport;
	}
	else
	{
		checkPort_SrvInLan=pPktHdr->dport;
		checkPort=pPktHdr->sport;
	}

	//**** If the ALG register function is NULL, and the ALG is turn on, we will trap this packet to protocol stack directly.

	//DEBUG("%s packet, the mask is %x, check port is %d, check port in LAN is %d",pPktHdr->tagif&TCP_TAGIF?"TCP":"UDP",rg_db.algFunctionMask,checkPort,checkPort_SrvInLan);

	//initial algAction
	pPktHdr->algAction=RG_ALG_ACT_NORMAL;

	//Check ServerInWAN and Passthrough first, then check ServerInLAN:
	if(pPktHdr->tagif&TCP_TAGIF)
	{				
		if(_rtk_rg_algCheckEnable(1,checkPort)==SUCCESS)
		{
			pPktHdr->algAction=RG_ALG_ACT_NORMAL;
			_rtk_rg_algDynamicPortCheck(pPktHdr,checkPort,1,0);
			if(pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE)
				goto ALG_DYNAMIC_RET;
			
			//DEBUG("Port %d enable!!",checkPort);
			//do the matching alg hook function
			//20130806 Luke:
			//Since SrvInWan will check before SrvInLan, 
			//if this packet hit SrvInWan here, it will use SrvInWan's register function correctly,
			//therefore it doesn't need to change!!
			checkSetting=rg_db.algFunctionMask;
			i=0;
			while(checkSetting>0 || i<MAX_ALG_FUNCTIONS)
			{
				if(((rg_db.algFunctionMask&(0x1<<i))>0) && 
					(rg_db.algTcpFunctionMapping[i].portNum==checkPort))
				{
					DEBUG("TCP before execute function...%x, port %d, isNULL?%s",(rg_db.algFunctionMask&(0x1<<i)),rg_db.algTcpFunctionMapping[i].portNum,rg_db.algTcpFunctionMapping[i].registerFunction!=NULL?"NO":"YES");
					if(rg_db.algTcpFunctionMapping[i].registerFunction!=NULL)
					{
						pPktHdr->algKeepExtPort=rg_db.algTcpFunctionMapping[i].keepExtPort;
						pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
						//pPktHdr->algFunctionMappingIdx=i;
						pPktHdr->algRegFun=rg_db.algTcpFunctionMapping[i].registerFunction;
						return RG_FWDENGINE_RET_CONTINUE;
					}
					else
						return RG_FWDENGINE_RET_TO_PS;
				}
				else
				{
					//check next
					checkSetting>>=1;
					i++;
				}
			}	

			//algPort enable but no hit any register function mask, goto slow path without add to shortcut!!
			pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
			//pPktHdr->algFunctionMappingIdx=-1;
			pPktHdr->algRegFun=NULL;
		}
		if(_rtk_rg_algSrvInLanCheckEnable(1,checkPort_SrvInLan)==SUCCESS)
		{
			pPktHdr->algAction=RG_ALG_ACT_NORMAL;
			_rtk_rg_algDynamicPortCheck(pPktHdr,checkPort_SrvInLan,1,1);
			if(pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE)
				goto ALG_DYNAMIC_RET;
			
			DEBUG("Port %d enable in SrvInLan!!",checkPort_SrvInLan);
			//do the matching alg hook function
			checkSetting=rg_db.algFunctionMask>>ALG_SRV_IN_LAN_IDX;
			i=ALG_SRV_IN_LAN_IDX;
			while(checkSetting>0 || i<MAX_ALG_FUNCTIONS)
			{				
				if(((rg_db.algFunctionMask&(0x1<<i))>0) && 
					(rg_db.algTcpFunctionMapping[i].portNum==checkPort_SrvInLan))
				{
					DEBUG("TCP SrvInLAN before execute function...%x, port %d, isNULL?%s",(rg_db.algFunctionMask&(0x1<<i)),rg_db.algTcpFunctionMapping[i].portNum,	rg_db.algTcpFunctionMapping[i].registerFunction!=NULL?"NO":"YES");
					if(rg_db.algTcpFunctionMapping[i].registerFunction!=NULL)
					{
						pPktHdr->algKeepExtPort=rg_db.algTcpFunctionMapping[i].keepExtPort;
						pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
						//pPktHdr->algFunctionMappingIdx=i;
						pPktHdr->algRegFun=rg_db.algTcpFunctionMapping[i].registerFunction;
						return RG_FWDENGINE_RET_CONTINUE;
					}
					else
						return RG_FWDENGINE_RET_TO_PS;
				}
				else
				{
					//check next
					checkSetting>>=1;
					i++;
				}
			}	

			//algPort enable but no hit any register function mask, goto slow path without add to shortcut!!
			pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
			//pPktHdr->algFunctionMappingIdx=-1;
			pPktHdr->algRegFun=NULL;
		}
	}
	else if(pPktHdr->tagif&UDP_TAGIF)
	{
		if(_rtk_rg_algCheckEnable(0,checkPort)==SUCCESS)
		{
			pPktHdr->algAction=RG_ALG_ACT_NORMAL;
			_rtk_rg_algDynamicPortCheck(pPktHdr,checkPort,0,0);
			if(pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE)
				goto ALG_DYNAMIC_RET;
			
			//DEBUG("UDP Port %d enable!!",checkPort);
			//do the matching alg hook function
			checkSetting=rg_db.algFunctionMask;
			i=0;
			while(checkSetting>0 || i<MAX_ALG_FUNCTIONS)
			{
				if(((rg_db.algFunctionMask&(0x1<<i))>0) && 
					(rg_db.algUdpFunctionMapping[i].portNum==checkPort))
				{
					//DEBUG("UDP before execute function...%x, port %d, isNULL?%s",(rg_db.algFunctionMask&(0x1<<i)),rg_db.algUdpFunctionMapping[i].portNum,rg_db.algUdpFunctionMapping[i].registerFunction!=NULL?"NO":"YES");
					if(rg_db.algUdpFunctionMapping[i].registerFunction!=NULL)
					{
						pPktHdr->algKeepExtPort=rg_db.algUdpFunctionMapping[i].keepExtPort;
						pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
						//pPktHdr->algFunctionMappingIdx=i;
						pPktHdr->algRegFun=rg_db.algUdpFunctionMapping[i].registerFunction;
						return RG_FWDENGINE_RET_CONTINUE;
					}
					else
						return RG_FWDENGINE_RET_TO_PS;
				}
				else
				{			
					//check next
					checkSetting>>=1;
					i++;
				}
			}

			//algPort enable but no hit any register function mask, goto slow path without add to shortcut!!
			pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
			//pPktHdr->algFunctionMappingIdx=-1;
			pPktHdr->algRegFun=NULL;
		}
		if(_rtk_rg_algSrvInLanCheckEnable(0,checkPort_SrvInLan)==SUCCESS)
		{
			pPktHdr->algAction=RG_ALG_ACT_NORMAL;
			_rtk_rg_algDynamicPortCheck(pPktHdr,checkPort_SrvInLan,0,1);
			if(pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE)
				goto ALG_DYNAMIC_RET;
			
			//DEBUG("UDP Port %d enable in SrvInLan!!",checkPort_SrvInLan);
			//do the matching alg hook function
			checkSetting=rg_db.algFunctionMask>>ALG_SRV_IN_LAN_IDX;
			i=ALG_SRV_IN_LAN_IDX;
			while(checkSetting>0 || i<MAX_ALG_FUNCTIONS)
			{
				if(((rg_db.algFunctionMask&(0x1<<i))>0) && 
					(rg_db.algUdpFunctionMapping[i].portNum==checkPort_SrvInLan))
				{
					//DEBUG("UDP SrvInLAN before execute function...%x, port %d, isNULL?%s",(rg_db.algFunctionMask&(0x1<<i)),rg_db.algUdpFunctionMapping[i].portNum,rg_db.algUdpFunctionMapping[i].registerFunction!=NULL?"NO":"YES");
					if(rg_db.algUdpFunctionMapping[i].registerFunction!=NULL)
					{
						pPktHdr->algKeepExtPort=rg_db.algUdpFunctionMapping[i].keepExtPort;
						pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
						//pPktHdr->algFunctionMappingIdx=i;
						pPktHdr->algRegFun=rg_db.algUdpFunctionMapping[i].registerFunction;
						return RG_FWDENGINE_RET_CONTINUE;
					}
					else
						return RG_FWDENGINE_RET_TO_PS;
				}
				else
				{			
					//check next
					checkSetting>>=1;
					i++;
				}
			}

			//algPort enable but no hit any register function mask, goto slow path without add to shortcut!!
			pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
			//pPktHdr->algFunctionMappingIdx=-1;
			pPktHdr->algRegFun=NULL;
		}
	}

ALG_DYNAMIC_RET:	
	return RG_FWDENGINE_RET_CONTINUE;
}

rtk_rg_fwdEngineAlgReturn_t _rtk_rg_algForward(int direct, u8 after, struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_fwdEngineAlgReturn_t ret=RG_FWDENGINE_ALG_RET_FAIL;
#if 0
	int i,checkPort,checkPort_SrvInLan;
	unsigned int checkSetting;

	if(direct == NAPT_DIRECTION_OUTBOUND)
	{
		checkPort_SrvInLan=pPktHdr->sport;
		checkPort=pPktHdr->dport;
	}
	else
	{
		checkPort_SrvInLan=pPktHdr->dport;
		checkPort=pPktHdr->sport;
	}
#endif


	//DEBUG("%s packet, the mask is %x, check port is %d",pPktHdr->tagif&TCP_TAGIF?"TCP":"UDP",checkSetting,checkPort);

	//Check ServerInWAN and Passthrough first, then check ServerInLAN:
	if(pPktHdr->tagif&TCP_TAGIF)
	{	
		if(pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE && pPktHdr->algRegFun!=NULL)
		{
			DEBUG("do the TCP register function %p",pPktHdr->algRegFun);
			ret = pPktHdr->algRegFun(direct,after,(unsigned char *)skb,(unsigned char *)pPktHdr);
		}
//20130814LUKE:Close it for normal packet can bypass
#if 0		
		else
		{
			//siyuan add for alg h323
			ret = rtk_rg_alg_expect_forward(direct,after,(unsigned char *)skb,(unsigned char *)pPktHdr);
		}
#endif
#if 0
		if(_rtk_rg_algCheckEnable(1,checkPort)==SUCCESS)
		{
			//DEBUG("Port %d enable!!",checkPort);
			//do the matching alg hook function
			checkSetting=rg_db.algFunctionMask;
			i=0;
			while(checkSetting>0 || i<MAX_ALG_FUNCTIONS)
			{
				//DEBUG("TCP before execute function...%d, port %d, isNULL?%s",(rg_db.algFunctionMask&(0x1<<i)),rg_db.algTcpFunctionMapping[i].portNum,	rg_db.algTcpFunctionMapping[i].registerFunction!=NULL?"NO":"YES");
				if(((rg_db.algFunctionMask&(0x1<<i))>0) && 
					(rg_db.algTcpFunctionMapping[i].portNum==checkPort) && 
					(rg_db.algTcpFunctionMapping[i].registerFunction!=NULL))
				{
					//DEBUG("do the register function[%d]!%p",i,rg_db.algTcpFunctionMapping[i].registerFunction);
					ret = rg_db.algTcpFunctionMapping[i].registerFunction(direct,after,(unsigned char *)skb,(unsigned char *)pPktHdr);
					break;
				}
				else
				{
					//check next
					checkSetting>>=1;
					i++;
				}
			}	
		}
		if(_rtk_rg_algSrvInLanCheckEnable(1,checkPort_SrvInLan)==SUCCESS)
		{
			//do the matching alg hook function
			checkSetting=rg_db.algFunctionMask;
			i=0;
			while(checkSetting>0 || i<MAX_ALG_FUNCTIONS)
			{
				//DEBUG("TCP SrvInLAN before execute function...%d, port %d, isNULL?%s",(rg_db.algFunctionMask&(0x1<<i)),rg_db.algTcpFunctionMapping[i].portNum,	rg_db.algTcpFunctionMapping[i].registerFunction!=NULL?"NO":"YES");
				if(((rg_db.algFunctionMask&(0x1<<i))>0) && 
					(rg_db.algTcpFunctionMapping[i].portNum==checkPort_SrvInLan) && 
					(rg_db.algTcpFunctionMapping[i].registerFunction!=NULL))
				{
					//DEBUG("do the register function[%d]!%p",i,rg_db.algTcpFunctionMapping[i].registerFunction);
					ret = rg_db.algTcpFunctionMapping[i].registerFunction(direct,after,(unsigned char *)skb,(unsigned char *)pPktHdr);
					break;
				}
				else
				{
					//check next
					checkSetting>>=1;
					i++;
				}
			}	
		}
#endif
	}
	else if(pPktHdr->tagif&UDP_TAGIF)
	{
		if(pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE && pPktHdr->algRegFun!=NULL)
		{
			DEBUG("do the UDP register function %p",pPktHdr->algRegFun);
			ret = pPktHdr->algRegFun(direct,after,(unsigned char *)skb,(unsigned char *)pPktHdr);
		}
//20130814LUKE:Close it for normal packet can bypass
#if 0
		else
		{
			//siyuan add for alg h323
			ret = rtk_rg_alg_expect_forward(direct,after,(unsigned char *)skb,(unsigned char *)pPktHdr);
		}
#endif
#if 0
		if(_rtk_rg_algCheckEnable(0,checkPort)==SUCCESS)
		{
			//do the matching alg hook function
			checkSetting=rg_db.algFunctionMask;
			i=0;
			while(checkSetting>0 || i<MAX_ALG_FUNCTIONS)
			{
				//DEBUG("UDP before execute function...%d, port %d, isNULL?%s",(rg_db.algFunctionMask&(0x1<<i)),rg_db.algUdpFunctionMapping[i].portNum,rg_db.algUdpFunctionMapping[i].registerFunction!=NULL?"NO":"YES");
				if(((rg_db.algFunctionMask&(0x1<<i))>0) && 
					(rg_db.algUdpFunctionMapping[i].portNum==checkPort) && 
					(rg_db.algUdpFunctionMapping[i].registerFunction!=NULL))
				{
					ret = rg_db.algUdpFunctionMapping[i].registerFunction(direct,after,(unsigned char *)skb,(unsigned char *)pPktHdr);
					break;
				}
				else
				{			
					//check next
					checkSetting>>=1;
					i++;
				}
			}
		}
		if(_rtk_rg_algSrvInLanCheckEnable(0,checkPort_SrvInLan)==SUCCESS)
		{
			//do the matching alg hook function
			checkSetting=rg_db.algFunctionMask;
			i=0;
			while(checkSetting>0 || i<MAX_ALG_FUNCTIONS)
			{
				//DEBUG("UDP SrvInLAN before execute function...%d, port %d, isNULL?%s",(rg_db.algFunctionMask&(0x1<<i)),rg_db.algUdpFunctionMapping[i].portNum,rg_db.algUdpFunctionMapping[i].registerFunction!=NULL?"NO":"YES");
				if(((rg_db.algFunctionMask&(0x1<<i))>0) && 
					(rg_db.algUdpFunctionMapping[i].portNum==checkPort_SrvInLan) && 
					(rg_db.algUdpFunctionMapping[i].registerFunction!=NULL))
				{
					ret = rg_db.algUdpFunctionMapping[i].registerFunction(direct,after,(unsigned char *)skb,(unsigned char *)pPktHdr);
					break;
				}
				else
				{			
					//check next
					checkSetting>>=1;
					i++;
				}
			}
		}
#endif
	}

	if(ret!=RG_FWDENGINE_ALG_RET_FAIL)
	{
		TRACE("ALG FINISHED..ret=%d",ret);
	}
	return ret;
}

//20130821LUKE:close it because implemented by separate module
#if 0
rtk_rg_pptpGreEntry_t *_rtk_rg_lookupPPTPOutboundGreFlow(int netifIdx, unsigned int internalIp,unsigned short internalCallID, unsigned int remoteIp)
{
	rtk_rg_pptpGreLinkList_t *pEntry=NULL;

	for (pEntry = rg_db.pPPTPGreOutboundHead[netifIdx]->pPrev; pEntry != rg_db.pPPTPGreOutboundHead[netifIdx]; pEntry = pEntry->pPrev)
	{
		if((pEntry->greEntry.valid==1) &&
			(pEntry->greEntry.internalIpAddr==internalIp)&&
			(pEntry->greEntry.internalCallID==internalCallID)&&
			(pEntry->greEntry.remoteIpAddr==remoteIp))
			break;
	}
	if(pEntry == rg_db.pPPTPGreOutboundHead[netifIdx])		//not found
		return NULL;

	return &pEntry->greEntry;
}

rtk_rg_pptpGreEntry_t *_rtk_rg_lookupPPTPOutboundGreFlowByCallID(int netifIdx, unsigned short externalCallID, unsigned short remoteCallID, unsigned int remoteIp)
{
	rtk_rg_pptpGreLinkList_t *pEntry=NULL;
	
	for (pEntry = rg_db.pPPTPGreOutboundHead[netifIdx]->pPrev; pEntry != rg_db.pPPTPGreOutboundHead[netifIdx]; pEntry = pEntry->pPrev)
	{
		if(pEntry->greEntry.valid==1 && 
			pEntry->greEntry.remoteIpAddr==remoteIp &&
			((externalCallID>=0 && (pEntry->greEntry.externalCallID==externalCallID)) ||
			(remoteCallID>=0 && (pEntry->greEntry.remoteCallID==remoteCallID))))
			break;
	}
	if(pEntry == rg_db.pPPTPGreOutboundHead[netifIdx])		//not found
		return NULL;

	return &pEntry->greEntry;
}

int _rtk_rg_PPTPExtCallIDGetAndUse(uint16 wishCallID)
{
	int wishIdx;
	uint32 wishBitValue;
	int i;

	i=wishCallID;
	while(1)
	{
		wishIdx=i>>5; // =wishCallID/32
		wishBitValue=1<<(i&0x1f);
		
		if((rg_db.algPPTPExtCallIDEnabled[wishIdx]&wishBitValue)==0)
		{
			//DEBUG("callID %d is set!",i);
			rg_db.algPPTPExtCallIDEnabled[wishIdx]|=wishBitValue;
			return i;
		}
		
		i++;
		i&=0xffff;
		if(i==wishCallID) break;		
	}	
	return RG_RET_FAIL;
}

void _rtk_rg_PPTPExtCallIDFree(int callID)
{
	int idx;
	uint32 bitValue;
	int i;

	i=callID;
	idx=i>>5; // =callID/32
	bitValue=1<<(i&0x1f);
	
	if((rg_db.algPPTPExtCallIDEnabled[idx]&bitValue)>0)
		rg_db.algPPTPExtCallIDEnabled[idx]&=(~bitValue);
}

int _rtk_rg_PPTP_GREModify(rtk_rg_naptDirection_t direct, struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	int ret;
	rtk_rg_pptpGreEntry_t *pGreEntry;
	
	if(((rg_db.algFunctionMask & RTK_RG_ALG_PPTP_TCP_PASSTHROUGH_BIT) > 0 && rg_db.algTcpFunctionMapping[RTK_RG_ALG_PPTP_TCP_PASSTHROUGH].registerFunction!=NULL) ||
		((rg_db.algFunctionMask & RTK_RG_ALG_PPTP_UDP_PASSTHROUGH_BIT) > 0 && rg_db.algUdpFunctionMapping[RTK_RG_ALG_PPTP_UDP_PASSTHROUGH].registerFunction!=NULL))
	{		
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			//DEBUG("$$$$ PPTP_GREModify:  OUTBOUND packet to WAN[%d], orig SIP is %x",pPktHdr->netifIdx,pPktHdr->ipv4Sip);

			//lookup flow from remoteCallID
			pGreEntry=_rtk_rg_lookupPPTPOutboundGreFlowByCallID(pPktHdr->netifIdx,-1,ntohs(*pPktHdr->pGRECallID),pPktHdr->ipv4Dip);
			if(pGreEntry!=NULL)
			{
				//Turn on action to prevent adding to shortCut
				pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
				
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,0, pPktHdr,skb,1,0);
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP

				//DEBUG("SIP change to %x",ntohl(*pPktHdr->pIpv4Sip));

				return RG_FWDENGINE_RET_DIRECT_TX;
			}
		}
		else
		{
			//DEBUG("$$$$ PPTP_GREModify:  INBOUND packet from WAN[%d], orig DIP is %x",pPktHdr->netifIdx,pPktHdr->ipv4Dip);
			
			//lookup DIP from CallID in key(ExtCallID)
			pGreEntry=_rtk_rg_lookupPPTPOutboundGreFlowByCallID(pPktHdr->netifIdx,ntohs(*pPktHdr->pGRECallID),-1,pPktHdr->ipv4Sip);		//peer's CallID is the external CallID
			if(pGreEntry!=NULL)
			{
				//Turn on action to prevent adding to shortCut
				pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;

				//Change DIP to internal IP
				*pPktHdr->pIpv4Dip=htonl(pGreEntry->internalIpAddr);
				pPktHdr->ipv4Dip=pGreEntry->internalIpAddr;
				
				//Change CallID in key to IntCallID	
				*pPktHdr->pGRECallID=htons(pGreEntry->internalCallID);
			
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,0,pPktHdr,skb,0,0);
				//dump_packet(skb->data,skb->len,"new");
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP

				//DEBUG("DIP change to %x, callID change to %x",pGreEntry->internalIpAddr,pGreEntry->internalCallID);
			
				return RG_FWDENGINE_RET_DIRECT_TX;
			}
		}
	}

	return RG_FWDENGINE_RET_TO_PS;
}

int _rtk_rg_algRegFunc_TCP_PPTP(int direct, int after, unsigned char *pSkb,unsigned char *pPktHdr)
{
//FIXME:here should be rewrite for model code
#ifdef __KERNEL__
	rtk_rg_pktHdr_t *pPktInfo;
	struct sk_buff *skb;
	rtk_rg_pptpGreEntry_t *pGreEntry;
	int usableCallID=-1;

	pPktInfo=(rtk_rg_pktHdr_t *)pPktHdr;
	skb=(struct sk_buff *)pSkb;

	if((pPktInfo->tagif&PPTP_TAGIF)==0)		//not PPTP packet
		return RG_RET_FAIL;
/*
	PPTP_StartCtrlConnRequest	= 1,
	PPTP_StartCtrlConnReply 	= 2,
	PPTP_StopCtrlConnRequest	= 3,
	PPTP_StopCtrlConnReply	= 4,
	PPTP_EchoRequest			= 5,
	PPTP_EchoReply			= 6,
	PPTP_OutCallRequest 		= 7,
	PPTP_OutCallReply			= 8,
	PPTP_InCallRequest		= 9,
	PPTP_InCallReply			= 10,
	PPTP_InCallConn 			= 11,
	PPTP_CallClearRequest		= 12,
	PPTP_CallDiscNotify 		= 13,
	PPTP_WanErrorNotify 		= 14,
	PPTP_SetLinkInfo			= 15
*/

	if(after==0)
	{
		//Pre function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			switch(pPktInfo->pptpCtrlType)
			{
				case PPTP_OutCallRequest:
					//find ExtCallID for use
					//Keep internalIpAddr and MAC
					//Keep internalCallID
					DEBUG("$$$$ PRE PPTP_OutCallRequest  pPktInfo->netifIdx is %d",pPktInfo->netifIdx);
					pGreEntry=_rtk_rg_lookupPPTPOutboundGreFlow(pPktInfo->netifIdx,pPktInfo->ipv4Sip,pPktInfo->pPptpCallId->cid1,pPktInfo->ipv4Dip);
					if(pGreEntry==NULL)
					{
						usableCallID=_rtk_rg_PPTPExtCallIDGetAndUse(pPktInfo->pPptpCallId->cid1);
						if(usableCallID==RG_RET_FAIL)
						{
							DEBUG("PPTP_OutCallRequest: Thers is no usable CallID now...");
							return RG_RET_FAIL;
						}
						//Setup this entry
						pGreEntry=&rg_db.pPPTPGreOutboundHead[pPktInfo->netifIdx]->greEntry;

						bzero(pGreEntry,sizeof(rtk_rg_pptpGreEntry_t));
						pGreEntry->internalIpAddr=pPktInfo->ipv4Sip;
						memcpy(pGreEntry->internalMacAddr.octet,pPktInfo->pSmac,ETHER_ADDR_LEN);
						pGreEntry->remoteIpAddr=pPktInfo->ipv4Dip;
						pGreEntry->internalCallID=pPktInfo->pPptpCallId->cid1;
						//Find out usable extCallID and keep it in greEntry
						pGreEntry->externalCallID=usableCallID;
						pGreEntry->valid=1;
						DEBUG("@@@the GreEntry has been set in PRE:internalIP=%08x, internalMac=%02x:%02x:%02x:%02x:%02x:%02x, internalCallID is %d, externalCallID is %d",
							pGreEntry->internalIpAddr,
							pGreEntry->internalMacAddr.octet[0],
							pGreEntry->internalMacAddr.octet[1],
							pGreEntry->internalMacAddr.octet[2],
							pGreEntry->internalMacAddr.octet[3],
							pGreEntry->internalMacAddr.octet[4],
							pGreEntry->internalMacAddr.octet[5],
							pGreEntry->internalCallID,pGreEntry->externalCallID);						

						rg_db.pPPTPGreOutboundHead[pPktInfo->netifIdx]=rg_db.pPPTPGreOutboundHead[pPktInfo->netifIdx]->pNext;
					}

					//Replace internal CallID to external CallID
					pPktInfo->pPptpCallId->cid1=pGreEntry->externalCallID;
					break;
				case PPTP_CallClearRequest:
					DEBUG("$$$$ PRE PPTP_CallClearRequest  pPktInfo->netifIdx is %d",pPktInfo->netifIdx);
					pGreEntry=_rtk_rg_lookupPPTPOutboundGreFlow(pPktInfo->netifIdx,pPktInfo->ipv4Sip,pPktInfo->pPptpCallId->cid1,pPktInfo->ipv4Dip);
					if(pGreEntry==NULL)
					{
						DEBUG("PPTP_CallClearRequest: Error..we can not find the GRE entry ..");
						return RG_RET_FAIL;
					}
					
					//Replace internal CallID to external CallID
					pPktInfo->pPptpCallId->cid1=pGreEntry->externalCallID;
					break;
				default:
					break;
			}
			
		}
		else
		{
			//INBOUND
			switch(pPktInfo->pptpCtrlType)
			{
				case PPTP_OutCallReply:
					//Keep remoteCallID
					DEBUG("$$$$ PPTP_OutCallReply  pPktInfo->netifIdx is %d",pPktInfo->netifIdx);
					pGreEntry=_rtk_rg_lookupPPTPOutboundGreFlowByCallID(pPktInfo->netifIdx,pPktInfo->pPptpCallId->cid2,-1,pPktInfo->ipv4Sip);		//peer's CallID
					if(pGreEntry==NULL)
					{
						DEBUG("PPTP_OutCallReply: Error..we can not find the GRE entry from ExternalCallID %d..",pPktInfo->pPptpCallId->cid2);
						return RG_RET_FAIL;
					}
					pGreEntry->remoteCallID=pPktInfo->pPptpCallId->cid1;
					DEBUG("@@@ the GreEntry has been set in PRE:retmoteCallID is %d",pGreEntry->remoteCallID);
					//Replace externalCallID by internalCallID
					pPktInfo->pPptpCallId->cid2=pGreEntry->internalCallID;
					break;
				case PPTP_WanErrorNotify:
					//replace peer's CallID to internalCallID
					pGreEntry=_rtk_rg_lookupPPTPOutboundGreFlowByCallID(pPktInfo->netifIdx,pPktInfo->pPptpCallId->cid1,-1,pPktInfo->ipv4Sip);		//peer's CallID
					if(pGreEntry==NULL)
					{
						DEBUG("PPTP_WanErrorNotify: Error..we can not find the GRE entry from ExternalCallID %d..",pPktInfo->pPptpCallId->cid1);
						return RG_RET_FAIL;
					}
					//Replace externalCallID by internalCallID
					pPktInfo->pPptpCallId->cid1=pGreEntry->internalCallID;
					break;
				case PPTP_CallDiscNotify:
					//release GRE entry if we receive CallDisconnectNotify from WAN interface
					DEBUG("$$$$ PPTP_CallDiscNotify  pPktInfo->netifIdx is %d",pPktInfo->netifIdx);
					pGreEntry=_rtk_rg_lookupPPTPOutboundGreFlowByCallID(pPktInfo->netifIdx,-1,pPktInfo->pPptpCallId->cid1,pPktInfo->ipv4Sip);
					if(pGreEntry==NULL)
					{
						DEBUG("PPTP_CallDiscNotify: Error..we can not find the GRE entry from RemoteCallID %d..",pPktInfo->pPptpCallId->cid1);
						return RG_RET_FAIL;
					}
					pGreEntry->valid=0;
					break;
				default:
					break;
			}
		}
	}
	else
	{
		//Post function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			switch(pPktInfo->pptpCtrlType)
			{
				case PPTP_OutCallRequest:
					//use ExtCallID to find match entry
					DEBUG("$$$$ POST PPTP_OutCallRequest  pPktInfo->netifIdx is %d",pPktInfo->netifIdx);
					pGreEntry=_rtk_rg_lookupPPTPOutboundGreFlowByCallID(pPktInfo->netifIdx,pPktInfo->pPptpCallId->cid1,-1,pPktInfo->ipv4Dip);
					if(pGreEntry==NULL)
					{
						DEBUG("PPTP_OutCallRequest: Error..we can not find the GRE entry from ExternalCallID %d..",pPktInfo->pPptpCallId->cid1);
						return RG_RET_FAIL;
					}
					//Keep remoteIpAddr and MAC
					//pGreEntry->remoteIpAddr=pPktInfo->ipv4Dip;
					memcpy(pGreEntry->remoteMacAddr.octet,pPktInfo->pDmac,ETHER_ADDR_LEN);
					DEBUG("@@@ the GreEntry has been set in PRE:remoteIP is %08x, remoteMac is %02x:%02x:%02x:%02x:%02x:%02x",
						pGreEntry->remoteIpAddr,pGreEntry->remoteMacAddr.octet[0],pGreEntry->remoteMacAddr.octet[1],pGreEntry->remoteMacAddr.octet[2],
						pGreEntry->remoteMacAddr.octet[3],pGreEntry->remoteMacAddr.octet[4],pGreEntry->remoteMacAddr.octet[5]);
					break;
				default:
					break;
			}
		}
		else
		{
			//INBOUND
			switch(pPktInfo->pptpCtrlType)
			{
				case PPTP_CallDiscNotify:
					//release GRE entry if we receive CallDisconnectNotify from LAN interface
					DEBUG("$$$$ PPTP_CallDiscNotify  pPktInfo->netifIdx is %d",pPktInfo->netifIdx);
					pGreEntry=_rtk_rg_lookupPPTPOutboundGreFlowByCallID(pPktInfo->netifIdx,-1,pPktInfo->pPptpCallId->cid1,pPktInfo->ipv4Sip);
					if(pGreEntry==NULL)
					{
						DEBUG("PPTP_CallDiscNotify: Error..we can not find the GRE entry from RemoteCallID %d..",pPktInfo->pPptpCallId->cid1);
						return RG_RET_FAIL;
					}
					pGreEntry->valid=0;
					break;
				default:
					break;
			}
		}
	}

#endif
	return RG_RET_SUCCESS;
}
#endif

#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
rtk_rg_fwdEngineAlgReturn_t _rtk_rg_algRegFunc_TCP_BattleNet(int direct, int after, unsigned char *pSkb,unsigned char *pPktHdr)
{
	rtk_rg_pktHdr_t *pPktInfo;
	unsigned char *pData;
	struct sk_buff *skb;
	int dlen;
	int doff;
	int i;
	
	skb= (struct sk_buff *)pSkb;		
	pData=(unsigned char *)skb->data;
	pPktInfo = (rtk_rg_pktHdr_t *)pPktHdr;

	if(after==1)
	{
		memDump(pData,skb->len,"BNCS data[Before]");
		doff = (pPktInfo->l4Offset+pPktInfo->headerLen);
		dlen = skb->len-doff;
		DEBUG("BNCS header offset:%d BNCS data length:%d\n",doff,dlen);
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			//Outbound
			if(pPktInfo->tagif&TCP_TAGIF)
			{
				pData+=doff;
				if(pData[0]==0xff) //Header : BNCS
				{
					if((pData[1]==0x09) || (pData[1]==0x50)) //SID_GETADVLISTEX or SID_AUTH_INFO
					{
						int i;
						for(i=0;i<dlen;i++)
						{
							if(memcmp(&pData[i],(void *)(&pPktInfo->ipv4Sip),4)==0)
							{
								memcpy(&pData[i],pPktInfo->pIpv4Sip,4);
								memDump(skb->data,skb->len,"BNCS data[After]");
								break;
							}
						}
					}
				}
			}
		}
		else
		{
			//Inbound
			if(*(pData+doff)==0xff) //Header : BNCS
			{
				if((*(pData+doff+1)==0x09) || (*(pData+doff+1)==0x50)) //SID_GETADVLISTEX or SID_AUTH_INFO
				{
					rtk_rg_upnpConnection_t upnp;
					int upnpIdx;
					for(i=0;i<RTK_RG_MAX_SC_CLIENT;i++)
					{
						if(rg_db.algBnetSCClient[i]==ntohl(*pPktInfo->pIpv4Dip)) break;
						if(rg_db.algBnetSCClient[i]!=0) continue;
						
						rg_db.algBnetSCClient[i]=ntohl(*pPktInfo->pIpv4Dip);
						memset(&upnp,0,sizeof(rtk_rg_upnpConnection_t));
						upnp.is_tcp=0;
						upnp.valid=1;
						upnp.wan_intf_idx=pPktInfo->netifIdx;
						upnp.gateway_port=6112;
						upnp.local_ip=rg_db.algBnetSCClient[i];
						upnp.local_port=6112;
						upnp.limit_remote_ip=0;
						upnp.limit_remote_port=0;
						upnp.remote_ip=0;
						upnp.remote_port=0;
						upnp.type=UPNP_TYPE_PERSIST;
						upnp.timeout=rg_db.algUserDefinedTimeout[RTK_RG_ALG_BATTLENET_TCP];	//auto time out if the server do not connect to this WAN
						ASSERT_EQ((pf.rtk_rg_upnpConnection_add)(&upnp,&upnpIdx),RT_ERR_RG_OK);
					}
				}
			}
		}
	}
	return RG_FWDENGINE_ALG_RET_SUCCESS;
}
#endif

//LUKE20130816: move to separate ALG file
#if 0
rtk_rg_ftpCtrlFlowEntry_t *_rtk_rg_lookupFTPCtrlFlow(unsigned int internalIp,unsigned short int internalPort, unsigned int remoteIp, unsigned short int remotePort)
{
	rtk_rg_ftpCtrlFlowEntry_t *pEntry=NULL;

	/*DEBUG("the input is %s, int %x,%d ret %x,%d",
		isTcp==1?"TCP":"UDP",
		internalIp,internalPort,
		remoteIp,remotePort);*/
	
	for (pEntry = rg_db.pAlgFTPCtrlFlowHead->pPrev; pEntry != rg_db.pAlgFTPCtrlFlowHead; pEntry = pEntry->pPrev)
	{
		/*DEBUG("the entry is %s, int %x,%d ret %x,%d",
			pEntry->isTcp==1?"TCP":"UDP",
			pEntry->internalIpAddr,pEntry->internalPort,
			pEntry->remoteIpAddr,pEntry->remotePort);*/
		if((pEntry->internalIpAddr==internalIp)&&
			(pEntry->internalPort==internalPort)&&
			(pEntry->remoteIpAddr==remoteIp)&&
			(pEntry->remotePort==remotePort))
			break;
	}
	if(pEntry == rg_db.pAlgFTPCtrlFlowHead)		//not found
		return NULL;

	return pEntry;
}

int _rtk_rg_algRegFunc_TCP_FTP(int direct, int after, unsigned char *pSkb,unsigned char *pPktHdr)
{
//FIXME:here should be rewrite for model code
#ifdef __KERNEL__
	char *startCP,*endCP,*pData;
	char portString[30]={0};
	unsigned int ipAddr,gwIPAddr;
	int newLen,newDelta;
	unsigned short portNum;
	int dataLen=0,dataOff=0,ret,flowIdx;
	rtk_rg_naptEntry_t naptEntry;
	rtk_rg_pktHdr_t *pPktInfo;
	struct sk_buff *skb;
	rtk_rg_ftpCtrlFlowEntry_t *pFtpCtrlFlow;
	pPktInfo = (rtk_rg_pktHdr_t *)pPktHdr;
	skb= (struct sk_buff *)pSkb;

	pData=skb->data;
	
	dataOff = pPktInfo->l4Offset + pPktInfo->headerLen;
	//if(pPktInfo->tagif&PPPOE_TAGIF)dataOff+=8;		//shift for pppoe packet
	dataLen = skb->len - dataOff;

	//DEBUG("the l4offset is %d, headerlen is %d, skb->len is %d, dataLen is %d, dataoff is %d",pPktInfo->l4Offset,pPktInfo->headerLen,skb->len,dataLen,dataOff);

	if(after==0)
	{
		//Pre function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			//DEBUG("the data is %x, port is %x",*((unsigned int *)(pData+dataOff)),htonl(FTP_PORT_STR));

			//Check if we had add FTP-flow 5-tuple link list entry
			//if not, add one, set Delta to 0
			//check each list entry, if not add, add
			pFtpCtrlFlow = _rtk_rg_lookupFTPCtrlFlow(pPktInfo->ipv4Sip,pPktInfo->sport,pPktInfo->ipv4Dip,pPktInfo->dport);
			if(pFtpCtrlFlow == NULL)
			{
				//DEBUG("ADD NEW CTRL FLOW of FTP!!DIP is %x, DPORT is %d",pPktInfo->ipv4Dip,pPktInfo->dport);
				//add one
				rg_db.pAlgFTPCtrlFlowHead->remoteIpAddr=pPktInfo->ipv4Dip;
				rg_db.pAlgFTPCtrlFlowHead->internalIpAddr=pPktInfo->ipv4Sip;
				rg_db.pAlgFTPCtrlFlowHead->remotePort=pPktInfo->dport;
				rg_db.pAlgFTPCtrlFlowHead->internalPort=pPktInfo->sport;

				//move to next one
				rg_db.pAlgFTPCtrlFlowHead = rg_db.pAlgFTPCtrlFlowHead->pNext;
			}
			else if((pPktInfo->tcpFlags.syn==1)&&(pPktInfo->tcpFlags.ack==0))	//If the same Ctrl Flow send SYN packet, we need to reset Delta to 0
			{	
				//DEBUG("Got SYN at same ctrl-flow..reset Delta!");
				pFtpCtrlFlow->Delta=0;
			}
			else if(pFtpCtrlFlow->Delta != 0)
			{
				*pPktInfo->pTcpSeq=htonl(pPktInfo->tcpSeq + pFtpCtrlFlow->Delta);
				//DEBUG("pre outbound Delta is %d",pFtpCtrlFlow->Delta);
			}
		}
		else
		{
			//Do nothing
		}
	}
	else
	{
		//Post function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{			
			if(dataLen > 0 && *((unsigned int *)(pData+dataOff)) == htonl(FTP_PORT_STR))
			{
				//DEBUG("len is %d, l4offset is %d, headerlen is %d",dataLen,pPktInfo->l4Offset, pPktInfo->headerLen);
				//DEBUG("POST_FUNCTION outbound: i am in _rtk_rg_algRegFunc_TCP_FTP!! data len is %d dport is %d",dataLen,pPktInfo->dport);
				//DEBUG("DATA is:");

				startCP=pData+dataOff+5;		//bypass string "PORT_"
				ipAddr = simple_strtoul(startCP,&endCP,0);

				ipAddr<<=8;
	
				startCP=endCP+1;
				ipAddr += simple_strtoul(startCP,&endCP,0);

				ipAddr<<=8;
	
				startCP=endCP+1;
				ipAddr += simple_strtoul(startCP,&endCP,0);

				ipAddr<<=8;
	
				startCP=endCP+1;
				ipAddr += simple_strtoul(startCP,&endCP,0);
				
				//DEBUG("the ipAddr is %x",ipAddr);
	
				startCP=endCP+1;
				portNum = simple_strtoul(startCP,&endCP,0);

				portNum<<=8;

				startCP=endCP+1;
				portNum += simple_strtoul(startCP,&endCP,0);

				//DEBUG("the port is %d",portNum);

				gwIPAddr=ntohl(*pPktInfo->pIpv4Sip);
				snprintf(portString,sizeof(portString),"PORT %d,%d,%d,%d,%d,%d\r\n",
					(gwIPAddr>>24)&0xff,
					(gwIPAddr>>16)&0xff,
					(gwIPAddr>>8)&0xff,
					gwIPAddr&0xff,
					portNum>>8,
					portNum&0xff);

				//DEBUG("the modified port command is \"%s\"",portString);
				//DEBUG("data before len is %d, after is %d",dataLen,strlen(portString));

				newLen=strlen(portString);
				newDelta=(newLen-dataLen);
				//DEBUG("the newDelta is %d",newDelta);
				
				if(skb->tail+newDelta > skb->end)	
				{
					//need to re-alloc skb data structure
					FIXME("Overflow! we need to re-alloc skb data stucture...");
				}
				else
				{
					//copy the new string into skb and enlarge or shrink the length
					memcpy(skb->data+dataOff,portString,newLen);

					//fix packet length
					//DEBUG("sip is %x, sport is %d",pPktInfo->ipv4Sip,pPktInfo->sport);					
					if(newDelta != 0)
					{
						skb->len += newDelta;
						*pPktInfo->pL3Len=htons(pPktInfo->l3Len + newDelta);
					}
				}

				pFtpCtrlFlow = _rtk_rg_lookupFTPCtrlFlow(pPktInfo->ipv4Sip,pPktInfo->sport,pPktInfo->ipv4Dip,pPktInfo->dport);
				if(pFtpCtrlFlow!=NULL)pFtpCtrlFlow->Delta+=newDelta;
			
				//Add to napt connection here
				bzero(&naptEntry,sizeof(naptEntry));
				naptEntry.is_tcp=1;
				naptEntry.local_ip=pPktInfo->ipv4Sip;
				naptEntry.remote_ip=pPktInfo->ipv4Dip;
				naptEntry.local_port=portNum;
				naptEntry.remote_port=RTK_RG_ALG_FTP_DATA_TCP_PORT;
				naptEntry.external_port=portNum;
				naptEntry.wan_intf_idx=pPktInfo->extipIdx;
				ret = rtk_rg_naptConnection_add(&naptEntry,&flowIdx);
				if(ret!=RT_ERR_RG_OK && ret!=RT_ERR_RG_NAPT_FLOW_DUPLICATE)
					assert_ok(ret);
			}
		}
		else
		{
			pFtpCtrlFlow = _rtk_rg_lookupFTPCtrlFlow(*pPktInfo->pIpv4Dip,*pPktInfo->pDport,pPktInfo->ipv4Sip,pPktInfo->sport);

			if(pFtpCtrlFlow != NULL && pFtpCtrlFlow->Delta != 0)
			{
				//DEBUG("post inbound Delta is %d, old ack is %x",pFtpCtrlFlow->Delta,*pPktInfo->pTcpAck);
				*pPktInfo->pTcpAck=htonl(pPktInfo->tcpAck - pFtpCtrlFlow->Delta);
				//DEBUG("new ack is %x",*pPktInfo->pTcpAck);
			}
		}
	}
#endif
	return RG_RET_SUCCESS;
}


int _rtk_rg_algSrvInLanRegFunc_TCP_FTP(int direct, int after, unsigned char *pSkb,unsigned char *pPktHdr)
{
//FIXME:here should be rewrite for model code
#ifdef __KERNEL__
	char *startCP,*endCP,*pData;
	char portString[60]={0};
	unsigned int ipAddr,gwIPAddr;
	int i,newLen,newDelta;
	unsigned short portNum,newPort;
	int dataLen=0,dataOff=0,ret;//,flowIdx;
	//rtk_rg_naptEntry_t naptEntry;
	rtk_rg_pktHdr_t *pPktInfo;
	struct sk_buff *skb;
	rtk_rg_ftpCtrlFlowEntry_t *pFtpCtrlFlow;
	rtk_rg_upnpConnection_t upnpConn;

	
	pPktInfo = (rtk_rg_pktHdr_t *)pPktHdr;
	skb= (struct sk_buff *)pSkb;

	pData=skb->data;
	
	dataOff = pPktInfo->l4Offset + pPktInfo->headerLen;
	//if(pPktInfo->tagif&PPPOE_TAGIF)dataOff+=8;		//shift for pppoe packet
	dataLen = skb->len - dataOff;
	
	//DEBUG("the l4offset is %d, headerlen is %d, skb->len is %d, dataLen is %d, dataoff is %d",pPktInfo->l4Offset,pPktInfo->headerLen,skb->len,dataLen,dataOff);

	if(after==0)
	{
		//Pre function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			pFtpCtrlFlow = _rtk_rg_lookupFTPCtrlFlow(pPktInfo->ipv4Sip,pPktInfo->sport,pPktInfo->ipv4Dip,pPktInfo->dport);

			if(pFtpCtrlFlow != NULL && pFtpCtrlFlow->Delta != 0)
			{
				*pPktInfo->pTcpSeq=htonl(pPktInfo->tcpSeq + pFtpCtrlFlow->Delta);
				//DEBUG("post outbound Delta is %d",pFtpCtrlFlow->Delta);
			}
		}
		else
		{
			//Do nothing
		}
	}
	else
	{
		//Post function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			if(dataLen > 0 && *((unsigned int *)(pData+dataOff)) == htonl(FTP_PASV_RESP_STR))		//"227_"
			{
				//DEBUG("len is %d, l4offset is %d, headerlen is %d",dataLen,pPktInfo->l4Offset, pPktInfo->headerLen);
				//DEBUG("POST_FUNCTION outbound: i am in _rtk_rg_algSrvInLanRegFunc_TCP_FTP!! data len is %d dport is %d",dataLen,pPktInfo->dport);
				//DEBUG("DATA is:");

				startCP=pData+dataOff+27;	//bypass string "227_Entering_Passive_Mode_("
				ipAddr = simple_strtoul(startCP,&endCP,0);

				ipAddr<<=8;
	
				startCP=endCP+1;
				ipAddr += simple_strtoul(startCP,&endCP,0);

				ipAddr<<=8;
	
				startCP=endCP+1;
				ipAddr += simple_strtoul(startCP,&endCP,0);

				ipAddr<<=8;
	
				startCP=endCP+1;
				ipAddr += simple_strtoul(startCP,&endCP,0);
				
				//DEBUG("the ipAddr is %x",ipAddr);
	
				startCP=endCP+1;
				portNum = simple_strtoul(startCP,&endCP,0);

				portNum<<=8;

				startCP=endCP+1;
				portNum += simple_strtoul(startCP,&endCP,0);

				//DEBUG("the port is %d",portNum);

				//Chekc if the external port could be used or not
				newPort=_rtk_rg_extPort_get(1,portNum);
				if(newPort==RG_RET_FAIL) return RG_RET_SUCCESS;	
				
				//DEBUG("the new port is %d",newPort);

				gwIPAddr=ntohl(*pPktInfo->pIpv4Sip);
				snprintf(portString,sizeof(portString),"227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)\r\n",
					(gwIPAddr>>24)&0xff,
					(gwIPAddr>>16)&0xff,
					(gwIPAddr>>8)&0xff,
					gwIPAddr&0xff,
					newPort>>8,
					newPort&0xff);

				//DEBUG("the modified port response is\"%s\"",portString);
				//DEBUG("data before len is %d, after is %d",dataLen,strlen(portString));

				newLen=strlen(portString);
				newDelta=(newLen-dataLen);
				//DEBUG("the newDelta is %d",newDelta);
				
				if(skb->tail+newDelta > skb->end)	
				{
					//need to re-alloc skb data structure
					FIXME("Overflow! we need to re-alloc skb data stucture...");
				}
				else
				{
					//copy the new string into skb and enlarge or shrink the length
					memcpy(skb->data+dataOff,portString,newLen);

					//fix packet length
					//DEBUG("sip is %x, sport is %d",pPktInfo->ipv4Sip,pPktInfo->sport);					
					if(newDelta != 0)
					{
						skb->len += newDelta;
						*pPktInfo->pL3Len=htons(pPktInfo->l3Len + newDelta);
					}
				}

				pFtpCtrlFlow = _rtk_rg_lookupFTPCtrlFlow(pPktInfo->ipv4Sip,pPktInfo->sport,pPktInfo->ipv4Dip,pPktInfo->dport);
				if(pFtpCtrlFlow!=NULL)pFtpCtrlFlow->Delta+=newDelta;
			
				//Add a one-shot UPnP flow for incoming connection to each L4 WAN
				for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
				{
					if(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type!=RTK_RG_BRIDGE && 
						rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->napt_enable)		//L4 WAN
					{
						upnpConn.is_tcp=1;
						upnpConn.wan_intf_idx=rg_db.systemGlobal.wanIntfGroup[i].index;
						upnpConn.gateway_port=newPort;
						upnpConn.local_ip=ipAddr;
						upnpConn.local_port=portNum;
						upnpConn.limit_remote_ip=1;
						upnpConn.limit_remote_port=0;		//can't restrict client using which port to connect
						upnpConn.remote_ip=pPktInfo->ipv4Dip;
						upnpConn.remote_port=0;
						upnpConn.type=UPNP_TYPE_ONESHOT;
						upnpConn.timeout=FTP_PASV_UPNP_TIMEOUT;	//auto time out after 10 second if the client do not connect by this WAN
						assert_ok(rtk_rg_upnpConnection_add(&upnpConn,&ret));
					}
				}				
			}
		}
		else
		{
			//DEBUG("the data is %x, port is %x",*((unsigned int *)(pData+dataOff)),htonl(FTP_PORT_STR));

			//Check if we had add FTP-flow 5-tuple link list entry
			//if not, add one, set Delta to 0
			//check each list entry, if not add, add
			pFtpCtrlFlow = _rtk_rg_lookupFTPCtrlFlow(*pPktInfo->pIpv4Dip,*pPktInfo->pDport,pPktInfo->ipv4Sip,pPktInfo->sport);
			if(pFtpCtrlFlow == NULL)
			{
				//DEBUG("ADD NEW CTRL FLOW of SrvInLan FTP!!DIP is %x, DPORT is %d",*pPktInfo->pIpv4Dip,*pPktInfo->pDport);
				//add one
				rg_db.pAlgFTPCtrlFlowHead->remoteIpAddr=pPktInfo->ipv4Sip;
				rg_db.pAlgFTPCtrlFlowHead->internalIpAddr=*pPktInfo->pIpv4Dip;
				rg_db.pAlgFTPCtrlFlowHead->remotePort=pPktInfo->sport;
				rg_db.pAlgFTPCtrlFlowHead->internalPort=*pPktInfo->pDport;

				//move to next one
				rg_db.pAlgFTPCtrlFlowHead = rg_db.pAlgFTPCtrlFlowHead->pNext;
			}
			else if((pPktInfo->tcpFlags.syn==1)&&(pPktInfo->tcpFlags.ack==0))	//If the same Ctrl Flow send SYN packet, we need to reset Delta to 0
			{	
				//DEBUG("Got SYN at same ctrl-flow..reset Delta!");
				pFtpCtrlFlow->Delta=0;
			}
			else if(pFtpCtrlFlow->Delta != 0)
			{
				//DEBUG("post inbound Delta is %d, old ack is %x",pFtpCtrlFlow->Delta,*pPktInfo->pTcpAck);
				*pPktInfo->pTcpAck=htonl(pPktInfo->tcpAck - pFtpCtrlFlow->Delta);
			}
		}
	}
#endif
	return RG_RET_SUCCESS;
}
#endif
void _rtk_rg_l2tpTunnelIDFree(uint16 tunnID)
{
	int idx;
	uint32 bitValue;
	int i;

	i=tunnID;
	idx=i>>5; // =port/32
	bitValue=1<<(i&0x1f);
	if((rg_db.algL2TPExternTulIDUsed[idx]&bitValue)>0)
	{
		rg_db.algL2TPExternTulIDUsed[idx]&=(~bitValue);
	}			
}

rtk_rg_lookupIdxReturn_t _rtk_rg_l2tpTunnelIDGetAndUse(uint16 wishTunnID)
{
	int wishIdx;
	uint32 wishBitValue;
	int i;

	i=wishTunnID;

	while(1)
	{
		wishIdx=i>>5; // =wishPort/32
		wishBitValue=1<<(i&0x1f);
		
		if(((rg_db.algL2TPExternTulIDUsed[wishIdx]&wishBitValue)==0))
		{
			rg_db.algL2TPExternTulIDUsed[wishIdx]|=wishBitValue;
			return i;
		}
		
		i++;
		i&=0xffff;
		if(i==wishTunnID) break;		
	}	
	return RG_RET_LOOKUPIDX_NOT_FOUND;
}


rtk_rg_alg_l2tp_flow_t *_rtk_rg_lookupL2TPCtrlFlow(unsigned int internalIp,unsigned short int internalID, unsigned int remoteIp, unsigned short int externalID)
{
	rtk_rg_alg_l2tp_linkList_t *pEntry=NULL;

	DEBUG("the input is: int %x,%d ret %x,%d",internalIp,internalID,remoteIp,externalID);
	
	for (pEntry = rg_db.pAlgL2TPCtrlFlowHead->pPrev; pEntry != rg_db.pAlgL2TPCtrlFlowHead; pEntry = pEntry->pPrev)
	{
		if(pEntry->l2tpFlow.valid)
			DEBUG("the entry is %s int %x,%d ret %x,%d",pEntry->l2tpFlow.valid==1?"VALID":"INVALID",pEntry->l2tpFlow.internalIP,pEntry->l2tpFlow.IntTulID,pEntry->l2tpFlow.remoteIP,pEntry->l2tpFlow.ExtTulID);
		if(pEntry->l2tpFlow.valid &&
			(internalIp==0 || pEntry->l2tpFlow.internalIP==internalIp)&&
			(internalID==0 || pEntry->l2tpFlow.IntTulID==internalID)&&
			(pEntry->l2tpFlow.remoteIP==remoteIp)&&
			(externalID==0 || pEntry->l2tpFlow.ExtTulID==externalID))
			break;
	}
	if(pEntry == rg_db.pAlgL2TPCtrlFlowHead)		//not found
		return NULL;

	return &pEntry->l2tpFlow;
}


rtk_rg_fwdEngineAlgReturn_t _rtk_rg_algRegFunc_UDP_L2TP(int direct, int after, unsigned char *pSkb,unsigned char *pPktHdr)
{
//FIXME:here should be rewrite for model code
#ifdef __KERNEL__
	char *pData;
	//char portString[60]={0};
	//unsigned int ipAddr,gwIPAddr;
	//int i,newLen,newDelta;
	//unsigned short portNum,newPort;
	int dataLen=0,dataOff=0,hdrLength;
	//rtk_rg_naptEntry_t naptEntry;
	rtk_rg_pktHdr_t *pPktInfo;
	struct sk_buff *skb;	

	rtk_rg_alg_l2tp_flow_t *pL2TPCtrlFlow;
	rtk_rg_alg_l2tp_ctrlHeader_t *pLl2tpHdr;
	rtk_rg_alg_l2tp_avpHeader_t *pAvpHdr;
	unsigned char *pAvpNext;
	
	
	pPktInfo = (rtk_rg_pktHdr_t *)pPktHdr;
	skb= (struct sk_buff *)pSkb;

	pData=skb->data;
	
	dataOff = pPktInfo->l4Offset + 8;//pPktInfo->headerLen;		//UDP header length are always 8 bytes
	//if(pPktInfo->tagif&PPPOE_TAGIF)dataOff+=8;		//shift for pppoe packet
	dataLen = skb->len - dataOff;
	pLl2tpHdr = (rtk_rg_alg_l2tp_ctrlHeader_t *)(pData+dataOff);
	
	DEBUG("the l4offset is %d, headerlen is %d, skb->len is %d, dataLen is %d, dataoff is %d",pPktInfo->l4Offset,pPktInfo->headerLen,skb->len,dataLen,dataOff);

	if(after==0)
	{
		//Pre function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			//Check if this packet is SCCRQ(TunnelID=0, SessionID=0, has Assigned TunnelID AVP)
			if(dataLen > 0 && pLl2tpHdr->flag==htons(L2TP_CTRL_MSG_STR))
			{
				pAvpHdr=(rtk_rg_alg_l2tp_avpHeader_t *)(pLl2tpHdr+1);
				if(pLl2tpHdr->tunnel_id==0 && pLl2tpHdr->session_id==0)
				{
					//Check each AVP for (Assigned Tunnel ID AVP), true value length = length - 6
					if(pAvpHdr->attType==0 && *(unsigned short *)(pAvpHdr+1)==1)	//control Message: Start_Control_Request
					{
						hdrLength=pLl2tpHdr->length - sizeof(rtk_rg_alg_l2tp_ctrlHeader_t);
						while(hdrLength>0)
						{
							if(pAvpHdr->attType==9)		//Assigned Tunnel ID
							{
								DEBUG("AVP: Assigned Tunnel ID!!");
								//Check if the assigned tunnel ID had been used
								pL2TPCtrlFlow=_rtk_rg_lookupL2TPCtrlFlow(pPktInfo->ipv4Sip,*(unsigned short *)(pAvpHdr+1),pPktInfo->ipv4Dip,0);	//since the external ID is not decide, use 0 to bypass this check
								if(pL2TPCtrlFlow==NULL)
								{
									//if so, replace it, otherwise keep the ID untouched
									//create new control flow for l2tp
									
									rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.remoteIP=pPktInfo->ipv4Dip;
									rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.internalIP=pPktInfo->ipv4Sip;
									rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.IntTulID=*(unsigned short *)(pAvpHdr+1);								
									rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.ExtTulID=_rtk_rg_l2tpTunnelIDGetAndUse(rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.IntTulID);
									if(rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.ExtTulID==RG_RET_LOOKUPIDX_NOT_FOUND)return RG_FWDENGINE_ALG_RET_FAIL;
									if(rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.IntTulID!=rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.ExtTulID)
										*(unsigned short *)(pAvpHdr+1)=rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.ExtTulID;
									
									DEBUG("ADD NEW CTRL FLOW of L2TP!!SIP is %x, DIP is %x, IntTunnID is %d, ExtTunnID is %d",pPktInfo->ipv4Sip,pPktInfo->ipv4Dip,rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.IntTulID,rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.ExtTulID);

									//move to next one
									rg_db.pAlgL2TPCtrlFlowHead->l2tpFlow.valid=1;
									rg_db.pAlgL2TPCtrlFlowHead = rg_db.pAlgL2TPCtrlFlowHead->pNext;
								}
								else
								{
									//Change from old id to external id
									*(unsigned short *)(pAvpHdr+1)=pL2TPCtrlFlow->ExtTulID;
								}
								break;
							}
							else
							{
								//other AVP, omit it
								pAvpNext=(unsigned char *)pAvpHdr;
								pAvpNext+=pAvpHdr->length;	//move to next avp
								hdrLength-=pAvpHdr->length;	//decrease length to check
								pAvpHdr=(rtk_rg_alg_l2tp_avpHeader_t *)pAvpNext;
								//DEBUG("Check Next AVP...length remain %d",hdrLength);
							}
						}
					}
				}
				else
				{
					if(pAvpHdr->attType==0 && *(unsigned short *)(pAvpHdr+1)==4)	//control Message: Stop_Control_Request
					{
						DEBUG("OUTBOUND!!! StopCCN from %x!!! TunnelID is %d",pPktInfo->ipv4Sip,pLl2tpHdr->tunnel_id);
						pL2TPCtrlFlow=_rtk_rg_lookupL2TPCtrlFlow(pPktInfo->ipv4Sip,pLl2tpHdr->tunnel_id,pPktInfo->ipv4Dip,0);	//since the external ID is unknown, use 0 to bypass this check
						if(pL2TPCtrlFlow!=NULL)
						{
							//release the external ID for next one to use
							_rtk_rg_l2tpTunnelIDFree(pL2TPCtrlFlow->ExtTulID);
						}
					}
				}
			}
		}
		else
		{
			//Do nothing
		}
	}
	else
	{
		//Post function
		if(direct==NAPT_DIRECTION_OUTBOUND)
		{
			//Do nothing
		}
		else
		{
			DEBUG("INBOUND!!! from %x with tunnel ID is %d",pPktInfo->ipv4Sip,pLl2tpHdr->tunnel_id);
			if(pLl2tpHdr->tunnel_id>0)
			{
				pL2TPCtrlFlow=_rtk_rg_lookupL2TPCtrlFlow(0,0,pPktInfo->ipv4Sip,pLl2tpHdr->tunnel_id);
				if(pL2TPCtrlFlow!=NULL)
				{
					//replace internal IP and TunnID
					*pPktInfo->pIpv4Dip=htonl(pL2TPCtrlFlow->internalIP);
					pLl2tpHdr->tunnel_id=pL2TPCtrlFlow->IntTulID;
					DEBUG("INBOUND!!! INTIP changes to %x, TUNNID changes to %d",htonl(pL2TPCtrlFlow->internalIP),pLl2tpHdr->tunnel_id);
					if(dataLen > 0 && pLl2tpHdr->flag==htons(L2TP_CTRL_MSG_STR))
					{
						pAvpHdr=(rtk_rg_alg_l2tp_avpHeader_t *)(pLl2tpHdr+1);
						if(pAvpHdr->attType==0 && *(unsigned short *)(pAvpHdr+1)==4)	//control Message: Stop_Control_Request
						{
							//release the external ID for next one to use
							_rtk_rg_l2tpTunnelIDFree(pL2TPCtrlFlow->ExtTulID);
						}
					}
				}
				else
					DEBUG("pL2TPCtrlFlow is not found, do nothing....");
			}
			else
				DEBUG("pL2TPCtrlFlow is not found, do nothing....");
		}
	}
#endif
	return RG_FWDENGINE_ALG_RET_SUCCESS;
}

rtk_rg_successFailReturn_t _rtk_rg_checkARPForMacLimit(rtk_rg_saLearningLimitProbe_t *limitInfo,rtk_rg_pktHdr_t *pPktHdr)
{
	if(atomic_read(&limitInfo->activity)>0 && limitInfo->arpReq.finished==0)
	{
		//Check IP if match
		if(pPktHdr->ipv4Sip==limitInfo->arpReq.reqIp)
		{
			limitInfo->arpReq.finished=1;
			limitInfo->arpCounter=-1;
			return RG_RET_SUCCESS;
		}
	}
	
	return RG_RET_FAIL;
}

void _rtk_rg_setupL2TP_PPTP_MAC_from_NH(rtk_rg_wan_type_t wanType, int intIdx, int nexthopIdx)
{
	if(wanType==RTK_RG_PPTP)
		_rtk_rg_internal_PPTPMACSetup(rg_db.systemGlobal.intfArpRequest[intIdx].reqIp, rg_db.nexthop[nexthopIdx].rtk_nexthop.nhIdx);
	else
		_rtk_rg_internal_L2TPMACSetup(rg_db.systemGlobal.intfArpRequest[intIdx].reqIp, rg_db.nexthop[nexthopIdx].rtk_nexthop.nhIdx);
}

rtk_rg_successFailReturn_t _rtk_rg_checkARPForL2TP_PPTP(rtk_rg_wanIntfInfo_t *wan_intf, int intIdx, rtk_rg_pktHdr_t *pPktHdr)
{
	int i;
	rtk_rg_interface_info_global_t *intfInfo;

	for(i=0;i<MAX_L3_SW_TABLE_SIZE;i++){
		if(rg_db.l3[i].rtk_l3.valid){
			switch(rg_db.l3[i].rtk_l3.process){
				case L34_PROCESS_NH:
					if(rg_db.nexthop[rg_db.l3[i].rtk_l3.nhStart].rtk_nexthop.ifIdx==pPktHdr->aclPolicyRoute){
						TRACE("get NH from interface!!");
						_rtk_rg_setupL2TP_PPTP_MAC_from_NH(wan_intf->wan_intf_conf.wan_type,intIdx, rg_db.l3[i].rtk_l3.nhStart);
						wan_intf->baseIntf_idx=pPktHdr->aclPolicyRoute;
						return RG_RET_SUCCESS;
					}
					break;
				case L34_PROCESS_ARP:
					{
						int aclPolicyRouteIdx = pPktHdr->aclPolicyRoute;
						if(aclPolicyRouteIdx>0)
						{
							if(rg_db.l3[i].rtk_l3.netifIdx==aclPolicyRouteIdx && rg_db.systemGlobal.interfaceInfo[aclPolicyRouteIdx].valid &&
								rg_db.systemGlobal.interfaceInfo[aclPolicyRouteIdx].p_wanStaticInfo->static_route_with_arp)
							{
								TRACE("get NH from interface which static route with ARP!!");						
								_rtk_rg_setupL2TP_PPTP_MAC_from_NH(wan_intf->wan_intf_conf.wan_type,intIdx, rg_db.systemGlobal.interfaceInfo[aclPolicyRouteIdx].storedInfo.wan_intf.nexthop_ipv4);
								wan_intf->baseIntf_idx=aclPolicyRouteIdx;
								return RG_RET_SUCCESS;
							}
						}
				}
					break;
				case L34_PROCESS_CPU:
					intfInfo=&rg_db.systemGlobal.interfaceInfo[rg_db.l3[i].rtk_l3.netifIdx];
					if(rg_db.l3[i].rtk_l3.netifIdx==pPktHdr->aclPolicyRoute && intfInfo->valid && intfInfo->storedInfo.is_wan){
						if(intfInfo->storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP ||
							intfInfo->storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP ||
							intfInfo->storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE ||
							intfInfo->storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE){
							TRACE("process=CPU, get NH from interface!!");
							_rtk_rg_setupL2TP_PPTP_MAC_from_NH(wan_intf->wan_intf_conf.wan_type,intIdx, intfInfo->storedInfo.wan_intf.nexthop_ipv4);
							wan_intf->baseIntf_idx=pPktHdr->aclPolicyRoute;
							return RG_RET_SUCCESS;
						}else if(rg_db.l3[i].rtk_l3.ipAddr>0 && intfInfo->p_wanStaticInfo->static_route_with_arp){
							TRACE("process=CPU, get NH from interface which static route with ARP!!");
							_rtk_rg_setupL2TP_PPTP_MAC_from_NH(wan_intf->wan_intf_conf.wan_type,intIdx, intfInfo->storedInfo.wan_intf.nexthop_ipv4);
							wan_intf->baseIntf_idx=pPktHdr->aclPolicyRoute;
							return RG_RET_SUCCESS;
						}
					}
					break;
				default:
					break;
			}
		}
	}

	return RG_RET_FAIL;
}

rtk_rg_successFailReturn_t _rtk_rg_arpAgent(u8 *pData, u32 len, rtk_rg_pktHdr_t *pPktHdr)
{
	int i,l2Idx;
	rtk_rg_successFailReturn_t ret=RG_RET_FAIL;
		
	TRACE("ARP Agent, arpOpCode=%d",pPktHdr->arpOpCode);
	pPktHdr->arpAgentCalled=1;
	if((pPktHdr->arpOpCode==2) || (pPktHdr->arpOpCode==1))		//check both ARP reply and request for complete rg add wan interface
	{
#ifdef CONFIG_RG_NAPT_ARP_AUTO_LEARN
		ret=_rtk_rg_arpAndMacEntryAdd(pPktHdr->ipv4Sip,pPktHdr->sipL3Idx,pPktHdr->pSmac,pPktHdr->ingressPort,pPktHdr->wlan_dev_idx,&l2Idx,pPktHdr->internalVlanID,0,1);
#endif

		for(i=0;i<(MAX_NETIF_HW_TABLE_SIZE<<1);i++){
			if(rg_db.systemGlobal.intfArpRequest[i].finished==0){
				TRACE("netif=%d ipv4Sip=%x ipv4Dip=%x reqIp=%x policy route=%d\n",i,pPktHdr->ipv4Sip,pPktHdr->ipv4Dip,rg_db.systemGlobal.intfArpRequest[i].reqIp,pPktHdr->aclPolicyRoute);
				if(pPktHdr->ipv4Sip==rg_db.systemGlobal.intfArpRequest[i].reqIp && ret==SUCCESS){
					if(rg_db.systemGlobal.intfArpRequest[i].gwMacReqCallBack!=NULL){
						rg_db.systemGlobal.intfArpRequest[i].gwMacReqCallBack(rg_db.systemGlobal.intfArpRequest[i].reqIp,l2Idx);
					}
					rg_db.systemGlobal.intfArpRequest[i].finished=1;
					pPktHdr->arpAgentCalled=1;
					return RG_RET_SUCCESS;
				}else if((pPktHdr->ipv4Dip==rg_db.systemGlobal.intfArpRequest[i].reqIp)&&(pPktHdr->aclPolicyRoute>=0)&&
					(rg_db.systemGlobal.interfaceInfo[i-MAX_NETIF_HW_TABLE_SIZE].storedInfo.is_wan)&&
					((rg_db.systemGlobal.interfaceInfo[i-MAX_NETIF_HW_TABLE_SIZE].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP)||
					(rg_db.systemGlobal.interfaceInfo[i-MAX_NETIF_HW_TABLE_SIZE].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP))){	//20150911LUKE: check for policy route may route l2tp/pptp server IP to some other WAN!
					if(_rtk_rg_checkARPForL2TP_PPTP(&rg_db.systemGlobal.interfaceInfo[i-MAX_NETIF_HW_TABLE_SIZE].storedInfo.wan_intf,i,pPktHdr)==RG_RET_SUCCESS)return RG_RET_SUCCESS;
				}
			}
		}
		//lookup for MAC limit req
		if(rg_db.systemGlobal.activeLimitFunction!=RG_ACCESSWAN_TYPE_UNLIMIT){
			//_rtk_rg_checkARPForMacLimit(&rg_kernel.lutReachLimit_port[pPktHdr->ingressPort], pPktHdr);
			//_rtk_rg_checkARPForMacLimit(&rg_kernel.lutReachLimit_wlan0dev[pPktHdr->wlan_dev_idx], pPktHdr);
			_rtk_rg_checkARPForMacLimit(&rg_kernel.lutReachLimit_portmask, pPktHdr);
			for(i=0;i<WanAccessCategoryNum;i++){
				if(_rtk_rg_checkARPForMacLimit(&rg_kernel.lutReachLimit_category[i], pPktHdr)==SUCCESS)
					break;
			}
		}
	}
	return RG_RET_FAIL;
}

rtk_rg_successFailReturn_t _rtk_rg_checkNeighborForMacLimit(rtk_rg_saLearningLimitProbe_t *limitInfo,rtk_rg_pktHdr_t *pPktHdr)
{
	if(atomic_read(&limitInfo->activity)>0 && limitInfo->neighborReq.finished==0)
	{
		//Check IPv6 if match
		if(memcmp(pPktHdr->pIpv6Sip,limitInfo->neighborReq.reqIp.ipv6_addr,IPV6_ADDR_LEN)==0)
		{
			limitInfo->neighborReq.finished=1;
			limitInfo->neighborCounter=-1;
			return RG_RET_SUCCESS;
		}
	}
	
	return RG_RET_FAIL;
}



rtk_rg_successFailReturn_t _rtk_rg_neighborAgent(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	int neighborIdx=-1,macIdx=-1,l3Idx,i,netIfIdx;
	
	TRACE("Neighbor Agent, ICMPv6 type=%d",pPktHdr->ICMPv6Type);
	pPktHdr->neighborAgentCalled=1;
	//Check for Neighbor advertisement packet
	if((rg_db.pktHdr->tagif&ICMPV6_TAGIF) && ((pPktHdr->ICMPv6Type==0x88)||(pPktHdr->ICMPv6Type==0x87)))
	{
		//Check for validation
		if(*pPktHdr->pIPv6HopLimit!=255)	//hop limit should be 255, solicitation flag should be set
			return RG_RET_FAIL;
		
#ifdef CONFIG_RG_IPV6_NEIGHBOR_AUTO_LEARN
		//Learning source IP with source Link-layer address and keep in neighbor table
		_rtk_rg_neighborAndMacEntryAdd(pPktHdr->pIpv6Sip,pPktHdr->sipL3Idx,pPktHdr->pSmac,pPktHdr->ingressPort,pPktHdr->wlan_dev_idx,&neighborIdx);
#endif

		//Check for solicited Neighbor Advetisement packets for complete RG WAN interface addition
		if((pPktHdr->ICMPv6Flag&0x2)==0) //ICMPv6Type=0x88
			return RG_RET_FAIL; //ICMPv6Type=0x87 will exit here.

		for(i=0;i<(MAX_NETIF_SW_TABLE_SIZE<<1);i++)
		{
			if(rg_db.systemGlobal.intfNeighborDiscovery[i].finished==0)
			{
				//over MAX_NETIF_SIZE belong to DSLITE
				if(i>=MAX_NETIF_SW_TABLE_SIZE)
					netIfIdx=i-MAX_NETIF_SW_TABLE_SIZE;
				else
					netIfIdx=i;
				/*
				DEBUG("Neighbor Agent, Intf[%d] request IP:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",i,
					rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[0],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[1],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[2],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[3],
					rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[4],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[5],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[6],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[7],
					rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[8],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[9],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[10],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[11],
					rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[12],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[13],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[14],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr[15]);

				int j;
				DEBUG("the target address is");
				for(j=0;j<16;j++)
					DEBUG("%02x ",skb->data[pPktHdr->l4Offset+8+j]);
				*/
				
				if(memcmp(&skb->data[pPktHdr->l4Offset+8],rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr,IPV6_ADDR_LEN)==0)				
				{
					//FIXME: there may have other options in the future[rfc 4861]
					if(skb->data[pPktHdr->l4Offset+24]!=0x2){	//ICMPv6 option: Target link-layer address
						DEBUG("Neighbor learning fail: ICMPv6 option: Target link-layer address, option data=0x%x",skb->data[pPktHdr->l4Offset+24]);
						return RG_RET_FAIL;
					}
					DEBUG("MATCH!!%x%x%x%x%x%x",skb->data[pPktHdr->l4Offset+26],skb->data[pPktHdr->l4Offset+27],skb->data[pPktHdr->l4Offset+28],skb->data[pPktHdr->l4Offset+29],skb->data[pPktHdr->l4Offset+30],skb->data[pPktHdr->l4Offset+31]);
					//20140826LUKE: handle link-local address
					if(*((unsigned int *)pPktHdr->pIpv6Sip)==0xfe800000 && *((unsigned int *)(pPktHdr->pIpv6Sip+4))==0x0)
					{
						l3Idx=_rtk_rg_v6L3lookup(pPktHdr->pIpv6Dip);
						//DEBUG("Link-local address.. use DIP to find VLAN, %d",l3Idx);
						_rtk_rg_neighborAndMacEntryAdd(pPktHdr->pIpv6Sip,l3Idx,&skb->data[pPktHdr->l4Offset+26],pPktHdr->ingressPort,pPktHdr->wlan_dev_idx,&macIdx);
					}
					else if((i>=MAX_NETIF_SW_TABLE_SIZE)&&(rg_db.systemGlobal.interfaceInfo[netIfIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE))
					{
						DEBUG("DSLITE, we just add mac without neighbor since we don't have ipv6 route");
						_rtk_rg_neighborAndMacEntryAdd(pPktHdr->pIpv6Sip,pPktHdr->sipL3Idx,&skb->data[pPktHdr->l4Offset+26],pPktHdr->ingressPort,pPktHdr->wlan_dev_idx,&macIdx);
					}
					else
					{
						_rtk_rg_neighborAndMacEntryAdd(pPktHdr->pIpv6Sip,pPktHdr->sipL3Idx,&skb->data[pPktHdr->l4Offset+26],pPktHdr->ingressPort,pPktHdr->wlan_dev_idx,&neighborIdx);
						macIdx=rg_db.v6neighbor[neighborIdx].rtk_v6neighbor.l2Idx;
					}					
					//macEntry.vlan_id=RG_GLB_INTF_INFO[i].wan_intf.wan_intf_conf.egress_vlan_id;						
					if(rg_db.systemGlobal.intfNeighborDiscovery[i].ipv6GwMacReqCallBack!=NULL)
					{
						//DEBUG("wan intf is %d, function pointer is %p",i,rg_db.systemGlobal.intfNeighborDiscovery[i].ipv6GwMacReqCallBack);
						//DEBUG("rg_db.v6neighbor[%d].rtk_v6neighbor.l2Idx is %d",neighborIdx,macIdx);
						rg_db.systemGlobal.intfNeighborDiscovery[i].ipv6GwMacReqCallBack(rg_db.systemGlobal.intfNeighborDiscovery[i].reqIp.ipv6_addr,macIdx);
						//DEBUG("");
					}
					rg_db.systemGlobal.intfNeighborDiscovery[i].finished=1;
					return RG_RET_SUCCESS;
				}
			}
		}

		//lookup for MAC limit req
		if(rg_db.systemGlobal.activeLimitFunction!=RG_ACCESSWAN_TYPE_UNLIMIT)
		{
			//_rtk_rg_checkNeighborForMacLimit(&rg_kernel.lutReachLimit_port[pPktHdr->ingressPort], pPktHdr);
			//_rtk_rg_checkNeighborForMacLimit(&rg_kernel.lutReachLimit_wlan0dev[pPktHdr->wlan_dev_idx], pPktHdr);
			_rtk_rg_checkNeighborForMacLimit(&rg_kernel.lutReachLimit_portmask, pPktHdr);
			for(i=0;i<WanAccessCategoryNum;i++)
			{
				if(_rtk_rg_checkNeighborForMacLimit(&rg_kernel.lutReachLimit_category[i], pPktHdr)==RG_RET_SUCCESS)
					break;
			}
		}
	}
	return RG_RET_FAIL;
}

__IRAM_FWDENG_L2
rtk_rg_successFailReturn_t _rtk_rg_portAndProtocolBasedVlanCheck(rtk_rg_pktHdr_t *pPktHdr, int *interVLANId)
{
	int groupID,port;
	rtk_vlan_protoVlanCfg_t *pVlanCfg;
	//Check ingress port(MAC only, no extension port)
	if(pPktHdr->ingressPort>= RTK_RG_PORT_CPU)
	{
		port=RTK_RG_MAC_PORT_CPU;
		//groupID=rg_db.systemGlobal.protoBasedVID[RTK_RG_MAC_PORT_CPU].protoGroupID;
		//pVlanCfg=&rg_db.systemGlobal.protoBasedVID[RTK_RG_MAC_PORT_CPU].protoVLANCfg;
	}
	else
	{
		port=pPktHdr->ingressPort;
		//groupID=rg_db.systemGlobal.protoBasedVID[pPktHdr->ingressPort].protoGroupID;
		//pVlanCfg=&rg_db.systemGlobal.protoBasedVID[pPktHdr->ingressPort].protoVLANCfg;
	}

	//if(pVlanCfg->valid==0)return RG_RET_FAIL;
	
	//Check protocol Group, if match, use the VLAN ID as the ingress ID
	for(groupID=0;groupID<MAX_PORT_PROTO_GROUP_SIZE;groupID++)
	{
		pVlanCfg=&rg_db.systemGlobal.protoBasedVID[port].protoVLANCfg[groupID];
		if(pVlanCfg->valid==0)
			continue;
		
		switch(rg_db.systemGlobal.protoGroup[groupID].frametype)
		{
			case FRAME_TYPE_ETHERNET:
				if(pPktHdr->etherType==rg_db.systemGlobal.protoGroup[groupID].framevalue)
				{
					//TRACE("Protocol Match!!using VLANID %d as ingress VLAN for untag packets!!",pVlanCfg->vid)
					*interVLANId=pVlanCfg->vid;
					return RG_RET_SUCCESS;
				}
				//else
					//DEBUG("Protocol unMatch...pkt=%04x, protoGroup=%04x...",pPktHdr->etherType,rg_db.systemGlobal.protoGroup[groupID].framevalue);
				break;
	    	case FRAME_TYPE_RFC1042:
	    	case FRAME_TYPE_LLCOTHER:
			default:
				//FIXME("ProtoGroup has un-supported frametype %d, fail to check...",rg_db.systemGlobal.protoGroup[groupID].frametype);
				break;
		}
	}

	return RG_RET_FAIL;
	
}

rtk_rg_successFailReturn_t _rtk_rg_ingressVlanDecision(rtk_rg_pktHdr_t *pPktHdr, uint16 *interVLANId)
{
	rtk_rg_successFailReturn_t ret;
	int protoVLANID;
	//The decided internal VLAN ID will be returned by reference of interVLANId
	//FIXME:here should be check ACL before 1Q and other VLAN decision
	//uint8 cVlanPri;
	if(pPktHdr->ingressLocation==RG_IGR_IGMP_OR_MLD)		//from IGMP module, vlan will carried by skb!!
	{
		TRACE("IGMP or MLD will use rxInfoFromIGMPMLD.opts2.bit.cvlan_tag:%d",rg_kernel.rxInfoFromIGMPMLD.opts2.bit.cvlan_tag);
		*interVLANId=rg_kernel.rxInfoFromIGMPMLD.opts2.bit.cvlan_tag;
	}
	else if(pPktHdr->tagif&CVLAN_TAGIF)
	{
		TRACE("1Q-based VLAN %d(by tag)",pPktHdr->ctagVid);
		*interVLANId=pPktHdr->ctagVid;
	}
#if 0
	else if(pPktHdr->ingressLocation==RG_IGR_PROTOCOL_STACK && pPktHdr->pRxDesc->opts2.bit.ctagva)		//PATCH for rtk_rg_fwdEngine_xmit may use ctagva to indicate tagging
	{
		//1Q-based VLAN (by descriptor)
		*interVLANId=((pPktHdr->pRxDesc->opts2.bit.cvlan_tag&0xf)<<0x8)+((pPktHdr->pRxDesc->opts2.bit.cvlan_tag&0xff00)>>0x8);
		TRACE("1Q from Protocol stack, interVLANId=%d\n",*interVLANId);		
		//update for pPktHdr VLAN message, because gw removed vlan: chuck
		pPktHdr->tagif |= CVLAN_TAGIF;
		pPktHdr->ctagVid=*interVLANId;
		pPktHdr->egressVlanID = *interVLANId;
		pPktHdr->ctagPri=((pPktHdr->pRxDesc->opts2.bit.cvlan_tag&0xf0)>>5);
		pPktHdr->egressPriority = pPktHdr->ctagPri;
		pPktHdr->ctagCfi=((pPktHdr->pRxDesc->opts2.bit.cvlan_tag&0x10)>>4);
	}
#endif
	else
	{
		ret=_rtk_rg_portAndProtocolBasedVlanCheck(pPktHdr,&protoVLANID);
		if(ret==RG_RET_SUCCESS)
		{
			*interVLANId=protoVLANID;
			TRACE("Port-Proto-Based interVLANId=%d\n",*interVLANId);
		}
		else
		{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
			//20150610LUKE: from WLAN0 untag we can use per ssid vid as it's ingress vlan id
			if(pPktHdr->ingressPort==RTK_RG_EXT_PORT0 && pPktHdr->wlan_dev_idx>=0){
				//ssid-based VLAN
				*interVLANId=rg_db.systemGlobal.wlan0DeviceBasedVID[pPktHdr->wlan_dev_idx];
				TRACE("WLAN SSID-Based interVLANId=%d\n",*interVLANId);
			}else{
				//port-based VLAN
				*interVLANId=rg_db.systemGlobal.portBasedVID[pPktHdr->ingressPort];
				TRACE("Port-Based interVLANId=%d\n",*interVLANId);
			}
#else
			//port-based VLAN
			*interVLANId=rg_db.systemGlobal.portBasedVID[pPktHdr->ingressPort];
			TRACE("Port-Based interVLANId=%d\n",*interVLANId);
#endif		
		}
	}

	/*Ingress ACL cvid action check and replace interVLANId if hit*/
	assert_ok(_rtk_rg_ingressACLPatternCheck(pPktHdr,rg_db.systemGlobal.acl_SWindex_sorting_by_weight_and_ingress_cvid_action));


	pPktHdr->ingressDecideVlanID=*interVLANId; 


	return RG_RET_SUCCESS;
}


rtk_rg_ipClassification_t _rtk_rg_ipv6_sip_classification(uint8 *pIpv6Sip, rtk_rg_pktHdr_t *pPktHdr)
{
	int l3Idx=_rtk_rg_v6L3lookup(pIpv6Sip);
	TRACE("SIP Lookup hit Routing[%d]",l3Idx);
	if(l3Idx>=0){
		if(rg_db.v6route[l3Idx].internal==1)
			return IP_CLASS_RP;
	}
	return IP_CLASS_NPI;
}

rtk_rg_ipClassification_t _rtk_rg_ipv6_dip_classification(uint8 *pIpv6Dip, rtk_rg_pktHdr_t *pPktHdr)
{
	int i ;
	int l3Idx;
	rtk_rg_table_v6ExtIp_t v6ExtIp_entry;
	for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++){
		bzero(&v6ExtIp_entry,sizeof(v6ExtIp_entry));
		_rtk_rg_ipv6_externalIp_get(i, &v6ExtIp_entry);

		if(!memcmp(pIpv6Dip,v6ExtIp_entry.externalIp.ipv6_addr,IPV6_ADDR_LEN)){ //DIP hit externapIP
			pPktHdr->extipIdx = i; //also record the expIp idex
			return IP_CLASS_NPE;
		}
	}

	//judge for routing
	l3Idx=_rtk_rg_v6L3lookup(pIpv6Dip);
	TRACE("DIP Lookup hit Routing[%d]",l3Idx);
	if(l3Idx>=0){
		if(rg_db.v6route[l3Idx].internal==0)
			return IP_CLASS_NPI;
	}
	
	return IP_CLASS_RP;
}

__SRAM_FWDENG_SLOWPATH
rtk_rg_ipClassification_t _rtk_rg_sip_classification(ipaddr_t sip, rtk_rg_pktHdr_t *pPktHdr)
{
	int i;
	int internal;
	rtk_l34_ext_intip_entry_t *pExtip;
	
	pPktHdr->sipL3Idx=_rtk_rg_l3lookup(sip);
	internal=rg_db.l3[pPktHdr->sipL3Idx].rtk_l3.internal;
	for(i=0;i<MAX_EXTIP_SW_TABLE_SIZE;i++)
	{
		pExtip=&rg_db.extip[i].rtk_extip;
		if(pExtip->valid==1 && sip==pExtip->intIpAddr)
		{
			return internal?IP_CLASS_NI:IP_CLASS_LP;
		}
	}	
	
	return internal?IP_CLASS_NPI:IP_CLASS_RP;		
}

__SRAM_FWDENG_SLOWPATH
rtk_rg_ipClassification_t _rtk_rg_dip_classification(ipaddr_t dip, rtk_rg_pktHdr_t *pPktHdr)
{
	int i;
	rtk_l34_ext_intip_entry_t *pExtip;
	
	for(i=0;i<MAX_EXTIP_SW_TABLE_SIZE;i++)
	{
		pExtip=&rg_db.extip[i].rtk_extip;
		if(pExtip->valid==1  && dip==pExtip->extIpAddr)
		{
			switch(pExtip->type)
			{
				case L34_EXTIP_TYPE_NAPT:
					return IP_CLASS_NPE;
				case L34_EXTIP_TYPE_NAT:
					return IP_CLASS_NE;
				case L34_EXTIP_TYPE_LP:
					return IP_CLASS_LP;
				default:
					return IP_CLASS_FAIL;
			}
		}
	}

	pPktHdr->dipL3Idx=_rtk_rg_l3lookup(dip);
	if(rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.internal==1)
	{
		//20140829LUKE: here return NPI or NI will produce same result.
		return IP_CLASS_NPI;
	}
	else
		return IP_CLASS_RP;
}

__SRAM_FWDENG_SLOWPATH
rtk_rg_fwdEngineReturn_t _rtk_rg_unmatchBindingAct(rtk_rg_pktHdr_t *pPktHdr, rtk_l34_bindAct_t bindAction, rtk_rg_sipDipClassification_t *sipDipClass)
{
	switch(bindAction)
	{
		case L34_BIND_ACT_DROP:
			TRACE("Unmatch Binding Action: do Drop");
			return (RG_FWDENGINE_RET_DROP);
		case L34_BIND_ACT_TRAP:
			TRACE("Unmatch Binding Action: do Trap");
			return (RG_FWDENGINE_RET_TO_PS);
		case L34_BIND_ACT_FORCE_L2BRIDGE:
			TRACE("Unmatch Binding Action: do Forced Bridge");		//for L2_bind_to_L3 or L2_bind_to_L34
			break;
		case L34_BIND_ACT_PERMIT_L2BRIDGE:
			TRACE("Unmatch Binding Action: do Permit");
			if(pPktHdr->layer2BindNetifIdx!=FAIL)pPktHdr->layer2BindNetifIdx=FAIL;	//for L3_bind_to_L2 or L2_bind_to_customized
			break;
		case L34_BIND_ACT_IPV4_LOOKUPL4TABLE_IPV6_TRAP:
			TRACE("Unmatch Binding Action: do L4 or Trap");
			if(pPktHdr->tagif&IPV6_TAGIF)return (RG_FWDENGINE_RET_TO_PS);	//for L3_bind_to_L34 or L3_bind_to_customized
			if(sipDipClass!=NULL)*sipDipClass=SIP_DIP_CLASS_NAPT;			//for L3_bind_to_L34 or L3_bind_to_customized
			break;
		case L34_BIND_ACT_FORCE_BINDL3_SKIP_LOOKUPL4:
			TRACE("Unmatch Binding Action: do L3 bind and skip L4");
			if(sipDipClass!=NULL)*sipDipClass=SIP_DIP_CLASS_ROUTING;			//for L34_bind_to_L3 or L34_bind_to_customized
			break;
 		case L34_BIND_ACT_FORCE_BINDL3:
			TRACE("Unmatch Binding Action: do bind L3");		//for L3_bind_to_L3 or L3_bind_to_customized
			break;
		case L34_BIND_ACT_NORMAL_LOOKUPL34:
			TRACE("Unmatch Binding Action: do Normal L34");
			if(pPktHdr->bindNextHopIdx!=FAIL)pPktHdr->bindNextHopIdx=FAIL;	//for L34_bind_to_customized
			break;
		default:
			break;
	} 

	return (RG_FWDENGINE_RET_CONTINUE);
}

__IRAM_FWDENG_L34
rtk_rg_fwdEngineReturn_t _rtk_rg_routingProcessing(rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_arp_linkList_t *pSoftwareArpEntry;
	
	/* arp table decision */
	if(rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.process==L34_PROCESS_ARP)
	{	
		pPktHdr->netifIdx=rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.netifIdx; //for Normal Route SMAC
		
		pPktHdr->dipArpOrNBIdx = FAIL;
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)

#ifdef CONFIG_APOLLO_MODEL
		pPktHdr->dipArpOrNBIdx=(rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.arpStart<<2)+((htonl(*pPktHdr->pIpv4Dip))& ((1<<(31-rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.ipMask))-1));
#else

		pPktHdr->dipArpOrNBIdx=(rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.arpStart<<2)+((*pPktHdr->pIpv4Dip)& ((1<<(31-rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.ipMask))-1));
#endif

#elif defined(CONFIG_RTL9602C_SERIES)
{
		_rtk_rg_softwareArpTableLookUp(pPktHdr->dipL3Idx,(*pPktHdr->pIpv4Dip),&pSoftwareArpEntry,0);
		if(pSoftwareArpEntry==NULL)	// sw arp is not found
			_rtk_rg_hardwareArpTableLookUp(pPktHdr->dipL3Idx,(*pPktHdr->pIpv4Dip),&pSoftwareArpEntry,0);
		else
			TRACE("Dip hit SW arp table.");
		if(pSoftwareArpEntry!=NULL)
			pPktHdr->dipArpOrNBIdx = pSoftwareArpEntry->idx;
}
#endif

		//if ARP miss, return to PS
		if(pPktHdr->dipArpOrNBIdx==FAIL || rg_db.arp[pPktHdr->dipArpOrNBIdx].rtk_arp.valid==0)
		{
			if(_rtk_rg_fwdengine_handleArpMissInRoutingLookUp(pPktHdr)==RG_FWDENGINE_RET_TO_PS)
				return RG_FWDENGINE_RET_TO_PS;	//trap gateway packet to protocol stack
			
			//if this packet hit ALG, just drop it, otherwise the ALG function will never be executed!!
			//if(pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE) 
			//{
				//TRACE("Drop!");
				//return RG_FWDENGINE_RET_DROP;
			//}

			TRACE("ARP miss, DROP!");
			return RG_FWDENGINE_RET_DROP;
			//return RG_FWDENGINE_RET_DROP; //fixed for no HW arp entry will trap to PS, the SW l4 connection can't create in fwdEngine. ==> but this patch will cause packet to prtocol-stack droped when arp-miss.
		}
		
		pPktHdr->dmacL2Idx=rg_db.arp[pPktHdr->dipArpOrNBIdx].rtk_arp.nhIdx;	
		pPktHdr->aclPolicyRoute_arp2Dmac=1;
		TRACE("ARP netif=%d arp=%d l2=%d\n",pPktHdr->netifIdx,pPktHdr->dipArpOrNBIdx,pPktHdr->dmacL2Idx);
	}

	/* nexthop table decision */
	else if(rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.process==L34_PROCESS_NH)
	{
		pPktHdr->nexthopIdx=rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.nhStart; //for Normal Route DMAC
		pPktHdr->dmacL2Idx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.nhIdx; //for Normal Route DMAC
		pPktHdr->netifIdx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.ifIdx; //for Normal Route SMAC
		TRACE("NH netif=%d nh=%d l2=%d\n",pPktHdr->netifIdx,pPktHdr->nexthopIdx,pPktHdr->dmacL2Idx);
	}
	else if(rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.process==L34_PROCESS_CPU)
	{
		if(rg_db.systemGlobal.interfaceInfo[rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.netifIdx].valid &&
			rg_db.systemGlobal.interfaceInfo[rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.netifIdx].storedInfo.is_wan &&
			(rg_db.systemGlobal.interfaceInfo[rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP ||
			rg_db.systemGlobal.interfaceInfo[rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP ||
			rg_db.systemGlobal.interfaceInfo[rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE ||
			rg_db.systemGlobal.interfaceInfo[rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE))
		{
			pPktHdr->nexthopIdx=rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.nhStart; //for Normal Route DMAC
			pPktHdr->dmacL2Idx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.nhIdx; //for Normal Route DMAC
			pPktHdr->netifIdx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.ifIdx; //for Normal Route SMAC
			pPktHdr->addNaptSwOnly=1;
			TRACE("do %s forwarding! netif=%d, dmac=%d, algAction=%d",rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP?"PPTP":
				rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP?"L2TP":rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE?"DSLITE":"PPPoeDSLITE"
				,pPktHdr->netifIdx,pPktHdr->dmacL2Idx,pPktHdr->algAction);

			return _rtk_rg_checkGwIp(pPktHdr);
		}
		else if(rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.ipAddr > 0)
		{
			pPktHdr->netifIdx=rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.netifIdx; //for Normal Route SMAC
			
			//FIXME:till now default route to CPU is to protocol, if sw routing link-list is implement, 
			//the default route in hw will means routing should check sw routing link-list, too.
			TRACE("L34_PROCESS_CPU:software ARP table lookup");
#ifdef CONFIG_APOLLO_MODEL
			_rtk_rg_softwareArpTableLookUp(pPktHdr->dipL3Idx,(htonl(*pPktHdr->pIpv4Dip)),&pSoftwareArpEntry,0);
#else
			_rtk_rg_softwareArpTableLookUp(pPktHdr->dipL3Idx,(*pPktHdr->pIpv4Dip),&pSoftwareArpEntry,0);
#endif
			if(pSoftwareArpEntry==NULL)
			{
				if(_rtk_rg_fwdengine_handleArpMissInRoutingLookUp(pPktHdr)==RG_FWDENGINE_RET_TO_PS)
					return RG_FWDENGINE_RET_TO_PS;	//trap gateway packet to protocol stack
				
				//if this packet hit ALG, just drop it, otherwise the ALG function will never be executed!!
				//if(pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE)
				//{
					//TRACE("Drop!");
					//return RG_FWDENGINE_RET_DROP;
				//}

				TRACE("ARP miss, DROP!");
				return RG_FWDENGINE_RET_DROP;
				//return RG_FWDENGINE_RET_TO_PS;	//arp not valid
			}
			
			//pPktHdr->dipArpOrNBIdx=(rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.arpStart<<2)+(pPktHdr->ipv4Dip& ((1<<(31-rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.ipMask))-1));
			pPktHdr->dmacL2Idx=rg_db.arp[pSoftwareArpEntry->idx].rtk_arp.nhIdx;
			pPktHdr->aclPolicyRoute_arp2Dmac=1;
		}
		else
		{
			//if this packet hit ALG, just drop it, otherwise the ALG function will never be executed!!
			if(pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE)
			{
				TRACE("Drop!");
				return RG_FWDENGINE_RET_DROP;
			}
			TRACE("return to PROTOCOL STACK(Check for policy route first)!");
			return RG_FWDENGINE_RET_ROUTING_TRAP;
		}
	}else {//if(rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.process==L34_PROCESS_DROP){
		TRACE("L3Decision: Drop!");
		return RG_FWDENGINE_RET_ROUTING_DROP;
	}

	return RG_FWDENGINE_RET_CONTINUE;
}

/* this function will lookup routing,netif,arp,nexthop,extip,l2 tables idx */
rtk_rg_fwdEngineReturn_t _rtk_rg_routingDecisionTablesLookup(rtk_rg_pktHdr_t *pPktHdr, rtk_rg_sipDipClassification_t *sipDipClass)
{
	int wanGroupIdx;
	rtk_rg_fwdEngineReturn_t ret;
	rtk_l34_bindAct_t bindAction=L34_BIND_ACT_END;
	if(*sipDipClass==SIP_DIP_CLASS_HAIRPIN_NAT)
	{
		TRACE("the class is Hairpin NAT");
	}
	else
	{
		TRACE("the class is %s",*sipDipClass==SIP_DIP_CLASS_NAPT?"NAPT":"NAPTR or ROUTING");
	}
	/* Only outbound packet will hit binding table. */
	//20140423LUKE:policyRoute>Binding>normal route, so hit binding only if policyRoute is FAIL!!
	if(rg_db.systemGlobal.initParam.macBasedTagDecision && pPktHdr->aclPolicyRoute==FAIL && ((*sipDipClass==SIP_DIP_CLASS_NAPT) || (*sipDipClass==SIP_DIP_CLASS_ROUTING)) && (rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.rt2waninf==1))
	{
		if(_rtk_rg_bindingRuleCheck(pPktHdr,&wanGroupIdx)==RG_FWDENGINE_RET_HIT_BINDING)
		{
			pPktHdr->netifIdx=rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].index;	//for Port/VLAN Bidning Route SMAC
			pPktHdr->bindNextHopIdx=rg_db.wantype[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.bind_wan_type_ipv4].rtk_wantype.nhIdx; //nextHopIdx for pPktHdr->bindNextHopIdx
			if(*sipDipClass==SIP_DIP_CLASS_NAPT)
			{
				switch(rg_db.wantype[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.bind_wan_type_ipv4].rtk_wantype.wanType)
				{
					case L34_WAN_TYPE_L2_BRIDGE:	//unmatch: follow hw register setting
						TRACE("Unmatch for L34 binding to L2 WAN...");
						bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_UNMATCHED_L34L2];
						break;
					case L34_WAN_TYPE_L3_ROUTE:	//unmatch: follow hw register setting
						TRACE("Unmatch for L34 binding to L3 WAN...");
						bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_UNMATCHED_L34L3];
						break;
					case L34_WAN_TYPE_L34NAT_ROUTE:
						TRACE("L4 binding look up success! nhidx = %d",pPktHdr->bindNextHopIdx);
						break;
					case L34_WAN_TYPE_L34_CUSTOMIZED:	//unmatch: follow hw register setting
						TRACE("Unmatch for L34 binding to customized WAN...");
						bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_CUSTOMIZED_L34];
						break;
					default:
						break;
				}
			}
			else
			{	
				switch(rg_db.wantype[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.bind_wan_type_ipv4].rtk_wantype.wanType)
				{
					case L34_WAN_TYPE_L2_BRIDGE:	//unmatch: follow hw register setting
						TRACE("Unmatch for L3 binding to L2 WAN...");
						bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_UNMATCHED_L3L2];
						break;
					case L34_WAN_TYPE_L3_ROUTE:	
						TRACE("Unmatch for L3 binding to L3 WAN...nhidx = %d",pPktHdr->bindNextHopIdx);
						//20140717LUKE: this is special case, though we are binding from L3 to L3, we can have option to trap such packet.
						bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_UNMATCHED_L3L3];						
						break;
					case L34_WAN_TYPE_L34NAT_ROUTE:	//unmatch: follow hw register setting
						TRACE("Unmatch for L3 binding to L34 WAN...");
						bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_UNMATCHED_L3L34];
						break;
					case L34_WAN_TYPE_L34_CUSTOMIZED:	//unmatch: follow hw register setting
						TRACE("Unmatch for L3 binding to customized WAN...");
						bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_CUSTOMIZED_L3];
						break;
					default:
						break;
				}
			}

			//if unmatch, do action!!
			ret=_rtk_rg_unmatchBindingAct(pPktHdr,bindAction,sipDipClass);
			if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;

			if(pPktHdr->bindNextHopIdx!=FAIL)
			{
				//do the normal route process for DA!!
				ret=_rtk_rg_routingProcessing(pPktHdr);
#if defined(CONFIG_RTL9602C_SERIES)
				if(ret==RG_FWDENGINE_RET_ROUTING_TRAP || ret==RG_FWDENGINE_RET_ROUTING_DROP)
					TRACE("L3 Binding will over normal route TRAP/DROP.");
#else
				if(ret==RG_FWDENGINE_RET_ROUTING_TRAP)return RG_FWDENGINE_RET_TO_PS;	//bind must no policy route
				else if(ret==RG_FWDENGINE_RET_ROUTING_DROP)return RG_FWDENGINE_RET_DROP;
#endif
				else if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;
				
				/* Rome driver should make sure that binding->nexthop is equal to napt->nexthop. */
				pPktHdr->nexthopIdx=pPktHdr->bindNextHopIdx; //for Port/VLAN Bidning Route SMAC/DMAC
				pPktHdr->netifIdx=rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].index;	//for Port/VLAN Bidning Route SMAC

#if defined(CONFIG_RTL9602C_SERIES)
				//20151012LUKE: for ARP-routing, destination address index retrieved from CAM.
				if(pPktHdr->aclPolicyRoute_arp2Dmac==0)
#else
				/* Special case: for binding but interface route, DMAC decision should be from L3->ARP->L2 */
				if(pPktHdr->dipL3Idx==V4_DEFAULT_ROUTE_IDX)
#endif
					pPktHdr->dmacL2Idx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.nhIdx; //for Port/VLAN Bidning Route DMAC				

				/* ext ip table decision */
				pPktHdr->extipIdx=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.extip_idx; /* for NAPT:DMAC, SMAC */
				TRACE("Binding to interface pPktHdr->netifIdx = %d, extipIdx = %d",pPktHdr->netifIdx,pPktHdr->extipIdx);

				//for PPTP or L2TP wan, we add to software napt-list
				//20150120LUKE: Dslite, too
				if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP ||
					rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP ||
					rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE ||
					rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)
					pPktHdr->addNaptSwOnly=1;

				//Check MTU
				//TRACE("rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu= %d ,pPktHdr->l3Len %d",rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu,pPktHdr->l3Len);
				if(rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu < pPktHdr->l3Len)		//TooBig, trap to protocol stack
				{
					//FIXME: ip fragment should be done in fwdEngine, not protocol stack...
					//and we should check "Don't fragmets" flag, if enable, we should return ICMP "Packet is too big" infomation.

					pPktHdr->overMTU=1;

					TRACE("packet L3 size(%d) is bigger than interface[%d]'s MTU(%d), DF=%d",pPktHdr->l3Len,pPktHdr->netifIdx,rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu,pPktHdr->ipv4DontFragment);
					if(pPktHdr->ipv4DontFragment)
						return RG_FWDENGINE_RET_TO_PS;
				}
	
				return RG_FWDENGINE_RET_CONTINUE;
			}
		}
#if 0
		//Port/VLAN Binding
		if(pPktHdr->bindingDecision==RG_BINDING_NOT_FINISHED)
		{
			pPktHdr->bindNextHopIdx=_rtk_rg_portBindingLookup(pPktHdr->pRxDesc->opts3.bit.src_port_num,0,pPktHdr->internalVlanID,*sipDipClass,pPktHdr);
			if(pPktHdr->bindNextHopIdx!=FAIL)
			{
				/* Rome driver should make sure that binding->nexthop is equal to napt->nexthop. */
				pPktHdr->nexthopIdx=pPktHdr->bindNextHopIdx; //for Port/VLAN Bidning Route SMAC/DMAC

				/* Special case: for binding but interface route, DMAC decision should be from L3->ARP->L2 */
				if((pPktHdr->dipL3Idx!=7) && (rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.process==L34_PROCESS_ARP))
				{
					pPktHdr->dipArpOrNBIdx=(rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.arpStart<<2)+((*pPktHdr->pIpv4Dip)& ((0x1<<(31-rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.ipMask))-1));
					pPktHdr->dmacL2Idx=rg_db.arp[pPktHdr->dipArpOrNBIdx].rtk_arp.nhIdx;	
				}
				else
				{
					pPktHdr->dmacL2Idx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.nhIdx; //for Port/VLAN Bidning Route DMAC
				}
				pPktHdr->netifIdx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.ifIdx; //for Port/VLAN Bidning Route SMAC
				/* ext ip table decision */
				pPktHdr->extipIdx=pPktHdr->netifIdx; /* for NAPT:DMAC, SMAC */			
				//DEBUG("Binding to interface pPktHdr->netifIdx = %d",pPktHdr->netifIdx);
				return RG_FWDENGINE_RET_CONTINUE;
			}
		}
		else
		{
			if(pPktHdr->bindNextHopIdx!=FAIL) return RG_FWDENGINE_RET_CONTINUE;
		}
#endif
	}

	//Interface Routing
	if((*sipDipClass==SIP_DIP_CLASS_NAPTR)||(*sipDipClass==SIP_DIP_CLASS_HAIRPIN_NAT))
#ifdef CONFIG_APOLLO_MODEL
		pPktHdr->dipL3Idx=_rtk_rg_l3lookup(htonl(*pPktHdr->pIpv4Dip));
#else
		pPktHdr->dipL3Idx=_rtk_rg_l3lookup(*pPktHdr->pIpv4Dip);
#endif
	//DEBUG("ipv4=%x dipL3Idx=%d process=%d\n",*pPktHdr->pIpv4Dip,pPktHdr->dipL3Idx,rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.process);	
	pPktHdr->aclPolicyRoute_arp2Dmac=0;
	ret=_rtk_rg_routingProcessing(pPktHdr);
	//TRACE("_rtk_rg_routingProcessing  ret=%d ",ret);
	if(ret!=RG_FWDENGINE_RET_CONTINUE && ret!=RG_FWDENGINE_RET_ROUTING_TRAP && ret!=RG_FWDENGINE_RET_ROUTING_DROP)
		return ret;

	//1 Use Policy Route WAN intf instead interface route decision!!
	if(*sipDipClass==SIP_DIP_CLASS_NAPT && pPktHdr->aclPolicyRoute!=FAIL)		//only L4(TCP, UDP, ICMP) will do policy route!!
	{
		TRACE("use WAN%d as policy route",pPktHdr->aclPolicyRoute);
		pPktHdr->netifIdx=pPktHdr->aclPolicyRoute;
		//20141111LUKE: for normal route as ARP, policy route should not change it's DMAC to nexthop, but keep original ARP decision.
		if(!pPktHdr->aclPolicyRoute_arp2Dmac)
		{
			if(pPktHdr->tagif&IPV4_TAGIF){
				if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.nexthop_ipv4>=0)
					pPktHdr->dmacL2Idx=rg_db.nexthop[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.nexthop_ipv4].rtk_nexthop.nhIdx;
				else{
					TRACE("Get IPv4 nexthop fail(%d)...trap!",rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.nexthop_ipv4);
					return RG_FWDENGINE_RET_TO_PS;
				}
			}else if(pPktHdr->tagif&IPV6_TAGIF){
				if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.nexthop_ipv6>=0)
					pPktHdr->dmacL2Idx=rg_db.nexthop[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.nexthop_ipv6].rtk_nexthop.nhIdx;
				else{
					TRACE("Get IPv6 nexthop fail(%d)...trap!",rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.nexthop_ipv6);
					return RG_FWDENGINE_RET_TO_PS;
				}	
			}
		}

		//for PPTP or L2TP wan, we add to software napt-list
		//20150120LUKE: Dslite, too
		if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP ||
			rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP ||
			rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE ||
			rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)
			pPktHdr->addNaptSwOnly=1;
	}
	else if(ret==RG_FWDENGINE_RET_ROUTING_TRAP)return RG_FWDENGINE_RET_TO_PS;	//no Policy-route, trap to protocol stack..
	else if(ret==RG_FWDENGINE_RET_ROUTING_DROP)return RG_FWDENGINE_RET_DROP;	//no Policy-route, drop..

	//Check MTU
	//TRACE("rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu= %d ,pPktHdr->l3Len %d",rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu,pPktHdr->l3Len);
	if(rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu < pPktHdr->l3Len)		//TooBig, trap to protocol stack
	{
		//FIXME: ip fragment should be done in fwdEngine, not protocol stack...
		//and we should check "Don't fragmets" flag, if enable, we should return ICMP "Packet is too big" infomation.

		pPktHdr->overMTU=1;

		TRACE("packet L3 size(%d) is bigger than interface[%d]'s MTU(%d), DF=%d",pPktHdr->l3Len,pPktHdr->netifIdx,rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu,pPktHdr->ipv4DontFragment);
		if(pPktHdr->ipv4DontFragment)
			return RG_FWDENGINE_RET_TO_PS;
	}

	/* ext ip table decision */
	pPktHdr->extipIdx=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.extip_idx; /* for NAPT:DMAC, SMAC */
	//TRACE("_rtk_rg_routingDecisionTablesLookup ret RG_FWDENGINE_RET_CONTINUE");
	return RG_FWDENGINE_RET_CONTINUE;
}
#if 0
int _rtk_rg_v6RoutingDecisionTablesLookup(rtk_rg_pktHdr_t *pPktHdr, int isLAN2WAN)
{
	rtk_ipv6Neighbor_entry_t *neighbor;
	int ret,neighbor_valid_idx,nb_hash_idx,i;
	if(isLAN2WAN==1) //Only for LAN to WAN packet will hit binding table.
	{
		//Port/VLAN Binding
		if(pPktHdr->bindLookUpFinished==0)
		{
			pPktHdr->bindNextHopIdx=_rtk_rg_portBindingLookup(pPktHdr->pRxDesc->opts3.bit.src_port_num,0,pPktHdr->ctagVid);
			pPktHdr->bindLookUpFinished=1;
			if(pPktHdr->bindNextHopIdx!=FAIL)
			{
				DEBUG("binding hit!");
				pPktHdr->nexthopIdx=pPktHdr->bindNextHopIdx; //for Port/VLAN Bidning Route SMAC/DMAC
				pPktHdr->dmacL2Idx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.nhIdx; //for Port/VLAN Bidning Route DMAC
				pPktHdr->netifIdx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.ifIdx; //for Port/VLAN Bidning Route SMAC
				/* ext ip table decision */
				//pPktHdr->extipIdx=pPktHdr->netifIdx; /* for NAPT:DMAC, SMAC */			
				//DEBUG("pPktHdr->netifIdx = %d",pPktHdr->netifIdx);
				return RG_FWDENGINE_RET_DIRECT_TX;
			}
		}
		else if(pPktHdr->bindNextHopIdx!=FAIL) 
			return RG_FWDENGINE_RET_DIRECT_TX;
		
	}

	//Interface Routing
	ret=_rtk_rg_v6L3lookup(pPktHdr->pIpv6Dip);
	if(ret<0)return RG_FWDENGINE_RET_TO_PS;
	
	pPktHdr->dipL3Idx=ret;
	//DEBUG("ipv4=%x dipL3Idx=%d process=%d\n",pPktHdr->ipv4Dip,pPktHdr->dipL3Idx,rg_db.l3[pPktHdr->dipL3Idx].rtk_l3.process);
	DEBUG("the rout type is %x",rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.type)
	switch(rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.type)
	{
		/* neighbor table decision */
		case L34_IPV6_ROUTE_TYPE_LOCAL:
		{
			pPktHdr->netifIdx=rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.nhOrIfidIdx; //for Normal Route SMAC
			ret=0;
			nb_hash_idx = _rtk_rg_IPv6NeighborHash(pPktHdr->pIpv6Dip+8, (unsigned char)pPktHdr->dipL3Idx);
			for(i=0;i<8;i++)
			{
				neighbor_valid_idx=(nb_hash_idx<<3)+i;
				//rtlglue_printf("the matchNeighbor idx = %d\n",matchNeighbor);
				DEBUG("the valid neighbor idx is %d",neighbor_valid_idx);
				neighbor = &rg_db.v6neighbor[neighbor_valid_idx].rtk_v6neighbor;
				//rtlglue_printf("the neighbor.ipv6RouteIdx = %d, matchEntry = %d\n",neighbor.ipv6RouteIdx,matchEntry);
				if(neighbor->valid && 
					(neighbor->ipv6RouteIdx == pPktHdr->dipL3Idx)&&
					(_rtk_rg_CompareIFID(pPktHdr->pIpv6Dip+8, neighbor->ipv6Ifid)))
				{
					//rtlglue_printf("HIT!!!!!\n");
					DEBUG("IPv6: Hit neighbor table!L2IDX = %d rt=%d",neighbor->l2Idx,pPktHdr->dipL3Idx);
					ret = 1;
					break;
				}
			}
			if(ret==0)	//neighbor lookup miss
			{
				DEBUG("before ret to PS in RDTL");
				return RG_FWDENGINE_RET_TO_PS;
			}
			pPktHdr->dipArpOrNBIdx=neighbor_valid_idx;
			pPktHdr->dmacL2Idx=neighbor->l2Idx;
			//DEBUG("Local netif=%d nb=%d l2=%d\n",pPktHdr->netifIdx,pPktHdr->dipArpOrNBIdx,pPktHdr->dmacL2Idx);		
			break;
		}
		/* nexthop table decision */
		case L34_IPV6_ROUTE_TYPE_GLOBAL:
		{
			pPktHdr->nexthopIdx=rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.nhOrIfidIdx; //for Normal Route DMAC
			pPktHdr->dmacL2Idx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.nhIdx; //for Normal Route DMAC
			pPktHdr->netifIdx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.ifIdx; //for Normal Route SMAC
			//DEBUG("Global netif=%d nh=%d l2=%d\n",pPktHdr->netifIdx,pPktHdr->nexthopIdx,pPktHdr->dmacL2Idx);
			break;
		}
		case L34_IPV6_ROUTE_TYPE_TRAP:
			return RG_FWDENGINE_RET_TO_PS;
		default:	//L34_IPV6_ROUTE_TYPE_DROP
			return RG_FWDENGINE_RET_DROP;
	}
	
	return RG_FWDENGINE_RET_DIRECT_TX;
}
#endif

//void dump_packet(u8 *pkt,u32 size,char *memo);

#ifdef CONFIG_ROME_NAPT_LRU		
#else
rtk_rg_successFailReturn_t _rtk_rg_fwdEngineNaptEarlyDrop(int16 ignorePort)
{
	int score,high_score=0;
	int earlyDropCandicate=-1;
	//int idleDropCandicate=-1;
	//int longestIdleTime=0;
	int i;

	for(i=0;i<MAX_NAPT_OUT_SW_TABLE_SIZE;i++) //Lookup SW dropable entry
	{
		if((rg_db.naptOut[i].state==INVALID) || (rg_db.naptOut[i].state==TCP_CONNECTED) || (rg_db.naptOut[i].state==UDP_CONNECTED)) continue;
		if(rg_db.naptOut[i].extPort==ignorePort) continue;

		score=((rg_db.naptOut[i].state==FIRST_FIN)||(rg_db.naptOut[i].state==RST_RECV))?2:1;
		if(score>high_score) 
		{
			high_score=score;
			earlyDropCandicate = i;
		}
		//DEBUG("Drop score:%d secs drop candicate:%d\n",score,earlyDropCandicate);
		if(score >= 2) break;
	}
	if(earlyDropCandicate>=0)
	{
		rtlglue_printf("NAPT Full, delete non-established entry :%d num:%d\n",earlyDropCandicate,atomic_read(&rg_db.naptForwardEngineEntryNumber[0]));
		(pf.rtk_rg_naptConnection_del)(earlyDropCandicate);
	}
	else
	{
		if(rg_db.longestIdleNaptIdx==FAIL)
		{
			WARNING("NAPT Full, delete longest expired entry failly!\n");
			return RG_RET_FAIL;
		}
		
		rtlglue_printf("NAPT Full, delete longest expired entry :%d num:%d\n",rg_db.longestIdleNaptIdx,atomic_read(&rg_db.naptForwardEngineEntryNumber[0]));
		(pf.rtk_rg_naptConnection_del)(rg_db.longestIdleNaptIdx);
		rg_db.longestIdleNaptIdx = FAIL;
	}
	return RG_RET_SUCCESS;
}
#endif

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
rtk_rg_extPortGetReturn_t _rtk_rg_ipv6_naptExtPortInUsedCheckint (int isTcp,uint16 wishPort)
{
	int wishIdx;
	uint32 wishBitValue;

	wishIdx=wishPort>>5; // =wishPort/32
	wishBitValue=1<<(wishPort&0x1f);
	
	if(isTcp)
	{
		if(((rg_db.ipv6naptTcpExternPortUsed[wishIdx]&wishBitValue)==0))
		{
			rg_db.ipv6naptTcpExternPortUsed[wishIdx]|=wishBitValue;
			return 0; //free
		}else{
			DEBUG("wishPort(%d) is already used!",wishPort);
		}			
	}
	else
	{
		if(((rg_db.ipv6naptUdpExternPortUsed[wishIdx]&wishBitValue)==0))
		{
			rg_db.ipv6naptUdpExternPortUsed[wishIdx]|=wishBitValue;
			return 0; //free
		}else{
			DEBUG("wishPort(%d) is already used!",wishPort);
		}
	}

	return RG_RET_EXTPORT_NOT_GET; //used
}



rtk_rg_extPortGetReturn_t _rtk_rg_ipv6_naptExtPortGetAndUse(int isTcp,uint16 wishPort){
	int i;
	i=wishPort;
	
	while(1)
	{
		if(_rtk_rg_ipv6_naptExtPortInUsedCheckint(isTcp,i)==0) // free
		{
			return i;
		}
		i++;
		i&=0xffff;

		//all port in used
		if(i==wishPort) break;

	}	

	//no port can be used
	return RG_RET_EXTPORT_NOT_GET;
}


rtk_rg_successFailReturn_t _rtk_rg_ipv6_naptExtPortFree(int isTcp,uint16 wishPort){
	
	int wishIdx;
	uint32 wishBitValue;

	wishIdx=wishPort>>5; // =wishPort/32
	wishBitValue=1<<(wishPort&0x1f);


	DEBUG("Free IPv6 NAPT Extport(%d), isTcp=%d, wishIdx=%d, wishBitValue=0x%x",wishPort,isTcp,wishIdx,wishBitValue);
	if(isTcp)
	{
		if(((rg_db.ipv6naptTcpExternPortUsed[wishIdx]&wishBitValue)))
		{
			rg_db.ipv6naptTcpExternPortUsed[wishIdx]&=(~wishBitValue);
			//DEBUG("rg_db.ipv6naptTcpExternPortUsed[%d]=0x%x",wishIdx,rg_db.ipv6naptTcpExternPortUsed[wishIdx]);
			return RG_RET_SUCCESS; //free
		}else{
			DEBUG("wishPort(%d) is already free!",wishPort);
		}			
	}
	else
	{
		if(((rg_db.ipv6naptUdpExternPortUsed[wishIdx]&wishBitValue)))
		{
			rg_db.ipv6naptUdpExternPortUsed[wishIdx]&=(~wishBitValue);	
			//DEBUG("rg_db.ipv6naptUdpExternPortUsed[%d]=0x%x",wishIdx,rg_db.ipv6naptTcpExternPortUsed[wishIdx]);
			return RG_RET_SUCCESS; //free
		}else{
			DEBUG("wishPort(%d) is already free!",wishPort);
		}
	}
	return RG_RET_FAIL;

}

#endif


/* force: Force to use the wishPort. For inbound connection, we don't modify the wishPort number. */
__SRAM_FWDENG_SLOWPATH
#if defined(CONFIG_APOLLO)
rtk_rg_extPortGetReturn_t _rtk_rg_naptExtPortGetAndUse(int force,int isTcp,uint16 wishPort,uint32 *pIp,int addRefCnt)
#elif defined(CONFIG_XDSL_ROMEDRIVER)
int _rtk_rg_naptExtPortGetAndUse(int force,int isTcp,uint32 sip,uint16 sport,uint16 wishPort,uint32 *pIp,int addRefCnt)
#endif
{
	int i;
	int maxFindHwInCnt=16;	
#if defined(CONFIG_APOLLO)
	uint32 shiftBits=2;	
	uint32 naptWay=3;	//4-way
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)	
	uint32 shiftBits=0; 
	uint32 naptWay=0;	//1-way
#endif
	i=wishPort;


	while(1)
	{
		if(_rtk_rg_naptExtPortInUsedCheck(force,isTcp,i,FALSE)==0) // free
		{
			if((force==FALSE)&&(rg_db.systemGlobal.enableL4ChoiceHwIn==1)&&(pIp!=NULL))
			{
				int hashIdx,j;
#if defined(CONFIG_APOLLO)
				hashIdx=_rtk_rg_NAPTRIndex_get(isTcp,*pIp,i);				
#elif defined(CONFIG_XDSL_ROMEDRIVER)
				hashIdx=_rtk_rg_NAPTRIndex_get(isTcp,sip,sport,*pIp,i);		
#endif
				for(j=(hashIdx<<shiftBits)+naptWay;j>=(hashIdx<<shiftBits);j--)
				{
					if(rg_db.naptIn[j].rtk_naptIn.valid==0)
					{
						if(addRefCnt)
						{
							atomic_inc(&rg_db.naptForwardEngineEntryNumber[isTcp]);	
							_rtk_rg_naptExtPortInUsedCheck(force,isTcp,i,TRUE);
						}
						return i; //found a valid entry in this hashIdx
					}						
				}
				maxFindHwInCnt--;
				if(maxFindHwInCnt<=0) 
				{
					if(addRefCnt)
					{
						atomic_inc(&rg_db.naptForwardEngineEntryNumber[isTcp]);	
						_rtk_rg_naptExtPortInUsedCheck(force,isTcp,i,TRUE);
					}
					return i; //not find any free HW inbound entry.				
				}
			}
			else
			{
				if(addRefCnt)
				{
					atomic_inc(&rg_db.naptForwardEngineEntryNumber[isTcp]);		
					_rtk_rg_naptExtPortInUsedCheck(force,isTcp,i,TRUE);
				}
#ifdef CONFIG_ROME_NAPT_LRU		
#else
				DEBUG("[NAPT] Add forwarding engine connection flow:%d\n",atomic_read(&rg_db.naptForwardEngineEntryNumber[0])+atomic_read(&rg_db.naptForwardEngineEntryNumber[1]));
				if((atomic_read(&rg_db.naptForwardEngineEntryNumber[0])+atomic_read(&rg_db.naptForwardEngineEntryNumber[1]))>MAX_NAPT_OUT_SW_TABLE_SIZE)
				{
					if(_rtk_rg_fwdEngineNaptEarlyDrop(i)!=RG_RET_FAIL)
						return RG_RET_EXTPORT_NOT_GET;
				}
#endif
				return i;
			}
		}
		i++;
		i&=0xffff;


		if(i==wishPort) break;

	}	
	return RG_RET_EXTPORT_NOT_GET;
}

#ifdef CONFIG_ROME_NAPT_SHORTCUT
rtk_rg_entryGetReturn_t _rtk_rg_shortcutARPFind(int routingIdx, ipaddr_t ipAddr)
{
	int arpIdx=RG_RET_ENTRY_NOT_GET;
	rtk_rg_arp_linkList_t *pSwArpList;

	if(routingIdx>=0)
	{
		if(rg_db.l3[routingIdx].rtk_l3.process==L34_PROCESS_ARP)
		{
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
			arpIdx=(rg_db.l3[routingIdx].rtk_l3.arpStart<<2)+(ipAddr & ((1<<(31-rg_db.l3[routingIdx].rtk_l3.ipMask))-1));
			if(!rg_db.arp[arpIdx].rtk_arp.valid)
				arpIdx=RG_RET_ENTRY_NOT_GET;
#elif defined(CONFIG_RTL9602C_SERIES)
			_rtk_rg_hardwareArpTableLookUp(routingIdx,ipAddr,&pSwArpList,0);
			if(pSwArpList==NULL)
				_rtk_rg_softwareArpTableLookUp(routingIdx,ipAddr,&pSwArpList,0);
			if(pSwArpList!=NULL)
			{
				arpIdx=pSwArpList->idx;
				if(!rg_db.arp[arpIdx].rtk_arp.valid)
					arpIdx=RG_RET_ENTRY_NOT_GET;
			}	
#endif
		}
		else if((rg_db.l3[routingIdx].rtk_l3.process==L34_PROCESS_CPU)&&(rg_db.l3[routingIdx].rtk_l3.ipAddr>0))
		{
			_rtk_rg_softwareArpTableLookUp(routingIdx,ipAddr,&pSwArpList,0);
			if(pSwArpList!=NULL)
			{
				arpIdx=pSwArpList->idx;
				if(!rg_db.arp[arpIdx].rtk_arp.valid)
					arpIdx=RG_RET_ENTRY_NOT_GET;
			}
		}
	}

	return arpIdx;
}

rtk_rg_entryGetReturn_t _rtk_rg_v4ShortCutFreeEntryGet(int hashIdx, uint32 sip, uint32 dip, uint16 sport, uint16 dport, uint8 isTcp)
{
	int i, first_invalid;
	uint32 longestIdx, longestIdletime;

	first_invalid = FAIL;
	longestIdx = hashIdx;
	longestIdletime = 0;
	for(i=hashIdx; i<hashIdx+MAX_NAPT_SHORTCUT_WAYS; i++)
	{
		if(rg_db.naptShortCut[i].sip==0)
		{
			if(first_invalid==FAIL)
				first_invalid = i;
			continue;
		}	
		if((rg_db.naptShortCut[i].sip==sip) &&
			(rg_db.naptShortCut[i].dip==dip) &&
			(rg_db.naptShortCut[i].sport==sport) &&
			(rg_db.naptShortCut[i].dport==dport) &&
			(rg_db.naptShortCut[i].isTcp==isTcp))
			return i;	  
			
		if(rg_db.naptShortCut[i].idleSecs > longestIdletime)
		{
			longestIdx = i;
			longestIdletime = rg_db.naptShortCut[i].idleSecs;
		}
	}

	if(first_invalid!=FAIL)
		return first_invalid;
		
	if(longestIdletime==0) //choose last add index+1
	{
		DEBUG("V4shortcut LRU, do round robin!");
		longestIdx += ((rg_db.v4ShortCut_lastAddIdx[hashIdx>>MAX_NAPT_SHORTCUT_WAYS_SHIFT]+1) & (MAX_NAPT_SHORTCUT_WAYS-1));
		longestIdletime = rg_db.naptShortCut[longestIdx].idleSecs;
	}
	
	DEBUG("V4shortcut LRU choose victim[%d], idletime: %d secs", longestIdx, longestIdletime);
	_rtk_rg_v4ShortCut_delete(longestIdx);

	if(rg_db.systemGlobal.fwdStatistic)
		rg_db.systemGlobal.statistic.perPortCnt_v4ShortcutLRU[rg_db.pktHdr->ingressPort]++;
	
	return longestIdx;
	
}


__SRAM_FWDENG_SLOWPATH
void _rtk_rg_naptShortcutUpdate(rtk_rg_pktHdr_t *pPktHdr,int dir,int naptIdx, int isNapt, int isBridge)
{
	int isTcp=0,shortcutIdx=0;
	int freeIdx;

	if(pPktHdr->aclHit) return;

	if(pPktHdr->tagif&TCP_TAGIF)
		isTcp=1;
	//Check for ALG
	if(isNapt && pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE)
		//((_rtk_rg_algCheckEnable(isTcp,pPktHdr->dport)==SUCCESS) || (_rtk_rg_algCheckEnable(isTcp,pPktHdr->sport)==SUCCESS)	 ||	//check ALG serverInWan outbound and inbound port
		//(_rtk_rg_algSrvInLanCheckEnable(isTcp,pPktHdr->sport)==SUCCESS) || (_rtk_rg_algSrvInLanCheckEnable(isTcp,pPktHdr->dport)==SUCCESS)))	//check ALG serverInLan outbound and inbound port
	{
		TRACE("ALG enable port...don't update shortCut!");
		return;
	}

	if(pPktHdr->overMTU==1)
	{
		TRACE("over MTU...don't update shortCut!");
		return;
	}
	
	//20140813LUKE: use hash to get index!!
	shortcutIdx=_rtk_rg_shortcutHashIndex(pPktHdr->ipv4Sip,pPktHdr->ipv4Dip,pPktHdr->sport,pPktHdr->dport);
	freeIdx = _rtk_rg_v4ShortCutFreeEntryGet(shortcutIdx, pPktHdr->ipv4Sip, pPktHdr->ipv4Dip, pPktHdr->sport, pPktHdr->dport, isTcp);
	if(freeIdx==RG_RET_ENTRY_NOT_GET)
	{
		WARNING("Can not get free v4 shortcut entry!");
		return;
	}

	pPktHdr->currentShortcutIdx=freeIdx;
	pPktHdr->pCurrentShortcutEntry=&rg_db.naptShortCut[freeIdx];
	pPktHdr->pCurrentShortcutEntry->idleSecs= 0;
	rg_db.v4ShortCutValidSet[freeIdx>>5] |= (0x1<<(freeIdx&0x1f));
	rg_db.v4ShortCut_lastAddIdx[shortcutIdx>>MAX_NAPT_SHORTCUT_WAYS_SHIFT] = (freeIdx-shortcutIdx);
	
	if((pPktHdr->isHairpinNat)&&(dir==NAPT_DIRECTION_OUTBOUND))
	{
		pPktHdr->pCurrentShortcutEntry->notFinishUpdated=0;
		pPktHdr->pCurrentShortcutEntry->isHairpinNat=1;
	}
	else
	{
		pPktHdr->pCurrentShortcutEntry->notFinishUpdated=1;
		pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE_BEFORE_SEND;
	}
	pPktHdr->pCurrentShortcutEntry->sip=pPktHdr->ipv4Sip; //SIP
	pPktHdr->pCurrentShortcutEntry->dip=pPktHdr->ipv4Dip; //DIP
	pPktHdr->pCurrentShortcutEntry->sport=pPktHdr->sport; //SPORT
	pPktHdr->pCurrentShortcutEntry->dport=pPktHdr->dport; //DPORT

#if defined(SHORTCUT_BITFILED_DEBUG)    
    assert(pPktHdr->ingressPort >= 0 && pPktHdr->ingressPort < (1 << SC_BFW_SPA));
    assert(isTcp >= 0 && isTcp < (1 << SC_BFW_ISTCP));
    assert(pPktHdr->dmacL2Idx < (1 << (SC_BFW_LUTIDX-1)));
    assert(pPktHdr->netifIdx < (1 << (SC_BFW_INTFIDX-1)));
    assert(pPktHdr->extipIdx < (1 << (SC_BFW_EIPIDX-1)));
    assert(naptIdx < (1 << (SC_BFW_NAPTIDX)));
    assert(isNapt >= 0 && isNapt < (1 << (SC_BFW_ISNAPT)));
    assert(isBridge >= 0 && isBridge < (1 << (SC_BFW_ISBRIDGE)));
    assert(dir >= 0 && dir < (1 << (SC_BFW_DIR)));
    assert(_rtk_rg_shortcutARPFind(pPktHdr->sipL3Idx, pPktHdr->ipv4Sip) < (1 << (SC_BFW_ARPIDX-1)));
#endif

	pPktHdr->pCurrentShortcutEntry->spa=pPktHdr->ingressPort;
	pPktHdr->pCurrentShortcutEntry->isTcp=isTcp;
	pPktHdr->pCurrentShortcutEntry->new_lut_idx=pPktHdr->dmacL2Idx; //DMAC
	pPktHdr->pCurrentShortcutEntry->new_intf_idx=pPktHdr->netifIdx; //SMAC
	pPktHdr->pCurrentShortcutEntry->new_eip_idx=pPktHdr->extipIdx; //for outbound sip
	pPktHdr->pCurrentShortcutEntry->naptIdx=naptIdx;	//routing won't referenced this field
	pPktHdr->pCurrentShortcutEntry->isNapt=isNapt;		//routing won't referenced this field
	pPktHdr->pCurrentShortcutEntry->isBridge=isBridge;	//routing won't referenced this field
	pPktHdr->pCurrentShortcutEntry->direction=dir;
	//20150115LUKE: keep ARP idx in shortcut for updating.
	pPktHdr->pCurrentShortcutEntry->arpIdx=_rtk_rg_shortcutARPFind(pPktHdr->sipL3Idx, pPktHdr->ipv4Sip);
	//20150922: keep smac L2 index in shoutcut for updating.
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
	pPktHdr->pCurrentShortcutEntry->smacL2Idx = pPktHdr->smacL2Idx;
#endif

	TRACE("[Update SC:%d] dir:%s da=%d sa_intf=%d eipIdx=%d naptIdx=%d\n",freeIdx,(dir==NAPT_DIRECTION_INBOUND)?"IN":"OUT",pPktHdr->dmacL2Idx,pPktHdr->netifIdx,pPktHdr->extipIdx,naptIdx);
	TRACE("isTcp:%d src:0x%x:%d dst:0x%x:%d",isTcp,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->ipv4Dip,pPktHdr->dport);
	TABLE("Add SC: [idx=%d][sip=0x%x][dip=0x%x][sport=%d][dport=%d][isTcp=%d][isHairpin=%d]",freeIdx,pPktHdr->ipv4Sip,pPktHdr->ipv4Dip,pPktHdr->sport,pPktHdr->dport,isTcp,pPktHdr->pCurrentShortcutEntry->isHairpinNat);


	//20140811LUKE: add dual direction shortcut to acclerate SYN-ACK when tcp_hw_learning_at_syn is on!!
	if(isNapt && isTcp && ((rg_db.systemGlobal.tcp_hw_learning_at_syn==1 && rg_db.systemGlobal.tcp_in_shortcut_learning_at_syn)||rg_db.systemGlobal.tcpDisableStatefulTracking) && dir==NAPT_DIRECTION_OUTBOUND)	//20151207LUKE: disable TCP stateful tracking
	{
		shortcutIdx=_rtk_rg_shortcutHashIndex(pPktHdr->ipv4Dip,rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,pPktHdr->dport,rg_db.naptOut[naptIdx].extPort);
		freeIdx = _rtk_rg_v4ShortCutFreeEntryGet(shortcutIdx, pPktHdr->ipv4Dip, rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr, pPktHdr->dport, rg_db.naptOut[naptIdx].extPort, isTcp);
		if(freeIdx==RG_RET_ENTRY_NOT_GET)
		{
			WARNING("Can not get free v4 shortcut entry!");
			return;
		}
		
		pPktHdr->inboundShortcutIdx=freeIdx;
		pPktHdr->pInboundShortcutEntry=&rg_db.naptShortCut[freeIdx];
		pPktHdr->pInboundShortcutEntry->idleSecs=0;
		rg_db.v4ShortCutValidSet[freeIdx>>5] |= (0x1<<(freeIdx&0x1f));
		rg_db.v4ShortCut_lastAddIdx[shortcutIdx>>MAX_NAPT_SHORTCUT_WAYS_SHIFT] = (freeIdx-shortcutIdx);
		
		pPktHdr->pInboundShortcutEntry->notFinishUpdated=1;
		pPktHdr->pInboundShortcutEntry->sip=pPktHdr->ipv4Dip;
		pPktHdr->pInboundShortcutEntry->dip=rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr;	//WAN IP
		pPktHdr->pInboundShortcutEntry->sport=pPktHdr->dport;
		pPktHdr->pInboundShortcutEntry->dport=rg_db.naptOut[naptIdx].extPort;	//external port
		pPktHdr->pInboundShortcutEntry->spa=RTK_RG_PORT_MAX;	//decided when inbou

#if defined(SHORTCUT_BITFILED_DEBUG)        
        assert(isTcp >= 0  && isTcp < (1 << SC_BFW_ISTCP));
        assert(pPktHdr->smacL2Idx < (1 << (SC_BFW_LUTIDX-1)));
        assert(pPktHdr->srcNetifIdx < (1 << (SC_BFW_INTFIDX-1)));
        //assert(pPktHdr->srcNetifIdx < (1 << (SC_BFW_EIPIDX-1)));
        assert(rg_db.naptOut[naptIdx].rtk_naptOut.hashIdx >= 0 && rg_db.naptOut[naptIdx].rtk_naptOut.hashIdx < (1 << (SC_BFW_NAPTIDX)));
        assert(isNapt >= 0 && isNapt < (1 << (SC_BFW_ISNAPT)));
        assert(isBridge >= 0 && isBridge < (1 << (SC_BFW_ISBRIDGE)));
        assert(_rtk_rg_shortcutARPFind(_rtk_rg_l3lookup(pPktHdr->ipv4Dip), pPktHdr->ipv4Dip) < (1 << (SC_BFW_ARPIDX-1)));
        assert(pPktHdr->dmacL2Idx < (1 << (SC_BFW_SMACL2IDX-1)));
        assert(pPktHdr->internalVlanID >= 0 && pPktHdr->internalVlanID < (1 << (SC_BFW_VLANID)));

#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
        assert(pPktHdr->dmacL2Idx < (1 << (SC_BFW_SMACL2IDX-1)));
#endif
        
#ifdef CONFIG_GPON_FEATURE		
        assert(pPktHdr->streamID >= 0 && pPktHdr->streamID < (1 << (SC_BFW_STREAMID)));
#endif
        assert(pPktHdr->ingressDecideVlanID >= 0 && pPktHdr->ingressDecideVlanID < (1 << (SC_BFW_INTERVLANID)));
#endif
		pPktHdr->pInboundShortcutEntry->isTcp=isTcp;
		pPktHdr->pInboundShortcutEntry->new_lut_idx=pPktHdr->smacL2Idx; //DMAC
		pPktHdr->pInboundShortcutEntry->new_intf_idx=pPktHdr->srcNetifIdx; //SMAC
		pPktHdr->pInboundShortcutEntry->new_eip_idx=pPktHdr->srcNetifIdx; //for outbound sip
		pPktHdr->pInboundShortcutEntry->naptIdx=rg_db.naptOut[naptIdx].rtk_naptOut.hashIdx;	//routing won't referenced this field
		pPktHdr->pInboundShortcutEntry->isNapt=isNapt;		//routing won't referenced this field
		pPktHdr->pInboundShortcutEntry->isBridge=isBridge;	//routing won't referenced this field
		pPktHdr->pInboundShortcutEntry->direction=NAPT_DIRECTION_INBOUND;		//routing won't referenced this field
		//20150115LUKE: keep ARP idx in shortcut for updating.
		pPktHdr->pInboundShortcutEntry->arpIdx=_rtk_rg_shortcutARPFind(_rtk_rg_l3lookup(pPktHdr->ipv4Dip), pPktHdr->ipv4Dip);
		//20150922: keep smac L2 index in shoutcut for updating.
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
		pPktHdr->pInboundShortcutEntry->smacL2Idx = pPktHdr->dmacL2Idx;
#endif

#ifdef CONFIG_GPON_FEATURE		
		pPktHdr->pInboundShortcutEntry->streamID=pPktHdr->streamID;
#endif
		pPktHdr->pInboundShortcutEntry->vlanID=pPktHdr->internalVlanID;
		pPktHdr->pInboundShortcutEntry->vlanTagif=(pPktHdr->tagif&CVLAN_TAGIF)>0;
		pPktHdr->pInboundShortcutEntry->dmac2cvlanID=FAIL;
		pPktHdr->pInboundShortcutEntry->dmac2cvlanTagif=0;

		pPktHdr->pInboundShortcutEntry->internalVlanID = pPktHdr->ingressDecideVlanID; //learn at syn, we should give it a reasonable internalVlan(now ithe vlan comes from _rtk_rg_ingressVlanDecision()) 
		/*
		if(pPktHdr->ingressPort>=RTK_RG_PORT_CPU)
		{
			pNaptSc->macPort=RTK_RG_PORT_CPU;
			pNaptSc->extPort=pPktHdr->ingressPort-RTK_RG_PORT_CPU;
		}
		else
		{
			pNaptSc->macPort=pPktHdr->ingressPort;
			pNaptSc->extPort=0;
		}
		*/
		if(rg_db.vlan[pPktHdr->internalVlanID].priorityEn==1)
		{
			pPktHdr->pInboundShortcutEntry->priority=rg_db.vlan[pPktHdr->internalVlanID].priority&0x7;
		}
		else
			pPktHdr->pInboundShortcutEntry->priority=0;
		pPktHdr->pInboundShortcutEntry->internalCFPri=0;
		pPktHdr->pInboundShortcutEntry->dscp=0;					
		pPktHdr->pInboundShortcutEntry->notFinishUpdated=0;
		TRACE("[Update SC:%d] dir:IN da=%d sa_intf=%d eipIdx=%d naptIdx=%d\n",freeIdx,pPktHdr->smacL2Idx,pPktHdr->srcNetifIdx,pPktHdr->srcNetifIdx,rg_db.naptOut[naptIdx].rtk_naptOut.hashIdx);
		TRACE("isTcp:%d src:0x%x:%d dst:0x%x:%d",isTcp,pPktHdr->ipv4Dip,pPktHdr->dport,rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,rg_db.naptOut[naptIdx].extPort);
		TABLE("Add SC: [idx=%d][sip=0x%x][dip=0x%x][sport=%d][dport=%d][isTcp=%d][isHairpin=%d]",freeIdx,pPktHdr->ipv4Sip,pPktHdr->ipv4Dip,pPktHdr->sport,pPktHdr->dport,isTcp,pPktHdr->pCurrentShortcutEntry->isHairpinNat);
	}
}
#endif

#if defined(CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT)||defined(CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT)
rtk_rg_entryGetReturn_t _rtk_rg_shortcutNEIGHBORFind(unsigned char *ipv6Addr)
{
	int i,neighborIdx=RG_RET_ENTRY_NOT_GET,hashValue,l3Idx;

	l3Idx=_rtk_rg_v6L3lookup(ipv6Addr);
	if(rg_db.v6route[l3Idx].rtk_v6route.type==L34_IPV6_ROUTE_TYPE_LOCAL)
	{
		hashValue=_rtk_rg_IPv6NeighborHash(ipv6Addr+8,l3Idx);
		DEBUG("Neighbor hashValue=%d  l3Idx=%d interfaceId(%02x%02x:%02x%02x:%02x%02x:%02x%02x) ",hashValue,l3Idx,
			ipv6Addr[8],ipv6Addr[9],ipv6Addr[10],ipv6Addr[11],ipv6Addr[12],ipv6Addr[13],ipv6Addr[14],ipv6Addr[15]);
		neighborIdx=(hashValue<<3);
		for(i=0;i<8;i++)		//8-way hash
		{
			//find the same entry first.
			if((rg_db.v6neighbor[neighborIdx+i].rtk_v6neighbor.valid==1)&&
				(rg_db.v6neighbor[neighborIdx+i].rtk_v6neighbor.ipv6RouteIdx==l3Idx)&&
				(memcmp(&rg_db.v6neighbor[neighborIdx+i].rtk_v6neighbor.ipv6Ifid,ipv6Addr+8,8)==0))
				break;
		}
		if(i==8)neighborIdx=RG_RET_ENTRY_NOT_GET;		//not found
	}

	return neighborIdx;
}

#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
rtk_rg_entryGetReturn_t _rtk_rg_v6ShortCutFreeEntryGet(int hashIdx, uint8* v6Sip, uint8* v6Dip, uint16 sport, uint16 dport, uint8 isTcp)
{
	rtk_ipv6_addr_t zeroIP={{0}};
	int i, first_invalid;
	uint32 longestIdx, longestIdletime;

	first_invalid = FAIL;
	longestIdx = hashIdx;
	longestIdletime = 0;
	for(i=hashIdx; i<hashIdx+MAX_NAPT_V6_SHORTCUT_WAYS; i++)
	{
		if(!memcmp(rg_db.naptv6ShortCut[i].sip.ipv6_addr, zeroIP.ipv6_addr, IPV6_ADDR_LEN)) //invalid
		{
			if(first_invalid==FAIL)
				first_invalid = i;
			continue;
		}
		if((!memcmp(rg_db.naptv6ShortCut[i].sip.ipv6_addr, v6Sip, IPV6_ADDR_LEN)) &&
			(!memcmp(rg_db.naptv6ShortCut[i].dip.ipv6_addr, v6Dip, IPV6_ADDR_LEN)) &&
			(rg_db.naptv6ShortCut[i].sport==sport) &&
			(rg_db.naptv6ShortCut[i].dport==dport) &&
			(rg_db.naptv6ShortCut[i].isTcp==isTcp))
			return i;
		
		if(rg_db.naptv6ShortCut[i].idleSecs > longestIdletime)
		{
			longestIdx = i;
			longestIdletime = rg_db.naptv6ShortCut[i].idleSecs;
		}
	}

	if(first_invalid!=FAIL)
		return first_invalid;
	
	if(longestIdletime==0) //choose last add index+1
	{
		DEBUG("V6shortcut LRU, do round robin!");
		longestIdx += ((rg_db.v6ShortCut_lastAddIdx[hashIdx>>MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT]+1) & (MAX_NAPT_V6_SHORTCUT_WAYS-1));
		longestIdletime = rg_db.naptv6ShortCut[longestIdx].idleSecs;
	}
	
	DEBUG("V6shortcut LRU choose victim[%d], idletime: %d secs", longestIdx, longestIdletime);
	_rtk_rg_v6ShortCut_delete(longestIdx);

	if(rg_db.systemGlobal.fwdStatistic)
		rg_db.systemGlobal.statistic.perPortCnt_v6ShortcutLRU[rg_db.pktHdr->ingressPort]++;
	
	return longestIdx;
	
}


__SRAM_FWDENG_SLOWPATH
void _rtk_rg_naptV6ShortcutUpdate(rtk_rg_pktHdr_t *pPktHdr, int isBridge)
{
	int isTcp=0,shortcutIdx=0;
	int freeIdx;

	if(pPktHdr->aclHit) return;

	if(pPktHdr->tagif&TCP_TAGIF)
		isTcp=1;

	if(pPktHdr->overMTU==1)
	{
		TRACE("over MTU...don't update shortCut!");
		return;
	}
	
	//20140813LUKE: use hash to get index!!
	shortcutIdx=_rtk_rg_ipv6ShortcutHashIndex(*((unsigned int *)(pPktHdr->pIpv6Sip+12)),pPktHdr->sport,pPktHdr->dport);
	freeIdx = _rtk_rg_v6ShortCutFreeEntryGet(shortcutIdx, pPktHdr->pIpv6Sip, pPktHdr->pIpv6Dip, pPktHdr->sport, pPktHdr->dport, isTcp);
	if(freeIdx==RG_RET_ENTRY_NOT_GET)
	{
		WARNING("Can not get free v6 shortcut entry!");
		return;
	}

	pPktHdr->currentV6ShortcutIdx=freeIdx;
	pPktHdr->pCurrentV6ShortcutEntry=&rg_db.naptv6ShortCut[freeIdx];
	pPktHdr->pCurrentV6ShortcutEntry->idleSecs= 0;
	rg_db.v6ShortCutValidSet[freeIdx>>5] |= (0x1<<(freeIdx&0x1f));
	rg_db.v6ShortCut_lastAddIdx[shortcutIdx>>MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT] = (freeIdx-shortcutIdx);
	
	pPktHdr->pCurrentV6ShortcutEntry->notFinishUpdated=1;
	pPktHdr->shortcutStatus=RG_SC_V6_NEED_UPDATE_BEFORE_SEND;
	memcpy(pPktHdr->pCurrentV6ShortcutEntry->sip.ipv6_addr,pPktHdr->pIpv6Sip,IPV6_ADDR_LEN); //SIP
	memcpy(pPktHdr->pCurrentV6ShortcutEntry->dip.ipv6_addr,pPktHdr->pIpv6Dip,IPV6_ADDR_LEN); //DIP
	pPktHdr->pCurrentV6ShortcutEntry->sport=pPktHdr->sport; //SPORT
	pPktHdr->pCurrentV6ShortcutEntry->dport=pPktHdr->dport; //DPORT

#if defined(SHORTCUT_BITFILED_DEBUG)
    assert(pPktHdr->ingressPort >= 0 && pPktHdr->ingressPort < (1 << (SC_BFW_SPA)));
    assert(isTcp >= 0 && isTcp < (1 << (SC_BFW_ISTCP)));
    assert(isBridge >=0 && isBridge < (1 << (SC_BFW_ISBRIDGE)));
    assert(pPktHdr->dmacL2Idx < (1 << (SC_BFW_LUTIDX-1)));
    assert(pPktHdr->netifIdx < (1 << (SC_BFW_INTFIDX-1)));
    assert(_rtk_rg_shortcutNEIGHBORFind(pPktHdr->pIpv6Sip) < (1 << (SC_BFW_NEIGHBORIDX-1)));
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
    assert(pPktHdr->smacL2Idx < (1 << (SC_BFW_SMACL2IDX-1)));
#endif

#endif

	pPktHdr->pCurrentV6ShortcutEntry->spa=pPktHdr->ingressPort;
	pPktHdr->pCurrentV6ShortcutEntry->isTcp=isTcp;
	pPktHdr->pCurrentV6ShortcutEntry->isBridge=isBridge;
	pPktHdr->pCurrentV6ShortcutEntry->new_lut_idx=pPktHdr->dmacL2Idx; //DMAC
	pPktHdr->pCurrentV6ShortcutEntry->new_intf_idx=pPktHdr->netifIdx; //SMAC
	//20150115LUKE: keep Neighbor idx in shortcut for updating.
	pPktHdr->pCurrentV6ShortcutEntry->neighborIdx=_rtk_rg_shortcutNEIGHBORFind(pPktHdr->pIpv6Sip);
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
	pPktHdr->pCurrentV6ShortcutEntry->smacL2Idx = pPktHdr->smacL2Idx;
#endif

	TRACE("[Update V6SC:%d] da=%d sa_intf=%d\n",freeIdx,pPktHdr->dmacL2Idx,pPktHdr->netifIdx);
	TRACE("isTcp:%d src:%08x:%08x:%08x:%08x:%d dst:%08x:%08x:%08x:%08x:%d",isTcp,
		*((unsigned int *)pPktHdr->pIpv6Sip),
		*((unsigned int *)(pPktHdr->pIpv6Sip+4)),
		*((unsigned int *)(pPktHdr->pIpv6Sip+8)),
		*((unsigned int *)(pPktHdr->pIpv6Sip+12)),
		pPktHdr->sport,
		*((unsigned int *)pPktHdr->pIpv6Dip),
		*((unsigned int *)(pPktHdr->pIpv6Dip+4)),
		*((unsigned int *)(pPktHdr->pIpv6Dip+8)),
		*((unsigned int *)(pPktHdr->pIpv6Dip+12)),
		pPktHdr->dport);
}
#endif
#endif

rtk_rg_successFailReturn_t _rtk_rg_getNaptOutFreeList(rtk_rg_table_naptOut_linkList_t	**pNaptOutFreeList)
{
	if(rg_db.pNaptOutFreeListHead==NULL) return RG_RET_FAIL;
	*pNaptOutFreeList=rg_db.pNaptOutFreeListHead;
	rg_db.pNaptOutFreeListHead=rg_db.pNaptOutFreeListHead->pNext;
	(*pNaptOutFreeList)->pNext=NULL;
	//DEBUG("FreeList=%d(%p)\n",(*pNaptOutFreeList)->idx,*pNaptOutFreeList);
	return RG_RET_SUCCESS;
}

rtk_rg_successFailReturn_t _rtk_rg_getNaptInFreeList(rtk_rg_table_naptIn_linkList_t	**pNaptInFreeList)
{
	if(rg_db.pNaptInFreeListHead==NULL) return RG_RET_FAIL;
	*pNaptInFreeList=rg_db.pNaptInFreeListHead;
	rg_db.pNaptInFreeListHead=rg_db.pNaptInFreeListHead->pNext;
	(*pNaptInFreeList)->pNext=NULL;
//	DEBUG("FreeList=%d(%x)\n",(*pNaptInFreeList)->idx,*pNaptInFreeList);
	return RG_RET_SUCCESS;
}

rtk_rg_successFailReturn_t _rtk_rg_addNaptOutHashList(int naptHashOutIdx,int *pNaptOutIdx)
{
	rtk_rg_table_naptOut_linkList_t *pNaptFreeOutList;	
	if(_rtk_rg_getNaptOutFreeList(&pNaptFreeOutList)==RG_RET_FAIL) return RG_RET_FAIL;

	rg_lock(&rg_kernel.naptTableLock);
	//========================critical region start=========================
	if(rg_db.pNaptOutHashListHead[naptHashOutIdx]==NULL)
	{
		rg_db.pNaptOutHashListHead[naptHashOutIdx]=pNaptFreeOutList;
	}
	else
	{		
		pNaptFreeOutList->pNext=rg_db.pNaptOutHashListHead[naptHashOutIdx];
		rg_db.pNaptOutHashListHead[naptHashOutIdx]=pNaptFreeOutList;	
	}
	*pNaptOutIdx=pNaptFreeOutList->idx;	
#if defined(NAPT_TABLE_SIZE_DEBUG)
    assert(naptHashOutIdx < (1 << MAX_NAPT_OUT_SW_ENTRY_WIDTH));
#endif
	rg_db.naptOut[*pNaptOutIdx].hashOutIdx=naptHashOutIdx;

	
	rg_unlock(&rg_kernel.naptTableLock);
	//========================critical region end=========================

	
	return RG_RET_SUCCESS;
}

rtk_rg_successFailReturn_t _rtk_rg_addNaptInHashList(int naptHashInIdx,int *pNaptInIdx)
{
	rtk_rg_table_naptIn_linkList_t *pNaptFreeInList;	
	if(_rtk_rg_getNaptInFreeList(&pNaptFreeInList)==RG_RET_FAIL) return RG_RET_FAIL;

	rg_lock(&rg_kernel.naptTableLock);
	//========================critical region start=========================

	if(rg_db.pNaptInHashListHead[naptHashInIdx]==NULL)
	{
		rg_db.pNaptInHashListHead[naptHashInIdx]=pNaptFreeInList;
	}
	else
	{		
		pNaptFreeInList->pNext=rg_db.pNaptInHashListHead[naptHashInIdx];
		rg_db.pNaptInHashListHead[naptHashInIdx]=pNaptFreeInList;	
	}
	*pNaptInIdx=pNaptFreeInList->idx;
#if defined(NAPT_TABLE_SIZE_DEBUG)    
    assert(naptHashInIdx < (1 << MAX_NAPT_IN_SW_ENTRY_WIDTH));
#endif
	rg_db.naptIn[*pNaptInIdx].hashIdx=naptHashInIdx;
	rg_unlock(&rg_kernel.naptTableLock);
	//========================critical region end=========================	
	return RG_RET_SUCCESS;
}

void _rtk_rg_freeFragOutList(int fragIdx, rtk_rg_ipv4_fragment_out_t *pReleaseOutList)
{
	//remove from hashIdx list
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragLock);
	if(rg_db.pFragOutHashListHead[fragIdx]==pReleaseOutList) 	//head need delete, move to next
		rg_db.pFragOutHashListHead[fragIdx]=pReleaseOutList->pNext;
	else
		pReleaseOutList->pPrev->pNext=pReleaseOutList->pNext;		//redirect free->prev's pointer pNext to next of free	

	if(pReleaseOutList->pNext!=NULL)		//last one
		pReleaseOutList->pNext->pPrev=pReleaseOutList->pPrev;		//redirect free->next's pointer pPrev to previous of free

	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv4FragLock);
	
	//add to free list		
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragFreeLock);
	pReleaseOutList->pNext=rg_db.pFragOutFreeListHead;
	rg_db.pFragOutFreeListHead=pReleaseOutList;
	//DEBUG("###### free out list..%p",pReleaseOutList);
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv4FragFreeLock);
				
	/*rtk_rg_ipv4_fragment_out_t *pPreDelOutList,*pDelOutList;
	
	pPreDelOutList=rg_db.pFragOutHashListHead[fragIdx];
	pDelOutList=pPreDelOutList;
	
	if(naptOutIdx==FAIL)		//free ICMP frag list
	{
		while(pDelOutList!=NULL)
		{
			if(pDelOutList->pktInfo.pICMPCtrlFlow==pICMPCtrlFlow)
			{
				//remove from hashIdx list
				//------------------ Critical Section start -----------------------//
				rg_lock(&rg_kernel.ipv4FragLock);
				if(pPreDelOutList==pDelOutList) 	//head need delete
					rg_db.pFragOutHashListHead[fragIdx]=pDelOutList->pNext;
				else
					pPreDelOutList->pNext=pDelOutList->pNext;
				//------------------ Critical Section End -----------------------//
				rg_unlock(&rg_kernel.ipv4FragLock);
				
				//add to free list		
				//------------------ Critical Section start -----------------------//
				rg_lock(&rg_kernel.ipv4FragFreeLock);
				pDelOutList->pNext=rg_db.pFragOutFreeListHead;
				rg_db.pFragOutFreeListHead=pDelOutList;
				//DEBUG("###### free it..%p",pDelOutList);
				//------------------ Critical Section End -----------------------//
				rg_unlock(&rg_kernel.ipv4FragFreeLock);
				break;
			}
			pPreDelOutList=pDelOutList;
			pDelOutList=pDelOutList->pNext;
		}		
	}
	else		//free napt frag list
	{
		while(pDelOutList!=NULL)
		{
			if(pDelOutList->NaptOutboundEntryIndex==naptOutIdx)
			{
				//remove from hashIdx list
				//------------------ Critical Section start -----------------------//
				rg_lock(&rg_kernel.ipv4FragLock);
				if(pPreDelOutList==pDelOutList) 	//head need delete
					rg_db.pFragOutHashListHead[fragIdx]=pDelOutList->pNext;
				else
					pPreDelOutList->pNext=pDelOutList->pNext;
				//------------------ Critical Section End -----------------------//
				rg_unlock(&rg_kernel.ipv4FragLock);
				
				//add to free list		
				//------------------ Critical Section start -----------------------//
				rg_lock(&rg_kernel.ipv4FragFreeLock);
				pDelOutList->pNext=rg_db.pFragOutFreeListHead;
				rg_db.pFragOutFreeListHead=pDelOutList;
				//DEBUG("###### free it..%p",pDelOutList);
				//------------------ Critical Section End -----------------------//
				rg_unlock(&rg_kernel.ipv4FragFreeLock);
				break;
			}
			pPreDelOutList=pDelOutList;
			pDelOutList=pDelOutList->pNext;
		}
	}*/
	
	//DEBUG("###### delete me..%d",fragIdx);
	//rg_db.ipv4FragmentOutTable[fragIdx].valid=0;
}

void _rtk_rg_freeFragInList(int fragIdx, rtk_rg_ipv4_fragment_in_t *pReleaseInList)
{
	//remove from hashIdx list
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragLock);
	if(rg_db.pFragInHashListHead[fragIdx]==pReleaseInList) 	//head need delete, move to next
		rg_db.pFragInHashListHead[fragIdx]=pReleaseInList->pNext;
	else
		pReleaseInList->pPrev->pNext=pReleaseInList->pNext;		//redirect free->prev's pointer pNext to next of free	

	if(pReleaseInList->pNext!=NULL)		//last one
		pReleaseInList->pNext->pPrev=pReleaseInList->pPrev;		//redirect free->next's pointer pPrev to previous of free

	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv4FragLock);
	
	//add to free list		
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragFreeLock);
	pReleaseInList->pNext=rg_db.pFragInFreeListHead;
	rg_db.pFragInFreeListHead=pReleaseInList;
	//DEBUG("###### free in list..%p",pReleaseInList);
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv4FragFreeLock);
	/*rtk_rg_ipv4_fragment_in_t *pPreDelInList,*pDelInList;

	pPreDelInList=rg_db.pFragInHashListHead[fragIdx];
	pDelInList=pPreDelInList;
	if(naptOutIdx==FAIL)		//free ICMP frag list
	{
		while(pDelInList!=NULL)
		{
			if(pDelInList->pktInfo.pICMPCtrlFlow==pICMPCtrlFlow)
			{
				//remove from hashIdx list
				//------------------ Critical Section start -----------------------//
				rg_lock(&rg_kernel.ipv4FragLock);			
				if(pPreDelInList==pDelInList)		//head need delete
					rg_db.pFragInHashListHead[fragIdx]=pDelInList->pNext;
				else
					pPreDelInList->pNext=pDelInList->pNext;
				//------------------ Critical Section End -----------------------//
				rg_unlock(&rg_kernel.ipv4FragLock);

				//add to free list
				//------------------ Critical Section start -----------------------//
				rg_lock(&rg_kernel.ipv4FragFreeLock);
				pDelInList->pNext=rg_db.pFragInFreeListHead;
				rg_db.pFragInFreeListHead=pDelInList;				
				//DEBUG("###### delete inbound frag list..%d",fragIdx);
				//------------------ Critical Section End -----------------------//
				rg_unlock(&rg_kernel.ipv4FragFreeLock);
				break;
			}
			pPreDelInList=pDelInList;
			pDelInList=pDelInList->pNext;
		}		
	}
	else		//free napt frag list
	{
		while(pDelInList!=NULL)
		{
			if(pDelInList->NaptOutboundEntryIndex==naptOutIdx)
			{
				//remove from hashIdx list
				//------------------ Critical Section start -----------------------//
				rg_lock(&rg_kernel.ipv4FragLock);			
				if(pPreDelInList==pDelInList)		//head need delete
					rg_db.pFragInHashListHead[fragIdx]=pDelInList->pNext;
				else
					pPreDelInList->pNext=pDelInList->pNext;
				//------------------ Critical Section End -----------------------//
				rg_unlock(&rg_kernel.ipv4FragLock);

				//add to free list
				//------------------ Critical Section start -----------------------//
				rg_lock(&rg_kernel.ipv4FragFreeLock);
				pDelInList->pNext=rg_db.pFragInFreeListHead;
				rg_db.pFragInFreeListHead=pDelInList;				
				//DEBUG("###### delete inbound frag list..%d",fragIdx);
				//------------------ Critical Section End -----------------------//
				rg_unlock(&rg_kernel.ipv4FragFreeLock);
				break;
			}
			pPreDelInList=pDelInList;
			pDelInList=pDelInList->pNext;
		}		
	}*/
	//DEBUG("###### delete me..%d",fragIdx);
	//rg_db.ipv4FragmentOutTable[fragIdx].valid=0;
}

void _rtk_rg_freeAllFragOutList(void)
{
	int i;
	
	for(i=0;i<MAX_NAPT_OUT_HW_TABLE_SIZE>>2;i++)
	{
		while(rg_db.pFragOutHashListHead[i]!=NULL)
		{
			//travel all list
			_rtk_rg_freeFragOutList(i,rg_db.pFragOutHashListHead[i]);
		}
	}
}

void _rtk_rg_freeAllFragInList(void)
{
	int i;
	
	for(i=0;i<MAX_NAPT_IN_HW_TABLE_SIZE>>2;i++)
	{
		while(rg_db.pFragInHashListHead[i]!=NULL)
		{
			//travel all list
			_rtk_rg_freeFragInList(i,rg_db.pFragInHashListHead[i]);
		}
	}
}

void _rtk_rg_getFragOutFreeList(rtk_rg_ipv4_fragment_out_t **pFragOutFreeList)
{
	if(rg_db.pFragOutFreeListHead==NULL)
	{
		//Clear all hashHead
		//mibdump_frag();
		DEBUG("============================== start clear OUT ======================");
		_rtk_rg_freeAllFragOutList();
		//mibdump_frag();
	}
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragFreeLock);
	*pFragOutFreeList=rg_db.pFragOutFreeListHead;
	rg_db.pFragOutFreeListHead=(*pFragOutFreeList)->pNext;
	(*pFragOutFreeList)->pNext=NULL;
	(*pFragOutFreeList)->pPrev=NULL;
	//DEBUG("get Free out List=%p\n",*pFragOutFreeList);
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv4FragFreeLock);

}

void _rtk_rg_getFragInFreeList(rtk_rg_ipv4_fragment_in_t **pFragInFreeList)
{
	if(rg_db.pFragInFreeListHead==NULL)
	{
		//Clear all hashHead
		//mibdump_frag();
		DEBUG("============================== start clear IN ======================");
		_rtk_rg_freeAllFragInList();
		//mibdump_frag();
	}
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragFreeLock);
	*pFragInFreeList=rg_db.pFragInFreeListHead;
	rg_db.pFragInFreeListHead=(*pFragInFreeList)->pNext;
	(*pFragInFreeList)->pNext=NULL;
	(*pFragInFreeList)->pPrev=NULL;
	//DEBUG("get Free in List=%p\n",*pFragInFreeList);	
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv4FragFreeLock);

}

__SRAM_FWDENG_SLOWPATH
rtk_rg_entryGetReturn_t _rtk_rg_swNaptOutFreeEntryGet(int naptHashOutIdx)
{
	int naptOutIdx;
	
	//found a free naptOut from link list		
	if(_rtk_rg_addNaptOutHashList(naptHashOutIdx,&naptOutIdx)==RG_RET_SUCCESS)
	{
		TRACE("free SW NaptOutIdx=%d",naptOutIdx);
		return naptOutIdx;
	}
	else
	{
#ifdef CONFIG_ROME_NAPT_LRU	
		//delete the longest idle connection.(by naptHashInIdx)
		int longestIdleIdx=0;
		uint32 longestIdleSec;
		int cnt=0;
		rtk_rg_successFailReturn_t ret=0;
		int delta=naptHashOutIdx;
		int loopTimes=0;
		
		//search from link list
		rtk_rg_table_naptOut_linkList_t *pNaptOutList,*pPreNaptOutList;
delete_again:
		cnt=0;
		longestIdleSec=0;
		rg_lock(&rg_kernel.naptTableLock);
		//========================critical region start=========================		
		pPreNaptOutList=rg_db.pNaptOutHashListHead[delta];
		pNaptOutList=pPreNaptOutList;

		if(pNaptOutList!=NULL)
			longestIdleIdx=pNaptOutList->idx;

		
		while(pNaptOutList!=NULL)
		{
			if(rg_db.naptOut[pNaptOutList->idx].idleSecs>longestIdleSec)
			{
				longestIdleSec=rg_db.naptOut[pNaptOutList->idx].idleSecs;
				longestIdleIdx=pNaptOutList->idx;
			}
			
			pPreNaptOutList=pNaptOutList;
			pNaptOutList=pNaptOutList->pNext;
			cnt++;
		}
		rg_unlock(&rg_kernel.naptTableLock);
		//========================critical region end=========================		

		//find a entry to delete.
		if((longestIdleSec==0)&&(loopTimes<=MAX_NAPT_OUT_HW_TABLE_SIZE>>2)&&(cnt!=0))
		{
			++delta;
			++loopTimes;
			if(delta>=(MAX_NAPT_OUT_HW_TABLE_SIZE>>2))
				delta=0;
			TABLE("delete outHashIdx=%d cnt=%d rg_db.pNaptOutHashListHead[delta]=%p",delta,cnt,rg_db.pNaptOutHashListHead[delta]);
			goto delete_again;
		}

		//delete the entry				
		ret=(pf.rtk_rg_naptConnection_del)(longestIdleIdx);
		if(ret)
		{
			//if the entry delete fail, find another.
			++delta;
			++loopTimes;
			if(delta>=(MAX_NAPT_OUT_HW_TABLE_SIZE>>2))
				delta=0;
			DEBUG("rtk_rg_naptConnection_del(longestIdleIdx)=[ret:%x][loopTimes=%d]\n",ret,loopTimes);
			if(loopTimes<=MAX_NAPT_OUT_HW_TABLE_SIZE>>2)
			{
				goto delete_again;
			}
		}
		else
		{			
			TABLE("[NAPT] outbound full, delete the longest idle connection[%d]. IdleTime=%d secs, searchTimes=%d",longestIdleIdx,longestIdleSec,cnt);
		}

		// add flow to free-list
		ret=_rtk_rg_addNaptOutHashList(naptHashOutIdx,&naptOutIdx);
		if(ret==RG_RET_FAIL)
		{
			//if flow not found in free list. delete another.
			++delta;
			++loopTimes;
			if(delta>=(MAX_NAPT_OUT_HW_TABLE_SIZE>>2))
				delta=0;
			DEBUG("_rtk_rg_addNaptOutHashList=[ret:%x][loopTimes=%d]\n",ret,loopTimes);
			if(loopTimes<=MAX_NAPT_OUT_HW_TABLE_SIZE>>2)
			{
				goto delete_again;
			}
		}

		if(rg_db.systemGlobal.fwdStatistic)
			rg_db.systemGlobal.statistic.perPortCnt_naptOutLRU[rg_db.pktHdr->ingressPort]++;

		if(loopTimes>(MAX_NAPT_OUT_HW_TABLE_SIZE>>2))
		{
			WARNING("can't find any free NAPT entry to delete!");
			return RG_RET_ENTRY_NOT_GET;
		}			
		
		return naptOutIdx;
#else
		return RG_RET_ENTRY_NOT_GET;
#endif
	}
}

__SRAM_FWDENG_SLOWPATH
rtk_rg_entryGetReturn_t _rtk_rg_swNaptInFreeEntryGet(int naptHashInIdx)
{
	int naptInIdx;

	//found a free naptIn from link list		
	if(_rtk_rg_addNaptInHashList(naptHashInIdx,&naptInIdx)==RG_RET_SUCCESS)
	{
		TRACE("free SW NaptInIdx=%d",naptInIdx);
		return naptInIdx;
	}
	else
	{
#ifdef CONFIG_ROME_NAPT_LRU	
		//delete the longest idle connection.(by naptHashInIdx)
		int longestIdleIdx=0;
		uint32 longestIdleSec=0;
		int cnt=0,ret;
//		int naptHashOutIdx;
		int delta=naptHashInIdx;
		int loopTimes=0;
		
		//search from link list
		rtk_rg_table_naptIn_linkList_t *pNaptInList,*pPreNaptInList;
delete_again:
		cnt=0;
		longestIdleSec=0;
		rg_lock(&rg_kernel.naptTableLock);
		//========================critical region start=========================		
		pPreNaptInList=rg_db.pNaptInHashListHead[delta];
		pNaptInList=pPreNaptInList;

		if(pNaptInList!=NULL)
			longestIdleIdx=pNaptInList->idx;

		
		while(pNaptInList!=NULL)
		{
			if((rg_db.naptIn[pNaptInList->idx].idleSecs>longestIdleSec)&&(rg_db.naptIn[pNaptInList->idx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT))
			{
				longestIdleSec=rg_db.naptIn[pNaptInList->idx].idleSecs;
				longestIdleIdx=pNaptInList->idx;
			}
			
			pPreNaptInList=pNaptInList;
			pNaptInList=pNaptInList->pNext;
			cnt++;
		}

		rg_unlock(&rg_kernel.naptTableLock);
		//========================critical region end=========================		

		if((longestIdleSec==0)&&(loopTimes<=(MAX_NAPT_IN_HW_TABLE_SIZE>>2))&&(cnt!=0))
		{
			delta++;
			loopTimes++;
			if(delta>=(MAX_NAPT_IN_HW_TABLE_SIZE>>2))
				delta=0;
			goto delete_again;			
		}

#if 0
		naptHashOutIdx=_rtk_rg_naptTcpUdpOutHashIndex(isTcp
		,rg_db.naptIn[longestIdleIdx].rtk_naptIn.intIp
		,rg_db.naptIn[longestIdleIdx].rtk_naptIn.intPort
		,rg_db.naptIn[longestIdleIdx].remoteIp
		,rg_db.naptIn[longestIdleIdx].remotePort);

		ret=rtk_rg_naptConnection_del(naptHashOutIdx);
#else
		if(rg_db.naptIn[longestIdleIdx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT)
			ret=(pf.rtk_rg_naptConnection_del)(rg_db.naptIn[longestIdleIdx].symmetricNaptOutIdx);
		else //don't kill NAPT_IN_TYPE_RESTRICTED_CONE & NAPT_IN_TYPE_FULL_CONE
			ret=FAIL;

#endif		
		if(ret)
		{
			//if the entry delete fail, find another.
			++delta;
			++loopTimes;
			if(delta>=(MAX_NAPT_IN_HW_TABLE_SIZE>>2))
				delta=0;
			DEBUG("rtk_rg_naptConnection_del(longestIdleIdx)=[ret:%d][loopTimes=%d]\n",ret,loopTimes);
			if(loopTimes<=MAX_NAPT_IN_HW_TABLE_SIZE>>2)
			{
				goto delete_again;
			}
		}

		TABLE("[NAPT] inbound full, delete the longest idle connection[%d]. IdleTime=%d secs, searchTimes=%d",longestIdleIdx,longestIdleSec,cnt);

		ret=_rtk_rg_addNaptInHashList(naptHashInIdx,&naptInIdx);
		if(ret==FAIL)
		{
			//if flow not found in free list. delete another.
			++delta;
			++loopTimes;
			if(delta>=(MAX_NAPT_IN_HW_TABLE_SIZE>>2))
				delta=0;
			DEBUG("_rtk_rg_addNaptInHashList=[ret:%x][loopTimes=%d]\n",ret,loopTimes);
			if(loopTimes<=MAX_NAPT_IN_HW_TABLE_SIZE>>2)
			{
				goto delete_again;
			}
		}

		if(rg_db.systemGlobal.fwdStatistic)
			rg_db.systemGlobal.statistic.perPortCnt_naptInLRU[rg_db.pktHdr->ingressPort]++;
		
		if(loopTimes>(MAX_NAPT_IN_HW_TABLE_SIZE>>2))
		{
			WARNING("can't find any free NAPT entry to delete!");
			return RG_RET_ENTRY_NOT_GET;
		}		
		return naptInIdx;
#else
		return RG_RET_ENTRY_NOT_GET;
#endif
	}
}


__SRAM_FWDENG_SLOWPATH	
rtk_rg_entryGetReturn_t _rtk_rg_naptTcpUdpOutFreeEntryGet(int addNaptSwOnly, rtk_rg_algAction_t algAction, int8 isTcp, ipaddr_t srcAddr, uint16 srcPort, ipaddr_t destAddr, uint16 destPort,int *pHashOutIdx)
{
	int naptHashOutIdx;
	int naptOutIdx;
	int i;
	
	//found a free naptOut entry				
	naptHashOutIdx=_rtk_rg_naptTcpUdpOutHashIndex(isTcp,srcAddr,srcPort,destAddr,destPort);
	if(pHashOutIdx!=NULL) *pHashOutIdx=naptHashOutIdx;
	naptOutIdx=naptHashOutIdx<<2;

	//Check for ALG
	if(algAction==RG_ALG_ACT_TO_FWDENGINE || addNaptSwOnly)
	{
		//DEBUG("Add to pure software..naptHashOutIdx=%d %p",naptHashOutIdx,rg_db.pNaptOutFreeListHead);
		goto PURESW;
	}

	for(i=naptOutIdx;i<naptOutIdx+4;i++)
	{
		if(rg_db.naptOut[i].rtk_naptOut.valid==0) 
		{
			naptOutIdx=i;			
			return naptOutIdx;
		}
	}

	if(rg_db.systemGlobal.tcpDoNotDelWhenRstFin==1)
	{
		for(i=naptOutIdx;i<naptOutIdx+4;i++)
		{
			if(rg_db.naptOut[i].canBeReplaced==1) 
			{
				naptOutIdx=i;
				return naptOutIdx;
			}
		}	
	}
		
	if(i==naptOutIdx+4)
	{
PURESW:
		//FIXME("NAPT 4-ways is full - found link list");	
			
		return _rtk_rg_swNaptOutFreeEntryGet(naptHashOutIdx);
	}
	return RG_RET_ENTRY_NOT_GET;
}


/*
	if hw has free return hw_idx first
	else if sw has free return software second
	else {#if CONFIG_ROME_NAPT_LRU try to delete napt by longest idle connection and return idx} return RG_RET_FAIL
*/
__SRAM_FWDENG_SLOWPATH
#if defined(CONFIG_APOLLO)
rtk_rg_entryGetReturn_t _rtk_rg_naptTcpUdpInFreeEntryGet(int addNaptSwOnly, rtk_rg_algAction_t algAction, int8 isTcp,int extIp,int extPort,int *pHashInIdx)
#elif defined(CONFIG_XDSL_ROMEDRIVER)
int _rtk_rg_naptTcpUdpInFreeEntryGet(int addNaptSwOnly, rtk_rg_algAction_t algAction,int8 isTcp,uint32 remoteIp,uint16 remotePort,int extIp,int extPort,int *pHashInIdx)
#endif
{
	int naptHashInIdx;
	int naptInIdx;
	int i;
	int inBoundFourWayHash=4;

#if defined(CONFIG_APOLLO)
	naptHashInIdx=_rtk_rg_naptTcpUdpInHashIndex(isTcp,extIp,extPort);
	naptInIdx=naptHashInIdx<<2;
#elif defined(CONFIG_XDSL_ROMEDRIVER)
	inBoundFourWayHash=1;//xdsl napt inbound only 1-way
	naptHashInIdx=_rtk_rg_naptTcpUdpInHashIndex(isTcp,remoteIp,remotePort,extIp,extPort);
	naptInIdx=naptHashInIdx;
	naptHashInIdx=naptHashInIdx>>2; //change to 4-way index for sw , xdsl inbound only 1-way
#endif
	if(pHashInIdx) *pHashInIdx=naptHashInIdx;



	//Check for ALG
	if(algAction==RG_ALG_ACT_TO_FWDENGINE || addNaptSwOnly)
	{
		//DEBUG("Add to pure software..naptHashOutIdx=%d %p",naptHashOutIdx,rg_db.pNaptOutFreeListHead);
		goto PURESW;
	}


	/* Search the same inbound full cone entry first in 4-way, reuse this entry if found */
#ifdef CONFIG_RG_NAPT_VIRTUAL_SERVER_SUPPORT

	if((rg_db.pktHdr->naptrLookupHit==2)&&(rg_db.systemGlobal.initParam.naptInboundConnLookupSecondCallBack==_rtk_rg_fwdEngine_virtualServerCheck))
	{
		for(i=naptInIdx;i<naptInIdx+inBoundFourWayHash;i++)
		{
			if((rg_db.naptIn[i].rtk_naptIn.valid==ASIC_NAPT_IN_TYPE_FULL_CONE)&&
				(rg_db.extip[rg_db.naptIn[i].rtk_naptIn.extIpIdx].rtk_extip.extIpAddr==extIp)&&
#if defined(CONFIG_RTL9602C_SERIES)
				((rg_db.naptIn[i].rtk_naptIn.extPortHSB<<8 | rg_db.naptIn[i].rtk_naptIn.extPortLSB)==extPort)&&
#else
				((rg_db.naptIn[i].rtk_naptIn.extPortLSB&0xff)==(extPort&0xff))&&
#endif
				(rg_db.naptIn[i].rtk_naptIn.isTcp==isTcp))
			{
				naptInIdx=i;
				TRACE("Reuse Full-Cone inbound NAPT index[%d]",naptInIdx);
				return naptInIdx;
			}
		}
	}
#endif
	
	for(i=naptInIdx;i<naptInIdx+inBoundFourWayHash;i++)
	{
		if(rg_db.naptIn[i].rtk_naptIn.valid==ASIC_NAPT_IN_TYPE_INVALID)
		{
			naptInIdx=i;
			TRACE("free HW NaptInIdx=%d",naptInIdx);
			return naptInIdx;		
		}	
	}





	


	if(rg_db.systemGlobal.tcpDoNotDelWhenRstFin==1)
	{
		for(i=naptInIdx;i<naptInIdx+inBoundFourWayHash;i++)
		{
			if(rg_db.naptIn[i].canBeReplaced==1) 
			{
				naptInIdx=i;
				return naptInIdx;
			}
		}	
	}	

PURESW:	
	//FIXME("NAPTR 4-ways is full - found link list");			
	
	return _rtk_rg_swNaptInFreeEntryGet(naptHashInIdx);

}

#if	defined(CONFIG_RG_NAPT_UPNP_SUPPORT)
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_upnpCheck(void *data,ipaddr_t *transIP,uint16 *transPort)
{
	int upnpIdx=0;
	int ret;
	rtk_rg_upnpConnection_t upnp;
	rtk_rg_pktHdr_t *pPktHdr=(rtk_rg_pktHdr_t *)data;

	DEBUG("UPNP Check...");
	memset(&upnp,0,sizeof(upnp));
	
	while(1)
	{
		ret = (pf.rtk_rg_upnpConnection_find)(&upnp,&upnpIdx);
		if(ret!=RT_ERR_RG_OK) break;

		if(((upnp.is_tcp && (pPktHdr->tagif&TCP_TAGIF)) || (upnp.is_tcp==0 && (pPktHdr->tagif&UDP_TAGIF)))&&
			(upnp.gateway_port == pPktHdr->dport) &&
			(upnp.wan_intf_idx == pPktHdr->netifIdx) &&
			((upnp.limit_remote_ip==0) || (upnp.remote_ip==pPktHdr->ipv4Sip)) &&
			((upnp.limit_remote_port==0) || (upnp.remote_port==pPktHdr->sport)))
		{
			*transIP = upnp.local_ip;
			*transPort = upnp.local_port;
			DEBUG("hit. ==> [localIP:0x%x] [localPort:0x%x]\n",pPktHdr->ipv4Dip,pPktHdr->dport);

			if(upnp.type == UPNP_TYPE_ONESHOT)
				(pf.rtk_rg_upnpConnection_del)(upnpIdx);

			//for PPTP and L2TP WAN, we just add to sw napt list!!
			//Dslite, too
			if(rg_db.systemGlobal.interfaceInfo[upnp.wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP ||
				rg_db.systemGlobal.interfaceInfo[upnp.wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP ||
				rg_db.systemGlobal.interfaceInfo[upnp.wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE ||
				rg_db.systemGlobal.interfaceInfo[upnp.wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)
				pPktHdr->addNaptSwOnly=1;

			return RG_RET_SUCCESS;
		}
		upnpIdx++;
	}
	DEBUG("not hit.\n");
	return RG_RET_FAIL;
}
#endif

#if	defined(CONFIG_RG_NAPT_VIRTUAL_SERVER_SUPPORT)
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_virtualServerCheck(void *data,ipaddr_t *transIP,uint16 *transPort)
{
	int vsIdx=0;
	int ret;
	int off;
	rtk_rg_virtualServer_t virtualServer;
	rtk_rg_pktHdr_t *pPktHdr=(rtk_rg_pktHdr_t *)data;
	unsigned int tmpmask;

	DEBUG("Virtual Server Check...");

	memset(&virtualServer,0,sizeof(virtualServer));
	
	while(1)
	{
		ret = (pf.rtk_rg_virtualServer_find)(&virtualServer,&vsIdx);
		if(ret!=RT_ERR_RG_OK) break;
		
		if(((virtualServer.is_tcp && (pPktHdr->tagif&TCP_TAGIF)) || (virtualServer.is_tcp==0 && (pPktHdr->tagif&UDP_TAGIF)))&&
			((virtualServer.disable_wan_check)||(virtualServer.wan_intf_idx == pPktHdr->netifIdx)) &&
			((pPktHdr->dport>=virtualServer.gateway_port_start) && (pPktHdr->dport<(virtualServer.gateway_port_start+virtualServer.mappingPortRangeCnt))))
		{
			off=pPktHdr->dport-virtualServer.gateway_port_start;
			*transIP = virtualServer.local_ip;
			if(virtualServer.mappingType==VS_MAPPING_N_TO_N)
				*transPort = virtualServer.local_port_start+off;
			else
				*transPort = virtualServer.local_port_start;
			
			TRACE("Virtual Server HIT.==> [localIP:0x%x] [localPort:0x%x] mappingType=[%s]\n",pPktHdr->ipv4Dip,pPktHdr->dport,(virtualServer.mappingType==VS_MAPPING_N_TO_N)?"N-to-N":"N-to-1");

			//for PPTP and L2TP WAN, we just add to sw napt list!!
			//Dslite, too
			if(rg_db.systemGlobal.interfaceInfo[virtualServer.wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP ||
				rg_db.systemGlobal.interfaceInfo[virtualServer.wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP ||
				rg_db.systemGlobal.interfaceInfo[virtualServer.wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE ||
				rg_db.systemGlobal.interfaceInfo[virtualServer.wan_intf_idx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)
				pPktHdr->addNaptSwOnly=1;

			//20150114LUKE: for sever-in-lan ALG while hit virtualServer
			if(virtualServer.hookAlgType && (pPktHdr->algAction==RG_ALG_ACT_NORMAL))
			{
				ret=0;
				tmpmask=virtualServer.hookAlgType;
				while(tmpmask>>=1)ret++;
				pPktHdr->algAction=RG_ALG_ACT_TO_FWDENGINE;
				pPktHdr->algRegFun=rg_db.algTcpFunctionMapping[ret].registerFunction;
				pPktHdr->addNaptSwOnly=1;
				TRACE("Hook ALG[%x]!!",virtualServer.hookAlgType);
			}
			return RG_RET_SUCCESS;
		}
		vsIdx++;
	}
	
	DEBUG("not hit.\n");
	return RG_RET_FAIL;
}
#endif

#if	defined(CONFIG_RG_NAPT_DMZ_SUPPORT)
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_dmzCheck(void *data,ipaddr_t *transIP,uint16 *transPort)
{
	rtk_rg_pktHdr_t *pPktHdr=(rtk_rg_pktHdr_t *)data;
	DEBUG("DMZ Check...");

	if(rg_db.dmzInfo[pPktHdr->netifIdx].enabled && (rg_db.dmzInfo[pPktHdr->netifIdx].mac_mapping_enabled==0))
	{
		*transIP = rg_db.dmzInfo[pPktHdr->netifIdx].private_ip;
		DEBUG("hit. ==> [localIP:0x%x] [localPort:0x%x]\n",pPktHdr->ipv4Dip,pPktHdr->dport);

		//for PPTP and L2TP WAN, we just add to sw napt list!!
		//Dslite, too
		if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP ||
			rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP ||
			rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE ||
			rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)
			pPktHdr->addNaptSwOnly=1;
	}
	else
	{
		DEBUG("not hit.\n");
		return RG_RET_FAIL;
	}

	return RG_RET_SUCCESS;
}
#endif

rtk_rg_fwdEngineReturn_t _rtk_rg_gatewayServicePort_check(rtk_rg_pktHdr_t *pPktHdr){
	int i;
	for(i=0;i<MAX_GATEWAYSERVICEPORT_TABLE_SIZE;i++){		
		if(rg_db.gatewayServicePortEntry[i].valid==ENABLED){
			if(rg_db.gatewayServicePortEntry[i].type==GATEWAY_SERVER_SERVICE){//check dport
				if(pPktHdr->dport==rg_db.gatewayServicePortEntry[i].port_num){
					TRACE("Hit gatewayServicePort[%d]: port_num=%d type=%s. trap to PS",i,rg_db.gatewayServicePortEntry[i].port_num,rg_db.gatewayServicePortEntry[i].type?"CLIENT(sport)":"SERVER(dport)");
					return RG_FWDENGINE_RET_TO_PS;
				}
			}else if(rg_db.gatewayServicePortEntry[i].type==GATEWAY_CLIENT_SERVICE){//check sport
				if(pPktHdr->sport==rg_db.gatewayServicePortEntry[i].port_num){
					TRACE("Hit gatewayServicePort[%d]: port_num=%d type=%s. trap to PS",i,rg_db.gatewayServicePortEntry[i].port_num,rg_db.gatewayServicePortEntry[i].type?"CLIENT(sport)":"SERVER(dport)");
					return RG_FWDENGINE_RET_TO_PS;
				}
			}	
		}
	}	
	return RG_FWDENGINE_RET_CONTINUE;
	
}



#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT

#if	defined(CONFIG_RG_NAPT_VIRTUAL_SERVER_SUPPORT)
int _rtk_rg_fwdEngine_ipv6VirtualServerCheck(void *data,rtk_ipv6_addr_t *transIP,uint16 *transPort)
{
	int vsIdx=0;
	int ret;
	int off;
	rtk_rg_virtualServer_t virtualServer;
	rtk_rg_pktHdr_t *pPktHdr=(rtk_rg_pktHdr_t *)data;

	DEBUG("Virtual Server Check...");

	memset(&virtualServer,0,sizeof(virtualServer));
	
	for(vsIdx=0;vsIdx<MAX_VIRTUAL_SERVER_SW_TABLE_SIZE;vsIdx++)
	{
		ret = rtk_rg_apollo_virtualServer_find(&virtualServer,&vsIdx);
		if(ret!=RT_ERR_RG_OK) continue;
		
		if(((pPktHdr->extipIdx!=FAIL))&&
			(virtualServer.valid==ENABLED)&&
			((virtualServer.is_tcp && (pPktHdr->tagif&TCP_TAGIF)) || (virtualServer.is_tcp==0 && (pPktHdr->tagif&UDP_TAGIF)))&&
			(virtualServer.wan_intf_idx == rg_db.nexthop[rg_db.extip[pPktHdr->extipIdx].rtk_extip.nhIdx].rtk_nexthop.ifIdx) &&
			((pPktHdr->dport>=virtualServer.gateway_port_start) && (pPktHdr->dport<(virtualServer.gateway_port_start+virtualServer.mappingPortRangeCnt))))
		{
			off=pPktHdr->dport-virtualServer.gateway_port_start;
			memcpy(transIP->ipv6_addr,virtualServer.local_ipv6.ipv6_addr,IPV6_ADDR_LEN);
			if(virtualServer.mappingType==VS_MAPPING_N_TO_N)
				*transPort = virtualServer.local_port_start+off;
			else
				*transPort = virtualServer.local_port_start;

			//record to pktHdr
			pPktHdr->ipv6_serverInLanLookup.serverInLanHit = RTK_RG_IPV6_LOOKUP_VIRTUALSERVER_HIT;
			pPktHdr->ipv6_serverInLanLookup.hitIndex = vsIdx;
			memcpy(pPktHdr->ipv6_serverInLanLookup.transIP.ipv6_addr,virtualServer.local_ipv6.ipv6_addr,IPV6_ADDR_LEN);
			pPktHdr->ipv6_serverInLanLookup.transPort=*transPort; 

			TRACE("Virtual Server HIT.==> [localIP: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x] [localPort:0x%x] mappingType=[%s]\n",
				transIP->ipv6_addr[0],transIP->ipv6_addr[1],transIP->ipv6_addr[2],transIP->ipv6_addr[3],
				transIP->ipv6_addr[4],transIP->ipv6_addr[5],transIP->ipv6_addr[6],transIP->ipv6_addr[7],
				transIP->ipv6_addr[8],transIP->ipv6_addr[9],transIP->ipv6_addr[10],transIP->ipv6_addr[11],
				transIP->ipv6_addr[12],transIP->ipv6_addr[13],transIP->ipv6_addr[14],transIP->ipv6_addr[15],
				*transPort,(virtualServer.mappingType==VS_MAPPING_N_TO_N)?"N-to-N":"N-to-1");

			return RT_ERR_RG_OK;
		}
	}
	
	DEBUG("not hit.\n");
	return RT_ERR_RG_FAILED;
}
#endif




#if	defined(CONFIG_RG_NAPT_DMZ_SUPPORT)
int _rtk_rg_fwdEngine_ipv6DmzCheck(void *data,rtk_ipv6_addr_t *transIP,uint16 *transPort)
{
	int netifIdx;
	rtk_rg_pktHdr_t *pPktHdr=(rtk_rg_pktHdr_t *)data;
	
	DEBUG("DMZ Check...");

	if(pPktHdr->extipIdx!=FAIL){
		netifIdx=rg_db.nexthop[rg_db.extip[pPktHdr->extipIdx].rtk_extip.nhIdx].rtk_nexthop.ifIdx;
		DEBUG("netifIdx=%d, enabled=%d mac_mapping_enabled=%d",netifIdx,rg_db.dmzInfo[netifIdx].enabled,rg_db.dmzInfo[netifIdx].mac_mapping_enabled);
		if(rg_db.dmzInfo[netifIdx].enabled && (rg_db.dmzInfo[netifIdx].mac_mapping_enabled==0))
		{
			//bring to up layer function for DIP lookup.
			memcpy(transIP->ipv6_addr,rg_db.dmzInfo[netifIdx].private_ipv6.ipv6_addr,IPV6_ADDR_LEN);

			//record to pktHdr
			pPktHdr->ipv6_serverInLanLookup.serverInLanHit = RTK_RG_IPV6_LOOKUP_DMZ_HIT;
			pPktHdr->ipv6_serverInLanLookup.hitIndex = netifIdx;
			memcpy(pPktHdr->ipv6_serverInLanLookup.transIP.ipv6_addr,rg_db.dmzInfo[netifIdx].private_ipv6.ipv6_addr,IPV6_ADDR_LEN);
			pPktHdr->ipv6_serverInLanLookup.transPort=*transPort; 
			
			DEBUG("hit DMZ[%d]. ==> [localIP %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x] \n",netifIdx,
				transIP->ipv6_addr[0],transIP->ipv6_addr[1],transIP->ipv6_addr[2],transIP->ipv6_addr[3],
				transIP->ipv6_addr[4],transIP->ipv6_addr[5],transIP->ipv6_addr[6],transIP->ipv6_addr[7],
				transIP->ipv6_addr[8],transIP->ipv6_addr[9],transIP->ipv6_addr[10],transIP->ipv6_addr[11],
				transIP->ipv6_addr[12],transIP->ipv6_addr[13],transIP->ipv6_addr[14],transIP->ipv6_addr[15]);
			return RT_ERR_RG_OK;
		}
	}

	DEBUG("not hit.\n");
	return RT_ERR_RG_FAILED;	
}
#endif


int _rtk_rg_fwdEngine_ipv6ConnType_lookup(rtk_rg_pktHdr_t *pPktHdr, rtk_ipv6_addr_t *transIP, int16 *transPort)
{
	int r1=-1,r2=-1,r3=-1;
	if((pPktHdr==NULL) || (transIP==NULL) || (transPort==NULL)) return RG_FWDENGINE_RET_TO_PS;

	//IPv6 use another set of l4_port
	//ret = _rtk_rg_gatewayServicePort_check(pPktHdr);
	//if(ret==RG_FWDENGINE_RET_TO_PS) return RG_FWDENGINE_RET_TO_PS;

	pPktHdr->ipv6_serverInLanLookup.serverInLanHit = RTK_RG_IPV6_LOOKUP_NONE_HIT;
	//Inbound connection table lookup, e.g. UPNP,virtual server & DMZ tables.
	if(((rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupFirstCallBack!=NULL) && ((r1=rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupFirstCallBack(pPktHdr,transIP,transPort))==RT_ERR_OK)) ||
		((rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupSecondCallBack!=NULL) && ((r2=rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupSecondCallBack(pPktHdr,transIP,transPort))==RT_ERR_OK)) ||
		((rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupThirdCallBack!=NULL) && ((r3=rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupThirdCallBack(pPktHdr,transIP,transPort))==RT_ERR_OK)))
	{
		if(r1==RT_ERR_OK)
		{
#if	defined(CONFIG_RG_NAPT_UPNP_SUPPORT)

			//pPktHdr->ipv6_serverInLanLookup.serverInLanHit = RTK_RG_IPV6_LOOKUP_UPNP_HIT;
			//if(rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupFirstCallBack==_rtk_rg_fwdEngine_ipv6UpnpCheck)
			//{
				//TRACE("UPnP entry hit!");
			//}
#endif
		}
		else if(r2==RT_ERR_OK)
		{
#ifdef CONFIG_RG_NAPT_VIRTUAL_SERVER_SUPPORT

			pPktHdr->ipv6_serverInLanLookup.serverInLanHit = RTK_RG_IPV6_LOOKUP_VIRTUALSERVER_HIT;
			if(rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupSecondCallBack==_rtk_rg_fwdEngine_ipv6VirtualServerCheck)
			{
				TRACE("Server Port entry hit!");
			}
#endif

		}
		else if(r3==RT_ERR_OK)
		{
#ifdef CONFIG_RG_NAPT_DMZ_SUPPORT
			pPktHdr->ipv6_serverInLanLookup.serverInLanHit = RTK_RG_IPV6_LOOKUP_DMZ_HIT;
			if(rg_db.systemGlobal.initParam.ipv6NaptInboundConnLookupThirdCallBack==_rtk_rg_fwdEngine_ipv6DmzCheck)
			{
				TRACE("DMZ hit!");
			}	
#endif
		}
		return RG_FWDENGINE_RET_CONTINUE;
	}

	return RG_FWDENGINE_RET_TO_PS;
}
#endif


rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_connType_lookup(rtk_rg_pktHdr_t *pPktHdr, ipaddr_t *transIP, int16 *transPort)
{
	rtk_rg_fwdEngineReturn_t ret;
	int r1=0,r2=0,r3=0;
	if((pPktHdr==NULL) || (transIP==NULL) || (transPort==NULL)) return RG_FWDENGINE_RET_TO_PS;

	ret = _rtk_rg_gatewayServicePort_check(pPktHdr);
	if(ret==RG_FWDENGINE_RET_TO_PS) return RG_FWDENGINE_RET_TO_PS;

	pPktHdr->naptrLookupHit=0;
	//Inbound connection table lookup, e.g. UPNP,virtual server & DMZ tables.
	if(((rg_db.systemGlobal.initParam.naptInboundConnLookupFirstCallBack!=NULL) && ((r1=rg_db.systemGlobal.initParam.naptInboundConnLookupFirstCallBack(pPktHdr,transIP,transPort))==RT_ERR_OK)) ||
		((rg_db.systemGlobal.initParam.naptInboundConnLookupSecondCallBack!=NULL) && ((r2=rg_db.systemGlobal.initParam.naptInboundConnLookupSecondCallBack(pPktHdr,transIP,transPort))==RT_ERR_OK)) ||
		((rg_db.systemGlobal.initParam.naptInboundConnLookupThirdCallBack!=NULL) && ((r3=rg_db.systemGlobal.initParam.naptInboundConnLookupThirdCallBack(pPktHdr,transIP,transPort))==RT_ERR_OK)))
	{
		if(r1==RG_RET_SUCCESS)
		{
#if	defined(CONFIG_RG_NAPT_UPNP_SUPPORT)

			pPktHdr->naptrLookupHit=1;
			if(rg_db.systemGlobal.initParam.naptInboundConnLookupFirstCallBack==_rtk_rg_fwdEngine_upnpCheck)
			{
				TRACE("UPnP entry hit!");
			}
#endif
		}
		else if(r2==RG_RET_SUCCESS)
		{
#ifdef CONFIG_RG_NAPT_VIRTUAL_SERVER_SUPPORT

			pPktHdr->naptrLookupHit=2;
			if(rg_db.systemGlobal.initParam.naptInboundConnLookupSecondCallBack==_rtk_rg_fwdEngine_virtualServerCheck)
			{
				TRACE("Server Port entry hit!");
			}
#endif

		}
		else if(r3==RG_RET_SUCCESS)
		{
#ifdef CONFIG_RG_NAPT_DMZ_SUPPORT
			pPktHdr->naptrLookupHit=3;
			if(rg_db.systemGlobal.initParam.naptInboundConnLookupThirdCallBack==_rtk_rg_fwdEngine_dmzCheck)
			{
				TRACE("DMZ hit!");
			}	
#endif
		}
		return RG_FWDENGINE_RET_CONTINUE;
	}

	return RG_FWDENGINE_RET_TO_PS;
}

void _url_parsing_string(rtk_rg_pktHdr_t *pPktHdr, char *fqdn, char *url_head, char *url_tail, char *path_head, char *path_tail)
{	
	char space = 0x20;
	char sep[3];
	int url_len = 0;
	int path_len = 0;
	
	/*use for parsing Host:*/
	sep[0]=0x0d;
	sep[1]=0x0a;
	sep[2]='\0';

	bzero(fqdn,sizeof(fqdn));

	if(pPktHdr->pL4Payload!=NULL){		
		path_head = strstr(pPktHdr->pL4Payload, "/");
	}else{
		//rtlglue_printf("pL4Payload is null\n");
	}

	if(path_head!=NULL){
		//rtlglue_printf("path_head:%s.\n",url_head);
		path_tail = strchr(path_head, space);
	}else{
		DEBUG("[URL_PARSER]path_head is null\n");
	}

	if(path_tail!=NULL){
		//rtlglue_printf("path_tail:%s.\n",url_tail);
		DEBUG("[URL_PARSER]Lookup for \"Referer:\"\n");
		url_head = strstr(path_tail, "Referer:");
		if(url_head!=NULL){
			url_head = strchr(url_head, space);//cut the "Referer:"
			if(url_head!=NULL)
				url_head = &url_head[1];//cut the " "
		}
		if(!url_head){
			DEBUG("[URL_PARSER]Lookup for \"Host:\"\n");
			url_head = strstr(path_tail, "Host:");
			if(url_head!=NULL){
				url_head = strchr(url_head, space);//cut the "Host:"
				if(url_head!=NULL)
					url_head = &url_head[1];//cut the " "
			}
		}
	}else{
		DEBUG("[URL_PARSER]path_tail is null\n");
	}

	if(url_head!=NULL){
		//rtlglue_printf("path_head:%s.\n",path_head);
		url_tail = strstr(url_head, sep);
	}else{
		DEBUG("[URL_PARSER]url_head is null\n");
	}

	if(url_tail!=NULL){
		//rtlglue_printf("url_tail:%s.\n",path_tail);
	}else{
		DEBUG("[URL_PARSER]url_tail is null\n");
	}

	if(url_head!=NULL && url_tail!=NULL && path_head!=NULL && path_tail!=NULL){
		url_len = url_tail - url_head;
		path_len = path_tail - path_head;
		if(url_len>=MAX_URL_FILTER_BUF_LENGTH)
		{
			//dump_packet(pData,len,"urlFilter Packet");
			DEBUG("[URL_PARSER]fqdn too long url_len=%d!!!\n",url_len);
			strncpy(fqdn,url_head,MAX_URL_FILTER_BUF_LENGTH);
			fqdn[MAX_URL_FILTER_BUF_LENGTH-1]='\0';
		}
		else if((url_len+path_len)>=MAX_URL_FILTER_BUF_LENGTH)
		{
			DEBUG("[URL_PARSER]fqdn too long url_len=%d  path_len=%d!!!\n",url_len,path_len);
			strncpy(fqdn,url_head,url_len);
			path_len=MAX_URL_FILTER_BUF_LENGTH-url_len;
			strncpy((fqdn+url_len),path_head,path_len);
			fqdn[MAX_URL_FILTER_BUF_LENGTH-1]='\0';
		}
		else
		{
			strncpy(fqdn,url_head,url_len);
			strncpy((fqdn+url_len),path_head,path_len);
			fqdn[url_len+path_len]='\0';
		}
	}else{
		//dump_packet(pData,len,"urlFilter Packet");
		//rtlglue_printf("fqdn: can not parse, trap!!! \n");
		fqdn=NULL;
	}
}

static char keyword_fqdn[MAX_URL_FILTER_STR_LENGTH+MAX_URL_FILTER_PATH_LENGTH];// urlfilter url_filter_string(128) + urlfilter path_filter_string(256)

rtk_rg_fwdEngineReturn_t _rtk_rg_urlFilter(u8 *pData, u32 len, rtk_rg_pktHdr_t *pPktHdr)
{

	int i,index;
	char *fqdn=rg_db.systemGlobal.urlFilter_parsingBuf;
	char *keyword; //urlfilter url_filter_string
	char *path;//urlfilter path_filter_string
	
	char *parseSuccess=NULL;
	char *url_head=NULL;
	char *url_tail=NULL;
	char *path_head=NULL;
	char *path_tail=NULL;
	int keyword_url_len = 0;
	int keyword_path_len = 0;

	rtk_rg_naptInfo_t naptInfo;
	int valid_idx;
	int ret;

	if(_rtk_rg_is_urlFilter_table_init())
		ASSERT_EQ(_rtk_rg_urlFilter_table_init(),RT_ERR_RG_OK);

	if(pPktHdr->pL4Payload==NULL){
		TRACE("[URLFILTER]urlFilter get a Empty payload packet, %s it!\n",rg_db.systemGlobal.urlFilterMode==RG_FILTER_BLACK?"DROP":"CONTINUE");
		if(rg_db.systemGlobal.urlFilterMode==RG_FILTER_BLACK)	
			return RG_FWDENGINE_RET_DROP;
		else
			return RG_FWDENGINE_RET_CONTINUE;
	}

	/*parsing FQDN*/
	if(rg_db.systemGlobal.urlFilter_totalNum)
	{
		_url_parsing_string(pPktHdr, fqdn, url_head, url_tail, path_head, path_tail);
		
		//DEBUG("[URLFILTER]fqdn:%s\n",fqdn);
		
		if(fqdn!=NULL){
			for(i=0;i<MAX_URL_FILTER_ENTRY_SIZE;i++){
				if(rg_db.systemGlobal.urlFilter_valid_entry[i]==-1){//use urlFilter_valid_entry search to promote efficiency
					break;
				}else{
					index = rg_db.systemGlobal.urlFilter_valid_entry[i]; //get the valid entry
					keyword= rg_db.systemGlobal.urlFilter_table_entry[index].urlFilter.url_filter_string;
					path = rg_db.systemGlobal.urlFilter_table_entry[index].urlFilter.path_filter_string;
					if(keyword==NULL || path==NULL){
						DEBUG("[URLFILTER]urlFilter get a NULL keyword or path!");
						continue;
					}
					//DEBUG("keyword: %s \n",keyword);
					/*compare FQDN*/
					if(rg_db.systemGlobal.urlFilter_table_entry[index].urlFilter.path_exactly_match){	
						keyword_url_len = strlen(rg_db.systemGlobal.urlFilter_table_entry[index].urlFilter.url_filter_string);
						keyword_path_len = strlen(rg_db.systemGlobal.urlFilter_table_entry[index].urlFilter.path_filter_string);
						DEBUG("[URLFILTER]keyword_url_len=%d keyword_path_len=%d\n",keyword_url_len,keyword_path_len);
						strncpy(keyword_fqdn,rg_db.systemGlobal.urlFilter_table_entry[index].urlFilter.url_filter_string,keyword_url_len);
						strncpy((keyword_fqdn+keyword_url_len),rg_db.systemGlobal.urlFilter_table_entry[index].urlFilter.path_filter_string,keyword_path_len);
						keyword_fqdn[keyword_url_len+keyword_path_len]='\0';
						
						DEBUG("[URLFILTER]keyword_fqdn:%s \n",keyword_fqdn);
						
						if(strlen(fqdn)!=strlen(keyword_fqdn)){
							DEBUG("[URLFILTER]len not the same! fqdn_len=%d  keyword_len=%d\n\n\n",strlen(fqdn),strlen(keyword_fqdn));
							//return RG_RET_SUCCESS;
						}else if(strcmp(fqdn,keyword_fqdn)){
							/*strcmp return true if fqdn & keyword not the same */
							DEBUG("[URLFILTER]strcmp not the same! keyword=%s \n\n\n",keyword);
							//return RG_RET_SUCCESS;
						}else{
							if(rg_db.systemGlobal.urlFilterMode==RG_FILTER_BLACK)
							{
								bzero(&naptInfo,sizeof(naptInfo));
								valid_idx= -1;
								naptInfo.naptTuples.is_tcp=1;
								naptInfo.naptTuples.local_ip=pPktHdr->ipv4Sip;
								naptInfo.naptTuples.local_port=pPktHdr->sport;
								naptInfo.naptTuples.remote_ip=pPktHdr->ipv4Dip;
								naptInfo.naptTuples.remote_port=pPktHdr->dport;
								ret = (pf.rtk_rg_naptConnection_find)(&naptInfo,&valid_idx);
								
								if(ret==RT_ERR_RG_OK){
									DEBUG("[URLFILTER]del napt[%d]: sip=0x%x dip=0x%x sport=%d dport=%d\n",valid_idx,pPktHdr->ipv4Sip,pPktHdr->ipv4Dip,pPktHdr->sport,pPktHdr->dport);
									assert_ok((pf.rtk_rg_naptConnection_del)(valid_idx));
								}
								
								TRACE("[URLFILTER]URL PACKET HAS BEEN DROP!\n\n\n");
								return RG_FWDENGINE_RET_DROP;
							}
							else
							{
								TRACE("[URLFILTER]URL PACKET HAS BEEN CONTINUE!\n\n\n");
								return RG_FWDENGINE_RET_CONTINUE;
							}
						}
					}else{
						//keyword only
						DEBUG("[URLFILTER]keyword: %s \n",keyword);
						parseSuccess = strstr(fqdn, keyword);
						if(parseSuccess!=NULL){
							if(rg_db.systemGlobal.urlFilterMode==RG_FILTER_BLACK)
							{
								bzero(&naptInfo,sizeof(naptInfo));
								valid_idx= -1;
								naptInfo.naptTuples.is_tcp=1;
								naptInfo.naptTuples.local_ip=pPktHdr->ipv4Sip;
								naptInfo.naptTuples.local_port=pPktHdr->sport;
								naptInfo.naptTuples.remote_ip=pPktHdr->ipv4Dip;
								naptInfo.naptTuples.remote_port=pPktHdr->dport;
								ret = (pf.rtk_rg_naptConnection_find)(&naptInfo,&valid_idx);
								
								if(ret==RT_ERR_RG_OK){
									DEBUG("[URLFILTER]del napt[%d]: sip=0x%x dip=0x%x sport=%d dport=%d\n",valid_idx,pPktHdr->ipv4Sip,pPktHdr->ipv4Dip,pPktHdr->sport,pPktHdr->dport);
									assert_ok((pf.rtk_rg_naptConnection_del)(valid_idx));
								}
								
								TRACE("[URLFILTER]URL PACKET HAS BEEN DROP!\n\n\n");
								return RG_FWDENGINE_RET_DROP;
							}
							else
							{
								TRACE("[URLFILTER]URL PACKET HAS BEEN CONTINUE!\n\n\n");
								return RG_FWDENGINE_RET_CONTINUE;
							}
						}
					}
				}
			}
		}
	}
	if(rg_db.systemGlobal.urlFilterMode==RG_FILTER_BLACK){
		TRACE("[URLFILTER]no urlFilter added and filterMode is BLACK, CONTINUE it!\n");
		return RG_FWDENGINE_RET_CONTINUE;
	}else{
		TRACE("[URLFILTER]no urlFilter added and filterMode is WHITE, DROP it!\n");
		return RG_FWDENGINE_RET_DROP;
	}
}


void _rtk_rg_fillMaxL4Ways(int naptOutIdx,int naptInIdx,int naptOutHashIdx,int naptInHashIdx)
{
	int i,inIdx,wanIntfIdx,wayIdx=0;
	if(naptOutIdx>=MAX_NAPT_OUT_HW_TABLE_SIZE)
	{
		rtk_rg_table_naptOut_linkList_t *pNaptOutList;
		wayIdx=4;
		pNaptOutList=rg_db.pNaptOutHashListHead[naptOutHashIdx];
		while(pNaptOutList!=NULL)
		{
			if(rg_db.systemGlobal.enableL4WaysList==1)
			{			
				if(wayIdx==4)
				{
					rtlglue_printf("outHashIdx=%d\n",naptOutHashIdx);
					for(i=naptOutHashIdx<<2;i<(naptOutHashIdx<<2)+4;i++)
					{
						if(rg_db.naptOut[i].rtk_naptOut.valid!=0)
						{
							inIdx=rg_db.naptOut[i].rtk_naptOut.hashIdx;
							wanIntfIdx=rg_db.nexthop[rg_db.extip[rg_db.naptIn[inIdx].rtk_naptIn.extIpIdx].rtk_extip.nhIdx].rtk_nexthop.ifIdx;
							rtlglue_printf("[O:%04d] [0x%08x:%d]-->[WANIF:%d:%d]-->[0x%08x:%d] %s\n",i,
								rg_db.naptIn[inIdx].rtk_naptIn.intIp,
								rg_db.naptIn[inIdx].rtk_naptIn.intPort,
								wanIntfIdx,
								rg_db.naptOut[i].extPort,
								rg_db.naptOut[i].remoteIp,
								rg_db.naptOut[i].remotePort,
								(rg_db.naptIn[inIdx].rtk_naptIn.isTcp)?"TCP":"UDP");
						}
						
					}
				}
				i=pNaptOutList->idx;
				if(rg_db.naptOut[i].rtk_naptOut.valid!=0)
				{
					inIdx=rg_db.naptOut[i].rtk_naptOut.hashIdx;
					wanIntfIdx=rg_db.nexthop[rg_db.extip[rg_db.naptIn[inIdx].rtk_naptIn.extIpIdx].rtk_extip.nhIdx].rtk_nexthop.ifIdx;
					rtlglue_printf("[O:%04d] [0x%08x:%d]-->[WANIF:%d:%d]-->[0x%08x:%d] %s\n",i,
						rg_db.naptIn[inIdx].rtk_naptIn.intIp,
						rg_db.naptIn[inIdx].rtk_naptIn.intPort,
						wanIntfIdx,
						rg_db.naptOut[i].extPort,
						rg_db.naptOut[i].remoteIp,
						rg_db.naptOut[i].remotePort,
						(rg_db.naptIn[inIdx].rtk_naptIn.isTcp)?"TCP":"UDP"
						);
				}				
			}
			
			pNaptOutList=pNaptOutList->pNext;
			wayIdx++;
		}	
	}
	else
	{
		wayIdx=(naptOutIdx&3)+1;
	}	
	if(wayIdx>rg_db.systemGlobal.l4OutboundMaxWays[naptOutHashIdx]) rg_db.systemGlobal.l4OutboundMaxWays[naptOutHashIdx]=wayIdx;

	if(naptInIdx>=MAX_NAPT_IN_HW_TABLE_SIZE)
	{
		rtk_rg_table_naptIn_linkList_t *pNaptInList;
		wayIdx=4;
		pNaptInList=rg_db.pNaptInHashListHead[naptInHashIdx];
		while(pNaptInList!=NULL)
		{
			if(rg_db.systemGlobal.enableL4WaysList==1)
			{
				if(wayIdx==4)
				{	
					rtlglue_printf("inHashIdx=%d\n",naptInHashIdx);
					for(i=naptInHashIdx<<2;i<(naptInHashIdx<<2)+4;i++)
					{
						if(rg_db.naptIn[i].rtk_naptIn.valid!=0)
						{
							wanIntfIdx=rg_db.nexthop[rg_db.extip[rg_db.naptIn[i].rtk_naptIn.extIpIdx].rtk_extip.nhIdx].rtk_nexthop.ifIdx;
#if defined(CONFIG_RTL9602C_SERIES)
							rtlglue_printf("[I:%04d] [RHASH:%x]-->[WANIF:%d:PORT:0x%x] %s\n",i,
								rg_db.naptIn[i].rtk_naptIn.remHash,
								wanIntfIdx,
								(rg_db.naptIn[i].rtk_naptIn.extPortHSB<<8) | rg_db.naptIn[i].rtk_naptIn.extPortLSB,								
								(rg_db.naptIn[i].rtk_naptIn.isTcp)?"TCP":"UDP");
#else
							rtlglue_printf("[I:%04d] [RHASH:%x]-->[WANIF:%d:LSB:0x%x] %s\n",i,
								rg_db.naptIn[i].rtk_naptIn.remHash,
								wanIntfIdx,
								rg_db.naptIn[i].rtk_naptIn.extPortLSB,								
								(rg_db.naptIn[i].rtk_naptIn.isTcp)?"TCP":"UDP");
#endif
						}						
					}
				}
				i=pNaptInList->idx;
				if(rg_db.naptIn[i].rtk_naptIn.valid!=0)
				{
					wanIntfIdx=rg_db.nexthop[rg_db.extip[rg_db.naptIn[i].rtk_naptIn.extIpIdx].rtk_extip.nhIdx].rtk_nexthop.ifIdx;
#if defined(CONFIG_RTL9602C_SERIES)
					rtlglue_printf("[I:%04d] [RHASH:%x]-->[WANIF:%d:PORT:0x%x] %s\n",i,
						rg_db.naptIn[i].rtk_naptIn.remHash,
						wanIntfIdx,
						(rg_db.naptIn[i].rtk_naptIn.extPortHSB<<8) | rg_db.naptIn[i].rtk_naptIn.extPortLSB,
						(rg_db.naptIn[i].rtk_naptIn.isTcp)?"TCP":"UDP");
#else
					rtlglue_printf("[I:%04d] [RHASH:%x]-->[WANIF:%d:LSB:0x%x] %s\n",i,
						rg_db.naptIn[i].rtk_naptIn.remHash,
						wanIntfIdx,
						rg_db.naptIn[i].rtk_naptIn.extPortLSB,								
						(rg_db.naptIn[i].rtk_naptIn.isTcp)?"TCP":"UDP");
#endif
				}
				
			}
		
			pNaptInList=pNaptInList->pNext;
			wayIdx++;
		}	
	}
	else
	{
#if defined(CONFIG_APOLLO)
		wayIdx=(naptInIdx&3)+1;
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
		wayIdx=4; //always set to four way
#endif
	}
	if(wayIdx>rg_db.systemGlobal.l4InboundMaxWays[naptInHashIdx]) rg_db.systemGlobal.l4InboundMaxWays[naptInHashIdx]=wayIdx;		

}


void _rtk_rg_recycleReusedSymmetricEntry(int naptOutIdx,int naptInIdx)
{
	// outbound reuse, del old inbound flow
	if(rg_db.naptOut[naptOutIdx].canBeReplaced==1)
	{
		int inIdx=rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;
		
		if(rg_db.naptIn[inIdx].rtk_naptIn.valid==ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE)
		{
			//20151202LUKE: call softwareNaptInfoDelete callback here.
			if(rg_db.systemGlobal.initParam.softwareNaptInfoDeleteCallBack != NULL){
				rtk_rg_naptInfo_t naptInfo;
#if 0//def __KERNEL__
				rtk_rg_wq_softwareNaptDelCallBack_t *softwareNaptDelWq;
				softwareNaptDelWq = kmalloc(sizeof(rtk_rg_wq_softwareNaptDelCallBack_t), GFP_ATOMIC);		
				INIT_WORK(&(softwareNaptDelWq->work), wq_do_softwareNaptDelCallBack);
#endif
				//20151202LUKE: collect info from NAPT/NAPTR table.
				_rtk_rg_naptInfoCollectForCallback(naptOutIdx, &naptInfo);
#if 0//def __KERNEL__
				memcpy(&(softwareNaptDelWq->naptInfo),&naptInfo,sizeof(rtk_rg_naptInfo_t)); 
				schedule_work(&(softwareNaptDelWq->work));
#else
				rg_db.systemGlobal.initParam.softwareNaptInfoDeleteCallBack(&naptInfo);
#endif
			}
		
			// only set software valid to zero, let next entry can be replaced.
			rg_db.naptIn[inIdx].rtk_naptIn.valid=ASIC_NAPT_IN_TYPE_INVALID;
			rg_db.naptIn[inIdx].canBeReplaced=0;
			_rtk_rg_naptExtPortFree(0,rg_db.naptIn[inIdx].rtk_naptIn.isTcp,rg_db.naptOut[naptOutIdx].extPort);
		}		
	}

	// inbound reuse, del old outbound flow.
	if(rg_db.naptIn[naptInIdx].canBeReplaced==1) //only hit when valid==ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE 
	{
		int outIdx=rg_db.naptIn[naptInIdx].symmetricNaptOutIdx;
		
		if(rg_db.naptOut[outIdx].rtk_naptOut.valid==1)
		{
			//20151202LUKE: call softwareNaptInfoDelete callback here.
			if(rg_db.systemGlobal.initParam.softwareNaptInfoDeleteCallBack != NULL){
				rtk_rg_naptInfo_t naptInfo;
#if 0//def __KERNEL__
				rtk_rg_wq_softwareNaptDelCallBack_t *softwareNaptDelWq;
				softwareNaptDelWq = kmalloc(sizeof(rtk_rg_wq_softwareNaptDelCallBack_t), GFP_ATOMIC);		
				INIT_WORK(&(softwareNaptDelWq->work), wq_do_softwareNaptDelCallBack);
#endif
				//20151202LUKE: collect info from NAPT/NAPTR table.
				_rtk_rg_naptInfoCollectForCallback(outIdx, &naptInfo);
#if 0//def __KERNEL__
				memcpy(&(softwareNaptDelWq->naptInfo),&naptInfo,sizeof(rtk_rg_naptInfo_t)); 
				schedule_work(&(softwareNaptDelWq->work));
#else
				rg_db.systemGlobal.initParam.softwareNaptInfoDeleteCallBack(&naptInfo);
#endif
			}
		
			// only set software valid to zero, let next entry can be replaced.
			rg_db.naptOut[outIdx].rtk_naptOut.valid=0;
			rg_db.naptOut[outIdx].canBeReplaced=0;
			_rtk_rg_naptExtPortFree(0,rg_db.naptIn[naptInIdx].rtk_naptIn.isTcp,rg_db.naptOut[outIdx].extPort);
		}		
	}	
}
	
#ifdef CONFIG_RG_NAPT_INBOUND_TRACKING
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_inbound_fillNaptInfo(int *outIdx, rtk_rg_pktHdr_t *pPktHdr, ipaddr_t transIP, uint16 transPort)
{
	rtk_rg_extPortGetReturn_t extPort;
	rtk_rg_entryGetReturn_t naptOutIdx,naptInIdx=0;
	int isTCP=0;
	int hashOutIdx,hashInIdx;

	if(pPktHdr->tagif&TCP_TAGIF) isTCP=1;
	//found a free naptOut entry			
	naptOutIdx=_rtk_rg_naptTcpUdpOutFreeEntryGet(pPktHdr->addNaptSwOnly,pPktHdr->algAction,isTCP,transIP,transPort,pPktHdr->ipv4Sip,pPktHdr->sport,&hashOutIdx);
	assert(naptOutIdx!=RG_RET_ENTRY_NOT_GET);
	if(naptOutIdx==RG_RET_ENTRY_NOT_GET) return RG_RET_FAIL;

	//found a free ext Port.
#if defined(CONFIG_APOLLO)
	extPort=_rtk_rg_naptExtPortGetAndUse(TRUE,isTCP,ntohs(*pPktHdr->pDport),pPktHdr->pIpv4Dip,TRUE);
#elif defined(CONFIG_XDSL_ROMEDRIVER)
	extPort=_rtk_rg_naptExtPortGetAndUse(TRUE,isTCP,(pPktHdr->ipv4Sip),(pPktHdr->sport),ntohs(*pPktHdr->pDport),pPktHdr->pIpv4Dip,TRUE);
#endif
	assert(extPort!=RG_RET_EXTPORT_NOT_GET);
	if(extPort==RG_RET_EXTPORT_NOT_GET) return RG_RET_FAIL;
	DEBUG("Found external port:%d\n",extPort);
	
#if defined(CONFIG_APOLLO) 
	//found a free naptIn entry
	naptInIdx=_rtk_rg_naptTcpUdpInFreeEntryGet(pPktHdr->addNaptSwOnly,pPktHdr->algAction,isTCP,rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,extPort,&hashInIdx);
	assert(naptInIdx!=RG_RET_ENTRY_NOT_GET);
	if(naptInIdx==RG_RET_ENTRY_NOT_GET) return RG_RET_FAIL;
#elif defined(CONFIG_XDSL_ROMEDRIVER)
	//temp set inbound and outbound to vaild for napt inbound search
	if(naptOutIdx<MAX_NAPT_OUT_HW_TABLE_SIZE){
		rg_db.naptOut[naptOutIdx].rtk_naptOut.valid=1;
		rg_db.naptIn[naptOutIdx].rtk_naptIn.valid=ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE;	
	}
	//found a free naptIn entry
	naptInIdx=_rtk_rg_naptTcpUdpInFreeEntryGet(pPktHdr->addNaptSwOnly,pPktHdr->algAction,isTCP,pPktHdr->ipv4Sip,pPktHdr->sport,rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,extPort,&hashInIdx);
	assert(naptInIdx!=RG_RET_ENTRY_NOT_GET);
	if(naptInIdx==RG_RET_ENTRY_NOT_GET){ 
		if(naptOutIdx<MAX_NAPT_OUT_HW_TABLE_SIZE){
			rg_db.naptOut[naptOutIdx].rtk_naptOut.valid=0;
			rg_db.naptIn[naptOutIdx].rtk_naptIn.valid=0;	
		}else{
			//FIXME:need free sw outentry in hash table 
		}
			_rtk_rg_naptExtPortFree(0,isTCP,extPort);
			return RG_RET_FAIL;
	}else{
		 //find naptIn entry sync to naptOut
		if(naptInIdx<MAX_NAPT_OUT_HW_TABLE_SIZE){
			rg_db.naptOut[naptInIdx].rtk_naptOut.valid=1;
		}
	}
#endif
	//_rtk_rg_arpAndMacEntryAdd(pPktHdr->ipv4Sip,pPktHdr->sipL3Idx,pPktHdr->pSmac,pPktHdr->pRxDesc->opts3.bit.src_port_num,&sipArpIdx,0);
	if(rg_db.systemGlobal.tcpDoNotDelWhenRstFin==1) _rtk_rg_recycleReusedSymmetricEntry(naptOutIdx,naptInIdx);

	rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx=naptInIdx;
	rg_db.naptOut[naptOutIdx].rtk_naptOut.valid=1;
	rg_db.naptOut[naptOutIdx].rtk_naptOut.priValid=0;
	rg_db.naptOut[naptOutIdx].rtk_naptOut.priValue=0;
	rg_db.naptOut[naptOutIdx].extPort=extPort;
	rg_db.naptOut[naptOutIdx].state=INVALID;
	rg_db.naptOut[naptOutIdx].canBeReplaced=0;
	rg_db.naptOut[naptOutIdx].cannotAddToHw=0;

	rg_db.naptIn[naptInIdx].rtk_naptIn.extIpIdx=pPktHdr->extipIdx;
#if defined(CONFIG_RTL9602C_SERIES)
	rg_db.naptIn[naptInIdx].rtk_naptIn.extPortHSB=(extPort>>8)&0xff;
#endif
	rg_db.naptIn[naptInIdx].rtk_naptIn.extPortLSB=extPort&0xff;
	rg_db.naptIn[naptInIdx].rtk_naptIn.intIp=transIP;
	rg_db.naptIn[naptInIdx].rtk_naptIn.intPort=transPort;
	rg_db.naptIn[naptInIdx].rtk_naptIn.isTcp=isTCP;
	rg_db.naptIn[naptInIdx].rtk_naptIn.priId=0;
	rg_db.naptIn[naptInIdx].rtk_naptIn.priValid=0;
#if defined(CONFIG_APOLLO)
	rg_db.naptIn[naptInIdx].rtk_naptIn.remHash=_rtk_rg_NAPTRemoteHash_get(pPktHdr->ipv4Sip,pPktHdr->sport);
#elif defined(CONFIG_XDSL_ROMEDRIVER)
	rg_db.naptIn[naptInIdx].rtk_naptIn.remHash=_rtk_rg_NAPTRemoteHash_get(isTCP,pPktHdr->ipv4Sip,pPktHdr->sport);
	rg_db.naptIn[naptInIdx].rtk_naptIn.extPort =extPort;
#endif
	rg_db.naptIn[naptInIdx].canBeReplaced=0;
	rg_db.naptIn[naptInIdx].cannotAddToHw=0;
	
#if 0	
	rg_db.naptIn[naptInIdx].remoteIp=pPktHdr->ipv4Sip;
	rg_db.naptIn[naptInIdx].remotePort=pPktHdr->sport;
#else
	rg_db.naptOut[naptOutIdx].remoteIp=pPktHdr->ipv4Sip;
	rg_db.naptOut[naptOutIdx].remotePort=pPktHdr->sport;	

#if	defined(CONFIG_RG_NAPT_VIRTUAL_SERVER_SUPPORT)
	if((pPktHdr->naptrLookupHit==2)&&(rg_db.systemGlobal.initParam.naptInboundConnLookupSecondCallBack==_rtk_rg_fwdEngine_virtualServerCheck))
	{
		rg_db.naptIn[naptInIdx].rtk_naptIn.valid=ASIC_NAPT_IN_TYPE_FULL_CONE;	
		rg_db.naptIn[naptInIdx].coneType=NAPT_IN_TYPE_FULL_CONE;
		rg_db.naptIn[naptInIdx].symmetricNaptOutIdx=0; //non-used field

	}
	else
#endif
	
	{
		rg_db.naptIn[naptInIdx].rtk_naptIn.valid=ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE;	
		rg_db.naptIn[naptInIdx].coneType=NAPT_IN_TYPE_SYMMETRIC_NAPT;
#if defined(NAPT_TABLE_SIZE_DEBUG)
        assert(naptOutIdx < (1 << MAX_NAPT_IN_NAPTOUTIDX_WIDTH));
#endif
		rg_db.naptIn[naptInIdx].symmetricNaptOutIdx=naptOutIdx;
	}
#endif	
	rg_db.naptIn[naptInIdx].refCount=1;
	if(rg_db.systemGlobal.enableL4MaxWays==1) _rtk_rg_fillMaxL4Ways(naptOutIdx,naptInIdx,hashOutIdx,hashInIdx);


	*outIdx=naptOutIdx;

	TABLE("Set NAPT in[%d] out[%d] Entry",naptInIdx,naptOutIdx);
	//Record valid NAPT entry
	rg_db.naptValidSet[naptOutIdx>>5] |= (0x1<<(naptOutIdx&31));

	//20151126LUKE: call softwareNaptInfoAdd callback here.
	if(rg_db.systemGlobal.initParam.softwareNaptInfoAddCallBack != NULL){
		rtk_rg_naptInfo_t naptInfo;
#if 0//def __KERNEL__
		rtk_rg_wq_softwareNaptAddCallBack_t *softwareNaptAddWq;
		softwareNaptAddWq = kmalloc(sizeof(rtk_rg_wq_softwareNaptAddCallBack_t), GFP_ATOMIC);		
		INIT_WORK(&(softwareNaptAddWq->work), wq_do_softwareNaptAddCallBack);
#endif
		//20151202LUKE: collect info from NAPT/NAPTR table.
		_rtk_rg_naptInfoCollectForCallback(naptOutIdx, &naptInfo);
#if 0//def __KERNEL__
		memcpy(&(softwareNaptAddWq->naptInfo),&naptInfo,sizeof(rtk_rg_naptInfo_t)); 
		schedule_work(&(softwareNaptAddWq->work));
#else
		rg_db.systemGlobal.initParam.softwareNaptInfoAddCallBack(&naptInfo);
#endif
	}

	return RG_RET_SUCCESS;
}
#endif

/* 
input:pPktHdr 
output:outIdx
find inIdx/outIdx and fill software table  rg_db.naptOut/rg_db.naptIn return *outIdx=outIdx
*/
__SRAM_FWDENG_SLOWPATH
rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_outbound_fillNaptInfo(int *outIdx, rtk_rg_pktHdr_t *pPktHdr)
{		
	rtk_rg_extPortGetReturn_t extPort;
	rtk_rg_entryGetReturn_t naptOutIdx,naptInIdx;
//	int sipArpIdx;
	int isTCP=0;
	int hashOutIdx,hashInIdx;
	
	if(pPktHdr->tagif&TCP_TAGIF) isTCP=1;

	//20151008LUKE: check for Extip valid or not, if invalid, return RG_RET_FAIL
	if(pPktHdr->extipIdx<0 || !rg_db.extip[pPktHdr->extipIdx].rtk_extip.valid) return RG_RET_FAIL;
	
	//found a free naptOut entry				
	naptOutIdx=_rtk_rg_naptTcpUdpOutFreeEntryGet(pPktHdr->addNaptSwOnly,pPktHdr->algAction,isTCP,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->ipv4Dip,pPktHdr->dport,&hashOutIdx);
	assert(naptOutIdx!=RG_RET_ENTRY_NOT_GET);
	if(naptOutIdx==RG_RET_ENTRY_NOT_GET) return RG_RET_FAIL;

	//found a free ext Port.
	if(pPktHdr->algAction==RG_ALG_ACT_TO_FWDENGINE)
	{	
#if defined(CONFIG_APOLLO)
		extPort=_rtk_rg_naptExtPortGetAndUse(pPktHdr->algKeepExtPort,isTCP,pPktHdr->sport,&rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,TRUE);
#elif defined(CONFIG_XDSL_ROMEDRIVER)
		extPort=_rtk_rg_naptExtPortGetAndUse(FALSE,isTCP,pPktHdr->ipv4Dip,pPktHdr->dport,pPktHdr->sport,&rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,TRUE);
#endif
		assert(extPort!=RG_RET_EXTPORT_NOT_GET);
		if(extPort==RG_RET_EXTPORT_NOT_GET) return RG_RET_FAIL;	
	}
	else
	{
#if defined(CONFIG_APOLLO)
		extPort=_rtk_rg_naptExtPortGetAndUse(FALSE,isTCP,pPktHdr->sport,&rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,TRUE);
#elif defined(CONFIG_XDSL_ROMEDRIVER)
		extPort=_rtk_rg_naptExtPortGetAndUse(FALSE,isTCP,pPktHdr->ipv4Dip,pPktHdr->dport,pPktHdr->sport,&rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,TRUE);
#endif
		assert(extPort!=RG_RET_EXTPORT_NOT_GET);
		if(extPort==RG_RET_EXTPORT_NOT_GET) return RG_RET_FAIL;	
	}
	

#if defined(CONFIG_APOLLO) 
	//found a free naptIn entry
	naptInIdx=_rtk_rg_naptTcpUdpInFreeEntryGet(pPktHdr->addNaptSwOnly,pPktHdr->algAction,isTCP,rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,extPort,&hashInIdx);
	assert(naptInIdx!=RG_RET_ENTRY_NOT_GET);
	if(naptInIdx==RG_RET_ENTRY_NOT_GET) return RG_RET_FAIL;

#elif defined(CONFIG_XDSL_ROMEDRIVER)
	if(naptOutIdx<MAX_NAPT_OUT_HW_TABLE_SIZE){
		rg_db.naptOut[naptOutIdx].rtk_naptOut.valid=1;
		rg_db.naptIn[naptOutIdx].rtk_naptIn.valid=ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE;
	}
	//found a free naptIn entry
	naptInIdx=_rtk_rg_naptTcpUdpInFreeEntryGet(pPktHdr->addNaptSwOnly,pPktHdr->algAction,isTCP,pPktHdr->ipv4Dip,pPktHdr->dport,rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,extPort,&hashInIdx);
	assert(naptInIdx!=RG_RET_ENTRY_NOT_GET);
	if(naptInIdx==RG_RET_ENTRY_NOT_GET){
		//free extport and free napt outbound entry
		if(naptOutIdx<MAX_NAPT_OUT_HW_TABLE_SIZE){
			rg_db.naptOut[naptOutIdx].rtk_naptOut.valid=0;
			rg_db.naptIn[naptOutIdx].rtk_naptIn.valid=0;
		}else{
			//FIXME:need free sw outentry in hash table	
		}
		_rtk_rg_naptExtPortFree(0,isTCP,extPort);
		return RG_RET_FAIL;
	}else{
		if(naptInIdx<MAX_NAPT_OUT_HW_TABLE_SIZE){
			rg_db.naptOut[naptInIdx].rtk_naptOut.valid=1;
		}
	}
#endif
	//_rtk_rg_arpAndMacEntryAdd(pPktHdr->ipv4Sip,pPktHdr->sipL3Idx,pPktHdr->pSmac,pPktHdr->pRxDesc->opts3.bit.src_port_num,&sipArpIdx,0,0,0);
	if(rg_db.systemGlobal.tcpDoNotDelWhenRstFin==1) _rtk_rg_recycleReusedSymmetricEntry(naptOutIdx,naptInIdx);
	
	rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx=naptInIdx;
	rg_db.naptOut[naptOutIdx].rtk_naptOut.valid=1;
	rg_db.naptOut[naptOutIdx].rtk_naptOut.priValid=0;
	rg_db.naptOut[naptOutIdx].rtk_naptOut.priValue=0;
	rg_db.naptOut[naptOutIdx].extPort=extPort;
	rg_db.naptOut[naptOutIdx].state=INVALID;
	rg_db.naptOut[naptOutIdx].canBeReplaced=0;
	rg_db.naptOut[naptOutIdx].cannotAddToHw=0;
	
	rg_db.naptIn[naptInIdx].rtk_naptIn.extIpIdx=pPktHdr->extipIdx;
#if defined(CONFIG_RTL9602C_SERIES)
	rg_db.naptIn[naptInIdx].rtk_naptIn.extPortHSB=(extPort>>8)&0xff;
#endif
	rg_db.naptIn[naptInIdx].rtk_naptIn.extPortLSB=extPort&0xff;
	rg_db.naptIn[naptInIdx].rtk_naptIn.intIp=pPktHdr->ipv4Sip;
	rg_db.naptIn[naptInIdx].rtk_naptIn.intPort=pPktHdr->sport;
	rg_db.naptIn[naptInIdx].rtk_naptIn.isTcp=isTCP;
	rg_db.naptIn[naptInIdx].rtk_naptIn.priId=0;
	rg_db.naptIn[naptInIdx].rtk_naptIn.priValid=0;
#if defined(CONFIG_APOLLO)
	rg_db.naptIn[naptInIdx].rtk_naptIn.remHash=_rtk_rg_NAPTRemoteHash_get(pPktHdr->ipv4Dip,pPktHdr->dport);
#elif defined(CONFIG_XDSL_ROMEDRIVER)
	rg_db.naptIn[naptInIdx].rtk_naptIn.remHash=_rtk_rg_NAPTRemoteHash_get(isTCP,pPktHdr->ipv4Dip,pPktHdr->dport);
	rg_db.naptIn[naptInIdx].rtk_naptIn.extPort =extPort;	//865x need
#endif	
	rg_db.naptIn[naptInIdx].canBeReplaced=0;
	rg_db.naptIn[naptInIdx].cannotAddToHw=0;
	rg_db.naptIn[naptInIdx].rtk_naptIn.valid=ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE;
	
	
#if 0	
	rg_db.naptIn[naptInIdx].remoteIp=pPktHdr->ipv4Dip;
	rg_db.naptIn[naptInIdx].remotePort=pPktHdr->dport;
#else
	rg_db.naptOut[naptOutIdx].remoteIp=pPktHdr->ipv4Dip;
	rg_db.naptOut[naptOutIdx].remotePort=pPktHdr->dport;
	rg_db.naptIn[naptInIdx].coneType=NAPT_IN_TYPE_SYMMETRIC_NAPT;
#if defined(NAPT_TABLE_SIZE_DEBUG)    
    assert(naptOutIdx < (1 << MAX_NAPT_IN_NAPTOUTIDX_WIDTH));
#endif
	rg_db.naptIn[naptInIdx].symmetricNaptOutIdx=naptOutIdx;	
#endif
	rg_db.naptIn[naptInIdx].refCount = 1;
	if(rg_db.systemGlobal.enableL4MaxWays==1) _rtk_rg_fillMaxL4Ways(naptOutIdx,naptInIdx,hashOutIdx,hashInIdx);

	//TABLE("Add napt ==> %x:%d<-->%x:%d<-->%x:%d\n",rg_db.naptIn[naptInIdx].rtk_naptIn.intIp,rg_db.naptIn[naptInIdx].rtk_naptIn.intPort,rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr,extPort,rg_db.naptOut[naptOutIdx].remoteIp,rg_db.naptOut[naptOutIdx].remotePort);

	*outIdx=naptOutIdx;

	TABLE("Set NAPT in[%d] out[%d] entry",naptInIdx,naptOutIdx);
	//Record valid NAPT entry
	rg_db.naptValidSet[naptOutIdx>>5] |= (0x1<<(naptOutIdx&31));

	//20151126LUKE: call softwareNaptInfoAdd callback here.
	if(rg_db.systemGlobal.initParam.softwareNaptInfoAddCallBack != NULL){
		rtk_rg_naptInfo_t naptInfo;
#if 0//def __KERNEL__
		rtk_rg_wq_softwareNaptAddCallBack_t *softwareNaptAddWq;
		softwareNaptAddWq = kmalloc(sizeof(rtk_rg_wq_softwareNaptAddCallBack_t), GFP_ATOMIC);		
		INIT_WORK(&(softwareNaptAddWq->work), wq_do_softwareNaptAddCallBack);
#endif
		//20151202LUKE: collect info from NAPT/NAPTR table.
		_rtk_rg_naptInfoCollectForCallback(naptOutIdx, &naptInfo);
#if 0//def __KERNEL__
		memcpy(&(softwareNaptAddWq->naptInfo),&naptInfo,sizeof(rtk_rg_naptInfo_t)); 
		schedule_work(&(softwareNaptAddWq->work));
#else
		rg_db.systemGlobal.initParam.softwareNaptInfoAddCallBack(&naptInfo);
#endif
	}

	return RG_RET_SUCCESS;
}	

void _rtk_rg_addPPPoETag(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify)
{
	int i;
	u16 len;					
	u16 sid;
	//int total_len;
#ifdef CONFIG_APOLLO_MODEL					
#else
	//add pppoe here
	if((u32)skb->data-8<(u32)skb->head)
	{
		FIXME("no more free skb buff in header room");
	}
	else					
#endif											
	{
		len = pPktHdr->l3Len + 2;
#if 0	
		if(pPktHdr->tagif&IPV6_TAGIF)
		{
			total_len=pPktHdr->l3Offset+pPktHdr->l3Len-2+pPktHdr->ipv6PayloadLen;	// 2 bytes: ether type
			//DEBUG("l3offset is %d, l3len is %d, payload is %d = total_len is %d",
				//pPktHdr->l3Offset,pPktHdr->l3Len,pPktHdr->ipv6PayloadLen,total_len);
			//FIXME:if we have QinQ, we should fix for double tag!!
			if(pPktHdr->tagif&CVLAN_TAGIF)
				len=total_len-14+2-4; // 2 bytes: ppp, 4 bytes: 1Q tag
			else
				len=total_len-14+2;	// 2 bytes: ppp
			//DEBUG("ipv6 new total_len is %d, len is %d",total_len,len);
			//sid=rg_db.pppoe[pPktHdr->netifIdx].rtk_pppoe.sessionID;
		}
		else
		{
			//DEBUG("the l3offset is %d, l3len is %d",pPktHdr->l3Offset,pPktHdr->l3Len);
			total_len=pPktHdr->l3Offset+pPktHdr->l3Len;
			//FIXME:if we have QinQ, we should fix for double tag!!
			//20140507LUKE:fix gmac padding when packet smaller than 60bytes will cause pppoe length wrong
			if(total_len<60)total_len=60;
			len = pPktHdr->l3Len + 2;
#if 0
			if(pPktHdr->tagif&CVLAN_TAGIF)
				len=total_len-14+2-4; // 2 bytes: ppp, 4 bytes: 1Q tag
			else
				len=total_len-14+2; // 2 bytes: ppp
				
#endif
			//DEBUG("new total len is %d, len is %d",total_len,len);
			//sid=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_info.after_dial.sessionId;
		}
#endif
		rg_db.pppoe[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_idx].idleSecs=0; //update pppoe idleTime
		
		sid=rg_db.pppoe[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_idx].rtk_pppoe.sessionID;
		//DEBUG("SID is %d",sid);
	
		//DEBUG("before add pppoe header l3off=%d l4off=%d len=%d, total_len=%d",pPktHdr->l3Offset,pPktHdr->l4Offset,skb->len,total_len);

		skb->data-=8;
		//skb->len=total_len+8;
		skb->len+=8;

		//DA,SA
		for(i=0;i<pPktHdr->l3Offset-2;i++)
			skb->data[i]=skb->data[i+8];
		
		//reset DMAC and SMAC pointer
		pPktHdr->pDmac=&skb->data[0];
		pPktHdr->pSmac=&skb->data[6];

		//8864
		skb->data[pPktHdr->l3Offset-2]=0x88;
		skb->data[pPktHdr->l3Offset-1]=0x64;

		//Code
		skb->data[pPktHdr->l3Offset]=0x11;
		skb->data[pPktHdr->l3Offset+1]=0;
		
		//session id
		skb->data[pPktHdr->l3Offset+2]=sid>>8;
		skb->data[pPktHdr->l3Offset+3]=sid&0xff;

		//len
		skb->data[pPktHdr->l3Offset+4]=len>>8;
		skb->data[pPktHdr->l3Offset+5]=len&0xff;
		pPktHdr->pPppoeLength=(u16*)&skb->data[pPktHdr->l3Offset+4];
		
		//ppp = IPv4:0x0021, IPv6:0x0057
		if(pPktHdr->tagif&IPV6_TAGIF)
		{
			skb->data[pPktHdr->l3Offset+6]=0x00;
			skb->data[pPktHdr->l3Offset+7]=0x57;
		}
		else
		{
			skb->data[pPktHdr->l3Offset+6]=0x00;
			skb->data[pPktHdr->l3Offset+7]=0x21;
		}

		//reset tagif and L3/4 offset
		pPktHdr->tagif|=PPPOE_TAGIF;
		if(pPktHdr->l4Offset>0)
			pPktHdr->l4Offset+=8;
		if(pPktHdr->l3Offset>0)
			pPktHdr->l3Offset+=8;
		//DEBUG("add pppoe header l3off=%d l4off=%d len=%d",pPktHdr->l3Offset,pPktHdr->l4Offset,skb->len);
		//dump_packet(skb->data,skb->len,"pppoe_add_after");
		pPktHdr->egressTagif|=PPPOE_TAGIF;
	}
}

void _rtk_rg_removePPPoETag(rtk_rg_pktHdr_t *pPktHdr)
{
	//remove pppoe header	
	int i;
	int total_len;
	struct sk_buff *skb;

	//update pppoe table
	for(i=0;i<MAX_PPPOE_SW_TABLE_SIZE;i++)
	{
		if(rg_db.pppoe[i].rtk_pppoe.sessionID==pPktHdr->sessionId)
		{
			rg_db.pppoe[i].idleSecs=0; //update pppoe idleTime
			break;
		}
	}
	//if we are PPTP or L2TP or DSLITE, we can remove pppoe tag with PPTP or L2TP or DSLITE at one time.
	if(pPktHdr->tagif&PPTP_INNER_TAGIF||pPktHdr->tagif&L2TP_INNER_TAGIF||pPktHdr->tagif&DSLITE_INNER_TAGIF)
		return;

	skb=pPktHdr->skb;

	if(pPktHdr->tagif&IPV6_TAGIF)
	{
		total_len=pPktHdr->l3Offset+pPktHdr->l3Len;
		skb->data[pPktHdr->l3Offset-2]=0x86;
		skb->data[pPktHdr->l3Offset-1]=0xdd;
	}
	else
	{
		total_len=pPktHdr->l3Offset+pPktHdr->l3Len;			
		skb->data[pPktHdr->l3Offset-2]=0x08;
		skb->data[pPktHdr->l3Offset-1]=0x00;
	}

	//DEBUG("%s...total len is %d",pPktHdr->tagif&IPV6_TAGIF?"IPV6":"IPv4",total_len);
	//remove pppoe header here
	//DEBUG("tot_len=%d l3off=%d l3len=%d l4off=%d",total_len,pPktHdr->l3Offset,pPktHdr->l3Len,pPktHdr->l4Offset);
	//dump_packet(skb->data,skb->len,"pppoe_del_before");	

	//copy new DA/SA
	for(i=pPktHdr->l3Offset-3;i>=8;i--)
		skb->data[i]=skb->data[i-8];			
	//use memcpy to accelerate this copy process
	//memcpy(skb->data+16,skb->data+8,pPktHdr->l3Offset-18);
	//memcpy(skb->data+8,skb->data,8);
	
	//FIXME for vlan tag
	skb->data+=8;
	skb->len=total_len-8;			

	//dump_packet(skb->data,skb->len,"pppoe_del_after");	

	//reset DMAC and SMAC pointer
	pPktHdr->pDmac=&skb->data[0];
	pPktHdr->pSmac=&skb->data[6];

	//reset tagif and L3/4 offset
	pPktHdr->tagif&=(~PPPOE_TAGIF);

	// Hairpin NAT must clear egressTagIf for turning on L34 hw checksum offload.
	pPktHdr->egressTagif&=(~PPPOE_TAGIF);
	
	if(pPktHdr->l4Offset>0)
		pPktHdr->l4Offset-=8;
	if(pPktHdr->l3Offset>0)
		pPktHdr->l3Offset-=8;


	//DEBUG("remove pppoe header skb->len=%d l4_len=%d",skb->len,total_len-pPktHdr->l4Offset);
	//dump_packet(skb->data,skb->len,"pppoe_del");				
}

rtk_rg_fwdEngineReturn_t _rtk_rg_lookupBasedWANForSourceIP(int *pNetIfIdx,ipaddr_t gateway_ipv4_addr)
{
	int l3Idx;

	l3Idx=_rtk_rg_l3lookup(gateway_ipv4_addr);

	if(rg_db.l3[l3Idx].rtk_l3.process==L34_PROCESS_NH && rg_db.nexthop[rg_db.l3[l3Idx].rtk_l3.nhStart].rtk_nexthop.ifIdx!=*pNetIfIdx)
	{
		*pNetIfIdx=rg_db.nexthop[rg_db.l3[l3Idx].rtk_l3.nhStart].rtk_nexthop.ifIdx;
	}
	else if(rg_db.l3[l3Idx].rtk_l3.process==L34_PROCESS_ARP && rg_db.l3[l3Idx].rtk_l3.netifIdx!=*pNetIfIdx)
	{
		*pNetIfIdx=rg_db.l3[l3Idx].rtk_l3.netifIdx;
	}
	else if(rg_db.l3[l3Idx].rtk_l3.process==L34_PROCESS_CPU)
	{
		if(rg_db.systemGlobal.interfaceInfo[rg_db.l3[l3Idx].rtk_l3.netifIdx].valid &&
			rg_db.systemGlobal.interfaceInfo[rg_db.l3[l3Idx].rtk_l3.netifIdx].storedInfo.is_wan &&
			(rg_db.systemGlobal.interfaceInfo[rg_db.l3[l3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPTP ||
			rg_db.systemGlobal.interfaceInfo[rg_db.l3[l3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_L2TP ||
			rg_db.systemGlobal.interfaceInfo[rg_db.l3[l3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_DSLITE ||
			rg_db.systemGlobal.interfaceInfo[rg_db.l3[l3Idx].rtk_l3.netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)&&
			rg_db.l3[l3Idx].rtk_l3.netifIdx!=*pNetIfIdx)
		{
			*pNetIfIdx=rg_db.nexthop[rg_db.l3[l3Idx].rtk_l3.nhStart].rtk_nexthop.ifIdx;
		}
		else if(rg_db.l3[l3Idx].rtk_l3.ipAddr > 0 && rg_db.l3[l3Idx].rtk_l3.netifIdx!=*pNetIfIdx)
		{
			*pNetIfIdx=rg_db.l3[l3Idx].rtk_l3.netifIdx;
		}
		else
			return RG_FWDENGINE_RET_ERROR;
	}
	else
		return RG_FWDENGINE_RET_ERROR;

	return RG_FWDENGINE_RET_CONTINUE;
}

rtk_rg_err_code_t _rtk_rg_addTunnelTag(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, rtk_rg_wan_type_t wan_type)
{	
	int sid=0,len,PPPoEHDR_Off=0,TUNN_tag_len=40;	//IP(20)+GRE(16)+PPP(4) for PPTP, IP(20)+UDP(8)+L2TP(8)+PPP(4) for L2TP
	unsigned short middle_header_len;
	unsigned char *pData;
	rtk_rg_ipPPTPClientInfo_t *pPPTPInfo;
	rtk_rg_ipL2TPClientInfo_t *pL2TPInfo;
	rtk_rg_wanIntfInfo_t *wan_intf;
	unsigned short *pIpv4_header_identifier;
	ipaddr_t *pIpv4_addr,*pGateway_ipv4_addr;
	unsigned int IP_flag_fragment_TTL_protocol;
	
	if(skb_cow_head(skb, TUNN_tag_len)<0)
	{
		WARNING("skb head room is not enough..return without insert %s tag",wan_type==RTK_RG_PPTP?"PPTP":"L2TP");
	}
	else
	{
		if(wan_type==RTK_RG_PPTP)
		{
			TRACE("add PPTP tag");
			pPktHdr->egressTagif|=PPTP_TAGIF;
			pPPTPInfo=&rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pptp_info;
			wan_intf=&rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf;
			middle_header_len=pPktHdr->l3Len+4;	//PPP(4)
			if(wan_intf->baseIntf_idx>=0)
				pPktHdr->netifIdx=wan_intf->baseIntf_idx;
			else{
				//20150916LUKE: if we can't decide which base WAN to use, drop the packet!
				if(_rtk_rg_lookupBasedWANForSourceIP(&pPktHdr->netifIdx, pPPTPInfo->before_dial.pptp_ipv4_addr)==RG_FWDENGINE_RET_ERROR){
					TRACE("based WAN do not exist! drop...");
					return RT_ERR_RG_ENTRY_NOT_EXIST;
				}
				wan_intf->baseIntf_idx=pPktHdr->netifIdx;
			}	
			pPktHdr->extipIdx=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.extip_idx;
			pIpv4_addr=&pPktHdr->ipv4Sip;
			if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan)
			{
				if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].p_wanStaticInfo->napt_enable)
					pIpv4_addr=&rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr;
				//if base-WAN if PPPoE, we add PPPoE tag, too.
				if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)
				{
					PPPoEHDR_Off=8;
					sid=rg_db.pppoe[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_idx].rtk_pppoe.sessionID;
				}
			}
			pGateway_ipv4_addr=&pPPTPInfo->before_dial.pptp_ipv4_addr;
			pIpv4_header_identifier=&pPPTPInfo->ipv4_header_identifier;
			IP_flag_fragment_TTL_protocol=0x0000402f;	//protocol = GRE

			pData=skb_push(skb, TUNN_tag_len+PPPoEHDR_Off);
			//copy the mac addresses to the beginning of the new header.
			//since we push the length longer than 2 MACs address, we use memcpy to replace memmove for speed.
			memcpy(skb->data, skb->data + TUNN_tag_len + PPPoEHDR_Off, VLAN_ETH_ALEN<<1);
			skb->mac_header -= (TUNN_tag_len+PPPoEHDR_Off);
			//reset DMAC and SMAC pointer
			pPktHdr->pDmac=pData;
			pPktHdr->pSmac=pData+VLAN_ETH_ALEN;
			pData+=PPPoEHDR_Off;
			
			//insert GRE header
			*(uint32 *)(pData + 34) = htonl(0x3081880b);								//Flags, version, Protocol Type=PPP
			*(uint16 *)(pData + 38) = htons(middle_header_len);							//Payload length
			*(uint16 *)(pData + 40) = htons(pPPTPInfo->after_dial.gateway_callId);		//Peer CallID
			*(uint32 *)(pData + 42) = htonl(++pPPTPInfo->gre_header_sequence);			//Sequence Num
			*(uint32 *)(pData + 46) = htonl(pPPTPInfo->gre_header_acknowledgment);		//Acknowledgment Num
		}
		else	//L2TP
		{
			TRACE("add L2TP tag");
			pPktHdr->egressTagif|=L2TP_TAGIF;
			pL2TPInfo=&rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.l2tp_info;
			wan_intf=&rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf;
			middle_header_len=pPktHdr->l3Len+20;	//UDP(8)+L2TP(8)+PPP(4)
			if(wan_intf->baseIntf_idx>=0)
				pPktHdr->netifIdx=wan_intf->baseIntf_idx;
			else{
				//20150916LUKE: if we can't decide which base WAN to use, drop the packet!
				if(_rtk_rg_lookupBasedWANForSourceIP(&pPktHdr->netifIdx, pL2TPInfo->before_dial.l2tp_ipv4_addr)==RG_FWDENGINE_RET_ERROR){
					TRACE("based WAN do not exist! drop...");
					return RT_ERR_RG_ENTRY_NOT_EXIST;
				}
				wan_intf->baseIntf_idx=pPktHdr->netifIdx;
			}	
			pPktHdr->extipIdx=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.extip_idx;
			pIpv4_addr=&pPktHdr->ipv4Sip;
			if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan)
			{
				if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].p_wanStaticInfo->napt_enable)
					pIpv4_addr=&rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr;
				//if base-WAN if PPPoE, we add PPPoE tag, too.
				if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)
				{
					PPPoEHDR_Off=8;
					sid=rg_db.pppoe[rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_idx].rtk_pppoe.sessionID;
				}
			}
			pGateway_ipv4_addr=&pL2TPInfo->before_dial.l2tp_ipv4_addr;
			pIpv4_header_identifier=&pL2TPInfo->ipv4_header_identifier;
			IP_flag_fragment_TTL_protocol=0x00004011;	//protocol = UDP

			pData=skb_push(skb, TUNN_tag_len+PPPoEHDR_Off);
			//copy the mac addresses to the beginning of the new header.
			//since we push the length longer than 2 MACs address, we use memcpy to replace memmove for speed.
			memcpy(skb->data, skb->data + TUNN_tag_len + PPPoEHDR_Off, VLAN_ETH_ALEN<<1);
			skb->mac_header -= (TUNN_tag_len+PPPoEHDR_Off);
			//reset DMAC and SMAC pointer
			pPktHdr->pDmac=pData;
			pPktHdr->pSmac=pData+VLAN_ETH_ALEN;
			pData+=PPPoEHDR_Off;

			//insert UDP header
			*(uint16 *)(pData + 34) = htons(pL2TPInfo->after_dial.outer_port);				//Source port
			*(uint16 *)(pData + 36) = htons(pL2TPInfo->after_dial.gateway_outer_port);		//Destination port
			*(uint16 *)(pData + 38) = htons(middle_header_len);								//Total length
			*(uint16 *)(pData + 40) = htons(0x0);											//Header checksum(caculated by gmac hw)

			//insert L2TP header
			*(uint16 *)(pData + 42) = htons(0x4002);									//Type(0), Length(1), Sequence(0), Offset(0), Priority(0), Version(2)
			*(uint16 *)(pData + 44) = htons(middle_header_len-8);						//Total length
			*(uint16 *)(pData + 46) = htons(pL2TPInfo->after_dial.gateway_tunnelId);	//Peer's Tunnel ID
			*(uint16 *)(pData + 48) = htons(pL2TPInfo->after_dial.gateway_sessionId);	//Peer's Session ID
		}

		if(PPPoEHDR_Off)
		{	
			len=pPktHdr->l3Len+2+TUNN_tag_len;
			//insert PPPoE header
			*(uint32 *)(pData + 12 - PPPoEHDR_Off) = htonl(0x88641100);						//EtherType, Code
			*(uint16 *)(pData + 16 - PPPoEHDR_Off) = htons(sid&0xffff);						//session ID
			*(uint16 *)(pData + 18 - PPPoEHDR_Off) = htons(len&0xffff);						//total Len
			pPktHdr->pPppoeLength=(uint16 *)(pData + 18 - PPPoEHDR_Off);
			
			//reset tagif and L3/4 offset
			pPktHdr->tagif|=PPPOE_TAGIF;
			pPktHdr->l4Offset+=8;
			pPktHdr->l3Offset+=8;
			pPktHdr->egressTagif|=PPPOE_TAGIF;

			//insert IP header
			*(uint32 *)(pData + 12) = htonl(0x00214500);						//EtherType, version, header length, DS field
		}
		else
		{
			//insert IP header
			*(uint32 *)(pData + 12) = htonl(0x08004500);						//EtherType, version, header length, DS field
		}
		*(uint16 *)(pData + 16) = htons(pPktHdr->l3Len+TUNN_tag_len);		//Total length
		*(uint16 *)(pData + 18) = htons((*pIpv4_header_identifier)++);		//Identification
		*(uint32 *)(pData + 20) = htonl(IP_flag_fragment_TTL_protocol);		//Flags, Fragment offset, TTL, protocol=GRE
		*(uint16 *)(pData + 24) = htons(0x0);								//Header checksum(caculate by hw)
		*(uint32 *)(pData + 26) = htonl(*pIpv4_addr);						//SIP
		*(uint32 *)(pData + 30) = htonl(*pGateway_ipv4_addr);				//DIP
		
		//insert PPP header
		*(uint32 *)(pData + 50) = htonl(0xff030021);						//Address, Control, Protocol=IPv4
		
		//pPktHdr->l3Offset += TUNN_tag_len;	//offset to outter IP header
		pPktHdr->l4Offset += TUNN_tag_len;	//offset to inner Layer4 header
	}

	return RT_ERR_RG_OK;
}

void _rtk_rg_addDSLITETag(rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb, rtk_rg_wan_type_t wan_type)
{	
	int TUNN_tag_len=40;
	unsigned char *pData;
#if defined(CONFIG_RTL9602C_SERIES)
	uint32 ihl_tc_flow;
#endif
	
	if(skb_cow_head(skb, TUNN_tag_len)<0)
	{
		WARNING("skb head room is not enough..return without insert DSLITE tag");
	}
	else
	{
		pData=skb_push(skb, TUNN_tag_len);
		//copy the mac addresses to the beginning of the new header.
		//since we push the length longer than 2 MACs address, we use memcpy to replace memmove for speed.
		memcpy(skb->data, skb->data + TUNN_tag_len, VLAN_ETH_ALEN<<1);
		skb->mac_header -= TUNN_tag_len;
		
		//reset DMAC and SMAC pointer
		pPktHdr->pDmac=pData;
		pPktHdr->pSmac=pData+VLAN_ETH_ALEN;

		//insert IPv6 header
#if defined(CONFIG_RTL9602C_SERIES)
		if(wan_type==RTK_RG_DSLITE)
		{
			*(uint16 *)(pData + 12) = htons(0x86dd);
			if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.dslite_info.rtk_dslite.tcOpt==RTK_L34_DSLITE_TC_OPT_ASSIGN)
				ihl_tc_flow=((rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.dslite_info.rtk_dslite.tc)|0x600)<<20;
			else
				ihl_tc_flow=((pPktHdr->tos)|0x600)<<20;
			ihl_tc_flow|=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.dslite_info.rtk_dslite.flowLabel;
			*(uint32 *)(pData + 14) = htonl(ihl_tc_flow);
			*(uint16 *)(pData + 18) = htons(pPktHdr->l3Len);					//Payload length
			*(uint16 *)(pData + 20) = htons(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.dslite_info.rtk_dslite.hopLimit|0x0400);	//Next header, Hop limit
			memcpy(pData + 22,rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr,IPV6_ADDR_LEN);
			memcpy(pData + 22 + IPV6_ADDR_LEN,rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr,IPV6_ADDR_LEN);
		}
		else
		{
			*(uint16 *)(pData + 12) = htons(0x86dd);
			if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.tcOpt==RTK_L34_DSLITE_TC_OPT_ASSIGN)
				ihl_tc_flow=((rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.tc)|0x600)<<20;
			else
				ihl_tc_flow=((pPktHdr->tos)|0x600)<<20;
			ihl_tc_flow|=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.flowLabel;
			*(uint32 *)(pData + 14) = htonl(ihl_tc_flow);
			*(uint16 *)(pData + 18) = htons(pPktHdr->l3Len);					//Payload length
			*(uint16 *)(pData + 20) = htons(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.hopLimit|0x0400);	//Next header, Hop limit
			memcpy(pData + 22,rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr,IPV6_ADDR_LEN);
			memcpy(pData + 22 + IPV6_ADDR_LEN,rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr,IPV6_ADDR_LEN);
		}
		
#else
		*(uint32 *)(pData + 12) = htonl(0x86dd6000);						//EtherType, version, traffic class, Flow label upper 4bits
		*(uint16 *)(pData + 16) = htons(0x0000);							//Flow label 16bits
		*(uint16 *)(pData + 18) = htons(pPktHdr->l3Len);					//Payload length
		*(uint16 *)(pData + 20) = htons(0x04ff);							//Next header, Hop limit

		if(wan_type==RTK_RG_DSLITE)
		{
			memcpy(pData + 22,rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.dslite_info.rtk_dslite.ipB4.ipv6_addr,IPV6_ADDR_LEN);
			memcpy(pData + 22 + IPV6_ADDR_LEN,rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.dslite_info.rtk_dslite.ipAftr.ipv6_addr,IPV6_ADDR_LEN);
		}
		else
		{
			memcpy(pData + 22,rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipB4.ipv6_addr,IPV6_ADDR_LEN);
			memcpy(pData + 22 + IPV6_ADDR_LEN,rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.pppoe_dslite_info.after_dial.dslite_hw_info.rtk_dslite.ipAftr.ipv6_addr,IPV6_ADDR_LEN);
		}
#endif		

		pPktHdr->ipv6PayloadLen=pPktHdr->l3Len;
		pPktHdr->l3Len+=TUNN_tag_len;
		pPktHdr->l4Offset+=TUNN_tag_len;
		pPktHdr->egressTagif|=DSLITE_TAGIF;
		pPktHdr->tagif|=IPV6_TAGIF;
	}
}

__IRAM_FWDENG
void _rtk_rg_removeTunnelTag(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	unsigned int tmpOffset,vlanOffset=0;

	//dump_packet(skb->data,skb->len,"before remove packet");
	//move the mac addresses to the beginning of the new offset
	if(pPktHdr->tagif&CVLAN_TAGIF)vlanOffset=4;
	tmpOffset=pPktHdr->l3Offset - (VLAN_ETH_ALEN<<1) - 2 - vlanOffset;
	//since the length we remove must longer than 2 MACs address, we use memcpy to replace memmove for speed.
	memcpy(skb->data + tmpOffset, skb->data, (VLAN_ETH_ALEN<<1)+vlanOffset);
	skb->data+=tmpOffset;
	skb->len-=tmpOffset;
	if(pPktHdr->l4Offset>0)
		pPktHdr->l4Offset-=tmpOffset;
	if(pPktHdr->l3Offset>0)
		pPktHdr->l3Offset-=tmpOffset;
	if(pPktHdr->tagif&IPV4_TAGIF)*(unsigned short *)(skb->data+(VLAN_ETH_ALEN<<1)+vlanOffset)=htons(0x0800);
	else *(unsigned short *)(skb->data+(VLAN_ETH_ALEN<<1)+vlanOffset)=htons(0x86dd);
	//dump_packet(skb->data,skb->len,"after remove packet");
}

__IRAM_FWDENG
rtk_rg_err_code_t _rtk_rg_fwdEngine_shortCutNaptPacketModify(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify)
{
	int l2Idx;
	
	//DEBUG("direct is %s",direct==NAPT_DIRECTION_OUTBOUND?"OUTBOUND":"INBOUND");	
	//TTL minus one
	if(pPktHdr->pIpv4TTL!=NULL)
		*pPktHdr->pIpv4TTL-=1;
	else
		dump_packet(skb->data,skb->len,"NULL packet..");


	
	if((direct==NAPT_DIRECTION_OUTBOUND)||(direct==NAPT_DIRECTION_ROUTING))
	{	

		// fool-proofing & debug
		if(l3Modify || l4Modify) assert(naptIdx<MAX_NAPT_OUT_SW_TABLE_SIZE);
	
		//update shortcut before packet modification
#ifdef CONFIG_ROME_NAPT_SHORTCUT
		if(pPktHdr->shortcutStatus==RG_SC_NEED_UPDATE)
		{
			//if Routing, l3modify, l4modify, and naptIdx are all zero
			//otherwise NAPT will set to one, respectively
			_rtk_rg_naptShortcutUpdate(pPktHdr,direct,naptIdx,(l3Modify&l4Modify),0);
		}
#endif

		l2Idx=pPktHdr->dmacL2Idx;
		assert((l2Idx!=FAIL) && (l2Idx<MAX_LUT_SW_TABLE_SIZE));
		/*
		FIXME("l2idx=%d mac=%02x:%02x:%02x:%02x:%02x:%02x\n",l2Idx,rg_db.mac[l2Idx].macAddr.octet[0]
		,rg_db.mac[l2Idx].macAddr.octet[1],rg_db.mac[l2Idx].macAddr.octet[2]
		,rg_db.mac[l2Idx].macAddr.octet[3],rg_db.mac[l2Idx].macAddr.octet[4]
		,rg_db.mac[l2Idx].macAddr.octet[5]);
		*/
		
		// fill DA
		memcpy(pPktHdr->pDmac,rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.mac.octet,ETHER_ADDR_LEN);

		//fill MSS
		if(pPktHdr->tagif&MSS_TAGIF)
		{
			if(pPktHdr->tagif&(IPV4_TAGIF|IPV6_TAGIF))
			{
				int offset=40;
				u16 netOrderOrgMss;

				if(pPktHdr->tagif&IPV6_TAGIF){
					offset+=20;					
				}				

				netOrderOrgMss=*pPktHdr->pMssLength;

				if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan==1)
				{
					*pPktHdr->pMssLength=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].p_wanStaticInfo->mtu-offset;
					*pPktHdr->pL4Checksum=htons(_rtk_rg_fwdengine_L4checksumUpdateForMss(*pPktHdr->pL4Checksum,ntohs(netOrderOrgMss),ntohs(*pPktHdr->pMssLength)));
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT					
					//20150518LUKE: Special Case!!Since we need to update other fields before send packet to WWAN, 
					//we update original checksum here for PRETENDING WE ARE NOT UPDATE L4 CHECKSUM YET!!!
					pPktHdr->l4Checksum=*pPktHdr->pL4Checksum;
#endif
					TRACE("outbound modify MSS from %d to %d, update L4 chksum(BOTH ORIGINAL AND POINTER)\n",netOrderOrgMss,*pPktHdr->pMssLength);				
				}
			}
			
		}

		//fill SIP
		if(l3Modify)
		{			
			*pPktHdr->pIpv4Sip=htonl(rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr);
#ifdef CONFIG_APOLLO_MODEL	
			TRACE("modify SIP to %d.%d.%d.%d",(rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr>>24)&0xff,(rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr>>16)&0xff,(rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr>>8)&0xff,rg_db.extip[pPktHdr->extipIdx].rtk_extip.extIpAddr&0xff);
#else
			TRACE("modify SIP to %d.%d.%d.%d",(*pPktHdr->pIpv4Sip>>24)&0xff,(*pPktHdr->pIpv4Sip>>16)&0xff,(*pPktHdr->pIpv4Sip>>8)&0xff,*pPktHdr->pIpv4Sip&0xff);
#endif
		}

		//fill SPORT
		if(l4Modify)
		{
			*pPktHdr->pSport=htons(rg_db.naptOut[naptIdx].extPort);
#ifdef CONFIG_APOLLO_MODEL
			TRACE("modify SPORT to %d",rg_db.naptOut[naptIdx].extPort);
#else
			TRACE("modify SPORT to %d",*pPktHdr->pSport);
#endif
		}

		assert(pPktHdr->netifIdx<MAX_NETIF_SW_TABLE_SIZE);
		if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan==1)
		{
			rtk_rg_wan_type_t wan_type=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type; 
			if(wan_type==RTK_RG_DSLITE||wan_type==RTK_RG_PPPoE_DSLITE)
			{
				_rtk_rg_addDSLITETag(pPktHdr,skb,wan_type);
			}
			else if(wan_type==RTK_RG_PPTP||wan_type==RTK_RG_L2TP)
			{
				//add pptp or l2tp header
				if(_rtk_rg_addTunnelTag(pPktHdr,skb,wan_type)==RT_ERR_RG_ENTRY_NOT_EXIST)
					return RT_ERR_RG_ENTRY_NOT_EXIST;
			}

			switch(wan_type)
			{
				case RTK_RG_PPPoE:
				case RTK_RG_PPPoE_DSLITE:
					//add pppoe header
					_rtk_rg_addPPPoETag(pPktHdr,skb,l3Modify,l4Modify);
				case RTK_RG_PPTP:
				case RTK_RG_L2TP:
				case RTK_RG_DSLITE:
				
					//FIXME: should we recaculate IP checksum for the new IP header in PPTP or L2TP??YES, inner checksum should update here
					//fragment checksum re-cal because GMAC cannot offload PPPoE packets
					//if(pPktHdr->ipv4FragPacket) 
					// 20130731: normal PPPoE can't checksum offload. because directTX turn on L34_KEEP.
					*pPktHdr->pIpv4Checksum=htons(_rtk_rg_fwdengine_L3checksumUpdate(*pPktHdr->pIpv4Checksum,pPktHdr->ipv4Sip,pPktHdr->ipv4TTL,pPktHdr->ipProtocol,ntohl(*pPktHdr->pIpv4Sip),*pPktHdr->pIpv4TTL));
					//20140625LUKE:when packet are fragemented, we should not re-caculate checksum here!!
					if(pPktHdr->ipv4FragPacket==0){
						if(pPktHdr->tagif&TCP_TAGIF)
							*pPktHdr->pL4Checksum = htons(_rtk_rg_fwdengine_L4checksumUpdate(pPktHdr->tcpFlags.ack,*pPktHdr->pL4Checksum,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->tcpSeq,pPktHdr->tcpAck,ntohl(*pPktHdr->pIpv4Sip),ntohs(*pPktHdr->pSport),ntohl(*pPktHdr->pTcpSeq),ntohl(*pPktHdr->pTcpAck)));			
						else if(pPktHdr->tagif&UDP_TAGIF)
							*pPktHdr->pL4Checksum = htons(_rtk_rg_fwdengine_L4checksumUpdate(0,*pPktHdr->pL4Checksum,pPktHdr->ipv4Sip,pPktHdr->sport,0,0,ntohl(*pPktHdr->pIpv4Sip),ntohs(*pPktHdr->pSport),0,0));
					}
					break;
				default:
					break;
			}
		}

		//fill SA
		memcpy(pPktHdr->pSmac,rg_db.netif[pPktHdr->netifIdx].rtk_netif.gateway_mac.octet,ETHER_ADDR_LEN);

		if((pPktHdr->overMTU==1)&&(pPktHdr->ipv4FragPacket==0)&&((pPktHdr->egressTagif&PPPOE_TAGIF)==0)&&((pPktHdr->egressTagif&PPTP_TAGIF)==0)&&((pPktHdr->egressTagif&L2TP_TAGIF)==0)&&((pPktHdr->egressTagif&DSLITE_TAGIF)==0)){				
			if(pPktHdr->tagif&TCP_TAGIF)
				*pPktHdr->pL4Checksum = htons(_rtk_rg_fwdengine_L4checksumUpdate(pPktHdr->tcpFlags.ack,*pPktHdr->pL4Checksum,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->tcpSeq,pPktHdr->tcpAck,ntohl(*pPktHdr->pIpv4Sip),ntohs(*pPktHdr->pSport),ntohl(*pPktHdr->pTcpSeq),ntohl(*pPktHdr->pTcpAck)));
			else if(pPktHdr->tagif&UDP_TAGIF)
				*pPktHdr->pL4Checksum = htons(_rtk_rg_fwdengine_L4checksumUpdate(0,*pPktHdr->pL4Checksum,pPktHdr->ipv4Sip,pPktHdr->sport,0,0,ntohl(*pPktHdr->pIpv4Sip),ntohs(*pPktHdr->pSport),0,0));
			TRACE("OverMTU checksum calculate by software ID=0x%x checksum==>0x%04x!",*pPktHdr->pIpv4Identification,*pPktHdr->pL4Checksum );
		}//else{
			// PPPoE, PPTP, L2TP L4 checksum already calculated when add tag
			// none-overMTU IPoE L4 checksum calculated by HW
			// fragmented packet checksum calculated in Layer34 forward if needed
		//}
	}else{ //INBOUND
		//ipaddr_t originalDIP;
		//int16 originalDPort;
		rtk_rg_sipDipClassification_t sipdipClass;

		// fool-proofing & debug
		if(l3Modify || l4Modify) assert(naptIdx<MAX_NAPT_IN_SW_TABLE_SIZE);

		//originalDIP=pPktHdr->ipv4Dip;
		//originalDPort=pPktHdr->dport;

		//fill DIP
		if(l3Modify){
			*pPktHdr->pIpv4Dip=htonl(rg_db.naptIn[naptIdx].rtk_naptIn.intIp);
#ifdef CONFIG_APOLLO_MODEL
			TRACE("modify DIP to %d.%d.%d.%d",(rg_db.naptIn[naptIdx].rtk_naptIn.intIp>>24)&0xff,(rg_db.naptIn[naptIdx].rtk_naptIn.intIp>>16)&0xff,(rg_db.naptIn[naptIdx].rtk_naptIn.intIp>>8)&0xff,rg_db.naptIn[naptIdx].rtk_naptIn.intIp&0xff);
#else
			TRACE("modify DIP to %d.%d.%d.%d [by NAPT inIdx=%d]",(*pPktHdr->pIpv4Dip>>24)&0xff,(*pPktHdr->pIpv4Dip>>16)&0xff,(*pPktHdr->pIpv4Dip>>8)&0xff,*pPktHdr->pIpv4Dip&0xff,naptIdx);
#endif

			//Decide DMACL2Idx using New DIP (*pPktHdr->pIpv4Dip)
		}
		//else
		//{
			//Decide DMACL2Idx using Original DIP (*pPktHdr->pIpv4Dip)
		//}
		sipdipClass=SIP_DIP_CLASS_NAPTR;
		if(_rtk_rg_routingDecisionTablesLookup(pPktHdr,&sipdipClass)!=RG_FWDENGINE_RET_CONTINUE)
			return RT_ERR_RG_ARP_NOT_FOUND;

#ifdef CONFIG_ROME_NAPT_SHORTCUT
		if(pPktHdr->shortcutStatus==RG_SC_NEED_UPDATE)	//for UDP inbound packet
			_rtk_rg_naptShortcutUpdate(pPktHdr,direct,naptIdx,1,0);
#endif

		//fill DPORT
		if(l4Modify){
			*pPktHdr->pDport=htons(rg_db.naptIn[naptIdx].rtk_naptIn.intPort);
#ifdef CONFIG_APOLLO_MODEL
			TRACE("modify DPORT to %d",rg_db.naptIn[naptIdx].rtk_naptIn.intPort);
#else
			TRACE("modify DPORT to %d",*pPktHdr->pDport);
#endif
		}


		//fill DA
		l2Idx=pPktHdr->dmacL2Idx;
		assert((l2Idx!=FAIL) && (l2Idx<MAX_LUT_SW_TABLE_SIZE));
		//assert_ok(rtk_rg_macEntry_find(&mac,&l2Idx));
		//memcpy(pPktHdr->pDmac,mac.mac.octet,6);

		//Check DMAC
#if 0
		{
			/* For inbound connection, the very first SYN packet will be forwarding to internal host but the MAC of host may be unknownd. */
			int zeroDA[6] = {0};
			int result = 0;
			result = memcmp(&zeroDA[0],pPktHdr->pDmac,ETHER_ADDR_LEN);

			//DA is not valid MAC address.
			if(result==0)
			{
				DEBUG("MAC miss....send arp packet...\n");
				_rtk_rg_fwdengine_handleArpMiss(pPktHdr);
				return RT_ERR_RG_L2_ENTRY_NOT_FOUND;
			}
		}
#else
		if(rg_db.lut[l2Idx].valid==0){
			DEBUG("DMAC miss...send ARP request!");
			_rtk_rg_fwdengine_handleArpMiss(pPktHdr);
			return RT_ERR_RG_L2_ENTRY_NOT_FOUND;
		}else
			memcpy(pPktHdr->pDmac,rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.mac.octet,ETHER_ADDR_LEN);
#endif
		//fill SA
		assert(pPktHdr->netifIdx<MAX_NETIF_SW_TABLE_SIZE);
		memcpy(pPktHdr->pSmac,rg_db.netif[pPktHdr->netifIdx].rtk_netif.gateway_mac.octet,ETHER_ADDR_LEN);

		if(pPktHdr->tagif&PPPOE_TAGIF)
			_rtk_rg_removePPPoETag(pPktHdr);

		//20141209LUKE: fix inbound overMTU didn't recalculate checksum problem
		if((pPktHdr->overMTU==1)&&(pPktHdr->ipv4FragPacket==0))
		{				
			if(pPktHdr->tagif&TCP_TAGIF)
				*pPktHdr->pL4Checksum = htons(_rtk_rg_fwdengine_L4checksumUpdate(pPktHdr->tcpFlags.ack,*pPktHdr->pL4Checksum,pPktHdr->ipv4Dip,pPktHdr->dport,pPktHdr->tcpSeq,pPktHdr->tcpAck,ntohl(*pPktHdr->pIpv4Dip),ntohs(*pPktHdr->pDport),ntohl(*pPktHdr->pTcpSeq),ntohl(*pPktHdr->pTcpAck)));
			else if(pPktHdr->tagif&UDP_TAGIF)
				*pPktHdr->pL4Checksum = htons(_rtk_rg_fwdengine_L4checksumUpdate(0,*pPktHdr->pL4Checksum,pPktHdr->ipv4Dip,pPktHdr->dport,0,0,ntohl(*pPktHdr->pIpv4Dip),ntohs(*pPktHdr->pDport),0,0));
			TRACE("OverMTU checksum calculate by software ID=0x%x checksum==>0x%04x!",*pPktHdr->pIpv4Identification,*pPktHdr->pL4Checksum );
		}
	}
	
	if(rg_db.systemGlobal.fwdStatistic){
		if(l3Modify)
			rg_db.systemGlobal.statistic.perPortCnt_L4FWD[pPktHdr->ingressPort]++;			
		else
			rg_db.systemGlobal.statistic.perPortCnt_IPv4_L3FWD[pPktHdr->ingressPort]++;			
	}

	return RT_ERR_RG_OK;
}

rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_naptPacketModify(int direct, int naptIdx, rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb,int l3Modify,int l4Modify)
{	
	int naptFilterRet;
	rtk_rg_err_code_t ret;
	rtk_rg_fwdEngineAlgReturn_t alg_ret;
	
	int naptOutIdx, naptInIdx;

	//rtk_rg_macEntry_t mac;
	
	//DEBUG("pPktHdr->dmacL2Idx=%d\n",pPktHdr->dmacL2Idx);

	if(l3Modify && l4Modify)
	{
		//Alg pre function check
		alg_ret=_rtk_rg_algForward(direct,0,skb,pPktHdr);
		if(alg_ret==RG_FWDENGINE_ALG_RET_DROP)
		{
			TRACE("Drop by PRE-ALG");
			return RG_FWDENGINE_RET_DROP;
		}
	}

	//20151211LUKE: execute pre-route callback for DPI check.
	if(direct!=NAPT_DIRECTION_ROUTING && rg_db.systemGlobal.initParam.naptPreRouteDPICallBack!=NULL){
		rtk_rg_naptPreRouteCallBackReturn_t ret=(rtk_rg_naptPreRouteCallBackReturn_t)rg_db.systemGlobal.initParam.naptPreRouteDPICallBack((void *)pPktHdr,direct);
		if(ret==RG_FWDENGINE_PREROUTECB_DROP){
			TRACE("PreRoute DPI CallBack return: DROP!");
			return RG_FWDENGINE_RET_DROP;
		}else if(ret==RG_FWDENGINE_PREROUTECB_TRAP){
			TRACE("PreRoute DPI CallBack return: TRAP to protocol stack!");
			return RG_FWDENGINE_RET_TO_PS;
		}
		TRACE("PreRoute DPI CallBack return: Continue.");
	}

	//DEBUG("skb is %p, pkthdr is %p, ttl is %p",skb,pPktHdr,pPktHdr->pIpv4TTL);
	pPktHdr->l3Modify=l3Modify;
	pPktHdr->l4Modify=l4Modify;
	ret = _rtk_rg_fwdEngine_shortCutNaptPacketModify(direct,naptIdx,pPktHdr,skb,l3Modify,l4Modify);
	DEBUG("naptPacketModify: l3Modify=%d, l4Modify=%d, naptIdx=%d",l3Modify,l4Modify,naptIdx);
	if((l3Modify && l4Modify && naptIdx>=0) || (pPktHdr->tagif&ICMP_TAGIF))
	{//naptPriority only supported in NAPT mode
		assert(naptIdx<MAX_NAPT_OUT_SW_TABLE_SIZE);

		if((pPktHdr->tagif&TCP_TAGIF) || (pPktHdr->tagif&UDP_TAGIF)||(pPktHdr->tagif&ICMP_TAGIF))
		{
			if((pPktHdr->tagif&TCP_TAGIF) || (pPktHdr->tagif&UDP_TAGIF))
			{
				//Napt index only check while NAPT(TCP/UDP) flow 
			naptOutIdx = pPktHdr->naptOutboundIndx;
			naptInIdx = pPktHdr->naptrInboundIndx;	

			if((pPktHdr->naptOutboundIndx<0) || (pPktHdr->naptOutboundIndx >= MAX_NAPT_OUT_SW_TABLE_SIZE)) {
				rtlglue_printf("naptPriority assign failed!!! naptOutIdx=%d is out of valid SW NAPT range!\n",pPktHdr->naptOutboundIndx);
				goto skipNaptPriorityAssign;
			}
			
			if((pPktHdr->naptrInboundIndx<0) || (pPktHdr->naptrInboundIndx >= MAX_NAPT_IN_SW_TABLE_SIZE)) {
				rtlglue_printf("naptPriority assign failed!!! naptInIdx=%d is out of valid SW NAPTr range!\n",pPktHdr->naptrInboundIndx);
				goto skipNaptPriorityAssign;
			}

			TRACE("naptPriority get naptOutIdx=%d naptInIdx=%d",pPktHdr->naptOutboundIndx,pPktHdr->naptrInboundIndx);
			}
			else
			{
				TRACE("naptPriority for ICMP packet without naptOutIdx/naptInIdx");
			}
			
			if(	(rg_db.systemGlobal.pValidUsNaptPriorityRuleStart!= NULL) || (rg_db.systemGlobal.pValidDsNaptPriorityRuleStart!=NULL)){//check naptFilter while any rule is set. else save the time.
				naptFilterRet=_rtk_rg_naptPriority_assign(direct,pPktHdr,&rg_db.naptOut[pPktHdr->naptOutboundIndx],&rg_db.naptIn[pPktHdr->naptrInboundIndx]);
				if(naptFilterRet==RG_FWDENGINE_RET_DROP)
					return RG_FWDENGINE_RET_DROP;
			}
		}
	}
		
skipNaptPriorityAssign:


	if(ret!=RT_ERR_RG_OK)
	{
		TRACE("Drop by Shortcut Modify!");
		return RG_FWDENGINE_RET_DROP;
	}

	//Alg post function check
	if(l3Modify && l4Modify)
	{		
		alg_ret=_rtk_rg_algForward(direct,1,skb,pPktHdr);
		if(alg_ret==RG_FWDENGINE_ALG_RET_DROP)
		{
			TRACE("Drop by POST-ALG");
			return RG_FWDENGINE_RET_DROP;
		}
	}

	//20151211LUKE: execute forward callback for DPI check.
	if(direct!=NAPT_DIRECTION_ROUTING && rg_db.systemGlobal.initParam.naptForwardDPICallBack!=NULL){
		rtk_rg_naptForwardCallBackReturn_t ret=(rtk_rg_naptForwardCallBackReturn_t)rg_db.systemGlobal.initParam.naptForwardDPICallBack((void *)pPktHdr,direct);
		if(ret==RG_FWDENGINE_FORWARDCB_CONTINUE_DPI){
			TRACE("Forward DPI CallBack return: DPI_CONTINUE_CHECK!");
			//Do not add to hw for this flow, also remove shortcut created
			pPktHdr->addNaptAfterNicTx=0;
			if(pPktHdr->shortcutStatus!=RG_SC_NORMAL_PATH){
				pPktHdr->shortcutStatus=RG_SC_NORMAL_PATH;
				_rtk_rg_v4ShortCut_delete(pPktHdr->currentShortcutIdx);
				if((direct==NAPT_DIRECTION_OUTBOUND)&&(pPktHdr->tagif&TCP_TAGIF)){
					if(rg_db.systemGlobal.tcp_hw_learning_at_syn==1 && rg_db.systemGlobal.tcp_in_shortcut_learning_at_syn)
						_rtk_rg_v4ShortCut_delete(pPktHdr->inboundShortcutIdx);
				}
			}
			
			if(direct==NAPT_DIRECTION_OUTBOUND){
				rg_db.naptOut[naptIdx].cannotAddToHw=1;
				rg_db.naptIn[rg_db.naptOut[naptIdx].rtk_naptOut.hashIdx].cannotAddToHw=1;
			}else{
				rg_db.naptIn[naptIdx].cannotAddToHw=1;
				rg_db.naptOut[rg_db.naptIn[naptIdx].symmetricNaptOutIdx].cannotAddToHw=1;
			}
		}else if(ret==RG_FWDENGINE_FORWARDCB_DROP){
			TRACE("Forward DPI CallBack return: DROP!");
			return RG_FWDENGINE_RET_DROP;
		}else{
			TRACE("Forward DPI CallBack return: DPI_FINISH_CHECK.");
			if(direct==NAPT_DIRECTION_OUTBOUND){
				rg_db.naptOut[naptIdx].cannotAddToHw=0;
				rg_db.naptIn[rg_db.naptOut[naptIdx].rtk_naptOut.hashIdx].cannotAddToHw=0;
			}else{
				rg_db.naptIn[naptIdx].cannotAddToHw=0;
				rg_db.naptOut[rg_db.naptIn[naptIdx].symmetricNaptOutIdx].cannotAddToHw=0;		
			}
		}
	}
	
	//dump_packet(skb->data,skb->len,"new");
	return RG_FWDENGINE_RET_CONTINUE;
}

__IRAM_FWDENG
void _rtk_rg_fwdEngine_ipv6ShortCutPacketModify(int direct, int intfType,rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb)
{	
	int l2Idx;
	
	*pPktHdr->pIPv6HopLimit-=1;

	if(rg_db.systemGlobal.fwdStatistic){
		rg_db.systemGlobal.statistic.perPortCnt_IPv6_L3FWD[pPktHdr->ingressPort]++;			
	}

	//update shortcut before packet modification
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	if(pPktHdr->shortcutStatus==RG_SC_NEED_UPDATE)_rtk_rg_naptV6ShortcutUpdate(pPktHdr,0);
#endif
	if(direct==NAPT_DIRECTION_OUTBOUND || direct==IPV6_ROUTE_OUTBOUND){
		l2Idx=pPktHdr->dmacL2Idx;
		assert(l2Idx!=FAIL);
		/*
		FIXME("l2idx=%d mac=%02x:%02x:%02x:%02x:%02x:%02x\n",l2Idx,rg_db.mac[l2Idx].macAddr.octet[0]
		,rg_db.mac[l2Idx].macAddr.octet[1],rg_db.mac[l2Idx].macAddr.octet[2]
		,rg_db.mac[l2Idx].macAddr.octet[3],rg_db.mac[l2Idx].macAddr.octet[4]
		,rg_db.mac[l2Idx].macAddr.octet[5]);
		*/
		
		// fill DA
		memcpy(pPktHdr->pDmac,rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.mac.octet,ETHER_ADDR_LEN);

		//fill SA
		memcpy(pPktHdr->pSmac,rg_db.netif[pPktHdr->netifIdx].rtk_netif.gateway_mac.octet,ETHER_ADDR_LEN);

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
		//fill EXTPORT
		DEBUG("direct = %d",direct);
		if(direct==NAPT_DIRECTION_OUTBOUND && ((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF))){
			//SIP
			memcpy(pPktHdr->pIpv6Sip,rg_db.v6Extip[pPktHdr->pIPv6StatefulList->extipIdx].externalIp.ipv6_addr,IPV6_ADDR_LEN);
			TRACE("modify SIP to %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
				pPktHdr->pIpv6Sip[0],pPktHdr->pIpv6Sip[1],pPktHdr->pIpv6Sip[2],pPktHdr->pIpv6Sip[3],
				pPktHdr->pIpv6Sip[4],pPktHdr->pIpv6Sip[5],pPktHdr->pIpv6Sip[6],pPktHdr->pIpv6Sip[7],
				pPktHdr->pIpv6Sip[8],pPktHdr->pIpv6Sip[9],pPktHdr->pIpv6Sip[10],pPktHdr->pIpv6Sip[11],
				pPktHdr->pIpv6Sip[12],pPktHdr->pIpv6Sip[13],pPktHdr->pIpv6Sip[14],pPktHdr->pIpv6Sip[15]);

			//Sport
			*pPktHdr->pSport=htons(pPktHdr->pIPv6StatefulList->externalPort);
			TRACE("modify SPORT to %d",pPktHdr->pIPv6StatefulList->externalPort);
		}		
#endif		

		//add pppoe header
		if(intfType==L34_NH_PPPOE)
		{
			#if defined(CONFIG_RTL9602C_SERIES)
			if((pPktHdr->tagif&PPPOE_TAGIF) == 0) 
					_rtk_rg_addPPPoETag(pPktHdr,skb,0,0);
			#else
			_rtk_rg_addPPPoETag(pPktHdr,skb,0,0);
			#endif
		}
		else if(pPktHdr->tagif&PPPOE_TAGIF)
			_rtk_rg_removePPPoETag(pPktHdr);	//remove pppoe header while routing mode

	}else{ //INBOUND
		//fill DA
		l2Idx=pPktHdr->dmacL2Idx;
		assert(l2Idx!=FAIL);
		//assert_ok(rtk_rg_macEntry_find(&mac,&l2Idx));
		//memcpy(pPktHdr->pDmac,mac.mac.octet,6);
		memcpy(pPktHdr->pDmac,rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.mac.octet,ETHER_ADDR_LEN);

		//fill SA
		memcpy(pPktHdr->pSmac,rg_db.netif[pPktHdr->netifIdx].rtk_netif.gateway_mac.octet,ETHER_ADDR_LEN);

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
		DEBUG("direct = %d",direct);
		if(direct==NAPT_DIRECTION_INBOUND && ((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF))){
			DEBUG("pPktHdr->pIPv6StatefulList is %p",pPktHdr->pIPv6StatefulList);
			//DIP
			memcpy(pPktHdr->pIpv6Dip,pPktHdr->pIPv6StatefulList->internalIP.ipv6_addr,IPV6_ADDR_LEN);
			TRACE("modify DIP to %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
				pPktHdr->pIpv6Dip[0],pPktHdr->pIpv6Dip[1],pPktHdr->pIpv6Dip[2],pPktHdr->pIpv6Dip[3],
				pPktHdr->pIpv6Dip[4],pPktHdr->pIpv6Dip[5],pPktHdr->pIpv6Dip[6],pPktHdr->pIpv6Dip[7],
				pPktHdr->pIpv6Dip[8],pPktHdr->pIpv6Dip[9],pPktHdr->pIpv6Dip[10],pPktHdr->pIpv6Dip[11],
				pPktHdr->pIpv6Dip[12],pPktHdr->pIpv6Dip[13],pPktHdr->pIpv6Dip[14],pPktHdr->pIpv6Dip[15]);

			//Dport
			*pPktHdr->pDport=htons(pPktHdr->pIPv6StatefulList->internalPort);
			TRACE("modify DPORT to %d",pPktHdr->pIPv6StatefulList->internalPort);
		}		
#endif	

		//remove pppoe header
		if(pPktHdr->tagif&PPPOE_TAGIF)
			_rtk_rg_removePPPoETag(pPktHdr);
	}

}

rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_ipv6PacketModify(rtk_rg_naptDirection_t direct, rtk_l34_nexthop_type_t intfType,rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb)
{

	//Hop Limit minus one with checking
	if(*pPktHdr->pIPv6HopLimit<=1)
		return RG_FWDENGINE_RET_TO_PS;

	//Alg pre function check
	//_rtk_rg_algForward(direct,0,skb,pPktHdr);
	
	_rtk_rg_fwdEngine_ipv6ShortCutPacketModify(direct,intfType,pPktHdr,skb);

	//Alg post function check
	//_rtk_rg_algForward(direct,1,skb,pPktHdr);

	return RG_FWDENGINE_RET_CONTINUE;
}

const unsigned char fragHashTable[256] =
{
    98,  6, 85,150, 36, 23,112,164,135,207,169,  5, 26, 64,165,219, //  1
    61, 20, 68, 89,130, 63, 52,102, 24,229,132,245, 80,216,195,115, //  2
    90,168,156,203,177,120,  2,190,188,  7,100,185,174,243,162, 10, //  3
   237, 18,253,225,  8,208,172,244,255,126,101, 79,145,235,228,121, //  4
   123,251, 67,250,161,  0,107, 97,241,111,181, 82,249, 33, 69, 55, //  5
    59,153, 29,  9,213,167, 84, 93, 30, 46, 94, 75,151,114, 73,222, //  6
   197, 96,210, 45, 16,227,248,202, 51,152,252,125, 81,206,215,186, //  7
    39,158,178,187,131,136,  1, 49, 50, 17,141, 91, 47,129, 60, 99, //  8
   154, 35, 86,171,105, 34, 38,200,147, 58, 77,118,173,246, 76,254, //  9
   133,232,196,144,198,124, 53,  4,108, 74,223,234,134,230,157,139, // 10
   189,205,199,128,176, 19,211,236,127,192,231, 70,233, 88,146, 44, // 11
   183,201, 22, 83, 13,214,116,109,159, 32, 95,226,140,220, 57, 12, // 12
   221, 31,209,182,143, 92,149,184,148, 62,113, 65, 37, 27,106,166, // 13
     3, 14,204, 72, 21, 41, 56, 66, 28,193, 40,217, 25, 54,179,117, // 14
   238, 87,240,155,180,170,242,212,191,163, 78,218,137,194,175,110, // 15
    43,119,224, 71,122,142, 42,160,104, 48,247,103, 15, 11,138,239  // 16
};

unsigned int _rtk_rg_hashFun_xPear(uint8 len, uint8 ipProto, uint32 srcAddr, uint32 destAddr, uint16 identification)
{
	unsigned char *x, ch, h, i;
	unsigned int hex;
	unsigned short hh[len>>1];

	ch=ipProto; 
	for (i=0; i<(len>>1); i++)
	{
		// standard Pearson hash (output is h)
		h=0;
		h=fragHashTable[h ^ ch];		//ipProtocol
		if(srcAddr!=0)		//inbound hash will assign srcAddr as 0
		{
			x=(uint8 *)&srcAddr;
			h=fragHashTable[h ^ *x];
			h=fragHashTable[h ^ *(x+1)];
			h=fragHashTable[h ^ *(x+2)];
			h=fragHashTable[h ^ *(x+3)];
		}
		x=(uint8 *)&destAddr;
		h=fragHashTable[h ^ *x];
		h=fragHashTable[h ^ *(x+1)];
		h=fragHashTable[h ^ *(x+2)];
		h=fragHashTable[h ^ *(x+3)];
		x=(uint8 *)&identification;
		h=fragHashTable[h ^ *(x)];
		h=fragHashTable[h ^ *(x+1)];
		
		hh[i]=h;	// store result
		ch=ch+1; // increment first data byte by 1
	}

	//FIXME("hh[0].a is %02x, hh[1].a is %02x",hh[0].a,hh[1].a);
	if(len==4)
		hex = ((hh[0]&0xff)<<8) + (hh[1]&0xff);		//16bits//hex = ((hh[0].a&0xff)<<8) + (hh[1].a&0xff);		//16bits
	else
		hex = hh[0]&0xff;		//8bits//hex = hh[0].a&0xff;		//8bits
    
	return hex;
}

uint32 _rtk_rg_ipv4FragInHashIndex(uint8 ipProto, uint32 destAddr, uint16 identification)
{
	return _rtk_rg_hashFun_xPear(2,ipProto,0,destAddr,identification);
}

uint32 _rtk_rg_ipv4FragOutHashIndex(uint8 ipProto, uint32 srcAddr, uint32 destAddr, uint16 identification) 
{
	return _rtk_rg_hashFun_xPear(4,ipProto,srcAddr,destAddr,identification)&0x1ff;
}

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
uint32 _rtk_rg_ipv6StatefulHashIndex(uint8 ipProto, uint8 *intAddr, uint8 *remoteAddr, uint16 intPort, uint16 remotePort)
{
	uint32 in,rem;

	in = (*(uint32 *)intAddr) ^ (*(uint32 *)(intAddr+4)) ^ (*(uint32 *)(intAddr+8)) ^ (*(uint32 *)(intAddr+12));
	rem = (*(uint32 *)remoteAddr) ^ (*(uint32 *)(remoteAddr+4)) ^ (*(uint32 *)(remoteAddr+8)) ^ (*(uint32 *)(remoteAddr+12));
	
	//return _rtk_rg_hashFun_xPear(4,ipProto,in,rem,intPort^remotePort)&0x1ff;
	return _rtk_rg_hashFun_xPear(2,ipProto,in,rem,intPort^remotePort);	//8bits
	//return ((in ^ rem)&0x1ff);
}
#endif

__IRAM_FWDENG
uint32 _rtk_rg_shortcutHashIndex(uint32 srcIP, uint32 destIP, uint16 srcPort, uint16 destPort)
{
	//one-way
	uint32 in=0,rem=0,mask;

	mask=(MAX_NAPT_SHORTCUT_SIZE>>MAX_NAPT_SHORTCUT_WAYS_SHIFT)-1;
#if 0	
	in=(srcIP&0xffff)^(srcIP>>16)^srcPort^destPort;
	while(in>0)
	{
		rem^=(in&mask);
		in>>=FWD_SHORTCUT_BIT_NUM;
	}
#else
	in=(srcPort<<6)+destPort+srcIP+destIP;
	while(in>0)
	{
		rem+=(in&mask);		
		in>>=FWD_SHORTCUT_BIT_NUM;
	}
	rem=((rem&mask)+(rem>>MAX_NAPT_SHORTCUT_SIZE_SHFIT))&mask;
#endif

	return (rem<<MAX_NAPT_SHORTCUT_WAYS_SHIFT);
}

__IRAM_FWDENG
uint32 _rtk_rg_ipv6ShortcutHashIndex(uint32 srcIP, uint16 srcPort, uint16 destPort)
{
	//one-way
	uint32 in=0,rem=0,mask;

	mask=(MAX_NAPT_V6_SHORTCUT_SIZE>>MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT)-1;

	in=(srcPort<<6)+destPort+srcIP;
	while(in>0)
	{
		rem+=(in&mask);		
		in>>=FWD_V6_SHORTCUT_BIT_NUM;
	}
	rem=((rem&mask)+(rem>>MAX_NAPT_V6_SHORTCUT_SIZE_SHFIT))&mask;

	return (rem<<MAX_NAPT_V6_SHORTCUT_WAYS_SHIFT);
}


void _rtk_rg_fwdEngine_fragmentPacketQueuing(rtk_rg_naptDirection_t direction, struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	int i,queueIdx;
	long compareTime;
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragQueueLock);

	//DEBUG("the queueTime is %ld",jiffies);
	queueIdx=0;
	compareTime=0;

	//find the first valid one or smallest queue_time(oldest) to use

	for(i=0;i<MAX_IPV4_FRAGMENT_QUEUE_SIZE;i++)
	{
		if(rg_db.ipv4FragmentQueue[i].occupied==0)
		{
			compareTime = 0;
			queueIdx = i;
			rg_db.systemGlobal.ipv4FragmentQueueNum++;
			break;
		}
		else if(rg_db.ipv4FragmentQueue[i].queue_time < compareTime || compareTime == 0)
		{
			compareTime = rg_db.ipv4FragmentQueue[i].queue_time;
			queueIdx = i;
		}
	}	

	//free the oldest one skb
	if(compareTime>0)
		_rtk_rg_dev_kfree_skb_any(rg_db.ipv4FragmentQueue[queueIdx].queue_skb);

	//insert new packet into this idx
	rg_db.ipv4FragmentQueue[queueIdx].queue_skb=skb;
	memcpy(&rg_db.ipv4FragmentQueue[queueIdx].queue_pktHdr,pPktHdr,sizeof(rtk_rg_pktHdr_t));
	memcpy(&rg_db.ipv4FragmentQueue[queueIdx].queue_rx_info,pPktHdr->pRxDesc,sizeof(struct rx_info));
	if(pPktHdr->cp)memcpy(&rg_db.ipv4FragmentQueue[queueIdx].queue_cp,pPktHdr->cp,sizeof(struct re_private));
	rg_db.ipv4FragmentQueue[queueIdx].queue_time=jiffies;
	rg_db.ipv4FragmentQueue[queueIdx].occupied=1;
	rg_db.ipv4FragmentQueue[queueIdx].direction=direction;

	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv4FragQueueLock);

}

void _rtk_rg_fwdEngine_fragmentQueueProcessing(int aclRet, rtk_rg_pktHdr_t *pPktHdr)
{
	int i,totalQueueNum;
	int naptIdx=FAIL;
	//If we are the first fragment packet:
	//Check the queue to see if there is any packet has same identification and SIP,DIP
	//Loop if match:
	//	no need to recompute the L3 checksum, let HW do it
	//	Forward the packet
	//	Move the last one to the proceed queue position
	//	Queue number --
	totalQueueNum=rg_db.systemGlobal.ipv4FragmentQueueNum;
	for(i=0;i<MAX_IPV4_FRAGMENT_QUEUE_SIZE&&totalQueueNum>0;i++)
	{
		//DEBUG("rg_db.systemGlobal.ipv4FragmentQueueNum is %d",rg_db.systemGlobal.ipv4FragmentQueueNum);
		//DEBUG("rg_db.ipv4FragmentQueue[i].occupied = %d",rg_db.ipv4FragmentQueue[i].occupied);
		//DEBUG("*pktHdr.pIpv4Identification is %x",*pPktHdr->pIpv4Identification);
		//DEBUG("*rg_db.ipv4FragmentQueue[i].queue_pktHdr.pIpv4Identification = %x",*rg_db.ipv4FragmentQueue[i].queue_pktHdr.pIpv4Identification);
		//------------------ Critical Section start -----------------------//
		rg_lock(&rg_kernel.ipv4FragQueueLock);
		if(rg_db.ipv4FragmentQueue[i].occupied)
		{
			totalQueueNum--;
			if(pPktHdr->ipv4Sip==rg_db.ipv4FragmentQueue[i].queue_pktHdr.ipv4Sip &&
				*(pPktHdr->pIpv4Identification)==*(rg_db.ipv4FragmentQueue[i].queue_pktHdr.pIpv4Identification))
			{
				//DEBUG("queue [%d] Match!!",i);

				//Check ACL action by first packet
				if(aclRet==RG_FWDENGINE_RET_TO_PS)
				{
#ifdef __KERNEL__
					//from master wifi
					if(rg_db.ipv4FragmentQueue[i].queue_pktHdr.ingressPort==RTK_RG_EXT_PORT0)
					{
						rg_db.ipv4FragmentQueue[i].queue_skb->data+=ETH_HLEN;
						rg_db.ipv4FragmentQueue[i].queue_skb->len-=ETH_HLEN;
						TRACE("WLAN0 queue_packet trap to netif_rx\n");
						netif_rx(rg_db.ipv4FragmentQueue[i].queue_skb);	
					}
					else
						re8670_rx_skb(&rg_db.ipv4FragmentQueue[i].queue_cp,rg_db.ipv4FragmentQueue[i].queue_skb,&rg_db.ipv4FragmentQueue[i].queue_rx_info);
#else
					model_nic_rx_skb(&rg_db.ipv4FragmentQueue[i].queue_cp,rg_db.ipv4FragmentQueue[i].queue_skb,&rg_db.ipv4FragmentQueue[i].queue_rx_info);
#endif
				}
				else if(aclRet==RG_FWDENGINE_RET_DROP)
					_rtk_rg_dev_kfree_skb_any(rg_db.ipv4FragmentQueue[i].queue_skb);
				else
				{
					//20141119LUKE: for oubound, we can modify IP without naptIdx, but for inbound we need to modify it to internal IP by naptInIdx!
					if(rg_db.ipv4FragmentQueue[i].direction==NAPT_DIRECTION_INBOUND)
						naptIdx=pPktHdr->naptrInboundIndx;
					_rtk_rg_fwdEngine_naptPacketModify(rg_db.ipv4FragmentQueue[i].direction,naptIdx,&rg_db.ipv4FragmentQueue[i].queue_pktHdr,rg_db.ipv4FragmentQueue[i].queue_skb,1,0);		//L4Modify is 0, so naptIdx is don't care

					//DEBUG("send the queued fragment packet [%d]!",i);
#ifdef CONFIG_APOLLO_MODEL			
#else
					_rtk_rg_fwdEngineDirectTx(rg_db.ipv4FragmentQueue[i].queue_skb,&rg_db.ipv4FragmentQueue[i].queue_pktHdr);
					//re8686_send_with_txInfo(rg_db.ipv4FragmentQueue[i].queue_skb,&txDesc,0);
					//_rtk_rg_splitJumboSendToNicWithTxInfoAndMask(??,rg_db.ipv4FragmentQueue[i].queue_skb,&rg_kernel.txDesc,0,&rg_kernel.txDescMask);
#endif
				}
				rg_db.systemGlobal.ipv4FragmentQueueNum--;
				rg_db.ipv4FragmentQueue[i].occupied = 0;
				rg_db.ipv4FragmentQueue[i].queue_time = 0;
			}
		}
		//------------------ Critical Section End -----------------------//
		rg_unlock(&rg_kernel.ipv4FragQueueLock);
	}

}

rtk_rg_lookupIdxReturn_t _rtk_rg_fwdEngine_fragmentOutHashIndexLookup(int *hashIdx, rtk_rg_ipv4_fragment_out_t **pRetFragOutList, uint8 ipProto, ipaddr_t srcAddr, ipaddr_t destAddr, uint16 identification, uint16 receiveLength)
{
	int ret=RG_RET_LOOKUPIDX_NOT_FOUND;
	uint32 naptHashOutIdx;
	//int naptInIdx;
	rtk_rg_ipv4_fragment_out_t *pFragOutList,*pNextFragList;
	rtk_rg_pkthdr_tagif_t layer4Type;

	if(ipProto==0x6)
		layer4Type=TCP_TAGIF;
	else if(ipProto==0x11)
		layer4Type=UDP_TAGIF;
	else
		layer4Type=ICMP_TAGIF;
	naptHashOutIdx=_rtk_rg_ipv4FragOutHashIndex(ipProto,srcAddr,destAddr,identification);
	//FIXME("the frag lookup index is %d",naptHashOutIdx);
	*hashIdx=naptHashOutIdx;
	//naptOutIdx=naptHashOutIdx<<2;

/*	
	for(i=naptOutIdx;i<naptOutIdx+4;i++)
	{
		if(rg_db.ipv4FragmentOutTable[i].valid && rg_db.ipv4FragmentOutTable[i].pNaptOutboundEntry->valid)
		{
			//DEBUG("get the napt index is %d",rg_db.ipv4FragmentOutTable[i].NaptOutboundEntryIndex);
			naptInIdx=rg_db.ipv4FragmentOutTable[i].pNaptOutboundEntry->hashIdx;
			//DEBUG("srcAddr is %x, the intIP is %x",srcAddr,rg_db.naptIn[naptInIdx].rtk_naptIn.intIp);
			//DEBUG("isTcp is %d, naptIn.isTcp is %d",isTcp,rg_db.naptIn[naptInIdx].rtk_naptIn.isTcp);
			//DEBUG("id is %x, fragoutTable id is %x",identification,rg_db.ipv4FragmentOutTable[i].identification);
			if((srcAddr==rg_db.naptIn[naptInIdx].rtk_naptIn.intIp)&&
				(isTcp==rg_db.naptIn[naptInIdx].rtk_naptIn.isTcp)&&
				(identification==rg_db.ipv4FragmentOutTable[i].identification))
			{
				ret=rg_db.ipv4FragmentOutTable[i].NaptOutboundEntryIndex;							
				*fragIdx = i;
				*aclAct = rg_db.ipv4FragmentOutTable[i].aclAction;
				//DEBUG("found naptOutIdx = %d, fragIdx = %d",ret,i);
				break;
			}
		}		
	}
*/
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragLock);
	pFragOutList=rg_db.pFragOutHashListHead[naptHashOutIdx];
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv4FragLock);
	//FIXME("after lock");
	while(pFragOutList!=NULL)
	{				
		pNextFragList=pFragOutList->pNext;
		if(pFragOutList->layer4Type&layer4Type)
		{
			if(layer4Type&ICMP_TAGIF)
			{
				if(identification==pFragOutList->identification &&
					srcAddr==pFragOutList->pktInfo.icmp.intIp)
				{
					*pRetFragOutList=pFragOutList;
					pFragOutList->beginIdleTime=jiffies;
					pFragOutList->receivedLength+=receiveLength;
					//*icmpCtrlFlow=pFragOutList->pktInfo.pICMPCtrlFlow;
					//DEBUG("found ICMPCtrlFlow = %p, ret=%d, received %d bytes",pFragOutList,ret,pFragOutList->receivedLength);
					//FIXME("get ICMP out list!");
					break;
				}
			}
			else	//TCP or UDP
			{
				if(identification==pFragOutList->identification &&
					srcAddr==pFragOutList->pktInfo.napt.intIp)
				{
					*pRetFragOutList=pFragOutList;
					pFragOutList->beginIdleTime=jiffies;
					pFragOutList->receivedLength+=receiveLength;
					ret=pFragOutList->pktInfo.napt.NaptOutboundEntryIndex;							
					//DEBUG("found naptOutIdx = %d, received %d bytes",ret,pFragOutList->receivedLength);
					//FIXME("get NAPT out list!");
					break;
				}			
			}
		}
		if(time_after_eq(jiffies,pFragOutList->beginIdleTime+(FRAGMENT_LIST_TIMEOUT*TICKTIME_PERIOD)))		//too old
		{
			//FIXME("free old %s %p",pFragOutList->pktType==FRAG_TYPE_ICMP?"ICMP":"NAPT",pFragOutList);
			//free it
			//------------------ Critical Section End -----------------------//
			//rg_unlock(&rg_kernel.ipv4FragLock);
			_rtk_rg_freeFragOutList(naptHashOutIdx,pFragOutList);

			//------------------ Critical Section start -----------------------//
			//rg_lock(&rg_kernel.ipv4FragLock);
		}
		
		//------------------ Critical Section start -----------------------//
		//rg_lock(&rg_kernel.ipv4FragLock);
		pFragOutList=pNextFragList;
		//------------------ Critical Section start -----------------------//
		//rg_unlock(&rg_kernel.ipv4FragLock);
	}
//FIXME("before unlock");
	//------------------ Critical Section End -----------------------//
	//rg_unlock(&rg_kernel.ipv4FragLock);
	
	return ret;
}

void _rtk_rg_fwdEngine_fillOutFragmentInfo(rtk_rg_fwdEngineReturn_t fragAction, unsigned int realNaptIdx, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_ipv4_fragment_out_t **pFragList)
{
	//int i;
	//int isTCP=0;
	uint32 naptHashOutIdx;
	//int32 naptOutIdx;
	rtk_rg_ipv4_fragment_out_t *pFragFreeOutList;
	rtk_rg_pkthdr_tagif_t layer4Type;
	
	if(pPktHdr->ipProtocol==0x6)
		layer4Type=TCP_TAGIF;
	else if(pPktHdr->ipProtocol==0x11)
		layer4Type=UDP_TAGIF;
	else
		layer4Type=ICMP_TAGIF;
	
	//then fill the fragment table
	naptHashOutIdx=_rtk_rg_ipv4FragOutHashIndex(pPktHdr->ipProtocol,pPktHdr->ipv4Sip,pPktHdr->ipv4Dip,ntohs(*pPktHdr->pIpv4Identification));
	//DEBUG("the fill out hash index is %d",naptHashOutIdx);
	//naptOutIdx=naptHashOutIdx<<2;
/*	
	for(i=naptOutIdx;i<naptOutIdx+4;i++)
	{
		if(rg_db.ipv4FragmentOutTable[i].valid==0)
		{
			//DEBUG("the add fragment table index is %d, napt index is %d",i,realNaptIdx);
			rg_db.ipv4FragmentOutTable[i].NaptOutboundEntryIndex=realNaptIdx;
			rg_db.ipv4FragmentOutTable[i].pNaptOutboundEntry=&rg_db.naptOut[realNaptIdx].rtk_naptOut;
			rg_db.ipv4FragmentOutTable[i].identification=ntohs(*(pPktHdr->pIpv4Identification));
			rg_db.ipv4FragmentOutTable[i].valid=1;
			*fragIdx=i;
			ret=SUCCESS;
			break;
		}
	}
*/
	_rtk_rg_getFragOutFreeList(&pFragFreeOutList);

	//DEBUG("fragAction is %d",fragAction);
	if(layer4Type&ICMP_TAGIF)	//ICMP
	{
		DEBUG("record icmp ctrl flow id=%x, intIp=%x",pPktHdr->ICMPIdentifier,pPktHdr->ipv4Sip);
		//pFragFreeOutList->pktInfo.icmp.identification=pPktHdr->ICMPIdentifier;
		pFragFreeOutList->pktInfo.icmp.intIp=pPktHdr->ipv4Sip;
	}
	else if((layer4Type&TCP_TAGIF)||(layer4Type&UDP_TAGIF))		//TCP or UDP
	{
		DEBUG("the add fragment table hashIdx is %d, napt index is %d",naptHashOutIdx,realNaptIdx);
		pFragFreeOutList->pktInfo.napt.intIp=pPktHdr->ipv4Sip;
		pFragFreeOutList->pktInfo.napt.NaptOutboundEntryIndex=realNaptIdx;
		//pFragFreeOutList->pktInfo.napt.pNaptOutboundEntry=&rg_db.naptOut[realNaptIdx].rtk_naptOut;
		//pFragFreeOutList->pktInfo.napt.identification=ntohs(*(pPktHdr->pIpv4Identification));
	}
	*pFragList = pFragFreeOutList;
	pFragFreeOutList->layer4Type=layer4Type;
	pFragFreeOutList->fragAction=fragAction;
	pFragFreeOutList->beginIdleTime=jiffies;
	pFragFreeOutList->identification=ntohs(*(pPktHdr->pIpv4Identification));
	pFragFreeOutList->receivedLength=pPktHdr->l3Len-pPktHdr->ipv4HeaderLen;
	if(!pPktHdr->ipv4MoreFragment)
		pFragFreeOutList->totalLength=(pPktHdr->ipv4FragmentOffset<<3)+pFragFreeOutList->receivedLength;
	else
		pFragFreeOutList->totalLength=0;
	//DEBUG("totalLength is %d, receivedLength is %d",pFragFreeOutList->totalLength,pFragFreeOutList->receivedLength);
	pFragFreeOutList->queueCount=0;

	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragLock);
	if(rg_db.pFragOutHashListHead[naptHashOutIdx]==NULL)
	{
		rg_db.pFragOutHashListHead[naptHashOutIdx]=pFragFreeOutList;
	}
	else
	{		
		rg_db.pFragOutHashListHead[naptHashOutIdx]->pPrev=pFragFreeOutList;
		pFragFreeOutList->pNext=rg_db.pFragOutHashListHead[naptHashOutIdx];
		rg_db.pFragOutHashListHead[naptHashOutIdx]=pFragFreeOutList;	
	}
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv4FragLock);


}

rtk_rg_lookupIdxReturn_t _rtk_rg_fwdEngine_fragmentInHashIndexLookup(int *hashIdx, rtk_rg_ipv4_fragment_in_t **pRetFragInList, uint8 ipProto, ipaddr_t srcAddr, ipaddr_t destAddr, uint16 identification, uint16 receiveLength)
{
	int ret=RG_RET_LOOKUPIDX_NOT_FOUND;
	uint32 naptHashInIdx;
	//int naptOutIdx;
	rtk_rg_ipv4_fragment_in_t *pFragInList,*pNextFragList;
	rtk_rg_pkthdr_tagif_t layer4Type;

	if(ipProto==0x6)
		layer4Type=TCP_TAGIF;
	else if(ipProto==0x11)
		layer4Type=UDP_TAGIF;
	else
		layer4Type=ICMP_TAGIF;
	
	naptHashInIdx=_rtk_rg_ipv4FragInHashIndex(ipProto,destAddr,identification);
	//DEBUG("the inbound frag lookup index is %d",naptHashInIdx);
	*hashIdx=naptHashInIdx;
	//naptOutIdx=naptHashOutIdx<<2;

/*	
	for(i=naptOutIdx;i<naptOutIdx+4;i++)
	{
		if(rg_db.ipv4FragmentOutTable[i].valid && rg_db.ipv4FragmentOutTable[i].pNaptOutboundEntry->valid)
		{
			//DEBUG("get the napt index is %d",rg_db.ipv4FragmentOutTable[i].NaptOutboundEntryIndex);
			naptInIdx=rg_db.ipv4FragmentOutTable[i].pNaptOutboundEntry->hashIdx;
			//DEBUG("srcAddr is %x, the intIP is %x",srcAddr,rg_db.naptIn[naptInIdx].rtk_naptIn.intIp);
			//DEBUG("isTcp is %d, naptIn.isTcp is %d",isTcp,rg_db.naptIn[naptInIdx].rtk_naptIn.isTcp);
			//DEBUG("id is %x, fragoutTable id is %x",identification,rg_db.ipv4FragmentOutTable[i].identification);
			if((srcAddr==rg_db.naptIn[naptInIdx].rtk_naptIn.intIp)&&
				(isTcp==rg_db.naptIn[naptInIdx].rtk_naptIn.isTcp)&&
				(identification==rg_db.ipv4FragmentOutTable[i].identification))
			{
				ret=rg_db.ipv4FragmentOutTable[i].NaptOutboundEntryIndex;							
				*fragIdx = i;
				*aclAct = rg_db.ipv4FragmentOutTable[i].aclAction;
				//DEBUG("found naptOutIdx = %d, fragIdx = %d",ret,i);
				break;
			}
		}		
	}
*/	
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragLock);
	pFragInList=rg_db.pFragInHashListHead[naptHashInIdx];
	//------------------ Critical Section start -----------------------//
	rg_unlock(&rg_kernel.ipv4FragLock);
	
	while(pFragInList!=NULL)
	{
		pNextFragList=pFragInList->pNext;
		if(pFragInList->layer4Type&layer4Type)
		{
			if(pFragInList->layer4Type&ICMP_TAGIF)
			{
				if(identification==pFragInList->identification&&
					srcAddr==pFragInList->pktInfo.icmp.remoteIp)
				{
					*pRetFragInList=pFragInList;
					pFragInList->beginIdleTime=jiffies;
					pFragInList->receivedLength+=receiveLength;
					//*icmpCtrlFlow=pFragInList->pktInfo.pICMPCtrlFlow;
					DEBUG("found ICMPCtrlFlow, ret= %d, action=%d, received %d bytes",ret,pFragInList->fragAction,pFragInList->receivedLength);
					break;
				}
			}
			else //TCP or UDP
			{
				if(identification==pFragInList->identification &&
					srcAddr==pFragInList->pktInfo.napt.remoteIp)
				{
					*pRetFragInList=pFragInList;
					pFragInList->beginIdleTime=jiffies;
					pFragInList->receivedLength+=receiveLength;
					ret=pFragInList->pktInfo.napt.NaptOutboundEntryIndex;
					DEBUG("found naptOutIdx = %d, action=%d, received %d bytes",ret,pFragInList->fragAction,pFragInList->receivedLength);
					break;
				}
			}
		}
		if(time_after_eq(jiffies,pFragInList->beginIdleTime+(FRAGMENT_LIST_TIMEOUT*TICKTIME_PERIOD)))		//too old
		{
			FIXME("free old %s %p",pFragInList->layer4Type&ICMP_TAGIF?"ICMP":"NAPT",pFragInList);
			//free it
			//------------------ Critical Section End -----------------------//
			//rg_unlock(&rg_kernel.ipv4FragLock);
			
			_rtk_rg_freeFragInList(naptHashInIdx,pFragInList);

			//------------------ Critical Section start -----------------------//
			//rg_lock(&rg_kernel.ipv4FragLock);
		}
		//------------------ Critical Section start -----------------------//
		//rg_lock(&rg_kernel.ipv4FragLock);
		pFragInList=pNextFragList;
		//------------------ Critical Section End -----------------------//
		//rg_unlock(&rg_kernel.ipv4FragLock);
	}
	
	//------------------ Critical Section End -----------------------//
	//rg_unlock(&rg_kernel.ipv4FragLock);
	
	return ret;
}

void _rtk_rg_fwdEngine_fillInFragmentInfo(rtk_rg_fwdEngineReturn_t fragAction, int realNaptOutIdx, rtk_rg_table_icmp_flow_t *icmpCtrlFlow, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_ipv4_fragment_in_t **pFragList)
{
	//int i;
	uint32 naptHashInIdx;
	//int32 naptOutIdx;
	rtk_rg_ipv4_fragment_in_t *pFragFreeInList;
	rtk_rg_pkthdr_tagif_t layer4Type;

	if(pPktHdr->ipProtocol==0x6)
		layer4Type=TCP_TAGIF;
	else if(pPktHdr->ipProtocol==0x11)
		layer4Type=UDP_TAGIF;
	else
		layer4Type=ICMP_TAGIF;
	
	//then fill the fragment table
	naptHashInIdx=_rtk_rg_ipv4FragInHashIndex(pPktHdr->ipProtocol,pPktHdr->ipv4Dip,ntohs(*pPktHdr->pIpv4Identification));
	//DEBUG("the fill in hash index is %d",naptHashInIdx);
	//naptOutIdx=naptHashOutIdx<<2;
	
/*	
	for(i=naptOutIdx;i<naptOutIdx+4;i++)
	{
		if(rg_db.ipv4FragmentOutTable[i].valid==0)
		{
			//DEBUG("the add fragment table index is %d, napt index is %d",i,realNaptIdx);
			rg_db.ipv4FragmentOutTable[i].NaptOutboundEntryIndex=realNaptIdx;
			rg_db.ipv4FragmentOutTable[i].pNaptOutboundEntry=&rg_db.naptOut[realNaptIdx].rtk_naptOut;
			rg_db.ipv4FragmentOutTable[i].identification=ntohs(*(pPktHdr->pIpv4Identification));
			rg_db.ipv4FragmentOutTable[i].valid=1;
			*fragIdx=i;
			ret=SUCCESS;
			break;
		}
	}
*/
	_rtk_rg_getFragInFreeList(&pFragFreeInList);

	DEBUG("fragAction is %d",fragAction);
	if(layer4Type&ICMP_TAGIF)		//ICMP
	{
		DEBUG("record icmp ctrl flow id=%x, remoteIp=%x,internalIp=%x",pPktHdr->ICMPIdentifier,pPktHdr->ipv4Sip,(icmpCtrlFlow!=NULL?icmpCtrlFlow->internalIP:0x0));
		//pFragFreeInList->pktInfo.icmp.identification=pPktHdr->ICMPIdentifier;
		pFragFreeInList->pktInfo.icmp.remoteIp=pPktHdr->ipv4Sip;
		if(icmpCtrlFlow!=NULL)pFragFreeInList->pktInfo.icmp.intIp=icmpCtrlFlow->internalIP;
	}
	else if((layer4Type&TCP_TAGIF)||(layer4Type&UDP_TAGIF))		//TCP or UDP 
	{
		DEBUG("the add fragment table hashindex is %d, napt index is %d",naptHashInIdx,realNaptOutIdx);
		pFragFreeInList->pktInfo.napt.remoteIp=pPktHdr->ipv4Sip;
		//pFragFreeInList->pktInfo.napt.NaptInboundEntryIndex=realNaptInIdx;
		pFragFreeInList->pktInfo.napt.NaptOutboundEntryIndex=realNaptOutIdx;		//FAIL if napt is not exist
		//pFragFreeInList->pktInfo.napt.pNaptInboundEntry=&rg_db.naptIn[realNaptInIdx].rtk_naptIn;
	}
	*pFragList = pFragFreeInList;
	pFragFreeInList->layer4Type=layer4Type;
	pFragFreeInList->fragAction=fragAction;
	pFragFreeInList->beginIdleTime=jiffies;
	pFragFreeInList->identification=ntohs(*(pPktHdr->pIpv4Identification));
	pFragFreeInList->receivedLength=pPktHdr->l3Len-pPktHdr->ipv4HeaderLen;
	if(!pPktHdr->ipv4MoreFragment)
		pFragFreeInList->totalLength=(pPktHdr->ipv4FragmentOffset<<3)+pFragFreeInList->receivedLength;
	else
		pFragFreeInList->totalLength=0;
	//DEBUG("totalLength is %d, receivedLength is %d",pFragFreeInList->totalLength,pFragFreeInList->receivedLength);
	pFragFreeInList->queueCount=0;
	
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv4FragLock);
	if(rg_db.pFragInHashListHead[naptHashInIdx]==NULL)
	{
		rg_db.pFragInHashListHead[naptHashInIdx]=pFragFreeInList;
	}
	else
	{
		rg_db.pFragInHashListHead[naptHashInIdx]->pPrev=pFragFreeInList;
		pFragFreeInList->pNext=rg_db.pFragInHashListHead[naptHashInIdx];
		rg_db.pFragInHashListHead[naptHashInIdx]=pFragFreeInList;	
	}
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv4FragLock);

}

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
void _rtk_rg_fwdEngine_ipv6ConnList_del(rtk_rg_ipv6_layer4_linkList_t *pConnList)
{
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	int ret;
#endif
	//this function do not protect by semaphore, please take care at outside caller function!!
	rtk_rg_ipv6_layer4_linkList_t *pPairConnList=pConnList->pPair_list;
	//DEBUG("[Before pair del]: pConnList->prev=%p, pConnList->next=%p",pConnList->layer4_list.prev,pConnList->layer4_list.next);
	if(pPairConnList!=NULL)
	{

		DEBUG("Delete LIST[%p]'s PAIR[%p],too!!",pConnList,pPairConnList);
		//TRACE("Delete LIST[%p]'s PAIR[%p],too!!",pConnList,pPairConnList);
		//DEBUG("[Before pair del]: pPairConnList->prev=%p, pPairConnList->next=%p",pPairConnList->layer4_list.prev,pPairConnList->layer4_list.next);
		list_del_init(&pPairConnList->layer4_list);
		//DEBUG("[After pair del]: pConnList->prev=%p, pConnList->next=%p",pConnList->layer4_list.prev,pConnList->layer4_list.next);
		
		pPairConnList->state=INVALID;
		pPairConnList->valid=0;
		pPairConnList->pPair_list=NULL;
		atomic_dec(&rg_db.systemGlobal.v6StatefulConnectionNum);
		
		//Add back to free list
		list_add(&pPairConnList->layer4_list,&rg_db.ipv6Layer4FreeListHead);


#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
		if(pConnList->direction==NAPT_DIRECTION_OUTBOUND || pConnList->direction==NAPT_DIRECTION_INBOUND){
			ret=_rtk_rg_ipv6_naptExtPortFree(pConnList->isTCP,pConnList->externalPort);
			if(ret!=RG_RET_SUCCESS)
				DEBUG("_rtk_rg_ipv6_naptExtPortFree FAIL!");
		}
#endif
		
	}

	//Force replace the victim and it's pair
	list_del_init(&pConnList->layer4_list);
	pConnList->state=INVALID;
	pConnList->valid=0;
	pConnList->pPair_list=NULL;
	atomic_dec(&rg_db.systemGlobal.v6StatefulConnectionNum);
	//Add back to free list
	list_add(&pConnList->layer4_list,&rg_db.ipv6Layer4FreeListHead);
}

rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_ipv6ConnList_LRU(uint32 hashIndex)
{
	int i,checkHash=hashIndex,longest_idle=-1;
	rtk_rg_ipv6_layer4_linkList_t *pTmpList,*pLongestIdleList=NULL;
	
	for(i=0;i<MAX_IPV6_STATEFUL_HASH_HEAD_SIZE;i++)
	{
		if(!list_empty(&rg_db.ipv6Layer4HashListHead[checkHash]))
		{
			list_for_each_entry(pTmpList,&rg_db.ipv6Layer4HashListHead[checkHash],layer4_list)
			{
				WARNING("pTmpList[%p]->idleSecs is %d",pTmpList,pTmpList->idleSecs);
				//Lookup for the longest idleSecs
				if(longest_idle<0 || pTmpList->idleSecs>longest_idle)
				{
					WARNING("pTmpList[%p] is longest idle!!",pTmpList);
					pLongestIdleList=pTmpList;
					longest_idle=pTmpList->idleSecs;
				}
			}
			WARNING("FORCED REPLACE LIST[%p] in hash[%d]!!",pLongestIdleList,checkHash);
			_rtk_rg_fwdEngine_ipv6ConnList_del(pLongestIdleList);
			
			return RG_RET_SUCCESS;
		}
		checkHash++;
		if(checkHash>=MAX_IPV6_STATEFUL_HASH_HEAD_SIZE)checkHash=0;
	}
	return RG_RET_FAIL;
}

rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_ipv6ConnList_get(rtk_rg_ipv6_layer4_linkList_t **pIPv6ConnList, uint32 hashIndex)
{
	rtk_rg_ipv6_layer4_linkList_t *pListEntry,*pListNextEntry;

	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv6StatefulLock);
	
	//Check if we have not-used free arp list
	if(list_empty(&rg_db.ipv6Layer4FreeListHead))
	{
		WARNING("all free IPv6 ConnList are allocated...LRU is needed!!");
		if(_rtk_rg_fwdEngine_ipv6ConnList_LRU(hashIndex)!=RG_RET_SUCCESS)
		{
			//------------------ Critical Section End -----------------------//
			rg_unlock(&rg_kernel.ipv6StatefulLock);
			return RG_RET_FAIL;
		}
	}

	//Get one from free list
	list_for_each_entry_safe(pListEntry,pListNextEntry,&rg_db.ipv6Layer4FreeListHead,layer4_list) 	//just return the first entry right behind of head
	{	
		list_del_init(&pListEntry->layer4_list);
		break;
	}
	DEBUG("the free IPv6 ConnList %p",pListEntry);

	atomic_inc(&rg_db.systemGlobal.v6StatefulConnectionNum);
	//Add to hash head list
	list_add(&pListEntry->layer4_list,&rg_db.ipv6Layer4HashListHead[hashIndex]);
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv6StatefulLock);
	
	*pIPv6ConnList=pListEntry;
	
	return RG_RET_SUCCESS;
}

void _rtk_rg_fwdEngine_ipv6ConnList_lookup(rtk_rg_ipv6_layer4_linkList_t **pIPv6ConnList, int32 *pHashIndex, uint8 *srcIP, uint8 *destIP, uint16 srcPort, uint16 destPort, int isTCP, int isFrag)
{
	rtk_rg_ipv6_layer4_linkList_t *pTempList;

	//DEBUG("pHashIndex before search = %d",*pHashIndex);
	if(*pHashIndex<0)
	{
		if(isTCP)
			*pHashIndex=_rtk_rg_ipv6StatefulHashIndex(0x6,srcIP,destIP,srcPort,destPort);
		else
			*pHashIndex=_rtk_rg_ipv6StatefulHashIndex(0x11,srcIP,destIP,srcPort,destPort);
	}
	
	DEBUG(" srcIP %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x   Sport:%d",
		srcIP[0],srcIP[1],srcIP[2],srcIP[3],
		srcIP[4],srcIP[5],srcIP[6],srcIP[7],
		srcIP[8],srcIP[9],srcIP[10],srcIP[11],
		srcIP[12],srcIP[13],srcIP[14],srcIP[15],
		srcPort);

	DEBUG(" destIP %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x   Dport:%d",
		destIP[0],destIP[1],destIP[2],destIP[3],
		destIP[4],destIP[5],destIP[6],destIP[7],
		destIP[8],destIP[9],destIP[10],destIP[11],
		destIP[12],destIP[13],destIP[14],destIP[15],
		destPort);
	DEBUG(" isTcp=%d  isFrag=%d",isTCP,isFrag);
	

	TRACE("hash idx is %d",*pHashIndex);
	//*pIPv6ConnList=NULL;
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv6StatefulLock);
	if(!list_empty(&rg_db.ipv6Layer4HashListHead[*pHashIndex]))
	{
		//TRACE("head[%d] is not empty!",*pHashIndex);
		list_for_each_entry(pTempList,&rg_db.ipv6Layer4HashListHead[*pHashIndex],layer4_list)
		{
			//TRACE("Comparing pTempList is %p, srcport:%d, destport:%d",pTempList,pTempList->srcPort,pTempList->destPort);
			
			DEBUG(" pTempList.srcIP %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x   pTempList->Sport:%d",
				pTempList->srcIP.ipv6_addr[0],pTempList->srcIP.ipv6_addr[1],pTempList->srcIP.ipv6_addr[2],pTempList->srcIP.ipv6_addr[3],
				pTempList->srcIP.ipv6_addr[4],pTempList->srcIP.ipv6_addr[5],pTempList->srcIP.ipv6_addr[6],pTempList->srcIP.ipv6_addr[7],
				pTempList->srcIP.ipv6_addr[8],pTempList->srcIP.ipv6_addr[9],pTempList->srcIP.ipv6_addr[10],pTempList->srcIP.ipv6_addr[11],
				pTempList->srcIP.ipv6_addr[12],pTempList->srcIP.ipv6_addr[13],pTempList->srcIP.ipv6_addr[14],pTempList->srcIP.ipv6_addr[15],
				pTempList->srcPort);	
			
			DEBUG(" pTempList.destIP %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x   pTempList->Dport:%d",
				pTempList->destIP.ipv6_addr[0],pTempList->destIP.ipv6_addr[1],pTempList->destIP.ipv6_addr[2],pTempList->destIP.ipv6_addr[3],
				pTempList->destIP.ipv6_addr[4],pTempList->destIP.ipv6_addr[5],pTempList->destIP.ipv6_addr[6],pTempList->destIP.ipv6_addr[7],
				pTempList->destIP.ipv6_addr[8],pTempList->destIP.ipv6_addr[9],pTempList->destIP.ipv6_addr[10],pTempList->destIP.ipv6_addr[11],
				pTempList->destIP.ipv6_addr[12],pTempList->destIP.ipv6_addr[13],pTempList->destIP.ipv6_addr[14],pTempList->destIP.ipv6_addr[15],
				pTempList->destPort);
			
			DEBUG(" pTempList.notFinishUpdated=%d",pTempList->notFinishUpdated);	
			
			
			if(pTempList->isTCP==isTCP && pTempList->notFinishUpdated==0 && pTempList->isFrag==isFrag &&
				!memcmp(pTempList->srcIP.ipv6_addr,srcIP,IPV6_ADDR_LEN) &&
				!memcmp(pTempList->destIP.ipv6_addr,destIP,IPV6_ADDR_LEN) &&
				pTempList->srcPort==srcPort && pTempList->destPort==destPort)
			{
				//Hit!!
				*pIPv6ConnList=pTempList;
				TRACE("Found *pIPv6ConnList is %p",*pIPv6ConnList);
				break;
			}
		}
		
	}
	TRACE("end of ipv6ConnList lookup..");
	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv6StatefulLock);
}

rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_fillIpv6StatefulInfo(rtk_rg_ipv6_layer4_linkList_t **pIPv6ConnList, uint32 hashIndex, rtk_rg_pktHdr_t *pPktHdr)
{		
	int isTCP=0;
	rtk_rg_successFailReturn_t ret;
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	int extport=0;
	int32	pairboundhashIndex;
	rtk_rg_ipv6_layer4_linkList_t *pPairboundIPv6ConnList=NULL;
#endif

	if(pPktHdr->tagif&TCP_TAGIF) isTCP=1;

	ret=_rtk_rg_fwdEngine_ipv6ConnList_get(pIPv6ConnList,hashIndex);
	if(ret==RG_RET_FAIL)return RG_RET_FAIL;

	//fill all information abount this connection
	memcpy((*pIPv6ConnList)->srcIP.ipv6_addr,pPktHdr->pIpv6Sip,IPV6_ADDR_LEN);
	memcpy((*pIPv6ConnList)->destIP.ipv6_addr,pPktHdr->pIpv6Dip,IPV6_ADDR_LEN);
	(*pIPv6ConnList)->srcPort=pPktHdr->sport;
	(*pIPv6ConnList)->destPort=pPktHdr->dport;
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	if(pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPT){
		extport = _rtk_rg_ipv6_naptExtPortGetAndUse(isTCP,pPktHdr->sport);
		memcpy((*pIPv6ConnList)->internalIP.ipv6_addr,pPktHdr->pIpv6Sip,IPV6_ADDR_LEN);
		(*pIPv6ConnList)->externalPort=extport;
		(*pIPv6ConnList)->internalPort=pPktHdr->sport;
		(*pIPv6ConnList)->extipIdx=pPktHdr->extipIdx;
		(*pIPv6ConnList)->direction=NAPT_DIRECTION_OUTBOUND; //???
	}else if(pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPTR){
		extport = _rtk_rg_ipv6_naptExtPortGetAndUse(isTCP,pPktHdr->dport);
		memcpy((*pIPv6ConnList)->internalIP.ipv6_addr,pPktHdr->ipv6_serverInLanLookup.transIP.ipv6_addr,IPV6_ADDR_LEN);
		(*pIPv6ConnList)->externalPort=pPktHdr->dport;
		(*pIPv6ConnList)->internalPort=pPktHdr->ipv6_serverInLanLookup.transPort;
		(*pIPv6ConnList)->extipIdx=pPktHdr->extipIdx;
		(*pIPv6ConnList)->direction=NAPT_DIRECTION_INBOUND; //???
	}
#endif
	(*pIPv6ConnList)->isTCP=isTCP;
	(*pIPv6ConnList)->state=INVALID;
	(*pIPv6ConnList)->idleSecs=0;
	(*pIPv6ConnList)->pPair_list=NULL;
	(*pIPv6ConnList)->valid=1;
	(*pIPv6ConnList)->notFinishUpdated=1;	//not finished 

	(*pIPv6ConnList)->isFrag=0;
	(*pIPv6ConnList)->fragAction=RG_FWDENGINE_RET_UN_INIT;
	(*pIPv6ConnList)->beginIdleTime=jiffies;
	(*pIPv6ConnList)->queueCount=0;
	(*pIPv6ConnList)->receivedLength=0;
	(*pIPv6ConnList)->totalLength=0;
	(*pIPv6ConnList)->netifIdx=pPktHdr->netifIdx;
	(*pIPv6ConnList)->dmacL2Idx=pPktHdr->dmacL2Idx;
	//20150922LUKE: keep Neighbor idx in stateful for updating.
	(*pIPv6ConnList)->neighborIdx=_rtk_rg_shortcutNEIGHBORFind(pPktHdr->pIpv6Sip);
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
	(*pIPv6ConnList)->smacL2Idx=pPktHdr->smacL2Idx;
#endif



#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	//if NAPT, also fill inipvInboundConn!
	if(pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPT){

			if(isTCP){
				pairboundhashIndex=_rtk_rg_ipv6StatefulHashIndex(0x6,pPktHdr->pIpv6Dip,rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr,pPktHdr->dport,extport);
			}else{
				pairboundhashIndex=_rtk_rg_ipv6StatefulHashIndex(0x11,pPktHdr->pIpv6Dip,rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr,pPktHdr->dport,extport);
			}	
			//DEBUG("inboundhashIndex=%d",inboundhashIndex);	
			ret=_rtk_rg_fwdEngine_ipv6ConnList_get(&pPairboundIPv6ConnList,pairboundhashIndex);
			if(ret==RG_RET_FAIL)return RG_RET_FAIL;
	
			//fill all information abount this connection
			memcpy((pPairboundIPv6ConnList)->srcIP.ipv6_addr,pPktHdr->pIpv6Dip,IPV6_ADDR_LEN);
			memcpy((pPairboundIPv6ConnList)->destIP.ipv6_addr,rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr,IPV6_ADDR_LEN);
			memcpy((pPairboundIPv6ConnList)->internalIP.ipv6_addr,pPktHdr->pIpv6Sip,IPV6_ADDR_LEN);
			(pPairboundIPv6ConnList)->srcPort=pPktHdr->dport;
			(pPairboundIPv6ConnList)->destPort=extport;
			(pPairboundIPv6ConnList)->internalPort=pPktHdr->sport;
			(pPairboundIPv6ConnList)->externalPort=extport;
			(pPairboundIPv6ConnList)->extipIdx=pPktHdr->extipIdx;
			(pPairboundIPv6ConnList)->isTCP=isTCP;
			(pPairboundIPv6ConnList)->state=INVALID;
			(pPairboundIPv6ConnList)->idleSecs=0;
			(pPairboundIPv6ConnList)->valid=1;
			(pPairboundIPv6ConnList)->isFrag=0;
			(pPairboundIPv6ConnList)->fragAction=RG_FWDENGINE_RET_UN_INIT;
			(pPairboundIPv6ConnList)->beginIdleTime=0;
			(pPairboundIPv6ConnList)->queueCount=0;
			(pPairboundIPv6ConnList)->netifIdx=pPktHdr->srcNetifIdx;
			(pPairboundIPv6ConnList)->dmacL2Idx=pPktHdr->smacL2Idx;
			//20151012LUKE: keep Neighbor idx in stateful for updating.
			(pPairboundIPv6ConnList)->neighborIdx=_rtk_rg_shortcutNEIGHBORFind(pPktHdr->pIpv6Dip);
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
			(pPairboundIPv6ConnList)->smacL2Idx=pPktHdr->dmacL2Idx;
#endif
			(pPairboundIPv6ConnList)->pPair_list=(*pIPv6ConnList);
			(pPairboundIPv6ConnList)->direction=NAPT_DIRECTION_INBOUND;  
			(pPairboundIPv6ConnList)->notFinishUpdated=0;	//finished, just create for inbound 

			//set outbound pIPv6ConnList pPair_list
			(*pIPv6ConnList)->pPair_list=pPairboundIPv6ConnList;
			
	}else if(pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPTR){
		if(isTCP){
			pairboundhashIndex=_rtk_rg_ipv6StatefulHashIndex(0x6,pPktHdr->ipv6_serverInLanLookup.transIP.ipv6_addr,pPktHdr->pIpv6Sip,pPktHdr->ipv6_serverInLanLookup.transPort,pPktHdr->sport);
		}else{
			pairboundhashIndex=_rtk_rg_ipv6StatefulHashIndex(0x11,pPktHdr->ipv6_serverInLanLookup.transIP.ipv6_addr,pPktHdr->pIpv6Sip,pPktHdr->ipv6_serverInLanLookup.transPort,pPktHdr->sport);
		}		
		
		//DEBUG("inboundhashIndex=%d",inboundhashIndex);	
		ret=_rtk_rg_fwdEngine_ipv6ConnList_get(&pPairboundIPv6ConnList,pairboundhashIndex);
		if(ret==RG_RET_FAIL)return RG_RET_FAIL;
		
		//fill all information about this connection
		memcpy((pPairboundIPv6ConnList)->srcIP.ipv6_addr,pPktHdr->ipv6_serverInLanLookup.transIP.ipv6_addr,IPV6_ADDR_LEN);
		memcpy((pPairboundIPv6ConnList)->destIP.ipv6_addr,pPktHdr->pIpv6Sip,IPV6_ADDR_LEN);
		memcpy((pPairboundIPv6ConnList)->internalIP.ipv6_addr,pPktHdr->ipv6_serverInLanLookup.transIP.ipv6_addr,IPV6_ADDR_LEN);
		(pPairboundIPv6ConnList)->srcPort=pPktHdr->ipv6_serverInLanLookup.transPort;
		(pPairboundIPv6ConnList)->destPort=pPktHdr->sport;
		(pPairboundIPv6ConnList)->internalPort=pPktHdr->ipv6_serverInLanLookup.transPort;
		(pPairboundIPv6ConnList)->externalPort=extport;
		(pPairboundIPv6ConnList)->extipIdx=pPktHdr->extipIdx;//is this valid when outbound?  
		(pPairboundIPv6ConnList)->isTCP=isTCP;
		(pPairboundIPv6ConnList)->state=INVALID;
		(pPairboundIPv6ConnList)->idleSecs=0;
		(pPairboundIPv6ConnList)->valid=1;
		(pPairboundIPv6ConnList)->isFrag=0;
		(pPairboundIPv6ConnList)->fragAction=RG_FWDENGINE_RET_UN_INIT;
		(pPairboundIPv6ConnList)->beginIdleTime=0;
		(pPairboundIPv6ConnList)->queueCount=0;
		(pPairboundIPv6ConnList)->netifIdx=pPktHdr->netifIdx;//is this valid when outbound?  
		(pPairboundIPv6ConnList)->dmacL2Idx=pPktHdr->smacL2Idx; 
		//20151012LUKE: keep Neighbor idx in stateful for updating.
		(pPairboundIPv6ConnList)->neighborIdx=_rtk_rg_shortcutNEIGHBORFind(pPktHdr->ipv6_serverInLanLookup.transIP.ipv6_addr);
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
		(pPairboundIPv6ConnList)->smacL2Idx=pPktHdr->dmacL2Idx;
#endif
		(pPairboundIPv6ConnList)->pPair_list=(*pIPv6ConnList);
		(pPairboundIPv6ConnList)->direction=NAPT_DIRECTION_OUTBOUND; 
		(pPairboundIPv6ConnList)->notFinishUpdated=0;	//finished, just create for inbound 
		
		//set outbound pIPv6ConnList pPair_list
		(*pIPv6ConnList)->pPair_list=pPairboundIPv6ConnList;

	}
	
	
	//DEBUG("pPktHdr->extipIdx=%d",pPktHdr->extipIdx);
	TABLE("Add IPv6 Connection(valid=%d, state=%d, isFrag=%d, direction=%d, neighbor=%d) ==> from SIP:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x SPort:%d (extIP:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x extPort:%d)--> DIP: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x DPort:%d\n",
		(*pIPv6ConnList)->valid,(*pIPv6ConnList)->state,(*pIPv6ConnList)->isFrag,(*pIPv6ConnList)->direction,(*pIPv6ConnList)->neighborIdx,
		(*pIPv6ConnList)->srcIP.ipv6_addr[0],(*pIPv6ConnList)->srcIP.ipv6_addr[1],(*pIPv6ConnList)->srcIP.ipv6_addr[2],(*pIPv6ConnList)->srcIP.ipv6_addr[3],
		(*pIPv6ConnList)->srcIP.ipv6_addr[4],(*pIPv6ConnList)->srcIP.ipv6_addr[5],(*pIPv6ConnList)->srcIP.ipv6_addr[6],(*pIPv6ConnList)->srcIP.ipv6_addr[7],
		(*pIPv6ConnList)->srcIP.ipv6_addr[8],(*pIPv6ConnList)->srcIP.ipv6_addr[9],(*pIPv6ConnList)->srcIP.ipv6_addr[10],(*pIPv6ConnList)->srcIP.ipv6_addr[11],
		(*pIPv6ConnList)->srcIP.ipv6_addr[12],(*pIPv6ConnList)->srcIP.ipv6_addr[13],(*pIPv6ConnList)->srcIP.ipv6_addr[14],(*pIPv6ConnList)->srcIP.ipv6_addr[15],
		(*pIPv6ConnList)->srcPort,
		rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[0],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[1],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[2],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[3],
		rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[4],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[5],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[6],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[7],
		rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[8],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[9],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[10],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[11],
		rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[12],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[13],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[14],rg_db.v6Extip[pPktHdr->extipIdx].externalIp.ipv6_addr[15],
		(*pIPv6ConnList)->externalPort,
		(*pIPv6ConnList)->destIP.ipv6_addr[0],(*pIPv6ConnList)->destIP.ipv6_addr[1],(*pIPv6ConnList)->destIP.ipv6_addr[2],(*pIPv6ConnList)->destIP.ipv6_addr[3],
		(*pIPv6ConnList)->destIP.ipv6_addr[4],(*pIPv6ConnList)->destIP.ipv6_addr[5],(*pIPv6ConnList)->destIP.ipv6_addr[6],(*pIPv6ConnList)->destIP.ipv6_addr[7],
		(*pIPv6ConnList)->destIP.ipv6_addr[8],(*pIPv6ConnList)->destIP.ipv6_addr[9],(*pIPv6ConnList)->destIP.ipv6_addr[10],(*pIPv6ConnList)->destIP.ipv6_addr[11],
		(*pIPv6ConnList)->destIP.ipv6_addr[12],(*pIPv6ConnList)->destIP.ipv6_addr[13],(*pIPv6ConnList)->destIP.ipv6_addr[14],(*pIPv6ConnList)->destIP.ipv6_addr[15],
		(*pIPv6ConnList)->destPort);

	if(pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPT || pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPTR ){
		TABLE("Add IPv6 pair Connection(valid=%d, state=%d, isFrag=%d, direction=%d, neighbor=%d) ==> from SIP:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x SPort:%d --> GatewaIP:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x extPort:%d (internalIP:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x internalPort:%d)\n",
			(pPairboundIPv6ConnList)->valid,(pPairboundIPv6ConnList)->state,(pPairboundIPv6ConnList)->isFrag,(pPairboundIPv6ConnList)->direction,(pPairboundIPv6ConnList)->neighborIdx,
			(pPairboundIPv6ConnList)->srcIP.ipv6_addr[0],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[1],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[2],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[3],
			(pPairboundIPv6ConnList)->srcIP.ipv6_addr[4],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[5],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[6],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[7],
			(pPairboundIPv6ConnList)->srcIP.ipv6_addr[8],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[9],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[10],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[11],
			(pPairboundIPv6ConnList)->srcIP.ipv6_addr[12],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[13],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[14],(pPairboundIPv6ConnList)->srcIP.ipv6_addr[15],
			(pPairboundIPv6ConnList)->srcPort,
			(pPairboundIPv6ConnList)->destIP.ipv6_addr[0],(pPairboundIPv6ConnList)->destIP.ipv6_addr[1],(pPairboundIPv6ConnList)->destIP.ipv6_addr[2],(pPairboundIPv6ConnList)->destIP.ipv6_addr[3],
			(pPairboundIPv6ConnList)->destIP.ipv6_addr[4],(pPairboundIPv6ConnList)->destIP.ipv6_addr[5],(pPairboundIPv6ConnList)->destIP.ipv6_addr[6],(pPairboundIPv6ConnList)->destIP.ipv6_addr[7],
			(pPairboundIPv6ConnList)->destIP.ipv6_addr[8],(pPairboundIPv6ConnList)->destIP.ipv6_addr[9],(pPairboundIPv6ConnList)->destIP.ipv6_addr[10],(pPairboundIPv6ConnList)->destIP.ipv6_addr[11],
			(pPairboundIPv6ConnList)->destIP.ipv6_addr[12],(pPairboundIPv6ConnList)->destIP.ipv6_addr[13],(pPairboundIPv6ConnList)->destIP.ipv6_addr[14],(pPairboundIPv6ConnList)->destIP.ipv6_addr[15],
			(pPairboundIPv6ConnList)->destPort,
			(pPairboundIPv6ConnList)->internalIP.ipv6_addr[0],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[1],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[2],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[3],
			(pPairboundIPv6ConnList)->internalIP.ipv6_addr[4],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[5],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[6],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[7],
			(pPairboundIPv6ConnList)->internalIP.ipv6_addr[8],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[9],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[10],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[11],
			(pPairboundIPv6ConnList)->internalIP.ipv6_addr[12],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[13],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[14],(pPairboundIPv6ConnList)->internalIP.ipv6_addr[15],
			(pPairboundIPv6ConnList)->internalPort);
	}
	
#else
	TABLE("Add IPv6 Connection ==> from %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x Port:%d --> %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x Port:%d\n",
		(*pIPv6ConnList)->srcIP.ipv6_addr[0],(*pIPv6ConnList)->srcIP.ipv6_addr[1],(*pIPv6ConnList)->srcIP.ipv6_addr[2],(*pIPv6ConnList)->srcIP.ipv6_addr[3],
		(*pIPv6ConnList)->srcIP.ipv6_addr[4],(*pIPv6ConnList)->srcIP.ipv6_addr[5],(*pIPv6ConnList)->srcIP.ipv6_addr[6],(*pIPv6ConnList)->srcIP.ipv6_addr[7],
		(*pIPv6ConnList)->srcIP.ipv6_addr[8],(*pIPv6ConnList)->srcIP.ipv6_addr[9],(*pIPv6ConnList)->srcIP.ipv6_addr[10],(*pIPv6ConnList)->srcIP.ipv6_addr[11],
		(*pIPv6ConnList)->srcIP.ipv6_addr[12],(*pIPv6ConnList)->srcIP.ipv6_addr[13],(*pIPv6ConnList)->srcIP.ipv6_addr[14],(*pIPv6ConnList)->srcIP.ipv6_addr[15],
		(*pIPv6ConnList)->srcPort,
		(*pIPv6ConnList)->destIP.ipv6_addr[0],(*pIPv6ConnList)->destIP.ipv6_addr[1],(*pIPv6ConnList)->destIP.ipv6_addr[2],(*pIPv6ConnList)->destIP.ipv6_addr[3],
		(*pIPv6ConnList)->destIP.ipv6_addr[4],(*pIPv6ConnList)->destIP.ipv6_addr[5],(*pIPv6ConnList)->destIP.ipv6_addr[6],(*pIPv6ConnList)->destIP.ipv6_addr[7],
		(*pIPv6ConnList)->destIP.ipv6_addr[8],(*pIPv6ConnList)->destIP.ipv6_addr[9],(*pIPv6ConnList)->destIP.ipv6_addr[10],(*pIPv6ConnList)->destIP.ipv6_addr[11],
		(*pIPv6ConnList)->destIP.ipv6_addr[12],(*pIPv6ConnList)->destIP.ipv6_addr[13],(*pIPv6ConnList)->destIP.ipv6_addr[14],(*pIPv6ConnList)->destIP.ipv6_addr[15],
		(*pIPv6ConnList)->destPort);
#endif

	return RG_RET_SUCCESS;
}

rtk_rg_successFailReturn_t _rtk_rg_fwdEngine_fillIpv6StatefulFragmentInfo(rtk_rg_ipv6_layer4_linkList_t **pIPv6FragList, rtk_rg_pktHdr_t *pPktHdr)
{		
	int isTCP=0,hashIndex;
	rtk_rg_successFailReturn_t ret;
	
	if(pPktHdr->ipProtocol==0x6) isTCP=1;

	if(isTCP)
		hashIndex=_rtk_rg_ipv6StatefulHashIndex(0x6,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->ipv6FragId_First,pPktHdr->ipv6FragId_Second);
	else
		hashIndex=_rtk_rg_ipv6StatefulHashIndex(0x11,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->ipv6FragId_First,pPktHdr->ipv6FragId_Second);

	ret=_rtk_rg_fwdEngine_ipv6ConnList_get(pIPv6FragList,hashIndex);
	if(ret==RG_RET_FAIL)return RG_RET_FAIL;

	//fill all information abount this connection
	memcpy((*pIPv6FragList)->srcIP.ipv6_addr,pPktHdr->pIpv6Sip,IPV6_ADDR_LEN);
	memcpy((*pIPv6FragList)->destIP.ipv6_addr,pPktHdr->pIpv6Dip,IPV6_ADDR_LEN);
	(*pIPv6FragList)->srcPort=pPktHdr->ipv6FragId_First;		//use fragment id to hash
	(*pIPv6FragList)->destPort=pPktHdr->ipv6FragId_Second;		//use fragment id to hash
	(*pIPv6FragList)->isTCP=isTCP;
	(*pIPv6FragList)->state=INVALID;
	(*pIPv6FragList)->idleSecs=0;
	(*pIPv6FragList)->pPair_list=NULL;
	(*pIPv6FragList)->valid=1;
	(*pIPv6FragList)->notFinishUpdated=0;	//fragment no need update

	(*pIPv6FragList)->isFrag=1;
	(*pIPv6FragList)->fragAction=RG_FWDENGINE_RET_DROP;
	(*pIPv6FragList)->beginIdleTime=jiffies;
	(*pIPv6FragList)->receivedLength=pPktHdr->ipv6PayloadLen;
	if(!pPktHdr->ipv6MoreFragment)
		(*pIPv6FragList)->totalLength=(pPktHdr->ipv6FragmentOffset<<3)+(*pIPv6FragList)->receivedLength;
	else
		(*pIPv6FragList)->totalLength=0;
	DEBUG("fillInfo, received is %d, total is %d",(*pIPv6FragList)->receivedLength,(*pIPv6FragList)->totalLength);
	(*pIPv6FragList)->queueCount=0;
	
	(*pIPv6FragList)->netifIdx=pPktHdr->netifIdx;
	(*pIPv6FragList)->dmacL2Idx=pPktHdr->dmacL2Idx;
	
	TABLE("Add IPv6 FragList ==> from %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x --> %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x ID:%04x%04x\n",
		(*pIPv6FragList)->srcIP.ipv6_addr[0],(*pIPv6FragList)->srcIP.ipv6_addr[1],(*pIPv6FragList)->srcIP.ipv6_addr[2],(*pIPv6FragList)->srcIP.ipv6_addr[3],
		(*pIPv6FragList)->srcIP.ipv6_addr[4],(*pIPv6FragList)->srcIP.ipv6_addr[5],(*pIPv6FragList)->srcIP.ipv6_addr[6],(*pIPv6FragList)->srcIP.ipv6_addr[7],
		(*pIPv6FragList)->srcIP.ipv6_addr[8],(*pIPv6FragList)->srcIP.ipv6_addr[9],(*pIPv6FragList)->srcIP.ipv6_addr[10],(*pIPv6FragList)->srcIP.ipv6_addr[11],
		(*pIPv6FragList)->srcIP.ipv6_addr[12],(*pIPv6FragList)->srcIP.ipv6_addr[13],(*pIPv6FragList)->srcIP.ipv6_addr[14],(*pIPv6FragList)->srcIP.ipv6_addr[15],
		(*pIPv6FragList)->destIP.ipv6_addr[0],(*pIPv6FragList)->destIP.ipv6_addr[1],(*pIPv6FragList)->destIP.ipv6_addr[2],(*pIPv6FragList)->destIP.ipv6_addr[3],
		(*pIPv6FragList)->destIP.ipv6_addr[4],(*pIPv6FragList)->destIP.ipv6_addr[5],(*pIPv6FragList)->destIP.ipv6_addr[6],(*pIPv6FragList)->destIP.ipv6_addr[7],
		(*pIPv6FragList)->destIP.ipv6_addr[8],(*pIPv6FragList)->destIP.ipv6_addr[9],(*pIPv6FragList)->destIP.ipv6_addr[10],(*pIPv6FragList)->destIP.ipv6_addr[11],
		(*pIPv6FragList)->destIP.ipv6_addr[12],(*pIPv6FragList)->destIP.ipv6_addr[13],(*pIPv6FragList)->destIP.ipv6_addr[14],(*pIPv6FragList)->destIP.ipv6_addr[15],
		(*pIPv6FragList)->srcPort,(*pIPv6FragList)->destPort);

	return RG_RET_SUCCESS;
}


rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_ipv6TCPOutboundConnectionTracking(rtk_rg_ipv6_layer4_linkList_t **pIPv6ConnList, rtk_rg_pktHdr_t *pPktHdr)
{
	int32 ret;

	//*pIPv6ConnList=NULL;
	//look up if we add this connection before
	_rtk_rg_fwdEngine_ipv6ConnList_lookup(pIPv6ConnList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport,1,0);

	if(!((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==0)))
	{
		//Connection not found, and not SYN packet, drop..
		if((*pIPv6ConnList)==NULL)
		{
			TRACE("Connection isn't found and isn't SYN packet, drop it!");
			return RG_FWDENGINE_RET_DROP;
		}
	}

	if((*pIPv6ConnList)!=NULL && (*pIPv6ConnList)->state==TCP_CONNECTED)
	{
		if(pPktHdr->tcpFlags.fin==1)
		{	
			//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
			(*pIPv6ConnList)->state=FIRST_FIN;
			if((*pIPv6ConnList)->pPair_list!=NULL)
				(*pIPv6ConnList)->pPair_list->state=FIRST_FIN;
		}
		else if(pPktHdr->tcpFlags.reset==1)
		{
			//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
			(*pIPv6ConnList)->state=RST_RECV;
			if((*pIPv6ConnList)->pPair_list!=NULL)
				(*pIPv6ConnList)->pPair_list->state=RST_RECV;
		}	
	}
	else
	{
		if((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==0))
		{		
			//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n%p",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset,rg_db.pNaptOutFreeListHead);
			if((*pIPv6ConnList)==NULL)
			{
				ret = _rtk_rg_fwdEngine_fillIpv6StatefulInfo(pIPv6ConnList,pPktHdr->ipv6StatefulHashValue,pPktHdr);
	            if(ret!=RG_RET_SUCCESS) 
				{
					TRACE("Drop!");
					return RG_FWDENGINE_RET_DROP;
				}
				if((*pIPv6ConnList)->state>SYN_RECV)
				{
					WARNING("pIPv6ConnList->state>SYN_RECV=%d",(*pIPv6ConnList)->state);
				}
				(*pIPv6ConnList)->state=SYN_RECV;
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
				if(pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPT || pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPTR){
					if((*pIPv6ConnList)->pPair_list!=NULL)(*pIPv6ConnList)->pPair_list->state=SYN_RECV;
				}
#endif			
			}
		}
		else if((pPktHdr->tcpFlags.syn==0)&&(pPktHdr->tcpFlags.ack==1))
		{
			if((*pIPv6ConnList)->state==SYN_ACK_RECV)
			{								
				//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
				//assert(rg_db.naptOut[*pNaptOutIdx].state==SYN_ACK_RECV);
				(*pIPv6ConnList)->state=TCP_CONNECTED;
				if((*pIPv6ConnList)->pPair_list!=NULL)
					(*pIPv6ConnList)->pPair_list->state=TCP_CONNECTED;
				
				TRACE("IPv6 TCP Connection is CONNECTED!");
			}
		}
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
		else if((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==1))//Server in Lan case
		{							
			DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
			if((*pIPv6ConnList)==NULL){
				TRACE("not found outbound pIPv6ConnList, DROP!");
				return RG_FWDENGINE_RET_DROP;
			}
			
			if((*pIPv6ConnList)->pPair_list==NULL){//the pair connList should be add while SYN packet from WAN (Server in Lan case)
				TRACE("not found inbound pair pIPv6ConnList, DROP!");
				return RG_FWDENGINE_RET_DROP;
			}

			if((*pIPv6ConnList)->state==INVALID) 
			{
				TRACE("loss SYN packet, DROP!");
				return RG_FWDENGINE_RET_DROP;
			}

			if((*pIPv6ConnList)->state<=SYN_ACK_RECV)
			{
				(*pIPv6ConnList)->state=SYN_ACK_RECV;
			}
			(*pIPv6ConnList)->state=SYN_ACK_RECV;
			(*pIPv6ConnList)->pPair_list->state=SYN_ACK_RECV;

		}

#endif	
		
	}
	return RG_FWDENGINE_RET_NAPT_OK;
}
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_ipv6TCPInboundConnectionTracking(rtk_rg_ipv6_layer4_linkList_t **pIPv6ConnList, rtk_rg_pktHdr_t *pPktHdr)
{
	int32 ret;
	rtk_rg_ipv6_layer4_linkList_t *pIPv6OutboundList=NULL,*pIPv6InboundList=NULL;

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	if(pPktHdr->fwdDecision!=RG_FWD_DECISION_V6NAPTR){ //The inbound connection is already added while NAPT outbound process.
#endif	
		//look up if we add outbound list before
		_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pIPv6OutboundList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Dip,pPktHdr->pIpv6Sip,pPktHdr->dport,pPktHdr->sport,1,0);
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	}else{
		if(pPktHdr->ipv6_serverInLanLookup.serverInLanHit!=RTK_RG_IPV6_LOOKUP_NONE_HIT){
			//Server in lan case: pIPv6OutboundList could be NULL, do not access directly.
			//if it is inbound SYN: it doesn't have setup the connList yet!  the connList will be add after _rtk_rg_fwdEngine_fillIpv6StatefulInfo() called.
			//if it is inbound ACK: it will find the connList.
		}else{
			//Normal inbound case: pIPv6InboundList has been lookup while searching DIP(pPktHdr->pIPv6StatefulList and pPktHdr->ipv6StatefulHashValue have assigned!), look up pIPv6OutboundList list which record in pIPv6InboundList
			pIPv6OutboundList = pPktHdr->pIPv6StatefulList->pPair_list;
			TRACE("Get pIPv6OutboundList[%p] by pair",pIPv6OutboundList);
		}
	}
#endif

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	if(pIPv6OutboundList==NULL && pPktHdr->ipv6_serverInLanLookup.serverInLanHit==RTK_RG_IPV6_LOOKUP_NONE_HIT)
#else
	if(pIPv6OutboundList==NULL)
#endif
	{
		//not server in lan case, the oubound connList should be created when oubound SYN  packet!
		TRACE("there is no outbound list...DROP");
		return RG_FWDENGINE_RET_DROP;
	}

	if((pIPv6OutboundList!=NULL) && (pIPv6OutboundList->state==TCP_CONNECTED))
	{
		if(pPktHdr->tcpFlags.fin==1)
		{	
			DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
			pIPv6OutboundList->state=FIRST_FIN;
			if(pIPv6OutboundList->pPair_list!=NULL)pIPv6OutboundList->pPair_list->state=FIRST_FIN;
		}
		else if(pPktHdr->tcpFlags.reset==1)
		{
			DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
			pIPv6OutboundList->state=RST_RECV;
			if(pIPv6OutboundList->pPair_list!=NULL)pIPv6OutboundList->pPair_list->state=RST_RECV;
		}
	}
	else
	{
		DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
		if((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==1))
		{							
			DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
			if(pIPv6OutboundList->state==INVALID) 
			{
				TRACE("loss SYN packet, DROP!");
				return RG_FWDENGINE_RET_DROP;
			}

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
			if(pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPTR){ //The inbound connection is already added while NAPT outbound process. (pIPv6InboundList has been lookup while searching DIP)
				pIPv6InboundList = pPktHdr->pIPv6StatefulList;
				if(pIPv6OutboundList->state<=SYN_ACK_RECV)
				{
					pIPv6OutboundList->state=SYN_ACK_RECV;
				}
				pIPv6InboundList->state=SYN_ACK_RECV;
			}else{
#endif			
				pPktHdr->ipv6StatefulHashValue=-1;
				_rtk_rg_fwdEngine_ipv6ConnList_lookup(pIPv6ConnList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport,1,0);
				if((*pIPv6ConnList)==NULL)
				{
					ret = _rtk_rg_fwdEngine_fillIpv6StatefulInfo(&pIPv6InboundList,pPktHdr->ipv6StatefulHashValue,pPktHdr);
					if(ret!=RG_RET_SUCCESS) 
					{
						TRACE("Drop!");
						return RG_FWDENGINE_RET_DROP;
					}
				
					//keep inbound list pointer in outbound list, for deleting when timeout
					pIPv6OutboundList->pPair_list=pIPv6InboundList;
					
					if(pIPv6OutboundList->state<=SYN_ACK_RECV)
					{
						pIPv6OutboundList->state=SYN_ACK_RECV;
					}
					pIPv6InboundList->state=SYN_ACK_RECV;
					pIPv6InboundList->pPair_list=pIPv6OutboundList;
					
					*pIPv6ConnList=pIPv6InboundList;
				}

				//keep inbound list pointer in outbound list, for deleting when timeout
				pIPv6OutboundList->pPair_list=pIPv6InboundList;
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
			} //End of if pPktHdr->fwdDecision!=RG_FWD_DECISION_V6NAPTR
#endif		
			*pIPv6ConnList=pIPv6InboundList;
		}		
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
		else if((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==0))//Server in lan case
		{		
			if(pPktHdr->ipv6_serverInLanLookup.serverInLanHit==RTK_RG_IPV6_LOOKUP_NONE_HIT){
				TRACE("Drop SYN from WAN, Server in Lan none hit!");
				return RG_FWDENGINE_RET_DROP;
			}
		
			//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n %p",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset,rg_db.pNaptOutFreeListHead);
			//the connList should not be found, but we have to get an empty connList
			pPktHdr->ipv6StatefulHashValue=-1;
			_rtk_rg_fwdEngine_ipv6ConnList_lookup(pIPv6ConnList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport,1,0);

			if((*pIPv6ConnList)==NULL)
			{
				ret = _rtk_rg_fwdEngine_fillIpv6StatefulInfo(pIPv6ConnList,pPktHdr->ipv6StatefulHashValue,pPktHdr);
	            if(ret!=SUCCESS) 
				{
					TRACE("Drop!");
					return RG_FWDENGINE_RET_DROP;
				}
				if((*pIPv6ConnList)->state>SYN_RECV)
				{
					WARNING("pIPv6ConnList->state>SYN_RECV=%d",(*pIPv6ConnList)->state);
				}
				(*pIPv6ConnList)->state=SYN_RECV;
				if((*pIPv6ConnList)->pPair_list!=NULL)
					(*pIPv6ConnList)->pPair_list->state=SYN_RECV;			
			}
		}
		else if((pPktHdr->tcpFlags.syn==0)&&(pPktHdr->tcpFlags.ack==1))//Server in lan case
		{
			//the connList should be found
			pPktHdr->ipv6StatefulHashValue=-1;
			_rtk_rg_fwdEngine_ipv6ConnList_lookup(pIPv6ConnList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport,1,0);
			if((*pIPv6ConnList)==NULL){
				TRACE("pIPv6ConnList not found, drop!");
				return RG_FWDENGINE_RET_DROP;
			}
		
			if((*pIPv6ConnList)->state==SYN_ACK_RECV)
			{								
				//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
				//assert(rg_db.naptOut[*pNaptOutIdx].state==SYN_ACK_RECV);
				(*pIPv6ConnList)->state=TCP_CONNECTED;
				if((*pIPv6ConnList)->pPair_list!=NULL)
					(*pIPv6ConnList)->pPair_list->state=TCP_CONNECTED;
				
				TRACE("IPv6 TCP Connection is CONNECTED!");
			}
		}
#endif	
	}
	return RG_FWDENGINE_RET_NAPT_OK;
}

rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_ipv6UDPOutboundConnectionTracking(rtk_rg_ipv6_layer4_linkList_t **pIPv6ConnList, rtk_rg_pktHdr_t *pPktHdr)
{
	//build connection when the first packet send out!
	rtk_rg_successFailReturn_t ret;

	//look up if we add this connection before
	_rtk_rg_fwdEngine_ipv6ConnList_lookup(pIPv6ConnList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport,0,0);

	if((*pIPv6ConnList)==NULL)
	{
		ret = _rtk_rg_fwdEngine_fillIpv6StatefulInfo(pIPv6ConnList,pPktHdr->ipv6StatefulHashValue,pPktHdr);
		if(ret!=RG_RET_SUCCESS) 
		{
			TRACE("Drop!");
			return RG_FWDENGINE_RET_DROP;
		}
	}

	if((*pIPv6ConnList)!=NULL)//suppose the pIPv6ConnList will be found or added by _rtk_rg_fwdEngine_fillIpv6StatefulInfo()
		(*pIPv6ConnList)->state=UDP_CONNECTED;

	return RG_FWDENGINE_RET_NAPT_OK;
}
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_ipv6UDPInboundConnectionTracking(rtk_rg_ipv6_layer4_linkList_t **pIPv6ConnList, rtk_rg_pktHdr_t *pPktHdr)
{
	//build connection when the outbound is built and receive first inbound packet!
	rtk_rg_successFailReturn_t ret;
	rtk_rg_ipv6_layer4_linkList_t *pIPv6OutboundList=NULL,*pIPv6InboundList=NULL;
	
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	//support for server in lan
	//rtk_ipv6_addr_t transIP;
	//int16 transPort;

	if(pPktHdr->fwdDecision!=RG_FWD_DECISION_V6NAPTR){ //Routing case
#endif	
		//look up if we add outbound list before
		_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pIPv6OutboundList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Dip,pPktHdr->pIpv6Sip,pPktHdr->dport,pPktHdr->sport,0,0);

		if(pIPv6OutboundList==NULL)
		{
			TRACE("there is no outbound list, drop it!");
			return RG_FWDENGINE_RET_DROP;	
		}

		//for UDP, when packet enter fwdEngine_input, the hashvalue will be caculated and stored in pPktHdr->ipv6StatefulHashValue
		//so we can use it directly.
		//if(pPktHdr->ipv6StatefulHashValue<0)
			//pPktHdr->ipv6StatefulHashValue=_rtk_rg_ipv6StatefulHashIndex(0x11,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport);


		//look up for inbound list
		pPktHdr->ipv6StatefulHashValue=-1;
		_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pIPv6InboundList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport,0,0);
		if(pIPv6InboundList==NULL){
			ret = _rtk_rg_fwdEngine_fillIpv6StatefulInfo(&pIPv6InboundList,pPktHdr->ipv6StatefulHashValue,pPktHdr);
		    if(ret!=RG_RET_SUCCESS) 
			{
				TRACE("Drop!");
				return RG_FWDENGINE_RET_DROP;
		    }
		}

		//keep inbound list pointer in outbound list, for deleting when timeout
		pIPv6OutboundList->pPair_list=pIPv6InboundList;
		
		pIPv6InboundList->state=UDP_CONNECTED;
		pIPv6InboundList->pPair_list=pIPv6OutboundList;
		
		*pIPv6ConnList=pIPv6InboundList;

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	}
	else //NAPTR case
	{
		if(pPktHdr->ipv6_serverInLanLookup.serverInLanHit!=RTK_RG_IPV6_LOOKUP_NONE_HIT){//Server in Lan case is hit.
			pPktHdr->ipv6StatefulHashValue=-1;
			_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pIPv6InboundList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport,0,0);		
			if(pIPv6InboundList==NULL)
			{
				ret = _rtk_rg_fwdEngine_fillIpv6StatefulInfo(&pIPv6InboundList,pPktHdr->ipv6StatefulHashValue,pPktHdr);
				if(ret!=RG_RET_SUCCESS) 
				{
					TRACE("Drop!");
					return RG_FWDENGINE_RET_DROP;
				}
				
				*pIPv6ConnList=pIPv6InboundList;
				pPktHdr->pIPv6StatefulList = *pIPv6ConnList;
				pIPv6InboundList->state=UDP_CONNECTED;
			}
				
		}else{//normal NAPTR case:The inbound connection is usually already added while NAPT outbound process.
			if(pPktHdr->pIPv6StatefulList==NULL){ //connList not found
				TRACE("Inbound connList not found, drop it!");
				return RG_FWDENGINE_RET_DROP;	
			}
			//pIPv6InboundList has been lookup while searching DIP(pPktHdr->pIPv6StatefulList and pPktHdr->ipv6StatefulHashValue have assigned!), look up pIPv6OutboundList list which record in pIPv6InboundList
			pIPv6InboundList=pPktHdr->pIPv6StatefulList;
			*pIPv6ConnList = pIPv6InboundList;
			pIPv6InboundList->state=UDP_CONNECTED;
		}
	}
#endif
	return RG_FWDENGINE_RET_NAPT_OK;
}

void _rtk_rg_fwdEngine_v6FragmentPacketQueuing(rtk_rg_naptDirection_t direction, rtk_l34_nexthop_type_t	wanType, struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	int i,queueIdx;
	long compareTime;
	
	//------------------ Critical Section start -----------------------//
	rg_lock(&rg_kernel.ipv6FragQueueLock);

	queueIdx=0;
	compareTime=0;

	//find the first valid one or smallest queue_time(oldest) to use
	for(i=0;i<MAX_IPV6_FRAGMENT_QUEUE_SIZE;i++)
	{
		if(rg_db.ipv6FragmentQueue[i].occupied==0)
		{
			compareTime = 0;
			queueIdx = i;
			rg_db.systemGlobal.ipv6FragmentQueueNum++;
			break;
		}
		else if(rg_db.ipv6FragmentQueue[i].queue_time < compareTime)
		{
			compareTime = rg_db.ipv6FragmentQueue[i].queue_time;
			queueIdx = i;
		}
	}	

	//free the oldest one skb
	if(compareTime>0)
		_rtk_rg_dev_kfree_skb_any(rg_db.ipv6FragmentQueue[queueIdx].queue_skb);

	//insert new packet into this idx
	rg_db.ipv6FragmentQueue[queueIdx].queue_skb=skb;
	memcpy(&rg_db.ipv6FragmentQueue[queueIdx].queue_pktHdr,pPktHdr,sizeof(rtk_rg_pktHdr_t));
	memcpy(&rg_db.ipv6FragmentQueue[queueIdx].queue_rx_info,pPktHdr->pRxDesc,sizeof(struct rx_info));
	if(pPktHdr->cp)memcpy(&rg_db.ipv6FragmentQueue[queueIdx].queue_cp,pPktHdr->cp,sizeof(struct re_private));
	rg_db.ipv6FragmentQueue[queueIdx].queue_time=jiffies;
	rg_db.ipv6FragmentQueue[queueIdx].occupied=1;
	rg_db.ipv6FragmentQueue[queueIdx].direction=direction;
	rg_db.ipv6FragmentQueue[queueIdx].wanType=wanType;

	//------------------ Critical Section End -----------------------//
	rg_unlock(&rg_kernel.ipv6FragQueueLock);

}

void _rtk_rg_fwdEngine_v6FragmentQueueProcessing(int aclRet, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_ipv6_layer4_linkList_t *pFragList)
{
	int i,totalQueueNum;
	//If we are the first fragment packet:
	//Check the queue to see if there is any packet has same identification and SIP,DIP
	//Loop if match:
	//	Forward the packet
	//	Move the last one to the proceed queue position
	//	Queue number --
	totalQueueNum=rg_db.systemGlobal.ipv6FragmentQueueNum;
	for(i=0;i<MAX_IPV6_FRAGMENT_QUEUE_SIZE&&totalQueueNum>0;i++)
	{
		DEBUG("rg_db.systemGlobal.ipv6FragmentQueueNum is %d",rg_db.systemGlobal.ipv6FragmentQueueNum);
		DEBUG("rg_db.ipv6FragmentQueue[%d].occupied = %d",i,rg_db.ipv6FragmentQueue[i].occupied);
		DEBUG("*rg_db.ipv6FragmentQueue[%d].queue_pktHdr.ipv6FragIdentification = %02x%02x",i,rg_db.ipv6FragmentQueue[i].queue_pktHdr.ipv6FragId_First,rg_db.ipv6FragmentQueue[i].queue_pktHdr.ipv6FragId_Second);
		//------------------ Critical Section start -----------------------//
		rg_lock(&rg_kernel.ipv6FragQueueLock);
		if(rg_db.ipv6FragmentQueue[i].occupied)
		{
			totalQueueNum--;
			if(!memcmp(pPktHdr->pIpv6Sip,rg_db.ipv6FragmentQueue[i].queue_pktHdr.pIpv6Sip,IPV6_ADDR_LEN) &&
				pPktHdr->ipv6FragId_First==rg_db.ipv6FragmentQueue[i].queue_pktHdr.ipv6FragId_First &&
				pPktHdr->ipv6FragId_Second==rg_db.ipv6FragmentQueue[i].queue_pktHdr.ipv6FragId_Second)
			{
				DEBUG("queue [%d] Match!! payload size is %d, v6payloadLength is %d",i,rg_db.ipv6FragmentQueue[i].queue_pktHdr.l3Len,rg_db.ipv6FragmentQueue[i].queue_pktHdr.ipv6PayloadLen);
				pFragList->queueCount--;

				//Check ACL action by first packet
				if(aclRet==RG_FWDENGINE_RET_TO_PS)
				{
					DEBUG("return to Protocol stack!!");
#ifdef __KERNEL__				
					//from master wifi
					if(rg_db.ipv6FragmentQueue[i].queue_pktHdr.ingressPort==RTK_RG_EXT_PORT0)
					{
						rg_db.ipv6FragmentQueue[i].queue_skb->data+=ETH_HLEN;
						rg_db.ipv6FragmentQueue[i].queue_skb->len-=ETH_HLEN;
						TRACE("WLAN0 queue_v6packet trap to netif_rx\n");
						netif_rx(rg_db.ipv6FragmentQueue[i].queue_skb);	
					}
					else
						re8670_rx_skb(&rg_db.ipv6FragmentQueue[i].queue_cp,rg_db.ipv6FragmentQueue[i].queue_skb,&rg_db.ipv6FragmentQueue[i].queue_rx_info);
#else
					model_nic_rx_skb(&rg_db.ipv6FragmentQueue[i].queue_cp,rg_db.ipv6FragmentQueue[i].queue_skb,&rg_db.ipv6FragmentQueue[i].queue_rx_info);
#endif
				}
				else if(aclRet==RG_FWDENGINE_RET_DROP)
				{
					DEBUG("Drop packet[%d]!!",i);
					_rtk_rg_dev_kfree_skb_any(rg_db.ipv6FragmentQueue[i].queue_skb);
				}
				else
				{		
					_rtk_rg_fwdEngine_ipv6PacketModify(rg_db.ipv6FragmentQueue[i].direction,rg_db.ipv6FragmentQueue[i].wanType,&rg_db.ipv6FragmentQueue[i].queue_pktHdr,rg_db.ipv6FragmentQueue[i].queue_skb);

					DEBUG("send the queued fragment packet [%d]!",i);
#ifdef CONFIG_APOLLO_MODEL			
#else				
					_rtk_rg_fwdEngineDirectTx(rg_db.ipv6FragmentQueue[i].queue_skb,&rg_db.ipv6FragmentQueue[i].queue_pktHdr);
#endif
				}
				rg_db.systemGlobal.ipv6FragmentQueueNum--;
				rg_db.ipv6FragmentQueue[i].occupied = 0;
				rg_db.ipv6FragmentQueue[i].queue_time = 0;
			}
		}
		//------------------ Critical Section End -----------------------//
		rg_unlock(&rg_kernel.ipv6FragQueueLock);
	}

}


rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_v6FragmentHandling(rtk_rg_naptDirection_t direction, rtk_l34_nexthop_type_t	wanType,rtk_rg_ipv6_layer4_linkList_t **pIPv6ConnList, rtk_rg_pktHdr_t *pPktHdr, struct sk_buff *skb)
{
	int hashIndex=-1,fragAction=RG_FWDENGINE_RET_CONTINUE;
	rtk_rg_ipv6_layer4_linkList_t *pFragList=NULL;
	rtk_rg_fwdEngineReturn_t ret;
	
	//First Fragment Packet
	if(pPktHdr->ipv6MoreFragment && pPktHdr->ipv6FragmentOffset==0)
	{
		//first check it's state
		//then create fragment list
		//modify packet and send!
		ret=RG_FWDENGINE_RET_NAPT_OK;
		if(direction==NAPT_DIRECTION_OUTBOUND || direction==IPV6_ROUTE_OUTBOUND)
		{
			if(pPktHdr->tagif&TCP_TAGIF)
				ret=_rtk_rg_fwdEngine_ipv6TCPOutboundConnectionTracking(pIPv6ConnList,pPktHdr);
			else if(pPktHdr->tagif&UDP_TAGIF)
				ret=_rtk_rg_fwdEngine_ipv6UDPOutboundConnectionTracking(pIPv6ConnList,pPktHdr);
			if(ret!=RG_FWDENGINE_RET_NAPT_OK)fragAction=ret;
		}
		else
		{
			if(pPktHdr->tagif&TCP_TAGIF)
				ret=_rtk_rg_fwdEngine_ipv6TCPInboundConnectionTracking(pIPv6ConnList,pPktHdr);
			else if(pPktHdr->tagif&UDP_TAGIF)
				ret=_rtk_rg_fwdEngine_ipv6UDPInboundConnectionTracking(pIPv6ConnList,pPktHdr);
			if(ret!=RG_FWDENGINE_RET_NAPT_OK)fragAction=ret;
		}

		//update direction and wanType(ether or pppoe)
		if((*pIPv6ConnList)!=NULL)
		{
			(*pIPv6ConnList)->direction=direction;
			(*pIPv6ConnList)->wanType=wanType;
			pPktHdr->pIPv6StatefulList=(*pIPv6ConnList);
			pPktHdr->shortcutStatus=RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND;
			pPktHdr->pIPv6StatefulList->idleSecs=0;
		}

		if(pPktHdr->pIPv6FragmentList==NULL)
		{
			ret=_rtk_rg_fwdEngine_fillIpv6StatefulFragmentInfo(&pFragList,pPktHdr);
			if(ret!=RG_RET_SUCCESS)return RG_FWDENGINE_RET_FRAG_ONE_DROP;
		}
		else
			pFragList=pPktHdr->pIPv6FragmentList;

		_rtk_rg_fwdEngine_ipv6PacketModify(direction,wanType,pPktHdr,skb);
		pFragList->fragAction=fragAction;
		pFragList->pPair_list=(*pIPv6ConnList);
		DEBUG("pFragList[%p]->fragAction is %d, receivedLength became %d, total is %d",pFragList,pFragList->fragAction,pFragList->receivedLength,pFragList->totalLength);

		pPktHdr->pIPv6FragmentList=pFragList;
		
		if(fragAction==RG_FWDENGINE_RET_DROP)
			return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
		else if(fragAction==RG_FWDENGINE_RET_TO_PS)
			return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue

		if(pFragList->totalLength>0 && pFragList->receivedLength>=pFragList->totalLength)
		{
			DEBUG("all frag packet are out, free the list...");
			pFragList->pPair_list=NULL;		//leave conn list un-touch
			//------------------ Critical Section start -----------------------//
			rg_lock(&rg_kernel.ipv6StatefulLock);
			_rtk_rg_fwdEngine_ipv6ConnList_del(pFragList);
			//------------------ Critical Section End -----------------------//
			rg_unlock(&rg_kernel.ipv6StatefulLock);
		}

		return RG_FWDENGINE_RET_FRAGMENT_ONE;
	}
	else	//other fragment packet
	{
		DEBUG("other fragment");
		if(pPktHdr->ipProtocol==0x6)
			_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pFragList,&hashIndex,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->ipv6FragId_First,pPktHdr->ipv6FragId_Second,1,1);
		else if(pPktHdr->ipProtocol==0x11)
			_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pFragList,&hashIndex,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->ipv6FragId_First,pPktHdr->ipv6FragId_Second,0,1);
		if(pFragList!=NULL)
		{
			//hit, do what first packet do

			//filled by first packet or other early fragment packets
			if(pPktHdr->ipv6MoreFragment==0)		//we can not free fragList here, unless there are all packets had forwarded
				pFragList->totalLength=(pPktHdr->ipv6FragmentOffset<<3)+pPktHdr->ipv6PayloadLen;
			DEBUG("pFragList[%p]->fragAction is %d, receivedLength became %d, total is %d",pFragList,pFragList->fragAction,pFragList->receivedLength,pFragList->totalLength);
			//DEBUG("later fragment modify... fragIdx=%d",fragIdx);
			if(pFragList->totalLength>0 && pFragList->receivedLength>=pFragList->totalLength)		//we can not free fragList here, unless there are all packets had forwarded
			{
				DEBUG("all frag packet are out, free the list...");
				fragAction=pFragList->fragAction;
				pFragList->pPair_list=NULL;		//leave conn list un-touch
				//------------------ Critical Section start -----------------------//
				rg_lock(&rg_kernel.ipv6StatefulLock);
				_rtk_rg_fwdEngine_ipv6ConnList_del(pFragList);
				//------------------ Critical Section End -----------------------//
				rg_unlock(&rg_kernel.ipv6StatefulLock);
			}
					
			//Check frag action from first packet
			DEBUG("pFragList[%p]->fragAction is %d",pFragList,pFragList->fragAction);
			if(pFragList->fragAction==RG_FWDENGINE_RET_TO_PS || pFragList->fragAction==RG_FWDENGINE_RET_DROP)
				return pFragList->fragAction;
			if(pFragList->fragAction==RG_FWDENGINE_RET_QUEUE_FRAG)	//first packet not come in yet
				goto IPV6_FRAG_QUEUE;

			ret = _rtk_rg_fwdEngine_ipv6PacketModify(direction,wanType,pPktHdr,skb);
			if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
			DEBUG("before ret fragment..");
			return RG_FWDENGINE_RET_FRAGMENT;			
		}
		else
		{	
			//Otherwise we need to queue this packet for later proceed
			ret=_rtk_rg_fwdEngine_fillIpv6StatefulFragmentInfo(&pFragList,pPktHdr);
			if(ret!=RG_RET_SUCCESS)return RG_FWDENGINE_RET_FRAG_ONE_DROP;

			pFragList->fragAction=RG_FWDENGINE_RET_QUEUE_FRAG;
		
IPV6_FRAG_QUEUE:				
			DEBUG("queuing packet.....");

			//Check if we already queue same identification for MAX_FRAGMENT_QUEUE_THRESHOLD times
			pFragList->queueCount++;
			if(pFragList->queueCount>=MAX_FRAGMENT_QUEUE_THRESHOLD)
			{
				//clear same identification in queue
				pFragList->queueCount=0;
				pFragList->fragAction=RG_FWDENGINE_RET_DROP;
				_rtk_rg_fwdEngine_v6FragmentQueueProcessing(pFragList->fragAction,pPktHdr,pFragList);
				TRACE("Drop!");
				return RG_FWDENGINE_RET_DROP;
			}
			else
			{
				//Put the fragment packet into queue
				_rtk_rg_fwdEngine_v6FragmentPacketQueuing(direction,wanType,skb,pPktHdr);
				return RG_FWDENGINE_RET_QUEUE_FRAG;
			}
		}
	}
}
#endif

#if 1
rtk_rg_entryGetReturn_t _rtk_rg_naptPriority_pattern_check(int rule_direction, int pkt_direction, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_sw_naptFilterAndQos_t *pNaptPriorityRuleStart){
	//the egress pattern will be filled to pPktHdr after _rtk_rg_fwdEngine_shortCutNaptPacketModify()
	//egress pattern should consider direction
	int i=0; //double make sure don't go into infinity loop
	int check_direct=CHECK_BOUND_PKT_WITH_BOUND_RULE_END;
	rtk_rg_sw_naptFilterAndQos_t *pCurrentCheckRule=NULL;
	if(pPktHdr==NULL) return RG_RET_ENTRY_NOT_GET; //no information can be compared
	if(pNaptPriorityRuleStart==NULL) return RG_RET_ENTRY_NOT_GET; //no rules need to check


	DEBUG("NaptFilter, %s rule Checking:",(rule_direction==RG_FWD_DECISION_NAPT)?"outbound":"inbound");
	if(pkt_direction==NAPT_DIRECTION_OUTBOUND&& rule_direction==RG_FWD_DECISION_NAPT){
		check_direct = CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE;
	}else if(pkt_direction==NAPT_DIRECTION_OUTBOUND && rule_direction==RG_FWD_DECISION_NAPTR){
		check_direct = CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE;
	}else if(pkt_direction==NAPT_DIRECTION_INBOUND && rule_direction==RG_FWD_DECISION_NAPT){
		check_direct = CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE;
	}else if(pkt_direction==NAPT_DIRECTION_INBOUND && rule_direction==RG_FWD_DECISION_NAPTR){
		check_direct = CHECK_INBOUND_PKT_WITH_INBOUND_RULE;
	}else{
		DEBUG("naptFilter can not judge direction, check failed!");
		goto unhit;
	}

	pCurrentCheckRule = pNaptPriorityRuleStart;
	while(pCurrentCheckRule!=NULL && i<MAX_NAPT_FILER_SW_ENTRY_SIZE){
		DEBUG("Checking naptFilterRule[%d]",pCurrentCheckRule->sw_index);
		if(pCurrentCheckRule->naptFilter.filter_fields & INGRESS_SIP){
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(pPktHdr->ipv4Sip!=pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr){
						DEBUG("INGRESS_SIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Sip,pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr);
						goto unhit;
					}	
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->ipv4Dip!=pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr){
						DEBUG("INGRESS_SIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(*(pPktHdr->pIpv4Dip)!=pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr){
						DEBUG("INGRESS_SIP UNHIT: pkt=0x%x rule=0x%x",*(pPktHdr->pIpv4Dip),pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->ipv4Sip!=pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr){
						DEBUG("INGRESS_SIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Sip,pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr);
						goto unhit;
					}
					break;
			}
			
		}
		if(pCurrentCheckRule->naptFilter.filter_fields & EGRESS_SIP){
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(*(pPktHdr->pIpv4Sip)!=pCurrentCheckRule->naptFilter.egress_src_ipv4_addr){
						DEBUG("EGRESS_SIP UNHIT: pkt=0x%x rule=0x%x",*(pPktHdr->pIpv4Sip),pCurrentCheckRule->naptFilter.egress_src_ipv4_addr);
						goto unhit;
					}					
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->ipv4Dip!=pCurrentCheckRule->naptFilter.egress_src_ipv4_addr){
						DEBUG("EGRESS_SIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.egress_src_ipv4_addr);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(pPktHdr->ipv4Dip!=pCurrentCheckRule->naptFilter.egress_src_ipv4_addr){
						DEBUG("EGRESS_SIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.egress_src_ipv4_addr);
						goto unhit;
					}					
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if(*(pPktHdr->pIpv4Sip)!=pCurrentCheckRule->naptFilter.egress_src_ipv4_addr){
						DEBUG("EGRESS_SIP UNHIT: pkt=0x%x rule=0x%x",*(pPktHdr->pIpv4Sip),pCurrentCheckRule->naptFilter.egress_src_ipv4_addr);
						goto unhit;
					}
					break;
			}
	
		}
		if(pCurrentCheckRule->naptFilter.filter_fields & INGRESS_DIP){
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(pPktHdr->ipv4Dip!=pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr){
						DEBUG("INGRESS_DIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr);
						goto unhit;
					}	
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if(*(pPktHdr->pIpv4Sip)!=pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr){
						DEBUG("INGRESS_DIP UNHIT: pkt=0x%x rule=0x%x",*(pPktHdr->pIpv4Sip),pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr);
						goto unhit;
					}	
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(pPktHdr->ipv4Sip!=pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr){
						DEBUG("INGRESS_DIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Sip,pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr);
						goto unhit;
					}	
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->ipv4Dip!=pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr){
						DEBUG("INGRESS_DIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr);
						goto unhit;
					}	
					break;
			}
		}
		if(pCurrentCheckRule->naptFilter.filter_fields & EGRESS_DIP){
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(pPktHdr->ipv4Dip!=pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr){
						DEBUG("EGRESS_DIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr);
						goto unhit;
					}						
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->ipv4Sip!=pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr){
						DEBUG("EGRESS_DIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Sip,pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr);
						goto unhit;
					}						
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(pPktHdr->ipv4Sip!=pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr){
						DEBUG("EGRESS_DIP UNHIT: pkt=0x%x rule=0x%x",pPktHdr->ipv4Sip,pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr);
						goto unhit;
					}						
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if(*(pPktHdr->pIpv4Dip)!=pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr){
						DEBUG("EGRESS_DIP UNHIT: pkt=0x%x rule=0x%x",*(pPktHdr->pIpv4Dip),pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr);
						goto unhit;
					}						
					break;
			}
	
		}
		if(pCurrentCheckRule->naptFilter.filter_fields & INGRESS_SPORT){
			if(pPktHdr->tagif&ICMP_TAGIF){
				DEBUG("INGRESS_SPORT UNHIT: pkt is ICMP rule=%d",pCurrentCheckRule->naptFilter.ingress_src_l4_port);
				goto unhit;	
			}
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(pPktHdr->sport!=pCurrentCheckRule->naptFilter.ingress_src_l4_port){
						DEBUG("INGRESS_SPORT UNHIT: pkt=%d rule=%d",pPktHdr->sport,pCurrentCheckRule->naptFilter.ingress_src_l4_port);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->dport!=pCurrentCheckRule->naptFilter.ingress_src_l4_port){
						DEBUG("INGRESS_SPORT UNHIT: pkt=%d rule=%d",pPktHdr->dport,pCurrentCheckRule->naptFilter.ingress_src_l4_port);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(*(pPktHdr->pDport)!=pCurrentCheckRule->naptFilter.ingress_src_l4_port){
						DEBUG("INGRESS_SPORT UNHIT: pkt=%d rule=%d",*(pPktHdr->pDport),pCurrentCheckRule->naptFilter.ingress_src_l4_port);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->sport!=pCurrentCheckRule->naptFilter.ingress_src_l4_port){
						DEBUG("INGRESS_SPORT UNHIT: pkt=%d rule=%d",pPktHdr->sport,pCurrentCheckRule->naptFilter.ingress_src_l4_port);
						goto unhit;
					}
					break;
			}	
		}
		if(pCurrentCheckRule->naptFilter.filter_fields & EGRESS_SPORT){
			if(pPktHdr->tagif&ICMP_TAGIF){
				DEBUG("EGRESS_SPORT UNHIT: pkt is ICMP rule=%d",pCurrentCheckRule->naptFilter.ingress_src_l4_port);
				goto unhit;	
			}
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(*(pPktHdr->pSport)!=pCurrentCheckRule->naptFilter.egress_src_l4_port){
						DEBUG("EGRESS_SPORT UNHIT: pkt=%d rule=%d",*(pPktHdr->pSport),pCurrentCheckRule->naptFilter.egress_src_l4_port);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->dport!=pCurrentCheckRule->naptFilter.egress_src_l4_port){
						DEBUG("EGRESS_SPORT UNHIT: pkt=%d rule=%d",pPktHdr->dport,pCurrentCheckRule->naptFilter.egress_src_l4_port);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(pPktHdr->dport!=pCurrentCheckRule->naptFilter.egress_src_l4_port){
						DEBUG("EGRESS_SPORT UNHIT: pkt=%d rule=%d",pPktHdr->dport,pCurrentCheckRule->naptFilter.egress_src_l4_port);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if(*(pPktHdr->pSport)!=pCurrentCheckRule->naptFilter.egress_src_l4_port){
						DEBUG("EGRESS_SPORT UNHIT: pkt=%d rule=%d",*(pPktHdr->pSport),pCurrentCheckRule->naptFilter.egress_src_l4_port);
						goto unhit;
					}
					break;
			}
		}
		if(pCurrentCheckRule->naptFilter.filter_fields & INGRESS_DPORT){
			if(pPktHdr->tagif&ICMP_TAGIF){
				DEBUG("INGRESS_DPORT UNHIT: pkt is ICMP rule=%d",pCurrentCheckRule->naptFilter.ingress_src_l4_port);
				goto unhit;	
			}
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(pPktHdr->dport!=pCurrentCheckRule->naptFilter.ingress_dest_l4_port){
						DEBUG("INGRESS_DPORT UNHIT: pkt=%d rule=%d",pPktHdr->dport,pCurrentCheckRule->naptFilter.ingress_dest_l4_port);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->sport!=pCurrentCheckRule->naptFilter.ingress_dest_l4_port){
						DEBUG("INGRESS_DPORT UNHIT: pkt=%d rule=%d",pPktHdr->sport,pCurrentCheckRule->naptFilter.ingress_dest_l4_port);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(*(pPktHdr->pSport)!=pCurrentCheckRule->naptFilter.ingress_dest_l4_port){
						DEBUG("INGRESS_DPORT UNHIT: pkt=%d rule=%d",*(pPktHdr->pSport),pCurrentCheckRule->naptFilter.ingress_dest_l4_port);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->dport!=pCurrentCheckRule->naptFilter.ingress_dest_l4_port){
						DEBUG("INGRESS_DPORT UNHIT: pkt=%d rule=%d",pPktHdr->dport,pCurrentCheckRule->naptFilter.ingress_dest_l4_port);
						goto unhit;
					}
					break;
			}			
		}
		if(pCurrentCheckRule->naptFilter.filter_fields & EGRESS_DPORT){
			if(pPktHdr->tagif&ICMP_TAGIF){
				DEBUG("EGRESS_DPORT UNHIT: pkt is ICMP rule=%d",pCurrentCheckRule->naptFilter.ingress_src_l4_port);
				goto unhit;	
			}
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(*(pPktHdr->pDport)!=pCurrentCheckRule->naptFilter.egress_dest_l4_port){
						DEBUG("EGRESS_DPORT UNHIT: pkt=%d rule=%d",*(pPktHdr->pDport),pCurrentCheckRule->naptFilter.egress_dest_l4_port);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if(pPktHdr->sport!=pCurrentCheckRule->naptFilter.egress_dest_l4_port){
						DEBUG("EGRESS_DPORT UNHIT: pkt=%d rule=%d",pPktHdr->sport,pCurrentCheckRule->naptFilter.egress_dest_l4_port);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(pPktHdr->sport!=pCurrentCheckRule->naptFilter.egress_dest_l4_port){
						DEBUG("EGRESS_DPORT UNHIT: pkt=%d rule=%d",pPktHdr->sport,pCurrentCheckRule->naptFilter.egress_dest_l4_port);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if(*(pPktHdr->pDport)!=pCurrentCheckRule->naptFilter.egress_dest_l4_port){
						DEBUG("EGRESS_DPORT UNHIT: pkt=%d rule=%d",*(pPktHdr->pDport),pCurrentCheckRule->naptFilter.egress_dest_l4_port);
						goto unhit;
					}
					break;
			}
		}
		if(pCurrentCheckRule->naptFilter.filter_fields & L4_PROTOCAL){
			if(pPktHdr->ipProtocol!=pCurrentCheckRule->naptFilter.ingress_l4_protocal){
				DEBUG("L4_PROTOCAL UNHIT: pkt=%d rule=%d",pPktHdr->ipProtocol,pCurrentCheckRule->naptFilter.ingress_l4_protocal);
				goto unhit;
			}
		}

		if(pCurrentCheckRule->naptFilter.filter_fields & INGRESS_SIP_RANGE){
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(( pPktHdr->ipv4Sip < pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_start) ||
						( pPktHdr->ipv4Sip > pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_end) ){
						DEBUG("INGRESS_SIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Sip,pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_end);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if(( pPktHdr->ipv4Dip < pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_start) ||
						( pPktHdr->ipv4Dip > pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_end) ){
						DEBUG("INGRESS_SIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_end);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(( *(pPktHdr->pIpv4Dip) < pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_start) ||
						( *(pPktHdr->pIpv4Dip) > pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_end) ){
						DEBUG("INGRESS_SIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",*(pPktHdr->pIpv4Dip),pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_end);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if(( pPktHdr->ipv4Sip < pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_start) ||
						( pPktHdr->ipv4Sip > pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_end) ){
						DEBUG("INGRESS_SIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Sip,pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.ingress_src_ipv4_addr_range_end);
						goto unhit;
					}
					break;
			}
		}
		if(pCurrentCheckRule->naptFilter.filter_fields & INGRESS_DIP_RANGE){
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((pPktHdr->ipv4Dip < pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_start) ||
						(pPktHdr->ipv4Dip > pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_end) ){
						DEBUG("INGRESS_DIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_end);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if((*(pPktHdr->pIpv4Sip) < pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_start) ||
						(*(pPktHdr->pIpv4Sip) > pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_end) ){
						DEBUG("INGRESS_DIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",*(pPktHdr->pIpv4Sip),pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_end);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((pPktHdr->ipv4Sip < pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_start) ||
						(pPktHdr->ipv4Sip > pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_end) ){
						DEBUG("INGRESS_DIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Sip,pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_end);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if((pPktHdr->ipv4Dip < pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_start) ||
						(pPktHdr->ipv4Dip > pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_end) ){
						DEBUG("INGRESS_DIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.ingress_dest_ipv4_addr_range_end);
						goto unhit;
					}
					break;
			}
		}
		if(pCurrentCheckRule->naptFilter.filter_fields & INGRESS_SPORT_RANGE){
			if(pPktHdr->tagif&ICMP_TAGIF){
				DEBUG("INGRESS_SPORT_RANGE UNHIT: pkt is ICMP rule=%d",pCurrentCheckRule->naptFilter.ingress_src_l4_port);
				goto unhit;	
			}
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((pPktHdr->sport < pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_start) ||
						(pPktHdr->sport > pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_end)){
						DEBUG("INGRESS_SPORT_RANGE UNHIT: pkt=%d rule=%d~%d",pPktHdr->sport,pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_start,pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_end);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if((pPktHdr->dport < pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_start) ||
						(pPktHdr->dport > pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_end)){
						DEBUG("INGRESS_SPORT_RANGE UNHIT: pkt=%d rule=%d~%d",pPktHdr->dport,pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_start,pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_end);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((*(pPktHdr->pDport) < pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_start) ||
						(*(pPktHdr->pDport) > pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_end)){
						DEBUG("INGRESS_SPORT_RANGE UNHIT: pkt=%d rule=%d~%d",*(pPktHdr->pDport),pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_start,pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_end);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if((pPktHdr->sport < pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_start) ||
						(pPktHdr->sport > pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_end)){
						DEBUG("INGRESS_SPORT_RANGE UNHIT: pkt=%d rule=%d~%d",pPktHdr->sport,pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_start,pCurrentCheckRule->naptFilter.ingress_src_l4_port_range_end);
						goto unhit;
					}
					break;
			}
		}		
		if(pCurrentCheckRule->naptFilter.filter_fields & INGRESS_DPORT_RANGE){
			if(pPktHdr->tagif&ICMP_TAGIF){
				DEBUG("INGRESS_DPORT_RANGE UNHIT: pkt is ICMP rule=%d",pCurrentCheckRule->naptFilter.ingress_src_l4_port);
				goto unhit;	
			}
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((pPktHdr->dport < pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_start) ||
						(pPktHdr->dport > pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_end)){
						DEBUG("INGRESS_DPORT_RANGE UNHIT: pkt=%d rule=%d~%d",pPktHdr->dport,pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_start,pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_end);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if((pPktHdr->sport < pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_start) ||
						(pPktHdr->sport > pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_end)){
						DEBUG("INGRESS_DPORT_RANGE UNHIT: pkt=%d rule=%d~%d",pPktHdr->sport,pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_start,pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_end);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((*(pPktHdr->pSport) < pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_start) ||
						(*(pPktHdr->pSport) > pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_end)){
						DEBUG("INGRESS_DPORT_RANGE UNHIT: pkt=%d rule=%d~%d",*(pPktHdr->pSport),pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_start,pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_end);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if((pPktHdr->dport < pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_start) ||
						(pPktHdr->dport > pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_end)){
						DEBUG("INGRESS_DPORT_RANGE UNHIT: pkt=%d rule=%d~%d",pPktHdr->dport,pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_start,pCurrentCheckRule->naptFilter.ingress_dest_l4_port_range_end);
						goto unhit;
					}
					break;
			}
		}


		if(pCurrentCheckRule->naptFilter.filter_fields & EGRESS_SIP_RANGE){
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(( *(pPktHdr->pIpv4Sip) < pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_start) ||
						( *(pPktHdr->pIpv4Sip) > pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_end) ){
						DEBUG("EGRESS_SIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",*(pPktHdr->pIpv4Sip),pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_end);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if(( pPktHdr->ipv4Dip < pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_start) ||
						( pPktHdr->ipv4Dip > pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_end) ){
						DEBUG("EGRESS_SIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_end);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if(( pPktHdr->ipv4Dip < pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_start) ||
						( pPktHdr->ipv4Dip > pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_end) ){
						DEBUG("EGRESS_SIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_end);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if(( *(pPktHdr->pIpv4Sip) < pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_start) ||
						( *(pPktHdr->pIpv4Sip) > pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_end) ){
						DEBUG("EGRESS_SIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",*(pPktHdr->pIpv4Sip),pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.egress_src_ipv4_addr_range_end);
						goto unhit;
					}
					break;
			}
		}


		if(pCurrentCheckRule->naptFilter.filter_fields & EGRESS_DIP_RANGE){
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((pPktHdr->ipv4Dip < pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_start) ||
						(pPktHdr->ipv4Dip > pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_end) ){
						DEBUG("EGRESS_DIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Dip,pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_end);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if((pPktHdr->ipv4Sip < pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_start) ||
						(pPktHdr->ipv4Sip > pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_end) ){
						DEBUG("EGRESS_DIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Sip,pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_end);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((pPktHdr->ipv4Sip < pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_start) ||
						(pPktHdr->ipv4Sip > pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_end) ){
						DEBUG("EGRESS_DIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",pPktHdr->ipv4Sip,pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_end);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if((*(pPktHdr->pIpv4Dip) < pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_start) ||
						(*(pPktHdr->pIpv4Dip) > pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_end) ){
						DEBUG("EGRESS_DIP_RANGE UNHIT: pkt=0x%x rule=0x%x~0x%x",*(pPktHdr->pIpv4Dip),pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_start,pCurrentCheckRule->naptFilter.egress_dest_ipv4_addr_range_end);
						goto unhit;
					}
					break;
			}
		}


		if(pCurrentCheckRule->naptFilter.filter_fields & EGRESS_SPORT_RANGE){
			if(pPktHdr->tagif&ICMP_TAGIF){
				DEBUG("INGRESS_SPORT_RANGE UNHIT: pkt is ICMP rule=%d",pCurrentCheckRule->naptFilter.ingress_src_l4_port);
				goto unhit; 
			}
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((*(pPktHdr->pSport) < pCurrentCheckRule->naptFilter.egress_src_l4_port_range_start) ||
						(*(pPktHdr->pSport) > pCurrentCheckRule->naptFilter.egress_src_l4_port_range_end)){
						DEBUG("EGRESS_SPORT_RANGE UNHIT: pkt=%d rule=%d~%d",*(pPktHdr->pSport),pCurrentCheckRule->naptFilter.egress_src_l4_port_range_start,pCurrentCheckRule->naptFilter.egress_src_l4_port_range_end);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if((pPktHdr->dport < pCurrentCheckRule->naptFilter.egress_src_l4_port_range_start) ||
						(pPktHdr->dport > pCurrentCheckRule->naptFilter.egress_src_l4_port_range_end)){
						DEBUG("EGRESS_SPORT_RANGE UNHIT: pkt=%d rule=%d~%d",pPktHdr->dport,pCurrentCheckRule->naptFilter.egress_src_l4_port_range_start,pCurrentCheckRule->naptFilter.egress_src_l4_port_range_end);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((pPktHdr->dport < pCurrentCheckRule->naptFilter.egress_src_l4_port_range_start) ||
						(pPktHdr->dport > pCurrentCheckRule->naptFilter.egress_src_l4_port_range_end)){
						DEBUG("EGRESS_SPORT_RANGE UNHIT: pkt=%d rule=%d~%d",pPktHdr->dport,pCurrentCheckRule->naptFilter.egress_src_l4_port_range_start,pCurrentCheckRule->naptFilter.egress_src_l4_port_range_end);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if((*(pPktHdr->pSport) < pCurrentCheckRule->naptFilter.egress_src_l4_port_range_start) ||
						(*(pPktHdr->pSport) > pCurrentCheckRule->naptFilter.egress_src_l4_port_range_end)){
						DEBUG("EGRESS_SPORT_RANGE UNHIT: pkt=%d rule=%d~%d",*(pPktHdr->pSport),pCurrentCheckRule->naptFilter.egress_src_l4_port_range_start,pCurrentCheckRule->naptFilter.egress_src_l4_port_range_end);
						goto unhit;
					}
					break;
			}
		}


		if(pCurrentCheckRule->naptFilter.filter_fields & EGRESS_DPORT_RANGE){
			if(pPktHdr->tagif&ICMP_TAGIF){
				DEBUG("INGRESS_DPORT_RANGE UNHIT: pkt is ICMP rule=%d",pCurrentCheckRule->naptFilter.ingress_src_l4_port);
				goto unhit; 
			}
			switch(check_direct){
				case CHECK_OUTBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((*(pPktHdr->pDport) < pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_start) ||
						(*(pPktHdr->pDport) > pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_end)){
						DEBUG("EGRESS_DPORT_RANGE UNHIT: pkt=%d rule=%d~%d",*(pPktHdr->pDport),pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_start,pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_end);
						goto unhit;
					}
					break;
				case CHECK_OUTBOUND_PKT_WITH_INBOUND_RULE: 
					if((pPktHdr->sport < pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_start) ||
						(pPktHdr->sport > pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_end)){
						DEBUG("EGRESS_DPORT_RANGE UNHIT: pkt=%d rule=%d~%d",pPktHdr->sport,pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_start,pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_end);
						goto unhit;
					}
					break;			
				case CHECK_INBOUND_PKT_WITH_OUTBOUND_RULE: 
					if((pPktHdr->sport < pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_start) ||
						(pPktHdr->sport > pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_end)){
						DEBUG("EGRESS_DPORT_RANGE UNHIT: pkt=%d rule=%d~%d",pPktHdr->sport,pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_start,pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_end);
						goto unhit;
					}
					break;
				case CHECK_INBOUND_PKT_WITH_INBOUND_RULE: 
					if((*(pPktHdr->pDport) < pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_start) ||
						(*(pPktHdr->pDport) > pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_end)){
						DEBUG("EGRESS_DPORT_RANGE UNHIT: pkt=%d rule=%d~%d",*(pPktHdr->pDport),pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_start,pCurrentCheckRule->naptFilter.egress_dest_l4_port_range_end);
						goto unhit;
					}
					break;
			}
		}

		
//hit:
		if(rule_direction==RG_FWD_DECISION_NAPT){
			TRACE("Hit Upstream NaptFilter[%d]",pCurrentCheckRule->sw_index);
		}else{
			TRACE("Hit Downstream NaptFilter[%d]",pCurrentCheckRule->sw_index);
		}
		return pCurrentCheckRule->sw_index;

unhit:
		pCurrentCheckRule = pCurrentCheckRule->pNextValid;
		i++;
	}

	return RG_RET_ENTRY_NOT_GET;

}


rtk_rg_fwdEngineReturn_t _rtk_rg_naptPriority_assign(int direct, rtk_rg_pktHdr_t *pPktHdr, rtk_rg_table_naptOut_t *naptout_entry, rtk_rg_table_naptIn_t *naptin_entry){

	//int ret;
	rtk_rg_entryGetReturn_t outBoundHitIdx = RG_RET_ENTRY_NOT_GET;
	rtk_rg_entryGetReturn_t inBoundHitIdx = RG_RET_ENTRY_NOT_GET;
	
	outBoundHitIdx = _rtk_rg_naptPriority_pattern_check(RG_FWD_DECISION_NAPT, direct ,pPktHdr, rg_db.systemGlobal.pValidUsNaptPriorityRuleStart);
	if(outBoundHitIdx!=RG_RET_ENTRY_NOT_GET){

		if(rg_db.systemGlobal.napt_SW_table_entry[outBoundHitIdx].naptFilter.action_fields & NAPT_DROP_BIT){
			TRACE("Drop by naptFilter!");
			return RG_FWDENGINE_RET_DROP;
		}
		else if(rg_db.systemGlobal.napt_SW_table_entry[outBoundHitIdx].naptFilter.action_fields & NAPT_PERMIT_BIT)
		{
			TRACE("Permit naptFilter!");
			//do nothing
		}
		else if(rg_db.systemGlobal.napt_SW_table_entry[outBoundHitIdx].naptFilter.action_fields & ASSIGN_NAPT_PRIORITY_BIT){
			if(pPktHdr->tagif&ICMP_TAGIF){
				TRACE("ICMP packet not suport napt_out priority assign! The naptFilter[%d] is useless!",outBoundHitIdx);
			}else{
			//sw
			naptout_entry->priValid=1;
			naptout_entry->priValue=rg_db.systemGlobal.napt_SW_table_entry[outBoundHitIdx].naptFilter.assign_priority;
			//hw
			naptout_entry->rtk_naptOut.priValid=1;
			naptout_entry->rtk_naptOut.priValue=rg_db.systemGlobal.napt_SW_table_entry[outBoundHitIdx].naptFilter.assign_priority;

			TRACE("assign napt_out[%d] priority to %d",naptout_entry->hashOutIdx,naptout_entry->priValue);
			}
		}
		
		if(rg_db.systemGlobal.napt_SW_table_entry[outBoundHitIdx].naptFilter.ruleType==RTK_RG_NAPT_FILTER_ONE_SHOT){
			(pf.rtk_rg_naptFilterAndQos_del)(outBoundHitIdx);
			TRACE("naptFilter[%d] outbound one shot rule is hit and removed!",outBoundHitIdx);
		}

	}

	inBoundHitIdx = _rtk_rg_naptPriority_pattern_check(RG_FWD_DECISION_NAPTR, direct, pPktHdr, rg_db.systemGlobal.pValidDsNaptPriorityRuleStart);	
	if(inBoundHitIdx!=RG_RET_ENTRY_NOT_GET){
		if(rg_db.systemGlobal.napt_SW_table_entry[inBoundHitIdx].naptFilter.action_fields & NAPT_DROP_BIT){
			TRACE("Drop by naptFilter!");
			return RG_FWDENGINE_RET_DROP;
		}
		else if(rg_db.systemGlobal.napt_SW_table_entry[inBoundHitIdx].naptFilter.action_fields & NAPT_PERMIT_BIT)
		{
			TRACE("Permit naptFilter!");
			//do nothing
		}
		else if(rg_db.systemGlobal.napt_SW_table_entry[inBoundHitIdx].naptFilter.action_fields & ASSIGN_NAPT_PRIORITY_BIT){

			if(pPktHdr->tagif&ICMP_TAGIF){
				TRACE("ICMP packet not suport napt_in priority assign! The naptFilter[%d] is useless!",inBoundHitIdx);
			}else{
			//sw
			naptin_entry->priValid=1;
			naptin_entry->priValue=rg_db.systemGlobal.napt_SW_table_entry[inBoundHitIdx].naptFilter.assign_priority;
			//hw
			naptin_entry->rtk_naptIn.priValid=1;
			naptin_entry->rtk_naptIn.priId=rg_db.systemGlobal.napt_SW_table_entry[inBoundHitIdx].naptFilter.assign_priority;
			TRACE("assign napt_in[%d] priority to %d",naptin_entry->hashIdx,naptin_entry->priValue);
			}
		}
		
		if(rg_db.systemGlobal.napt_SW_table_entry[inBoundHitIdx].naptFilter.ruleType==RTK_RG_NAPT_FILTER_ONE_SHOT){
			(pf.rtk_rg_naptFilterAndQos_del)(inBoundHitIdx);
			TRACE("naptFilter[%d] inbound one shot rule is hit and removed!",inBoundHitIdx);
		}
		
	}

	return RG_FWDENGINE_RET_CONTINUE;
}
#endif


__IRAM_FWDENG
void _rtk_rg_tcpShortTimeoutRecycle(uint16 naptOutIdx)
{
	rg_db.tcpShortTimeoutRing[rg_db.tcpShortTimeoutRecycleIdx].naptOutIdx=naptOutIdx;
	rg_db.tcpShortTimeoutRing[rg_db.tcpShortTimeoutRecycleIdx].timeoutJiffies=(u32)jiffies+rg_db.systemGlobal.tcpShortTimeoutHousekeepJiffies;
	rg_db.tcpShortTimeoutRecycleIdx++;
	if(rg_db.tcpShortTimeoutRecycleIdx>=MAX_NAPT_OUT_SW_TABLE_SIZE)
		rg_db.tcpShortTimeoutRecycleIdx=0;
}
__IRAM_FWDENG
void _rtk_rg_fwdEngine_receivedTCPReset(rtk_rg_pktHdr_t *pPktHdr, int *pNaptOutIdx)
{
	int inIdx;
	
	//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
	rg_db.naptOut[*pNaptOutIdx].state=RST_RECV;

	if(rg_db.systemGlobal.tcpDoNotDelWhenRstFin==0)
	{
		if(rg_db.systemGlobal.tcpShortTimeoutHousekeepJiffies!=0)
		{
			_rtk_rg_tcpShortTimeoutRecycle(*pNaptOutIdx);
		}
		else
		{
			if(rg_db.systemGlobal.tcp_short_timeout<=1) pPktHdr->delNaptConnection=1;
		}
	}
	else
	{
		inIdx=rg_db.naptOut[*pNaptOutIdx].rtk_naptOut.hashIdx;
		//20151208LUKE: we must make sure both inbound and outbound entries are locate in hw table, otherwise this flow can't be replace.
		if((*pNaptOutIdx<MAX_NAPT_OUT_HW_TABLE_SIZE)&&(inIdx<MAX_NAPT_IN_HW_TABLE_SIZE)){
			rg_db.naptOut[*pNaptOutIdx].canBeReplaced=1;
			rg_db.naptIn[inIdx].canBeReplaced=1;
		}
	}	
}
__IRAM_FWDENG
void _rtk_rg_fwdEngine_receivedTCPFin(rtk_rg_pktHdr_t *pPktHdr, int *pNaptOutIdx, rtk_rg_naptState_t newState)
{
	int inIdx;
	//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);	
	if(rg_db.naptOut[*pNaptOutIdx].state==TCP_CONNECTED)
	{
		rg_db.naptOut[*pNaptOutIdx].state=newState;
		
#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)/*xdsl : alway delete hw table when fin send or receive Boyce 2015-06-08*/
		inIdx=rg_db.naptOut[*pNaptOutIdx].rtk_naptOut.hashIdx;
		if(rg_db.naptOut[*pNaptOutIdx].state==FIRST_FIN)
		{
			// delete HW entry and shortcut but keep SW entry for the fin handshake.
			rg_db.naptOut[*pNaptOutIdx].rtk_naptOut.valid=ASIC_NAPT_IN_TYPE_INVALID; //let outbound last ack is able to handle by software. 			
			RTK_L34_NAPTOUTBOUNDTABLE_SET(1,*pNaptOutIdx,&rg_db.naptOut[*pNaptOutIdx].rtk_naptOut);
			rg_db.naptOut[*pNaptOutIdx].rtk_naptOut.valid=1;
			TABLE("del NAPT HW entry out by first fin[%d]",*pNaptOutIdx);

			if(rg_db.naptIn[inIdx].rtk_naptIn.valid==ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE)
			{
				rg_db.naptIn[inIdx].rtk_naptIn.valid=ASIC_NAPT_IN_TYPE_INVALID; //let inbound last ack is able to handle by software.
				RTK_L34_NAPTINBOUNDTABLE_SET(1,inIdx,&rg_db.naptIn[inIdx].rtk_naptIn);
				rg_db.naptIn[inIdx].rtk_naptIn.valid=ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE;
				TABLE("del NAPT HW entry in by first fin[%d]",inIdx);
			}	

			_rtk_rg_delNaptShortCutEntrybyOutboundIdx(*pNaptOutIdx);

			//update idle time 
			rg_db.naptIn[inIdx].idleSecs=0;
			rg_db.naptOut[*pNaptOutIdx].idleSecs=0;
		}
#endif
		if(rg_db.systemGlobal.tcpDoNotDelWhenRstFin==1)
		{	
			inIdx=rg_db.naptOut[*pNaptOutIdx].rtk_naptOut.hashIdx;
			//20151208LUKE: we must make sure both inbound and outbound entries are locate in hw table, otherwise this flow can't be replace.
			if((*pNaptOutIdx<MAX_NAPT_OUT_HW_TABLE_SIZE)&&(inIdx<MAX_NAPT_IN_HW_TABLE_SIZE)){
				rg_db.naptOut[*pNaptOutIdx].canBeReplaced=1;
				rg_db.naptIn[inIdx].canBeReplaced=1;
			}
		}
		else if(rg_db.systemGlobal.tcpShortTimeoutHousekeepJiffies!=0)
		{
			_rtk_rg_tcpShortTimeoutRecycle(*pNaptOutIdx);
		}
	}
	else
	{
		rg_db.naptOut[*pNaptOutIdx].state=FIN_SEND_AND_RECV;
		inIdx=rg_db.naptOut[*pNaptOutIdx].rtk_naptOut.hashIdx;

#if !defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) // xdsl:del hw table at first fin so ingore
		if(rg_db.systemGlobal.tcpDoNotDelWhenRstFin==0)
		{

			if(rg_db.systemGlobal.tcp_short_timeout<=1)
			{				
				// delete HW entry but keep software entry for the last ack.
				rg_db.naptOut[*pNaptOutIdx].rtk_naptOut.valid=ASIC_NAPT_IN_TYPE_INVALID; //let outbound last ack is able to handle by software. 			
				RTK_L34_NAPTOUTBOUNDTABLE_SET(1,*pNaptOutIdx,&rg_db.naptOut[*pNaptOutIdx].rtk_naptOut);
				rg_db.naptOut[*pNaptOutIdx].rtk_naptOut.valid=1;
				TABLE("del NAPT HW entry out[%d]",*pNaptOutIdx);

				
				if(rg_db.naptIn[inIdx].rtk_naptIn.valid==ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE)
				{
					rg_db.naptIn[inIdx].rtk_naptIn.valid=ASIC_NAPT_IN_TYPE_INVALID; //let inbound last ack is able to handle by software.
					RTK_L34_NAPTINBOUNDTABLE_SET(1,inIdx,&rg_db.naptIn[inIdx].rtk_naptIn);
					rg_db.naptIn[inIdx].rtk_naptIn.valid=ASIC_NAPT_IN_TYPE_PORT_RESTRICTED_CONE;
					TABLE("del NAPT HW entry in[%d]",inIdx);
				}	

				_rtk_rg_delNaptShortCutEntrybyOutboundIdx(*pNaptOutIdx);
			}
		}
#endif
	}
				
	//if(rg_db.systemGlobal.tcp_short_timeout<=1) pPktHdr->delNaptConnection=1;
}
static char* rgHttpRedirectHead = 
	"HTTP/1.1 302 Object Moved\r\n"
	"Location: http://%s%s\r\n"
	"Server: adsl-router-gateway\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: %d\r\n"
	"\r\n"
	"%s";
static char* rgHttpRedirectContent = 
	"<html><head><title>Object Moved</title></head>"
	"<body><h1>Object Moved</h1>This Object may be found in "
	"<a HREF=\"http://%s%s\">here</a>.</body><html>";


#if defined(CONFIG_APOLLO_ROMEDRIVER) || defined(CONFIG_XDSL_ROMEDRIVER)
static char szRedirectPack[512];
static char szRedirectContent[260];
#endif
int _rtk_rg_redirectGeneration(rtk_rg_pktHdr_t *pPktHdr, int type_idx, char *url_string)
{
#ifdef __KERNEL__
#if defined(CONFIG_APOLLO_ROMEDRIVER) || defined(CONFIG_XDSL_ROMEDRIVER)
	struct sk_buff *skb;
	unsigned char *bufptr;
	int ret_code=0;
	int vlan_offset=0;
	char *att_url="";
	char *fqdn=rg_db.systemGlobal.urlFilter_parsingBuf;
	char *url_head=NULL;
	char *url_tail=NULL;
	char *path_head=NULL;
	char *path_tail=NULL;

	//use rsvd to save the netif idx for Layer2 forward
	rg_kernel.rxInfoFromARPND.opts2.bit.rsvd_1=pPktHdr->srcNetifIdx;
		
	skb=_rtk_rg_dev_alloc_skb(RG_FWDENGINE_PKT_LEN);
	//call fwdEngineInput, the alloc counter will be added. so don't need to add again
	
	if((skb)&&(rg_db.systemGlobal.fwdStatistic))
	{
#if RTK_RG_SKB_PREALLOCATE
		rg_db.systemGlobal.statistic.perPortCnt_skb_pre_alloc_for_uc[pPktHdr->ingressPort]--;
#else
		rg_db.systemGlobal.statistic.perPortCnt_skb_alloc[pPktHdr->ingressPort]--;
#endif
	}
	
	bzero(skb->data,RG_FWDENGINE_PKT_LEN);//clean ptk buffer
	skb_reserve(skb, RX_OFFSET);
	bufptr=skb->data;

	/* Construct destination MAC */
	memcpy(bufptr,pPktHdr->pSmac,ETHER_ADDR_LEN);

	/* Construct source MAC */
	memcpy(bufptr + 6,pPktHdr->pDmac,ETHER_ADDR_LEN);

	if(pPktHdr->tagif&CVLAN_TAGIF)
	{
		unsigned short ctagdata=0;
		vlan_offset=4;
		*(uint16 *)(bufptr + 12)= htons(0x8100);
		ctagdata=(pPktHdr->ctagVid&0xfff);
		ctagdata|=((pPktHdr->ctagCfi&1)<<12);
		ctagdata|=((pPktHdr->ctagPri&0x7)<<13);
		*(uint16 *)(bufptr + 14)= htons(ctagdata);
	}
	
	/* construct IP header */
	*(uint32 *)(bufptr + 12 + vlan_offset) = htonl(0x08004500);							//EtherType, version, header length, DS field
	*(uint16 *)(bufptr + 18 + vlan_offset) = htons(0);									//Identification
	*(uint32 *)(bufptr + 20 + vlan_offset) = htonl(0x4000ff06);							//Flags, Fragment offset, TTL, protocol=TCP
	*(uint16 *)(bufptr + 24 + vlan_offset) = htons(0x0);								//Header checksum(caculate by hw)
	*(uint32 *)(bufptr + 26 + vlan_offset) = htonl(pPktHdr->ipv4Dip);					//SIP
	*(uint32 *)(bufptr + 30 + vlan_offset) = htonl(pPktHdr->ipv4Sip);					//DIP
	
	/* construct TCP header */
	*(uint16 *)(bufptr + 34 + vlan_offset) = htons(pPktHdr->dport);						//SPORT
	*(uint16 *)(bufptr + 36 + vlan_offset) = htons(pPktHdr->sport);						//DPORT
	*(uint32 *)(bufptr + 38 + vlan_offset) = htonl(pPktHdr->tcpAck);					//Sequence
	*(uint32 *)(bufptr + 42 + vlan_offset) = htonl(pPktHdr->tcpSeq+(pPktHdr->l3Len-pPktHdr->ipv4HeaderLen-pPktHdr->headerLen));					//Acknowledgment
	*(uint16 *)(bufptr + 46 + vlan_offset) = htons(0x5018);								//DataOff, Flag
	*(uint16 *)(bufptr + 48 + vlan_offset) = htons(pPktHdr->tcpWindow);					//Window
	*(uint32 *)(bufptr + 50 + vlan_offset) = htonl(0);									//Checksum, urgent point

	//20160113LUKE: if we got pushweb content, use it as payload.
	if(rg_db.redirectHttpAll.enable){		
		//20160204LUKE: if enable is 2, we should write original URL to pushweb!!
		if(rg_db.redirectHttpAll.enable==2){
			_url_parsing_string(pPktHdr, fqdn, url_head, url_tail, path_head, path_tail);
			if(fqdn){
				//cut final '/' if necessary
				if(fqdn[strlen(fqdn)-1]=='/')fqdn[strlen(fqdn)-1]='\0';
				att_url=fqdn;
			}else{
				TRACE("[REDIRECT]can't parse fqdn string, Drop it!");
				_rtk_rg_dev_kfree_skb_any(skb);
				return 0;
			}
			sprintf(bufptr+54+vlan_offset,rg_db.redirectHttpAll.pushweb,att_url);
		}else
			memcpy(bufptr+54+vlan_offset, rg_db.redirectHttpAll.pushweb, strlen(rg_db.redirectHttpAll.pushweb));
		*(uint16 *)(bufptr + 16 + vlan_offset) = htons(40+strlen(rg_db.redirectHttpAll.pushweb)+strlen(att_url));			//Total length
		skb->len=54+vlan_offset+strlen(rg_db.redirectHttpAll.pushweb)+strlen(att_url);
	}else{
		//20160113LUKE: for redirectHttpURL, we redirect to specific destination URL.
		if(url_string!=NULL){
			sprintf(szRedirectContent, rgHttpRedirectContent, url_string, att_url);
			sprintf(szRedirectPack, rgHttpRedirectHead, url_string, att_url,  
				strlen(szRedirectContent), szRedirectContent); 
		}else{
			//20160114LUKE: if we get dst_URL end as "&url=", we need to attach original URL to it.
			if(rg_db.systemGlobal.forcePortal_url_list[type_idx].attach_orig_url){
				_url_parsing_string(pPktHdr, fqdn, url_head, url_tail, path_head, path_tail);
				if(fqdn){
					//cut final '/' if necessary
					if(fqdn[strlen(fqdn)-1]=='/')fqdn[strlen(fqdn)-1]='\0';
					att_url=fqdn;
				}else{
					TRACE("[REDIRECT]can't parse fqdn string, Drop it!");
					_rtk_rg_dev_kfree_skb_any(skb);
					return 0;
				}
			}
			sprintf(szRedirectContent, rgHttpRedirectContent, rg_db.systemGlobal.forcePortal_url_list[type_idx].url_string, att_url);
			sprintf(szRedirectPack, rgHttpRedirectHead, rg_db.systemGlobal.forcePortal_url_list[type_idx].url_string, att_url,  
				strlen(szRedirectContent), szRedirectContent); 
		}
		memcpy(bufptr+54+vlan_offset, szRedirectPack, strlen(szRedirectPack));
		*(uint16 *)(bufptr + 16 + vlan_offset) = htons(40+strlen(szRedirectPack));			//Total length
		skb->len=54+vlan_offset+strlen(szRedirectPack);
	}
	//dump_packet(skb->data,skb->len,"redirect_dump");

	*(u32*)(skb->data+skb->len)=0; //save null point into end of skb data.(for trace filter debug)

	//backup original pkthdr
	rg_db.pktHdr=&rg_db.systemGlobal.pktHeader_2;
#if defined(CONFIG_APOLLO_ROMEDRIVER) || defined(CONFIG_XDSL_ROMEDRIVER)

	ret_code = rtk_rg_fwdEngineInput(NULL,skb,(void*)&rg_kernel.rxInfoFromARPND);

	//Processing packets
	if(ret_code == RG_FWDENGINE_RET_TO_PS)
	{
		//FIXME:iPhone 5 change wireless connection from master to slave will send strange unicast ARP request for LAN gateway IP, and forwarded by protocol stack
		TRACE("RED_GEN[%x]: To Protocol-Stack...FREE SKB!!",(unsigned int)skb&0xffff);
		//dump_packet(skb->data,skb->len,"dump_back_to_PS");
		_rtk_rg_dev_kfree_skb_any(skb);
	}
	else if (ret_code == RG_FWDENGINE_RET_DROP)
	{
		TRACE("RED_GEN[%x]: Drop...FREE SKB!!",(unsigned int)skb&0xffff);
		_rtk_rg_dev_kfree_skb_any(skb);
	}	
	else
	{
		TRACE("RED_GEN[%x]: Forward",(unsigned int)skb&0xffff);
	}
	//point back to original pkthdr
	rg_db.pktHdr=&rg_db.systemGlobal.pktHeader_1;
#else
	ret_code=_rtk_rg_broadcastForward(skb,rg_db.systemGlobal.interfaceInfo[netIfIdx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id,RTK_RG_MAC_PORT_CPU,0);
	if(ret_code==RG_FWDENGINE_RET_DROP)
		_rtk_rg_dev_kfree_skb_any(skb);
#endif
	//memDump(bufptr,skb->len,"ARPGEN");	
	//rtk_rg_fwdEngine_xmit(skb,&txInfo,NULL);
	//re8686_send_with_txInfo(skb,&txInfo,0);
	//re8686_send_with_txInfo_and_mask(skb,&txInfo,0,&txInfoMask);
#else
	//FIXME:in module code, we need some other api to send packets
#endif
#endif	

	return 0;
}

rtk_rg_fwdEngineReturn_t _forcePortal_check(rtk_rg_pktHdr_t *pPktHdr, int *pNaptOutIdx, int type_idx)
{
	int i;
	
	//fragment won't handle
	if(pPktHdr->ipv4FragPacket||pPktHdr->ipv6FragPacket)
		return RG_FWDENGINE_RET_CONTINUE;

	if(pPktHdr->pL4Payload==NULL){
		TRACE("ForcePortal get a Empty payload packet, drop here!\n");	
		return RG_FWDENGINE_RET_DROP;
	}

	//check if match http command
	for(i=0; rg_http_request_cmd[i]; i++) {
		if (memcmp(pPktHdr->pL4Payload, rg_http_request_cmd[i], strlen(rg_http_request_cmd[i])) == 0) {
			goto SEND_REDIRECT;
		}
	}
	return RG_FWDENGINE_RET_CONTINUE;
	
	//send back redirect http
SEND_REDIRECT:
	_rtk_rg_redirectGeneration(pPktHdr, type_idx, NULL);

	rg_db.lut[pPktHdr->smacL2Idx].redirect_http_req=0;
	
	//reset the original connection
	//pPktHdr->tcpFlags.reset=1;
	//_rtk_rg_fwdEngine_receivedTCPReset(pPktHdr, pNaptOutIdx);
	//Delete connection and shortcut
	(pf.rtk_rg_naptConnection_del)(*pNaptOutIdx);
	return RG_FWDENGINE_RET_DROP;
}

rtk_rg_fwdEngineReturn_t _check_Http_URL(rtk_rg_pktHdr_t *pPktHdr, int *pNaptOutIdx)
{
	int match_white_list;
	rtk_rg_redirectHttpURL_linkList_t *pURLEntry;
	rtk_rg_redirectHttpWhiteList_linkList_t *pWhiteEntry;
	char *fqdn=rg_db.systemGlobal.urlFilter_parsingBuf;
	char *url_head=NULL;
	char *url_tail=NULL;
	char *path_head=NULL;
	char *path_tail=NULL;
	char *keyword_head=NULL;

	if(pPktHdr->pL4Payload==NULL){
		TRACE("[REDIRECT_URL]get a empty payload packet, drop it!");
		return RG_FWDENGINE_RET_DROP;
	}
	
	_url_parsing_string(pPktHdr, fqdn, url_head, url_tail, path_head, path_tail);
	
	if(fqdn==NULL){
		TRACE("[REDIRECT_URL]can't parse fqdn string, drop it!");
		return RG_FWDENGINE_RET_DROP;
	}else{
		DEBUG("Check fqdn is %s",fqdn);
		list_for_each_entry(pURLEntry,&rg_db.redirectHttpURLListHead,url_list){
			if(atomic_read(&pURLEntry->count)!=0){
				DEBUG("pRedEntry->count is %d, str is %s",atomic_read(&pURLEntry->count),pURLEntry->url_data.url_str);
				//compare URL first
				if(!strncmp(fqdn,pURLEntry->url_data.url_str,pURLEntry->url_len)){//match!!
					//match URL, then compare White list
					keyword_head=strstr(fqdn,"?");
					DEBUG("keyword string is %s",keyword_head);
					if(keyword_head==NULL || list_empty(&rg_db.redirectHttpWhiteListListHead)){
						if(atomic_dec_return(&pURLEntry->count)<0)atomic_set(&pURLEntry->count,-1);
						DEBUG("no keyword or no whiteList, do redirect..");
						goto DO_REDIRECT;
					}else{
						match_white_list=0;
						list_for_each_entry(pWhiteEntry,&rg_db.redirectHttpWhiteListListHead,white_list){
							DEBUG("check white list %s %s...",pWhiteEntry->white_data.url_str,pWhiteEntry->white_data.keyword_str);
							if(!strncmp(fqdn,pWhiteEntry->white_data.url_str,pWhiteEntry->url_len) &&
								strstr(keyword_head,pWhiteEntry->white_data.keyword_str)!=NULL){
								//match white list, should not be redirected!!
								DEBUG("hit white list!! continue!!");
								match_white_list=1;
								break;
							}
						}
						if(match_white_list){
							break;	//leave outside list_for_each_entry
						}else{
							if(atomic_dec_return(&pURLEntry->count)<0)atomic_set(&pURLEntry->count,-1);
							DEBUG("not hit whiteList, do redirect..");
							goto DO_REDIRECT;
						}
					}
				}
			}
		}
	}
	
	return RG_FWDENGINE_RET_CONTINUE;
	
DO_REDIRECT:
	DEBUG("Redirect to dstURL:%s",pURLEntry->url_data.dst_url_str);
	_rtk_rg_redirectGeneration(pPktHdr, 0, pURLEntry->url_data.dst_url_str);
	
	//reset the original connection
	//pPktHdr->tcpFlags.reset=1;
	//_rtk_rg_fwdEngine_receivedTCPReset(pPktHdr, pNaptOutIdx);
	//Delete connection and shortcut
	(pf.rtk_rg_naptConnection_del)(*pNaptOutIdx);
	return RG_FWDENGINE_RET_DROP;
}

void _user_agent_parsing_string(rtk_rg_pktHdr_t *pPktHdr, char *user_agent_str, char *str_head, char *str_tail)
{	
	char space = 0x20;
	char sep[3];
	char *capStr;
	int str_len = 0;
	
	/*use for parsing Host:*/
	sep[0]=0x0d;
	sep[1]=0x0a;
	sep[2]='\0';

	bzero(user_agent_str,sizeof(user_agent_str));

	if(pPktHdr->pL4Payload!=NULL){
		//rtlglue_printf("path_tail:%s.\n",url_tail);
		//DEBUG("[USERAG_PARSER]Lookup for \"Referer:\"\n");
		str_head = strstr(pPktHdr->pL4Payload, "User-Agent:");
		if(str_head!=NULL){
			str_head = strchr(str_head, space);//cut the "User-Agent:"
			if(str_head!=NULL)
				str_head = &str_head[1];//cut the " "
		}
	}else{
		DEBUG("[USERAG_PARSER]pL4Payload is null\n");
	}

	if(str_head!=NULL){
		//DEBUG("str_head:%s.\n",str_head);
		str_tail = strstr(str_head, sep);
	}else{
		DEBUG("[USERAG_PARSER]str_head is null\n");
	}


	if(str_head!=NULL && str_tail!=NULL){
		str_len = str_tail - str_head;
		if(str_len>=MAX_URL_FILTER_BUF_LENGTH){
			//dump_packet(pData,len,"urlFilter Packet");
			DEBUG("[USERAG_PARSER]user agent too long str_len=%d!!!\n",str_len);
			strncpy(user_agent_str,str_head,MAX_URL_FILTER_BUF_LENGTH);
			user_agent_str[MAX_URL_FILTER_BUF_LENGTH-1]='\0';
		}else{
			strncpy(user_agent_str,str_head,str_len);
			user_agent_str[str_len]='\0';
		}
		capStr = user_agent_str;
		//Captilize all letters in user-agent string
		while(*capStr != '\0')
            if (*capStr >= 'a' && *capStr <= 'z')
				*(capStr)-=0x20;
			else 
				capStr++;
	}else{
		//dump_packet(pData,len,"urlFilter Packet");
		DEBUG("user_agent_str: can not parse... \n");
		user_agent_str=NULL;
	}
}

rtk_rg_fwdEngineReturn_t _parse_Http_UserAgent(rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_arpInfo_t arpInfo;
	int i,j,arpIdx;
	rtk_rg_lanNet_device_type_t device_type;
	rtk_rg_lanNet_brand_t brand=RG_BRAND_OTHER;
	rtk_rg_lanNet_os_t os_type=RG_OS_OTHER;
	char *user_agent_str=rg_db.systemGlobal.urlFilter_parsingBuf;
	char *str_head=NULL;
	char *str_tail=NULL;

	if(pPktHdr->pL4Payload==NULL){
		TRACE("[USERAG_PARSER]get a empty payload packet...");
		return RG_FWDENGINE_RET_CONTINUE;
	}

	_user_agent_parsing_string(pPktHdr, user_agent_str, str_head, str_tail);
	
	if(user_agent_str==NULL){
		TRACE("[USERAG_PARSER]can't parse user agent string...");
		return RG_FWDENGINE_RET_CONTINUE;
	}else{
		DEBUG("Check user_agent_str is %s",user_agent_str);
		//Check for device type
		for(i=0;rg_lanNet_phone_type[i]!=NULL;i++){
			if(strstr(user_agent_str,rg_lanNet_phone_type[i])){
				device_type=RG_LANNET_TYPE_PHONE;
				break;
			}
		}
		if(rg_lanNet_phone_type[i]==NULL){
			for(i=0;rg_lanNet_computer_type[i]!=NULL;i++){
				if(strstr(user_agent_str,rg_lanNet_computer_type[i])){
					device_type=RG_LANNET_TYPE_COMPUTER;
					break;
				}
			}
		}
		if(rg_lanNet_computer_type[i]==NULL)device_type=RG_LANNET_TYPE_OTHER;
		DEBUG("Device type is %s",device_type==RG_LANNET_TYPE_PHONE?"Phone.":device_type==RG_LANNET_TYPE_COMPUTER?"Computer.":"Other.");
		//Check for brand
		for(i=RG_BRAND_OTHER+1;i<RG_BRAND_END;i++)
			for(j=0;rg_lanNet_brand[i][j]!=NULL;j++)
				if(strstr(user_agent_str, rg_lanNet_brand[i][j])){
					brand=(rtk_rg_lanNet_brand_t)i;
					i=RG_BRAND_END;
					break;
				}
		DEBUG("BRAND is %s",brand==RG_BRAND_OTHER?"Other":rg_lanNet_brand[(int)brand][0]);
		//Check for os_type
		for(i=RG_OS_OTHER+1;i<RG_OS_END;i++)
			for(j=0;rg_lanNet_os[i][j]!=NULL;j++)
				if(strstr(user_agent_str, rg_lanNet_os[i][j])){
					os_type=(rtk_rg_lanNet_os_t)i;
					i=RG_OS_END;
					break;
				}
		DEBUG("OS is %s",os_type==RG_OS_OTHER?"Other":rg_lanNet_os[(int)os_type][0]);

		//Special Device
		if(device_type==RG_LANNET_TYPE_PHONE && os_type==RG_OS_MACINTOSH)os_type=RG_OS_IOS;	//for iPad safari

		//Find the ARP of SIP
		arpIdx=-1;
		arpInfo.arpEntry.ipv4Addr=pPktHdr->ipv4Sip;
		if((pf.rtk_rg_arpEntry_find)(&arpInfo,&arpIdx)==RT_ERR_RG_OK){
			rg_db.arp[arpIdx].lanNetInfo.dev_type=device_type;
			rg_db.arp[arpIdx].lanNetInfo.brand=brand;
			rg_db.arp[arpIdx].lanNetInfo.os=os_type;
			if(rg_db.lut[rg_db.arp[arpIdx].rtk_arp.nhIdx].rtk_lut.entry.l2UcEntry.port==RTK_RG_PORT_CPU && 
				rg_db.lut[rg_db.arp[arpIdx].rtk_arp.nhIdx].rtk_lut.entry.l2UcEntry.ext_port!=(RTK_RG_PORT_CPU-RTK_RG_PORT_CPU))
				rg_db.lut[rg_db.arp[arpIdx].rtk_arp.nhIdx].conn_type=RG_CONN_WIFI;
			else
				rg_db.lut[rg_db.arp[arpIdx].rtk_arp.nhIdx].conn_type=RG_CONN_MAC_PORT;
			DEBUG("ARP[%d] dev_type:%s brand:%s os:%s connType:%s",arpIdx,
				rg_db.arp[arpIdx].lanNetInfo.dev_type==RG_LANNET_TYPE_PHONE?"Phone":rg_db.arp[arpIdx].lanNetInfo.dev_type==RG_LANNET_TYPE_COMPUTER?"Computer":"Other",
				rg_lanNet_brand[(int)rg_db.arp[arpIdx].lanNetInfo.brand][0],
				rg_lanNet_os[(int)rg_db.arp[arpIdx].lanNetInfo.os][0],
				rg_db.lut[rg_db.arp[arpIdx].rtk_arp.nhIdx].conn_type==RG_CONN_MAC_PORT?"MacPort":"Wifi");
		}
	}
	
	return RG_FWDENGINE_RET_CONTINUE;
}


rtk_rg_fwdEngineReturn_t _check_Http_mechanism(rtk_rg_pktHdr_t *pPktHdr, int *pNaptOutIdx)
{
	int ret;

	if(rg_db.lut[pPktHdr->smacL2Idx].redirect_http_req){
		ret=_forcePortal_check(pPktHdr, pNaptOutIdx, rg_db.lut[pPktHdr->smacL2Idx].redirect_http_req-1);
		if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;
	}else if(pPktHdr->httpFirstPacket==1){//Parse URL
		//20160120LUKE: parsing lan netInfo from user Agent field in Http request
		if(rg_db.systemGlobal.gatherLanNetInfo)_parse_Http_UserAgent(pPktHdr);
		if(!list_empty(&rg_db.redirectHttpURLListHead)){
			ret=_check_Http_URL(pPktHdr, pNaptOutIdx);
			if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;
		}
		if(rg_db.systemGlobal.urlFilter_valid_entry[0]!=-1){
			ret=_rtk_rg_urlFilter(pPktHdr->skb->data,pPktHdr->skb->len,pPktHdr);
			if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;
		}
	}

	pPktHdr->addNaptAfterNicTx=1;
	//int naptInIdx = rg_db.naptOut[*pNaptOutIdx].rtk_naptOut.hashIdx;	
	//TRACE("HTTP packet checked by urlfilter: add naptOut[%d] naptIn[%d]",*pNaptOutIdx,naptInIdx);
#ifdef CONFIG_ROME_NAPT_SHORTCUT
	if(pPktHdr->ipv4FragPacket==0)	//fragment  packets should always go normal path
		pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;
#endif
	return RG_FWDENGINE_RET_CONTINUE;
}

int _check_Http_keep_in_sw(rtk_rg_pktHdr_t *pPktHdr)
{
	//Http packet(l4_dport==80) should establish NAPT-flow after checked  by urlfilter and forcePortal and redirectHttp check. 
	if((rg_db.systemGlobal.urlFilter_valid_entry[0]!=-1)||
		(rg_db.lut[pPktHdr->smacL2Idx].redirect_http_req)||
		(!list_empty(&rg_db.redirectHttpURLListHead))||
		rg_db.systemGlobal.gatherLanNetInfo)
			return 1;

	return 0;
}

__IRAM_FWDENG
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_TCPConnectionTracking(rtk_rg_pktHdr_t *pPktHdr, int *pNaptOutIdx)
{
	int ret=0;
		
	if(*pNaptOutIdx<0) 
	{
		if(pPktHdr->fwdDecision==RG_FWD_DECISION_NAPT){
			// Never trap any forwarding NAPT connection packets to PS. (for syncing issue)			
			if(!((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==0))){
#ifdef CONFIG_ROME_NAPT_LRU
				TRACE("flow is not in NAPT table, Drop!");
				return RG_FWDENGINE_RET_DROP;
#else
				TRACE("flow is not in NAPT table, trap to PS");
				return RG_FWDENGINE_RET_TO_PS;	
#endif
			}
		}else{
			if((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==0)){
#ifdef CONFIG_RG_NAPT_INBOUND_TRACKING
				//There are no NAPT flows, lookup UPNP/virtual server/DMZ tables.
				// FAIL: entry not found.  -2: Inbound entry is found, but Outbound entry is not found.
				if(_rtk_rg_fwdEngine_connType_lookup(pPktHdr,&pPktHdr->ipv4Dip,&pPktHdr->dport)==RG_FWDENGINE_RET_TO_PS)return RG_FWDENGINE_RET_TO_PS;
#endif
			}else{
				//NAPT 4-ways not found, and not SYN packet, trap to CPU handle
				TRACE("flow is not in NAPT table, trap to PS");
				return RG_FWDENGINE_RET_TO_PS;
			}
		}
	}
	
	if((*pNaptOutIdx>=0) && (pPktHdr->tcpFlags.reset==1)){
		_rtk_rg_fwdEngine_receivedTCPReset(pPktHdr, pNaptOutIdx);
	}else if((*pNaptOutIdx>=0) && ((rg_db.naptOut[*pNaptOutIdx].state==TCP_CONNECTED)||(rg_db.naptOut[*pNaptOutIdx].state==FIRST_FIN))){
		if(pPktHdr->tcpFlags.fin==1) {	
			_rtk_rg_fwdEngine_receivedTCPFin(pPktHdr, pNaptOutIdx, FIRST_FIN);
		}
		else{		
			//check for HTTP packet
			if((pPktHdr->fwdDecision==RG_FWD_DECISION_NAPT)&&(pPktHdr->tagif&TCP_TAGIF)&&(pPktHdr->dport==80)){
				ret=_check_Http_mechanism(pPktHdr, pNaptOutIdx);
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;
			}
#ifdef CONFIG_ROME_NAPT_SHORTCUT
			if(rg_db.naptOut[*pNaptOutIdx].state==TCP_CONNECTED){
				//DEBUG("%s,%d  WE NEED UPDATE HERE!!",__FUNCTION__,__LINE__);
				if(pPktHdr->ipv4FragPacket==0)	//fragment  packets should always go normal path
					pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;	
			}		
#endif
		}
	}else{
		if((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==0)){		
			//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n%p",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset,rg_db.pNaptOutFreeListHead);

			//Lookup and write to free SW NAPT entry
			if(*pNaptOutIdx<0)
			{
				if(pPktHdr->fwdDecision==RG_FWD_DECISION_NAPT){
	            	ret = _rtk_rg_fwdEngine_outbound_fillNaptInfo(pNaptOutIdx,pPktHdr);
#ifdef CONFIG_ROME_NAPT_LRU
					if(ret!=RG_RET_SUCCESS) return RG_FWDENGINE_RET_DROP;
#else
					if(ret!=RG_RET_SUCCESS) return RG_FWDENGINE_RET_TO_PS;
#endif
				}else{
					ret = _rtk_rg_fwdEngine_inbound_fillNaptInfo(pNaptOutIdx,pPktHdr,pPktHdr->ipv4Dip,pPktHdr->dport);
					//DEBUG("set napt out[%d] software entry",*pNaptOutIdx);
					if(ret!=SUCCESS) return RG_FWDENGINE_RET_TO_PS;
				}

				if(rg_db.naptOut[*pNaptOutIdx].state>SYN_RECV)
				{
					WARNING("rg_db.naptOut[*pNaptOutIdx(%d)].state>SYN_RECV=%d",*pNaptOutIdx,rg_db.naptOut[*pNaptOutIdx].state);
				}				
			}
			rg_db.naptOut[*pNaptOutIdx].state=SYN_RECV;
		}
		if(((pPktHdr->tcpFlags.syn==0)&&(pPktHdr->tcpFlags.ack==1)&&(pPktHdr->tcpFlags.fin==0))||
			((pPktHdr->fwdDecision==RG_FWD_DECISION_NAPT)&&(rg_db.systemGlobal.tcp_hw_learning_at_syn==1)&&(rg_db.naptOut[*pNaptOutIdx].state==SYN_RECV)))
		{
			if((rg_db.naptOut[*pNaptOutIdx].state==SYN_ACK_RECV)||(rg_db.naptOut[*pNaptOutIdx].state==SYN_RECV))
			{					
				if((pPktHdr->fwdDecision==RG_FWD_DECISION_NAPT)&&(pPktHdr->dport==80)&&_check_Http_keep_in_sw(pPktHdr)){
					DEBUG("keep in sw for check URL later!!");
					//naptInIdx = rg_db.naptOut[*pNaptOutIdx].rtk_naptOut.hashIdx;
					rg_db.naptOut[*pNaptOutIdx].state=TCP_CONNECTED;
					//TRACE("Http Packet(dport==80), delay adding TCP naptOut[%d],naptIn[%d] until urlFilter or forcePortal Check.",*pNaptOutIdx,naptInIdx);
					//http80 always go normal path
				}else{
					//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
					//assert(rg_db.naptOut[*pNaptOutIdx].state==SYN_ACK_RECV);
					
					pPktHdr->addNaptAfterNicTx=1;
#ifdef CONFIG_ROME_NAPT_SHORTCUT
					//DEBUG("%s,%d  WE NEED UPDATE HERE!!",__FUNCTION__,__LINE__);
					if(pPktHdr->ipv4FragPacket==0)	//fragment  packets should always go normal path
						pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;					
#endif
				}

			}
			else if(rg_db.naptOut[*pNaptOutIdx].state==FIN_SEND_AND_RECV)
			{
				rg_db.naptOut[*pNaptOutIdx].state=LAST_ACK;
				if(rg_db.systemGlobal.tcpDoNotDelWhenRstFin==0)
				{
					if(rg_db.systemGlobal.tcp_short_timeout<=1) pPktHdr->delNaptConnection=1;
				}
			}
		}
		else if((pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==1))
		{
			//DEBUG("SYN:%d ACK:%d FIN:%d RST:%d\n",pPktHdr->tcpFlags.syn,pPktHdr->tcpFlags.ack,pPktHdr->tcpFlags.fin,pPktHdr->tcpFlags.reset);
			if(rg_db.naptOut[*pNaptOutIdx].state==INVALID) 
			{
				//DEBUG("loss syn naptOutIdx=%d",*pNaptOutIdx); 				
				return RG_FWDENGINE_RET_TO_PS;
			}
			if(rg_db.naptOut[*pNaptOutIdx].state<=SYN_ACK_RECV)
			{
				rg_db.naptOut[*pNaptOutIdx].state=SYN_ACK_RECV;

				// the 3 ways handshake maybe forward by HW, so must add hw entry at this time.
				//20150908LUKE: handle for virtual server with full cone
				if(pPktHdr->fwdDecision==RG_FWD_DECISION_NAPT)pPktHdr->addNaptAfterNicTx=1;
				
#ifdef CONFIG_ROME_NAPT_SHORTCUT
				//DEBUG("%s,%d  WE NEED UPDATE HERE!!",__FUNCTION__,__LINE__);
				if(pPktHdr->ipv4FragPacket==0)	//fragment  packets should always go normal path
					pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;
#endif
			}
		}
	}
	return RG_FWDENGINE_RET_NAPT_OK;
}

void _rtk_rg_force_tcp_add_connection(rtk_rg_pktHdr_t *pPktHdr, int *pNaptOutIdx)
{	
	if(_rtk_rg_fwdEngine_outbound_fillNaptInfo(pNaptOutIdx,pPktHdr)==RG_RET_SUCCESS){
		rg_db.naptOut[*pNaptOutIdx].state=TCP_CONNECTED;
		pPktHdr->addNaptAfterNicTx=1;
		TRACE("TCP disable stateful tracking: add naptOut[%d]",*pNaptOutIdx);
#ifdef CONFIG_ROME_NAPT_SHORTCUT
		if(pPktHdr->ipv4FragPacket==0)	//fragment  packets should always go normal path
			pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;
#endif
	}
}

__IRAM_FWDENG
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_TCPOutboundConnectionTracking(rtk_rg_pktHdr_t *pPktHdr, int *pNaptOutIdx)
{
	//20151207LUKE: disable TCP stateful tracking
	if(rg_db.systemGlobal.tcpDisableStatefulTracking && *pNaptOutIdx<0)
		_rtk_rg_force_tcp_add_connection(pPktHdr, pNaptOutIdx);
	return _rtk_rg_fwdEngine_TCPConnectionTracking(pPktHdr, pNaptOutIdx);
}


__IRAM_FWDENG
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_TCPInboundConnectionTracking(rtk_rg_pktHdr_t *pPktHdr, int *pNaptOutIdx)
{
	return _rtk_rg_fwdEngine_TCPConnectionTracking(pPktHdr, pNaptOutIdx);
}

rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_UDPOutboundConnectionTracking(rtk_rg_pktHdr_t *pPktHdr, int *retIdx)
{
	int naptOutIdx=0,naptInIdx=0;
	rtk_rg_successFailReturn_t sf_ret=RG_RET_SUCCESS;

	//naptOutIdx=_rtk_rg_naptTcpUdpOutHashIndexLookup(0,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->ipv4Dip,pPktHdr->dport);
	naptOutIdx=_rtk_rg_naptTcpUdpOutHashIndexLookupByPktHdr(0,pPktHdr);
	if(naptOutIdx==RG_RET_LOOKUPIDX_NOT_FOUND)
	{
		//Lookup and set NAPT SW tables
		sf_ret = _rtk_rg_fwdEngine_outbound_fillNaptInfo(&naptOutIdx,pPktHdr);
#ifdef CONFIG_ROME_NAPT_LRU
		assert_ok(sf_ret);
		if(sf_ret!=RG_RET_SUCCESS) 
		{
			TRACE("Drop!");
			return RG_FWDENGINE_RET_DROP; //all fwdEngine sw-entry is full.(NEVER HAPPEN)
		}
#else
		if(sf_ret!=SUCCESS) return RG_FWDENGINE_RET_TO_PS;
#endif
		rg_db.naptOut[naptOutIdx].state=UDP_FIRST;
		//DEBUG("UDP state=%d extport=%d\n",rg_db.naptOut[naptOutIdx].state,rg_db.naptOut[naptOutIdx].extPort);
	}
	else if(rg_db.naptOut[naptOutIdx].state==UDP_FIRST)
	{
		rg_db.naptOut[naptOutIdx].state=UDP_SECOND;
		//DEBUG("UDP state=%d extport=%d\n",rg_db.naptOut[naptOutIdx].state,rg_db.naptOut[naptOutIdx].extPort);
	}
	else if(rg_db.naptOut[naptOutIdx].state==UDP_SECOND)
	{
		//rg_db.naptOut[naptOutIdx].state=UDP_CONNECTED;
		//DEBUG("UDP state=%d extport=%d\n",rg_db.naptOut[naptOutIdx].state,rg_db.naptOut[naptOutIdx].extPort);
		//Add NAPT connection to ASIC
		naptInIdx = rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;
		pPktHdr->addNaptAfterNicTx = 1;
		//ret = _rtk_rg_naptConnection_add(naptOutIdx,&rg_db.naptOut[naptOutIdx].rtk_naptOut,&rg_db.naptIn[naptInIdx].rtk_naptIn);
		//assert_ok(ret);

		//TRACE("add UDP naptOut[%d],naptIn[%d] extPort=%d",naptOutIdx,naptInIdx,rg_db.naptOut[naptOutIdx].extPort);
#ifdef CONFIG_ROME_NAPT_SHORTCUT
		//DEBUG("%s,%d  WE NEED UPDATE HERE!!",__FUNCTION__,__LINE__);
		if(pPktHdr->ipv4FragPacket==0)	//fragment  packets should always go normal path
			pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;
#endif
	}
	else if(rg_db.naptOut[naptOutIdx].state==UDP_CONNECTED)
	{
		//since the connection is created, we can updat shortcut to accelerate the data path!!
		if(pPktHdr->shortcutStatus==RG_SC_NORMAL_PATH)
		{
			//DEBUG("in %s, the state is %d, we should update shortcut!!",__FUNCTION__,rg_db.naptOut[naptOutIdx].state);
#ifdef CONFIG_ROME_NAPT_SHORTCUT
			//DEBUG("%s,%d  WE NEED UPDATE HERE!!",__FUNCTION__,__LINE__);
			if(pPktHdr->ipv4FragPacket==0)	//fragment	packets should always go normal path
				pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;
#endif			
		}
	}

	*retIdx = naptOutIdx;

	return RG_FWDENGINE_RET_NAPT_OK;
}

int _rtk_rg_fwdEngine_ICMPOutboundControlFlowTracking(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_table_icmp_flow_t **icmpCtrlList)
{
	rtk_rg_table_icmp_flow_t *pEntry;

	//create new control flow in link-list
	pEntry=&rg_db.pICMPCtrlFlowHead->icmpFlow;
	rg_db.pICMPCtrlFlowHead=rg_db.pICMPCtrlFlowHead->pNext;

	pEntry->valid=1;
	pEntry->internalIP=pPktHdr->ipv4Sip;
	pEntry->remoteIP=pPktHdr->ipv4Dip;
	pEntry->IPID=ntohs(*pPktHdr->pIpv4Identification);
	//pEntry->ICMPType=pPktHdr->ICMPType;
	//pEntry->ICMPCode=pPktHdr->ICMPCode;
	pEntry->ICMPID=pPktHdr->ICMPIdentifier;
	pEntry->ICMPSeqNum = pPktHdr->ICMPSeqNum;

	*icmpCtrlList=pEntry;
	// DEBUG("adding Outbound: sip=%08x, dip=%08x ipid=%x, type is %d, code is %d, identifier is %d ICMPSeqNum=%d",
	// 	pEntry->internalIP, pEntry->remoteIP, pEntry->IPID, pEntry->ICMPType, pEntry->ICMPCode, pEntry->ICMPID,pEntry->ICMPSeqNum);

	return RG_FWDENGINE_RET_NAPT_OK;
}

rtk_rg_table_icmp_flow_t * _rtk_rg_fwdEngine_ICMPInboundControlFlowTracking(rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_table_icmp_linkList_t *pEntry;
	
	//lookup all link-list
	for(pEntry=rg_db.pICMPCtrlFlowHead->pPrev;pEntry!=rg_db.pICMPCtrlFlowHead;pEntry=pEntry->pPrev)
	{
		// TRACE("valid=%d id1=%d id2=%d ip1=%x ip2=%x seq1=%d seq2=%d",
		// 	pEntry->icmpFlow.valid,pEntry->icmpFlow.ICMPID,pPktHdr->ICMPIdentifier,pEntry->icmpFlow.remoteIP,pPktHdr->ipv4Sip,pEntry->icmpFlow.ICMPSeqNum,pPktHdr->ICMPSeqNum);
		if(pEntry->icmpFlow.valid==1 && 
			pEntry->icmpFlow.ICMPID==pPktHdr->ICMPIdentifier &&
			pEntry->icmpFlow.ICMPSeqNum == pPktHdr->ICMPSeqNum &&
			pEntry->icmpFlow.remoteIP==pPktHdr->ipv4Sip )
		{
			//match
			return &pEntry->icmpFlow;
		}
	}

	//unmatch
	return NULL;
}

#if 0
rtk_rg_table_icmp_flow_t * _rtk_rg_fwdEngine_ICMPOutboundFragmentLookup(rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_table_icmp_linkList_t *pEntry;
	
	//lookup all link-list
	for(pEntry=rg_db.pICMPCtrlFlowHead->pPrev;pEntry!=rg_db.pICMPCtrlFlowHead;pEntry=pEntry->pPrev)
	{
		if(pEntry->icmpFlow.valid==1 && pEntry->icmpFlow.internalIP==pPktHdr->ipv4Sip && pEntry->icmpFlow.IPID==ntohs(*pPktHdr->pIpv4Identification))
		{
			//match
			return &pEntry->icmpFlow;
		}
	}

	//unmatch
	return NULL;
}

rtk_rg_table_icmp_flow_t * _rtk_rg_fwdEngine_ICMPInboundFragmentLookup(rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_table_icmp_linkList_t *pEntry;
	
	//lookup all link-list
	for(pEntry=rg_db.pICMPCtrlFlowHead->pPrev;pEntry!=rg_db.pICMPCtrlFlowHead;pEntry=pEntry->pPrev)
	{
		if(pEntry->icmpFlow.valid==1 && pEntry->icmpFlow.remoteIP==pPktHdr->ipv4Sip && pEntry->icmpFlow.inboundIPID==ntohs(*pPktHdr->pIpv4Identification))
		{
			//match
			return &pEntry->icmpFlow;
		}
	}

	//unmatch
	return NULL;
}
#endif

#ifdef CONFIG_RG_NAPT_INBOUND_TRACKING
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_UDPInboundConnectionTracking(rtk_rg_pktHdr_t *pPktHdr, int *retIdx)
{
	rtk_rg_lookupIdxReturn_t naptOutIdx=0,naptInIdx=0;
	rtk_rg_fwdEngineReturn_t ret=0;
	rtk_rg_successFailReturn_t sf_ret=RG_RET_SUCCESS;
	ipaddr_t transIP = pPktHdr->ipv4Dip;
	uint16 transPort = pPktHdr->dport;

	naptOutIdx=_rtk_rg_naptTcpUdpInHashIndexLookup(0,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->ipv4Dip,pPktHdr->dport);
	if(naptOutIdx<0) // FAIL: entry not found.  -2: Inbound entry is found, but Outbound entry is not found.
	{
		ret = _rtk_rg_fwdEngine_connType_lookup(pPktHdr,&transIP,&transPort);
#ifdef CONFIG_ROME_NAPT_LRU		
		if(ret==RG_FWDENGINE_RET_TO_PS) 
		{
			TRACE("Inbound flow not found in NAPTR table or hit GatewayServicePort. To PS!");
			return RG_FWDENGINE_RET_TO_PS;
		}
#else		
		if(ret==RG_FWDENGINE_RET_TO_PS) return RG_FWDENGINE_RET_TO_PS;
#endif		

		//Lookup and set NAPT SW tables
		sf_ret = _rtk_rg_fwdEngine_inbound_fillNaptInfo(&naptOutIdx,pPktHdr,transIP,transPort);
		if(sf_ret!=RG_RET_SUCCESS) return RG_FWDENGINE_RET_TO_PS;
		rg_db.naptOut[naptOutIdx].state=UDP_FIRST;
		//DEBUG("UDP state=%d extport=%d\n",rg_db.naptOut[naptOutIdx].state,rg_db.naptOut[naptOutIdx].extPort);
	}
	else if(rg_db.naptOut[naptOutIdx].state==UDP_FIRST)
	{
		rg_db.naptOut[naptOutIdx].state=UDP_SECOND;
		//DEBUG("UDP state=%d extport=%d\n",rg_db.naptOut[naptOutIdx].state,rg_db.naptOut[naptOutIdx].extPort);
	}
	else if(rg_db.naptOut[naptOutIdx].state==UDP_SECOND)
	{
		//rg_db.naptOut[naptOutIdx].state=UDP_CONNECTED;
		//DEBUG("UDP state=%d extport=%d\n",rg_db.naptOut[naptOutIdx].state,rg_db.naptOut[naptOutIdx].extPort);
		//Add NAPT connection to ASIC
		naptInIdx = rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;
		pPktHdr->addNaptAfterNicTx = 1;
		//ret = _rtk_rg_naptConnection_add(naptOutIdx,&rg_db.naptOut[naptOutIdx].rtk_naptOut,&rg_db.naptIn[naptInIdx].rtk_naptIn);
		//TRACE("add UDP naptOut[%d],naptIn[%d] extPort=%d",naptOutIdx,naptInIdx,rg_db.naptOut[naptOutIdx].extPort);
		//pPktHdr->ipv4Dip = rg_db.naptIn[naptInIdx].rtk_naptIn.intIp;
#ifdef CONFIG_ROME_NAPT_SHORTCUT
		//DEBUG("%s,%d  WE NEED UPDATE HERE!!",__FUNCTION__,__LINE__);
		if(pPktHdr->ipv4FragPacket==0)	//fragment	packets should always go normal path
			pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;
#endif
		//assert_ok(ret);
	}
	else if(rg_db.naptOut[naptOutIdx].state==UDP_CONNECTED)
	{
		//since the connection is created, we can updat shortcut to accelerate the data path!!
		if(pPktHdr->shortcutStatus==RG_SC_NORMAL_PATH)
		{
			//DEBUG("in %s, the state is %d, we should update shortcut!!",__FUNCTION__,rg_db.naptOut[naptOutIdx].state);
#ifdef CONFIG_ROME_NAPT_SHORTCUT
			//DEBUG("%s,%d  WE NEED UPDATE HERE!!",__FUNCTION__,__LINE__);
			if(pPktHdr->ipv4FragPacket==0)	//fragment  packets should always go normal path
				pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;
#endif			
		}
	}

	*retIdx = naptOutIdx;

	return RG_FWDENGINE_RET_NAPT_OK;
}
#endif

int _rtk_rg_fwdengine_handleArpMiss(rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_arp_request_t arpReq;
	ipaddr_t ipAddr;
	int ret;
	rtk_rg_sipDipClassification_t sipdipClass;

	pPktHdr->dipL3Idx=_rtk_rg_l3lookup(*pPktHdr->pIpv4Dip);

	sipdipClass=SIP_DIP_CLASS_ROUTING;
	ret=_rtk_rg_routingDecisionTablesLookup(pPktHdr,&sipdipClass);
	if(pPktHdr->netifIdx==FAIL/*ret!=RG_FWDENGINE_RET_CONTINUE*/) 
		return ret;
	
	if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan==1)
	{
		ipAddr=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].p_wanStaticInfo->ip_addr;
	}
	else
	{
		ipAddr=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.lan_intf.ip_addr;
	}
	
	if(*pPktHdr->pIpv4Dip!=ipAddr) //skip gateway ip arp request
	{
		arpReq.finished=0;
		arpReq.gwMacReqCallBack=NULL;
		arpReq.reqIp=*pPktHdr->pIpv4Dip;
		DEBUG("ARP [0x%x] Miss, Send ARP Request!",*pPktHdr->pIpv4Dip);
		_rtk_rg_arpGeneration(pPktHdr->netifIdx,ipAddr,&arpReq);
	}

	return RG_FWDENGINE_RET_L2FORWARDED;
}

int _rtk_rg_fwdengine_handleArpMissInRoutingLookUp(rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_arp_request_t arpReq;
	ipaddr_t ipAddr;

	ipAddr=rg_db.l3[pPktHdr->dipL3Idx].gateway_ip;
	if(ipAddr==0){
		if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan==1)
			ipAddr=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].p_wanStaticInfo->ip_addr;
		else
			ipAddr=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.lan_intf.ip_addr;
	}
	
	if(*pPktHdr->pIpv4Dip!=ipAddr) //skip gateway ip arp request
	{	
		arpReq.finished=0;
		arpReq.gwMacReqCallBack=NULL;
		arpReq.reqIp=*pPktHdr->pIpv4Dip;
		TRACE("ARP [0x%x] Miss, Send ARP Request!from gwIP %x..",*pPktHdr->pIpv4Dip,ipAddr);
		_rtk_rg_arpGeneration(pPktHdr->netifIdx,ipAddr,&arpReq);
	}else{
		TRACE("DIP equal to gateway IP(%x), TRAP!!",ipAddr);
		return RG_FWDENGINE_RET_TO_PS;
	}

	return RG_FWDENGINE_RET_L2FORWARDED;
}


int _rtk_rg_fwdengine_handleNeighborMiss(rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_neighbor_discovery_t neighborDisc;
	rtk_ipv6_addr_t ipAddr;
	int ret,netIfIdx;
	//DEBUG("handle neighbor miss!");
	//_rtk_rg_v6RoutingDecisionTablesLookup(pPktHdr,0 /*It's not LAN to WAN*/);
	ret=_rtk_rg_v6L3lookup(pPktHdr->pIpv6Dip);
	//DEBUG("the v6L3lookup ret is %d",ret);
	if(ret<0)return RG_FWDENGINE_RET_L2FORWARDED;

	if(rg_db.v6route[ret].rtk_v6route.type == L34_IPV6_ROUTE_TYPE_LOCAL)
	{
		netIfIdx=rg_db.v6route[ret].rtk_v6route.nhOrIfidIdx;
	}
	else if(rg_db.v6route[ret].rtk_v6route.type == L34_IPV6_ROUTE_TYPE_GLOBAL)
	{
		netIfIdx=rg_db.nexthop[rg_db.v6route[ret].rtk_v6route.nhOrIfidIdx].rtk_nexthop.ifIdx;
	}
	else
		return RG_FWDENGINE_RET_L2FORWARDED;
		
	if(rg_db.systemGlobal.interfaceInfo[netIfIdx].storedInfo.is_wan==1)
	{
		memcpy(ipAddr.ipv6_addr,rg_db.systemGlobal.interfaceInfo[netIfIdx].p_wanStaticInfo->ipv6_addr.ipv6_addr,IPV6_ADDR_LEN);
	}
	else
	{
		memcpy(ipAddr.ipv6_addr,rg_db.systemGlobal.interfaceInfo[netIfIdx].storedInfo.lan_intf.ipv6_addr.ipv6_addr,IPV6_ADDR_LEN);
	}
	/*DEBUG("the ipaddr is %08x:%08x:%08x:%08x",*(unsigned int *)ipAddr.ipv6_addr,
		*(unsigned int *)(ipAddr.ipv6_addr+4),
		*(unsigned int *)(ipAddr.ipv6_addr+8),
		*(unsigned int *)(ipAddr.ipv6_addr+12));*/
	if(memcmp(pPktHdr->pIpv6Dip,ipAddr.ipv6_addr,IPV6_ADDR_LEN))	//skip gateway ip neighbor discovery
	{
		neighborDisc.finished=0;
		neighborDisc.ipv6GwMacReqCallBack=NULL;
		memcpy(neighborDisc.reqIp.ipv6_addr,pPktHdr->pIpv6Dip,IPV6_ADDR_LEN);
		DEBUG("Neighbor [0x%08x:%08x:%08x:%08x] Miss, Send Neighbor Discovery!",
			*(unsigned int *)pPktHdr->pIpv6Dip,
			*(unsigned int *)(pPktHdr->pIpv6Dip+4),
			*(unsigned int *)(pPktHdr->pIpv6Dip+8),
			*(unsigned int *)(pPktHdr->pIpv6Dip+12));
		_rtk_rg_NDGeneration(netIfIdx,ipAddr,&neighborDisc);
		//return RG_FWDENGINE_RET_TO_PS;
	}

	return RG_FWDENGINE_RET_L2FORWARDED;
}

uint16 _rtk_rg_CompareByte(uint8 char_a, uint8 char_b, uint8 mask)
{
	uint8 tmp_a,tmp_b,res;
	tmp_a = char_a&mask;
	tmp_b = char_b&mask;
	res = tmp_a^tmp_b;
	if (res == 0)	//tmp_a == tmp_b
		return 1;
	else
		return 0;
}

rtk_rg_successFailReturn_t _rtk_rg_getInternalPriByPortbased(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, uint8 *internalPri){

	if(pPktHdr->pRxDesc->opts3.bit.src_port_num < RTK_RG_MAC_PORT_MAX){
		*internalPri = rg_db.systemGlobal.qosInternalDecision.qosPortBasedPriority[pPktHdr->pRxDesc->opts3.bit.src_port_num];
		TRACE("Internal Pri:%d choosed by Portbased[%d]",*internalPri,pPktHdr->pRxDesc->opts3.bit.src_port_num);
		return RG_RET_SUCCESS;
	}
	return RG_RET_FAIL;
}

int _rtk_rg_getInternalPriByDot1q(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, uint8 *internalPri){
	
	if(pPktHdr->tagif & CVLAN_TAGIF){
		*internalPri = rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemapToInternalPriTbl[pPktHdr->ctagPri];
		TRACE("Internal Pri:%d choosed by dot1q[%d]",*internalPri,pPktHdr->ctagPri);
		return RT_ERR_RG_OK;
	}

	return RT_ERR_RG_FAILED;
}

int _rtk_rg_getInternalPriByDscp(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, uint8 *internalPri){
	uint8 dscp;
	if(pPktHdr->pTos!=NULL){
		dscp = ((*(pPktHdr->pTos))>>2);
	}else{
		return RT_ERR_RG_FAILED; //no dscp field
	}
	
	if((pPktHdr->tagif & IPV4_TAGIF) || (pPktHdr->tagif & IPV6_TAGIF)){
		if(pPktHdr->pTos!=NULL){
			*internalPri = rg_db.systemGlobal.qosInternalDecision.qosDscpRemapToInternalPri[dscp];
			TRACE("Internal Pri:%d choosed by dscp[%d]",*internalPri,dscp);
			return RT_ERR_RG_OK;
		}
	}
	return RT_ERR_RG_FAILED;
}

int _rtk_rg_getInternalPriByVlan(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, uint8 *internalPri){

	int internalVid;
	internalVid = pPktHdr->internalVlanID;
	if(rg_db.vlan[internalVid].priorityEn==ENABLED){
		*internalPri = rg_db.vlan[internalVid].priority;
		TRACE("Internal Pri:%d choosed by Vlan[%d]",*internalPri,internalVid);
		return RT_ERR_RG_OK;
	}
	return RT_ERR_RG_FAILED;
}

int _rtk_rg_getInternalPriByLayer4(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, uint8 *internalPri){
	if(pPktHdr->l4Direction==RG_NAPT_OUTBOUND_FLOW){
		if(rg_db.naptOut[pPktHdr->naptOutboundIndx].rtk_naptOut.priValid){
			*internalPri = rg_db.naptOut[pPktHdr->naptOutboundIndx].rtk_naptOut.priValue;
			TRACE("Internal Pri:%d choosed by NAPT[%d]",*internalPri,pPktHdr->naptOutboundIndx);
			return RT_ERR_RG_OK;
		}
	}else if(pPktHdr->l4Direction==RG_NAPTR_INBOUND_FLOW){
		if(rg_db.naptIn[pPktHdr->naptrInboundIndx].rtk_naptIn.priValid){
			*internalPri = rg_db.naptIn[pPktHdr->naptrInboundIndx].rtk_naptIn.priId;
			TRACE("Internal Pri:%d choosed by NAPTR[%d]",*internalPri,pPktHdr->naptrInboundIndx);
			return RT_ERR_RG_OK;
		}
	}
	return RT_ERR_RG_FAILED;
}

int _rtk_rg_getInternalPriByAcl(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, uint8 *internalPri){

	if(pPktHdr->aclPriority!=-1/*acl internal action will change this value*/){
		*internalPri  = pPktHdr->aclPriority;
		TRACE("Internal Pri:%d choosed by ACL",*internalPri);
		return RT_ERR_RG_OK;
	}
	return RT_ERR_RG_FAILED;
}

int _rtk_rg_getInternalPriByLut(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb, uint8 *internalPri){

	int i;

	//search the lut 
	for(i=0;i<MAX_LUT_SW_TABLE_SIZE;i++){
		if(rg_db.lut[i].valid==1 && rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC){//check vlaid and is unicast

			if(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_IVL){//IVL compatre vid
				if(pPktHdr->internalVlanID != rg_db.lut[i].rtk_lut.entry.l2UcEntry.vid) continue;
			}else{//SVL compare fid
				//FID defined at LAN_FID/WAN_FID, now the setting is the same!
			}
			
			//compare dmac
			if(rg_db.lut[i].rtk_lut.entry.l2UcEntry.mac.octet[0] == pPktHdr->pDmac[0] &&
				rg_db.lut[i].rtk_lut.entry.l2UcEntry.mac.octet[1] == pPktHdr->pDmac[1] &&
				rg_db.lut[i].rtk_lut.entry.l2UcEntry.mac.octet[2] == pPktHdr->pDmac[2] &&
				rg_db.lut[i].rtk_lut.entry.l2UcEntry.mac.octet[3] == pPktHdr->pDmac[3] &&
				rg_db.lut[i].rtk_lut.entry.l2UcEntry.mac.octet[4] == pPktHdr->pDmac[4] &&
				rg_db.lut[i].rtk_lut.entry.l2UcEntry.mac.octet[5] == pPktHdr->pDmac[5]){

				if(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_FWD_PRI){//lutFwd priority is enabled
					*internalPri  = rg_db.lut[i].rtk_lut.entry.l2UcEntry.priority;
					TRACE("Internal Pri:%d choosed by LUT(l2Idx=%d)",*internalPri,i);
					return RT_ERR_RG_OK;
				}else{
					break;
				}
			}else{
				continue;
			}
		}
	}

	//no entry found, return failed
	return RT_ERR_RG_FAILED;
}

rtk_rg_successFailReturn_t _rtk_rg_internalPrioritySelect(rtk_rg_pktHdr_t *pPktHdr,struct sk_buff *skb){

	int i,j;
	rtk_rg_successFailReturn_t ret;
	uint8 highestWeight=WEIGHT_OF_END;
	uint8 internalPri=0;
	
	//DEBUG("================================================");

	/*if no need to remarking, then we don't need to calculate the internal Prioirty.*/
	/* ACL filter egress_ctag_pri need this information
	if(rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[RTK_RG_MAC_PORT0]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[RTK_RG_MAC_PORT1]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[RTK_RG_MAC_PORT2]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[RTK_RG_MAC_PORT3]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[RTK_RG_MAC_PORT_PON]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[RTK_RG_MAC_PORT_RGMII]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[RTK_RG_MAC_PORT_CPU]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkEgressPortEnableAndSrcSelect[RTK_RG_MAC_PORT0]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkEgressPortEnableAndSrcSelect[RTK_RG_MAC_PORT1]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkEgressPortEnableAndSrcSelect[RTK_RG_MAC_PORT2]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkEgressPortEnableAndSrcSelect[RTK_RG_MAC_PORT3]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkEgressPortEnableAndSrcSelect[RTK_RG_MAC_PORT_PON]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkEgressPortEnableAndSrcSelect[RTK_RG_MAC_PORT_RGMII]==DISABLED&&
		rg_db.systemGlobal.qosInternalDecision.qosDscpRemarkEgressPortEnableAndSrcSelect[RTK_RG_MAC_PORT_CPU]==DISABLED){
		return RT_ERR_RG_OK;
	}
	*/

	i=15; /*search from the highest weight(15)*/
findHighestValidWeight:
	highestWeight = WEIGHT_OF_END;
	while(i>0){
		//rtlglue_printf("search i=%d\n",i);
		for(j=0;j<WEIGHT_OF_END;j++){
			//TRACE("rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[%d]=%d",j,rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[j]);
			if(rg_db.systemGlobal.qosInternalDecision.internalPriSelectWeight[j]==i){
				highestWeight = j;
				break;
			}
		}
		i--; /*search next hight*/
		if(highestWeight!=WEIGHT_OF_END){
			//DEBUG("Internal Pri weight by(%d): %d",j,i);
			break;
		}	

		/*all kind of weight is 0 is imposible*/
		if(i==0)
			return RT_ERR_RG_FAILED;
	}
	
	//TRACE("highestWeight=%d",highestWeight);	
	switch(highestWeight){
		case WEIGHT_OF_PORTBASED: 
			ret = _rtk_rg_getInternalPriByPortbased(pPktHdr,skb,&internalPri);
			if(ret!=RG_RET_SUCCESS) goto findHighestValidWeight;
			break;
		case WEIGHT_OF_DOT1Q:
			ret = _rtk_rg_getInternalPriByDot1q(pPktHdr,skb,&internalPri);
			if(ret!=RG_RET_SUCCESS) goto findHighestValidWeight;
			break;
		case WEIGHT_OF_DSCP:
			ret = _rtk_rg_getInternalPriByDscp(pPktHdr,skb,&internalPri);
			if(ret!=RG_RET_SUCCESS) goto findHighestValidWeight;
			break;
		case WEIGHT_OF_VLANBASED:
			ret = _rtk_rg_getInternalPriByVlan(pPktHdr,skb,&internalPri);
			if(ret!=RG_RET_SUCCESS) goto findHighestValidWeight;
			break;
		case WEIGHT_OF_L4BASED:
			ret = _rtk_rg_getInternalPriByLayer4(pPktHdr,skb,&internalPri);
			if(ret!=RG_RET_SUCCESS) goto findHighestValidWeight;
			break;			
		case WEIGHT_OF_ACL:
			ret = _rtk_rg_getInternalPriByAcl(pPktHdr,skb,&internalPri);
			if(ret!=RG_RET_SUCCESS) goto findHighestValidWeight;
			break;			
		case WEIGHT_OF_LUTFWD:
			//ret = _rtk_rg_getInternalPriByLut(pPktHdr,skb,&internalPri);
			//if(ret!=RT_ERR_RG_OK) goto findHighestValidWeight;
			//break;	
		case WEIGHT_OF_SABASED:
		case WEIGHT_OF_SVLANBASED:

		default:
			goto findHighestValidWeight;
			/*FIXME: else weight should supported!*/
			break;
	}

#ifdef CONFIG_RG_QOS_DEF_SET
	if (internalPri < 3) internalPri = 3;
#endif

	pPktHdr->internalPriority = internalPri;
	//TRACE("Internal Priority:%d",pPktHdr->internalPriority);

	
	
	return RT_ERR_RG_OK;
}

/*
* _rtk_rg_get_aclActionAccelerationType is used for check if fix pktBuff is needed
* @dataPathToWifi: 0 means to HW or wifi slave datapath,  1 means to wifi master datapath
* return:
*   RG_FWDENGINE_ACL_ACC_TYPE_TX_PKTBUFF: pkt related with Stag, must have to modified pktbuff and slow efficiency
*   RG_FWDENGINE_ACL_ACC_TYPE_TX_DESC: pkt vlanTag can handeld by tx_desc, more efficiency.
*/
void _rtk_rg_get_stagCtagAccelerationType(rtk_rg_pktHdr_t *pPktHdr,uint32 dataPathToWifi){
	uint32 txPortMask = 0;
	//master wifi do not have tx_desc, and must send untagged. always handel by skbuff
	if(dataPathToWifi)
	{
		if(pPktHdr->gponDsBcModuleRuleHit)
		{
			TRACE("tagging by gponDsBcModuleRule for master wifi!");
			//continue by internal tagging decision.	
		}
		else
		{
			if(!rg_db.systemGlobal.initParam.macBasedTagDecision
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
			//20150507LUKE: to WWAN, we should not always untag
			&& pPktHdr->wlan_dev_idx!=RG_WWAN_WLAN0_VXD && pPktHdr->wlan_dev_idx!=RG_WWAN_WLAN1_VXD
#endif
			)
			{
				//20150310LUKE: if macBasedTagDecision is off, we should always remove vlan-tag here!
				TRACE("force unCTag for master wifi if macBasedTagDecision disabled!");
				pPktHdr->egressVlanTagif = 0; //force untag c
				pPktHdr->egressServiceVlanTagif = 0; // //force untag s
			}
			pPktHdr->egressTagAccType=RG_FWDENGINE_ACL_ACC_TYPE_TX_PKTBUFF;
			goto BEFORE_RET;
		}
	}
#ifdef CONFIG_DUALBAND_CONCURRENT	
	//slave wifi, force using skbuff, too.
	if(pPktHdr->egressVlanTagif==1 &&
		pPktHdr->egressVlanID==CONFIG_DEFAULT_TO_SLAVE_GMAC_VID && 
		pPktHdr->egressPriority==CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI){
		//TRACE("datapath for slave wifi!");
		pPktHdr->egressTagAccType=RG_FWDENGINE_ACL_ACC_TYPE_TX_PKTBUFF;
		goto BEFORE_RET;
	}
#endif

	/*only following 4 cases can using TX_DESC*/
	//(in)untag => (out)untag
	//(in)untag => (out)ctag
	//(in)ctag => (out)untag
	//(in)ctag => (out)ctag
	if(((pPktHdr->tagif & CVLAN_TAGIF)==0x0 &&
		(pPktHdr->tagif & SVLAN_TAGIF)==0x0 &&
		(pPktHdr->egressVlanTagif)==0x0 &&
		(pPktHdr->egressServiceVlanTagif)==0x0)||
		((pPktHdr->tagif & CVLAN_TAGIF) &&
		(pPktHdr->tagif & SVLAN_TAGIF)==0x0 &&
		(pPktHdr->egressVlanTagif) &&
		(pPktHdr->egressServiceVlanTagif)==0x0)){
		pPktHdr->egressTagAccType=RG_FWDENGINE_ACL_ACC_TYPE_TX_DESC;
	}else if((pPktHdr->tagif & CVLAN_TAGIF)==0x0 &&
		(pPktHdr->tagif & SVLAN_TAGIF)==0x0 &&
		(pPktHdr->egressVlanTagif) &&
		(pPktHdr->egressServiceVlanTagif)==0x0){
		pPktHdr->egressTagAccType=RG_FWDENGINE_ACL_ACC_TYPE_TX_DESC;
		//add 4 byte for CVLAN in mib counter
#if defined(CONFIG_RTL9602C_SERIES)
		pPktHdr->mibTagDelta=4;
#endif
	}else if((pPktHdr->tagif & CVLAN_TAGIF) &&
		(pPktHdr->tagif & SVLAN_TAGIF)==0x0 &&
		(pPktHdr->egressVlanTagif)==0x0 &&
		(pPktHdr->egressServiceVlanTagif)==0x0){
		//minus 4 byte for CVLAN in mib counter
		pPktHdr->egressTagAccType=RG_FWDENGINE_ACL_ACC_TYPE_TX_DESC;
#if defined(CONFIG_RTL9602C_SERIES)
		pPktHdr->mibTagDelta=-4;
#endif
	//rest should handeled by pktbuff
	}else
		pPktHdr->egressTagAccType=RG_FWDENGINE_ACL_ACC_TYPE_TX_PKTBUFF;

#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
	/* ysleu: For xDSL series chip, ASIC whould not keep/offload specific function but all together(checksum,pppoe,vlan).
	              Therefore, we prepare all while packet is from protocol stack.
	              Here, we tag/untag vlan by set pPktHdr->egressTagAccType=RG_FWDENGINE_ACL_ACC_TYPE_TX_PKTBUFF */
	if(pPktHdr->ingressLocation==RG_IGR_PROTOCOL_STACK)
		pPktHdr->egressTagAccType=RG_FWDENGINE_ACL_ACC_TYPE_TX_PKTBUFF;
#endif

BEFORE_RET:
	if(pPktHdr->egressUniPortmask!=0)
		txPortMask = pPktHdr->egressUniPortmask;
	else
		txPortMask = rg_kernel.txDesc.opts3.bit.tx_portmask;
	
	//TRACE("Portmask [0x%x] Qos accType: %s", txPortMask, pPktHdr->egressTagAccType?"ACC BY TX_DESC":"MUST MODIFY PKTBUFF");

	TRACE("Portmask [0x%x] Egress(CVID:%d CTAG_IF:%d STAG_IF:%d) [NIC TX: CVLAN HW offload(%s)]",
		txPortMask,
		pPktHdr->egressVlanID,
		pPktHdr->egressVlanTagif,
		pPktHdr->egressServiceVlanTagif,
		pPktHdr->egressTagAccType?"on":"off"
		);

	//update accType
	if(pPktHdr->shortcutStatus==RG_SC_NEED_UPDATE_BEFORE_SEND){
#ifdef CONFIG_ROME_NAPT_SHORTCUT
#if defined(CONFIG_RTL9602C_SERIES)
		pPktHdr->pCurrentShortcutEntry->mibTagDelta=pPktHdr->mibTagDelta;
#endif
		pPktHdr->pCurrentShortcutEntry->tagAccType=pPktHdr->egressTagAccType;
		pPktHdr->pCurrentShortcutEntry->notFinishUpdated=0;
#endif				
	}
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	else if(pPktHdr->shortcutStatus==RG_SC_V6_NEED_UPDATE_BEFORE_SEND){
#if defined(CONFIG_RTL9602C_SERIES)
		pPktHdr->pCurrentV6ShortcutEntry->mibTagDelta=pPktHdr->mibTagDelta;
#endif
		pPktHdr->pCurrentV6ShortcutEntry->tagAccType=pPktHdr->egressTagAccType;
		pPktHdr->pCurrentV6ShortcutEntry->notFinishUpdated=0;
	}
#endif
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	else if(pPktHdr->shortcutStatus==RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND){
#if defined(CONFIG_RTL9602C_SERIES)
		pPktHdr->pIPv6StatefulList->mibTagDelta=pPktHdr->mibTagDelta;
#endif
		pPktHdr->pIPv6StatefulList->tagAccType=pPktHdr->egressTagAccType;
		pPktHdr->pIPv6StatefulList->notFinishUpdated=0;
	}
#endif
}

void _rtk_rg_vlanSvlanTag2SkbBuffer(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	uint16 vlanContent,svlanContent;
	
	//init with just intact
	rg_kernel.txDescMask.opts2.bit.tx_vlan_action=0x3;
	rg_kernel.txDesc.opts2.bit.tx_vlan_action=0;
	
	//ingress untag
	if((pPktHdr->tagif & CVLAN_TAGIF)==0x0 && (pPktHdr->tagif & SVLAN_TAGIF)==0x0){
		if(pPktHdr->egressVlanTagif==0x0 && pPktHdr->egressServiceVlanTagif==0x0){ //untag => untag
			TRACE("VLAN decision: (in)untag => (out)untag");
			// no need to change
		}
		else if(pPktHdr->egressVlanTagif==0x1 && pPktHdr->egressServiceVlanTagif==0x0){ //untag => ctag
			TRACE("VLAN decision: (in)untag => (out)ctag");
			vlanContent = (((pPktHdr->egressPriority&0x7)<<13)|((pPktHdr->egressVlanCfi&0x1)<<12)|(pPktHdr->egressVlanID&0xfff));
			_vlan_insert_tag(pPktHdr,skb,1,0x8100,vlanContent,0,0x0,0x0);

		}
		else if(pPktHdr->egressVlanTagif==0x0 && pPktHdr->egressServiceVlanTagif){ //untag => stag: insert stag to pktbuff
			TRACE("VLAN decision: (in)untag => (out)stag");
			svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));

#if defined(CONFIG_RTL9602C_SERIES)
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){		
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,0,0x0,0x0);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with TPID2*/){
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid2,svlanContent,0,0x0,0x0);
			}else if(pPktHdr->egressServiceVlanTagif==0x3/*tag with original stag tpid*/){
				//ingress without stag, force using TPID
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,0,0x0,0x0);
			}
#else //9600series
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){		
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,0,0x0,0x0);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with 0x8100*/){
				_vlan_insert_tag(pPktHdr,skb,1,0x8100,svlanContent,0,0x0,0x0);
			}
#endif
		}
		else if(pPktHdr->egressVlanTagif==0x1 && pPktHdr->egressServiceVlanTagif){ //untag => s+c tag: insert stag & ctag to pktbuff
			TRACE("VLAN decision: (in)untag => (out)s+c tag");
			vlanContent = (((pPktHdr->egressPriority&0x7)<<13)|((pPktHdr->egressVlanCfi&0x1)<<12)|(pPktHdr->egressVlanID&0xfff));
			svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));
#if defined(CONFIG_RTL9602C_SERIES)
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,1,0x8100,vlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with TPID2*/){
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid2,svlanContent,1,0x8100,vlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x3/*tag with original stag tpid*/){
				//ingress without stag, force using TPID
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,1,0x8100,vlanContent);
			}
#else //9600series
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,1,0x8100,vlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with 0x8100*/){
				_vlan_insert_tag(pPktHdr,skb,1,0x8100,svlanContent,1,0x8100,vlanContent);
			}
#endif
		}
	}

	//ingress with Ctag only
	else if((pPktHdr->tagif & CVLAN_TAGIF) && (pPktHdr->tagif & SVLAN_TAGIF)==0x0){
		if(pPktHdr->egressVlanTagif==0x0 && pPktHdr->egressServiceVlanTagif==0x0){ //ctag => untag
			TRACE("VLAN decision: (in)ctag => (out)untag");
			_vlan_remove_tag(pPktHdr,skb,0x8100);

		}
		else if(pPktHdr->egressVlanTagif==0x1 && pPktHdr->egressServiceVlanTagif==0x0){ //ctag => ctag
			TRACE("VLAN decision: (in)ctag => (out)ctag");
			vlanContent = (((pPktHdr->egressPriority&0x7)<<13)|((pPktHdr->egressVlanCfi&0x1)<<12)|(pPktHdr->egressVlanID&0xfff));
			_vlan_modify_tag(pPktHdr, skb, 0x8100, 0x8100, vlanContent);
		}
		else if(pPktHdr->egressVlanTagif==0x0 && pPktHdr->egressServiceVlanTagif){ //ctag => stag
			TRACE("VLAN decision: (in)ctag => (out)stag");


#if defined(CONFIG_RTL9602C_SERIES)
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));
				_vlan_modify_tag(pPktHdr, skb, 0x8100, rg_db.systemGlobal.tpid, svlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with TPID2*/){
				svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));
				_vlan_modify_tag(pPktHdr, skb, 0x8100, rg_db.systemGlobal.tpid2, svlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x3/*tag with original stag tpid*/){
				//ingress without stag, force using TPID
				svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));
				_vlan_modify_tag(pPktHdr, skb, 0x8100, rg_db.systemGlobal.tpid, svlanContent);
			}
#else //9600series
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));
				_vlan_modify_tag(pPktHdr, skb, 0x8100, rg_db.systemGlobal.tpid, svlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with 0x8100*/){
				svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));
				_vlan_modify_tag(pPktHdr, skb, 0x8100, 0x8100, svlanContent);
			}
#endif
		}
		else if(pPktHdr->egressVlanTagif==0x1 && pPktHdr->egressServiceVlanTagif){ //ctag => s+c tag
			TRACE("VLAN decision: (in)ctag => (out)s+c tag");
			vlanContent = (((pPktHdr->egressPriority&0x7)<<13)|((pPktHdr->egressVlanCfi&0x1)<<12)|(pPktHdr->egressVlanID&0xfff));
			svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));
			//modified ctag
			_vlan_modify_tag(pPktHdr, skb, 0x8100, 0x8100, vlanContent);
#if defined(CONFIG_RTL9602C_SERIES)
			//insert stag
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,0,0x0,0x0);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with TPID2*/){
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid2,svlanContent,0,0x0,0x0);
			}else if(pPktHdr->egressServiceVlanTagif==0x3/*tag with original stag tpid*/){
				//ingress without stag, force using TPID
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,0,0x0,0x0);
			}	
#else //9600series
			//insert stag
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,0,0x0,0x0);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with 0x8100*/){
				_vlan_insert_tag(pPktHdr,skb,1,0x8100,svlanContent,0,0x0,0x0);
			}
#endif
		}

	}	

	//ingress with Stag only
	else if((pPktHdr->tagif & CVLAN_TAGIF)==0x0 && (pPktHdr->tagif & SVLAN_TAGIF)){
		if(pPktHdr->egressVlanTagif==0x0 && pPktHdr->egressServiceVlanTagif==0x0){ //stag => untag
			TRACE("VLAN decision: (in)stag => (out)untag");		

#if defined(CONFIG_RTL9602C_SERIES)
			//del stag: ingress only consider original tpid.
			_vlan_remove_tag(pPktHdr,skb,pPktHdr->stagTpid);	
#else //9600 series
			//del stag: ingress only consider tpid case.
			_vlan_remove_tag(pPktHdr,skb,rg_db.systemGlobal.tpid);
#endif
		}
		else if(pPktHdr->egressVlanTagif==0x1 && pPktHdr->egressServiceVlanTagif==0x0){ //stag => ctag
			TRACE("VLAN decision: (in)stag => (out)ctag");
			vlanContent = (((pPktHdr->egressPriority&0x7)<<13)|((pPktHdr->egressVlanCfi&0x1)<<12)|(pPktHdr->egressVlanID&0xfff));
#if defined(CONFIG_RTL9602C_SERIES)
			//modified stag to ctag
			_vlan_modify_tag(pPktHdr, skb, pPktHdr->stagTpid, 0x8100, vlanContent);
#else
			//modified stag to ctag
			_vlan_modify_tag(pPktHdr, skb, rg_db.systemGlobal.tpid, 0x8100, vlanContent);
#endif
		}
		else if(pPktHdr->egressVlanTagif==0x0 && pPktHdr->egressServiceVlanTagif){ //stag => stag
			TRACE("VLAN decision: (in)stag => (out)stag");
			svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));

#if defined(CONFIG_RTL9602C_SERIES)
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_modify_tag(pPktHdr, skb, pPktHdr->stagTpid, rg_db.systemGlobal.tpid, svlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with TPID2*/){
				_vlan_modify_tag(pPktHdr, skb, pPktHdr->stagTpid, rg_db.systemGlobal.tpid2, svlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x3/*tag with original stag tpid*/){
				_vlan_modify_tag(pPktHdr, skb, pPktHdr->stagTpid, pPktHdr->stagTpid, svlanContent);
			}			
#else //9600series
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_modify_tag(pPktHdr, skb, rg_db.systemGlobal.tpid, rg_db.systemGlobal.tpid, svlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with 0x8100*/){
				_vlan_modify_tag(pPktHdr, skb, rg_db.systemGlobal.tpid, 0x8100, svlanContent);
			}
#endif
		}
		else if(pPktHdr->egressVlanTagif==0x1 && pPktHdr->egressServiceVlanTagif){ //stag => s+c tag
			TRACE("VLAN decision: (in)stag => (out)s+c tag");
			vlanContent = (((pPktHdr->egressPriority&0x7)<<13)|((pPktHdr->egressVlanCfi&0x1)<<12)|(pPktHdr->egressVlanID&0xfff));
			svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));

#if defined(CONFIG_RTL9602C_SERIES)

			//modify stag to ctag
			_vlan_modify_tag(pPktHdr, skb, pPktHdr->stagTpid, 0x8100, vlanContent);

			//insert stag
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,0,0x0,0x0);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with TPID2*/){
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid2,svlanContent,0,0x0,0x0);
			}else if(pPktHdr->egressServiceVlanTagif==0x3/*tag with original stag tpid*/){
				_vlan_insert_tag(pPktHdr,skb,1,pPktHdr->stagTpid,svlanContent,0,0x0,0x0);
			}
#else //9600series
			//modify stag to ctag
			_vlan_modify_tag(pPktHdr, skb, rg_db.systemGlobal.tpid, 0x8100, vlanContent);

			//insert stag
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_insert_tag(pPktHdr,skb,1,rg_db.systemGlobal.tpid,svlanContent,0,0x0,0x0);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with 0x8100*/){
				_vlan_insert_tag(pPktHdr,skb,1,0x8100,svlanContent,0,0x0,0x0);
			}
#endif
		}
	}

	//ingress with S+C tag
	else if((pPktHdr->tagif & CVLAN_TAGIF) && (pPktHdr->tagif & SVLAN_TAGIF)){
		if(pPktHdr->egressVlanTagif==0x0 && pPktHdr->egressServiceVlanTagif==0x0){ //s+c tag => untag
			TRACE("VLAN decision: (in)s+c tag => (out)untag");
			_vlan_remove_doubleTag(pPktHdr,skb);
		}
		else if(pPktHdr->egressVlanTagif==0x1 && pPktHdr->egressServiceVlanTagif==0x0){ //s+c tag => ctag
			TRACE("VLAN decision: (in)s+c tag => (out)ctag");
#if defined(CONFIG_RTL9602C_SERIES)
			//del stag
			_vlan_remove_tag(pPktHdr,skb,pPktHdr->stagTpid);			
#else //9600series
			//del stag
			_vlan_remove_tag(pPktHdr,skb,rg_db.systemGlobal.tpid);
#endif
			//modify ctag
			vlanContent = (((pPktHdr->egressPriority&0x7)<<13)|((pPktHdr->egressVlanCfi&0x1)<<12)|(pPktHdr->egressVlanID&0xfff));
			_vlan_modify_tag(pPktHdr, skb, 0x8100, 0x8100, vlanContent);
		}
		else if(pPktHdr->egressVlanTagif==0x0 && pPktHdr->egressServiceVlanTagif){ //s+c tag => stag
			TRACE("VLAN decision: (in)s+c tag => (out)stag");
#if defined(CONFIG_RTL9602C_SERIES)
			//del stag
			_vlan_remove_tag(pPktHdr,skb,pPktHdr->stagTpid);

			//modify ctag to stag
			svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));

			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_modify_tag(pPktHdr, skb, 0x8100, rg_db.systemGlobal.tpid, svlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with TPID2*/){
				_vlan_modify_tag(pPktHdr, skb, 0x8100, rg_db.systemGlobal.tpid2, svlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x3/*tag with original stag tpid*/){
				_vlan_modify_tag(pPktHdr, skb, 0x8100, pPktHdr->stagTpid, svlanContent);
			}
#else //9600series
			//del stag
			_vlan_remove_tag(pPktHdr,skb,rg_db.systemGlobal.tpid);

			//modify ctag to stag
			svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));

			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_modify_tag(pPktHdr, skb, 0x8100, rg_db.systemGlobal.tpid, svlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with 0x8100*/){
				_vlan_modify_tag(pPktHdr, skb, 0x8100, 0x8100, svlanContent);
			}
#endif
		}
		else if(pPktHdr->egressVlanTagif==0x1 && pPktHdr->egressServiceVlanTagif){ //s+c tag => s+c tag
			TRACE("VLAN decision: (in)s+c tag => (out)s+c tag");
			//modify s+c tag
			vlanContent = (((pPktHdr->egressPriority&0x7)<<13)|((pPktHdr->egressVlanCfi&0x1)<<12)|(pPktHdr->egressVlanID&0xfff));
			svlanContent = (((pPktHdr->egressServicePriority&0x7)<<13)|((pPktHdr->egressServiceVlanDei&0x1)<<12)|(pPktHdr->egressServiceVlanID&0xfff));

#if defined(CONFIG_RTL9602C_SERIES)
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_modify_doubleTag(pPktHdr,skb,rg_db.systemGlobal.tpid,svlanContent,0x8100,vlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with TPID2*/){
				_vlan_modify_doubleTag(pPktHdr,skb,rg_db.systemGlobal.tpid2,svlanContent,0x8100,vlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x3/*tag with original stag tpid*/){
				_vlan_modify_doubleTag(pPktHdr,skb,pPktHdr->stagTpid,svlanContent,0x8100,vlanContent);
			}
#else //9600series
			if( pPktHdr->egressServiceVlanTagif==0x1/*tagged with tpid*/){ 
				_vlan_modify_doubleTag(pPktHdr,skb,rg_db.systemGlobal.tpid,svlanContent,0x8100,vlanContent);
			}else if(pPktHdr->egressServiceVlanTagif==0x2/*tag with 0x8100*/){
				_vlan_modify_doubleTag(pPktHdr,skb,0x8100,svlanContent,0x8100,vlanContent);
			}
#endif
		}
	}
}

/*
* Tranlate pPktHdr->(final cvlan/svlan decition) into tx_desc or pktbuff. 
*/
__IRAM_FWDENG
int _rtk_rg_TranslateVlanSvlan2Packet(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr, uint32 dataPathToWifi)
{
	//20141002LUKE: remove outter IP header, GRE header, PPP header
	//20141017LUKE: remove outter IP header, UDP header, L2TP header, PPP header
	//20150206LUKE: remove outter IPv6 header
	if(pPktHdr->tagif&PPTP_INNER_TAGIF||pPktHdr->tagif&L2TP_INNER_TAGIF||pPktHdr->tagif&DSLITE_INNER_TAGIF){
		TRACE("Remove Tunnel tag");
		_rtk_rg_removeTunnelTag(skb, pPktHdr);
	}
	
	//20140721LUKE: check for dmac2cvid or not!!
	if(pPktHdr->dmac2VlanID!=FAIL){
		DEBUG("rg_db.systemGlobal.ponDmac2cvidDisabled=0x%x, force disbled DAMC2CVID to port[%d]!",rg_db.systemGlobal.dmac2cvidDisabledPortmask, pPktHdr->egressMACPort);
		if(rg_db.systemGlobal.dmac2cvidDisabledPortmask & (1<< pPktHdr->egressMACPort))//force diabled PON DAMC2CVID
		{
			TRACE("rg_db.systemGlobal.ponDmac2cvidDisabled=0x%x, force disbled DAMC2CVID to port[%d]!",rg_db.systemGlobal.dmac2cvidDisabledPortmask, pPktHdr->egressMACPort);
		}
		else
		{
			TRACE("DMAC2CVID decision: egressVlan=>%d, egressTagif=>%s",pPktHdr->dmac2VlanID,pPktHdr->dmac2VlanTagif==1?"TAG":"UNTAG");
			pPktHdr->egressVlanID=pPktHdr->dmac2VlanID;
			pPktHdr->egressVlanTagif=pPktHdr->dmac2VlanTagif;
		}
	}	
	

	/* do Stag/Ctag final Tx decision, by tx_desc or modify pktbuff*/
	if(pPktHdr->shortcutStatus!=RG_SC_MATCH)
		_rtk_rg_get_stagCtagAccelerationType(pPktHdr,dataPathToWifi);
	
#if defined(CONFIG_APOLLO_FPGA_PHY_TEST)
	pPktHdr->egressTagAccType = RG_FWDENGINE_ACL_ACC_TYPE_TX_PKTBUFF;
#endif

	if(pPktHdr->egressTagAccType==RG_FWDENGINE_ACL_ACC_TYPE_TX_DESC){
		/*init tx_desc*/
		rg_kernel.txDescMask.opts2.bit.tx_vlan_action=0x3;
		rg_kernel.txDescMask.opts2.bit.vidl=0xff;
		rg_kernel.txDescMask.opts2.bit.vidh=0xf;
		rg_kernel.txDescMask.opts2.bit.prio=0x7;
		rg_kernel.txDescMask.opts2.bit.cfi=0x1;

		//rg_kernel.txDesc.opts2.bit.tx_vlan_action=0;
		//rg_kernel.txDesc.opts2.bit.vidl=0;
		//rg_kernel.txDesc.opts2.bit.vidh=0;
		//rg_kernel.txDesc.opts2.bit.prio=0;
		//rg_kernel.txDesc.opts2.bit.cfi=0;
		if(pPktHdr->egressVlanTagif==1){//Tag
			rg_kernel.txDesc.opts2.bit.tx_vlan_action=0x3;//remarking
			rg_kernel.txDesc.opts2.bit.vidl=(pPktHdr->egressVlanID&0xff);
			rg_kernel.txDesc.opts2.bit.vidh=((pPktHdr->egressVlanID&0xf00)>>8);
			rg_kernel.txDesc.opts2.bit.prio=pPktHdr->egressPriority;
			rg_kernel.txDesc.opts2.bit.cfi=pPktHdr->egressVlanCfi;
			
		}else{//Untag
			rg_kernel.txDesc.opts2.bit.tx_vlan_action=0x2;//remove ctag
		}
	}else	{
		_rtk_rg_vlanSvlanTag2SkbBuffer(skb, pPktHdr);
	}
	
#if 0

	if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TRACE_DUMP){
		if(pPktHdr->egressVlanTagif==1 &&
			pPktHdr->egressVlanID==CONFIG_DEFAULT_TO_SLAVE_GMAC_VID && 
			pPktHdr->egressPriority==CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI){
				//FIXEME: Test by dump_packet
				dump_packet(skb->data,64,"wifi slave packet:");
		}else if(dataPathFormWifi==1){
				//FIXEME: Test by dump_packet
				dump_packet(skb->data,64,"wifi master packet:");
		}else{
			//FIXEME: Test by dump_packet
			dump_packet(skb->data,64,"HW packet:");
		}
	}
#endif

	return RT_ERR_RG_OK;
}

rtk_rg_fwdEngineReturn_t _rtk_rg_ipv6BindingDecision(rtk_rg_pktHdr_t *pPktHdr)
{
	int wanGroupIdx,wanTypeIdx;
	rtk_rg_fwdEngineReturn_t ret=RG_FWDENGINE_RET_CONTINUE;
	rtk_l34_bindAct_t bindAction=L34_BIND_ACT_END;
	
	if(_rtk_rg_bindingRuleCheck(pPktHdr,&wanGroupIdx)==RG_FWDENGINE_RET_HIT_BINDING)
	{
		pPktHdr->netifIdx=rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].index;		
		if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.bind_wan_type_ipv6>=0)
			wanTypeIdx=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.bind_wan_type_ipv6;
		else
			wanTypeIdx=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.bind_wan_type_ipv4;
		pPktHdr->bindNextHopIdx=rg_db.wantype[wanTypeIdx].rtk_wantype.nhIdx; //nextHopIdx for pPktHdr->bindNextHopIdx
		switch(rg_db.wantype[wanTypeIdx].rtk_wantype.wanType)
		{
			case L34_WAN_TYPE_L2_BRIDGE:	//unmatch: follow hw register setting
				TRACE("Unmatch for L3 binding to L2 WAN...");
				bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_UNMATCHED_L3L2];
				break;
			case L34_WAN_TYPE_L3_ROUTE:	
				TRACE("Unmatch for L3 binding to L3 WAN...nhidx = %d",rg_db.wantype[wanTypeIdx].rtk_wantype.nhIdx);
				//20140717LUKE: this is special case, though we are binding from L3 to L3, we can have option to trap such packet.
				bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_UNMATCHED_L3L3];
				break;
			case L34_WAN_TYPE_L34NAT_ROUTE:	//unmatch: drop!!
				TRACE("Unmatch for L3 binding to L34 WAN...DROP!!");
				//FIXME: if one binding rule contain v4 and v6, and v4 is set to NAPT, then v6 will go here...forced drop!
				bindAction=L34_BIND_ACT_DROP;//rg_db.systemGlobal.l34BindAction[L34_BIND_UNMATCHED_L3L34];
				break;
			case L34_WAN_TYPE_L34_CUSTOMIZED:	//unmatch: follow hw register setting
				TRACE("Unmatch for L3 binding to customized WAN...");
				bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_CUSTOMIZED_L3];
				break;
			default:
				break;
		}

		//if unmatch, do action!!
		ret=_rtk_rg_unmatchBindingAct(pPktHdr,bindAction,NULL);
	}

	return ret;
}

rtk_rg_fwdEngineReturn_t _rtk_rg_ipv6L34Forward(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	int i,ret=0;
	
	int8 gatewayIP = -1;
	//int8 compareBit;
	uint8 nb_hash_idx;
	//uint16 index;
	//uint16 max;
	uint16 res;
	uint16 matchNeighbor;
	rtk_rg_naptDirection_t toWANIntf;

	rtk_ipv6_addr_t unspecidiedIP={{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
	rtk_l34_nexthop_entry_t *nexthopEntry;
	rtk_ipv6Routing_entry_t *entry;
	rtk_ipv6Neighbor_entry_t *neighbor;
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	rtk_rg_ipv6_layer4_linkList_t *pIPv6ConnList;
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	rtk_rg_ipClassification_t sipClass,dipClass;
	rtk_rg_sipDipClassification_t sipDipClass;
#endif
#endif
	TRACE("IPv6 L34 Forward");

	//Trap IPv6 Routing Link Local to protocal stack.
	if(pPktHdr->ingressLocation!=RG_IGR_PROTOCOL_STACK){
		if(pPktHdr->pIpv6Dip[0]==0xfe && pPktHdr->pIpv6Dip[1]==0x80){
			TRACE("IPv6 Routing to Link local address, Trap to PS!");
			return RG_FWDENGINE_RET_TO_PS;
		}else if(pPktHdr->tagif&V6TRAP_TAGIF){
			TRACE("IPv6 Routing with Hop-by-hop, Trap to PS withou ACL!");
			return RG_FWDENGINE_RET_ACL_TO_PS;
		}else if(!memcmp(pPktHdr->pIpv6Sip,&unspecidiedIP,sizeof(rtk_ipv6_addr_t))){
			TRACE("IPv6 Routing with unspecified address, Drop!");
			return RG_FWDENGINE_RET_DROP;
		}
	}

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT
	sipClass=_rtk_rg_ipv6_sip_classification(pPktHdr->pIpv6Sip,pPktHdr);
	dipClass=_rtk_rg_ipv6_dip_classification(pPktHdr->pIpv6Dip,pPktHdr);
	sipDipClass=rg_db.systemGlobal.sipDipClass[sipClass][dipClass];
	TRACE("sipClass=%d, dipClass=%d, sipDipClass=%d",sipClass,dipClass,sipDipClass);
	if(sipDipClass==SIP_DIP_CLASS_NAPT){
		pPktHdr->fwdDecision=RG_FWD_DECISION_V6NAPT;
		if(pPktHdr->tagif&ESP_TAGIF)
		{
			TRACE("IPv6 ESP with NAPT, Trap to PS withou ACL!");
			return RG_FWDENGINE_RET_ACL_TO_PS;
		}
	}else if(sipDipClass==SIP_DIP_CLASS_NAPTR){
		pPktHdr->fwdDecision=RG_FWD_DECISION_V6NAPTR;
		if(pPktHdr->tagif&ESP_TAGIF)
		{
			TRACE("IPv6 ESP with NAPTR, Trap to PS withou ACL!");
			return RG_FWDENGINE_RET_ACL_TO_PS;
		}
	}else{	
		pPktHdr->fwdDecision=RG_FWD_DECISION_V6ROUTING;
	}
#endif

	//Check if hop limit reach
	if((pPktHdr->pIPv6HopLimit==NULL)||(pPktHdr->pIPv6HopLimit!=NULL && *pPktHdr->pIPv6HopLimit<=1))
	{
		TRACE("IPV6: Hop limit reach, trap to PS!");
		return RG_FWDENGINE_RET_TO_PS;
	}
	
	//check ingress PPPoE
	#if defined(CONFIG_RTL9602C_SERIES)
	if(pPktHdr->tagif&PPPOE_TAGIF)
	#else
	if((pPktHdr->tagif&PPPOE_TAGIF) && (rg_db.systemGlobal.l34GlobalState[L34_GLOBAL_PPPKEEP_STATE]==DISABLED))
	#endif
	{
		for (i=0;i<MAX_PPPOE_SW_TABLE_SIZE;i++)
		{
			if(pPktHdr->sessionId == rg_db.pppoe[i].rtk_pppoe.sessionID)
				break;
		}
		if (i==MAX_PPPOE_SW_TABLE_SIZE)	//unmatch, trap
		{
			TRACE("IPV6: PPPoE ID lookup miss, trap to PS!");
			return RG_FWDENGINE_RET_TO_PS;
		}
	}


	//learning neighbor for syn packet
	if((pPktHdr->tagif&TCP_TAGIF)&&(pPktHdr->tcpFlags.syn==1))
	{
		int neighborIdx;
		int l3Idx=_rtk_rg_v6L3lookup(pPktHdr->pIpv6Sip);
		int hashValue,i;
		hashValue=_rtk_rg_IPv6NeighborHash(pPktHdr->pIpv6Sip+8,l3Idx);	
		//TRACE("SIP neighbor learn!");
		TRACE("Learning Neighbor hashValue=%d  l3Idx=%d interfaceId(%02x%02x:%02x%02x:%02x%02x:%02x%02x) ",hashValue,l3Idx,
			(pPktHdr->pIpv6Sip+8)[0],(pPktHdr->pIpv6Sip+8)[1],(pPktHdr->pIpv6Sip+8)[2],(pPktHdr->pIpv6Sip+8)[3],
			(pPktHdr->pIpv6Sip+8)[4],(pPktHdr->pIpv6Sip+8)[5],(pPktHdr->pIpv6Sip+8)[6],(pPktHdr->pIpv6Sip+8)[7]);
		neighborIdx=(hashValue<<3);
		for(i=0;i<8;i++)
		{
			if((rg_db.v6neighbor[neighborIdx+i].rtk_v6neighbor.valid==1)&&
				(rg_db.v6neighbor[neighborIdx+i].rtk_v6neighbor.ipv6RouteIdx==l3Idx)&&
				(memcmp(&rg_db.v6neighbor[neighborIdx+i].rtk_v6neighbor.ipv6Ifid,pPktHdr->pIpv6Sip+8,8)==0))
			{
				break;
			}
				
		}
		if(i==8) //SrcIP is not in neighbor table, add it.
		{	
			_rtk_rg_neighborAndMacEntryAdd(pPktHdr->pIpv6Sip,pPktHdr->sipL3Idx,pPktHdr->pSmac,pPktHdr->ingressPort,pPktHdr->wlan_dev_idx,&neighborIdx);
			TRACE("Src IPv6 is not in neighbor table, add it at idx[%d]!",neighborIdx);
		}
	}

#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT //Seperate for pPktHdr->fwdDecision is NAPT/NAPTR case

	//learning neighbor for NA packet in NAPT, NAPTR mode
	if(pPktHdr->ICMPv6Type==0x88 &&(pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPT || pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPTR)){
		pPktHdr->dipL3Idx=_rtk_rg_v6L3lookup(pPktHdr->pIpv6Dip);
		if(pPktHdr->dipL3Idx>=0){
			//make sure DA is to gwteway IP
			if(memcmp(pPktHdr->pIpv6Dip,rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.ipv6Addr.ipv6_addr,IPV6_ADDR_LEN)==0){
				DEBUG("NA packet DA=gatewayIP, dipL3Idx=%d",pPktHdr->dipL3Idx);
				_rtk_rg_neighborAgent(skb,pPktHdr);
				TRACE("IPv6: either packet-to-gateway or neighbor advertisement, to PS!");
				return RG_FWDENGINE_RET_TO_PS;
			}
		}
	}

	if(pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPT){

			pPktHdr->dipL3Idx=_rtk_rg_v6L3lookup(pPktHdr->pIpv6Dip);

			if(pPktHdr->dipL3Idx>=0)
			{
				TRACE("IPv6(NAPT): routing table lookup, Hit Routing[%d]",pPktHdr->dipL3Idx);
			}
			
			if (pPktHdr->dipL3Idx == -1)
			{
				//no entry matched, Trap
				TRACE("IPv6: routing table lookup failed, to PS!");
			} 
			else
			{
				//20150731LUKE:if we are routing with link-local address, drop it!
				if(pPktHdr->pIpv6Sip[0]==0xfe && pPktHdr->pIpv6Sip[1]==0x80){
					TRACE("IPv6 Routing with Link local address, Trap!");
					return RG_FWDENGINE_RET_TO_PS;
				}
				//hit! check process column
				switch (rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.type)
				{
					case L34_IPV6_ROUTE_TYPE_DROP:
						//sprintf(msg,"After bindForward, Drop!");
						TRACE("IPv6: Drop by routing table!");
						return RG_FWDENGINE_RET_DROP;
					case L34_IPV6_ROUTE_TYPE_TRAP:
						//sprintf(msg,"After bindForward, trap to CPU!");
						TRACE("IPv6: Trap to PS by routing table!");
						return RG_FWDENGINE_RET_TO_PS;
					case L34_IPV6_ROUTE_TYPE_GLOBAL:
					{
						/* Read NextHop */
						TRACE("IPv6: Global routing..%s",rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.rt2waninf?"rt2WAN":"rt2LAN");
						//since the entry should be the one we matched when we left the loop,
						//there is no need to get it out once again!
						entry=&rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route;		
						/* Read NextHop (Routing) */
						pPktHdr->nexthopIdx=entry->nhOrIfidIdx;
						nexthopEntry=&rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop;
						pPktHdr->dmacL2Idx=nexthopEntry->nhIdx;
						pPktHdr->netifIdx=nexthopEntry->ifIdx;
						pPktHdr->extipIdx=pPktHdr->netifIdx;

						//20150310LUKE: Check MTU, if over, just return to protocol stack since we can't split here
						if(rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu < pPktHdr->l3Len)
						{
							pPktHdr->overMTU=1;
							TRACE("v6Packet L3 size(%d) is bigger than interface[%d]'s MTU(%d)",pPktHdr->l3Len,pPktHdr->netifIdx,rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu);
							return RG_FWDENGINE_RET_TO_PS;
						}
						//DEBUG("pPktHdr->dipL3Idx=%d",pPktHdr->dipL3Idx);
						//DEBUG("pPktHdr->nexthopIdx=%d",pPktHdr->nexthopIdx);
						//DEBUG("pPktHdr->netifIdx=%d",pPktHdr->netifIdx);
						//DEBUG("pPktHdr->extipIdx=%d",pPktHdr->extipIdx);
						//DEBUG("pPktHdr->dmacL2Idx=%d",pPktHdr->dmacL2Idx);
						
						if(entry->rt2waninf)
						{					
							toWANIntf=NAPT_DIRECTION_OUTBOUND;					
							pIPv6ConnList=NULL;
							if(pPktHdr->tagif&V6FRAG_TAGIF)
							{
								return _rtk_rg_fwdEngine_v6FragmentHandling(toWANIntf,nexthopEntry->type,&pIPv6ConnList,pPktHdr,skb);
							}
							else	//unfragment
							{
								//ipv6 stateful connection tracking, if outbound first time, fill software data structure
								//if inbound without outbound, drop it.
								ret=RG_FWDENGINE_RET_NAPT_OK;
								if(pPktHdr->tagif&TCP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6TCPOutboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								else if(pPktHdr->tagif&UDP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6UDPOutboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;
							}
		
							//update direction and wanType(ether or pppoe)
							if(pIPv6ConnList!=NULL)
							{
								pIPv6ConnList->direction=toWANIntf;
								pIPv6ConnList->wanType=nexthopEntry->type;
								pPktHdr->pIPv6StatefulList=pIPv6ConnList;
								pPktHdr->shortcutStatus=RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND;
								pPktHdr->pIPv6StatefulList->idleSecs=0;
							}
						}
						else
						{
							TRACE("NAPT, but hit Routing[%d]! The routing should not to Lan! Drop!!!",pPktHdr->dipL3Idx);
							return RG_FWDENGINE_RET_DROP;
						}
						
						ret = _rtk_rg_fwdEngine_ipv6PacketModify(toWANIntf,nexthopEntry->type,pPktHdr,skb);
						if(ret!=RG_FWDENGINE_RET_CONTINUE)
							return ret; 	//RG_FWDENGINE_RET_TO_PS				
						
						return RG_FWDENGINE_RET_DIRECT_TX;
					}
						
					case L34_IPV6_ROUTE_TYPE_LOCAL:
					{
						TRACE("IPv6: Local routing..%s",rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.rt2waninf?"rt2WAN":"rt2LAN");
						//lookup for neighbor table
						res = 0;
						nb_hash_idx = _rtk_rg_IPv6NeighborHash(pPktHdr->pIpv6Dip+8, (uint8)pPktHdr->dipL3Idx);
						
						for(i=0;i<8;i++)
						{
							matchNeighbor = (nb_hash_idx<<3)+i;
							//TRACE("the matchNeighbor idx = %d\n",matchNeighbor);
							//ASSERT_EQ(dal_apollomp_l34_ipv6NeighborTable_get(matchNeighbor, &neighbor), RT_ERR_OK);
							neighbor = &rg_db.v6neighbor[matchNeighbor].rtk_v6neighbor;
							//TRACE("the neighbor.ipv6RouteIdx = %d, matchEntry = %d\n",neighbor->ipv6RouteIdx,matchEntry);
							if(neighbor->valid && 
								(neighbor->ipv6RouteIdx == pPktHdr->dipL3Idx)&&
								(_rtk_rg_CompareIFID(pPktHdr->pIpv6Dip+8, neighbor->ipv6Ifid)))
							{
								//rtlglue_printf("HIT!!!!!\n");
								TRACE("IPv6: Hit neighbor table!L2IDX = %d rt=%d",neighbor->l2Idx,pPktHdr->dipL3Idx);
								res = 1;
								break;
							}
						}
						
						if (res == 0)
						{
							//if not hit, trap to cpu
							TRACE("IPv6: Neighbor table un-hit, DROP!");
							_rtk_rg_fwdengine_handleNeighborMiss(pPktHdr);
							return RG_FWDENGINE_RET_DROP;
						}
						
						entry=&rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route;		
		
						pPktHdr->dmacL2Idx=neighbor->l2Idx;
						pPktHdr->netifIdx=entry->nhOrIfidIdx&0x7;
						pPktHdr->extipIdx=pPktHdr->netifIdx;

						//20150310LUKE: Check MTU, if over, just return to protocol stack since we can't split here
						if(rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu < pPktHdr->l3Len)
						{
							pPktHdr->overMTU=1;
							TRACE("v6Packet L3 size(%d) is bigger than interface[%d]'s MTU(%d)",pPktHdr->l3Len,pPktHdr->netifIdx,rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu);
							return RG_FWDENGINE_RET_TO_PS;
						}
						//DEBUG("pPktHdr->dipL3Idx=%d",pPktHdr->dipL3Idx);
						//DEBUG("pPktHdr->netifIdx=%d",pPktHdr->netifIdx);
						//DEBUG("pPktHdr->extipIdx=%d",pPktHdr->extipIdx);
						//DEBUG("pPktHdr->dmacL2Idx=%d",pPktHdr->dmacL2Idx);
						
						if(entry->rt2waninf==1)
						{		
							toWANIntf=NAPT_DIRECTION_OUTBOUND;
							pIPv6ConnList=NULL;
							if(pPktHdr->tagif&V6FRAG_TAGIF)
							{
								return _rtk_rg_fwdEngine_v6FragmentHandling(toWANIntf,L34_NH_ETHER,&pIPv6ConnList,pPktHdr,skb);
							}
							else	//unfragment
							{
								//ipv6 stateful connection tracking, if outbound first time, fill software data structure
								//if inbound without outbound, drop it.
								ret=RG_FWDENGINE_RET_NAPT_OK;
								if(pPktHdr->tagif&TCP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6TCPOutboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								else if(pPktHdr->tagif&UDP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6UDPOutboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;
							}
		
							//update direction and wanType(ether or pppoe)
							if(pIPv6ConnList!=NULL)
							{
								pIPv6ConnList->direction=toWANIntf;
								pIPv6ConnList->wanType=L34_NH_ETHER;
								pPktHdr->pIPv6StatefulList=pIPv6ConnList;
								pPktHdr->shortcutStatus=RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND;
								pPktHdr->pIPv6StatefulList->idleSecs=0;
							}
						}
						else
						{
							TRACE("NAPT, but hit Routing[%d]! The routing should not to Lan! Drop!!!",pPktHdr->dipL3Idx);
							return RG_FWDENGINE_RET_DROP;
						}
						ret = _rtk_rg_fwdEngine_ipv6PacketModify(toWANIntf,L34_NH_ETHER,pPktHdr,skb);
						if(ret!=RG_FWDENGINE_RET_CONTINUE)
							return ret; 	//RG_FWDENGINE_RET_TO_PS
		
						return RG_FWDENGINE_RET_DIRECT_TX;
					}
					default:
						//we should not get here
						assert_ok(0);
						break;
		
				}
					
			}


	}else if(pPktHdr->fwdDecision==RG_FWD_DECISION_V6NAPTR){

			int32 hashIndex=-1, isTcp=0;
			rtk_rg_ipv6_layer4_linkList_t *pIPv6InboundList=NULL;
			rtk_ipv6_addr_t transIP;
			uint16 transPort;

			if(pPktHdr->tagif&TCP_TAGIF)
				isTcp=1;
			else
				isTcp=0;

			//search for the pIPv6InboundList
			_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pIPv6InboundList,&hashIndex,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport,isTcp,0);

			if(pIPv6InboundList==NULL)
			{
				//not found inbound connList, check upnp, virtual server,dmz
				//initial with original sip, sport
				memcpy(transIP.ipv6_addr,pPktHdr->pIpv6Dip,IPV6_ADDR_LEN);
				transPort = pPktHdr->dport;
				ret = _rtk_rg_fwdEngine_ipv6ConnType_lookup(pPktHdr, &transIP, &transPort);
				if(ret == RG_FWDENGINE_RET_CONTINUE){
					//DMZ or Virtual Server or Upnp hit. 
					TRACE("Server in Lan check success... start to fill connList!");
				}else{
					TRACE("Can not Found inbound list...DROP");
					return RG_FWDENGINE_RET_DROP;
				}
				
			}else{
				//found inbound connList
				pPktHdr->ipv6StatefulHashValue = hashIndex;
				pPktHdr->pIPv6StatefulList = pIPv6InboundList;
				TRACE("Found inbound pIPv6InboundList[%d] %p",pPktHdr->ipv6StatefulHashValue,pPktHdr->pIPv6StatefulList);
	
				//get routing entry of lan IP
				memcpy(transIP.ipv6_addr,pIPv6InboundList->internalIP.ipv6_addr,IPV6_ADDR_LEN);
			}

			DEBUG("transIP is %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
				transIP.ipv6_addr[0],transIP.ipv6_addr[1],transIP.ipv6_addr[2],transIP.ipv6_addr[3],
				transIP.ipv6_addr[4],transIP.ipv6_addr[5],transIP.ipv6_addr[6],transIP.ipv6_addr[7],
				transIP.ipv6_addr[8],transIP.ipv6_addr[9],transIP.ipv6_addr[10],transIP.ipv6_addr[11],
				transIP.ipv6_addr[12],transIP.ipv6_addr[13],transIP.ipv6_addr[14],transIP.ipv6_addr[15]);


			pPktHdr->dipL3Idx=_rtk_rg_v6L3lookup(transIP.ipv6_addr);	
			TRACE("v6L3lookup dipL3Idx=%d",pPktHdr->dipL3Idx);
			
			if(pPktHdr->dipL3Idx>=0)
			{
				TRACE("IPv6(NAPT): routing table lookup, Hit Routing[%d]",pPktHdr->dipL3Idx);
			}
			
			if (pPktHdr->dipL3Idx == -1)
			{
				//no entry matched, Trap
				TRACE("IPv6: routing table lookup failed, to PS!");
				return RG_FWDENGINE_RET_TO_PS;
			} 
			else
			{	
				//20150731LUKE:if we are routing with link-local address, drop it!
				if(pPktHdr->pIpv6Sip[0]==0xfe && pPktHdr->pIpv6Sip[1]==0x80){
					TRACE("IPv6 Routing with Link local address, Trap!");
					return RG_FWDENGINE_RET_TO_PS;
				}
				//hit! check process column
				switch (rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.type)
				{
					case L34_IPV6_ROUTE_TYPE_DROP:
						//sprintf(msg,"After bindForward, Drop!");
						TRACE("IPv6: Drop by routing table!");
						return RG_FWDENGINE_RET_DROP;
					case L34_IPV6_ROUTE_TYPE_TRAP:
						//sprintf(msg,"After bindForward, trap to CPU!");
						TRACE("IPv6: Trap to PS by routing table!");
						return RG_FWDENGINE_RET_TO_PS;
					case L34_IPV6_ROUTE_TYPE_GLOBAL:
					{
						/* Read NextHop */
						TRACE("IPv6: Global routing..%s",rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.rt2waninf?"rt2WAN":"rt2LAN");
						//since the entry should be the one we matched when we left the loop,
						//there is no need to get it out once again!
						entry=&rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route;		
						/* Read NextHop (Routing) */
						pPktHdr->nexthopIdx=entry->nhOrIfidIdx;
						nexthopEntry=&rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop;
						pPktHdr->dmacL2Idx=nexthopEntry->nhIdx;
						pPktHdr->netifIdx=nexthopEntry->ifIdx;

						//20150310LUKE: Check MTU, if over, just return to protocol stack since we can't split here
						if(rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu < pPktHdr->l3Len)
						{
							pPktHdr->overMTU=1;
							TRACE("v6Packet L3 size(%d) is bigger than interface[%d]'s MTU(%d)",pPktHdr->l3Len,pPktHdr->netifIdx,rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu);
							return RG_FWDENGINE_RET_TO_PS;
						}
						if(entry->rt2waninf)
						{
							TRACE("NAPTR, but hit Routing[%d]! The routing should not to Wan! Drop!!!",pPktHdr->dipL3Idx);
							return RG_FWDENGINE_RET_DROP;
						}
						else
						{
							toWANIntf=NAPT_DIRECTION_INBOUND;					
							pIPv6ConnList=NULL;
							if(pPktHdr->tagif&V6FRAG_TAGIF)
							{
								return _rtk_rg_fwdEngine_v6FragmentHandling(toWANIntf,nexthopEntry->type,&pIPv6ConnList,pPktHdr,skb);
							}
							else	//unfragment
							{
								//ipv6 stateful connection tracking, if outbound first time, fill software data structure
								//if inbound without outbound, drop it.
								ret=RG_FWDENGINE_RET_NAPT_OK;
								if(pPktHdr->tagif&TCP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6TCPInboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								else if(pPktHdr->tagif&UDP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6UDPInboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;
							}
		
							//update direction and wanType(ether or pppoe)
							if(pIPv6ConnList!=NULL)
							{
								pIPv6ConnList->direction=toWANIntf;
								pIPv6ConnList->wanType=nexthopEntry->type;
								pPktHdr->pIPv6StatefulList=pIPv6ConnList;
								pPktHdr->shortcutStatus=RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND;
								pPktHdr->pIPv6StatefulList->idleSecs=0;
							}
						}
						ret = _rtk_rg_fwdEngine_ipv6PacketModify(toWANIntf,nexthopEntry->type,pPktHdr,skb);
						if(ret!=RG_FWDENGINE_RET_CONTINUE)
							return ret; 	//RG_FWDENGINE_RET_TO_PS				
						
						return RG_FWDENGINE_RET_DIRECT_TX;
					}
					case L34_IPV6_ROUTE_TYPE_LOCAL:
					{
						TRACE("IPv6: Local routing..%s",rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.rt2waninf?"rt2WAN":"rt2LAN");
						//lookup for neighbor table
						res = 0;
						nb_hash_idx = _rtk_rg_IPv6NeighborHash(transIP.ipv6_addr+8, (uint8)pPktHdr->dipL3Idx);
						DEBUG("nb_hash_idx=%d pPktHdr->dipL3Idx=%d internalIP[64:128](%02x%02x:%02x%02x:%02x%02x:%02x%02x)",
							nb_hash_idx,pPktHdr->dipL3Idx,
							transIP.ipv6_addr[8],transIP.ipv6_addr[9],transIP.ipv6_addr[10],transIP.ipv6_addr[11],
							transIP.ipv6_addr[12],transIP.ipv6_addr[13],transIP.ipv6_addr[14],transIP.ipv6_addr[15]);
						
						for(i=0;i<8;i++)
						{
							matchNeighbor = (nb_hash_idx<<3)+i;
							//TRACE("the matchNeighbor idx = %d\n",matchNeighbor);
							//ASSERT_EQ(dal_apollomp_l34_ipv6NeighborTable_get(matchNeighbor, &neighbor), RT_ERR_OK);
							neighbor = &rg_db.v6neighbor[matchNeighbor].rtk_v6neighbor;
							//TRACE("the neighbor.ipv6RouteIdx = %d, matchEntry = %d\n",neighbor->ipv6RouteIdx,matchEntry);
							if(neighbor->valid && 
								(neighbor->ipv6RouteIdx == pPktHdr->dipL3Idx)&&
								(_rtk_rg_CompareIFID(transIP.ipv6_addr+8, neighbor->ipv6Ifid)))
							{
								//rtlglue_printf("HIT!!!!!\n");
								TRACE("IPv6: Hit neighbor table!L2IDX = %d rt=%d",neighbor->l2Idx,pPktHdr->dipL3Idx);
								res = 1;
								break;
							}
						}
						
						if (res == 0)
						{
							//if not hit, trap to cpu
							TRACE("IPv6: Neighbor table un-hit, DROP!");
							_rtk_rg_fwdengine_handleNeighborMiss(pPktHdr);
							return RG_FWDENGINE_RET_DROP;
						}
						
						entry=&rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route;		
		
						pPktHdr->dmacL2Idx=neighbor->l2Idx;
						pPktHdr->netifIdx=entry->nhOrIfidIdx&0x7;

						//20150310LUKE: Check MTU, if over, just return to protocol stack since we can't split here
						if(rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu < pPktHdr->l3Len)
						{
							pPktHdr->overMTU=1;
							TRACE("v6Packet L3 size(%d) is bigger than interface[%d]'s MTU(%d)",pPktHdr->l3Len,pPktHdr->netifIdx,rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu);
							return RG_FWDENGINE_RET_TO_PS;
						}
						//DEBUG("pPktHdr->dipL3Idx=%d",pPktHdr->dipL3Idx);
						//DEBUG("pPktHdr->netifIdx=%d",pPktHdr->netifIdx);
						//DEBUG("pPktHdr->extipIdx=%d",pPktHdr->extipIdx);
						//DEBUG("pPktHdr->dmacL2Idx=%d",pPktHdr->dmacL2Idx);
							
						if(entry->rt2waninf==1)
						{
							TRACE("NAPTR, but hit Routing[%d]! The routing should not to Wan! Drop!!!",pPktHdr->dipL3Idx);
							return RG_FWDENGINE_RET_DROP;
						}
						else
						{
							toWANIntf=NAPT_DIRECTION_INBOUND;
							pIPv6ConnList=NULL;
							if(pPktHdr->tagif&V6FRAG_TAGIF)
							{
								return _rtk_rg_fwdEngine_v6FragmentHandling(toWANIntf,L34_NH_ETHER,&pIPv6ConnList,pPktHdr,skb);
							}
							else	//unfragment
							{
								//ipv6 stateful connection tracking, if outbound first time, fill software data structure
								//if inbound without outbound, drop it.
								ret=RG_FWDENGINE_RET_NAPT_OK;
								if(pPktHdr->tagif&TCP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6TCPInboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								else if(pPktHdr->tagif&UDP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6UDPInboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;
							}
		
							//update direction and wanType(ether or pppoe)
							if(pIPv6ConnList!=NULL)
							{
								pIPv6ConnList->direction=toWANIntf;
								pIPv6ConnList->wanType=L34_NH_ETHER;
								pPktHdr->pIPv6StatefulList=pIPv6ConnList;
								pPktHdr->shortcutStatus=RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND;
								pPktHdr->pIPv6StatefulList->idleSecs=0;
							}
						}
						
						ret = _rtk_rg_fwdEngine_ipv6PacketModify(toWANIntf,L34_NH_ETHER,pPktHdr,skb);
						if(ret!=RG_FWDENGINE_RET_CONTINUE)
							return ret; 	//RG_FWDENGINE_RET_TO_PS
		
						return RG_FWDENGINE_RET_DIRECT_TX;
					}
					default:
						//we should not get here
						assert_ok(0);
						break;
		
				}
				
			}

	}else{//routing
#endif //End of : #ifdef CONFIG_RG_IPV6_NAPT_SUPPORT //Seperate for pPktHdr->fwdDecision is NAPT/NAPTR case

			pPktHdr->dipL3Idx=_rtk_rg_v6L3lookup(pPktHdr->pIpv6Dip);
			if(pPktHdr->dipL3Idx>=0)
			{
				//Check gwteway IP
				if(memcmp(pPktHdr->pIpv6Dip,rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.ipv6Addr.ipv6_addr,IPV6_ADDR_LEN)==0)
					gatewayIP=pPktHdr->dipL3Idx;
			}
			
			//either packet-to-gateway or neighbor advertisement
			if(gatewayIP != -1 || ((rg_db.pktHdr->tagif&ICMPV6_TAGIF)&&(pPktHdr->ICMPv6Type==0x88)))	
			{
				DEBUG("gatewayIP idx=%d",gatewayIP);
				_rtk_rg_neighborAgent(skb,pPktHdr);
				TRACE("IPv6: either packet-to-gateway or neighbor advertisement, to PS!");
				return RG_FWDENGINE_RET_TO_PS;
			}
			
			if (pPktHdr->dipL3Idx == -1)
			{
				//no entry matched, Trap
				TRACE("IPv6: routing table lookup failed, to PS!");
				return RG_FWDENGINE_RET_TO_PS;
			} 
			else
			{
				pPktHdr->fwdDecision=RG_FWD_DECISION_V6ROUTING;
				//20150731LUKE:if we are routing with link-local address, drop it!
				if(pPktHdr->pIpv6Sip[0]==0xfe && pPktHdr->pIpv6Sip[1]==0x80){
					TRACE("IPv6 Routing with Link local address, Trap!");
					return RG_FWDENGINE_RET_TO_PS;
				}
				//hit! check process column
				switch (rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.type)
				{
					case L34_IPV6_ROUTE_TYPE_DROP:
						//sprintf(msg,"After bindForward, Drop!");
						TRACE("IPv6: Drop by routing table!");
						return RG_FWDENGINE_RET_DROP;
					case L34_IPV6_ROUTE_TYPE_TRAP:
						//sprintf(msg,"After bindForward, trap to CPU!");
						TRACE("IPv6: Trap to PS by routing table!");
						return RG_FWDENGINE_RET_TO_PS;
					case L34_IPV6_ROUTE_TYPE_GLOBAL:
						/* Read NextHop */
						TRACE("IPv6: Global routing..%s",rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.rt2waninf?"rt2WAN":"rt2LAN");
						//since the entry should be the one we matched when we left the loop,
						//there is no need to get it out once again!
						entry=&rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route;		
						/* Read NextHop (Routing) */
						pPktHdr->nexthopIdx=entry->nhOrIfidIdx;
						nexthopEntry=&rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop;
						pPktHdr->dmacL2Idx=nexthopEntry->nhIdx;
						pPktHdr->netifIdx=nexthopEntry->ifIdx;

						//20150310LUKE: Check MTU, if over, just return to protocol stack since we can't split here
						if(rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu < pPktHdr->l3Len)
						{
							pPktHdr->overMTU=1;
							TRACE("v6Packet L3 size(%d) is bigger than interface[%d]'s MTU(%d)",pPktHdr->l3Len,pPktHdr->netifIdx,rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu);
							return RG_FWDENGINE_RET_TO_PS;
						}
						if(entry->rt2waninf)
						{
							//20140814LUKE: binding only activated when rt2WAN!!
							if(rg_db.systemGlobal.initParam.macBasedTagDecision)
							{
								ret=_rtk_rg_ipv6BindingDecision(pPktHdr);
								if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;
		
								if(pPktHdr->bindNextHopIdx!=FAIL)
								{
									/* Rome driver should make sure that binding->nexthop is equal to napt->nexthop. */
									pPktHdr->nexthopIdx=pPktHdr->bindNextHopIdx; //for Port/VLAN Bidning Route SMAC/DMAC
									nexthopEntry=&rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop;
#if defined(CONFIG_RTL9602C_SERIES)
#else
									/* Special case: for binding but default route, DMAC decision should be from BD->WT->NH->L2 */
									if(pPktHdr->dipL3Idx==V6_DEFAULT_ROUTE_IDX)
#endif
										pPktHdr->dmacL2Idx=rg_db.nexthop[pPktHdr->nexthopIdx].rtk_nexthop.nhIdx; //for Port/VLAN Bidning Route DMAC
									TRACE("Binding to interface pPktHdr->netifIdx = %d",pPktHdr->netifIdx);
								}
							}
							
							toWANIntf=IPV6_ROUTE_OUTBOUND;					
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
							pIPv6ConnList=NULL;
							if(pPktHdr->tagif&V6FRAG_TAGIF)
							{
								return _rtk_rg_fwdEngine_v6FragmentHandling(toWANIntf,nexthopEntry->type,&pIPv6ConnList,pPktHdr,skb);
							}
							else	//unfragment
							{
								//ipv6 stateful connection tracking, if outbound first time, fill software data structure
								//if inbound without outbound, drop it.
								ret=RG_FWDENGINE_RET_NAPT_OK;
								if(pPktHdr->tagif&TCP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6TCPOutboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								else if(pPktHdr->tagif&UDP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6UDPOutboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;
							}
		
							//update direction and wanType(ether or pppoe)
							if(pIPv6ConnList!=NULL)
							{
								pIPv6ConnList->direction=toWANIntf;
								pIPv6ConnList->wanType=nexthopEntry->type;
								pPktHdr->pIPv6StatefulList=pIPv6ConnList;
								pPktHdr->shortcutStatus=RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND;
								pPktHdr->pIPv6StatefulList->idleSecs=0;
							}
#endif
						}
						else
						{
							toWANIntf=IPV6_ROUTE_INBOUND;					
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
							pIPv6ConnList=NULL;
							if(pPktHdr->tagif&V6FRAG_TAGIF)
							{
								return _rtk_rg_fwdEngine_v6FragmentHandling(toWANIntf,nexthopEntry->type,&pIPv6ConnList,pPktHdr,skb);
							}
							else	//unfragment
							{
								//ipv6 stateful connection tracking, if outbound first time, fill software data structure
								//if inbound without outbound, drop it.
								ret=RG_FWDENGINE_RET_NAPT_OK;
								if(pPktHdr->tagif&TCP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6TCPInboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								else if(pPktHdr->tagif&UDP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6UDPInboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;
							}
		
							//update direction and wanType(ether or pppoe)
							if(pIPv6ConnList!=NULL)
							{
								pIPv6ConnList->direction=toWANIntf;
								pIPv6ConnList->wanType=nexthopEntry->type;
								pPktHdr->pIPv6StatefulList=pIPv6ConnList;
								pPktHdr->shortcutStatus=RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND;
								pPktHdr->pIPv6StatefulList->idleSecs=0;
							}
#endif
						}
						
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
						pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;
#endif
						ret = _rtk_rg_fwdEngine_ipv6PacketModify(toWANIntf,nexthopEntry->type,pPktHdr,skb);
						if(ret!=RG_FWDENGINE_RET_CONTINUE)
							return ret; 	//RG_FWDENGINE_RET_TO_PS				
						
						return RG_FWDENGINE_RET_DIRECT_TX;
					case L34_IPV6_ROUTE_TYPE_LOCAL:
						TRACE("IPv6: Local routing..%s",rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route.rt2waninf?"rt2WAN":"rt2LAN");
						//lookup for neighbor table
						res = 0;
						nb_hash_idx = _rtk_rg_IPv6NeighborHash(pPktHdr->pIpv6Dip+8, (uint8)pPktHdr->dipL3Idx);
						
						for(i=0;i<8;i++)
						{
							matchNeighbor = (nb_hash_idx<<3)+i;
							//TRACE("the matchNeighbor idx = %d\n",matchNeighbor);
							//ASSERT_EQ(dal_apollomp_l34_ipv6NeighborTable_get(matchNeighbor, &neighbor), RT_ERR_OK);
							neighbor = &rg_db.v6neighbor[matchNeighbor].rtk_v6neighbor;
							//TRACE("the neighbor.ipv6RouteIdx = %d, matchEntry = %d\n",neighbor->ipv6RouteIdx,matchEntry);
							if(neighbor->valid && 
								(neighbor->ipv6RouteIdx == pPktHdr->dipL3Idx)&&
								(_rtk_rg_CompareIFID(pPktHdr->pIpv6Dip+8, neighbor->ipv6Ifid)))
							{
								//rtlglue_printf("HIT!!!!!\n");
								TRACE("IPv6: Hit neighbor table!L2IDX = %d rt=%d",neighbor->l2Idx,pPktHdr->dipL3Idx);
								res = 1;
								break;
							}
						}
						
						if (res == 0)
						{
							//if not hit, trap to cpu
							TRACE("IPv6: Neighbor table un-hit, DROP!");
							_rtk_rg_fwdengine_handleNeighborMiss(pPktHdr);
							return RG_FWDENGINE_RET_DROP;
						}
						
						entry=&rg_db.v6route[pPktHdr->dipL3Idx].rtk_v6route;		
		
						pPktHdr->dmacL2Idx=neighbor->l2Idx;
						pPktHdr->netifIdx=entry->nhOrIfidIdx&0x7;
						
						//20150310LUKE: Check MTU, if over, just return to protocol stack since we can't split here
						if(rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu < pPktHdr->l3Len)
						{
							pPktHdr->overMTU=1;
							TRACE("v6Packet L3 size(%d) is bigger than interface[%d]'s MTU(%d)",pPktHdr->l3Len,pPktHdr->netifIdx,rg_db.netif[pPktHdr->netifIdx].rtk_netif.mtu);
							return RG_FWDENGINE_RET_TO_PS;
						}
		
						if(entry->rt2waninf==1)
						{
							//20140814LUKE: binding only activated when rt2WAN!!
							if(rg_db.systemGlobal.initParam.macBasedTagDecision)
							{
								ret=_rtk_rg_ipv6BindingDecision(pPktHdr);
								if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;
		
								if(pPktHdr->bindNextHopIdx!=FAIL)
								{
									/* Rome driver should make sure that binding->nexthop is equal to napt->nexthop. */
									pPktHdr->nexthopIdx=pPktHdr->bindNextHopIdx; //for Port/VLAN Bidning Route SMAC/DMAC
									
									TRACE("Binding to interface pPktHdr->netifIdx = %d",pPktHdr->netifIdx);
								}
							}
							
							toWANIntf=IPV6_ROUTE_OUTBOUND;
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
							pIPv6ConnList=NULL;
							if(pPktHdr->tagif&V6FRAG_TAGIF)
							{
								return _rtk_rg_fwdEngine_v6FragmentHandling(toWANIntf,L34_NH_ETHER,&pIPv6ConnList,pPktHdr,skb);
							}
							else	//unfragment
							{
								//ipv6 stateful connection tracking, if outbound first time, fill software data structure
								//if inbound without outbound, drop it.
								ret=RG_FWDENGINE_RET_NAPT_OK;
								if(pPktHdr->tagif&TCP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6TCPOutboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								else if(pPktHdr->tagif&UDP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6UDPOutboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;
							}
		
							//update direction and wanType(ether or pppoe)
							if(pIPv6ConnList!=NULL)
							{
								pIPv6ConnList->direction=toWANIntf;
								pIPv6ConnList->wanType=L34_NH_ETHER;
								pPktHdr->pIPv6StatefulList=pIPv6ConnList;
								pPktHdr->shortcutStatus=RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND;
								pPktHdr->pIPv6StatefulList->idleSecs=0;
							}
#endif
						}
						else
						{
							toWANIntf=IPV6_ROUTE_INBOUND;
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
							pIPv6ConnList=NULL;
							if(pPktHdr->tagif&V6FRAG_TAGIF)
							{
								return _rtk_rg_fwdEngine_v6FragmentHandling(toWANIntf,L34_NH_ETHER,&pIPv6ConnList,pPktHdr,skb);
							}
							else	//unfragment
							{
								//ipv6 stateful connection tracking, if outbound first time, fill software data structure
								//if inbound without outbound, drop it.
								ret=RG_FWDENGINE_RET_NAPT_OK;
								if(pPktHdr->tagif&TCP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6TCPInboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								else if(pPktHdr->tagif&UDP_TAGIF)
									ret=_rtk_rg_fwdEngine_ipv6UDPInboundConnectionTracking(&pIPv6ConnList,pPktHdr);
								if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;
							}
		
							//update direction and wanType(ether or pppoe)
							if(pIPv6ConnList!=NULL)
							{
								pIPv6ConnList->direction=toWANIntf;
								pIPv6ConnList->wanType=L34_NH_ETHER;
								pPktHdr->pIPv6StatefulList=pIPv6ConnList;
								pPktHdr->shortcutStatus=RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND;
								pPktHdr->pIPv6StatefulList->idleSecs=0;
							}
#endif
						}
						
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
						pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;
#endif
						ret = _rtk_rg_fwdEngine_ipv6PacketModify(toWANIntf,L34_NH_ETHER,pPktHdr,skb);
						if(ret!=RG_FWDENGINE_RET_CONTINUE)
							return ret; 	//RG_FWDENGINE_RET_TO_PS
		
						return RG_FWDENGINE_RET_DIRECT_TX;
					default:
						//we should not get here
						assert_ok(0);
						break;
		
				}
				
			}
#ifdef CONFIG_RG_IPV6_NAPT_SUPPORT //Seperate for pPktHdr->fwdDecision is NAPT/NAPTR case
	}//End of else{//routing
#endif	
	return RG_FWDENGINE_RET_TO_PS;
}

rtk_rg_fwdEngineReturn_t _rtk_rg_layer34Forward(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
//	int isL34=0;
	//int sipArpIdx;
	rtk_rg_ipClassification_t sipClass,dipClass;
	rtk_rg_sipDipClassification_t sipDipClass;
	//u8 *pData=skb->data;
	//u32 len=skb->len;
	rtk_rg_fwdEngineReturn_t ret=RG_FWDENGINE_RET_DIRECT_TX;
	rtk_rg_lookupIdxReturn_t naptOutIdx=0,naptInIdx=0;
	int fragIdx=FAIL;
	uint16 ori_srcPort,ori_dstPort;
	uint32 ori_srcIP,ori_dstIP,ori_seq,ori_ack;
	uint16 checksumBefore;
	rtk_rg_table_icmp_flow_t *icmpCtrlFlow=NULL;
	rtk_rg_ipv4_fragment_out_t *pFragList=NULL;
	rtk_rg_ipv4_fragment_in_t *pFragInList=NULL;
	rtk_rg_isakmp_t * pIsakmp = NULL; /* siyuan add for alg IPsec passthrough */
		
	//DEBUG("L34 Input");

	if(pPktHdr->tagif&IPV6_TAGIF)
		return _rtk_rg_ipv6L34Forward(skb,pPktHdr);
	
	TRACE("IPv4 L34 Forward");

	if((pPktHdr->pIpv4TTL==NULL)||(pPktHdr->pIpv4TTL!=NULL && *pPktHdr->pIpv4TTL<=1)) 
	{
		TRACE("TTL fail, to PS!");
		return RG_FWDENGINE_RET_TO_PS;
	}

	//pPktHdr->sipL3Idx=0;
	sipClass=_rtk_rg_sip_classification(pPktHdr->ipv4Sip,pPktHdr);
	dipClass=_rtk_rg_dip_classification(pPktHdr->ipv4Dip,pPktHdr);
	sipDipClass=rg_db.systemGlobal.sipDipClass[sipClass][dipClass];

	//DEBUG("sipClass=%d,dipClass=%d,sipDipClass=%d\n",sipClass,dipClass,sipDipClass);

	if(pPktHdr->tagif&IPV4_TAGIF)
	{
		//dump_packet(skb->data,skb->len,"learn arp");
		//DEBUG("L34 learning ARP...");
		_rtk_rg_arpAndMacEntryAdd(pPktHdr->ipv4Sip,pPktHdr->sipL3Idx,pPktHdr->pSmac,pPktHdr->ingressPort,pPktHdr->wlan_dev_idx,NULL,pPktHdr->internalVlanID,0,0);
	}

	
	
	if((sipClass==IP_CLASS_NPI)&&(dipClass==IP_CLASS_NPE))
	{
		// 20141129: When 2nd WAN is routing WAN. RP-->NPE will become NPI-->NPE. so we do this patch.
		if((1<<pPktHdr->ingressPort)&rg_db.systemGlobal.wanPortMask.portmask)
		{
			sipDipClass=SIP_DIP_CLASS_NAPTR;
		}
		else if((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF)) // 20150420: support Hairpin NAT
		{
			sipDipClass=SIP_DIP_CLASS_HAIRPIN_NAT;
			pPktHdr->isHairpinNat=1;
		}
	}

	TRACE("sipClass[%d] dipClass[%d] sipDipClass[%d]",sipClass,dipClass,sipDipClass);

	//do binding check first!!for unmatch L34L3 or L3L34
	if((sipDipClass==SIP_DIP_CLASS_NAPT) || (sipDipClass==SIP_DIP_CLASS_ROUTING))
	{
		ret=_rtk_rg_routingDecisionTablesLookup(pPktHdr,&sipDipClass);
		if(ret!=RG_FWDENGINE_RET_CONTINUE) return ret;
	}

	if(sipDipClass==SIP_DIP_CLASS_HAIRPIN_NAT)
	{
		
		pPktHdr->extipIdx = _rtk_rg_eiplookup(pPktHdr->ipv4Dip);
		assert(pPktHdr->extipIdx!=-1); 
		pPktHdr->netifIdx = rg_db.nexthop[rg_db.extip[pPktHdr->extipIdx].rtk_extip.nhIdx].rtk_nexthop.ifIdx;

		//assign gateway mac to dmacL2Idx
		pPktHdr->dmacL2Idx = rg_db.netif[pPktHdr->netifIdx].l2_idx;
	}

	if((sipDipClass==SIP_DIP_CLASS_NAPT)||(sipDipClass==SIP_DIP_CLASS_HAIRPIN_NAT))
	{
		pPktHdr->fwdDecision=RG_FWD_DECISION_NAPT;
		
		//Check ALG
		ret=_rtk_rg_algFunctionCheck(NAPT_DIRECTION_OUTBOUND,pPktHdr);
		//DEBUG("the algFunction ret=%d",ret);
		if(ret==RG_FWDENGINE_RET_TO_PS)
		{
			TRACE("hit ALG and registered function is NULL, to PS!");
			return ret;	//hit ALG and registered function is NULL
		}
	
		//DEBUG("skb is %p, ppkthdr is %p, l3offset is %d, l3len is %d",skb,pPktHdr,pPktHdr->l3Offset,pPktHdr->l3Len);
		//DEBUG("before mf and fragoffset check:mf=%d,fragoffset=%d",pPktHdr->ipv4MoreFragment,pPktHdr->ipv4FragmentOffset);

#ifdef __KERNEL__ //model skip alg
		if(pPktHdr->tagif&GRE_TAGIF)
		{
			//check PPTP flow for modify GRE packets, only inbound need to change internal IP according to external CallID
			//if the flow is not found, trap to PS
			return _rtk_rg_PPTP_GREModify(NAPT_DIRECTION_OUTBOUND,skb,pPktHdr);
		}
#endif

		if(pPktHdr->ipv4FragPacket==0) //normal packet (non-fragment)
		{
#ifdef CONFIG_RG_NAPT_TCP_AUTO_LEARN
			if(pPktHdr->tagif&TCP_TAGIF)
			{
				naptOutIdx=_rtk_rg_naptTcpUdpOutHashIndexLookupByPktHdr(1,pPktHdr);
				ret = _rtk_rg_fwdEngine_TCPOutboundConnectionTracking(pPktHdr,&naptOutIdx);
				if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;

				assert(naptOutIdx>=0);

				/*record result for internalPri decision*/
				pPktHdr->l4Direction = RG_NAPT_OUTBOUND_FLOW;
				pPktHdr->naptOutboundIndx = naptOutIdx;
				pPktHdr->naptrInboundIndx = rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;

				/* Packet forwarded by Forwarding Engine, reset idle time */
				rg_db.naptOut[naptOutIdx].idleSecs=0;

//					dump_packet(skb->data,skb->len,"org");
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,naptOutIdx, pPktHdr,skb,1,1);
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
				//assert_ok(ret);
				//dump_packet(skb->data,skb->len,"new");
				if(sipDipClass!=SIP_DIP_CLASS_HAIRPIN_NAT)
					return RG_FWDENGINE_RET_DIRECT_TX;				
			}
#endif

			//UDP auto-learning
			if(pPktHdr->tagif&UDP_TAGIF)
			{
				ret = _rtk_rg_fwdEngine_UDPOutboundConnectionTracking(pPktHdr, &naptOutIdx);
				//DEBUG("ret from _rtk_rg_fwdEngine_UDPOutboundConnectionTracking is %d",ret);
				if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;

				assert(naptOutIdx>=0);
				
				/*record result for internalPri decision*/
				pPktHdr->l4Direction = RG_NAPT_OUTBOUND_FLOW;
				pPktHdr->naptOutboundIndx = naptOutIdx;
				pPktHdr->naptrInboundIndx = rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;

				/* Packet forwarded by Forwarding Engine, reset idle time */
				rg_db.naptOut[naptOutIdx].idleSecs=0;

				//dump_packet(pData,len,"org");
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,naptOutIdx,pPktHdr,skb,1,1);
				//DEBUG("ret from _rtk_rg_fwdEngine_naptPacketModify is %d",ret);
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
				//assert_ok(ret);
				//dump_packet(skb->data,skb->len,"new");
				if(sipDipClass!=SIP_DIP_CLASS_HAIRPIN_NAT)
					return RG_FWDENGINE_RET_DIRECT_TX;		
			}

			//ICMP forwarding
			if(pPktHdr->tagif&ICMP_TAGIF)
			{
				//DEBUG("before add ICMP control flow!! pPktHdr->overMTU is %d",pPktHdr->overMTU);
				_rtk_rg_fwdEngine_ICMPOutboundControlFlowTracking(pPktHdr,&icmpCtrlFlow);

				//Check MTU
				if(pPktHdr->overMTU)
				{
					TRACE("Normal Packet ICMP Over MTU");
					//return RG_FWDENGINE_RET_TO_PS;
				}
				
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,FAIL,pPktHdr,skb,1,0);
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
				//assert_ok(ret);
				//dump_packet(skb->data,skb->len,"new"); 
				if(sipDipClass!=SIP_DIP_CLASS_HAIRPIN_NAT)
					return RG_FWDENGINE_RET_DIRECT_TX;		
			}


#ifdef __KERNEL__ //model skip alg
			/*siyuan add for alg IPsec passthrough*/
			if(pPktHdr->tagif & ESP_TAGIF)
			{
				ret = rtk_rg_alg_ESP(NAPT_DIRECTION_OUTBOUND, (unsigned char *)pPktHdr, (unsigned char *)skb, pIsakmp);
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
				
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,FAIL,pPktHdr,skb,1,0);
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
					
				return RG_FWDENGINE_RET_DIRECT_TX;		
			}
#endif
	
		}

		//fwdEngine only support TCP,UDP,ICMP fragment now
		//first packet which has L4 header of these fragments
		else if(pPktHdr->ipv4MoreFragment && pPktHdr->ipv4FragmentOffset==0)
		{
			//Add software NAPT entry(TCP or UDP)
#ifdef CONFIG_RG_NAPT_TCP_AUTO_LEARN
			if(pPktHdr->tagif&TCP_TAGIF)
			{
				//Keep original information for L4 checksum recaculate
				ori_srcIP = ntohl(*pPktHdr->pIpv4Sip);
				ori_srcPort = ntohs(*pPktHdr->pSport);
				ori_seq = ntohl(*pPktHdr->pTcpSeq);
				ori_ack = ntohl(*pPktHdr->pTcpAck);

				naptOutIdx=_rtk_rg_naptTcpUdpOutHashIndexLookupByPktHdr(1,pPktHdr);
				ret = _rtk_rg_fwdEngine_TCPOutboundConnectionTracking(pPktHdr,&naptOutIdx);

				assert(naptOutIdx>=0);
				
				/*record result for internalPri decision*/
				pPktHdr->l4Direction = RG_NAPT_OUTBOUND_FLOW;
				pPktHdr->naptOutboundIndx = naptOutIdx;
				pPktHdr->naptrInboundIndx = rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;

				//before add to fragment table, lookup for created frag list
				pFragList = NULL;
				_rtk_rg_fwdEngine_fragmentOutHashIndexLookup(&fragIdx,&pFragList,pPktHdr->ipProtocol,pPktHdr->ipv4Sip, pPktHdr->ipv4Dip, ntohs(*pPktHdr->pIpv4Identification),pPktHdr->l3Len-pPktHdr->ipv4HeaderLen);
				if(pFragList!=NULL)
				{
					//DEBUG("totalLength is %d",pFragList->receivedLength);
					if(pFragList->fragAction==RG_FWDENGINE_RET_DROP || pFragList->fragAction==RG_FWDENGINE_RET_TO_PS){
						TRACE("pFragList->fragAction==RG_FWDENGINE_RET_TO_PS or RG_FWDENGINE_RET_DROP");
						return pFragList->fragAction;
					}
					else{
						//DEBUG("set pFragList->fragAction to %d",ret);
						pFragList->fragAction=ret;	//forward, drop, or to_ps
					}
				}
				else
					_rtk_rg_fwdEngine_fillOutFragmentInfo(ret,naptOutIdx,pPktHdr,&pFragList);

				//Check total fragment length
				if(pFragList->totalLength>0 && pFragList->receivedLength>=pFragList->totalLength)
				{
					DEBUG("[outBound]fragments(%d) are all received(%d)!! free the list %p in [%d]...",pFragList->totalLength,pFragList->receivedLength,pFragList,fragIdx);
					_rtk_rg_freeFragOutList(fragIdx,pFragList);
				}

				if(ret==RG_FWDENGINE_RET_DROP)
				{					
					TRACE("L4 lookup fail, DROP");
					return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
				}
				else if(ret==RG_FWDENGINE_RET_TO_PS)
				{
					TRACE("L4 lookup miss, to PS");				
					return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue
				}
				
				//dump_packet(pData,len,"org");
				//DEBUG("the shortcut state is %d!!",pPktHdr->shortcutStatus);
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,naptOutIdx, pPktHdr,skb,1,1);
				pFragList->fragAction=ret;
				if(ret==RG_FWDENGINE_RET_DROP)
					return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
				else if(ret==RG_FWDENGINE_RET_TO_PS)
					return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue
				
				//Recaculate L4 checksum, let HW do L3 checksum
				checksumBefore=*pPktHdr->pL4Checksum;
				*pPktHdr->pL4Checksum = htons(_rtk_rg_fwdengine_L4checksumUpdate(pPktHdr->tcpFlags.ack,*pPktHdr->pL4Checksum,ori_srcIP,ori_srcPort,ori_seq,ori_ack,ntohl(*pPktHdr->pIpv4Sip),ntohs(*pPktHdr->pSport),ntohl(*pPktHdr->pTcpSeq),ntohl(*pPktHdr->pTcpAck)));
				TRACE("First Fragment TCP checksum calculate by software ID=0x%x checksum=0x%04x==>0x%04x!",*pPktHdr->pIpv4Identification,checksumBefore,*pPktHdr->pL4Checksum );
				//dump_packet(pData,len,"new");
				return RG_FWDENGINE_RET_FRAGMENT_ONE;
							
			}
#endif
			//UDP auto-learning
			if(pPktHdr->tagif&UDP_TAGIF)
			{		
				//Keep original information for L4 checksum recaculate
				ori_srcIP = ntohl(*pPktHdr->pIpv4Sip);
				ori_srcPort = ntohs(*pPktHdr->pSport);
				
				ret = _rtk_rg_fwdEngine_UDPOutboundConnectionTracking(pPktHdr, &naptOutIdx);
				assert(naptOutIdx>=0);

				/*record result for internalPri decision*/
				pPktHdr->l4Direction = RG_NAPT_OUTBOUND_FLOW;
				pPktHdr->naptOutboundIndx = naptOutIdx;
				pPktHdr->naptrInboundIndx = rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;

				//before add to fragment table, lookup for created frag list
				pFragList = NULL;
				_rtk_rg_fwdEngine_fragmentOutHashIndexLookup(&fragIdx,&pFragList,pPktHdr->ipProtocol,pPktHdr->ipv4Sip, pPktHdr->ipv4Dip, ntohs(*pPktHdr->pIpv4Identification), pPktHdr->l3Len-pPktHdr->ipv4HeaderLen);
				if(pFragList!=NULL)
				{
					//DEBUG("totalLength is %d",pFragList->totalLength);
					if(pFragList->fragAction==RG_FWDENGINE_RET_DROP || pFragList->fragAction==RG_FWDENGINE_RET_TO_PS)
					{
						TRACE("%s!",(pFragList->fragAction==RG_FWDENGINE_RET_DROP)?"Drop":"ToPS");
						return pFragList->fragAction;
					}
					else
					{
						//DEBUG("set pFragList->fragAction to %d",ret);
						pFragList->fragAction=ret;	//forward, drop, or to_ps
					}
				}
				else
					_rtk_rg_fwdEngine_fillOutFragmentInfo(ret,naptOutIdx,pPktHdr,&pFragList);

				//Check total fragment length
				if(pFragList->totalLength>0 && pFragList->receivedLength>=pFragList->totalLength)
				{
					DEBUG("[outBound]fragments(%d) are all received(%d)!! free the list %p in [%d]...",pFragList->totalLength,pFragList->receivedLength,pFragList,fragIdx);
					_rtk_rg_freeFragOutList(fragIdx,pFragList);
				}

				if(ret==RG_FWDENGINE_RET_DROP)
					return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
				else if(ret==RG_FWDENGINE_RET_TO_PS)
					return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue
				
				//dump_packet(pData,len,"org");
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,naptOutIdx,pPktHdr,skb,1,1);
				pFragList->fragAction=ret;
				if(ret==RG_FWDENGINE_RET_DROP)
					return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
				else if(ret==RG_FWDENGINE_RET_TO_PS)
					return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue

				//Recaculate L4 checksum, let HW do L3 checksum
				//DEBUG("the original checksum is %x",*pPktHdr->pL4Checksum);
				checksumBefore=*pPktHdr->pL4Checksum;
				*pPktHdr->pL4Checksum = htons(_rtk_rg_fwdengine_L4checksumUpdate(0,*pPktHdr->pL4Checksum,ori_srcIP,ori_srcPort,0,0,ntohl(*pPktHdr->pIpv4Sip),ntohs(*pPktHdr->pSport),0,0));
				TRACE("First Fragment UDP checksum calculate by software ID=0x%x checksum=0x%04x==>0x%04x!",*pPktHdr->pIpv4Identification,checksumBefore,*pPktHdr->pL4Checksum );
				//DEBUG("the new checksum is %x",*pPktHdr->pL4Checksum);
				//DEBUG("UDP first packet modify for fragment...");

				//dump_packet(pData,len,"new");
				return RG_FWDENGINE_RET_FRAGMENT_ONE;
			}		

			//ICMP forwarding
			if(pPktHdr->tagif&ICMP_TAGIF)
			{
				//Check MTU
				if(pPktHdr->overMTU)
				{
					TRACE("First Fragment Packet ICMP Over MTU");
					//return RG_FWDENGINE_RET_FRAG_ONE_PS;
				}
				
				//DEBUG("before add ICMP control flow!!(first fragment packet)");

				//before add to fragment table, lookup for created frag list
				pFragList = NULL;
				_rtk_rg_fwdEngine_fragmentOutHashIndexLookup(&fragIdx,&pFragList,pPktHdr->ipProtocol,pPktHdr->ipv4Sip, pPktHdr->ipv4Dip, ntohs(*pPktHdr->pIpv4Identification), pPktHdr->l3Len-pPktHdr->ipv4HeaderLen);
				if(pFragList!=NULL)
				{
					//DEBUG("totalLength is %d",pFragList->totalLength);
					if(pFragList->fragAction==RG_FWDENGINE_RET_DROP || pFragList->fragAction==RG_FWDENGINE_RET_TO_PS)
						return pFragList->fragAction;
					else
						pFragList->fragAction=RG_FWDENGINE_RET_NAPT_OK;	//forward, drop, or to_ps
				}
				else
				{
					ret = _rtk_rg_fwdEngine_ICMPOutboundControlFlowTracking(pPktHdr,&icmpCtrlFlow);
					_rtk_rg_fwdEngine_fillOutFragmentInfo(ret,FAIL,pPktHdr,&pFragList);
				}

				//Check total fragment length
				if(pFragList->totalLength>0 && pFragList->receivedLength>=pFragList->totalLength)
				{
					DEBUG("[outBound]fragments(%d) are all received(%d)!! free the list %p in [%d]...",pFragList->totalLength,pFragList->receivedLength,pFragList,fragIdx);
					_rtk_rg_freeFragOutList(fragIdx,pFragList);
				}

				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,FAIL,pPktHdr,skb,1,0);
				pFragList->fragAction=ret;
				if(ret==RG_FWDENGINE_RET_DROP)
					return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
				else if(ret==RG_FWDENGINE_RET_TO_PS)
					return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue
				
				//dump_packet(skb->data,skb->len,"ICMP first fragment"); 	
				return RG_FWDENGINE_RET_FRAGMENT_ONE;		
			}
		}
		else if(pPktHdr->ipv4FragmentOffset>0)	//other fragments 
		{
			//DEBUG("i am non-first fragment packets...");
			//Check if we had already add software NAPT entry for the fragment packets
			naptOutIdx = FAIL;
			//icmpCtrlFlow = NULL;
			pFragList = NULL;
			//if(pPktHdr->tagif&TCP_TAGIF || pPktHdr->tagif&UDP_TAGIF)
			naptOutIdx = _rtk_rg_fwdEngine_fragmentOutHashIndexLookup(&fragIdx,&pFragList,pPktHdr->ipProtocol,pPktHdr->ipv4Sip, pPktHdr->ipv4Dip, ntohs(*pPktHdr->pIpv4Identification), pPktHdr->l3Len-pPktHdr->ipv4HeaderLen);

			/*record result for internalPri decision*/
			pPktHdr->l4Direction = RG_NAPT_OUTBOUND_FLOW;
			pPktHdr->naptOutboundIndx = naptOutIdx;
			pPktHdr->naptrInboundIndx = rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;

			/*else if(pPktHdr->tagif&ICMP_TAGIF)
			{
				//if IPID is match, direct forward as L3 packet!!
				
				icmpCtrlFlow = _rtk_rg_fwdEngine_ICMPOutboundFragmentLookup(pPktHdr);
				if(icmpCtrlFlow!=NULL)
				{
					//Check MTU
					if(pPktHdr->ICMPOverMTU)return RG_FWDENGINE_RET_TO_PS;
				
					ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,0,pPktHdr,skb,1,0);
					if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
					//assert_ok(ret);
					//dump_packet(skb->data,skb->len,"ICMP outbound(frag)"); 		
					return RG_FWDENGINE_RET_FRAGMENT;
				}
				//DEBUG("icmpCtrlFlow is NULL in system......queuing");
			}*/

			if(pFragList!=NULL)		//filled by first packet or other early fragment packets
			{
				//DEBUG("later fragment modify... fragIdx=%d",fragIdx);
				if(pPktHdr->ipv4MoreFragment == 0 && fragIdx != FAIL)		//we can not free fragList here, unless there are all packets had forwarded
					pFragList->totalLength=(pPktHdr->ipv4FragmentOffset<<3)+pPktHdr->l3Len-pPktHdr->ipv4HeaderLen;
				//DEBUG("totalLength is %d",pFragList->totalLength);
				//Check total fragment length
				if(pFragList->totalLength>0 && pFragList->receivedLength>=pFragList->totalLength)
				{
					DEBUG("[outBound]fragments(%d) are all received(%d)!! free the list %p in [%d]...",pFragList->totalLength,pFragList->receivedLength,pFragList,fragIdx);
					_rtk_rg_freeFragOutList(fragIdx,pFragList);
				}
						
				//Check frag action from first packet
				if(pFragList->fragAction==RG_FWDENGINE_RET_TO_PS || pFragList->fragAction==RG_FWDENGINE_RET_DROP)
				{
					TRACE("%s!",(pFragList->fragAction==RG_FWDENGINE_RET_TO_PS)?"ToPS":"Drop");
					return pFragList->fragAction;
				}
				if(pFragList->fragAction==RG_FWDENGINE_RET_QUEUE_FRAG)	//first packet not come in yet
					goto OUTBOUND_FRAG_QUEUE;

				if(naptOutIdx!=FAIL)	//TCP or UDP
				{
					//Modify the SIP and forward it 
					ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,FAIL,pPktHdr,skb,1,0);
					if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
					//assert_ok(ret);
					
					return RG_FWDENGINE_RET_FRAGMENT;
				}
				else		//ICMP
				{
					//Check MTU
					if(pPktHdr->overMTU)
					{
						TRACE("Offset>0 Fragment Packet ICMP Over MTU");
						//return RG_FWDENGINE_RET_TO_PS;
					}
				
					//DEBUG("later fragment modify... fragIdx=%d",fragIdx);
					//if(pPktHdr->ipv4MoreFragment == 0 && fragIdx != FAIL)		//we can not free fragList here, if the last one is coming before other frags....
						//_rtk_rg_freeFragOutList(fragIdx,pFragList);
				
					ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_OUTBOUND,FAIL,pPktHdr,skb,1,0);
					if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
					//assert_ok(ret);
					//dump_packet(skb->data,skb->len,"ICMP outbound(frag)"); 		
					return RG_FWDENGINE_RET_FRAGMENT;
				}
			}
			else
			{	
				//Otherwise we need to queue this packet for later proceed
				_rtk_rg_fwdEngine_fillOutFragmentInfo(RG_FWDENGINE_RET_QUEUE_FRAG,FAIL,pPktHdr,&pFragList);
				
				//DEBUG("queuing packet.....");
OUTBOUND_FRAG_QUEUE:				

				//Check if we already queue same identification for MAX_FRAGMENT_QUEUE_THRESHOLD times
				pFragList->queueCount++;
				if(pFragList->queueCount>=MAX_FRAGMENT_QUEUE_THRESHOLD)
				{
					//clear same identification in queue
					pFragList->queueCount=0;
					pFragList->fragAction=RG_FWDENGINE_RET_DROP;
					_rtk_rg_fwdEngine_fragmentQueueProcessing(pFragList->fragAction,pPktHdr);
					TRACE("Drop!");
					return RG_FWDENGINE_RET_DROP;
				}
				else
				{
					//Put the fragment packet into queue
					_rtk_rg_fwdEngine_fragmentPacketQueuing(NAPT_DIRECTION_OUTBOUND,skb,pPktHdr);
					return RG_FWDENGINE_RET_QUEUE_FRAG;
				}
			}
		}
	}


	if(sipDipClass==SIP_DIP_CLASS_HAIRPIN_NAT)
	{
		TRACE("Hairpin NAPT is finished and Hairpin NAPTR will start.");
		_rtk_rg_fwdEngine_updateFlowStatus(skb,pPktHdr);		
		pPktHdr->ipv4Sip=*pPktHdr->pIpv4Sip;
		pPktHdr->sport=*pPktHdr->pSport;		
	}
	
	if((sipDipClass==SIP_DIP_CLASS_NAPTR)||(sipDipClass==SIP_DIP_CLASS_HAIRPIN_NAT))
	{	
		pPktHdr->fwdDecision=RG_FWD_DECISION_NAPTR;
		
		//Check ALG
		ret=_rtk_rg_algFunctionCheck(NAPT_DIRECTION_INBOUND,pPktHdr);
		//DEBUG("the algFunction ret=%d",ret);
		if(ret==RG_FWDENGINE_RET_TO_PS)return ret;	//hit ALG and registered function is NULL
	
		// From which WAN?
		pPktHdr->extipIdx = _rtk_rg_eiplookup(pPktHdr->ipv4Dip);
		assert(pPktHdr->extipIdx!=-1); //For NAPTR packet, external ip table lookup will not miss.
		pPktHdr->netifIdx = rg_db.nexthop[rg_db.extip[pPktHdr->extipIdx].rtk_extip.nhIdx].rtk_nexthop.ifIdx;

#ifdef CONFIG_RG_SIMPLE_PROTOCOL_STACK
		//Keep WAN interface index in CP structure
		/*for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
		{
			if(rg_db.systemGlobal.wanIntfGroup[i].index==pPktHdr->netifIdx)
			{
				cp->wanInterfaceIdx=i;
				break;
			}
		}*/
		if(pPktHdr->cp!=NULL)pPktHdr->cp->wanInterfaceIdx=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].lan_or_wan_index;		
		//DEBUG("this packet came from WAN index %d (eth%d)",cp->wanInterfaceIdx,cp->wanInterfaceIdx+1);
#endif


#ifdef __KERNEL__ //model skip alg
		if(pPktHdr->tagif&GRE_TAGIF)
		{
			//check PPTP flow for modify GRE packets, only inbound need to change internal IP according to external CallID
			//if the flow is not found, trap to PS
			return _rtk_rg_PPTP_GREModify(NAPT_DIRECTION_INBOUND,skb,pPktHdr);
		}
#endif		

		if(pPktHdr->ipv4FragPacket==0) //normal packet (non-fragment)
		{
#ifdef CONFIG_RG_NAPT_TCP_AUTO_LEARN
			if(pPktHdr->tagif&TCP_TAGIF)
			{		

				naptOutIdx=_rtk_rg_naptTcpUdpInHashIndexLookup(1,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->ipv4Dip,pPktHdr->dport);
				ret = _rtk_rg_fwdEngine_TCPInboundConnectionTracking(pPktHdr,&naptOutIdx);
				if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;

				assert(naptOutIdx>=0);
				if(naptOutIdx<0) 
				{
					return RG_FWDENGINE_RET_TO_PS;
				}				

				naptInIdx=rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;
				/*record result for internalPri decision*/
				pPktHdr->l4Direction = RG_NAPTR_INBOUND_FLOW;
				pPktHdr->naptOutboundIndx = naptOutIdx;	//value get in _rtk_rg_fwdEngine_TCPInboundConnectionTracking()			
				pPktHdr->naptrInboundIndx = naptInIdx;

				/* Packet forwarded by Forwarding Engine, reset idle time */
				rg_db.naptOut[naptOutIdx].idleSecs=0;

				//packet modify and directTX
				//dump_packet(skb->data,skb->len,"org");
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,naptInIdx,pPktHdr,skb,1,1);
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
				//assert_ok(ret);
				//dump_packet(skb->data,skb->len,"new");
				return RG_FWDENGINE_RET_DIRECT_TX;
			}
#endif
			if(pPktHdr->tagif&UDP_TAGIF)
			{
#ifdef CONFIG_RG_NAPT_INBOUND_TRACKING
				ret = _rtk_rg_fwdEngine_UDPInboundConnectionTracking(pPktHdr, &naptOutIdx);
				if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;
#else
				naptOutIdx=_rtk_rg_naptTcpUdpInHashIndexLookup(0,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->ipv4Dip,pPktHdr->dport);
#endif

				assert(naptOutIdx>=0);
				if(naptOutIdx<0) 
				{
					return RG_FWDENGINE_RET_TO_PS;
				}

				naptInIdx=rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;
				/*record result for internalPri decision*/
				pPktHdr->l4Direction = RG_NAPTR_INBOUND_FLOW;
				pPktHdr->naptrInboundIndx = naptInIdx;
				pPktHdr->naptOutboundIndx = naptOutIdx;//value get in _rtk_rg_fwdEngine_UDPInboundConnectionTracking()

				/* Packet forwarded by Forwarding Engine, reset idle time */
				rg_db.naptOut[naptOutIdx].idleSecs=0;
				
				//packet modify and directTX
				//dump_packet(pData,len,"org");
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,naptInIdx,pPktHdr,skb,1,1);
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
				//assert_ok(ret);
				//if(ret!=RT_ERR_RG_OK) return RG_FWDENGINE_RET_DROP;
				//dump_packet(pData,len,"new");
				return RG_FWDENGINE_RET_DIRECT_TX;
			}

			//ICMP link list lookup
			if(pPktHdr->tagif&ICMP_TAGIF) 
			{
				//if(pPktHdr->ICMPType==11) //TTL exceeded (for "trace route")
				if((pPktHdr->ICMPType==11)||(pPktHdr->ICMPType==3)) //Destination unreachable (for "trace route")
				{
					uint8 *ipHdr;
					uint8 *l4Hdr;
					uint8 protocol;
					uint32 sip;
					uint32 dip;
					uint32 newsip;
					uint16 sport;
					uint16 dport;
					//must NAPTR by ICMP payload
					ipHdr=skb->data+pPktHdr->l4Offset+8;
					//memDump(ipHdr,skb->len-pPktHdr->l4Offset-8,"icmp");
					if((ipHdr[0]&0xf0)!=0x40)
					{
						if(pPktHdr->ICMPType==11)
						{
							TRACE("Unkown ICMP TTL exceeded payload(1)!");
						}
						else
						{
							TRACE("Unkown ICMP Destination unreachable payload(1)!");
						}
					}
					else
					{
						protocol=ipHdr[9];
						if(!((protocol==0x6)||(protocol==0x11)||(protocol==0x1))) //TCP,UDP,ICMP
						{
							if(pPktHdr->ICMPType==11)
							{
								TRACE("Unkown ICMP TTL exceeded payload(2)!");
							}
							else
							{
								TRACE("Unkown ICMP Destination unreachable payload(2)!");
							}
						}
						else
						{
							int naptOutIdx,naptInIdx;
							sip=htonl(*(uint32 *)&ipHdr[12]);
							dip=htonl(*(uint32 *)&ipHdr[16]);
							l4Hdr=&ipHdr[(ipHdr[0]&0xf)<<2];
							sport=htons(*(uint16 *)&l4Hdr[0]);
							dport=htons(*(uint16 *)&l4Hdr[2]);
							if(pPktHdr->ICMPType==11)
							{
								TRACE("ICMP TTL exceeded(SIP=0x%x DIP=0x%x %s SPORT=%d DPORT=%d)",sip,dip,(protocol==0x6)?"TCP":"UDP",sport,dport);
							}
							else
							{
								TRACE("ICMP Destination unreachable(SIP=0x%x DIP=0x%x %s SPORT=%d DPORT=%d)",sip,dip,(protocol==0x6)?"TCP":"UDP",sport,dport);
							}
							naptOutIdx=_rtk_rg_naptTcpUdpInHashIndexLookup((protocol==0x6)?1:0,dip,dport,sip,sport);
							if(naptOutIdx>=0) //for linux trace route
							{
								rtk_rg_err_code_t rg_ret;
								naptInIdx = rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;
								*pPktHdr->pIpv4Dip=rg_db.naptIn[naptInIdx].rtk_naptIn.intIp;
								TRACE("Forward to LAN IP(0x%x), modify SIP of ICMP payload.",*pPktHdr->pIpv4Dip);
								//memDump(skb->data,skb->len,"data");

								newsip=*pPktHdr->pIpv4Dip;
								*(uint32*)(&ipHdr[12])=newsip;
								*(uint16*)(&ipHdr[10])=htons(_rtk_rg_fwdengine_L3checksumUpdate(*(uint16*)&ipHdr[10],sip,0,protocol,newsip,0));

								if(pPktHdr->ipv4FragPacket==0)
								{
									if(protocol==0x6)
										*(uint16*)(&l4Hdr[16]) = htons(_rtk_rg_fwdengine_L4checksumUpdate(0,*(uint16*)(&l4Hdr[16]),sip,0,0,0,newsip,0,0,0)); 		
									else if(protocol==0x11)
										*(uint16*)(&l4Hdr[6]) = htons(_rtk_rg_fwdengine_L4checksumUpdate(0,*(uint16*)(&l4Hdr[6]),sip,0,0,0,newsip,0,0,0));
								}

								rg_ret = _rtk_rg_fwdEngine_shortCutNaptPacketModify(NAPT_DIRECTION_INBOUND,FAIL,pPktHdr,skb,0,0);
								if(rg_ret!=RT_ERR_RG_OK) return RG_FWDENGINE_RET_DROP;
								return RG_FWDENGINE_RET_DIRECT_TX;	
							}
							else //for windows trace route
							{
								if((pPktHdr->ICMPType==11)&&(protocol==0x1))
								{
									rtk_rg_err_code_t rg_ret;
									
									//parsing inner tag for icmp lookup
									pPktHdr->ICMPIdentifier=*(uint16*)&l4Hdr[4];
									pPktHdr->ICMPSeqNum=*(uint16*)&l4Hdr[6];
									pPktHdr->ipv4Sip=*(uint32*)(&ipHdr[16]);
									icmpCtrlFlow = _rtk_rg_fwdEngine_ICMPInboundControlFlowTracking(pPktHdr);
				
									if(icmpCtrlFlow==NULL)
									{
										TRACE("ICMPR lookup fail...return to PS");
										return RG_FWDENGINE_RET_TO_PS;
									}

									*pPktHdr->pIpv4Dip=htonl(icmpCtrlFlow->internalIP);
									//pPktHdr->ipv4Dip=icmpCtrlFlow->internalIP;

									newsip=*pPktHdr->pIpv4Dip;
									*(uint32*)(&ipHdr[12])=newsip;
									*(uint16*)(&ipHdr[10])=htons(_rtk_rg_fwdengine_L3checksumUpdate(*(uint16*)&ipHdr[10],sip,0,protocol,newsip,0));
									
									//invalid this ctrl flow
									icmpCtrlFlow->valid=0;
									
									rg_ret = _rtk_rg_fwdEngine_shortCutNaptPacketModify(NAPT_DIRECTION_INBOUND,FAIL,pPktHdr,skb,0,0);
									if(rg_ret!=RT_ERR_RG_OK) return RG_FWDENGINE_RET_DROP;
									//assert_ok(ret);
									//dump_packet(skb->data,skb->len,"ICMP inbound");		
									pPktHdr->l3Modify=1;	//20141023LUKE: for wifi to check if recalculate chksum or not
									return RG_FWDENGINE_RET_DIRECT_TX;	

									
								}
							}
						}
					}		
				}

				//DEBUG("before ICMP flow lookup!");
				icmpCtrlFlow = _rtk_rg_fwdEngine_ICMPInboundControlFlowTracking(pPktHdr);
				
				if(icmpCtrlFlow==NULL)
				{
					TRACE("packet in ICMPR...before return to PS");
					//dump_packet(skb->data,skb->len,"return to PS");
					return RG_FWDENGINE_RET_TO_PS;
				}
			
				//1 FIXME:Here should check MTU size, too!!!
				
				//DEBUG("the internal IP from ICMP flow is %x",icmpCtrlFlow->internalIP);
				//Modify DIP
				*pPktHdr->pIpv4Dip=htonl(icmpCtrlFlow->internalIP);
				//pPktHdr->ipv4Dip=icmpCtrlFlow->internalIP;

				//invalid this ctrl flow
				icmpCtrlFlow->valid=0;

				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,FAIL,pPktHdr,skb,0,0); //DIP already be modified by this function. Don't need to modify again.
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
				//assert_ok(ret);
				//dump_packet(skb->data,skb->len,"ICMP inbound"); 
				pPktHdr->l3Modify=1;	//20141023LUKE: for wifi to check if recalculate chksum or not
				return RG_FWDENGINE_RET_DIRECT_TX;	

			}


#ifdef __KERNEL__ //model skip alg
			/*siyuan add for alg IPsec passthrough*/
			if(pPktHdr->tagif & ESP_TAGIF)
			{			
				ret = rtk_rg_alg_ESP(NAPT_DIRECTION_INBOUND, (unsigned char *)pPktHdr, (unsigned char *)skb, pIsakmp);
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP

				*pPktHdr->pIpv4Dip=htonl(pIsakmp->local_ip);
				pPktHdr->ipv4Dip=pIsakmp->local_ip;
				
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,FAIL,pPktHdr,skb,0,0); //DIP already be modified by this function. Don't need to modify again.
				if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
					
				return RG_FWDENGINE_RET_DIRECT_TX;		
			}
#endif			

		}
		else if(pPktHdr->ipv4MoreFragment && pPktHdr->ipv4FragmentOffset==0)	//first packet which has L4 header of these fragments
		{
#ifdef CONFIG_RG_NAPT_TCP_AUTO_LEARN
			if(pPktHdr->tagif&TCP_TAGIF)
			{				
				//Keep original information for L4 checksum recaculate
				ori_dstIP = ntohl(*pPktHdr->pIpv4Dip);
				ori_dstPort = ntohs(*pPktHdr->pDport);
				ori_seq = ntohl(*pPktHdr->pTcpSeq);
				ori_ack = ntohl(*pPktHdr->pTcpAck);

				naptOutIdx=_rtk_rg_naptTcpUdpInHashIndexLookup(1,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->ipv4Dip,pPktHdr->dport);
				ret = _rtk_rg_fwdEngine_TCPInboundConnectionTracking(pPktHdr,&naptOutIdx);

				assert(naptOutIdx>=0);

				//before add to fragment table, look up for created frag list
				pFragInList = NULL;
				_rtk_rg_fwdEngine_fragmentInHashIndexLookup(&fragIdx,&pFragInList,pPktHdr->ipProtocol,pPktHdr->ipv4Sip, pPktHdr->ipv4Dip, ntohs(*pPktHdr->pIpv4Identification), pPktHdr->l3Len-pPktHdr->ipv4HeaderLen);
				if(pFragInList!=NULL)
				{
					//DEBUG("totalLength is %d",pFragInList->totalLength);
					//20141119LUKE: for inbound list we should keep outIdx in it!!
					pFragInList->pktInfo.napt.NaptOutboundEntryIndex=naptOutIdx;
					if(pFragInList->fragAction==RG_FWDENGINE_RET_DROP || pFragInList->fragAction==RG_FWDENGINE_RET_TO_PS)
					{
						TRACE("%s!",(pFragInList->fragAction==RG_FWDENGINE_RET_DROP)?"Drop":"ToPS");						
						return pFragInList->fragAction;
					}
					else
					{
						//DEBUG("set pFragInList->fragAction to %d",ret);
						pFragInList->fragAction=ret;	//forward, drop, or to_ps
					}
				}
				else
					_rtk_rg_fwdEngine_fillInFragmentInfo(ret,naptOutIdx,NULL,pPktHdr,&pFragInList);

				//Check total fragment length
				if(pFragInList->totalLength>0 && pFragInList->receivedLength>=pFragInList->totalLength)
				{
					DEBUG("[inBound]fragments(%d) are all received(%d)!! free the list %p in [%d]...",pFragInList->totalLength,pFragInList->receivedLength,pFragInList,fragIdx);
					_rtk_rg_freeFragInList(fragIdx,pFragInList);
				}
				
				if(ret!=RG_FWDENGINE_RET_NAPT_OK)return ret;

				naptInIdx=rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;
				/*record result for internalPri decision*/
				pPktHdr->l4Direction = RG_NAPTR_INBOUND_FLOW;
				pPktHdr->naptrInboundIndx = naptInIdx;
				pPktHdr->naptOutboundIndx = naptOutIdx;	//value get in _rtk_rg_fwdEngine_TCPInboundConnectionTracking()	

				/* Packet forwarded by Forwarding Engine, reset idle time */
				rg_db.naptOut[naptOutIdx].idleSecs=0;				
					
				//packet modify and directTX
				//dump_packet(skb->data,skb->len,"org");
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,naptInIdx,pPktHdr,skb,1,1);
				pFragInList->fragAction=ret;
				if(ret==RG_FWDENGINE_RET_DROP)				
					return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
				else if(ret==RG_FWDENGINE_RET_TO_PS)
					return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue
				
				//Recaculate L4 checksum, let HW do L3 checksum
				*pPktHdr->pL4Checksum = htons(_rtk_rg_fwdengine_L4checksumUpdate(pPktHdr->tcpFlags.ack,*pPktHdr->pL4Checksum,ori_dstIP,ori_dstPort,ori_seq,ori_ack,ntohl(*pPktHdr->pIpv4Dip),ntohs(*pPktHdr->pDport),ntohl(*pPktHdr->pTcpSeq),ntohl(*pPktHdr->pTcpAck)));
					
				//dump_packet(skb->data,skb->len,"new");
				return RG_FWDENGINE_RET_FRAGMENT_ONE;
			}
#endif
			
			if(pPktHdr->tagif&UDP_TAGIF)
			{
				//Keep original information for L4 checksum recaculate
				ori_dstIP = ntohl(*pPktHdr->pIpv4Dip);
				ori_dstPort = ntohs(*pPktHdr->pDport);
	
#ifdef CONFIG_RG_NAPT_INBOUND_TRACKING
				ret = _rtk_rg_fwdEngine_UDPInboundConnectionTracking(pPktHdr, &naptOutIdx);
#else
				naptOutIdx=_rtk_rg_naptTcpUdpInHashIndexLookup(0,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->ipv4Dip,pPktHdr->dport);
				ret = RG_FWDENGINE_RET_NAPT_OK;
#endif
				assert(naptOutIdx>=0);
				if(naptOutIdx<0) 
				{
					//dump_packet(pData,len,"outIdx=fail");
					ret = RG_FWDENGINE_RET_TO_PS;
				}

				//before add to fragment table, look up for created frag list
				pFragInList = NULL;
				_rtk_rg_fwdEngine_fragmentInHashIndexLookup(&fragIdx,&pFragInList,pPktHdr->ipProtocol,pPktHdr->ipv4Sip, pPktHdr->ipv4Dip, ntohs(*pPktHdr->pIpv4Identification), pPktHdr->l3Len-pPktHdr->ipv4HeaderLen);
				if(pFragInList!=NULL)
				{
					//DEBUG("totalLength is %d",pFragInList->totalLength);
					//20141119LUKE: for inbound list we should keep outIdx in it!!
					pFragInList->pktInfo.napt.NaptOutboundEntryIndex=naptOutIdx;
					if(pFragInList->fragAction==RG_FWDENGINE_RET_DROP || pFragInList->fragAction==RG_FWDENGINE_RET_TO_PS)
					{
						TRACE("%s!",(pFragInList->fragAction==RG_FWDENGINE_RET_DROP)?"Drop":"ToPS");
						return pFragInList->fragAction;
					}
					else
					{
						//DEBUG("set pFragInList->fragAction to %d",ret);
						pFragInList->fragAction=ret;	//forward or to_ps
					}
				}
				else
					_rtk_rg_fwdEngine_fillInFragmentInfo(ret,naptOutIdx,NULL,pPktHdr,&pFragInList);

				//Check total fragment length
				if(pFragInList->totalLength>0 && pFragInList->receivedLength>=pFragInList->totalLength)
				{
					DEBUG("[inBound]fragments(%d) are all received(%d)!! free the list %p in [%d]...",pFragInList->totalLength,pFragInList->receivedLength,pFragInList,fragIdx);
					_rtk_rg_freeFragInList(fragIdx,pFragInList);
				}

				if(ret==RG_FWDENGINE_RET_DROP)
					return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
				else if(ret==RG_FWDENGINE_RET_TO_PS)
					return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue

				naptInIdx=rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;
				/*record result for internalPri decision*/
				pPktHdr->l4Direction = RG_NAPTR_INBOUND_FLOW;
				pPktHdr->naptrInboundIndx = naptInIdx;
				pPktHdr->naptOutboundIndx = naptOutIdx;	//value get in _rtk_rg_fwdEngine_UDPInboundConnectionTracking()	

				/* Packet forwarded by Forwarding Engine, reset idle time */
				rg_db.naptOut[naptOutIdx].idleSecs=0;
				
				//packet modify and directTX
				//dump_packet(pData,len,"org");
				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,naptInIdx,pPktHdr,skb,1,1);
				pFragInList->fragAction=ret;
				if(ret==RG_FWDENGINE_RET_DROP)
					return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
				else if(ret==RG_FWDENGINE_RET_TO_PS)
					return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue
				
				//Recaculate L4 checksum, let HW do L3 checksum
				*pPktHdr->pL4Checksum = htons(_rtk_rg_fwdengine_L4checksumUpdate(0,*pPktHdr->pL4Checksum,ori_dstIP,ori_dstPort,0,0,ntohl(*pPktHdr->pIpv4Dip),ntohs(*pPktHdr->pDport),0,0));
				//dump_packet(pData,len,"new");
				return RG_FWDENGINE_RET_FRAGMENT_ONE;
			}

			if(pPktHdr->tagif&ICMP_TAGIF)
			{
				//DEBUG("before lookup ICMP control flow!!(first inbound fragment packet)");
				icmpCtrlFlow = _rtk_rg_fwdEngine_ICMPInboundControlFlowTracking(pPktHdr);

				if(icmpCtrlFlow==NULL)
				{
					TRACE("ICMP flow lookup failed in NAPTR..ret=to_PS");
					//dump_packet(skb->data,skb->len,"first return to PS");
					ret = RG_FWDENGINE_RET_TO_PS;
				}
				else
				{
					//Keep IP identifier for other fragment packets
					icmpCtrlFlow->inboundIPID=ntohs(*pPktHdr->pIpv4Identification);
					ret = RG_FWDENGINE_RET_NAPT_OK;
				}

				//1 FIXME:Here should check MTU size, too!!!

				//before add to fragment table, look up for created frag list
				pFragInList = NULL;
				_rtk_rg_fwdEngine_fragmentInHashIndexLookup(&fragIdx,&pFragInList,pPktHdr->ipProtocol,pPktHdr->ipv4Sip, pPktHdr->ipv4Dip, ntohs(*pPktHdr->pIpv4Identification), pPktHdr->l3Len-pPktHdr->ipv4HeaderLen);
				if(pFragInList!=NULL)
				{
					//DEBUG("totalLength is %d",pFragInList->totalLength);
					if(pFragInList->fragAction==RG_FWDENGINE_RET_DROP || pFragInList->fragAction==RG_FWDENGINE_RET_TO_PS)
					{
						TRACE("%s!",(pFragInList->fragAction==RG_FWDENGINE_RET_DROP)?"Drop":"ToPS");
						return pFragInList->fragAction;
					}
					else
					{
						//DEBUG("set pFragInList->fragAction to %d",ret);
						pFragInList->fragAction=ret;	//forward  or to_ps
					}
				}
				else
					_rtk_rg_fwdEngine_fillInFragmentInfo(ret,FAIL,icmpCtrlFlow,pPktHdr,&pFragInList);

				//Check total fragment length
				if(pFragInList->totalLength>0 && pFragInList->receivedLength>=pFragInList->totalLength)
				{
					DEBUG("[inBound]fragments(%d) are all received(%d)!! free the list %p in [%d]...",pFragInList->totalLength,pFragInList->receivedLength,pFragInList,fragIdx);
					_rtk_rg_freeFragInList(fragIdx,pFragInList);
				}

				if(ret==RG_FWDENGINE_RET_DROP)
					return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
				else if(ret==RG_FWDENGINE_RET_TO_PS)
					return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue

				//Modify DIP
				*pPktHdr->pIpv4Dip=htonl(icmpCtrlFlow->internalIP);
				//pPktHdr->ipv4Dip=icmpCtrlFlow->internalIP;

				ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,FAIL,pPktHdr,skb,0,0);
				pFragInList->fragAction=ret;
				if(ret==RG_FWDENGINE_RET_DROP)
					return RG_FWDENGINE_RET_FRAG_ONE_DROP;		//drop all same identification in queue
				else if(ret==RG_FWDENGINE_RET_TO_PS)
					return RG_FWDENGINE_RET_FRAG_ONE_PS;		//trap all same identification in queue
				pPktHdr->l3Modify=1;	//20141023LUKE: for wifi to check if recalculate chksum or not
				return RG_FWDENGINE_RET_FRAGMENT_ONE;
			}
		}
		else if(pPktHdr->ipv4FragmentOffset>0)	//other fragments 
		{
			naptOutIdx = FAIL;
			//icmpCtrlFlow = NULL;
			pFragInList = NULL;
			//if(pPktHdr->tagif&TCP_TAGIF)
			naptOutIdx = _rtk_rg_fwdEngine_fragmentInHashIndexLookup(&fragIdx,&pFragInList,pPktHdr->ipProtocol,pPktHdr->ipv4Sip, pPktHdr->ipv4Dip, ntohs(*pPktHdr->pIpv4Identification), pPktHdr->l3Len-pPktHdr->ipv4HeaderLen);
			/*else if(pPktHdr->tagif&UDP_TAGIF)
				naptOutIdx = _rtk_rg_fwdEngine_fragmentTcpUdpInHashIndexLookup(&fragIdx,&fragAct,0,pPktHdr->ipv4Sip, pPktHdr->ipv4Dip, ntohs(*pPktHdr->pIpv4Identification));
			else if(pPktHdr->tagif&ICMP_TAGIF)
			{
				icmpCtrlFlow = _rtk_rg_fwdEngine_ICMPInboundFragmentLookup(pPktHdr);
				if(icmpCtrlFlow!=NULL)
				{
					//DEBUG("find the inbound fragment flow!! inboundIPID is %d",icmpCtrlFlow->inboundIPID);
					//Modify DIP
					*pPktHdr->pIpv4Dip=htonl(icmpCtrlFlow->internalIP);
					pPktHdr->ipv4Dip=icmpCtrlFlow->internalIP;

					ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,0,pPktHdr,skb,0,0);
					if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
					//assert_ok(ret);
					//dump_packet(skb->data,skb->len,"ICMP fragment"); 	
					if(pPktHdr->ipv4MoreFragment==0)		//final fragment packet, invalid the ctrl flow
					{
						icmpCtrlFlow->valid=0;
						//DEBUG("ICMP inbound final one, invalid the ctrl flow...");
					}
					return RG_FWDENGINE_RET_FRAGMENT;
				}

				//1 FIXME:Here should check MTU size, too!!!

				if(pPktHdr->isGatewayPacket)		//fragment packet to gateway should goto protocol stack directly, not queuing
					return RG_FWDENGINE_RET_TO_PS;
				//DEBUG("inbound fragment ICMP flow is NULL.....queuing");
			}*/

			if(pFragInList!=NULL)		//filled by first packet or other early fragment packets
			{
				//DEBUG("later fragment modify... fragIdx=%d,pktcnt=%d",fragIdx,pFragList->pktCount);
				if(pPktHdr->ipv4MoreFragment == 0 && fragIdx != FAIL)		//we can not free fragList here, unless there are all packets had forwarded
					pFragInList->totalLength=(pPktHdr->ipv4FragmentOffset<<3)+pPktHdr->l3Len-pPktHdr->ipv4HeaderLen;
				//DEBUG("totalLength is %d",pFragInList->totalLength);
				//Check total fragment length
				if(pFragInList->totalLength>0 && pFragInList->receivedLength>=pFragInList->totalLength)
				{
					DEBUG("[inBound]fragments(%d) are all received(%d)!! free the list %p in [%d]...",pFragInList->totalLength,pFragInList->receivedLength,pFragInList,fragIdx);
					_rtk_rg_freeFragInList(fragIdx,pFragInList);
				}
				
				DEBUG("Check for frag action:%d",pFragInList->fragAction);
				//Check frag action from first packet
				if(pFragInList->fragAction==RG_FWDENGINE_RET_TO_PS || pFragInList->fragAction==RG_FWDENGINE_RET_DROP)
				{
					TRACE("%s!",(pFragInList->fragAction==RG_FWDENGINE_RET_DROP)?"Drop":"ToPS");
					return pFragInList->fragAction;
				}
				if(pFragInList->fragAction==RG_FWDENGINE_RET_QUEUE_FRAG)	//first packet not come in yet
					goto INBOUND_FRAG_QUEUE;
				//DEBUG("normal forward!");
				if(naptOutIdx!=FAIL)		//TCP or UDP
				{
					naptInIdx=rg_db.naptOut[naptOutIdx].rtk_naptOut.hashIdx;
					/*record result for internalPri decision*/
					pPktHdr->l4Direction = RG_NAPTR_INBOUND_FLOW;
					pPktHdr->naptOutboundIndx = naptOutIdx;
					pPktHdr->naptrInboundIndx = naptInIdx;

					/* Packet forwarded by Forwarding Engine, reset idle time */
					rg_db.naptOut[naptOutIdx].idleSecs=0;

					//DEBUG("UDP last fragment modify...");
					//if(pPktHdr->ipv4MoreFragment == 0 && fragIdx != FAIL)		//we can not free fragList here, if the last one is coming before other frags....
						//_rtk_rg_freeFragInList(fragIdx,pFragInList);

					//Modify the DIP and forward it 
					ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,naptInIdx,pPktHdr,skb,1,0);		//fragment packet without dport
					if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
					//assert_ok(ret);
					
					return RG_FWDENGINE_RET_FRAGMENT;
				}
				else	//ICMP
				{
					DEBUG("find the inbound fragment flow!! inboundIPID is %d",pFragInList->identification);

					//Modify DIP
					*pPktHdr->pIpv4Dip=htonl(pFragInList->pktInfo.icmp.intIp);
					//pPktHdr->ipv4Dip=pFragInList->pktInfo.icmp.intIp;
					
					//DEBUG("ICMP last fragment modify...");
					//if(pPktHdr->ipv4MoreFragment == 0 && fragIdx != FAIL)		//we can not free fragList here, if the last one is coming before other frags....
					//{
						//pFragInList->pktInfo.pICMPCtrlFlow->valid=0;
						//_rtk_rg_freeFragInList(fragIdx,pFragInList);
					//}

					ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_INBOUND,FAIL,pPktHdr,skb,0,0); //DIP already be modified by this function.Don't need to modify again.
					if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
					//assert_ok(ret);
					//dump_packet(skb->data,skb->len,"ICMP fragment"); 	
					pPktHdr->l3Modify=1;	//20141023LUKE: for wifi to check if recalculate chksum or not
					return RG_FWDENGINE_RET_FRAGMENT;
				}
			}
			else
			{
				//1 FIXME:Here should check MTU size, too!!!

				//Otherwise we need to queue this packet for later proceed
				_rtk_rg_fwdEngine_fillInFragmentInfo(RG_FWDENGINE_RET_QUEUE_FRAG,FAIL,NULL,pPktHdr,&pFragInList);
				
INBOUND_FRAG_QUEUE:				
				//DEBUG("queuing packet.....");
				//DEBUG("before to PS");
				//20141113LUKE: here always DMAC==gatewayMAC since we are doing NAPTR, so just queue it for decision when first packet comein.
				/*if(pPktHdr->isGatewayPacket)		//fragment packet to gateway should goto protocol stack directly, not queuing
				{
					pFragInList->fragAction=RG_FWDENGINE_RET_TO_PS;
					return RG_FWDENGINE_RET_TO_PS;
				}*/
				//DEBUG("before queuing...");

				//Check if we already queue same identification for MAX_FRAGMENT_QUEUE_THRESHOLD times
				pFragInList->queueCount++;
				if(pFragInList->queueCount>=MAX_FRAGMENT_QUEUE_THRESHOLD)
				{
					//clear same identification in queue
					pFragInList->queueCount=0;
					pFragInList->fragAction=RG_FWDENGINE_RET_DROP;
					_rtk_rg_fwdEngine_fragmentQueueProcessing(pFragInList->fragAction,pPktHdr);
					TRACE("Drop!");
					return RG_FWDENGINE_RET_DROP;
				}
				else
				{
					//Put the fragment packet into queue
					_rtk_rg_fwdEngine_fragmentPacketQueuing(NAPT_DIRECTION_INBOUND,skb,pPktHdr);
					return RG_FWDENGINE_RET_QUEUE_FRAG;
				}
			}
		}
	}

	if(sipDipClass==SIP_DIP_CLASS_ROUTING)
	{
		pPktHdr->fwdDecision=RG_FWD_DECISION_ROUTING;

		//ret = _rtk_rg_routingDecisionTablesLookup(pPktHdr,SIP_DIP_CLASS_ROUTING);
		//if(ret!=RG_FWDENGINE_RET_CONTINUE) return ret;
		
		//dump_packet(skb->data,skb->len,"org");
		TRACE("Routing...modify packet and update shortcut");		
		pPktHdr->shortcutStatus=RG_SC_NEED_UPDATE;
		ret = _rtk_rg_fwdEngine_naptPacketModify(NAPT_DIRECTION_ROUTING,FAIL,pPktHdr,skb,0,0);
		//dump_packet(skb->data,skb->len,"new");
		if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;		//TO PS or DROP
		//assert_ok(ret);

		return RG_FWDENGINE_RET_DIRECT_TX;
	}

	return RG_FWDENGINE_RET_TO_PS;
}

static rtk_rg_fwdEngineReturn_t _rtk_rg_multicastRxCheck(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr, int pid)
{
#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
	unsigned int  vlanRelayPortMask=0;
	if(pPktHdr->ingressPort==RTK_RG_PORT_CPU) return RG_FWDENGINE_RET_CONTINUE;

	if(((pPktHdr->pDmac[0]==0x01 && pPktHdr->pDmac[1]==0x00 && pPktHdr->pDmac[2]==0x5e) || 
	    (((pPktHdr->pDmac[0]&1)==0)&&((pPktHdr->tagif&(PPPOE_TAGIF|IPV4_TAGIF))==(PPPOE_TAGIF|IPV4_TAGIF))&&(pPktHdr->ipv4Dip>=0xe0000000))
	   ))
	  //  &&(pPktHdr->ipv4Dip>=0xe0000100)&&(pPktHdr->ipv4Dip!=0xeffffffa)//for pppoe_multicast_v4
	{
		if(pPktHdr->tagif&IGMP_TAGIF && rg_db.systemGlobal.multicastProtocol!=RG_MC_MLD_ONLY) 
		{	
			rtl_igmpMldProcess(rg_db.systemGlobal.nicIgmpModuleIndex, skb->data, pPktHdr, pid, &vlanRelayPortMask);
			//return RG_FWDENGINE_RET_CONTINUE;	//20130722:multicast should continue to broadcast module in fwdEngine, not directly return to protocol stack
		}
	}
	else if ((pPktHdr->pDmac[0]==0x33 && pPktHdr->pDmac[1]==0x33 && pPktHdr->pDmac[2]!=0xff) ||
		(((pPktHdr->pDmac[0]&1)==0)&&((pPktHdr->tagif&(PPPOE_TAGIF|IPV6_TAGIF))==(PPPOE_TAGIF|IPV6_TAGIF))&&(pPktHdr->pIpv6Dip[0]==0xff)))	//for pppoe_multicast_v6
	{
		if(pPktHdr->tagif&IPV6_MLD_TAGIF && rg_db.systemGlobal.multicastProtocol!=RG_MC_IGMP_ONLY)
		{
			/*icmpv6 packet*/
			rtl_igmpMldProcess(rg_db.systemGlobal.nicIgmpModuleIndex, skb->data, pPktHdr, pid, &vlanRelayPortMask);
			//return RG_FWDENGINE_RET_CONTINUE;	//20130722:multicast should continue to broadcast module in fwdEngine, not directly return to protocol stack
		}
	} 

#endif	
	return RG_FWDENGINE_RET_CONTINUE;
}

rtk_rg_entryGetReturn_t _rtk_rg_fwdEngineDestinationLookup(rtk_rg_pktHdr_t *pPktHdr)
{
	//First use packet's vlan value to find SVL or IVL,
	//then use packet header's Destination Mac address to look up for the destination port
	rtk_fidMode_t fidMode;
	int hashedIdx,VLANId,FId,EFId,i,search_index;
	rtk_rg_lut_linkList_t *pSoftLut;

	if(pPktHdr->netifIdx==FAIL)	//L2
	{
		//DEBUG("$$$$$$$$$$$$$$$$ in %s, the pkthdr internal VID is %d",__FUNCTION__,pPktHdr->internalVlanID);
		VLANId=pPktHdr->internalVlanID;
		fidMode=rg_db.vlan[VLANId].fidMode;
		FId=rg_db.vlan[VLANId].fid;
		EFId=0;		//FIXME:we did not store efid right now
	}
	else
	{
		//DEBUG("$$$$$$$$$$$$$$ the netif idx is %d",pPktHdr->netifIdx);
		if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan)
		{
			VLANId=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.egress_vlan_id;
			fidMode=rg_db.vlan[VLANId].fidMode;
			FId=rg_db.vlan[VLANId].fid;
			EFId=0;		//FIXME:we did not store efid right now
			//DEBUG("WAN!! pPktHdr->netifIdx is %d, VLANID is %d",pPktHdr->netifIdx,VLANId);
		}
		else
		{
			VLANId=rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.lan_intf.intf_vlan_id;
			fidMode=rg_db.vlan[VLANId].fidMode;
			FId=rg_db.vlan[VLANId].fid;
			EFId=0;		//FIXME:we did not store efid right now
			//DEBUG("LAN!! pPktHdr->netifIdx is %d, VLANID is %d",pPktHdr->netifIdx,VLANId);
		}
	}
	
	if(fidMode==VLAN_FID_IVL)
		hashedIdx=_rtk_rg_hash_mac_vid_efid(pPktHdr->pDmac,VLANId,EFId);
	else
		hashedIdx=_rtk_rg_hash_mac_fid_efid(pPktHdr->pDmac,FId,EFId);
	
	hashedIdx<<=2;
	for(i=0;i<4;i++)
	{
		search_index=hashedIdx+i;
		if(rg_db.lut[search_index].valid==0)
			continue;
		
		if(rg_db.lut[search_index].rtk_lut.entryType==RTK_LUT_L2UC &&
			(memcmp(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.mac.octet,pPktHdr->pDmac,ETHER_ADDR_LEN)==0))
		{
			if((fidMode==VLAN_FID_IVL && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid==VLANId) || 
				(fidMode==VLAN_FID_SVL && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.fid==FId))
			{
				//rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;
				//rg_kernel.txDescMask.opts3.bit.l34_keep=1;
				
				//rg_kernel.txDesc.opts3.bit.tx_portmask=0x1<<rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port;
				//rg_kernel.txDesc.opts3.bit.l34_keep=0;
				//DEBUG("lookup! to portmask %x",0x1<<rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port);
				goto LUT_FOUND;
			}
		}
	}
	if(i==4)
	{
		//Check bCAM LUT, if match, just return
		for(search_index=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE;search_index<MAX_LUT_HW_TABLE_SIZE;search_index++)
		{
			if(rg_db.lut[search_index].valid && rg_db.lut[search_index].rtk_lut.entryType==RTK_LUT_L2UC)
			{
				if(memcmp(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.mac.octet,pPktHdr->pDmac,ETHER_ADDR_LEN)==0)
				{
					if((fidMode==VLAN_FID_IVL && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid==VLANId) ||
					(fidMode==VLAN_FID_SVL && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.fid==FId))
					{
						//HIT!
						goto LUT_FOUND;
					}
				}
			}
		}	
	}

	//look up software LUT link-list
	if(!list_empty(&rg_db.softwareLutTableHead[hashedIdx>>2]))
	{
		list_for_each_entry(pSoftLut,&rg_db.softwareLutTableHead[hashedIdx>>2],lut_list)
		{
			if(memcmp(rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.mac.octet,pPktHdr->pDmac,ETHER_ADDR_LEN)==0)
			{
				if((fidMode==VLAN_FID_IVL && rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.vid==VLANId) ||
				(fidMode==VLAN_FID_SVL && rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.fid==FId))
				{
					//HIT!
					search_index=pSoftLut->idx;
					goto LUT_FOUND;
				}
			}
		}
	}

	TRACE("DA Lookup Fail: unknown DA..do broadcast!");
	pPktHdr->fwdDecision=RG_FWD_DECISION_NO_PS_BC;		//do not goto protocol stack!
	pPktHdr->directTxResult=_rtk_rg_broadcastForwardWithPkthdr(pPktHdr,pPktHdr->skb,pPktHdr->internalVlanID,pPktHdr->pRxDesc->opts3.bit.src_port_num,pPktHdr->ingressPort);
	return RG_RET_ENTRY_NOT_GET;
	//rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;					
	//rg_kernel.txDesc.opts3.bit.tx_portmask=0x1<<rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port;
LUT_FOUND:
	pPktHdr->dmacL2Idx=search_index;
	return search_index;	
}

rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngineVLANFiltering(int vlanId, int port, int extPort)
{
	if(rg_db.vlan[vlanId].valid)
	{
		if(port==RTK_RG_PORT_CPU)
		{
			if((rg_db.vlan[vlanId].MemberPortmask.bits[0]&(0x1<<RTK_RG_PORT_CPU)) && (rg_db.vlan[vlanId].Ext_portmask.bits[0]&(0x1<<extPort)))
				return RG_FWDENGINE_RET_CONTINUE;
		}
		else
		{
			if(rg_db.vlan[vlanId].MemberPortmask.bits[0]&(0x1<<port))
				return RG_FWDENGINE_RET_CONTINUE;
		}
	}

	TRACE("Drop!");
	return RG_FWDENGINE_RET_DROP;
}

int _rtk_rg_fwdEngineMacFilter(rtk_rg_pktHdr_t *pPktHdr)
{
	int32 l2Idx=FAIL;
	
	//SA
	l2Idx=_rtk_rg_macLookup(pPktHdr->pSmac,pPktHdr->internalVlanID);
	if(l2Idx!=FAIL && l2Idx<MAX_LUT_HW_TABLE_SIZE)
	{
		if(rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.flags & RTK_L2_UCAST_FLAG_SA_BLOCK)
		{
			TRACE("Drop by SA macfilter!");
			return (RG_FWDENGINE_RET_DROP);
		}
	}

	//DA
	l2Idx=_rtk_rg_macLookup(pPktHdr->pDmac,pPktHdr->internalVlanID);
	if(l2Idx!=FAIL && l2Idx<MAX_LUT_HW_TABLE_SIZE)
	{
		if(rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.flags & RTK_L2_UCAST_FLAG_DA_BLOCK)
		{
			TRACE("Drop by DA macfilter!");
			return (RG_FWDENGINE_RET_DROP);
		}
	}

	return (RG_FWDENGINE_RET_CONTINUE);
}

rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngineDMAC2CVIDTransfer(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{	
	//Use packet header's Destination Mac address to look up for the vlanid and destination port
	int VLANId;
	int aclRet;
	int i,naptIdx=0;
	int bridge_netf_idx=FAIL;
	

	//20150528LUKE: only non-shortcut will do dport lookup and egress ACL
	//Assign Port
	pPktHdr->egressMACPort=rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.port;
	pPktHdr->egressExtPort=rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.ext_port;

	//20140711LUKE:only apply egressVlanId and netif when egressMACport is in WAN
	if((0x1<<pPktHdr->egressMACPort)&rg_db.systemGlobal.wanPortMask.portmask){
		if(pPktHdr->layer2BindNetifIdx!=FAIL){//binding to bridgeWan
			pPktHdr->netifIdx=pPktHdr->layer2BindNetifIdx;
			_rtk_rg_interfaceVlanIDPriority(pPktHdr,&rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo,NULL,NULL);
			TRACE("Apply Binding interface index:%d, egress VLAN changes to %d",pPktHdr->netifIdx,pPktHdr->egressVlanID);
		}
	}

#if defined(CONFIG_RTL9602C_SERIES)	
	DEBUG("rg_db.systemGlobal.ponDmac2cvidDisabled=0x%x, force disbled DAMC2CVID to port[%d]!",rg_db.systemGlobal.dmac2cvidDisabledPortmask, pPktHdr->egressMACPort);
	if(rg_db.systemGlobal.dmac2cvidDisabledPortmask & (1<<pPktHdr->egressMACPort))//force diabled PON DAMC2CVID
	{
		TRACE("rg_db.systemGlobal.ponDmac2cvidDisabled=0x%x, force disbled DAMC2CVID to port[%d]!",rg_db.systemGlobal.dmac2cvidDisabledPortmask, pPktHdr->egressMACPort);
	}
	else
	{
		//20150925LUKE: for 9602C, vlan egress filtering should include DMAC2CVID decision!
		VLANId=rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.vid;
		if(rg_db.vlan[VLANId].fidMode==VLAN_FID_SVL && ((rg_db.systemGlobal.svlan_EP_DMAC_CTRL_pmsk.portmask&(0x1<<(rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.port))) || (pPktHdr->egressMACPort==RTK_RG_PORT_CPU))){
			//assign DMAC2CVID to internalVlanID !!
			pPktHdr->internalVlanID = VLANId;
			//pPktHdr->dmac2VlanID = VLANId;
			//pPktHdr->dmac2VlanTagif=((rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_CTAG_IF)>0);
			//TRACE("Do DMAC2CVID before filtering...VID=%d(%s)",VLANId,pPktHdr->dmac2VlanTagif?"TAGGED":"UNTAG");
		}
	}
#endif

	//20140707LUKE:since DMAC2CVID only change VLANID and tagif in hw, we should use egressVlanID to check egress filterv before DMAC2CVID!!
	//20150325CHUCK: egress vlan filter should filter internal VLAN(not include CF and DAM2CVID)
	//20150924LUKE: for 9602C, egress vlan filter contains DMAC2CVID decision!!
	//VLAN egress filter
	if(_rtk_rg_fwdEngineVLANFiltering(pPktHdr->internalVlanID,pPktHdr->egressMACPort,pPktHdr->egressExtPort)==RG_FWDENGINE_RET_DROP){
		TRACE("Drop by VLAN(%d) Egress filter",pPktHdr->internalVlanID);
		return RG_FWDENGINE_RET_DROP;
	}

	//20150520LUKE: for packet from procotol stack we should keep its original format of cvlan if keepPsOrigCvlan is on.
	if(pPktHdr->ingressLocation==RG_IGR_PROTOCOL_STACK && rg_db.systemGlobal.keepPsOrigCvlan){
		pPktHdr->egressVlanTagif=(pPktHdr->tagif&CVLAN_TAGIF?1:0);
		pPktHdr->egressPriority=(pPktHdr->tagif&CVLAN_TAGIF?pPktHdr->ctagPri:0);
	}else{
		//no need, egressVlanID is decided in _rtk_rg_interfaceVlanIDPriority!!
		/*VLANId=rg_kernel.txDesc.opts2.bit.vidh;
		VLANId<<=8;
		VLANId|=rg_kernel.txDesc.opts2.bit.vidl;
		pPktHdr->egressVlanID = VLANId;//get egress CVID*/
		VLANId = pPktHdr->egressVlanID; 		
		if(rg_db.vlan[VLANId].fidMode==VLAN_FID_IVL){	
			//tag/untag by VLAN untag setting
			if(rg_db.vlan[VLANId].UntagPortmask.bits[0]&(0x1<<pPktHdr->egressMACPort)){
				TRACE("IVL:untagged!");
				pPktHdr->egressVlanTagif=0;
			}else{
				TRACE("IVL:tagged!");
				pPktHdr->egressVlanTagif=1;

				//Qos remarking: Chuck
				if(rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[pPktHdr->egressMACPort]){
					TRACE("QoS dop1p Remarking by port[%d]:%s, CRI %d",pPktHdr->egressMACPort,rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[pPktHdr->egressMACPort]?"ENABLED":"DISABLED",rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPri[pPktHdr->internalPriority]); 				
					//record egressPri
					pPktHdr->egressPriority=rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPri[pPktHdr->internalPriority];
				}
			}
		}else{

			if(rg_db.vlan[VLANId].UntagPortmask.bits[0]&(0x1<<pPktHdr->egressMACPort)){
				TRACE("SVL:untagged!");
				pPktHdr->egressVlanTagif=0; 
			}else{
				TRACE("SVL:tagged!");
				pPktHdr->egressVlanTagif=1;
			}
		
			//Check if we turn on DMAC2CVID or not
			//20150925LUKE: for 9602C, dmac2VlanID would be decided above, so check it here!
			//20150313LUKE: from protocol stack should do DMAC2CVID, too!!
			//20140715LUKE:binding and packet from protocol stack won't proceed DMAC2CVID!
			if(rg_db.systemGlobal.initParam.macBasedTagDecision  
#if defined(CONFIG_RTL9600_SERIES)	// BINDING > DMAC2CVID
				&& pPktHdr->layer2BindNetifIdx==FAIL 				//no Layer2 binding
				&& pPktHdr->bindNextHopIdx==FAIL 					//no Layer34 binding	
#endif
				/*&& pPktHdr->ingressLocation!=RG_IGR_PROTOCOL_STACK*/){	//not from protocol stack
				//Get VLANId from LUT, it will be 0 if learned by untag!
				VLANId=rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.vid;
				//pPktHdr->egressVlanID = VLANId; //egress VID change to DMAC2CVID
				pPktHdr->dmac2VlanID = VLANId;
				//if(rg_db.vlan[VLANId].fidMode==VLAN_FID_SVL)
#if defined(CONFIG_RTL9600_SERIES)
				if(VLANId==0)
#else
				if((rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_CTAG_IF)==0)
#endif
				{
					TRACE("dmac2cvid:untagged!");
					//pPktHdr->egressVlanTagif=0;
					pPktHdr->dmac2VlanTagif=0;
				}else if((rg_db.systemGlobal.fix_l34_to_untag_enable) && (rg_db.lut[pPktHdr->dmacL2Idx].fix_l34_vlan) &&
						(pPktHdr->fwdDecision == RG_FWD_DECISION_ROUTING ||
						 pPktHdr->fwdDecision == RG_FWD_DECISION_V6ROUTING ||
						 pPktHdr->fwdDecision == RG_FWD_DECISION_NAPT ||
						 pPktHdr->fwdDecision == RG_FWD_DECISION_NAPTR)){
					MACLN("For smart STB issue: Do not add vlan %d for router packet!!",VLANId);
					TRACE("dmac2cvid:L34 untagged!");
					pPktHdr->dmac2VlanTagif=0;		
				}else{
					//Assign VLAN by DMAC2CVID, and this VLAN should be the same with VLAN above
					TRACE("dmac2cvid:tagged!");
					//pPktHdr->egressVlanTagif=1;
					pPktHdr->dmac2VlanTagif=1;
					/*if(rg_db.vlan[VLANId].priorityEn==1)
					{
						rg_kernel.txDescMask.opts2.bit.prio=0x7;
						rg_kernel.txDesc.opts2.bit.prio=rg_db.vlan[VLANId].priority;
					}*/

				}
			}

			if(pPktHdr->egressVlanTagif==1 || pPktHdr->dmac2VlanTagif==1)
			{
				//Qos remarking: Chuck
				if(rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[pPktHdr->egressMACPort]){
					TRACE("QoS dop1p Remarking by port[%d]:%s, CRI %d",pPktHdr->egressMACPort,rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPriEgressPortEnable[pPktHdr->egressMACPort]?"ENABLED":"DISABLED",rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPri[pPktHdr->internalPriority]);
					//record egressPri
					pPktHdr->egressPriority=rg_db.systemGlobal.qosInternalDecision.qosDot1pPriRemarkByInternalPri[pPktHdr->internalPriority];
				}
			}
		}

		//DSCP remarking:Chuck	
		_rtk_rg_qosDscpRemarking(pPktHdr->egressMACPort,pPktHdr,skb);
	}


	if((0x1<<pPktHdr->egressMACPort)&rg_db.systemGlobal.wanPortMask.portmask){//from Lan to Wan
		if(pPktHdr->layer2BindNetifIdx!=FAIL){//binding to bridgeWan
			bridge_netf_idx=pPktHdr->layer2BindNetifIdx;
		}else if(pPktHdr->netifIdx==FAIL){//normal bridge to bridgeWan
			//search the egress wan interface by egressVLAN
			for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++){
				if( (pPktHdr->isGatewayPacket==0) && rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo!=NULL && rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf!=NULL &&
					rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type==RTK_RG_BRIDGE){
					if(rg_db.systemGlobal.initParam.macBasedTagDecision){
						if(rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.vid==0 && rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->egress_vlan_tag_on==0){//get the bridgeWan netif_index from DMAC2CVID (untag bridgeWan)
							//check the bridgeWan netif index by vlanID from DMAC2CVID
							bridge_netf_idx=rg_db.systemGlobal.wanIntfGroup[i].index;
							TRACE("Apply Normal BridgeWan interface index:%d, upstream egress VLAN is %d with DMAC2CVID (internal VLAN is %d)",bridge_netf_idx,pPktHdr->egressVlanID,pPktHdr->internalVlanID);
						}else if(rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.vid!=0 && rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.vid==rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->egress_vlan_id){//get the bridgeWan netif_index from DMAC2CVID (Lan/bridgeWan with different VLAN)
							//check the bridgeWan netif index by vlanID from DMAC2CVID
							bridge_netf_idx=rg_db.systemGlobal.wanIntfGroup[i].index;
							TRACE("Apply Normal BridgeWan interface index:%d, upstream egress VLAN is %d with DMAC2CVID (internal VLAN is %d)",bridge_netf_idx,pPktHdr->egressVlanID,pPktHdr->internalVlanID);
						}	
					}else{
						if(pPktHdr->internalVlanID==rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->egress_vlan_id){//get the bridgeWan netif_index
							//check the bridgeWan netif index by internal vlanID
							bridge_netf_idx=rg_db.systemGlobal.wanIntfGroup[i].index;
							TRACE("Apply Normal BridgeWan interface index:%d, upstream egress VLAN is %d wouthout DMAC2CVID (internal VLAN is %d)",bridge_netf_idx,pPktHdr->egressVlanID,pPktHdr->internalVlanID);
						}	
					}
				}
			}
		}
	}else if((0x1<<pPktHdr->ingressPort)&rg_db.systemGlobal.wanPortMask.portmask){//from Wan to Lan
		//search the ingress wan interface by ingressVLAN(bridgeWan: ingressVLAN==internalVLAN)
		for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++){
			if( (pPktHdr->isGatewayPacket==0) && rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo!=NULL && rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf!=NULL &&
				rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type==RTK_RG_BRIDGE){
				
				if(pPktHdr->internalVlanID==rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->egress_vlan_id){//get the bridgeWan netif_index (Wan to Lan compare by internal Vlan)
					bridge_netf_idx=rg_db.systemGlobal.wanIntfGroup[i].index;
					TRACE("Apply Normal BridgeWan interface index:%d, downstream (internal VLAN is %d)",bridge_netf_idx,pPktHdr->internalVlanID);
				}	
			}
		}
	}

	//20140916 Chuck: Supporting IPv4/IPv6 portocal drop by Wan_type
	if(bridge_netf_idx!=FAIL && rg_db.systemGlobal.interfaceInfo[bridge_netf_idx].valid==1 && rg_db.systemGlobal.interfaceInfo[bridge_netf_idx].storedInfo.is_wan==1){
		if(rg_db.systemGlobal.bridge_netIfIdx_drop_by_portocal[bridge_netf_idx]==1){//drop IPv6
			//bridgeWan for IPv4_only, drop IPv6 related packet
			if(pPktHdr->etherType==0x86dd || pPktHdr->pppProtocal==0x8057 || pPktHdr->pppProtocal==0x0057){
				TRACE("Drop by IPv4_only bridgeWan");
				return RG_FWDENGINE_RET_DROP;
			}

		}
		if(rg_db.systemGlobal.bridge_netIfIdx_drop_by_portocal[bridge_netf_idx]==2){//drop IPv4
			//bridgeWan for IPv6_only, drop IPv4 related packet
			if(pPktHdr->etherType==0x0800 || pPktHdr->pppProtocal==0x8021 || pPktHdr->pppProtocal==0x0021){
				TRACE("Drop by IPv6_only bridgeWan");
				return RG_FWDENGINE_RET_DROP;
			}
		}
	}

	//especial filter IPCP/IP6CP with especial vid for port_binding_by_protocal
	if((0x1<<pPktHdr->ingressPort)&rg_db.systemGlobal.wanPortMask.portmask){//ingress  from Wan
		if(((pPktHdr->tagif&CVLAN_TAGIF)&&(pPktHdr->ctagVid==rg_db.systemGlobal.port_binding_by_protocal_filter_vid))||
			(((pPktHdr->tagif&CVLAN_TAGIF)== 0x0)&&(0==rg_db.systemGlobal.port_binding_by_protocal_filter_vid))){//packet with assigned PPPoE Vid decision
			if(rg_db.systemGlobal.port_binding_by_protocal==1){//IPv4 Routing, IPv6 Bridge => drop downstream IPCP
				if(pPktHdr->etherType==0x8864 && (pPktHdr->pppProtocal==0x8021 || pPktHdr->pppProtocal==0x0021)){
					TRACE("IPCP drop by  binding by protocal, IPv4 us routing");
					return RG_FWDENGINE_RET_DROP;
				}
				
			}else if(rg_db.systemGlobal.port_binding_by_protocal==2){//IPv6 Routing, IPv4 Bridge => drop downstream IP6CP
				if(pPktHdr->etherType==0x8864 && ( pPktHdr->pppProtocal==0x8057 || pPktHdr->pppProtocal==0x0057)){
					TRACE("IP6CP drop by  binding by protocal, IPv6 is routing");
					return RG_FWDENGINE_RET_DROP;
				}
			}
		}
	}


	
	//Do ACL egress check
	if(pPktHdr->fwdDecision==RG_FWD_DECISION_NAPT){
		naptIdx = pPktHdr->naptOutboundIndx;
	}else if(pPktHdr->fwdDecision==RG_FWD_DECISION_NAPTR){
		naptIdx = pPktHdr->naptrInboundIndx;
	}
	assert_ok(_rtk_rg_egressACLPatternCheck(pPktHdr->fwdDecision,naptIdx,pPktHdr,skb,pPktHdr->l3Modify,pPktHdr->l4Modify,pPktHdr->egressMACPort));		
	 ACL("call to _rtk_rg_egressACLAction by FWD_DMAC2CVID	direct=%d",pPktHdr->fwdDecision);
	aclRet = _rtk_rg_egressACLAction(pPktHdr->fwdDecision,pPktHdr);
	if(aclRet==RG_FWDENGINE_RET_DROP){				
		TRACE("Drop by Egress ACL Action");
		return RG_FWDENGINE_RET_DROP;
	}
	
	//Modify Packet by ACL actions
	_rtk_rg_modifyPacketByACLAction(skb,pPktHdr,pPktHdr->egressMACPort);
				
	if(pPktHdr->shortcutStatus==RG_SC_NEED_UPDATE_BEFORE_SEND){
#ifdef CONFIG_ROME_NAPT_SHORTCUT			
		//update vlan, priority, dscp, and streamID to shortcut
		TRACE("RG_SC_NEED_UPDATE_BEFORE_SEND!! SC_addr=[%p] VLAN[%d] Pri[%d] SVLANTAGIF[%d] SVLAN[%d] SPri[%d] dscp[%d]!!",pPktHdr->pCurrentShortcutEntry,pPktHdr->egressVlanID,pPktHdr->egressPriority,pPktHdr->egressServiceVlanTagif,pPktHdr->egressServiceVlanID,pPktHdr->egressServicePriority,pPktHdr->egressDSCP);
		if(pPktHdr->pCurrentShortcutEntry->isBridge)pPktHdr->pCurrentShortcutEntry->new_lut_idx=pPktHdr->dmacL2Idx;
#ifdef CONFIG_GPON_FEATURE		
		pPktHdr->pCurrentShortcutEntry->streamID=pPktHdr->streamID;
#endif
		pPktHdr->pCurrentShortcutEntry->vlanID=pPktHdr->egressVlanID;
		pPktHdr->pCurrentShortcutEntry->vlanTagif=pPktHdr->egressVlanTagif;
		pPktHdr->pCurrentShortcutEntry->serviceVlanID=pPktHdr->egressServiceVlanID;
		pPktHdr->pCurrentShortcutEntry->serviceVlanTagif=pPktHdr->egressServiceVlanTagif;
		pPktHdr->pCurrentShortcutEntry->dmac2cvlanID=pPktHdr->dmac2VlanID;
		pPktHdr->pCurrentShortcutEntry->dmac2cvlanTagif=pPktHdr->dmac2VlanTagif;
		//pPktHdr->pCurrentShortcutEntry->macPort=pPktHdr->egressMACPort;
		//pPktHdr->pCurrentShortcutEntry->extPort=pPktHdr->egressExtPort;
		pPktHdr->pCurrentShortcutEntry->priority=pPktHdr->egressPriority;
		pPktHdr->pCurrentShortcutEntry->servicePriority=pPktHdr->egressServicePriority;
		pPktHdr->pCurrentShortcutEntry->internalVlanID=pPktHdr->internalVlanID;
		pPktHdr->pCurrentShortcutEntry->internalCFPri=pPktHdr->internalPriority;
		pPktHdr->pCurrentShortcutEntry->dscp=pPktHdr->egressDSCP;					
		pPktHdr->pCurrentShortcutEntry->uniPortmask=pPktHdr->egressUniPortmask;
#if defined(CONFIG_RTL9602C_SERIES)
		//mib counter and cf decision
		pPktHdr->pCurrentShortcutEntry->mibNetifIdx=pPktHdr->mibNetifIdx;
		pPktHdr->pCurrentShortcutEntry->mibDirect=pPktHdr->mibDirect;
#endif
#endif				
	}
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	else if(pPktHdr->shortcutStatus==RG_SC_V6_NEED_UPDATE_BEFORE_SEND){
		//update vlan, priority, dscp, and streamID to shortcut
		TRACE("RG_SC_V6_NEED_UPDATE_BEFORE_SEND!! v6SC_addr=[%p] VLAN[%d] Pri[%d] SVLANTAGIF[%d] SVLAN[%d] SPri[%d] dscp[%d]!!",pPktHdr->pCurrentV6ShortcutEntry,pPktHdr->egressVlanID,pPktHdr->egressPriority,pPktHdr->egressServiceVlanTagif,pPktHdr->egressServiceVlanID,pPktHdr->egressServicePriority,pPktHdr->egressDSCP);
		if(pPktHdr->pCurrentV6ShortcutEntry->isBridge)pPktHdr->pCurrentV6ShortcutEntry->new_lut_idx=pPktHdr->dmacL2Idx;
#ifdef CONFIG_GPON_FEATURE		
		pPktHdr->pCurrentV6ShortcutEntry->streamID=pPktHdr->streamID;
#endif
		pPktHdr->pCurrentV6ShortcutEntry->vlanID=pPktHdr->egressVlanID;
		pPktHdr->pCurrentV6ShortcutEntry->vlanTagif=pPktHdr->egressVlanTagif;
		pPktHdr->pCurrentV6ShortcutEntry->serviceVlanID=pPktHdr->egressServiceVlanID;
		pPktHdr->pCurrentV6ShortcutEntry->serviceVlanTagif=pPktHdr->egressServiceVlanTagif;
		pPktHdr->pCurrentV6ShortcutEntry->dmac2cvlanID=pPktHdr->dmac2VlanID;
		pPktHdr->pCurrentV6ShortcutEntry->dmac2cvlanTagif=pPktHdr->dmac2VlanTagif;
		//pPktHdr->pCurrentV6ShortcutEntry->macPort=pPktHdr->egressMACPort;
		//pPktHdr->pCurrentV6ShortcutEntry->extPort=pPktHdr->egressExtPort;
		pPktHdr->pCurrentV6ShortcutEntry->priority=pPktHdr->egressPriority;
		pPktHdr->pCurrentV6ShortcutEntry->servicePriority=pPktHdr->egressServicePriority;
		pPktHdr->pCurrentV6ShortcutEntry->internalVlanID=pPktHdr->internalVlanID;
		pPktHdr->pCurrentV6ShortcutEntry->internalCFPri=pPktHdr->internalPriority;
		pPktHdr->pCurrentV6ShortcutEntry->dscp=pPktHdr->egressDSCP;
		pPktHdr->pCurrentV6ShortcutEntry->uniPortmask=pPktHdr->egressUniPortmask;
#if defined(CONFIG_RTL9602C_SERIES)
		//mib counter and cf decision
		pPktHdr->pCurrentV6ShortcutEntry->mibNetifIdx=pPktHdr->mibNetifIdx;
		pPktHdr->pCurrentV6ShortcutEntry->mibDirect=pPktHdr->mibDirect;
#endif
	}
#endif
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	else if(pPktHdr->shortcutStatus==RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND){
		//update vlan, priority, dscp, and streamID to stateful linkList
		TRACE("RG_SC_STATEFUL_NEED_UPDATE_BEFORE_SEND!! linkList=[%p] VLAN[%d] Pri[%d] SVLANTAGIF[%d] SVLAN[%d] SPri[%d] dscp[%d]!!",pPktHdr->pIPv6StatefulList,pPktHdr->egressVlanID,pPktHdr->egressPriority,pPktHdr->egressServiceVlanTagif,pPktHdr->egressServiceVlanID,pPktHdr->egressServicePriority,pPktHdr->egressDSCP);
#ifdef CONFIG_GPON_FEATURE		
		pPktHdr->pIPv6StatefulList->streamID=pPktHdr->streamID;
#endif
		pPktHdr->pIPv6StatefulList->vlanID=pPktHdr->egressVlanID;
		pPktHdr->pIPv6StatefulList->vlanTagif=pPktHdr->egressVlanTagif;
		pPktHdr->pIPv6StatefulList->serviceVlanID=pPktHdr->egressServiceVlanID;
		pPktHdr->pIPv6StatefulList->serviceVlanTagif=pPktHdr->egressServiceVlanTagif;
		pPktHdr->pIPv6StatefulList->dmac2cvlanID=pPktHdr->dmac2VlanID;
		pPktHdr->pIPv6StatefulList->dmac2cvlanTagif=pPktHdr->dmac2VlanTagif;
		pPktHdr->pIPv6StatefulList->macPort=pPktHdr->egressMACPort;
		pPktHdr->pIPv6StatefulList->extPort=pPktHdr->egressExtPort;
		pPktHdr->pIPv6StatefulList->priority=pPktHdr->egressPriority;
		pPktHdr->pIPv6StatefulList->servicePriority=pPktHdr->egressServicePriority;
		pPktHdr->pIPv6StatefulList->internalVlanID=pPktHdr->internalVlanID;
		pPktHdr->pIPv6StatefulList->internalCFPri=pPktHdr->internalPriority;
		pPktHdr->pIPv6StatefulList->dscp=pPktHdr->egressDSCP;
		pPktHdr->pIPv6StatefulList->uniPortmask=pPktHdr->egressUniPortmask;
#if defined(CONFIG_RTL9602C_SERIES)
		//mib counter and cf decision
		pPktHdr->pIPv6StatefulList->mibNetifIdx=pPktHdr->mibNetifIdx;
		pPktHdr->pIPv6StatefulList->mibDirect=pPktHdr->mibDirect;
#endif
	}
#endif

	return RG_FWDENGINE_RET_CONTINUE;
}

#ifdef CONFIG_RG_WLAN_HWNAT_ACCELERATION
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngineWIFITransfer(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	//int total_len;
	if(pPktHdr->egressExtPort==(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))	// to EXT0, wlan0-vxd, wlan1-vxd
	{
		TRACE("send to WIFI0");

		//patch for wifi no L3/L4 checksum offload
#if 0
		//because IPv6 packet won't change IP, therefore L4 checksum do not need to recaculate here!
		if(pPktHdr->tagif&IPV4_TAGIF)
		{	
			FIXME("to WIFI1 software L3 checksum");
			total_len=pPktHdr->l3Offset+pPktHdr->l3Len;
			
			*pPktHdr->pIpv4Checksum=0;
			*pPktHdr->pIpv4Checksum=htons(inet_chksum(skb->data+pPktHdr->l3Offset,pPktHdr->l4Offset-pPktHdr->l3Offset));

			if((pPktHdr->tagif&(TCP_TAGIF|UDP_TAGIF)) && pPktHdr->fwdDecision!=RG_FWD_DECISION_ROUTING && pPktHdr->ipv4FragPacket==0)	//only recaculate the non-fragment packet, since fragment packet was recaculate in l34forward
			{
				FIXME("to WIFI1 software L4 checksum");
				*pPktHdr->pL4Checksum=0;
				*pPktHdr->pL4Checksum=htons(inet_chksum_pseudo(skb->data+pPktHdr->l4Offset,total_len-pPktHdr->l4Offset,ntohl(*pPktHdr->pIpv4Sip),ntohl(*pPktHdr->pIpv4Dip),pPktHdr->ipProtocol));
			}			
		}
#endif

		//memDump(skb->data,skb->len,"to wifi0");
		if(_rtk_master_wlan_mbssid_tx(pPktHdr,skb)==RG_RET_MBSSID_NOT_FOUND)
		{
			_rtk_rg_dev_kfree_skb_any(skb);
		}
		return RG_FWDENGINE_RET_SEND_TO_WIFI;
	}
	else if(pPktHdr->egressExtPort==(RTK_RG_EXT_PORT1-RTK_RG_PORT_CPU)) // to EXT1
	{
		TRACE("send to EXT_PORT1..do hardware lookup");
#ifdef CONFIG_DUALBAND_CONCURRENT
		if(unlikely(pPktHdr->dmacL2Idx>=MAX_LUT_HW_TABLE_SIZE))
		{
			TRACE("hit software LUT...change egressVID to %d,pri to %d",CONFIG_DEFAULT_TO_SLAVE_GMAC_VID,CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI);
			pPktHdr->egressVlanID=CONFIG_DEFAULT_TO_SLAVE_GMAC_VID;
			pPktHdr->egressVlanTagif=1;
			pPktHdr->egressPriority=CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI;
			//dismiss DMAC2CVID decision
			pPktHdr->dmac2VlanID=FAIL;
		}
		return RG_FWDENGINE_RET_HWLOOKUP;
#endif
	}

	// to CPU
	pPktHdr->fwdDecision=RG_FWD_DECISION_TO_PS;
	return RG_FWDENGINE_RET_TO_PS;
}
#endif

__IRAM_FWDENG
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngineCheckDestination(rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_fwdEngineReturn_t ret;
	
	if(pPktHdr->dmacL2Idx==FAIL){
		//20150909LUKE:pPktHdr->dmacL2Idx will be assigned at _rtk_rg_fwdEngineDestinationLookup
		if(_rtk_rg_fwdEngineDestinationLookup(pPktHdr)==RG_RET_ENTRY_NOT_GET)
			return pPktHdr->directTxResult;
	}

	TRACE("DA Lookup Success: PhyPort(%d),ExtPort(%d),VLAN(%d)",rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.port,rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.ext_port,pPktHdr->egressVlanID);
			
	if(pPktHdr->shortcutStatus==RG_SC_MATCH){
		if(pPktHdr->egressDSCP>=0){
			//unsigned char tos;
			if(pPktHdr->tagif&IPV6_TAGIF){
				//dscp is the MSB 6 bits of traffic class
				//tos = pPktHdr->egressDSCP>>0x2; //dscp MSB 4 bits
				//tos |= (*pPktHdr->pTos)&0xf0;		//keep version 4 bits
				//*pPktHdr->pTos=tos;
				*pPktHdr->pTos = (pPktHdr->egressDSCP>>0x2)|((*pPktHdr->pTos)&0xf0);

				//tos = (pPktHdr->egressDSCP&0x3)<<0x6;	//dscp LSB 2 bits
				//tos |= (*(pPktHdr->pTos+1))&0x3f;		//keep original traffic label LSB 2 bits and flow label MSB 4 bits
				//*(pPktHdr->pTos+1)=tos;
				*(pPktHdr->pTos+1) = ((pPktHdr->egressDSCP&0x3)<<0x6)|((*(pPktHdr->pTos+1))&0x3f);
			}else if(pPktHdr->tagif&IPV4_TAGIF){
				//tos = pPktHdr->egressDSCP<<0x2;
				//tos |= (*pPktHdr->pTos)&0x3;		//keep 2 bits from LSB
				//*pPktHdr->pTos=tos; 	//remarking tos of packet
				*pPktHdr->pTos = (pPktHdr->egressDSCP<<0x2)|((*pPktHdr->pTos)&0x3);
			}
		}
	}else{
		ret=_rtk_rg_fwdEngineDMAC2CVIDTransfer(pPktHdr->skb,pPktHdr);
		if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;
	}

	if(pPktHdr->egressMACPort==RTK_RG_PORT_CPU) 
	{
#ifdef CONFIG_RG_WLAN_HWNAT_ACCELERATION		
		return _rtk_rg_fwdEngineWIFITransfer(pPktHdr->skb,pPktHdr);
#else
		//20151130LUKE: forward to CPU here!
		pPktHdr->fwdDecision=RG_FWD_DECISION_TO_PS;
		return RG_FWDENGINE_RET_TO_PS;
#endif
	}


	
	return RG_FWDENGINE_RET_DIRECT_TX;	
}

__IRAM_FWDENG
rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngineDirectTx(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	//VLAN and Priority can retrived from shortcut. if match!!
	if(pPktHdr->shortcutStatus!=RG_SC_MATCH){	//normal path
		_rtk_rg_interfaceVlanIDPriority_directTX(pPktHdr,&rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo,&rg_kernel.txDesc,&rg_kernel.txDescMask);
	} 

#if defined(CONFIG_RG_LAYER2_SOFTWARE_LEARN)
	//To Master Wifi or unknown DA will return here!
{
	rtk_rg_fwdEngineReturn_t ret;
	//To Master Wifi or unknown DA will return FAIL and just return here!
	ret=_rtk_rg_fwdEngineCheckDestination(pPktHdr);
	TRACE("Check Destination:%d",ret);
	switch(ret)
	{
		case RG_FWDENGINE_RET_SEND_TO_WIFI:
			return ret;
		case RG_FWDENGINE_RET_TO_PS:
			TRACE("To PS!");
			return ret;
		case RG_FWDENGINE_RET_DROP:
			TRACE("Drop!");
			return ret;
		default:
			break;
	}
}
#endif

	//20141210LUKE: recalculate unicast packets' IP-checksum for pppoe WAN if DSCP remarking
	//20150731LUKE:we don't use egressTagIf to check because bridge packet from protocol stack will be needed recalculate ckecksum if dscp remarking, either.
	if(pPktHdr->tagif&IPV4_TAGIF)*pPktHdr->pIpv4Checksum=htons(_rtk_rg_fwdengine_L3checksumUpdateDSCP(*pPktHdr->pIpv4Checksum, pPktHdr->ipv4HeaderLen, pPktHdr->tos, *pPktHdr->pTos));
	
	//clear old value
	rg_kernel.txDescMask.opts1.dw=0;
	rg_kernel.txDescMask.opts2.dw=0;
	rg_kernel.txDescMask.opts3.dw=0;
	rg_kernel.txDesc.opts1.dw=0;
	rg_kernel.txDesc.opts2.dw=0;
	rg_kernel.txDesc.opts3.dw=0;

	//turn on txInfo mask, otherwise value won't be add
	rg_kernel.txDescMask.opts1.bit.ipcs=1;
	rg_kernel.txDescMask.opts1.bit.cputag_ipcs=1;
	rg_kernel.txDescMask.opts1.bit.cputag_l4cs=1;
	rg_kernel.txDescMask.opts1.bit.dislrn=1;
	rg_kernel.txDescMask.opts1.bit.keep=1;
	rg_kernel.txDescMask.opts3.bit.l34_keep=1;
	rg_kernel.txDesc.opts1.bit.cputag_ipcs=0;	//disable switch L3 offload
	rg_kernel.txDesc.opts1.bit.cputag_l4cs=0;	//disable switch L4 offload
	//20141222LUKE: disable PPPoE L3 offload
	if(pPktHdr->egressTagif&PPPOE_TAGIF)
		rg_kernel.txDesc.opts1.bit.ipcs=0;
	else
		rg_kernel.txDesc.opts1.bit.ipcs=1;
	
	rg_kernel.txDesc.opts1.bit.dislrn=1;			//disable HW to check and learn SA, prevent port-moving to CPU
	rg_kernel.txDesc.opts1.bit.keep=1;				//20141104LUKE: when L34Keep is on, Keep is also needed for gpon.
	rg_kernel.txDesc.opts3.bit.l34_keep=1;			//ensure switch won't modify or filter packet

	//Assign tx port mask before return
	rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;	
	rg_kernel.txDesc.opts3.bit.tx_portmask=0x1<<pPktHdr->egressMACPort;

	return _rtk_rg_egressPacketSend(skb,pPktHdr);
}

void _rtk_rg_fwdEngineHwLookup(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	//clear old value
	rg_kernel.txDescMask.opts1.dw=0;
	rg_kernel.txDescMask.opts2.dw=0;
	rg_kernel.txDescMask.opts3.dw=0;
	rg_kernel.txDesc.opts1.dw=0;
	rg_kernel.txDesc.opts2.dw=0;
	rg_kernel.txDesc.opts3.dw=0;

	//turn on txInfo mask, otherwise value won't be add

	//2 REC20131203:from protocol stack packets, the CVLAN info will be carried by skb->vlan_tci,
	//2 therefore we should not override it by txDescMask and txDesc!!

	rg_kernel.txDescMask.opts1.bit.ipcs=1;
	rg_kernel.txDescMask.opts1.bit.l4cs=1;
	rg_kernel.txDescMask.opts1.bit.cputag_ipcs=1;
	rg_kernel.txDescMask.opts1.bit.cputag_l4cs=1;
	rg_kernel.txDescMask.opts1.bit.dislrn=1;
	rg_kernel.txDescMask.opts2.bit.tx_vlan_action=0x3;
	rg_kernel.txDescMask.opts2.bit.vidl=0xff;
	rg_kernel.txDescMask.opts2.bit.vidh=0xf;
	rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;	
	
	rg_kernel.txDesc.opts1.bit.cputag_ipcs=0;	//disable switch L3 offload
	rg_kernel.txDesc.opts1.bit.cputag_l4cs=0;	//disable switch L4 offload
	
	

	if(pPktHdr->ingressPort==RTK_RG_EXT_PORT0)
	{
		//rg_kernel.txDesc.opts2.bit.vidl=1;
		//rg_kernel.txDesc.opts2.bit.tx_vlan_action=3;
		rg_kernel.txDescMask.opts3.bit.extspa=0x7;
		rg_kernel.txDesc.opts3.bit.extspa=0x1;
		rg_kernel.txDesc.opts1.bit.dislrn=0;	// if packet is from extension port, must auto learn to hw.
	}
	else
	{
		rg_kernel.txDesc.opts1.bit.dislrn=1;		//disable HW to check and learn SA, prevent port-moving to CPU	
	}

	//20141222LUKE: disable PPPoE L3 offload
	if(pPktHdr->egressTagif&PPPOE_TAGIF)
		rg_kernel.txDesc.opts1.bit.ipcs=0;
	else
		rg_kernel.txDesc.opts1.bit.ipcs=1;

	//20140528LUKE:for ipv4, we recaculate Layer4 checksum if napt; for ipv6, we just keep the original value
	//20140902LUKE: for packet from shortcut won't be fragmented, so let hw do layer4 checksum.
	//20141203LUKE: for fagment packet, we shoud disable hw L4 checksum offload.
	//20141222LUKE: disable PPPoE L4 offload
	if((pPktHdr->egressTagif&PPPOE_TAGIF)||(pPktHdr->egressTagif&PPTP_TAGIF)||(pPktHdr->ipv6FragPacket)||(pPktHdr->ipv4FragPacket))
		rg_kernel.txDesc.opts1.bit.l4cs=0;	
	else
		rg_kernel.txDesc.opts1.bit.l4cs=1;

	//CPri remarking: Chuck
	//Patch20131122:GMAC disable deTAG function, therefore we just keep packet VLAN status "INTACT"!
#if 0
	if(pRxDesc->opts2.bit.ctagva){//original packet with vlanTag, keep original value. 
		//TRACE("Original CTag: 0x%x",pRxDesc->opts2.bit.cvlan_tag);
		rg_kernel.txDescMask.opts2.bit.tx_vlan_action=0x3;
		rg_kernel.txDescMask.opts2.bit.vidl=0xff;
		rg_kernel.txDescMask.opts2.bit.vidh=0xf;
		rg_kernel.txDescMask.opts2.bit.prio=0x7;
		rg_kernel.txDescMask.opts2.bit.cfi=0x1;

		rg_kernel.txDesc.opts2.bit.tx_vlan_action=0x3;//remark
		rg_kernel.txDesc.opts2.bit.vidh=((pRxDesc->opts2.bit.cvlan_tag))&0xf;
		rg_kernel.txDesc.opts2.bit.vidl=((pRxDesc->opts2.bit.cvlan_tag)>>8)&0xff;
		rg_kernel.txDesc.opts2.bit.prio=((pRxDesc->opts2.bit.cvlan_tag)>>5)&0x7;
		rg_kernel.txDesc.opts2.bit.cfi=((pRxDesc->opts2.bit.cvlan_tag))&0x1;

	}else{//original packet without vlan
		rg_kernel.txDescMask.opts2.bit.tx_vlan_action=0x3;
		rg_kernel.txDesc.opts2.bit.tx_vlan_action=0x2;//remove
	}
#else
	rg_kernel.txDesc.opts2.bit.tx_vlan_action=0x0;//intact
#endif

	//DSCP Remarking: Chuck
	//DSCP will handeled by HW Qos API. No need to modify.	
	
		
	/*DEBUG("before hardware lookup");
	dump_packet(skb->data,skb->len,"hardward lookup");
	DEBUG("txDesc.opts1 is %x, txDesc.opts2 is %x,txDesc.opts3 is %x",
		rg_kernel.txDesc.opts1.dw,rg_kernel.txDesc.opts2.dw,rg_kernel.txDesc.opts3.dw);
	DEBUG("txDescMask.opts1 is %x, txDescMask.opts2 is %x,txDescMask.opts3 is %x",
		rg_kernel.txDescMask.opts1.dw,rg_kernel.txDescMask.opts2.dw,rg_kernel.txDescMask.opts3.dw);*/
	_rtk_rg_egressPacketSend(skb,pPktHdr);
	//_rtk_rg_splitJumboSendToNicWithTxInfoAndMask(pPktHdr,skb,&rg_kernel.txDesc,0,&rg_kernel.txDescMask);
}

void _rtk_rg_fwdEngineLIMDBC(rtk_rg_pktHdr_t *pPktHdr)
{
	int i;
	//initialize
	pPktHdr->isGatewayPacket=0;
	
	/* lookup DA in netIf table */
	for(i=0;i<6/*MAX_NETIF_SW_TABLE_SIZE*/;i++)		//in Apollo L34 document said that we check first 6 entries only
	{
		if(rg_db.netif[i].rtk_netif.valid == 1)
		{
			if(memcmp(pPktHdr->pDmac,rg_db.netif[i].rtk_netif.gateway_mac.octet,ETHER_ADDR_LEN)==0)
			{
				pPktHdr->isGatewayPacket=1;
				//20140811LUKE: keep the ingress net interface!!
				pPktHdr->srcNetifIdx=i;
				break;
			}
		}
	}
}

#if 1
rtk_rg_successFailReturn_t _rtk_rg_lutReachLimit_clearARPNeighbor(int l2Idx, rtk_rg_saLearningLimitProbe_t *limitInfo)
{	
	//find all L34 table used this l2Idx, and clear them. Finally clear l2Idx also.
	MACLN("L2[%d] is deleting now!!",l2Idx);
	(pf.rtk_rg_macEntry_del)(l2Idx);
	
	atomic_set(&limitInfo->activity,0);
	return RG_RET_SUCCESS;
}

rtk_rg_successFailReturn_t _rtk_rg_lutReachLimit_sendARPAndModTimer(rtk_rg_saLearningLimitProbe_t *limitInfo)
{
	int intfIdx,routingIdx;
	ipaddr_t gwipAddr;
	
	limitInfo->arpReq.finished=0;
	limitInfo->arpReq.gwMacReqCallBack=NULL;
	limitInfo->arpReq.reqIp=limitInfo->v4IP;
	routingIdx=_rtk_rg_l3lookup(limitInfo->v4IP);
	intfIdx=rg_db.l3[routingIdx].rtk_l3.netifIdx;
	gwipAddr=rg_db.systemGlobal.interfaceInfo[intfIdx].storedInfo.lan_intf.ip_addr;
	_rtk_rg_arpGeneration(intfIdx,gwipAddr,&limitInfo->arpReq);
	//increase counter, modify timer, return
	limitInfo->arpCounter++;
	mod_timer(&limitInfo->timer, jiffies+(TICKTIME_PERIOD*CONFIG_RG_ACCESSWAN_TIMER_DELAY/1000));
	
	return RG_RET_SUCCESS;
}

rtk_rg_successFailReturn_t _rtk_rg_lutReachLimit_sendNeighborAndModTimer(rtk_rg_saLearningLimitProbe_t *limitInfo)
{
	int netIfIdx;
	rtk_ipv6_addr_t ipAddr;
	
	netIfIdx=rg_db.v6route[limitInfo->v6Route].rtk_v6route.nhOrIfidIdx;
	
	memcpy(ipAddr.ipv6_addr,rg_db.systemGlobal.interfaceInfo[netIfIdx].storedInfo.lan_intf.ipv6_addr.ipv6_addr,IPV6_ADDR_LEN);	
	limitInfo->neighborReq.finished=0;
	limitInfo->neighborReq.ipv6GwMacReqCallBack=NULL;
	memcpy(limitInfo->neighborReq.reqIp.ipv6_addr,limitInfo->v6IP.ipv6_addr,IPV6_ADDR_LEN);
	_rtk_rg_NDGeneration(netIfIdx,ipAddr,&limitInfo->neighborReq);
	//increase counter, modify timer, return
	limitInfo->neighborCounter++;
	mod_timer(&limitInfo->timer, jiffies+(TICKTIME_PERIOD*CONFIG_RG_ACCESSWAN_TIMER_DELAY/1000));	//one second
	return RG_RET_SUCCESS;
}

//SUCCESS: 1. ARP or Neighbor had been sent. 2. the MAC and ARP(Neighbor) had been cleared. 
//FAIL: this MAC is on-line, continue to next
rtk_rg_successFailReturn_t _rtk_rg_lutReachLimit_lookup(int l2Idx, rtk_rg_saLearningLimitProbe_t *limitInfo)
{
	int i,count;
	
	if(limitInfo->neighborIdx>=0)
	{
		if(limitInfo->neighborCounter==3)
		{
			//off-line, clear all ARP and Neighbor reference this MAC
			MACLN("the l2Idx[%d] maybe off-line...kick it out!!",l2Idx);
			return _rtk_rg_lutReachLimit_clearARPNeighbor(l2Idx,limitInfo);
		}
		if(limitInfo->neighborCounter!=-1)
		{
			//send Neighbor Discovery for the IPv6
			MACLN("ask the l2Idx[%d] for response by Neighbor Discovery...",l2Idx);
			return _rtk_rg_lutReachLimit_sendNeighborAndModTimer(limitInfo);
		}
		
		//reach here means MAC on-line, search another Neighbor
		goto Next_Neighbor;
	}

	if(limitInfo->arpIdx>=0)
	{
		MACLN("the arpIdx is %d, l2Idx is %d, counter is %d",limitInfo->arpIdx,l2Idx,limitInfo->arpCounter);
		//check counter
		if(limitInfo->arpCounter==3)
		{
			//off-line, clear all ARP and Neighbor reference this MAC
			MACLN("the l2Idx[%d] maybe off-line...kick it out!!",l2Idx);
			return _rtk_rg_lutReachLimit_clearARPNeighbor(l2Idx,limitInfo);
		}
		if(limitInfo->arpCounter!=-1)
		{
			//send ARP request for the IP
			MACLN("ask the l2Idx[%d] for response by ARP request...",l2Idx);
			return _rtk_rg_lutReachLimit_sendARPAndModTimer(limitInfo);
		}

		//reach here means MAC on-line, search another ARP
		MACLN("search another ARP...");
	}

	//lookup ARP for the l2Idx, if no ARP, goto Send_Neighbor
	for(i=limitInfo->arpIdx+1;i<MAX_ARP_SW_TABLE_SIZE;i++)
	{
		if(rg_db.arp[i].rtk_arp.valid && rg_db.arp[i].rtk_arp.nhIdx==l2Idx)
		{
			//Hit!!
			limitInfo->arpIdx=i;
			limitInfo->v4IP=rg_db.arp[i].ipv4Addr;
			limitInfo->arpCounter=0;
			//send ARP request for the IP
			MACLN("ARP[%d]->[%x] found! ask the l2Idx[%d] for response by ARP request...",i,rg_db.arp[i].ipv4Addr,l2Idx);
			return _rtk_rg_lutReachLimit_sendARPAndModTimer(limitInfo);
		}
	}
	MACLN("find new neighbor");

Next_Neighbor:
	//lookup Neighbor for the l2Idx, if no Neighbor, find next MAC
	for(i=limitInfo->neighborIdx+1;i<MAX_IPV6_NEIGHBOR_SW_TABLE_SIZE;i++)
	{
		if(rg_db.v6neighbor[i].rtk_v6neighbor.valid && rg_db.v6neighbor[i].rtk_v6neighbor.l2Idx==l2Idx)
		{
			//Hit!!
			limitInfo->neighborIdx=i;
			limitInfo->v6Route=rg_db.v6neighbor[i].rtk_v6neighbor.ipv6RouteIdx;
			//prefix 64bits from routing, postfix 64bits from neighbor
			memcpy(limitInfo->v6IP.ipv6_addr,rg_db.v6route[limitInfo->v6Route].rtk_v6route.ipv6Addr.ipv6_addr,8);
			for(count=0;count<8;count++)
				limitInfo->v6IP.ipv6_addr[count+8]=(rg_db.v6neighbor[i].rtk_v6neighbor.ipv6Ifid>>(56-(8*count)))&0xff;
			limitInfo->neighborCounter=0;
			//send Neighbor Discovery for the IPv6
			MACLN("Neighbor[%d]->[%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x] found! ask the l2Idx[%d] for response by Neighbor Discovery...",i,
				limitInfo->v6IP.ipv6_addr[0],limitInfo->v6IP.ipv6_addr[1],limitInfo->v6IP.ipv6_addr[2],limitInfo->v6IP.ipv6_addr[3],
				limitInfo->v6IP.ipv6_addr[4],limitInfo->v6IP.ipv6_addr[5],limitInfo->v6IP.ipv6_addr[6],limitInfo->v6IP.ipv6_addr[7],
				limitInfo->v6IP.ipv6_addr[8],limitInfo->v6IP.ipv6_addr[9],limitInfo->v6IP.ipv6_addr[10],limitInfo->v6IP.ipv6_addr[11],
				limitInfo->v6IP.ipv6_addr[12],limitInfo->v6IP.ipv6_addr[13],limitInfo->v6IP.ipv6_addr[14],limitInfo->v6IP.ipv6_addr[15],
				l2Idx);
			return _rtk_rg_lutReachLimit_sendNeighborAndModTimer(limitInfo);
		}
	}
	MACLN("no more arp or neighbor can ask..");

	//reach here means no ARP or Neighbor of this MAC any more
	//return _rtk_rg_lutReachLimit_clearARPNeighbor(l2Idx,limitInfo);
	limitInfo->arpIdx=-1;
	limitInfo->neighborIdx=-1;
	return RG_RET_FAIL;
	
}

void _rtk_rg_lutReachLimit_port(unsigned long spa)
{
	int i;
	rtk_rg_successFailReturn_t ret;
	unsigned int physpa=spa&0xffff;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0DevIdx=(spa>>16)&0xffff;

	if(physpa==RTK_RG_EXT_PORT0 && rg_db.systemGlobal.sourceAddrLearningLimitNumber[physpa]==DEF_SOFTWARE_LEARNING_LIMIT)
	{
		//ext port0 is not set as limit, so we should check by WLAN dev idx
		if(atomic_read(&rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx].activity)==0)
			return;
		
		//lookup LUT for MAC at same port, search ARP first and Neighbor later
		if(rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx].l2Idx!=-1)
		{
			MACLN("Lookup for l2Idx[%d] at WLAN0DEV[%ld]...",rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx].l2Idx,wlan0DevIdx);
			ret=_rtk_rg_lutReachLimit_lookup(rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx].l2Idx,&rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx]);
			if(ret==RG_RET_SUCCESS)
				return;
		}

		for(i=0;i<MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE;i++)
		{
			if(rg_db.lut[i].valid && rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC && !(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC) &&
				rg_db.lut[i].rtk_lut.entry.l2UcEntry.port==RTK_RG_PORT_CPU && rg_db.lut[i].rtk_lut.entry.l2UcEntry.ext_port==(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU) &&
				rg_db.lut[i].wlan_device_idx==wlan0DevIdx && i!=rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx].l2Idx)
			{
				//Match same port!
				MACLN("Lookup for l2Idx[%d] at DEV[%ld]...",i,wlan0DevIdx);
				rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx].l2Idx=i;
				ret=_rtk_rg_lutReachLimit_lookup(i,&rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx]);
				if(ret==RG_RET_SUCCESS)
					break;
			}
		}

		if(atomic_read(&rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx].activity)==1)
		{
			ret=RG_RET_FAIL;
			for(i=MAX_LUT_HW_TABLE_SIZE;i<MAX_LUT_SW_TABLE_SIZE;i++)
			{
				if(rg_db.lut[i].valid && rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC)
				{
					if(rg_db.lut[i].rtk_lut.entry.l2UcEntry.port==RTK_RG_PORT_CPU &&
						rg_db.lut[i].rtk_lut.entry.l2UcEntry.ext_port==(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))
					{
						if(wlan0DevIdx==rg_db.lut[i].wlan_device_idx)
						{
							ret=RG_RET_SUCCESS;
							break;
						}
					}
				}
			}	
			if(ret==RG_RET_FAIL)
				atomic_set(&rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx].activity,0);
			else
				mod_timer(&rg_kernel.lutReachLimit_wlan0dev[wlan0DevIdx].timer, jiffies+(TICKTIME_PERIOD*CONFIG_RG_ACCESSWAN_TIMER_DELAY/1000));	//not found.. and still have software LUTwaiting...repeat after one second
		}
	}
	else
#endif		
	{
		if(atomic_read(&rg_kernel.lutReachLimit_port[physpa].activity)==0)
			return;
		
		//lookup LUT for MAC at same port, search ARP first and Neighbor later
		if(rg_kernel.lutReachLimit_port[physpa].l2Idx!=-1)
		{
			MACLN("Lookup for l2Idx[%d] at PORT[%ld]...",rg_kernel.lutReachLimit_port[physpa].l2Idx,physpa);
			ret=_rtk_rg_lutReachLimit_lookup(rg_kernel.lutReachLimit_port[physpa].l2Idx,&rg_kernel.lutReachLimit_port[physpa]);
			if(ret==RG_RET_SUCCESS)
				return;
		}

		if(physpa>=RTK_RG_PORT_CPU)
		{
			for(i=0;i<MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE;i++)
			{
				if(rg_db.lut[i].valid && rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC && !(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC) &&
					rg_db.lut[i].rtk_lut.entry.l2UcEntry.port==RTK_RG_PORT_CPU && rg_db.lut[i].rtk_lut.entry.l2UcEntry.ext_port==physpa-RTK_RG_PORT_CPU &&
					i!=rg_kernel.lutReachLimit_port[physpa].l2Idx)
				{
					//Match same port!
					MACLN("Lookup for l2Idx[%d] at PORT[%ld]...",i,physpa);
					rg_kernel.lutReachLimit_port[physpa].l2Idx=i;
					ret=_rtk_rg_lutReachLimit_lookup(i,&rg_kernel.lutReachLimit_port[physpa]);
					if(ret==RG_RET_SUCCESS)
						break;
				}
			}
		}
		else
		{
			for(i=0;i<MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE;i++)
			{
				if(rg_db.lut[i].valid && rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC && 
					rg_db.lut[i].rtk_lut.entry.l2UcEntry.port==physpa && i!=rg_kernel.lutReachLimit_port[physpa].l2Idx)
				{
					//Match same port!
					MACLN("Lookup for l2Idx[%d] at PORT[%ld]...",i,physpa);
					rg_kernel.lutReachLimit_port[physpa].l2Idx=i;
					ret=_rtk_rg_lutReachLimit_lookup(i,&rg_kernel.lutReachLimit_port[physpa]);
					if(ret==RG_RET_SUCCESS)
						break;
				}
			}
		}

		if(atomic_read(&rg_kernel.lutReachLimit_port[physpa].activity)==1)
		{
			ret=FAIL;
			for(i=MAX_LUT_HW_TABLE_SIZE;i<MAX_LUT_SW_TABLE_SIZE;i++)
			{
				if(rg_db.lut[i].valid && rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC)
				{
					if(rg_db.lut[i].rtk_lut.entry.l2UcEntry.port==RTK_RG_PORT_CPU)
					{
						if(physpa==rg_db.lut[i].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU)
						{
							ret=RG_RET_SUCCESS;
							break;
						}
					}
					else if(rg_db.lut[i].rtk_lut.entry.l2UcEntry.port==physpa)
					{			
						ret=RG_RET_SUCCESS;
						break;
					}
				}
			}	
			if(ret==RG_RET_FAIL)
				atomic_set(&rg_kernel.lutReachLimit_port[physpa].activity,0);
			else
				mod_timer(&rg_kernel.lutReachLimit_port[physpa].timer, jiffies+(TICKTIME_PERIOD*CONFIG_RG_ACCESSWAN_TIMER_DELAY/1000));	//not found.. and still have software LUTwaiting...repeat after one second
		}
	}
	return;
}

void _rtk_rg_lutReachLimit_portmask(unsigned long portmsk)
{
	int i=0,ret;
	unsigned int phyPortmask=portmsk&0xffff;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	unsigned int wlan0Devmask=(portmsk>>16)&0xffff;
#endif
	
	if(atomic_read(&rg_kernel.lutReachLimit_portmask.activity)==0)
		return;
	
	//lookup LUT for MAC at all port under system port-mask, search ARP first and Neighbor later
	if(rg_kernel.lutReachLimit_portmask.l2Idx!=-1)
	{
		MACLN("Lookup for previous-l2Idx[%d] at PORTMASK[%lx]...",rg_kernel.lutReachLimit_portmask.l2Idx,phyPortmask);
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		MACLN("Also lookup for previous-l2Idx[%d] at WLAN0DEVMASK[%lx]...",rg_kernel.lutReachLimit_portmask.l2Idx,wlan0Devmask);
#endif
		ret=_rtk_rg_lutReachLimit_lookup(rg_kernel.lutReachLimit_portmask.l2Idx,&rg_kernel.lutReachLimit_portmask);
		if(ret==SUCCESS)
			return;
		else
		{
			i=rg_kernel.lutReachLimit_portmask.l2Idx+1;
			rg_kernel.lutReachLimit_portmask.l2Idx=-1;
		}
	}

	for(;i<MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE;i++)
	{
		if(rg_db.lut[i].valid && rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC && !(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC))
		{
			if(rg_db.lut[i].rtk_lut.entry.l2UcEntry.port==RTK_RG_PORT_CPU)
			{
				if(phyPortmask&(0x1<<(rg_db.lut[i].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU)))
				{
					//Match system port-mask!
					MACLN("Lookup for l2Idx[%d] of ExtPORT[%d] at PORTMASK[%lx]...",i,rg_db.lut[i].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU,phyPortmask);
					rg_kernel.lutReachLimit_portmask.l2Idx=i;
					ret=_rtk_rg_lutReachLimit_lookup(i,&rg_kernel.lutReachLimit_portmask);
					if(ret==SUCCESS)
						break;
				}
#ifdef CONFIG_MASTER_WLAN0_ENABLE
				else if(rg_db.lut[i].rtk_lut.entry.l2UcEntry.ext_port==(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))
				{
					if(wlan0Devmask&(0x1<<rg_db.lut[i].wlan_device_idx))
					{
						//Match system port-mask!
						MACLN("Lookup for l2Idx[%d] of WLAN0DEV[%d] at DEVMASK[%lx]...",i,rg_db.lut[i].wlan_device_idx,wlan0Devmask);
						rg_kernel.lutReachLimit_portmask.l2Idx=i;
						ret=_rtk_rg_lutReachLimit_lookup(i,&rg_kernel.lutReachLimit_portmask);
						if(ret==SUCCESS)
							break;
					}
				}
#endif				
			}
			else if(phyPortmask&(0x1<<(rg_db.lut[i].rtk_lut.entry.l2UcEntry.port)))
			{
				//Match system port-mask!
				MACLN("Lookup for l2Idx[%d] of PORT[%d] at PORTMASK[%lx]...",i,rg_db.lut[i].rtk_lut.entry.l2UcEntry.port,phyPortmask);
				rg_kernel.lutReachLimit_portmask.l2Idx=i;
				ret=_rtk_rg_lutReachLimit_lookup(i,&rg_kernel.lutReachLimit_portmask);
				if(ret==SUCCESS)
					break;
			}
		}
	}

	if(i==MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)
	{
		atomic_inc(&rg_kernel.lutReachLimit_portmask.activity);
		MACLN("#### activity is %d",atomic_read(&rg_kernel.lutReachLimit_portmask.activity)-1);
	}

	if(atomic_read(&rg_kernel.lutReachLimit_portmask.activity)>0)
	{
		ret=FAIL;
		for(i=MAX_LUT_HW_TABLE_SIZE;i<MAX_LUT_SW_TABLE_SIZE;i++)
		{
			if(rg_db.lut[i].valid && rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC)
			{
				//portmask may change, so we have to use global variable to check!
				if(rg_db.lut[i].rtk_lut.entry.l2UcEntry.port==RTK_RG_PORT_CPU)
				{
					if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(rg_db.lut[i].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU)))
					{
						ret=SUCCESS;
						break;
					}
#ifdef CONFIG_MASTER_WLAN0_ENABLE
					else if(rg_db.lut[i].rtk_lut.entry.l2UcEntry.ext_port==(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))
					{
						if(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member&(0x1<<rg_db.lut[i].wlan_device_idx))
						{
							ret=SUCCESS;
							break;
						}
					}
#endif	
				}
				else if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(rg_db.lut[i].rtk_lut.entry.l2UcEntry.port)))
				{			
					ret=SUCCESS;
					break;
				}
			}
		}	
		if(ret==FAIL || atomic_read(&rg_kernel.lutReachLimit_portmask.activity)>MAX_WanAccessARPCount)
			atomic_set(&rg_kernel.lutReachLimit_portmask.activity,0);
		else
			mod_timer(&rg_kernel.lutReachLimit_portmask.timer, jiffies+(TICKTIME_PERIOD*CONFIG_RG_ACCESSWAN_TIMER_DELAY/1000));	//not found.. and still have software LUTwaiting...repeat after one second
	}
	
	return;
}

void _rtk_rg_lutReachLimit_category(unsigned long category)
{
	int i=0,ret;

	if(atomic_read(&rg_kernel.lutReachLimit_category[category].activity)==0)
		return;
	
	//lookup LUT for MAC at same category, search ARP first and Neighbor later
	if(rg_kernel.lutReachLimit_category[category].l2Idx!=-1)
	{
		MACLN("Lookup for previous-l2Idx[%d] at CATEGORY[%ld]...",rg_kernel.lutReachLimit_category[category].l2Idx,category);
		ret=_rtk_rg_lutReachLimit_lookup(rg_kernel.lutReachLimit_category[category].l2Idx,&rg_kernel.lutReachLimit_category[category]);
		if(ret==SUCCESS)
			return;
		else
		{
			i=rg_kernel.lutReachLimit_category[category].l2Idx+1;
			rg_kernel.lutReachLimit_category[category].l2Idx=-1;
		}
	}

	for(;i<MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE;i++)
	{
		if(rg_db.lut[i].valid && rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC && !(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC) && rg_db.lut[i].category==category)
		{			
			//Match same category!
			MACLN("Lookup for l2Idx[%d] at CATEGORY[%ld]...",i,category);
			rg_kernel.lutReachLimit_category[category].l2Idx=i;
			ret=_rtk_rg_lutReachLimit_lookup(i,&rg_kernel.lutReachLimit_category[category]);
			if(ret==SUCCESS)
				break;
		}
	}

	if(i==MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE)
	{
		atomic_inc(&rg_kernel.lutReachLimit_category[category].activity);
		MACLN("#### activity is %d",atomic_read(&rg_kernel.lutReachLimit_category[category].activity)-1);
	}

	if(atomic_read(&rg_kernel.lutReachLimit_category[category].activity)>0)
	{
		ret=FAIL;
		for(i=MAX_LUT_HW_TABLE_SIZE;i<MAX_LUT_SW_TABLE_SIZE;i++)
		{
			if(rg_db.lut[i].valid && rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC && rg_db.lut[i].category==category)
			{
				ret=SUCCESS;
				break;
			}
		}

		if(ret==FAIL || atomic_read(&rg_kernel.lutReachLimit_category[category].activity)>MAX_WanAccessARPCount)
			atomic_set(&rg_kernel.lutReachLimit_category[category].activity,0);
		else
			mod_timer(&rg_kernel.lutReachLimit_category[category].timer, jiffies+(TICKTIME_PERIOD*CONFIG_RG_ACCESSWAN_TIMER_DELAY/1000));	//not found.. and still have software LUTwaiting...repeat after one second
	}
	
	return;
}

void _rtk_rg_lutReachLimit_init(rtk_rg_accessWanLimitType_t type, void (*function)(unsigned long), unsigned long data)
{
	rtk_rg_saLearningLimitProbe_t *limitInfo=NULL;

	switch(type)
	{
		case RG_ACCESSWAN_TYPE_PORT:
#ifdef CONFIG_MASTER_WLAN0_ENABLE
			if(((data>>16)&0xffff)>0)
			{
				MACLN("#### Start to check all MACs at WLAN0DEV[%ld]",data>>16);
				limitInfo=&rg_kernel.lutReachLimit_wlan0dev[data>>16];
			}
			else
#endif
			{
				MACLN("#### Start to check all MACs at PORT[%ld]",data);
				limitInfo=&rg_kernel.lutReachLimit_port[data];
			}
			break;
		case RG_ACCESSWAN_TYPE_PORTMASK:
#ifdef CONFIG_MASTER_WLAN0_ENABLE
			if(((data>>16)&0xffff)>0)
			{
				MACLN("#### Start to check all MACs at WLAN0DEVMASK[%ld]",data>>16);
			}
			else
#endif			
			{
				MACLN("#### Start to check all MACs at PORTMASK[%lx]",data);
			}
			limitInfo=&rg_kernel.lutReachLimit_portmask;
			break;
		case RG_ACCESSWAN_TYPE_CATEGORY:
			MACLN("#### Start to check all MACs at same CATEGORY[%ld]",data);
			limitInfo=&rg_kernel.lutReachLimit_category[data];
			break;
		default:
			break;
	}

	if(limitInfo==NULL)
		return;
		
	if(atomic_read(&limitInfo->activity)==0)
	{
		if(timer_pending(&limitInfo->timer))
			del_timer(&limitInfo->timer);
		init_timer(&limitInfo->timer);
		limitInfo->timer.function = function;
		limitInfo->timer.data = data;
		
		limitInfo->l2Idx=-1;
		limitInfo->arpIdx=-1;
		limitInfo->arpCounter=0;
		memset(&limitInfo->arpReq,0,sizeof(rtk_rg_arp_request_t));
		limitInfo->neighborIdx=-1;
		limitInfo->neighborCounter=0;
		memset(&limitInfo->neighborReq,0,sizeof(rtk_rg_neighbor_discovery_t));
		limitInfo->v4IP=0;
		memset(&limitInfo->v6IP,0,sizeof(rtk_ipv6_addr_t));
		limitInfo->v6Route=-1;
		atomic_set(&limitInfo->activity,1);
		
		mod_timer(&limitInfo->timer, jiffies+(TICKTIME_PERIOD*CONFIG_RG_ACCESSWAN_TIMER_DELAY/1000));	//one second
	}
	else
		MACLN("#### The limit is active now...");
}

rtk_rg_successFailReturn_t _rtk_rg_softwareLut_addFromHw(int l2Idx, char category)
{
	rtk_rg_lut_linkList_t *pLutListEntry,*pLutListNextEntry;
	rtk_l2_ucastAddr_t *softLut;
	int hashIdx=l2Idx>>2;
	
	//Check if we have not-used free arp list
	if(list_empty(&rg_db.softwareLutFreeListHead))
	{
		FIXME("all free LUT list are allocated...");
			return RG_RET_FAIL;
	}

	//Get one from free list
	list_for_each_entry_safe(pLutListEntry,pLutListNextEntry,&rg_db.softwareLutFreeListHead,lut_list) 	//just return the first entry right behind of head
	{	
		list_del_init(&pLutListEntry->lut_list);
		break;
	}
	DEBUG("the free LUT %p idx is %d",pLutListEntry,pLutListEntry->idx);

	softLut=&rg_db.lut[pLutListEntry->idx].rtk_lut.entry.l2UcEntry;

	//Setup LUT information
	memcpy(softLut,&rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry,sizeof(rtk_l2_ucastAddr_t));
	rg_db.lut[pLutListEntry->idx].rtk_lut.entryType=RTK_LUT_L2UC;
	rg_db.lut[pLutListEntry->idx].valid=1;
	rg_db.lut[pLutListEntry->idx].category=category;
	rg_db.lut[pLutListEntry->idx].wlan_device_idx=rg_db.lut[l2Idx].wlan_device_idx;
	rg_db.lut[pLutListEntry->idx].fix_l34_vlan=rg_db.lut[l2Idx].fix_l34_vlan;

	//Add to hash head list
	list_add(&pLutListEntry->lut_list,&rg_db.softwareLutTableHead[hashIdx]);

	return RG_RET_SUCCESS;
}


rtk_rg_successFailReturn_t _rtk_rg_softwareLut_add(rtk_rg_macEntry_t *macEntry, int hashIdx, char category, char wlan_dev_index)
{
	rtk_rg_lut_linkList_t *pLutListEntry,*pLutListNextEntry;
	rtk_l2_ucastAddr_t *softLut;
	
	//Check if we have not-used free arp list
	if(list_empty(&rg_db.softwareLutFreeListHead))
	{
		FIXME("all free LUT list are allocated...");
			return RG_RET_FAIL;
	}

	//Get one from free list
	list_for_each_entry_safe(pLutListEntry,pLutListNextEntry,&rg_db.softwareLutFreeListHead,lut_list) 	//just return the first entry right behind of head
	{	
		list_del_init(&pLutListEntry->lut_list);
		break;
	}
	DEBUG("the free LUT %p idx is %d",pLutListEntry,pLutListEntry->idx);

	softLut=&rg_db.lut[pLutListEntry->idx].rtk_lut.entry.l2UcEntry;

	//Setup LUT information
	memcpy(softLut->mac.octet,macEntry->mac.octet,ETHER_ADDR_LEN);
	softLut->fid=macEntry->fid;
	softLut->vid=macEntry->vlan_id;
	softLut->age=7;
	
	if(macEntry->port_idx>=RTK_RG_PORT_CPU)
	{
		softLut->port=RTK_RG_PORT_CPU;
		softLut->ext_port=macEntry->port_idx-RTK_RG_PORT_CPU;
#ifdef CONFIG_DUALBAND_CONCURRENT
#if 1 	//20130722: if the packet is from ext0, this packet will hit ACL rule.(modify dpmask to 8)
		// the GMAC hw will reference CPU_RRING_ROUTING.		
		if(macEntry->port_idx==RTK_RG_EXT_PORT1)
		{
			//lut.flags|=(RTK_L2_UCAST_FLAG_FWD_PRI|RTK_L2_UCAST_FLAG_STATIC); // must set static becasue unknow DA can't forward to CPU2(trap to CPU1 again)
			softLut->flags|=(RTK_L2_UCAST_FLAG_FWD_PRI); //2013071
			softLut->priority=CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI;		
		}
#endif		
#endif		
	}
	else
	{
		softLut->port=macEntry->port_idx;
	}

	if(macEntry->static_entry)
		softLut->flags|=RTK_L2_UCAST_FLAG_STATIC; 
	if(macEntry->isIVL)
		softLut->flags|=RTK_L2_UCAST_FLAG_IVL; 
	if(macEntry->arp_used)
		softLut->flags|=RTK_L2_UCAST_FLAG_ARP_USED;
#if defined(CONFIG_RTL9602C_SERIES)
	if(macEntry->ctag_if)
		softLut->flags|=RTK_L2_UCAST_FLAG_CTAG_IF;
	else
		softLut->flags&=(~RTK_L2_UCAST_FLAG_CTAG_IF);
#endif

	rg_db.lut[pLutListEntry->idx].rtk_lut.entryType=RTK_LUT_L2UC;
	rg_db.lut[pLutListEntry->idx].valid=1;
	//20160113LUKE: if we had registered default URL for redirect, set redirect_http_req of lut in LAN.
	if(((rg_db.systemGlobal.forcePortal_url_list[0].valid)||(rg_db.redirectHttpAll.enable))&&(macEntry->port_idx!=RTK_RG_PORT_CPU)&&(rg_db.systemGlobal.lanPortMask.portmask&(0x1<<macEntry->port_idx)))
		rg_db.lut[pLutListEntry->idx].redirect_http_req=1;
	rg_db.lut[pLutListEntry->idx].category=category;
	rg_db.lut[pLutListEntry->idx].wlan_device_idx=wlan_dev_index;
	rg_db.lut[pLutListEntry->idx].fix_l34_vlan=macEntry->fix_l34_vlan;

	//Add to hash head list
	list_add(&pLutListEntry->lut_list,&rg_db.softwareLutTableHead[hashIdx]);

	return RG_RET_SUCCESS;
}

rtk_rg_successFailReturn_t _rtk_rg_softwareLut_checkAndDelete(rtk_l2_ucastAddr_t *lutEntry, int hashIdx)
{
	rtk_rg_lut_linkList_t *pSoftLut,*pSoftLutNext;
	
	//Check if we had been add to software LUT link-list
	if(!list_empty(&rg_db.softwareLutTableHead[hashIdx]))
	{
		list_for_each_entry_safe(pSoftLut,pSoftLutNext,&rg_db.softwareLutTableHead[hashIdx],lut_list)
		{
			if(memcmp(rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.mac.octet,lutEntry->mac.octet,ETHER_ADDR_LEN)==0)
			{
				if(((lutEntry->flags&RTK_L2_UCAST_FLAG_IVL)>0 && rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.vid==lutEntry->vid) ||
				((lutEntry->flags&RTK_L2_UCAST_FLAG_IVL)==0 && rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.fid==lutEntry->fid))
				{
					if(rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.port!=lutEntry->port ||
						rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.ext_port!=lutEntry->ext_port)
					{
						MACLN("software LUT had been added before and port-moving....delete it!");
						//Delete from head list
						list_del_init(&pSoftLut->lut_list);

						//set lut invalid(quicker than set all data to zero)
						rg_db.lut[pSoftLut->idx].valid=0;

						//Add back to free list
						list_add(&pSoftLut->lut_list,&rg_db.softwareLutFreeListHead);
						return RG_RET_SUCCESS;
					}
				}
			}
		}
	}

	return RG_RET_FAIL;
}

rtk_rg_successFailReturn_t _rtk_rg_softwareLut_checkAllAndDelete(rtk_l2_ucastAddr_t *lutEntry)
{
	int i;
	rtk_rg_successFailReturn_t ret;
	
	//Check if we had been add to software LUT in all link-list-head
	for(i=0;i<MAX_LUT_SW_TABLE_HEAD;i++)
	{
		ret=_rtk_rg_softwareLut_checkAndDelete(lutEntry,i);
		if(ret==RG_RET_SUCCESS)
			return ret;
	}

	return RG_RET_FAIL;
}

void _rtk_rg_softwareLut_allDelete(void)
{
	int i;
	rtk_rg_lut_linkList_t *pSoftLut,*pSoftLutNext;
	
	//Check if we had been add to software LUT in all link-list-head
	for(i=0;i<MAX_LUT_SW_TABLE_HEAD;i++)
	{	
		if(!list_empty(&rg_db.softwareLutTableHead[i]))
		{
			list_for_each_entry_safe(pSoftLut,pSoftLutNext,&rg_db.softwareLutTableHead[i],lut_list)
			{				
				//Delete from head list
				list_del_init(&pSoftLut->lut_list);

				//set lut invalid(quicker than set to zero)
				rg_db.lut[pSoftLut->idx].valid=0;

				//Add back to free list
				list_add(&pSoftLut->lut_list,&rg_db.softwareLutFreeListHead);
			}
		}
	}

}

rtk_rg_fwdEngineReturn_t _rtk_rg_checkGwIp(rtk_rg_pktHdr_t *pPktHdr)
{
	int i;
	for(i=0;i<rg_db.systemGlobal.lanIntfTotalNum;i++)
	{
		if(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo!=NULL)
		{
			if(((pPktHdr->tagif&IPV4_TAGIF||pPktHdr->tagif&ARP_TAGIF) && rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->ip_addr==pPktHdr->ipv4Dip) ||
				(pPktHdr->tagif&IPV6_TAGIF && !memcmp(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->ipv6_addr.ipv6_addr,pPktHdr->pIpv6Dip,IPV6_ADDR_LEN)))
			{
				TRACE("Packet to LAN interface %d, to PS!",rg_db.systemGlobal.lanIntfGroup[i].index);						
				return RG_FWDENGINE_RET_TO_PS;
			}
		}
	}

	//Check Wan interface IP address
	for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
	{
		//DEBUG("the wan type is %d, ip is %x",rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type,rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ip_addr);
		if(rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo!=NULL && rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf!=NULL &&
			rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type!=RTK_RG_BRIDGE)
		{
			if(((pPktHdr->tagif&IPV4_TAGIF||pPktHdr->tagif&ARP_TAGIF) && rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ip_addr==pPktHdr->ipv4Dip) || 
				(pPktHdr->tagif&IPV6_TAGIF && !memcmp(rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ipv6_addr.ipv6_addr,pPktHdr->pIpv6Dip,IPV6_ADDR_LEN)))
			{
				TRACE("Packet to WAN interface %d, to PS!",rg_db.systemGlobal.wanIntfGroup[i].index);
				//dump_packet(skb->data,skb->len,"arp packet");
#ifdef CONFIG_RG_SIMPLE_PROTOCOL_STACK
				if(pPktHdr->cp!=NULL)pPktHdr->cp->wanInterfaceIdx=i;
#endif				
				return RG_FWDENGINE_RET_TO_PS;	
			}
		}
	}

	return RG_FWDENGINE_RET_CONTINUE;
}

rtk_rg_successFailReturn_t _rtk_rg_layer2LutLearning(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr,rtk_rg_port_idx_t spa)
{
	rtk_rg_macEntry_t macEntry={{{0}},0,0,0,0,0,0,0,0};
	rtk_rg_lut_linkList_t *pSoftLut,*pSoftLut_hit=NULL;
	int i,ret,l2Idx,search_index,count=0,first_invalid=-1,port_move_orig=-1,category_orig=0,lut_orig=-1,wlan_move_orig=-1;
	char limit_action_mask=0,matchLanPort=0,matchLanMac=0,matchLanIP=0,check_category=0;

	//rtk_rg_port_idx_t spa=pPktHdr->pRxDesc->opts3.bit.src_port_num;
	//DEBUG("spa=%d SMAC=%02x:%02x:%02x:%02x:%02x:%02x\n",spa,skb->data[6],skb->data[7],skb->data[8],skb->data[9],skb->data[10],skb->data[11]);

	//DEBUG("@@@ port%d %02x:%02x:%02x:%02x:%02x:%02x @@@\n",pPktHdr->pRxDesc->opts3.bit.src_port_num,pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],
		//pPktHdr->pSmac[3],pPktHdr->pSmac[4],pPktHdr->pSmac[5]);
	
	if(rg_db.vlan[pPktHdr->internalVlanID].valid==0)		//unknown 1Q vlan should't learn
	{
		DEBUG("internalVID=%d valid=%d\n",pPktHdr->internalVlanID,rg_db.vlan[pPktHdr->internalVlanID].valid);
		return RG_RET_FAIL;
	}
	macEntry.vlan_id=pPktHdr->internalVlanID;
	macEntry.fid=rg_db.vlan[macEntry.vlan_id].fid;
	//DEBUG("the internalVlanID is %d, fid is %d",macEntry.vlan_id,macEntry.fid);
#if 1

	if(/*macEntry.vlan_id!=0 && */rg_db.vlan[macEntry.vlan_id].fidMode==VLAN_FID_IVL)
	{
		macEntry.isIVL=1;
		l2Idx=_rtk_rg_hash_mac_vid_efid(pPktHdr->pSmac,macEntry.vlan_id,0);		//FIXME:EFID is 0 now
	}
	else
	{
ADD_SVL_LUT:
		count=0;
		macEntry.isIVL=0;
		first_invalid=-1;
		macEntry.vlan_id=pPktHdr->ctagVid; //leo:add
#if defined(CONFIG_RTL9602C_SERIES)
		macEntry.ctag_if=(pPktHdr->tagif&CVLAN_TAGIF)?1:0;
		if ((pPktHdr->tagif&CVLAN_TAGIF)==0) //leo:add
		{	//leo:add
			macEntry.vlan_id=DEFAULT_CPU_VLAN; //leo:add
		}
#else
		if((pPktHdr->tagif&CVLAN_TAGIF)==0)
			macEntry.vlan_id=0;		//untag
#endif
		//if(rg_db.vlan[macEntry.vlan_id].UntagPortmask.bits[0]&(0x1<<pPktHdr->pRxDesc->opts3.bit.src_port_num))macEntry.vlan_id=0;		//untag
		l2Idx=_rtk_rg_hash_mac_fid_efid(pPktHdr->pSmac,macEntry.fid,0);			//FIXME:EFID is 0 now
	}

	l2Idx<<=2;
	do
	{
		search_index = l2Idx+count;
		//DEBUG("search_idx is %d\n",search_index);
		if(rg_db.lut[search_index].valid==0)
		{
			if(first_invalid==-1)
				first_invalid=search_index;
			//break;	//empty, just add
			count++; //search from next entry
			continue;
		}

		if(rg_db.lut[search_index].valid && rg_db.lut[search_index].rtk_lut.entryType==RTK_LUT_L2UC &&
			(memcmp(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.mac.octet,pPktHdr->pSmac,ETHER_ADDR_LEN)==0))
		{
			if(((macEntry.isIVL==1) && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid==macEntry.vlan_id) ||
			((macEntry.isIVL==0) && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.fid==macEntry.fid))
			{
				if(pPktHdr->pRxDesc->opts3.bit.reason==192 /* SA learning limit */ || pPktHdr->pRxDesc->opts3.bit.reason==196 /* unknown SA action */)
				{
					if((spa<RTK_RG_PORT_CPU && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port==spa) || 
						(spa>=RTK_RG_PORT_CPU && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port==(spa-RTK_RG_PORT_CPU)))
					{
						//Force SA learning
						TRACE("Forced SA learning...reason=%d",pPktHdr->pRxDesc->opts3.bit.reason);
						if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(spa)))
							atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
						atomic_dec(&rg_db.systemGlobal.sourceAddrLearningCount[spa]);
						//decrease wlan's device count				
						if(spa==RTK_RG_EXT_PORT0)
						{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
							atomic_dec(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[pPktHdr->wlan_dev_idx]);
#endif
						}
						port_move_orig=spa;
						category_orig=rg_db.lut[search_index].category;
						break;
					}
				}
				//DEBUG("match!!fix_l34_vlan:%d",rg_db.lut[search_index].fix_l34_vlan);
				//FIXME: here reserved for WiFi interface may also need to handle port-moving in the future.	
				if(	!(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC))
				{
					//20140722LUKE: drop strange packet learned not in WAN but comeback in WAN
					if((spa<RTK_RG_PORT_CPU && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port!=spa) || 
						(spa>=RTK_RG_PORT_CPU && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port!=(spa-RTK_RG_PORT_CPU)))
						if(((0x1<<spa)&rg_db.systemGlobal.wanPortMask.portmask)&&(rg_db.systemGlobal.strangeSA_drop==RG_HWNAT_ENABLE))
							return RG_RET_FAIL;
						
					if((spa<RTK_RG_PORT_CPU && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port!=spa) || 
						(spa>=RTK_RG_PORT_CPU && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port!=(spa-RTK_RG_PORT_CPU)) ||
						(macEntry.vlan_id!=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid && !rg_db.lut[search_index].fix_l34_vlan) ||
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#else	//support ctag_if
						(macEntry.ctag_if!=((rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_CTAG_IF)?1:0) && !rg_db.lut[search_index].fix_l34_vlan) ||
#endif
						(spa==RTK_RG_EXT_PORT0 && pPktHdr->wlan_dev_idx!=rg_db.lut[search_index].wlan_device_idx))
					{
						//Mac port-moving or Vlan-moving or wlan-moving, update LUT table without change ARP_USED flag and auth field
						//------------------ Critical Section start -----------------------//
						//rg_lock(&rg_kernel.saLearningLimitLock);
						if(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port>=RTK_RG_PORT_CPU)
						{
							if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU)))
								atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
							atomic_dec(&rg_db.systemGlobal.sourceAddrLearningCount[rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU]);
							//decrease wlan's device count
							if(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port==(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))
							{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
								MACLN("wlan0 dev[%d] count is %d",rg_db.lut[search_index].wlan_device_idx,atomic_read(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[(int)rg_db.lut[search_index].wlan_device_idx]));
								if(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member&(0x1<<(rg_db.lut[search_index].wlan_device_idx)))
									atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
								atomic_dec(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[(int)rg_db.lut[search_index].wlan_device_idx]);				
								wlan_move_orig=rg_db.lut[search_index].wlan_device_idx;
#endif
							}
							port_move_orig=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU;
						}
						else
						{
							if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port)))
								atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
							atomic_dec(&rg_db.systemGlobal.sourceAddrLearningCount[rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port]);
							port_move_orig=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port;
							if(rg_db.systemGlobal.fix_l34_to_untag_enable==RG_HWNAT_ENABLE && (macEntry.vlan_id!=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid))
							{
								if(macEntry.vlan_id==0)
									macEntry.vlan_id=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid;
								MACLN("set fix_l34_vlan");
								macEntry.fix_l34_vlan=1;
							}
						}
						lut_orig=search_index;
						category_orig=rg_db.lut[search_index].category;
						//------------------ Critical Section End -----------------------//
						//rg_unlock(&rg_kernel.saLearningLimitLock);
		
						macEntry.arp_used=((rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)>0)?1:0;
						macEntry.auth=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.auth;
						TRACE("the port moving or vlan changing..arp used is %d, auth is %d\n",macEntry.arp_used,macEntry.auth);
						break;
					}
				}
				
				//hit hardware! check if we had already add to sw, if port-moving, delete sw one...
				_rtk_rg_softwareLut_checkAndDelete(&rg_db.lut[search_index].rtk_lut.entry.l2UcEntry,l2Idx>>2);

#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
				rg_db.lut[search_index].idleSecs = 0;
#endif
				if(macEntry.isIVL)goto ADD_SVL_LUT;		//check SVL,too

				//20140811LUKE: keep src mac lut idx!!
				pPktHdr->smacL2Idx=search_index;
				
				return RG_RET_SUCCESS;		//exist, do nothing
			}
		}

		count++; //search from next entry
	}while(count < 4);

	if(count==4)
	{
		//Check bCAM LUT first, if match, just return
		for(search_index=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE;search_index<MAX_LUT_HW_TABLE_SIZE;search_index++)
		{
			if(rg_db.lut[search_index].valid && rg_db.lut[search_index].rtk_lut.entryType==RTK_LUT_L2UC)
			{
				if(memcmp(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.mac.octet,pPktHdr->pSmac,ETHER_ADDR_LEN)==0)
				{
					if(((macEntry.isIVL==1) && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid==macEntry.vlan_id) ||
					((macEntry.isIVL==0) && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.fid==macEntry.fid))
					{
						//HIT!
						if(pPktHdr->pRxDesc->opts3.bit.reason==192 /* SA learning limit */ || pPktHdr->pRxDesc->opts3.bit.reason==196 /* unknown SA action */)
						{
							if((spa<RTK_RG_PORT_CPU && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port==spa) || 
							(spa>=RTK_RG_PORT_CPU && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port==(spa-RTK_RG_PORT_CPU)))
							{
								//Force SA learning
								TRACE("Forced SA learning...reason=%d",pPktHdr->pRxDesc->opts3.bit.reason);
								if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(spa)))
									atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
								atomic_dec(&rg_db.systemGlobal.sourceAddrLearningCount[spa]);
								//decrease wlan's device count				
								if(spa==RTK_RG_EXT_PORT0)
								{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
									atomic_dec(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[pPktHdr->wlan_dev_idx]);				
#endif
								}
								port_move_orig=spa;
								category_orig=rg_db.lut[search_index].category;
								break;
							}
						}
						
						if(	!(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC))
						{
							if((spa<RTK_RG_PORT_CPU && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port!=spa) || 
								(spa>=RTK_RG_PORT_CPU && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port!=(spa-RTK_RG_PORT_CPU)) ||
								(macEntry.vlan_id!=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid && !rg_db.lut[search_index].fix_l34_vlan) ||
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#else	//support ctag_if
								(macEntry.ctag_if!=((rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_CTAG_IF)?1:0) && !rg_db.lut[search_index].fix_l34_vlan) ||
#endif
								(spa==RTK_RG_EXT_PORT0 && pPktHdr->wlan_dev_idx!=rg_db.lut[search_index].wlan_device_idx))
							{
								//Mac port-moving or Vlan-moving or wlan-moving, update LUT table without change ARP_USED flag and auth field
								if(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port>=RTK_RG_PORT_CPU)
								{
									if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU)))
										atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
									atomic_dec(&rg_db.systemGlobal.sourceAddrLearningCount[rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU]);
									//decrease wlan's device count				
									if(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port==(RTK_RG_EXT_PORT0-RTK_RG_PORT_CPU))
									{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
										MACLN("wlan0 dev[%d] count is %d",rg_db.lut[search_index].wlan_device_idx,atomic_read(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[(int)rg_db.lut[search_index].wlan_device_idx]));
										if(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member&(0x1<<(rg_db.lut[search_index].wlan_device_idx)))
											atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
										atomic_dec(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[(int)rg_db.lut[search_index].wlan_device_idx]);
										wlan_move_orig=rg_db.lut[search_index].wlan_device_idx;
#endif
									}
									port_move_orig=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU;
								}
								else
								{
									if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port)))
										atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
									atomic_dec(&rg_db.systemGlobal.sourceAddrLearningCount[rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port]);
									port_move_orig=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port;
									if(rg_db.systemGlobal.fix_l34_to_untag_enable==RG_HWNAT_ENABLE && (macEntry.vlan_id!=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid))
									{
										if(macEntry.vlan_id==0)
											macEntry.vlan_id=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid;
										MACLN("set fix_l34_vlan");
										macEntry.fix_l34_vlan=1;
									}
								}
								lut_orig=search_index;
								category_orig=rg_db.lut[search_index].category;
						
								macEntry.arp_used=((rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)>0)?1:0;
								macEntry.auth=rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.auth;
								TRACE("the port moving or vlan changing..arp used is %d, auth is %d\n",macEntry.arp_used,macEntry.auth);
								break;
							}
						}

						//hit hardware! check if we had already add to sw, if port-moving, delete sw one...
						_rtk_rg_softwareLut_checkAllAndDelete(&rg_db.lut[search_index].rtk_lut.entry.l2UcEntry);

#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
						rg_db.lut[search_index].idleSecs = 0;
#endif
						if(macEntry.isIVL)goto ADD_SVL_LUT;		//check SVL,too

						//20140811LUKE: keep src mac lut idx!!
						pPktHdr->smacL2Idx=search_index;

						return RG_RET_SUCCESS;
					}
				}
			}
		}

		if(first_invalid==-1)
			count=_rtk_rg_layer2GarbageCollection(l2Idx);		//check if there is asynchronus between software and hardware table
	}

	//Check if we had been add to software LUT link-list
	if(!list_empty(&rg_db.softwareLutTableHead[l2Idx>>2]))
	{
		list_for_each_entry(pSoftLut,&rg_db.softwareLutTableHead[l2Idx>>2],lut_list)
		{
			if(memcmp(rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.mac.octet,pPktHdr->pSmac,ETHER_ADDR_LEN)==0)
			{
				if(((macEntry.isIVL==1) && rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.vid==macEntry.vlan_id) ||
				((macEntry.isIVL==0) && rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.fid==macEntry.fid))
				{
					//HIT!
					MACLN("software LUT had been added before!!");
					pSoftLut_hit=pSoftLut;
					break;
				}
			}
		}
	}

	//Check per port SA learning limit
	//------------------ Critical Section start -----------------------//
	//rg_lock(&rg_kernel.saLearningLimitLock);
	if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_UNLIMIT)
	{
		TRACE("Wan Access Limit is turn off.");
	}
	else if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_PORT)
	{
		if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[spa]>=0 &&
			rg_db.systemGlobal.sourceAddrLearningLimitNumber[spa]<=atomic_read(&rg_db.systemGlobal.sourceAddrLearningCount[spa]))		//no way to learn
		{
			//------------------ Critical Section End -----------------------//
			//rg_unlock(&rg_kernel.saLearningLimitLock);
			MACLN("Port %d SA learning limit is reached(%d)...action is %s!!",spa,rg_db.systemGlobal.sourceAddrLearningLimitNumber[spa],
				rg_db.systemGlobal.sourceAddrLearningAction[spa]==SA_LEARN_EXCEED_ACTION_PERMIT?"Permit and Forward":
				rg_db.systemGlobal.sourceAddrLearningAction[spa]==SA_LEARN_EXCEED_ACTION_PERMIT_L2?"Permit L2 only":"Drop");

			if(rg_db.systemGlobal.sourceAddrLearningAction[spa]!=SA_LEARN_EXCEED_ACTION_PERMIT)
			{
				//port-moving fail, recovery old count
				if(port_move_orig>=0)
				{
					//delete original hw mac
					_rtk_rg_shortCut_clear();
					(pf.rtk_rg_macEntry_del)(lut_orig);
					lut_orig=-1;
				}
				
				if(wlan_move_orig>=0)
				{
#ifdef CONFIG_MASTER_WLAN0_ENABLE				
					//add before delete because macEntry_del will dec again!!
					atomic_inc(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[wlan_move_orig]);
					if(lut_orig>=0)
					{												
						//delete original hw mac
						_rtk_rg_shortCut_clear();
						(pf.rtk_rg_macEntry_del)(lut_orig);
					}
#endif
				}

				if(rg_db.systemGlobal.sourceAddrLearningAction[spa]==SA_LEARN_EXCEED_ACTION_PERMIT_L2)
					limit_action_mask|=(0x1<<rg_db.systemGlobal.sourceAddrLearningAction[spa]);
				else
				{
					//recovery count because delete mac will decrease once
					if(port_move_orig>=0)
					{
						if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(port_move_orig)))
							atomic_inc(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
						atomic_inc(&rg_db.systemGlobal.sourceAddrLearningCount[port_move_orig]);
					}
					//exist software LUT, delete it before we drop
					if(pSoftLut_hit!=NULL)
					{
						//Delete from head list
						list_del_init(&pSoftLut_hit->lut_list);

						//Clear data
						//memset(&rg_db.lut[pSoftLut_hit->idx],0,sizeof(rtk_rg_table_lut_t));
						rg_db.lut[pSoftLut_hit->idx].valid=0;

						//Add back to free list
						list_add(&pSoftLut_hit->lut_list,&rg_db.softwareLutFreeListHead);
					}
					MACLN("Port Limit Action: DROP..");
					return RG_RET_FAIL;
				}
			}
			else
				return RG_RET_SUCCESS;	//20150420LUKE: action permit should forward the packet without learning
		}
#ifdef CONFIG_MASTER_WLAN0_ENABLE
		else if(spa==RTK_RG_EXT_PORT0 && rg_db.systemGlobal.wlan0SourceAddrLearningLimitNumber[pPktHdr->wlan_dev_idx]>=0 &&
			rg_db.systemGlobal.wlan0SourceAddrLearningLimitNumber[pPktHdr->wlan_dev_idx]<=atomic_read(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[pPktHdr->wlan_dev_idx]))
		{
			MACLN("WlanDev %d SA learning limit is reached(%d)...action is %s!!",pPktHdr->wlan_dev_idx,rg_db.systemGlobal.wlan0SourceAddrLearningLimitNumber[pPktHdr->wlan_dev_idx],
				rg_db.systemGlobal.wlan0SourceAddrLearningAction[pPktHdr->wlan_dev_idx]==SA_LEARN_EXCEED_ACTION_PERMIT?"Permit and Forward":
				rg_db.systemGlobal.wlan0SourceAddrLearningAction[pPktHdr->wlan_dev_idx]==SA_LEARN_EXCEED_ACTION_PERMIT_L2?"Permit L2 only":"Drop");
			
			if(rg_db.systemGlobal.wlan0SourceAddrLearningAction[pPktHdr->wlan_dev_idx]!=SA_LEARN_EXCEED_ACTION_PERMIT)
			{
				//port-moving fail, recovery old count
				if(port_move_orig>=0)
				{
					//delete original hw mac
					_rtk_rg_shortCut_clear();
					(pf.rtk_rg_macEntry_del)(lut_orig);
					lut_orig=-1;
				}
				
				if(wlan_move_orig>=0)
				{
					//add before delete because macEntry_del will dec again!!
					atomic_inc(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[wlan_move_orig]);
					if(lut_orig>=0)
					{
						//delete original hw mac
						_rtk_rg_shortCut_clear();
						(pf.rtk_rg_macEntry_del)(lut_orig);
					}
				}

				if(rg_db.systemGlobal.wlan0SourceAddrLearningAction[pPktHdr->wlan_dev_idx]==SA_LEARN_EXCEED_ACTION_PERMIT_L2)
					limit_action_mask|=(0x1<<rg_db.systemGlobal.wlan0SourceAddrLearningAction[pPktHdr->wlan_dev_idx]);
				else
				{
					//recovery count because delete mac will decrease once
					if(port_move_orig>=0)
					{
						if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(port_move_orig)))
							atomic_inc(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
						atomic_inc(&rg_db.systemGlobal.sourceAddrLearningCount[port_move_orig]);
					}
					//exist software LUT, delete it before we drop
					if(pSoftLut_hit!=NULL)
					{
						//Delete from head list
						list_del_init(&pSoftLut_hit->lut_list);

						//Clear data
						//memset(&rg_db.lut[pSoftLut_hit->idx],0,sizeof(rtk_rg_table_lut_t));
						rg_db.lut[pSoftLut_hit->idx].valid=0;

						//Add back to free list
						list_add(&pSoftLut_hit->lut_list,&rg_db.softwareLutFreeListHead);
					}
					MACLN("WLAN0Dev Limit Action: DROP..");
					return RG_RET_FAIL;
				}
			}
	 		else
				return RG_RET_SUCCESS;	//20150420LUKE: action permit should forward the packet without learning
		}
#endif
	}
	else
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT		
		if(pPktHdr->wlan_dev_idx!=RG_WWAN_WLAN0_VXD && pPktHdr->wlan_dev_idx!=RG_WWAN_WLAN1_VXD)	//20150514LUKE: packet from WWAN should not check access limit
#endif
	{
		MACLN("Check portmask or category access WAN limit, dip is %x, dmac is %02x:%02x:%02x:%02x:%02x:%02x",pPktHdr->ipv4Dip,
			pPktHdr->pDmac[0],pPktHdr->pDmac[1],pPktHdr->pDmac[2],pPktHdr->pDmac[3],pPktHdr->pDmac[4],pPktHdr->pDmac[5]);

		//At first check spa locate at LAN or not
		for(i=0;i<rg_db.systemGlobal.lanIntfTotalNum;i++)
		{
			if(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->port_mask.portmask&(0x1<<spa))
			{
				matchLanPort=1;
				if(memcmp(pPktHdr->pDmac,rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet,ETHER_ADDR_LEN)==0)
				{
					MACLN("Hit LAN[%d]!!Check if we are going to protocol stack or WAN",i);
					matchLanMac=1;
					ret=_rtk_rg_checkGwIp(pPktHdr);
					if(ret==RG_FWDENGINE_RET_TO_PS)
					{
						MACLN("goto protocol stack, so we just need software LUT as usual..");
						limit_action_mask|=SA_LEARN_EXCEED_ACTION_PERMIT_L2;
						matchLanIP=1;
						break;
					}
				}
			}
		}
		if(matchLanPort==1 && pPktHdr->ingressLocation!=RG_IGR_PROTOCOL_STACK)	//only care about LAN host
		{
			if(matchLanMac==0)
			{	
				//20140716LUKE:if the wan access type is CATEGORY, layer2 traffic will add to software-LUT first
				if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_CATEGORY)
				{
					//Category:default set to category 0, if not learned before
					if(pSoftLut_hit==NULL)
					{
						//new added!! check default category
						if(port_move_orig<0 && wlan_move_orig<0)
						{
							check_category=1;
							category_orig=0;
						}

						//otherwise won't check category!!because we are already in hw..					
					}
					else
					{
						//Check if this software LUT can add to hw
						check_category=1;
						category_orig=rg_db.lut[pSoftLut_hit->idx].category;
					}

					//Check if the spa is under LAN portmask
					if(_rtK_rg_checkCategoryPortmask_spa(spa)!=SUCCESS)
						category_orig=0;
					
					if(check_category==1)
					{
						if(rg_db.systemGlobal.accessWanLimitCategory[category_orig]>=0 && rg_db.systemGlobal.accessWanLimitCategory[category_orig]<=atomic_read(&rg_db.systemGlobal.accessWanLimitCategoryCount[category_orig]))
						{
							MACLN("Category %d access WAN limit is reached(%d)...action is %s!!",category_orig,rg_db.systemGlobal.accessWanLimitCategory[category_orig],
								rg_db.systemGlobal.accessWanLimitCategoryAction[category_orig]==SA_LEARN_EXCEED_ACTION_PERMIT?"Permit and Forward":
								rg_db.systemGlobal.accessWanLimitCategoryAction[category_orig]==SA_LEARN_EXCEED_ACTION_PERMIT_L2?"Permit L2 only":"Drop");

							if(pSoftLut_hit!=NULL)
							{
								//maybe port-moving, renew sw LUT data
								if(spa>=RTK_RG_PORT_CPU)
								{
									rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.port=RTK_RG_PORT_CPU;
									rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.ext_port=spa-RTK_RG_PORT_CPU;
									rg_db.lut[pSoftLut_hit->idx].wlan_device_idx=pPktHdr->wlan_dev_idx;
								}
								else
								{
									rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.port=spa;
									rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.ext_port=0;
								}
							}
							if(rg_db.systemGlobal.accessWanLimitCategoryAction[category_orig]==SA_LEARN_EXCEED_ACTION_DROP)
							{
								MACLN("Category Limit Action: DROP..");
								return RG_RET_FAIL;							
							}
							
							//permit or permit_l2
							limit_action_mask|=SA_LEARN_EXCEED_ACTION_PERMIT_L2;
						}
						else
							limit_action_mask|=SA_LEARN_EXCEED_ACTION_PERMIT_L2;	//even the limit is not reach, L2 packet should not add to hw
					}
				}
				else if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_PORTMASK) 
				{
					if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(spa))
#ifdef CONFIG_MASTER_WLAN0_ENABLE										
						|| (spa==RTK_RG_EXT_PORT0 && (rg_db.systemGlobal.accessWanLimitPortMask_wlan0member&(0x1<<pPktHdr->wlan_dev_idx)))
#endif						
					)
					{
						if(rg_db.systemGlobal.accessWanLimitPortMask<=atomic_read(&rg_db.systemGlobal.accessWanLimitPortMaskCount))
						{
							MACLN("Portmask access WAN limit is reached(%d)...action is %s!!",rg_db.systemGlobal.accessWanLimitPortMask,
								rg_db.systemGlobal.accessWanLimitPortMaskAction==SA_LEARN_EXCEED_ACTION_PERMIT?"Permit and Forward":
								rg_db.systemGlobal.accessWanLimitPortMaskAction==SA_LEARN_EXCEED_ACTION_PERMIT_L2?"Permit L2 only":"Drop");
								
							if(rg_db.systemGlobal.accessWanLimitPortMaskAction!=SA_LEARN_EXCEED_ACTION_PERMIT)
							{
								//port-moving fail, recovery old count
								if(port_move_orig>=0)
								{
									//delete original hw mac
									_rtk_rg_shortCut_clear();
									(pf.rtk_rg_macEntry_del)(lut_orig);
									lut_orig=-1;
								}

								if(wlan_move_orig>=0)
								{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
									//add before delete because macEntry_del will dec again!!
									atomic_inc(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[wlan_move_orig]);
									if(lut_orig>=0)
									{
										//delete original hw mac
										_rtk_rg_shortCut_clear();
										(pf.rtk_rg_macEntry_del)(lut_orig);
									}
#endif
								}
								if(rg_db.systemGlobal.accessWanLimitPortMaskAction==SA_LEARN_EXCEED_ACTION_DROP)
								{
									//port-moving fail, recovery old count
									if(port_move_orig>=0)
									{
										if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(port_move_orig)))
											atomic_inc(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
										atomic_inc(&rg_db.systemGlobal.sourceAddrLearningCount[port_move_orig]);
									}
									
									//exist software LUT, delete it before we drop
									if(pSoftLut_hit!=NULL)
									{
										//Delete from head list
										list_del_init(&pSoftLut_hit->lut_list);

										//Clear data
										//memset(&rg_db.lut[pSoftLut_hit->idx],0,sizeof(rtk_rg_table_lut_t));
										rg_db.lut[pSoftLut_hit->idx].valid=0;
										MACLN("lut[%d] is set to invalid!",pSoftLut_hit->idx);

										//Add back to free list
										list_add(&pSoftLut_hit->lut_list,&rg_db.softwareLutFreeListHead);
									}
									MACLN("Portmask Limit Action: DROP..");
									return RG_RET_FAIL;
								}
								//permit_l2
								limit_action_mask|=SA_LEARN_EXCEED_ACTION_PERMIT_L2;
							}
							else
								return RG_RET_SUCCESS;	//20150420LUKE: action permit should forward the packet without learning
						}
					}
				}
			}
			else if(matchLanIP==0)
			{
				//to WAN!!we have to check limit, if over limit, add to SW and trigger ReachLimit 
				//if not over limit, add to HW, and add count
				MACLN("to WAN!! activeLimitFunction is %d",rg_db.systemGlobal.activeLimitFunction);
				if(rg_db.systemGlobal.activeLimitFunction==RG_ACCESSWAN_TYPE_PORTMASK)
				{
#ifdef CONFIG_MASTER_WLAN0_ENABLE				
					MACLN("portmask is %x, wlan_dev_mask is %x, spa is %x, wlan_dev_idx is %x, limit is %d count is %d",
						rg_db.systemGlobal.accessWanLimitPortMask_member.portmask,
						rg_db.systemGlobal.accessWanLimitPortMask_wlan0member,
						0x1<<spa,pPktHdr->wlan_dev_idx,rg_db.systemGlobal.accessWanLimitPortMask,
						atomic_read(&rg_db.systemGlobal.accessWanLimitPortMaskCount));
#endif
					if(rg_db.systemGlobal.accessWanLimitPortMask<=atomic_read(&rg_db.systemGlobal.accessWanLimitPortMaskCount))
					{
						
						if((rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(spa))) 
#ifdef CONFIG_MASTER_WLAN0_ENABLE						
							|| (spa==RTK_RG_EXT_PORT0 && (rg_db.systemGlobal.accessWanLimitPortMask_wlan0member&(0x1<<pPktHdr->wlan_dev_idx)))
#endif						
						)
						{
							MACLN("Portmask access WAN limit is reached(%d)...action is %s!!",rg_db.systemGlobal.accessWanLimitPortMask,
								rg_db.systemGlobal.accessWanLimitPortMaskAction==SA_LEARN_EXCEED_ACTION_PERMIT?"Permit and Forward":
								rg_db.systemGlobal.accessWanLimitPortMaskAction==SA_LEARN_EXCEED_ACTION_PERMIT_L2?"Permit L2 only":"Drop");
							
							if(rg_db.systemGlobal.accessWanLimitPortMaskAction!=SA_LEARN_EXCEED_ACTION_PERMIT)
							{
								//port-moving fail, recovery old count
								if(port_move_orig>=0)
								{
									//delete original hw mac
									_rtk_rg_shortCut_clear();
									(pf.rtk_rg_macEntry_del)(lut_orig);
									lut_orig=-1;
								}

								if(wlan_move_orig>=0)
								{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
									//add before delete because macEntry_del will dec again!!
									atomic_inc(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[wlan_move_orig]);
									if(lut_orig>=0)
									{
										//delete original hw mac
										_rtk_rg_shortCut_clear();
										(pf.rtk_rg_macEntry_del)(lut_orig);
									}
#endif
								}
								if(rg_db.systemGlobal.accessWanLimitPortMaskAction==SA_LEARN_EXCEED_ACTION_DROP)
								{
									//port-moving fail, recovery old count
									if(port_move_orig>=0)
									{
										if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(port_move_orig)))
											atomic_inc(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
										atomic_inc(&rg_db.systemGlobal.sourceAddrLearningCount[port_move_orig]);
									}
									//exist software LUT, delete it before we drop
									if(pSoftLut_hit!=NULL)
									{
										//Delete from head list
										list_del_init(&pSoftLut_hit->lut_list);

										//Clear data
										//memset(&rg_db.lut[pSoftLut_hit->idx],0,sizeof(rtk_rg_table_lut_t));
										rg_db.lut[pSoftLut_hit->idx].valid=0;
										MACLN("lut[%d] is set to invalid!",pSoftLut_hit->idx);

										//Add back to free list
										list_add(&pSoftLut_hit->lut_list,&rg_db.softwareLutFreeListHead);
									}
									MACLN("Portmask Limit Action: DROP..");
									return RG_RET_FAIL;
								}
								//permit_l2
								limit_action_mask|=(0x1<<rg_db.systemGlobal.accessWanLimitPortMaskAction);
							}
						}
					}
				}
				else
				{
					//Category:default set to category 0, if not learned before
					if(pSoftLut_hit==NULL)
					{
						//new added!! check default category
						if(port_move_orig<0 && wlan_move_orig<0)
						{
							check_category=1;
							category_orig=0;
						}

						//otherwise won't check category!!because we are already in hw..					
					}
					else
					{
						//Check if this software LUT can add to hw
						check_category=1;
						category_orig=rg_db.lut[pSoftLut_hit->idx].category;
					}

					//Check if the spa is under LAN portmask
					if(_rtK_rg_checkCategoryPortmask_spa(spa)!=SUCCESS)
						category_orig=0;
					
					if(check_category==1 && rg_db.systemGlobal.accessWanLimitCategory[category_orig]>=0 &&
						rg_db.systemGlobal.accessWanLimitCategory[category_orig]<=atomic_read(&rg_db.systemGlobal.accessWanLimitCategoryCount[category_orig]))	//no way to learn
					{
						//------------------ Critical Section End -----------------------//
						//rg_unlock(&rg_kernel.saLearningLimitLock);
						MACLN("Category %d access WAN limit is reached(%d)...action is %s!!",category_orig,rg_db.systemGlobal.accessWanLimitCategory[category_orig],
							rg_db.systemGlobal.accessWanLimitCategoryAction[category_orig]==SA_LEARN_EXCEED_ACTION_PERMIT?"Permit and Forward":
							rg_db.systemGlobal.accessWanLimitCategoryAction[category_orig]==SA_LEARN_EXCEED_ACTION_PERMIT_L2?"Permit L2 only":"Drop");
						
						if(rg_db.systemGlobal.accessWanLimitCategoryAction[category_orig]!=SA_LEARN_EXCEED_ACTION_PERMIT)
						{
							if(pSoftLut_hit!=NULL)
							{
								//maybe port-moving, renew sw LUT data
								if(spa>=RTK_RG_PORT_CPU)
								{
									rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.port=RTK_RG_PORT_CPU;
									rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.ext_port=spa-RTK_RG_PORT_CPU;
									rg_db.lut[pSoftLut_hit->idx].wlan_device_idx=pPktHdr->wlan_dev_idx;
								}
								else
								{
									rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.port=spa;
									rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.ext_port=0;
								}
							}
							if(rg_db.systemGlobal.accessWanLimitCategoryAction[category_orig]==SA_LEARN_EXCEED_ACTION_DROP)
							{
								MACLN("Category Limit Action: DROP..");
								return RG_RET_FAIL;							
							}
							
							//permit_l2
							limit_action_mask|=SA_LEARN_EXCEED_ACTION_PERMIT_L2;
						}
					}
				}
			}
		}
	}
	//------------------ Critical Section End -----------------------//
	//rg_unlock(&rg_kernel.saLearningLimitLock);
	//Use the first meet valid empty index
	if(first_invalid>=0)
	{
		search_index=first_invalid;
	}	
	else if(count==4)	//Replace the least recently used entry for new entry
	{
		search_index=_rtk_rg_layer2LeastRecentlyUsedReplace(l2Idx);
		if(search_index==RG_RET_ENTRY_NOT_GET)
		{
			FIXME("must add software LUT entry for LUT entry full.");
			return RG_RET_FAIL;
		}			
			
	}
	else
		search_index=l2Idx+count;
#else
	//test for rtk_l2_addr_get
	rtk_l2_ucastAddr_t l2Addr;
	memset(&l2Addr,0,sizeof(rtk_l2_ucastAddr_t));

	l2Addr.fid=LAN_FID;
	memcpy(&l2Addr.mac.octet,pPktHdr->pSmac,6);

	ret=rtk_l2_addr_get(&l2Addr);
	DEBUG("rtk_l2_addr_get ret=%x",ret);
	if(ret==RT_ERR_OK)return RG_RET_SUCCESS;		//exist

#endif
	memcpy(macEntry.mac.octet,pPktHdr->pSmac,ETHER_ADDR_LEN);
	//set SVL for lanIntf, patched in 201221203
	//macEntry.fid=LAN_FID;
	//macEntry.isIVL=0;
	macEntry.port_idx=spa;
	macEntry.static_entry=0;	//FIXME:here turn off static entry to enable hardware auto age-out and port-moving

	//20150515LUKE: for WWAN port-moving, we should update hw directly without add sw lut
	if(limit_action_mask>0
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT	
	 || (port_move_orig<0 && (pPktHdr->wlan_dev_idx==RG_WWAN_WLAN0_VXD || pPktHdr->wlan_dev_idx==RG_WWAN_WLAN1_VXD))
#endif
	)
	{
		MACLN("add to software LUT only..");
		//port-moving fail, recovery old count
		if(port_move_orig>=0)
		{
			if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(port_move_orig)))
				atomic_inc(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
			atomic_inc(&rg_db.systemGlobal.sourceAddrLearningCount[port_move_orig]);
		}

		if(wlan_move_orig>=0)
		{
#ifdef CONFIG_MASTER_WLAN0_ENABLE		
			//add before delete because macEntry_del will dec again!!
			//atomic_inc(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[wlan_move_orig]);
			//delete original mac
			//ret=rtk_rg_macEntry_del(lut_orig);
			//MACLN("old MAC [%d] had been delete!!ret is %d",lut_orig,ret);
#endif
		}

		//over limit!!if not added before and not port-moving, add to software link list
		if(pSoftLut_hit==NULL)
		{
			MACLN("add new MAC[sw]!!");
			//move MAC from hardward to software
			ret=_rtk_rg_softwareLut_add(&macEntry,l2Idx>>2,0,pPktHdr->wlan_dev_idx);
			DEBUG("### add l2[sw]=%02x:%02x:%02x:%02x:%02x:%02x SPA=%d ###\n",pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],
				pPktHdr->pSmac[3],pPktHdr->pSmac[4],pPktHdr->pSmac[5],spa);
			TRACE("MAC[sw](%02x:%02x:%02x:%02x:%02x:%02x) learning at Port=%d\n",pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],
				pPktHdr->pSmac[3],pPktHdr->pSmac[4],pPktHdr->pSmac[5],spa);	
			assert_ok(ret);
			//delete hw entry
			//ret=rtk_rg_macEntry_del(i);
			//assert_ok(ret);	
		}
		else
		{
			//maybe port-moving, renew sw LUT data
			if(spa>=RTK_RG_PORT_CPU)
			{
				rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.port=RTK_RG_PORT_CPU;
				rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.ext_port=spa-RTK_RG_PORT_CPU;
				rg_db.lut[pSoftLut_hit->idx].wlan_device_idx=pPktHdr->wlan_dev_idx;
			}
			else
			{
				rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.port=spa;
				rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.ext_port=0;
				if(rg_db.systemGlobal.fix_l34_to_untag_enable==RG_HWNAT_ENABLE && (macEntry.vlan_id!=rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.vid))
				{
					if(rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.vid==0)
						rg_db.lut[pSoftLut_hit->idx].rtk_lut.entry.l2UcEntry.vid=macEntry.vlan_id;
					MACLN("set fix_l34_vlan");
					rg_db.lut[pSoftLut_hit->idx].fix_l34_vlan=1;
				}
			}
		}

		//trigger for ARP and Neighbor discovery for port, mask, and category
		//if this packet match LANIP, just pass lookup procedure
		if(matchLanIP==0 && matchLanMac==1)
		{
			switch(rg_db.systemGlobal.activeLimitFunction)
			{
				//send by all ARP and ND of same port
				case RG_ACCESSWAN_TYPE_PORT:					
#ifdef CONFIG_MASTER_WLAN0_ENABLE
					//_rtk_rg_lutReachLimit_init(RG_ACCESSWAN_TYPE_PORT, _rtk_rg_lutReachLimit_port, (unsigned long)(spa|pPktHdr->wlan_dev_idx<<16));	//do nothing here
#else
					//_rtk_rg_lutReachLimit_init(RG_ACCESSWAN_TYPE_PORT, _rtk_rg_lutReachLimit_port, (unsigned long)spa);	//do nothing here
#endif
					break;
				//send by all ARP and ND of all port in port mask
				case RG_ACCESSWAN_TYPE_PORTMASK:
#ifdef CONFIG_MASTER_WLAN0_ENABLE
					_rtk_rg_lutReachLimit_init(RG_ACCESSWAN_TYPE_PORTMASK, _rtk_rg_lutReachLimit_portmask, (unsigned long)(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask|(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member<<16)));
#else
					_rtk_rg_lutReachLimit_init(RG_ACCESSWAN_TYPE_PORTMASK, _rtk_rg_lutReachLimit_portmask, (unsigned long)rg_db.systemGlobal.accessWanLimitPortMask_member.portmask);
#endif
					break;
				//send by all ARP and ND of same category
				case RG_ACCESSWAN_TYPE_CATEGORY:
					_rtk_rg_lutReachLimit_init(RG_ACCESSWAN_TYPE_CATEGORY, _rtk_rg_lutReachLimit_category, (unsigned long)category_orig);
					break;
				default:
					break;
			}
		}

		if(limit_action_mask&(0x1<<SA_LEARN_EXCEED_ACTION_PERMIT_L2))
		{
			pPktHdr->swLutL2Only=1;
		}
	}
	else
	{
		MACLN("add to HW LUT!!");
		//exist software LUT, delete it before we add to HW
		if(pSoftLut_hit!=NULL)
		{
			//Delete from head list
			list_del_init(&pSoftLut_hit->lut_list);

			//Clear data
			//memset(&rg_db.lut[pSoftLut_hit->idx],0,sizeof(rtk_rg_table_lut_t));
			rg_db.lut[pSoftLut_hit->idx].valid=0;

			//Add back to free list
			list_add(&pSoftLut_hit->lut_list,&rg_db.softwareLutFreeListHead);
		}
		
		//add to both software and hardware
		ret=(pf.rtk_rg_macEntry_add)(&macEntry,&search_index);
		//DEBUG("### add l2[%d]=%02x:%02x:%02x:%02x:%02x:%02x SPA=%d ###\n",search_index,pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],
		//	pPktHdr->pSmac[3],pPktHdr->pSmac[4],pPktHdr->pSmac[5],spa);
		TRACE("MAC[%d](%02x:%02x:%02x:%02x:%02x:%02x) learning at Port=%d\n",search_index,pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],
			pPktHdr->pSmac[3],pPktHdr->pSmac[4],pPktHdr->pSmac[5],spa);	

#ifdef CONFIG_RG_WLAN_HWNAT_ACCELERATION
		if(pPktHdr->ingressPort==RTK_RG_EXT_PORT0) //update mbssid table
		{
			_rtk_rg_wlanMbssidLearning(pPktHdr->pSmac,pPktHdr);
		}
#endif		
		
		assert_ok(ret);

		//add to SA learning count
		//------------------ Critical Section start -----------------------//
		//rg_lock(&rg_kernel.saLearningLimitLock);
		//if MAC already added in hw, recovery it's category from old record
		rg_db.lut[search_index].category=category_orig;
		rg_db.lut[search_index].wlan_device_idx=pPktHdr->wlan_dev_idx;
		if(port_move_orig<0 && _rtK_rg_checkCategoryPortmask_spa(spa)==SUCCESS)
			atomic_inc(&rg_db.systemGlobal.accessWanLimitCategoryCount[category_orig]);
		if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(spa)))
			atomic_inc(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
		atomic_inc(&rg_db.systemGlobal.sourceAddrLearningCount[spa]);
		//increase wlan's device count				
		if(spa==RTK_RG_EXT_PORT0)
		{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
			if(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member&(0x1<<(rg_db.lut[search_index].wlan_device_idx)))
				atomic_inc(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
			atomic_inc(&rg_db.systemGlobal.wlan0SourceAddrLearningCount[(int)rg_db.lut[search_index].wlan_device_idx]);				
#endif
		}
		//20140811LUKE: keep src mac lut idx!!
		pPktHdr->smacL2Idx=search_index;
		//------------------ Critical Section End -----------------------//
		//rg_unlock(&rg_kernel.saLearningLimitLock);
	}

	if(macEntry.isIVL)goto ADD_SVL_LUT;		//add SVL,too
	return RG_RET_SUCCESS;
}

rtk_rg_fwdEngineReturn_t _rtk_rg_layer2Agent(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_successFailReturn_t sf_ret=RG_RET_SUCCESS;
 
 	//Drop multicast SMAC here
 	if((pPktHdr->pSmac[0]&0x1)>0)
 	{
  		//dump_hs();
  		//dump_packet(skb->data,64,"");
#ifdef __KERNEL__
  		printk("Strange multicast source mac len=%d, psmac=0x%x",skb->len,(u32)&pPktHdr->pSmac[0]);
		//memDump((u8*)((u32)skb->data),64,"");
		memDump((u8*)((u32)skb->data|0xa0000000),64,"");
#endif
  		DEBUG("Drop strange multicast packet...");
  		return RG_FWDENGINE_RET_DROP;
 	}

	if(pPktHdr->ingressPort!=RTK_RG_PORT_CPU)
	 	sf_ret=_rtk_rg_layer2LutLearning(skb,pPktHdr,pPktHdr->ingressPort);

	pPktHdr->l2AgentCalled=1;
	if(sf_ret==RG_RET_SUCCESS)
		return RG_FWDENGINE_RET_L2FORWARDED;
	else
	{
		TRACE("Drop!");
		return RG_FWDENGINE_RET_DROP;		//drop because sa learning limit action or unknown VLAN id 
	}
}
#else
int _rtk_rg_layer2Agent(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_macEntry_t macEntry;
	int ret,l2Idx,search_index,count=0;

	/* Handle multicast packet*/
#ifdef __KERNEL__
	if(rtk_rg_multicastRxCheck(skb,pPktHdr,pPktHdr->pRxDesc->opts3.bit.src_port_num)==SUCCESS)
		return RG_FWDENGINE_RET_TO_PS;
#endif

	//Drop multicast SMAC here
	if((pPktHdr->pSmac[0]&0x1)>0)
	{
		//dump_hs();
		dump_packet(skb->data,skb->len,"strange multicast source mac");
		DEBUG("drop strange multicast packet...");
		return RG_FWDENGINE_RET_DROP;
	}

	memset(&macEntry,0,sizeof(rtk_rg_macEntry_t));

	//DEBUG("@@@ port%d %02x:%02x:%02x:%02x:%02x:%02x @@@\n",pPktHdr->pRxDesc->opts3.bit.src_port_num,pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],
		//pPktHdr->pSmac[3],pPktHdr->pSmac[4],pPktHdr->pSmac[5]);
	/*if(pPktHdr->pRxDesc->opts2.bit.ctagva==1)
	{		
		macEntry.vlan_id=((pPktHdr->pRxDesc->opts2.bit.cvlan_tag&0xf)<<0x8)+((pPktHdr->pRxDesc->opts2.bit.cvlan_tag&0xff00)>>0x8);
		macEntry.fid=rg_db.vlan[macEntry.vlan_id].fid;
		//DEBUG("in layer2 agent, the vlan id from RX descriptor is %d\n",macEntry.vlan_id);
	}
	else if((pPktHdr->tagif&CVLAN_TAGIF)>0)
	{
		macEntry.vlan_id=pPktHdr->ctagVid;
		macEntry.fid=rg_db.vlan[macEntry.vlan_id].fid;
		//DEBUG("in layer2 agent, the vlan id from CTAG is %d\n",macEntry.vlan_id);
	}
	else
	{
		//DEBUG("in layer2 agent, untag..");
		macEntry.vlan_id=0;		//untagged
		macEntry.fid=LAN_FID;
	}*/
	macEntry.vlan_id=pPktHdr->internalVlanID;
	macEntry.fid=rg_db.vlan[macEntry.vlan_id].fid;
	//DEBUG("the internalVlanID is %d, fid is %d",macEntry.vlan_id,macEntry.fid);
#if 1

	if(/*macEntry.vlan_id!=0 && */rg_db.vlan[macEntry.vlan_id].fidMode==VLAN_FID_IVL)
	{
		macEntry.isIVL=1;
		l2Idx=_rtk_rg_hash_mac_vid_efid(pPktHdr->pSmac,macEntry.vlan_id,0);		//FIXME:EFID is 0 now
	}
	else
	{
ADD_SVL_LUT:
		count=0;
		macEntry.isIVL=0;
		if(pPktHdr->pRxDesc->opts2.bit.ctagva==0 || (pPktHdr->tagif&CVLAN_TAGIF)==0)
			macEntry.vlan_id=0;		//untag
		//if(rg_db.vlan[macEntry.vlan_id].UntagPortmask.bits[0]&(0x1<<pPktHdr->pRxDesc->opts3.bit.src_port_num))macEntry.vlan_id=0;		//untag
		l2Idx=_rtk_rg_hash_mac_fid_efid(pPktHdr->pSmac,macEntry.fid,0);			//FIXME:EFID is 0 now
	}
	
	l2Idx<<=2;
	do
	{
		search_index = l2Idx+count;
		//DEBUG("search_idx is %d\n",search_index);
		if(rg_db.lut[search_index].valid==0)
			break;	//empty, just add
						
		if(rg_db.lut[search_index].rtk_lut.entryType==RTK_LUT_L2UC &&
			(memcmp(&rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.mac.octet,pPktHdr->pSmac,6)==0))
		{
			if(((macEntry.isIVL==1) && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid==macEntry.vlan_id) ||
			((macEntry.isIVL==0) && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.fid==macEntry.fid))
			{
				//DEBUG("match!!");
				/*
				//FIXME: here reserved for WiFi interface may also need to handle port-moving in the future.
				if(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port==RTK_RG_MAC_PORT_CPU)
				{

				}
				else */if(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port!=pPktHdr->pRxDesc->opts3.bit.src_port_num)
				{
					//Mac port-moving, update LUT table without change ARP_USED flag
					//------------------ Critical Section start -----------------------//
					//rg_lock(&rg_kernel.saLearningLimitLock);
					rg_db.systemGlobal.sourceAddrLearningCount[rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.port]--;
					//------------------ Critical Section End -----------------------//
					//rg_unlock(&rg_kernel.saLearningLimitLock);
	
					macEntry.arp_used=((rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)>0)?1:0;
					//DEBUG("the port is moving..arp used is %d\n",macEntry.arp_used);
					break;
				}

				if(macEntry.isIVL)goto ADD_SVL_LUT;		//check SVL,too
				return RG_FWDENGINE_RET_L2FORWARDED;		//exist, do nothing
			}
		}

		count++; //search from next entry
	}while(count < 4);

	if(count==4)
	{
		count=_rtk_rg_layer2GarbageCollection(l2Idx);		//check if there is asynchronus between software and hardware table
		if(count==4)
			search_index=_rtk_rg_layer2LeastRecentlyUsedReplace(l2Idx);		//replace the least recently used entry for new entry
		else
			search_index=l2Idx+count;
	}

	//Check per port SA learning limit
	//------------------ Critical Section start -----------------------//
	//rg_lock(&rg_kernel.saLearningLimitLock);
	if(rg_db.systemGlobal.sourceAddrLearningLimitNumber[pPktHdr->pRxDesc->opts3.bit.src_port_num]==rg_db.systemGlobal.sourceAddrLearningCount[pPktHdr->pRxDesc->opts3.bit.src_port_num])		//no way to learn
	{
		//------------------ Critical Section End -----------------------//
		//rg_unlock(&rg_kernel.saLearningLimitLock);
		DEBUG("Port %d SA learning limit is reached(%d)...action is %s!!",pPktHdr->pRxDesc->opts3.bit.src_port_num,rg_db.systemGlobal.sourceAddrLearningLimitNumber[pPktHdr->pRxDesc->opts3.bit.src_port_num],
			rg_db.systemGlobal.sourceAddrLearningAction[pPktHdr->pRxDesc->opts3.bit.src_port_num]==SA_LEARN_EXCEED_ACTION_PERMIT?"Permit and Forward":"Drop");
		if(rg_db.systemGlobal.sourceAddrLearningAction[pPktHdr->pRxDesc->opts3.bit.src_port_num]==SA_LEARN_EXCEED_ACTION_PERMIT)
			return RG_FWDENGINE_RET_L2FORWARDED;
		else
			return RG_FWDENGINE_RET_DROP;
	}
	//------------------ Critical Section End -----------------------//
	//rg_unlock(&rg_kernel.saLearningLimitLock);
#else
	//test for rtk_l2_addr_get
	rtk_l2_ucastAddr_t l2Addr;
	memset(&l2Addr,0,sizeof(rtk_l2_ucastAddr_t));

	l2Addr.fid=LAN_FID;
	memcpy(&l2Addr.mac.octet,pPktHdr->pSmac,6);

	ret=rtk_l2_addr_get(&l2Addr);
	DEBUG("rtk_l2_addr_get ret=%x",ret);
	if(ret==RT_ERR_OK)return RG_RET_SUCCESS;		//exist

#endif
	memcpy(macEntry.mac.octet,pPktHdr->pSmac,6);
	//set SVL for lanIntf, patched in 201221203
	//macEntry.fid=LAN_FID;
	//macEntry.isIVL=0;
	macEntry.port_idx=pPktHdr->pRxDesc->opts3.bit.src_port_num;
	macEntry.static_entry=0;	//FIXME:here turn off static entry to enable hardware auto age-out and port-moving
	ret=rtk_rg_macEntry_add(&macEntry,&search_index);
	DEBUG("### add l2[%d]=%02x:%02x:%02x:%02x:%02x:%02x ###\n",search_index,pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],
		pPktHdr->pSmac[3],pPktHdr->pSmac[4],pPktHdr->pSmac[5]);
	assert_ok(ret);

	//add to SA learning count
	//------------------ Critical Section start -----------------------//
	//rg_lock(&rg_kernel.saLearningLimitLock);
	rg_db.systemGlobal.sourceAddrLearningCount[pPktHdr->pRxDesc->opts3.bit.src_port_num]++;
	//------------------ Critical Section End -----------------------//
	//rg_unlock(&rg_kernel.saLearningLimitLock);

	if(macEntry.isIVL)goto ADD_SVL_LUT;		//add SVL,too
	return RG_FWDENGINE_RET_L2FORWARDED;
}
#endif

rtk_rg_lookupIdxReturn_t _rtk_rg_macLookup(u8 *pMac, int vlanId)
{
	int l2Idx,search_index;
	int count=0;
	rtk_rg_lut_linkList_t *pSoftLut;
	
	if(rg_db.vlan[vlanId].fidMode==VLAN_FID_IVL)
	{
		l2Idx=_rtk_rg_hash_mac_vid_efid(pMac,vlanId,0);		//FIXME;current efid is always 0
	}
	else
	{
		l2Idx=_rtk_rg_hash_mac_fid_efid(pMac,rg_db.vlan[vlanId].fid,0);		//FIXME;current efid is always 0
	}
	
	search_index=l2Idx<<2;
	do
	{		
		search_index+=count;
		if(rg_db.lut[search_index].valid && rg_db.lut[search_index].rtk_lut.entryType==RTK_LUT_L2UC &&
			(!memcmp(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.mac.octet,pMac,ETHER_ADDR_LEN)))
		{
			if((rg_db.vlan[vlanId].fidMode==VLAN_FID_IVL && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid==vlanId) ||
				(rg_db.vlan[vlanId].fidMode==VLAN_FID_SVL && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.fid==rg_db.vlan[vlanId].fid))
			{
				return search_index;
			}
		}
		
		count++; //search from next entry
	}
	while(count < 4);	

	//Check bCAM LUT, if match, just return
	for(search_index=MAX_LUT_HW_TABLE_SIZE-MAX_LUT_BCAM_TABLE_SIZE;search_index<MAX_LUT_HW_TABLE_SIZE;search_index++)
	{
		if(rg_db.lut[search_index].valid && rg_db.lut[search_index].rtk_lut.entryType==RTK_LUT_L2UC)
		{
			if(memcmp(rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.mac.octet,pMac,ETHER_ADDR_LEN)==0)
			{
				if((rg_db.vlan[vlanId].fidMode==VLAN_FID_IVL && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.vid==vlanId) ||
				(rg_db.vlan[vlanId].fidMode==VLAN_FID_SVL && rg_db.lut[search_index].rtk_lut.entry.l2UcEntry.fid==rg_db.vlan[vlanId].fid))
				{
					//HIT!
					return search_index;
				}
			}
		}
	}

	//Check if we had been add to software LUT link-list
	if(!list_empty(&rg_db.softwareLutTableHead[l2Idx]))
	{
		list_for_each_entry(pSoftLut,&rg_db.softwareLutTableHead[l2Idx],lut_list)
		{
			if(memcmp(rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.mac.octet,pMac,ETHER_ADDR_LEN)==0)
			{
				if((rg_db.vlan[vlanId].fidMode==VLAN_FID_IVL && rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.vid==vlanId) ||
				(rg_db.vlan[vlanId].fidMode==VLAN_FID_SVL && rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.fid==rg_db.vlan[vlanId].fid))
				{
					//HIT!
					MACLN("software LUT had been learned before!!");
					return pSoftLut->idx;
				}
			}
		}
	}

	return RG_RET_LOOKUPIDX_NOT_FOUND;

}

rtk_rg_fwdEngineReturn_t _rtk_rg_checkIPv6MulticastScope(rtk_rg_pktHdr_t *pPktHdr)
{
	switch(pPktHdr->pIpv6Dip[1]&0xf){
		case 0x0:	// 0  reserved
		case 0x1:	// 1   Interface-Local scope 
		//case 0x2:	// 2  Link-Local scope
			return RG_FWDENGINE_RET_DROP;
		default:
			return RG_FWDENGINE_RET_BROADCAST;
	}
}

__SRAM_FWDENG_SLOWPATH
rtk_rg_fwdEngineReturn_t _rtk_rg_layer2Forward(struct sk_buff *skb, rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_fwdEngineReturn_t ret;
	int l2Idx;
	//unsigned char WANIntfMask=0;

	/* Handle software Learning of LUT table*/
#ifdef CONFIG_RG_LAYER2_SOFTWARE_LEARN
	//if(pPktHdr->isGatewayPacket == 0)	//DA =\= Gateway MAC, do Layer2 agent
	//{
		ret=_rtk_rg_layer2Agent(skb,pPktHdr);
		if(ret!=RG_FWDENGINE_RET_L2FORWARDED)return ret;		//multicast packet may return to PS
	//}
#endif

	/* Handle multicast packet*/
#ifdef __KERNEL__
	if(rg_db.systemGlobal.initParam.igmpSnoopingEnable)
	{
		ret=_rtk_rg_multicastRxCheck(skb,pPktHdr,pPktHdr->ingressPort);
		if(ret==RG_FWDENGINE_RET_TO_PS) return RG_FWDENGINE_RET_TO_PS;
		if (rg_db.pktHdr->ingressIgmpMldDrop) return RG_FWDENGINE_RET_DROP;
	}
#endif

	/* Handle packet has special CPU trap reason*/
	switch(pPktHdr->pRxDesc->opts3.bit.reason)
	{
   	    case 0: //normal forward
   	    case 0x8:	//8: NAT/NAPT (Layer 4) Fragmented IPv4 packets
		case 0x9:	//9: Routed (Layer 3) Fragmented IPv4/v6 packets
		case 0x22: //34: NAPT lookup miss
   	    	break;
			
		case 0x6:	//6: PPPoE ID lookup miss for Layer 3/4 forwarding
			DEBUG("PPPoE ID lookup miss for L34 forwarding");
			break;
		case 0x15:	//21: ARP or Neighbor miss
			/* layer2 no need to do ARP,  The ARP miss procedure will be done by L34 dataPath.
			if((pPktHdr->tagif&IPV6_TAGIF)==0)
			{
				ret=_rtk_rg_fwdengine_handleArpMiss(pPktHdr);
				//if(ret==RG_FWDENGINE_RET_TO_PS)return ret;
				//return RG_FWDENGINE_RET_DROP;
			}
			else
			{
				ret=_rtk_rg_fwdengine_handleNeighborMiss(pPktHdr);
				//if(ret==RG_FWDENGINE_RET_TO_PS)return ret;
			}
			*/
			break;
		case 0x16: //22: over MTU Size
			pPktHdr->overMTU=1;
			break;
		case 0xCC:	//204: unknown DA for unicast packet
			if(pPktHdr->isGatewayPacket==0)
			{
#if 0
////=======lookup DMAC start==========
				int l2Idx;
				int count=0;
				
				if(rg_db.vlan[pPktHdr->internalVlanID].fidMode==VLAN_FID_IVL)
				{
					l2Idx=_rtk_rg_hash_mac_vid_efid(pPktHdr->pDmac,pPktHdr->internalVlanID,0);		//FIXME;current efid is always 0
				}
				else
				{
					l2Idx=_rtk_rg_hash_mac_fid_efid(pPktHdr->pDmac,rg_db.vlan[pPktHdr->internalVlanID].fid,0);		//FIXME;current efid is always 0
				}
				
				l2Idx<<=2;
				do
				{						
					if(rg_db.lut[l2Idx].rtk_lut.entryType==RTK_LUT_L2UC &&
						(!memcmp(&rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.mac.octet,pPktHdr->pDmac,ETHER_ADDR_LEN)))
					{
						if((rg_db.vlan[pPktHdr->internalVlanID].fidMode==VLAN_FID_IVL && rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.vid==pPktHdr->internalVlanID) ||
							(rg_db.vlan[pPktHdr->internalVlanID].fidMode==VLAN_FID_SVL && rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.fid==rg_db.vlan[pPktHdr->internalVlanID].fid))
						{
							rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.age=7;
							RTK_L2_ADDR_ADD(&rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry);
							DEBUG("unicast unknown DA %02x:%02x:%02x:%02x:%02x:%02x...force sync to HW-LUT",pPktHdr->pDmac[0],pPktHdr->pDmac[1],pPktHdr->pDmac[2],pPktHdr->pDmac[3],pPktHdr->pDmac[4],pPktHdr->pDmac[5]);
							break;
						}
					}
					else
					{
						count++; //search from next entry
						l2Idx++;
					}
				}
				while(count < 4);
				
				if(count<4) break;

////=======lookup DMAC End==========
#else
				l2Idx=_rtk_rg_macLookup(pPktHdr->pDmac,pPktHdr->internalVlanID);
				if(l2Idx!=RG_RET_LOOKUPIDX_NOT_FOUND)
				{
					rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.age=7;
					if(l2Idx >= MAX_LUT_HW_TABLE_SIZE)
					{
						TRACE("unicast unknown DA %02x:%02x:%02x:%02x:%02x:%02x in SW-LUT-Link-list...update SW-LUT age time",pPktHdr->pDmac[0],pPktHdr->pDmac[1],pPktHdr->pDmac[2],pPktHdr->pDmac[3],pPktHdr->pDmac[4],pPktHdr->pDmac[5]);
					}
					else
					{
						RTK_L2_ADDR_ADD(&rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry);
						TRACE("unicast unknown DA %02x:%02x:%02x:%02x:%02x:%02x in SW-LUT...force sync to HW-LUT",pPktHdr->pDmac[0],pPktHdr->pDmac[1],pPktHdr->pDmac[2],pPktHdr->pDmac[3],pPktHdr->pDmac[4],pPktHdr->pDmac[5]);
					}
					break;
				}
#endif
				//unknown DA rate limit check
				if(rg_db.systemGlobal.unKnownDARateLimitPortMask&(1<<(pPktHdr->ingressPort))){
					ret=_rtk_rg_unknownDARateLimit_check(skb,pPktHdr);
					if(ret==RG_FWDENGINE_RET_DROP)
						return RG_FWDENGINE_RET_DROP;
				}

				TRACE("unicast unknown DA %02x:%02x:%02x:%02x:%02x:%02x...go to broadcast",pPktHdr->pDmac[0],pPktHdr->pDmac[1],pPktHdr->pDmac[2],pPktHdr->pDmac[3],pPktHdr->pDmac[4],pPktHdr->pDmac[5]);
				return RG_FWDENGINE_RET_BROADCAST;
			}
			break;
		default:
			TRACE("not be handled reason=%d\n",pPktHdr->pRxDesc->opts3.bit.reason);
			break;
	}

	/* Handle packet has special etherType*/
	switch(pPktHdr->etherType)
	{
//		case 0x8809:										/* pass OAM packet */
//#ifdef CONFIG_DUALBAND_CONCURRENT
//		case CONFIG_DEFAULT_IPC_SEND_ETHERTYPE:
//		case CONFIG_DEFAULT_IPC_RECV_ETHERTYPE:			
//		case CONFIG_DEFAULT_IPC_SIGNAL_ETHERTYPE:
//#endif			
//			return RG_FWDENGINE_RET_TO_PS;
		case 0x0806:										/* Handle ARP packet */
			//memDump(skb->data,skb->len,"rx 0806 data");
			_rtk_rg_arpAgent(skb->data,skb->len,pPktHdr);

			ret=_rtk_rg_checkGwIp(pPktHdr);
			if(ret==RG_FWDENGINE_RET_TO_PS)
				return ret;
			break;
		case 0x8863:										/* Handle PPPoE control packet */
			//if((rg_db.algFunctionMask & RTK_RG_ALG_PPPOE_PASSTHROUGH_BIT) > 0)		/* Handle broadcast packet to routing WAN if PPPoE pass through is turn on*/
			//{
				//DEBUG("Pass through turn on!!");
				//Check DA for WAN interface, return to PS if match
				/*for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
				{
					if(memcmp(rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf.wan_intf_conf.gmac.octet,pPktHdr->pDmac,6)==0)
					{
						//DEBUG("match WAN! to PS now");
						return RG_FWDENGINE_RET_TO_PS;
					}

					//get all WAN should be put in broadcast mask
					
				}*/
				if(pPktHdr->isGatewayPacket == 1)
				{
					//packet should be learned in layer2Agent before goto protocol stack!!
					//ret=_rtk_rg_layer2Agent(skb,pPktHdr);
					//if(ret!=RG_FWDENGINE_RET_L2FORWARDED)return ret;		//multicast packet may return to PS
					
					//DEBUG("8863 packet, before return to protocol stack");
					//dump_packet(skb->data,skb->len,"8863 packet");
#ifdef CONFIG_RG_SIMPLE_PROTOCOL_STACK
					int i;
					//Check Lan MAC address, actually this may not be needed, since PPPoE should on WAN interface only....
					/*for(i=0;i<rg_db.systemGlobal.lanIntfTotalNum;i++)
					{
						if(memcmp(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet,pPktHdr->pDmac,6)==0)
						{
							cp->wanInterfaceIdx=0;
							goto RET_TO_PS;
						}
					}*/

					//Check Wan 
					for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
					{
						if(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type==RTK_RG_PPPoE && 
							memcmp(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->gmac.octet,pPktHdr->pDmac,ETHER_ADDR_LEN)==0)
						{
							if(pPktHdr->cp!=NULL)pPktHdr->cp->wanInterfaceIdx=i;
							break;
						}
					}
//RET_TO_PS:
#endif
					//DEBUG("the cp->waninterfaceIdx is %d",cp->wanInterfaceIdx);
					TRACE("DMAC=GMAC & ETHTYPE=0x8863, to PS!");

					return RG_FWDENGINE_RET_TO_PS;
				}
			
				if((pPktHdr->pDmac[0]&pPktHdr->pDmac[1]&pPktHdr->pDmac[2]&pPktHdr->pDmac[3]&pPktHdr->pDmac[4]&pPktHdr->pDmac[5])==0xff)
				{
					//DEBUG("layer2 broadcast packet");
					if((rg_db.algFunctionMask & RTK_RG_ALG_PPPOE_PASSTHROUGH_BIT) > 0)		/* Handle broadcast packet to routing WAN if PPPoE pass through is turn on*/
						pPktHdr->internalVlanID=rg_db.systemGlobal.initParam.fwdVLAN_CPU;	//pass through between LAN and WAN interface
					return RG_FWDENGINE_RET_BROADCAST;
				} 
				else
				{
					if((rg_db.algFunctionMask & RTK_RG_ALG_PPPOE_PASSTHROUGH_BIT) > 0)		/* Handle broadcast packet to routing WAN if PPPoE pass through is turn on*/
						pPktHdr->internalVlanID=rg_db.systemGlobal.initParam.fwdVLAN_CPU;	//pass through between LAN and WAN interface, and DA search if original vlan is IVL (using SVL vlan fwdVLAN_CPU to let it find DA)  

					//DEBUG("layer2 unicast packet...hardware lookup");
					goto layer2_return;		//unicast L2 packet should be forwarded as hardware lookup
				}
				//{
	 				//Unicast packet in PPPoE Passthrough, change VID and port from DMAC2CVID
					//DEBUG("the packet is PPPoE control unicast packet, just bridging..");
//#ifdef __KERNEL__
					//turn on txInfo mask, otherwise value won't be add
					/*rg_kernel.txDescMask.opts2.bit.tx_vlan_action=0x3;
					rg_kernel.txDescMask.opts1.bit.ipcs=1;
					rg_kernel.txDescMask.opts1.bit.l4cs=1;
					rg_kernel.txDesc.opts1.bit.ipcs=1;
					rg_kernel.txDesc.opts1.bit.l4cs=1;
					rg_kernel.txDesc.opts2.bit.tx_vlan_action = 0x2;		//removing

					ret=_rtk_rg_fwdEngineDMAC2CVIDLookup(pPktHdr,LAN_FID,0);		//FIXME:we use only one FID right now
					if(ret!=RT_ERR_RG_OK)
						return RG_FWDENGINE_RET_TO_PS;

					_rtk_rg_splitJumboSendToNicWithTxInfoAndMask(pPktHdr,skb,&rg_kernel.txDesc,0,&rg_kernel.txDescMask);	
					return RG_FWDENGINE_RET_DIRECT_TX;*/

					
					/*ret=_rtk_rg_fwdEngineDirectTx(skb,pPktHdr);
					if(ret==RT_ERR_RG_OK)
						return RG_FWDENGINE_RET_DIRECT_TX;*/
//#endif	
				//}
			//}
			
			//Otherwise 8863 will be sended to protocol stack
			//return RG_FWDENGINE_RET_TO_PS;
		case 0x8864:									/* Handle PPPoE data packet */
			//Check layer2 packet(without IP header)
			if(pPktHdr->isGatewayPacket==1)
			{
				if((pPktHdr->tagif&(IPV4_TAGIF|IPV6_TAGIF))==0)
				{
					TRACE("DMAC=GMAC & ETHTYPE=0x8864 & Not (IPv4 or IPv6), to PS!");
					return RG_FWDENGINE_RET_TO_PS;
				}
				else{
					if((pPktHdr->tagif&IPV6_TAGIF)&&(pPktHdr->pIpv6Dip[0]==0xff)){
						return _rtk_rg_checkIPv6MulticastScope(pPktHdr);
					}
					return RG_FWDENGINE_RET_CONTINUE;	//continue to L34 forward
				}
			}
			
			if((rg_db.algFunctionMask & RTK_RG_ALG_PPPOE_PASSTHROUGH_BIT) > 0)		/* Handle broadcast packet to routing WAN if PPPoE pass through is turn on*/
			{
				if((pPktHdr->pDmac[0]&pPktHdr->pDmac[1]&pPktHdr->pDmac[2]&pPktHdr->pDmac[3]&pPktHdr->pDmac[4]&pPktHdr->pDmac[5])==0xff)
				{
					//DEBUG("broadcast packet");
					if((rg_db.algFunctionMask & RTK_RG_ALG_PPPOE_PASSTHROUGH_BIT) > 0)		/* Handle broadcast packet to routing WAN if PPPoE pass through is turn on*/
						pPktHdr->internalVlanID=rg_db.systemGlobal.initParam.fwdVLAN_CPU;	//pass through between LAN and WAN interface
					return RG_FWDENGINE_RET_BROADCAST;
				} 
				else
				{
					/* Handle broadcast packet to routing WAN if PPPoE pass through is turn on*/
					pPktHdr->internalVlanID=rg_db.systemGlobal.initParam.fwdVLAN_CPU;	//pass through between LAN and WAN interface, and DA search if original vlan is IVL (using SVL vlan fwdVLAN_CPU to let it find DA)  
				
					//DEBUG("the packet is PPPoE data unicast packet to %02x:%02x:%02x:%02x:%02x:%02x, just bridging..",
						//pPktHdr->pDmac[0],pPktHdr->pDmac[1],pPktHdr->pDmac[2],pPktHdr->pDmac[3],pPktHdr->pDmac[4],pPktHdr->pDmac[5]);
					goto layer2_return;		//unicast L2 packet should be forwarded as hardware lookup
	 				//Unicast packet in PPPoE Passthrough, change VID and port from DMAC2CVID
					//dump_packet(skb->data,skb->len,"sc");
//#ifdef __KERNEL__
//					ret=_rtk_rg_fwdEngineDirectTx(skb,pPktHdr);
//					if(ret==RT_ERR_RG_OK)
//						return RG_FWDENGINE_RET_DIRECT_TX;
//#endif	
				}
			}

#if 0
			if((pPktHdr->tagif&ICMPV6_TAGIF)&&(pPktHdr->ICMPv6Type==0x87)) //IPv6 Neighbor Solicitation
			{
				unsigned char zero[16]={0};
				if(memcmp(pPktHdr->pIpv6Sip,zero,16)!=0)
					_rtk_rg_neighborAgent(skb,pPktHdr);
			}
#endif			
			
			break;
		case 0x0800:	//IPv4
			break;
		case 0x86dd:	//IPv6
#if 0		
			if((pPktHdr->tagif&ICMPV6_TAGIF)&&(pPktHdr->ICMPv6Type==0x87)) //IPv6 Neighbor Solicitation
			{
				unsigned char zero[16]={0};
				if(memcmp(pPktHdr->pIpv6Sip,zero,16)!=0)
					_rtk_rg_neighborAgent(skb,pPktHdr);
			}
#endif
			//20150520LUKE: for FF:XX...XX IPv6 address, we should always broadcast it!
			if(pPktHdr->pIpv6Dip[0]==0xff){
				return _rtk_rg_checkIPv6MulticastScope(pPktHdr);
			}
			break;
		default:
			if(pPktHdr->isGatewayPacket) //unknown ethertype & DMAC=GMAC
			{
				TRACE("DMAC=GMAC & Ethertype can't handle by FwdEngine, to PS!");
				return RG_FWDENGINE_RET_TO_PS; 
			}
			break;
	}

	//Continue to L34forward
	if(pPktHdr->isGatewayPacket) //known ethertype & DMAC=GMAC
	{
		//20140403LUKE:if learning limit is reach and action is SA_LEARN_EXCEED_ACTION_PERMIT_L2, we can't do L34
		if(pPktHdr->swLutL2Only==1)
		{
			//Check if we are heading to PS
			ret=_rtk_rg_checkGwIp(pPktHdr);
			if(ret==RG_FWDENGINE_RET_TO_PS)
			{
				TRACE("ToPS!");
				return ret;
			}
			else
			{
				TRACE("Drop!");
				return RG_FWDENGINE_RET_DROP;
			}
		}
		else
			return RG_FWDENGINE_RET_CONTINUE;
	}

	/* Handle broadcast packet or unknown DA unicast packet*/
	if((pPktHdr->pDmac[0]&1)==1)
	{
		return RG_FWDENGINE_RET_BROADCAST;
	}
	
layer2_return:
	TRACE("Layer2 Forward");
	return RG_FWDENGINE_RET_L2FORWARDED;
}


__IRAM_FWDENG
void _rtk_rg_extIngressPortDecision(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_rxdesc_t	*pRxDesc)
{
	//init
#if defined(CONFIG_RTL9600_SERIES) || defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)

	pPktHdr->ingressLocation=RG_IGR_PHY_PORT;
	if(pRxDesc->opts3.bit.src_port_num==RTK_RG_MAC_PORT_CPU){
		pPktHdr->ingressPort=RTK_RG_PORT_CPU;
		if(pRxDesc->opts3.bit.dst_port_mask==0x8){ //from EXT0 (Using ACL modify EXT_SPA to DST_PMSK)
			pPktHdr->ingressPort=RTK_RG_EXT_PORT0;
		}
#ifdef CONFIG_DUALBAND_CONCURRENT
		else if(pRxDesc->opts3.bit.dst_port_mask==0x10){ //from EXT1 (Using ACL modify EXT_SPA to DST_PMSK)
			pPktHdr->ingressPort=RTK_RG_EXT_PORT1;				
		}
#endif
		else if(pRxDesc->opts3.bit.dst_port_mask==0x20){
			if(pRxDesc->opts3.bit.rsvd==0x0){	//from protocol statck			
				pPktHdr->ingressLocation=RG_IGR_PROTOCOL_STACK;
			}else if(pRxDesc->opts3.bit.rsvd==0x1){	//from ARP or ND
				pPktHdr->ingressLocation=RG_IGR_ARP_OR_ND;
			}else if(pRxDesc->opts3.bit.rsvd==0x2){	//from IGMP or MLD
				pPktHdr->ingressLocation=RG_IGR_IGMP_OR_MLD;
			}
		}else{
#ifdef CONFIG_MASTER_WLAN0_ENABLE		
			if(pRxDesc==(rtk_rg_rxdesc_t *)&rg_db.systemGlobal.rxInfoFromWLAN){
				pPktHdr->ingressPort=RTK_RG_EXT_PORT0;
			}
#endif
		}		
	}else{
		pPktHdr->ingressPort=pRxDesc->opts3.bit.src_port_num;
	}
	
#elif defined(CONFIG_RTL9602C_SERIES)

	pPktHdr->ingressLocation=RG_IGR_PHY_PORT;
	if(pRxDesc->opts3.bit.src_port_num==RTK_RG_MAC_PORT_CPU){
		pPktHdr->ingressPort=RTK_RG_PORT_CPU;
		if(pRxDesc->opts3.bit.dst_port_mask==0x20){
			if(pRxDesc->opts3.bit.rsvd==0x0){	//from protocol statck			
				pPktHdr->ingressLocation=RG_IGR_PROTOCOL_STACK;
			}else if(pRxDesc->opts3.bit.rsvd==0x1){	//from ARP or ND
				pPktHdr->ingressLocation=RG_IGR_ARP_OR_ND;
			}else if(pRxDesc->opts3.bit.rsvd==0x2){	//from IGMP or MLD
				pPktHdr->ingressLocation=RG_IGR_IGMP_OR_MLD;
			}
		}else{
#ifdef CONFIG_MASTER_WLAN0_ENABLE		
			pPktHdr->ingressPort=RTK_RG_EXT_PORT0;
#endif
		}		
	}else{
		pPktHdr->ingressPort=pRxDesc->opts3.bit.src_port_num;
	}

#endif

}

rtk_rg_fwdEngineReturn_t _rtk_rg_wlanExtraDataPathDecision(rtk_rg_pktHdr_t *pPktHdr,rtk_rg_rxdesc_t *pRxDesc,struct sk_buff *skb)
{
#ifdef CONFIG_RG_WLAN_HWNAT_ACCELERATION
		//check from which WLAN interface(root,vap0,vap1,vap2,vap3)

#ifdef CONFIG_MASTER_WLAN0_ENABLE

		if(pRxDesc==(rtk_rg_rxdesc_t *)&rg_db.systemGlobal.rxInfoFromWLAN)
		{
			_rtk_rg_wlanMbssidLearning(&skb->data[6],pPktHdr);
			TRACE("the wlan device is from %d",pPktHdr->wlan_dev_idx);
		}

#endif

		if(pRxDesc->opts3.bit.src_port_num==RTK_RG_MAC_PORT_CPU) //from CPU,EXT0,EXT1
		{
ingress_port_changed:
			if(pPktHdr->ingressPort==RTK_RG_EXT_PORT0) //from EXT0, wireless WAN
			{
				if((skb->data[0]&1)==1) //from EXT0 BC/MC to-PS
				{
					TRACE("(from EXT0 BC/MC), into FwdEngine!"); //let IGMP packets learn to HW.
					return RG_FWDENGINE_RET_SLOWPATH;
				}
				else //Unicsat
				{				
					if(pRxDesc==(rtk_rg_rxdesc_t *)&rg_db.systemGlobal.rxInfoFromWLAN) //from WIFI directly, not trap from GMAC trap
					{
						//ARP should not use shortcut, otherwise won't learn in hw
						if(pPktHdr->tagif&ARP_TAGIF)
							return RG_FWDENGINE_RET_SLOWPATH;

						//Disable HWNAT should continue to slow path.
						if(rg_db.systemGlobal.hwnat_enable == RG_HWNAT_DISABLE)
							return RG_FWDENGINE_RET_SLOWPATH;
#ifdef CONFIG_MASTER_WLAN0_ENABLE
						if(rg_db.systemGlobal.wlan0BindDecision[pPktHdr->wlan_dev_idx].set_bind)
						{
							TRACE("From EXT0 UC which from WIFI1 hit Binding, into FwdEngine!");
							return RG_FWDENGINE_RET_SLOWPATH;
						}

						if(rg_db.systemGlobal.accessWanLimitPortMask_wlan0member&(0x1<<pPktHdr->wlan_dev_idx))
						{
							TRACE("Wan Access Limit had been set for dev[%d], into FwdEngine!",pPktHdr->wlan_dev_idx);
							return RG_FWDENGINE_RET_SLOWPATH;
						}
#endif
						if(rg_db.systemGlobal.wlanDevPatternValidInACL){
							TRACE("RG ACL exist WLANDEV compare rules, into FwdEngine!");
							return RG_FWDENGINE_RET_SLOWPATH;
						}

						//20141030LUKE: we should not hwlookup ip-fragment packet, since gmac and switch both could't offload L4CS
						//20141104LUKE: and hwlookup through gmac will cause Layer4 checksum offload to wrong value, 
						//after trap fwdEngine can't recalculate to correct one since we are using difference update.
						//20141112LUKE: Consider IPv6 also.
						if((pPktHdr->ipv4FragPacket||pPktHdr->ipv6FragPacket) && ((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF))){
							TRACE("IP fragment packet, into FwdEngine!");
							return RG_FWDENGINE_RET_SLOWPATH;
						}

						//20141112LUKE: When TCP SYN packet from wifi to pure-routing WAN, we should go to slow path in case neighbor or ARP can be learned
						if((pPktHdr->tagif&TCP_TAGIF)&&(pPktHdr->tcpFlags.syn==1))
						{
							TRACE("TCP packet with SYN, into FwdEngine!");
							return RG_FWDENGINE_RET_SLOWPATH;
						}

						//HWLOOKUP
						TRACE("From EXT0 UC which from WIFI1, forward by HWLOOKUP.");
						return RG_FWDENGINE_RET_HWLOOKUP;
					}
					else
					{
						//if from GMAC and without trap reason, just send to Master; otherwise goto slow path
						if(pRxDesc->opts3.bit.reason==0)
						{
							//20140122LUKE:if there is no any interface, we just let directTX to forward.
							if(rg_db.systemGlobal.lanIntfTotalNum==0)
							{
								TRACE("there is no any LAN interface...into FwdEngine!");
								return RG_FWDENGINE_RET_SLOWPATH;	//goto normal path
							}

							TRACE("Sending EXT0 UC to WIFI1 if DMAC had been learned");
							if(_rtk_master_wlan_mbssid_tx(pPktHdr,skb)==RG_RET_MBSSID_NOT_FOUND)
							{
								TRACE("DMAC not found in MBSSID table, Drop!");
								return RG_FWDENGINE_RET_DROP;
							}

							return RG_FWDENGINE_RET_CONTINUE;
						}
					}
				}
				// from EXT0 UC normal forward
			}
#ifdef CONFIG_DUALBAND_CONCURRENT
			else if((pPktHdr->ingressPort==RTK_RG_EXT_PORT1)&&((skb->data[0]&1)==0))	//from EXT1 Unicast
			{
				if(pRxDesc->opts3.bit.reason==0)
				{
					int i;
					//normal forward to Master CPU				
					for(i=0;i<rg_db.systemGlobal.lanIntfTotalNum;i++)
					{
						if(memcmp(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet,skb->data,ETHER_ADDR_LEN)==0)
						{
							// DMAC=GMAC to Protocol Stack
							TRACE("from (WIFI2)EXT1 UC to LAN_MAC to PS!");
							return RG_FWDENGINE_RET_TO_PS;
						}
					}
					
					//20140122LUKE:if there is no any interface, we just let directTX to forward.
					if(rg_db.systemGlobal.lanIntfTotalNum==0)
					{
						TRACE("there is no any LAN interface...into FwdEngine!");
						return RG_FWDENGINE_RET_SLOWPATH;	//goto normal path
					}
						
					if(memcmp(master_ipc_macAddr.octet,skb->data,ETHER_ADDR_LEN)==0)
					{
							// DMAC=GMAC to Protocol Stack
							TRACE("from (WIFI2)EXT1 UC to LAN_MAC to PS!");
							return RG_FWDENGINE_RET_TO_PS;
					}
#ifdef CONFIG_MASTER_WLAN0_ENABLE
					if(wlan_root_netdev!=NULL)
					{
						TRACE("from (WIFI2)EXT1 UC to WIFI1(EXT0)!\n");
		
						if(_rtk_master_wlan_mbssid_tx(pPktHdr,skb)==FAIL) 
						{
							TRACE("Drop!");
							return RG_FWDENGINE_RET_DROP;
						}
						return RG_FWDENGINE_RET_CONTINUE; //free skb by wifi driver.
					}
					else
					{
						TRACE("Drop!");
						return RG_FWDENGINE_RET_DROP;
					}
#endif					
					
				}
				else
				{				
					TRACE("from (WIFI2)EXT1 UC to Master-CPU(trap) into fwdEngine!"); 
				}
			}		
			else if((pPktHdr->ingressPort==RTK_RG_EXT_PORT1)&&((skb->data[0]&1)==1))	//from EXT1 MC/BC
			{
				if(skb->data[0]==0xff)
				{
					TRACE("from (WIFI2)EXT1 BC to Master-CPU goto FwdEngine, skb_len=%d!",skb->len);
				}
				else
				{
					TRACE("from (WIFI2)EXT1 MC to Master-CPU goto FwdEngine, skb_len=%d!",skb->len);					
				}
			} 
			else if(pRxDesc->opts3.bit.dst_port_mask==0x4) // to EXT1
			{
				TRACE("unknow from CPU to (WIFI2)EXT1 packet, skb_len=%d, Drop!",skb->len);
				return RG_FWDENGINE_RET_DROP;
			}		
#endif

#ifdef CONFIG_MASTER_WLAN0_ENABLE

			else if(pRxDesc->opts3.bit.dst_port_mask==0x2) // to EXT0
			{
				if(wlan_root_netdev!=NULL)
				{	
					TRACE("CPU(GMAC1) to EXT0(WIFI1) packet, skb_len=%d!",skb->len);					
					
					if(_rtk_master_wlan_mbssid_tx(pPktHdr,skb)==FAIL) 
					{
						TRACE("Drop!");
						return RG_FWDENGINE_RET_DROP;
					}
					return RG_FWDENGINE_RET_CONTINUE; //free skb by wifi driver
				}
				else
				{
					TRACE("Drop!");
					return RG_FWDENGINE_RET_DROP;
				}
			}
#endif
	
			else  // from CPU to CPU
			{				
				//patch for ACL+CF rule will over-write acl(src ext-port  to dest-ext-portmask) rule.
				//trace SMAC is from which port? if from EXT port, modify ingress port and goto ingress_port_changed.
				if(pRxDesc->opts3.bit.reason==0)
				{	
					int l2Idx=_rtk_rg_macLookup(pPktHdr->pSmac,pPktHdr->internalVlanID);
					if(l2Idx!=RG_RET_LOOKUPIDX_NOT_FOUND)
					{
						if((rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.port==RTK_RG_PORT_CPU)&&(rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.ext_port!=0))
						{													
							TRACE("*** SMAC lookups from vlan %d and modifies ingress port(%d) to %d ***",pPktHdr->internalVlanID,pPktHdr->ingressPort,rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU);
							pPktHdr->ingressPort=rg_db.lut[l2Idx].rtk_lut.entry.l2UcEntry.ext_port+RTK_RG_PORT_CPU;
							if(pPktHdr->ingressPort==RTK_RG_EXT_PORT0)
								pRxDesc->opts3.bit.dst_port_mask=0x8;
							else if(pPktHdr->ingressPort==RTK_RG_EXT_PORT1)
								pRxDesc->opts3.bit.dst_port_mask=0x10;
							goto ingress_port_changed;
						}
					}
				}
				else // have trap reason
				{
#ifdef CONFIG_MASTER_WLAN0_ENABLE
					rtk_rg_mbssidDev_t wlan_dev_idx;
					rtk_rg_lookupIdxReturn_t retIdx=_rtk_rg_wlanMbssidLookup(pPktHdr->pSmac,&wlan_dev_idx);
					if(retIdx!=RG_RET_LOOKUPIDX_NOT_FOUND)
					{
						// This is a patch for ACL+CF latch can't get src ext port info by ACL.
						// If the packet is from Master-CPU(EXT0), the learn_jiffies will expect very close to jiffies.
						// otherwise, it is from Slave-CPU(EXT1).
						if(time_before(jiffies,rg_db.wlanMbssid[retIdx].learn_jiffies+HZ))
						{
							TRACE("This packet is from EXT0, because it is found in master mbssid table and just update the aging time.");
							pPktHdr->ingressPort=RTK_RG_EXT_PORT0;
							pRxDesc->opts3.bit.dst_port_mask=0x8;
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
							//20150427LUKE: from vxd but trap to cpu, we should not treat it as from ext0!
							if(wlan_dev_idx==RG_WWAN_WLAN0_VXD)pPktHdr->wlan_dev_idx=RG_WWAN_WLAN0_VXD;
							else if(wlan_dev_idx==RG_WWAN_WLAN1_VXD)pPktHdr->wlan_dev_idx=RG_WWAN_WLAN1_VXD;
#endif	
							goto ingress_port_changed;
						}
						else
						{
							TRACE("This packet is guessed from EXT1(It is found in master mbssid table but not update the aging time > 1sec)");
							DEBUG("#### DELETE MBSSID INFO, MAC=%02x:%02x:%02x:%02x:%02x:%02x WLAN_DEV_IDX=%d ####",
							rg_db.wlanMbssid[retIdx].mac.octet[0],
							rg_db.wlanMbssid[retIdx].mac.octet[1],
							rg_db.wlanMbssid[retIdx].mac.octet[2],
							rg_db.wlanMbssid[retIdx].mac.octet[3],
							rg_db.wlanMbssid[retIdx].mac.octet[4],
							rg_db.wlanMbssid[retIdx].mac.octet[5],
							rg_db.wlanMbssid[retIdx].wlan_dev_idx);
							memset(&rg_db.wlanMbssid[retIdx],0,sizeof(rtk_rg_table_wlan_mbssid_t));
						}
					}
					else
#endif
#ifdef CONFIG_DUALBAND_CONCURRENT
					{
						TRACE("This packet is guessed from EXT1(It doesn't found in master mbssid table)");
					}

					//we assume that this packet is from EXT1, go back to decide again!
					pPktHdr->ingressPort=RTK_RG_EXT_PORT1;
					pRxDesc->opts3.bit.dst_port_mask=0x10;
					goto ingress_port_changed;
#else
					FIXME("unknow from CPU packet, trap reason is %d!",pRxDesc->opts3.bit.reason);
#endif
				}

#if 0
				if(pRxDesc->opts3.bit.reason==207) //Unkown IPv6 MC
				{
					TRACE("From PS unkown MC DA trap to here, and will forward by FwdEngine\n");
					return RG_FWDENGINE_RET_SLOWPATH; //goto slow path (flooding to each interface)
				}
#endif				
				TRACE("CPU to CPU: Drop by src port filter, skb_len=%d! SPA=%d dstPmsk=0x%x",skb->len,pRxDesc->opts3.bit.src_port_num,pRxDesc->opts3.bit.dst_port_mask);
				return RG_FWDENGINE_RET_DROP;
			}
		}
		else //NOT FROM CPU
		{
			if((skb->data[0]&1)==0) //Unicast
			{
				if(pRxDesc->opts3.bit.dst_port_mask==0x2)  //from Physical Port to EXT0 Unicast
				{
		//			_rtk_rg_layer2LutLearning(skb,&pPktHdr->,2); //SA Learnning
		#ifdef CONFIG_MASTER_WLAN0_ENABLE	
					if(wlan_root_netdev!=NULL)
					{
						int dip_lookup_idx;
						TRACE("from Physical Port to EXT0 Unicast, send to WIFI1, skb_len=%d!",skb->len);


						//Routing/L34 to EXT0 has vlan problem (vlan already traslate by H/W) 
						if(pRxDesc->opts1.bit.l3routing==1)//Wan routing to  EXT0
						{
							//Lookup SA for LAN interface, if match, change egressVLANID
							//20150325CHUCK: if H/W routing from physical wan port to wifi, the vlan wcould be removed  by H/W and fwd to software. then vlan will be consider as WAN PVID, need to change it for egress vlan filter.  
							//comprae DIPv4 & DIPv6 in Lan IP subnet
							if(pPktHdr->tagif&IPV4_TAGIF){

								dip_lookup_idx = _rtk_rg_l3lookup(pPktHdr->ipv4Dip);
								if(rg_db.systemGlobal.interfaceInfo[dip_lookup_idx].valid==1 && rg_db.systemGlobal.interfaceInfo[dip_lookup_idx].storedInfo.is_wan==0)//valid lan intf
								{
									TRACE("to Lan[%d]..change egressVLANID to %d",dip_lookup_idx, rg_db.systemGlobal.interfaceInfo[dip_lookup_idx].p_lanIntfConf->intf_vlan_id);
									pPktHdr->internalVlanID = rg_db.systemGlobal.interfaceInfo[dip_lookup_idx].p_lanIntfConf->intf_vlan_id;
								}
							}
							else if(pPktHdr->tagif&IPV6_TAGIF)
							{
								dip_lookup_idx = _rtk_rg_v6L3lookup(pPktHdr->pIpv6Dip);
								if(rg_db.systemGlobal.interfaceInfo[dip_lookup_idx].valid==1 && rg_db.systemGlobal.interfaceInfo[dip_lookup_idx].storedInfo.is_wan==0)//valid lan intf	
								{
									TRACE("to Lan[%d]..change egressVLANID to %d",dip_lookup_idx, rg_db.systemGlobal.interfaceInfo[dip_lookup_idx].p_lanIntfConf->intf_vlan_id);
									pPktHdr->internalVlanID = rg_db.systemGlobal.interfaceInfo[dip_lookup_idx].p_lanIntfConf->intf_vlan_id;
								}
							}
							
						}
				
						if(_rtk_master_wlan_mbssid_tx(pPktHdr,skb)==FAIL) 
						{
							TRACE("Drop!");
							return RG_FWDENGINE_RET_DROP;			
						}

						return RG_FWDENGINE_RET_CONTINUE;
					}
					else
					{
						TRACE("Drop!");
						return RG_FWDENGINE_RET_DROP;
					}

		#endif
				}
#ifdef CONFIG_DUALBAND_CONCURRENT
				else if(pRxDesc->opts3.bit.dst_port_mask==0x4) //from Physical Port to EXT1 Unicast
				{
					TRACE("To EXT1 UC packets which trap to master CPU must be forwarded by fwdEngine");
				}
#endif
			}
			else if(skb->data[0]==0xff) //Broadcast
			{
				TRACE("not from CPU Broadcast goto FwdEngine, skb_len=%d!",skb->len);
			}
			else if((skb->data[0]&1)==1) //Multicast
			{
				if(rg_db.systemGlobal.initParam.igmpSnoopingEnable==0) return RG_FWDENGINE_RET_SLOWPATH; //must send to phyiscal port when igmp snooping is disabled.
#if 1			
				if(pRxDesc->opts3.bit.dst_port_mask&0x1)
				{
					if(pRxDesc->opts3.bit.reason!=0)
					{
						TRACE("Unkown Trap Reason, goto to slow path.");
						return RG_FWDENGINE_RET_SLOWPATH;
					}					
					else if((pPktHdr->tagif&IPV4_TAGIF)&&(pPktHdr->ipv4Dip>=0xe0000100)&&(pPktHdr->ipv4Dip<=0xeeffffff))
					{						
						TRACE("Drop IPv4 multicast data to CPU packet...(just send to WIFI)");
						//don't do anything here
					}
					else if((pPktHdr->tagif&IPV6_TAGIF)&&(pPktHdr->pIpv6Dip[0]==0xff)&&((pPktHdr->pIpv6Dip[1]&0x0f)==0xe))
					{
						TRACE("Drop IPv6 multicast data to CPU packet...(just send to WIFI)");
						//don't do anything here
					}
					/*else if((pPktHdr->tagif&IPV4_TAGIF)&&(pPktHdr->ipv4Dip==0xeffffffa))
					{ 
						TRACE("SSDP multicast data, to PS!");
						return RG_FWDENGINE_RET_TO_PS;
					}*/
					else
					{
						TRACE("Unkown Multicast packets send to CPU and go to slow path.");
						return RG_FWDENGINE_RET_SLOWPATH;
					}
				}
				
				if(pRxDesc->opts3.bit.dst_port_mask&0x6)  //from Physical Port to EXT1 or EXT0 Multicast, if DstPortMask have CPU Port, this packet will forward by normal path.
				{				
					TRACE("Multicast packets to EXT0 or EXT1 port will forward by master CPU.");
				
					if(memcmp(skb->data,"\x01\x80\xc2",3)==0)
					{
						TRACE("Reserved MAC: 01:80:C2:%02x:%02x:%02x, to PS!",skb->data[3],skb->data[4],skb->data[5]);
						return RG_FWDENGINE_RET_TO_PS;
					}
					
#ifdef CONFIG_DUALBAND_CONCURRENT				
					if(pRxDesc->opts3.bit.dst_port_mask&0x4)
					{
						struct sk_buff *skb_new=NULL;
						if(pRxDesc->opts3.bit.dst_port_mask&0x2)		//goto Master & Slave at same time
						{
							skb_new=_rtk_rg_skb_clone(skb,GFP_ATOMIC);
							if(skb_new==NULL) 
							{
								TRACE("Drop!");
								return RG_FWDENGINE_RET_DROP;
							}
						}
						else	//goto Slave Only, should be forwarded by hw, but some how be traped to here
						{
							skb_new=skb;
						}
					
						//send to WIFI2(EXT1)
						rg_kernel.txDescMask.opts1.dw=0;
						rg_kernel.txDescMask.opts2.dw=0;	
						rg_kernel.txDescMask.opts3.dw=0;
						//rg_kernel.txDescMask.opts4.dw=0;	//not used now!
						rg_kernel.txDesc.opts1.dw=0;
						rg_kernel.txDesc.opts2.dw=0;	
						rg_kernel.txDesc.opts3.dw=0;
	

						rg_kernel.txDescMask.opts1.bit.dislrn=1;					
						rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f; //no cpu port
						rg_kernel.txDescMask.opts2.bit.tx_vlan_action=0;// no-action

						rg_kernel.txDesc.opts1.bit.dislrn=1; // patch for reason 192				
						rg_kernel.txDesc.opts3.bit.tx_portmask=0; //HWLOOKUP (because: HW do not have extension port & CPU port bit)
						rg_kernel.txDesc.opts2.bit.tx_vlan_action=0;// intact
						
						pPktHdr->egressVlanID=CONFIG_DEFAULT_TO_SLAVE_GMAC_VID;
						pPktHdr->egressVlanTagif=1;
						pPktHdr->egressPriority=CONFIG_DEFAULT_TO_SLAVE_GMAC_PRI;

						
						TRACE("Multicast from master CPU to WIFI2 by GMAC(VID=1,PRI=4,HWLOOKUP)");	
						_rtk_rg_egressPacketSend(skb_new,pPktHdr);				
						
					}	
#endif

					if(pRxDesc->opts3.bit.dst_port_mask&0x2)
					{
#ifdef CONFIG_MASTER_WLAN0_ENABLE					
						if(wlan_root_netdev)
						{
							TRACE("Multicast to WIFI1(EXT0)");							
							if(_rtk_master_wlan_mbssid_tx(pPktHdr,skb)==FAIL) 
							{
								TRACE("Drop!");
								return RG_FWDENGINE_RET_DROP;
							}
						}
						else
						{
							TRACE("Drop!");
							return RG_FWDENGINE_RET_DROP;
						}
#else //for slave only
						TRACE("Drop!");
						return RG_FWDENGINE_RET_DROP;
#endif
					}
					return RG_FWDENGINE_RET_CONTINUE;
				}
				else
				{
					TRACE("Drop!");
					return RG_FWDENGINE_RET_DROP;
				}
#endif	
			}
		}
	
#endif
	return RG_FWDENGINE_RET_SLOWPATH;

}

void _rtk_rg_updatePPTPInfo(int wanGroupIdx, rtk_rg_pktHdr_t *pPktHdr)
{
	if(rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].p_wanIntfConf->wan_type==RTK_RG_PPTP)
	{
		if(pPktHdr->tagif&GRE_SEQ_TAGIF)
		{
			//20150617LUKE: store seq from protocol stack, for response packet can restore it's ack
			rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].p_intfInfo->storedInfo.wan_intf.pptp_info.sw_gre_header_sequence=pPktHdr->GRESequence;
			//update sequence to next value
			//rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].p_intfInfo->storedInfo.wan_intf.pptp_info.gre_header_sequence=pPktHdr->GRESequence+1;
			if(pPktHdr->GRESequence>rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].p_intfInfo->storedInfo.wan_intf.pptp_info.gre_header_sequence)
				rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].p_intfInfo->storedInfo.wan_intf.pptp_info.gre_header_sequence=pPktHdr->GRESequence;
			else
				*pPktHdr->pGRESequence=htonl(++rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].p_intfInfo->storedInfo.wan_intf.pptp_info.gre_header_sequence);
		}
		if(pPktHdr->tagif&GRE_ACK_TAGIF)
		{
			//update acknowledgment for packet from protocol stack by WAN's acknowledgment value
			*pPktHdr->pGREAcknowledgment=htonl(rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].p_intfInfo->storedInfo.wan_intf.pptp_info.gre_header_acknowledgment);
		}
	}
}

rtk_rg_fwdEngineReturn_t _rtk_rg_checkFromProtocolStackInterface(rtk_rg_pktHdr_t *pPktHdr)
{
	int i;
	
	//20140122LUKE:if there is no any interface, we just let directTX to forward.
	if(rg_db.systemGlobal.lanIntfTotalNum==0 && rg_db.systemGlobal.wanIntfTotalNum==0)
		goto egress_acl_check_and_direct_tx;
	
	if(pPktHdr->tagif&CVLAN_TAGIF)
	{
		//broadcast should not check LAN interface...LAN should follow VLAN decision!!
		if(pPktHdr->fwdDecision!=RG_FWD_DECISION_NORMAL_BC)
		{
			for(i=0;i<rg_db.systemGlobal.lanIntfTotalNum;i++)
			{
				if(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->intf_vlan_id==pPktHdr->ctagVid)
				{
					pPktHdr->netifIdx=rg_db.systemGlobal.lanIntfGroup[i].index; //keep first VLAN match interface
					TRACE("HIT LAN VLAN[%d]! netifIdx is %d",pPktHdr->ctagVid,pPktHdr->netifIdx);
					goto egress_acl_check_and_direct_tx;
				}
			}
		}
		for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
		{
			if(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->egress_vlan_id==pPktHdr->ctagVid)
			{
				pPktHdr->netifIdx=rg_db.systemGlobal.wanIntfGroup[i].index; //keep first VLAN match interface
				TRACE("HIT WAN VLAN[%d]! netifIdx is %d",pPktHdr->ctagVid,pPktHdr->netifIdx);
				//20141002LUKE: check for PPTP WAN, update the sequence and acknowledgment
				_rtk_rg_updatePPTPInfo(i,pPktHdr);
				goto egress_acl_check_and_direct_tx;
			}
		}
	}
	else
	{
		//untag, so check untag interface only
		//choose SA and SIP both match interface first, then choose SA match only interface
		
		//broadcast should not check LAN interface...LAN should follow VLAN decision!!
		if(pPktHdr->fwdDecision!=RG_FWD_DECISION_NORMAL_BC)
		{
			for(i=0;i<rg_db.systemGlobal.lanIntfTotalNum;i++)
			{
				if((rg_db.vlan[rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->intf_vlan_id].UntagPortmask.bits[0]&(0x1<<RTK_RG_PORT_CPU))==0)
					continue;
				
				if(!memcmp(pPktHdr->pSmac,rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->gmac.octet,ETHER_ADDR_LEN))
				{
					if(pPktHdr->netifIdx==FAIL)
						pPktHdr->netifIdx=rg_db.systemGlobal.lanIntfGroup[i].index; //keep first MAC match LAN interface
						
					if(pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&ARP_TAGIF)
					{
						if(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->ip_version!=IPVER_V6ONLY && 
							pPktHdr->ipv4Sip==rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->ip_addr)
						{
							pPktHdr->netifIdx=rg_db.systemGlobal.lanIntfGroup[i].index;
							TRACE("HIT LAN MAC[%02x:%02x:%02x:%02x:%02x:%02x] with IP[%x]! netifIdx is %d",
								pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],pPktHdr->pSmac[3],
								pPktHdr->pSmac[4],pPktHdr->pSmac[5],pPktHdr->ipv4Sip,pPktHdr->netifIdx);
							goto egress_acl_check_and_direct_tx;
						}
					}
					else if(pPktHdr->tagif&IPV6_TAGIF)
					{
						if(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->ip_version!=IPVER_V4ONLY && 
							!memcmp(pPktHdr->pIpv6Sip,rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->ipv6_addr.ipv6_addr,IPV6_ADDR_LEN))
						{
							pPktHdr->netifIdx=rg_db.systemGlobal.lanIntfGroup[i].index;
							TRACE("HIT LAN MAC[%02x:%02x:%02x:%02x:%02x:%02x] with IP[%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x]! netifIdx is %d",
								pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],pPktHdr->pSmac[3],pPktHdr->pSmac[4],pPktHdr->pSmac[5],
								pPktHdr->pIpv6Sip[0],pPktHdr->pIpv6Sip[1],pPktHdr->pIpv6Sip[2],pPktHdr->pIpv6Sip[3],
								pPktHdr->pIpv6Sip[4],pPktHdr->pIpv6Sip[5],pPktHdr->pIpv6Sip[6],pPktHdr->pIpv6Sip[7],
								pPktHdr->pIpv6Sip[8],pPktHdr->pIpv6Sip[9],pPktHdr->pIpv6Sip[10],pPktHdr->pIpv6Sip[11],
								pPktHdr->pIpv6Sip[12],pPktHdr->pIpv6Sip[13],pPktHdr->pIpv6Sip[14],pPktHdr->pIpv6Sip[15],
								pPktHdr->netifIdx);
							goto egress_acl_check_and_direct_tx;
						}
					}
				}
			}
			if(pPktHdr->netifIdx!=FAIL)
			{
				TRACE("HIT LAN MAC[%02x:%02x:%02x:%02x:%02x:%02x]! netifIdx is %d",
					pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],pPktHdr->pSmac[3],
					pPktHdr->pSmac[4],pPktHdr->pSmac[5],pPktHdr->netifIdx);
				goto egress_acl_check_and_direct_tx;	//no IP-MAC match, so choose MAC match
			}
		}

		//20150407LUKE: check PPTP WAN first!!
		if((pPktHdr->tagif&GRE_TAGIF)&&(pPktHdr->tagif&IPV4_TAGIF))
		{
			for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
			{
				if(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type!=RTK_RG_PPTP)
					continue;

				//20150408LUKE: PPTP WAN may has different MAC address with Based WAN, so we don't check MAC here.
				//if(!memcmp(pPktHdr->pSmac,rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->gmac.octet,ETHER_ADDR_LEN))
				//{
					//DEBUG("*pPktHdr->pGRECallID is %d, gateway is %d",*pPktHdr->pGRECallID,rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf.pptp_info.after_dial.gateway_callId);
					if(rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ip_version!=IPVER_V6ONLY &&
						pPktHdr->ipv4Dip==rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf.pptp_info.before_dial.pptp_ipv4_addr &&
						*pPktHdr->pGRECallID==rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf.pptp_info.after_dial.gateway_callId)
					{
						pPktHdr->netifIdx=rg_db.systemGlobal.wanIntfGroup[i].index;
						TRACE("HIT PPTP WAN MAC[%02x:%02x:%02x:%02x:%02x:%02x] with IP[%x]! netifIdx is %d",
							pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],pPktHdr->pSmac[3],
							pPktHdr->pSmac[4],pPktHdr->pSmac[5],pPktHdr->ipv4Sip,pPktHdr->netifIdx);
						//20150107LUKE: check for PPTP WAN, update the sequence and acknowledgment
						_rtk_rg_updatePPTPInfo(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].lan_or_wan_index,pPktHdr);
						goto egress_acl_check_and_direct_tx;
					}
				//}
			}
		}
		
		for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++)
		{
			if((rg_db.vlan[rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->egress_vlan_id].UntagPortmask.bits[0]&(0x1<<RTK_RG_PORT_CPU))==0)
				continue;
			
			if(!memcmp(pPktHdr->pSmac,rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->gmac.octet,ETHER_ADDR_LEN))
			{
				if(pPktHdr->netifIdx==FAIL)
					pPktHdr->netifIdx=rg_db.systemGlobal.wanIntfGroup[i].index; //keep first MAC match WAN interface

				if(pPktHdr->tagif&IPV4_TAGIF || pPktHdr->tagif&ARP_TAGIF)
				{
					if(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type!=RTK_RG_BRIDGE && 
						rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ip_version!=IPVER_V6ONLY &&
						pPktHdr->ipv4Sip==rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ip_addr)
					{
						pPktHdr->netifIdx=rg_db.systemGlobal.wanIntfGroup[i].index;
						TRACE("HIT WAN MAC[%02x:%02x:%02x:%02x:%02x:%02x] with IP[%x]! netifIdx is %d",
							pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],pPktHdr->pSmac[3],
							pPktHdr->pSmac[4],pPktHdr->pSmac[5],pPktHdr->ipv4Sip,pPktHdr->netifIdx);
						//20150107LUKE: check for PPTP WAN, update the sequence and acknowledgment
						_rtk_rg_updatePPTPInfo(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].lan_or_wan_index,pPktHdr);
						goto egress_acl_check_and_direct_tx;
					}
				} 
				else if(pPktHdr->tagif&IPV6_TAGIF)
				{
					if(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type!=RTK_RG_BRIDGE && 
						rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ip_version!=IPVER_V4ONLY &&
						!memcmp(pPktHdr->pIpv6Sip,rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ipv6_addr.ipv6_addr,IPV6_ADDR_LEN))
					{
						pPktHdr->netifIdx=rg_db.systemGlobal.wanIntfGroup[i].index;
						TRACE("HIT WAN MAC[%02x:%02x:%02x:%02x:%02x:%02x] with IP[%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x:%x%x]! netifIdx is %d",
							pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],pPktHdr->pSmac[3],pPktHdr->pSmac[4],pPktHdr->pSmac[5],
							pPktHdr->pIpv6Sip[0],pPktHdr->pIpv6Sip[1],pPktHdr->pIpv6Sip[2],pPktHdr->pIpv6Sip[3],
							pPktHdr->pIpv6Sip[4],pPktHdr->pIpv6Sip[5],pPktHdr->pIpv6Sip[6],pPktHdr->pIpv6Sip[7],
							pPktHdr->pIpv6Sip[8],pPktHdr->pIpv6Sip[9],pPktHdr->pIpv6Sip[10],pPktHdr->pIpv6Sip[11],
							pPktHdr->pIpv6Sip[12],pPktHdr->pIpv6Sip[13],pPktHdr->pIpv6Sip[14],pPktHdr->pIpv6Sip[15],
							pPktHdr->netifIdx);
						//20150107LUKE: check for PPTP WAN, update the sequence and acknowledgment
						_rtk_rg_updatePPTPInfo(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].lan_or_wan_index,pPktHdr);
						goto egress_acl_check_and_direct_tx;
					}
				}
			}
		}
		if(pPktHdr->netifIdx!=FAIL)
		{
			TRACE("HIT WAN MAC[%02x:%02x:%02x:%02x:%02x:%02x]! netifIdx is %d",
				pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],pPktHdr->pSmac[3],
				pPktHdr->pSmac[4],pPktHdr->pSmac[5],pPktHdr->netifIdx);
			//20141002LUKE: check for PPTP WAN, update the sequence and acknowledgment
			_rtk_rg_updatePPTPInfo(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].lan_or_wan_index,pPktHdr);
			goto egress_acl_check_and_direct_tx;	//no IP-MAC match, so choose MAC match
		}
	}
	
#ifdef CONFIG_DUALBAND_CONCURRENT
	//from master WiFi IPC, let it do layer2 forward by hardware lookup;
	if(!memcmp(pPktHdr->pSmac,master_ipc_macAddr.octet,ETHER_ADDR_LEN) /*|| !memcmp(pPktHdr->pSmac,slave_ipc_macAddr.octet,ETHER_ADDR_LEN)*/)
		return RG_FWDENGINE_RET_HWLOOKUP;
#endif

	//from protocol stack don't match any gateway mac??dump it here....
	TRACE("strange packet from protocol stack...SA=%02x:%02x:%02x:%02x:%02x:%02x, DA=%02x:%02x:%02x:%02x:%02x:%02x.......Continue!",
	pPktHdr->pSmac[0],pPktHdr->pSmac[1],pPktHdr->pSmac[2],pPktHdr->pSmac[3],pPktHdr->pSmac[4],pPktHdr->pSmac[5],
	pPktHdr->pDmac[0],pPktHdr->pDmac[1],pPktHdr->pDmac[2],pPktHdr->pDmac[3],pPktHdr->pDmac[4],pPktHdr->pDmac[5]);
	//dump_packet(skb->data,skb->len,"do not match smac packet");
	return RG_FWDENGINE_RET_CONTINUE;

egress_acl_check_and_direct_tx:
	return RG_FWDENGINE_RET_DIRECT_TX;
}

#ifdef CONFIG_MASTER_WLAN0_ENABLE
#if 0
int _rtk_rg_wlanInsert1QTag(rtk_rg_pktHdr_t *pPktHdr)
{
	//Insert CVLAN tag only DeviceVID is different to PortBasedVID of ext0 port.
	if(rg_db.systemGlobal.portBasedVID[RTK_RG_EXT_PORT0]!=rg_db.systemGlobal.wlan0DeviceBasedVID[pPktHdr->wlan_dev_idx])
	{
		uint16 vlanContent;
		vlanContent = (((rg_db.vlan[rg_db.systemGlobal.wlan0DeviceBasedVID[pPktHdr->wlan_dev_idx]].priority&0x7)<<13)|(rg_db.systemGlobal.wlan0DeviceBasedVID[pPktHdr->wlan_dev_idx]&0xfff));
		_vlan_insert_tag(NULL,pPktHdr->skb,1,0x8100,vlanContent,0,0x0,0x0);
		
		TRACE("WLAN_TX_O[%x]: from Ext0 Dev[%d] insert CVLAN tag(VID=%d, Pri=%d)",(unsigned int)pPktHdr->skb&0xffff,pPktHdr->wlan_dev_idx,rg_db.systemGlobal.wlan0DeviceBasedVID[pPktHdr->wlan_dev_idx],rg_db.vlan[rg_db.systemGlobal.wlan0DeviceBasedVID[pPktHdr->wlan_dev_idx]].priority);
	}
	return (RG_FWDENGINE_RET_CONTINUE);
}
#endif

rtk_rg_fwdEngineReturn_t _rtk_rg_wlan_decision(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr)
{
	int i;

	//decide src wlan device index
	for(i=0;i<MAX_WLAN_DEVICE_NUM;i++)
	{
		switch(i)
		{
			case 0:
				if(wlan_root_netdev && skb->dev==wlan_root_netdev)pPktHdr->wlan_dev_idx=i;
				break;
			case 1:
			case 2:
			case 3:
			case 4:
				if(wlan_vap_netdev[i-1] && skb->dev==wlan_vap_netdev[i-1])pPktHdr->wlan_dev_idx=i;
				break;
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
				if(wlan_wds_netdev[i-5] && skb->dev==wlan_wds_netdev[i-5])pPktHdr->wlan_dev_idx=i;
				break;
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
			case 13:
				if(wlan_vxd_netdev && skb->dev==wlan_vxd_netdev)pPktHdr->wlan_dev_idx=i;
				break;
#endif
#if defined(CONFIG_RG_WLAN_HWNAT_ACCELERATION) && !defined(CONFIG_ARCH_LUNA_SLAVE)				
			case WLAN_DEVICE_NUM:
				if(wlan1_root_netdev && skb->dev==wlan1_root_netdev)pPktHdr->wlan_dev_idx=i;
				break;
			case WLAN_DEVICE_NUM+1:
			case WLAN_DEVICE_NUM+2:
			case WLAN_DEVICE_NUM+3:
			case WLAN_DEVICE_NUM+4:
				if(wlan1_vap_netdev[i-WLAN_DEVICE_NUM-1] && skb->dev==wlan1_vap_netdev[i-WLAN_DEVICE_NUM-1])pPktHdr->wlan_dev_idx=i;
				break;
			case WLAN_DEVICE_NUM+5:
			case WLAN_DEVICE_NUM+6:
			case WLAN_DEVICE_NUM+7:
			case WLAN_DEVICE_NUM+8:
			case WLAN_DEVICE_NUM+9:
			case WLAN_DEVICE_NUM+10:
			case WLAN_DEVICE_NUM+11:
			case WLAN_DEVICE_NUM+12:
				if(wlan1_wds_netdev[i-WLAN_DEVICE_NUM-5] && skb->dev==wlan1_wds_netdev[i-WLAN_DEVICE_NUM-5])pPktHdr->wlan_dev_idx=i;
				break;
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
			case WLAN_DEVICE_NUM+13:
				if(wlan1_vxd_netdev && skb->dev==wlan1_vxd_netdev)pPktHdr->wlan_dev_idx=i;
				break;
#endif
#endif
			default:
				break;
		}
		if(pPktHdr->wlan_dev_idx!=FAIL)break;
	}	
	
	if(pPktHdr->wlan_dev_idx==FAIL)
	{
		if(_rtk_rg_wlanMbssidLookup(skb->data+6,&pPktHdr->wlan_dev_idx)==RG_RET_LOOKUPIDX_NOT_FOUND)
		{
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
			//20150515LUKE: from WWAN to WLAN will do hwlookup and forward to CPU, 
			//therefore we can't decide src wlan_dev_idx, but we can still forward this packet for known DMAC
			if((!pPktHdr->pRxDesc->opts1.bit.origformat)&&(pPktHdr->pRxDesc->opts1.bit.l3routing))
			{
				TRACE("packet from wifi to wifi...try to send!");
				return RG_FWDENGINE_RET_CONTINUE;
			}
#endif
			TRACE("strange master wifi packet....can't decide wlan_dev_idx, Drop!");
			return RG_FWDENGINE_RET_DROP;
		}
	}
	TRACE("ppkthdr->wlan_dev_idx is %d",pPktHdr->wlan_dev_idx);
	//20150610LUKE: we didn't add 1Q since we can decide ingress cvid from ssid by wlan idx
	//if(!pPktHdr->pRxDesc->opts3.bit.reason)
	//{
		//ret=_rtk_rg_wlanInsert1QTag(pPktHdr);
	//}

	//20151106LUKE: wifi ingress rate limie check here
	if(rg_db.systemGlobal.wifiIngressRateLimitMeter[pPktHdr->wlan_dev_idx]){
		if(rg_db.systemGlobal.wifiIngressRateLimitDevOverMask&(0x1<<pPktHdr->wlan_dev_idx))goto OVERLIMIT_DROP;
		rg_db.systemGlobal.wifiIngressByteCount[pPktHdr->wlan_dev_idx] += skb->len;
		if(rg_db.systemGlobal.wifiIngressByteCount[pPktHdr->wlan_dev_idx]<<3/*flow bits in time period*/ > (rg_db.systemGlobal.wifiIngressRateLimitMeter[pPktHdr->wlan_dev_idx]<<6/*Kbps*/*RTK_RG_SWRATELIMIT_SECOND/*unit:(1/16)sec*/)/*rate limit bits in time period*/){
			rg_db.systemGlobal.wifiIngressRateLimitDevOverMask|=(0x1<<pPktHdr->wlan_dev_idx);
			goto OVERLIMIT_DROP;
		}
	}

	return RG_FWDENGINE_RET_CONTINUE;

OVERLIMIT_DROP:
	TRACE("Drop! Wlan[%d] ingress packet rate higher than %d kbps.",pPktHdr->wlan_dev_idx,rg_db.systemGlobal.wifiIngressRateLimitMeter[pPktHdr->wlan_dev_idx]);
	return RG_FWDENGINE_RET_DROP;
}
#endif


rtk_rg_fwdEngineReturn_t _rtk_rg_unknownDARateLimit_check(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr){

	u32 len=skb->len;
	uint32 rate;
	rtk_rg_enable_t ifgInclude;	

	if(rg_db.systemGlobal.vlanInit==0) RETURN_ERR(RT_ERR_RG_NOT_INIT);

    if(len==0) return RG_FWDENGINE_RET_CONTINUE;
  
	//rate check
	if(rg_db.systemGlobal.unKnownDARateLimitShareMeterIdx >=0 && rg_db.systemGlobal.unKnownDARateLimitShareMeterIdx<MAX_SHAREMETER_TABLE_SIZE){//meterIdx valid
		if(rg_db.systemGlobal.unKnownDARateLimitPortMask & (1<<(pPktHdr->ingressPort))){//valid port
			int ret;
			rg_db.systemGlobal.unKnownDAByteCount += len;
			ret=(pf.rtk_rg_shareMeter_get)(rg_db.systemGlobal.unKnownDARateLimitShareMeterIdx, &rate , &ifgInclude);
			assert_ok(ret);
			//rtlglue_printf("_rtk_rg_unknownDARateLimit_check couint=%d rate_limit=%d\n",rg_db.systemGlobal.unKnownDAByteCount*8,(rate*1024*RTK_RG_SWRATELIMIT_SECOND/16));
			if(rg_db.systemGlobal.unKnownDAByteCount*8/*flow bits in time period*/ > (rate*1024/*Kbps*/*RTK_RG_SWRATELIMIT_SECOND/16/*unit:(1/16)sec*/)/*rate limit bits in time period*/){
				TRACE("Drop! unknownDA packet rate higher than shareMeter[%d]",rg_db.systemGlobal.unKnownDARateLimitShareMeterIdx);
				//rtlglue_printf("Drop! unknownDA packet rate higher than shareMeter[%d] byte_count=%d\n\n",rg_db.systemGlobal.unKnownDARateLimitShareMeterIdx,rg_db.systemGlobal.unKnownDAByteCount);
				return RG_FWDENGINE_RET_DROP;
			}	
		}
	}
	return RG_FWDENGINE_RET_CONTINUE;
}


rtk_rg_fwdEngineReturn_t _rtk_rg_BCMCRateLimit_check(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr){

	u8 *pData=skb->data;
	u32 len=skb->len;
	uint32 rate;
	rtk_rg_enable_t ifgInclude;

	if(rg_db.systemGlobal.vlanInit==0) return RG_FWDENGINE_RET_CONTINUE;

    if(len==0) return RG_FWDENGINE_RET_CONTINUE;


  
	//BC check
	if(rg_db.systemGlobal.BCRateLimitShareMeterIdx >=0 && rg_db.systemGlobal.BCRateLimitShareMeterIdx<MAX_SHAREMETER_TABLE_SIZE){//meterIdx valid
		if(rg_db.systemGlobal.BCRateLimitPortMask & (1<<(pPktHdr->ingressPort))){//valid port
			if(pData[0]==0xff && pData[1]==0xff && pData[2]==0xff && 
				pData[3]==0xff && pData[4]==0xff && pData[5]==0xff){
				int ret;

				rg_db.systemGlobal.BCByteCount += len;
				ret=(pf.rtk_rg_shareMeter_get)(rg_db.systemGlobal.BCRateLimitShareMeterIdx, &rate , &ifgInclude);
				assert_ok(ret);
				if(rg_db.systemGlobal.BCByteCount*8/*flow bits in time period*/ > (rate*1024/*Kbps*/*RTK_RG_SWRATELIMIT_SECOND/16/*unit:(1/16)sec*/)/*rate limit bits in time period*/){
					TRACE("Drop! BC packet rate higher than shareMeter[%d]",rg_db.systemGlobal.BCRateLimitShareMeterIdx);
					return RG_FWDENGINE_RET_DROP;
				}
			}
		}
	}

	//IPv6 MC check
	if(rg_db.systemGlobal.IPv6MCRateLimitShareMeterIdx >=0 && rg_db.systemGlobal.IPv6MCRateLimitShareMeterIdx<MAX_SHAREMETER_TABLE_SIZE){//meterIdx valid
		if(rg_db.systemGlobal.IPv6MCRateLimitPortMask & (1<<(pPktHdr->ingressPort))){//valid port
			if(pData[0]==0x33 && pData[1]==0x33){
				rg_db.systemGlobal.IPv6MCByteCount += len;
				assert_ok((pf.rtk_rg_shareMeter_get)(rg_db.systemGlobal.IPv6MCRateLimitShareMeterIdx, &rate , &ifgInclude));
				if(rg_db.systemGlobal.IPv6MCByteCount*8/*flow bits in time period*/ > (rate*1024/*Kbps*/*RTK_RG_SWRATELIMIT_SECOND/16/*unit:(1/16)sec*/)/*rate limit bits in time period*/){
					TRACE("Drop! IPv6 MC packet rate higher than shareMeter[%d]",rg_db.systemGlobal.BCRateLimitShareMeterIdx);
					return RG_FWDENGINE_RET_DROP;
				}

			}
		}
	}

	//IPv4 MC check
	if(rg_db.systemGlobal.IPv4MCRateLimitShareMeterIdx >=0 && rg_db.systemGlobal.IPv4MCRateLimitShareMeterIdx<MAX_SHAREMETER_TABLE_SIZE){//meterIdx valid
		if(rg_db.systemGlobal.IPv4MCRateLimitPortMask & (1<<(pPktHdr->ingressPort))){//valid port
			if(pData[0]==0x01 && pData[1]==0x00 && pData[2]==0x5e){
				rg_db.systemGlobal.IPv4MCByteCount += len;
				assert_ok((pf.rtk_rg_shareMeter_get)(rg_db.systemGlobal.IPv4MCRateLimitShareMeterIdx, &rate , &ifgInclude));
				if(rg_db.systemGlobal.IPv4MCByteCount*8/*flow bits in time period*/ > (rate*1024/*Kbps*/*RTK_RG_SWRATELIMIT_SECOND/16/*unit:(1/16)sec*/)/*rate limit bits in time period*/){
					TRACE("Drop! IPv4 MC packet rate higher than shareMeter[%d]",rg_db.systemGlobal.BCRateLimitShareMeterIdx);
					return RG_FWDENGINE_RET_DROP;
				}

			}
		}
	}

	return RG_FWDENGINE_RET_CONTINUE;


}

#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
rtk_rg_fwdEngineReturn_t _rtk_rg_ipv6StatefulDecision(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr)
{
	rtk_rg_fwdEngineReturn_t ret;
	rtk_rg_ipv6_layer4_linkList_t *pIPv6ConnList=NULL,*pIPv6FragList=NULL;
			
	pPktHdr->ipv6StatefulHashValue=-1;
	pPktHdr->pIPv6FragmentList=NULL;

	//SYN, RST, FIN should goto slow path
	if(pPktHdr->tagif&TCP_TAGIF)
	{
		if((pPktHdr->tcpFlags.syn==1) || (pPktHdr->tcpFlags.fin==1) || (pPktHdr->tcpFlags.reset==1))
			return RG_FWDENGINE_RET_CONTINUE;	//bypass IPv4 shortcut

		if(pPktHdr->tagif&V6FRAG_TAGIF)
		{
			_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pIPv6FragList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->ipv6FragId_First,pPktHdr->ipv6FragId_Second,1,1);
			if(pIPv6FragList!=NULL)
			{
				TRACE("TCP HIT Fragment[%p](action:%d), use its conn list[%p]",pIPv6FragList,pIPv6FragList->fragAction,pIPv6FragList->pPair_list);
				pIPv6ConnList=pIPv6FragList->pPair_list;
				pIPv6FragList->receivedLength+=pPktHdr->ipv6PayloadLen;
				DEBUG("pIPv6FragList->receivedLength become %d, total is %d",pIPv6FragList->receivedLength,pIPv6FragList->totalLength);
				pIPv6FragList->beginIdleTime=jiffies;
				pPktHdr->pIPv6FragmentList=pIPv6FragList;
			}
			else
				return RG_FWDENGINE_RET_CONTINUE;	//fragment not created, goto slow path
		}
		else
			_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pIPv6ConnList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport,1,0);
			
	}
	else	//UDP
	{
		if(pPktHdr->tagif&V6FRAG_TAGIF)
		{
			DEBUG("UDP fragement lookup id is %04x%04x",pPktHdr->ipv6FragId_First,pPktHdr->ipv6FragId_Second);
			_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pIPv6FragList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->ipv6FragId_First,pPktHdr->ipv6FragId_Second,0,1);	
			if(pIPv6FragList!=NULL)
			{
				TRACE("UDP HIT Fragment[%p](action:%d), use its conn list[%p]",pIPv6FragList,pIPv6FragList->fragAction,pIPv6FragList->pPair_list);
				pIPv6ConnList=pIPv6FragList->pPair_list;
				pIPv6FragList->receivedLength+=pPktHdr->ipv6PayloadLen;
				DEBUG("pIPv6FragList->receivedLength become %d, total is %d",pIPv6FragList->receivedLength,pIPv6FragList->totalLength);
				pIPv6FragList->beginIdleTime=jiffies;
				pPktHdr->pIPv6FragmentList=pIPv6FragList;
			}
			else
				return RG_FWDENGINE_RET_CONTINUE;	//fragment not created, goto slow path
		}
		else
			_rtk_rg_fwdEngine_ipv6ConnList_lookup(&pIPv6ConnList,&pPktHdr->ipv6StatefulHashValue,pPktHdr->pIpv6Sip,pPktHdr->pIpv6Dip,pPktHdr->sport,pPktHdr->dport,0,0);		
	}
	
	if(pIPv6ConnList!=NULL && (pIPv6ConnList->state==TCP_CONNECTED || pIPv6ConnList->state==UDP_CONNECTED))
	{
		if(rg_db.lut[pIPv6ConnList->dmacL2Idx].valid==0 || 
#if defined(CONFIG_RTL9600_SERIES)
#else	//support lut traffic bit
			rg_db.lut[pIPv6ConnList->smacL2Idx].valid==0 || 
#endif
			(pIPv6ConnList->neighborIdx>=0 && rg_db.v6neighbor[pIPv6ConnList->neighborIdx].rtk_v6neighbor.valid==0)) 
		{
			TABLE("del v6 stateful SC.");
			//------------------ Critical Section start -----------------------//
			rg_lock(&rg_kernel.ipv6StatefulLock);
			_rtk_rg_fwdEngine_ipv6ConnList_del(pIPv6ConnList);
			//------------------ Critical Section End -----------------------//
			rg_unlock(&rg_kernel.ipv6StatefulLock);
			
			return RG_FWDENGINE_RET_CONTINUE;
		}
		
		TRACE("hash[%d] hit!! netifidx is %d, dmacL2idx is %d, direct is %d, wantype is %d",
			pPktHdr->ipv6StatefulHashValue,
			pIPv6ConnList->netifIdx,pIPv6ConnList->dmacL2Idx,
			pIPv6ConnList->direction,pIPv6ConnList->wanType);
		
		pIPv6ConnList->idleSecs=0;
		pPktHdr->pIPv6StatefulList=pIPv6ConnList;
		pPktHdr->netifIdx=pIPv6ConnList->netifIdx;
		pPktHdr->dmacL2Idx=pIPv6ConnList->dmacL2Idx;
		
		TRACE("hash[%d] hit!! addr is %p , valid is %d ",pPktHdr->ipv6StatefulHashValue,pPktHdr->pIPv6StatefulList,pIPv6ConnList->valid);
		ret = _rtk_rg_fwdEngine_ipv6PacketModify(pIPv6ConnList->direction,pIPv6ConnList->wanType,pPktHdr,skb);
		if(ret!=RG_FWDENGINE_RET_CONTINUE)
			return ret;		//RG_FWDENGINE_RET_TO_PS

		pPktHdr->egressDSCP=pIPv6ConnList->dscp>=0?pIPv6ConnList->dscp:((((*pPktHdr->pTos)&0xf)<<2) | (((*(pPktHdr->pTos+1))&0xc0)>>6));	//keep original DSCP if no ACL or remarking
		pPktHdr->egressVlanID=pIPv6ConnList->vlanID;
		pPktHdr->egressVlanTagif=pIPv6ConnList->vlanTagif;
		pPktHdr->egressServiceVlanID=pIPv6ConnList->serviceVlanID;
		pPktHdr->egressServiceVlanTagif=pIPv6ConnList->serviceVlanTagif;
		pPktHdr->egressTagAccType=pIPv6ConnList->tagAccType;
		pPktHdr->dmac2VlanID=pIPv6ConnList->dmac2cvlanID;
		pPktHdr->dmac2VlanTagif=pIPv6ConnList->dmac2cvlanTagif;
		pPktHdr->egressMACPort=pIPv6ConnList->macPort;
		pPktHdr->egressExtPort=pIPv6ConnList->extPort;
		pPktHdr->egressPriority=pIPv6ConnList->priority;
		pPktHdr->egressServicePriority=pIPv6ConnList->servicePriority;
		pPktHdr->internalVlanID=pIPv6ConnList->internalVlanID;
		pPktHdr->internalPriority=pIPv6ConnList->internalCFPri;
		pPktHdr->egressUniPortmask=pIPv6ConnList->uniPortmask;
#if defined(CONFIG_RTL9602C_SERIES)
		//mib counter and cf decision
		pPktHdr->mibNetifIdx=pIPv6ConnList->mibNetifIdx;
		pPktHdr->mibDirect=pIPv6ConnList->mibDirect;
		pPktHdr->mibTagDelta=pIPv6ConnList->mibTagDelta;
#endif
		//20150922LUKE: update Neighbor idle time when hit stateful!
		if(pIPv6ConnList->neighborIdx>=0)
		{
			rg_db.v6neighbor[pIPv6ConnList->neighborIdx].idleSecs=0;
		}
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
		rg_db.lut[pIPv6ConnList->smacL2Idx].idleSecs=0;
#endif

#ifdef CONFIG_GPON_FEATURE						
		pPktHdr->streamID=pIPv6ConnList->streamID;
#endif
		pPktHdr->shortcutStatus=RG_SC_MATCH;
		pPktHdr->fwdDecision=RG_FWD_DECISION_V6ROUTING;	//for initialize it

		if(pIPv6FragList!=NULL)
		{
			//filled by first packet or other early fragment packets
			if(pPktHdr->ipv6MoreFragment==0)		//we can not free fragList here, unless there are all packets had forwarded
				pIPv6FragList->totalLength=(pPktHdr->ipv6FragmentOffset<<3)+pPktHdr->ipv6PayloadLen;
			DEBUG("pIPv6FragList[%p]->fragAction is %d, receivedLength became %d, total is %d",pIPv6FragList,pIPv6FragList->fragAction,pIPv6FragList->receivedLength,pIPv6FragList->totalLength);
			if(pIPv6FragList->totalLength>0 && pIPv6FragList->receivedLength>=pIPv6FragList->totalLength)
			{
				DEBUG("all frag packet are out, free the list...");
				pIPv6FragList->pPair_list=NULL;		//leave conn list un-touch
				//------------------ Critical Section start -----------------------//
				rg_lock(&rg_kernel.ipv6StatefulLock);
				_rtk_rg_fwdEngine_ipv6ConnList_del(pIPv6FragList);
				//------------------ Critical Section End -----------------------//
				rg_unlock(&rg_kernel.ipv6StatefulLock);
			}
		}

		return RG_FWDENGINE_RET_DIRECT_TX;
	}
	
	return RG_FWDENGINE_RET_CONTINUE;	//bypass IPv4 shortcut
}
#endif

void _rtk_rg_packetEgressValueInit(rtk_rg_pktHdr_t *pPktHdr)
{
	//SVLAN
	if(pPktHdr->tagif&SVLAN_TAGIF)
	{
		//pPktHdr->egressServiceVlanTagif=1;
		pPktHdr->egressServiceVlanTagif=0;
	}
	//CVLAN
	if(pPktHdr->tagif&CVLAN_TAGIF)
	{
		pPktHdr->egressVlanTagif=1;
		pPktHdr->egressVlanID=pPktHdr->ctagVid;//record to egress vid if no need to change
		pPktHdr->egressPriority=pPktHdr->ctagPri;//record to egress priority if no need to qos remarking or do ACL
	}
	//DSCP
	pPktHdr->egressDSCP = FAIL;

	/* Handle VLAN decision*/
	assert_ok(_rtk_rg_ingressVlanDecision(pPktHdr,&pPktHdr->internalVlanID));
	pPktHdr->egressVlanID=pPktHdr->internalVlanID;
	pPktHdr->dmac2VlanID=FAIL;
	
	//ALE action
	pPktHdr->sipL3Idx=FAIL;
	pPktHdr->dipL3Idx=FAIL;
	//Patch for Layer2 when check ACL pattern without netifIdx will cause error
	if(pPktHdr->ingressLocation==RG_IGR_ARP_OR_ND)
	{
		pPktHdr->netifIdx=pPktHdr->pRxDesc->opts2.bit.rsvd_1;		//netif from ARP or ND		
	}
	else
		pPktHdr->netifIdx=FAIL;
	pPktHdr->extipIdx=FAIL;
	pPktHdr->dipArpOrNBIdx=FAIL;
	pPktHdr->nexthopIdx=FAIL;	
	pPktHdr->dmacL2Idx=FAIL;
	pPktHdr->bindNextHopIdx=FAIL;
	pPktHdr->aclPolicyRoute=FAIL;
	pPktHdr->layer2BindNetifIdx=FAIL;
	pPktHdr->dipArpOrNBIdx=FAIL;		
	pPktHdr->naptOutboundIndx=FAIL;
	pPktHdr->naptrInboundIndx=FAIL;
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT			
	pPktHdr->ipv6StatefulHashValue=FAIL;
#endif

#if defined(CONFIG_RTL9602C_SERIES)
	//mib counter and cf decision
	pPktHdr->mibNetifIdx=FAIL;
	pPktHdr->mibDirect=RTK_RG_CLASSIFY_DIRECTION_END;
#endif

}


rtk_rg_fwdEngineReturn_t _rtk_rg_pppoe_multicast_process(rtk_rg_pktHdr_t *pPktHdr)
{
	int i,j,ret=-1,not_dslite=1;
	rtk_portmask_t out_mac_pmask, out_ext_pmask;
	uint32 *mcPmsk, *mcExtPmsk;
	rtk_lut_entry_type_t mc_type;

	//check if session ID and MAC match the WAN
	for(i=0;i<rg_db.systemGlobal.wanIntfTotalNum;i++){
		if(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type==RTK_RG_PPPoE &&
			rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf.pppoe_info.after_dial.sessionId==pPktHdr->sessionId &&
			!memcmp(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->gmac.octet,pPktHdr->pDmac,ETHER_ADDR_LEN))
			break;
		else if(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->wan_type==RTK_RG_PPPoE_DSLITE &&
			rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->storedInfo.wan_intf.pppoe_dslite_info.after_dial.sessionId==pPktHdr->sessionId &&
			!memcmp(rg_db.systemGlobal.wanIntfGroup[i].p_wanIntfConf->gmac.octet,pPktHdr->pDmac,ETHER_ADDR_LEN)){
			not_dslite=0;
			break;
		}
	}
	if(i==rg_db.systemGlobal.wanIntfTotalNum)goto error_out2;//unmatch

	//20151007LUKE: for ICMPv6, we should keep original format and trap to protocol stack.
	//20151005LUKE: if igmpSnoop is disable, treat it as normal multicast packet!
	if(pPktHdr->tagif&ICMPV6_TAGIF){
		TRACE("pppoe ICMPv6 packet...Trap!");
		return RG_FWDENGINE_RET_TO_PS;
	}else if(!rg_db.systemGlobal.initParam.igmpSnoopingEnable){
		TRACE("IGMP/MLD snooping disabled...Transformed and Broadcast!");
		goto TRANSFORM;
	}

	TRACE("pppoe multicast routing.");
	
	//multicast test
#if defined(CONFIG_RTL9602C_SERIES)
	if(pPktHdr->tagif&IPV6_TAGIF && not_dslite){
		rtk_mac_t mac;
		//change DA to multicast MAC from IPv6 address low order 32 bits
		mac.octet[0]=0x33;
		mac.octet[1]=0x33;
		mac.octet[2]=pPktHdr->pIpv6Dip[12];
		mac.octet[3]=pPktHdr->pIpv6Dip[14];
		mac.octet[4]=pPktHdr->pIpv6Dip[13];
		mac.octet[5]=pPktHdr->pIpv6Dip[15];
		/* PATH2 */
		if(rg_db.vlan[pPktHdr->internalVlanID].fidMode==VLAN_FID_IVL){
			ret=_rtk_rg_findAndReclamL2mcEntryIVL(&mac, pPktHdr->internalVlanID);
			mc_type=RTK_LUT_L2MC;
		}else{
#if RTK_RG_MULTICAST_MODE_MACFID 			
			ret=_rtk_rg_findAndReclamL2mcEntry(&mac, rg_db.vlan[pPktHdr->internalVlanID].fid);
			mc_type=RTK_LUT_L2MC;
#else
			rtk_ipv6_addr_t gipv6;
			/* PATH4 */
			memcpy(gipv6.ipv6_addr,pPktHdr->pIpv6Dip,IPV6_ADDR_LEN);
			ret=_rtk_rg_findAndReclamIPv6mcEntry(&gipv6, 0);
			mc_type=RTK_LUT_L3V6MC;
#endif
		}
	}else{
		rtk_mac_t mac;
		//change DA to multicast MAC from IP address low order 23 bits
		mac.octet[0]=0x01;
		mac.octet[1]=0x00;
		mac.octet[2]=0x5e;
		mac.octet[3]=(0x007f0000&(pPktHdr->ipv4Dip))>>16;
		mac.octet[4]=(0xff00&(pPktHdr->ipv4Dip))>>8;
		mac.octet[5]=0xff&(pPktHdr->ipv4Dip);

		if(rg_db.vlan[pPktHdr->internalVlanID].fidMode==VLAN_FID_IVL){
			ret=_rtk_rg_findAndReclamL2mcEntryIVL(&mac, pPktHdr->internalVlanID);
			mc_type=RTK_LUT_L2MC;
		}else{
#if RTK_RG_MULTICAST_MODE_MACFID
			ret=_rtk_rg_findAndReclamL2mcEntry(&mac, rg_db.vlan[pPktHdr->internalVlanID].fid);
			mc_type=RTK_LUT_L2MC;
#else
			ret=_rtk_rg_findAndReclamIpmcEntry(0, pPktHdr->ipv4Dip,0,rg_db.vlan[pPktHdr->internalVlanID].fid,0,0);	//don't care src IP
			mc_type=RTK_LUT_L3MC;
#endif
		}
	}
#elif defined(CONFIG_RTL9600_SERIES)
	if(pPktHdr->tagif&IPV6_TAGIF && not_dslite) {
		rtk_mac_t mac;
		//change DA to multicast MAC from IPv6 address low order 32 bits
		mac.octet[0]=0x33;
		mac.octet[1]=0x33;
		mac.octet[2]=pPktHdr->pIpv6Dip[12];
		mac.octet[3]=pPktHdr->pIpv6Dip[14];
		mac.octet[4]=pPktHdr->pIpv6Dip[13];
		mac.octet[5]=pPktHdr->pIpv6Dip[15];
		if(rg_db.vlan[pPktHdr->internalVlanID].fidMode==VLAN_FID_IVL)
			ret=_rtk_rg_findAndReclamL2mcEntryIVL(&mac, pPktHdr->internalVlanID);
		else
			ret=_rtk_rg_findAndReclamL2mcEntry(&mac, rg_db.vlan[pPktHdr->internalVlanID].fid);
		mc_type=RTK_LUT_L2MC;
	}else{
		rtk_mac_t mac;
		//change DA to multicast MAC from IP address low order 23 bits
		mac.octet[0]=0x01;
		mac.octet[1]=0x00;
		mac.octet[2]=0x5e;
		mac.octet[3]=(0x007f0000&(pPktHdr->ipv4Dip))>>16;
		mac.octet[4]=(0xff00&(pPktHdr->ipv4Dip))>>8;
		mac.octet[5]=0xff&(pPktHdr->ipv4Dip);
		if(rg_db.vlan[pPktHdr->internalVlanID].fidMode==VLAN_FID_IVL){
			ret=_rtk_rg_findAndReclamL2mcEntryIVL(&mac, pPktHdr->internalVlanID);
			mc_type=RTK_LUT_L2MC;
		}else{
#if RTK_RG_MULTICAST_MODE_MACFID			
			ret=_rtk_rg_findAndReclamL2mcEntry(&mac, rg_db.vlan[pPktHdr->internalVlanID].fid);
			mc_type=RTK_LUT_L2MC;
#else
			ret=_rtk_rg_findAndReclamIpmcEntry(0, pPktHdr->ipv4Dip,0,rg_db.vlan[pPktHdr->internalVlanID].fid,0,0);	//don't care src IP
			mc_type=RTK_LUT_L3MC;
#endif
		}
	}	
#endif

	if(ret>=0 && rg_db.lut[ret].valid && rg_db.lut[ret].rtk_lut.entryType==mc_type){
		switch(mc_type){
			case RTK_LUT_L2MC:
				mcPmsk=rg_db.lut[ret].rtk_lut.entry.l2McEntry.portmask.bits;
				mcExtPmsk=rg_db.lut[ret].rtk_lut.entry.l2McEntry.ext_portmask.bits;
				break;
			case RTK_LUT_L3MC:
			case RTK_LUT_L3V6MC:
				mcPmsk=rg_db.lut[ret].rtk_lut.entry.ipmcEntry.portmask.bits;
				mcExtPmsk=rg_db.lut[ret].rtk_lut.entry.ipmcEntry.ext_portmask.bits;
				break;
			default:
				goto error_out;
		}

		//FIXME: if the port belong to two different LAN interface, this mechanism will fail.
		//change SA from LAN interface
		for(j=0;j<rg_db.systemGlobal.lanIntfTotalNum;j++){
			_rtk_rg_portmask_translator(rg_db.systemGlobal.lanIntfGroup[j].p_intfInfo->p_lanIntfConf->port_mask, &out_mac_pmask, &out_ext_pmask);
			if(((out_mac_pmask.bits[0]&(*mcPmsk))==(*mcPmsk))&&
				((out_ext_pmask.bits[0]&(*mcExtPmsk))==(*mcExtPmsk))){
				memcpy(pPktHdr->pSmac,rg_db.systemGlobal.lanIntfGroup[j].p_intfInfo->p_lanIntfConf->gmac.octet,ETHER_ADDR_LEN);
				break;
			}
		}
		if(j==rg_db.systemGlobal.lanIntfTotalNum)goto error_out;//unmatch

		//found!
		pPktHdr->multicastMacPortMask.portmask=*mcPmsk;
		pPktHdr->multicastExtPortMask.portmask=*mcExtPmsk;
		pPktHdr->fwdDecision=RG_FWD_DECISION_PPPOE_MC;
		TRACE("the group IP is %x, member is %x, extmember is %x",rg_db.lut[ret].rtk_lut.entry.ipmcEntry.dip,pPktHdr->multicastMacPortMask.portmask,pPktHdr->multicastExtPortMask.portmask);

TRANSFORM:
		if(pPktHdr->tagif&IPV6_TAGIF && not_dslite){	//ipv6 pppoe Mulitcast
			//change DA to multicast MAC from IPv6 address low order 32 bits
			pPktHdr->pDmac[0]=0x33;
			pPktHdr->pDmac[1]=0x33;
			pPktHdr->pDmac[2]=pPktHdr->pIpv6Dip[12];
			pPktHdr->pDmac[3]=pPktHdr->pIpv6Dip[14];
			pPktHdr->pDmac[4]=pPktHdr->pIpv6Dip[13];
			pPktHdr->pDmac[5]=pPktHdr->pIpv6Dip[15];
			if(*pPktHdr->pIPv6HopLimit<=1)
				goto error_out;
			else{
				(*pPktHdr->pIPv6HopLimit)--;
			}
		}else{
			//change DA to multicast MAC from IP address low order 23 bits
			pPktHdr->pDmac[0]=0x01;
			pPktHdr->pDmac[1]=0x00;
			pPktHdr->pDmac[2]=0x5e;
			pPktHdr->pDmac[3]=(0x007f0000&(pPktHdr->ipv4Dip))>>16;
			pPktHdr->pDmac[4]=(0xff00&(pPktHdr->ipv4Dip))>>8;
			pPktHdr->pDmac[5]=0xff&(pPktHdr->ipv4Dip);
			if(*pPktHdr->pIpv4TTL<=1)
				goto error_out;
			else{
				(*pPktHdr->pIpv4TTL)--;
			}
		}
		
		if(not_dslite)
			_rtk_rg_removePPPoETag(pPktHdr);        //remove PPPoE tag
		else
			_rtk_rg_removeTunnelTag(pPktHdr->skb, pPktHdr); //remove PPPoE tag and dslite tag

#if 0  //modify etherType, already modify in function _rtk_rg_removePPPoETag
		if((pPktHdr->pppProtocal&0x00ff)==0x0021)
			pPktHdr->etherType=0x0800;
		else 
			pPktHdr->etherType=0x86dd;
#endif
		return RG_FWDENGINE_RET_CONTINUE;
	}
error_out:
	TRACE("pppoe_multicast error..drop!");
	return RG_FWDENGINE_RET_DROP;
error_out2:	
	//20150605LUKE: for pppoe bridge multicast should continue as unicast!
	TRACE("pppoe bridge multicast..continue as unicast!");
	return RG_FWDENGINE_RET_CONTINUE;	
}

rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngineSlowPath(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr)
{
	int aclRet=RG_FWDENGINE_RET_TO_PS,bcRet;
	int wanGroupIdx;
	rtk_rg_fwdEngineReturn_t ret;
	rtk_l34_bindAct_t bindAction=L34_BIND_ACT_END;
	rtk_mac_t zeroMac={{0}};
	
	//Clear old pkthdr information
	bzero(&pPktHdr->COUNT_LENGTH_FIELD,sizeof(rtk_rg_pktHdr_t)-((uint32)(&pPktHdr->COUNT_LENGTH_FIELD)-(uint32)pPktHdr));
	_rtk_rg_packetEgressValueInit(pPktHdr);

	//20151019LUKE: drop DMAC is all zero packet!
	if(!memcmp(pPktHdr->pDmac,zeroMac.octet,ETHER_ADDR_LEN))return RG_FWDENGINE_RET_DROP;

	//20150729LUKE: Handle IPv6 unspecified address, loopback address, and source mulitcast address
	if(pPktHdr->tagif&IPV6_TAGIF)
	{
		//drop source address as multicast address
		if(pPktHdr->pIpv6Sip[0]==0xff)return RG_FWDENGINE_RET_DROP;
		//drop source address as loopback address
		if((*(unsigned int *)(pPktHdr->pIpv6Sip)==0)&&
			(*(((unsigned int *)pPktHdr->pIpv6Sip)+1)==0)&&
			(*(((unsigned int *)pPktHdr->pIpv6Sip)+2)==0)&&
			(*(((unsigned int *)pPktHdr->pIpv6Sip)+3)==1))return RG_FWDENGINE_RET_DROP;
		//drop destination address as loopback or unspecified address
		if((*(unsigned int *)(pPktHdr->pIpv6Dip)==0)&&
			(*(((unsigned int *)pPktHdr->pIpv6Dip)+1)==0)&&
			(*(((unsigned int *)pPktHdr->pIpv6Dip)+2)==0)&&
			((*(((unsigned int *)pPktHdr->pIpv6Dip)+3)==0)||(*(((unsigned int *)pPktHdr->pIpv6Dip)+3)==1)))return RG_FWDENGINE_RET_DROP;
	}
#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) && defined(CONFIG_XDSL_ROMEDRIVER)
#else
	/*DHCP Wan packet*/
	for(ret=0;ret<rg_db.systemGlobal.wanIntfTotalNum;ret++){
		if(rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[rg_db.systemGlobal.wanIntfGroup[ret].index]!=FAIL){
			if(pPktHdr->pRxDesc->opts3.bit.reason==rg_db.systemGlobal.hwAclIdx_for_trap_dhcp[rg_db.systemGlobal.wanIntfGroup[ret].index]+TRAP_RSN_START/*ACL reason base*/){
				TRACE("Trap DHCP packet to PS by DHCP Wan[%d]!",ret);
				return RG_FWDENGINE_RET_TO_PS; //it should passthrought user defined ACL check!
			}
		}
	}
#endif

	//Trap IGMP or MLD to protocal stack.
	if(pPktHdr->tagif&ICMP_TAGIF && rg_db.systemGlobal.ctrlPathByProtocolStack_ICMP==1)
		return RG_FWDENGINE_RET_TO_PS;
	
	//if(rg_db.systemGlobal.hwnat_enable == RG_HWNAT_PROTOCOL_STACK)
			//return RG_FWDENGINE_RET_TO_PS;

	if(pPktHdr->ingressLocation==RG_IGR_PROTOCOL_STACK || pPktHdr->ingressLocation==RG_IGR_ARP_OR_ND)
	{
		TRACE("from PS or ARP/ND, into SLOW PATH");	
		goto PROTOCOL_SATCK_SLOW_PATH;
	}
	else if(pPktHdr->ingressLocation==RG_IGR_IGMP_OR_MLD)	//IGMP query and MLD query will do broadcast to LAN!
	{
		TRACE("IGMP/MLD query, broadcast to LAN");
		goto PROTOCOL_SATCK_BROADCAST;
	}

	/* WIFI DATA PATH Decision */
#ifdef CONFIG_RG_WLAN_HWNAT_ACCELERATION
//#if 0
	ret=_rtk_rg_wlanExtraDataPathDecision(pPktHdr,pPktHdr->pRxDesc,skb);
	if(ret==RG_FWDENGINE_RET_HWLOOKUP) goto hardware_lookup;
	else if(ret==RG_FWDENGINE_RET_CONTINUE) return ret;	
	else if(ret==RG_FWDENGINE_RET_TO_PS) return ret;	
	else if(ret==RG_FWDENGINE_RET_DROP) return ret;

#endif

PROTOCOL_SATCK_SLOW_PATH:
	TRACE("into SLOW PATH");
	if(rg_db.systemGlobal.fwdStatistic)
	{
		rg_db.systemGlobal.statistic.perPortCnt_slowPath[pPktHdr->ingressPort]++;			
	}

#if defined(CONFIG_XDSL_NEW_HWNAT_DRIVER) && defined(CONFIG_RTL8685)
/*  for 8685 sync lut table from hardware to software	*/
		assert_ok(_rtk_syncLutTb4way_FormHw2Sw(pPktHdr->pDmac));
		assert_ok(_rtk_syncLutTb4way_FormHw2Sw(pPktHdr->pSmac));
#endif

	//VLAN ingress filter
	if(pPktHdr->ingressPort>=RTK_RG_PORT_CPU)
	{
		if(_rtk_rg_fwdEngineVLANFiltering(pPktHdr->internalVlanID,RTK_RG_PORT_CPU,pPktHdr->ingressPort-RTK_RG_PORT_CPU)==RG_FWDENGINE_RET_DROP)
		{
			TRACE("Drop by VLAN ingress filter");
			return RG_FWDENGINE_RET_DROP;
		}
	}
	else
	{
		if(_rtk_rg_fwdEngineVLANFiltering(pPktHdr->internalVlanID,pPktHdr->ingressPort,pPktHdr->ingressPort)==RG_FWDENGINE_RET_DROP)
		{
			TRACE("Drop by VLAN ingress filter");
			return RG_FWDENGINE_RET_DROP;
		}
	}
	
	/*clear acl igr/egr hit info & action*/
	//bzero(&(pPktHdr->aclDecision),sizeof(rtk_rg_aclHitAndAction_t));

	/*Ingress ACL check*/
	assert_ok(_rtk_rg_ingressACLPatternCheck(pPktHdr,NULL));

	ret = _rtk_rg_ingressACLAction(pPktHdr);
	if(ret!=RG_FWDENGINE_RET_CONTINUE)
		return ret;

	ret = _rtk_rg_fwdEngineMacFilter(pPktHdr);
	if(ret!=RG_FWDENGINE_RET_CONTINUE)
		return ret;

	//20150422LUKE: check for PPPoE multicast packet
	if((!(pPktHdr->tagif&IGMP_TAGIF))&&(!(pPktHdr->tagif&IPV6_MLD_TAGIF))&&(pPktHdr->tagif&PPPOE_TAGIF)&&
		(((pPktHdr->tagif&IPV4_TAGIF)&&((pPktHdr->ipv4Dip&0xf0000000)==0xe0000000)) ||
		((pPktHdr->tagif&IPV6_TAGIF)&&((pPktHdr->pIpv6Dip[0])==0xff))))
	{
		ret=_rtk_rg_pppoe_multicast_process(pPktHdr);
		if(ret!=RG_FWDENGINE_RET_CONTINUE)
			return ret;
	}

	/* Check the DMAC */
	if(pPktHdr->ingressLocation!=RG_IGR_PROTOCOL_STACK)
	{
		_rtk_rg_fwdEngineLIMDBC(pPktHdr);
	}

	if(!pPktHdr->isGatewayPacket)
	{
		/*get internal Priority: for layer2 pkt decision*/
		if(_rtk_rg_internalPrioritySelect(pPktHdr,skb)!=RG_RET_SUCCESS)
		{
			TRACE("Drop!");
			return RG_FWDENGINE_RET_DROP;
		}
	}

//============================= L2 forward ============================
	ret=_rtk_rg_layer2Forward(skb,pPktHdr);
	TRACE("layer2Forward result: 0x%x",ret);


	if(ret==RG_FWDENGINE_RET_L2FORWARDED)
	{
		pPktHdr->fwdDecision=RG_FWD_DECISION_BRIDGING;

		if(rg_db.systemGlobal.fwdStatistic)
		{
			rg_db.systemGlobal.statistic.perPortCnt_L2FWD[pPktHdr->ingressPort]++;			
		}
		
		
		//Patch20131014:from protocol stack's untag packet will use CPU's PVID which is LAN's VLAN, 
		//if destination port is WAN port the packets will be filtered!!So we do directTx here~
		if(pPktHdr->ingressLocation==RG_IGR_PROTOCOL_STACK)
		{
			ret=_rtk_rg_checkFromProtocolStackInterface(pPktHdr);
			switch(ret)
			{
				//case RG_FWDENGINE_RET_DROP:
					//return ret;
				case RG_FWDENGINE_RET_HWLOOKUP:		//wifi
					goto hardware_lookup;
				default:	//RG_FWDENGINE_RET_DIRECT_TX
					break;
			}
		}
		else if(rg_db.systemGlobal.initParam.macBasedTagDecision && (_rtk_rg_bindingRuleCheck(pPktHdr, &wanGroupIdx)==RG_FWDENGINE_RET_HIT_BINDING))
		{
			//20140606LUKE:From protocol stack will skip binding check~
			//20140605LUKE:if hit Layer2 binding, save binding wan interface in pkthdr->netifIdx!!
			//pPktHdr->netifIdx=rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].index;
			//20140711LUKE:store binding decision in pkthdr, and apply it only when destination is located in rg_db.systemGlobal.wanPortMask.portmask
			pPktHdr->layer2BindNetifIdx=rg_db.systemGlobal.wanIntfGroup[wanGroupIdx].index;
			//20140808LUKE: use bind_wan_type_ipv4 for Layer2 binding check!!

			/*20141015CHUCK: PPPoE dropped by protocal patch*/
			if(rg_db.systemGlobal.port_binding_by_protocal==1){//IPv4 Routing, IPv6 Bridge
				TRACE("IPv4 Routing, IPv6 Bridge");
				if(pPktHdr->pppProtocal==0x8021 || pPktHdr->pppProtocal==0x0021){
					TRACE("Drop! IPCP dropped by Binding WAN[%d]",pPktHdr->layer2BindNetifIdx);
					return RG_FWDENGINE_RET_DROP;
				}
			}else if(rg_db.systemGlobal.port_binding_by_protocal==2){//IPv6 Routing, IPv4 Bridge
				TRACE("IPv6 Routing, IPv4 Bridge");
				//drop IP6CP
				if(pPktHdr->pppProtocal==0x8057 || pPktHdr->pppProtocal==0x0057){
					TRACE("Drop! IP6CP dropped by Binding WAN[%d]",pPktHdr->layer2BindNetifIdx);
					return RG_FWDENGINE_RET_DROP;
				}
			}else{// IPv4+IPv6 both Bridge
				//TRACE("IPv4+IPv6 both Bridge");
			}
	
			switch(rg_db.wantype[rg_db.systemGlobal.interfaceInfo[pPktHdr->layer2BindNetifIdx].storedInfo.wan_intf.bind_wan_type_ipv4].rtk_wantype.wanType)
			{
				case L34_WAN_TYPE_L2_BRIDGE:
					TRACE("L2 binding lookup success! netif is %d",pPktHdr->layer2BindNetifIdx);
					break;
				case L34_WAN_TYPE_L3_ROUTE:	//unmatch: follow hw register setting
					TRACE("Unmatch for L2 binding to L3 WAN...");
					bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_UNMATCHED_L2L3];
					break;
				case L34_WAN_TYPE_L34NAT_ROUTE:		//unmatch: follow hw register setting
					TRACE("Unmatch for L2 binding to L34 WAN...");
					bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_UNMATCHED_L2L34];
					break;
				case L34_WAN_TYPE_L34_CUSTOMIZED:	//unmatch: follow hw register setting
					TRACE("Unmatch for L34 binding to customized WAN...");
					bindAction=rg_db.systemGlobal.l34BindAction[L34_BIND_CUSTOMIZED_L2];
					break;
				default:
					break;
			}

			ret=_rtk_rg_unmatchBindingAct(pPktHdr,bindAction,NULL);
			if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;
		}
		
		pPktHdr->naptIdx=0;	
		pPktHdr->l3Modify=0;
		pPktHdr->l4Modify=0;

		//20150116LUKE: update shortcut for TCP or UDP traffic
		if((pPktHdr->tagif&IPV4_TAGIF)&&((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF)))
		{
#ifdef CONFIG_ROME_NAPT_SHORTCUT
			_rtk_rg_naptShortcutUpdate(pPktHdr, 0, 0, 0, 1);
#endif
		}
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
		else if((pPktHdr->tagif&IPV6_TAGIF)&&((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF)))
		{
			_rtk_rg_naptV6ShortcutUpdate(pPktHdr,1);
		}
#endif
		
/*
		//Do ACL egress check
		assert_ok(_rtk_rg_egressACLPatternCheck(2,0,pPktHdr,skb,0,0));	//direct=2 means bridge mode		
		aclRet = _rtk_rg_egressACLAction(pPktHdr);
		if(aclRet == RG_FWDENGINE_RET_TO_PS || aclRet == RG_FWDENGINE_RET_DROP)
			return aclRet;
*/

		//goto hardware_lookup;//Let hardware do lookup works (destination port, VLAN, DMAC2CVID)
		//PATCH20131213:since we can't figure out if packet is unknown DA because trap reason may covered by ACL or something else,
		//we will not use hardware lookup in layer2 forward anymore!
		//goto direct_tx;	//Because CPU learning limit action set to trap now(for dual wifi), layer2 forward should use directTX, otherwise this packet will always fail.
		return RG_FWDENGINE_RET_DIRECT_TX_SLOW;
	}
	else if(ret==RG_FWDENGINE_RET_BROADCAST)
	{
PROTOCOL_SATCK_BROADCAST:
		
		if(pPktHdr->fwdDecision!=RG_FWD_DECISION_PPPOE_MC)pPktHdr->fwdDecision=RG_FWD_DECISION_NORMAL_BC;
		//DEBUG("broadcast or flooding unknown DA packet");
		//DEBUG("before broadcast forward!! internal vlan is %d",pPktHdr->internalVlanID);
		if(rg_db.systemGlobal.ctrlPathByProtocolStack_broadcast==1){
			//TRACE("ctrlPathByProtocolStack_broadcast==1 ret to PS");
			return RG_FWDENGINE_RET_TO_PS;
		}

		//20140508LUKE:from protocol stack we should check which interface send this out!!
		if(pPktHdr->ingressLocation==RG_IGR_PROTOCOL_STACK && (rg_kernel.protocolStackTxPortMask&rg_db.systemGlobal.wanPortMask.portmask))
		{
			ret=_rtk_rg_checkFromProtocolStackInterface(pPktHdr);
			if(ret==RG_FWDENGINE_RET_HWLOOKUP)
				goto hardware_lookup;
		}
		
		//DirectTX or To_ProtocolStack or DROP
		bcRet = _rtk_rg_broadcastForwardWithPkthdr(pPktHdr,skb,pPktHdr->internalVlanID,pPktHdr->pRxDesc->opts3.bit.src_port_num,pPktHdr->ingressPort);
		//DEBUG("bcRet=0x%x",bcRet);
		return bcRet;

	}
	else if(ret==RG_FWDENGINE_RET_CONTINUE)
	{
		//continue do L34 forward
	}else{
		//ret==RG_FWDENGINE_RET_TO_PS || RG_FWDENGINE_RET_DROP
		return ret;	
	}

	//DA == Gateway MAC, do Layer34 forward
//============================= L34 forward ============================

//	if(pPktHdr->tagif&IPV4_TAGIF)
//		rtlglue_printf("sip=%x sport=%d\n",pPktHdr->ipv4Sip,pPktHdr->sport);

//	memset(&rxinfo,0,sizeof(rtk_rg_rxdesc_t));
//	data = RG_GLB_FWDENGINE_initParam.nicRxCallBack(net_buf,&rxinfo);


	ret=_rtk_rg_layer34Forward(skb,pPktHdr);

	/*get internal Priority: for layer34 pkt decision*/
	if(_rtk_rg_internalPrioritySelect(pPktHdr,skb)!=RG_RET_SUCCESS)
	{
		TRACE("Drop!");
		return RG_FWDENGINE_RET_DROP;
	}

	switch(ret)
	{
		case RG_FWDENGINE_RET_QUEUE_FRAG:
			TRACE("Queue Framgment packet.");
			return ret;
		case RG_FWDENGINE_RET_DROP:
			TRACE("Drop by L34Fwd");
			return ret;
		case RG_FWDENGINE_RET_TO_PS:
			TRACE("Trap by L34Fwd");
			return ret;
		case RG_FWDENGINE_RET_DIRECT_TX:
			//goto direct_tx;
			return RG_FWDENGINE_RET_DIRECT_TX_SLOW;
		case RG_FWDENGINE_RET_FRAG_ONE_PS:		//Hit ACL action TRAP
			TRACE("Fragment First Packet TO_PS!");
			aclRet=RG_FWDENGINE_RET_TO_PS;
			goto fragment_send;
		case RG_FWDENGINE_RET_FRAG_ONE_DROP:	//Hit ACL action DROP
			TRACE("Fragment First Packet DROP!");
			aclRet=RG_FWDENGINE_RET_DROP;
			goto fragment_send;
		case RG_FWDENGINE_RET_FRAGMENT_ONE:
			TRACE("Fragment First Packet send!");
			aclRet=RG_FWDENGINE_RET_DIRECT_TX;
		case RG_FWDENGINE_RET_FRAGMENT:			
			goto fragment_send;
#if 0			
		case FAIL: //not l34
			TRACE("L34 Fail..return to PS!");
			//if(rg_db.systemGlobal.hwnat_enable == ENABLE)
			//{
				//DEBUG("layer34Forward fail...return to PS");
				return RG_FWDENGINE_RET_TO_PS;
#endif			
			/*}
			else
			{	
				//FIXME: to WLAN packet don't use Direct_TX (it will send to CPU again, and reason is srcPortFilter)
				for(i=0; i<rg_db.systemGlobal.lanIntfTotalNum; i++)
				{
					if(rg_db.systemGlobal.lanIntfGroup[i].p_intfInfo->p_lanIntfConf->ip_addr == pPktHdr->ipv4Dip)
					{
						return RG_FWDENGINE_RET_TO_PS;
					}
				}
				
				for(i=0; i<rg_db.systemGlobal.wanIntfTotalNum;i++)
				{
					//Bridge WAN won't be compared with
					if(rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo==NULL)
						continue;
					
				   	if(rg_db.systemGlobal.wanIntfGroup[i].p_intfInfo->p_wanStaticInfo->ip_addr==pPktHdr->ipv4Dip)
				   	{
						return RG_FWDENGINE_RET_TO_PS;
				   	}
				}
				//DEBUG("before direct tx");
				goto direct_tx;
			}*/
		default:
			break;
	}	
	return RG_FWDENGINE_RET_TO_PS;

hardware_lookup:
	_rtk_rg_fwdEngineHwLookup(skb,pPktHdr);

	if(pPktHdr->tagif&IPV4_TAGIF)
	{
		TRACE("Egress Src=%d.%d.%d.%d(%d) Dst=%d.%d.%d.%d(%d) %s %s%s%s%s%s%s"
			,(*pPktHdr->pIpv4Sip>>24)&0xff,(*pPktHdr->pIpv4Sip>>16)&0xff,(*pPktHdr->pIpv4Sip>>8)&0xff,(*pPktHdr->pIpv4Sip)&0xff,pPktHdr->tagif&(TCP_TAGIF|UDP_TAGIF)?*pPktHdr->pSport:0
			,(*pPktHdr->pIpv4Dip>>24)&0xff,(*pPktHdr->pIpv4Dip>>16)&0xff,(*pPktHdr->pIpv4Dip>>8)&0xff,(*pPktHdr->pIpv4Dip)&0xff,pPktHdr->tagif&(TCP_TAGIF|UDP_TAGIF)?*pPktHdr->pDport:0
			,(pPktHdr->tagif&TCP_TAGIF)?"TCP":((pPktHdr->tagif&UDP_TAGIF)?"UDP":"OTHER")
			,(pPktHdr->tagif&TCP_TAGIF)?"Flags:":""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.syn?"SYN ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.ack?"ACK ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.push?"PSH ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.fin?"FIN ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.reset?"RST ":""):""
			);
	}

	TRACE("%s_TX_O[%lx]: HWLOOKUP to portmask=0x%x extspa=%d vlanAct=%d vid=%d pri=%d l34Keep=%d keep=%d",pPktHdr->ingressLocation==RG_IGR_PHY_PORT?"FWD":"PS",((unsigned long)skb)&0xffff,
		rg_kernel.txDesc.opts3.bit.tx_portmask,
		rg_kernel.txDesc.opts3.bit.extspa,
		rg_kernel.txDesc.opts2.bit.tx_vlan_action,
		(rg_kernel.txDesc.opts2.bit.vidh<<8)|rg_kernel.txDesc.opts2.bit.vidl,
		rg_kernel.txDesc.opts2.bit.prio,
		rg_kernel.txDesc.opts3.bit.l34_keep,
		rg_kernel.txDesc.opts1.bit.keep);		
	
	return RG_FWDENGINE_RET_HWLOOKUP;
	
fragment_send:
	if(ret==RG_FWDENGINE_RET_FRAGMENT || ret==RG_FWDENGINE_RET_FRAGMENT_ONE)
	{
#ifdef __KERNEL__
		_rtk_rg_fwdEngineDirectTx(skb,pPktHdr);
		TRACE("FWD_TX_O[%x]: DIRECT TX to portmask=0x%x extspa=%d vlanAct=%d vid=%d pri=%d",((u32)skb)&0xffff,
			rg_kernel.txDesc.opts3.bit.tx_portmask,
			rg_kernel.txDesc.opts3.bit.extspa,
			rg_kernel.txDesc.opts2.bit.tx_vlan_action,
			(rg_kernel.txDesc.opts2.bit.vidh<<8)|rg_kernel.txDesc.opts2.bit.vidl,
			rg_kernel.txDesc.opts2.bit.prio);

	//_rtk_rg_splitJumboSendToNicWithTxInfoAndMask(pPktHdr,skb,&rg_kernel.txDesc,0,&rg_kernel.txDescMask);
#endif	

		if(ret==RG_FWDENGINE_RET_FRAGMENT)		//just stop here
			return RG_FWDENGINE_RET_DIRECT_TX;
	}

	//Fragment_one, one_ps, one_drop will reach here
	_rtk_rg_fwdEngine_updateFlowStatus(skb, pPktHdr);

	if(pPktHdr->tagif&IPV6_TAGIF)
	{
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
		//send or drop or trap these queued packet which idendification is same to pktHdr
		_rtk_rg_fwdEngine_v6FragmentQueueProcessing(aclRet,pPktHdr,pPktHdr->pIPv6FragmentList);
#endif
	}
	else
	{
		//send or drop or trap these queued packet which idendification is same to pktHdr
		_rtk_rg_fwdEngine_fragmentQueueProcessing(aclRet,pPktHdr);
	}	
	
	return aclRet;
}

void _rtk_rg_fwdEngine_displayInfo(rtk_rg_pktHdr_t *pPktHdr)
{
	TRACE(
#ifdef CONFIG_APOLLO_MODEL	
	"==================== PACKET BOUNDARY ====================\n        FWD_RX_I[%lx]: DA=%02x:%02x:%02x:%02x:%02x:%02x SA=%02x:%02x:%02x:%02x:%02x:%02x ETH=%04x len=%d (EXT)SPA=%d\n        EXTMASK=0x%x%s Reason=%d IntPri=%d stagIf=%d(VID=%d, PRI=%d, DEI=%d) ctagIf=%d(VID=%d, PRI=%d, CFI=%d)",
		(unsigned long)pPktHdr->skb&0xffff,
#else
	"==================== PACKET BOUNDARY ====================\n        FWD_RX_I[%x]: DA=%02x:%02x:%02x:%02x:%02x:%02x SA=%02x:%02x:%02x:%02x:%02x:%02x ETH=%04x len=%d (EXT)SPA=%d\n        EXTMASK=0x%x%s Reason=%d IntPri=%d stagIf=%d(VID=%d, PRI=%d, DEI=%d, TPID=0x%x) ctagIf=%d(VID=%d, PRI=%d, CFI=%d)",
		(unsigned int)pPktHdr->skb&0xffff,
#endif
		pPktHdr->skb->data[0],pPktHdr->skb->data[1],pPktHdr->skb->data[2],pPktHdr->skb->data[3],pPktHdr->skb->data[4],pPktHdr->skb->data[5],
		pPktHdr->skb->data[6],pPktHdr->skb->data[7],pPktHdr->skb->data[8],pPktHdr->skb->data[9],pPktHdr->skb->data[10],pPktHdr->skb->data[11],
		pPktHdr->etherType,
		pPktHdr->skb->len,
		pPktHdr->ingressPort,
		pPktHdr->pRxDesc->opts3.bit.dst_port_mask,
		(pPktHdr->pRxDesc->opts3.bit.dst_port_mask==0x20 && pPktHdr->pRxDesc->opts3.bit.rsvd==0x2)?"(IGMP/MLD) ":((pPktHdr->pRxDesc->opts3.bit.dst_port_mask==0x20 && pPktHdr->pRxDesc->opts3.bit.rsvd==0x1)?"(ARP/ND) ":((pPktHdr->pRxDesc->opts3.bit.dst_port_mask==0x20 && pPktHdr->pRxDesc->opts3.bit.rsvd==0x0)?"(fromPS) ":((pPktHdr->pRxDesc->opts3.bit.dst_port_mask==0x10)?"(fromEXT1) ":((pPktHdr->pRxDesc->opts3.bit.dst_port_mask==0x8)?"(fromEXT0) ":((pPktHdr->pRxDesc->opts3.bit.dst_port_mask==0x4)?"(toEXT1) ":((pPktHdr->pRxDesc->opts3.bit.dst_port_mask==0x2)?"(toEXT0) ":"(toCPU) ")))))),

		pPktHdr->pRxDesc->opts3.bit.reason,		
		pPktHdr->pRxDesc->opts3.bit.internal_priority,
		(pPktHdr->tagif&SVLAN_TAGIF)>0,
		(pPktHdr->tagif&SVLAN_TAGIF)>0?pPktHdr->stagVid:0,
		(pPktHdr->tagif&SVLAN_TAGIF)>0?pPktHdr->stagPri:0,
		(pPktHdr->tagif&SVLAN_TAGIF)>0?pPktHdr->stagDei:0,
		(pPktHdr->tagif&SVLAN_TAGIF)>0?pPktHdr->stagTpid:0,
		(pPktHdr->tagif&CVLAN_TAGIF)>0,
		(pPktHdr->tagif&CVLAN_TAGIF)>0?pPktHdr->ctagVid:0,
		(pPktHdr->tagif&CVLAN_TAGIF)>0?pPktHdr->ctagPri:0,
		(pPktHdr->tagif&CVLAN_TAGIF)>0?pPktHdr->ctagCfi:0	);

	if(pPktHdr->tagif&IPV4_TAGIF)
	{
		TRACE("Ingress Src=%d.%d.%d.%d(%d) Dst=%d.%d.%d.%d(%d) %s L4_PROTO=0x%02x %s%s%s%s%s%s"
			,(pPktHdr->ipv4Sip>>24)&0xff,(pPktHdr->ipv4Sip>>16)&0xff,(pPktHdr->ipv4Sip>>8)&0xff,(pPktHdr->ipv4Sip)&0xff,((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF))?pPktHdr->sport:0
			,(pPktHdr->ipv4Dip>>24)&0xff,(pPktHdr->ipv4Dip>>16)&0xff,(pPktHdr->ipv4Dip>>8)&0xff,(pPktHdr->ipv4Dip)&0xff,((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF))?pPktHdr->dport:0
			,(pPktHdr->tagif&TCP_TAGIF)?"TCP":((pPktHdr->tagif&UDP_TAGIF)?"UDP":((pPktHdr->ipProtocol==1)?"ICMP":((pPktHdr->ipProtocol==2)?"IGMP":"OTHER")))
			,pPktHdr->ipProtocol
			,(pPktHdr->tagif&TCP_TAGIF)?"Flags:":""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.syn?"SYN ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.ack?"ACK ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.push?"PSH ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.fin?"FIN ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.reset?"RST ":""):""
			);

	}

	if(pPktHdr->tagif&IPV6_TAGIF){
		TRACE("Src=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x(%d)  Dst=%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x(%d)  %s L4_PROTO=0x%02x",
			pPktHdr->pIpv6Sip[0],pPktHdr->pIpv6Sip[1],pPktHdr->pIpv6Sip[2],pPktHdr->pIpv6Sip[3],
			pPktHdr->pIpv6Sip[4],pPktHdr->pIpv6Sip[5],pPktHdr->pIpv6Sip[6],pPktHdr->pIpv6Sip[7],
			pPktHdr->pIpv6Sip[8],pPktHdr->pIpv6Sip[9],pPktHdr->pIpv6Sip[10],pPktHdr->pIpv6Sip[11],
			pPktHdr->pIpv6Sip[12],pPktHdr->pIpv6Sip[13],pPktHdr->pIpv6Sip[14],pPktHdr->pIpv6Sip[15],((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF))?pPktHdr->sport:0,
			pPktHdr->pIpv6Dip[0],pPktHdr->pIpv6Dip[1],pPktHdr->pIpv6Dip[2],pPktHdr->pIpv6Dip[3],
			pPktHdr->pIpv6Dip[4],pPktHdr->pIpv6Dip[5],pPktHdr->pIpv6Dip[6],pPktHdr->pIpv6Dip[7],
			pPktHdr->pIpv6Dip[8],pPktHdr->pIpv6Dip[9],pPktHdr->pIpv6Dip[10],pPktHdr->pIpv6Dip[11],
			pPktHdr->pIpv6Dip[12],pPktHdr->pIpv6Dip[13],pPktHdr->pIpv6Dip[14],pPktHdr->pIpv6Dip[15],((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF))?pPktHdr->dport:0,
			(pPktHdr->tagif&TCP_TAGIF)?"TCP":((pPktHdr->tagif&UDP_TAGIF)?"UDP":((pPktHdr->tagif&ICMPV6_TAGIF)?"ICMPv6":((pPktHdr->ipProtocol==2)?"IGMP":"OTHER"))),pPktHdr->ipProtocol
		);
	}	
	
#ifdef CONFIG_APOLLO_MODEL	
#else
		
#if defined(CONFIG_RG_DEBUG)
	if(rg_kernel.debug_level&RTK_RG_DEBUG_LEVEL_TRACE_DUMP)
	{
		int trace=1;
		if(rg_kernel.filter_level& RTK_RG_DEBUG_LEVEL_TRACE_DUMP)
			trace=_rtk_rg_trace_filter_compare(pPktHdr->skb,pPktHdr);
		if(trace==1)
		{
			dump_packet(pPktHdr->skb->data,pPktHdr->skb->len,"trace_dump");
		}		
	}
#endif
#endif
}

void _rtk_rg_fwdEngine_updateFlowStatus(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr)
{
	//We add HW entry after NIC TX to enhance performance.
	//20151211LUKE: add to HW only when DPI return FINISH, otherwise the flow keep in slowpath.
	if(pPktHdr->addNaptAfterNicTx==1 && !rg_db.naptOut[pPktHdr->naptOutboundIndx].cannotAddToHw){
		assert_ok(_rtk_rg_naptConnection_add(pPktHdr->naptOutboundIndx,&rg_db.naptOut[pPktHdr->naptOutboundIndx].rtk_naptOut,&rg_db.naptIn[pPktHdr->naptrInboundIndx].rtk_naptIn));
		TRACE("add HW TCP/UDP naptOut[%d],naptIn[%d]",pPktHdr->naptOutboundIndx,pPktHdr->naptrInboundIndx); 	
	}

	//delete napt entry immediately. (for special test scenario)
	if(pPktHdr->delNaptConnection==1){
		(pf.rtk_rg_naptConnection_del)(pPktHdr->naptOutboundIndx);
		TRACE("del HW TCP/UDP naptOut[%d]",pPktHdr->naptOutboundIndx);		
	}
}

rtk_rg_fwdEngineReturn_t _rtk_rg_fwdEngine_afterProcess(struct sk_buff *skb,rtk_rg_pktHdr_t *pPktHdr, rtk_rg_fwdEngineReturn_t ret)
{	
	//20141226LUKE: if packet is droped because of port isolation, just return
	if(pPktHdr->fwdDecision==RG_FWD_DECISION_PORT_ISO)return ret;

	if((pPktHdr->tagif&IPV4_TAGIF)&&(pPktHdr->tagif&(TCP_TAGIF|UDP_TAGIF)))
	{
		TRACE("Egress Src=%d.%d.%d.%d(%d) Dst=%d.%d.%d.%d(%d) %s L4_PROTO=0x%02x %s%s%s%s%s%s"
			,(*pPktHdr->pIpv4Sip>>24)&0xff,(*pPktHdr->pIpv4Sip>>16)&0xff,(*pPktHdr->pIpv4Sip>>8)&0xff,(*pPktHdr->pIpv4Sip)&0xff,*pPktHdr->pSport
			,(*pPktHdr->pIpv4Dip>>24)&0xff,(*pPktHdr->pIpv4Dip>>16)&0xff,(*pPktHdr->pIpv4Dip>>8)&0xff,(*pPktHdr->pIpv4Dip)&0xff,*pPktHdr->pDport
			,(pPktHdr->tagif&TCP_TAGIF)?"[TCP]":"[UDP]"
			,pPktHdr->ipProtocol
			,(pPktHdr->tagif&TCP_TAGIF)?"Flags:":""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.syn?"SYN ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.ack?"ACK ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.push?"PSH ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.fin?"FIN ":""):""
			,(pPktHdr->tagif&TCP_TAGIF)?(pPktHdr->tcpFlags.reset?"RST ":""):""
			);
	}

	//20140122LUKE:if directTX decide to go to CPU port, we just return the packet to protocol stack.
	if(pPktHdr->fwdDecision==RG_FWD_DECISION_TO_PS)
		return RG_FWDENGINE_RET_TO_PS;
	
	if(ret==RG_FWDENGINE_RET_DIRECT_TX)
	{
#ifdef CONFIG_GPON_FEATURE 
		TRACE("%s_TX_O[%lx]: %s portmask=0x%x extspa=%d vlanAct=%d(%s) vid=%d pri=%d l34Keep=%d Keep=%d PON_SID=%d",pPktHdr->ingressLocation==RG_IGR_PHY_PORT?"FWD":"PS",((unsigned long)skb)&0xffff,
				pPktHdr->fwdDecision==RG_FWD_DECISION_NO_PS_BC?"BROADCAST to":rg_kernel.txDesc.opts3.bit.tx_portmask==0?"HW LOOKUP:":"DIRECT TX to",
				rg_kernel.txDesc.opts3.bit.tx_portmask,
				rg_kernel.txDesc.opts3.bit.extspa,
				rg_kernel.txDesc.opts2.bit.tx_vlan_action,
				(rg_kernel.txDesc.opts2.bit.tx_vlan_action==2)?"Remove":((rg_kernel.txDesc.opts2.bit.tx_vlan_action==3)?"Modify":((rg_kernel.txDesc.opts2.bit.tx_vlan_action==0)?"Intact":"Add")),
				(rg_kernel.txDesc.opts2.bit.vidh<<8)|rg_kernel.txDesc.opts2.bit.vidl,
				rg_kernel.txDesc.opts2.bit.prio,
				rg_kernel.txDesc.opts3.bit.l34_keep,
				rg_kernel.txDesc.opts1.bit.keep,
				rg_kernel.txDesc.opts3.bit.tx_dst_stream_id);
#else
		TRACE("%s_TX_O[%lx]: %s portmask=0x%x extspa=%d vlanAct=%d(%s) vid=%d pri=%d l34Keep=%d Keep=%d",pPktHdr->ingressLocation==RG_IGR_PHY_PORT?"FWD":"PS",((unsigned long)skb)&0xffff,
				pPktHdr->fwdDecision==RG_FWD_DECISION_NO_PS_BC?"BROADCAST to":rg_kernel.txDesc.opts3.bit.tx_portmask==0?"HW LOOKUP:":"DIRECT TX to",
				rg_kernel.txDesc.opts3.bit.tx_portmask,
				rg_kernel.txDesc.opts3.bit.extspa,
				rg_kernel.txDesc.opts2.bit.tx_vlan_action,
				(rg_kernel.txDesc.opts2.bit.tx_vlan_action==2)?"Remove":((rg_kernel.txDesc.opts2.bit.tx_vlan_action==3)?"Modify":((rg_kernel.txDesc.opts2.bit.tx_vlan_action==0)?"Intact":"Add")),
				(rg_kernel.txDesc.opts2.bit.vidh<<8)|rg_kernel.txDesc.opts2.bit.vidl,
				rg_kernel.txDesc.opts2.bit.prio,
				rg_kernel.txDesc.opts3.bit.l34_keep,
				rg_kernel.txDesc.opts1.bit.keep);
#endif

		return RG_FWDENGINE_RET_DIRECT_TX;
	}
	else if((ret==RG_FWDENGINE_RET_TO_PS)||(ret==RG_FWDENGINE_RET_DROP)||(ret==RG_FWDENGINE_RET_SEND_TO_WIFI))
	{
		return ret;
	}
	else
	{
		WARNING("ret=%d is not defined!",ret);
		return RG_FWDENGINE_RET_DIRECT_TX;
	}
}

#define RG_MIN_MRU	60

__IRAM_FWDENG
rtk_rg_fwdEngineReturn_t rtk_rg_fwdEngineInput(struct re_private *cp, struct sk_buff *skb, void *pRxDescPtr)
{
	int i,j,parserRet/*,naptOutIdx=FAIL,fragIdx=FAIL,totalQueueNum*/;
	rtk_rg_fwdEngineReturn_t ret;
	rtk_rg_rxdesc_t	*pRxDesc=(rtk_rg_rxdesc_t *)pRxDescPtr;
#ifdef CONFIG_ROME_NAPT_SHORTCUT
	rtk_rg_napt_shortcut_t *pNaptSc=NULL;
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	rtk_rg_naptv6_shortcut_t *pNaptV6Sc=NULL;
#endif
	
	//rtk_rg_route_shortcut_t *pRoutingSc;
	rtk_rg_pktHdr_t *pPktHdr=rg_db.pktHdr;
#if 0
	TRACE("debug opts1.bit.data_length   [%d]", pRxDesc->opts1.bit.data_length);
	TRACE("debug opts3.bit.src_port_num  [%d]", pRxDesc->opts3.bit.src_port_num);
	TRACE("debug opts3.bit.dst_port_mask [0x%x]", pRxDesc->opts3.bit.dst_port_mask);
	TRACE("debug opts3.bit.reason        [%d]", pRxDesc->opts3.bit.reason);
	TRACE("debug opts3.bit.internal_priority [%d]", pRxDesc->opts3.bit.internal_priority);
#endif

	//Clear old pkthdr information
	bzero(pPktHdr,((uint32)(&pPktHdr->COUNT_LENGTH_FIELD)-(uint32)pPktHdr));
	pRxDesc->opts2.bit.rsvd_2=1;

	pPktHdr->cp=cp;
	pPktHdr->skb=skb;
	pPktHdr->pRxDesc=pRxDesc;
	
	_rtk_rg_extIngressPortDecision(pPktHdr,pRxDesc);
	//*(unsigned long*)(skb->data+skb->len)=(unsigned long)pPktHdr; //save point of pktHdr into end of skb data.(for trace filter debug)
	
	if((rg_db.systemGlobal.BCRateLimitPortMask)||(rg_db.systemGlobal.IPv6MCRateLimitPortMask)||(rg_db.systemGlobal.IPv4MCRateLimitPortMask))
	{
		ret = _rtk_rg_BCMCRateLimit_check(skb,pPktHdr);
		if(ret == RG_FWDENGINE_RET_DROP)
		{
			TRACE("Drop!");
			return RG_FWDENGINE_RET_DROP;
		}
	}

#if defined(CONFIG_DUALBAND_CONCURRENT)||defined(CONFIG_MASTER_WLAN0_ENABLE)
	pPktHdr->wlan_dev_idx=FAIL;		//for slave wifi this should be FAIL
#endif
#ifdef CONFIG_MASTER_WLAN0_ENABLE
	//20140709LUKE:insert 1Q tag from WLAN0
	if(pPktHdr->ingressPort==RTK_RG_EXT_PORT0){
		ret = _rtk_rg_wlan_decision(skb,pPktHdr);
		if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;
	}
#endif
	
	parserRet=_rtk_rg_packetParser(skb,pPktHdr);
	if(parserRet!=RG_FWDENGINE_RET_CONTINUE)
		return parserRet;

	if(rg_db.systemGlobal.hwnat_enable==RG_HWNAT_UC_FORCE_HW_FWD && (pPktHdr->pDmac[0]&1)==0)
	{
		TRACE("rg_db.systemGlobal.hwnat_enable is RG_HWNAT_UC_FORCE_HW_FWD, drop unicast in fwdEngine!!!");
		return RG_FWDENGINE_RET_DROP;
	}
	

	//20150910LUKE: since IPv6's l3len is correct now, we can use it directly.
	//20141030LUKE: elimintate rx padding when IP total length smaller than 60 bytes
	if((pPktHdr->ingressLocation!=RG_IGR_PROTOCOL_STACK)&&((pPktHdr->tagif&IPV4_TAGIF)||(pPktHdr->tagif&IPV6_TAGIF))){
		if((pPktHdr->l3Offset+pPktHdr->l3Len)<RG_MIN_MRU){
			//TRACE("%s packet smaller than %dB, correct skb->len from %d to %d.",(pPktHdr->tagif&IPV4_TAGIF)?"IPv4":"IPv6",RG_MIN_MRU,skb->len,(pPktHdr->l3Offset+pPktHdr->l3Len));
			skb_trim(skb, (pPktHdr->l3Offset+pPktHdr->l3Len));
		}
	}

	if(rg_db.systemGlobal.fwdStatistic)
		_rtk_rg_fwdEngine_displayInfo(pPktHdr);

	//20151119LUKE: for multicast packet we should never hit shortcut!
	if(pPktHdr->pDmac[0]&0x1)goto SLOW_PATH;
	
#ifdef CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	//Check if we had add IPv6 Layer4 connection
	if(pPktHdr->tagif&IPV6_TAGIF && ((pPktHdr->tagif&TCP_TAGIF)||(pPktHdr->tagif&UDP_TAGIF)))
	{
		ret=_rtk_rg_ipv6StatefulDecision(skb,pPktHdr);
		if(ret==RG_FWDENGINE_RET_DIRECT_TX)goto direct_tx;
		else if(ret!=RG_FWDENGINE_RET_CONTINUE)return ret;	//RG_FWDENGINE_RET_TO_PS
	}
#endif

#ifdef CONFIG_ROME_NAPT_SHORTCUT
	if(!rg_db.systemGlobal.ipv4_shortcut_off){
		if(pPktHdr->ipv4FragPacket) goto OUT_OF_SHORTCUT;
		/* 192: SA learning limit */ 
		/* 022: Over MTU Size must calculate checksum by software.*/
		if((pPktHdr->tagif&IPV4_TAGIF)&&(!((pPktHdr->pRxDesc->opts3.bit.reason==192)||(pPktHdr->pRxDesc->opts3.bit.reason==22)))){
#if 1

hairpinNaptr:
			
			i=_rtk_rg_shortcutHashIndex(pPktHdr->ipv4Sip,pPktHdr->ipv4Dip,pPktHdr->sport,pPktHdr->dport);
			//TRACE("shortcut hash start=%d(sip=%x,sport=%d,dport=%d)",i,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->dport);
			for(j=i;j<i+MAX_NAPT_SHORTCUT_WAYS;j++){
				pNaptSc=&rg_db.naptShortCut[j];
				if((pPktHdr->ipv4Sip!=pNaptSc->sip) || (pPktHdr->sport!=pNaptSc->sport) 
					|| (pPktHdr->dport!=pNaptSc->dport) || (pPktHdr->ipv4Dip!=pNaptSc->dip)) continue;
				if(pNaptSc->isTcp==0){
					if((pPktHdr->tagif&UDP_TAGIF)==0) continue;
				}else{
					if((pPktHdr->tagif&TCP_TAGIF)==0) continue;
				}		
				
				if(pNaptSc->notFinishUpdated){
					//TABLE("del SC:[idx:%d]",j);
					//pNaptSc->sip=0;	//clear cracked shortcut
					_rtk_rg_v4ShortCut_delete(j);
					continue;
				}

				//NAPT shortcut
				if(rg_db.lut[pNaptSc->new_lut_idx].valid==0 || 
#if defined(CONFIG_RTL9600_SERIES)
#else	//support lut traffic bit
					rg_db.lut[pNaptSc->smacL2Idx].valid==0 || 
#endif
					(pNaptSc->arpIdx>=0 && (rg_db.arp[pNaptSc->arpIdx].rtk_arp.valid==0 || rg_db.arp[pNaptSc->arpIdx].ipv4Addr!=pPktHdr->ipv4Sip))) 
				{
					//TABLE("del SC:[idx:%d]",j);
					//pNaptSc->sip=0;
					_rtk_rg_v4ShortCut_delete(j);
					goto OUT_OF_SHORTCUT;
				}

				if(pPktHdr->tagif&TCP_TAGIF){
					//20140813LUKE: 
					//if tcp_hw_learning_at_syn is 0, syn or syn-ack should not check shortcut.
					//if tcp_hw_learning_at_syn is 1, syn should not check shortcut.
					//if((pPktHdr->tagif&TCP_TAGIF) && ((pPktHdr->tcpFlags.syn==1 && (pPktHdr->tcpFlags.ack==0 || (rg_db.systemGlobal.tcp_hw_learning_at_syn==0&&rg_db.systemGlobal.tcp_in_shortcut_learning_at_syn==0))) || (pPktHdr->tcpFlags.fin==1) || (pPktHdr->tcpFlags.reset==1) || (pNaptSc->isTcp==0))) goto OUT_OF_SHORTCUT;
					if((!rg_db.systemGlobal.tcpDisableStatefulTracking)&&(pPktHdr->tcpFlags.syn==1)&&(pPktHdr->tcpFlags.ack==0))	//20151203LUKE: disable TCP stateful tracking
						goto OUT_OF_SHORTCUT;

					if(!pNaptSc->isBridge){
						if(pNaptSc->direction==NAPT_DIRECTION_OUTBOUND){
							pPktHdr->fwdDecision=RG_FWD_DECISION_NAPT;
							if((rg_db.systemGlobal.tcp_short_timeout>1)&&(rg_db.naptOut[pNaptSc->naptIdx].state==SYN_ACK_RECV))
								goto OUT_OF_SHORTCUT; //first ACK goto slow path to add HW entry.	
							else if(rg_db.naptOut[pNaptSc->naptIdx].cannotAddToHw){
								//20151211LUKE: if this flow can't add to hw because of DPI, we clear the shortcut here
								//TABLE("del SC:[idx:%d]",j);
								//pNaptSc->sip=0;
								_rtk_rg_v4ShortCut_delete(j);
								goto OUT_OF_SHORTCUT;
							}
						}else{
							pPktHdr->fwdDecision=RG_FWD_DECISION_NAPTR;
							//if(rg_db.naptIn[pNaptSc->naptIdx].coneType!=NAPT_IN_TYPE_SYMMETRIC_NAPT)
							//	goto OUT_OF_SHORTCUT;
							if((rg_db.systemGlobal.tcp_short_timeout>1)&&(rg_db.naptOut[rg_db.naptIn[pNaptSc->naptIdx].symmetricNaptOutIdx].state==SYN_ACK_RECV))
								goto OUT_OF_SHORTCUT; //first ACK goto slow path to add HW entry.	
							else if(rg_db.naptOut[rg_db.naptIn[pNaptSc->naptIdx].symmetricNaptOutIdx].cannotAddToHw){
								//20151211LUKE: if this flow can't add to hw because of DPI, we clear the shortcut here
								//TABLE("del SC:[idx:%d]",j);
								//pNaptSc->sip=0;
								_rtk_rg_v4ShortCut_delete(j);
								goto OUT_OF_SHORTCUT;
							}
						}
					}
				}
				
				//20150203LUKE: if the ingressPort or egressPort is different since shortcut created, clear it and goto slow path.
				if(pNaptSc->spa!=pPktHdr->ingressPort){
					//20150812LUKE: replace dummy spa from ingressPort if needed
					if(pNaptSc->spa==RTK_RG_PORT_MAX){
						TRACE("replace spa for real incoming port%d!",pPktHdr->ingressPort);
						pNaptSc->spa=pPktHdr->ingressPort;
					}else{
						//TABLE("del SC:[idx:%d]",j);
						//pNaptSc->sip=0;
						_rtk_rg_v4ShortCut_delete(j);
						goto OUT_OF_SHORTCUT;
					}
				}

				if(pNaptSc->new_intf_idx>=0 && rg_db.netif[(uint8)(pNaptSc->new_intf_idx)].rtk_netif.mtu < pPktHdr->l3Len){
					pPktHdr->overMTU=1;
					TRACE("Packet L3 size(%d) is bigger than interface[%d]'s MTU(%d) in SC",pPktHdr->l3Len,pNaptSc->new_intf_idx,rg_db.netif[(uint8)(pNaptSc->new_intf_idx)].rtk_netif.mtu);
					goto OUT_OF_SHORTCUT;
				}				

				TRACE("Hit %s shortcut[%d].\n",(pNaptSc->isBridge?"L2":(pNaptSc->isNapt?"L4":"L3")),j);
				//update v4 shortcut idlse time
				pNaptSc->idleSecs = 0;
				pPktHdr->shortcutStatus=RG_SC_MATCH;
				pPktHdr->pCurrentShortcutEntry=pNaptSc;
				pPktHdr->delNaptConnection=0;	//init
				pPktHdr->addNaptAfterNicTx=0;	//init
				pPktHdr->dmacL2Idx=pNaptSc->new_lut_idx;
				pPktHdr->netifIdx=pNaptSc->new_intf_idx;					
				pPktHdr->extipIdx=pNaptSc->new_eip_idx;
				
				pPktHdr->egressDSCP=pNaptSc->dscp>=0?pNaptSc->dscp:((*pPktHdr->pTos)>>2);	//keep original DSCP if no ACL or remarking
				pPktHdr->egressVlanID=pNaptSc->vlanID;
				pPktHdr->egressVlanTagif=pNaptSc->vlanTagif;
				pPktHdr->egressServiceVlanID=pNaptSc->serviceVlanID;
				pPktHdr->egressServiceVlanTagif=pNaptSc->serviceVlanTagif;
				pPktHdr->egressTagAccType=pNaptSc->tagAccType;
				pPktHdr->dmac2VlanID=pNaptSc->dmac2cvlanID;
				pPktHdr->dmac2VlanTagif=pNaptSc->dmac2cvlanTagif;
				pPktHdr->egressMACPort=rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.port; //pNaptSc->macPort;
				pPktHdr->egressExtPort=rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.ext_port; //pNaptSc->macPort;
				//pPktHdr->egressExtPort=pNaptSc->extPort;
				pPktHdr->egressPriority=pNaptSc->priority;
				pPktHdr->egressServicePriority=pNaptSc->servicePriority;
				pPktHdr->internalVlanID=pNaptSc->internalVlanID;
				pPktHdr->internalPriority=pNaptSc->internalCFPri;
				pPktHdr->egressUniPortmask=pNaptSc->uniPortmask;
#if defined(CONFIG_RTL9602C_SERIES)
				//mib counter and cf decision
				pPktHdr->mibNetifIdx=pNaptSc->mibNetifIdx;
				pPktHdr->mibDirect=pNaptSc->mibDirect;
				pPktHdr->mibTagDelta=pNaptSc->mibTagDelta;
#endif
				//20150909LUKE: update ARP idle time when hit SC!
				if(pNaptSc->arpIdx>=0)
				{
					rg_db.arp[pNaptSc->arpIdx].idleSecs=0;
					rg_db.arp[pNaptSc->arpIdx].sendReqCount=0;
				}
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
				rg_db.lut[pNaptSc->smacL2Idx].idleSecs=0;
#endif

#ifdef CONFIG_GPON_FEATURE						
				pPktHdr->streamID=pNaptSc->streamID;
#endif
				if(rg_db.systemGlobal.fwdStatistic){
					rg_db.systemGlobal.statistic.perPortCnt_shortcut[pPktHdr->ingressPort]++;
					if(pNaptSc->isBridge)rg_db.systemGlobal.statistic.perPortCnt_L2FWD[pPktHdr->ingressPort]++;
				}

				if(pNaptSc->isBridge){
					pPktHdr->fwdDecision=RG_FWD_DECISION_BRIDGING;
				}else{
					if(pNaptSc->isNapt){
						if(pPktHdr->fwdDecision==RG_FWD_DECISION_NAPT){					
							pPktHdr->naptOutboundIndx=pNaptSc->naptIdx;
							pPktHdr->naptrInboundIndx=rg_db.naptOut[pPktHdr->naptOutboundIndx].rtk_naptOut.hashIdx;
							
							if((pPktHdr->tagif&TCP_TAGIF)&&((pPktHdr->tcpFlags.syn==1)||(pPktHdr->tcpFlags.fin==1)||(pPktHdr->tcpFlags.reset==1)))
								_rtk_rg_fwdEngine_TCPOutboundConnectionTracking(pPktHdr,&pPktHdr->naptOutboundIndx);
						}else{
							pPktHdr->naptrInboundIndx=pNaptSc->naptIdx;
							if(rg_db.naptIn[pPktHdr->naptrInboundIndx].coneType==NAPT_IN_TYPE_SYMMETRIC_NAPT)
								pPktHdr->naptOutboundIndx=rg_db.naptIn[pPktHdr->naptrInboundIndx].symmetricNaptOutIdx;
							else //full cone can't get outIdx by naptIn entry correctly.
								pPktHdr->naptOutboundIndx=_rtk_rg_naptTcpUdpInHashIndexLookup(1,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->ipv4Dip,pPktHdr->dport);
				
							if((pPktHdr->tagif&TCP_TAGIF)&&((pPktHdr->tcpFlags.syn==1)||(pPktHdr->tcpFlags.fin==1)||(pPktHdr->tcpFlags.reset==1)))
								_rtk_rg_fwdEngine_TCPInboundConnectionTracking(pPktHdr,&pPktHdr->naptOutboundIndx);
						}
						rg_db.naptOut[pPktHdr->naptOutboundIndx].idleSecs = 0;
						rg_db.naptIn[pPktHdr->naptrInboundIndx].idleSecs = 0;

						pPktHdr->l3Modify=1;
						pPktHdr->l4Modify=1;
						//ret = _rtk_rg_fwdEngine_shortCutNaptPacketModify(pNaptSc->direction,pNaptSc->naptIdx,pPktHdr,skb,1,1);					
					}else{
						pPktHdr->fwdDecision=RG_FWD_DECISION_ROUTING;
						pPktHdr->l3Modify=0;
						pPktHdr->l4Modify=0;
						//ret = _rtk_rg_fwdEngine_shortCutNaptPacketModify(0,0,pPktHdr,skb,0,0);
					}
					if(_rtk_rg_fwdEngine_shortCutNaptPacketModify(pNaptSc->direction,pNaptSc->naptIdx,pPktHdr,skb,pPktHdr->l3Modify,pPktHdr->l4Modify)!=RT_ERR_RG_OK)return RG_FWDENGINE_RET_DROP;
					//if(ret!=RT_ERR_RG_OK) return RG_FWDENGINE_RET_DROP;
					//dump_packet(skb->data,skb->len,"sc");
					if((pNaptSc->isHairpinNat)&&(pNaptSc->direction==NAPT_DIRECTION_OUTBOUND)){
						pPktHdr->ipv4Sip=*pPktHdr->pIpv4Sip;
						pPktHdr->sport=*pPktHdr->pSport;
						goto hairpinNaptr;
					}
				}
				goto direct_tx;
			}
#else
			for(i=0;i<MAX_NAPT_SHORTCUT_SIZE;i++)
			{
				pNaptSc=&rg_db.naptShortCut[i];
				//TRACE("SC: %d %x %d %x %d notFinishUpdated:%d new_lut:%d\n",pNaptSc->isTcp,pNaptSc->sip,pNaptSc->sport,pNaptSc->dip,pNaptSc->dport,pNaptSc->notFinishUpdated,pNaptSc->new_lut_idx);
				//TRACE("Packet: %x %x %d %x %d\n",pPktHdr->tagif,pPktHdr->ipv4Sip,pPktHdr->sport,pPktHdr->ipv4Dip,pPktHdr->dport);

				if(pNaptSc->notFinishUpdated) continue;
				if(pPktHdr->sport!=pNaptSc->sport) continue;
				if(pPktHdr->ipv4Sip!=pNaptSc->sip) continue;
				if(pPktHdr->dport!=pNaptSc->dport) continue;
				if(pPktHdr->ipv4Dip!=pNaptSc->dip) continue;

				if(pNaptSc->isNapt)
				{
					//20140811LUKE: 
					//if tcp_hw_learning_at_syn is 0, syn or syn-ack should not check shortcut.
					//if tcp_hw_learning_at_syn is 1, syn should not check shortcut.
					if((pPktHdr->tagif&TCP_TAGIF) && ((pPktHdr->tcpFlags.syn==1 && (pPktHdr->tcpFlags.ack==0 || rg_db.systemGlobal.tcp_hw_learning_at_syn==0)) || (pPktHdr->tcpFlags.fin==1) || (pPktHdr->tcpFlags.reset==1) || (pNaptSc->isTcp==0))) continue;
					if((pPktHdr->tagif&UDP_TAGIF) && (pNaptSc->isTcp==1)) continue;
					if(pPktHdr->ipv4FragPacket) continue;

					//NAPT shortcut
					if(((pPktHdr->tagif&TCP_TAGIF)&&(pPktHdr->tcpFlags.fin==0)&&(pPktHdr->tcpFlags.reset==0))||(pPktHdr->tagif&UDP_TAGIF))
					{
						if(rg_db.lut[pNaptSc->new_lut_idx].valid==0) break;
						TRACE("Hit NAPT shortcut[%d].\n",i);
						pPktHdr->shortcutStatus=RG_SC_MATCH;
						pPktHdr->dmacL2Idx=pNaptSc->new_lut_idx;
						pPktHdr->netifIdx=pNaptSc->new_intf_idx;					
						pPktHdr->extipIdx=pNaptSc->new_eip_idx;
						
						pPktHdr->egressDSCP=pNaptSc->dscp;
						pPktHdr->egressVlanID=pNaptSc->vlanID;
						pPktHdr->egressVlanTagif=pNaptSc->vlanTagif;
						pPktHdr->dmac2VlanID=pNaptSc->dmac2cvlanID;
						pPktHdr->dmac2VlanTagif=pNaptSc->dmac2cvlanTagif;
						pPktHdr->egressMACPort=pNaptSc->macPort;
						pPktHdr->egressExtPort=pNaptSc->extPort;
						pPktHdr->egressPriority=pNaptSc->priority;
						pPktHdr->internalPriority=pNaptSc->internalCFPri;
						pPktHdr->egressUniPortmask=pNaptSc->uniPortmask;
#ifdef CONFIG_GPON_FEATURE						
						pPktHdr->streamID=pNaptSc->streamID;
#endif
						ret = _rtk_rg_fwdEngine_shortCutNaptPacketModify(pNaptSc->direction,pNaptSc->naptIdx,pPktHdr,skb,1,1);
						assert_ok(ret);
						//dump_packet(skb->data,skb->len,"sc");
						goto direct_tx;	
					}
					
				}
				else
				{
					if((pPktHdr->tagif&TCP_TAGIF) && (pNaptSc->isTcp==0)) continue;
					if((pPktHdr->tagif&UDP_TAGIF) && (pNaptSc->isTcp==1)) continue;
					if(rg_db.lut[pNaptSc->new_lut_idx].valid==0) break;
					//Routing shortcut
					TRACE("Hit Routing shortcut[%d].\n",i);
					pPktHdr->shortcutStatus=RG_SC_MATCH;
					pPktHdr->dmacL2Idx=pNaptSc->new_lut_idx;
					pPktHdr->netifIdx=pNaptSc->new_intf_idx;				
					pPktHdr->egressDSCP=pNaptSc->dscp;
					pPktHdr->egressVlanID=pNaptSc->vlanID;
					pPktHdr->egressVlanTagif=pNaptSc->vlanTagif;
					pPktHdr->dmac2VlanID=pNaptSc->dmac2cvlanID;
					pPktHdr->dmac2VlanTagif=pNaptSc->dmac2cvlanTagif;
					pPktHdr->egressMACPort=rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.port; //pNaptSc->macPort;
					pPktHdr->egressExtPort=rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.ext_port; //pNaptSc->macPort;
					pPktHdr->egressPriority=pNaptSc->priority;
					pPktHdr->internalPriority=pNaptSc->internalCFPri;
					pPktHdr->egressUniPortmask=pNaptSc->uniPortmask;
#ifdef CONFIG_GPON_FEATURE
					pPktHdr->streamID=pNaptSc->streamID;
#endif
					ret = _rtk_rg_fwdEngine_shortCutNaptPacketModify(0,0,pPktHdr,skb,0,0);
					assert_ok(ret);
					//dump_packet(skb->data,skb->len,"sc");
					goto direct_tx; 			
				}
			}
#endif
		}
	}
OUT_OF_SHORTCUT:
#endif
#ifdef CONFIG_RG_IPV6_SOFTWARE_SHORTCUT_SUPPORT
	if(!rg_db.systemGlobal.ipv6_shortcut_off){
		if(pPktHdr->ipv6FragPacket) goto OUT_OF_V6SHORTCUT;
		/* 192: SA learning limit */ 
		/* 022: Over MTU Size must calculate checksum by software.*/
		if((pPktHdr->tagif&IPV6_TAGIF)&&(!((pPktHdr->pRxDesc->opts3.bit.reason==192)||(pPktHdr->pRxDesc->opts3.bit.reason==22)))){
			//use the last 32bits of SIP to hash!!
			i=_rtk_rg_ipv6ShortcutHashIndex(*((unsigned int *)(pPktHdr->pIpv6Sip+12)),pPktHdr->sport,pPktHdr->dport);
			for(j=i;j<i+MAX_NAPT_V6_SHORTCUT_WAYS;j++){
				pNaptV6Sc=&rg_db.naptv6ShortCut[j];
				if(memcmp(pPktHdr->pIpv6Sip,pNaptV6Sc->sip.ipv6_addr,IPV6_ADDR_LEN)) continue;
				if(pPktHdr->sport!=pNaptV6Sc->sport) continue;
				if(pPktHdr->dport!=pNaptV6Sc->dport) continue;
				if(memcmp(pPktHdr->pIpv6Dip,pNaptV6Sc->dip.ipv6_addr,IPV6_ADDR_LEN)) continue;
				if(pNaptV6Sc->isTcp==0){
					if((pPktHdr->tagif&UDP_TAGIF)==0) continue;
				}else{
					if((pPktHdr->tagif&TCP_TAGIF)==0) continue;
				}
				
				if(pNaptV6Sc->notFinishUpdated){
					//bzero(pNaptV6Sc->sip.ipv6_addr,IPV6_ADDR_LEN);	//clear cracked shortcut
					_rtk_rg_v6ShortCut_delete(j);
					continue;
				}
				if(rg_db.lut[pNaptV6Sc->new_lut_idx].valid==0 || 
#if defined(CONFIG_RTL9600_SERIES)
#else	//support lut traffic bit
					rg_db.lut[pNaptV6Sc->smacL2Idx].valid==0 || 
#endif
					(pNaptV6Sc->neighborIdx>=0 && rg_db.v6neighbor[pNaptV6Sc->neighborIdx].rtk_v6neighbor.valid==0))
				{
					//TABLE("del v6SC:[idx:%d]",j);
					//bzero(pNaptV6Sc->sip.ipv6_addr,IPV6_ADDR_LEN);
					_rtk_rg_v6ShortCut_delete(j);
					goto OUT_OF_V6SHORTCUT;
				}
				
				//20150203LUKE: if the egressPort is different since shortcut created, clear it and goto slow path.
				if(pNaptV6Sc->spa!=pPktHdr->ingressPort){
					//bzero(pNaptV6Sc->sip.ipv6_addr,IPV6_ADDR_LEN);
					_rtk_rg_v6ShortCut_delete(j);
					goto OUT_OF_V6SHORTCUT;
				}
				
				TRACE("Hit IPv6 shortcut[%d].\n",j);
				//update v4 shortcut idlse time
				pNaptV6Sc->idleSecs = 0;
				pPktHdr->shortcutStatus=RG_SC_MATCH;
				pPktHdr->pCurrentV6ShortcutEntry=pNaptV6Sc;
				pPktHdr->delNaptConnection=0;	//init
				pPktHdr->addNaptAfterNicTx=0;	//init
				pPktHdr->dmacL2Idx=pNaptV6Sc->new_lut_idx;
				pPktHdr->netifIdx=pNaptV6Sc->new_intf_idx;					
				pPktHdr->extipIdx=FAIL;
				
				pPktHdr->egressDSCP=pNaptV6Sc->dscp>=0?pNaptV6Sc->dscp:((((*pPktHdr->pTos)&0xf)<<2) | (((*(pPktHdr->pTos+1))&0xc0)>>6));	//keep original DSCP if no ACL or remarking
				pPktHdr->egressVlanID=pNaptV6Sc->vlanID;
				pPktHdr->egressVlanTagif=pNaptV6Sc->vlanTagif;
				pPktHdr->egressServiceVlanID=pNaptV6Sc->serviceVlanID;
				pPktHdr->egressServiceVlanTagif=pNaptV6Sc->serviceVlanTagif;
				pPktHdr->egressTagAccType=pNaptV6Sc->tagAccType;
				pPktHdr->dmac2VlanID=pNaptV6Sc->dmac2cvlanID;
				pPktHdr->dmac2VlanTagif=pNaptV6Sc->dmac2cvlanTagif;
				pPktHdr->egressMACPort=rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.port; //pNaptSc->macPort;
				pPktHdr->egressExtPort=rg_db.lut[pPktHdr->dmacL2Idx].rtk_lut.entry.l2UcEntry.ext_port; //pNaptSc->macPort;

				pPktHdr->egressPriority=pNaptV6Sc->priority;
				pPktHdr->egressServicePriority=pNaptV6Sc->servicePriority;
				pPktHdr->internalVlanID=pNaptV6Sc->internalVlanID;
				pPktHdr->internalPriority=pNaptV6Sc->internalCFPri;
				pPktHdr->egressUniPortmask=pNaptV6Sc->uniPortmask;
#if defined(CONFIG_RTL9602C_SERIES)
				//mib counter and cf decision
				pPktHdr->mibNetifIdx=pNaptV6Sc->mibNetifIdx;
				pPktHdr->mibDirect=pNaptV6Sc->mibDirect;
				pPktHdr->mibTagDelta=pNaptV6Sc->mibTagDelta;
#endif
				//20150909LUKE: update Neighbor idle time when hit SC!
				if(pNaptV6Sc->neighborIdx>=0)
				{
					rg_db.v6neighbor[pNaptV6Sc->neighborIdx].idleSecs=0;
				}
#if defined(CONFIG_RTL9600_SERIES)		
#else	//support lut traffic bit
				rg_db.lut[pNaptV6Sc->smacL2Idx].idleSecs=0;
#endif

#ifdef CONFIG_GPON_FEATURE						
				pPktHdr->streamID=pNaptV6Sc->streamID;
#endif
				if(rg_db.systemGlobal.fwdStatistic){
					rg_db.systemGlobal.statistic.perPortCnt_shortcutv6[pPktHdr->ingressPort]++;			
				}

				if(pNaptV6Sc->isBridge){
					pPktHdr->fwdDecision=RG_FWD_DECISION_BRIDGING;
				}else{
					pPktHdr->fwdDecision=RG_FWD_DECISION_V6ROUTING;
					
					if(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.is_wan && 
						((rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE)||
						(rg_db.systemGlobal.interfaceInfo[pPktHdr->netifIdx].storedInfo.wan_intf.wan_intf_conf.wan_type==RTK_RG_PPPoE_DSLITE)))
						_rtk_rg_fwdEngine_ipv6ShortCutPacketModify(0,L34_NH_PPPOE,pPktHdr,skb);
					else
						_rtk_rg_fwdEngine_ipv6ShortCutPacketModify(0,L34_NH_ETHER,pPktHdr,skb);
					
					//dump_packet(skb->data,skb->len,"sc");
				}
				goto direct_tx;
			}
		}
	}
OUT_OF_V6SHORTCUT:	
#endif
SLOW_PATH:
	ret=_rtk_rg_fwdEngineSlowPath(skb,pPktHdr);
	if(ret!=RG_FWDENGINE_RET_DIRECT_TX_SLOW)return ret;

direct_tx:
	
	ret=_rtk_rg_fwdEngineDirectTx(skb,pPktHdr);

	if(pPktHdr->addNaptAfterNicTx|pPktHdr->delNaptConnection)
		_rtk_rg_fwdEngine_updateFlowStatus(skb,pPktHdr);
	
	return _rtk_rg_fwdEngine_afterProcess(skb, pPktHdr, ret);
#if 0
dmac2cvid_tx:
#ifdef __KERNEL__
	//turn on txInfo mask, otherwise value won't be add
	rg_kernel.txDescMask.opts2.bit.tx_vlan_action=0x3;
	rg_kernel.txDescMask.opts1.bit.ipcs=1;
	rg_kernel.txDescMask.opts1.bit.l4cs=1;
	rg_kernel.txDesc.opts1.bit.ipcs=1;
	rg_kernel.txDesc.opts1.bit.l4cs=1;
	rg_kernel.txDesc.opts2.bit.tx_vlan_action = 0x2;		//removing

	ret=_rtk_rg_fwdEngineDMAC2CVIDLookup(pPktHdr,LAN_FID,0);		//FIXME:we use only one FID right now
	if(ret!=RT_ERR_RG_OK)
		return RG_FWDENGINE_RET_TO_PS;

	_rtk_rg_splitJumboSendToNicWithTxInfoAndMask(pPktHdr,skb,&rg_kernel.txDesc,0,&rg_kernel.txDescMask);

#endif
//	DEBUG("");
	return RG_FWDENGINE_RET_DIRECT_TX;
#endif



}

#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
int _rtk_rg_fwdEngine_fromWWANToProtocolStack(struct sk_buff *skb, struct net_device *dev)
{
	TRACE("change skb dev to %s before enter netif_rx",dev->name);
	skb->protocol = eth_type_trans (skb, dev);
	skb->dev=dev;
	skb->from_dev=dev;
	
	if(rg_db.systemGlobal.fwdStatistic)
		rg_db.systemGlobal.statistic.perPortCnt_ToPS[rg_db.pktHdr->ingressPort]++;	
	
	netif_rx(skb);
	
	return RE8670_RX_STOP_SKBNOFREE;	//the SKB had been sended or droped, kfree is no need
}
#endif

#if defined(CONFIG_APOLLO_GPON_FPGATEST)
rtk_rg_successFailReturn_t _rtk_rg_store_vmac_free_list(struct sk_buff *skb)
{	
	rtk_rg_vmac_skb_linlList_t *pVmacEntry,*pNextEntry;
	
	if(list_empty(&rg_db.vmacSkbListFreeListHead))	
	{
		list_for_each_entry_safe(pVmacEntry,pNextEntry,&rg_db.vmacSkbListHead,vmac_list)
		{
			//Delete from head list
			list_del_init(&pVmacEntry->vmac_list);

			//Free skb
			dev_kfree_skb_any(pVmacEntry->skb);
			pVmacEntry->skb=NULL;

			//Add back to free list
			list_add(&pVmacEntry->vmac_list,&rg_db.vmacSkbListFreeListHead);
			break;
		}
	}

	//Get one from free list
	list_for_each_entry_safe(pVmacEntry,pNextEntry,&rg_db.vmacSkbListFreeListHead,vmac_list)		//just return the first entry right behind of head
	{	
		list_del_init(&pVmacEntry->vmac_list);
		break;
	}

	//store the skb in the list
	pVmacEntry->skb=skb_copy(skb,GFP_ATOMIC);
	if(pVmacEntry->skb==NULL){
		//Add back to free list
		list_add(&pVmacEntry->vmac_list,&rg_db.vmacSkbListFreeListHead);		
		WARNING("Error pVmacEntry->skb==NULL !!!!!");
		return RG_RET_FAIL;
	}
	WARNING("store skb[%x] into list[%p]",(unsigned int)pVmacEntry->skb&0xffff,pVmacEntry);

	//Add to hash head list
	list_add_tail(&pVmacEntry->vmac_list,&rg_db.vmacSkbListHead);

	return RG_RET_SUCCESS;
}
#endif

void _rtk_rg_get_netInfo_from_DHCP(rtk_rg_pktHdr_t *pPktHdr)
{
	int ret=0, reqIP=0/*, arpIdx*/;
	int optStart,optLen,msgType;
	char dev_name[MAX_LANNET_DEV_NAME_LENGTH];
	rtk_rg_dhcpMsgType_t dhcpMsg,*pDhcpMsg;
	
	if(pPktHdr->l4Len > (8+sizeof(dhcpMsg))){
		DEBUG("[DHCP-PACKET]:get DHCP Packet\n");
		optStart=pPktHdr->l4Offset+8+sizeof(dhcpMsg);
		msgType=pPktHdr->skb->data[optStart];
MORE_DHCP_OPT:
		DEBUG("msgType is %d, optStart is %d",msgType,optStart);
		switch(msgType){
			case 0x35:	//DHCP Message Type
				optLen=pPktHdr->skb->data[++optStart];	//length byte
				if(pPktHdr->skb->data[++optStart]!=0x3){	//care Request or Inform type only
					if(pPktHdr->skb->data[optStart]==0x8){	//Inform, get IP from 
						DEBUG("Inform type");
						pDhcpMsg = (rtk_rg_dhcpMsgType_t *)&pPktHdr->skb->data[pPktHdr->l4Offset+8];
						reqIP=pPktHdr->ipv4Sip;
						if(pDhcpMsg->ciaddr!=pPktHdr->ipv4Sip)
							DEBUG("sip %x is diff with ciaddr %x",pPktHdr->ipv4Sip,pDhcpMsg->ciaddr);
						ret|=0x2;
					}else
						return;
				}
				optStart+=optLen;
				if(optStart>pPktHdr->skb->len)break;
				msgType=pPktHdr->skb->data[optStart];
				goto MORE_DHCP_OPT;
			case 0xc:	//Host Name Option
				optLen=pPktHdr->skb->data[++optStart];	//length byte
				if(optLen>=MAX_LANNET_DEV_NAME_LENGTH){
					strncpy(dev_name,&pPktHdr->skb->data[++optStart],MAX_LANNET_DEV_NAME_LENGTH);
					dev_name[MAX_LANNET_DEV_NAME_LENGTH-1]='\0';
				}else{
					strncpy(dev_name,&pPktHdr->skb->data[++optStart],optLen);
					dev_name[(unsigned int)optLen]='\0';
				}
				DEBUG("dev name is %s",dev_name);
				optStart+=optLen;
				ret|=0x1;
				if(optStart>pPktHdr->skb->len || ret==0x3)break;
				msgType=pPktHdr->skb->data[optStart];
				goto MORE_DHCP_OPT;
			case 0x32:	//Requested IP address
				optLen=pPktHdr->skb->data[++optStart];	//length byte
				if(optLen==0x4)reqIP=ntohl(*(u32*)&pPktHdr->skb->data[++optStart]);
				DEBUG("req ip is %x",reqIP);
				optStart+=optLen;
				ret|=0x2;
				if(optStart>pPktHdr->skb->len || ret==0x3)break;
				msgType=pPktHdr->skb->data[optStart];
				goto MORE_DHCP_OPT;
			case 0xff:	//End Option
				break;
			default:
				optLen=pPktHdr->skb->data[++optStart];	//length byte
				optStart+=optLen;
				DEBUG("msgType is %d, length is %d, bypass..",msgType,optLen);
				msgType=pPktHdr->skb->data[++optStart];
				if(optStart>pPktHdr->skb->len || ret==0x3)break;
				goto MORE_DHCP_OPT;
		}
		if(ret==0x3){
			//keep info in SMAC entry
			if(rg_db.lut[pPktHdr->smacL2Idx].valid){
				strncpy(rg_db.lut[pPktHdr->smacL2Idx].dev_name,dev_name,MAX_LANNET_DEV_NAME_LENGTH);
				if(rg_db.lut[pPktHdr->smacL2Idx].rtk_lut.entry.l2UcEntry.port==RTK_RG_PORT_CPU && 
				rg_db.lut[pPktHdr->smacL2Idx].rtk_lut.entry.l2UcEntry.ext_port!=(RTK_RG_PORT_CPU-RTK_RG_PORT_CPU))
					rg_db.lut[pPktHdr->smacL2Idx].conn_type=RG_CONN_MAC_PORT;
				else
					rg_db.lut[pPktHdr->smacL2Idx].conn_type=RG_CONN_MAC_PORT;
				DEBUG("LUT[%d] dev_name:%s connType:%s",pPktHdr->smacL2Idx,rg_db.lut[pPktHdr->smacL2Idx].dev_name,rg_db.lut[pPktHdr->smacL2Idx].conn_type==RG_CONN_MAC_PORT?"MacPort":"Wifi");
			}
		}
	}
}


#ifdef CONFIG_SMP
int fwdEngine_rx_skb_bh(struct re_private *cp, struct sk_buff *skb,struct rx_info *pRxInfo)
#else
int fwdEngine_rx_skb(struct re_private *cp, struct sk_buff *skb,struct rx_info *pRxInfo)
#endif
{
	int rg_fwdengine_ret_code=0;
	int aclRet;

#ifdef CONFIG_SMP
	if(pRxInfo==NULL) pRxInfo=&rg_db.systemGlobal.rxInfoFromWLAN;
#else
	if(pRxInfo==NULL) {
		//20151027LUKE: patch from WLAN won't assig data_length for mib counter problem.
#ifdef CONFIG_RG_JUMBO_FRAME
		rg_db.systemGlobal.rxInfoFromWLAN.opts1.bit.data_length=(skb->len+4)&0x3fff;	//4byte CRC
#else
		rg_db.systemGlobal.rxInfoFromWLAN.opts1.bit.data_length=(skb->len+4)&0xfff;		//4byte CRC
#endif
		pRxInfo=&rg_db.systemGlobal.rxInfoFromWLAN;
	}
#endif
#if defined(CONFIG_APOLLO_GPON_FPGATEST)
	//if we are using virtualMAC_with_GPON_switch, we should store packet into queue for virtualMACOutput use.
	if((0x1<<pRxInfo->opts3.bit.src_port_num)&rg_db.systemGlobal.virtualMAC_with_PON_switch_mask.portmask)
	{
		WARNING("GET PACKET from virtualMAC_with_PON_switch PORTMASK 0x%x",rg_db.systemGlobal.virtualMAC_with_PON_switch_mask.portmask);
		//allocate skb and copy to it, get a entry from free list, store skb's pointer in it and reutrn STOP.
		_rtk_rg_store_vmac_free_list(skb);
		
		return RE8670_RX_STOP;
	}
#endif

	//if we receive packet during initial state, goto protocol stack bypass fwdEngine!!
	if(_rtk_rg_get_initState()==RTK_RG_DURING_INIT)
		return RE8670_RX_CONTINUE;

	//if hwnat state is RG_HWNAT_PROTOCOL_STACK, goto protocol stack bypass fwdEngine!!20140120LUKE
	//before goto protocol stack, we should enter WMUX for deciding interface!! 20140226LUKE
	if(rg_db.systemGlobal.hwnat_enable==RG_HWNAT_PROTOCOL_STACK){
		TRACE("rg_db.systemGlobal.hwnat_enable is RG_HWNAT_PROTOCOL_STACK, by pass fwdEngine");
		goto BYPASS_FWDENGINE;
	}

	rg_fwdengine_ret_code = rtk_rg_fwdEngineInput(cp,skb,(void*)pRxInfo);
	
	//Processing packets
	if(rg_fwdengine_ret_code==RG_FWDENGINE_RET_TO_PS || rg_fwdengine_ret_code==RG_FWDENGINE_RET_ACL_TO_PS)
	{
		if(rg_db.systemGlobal.fwdStatistic)
		{
			rg_db.systemGlobal.statistic.perPortCnt_ToPS[rg_db.pktHdr->ingressPort]++;			
		}

		//20150617LUKE: change sequence when trap to protocol stack.
		if((rg_db.pktHdr->tagif&PPTP_INNER_TAGIF)&&(rg_db.pktHdr->tagif&GRE_SEQ_TAGIF)){
			*(rg_db.pktHdr->pGRESequence)=htonl(*(rg_db.pktHdr->pServerGRESequence));
			*(rg_db.pktHdr->pServerGRESequence)+=1;
		}

		//20160119LUKE: parsing DHCP option 12 to get device name
		//20160126LUKE: parsing User Agent for device type, brand, os, etc.
		if(rg_db.systemGlobal.gatherLanNetInfo){
			if((rg_db.pktHdr->tagif&UDP_TAGIF)&&(rg_db.pktHdr->sport==68)&&(rg_db.pktHdr->dport==67))
				_rtk_rg_get_netInfo_from_DHCP(rg_db.pktHdr);
			if((rg_db.pktHdr->tagif&TCP_TAGIF)&&(rg_db.pktHdr->httpFirstPacket))
				_parse_Http_UserAgent(rg_db.pktHdr);
		}

#ifdef CONFIG_RG_LAYER2_SOFTWARE_LEARN
		/*Force L2 auto learning */
		if(rg_db.pktHdr->l2AgentCalled==0)
		{
			TRACE("To PS: If SMAC is not learned, fwdEngine will learn it!");
			_rtk_rg_layer2Agent(skb,rg_db.pktHdr);
		}
#endif
		
		//20140627LUKE:for ARP packet may be TRAP by ACL without entering Layer2Forward
		/*Force ARP auto learning if etherType=0x0806*/
		if((rg_db.pktHdr->tagif&ARP_TAGIF) && rg_db.pktHdr->arpAgentCalled==0)
		{
			TRACE("To PS: If ARP is not learned, fwdEngine will learn it!");
			_rtk_rg_arpAgent(skb->data,skb->len,rg_db.pktHdr);
		}

		//20140826LUKE:for Neighbor Discovery packet may be TRAP by ACL without entering Layer2Forward
		/*Force Neighbor auto learning if ICMPv6*/
		if((rg_db.pktHdr->tagif&ICMPV6_TAGIF) && rg_db.pktHdr->neighborAgentCalled==0)
		{
			TRACE("To PS: If Neighbor is not learned, fwdEngine will learn it!");
			_rtk_rg_neighborAgent(skb,rg_db.pktHdr);
		}
		
		/*To PS by ACL Trap, skip fwd process*/
		if(rg_fwdengine_ret_code==RG_FWDENGINE_RET_ACL_TO_PS){
			rtk_rg_rxdesc_t	*pRxDesc=(rtk_rg_rxdesc_t *)pRxInfo;
			pRxDesc->opts2.bit.rsvd_2=0;
			TRACE("FWD_RX_O[%x]: To Protocol-Stack",(unsigned int)skb&0xffff);
			goto BYPASS_FWDENGINE;
		}
		//set vlan filter
		if(rg_db.pktHdr->ingressPort>=RTK_RG_PORT_CPU)
		{
			if(_rtk_rg_fwdEngineVLANFiltering(rg_db.pktHdr->internalVlanID,RTK_RG_PORT_CPU,rg_db.pktHdr->ingressPort-RTK_RG_PORT_CPU)==RG_FWDENGINE_RET_DROP)
			{
				TRACE("Drop by VLAN ingress filter");
				return RE8670_RX_STOP;
			}
		}
		else
		{
			if(_rtk_rg_fwdEngineVLANFiltering(rg_db.pktHdr->internalVlanID,rg_db.pktHdr->ingressPort,rg_db.pktHdr->ingressPort)==RG_FWDENGINE_RET_DROP)
			{
				TRACE("Drop by VLAN ingress filter");
				return RE8670_RX_STOP;
			}
		}
		/*To PS by fwd*/
		if(rg_db.pktHdr->egressMACPort!=7) rg_db.pktHdr->egressMACPort=RTK_RG_MAC_PORT_CPU; //make sure it is not to multiple-port.
		if(!(rg_db.vlan[rg_db.pktHdr->internalVlanID].UntagPortmask.bits[0]&(1<<RTK_RG_MAC_PORT_CPU))) rg_db.pktHdr->egressVlanTagif=1;
		TRACE("To PS: orgBit=%d, ingressCtagIf=%d, internalVlanID=%d(untagPmsk=0x%x), CPU(%s)",pRxInfo->opts1.bit.origformat,(rg_db.pktHdr->tagif&CVLAN_TAGIF)>0,rg_db.pktHdr->internalVlanID,rg_db.vlan[rg_db.pktHdr->internalVlanID].UntagPortmask.bits[0],(rg_db.vlan[rg_db.pktHdr->internalVlanID].UntagPortmask.bits[0]&(1<<RTK_RG_MAC_PORT_CPU))?"untagged":"tagged");
		if(1)//if(pRxInfo->opts1.bit.origformat)
		{

			//ACL("spa=%d  ",rg_db.pktHdr->pRxDesc->opts3.bit.src_port_num);
		
			//[FIXME] now, downstream(spa=PON) & to PS need to do CF.
			if(rg_db.pktHdr->pRxDesc->opts3.bit.src_port_num==RTK_RG_MAC_PORT_PON)
			{
				//[FIXME] for testing!!!!

				/*Ingress ACL check, avoid reserved acl trap to PS mechanism before _rtk_rg_ingressACLPatternCheck() */
				assert_ok(_rtk_rg_ingressACLPatternCheck(rg_db.pktHdr,NULL));
				aclRet = _rtk_rg_ingressACLAction(rg_db.pktHdr);
				if(aclRet==RG_FWDENGINE_RET_DROP){
					return RE8670_RX_STOP;
				}
				
				//Do ACL egress check.  To PS we consider as L2.
				TRACE("call to _rtk_rg_egressACLAction by FWD_TO_CPU");
				assert_ok(_rtk_rg_egressACLPatternCheck(RG_FWD_DECISION_BRIDGING,0,rg_db.pktHdr,skb,0,0,RTK_RG_PORT_CPU));	
				aclRet = _rtk_rg_egressACLAction(RG_FWD_DECISION_BRIDGING,rg_db.pktHdr);
				if(aclRet==RG_FWDENGINE_RET_DROP){
					TRACE("Drop by Egress ACL");
					//if(skb) _rtk_rg_dev_kfree_skb_any(skb); 					
					//return RG_FWDENGINE_RET_DROP;
					return RE8670_RX_STOP;
				}
			}else{
				TRACE("None related to CF port, directly FWD_TO_CPU without _rtk_rg_egressACLAction.");
				_rtk_rg_aclDecisionClear(rg_db.pktHdr);
			}
#if 0
			//ACL("l34CFRuleHit=%d cf64to511RuleHit=%d",rg_db.pktHdr->aclDecision.l34CFRuleHit,rg_db.pktHdr->aclDecision.cf64to511RuleHit);
			if((rg_db.pktHdr->aclDecision.l34CFRuleHit || rg_db.pktHdr->aclDecision.cf64to511RuleHit)){//if any CF rule hit, just do CF actions.(now only support ctag actions)				

				//ACL("To PS, but need to do egressACL(change CTag)");
				//Modify Packet by CF actions (tag final decision are recored in pktHdr)
				_rtk_rg_modifyPacketByACLAction(skb,rg_db.pktHdr,RTK_RG_PORT_CPU);
				_rtk_rg_vlanSvlanTag2SkbBuffer(skb,rg_db.pktHdr);			
			
			}else{//no hit CF, do normal CPU vlan change
				//check CPU port for tagging or untagging
				if((rg_db.vlan[rg_db.pktHdr->internalVlanID].UntagPortmask.bits[0]&(1<<RTK_RG_MAC_PORT_CPU))&&(rg_db.pktHdr->tagif&CVLAN_TAGIF))
				{
					TRACE("remove CTAG!");
					//removing original CTAG
#ifdef __KERNEL__					
					skb->protocol=((struct vlan_ethhdr *)skb->data)->h_vlan_encapsulated_proto;
					memmove(skb->data + VLAN_HLEN, skb->data, VLAN_ETH_ALEN<<1);
					skb_pull(skb,VLAN_HLEN);
					skb_reset_mac_header(skb);
#else
					printf("FIXME at %s %d\n",__FUNCTION__,__LINE__);
#endif
				}
				else if(((rg_db.vlan[rg_db.pktHdr->internalVlanID].UntagPortmask.bits[0]&(1<<RTK_RG_MAC_PORT_CPU))==0)&&((rg_db.pktHdr->tagif&CVLAN_TAGIF)==0))
				{
					TRACE("add CTAG!");
					//adding CTAG
#ifdef __KERNEL__				
					if(__vlan_put_tag(skb,rg_db.pktHdr->internalVlanID)==NULL)
						return RE8670_RX_STOP_SKBNOFREE;
#else
					printf("FIXME at %s %d\n",__FUNCTION__,__LINE__);
#endif
				}
				else
				{
					TRACE("no need to change CTAG state.(%s)",(rg_db.pktHdr->tagif&CVLAN_TAGIF)?"Tag":"Untag");
				}


			}

#endif
				//check CPU port for tagging or untagging
				if((rg_db.vlan[rg_db.pktHdr->internalVlanID].UntagPortmask.bits[0]&(1<<RTK_RG_MAC_PORT_CPU))&&(rg_db.pktHdr->tagif&CVLAN_TAGIF))
				{
					TRACE("remove CTAG!");
					//removing original CTAG
					rg_db.pktHdr->egressVlanTagif=0;
				}
				else if(((rg_db.vlan[rg_db.pktHdr->internalVlanID].UntagPortmask.bits[0]&(1<<RTK_RG_MAC_PORT_CPU))==0)&&((rg_db.pktHdr->tagif&CVLAN_TAGIF)==0))
				{
					TRACE("add CTAG!");
					//adding CTAG
					rg_db.pktHdr->egressVlanTagif=1;

				}
				else
				{
					TRACE("no need to change CTAG state.(%s)",(rg_db.pktHdr->tagif&CVLAN_TAGIF)?"Tag":"Untag");
				}
#if defined(CONFIG_RTL9602C_SERIES)
				//check CF hit for tagging or untagging.
				if(rg_db.pktHdr->aclDecision.aclEgrDoneAction){//if any CF rule hit, just do CF actions.			
					_rtk_rg_modifyPacketByACLAction(skb,rg_db.pktHdr,RTK_RG_PORT_CPU);
				}
#else
				//check CF hit for tagging or untagging.
				if((rg_db.pktHdr->aclDecision.l34CFRuleHit || rg_db.pktHdr->aclDecision.cf64to511RuleHit)){//if any CF rule hit, just do CF actions.(now only support ctag actions) 			
					_rtk_rg_modifyPacketByACLAction(skb,rg_db.pktHdr,RTK_RG_PORT_CPU);
				}
#endif
				//special case: to CPU, check the gponDsBcFilter by original skb, not copied skb.
				if(rg_db.systemGlobal.gponDsBCModuleEnable && (rg_db.pktHdr->ingressPort==RTK_RG_PORT_PON) && (((rg_db.pktHdr->pDmac[0]&rg_db.pktHdr->pDmac[1]&rg_db.pktHdr->pDmac[2]&rg_db.pktHdr->pDmac[3]&rg_db.pktHdr->pDmac[4]&rg_db.pktHdr->pDmac[5])==0xff)||(rg_db.pktHdr->pDmac[0]==0x01 && rg_db.pktHdr->pDmac[1]==0x00 && rg_db.pktHdr->pDmac[2]==0x5e)) && (rg_db.systemGlobal.initParam.wanPortGponMode==1))//must be GPON, BC, from PON
				{
					DEBUG("do gponDsBcFilterAndRemarking before fwd to PS!");
					_rtk_rg_egressPacketSend_for_gponDsBcFilterAndRemarking(skb,rg_db.pktHdr,2);
				}
				
				_rtk_rg_vlanSvlanTag2SkbBuffer(skb,rg_db.pktHdr);
			
		}
		else
		{
			TRACE("do nothing with CTAG!");
		}

		
#ifdef CONFIG_APOLLO_MODEL	
		TRACE("FWD_RX_O[%lx]: To Protocol-Stack",(unsigned long)skb&0xffff);
#else
		TRACE("FWD_RX_O[%x]: To Protocol-Stack",(unsigned int)skb&0xffff);

#if defined(CONFIG_APOLLO)
		// 20131219: patch the issue for wifi can't ping tagged protocol stack interface.
		TRACE("ingressport is %d, wlan_dev_idx is %d",rg_db.pktHdr->ingressPort,rg_db.pktHdr->wlan_dev_idx);
		if(rg_db.pktHdr->ingressPort==RTK_RG_EXT_PORT0)
		{
#ifdef CONFIG_RTL_CLIENT_MODE_SUPPORT
			//20150505LUKE: for WWAN to protocol stack we should change dev to vxd device
			if(rg_db.pktHdr->wlan_dev_idx==RG_WWAN_WLAN0_VXD)
			{
				return _rtk_rg_fwdEngine_fromWWANToProtocolStack(skb, wlan_vxd_netdev);
			}
			else if(rg_db.pktHdr->wlan_dev_idx==RG_WWAN_WLAN1_VXD)
			{
				return _rtk_rg_fwdEngine_fromWWANToProtocolStack(skb, wlan1_vxd_netdev);
			}
			else
#endif
			{
				skb->dev=nicRootDev; //if from wlan0, change it from eth0. so, we can remove the vlan tag for sending to WIFI TX packets.
				skb->from_dev=nicRootDev;
				cp=nicRootDevCp;	//20140210LUKE: from Master wifi didn't bring cp, so we have to recover it from dev here
			}
		}
#elif defined(CONFIG_XDSL_ROMEDRIVER)
		if(rg_db.pktHdr->ingressPort==RTK_RG_EXT_PORT0)
		{
			FIXME("skb->dev=nicRootDev");
		}
#endif

#endif
BYPASS_FWDENGINE:
#ifdef CONFIG_RG_WMUX_SUPPORT
		//do wmux(wan mux) here
		
		{
			int ret;
			if(cp)
				ret=wmux_pkt_recv(skb,decideRxDevice(cp,pRxInfo));	
			else
				ret=wmux_pkt_recv(skb,skb->dev);		//for packets from WLAN

			if((rg_db.systemGlobal.psRxMirrorToPort0)&&(ret==RE8670_RX_CONTINUE))
			{
				if(cp)
					_rtk_rg_psRxMirrorToPort0(skb,decideRxDevice(cp,pRxInfo));	
				else
					_rtk_rg_psRxMirrorToPort0(skb,skb->dev);			
			}
			return ret;
			
		}
#else
		if(rg_db.systemGlobal.psRxMirrorToPort0)
		{
			if(cp)
				_rtk_rg_psRxMirrorToPort0(skb,decideRxDevice(cp,pRxInfo));	
			else
				_rtk_rg_psRxMirrorToPort0(skb,skb->dev);		//for packets from WLAN		
		}
		return RE8670_RX_CONTINUE;
#endif
	}
	else if(rg_fwdengine_ret_code == RG_FWDENGINE_RET_DROP)
	{
		if(rg_db.systemGlobal.fwdStatistic)
		{
			rg_db.systemGlobal.statistic.perPortCnt_Drop[rg_db.pktHdr->ingressPort]++;			
		}	
#ifdef CONFIG_APOLLO_MODEL	
		TRACE("FWD_RX_O[%lx]: Drop",(unsigned long)skb&0xffff);
#else
		TRACE("FWD_RX_O[%x]: Drop",(unsigned int)skb&0xffff);
#endif
		//printk("drop skb=%x skb->data=%x\n",(u32)skb,(u32)skb->data);
		return RE8670_RX_STOP;			//drop this packet by caller
	}
	else if(rg_db.pktHdr->fwdDecision==RG_FWD_DECISION_PORT_ISO)
	{
#ifdef CONFIG_APOLLO_MODEL
		TRACE("FWD_RX_O[%lx]: Stop by port-isolation",(unsigned long)skb&0xffff);
#else
		TRACE("FWD_RX_O[%x]: Stop by port-isolation",(unsigned int)skb&0xffff);
#endif
		return RE8670_RX_STOP_SKBNOFREE;		//the SKB had been freed, kfree is no need
	}
	else//cxy: fwd engine processed skb
	{
#ifdef CONFIG_APOLLO_MODEL
		TRACE("FWD_RX_O[%lx]: Processed.(ret=%d)",(unsigned long)skb&0xffff,rg_fwdengine_ret_code);
#else
		TRACE("FWD_RX_O[%x]: Processed.(ret=%d)",(unsigned int)skb&0xffff,rg_fwdengine_ret_code);
#endif
		return RE8670_RX_STOP_SKBNOFREE;		//the SKB had been sended or queued, kfree is no need
	}
}


#ifdef CONFIG_SMP
void _rtk_rg_tasklet_queue_func(struct rg_private *rg_data)
{
	int r;
	int start_index,end_index;
	unsigned long lock_flags;

	while(1)
	{
		rg_tasklet_queue_lock_irqsave(&rg_kernel.rg_tasklet_queue_lock,lock_flags);
		start_index=atomic_read(&rg_data->start_index);
		end_index=atomic_read(&rg_data->end_index);	
		if(start_index==end_index) 
		{
			rg_tasklet_queue_unlock_irqrestore(&rg_kernel.rg_tasklet_queue_lock,lock_flags);
			break;
		}
		atomic_set(&rg_data->start_index,(start_index+1)&(MAX_RG_TASKLET_QUEUE_SIZE-1));	
		rg_tasklet_queue_unlock_irqrestore(&rg_kernel.rg_tasklet_queue_lock,lock_flags);
		
		//printk("start_idx=%d skb=%x rxdesc=%x\n",start_index,(u32)rg_data->skb[start_index],(u32)&rg_data->rxInfo[start_index]);

		switch(rg_data->tasklet_type[start_index])
		{
			case RG_TASKLET_TYPE_FROM_NIC:
			case RG_TASKLET_TYPE_FROM_WIFI:
				if(rg_data->tasklet_type[start_index]==RG_TASKLET_TYPE_FROM_NIC)
					r=fwdEngine_rx_skb_bh(rg_data->nic_data,rg_data->skb[start_index],&rg_data->rxInfo[start_index]);				
				else				
					r=fwdEngine_rx_skb_bh(rg_data->nic_data,rg_data->skb[start_index],NULL);
				
				switch(r)
				{
					case RE8670_RX_STOP:
						kfree_skb(rg_data->skb[start_index]);
						break;
					case RE8670_RX_CONTINUE:
						if(rg_data->tasklet_type[start_index]==RG_TASKLET_TYPE_FROM_NIC) 
						{
							re8670_rx_skb(rg_data->nic_data,rg_data->skb[start_index],&rg_data->rxInfo[start_index]);
						}
						else // if(rg_data->tasklet_type[start_index]==RG_TASKLET_TYPE_FROM_NIC) 
						{
							struct sk_buff *skb=rg_data->skb[start_index];
							skb->protocol = eth_type_trans (skb, skb->dev);
							//dump_packet(skb->data,skb->len,"wifi netif_rx");
							netif_rx(skb);
						}
						break;
					case RE8670_RX_STOP_SKBNOFREE:			
						break;
				}
				break;

				
			case RG_TASKLET_TYPE_FROM_TIMER:
				rtk_rg_fwdEngineHouseKeepingTimerFuncTasklet(0);
				break;
		}			
	}	

}

int fwdEngine_rx_skb(struct re_private *cp, struct sk_buff *skb,struct rx_info *pRxInfo)
{
	//spin lock here
	unsigned long lock_flags;
	int end_index,next_end_index;

	rg_tasklet_queue_lock_irqsave(&rg_kernel.rg_tasklet_queue_lock,lock_flags);
	
	end_index=atomic_read(&rg_kernel.rg_tasklet_data.end_index);
	next_end_index=((end_index+1)&(MAX_RG_TASKLET_QUEUE_SIZE-1));
	
	if(next_end_index==atomic_read(&rg_kernel.rg_tasklet_data.start_index))
	{
		printk("RG tasklet queue full\n");
		rg_tasklet_queue_unlock_irqrestore(&rg_kernel.rg_tasklet_queue_lock,lock_flags);
		return RE8670_RX_STOP; //drop this packet, //must unlock before return 
	}
	else
	{
		rg_kernel.rg_tasklet_data.nic_data=cp;
		rg_kernel.rg_tasklet_data.skb[end_index]=skb;
		if(pRxInfo==NULL)
		{
			rg_kernel.rg_tasklet_data.tasklet_type[end_index]=RG_TASKLET_TYPE_FROM_WIFI;
#ifdef CONFIG_RG_JUMBO_FRAME
			rg_db.systemGlobal.rxInfoFromWLAN.opts1.bit.data_length=(skb->len+4)&0x3fff;	//4byte CRC
#else
			rg_db.systemGlobal.rxInfoFromWLAN.opts1.bit.data_length=(skb->len+4)&0xfff;		//4byte CRC
#endif
		}
		else
		{
			rg_kernel.rg_tasklet_data.tasklet_type[end_index]=RG_TASKLET_TYPE_FROM_NIC;
			memcpy(&rg_kernel.rg_tasklet_data.rxInfo[end_index],pRxInfo,sizeof(struct rx_info));		
		}		
		atomic_set(&rg_kernel.rg_tasklet_data.end_index,next_end_index);

		tasklet_hi_schedule(&rg_kernel.rg_tasklets);
		rg_tasklet_queue_unlock_irqrestore(&rg_kernel.rg_tasklet_queue_lock,lock_flags);
		return RE8670_RX_STOP_SKBNOFREE; //must unlock before return 
	}

	rg_tasklet_queue_unlock_irqrestore(&rg_kernel.rg_tasklet_queue_lock,lock_flags);
	return RE8670_RX_STOP_SKBNOFREE; //must unlock before return 
}

#endif


#ifdef __KERNEL__
//#if defined(CONFIG_APOLLO)
void _rtk_rg_switchLinkChangeHandler(intrBcasterMsg_t	*pMsgData)
{
	int i;
#if 1
	rtk_rg_lut_linkList_t *pSoftLut,*pSoftLutNext;
	if(_rtk_rg_get_initState()==RTK_RG_INIT_FINISHED && pMsgData->intrType==MSG_TYPE_LINK_CHANGE && pMsgData->intrSubType==INTR_STATUS_LINKDOWN)
	{
		DEBUG("%s: get link-down event",__FUNCTION__);
		DEBUG("portIdx	= %u", pMsgData->intrBitMask);

		//Clear All shortcut, otherwise QoS remarking or DSCP may be diff because of PORT-MOVING!
		_rtk_rg_shortCut_clear();

		for(i=0;i<MAX_LUT_HW_TABLE_SIZE;i++)
		{
			if(rg_db.lut[i].valid && 
				rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC &&
				pMsgData->intrBitMask==rg_db.lut[i].rtk_lut.entry.l2UcEntry.port &&
				(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)==0 /*&&			//FIXME: this condition may not work if hw setting change
				(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)==0*/)		//FIXME: this condition may not work if hw setting change
			{
				assert_ok(RTK_L2_ADDR_DEL(&rg_db.lut[i].rtk_lut.entry.l2UcEntry));
			}
		}

		for(i=0;i<MAX_LUT_SW_TABLE_HEAD;i++)
		{
			//Check if we had been add to software LUT link-list
			if(!list_empty(&rg_db.softwareLutTableHead[i]))
			{
				list_for_each_entry_safe(pSoftLut,pSoftLutNext,&rg_db.softwareLutTableHead[i],lut_list)
				{
					if(rg_db.lut[pSoftLut->idx].valid && 
						rg_db.lut[pSoftLut->idx].rtk_lut.entryType==RTK_LUT_L2UC &&
						pMsgData->intrBitMask==rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.port &&
						(rg_db.lut[pSoftLut->idx].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)==0 /*&&			//FIXME: this condition may not work if hw setting change
						(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)==0*/)		//FIXME: this condition may not work if hw setting change
					{
						//Delete from head list
						list_del_init(&pSoftLut->lut_list);

						//Clear data
						//memset(&rg_db.lut[pSoftLut->idx],0,sizeof(rtk_rg_table_lut_t));
						rg_db.lut[pSoftLut->idx].valid=0;

						//Add back to free list
						list_add(&pSoftLut->lut_list,&rg_db.softwareLutFreeListHead);
					}
				}
			}
		}
	}

	if(pMsgData->intrType==MSG_TYPE_LINK_CHANGE && pMsgData->intrSubType==INTR_STATUS_LINKUP)
	{
		/* Link up: setup port mask */
		rg_db.portLinkupMask |= (1<<pMsgData->intrBitMask);
		DEBUG("@%s, port %d linkup, linkupmask = 0x%x", __FUNCTION__, pMsgData->intrBitMask, rg_db.portLinkupMask);
	}else if (pMsgData->intrType==MSG_TYPE_LINK_CHANGE && pMsgData->intrSubType==INTR_STATUS_LINKDOWN)
	{
		/* Link down: clear port mask */
		rg_db.portLinkupMask &= ~(1<<pMsgData->intrBitMask);
		DEBUG("@%s, port %d linkdown, linkupmask = 0x%x", __FUNCTION__, pMsgData->intrBitMask, rg_db.portLinkupMask);
	}
#else
	rtk_portmask_t linkDownPort;
#if defined(CONFIG_APOLLO)	
	//Check and clear link-down indicator register
	ret=rtk_intr_linkdownStatus_get(&linkDownPort);
	if(ret!=RT_ERR_OK)goto CLR_INTR;
	ret=rtk_intr_linkdownStatus_clear();
	if(ret!=RT_ERR_OK)goto CLR_INTR;
#elif defined(CONFIG_XDSL_ROMEDRIVER) 
//FIXME:rtl865x
#endif
	//Sync per-port LUT entry between hardware and software table
	DEBUG("the link-down portmask is %x",linkDownPort.bits[0]);
	if(linkDownPort.bits[0]==0x0)goto CLR_INTR;

	//Clear All shortcut, otherwise QoS remarking or DSCP may be diff because of PORT-MOVING!
	_rtk_rg_shortCut_clear();

	for(i=0;i<MAX_LUT_HW_TABLE_SIZE;i++)
	{
		if(rg_db.lut[i].valid && 
			rg_db.lut[i].rtk_lut.entryType==RTK_LUT_L2UC &&
			(linkDownPort.bits[0]&(0x1<<rg_db.lut[i].rtk_lut.entry.l2UcEntry.port)) &&
			(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_STATIC)==0 /*&&			//FIXME: this condition may not work if hw setting change
			(rg_db.lut[i].rtk_lut.entry.l2UcEntry.flags&RTK_L2_UCAST_FLAG_ARP_USED)==0*/)		//FIXME: this condition may not work if hw setting change
		{
#if defined(CONFIG_APOLLO)
			ret=rtk_l2_addr_del(&rg_db.lut[i].rtk_lut.entry.l2UcEntry);
#elif defined(CONFIG_XDSL_ROMEDRIVER)  
//FIXME:rtl865x
#endif
			if(ret==RT_ERR_OK)
			{
				_rtk_rg_layer2CleanL34ReferenceTable(i);
				if(rg_db.systemGlobal.accessWanLimitPortMask_member.portmask&(0x1<<(rg_db.lut[i].rtk_lut.entry.l2UcEntry.port)))
					atomic_dec(&rg_db.systemGlobal.accessWanLimitPortMaskCount);
				atomic_dec(&rg_db.systemGlobal.sourceAddrLearningCount[rg_db.lut[i].rtk_lut.entry.l2UcEntry.port]);
				if(_rtK_rg_checkCategoryPortmask(&rg_db.lut[i].rtk_lut.entry.l2UcEntry)==SUCCESS)
					atomic_dec(&rg_db.systemGlobal.accessWanLimitCategoryCount[(unsigned int)rg_db.lut[i].category]);
						
				rg_db.lut[i].valid=0;
			}
		}
	}
CLR_INTR:
	DEBUG("before clear the ISR state");
	//Clear ISR state
#if defined(CONFIG_APOLLO)
	rtk_intr_ims_clear(INTR_TYPE_LINK_CHANGE);
#elif defined(CONFIG_XDSL_ROMEDRIVER) 
#endif
#endif
}
//#endif  //end CONFIG_APOLLO


extern rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_TCP_h323(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);
extern rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_UDP_ras(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);
extern rtk_rg_fwdEngineAlgReturn_t rtk_rg_algRegFunc_rtsp(int direct, int after, unsigned char *pSkb,unsigned char *pPktInfo);
void _rtk_rg_alg_functions_registration(void)
{
	int i;
	//When the function is set to NULL, if the function is turn on, the packets will trap to protocol stack directly
	for(i=0;i<MAX_ALG_FUNCTIONS;i++)
	{
		rg_db.algTcpFunctionMapping[i].portNum=0;
		rg_db.algTcpFunctionMapping[i].registerFunction=NULL;
		rg_db.algUdpFunctionMapping[i].portNum=0;
		rg_db.algUdpFunctionMapping[i].registerFunction=NULL;
	}
	
	//TCP port
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_SIP_TCP].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_SIP_TCP];
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_H323_TCP].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_H323_TCP];
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_RTSP_TCP].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_RTSP_TCP];
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_FTP_TCP].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_FTP_TCP];

	//TCP function
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_SIP_TCP].registerFunction=NULL;
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_H323_TCP].registerFunction=rtk_rg_algRegFunc_TCP_h323;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_RTSP_TCP].registerFunction=NULL;
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_RTSP_TCP].registerFunction=rtk_rg_algRegFunc_rtsp;
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_FTP_TCP].registerFunction=rtk_rg_algRegFunc_ftp;//_rtk_rg_algRegFunc_TCP_FTP;

	//TCP ext port keep
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_SIP_TCP].keepExtPort=0;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_H323_TCP].keepExtPort=0;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_RTSP_TCP].keepExtPort=0;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_FTP_TCP].keepExtPort=0;
	
	//UDP port
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_SIP_UDP].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_SIP_UDP];
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_H323_UDP].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_H323_UDP];
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_RTSP_UDP].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_RTSP_UDP];
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_FTP_UDP].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_FTP_UDP];
	
	//UDP function
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_SIP_UDP].registerFunction=rtk_rg_algRegFunc_sip;//_rtk_rg_algRegFunc_UDP_SIP;
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_H323_UDP].registerFunction=rtk_rg_algRegFunc_UDP_ras;
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_RTSP_UDP].registerFunction=rtk_rg_algRegFunc_rtsp;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_RTSP_UDP].registerFunction=NULL;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_FTP_UDP].registerFunction=NULL;

	//UDP ext port keep
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_SIP_UDP].keepExtPort=0;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_H323_UDP].keepExtPort=0;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_RTSP_UDP].keepExtPort=0;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_FTP_UDP].keepExtPort=0;

	//TCP port Server in Lan
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_SIP_TCP_SRV_IN_LAN].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_SIP_TCP_SRV_IN_LAN];
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_H323_TCP_SRV_IN_LAN].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_H323_TCP_SRV_IN_LAN];
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN];
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_FTP_TCP_SRV_IN_LAN].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_FTP_TCP_SRV_IN_LAN];

	//TCP function Server in Lan
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_SIP_TCP_SRV_IN_LAN].registerFunction=NULL;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_H323_TCP_SRV_IN_LAN].registerFunction=NULL;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN].registerFunction=NULL;
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_FTP_TCP_SRV_IN_LAN].registerFunction=rtk_rg_algRegFunc_ftp;//_rtk_rg_algSrvInLanRegFunc_TCP_FTP;

	//TCP ext port keep
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_SIP_TCP_SRV_IN_LAN].keepExtPort=0;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_H323_TCP_SRV_IN_LAN].keepExtPort=0;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_RTSP_TCP_SRV_IN_LAN].keepExtPort=0;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_FTP_TCP_SRV_IN_LAN].keepExtPort=0;

	//UDP port Server in Lan
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_SIP_UDP_SRV_IN_LAN].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_SIP_UDP_SRV_IN_LAN];
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_H323_UDP_SRV_IN_LAN].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_H323_UDP_SRV_IN_LAN];
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN];
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_FTP_UDP_SRV_IN_LAN].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_FTP_UDP_SRV_IN_LAN];

	//UDP function Server in Lan
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_SIP_UDP_SRV_IN_LAN].registerFunction=NULL;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_H323_UDP_SRV_IN_LAN].registerFunction=NULL;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN].registerFunction=NULL;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_FTP_UDP_SRV_IN_LAN].registerFunction=NULL;

	//UDP ext port keep
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_SIP_UDP_SRV_IN_LAN].keepExtPort=0;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_H323_UDP_SRV_IN_LAN].keepExtPort=0;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_RTSP_UDP_SRV_IN_LAN].keepExtPort=0;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_FTP_UDP_SRV_IN_LAN].keepExtPort=0;

	//Pass through TCP port setting
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_PPTP_TCP_PASSTHROUGH].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_PPTP_TCP_PASSTHROUGH];
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_L2TP_TCP_PASSTHROUGH].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_L2TP_TCP_PASSTHROUGH];
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH];

	//Pass through TCP function setting
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_PPTP_TCP_PASSTHROUGH].registerFunction=rtk_rg_algRegFunc_pptp;//_rtk_rg_algRegFunc_TCP_PPTP;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_L2TP_TCP_PASSTHROUGH].registerFunction=NULL;
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH].registerFunction=NULL;

	//Pass through TCP ext port keep
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_PPTP_TCP_PASSTHROUGH].keepExtPort=0;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_L2TP_TCP_PASSTHROUGH].keepExtPort=0;
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_IPSEC_TCP_PASSTHROUGH].keepExtPort=0;

	//Pass through UDP port setting
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_PPTP_UDP_PASSTHROUGH].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_PPTP_UDP_PASSTHROUGH];
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_L2TP_UDP_PASSTHROUGH].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_L2TP_UDP_PASSTHROUGH];
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH];

	//Pass through UDP function setting
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_PPTP_UDP_PASSTHROUGH].registerFunction=NULL;
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_L2TP_UDP_PASSTHROUGH].registerFunction=_rtk_rg_algRegFunc_UDP_L2TP;
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH].registerFunction=rtk_rg_algRegFunc_IPsec;	//ipsec should be udp from 500 to 500

	//Pass through UDP ext port keep
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_PPTP_UDP_PASSTHROUGH].keepExtPort=0;
	rg_db.algUdpFunctionMapping[RTK_RG_ALG_L2TP_UDP_PASSTHROUGH].keepExtPort=1;
	//rg_db.algUdpFunctionMapping[RTK_RG_ALG_IPSEC_UDP_PASSTHROUGH].keepExtPort=0;

#ifdef CONFIG_RG_ROMEDRIVER_ALG_BATTLENET_SUPPORT
	//BattleNet TCP port setting
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_BATTLENET_TCP].portNum=rg_db.algUserDefinedPort[RTK_RG_ALG_BATTLENET_TCP];

	//BattleNet TCP function setting
	rg_db.algTcpFunctionMapping[RTK_RG_ALG_BATTLENET_TCP].registerFunction=_rtk_rg_algRegFunc_TCP_BattleNet;

	//BattleNet TCP ext port keep
	//rg_db.algTcpFunctionMapping[RTK_RG_ALG_BATTLENET_TCP].keepExtPort=0;
#endif
}
#endif

void _rtk_rg_fwdEngineGlobalVariableReset(void)
{
	int i;

	//init alg dynamic free link list
	INIT_LIST_HEAD(&rg_db.algDynamicFreeListHead);
	INIT_LIST_HEAD(&rg_db.algDynamicCheckListHead);

	for(i=0;i<MAX_ALG_DYNAMIC_PORT_NUM;i++)
	{
		INIT_LIST_HEAD(&rg_db.algDynamicFreeList[i].alg_list);
		rg_db.algDynamicFreeList[i].algFun=NULL;
		rg_db.algDynamicFreeList[i].portNum=0;
		rg_db.algDynamicFreeList[i].timeout=0;
		rg_db.algDynamicFreeList[i].isTCP=0;
		rg_db.algDynamicFreeList[i].serverInLan=0;
		rg_db.algDynamicFreeList[i].intIP=0;

		//add free list to free list head
		list_add_tail(&rg_db.algDynamicFreeList[i].alg_list,&rg_db.algDynamicFreeListHead);
	}

#ifdef	CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	//initialize ipv6 stateful data structure
	atomic_set(&rg_db.systemGlobal.v6StatefulConnectionNum,0);

	//init software stateful free link list
	INIT_LIST_HEAD(&rg_db.ipv6Layer4FreeListHead);
	//init software stateful head table
	for(i=0;i<MAX_IPV6_STATEFUL_HASH_HEAD_SIZE;i++)
		INIT_LIST_HEAD(&rg_db.ipv6Layer4HashListHead[i]);

	for(i=0;i<MAX_IPV6_STATEFUL_TABLE_SIZE;i++)
	{
		INIT_LIST_HEAD(&rg_db.ipv6Layer4FreeList[i].layer4_list);
		rg_db.ipv6Layer4FreeList[i].idleSecs=0;
		rg_db.ipv6Layer4FreeList[i].state=INVALID;
		rg_db.ipv6Layer4FreeList[i].valid=0;

		//add free list to free list head
		list_add_tail(&rg_db.ipv6Layer4FreeList[i].layer4_list,&rg_db.ipv6Layer4FreeListHead);
	}

	bzero(rg_db.ipv6FragmentQueue,MAX_IPV6_FRAGMENT_QUEUE_SIZE*sizeof(rtk_rg_fragment_queue_t));
#endif

	//initialize the IPv4 fragment table and queue
	rg_db.systemGlobal.ipv4FragmentQueueNum = 0;
	//for(i=0;i<MAX_IPV4_FRAGMENT_QUEUE_SIZE;i++)
	//{
		//if(i<MAX_IPV4_FRAGMENT_QUEUE_SIZE)
		//{
			bzero(rg_db.ipv4FragmentQueue,MAX_IPV4_FRAGMENT_QUEUE_SIZE*sizeof(rtk_rg_fragment_queue_t));
			//rg_db.ipv4FragmentQueue[i].queue_skb=NULL;
		//}
		//bzero(&rg_db.ipv4FragmentOutTable[i],sizeof(rtk_rg_fragment_napt_out_t));
		//rg_db.ipv4FragmentOutTable[i].pNaptOutboundEntry=NULL;
	//}

	//init napt Out free link list	
	rg_db.pNaptOutFreeListHead=&rg_db.naptOutFreeList[0];	
	
	for(i=MAX_NAPT_OUT_HW_TABLE_SIZE;i<MAX_NAPT_OUT_SW_TABLE_SIZE;i++)
	{
		rg_db.naptOutFreeList[i-MAX_NAPT_OUT_HW_TABLE_SIZE].idx=i;
		if(i+1<MAX_NAPT_OUT_SW_TABLE_SIZE)
			rg_db.naptOutFreeList[i-MAX_NAPT_OUT_HW_TABLE_SIZE].pNext=&rg_db.naptOutFreeList[i-MAX_NAPT_OUT_HW_TABLE_SIZE+1];
		else
			rg_db.naptOutFreeList[i-MAX_NAPT_OUT_HW_TABLE_SIZE].pNext=NULL;
	}

	//init napt out hash index link list
	//for(i=0;i<(MAX_NAPT_OUT_HW_TABLE_SIZE>>2);i++)
		bzero(rg_db.pNaptOutHashListHead,(MAX_NAPT_OUT_HW_TABLE_SIZE>>2)*sizeof(rtk_rg_table_naptOut_linkList_t	*));

	//init napt In free link list	
	rg_db.pNaptInFreeListHead=&rg_db.naptInFreeList[0];	

	for(i=MAX_NAPT_IN_HW_TABLE_SIZE;i<MAX_NAPT_IN_SW_TABLE_SIZE;i++)
	{
		rg_db.naptInFreeList[i-MAX_NAPT_IN_HW_TABLE_SIZE].idx=i;
		if(i+1<MAX_NAPT_IN_SW_TABLE_SIZE)
			rg_db.naptInFreeList[i-MAX_NAPT_IN_HW_TABLE_SIZE].pNext=&rg_db.naptInFreeList[i-MAX_NAPT_IN_HW_TABLE_SIZE+1];
		else
			rg_db.naptInFreeList[i-MAX_NAPT_IN_HW_TABLE_SIZE].pNext=NULL;
	}

	//init napt In hash index link list
	//for(i=0;i<(MAX_NAPT_IN_HW_TABLE_SIZE>>2);i++)
		bzero(rg_db.pNaptInHashListHead,(MAX_NAPT_IN_HW_TABLE_SIZE>>2)*sizeof(rtk_rg_table_naptIn_linkList_t *));

	//init fragment Out free link list	
	rg_db.pFragOutFreeListHead=&rg_db.fragOutFreeList[0];	
	
	for(i=0;i<MAX_FRAG_OUT_FREE_TABLE_SIZE;i++)
	{
		bzero(&rg_db.fragOutFreeList[i],sizeof(rtk_rg_ipv4_fragment_out_t));
		if(i+1<MAX_FRAG_OUT_FREE_TABLE_SIZE)
			rg_db.fragOutFreeList[i].pNext=&rg_db.fragOutFreeList[i+1];
		else
			rg_db.fragOutFreeList[i].pNext=NULL;
	}

	//init fragment out hash index link list
	//for(i=0;i<(MAX_NAPT_OUT_HW_TABLE_SIZE>>2);i++)
		bzero(rg_db.pFragOutHashListHead,(MAX_NAPT_OUT_HW_TABLE_SIZE>>2)*sizeof(rtk_rg_ipv4_fragment_out_t *));
	
	//init fragment In free link list	
	rg_db.pFragInFreeListHead=&rg_db.fragInFreeList[0];	

	for(i=0;i<MAX_FRAG_IN_FREE_TABLE_SIZE;i++)
	{
		bzero(&rg_db.fragInFreeList[i],sizeof(rtk_rg_ipv4_fragment_in_t));
		if(i+1<MAX_FRAG_IN_FREE_TABLE_SIZE)
			rg_db.fragInFreeList[i].pNext=&rg_db.fragInFreeList[i+1];
		else
			rg_db.fragInFreeList[i].pNext=NULL;
	}

	//init fragment In hash index link list
	//for(i=0;i<(MAX_NAPT_IN_HW_TABLE_SIZE>>2);i++)
		bzero(rg_db.pFragInHashListHead,(MAX_NAPT_IN_HW_TABLE_SIZE>>2)*sizeof(rtk_rg_ipv4_fragment_in_t	*));

	//init ICMP control flow head
	rg_db.pICMPCtrlFlowHead=&rg_db.icmpCtrlFlowLinkList[0];

	//init ICMP control flow free link list
	for(i=0;i<MAX_ICMPCTRLFLOW_SIZE;i++)
	{
		if(i==MAX_ICMPCTRLFLOW_SIZE-1)
			rg_db.icmpCtrlFlowLinkList[i].pNext=&rg_db.icmpCtrlFlowLinkList[0];		//ring-buffer
		else
			rg_db.icmpCtrlFlowLinkList[i].pNext=&rg_db.icmpCtrlFlowLinkList[i+1];

		if(i==0)
			rg_db.icmpCtrlFlowLinkList[i].pPrev=&rg_db.icmpCtrlFlowLinkList[MAX_ICMPCTRLFLOW_SIZE-1];
		else
			rg_db.icmpCtrlFlowLinkList[i].pPrev=&rg_db.icmpCtrlFlowLinkList[i-1];
	}

#if defined(CONFIG_APOLLO_GPON_FPGATEST)
	//init vmac skb head
	INIT_LIST_HEAD(&rg_db.vmacSkbListHead);
	INIT_LIST_HEAD(&rg_db.vmacSkbListFreeListHead);
	
	//init vmac skb free link list
	for(i=0;i<MAX_VMAC_SKB_QUEUE_SIZE;i++){
		INIT_LIST_HEAD(&rg_db.vmacSkbFreeList[i].vmac_list);
		if(rg_db.vmacSkbFreeList[i].skb)dev_kfree_skb_any(rg_db.vmacSkbFreeList[i].skb);
		rg_db.vmacSkbFreeList[i].skb=NULL;

		//add free list to free list head
		list_add_tail(&rg_db.vmacSkbFreeList[i].vmac_list,&rg_db.vmacSkbListFreeListHead);
	}
#endif

#ifdef __KERNEL__
	//used to reset all data of ALG
	_rtk_rg_alg_resetAllDataBase();
#endif
	
//LUKE20130816: move to separate ALG file
#if 0
	//init FTP control flow head
	rg_db.pAlgFTPCtrlFlowHead=&rg_db.algFTPCtrlFlowList[0]; 

	//init FTP control flow free link list	
	for(i=0;i<MAX_FTP_CTRL_FLOW_SIZE;i++)
	{
		if(i==MAX_FTP_CTRL_FLOW_SIZE-1)
			rg_db.algFTPCtrlFlowList[i].pNext=&rg_db.algFTPCtrlFlowList[0];		//ring-buffer
		else
			rg_db.algFTPCtrlFlowList[i].pNext=&rg_db.algFTPCtrlFlowList[i+1];

		if(i==0)
			rg_db.algFTPCtrlFlowList[i].pPrev=&rg_db.algFTPCtrlFlowList[MAX_FTP_CTRL_FLOW_SIZE-1];
		else
			rg_db.algFTPCtrlFlowList[i].pPrev=&rg_db.algFTPCtrlFlowList[i-1];
	}
#endif

//20130821LUKE:close it because implemented by separate module
#if 0
	//init PPTP link list
	for(i=0;i<MAX_NETIF_SW_TABLE_SIZE;i++)
	{
		rg_db.pPPTPGreOutboundHead[i]=&rg_db.pptpGreOutboundLinkList[i][0];
		
		for(j=0;j<MAX_PPTP_SESSION_SIZE;j++)
		{
			if(j==MAX_PPTP_SESSION_SIZE-1)
				rg_db.pptpGreOutboundLinkList[i][j].pNext=&rg_db.pptpGreOutboundLinkList[i][0];		//ring-buffer
			else
				rg_db.pptpGreOutboundLinkList[i][j].pNext=&rg_db.pptpGreOutboundLinkList[i][j+1];

			if(j==0)
				rg_db.pptpGreOutboundLinkList[i][j].pPrev=&rg_db.pptpGreOutboundLinkList[i][MAX_PPTP_SESSION_SIZE-1];
			else
				rg_db.pptpGreOutboundLinkList[i][j].pPrev=&rg_db.pptpGreOutboundLinkList[i][j-1];
		}
	}
#endif
	//init L2TP control flow head
	rg_db.pAlgL2TPCtrlFlowHead=&rg_db.algL2TPCtrlFlowLinkList[0]; 

	//init L2TP control flow free link list	
	for(i=0;i<MAX_L2TP_CTRL_FLOW_SIZE;i++)
	{
		if(i==MAX_L2TP_CTRL_FLOW_SIZE-1)
			rg_db.algL2TPCtrlFlowLinkList[i].pNext=&rg_db.algL2TPCtrlFlowLinkList[0];		//ring-buffer
		else
			rg_db.algL2TPCtrlFlowLinkList[i].pNext=&rg_db.algL2TPCtrlFlowLinkList[i+1];

		if(i==0)
			rg_db.algL2TPCtrlFlowLinkList[i].pPrev=&rg_db.algL2TPCtrlFlowLinkList[MAX_L2TP_CTRL_FLOW_SIZE-1];
		else
			rg_db.algL2TPCtrlFlowLinkList[i].pPrev=&rg_db.algL2TPCtrlFlowLinkList[i-1];
	}

	//reset access WAN limit timer
	if(timer_pending(&rg_kernel.lutReachLimit_portmask.timer))
		del_timer(&rg_kernel.lutReachLimit_portmask.timer);
	atomic_set(&rg_kernel.lutReachLimit_portmask.activity,0);
	for(i=0;i<WanAccessCategoryNum;i++)
	{
		if(timer_pending(&rg_kernel.lutReachLimit_category[i].timer))
			del_timer(&rg_kernel.lutReachLimit_category[i].timer);
		atomic_set(&rg_kernel.lutReachLimit_category[i].activity,0);
	}

#ifdef __KERNEL__	//model skip ALG
	//init Alg port-function mapping
	_rtk_rg_alg_functions_registration();
#endif	

}



int _rtk_rg_pasring_proc_string_to_integer(const char *buff,unsigned long len)
{
	char *tmpbuf;	
	int ret;
	tmpbuf=&rg_kernel.proc_parsing_buf[0];

	if (buff && !copy_from_user(tmpbuf, buff, len))	
	{		
		tmpbuf[len] = '\0';
	}	
	ret=simple_strtol(tmpbuf, NULL, 0);	

	return ret;
}

int _rtk_rg_igmpSnooping_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rg_db.systemGlobal.initParam.igmpSnoopingEnable=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	_rtk_rg_igmpSnoopingOnOff(rg_db.systemGlobal.initParam.igmpSnoopingEnable,0,rg_db.systemGlobal.initParam.ivlMulticastSupport);
	return len;
}


#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)

int _rtk_rg_igmp_fastLeave_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rtl_mCastModuleArray[rg_db.systemGlobal.nicIgmpModuleIndex].enableFastLeave=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	return len;
}

int _rtk_rg_igmp_groupMemberAgingTime_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rtl_mCastTimerParas.groupMemberAgingTime=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	return len;
}

int _rtk_rg_igmp_lastMemberAgingTime_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rtl_mCastTimerParas.lastMemberAgingTime=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	return len;
}

int _rtk_rg_igmp_querierPresentInterval_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rtl_mCastTimerParas.querierPresentInterval=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	return len;
}

int _rtk_rg_igmp_dvmrpRouterAgingTime_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rtl_mCastTimerParas.dvmrpRouterAgingTime=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	return len;
}

int _rtk_rg_igmp_mospfRouterAgingTime_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rtl_mCastTimerParas.mospfRouterAgingTime=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	return len;
}

int _rtk_rg_igmp_pimRouterAgingTime_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	rtl_mCastTimerParas.pimRouterAgingTime=_rtk_rg_pasring_proc_string_to_integer(buff,len);
	return len;
}

#endif

int ivlMulticastSupport_show(struct seq_file *s, void *v)
{
	PROC_PRINTF( "IVL Multicast Support: %d\n",rg_db.systemGlobal.initParam.ivlMulticastSupport);
	PROC_PRINTF( "* Set ivlMulticastSupport to 1 will use MAC+VID/FID multicast hash mode.\n");
	PROC_PRINTF( "* Set ivlMulticastSupport to 0 will use DIP only multicast hash mode.\n");
	return 0;
}

int _rtk_rg_ivlMulticastSupport_set( struct file *filp, const char *buff,unsigned long len, void *data )
{
	uint32 isIVL;	
	if(rg_db.systemGlobal.initParam.igmpSnoopingEnable)
	{	
		isIVL = _rtk_rg_pasring_proc_string_to_integer(buff, len);
#if 0
		rg_db.systemGlobal.initParam.ivlMulticastSupport = _rtk_rg_pasring_proc_string_to_integer(buff, len);
		if( rg_db.systemGlobal.initParam.ivlMulticastSupport == 1)
		{
			rtk_rg_l2_ipmcMode_set(LOOKUP_ON_MAC_AND_VID_FID);
		}
		else
		{
			if( RT_ERR_RG_OK == rtk_rg_l2_ipmcMode_set(LOOKUP_ON_DIP_AND_SIP) )
			{
				rtk_rg_l2_ipmcGroupLookupMissHash_set(HASH_DIP_ONLY);
			}	
		}
#endif		
		/* restart igmp module */
		_rtk_rg_igmpSnoopingOnOff(0, 0, isIVL);
		_rtk_rg_igmpSnoopingOnOff(1, 0, isIVL);
	}
	return len;
}

rtk_rg_proc_t fwdEngineProc[]=
{
#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)

	{
		.name="igmpSnooping" ,
		.get = igmp_show ,
		.set = _rtk_rg_igmpSnooping_set ,
	},
	{
		.name="igmp_fastLeave" ,
		.get = igmp_show ,
		.set = _rtk_rg_igmp_fastLeave_set ,
	},
	{
		.name="igmp_groupMemberAgingTime" ,
		.get = igmp_show ,
		.set = _rtk_rg_igmp_groupMemberAgingTime_set ,
	},
	{
		.name="igmp_lastMemberAgingTime" ,
		.get = igmp_show ,
		.set = _rtk_rg_igmp_lastMemberAgingTime_set ,
	},
	{
		.name="igmp_querierPresentInterval" ,
		.get = igmp_show ,
		.set = _rtk_rg_igmp_querierPresentInterval_set ,
	},
	{
		.name="igmp_dvmrpRouterAgingTime" ,
		.get = igmp_show ,
		.set = _rtk_rg_igmp_dvmrpRouterAgingTime_set ,
	},
	{
		.name="igmp_mospfRouterAgingTime" ,
		.get = igmp_show ,
		.set = _rtk_rg_igmp_mospfRouterAgingTime_set ,
	},
	{
		.name="igmp_pimRouterAgingTime" ,
		.get = igmp_show ,
		.set = _rtk_rg_igmp_pimRouterAgingTime_set ,
	},
	{
		.name="igmp_groupList_memDump" ,
		.get = igmp_groupList_memDump ,
		.set = NULL ,
	},
#endif
#ifdef CONFIG_RG_WMUX_SUPPORT
	{
		.name="wmux_init" ,
		.get = _rtk_rg_wmux_init_read ,
		.set = _rtk_rg_wmux_init_write ,
	},
	
	{
		.name="wmux_add" ,
		.get = _rtk_rg_wmux_add_read ,
		.set = _rtk_rg_wmux_add_write ,
	},
	{
		.name="wmux_del" ,
		.get = _rtk_rg_wmux_del_read ,
		.set = _rtk_rg_wmux_del_write ,
	},
	{
		.name="wmux_flag" ,
		.get = _rtk_rg_wmux_flag_read ,
		.set = _rtk_rg_wmux_flag_write ,
	},
	{
		.name="wmux_info" ,
		.get = _rtk_rg_wmux_info ,
		.set = NULL ,
	},
#endif
/* martin ZHU add for EPON oam */
	{
		.name="ivlMulticastSupport" ,
		.get = ivlMulticastSupport_show ,
		.set = _rtk_rg_ivlMulticastSupport_set ,
	},

};


static int NULL_fwdEngine_get(struct seq_file *s, void *v){ return 0;}
static int NULL_fwdEngine_single_open(struct inode *inode, struct file *file){return(single_open(file, NULL_fwdEngine_get, NULL));}


static int common_fwdEngine_single_open(struct inode *inode, struct file *file)
{
    int i;
    for( i=0; i< (sizeof(fwdEngineProc)/sizeof(rtk_rg_proc_t)) ;i++)
    {
		//printk("common_single_open inode_id=%u i_ino=%u\n",fwdEngineProc[i].inode_id,(unsigned int)inode->i_ino);
        if(fwdEngineProc[i].inode_id==(unsigned int)inode->i_ino)
        {
			return(single_open(file, fwdEngineProc[i].get, NULL));
        }
    }
    return -1;       
}


static ssize_t common_fwdEngine_single_write(struct file * file, const char __user * userbuf,
				size_t count, loff_t * off)
{
    int i;
    for( i=0; i< (sizeof(fwdEngineProc)/sizeof(rtk_rg_proc_t)) ;i++)
    {
		//printk("common_single_write inode_id=%u i_ino=%u\n",fwdEngineProc[i].inode_id,(unsigned int)file->f_dentry->d_inode->i_ino);
        if(fwdEngineProc[i].inode_id==(unsigned int)file->f_dentry->d_inode->i_ino)
        {
			return fwdEngineProc[i].set(file,userbuf,count,off);
        }
    }
    return -1;      
}


/* This function is called by romeDriver & test case */
int rtk_rg_rome_driver_init(void)
{
	int err,i;
	
#ifdef __KERNEL__	
	struct proc_dir_entry *p;
#endif

	_rtk_rg_fwdEngineGlobalVariableReset();

#if defined(CONFIG_RG_IGMP_SNOOPING) || defined(CONFIG_RG_MLD_SNOOPING)
	{		
		struct rtl_mCastSnoopingGlobalConfig mCastSnoopingGlobalConfig;
		rtl_multicastDeviceInfo_t devInfo;
		
		/* init igmp snooping module */ 	
		memset(&mCastSnoopingGlobalConfig, 0, sizeof(struct rtl_mCastSnoopingGlobalConfig));
		mCastSnoopingGlobalConfig.maxGroupNum=256;
		mCastSnoopingGlobalConfig.maxSourceNum=300;
		mCastSnoopingGlobalConfig.hashTableSize=64;
		mCastSnoopingGlobalConfig.groupMemberAgingTime=260;
		mCastSnoopingGlobalConfig.lastMemberAgingTime=10;
		mCastSnoopingGlobalConfig.querierPresentInterval=260;  
		mCastSnoopingGlobalConfig.dvmrpRouterAgingTime=120;
		mCastSnoopingGlobalConfig.mospfRouterAgingTime=120;
		mCastSnoopingGlobalConfig.pimRouterAgingTime=120;	
		ASSERT_EQ(rtl_initMulticastSnooping(mCastSnoopingGlobalConfig),SUCCESS);
		
		/* register igmp snooping module */
		ASSERT_EQ(rtl_registerIgmpSnoopingModule(&rg_db.systemGlobal.nicIgmpModuleIndex),SUCCESS);		
		memset(&devInfo, 0 , sizeof(rtl_multicastDeviceInfo_t));
		strcpy(devInfo.devName, RG_IGMP_SNOOPING_MODULE_NAME);
		devInfo.portMask	= (1<<RTK_RG_MAC_PORT_MAX)-1;
		devInfo.swPortMask	= 0;		
		ASSERT_EQ(rtl_setIgmpSnoopingModuleDevInfo(rg_db.systemGlobal.nicIgmpModuleIndex, &devInfo),SUCCESS);	
	}
#endif


#ifdef __KERNEL__	
#ifdef CONFIG_RTL8686NIC
/*
	// init DirectTx Descs
	memset(&rg_kernel.txDesc,0,sizeof(struct tx_info));
	memset(&rg_kernel.txDescMask,0,sizeof(struct tx_info));
	rg_kernel.txDescMask.opts2.bit.cputag=1;
	rg_kernel.txDesc.opts2.bit.cputag=1;

	rg_kernel.txDescMask.opts3.bit.l34_keep=1;
	rg_kernel.txDescMask.opts3.bit.tx_portmask=0x3f;
	rg_kernel.txDesc.opts3.bit.l34_keep=0;
	rg_kernel.txDesc.opts3.bit.tx_portmask=0;		//hardware auto look up
	//rg_kernel.txDesc.opts3.bit.tx_portmask=(1<<RTK_RG_MAC_PORT0)|(1<<RTK_RG_MAC_PORT_RGMII);

	rg_kernel.txDescMask.opts1.bit.ipcs=1;
	rg_kernel.txDescMask.opts1.bit.l4cs=1;	
	rg_kernel.txDescMask.opts1.bit.cputag_ipcs=1;
	rg_kernel.txDescMask.opts1.bit.cputag_l4cs=1;
	rg_kernel.txDesc.opts1.bit.ipcs=1;
	rg_kernel.txDesc.opts1.bit.l4cs=1;
	rg_kernel.txDesc.opts1.bit.cputag_ipcs=1;
	rg_kernel.txDesc.opts1.bit.cputag_l4cs=1;
*/
	/*register NIC rx handler*/
	err = drv_nic_register_rxhook(0x7f,RE8686_RXPRI_RG,fwdEngine_rx_skb);
	if(err < 0)
		return RG_RET_FAIL;
#endif

	if(rg_kernel.proc_rg==NULL)
		rg_kernel.proc_rg = proc_mkdir("rg", NULL); 


#ifdef CONFIG_RG_WMUX_SUPPORT
	err=wmux_drv_init();
#endif


	for( i=0; i< (sizeof(fwdEngineProc)/sizeof(rtk_rg_proc_t)) ;i++)
	{
		if(fwdEngineProc[i].get==NULL)
			fwdEngineProc[i].proc_fops.open=NULL_fwdEngine_single_open;
		else
			fwdEngineProc[i].proc_fops.open=common_fwdEngine_single_open;
		
		if(fwdEngineProc[i].set==NULL)
			fwdEngineProc[i].proc_fops.write=NULL;
		else
			fwdEngineProc[i].proc_fops.write=common_fwdEngine_single_write;
		
		fwdEngineProc[i].proc_fops.read=seq_read;
		fwdEngineProc[i].proc_fops.llseek=seq_lseek;
		fwdEngineProc[i].proc_fops.release=single_release;

		
		p = proc_create_data(fwdEngineProc[i].name, 0644, rg_kernel.proc_rg , &(fwdEngineProc[i].proc_fops),NULL);
		if(!p){
			printk("create proc rg/%s failed!\n",fwdEngineProc[i].name);
		}
		fwdEngineProc[i].inode_id = p->low_ino;
	}


#if 0//def CONFIG_RG_LAYER2_SOFTWARE_LEARN
	p = proc_create_data("saLearningCount", 0644, rg_kernel.proc_rg,NULL,NULL);
	if (p){
		p->read_proc = (void *)_rtk_rg_layer2LutLearningCountShow;
	}else{
		printk("create proc rg/saLearningCount failed!\n");
	}
#endif
#endif
#ifdef CONFIG_DUALBAND_CONCURRENT
	_rtk_rg_str2mac(CONFIG_DEFAULT_MASTER_IPC_MAC_ADDRESS,&master_ipc_macAddr);
	//_rtk_rg_str2mac(CONFIG_DEFAULT_SLAVE_IPC_MAC_ADDRESS,&slave_ipc_macAddr);
#endif



	return RG_RET_SUCCESS;
}

#ifdef __KERNEL__
/*cp is the NIC private data*/

//FIXME:since _re_dev_private defined in re8686.c, and we just need re_private's pointer which at the first field,
//we just define another similar structure here.
//20140502LUKE:we need txPortMask to decide which port for sending packet from protocol stack, so we copy all
//fields from _re_dev_private defined in re8686.c.
struct _rg_re_dev_private {
	struct re_private* pCp;
	struct net_device_stats net_stats;
	unsigned char txPortMask;
};


/*romedriver 865x Boyce 2014-07-10*/
#if defined(CONFIG_XDSL_ROMEDRIVER)
extern int re865x_start_xmit(struct sk_buff *skb, struct net_device *dev);
#elif defined (CONFIG_APOLLO)
extern int re8670_start_xmit (struct sk_buff *skb, struct net_device *dev);
#endif

#ifdef CONFIG_RG_IP_UNNUMBER
static inline int should_bypass_fwdEngine(struct sk_buff* skb)
{
	//ip unnubmer can only be process by protocol stack, so don't deliver such packet to romedriver
	//I will mark such packet with mark 0x00F00000/0x00F00000
	return (((skb->mark & 0x00F00000) == 0x00F00000) || ((skb->mark & 0x000F0000) == 0x00010000));
}
#endif

//for GPON stream ID remarking or Simple Protocol Stack implementation
//extern int re8670_start_xmit_txInfo (struct sk_buff *skb, struct net_device *dev, struct tx_info* ptxInfo, struct tx_info* ptxInfoMask);
int rtk_rg_fwdEngine_xmit (struct sk_buff *skb, struct net_device *dev)
{
	int rg_fwdengine_ret_code=0;
	rg_db.pktHdr->skb=skb;	//incase trace_filter need skb for comparing
#if 0
	//from Protocol Stack, to nas0_x packets will carry vlan_tci if they want to tag
	//to eth0 packets will carry ctag in skb directly by vconfig
	if(skb->vlan_tci)
	{
		//TAG packet to WAN
		rg_kernel.rxInfoFromPS.opts2.bit.ctagva=1;
		rg_kernel.rxInfoFromPS.opts2.bit.cvlan_tag=((skb->vlan_tci&0xf00)>>8);
		rg_kernel.rxInfoFromPS.opts2.bit.cvlan_tag|=((skb->vlan_tci&0xff)<<8);
	}
	else
	{
		//"unTAG packet to WAN" or "packet to LAN(tag or untag)"
		rg_kernel.rxInfoFromPS.opts2.bit.ctagva=0;
		rg_kernel.rxInfoFromPS.opts2.bit.cvlan_tag=0;
	}
#endif
	//if protocol stack want to send packets during initial state, just send it bypass fwdEngine!!
	//if hwnat state is RG_HWNAT_PROTOCOL_STACK, just send it bypass fwdEngine!!20140120LUKE
	//using re8686_send_with_txInfo will force skb->dev to eth0!! so use re8670_start_xmit here!!20140226LUKE

#ifdef CONFIG_RG_IP_UNNUMBER
	if (should_bypass_fwdEngine(skb)) {
		//printk("go shortcut.\n");
		return re8670_start_xmit(skb,dev);
	}
#endif

	if(_rtk_rg_get_initState()==RTK_RG_DURING_INIT || rg_db.systemGlobal.hwnat_enable==RG_HWNAT_PROTOCOL_STACK)
#if	defined (CONFIG_APOLLO)
		return re8670_start_xmit(skb,dev);
#elif defined(CONFIG_XDSL_ROMEDRIVER)
		return re865x_start_xmit(skb,dev);
#endif

#if defined(CONFIG_APOLLO)
	TRACE("PS_TX_O[%x]: From %s, txPmsk: 0x%x",(unsigned int)skb&0xffff,dev->name,((struct _rg_re_dev_private*)dev->priv)->txPortMask);
	rg_kernel.protocolStackTxPortMask=((struct _rg_re_dev_private*)dev->priv)->txPortMask;	
#elif defined(CONFIG_XDSL_NEW_HWNAT_DRIVER)
#if defined(CONFIG_OPENWRT_RG)
	TRACE("PS_TX_O[%x]: From %s, txPmsk: 0x%x ",(unsigned int)skb&0xffff,dev->name,((struct dev_priv*)netdev_priv(dev))->portmask);
	rg_kernel.protocolStackTxPortMask=((struct dev_priv*)netdev_priv(dev))->portmask;	
#else
//	printk("PS_TX_O[%p]: From %s, txPmsk: 0x%x \n",(void*)skb,dev->name,((struct dev_priv*)dev->priv)->portmask);
	TRACE("PS_TX_O[%x]: From %s, txPmsk: 0x%x ",(unsigned int)skb&0xffff,dev->name,((struct dev_priv*)dev->priv)->portmask);
	rg_kernel.protocolStackTxPortMask=((struct dev_priv*)dev->priv)->portmask;	
#endif
#endif
#if defined(CONFIG_OPENWRT_RG)
	rg_fwdengine_ret_code = rtk_rg_fwdEngineInput(((struct _rg_re_dev_private*)netdev_priv(dev))->pCp,skb,(void*)&rg_kernel.rxInfoFromPS);
#else
	rg_fwdengine_ret_code = rtk_rg_fwdEngineInput(((struct _rg_re_dev_private*)dev->priv)->pCp,skb,(void*)&rg_kernel.rxInfoFromPS);
#endif
	
	//Processing packets
	if(rg_fwdengine_ret_code == RG_FWDENGINE_RET_TO_PS)
	{
		//FIXME:iPhone 5 change wireless connection from master to slave will send strange unicast ARP request for LAN gateway IP, and forwarded by protocol stack
		TRACE("PS_TX_O[%x]: To Protocol-Stack...FREE SKB!!",(unsigned int)skb&0xffff);
		//dump_packet(skb->data,skb->len,"dump_back_to_PS");
		_rtk_rg_dev_kfree_skb_any(skb);
	}
	else if (rg_fwdengine_ret_code == RG_FWDENGINE_RET_DROP)
	{
		TRACE("PS_TX_O[%x]: Drop...FREE SKB!!",(unsigned int)skb&0xffff);
		_rtk_rg_dev_kfree_skb_any(skb);
	}
	/*else
	{
		TRACE("PS_TX_O[%x]: Forward",(unsigned int)skb&0xffff);
	}*/

	return 0;
#if 0
#if defined(CONFIG_GPON_FEATURE)||defined(CONFIG_RG_SIMPLE_PROTOCOL_STACK)
	unsigned char interfaceIdx;
#endif

#ifdef CONFIG_RG_SIMPLE_PROTOCOL_STACK	
	char *startCP,*endCP;
	unsigned int internalVlanID;
	int ret;
#endif

#ifdef __KERNEL__
#ifdef CONFIG_RTL8686NIC
	struct tx_info txDesc,txDescMask;
#else
	rtk_rg_txdesc_t txDesc,txDescMask;
#endif
#endif
	
	memset(&txDesc, 0, sizeof(txDesc));
	memset(&txDescMask, 0, sizeof(txDescMask));

//FIXME:GPON_TX SHOULD CHECK ACL FOR STREAMID
#ifdef CONFIG_GPON_FEATURE
	//GPON WAN should add streamID by tx descriptor here
	if(rg_db.systemGlobal.initParam.wanPortGponMode)
	{
		unsigned int cvid, cpri, sid;
		cvid=rg_kernel.txDesc.opts2.bit.vidl|(rg_kernel.txDesc.opts2.bit.vidh<<8);
		cpri=rg_kernel.txDesc.opts2.bit.prio;
		if(_rtk_rg_cvidCpri2Sidmapping_get(cvid,cpri,&sid)==RT_ERR_RG_OK)
		{
			rg_kernel.txDescMask.opts1.bit.cputag_psel=1;
			rg_kernel.txDescMask.opts3.bit.tx_dst_stream_id=0x7f;
			rg_kernel.txDesc.opts1.bit.cputag_psel=1;
			rg_kernel.txDesc.opts3.bit.tx_dst_stream_id=sid;
		}
	}

	//Handle streamID remarking for GPON
	/*if(rg_db.systemGlobal.initParam.wanPortGponMode && (skb->dev->priv_flags & IFF_DOMAIN_WAN))
	{
		for(interfaceIdx=0;interfaceIdx<rg_db.systemGlobal.wanIntfTotalNum;interfaceIdx++)
		{
			if(!memcmp(rg_db.systemGlobal.wanIntfGroup[interfaceIdx].p_wanIntfConf->gmac.octet,skb->data+6,ETHER_ADDR_LEN))		//sourceMAC == gwMAC
			{
				txDescMask.opts1.bit.cputag_psel=1;
				txDesc.opts1.bit.cputag_psel=1;
				txDescMask.opts3.bit.tx_dst_stream_id=0x7f;
				txDesc.opts3.bit.tx_dst_stream_id=rg_db.systemGlobal.interfaceInfo[rg_db.systemGlobal.wanIntfGroup[interfaceIdx].index].storedInfo.wan_intf.wan_intf_conf.gponStreamID;
				break;
			}
		}
	}*/
#endif

#ifdef CONFIG_RG_SIMPLE_PROTOCOL_STACK
	//Handle broadcast or multicast packet
	if(rg_db.systemGlobal.ctrlPathByProtocolStack_broadcast==1)
		goto BYPASS;

	if(((*(unsigned short *)skb->data)&(*(unsigned short *)(skb->data+1))&(*(unsigned short *)(skb->data+2)))==0xffff ||	//broadcast
		(skb->data[0]==0x01&&skb->data[1]==0x00&&skb->data[2]==0x5e) ||		//ipv4 multicast
		(*(unsigned short *)skb->data==0x3333))	 //ipv6 multicast
	{
		//Check interface
		startCP=dev->name+3;		//bypass "eth" characters
		interfaceIdx = simple_strtoul(startCP,&endCP,0);
		//Get VLAN setting for member set and untag set
		internalVlanID=rg_db.netif[interfaceIdx].rtk_netif.vlan_id;
		//Allocate skb for tagged and untagged set, if needed
		//DEBUG("dev name is %s, interface index is %d,vlanID is %d",dev->name,interfaceIdx,internalVlanID);
		//dump_packet(skb->data,skb->len,"SPS xmit packet");
		ret=_rtk_rg_broadcastForward(skb,internalVlanID,RTK_RG_MAC_PORT_CPU,0);	
		if(ret==RG_FWDENGINE_RET_DROP)
			_rtk_rg_dev_kfree_skb_any(skb);

		return 0;
	}
	else	//unicast packet transmit by hardware lookup
	{
		//DEBUG("unicast packet, just forward and let hardware do the job");
		/*if(skb->data[0]==0x68 &&
			skb->data[1]==0x05&&
			skb->data[2]==0xCA&&
			skb->data[3]==0x0F&&
			skb->data[4]==0x8B&&
			skb->data[5]==0x73)
			dump_packet(skb->data,skb->len,"send to server!!!");*/
		//return re8670_start_xmit(skb,dev);
	}
#endif
//BYPASS:

	return re8670_start_xmit_txInfo(skb,dev,&txDesc,&txDescMask);
#endif
}

#if defined(CONFIG_APOLLO_GPON_FPGATEST)

#if defined(CONFIG_RTL9602C_SERIES)
extern int32 rtl9602c_virtualMac_Input(rtk_port_t fromPort, uint8 *pPkt, uint32 len);
extern int32 rtl9602c_virtualMac_Output(rtk_port_t toPort, uint8 *pPkt, uint32 *plen);
#endif

/*copy from packetGen Boyce 2015-08-13*/
#if 1 
#define CRC(crc, ch)	 (crc = (crc >> 8) ^ crctab[(crc ^ (ch)) & 0xff])

static uint32 crctab[256] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

static void pktForm_crc32(int8 * buf, uint32 len, uint32 * cval){ 

	uint32 crc = ~0;
    int8 *p ;
	
	for(p = buf; len--; ++p)
		CRC(crc, *p) ;

    *cval = ~crc ;
}

/* payloadSize not include crc */
static void l2FormCrc(uint32 flag, int8 * pktBuf, uint32 payloadSize) {
	uint32 crc, i;
	/* Ethernet assume a packet will longer than 60 bytes, therefore, 
	 * we must pad packet content first if the length is smaller than 60 bytes.
	 * Then, we append L2 CRC in the last 4 bytes. */
	if ( payloadSize < 60 )
	{
		memset(&pktBuf[payloadSize], 0x00/* backward compatible for drvTest*/, (60 - payloadSize));
		payloadSize = 60;
	}
	pktForm_crc32(pktBuf,payloadSize,&crc);

	for(i=0;i<4;i++){
		pktBuf[payloadSize+i] = (crc>>(i*8)) & 0xff; /* New version, reversed again */
		}
}
#endif

int32 _rtk_rg_PON_virtualMac_Input(rtk_port_t fromPort, uint8 *pPkt, uint32 len)
{
	struct sk_buff *skb;
	struct tx_info ptxInfo={{{0}},0,{{0}},{{0}}};
	struct tx_info ptxInfoMask={{{0}},0,{{0}},{{0}}};
	//if we are sending packet to PON, we should call NIC directTX to send it out!!
	if(rg_db.systemGlobal.virtualMAC_with_PON_switch_mask.portmask && fromPort==RTK_RG_MAC_PORT_PON){
		if(!pPkt)return RT_ERR_NULL_POINTER;
		if(len > SKB_BUF_SIZE)return RT_ERR_ENTRY_FULL;
		rg_db.pktHdr->ingressPort=RTK_RG_MAC_PORT_PON;
		skb=re8670_getAlloc(SKB_BUF_SIZE);
		if(skb==NULL) return RT_ERR_NOT_ALLOWED;
		WARNING("copy data to skb[%x], len=%d",(unsigned int)skb&0xffff,len);
		memcpy(skb->data,pPkt,len);
		skb->dev = nicRootDev;
		/* subtract CRC Boyce 2015-08-13 */
		skb->len=len-4;
		ptxInfo.opts3.bit.tx_portmask=rg_db.systemGlobal.virtualMAC_with_PON_switch_mask.portmask;
		ptxInfoMask.opts3.bit.tx_portmask=0x3f;
		//WARNING("before send out to NIC!!");
		return re8686_send_with_txInfo_and_mask(skb, &ptxInfo, 0, &ptxInfoMask);
	}else{
#if defined(CONFIG_RTL9602C_SERIES)	
		return rtl9602c_virtualMac_Input(fromPort, pPkt, len);
#else
		return 0;
#endif
	}
}

int32 _rtk_rg_PON_virtualMac_Output(rtk_port_t toPort, uint8 *pPkt, uint32 *plen)
{
	//if we are receiving packet from PON, we should store it to struct list_head vmacSkbListHead
	//And when we are need for virtualMAC output, we choose the every first skb from the queue.
	mdelay(500);
	if(rg_db.systemGlobal.virtualMAC_with_PON_switch_mask.portmask && toPort==RTK_RG_MAC_PORT_PON){
		if(!pPkt || !plen)return RT_ERR_NULL_POINTER;
		if(!list_empty(&rg_db.vmacSkbListHead)){
			rtk_rg_vmac_skb_linlList_t *pVmacEntry,*pNextEntry;
			list_for_each_entry_safe(pVmacEntry,pNextEntry,&rg_db.vmacSkbListHead,vmac_list){
				//Delete from head list
				list_del_init(&pVmacEntry->vmac_list);
	
				//copy the skb data and len
				*plen=pVmacEntry->skb->len;
				memcpy(pPkt,pVmacEntry->skb->data,*plen);
				WARNING("copy skb[%x] len=%d",(unsigned int)pVmacEntry->skb&0xffff,pVmacEntry->skb->len);
				/*calculate CRC Boyce 2015-08-13*/
				l2FormCrc(0,pPkt,*plen);
				*plen+=4;
				dev_kfree_skb_any(pVmacEntry->skb);
				pVmacEntry->skb=NULL;

				//Add back to free list
				list_add(&pVmacEntry->vmac_list,&rg_db.vmacSkbListFreeListHead);
				return RT_ERR_OK;
			}
		}
		*plen=0x0;	//no packet queued
		return RT_ERR_ENTRY_NOTFOUND;
	}else{
#if defined(CONFIG_RTL9602C_SERIES)	
		return rtl9602c_virtualMac_Output(toPort, pPkt, plen);
#else
		return 0;
#endif
	}
}

#endif  //end if CONFIG_APOLLO

int __init rtk_rg_rome_driver_module_init(void)
{
	rtk_rg_rome_driver_init();
	return 0;
}


void __exit rtk_rg_rome_driver_module_exit(void)
{
	int i;
	//rtlglue_printf("%s\n",__func__);

	//Free all queue packet if any
	for(i=0;i<MAX_IPV4_FRAGMENT_QUEUE_SIZE;i++)
	{
		_rtk_rg_dev_kfree_skb_any(rg_db.ipv4FragmentQueue[i].queue_skb);
	}
	
#ifdef	CONFIG_RG_IPV6_STATEFUL_ROUTING_SUPPORT
	//Free all queue packet if any
	for(i=0;i<MAX_IPV6_FRAGMENT_QUEUE_SIZE;i++)
	{
		_rtk_rg_dev_kfree_skb_any(rg_db.ipv6FragmentQueue[i].queue_skb);
	}
#endif

	/*unregister NIC rx handler*/
#if defined(CONFIG_APOLLO)
	drv_nic_unregister_rxhook(0x7f,RE8686_RXPRI_RG,fwdEngine_rx_skb);
#endif
#ifdef CONFIG_RG_WMUX_SUPPORT
	wmux_drv_exit();
#endif
}

late_initcall(rtk_rg_rome_driver_module_init);		//have to be called after than liteRomeDriver's init function
module_exit(rtk_rg_rome_driver_module_exit);
#endif
#endif //CONFIG_APOLLO_ROMEDRIVER


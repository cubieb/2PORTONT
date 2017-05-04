/*
* ----------------------------------------------------------------
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* $Header: /usr/local/dslrepos/linux-2.6.30/drivers/net/rtl819x/rtl865xc_swNic.c,v 1.21 2012/05/23 03:31:42 czpinging Exp $
*
* Abstract: Switch core polling mode NIC driver source code.
*
* $Author: czpinging $
*
* $Log: rtl865xc_swNic.c,v $
* Revision 1.21  2012/05/23 03:31:42  czpinging
* fix compiler error
*
* Revision 1.20  2012/05/22 04:07:46  czpinging
* refill priority in descriptor
*
* Revision 1.19  2012/05/21 08:47:44  czpinging
* refill priority in descriptork
*
* Revision 1.18  2012/04/23 09:19:27  ikevin362
* skb_debug when enable dump_swNicTxRx_pkt
*
* Revision 1.17  2012/03/09 13:28:23  kaohj
* invalidate cache before DMA
*
* Revision 1.16  2012/02/21 08:49:53  tylo
* add debug by Kevin
*
* Revision 1.14  2011/12/09 08:37:56  cathy
* enable debug for rx and remove trap_by_ingress_acl
*
* Revision 1.13  2011/11/17 07:47:26  czpinging
* (Redef)8367b
*
* Revision 1.12  2011/11/17 03:37:34  czpinging
* Add RL6000 testing function
*
* Revision 1.11  2011/11/16 14:09:09  czpinging
* Add RL6000 testing function
*
* Revision 1.10  2011/11/04 10:44:09  kaohj
* Fix compile error, use DELAY_REFILL_ETH_RX_BUF
*
* Revision 1.9  2011/07/29 14:18:35  ql
* nothing change
*
* Revision 1.8  2011/07/25 07:01:29  ql
* add debug info.
*
* Revision 1.7  2011/07/22 08:01:15  ikevin362
* if the pkt is only toward to extPort, reset vid to a magic number
*
* Revision 1.6  2011/07/11 09:04:56  cathy
* avoid free the same skb in swNic_txDone
*
* Revision 1.5  2011/06/27 14:12:45  ikevin362
* 1. print the reason when trap to cpu  2.mark the packet which is trapped by ingress acl rule
*
* Revision 1.4  2011/06/20 12:07:05  tylo
* fix unknown unicast rx issue
*
* Revision 1.3  2011/06/13 10:27:49  ikevin362
* add trap2cpu pkt debug
*
* Revision 1.2  2011/04/11 12:45:18  tylo
* update hw nat driver from AP team
*
* Revision 1.11  2008/04/11 10:49:14  bo_zhao
* * restore the original cache flush
*
* Revision 1.10  2008/04/11 10:12:38  bo_zhao
* *: swap nic drive to 8186 style
*
* Revision 1.6  2008/02/22 05:31:52  joeylin
* set one VLAN group for Bridge/WISP mode, and fix the issue:
* WAN port PC can not ping br0 (192.168.1.254) in Bridge/WISP mode
*
* Revision 1.5  2008/02/15 09:52:46  forrest
* 1. Add hardware accelerated PPTP processing. 2. Fine tune some hardware NAT to be compatible to hardware accelerated PPTP.
*
* Revision 1.4  2007/12/08 08:24:26  davidhsu
* Adjust tx desc size. Hide error message
*
* Revision 1.3  2007/12/04 12:00:18  joeylin
* add hardware NAT feature
*
* Revision 1.2  2007/11/11 02:51:24  davidhsu
* Fix the bug that do not fre rx skb in rx descriptor when driver is shutdown
*
* Revision 1.1.1.1  2007/08/06 10:04:52  root
* Initial import source to CVS
*
* Revision 1.11  2007/03/27 12:51:07  michaelhuang
* +: add function swNic_send_portmbr for FT2
*
*
*
* ---------------------------------------------------------------
*/

#include <net/rtl/rtl_types.h>
#include <net/rtl/rtl_glue.h>
#include "common/rtl_errno.h"
#include "AsicDriver/asicRegs.h"
#include "rtl865xc_swNic.h"
#include "common/mbuf.h"
#include "AsicDriver/rtl865x_asicCom.h"
#include "AsicDriver/rtl865x_asicL2.h"

#include <linux/skbuff.h>
#include "AsicDriver/rtl865xC_hs.h"
#ifdef	CONFIG_RTL865X_ROMEPERF
#include "romeperf.h"
#endif
#include <linux/netdevice.h>

int DumpTrapCPUpkt_debug = 0;
int DumpSwNicTxRx_debug = 0;

extern void (*_dma_cache_wback_inv)(unsigned long start, unsigned long size);
extern void tx_done_callback(void *skb);

/* RX Ring */
static uint32*  rxPkthdrRing[RTL865X_SWNIC_RXRING_HW_PKTDESC];                 /* Point to the starting address of RX pkt Hdr Ring */
__DRAM_FWD static uint32   rxPkthdrRingCnt[RTL865X_SWNIC_RXRING_HW_PKTDESC];              /* Total pkt count for each Rx descriptor Ring */
__DRAM_FWD static uint32   rxPkthdrRefillThreshold[RTL865X_SWNIC_RXRING_HW_PKTDESC];              /* Ether refill threshold for each Rx descriptor Ring */

/* TX Ring */
static uint32*  txPkthdrRing[RTL865X_SWNIC_TXRING_HW_PKTDESC];             /* Point to the starting address of TX pkt Hdr Ring */

#if defined(CONFIG_RTL8196C_REVISION_B)
__DRAM_FWD static uint32	rtl_chip_version;
static uint32*  txPkthdrRing_backup[RTL865X_SWNIC_TXRING_HW_PKTDESC];             /* Point to the starting address of TX pkt Hdr Ring */
#endif

__DRAM_FWD static uint32   txPkthdrRingCnt[RTL865X_SWNIC_TXRING_HW_PKTDESC];          /* Total pkt count for each Tx descriptor Ring */

#define txPktHdrRingFull(idx)   (((txPkthdrRingFreeIndex[idx] + 1) & (txPkthdrRingMaxIndex[idx])) == (txPkthdrRingDoneIndex[idx]))

/* Mbuf */
uint32* rxMbufRing;                                                     /* Point to the starting address of MBUF Ring */
__DRAM_FWD uint32  rxMbufRingCnt;                                                  /* Total MBUF count */

__DRAM_FWD static uint32  size_of_cluster;

/* descriptor ring tracing pointers */
__DRAM_FWD static int32   currRxPkthdrDescIndex[RTL865X_SWNIC_RXRING_HW_PKTDESC];      /* Rx pkthdr descriptor to be handled by CPU */
__DRAM_FWD static int32   currRxMbufDescIndex;        /* Rx mbuf descriptor to be handled by CPU */

__DRAM_FWD static int32   currTxPkthdrDescIndex[RTL865X_SWNIC_TXRING_HW_PKTDESC];      /* Tx pkthdr descriptor to be handled by CPU */
__DRAM_FWD static int32 txPktDoneDescIndex[RTL865X_SWNIC_TXRING_HW_PKTDESC];

/* debug counters */
__DRAM_FWD static int32   rxPktCounter;
__DRAM_FWD static int32   txPktCounter;

#ifdef DELAY_REFILL_ETH_RX_BUF
__DRAM_FWD static int32   rxDescReadyForHwIndex[RTL865X_SWNIC_RXRING_HW_PKTDESC];
#endif

__DRAM_FWD static uint8 extPortMaskToPortNum[_RTL865XB_EXTPORTMASKS+1] =
{
	0, 1, 2, 0, 3, 0, 0, 0
};

static void skb_debug(const char* data)
{
#define NUM2PRINT 100
	int i;
	for (i=0; i<NUM2PRINT; i++) 
	{
		printk("%02X  ",data[i]&0xFF);
		if(i%16==15)
			printk("\n");
		else if(i%8==7)
			printk("  ");		
	}
	printk("\n");
}

/*************************************************************************
*   FUNCTION                                                              
*       swNic_intHandler                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function is the handler of NIC interrupts
*                                                                         
*   INPUTS                                                                
*       intPending      Pending interrupt sources.
*                                                                         
*   OUTPUTS                                                               
*       None
*************************************************************************/
void swNic_intHandler(uint32 intPending) {return;}
__MIPS16
__IRAM_FWD inline int32 rtl8651_rxPktPreprocess(void *pkt, unsigned int *vid)
{
	struct rtl_pktHdr *m_pkthdr = (struct rtl_pktHdr *)pkt;
	uint32 srcPortNum;
	srcPortNum = m_pkthdr->ph_portlist;

	/* ph_vlanId:
	 * RX: Destination VLAN ID(after routing).
	 * TX: Source Destination VLAN ID select
	 */
	*vid = m_pkthdr->ph_vlanId;
	#if 0
	if (srcPortNum >= RTL8651_CPU_PORT)
	{        
		if (m_pkthdr->ph_extPortList == 0)
		{
			/* No any destination ( extension port or CPU) : ASIC's BUG */
			return FAILED;
		}else if ((m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) == 0)// to extension port
		{
			/*
				if dest Ext port 0x1 => to dst ext port 1 => from src port 1+5=6
				if dest Ext port 0x2 => to dst ext port 2 => from src port 2+5=7
				if dest Ext port 0x4 => to dst ext port 3 => from src port 3+5=8
			*/
			srcPortNum = extPortMaskToPortNum[m_pkthdr->ph_extPortList]+RTL8651_PORT_NUMBER-1;
			m_pkthdr->ph_portlist = srcPortNum;//now ph_portlist assign tx destinatino port mask
#if	defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
			*vid = PKTHDR_EXTPORT_MAGIC;
#endif
		}else//to cpu port
		{
			/* has CPU bit, pkt is original pkt from port 6~8 */
			srcPortNum = m_pkthdr->ph_srcExtPortNum + RTL8651_PORT_NUMBER - 1;
			m_pkthdr->ph_portlist = srcPortNum;
#if	defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
			*vid = PKTHDR_EXTPORT_MAGIC2;
#endif
		}        
	}
       else
	{
#ifndef CONFIG_RTL_8676HWNAT
		/* otherwise, pkt is rcvd from PHY */
		m_pkthdr->ph_srcExtPortNum = 0;
		if((m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) == 0)//to extension port or phy
		{	/* No CPU bit, only dest ext mbr port... */
			/*
				if dest Ext port 0x1 => to dst ext port 1 => from src port 1+5=6
				if dest Ext port 0x2 => to dst ext port 2 => from src port 2+5=7
				if dest Ext port 0x4 => to dst ext port 3 => from src port 3+5=8
			*/
			if(m_pkthdr->ph_extPortList&&0!=extPortMaskToPortNum[m_pkthdr->ph_extPortList])
			{
				/* redefine src port number */
				srcPortNum = extPortMaskToPortNum[m_pkthdr->ph_extPortList] + RTL8651_PORT_NUMBER - 1;
				m_pkthdr->ph_portlist = srcPortNum;
				#if	defined(CONFIG_RTL_HARDWARE_NAT)&&(defined(CONFIG_RTL8192SE)||defined(CONFIG_RTL8192CD))
				*vid = PKTHDR_EXTPORT_MAGIC;
				#endif
			}
		}
#endif
	}		
	#endif
	//Kevin, if the pkt is only toward to extPort, reset vid to a magic number
	if (srcPortNum < RTL8651_CPU_PORT)
	{
		/* No CPU bit, only dest ext mbr port... */
		if( !(m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) && (m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_P0))
			*vid = PKTHDR_EXTPORT_MAGIC;	

		if( !(m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) && (m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_P1))
			*vid = PKTHDR_EXTPORT_MAGIC2;	

		if( !(m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_CPU) && (m_pkthdr->ph_extPortList & PKTHDR_EXTPORTMASK_P2))
			*vid = PKTHDR_EXTPORT_MAGIC3;		

		return SUCCESS;
	}
	else 
		return FAILED;	
}

#ifdef DELAY_REFILL_ETH_RX_BUF
inline int return_to_rxing_check(int ringIdx)
{
	int ret;
	unsigned long flags;
	local_irq_save(flags);
	ret = ((rxPkthdrRingCnt[ringIdx]!=0) && (rxDescReadyForHwIndex[ringIdx] != currRxPkthdrDescIndex[ringIdx]))? 1:0;
	local_irq_restore(flags);
	return ret;	
}
inline int buffer_reuse(int ringIdx) 
{
	int index1,index2,gap;
	unsigned long flags;
	local_irq_save(flags);
	index1 = rxDescReadyForHwIndex[ringIdx];
	index2 = currRxPkthdrDescIndex[ringIdx]+1;
	gap = (index2 > index1) ? (index2 - index1) : (index2 + rxPkthdrRingCnt[ringIdx] - index1);
	
	if ((rxPkthdrRingCnt[ringIdx] - gap) < (rxPkthdrRefillThreshold[ringIdx]))
	{
		local_irq_restore(flags);
		return 1;
	}
	else
	{
		local_irq_restore(flags);
		return 0;
	}
}

inline void set_RxPkthdrRing_OwnBit(uint32 rxRingIdx)
{
	rxPkthdrRing[rxRingIdx][rxDescReadyForHwIndex[rxRingIdx]] |= DESC_SWCORE_OWNED;
	
	if ( ++rxDescReadyForHwIndex[rxRingIdx] == rxPkthdrRingCnt[rxRingIdx] )
		rxDescReadyForHwIndex[rxRingIdx] = 0;
}

__IRAM_FWD static void release_pkthdr(struct sk_buff  *skb, int idx)
{
	struct rtl_pktHdr *pReadyForHw;
	uint32 mbufIndex;
	unsigned long flags;

	_dma_cache_wback_inv((unsigned long)skb->head, skb->truesize);
	local_irq_save(flags);
	pReadyForHw = (struct rtl_pktHdr *)(rxPkthdrRing[idx][rxDescReadyForHwIndex[idx]] & 
						~(DESC_OWNED_BIT | DESC_WRAP));
	mbufIndex = ((uint32)(pReadyForHw->ph_mbuf) - (rxMbufRing[0] & ~(DESC_OWNED_BIT | DESC_WRAP))) /
					(sizeof(struct rtl_mBuf));
	
	pReadyForHw->ph_mbuf->m_data = skb->data;
	pReadyForHw->ph_mbuf->m_extbuf = skb->data;
	pReadyForHw->ph_mbuf->skb = skb;

	rxMbufRing[mbufIndex] |= DESC_SWCORE_OWNED;
	set_RxPkthdrRing_OwnBit(idx);
	local_irq_restore(flags);
}

/*
	return value: 1 ==> success, returned to rx pkt hdr desc
	return value: 0 ==> failed, no return ==> release to priv skb buf pool
 */	
extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);
int return_to_rx_pkthdr_ring(unsigned char *head) 
{
	struct sk_buff *skb;
	int ret, i;
	//unsigned long flags;

	ret=FAILED;

	//local_irq_save(flags);
	for(i = RTL865X_SWNIC_RXRING_MAX_PKTDESC -1; i >= 0; i--)
	{
		if (return_to_rxing_check(i)) {

			skb = dev_alloc_8190_skb(head, CROSS_LAN_MBUF_LEN);
			if (skb == NULL)
				goto _ret1;

			skb_reserve(skb, RX_OFFSET);
			release_pkthdr(skb, i);
			ret = SUCCESS;
			break;
		}
	}

_ret1:
	//local_irq_restore(flags);
	return ret;
}
#else //DELAY_REFILL_ETH_RX_BUF
static void release_pkthdr(struct sk_buff  *skb, int idx)
{}
#endif //DELAY_REFILL_ETH_RX_BUF

/*	It's the caller's responsibility to make sure "rxRingIdx" and 
*	"currRxPktDescIdx" NOT NULL, since the callee never check
*	sanity of the parameters, in order to speed up.
*/
__MIPS16
__IRAM_FWD
static int32 swNic_getRxringIdx(uint32 *rxRingIdx, uint32 *currRxPktDescIdx,uint32 policy)
{
	int32	i;
	int32	priority;

	priority = policy;	
	for(i = RTL865X_SWNIC_RXRING_MAX_PKTDESC -1; i >= priority; i--)
	{
		if(rxPkthdrRingCnt[i] == 0)
			continue;
		
		if((rxPkthdrRing[i][currRxPkthdrDescIndex[i]] & DESC_OWNED_BIT) == DESC_RISC_OWNED)
		{
			*rxRingIdx = i;
			*currRxPktDescIdx = currRxPkthdrDescIndex[i];			
			return SUCCESS;
		}
	}

	return FAILED;
}

__IRAM_FWD
int swNic_increaseRxIdx(int rxRingIdx)
{
	unsigned long	flags;
	//int32		nextIdx;

	local_irq_save(flags);
        if ( ++currRxPkthdrDescIndex[rxRingIdx] == rxPkthdrRingCnt[rxRingIdx] )
            currRxPkthdrDescIndex[rxRingIdx] = 0;

	#if 0
	if (currRxPkthdrDescIndex[rxRingIdx]+1 == rxPkthdrRingCnt[rxRingIdx])
		nextIdx = 0;
	else
		nextIdx = currRxPkthdrDescIndex[rxRingIdx]+1;
	#endif
	local_irq_restore(flags);

	return SUCCESS;
}

#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
int get_nic_txRing_buf(void)
{
	int txCnt = 0;
	int i,j;
	struct rtl_pktHdr *pPkthdr;
	for(i = RTL865X_SWNIC_TXRING_MAX_PKTDESC -1; i >= 0; i--)
	{
		if(txPkthdrRingCnt[i] == 0)
			continue;

		for(j = 0; j <txPkthdrRingCnt[i]; j++)
		{
			
			pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[i][j]& ~(DESC_OWNED_BIT | DESC_WRAP));

			if(pPkthdr->ph_mbuf->skb)
			{
				if(is_rtl865x_eth_priv_buf(((struct sk_buff *)pPkthdr->ph_mbuf->skb)->head))
					txCnt++;
			}
		}		
	}
	
	return txCnt;
}

int get_nic_rxRing_buf(void)
{
	int rxCnt = 0;
	int i,j;
	struct rtl_pktHdr *pPkthdr;
	for(i = RTL865X_SWNIC_RXRING_MAX_PKTDESC -1; i >= 0; i--)
	{
		if(rxPkthdrRingCnt[i] == 0)
			continue;

		for(j = 0; j < rxPkthdrRingCnt[i]; j++)
		{
			{
				pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[i][j] & ~(DESC_OWNED_BIT | DESC_WRAP));
				if(pPkthdr->ph_mbuf->skb)
				{
					if(is_rtl865x_eth_priv_buf(((struct sk_buff *)pPkthdr->ph_mbuf->skb)->head))
						rxCnt++;
				}
			}
		}
	}

	return rxCnt;
}
#endif

int32 swNic_flushRxRingByPriority(int priority)
{
	int32	i;
	struct rtl_pktHdr * pPkthdr;
	void *skb;
	unsigned long flags;

	#if defined(CONFIG_RTL865X_WTDOG)
	REG32(WDTCNR) |=  WDTCLR; /* reset watchdog timer */
	#endif
	local_irq_save(flags);
	for(i = priority -1; i >= 0; i--)
	{
		if(rxPkthdrRingCnt[i] == 0)
			continue;
		
		while((rxPkthdrRing[i][currRxPkthdrDescIndex[i]] & DESC_OWNED_BIT) == DESC_RISC_OWNED)
		{
			pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[i][currRxPkthdrDescIndex[i]] & ~(DESC_OWNED_BIT | DESC_WRAP));
			skb = pPkthdr->ph_mbuf->skb;
			release_pkthdr(skb, i);
			swNic_increaseRxIdx(i);
		}
	}
	local_irq_restore(flags);
	REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
	return SUCCESS;
}


/*************************************************************************
*   FUNCTION                                                              
*       swNic_receive                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function reads one packet from rx descriptors, and return the 
*       previous read one to the switch core. This mechanism is based on 
*       the assumption that packets are read only when the handling 
*       previous read one is done.
*                                                                         
*   INPUTS                                                                
*       None
*                                                                         
*   OUTPUTS                                                               
*       None
*************************************************************************/
//__MIPS16
__IRAM_FWD
int32 swNic_receive(rtl_nicRx_info *info)
{
	/*QL 20110615 start: MTU is 1522, so one mbuf is sufficient to receive a packet. 
	 * don't care the case that consecutive mbuf occupied by one packet.
	 */
	struct rtl_pktHdr * pPkthdr;
	unsigned char *buf;
	void *skb;
	uint32 rxRingIdx;
	uint32 currRxPktDescIdx;
	int32 retval;
	unsigned long flags;
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	uint32	vid;
	#endif
	
	int i;
	char* ptr_pPkthdr_debug;
	if(DumpSwNicTxRx_debug)
		printk("Enter %s\n",__func__);
get_next:	
	 /* Check OWN bit of descriptors */
	 local_irq_save(flags);
	 retval = swNic_getRxringIdx(&rxRingIdx,&currRxPktDescIdx,info->priority);
	 local_irq_restore(flags);

	 /* Check OWN bit of descriptors */
	if (retval == SUCCESS ) 
	{   
		info->priority = rxRingIdx;
		/* Fetch pkthdr */
		pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[rxRingIdx][currRxPktDescIdx] & ~(DESC_OWNED_BIT | DESC_WRAP));    

		/* Increment counter */
		rxPktCounter++;

		/*	checksum error drop it	*/
		if ((pPkthdr->ph_flags & (CSUM_TCPUDP_OK | CSUM_IP_OK)) != (CSUM_TCPUDP_OK | CSUM_IP_OK))
		{
			printk("%s %d checksum error.\n", __func__, __LINE__);
			buf = NULL;
			#ifdef DELAY_REFILL_ETH_RX_BUF
			goto release1;
			#else
			goto release;
			#endif
		}

		if(DumpTrapCPUpkt_debug)
		{
			printk("rxring_idx=%d     priority:%d   from port%d  vid=%d  reason:0x%X (",rxRingIdx,pPkthdr->ph_rxPriority,pPkthdr->ph_portlist,pPkthdr->ph_vlanId,pPkthdr->ph_reason);
			if(pPkthdr->ph_reason >> 9 == 0)
			{
				printk("gerneral purpose");
			}
			else if(pPkthdr->ph_reason >> 9 == 1)
			{
				printk("trap by ingresss acl no. %d",pPkthdr->ph_reason & 0x7f);
			}
			else if(pPkthdr->ph_reason >> 9 == 2)
			{
				printk("trap by egresss acl no. %d",pPkthdr->ph_reason & 0x7f);
			}
			else if(pPkthdr->ph_reason >> 9 == 3)
			{
				printk("protocal parsing failed");
			}
			else
			{
				printk("no reason ?? ");
			}
			printk(")\n");
		}

#if defined(CONFIG_RTL_HARDWARE_NAT)
		if (rtl8651_rxPktPreprocess(pPkthdr, &vid) != SUCCESS)
		{
			buf = NULL;
		}
		else
		{
			buf = alloc_rx_buf(&skb, size_of_cluster);
		}
		info->vid = vid;
#else
		/*
		 * vid is assigned in rtl8651_rxPktPreprocess() 
		 * do not update it when CONFIG_RTL_HARDWARE_NAT is defined
		 */
		info->vid=pPkthdr->ph_vlanId;
		buf = alloc_rx_buf(&skb, size_of_cluster);
#endif
		info->pid=pPkthdr->ph_portlist;
		if(DumpSwNicTxRx_debug)
		{
			ptr_pPkthdr_debug = (char*)pPkthdr;
			printk("(%s)info->vid : %d   info->pid :%d  \n",__func__,info->vid,info->pid);
			printk("(%s)--------pPkthdr-----------------\n",__func__);
			for(i=0;i<20;i++)
				printk("%02X  ",ptr_pPkthdr_debug[i]&0xFF);
			printk("\n(%s)--------packet content -----------------\n",__func__);
			skb_debug(((struct sk_buff*)pPkthdr->ph_mbuf->skb)->data);	
			printk("\n------------------------------------------\n");
		}
		if (buf) 
		{
			info->input = pPkthdr->ph_mbuf->skb;
			info->len = pPkthdr->ph_len - 4;

			#ifdef DELAY_REFILL_ETH_RX_BUF
			release_pkthdr(skb, rxRingIdx);
			#else
			pPkthdr->ph_mbuf->m_data = pPkthdr->ph_mbuf->m_extbuf = buf;
			pPkthdr->ph_mbuf->skb = skb;
			#endif
			REG32(CPUIISR) = (MBUF_DESC_RUNOUT_IP_ALL|PKTHDR_DESC_RUNOUT_IP_ALL);
		}
#ifdef DELAY_REFILL_ETH_RX_BUF
		else if (!buffer_reuse(rxRingIdx)) {
			info->input = pPkthdr->ph_mbuf->skb;
			info->len = pPkthdr->ph_len - 4;
			#if defined(CONFIG_RTL_ETH_PRIV_SKB_DEBUG)
			pPkthdr->ph_mbuf->skb = NULL;
			#endif
			buf = (unsigned char *)info->input; // just only for "if (buf == NULL)" below
		}
		else {
			return RTL_NICRX_REPEAT;
release1:
			skb = pPkthdr->ph_mbuf->skb;
			release_pkthdr(skb, rxRingIdx);
		}		
#else
release:
		rxPkthdrRing[rxRingIdx][currRxPkthdrDescIndex[rxRingIdx]] |= DESC_SWCORE_OWNED;
		rxMbufRing[currRxMbufDescIndex] |= DESC_SWCORE_OWNED;
		if ( ++currRxMbufDescIndex == rxMbufRingCnt )
			currRxMbufDescIndex = 0;
#endif

        /* Increment index */
		if (swNic_increaseRxIdx(rxRingIdx)!=SUCCESS)
			return RTL_NICRX_REPEAT;

		if (buf == NULL)
			goto get_next;

		return RTL_NICRX_OK;
	}
	else
		return RTL_NICRX_NULL;
}

/*************************************************************************
*   FUNCTION                                                              
*       swNic_send                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function writes one packet to tx descriptors, and waits until 
*       the packet is successfully sent.
*                                                                         
*   INPUTS                                                                
*       None
*                                                                         
*   OUTPUTS                                                               
*       None
*************************************************************************/

#ifdef CONFIG_RTL_8367B

/************************************
*	const variable defination
*************************************/
#define	RTL_BridgeWANVLANID		7 /* WAN vid (bridged, default no vlan tag)*/
#define	RTL_WANVLANID			8 /* WAN vid (routed,   default no vlan tag)*/
#define	RTL_LANVLANID			9 /* LAN vid  (default no vlan tag) */
#endif

__MIPS16
__IRAM_FWD  inline int32 _swNic_send(void *skb, void * output, uint32 len,rtl_nicTx_info *nicTx)
{
	struct rtl_pktHdr * pPkthdr;
	char* ptr_pPkthdr_debug;
	int i;	
	int next_index, ret;
	if(DumpSwNicTxRx_debug)
		printk("Enter %s  (nicTx->txIdx:%d)\n",__func__,nicTx->txIdx);

	if ((currTxPkthdrDescIndex[nicTx->txIdx]+1)==txPkthdrRingCnt[nicTx->txIdx])
		next_index = 0;
	else
		next_index = currTxPkthdrDescIndex[nicTx->txIdx]+1;
	
	if (next_index == txPktDoneDescIndex[nicTx->txIdx])	{
		/*	TX ring full	*/
			return -1;
	}
	
	// Kaohj -- invalidate cache before DMA
	_dma_cache_wback_inv((unsigned long) output, len);
	/* Fetch packet header from Tx ring */
	pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[nicTx->txIdx][currTxPkthdrDescIndex[nicTx->txIdx]] 
                                                & ~(DESC_OWNED_BIT | DESC_WRAP));
	ptr_pPkthdr_debug = (char*)pPkthdr;

	/* Pad small packets and add CRC */
	if ( len < 60 )
		len = 64;
	else
		len += 4;

#ifdef CONFIG_RTL_8367B

    if (rtl8651_tblAsicDrvPara.externalPHYProperty & RTL8676_TBLASIC_EXTPHYPROPERTY_PORT0_RTL8367B) {   
    	if ((RTL_WANVLANID != nicTx->vid) && (RTL_LANVLANID != nicTx->vid) && (RTL_BridgeWANVLANID != nicTx->vid)) {
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)    	
    		nicTx->priority=(((char*)output)[14]&0xFF)>>5;	
#endif    		
    		pPkthdr->ph_vlanTagged=1;
    	}
		else
			pPkthdr->ph_vlanTagged=0;
    }
    	//printk("nicTx->priority %x\n",nicTx->priority);
#endif	

	pPkthdr->ph_mbuf->m_len  = len;
	pPkthdr->ph_mbuf->m_extsize = len;
	pPkthdr->ph_mbuf->skb = skb;
	pPkthdr->ph_len = len;
	
	pPkthdr->ph_vlanId = nicTx->vid;
	#if defined (CONFIG_8198_PORT5_GMII)
	pPkthdr->ph_portlist = nicTx->portlist&0x3f;
	#else
	pPkthdr->ph_portlist = nicTx->portlist&0x1f;
	#endif
	pPkthdr->ph_srcExtPortNum = nicTx->srcExtPort;
	pPkthdr->ph_flags = nicTx->flags;
#if	defined(CONFIG_RTL_HW_QOS_SUPPORT)
	pPkthdr->ph_txPriority = nicTx->priority;
#endif
	//printk("%s ph_vlanId=%d ph_portlist=0x%x ph_flags=0x%x\n", __func__, pPkthdr->ph_vlanId, pPkthdr->ph_portlist, pPkthdr->ph_flags);

    /* Set cluster pointer to buffer */		
	pPkthdr->ph_mbuf->m_data    = (output);
	pPkthdr->ph_mbuf->m_extbuf = (output);
	if(DumpSwNicTxRx_debug)
	{
		printk("(%s)  pPkthdr->ph_vlanId=%d   pPkthdr->ph_portlist=0x%X    pPkthdr->ph_srcExtPortNum=0x%X\n",__func__
			,pPkthdr->ph_vlanId,pPkthdr->ph_portlist,pPkthdr->ph_srcExtPortNum);
		printk("(%s)--------pPkthdr-----------------\n",__func__);
		for(i=0;i<20;i++)
			printk("%02X  ",ptr_pPkthdr_debug[i]&0xFF);
		printk("\n(%s)--------packet content -----------------\n",__func__);
		skb_debug((char*)output);	
		printk("\n------------------------------------------\n");
	}
	
	ret = currTxPkthdrDescIndex[nicTx->txIdx];
	currTxPkthdrDescIndex[nicTx->txIdx] = next_index;
	/* Give descriptor to switch core */
	txPkthdrRing[nicTx->txIdx][ret] |= DESC_SWCORE_OWNED;

#if 0
	memDump((void*)output, 64, "TX");
	printk("index %d address 0x%p, 0x%x 0x%p.\n", ret, &txPkthdrRing[nicTx->txIdx][ret], (*(volatile uint32 *)&txPkthdrRing[nicTx->txIdx][ret]), pPkthdr);
	printk("Flags 0x%x proto 0x%x portlist 0x%x vid %d extPort %d srcExtPort %d len %d.\n", 
		pPkthdr->ph_flags, pPkthdr->ph_proto, pPkthdr->ph_portlist, pPkthdr->ph_vlanId, 
		pPkthdr->ph_extPortList, pPkthdr->ph_srcExtPortNum, pPkthdr->ph_len);
#endif

	/* Set TXFD bit to start send */
	REG32(CPUICR) |= TXFD;

	return ret;
}

int32 swNic_send(void *skb, void * output, uint32 len,rtl_nicTx_info *nicTx)
{
	int	ret;
	unsigned long flags;

	
	local_irq_save(flags);
	ret = _swNic_send(skb, output, len, nicTx);
	local_irq_restore(flags);	
	return ret;
}

__IRAM_FWD
int32 swNic_txDone(int idx)
{
	struct rtl_pktHdr	*pPkthdr;
	int				free_num;
	unsigned long flags;

	local_irq_save(flags);
	free_num = 0;
	{
		while (txPktDoneDescIndex[idx] != currTxPkthdrDescIndex[idx]) {		
			if ( (*(volatile uint32 *)&txPkthdrRing[idx][txPktDoneDescIndex[idx]] 
				& DESC_OWNED_BIT) == DESC_RISC_OWNED )
			{
				#ifdef CONFIG_RTL8196C_REVISION_B
				if (rtl_chip_version == RTL8196C_REVISION_A)
					txPkthdrRing[idx][txPktDoneDescIndex[idx]] =txPkthdrRing_backup[idx][txPktDoneDescIndex[idx]] ;
				#endif

				pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[idx][txPktDoneDescIndex[idx]] 
					& ~(DESC_OWNED_BIT | DESC_WRAP));

				if (pPkthdr->ph_mbuf->skb)
				{
					tx_done_callback(pPkthdr->ph_mbuf->skb);
					pPkthdr->ph_mbuf->skb = NULL;
				}
				
				if (++txPktDoneDescIndex[idx] == txPkthdrRingCnt[idx])
					txPktDoneDescIndex[idx] = 0;

				free_num++;
			}
			else
				break;
		}
	}

	local_irq_restore(flags);
	return free_num;	
}

#ifdef  CONFIG_RTL865X_MODEL_TEST_FT2
int32 swNic_send_portmbr(void * output, uint32 len, uint32 portmbr)
{
    struct rtl_pktHdr * pPkthdr;
    uint8 pktbuf[2048];
    uint8* pktbuf_alligned = (uint8*) (( (uint32) pktbuf & 0xfffffffc) | 0xa0000000);

    /* Copy Packet Content */
    memcpy(pktbuf_alligned, output, len);

    ASSERT_CSP( ((int32) txPkthdrRing[0][currTxPkthdrDescIndex] & DESC_OWNED_BIT) == DESC_RISC_OWNED );

    /* Fetch packet header from Tx ring */
    pPkthdr = (struct rtl_pktHdr *) ((int32) txPkthdrRing[0][currTxPkthdrDescIndex] 
                                                & ~(DESC_OWNED_BIT | DESC_WRAP));

    /* Pad small packets and add CRC */
    if ( len < 60 )
        pPkthdr->ph_len = 64;
    else
        pPkthdr->ph_len = len + 4;

    pPkthdr->ph_mbuf->m_len = pPkthdr->ph_len;
    pPkthdr->ph_mbuf->m_extsize = pPkthdr->ph_len;

    /* Set cluster pointer to buffer */
    pPkthdr->ph_mbuf->m_data = pktbuf_alligned;
    pPkthdr->ph_mbuf->m_extbuf = pktbuf_alligned;

    /* Set destination port */
    pPkthdr->ph_portlist = portmbr;

    /* Give descriptor to switch core */
    txPkthdrRing[0][currTxPkthdrDescIndex] |= DESC_SWCORE_OWNED;

    /* Set TXFD bit to start send */
    REG32(CPUICR) |= TXFD;
    
    /* Wait until packet is successfully sent */
#if 1    
    while ( (*(volatile uint32 *)&txPkthdrRing[0][currTxPkthdrDescIndex] 
                    & DESC_OWNED_BIT) == DESC_SWCORE_OWNED );
#endif    
    txPktCounter++;
    
    if ( ++currTxPkthdrDescIndex == txPkthdrRingCnt[0] )
        currTxPkthdrDescIndex = 0;

    return 0;
}
#endif


void swNic_freeRxBuf(void)
{
	int idx, i;
	struct rtl_pktHdr * pPkthdr;

	for(i=RTL865X_SWNIC_RXRING_MAX_PKTDESC-1; i >= 0 ; i--)
	{
		for (idx=0; idx<rxPkthdrRingCnt[i]; idx++)
		{
			if (!((rxPkthdrRing[i][idx] & DESC_OWNED_BIT) == DESC_RISC_OWNED)) {
				pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[i][idx] & 
					~(DESC_OWNED_BIT | DESC_WRAP));    

				/*if(pPkthdr == NULL || pPkthdr->ph_mbuf == NULL)
				*	continue;
				*/
				if (pPkthdr->ph_mbuf->skb)
				{
					free_rx_buf(pPkthdr->ph_mbuf->skb);
					pPkthdr->ph_mbuf->skb = NULL;
				}

				pPkthdr->ph_mbuf->m_data = NULL;
				pPkthdr->ph_mbuf->m_extbuf = NULL;
		    	}
	    	}
	}
}

//#pragma ghs section text=default
/*************************************************************************
*   FUNCTION                                                              
*       swNic_init                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function initializes descriptors and data structures.
*                                                                         
*   INPUTS                                                                
*       userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_HW_PKTDESC] :
*          Number of Rx pkthdr descriptors of each ring.
*       userNeedRxMbufRingCnt :
*          Number of Tx mbuf descriptors.
*       userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_HW_PKTDESC] :
*          Number of Tx pkthdr descriptors of each ring.
*       clusterSize :
*          Size of cluster.
*                                                                         
*   OUTPUTS                                                               
*       Status.
*************************************************************************/

int32 swNic_init(uint32 userNeedRxPkthdrRingCnt[RTL865X_SWNIC_RXRING_HW_PKTDESC],
                 uint32 userNeedRxMbufRingCnt,
                 uint32 userNeedTxPkthdrRingCnt[RTL865X_SWNIC_TXRING_HW_PKTDESC],
                 uint32 clusterSize)
{
	uint32 i, j, k;
	static uint32 totalRxPkthdrRingCnt = 0, totalTxPkthdrRingCnt = 0;
	static struct rtl_pktHdr *pPkthdrList_start;
	static struct rtl_mBuf *pMbufList_start;		
	struct rtl_pktHdr *pPkthdrList;
	struct rtl_mBuf *pMbufList;
	struct rtl_pktHdr * pPkthdr;
	struct rtl_mBuf * pMbuf;

	/* init const array for rx pre-process	*/
	extPortMaskToPortNum[0] = 0;
	extPortMaskToPortNum[1] = 1;
	extPortMaskToPortNum[2] = 2;
	extPortMaskToPortNum[3] = 0;
	extPortMaskToPortNum[4] = 3;
	extPortMaskToPortNum[5] = 0;
	extPortMaskToPortNum[6] = 0;
	extPortMaskToPortNum[7] = 0;
	
	rxPkthdrRefillThreshold[0] = ETH_REFILL_THRESHOLD;
	rxPkthdrRefillThreshold[1] = ETH_REFILL_THRESHOLD1;
	rxPkthdrRefillThreshold[2] = ETH_REFILL_THRESHOLD2;
	rxPkthdrRefillThreshold[3] = ETH_REFILL_THRESHOLD3;
	rxPkthdrRefillThreshold[4] = ETH_REFILL_THRESHOLD4;
	rxPkthdrRefillThreshold[5] = ETH_REFILL_THRESHOLD5;

	#if defined(CONFIG_RTL8196C_REVISION_B)
	rtl_chip_version = REG32(REVR);
	#endif
	
	if (rxMbufRing == NULL)
	{ 
		size_of_cluster = clusterSize;

		/* Allocate Rx descriptors of rings */
		for (i = 0; i < RTL865X_SWNIC_RXRING_HW_PKTDESC; i++) {   
			rxPkthdrRingCnt[i] = userNeedRxPkthdrRingCnt[i];//rxRingSize[]
			if (rxPkthdrRingCnt[i] == 0)
			{
				rxPkthdrRing[i] = NULL;
				continue;
			}

			rxPkthdrRing[i] = (uint32 *) UNCACHED_MALLOC(rxPkthdrRingCnt[i] * sizeof(uint32*));
			ASSERT_CSP( (uint32) rxPkthdrRing[i] & 0x0fffffff );

			totalRxPkthdrRingCnt += rxPkthdrRingCnt[i];
		}

		if (totalRxPkthdrRingCnt == 0)
			return EINVAL;

		/* Allocate Tx descriptors of rings */
		for (i = 0; i < RTL865X_SWNIC_TXRING_HW_PKTDESC; i++) {    
			txPkthdrRingCnt[i] = userNeedTxPkthdrRingCnt[i];//txRingSize[]

			if (txPkthdrRingCnt[i] == 0)
			{
				txPkthdrRing[i] = NULL;
				continue;
			}

			txPkthdrRing[i] = (uint32 *) UNCACHED_MALLOC(txPkthdrRingCnt[i] * sizeof(uint32*));
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing_backup[i]=(uint32 *) UNCACHED_MALLOC(txPkthdrRingCnt[i] * sizeof(uint32));
			#endif

			ASSERT_CSP( (uint32) txPkthdrRing[i] & 0x0fffffff );

			totalTxPkthdrRingCnt += txPkthdrRingCnt[i];
		}

		if (totalTxPkthdrRingCnt == 0)
			return EINVAL;

		/* Allocate MBuf descriptors of rings */
		rxMbufRingCnt = userNeedRxMbufRingCnt;

		if (userNeedRxMbufRingCnt == 0)
			return EINVAL;

		rxMbufRing = (uint32 *) UNCACHED_MALLOC(rxMbufRingCnt * sizeof(uint32*));
		ASSERT_CSP( (uint32) rxMbufRing & 0x0fffffff );

		/* Allocate pkthdr */
		pPkthdrList_start = (struct rtl_pktHdr *) UNCACHED_MALLOC(
		(totalRxPkthdrRingCnt + totalTxPkthdrRingCnt) * sizeof(struct rtl_pktHdr));
		ASSERT_CSP( (uint32) pPkthdrList_start & 0x0fffffff );

		/* Allocate mbufs */
		pMbufList_start = (struct rtl_mBuf *) UNCACHED_MALLOC(
		(rxMbufRingCnt + totalTxPkthdrRingCnt) * sizeof(struct rtl_mBuf));
		ASSERT_CSP( (uint32) pMbufList_start & 0x0fffffff );

	}

	/* Initialize interrupt statistics counter */
	rxPktCounter = txPktCounter = 0;

	/* Initialize index of Tx pkthdr descriptor */
	for (i=0;i<RTL865X_SWNIC_TXRING_HW_PKTDESC;i++)
	{
		currTxPkthdrDescIndex[i] = 0;
		txPktDoneDescIndex[i]=0;
	}

	pPkthdrList = pPkthdrList_start;
	pMbufList = pMbufList_start;

	/* Initialize Tx packet header descriptors */
	for (i = 0; i < RTL865X_SWNIC_TXRING_HW_PKTDESC; i++)
	{
		for (j = 0; j < txPkthdrRingCnt[i]; j++)
		{
			/* Dequeue pkthdr and mbuf */
			pPkthdr = pPkthdrList++;
			pMbuf = pMbufList++;

			bzero((void *) pPkthdr, sizeof(struct rtl_pktHdr));
			bzero((void *) pMbuf, sizeof(struct rtl_mBuf));

			pPkthdr->ph_mbuf = pMbuf;
			pPkthdr->ph_len = 0;
			pPkthdr->ph_flags = PKTHDR_USED | PKT_OUTGOING;
			pPkthdr->ph_type = PKTHDR_ETHERNET;
			pPkthdr->ph_portlist = 0;

			pMbuf->m_next = NULL;
			pMbuf->m_pkthdr = pPkthdr;
			pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
			pMbuf->m_data = NULL;
			pMbuf->m_extbuf = NULL;
			pMbuf->m_extsize = 0;

			txPkthdrRing[i][j] = (int32) pPkthdr | DESC_RISC_OWNED;
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing_backup[i][j]=(int32) pPkthdr | DESC_RISC_OWNED;
			#endif
		}

		if(txPkthdrRingCnt[i] > 0)
		{
			/* Set wrap bit of the last descriptor */
			txPkthdrRing[i][txPkthdrRingCnt[i] - 1] |= DESC_WRAP;
			#ifdef CONFIG_RTL8196C_REVISION_B
			if (rtl_chip_version == RTL8196C_REVISION_A)
				txPkthdrRing_backup[i][txPkthdrRingCnt[i] - 1] |= DESC_WRAP;
			#endif
		}

	}

	/* Fill Tx packet header FDP */
	REG32(CPUTPDCR0) = (uint32) txPkthdrRing[0];
	REG32(CPUTPDCR1) = (uint32) txPkthdrRing[1];

#ifdef CONFIG_RTL_8196D
	/* Fill Tx packet header FDP */
	REG32(CPUTPDCR2) = (uint32) txPkthdrRing[2];
	REG32(CPUTPDCR3) = (uint32) txPkthdrRing[3];
#endif

	/* Initialize Rx packet header descriptors */
	k = 0;

	for (i = 0; i < RTL865X_SWNIC_RXRING_HW_PKTDESC; i++)
	{
		for (j = 0; j < rxPkthdrRingCnt[i]; j++)
		{
			/* Dequeue pkthdr and mbuf */
			pPkthdr = pPkthdrList++;
			pMbuf = pMbufList++;

			bzero((void *) pPkthdr, sizeof(struct rtl_pktHdr));
			bzero((void *) pMbuf, sizeof(struct rtl_mBuf));

			/* Setup pkthdr and mbuf */
			pPkthdr->ph_mbuf = pMbuf;
			pPkthdr->ph_len = 0;
			pPkthdr->ph_flags = PKTHDR_USED | PKT_INCOMING;
			pPkthdr->ph_type = PKTHDR_ETHERNET;
			pPkthdr->ph_portlist = 0;
			pMbuf->m_next = NULL;
			pMbuf->m_pkthdr = pPkthdr;
			pMbuf->m_len = 0;
			pMbuf->m_flags = MBUF_USED | MBUF_EXT | MBUF_PKTHDR | MBUF_EOR;
			pMbuf->m_extsize = size_of_cluster;
			pMbuf->m_data = pMbuf->m_extbuf = alloc_rx_buf(&pPkthdr->ph_mbuf->skb, size_of_cluster);			

			/* Setup descriptors */
			rxPkthdrRing[i][j] = (int32) pPkthdr | DESC_SWCORE_OWNED;
			rxMbufRing[k++] = (int32) pMbuf | DESC_SWCORE_OWNED;
		}

		/* Initialize index of current Rx pkthdr descriptor */
		currRxPkthdrDescIndex[i] = 0;

		/* Initialize index of current Rx Mbuf descriptor */
		currRxMbufDescIndex = 0;

		/* Set wrap bit of the last descriptor */
		if(rxPkthdrRingCnt[i] > 0)
			rxPkthdrRing[i][rxPkthdrRingCnt[i] - 1] |= DESC_WRAP;

		#ifdef DELAY_REFILL_ETH_RX_BUF
		rxDescReadyForHwIndex[i] = 0;
		#endif
	}

	rxMbufRing[rxMbufRingCnt - 1] |= DESC_WRAP;

	/* Fill Rx packet header FDP */
	REG32(CPURPDCR0) = (uint32) rxPkthdrRing[0];
	REG32(CPURPDCR1) = (uint32) rxPkthdrRing[1];
	REG32(CPURPDCR2) = (uint32) rxPkthdrRing[2];
	REG32(CPURPDCR3) = (uint32) rxPkthdrRing[3];
	REG32(CPURPDCR4) = (uint32) rxPkthdrRing[4];
	REG32(CPURPDCR5) = (uint32) rxPkthdrRing[5];

	REG32(CPURMDCR0) = (uint32) rxMbufRing;

	return SUCCESS;
}


#ifdef FAT_CODE
/*************************************************************************
*   FUNCTION                                                              
*       swNic_resetDescriptors                                         
*                                                                         
*   DESCRIPTION                                                           
*       This function resets descriptors.
*                                                                         
*   INPUTS                                                                
*       None.
*                                                                         
*   OUTPUTS                                                               
*       None.
*************************************************************************/
void swNic_resetDescriptors(void)
{
    /* Disable Tx/Rx and reset all descriptors */
    REG32(CPUICR) &= ~(TXCMD | RXCMD);
    return;
}
#endif//FAT_CODE

#if 	defined(CONFIG_RTL_PROC_DEBUG)
/*	dump the rx ring info	*/
int32	rtl_dumpRxRing(void)
{
	int	idx, cnt;
	struct rtl_pktHdr * pPkthdr;

	for(idx=0;idx<RTL865X_SWNIC_RXRING_HW_PKTDESC;idx++)
	{
		printk("**********************************************\nRxRing%d: cnt %d\n",
			idx, rxPkthdrRingCnt[idx]);

		/*	skip the null rx ring */
		if (rxPkthdrRingCnt[idx]==0)
			continue;

		/*	dump all the pkt header	*/
		for(cnt=0;cnt<rxPkthdrRingCnt[idx];cnt++)
		{
			pPkthdr = (struct rtl_pktHdr *) (rxPkthdrRing[idx][cnt] & ~(DESC_OWNED_BIT | DESC_WRAP));
			
				#ifdef DELAY_REFILL_ETH_RX_BUF
			printk("  idx[%d]: 0x%p-->mbuf[0x%p],skb[0x%p]%s%s%s%s\n",  cnt, pPkthdr, pPkthdr->ph_mbuf, pPkthdr->ph_mbuf->skb,
				(rxPkthdrRing[idx][cnt]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE", 
				(rxPkthdrRing[idx][cnt]&DESC_WRAP)!=0?" :WRAP":"",
				cnt==currRxPkthdrDescIndex[idx]?"  <===currIdx":"",
				cnt ==rxDescReadyForHwIndex[idx]?" <===readyForHw":"");
				#endif
		}
	}
	return SUCCESS;
}

/*	dump the tx ring info	*/
int32	rtl_dumpTxRing(void)
{
	int	idx, cnt;
	struct rtl_pktHdr * pPkthdr = NULL;

	for(idx=0;idx<RTL865X_SWNIC_TXRING_HW_PKTDESC;idx++)
	{
		printk("**********************************************\nTxRing%d: cnt %d\n",
			idx, txPkthdrRingCnt[idx]);

		/*	skip the null rx ring */
		if (txPkthdrRingCnt[idx]==0)
			continue;

		/*	dump all the pkt header	*/
		for(cnt=0;cnt<txPkthdrRingCnt[idx];cnt++)
		{
 #ifdef CONFIG_RTL8196C_REVISION_B
		  if (rtl_chip_version == RTL8196C_REVISION_A)
			pPkthdr = (struct rtl_pktHdr *) (txPkthdrRing_backup[idx][cnt] & ~(DESC_OWNED_BIT | DESC_WRAP));
		  else
#endif
			pPkthdr = (struct rtl_pktHdr *) (txPkthdrRing[idx][cnt] & ~(DESC_OWNED_BIT | DESC_WRAP));

			printk("  idx[%d]: 0x%p-->mbuf[0x%p],skb[0x%p]%s%s%s%s\n",  cnt, pPkthdr, pPkthdr->ph_mbuf, pPkthdr->ph_mbuf->skb, 
				(txPkthdrRing[idx][cnt]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE", 
				(txPkthdrRing[idx][cnt]&DESC_WRAP)!=0?" :WRAP":"",
				cnt==currTxPkthdrDescIndex[idx]?"  <===currIdx":"", 
				cnt==txPktDoneDescIndex[idx]?"  <===txDoneIdx":"");
		}
	}
	return SUCCESS;
}

/*	dump the tx ring info	*/
int32	rtl_dumpMbufRing(void)
{
	int	idx;
	struct rtl_mBuf *mbuf;

	idx = 0;
	printk("**********************************************\nMbufRing:\n");
	while(1)
	{
		mbuf = (struct rtl_mBuf *)(rxMbufRing[idx] & ~(DESC_OWNED_BIT | DESC_WRAP));
		printk("mbuf[%d]: 0x%p: ==> pkthdr[0x%p] ==> skb[0x%p]%s%s%s\n", idx, mbuf, mbuf->m_pkthdr, 
				mbuf->skb, 
				(rxMbufRing[idx]&DESC_OWNED_BIT)==DESC_RISC_OWNED?" :CPU":" :SWCORE", 
				(rxMbufRing[idx]&DESC_WRAP)==DESC_ENG_OWNED?" :WRAP":"",
				idx==currRxMbufDescIndex?"  <===currIdx":"");
			if ((rxMbufRing[idx]&DESC_WRAP)!=0)
				break;
			idx++;
	}
	return SUCCESS;
}
#endif



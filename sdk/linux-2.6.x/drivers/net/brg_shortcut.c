#include "brg_shortcut.h"
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/crc32.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/proc_fs.h>

#ifdef CONFIG_RTL_MULTI_ETH_WAN
#include <linux/if_smux.h>
#endif
#ifdef CONFIG_RTL8676_Dynamic_ACL
#include <net/rtl/rtl867x_hwnat_api.h>
#endif

//#include <linux/imq.h>

#define BRG_SC_HASH_TABLE

//#define CONFIG_RTL8672_ETHSKB_CONTROL_POOL
//#define CONFIG_RTK_ETHUP 

#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL
extern int net_smallpkt_heavytraffic;
#endif
#ifdef CONFIG_RTL8672_BRIDGE_FASTPATH
#ifdef BRG_SC_HASH_TABLE
#define MAX_BRG_SC_ENTRY_NUM	256
#else
#define MAX_BRG_SC_ENTRY_NUM	8
#endif



int BRG_ENTRY_AGING_TIME =BRG_ENTRY_AGING_TIME_NORMAL;
//int BRG_ENTRY_FORCE_TIMEOUT=BRG_ENTRY_FORCE_TIMEOUT_NORMAL;



typedef struct brg_shortcut_entry {
	unsigned char enable;
	unsigned short macPair[6];
	struct net_device *srcDev;
	struct net_device *dstDev;
	unsigned int mark;//for IP QoS
	unsigned long tick;
}BRG_SHORTCUT_ENTRY;


#ifdef CONFIG_RTL8672
__DRAM
#endif 
static BRG_SHORTCUT_ENTRY fbTbl[MAX_BRG_SC_ENTRY_NUM]; // for downstream

#ifndef BRG_SC_HASH_TABLE
#ifdef CONFIG_RTL8672
__DRAM
#endif 
static BRG_SHORTCUT_ENTRY fbTblup[MAX_BRG_SC_ENTRY_NUM]; //for upstream
#endif

static unsigned int brg_shortcut_enable = 1;

#ifdef BRG_SC_HASH_TABLE
/*
 * hash algorithm
 * hash = (dmac[0]^dmac[1]^dmac[2]^dmac[3]^dmac[4]^dmac[5])^(~(smac[0]^smac[1]^smac[2]^smac[3]^smac[4]^smac[5]))
 */
__IRAM  inline static unsigned char
BrgSC_Hash_Entry(unsigned short *mac)
{
	register unsigned int hash, tmp;
	
	tmp = *mac ^ *(unsigned int *)(mac+1);
	tmp = (tmp ^ (tmp>>16));
	tmp = (tmp ^ (tmp>>8));

	hash = *(unsigned int *)(mac+3) ^ *(mac+5);
	hash = (hash ^ (hash>>16));
	hash = ~(hash ^ (hash>>8));

	hash = (hash + tmp) & 0xFF;
	
	return ((hash ^ (hash >> 6)) << 2) & 0xFF;
}
#endif

void brgClearTableByDev(struct net_device *dev)
{
	BRG_SHORTCUT_ENTRY *pfbTbl;
	int i;

#ifndef BRG_SC_HASH_TABLE
	pfbTbl=fbTblup;
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
	{
		if ((pfbTbl[i].srcDev->name[0]==dev->name[0]) || (pfbTbl[i].dstDev->name[0]==dev->name[0]))
			pfbTbl[i].enable=0;
	}
#endif

	pfbTbl=fbTbl;
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
	{
		if ((pfbTbl[i].srcDev->name[0]==dev->name[0]) || (pfbTbl[i].dstDev->name[0]==dev->name[0]))
			pfbTbl[i].enable=0;
	}
}

/*when src mac learned by a new interface, all related brgSC entry should be deleted*/
int brgScDelete(unsigned char *mac)
{
	BRG_SHORTCUT_ENTRY *pfbTbl;
	unsigned short *pmac = (unsigned short *)mac;
	unsigned short *smac, *dmac;
	//unsigned long tick_tmp=0;
	int i;

#ifndef BRG_SC_HASH_TABLE
	pfbTbl = fbTblup;
DEL:
#else
	pfbTbl = fbTbl;
#endif
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
	{
		if (!pfbTbl[i].enable)
			continue;

			smac = &pfbTbl[i].macPair[3];
			dmac = &pfbTbl[i].macPair[0];
			if (!((pmac[0]^smac[0])|(pmac[1]^smac[1])|(pmac[2]^smac[2])) ||
				!((pmac[0]^dmac[0])|(pmac[1]^dmac[1])|(pmac[2]^dmac[2]))){
				pfbTbl[i].enable = 0;
		}
	}
#ifndef BRG_SC_HASH_TABLE
	if (pfbTbl == fbTblup) {
		pfbTbl = fbTbl;
		goto DEL;
	}
#endif

	return 0;
}


/* Kevin, when the skb pass through br_flood, it cannot record rest. if , so delete the entry */

void brgEntryDelete(unsigned short *s_mac,unsigned short *d_mac,int dir)
{
    BRG_SHORTCUT_ENTRY *pfbTbl;
    unsigned short *smac, *dmac;
    int i;

#ifndef BRG_SC_HASH_TABLE
    if (dir == DIR_LAN)
		pfbTbl = fbTblup;
	else
		pfbTbl = fbTbl;
#else
	pfbTbl = fbTbl;
#endif

    for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
	{
		if (!pfbTbl[i].enable)
			continue;

		smac = &pfbTbl[i].macPair[3];
		dmac = &pfbTbl[i].macPair[0];
		if (!((s_mac[0]^smac[0])|(s_mac[1]^smac[1])|(s_mac[2]^smac[2])) &&
			!((d_mac[0]^dmac[0])|(d_mac[1]^dmac[1])|(d_mac[2]^dmac[2])))
			pfbTbl[i].enable = 0;		
	}


}



/*
 * dst: source device
 * mac: source mac
 */
int brgScFind(struct net_device *dst, unsigned char *mac, unsigned long *tick)
{
	BRG_SHORTCUT_ENTRY *pfbTbl;
	unsigned short *pmac = (unsigned short *)mac;
	unsigned short *smac;
	unsigned long tick_tmp=0;
	int i;

#ifndef BRG_SC_HASH_TABLE
	pfbTbl = fbTblup;
FIND:
#else
	pfbTbl = fbTbl;
#endif
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
	{
		if (!pfbTbl[i].enable)
			continue;

		if (pfbTbl[i].srcDev->name[0] == dst->name[0]) {
			smac = &pfbTbl[i].macPair[3];
			if (!((pmac[0]^smac[0])|(pmac[1]^smac[1])|(pmac[2]^smac[2]))){
				if ((jiffies-pfbTbl[i].tick) > BRG_ENTRY_AGING_TIME) {
					pfbTbl[i].enable = 0;
					continue;
		}

				if (tick_tmp == 0)
					tick_tmp = pfbTbl[i].tick;
				else
					tick_tmp = (tick_tmp >= pfbTbl[i].tick)?tick_tmp:pfbTbl[i].tick;
			}
			}
		}

	if (tick_tmp) {
		*tick = tick_tmp;
				return 1;
			}
#ifndef BRG_SC_HASH_TABLE
	if (pfbTbl == fbTblup) {
		pfbTbl = fbTbl;
		goto FIND;
	}
#endif

	return 0;
}

//rx will learn source mac and destination mac, and tx will learn related destination interface
/*****************************************************************
** NAME: brgShortcutLearnMac
** PARA: pMac - ethernet header, that is DA/SA
              srcDev - packet receive device.
** RETURN: void
*****************************************************************/
static void  brgShortcutLearnMac(unsigned short *pMac, struct net_device *srcDev, int dir)
{
	BRG_SHORTCUT_ENTRY *pfbTbl;
	int i, index=-1, selected=0;
	unsigned long maxelapse=0;

#ifndef BRG_SC_HASH_TABLE
	if (dir == DIR_LAN)
		pfbTbl = fbTblup;
	else
		pfbTbl = fbTbl;
#else
	int hash;
	pfbTbl = fbTbl;
	hash = BrgSC_Hash_Entry(pMac);
#endif

#ifndef BRG_SC_HASH_TABLE
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
#else
	for (i=hash; i<hash+4; i++)
#endif
	{
		if (pfbTbl[i].enable)
		{
			unsigned short *pmac = pfbTbl[i].macPair;
			if (!((pmac[0]^pMac[0])|(*(unsigned int *)&pmac[1]^*(unsigned int *)&pMac[1])|(*(unsigned int *)&pmac[3]^*(unsigned int *)&pMac[3])|(pmac[5]^pMac[5])))
			{
				if(pfbTbl[i].srcDev != srcDev)
				{
					pfbTbl[i].srcDev = srcDev;
					pfbTbl[i].dstDev = NULL;
					pfbTbl[i].tick = jiffies;
				}
				return;
			}

			if ((maxelapse==0) || (time_after_eq(maxelapse, pfbTbl[i].tick))){
				maxelapse = pfbTbl[i].tick;
				selected = i;
			}
		}
		else{
			if (index == -1)
				index = i;
		}
	}

	index = index<0?selected:index;
	pfbTbl[index].macPair[0] = pMac[0];
	pfbTbl[index].macPair[1] = pMac[1];
	pfbTbl[index].macPair[2] = pMac[2];
	pfbTbl[index].macPair[3] = pMac[3];
	pfbTbl[index].macPair[4] = pMac[4];
	pfbTbl[index].macPair[5] = pMac[5];
	pfbTbl[index].srcDev = srcDev;
	pfbTbl[index].dstDev = NULL;
	pfbTbl[index].tick = jiffies;
	pfbTbl[index].enable = 1;

	
}

/*****************************************************************
** NAME: brgShortcutGetEntry
** PARA: pMacPair - ethernet header, that is DA/SA
              srcDev - packet receive device.
** RETURN: bridge shortcut entry
*****************************************************************/
__IRAM BRG_SHORTCUT_ENTRY * brgShortcutGetEntry(unsigned short *pMacPair, struct net_device *srcDev, int dir)
{
	BRG_SHORTCUT_ENTRY *pfbTbl;
	unsigned short *pmac;
	int i;
	BRG_SHORTCUT_ENTRY * pEntry = NULL;

#ifndef BRG_SC_HASH_TABLE
	if (dir == DIR_LAN)
		pfbTbl = fbTblup;
	else
		pfbTbl = fbTbl;
#else
	int hash;
	pfbTbl = fbTbl;
	hash = BrgSC_Hash_Entry(pMacPair);
#endif

#ifndef BRG_SC_HASH_TABLE
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
#else
	for (i=hash; i<hash+4; i++)
#endif
	{
		if (pfbTbl[i].enable && pfbTbl[i].dstDev && (pfbTbl[i].srcDev->name[0]==srcDev->name[0]))
		{
			{
#ifdef CONFIG_RTL8672_ETHSKB_CONTROL_POOL			
				if(!net_smallpkt_heavytraffic)
#endif					
				{
					if((jiffies-pfbTbl[i].tick) > BRG_ENTRY_AGING_TIME){
						pfbTbl[i].enable = 0;
						continue;
					}
				}	
			}
			pmac = pfbTbl[i].macPair;
			if ( !((pmac[0]^pMacPair[0])|(*(unsigned int *)&pmac[1]^*(unsigned int *)&pMacPair[1])|(*(unsigned int *)&pmac[3]^*(unsigned int *)&pMacPair[3])|(pmac[5]^pMacPair[5])) )
			{
				//get entry, update age time.				
				pfbTbl[i].tick = jiffies;
				pEntry = pfbTbl+i;
				break;
			}
		}
	}

	return pEntry;
}






int debugBridge;
/*****************************************************************
** NAME: brgShortcutLearnDestItf
** PARA: skb - transmit packet
              dstDev -packet transmit device.
** RETURN: 
*****************************************************************/
void brgShortcutLearnDestItf(struct sk_buff *skb, struct net_device *dstDev)
{
	BRG_SHORTCUT_ENTRY *pfbTbl;
	int i;
	unsigned short *pMacPair,*pmac;
#ifdef BRG_SC_HASH_TABLE
	int hash;
#endif

	if(brg_shortcut_enable){
		pmac = (unsigned short *)skb->data;

		if((*(unsigned char *)pmac) & 0x1)	/*not support multicast*/
			return;

#ifndef BRG_SC_HASH_TABLE
		pfbTbl = fbTblup;

FIND:
		for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
#else
		pfbTbl = fbTbl;
		hash = BrgSC_Hash_Entry(pmac);
		for (i=hash; i<hash+4; i++)
#endif
		{
			if (pfbTbl[i].enable)
			{			
				pMacPair = pfbTbl[i].macPair;			
				if (!((pmac[0]^pMacPair[0])|(*(unsigned int *)&pmac[1]^*(unsigned int *)&pMacPair[1])|(*(unsigned int *)&pmac[3]^*(unsigned int *)&pMacPair[3])|(pmac[5]^pMacPair[5])))
				{ 

					pfbTbl[i].tick = jiffies;
					pfbTbl[i].dstDev = dstDev;	
									
				#ifdef CONFIG_RTL8676_Dynamic_ACL
				{
					unsigned char *input_mac = (unsigned char *)pfbTbl[i].macPair;					
					rtl8676_add_L2Unicast_hwacc(&input_mac[6],&input_mac[0],pfbTbl[i].srcDev->name,pfbTbl[i].dstDev->name);	
				}
				#endif
					
					return;
				}
			}
		}

#ifndef BRG_SC_HASH_TABLE
        if(pfbTbl== fbTblup)
        {
            pfbTbl = fbTbl;
            goto FIND;
        }
#endif
	}
}
EXPORT_SYMBOL(brgShortcutLearnDestItf);

/*****************************************************************
** NAME: brgShortcutProcess
** PARA: skb - transmit packet
		srcDev -packet receive device.
** RETURN: 1 - go through bridge shortcut.
		    0 - transfer to upper layer
*****************************************************************/
__IRAM_NIC int brgShortcutProcess(struct sk_buff *skb, struct net_device *srcDev, int dir)
{
	BRG_SHORTCUT_ENTRY *pFbEntry;
	unsigned short *pMacHdr;
	unsigned long flags;

	if(*(unsigned char *)skb->data & 0x1) /*not support multicast packet*/
		return 0;

	local_irq_save(flags);
	
	if(brg_shortcut_enable){
		pMacHdr = (unsigned short *)skb->data;
		if(pMacHdr[6] == 0x8100){	/*not support vlan packet*/
			local_irq_restore(flags);
			return 0;
		}

		if((pFbEntry = brgShortcutGetEntry(pMacHdr, srcDev, dir)))
		{
			skb->dev = pFbEntry->dstDev;


			if(pFbEntry->dstDev->hard_start_xmit(skb,pFbEntry->dstDev))
			{
				pFbEntry->enable = 0;
				dev_kfree_skb(skb);
				printk("fast bridge tx error!\n");
			}
			local_irq_restore(flags);
			return 1;
		}
		else
			brgShortcutLearnMac(pMacHdr, srcDev, dir);
	}

	local_irq_restore(flags);
	return 0;
}

#ifdef CONFIG_RTL865X_ETH_PRIV_SKB_ADV
__IRAM
int brgFastForwarding(struct sk_buff *skb, int dir)
{
        BRG_SHORTCUT_ENTRY *pFbEntry;
        unsigned short *pMacHdr;
        unsigned long flags;

        if(*(unsigned char *)skb->data & 0x1) /*not support multicast packet*/
                return 0;

        local_irq_save(flags);

        if(brg_shortcut_enable){
                pMacHdr = (unsigned short *)skb->data;
                if(pMacHdr[6] == 0x8100){       /*not support vlan packet*/
                        local_irq_restore(flags);
                        return 0;
                }
                if ((pFbEntry = brgShortcutGetEntry(pMacHdr, skb->dev, dir)))
                {
                        if ( (skb->prealloc_flags & RETFREEQ_FF) && //only packet from NIC should be checked here(BUg in WS331, because of wlan<->wlan stream).
                                        ((pFbEntry->dstDev->priv_flags & IFF_DOMAIN_WLAN) ||
                                         ((pFbEntry->dstDev->priv_flags & (IFF_DOMAIN_WAN|IFF_OSMUX)) == (IFF_DOMAIN_WAN))))
                        {
                                extern void rtl865x_free_eth_priv_buf(struct sk_buff *skb, unsigned int flag);
                                extern void reinit_skbhdr(struct sk_buff *skb, void (*prealloc_cb)(struct sk_buff *, unsigned));
                                reinit_skbhdr(skb, rtl865x_free_eth_priv_buf);
				skb->len = skb->tail - skb->data;
                        }

                        skb->dev = pFbEntry->dstDev;

                        if (pFbEntry->dstDev->netdev_ops->ndo_start_xmit(skb,pFbEntry->dstDev))
                        {
                                pFbEntry->enable = 0;
                                dev_kfree_skb(skb);
                                printk("fast bridge tx error!\n");
                        }
                        local_irq_restore(flags);
                        return 1;
                }
        }

        local_irq_restore(flags);
        return 0;
}
#endif//end of CONFIG_RTL865X_ETH_PRIV_SKB_ADV


void brgClearTable(void)
{
	BRG_SHORTCUT_ENTRY *pfbTbl;
	int i;
#ifndef BRG_SC_HASH_TABLE
	pfbTbl=fbTblup;
	
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
	{
		pfbTbl[i].enable=0;
	}
#endif
	pfbTbl=fbTbl;
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
	{
		pfbTbl[i].enable=0;
	}
	BRG_ENTRY_AGING_TIME =BRG_ENTRY_AGING_TIME_NORMAL;
 	//BRG_ENTRY_FORCE_TIMEOUT=BRG_ENTRY_FORCE_TIMEOUT_NORMAL;

	
}


static int fastbridge_read_proc(char *page, char **start, off_t off,
				int count, int *eof, void *data)
{
	int i;
	char srcportbuf[16];
	char dstportbuf[16];

#ifndef BRG_SC_HASH_TABLE
	printk("\nrtk fast bridge is %s\n",brg_shortcut_enable?"enabled":"disabled");
	printk("************************fast bridge table***************************\n");
	printk("Index   Enabled   TimeOut   DstMac           SrcMac           SrcItf     DstItf    MARK   DIR\n");
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
	{
		if (fbTbl[i].enable){
			if (fbTbl[i].srcDev)
				sprintf(srcportbuf,"%s",fbTbl[i].srcDev->name);
			else
				sprintf(srcportbuf,"---");
			
			if(fbTbl[i].dstDev){
				sprintf(dstportbuf,"%s",fbTbl[i].dstDev->name);
			}else{
				sprintf(dstportbuf,"---");
			}
			printk("%-5d   %-7d   %-7d   %04x:%04x:%04x   %04x:%04x:%04x   %-10s   %-9s   %-6u  DOWN\n",
				i,fbTbl[i].enable,(jiffies-fbTbl[i].tick)>BRG_ENTRY_AGING_TIME?1:0,fbTbl[i].macPair[0],fbTbl[i].macPair[1],fbTbl[i].macPair[2],
				fbTbl[i].macPair[3],fbTbl[i].macPair[4],fbTbl[i].macPair[5],srcportbuf,dstportbuf, fbTbl[i].mark);
		}
	}
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
	{
		if (fbTblup[i].enable){
			if (fbTblup[i].srcDev)
				sprintf(srcportbuf,"%s",fbTblup[i].srcDev->name);
			else
				sprintf(srcportbuf,"---");
			
			if(fbTblup[i].dstDev){
				sprintf(dstportbuf,"%s",fbTblup[i].dstDev->name);
			}else{
				sprintf(dstportbuf,"---");
			}
			printk("%-5d   %-7d   %-7d   %04x:%04x:%04x   %04x:%04x:%04x   %-10s   %-9s  %10x  UP\n",
				i,fbTblup[i].enable,(jiffies-fbTblup[i].tick)>BRG_ENTRY_AGING_TIME?1:0,fbTblup[i].macPair[0],fbTblup[i].macPair[1],fbTblup[i].macPair[2],
				fbTblup[i].macPair[3],fbTblup[i].macPair[4],fbTblup[i].macPair[5],srcportbuf,dstportbuf, fbTblup[i].mark);
		}
	}
#else
	int hash;

	printk("\nrtk fast bridge is %s\n",brg_shortcut_enable?"enabled":"disabled");
	printk("************************fast bridge table***************************\n");
	printk("%-8s%-10s%-16s%-14s%-9s%-9s%-6s\n", 
			"Index", "valid", "DstMac", "SrcMac", "SrcItf", "DstItf", "MARK");
	for (i=0; i<MAX_BRG_SC_ENTRY_NUM; i++)
	{
		if (fbTbl[i].enable){
			if (fbTbl[i].srcDev)
				sprintf(srcportbuf,"%s",fbTbl[i].srcDev->name);
			else
				sprintf(srcportbuf,"---");
			
			if(fbTbl[i].dstDev){
				sprintf(dstportbuf,"%s",fbTbl[i].dstDev->name);
			}else{
				sprintf(dstportbuf,"---");
			}
			printk("%-3d[%-2d] %-9s %04x:%04x:%04x %04x:%04x:%04x  %-9s%-9s%-10x\n",
				i, i>>2, (fbTbl[i].enable && (jiffies-fbTbl[i].tick)<BRG_ENTRY_AGING_TIME)?"valid":"invalid",
				fbTbl[i].macPair[0],fbTbl[i].macPair[1],fbTbl[i].macPair[2],
				fbTbl[i].macPair[3],fbTbl[i].macPair[4],fbTbl[i].macPair[5],
				srcportbuf,dstportbuf, fbTbl[i].mark);
		}
	}
#endif

	return 0;
}


static int fastbridge_write_proc(struct file *file, const char *buffer,	unsigned long count, void *data)
{
	unsigned char flag;

	if (count < 2)
		return -EFAULT;
	
	if (buffer && !copy_from_user(&flag, buffer, 1)) {
		if(flag == '0')
		{
			brg_shortcut_enable = 0;
            printk("rtk fast bridge is disabled\n");
		}
		else if(flag == '1')
		{
			brg_shortcut_enable = 1;
            printk("rtk fast bridge is enabled\n");
		}
		else if(flag == '2')
		{                    
            brgClearTable();
            printk("clean rtk fast bridge table\n");     
		}
		return count;
	}else
		return -EFAULT;
}

static int __init fastbridge_init(void) 
{

	struct proc_dir_entry *entry=NULL;	
	entry = create_proc_entry("fastbridge", 0, NULL);
	if (entry) {
		entry->read_proc = fastbridge_read_proc;
		entry->write_proc = fastbridge_write_proc;
	}
	else {
		printk("Realtek fastbridge, create proc failed!\n");
	}

	return 0;
}

static void __exit fastbridge_exit(void) 
{

}

module_init(fastbridge_init);
module_exit(fastbridge_exit);

#endif


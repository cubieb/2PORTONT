/*
 *   Handling routines for 802.11 Beacon 
 *
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */


#ifdef __KERNEL__
#ifdef __MIPSEB__
#include <asm/addrspace.h>
#include <linux/module.h>
#endif
#include <linux/list.h>
#include <linux/random.h>
#endif
#include "../8190n_cfg.h"
#include "../8190n.h"
#include "../wifi.h"
#include "../8190n_hw.h"
#include "../8190n_headers.h"
#include "../8190n_rx.h"
#include "../8190n_debug.h"

#ifdef RTL8192SE
int TxBeacon(struct mac80211_shared_priv *priv, 
								unsigned char *dat_content, 
								unsigned short txLength)
{
		
		struct rtl8190_hw	*phw;
		struct tx_desc	*pdesc;
		unsigned int	frlen;
		unsigned long	ioaddr = priv->pshare_hw->ioaddr;
		int next_idx = 1;

		phw = GET_HW(priv);
		pdesc = phw->tx_descB;
		frlen = txLength;
		
		
		// prevent DMA right now
		pdesc->Dword0 = 0;

		pdesc->Dword0 = set_desc(TX_FirstSeg| TX_LastSeg|  (32)<<TX_OFFSETSHIFT | (frlen) << TX_PktSizeSHIFT);
		pdesc->Dword1 = set_desc(0x10 << TX_QueueSelSHIFT);
		pdesc->Dword4 = set_desc((0x7 << TX_RaBRSRIDSHIFT) | TX_UserRate);
		pdesc->Dword5 = set_desc(TX_DISFB);
		pdesc->Dword7 = set_desc(frlen & TX_TxBufferSizeMask);


		if (!IS_DRV_OPEN(priv))
			return FALSE;

		pdesc->Dword0 |= set_desc(TX_OWN);
		SetSeqNum((unsigned char *)dat_content, phw->seq);
		phw->seq = (phw->seq + 1) & 0xfff;
		pdesc->Dword3 |= set_desc((GetSequence(dat_content) & TX_SeqMask)<< TX_SeqSHIFT);
		pdesc->Dword8 = set_desc(get_physical_addr(priv, dat_content, 128*sizeof(unsigned int), PCI_DMA_TODEVICE));
		pdesc->Dword9 = set_desc(get_physical_addr(priv, &phw->tx_descB[next_idx], sizeof(struct tx_desc), PCI_DMA_TODEVICE));
		rtl_cache_sync_wback(priv, get_desc(pdesc->Dword8), 128*sizeof(unsigned int), PCI_DMA_TODEVICE);

		// Now we use sw beacon, we need to poll it every time.
		RTL_W16(TPPoll, TPPoll_BQ);

		return TRUE;
}


void mac80211_send_beacon(struct mac80211_shared_priv *priv)
{
	//unsigned long	ioaddr = priv->pshare_hw->ioaddr;
	struct sk_buff *skb;
	

	if (!IS_DRV_OPEN(priv))
		return;

	if (priv->bcn_skb != NULL){
		skb = priv->bcn_skb;
		pci_unmap_single(priv->pshare_hw->pdev, priv->bcndma_addr, 
				skb->len, PCI_DMA_FROMDEVICE);
		dev_kfree_skb_any(skb);
		priv->bcn_skb = NULL;
	}

	skb = ieee80211_beacon_get(priv->hw, priv->vif);
	//printk("seq = %d\n", GetSequence(skb->data));
	if (skb == NULL) {
		printk("cannot get beacon skb\n");
		return;
	}

	if (!TxBeacon(priv, (unsigned char *)skb->data, skb->len)) {
		printk("TxBeacon Error\n");
		dev_kfree_skb_any(skb);
		return;
	}
	priv->bcn_skb = skb;
	priv->bcndma_addr =	get_physical_addr(priv, skb->data, 128*sizeof(unsigned int), PCI_DMA_TODEVICE);
}
#endif

#ifdef RTL8192SU
void mac80211_send_beacon(struct mac80211_shared_priv *priv)
{

}
#endif


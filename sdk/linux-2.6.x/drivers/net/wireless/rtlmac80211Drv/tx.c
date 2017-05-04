/*
 *  TX handle routines
 *
 *  $Id: tx.c,v 1.1.1.1 2010/05/05 09:00:44 jiunming Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/netdevice.h>
#endif

#define _DEBUG_RTL8190_

#include "./8190n_cfg.h"
#include "./8190n.h"
#include "./8190n_hw.h"

#include "./rtl80211.h"


#include "./8190n_util.h"
#include "./8190n_headers.h"

void rtl_tx_aggr_resume(struct mac80211_shared_priv *priv, struct ieee80211_sta *sta, u16 tid)
{
	struct rtl_atx_tid *txtid;
	struct priv_stainfo *node;

	node = (struct priv_stainfo *)sta->drv_priv;

	if (priv->sc_flags & SC_OP_TXAGGR) {
		txtid = RTL_ND_2_TID(node, tid);
		txtid->baw_size =
		        IEEE80211_MIN_AMPDU_BUF << sta->ht_cap.ampdu_factor;
		txtid->state |= AGGR_ADDBA_COMPLETE;
		txtid->state &= ~AGGR_ADDBA_PROGRESS;
		/* brian replace with ampdu_xmit */
		//ath_tx_resume_tid(sc, txtid);
	}
}

/*
static void rtl_tx_pause_tid(struct mac80211_shared_priv *priv, struct rtl_atx_tid *tid)
{
        struct rtl_txq *txq = &priv->tx.txq[tid->ac->qnum];

        spin_lock_bh(&txq->axq_lock);
        tid->paused++;
        spin_unlock_bh(&txq->axq_lock);
}
*/

int rtl_tx_aggr_start(struct mac80211_shared_priv *rtl_priv, struct ieee80211_sta *sta,
                      u16 tid, u16 *ssn)
{
	struct rtl_atx_tid *txtid;
	struct priv_stainfo *node;

	node = (struct priv_stainfo *)sta->drv_priv;

	if (rtl_priv->sc_flags & SC_OP_TXAGGR) {
		txtid = RTL_ND_2_TID(node, tid);
		txtid->state |= AGGR_ADDBA_PROGRESS;
		/* rtl_tx_pause_tid(sc, txtid); */
		*ssn = txtid->seq_start;
	}

	return 0;
	}

bool rtl_tx_aggr_check(struct mac80211_shared_priv *rtl_priv, struct priv_stainfo *node, u8 tidno)
{
        struct rtl_atx_tid *txtid;

        if (!(rtl_priv->sc_flags & SC_OP_TXAGGR))
                return false;

        txtid = RTL_ND_2_TID(node, tidno);

        if (!(txtid->state & AGGR_ADDBA_COMPLETE)) {
                if (!(txtid->state & AGGR_ADDBA_PROGRESS) &&
                    (txtid->addba_exchangeattempts < ADDBA_EXCHANGE_ATTEMPTS)) {
                        txtid->addba_exchangeattempts++;
                        return true;
                }
        }

        return false;
}

int rtl_tx_aggr_stop(struct mac80211_shared_priv *priv, struct ieee80211_sta *sta, u16 tid)
{
	struct priv_stainfo *node = (struct priv_stainfo *)sta->drv_priv;
	struct rtl_atx_tid *txtid = RTL_ND_2_TID(node, tid);
	/* brian, should be removed
	struct ath_txq *txq = &sc->tx.txq[txtid->priv->qnum];
	struct ath_buf *bf;
	*/
	struct list_head bf_head;
	INIT_LIST_HEAD(&bf_head);

	if (txtid->state & AGGR_CLEANUP)
		return 0;

	if (!(txtid->state & AGGR_ADDBA_COMPLETE)) {
        txtid->addba_exchangeattempts = 0;
        return 0;
	}

	//rtl_tx_pause_tid(sc, txtid);

	/* drop all software retried frames and mark this TID */
	/*spin_lock_bh(&txq->axq_lock);
	while (!list_empty(&txtid->buf_q)) {
        bf = list_first_entry(&txtid->buf_q, struct ath_buf, list);
        if (!bf_isretried(bf)) {
				*
                 * NB: it's based on the assumption that
                 * software retried frame will always stay
                 * at the head of software queue.
                 *
                break;
        }
        list_move_tail(&bf->list, &bf_head);
        ath_tx_update_baw(sc, txtid, bf->bf_seqno);
        ath_tx_complete_buf(sc, bf, &bf_head, 0, 0);
	}
	spin_unlock_bh(&txq->axq_lock);

	if (txtid->baw_head != txtid->baw_tail) {
        txtid->state |= AGGR_CLEANUP;
	} else */{
        txtid->state &= ~AGGR_ADDBA_COMPLETE;
        txtid->addba_exchangeattempts = 0;
		//ath_tx_flush_tid(sc, txtid);
	}

}



#ifdef RTL8192SE
#define RECORD_FREE_INFO(priv, pdescinfo, buf) \
	do { \
		pdescinfo->type = buf->type; \
		if (buf->skb) \
			pdescinfo->pframe = buf->skb; \
		else \
			pdescinfo->pframe = buf->data; \
	} while (0);
#ifdef NEW_MAC80211_DRV
static void fill_next_desc(struct mac80211_shared_priv *priv,	struct tx_desc  *pdesc,
								struct tx_desc_info *pdescinfo, struct tx_buffer *buf)
#else
static void fill_next_desc(struct rtl8190_priv *priv, struct tx_desc  *pdesc,
								struct tx_desc_info *pdescinfo, struct tx_buffer *buf)
#endif
{
	memset(pdesc, 0, 32); 
	pdesc->Dword0 = set_desc((32 << TX_OFFSETSHIFT) | TX_OWN); 
	pdesc->Dword7 = set_desc(buf->len & TX_TxBufferSizeMask); 
	pdesc->Dword8 = set_desc(get_physical_addr(priv, buf->data, buf->len, PCI_DMA_TODEVICE)); 

	RECORD_FREE_INFO(priv, pdescinfo, buf);
#ifndef NEW_MAC80211_DRV 
	pdescinfo->priv = priv;
#endif
}

// the function should be called in critical section
static __inline__ void txdesc_rollover(struct tx_desc *ptxdesc, unsigned int *ptxhead)
{
	*ptxhead = (*ptxhead + 1) % NUM_TX_DESC;
}


static __inline__ void tx_poll(struct mac80211_shared_priv *priv, int q_num)
{
	unsigned long ioaddr = priv->pshare_hw->ioaddr;

	switch (q_num) {
	case MGNT_QUEUE:
		RTL_W16(TPPoll, TPPoll_MQ);
		break;
	case BK_QUEUE:
		RTL_W16(TPPoll, TPPoll_BKQ);
		break;
	case BE_QUEUE:
		RTL_W16(TPPoll, TPPoll_BEQ);
		break;
	case VI_QUEUE:
		RTL_W16(TPPoll, TPPoll_VIQ);
		break;
	case VO_QUEUE:
		RTL_W16(TPPoll, TPPoll_VOQ);
		break;
	case HIGH_QUEUE:
		RTL_W16(TPPoll, TPPoll_HQ);
		break;
	default:
		break;
	}
}



/* 
  *  Output a single MPDU to air (including 802.11 header and payload).
  *   
  *  Caller must be responsible to fill 802.11 header. In this routime, it 
  *  will not alter any header or payload.
  *  Also, caller must do packet segmentation and fill IV when necessary.
  *
  */

#ifdef CONFIG_MIPS
__MIPS16
__IRAM_FASTEXTDEV
#endif

#ifdef NEW_MAC80211_DRV
void fill_tx_desc(struct mac80211_shared_priv *priv, struct tx_info *txinfo)
#else
void fill_tx_desc(struct rtl8190_priv *priv, struct tx_info *txinfo)
#endif
{
	int *tx_head, q_num, q_select, *tx_tail, i, totol_len;
	struct rtl8190_hw	*phw;	
	struct tx_desc  *phdesc, *pdesc, *pfrstdesc;
	struct tx_desc_info *pswdescinfo, *pdescinfo;
#if !defined(NEW_MAC80211_DRV) && defined(TX_SHORTCUT)
	struct tx_desc_info *pfrstdescinfo;
#endif

	unsigned char *pwlhdr;	
	static struct tx_desc  *pfrstdesc_frag;	/* first desc of flagment list */

	ASSERT(txinfo->buf_num > 0);
	ASSERT(txinfo->tx_rate >= CCK_1M && txinfo->tx_rate <= MCS15);
	ASSERT(txinfo->q_num <= HIGH_QUEUE);
	ASSERT(txinfo->priority <= 7);
	
	q_num = txinfo->q_num;
	phw	= GET_HW(priv);
	tx_head = get_txhead_addr(phw, q_num);
	tx_tail = get_txtail_addr(phw, q_num);
	
//	printk("head %u, tail %u <<<>>> %x %x\n", *tx_head, *tx_tail, tx_head, tx_tail);

	phdesc = get_txdesc(phw, q_num);	
	pswdescinfo = get_txdesc_info(priv->pshare_hw->pdesc_info, q_num);	
	pwlhdr = txinfo->buf[0].data;

	for (totol_len=0, i=0; i<txinfo->buf_num; i++) {
		ASSERT(txinfo->buf[i].len > 0 && txinfo->buf[i].data != NULL);
		totol_len += txinfo->buf[i].len;
		rtl_cache_sync_wback(priv, (unsigned int)txinfo->buf[i].data, txinfo->buf[i].len, PCI_DMA_TODEVICE);
	}

	
	if (CIRC_SPACE_RTK(*tx_head, *tx_tail, NUM_TX_DESC) <1)
	{
		printk("space < 1, free skb\n");
		for (i=0; i<txinfo->buf_num; i++)
			if(txinfo->buf[i].type == _SKB_FRAME_TYPE_)
				dev_kfree_skb_any((struct sk_buff *)txinfo->buf[i].skb);
			else
				printk("unknown type!!!\n");
		return;
	}
	
/*
	if(!IS_MCAST(GetAddr1Ptr(pwlhdr)))
	{
		printk("pkt content(%d)\n", txcfg->desc1_len);
		printHex(pwlhdr, txcfg->desc1_len);
		printk("\n");
//		dev_kfree_skb_any((struct sk_buff *)txcfg->desc1_skb);
//		return;
	}
*/

	pdesc = phdesc + *tx_head;
	pdescinfo = pswdescinfo + *tx_head;

	memset(pdesc, 0, 32);
	pdesc->Dword3 |= set_desc((GetSequence(pwlhdr) & TX_SeqMask)  << TX_SeqSHIFT);

	if (txinfo->is_40m_bw) {
		pdesc->Dword4 |= set_desc(TX_TxBw);	
		pdesc->Dword4 |= set_desc((txinfo->side_band & SIDE_BAND_MASK) << TX_TXSCSHIFT);
	}

	if (txinfo->tx_rate >= MCS0) {
		pdesc->Dword4 |= set_desc(TX_TXHT);
		//if (txinfo->aggre_mthd >= AGGRE_AMPDU) {
		if (txinfo->aggre_mthd > AGGRE_AMPDU) {
			pdesc->Dword2 |= set_desc(TX_AggEn);
			
			if (priv->pshare_hw->last_ht_dst_cached &&
					memcmp(get_da(txinfo->buf[0].data), priv->pshare_hw->last_ht_dst_mac, 6))
				pdesc->Dword2 |= set_desc(TX_BK);								

			memcpy(priv->pshare_hw->last_ht_dst_mac, get_da(txinfo->buf[0].data), 6);
			priv->pshare_hw->last_ht_dst_cached = 1;
		}
		else
			priv->pshare_hw->last_ht_dst_cached = 0;

		if (txinfo->stbc)
			pdesc->Dword4 |= set_desc(1 << TX_STBCSHIFT);

		if (txinfo->short_gi)
			txinfo->tx_rate = 0x1c;	/* may need to be revised in new chip */		
	}
	else	 {
		if (txinfo->short_preamble)
			pdesc->Dword4 |= set_desc(TX_TxShort);
	}

	pdesc->Dword5 |= set_desc((txinfo->tx_rate & TX_TxRateMask) << TX_TxRateSHIFT);

	if (txinfo->need_ack) {
		if (txinfo->rts & RTS_ENABLE) {
			pdesc->Dword4 |=set_desc(TX_RTSEn);

			if (txinfo->rts & RTS_SHORT_PREAMBLE)
				pdesc->Dword4 |= set_desc(TX_RTSShort);

			pdesc->Dword4 |= set_desc((txinfo->rts_rate & TX_RTSRateMask) << TX_RTSRateSHIFT);
			pdesc->Dword4 |= set_desc((txinfo->rts_retry & TX_RTSRateFBLmtMask) << TX_RTSRateFBLmtSHIFT);	

			if (txinfo->rts_bw)
				pdesc->Dword4 |= set_desc(TX_RTSBW); /* dupplicated */
		}
		
		if (txinfo->cts2self)
			pdesc->Dword4 |= set_desc(TX_CTS2Self);

		if (txinfo->retry != -1) {
			pdesc->Dword2 |= set_desc(TX_RetryLmtEn);
			pdesc->Dword2 |= set_desc(
			(txinfo->retry & TX_DataRetryLmtMask) << TX_DataRetryLmtSHIFT);
		}
	}
	
	pdesc->Dword0 |= set_desc((32 << TX_OFFSETSHIFT) |
			((totol_len & TX_PktSizeMask) << TX_PktSizeSHIFT) |TX_FirstSeg);
	pdesc->Dword7 |= set_desc((txinfo->buf[0].len & TX_TxBufferSizeMask)
												<< TX_TxBufferSizeSHIFT);
	switch (q_num) {
#ifdef SW_MCAST
	case VO_QUEUE:
		q_select = 6;
		break;

	case VI_QUEUE:
		q_select = 5;
		break;
#else
	case HIGH_QUEUE:
		q_select = 0x11;
		break;
#endif
	case MGNT_QUEUE:
		q_select = 0x12;
		break;

	default: 
		q_select = txinfo->priority;
		break;
	}

	pdesc->Dword1 |= set_desc(((q_select & TX_QueueSelMask) 
						<< TX_QueueSelSHIFT) | (txinfo->aid & TX_MACIDMask));

	if (txinfo->more_frag & IS_FRAG) 
		pdesc->Dword1 |= set_desc(TX_MoreFrag);
	
	pdesc->Dword5 |= set_desc(0x1f << TX_DataRateFBLmtSHIFT);

	if (txinfo->fixed_rate) {
		pdesc->Dword4 |= set_desc(TX_UserRate);
		pdesc->Dword5 |= set_desc(TX_DISFB);
		pdesc->Dword4 |= set_desc(TX_DisRTSFB); /* disable RTS fall-back */
	}

#ifdef STA_EXT
	if (txinfo->aid == FW_NUM_STAT-1)
		pdesc->Dword4 |= set_desc(TX_UserRate);
#endif

	if (txinfo->hw_crypto) {
		switch(txinfo->hw_crypto) {				
		case _WEP_104_PRIVACY_:
		case _WEP_40_PRIVACY_:
			pdesc->Dword1 |= set_desc(0x1 << TX_SecTypeSHIFT);
			break;

		case _TKIP_PRIVACY_:
			pdesc->Dword1 |= set_desc(0x2 << TX_SecTypeSHIFT);
			break;

		case _CCMP_PRIVACY_:
			pdesc->Dword1 |= set_desc(0x3 << TX_SecTypeSHIFT);
			break;

		default:
			printk(KERN_ERR "Unknown privacy\n");
			break;
		}
	}

	pdesc->Dword8 = set_desc(get_physical_addr(priv, pwlhdr,
										txinfo->buf[0].len, PCI_DMA_TODEVICE));

	RECORD_FREE_INFO(priv, pdescinfo, (&txinfo->buf[0]))
	pfrstdesc = pdesc;

#ifndef NEW_MAC80211_DRV
	pdescinfo->priv = priv; 
#ifdef TX_SHORTCUT
	pfrstdescinfo = pdescinfo;
#endif
#endif

	if (txinfo->buf_num > 1) {
		for (i=1; i<txinfo->buf_num; i++) {
			if (txinfo->buf[i].len > 0 && txinfo->buf[i].data) {
		txdesc_rollover(pdesc, (unsigned int *)tx_head);
		pdesc = phdesc + *tx_head;
		pdescinfo = pswdescinfo + *tx_head;
				fill_next_desc(priv, pdesc, pdescinfo, &txinfo->buf[i]);
			
				if (i < txinfo->buf_num-1)
		rtl_cache_sync_wback(priv, (unsigned int)pdesc, sizeof(struct tx_desc), PCI_DMA_TODEVICE);
			}		
		}
	}
	
#ifdef SUPPORT_TX_AMSDU
	if (txinfo->aggre_mthd != AGGRE_AMSDU)
#endif
		pdesc->Dword0 |= set_desc(TX_LastSeg);
#if !defined(NEW_MAC80211_DRV) && defined(TX_SHORTCUT)
	if (txinfo->pstat) {
		desc_copy(&txinfo->pstat->hwdesc1, pfrstdesc);
		descinfo_copy(&txinfo->pstat->swdesc1, pfrstdescinfo);
		desc_copy(&txinfo->pstat->hwdesc2, pdesc);
		descinfo_copy(&txinfo->pstat->swdesc2, pdescinfo);
	}

#endif

		//pdesc->Dword0 |= set_desc(TX_LastSeg);
	rtl_cache_sync_wback(priv, (unsigned int)pdesc, sizeof(struct tx_desc), PCI_DMA_TODEVICE);	
	txdesc_rollover(pdesc, (unsigned int *)tx_head);
	
#ifdef SUPPORT_TX_AMSDU
	if (txinfo->aggre_mthd == AGGRE_AMSDU) {
		priv->amsdu_first_desc = pfrstdesc;
		priv->amsdu_len = totol_len;
		return;
	}
#endif
	
	if (txinfo->more_frag == NONE_FRAG) {
		pfrstdesc->Dword0 |= set_desc(TX_OWN);
		rtl_cache_sync_wback(priv, (unsigned int)pfrstdesc, sizeof(struct tx_desc), PCI_DMA_TODEVICE);					
	}
	else {	
		if (txinfo->more_frag & IS_FIRST_FRAG) 
		pfrstdesc_frag = pfrstdesc;	
	else {
		pfrstdesc->Dword0 |= set_desc(TX_OWN);
		//if (txcfg->need_ack == 0){
			printk("tx Dw0 >> %08x\n", pdesc->Dword0);
			printk("tx Dw1 >> %08x\n", pdesc->Dword1);
			printk("tx Dw2 >> %08x\n", pdesc->Dword2);
			printk("tx Dw3 >> %08x\n", pdesc->Dword3);
			printk("tx Dw4 >> %08x\n", pdesc->Dword4);
			printk("tx Dw5 >> %08x\n", pdesc->Dword5);
			printk("tx Dw6 >> %08x\n", pdesc->Dword6);
			printk("tx Dw7 >> %08x\n", pdesc->Dword7);
			printk("tx Dw8 >> %08x\n", pdesc->Dword8);
		
		/*}
		else {
			//pfrstdesc->Dword2 = 0;
			pfrstdesc->Dword0 |= set_desc(TX_OWN);
			printk("Dw0 >> %08x\n", pdesc->Dword0);
			printk("Dw1 >> %08x\n", pdesc->Dword1);
			printk("Dw2 >> %08x\n", pdesc->Dword2);
			printk("Dw3 >> %08x\n", pdesc->Dword3);
			printk("Dw4 >> %08x\n", pdesc->Dword4);
			printk("Dw5 >> %08x\n", pdesc->Dword5);
			printk("Dw6 >> %08x\n", pdesc->Dword6);
			printk("Dw7 >> %08x\n", pdesc->Dword7);
			printk("Dw8 >> %08x\n", pdesc->Dword8);
		}
		*/
		rtl_cache_sync_wback(priv, (unsigned int)pfrstdesc, sizeof(struct tx_desc), PCI_DMA_TODEVICE);
	}

		if (txinfo->more_frag & IS_LAST_FRAG) {
                        pfrstdesc_frag->Dword0 |= set_desc(TX_OWN);
                        rtl_cache_sync_wback(priv, (unsigned int)pfrstdesc_frag, sizeof(struct tx_desc), PCI_DMA_TODEVICE);		
                }	
	}
	
	if (txinfo->more_frag == NONE_FRAG || txinfo->more_frag == IS_LAST_FRAG) {
#ifndef SW_MCAST
		if (q_num == HIGH_QUEUE) 
			priv->pkt_in_dtimQ = 1;
		else
#endif
			tx_poll(priv, q_num);
	
	}
}
#if !defined(NEW_MAC80211_DRV) && defined(TX_SHORTCUT)
__MIPS16
__IRAM_IN_865X_HI
void fill_tx_desc_shortcut(struct rtl8190_priv *priv, struct tx_insn *txcfg)
{
	struct tx_desc *phdesc, *pdesc, *pfrstdesc;
	struct tx_desc_info *pswdescinfo, *pdescinfo;
	struct rtl8190_hw	*phw;
	int *tx_head, q_num;
	struct stat_info *pstat;
	struct sk_buff *pskb;

	pstat = txcfg->pstat;
	pskb = (struct sk_buff *)txcfg->pframe;

	phw	= GET_HW(priv);
	q_num = txcfg->q_num;

	tx_head = get_txhead_addr(phw, q_num);
	phdesc = get_txdesc(phw, q_num);
	pswdescinfo = get_txdesc_info(priv->pshare->pdesc_info, q_num);

	pfrstdesc = pdesc = phdesc + *tx_head;
	pdescinfo = pswdescinfo + *tx_head;
	desc_copy(pdesc, &pstat->hwdesc1);

	assign_wlanseq(GET_HW(priv), txcfg->phdr, pstat, GET_MIB(priv)
#ifdef CONFIG_RTK_MESH
	, txcfg->is_11s
#endif
		);

	pdesc->Dword3 = set_desc((GetSequence(txcfg->phdr) & TX_SeqMask) << TX_SeqSHIFT);
	if (get_desc(pdesc->Dword2) & TX_AggEn) {
		pdesc->Dword2 |= set_desc(TX_AggEn);
			
		if (priv->pshare->last_ht_dst_cached &&
			memcmp(get_da(txcfg->phdr), priv->pshare->last_ht_dst_mac, 6)) 			
			pdesc->Dword2 |= set_desc(TX_BK);	
		else
			pdesc->Dword2 &= set_desc(~TX_BK);
		
		memcpy(priv->pshare->last_ht_dst_mac, get_da(txcfg->phdr), 6);
		priv->pshare->last_ht_dst_cached = 1;
	}
	else 
		priv->pshare->last_ht_dst_cached = 0;		

	if (txcfg->one_txdesc) {
		pdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & 0xffff0000) |
			TX_LastSeg | (txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len));
		pdesc->Dword7 = set_desc((get_desc(pdesc->Dword7) & 0xffff0000) |
			(txcfg->hdr_len + txcfg->llc + txcfg->iv + txcfg->fr_len));
	}

	pdesc->Dword8 = set_desc(get_physical_addr(priv, txcfg->phdr,
		(get_desc(pdesc->Dword7)& TX_TxBufferSizeMask), PCI_DMA_TODEVICE));

	descinfo_copy(pdescinfo, &pstat->swdesc1);
	pdescinfo->paddr  = get_desc(pdesc->Dword8);

	if (txcfg->one_txdesc) {
		pdescinfo->type = _SKB_FRAME_TYPE_;
		pdescinfo->pframe = pskb;
		pdescinfo->priv = priv;
#if defined(SEMI_QOS) && defined(WMM_APSD)
		pdescinfo->pstat = txcfg->pstat;
#endif
	}
	else {
		pdescinfo->pframe = txcfg->phdr;
#if defined(SEMI_QOS) && defined(WMM_APSD)
		pdescinfo->priv = priv;
		pdescinfo->pstat = txcfg->pstat;
#endif
	}

	txdesc_rollover(pdesc, (unsigned int *)tx_head);
	if (txcfg->one_txdesc)
		goto one_txdesc;

	pdesc = phdesc + *tx_head;
	pdescinfo = pswdescinfo + *tx_head;
	desc_copy(pdesc, &pstat->hwdesc2);

	pdesc->Dword8 = set_desc(get_physical_addr(priv, pskb->data,
		(get_desc(pdesc->Dword7)&0x0fff), PCI_DMA_TODEVICE));

	descinfo_copy(pdescinfo, &pstat->swdesc2);
	pdescinfo->paddr  = get_desc(pdesc->Dword8);
	pdescinfo->pframe = pskb;
	pdescinfo->priv = priv;

	txdesc_rollover(pdesc, (unsigned int *)tx_head);

#if defined(CONFIG_RTL_WAPI_SUPPORT)
	if (txcfg->privacy == _WAPI_SMS4_)	
		SecSWSMS4Encryption(priv, txcfg);
#endif

	rtl_cache_sync_wback(priv, get_desc(pdesc->Dword8), (get_desc(pdesc->Dword7)&TX_TxBufferSizeMask), PCI_DMA_TODEVICE);

one_txdesc:
	rtl_cache_sync_wback(priv, get_desc(pfrstdesc->Dword8), (get_desc(pfrstdesc->Dword7)&TX_TxBufferSizeMask), PCI_DMA_TODEVICE);

#ifdef SUPPORT_SNMP_MIB
	if (txcfg->rts_thrshld <= get_mpdu_len(txcfg, txcfg->fr_len))
		SNMP_MIB_INC(dot11RTSSuccessCount, 1);
#endif

	pfrstdesc->Dword0 |= set_desc(TX_OWN);

	if (q_num == HIGH_QUEUE) {
		DEBUG_WARN("signin shortcut for DTIM pkt?\n");
		return;
	}
	else
		tx_poll(priv, q_num);
} 
#endif // !defined(NEW_MAC80211_DRV) && defined(TX_SHORTCUT)

#ifdef SUPPORT_TX_AMSDU
/*  
 *   Fill Tx desc for AMSDU. 
  *  Please note, first AMSDU will still go to fill_tx_desc(). Other AMSDU 
  *  fragments will go to here. When data come to here, it will only carry
  *  one buffer because no wlan header needed.
  */
void fill_tx_desc_amsdu(struct rtl8190_priv *priv, struct tx_insn* txcfg)
{
	struct tx_desc *phdesc, *pdesc, *pfrstdesc;
	struct tx_desc_info *pswdescinfo, *pdescinfo;
	unsigned int  tx_len;
	int *tx_head, q_num;
	unsigned long	tmpphyaddr;
	unsigned char *pbuf;
	struct rtl8190_hw *phw;
	unsigned int *dma_txhead;

	q_num = txcfg->q_num;
	phw	= GET_HW(priv);

	dma_txhead	= get_txdma_addr(phw, q_num);
	tx_head		= get_txhead_addr(phw, q_num);
	phdesc   	= get_txdesc(phw, q_num);
	pswdescinfo = get_txdesc_info(priv->pshare->pdesc_info, q_num);

	pbuf = ((struct sk_buff *)txcfg->pframe)->data;
	tx_len = ((struct sk_buff *)txcfg->pframe)->len;
	tmpphyaddr = get_physical_addr(priv, pbuf, tx_len, PCI_DMA_TODEVICE);
	
	rtl_cache_sync_wback(priv, (unsigned int)pbuf, tx_len, PCI_DMA_TODEVICE);

	pdesc     = phdesc + (*tx_head);
	pdescinfo = pswdescinfo + *tx_head;

	memset(pdesc, 0, 32);

	pdesc->Dword8 = set_desc(tmpphyaddr); 
	pdesc->Dword7 |= (set_desc(tx_len & TX_TxBufferSizeMask));
	pdesc->Dword0 |= set_desc(32 << TX_OFFSETSHIFT);

	if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST) 
		pdesc->Dword0 = set_desc(TX_OWN | TX_LastSeg);
	else
		pdesc->Dword0 = set_desc(TX_OWN);

	pdescinfo->type = _SKB_FRAME_TYPE_;
	pdescinfo->paddr = get_desc(pdesc->Dword8); 
	pdescinfo->pframe = txcfg->pframe;
	pdescinfo->len = txcfg->fr_len;
	pdescinfo->priv = priv;

	txdesc_rollover(pdesc, (unsigned int *)tx_head);

	priv->amsdu_len += tx_len;

	if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST) {
		pfrstdesc = priv->amsdu_first_desc;
		pfrstdesc->Dword0 = set_desc((get_desc(pfrstdesc->Dword0) &0xff0000) | priv->amsdu_len | TX_FirstSeg | TX_OWN);
		tx_poll(priv, q_num);
	}
}
#endif // SUPPORT_TX_AMSDU
#endif

#ifdef RTL8192SU

void fill_tx_desc(struct mac80211_shared_priv *priv, struct tx_info *txinfo)
{
	struct sk_buff *skb;

	ASSERT(txinfo->buf_num > 0);
	ASSERT(txinfo->tx_rate >= CCK_1M && txinfo->tx_rate <= MCS15);
	ASSERT(txinfo->q_num <= HIGH_QUEUE);
	ASSERT(txinfo->priority <= 7);

	skb = txinfo->buf[0].skb;

	rtl8192s_usb_xmit(priv, skb, txinfo);


}
#endif


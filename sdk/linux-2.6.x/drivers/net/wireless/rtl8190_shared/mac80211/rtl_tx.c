/*
 *  TX handle routines
 *
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
 


//#include <linux/module.h>

#include "../8190n_cfg.h"
#include "../8190n.h"
#include "../8190n_hw.h"
#include "../8190n_headers.h"
#include "../8190n_debug.h"

#include <linux/etherdevice.h>
#ifdef NEW_HAL_API
#include "../rtl80211.h"
#endif

#define RET_AGGRE_BYPASS		0
#define RET_AGGRE_ENQUE			1
#define RET_AGGRE_DESC_FULL		2

#define TX_NORMAL				0
#define TX_NO_MUL2UNI			1
#define TX_AMPDU_BUFFER_SIG		2
#define TX_AMPDU_BUFFER_FIRST	3
#define TX_AMPDU_BUFFER_MID		4
#define TX_AMPDU_BUFFER_LAST	5

int __rtl80211_tx(struct ieee80211_hw *hw, struct sk_buff *skb, int tx_mthd);

#define PRI_TO_QNUM(priority, q_num, wifi_specific) { \
		if ((priority == 0) || (priority == 3)) \
			q_num = BE_QUEUE; \
		else if ((priority == 7) || (priority == 6)) \
			q_num = VO_QUEUE; \
		else if ((priority == 5) || (priority == 4)) \
			q_num = VI_QUEUE; \
		else  \
			q_num = BK_QUEUE; \
		}

static unsigned int get_skb_priority(struct mac80211_shared_priv *priv, struct sk_buff *skb)
{
	unsigned int pri=0, parsing=0;
	unsigned char protocol[2];

#ifdef SEMI_QOS
	if (QOS_ENABLE)
		parsing = 1;
#endif

	if (parsing) {
#ifdef CONFIG_RTK_VLAN_SUPPORT
		if (skb->cb[0]) 
			pri =  skb->cb[0];		
		else
#endif
		{		
		protocol[0] = skb->data[12];
		protocol[1] = skb->data[13];

		if ((protocol[0] == 0x08) && (protocol[1] == 0x00))
		{
#ifdef CONFIG_RTK_MESH
			if(is_11s & 8)
			{
				if(skb->data[14]&1) // 6 addr
					pri = (skb->data[31] & 0xe0) >> 5;
				else
					pri = (skb->data[19] & 0xe0) >> 5;
			}
			else
#endif
				pri = (skb->data[15] & 0xe0) >> 5;
		}
		else if ((skb->cb[0]>0) && (skb->cb[0]<8))	// Ethernet driver will parse priority and put in cb[0]
			pri = skb->cb[0];
		else
			pri = 0;
		}
		skb->cb[1] = pri;

		return pri;
	}
	else {
		// default is no priority
		skb->cb[1] = 0;
		return 0;
	}
}


#ifdef BUFFER_TX_AMPDU
#ifdef NEW_MAC80211_DRV
static int rtl_aggr_query(struct mac80211_shared_priv *priv, struct priv_stainfo *node, u8 tidno)
{
		struct rtl_atx_tid *tid;
        tid = RTL_ND_2_TID(node, tidno);

        if (tid->state & AGGR_ADDBA_COMPLETE ||
            tid->state & AGGR_ADDBA_PROGRESS)
                return 1;
        else
                return 0;
}

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


static void assign_aggr_tid_seqno(struct sk_buff *skb,
                                  struct ieee80211_sta *sta)
{
    struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
    struct ieee80211_hdr *hdr;
	struct priv_stainfo *node = (struct priv_stainfo *)sta->drv_priv;
    struct rtl_atx_tid *tid;
	int tidno;
    __le16 fc;
    u8 *qc;

    if (!tx_info->control.sta)
            return;

    hdr = (struct ieee80211_hdr *)skb->data;
    fc = hdr->frame_control;

	if (ieee80211_is_data_qos(fc)) {
	    qc = ieee80211_get_qos_ctl(hdr);
		tidno = qc[0] & 0xf;
	}

	/*
	     * For HT capable stations, we save tidno for later use.
	     * We also override seqno set by upper layer with the one
	     * in tx aggregation state.
	     *
	     * If fragmentation is on, the sequence number is
	     * not overridden, since it has been
	     * incremented by the fragmentation routine.
	     *
	     * FIXME: check if the fragmentation threshold exceeds
	     * IEEE80211 max.
	     */
	    tid = RTL_ND_2_TID(node, tidno);
	    hdr->seq_ctrl = cpu_to_le16(tid->seq_next << IEEE80211_SEQ_SEQ_SHIFT);
	    INCR(tid->seq_next, IEEE80211_SEQ_MAX);
}
#endif	//NEW_MAC80211_DRV

static int ampdu_xmit(struct mac80211_shared_priv *priv, struct priv_stainfo *pstat, struct net_device *dev, int tid, int from_timeout)
{
	int q_num, is_first=1, total_len=0, total_num=0;
	struct sk_buff *pskb;
	unsigned long	flags;
	int *tx_head, *tx_tail, space=0, tx_flag;
	struct rtl8190_hw	*phw = GET_HW(priv);

	PRI_TO_QNUM(tid, q_num, priv->pmib->dot11OperationEntry.wifi_specific);

	tx_head = get_txhead_addr(phw, q_num);
	tx_tail = get_txtail_addr(phw, q_num);

	// start to transmit queued skb
	SAVE_INT_AND_CLI(flags);

	/*
	printk("[ampdu_xmit]pstat:%x\n",pstat);
	printk("ampdu_tx_que[%d] length:%d\n",tid,pstat->ampdu_tx_que[tid].qlen);
	*/
	while (skb_queue_len(&pstat->ampdu_tx_que[tid]) > 0) {
		pskb = __skb_dequeue(&pstat->ampdu_tx_que[tid]);
		if (pskb == NULL)
			break;

		total_len += pskb->len;

		if (is_first) {
			if (skb_queue_len(&pstat->ampdu_tx_que[tid]) > 0) { // next one is avaiable
				space = CIRC_SPACE_RTK(*tx_head, *tx_tail, NUM_TX_DESC);
				if (space < 10) {
					//rtl8190_tx_dsr((unsigned long)priv);
					space = CIRC_SPACE_RTK(*tx_head, *tx_tail, NUM_TX_DESC);
					if (space < 10) {
						printk("Tx desc not enough for A-MPDU!\n");
						__skb_queue_head(&pstat->ampdu_tx_que[tid], pskb);
						RESTORE_INT(flags);
						return 0;
					}
				}

				tx_flag = TX_AMPDU_BUFFER_FIRST;
				is_first = 0;
				total_num = 1;
				//printk("ampdu_xmit first\n");
			}
			else { // next one is empty
				if (!from_timeout) {
					__skb_queue_head(&pstat->ampdu_tx_que[tid], pskb);
					RESTORE_INT(flags);
					//printk("ampdu_xmit back\n");
					return 0;
				}
				// timeout, and only one skb, tx it as normal pkt
				tx_flag = TX_AMPDU_BUFFER_SIG;
				//printk("ampdu_xmit single\n");
			}
		}
		else if ((skb_queue_len(&pstat->ampdu_tx_que[tid]) == 0) ||
						(total_len > priv->pshare_hw->rf_ft_var.dot11nAMPDUBufferMax) ||
							(space < ((total_num+1)*2 +4)) ||
							((total_num+1) >= priv->pshare_hw->rf_ft_var.dot11nAMPDUBufferNum)) {
			tx_flag = TX_AMPDU_BUFFER_LAST;
			total_len = 0;
			is_first = 1;
			total_num++;
			//printk("ampdu_xmit last\n");
		}
		else {
			tx_flag = TX_AMPDU_BUFFER_MID;
			total_num++;
			//printk("ampdu_xmit mid\n");
		}

		pstat->ampdu_size[tid] -= pskb->len;

		__rtl80211_tx(priv->hw, pskb, tx_flag);
		/* printk("__rtl80211_tx with flag:%d\n",tx_flag); */
	}
	RESTORE_INT(flags);

	return 1;
}


static int ampdu_timer_add(struct mac80211_shared_priv *priv, struct priv_stainfo *pstat, int tid, int from_timeout)
{
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	unsigned int now, timeout, new_timer=0;
	int setup_timer;
	int current_idx, next_idx;

	current_idx = priv->pshare_hw->ampdu_timer_head;

	while (CIRC_CNT(current_idx, priv->pshare_hw->ampdu_timer_tail, AMPDU_TIMER_NUM)) {
		if (priv->pshare_hw->ampdu_timer[priv->pshare_hw->ampdu_timer_tail].pstat == NULL) {
			priv->pshare_hw->ampdu_timer_tail = (priv->pshare_hw->ampdu_timer_tail + 1) & (AMPDU_TIMER_NUM - 1);
			new_timer = 1;
			/* printk("ampdu_timer_add new_timer\n"); */
		}
		else
			break;
	}

	if (CIRC_CNT(current_idx, priv->pshare_hw->ampdu_timer_tail, AMPDU_TIMER_NUM) == 0) {
		cancel_timer1(priv);
		setup_timer = 1;
		/* printk("ampdu_timer_add setup_timer\n"); */
	}
	else if (CIRC_SPACE(current_idx, priv->pshare_hw->ampdu_timer_tail, AMPDU_TIMER_NUM) == 0) {
		printk("%s: %s, ampdu timer overflow!\n", priv->dev->name, __FUNCTION__ );
		return -1;
	}
	else {	// some items in timer queue
		setup_timer = 0;
		if (new_timer)
			new_timer = priv->pshare_hw->ampdu_timer[priv->pshare_hw->ampdu_timer_tail].timeout;
	}

	next_idx = (current_idx + 1) & (AMPDU_TIMER_NUM - 1);

	priv->pshare_hw->ampdu_timer[current_idx].priv = priv;
	priv->pshare_hw->ampdu_timer[current_idx].pstat = pstat;
	priv->pshare_hw->ampdu_timer[current_idx].tid = (unsigned char)tid;
	priv->pshare_hw->ampdu_timer_head = next_idx;

	now = RTL_R32(_TSFTR_L_);
	//timeout = now + priv->pshare_hw->rf_ft_var.dot11nAMPDUBufferTimeout;
	timeout = now + 400;
	priv->pshare_hw->ampdu_timer[current_idx].timeout = timeout;

	if (!from_timeout) {
		if (setup_timer)
			setup_timer1(priv, timeout);
		else if (new_timer) {
			if (TSF_LESS(new_timer, now))
				setup_timer1(priv, timeout);
			else
				setup_timer1(priv, new_timer);
		}
	}
	/* printk("setup timer start%d, out:%d\n",now,timeout); */

	return current_idx;
}


void ampdu_timeout(struct RTL_PRIV *priv, unsigned int current_time)
{
	struct priv_stainfo *pstat;
	struct net_device *dev_this;
	struct mac80211_shared_priv *priv_this=NULL;
	int tid=0, head;

	head = priv->pshare_hw->ampdu_timer_head;

	//printk("ampdu_timeout\n");

	while (CIRC_CNT(head, priv->pshare_hw->ampdu_timer_tail, AMPDU_TIMER_NUM))
	{
		pstat = priv->pshare_hw->ampdu_timer[priv->pshare_hw->ampdu_timer_tail].pstat;
		if (pstat) {
			tid = priv->pshare_hw->ampdu_timer[priv->pshare_hw->ampdu_timer_tail].tid;
			priv_this = priv->pshare_hw->ampdu_timer[priv->pshare_hw->ampdu_timer_tail].priv;
			priv->pshare_hw->ampdu_timer[priv->pshare_hw->ampdu_timer_tail].pstat = NULL;
		}

		priv->pshare_hw->ampdu_timer_tail = (priv->pshare_hw->ampdu_timer_tail + 1) & (AMPDU_TIMER_NUM - 1);

		if (pstat) {
			dev_this = priv_this->dev;
#ifdef WDS
			if (pstat->state & WIFI_WDS)
				dev_this = getWdsDevByAddr(priv, pstat->hwaddr);
#endif

			/*
			if (pstat->state & WIFI_SLEEP_STATE)
				pstat->ampdu_timer_id[tid] = ampdu_timer_add(priv_this, pstat, tid, 1) + 1;
			else
			*/
			if (ampdu_xmit(priv_this, pstat, dev_this, tid, 1) == 0) // not finish
				pstat->ampdu_timer_id[tid] = ampdu_timer_add(priv_this, pstat, tid, 1) + 1;
			else
				pstat->ampdu_timer_id[tid] = 0;
		}
	}

	if (CIRC_CNT(priv->pshare_hw->ampdu_timer_head, priv->pshare_hw->ampdu_timer_tail, AMPDU_TIMER_NUM)) {
		setup_timer1(priv, priv->pshare_hw->ampdu_timer[priv->pshare_hw->ampdu_timer_tail].timeout);
		if (TSF_LESS(priv->pshare_hw->ampdu_timer[priv->pshare_hw->ampdu_timer_tail].timeout, current_time))
			printk("Setup timer2 %d too late (now %d)\n", priv->pshare_hw->ampdu_timer[priv->pshare_hw->ampdu_timer_tail].timeout, current_time);
	}
}


static int ampdu_check(struct mac80211_shared_priv *priv,  struct sk_buff *skb, struct priv_stainfo *pstat)
{
	int q_num;
	unsigned int priority;
	unsigned short protocol;
	int *tx_head, *tx_tail, cnt, add_timer=1;
	struct rtl8190_hw	*phw;
	unsigned long flags;
	struct net_device *dev_this=priv->dev;

//DBFENTER;
	protocol = ntohs(*((UINT16 *)((UINT8 *)skb->data + ETH_ALEN*2)));
#if defined(CONFIG_RTL_WAPI_SUPPORT)
	if ((protocol == 0x888e)||(protocol == ETH_P_WAPI))
#else
	if (protocol == 0x888e)
#endif
	{
		return RET_AGGRE_BYPASS;
	}

	priority = get_skb_priority(priv, skb);
	PRI_TO_QNUM(priority, q_num, priv->mib->dot11OperationEntry.wifi_specific);

	phw = GET_HW(priv);
	tx_head = get_txhead_addr(phw, q_num);
	tx_tail = get_txtail_addr(phw, q_num);

	cnt = CIRC_CNT_RTK(*tx_head, *tx_tail, NUM_TX_DESC);

	if (cnt == (NUM_TX_DESC - 1))
		return RET_AGGRE_DESC_FULL;

	SAVE_INT_AND_CLI(flags);
	__skb_queue_tail(&pstat->ampdu_tx_que[priority], skb);
	pstat->ampdu_size[priority] += skb->len;
	/*
	printk("[ampdu_check]pstat:%x\n",pstat);
	printk("ampdu_tx_que[%d] length:%d\n",priority,pstat->ampdu_tx_que[priority].qlen);
	printk("ampdu_size[%d] length:%d\n",priority,pstat->ampdu_size[priority]);
	*/

	if (pstat->ampdu_size[priority] > priv->pshare_hw->rf_ft_var.dot11nAMPDUBufferMax) {
		// delete timer entry
		if (pstat->ampdu_timer_id[priority] > 0) {
			priv->pshare_hw->ampdu_timer[pstat->ampdu_timer_id[priority] - 1].pstat = NULL;
			pstat->ampdu_timer_id[priority] = 0;
		}

#if 0
//#ifdef WDS
		if (pstat->state & WIFI_WDS)
			dev_this = getWdsDevByAddr(priv, pstat->hwaddr);
#endif

		if (ampdu_xmit(priv, pstat, dev_this, priority, 0) == 0) { // not finish
			int tidno;
			u8 *qc;
			struct rtl_atx_tid *tid;
			
			qc = ieee80211_get_qos_ctl((struct ieee80211_hdr *)skb->data);
			tidno = qc[0] & 0xf;
			tid = RTL_ND_2_TID(pstat, tidno);

			if (tid->state & AGGR_ADDBA_COMPLETE){
				/*
				 * Try aggregation if it's a unicast data frame
				 * and the destination is HT capable.
				 */
				pstat->ampdu_timer_id[priority] = ampdu_timer_add(priv, pstat,  priority, 0) + 1;
			}
			else{
				add_timer = 0;
			}
		}
		else
			add_timer = 0;
	}

	if (add_timer) {
		if (pstat->ampdu_timer_id[priority] == 0)
			pstat->ampdu_timer_id[priority] = ampdu_timer_add(priv, pstat, priority, 0) + 1;
		/* printk("ampdu timer added\n"); */
	}
	RESTORE_INT(flags);

	/* printk("mpdu queued\n"); */

	return RET_AGGRE_ENQUE;
}

#endif // BUFFER_TX_AMPDU


#ifdef RTL8192SE
// copy from tx

//-----------------------------------------------------------------------------
// Procedure:	 Fill Tx Command Packet Descriptor
//
// Description:   This routine fill command packet descriptor. We assum that command packet
//				require only one descriptor.
//
// Arguments:	This function is only for Firmware downloading in current stage
//
// Returns:
//-----------------------------------------------------------------------------
int rtl8192SE_SetupOneCmdPacket(struct mac80211_shared_priv *priv, unsigned char *dat_content, unsigned short txLength, unsigned char LastPkt)
/*
	IN	PADAPTER		Adapter,
	IN	PRT_TCB 		pTcb,
	IN	u1Byte			QueueIndex,
	IN	u2Byte			index,
	IN	BOOLEAN 		bFirstSeg,
	IN	BOOLEAN 		bLastSeg,
	IN	pu1Byte 		VirtualAddress,
	IN	u4Byte			PhyAddressLow,
	IN	u4Byte			BufferLen,
	IN	BOOLEAN 			bSetOwnBit,
	IN	BOOLEAN 		bLastInitPacket,
	IN	  u4Byte			DescPacketType,
	IN	u4Byte			PktLen
	)
*/
{

	unsigned char	ih=0;
	unsigned char	DescNum;
	unsigned short	DebugTimerCount;

	struct tx_desc	*pdesc;
	struct tx_desc	*phdesc;
	volatile unsigned int *ppdesc  ; //= (unsigned int *)pdesc;
	int *tx_head, *tx_tail;
	struct rtl8190_hw	*phw = GET_HW(priv);

	tx_head = &phw->txcmdhead;
	tx_tail = &phw->txcmdtail;
	phdesc = phw->txcmd_desc;

	DebugTimerCount = 0; // initialize debug counter to exit loop
	DescNum = 1;

//TODO: Fill the dma check here

//	printk("data lens: %d\n", txLength );

	for (ih=0; ih<DescNum; ih++) {
		pdesc	   = (phdesc + (*tx_head));
		ppdesc = (unsigned int *)pdesc;
		// Clear all status
		memset(pdesc, 0, 36);
//		rtl_cache_sync_wback(priv, phw->txcmd_desc_dma_addr[*tx_head], 32, PCI_DMA_TODEVICE);
		// For firmware downlaod we only need to set LINIP
		if (LastPkt)
			pdesc->Dword0 |= set_desc(TX_LINIP);

		// From Scott --> 92SE must set as 1 for firmware download HW DMA error
		pdesc->Dword0 |= set_desc(TX_FirstSeg);;//bFirstSeg;
		pdesc->Dword0 |= set_desc(TX_LastSeg);;//bLastSeg;

		// 92SE need not to set TX packet size when firmware download
		pdesc->Dword7 |=  (set_desc((unsigned short)(txLength) << TX_TxBufferSizeSHIFT));

		memcpy(priv->pshare_hw->txcmd_buf, dat_content, txLength);

		rtl_cache_sync_wback(priv, (unsigned int)priv->pshare_hw->txcmd_buf, txLength, PCI_DMA_TODEVICE);


		pdesc->Dword8 =  set_desc(priv->pshare_hw->cmdbuf_phyaddr);

		pdesc->Dword0	|= set_desc((unsigned short)(txLength) << TX_PktIDSHIFT);
		//if (bSetOwnBit)
		{
			pdesc->Dword0 |= set_desc(TX_OWN);
//			*(ppdesc) |= set_desc(BIT(31));
		}

#ifndef USE_RTL8186_SDK
		rtl_cache_sync_wback(priv, phw->txcmd_desc_dma_addr[*tx_head], sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
		*tx_head = (*tx_head + 1) & (NUM_CMD_DESC - 1);
	}

	return TRUE;
}
		
/*
	Procedure to re-cycle TXed packet in Queue index "txRingIdx"

	=> Return value means if system need restart-TX-queue or not.

		1: Need Re-start Queue
		0: Don't Need Re-start Queue
*/
static int rtl8190_tx_recycle(struct mac80211_shared_priv *priv, unsigned int txRingIdx, int *recycleCnt_p)
{
//	unsigned long	flags;
	struct tx_desc	*pdescH, *pdesc;
	struct tx_desc_info *pdescinfoH, *pdescinfo;
	volatile int	head, tail;
	struct rtl8190_hw	*phw=GET_HW(priv);
	int				needRestartQueue=0;
	int				recycleCnt=0;

//	SAVE_INT_AND_CLI(flags);

	head		= get_txhead(phw, txRingIdx);
	tail		= get_txtail(phw, txRingIdx);
	pdescH		= get_txdesc(phw, txRingIdx);
	pdescinfoH	= get_txdesc_info(priv->pshare_hw->pdesc_info, txRingIdx);

	while (CIRC_CNT_RTK(head, tail, NUM_TX_DESC))
	{
		pdesc = pdescH + (tail);
		pdescinfo = pdescinfoH + (tail);

#ifdef __MIPSEB__
		pdesc = (struct tx_desc *)KSEG1ADDR(pdesc);
#endif

		if (!pdesc || (get_desc(pdesc->Dword0) & TX_OWN))
			break;

//#ifdef CONFIG_NET_PCI
		//if (IS_PCIBIOS_TYPE)

			//use the paddr and flen of pdesc field for icv, mic case which doesn't fill the pdescinfo
			pci_unmap_single(priv->pshare_hw->pdev,
							 get_desc(pdesc->Dword8),
							 (get_desc(pdesc->Dword7)&0xffff),
							 PCI_DMA_TODEVICE);

//#endif

		if (pdescinfo->type == _SKB_FRAME_TYPE_)
		{
/*
#ifdef MP_TEST

			if (OPMODE & WIFI_MP_CTX_BACKGROUND) {
				struct sk_buff *skb = (struct sk_buff *)(pdescinfo->pframe);
				skb->data = skb->head;
				skb->tail = skb->data;
				skb->len = 0;
				priv->pshare->skb_tail = (priv->pshare->skb_tail + 1) & (NUM_MP_SKB - 1);
			}
			else
#endif
*/
			{
#ifdef __LINUX_2_6__
//		rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
				
	#ifdef NEW_MAC80211_DRV
			{
				struct ieee80211_tx_info *info;
				struct ieee80211_hw *hw = priv->hw;
				struct sk_buff *skb = (struct sk_buff *)(pdescinfo->pframe);
				struct ieee80211_hdr *hdr  = (struct ieee80211_hdr *)skb->data;
				struct ieee80211_sta *sta = NULL;

				/* tx_status */
				info = IEEE80211_SKB_CB(skb);
				ieee80211_tx_info_clear_status(info);

				if (!(info->flags & IEEE80211_TX_CTL_NO_ACK) 
				    //&& (flags & RTL818X_TX_DESC_FLAG_TX_OK)
				    )
					info->flags |= IEEE80211_TX_STAT_ACK;

				ieee80211_tx_status(hw, skb);
			}
	#else

				dev_kfree_skb_any((struct sk_buff *)(pdescinfo->pframe));

	#endif //NEW_MAC80211_DRV
#else
// for debug ------------
//				rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
				if (pdescinfo->pframe) {  
				if (((struct sk_buff *)pdescinfo->pframe)->list) {
					DEBUG_ERR("Free tx skb error, skip it!\n");
					priv->ext_stats.freeskb_err++;
				}
				else
					rtl_kfree_skb(pdescinfo->priv, (struct sk_buff *)(pdescinfo->pframe), _SKB_TX_IRQ_);
				}
#endif
				needRestartQueue = 1;
			}
		}
		
		else
		{
			DEBUG_ERR("Unknown tx frame type %d\n", pdescinfo->type);
		}

		// for skb buffer free
		pdescinfo->pframe = NULL;

		recycleCnt ++;

		tail = (tail + 1) % NUM_TX_DESC;
	}

	*get_txtail_addr(phw, txRingIdx) = tail;

//	RESTORE_INT(flags);

	if (recycleCnt_p)
		*recycleCnt_p = recycleCnt;

	return needRestartQueue;
}


void rtl80211_tx_tasklet(unsigned long task_priv)
{

	struct mac80211_shared_priv	*priv = (struct mac80211_shared_priv *)task_priv;
	unsigned int	j=0;
	unsigned int	restart_queue=0;
	struct rtl8190_hw	*phw=GET_HW(priv);
	int needRestartQueue;

	if (!phw)
		return;

	for(j=0; j<=HIGH_QUEUE; j++)
	{
//		SAVE_INT_AND_CLI(flags);
		spin_lock_bh(&priv->pshare_hw->lock);

		needRestartQueue = rtl8190_tx_recycle(priv, j, NULL);
//		RESTORE_INT(flags);
		spin_unlock_bh(&priv->pshare_hw->lock);

		/* If anyone of queue report the TX queue need to be restart : we would set "restart_queue" to process ALL queues */
		if (needRestartQueue == 1)
			restart_queue = 1;
	}
/*
	if (restart_queue)
		rtl8190_tx_restartQueue(priv);

#ifdef MP_TEST
	if ((OPMODE & (WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND))==(WIFI_MP_STATE|WIFI_MP_CTX_BACKGROUND)) {
		int *tx_head, *tx_tail;
		tx_head = get_txhead_addr(phw, BE_QUEUE);
		tx_tail = get_txtail_addr(phw, BE_QUEUE);
		if (CIRC_SPACE_RTK(*tx_head, *tx_tail, NUM_TX_DESC) > (NUM_TX_DESC/2))
			mp_ctx(priv, (unsigned char *)"tx-isr");
	}
#endif
*/
//	refill_skb_queue(priv);

#ifdef RTL8192SE
	{
		//unsigned long x;
//		SAVE_INT_AND_CLI(x);
		spin_lock_bh(&priv->pshare_hw->lock);

		priv->pshare_hw->has_triggered_tx_tasklet = 0;
//		RESTORE_INT(x);
		spin_unlock_bh(&priv->pshare_hw->lock);

	}
#endif

}

#endif

int rtl8192se_get_qnum(u16 queue)
{
	int qnum;

	switch (queue) {
	case 0:
		qnum = VO_QUEUE;
		break;
	case 1:
		qnum = VI_QUEUE;
		break;
	case 2:
		qnum = BE_QUEUE;
		break;
	case 3:
		qnum = BK_QUEUE;
		break;
	default:
		qnum = BE_QUEUE;
		break;
	}

	return qnum;
}

int rtl80211_tx(struct ieee80211_hw *hw, struct sk_buff *skb)
{
#if defined(NEW_MAC80211_DRV)
	int tidno;
	u8 *qc, tid;
	struct priv_stainfo *node;
	struct ieee80211_hdr *hdr  = (struct ieee80211_hdr *)skb->data;
	struct mac80211_shared_priv *priv = (struct mac80211_shared_priv *) hw->priv;
	struct ieee80211_sta *sta = NULL;
#ifdef BUFFER_TX_AMPDU
	//rcu_read_lock();
	sta = ieee80211_find_sta(hw, hdr->addr1);

	if(sta){
		qc = ieee80211_get_qos_ctl(hdr);
		tidno = qc[0] & 0xf;

		if (rtl_aggr_query(priv, (struct priv_stainfo *)sta->drv_priv, tidno)){
			int ret;
			assign_aggr_tid_seqno(skb, sta);
			/*
			 * Try aggregation if it's a unicast data frame
			 * and the destination is HT capable.
			 */
			ret = ampdu_check(priv,skb,(struct priv_stainfo *)sta->drv_priv);
			if( ret == 0 )	//return 0 means to bypass
				__rtl80211_tx(hw, skb, TX_NORMAL);	/* transmit as normal packet */

			//rcu_read_unlock();
			return NETDEV_TX_OK;
		}
	}
	//rcu_read_unlock();
#endif //ifdef BUFFER_TX_AMPDU

	__rtl80211_tx(hw, skb, TX_NORMAL);	/* transmit as normal packet */
	return NETDEV_TX_OK;
#else	// !(NEW_MAC80211_DRV)
	__rtl80211_tx(hw, skb, TX_NORMAL);	/* transmit as normal packet */
	return NETDEV_TX_OK;
#endif
}

int __rtl80211_tx(struct ieee80211_hw *hw, struct sk_buff *skb, int tx_mthd)
{
//unsigned int fr_len, tx_len, i, keyid = 0, frag_idx=0;
//	unsigned char *da, *pbuf, *pwlhdr;

	static struct mac80211_shared_priv *priv;
	static struct ieee80211_tx_info *info;
	u8 rc_flags;

	static struct tx_info txinfo;
	static struct ieee80211_hdr *hdr;
	static struct ieee80211_sta *sta = NULL;

	priv = (struct mac80211_shared_priv *) hw->priv;
	info = IEEE80211_SKB_CB(skb);
	hdr  = (struct ieee80211_hdr *)skb->data;	
	memset(&txinfo, '\0', sizeof(txinfo));	

	rcu_read_lock();
	sta = ieee80211_find_sta(hw, hdr->addr1);

	if (!sta) {
		txinfo.aid = 0;
		txinfo.fixed_rate = 1;
		//txinfo.tx_rate = CCK_11M;// Basic rate is set to 11Mbps
		txinfo.tx_rate = CCK_1M;
	} else {
		struct priv_stainfo *stainfo = (struct priv_stainfo *) sta->drv_priv;
		int fw_aid = sta->aid + 1;
		txinfo.aid = fw_aid;

		if (is_MCS_rate(stainfo->current_tx_rate)) {
			int ret;
			/*
 			printk("CTL_AMPDU\n");
			txinfo.aggre_mthd = AGGRE_AMPDU;
			*/
			txinfo.tx_rate = stainfo->current_tx_rate & 0x7f;
			txinfo.rts_bw = txinfo.is_40m_bw = 
				((sta->ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40) && 
					conf_is_ht40(&hw->conf))? 1: 0; /* 0: 20m, 1: 40m */
		} else {
			if (stainfo->current_tx_rate > 0)
				txinfo.tx_rate = get_rate_index_from_ieee_value(stainfo->current_tx_rate);
		}
		
	}
	rcu_read_unlock();

#ifdef BUFFER_TX_AMPDU
	if ((tx_mthd >= TX_AMPDU_BUFFER_SIG) && (tx_mthd <= TX_AMPDU_BUFFER_LAST)) {
		if (tx_mthd == TX_AMPDU_BUFFER_FIRST)
			txinfo.aggre_mthd = FG_AGGRE_MPDU_BUFFER_FIRST;
		else if (tx_mthd == TX_AMPDU_BUFFER_MID)
			txinfo.aggre_mthd = FG_AGGRE_MPDU_BUFFER_MID;
		else  if (tx_mthd == TX_AMPDU_BUFFER_LAST)
			txinfo.aggre_mthd = FG_AGGRE_MPDU_BUFFER_LAST;
		goto skip_aggre_que;
	}
	/* brian move to rtl_get_rate
	if (pstat && (pstat->aggre_mthd == AGGRE_MTHD_MPDU) && (AMPDU_ENABLE != 3)  // not buffered AMPDU )  {*/
	/*
	if( sta ){
		int ret = ampdu_check(priv, skb, (struct priv_stainfo *)sta->drv_priv);

		if (ret == RET_AGGRE_ENQUE)
			goto stop_proc;

		if (ret == RET_AGGRE_DESC_FULL)
			goto free_and_stop;
	}
	*/

skip_aggre_que:
#endif

	// config from txinfo
	if ((info->flags & ~IEEE80211_TX_CTL_NO_ACK) && 
		!is_multicast_ether_addr(hdr->addr1)) {
		txinfo.need_ack = 1;
	} else {
		//printk("NO_ACK\n");
	}

	/*
	if (info->flags & IEEE80211_TX_CTL_FIRST_FRAGMENT) {
		//printk("FIRST_FRAGMENT\n");
	}

	if (info->flags & IEEE80211_TX_CTL_AMPDU) {
		//printk("CTL_AMPDU\n");
		txinfo.aggre_mthd = AGGRE_AMPDU;
	}*/

	rc_flags = info->control.rates[0].flags;

	//printk("40_MHZ = %d\n", priv->pshare_hw->is_40m_bw);
	
	
	if (rc_flags & IEEE80211_TX_RC_SHORT_GI) {
		//printk("SHORT_GI\n");
		txinfo.short_gi = 1;
	}
	if (rc_flags & IEEE80211_TX_RC_USE_SHORT_PREAMBLE) {
		//printk("SHORT_PREAMBLE\n");
		txinfo.short_preamble = 1;
	}
	if (rc_flags & IEEE80211_TX_RC_USE_RTS_CTS) {
		txinfo.rts |= RTS_ENABLE;
		//printk("USE_RTS_CTS\n");
		if (rc_flags & IEEE80211_TX_RC_USE_SHORT_PREAMBLE)
			txinfo.rts |= RTS_SHORT_PREAMBLE;
	}
	if (rc_flags & IEEE80211_TX_RC_USE_CTS_PROTECT){
		//printk("USE_CTS_PROTECT\n");
		txinfo.cts2self = 1;
	}

	txinfo.buf_num = 1;
	txinfo.buf[0].skb = skb;
	txinfo.buf[0].len = skb->len;
	txinfo.buf[0].data = skb->data;
	txinfo.buf[0].type = _SKB_FRAME_TYPE_;
	txinfo.rts_retry = 0xf;
	txinfo.retry = -1;	/* no retry limit in desc */		
	if (ieee80211_has_morefrags(hdr->frame_control))
		txinfo.more_frag = 1;

	if (ieee80211_is_mgmt(hdr->frame_control)){
		txinfo.q_num = MGNT_QUEUE;
		txinfo.fixed_rate = 1;
		//txinfo.tx_rate = CCK_11M;
		txinfo.tx_rate = CCK_1M;
	} else
		txinfo.q_num = rtl8192se_get_qnum(skb_get_queue_mapping(skb));
	/*
	if( tx_mthd < FG_AGGRE_MSDU_FIRST )
		txinfo.q_num = BE_QUEUE;
	*/

	//txinfo.tx_rate = ieee80211_get_tx_rate(hw, info)->hw_value;
	//printk("tx que = %d, tx_rate= %d \n", txinfo.q_num, ieee80211_get_tx_rate(hw, info)->hw_value);

	switch(priv->pshare_hw->offset_2nd_chan){
	case HT_2NDCH_OFFSET_BELOW: 
		txinfo.side_band = SIDE_BAND_LOWER;
		break;
	case HT_2NDCH_OFFSET_ABOVE:
		txinfo.side_band = SIDE_BAND_UPPER;
		break;
	default:
		txinfo.side_band = SIDE_BAND_DONOT_CARE;
		break;
	}

	/*
		
		txinfo.stbc
	
	*/

	fill_tx_desc(hw->priv, &txinfo);
stop_proc:
free_and_stop:
	return 0;
}



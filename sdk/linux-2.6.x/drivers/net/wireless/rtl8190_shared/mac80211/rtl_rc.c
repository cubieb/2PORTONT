/*
 * Copyright (c) 2004 Video54 Technologies, Inc.
 * Copyright (c) 2004-2009 Atheros Communications, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include "rtl_rc.h"

static void check_RA_by_rssi(struct mac80211_shared_priv *priv, struct ieee80211_sta *sta)
{
	int level = 0;
	struct priv_stainfo *stainfo = (struct priv_stainfo *) sta->drv_priv;

	switch (stainfo->rssi_level) {
		case 1:
			if (stainfo->rssi >= 50 /*priv->pshare_hw->rf_ft_var.raGoDownUpper*/)
				level = 1;
			else if ((stainfo->rssi >= 18 /*priv->pshare_hw->rf_ft_var.raGoDown20MLower*/) ||
				((priv->pshare_hw->is_40m_bw) && (sta->ht_cap.ht_supported) &&
				(stainfo->rssi >= 15 /*priv->pshare_hw->rf_ft_var.raGoDown40MLower*/) &&
				(sta->ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40)))
				level = 2;
			else
				level = 3;
			break;
		case 2:
			if (stainfo->rssi > 50 /*priv->pshare_hw->rf_ft_var.raGoUpUpper*/)
				level = 1;
			else if ((stainfo->rssi < 15 /*priv->pshare_hw->rf_ft_var.raGoDown40MLower*/) ||
				((!sta->ht_cap.ht_supported || !priv->pshare_hw->is_40m_bw ||
				!(sta->ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40)) &&
				(stainfo->rssi < 18 /*priv->pshare_hw->rf_ft_var.raGoDown20MLower*/)))
				level = 3;
			else
				level = 2;
			break;
		case 3:
			if (stainfo->rssi > 50 /*priv->pshare_hw->rf_ft_var.raGoUpUpper*/)
				level = 1;
			else if ((stainfo->rssi > 18 /*priv->pshare_hw->rf_ft_var.raGoUp20MLower*/) ||
				((priv->pshare_hw->is_40m_bw) && (sta->ht_cap.ht_supported) &&
				(stainfo->rssi > 15 /*priv->pshare_hw->rf_ft_var.raGoUp40MLower*/) &&
				(sta->ht_cap.cap & IEEE80211_HT_CAP_SUP_WIDTH_20_40)))
				level = 2;
			else
				level = 3;
			break;
		default:
			if (rtl80211_isErpSta(sta))
				DEBUG_ERR("wrong rssi level setting\n");
			break;
	}
/* chris 
	if (!pstat->is_2t_mimo_sta) {
		if (pstat->highest_rx_rate >= _MCS8_RATE_) {	// higher than MCS8 (2 spacial streams)
			pstat->is_2t_mimo_sta = TRUE;
			//if (pstat->is_rtl8190_sta)
			//	pstat->rssi_level = 0xff;		// force reassign RA bitmap
		}
	}
*/
	if (level != stainfo->rssi_level) {
		stainfo->rssi_level = level;
		rtl80211_addRATid(priv, sta);
	}
}

static void check_txrate_by_reg(struct mac80211_shared_priv *priv
#ifdef RTL8192SE
, struct ieee80211_sta *sta
#endif
)
{
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	struct priv_stainfo *stainfo = (struct priv_stainfo *) sta->drv_priv;
#if 0 //defined(RTL8190)
	unsigned short reg_val = RTL_R16(0x13c);
	unsigned char txrate = reg_val & 0x00ff;
	unsigned char ratid = (reg_val & 0x7f00) >> 8;
	struct stat_info *pstat = NULL;
	struct rtl8190_priv *priv_new = NULL;

	if (reg_val == 0)
		return;

	if (ratid == 7) {
		RTL_W16(0x13c, 0);
		return;
	}

	if (ratid == 0) {
		if (priv->pshare->aidarray[6] && (priv->pshare->aidarray[6]->used == TRUE)) {
			pstat = &(priv->pshare->aidarray[6]->station);
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			priv_new = priv->pshare->aidarray[6]->priv;
#else
			priv_new = priv;
#endif
		}
	}
	else {
		if (priv->pshare->aidarray[ratid-1] && (priv->pshare->aidarray[ratid-1]->used == TRUE)) {
			pstat = &(priv->pshare->aidarray[ratid-1]->station);
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			priv_new = priv->pshare->aidarray[ratid-1]->priv;
#else
			priv_new = priv;
#endif
		}
	}

	if (pstat && priv_new) {
		if (priv_new->pmib->dot11StationConfigEntry.autoRate) {
			if (txrate < 12) {
				if (pstat->tx_ra_bitmap & BIT(txrate))
					pstat->current_tx_rate = dot11_rate_table[txrate];
			}
			else {
				if (pstat->tx_ra_bitmap & (BIT(txrate & 0x0f) << 12)) {
					pstat->current_tx_rate = 0x80 | (txrate & 0x0f);

					if (((txrate>>4) == 2) || ((txrate>>4) == 4))
						pstat->ht_current_tx_info |= TX_USE_40M_MODE;
					else
						pstat->ht_current_tx_info &= ~TX_USE_40M_MODE;

					if (((txrate>>4) > 2) && ((txrate>>4) < 5))
						pstat->ht_current_tx_info |= TX_USE_SHORT_GI;
					else
						pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;
				}
			}
			check_AMSDULevel_by_rate(priv_new, pstat);
		}
		else {
			if ((priv->pshare->is_40m_bw) && (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40))
				pstat->ht_current_tx_info |= TX_USE_40M_MODE;
			else
				pstat->ht_current_tx_info &= ~TX_USE_40M_MODE;

			if (((pstat->tx_bw == HT_CHANNEL_WIDTH_20)
				&& (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M)
				&& (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
				|| ((pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)
				&& (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M)
				&& (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_))))
				pstat->ht_current_tx_info |= TX_USE_SHORT_GI;
			else
				pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;
		}
	}

	RTL_W16(0x13c, 0);

#else	// RTL8192SE
	unsigned char initial_rate = 0x3f;
	unsigned short fw_aid = sta->aid + 1;
#ifdef STA_EXT
	if (pstat->remapped_aid && (pstat->remapped_aid < FW_NUM_STAT-1)) {
#else
	if (fw_aid && (fw_aid < 32)) {
#endif
		// chris if (priv->pmib->dot11StationConfigEntry.autoRate) {
		if (1) {
#ifdef STA_EXT
			initial_rate = RTL_R8(INIMCS_SEL + pstat->remapped_aid) & 0x3f;
#else
			initial_rate = RTL_R8(INIMCS_SEL + fw_aid) & 0x3f;
#endif
			if (initial_rate == 0x3f)
				return;
			if (initial_rate < 12)
				stainfo->current_tx_rate = dot11_rate_table[initial_rate];
			else if (initial_rate > 27)
				stainfo->current_tx_rate = (stainfo->tx_ra_bitmap & 0xff00000) ? 0x8f : 0x87;
			else
				stainfo->current_tx_rate = 0x80|(initial_rate -12);

/* chris: no need to check here
			if (initial_rate == 28)
				stainfo->ht_current_tx_info |= TX_USE_SHORT_GI;
			else
				stainfo->ht_current_tx_info &= ~TX_USE_SHORT_GI;
*/
		}
		/* chris : auto rate
		else {
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			if (IS_ROOT_INTERFACE(priv))
#endif
			{
				if (((sta->tx_bw == HT_CHANNEL_WIDTH_20)
					&& (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M)
					&& (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_)))
					|| ((pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)
					&& (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M)
					&& (pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_))))
					pstat->ht_current_tx_info |= TX_USE_SHORT_GI;
				else
					pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;
			}
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			else
				pstat->ht_current_tx_info &= ~TX_USE_SHORT_GI;
#endif
		}*/
/* chris: do not have to check it here
		if (priv->pshare_hw->is_40m_bw && (pstat->tx_bw == HT_CHANNEL_WIDTH_20_40))
			pstat->ht_current_tx_info |= TX_USE_40M_MODE;
		else
			pstat->ht_current_tx_info &= ~TX_USE_40M_MODE;
	*/
	}
	else {
		DEBUG_INFO("sta has no aid found to check current tx rate\n");
	}
#endif
}

/***********************************/
/* mac80211 Rate Control callbacks */
/***********************************/

static void rtl_tx_status(void *priv, struct ieee80211_supported_band *sband,
			  struct ieee80211_sta *sta, void *priv_sta,
			  struct sk_buff *skb)
{
	struct mac80211_shared_priv *rtl_priv = priv;
	//struct rtl_rate_priv *rtl_rc_priv = priv_sta;
	//struct rtl_tx_info_priv *tx_info_priv = NULL;
	struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
	struct ieee80211_hdr *hdr;
	int final_ts_idx, tx_status = 0, is_underrun = 0;
	__le16 fc;

	hdr = (struct ieee80211_hdr *)skb->data;
	fc = hdr->frame_control;
	/*
	tx_info_priv = ATH_TX_INFO_PRIV(tx_info);
	final_ts_idx = tx_info_priv->tx.ts_rateindex;

	if (!priv_sta || !ieee80211_is_data(fc) ||
		!tx_info_priv->update_rc)
		goto exit;

	if (tx_info_priv->tx.ts_status & RTL_TXERR_FILT)
		goto exit;
	*/

	/*
	 * If underrun error is seen assume it as an excessive retry only
	 * if prefetch trigger level have reached the max (0x3f for 5416)
	 * Adjust the long retry as if the frame was tried ATH_11N_TXMAXTRY
	 * times. This affects how ratectrl updates PER for the failed rate.
	 */
	/*
	if (tx_info_priv->tx.ts_flags &
		(ATH9K_TX_DATA_UNDERRUN | ATH9K_TX_DELIM_UNDERRUN) &&
		((sc->sc_ah->tx_trig_level) >= ath_rc_priv->tx_triglevel_max)) {
		tx_status = 1;
		is_underrun = 1;
	}

	if ((tx_info_priv->tx.ts_status & ATH9K_TXERR_XRETRY) ||
		(tx_info_priv->tx.ts_status & ATH9K_TXERR_FIFO))
		tx_status = 1;

	ath_rc_tx_status(sc, ath_rc_priv, tx_info, final_ts_idx, tx_status,
			 (is_underrun) ? ATH_11N_TXMAXTRY :
			 tx_info_priv->tx.ts_longretry);
	*/

	/* Check if aggregation has to be enabled for this tid */
	if (conf_is_ht(&rtl_priv->hw->conf) &&
		!(skb->protocol == cpu_to_be16(ETH_P_PAE))) {
		if (ieee80211_is_data_qos(fc)) {
			u8 *qc, tid;
			struct priv_stainfo *node;

			qc = ieee80211_get_qos_ctl(hdr);
			tid = qc[0] & 0xf;
			node = (struct priv_stainfo *)sta->drv_priv;

			if(rtl_tx_aggr_check(rtl_priv, node, tid)){
				ieee80211_start_tx_ba_session(rtl_priv->hw, hdr->addr1, tid);
			}
		}
	}

	/*
	rtl_debug_stat_rc(sc, skb);
exit:
	kfree(tx_info_priv);
	*/
	return;
}

static void rtl_get_rate(void *priv, struct ieee80211_sta *sta, void *priv_sta,
			 struct ieee80211_tx_rate_control *txrc)
{
    struct ieee80211_supported_band *sband = txrc->sband;
	struct sk_buff *skb = txrc->skb;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
	struct mac80211_shared_priv *rate_priv = (struct mac80211_shared_priv *)priv;
	__le16 fc = hdr->frame_control;
	
	struct ieee80211_hw *hw = txrc->hw;
	
	if (!ieee80211_is_data(fc) || is_multicast_ether_addr(hdr->addr1) ||
	    !sta) {
		tx_info->control.rates[0].idx = rate_lowest_index(sband, sta);
		tx_info->control.rates[0].count =
			is_multicast_ether_addr(hdr->addr1) ? 1 : RTL_MGT_TXMAXTRY;
		return;
	}
	/* ra update */
	rcu_read_lock();
	sta = ieee80211_find_sta(hw, hdr->addr1);
	if (sta){
		struct priv_stainfo* stainfo = (struct priv_stainfo *) sta->drv_priv;
		if (time_after(jiffies, stainfo->t_raupdate + (rate_priv->raupdate_intv * HZ) / 1000)) {
			check_RA_by_rssi(rate_priv, sta);
			//check_txrate_by_reg(rate_priv, sta);
			stainfo->t_raupdate = jiffies;
		}
	}
	rcu_read_unlock();

	if( sta ){
		struct priv_stainfo *priv_sta = (struct priv_stainfo *)sta->drv_priv;
		//printk("check_txrate_by_reg return rate index:%x\n",priv_sta->current_tx_rate);
		u8 *qc, tid;
		struct priv_stainfo *node;
		struct rtl_atx_tid *txtid;

		qc = ieee80211_get_qos_ctl(hdr);
		tid = qc[0] & 0xf;
		node = (struct priv_stainfo *)sta->drv_priv;
		txtid = RTL_ND_2_TID(node, tid);

		priv_sta->current_tx_rate = 0x80 | (MCS15-12);
	}

	tx_info->control.rates[0].idx = 0;
	tx_info->control.rates[0].count = 3;

	/*
	if (txrc->short_preamble)
		tx_info->control.rates[0].flags |= IEEE80211_TX_RC_USE_SHORT_PREAMBLE;
	if (txrc->rts || rtsctsenable)
	*/
	tx_info->control.rates[0].flags |= IEEE80211_TX_RC_USE_RTS_CTS;
	//tx_info->control.rates[0].flags |= IEEE80211_TX_RC_USE_CTS_PROTECT;

	/*
	if (WLAN_RC_PHY_40(rate_table->info[rix].phy))
		rate->flags |= IEEE80211_TX_RC_40_MHZ_WIDTH;
	if (WLAN_RC_PHY_SGI(rate_table->info[rix].phy))
		rate->flags |= IEEE80211_TX_RC_SHORT_GI;
	if (WLAN_RC_PHY_HT(rate_table->info[rix].phy))
		rate->flags |= IEEE80211_TX_RC_MCS;*/
}

static int rtl_rate_init(void)
{
	return 0;
}

static int rtl_rate_update(void)
{
	return 0;
}

static void *rtl_rate_alloc(struct ieee80211_hw *hw, struct dentry *debugfsdir)
{
        struct mac80211_shared_priv *priv = hw->priv;
	return priv;
}

static void rtl_rate_free(void *priv)
{
	return;
}

static void *rtl_rate_alloc_sta(void *priv, struct ieee80211_sta *sta, gfp_t gfp)
{
#ifdef BUFFER_TX_AMPDU
	struct priv_stainfo *pstat = sta->drv_priv;
	int i=0, tidno;
	struct rtl_atx_tid *tid;

	for (tidno = 0, tid = &pstat->tid[tidno];
		 tidno < WME_NUM_TID;
		 tidno++, tid++) {
			//tid->rtl_node = ??;
			tid->tidno		 = tidno;
			tid->seq_start	 = tid->seq_next = 0;
			tid->baw_size	 = WME_MAX_BA;
			tid->baw_head	 = tid->baw_tail = 0;
			/* unused parameters
			tid->sched		 = false;
			tid->paused		 = false;
			tid->state &= ~AGGR_CLEANUP;
			INIT_LIST_HEAD(&tid->buf_q);
			acno = TID_TO_WME_AC(tidno);
			tid->ac = &an->ac[acno];
			*/
			tid->state &= ~AGGR_ADDBA_COMPLETE;
			tid->state &= ~AGGR_ADDBA_PROGRESS;
			tid->addba_exchangeattempts = 0;
		}

	for (i=0; i<8; i++)
		skb_queue_head_init(&pstat->ampdu_tx_que[i]);
#endif

	return (struct mac80211_shared_priv *)priv;
}

static int rtl_rate_free_sta(void)
{
	return 0;
}

static struct rate_control_ops rtl_rate_ops = {
	.module = NULL,
	.name = "rtl_rate_control",
	.tx_status = rtl_tx_status,
	.get_rate = rtl_get_rate,
	.rate_init = rtl_rate_init,
	.rate_update = rtl_rate_update,
	.alloc = rtl_rate_alloc,
	.free = rtl_rate_free,
	.alloc_sta = rtl_rate_alloc_sta,
	.free_sta = rtl_rate_free_sta,
};

int rtl_rate_control_register(void)
{
	return ieee80211_rate_control_register(&rtl_rate_ops);
}

void rtl_rate_control_unregister(void)
{
	ieee80211_rate_control_unregister(&rtl_rate_ops);
}

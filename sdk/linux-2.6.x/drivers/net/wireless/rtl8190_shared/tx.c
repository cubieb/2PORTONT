/*
 *  TX handle routines
 *
 *  $Id: tx.c,v 1.1.1.1 2010/05/05 09:00:38 jiunming Exp $
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

//#include "./rtl80211.h"

#include "./8190n_util.h"
#include "./8190n_headers.h"

#ifdef NEW_MAC80211_DRV
#define TX_RATE txinfo->tx_rate
#define IS_AMPDU (txinfo->aggre_mthd > AGGRE_AMPDU)
#define IS_STBC txinfo->stbc
#define IS_TX_SPREAMBLE txinfo->short_preamble
#define NEED_ACK txinfo->need_ack
#define IS_40M_BW txinfo->is_40m_bw
#else
#define TX_RATE txcfg->tx_rate
#define IS_AMPDU ((txcfg->aggre_en >= FG_AGGRE_MPDU) && (txcfg->aggre_en <= FG_AGGRE_MPDU_BUFFER_LAST))
#define IS_STBC (priv->pmib->dot11nConfigEntry.dot11nTxSTBC && \
					txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_RX_STBC_)))
#define IS_TX_SPREAMBLE check_if_bgtxshort(priv, txcfg)
#define NEED_ACK txcfg->need_ack
#ifdef WIFI_11N_2040_COEXIST
#define IS_40M_BW ((priv->pshare->is_40m_bw) &&(txcfg->pstat && (txcfg->pstat->tx_bw == HT_CHANNEL_WIDTH_20_40) \
					&& !((OPMODE & WIFI_AP_STATE) && priv->pshare->rf_ft_var.coexist_enable && \
					(priv->bg_ap_timeout || priv->force_20_sta || priv->switch_20_sta))))
#else
#define IS_40M_BW ((priv->pshare->is_40m_bw) &&(txcfg->pstat && (txcfg->pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)))
#endif
extern void wep_fill_iv(struct rtl8190_priv *priv,
			unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid);

extern void tkip_fill_encheader(struct rtl8190_priv *priv,
			unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid_out);

extern void aes_fill_encheader(struct rtl8190_priv *priv,
			unsigned char *pwlhdr, unsigned int hdrlen, unsigned long keyid);

extern int generate_legacy_phdr(struct rtl8190_priv *priv, struct tx_insn *txcfg);
#endif //ifdef NEW_MAC80211_DRV

#ifdef RTL8192SE
#define RECORD_FREE_INFO(pdescinfo, buf) \
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
{
	memset(pdesc, 0, 32); 
	pdesc->Dword0 = set_desc((32 << TX_OFFSETSHIFT) | TX_OWN); 
	pdesc->Dword7 = set_desc(buf->len & TX_TxBufferSizeMask); 
	pdesc->Dword8 = set_desc(get_physical_addr(priv, buf->data, buf->len, PCI_DMA_TODEVICE)); 

	RECORD_FREE_INFO(pdescinfo, buf)
#if 0
//#ifndef NEW_MAC80211_DRV
	pdescinfo->priv = priv;
#endif
}
#endif

// the function should be called in critical section
static __inline__ void rtl_txdesc_rollover(struct tx_desc *ptxdesc, unsigned int *ptxhead)
{
	*ptxhead = (*ptxhead + 1) % NUM_TX_DESC;
}

#ifdef NEW_MAC80211_DRV
static __inline__ void rtl_tx_poll(struct mac80211_shared_priv *priv, int q_num)
#else
static __inline__ void rtl_tx_poll(struct rtl8190_priv *priv, int q_num)
#endif
{
#ifdef NEW_MAC80211_DRV
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
#else
	unsigned long ioaddr = priv->pshare->ioaddr;
#endif

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

static int config_legacy_MP(struct rtl8190_priv *priv, struct tx_desc  *pdesc, struct tx_insn *txcfg)
{
	int n_txshort=0;
	unsigned long ioaddr = priv->pshare->ioaddr;
	unsigned char txRate = 0;

	if (OPMODE & WIFI_MP_STATE) {
		if (is_MCS_rate(txcfg->tx_rate)) {	// HT rates
			txRate = txcfg->tx_rate & 0x7f;
			txRate += 12; // for 92SE
			pdesc->Dword4 |= set_desc(TX_TXHT);
		}
		else{
			txRate = get_rate_index_from_ieee_value((UINT8)txcfg->tx_rate);
		}

		if (priv->pshare->CurrentChannelBW) {
			pdesc->Dword4 |= set_desc(TX_TxBw);
			pdesc->Dword4 |= set_desc(3 << TX_TXSCSHIFT);
			if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M && IS_ROOT_INTERFACE(priv))
				n_txshort = 1;
		}
		else {
			if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M && IS_ROOT_INTERFACE(priv))
				n_txshort = 1;
		}

		if (txcfg->retry) {
			pdesc->Dword2 |= set_desc(TX_RetryLmtEn);
			pdesc->Dword2 |= set_desc((txcfg->retry & TX_DataRetryLmtMask) << TX_DataRetryLmtSHIFT);
		}

		if(n_txshort == 1){ // short GI, modify the rate
			unsigned short max_sg_rate, multibss_sg;
			max_sg_rate = (txRate-12) & 0xf;
			txRate = 0x1c; // MAX is MSC 15
			multibss_sg = max_sg_rate | (max_sg_rate<<4) | (max_sg_rate<<8) | (max_sg_rate<<12);
			if (RTL_R16(SG_RATE) != multibss_sg)
			{
				printk("Short GI, rate: %x\n", multibss_sg);
				RTL_W16(SG_RATE, multibss_sg); // real rate we want to send
			}
		}

		txcfg->tx_rate = txRate;
		pdesc->Dword5 |= set_desc((txRate & TX_TxRateMask) << TX_TxRateSHIFT);

		return 0;
	}
	return 1;
}

static int check_if_ntxshort(struct rtl8190_priv *priv, struct tx_insn *txcfg)
{
	int n_txshort=0;

	if (priv->pshare->is_40m_bw){
		if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M &&
			txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_40M_))
			&& IS_ROOT_INTERFACE(priv))
				n_txshort = 1;

		if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
			txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_))
			&& IS_ROOT_INTERFACE(priv))
				n_txshort = 1;
	}
	else{
		if (priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M &&
			txcfg->pstat && (txcfg->pstat->ht_cap_buf.ht_cap_info & cpu_to_le16(_HTCAP_SHORTGI_20M_))
			&& IS_ROOT_INTERFACE(priv))
			n_txshort = 1;
	}
	return n_txshort;
}

static int check_if_bgtxshort(struct rtl8190_priv *priv, struct tx_insn *txcfg)
{
	int bg_txshort=0;

	if (is_CCK_rate(txcfg->tx_rate) && (txcfg->tx_rate != 2)) {
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
				(priv->pmib->dot11ErpInfo.longPreambleStaNum > 0))
			; // txfw->txshort = 0
		else {
			if (txcfg->pstat)
				bg_txshort = (priv->pmib->dot11RFEntry.shortpreamble) &&
								(txcfg->pstat->useShortPreamble);
			else
				bg_txshort = priv->pmib->dot11RFEntry.shortpreamble;
		}
	}
	return bg_txshort;
}

#ifdef NEW_MAC80211_DRV
static int config_unicast_tx(struct tx_info *txinfo, struct tx_desc *pdesc)
#else
static int config_unicast_tx(struct rtl8190_priv *priv, struct tx_insn *txcfg, struct tx_desc *pdesc, int frag_idx)
#endif
{
#ifdef NEW_MAC80211_DRV
	if (NEED_ACK) {
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
#else
	int erp_protection = 0, n_protection = 0;
	unsigned char rate;

	if (NEED_ACK) { // unicast
		if (frag_idx == 0) {
			if ((txcfg->rts_thrshld <= get_mpdu_len(txcfg, txcfg->fr_len)) || 
				(txcfg->pstat && txcfg->pstat->is_forced_rts))
				pdesc->Dword4 |=set_desc(TX_RTSEn);
			else {
				if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
					is_MCS_rate(txcfg->tx_rate) &&
					(priv->ht_protection /*|| txcfg->pstat->is_rtl8190_sta*/))
				{
					n_protection = 1;
					pdesc->Dword4 |=set_desc(TX_RTSEn);
					if (priv->pmib->dot11ErpInfo.ctsToSelf)
						pdesc->Dword4 |= set_desc(TX_CTS2Self);
				}
				else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
					(!is_CCK_rate(txcfg->tx_rate)) && // OFDM mode
					priv->pmib->dot11ErpInfo.protection)
				{
					erp_protection = 1;
					pdesc->Dword4 |=set_desc(TX_RTSEn);
					if (priv->pmib->dot11ErpInfo.ctsToSelf)
						pdesc->Dword4 |= set_desc(TX_CTS2Self);
				}
				else if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
					(txcfg->pstat) && (txcfg->pstat->MIMO_ps & _HT_MIMO_PS_DYNAMIC_) &&
					is_MCS_rate(txcfg->tx_rate) && ((txcfg->tx_rate & 0x7f)>7))
				{	// when HT MIMO Dynamic power save is set and rate > MCS7, RTS is needed
					pdesc->Dword4 |=set_desc(TX_RTSEn);
				}
			}
		}
	}

	if (get_desc(pdesc->Dword4 ) & TX_RTSEn) {
		if (erp_protection)
			rate = (unsigned char)find_rate(priv, NULL, 1, 3);
		else
			rate = (unsigned char)find_rate(priv, NULL, 1, 1);

		if (is_MCS_rate(rate)) {	// HT rates
			// can we use HT rates for RTS?
		}
		else {
			unsigned int rtsTxRate	= 0;
			rtsTxRate = get_rate_index_from_ieee_value(rate);
			if (erp_protection) {
				unsigned char  rtsShort = 0;
				if (is_CCK_rate(rate) && (rate != 2)) {
					if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11G) &&
							(priv->pmib->dot11ErpInfo.longPreambleStaNum > 0))
						rtsShort = 0; // do nothing
					else {
						if (txcfg->pstat)
							rtsShort = (priv->pmib->dot11RFEntry.shortpreamble) &&
											(txcfg->pstat->useShortPreamble);
						else
							rtsShort = priv->pmib->dot11RFEntry.shortpreamble;
					}
				}
				pdesc->Dword4 |= (rtsShort == 1)? set_desc(TX_RTSShort): 0;
			}
			else if (n_protection)
				rtsTxRate = get_rate_index_from_ieee_value(48);
			else {	// > RTS threshold
			}
			pdesc->Dword4 |= set_desc((rtsTxRate & TX_RTSRateMask) << TX_RTSRateSHIFT);
			pdesc->Dword4 |= set_desc((0xf & TX_RTSRateFBLmtMask) << TX_RTSRateFBLmtSHIFT);
			//8192SE Must specified BW mode while sending RTS ...
			if (priv->pshare->is_40m_bw)
				pdesc->Dword4 |= set_desc(TX_RTSBW);
		}
	}
#endif
	return 0;
}

#ifdef CONFIG_MIPS
__MIPS16
__IRAM_FASTEXTDEV
#endif
#ifdef NEW_MAC80211_DRV
int rtl_fill_fwinfo(struct mac80211_shared_priv *priv, struct tx_info *txinfo, struct tx_desc *pdesc)
#else
int rtl_fill_fwinfo(struct rtl8190_priv *priv, struct tx_insn *txcfg, struct tx_desc *pdesc, unsigned int frag_idx)
#endif
{
//	struct FWtemplate *txfw = (struct FWtemplate *)ptxfw;
//	unsigned char ampdu_des;
//	static unsigned short multibss_sg_old = 0x1234;
	unsigned char txRate = 0;

//	memset(txfw, 0, sizeof(struct FWtemplate)); // initialize to zero

#if defined(MP_TEST) && !defined(NEW_MAC80211_DRV)
	if( config_legacy_MP(priv, pdesc, txcfg) == 0)
		return 0;
#endif
	if (is_MCS_rate(TX_RATE)) {
#ifndef NEW_MAC80211_DRV
		txRate = TX_RATE & 0x7f;
		txRate += 12; // for 92SE
#endif
		pdesc->Dword4 |= set_desc(TX_TXHT);
		if (IS_40M_BW) {
			pdesc->Dword4 |= set_desc(TX_TxBw);	
#ifdef NEW_MAC80211_DRV
			pdesc->Dword4 |= set_desc((txinfo->side_band & SIDE_BAND_MASK) << TX_TXSCSHIFT);
#else
			pdesc->Dword4 |= set_desc(0 << TX_TXSCSHIFT);
#endif
		}

#ifdef NEW_MAC80211_DRV
		if (txinfo->short_gi)
			txinfo->tx_rate = 0x1c;	/* may need to be revised in new chip */
#else
		if(check_if_ntxshort(priv,txcfg)
#ifdef STA_EXT
			&& txcfg->pstat && (txcfg->pstat->remapped_aid < FW_NUM_STAT -1)
#endif
			)
			txRate = 0x1c;	/* may need to be revised in new chip */
#endif

		//if (txinfo->aggre_mthd >= AGGRE_AMPDU) {
		if (IS_AMPDU) {
			pdesc->Dword2 |= set_desc(TX_AggEn);
#ifdef NEW_MAC80211_DRV
			if (priv->pshare_hw->last_ht_dst_cached &&
					memcmp(get_da(txinfo->buf[0].data), priv->pshare_hw->last_ht_dst_mac, 6)) {
				pdesc->Dword2 |= set_desc(TX_BK);
				memcpy(priv->pshare_hw->last_ht_dst_mac, get_da(txinfo->buf[0].data), 6);
				priv->pshare_hw->last_ht_dst_cached = 1;
			}
			else{
				priv->pshare_hw->last_ht_dst_cached = 0;
			}
#else
			if(txcfg->pstat != priv->pshare->CurPstat){
				pdesc->Dword2 |= set_desc(TX_BK);
				priv->pshare->CurPstat = txcfg->pstat;
			}
			/*
			switch (priv->pmib->dot11nConfigEntry.dot11nAMPDUSendSz) {
			case 8:
				txfw->rxMF = 0;
				break;
			case 16:
				txfw->rxMF = 1;
				break;
			case 32:
				txfw->rxMF = 2;
				break;
			case 64:
				txfw->rxMF = 3;
				break;
			default:
				if (txcfg->pstat->is_rtl8190_sta) {
					txfw->rxMF = 3;
				}
				else {
					//txfw->rxMF = txcfg->pstat->ht_cap_buf.ampdu_para & 0x03;
					if ((txcfg->pstat->ht_cap_buf.ampdu_para & 0x03) > 0)
						txfw->rxMF = 1; 	// default 16K of AMPDU size to other clients support more than 8K
					else
						txfw->rxMF = 0; 	// default 8K of AMPDU size to other clients support 8K only
				}
				break;
			}
			ampdu_des = (txcfg->pstat->ht_cap_buf.ampdu_para & _HTCAP_AMPDU_SPC_MASK_) >> _HTCAP_AMPDU_SPC_SHIFT_;
			if ((ampdu_des > 0) && (ampdu_des < 7))
				ampdu_des++; // 8190 Spec doesn't fit to 802.11n
			if (priv->pshare->is_40m_bw && txcfg->pstat
				&& (txcfg->pstat->tx_bw == HT_CHANNEL_WIDTH_20_40)
				&& (get_sta_encrypt_algthm(priv, txcfg->pstat) == _CCMP_PRIVACY_)
				&& txcfg->pstat->dot11KeyMapping.keyInCam == TRUE)
				txfw->rxAMD = 7;
			else
				txfw->rxAMD = ampdu_des;
			*/
#endif
		}

		if (IS_STBC)
			pdesc->Dword4 |= set_desc(1 << TX_STBCSHIFT);
	}
	else { //legacy rate
#ifdef NEW_MAC80211_DRV
		txinfo->tx_rate = get_rate_index_from_ieee_value((UINT8)txinfo->tx_rate);
#else
		txRate = get_rate_index_from_ieee_value((UINT8)txcfg->tx_rate);

		if (priv->pshare->is_40m_bw) {
			if (priv->pshare->offset_2nd_chan == HT_2NDCH_OFFSET_BELOW)
				pdesc->Dword4 |= set_desc(2 << TX_TXSCSHIFT);
			else
				pdesc->Dword4 |= set_desc(1 << TX_TXSCSHIFT);
		}
#endif
		// 20080311 Bryant modified Tx power differences and omitted L-OFDM setting
		//if (!is_CCK_rate(txcfg->tx_rate) && !priv->pshare->use_default_para) {
		//	txfw->txAGCOffset = priv->pshare->legacyOFDM_pwrdiff & 0x0f;
		//	txfw->txAGCSign = (priv->pshare->legacyOFDM_pwrdiff & 0x80)? 1:0;
		//}
		if (IS_TX_SPREAMBLE)
			pdesc->Dword4 |= set_desc(TX_TxShort);
	}

#ifdef NEW_MAC80211_DRV
	config_unicast_tx(txinfo, pdesc);
#else
	config_unicast_tx(priv, txcfg, pdesc, frag_idx);
#endif

	// set TxRate
#if defined(RTL8192SE_ACUT) && !defined(NEW_MAC80211_DRV)
	if(txRate < 0x4)
		txRate = 0x4; // 6Mhz
#endif

/*
	if(txshort == 1){ // short GI, modify the rate
		unsigned int mbssidRate = 0;
		unsigned short max_sg_rate, multibss_sg;
		max_sg_rate = (txRate-12) & 0xf;
		txRate = 0x1c; // MAX is MSC 15
		multibss_sg = max_sg_rate | (max_sg_rate<<4) | (max_sg_rate<<8) | (max_sg_rate<<12);
		if (multibss_sg_old != multibss_sg)
		{
			printk("Short GI, rate: %x\n", multibss_sg);
			RTL_W16(SG_RATE, multibss_sg); // real rate we want to send
			multibss_sg_old = multibss_sg;
		}
	}
*/
	pdesc->Dword5 |= set_desc((txRate & TX_TxRateMask) << TX_TxRateSHIFT);

#ifndef NEW_MAC80211_DRV
	if (txcfg->need_ack) {
		//txfw->enCPUDur = 1;	// no need to count duration for broadcast & multicast pkts

		// give retry limit to management frame
		if (txcfg->q_num == MANAGE_QUE_NUM) {
			pdesc->Dword2 |= set_desc(TX_RetryLmtEn);
			if (GetFrameSubType(txcfg->phdr) == WIFI_PROBERSP) {
				;	// 0 no need to set
			}
#ifdef WDS
			else if ((GetFrameSubType(txcfg->phdr) == WIFI_PROBEREQ) && (txcfg->pstat->state & WIFI_WDS)) {
				pdesc->Dword2 |= set_desc((2 & TX_DataRetryLmtMask) << TX_DataRetryLmtSHIFT);
			}
#endif
			else {
				pdesc->Dword2 |= set_desc((6 & TX_DataRetryLmtMask) << TX_DataRetryLmtSHIFT);
			}
		}
#ifdef WDS
		else if (txcfg->wdsIdx >= 0) {
			if (txcfg->pstat->rx_avarage == 0) {
				pdesc->Dword2 |= set_desc(TX_RetryLmtEn);
				pdesc->Dword2 |= set_desc((3 & TX_DataRetryLmtMask) << TX_DataRetryLmtSHIFT);
			}
		}
#endif
	}
#endif
}

#if 0
static int generate_legacy_phdr(struct rtl8190_priv *priv, struct tx_insn *txcfg)
{
	unsigned int 	fr_len, tx_len, i, q_num, keyid;
	unsigned char	 q_select;
	int *tx_head;
	struct tx_desc  *phdesc, *pdesc, *pndesc, *picvdesc, *pmicdesc, *pfrstdesc;
	struct tx_desc_info	*pswdescinfo, *pdescinfo, *pndescinfo, *picvdescinfo, *pmicdescinfo;
	unsigned long		tmpphyaddr;
	unsigned char		*da, *pbuf, *pwlhdr, *pmic, *picv;
#ifdef TX_SHORTCUT
		int fit_shortcut = 0;
#endif
	unsigned int		pfrst_dma_desc=0;
	unsigned int		*dma_txhead;
	unsigned long flush_addr[20];
	int flush_len[20];
	int flush_num=0;

	q_num = txcfg->q_num;
	tx_len = txcfg->fr_len;

	for(i=0, pfrstdesc= phdesc + (*tx_head); i < txcfg->frg_num; i++)
	{
		/*------------------------------------------------------------*/
		/*           fill descriptor of header + iv + llc             */
		/*------------------------------------------------------------*/
		pdesc     = phdesc + (*tx_head);
		pdescinfo = pswdescinfo + *tx_head;

		//clear all bits
		memset(pdesc, 0, 32);

		if (i != 0)
		{
			// we have to allocate wlan_hdr
			pwlhdr = (UINT8 *)get_wlanhdr_from_poll(priv);
			if (pwlhdr == (UINT8 *)NULL)
			{
				//DEBUG_ERR("System-bug... should have enough wlan_hdr\n");
				return;
			}
			// other MPDU will share the same seq with the first MPDU
			memcpy((void *)pwlhdr, (void *)(txcfg->phdr), txcfg->hdr_len); // data pkt has 24 bytes wlan_hdr
		}
		else
		{
#ifdef SEMI_QOS
			if (txcfg->pstat && (is_qos_data(txcfg->phdr))) {
				if ((GetFrameSubType(txcfg->phdr) & (WIFI_DATA_TYPE | BIT(6) | BIT(7))) == (WIFI_DATA_TYPE | BIT(7))) {
					unsigned char tempQosControl[2];
					memset(tempQosControl, 0, 2);
					tempQosControl[0] = ((struct sk_buff *)txcfg->pframe)->cb[1];
#ifdef WMM_APSD
					if ((APSD_ENABLE) && (txcfg->pstat) && (txcfg->pstat->state & WIFI_SLEEP_STATE) &&
						(!GetMData(txcfg->phdr)) &&
						((((tempQosControl[0] == 7) || (tempQosControl[0] == 6)) && (txcfg->pstat->apsd_bitmap & 0x01)) ||
						 (((tempQosControl[0] == 5) || (tempQosControl[0] == 4)) && (txcfg->pstat->apsd_bitmap & 0x02)) ||
						 (((tempQosControl[0] == 3) || (tempQosControl[0] == 0)) && (txcfg->pstat->apsd_bitmap & 0x08)) ||
						 (((tempQosControl[0] == 2) || (tempQosControl[0] == 1)) && (txcfg->pstat->apsd_bitmap & 0x04))))
						tempQosControl[0] |= BIT(4);
#endif
					if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
						tempQosControl[0] |= BIT(7);

					memcpy((void *)GetQosControl((txcfg->phdr)), tempQosControl, 2);
				}
			}
#endif

			assign_wlanseq(GET_HW(priv), txcfg->phdr, txcfg->pstat, GET_MIB(priv)
#ifdef CONFIG_RTK_MESH	// For broadcast data frame via mesh (ex:ARP requst)
				, txcfg->is_11s
#endif
				);
			pdesc->Dword3 |= set_desc( (GetSequence(txcfg->phdr) & TX_SeqMask)  << TX_SeqSHIFT );
//			pdesc->Dword3 |= ( (GetSequence(txcfg->phdr) & TX_SeqMask));
/*
			{
				unsigned short seqnum = GetSequence(txcfg->phdr);
				unsigned int *ppdesc = (unsigned int *)pdesc;
				printk("seqnum: %x\n", seqnum);
				pdesc->Dword3 |= set_desc((seqnum&TX_SeqMask) << TX_SeqSHIFT);
				printk("Dword3 in singin: %x\n", get_desc(*(ppdesc+3)));
				printk("Dword3 in singin, no get_desc: %x\n",*(ppdesc+3));
			}
*/

//			printk("Dword3 in singin: %x\n", get_desc(*(unsigned int *)(&pdesc->Dword3)));
			pwlhdr = txcfg->phdr;
		}
		SetDuration(pwlhdr, 0);

		rtl_fill_fwinfo(priv, txcfg, pdesc, i);
#if 0
		// There is no fwinfo in 8192SE desgin, use TX desc instead
		{
			struct FWtemplate *ppfwinfo =  (struct FWtemplate *)pfwinfo;
			// so far, we cannot send cck rate, use 6Mhz instead.
			// for test, use fix rate
			// 92SE 1st cut can't use CCK rate, avoid.
//			printk("txRate: %d\n", ppfwinfo->txRate);

#ifdef RTL8192SE_ACUT
			if(ppfwinfo->txRate < 0x4)
				ppfwinfo->txRate = 0x4; // 6Mhz
#endif

//			if(txcfg->pstat)
//				pdesc->Dword1 |= set_desc(txcfg->pstat->aid && TX_MACIDMask);

			pdesc->Dword5 |= set_desc((ppfwinfo->txRate & TX_TxRateMask) << TX_TxRateSHIFT);
			pdesc->Dword4 |= (ppfwinfo->txHt == 1)? set_desc(TX_TXHT): 0;
			pdesc->Dword4 |= (ppfwinfo->txbw == 1)? set_desc(TX_TxBw): 0;
			pdesc->Dword4 |= set_desc((ppfwinfo->txSC & TX_TXSCMask) << TX_TXSCSHIFT);

//			printk("txHt: %x, txBW: %x, TxSC: %x, TxRate: %x\n",ppfwinfo->txHt, ppfwinfo->txbw, ppfwinfo->txSC, ppfwinfo->txRate);

//			pdesc->Dword4 |= (ppfwinfo->txshort == 1)?set_desc(TX_TxShort): 0;
//			pdesc->Dword4 |= set_desc(TX_TxShort);
//			pdesc->Dword2 |= (ppfwinfo->aggren == 1)?set_desc(TX_AggEn): 0;
			// force aggren
			pdesc->Dword2 |= set_desc(TX_AggEn);

			//set Break
			if(txcfg->pstat != priv->pshare->CurPstat){
				pdesc->Dword2 |= set_desc(TX_BK);
				priv->pshare->CurPstat = txcfg->pstat;
			}


//			pdesc->xxx = ppfwinfo->rxMF;
//			pdesc->xxx = ppfwinfo->rxAMD;
//			pdesc->xxx = ppfwinfo->retryLimit1;
//			pdesc->xxx = ppfwinfo->retryLimit2;
			//protection related
			pdesc->Dword4 |= (ppfwinfo->rtsEn == 1)? set_desc(TX_RTSEn): 0;
			pdesc->Dword4 |= (ppfwinfo->ctsEn == 1)? set_desc(TX_CTS2Self): 0;
			pdesc->Dword4 |= (ppfwinfo->rtsShort == 1)? set_desc(TX_RTSShort): 0;
//			pdesc->RTSSTBC = 0;	//RTS STBC mode... what is that ...?
			pdesc->Dword4 |= (ppfwinfo->rtsHt == 1)? set_desc(TX_RTSHT): 0;
			pdesc->Dword4 |= set_desc((ppfwinfo->rtsTxRate & TX_RTSRateMask) << TX_RTSRateSHIFT);//1 rate table is different from 8190, need to modify later!!
//			pdesc->RTSRate	= MRateToHwRate8192SE(Adapter, MGN_24M);
			pdesc->Dword4 |= (ppfwinfo->rtsbw == 1)? set_desc(TX_RTSBW): 0;
			pdesc->Dword4 |= set_desc((ppfwinfo->rtsSC & TX_RTSSCMask) << TX_RTSSCSHIFT);
			pdesc->Dword4 |= (ppfwinfo->rtsShort == 1)? set_desc(TX_RTSShort): 0;

			//fill necessary field in First Descriptor
//			pdesc->LINIP = 0;
			pdesc->Dword0 |= set_desc(32 << TX_OFFSETSHIFT); // tx_desc size
//			pdesc->Offset = USB_HWDESC_HEADER_LEN;
//			pdesc->PktSize = (u2Byte)PktLen;
		}

#endif
		pdesc->Dword0 |= set_desc(32 << TX_OFFSETSHIFT); // tx_desc size

		if (i != (txcfg->frg_num - 1))
		{
			SetMFrag(pwlhdr);
			if (i == 0) {
				fr_len = (txcfg->frag_thrshld - txcfg->llc);
				tx_len -= (txcfg->frag_thrshld - txcfg->llc);
			}
			else {
				fr_len = txcfg->frag_thrshld;
				tx_len -= txcfg->frag_thrshld;
			}
		}
		else	// last seg, or the only seg (frg_num == 1)
		{
			fr_len = tx_len;
			ClearMFrag(pwlhdr);
		}
		SetFragNum((pwlhdr), i);

		if ((i == 0) && (txcfg->fr_type == _SKB_FRAME_TYPE_))
		{
//			pdesc->flen = set_desc(txcfg->hdr_len + txcfg->llc + sizeof(struct FWtemplate));
//			pdesc->cmd = set_desc((fr_len + get_desc(pdesc->flen) - sizeof(struct FWtemplate)) | _CMDINIT_ | _FS_);
			pdesc->Dword7 |= set_desc((txcfg->hdr_len + txcfg->llc) << TX_TxBufferSizeSHIFT);
			pdesc->Dword0 |= set_desc((fr_len + (get_desc(pdesc->Dword7) & TX_TxBufferSizeMask)) << TX_PktSizeSHIFT);
			pdesc->Dword0 |= set_desc(TX_FirstSeg);
			pdescinfo->type = _PRE_ALLOCLLCHDR_;
		}
		else
		{
//			pdesc->flen = set_desc(txcfg->hdr_len + sizeof(struct FWtemplate));
//			pdesc->cmd = set_desc((fr_len + get_desc(pdesc->flen) - sizeof(struct FWtemplate))	| _CMDINIT_ | _FS_);
			pdesc->Dword7 |= set_desc(txcfg->hdr_len << TX_TxBufferSizeSHIFT);
			pdesc->Dword0 |= set_desc((fr_len + (get_desc(pdesc->Dword7) & TX_TxBufferSizeMask)) << TX_PktSizeSHIFT);
			pdesc->Dword0 |= set_desc(TX_FirstSeg);
			pdescinfo->type = _PRE_ALLOCHDR_;
		}


#ifdef _11s_TEST_MODE_
		mesh_debug_tx9(txcfg, pdescinfo);
#endif

//		pdesc->opt = set_desc(sizeof(struct FWtemplate));
//		pdesc->cmd |= set_desc((sizeof(struct FWtemplate) + 8) << _OFFSETSHIFT_);
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
			q_select = 0x11;// High Queue
			break;
#endif
		case MGNT_QUEUE:
			q_select = 0x12;
			break;
		default:
			// data packet
			q_select = ((struct sk_buff *)txcfg->pframe)->cb[1];
			break;
		}
//		pdesc->opt |= set_desc(q_select << _QSELECTSHIFT_);
/*
#if	defined(RTL8192SE)
		switch(q_select){
			case 0x01:
				q_select = 0x02;
				break;
			default:
				break;
		}

#endif
*/
		pdesc->Dword1 |= set_desc((q_select & TX_QueueSelMask)<< TX_QueueSelSHIFT);

		if (i != (txcfg->frg_num - 1))
			pdesc->Dword1 |= set_desc(TX_MoreFrag);
//			pdesc->opt |= set_desc(_MFRAG_);

		// Set MacID
		if (txcfg->pstat) {
			if (txcfg->pstat->aid != MANAGEMENT_AID)	{
//				pdesc->opt |= set_desc((txcfg->pstat->aid & 0x0007) << _RATIDSHIFT_);
//				((struct FWtemplate *)pfwinfo)->rsvd0 = (txcfg->pstat->aid & 0x0038) >> 3;
#ifdef STA_EXT
				pdesc->Dword1 |= set_desc(txcfg->pstat->remapped_aid & TX_MACIDMask);
#else
				pdesc->Dword1 |= set_desc(txcfg->pstat->aid & TX_MACIDMask);
#endif
			}
		}
//		else {
//			pdesc->opt |= set_desc((7 & 0x0007) << _RATIDSHIFT_);
//			((struct FWtemplate *)pfwinfo)->rsvd0 = (7 & 0x0038) >> 3;
//		}


/*
		if (
#ifdef WDS
				(txcfg->pstat && (txcfg->pstat->state & WIFI_WDS) &&
					priv->pmib->dot11WdsInfo.entry[txcfg->pstat->wds_idx].txRate) ||
				(txcfg->pstat && !(txcfg->pstat->state & WIFI_WDS) &&
						!priv->pmib->dot11StationConfigEntry.autoRate) ||
#else
				(!priv->pmib->dot11StationConfigEntry.autoRate) ||
#endif
				(txcfg->pstat == NULL) ||
				(txcfg->fr_type != _SKB_FRAME_TYPE_))
			pdesc->opt |= set_desc(_USERATE_ | _DISFB_);
*/

//		pdesc->Dword4 |= set_desc(TX_UserRate);
		pdesc->Dword5 |= set_desc((0x1f) << TX_DataRateFBLmtSHIFT);
//		pdesc->Dword5 |= set_desc(TX_DISFB);
		if (txcfg->fixed_rate)
		{
			pdesc->Dword4 |= set_desc(TX_UserRate);
			pdesc->Dword5 |= set_desc(TX_DISFB);
			pdesc->Dword4 |= set_desc(TX_DisRTSFB);// disable RTS fall back
		}
#ifdef STA_EXT
		if(txcfg->pstat && txcfg->pstat->remapped_aid == FW_NUM_STAT-1/*(priv->pshare->STA_map & BIT(txcfg->pstat->aid)*/){
			pdesc->Dword4 |= set_desc(TX_UserRate);
		}
#endif

		if (!txcfg->need_ack && txcfg->privacy && UseSwCrypto(priv, NULL, TRUE))
			pdesc->Dword1 &= set_desc( ~(TX_SecTypeMask<< TX_SecTypeSHIFT));
//			pdesc->opt |= set_desc(_NOENC_);

		if (txcfg->privacy)
		{
			if (UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))
			{
//				pdesc->cmd = set_desc(get_desc(pdesc->cmd) + txcfg->icv + txcfg->mic + txcfg->iv);
//				pdesc->flen = set_desc(get_desc(pdesc->flen) + txcfg->iv);
				pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0)+ txcfg->icv + txcfg->mic + txcfg->iv);
				pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7)+ txcfg->iv);
			}
			else // hw encrypt
			{
				switch(txcfg->privacy)
				{
				case _WEP_104_PRIVACY_:
				case _WEP_40_PRIVACY_:
//					pdesc->cmd = set_desc(get_desc(pdesc->cmd) + txcfg->iv);
//					pdesc->flen = set_desc(get_desc(pdesc->flen) + txcfg->iv);
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					wep_fill_iv(priv, pwlhdr, txcfg->hdr_len, keyid);
					//pdesc->opt = set_desc(get_desc(pdesc->opt) | BIT(30));
					pdesc->Dword1 |= set_desc(0x1 << TX_SecTypeSHIFT);
					break;

				case _TKIP_PRIVACY_:
//					pdesc->cmd = set_desc(get_desc(pdesc->cmd) + txcfg->iv + txcfg->mic);
//					pdesc->flen = set_desc(get_desc(pdesc->flen) + txcfg->iv);
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv + txcfg->mic);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					tkip_fill_encheader(priv, pwlhdr, txcfg->hdr_len, keyid);
					//pdesc->opt = set_desc(get_desc(pdesc->opt) | BIT(31));
					pdesc->Dword1 |= set_desc(0x2 << TX_SecTypeSHIFT);
					// cam id is no need to sign in at present, victoryman 20081130
//					if ((!IS_MCAST(da)) && (txcfg->pstat))
//						pdesc->Dword1 |= set_desc((txcfg->pstat->cam_id & TX_MACIDMask)<< TX_MACIDSHIFT);  // am i right??
//						pdesc->opt |= set_desc(BIT(29) | ((txcfg->pstat->cam_id) & 0x1f)<<24);
					break;

				case _CCMP_PRIVACY_:
					//michal also hardware...
//					pdesc->cmd = set_desc(get_desc(pdesc->cmd) + txcfg->iv);
//					pdesc->flen = set_desc(get_desc(pdesc->flen) + txcfg->iv);
					pdesc->Dword0 = set_desc(get_desc(pdesc->Dword0) + txcfg->iv);
					pdesc->Dword7 = set_desc(get_desc(pdesc->Dword7) + txcfg->iv);
					aes_fill_encheader(priv, pwlhdr, txcfg->hdr_len, keyid);
//					pdesc->opt = set_desc(get_desc(pdesc->opt) | BIT(31) | BIT(30));
					pdesc->Dword1 |= set_desc(0x3 << TX_SecTypeSHIFT);
					// cam id is no need to sign in at present, victoryman 20081130
//					if ((!IS_MCAST(da)) && (txcfg->pstat))
//						pdesc->Dword1 |= set_desc((txcfg->pstat->cam_id & TX_MACIDMask)<< TX_MACIDSHIFT);
//						pdesc->opt |= set_desc(BIT(29) | ((txcfg->pstat->cam_id) & 0x1f)<<24);
					break;

				default:
					//DEBUG_ERR("Unknow privacy\n");
					break;
				}
			}
		}
//		pdesc->paddr = set_desc(get_physical_addr(priv, pwlhdr-sizeof(struct FWtemplate),
//			(get_desc(pdesc->flen)&0xffff), PCI_DMA_TODEVICE));
		// we don't have fwinfo for 8192SE tx
		//TxBufferAddr


		pdesc->Dword8 = set_desc(get_physical_addr(priv, pwlhdr,
			(get_desc(pdesc->Dword7)&0xffff), PCI_DMA_TODEVICE));

		// below is for sw desc info
		pdescinfo->paddr  = get_desc(pdesc->Dword8);
		pdescinfo->pframe = pwlhdr;
#if defined(SEMI_QOS) && defined(WMM_APSD)
		pdescinfo->priv = priv;
		pdescinfo->pstat = txcfg->pstat;
#endif

#ifdef TX_SHORTCUT
		if (!priv->pmib->dot11OperationEntry.disable_txsc && txcfg->pstat &&
				(txcfg->fr_type == _SKB_FRAME_TYPE_)) {
			desc_copy(&txcfg->pstat->hwdesc1, pdesc);
			descinfo_copy(&txcfg->pstat->swdesc1, pdescinfo);
			txcfg->pstat->protection = priv->pmib->dot11ErpInfo.protection;
			txcfg->pstat->sc_keyid = keyid;
			txcfg->pstat->pktpri = ((struct sk_buff *)txcfg->pframe)->cb[1];
//			memcpy(&txcfg->pstat->fw_bckp, pfwinfo, sizeof(struct FWtemplate));
			fit_shortcut = 1;
		}
		else {
			if (txcfg->pstat)
					txcfg->pstat->hwdesc1.Dword7 &= ~TX_TxBufferSizeMask;
		}
#endif

		pfrst_dma_desc = dma_txhead[*tx_head];

		if (i != 0) {
//			pdesc->OWN = 1;
			pdesc->Dword0 |= set_desc(TX_OWN);
#ifndef USE_RTL8186_SDK
			rtl_cache_sync_wback(priv, dma_txhead[*tx_head], sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
		}

#ifdef USE_RTL8186_SDK
		flush_addr[flush_num]=(unsigned long)bus_to_virt(get_desc(pdesc->Dword8)); //TxBufferAddr
		flush_len[flush_num++]= (get_desc(pdesc->Dword7) & TX_TxBufferSizeMask);
#endif

/*
		//printk desc content
		{
			unsigned int *ppdesc = (unsigned int *)pdesc;
			printk("%08x    %08x    %08x \n", get_desc(*(ppdesc)), get_desc(*(ppdesc+1)), get_desc(*(ppdesc+2)));
			printk("%08x    %08x    %08x \n", get_desc(*(ppdesc+3)), get_desc(*(ppdesc+4)), get_desc(*(ppdesc+5)));
			printk("%08x    %08x    %08x \n", get_desc(*(ppdesc+6)), get_desc(*(ppdesc+7)), get_desc(*(ppdesc+8)));
			printk("%08x\n", *(ppdesc+9));
			printk("===================================================\n");
		}
*/

		rtl_txdesc_rollover(pdesc, (unsigned int *)tx_head);

		if (txcfg->fr_len == 0)
		{
			if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST)
				pdesc->Dword0 |= set_desc(TX_LastSeg);
//				pdesc->LastSeg= 1;
//			  pdesc->cmd |= set_desc(_LS_);
			goto init_deschead;
		}

		/*------------------------------------------------------------*/
		/*              fill descriptor of frame body                 */
		/*------------------------------------------------------------*/
		pndesc     = phdesc + *tx_head;
		pndescinfo = pswdescinfo + *tx_head;
		//clear all bits
		memset(pndesc, 0,32);
/*
		pndesc->cmd	= set_desc((get_desc(pdesc->cmd) & (~_FS_)) | _OWN_);
		pndesc->rsvd0 = pdesc->rsvd0;
		pndesc->rsvd1 = pdesc->rsvd1;
		pndesc->rsvd2 = pdesc->rsvd2;
*/

		pndesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | (TX_OWN));
//		pndesc->FirstSeg= 0;
//		pndesc->OWN = 1;

		if (txcfg->privacy)
		{
			if (txcfg->privacy == _WAPI_SMS4_)
			{
				if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST)
					pndesc->Dword0 |= set_desc(TX_LastSeg);
//					pndesc->LastSeg = 1;
//				  pndesc->cmd |= set_desc(_LS_);
				pndescinfo->pstat = txcfg->pstat;
				pndescinfo->rate = txcfg->tx_rate;
			}
			else if (!UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))
			{
				if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST)
					pndesc->Dword0 |= set_desc(TX_LastSeg);
//					pndesc->LastSeg = 1;
//				  pndesc->cmd |= set_desc(_LS_);
				pndescinfo->pstat = txcfg->pstat;
				pndescinfo->rate = txcfg->tx_rate;
			}
		}
		else
		{
			if (txcfg->aggre_en != FG_AGGRE_MSDU_FIRST)
				pndesc->Dword0 |= set_desc(TX_LastSeg);
//			  pndesc->cmd |= set_desc(_LS_);
			pndescinfo->pstat = txcfg->pstat;
			pndescinfo->rate = txcfg->tx_rate;
		}

//		pndesc->flen = set_desc(fr_len);
#if defined(CONFIG_RTL_WAPI_SUPPORT)
		if (txcfg->privacy == _WAPI_SMS4_)
			pndesc->Dword7 |= set_desc( (fr_len+SMS4_MIC_LEN) & TX_TxBufferSizeMask);
		else
#endif
			pndesc->Dword7 |= set_desc(fr_len & TX_TxBufferSizeMask);

		if (i == 0)
			pndescinfo->type = txcfg->fr_type;
		else
			pndescinfo->type = _RESERVED_FRAME_TYPE_;

#if defined(CONFIG_RTK_MESH) && defined(MESH_USE_METRICOP)
		if( (txcfg->fr_type == _PRE_ALLOCMEM_) && (txcfg->is_11s & 128)) // for 11s link measurement frame
			pndescinfo->type =_RESERVED_FRAME_TYPE_;
#endif

#ifdef _11s_TEST_MODE_
		mesh_debug_tx10(txcfg, pndescinfo);
#endif

//		pndesc->paddr = set_desc(tmpphyaddr);
		pndesc->Dword8 = set_desc(tmpphyaddr); //TxBufferAddr
		pndescinfo->paddr = get_desc(pndesc->Dword8);
		pndescinfo->pframe = txcfg->pframe;
		pndescinfo->len = txcfg->fr_len;	// for pci_unmap_single
		pndescinfo->priv = priv;

		pbuf += fr_len;
		tmpphyaddr += fr_len;

#ifdef TX_SHORTCUT
		if (fit_shortcut) {
			desc_copy(&txcfg->pstat->hwdesc2, pndesc);
			descinfo_copy(&txcfg->pstat->swdesc2, pndescinfo);
		}
#endif

#ifndef USE_RTL8186_SDK
		rtl_cache_sync_wback(priv, dma_txhead[*tx_head], sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

		flush_addr[flush_num]=(unsigned long)bus_to_virt(get_desc(pndesc->Dword8));
		flush_len[flush_num++]=get_desc(pndesc->Dword7) & TX_TxBufferSizeMask; // TxBufferSize

		rtl_txdesc_rollover(pndesc, (unsigned int *)tx_head);

		// retrieve H/W MIC and put in payload
#if defined(CONFIG_RTL_WAPI_SUPPORT)
		if (txcfg->privacy == _WAPI_SMS4_)
		{
			SecSWSMS4Encryption(priv, txcfg);
		} else
#endif
		{
		if ((txcfg->privacy == _TKIP_PRIVACY_) &&
			(priv->pshare->have_hw_mic) &&
			!(priv->pmib->dot11StationConfigEntry.swTkipMic) &&
			(i == (txcfg->frg_num-1)) )	// last segment
			{
				register unsigned long int l,r;
				unsigned char *mic;
				volatile int i;

				while ((*(volatile unsigned int *)GDMAISR & GDMA_COMPIP) == 0)
					for (i=0; i<10; i++)
						;

				l = *(volatile unsigned int *)GDMAICVL;
				r = *(volatile unsigned int *)GDMAICVR;

				mic = ((struct sk_buff *)txcfg->pframe)->data + txcfg->fr_len - 8;
				mic[0] = (unsigned char)(l & 0xff);
				mic[1] = (unsigned char)((l >> 8) & 0xff);
				mic[2] = (unsigned char)((l >> 16) & 0xff);
				mic[3] = (unsigned char)((l >> 24) & 0xff);
				mic[4] = (unsigned char)(r & 0xff);
				mic[5] = (unsigned char)((r >> 8) & 0xff);
				mic[6] = (unsigned char)((r >> 16) & 0xff);
				mic[7] = (unsigned char)((r >> 24) & 0xff);

#ifdef MICERR_TEST
				if (priv->micerr_flag) {
					mic[7] ^= mic[7];
					priv->micerr_flag = 0;
				}
#endif
			}
		}
		/*------------------------------------------------------------*/
		/*                insert sw encrypt here!                     */
		/*------------------------------------------------------------*/
		if (txcfg->privacy && UseSwCrypto(priv, txcfg->pstat, (txcfg->pstat ? FALSE : TRUE)))
		{
			if (txcfg->privacy == _TKIP_PRIVACY_ ||
				txcfg->privacy == _WEP_40_PRIVACY_ ||
				txcfg->privacy == _WEP_104_PRIVACY_)
			{
				picvdesc     = phdesc + *tx_head;
				picvdescinfo = pswdescinfo + *tx_head;
				//clear all bits
				memset(picvdesc, 0,32);

				if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST){
//					memcpy(picvdesc, pdesc, sizeof(struct _TX_DESC_8192SE));
//					picvdesc->FirstSeg = 0;
//					picvdesc->OWN = 1;
					picvdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN);
//				  picvdesc->cmd = set_desc((get_desc(pdesc->cmd) & (~_FS_)) | _OWN_);
				}
				else{
//					memcpy(picvdesc, pdesc, sizeof(struct _TX_DESC_8192SE));
//					picvdesc->FirstSeg = 0;
//					picvdesc->LastSeg = 1;
//					picvdesc->OWN = 1;
					picvdesc->Dword0   = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN | TX_LastSeg);
//	  			  picvdesc->cmd   = set_desc((get_desc(pdesc->cmd) & (~_FS_)) | _OWN_ | _LS_);
				}


//				picvdesc->flen  = set_desc(txcfg->icv);
//				picvdesc->rsvd0 = pdesc->rsvd0;
//				picvdesc->rsvd1 = pdesc->rsvd1;
//				picvdesc->rsvd2 = pdesc->rsvd2;
				picvdesc->Dword7 |= (set_desc(txcfg->icv & TX_TxBufferSizeMask)); //TxBufferSize

				// append ICV first...
				picv = get_icv_from_poll(priv);
				if (picv == NULL)
				{
					//DEBUG_ERR("System-Buf! can't alloc picv\n");
					BUG();
				}

				picvdescinfo->type = _PRE_ALLOCICVHDR_;
				picvdescinfo->pframe = picv;
				picvdescinfo->pstat = txcfg->pstat;
				picvdescinfo->rate = txcfg->tx_rate;
				picvdescinfo->priv = priv;
//				picvdesc->paddr = set_desc(get_physical_addr(priv, picv, txcfg->icv, PCI_DMA_TODEVICE));
				//TxBufferAddr
				picvdesc->Dword8 = set_desc(get_physical_addr(priv, picv, txcfg->icv, PCI_DMA_TODEVICE));
#ifdef USE_RTL8186_SDK
				rtl_cache_sync_wback(priv, (unsigned int)picv, txcfg->icv, PCI_DMA_TODEVICE);
#endif

				if (i == 0)
					tkip_icv(picv, \
						pwlhdr + txcfg->hdr_len + txcfg->iv, txcfg->llc, \
						pbuf - (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask));
				else
					tkip_icv(picv, \
						NULL, 0, \
						pbuf - (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask));

				if ((i == 0) && (txcfg->llc != 0)) {
					if (txcfg->privacy == _TKIP_PRIVACY_)
						tkip_encrypt(priv, pwlhdr, txcfg->hdr_len, \
							pwlhdr + txcfg->hdr_len + 8, sizeof(struct llc_snap), \
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), picv, txcfg->icv);
					else
						wep_encrypt(priv, pwlhdr, txcfg->hdr_len, \
							pwlhdr + txcfg->hdr_len + 4, sizeof(struct llc_snap), \
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), picv, txcfg->icv,
							txcfg->privacy);
				}
				else { // not first segment or no snap header
					if (txcfg->privacy == _TKIP_PRIVACY_)
						tkip_encrypt(priv, pwlhdr, txcfg->hdr_len, NULL, 0, \
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), picv, txcfg->icv);
					else
						wep_encrypt(priv, pwlhdr, txcfg->hdr_len, NULL, 0, \
							pbuf - (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), picv, txcfg->icv,
							txcfg->privacy);
				}
#ifndef USE_RTL8186_SDK
				rtl_cache_sync_wback(priv, dma_txhead[*tx_head], sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

				flush_addr[flush_num]=(unsigned long)bus_to_virt(get_desc(picvdesc->Dword8));//TxBufferAddr
				flush_len[flush_num++]=(get_desc(picvdesc->Dword7) & TX_TxBufferSizeMask);

				rtl_txdesc_rollover(picvdesc, (unsigned int *)tx_head);
			}

			else if (txcfg->privacy == _CCMP_PRIVACY_)
			{
				pmicdesc = phdesc + *tx_head;
				pmicdescinfo = pswdescinfo + *tx_head;
				if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST)
					pmicdesc->Dword0 = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN);
//				  pmicdesc->cmd = set_desc((get_desc(pdesc->cmd) & (~_FS_)) | _OWN_);
				else
				  pmicdesc->Dword0   = set_desc((get_desc(pdesc->Dword0) & (~TX_FirstSeg)) | TX_OWN | TX_LastSeg);
//				  pmicdesc->cmd   = set_desc((get_desc(pdesc->cmd) & (~_FS_)) | _OWN_ | _LS_);
//				pmicdesc->rsvd0 = pdesc->rsvd0;
//				pmicdesc->rsvd1 = pdesc->rsvd1;
//				pmicdesc->rsvd2 = pdesc->rsvd2;

				// set TxBufferSize
				pmicdesc->Dword7 = set_desc(txcfg->mic & TX_TxBufferSizeMask);

				// append MIC first...
				pmic = get_mic_from_poll(priv);
				if (pmic == NULL)
				{
					//DEBUG_ERR("System-Buf! can't alloc pmic\n");
					BUG();
				}

				pmicdescinfo->type = _PRE_ALLOCMICHDR_;
				pmicdescinfo->pframe = pmic;
				pmicdescinfo->pstat = txcfg->pstat;
				pmicdescinfo->rate = txcfg->tx_rate;
				pmicdescinfo->priv = priv;
				// set TxBufferAddr
				pmicdesc->Dword8= set_desc(get_physical_addr(priv, pmic, txcfg->mic, PCI_DMA_TODEVICE));
#ifdef USE_RTL8186_SDK
				rtl_cache_sync_wback(priv, (unsigned int)pmic, txcfg->mic, PCI_DMA_TODEVICE);
#endif

				// then encrypt all (including ICV) by AES
				if (i == 0) // encrypt 3 segments ==> llc, mpdu, and mic
					aesccmp_encrypt(priv, pwlhdr, pwlhdr + txcfg->hdr_len + 8,
						pbuf - (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), pmic);
				else // encrypt 2 segments ==> mpdu and mic
					aesccmp_encrypt(priv, pwlhdr, NULL,
						pbuf - (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), (get_desc(pndesc->Dword7) & TX_TxBufferSizeMask), pmic);
#ifndef USE_RTL8186_SDK
				rtl_cache_sync_wback(priv, dma_txhead[*tx_head], sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
				flush_addr[flush_num]=(unsigned long)bus_to_virt(get_desc(pmicdesc->Dword8));
				flush_len[flush_num++]= (get_desc(pmicdesc->Dword7) & TX_TxBufferSizeMask);

				rtl_txdesc_rollover(pmicdesc, (unsigned int *)tx_head);
			}
		}
	}

init_deschead:
	for (i=0; i<flush_num; i++)
		rtl_cache_sync_wback(priv, flush_addr[i], flush_len[i], PCI_DMA_TODEVICE);

	if (txcfg->aggre_en == FG_AGGRE_MSDU_FIRST) {
		priv->amsdu_first_desc = pfrstdesc;
#ifndef USE_RTL8186_SDK
		priv->amsdu_first_dma_desc = pfrst_dma_desc;
#endif
		priv->amsdu_len = get_desc(pfrstdesc->Dword0) & 0xffff; // get pktSize
		return -1;	// return because of AMSDU
	}

#ifdef BUFFER_TX_AMPDU
	if ((txcfg->aggre_en >= FG_AGGRE_MPDU_BUFFER_FIRST) &&
					(txcfg->aggre_en <= FG_AGGRE_MPDU_BUFFER_LAST)) {
		if (txcfg->aggre_en == FG_AGGRE_MPDU_BUFFER_FIRST) {
			priv->ampdu_first_desc = pfrstdesc;
#ifndef USE_RTL8186_SDK
			priv->ampdu_first_dma_desc = pfrst_dma_desc;
#endif
		}
		else {
			//pfrstdesc->cmd |= set_desc(_OWN_);
			pfrstdesc->Dword0 |= set_desc(TX_OWN);
#ifndef USE_RTL8186_SDK
			rtl_cache_sync_wback(priv, pfrst_dma_desc, sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
			if (txcfg->aggre_en == FG_AGGRE_MPDU_BUFFER_LAST) {
				pfrstdesc = priv->ampdu_first_desc;
				pfrstdesc->Dword0 |= set_desc(TX_OWN);

#ifndef USE_RTL8186_SDK
				rtl_cache_sync_wback(priv, priv->ampdu_first_dma_desc, sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif
				tx_poll(priv, q_num);
			}
		}
		return -2;	// return because of AMPDU
	}
#endif // BUFFER_TX_AMPDU

	//	pfrstdesc->cmd |= set_desc(_OWN_);
	pfrstdesc->Dword0 |= set_desc(TX_OWN);
#ifndef USE_RTL8186_SDK
	rtl_cache_sync_wback(priv, pfrst_dma_desc, sizeof(struct tx_desc), PCI_DMA_TODEVICE);
#endif

	return 0;
}
#endif

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
void fill_tx_desc(struct rtl8190_priv *priv, struct tx_insn *txcfg)
#endif
{
	static int *tx_head, q_num, q_select, *tx_tail, i, totol_len, n_txshort;
	static struct rtl8190_hw	*phw;
	static struct tx_desc  *phdesc, *pdesc, *pfrstdesc;
	static struct tx_desc_info *pswdescinfo, *pdescinfo;
#if !defined(NEW_MAC80211_DRV) && defined(TX_SHORTCUT)
	static struct tx_desc_info *pfrstdescinfo;
#endif

#ifdef NEW_MAC80211_DRV
	static struct tx_info *fminfo;
#else
	static struct priv_shared_info *fminfo;
#endif
	static unsigned char *pwlhdr;
	static struct tx_desc  *pfrstdesc_frag;	/* first desc of flagment list */
	int rc=0;

#ifdef NEW_MAC80211_DRV
	fminfo = txinfo;
#else
	fminfo = priv->pshare;
#endif

#ifdef NEW_MAC80211_DRV
	ASSERT(txinfo->buf_num > 0);
	ASSERT(txinfo->tx_rate >= CCK_1M && txinfo->tx_rate <= MCS15);
	ASSERT(txinfo->q_num <= HIGH_QUEUE);
	ASSERT(txinfo->priority <= 7);
	q_num = txinfo->q_num;
#else
	q_num = txcfg->q_num;
#endif

	phw	= GET_HW(priv);
	tx_head = get_txhead_addr(phw, q_num);
	tx_tail = get_txtail_addr(phw, q_num);

//	printk("head %u, tail %u <<<>>> %x %x\n", *tx_head, *tx_tail, tx_head, tx_tail);

	phdesc = get_txdesc(phw, q_num);
#ifdef NEW_MAC80211_DRV
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

	rtl_fill_fwinfo(priv, txinfo, pdesc);

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

	pdesc->Dword1 |= set_desc(((q_select & TX_QueueSelMask) \
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

	pdesc->Dword8 = set_desc(get_physical_addr(priv, pwlhdr, \
										txinfo->buf[0].len, PCI_DMA_TODEVICE));

	RECORD_FREE_INFO(pdescinfo, (&txinfo->buf[0]))
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
				rtl_txdesc_rollover(pdesc, (unsigned int *)tx_head);
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

#if defined(TX_SHORTCUT)
	if (txinfo->pstat) {
		desc_copy(&txinfo->pstat->hwdesc1, pfrstdesc);
		descinfo_copy(&txinfo->pstat->swdesc1, pfrstdescinfo);
		desc_copy(&txinfo->pstat->hwdesc2, pdesc);
		descinfo_copy(&txinfo->pstat->swdesc2, pdescinfo);
	}
#endif

	//pdesc->Dword0 |= set_desc(TX_LastSeg);
	rtl_cache_sync_wback(priv, (unsigned int)pdesc, sizeof(struct tx_desc), PCI_DMA_TODEVICE);
	rtl_txdesc_rollover(pdesc, (unsigned int *)tx_head);

#if defined(SUPPORT_TX_AMSDU)
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
			/*if (txcfg->need_ack == 0){
				printk("tx Dw0 >> %08x\n", pdesc->Dword0);
				printk("tx Dw1 >> %08x\n", pdesc->Dword1);
				printk("tx Dw2 >> %08x\n", pdesc->Dword2);
				printk("tx Dw3 >> %08x\n", pdesc->Dword3);
				printk("tx Dw4 >> %08x\n", pdesc->Dword4);
				printk("tx Dw5 >> %08x\n", pdesc->Dword5);
				printk("tx Dw6 >> %08x\n", pdesc->Dword6);
				printk("tx Dw7 >> %08x\n", pdesc->Dword7);
				printk("tx Dw8 >> %08x\n", pdesc->Dword8);
			
			}
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
			rtl_tx_poll(priv, q_num);
	}
#else // !NEW_MAC80211_DRV
	rc = generate_legacy_phdr(priv, txcfg);
	if( rc != 0 )	// means AMSDU_FIRST
		return;

#ifndef SW_MCAST
	if (q_num == HIGH_QUEUE) {
		priv->pkt_in_dtimQ = 1;
		return;
	}
	else
#endif
	{
		tx_poll(priv, q_num);
	}
#endif //NEW_MAC80211_DRV
	return;
}

#if 0
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

	rtl_txdesc_rollover(pdesc, (unsigned int *)tx_head);
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

	rtl_txdesc_rollover(pdesc, (unsigned int *)tx_head);

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
#endif

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

	rtl_txdesc_rollover(pdesc, (unsigned int *)tx_head);

	priv->amsdu_len += tx_len;

	if (txcfg->aggre_en == FG_AGGRE_MSDU_LAST) {
		pfrstdesc = priv->amsdu_first_desc;
		pfrstdesc->Dword0 = set_desc((get_desc(pfrstdesc->Dword0) &0xff0000) | priv->amsdu_len | TX_FirstSeg | TX_OWN);
		tx_poll(priv, q_num);
	}
}
#endif // SUPPORT_TX_AMSDU
#endif	//ifdef RTL8192SE

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


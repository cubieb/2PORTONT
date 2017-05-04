/*
 *  RX handle routines

 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/mac80211.h>


#include "./8190n_cfg.h"
#include "./8190n.h"
#include "./8190n_hw.h"
#include "./8190n_headers.h"
#include "./8190n_rx.h"
#include "./8190n_debug.h"




static unsigned char QueryRxPwrPercentage(signed char AntPower)
{
	if ((AntPower <= -100) || (AntPower >= 20))
		return	0;
	else if (AntPower >= 0)
		return	100;
	else
		return	(100+AntPower);

}

static unsigned char EVMdbToPercentage(signed char Value)
{
	signed char ret_val;

	ret_val = Value;

	if (ret_val >= 0)
		ret_val = 0;
	if (ret_val <= -33)
		ret_val = -33;
	ret_val = 0 - ret_val;
	ret_val*=3;
	if (ret_val == 99)
		ret_val = 100;
	return(ret_val);
}


static __inline__ void translate_rssi_sq(struct mac80211_shared_priv *priv, struct rx_frinfo *pfrinfo)
{
	typedef signed char		s1Byte;
	typedef unsigned char	u1Byte;
	typedef int				s4Byte;
	typedef unsigned int	u4Byte;

	PHY_STS_OFDM_8190_T	*pOfdm_buf;
	PHY_STS_CCK_8190_T	*pCck_buf;
	u1Byte				*prxpkt;
	u1Byte				i, Max_spatial_stream, tmp_rxsnr, tmp_rxevm, tmp_rxrssi;
	s1Byte				rx_pwr[4], rx_pwr_all=0;
	s1Byte				rx_snrX, rx_evmX, rx_rssiX;
	u1Byte				EVM, PWDB_ALL;
	u4Byte				RSSI;
	u1Byte				isCCKrate=0, report;

	/* 2007/07/04 MH For OFDM RSSI. For high power or not. */
	static	u1Byte		check_reg824 = 0;
	static	u4Byte		reg824_bit9 = 0;

	isCCKrate = is_CCK_rate(pfrinfo->rx_rate);

	/*2007.08.30 requested by SD3 Jerry */
	if (check_reg824 == 0) {
		reg824_bit9 = PHY_QueryBBReg(priv, rFPGA0_XA_HSSIParameter2, 0x200);
		check_reg824 = 1;
	}

	prxpkt = (u1Byte *)pfrinfo->driver_info;

	/* Initial the cck and ofdm buffer pointer */
	pCck_buf = (PHY_STS_CCK_8190_T *)prxpkt;
	pOfdm_buf = (PHY_STS_OFDM_8190_T *)prxpkt;

	memset(&pfrinfo->rf_info, 0, sizeof(struct rf_misc_info));
	pfrinfo->rf_info.mimosq[0] = -1;
	pfrinfo->rf_info.mimosq[1] = -1;

	if (isCCKrate)
	{
		//
		// (1)Hardware does not provide RSSI for CCK
		//
#ifdef RTL8190
		if ((get_rf_mimo_mode(priv) == MIMO_2T4R) && (priv->pshare_hw->rf_ft_var.cck_sel_ver == 2)) {
			for (i=RF90_PATH_A; i<RF90_PATH_MAX; i++) {
				tmp_rxrssi = pCck_buf->adc_pwdb_X[i];
				rx_rssiX = (s1Byte)(tmp_rxrssi);
				rx_rssiX /= 2;
				pfrinfo->cck_mimorssi[i] = rx_rssiX;
			}
		}
#endif

		//
		// (2)PWDB, Average PWDB cacluated by hardware (for rate adaptive)
		//
		if (!reg824_bit9)
		{
			report = pCck_buf->cck_agc_rpt & 0xc0;
			report = report>>6;
			switch (report)
			{
				//Fixed by Jacken from Bryant 2008-03-20
				//Original value is -38 , -26 , -14 , -2
				//Fixed value is -35 , -23 , -11 , 6
				case 0x3:
					rx_pwr_all = -35 - (pCck_buf->cck_agc_rpt & 0x3e);
					break;
				case 0x2:
					rx_pwr_all = -23 - (pCck_buf->cck_agc_rpt & 0x3e);
					break;
				case 0x1:
					rx_pwr_all = -11 - (pCck_buf->cck_agc_rpt & 0x3e);
					break;
				case 0x0:
					rx_pwr_all = 8 - (pCck_buf->cck_agc_rpt & 0x3e);
					break;
			}
		}
		else
		{
			report = pCck_buf->cck_agc_rpt & 0x60;
			report = report>>5;
			switch (report)
			{
				//Fixed by Jacken from Bryant 2008-03-20
				case 0x3:
					rx_pwr_all = -35 - ((pCck_buf->cck_agc_rpt & 0x1f)<<1);
					break;
				case 0x2:
					rx_pwr_all = -23 - ((pCck_buf->cck_agc_rpt & 0x1f)<<1);
					break;
				case 0x1:
					rx_pwr_all = -11 - ((pCck_buf->cck_agc_rpt & 0x1f)<<1);
					break;
				case 0x0:
					rx_pwr_all = -8 - ((pCck_buf->cck_agc_rpt & 0x1f)<<1);
					break;
			}
		}

		PWDB_ALL = QueryRxPwrPercentage(rx_pwr_all);
		pfrinfo->rssi = PWDB_ALL;
		pfrinfo->rssi+=3;

#ifdef NEW_MAC80211_DRV
		pfrinfo->rx_pwr_all = rx_pwr_all;
#endif 
		//
		// (3) Get Signal Quality (EVM)
		//
		// if(bPacketMatchBSSID)
		{
			u1Byte SQ;

			if (pfrinfo->rssi > 40)
				SQ = 100;
			else
			{
				SQ = pCck_buf->SQ_rpt;

				if (pCck_buf->SQ_rpt > 64)
					SQ = 0;
				else if (pCck_buf->SQ_rpt < 20)
					SQ = 100;
				else
					SQ = ((64-SQ) * 100) / 44;
			}
			pfrinfo->sq = SQ;
			pfrinfo->rf_info.mimosq[0] = SQ;
		}
	}
	else
	{
		//
		// (1)Get RSSI for HT rate
		//

		for (i=RF90_PATH_A; i<RF90_PATH_C; i++)
		{
			rx_pwr[i] = ((pOfdm_buf->trsw_gain_X[i]&0x3F)*2) - 106;

			//Get Rx snr value in DB
			if (priv->pshare_hw->rf_ft_var.rssi_dump) {
				tmp_rxsnr =	pOfdm_buf->rxsnr_X[i];
				rx_snrX = (s1Byte)(tmp_rxsnr);
				rx_snrX >>= 1;;
				pfrinfo->rf_info.RxSNRdB[i] = (s4Byte)rx_snrX;
			}

			/* Translate DBM to percentage. */
			RSSI = QueryRxPwrPercentage(rx_pwr[i]);
			//total_rssi += RSSI;

			/* Record Signal Strength for next packet */
			//if(bPacketMatchBSSID)
			{
				pfrinfo->rf_info.mimorssi[i] = (u1Byte)RSSI;
			}
		}

		//
		// (2)PWDB, Average PWDB cacluated by hardware (for rate adaptive)
		//
		rx_pwr_all = (((pOfdm_buf->pwdb_all ) >> 1 )& 0x7f) -106;
		PWDB_ALL = QueryRxPwrPercentage(rx_pwr_all);

		pfrinfo->rssi = PWDB_ALL;

		//
		// (3)EVM of HT rate
		//
		if ((pfrinfo->rx_rate >= 0x88) && (pfrinfo->rx_rate <= 0x8f))
			Max_spatial_stream = 2; //both spatial stream make sense
		else
			Max_spatial_stream = 1; //only spatial stream 1 makes sense

		for (i=0; i<Max_spatial_stream; i++)
		{
			tmp_rxevm =	pOfdm_buf->rxevm_X[i];
			rx_evmX = (s1Byte)(tmp_rxevm);

			// Do not use shift operation like "rx_evmX >>= 1" because the compilor of free build environment
			// fill most significant bit to "zero" when doing shifting operation which may change a negative
			// value to positive one, then the dbm value (which is supposed to be negative)  is not correct anymore.
			rx_evmX /= 2;	//dbm

			EVM = EVMdbToPercentage(rx_evmX);

			//if(bPacketMatchBSSID)
			{
				if (i==0) // Fill value in RFD, Get the first spatial stream only
				{
					pfrinfo->sq = (u1Byte)(EVM & 0xff);
				}
				pfrinfo->rf_info.mimosq[i] = (u1Byte)(EVM & 0xff);
			}
		}
	}
}


int rtl80211_rxprocdesc(struct sk_buff *pskb, struct rx_frinfo *pfrinfo, struct rx_desc *pdesc, struct mac80211_shared_priv *priv)
{
	unsigned int cmd = get_desc(pdesc->Dword0); 

	pfrinfo->pktlen = (cmd & RX_LengthMask) - _CRCLNG_;
	pfrinfo->pktlen = (cmd & RX_LengthMask);
	pfrinfo->driver_info_size = ((cmd >> RX_DrvInfoSizeSHIFT) & RX_DrvInfoSizeMask) * 8;
	pfrinfo->rxbuf_shift = (cmd & (RX_ShiftMask << RX_ShiftSHIFT)) >> RX_ShiftSHIFT;
	pfrinfo->sw_dec = (cmd & RX_SWDec) >> 27;
	pfrinfo->pktlen -= pfrinfo->rxbuf_shift;
//			pskb->data += pfrinfo->rxbuf_shift;
	//92SE A-Cut bug or firmware bug? ... avoid abnormal packets
	//Reference to 92SE card driver
	if ((pfrinfo->pktlen > 0x2000) || (pfrinfo->pktlen < 16))
		return -EINPROGRESS;


	pfrinfo->driver_info = (struct RxFWInfo *)(get_pframe(pfrinfo));

	pfrinfo->physt	  = (cmd & RX_PHYStatus)? 1:0;

	if (!pfrinfo->physt)
		pfrinfo->rssi = 0;
	else
		translate_rssi_sq(priv, pfrinfo);

	
	pfrinfo->faggr	  = (get_desc(pdesc->Dword1) & RX_FAGGR)? 1:0;
	pfrinfo->paggr	  = (get_desc(pdesc->Dword1) & RX_PAGGR)? 1:0;
	pfrinfo->rx_bw	  = (get_desc(pdesc->Dword3) & RX_BandWidth)? 1:0;
	pfrinfo->rx_splcp = (get_desc(pdesc->Dword3) & RX_SPLCP)? 1:0;
	if ((get_desc(pdesc->Dword3)&RX_RxMCSMask) < 12)
		pfrinfo->rx_rate = dot11_rate_table[(get_desc(pdesc->Dword3)&RX_RxMCSMask)];
	else
		pfrinfo->rx_rate = 0x80|((get_desc(pdesc->Dword3)&RX_RxMCSMask)-12);
	

	return 0;
}

static long ieee80211_translate_todbm(u8 signal_strength_index	)// 0-100 index.
{
	long	signal_power; // in dBm.

	// Translate to dBm (x=0.5y-95).
	signal_power = (long)((signal_strength_index + 1) >> 1); 
	signal_power -= 95; 

	return signal_power;
}

int rtl80211_rx_status(struct rx_frinfo *pfrinfo, struct ieee80211_rx_status *rx_status, 
							struct rx_desc *pdesc, struct mac80211_shared_priv *priv)
{
	struct ieee80211_hw *hw = priv->hw;
	
	memset(rx_status, 0, sizeof(struct ieee80211_rx_status));
	rx_status->mactime = le64_to_cpu(get_desc(pdesc->Dword5));
	rx_status->band = hw->conf.channel->band; 
	rx_status->freq = hw->conf.channel->center_freq; 
	rx_status->signal = pfrinfo->rx_pwr_all;
	rx_status->noise = ieee80211_translate_todbm((u8)(100-pfrinfo->rssi)) -25;; 
	rx_status->qual = pfrinfo->rssi;
	//rx_status->antenna = 0; //not sure
	
	if (get_desc(pdesc->Dword3) & RX_RxHT)
		rx_status->flag |= RX_FLAG_HT;
	
	if ((get_desc(pdesc->Dword3)&RX_RxMCSMask) > 12)
		rx_status->rate_idx = (get_desc(pdesc->Dword3)&RX_RxMCSMask)-12;
	else
		rx_status->rate_idx = get_desc(pdesc->Dword3)&RX_RxMCSMask;
	
	//printk("rate = %d \n", get_desc(pdesc->Dword3)&RX_RxMCSMask);
	rx_status->rate_idx = 0;
	if (pfrinfo->rx_bw)
		rx_status->flag |= RX_FLAG_40MHZ;
	if (pfrinfo->rx_splcp)
		rx_status->flag |= RX_FLAG_SHORT_GI;
	if (rx_status->mactime)
		rx_status->flag |= RX_FLAG_TSFT;
	if ((get_desc(pdesc->Dword0) & (RX_SecurityMask << RX_SecuritySHIFT)) >> RX_SecuritySHIFT)
		rx_status->flag |= RX_FLAG_DECRYPTED;

	return 0;
}


#ifdef RTL8192SE
void rtl80211_rx_tasklet(unsigned long task_priv)
{
	struct mac80211_shared_priv *priv;
	static unsigned long x;
	struct ieee80211_rx_status rx_status;
	unsigned long ioaddr;

	struct rx_desc *pdesc=NULL, *prxdesc=NULL;
	struct rtl8190_hw *phw=NULL;
	struct sk_buff *pskb=NULL, *new_skb=NULL;
	struct rx_frinfo	*pfrinfo=NULL;
	unsigned int	tail=0;
	unsigned int	cmd=0;
	unsigned int	flush_range=0;
	int	retval;
	
	priv = (struct mac80211_shared_priv *) task_priv;
	ioaddr = priv->pshare_hw->ioaddr;
	phw = GET_HW(priv);
	
	SAVE_INT_AND_CLI(x);
	RTL_W32(IMR, RTL_R32(IMR) | (IMR_RXFOVW | IMR_RDU | IMR_ROK));

	if (!IS_DRV_OPEN(priv)){
		priv->pshare_hw->has_triggered_rx_tasklet = 0;
		RESTORE_INT(x);
		return;
	}
		
	tail = phw->cur_rx;

#ifdef __MIPSEB__
	prxdesc = (struct rx_desc *)((unsigned int)(phw->rx_descL) | 0x20000000);
#else
	prxdesc = (struct rx_desc *)(phw->rx_descL);
#endif
	
	pci_dma_sync_single_for_cpu(priv->pshare_hw->pdev, 
		(unsigned int)prxdesc, sizeof(struct rx_desc), PCI_DMA_FROMDEVICE);
	
	do {
		pdesc = prxdesc + tail;
		cmd = get_desc(pdesc->Dword0);
		if (cmd & RX_OWN)
			break;

		if ((cmd & RX_CRC32)
			|| ((cmd & (RX_FirstSeg | RX_LastSeg)) != (RX_FirstSeg | RX_LastSeg))
			|| (!IS_DRV_OPEN(priv)))
		{
			goto rx_reuse;
		}
		else
		{
			struct ieee80211_sta *sta = NULL;
			struct ieee80211_hdr *hdr;

			pskb = (struct sk_buff *)(phw->rx_infoL[tail].pbuf);
			//collect pfrinfo from rx_desc
			pfrinfo = get_pfrinfo(pskb);
			retval = rtl80211_rxprocdesc(pskb, pfrinfo, pdesc, priv);
			if (retval == -EINPROGRESS) {
				goto rx_reuse;
			}
			
			//fill rx_status for mac80211
			rtl80211_rx_status(pfrinfo, &rx_status, pdesc, priv);

			flush_range = pfrinfo->rxbuf_shift + pfrinfo->driver_info_size + pfrinfo->pktlen;
			//flush_range = MAX_RX_BUF_LEN;
			pci_dma_sync_single_for_cpu(priv->pshare_hw->pdev, 
				phw->rx_infoL[tail].paddr, flush_range, PCI_DMA_FROMDEVICE);
						
			new_skb = rtl_dev_alloc_skb(priv, MAX_RX_BUF_LEN, _SKB_RX_, 0);
			if (new_skb == (struct sk_buff *)NULL)
			{
				DEBUG_WARN("out of skb_buff\n");
				priv->ext_stats.reused_skb++;
				goto rx_reuse;
			}
			
			if (pfrinfo->pktlen < (MAX_RX_BUF_LEN-sizeof(struct rx_frinfo)-64)) {
				skb_reserve(pskb, (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size));
				if (cmd & RX_ICVError) {
					dev_kfree_skb_any(new_skb);
					pskb->data -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
					pskb->tail -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
					goto rx_reuse;
				}
			}
	
			pci_unmap_single(priv->pshare_hw->pdev, phw->rx_infoL[tail].paddr, (MAX_RX_BUF_LEN - sizeof(struct rx_frinfo)), PCI_DMA_FROMDEVICE);

			pskb->len = pfrinfo->pktlen;
		
			/* kernel 2.6.31
			memcpy(IEEE80211_SKB_RXCB(pskb), &rx_status, sizeof(struct ieee80211_rx_status));
			ieee80211_rx(priv->hw, pskb);
			*/
			
			hdr = (struct ieee80211_hdr *)pskb->data;
			
			sta = ieee80211_find_sta(priv->hw, hdr->addr2);
			if (sta){
				struct priv_stainfo *stainfo = (struct priv_stainfo *)sta->drv_priv;
				stainfo->rssi = pfrinfo->rssi;
			}

			__ieee80211_rx(priv->hw, pskb, &rx_status);
			
			pskb = new_skb;
			init_rxdesc(pskb, tail, priv);	
			goto rx_done;
		}
		
rx_reuse:

		pdesc->Dword6 = set_desc(phw->rx_infoL[tail].paddr);
		pdesc->Dword0 = set_desc((tail == (NUM_RX_DESC - 1)? RX_EOR : 0) | RX_OWN | (MAX_RX_BUF_LEN - sizeof(struct rx_frinfo)));
		
rx_done:
		
		tail = (tail + 1) % NUM_RX_DESC;

		
	}while(1);
//rx_exit:
	phw->cur_rx = tail;

	priv->pshare_hw->has_triggered_rx_tasklet = 0;
	RESTORE_INT(x);
	
}
#endif
 #ifdef RTL8192SU
 void rtl80211_rx_tasklet(unsigned long task_priv)
 {
 	unsigned int isevt, *pbuf; 	
	struct sk_buff *skb=NULL;
 	struct mac80211_shared_priv *priv = (struct mac80211_shared_priv *)task_priv;	
	struct rtl8190_hw *phw = GET_HW(priv);
	
	//printk("%s\n", __FUNCTION__);	
	
	while (NULL != (skb = skb_dequeue(&phw->rx_skb_queue)))
	{
		pbuf = (uint*)skb->data;
		if((isevt = le32_to_cpu(*(pbuf+1))&0x1ff) == 0x1ff)
		{			
			printk("usb_read_port_complete():rxcmd_event_hdl\n");

			//rxcmd_event_hdl(pintfpriv, pbuf);//rx c2h events

			dev_kfree_skb_any(skb);
		
		}
		else
		{		
			rtl8192s_usb_recv_entry(priv, skb);
		}	

	}

 	
 }	
 #endif
 

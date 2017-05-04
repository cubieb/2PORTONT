/*
 *      RX handle routines
 *
 *      $Id: 8190n_rx.c,v 1.1.1.1 2010/05/05 09:00:39 jiunming Exp $
 */

#define _8190N_RX_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#ifdef	CONFIG_X86
#include </root/linux-2.6.18.1/net/bridge/br_private.h>
#else
#include <../net/bridge/br_private.h> // modified for porting ... joshua
#endif

#endif

#include "./8190n_cfg.h"
#include "./8190n.h"
#include "./8190n_hw.h"
#include "./8190n_headers.h"
#include "./8190n_rx.h"
#include "./8190n_debug.h"

#if	defined(CONFIG_RTL865X_EXTPORT)
#include <common/rtl_types.h>
#include <common/mbuf.h>
#include <rtl865xc_swNic.h>
#endif

#ifdef CONFIG_RTK_MESH
#include "./mesh_ext/mesh_route.h"
#endif

#ifdef BR_SHORTCUT
#ifdef WDS
__DRAM_IN_865X unsigned char cached_wds_mac[MACADDRLEN];
__DRAM_IN_865X struct net_device *cached_wds_dev = NULL;
#endif
#ifdef CONFIG_RTK_MESH
__DRAM_IN_865X unsigned char cached_mesh_mac[MACADDRLEN];
__DRAM_IN_865X struct net_device *cached_mesh_dev = NULL;
#endif
#ifdef CLIENT_MODE
__DRAM_IN_865X unsigned char cached_sta_mac[MACADDRLEN];
__DRAM_IN_865X struct net_device *cached_sta_dev = NULL;
#endif

#if	defined(RTL_CACHED_BR_STA)
__DRAM_IN_865X unsigned char cached_br_sta_mac[MACADDRLEN];
__DRAM_IN_865X struct net_device *cached_br_sta_dev = NULL;
#endif

#endif

//for 8671 IGMP snooping
#ifdef CONFIG_RTL8671
#define wlan_igmp_tag 5
extern int enable_IGMP_SNP;
extern void check_IGMP_snoop_rx(struct sk_buff *skb, int tag);
// MBSSID Port Mapping
extern struct port_map wlanDev[5];
extern int g_port_mapping;
#endif

#ifdef CONFIG_RTL_STP
//#include "../../driver/net/re865x/AsicDriver/rtl865xC_tblAsicDrv.h"
static unsigned char STPmac[6] = { 1, 0x80, 0xc2, 0,0,0};
static struct net_device* wlan_pseudo_dev;
#define WLAN_INTERFACE_NAME			"wlan0"

#if defined(RTL_PATCHED_26) && defined(CONFIG_RTK_MESH)
static struct net_device* wlan_mesh_pseudo_dev;
#define WLAN_MESH_INTERFACE_NAME			"wlan0-msh0"
#endif

#endif

/* ======================== RX procedure declarations ======================== */
#ifndef CONFIG_RTK_MESH
static
#endif
 void rtl8190_rx_mgntframe(struct rtl8190_priv *priv,
				struct list_head *list, struct rx_frinfo *inputPfrinfo);
static void rtl8190_rx_ctrlframe(struct rtl8190_priv *priv,
				struct list_head *list, struct rx_frinfo *inputPfrinfo);
#ifndef CONFIG_RTK_MESH
static
#endif
 void rtl8190_rx_dataframe(struct rtl8190_priv *priv,
				struct list_head *list, struct rx_frinfo *inputPfrinfo);


static int auth_filter(struct rtl8190_priv *priv, struct stat_info *pstat,
				struct rx_frinfo *pfrinfo);
static void ctrl_handler(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo);

static void process_amsdu(struct rtl8190_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo);

#ifdef RTL8190_FASTEXTDEV_FUNCALL
void rtl865x_fastExtDev_init_rxdesc(struct sk_buff *pskb, int i, struct rtl8190_priv *priv)
{
	return init_rxdesc(pskb, i, priv);
}
#endif


static unsigned char QueryRxPwrPercentage(signed char AntPower)
{
	if ((AntPower <= -100) || (AntPower >= 20))
		return	0;
	else if (AntPower >= 0)
		return	100;
	else
		return	(100+AntPower);

}


int SignalScaleMapping(int CurrSig)
{
	int RetSig;

	// Step 1. Scale mapping.
	if(CurrSig >= 61 && CurrSig <= 100)
	{
		RetSig = 90 + ((CurrSig - 60) / 4);
	}
	else if(CurrSig >= 41 && CurrSig <= 60)
	{
		RetSig = 78 + ((CurrSig - 40) / 2);
	}
	else if(CurrSig >= 31 && CurrSig <= 40)
	{
		RetSig = 66 + (CurrSig - 30);
	}
	else if(CurrSig >= 21 && CurrSig <= 30)
	{
		RetSig = 54 + (CurrSig - 20);
	}
	else if(CurrSig >= 5 && CurrSig <= 20)
	{
		RetSig = 42 + (((CurrSig - 5) * 2) / 3);
	}
	else if(CurrSig == 4)
	{
		RetSig = 36;
	}
	else if(CurrSig == 3)
	{
		RetSig = 27;
	}
	else if(CurrSig == 2)
	{
		RetSig = 18;
	}
	else if(CurrSig == 1)
	{
		RetSig = 9;
	}
	else
	{
		RetSig = CurrSig;
	}

	return RetSig;
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


static __inline__ void translate_rssi_sq(struct RTL_PRIV *priv, struct rx_frinfo *pfrinfo)
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
#ifdef NEW_MAC80211_DRV
	struct priv_shared_hw *pshare = priv->pshare_hw;
#else
	struct priv_shared_info *pshare = priv->pshare;
#endif

	isCCKrate = is_CCK_rate(pfrinfo->rx_rate);

	/*2007.08.30 requested by SD3 Jerry */
	if (check_reg824 == 0) {
		reg824_bit9 = PHY_QueryBBReg(priv, rFPGA0_XA_HSSIParameter2, 0x200);
		check_reg824 = 1;
	}

	prxpkt = (u1Byte *)pfrinfo->driver_info;
#if defined(RTL8190) || defined(RTL8192E)
	/* Move pointer to the 16th bytes. Phy status start address. */
	prxpkt += sizeof(struct RxFWInfo);
#endif

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
		if ((get_rf_mimo_mode(priv) == MIMO_2T4R) && (pshare->rf_ft_var.cck_sel_ver == 2)) {
			for (i=RF90_PATH_A; i<RF90_PATH_MAX; i++) {
				tmp_rxrssi = pCck_buf->adc_pwdb_X[i];
				rx_rssiX = (s1Byte)(tmp_rxrssi);
				rx_rssiX /= 2;
				pfrinfo->cck_mimorssi[i] = rx_rssiX;
			}
		}

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
#ifdef RTL8192SE
		pfrinfo->rssi+=3;
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
#ifdef RTL8190
		for (i=RF90_PATH_A; i<RF90_PATH_MAX; i++)
#else	// RTL8192E, RTL8192SE
		for (i=RF90_PATH_A; i<RF90_PATH_C; i++)
#endif
		{
			rx_pwr[i] = ((pOfdm_buf->trsw_gain_X[i]&0x3F)*2) - 106;

			//Get Rx snr value in DB
			if (pshare->rf_ft_var.rssi_dump) {
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


#ifdef CONFIG_RTL_STP
int rtl865x_wlanIF_Init(struct net_device *dev)
{
	if (dev == NULL)
		return FALSE;
	else
	{
		wlan_pseudo_dev = dev;
		printk("init wlan pseudo dev =====> %s\n", wlan_pseudo_dev->name);
	}
	return TRUE;
}

#if defined(RTL_PATCHED_26) && defined(CONFIG_RTK_MESH)
int rtl865x_wlanMeshIF_Init(struct net_device *dev)
{
	if (dev == NULL)
		return FALSE;
	else
	{
		wlan_mesh_pseudo_dev = dev;
		printk("init wlan mesh pseudo dev =====> %s\n", wlan_mesh_pseudo_dev->name);
	}
	return TRUE;
}
#endif

#endif

#if defined(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI)

extern void (*__p_rtl_netif_rx)(struct rtl8190_priv *priv, struct sk_buff *pskb, struct stat_info *pstat);

__IRAM_IN_865X_WAPI
void __rtl_netif_rx_wapi(struct rtl8190_priv *priv, struct sk_buff *pskb, struct stat_info *pstat)
{
#ifdef CONFIG_RTL8671
	int k;
#endif

#ifdef GBWC
	if (priv->pmib->gbwcEntry.GBWCMode && pstat) {
		if (((priv->pmib->gbwcEntry.GBWCMode == 1) && (pstat->GBWC_in_group)) ||
			((priv->pmib->gbwcEntry.GBWCMode == 2) && !(pstat->GBWC_in_group))) {
			if ((priv->GBWC_rx_count + pskb->len) > ((priv->pmib->gbwcEntry.GBWCThrd * 1024 / 8) / (100 / GBWC_TO))) {
				// over the bandwidth
				if (priv->GBWC_consuming_Q) {
					// in rtl8190_GBWC_timer context
					priv->ext_stats.rx_data_drops++;
					DEBUG_ERR("RX DROP: BWC bandwidth over!\n");
					rtl_kfree_skb(priv, pskb, _SKB_RX_);
				}
				else {
					// normal Rx path
					int ret = enque(priv, &(priv->GBWC_rx_queue.head), &(priv->GBWC_rx_queue.tail),
							(unsigned int)(priv->GBWC_rx_queue.pSkb), NUM_TXPKT_QUEUE, (void *)pskb);
					if (ret == FALSE) {
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: BWC rx queue full!\n");
						rtl_kfree_skb(priv, pskb, _SKB_RX_);
					}
					else
						*(unsigned int *)&(pskb->cb[4]) = (unsigned int)pstat;	// backup pstat pointer
				}
				return;
			}
			else {
				// not over the bandwidth
				if (CIRC_CNT(priv->GBWC_rx_queue.head, priv->GBWC_rx_queue.tail, NUM_TXPKT_QUEUE) &&
						!priv->GBWC_consuming_Q) {
					// there are already packets in queue, put in queue too for order
					int ret = enque(priv, &(priv->GBWC_rx_queue.head), &(priv->GBWC_rx_queue.tail),
							(unsigned int)(priv->GBWC_rx_queue.pSkb), NUM_TXPKT_QUEUE, (void *)pskb);
					if (ret == FALSE) {
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: BWC rx queue full!\n");
						rtl_kfree_skb(priv, pskb, _SKB_RX_);
					}
					else
						*(unsigned int *)&(pskb->cb[4]) = (unsigned int)pstat;	// backup pstat pointer
					return;
				}
				else {
					// can pass up directly
					priv->GBWC_rx_count += pskb->len;
				}
			}
		}
	}
#endif

#ifdef ENABLE_RTL_SKB_STATS
	rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif

#ifdef CONFIG_RTL865X
	rtl865x_wlanAccelerate(priv, pskb);
#else

#ifdef	_11s_TEST_MODE_
	mesh_debug_rx1(priv, pskb);
#endif

#ifdef CONFIG_RTL_STP
	if (((unsigned char)pskb->data[12]) < 0x06)
	{
		if (!memcmp(pskb->data, STPmac, 5) && !(((unsigned char )pskb->data[5])& 0xF0))
		{
			/*printk("received bpdu packet from %s\n", pskb->dev->name);*/
			if (memcmp(pskb->dev->name, WLAN_INTERFACE_NAME, sizeof(WLAN_INTERFACE_NAME)) == 0)
			{
				if (wlan_pseudo_dev != NULL)
					pskb->dev = wlan_pseudo_dev;
				/*printk("change dev to %s\n", wlan_pseudo_dev->name);*/
				pskb->protocol = eth_type_trans(pskb, priv->dev);
				netif_rx(pskb);
			}
			#if defined(RTL_PATCHED_26) && defined(CONFIG_RTK_MESH)
			else if (memcmp(pskb->dev->name, WLAN_MESH_INTERFACE_NAME, sizeof(WLAN_MESH_INTERFACE_NAME)) == 0)
			{
				if (wlan_mesh_pseudo_dev != NULL)
					pskb->dev = wlan_mesh_pseudo_dev;
				/*printk("change dev to %s\n", wlan_mesh_pseudo_dev->name);*/
				pskb->protocol = eth_type_trans(pskb, priv->dev);
				netif_rx(pskb);
			}
			#endif
		}
	}
	else
#endif
	{
#if	defined(CONFIG_RTL865X_EXTPORT)
		#define GATEWAY_MODE				0
		#define BRIDGE_MODE				1
		#define WISP_MODE					2
		extern int rtl865x_curOpMode;
		if ((rtl865x_curOpMode!=WISP_MODE) && ((pskb->data[0]&0x1) == 0) && (pskb->dev==cachedWlanDev))
		{
			/* unicast pkt & rx from cached WlanDevice	*/
			rtl_nicTx_info        nicTx;
			/* the pkt must be tx to lan vlan */
			nicTx.vid = CONFIG_RTL865X_EXTPORT_VLANID;
			nicTx.portlist = RTL8651_CPU_PORT;              /* must be set 0x7 */
			nicTx.srcExtPort = PKTHDR_EXTPORT_LIST_P1;
			if( memcmp(pskb->data, cachedWlanDev->dev_addr, MACADDRLEN))
				nicTx.flags = (PKTHDR_USED|PKTHDR_HWLOOKUP|PKTHDR_BRIDGING|PKT_OUTGOING);
			else
				nicTx.flags = (PKTHDR_USED|PKTHDR_HWLOOKUP|PKT_OUTGOING);
			swNic_send((void *)pskb, pskb->data, pskb->len, &nicTx);
			return;
		}
#elif defined(BR_SHORTCUT)
	{
		extern 	 unsigned char cached_eth_addr[MACADDRLEN];
		extern struct net_device *cached_dev;

#ifdef CONFIG_RTK_MESH

        if (pskb->dev && (pskb->dev == priv->mesh_dev))
        {
                memcpy(cached_mesh_mac, &pskb->data[6], 6);
                cached_mesh_dev = pskb->dev;
        }
#endif

#ifdef WDS
		if (pskb->dev && pskb->dev->base_addr==0) {
			memcpy(cached_wds_mac, &pskb->data[6], 6);
			cached_wds_dev = pskb->dev;
		}
#endif

#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE) {
			memcpy(cached_sta_mac, &pskb->data[6], 6);
			cached_sta_dev = pskb->dev;
		}
#endif

		if (!(pskb->data[0] & 0x01) &&
				!priv->pmib->dot11OperationEntry.disable_brsc &&
				(priv->dev->br_port) &&
				cached_dev &&
				!memcmp(pskb->data, cached_eth_addr, MACADDRLEN) &&
				netif_running(cached_dev)) {
			pskb->dev = cached_dev;
		#if defined(CONFIG_COMPAT_NET_DEV_OPS)
			cached_dev->hard_start_xmit(pskb, cached_dev);
		#else
			cached_dev->netdev_ops->ndo_start_xmit(pskb, cached_dev);
		#endif
			return;
		}
	}
#endif // BR_SHORTCUT

#ifdef MBSSID
	if ((OPMODE & WIFI_AP_STATE) && (pskb->dev->base_addr != 0)) {
		*(unsigned int *)&(pskb->cb[8]) = 0x86518190;						// means from wlan interface
		*(unsigned int *)&(pskb->cb[12]) = priv->pmib->miscEntry.groupID;	// remember group ID
	}
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
	if (priv->pmib->vlan.global_vlan) {
		if (rx_vlan_process(priv->dev, &priv->pmib->vlan, pskb)){
			priv->ext_stats.rx_data_drops++;
			DEBUG_ERR("RX DROP: by vlan!\n");
			rtl_kfree_skb(priv, pskb, _SKB_RX_);
			return;
		}
	}
#endif

	pskb->protocol = eth_type_trans(pskb, priv->dev);

#ifdef CONFIG_RTL8671
	if(enable_IGMP_SNP)
		check_IGMP_snoop_rx(pskb, wlan_igmp_tag);
	// Modofied by Mason Yu
	// MBSSID Port Mapping
#if 0
	pskb->vlan_member = ifgrp_member;
	pskb->switch_port = priv->dev->name;
#endif
	if ( g_port_mapping == TRUE ) {
		for (k=0; k<5; k++) {
			if ( priv->dev == wlanDev[k].dev_pointer ) {
				pskb->vlan_member = wlanDev[k].dev_ifgrp_member;
				break;
			}
		}
	}
	pskb->switch_port = priv->dev->name;
#endif

	netif_rx(pskb);
	}
#endif // CONFIG_RTL865X
}


__MIPS16
__IRAM_FASTEXTDEV
void __rtl_netif_rx(struct rtl8190_priv *priv, struct sk_buff *pskb, struct stat_info *pstat)
{
#ifdef CONFIG_RTL8671
	int k;
#endif

#ifdef GBWC
	if (priv->pmib->gbwcEntry.GBWCMode && pstat) {
		if (((priv->pmib->gbwcEntry.GBWCMode == 1) && (pstat->GBWC_in_group)) ||
			((priv->pmib->gbwcEntry.GBWCMode == 2) && !(pstat->GBWC_in_group))) {
			if ((priv->GBWC_rx_count + pskb->len) > ((priv->pmib->gbwcEntry.GBWCThrd * 1024 / 8) / (100 / GBWC_TO))) {
				// over the bandwidth
				if (priv->GBWC_consuming_Q) {
					// in rtl8190_GBWC_timer context
					priv->ext_stats.rx_data_drops++;
					DEBUG_ERR("RX DROP: BWC bandwidth over!\n");
					rtl_kfree_skb(priv, pskb, _SKB_RX_);
				}
				else {
					// normal Rx path
					int ret = enque(priv, &(priv->GBWC_rx_queue.head), &(priv->GBWC_rx_queue.tail),
							(unsigned int)(priv->GBWC_rx_queue.pSkb), NUM_TXPKT_QUEUE, (void *)pskb);
					if (ret == FALSE) {
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: BWC rx queue full!\n");
						rtl_kfree_skb(priv, pskb, _SKB_RX_);
					}
					else
						*(unsigned int *)&(pskb->cb[4]) = (unsigned int)pstat;	// backup pstat pointer
				}
				return;
			}
			else {
				// not over the bandwidth
				if (CIRC_CNT(priv->GBWC_rx_queue.head, priv->GBWC_rx_queue.tail, NUM_TXPKT_QUEUE) &&
						!priv->GBWC_consuming_Q) {
					// there are already packets in queue, put in queue too for order
					int ret = enque(priv, &(priv->GBWC_rx_queue.head), &(priv->GBWC_rx_queue.tail),
							(unsigned int)(priv->GBWC_rx_queue.pSkb), NUM_TXPKT_QUEUE, (void *)pskb);
					if (ret == FALSE) {
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: BWC rx queue full!\n");
						rtl_kfree_skb(priv, pskb, _SKB_RX_);
					}
					else
						*(unsigned int *)&(pskb->cb[4]) = (unsigned int)pstat;	// backup pstat pointer
					return;
				}
				else {
					// can pass up directly
					priv->GBWC_rx_count += pskb->len;
				}
			}
		}
	}
#endif

#ifdef ENABLE_RTL_SKB_STATS
	rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif

#ifdef CONFIG_RTL865X
	rtl865x_wlanAccelerate(priv, pskb);
#else

#ifdef	_11s_TEST_MODE_
	mesh_debug_rx1(priv, pskb);
#endif

#ifdef CONFIG_RTL_STP
	if (((unsigned char)pskb->data[12]) < 0x06)
	{
		if (!memcmp(pskb->data, STPmac, 5) && !(((unsigned char )pskb->data[5])& 0xF0))
		{
			/*printk("received bpdu packet from %s\n", pskb->dev->name);*/
			if (memcmp(pskb->dev->name, WLAN_INTERFACE_NAME, sizeof(WLAN_INTERFACE_NAME)) == 0)
			{
				if (wlan_pseudo_dev != NULL)
					pskb->dev = wlan_pseudo_dev;
				/*printk("change dev to %s\n", wlan_pseudo_dev->name);*/
				pskb->protocol = eth_type_trans(pskb, priv->dev);
				netif_rx(pskb);
			}
			#if defined(RTL_PATCHED_26) && defined(CONFIG_RTK_MESH)
			else if (memcmp(pskb->dev->name, WLAN_MESH_INTERFACE_NAME, sizeof(WLAN_MESH_INTERFACE_NAME)) == 0)
			{
				if (wlan_mesh_pseudo_dev != NULL)
					pskb->dev = wlan_mesh_pseudo_dev;
				/*printk("change dev to %s\n", wlan_mesh_pseudo_dev->name);*/
				pskb->protocol = eth_type_trans(pskb, priv->dev);
				netif_rx(pskb);
			}
			#endif
			
			return;		
		
		}
	}
/*	else*/
#endif
	{
#if	defined(CONFIG_RTL865X_EXTPORT)
		#define GATEWAY_MODE				0
		#define BRIDGE_MODE				1
		#define WISP_MODE					2
		extern int rtl865x_curOpMode;
		if ((rtl865x_curOpMode!=WISP_MODE) && ((pskb->data[0]&0x1) == 0) && (pskb->dev==cachedWlanDev))
		{
			/* unicast pkt & rx from cached WlanDevice	*/
			rtl_nicTx_info        nicTx;
			/* the pkt must be tx to lan vlan */
			nicTx.vid = CONFIG_RTL865X_EXTPORT_VLANID;
			nicTx.portlist = RTL8651_CPU_PORT;              /* must be set 0x7 */
			nicTx.srcExtPort = PKTHDR_EXTPORT_LIST_P1;
			if( memcmp(pskb->data, cachedWlanDev->dev_addr, MACADDRLEN))
				nicTx.flags = (PKTHDR_USED|PKTHDR_HWLOOKUP|PKTHDR_BRIDGING|PKT_OUTGOING);
			else
				nicTx.flags = (PKTHDR_USED|PKTHDR_HWLOOKUP|PKT_OUTGOING);
			swNic_send((void *)pskb, pskb->data, pskb->len, &nicTx);
			return;
		}
#elif defined(BR_SHORTCUT)
	{
		extern 	 unsigned char cached_eth_addr[MACADDRLEN];
		extern struct net_device *cached_dev;

#ifdef CONFIG_RTK_MESH

        if (pskb->dev && (pskb->dev == priv->mesh_dev))
        {
                memcpy(cached_mesh_mac, &pskb->data[6], 6);
                cached_mesh_dev = pskb->dev;
        }
#endif

#ifdef WDS
		if (pskb->dev && pskb->dev->base_addr==0) {
			memcpy(cached_wds_mac, &pskb->data[6], 6);
			cached_wds_dev = pskb->dev;
		}
#endif

#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE) {
			memcpy(cached_sta_mac, &pskb->data[6], 6);
			cached_sta_dev = pskb->dev;
		}
#endif

		if (!(pskb->data[0] & 0x01) &&
				!priv->pmib->dot11OperationEntry.disable_brsc &&
				(priv->dev->br_port) &&
				cached_dev &&
				!memcmp(pskb->data, cached_eth_addr, MACADDRLEN) &&
				netif_running(cached_dev)) {
			pskb->dev = cached_dev;
		#if defined(CONFIG_COMPAT_NET_DEV_OPS)
			cached_dev->hard_start_xmit(pskb, cached_dev);
		#else
			cached_dev->netdev_ops->ndo_start_xmit(pskb,cached_dev);
		#endif
			return;
		}
	}
#endif // BR_SHORTCUT

#ifdef MBSSID
	if ((OPMODE & WIFI_AP_STATE) && (pskb->dev->base_addr != 0)) {
		*(unsigned int *)&(pskb->cb[8]) = 0x86518190;						// means from wlan interface
		*(unsigned int *)&(pskb->cb[12]) = priv->pmib->miscEntry.groupID;	// remember group ID
	}
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
	if (priv->pmib->vlan.global_vlan) {
		if (rx_vlan_process(priv->dev, &priv->pmib->vlan, pskb)){
			priv->ext_stats.rx_data_drops++;
			DEBUG_ERR("RX DROP: by vlan!\n");
			rtl_kfree_skb(priv, pskb, _SKB_RX_);
			return;
		}
	}
#endif

	pskb->protocol = eth_type_trans(pskb, priv->dev);

#ifdef CONFIG_RTL8671
	if(enable_IGMP_SNP)
		check_IGMP_snoop_rx(pskb, wlan_igmp_tag);
	// Modofied by Mason Yu
	// MBSSID Port Mapping
#if 0
	pskb->vlan_member = ifgrp_member;
	pskb->switch_port = priv->dev->name;
#endif
	if ( g_port_mapping == TRUE ) {
		for (k=0; k<5; k++) {
			if ( priv->dev == wlanDev[k].dev_pointer ) {
				pskb->vlan_member = wlanDev[k].dev_ifgrp_member;
				break;
			}
		}
	}
	pskb->switch_port = priv->dev->name;
#endif

	netif_rx(pskb);
	}
#endif // CONFIG_RTL865X
}

void rtl_netif_rx(struct rtl8190_priv *priv, struct sk_buff *pskb, struct stat_info *pstat)
{
	#if defined(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI_DEBUG)
	FUNCTION_CHECK(__p_rtl_netif_rx);
	#endif

	__p_rtl_netif_rx(priv,pskb,pstat);
}

#else

//__MIPS16
__IRAM_FASTEXTDEV
void rtl_netif_rx(struct rtl8190_priv *priv, struct sk_buff *pskb, struct stat_info *pstat)
{
#ifdef CONFIG_RTL8671
	int k;
#endif

#ifdef GBWC
	if (priv->pmib->gbwcEntry.GBWCMode && pstat) {
		if (((priv->pmib->gbwcEntry.GBWCMode == 1) && (pstat->GBWC_in_group)) ||
			((priv->pmib->gbwcEntry.GBWCMode == 2) && !(pstat->GBWC_in_group))) {
			if ((priv->GBWC_rx_count + pskb->len) > ((priv->pmib->gbwcEntry.GBWCThrd * 1024 / 8) / (100 / GBWC_TO))) {
				// over the bandwidth
				if (priv->GBWC_consuming_Q) {
					// in rtl8190_GBWC_timer context
					priv->ext_stats.rx_data_drops++;
					DEBUG_ERR("RX DROP: BWC bandwidth over!\n");
					rtl_kfree_skb(priv, pskb, _SKB_RX_);
				}
				else {
					// normal Rx path
					int ret = enque(priv, &(priv->GBWC_rx_queue.head), &(priv->GBWC_rx_queue.tail),
							(unsigned int)(priv->GBWC_rx_queue.pSkb), NUM_TXPKT_QUEUE, (void *)pskb);
					if (ret == FALSE) {
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: BWC rx queue full!\n");
						rtl_kfree_skb(priv, pskb, _SKB_RX_);
					}
					else
						*(unsigned int *)&(pskb->cb[4]) = (unsigned int)pstat;	// backup pstat pointer
				}
				return;
			}
			else {
				// not over the bandwidth
				if (CIRC_CNT(priv->GBWC_rx_queue.head, priv->GBWC_rx_queue.tail, NUM_TXPKT_QUEUE) &&
						!priv->GBWC_consuming_Q) {
					// there are already packets in queue, put in queue too for order
					int ret = enque(priv, &(priv->GBWC_rx_queue.head), &(priv->GBWC_rx_queue.tail),
							(unsigned int)(priv->GBWC_rx_queue.pSkb), NUM_TXPKT_QUEUE, (void *)pskb);
					if (ret == FALSE) {
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: BWC rx queue full!\n");
						rtl_kfree_skb(priv, pskb, _SKB_RX_);
					}
					else
						*(unsigned int *)&(pskb->cb[4]) = (unsigned int)pstat;	// backup pstat pointer
					return;
				}
				else {
					// can pass up directly
					priv->GBWC_rx_count += pskb->len;
				}
			}
		}
	}
#endif

#ifdef ENABLE_RTL_SKB_STATS
	rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif

#ifdef CONFIG_RTL865X
	rtl865x_wlanAccelerate(priv, pskb);
#else

#ifdef	_11s_TEST_MODE_
	mesh_debug_rx1(priv, pskb);
#endif

#ifdef CONFIG_RTL_STP
	if (((unsigned char)pskb->data[12]) < 0x06)
	{
		if (!memcmp(pskb->data, STPmac, 5) && !(((unsigned char )pskb->data[5])& 0xF0))
		{
			/*printk("received bpdu packet from %s\n", pskb->dev->name);*/
			if (memcmp(pskb->dev->name, WLAN_INTERFACE_NAME, sizeof(WLAN_INTERFACE_NAME)) == 0)
			{
				if (wlan_pseudo_dev != NULL)
					pskb->dev = wlan_pseudo_dev;
				/*printk("change dev to %s\n", wlan_pseudo_dev->name);*/
				pskb->protocol = eth_type_trans(pskb, priv->dev);
				netif_rx(pskb);
			}
			#if defined(RTL_PATCHED_26) && defined(CONFIG_RTK_MESH)
			else if (memcmp(pskb->dev->name, WLAN_MESH_INTERFACE_NAME, sizeof(WLAN_MESH_INTERFACE_NAME)) == 0)
			{
				if (wlan_mesh_pseudo_dev != NULL)
					pskb->dev = wlan_mesh_pseudo_dev;
				/*printk("change dev to %s\n", wlan_mesh_pseudo_dev->name);*/
				pskb->protocol = eth_type_trans(pskb, priv->dev);
				netif_rx(pskb);
			}
			#endif
		}
	}
	else
#endif
	{
#if	defined(CONFIG_RTL865X_EXTPORT)
		#define GATEWAY_MODE				0
		#define BRIDGE_MODE				1
		#define WISP_MODE					2
		extern int rtl865x_curOpMode;
		if ((rtl865x_curOpMode!=WISP_MODE) && ((pskb->data[0]&0x1) == 0) && (pskb->dev==cachedWlanDev))
		{
			/* unicast pkt & rx from cached WlanDevice	*/
			rtl_nicTx_info        nicTx;
			/* the pkt must be tx to lan vlan */
			nicTx.vid = CONFIG_RTL865X_EXTPORT_VLANID;
			nicTx.portlist = RTL8651_CPU_PORT;              /* must be set 0x7 */
			nicTx.srcExtPort = PKTHDR_EXTPORT_LIST_P1;
			if( memcmp(pskb->data, cachedWlanDev->dev_addr, MACADDRLEN))
				nicTx.flags = (PKTHDR_USED|PKTHDR_HWLOOKUP|PKTHDR_BRIDGING|PKT_OUTGOING);
			else
				nicTx.flags = (PKTHDR_USED|PKTHDR_HWLOOKUP|PKT_OUTGOING);
			swNic_send((void *)pskb, pskb->data, pskb->len, &nicTx);
			return;
		}
#elif defined(BR_SHORTCUT)
	{
		extern 	 unsigned char cached_eth_addr[MACADDRLEN];
		extern struct net_device *cached_dev;

#ifdef CONFIG_RTK_MESH

        if (pskb->dev && (pskb->dev == priv->mesh_dev))
        {
        	//chris: 
        	proxy_table_chkcln(priv, pskb);
                memcpy(cached_mesh_mac, &pskb->data[6], 6);
                cached_mesh_dev = pskb->dev;
        }
#endif

#ifdef WDS
		if (pskb->dev && pskb->dev->base_addr==0) {
			memcpy(cached_wds_mac, &pskb->data[6], 6);
			cached_wds_dev = pskb->dev;
		}
#endif

#ifdef CLIENT_MODE
		if (OPMODE & WIFI_STATION_STATE) {
			memcpy(cached_sta_mac, &pskb->data[6], 6);
			cached_sta_dev = pskb->dev;
		}
#endif

		if (!(pskb->data[0] & 0x01) &&
				!priv->pmib->dot11OperationEntry.disable_brsc &&
				(priv->dev->br_port) &&
				cached_dev &&
				!memcmp(pskb->data, cached_eth_addr, MACADDRLEN) &&
				netif_running(cached_dev)) {
			pskb->dev = cached_dev;
			
		#if defined(CONFIG_COMPAT_NET_DEV_OPS)
			cached_dev->hard_start_xmit(pskb, cached_dev);
		#else
			cached_dev->netdev_ops->ndo_start_xmit(pskb,cached_dev);
		#endif

			return;
		}
	}
#endif // BR_SHORTCUT

#ifdef MBSSID
	if ((OPMODE & WIFI_AP_STATE) && (pskb->dev->base_addr != 0)) {
		*(unsigned int *)&(pskb->cb[8]) = 0x86518190;						// means from wlan interface
		*(unsigned int *)&(pskb->cb[12]) = priv->pmib->miscEntry.groupID;	// remember group ID
	}
#endif

#ifdef CONFIG_RTK_VLAN_SUPPORT
	if (priv->pmib->vlan.global_vlan) {
		if (rx_vlan_process(priv->dev, &priv->pmib->vlan, pskb)){
			priv->ext_stats.rx_data_drops++;
			DEBUG_ERR("RX DROP: by vlan!\n");
			rtl_kfree_skb(priv, pskb, _SKB_RX_);
			return;
		}
	}
#endif

	pskb->protocol = eth_type_trans(pskb, priv->dev);

#ifdef CONFIG_RTL8671
	if(enable_IGMP_SNP)
		check_IGMP_snoop_rx(pskb, wlan_igmp_tag);
	// Modofied by Mason Yu
	// MBSSID Port Mapping
#if 0
	pskb->vlan_member = ifgrp_member;
	pskb->switch_port = priv->dev->name;
#endif
	if ( g_port_mapping == TRUE ) {
		for (k=0; k<5; k++) {
			if ( priv->dev == wlanDev[k].dev_pointer ) {
				pskb->vlan_member = wlanDev[k].dev_ifgrp_member;
				break;
			}
		}
	}
	pskb->switch_port = priv->dev->name;
#endif

	netif_rx(pskb);
	}
#endif // CONFIG_RTL865X
}

#endif //CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI


#ifdef GBWC
static int GBWC_forward_check(struct rtl8190_priv *priv, struct sk_buff *pskb, struct stat_info *pstat)
{
	if (priv->pmib->gbwcEntry.GBWCMode && pstat) {
		if (((priv->pmib->gbwcEntry.GBWCMode == 1) && (pstat->GBWC_in_group)) ||
			((priv->pmib->gbwcEntry.GBWCMode == 2) && !(pstat->GBWC_in_group))) {
			if ((priv->GBWC_rx_count + pskb->len) > ((priv->pmib->gbwcEntry.GBWCThrd * 1024 / 8) / (100 / GBWC_TO))) {
				// over the bandwidth
				int ret = enque(priv, &(priv->GBWC_tx_queue.head), &(priv->GBWC_tx_queue.tail),
						(unsigned int)(priv->GBWC_tx_queue.pSkb), NUM_TXPKT_QUEUE, (void *)pskb);
				if (ret == FALSE) {
					priv->ext_stats.rx_data_drops++;
					DEBUG_ERR("RX DROP: BWC tx queue full!\n");
#ifdef RTL8190_FASTEXTDEV_FUNCALL
					rtl865x_extDev_kfree_skb(pskb, FALSE);
#else
					dev_kfree_skb_any(pskb);
#endif
				}
				else {
#ifdef ENABLE_RTL_SKB_STATS
					rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
#endif
				}
				return 1;
			}
			else {
				// not over the bandwidth
				if (CIRC_CNT(priv->GBWC_tx_queue.head, priv->GBWC_tx_queue.tail, NUM_TXPKT_QUEUE)) {
					// there are already packets in queue, put in queue too for order
					int ret = enque(priv, &(priv->GBWC_tx_queue.head), &(priv->GBWC_tx_queue.tail),
							(unsigned int)(priv->GBWC_tx_queue.pSkb), NUM_TXPKT_QUEUE, (void *)pskb);
					if (ret == FALSE) {
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: BWC tx queue full!\n");
#ifdef RTL8190_FASTEXTDEV_FUNCALL
						rtl865x_extDev_kfree_skb(pskb, FALSE);
#else
						dev_kfree_skb_any(pskb);
#endif
					}
					else {
#ifdef ENABLE_RTL_SKB_STATS
						rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
#endif
					}
					return 1;
				}
				else {
					// can forward directly
					priv->GBWC_rx_count += pskb->len;
				}
			}
		}
	}

	return 0;
}
#endif


static void reorder_ctrl_pktout(struct rtl8190_priv *priv, struct sk_buff *pskb, struct stat_info *pstat)
{
//Joule 2009.03.10
//	if ((*(unsigned int *)&(pskb->cb[4])) == 0)
	struct stat_info *dst_pstat = (struct stat_info*)(*(unsigned int *)&(pskb->cb[4]));

	if( dst_pstat ==0 )
		rtl_netif_rx(priv, pskb, pstat);
	else
	{
#ifdef CONFIG_RTK_MESH
	#ifdef RX_RL_SHORTCUT
		if (GET_MIB(priv)->dot1180211sInfo.mesh_enable && isMeshPoint(pstat) && isMeshPoint(dst_pstat)){
        	DECLARE_TXINSN(txinsn);
			memcpy(txinsn.nhop_11s, dst_pstat->hwaddr, MACADDRLEN);
			txinsn.is_11s = 8;							
			fire_data_frame(pskb, priv->mesh_dev, &txinsn);
		} else
	#endif
        if(rtl8190_start_xmit(pskb, isMeshPoint(dst_pstat) ? priv->mesh_dev: priv->dev))
#else
		if (rtl8190_start_xmit(pskb, priv->dev))
#endif
			rtl_kfree_skb(priv, pskb, _SKB_RX_);
	}
}


static void reorder_ctrl_consumeQ(struct rtl8190_priv *priv, struct stat_info *pstat, unsigned char tid, int seg)
{
	int win_start, win_size;
	struct reorder_ctrl_entry *rc_entry;

	rc_entry  = &pstat->rc_entry[tid];
	win_start = rc_entry->win_start;
	win_size  = priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz;

	while (SN_LESS(win_start, rc_entry->last_seq) || (win_start == rc_entry->last_seq)) {
		if (rc_entry->packet_q[win_start & (win_size - 1)]) {
			reorder_ctrl_pktout(priv, rc_entry->packet_q[win_start & (win_size - 1)], pstat);
			rc_entry->packet_q[win_start & (win_size - 1)] = NULL;
			#ifdef _DEBUG_RTL8190_
			if (seg == 0)
				pstat->rx_rc_passupi++;
			else if (seg == 2)
				pstat->rx_rc_passup2++;
			else if (seg == 3)
				pstat->rx_rc_passup3++;
			else if (seg == 4)
				pstat->rx_rc_passup4++;
			#endif
		}
		win_start = SN_NEXT(win_start);
	}
	rc_entry->start_rcv = FALSE;
}


static int reorder_ctrl_timer_add(struct rtl8190_priv *priv, struct stat_info *pstat, int tid, int from_timeout)
{
	unsigned int now, timeout, new_timer=0;
	int setup_timer;
	int current_idx, next_idx;
	int sys_tick;

	current_idx = priv->pshare->rc_timer_head;

	while (CIRC_CNT(current_idx, priv->pshare->rc_timer_tail, RC_TIMER_NUM)) {
		if (priv->pshare->rc_timer[priv->pshare->rc_timer_tail].pstat == NULL) {
			priv->pshare->rc_timer_tail = (priv->pshare->rc_timer_tail + 1) & (RC_TIMER_NUM - 1);
			new_timer = 1;
		}
		else
			break;
	}

	if (CIRC_CNT(current_idx, priv->pshare->rc_timer_tail, RC_TIMER_NUM) == 0) {
		del_timer_sync(&priv->pshare->rc_sys_timer);
		setup_timer = 1;
	}
	else if (CIRC_SPACE(current_idx, priv->pshare->rc_timer_tail, RC_TIMER_NUM) == 0) {
		printk("%s: %s, RC timer overflow!\n", priv->dev->name, __FUNCTION__ );
		return -1;
	}
	else {	// some items in timer queue
		setup_timer = 0;
		if (new_timer)
			new_timer = priv->pshare->rc_timer[priv->pshare->rc_timer_tail].timeout;
	}

	next_idx = (current_idx + 1) & (RC_TIMER_NUM - 1);

	priv->pshare->rc_timer[current_idx].priv = priv;
	priv->pshare->rc_timer[current_idx].pstat = pstat;
	priv->pshare->rc_timer[current_idx].tid = (unsigned char)tid;
	priv->pshare->rc_timer_head = next_idx;

	now = (jiffies * 10000);
	timeout = now + priv->pmib->reorderCtrlEntry.ReorderCtrlTimeout;
	priv->pshare->rc_timer[current_idx].timeout = timeout;
	sys_tick = priv->pmib->reorderCtrlEntry.ReorderCtrlTimeout / 10000;

	if (!from_timeout) {
		if (setup_timer) {
			mod_timer(&priv->pshare->rc_sys_timer, jiffies + sys_tick);
		}
		else if (new_timer) {
			if (TSF_LESS(new_timer, now)) {
				mod_timer(&priv->pshare->rc_sys_timer, jiffies + sys_tick);
			}
			else {
				sys_tick = (new_timer - now) / 10000;
				if (sys_tick < 1)
					sys_tick = 1;
				mod_timer(&priv->pshare->rc_sys_timer, jiffies + sys_tick);
			}
		}
	}

	return current_idx;
}

#ifndef NEW_MAC80211_DRV
void reorder_ctrl_timeout(unsigned long task_priv)
{
	struct rtl8190_priv *priv = (struct rtl8190_priv *)task_priv;
	unsigned int timeout, current_time;
	struct reorder_ctrl_entry *rc_entry=NULL;
	struct rtl8190_priv *priv_this=NULL;
	struct stat_info *pstat;
	int win_start=0, win_size, win_end, head, tid=0, sys_tick;
	unsigned long flags;

	SAVE_INT_AND_CLI(flags);

	current_time = (jiffies * 10000);

	head = priv->pshare->rc_timer_head;
	win_size = priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz;

	while (CIRC_CNT(head, priv->pshare->rc_timer_tail, RC_TIMER_NUM))
	{
		pstat = priv->pshare->rc_timer[priv->pshare->rc_timer_tail].pstat;
		if (pstat) {
			timeout = priv->pshare->rc_timer[priv->pshare->rc_timer_tail].timeout;
			if (TSF_LESS(timeout, current_time) || (TSF_DIFF(timeout, current_time) < 5000)) {
				priv_this = priv->pshare->rc_timer[priv->pshare->rc_timer_tail].priv;
				tid       = priv->pshare->rc_timer[priv->pshare->rc_timer_tail].tid;
				rc_entry  = &pstat->rc_entry[tid];
				win_start = rc_entry->win_start;
				win_end   = (win_start + win_size) & 0xfff;
				priv->pshare->rc_timer[priv->pshare->rc_timer_tail].pstat = NULL;
			}
			else {
				sys_tick = (timeout - current_time) / 10000;
				if (sys_tick < 1)
					sys_tick = 1;
				mod_timer(&priv->pshare->rc_sys_timer, jiffies + sys_tick);

				if (TSF_LESS(timeout, current_time))
					printk("Setup RC timer %d too late (now %d)\n", timeout, current_time);

				RESTORE_INT(flags);
				return;
			}
		}

		priv->pshare->rc_timer_tail = (priv->pshare->rc_timer_tail + 1) & (RC_TIMER_NUM - 1);

		if (pstat) {
			if (!(rc_entry->packet_q[win_start & (win_size - 1)]))
				win_start = SN_NEXT(win_start);

			while (rc_entry->packet_q[win_start & (win_size - 1)]) {
				reorder_ctrl_pktout(priv_this, rc_entry->packet_q[win_start & (win_size - 1)], pstat);
				rc_entry->packet_q[win_start & (win_size - 1)] = NULL;
				#ifdef _DEBUG_RTL8190_
				pstat->rx_rc_passupi++;
				#endif
				win_start = SN_NEXT(win_start);
			}

			rc_entry->win_start = win_start;

			if (SN_LESS(win_start, rc_entry->last_seq) || (win_start == rc_entry->last_seq)) {
				rc_entry->rc_timer_id = reorder_ctrl_timer_add(priv_this, pstat, tid, 1) + 1;
				if (rc_entry->rc_timer_id == 0)
					reorder_ctrl_consumeQ(priv_this, pstat, tid, 0);
			}
			else {
				rc_entry->start_rcv = FALSE;
				rc_entry->rc_timer_id = 0;
			}
		}
	}

	if (CIRC_CNT(priv->pshare->rc_timer_head, priv->pshare->rc_timer_tail, RC_TIMER_NUM)) {
		sys_tick = (priv->pshare->rc_timer[priv->pshare->rc_timer_tail].timeout - current_time) / 10000;
		if (sys_tick < 1)
				sys_tick = 1;
		mod_timer(&priv->pshare->rc_sys_timer, jiffies + sys_tick);

		if (TSF_LESS(priv->pshare->rc_timer[priv->pshare->rc_timer_tail].timeout, current_time))
			printk("Setup RC timer %d too late (now %d)\n", priv->pshare->rc_timer[priv->pshare->rc_timer_tail].timeout, current_time);
	}
	RESTORE_INT(flags);
}
#endif  //ifndef NEW_MAC80211_DRV

/* ====================================================================================
   segment   1      2               3                         4
   -----------------+--------------------------------+-------------------------
                  win_start                       win_end
                    +--------------------------------+
                                  win_size

   segment 1: drop this packet
   segment 2: indicate this packet, then indicate the following packets until a hole
   segment 3: queue this packet in corrosponding position
   segment 4: indicate queued packets until SN_DIFF(seq, win_start)<win_size, then
              queue this packet
====================================================================================*/
int check_win_seqment(unsigned short win_start, unsigned short win_end, unsigned short seq)
{
	if (SN_LESS(seq, win_start))
		return 1;
	else if (seq == win_start)
		return 2;
	else if (SN_LESS(win_start, seq) && SN_LESS(seq, win_end))
		return 3;
	else
		return 4;
}


static int reorder_ctrl_check(struct rtl8190_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo)
{
	unsigned short	seq;
	unsigned char	tid;
	int index, segment;
	int win_start, win_size, win_end;
	struct reorder_ctrl_entry *rc_entry;

	seq       = pfrinfo->seq;
	tid       = pfrinfo->tid;
	rc_entry  = &pstat->rc_entry[tid];
	win_start = rc_entry->win_start;
	win_size  = priv->pmib->reorderCtrlEntry.ReorderCtrlWinSz;
	win_end   = (win_start + win_size) & 0xfff;

	if (!pfrinfo->paggr && (rc_entry->start_rcv == FALSE))
		return TRUE;

	if (rc_entry->start_rcv == FALSE)
	{
		rc_entry->start_rcv = TRUE;
		rc_entry->win_start = SN_NEXT(seq);
		rc_entry->last_seq = seq;
		return TRUE;
	}
	else
	{
		segment = check_win_seqment(win_start, win_end, seq);
		if (segment == 1) {
			#ifdef _DEBUG_RTL8190_
			pstat->rx_rc_drop1++;
			#endif
			//priv->ext_stats.rx_data_drops++;
			//DEBUG_ERR("RX DROP: skb behind the window\n");
			//rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
			//return FALSE;
			return TRUE;
		}
		else if (segment == 2) {
			reorder_ctrl_pktout(priv, pfrinfo->pskb, pstat);
			win_start = SN_NEXT(win_start);
			while (rc_entry->packet_q[win_start & (win_size - 1)]) {
				reorder_ctrl_pktout(priv, rc_entry->packet_q[win_start & (win_size - 1)], pstat);
				rc_entry->packet_q[win_start & (win_size - 1)] = NULL;
				win_start = SN_NEXT(win_start);
				#ifdef _DEBUG_RTL8190_
				pstat->rx_rc_passup2++;
				#endif
			}
			rc_entry->win_start = win_start;
			if (SN_LESS(rc_entry->last_seq, seq))
				rc_entry->last_seq = seq;

			if (rc_entry->rc_timer_id)
				priv->pshare->rc_timer[rc_entry->rc_timer_id - 1].pstat = NULL;
			if (SN_LESS(rc_entry->last_seq, win_start))
				rc_entry->rc_timer_id = 0;
			else {
				rc_entry->rc_timer_id = reorder_ctrl_timer_add(priv, pstat, tid, 0) + 1;
				if (rc_entry->rc_timer_id == 0)
					reorder_ctrl_consumeQ(priv, pstat, tid, 2);
			}
			return FALSE;
		}
		else if (segment == 3) {
			index = seq & (win_size - 1);
			if (rc_entry->packet_q[index]) {
				#ifdef _DEBUG_RTL8190_
				pstat->rx_rc_drop3++;
				#endif
				priv->ext_stats.rx_data_drops++;
				DEBUG_ERR("RX DROP: skb already in rc queue\n");
				rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
				return FALSE;
			}
			else {
				rc_entry->packet_q[index] = pfrinfo->pskb;
				#ifdef _DEBUG_RTL8190_
				pstat->rx_rc_reorder3++;
				#endif
			}
			if (SN_LESS(rc_entry->last_seq, seq))
				rc_entry->last_seq = seq;
			if (rc_entry->rc_timer_id == 0) {
				rc_entry->rc_timer_id = reorder_ctrl_timer_add(priv, pstat, tid, 0) + 1;
				if (rc_entry->rc_timer_id == 0)
					reorder_ctrl_consumeQ(priv, pstat, tid, 3);
			}
			return FALSE;
		}
		else {	// (segment == 4)
			while ((SN_DIFF(seq, win_start) >= win_size) || (rc_entry->packet_q[win_start & (win_size - 1)])) {
				if (rc_entry->packet_q[win_start & (win_size - 1)]) {
					reorder_ctrl_pktout(priv, rc_entry->packet_q[win_start & (win_size - 1)], pstat);
					rc_entry->packet_q[win_start & (win_size - 1)] = NULL;
					#ifdef _DEBUG_RTL8190_
					pstat->rx_rc_passup4++;
					#endif
				}
				win_start = SN_NEXT(win_start);
			}
			rc_entry->win_start = win_start;

			index = seq & (win_size - 1);
			rc_entry->packet_q[index] = pfrinfo->pskb;
			#ifdef _DEBUG_RTL8190_
			pstat->rx_rc_reorder4++;
			#endif
			rc_entry->last_seq = seq;
			if (rc_entry->rc_timer_id)
				priv->pshare->rc_timer[rc_entry->rc_timer_id - 1].pstat = NULL;
			rc_entry->rc_timer_id = reorder_ctrl_timer_add(priv, pstat, tid, 0) + 1;
			if (rc_entry->rc_timer_id == 0)
				reorder_ctrl_consumeQ(priv, pstat, tid, 4);
			return FALSE;
		}
	}
}


#ifdef RX_SHORTCUT
/*---------------------------------------------------------------
	return value:
	TRUE:	MIC ok
	FALSE:	MIC error
----------------------------------------------------------------*/
static int wait_mic_done_and_compare(unsigned char *org_mic, unsigned char *tkipmic)
{
	register unsigned long int l,r;
	int delay = 20;

	while ((*(volatile unsigned int *)GDMAISR & GDMA_COMPIP) == 0) {
		delay_us(delay);
		delay = delay / 2;
	}

	l = *(volatile unsigned int *)GDMAICVL;
	r = *(volatile unsigned int *)GDMAICVR;

	tkipmic[0] = (unsigned char)(l & 0xff);
	tkipmic[1] = (unsigned char)((l >> 8) & 0xff);
	tkipmic[2] = (unsigned char)((l >> 16) & 0xff);
	tkipmic[3] = (unsigned char)((l >> 24) & 0xff);
	tkipmic[4] = (unsigned char)(r & 0xff);
	tkipmic[5] = (unsigned char)((r >> 8) & 0xff);
	tkipmic[6] = (unsigned char)((r >> 16) & 0xff);
	tkipmic[7] = (unsigned char)((r >> 24) & 0xff);

	return (memcmp(org_mic, tkipmic, 8) ? FALSE : TRUE);
}

unsigned char rfc1042_header[WLAN_LLC_HEADER_SIZE]={0xaa,0xaa,0x03,00,00,00};
#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)
extern void mesh_shortcut_update(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo);
#endif
/*---------------------------------------------------------------
	return value:
	0:	shortcut ok, rx data has passup
	1:	discard this packet
	-1:	can't do shortcut, data path should be continued
 ---------------------------------------------------------------*/
__MIPS16
__IRAM_FASTEXTDEV
static int rx_shortcut(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo)
{
//	unsigned long flags=0;
	struct stat_info *pstat, *dst_pstat;
	int privacy, tkip_mic_ret=0;
	int payload_length, offset, pos;
	struct wlan_ethhdr_t *e_hdr;
	unsigned char rxmic[8], tkipmic[8];
	struct sk_buff skb_copy;
	unsigned char wlanhdr_copy[sizeof(struct wlanllc_hdr)];
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned char da[MACADDRLEN];
	unsigned short tpcache;
#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)

	unsigned char *meshHdrPtr=NULL;
	//unsigned char zeromac[6]={0x00,0x00,0x00,0x00,0x00,0x00};
	struct path_sel_entry *pEntry = NULL;
#endif

	pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
/*
	printk("pfrinfo->is_11s = %d\n", pfrinfo->is_11s);
	//printk("pfrinfo->mesh_header.DestMACAddr = %02x\n", pfrinfo->mesh_header.DestMACAddr[5]);
	//printk("pfrinfo->mesh_header.SrcMACAddr = %02x\n", pfrinfo->mesh_header.SrcMACAddr[5]);
	printk("pstat->rx_wlanhdr.wlanhdr.meshhdr.DestMACAddr = %02x\n", pstat->rx_wlanhdr.wlanhdr.meshhdr.DestMACAddr[5]);
	printk("pstat->rx_wlanhdr.wlanhdr.meshhdr.SrcMACAddr = %02x\n", pstat->rx_wlanhdr.wlanhdr.meshhdr.SrcMACAddr[5]);
	printk("pstat->rx_payload_offset = %d\n", pstat->rx_payload_offset);
	*/				

#if defined(CONFIG_RTK_MESH) && !defined(RX_RL_SHORTCUT)

	// RTK mesh doesn't support shortcut now-- chris 071909.
	if (GET_MIB(priv)->dot1180211sInfo.mesh_enable)
	return -1;
#endif

#if 0	// already flush cache in rtl8190_rx_isr()
#ifndef RTL8190_CACHABLE_CLUSTER
#ifdef __MIPSEB__
	pframe = (UINT8 *)((unsigned int)pframe | 0x20000000);
#endif
#endif
#endif


	// bcm old 11n chipset iot debug
#if defined(RTL8190) || defined(RTL8192E)
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	if (IS_ROOT_INTERFACE(priv))
#endif
	if ((pstat) && (pstat == priv->fsync_monitor_pstat) && is_MCS_rate(pfrinfo->rx_rate)) {
		if (((pfrinfo->rx_rate & 0x7f) < priv->pshare->rf_ft_var.mcs_ignore_lower)
			|| ((pfrinfo->rx_rate & 0x7f) > priv->pshare->rf_ft_var.mcs_ignore_upper))
			pstat->rx_rate_bitmap |= BIT(pfrinfo->rx_rate & 0x7f);
	}
#endif

	if (priv->pmib->dot11OperationEntry.guest_access
#ifdef CONFIG_RTL8186_KB
			||(pstat && pstat->ieee8021x_ctrlport == DOT11_PortStatus_Guest)
#endif
		)
		return -1;

	if (pstat && (pstat->rx_payload_offset > 0) &&
		(GetFragNum(pframe) == 0) && (GetMFrag(pframe) == 0))
	{
		tpcache = GetTupleCache(pframe);
		pos = GetSequence(pframe) & (TUPLE_WINDOW - 1);
		if (tpcache == pstat->tpcache[pfrinfo->tid][pos]) {
			priv->ext_stats.rx_decache++;
			rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
			SNMP_MIB_INC(dot11FrameDuplicateCount, 1);
			return 0;
		}

		privacy = GetPrivacy(pframe);
		memcpy(da, pfrinfo->da, MACADDRLEN);

		// check wlan header
		if ((pstat->rx_privacy == privacy) &&
			!memcmp(GetAddr1Ptr(pframe), pstat->rx_wlanhdr.wlanhdr.addr1, 18))
		{

#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)

			if (pfrinfo->is_11s) {
				
				if (memcmp(GetAddr4Ptr(pframe), pstat->rx_wlanhdr.wlanhdr.addr4, MACADDRLEN)){
					return -1;
				}
				if (memcmp(GetAddr3Ptr(pframe), GET_MY_HWADDR, MACADDRLEN)) {
					pEntry = priv->pathsel_table->search_entry(priv->pathsel_table, GetAddr3Ptr(pframe));
					if (pEntry) {	
						//printk("pEntry -> nh %02x \n", pEntry->nexthopMAC[5]);
					}
					else
					return -1;
				}
				
				
				meshHdrPtr = getMeshHeader(priv, priv->pmib->dot11sKeysTable.dot11Privacy, pframe); 
				if( meshHdrPtr ) 
				{
					if((int)(*(meshHdrPtr+1)) == 0)	// *(meshHdrPtr+1)) = TTL; filter packets due to duplicate 
						return -1;
					
					
					memcpy(&(pfrinfo->mesh_header), meshHdrPtr, sizeof(struct lls_mesh_header) );

				} 
				else {
					return -1;
				}
				
				
				if (( pfrinfo->mesh_header.mesh_flag)&1) //only shorcut 6 address
				{
					if (memcmp(pfrinfo->mesh_header.DestMACAddr, pstat->rx_wlanhdr.wlanhdr.meshhdr.DestMACAddr, 6) ||
						memcmp(pfrinfo->mesh_header.SrcMACAddr, pstat->rx_wlanhdr.wlanhdr.meshhdr.SrcMACAddr, 6)) {
							return -1;
						}
				} else {
					return -1;
				}
				
				mesh_shortcut_update(priv, pfrinfo);
				pfrinfo->pskb->dev = priv->mesh_dev;
			}
			else
				
#endif //CONFIG_RTK_MESH

#ifdef WDS
			if (pfrinfo->to_fr_ds == 3 
	#ifdef CONFIG_RTK_MESH
				&& priv->pmib->dot11WdsInfo.wdsEnabled
	#endif
				) {
				if (memcmp(GetAddr4Ptr(pframe), pstat->rx_wlanhdr.wlanhdr.addr4, 6))
					return -1;
				pfrinfo->pskb->dev = getWdsDevByAddr(priv, GetAddr2Ptr(pframe));
			}
			else
#endif
				pfrinfo->pskb->dev = priv->dev;

			offset = pstat->rx_payload_offset + sizeof(rfc1042_header);

/*
			printk("pstat->rx_payload_offset = %02x, 0xaa, cmp = %d \n",pframe[pstat->rx_payload_offset],
				memcmp(&pframe[pstat->rx_payload_offset], rfc1042_header, 6));
			printk("pstat->rx_ethhdr.type = %02x, pframe[offset] = %02x, cmp= %d \n", pstat->rx_ethhdr.type, pframe[offset],
				memcmp(&pstat->rx_ethhdr.type,&pframe[offset], 2));
*/			
			// check snap header
			if (memcmp(&pframe[pstat->rx_payload_offset], rfc1042_header, 6) ||
				memcmp(&pstat->rx_ethhdr.type,&pframe[offset], 2))
				return -1;

			payload_length = pfrinfo->pktlen - offset - pstat->rx_trim_pad - 2;
			if (payload_length < WLAN_ETHHDR_LEN)
				return -1;

			if (privacy)
			{

#ifdef WDS
				if (pfrinfo->to_fr_ds == 3
	#ifdef CONFIG_RTK_MESH
					&& priv->pmib->dot11WdsInfo.wdsEnabled
	#endif
				)
					privacy = priv->pmib->dot11WdsInfo.wdsPrivacy;
				 
				else
#endif
				privacy = get_sta_encrypt_algthm(priv, pstat);

#if defined(CONFIG_RTL_WAPI_SUPPORT)
				if (privacy==_WAPI_SMS4_)
				{
					/*	Decryption	*/
					if (SecSWSMS4Decryption(priv, pstat, pfrinfo) == FAIL)
					{
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: WAPI decrpt error!\n");
						priv->ext_stats.rx_data_drops++;
						rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
						return 1;
					}
					pframe = get_pframe(pfrinfo);
				}
				else
#endif
				{					
					if (privacy == _TKIP_PRIVACY_)
					{
						memcpy((void *)rxmic, (void *)(pframe + pfrinfo->pktlen - 8 - 4), 8); // 8 michael, 4 icv
	//					SAVE_INT_AND_CLI(flags);
						tkip_mic_ret = tkip_rx_mic(priv, pframe, pfrinfo->da, pfrinfo->sa,
							pfrinfo->tid, pframe + pfrinfo->hdr_len + 8,
							pfrinfo->pktlen - pfrinfo->hdr_len - 8 - 8 - 4, tkipmic, 1); // 8 IV, 8 Mic, 4 ICV
						if (tkip_mic_ret) { // MIC completed
	//						RESTORE_INT(flags);
							if (memcmp(rxmic, tkipmic, 8)) {
								return -1;
							}
						}
						else {
							memcpy(&skb_copy, pfrinfo->pskb, sizeof(skb_copy));
							memcpy(wlanhdr_copy, pframe, sizeof(wlanhdr_copy));
						}
					}
				}
			}

			rx_sum_up(NULL, pstat, pfrinfo->pktlen, 0);
			update_sta_rssi(priv, pstat, pfrinfo);

			//printk("RXSC\n");
			
#ifdef SUPPORT_SNMP_MIB
			if (IS_MCAST(da))
				SNMP_MIB_INC(dot11MulticastReceivedFrameCount, 1);
#endif

			/* chop 802.11 header from skb. */
			//skb_put(pfrinfo->pskb, pfrinfo->pktlen);	// pskb->tail will be wrong
			pfrinfo->pskb->tail = pfrinfo->pskb->data + pfrinfo->pktlen;
			pfrinfo->pskb->len = pfrinfo->pktlen;

			skb_pull(pfrinfo->pskb, offset+2);
			
#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)

			if (pfrinfo->is_11s && (pEntry==NULL)) {
				skb_pull(pfrinfo->pskb, 16); //because we guarantee only 6 address frame enters shortcut
				payload_length -= 16;
			}
#endif
			
			e_hdr = (struct wlan_ethhdr_t *)skb_push(pfrinfo->pskb, WLAN_ETHHDR_LEN);
			memcpy((unsigned char *)e_hdr, (unsigned char *)&pstat->rx_ethhdr, sizeof(struct wlan_ethhdr_t));
			/* chop off the 802.11 CRC */
			
			skb_trim(pfrinfo->pskb, payload_length + WLAN_ETHHDR_LEN);

			
			//printk("RXSC skb_pull offset+2 = %d\n", offset+2);
			//printk("RXSC pstat->rx_ethhdr dest= %02x\n", e_hdr->daddr[5]);
			//printk("RXSC pstat->rx_ethhdr src = %02x\n", e_hdr->saddr[5]);

			if ((privacy == _TKIP_PRIVACY_) && (tkip_mic_ret == FALSE)) {
				if (wait_mic_done_and_compare(rxmic, tkipmic) == FALSE) {
//					RESTORE_INT(flags);
					memcpy(pfrinfo->pskb, &skb_copy, sizeof(skb_copy));
					memcpy(pframe, wlanhdr_copy, sizeof(wlanhdr_copy));
					return -1;
				}
//				RESTORE_INT(flags);
			}

			pstat->tpcache[pfrinfo->tid][pos] = tpcache;

			if (OPMODE & WIFI_AP_STATE)
			{

// Button 2009.7.31	
#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)
                                if (pfrinfo->is_11s )
				dst_pstat = (pEntry) ? get_stainfo(priv, pEntry->nexthopMAC) : 0;	
else
#endif
				dst_pstat = get_stainfo(priv, da);

#if defined(CONFIG_RTK_MESH) ||  defined(WDS)
				if ((pfrinfo->to_fr_ds==3) ||
					(dst_pstat == NULL) || !(dst_pstat->state & WIFI_ASOC_STATE))
#else
				if ((dst_pstat == NULL) || (!(dst_pstat->state & WIFI_ASOC_STATE)))
#endif
				{

// Button 2009.7.31
	#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)
							if (pfrinfo->is_11s && (pEntry!=NULL)) {
						//chris 080409
								DECLARE_TXINSN(txinsn);
								memcpy(txinsn.nhop_11s, pEntry->nexthopMAC, MACADDRLEN);
										(int)(*(meshHdrPtr+1))--;
								txinsn.is_11s = 8;
						if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
						  priv->pmib->reorderCtrlEntry.ReorderCtrlEnable) {
							*(unsigned int *)&(pfrinfo->pskb->cb[4]) = dst_pstat;
							if (reorder_ctrl_check(priv, pstat, pfrinfo)){
								fire_data_frame(pfrinfo->pskb, priv->mesh_dev, &txinsn);
							}
						}else {
							fire_data_frame(pfrinfo->pskb, priv->mesh_dev, &txinsn);
						}
					} else 
	#endif 
					if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
						priv->pmib->reorderCtrlEntry.ReorderCtrlEnable) {
						*(unsigned int *)&(pfrinfo->pskb->cb[4]) = 0;
						if (reorder_ctrl_check(priv, pstat, pfrinfo)){
							rtl_netif_rx(priv, pfrinfo->pskb, pstat);
						}
						}
					else
						rtl_netif_rx(priv, pfrinfo->pskb, pstat);
					}
				else
				{
					if (priv->pmib->dot11OperationEntry.block_relay == 1) {
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: Relay unicast packet is blocked in shortcut!\n");
						rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
						return 1;
					}
					else if (priv->pmib->dot11OperationEntry.block_relay == 2) {
						DEBUG_INFO("Relay unicast packet is blocked! Indicate to bridge in shortcut\n");
						rtl_netif_rx(priv, pfrinfo->pskb, pstat);
					}
					else {
#ifdef ENABLE_RTL_SKB_STATS
						rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif
#ifdef GBWC
						if (GBWC_forward_check(priv, pfrinfo->pskb, pstat)) {
							// packet is queued, nothing to do
						}
						else
#endif
						if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
							priv->pmib->reorderCtrlEntry.ReorderCtrlEnable) {
							*(unsigned int *)&(pfrinfo->pskb->cb[4]) = (unsigned int)dst_pstat;	// backup pstat pointer
							if (reorder_ctrl_check(priv, pstat, pfrinfo)) {
					#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)
								if (rtl8190_start_xmit(pfrinfo->pskb, isMeshPoint(dst_pstat)? priv->mesh_dev: priv->dev))
					#else
								if (rtl8190_start_xmit(pfrinfo->pskb, priv->dev))
					#endif
									rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
							}
						}
						else {
					#if defined(CONFIG_RTK_MESH) && defined(RX_RL_SHORTCUT)
							if (rtl8190_start_xmit(pfrinfo->pskb, isMeshPoint(dst_pstat)? priv->mesh_dev: priv->dev))
					#else
							if (rtl8190_start_xmit(pfrinfo->pskb, priv->dev))
					#endif
								rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
						}
					}
				}
				return 0;
			}
#ifdef CLIENT_MODE
			else if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE))
			{
				priv->rxDataNumInPeriod++;
				if (IS_MCAST(pfrinfo->pskb->data))
					priv->rxMlcstDataNumInPeriod++;

#ifdef RTK_BR_EXT
				if (!priv->pmib->ethBrExtInfo.nat25sc_disable &&
						!(pfrinfo->pskb->data[0] & 1) &&
						*((unsigned short *)(pfrinfo->pskb->data+ETH_ALEN*2)) == __constant_htons(ETH_P_IP) &&
							!memcmp(priv->scdb_ip, pfrinfo->pskb->data+ETH_HLEN+16, 4))
					memcpy(pfrinfo->pskb->data, priv->scdb_mac, ETH_ALEN);
				else
				if(nat25_handle_frame(priv, pfrinfo->pskb) == -1) {
					priv->ext_stats.rx_data_drops++;
					DEBUG_ERR("RX DROP: nat25_handle_frame fail in shortcut!\n");
					rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
					return 1;
				}
#endif
				if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
					priv->pmib->reorderCtrlEntry.ReorderCtrlEnable) {
					*(unsigned int *)&(pfrinfo->pskb->cb[4]) = 0;
					if (reorder_ctrl_check(priv, pstat, pfrinfo))
						rtl_netif_rx(priv, pfrinfo->pskb, pstat);
				}
				else
					rtl_netif_rx(priv, pfrinfo->pskb, pstat);
				return 0;
			}
#endif
			else
			{
				priv->ext_stats.rx_data_drops++;
				DEBUG_ERR("RX DROP: Non supported mode in process_datafrme in shortcut\n");
				rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
				return 1;
			}
		}
	}
	return -1;
}
#endif // RX_SHORTCUT


#ifdef DRVMAC_LB
void lb_convert(struct rtl8190_priv *priv, unsigned char *pframe)
{
	unsigned char *addr1, *addr2, *addr3;

	if (get_tofr_ds(pframe) == 0x01)
	{
		SetToDs(pframe);
		ClearFrDs(pframe);
		addr1 = GetAddr1Ptr(pframe);
		addr2 = GetAddr2Ptr(pframe);
		addr3 = GetAddr3Ptr(pframe);
		if (addr1[0] & 0x01) {
			memcpy(addr3, addr1, 6);
			memcpy(addr1, addr2, 6);
			memcpy(addr2, priv->pmib->miscEntry.lb_da, 6);
		}
		else {
			memcpy(addr1, addr2, 6);
			memcpy(addr2, priv->pmib->miscEntry.lb_da, 6);
		}
	}
}
#endif


/*
	Strip from "validate_mpdu()"

	0:	no reuse, allocate new skb due to the current is queued.
	1:	reuse! due to error pkt or short pkt.
*/
static int rtl8190_rx_procCtrlPkt(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo
#ifdef MBSSID
				,int vap_idx
#endif
				)
{
	unsigned char *pframe = get_pframe(pfrinfo);

	if (((GetFrameSubType(pframe)) != WIFI_PSPOLL) ||
			(pfrinfo->to_fr_ds != 0x00))
		return 1;

#ifdef MBSSID
	if (
#ifdef RTL8192SE
	GET_ROOT(priv)->pmib->miscEntry.vap_enable &&
#endif
	(vap_idx >= 0))
	{
		priv = priv->pvap_priv[vap_idx];
		if (!(OPMODE & WIFI_AP_STATE))
			return 1;
	}
	else
#endif
	{
		if (!(OPMODE & WIFI_AP_STATE)) {
#ifdef UNIVERSAL_REPEATER
			if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
				priv = GET_VXD_PRIV(priv);
			else
				return 1;
#else
			return 1;
#endif
		}
	}

	if (!IS_BSSID(priv, GetAddr1Ptr(pframe)))
		return 1;

	// check power save state
	if (get_stainfo(priv, GetAddr2Ptr(pframe)) != NULL)
			pwr_state(priv, pfrinfo);
	else
		return 1;

#ifdef RTL8190_DIRECT_RX
	rtl8190_rx_ctrlframe(priv, NULL, pfrinfo);
#else
	list_add_tail(&(pfrinfo->rx_list), &(priv->rx_ctrllist));
#endif
	return 0;
}


static int rtl8190_rx_procNullPkt(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo
#ifdef MBSSID
				,int vap_idx
#endif
				)
{
	unsigned char *sa = pfrinfo->sa;
	struct stat_info *pstat = get_stainfo(priv, sa);
	unsigned char *pframe = get_pframe(pfrinfo);

#ifdef UNIVERSAL_REPEATER
	if ((pstat == NULL) && IS_DRV_OPEN(GET_VXD_PRIV(priv))) {
		pstat = get_stainfo(GET_VXD_PRIV(priv), sa);
		if (pstat)
			priv = GET_VXD_PRIV(priv);
	}
#endif

#ifdef MBSSID
	if ((pstat == NULL)
#ifdef RTL8192SE
		&& GET_ROOT(priv)->pmib->miscEntry.vap_enable
#endif
		&& (vap_idx >= 0)) {
		pstat = get_stainfo(priv->pvap_priv[vap_idx], sa);
		if (pstat)
			priv = priv->pvap_priv[vap_idx];
	}
#endif

	if (pstat && (pstat->state & WIFI_ASOC_STATE)) {
		rx_sum_up(NULL, pstat, pfrinfo->pktlen, 0);
		update_sta_rssi(priv, pstat, pfrinfo);

		// check power save state
		if (OPMODE & WIFI_AP_STATE) {
			if (IS_BSSID(priv, GetAddr1Ptr(pframe)))
				pwr_state(priv, pfrinfo);
		}
	}

#if defined(SEMI_QOS) && defined(WMM_APSD)
	if ((QOS_ENABLE) && (APSD_ENABLE) && (OPMODE & WIFI_AP_STATE) && pstat && (pstat->state & WIFI_SLEEP_STATE) &&
		(pstat->QosEnabled) && (pstat->apsd_bitmap & 0x0f) && ((GetFrameSubType(pframe)) == (WIFI_DATA_NULL | BIT(7)))) {
		unsigned char *bssid = GetAddr1Ptr(pframe);
		if (IS_BSSID(priv, bssid))
		{
#ifdef RTL8190_DIRECT_RX
			rtl8190_rx_dataframe(priv, NULL, pfrinfo);
#else
			list_add_tail(&(pfrinfo->rx_list), &(priv->rx_datalist));
#endif
			return 0;
		}
	}
#endif

	return 1;
}


/*
	Check the "to_fr_ds" field:

						FromDS = 0
						ToDS = 0
*/
static int rtl8190_rx_dispatch_mgmt_adhoc(struct rtl8190_priv **priv_p, struct rx_frinfo *pfrinfo
#ifdef MBSSID
				,int vap_idx
#endif
				)
{
	int reuse = 1;
	struct rtl8190_priv *priv = *priv_p;
	unsigned int opmode = OPMODE;
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned int frtype = GetFrameType(pframe);
	unsigned char *da = pfrinfo->da;
	unsigned char *bssid = GetAddr3Ptr(pframe);
#ifdef MBSSID
	int i;
#endif

	if ((GetMFrag(pframe)) && (frtype == WIFI_MGT_TYPE))
		goto out;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	// If mgt packet & (beacon or prob-rsp), put in root interface Q
	//		then it will be handled by root & virtual interface
	// If mgt packet & (prob-req), put in AP interface Q
	// If mgt packet & (others), check BSSID (addr3) for matched interface

	if (frtype == WIFI_MGT_TYPE)
	{
		unsigned short frame_type = GetFrameSubType(pframe);
		int vxd_interface_ready=1, vap_interface_ready=0;

#ifdef UNIVERSAL_REPEATER
		if (!IS_DRV_OPEN(GET_VXD_PRIV(priv)) ||
			((opmode & WIFI_STATION_STATE) && !(GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)))
			vxd_interface_ready = 0;
#endif

#ifdef MBSSID
#ifdef RTL8192SE
		if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
#endif
		{
		if (opmode & WIFI_AP_STATE) {
			for (i=0; i<RTL8190_NUM_VWLAN; i++) {
				if (IS_DRV_OPEN(priv->pvap_priv[i]))
					vap_interface_ready = 1;
			}
		}
		}
#endif

		if (!vxd_interface_ready && !vap_interface_ready)
			goto put_in_que;

		if (frame_type == WIFI_BEACON || frame_type == WIFI_PROBERSP) {
			pfrinfo->is_br_mgnt = 1;
			goto put_in_que;
		}

		if (frame_type == WIFI_PROBEREQ) {
#ifdef MBSSID
#ifdef RTL8192SE
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
#endif
			{
			if (vap_interface_ready) {
				pfrinfo->is_br_mgnt = 1;
				goto put_in_que;
			}
			}
#endif
#ifdef UNIVERSAL_REPEATER
			if (opmode & WIFI_STATION_STATE) {
				priv = GET_VXD_PRIV(priv);
				opmode = OPMODE;
				goto put_in_que;
			}
#endif
		}
		else { // not (Beacon, Probe-rsp, probe-rsp)
#ifdef MBSSID
#ifdef RTL8192SE
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
#endif
			{
			if (vap_idx >= 0) {
				priv = priv->pvap_priv[vap_idx];
				goto put_in_que;
			}
			}
#endif
#ifdef UNIVERSAL_REPEATER
			if (!memcmp(GET_VXD_PRIV(priv)->pmib->dot11Bss.bssid, bssid, MACADDRLEN)) {
				priv = GET_VXD_PRIV(priv);
				opmode = OPMODE;
			}
#endif
		}
	}
put_in_que:
#endif // UNIVERSAL_REPEATER || MBSSID

	if (opmode & WIFI_AP_STATE)
	{
		if (IS_MCAST(da))
		{
			// For AP mode, if DA == MCAST, then BSSID should be also MCAST
			if (IS_MCAST(bssid))
				reuse = 0;

			else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G)
				reuse = 0;

#ifdef WDS
			else if (priv->pmib->dot11WdsInfo.wdsEnabled)
				reuse = 0;
#endif

#ifdef CONFIG_RTK_MESH
			else if (GET_MIB(priv)->dot1180211sInfo.mesh_enable)
				reuse = 0;
#endif

			else if (opmode & WIFI_SITE_MONITOR)
				reuse = 0;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			else if (pfrinfo->is_br_mgnt && reuse)
				reuse = 0;
#endif

			else
				{}
		}
		else
		{
			// For AP mode, if DA is non-MCAST, then it must be BSSID, and bssid == BSSID
			if (IS_BSSID(priv, da) && (IS_BSSID(priv, bssid)
#ifdef CONFIG_RTK_MESH // 802.11s Draft 1.0: page 11, Line 19~20
						// wildcard bssid is also acceptable -- chris
					|| (GET_MIB(priv)->dot1180211sInfo.mesh_enable 
							&& (!memcmp(bssid, "\x0\x0\x0\x0\x0\x0", 6) || !memcmp(bssid, "\xff\xff\xff\xff\xff\xff", 6)) 
							&& (frtype == WIFI_MGT_TYPE))
#endif
			))
				reuse = 0;

			else if (opmode & WIFI_SITE_MONITOR)
				reuse = 0;

#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
			else if (pfrinfo->is_br_mgnt && reuse)
				reuse = 0;
#endif
#ifdef WDS
			else if (priv->pmib->dot11WdsInfo.wdsEnabled && priv->pmib->dot11WdsInfo.wdsNum)
				reuse = 0;
#endif

			else
				{}
		}

		if (!reuse) {
			if (frtype == WIFI_MGT_TYPE)
			{
#ifdef RTL8190_DIRECT_RX
				rtl8190_rx_mgntframe(priv, NULL, pfrinfo);
#else
				list_add_tail(&(pfrinfo->rx_list), &(priv->rx_mgtlist));
#endif
			}
			else
				reuse = 1;
		}
	}
#ifdef CLIENT_MODE
	else if (opmode & WIFI_STATION_STATE)
	{
		// For Station mode, sa and bssid should always be BSSID, and DA is my mac-address
		// in case of to_fr_ds = 0x00, then it must be mgt frame type

		unsigned char *myhwaddr = priv->pmib->dot11OperationEntry.hwaddr;
		if (IS_MCAST(da) || !memcmp(da, myhwaddr, MACADDRLEN))
			reuse = 0;

		if (!reuse) {
			if (frtype == WIFI_MGT_TYPE)
			{
#ifdef RTL8190_DIRECT_RX
				rtl8190_rx_mgntframe(priv, NULL, pfrinfo);
#else
				list_add_tail(&(pfrinfo->rx_list), &(priv->rx_mgtlist));
#endif
			}
			else
				reuse = 1;
		}
	}
	else if (opmode & WIFI_ADHOC_STATE)
	{
		unsigned char *myhwaddr = priv->pmib->dot11OperationEntry.hwaddr;
		if (IS_MCAST(da) || !memcmp(da, myhwaddr, MACADDRLEN))
		{
			if (frtype == WIFI_MGT_TYPE)
			{
#ifdef RTL8190_DIRECT_RX
				rtl8190_rx_mgntframe(priv, NULL, pfrinfo);
#else
				list_add_tail(&(pfrinfo->rx_list), &(priv->rx_mgtlist));
#endif
				reuse = 0;
			}
			else
			{	// data frames
				if (memcmp(bssid, "\x0\x0\x0\x0\x0\x0", MACADDRLEN) &&
						memcmp(BSSID, "\x0\x0\x0\x0\x0\x0", MACADDRLEN) &&
						!memcmp(bssid, BSSID, MACADDRLEN))
				{
#ifdef RTL8190_DIRECT_RX
					rtl8190_rx_dataframe(priv, NULL, pfrinfo);
#else
					list_add_tail(&(pfrinfo->rx_list), &(priv->rx_datalist));
#endif
					reuse = 0;
				}
			}
		}
	}
#endif // CLIENT_MODE
	else
		reuse = 1;

out:

	/* update priv's point */
	*priv_p = priv;

	return reuse;
}


/*
	Check the "to_fr_ds" field:

						FromDS != 0
						ToDS = 0
*/
#if defined(CLIENT_MODE) || defined(CONFIG_RTK_MESH)
static int rtl8190_rx_dispatch_fromDs(struct rtl8190_priv **priv_p, struct rx_frinfo *pfrinfo)
{
	int reuse = 1;
	struct rtl8190_priv *priv = *priv_p;
	unsigned int opmode = OPMODE;
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned int frtype = GetFrameType(pframe);
	unsigned char *da = pfrinfo->da;
	unsigned char *myhwaddr = priv->pmib->dot11OperationEntry.hwaddr;
	unsigned char *bssid = GetAddr2Ptr(pframe);

#ifdef CONFIG_RTK_MESH
	struct stat_info	*pstat = get_stainfo(priv, pfrinfo->sa);

// Gakki 2008.10.13
	if (((opmode & WIFI_AP_STATE) && (GET_MIB(priv)->dot1180211sInfo.mesh_enable))
		&& ((pstat == NULL) || isPossibleNeighbor(pstat)))
		goto out;
#endif	// CONFIG_RTK_MESH

#ifdef CLIENT_MODE	 //(add for Mesh)
	if (frtype == WIFI_MGT_TYPE)
		goto out;

	if ((opmode & (WIFI_STATION_STATE | WIFI_ASOC_STATE)) ==
				  (WIFI_STATION_STATE | WIFI_ASOC_STATE))
	{
		// For Station mode,
		// da should be for me, bssid should be BSSID
		if (IS_BSSID(priv, bssid)) {
			if (IS_MCAST(da) || !memcmp(da, myhwaddr, MACADDRLEN))
			{
#ifdef RTL8190_DIRECT_RX
				rtl8190_rx_dataframe(priv, NULL, pfrinfo);
#else
				list_add_tail(&(pfrinfo->rx_list), &(priv->rx_datalist));
#endif
				reuse = 0;
			}
		}
	}
#ifdef UNIVERSAL_REPEATER
	else if ((opmode & WIFI_AP_STATE) && IS_DRV_OPEN(GET_VXD_PRIV(priv)))
	{
		if (IS_BSSID(GET_VXD_PRIV(priv), bssid)) {
			if (IS_MCAST(da) || !memcmp(da, myhwaddr, MACADDRLEN))
			{
				priv = GET_VXD_PRIV(priv);
#ifdef RTL8190_DIRECT_RX
				rtl8190_rx_dataframe(priv, NULL, pfrinfo);
#else
				list_add_tail(&(pfrinfo->rx_list), &priv->rx_datalist);
#endif
				reuse = 0;
			}
		}
	}
#endif
	else
		reuse = 1;

#endif	// CLIENT_MODE (add for Mesh)
out:

	/* update priv's point */
	*priv_p = priv;

	return reuse;
}
#endif	// defined(CLIENT_MODE) || defined(CONFIG_RTK_MESH)


/*
	Check the "to_fr_ds" field:

						FromDS = 0
						ToDS != 0
*/
#if defined(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI)
__MIPS16
__IRAM_IN_865X_WAPI
#endif
static inline int rtl8190_rx_dispatch_toDs(struct rtl8190_priv **priv_p, 	struct rx_frinfo *pfrinfo
#ifdef MBSSID
				,int vap_idx
#endif
				)
{
	int reuse = 1;
	struct rtl8190_priv *priv = *priv_p;
	unsigned int opmode = OPMODE;
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned int frtype = GetFrameType(pframe);
	unsigned char *bssid = GetAddr1Ptr(pframe);

#ifdef CONFIG_RTK_MESH

// totti ...
    if (GET_MIB(priv)->dot1180211sInfo.mesh_enable == 1) {    // For MBSSID enable and STA connect to Virtual-AP can't use problem.
	struct stat_info	*pstat = get_stainfo(priv, pfrinfo->sa);

		if ((pstat == NULL) || isPossibleNeighbor(pstat))
		goto out;
    }
#endif	// CONFIG_RTK_MESH


	if (frtype == WIFI_MGT_TYPE)
		goto out;

	if (opmode & WIFI_AP_STATE)
	{
#ifdef MBSSID
		if (
#ifdef RTL8192SE
		GET_ROOT(priv)->pmib->miscEntry.vap_enable &&
#endif
			(vap_idx >= 0))
		{
			priv = priv->pvap_priv[vap_idx];
#ifdef RTL8190_DIRECT_RX
			rtl8190_rx_dataframe(priv, NULL, pfrinfo);
#else
			list_add_tail(&(pfrinfo->rx_list), &(priv->rx_datalist));
#endif
			reuse = 0;
		}
		else
#endif
		// For AP mode, the data frame should have bssid==BSSID. (sa state checked laterly)
		if (IS_BSSID(priv, bssid))
		{
			// we only receive mgt frame when we are in SITE_MONITOR or link_loss
			// please consider the case: re-auth and re-assoc
#ifdef RTL8190_DIRECT_RX
			rtl8190_rx_dataframe(priv, NULL, pfrinfo);
#else
			list_add_tail(&(pfrinfo->rx_list), &(priv->rx_datalist));
#endif
			reuse = 0;
		}
	}
#ifdef UNIVERSAL_REPEATER
	else if ((opmode & WIFI_STATION_STATE) && IS_DRV_OPEN(GET_VXD_PRIV(priv))
			&& (GET_VXD_PRIV(priv)->drv_state & DRV_STATE_VXD_AP_STARTED)) {
		if (IS_BSSID(GET_VXD_PRIV(priv), bssid))
		{
			priv = GET_VXD_PRIV(priv);
#ifdef RTL8190_DIRECT_RX
			rtl8190_rx_dataframe(priv, NULL, pfrinfo);
#else
			list_add_tail(&(pfrinfo->rx_list), &priv->rx_datalist);
#endif
			reuse = 0;
		}
	}
#endif
	else
		reuse = 1;

out:

	/* update priv's point */
	*priv_p = priv;

	return reuse;
}


/*
	Check the "to_fr_ds" field:

						FromDS != 0
						ToDS != 0
	NOTE: The functuion duplicate to rtl8190_rx_dispatch_mesh (mesh_rx.c)
*/
#ifdef WDS
static int rtl8190_rx_dispatch_wds(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo)
{
	int reuse = 1;
	unsigned char *pframe = get_pframe(pfrinfo);
	unsigned int frtype = GetFrameType(pframe);
	struct net_device *dev;
	int fragnum;
#ifdef CONFIG_RTL865X
	struct sk_buff	*skb = get_pskb(pfrinfo);
#endif

	if (memcmp(GET_MY_HWADDR, GetAddr1Ptr(pframe), MACADDRLEN)) {
		DEBUG_INFO("Rx a WDS packet but which addr1 is not matched own, drop it!\n");
		goto out;
	}

	if (!priv->pmib->dot11WdsInfo.wdsEnabled) {
		DEBUG_ERR("Rx a WDS packet but WDS is not enabled in local mib, drop it!\n");
		goto out;
	}
	dev = getWdsDevByAddr(priv, GetAddr2Ptr(pframe));
	if (dev==NULL) {
		DEBUG_ERR("Rx a WDS packet but which TA is not valid, drop it!\n");
		goto out;
	}
#ifdef CONFIG_RTL865X
	skb->cb[0] = 1 + getWdsIdxByDev(priv, dev); /* rewrite cb[0] to indicate from which wds interface the pkt is received from */
#endif
	if (!netif_running(dev)) {
		DEBUG_ERR("Rx a WDS packet but which interface is not up, drop it!\n");
		goto out;
	}
	fragnum = GetFragNum(pframe);
	if (GetMFrag(pframe) || fragnum) {
		DEBUG_ERR("Rx a fragment WDS packet, drop it!\n");
		goto out;
	}
	if (frtype != WIFI_DATA_TYPE) {
		DEBUG_ERR("Rx a WDS packet but which type is not DATA, drop it!\n");
		goto out;
	}

#ifdef RTL8190_DIRECT_RX
	rtl8190_rx_dataframe(priv, NULL, pfrinfo);
#else
	list_add_tail(&pfrinfo->rx_list, &priv->rx_datalist);
#endif
	reuse = 0;
	goto out;

out:

	return reuse;
}
#endif	// WDS


/*---------------------------------------------------------------
	return value:
	0:	no reuse, allocate new skb due to the current is queued.
	1:	reuse! due to error pkt or short pkt.
----------------------------------------------------------------*/
__MIPS16
__IRAM_FASTEXTDEV
static int validate_mpdu(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char	*sa, *da, *myhwaddr, *pframe;
	unsigned int	frtype;
	int reuse = 1;
#ifdef CONFIG_RTL865X
	struct sk_buff	*skb = get_pskb(pfrinfo);
#endif
#ifdef MBSSID
	unsigned int opmode = OPMODE;
	int i, vap_idx=-1;
#endif

	pframe = get_pframe(pfrinfo);

#if 0	// already flush cache in rtl8190_rx_isr()
#ifndef RTL8190_CACHABLE_CLUSTER
#ifdef __MIPSEB__
	pframe = (UINT8 *)((unsigned int)pframe | 0x20000000);
#endif
#endif
#endif

#ifdef CONFIG_RTL865X
	skb->cb[0] = 0; /* by default, it's from the main wlan link, not from wds8 */
#endif

#ifdef DRVMAC_LB
	if (priv->pmib->miscEntry.drvmac_lb)
		lb_convert(priv, pframe);
#endif

	pfrinfo->hdr_len =  get_hdrlen(priv, pframe);
	if (pfrinfo->hdr_len == 0)	// unallowed packet type
		return 1;

	frtype = GetFrameType(pframe);
	pfrinfo->to_fr_ds = get_tofr_ds(pframe);
	pfrinfo->da	= da = 	get_da(pframe);
	pfrinfo->sa	= sa =  get_sa(pframe);
	pfrinfo->seq = GetSequence(pframe);

#ifdef CONFIG_RTK_MESH
	pfrinfo->is_11s = 0;

	// WIFI_11S_MESH_ACTION use QoS bit, but it's not a QoS data (8186 compatible)
	if (is_qos_data(pframe) && GetFrameSubType(pframe)!=WIFI_11S_MESH_ACTION) {
#else
	if (is_qos_data(pframe)) {
#endif
		pfrinfo->tid = GetTid(pframe) & 0x07;	// we only process TC of 0~7
	}
	else {
		pfrinfo->tid = 0;
	}

	myhwaddr = priv->pmib->dot11OperationEntry.hwaddr;

	// filter packets that SA is myself
	if (!memcmp(myhwaddr, sa, MACADDRLEN)
#ifdef CONFIG_RTK_MESH
        // for e.g.,  MPP1 - THIS_DEVICE - ROOT ;
        //            THIS_DEVICE use ROOT first, and then ROOT dispatch the packet to MPP1
		&& (priv->pmib->dot1180211sInfo.mesh_enable ? ((pfrinfo->to_fr_ds != 0x03) && GetFrameSubType(pframe) != WIFI_11S_MESH) : TRUE)
#endif
	)
		return 1;

#ifdef MBSSID
#ifdef RTL8192SE
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
#endif
	{
	if (opmode & WIFI_AP_STATE) {
		for (i=0; i<RTL8190_NUM_VWLAN; i++) {
			if (IS_DRV_OPEN(priv->pvap_priv[i])) {
				if (((pfrinfo->to_fr_ds == 0x00) && !memcmp(priv->pvap_priv[i]->pmib->dot11StationConfigEntry.dot11Bssid, GetAddr3Ptr(pframe), MACADDRLEN)) ||
					((pfrinfo->to_fr_ds == 0x02) && !memcmp(priv->pvap_priv[i]->pmib->dot11StationConfigEntry.dot11Bssid, GetAddr1Ptr(pframe), MACADDRLEN))) {
					vap_idx = i;
					break;
				}
			}
		}
	}
	}
#endif

#if 0
	// check power save state
	if ((opmode & WIFI_AP_STATE)
#ifdef UNIVERSAL_REPEATER
		|| ((opmode & WIFI_STATION_STATE) && IS_DRV_OPEN(GET_VXD_PRIV(priv)))
#endif
		)
	{
		bssid = GetAddr1Ptr(pframe);

#ifdef MBSSID
		if (vap_idx >= 0) {
			if (IS_BSSID(priv->pvap_priv[vap_idx], bssid))
				pwr_state(priv->pvap_priv[vap_idx], pfrinfo);
		}
		else
#endif
#ifdef UNIVERSAL_REPEATER
		if (opmode & WIFI_AP_STATE) {
			if (IS_BSSID(priv, bssid))
				pwr_state(priv, pfrinfo);
		}
		else {
			if (IS_BSSID(GET_VXD_PRIV(priv), bssid))
				pwr_state(GET_VXD_PRIV(priv), pfrinfo);
		}
#else
		if (IS_BSSID(priv, bssid))
			pwr_state(priv, pfrinfo);
#endif
	}
#endif

	// if receiving control frames, we just handle PS-Poll only
	if (frtype == WIFI_CTRL_TYPE)
	{
		return rtl8190_rx_procCtrlPkt(priv, pfrinfo
#ifdef MBSSID
									  ,vap_idx
#endif
									  );
	}

	// for ap, we have handled; for station/ad-hoc, we reject null_data frame
	if (((GetFrameSubType(pframe)) == WIFI_DATA_NULL) ||
		(((GetFrameSubType(pframe)) == WIFI_DATA) && pfrinfo->pktlen <= 24)
#ifdef SEMI_QOS
		||((QOS_ENABLE) && ((GetFrameSubType(pframe)) == (WIFI_DATA_NULL | BIT(7))))
#endif
	)
	{
		return rtl8190_rx_procNullPkt(priv, pfrinfo
#ifdef MBSSID
									  ,vap_idx
#endif
									  );
	}

	// enqueue data frames and management frames
	switch (pfrinfo->to_fr_ds)
	{
		case 0x00:	// ToDs=0, FromDs=0
			reuse = rtl8190_rx_dispatch_mgmt_adhoc(&priv, pfrinfo
#ifdef MBSSID
												   ,vap_idx
#endif
												   );
			break;

		case 0x01:	// ToDs=0, FromDs=1
#if defined(CLIENT_MODE) || defined(CONFIG_RTK_MESH)
			reuse = rtl8190_rx_dispatch_fromDs(&priv, pfrinfo);
#endif
			break;

		case 0x02:	// ToDs=1, FromDs=0
			reuse = rtl8190_rx_dispatch_toDs(&priv, pfrinfo
#ifdef MBSSID
											 ,vap_idx
#endif
											 );
			break;

		case 0x03:	// ToDs=1, FromDs=1
#ifdef CONFIG_RTK_MESH
			if( 1 == GET_MIB(priv)->dot1180211sInfo.mesh_enable)
			{
				reuse = rtl8190_rx_dispatch_mesh(priv, pfrinfo);
				break;
			}
#endif
#ifdef WDS
			reuse = rtl8190_rx_dispatch_wds(priv, pfrinfo);
#endif
			break;
	}

	// david, move rx statistics from rtl8190_rx_isr() to here because repeater issue
	rx_sum_up(priv, NULL, pfrinfo->pktlen, GetRetry(pframe));

	return reuse;
}


static void rx_pkt_exception(struct rtl8190_priv *priv, unsigned int cmd)
{
	struct net_device_stats *pnet_stats = &(priv->net_stats);

#if	defined(RTL8190) || defined(RTL8192E)
	if (cmd & _CRC32_)
#elif defined(RTL8192SE)
	if (cmd & RX_CRC32)
#endif
	{
		pnet_stats->rx_crc_errors++;
		pnet_stats->rx_errors++;
		SNMP_MIB_INC(dot11FCSErrorCount, 1);
#ifdef	_11s_TEST_MODE_
		mesh_debug_rx2(priv, cmd);
#endif
	}
#if	defined(RTL8190) || defined(RTL8192E)
	else if (cmd & _ICV_)
 #elif defined(RTL8192SE)
         else if (cmd & RX_ICVError)
 #endif
	{
		pnet_stats->rx_errors++;
		SNMP_MIB_ASSIGN(dot11WEPICVErrorCount, 1);
	}
	else
	{
	}
}

#if !defined(NEW_MAC80211_DRV)
#if defined(SEMI_QOS) && defined(WMM_APSD)
void SendQosNullData(struct rtl8190_priv *priv, unsigned char *da)
{
	struct wifi_mib *pmib;
	unsigned char *hwaddr;
	unsigned char tempQosControl[2];
	DECLARE_TXINSN(txinsn);

	txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	pmib = GET_MIB(priv);

	hwaddr = pmib->dot11OperationEntry.hwaddr;

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
	txinsn.lowest_tx_rate = txinsn.tx_rate;
	txinsn.fixed_rate = 1;
	txinsn.phdr = get_wlanhdr_from_poll(priv);
	txinsn.pframe = NULL;

	if (txinsn.phdr == NULL)
		goto send_qos_null_fail;

	memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

	SetFrameSubType(txinsn.phdr, BIT(7) | WIFI_DATA_NULL);
	SetFrDs(txinsn.phdr);
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	txinsn.hdr_len = WLAN_HDR_A3_QOS_LEN;

	memset(tempQosControl, 0, 2);
	tempQosControl[0] = 0x07;		//set priority to VO
	tempQosControl[0] |= BIT(4);	//set EOSP
	memcpy((void *)GetQosControl((txinsn.phdr)), tempQosControl, 2);

	if ((rtl8190_firetx(priv, &txinsn)) == SUCCESS)
		return;

send_qos_null_fail:

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
}


static void process_APSD_dz_queue(struct rtl8190_priv *priv, struct stat_info *pstat, unsigned short tid)
{
	unsigned int deque_level = 1;		// deque pkts level, VO = 4, VI = 3, BE = 2, BK = 1
	struct sk_buff *pskb = NULL;
	DECLARE_TXINSN(txinsn);

	if ((((tid == 7) || (tid == 6)) && !(pstat->apsd_bitmap & 0x01))
		|| (((tid == 5) || (tid == 4)) && !(pstat->apsd_bitmap & 0x02))
		|| (((tid == 3) || (tid == 0)) && !(pstat->apsd_bitmap & 0x08))
		|| (((tid == 2) || (tid == 1)) && !(pstat->apsd_bitmap & 0x04))) {
		DEBUG_INFO("RcvQosNull legacy ps tid=%d", tid);
		return;
	}

	if (pstat->apsd_pkt_buffering == 0)
		goto sendQosNull;

	if ((pstat->apsd_bitmap & 0x01) && (!isFFempty(pstat->VO_dz_queue->head, pstat->VO_dz_queue->tail)))
		deque_level = 4;
	else if ((pstat->apsd_bitmap & 0x02) && (!isFFempty(pstat->VI_dz_queue->head, pstat->VI_dz_queue->tail)))
		deque_level = 3;
	else if ((pstat->apsd_bitmap & 0x08) && (!isFFempty(pstat->BE_dz_queue->head, pstat->BE_dz_queue->tail)))
		deque_level = 2;
	else if ((!(pstat->apsd_bitmap & 0x04)) || (isFFempty(pstat->BK_dz_queue->head, pstat->BK_dz_queue->tail))) {
		//send QoS Null packet
sendQosNull:
		SendQosNullData(priv, pstat->hwaddr);
		DEBUG_INFO("sendQosNull  tid=%d\n", tid);
		return;
	}

	while(1) {
		if (deque_level == 4) {
			pskb = (struct sk_buff *)deque(priv, &(pstat->VO_dz_queue->head), &(pstat->VO_dz_queue->tail),
					(unsigned int)(pstat->VO_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE);
			if (pskb == NULL) {
				if ((pstat->apsd_bitmap & 0x02) && (!isFFempty(pstat->VI_dz_queue->head, pstat->VI_dz_queue->tail)))
					deque_level--;
				else if ((pstat->apsd_bitmap & 0x08) && (!isFFempty(pstat->BE_dz_queue->head, pstat->BE_dz_queue->tail)))
					deque_level = 2;
				else if ((pstat->apsd_bitmap & 0x04) && (!isFFempty(pstat->BK_dz_queue->head, pstat->BK_dz_queue->tail)))
					deque_level = 1;
				else
					deque_level = 0;
			}
			else {
				DEBUG_INFO("deque VO pkt\n");
			}
		}
		else if (deque_level == 3) {
			pskb = (struct sk_buff *)deque(priv, &(pstat->VI_dz_queue->head), &(pstat->VI_dz_queue->tail),
					(unsigned int)(pstat->VI_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE);
			if (pskb == NULL) {
				if ((pstat->apsd_bitmap & 0x08) && (!isFFempty(pstat->BE_dz_queue->head, pstat->BE_dz_queue->tail)))
					deque_level--;
				else if ((pstat->apsd_bitmap & 0x04) && (!isFFempty(pstat->BK_dz_queue->head, pstat->BK_dz_queue->tail)))
					deque_level = 1;
				else
					deque_level = 0;
			}
			else {
				DEBUG_INFO("deque VI pkt\n");
			}
		}
		else if (deque_level == 2) {
			pskb = (struct sk_buff *)deque(priv, &(pstat->BE_dz_queue->head), &(pstat->BE_dz_queue->tail),
					(unsigned int)(pstat->BE_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE);
			if (pskb == NULL) {
				if ((pstat->apsd_bitmap & 0x04) && (!isFFempty(pstat->BK_dz_queue->head, pstat->BK_dz_queue->tail)))
					deque_level--;
				else
					deque_level = 0;
			}
			else {
				DEBUG_INFO("deque BE pkt\n");
			}
		}
		else if (deque_level == 1) {
			pskb = (struct sk_buff *)deque(priv, &(pstat->BK_dz_queue->head), &(pstat->BK_dz_queue->tail),
					(unsigned int)(pstat->BK_dz_queue->pSkb), NUM_APSD_TXPKT_QUEUE);
			if(pskb)
				DEBUG_INFO("deque BK pkt\n");
		}

		if (pskb) {
			txinsn.q_num   = BE_QUEUE;
			txinsn.fr_type = _SKB_FRAME_TYPE_;
			txinsn.pframe  = pskb;
			txinsn.phdr	   = (UINT8 *)get_wlanllchdr_from_poll(priv);
			pskb->cb[1] = 0;

			if (pskb->len > priv->pmib->dot11OperationEntry.dot11RTSThreshold)
				txinsn.retry = priv->pmib->dot11OperationEntry.dot11LongRetryLimit;
			else
				txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

			if (txinsn.phdr == NULL) {
				DEBUG_ERR("Can't alloc wlan header!\n");
				goto xmit_skb_fail;
			}

			memset((void *)txinsn.phdr, 0, sizeof(struct wlanllc_hdr));

			SetFrDs(txinsn.phdr);
			SetFrameSubType(txinsn.phdr, WIFI_QOS_DATA);
			if (((deque_level == 4) && (!isFFempty(pstat->VO_dz_queue->head, pstat->VO_dz_queue->tail)) && (pstat->apsd_bitmap & 0x01)) ||
				((deque_level >= 3) && (!isFFempty(pstat->VI_dz_queue->head, pstat->VI_dz_queue->tail)) && (pstat->apsd_bitmap & 0x02)) ||
				((deque_level >= 2) && (!isFFempty(pstat->BE_dz_queue->head, pstat->BE_dz_queue->tail)) && (pstat->apsd_bitmap & 0x08)) ||
				((deque_level >= 1) && (!isFFempty(pstat->BK_dz_queue->head, pstat->BK_dz_queue->tail)) && (pstat->apsd_bitmap & 0x04)))
				SetMData(txinsn.phdr);

			if (rtl8190_wlantx(priv, &txinsn) == CONGESTED) {

xmit_skb_fail:
				priv->ext_stats.tx_drops++;
				DEBUG_WARN("TX DROP: Congested!\n");
				if (txinsn.phdr)
					release_wlanllchdr_to_poll(priv, txinsn.phdr);
				if (pskb)
					rtl_kfree_skb(priv, pskb, _SKB_TX_);
			}
		}
		else if (deque_level <= 1) {
			if ((pstat->apsd_pkt_buffering) &&
				(isFFempty(pstat->VO_dz_queue->head, pstat->VO_dz_queue->tail)) &&
				(isFFempty(pstat->VI_dz_queue->head, pstat->VI_dz_queue->tail)) &&
				(isFFempty(pstat->BE_dz_queue->head, pstat->BE_dz_queue->tail)) &&
				(isFFempty(pstat->BK_dz_queue->head, pstat->BK_dz_queue->tail)))
				pstat->apsd_pkt_buffering = 0;
			break;
		}
	}
}


static void process_qos_null(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char *pframe;
	unsigned int aid;
	struct stat_info *pstat = NULL;

	pframe  = get_pframe(pfrinfo);
	aid = GetAid(pframe);
	pstat = get_aidinfo(priv, aid);

	if ((!(OPMODE & WIFI_AP_STATE)) || (pstat == NULL) || (memcmp(pstat->hwaddr, get_sa(pframe), MACADDRLEN))) {
		rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
		return;
	}
	process_APSD_dz_queue(priv, pstat, pfrinfo->tid);

	rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
}
#endif

#if defined(RTL8192SE) && defined(RX_TASKLET)
#if defined(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI)
__IRAM_IN_865X_WAPI
#endif
//__IRAM_FASTEXTDEV //hyking: iram full!!
void rtl8192_rx_isr(unsigned long task_priv)
{
	static struct rtl8190_priv *priv;
	static unsigned long x;
	static unsigned long ioaddr;

	priv = (struct rtl8190_priv *)task_priv;

	ioaddr = priv->pshare->ioaddr;
	SAVE_INT_AND_CLI(x);
	RTL_W32(IMR, RTL_R32(IMR) | (IMR_RXFOVW | IMR_RDU | IMR_ROK));

#ifdef DELAY_REFILL_RX_BUF
	priv->pshare->has_triggered_rx_tasklet = 2; // indicate as ISR in service
#endif

	rtl8190_rx_isr(priv);

	priv->pshare->has_triggered_rx_tasklet = 0;
	RESTORE_INT(x);
}
#endif	//if defined(RTL8192SE) && defined(RX_TASKLET)

#ifdef DELAY_REFILL_RX_BUF
__MIPS16
#if defined(CONFIG_RTL_DYNAMIC_IRAM_MAPPING_FOR_WAPI)
__IRAM_IN_865X_WAPI
#else
__IRAM_FASTEXTDEV //hyking: iram is full...
#endif
int refill_rx_ring(struct rtl8190_priv *priv, struct sk_buff *skb, unsigned char *data)
{
	struct rtl8190_hw *phw=GET_HW(priv);
	struct sk_buff *new_skb;
	extern struct sk_buff *dev_alloc_8190_skb(unsigned char *data, int size);
	if (skb ||
			(priv->pshare->has_triggered_rx_tasklet != 2 && phw->cur_rx_refill != phw->cur_rx)) {
		if (skb)
			new_skb = skb;
		else {
			new_skb = dev_alloc_8190_skb(data, RX_BUF_LEN);
			if (new_skb == NULL) {
				DEBUG_ERR("out of skb struct!\n");
				return 0;
			}
		}

		init_rxdesc(new_skb, phw->cur_rx_refill, priv);
		phw->cur_rx_refill = (phw->cur_rx_refill + 1) % NUM_RX_DESC;
		return 1;
	}
	else
		return 0;
}
#endif

__IRAM_FASTEXTDEV
int legacy_rx_handling(struct rtl8190_priv *priv, struct rtl8190_hw *phw, struct rx_frinfo *pfrinfo,
                                struct sk_buff *pskb, struct rx_desc *pdesc, int *cur_tail, int *preuse, int *cur_cmd)
{
        __DRAM_IN_865X static unsigned int flush_range=0;
        struct sk_buff *new_skb=NULL;
#if	defined(RTL8190) || defined(RTL8192E)
	__DRAM_IN_865X static unsigned char	info0=0, info1=0, info2=0;
#endif
#if defined(RTL8192SE)
	static unsigned int rtl8192SE_ICV, privacy;
	static struct stat_info *pstat;
#endif
        int tail = *cur_tail, cmd = *cur_cmd;

        pskb = (struct sk_buff *)(phw->rx_infoL[tail].pbuf);
	pfrinfo = get_pfrinfo(pskb);
#if defined(RTL8190) || defined(RTL8192E)
	pfrinfo->pktlen = (cmd & _RXFRLEN_MSK_) - _CRCLNG_;
	pfrinfo->driver_info_size = (cmd >> _RXDRVINFOSZ_SHIFT_) & 0xff;
	pfrinfo->rxbuf_shift = (cmd & (_SHIFT0_ | _SHIFT1_)) >> 24;
	pfrinfo->sw_dec = (cmd & _SWDEC_) >> 27;
#elif defined(RTL8192SE)
	pfrinfo->pktlen = (cmd & RX_LengthMask) - _CRCLNG_;
	pfrinfo->driver_info_size = ((cmd >> RX_DrvInfoSizeSHIFT) & RX_DrvInfoSizeMask) * 8;
	pfrinfo->rxbuf_shift = (cmd & (RX_ShiftMask << RX_ShiftSHIFT)) >> RX_ShiftSHIFT;
	pfrinfo->sw_dec = (cmd & RX_SWDec) >> 27;
	pfrinfo->pktlen -= pfrinfo->rxbuf_shift;
//			pskb->data += pfrinfo->rxbuf_shift;
	//92SE A-Cut bug or firmware bug? ... avoid abnormal packets
	//Reference to 92SE card driver
	if ((pfrinfo->pktlen > 0x2000) || (pfrinfo->pktlen < 16))
	        return -1;
		//goto rx_reuse;
#endif

#ifdef RTL8190_FASTEXTDEV_FUNCALL
	{
		unsigned short skbOwner;
		rtl865x_extDev_getSkbProperty(pskb, NULL, &skbOwner, NULL);
		if (skbOwner == RTL865X_DRIVER_OWN)
		{
			rtl865x_extDev_rxRunoutTxPending(pskb, priv->dev);
			/* Runout found : Skip ! */
			return -2;
			//goto rx_exit;
		}
		rtl865x_extDev_updateRxRingSkbOwner(pskb, RTL865X_DRIVER_OWN);
	}
#endif

	flush_range = pfrinfo->rxbuf_shift + pfrinfo->driver_info_size + pfrinfo->pktlen;
	rtl_cache_sync_wback(priv, phw->rx_infoL[tail].paddr, flush_range, PCI_DMA_FROMDEVICE);

	pfrinfo->driver_info = (struct RxFWInfo *)(get_pframe(pfrinfo)
#ifndef RTL8192SE
		+ pfrinfo->rxbuf_shift
#endif
		);
#if	defined(RTL8190) || defined(RTL8192E)
	info0 = *(((unsigned char *)pfrinfo->driver_info) + 1);
	info1 = *(((unsigned char *)pfrinfo->driver_info) + 2);
	info2 = *(((unsigned char *)pfrinfo->driver_info) + 3);

	pfrinfo->faggr    = (info0 & 0x20)? 1:0;
	pfrinfo->paggr    = (info0 & 0x10)? 1:0;
	pfrinfo->rx_rate  = (info1 & 0x80)? (info1):(dot11_rate_table[info1&0x7f]);
	pfrinfo->rx_bw    = (info2 & 0x01)? 1:0;
	pfrinfo->rx_splcp = (info2 & 0x02)? 1:0;
#elif	defined(RTL8192SE)
	//printk("Dword3: %d   ------------->", get_desc(pdesc->Dword3));
	pfrinfo->physt = (get_desc(pdesc->Dword0) & RX_PHYStatus)? 1:0;
	pfrinfo->faggr    = (get_desc(pdesc->Dword1) & RX_FAGGR)? 1:0;
	pfrinfo->paggr    = (get_desc(pdesc->Dword1) & RX_PAGGR)? 1:0;
	pfrinfo->rx_bw    = (get_desc(pdesc->Dword3) & RX_BandWidth)? 1:0;
	pfrinfo->rx_splcp = (get_desc(pdesc->Dword3) & RX_SPLCP)? 1:0;

	if ((get_desc(pdesc->Dword3)&RX_RxMCSMask) < 12)
		pfrinfo->rx_rate = dot11_rate_table[(get_desc(pdesc->Dword3)&RX_RxMCSMask)];
	else
		pfrinfo->rx_rate = 0x80|((get_desc(pdesc->Dword3)&RX_RxMCSMask)-12);
	//printk("RX_RATE: %s%d \n", pfrinfo->rx_rate&0x80? "MCS" : "",
	//		pfrinfo->rx_rate&0x80? pfrinfo->rx_rate&0x7f : pfrinfo->rx_rate/2);
#endif

#ifdef RTL8192SE
	if (!pfrinfo->physt)
#else
	if (pfrinfo->faggr)
#endif
		pfrinfo->rssi = 0;
	else
		translate_rssi_sq(priv, pfrinfo);

#ifndef RTL8190_FASTEXTDEV_FUNCALL

#ifdef CONFIG_RTL8190_PRIV_SKB
	{
#ifdef DELAY_REFILL_RX_BUF
		if (CIRC_CNT_RTK(tail, phw->cur_rx_refill, NUM_RX_DESC) > REFILL_THRESHOLD) {
			DEBUG_WARN("out of skb_buff\n");
			priv->ext_stats.reused_skb++;
			return -1;
			//goto rx_reuse;
		}
#else
		if (skb_free_num== 0 && priv->pshare->skb_queue.qlen == 0) {
			DEBUG_WARN("out of skb_buff\n");
			priv->ext_stats.reused_skb++;
			return -1;
			//goto rx_reuse;
		}
#endif
	}
#else
	// allocate new one in advance
	new_skb = rtl_dev_alloc_skb(priv, RX_BUF_LEN, _SKB_RX_, 0);
	if (new_skb == (struct sk_buff *)NULL)
	{
		DEBUG_WARN("out of skb_buff\n");
		priv->ext_stats.reused_skb++;
		return -1;
		//goto rx_reuse;
	}
#endif
#endif

	/*-----------------------------------------------------
	 validate_mpdu will check if we still can reuse the skb
	------------------------------------------------------*/

#ifdef MP_TEST
	if (OPMODE & WIFI_MP_STATE) {
		skb_reserve(pskb, (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size));
#ifdef B2B_TEST
		mp_validate_rx_packet(priv, pskb->data, pfrinfo->pktlen);
#endif
		*preuse = 1;

		rx_sum_up(priv, NULL, pfrinfo->pktlen, GetRetry(get_pframe(pfrinfo)));
		if (priv->pshare->rf_ft_var.rssi_dump)
			update_sta_rssi(priv, NULL, pfrinfo);

		pskb->data -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
		pskb->tail -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
	}
	else
#endif // MP_TEST
	{
		if (pfrinfo->pktlen < (RX_BUF_LEN-sizeof(struct rx_frinfo)-64)) {
			skb_reserve(pskb, (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size));

#ifdef RTL8192SE
			if (cmd & RX_ICVError) {
				rtl8192SE_ICV = privacy = 0;
				pstat = NULL;
#if defined(WDS) || defined(CONFIG_RTK_MESH)
				if (get_tofr_ds((unsigned char *)get_pframe(pfrinfo)) == 3)
					pstat = get_stainfo(priv, (unsigned char *)GetAddr2Ptr((unsigned char *)get_pframe(pfrinfo)));
				else
#endif
					pstat = get_stainfo(priv, (unsigned char *)get_sa((unsigned char *)get_pframe(pfrinfo)));

				if (!pstat) {
					rtl8192SE_ICV++;
				}
				else {
					if (OPMODE & WIFI_AP_STATE) {
#if defined(WDS) || defined(CONFIG_RTK_MESH)
						if (get_tofr_ds((unsigned char *)get_pframe(pfrinfo)) == 3){
#if defined(CONFIG_RTK_MESH)
							if(priv->pmib->dot1180211sInfo.mesh_enable)
								privacy = (IS_MCAST(GetAddr1Ptr((unsigned char *)get_pframe(pfrinfo)))) ? _NO_PRIVACY_ : priv->pmib->dot11sKeysTable.dot11Privacy;
							else
#endif
							privacy = priv->pmib->dot11WdsInfo.wdsPrivacy;
						}
						else
#endif
							privacy = get_sta_encrypt_algthm(priv, pstat);
					}
#ifdef CLIENT_MODE
					else {
							privacy = get_sta_encrypt_algthm(priv, pstat);
					}
#endif
					if (privacy != _CCMP_PRIVACY_)
						rtl8192SE_ICV++;
				}

				if (rtl8192SE_ICV) {
					rx_pkt_exception(priv, cmd);
					pskb->data -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
					pskb->tail -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
					return -1;
                                        //goto rx_reuse;
				}
			}
#endif

			SNMP_MIB_INC(dot11ReceivedFragmentCount, 1);

			*preuse = validate_mpdu(priv, pfrinfo);

			if (*preuse) {
				pskb->data -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
				pskb->tail -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
			}
		}
	}
	return 0;
}

// The purpose of reassemble is to assemble the frag into a complete one.
static struct rx_frinfo *reassemble(struct rtl8190_priv *priv, struct stat_info *pstat)
{
	struct list_head	*phead, *plist;
	unsigned short		seq=0;
	unsigned char		tofr_ds=0;
	unsigned int		iv, icv, mic, privacy=0, offset;
	struct sk_buff		*pskb;
	struct rx_frinfo	*pfrinfo, *pfirstfrinfo=NULL;
	unsigned char		*pframe=NULL, *pfirstframe=NULL, tail[16]; //4, 12, and 8 for WEP/TKIP/AES
	unsigned long		flags;
	int			i;

	phead = &pstat->frag_list;

	SAVE_INT_AND_CLI(flags);

	// checking  all the seq should be the same, and the frg should monotically increase
	for(i=0, plist=phead->next; plist!=phead; plist=plist->next, i++)
	{
		pfrinfo = list_entry(plist, struct rx_frinfo, mpdu_list);
		pframe = get_pframe(pfrinfo);

		if ((GetFragNum(pframe)) != i)
		{
			DEBUG_ERR("RX DROP: FragNum did not match, FragNum=%d, GetFragNum(pframe)=%d\n",
				i, GetFragNum(pframe));
			goto unchainned_all;
		}

		if (i == 0)
		{
			seq = GetSequence(pframe);
			privacy = GetPrivacy(pframe);
			tofr_ds = pfrinfo->to_fr_ds;
		}
		else
		{
			if (GetSequence(pframe) != seq)
			{
				DEBUG_ERR("RX DROP: Seq is not correct, seq=%d, GetSequence(pframe)=%d\n",
					seq, GetSequence(pframe));
				goto unchainned_all;
			}

			if (GetPrivacy(pframe) != privacy)
			{
				DEBUG_ERR("RX DROP: Privacy is not correct, privacy=%d, GetPrivacy(pframe)=%d\n",
					privacy, GetPrivacy(pframe));
				goto unchainned_all;
			}

			if (pfrinfo->to_fr_ds != tofr_ds)
			{
				DEBUG_ERR("RX DROP: to_fr_ds did not match, tofr_ds=%d, pfrinfo->to_fr_ds=%d\n",
					tofr_ds, pfrinfo->to_fr_ds);
				goto unchainned_all;
			}
		}
	}

	privacy = get_privacy(priv, pstat, &iv, &icv, &mic);

	offset = iv;
	offset += get_hdrlen(priv, pframe);

	// below we are going to re-assemble the whole pkts...
	for(i=0, plist=phead->next; plist!=phead; plist=plist->next, i++)
	{
		pfrinfo = list_entry(plist, struct rx_frinfo, mpdu_list);

		if (pfrinfo->pktlen <= (offset + icv + mic))
		{
			DEBUG_ERR("RX DROP: Frame length bad (%d)\n", pfrinfo->pktlen);
			pfirstfrinfo = NULL;
			goto unchainned_all;
		}

		if (i == 0)
		{
			pfirstfrinfo = pfrinfo;
			pfirstframe  = get_pframe(pfrinfo);
			pfirstframe += pfrinfo->pktlen - (icv + mic);

			if (icv + mic)
			{
				memcpy((void *)tail, (void *)(pfirstframe), (icv + mic));
				pfirstfrinfo->pktlen -= (icv + mic);
			}
			continue;
		}

		// check if too many frags...
		if ((pfirstfrinfo->pktlen += (pfrinfo->pktlen - offset - icv - mic)) >=
			(RX_BUF_LEN - offset - icv - mic - 200))
		{
			DEBUG_ERR("RX DROP: over rx buf size after reassemble...\n");
			pfirstfrinfo = NULL;
			goto unchainned_all;
		}

		// here we should check if all these frags exceeds the buf size
		memcpy(pfirstframe, get_pframe(pfrinfo) + offset, pfrinfo->pktlen - offset - icv - mic);
		pfirstframe += (pfrinfo->pktlen - offset - icv - mic);
	}

	if (icv + mic)
	{
		memcpy((void *)pfirstframe, (void *)tail, (icv + mic));
		pfirstfrinfo->pktlen += (icv + mic);
	}

	// take the first frame out of fragment list
	plist = phead->next;
	list_del_init(plist);

unchainned_all:	// dequeue all the queued-up frag, free skb, and init_list_head again...

	while(!list_empty(phead))
	{
		plist = phead->next;
		pfrinfo = list_entry(plist, struct rx_frinfo, mpdu_list);
		pskb = get_pskb(pfrinfo);
		list_del_init(plist);
		if (pfirstfrinfo == NULL)
			priv->ext_stats.rx_data_drops++;
		rtl_kfree_skb(priv, pskb, _SKB_RX_IRQ_);
	}
	INIT_LIST_HEAD(phead);

	list_del_init(&pstat->defrag_list);
	pstat->frag_count = 0;

	RESTORE_INT(flags);

	return pfirstfrinfo;
}

/*----------------------------------------------------------------------------------------
	So far, only data pkt will be defragmented.
-----------------------------------------------------------------------------------------*/
static struct rx_frinfo *defrag_frame_main(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char *da, *sa;
	struct stat_info *pstat=NULL;
	unsigned int  res, hdr_len, len;
	int status, privacy=0, pos;
	unsigned long flags;
	unsigned char tkipmic[8], rxmic[8];
	unsigned char *pframe;

	pframe = get_pframe(pfrinfo);
	hdr_len = pfrinfo->hdr_len;
	da	= pfrinfo->da;
	sa  = pfrinfo->sa;
	len = pfrinfo->pktlen;

	/*---------first of all check if sa is assocated---------*/
	if (OPMODE & WIFI_AP_STATE) {
#if defined(CONFIG_RTK_MESH) || defined(WDS)
		// for 802.11s case, pstat will not be NULL, because we have check it in validate-mpdu
		if (pfrinfo->to_fr_ds == 3) {
			pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
			goto check_privacy;
		}
		else
#endif
		pstat = get_stainfo(priv, sa);
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE) {
		unsigned char *bssid = GetAddr2Ptr(pframe);
		pstat = get_stainfo(priv, bssid);
	}
	else	// Ad-hoc
		pstat = get_stainfo(priv, sa);
#endif

	if (pstat == NULL)
	{
		status = _RSON_CLS2_;
		priv->ext_stats.rx_data_drops++;
		DEBUG_ERR("RX DROP: class 2 error!\n");
		goto data_defrag_error;
	}
	else if (!(pstat->state & WIFI_ASOC_STATE))
	{
		status = _RSON_CLS3_;
		priv->ext_stats.rx_data_drops++;
		DEBUG_ERR("RX DROP: class 3 error!\n");
		goto data_defrag_error;
	}
	else {}

	/*-------------------get privacy-------------------*/
#if defined(CONFIG_RTK_MESH) || defined(WDS)
check_privacy:
#endif

	if (OPMODE & WIFI_AP_STATE) {
#ifdef CONFIG_RTK_MESH
		//modify by Joule for SECURITY
		if(pfrinfo->is_11s)
			privacy = IS_MCAST(da) ? _NO_PRIVACY_ : get_sta_encrypt_algthm(priv, pstat);
		else
#endif
#ifdef WDS
		if (pfrinfo->to_fr_ds == 3)
			privacy = priv->pmib->dot11WdsInfo.wdsPrivacy;
		else
#endif
			privacy = get_sta_encrypt_algthm(priv, pstat);
	}
#ifdef CLIENT_MODE
	else {
		if (IS_MCAST(da)) {
#if defined(UNIVERSAL_REPEATER) && defined(MBSSID)
			if (IS_VXD_INTERFACE(priv) && !IEEE8021X_FUN &&
				((priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_40_PRIVACY_) ||
				 (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _WEP_104_PRIVACY_)))
				 privacy = priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm;
			else
#endif
			// iv, icv and mic are not be used below. Don't care them!
			privacy = priv->pmib->dot11GroupKeysTable.dot11Privacy;
		}
		else
			privacy = get_sta_encrypt_algthm(priv, pstat);
	}
#endif

	/*-------------------check retry-------------------*/
	pos = GetSequence(pframe) & (TUPLE_WINDOW - 1);
	if (GetTupleCache(pframe) == pstat->tpcache[pfrinfo->tid][pos]) {
		priv->ext_stats.rx_decache++;
		SNMP_MIB_INC(dot11FrameDuplicateCount, 1);
		goto free_skb_in_defrag;
	}
	else
		pstat->tpcache[pfrinfo->tid][pos] = GetTupleCache(pframe);

	/*-------------------------------------------------------*/
	/*-----------insert MPDU-based decrypt below-------------*/
	/*-------------------------------------------------------*/

#ifdef SUPPORT_SNMP_MIB
	if (GetPrivacy(pframe) && privacy == _NO_PRIVACY_)
		SNMP_MIB_INC(dot11WEPUndecryptableCount, 1);

	if (!GetPrivacy(pframe) && privacy != _NO_PRIVACY_)
		SNMP_MIB_INC(dot11WEPExcludedCount, 1);
#endif

	// check whether WEP bit is set in mac header and sw encryption
	if (GetPrivacy(pframe) && UseSwCrypto(priv, pstat, IS_MCAST(GetAddr1Ptr(pframe))))
	{
#if defined(CONFIG_RTL_WAPI_SUPPORT)
		if (privacy==_WAPI_SMS4_)
		{
			/*	Decryption	*/
//			SAVE_INT_AND_CLI(flags);
			res = SecSWSMS4Decryption(priv, pstat, pfrinfo);
//			RESTORE_INT(flags);
			if (res == FAIL)
			{
				priv->ext_stats.rx_data_drops++;
				DEBUG_ERR("RX DROP: WAPI decrpt error!\n");
				goto free_skb_in_defrag;
			}
			pframe = get_pframe(pfrinfo);
		} else
#endif
		if (privacy == _TKIP_PRIVACY_)
		{
			res = tkip_decrypt(priv, pfrinfo, pfrinfo->pktlen);
			if (res == FAIL)
			{
				priv->ext_stats.rx_data_drops++;
				DEBUG_ERR("RX DROP: Tkip decrpt error!\n");
				goto free_skb_in_defrag;
			}
		}
		else if (privacy == _CCMP_PRIVACY_)
		{
			res = aesccmp_decrypt(priv, pfrinfo);
			if (res == FAIL)
			{
				priv->ext_stats.rx_data_drops++;
				DEBUG_ERR("RX DROP: AES decrpt error!\n");
				goto free_skb_in_defrag;
			}
		}
		else if (privacy == _WEP_40_PRIVACY_ || privacy == _WEP_104_PRIVACY_)
		{
			res = wep_decrypt(priv, pfrinfo, pfrinfo->pktlen, privacy, 0);
			if (res == FAIL)
			{
				priv->ext_stats.rx_data_drops++;
				DEBUG_ERR("RX DROP: WEP decrpt error!\n");
				goto free_skb_in_defrag;
			}
		}
		else
		{
			DEBUG_ERR("RX DROP: encrypted packet but no key in sta or wrong enc type!\n");
			goto free_skb_in_defrag;
		}

	}
	/*----------------End of MPDU-based decrypt--------------*/

	if (GetMFrag(pframe))
	{
		if (pstat->frag_count > MAX_FRAG_COUNT)
		{
			priv->ext_stats.rx_data_drops++;
			DEBUG_ERR("RX DROP: station has received too many frags!\n");
			goto free_skb_in_defrag;
		}

		SAVE_INT_AND_CLI(flags);

		if (pstat->frag_count == 0) // the first frag...
			pstat->frag_to = priv->frag_to;

		if (list_empty(&pstat->defrag_list))
			list_add_tail(&pstat->defrag_list, &priv->defrag_list);

		list_add_tail(&pfrinfo->mpdu_list, &pstat->frag_list);
		pstat->frag_count++;

		RESTORE_INT(flags);
		return (struct rx_frinfo *)NULL;
	}
	else
	{
		if(GetFragNum(pframe))
		{
			SAVE_INT_AND_CLI(flags);
			list_add_tail(&pfrinfo->mpdu_list, &pstat->frag_list);
			RESTORE_INT(flags);

			pfrinfo = reassemble(priv, pstat);
			if (pfrinfo == NULL)
				return (struct rx_frinfo *)NULL;
		}
	}

	/*-----discard non-authorized packet before MIC check----*/
	if (OPMODE & WIFI_AP_STATE) {
#if defined(CONFIG_RTK_MESH) || defined(WDS)
		if (pfrinfo->to_fr_ds != 3)
#endif
		if (auth_filter(priv, pstat, pfrinfo) == FAIL) {
			priv->ext_stats.rx_data_drops++;
			DEBUG_ERR("RX DROP: due to auth_filter fails\n");
			goto free_skb_in_defrag;
		}
	}
#ifdef CLIENT_MODE
	else if (OPMODE & WIFI_STATION_STATE) {
		if (auth_filter(priv, pstat, pfrinfo) == FAIL) {
			priv->ext_stats.rx_data_drops++;
			DEBUG_ERR("RX DROP: due to auth_filter fails\n");
			goto free_skb_in_defrag;
		}
	}
#endif

	/*------------------------------------------------------------------------*/
						//insert MSDU-based digest here!
	/*------------------------------------------------------------------------*/
	if (privacy == _TKIP_PRIVACY_)
	{
		pframe = get_pframe(pfrinfo);
		len = pfrinfo->pktlen;

		//truncate Michael...
		memcpy((void *)rxmic, (void *)(pframe + len - 8 - 4), 8); // 8 michael, 4 icv
		SAVE_INT_AND_CLI(flags);
		tkip_rx_mic(priv, pframe, da, sa,
			pfrinfo->tid, pframe + hdr_len + 8,
			len - hdr_len - 8 - 8 - 4, tkipmic, 0); // 8 IV, 8 Mic, 4 ICV
		RESTORE_INT(flags);

		if(memcmp(rxmic, tkipmic, 8))
		{
			priv->ext_stats.rx_data_drops++;
			DEBUG_ERR("RX DROP: MIC error! Indicate to protection mechanism\n");
			if (OPMODE & WIFI_AP_STATE) {
#ifdef RTL_WPA2
				PRINT_INFO("%s: DOT11_Indicate_MIC_Failure %02X:%02X:%02X:%02X:%02X:%02X \n", (char *)__FUNCTION__,
					pstat->hwaddr[0],pstat->hwaddr[1],pstat->hwaddr[2],pstat->hwaddr[3],pstat->hwaddr[4],pstat->hwaddr[5]);
#endif
#ifdef WDS
				if ((pfrinfo->to_fr_ds == 3) &&
						pstat && (pstat->state & WIFI_WDS))
					goto free_skb_in_defrag;
#endif
				DOT11_Indicate_MIC_Failure(priv->dev, pstat);
			}
#ifdef CLIENT_MODE
			else if (OPMODE & WIFI_STATION_STATE)
				DOT11_Indicate_MIC_Failure_Clnt(priv, sa);
#endif
			goto free_skb_in_defrag;
		}
	}

	return pfrinfo;

data_defrag_error:

	if (OPMODE & WIFI_AP_STATE)
		issue_deauth(priv,sa,status);
#ifdef CLIENT_MODE
	else {
		if (pstat == NULL) {
			DEBUG_ERR("rx data with pstat == NULL\n");
		}
		else if (!(pstat->state & WIFI_ASOC_STATE)) {
			DEBUG_ERR("rx data with pstat not associated\n");
		}
	}
#endif

free_skb_in_defrag:

	rtl_kfree_skb(priv, get_pskb(pfrinfo), _SKB_RX_);

	return (struct rx_frinfo *)NULL;
}

static struct rx_frinfo *defrag_frame(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo)
{
	struct sk_buff *pskb = get_pskb(pfrinfo);
	struct rx_frinfo *prx_frinfo = NULL;
	unsigned int encrypt;
	unsigned char *pframe;

	// rx a encrypt packet but encryption is not enabled in local mib, discard it
	pframe = get_pframe(pfrinfo);
	encrypt = GetPrivacy(pframe);

//modify by Joule for SECURITY
// here maybe need do some tune; plus
#ifdef CONFIG_RTK_MESH	/*-------*/
	if (encrypt && (
		(pfrinfo->to_fr_ds==3 && (
#ifdef WDS
		GET_MIB(priv)->dot1180211sInfo.mesh_enable ==0	?
		priv->pmib->dot11WdsInfo.wdsPrivacy==_NO_PRIVACY_ :
#endif
		priv->pmib->dot11sKeysTable.dot11Privacy ==_NO_PRIVACY_	)) ||
		(pfrinfo->to_fr_ds!=3 && priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_NO_PRIVACY_
#if defined(CONFIG_RTL_WAPI_SUPPORT)
			&& priv->pmib->wapiInfo.wapiType==wapiDisable
#endif
		)))
#else	/*-------*/
// origin
#ifdef WDS
	if (encrypt && (
			(pfrinfo->to_fr_ds==3 && priv->pmib->dot11WdsInfo.wdsPrivacy==_NO_PRIVACY_) ||
			(pfrinfo->to_fr_ds!=3 && priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm==_NO_PRIVACY_))
#if defined(CONFIG_RTL_WAPI_SUPPORT)
			&& priv->pmib->wapiInfo.wapiType==wapiDisable
#endif
	)	
#else
	if (encrypt && priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == _NO_PRIVACY_
#if defined(CONFIG_RTL_WAPI_SUPPORT)
			&& priv->pmib->wapiInfo.wapiType==wapiDisable
#endif
	)
#endif
#endif/*-------*/
	{
		priv->ext_stats.rx_data_drops++;
		DEBUG_ERR("RX DROP: Discard a encrypted packet!\n");
		rtl_kfree_skb(priv, pskb, _SKB_RX_);
		return (struct rx_frinfo *)NULL;
	}

#ifdef CONFIG_RTK_MESH
        if (pfrinfo->to_fr_ds==3 && !encrypt && (
#ifdef WDS
		GET_MIB(priv)->dot1180211sInfo.mesh_enable ==0	?
		priv->pmib->dot11WdsInfo.wdsPrivacy!=_NO_PRIVACY_ :
#endif
		(priv->pmib->dot11sKeysTable.dot11Privacy != _NO_PRIVACY_ && !IS_MCAST(pfrinfo->da)))) {
		priv->ext_stats.rx_data_drops++;
		DEBUG_ERR("RX DROP: Discard a un-encrypted WDS/MESH packet!\n");
		rtl_kfree_skb(priv, pskb, _SKB_RX_);
		SNMP_MIB_INC(dot11WEPExcludedCount, 1);
		return (struct rx_frinfo *)NULL;
	}
#else
//origin
#ifdef WDS
	if (pfrinfo->to_fr_ds==3 && !encrypt && priv->pmib->dot11WdsInfo.wdsPrivacy!=_NO_PRIVACY_) {
		priv->ext_stats.rx_data_drops++;
		DEBUG_ERR("RX DROP: Discard a un-encrypted WDS packet!\n");
		rtl_kfree_skb(priv, pskb, _SKB_RX_);
		SNMP_MIB_INC(dot11WEPExcludedCount, 1);
		return (struct rx_frinfo *)NULL;
	}
#endif
#endif
	prx_frinfo = defrag_frame_main(priv, pfrinfo);

	return prx_frinfo;
}

#if defined(CONFIG_RTL_WAPI_MODULE_SUPPORT)
int (*wapiHandleRecvPacket_hook)(struct rx_frinfo *pfrinfo, struct stat_info *pstat)=NULL;
EXPORT_SYMBOL(wapiHandleRecvPacket_hook);
#endif
static int process_datafrme(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo)
{
	unsigned char 	*pframe, da[MACADDRLEN];
	unsigned int  	privacy;
	unsigned int	res;
	struct stat_info *pstat = NULL, *dst_pstat = NULL;
	struct sk_buff	 *pskb  = NULL, *pnewskb = NULL;
	unsigned char	qosControl[2];

	pframe = get_pframe(pfrinfo);

	pskb = get_pskb(pfrinfo);
	//skb_put(pskb, pfrinfo->pktlen);	// pskb->tail will be wrong
	pskb->tail = pskb->data + pfrinfo->pktlen;
	pskb->len = pfrinfo->pktlen;
	pskb->dev = priv->dev;

	if (OPMODE & WIFI_AP_STATE)
	{
		memcpy(da, pfrinfo->da, MACADDRLEN);

#ifdef CONFIG_RTK_MESH
		if (pfrinfo->is_11s)
		{
			pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
			rx_sum_up(NULL, pstat, pfrinfo->pktlen, 0);
			update_sta_rssi(priv, pstat, pfrinfo);
			return process_11s_datafrme(priv,pfrinfo, pstat);
		}
		else
#endif
#ifdef WDS
		if (pfrinfo->to_fr_ds == 3) {
			pstat = get_stainfo(priv, GetAddr2Ptr(pframe));
			pskb->dev = getWdsDevByAddr(priv, GetAddr2Ptr(pframe));
		}
		else
#endif
		pstat = get_stainfo(priv, pfrinfo->sa);

#if defined(CONFIG_RTL_WAPI_SUPPORT)
	#if defined(CONFIG_RTL_WAPI_MODULE_SUPPORT)
	if(wapiHandleRecvPacket_hook != NULL)
	{
		if (wapiHandleRecvPacket_hook(pfrinfo, pstat)==SUCCESS)
		{
			return SUCCESS;
		}
	}
	#else
	if (wapiHandleRecvPacket(pfrinfo, pstat)==SUCCESS)
	{
		return SUCCESS;
	}
	#endif
#endif

		// bcm old 11n chipset iot debug
#if defined(RTL8190) || defined(RTL8192E)
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		if ((pstat) && (pstat == priv->fsync_monitor_pstat) && is_MCS_rate(pfrinfo->rx_rate)) {
			if (((pfrinfo->rx_rate & 0x7f) < priv->pshare->rf_ft_var.mcs_ignore_lower)
				|| ((pfrinfo->rx_rate & 0x7f) > priv->pshare->rf_ft_var.mcs_ignore_upper))
				pstat->rx_rate_bitmap |= BIT(pfrinfo->rx_rate & 0x7f);
		}
#endif

		// log rx statistics...
#ifdef  WDS
		if (pfrinfo->to_fr_ds == 3) {
			privacy = priv->pmib->dot11WdsInfo.wdsPrivacy;
		}
		else
#endif
		{
			privacy = get_sta_encrypt_algthm(priv, pstat);
		}
		rx_sum_up(NULL, pstat, pfrinfo->pktlen, 0);
		update_sta_rssi(priv, pstat, pfrinfo);

#ifdef SUPPORT_SNMP_MIB
		if (IS_MCAST(da))
			SNMP_MIB_INC(dot11MulticastReceivedFrameCount, 1);
#endif

#if defined(SEMI_QOS) && defined(WMM_APSD)
		if((QOS_ENABLE) && (APSD_ENABLE) && (pstat->QosEnabled) && (pstat->apsd_bitmap & 0x0f) &&
			((pstat->state & (WIFI_ASOC_STATE|WIFI_SLEEP_STATE)) == (WIFI_ASOC_STATE|WIFI_SLEEP_STATE)) &&
			(GetFrameSubType(get_pframe(pfrinfo)) == (WIFI_QOS_DATA))) {
			process_APSD_dz_queue(priv, pstat, pfrinfo->tid);
		}
#endif

		// Process A-MSDU
		if (is_qos_data(pframe)) {
			memcpy(qosControl, GetQosControl(pframe), 2);
			if (qosControl[0] & BIT(7))	// A-MSDU present
			{
				if (!pstat->is_rtl8190_sta && !pstat->is_marvell_sta)
					pstat->is_marvell_sta = 1;
				process_amsdu(priv, pstat, pfrinfo);
				return SUCCESS;
			}
		}

		// AP always receive unicast frame only
#ifdef WDS
		if (pfrinfo->to_fr_ds!=3 && IS_MCAST(da))
#else
		if (IS_MCAST(da))
#endif
		{
#ifdef DRVMAC_LB
			if (priv->pmib->miscEntry.drvmac_lb) {
				priv->ext_stats.rx_data_drops++;
				DEBUG_ERR("RX DROP: drop br/ml packet in loop-back mode!\n");
				return FAIL;
			}
#endif

			// This is a legal frame, convert it to skb
			res = skb_p80211_to_ether(priv->dev, privacy, pfrinfo);
			if (res == FAIL) {
				priv->ext_stats.rx_data_drops++;
				DEBUG_ERR("RX DROP: skb_p80211_to_ether fail!\n");
				return FAIL;
			}

#ifdef __KERNEL__
			// if we are STP aware, don't broadcast received BPDU
			if (!(priv->dev->br_port &&
				 priv->dev->br_port->br->stp_enabled &&
				 !memcmp(pskb->data, "\x01\x80\xc2\x00\x00\x00", 6)))
#endif
			{
				if (!priv->pmib->dot11OperationEntry.block_relay)
				{
#ifdef RTL8190_FASTEXTDEV_FUNCALL
					pnewskb = skb_copy(pskb, GFP_ATOMIC);
					rtl865x_extDev_pktFromProtocolStack(pnewskb);
#else
					pnewskb = skb_clone(pskb, GFP_ATOMIC);
#endif
					if (pnewskb) {
#ifdef GBWC
						if (GBWC_forward_check(priv, pnewskb, pstat)) {
							// packet is queued, nothing to do
						}
						else
#endif
						if (rtl8190_start_xmit(pnewskb, priv->dev))
							rtl_kfree_skb(priv, pnewskb, _SKB_TX_);
					}
				}
			}

			rtl_netif_rx(priv, pskb, pstat);
		}
		else
		{
			// unicast.. the status of sa has been checked in defrag_frame.
			// however, we should check if the da is in the WDS to see if we should
			res = skb_p80211_to_ether(pskb->dev, privacy, pfrinfo);
			if (res == FAIL) {
				priv->ext_stats.rx_data_drops++;
				DEBUG_ERR("RX DROP: skb_p80211_to_ether fail!\n");
				return FAIL;
			}
		
			dst_pstat = get_stainfo(priv, da);

#ifdef WDS
			if ((pfrinfo->to_fr_ds==3) ||
				(dst_pstat == NULL) || !(dst_pstat->state & WIFI_ASOC_STATE))
#else
			if ((dst_pstat == NULL) || (!(dst_pstat->state & WIFI_ASOC_STATE)))
#endif
			{

			if (priv->pmib->dot11OperationEntry.guest_access
#ifdef CONFIG_RTL8186_KB
					||(pstat && pstat->ieee8021x_ctrlport == DOT11_PortStatus_Guest)
#endif
				) {
				if (
#ifdef LINUX_2_6_22_
					*(unsigned short *)(pskb->mac_header + MACADDRLEN*2) != __constant_htons(0x888e) &&
#else
					*(unsigned short *)(pskb->mac.raw + MACADDRLEN*2) != __constant_htons(0x888e) &&
#endif
					priv->dev->br_port &&
#ifdef __LINUX_2_6__
					memcmp(da, priv->dev->br_port->br->dev->dev_addr, MACADDRLEN)
#else
					memcmp(da, priv->dev->br_port->br->dev.dev_addr, MACADDRLEN)
#endif
					) {
					priv->ext_stats.rx_data_drops++;
					DEBUG_ERR("RX DROP: guest access fail!\n");
					return FAIL;
				}
#ifndef CONFIG_X86
				pskb->__unused = 0xe5;
#endif

#ifdef CONFIG_RTL8186_KB
				if (priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm == 0)
				{
					/* hotel style guest access */
					if (!rtl8190_guestmac_valid(priv, pskb->mac.raw+MACADDRLEN))
					{
#ifndef CONFIG_X86
						pskb->__unused = 0xd3;
#endif
					}
				}
				else
				{
					/* wpa/wp2 guest access */
					/* just let __unused flag be 0xe5 */
				}
#endif

	//			printk("guest packet, addr: %0x2:%02x:%02x:%02x:%02x:%02x\n",da[0],da[1],da[2],da[3],da[4],da[5]);
			}
#ifndef CONFIG_X86
			else
				pskb->__unused = 0;
#endif

				pskb->protocol = eth_type_trans(pskb, priv->dev);

#ifdef EAPOLSTART_BY_QUEUE
#ifdef LINUX_2_6_22_
				if (*(unsigned short *)(pskb->mac_header + MACADDRLEN*2) == __constant_htons(0x888e))
#else
				if (*(unsigned short *)(pskb->mac.raw + MACADDRLEN*2) == __constant_htons(0x888e))
#endif
				{
					unsigned char		szEAPOL[] = {0x01, 0x01, 0x00, 0x00};
					DOT11_EAPOL_START	Eapol_Start;

					if (!memcmp(pskb->data, szEAPOL, sizeof(szEAPOL)))
					{
						Eapol_Start.EventId = DOT11_EVENT_EAPOLSTART;
						Eapol_Start.IsMoreEvent = FALSE;
#ifdef LINUX_2_6_22_
						memcpy(&Eapol_Start.MACAddr, pskb->mac_header + MACADDRLEN, WLAN_ETHHDR_LEN);
#else
						memcpy(&Eapol_Start.MACAddr, pskb->mac.raw + MACADDRLEN, WLAN_ETHHDR_LEN);
#endif
						DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)&Eapol_Start, sizeof(DOT11_EAPOL_START));
#ifdef LINUX_2_6_22_
						event_indicate(priv, pskb->mac_header + MACADDRLEN, 4);
#else
						event_indicate(priv, pskb->mac.raw + MACADDRLEN, 4);
#endif
						return FAIL;	// let dsr free this skb
					}
				}
#endif

#if defined(EAP_BY_QUEUE) || defined(INCLUDE_WPA_PSK)
#ifdef WDS
#ifdef LINUX_2_6_22_
				if ((pfrinfo->to_fr_ds != 3) && (*(unsigned short *)(pskb->mac_header + MACADDRLEN*2) == __constant_htons(0x888e)))
#else
				if ((pfrinfo->to_fr_ds != 3) && (*(unsigned short *)(pskb->mac.raw + MACADDRLEN*2) == __constant_htons(0x888e)))
#endif
#else
#ifdef LINUX_2_6_22_
				if (*(unsigned short *)(pskb->mac_header + MACADDRLEN*2) == __constant_htons(0x888e))
#else
				if (*(unsigned short *)(pskb->mac.raw + MACADDRLEN*2) == __constant_htons(0x888e))
#endif
#endif
				{
					unsigned short		pkt_len;

					pkt_len = WLAN_ETHHDR_LEN + pskb->len;
					priv->Eap_packet->EventId = DOT11_EVENT_EAP_PACKET;
					priv->Eap_packet->IsMoreEvent = FALSE;
					memcpy(&(priv->Eap_packet->packet_len), &pkt_len, sizeof(unsigned short));
#ifdef LINUX_2_6_22_
					memcpy(&(priv->Eap_packet->packet[0]), pskb->mac_header, WLAN_ETHHDR_LEN);
#else
					memcpy(&(priv->Eap_packet->packet[0]), pskb->mac.raw, WLAN_ETHHDR_LEN);
#endif
					memcpy(&(priv->Eap_packet->packet[WLAN_ETHHDR_LEN]), pskb->data, pskb->len);
#ifdef EAP_BY_QUEUE
					DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)priv->Eap_packet, sizeof(DOT11_EAP_PACKET));
					event_indicate(priv, NULL, -1);
#endif
#ifdef INCLUDE_WPA_PSK
					psk_indicate_evt(priv, DOT11_EVENT_EAP_PACKET, (unsigned char*)&(priv->Eap_packet->packet[6]),
						(unsigned char*)priv->Eap_packet->packet, WLAN_ETHHDR_LEN+pskb->len);
#endif
					return FAIL;	// let dsr free this skb
				}
#endif

				skb_push(pskb, WLAN_ETHHDR_LEN);	// push back due to be pulled by eth_type_trans()

				if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
					priv->pmib->reorderCtrlEntry.ReorderCtrlEnable) {
					*(unsigned int *)&(pfrinfo->pskb->cb[4]) = 0;
					if (reorder_ctrl_check(priv, pstat, pfrinfo))
						rtl_netif_rx(priv, pfrinfo->pskb, pstat);
				}
				else
					rtl_netif_rx(priv, pskb, pstat);
			}
			else
			{
				if (priv->pmib->dot11OperationEntry.block_relay == 1) {
					priv->ext_stats.rx_data_drops++;
					DEBUG_ERR("RX DROP: Relay unicast packet is blocked!\n");
#ifdef RX_SHORTCUT
					if (dst_pstat->rx_payload_offset > 0) // shortcut data saved, clear it
						dst_pstat->rx_payload_offset = 0;
#endif
					return FAIL;
				}
				else if (priv->pmib->dot11OperationEntry.block_relay == 2) {
					DEBUG_INFO("Relay unicast packet is blocked! Indicate to bridge.\n");
					rtl_netif_rx(priv, pskb, pstat);
				}
				else {
#ifdef ENABLE_RTL_SKB_STATS
					rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif
#ifdef GBWC
					if (GBWC_forward_check(priv, pfrinfo->pskb, pstat)) {
						// packet is queued, nothing to do
					}
					else
#endif
					if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
						priv->pmib->reorderCtrlEntry.ReorderCtrlEnable) {
						*(unsigned int *)&(pfrinfo->pskb->cb[4]) = (unsigned int)dst_pstat;	// backup pstat pointer
						if (reorder_ctrl_check(priv, pstat, pfrinfo)) {
//Joule 2009.03.10
#ifdef CONFIG_RTK_MESH
							if (rtl8190_start_xmit(pskb, isMeshPoint(dst_pstat) ? priv->mesh_dev: priv->dev))
#else
							if (rtl8190_start_xmit(pskb, priv->dev))
#endif
								rtl_kfree_skb(priv, pskb, _SKB_RX_);
						}
					}
					else {
#ifdef CONFIG_RTK_MESH
// mantis bug 0000081	2008.07.16
					if (rtl8190_start_xmit(pskb, isMeshPoint(dst_pstat) ? priv->mesh_dev: priv->dev))
#else
					if (rtl8190_start_xmit(pskb, priv->dev))
#endif
							rtl_kfree_skb(priv, pskb, _SKB_RX_);
					}
				}
			}
		}
	}
#ifdef CLIENT_MODE
	else if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE))
	{
		// I am station, and just report every frame I received to protocol statck
		if (OPMODE & WIFI_STATION_STATE)
			pstat = get_stainfo(priv, BSSID);
		else	// Ad-hoc
			pstat = get_stainfo(priv, pfrinfo->sa);

		// bcm old 11n chipset iot debug
#if defined(RTL8190) || defined(RTL8192E)
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
#endif
		if ((pstat) && (pstat == priv->fsync_monitor_pstat) && is_MCS_rate(pfrinfo->rx_rate)) {
			if (((pfrinfo->rx_rate & 0x7f) < priv->pshare->rf_ft_var.mcs_ignore_lower)
				|| ((pfrinfo->rx_rate & 0x7f) > priv->pshare->rf_ft_var.mcs_ignore_upper))
				pstat->rx_rate_bitmap |= BIT(pfrinfo->rx_rate & 0x7f);
		}
#endif

		if (IS_MCAST(pfrinfo->da)) {
			// iv, icv and mic are not be used below. Don't care them!
			privacy = get_mcast_encrypt_algthm(priv);
		}
		else
			privacy = get_sta_encrypt_algthm(priv, pstat);

		rx_sum_up(NULL, pstat, pfrinfo->pktlen, 0);
		update_sta_rssi(priv, pstat, pfrinfo);
		priv->rxDataNumInPeriod++;
		if (IS_MCAST(pfrinfo->da))
			priv->rxMlcstDataNumInPeriod++;

#ifdef SUPPORT_SNMP_MIB
		if (IS_MCAST(pfrinfo->da))
			SNMP_MIB_INC(dot11MulticastReceivedFrameCount, 1);
#endif

		// Process A-MSDU
		if (is_qos_data(pframe)) {
			memcpy(qosControl, GetQosControl(pframe), 2);
			if (qosControl[0] & BIT(7))	// A-MSDU present
			{
				process_amsdu(priv, pstat, pfrinfo);
				return SUCCESS;
			}
		}

		res = skb_p80211_to_ether(priv->dev, privacy, pfrinfo);
		if (res == FAIL) {
			priv->ext_stats.rx_data_drops++;
			DEBUG_ERR("RX DROP: skb_p80211_to_ether fail!\n");
			return FAIL;
		}

#ifdef A4_TUNNEL
		if (priv->pmib->miscEntry.a4tnl_enable) {
			if (pstat == A4_tunnel_lookup(priv, &(pskb->data[6]))) {
				DEBUG_ERR("RX DROP: tunnel frame comes from me!\n");
				return FAIL;
			}
		}
#endif

#ifdef RTK_BR_EXT
		if(nat25_handle_frame(priv, pskb) == -1) {
			priv->ext_stats.rx_data_drops++;
			DEBUG_ERR("RX DROP: nat25_handle_frame fail!\n");
			return FAIL;
		}
#endif

		pskb->protocol = eth_type_trans(pskb, priv->dev);

#if defined(EAP_BY_QUEUE) || defined(INCLUDE_WPA_PSK)
#ifdef LINUX_2_6_22_
		if (*(unsigned short *)(pskb->mac_header + MACADDRLEN*2) == __constant_htons(0x888e))
#else
		if (*(unsigned short *)(pskb->mac.raw + MACADDRLEN*2) == __constant_htons(0x888e))
#endif
		{
			unsigned short		pkt_len;

			pkt_len = WLAN_ETHHDR_LEN + pskb->len;
			priv->Eap_packet->EventId = DOT11_EVENT_EAP_PACKET;
			priv->Eap_packet->IsMoreEvent = FALSE;
			memcpy(&(priv->Eap_packet->packet_len), &pkt_len, sizeof(unsigned short));
#ifdef LINUX_2_6_22_
			memcpy(&(priv->Eap_packet->packet[0]), pskb->mac_header, WLAN_ETHHDR_LEN);
#else
			memcpy(&(priv->Eap_packet->packet[0]), pskb->mac.raw, WLAN_ETHHDR_LEN);
#endif
			memcpy(&(priv->Eap_packet->packet[WLAN_ETHHDR_LEN]), pskb->data, pskb->len);
#ifdef EAP_BY_QUEUE
			DOT11_EnQueue((unsigned long)priv, priv->pevent_queue, (unsigned char*)priv->Eap_packet, sizeof(DOT11_EAP_PACKET));
			event_indicate(priv, NULL, -1);
#endif
#ifdef INCLUDE_WPA_PSK
			psk_indicate_evt(priv, DOT11_EVENT_EAP_PACKET, (unsigned char*)&(priv->Eap_packet->packet[6]),
				(unsigned char*)priv->Eap_packet->packet, WLAN_ETHHDR_LEN+pskb->len);
#endif
			return FAIL;	// let dsr free this skb
		}
#endif

		skb_push(pskb, WLAN_ETHHDR_LEN);	// push back due to be pulled by eth_type_trans()
		if ((priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) &&
			priv->pmib->reorderCtrlEntry.ReorderCtrlEnable) {
			*(unsigned int *)&(pfrinfo->pskb->cb[4]) = 0;
			if (reorder_ctrl_check(priv, pstat, pfrinfo))
				rtl_netif_rx(priv, pskb, pstat);
		}
		else
			rtl_netif_rx(priv, pskb, pstat);
	}
#endif // CLIENT_MODE
	else
	{
		priv->ext_stats.rx_data_drops++;
		DEBUG_ERR("RX DROP: Non supported mode in process_datafrme\n");
		return FAIL;
	}

	return SUCCESS;
}

__MIPS16
__IRAM_FASTEXTDEV
#ifndef CONFIG_RTK_MESH
static
#endif
 void rtl8190_rx_dataframe(struct rtl8190_priv *priv, struct list_head *list,
				struct rx_frinfo *inputPfrinfo)
{
	struct rx_frinfo *pfrinfo = NULL;
	unsigned char *pframe;

	/* ============== Do releted process for Packet RX ============== */
	// for SW LED
	priv->pshare->LED_rx_cnt++;

	// for Rx dynamic tasklet
	priv->pshare->rxInt_data_delta++;

	/* Get RX frame info */
	if (list) {
		/* Indicate the frame information can be gotten from "list" */
		pfrinfo = list_entry(list, struct rx_frinfo, rx_list);
	}
	else {
		/* Indicate the frame information is stored in "inputPfrinfo " */
		pfrinfo = inputPfrinfo;
	}

	if (pfrinfo == NULL)
		goto out;

	pframe = get_pframe(pfrinfo);

#ifdef SEMI_QOS
	if (is_qos_data(pframe)) {
		if (
#if defined(RTL8190) || defined(RTL8192E)
			(!priv->pmib->dot11OperationEntry.wifi_specific) &&
#endif
			(OPMODE & WIFI_AP_STATE) && (QOS_ENABLE)) {
			if ((pfrinfo->tid == 7) || (pfrinfo->tid == 6))
				priv->pshare->phw->VO_pkt_count++;
			else if ((pfrinfo->tid == 5) || (pfrinfo->tid == 4))
				priv->pshare->phw->VI_pkt_count++;
			else if ((pfrinfo->tid == 2) || (pfrinfo->tid == 1))
				priv->pshare->phw->BK_pkt_count++;
		}
	}
#endif

	// check power save state
	if (OPMODE & WIFI_AP_STATE) {
		if (get_stainfo(priv, GetAddr2Ptr(pframe)) != NULL) {
			if (IS_BSSID(priv, GetAddr1Ptr(pframe))) {
				struct stat_info *pstat = get_stainfo(priv, pfrinfo->sa);
				if (pstat && (pstat->state & WIFI_ASOC_STATE) &&
						(GetPwrMgt(pframe) != ((pstat->state & WIFI_SLEEP_STATE ? 1 : 0))))
				  pwr_state(priv, pfrinfo);
			}
		}
	}

	/* ============== Start to process RX dataframe ============== */
#if defined(SEMI_QOS) && defined(WMM_APSD)
	if((QOS_ENABLE) && (APSD_ENABLE) && (GetFrameSubType(get_pframe(pfrinfo)) == (BIT(7)|WIFI_DATA_NULL))) {
		process_qos_null(priv, pfrinfo);
		goto out;
	}
#endif

#ifdef RX_SHORTCUT
	if (!priv->pmib->dot11OperationEntry.disable_rxsc &&
		!IS_MCAST(pfrinfo->da)
#if defined(SEMI_QOS) && defined(WMM_APSD)
		&& (!((APSD_ENABLE) && (GetFrameSubType(get_pframe(pfrinfo)) == (WIFI_QOS_DATA)) && (GetPwrMgt(get_pframe(pfrinfo)))))
#endif
		) {
		if (rx_shortcut(priv, pfrinfo) >= 0)
			goto out;
	}
#endif

	pfrinfo = defrag_frame(priv, pfrinfo);

	if (pfrinfo == NULL)
		goto out;

	if (process_datafrme(priv, pfrinfo) == FAIL) {
		rtl_kfree_skb(priv, pfrinfo->pskb, _SKB_RX_);
	}

out:
	return;
}
#else   //ifdef NEW_MAC80211_DRV
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

__IRAM_FASTEXTDEV
int mac80211_rx_handling(struct mac80211_shared_priv *priv, struct rtl8190_hw *phw, struct rx_frinfo *pfrinfo, 
                        struct rx_desc *rx_desc, struct sk_buff *pskb, int *cur_tail, int *cur_cmd)
{
	struct ieee80211_sta *sta = NULL;
	struct ieee80211_hdr *hdr;
        struct ieee80211_rx_status rx_status;
        unsigned int	flush_range=0, tail = *cur_tail;
        int	retval;
        struct sk_buff *new_skb=NULL;

	pskb = (struct sk_buff *)(phw->rx_infoL[tail].pbuf);
	//collect pfrinfo from rx_desc
	pfrinfo = get_pfrinfo(pskb);
	retval = rtl80211_rxprocdesc(pskb, pfrinfo, rx_desc, priv);
	if (retval == -EINPROGRESS) {
	        return -1;
		//goto rx_reuse;
	}
	
	//fill rx_status for mac80211
	rtl80211_rx_status(pfrinfo, &rx_status, rx_desc, priv);

	flush_range = pfrinfo->rxbuf_shift + pfrinfo->driver_info_size + pfrinfo->pktlen;
	//flush_range = MAX_RX_BUF_LEN;
	pci_dma_sync_single_for_cpu(priv->pshare_hw->pdev, 
		phw->rx_infoL[tail].paddr, flush_range, PCI_DMA_FROMDEVICE);
				
	new_skb = rtl_dev_alloc_skb(priv, MAX_RX_BUF_LEN, _SKB_RX_, 0);
	if (new_skb == (struct sk_buff *)NULL)
	{
		DEBUG_WARN("out of skb_buff\n");
		priv->ext_stats.reused_skb++;
		return -1;
		//goto rx_reuse;
	}
	
	if (pfrinfo->pktlen < (MAX_RX_BUF_LEN-sizeof(struct rx_frinfo)-64)) {
		skb_reserve(pskb, (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size));
		if (*cur_cmd & RX_ICVError) {
			dev_kfree_skb_any(new_skb);
			pskb->data -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
			pskb->tail -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
			//goto rx_reuse;
			return -1;   //-1 measn reuse
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
	return 0;
}
#endif  //ifdnef NEW_MAC80211_DRV

__MIPS16
__IRAM_FASTEXTDEV
void rtl8190_rx_isr(struct RTL_PRIV *priv)
{
// reduce stack size -----------------------
#if 0
	struct rx_desc		*pdesc, *prxdesc;
	struct rtl8190_hw	*phw;
	struct sk_buff		*pskb, *new_skb=NULL;
	struct rx_frinfo	*pfrinfo=NULL;
	unsigned int	tail;
	unsigned int	cmd, reuse;
	unsigned char	info0, info1, info2;
	unsigned int	flush_range;
#endif

	__DRAM_IN_865X static struct rx_desc *pdesc=NULL, *prxdesc=NULL;
	__DRAM_IN_865X static struct rtl8190_hw *phw=NULL;
	static struct sk_buff *pskb=NULL, *new_skb=NULL;
	static struct rx_frinfo	*pfrinfo=NULL;
	__DRAM_IN_865X static unsigned int	tail=0;
	__DRAM_IN_865X static unsigned int	cmd=0, reuse=0;
//---------------------------------------------

#ifndef NEW_MAC80211_DRV
#ifdef CONFIG_RTL8190_PRIV_SKB
	extern int skb_free_num;
#endif
#endif  //ifndef NEW_MAC80211_DRV

	phw = GET_HW(priv);
	tail = phw->cur_rx;

#if defined(RTL8190_CACHABLE_DESC) && !defined(NEW_MAC80211_DRV)
	prxdesc = (struct rx_desc *)(phw->rx_descL);
	rtl_cache_sync_wback(priv, (unsigned int)prxdesc, sizeof(struct rx_desc), PCI_DMA_FROMDEVICE);
#else

#ifdef __MIPSEB__
	prxdesc = (struct rx_desc *)((unsigned int)(phw->rx_descL) | 0x20000000);
#else
	prxdesc = (struct rx_desc *)(phw->rx_descL);
#endif

#endif	/* RTL8190_CACHABLE_DESC */

	while (1)
	{
#if defined(DELAY_REFILL_RX_BUF) && !defined(NEW_MAC80211_DRV)
		int refill = 1;
		if (((tail+1) % NUM_RX_DESC) == phw->cur_rx_refill) {
			break;
		}
#endif

		pdesc = prxdesc + tail;

#if defined(RTL8190) || defined(RTL8192E)
		cmd = get_desc(pdesc->cmd);
		reuse = 1;

		if (cmd & _OWN_)
			break;

		if (cmd & (_CRC32_ | _ICV_))
#elif defined(RTL8192SE)
		cmd = get_desc(pdesc->Dword0);
		reuse = 1;

		if (cmd & RX_OWN)
			break;

		if (cmd & RX_CRC32)
#endif
		{
#ifndef NEW_MAC80211_DRV
			rx_pkt_exception(priv, cmd);

#ifdef RTL8190_FASTEXTDEV_FUNCALL
			if (0)
			{
				rtl865x_extDev_rxRunoutTxPending((struct sk_buff *)(phw->rx_infoL[tail].pbuf), priv->dev);
			}
#endif
#endif  //ifndef NEW_MAC80211_DRV
			goto rx_reuse;
		}
#if	defined(RTL8190) || defined(RTL8192E)
		else if ((cmd & (_FS_ | _LS_)) != (_FS_ | _LS_))
#elif defined(RTL8192SE)
		else if ((cmd & (RX_FirstSeg | RX_LastSeg)) != (RX_FirstSeg | RX_LastSeg))
#endif
		{
			// h/w use more than 1 rx descriptor to receive a packet
			// that means this packet is too large
			// drop such kind of packet
			goto rx_reuse;
		}
		else if (!IS_DRV_OPEN(priv)) {
			goto rx_reuse;
		}
		else
		{
		      int rc=0;
#ifdef NEW_MAC80211_DRV
                        rc = mac80211_rx_handling(priv, phw, pfrinfo, pdesc, pskb, &tail, &cmd);
                        if( rc )
                                goto rx_reuse;
                        else
                                goto rx_done;
#else
                        rc = legacy_rx_handling(priv, phw, pfrinfo, pskb, pdesc, &tail, &reuse, &cmd);
                        if( rc == -1 )
                                goto rx_reuse;
#ifdef RTL8190_FASTEXTDEV_FUNCALL
                        if ( rc == -2 )
                                goto rx_exit;
#endif
#endif
		}

#ifndef NEW_MAC80211_DRV
		if (!reuse)
		{
#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
			if (IS_PCIBIOS_TYPE)
				pci_unmap_single(priv->pshare->pdev, phw->rx_infoL[tail].paddr, (RX_BUF_LEN - sizeof(struct rx_frinfo)), PCI_DMA_FROMDEVICE);
#endif

#ifdef RTL8190_FASTEXTDEV_FUNCALL
			/* Do nothing */
#else
#if 0
			// allocate new one
			pskb = rtl_dev_alloc_skb(priv, RX_BUF_LEN, _SKB_RX_);

			if (pskb == (struct sk_buff *)NULL)
			{
				DEBUG_WARN("out of skb_buff\n");
				list_del(&pfrinfo->rx_list);
				pskb = (struct sk_buff *)(phw->rx_infoL[tail].pbuf);
				pskb->data -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
				pskb->tail -= (pfrinfo->rxbuf_shift + pfrinfo->driver_info_size);
				goto rx_reuse;
			}
#endif

#ifdef CONFIG_RTL8190_PRIV_SKB
#ifdef DELAY_REFILL_RX_BUF
			if (skb_free_num == 0 && priv->pshare->skb_queue.qlen == 0)  {
				refill = 0;
				goto rx_done;
			}
#endif
			new_skb = rtl_dev_alloc_skb(priv, RX_BUF_LEN, _SKB_RX_, 0);
			ASSERT(new_skb);
#endif
			pskb = new_skb;

#ifdef CONFIG_RTK_MESH
			if( pskb == NULL)
				goto rx_reuse;
#endif
#ifdef DELAY_REFILL_RX_BUF
			init_rxdesc(pskb, phw->cur_rx_refill, priv);
#else
			init_rxdesc(pskb, tail, priv);
#endif
#endif	/* RTL8190_FASTEXTDEV_FUNCALL */

			goto rx_done;
		}
#if !defined(RTL8190_FASTEXTDEV_FUNCALL) && !defined(CONFIG_RTL8190_PRIV_SKB)
		else
			rtl_kfree_skb(priv, new_skb, _SKB_RX_);
#endif
#endif  //ifndef NEW_MAC80211_DRV

#ifndef RTL8190_FASTEXTDEV_FUNCALL
rx_reuse:
#endif

#ifndef NEW_MAC80211_DRV
#ifdef RTL8190_FASTEXTDEV_FUNCALL
		rtl865x_extDev_updateRxRingSkbOwner(phw->rx_infoL[tail].pbuf, RTL865X_RXRING_OWN);
#endif

#if	defined(RTL8190) || defined(RTL8192E)
		pdesc->paddr = set_desc(phw->rx_infoL[tail].paddr);
		pdesc->cmd = set_desc((tail == (NUM_RX_DESC - 1)? _EOR_ : 0) | _OWN_ | (RX_BUF_LEN - sizeof(struct rx_frinfo)));
#elif	defined(RTL8192SE)

#ifdef DELAY_REFILL_RX_BUF
		if (tail != phw->cur_rx_refill)  {
			pskb->data = pskb->head;
			pskb->tail = pskb->head;
			skb_reserve(pskb, 128);
#ifdef CONFIG_RTL8196_RTL8366
			skb_reserve(pskb, 8);
#endif
#ifdef CONFIG_RTK_VOIP_VLAN_ID
			skb_reserve(pskb, 4);
#endif
			refill_rx_ring(priv, pskb, NULL);
			refill = 0;
		}
		else
#endif
#endif  //ifndef NEW_MAC80211_DRV
		{
			pdesc->Dword6 = set_desc(phw->rx_infoL[tail].paddr);
			pdesc->Dword0 = set_desc((tail == (NUM_RX_DESC - 1)? RX_EOR : 0) | RX_OWN | (RX_BUF_LEN - sizeof(struct rx_frinfo)));
		}
#endif

rx_done:

		tail = (tail + 1) % NUM_RX_DESC;
#if defined(DELAY_REFILL_RX_BUF) && !defined(NEW_MAC80211_DRV)
		if (refill)
			phw->cur_rx_refill = (phw->cur_rx_refill + 1) % NUM_RX_DESC;
#endif
	}

#ifdef RTL8190_FASTEXTDEV_FUNCALL
rx_exit:
#endif

	phw->cur_rx = tail;

	if (!IS_DRV_OPEN(priv))
		return;

#if defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX) && !defined(NEW_MAC80211_DRV)
	if (!list_empty(&priv->wakeup_list))
		process_dzqueue(priv);
#endif
}

#if defined(NEW_MAC80211_DRV) && defined(RTL8192SE)
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

        rtl8190_rx_isr(priv);
        // legacy isr start
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
#endif  //if defined(NEW_MAC80211_DRV) && defined(RTL8192SE)

static int auth_filter(struct rtl8190_priv *priv, struct stat_info *pstat,
				struct rx_frinfo *pfrinfo)
{
	unsigned int	hdr_len;
	unsigned char	*pframe, *pbuf;
	unsigned short	proto;

	hdr_len = pfrinfo->hdr_len;
	pframe = get_pframe(pfrinfo);
	pbuf = pframe + hdr_len + sizeof(struct wlan_llc_t) + 3;
	proto = *(unsigned short *)pbuf;

	if(IEEE8021X_FUN) {
		if (pstat) {
			if (pstat->ieee8021x_ctrlport) // controlled port is enable...
				return SUCCESS;
			else {
				//only 802.1x frame can pass...
				if (proto == __constant_htons(0x888e))
					return SUCCESS;
				else {
					return FAIL;
				}
			}
		}
		else {
			DEBUG_ERR("pstat == NULL in auth_filter\n");
			return FAIL;
		}
	}

	return SUCCESS;
}


#ifdef CONFIG_RTL865X
void rtl865x_wlanAccelerate(struct rtl8190_priv *priv, struct sk_buff *pskb)
{
	int retval, id=0;

	priv->dev->last_rx = jiffies;
#if defined(WDS) && defined(CONFIG_RTL865X)
	id = pskb->cb[0];
#endif

#ifdef RTL8190_FASTEXTDEV_FUNCALL
	retval = -1;
	if (	(pskb->data[0] & 0x01) == 0 )
	{
		retval = rtl865x_extDev_unicastFastRecv(pskb,
												priv->WLAN_VLAN_ID,
												priv->dev,
												(1 << priv->rtl8650extPortNum));
	}

	if ( retval == 0 )
	{
		/* Packet is received by Unicast Fast Recv procedure */
	} else
#endif
	{

#ifdef RTL8190_FASTEXTDEV_FUNCALL
		rtl865x_extDev_mbuf2Skb(priv->dev, pskb);
#endif

		retval = rtlglue_extPortRecv(pskb,
									 pskb->data,
									 pskb->len,
									 priv->WLAN_VLAN_ID,
									 (1<<priv->rtl8650extPortNum),
									 priv->rtl8650linkNum[id]);
		if (retval == 0) {
			/* 8651 fwd engine consumed the packet. */
		}
		else if (retval == -1) {
			/* exception. Drop it. */
			kfree(pskb);
		}
		else {
			/* other cases, send this packet to wlan interface */
			pskb->dev = priv->dev;								/* important, crash if not correct */
			pskb->protocol = eth_type_trans(pskb, priv->dev);		/* before sending to protocol stack, we MUSt got its protocol and modify packet's pointer */
			netif_rx(pskb);
		}
	}
}
#endif

#ifdef CONFIG_RTL8186_KB
int rtl8190_guestmac_valid(struct rtl8190_priv *priv, char *macaddr)
{
	int i=0;

	for (i=0; i<MAX_GUEST_NUM; i++)
	{
		if (priv->guestMac[i].valid && !memcmp(priv->guestMac[i].macaddr, macaddr, 6))
			return 1;
	}
	return 0;
}
#endif


/*
	Actually process RX management frame:

		Process management frame stored in "inputPfrinfo" or gotten from "list",
		only one of them is used to get Frame information.

			Note that:
			1. If frame information is gotten from "list", "inputPfrinfo" MUST be NULL.
			2. If frame information is gotten from "inputPfrinfo", "list" MUST be NULL
*/
#ifndef CONFIG_RTK_MESH
static
#endif
 void rtl8190_rx_mgntframe(struct rtl8190_priv *priv, struct list_head *list,
				struct rx_frinfo *inputPfrinfo)
{
	struct rx_frinfo *pfrinfo = NULL;

	// for SW LED
	if ((LED_TYPE >= LEDTYPE_SW_LINK_TXRX) && (LED_TYPE <= LEDTYPE_SW_LINKTXRX))
		priv->pshare->LED_rx_cnt++;

	/* Get RX frame info */
	if (list) {
		/* Indicate the frame information can be gotten from "list" */
		pfrinfo = list_entry(list, struct rx_frinfo, rx_list);
	}
	else {
		/* Indicate the frame information is stored in "inputPfrinfo" */
		pfrinfo = inputPfrinfo;
	}

	if (pfrinfo == NULL)
		goto out;

#if defined(CONFIG_RTL865X) && defined(WDS)
	pfrinfo->pskb->cb[0] = 0;
#endif

	mgt_handler(priv, pfrinfo);

out:
	return;
}


/*
	Actually process RX control frame:

		Process control frame stored in "inputPfrinfo" or gotten from "list",
		only one of them is used to get Frame information.

			Note that:
			1. If frame information is gotten from "list", "inputPfrinfo" MUST be NULL.
			2. If frame information is gotten from "inputPfrinfo", "list" MUST be NULL
*/
static void rtl8190_rx_ctrlframe(struct rtl8190_priv *priv, struct list_head *list,
				struct rx_frinfo *inputPfrinfo)
{
	struct rx_frinfo *pfrinfo = NULL;

	/* Get RX frame info */
	if (list) {
		/* Indicate the frame information can be gotten from "list" */
		pfrinfo = list_entry(list, struct rx_frinfo, rx_list);
	}
	else {
		/* Indicate the frame information is stored in "inputPfrinfo " */
		pfrinfo = inputPfrinfo;
	}

	if (pfrinfo == NULL)
		goto out;

#if defined(CONFIG_RTL865X) && defined(WDS)
	pfrinfo->pskb->cb[0] = 0;
#endif

	ctrl_handler(priv, pfrinfo);

out:
	return;
}


/*
	Actually process RX data frame:

		Process data frame stored in "inputPfrinfo" or gotten from "list",
		only one of them is used to get Frame information.

			Note that:
			1. If frame information is gotten from "list", "inputPfrinfo" MUST be NULL.
			2. If frame information is gotten from "inputPfrinfo", "list" MUST be NULL
*/

#if !(defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX))
void process_all_queues(struct rtl8190_priv *priv)
{
	struct list_head *list = NULL;

	// processing data frame first...
	while(1)
	{
		list = dequeue_frame(priv, &(priv->rx_datalist));

		if (list == NULL)
			break;

		rtl8190_rx_dataframe(priv, list, NULL);
	}

	// going to process management frame
	while(1)
	{
		list = dequeue_frame(priv, &(priv->rx_mgtlist));

		if (list == NULL)
			break;

		rtl8190_rx_mgntframe(priv, list, NULL);
	}

	while(1)
	{
		list = dequeue_frame(priv, &(priv->rx_ctrllist));

		if (list == NULL)
			break;

		rtl8190_rx_ctrlframe(priv, list, NULL);
	}

	if (!list_empty(&priv->wakeup_list))
		process_dzqueue(priv);
}


__IRAM_FASTEXTDEV
void rtl8190_rx_dsr(unsigned long task_priv)
{
	struct rtl8190_priv	*priv = (struct rtl8190_priv *)task_priv;
	unsigned long ioaddr = priv->pshare->ioaddr;
	__DRAM_IN_865X static unsigned long flags;
	__DRAM_IN_865X static unsigned long mask, mask_rx;
#ifdef MBSSID
	int i;
#endif

#ifndef __ASUS_DVD__
	extern int r3k_flush_dcache_range(int, int);
#endif

#ifdef __KERNEL__
#if	defined(RTL8190) || defined(RTL8192E)
	// disable rx interrupt in DSR
	SAVE_INT_AND_CLI(flags);
	mask = RTL_R32(_IMR_);
	mask_rx = mask & (_RXFOVW_ | _RDU_ | _ROK_);
	RTL_W32(_IMR_, mask & ~mask_rx);
	RTL_W32(_ISR_, mask_rx);
#elif defined(RTL8192SE)
	// disable rx interrupt in DSR
	SAVE_INT_AND_CLI(flags);
	mask = RTL_R32(IMR);
	mask_rx = mask & (IMR_RXFOVW | IMR_RDU | IMR_ROK);
	RTL_W32(IMR, mask & ~mask_rx);
	RTL_W32(ISR, mask_rx);
#endif
#endif

#if	defined(RTL8190) || defined(RTL8192E)
	rtl8190_rx_isr(priv);
#elif	defined(RTL8192SE)
	rtl8190_rx_isr(priv);
#endif

#ifdef __KERNEL__
	RESTORE_INT(flags);
#endif

	process_all_queues(priv);

#ifdef UNIVERSAL_REPEATER
	if (IS_DRV_OPEN(GET_VXD_PRIV(priv)))
		process_all_queues(GET_VXD_PRIV(priv));
#endif

#ifdef MBSSID
#ifdef RTL8192SE
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable)
#endif
	{
	for (i=0; i<RTL8190_NUM_VWLAN; i++) {
		if (IS_DRV_OPEN(priv->pvap_priv[i]))
			process_all_queues(priv->pvap_priv[i]);
	}
	}
#endif

#ifdef __KERNEL__
#if	defined(RTL8190) || defined(RTL8192E)
	// enable rx interrupt in DSR
	mask = RTL_R32(_IMR_);
	RTL_W32(_IMR_, mask | mask_rx);
#elif defined(RTL8192SE)
	mask = RTL_R32(IMR);
	RTL_W32(IMR, mask | mask_rx);
#endif
#endif
}
#endif // !(defined(RTL8190_ISR_RX) && defined(RTL8190_DIRECT_RX))

static void ctrl_handler(struct rtl8190_priv *priv, struct rx_frinfo *pfrinfo)
{
	struct sk_buff *pskbpoll, *pskb;
	unsigned char *pframe;
	struct stat_info *pstat;
	unsigned short aid;
	DECLARE_TXINSN(txinsn);

	pframe  = get_pframe(pfrinfo);
	pskbpoll = get_pskb(pfrinfo);

	aid = GetAid(pframe);

	pstat = get_aidinfo(priv, aid);

	if (pstat == NULL)
		goto end_ctrl;

	// check if hardware address matches...
	if (memcmp(pstat->hwaddr, (void *)(pframe + 10), MACADDRLEN))
		goto end_ctrl;

	// now dequeue from the pstat's dz_queue
	pskb = __skb_dequeue(&pstat->dz_queue);

	if (pskb == NULL)
		goto end_ctrl;

	txinsn.q_num   = BE_QUEUE; //using low queue for data queue
	txinsn.fr_type = _SKB_FRAME_TYPE_;
	txinsn.pframe  = pskb;
	txinsn.phdr	   = (UINT8 *)get_wlanllchdr_from_poll(priv);
	pskb->cb[1] = 0;

	if (pskb->len > priv->pmib->dot11OperationEntry.dot11RTSThreshold)
		txinsn.retry = priv->pmib->dot11OperationEntry.dot11LongRetryLimit;
	else
		txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;

	if (txinsn.phdr == NULL) {
		DEBUG_ERR("Can't alloc wlan header!\n");
		goto xmit_skb_fail;
	}

	memset((void *)txinsn.phdr, 0, sizeof(struct wlanllc_hdr));

	SetFrDs(txinsn.phdr);
#ifdef SEMI_QOS
	if (pstat && (QOS_ENABLE) && (pstat->QosEnabled))
		SetFrameSubType(txinsn.phdr, WIFI_QOS_DATA);
	else
#endif
	SetFrameSubType(txinsn.phdr, WIFI_DATA);

	if (skb_queue_len(&pstat->dz_queue))
		SetMData(txinsn.phdr);

	if (rtl8190_wlantx(priv, &txinsn) == CONGESTED)
	{

xmit_skb_fail:

		priv->ext_stats.tx_drops++;
		DEBUG_WARN("TX DROP: Congested!\n");
		if (txinsn.phdr)
			release_wlanllchdr_to_poll(priv, txinsn.phdr);
		if (pskb)
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
	}

end_ctrl:

	if (pskbpoll) {
		rtl_kfree_skb(priv, pskbpoll, _SKB_RX_);
	}

	return;
}


/*
typedef struct tx_sts_struct
{
	// DW 1
	UINT8	TxRateid;
	UINT8	TxRate;
} tx_sts;

typedef struct tag_Tx_Status_Feedback
{
	// For endian transfer --> for driver
	// DW 0
	UINT16	Length;					// Command packet length
	UINT8 	Reserve1;
	UINT8 	Element_ID;			// Command packet type

	tx_sts    Tx_Sts[NUM_STAT];
} CMPK_TX_STATUS_T;
*/


#if	defined(RTL8190) || defined(RTL8192E)
void rtl8190_rxcmd_isr(struct rtl8190_priv *priv)
{
// reduce stack usage
#if 0
	struct rx_desc		*pdesc, *prxdesc;
	struct rtl8190_hw	*phw;
	struct sk_buff		*pskb;
	struct rx_frinfo	*pfrinfo=NULL;
	unsigned int		tail, cmd, i;
	struct stat_info	*pstat;
	unsigned char		*ptr, ratid, txrate;
#else

	static	struct rx_desc		*pdesc, *prxdesc;
	static	struct rtl8190_hw	*phw;
	static	struct sk_buff		*pskb;
	static	struct rx_frinfo	*pfrinfo;
	static	unsigned int		tail, cmd, i;
	static	struct stat_info	*pstat;
	static	unsigned char		*ptr, ratid, txrate;

	pfrinfo=NULL;
#endif

	phw = GET_HW(priv);
	tail = phw->cur_rxcmd;

#ifdef RTL8190_CACHABLE_DESC

	prxdesc = (struct rx_desc *)(phw->rxcmd_desc);
	rtl_cache_sync_wback(priv, (unsigned int)prxdesc, sizeof(struct rx_desc), PCI_DMA_FROMDEVICE);

#else

#ifdef __MIPSEB__
	prxdesc = (struct rx_desc *)((unsigned int)(phw->rxcmd_desc) | 0x20000000);
#else
	prxdesc = (struct rx_desc *)(phw->rxcmd_desc);
#endif

#endif	/* RTL8190_CACHABLE_DESC */

	while (1)
	{
		pdesc = prxdesc + tail;
		cmd = get_desc(pdesc->cmd);

		if (cmd & _OWN_)
			break;

 		pskb = (struct sk_buff *)(phw->rxcmd_info[tail].pbuf);
		pfrinfo = get_pfrinfo(pskb);
		pfrinfo->pktlen = cmd & _RXFRLEN_MSK_;
		rtl_cache_sync_wback(priv, phw->rxcmd_info[tail].paddr, pfrinfo->pktlen, PCI_DMA_FROMDEVICE);

		/*-----------------------------------------------------
						Process Rx command packets
		------------------------------------------------------*/
		ptr = pskb->data + 4;
		for (i=0; i<NUM_STAT; i++)
		{
			ratid = *(ptr + 1);

#ifdef RTL8192E
			//Joshua modified for reduce AID
			if ((i != 0) && (ratid == 0))
				continue;
			pstat = get_aidinfo(priv, ratid);
#elif	defined(RTL8190)
			if ((i != 0) && (ratid == 0) && (ratid == 7))
				continue;

			if (ratid == 0)
				pstat = get_aidinfo(priv, 7);
			else
				pstat = get_aidinfo(priv, ratid);
#endif

			if (pstat) {
				txrate = *ptr;

				if (txrate < 12)
					pstat->current_tx_rate = dot11_rate_table[txrate];
				else
					pstat->current_tx_rate = 0x80 | (txrate & 0x0f);
			}

			ptr += 2;
		}

		pdesc->paddr = set_desc(phw->rxcmd_info[tail].paddr);
		pdesc->cmd = set_desc((tail == (NUM_CMD_DESC - 1)? _EOR_ : 0) | _OWN_ | (RX_BUF_LEN - sizeof(struct rx_frinfo)));

		tail = (tail + 1) % NUM_CMD_DESC;
	}

	phw->cur_rxcmd = tail;
}
#endif


#ifdef CONFIG_RTL_KERNEL_MIPS16_WLAN
__NOMIPS16
#endif
static void process_amsdu(struct rtl8190_priv *priv, struct stat_info *pstat, struct rx_frinfo *pfrinfo)
{
	unsigned char	*pframe, *da;
	struct stat_info	*dst_pstat = NULL;
	struct sk_buff	*pskb  = NULL, *pnewskb = NULL;
	unsigned char	*next_head;
	int				rest, agg_pkt_num=0, i, privacy;
	unsigned int	subfr_len, padding;
const	unsigned char	rfc1042_ip_header[8]={0xaa,0xaa,0x03,00,00,00,0x08,0x00};

	pframe = get_pframe(pfrinfo);
	pskb = get_pskb(pfrinfo);

	rest = pfrinfo->pktlen - pfrinfo->hdr_len;
	next_head = pframe + pfrinfo->hdr_len;

	if (GetPrivacy(pframe)) {
#ifdef WDS
		if (pfrinfo->to_fr_ds==3)
			privacy = priv->pmib->dot11WdsInfo.wdsPrivacy;
		else
#endif
			privacy = get_sta_encrypt_algthm(priv, pstat);
		if ((privacy == _CCMP_PRIVACY_) || (privacy == _TKIP_PRIVACY_)) {
			rest -= 8;
			next_head += 8;
		}
		else {	// WEP
			rest -= 4;
			next_head += 4;
		}
	}

	while (rest > WLAN_ETHHDR_LEN) {
		pnewskb = skb_clone(pskb, GFP_ATOMIC);
		if (pnewskb) {
			pnewskb->data = next_head;
			subfr_len = (*(next_head + MACADDRLEN*2) << 8) + (*(next_head + MACADDRLEN*2 + 1));
			pnewskb->len = WLAN_ETHHDR_LEN + subfr_len;
			pnewskb->tail = pnewskb->data + pnewskb->len;
			if (!memcmp(rfc1042_ip_header, pnewskb->data+WLAN_ETHHDR_LEN, 8)) {
				for (i=0; i<MACADDRLEN*2; i++)
					pnewskb->data[19-i] = pnewskb->data[11-i];
				pnewskb->data += 8;
				pnewskb->len -= 8;
			}
			else
				strip_amsdu_llc(priv, pnewskb, pstat);
			agg_pkt_num++;

			if (OPMODE & WIFI_AP_STATE)
			{
				da = pnewskb->data;
				dst_pstat = get_stainfo(priv, da);
				if ((dst_pstat == NULL) || (!(dst_pstat->state & WIFI_ASOC_STATE)))
					rtl_netif_rx(priv, pnewskb, pstat);
				else
				{
					if (priv->pmib->dot11OperationEntry.block_relay == 1) {
						priv->ext_stats.rx_data_drops++;
						DEBUG_ERR("RX DROP: Relay unicast packet is blocked!\n");
#ifdef RX_SHORTCUT
						if (dst_pstat->rx_payload_offset > 0) // shortcut data saved, clear it
							dst_pstat->rx_payload_offset = 0;
#endif
						rtl_kfree_skb(priv, pnewskb, _SKB_RX_);
					}
					else if (priv->pmib->dot11OperationEntry.block_relay == 2) {
						DEBUG_INFO("Relay unicast packet is blocked! Indicate to bridge.\n");
						rtl_netif_rx(priv, pnewskb, pstat);
					}
					else {
#ifdef ENABLE_RTL_SKB_STATS
						rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif
#ifdef GBWC
						if (GBWC_forward_check(priv, pfrinfo->pnewskb, pstat)) {
							// packet is queued, nothing to do
						}
						else
#endif
#ifdef CONFIG_RTK_MESH
							if (rtl8190_start_xmit(pnewskb, isMeshPoint(dst_pstat) ? priv->mesh_dev: priv->dev))
#else
						if (rtl8190_start_xmit(pnewskb, priv->dev))
#endif
							rtl_kfree_skb(priv, pnewskb, _SKB_RX_);
					}
				}
			}
#ifdef CLIENT_MODE
			else if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE))
			{
#ifdef RTK_BR_EXT
				if(nat25_handle_frame(priv, pnewskb) == -1) {
					priv->ext_stats.rx_data_drops++;
					DEBUG_ERR("RX DROP: nat25_handle_frame fail!\n");
					rtl_kfree_skb(priv, pnewskb, _SKB_RX_);
				}
#endif
				rtl_netif_rx(priv, pnewskb, pstat);
			}
#endif

			padding = 4 - ((WLAN_ETHHDR_LEN + subfr_len) % 4);
			if (padding == 4)
				padding = 0;
			rest -= (WLAN_ETHHDR_LEN + subfr_len + padding);
			next_head += (WLAN_ETHHDR_LEN + subfr_len + padding);
		}
		else {
			// Can't get new skb header, drop this packet
			break;
		}
	}

	// clear saved shortcut data
#ifdef RX_SHORTCUT
	if (pstat->rx_payload_offset)
		pstat->rx_payload_offset = 0;
#endif

#ifdef _DEBUG_RTL8190_
	switch (agg_pkt_num) {
	case 0:
		pstat->rx_amsdu_err++;
		break;
	case 1:
		pstat->rx_amsdu_1pkt++;
		break;
	case 2:
		pstat->rx_amsdu_2pkt++;
		break;
	case 3:
		pstat->rx_amsdu_3pkt++;
		break;
	case 4:
		pstat->rx_amsdu_4pkt++;
		break;
	case 5:
		pstat->rx_amsdu_5pkt++;
		break;
	default:
		pstat->rx_amsdu_gt5pkt++;
		break;
	}
#endif

	rtl_kfree_skb(priv, pskb, _SKB_RX_);
}


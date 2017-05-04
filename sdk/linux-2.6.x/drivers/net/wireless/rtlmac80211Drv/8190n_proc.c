/*
 *  Handle routines for proc file system
 *
 *  $Id: 8190n_proc.c,v 1.1.1.1 2010/05/05 09:00:44 jiunming Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#define _8190N_PROC_C_

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/netdevice.h>
#include <linux/compile.h>
#include <linux/init.h>
#endif

#include "./8190n_cfg.h"
#include "./8190n.h"
#include "./8190n_headers.h"
#include "./8190n_debug.h"
#include <asm/uaccess.h>


/*
 *	Note : These define someone copy to ./mesh_ext/mesh_proc.c !!
*/
#define PRINT_ONE(val, format, line_end) { 		\
	pos += sprintf(&buf[pos], format, val);		\
	if (line_end)					\
		strcat(&buf[pos++], "\n");		\
}

#define PRINT_ARRAY(val, format, len, line_end) { 	\
	int index;					\
	for (index=0; index<len; index++)		\
		pos += sprintf(&buf[pos], format, val[index]); \
	if (line_end)					\
		strcat(&buf[pos++], "\n");		\
							\
}

#define PRINT_SINGL_ARG(name, para, format) { \
	PRINT_ONE(name, "%s", 0); \
	PRINT_ONE(para, format, 1); \
}

#define PRINT_ARRAY_ARG(name, para, format, len) { \
	PRINT_ONE(name, "%s", 0); \
	PRINT_ARRAY(para, format, len, 1); \
}

#define CHECK_LEN { \
	len += size; \
	pos = begin + len; \
	if (pos < offset) { \
		len = 0; \
		begin = pos; \
	} \
	if (pos > offset + length) \
		goto _ret; \
}


const unsigned char* MCS_DATA_RATEStr[2][2][16] =
{
	{{"6.5", "13", "19.5", "26", "39", "52", "58.5", "65", "13", "26", "39" ,"52", "78", "104", "117", "130"},		// Long GI, 20MHz
	 {"7.2", "14.4", "21.7", "28.9", "43.3", "57.8", "65", "72.2", "14.4", "28.9", "43.3", "57.8", "86.7", "115.6", "130", "144.5"}},		// Short GI, 20MHz
	{{"13.5", "27", "40.5", "54", "81", "108", "121.5", "135", "27", "54", "81", "108", "162", "216", "243", "270"},	// Long GI, 40MHz
	 {"15", "30", "45", "60", "90", "120", "135", "150", "30", "60", "90", "120", "180", "240", "270", "300"}}	// Short GI, 40MHz
};


static int rtl8190_proc_cam_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct mac80211_shared_priv *priv = (struct mac80211_shared_priv*) data;

	int pos = 0, i;
	unsigned char TempOutputMac[6];
	unsigned char TempOutputKey[16];
	unsigned short TempOutputCfg=0;

	PRINT_ONE("  CAM info...", "%s", 1);
	PRINT_ONE("    CAM occupied: ", "%s", 0);
	PRINT_ONE(priv->pshare_hw->CamEntryOccupied, "%d", 1);
	for (i=0; i<32; i++)
	{
		PRINT_ONE("    Entry", "%s", 0);
		PRINT_ONE(i, " %2d:", 0);
		CAM_read_entry(priv,i,TempOutputMac,TempOutputKey,&TempOutputCfg);
		PRINT_ARRAY_ARG(" MAC addr: ", TempOutputMac, "%02x", 6);
		PRINT_SINGL_ARG("              Config: ", TempOutputCfg, "%x");
		PRINT_ARRAY_ARG("              Key: ", TempOutputKey, "%02x", 16);
	}

	return pos;
}

static int rtl8190_proc_tx_pkt(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{

	struct mac80211_shared_priv *priv = (struct mac80211_shared_priv*) data;
	int pos = 0;
#if 1
	struct sk_buff *pskb = NULL;
	unsigned char nullpkt[] = { 
		0x48,0x11,0x1c,0x00,
//		0x00,0x11,0x22,0xcc,0xdd,0xee,
		0xff,0xff,0xff,0xff,0xff,0xff,

		0x00,0x00,0x00,0x00,0x0a,0x02,
//		0x00,0x11,0x22,0xcc,0xdd,0xee,
		0xff,0xff,0xff,0xff,0xff,0xff,

		0x80,0x97 };

	pskb = dev_alloc_skb(800);	
	if(pskb) 
	{
		PRINT_ONE("  tx pkt...", "%s", 1);
		memcpy(skb_put(pskb, sizeof(nullpkt)), nullpkt, sizeof(nullpkt)) ;
		rtl80211_tx(priv->hw, pskb);
	}
#elif 0
	//unsigned long	ioaddr = priv->pshare_hw->ioaddr;
	//struct rtl8190_hw	*phw=GET_HW(priv);
	struct sk_buff *skb;
	if (priv->beaconbuf != NULL){
		skb = priv->beaconbuf;
		pci_unmap_single(priv->pshare_hw->pdev, priv->bcndma_addr, 
				skb->len, PCI_DMA_FROMDEVICE);
		dev_kfree_skb_any(skb);
		priv->beaconbuf = NULL;
	}
	skb = ieee80211_beacon_get(priv->hw, priv->vif);
	
	if (skb == NULL) {
		printk("cannot get beacon skb\n");
		return -ENOMEM;
	}
	if(skb) 
	{
		printHex(skb->data, skb->len);
	}
	priv->beaconbuf = skb;
	priv->bcndma_addr =	get_physical_addr(priv, skb, skb->len, PCI_DMA_TODEVICE);

	
#else
	update_beacon(priv);
	
#endif
	return pos;
}



static int rtl8190_proc_txdesc_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data, int q_num)
{
	struct mac80211_shared_priv *priv = (struct mac80211_shared_priv*) data;

	struct rtl8190_hw *phw;
	unsigned long *txdescptr;

	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	int i, size;

	phw = GET_HW(priv);
	size = sprintf(buf, "  Tx queue %d descriptor ..........\n", q_num);
	CHECK_LEN;
	if (get_txdesc(phw, q_num)) {
		size = sprintf(buf+len, "  tx_desc%d/physical: 0x%.8x/0x%.8x\n", q_num, (UINT)get_txdesc(phw, q_num),
						*(UINT *)(((UINT)&phw->tx_ring0_addr)+sizeof(unsigned long)*q_num));
		CHECK_LEN;
		size = sprintf(buf+len, "  head/tail: %3d/%-3d  DW0      DW1      DW2      DW3      DW4      DW5\n",
			get_txhead(phw, q_num), get_txtail(phw, q_num));
		CHECK_LEN;
		for (i=0; i<NUM_TX_DESC; i++) {
			txdescptr = (unsigned long *)(get_txdesc(phw, q_num) + i);
			size = sprintf(buf+len, "      txdesc%d[%3d]: %.8x %.8x %.8x %.8x %.8x \n", q_num, i,
					(UINT)get_desc(txdescptr[0]), (UINT)get_desc(txdescptr[1]),
					(UINT)get_desc(txdescptr[2]), (UINT)get_desc(txdescptr[3]),
					(UINT)get_desc(txdescptr[4]));
			CHECK_LEN;
		}
	}
	*eof = 1;

_ret:
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return len;
}


static int rtl8190_proc_txdesc0_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	return rtl8190_proc_txdesc_info(buf, start, offset, length, eof, data, 0);
}


static int rtl8190_proc_txdesc1_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	return rtl8190_proc_txdesc_info(buf, start, offset, length, eof, data, 1);
}


static int rtl8190_proc_txdesc2_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	return rtl8190_proc_txdesc_info(buf, start, offset, length, eof, data, 2);
}


static int rtl8190_proc_txdesc3_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	return rtl8190_proc_txdesc_info(buf, start, offset, length, eof, data, 3);
}


static int rtl8190_proc_txdesc4_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	return rtl8190_proc_txdesc_info(buf, start, offset, length, eof, data, 4);
}


static int rtl8190_proc_txdesc5_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	return rtl8190_proc_txdesc_info(buf, start, offset, length, eof, data, 5);
}


static int rtl8190_proc_rxdesc_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct mac80211_shared_priv *priv = (struct mac80211_shared_priv*) data;

	struct rtl8190_hw *phw;
	unsigned long *rxdescptr;

	int len = 0;
	off_t begin = 0;
	off_t pos = 0;
	int i, size;

	phw = GET_HW(priv);
	size = sprintf(buf+len, "  Rx queue descriptor ..........\n");
	CHECK_LEN;
	if(phw->rx_descL){
		size = sprintf(buf+len, "  rx_descL/physical: 0x%.8x/0x%.8x\n", (UINT)phw->rx_descL, (UINT)phw->rx_ring_addr);
		CHECK_LEN;
		size = sprintf(buf+len, "  cur_rx: %d\n", phw->cur_rx);
		CHECK_LEN;
		for(i=0; i<NUM_RX_DESC; i++) {
			rxdescptr = (unsigned long *)(phw->rx_descL+i);
			size = sprintf(buf+len, "      rxdesc[%02d]: 0x%.8x 0x%.8x 0x%.8x 0x%.8x \n", i,
					(UINT)get_desc(rxdescptr[0]), (UINT)get_desc(rxdescptr[1]),
					(UINT)get_desc(rxdescptr[2]), (UINT)get_desc(rxdescptr[3]));
			CHECK_LEN;
		}
	}
	*eof = 1;

_ret:
	*start = buf + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return len;
}


static int rtl8190_proc_desc_info(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
//truct net_device *dev = (struct net_device *)data;
//truct rtl8190_priv *priv = (struct rtl8190_priv *)dev->priv;
	struct mac80211_shared_priv *priv = (struct mac80211_shared_priv*) data;

	struct rtl8190_hw *phw = GET_HW(priv);
	unsigned long ioaddr = priv->pshare_hw->ioaddr;
	int pos = 0;

	PRINT_ONE("  descriptor info...", "%s", 1);
	PRINT_ONE("    RX queue:", "%s", 1);
	PRINT_ONE("      rx_descL/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->rx_descL, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->rx_ring_addr, "0x%.8x", 1);
	PRINT_ONE("      RDSAR: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_RDSAR_), "0x%.8x", 0);
	PRINT_ONE("  cur_rx: ", "%s", 0);
	PRINT_ONE((UINT)phw->cur_rx, "%d", 1);

	PRINT_ONE("    queue 0:", "%s", 1);
	PRINT_ONE("      tx_desc0/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc0, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring0_addr, "0x%.8x", 1);
	PRINT_ONE("      TMGDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_TMGDA_), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead0, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail0, "%d", 1);

	PRINT_ONE("    queue 1:", "%s", 1);
	PRINT_ONE("      tx_desc1/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc1, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring1_addr, "0x%.8x", 1);
	PRINT_ONE("      TBKDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_TBKDA_), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead1, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail1, "%d", 1);

	PRINT_ONE("    queue 2:", "%s", 1);
	PRINT_ONE("      tx_desc2/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc2, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring2_addr, "0x%.8x", 1);
	PRINT_ONE("      TBEDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_TBEDA_), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead2, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail2, "%d", 1);

	PRINT_ONE("    queue 3:", "%s", 1);
	PRINT_ONE("      tx_desc3/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc3, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring3_addr, "0x%.8x", 1);
	PRINT_ONE("      TLPDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_TLPDA_), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead3, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail3, "%d", 1);

	PRINT_ONE("    queue 4:", "%s", 1);
	PRINT_ONE("      tx_desc4/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc4, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring4_addr, "0x%.8x", 1);
	PRINT_ONE("      TNPDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_TNPDA_), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead4, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail4, "%d", 1);

	PRINT_ONE("    queue 5:", "%s", 1);
	PRINT_ONE("      tx_desc5/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->tx_desc5, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->tx_ring5_addr, "0x%.8x", 1);
	PRINT_ONE("      THPDA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_THPDA_), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txhead5, "%d/", 0);
	PRINT_ONE((UINT)phw->txtail5, "%d", 1);

	PRINT_ONE("    RX cmd queue:", "%s", 1);
	PRINT_ONE("      rxcmd_desc/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->rxcmd_desc, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->rxcmd_ring_addr, "0x%.8x", 1);
	PRINT_ONE("      RCDSA: ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_RCDSA_), "0x%.8x", 0);
	PRINT_ONE("  cur_rx: ", "%s", 0);
	PRINT_ONE((UINT)phw->cur_rxcmd, "%d", 1);

	PRINT_ONE("    TX cmd queue:", "%s", 1);
	PRINT_ONE("      txcmd_desc/physical: ", "%s", 0);
	PRINT_ONE((UINT)phw->txcmd_desc, "0x%.8x/", 0);
	PRINT_ONE((UINT)phw->txcmd_ring_addr, "0x%.8x", 1);
	PRINT_ONE("      TCDA:  ", "%s", 0);
	PRINT_ONE((UINT)RTL_R32(_TCDA_), "0x%.8x", 0);
	PRINT_ONE("  head/tail: ", "%s", 0);
	PRINT_ONE((UINT)phw->txcmdhead, "%d/", 0);
	PRINT_ONE((UINT)phw->txcmdtail, "%d", 1);

#if defined(CONFIG_RTL865X) && defined(RTL865X_INFO)
	PRINT_ONE("    Total Rx:", "%s", 1);
	PRINT_ONE("      pkt/schedule=avg Rx pkt handled: ", "%s", 0);
	PRINT_ONE((UINT)rcvPktCnt, "%d/", 0);
	PRINT_ONE((UINT)rcvLoopCount, "%d=", 0);
	PRINT_ONE((UINT)rcvPktCnt/rcvLoopCount, "%d", 1);
#endif

	return pos;
}


#if defined(RTL8192SE) && defined(MERGE_FW)
static char *get_one_line(char *start, char *end, char *out, int max_len)
{
	int len = 0;

	while (1) {
		if (((unsigned long)start) >= ((unsigned long)end)) {
			if (len > 0) {
				out[len] = '\0';
				return start;
			}
			else
				return NULL;
		}

		if (*start == '\n' || *start == '\r' || len >= max_len) {
			if (*start == '\n' || *start == '\r')
				start++;

			if (len > 0) {
				out[len] = '\0';
				return start;
			}
		}
		else
			out[len++] = *start++;
	}
}


static int rtl8190_proc_agc_tab(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	int pos = 0;
	char tmpbuf[256];
	char *ptr = __AGC_TAB_start;

	while (1) {
		ptr = get_one_line(ptr, __AGC_TAB_end, tmpbuf, 256);
		if (ptr) {
			PRINT_ONE(tmpbuf, "%s", 1);
		}
		else
			break;
	}
	return pos;
}

static int rtl8190_proc_phy_reg(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	int pos = 0;
	char tmpbuf[256];
	char *ptr = __phy_reg_start;

	while (1) {
		ptr = get_one_line(ptr, __phy_reg_end, tmpbuf, 256);
		if (ptr) {
			PRINT_ONE(tmpbuf, "%s", 1);
		}
		else
			break;
	}
	return pos;
}


static int rtl8190_proc_macphy_reg(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	int pos = 0;
	char tmpbuf[256];
	char *ptr = __MACPHY_REG_start;

	while (1) {
		ptr = get_one_line(ptr, __MACPHY_REG_end, tmpbuf, 256);
		if (ptr) {
			PRINT_ONE(tmpbuf, "%s", 1);
		}
		else
			break;
	}
	return pos;
}


static int rtl8190_proc_radio_a(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	int pos = 0;
	char tmpbuf[256];
	char *ptr = __radio_a_start;

	while (1) {
		ptr = get_one_line(ptr, __radio_a_end, tmpbuf, 256);
		if (ptr) {
			PRINT_ONE(tmpbuf, "%s", 1);
		}
		else
			break;
	}
	return pos;
}


static int rtl8190_proc_radio_b(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	int pos = 0;
	char tmpbuf[256];
	char *ptr = __radio_b_start;

	while (1) {
		ptr = get_one_line(ptr, __radio_b_end, tmpbuf, 256);
		if (ptr) {
			PRINT_ONE(tmpbuf, "%s", 1);
		}
		else
			break;
	}
	return pos;
}


static int rtl8190_proc_phy_reg_pg(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	int pos = 0;
	char tmpbuf[256];
	char *ptr = __PHY_REG_PG_start;

	while (1) {
		ptr = get_one_line(ptr, __PHY_REG_PG_end, tmpbuf, 256);
		if (ptr) {
			PRINT_ONE(tmpbuf, "%s", 1);
		}
		else
			break;
	}
	return pos;
}


static int rtl8190_proc_phy_to1t2r(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	int pos = 0;
	char tmpbuf[256];
	char *ptr = __PHY_to1T2R_start;

	while (1) {
		ptr = get_one_line(ptr, __PHY_to1T2R_end, tmpbuf, 256);
		if (ptr) {
			PRINT_ONE(tmpbuf, "%s", 1);
		}
		else
			break;
	}
	return pos;
}
#endif // defined(RTL8192SE) && defined(MERGE_FW)



static int rtl8190_proc_stats(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{

	//struct mac80211_shared_priv *priv = (struct mac80211_shared_priv*) data;

	int pos = 0;//, idx = 0;
/*	
	unsigned int m, n, print=0;
	char tmp[32];

	PRINT_ONE("  Statistics...", "%s", 1);

	m = priv->up_time / 86400;
	n = priv->up_time % 86400;
	if (m) {
		idx += sprintf(tmp, "%d day ", m);
		print = 1;
	}
	m = n / 3600;
	n = n % 3600;
	if (m || print) {
		idx += sprintf(tmp+idx, "%d hr ", m);
		print = 1;
	}
	m = n / 60;
	n = n % 60;
	if (m || print) {
		idx += sprintf(tmp+idx, "%d min ", m);
		print = 1;
	}
	idx += sprintf(tmp+idx, "%d sec ", n);
	PRINT_SINGL_ARG("    up_time: ", tmp, "%s");

	PRINT_SINGL_ARG("    tx_packets:    ", priv->net_stats.tx_packets, "%lu");
	PRINT_SINGL_ARG("    tx_bytes:      ", priv->net_stats.tx_bytes, "%lu");
	PRINT_SINGL_ARG("    tx_retrys:     ", priv->ext_stats.tx_retrys, "%lu");
	PRINT_SINGL_ARG("    tx_fails:      ", priv->net_stats.tx_errors, "%lu");
	PRINT_SINGL_ARG("    tx_drops:      ", priv->ext_stats.tx_drops, "%lu");
	PRINT_SINGL_ARG("    rx_packets:    ", priv->net_stats.rx_packets, "%lu");
	PRINT_SINGL_ARG("    rx_bytes:      ", priv->net_stats.rx_bytes, "%lu");
	PRINT_SINGL_ARG("    rx_retrys:     ", priv->ext_stats.rx_retrys, "%lu");
	PRINT_SINGL_ARG("    rx_crc_errors: ", priv->net_stats.rx_crc_errors, "%lu");
	PRINT_SINGL_ARG("    rx_errors:     ", priv->net_stats.rx_errors, "%lu");
	PRINT_SINGL_ARG("    rx_data_drops: ", priv->ext_stats.rx_data_drops, "%lu");
	PRINT_SINGL_ARG("    rx_decache:    ", priv->ext_stats.rx_decache, "%lu");
 	PRINT_SINGL_ARG("    beacon_ok:     ", priv->ext_stats.beacon_ok, "%lu");
	PRINT_SINGL_ARG("    beacon_er:     ", priv->ext_stats.beacon_er, "%lu");
	PRINT_SINGL_ARG("    freeskb_err:   ", priv->ext_stats.freeskb_err, "%lu");
	PRINT_SINGL_ARG("    reused_skb:    ", priv->ext_stats.reused_skb, "%lu");

*/ 

	return pos;
}


/* static int rtl8190_proc_stats_clear(struct file *file, const char *buffer,
				unsigned long count, void *data)
{
	struct mac80211_shared_priv *priv = (struct mac80211_shared_priv*) data;

	//memset(&priv->net_stats, 0, sizeof(struct net_device_stats));
	//memset(&priv->ext_stats, 0, sizeof(struct extra_stats));

	return count;
}
*/

#ifdef RF_FINETUNE
static int rtl8190_proc_rfft(char *buf, char **start, off_t offset,
			int length, int *eof, void *data)
{
	struct net_device *dev = (struct net_device *)data;
	struct rtl8190_priv *priv = (struct rtl8190_priv *)dev->priv;
	int pos = 0;

	PRINT_ONE("  RF fine tune variables...", "%s", 1);

	PRINT_SINGL_ARG("    rssi: ", priv->pshare_hw->rf_ft_var.rssi_dump, "%d");
	PRINT_SINGL_ARG("    rxfifoO: ", priv->pshare_hw->rf_ft_var.rxfifoO, "%x");
	PRINT_SINGL_ARG("    raGoDownUpper: ", priv->pshare_hw->rf_ft_var.raGoDownUpper, "%d");
	PRINT_SINGL_ARG("    raGoDown20MLower: ", priv->pshare_hw->rf_ft_var.raGoDown20MLower, "%d");
	PRINT_SINGL_ARG("    raGoDown40MLower: ", priv->pshare_hw->rf_ft_var.raGoDown40MLower, "%d");
	PRINT_SINGL_ARG("    raGoUpUpper: ", priv->pshare_hw->rf_ft_var.raGoUpUpper, "%d");
	PRINT_SINGL_ARG("    raGoUp20MLower: ", priv->pshare_hw->rf_ft_var.raGoUp20MLower, "%d");
	PRINT_SINGL_ARG("    raGoUp40MLower: ", priv->pshare_hw->rf_ft_var.raGoUp40MLower, "%d");
	PRINT_SINGL_ARG("    digGoUpperLevel: ", priv->pshare_hw->rf_ft_var.digGoUpperLevel, "%d");
	PRINT_SINGL_ARG("    digGoLowerLevel: ", priv->pshare_hw->rf_ft_var.digGoLowerLevel, "%d");
	PRINT_SINGL_ARG("    rssiTx20MUpper: ", priv->pshare_hw->rf_ft_var.rssiTx20MUpper, "%d");
	PRINT_SINGL_ARG("    rssiTx20MLower: ", priv->pshare_hw->rf_ft_var.rssiTx20MLower, "%d");
	PRINT_SINGL_ARG("    rssi_expire_to: ", priv->pshare_hw->rf_ft_var.rssi_expire_to, "%d");


#if defined(RTL8192SE)
	PRINT_SINGL_ARG("    cck_pwr_max: ", priv->pshare_hw->rf_ft_var.cck_pwr_max, "%d");
#endif

	PRINT_SINGL_ARG("    tx_pwr_ctrl: ", priv->pshare_hw->rf_ft_var.tx_pwr_ctrl, "%d");

	// 11n ap AES debug
	PRINT_SINGL_ARG("    aes_check_th: ", priv->pshare_hw->rf_ft_var.aes_check_th, "%d KB");

	// Tx power tracking
	PRINT_SINGL_ARG("    tpt_period: ", priv->pshare_hw->rf_ft_var.tpt_period, "%d");

	// TXOP enlarge
	PRINT_SINGL_ARG("    txop_enlarge_upper: ", priv->pshare_hw->rf_ft_var.txop_enlarge_upper, "%d");
	PRINT_SINGL_ARG("    txop_enlarge_lower: ", priv->pshare_hw->rf_ft_var.txop_enlarge_lower, "%d");

	// 2.3G support
	PRINT_SINGL_ARG("    frq_2_3G: ", priv->pshare_hw->rf_ft_var.use_frq_2_3G, "%d");

	//Support IP multicast->unicast
#ifdef SUPPORT_TX_MCAST2UNI
	PRINT_SINGL_ARG("    mc2u_disable: ", priv->pshare_hw->rf_ft_var.mc2u_disable, "%d");
#endif

#ifdef WIFI_11N_2040_COEXIST
	PRINT_SINGL_ARG("    coexist_enable: ", priv->pshare_hw->rf_ft_var.coexist_enable, "%d");
	if (priv->pshare_hw->rf_ft_var.coexist_enable) {
		PRINT_SINGL_ARG("    bg_ap_timeout: ", priv->bg_ap_timeout, "%d");
		PRINT_SINGL_ARG("    force_20_sta: ", priv->force_20_sta, "0x%08x");
		PRINT_SINGL_ARG("    switch_20_sta: ", priv->switch_20_sta, "0x%08x");
	}
#endif

	return pos;
}
#endif



#ifdef __KERNEL__
void MDL_DEVINIT rtl8190_proc_init(struct mac80211_shared_priv *priv)
{
	struct proc_dir_entry *rtl8190_proc_root = NULL ;
//	struct proc_dir_entry *res;

	rtl8190_proc_root = proc_mkdir(wiphy_name(priv->hw->wiphy), NULL);
	priv->proc_root = rtl8190_proc_root ;
	if (rtl8190_proc_root == NULL) {
		printk("create proc root failed!\n");
		return;
	}

	if ( create_proc_read_entry ("txdesc0", 0644, rtl8190_proc_root,
			rtl8190_proc_txdesc0_info, (void *)priv) == NULL ) {
		printk("create proc txdesc0 failed!\n");
		return;
	}
	if ( create_proc_read_entry ("txdesc1", 0644, rtl8190_proc_root,
			rtl8190_proc_txdesc1_info, (void *)priv) == NULL ) {
		printk("create proc txdesc1 failed!\n");
		return;
	}
	if ( create_proc_read_entry ("txdesc2", 0644, rtl8190_proc_root,
			rtl8190_proc_txdesc2_info, (void *)priv) == NULL ) {
		printk("create proc txdesc2 failed!\n");
		return;
	}
	if ( create_proc_read_entry ("txdesc3", 0644, rtl8190_proc_root,
			rtl8190_proc_txdesc3_info, (void *)priv) == NULL ) {
		printk("create proc txdesc3 failed!\n");
		return;
	}
	if ( create_proc_read_entry ("txdesc4", 0644, rtl8190_proc_root,
			rtl8190_proc_txdesc4_info, (void *)priv) == NULL ) {
		printk("create proc txdesc4 failed!\n");
		return;
	}
	if ( create_proc_read_entry ("txdesc5", 0644, rtl8190_proc_root,
			rtl8190_proc_txdesc5_info, (void *)priv) == NULL ) {
		printk("create proc txdesc5 failed!\n");
		return;
	}
	if ( create_proc_read_entry ("rxdesc", 0644, rtl8190_proc_root,
			rtl8190_proc_rxdesc_info, (void *)priv) == NULL ) {
		printk("create proc rxdesc failed!\n");
		return;
	}
	if ( create_proc_read_entry ("desc_info", 0644, rtl8190_proc_root,
			rtl8190_proc_desc_info, (void *)priv) == NULL ) {
		printk("create proc desc_info failed!\n");
		return;
	}

	if ( (create_proc_read_entry ("stats", 0644, rtl8190_proc_root,
			rtl8190_proc_stats, (void *)priv)) == NULL ) {
		printk("create proc stats failed!\n");
		return;
	}

	if ( create_proc_read_entry ("cam_info", 0644, rtl8190_proc_root,
			rtl8190_proc_cam_info, (void *)priv) == NULL ) {
		printk("create proc cam_info failed!\n");
		return;
	}

	if ( create_proc_read_entry ("tx_pkt", 0644, rtl8190_proc_root,
			rtl8190_proc_tx_pkt, (void *)priv) == NULL ) {
		printk("create proc tx_pkt failed!\n");
		return;
	}


#ifdef RF_FINETUNE
	if ( create_proc_read_entry ("rf_finetune", 0644, rtl8190_proc_root,
			rtl8190_proc_rfft, (void *)priv) == NULL ) {
		printk("create proc rf_finetune failed!\n");
		return;
	}
#endif


//	res->write_proc = rtl8190_proc_led;
//	res->data =  (void *)dev;

#if defined(RTL8192SE) && defined(MERGE_FW)
	if ( create_proc_read_entry ("AGC_TAB.txt", 0644, rtl8190_proc_root,
			rtl8190_proc_agc_tab, (void *)priv) == NULL ) {
		printk("create proc AGC_TAB.txt failed!\n");
		return;
	}
	if ( create_proc_read_entry ("phy_reg.txt", 0644, rtl8190_proc_root,
			rtl8190_proc_phy_reg, (void *)priv) == NULL ) {
		printk("create proc phy_reg.txt failed!\n");
		return;
	}

	if ( create_proc_read_entry ("MACPHY_REG.txt", 0644, rtl8190_proc_root,
			rtl8190_proc_macphy_reg, (void *)priv) == NULL ) {
		printk("create proc MACPHY_REG.txt failed!\n");
		return;
	}
	if ( create_proc_read_entry ("radio_a.txt", 0644, rtl8190_proc_root,
			rtl8190_proc_radio_a, (void *)priv) == NULL ) {
		printk("create proc radio_a.txt failed!\n");
		return;
	}
	if ( create_proc_read_entry ("radio_b.txt", 0644, rtl8190_proc_root,
			rtl8190_proc_radio_b, (void *)priv) == NULL ) {
		printk("create proc radio_b.txt failed!\n");
		return;
	}
	if ( create_proc_read_entry ("PHY_REG_PG.txt", 0644, rtl8190_proc_root,
				rtl8190_proc_phy_reg_pg, (void *)priv) == NULL ) {
		printk("create proc PHY_REG_PG.txt failed!\n");
		return;
	}
	if ( create_proc_read_entry ("PHY_to1T2R.txt", 0644, rtl8190_proc_root,
				rtl8190_proc_phy_to1t2r, (void *)priv) == NULL ) {
		printk("create proc PHY_to1T2R.txt failed!\n");
		return;
	}
#endif // defined(RTL8192SE) && defined(MERGE_FW)
}


void __devexit rtl8190_proc_remove (struct mac80211_shared_priv *priv)
{
	struct proc_dir_entry *rtl8190_proc_root = priv->proc_root;

	if (rtl8190_proc_root != NULL) 
	{

		remove_proc_entry( "txdesc0", rtl8190_proc_root );
		remove_proc_entry( "txdesc1", rtl8190_proc_root );
		remove_proc_entry( "txdesc2", rtl8190_proc_root );
		remove_proc_entry( "txdesc3", rtl8190_proc_root );
		remove_proc_entry( "txdesc4", rtl8190_proc_root );
		remove_proc_entry( "txdesc5", rtl8190_proc_root );
		remove_proc_entry( "rxdesc", rtl8190_proc_root );
		remove_proc_entry( "desc_info", rtl8190_proc_root );
		remove_proc_entry( "stats", rtl8190_proc_root );
		remove_proc_entry( "cam_info", rtl8190_proc_root );
		remove_proc_entry( "tx_pkt", rtl8190_proc_root );

#ifdef RF_FINETUNE
		remove_proc_entry( "rf_finetune", rtl8190_proc_root );
#endif

#if defined(RTL8192SE) && defined(MERGE_FW)
		remove_proc_entry( "AGC_TAB.txt", rtl8190_proc_root );
		remove_proc_entry( "phy_reg.txt", rtl8190_proc_root );
		remove_proc_entry( "MACPHY_REG.txt", rtl8190_proc_root );
		remove_proc_entry( "radio_a.txt", rtl8190_proc_root );
		remove_proc_entry( "radio_b.txt", rtl8190_proc_root );
		remove_proc_entry( "PHY_REG_PG.txt", rtl8190_proc_root );
		remove_proc_entry( "PHY_to1T2R.txt", rtl8190_proc_root );
#endif


		remove_proc_entry( wiphy_name(priv->hw->wiphy), NULL );
		rtl8190_proc_root = NULL;
	}
	
}

#else // not __KERNEL__

struct _proc_table_
{
	char *cmd;
	int (*func)(char *buf, char **start, off_t offset,
			int length, int *eof, void *data);
};

static struct _proc_table_ proc_table[] =
{

	{"txdesc0",				rtl8190_proc_txdesc0_info},
	{"txdesc1",				rtl8190_proc_txdesc1_info},
	{"txdesc2",				rtl8190_proc_txdesc2_info},
	{"txdesc3",				rtl8190_proc_txdesc3_info},
	{"txdesc4",				rtl8190_proc_txdesc4_info},
	{"txdesc5",				rtl8190_proc_txdesc5_info},
	{"rxdesc",				rtl8190_proc_rxdesc_info},
	{"desc_info",			rtl8190_proc_desc_info},
	{"stats",				rtl8190_proc_stats},
	{"cam_info",			rtl8190_proc_cam_info},
};

#define NUM_CMD_TABLE_ENTRY		(sizeof(proc_table) / sizeof(struct _proc_table_))

/*
void rtl8190_proc_debug(struct net_device *dev, char *cmd)
{
	int i, j, eof, len;
	char *tmpbuf, *start;

	start = tmpbuf = (char *)kmalloc(4096, 0);
	for (i=0; i<NUM_CMD_TABLE_ENTRY; i++) {
		if (!strcmp(cmd, proc_table[i].cmd)) {
			memset(tmpbuf, 0, 4096);
			len = proc_table[i].func(tmpbuf, &start, 0, 4096, &eof, dev);
			for(j=0; j<len; j++)
				printk("%c", tmpbuf[j]);
		}
	}
	kfree(tmpbuf);
}
*/
#endif // __KERNEL__




/*
 *  Header file defines some common inline funtions
 *
 *  $Id: 8190n_util.h,v 1.1.1.1 2010/05/05 09:00:44 jiunming Exp $
 *
 *  Copyright (c) 2009 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _8190N_UTIL_H_
#define _8190N_UTIL_H_
#include <linux/module.h>
#ifdef __KERNEL__
#ifdef RTL8192SU
#include <linux/usb.h>
#else
#include <linux/pci.h>
#endif
#include <linux/spinlock.h>
#include <linux/circ_buf.h>
#endif

#include "./8190n_cfg.h"
#include "./8190n.h"
#include "./wifi.h"
#include "./8190n_hw.h"

#ifdef CONFIG_RTK_MESH
#include "./mesh_ext/mesh_util.h"
#endif

#ifdef GREEN_HILL
#define	SAVE_INT_AND_CLI(x)		{ x = save_and_cli(); }
#define RESTORE_INT(x)			restore_flags(x)
#else
#ifdef NEW_MAC80211_DRV
#define SAVE_INT_AND_CLI(__x__)		spin_lock_irqsave(&priv->pshare_hw->lock, (__x__))
#define RESTORE_INT(__x__)			spin_unlock_irqrestore(&priv->pshare_hw->lock, (__x__))
#else
#define SAVE_INT_AND_CLI(__x__)		spin_lock_irqsave(&priv->pshare->lock, (__x__))
#define RESTORE_INT(__x__)			spin_unlock_irqrestore(&priv->pshare->lock, (__x__))
#endif
#endif

#ifdef __LINUX_2_6__
/*
#ifdef virt_to_bus
	#undef virt_to_bus
	#define virt_to_bus			CPHYSADDR
#endif*/
#endif


#ifdef RTL8192SU
#define GET_PRIV_IOADDR	
#else
#define GET_PRIV_IOADDR \
	ULONG ioaddr = priv->pshare_hw->ioaddr
#endif

#ifdef USE_IO_OPS

#define get_desc(val)           (val)
#define set_desc(val)           (val)

#define RTL_R8(reg)             inb(((unsigned long)ioaddr) + (reg))
#define RTL_R16(reg)            inw(((unsigned long)ioaddr) + (reg))
#define RTL_R32(reg)            ((unsigned long)inl(((unsigned long)ioaddr) + (reg)))
#define RTL_W8(reg, val8)       outb((val8), ((unsigned long)ioaddr) + (reg))
#define RTL_W16(reg, val16)     outw((val16), ((unsigned long)ioaddr) + (reg))
#define RTL_W32(reg, val32)     outl((val32), ((unsigned long)ioaddr) + (reg))
#define RTL_W8_F                RTL_W8
#define RTL_W16_F               RTL_W16
#define RTL_W32_F               RTL_W32
#undef readb
#undef readw
#undef readl
#undef writeb
#undef writew
#undef writel
#define readb(addr)             inb((unsigned long)(addr))
#define readw(addr)             inw((unsigned long)(addr))
#define readl(addr)             inl((unsigned long)(addr))
#define writeb(val,addr)        outb((val), (unsigned long)(addr))
#define writew(val,addr)        outw((val), (unsigned long)(addr))
#define writel(val,addr)        outl((val), (unsigned long)(addr))

#elif defined(NEW_MAC80211_DRV)

#ifdef RTL8192SU
#define RTL_R8(reg)		read8(priv, reg)
#define RTL_R16(reg)            read16(priv, reg)
#define RTL_R32(reg)            read32(priv, reg)
#define RTL_W8(reg, val8)       write8(priv, reg, val8)
#define RTL_W16(reg, val16)     write16(priv, reg, val16)
#define RTL_W32(reg, val32)     write32(priv, reg, val32)

#define RTL_W8_ASYNC(reg, val8) write32_async(priv, reg, val8)
#define RTL_W16_ASYNC(reg, val16) write32_async(priv, reg, val16)
#define RTL_W32_ASYNC(reg, val32) write32_async(priv, reg, val32)

#define RTL_WRITE_BB(reg, val32)	phy_SetUsbBBReg(priv, reg, val32)
#define RTL_READ_BB(reg)	phy_QueryUsbBBReg(priv, reg)

#else
#define RTL_R8(reg)             ioread8(((void *)ioaddr) + (reg))
#define RTL_R16(reg)            ioread16(((void *)ioaddr) + (reg))
#define RTL_R32(reg)            ioread32(((void *)ioaddr) + (reg))
#define RTL_W8(reg, val8)       iowrite8(((u8)val8), ((void *)ioaddr) + (reg))
#define RTL_W16(reg, val16)     iowrite16(((u16)val16), ((void *)ioaddr) + (reg))
#define RTL_W32(reg, val32)     iowrite32(((u32)val32), ((void *)ioaddr) + (reg))

#define RTL_W8_ASYNC	RTL_W8
#define RTL_W16_ASYNC	RTL_R16
#define RTL_W32_ASYNC	RTL_W32

#define RTL_WRITE_BB	RTL_W32
#define RTL_READ_BB RTL_R32


#endif
#define RTL_W8_F                RTL_W8
#define RTL_W16_F               RTL_W16
#define RTL_W32_F               RTL_W32


#define get_desc(val)	(le32_to_cpu(val))
#define set_desc(val)	(cpu_to_le32(val))



#else // !USE_IO_OPS

#define PAGE_NUM 15

#ifdef __LINUX_2_6__
	#define IO_TYPE_CAST	(unsigned char *)
#else
	#define IO_TYPE_CAST	(unsigned int)
#endif

extern unsigned char rfc1042_header[WLAN_LLC_HEADER_SIZE];

static __inline__ unsigned char RTL_R8_F(struct rtl8190_priv *priv, unsigned long ioaddr, unsigned int reg)
{
	unsigned char val8 = 0;
#if defined(USE_RTL8186_SDK) && !defined(CONFIG_RTL8196B)
	int swap[4]={3,2,1,0};
	int diff = swap[reg&0x3];
#endif

#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
#if defined(USE_RTL8186_SDK) && !defined(CONFIG_RTL8196B)
		val8 = readb(IO_TYPE_CAST(ioaddr + (((reg&~3)+diff) & 0x000000ff)));
#else
		val8 = readb(IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
#endif
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
#if defined(USE_RTL8186_SDK) && !defined(CONFIG_RTL8196B)
		val8 = readb(IO_TYPE_CAST(ioaddr + ((reg&~3)+diff)));
#else
		val8 = readb(IO_TYPE_CAST(ioaddr + reg));
#endif
	}

	return val8;
}

static __inline__ unsigned short RTL_R16_F(struct rtl8190_priv *priv, unsigned long ioaddr, unsigned int reg)
{
	unsigned short val16 = 0;
#if defined(USE_RTL8186_SDK) && !defined(CONFIG_RTL8196B)
	int diff = 2-(reg&0x3);
#endif

#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
#if defined(USE_RTL8186_SDK) && !defined(CONFIG_RTL8196B)
		val16 = readw(IO_TYPE_CAST(ioaddr + (((reg&~3) + diff) & 0x000000ff)));
#else
		val16 = readw(IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
#endif
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
#if defined(USE_RTL8186_SDK) && !defined(CONFIG_RTL8196B)
		val16 = readw(IO_TYPE_CAST(ioaddr + ((reg&~3) + diff)));
#else
		val16 = readw(IO_TYPE_CAST(ioaddr + reg));
#endif
	}

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val16 = le16_to_cpu(val16);
#endif

	return val16;
}

static __inline__ unsigned int RTL_R32_F(struct rtl8190_priv *priv, unsigned long ioaddr, unsigned int reg)
{
	unsigned int val32 = 0;

#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		val32 = readl(IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		val32 = readl(IO_TYPE_CAST(ioaddr + reg));
	}

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val32 = le32_to_cpu(val32);
#endif

	return val32;
}

static __inline__ void RTL_W8_F(struct rtl8190_priv *priv, unsigned long ioaddr, unsigned int reg, unsigned char val8)
{
#ifdef IO_MAPPING
	unsigned char page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		writeb(val8, IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		writeb(val8, IO_TYPE_CAST(ioaddr + reg));
	}
}

static __inline__ void RTL_W16_F(struct rtl8190_priv *priv, unsigned long ioaddr, unsigned int reg, unsigned short val16)
{
	unsigned short val16_n = val16;
#ifdef IO_MAPPING
	unsigned char page;
#endif

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val16_n = cpu_to_le16(val16);
#endif

#ifdef IO_MAPPING
	page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		writew(val16_n, IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		writew(val16_n, IO_TYPE_CAST(ioaddr + reg));
	}
}

static __inline__ void RTL_W32_F(struct rtl8190_priv *priv, unsigned long ioaddr, unsigned int reg, unsigned int val32)
{
	unsigned int val32_n = val32;
#ifdef IO_MAPPING
	unsigned char page;
#endif

#ifdef CHECK_SWAP
	if (priv->pshare->type & ACCESS_SWAP_IO)
		val32_n = cpu_to_le32(val32);
#endif

#ifdef IO_MAPPING
	page = ((unsigned char)(reg >> 8)) & PAGE_NUM;
	if (priv->pshare->io_mapping && page)
	{
		unsigned long x;
		SAVE_INT_AND_CLI(x);

		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) | page, IO_TYPE_CAST(ioaddr + _PSR_));
		writel(val32_n, IO_TYPE_CAST(ioaddr + (reg & 0x000000ff)));
		writeb(readb(IO_TYPE_CAST(ioaddr + _PSR_)) & (~PAGE_NUM), IO_TYPE_CAST(ioaddr + _PSR_));

		RESTORE_INT(x);
	}
	else
#endif
	{
		writel(val32_n, IO_TYPE_CAST(ioaddr + reg));
	}
}

#define RTL_R8(reg)		\
	(RTL_R8_F(priv, ioaddr, reg))

#define RTL_R16(reg)	\
	(RTL_R16_F(priv, ioaddr, reg))

#define RTL_R32(reg)	\
	(RTL_R32_F(priv, ioaddr, reg))

#define RTL_W8(reg, val8)	\
	do { \
		RTL_W8_F(priv, ioaddr, reg, val8); \
	} while (0)

#define RTL_W16(reg, val16)	\
	do { \
		RTL_W16_F(priv, ioaddr, reg, val16); \
	} while (0)

#define RTL_W32(reg, val32)	\
	do { \
		RTL_W32_F(priv, ioaddr, reg, val32) ; \
	} while (0)

#ifdef CHECK_SWAP
#define get_desc(val)	((priv->pshare->type & ACCESS_SWAP_MEM) ? le32_to_cpu(val) : val)
#define set_desc(val)	((priv->pshare->type & ACCESS_SWAP_MEM) ? cpu_to_le32(val) : val)
#else
#define get_desc(val)	(val)
#define set_desc(val)	(val)
#endif

#endif // USE_IO_OPS


#define get_tofr_ds(pframe)	((GetToDs(pframe) << 1) | GetFrDs(pframe))

#define is_qos_data(pframe)	((GetFrameSubType(pframe) & (WIFI_DATA_TYPE | BIT(7))) == (WIFI_DATA_TYPE | BIT(7)))

#define UINT32_DIFF(a, b)		((a >= b)? (a - b):(0xffffffff - b + a + 1))

static __inline__ struct list_head *dequeue_frame(struct mac80211_shared_priv *priv, struct list_head *head)
{
	unsigned long flags;
	struct list_head *pnext;

	if (list_empty(head))
		return (void *)NULL;

	SAVE_INT_AND_CLI(flags);

	pnext = head->next;
	list_del_init(pnext);

	RESTORE_INT(flags);

	return pnext;
}

static __inline__ int wifi_mac_hash(unsigned char *mac)
{
	unsigned long x;

	x = mac[0];
	x = (x << 2) ^ mac[1];
	x = (x << 2) ^ mac[2];
	x = (x << 2) ^ mac[3];
	x = (x << 2) ^ mac[4];
	x = (x << 2) ^ mac[5];

	x ^= x >> 8;

	return x & (NUM_STAT - 1);
}

static __inline__ struct rx_frinfo *get_pfrinfo(struct sk_buff *pskb)
{
	return (struct rx_frinfo *)((unsigned long)(pskb->data) - sizeof(struct rx_frinfo));
}

static __inline__ struct sk_buff *get_pskb(struct rx_frinfo *pfrinfo)
{
	return (pfrinfo->pskb);
}

static __inline__ UINT8 *get_pframe(struct rx_frinfo *pfrinfo)
{
	return (UINT8 *)((UINT)(pfrinfo->pskb->data));
}

static __inline__ UINT8	get_hdrlen(struct rtl8190_priv *priv, UINT8 *pframe)
{
	if (GetFrameType(pframe) == WIFI_DATA_TYPE)
	{
#ifdef CONFIG_RTK_MESH
		if ((get_tofr_ds(pframe) == 0x03) && ( (GetFrameSubType(pframe) == WIFI_11S_MESH) || (GetFrameSubType(pframe) == WIFI_11S_MESH_ACTION)))
		{
			if(GetFrameSubType(pframe) == WIFI_11S_MESH)  // DATA frame, qos might be on (TRUE on 8186)
			{
					return WLAN_HDR_A4_QOS_LEN;
			} // WIFI_11S_MESH
			else // WIFI_11S_MESH_ACTION frame, although qos flag is on, the qos field(2bytes) is not used for 8186
			{
				if(is_mesh_6addr_format_without_qos(pframe)) {
					return WLAN_HDR_A6_MESH_DATA_LEN;
				} else {
					return WLAN_HDR_A4_MESH_DATA_LEN;
				}
			}
		} // end of get_tofr_ds == 0x03 & (MESH DATA or MESH ACTION)
		else
#endif // CONFIG_RTK_MESH
		if (is_qos_data(pframe)) {
			if (get_tofr_ds(pframe) == 0x03)
				return WLAN_HDR_A4_QOS_LEN;
#ifdef A4_TUNNEL
			else if (priv->pmib->miscEntry.a4tnl_enable)
				return WLAN_HDR_A4_QOS_LEN;
#endif
			else
				return WLAN_HDR_A3_QOS_LEN;
		}
		else {
			if (get_tofr_ds(pframe) == 0x03)
				return WLAN_HDR_A4_LEN;
#ifdef A4_TUNNEL
			else if (priv->pmib->miscEntry.a4tnl_enable)
				return WLAN_HDR_A4_LEN;
#endif
			else
				return WLAN_HDR_A3_LEN;
		}
	}
	else if (GetFrameType(pframe) == WIFI_MGT_TYPE)
		return 	WLAN_HDR_A3_LEN;
	else if (GetFrameType(pframe) == WIFI_CTRL_TYPE)
	{
		if (GetFrameSubType(pframe) == WIFI_PSPOLL)
			return 16;
		else if (GetFrameSubType(pframe) == WIFI_BLOCKACK_REQ)
			return 16;
		else if (GetFrameSubType(pframe) == WIFI_BLOCKACK)
			return 16;
		else
		{
#ifdef _DEBUG_RTL8190_
			printk("unallowed control pkt type! 0x%04X\n", GetFrameSubType(pframe));
#endif
			return 0;
		}
	}
	else
	{
#ifdef _DEBUG_RTL8190_
		printk("unallowed pkt type! 0x%04X\n", GetFrameType(pframe));
#endif
		return 0;
	}
}

#ifdef CONFIG_NET_PCI
#define IS_PCIBIOS_TYPE		(((priv->pshare_hw->type >> TYPE_SHIFT) & TYPE_MASK) == TYPE_PCI_BIOS)
#endif

#define rtl_atomic_inc(ptr_atomic_t)	atomic_inc(ptr_atomic_t)
#define rtl_atomic_dec(ptr_atomic_t)	atomic_dec(ptr_atomic_t)
#define rtl_atomic_read(ptr_atomic_t)	atomic_read(ptr_atomic_t)
#define rtl_atomic_set(ptr_atomic_t, i)	atomic_set(ptr_atomic_t,i)

enum _skb_flag_ {
	_SKB_TX_ = 1,
	_SKB_RX_ = 2,
	_SKB_RX_IRQ_ = 4,
	_SKB_TX_IRQ_ = 8
};

#ifdef RTL8190_FASTEXTDEV_FUNCALL
/*
	For Fast Extension Device module, we need to distinguish "Normal socket buffer allocation"
	( use original function call : rtl_dev_alloc_skb() )
	and "Socket buffer allocation for RX Ring" ( use new this new function call : rtl_dev_alloc_rxRing_skb() ).
	It is because we have some additional processes for socket buffer allocated for RX Ring.
*/
static __inline__ struct sk_buff *rtl_dev_alloc_rxRing_skb(	struct rtl8190_priv *priv,
															unsigned int length,
															int flag,
															unsigned int rxRingIdx,
															struct net_device *dev)
{
	struct sk_buff *skb = NULL;

	skb = rtl865x_extDev_alloc_skb(	length,
									RTL865X_EXTDEV_USED_SKB_HEADROOM,
									rxRingIdx,
									dev);

#ifdef ENABLE_RTL_SKB_STATS
	if (flag & (_SKB_TX_ | _SKB_TX_IRQ_))
		rtl_atomic_inc(&priv->rtl_tx_skb_cnt);
	else
		rtl_atomic_inc(&priv->rtl_rx_skb_cnt);
#endif

	return skb;
}
#endif /* RTL8190_FASTEXTDEV_FUNCALL */

// Allocate net device socket buffer
static __inline__ struct sk_buff *rtl_dev_alloc_skb(struct mac80211_shared_priv *priv,
				unsigned int length, int flag, int could_alloc_from_kerenl)
{
	struct sk_buff *skb = NULL;
	
	//chris: currently alloc skbuff from kernel. May modify this function to alloc private skb.

	skb = dev_alloc_skb(length);

	return skb;
}

#ifdef RTL8190_FASTEXTDEV_FUNCALL
/*
	For Fast Extension Device module, we need to distinguish "Normal socket buffer release"
	( use original function call : rtl_kfree_skb() )
	and "Socket buffer release including RX ring socket buffer release" ( use new this new function call : rtl_actually_kfree_skb() ).
	It is because we have some additional processes for socket buffer free for RX Ring.
*/
static __inline__ void rtl_actually_kfree_skb(struct rtl8190_priv *priv, struct sk_buff *skb, struct net_device *dev, int flag)
{
#ifdef ENABLE_RTL_SKB_STATS
	if (flag & (_SKB_TX_ | _SKB_TX_IRQ_))
		rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
	else
		rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif

#ifdef RTL8190_FASTEXTDEV_FUNCALL
	/* Always release this socket buffer, even if this socket buffer is from RX Ring */
	{
		unsigned short skbType;
		unsigned short skbOwner;

		rtl865x_extDev_getSkbProperty(skb, &skbType, &skbOwner, NULL);
		if ((skbType == RTL865X_TYPE_RXRING) && (skbOwner == RTL865X_DRIVER_OWN)) {
			rtl865x_extDev_rxRunoutTxPending(skb, dev);
		}
		else {
			rtl865x_extDev_kfree_skb(skb, TRUE);
		}
	}
#else
	dev_kfree_skb_any(skb);
#endif
}
#endif /* RTL8190_FASTEXTDEV_FUNCALL */

// Free net device socket buffer
static __inline__ void rtl_kfree_skb(struct mac80211_shared_priv *priv, struct sk_buff *skb, int flag)
{
#ifdef ENABLE_RTL_SKB_STATS
	if (flag & (_SKB_TX_ | _SKB_TX_IRQ_))
		rtl_atomic_dec(&priv->rtl_tx_skb_cnt);
	else
		rtl_atomic_dec(&priv->rtl_rx_skb_cnt);
#endif

#ifdef RTL8190_FASTEXTDEV_FUNCALL
	/* Do NOT actually free Fast Extension Device's mbuf if it is from RX Ring */
	rtl865x_extDev_kfree_skb(skb, FALSE);
#else
	dev_kfree_skb_any(skb);
#endif
}

static __inline__ int is_CCK_rate(unsigned char rate)
{
	if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
		return TRUE;
	else
		return FALSE;
}


static __inline__ int is_MCS_rate(unsigned char rate)
{
	if (rate & 0x80)
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_MCS_1SS_rate(unsigned char rate)
{
	if ((rate & 0x80) && (rate < _MCS8_RATE_))
		return TRUE;
	else
		return FALSE;
}

static __inline__ int is_MCS_2SS_rate(unsigned char rate)
{
	if ((rate & 0x80) && (rate > _MCS7_RATE_))
		return TRUE;
	else
		return FALSE;
}
#ifndef RTL8192SU
static __inline__ void rtl_cache_sync_wback(struct mac80211_shared_priv *priv, unsigned int start,
				unsigned int size, int direction)
{
/*#ifdef __LINUX_2_6__
#ifndef CONFIG_X86	//joshua
		start = CPHYSADDR(start);
#endif
#endif
#ifdef CONFIG_NET_PCI
		if (IS_PCIBIOS_TYPE) {
#ifdef __LINUX_2_6__*/
			pci_dma_sync_single_for_cpu(priv->pshare_hw->pdev, start, size, direction);
/*#else
			pci_dma_sync_single(priv->pshare->pdev, start, size, direction);
#endif
		}
		else
			dma_cache_wback_inv((unsigned long)bus_to_virt(start), size);
#else
		dma_cache_wback_inv((unsigned long)bus_to_virt(start), size);
#endif
*/

}


static __inline__ unsigned long get_physical_addr(struct mac80211_shared_priv *priv, void *ptr,
				unsigned int size, int direction)
{
//#if defined(CONFIG_NET_PCI) && !defined(USE_RTL8186_SDK)
	//if (IS_PCIBIOS_TYPE)
		return pci_map_single(priv->pshare_hw->pdev, ptr, size, direction);
//	else
//#endif
	//	return virt_to_bus(ptr);
}
#endif

static __inline__ int get_rf_mimo_mode(struct mac80211_shared_priv *priv)
{
/*
#if defined(RTL8190) || defined(RTL8192E)
	if (GET_HW(priv)->MIMO_TR_hw_support == MIMO_1T2R) {
		if (GET_MIB(priv)->dot11RFEntry.MIMO_TR_mode == MIMO_1T1R)
			return MIMO_1T1R;
		else
			return MIMO_1T2R;
	}
	else {	// 2 rtl8256 chips on board
		if (GET_MIB(priv)->dot11RFEntry.MIMO_TR_mode == MIMO_1T1R)
			return MIMO_1T1R;
		else if (GET_MIB(priv)->dot11RFEntry.MIMO_TR_mode == MIMO_1T2R)
			return MIMO_1T2R;
		else if (GET_MIB(priv)->dot11RFEntry.MIMO_TR_mode == MIMO_2T2R)
			return MIMO_2T2R;
		else
			return MIMO_2T4R;
	}
#elif defined(RTL8192SE)
	if (GET_MIB(priv)->dot11RFEntry.MIMO_TR_mode == MIMO_1T2R)
		return MIMO_1T2R;
	else if(GET_MIB(priv)->dot11RFEntry.MIMO_TR_mode == MIMO_1T1R)
		return MIMO_1T1R;
	else
		return MIMO_2T2R;
#endif
*/
	return MIMO_2T2R;
}


static __inline__ void tx_sum_up(struct rtl8190_priv *priv, struct stat_info *pstat, int pktlen)
{
	struct net_device_stats *pnet_stats;

	if (priv) {
		pnet_stats = &(priv->net_stats);
		pnet_stats->tx_packets++;
		pnet_stats->tx_bytes += pktlen;

		priv->share_priv->ext_stats.tx_byte_cnt += pktlen;

		// bcm old 11n chipset iot debug, and TXOP enlarge
		priv->pshare->current_tx_bytes += pktlen;
	}

	if (pstat) {
		pstat->tx_pkts++;
		pstat->tx_bytes += pktlen;
		pstat->tx_byte_cnt += pktlen;
	}
}


static __inline__ void rx_sum_up(struct rtl8190_priv *priv, struct stat_info *pstat, int pktlen, int retry)
{
	struct net_device_stats *pnet_stats;

	if (priv) {
		pnet_stats = &(priv->net_stats);
		pnet_stats->rx_packets++;
		pnet_stats->rx_bytes += pktlen;

		if (retry)
			priv->share_priv->ext_stats.rx_retrys++;

		priv->share_priv->ext_stats.rx_byte_cnt += pktlen;

		// bcm old 11n chipset iot debug
		priv->pshare->current_rx_bytes += pktlen;
	}

	if (pstat) {
		pstat->rx_pkts++;
		pstat->rx_bytes += pktlen;
		pstat->rx_byte_cnt += pktlen;
	}
}


static __inline__ unsigned char get_cck_swing_idx(unsigned int bandwidth, unsigned char ofdm_swing_idx)
{
	unsigned char cck_swing_idx;

	if (bandwidth == HT_CHANNEL_WIDTH_20) {
		if (ofdm_swing_idx >= TxPwrTrk_CCK_SwingTbl_Len)
			cck_swing_idx = TxPwrTrk_CCK_SwingTbl_Len - 1;
		else
			cck_swing_idx = ofdm_swing_idx;
	}
	else {	// 40M bw
		if (ofdm_swing_idx < 12)
			cck_swing_idx = 0;
		else if (ofdm_swing_idx > (TxPwrTrk_CCK_SwingTbl_Len - 1 + 12))
			cck_swing_idx = TxPwrTrk_CCK_SwingTbl_Len - 1;
		else
			cck_swing_idx = ofdm_swing_idx - 12;
	}

	return cck_swing_idx;
}



#define CIRC_CNT_RTK(head,tail,size)	((head>=tail)?(head-tail):(size-tail+head))
#define CIRC_SPACE_RTK(head,tail,size)	CIRC_CNT_RTK((tail),((head)+1),(size))


#endif // _8190N_UTIL_H_



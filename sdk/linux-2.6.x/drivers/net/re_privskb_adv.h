/*	
 *	re_privskb.h
*/

#ifndef _RE_PRIVSKB_H_
#define _RE_PRIVSKB_H_
#include <linux/skbuff.h>
#ifdef CONFIG_RTL8686
#ifdef CONFIG_ARCH_LUNA_SLAVE
#include "re8686_dsp.h"
#else
#if defined(CONFIG_RTL9600_SERIES)
#include "re8686.h"
#endif
#if defined(CONFIG_RTL9601B_SERIES)
#include "re8686_rtl9601b.h"
#endif
#if defined(CONFIG_RTL9602C_SERIES)
#include "re8686_rtl9602c.h"
#endif
#endif
#else
#include "re867x.h"
#endif

#if defined(CONFIG_DUALBAND_CONCURRENT)
#define MBUF_LEN	1800
#else
#define MBUF_LEN        1600
#endif
#define CROSS_LAN_MBUF_LEN		(MBUF_LEN+16)

#if defined(CONFIG_RTL865X_ETH_PRIV_SKB) || defined(CONFIG_RTL865X_ETH_PRIV_SKB_ADV)
#ifdef DELAY_REFILL_ETH_RX_BUF
#define MAX_PRE_ALLOC_RX_SKB	64
#define MAX_ETH_SKB_NUM	(RE8670_RX_RING_SIZE + MAX_PRE_ALLOC_RX_SKB + 600)
#elif defined(CONFIG_DUALBAND_CONCURRENT) && defined(CONFIG_ARCH_LUNA_SLAVE)
#define MAX_PRE_ALLOC_RX_SKB	160
#define MAX_ETH_SKB_NUM	(RE8670_RX_RING_SIZE + MAX_PRE_ALLOC_RX_SKB + 400)
#else
#define MAX_PRE_ALLOC_RX_SKB    160
#define MAX_ETH_SKB_NUM ((RE8670_RX_RING_SIZE) + ((MAX_PRE_ALLOC_RX_SKB + 400) * 2))
#endif  //end DELAY_REFILL_ETH_RX_BUF

#define ETH_PRIV_SKB_PROC	1
#endif  //end CONFIG_RTL865X_ETH_PRIV_SKB

#if 0 /* 2012-3-12 krammer add */
#define ETH_SKB_BUF_SIZE	(CROSS_LAN_MBUF_LEN+sizeof(struct skb_shared_info)+128)
#else
#define ETH_SKB_BUF_SIZE	(CROSS_LAN_MBUF_LEN+128+NET_SKB_PAD+sizeof(struct skb_shared_info)+128)
#endif /* 2012-3-12 krammer add */
#define SKB_ALIGNED_SIZE	((sizeof(struct sk_buff) + 0x0f) & ~0x0f)
#define ETH_SKB_DATA_LEN	(CROSS_LAN_MBUF_LEN+128+NET_SKB_PAD)

#define RX_OFFSET		2


void init_priv_eth_skb_buf(void);
struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size);
void rtl865x_free_eth_priv_buf(struct sk_buff *skb, unsigned int flag);

#endif /*_RE_PRIVSKB_H_*/

/*
 *	ra_debug.h
 */

#include <linux/atm.h>

extern struct SAR_IOCTL_DEBUG dbg_cfg;

#if 1
void skb_pvc_debug(const struct sk_buff *skb, int ch_no, int flag);
void cell_debug(const void *pcell, int enable, int flag);
void skb_oamaal5_debug(const struct sk_buff *skb, int enable, int flag);
#else
#define skb_pvc_debug(skb, ch_no, flag)	do {} while (0)
#define cell_debug(pcell, enable, flag)	do {} while (0)
#define skb_oamaal5_debug(skb, enable,  flag)	do {} while (0)
#endif

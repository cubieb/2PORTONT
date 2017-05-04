/* rtk_fastbridge.h: define fast bridge parameters*/

#include <linux/autoconf.h>

#ifdef CONFIG_RTL8672_BRIDGE_FASTPATH
#include <linux/netdevice.h>
#include <linux/skbuff.h>

enum {DIR_LAN, DIR_WAN};

int brgShortcutProcess(struct sk_buff *skb, struct net_device *srcDev, int dir);
void brgShortcutLearnDestItf(struct sk_buff *skb, struct net_device *dstDev);

int brgUpdateDstdev(struct net_device *dst, unsigned char *mac);
int brgFind(struct net_device *dst, unsigned char *mac, unsigned long *tick);
void brgClearTableByDev(struct net_device *dev);
void brgClearTable(void);
int brgScFind(struct net_device *dst, unsigned char *mac, unsigned long *tick);
int brgScDelete(unsigned char *mac);
void brgEntryDelete(unsigned short *s_mac,unsigned short *d_mac,int dir);
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB_ADV
int brgFastForwarding(struct sk_buff *skb, int dir);
#endif 

extern int BRG_ENTRY_AGING_TIME;


#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)||defined(CONFIG_RTK_ETHUP)
extern int qos_enable;
#endif
#define BRG_ENTRY_AGING_TIME_NORMAL		200
#define BRG_ENTRY_AGING_TIME_HEAVY      100000



#endif



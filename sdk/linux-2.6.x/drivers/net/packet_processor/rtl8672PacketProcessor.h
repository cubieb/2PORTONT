
#include <linux/atm.h>
#include <linux/atmdev.h>
extern int enable_port_mapping;
extern int enable_vlan_grouping;
extern struct sk_buff *pp_sar_send_vlan(int ch_no, struct sk_buff *skb);
#define READWD(addr) ((unsigned char *)addr)[0]<<24 | ((unsigned char *)addr)[1]<<16 | ((unsigned char *)addr)[2]<<8 | ((unsigned char *)addr)[3]

struct mac_private {
	int tx_intf;
	struct net_device	*dev;
	struct net_device_stats stats;
	spinlock_t		lock;
};

extern struct net_device *mac_dev[2];
int start_pp_nic_rx(struct net_device* dev);
int stop_pp_nic_rx (struct net_device *dev);
int rtl8672_pp_set_mac_addr(struct net_device *dev, void *p);
int rtl8672_mac_vtx_start_xmit (struct sk_buff *skb, struct net_device *dev);
struct net_device_stats *rtl8672_mac_get_stats (struct net_device *dev);
irqreturn_t mac_intHandler(int irq, void *dev_instance);

#ifdef CONFIG_RTL8681_PTM
extern struct net_device *ptmPP_dev;
int start_pp_ptm_rx(struct net_device* dev);
int stop_pp_ptm_rx (struct net_device *dev);
int rtl8672_ptm_vtx_start_xmit (struct sk_buff *skb, struct net_device *dev);
void ppptm_rx_bh(unsigned long cp);
#endif

int SARvtx(struct atm_vcc *vcc, struct sk_buff* skb, unsigned int len, int port);
int set_sar_intf(int channel, int mode, int encap);
extern int index_dev2ch(struct net_device* dev);
extern int free_cnt;
extern int enable_ipqos;
extern int rtl8672_L2EntryTimeout(void);
extern int rtl8672_L4EntryTimeout(void);

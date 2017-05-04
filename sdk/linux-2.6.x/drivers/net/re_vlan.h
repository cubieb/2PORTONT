#ifndef	__RE_VLAN__
#define __RE_VLAN__	"re_vlan: "
#define RE_VLAN_PROC	"re_vlan"

//#define RE_VLAN_DBG	1
#ifdef RE_VLAN_DBG
#define _RE_VLAN_DBG printk
#define _RE_VLAN_DBG_FUNC_LINE printk("%s %d\n", __func__, __LINE__)
#define RE_VLAN_MEM_DBG
#else //RE_VLAN_DBG
#define _RE_VLAN_DBG
#define _RE_VLAN_DBG_FUNC_LINE
#endif //RE_VLAN_DBG

#ifdef RE_VLAN_MEM_DBG
#define _RE_VLAN_DEG_MEM_ADD(sum, size) (sum)+=(size)
#define _RE_VLAN_DEG_MEM_DEC(sum, size) (sum)-=(size)
#else //RE_VLAN_MEM_DBG
#define _RE_VLAN_DEG_MEM_ADD(sum, size)
#define _RE_VLAN_DEG_MEM_DEC(sum, size)
#endif //RE_VLAN_MEM_DBG


#include <linux/netdevice.h>

enum re_vlan_flags_bitmap {
	RE_VLAN_CHECK_ENABLE = (1 << 0),
	RE_VLAN_TX_RX_DBG_ENABLE = (1 << 1),
	RE_VLAN_TX_RX_DUMP_ENABLE = (1 << 2)
};

#define RE_VLAN_CHECK_ENABLED (re_vlan_flags & RE_VLAN_CHECK_ENABLE)
#define RE_VLAN_TX_RX_DBG_ENABLED (re_vlan_flags & RE_VLAN_TX_RX_DBG_ENABLE)
#define RE_VLAN_TX_RX_DUMP_ENABLED (re_vlan_flags & RE_VLAN_TX_RX_DUMP_ENABLE)

#define RE_VLAN_SET_FLAGS(bit) (re_vlan_flags |= bit)
#define RE_VLAN_CLEAR_FLAGS(bit) (re_vlan_flags &= ~bit)

enum re_vlan_check_vid {
	RE_VLAN_CHECK_LANID,
	RE_VLAN_CHECK_WANID,
	RE_VLAN_CHECK_BOTH_LANID_WANID
};

enum re_vlan_ioctl_cmds {
	RE_VLAN_ADD_VLANPAIR_CMD = 1,
	RE_VLAN_DEL_VLANPAIR_CMD,
	RE_VLAN_DEL_VLANDEV_CMD,
	RE_VLAN_SHOW_ALL_CMD,
	RE_VLAN_CLEAN_UP_CMD
};

struct re_vlan_ioctl_args {
	enum re_vlan_ioctl_cmds cmd;
	char ifname[IFNAMSIZ];
	unsigned short lanVid;
	unsigned short wanVid;
};

struct re_vlan_pair {
	char ifname[IFNAMSIZ];
	unsigned short lanVid;
	unsigned short wanVid;
	struct list_head lan_pair_list;
	struct list_head wan_pair_list;
};

struct re_vlan_dev {
	struct net_device *dev;
	unsigned short vid;
	struct list_head vlan_pair_list;
	struct list_head dev_list;
};

extern int re_vlan_loaded(void);
extern int re_vlan_skb_recv(struct sk_buff *skb);
extern int re_vlan_skb_xmit(struct sk_buff *skb, unsigned short *vid);
extern void re_vlan_untag(struct sk_buff *skb);
extern int re_vlan_addtag(struct sk_buff *skb, unsigned short vid);
#endif

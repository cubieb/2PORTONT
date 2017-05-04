/***************************************************************************
 * File Name    : rtk_rg_wmux.h
 * Description  : wmux mean wan mux.
 ***************************************************************************/
#ifndef RTK_RG_WMUX_H
#define RTK_RG_WMUX_H

//extern unsigned int pvid_per_port[RTL8651_PORT_NUMBER+3];

//#define UNIQUE_MAC_PER_DEV
#undef UNIQUE_MAC_PER_DEV

#define SET_MODULE_OWNER(dev) do { } while (0)

//static int wmux_device_event(struct notifier_block *, unsigned long, void *);
#if defined(CONFIG_OPENWRT_RG)
#define WMUX_DEV_INFO(x) ((struct wmux_dev_info *)netdev_priv(x))
#else
#define WMUX_DEV_INFO(x) ((struct wmux_dev_info *)(x->priv))
#endif
struct net_device_stats *wmux_dev_get_stats(struct net_device *dev);


#if defined(CONFIG_COMPAT_NET_DEV_OPS)
#else
int wmux_dev_open(struct net_device *vdev);
int wmux_dev_stop(struct net_device *dev);
int wmux_dev_set_mac_address(struct net_device *dev, void *addr_struct_p);
int wmux_dev_hard_start_xmit(struct sk_buff *skb, struct net_device *dev);
int wmux_dev_change_mtu(struct net_device *vdev, int new_mtu);
extern int wmux_dev_ioctl(struct net_device *dev,struct ifreq *ifr, int cmd);
//struct net_device_stats *wmux_dev_get_stats(struct net_device *dev);

static const struct net_device_ops wmux_netdev_ops = {
	.ndo_open		= wmux_dev_open,
	.ndo_stop		= wmux_dev_stop,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address 	= wmux_dev_set_mac_address,
//	.ndo_do_ioctl		= wmux_dev_ioctl,
	.ndo_start_xmit		= wmux_dev_hard_start_xmit,
	.ndo_change_mtu		= wmux_dev_change_mtu,
	.ndo_get_stats      = wmux_dev_get_stats,
};
#endif
#ifdef CONFIG_DEFAULTS_KERNEL_3_18
static const struct ethtool_ops wmux_ethtool_ops = {
	.get_link = ethtool_op_get_link,
};
#endif

/* found in socket.c */
extern void wmux_ioctl_set(int (*hook)(void __user *));
extern int getVidOfWmuxDev(char *name);
extern int wmuxUpstreamPortmappingCheck(struct sk_buff *skb);
extern int wmuxDownstreamPortmappingCheck(struct sk_buff *skb);
//extern struct net_device * getWmuxDev(struct sk_buff *skb);
extern unsigned int getMemberOfWmuxDevByVid(unsigned int vid);


#ifndef VLAN_VID_MASK
#define VLAN_VID_MASK	0xfff
#endif
#define VLAN_1P_MASK 	0xe000


/* wmux device info in net_device. */
struct wmux_dev_info {
	struct wmux_group *wmux_grp;
	struct net_device *vdev;
	struct net_device_stats stats; 
	rtk_rg_wan_type_t	proto;
	int    vid;		/* -1 means vlan disable */
	//int    napt;
	int	intf_idx;	/* rg interface index */
	int keep_order;	/* 0 means always detag in skb */
	unsigned int    m_1p;
	unsigned int    member;	/* for port mapping */
	struct list_head  wdev_list;
};

/* represents a group of wmux devices */
struct wmux_group {
	struct net_device	*real_dev;
	struct list_head	grp_list;	
	struct list_head	virtual_devs_head;
};


/* SMUX IOCTLs are found in sockios.h */

/* Passed in wmux_ioctl_args structure to determine behaviour. Should be same as busybox/networking/wmuxctl.c  */
enum wmux_ioctl_cmds {
	ADD_WMUX_CMD,
	REM_WMUX_CMD,
	GET_WMUX_CMD,
};

enum wmux_proto_types {
	WMUX_PROTO_PPPOE,
	WMUX_PROTO_IPOE,
	WMUX_PROTO_BRIDGE
};

/* 
 * for vlan device, wmux dev name is nas0.VID, others' name is nas0_No
 */
struct wmux_ioctl_args {
	int cmd; /* Should be one of the wmux_ioctl_cmds enum above. */
	int proto;
	int vid; /* vid==-1 means vlan disabled on this dev. */
	//int napt;
	//int brpppoe;
	char ifname[IFNAMSIZ];
	union {
		char ifname[IFNAMSIZ]; /* wmux device info */
	} u;
};

struct wmux_args {
	struct wmux_ioctl_args args;
	unsigned int member;
	unsigned int valid;
};

#define rwmux_ifname	ifname
#define owmux_ifname	u.ifname

//Usage Define
#define __wmux_init_usage() do {\
		PROC_PRINTF("SYNOPSIS:\n");\
		PROC_PRINTF("\techo [init] > /proc/rg/wmux_init\n");\
		PROC_PRINTF("DESCRIPTION:\n");\
		PROC_PRINTF("\treset all wmux settings.\n");\
		PROC_PRINTF("\t[init]:integer, available value is \"1\", other value will be ignored.\n");\
	}while(0)
#define __wmux_add_usage() do {\
		PROC_PRINTF("SYNOPSIS:\n");\
		PROC_PRINTF("\techo [real_dev] [dev_index] > /proc/rg/wmux_add\n");\
		PROC_PRINTF("DESCRIPTION:\n");\
		PROC_PRINTF("\tadd one wmux interface.\n");\
		PROC_PRINTF("\t[real_dev]:string, WAN physical interface for trasmit packets\n");\
		PROC_PRINTF("\t[dev_index]:integer, virtual interface index for simulate multiplex interfaces, range from 0 to 7.\n");\
	}while(0)
#define __wmux_del_usage() do {\
		PROC_PRINTF("SYNOPSIS:\n");\
		PROC_PRINTF("\techo [wmux_dev] > /proc/rg/wmux_del\n");\
		PROC_PRINTF("DESCRIPTION:\n");\
		PROC_PRINTF("\tdelete specific wmux interface.\n");\
		PROC_PRINTF("\t[wmux_dev]:string, virtual interface name which to be deleted.\n");\
	}while(0)
#define __wmux_flag_usage() do {\
		PROC_PRINTF("SYNOPSIS:\n");\
		PROC_PRINTF("\techo [wmux_dev] [keep_order] > /proc/rg/wmux_flag\n");\
		PROC_PRINTF("DESCRIPTION:\n");\
		PROC_PRINTF("\tset flag for specific wmux interface.\n");\
		PROC_PRINTF("\t[wmux_dev]:string, virtual interface name which to be deleted.\n");\
		PROC_PRINTF("\t[keep_order]:integer, set to \"1\" means keep packet VLAN tag in skb, only manipulat pointer;\n");\
		PROC_PRINTF("\t             set to \"0\" means remove VLAN tag, if necessary, from packet skb truly,\n");\
		PROC_PRINTF("\t             and it's the DEFAULT option when wmux interface created.\n");\
	}while(0)

//Function Declaration
int _rtk_rg_wmux_init_read(struct seq_file *s, void *v);
int _rtk_rg_wmux_add_read(struct seq_file *s, void *v);
int _rtk_rg_wmux_del_read(struct seq_file *s, void *v);
int _rtk_rg_wmux_flag_read(struct seq_file *s, void *v);
int _rtk_rg_wmux_info(struct seq_file *s, void *v);
int _rtk_rg_wmux_init_write(struct file *file, const char *buff, unsigned long len, void *data);
int _rtk_rg_wmux_add_write(struct file *file, const char *buff, unsigned long len, void *data);
int _rtk_rg_wmux_del_write(struct file *file, const char *buff, unsigned long len, void *data);
int _rtk_rg_wmux_flag_write(struct file *file, const char *buff, unsigned long len, void *data);

int wmux_pkt_recv(struct sk_buff *skb, struct net_device *dev);
int wmux_drv_init(void);
void wmux_drv_exit(void);

#endif

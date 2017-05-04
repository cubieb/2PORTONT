#ifndef _LINUX_IF_SMUX_H_
#define _LINUX_IF_SMUX_H_

#include <linux/netdevice.h>
#include <linux/skbuff.h>

/* found in socket.c */
extern void smux_ioctl_set(int (*hook)(void __user *));
extern int getVidOfSmuxDev(char *name);
extern int smuxUpstreamPortmappingCheck(struct sk_buff *skb);
extern int smuxDownstreamPortmappingCheck(struct sk_buff *skb);
extern struct net_device * getSmuxDev(struct sk_buff *skb);
extern unsigned int getMemberOfSmuxDevByVid(unsigned int vid);
extern int smuxDeviceCheck(const struct net_device *dev);

#ifndef VLAN_VID_MASK
#define VLAN_VID_MASK	0xfff
#endif
#define VLAN_1P_MASK 	0xe000


/* smux device info in net_device. */
struct smux_dev_info {
	struct smux_group *smux_grp;
	struct net_device *vdev;
	struct net_device_stats stats; 
	int    proto;
	int    vid;		/* -1 means vlan disable */
	int    napt;
	int    brpppoe;
	unsigned int    m_1p;
	unsigned int    member;	/* for port mapping */
	unsigned char	dev_addr[MAX_ADDR_LEN]; /* for omci module reading*/
	struct list_head  list;

#ifdef CONFIG_PORT_MIRROR
	int port_mirror;
	struct net_device* mirror_dev;
	
#define OUT_MIR_MASK			0x1
#define IN_MIR_MASK			0x2
#define OUT_NEED_MIR(val) ((val)  & (OUT_MIR_MASK)) 
#define IN_NEED_MIR(val)  ((val)  & (IN_MIR_MASK)) 

#endif
};

/* represents a group of smux devices */
struct smux_group {
	struct net_device	*real_dev;
	struct list_head	smux_grp_devs;	
	struct list_head	virtual_devs;
};

#define SMUX_DEV_INFO(x) ((struct smux_dev_info *)(x->priv))

/* inline functions */

static inline struct net_device_stats *smux_dev_get_stats(struct net_device *dev)
{
  return &(SMUX_DEV_INFO(dev)->stats);
}

/* SMUX IOCTLs are found in sockios.h */

/* Passed in smux_ioctl_args structure to determine behaviour. Should be same as busybox/networking/smuxctl.c  */
enum smux_ioctl_cmds {
	ADD_SMUX_CMD,
	REM_SMUX_CMD,
	GET_SMUX_CMD,
};

enum smux_proto_types {
	SMUX_PROTO_PPPOE,
	SMUX_PROTO_IPOE,
	SMUX_PROTO_BRIDGE
};

/* 
 * for vlan device, smux dev name is nas0.VID, others' name is nas0_No
 */
struct smux_ioctl_args {
	int cmd; /* Should be one of the smux_ioctl_cmds enum above. */
	int proto;
	int vid; /* vid==-1 means vlan disabled on this dev. */
	int napt;
	int brpppoe;
	char ifname[IFNAMSIZ];
	union {
		char ifname[IFNAMSIZ]; /* smux device info */
	} u;
};

struct smux_args {
	struct smux_ioctl_args args;
	unsigned int member;
	unsigned int valid;
};

#define rsmux_ifname	ifname
#define osmux_ifname	u.ifname
#endif /* _LINUX_IF_SMUX_H_ */

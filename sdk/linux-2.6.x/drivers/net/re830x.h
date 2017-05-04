/*	
 *	re830x.h
*/
#ifndef _RE830x_H_
#define _RE830x_H_
#include <linux/skbuff.h>

 //for switch function
extern int enable_IGMP_SNP;
extern int enable_port_mapping;
extern int enable_vlan_grouping;
extern int enable_virtual_port;
extern int DropUnknownMulticast;
void process_eth_vlan(struct sk_buff *skb, int *tag);

void select_page(unsigned int npage);
void detect_switch_version(void);
#define SW_PORT_NUM 4
#define MAX_SWITCH_PORT	7
#define SWITCH_PORT0 		0x0
#define SWITCH_PORT1 		0x1
#define SWITCH_PORT2 		0x2
#define SWITCH_PORT3 		0x3
#define SWITCH_PORT4 		0x4
#define SWITCH_PORT5 		0x5

// Kaohj -- SWITCH_PORT_NUMBER: used to identify switch ports;
//		5: used for system vlan (0: default group; 1~4: user-defined vlan)
#define VLAN_NUM	MAX_SWITCH_PORT+5

// per-port info
struct r8305_phy_struc {
	unsigned	egtag;		// egress tag
	unsigned	vlanIndex;	// pvid
};

// per vlan info
struct r8305_vlan_struc {
	unsigned	vid;		// vlan ID
	unsigned	member;		// 5-bit field to specifies which
					// ports are the memers of this vlan.
};

struct r8305_struc {
	unsigned		vlan_en;	// vlan enable flag
	unsigned		vlan_tag_aware;
	// Kaohj -- MAX_SWITCH_PORT: used to identify switch ports
	//		4: system-wise LAN ports info.
	struct r8305_phy_struc	phy[MAX_SWITCH_PORT+4];	// 5-port 830x switch + 4 ports for vlan-grouping
	// Kaohj -- SWITCH_PORT_NUMBER: used to identify switch ports;
	//		5: used for system vlan (0: default group; 1~4: user-defined vlan)
	struct r8305_vlan_struc	vlan[VLAN_NUM];	// vlan A,B,C,D,E info.
};

extern struct r8305_struc	rtl8305_info;
extern int virt2phy[SW_PORT_NUM+1];
void set_8305(struct r8305_struc *info);
void select_page(unsigned int npage);
int rtl8305sc_setAsicVlan(unsigned short vlanIndex, unsigned short vid, unsigned short memberPortMask);
int rtl8305sc_setAsicVlanTagAware(char enabled);
int rtl8305sc_setAsicBroadcastInputDrop(char enabled);
int rtl8305sc_setAsicVlanEnable(char enabled);
void rtl8305sc_setVLANTagInsertRemove(char port, char option);
int rtl8305sc_setPower(char port,char enabled);
void rtl8305sc_setVLANTag1pProirity(char port, char option);
int rtl8305sc_setAsicPortVlanIndex(unsigned short port, unsigned short vlanIndex);
#ifdef CONFIG_RE8306_API
void dump_8306_qos(void);
void reset_8306_counter(void);
void dump_8306_counter(void);
#endif // of CONFIG_RE8306_API

#define  TAG_DCARE 0x03
#define  TAG_ADD  0x02
#define  TAG_REMOVE 0x01
#define  TAG_REPLACE 0x00

extern struct net_device	*dev_sw[SW_PORT_NUM];
void check_IGMP_snoop_rx(struct sk_buff *skb, int tag);
#ifdef CONFIG_IPV6
//void check_MLD_snoop_rx(struct sk_buff *skb, int tag);
//int MLD_snoop_tx(struct sk_buff *skb, unsigned int *vid);
#endif
#ifdef CONFIG_ETHWAN
int process_cpu_tag(struct sk_buff *skb,int *port_tag);
#else
void process_cpu_tag(struct sk_buff *skb,int *port_tag);
#endif //CONFIG_ETHWAN
int get_vid(int pvid);
int get_pvid(int vid);
int re8670_xmit_sw(struct sk_buff *skb, unsigned int *vid, unsigned int port);

extern void _rtl8306_QoS_parameter_patch(void);

//for 8306N version
#define RTL830X 0x0   //default value
#define RTL8306G 0x5988
#define RTL8306N 0x6167
#endif /*_RE830x_H_*/

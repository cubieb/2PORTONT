#ifndef RTL865X_ACL_CONTROL_H
#define RTL865X_ACL_CONTROL_H

#include <linux/list.h>
#include <linux/if.h>
#include <net/dst.h>
#include <net/rtl/rtl865x_netif.h>


//#define ACL_CONTROL_DEBUG
#ifdef ACL_CONTROL_DEBUG
#define ACL_CONTROL_DEBUG_PRK printk
#else
#define ACL_CONTROL_DEBUG_PRK(format, args...)
#endif

#define RTL865X_PROC_DIR_NAME "rtl865x"

typedef struct xt_rule_to_acl_s
{
	struct list_head list;
	char iniface[IFNAMSIZ], outiface[IFNAMSIZ];
	rtl865x_AclRule_t *aclRule;	
} xt_rule_to_acl_t;

/* ACL Mode Definition */
#define RTL865X_ACL_Mode_Always_Permit	0x00
#define RTL865X_ACL_Mode_Always_Trap	0x01
#define RTL865X_ACL_Mode_Normal		0x02


#ifdef CONFIG_RTL8676_Static_ACL
/* MAC filter Mode Definition */
#define RTL865X_ACL_MAC_FILTER_ALL_PERMIT		0x00
#define RTL865X_ACL_MAC_FILTER_IN_DROP_OUT_PERMIT	0x01
#define RTL865X_ACL_MAC_FILTER_IN_PERMIT_OUT_DROP	0x02
#define RTL865X_ACL_MAC_FILTER_ALL_DROP			0x03
#endif

/* here, it decides the number of chain in RTL865X_ACL_USER_USED */
/*chain list priority: 0 > 1 > ...> 4*/
#ifdef CONFIG_RTL8676_Static_ACL
	#define RTL865X_CHAINLIST_NUMBER_PER_TBL 			2
	#define RTL865x_CHAINLIST_PRIORITY_LEVEL_FORCE_TRAP		0
	#define RTL865x_CHAINLIST_PRIORITY_LEVEL_NORMAL			1
#else  /* CONFIG_RTL8676_Dynamic_ACL  */
	#define RTL865X_CHAINLIST_NUMBER_PER_TBL 			3
	#define RTL865x_CHAINLIST_PRIORITY_LEVEL_L2_UNI_PERMIT		0 /* permit the pkts  in bridge shortcut */
	#define RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_REDIRECT		1 /* redirect pkts to specific interface  */
	#define RTL865x_CHAINLIST_PRIORITY_LEVEL_L3_PERMIT 		2  /* permit the pkts  in iptables fastpath */
#endif

int rtl865x_acl_control_set_mode(int mode);
/* functions */
#ifdef CONFIG_RTL8676_Static_ACL
int rtl865x_acl_control_set_mac_filter_mode(int mode);
int rtl865x_acl_control_get_mac_filter_mode(void);
int rtl865x_acl_control_filter_rule_add(xt_rule_to_acl_t* node,int force_trap);
int rtl865x_acl_control_filter_rule_clean(void);
#else  /* CONFIG_RTL8676_Dynamic_ACL  */
int rtl865x_acl_control_L2_permit_add(unsigned char* src_mac,unsigned char* dst_mac,char *in_ifname);
int rtl865x_acl_control_L2_permit_del(unsigned char* del_mac);
int rtl865x_acl_control_L2_permit_del_pair(unsigned char* src_mac,unsigned char* dst_mac);
int rtl865x_acl_control_L2_permit_clean(void);
int rtl865x_acl_control_L34_permit_add(__u32 src_ip,__u16 src_port,__u32 dst_ip,__u16 dst_port,__u8 protocol,char* in_netifname);
int rtl865x_acl_control_L34_permit_del(__u32 src_ip,__u16 src_port,__u32 dst_ip,__u16 dst_port,__u8 protocol);
int rtl865x_acl_control_L34_permit_clean(void);
int rtl865x_acl_control_L34_redirect_add(__u32 int_ip,__u16 int_port,__u32 rem_ip,__u16 rem_port,__u8 protocol,char *acl_in_ifname
													,__u32 redir_scr_ip,__u32 redir_dst_ip,char *redir_out_netif);
int rtl865x_acl_control_L34_redirect_del(__u32 src_ip,__u16 src_port,__u32 dst_ip,__u16 dst_port,__u8 protocol);
int rtl865x_acl_control_L34_redirect_clean(void);
int rtl865x_acl_control_delete_all_by_netif(char *in_ifname);
#endif
#endif /* RTL865X_ACL_CONTROL_H */

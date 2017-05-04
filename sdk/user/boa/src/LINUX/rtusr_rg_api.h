#include "options.h"
#include "mib.h"

#define UntagCPort 1
#define TagCPort 0

#define BridgeWan 0
#define RoutingWan 1

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAIL
#define FAIL -1
#endif

//ccwei: for debug
#define NIP_QUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#if defined(CONFIG_RTK_RG_INIT) || defined(CONFIG_GPON_FEATURE)
int RTK_RG_Sync_OMCI_WAN_INFO(void);
#endif
int RG_add_wan(MIB_CE_ATM_VC_Tp entry, int mib_vc_idx);
int Init_RG_API(int isUnTagCPort);

int RG_set_static(MIB_CE_ATM_VC_Tp entry);
int RG_release_static(int wanIntfIdx);
int RG_set_dhcp(unsigned int ipaddr, unsigned int submsk, MIB_CE_ATM_VC_Tp entry);
int RG_release_dhcp(int wanIntfIdx);

int RG_release_pppoe(int wanIntfIdx);

int RG_Del_All_LAN_Interfaces();
int RG_WAN_Interface_Del(unsigned int);

#ifdef MAC_FILTER
int AddRTK_RG_MAC_Filter(MIB_CE_MAC_FILTER_T *, int);
int RTK_RG_MAC_Filter_Default_Policy(int out_policy);
int FlushRTK_RG_MAC_Filters_in_ACL();
int FlushRTK_RG_MAC_Filters();
#endif
#ifdef IP_PORT_FILTER
int AddRTK_RG_ACL_IPPort_Filter(MIB_CE_IP_PORT_FILTER_T *);
#endif
int RTK_RG_ACL_IPPort_Filter_Default_Policy(int out_policy);
int RTK_RG_ACL_IPPort_Filter_Allow_LAN_to_GW();
int FlushRTK_RG_ACL_Filters();
#ifdef CONFIG_IPV6
int AddRTK_RG_ACL_IPv6Port_Filter(MIB_CE_V6_IP_PORT_FILTER_T *, char * prefixIP);
int RTK_RG_ACL_IPv6Port_Filter_Default_Policy(int out_policy);
int FlushRTK_RG_ACL_IPv6Port_Filters();
#endif
int RTK_RG_DMZ_Set(int enabled, in_addr_t ip_addr);
#ifdef PORT_FORWARD_GENERAL
int RTK_RG_Vertual_Server_Set(MIB_CE_PORT_FW_T *pf);
#endif
#ifdef CONFIG_USER_IP_QOS_3
int RTK_RG_QoS_Queue_Set();
int RTK_RG_QoS_Queue_Remove();
int RTK_RG_QoS_Rule_Set(MIB_CE_IP_QOS_Tp);
#endif
#ifdef CONFIG_USER_MINIUPNPD
int AddRTK_RG_UPnP_Connection(unsigned short, const char *, unsigned short, int);
int DelRTK_RG_UPnP_Connection(unsigned short, int);
#endif
#ifdef URL_BLOCKING_SUPPORT
int RTK_RG_URL_Filter_Set();
int Flush_RTK_RG_URL_Filter();
#endif
#if 0
int callbackRegistCheck(void);
#endif
int Init_rg_api(void);
int RG_del_All_Acl_Rules(void);
int RG_add_default_Acl_Qos(void);
#ifdef ROUTING
int RG_add_static_route(MIB_CE_IP_ROUTE_T *entry, char *mac_str, int entryID);
int RG_add_static_route_PPP(MIB_CE_IP_ROUTE_T *entry,MIB_CE_ATM_VC_T *vc_entry,int entryID);
#endif
int RG_reset_LAN(void);
int RG_check_Droute(int configAll, MIB_CE_ATM_VC_Tp pEntry, int *EntryID);
#ifdef DOS_SUPPORT
int RTK_RG_DoS_Set(int enable);
#endif
int Check_RG_Intf_Count(void);
int RG_get_lan_phyPortId(int logPortId);
int RG_get_wan_phyPortId();
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
int RG_add_pptp_wan(MIB_PPTP_T *pentry, int mib_pptp_idx);
int RG_add_pptp( unsigned long gw_ip, unsigned long my_ip, MIB_PPTP_T *pentry);
#endif
#ifdef CONFIG_USER_L2TPD_L2TPD
int RG_add_l2tp_wan(MIB_L2TP_T *pentry, int mib_l2tp_idx);
int RG_add_l2tp(unsigned long gw_ip, unsigned long my_ip, MIB_L2TP_T *pentry);
#endif
unsigned int RG_get_portCounter(unsigned int portIndex,unsigned long *tx_pkts,unsigned long *tx_drops,unsigned long *tx_errs,
										unsigned long *rx_pkts,unsigned long *rx_drops,unsigned long *rx_errs);
void RTK_RG_gatewayService_add();
void Flush_RTK_RG_gatewayService();


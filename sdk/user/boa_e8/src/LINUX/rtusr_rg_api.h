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

extern int patch_for_avalanche;
//ccwei: for debug
#define NIP_QUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]

#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
int RTK_RG_AccessWanLimit_Set(void);
int RTK_RG_AccessWanLimitCategory_Set(unsigned char *mac, int category);
#endif
int RTK_RG_Sync_OMCI_WAN_INFO(void);

int Flush_RTK_RG_IPv4_IPv6_Vid_Binding_ACL(void);
int RTK_RG_Set_IPv4_IPv6_Vid_Binding_ACL(void);

int RTK_RG_FLUSH_Bridge_DHCP_ACL_FILE(void);
int RTK_RG_Set_ACL_Bridge_DHCP_Filter(void);

int RG_add_wan(MIB_CE_ATM_VC_Tp entry, int mib_vc_idx);
int Init_RG_API(int isUnTagCPort);

int RG_set_static(MIB_CE_ATM_VC_Tp entry);
int RG_release_static(int wanIntfIdx);
int RG_set_dhcp(unsigned int ipaddr, unsigned int submsk, MIB_CE_ATM_VC_Tp entry);
int RG_release_dhcp(int wanIntfIdx);

int RG_release_pppoe(int wanIntfIdx);

int RG_Del_All_LAN_Interfaces();
int RG_WAN_Interface_Del(unsigned int);
#ifdef CONFIG_MCAST_VLAN
int RTK_RG_ACL_Add_mVlan(void);
int RTK_RG_ACL_Flush_mVlan(void);
#endif

#ifdef CONFIG_USER_MINIUPNPD
int FLUSH_RTK_RG_UPnP_Entry(void);
#endif

#ifdef MAC_FILTER
int AddRTK_RG_MAC_Filter(MIB_CE_MAC_FILTER_T *);
int RTK_RG_MAC_Filter_Default_Policy(int out_policy, int in_policy);
int FlushRTK_RG_MAC_Filters();
#endif
int AddRTK_RG_ACL_IPPort_Filter(MIB_CE_IP_PORT_FILTER_T *);
int RTK_RG_ACL_IPPort_Filter_Default_Policy(int out_policy, int in_policy);
int RTK_RG_ACL_IPPort_Filter_Allow_LAN_to_GW();
int FlushRTK_RG_ACL_Filters();
#ifdef CONFIG_IPV6
//int AddRTK_RG_ACL_IPv6Port_Filter(MIB_CE_V6_IP_PORT_FILTER_T *, char * prefixIP);
int AddRTK_RG_ACL_IPPort_Filter_IPv6(MIB_CE_IP_PORT_FILTER_T *);
int RTK_RG_ACL_IPv6Port_Filter_Default_Policy(int out_policy, int in_policy);
int FlushRTK_RG_ACL_IPv6Port_Filters();
#endif
int RTK_RG_DMZ_Set(int enabled, in_addr_t ip_addr);
#ifdef PORT_FORWARD_GENERAL
int RTK_RG_Vertual_Server_Set(MIB_CE_PORT_FW_T *pf);
#endif
#ifdef VIRTUAL_SERVER_SUPPORT
int RTK_RG_Virtual_Server_Set(MIB_VIRTUAL_SVR_T *pf);
int FlushRTK_RG_Virtual_Server();
#endif
#ifdef CONFIG_USER_IP_QOS_3
int RTK_RG_QoS_Queue_Set();
int RTK_RG_QoS_Queue_Remove();
int RTK_RG_QoS_Rule_Set(MIB_CE_IP_QOS_Tp qos_entry);
#endif
#ifdef CONFIG_USER_MINIUPNPD
int AddRTK_RG_UPnP_Connection(unsigned short, const char *, unsigned short, int);
int DelRTK_RG_UPnP_Connection(unsigned short, int);
#endif
#ifdef URL_BLOCKING_SUPPORT
int RTK_RG_URL_Filter_Set();
int RTK_RG_URL_Filter_Set_By_Key(int);
int Flush_RTK_RG_URL_Filter();
#endif
#if 0
int callbackRegistCheck(void);
#endif
int Init_rg_api(void);
int RTK_RG_DoS_Set(int enable);
int RG_del_All_Acl_Rules(void);
int RG_add_default_Acl_Qos(void);
int RG_add_PPPoE_RB_passthrough_Acl(void);
int RG_del_PPPoE_Acl(void);

int RG_add_static_route(MIB_CE_IP_ROUTE_T *entry, char *mac_str, int entryID);
int RG_add_static_route_PPP(MIB_CE_IP_ROUTE_T *entry,MIB_CE_ATM_VC_T *vc_entry,int entryID);
int RG_reset_LAN(void);
int RG_check_Droute(int configAll, MIB_CE_ATM_VC_Tp pEntry, int *EntryID);
int Check_RG_Intf_Count(void);
int RG_flush_vlanBinding(int LanPortIdx);
int RG_add_vlanBinding(MIB_CE_ATM_VC_Tp pEntry,int pairID, unsigned short LanVid, int LanPortIdx);
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
unsigned int RG_get_portCounter(unsigned int portIndex, unsigned long long *tx_bytes,unsigned long *tx_pkts,unsigned long *tx_drops,unsigned long *tx_errs,
										unsigned long long *rx_bytes, unsigned long *rx_pkts,unsigned long *rx_drops,unsigned long *rx_errs);
void RTK_RG_gatewayService_add();
void Flush_RTK_RG_gatewayService();
void RG_tcp_stateful_tracking(int enable);

#ifdef SUPPORT_WAN_BANDWIDTH_INFO
int RG_get_interface_counter(int rg_wan_idx, unsigned long long * uploadcnt, unsigned long long * downloadcnt);
#endif

#ifdef SUPPORT_WEB_REDIRECT
int RG_set_redirect_http_all(int enable, char * httpContent, int size);
int RG_set_welcome_redirect(int enable, char * url);
int RG_add_redirectHttpURL(MIB_REDIRECT_URL_LIST_T * redirectUrl);
int RG_del_redirectHttpURL(MIB_REDIRECT_URL_LIST_T * redirectUrl);
int RG_add_redirectWhiteUrl(MIB_REDIRECT_WHITE_LIST_T * whiteUrl);
int RG_del_redirectWhiteUrl(MIB_REDIRECT_WHITE_LIST_T * whiteUrl);
#endif

#ifndef _PRMT_APPLY_H_
#define _PRMT_APPLY_H_

#include <rtk/options.h>

/*action type for applying new values*/
#define CWMP_NONE		0
#define CWMP_START		1
#define CWMP_STOP		2
#define CWMP_RESTART	3

#ifdef __cplusplus
extern "C" {
#endif

int apply_UserAccount( int action_type, int id, void *olddata );
int lower_layers_changed(int action_type, int id, void *olddata);

#ifdef TIME_ZONE
int apply_NTP( int action_type, int id, void *olddata );
#endif //TIME_ZONE

int apply_DNS( int action_type, int id, void *olddata );
int apply_DHCP( int action_type, int id, void *olddata );

int apply_Layer3Forwarding( int action_type, int id, void *olddata );
int apply_RIP( int action_type, int id, void *olddata );
int apply_DefaultRoute( int action_type, int id, void *olddata );

#ifdef PORT_FORWARD_GENERAL
int apply_PortForwarding( int action_type, int id, void *olddata );
#endif

// Mason Yu. combine_1p_4p_PortMapping
#ifdef NEW_PORTMAPPING
int apply_PortMapping( int action_type, int id, void *olddata );
#endif //NEW_PORTMAPPING

#ifdef WLAN_SUPPORT
int apply_WLAN( int action_type, int id, void *olddata );
#endif

int apply_ETHER( int action_type, int id, void *olddata );
int apply_LANIP( int action_type, int id, void *olddata );

#ifdef MAC_FILTER
int apply_MACFILTER( int action_type, int id, void *olddata );
#endif

#ifdef CONFIG_USER_DDNS
int apply_DDNS( int action_type, int id, void *olddata );
#endif

#ifdef REMOTE_ACCESS_CTL
int apply_RemoteAccess( int action_type, int id, void *olddata );
#endif

#if defined(CONFIG_USER_IGMPPROXY) && defined(CONFIG_IGMPPROXY_MULTIWAN)
int apply_IGMPProxy( int action_type, int id, void *olddata );
#endif

#ifdef _PRMT_X_CT_COM_IPTV_
int apply_IGMPSnooping( int action_type, int id, void *olddata );
#endif

#ifdef _PRMT_TR143_
int apply_UDPEchoConfig( int action_type, int id, void *olddata );
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
int Apply_CaptivePortal(int action_type, int id, void *olddata);
#endif

#ifdef CONFIG_IP_NF_ALG_ONOFF
int apply_ALGONOFF( int action_type, int id, void *olddata );
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT) || defined(CONFIG_USER_IP_QOS_3)
int apply_IPQoSRule( int action_type, int id, void *olddata );
int apply_IPQoS( int action_type, int id, void *olddata );
#endif

#ifdef CONFIG_USER_TR104
int apply_voip( int action_type, int id, void *olddata );
#endif

#if defined(CONFIG_USER_UPNPD) || defined(CONFIG_USER_MINIUPNPD)
#ifdef _PRMT_X_CT_COM_UPNP_
int apply_UPnP(int action_type, int id, void *olddata);
#endif
#endif

#ifdef CONFIG_USER_MINIDLNA
int apply_DLNA( int action_type, int id, void *olddata );
#endif

#ifdef CONFIG_NO_REDIAL
int apply_Reconnect( int action_type, int id, void *olddata );
#endif

#ifdef _PRMT_X_CT_COM_PORTALMNT_
int apply_PortalMNT(int action_type, int id, void *olddata);
#endif

#ifdef CONFIG_IPV6
int apply_DHCPv6S(int action_type, int id, void *olddata);
int apply_lanv6_server( int action_type, int id, void *olddata );
#endif

#ifdef _PRMT_X_CT_COM_SYSLOG_
int apply_Syslog( int action_type, int id, void *olddata );
#endif

#if defined(CONFIG_USER_RADVD) && defined(CONFIG_E8B)
int apply_radvd( int action_type, int id, void *olddata );
#endif

#ifdef _PRMT_X_CT_COM_IPv6_
int apply_ip6_lan_addr( int action_type, int id, void *olddata );
#endif

#ifdef _PRMT_X_CT_COM_LBD_
int apply_lbd( int action_type, int id, void *olddata );
#endif

#ifdef CONFIG_MCAST_VLAN
int apply_mcast_vlan( int action_type, int id, void *olddata );
#endif

#ifdef __cplusplus
}
#endif
#endif /*_PRMT_APPLY_H_*/

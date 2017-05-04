#ifndef _CWMPC_UTILITY_H_
#define _CWMPC_UTILITY_H_
#ifdef CONFIG_DEV_xDSL
#include <linux/atm.h>
#endif
#include <rtk/utility.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Common Utilities*/
int is_hex(char c);
int string_to_hex(char *string, unsigned char *key, int len);
int get_wan_info_tr181(int num, MIB_CE_ATM_VC_T *pEntry, int *id, char *ifname);

/* Routing */
#define MAX_DYNAMIC_ROUTE_INSTNUM	10000

#ifdef CONFIG_DEV_xDSL
/* DSL Utilities */
extern char *strAdsl2WanStd[];

/* Only get required fields*/
int get_DSLWANStat( struct net_device_stats *nds );
int getAAL5CRCErrors( unsigned char vpi, unsigned short vci, unsigned int *count );
int getATMStats( unsigned char vpi, unsigned short vci, struct ch_stat *stat);
#endif

#ifdef WLAN_SUPPORT
/* WLAN Utilities*/
#if defined(WLAN_DUALBAND_CONCURRENT) && defined(WLAN_MBSSID)
#define WLAN_IF_NUM	10
#elif !defined(WLAN_DUALBAND_CONCURRENT) && defined(WLAN_MBSSID)
#define WLAN_IF_NUM	5
#elif defined(WLAN_DUALBAND_CONCURRENT) && !defined(WLAN_MBSSID)
#define WLAN_IF_NUM	2
#else
#define WLAN_IF_NUM	1	//single band, no MBSSID
#endif

extern char *wlan_name[ WLAN_IF_NUM ];
extern unsigned int	gWLANTotalClients;
extern char	gWLANAssociations[ sizeof(WLAN_STA_INFO_T)*(MAX_STA_NUM+1) ];

int loadWLANAssInfoByInstNum( unsigned int instnum );
#else
#define WLAN_IF_NUM 0
#endif

/* PPP Utilities*/
extern int gStartReset;
extern int resetChainID;

#ifdef CONFIG_PPP
int getPPPConStatus( char *pppname, char *status );
const char * getLastConnectionError(unsigned int ifindex);
int getPPPCurrentMRU( char *pppname, int ppptype, unsigned int *cmru );
int getPPPLCPEcho( char *pppname, int ppptype, unsigned int *echo );
int getPPPEchoRetry( char *pppname, int ppptype, unsigned int *retry );
#else
#define getLastConnectionError(ifindex) "ERROR_NONE"
#endif

#ifdef __cplusplus
}
#endif
#endif /*_CWMPC_UTILITY_H_*/




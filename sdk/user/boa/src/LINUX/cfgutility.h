#ifndef _CFGUTILITY_H_
#define _CFGUTILITY_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/if_bridge.h>
#include <sys/param.h>
#include <net/route.h>
#ifdef CONFIG_USER_BUSYBOX_BUSYBOX1124
#include "../../../busybox-1.12.4/include/autoconf.h"
#endif

//#define SECONDARY_IP		//xl_yue: for testing

enum {
	INPUT_TYPE_UINT,
	INPUT_TYPE_STRING,
	INPUT_TYPE_IPADDR,
	INPUT_TYPE_IPMASK,
	INPUT_TYPE_ETHADDR,
	INPUT_TYPE_INT
};

typedef struct atm_para {
	unsigned char vpi ;
	unsigned char qos;
	unsigned short vci;
	unsigned short pcr;
	unsigned short scr;
	unsigned short mbs;
	unsigned int cdvt;
}ATM_PARA, *ATM_PARA_P;

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
typedef struct snmp_para
{	/*NULL means don't modify this parameter*/
	char *pSysDescription;	/*system description*/
	char *pSysContact;		/*system contact*/
	char *pSysName;		/*system name*/
	char *pSysLocation;	/*system location*/
	char *pSysObjectID;	/*system object id*/
	char *pROCommunity;	/*read only community name*/
	char *pRWCommunity;	/*write  community name*/
	struct in_addr *pTrapIp;	/*trap ip address*/
} SNMP_PARA, *SNMP_PARA_P;
#endif

#ifdef CONFIG_USER_CWMP_TR069
typedef struct cwmp_acs
{	/*NULL means don't modify this parameter*/
	char *pUrl;			/*acs url*/
	char *pUsrname;		/*username*/ 
	char *pPwd;			/*password*/
	int *pIfmEn;			/* inform enabled */
	unsigned int *pIfmItvl;	/* inform interval */
} CWMP_ACS, *CWMP_ACS_P;

typedef struct cwmp_conn_req
{	/*NULL means don't modify this parameter*/
	char *pUsrname;	/*username*/ 	
	char *pPwd;		/*password*/
	char *pPath;		/*path*/
	int *pPort;		/*port*/
} CWMP_CONN_REQ, *CWMP_CONN_REQ_P;

#endif

typedef struct dhcp_server_para
{	/*NULL means don't modify this parameter*/
#ifdef DHCPS_POOL_COMPLETE_IP
	struct in_addr *pPoolstart;	/*pool start*/
	struct in_addr *pPoolend;	/*pool end*/
#else
	unsigned char *pPoolstart;	/*pool start*/	
	unsigned char *pPoolend;		/*pool end*/
#endif
	int *pLeaseTime;			/*lease time*/
	char *pDomain;			/*domain name*/
	struct in_addr *pGateway;	/*gateway*/
#ifdef DHCPS_DNS_OPTIONS
	unsigned char *pDnsMode;	/*dns mode: 0(relay),1(manual)*/
	struct in_addr *pDns1;		/*dns1*/
	struct in_addr *pDns2;		/*dns2*/
	struct in_addr *pDns3;		/*dns3*/
#endif
}DHCP_SERVER_PARA, *DHCP_SERVER_PARA_P;



#define CLEAR		printf("\033[H\033[J")

#ifdef URL_BLOCKING_SUPPORT
#define	URL_BLOCK_ADD		1
#define	URL_BLOCK_DEL		0
#endif

extern char *getMibInfo(int id);
extern int getInputNum();
extern int getInputStr( char *value, int maxlen, char *pcIsInputInvalid);
extern int getTypedInputDefault(int type, char *prompt, void *rv, void *arg1, void *arg2);
extern int getTypedInput(int type, char *prompt, void *rv, void *arg1, void *arg2);
extern int getInputIpAddr(char *prompt, struct in_addr *rv);
extern int getInputIpMask(char *prompt, struct in_addr *rv);
extern int getInputString(char *prompt, char *rv, int len);
extern int getInputUint(char *prompt, unsigned int *rv, unsigned int *min, unsigned int *max);
extern int getInputInt(char *prompt, int *rv, int *min, int *max);
extern int getInputOption(unsigned int *rv, unsigned int min0, unsigned int max0);
extern int mac17ToMac6(char *mac17, char *mac6);
extern void writePasswdFile();
extern void write_etcPassword();
extern void showStatus();
extern  int getDGW(char *buf);
extern int wanConnList(int bShowIndex);
#ifdef CONFIG_SECONDARY_IP
extern int setLanIpAddr(struct in_addr *pIp1,struct in_addr *pMask1,int enIp2,struct in_addr *pIp2,struct in_addr *pMask2,int dhcp_pool);
#else
extern int setLanIpAddr(struct in_addr *pIp1,struct in_addr *pMask1);
#endif
#ifdef CONFIG_EXT_SWITCH
extern int setLanIgmpSnooping(int enable);
#endif
extern void showInterfaceStatistics();
extern void showDSLSettings();
extern void showDSLStatistics();
extern void showLanInterface();
extern void showWanInterface();
extern void showARPTable();
extern void showRoutes();
extern void showMACs();
extern void showStaticRoutes();
extern int setDHCPMode(unsigned char mode);
extern int setDhcpRelayServer(struct in_addr *serverIp);
extern int setDhcpServer(DHCP_SERVER_PARA_P pDhcpServerPara);
#if 0
#ifdef DHCPS_POOL_COMPLETE_IP
extern int setDhcpsPoolRange(struct in_addr * pStartIp, struct in_addr *pEndIp);
#else
extern int setDhcpsPoolRange(unsigned char start, unsigned char end);
#endif
#ifdef DHCPS_DNS_OPTIONS
extern int setDhcpsDnsOption(unsigned char option);
extern int setDhcpsDns(int mib_id, struct in_addr * ip);
#endif
extern int setDhcpsGateway(struct in_addr * ip);
extern int setDhcpsLeaseTime(unsigned int time);
extern int setDhcpsDomainName(char * pName);
#endif

#ifdef IP_ACL
extern void showAclCfg();
extern int setAclCapability(int enable);
extern int addAclEntry(struct in_addr *ip, struct in_addr *mask, int interface, int enabled);
extern int delAclEntry(int index);
#endif
extern void showBridgeCfg();
extern int setBridgeAgingTime(unsigned short time);
extern int setBridgeStp(unsigned char enabled);
extern int setAccountPassword(char * user, char *oldpasswd, char *newpasswd);
extern int addDhcpReservedIp(struct in_addr *ip,char *pMac6);
extern int deleteDhcpReservedIp(struct in_addr *ip);
extern void showDhcpsReservedIp();
extern void showDhcpsClient();
extern void showDhcpsSetting();
extern int setDmzCapability(char enabled, struct in_addr *ip);
extern void showDmzSetting();
extern int setDnsServer(unsigned char mode,struct in_addr *dns1,struct in_addr *dns2,struct in_addr *dns3);
extern void showDnsSetting();
#ifdef DOMAIN_BLOCKING_SUPPORT
extern int addDomainBlockingEntry(char *domainName);
extern int deleteDomainBlockingEntry(char *domainName);
extern int setDomainBlockingCapablility(unsigned char enabled);
extern void showDomainBlocking();
#endif
extern int getIfModeByName(char *pIfname);
extern int getIfIndexByName(char *pIfname);
extern int getIfModeByIndex(int ifIndex);
#ifdef CONFIG_USER_IGMPPROXY
#if defined(APPLY_CHANGE)
extern int resetIGMProxy(int restart, char * pIfname);
#endif
extern int setIgmpProxyCapability(char enabled, char * pIfname);
extern void showIgmpProxy();
#endif
#ifdef URL_BLOCKING_SUPPORT
extern int setUrlBlockingEntry(int action, int mibId, char *urlstr);
extern int setUrlBlockingCapability(char enabled);
extern void showUrlBlocking();
#endif
#ifdef WLAN_SUPPORT
extern void showWLAN(void);
#endif
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
extern int setUpnpCapability(unsigned char enabled,unsigned char ifIndex);
extern void showUpnp();
#endif
extern int setDefaultIpPortFilterAction(char inact, char outact);
extern int ipPortFilterAdd(MIB_CE_IP_PORT_FILTER_Tp pFilterEntry);

extern int ipPortFilterDel(int index);
extern void showIPPortFilteringTable();
#if defined(IP_QOS)
extern int isIpQosEnabled();
extern int setDefaultQoS(unsigned char value);
extern int setQosCapability(unsigned char enabled);
extern int ipQosAdd(MIB_CE_IP_QOS_Tp pEntry);
extern int ipQosDel(int index);
extern void showIpQoS();
#endif
#ifdef MAC_FILTER
extern int setDefaultMacFilteringAction(char inact, char outact);
extern int addMacFiltering(MIB_CE_MAC_FILTER_Tp pEntry);
extern int deleteMacFiltering(int index);
extern void showMACFiltering();
#endif
#if defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
extern int isPortMappingEnabled();
extern int setPortMappingCapability(int enabled);
extern int addGroupInterface(enum PortMappingGrp, char *pIfname);
extern int delGroupInterface(enum PortMappingGrp, char *pIfname);
extern void showPortmapping();
#endif
extern int setPortForwardingCapability(unsigned char enabled);
extern int addPortForwarding(MIB_CE_PORT_FW_Tp pEntry);
extern int deletePortForwarding(int index);
extern void showPortForwarding();
#ifdef REMOTE_ACCESS_CTL
extern int setRmtAccess(int rmtType,int lanAct,int wanAct,int port);
extern void showRmtAccess();
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
extern int setRipCapability(unsigned char enabled);
extern int addRipInterface(MIB_CE_RIP_Tp pEntry);
extern int delRipInterface(int index);
extern void showRipStatusTbl();
#endif
#ifdef ROUTING
extern int staticRouteAdd(MIB_CE_IP_ROUTE_Tp pEntry);
extern int staticRouteDel(struct in_addr *dstip, struct in_addr *netmask);
#endif
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
extern int isSnmpEnabled();
extern int setSnmpCapability(char enabled);
extern int cfgSnmpSetting(SNMP_PARA_P pSnmpPara);
extern void showSnmpSettings();
#endif
#ifdef CONFIG_USER_CWMP_TR069
extern void showTr069Settings();
extern int setTr069Capability(char enabled);
extern int setTr069Acs(CWMP_ACS_P pAcs);
extern int setTr069ConnectReq(CWMP_CONN_REQ_P pConnReq);
#endif
extern int saveMibAll();
extern int setDSLSettings(unsigned short mode, char olr, unsigned short profile);
extern int modifyATMSettings(ATM_PARA_P pAtmPara);
extern void showATMSettings();
extern int addWanInterface(MIB_CE_ATM_VC_Tp pEntry);
extern int delWanInterface(char *pIfname);
#endif

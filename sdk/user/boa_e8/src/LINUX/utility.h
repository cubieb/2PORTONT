/*
 *      Include file of utility.c
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 *
 */

#ifndef INCLUDE_UTILITY_H
#define INCLUDE_UTILITY_H

#ifndef CONFIG_RTL_ALIASNAME
#define ALIASNAME_VC   "vc"
#define ALIASNAME_BR   "br"
#define ALIASNAME_NAS  "nas"
#define ALIASNAME_DSL  "dsl"
#define ALIASNAME_ETH  "eth"
#define ALIASNAME_WLAN "wlan"
#define ALIASNAME_PPP  "ppp"
#define ALIASNAME_MWNAS  "nas0_"
#define ALIASNAME_ELAN_PREFIX  "eth0."
#define ALIASNAME_PTM  "ptm"
#define ALIASNAME_MWPTM  "ptm0_"
#define ORIGINATE_NUM 2

#else
#define ALIASNAME_VC   CONFIG_ALIASNAME_VC//"vc"
#define ALIASNAME_BR   CONFIG_ALIASNAME_BR//"br"
#define ALIASNAME_NAS  CONFIG_ALIASNAME_NAS//"nas"
#define ALIASNAME_DSL  CONFIG_ALIASNAME_DSL//"dsl"
#define ALIASNAME_ETH  CONFIG_ALIASNAME_ETH//"eth"
#define ALIASNAME_WLAN  CONFIG_ALIASNAME_WLAN//"wlan"
#define ALIASNAME_PPP  CONFIG_ALIASNAME_PPP//"ppp"
#define ALIASNAME_MWNAS  CONFIG_ALIASNAME_MWNAS//"nas0_"
#define ALIASNAME_ELAN_PREFIX  CONFIG_ALIASNAME_ELAN_PREFIX//"eth0."
#define ALIASNAME_PTM  CONFIG_ALIASNAME_PTM//"ptm"
#define ALIASNAME_MWPTM  CONFIG_ALIASNAME_MWPTM//"ptm0_"
#define ORIGINATE_NUM CONFIG_ORIGINATE_NUM

#endif

#define RTL_DEV_NAME_NUM(name,num)	name#num

#define ALIASNAME_NAS0  RTL_DEV_NAME_NUM(ALIASNAME_NAS,0)//"nas0"
#define ALIASNAME_PTM0  RTL_DEV_NAME_NUM(ALIASNAME_PTM,0)//"ptm0"

#define ALIASNAME_DSL0  RTL_DEV_NAME_NUM(ALIASNAME_DSL,0)//dsl0
#define ALIASNAME_ELAN0  RTL_DEV_NAME_NUM(ALIASNAME_ELAN_PREFIX,2)
#define ALIASNAME_ELAN1  RTL_DEV_NAME_NUM(ALIASNAME_ELAN_PREFIX,3)
#define ALIASNAME_ELAN2  RTL_DEV_NAME_NUM(ALIASNAME_ELAN_PREFIX,4)
#define ALIASNAME_ELAN3  RTL_DEV_NAME_NUM(ALIASNAME_ELAN_PREFIX,5)

#define ALIASNAME_BR0   RTL_DEV_NAME_NUM(ALIASNAME_BR,0)//"br0"
#define ALIASNAME_WLAN0  RTL_DEV_NAME_NUM(ALIASNAME_WLAN,0)//"wlan0"

#define ALIASNAME_VAP   "-vap" //must include '-' at fast
#define ALIASNAME_WLAN0_VAP  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0,-vap)//"wlan0-vap"

#define ALIASNAME_WLAN0_VAP0  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VAP,0)//"wlan0-vap0"
#define ALIASNAME_WLAN0_VAP1  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VAP,1)//"wlan0-vap1"
#define ALIASNAME_WLAN0_VAP2  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VAP,2)//"wlan0-vap2"
#define ALIASNAME_WLAN0_VAP3  RTL_DEV_NAME_NUM(ALIASNAME_WLAN0_VAP,3)//"wlan0-vap3"


#include <sys/socket.h>
#include <linux/if.h>
#include <stdarg.h>
#include <stdio.h>
#include <net/route.h>
#include <netdb.h>
#include <dirent.h>
#include "adslif.h"
#include "mib.h"
#include "sysconfig.h"
#include "subr_dhcpv6.h"
#include "options.h"
#ifdef CONFIG_USER_XDSL_SLAVE
#include "subr_nfbi_api.h"
#endif /*CONFIG_USER_XDSL_SLAVE*/
#ifdef WLAN_SUPPORT
#include "subr_wlan.h"
#endif
#ifdef TIME_ZONE
#include "tz.h"
#endif

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
// kaotest --- temporary used, should be removed after new toolchain(for IPv6) is ready.
#define IPV6_ADDR_ANY		0x0000U
#define IPV6_ADDR_UNICAST	0x0001U
#define IPV6_ADDR_MULTICAST	0x0002U
#define IPV6_ADDR_LOOPBACK	0x0010U
#define IPV6_ADDR_LINKLOCAL	0x0020U
#define IPV6_ADDR_SITELOCAL	0x0040U
#define IPV6_ADDR_COMPATv4	0x0080U
#define IPV6_ADDR_SCOPE_MASK	0x00f0U
#define IPV6_ADDR_MAPPED	0x1000U
#define IPV6_ADDR_RESERVED	0x2000U	/* reserved address space */
// kaotest -- end of temporary used

// defined to use pppd, otherwise, use spppd if not defined
//#define USE_PPPD

/* Magician: Debug macro */
/* Example: CWMPDBP(2, "File not fould, file name=%s", filename);*/
/* Output: <DEBUG: abc.c, 1122>File not fould, file name=test.txt */
#define LINE_(line) #line
#define LINE(line) LINE_(line)
#define DBPRINT0(...) while(0){}
#define DBPRINT1(...) fprintf(stderr, "<"__FILE__","LINE(__LINE__)">"__VA_ARGS__)
#define DBPRINT2(...) fprintf(stderr, "<DEBUG:"__FILE__","LINE(__LINE__)">"__VA_ARGS__)
#define DBPRINT(level, ...) DBPRINT##level(__VA_ARGS__)

#define BUF_SIZE		256
#define MAX_POE_PER_VC		5
struct data_to_pass_st {
	int	id;
	char data[BUF_SIZE];
};

// Mason Yu. 2630-e8b
#ifdef TIME_ZONE
#define SNTP_DISABLED 0
#define SNTP_ENABLED 1
#endif

#ifdef VIRTUAL_SERVER_SUPPORT
#define VIRTUAL_SERVER_DELETE 0
#define VIRTUAL_SERVER_ADD  1
#define VIRTUAL_SERVER_ACTION_APPEND(type) (type==VIRTUAL_SERVER_ADD)?"-A":"-D"
#define VIRTUAL_SERVER_ACTION_INSERT(type)   (type==VIRTUAL_SERVER_ADD)?"-I":"-D"
#define VIRTUAL_SERVER_ACTION_PARAM_INT(type,d) (type==VIRTUAL_SERVER_ADD)?int2str(d):""

int setupVtlsvr(int type);
#endif

// Mason Yu. For Set IPQOS
#ifdef CONFIG_USER_IP_QOS
#define		SETIPQOS		0x01

/*
 * Structure used in SIOCSIPQOS request.
 */

struct ifIpQos
{
	int	cmd;
	char	enable;
};
#endif
#if defined(NEW_IP_QOS_SUPPORT) || defined(CONFIG_USER_IP_QOS_3)
enum qos_policy_t
{
	PLY_PRIO=0,
	PLY_WRR,
	PLY_NONE
};
#endif
#ifdef NEW_IP_QOS_SUPPORT
int setup_qos_setting(void);
void take_qos_effect(void);
void stop_IPQoS(void);
#ifdef CONFIG_DEV_xDSL
int monitor_qos_setting(void);
#endif
int delIpQosTcRule(MIB_CE_ATM_VC_Tp pEntry);
#endif

// Mason Yu
#ifdef IP_PASSTHROUGH
struct ippt_para
{
	unsigned int old_ippt_itf;
	unsigned int new_ippt_itf;
	unsigned char old_ippt_lanacc;
	unsigned char new_ippt_lanacc;
	unsigned int old_ippt_lease;
	unsigned int new_ippt_lease;
};
#endif

// Mason Yu. combine_1p_4p_PortMapping
#if (defined( ITF_GROUP_1P) && defined(ITF_GROUP)) || defined(NEW_PORTMAPPING)
#define		VLAN_ENABLE		0x01
#define		VLAN_SETINFO		0x02
#define		VLAN_SETPVIDX		0x03
#define		VLAN_SETTXTAG		0x04
#define		VLAN_DISABLE1PPRIORITY	0x05
#define		VLAN_SETIGMPSNOOP	0x06
#define		VLAN_SETPORTMAPPING	0x07
#define		VLAN_SETIPQOS		0x08
#define		VLAN_VIRTUAL_PORT	0x09
#define		VLAN_SETVLANGROUPING	0x0a
#ifdef CONFIG_PPPOE_PROXY
#define    SET_PPPOE_PROXY_PORTMAP  0x0b
#endif

#ifdef CONFIG_IGMP_FORBID
#define           IGMP_FORBID              0x0a
#endif
#define		TAG_DCARE	0x03
#define		TAG_ADD		0x02
#define		TAG_REMOVE	0x01
#define		TAG_REPLACE	0x00

/*
 * Structure used in SIOCSIFVLAN request.
 */

struct ifvlan
{
	int	cmd;
	char	enable;
	short	vlanIdx;
	short	vid;
	char		disable_priority;
	int	member;
	int	port;
	char	txtag;
};

struct brmap {
	int	brid;
	unsigned char pvcIdx;
};

extern int virtual_port_enabled;
extern const int virt2user[];
#ifdef CONFIG_USER_DDNS		// Mason Yu. Support ddns status file.
void remove_ddns_status(void);
#endif
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
void write_to_dhcpc_info(unsigned long requested_ip,unsigned long subnet_mask,unsigned long gw_addr);
#endif
#define MSG_BOA_PID		2222
// mtype for configd: Used as mtype to send message to configd; should be
//	well-configured to avoid conflict with the pid of any other processes
//	since the processes use	their pid as mtype to receive reply message
//	from configd.
#define MSG_CONFIGD_PID		8

// Mason Yu. Support ddns status file.
enum ddns_status{
	SUCCESSFULLY_UPDATED=0,
	CONNECTION_ERROR=1,
	AUTH_FAILURE=2,
	WRONG_OPTION=3,
	HANDLING=4,
	LINK_DOWN=5
};

// Mason Yu
enum PortMappingGrp
{
	PM_DEFAULTGRP=0,
	PM_GROUP1=1,
	PM_GROUP2=2,
	PM_GROUP3=3,
	PM_GROUP4=4
};

enum PortMappingAction
{
	PM_PRINT=0,
	PM_ADD=1,
	PM_REMOVE=2
};
#endif

extern const char*  wlan[];
extern const char*  wlan_itf[];
extern const int  wlan_en[];

typedef enum MARK
{
	PMAP_ETH0_SW0 = 0,
	PMAP_ETH0_SW1,
	PMAP_ETH0_SW2,
	PMAP_ETH0_SW3,
#ifdef WLAN_SUPPORT
	PMAP_WLAN0 = 4,
	PMAP_WLAN0_VAP0,
	PMAP_WLAN0_VAP1,
	PMAP_WLAN0_VAP2,
	PMAP_WLAN0_VAP3 = 8,
	PMAP_WLAN1 = 9,
	PMAP_WLAN1_VAP0,
	PMAP_WLAN1_VAP1,
	PMAP_WLAN1_VAP2,
	PMAP_WLAN1_VAP3 = 13,
#endif
	PMAP_ITF_END
} PMAP_LAN_T;

struct itfInfo
{
	#define	DOMAIN_ELAN	0x1
	#define	DOMAIN_WAN	0x2
	#define	DOMAIN_WLAN	0x4
	#define	DOMAIN_ULAN	0x8	//usbeth
	int	ifdomain;
	int	ifid;
	char	name[40];// changed by jim
};

// IF_ID(domain, ifIndex)
#define IF_ID(x, y)		((x<<24)|y)
#define IF_DOMAIN(x)		(x>>24)
#define IF_INDEX(x)		(x&0x00ffffff)
#define IFGROUP_NUM		5

#if defined(CONFIG_USBCLIENT) || !defined(CONFIG_RTL8672NIC)
#define DEVICE_SHIFT		5
#else
#define DEVICE_SHIFT		4
#endif
#define IFWLAN_SHIFT		6
#define IFWLAN1_SHIFT 12

#define MAX_NUM_OF_ITFS 32


#ifdef CONFIG_USB_ETH
#ifdef WLAN_SUPPORT
#define IFUSBETH_SHIFT		(IFWLAN_SHIFT+WLAN_MBSSID_NUM+1)
#else
#define IFUSBETH_SHIFT          (IFWLAN_SHIFT+1)
#endif
#define IFUSBETH_PHYNUM		(SW_LAN_PORT_NUM+5+1)  //for ipqos.phyPort (5: wlanphy max, 1:usb0)
#endif //CONFIG_USB_ETH


#define	IPQOS_NUM_PKT_PRIO	8
#define	IPQOS_NUM_PRIOQ		4

#ifdef _PRMT_X_CT_COM_QOS_
#define MODEINTERNET	0
#define MODETR069	1
#define MODEIPTV	2
#define MODEVOIP	3
#define MODEOTHER	4
#endif

struct mymsgbuf;

typedef enum { IP_ADDR, DST_IP_ADDR, SUBNET_MASK, DEFAULT_GATEWAY, HW_ADDR } ADDR_T;
typedef enum {
	SYS_UPTIME,
	SYS_DATE,
	SYS_YEAR,
	SYS_MONTH,
	SYS_DAY,
	SYS_HOUR,
	SYS_MINUTE,
	SYS_SECOND,
	SYS_FWVERSION,
	SYS_LAN_DHCP,
	SYS_DHCP_LAN_IP,
	SYS_DHCP_LAN_SUBNET,
	SYS_DHCPS_IPPOOL_PREFIX,
	SYS_DNS_MODE,
	SYS_WLAN,
	SYS_WLAN_SSID,
	SYS_WLAN_DISABLED,
	SYS_WLAN_HIDDEN_SSID,
	SYS_WLAN_BAND,
	SYS_WLAN_AUTH,
	SYS_WLAN_PREAMBLE,
	SYS_WLAN_BCASTSSID,
	SYS_WLAN_ENCRYPT,
	SYS_WLAN_MODE_VAL,
	SYS_WLAN_ENCRYPT_VAL,
	SYS_WLAN_WPA_CIPHER_SUITE,
	SYS_WLAN_WPA2_CIPHER_SUITE,
	SYS_WLAN_WPA_AUTH,
	SYS_WLAN_PSKFMT,
	SYS_WLAN_PSKVAL,
	SYS_WLAN_WEP_KEYLEN,
	SYS_WLAN_WEP_KEYFMT,
	SYS_WLAN_WPA_MODE,
	SYS_WLAN_RSPASSWD,
	SYS_WLAN_RS_PORT,
	SYS_WLAN_RS_IP,
	SYS_WLAN_RS_PASSWORD,
	SYS_WLAN_ENABLE_1X,
	SYS_TX_POWER,
	SYS_WLAN_MODE,
	SYS_WLAN_TXRATE,
	SYS_WLAN_BLOCKRELAY,
	SYS_WLAN_AC_ENABLED,
	SYS_WLAN_WDS_ENABLED,
	SYS_WLAN_QoS,
	SYS_WLAN_WPS_ENABLED,
	SYS_WLAN_WPS_STATUS,
	SYS_WLAN_WPS_LOCKDOWN,
	SYS_WSC_DISABLE,
	SYS_WSC_AUTH,
	SYS_WSC_ENC,
	SYS_DHCP_MODE,
	SYS_IPF_OUT_ACTION,
	SYS_DEFAULT_PORT_FW_ACTION,
	SYS_MP_MODE,
	SYS_IGMP_SNOOPING,
	SYS_PORT_MAPPING,
	SYS_IP_QOS,
	SYS_IPF_IN_ACTION,
	SYS_WLAN_BLOCK_ETH2WIR,
	SYS_DNS_SERVER,
	SYS_LAN_IP2,
	SYS_LAN_DHCP_POOLUSE,
	SYS_DEFAULT_URL_BLK_ACTION,
	SYS_DEFAULT_DOMAIN_BLK_ACTION,
	SYS_DSL_OPSTATE,
//#ifdef CONFIG_USER_XDSL_SLAVE
	SYS_DSL_SLV_OPSTATE,
//#endif /*CONFIG_USER_XDSL_SLAVE*/
	SYS_DHCPV6_MODE,
	SYS_DHCPV6_RELAY_UPPER_ITF,
	SYS_LAN_IP6_LL,
	SYS_LAN_IP6_GLOBAL,
	SYS_WLAN_WPA_CIPHER,
	SYS_WLAN_WPA2_CIPHER
} SYSID_T;

// enumeration of user process bit-shift for process bit-mapping
typedef enum {
	PID_DNSMASQ=0,
	PID_SNMPD,
	PID_WEB,
	PID_CLI,
	PID_DHCPD,
	PID_DHCPRELAY,
	PID_TELNETD,
	PID_FTPD,
	PID_TFTPD,
	PID_SSHD,
	PID_SYSLOGD,
	PID_KLOGD,
	PID_IGMPPROXY,
	PID_RIPD,
	PID_WATCHDOGD,
	PID_SNTPD,
	PID_MPOAD,
	PID_SPPPD,
	PID_UPNPD,
	PID_UPDATEDD,
	PID_CWMP, /*tr069/cwmpClient pid,jiunming*/
	PID_WSCD,
	PID_MINIUPNPD,
	PID_SMBD,
	PID_NMBD,
#ifdef VOIP_SUPPORT
	PID_VOIPGWDT,
	PID_SOLAR,
#endif
} PID_SHIFT_T;

enum PortMappingPriority
{
	HighestPrio=0,
	HighPrio=1,
	MediumPrio=2,
	lowPrio=3
};


#define		PID_SHIFT(x)		(1<<x)
#define		NET_PID			PID_SHIFT(PID_MPOAD)|PID_SHIFT(PID_SPPPD)
#define		ALL_PID			0xffffffff & ~(NET_PID)


#ifdef _USE_RSDK_WRAPPER_
#include <sys/syslog.h>
#endif //_USE_RSDK_WRAPPER_

int startSSDP();
int IfName2ItfId(char *s);
int do_ioctl(unsigned int cmd, struct ifreq *ifr);
int isDirectConnect(struct in_addr *haddr, MIB_CE_ATM_VC_Tp pEntry);
int getInAddr(char *interface, ADDR_T type, void *pAddr);
int getInFlags(char *interface, int *flags );
int setInFlags(char *interface, int flags );
int INET_resolve(char *name, struct sockaddr *sa);
int read_pid(const char *filename);
int getLinkStatus(struct ifreq *ifr);

extern const char AUTO_RESOLV[];
extern const char DNS_RESOLV[];
extern const char DNS6_RESOLV[];
extern const char PPP_RESOLV[];
extern const char RESOLV[];
extern const char DNSMASQ_CONF[];
extern const char RESOLV_BACKUP[];
extern const char HOSTS[];
extern const char MINIDLNAPID[];

#define MAX_CONFIG_FILESIZE 200000
extern int wlan_idx;	// interface index
// Added by Kaohj
extern const char LANIF[];
extern const char LAN_ALIAS[];	// alias for secondary IP
extern const char LAN_IPPT[];	// alias for IP passthrough
extern const char ELANIF[];
#ifdef CONFIG_RTL_MULTI_LAN_DEV
#define ELANVIF_NUM CONFIG_LAN_PORT_NUM //eth lan virtual interface number
#else
#define ELANVIF_NUM 1
#endif
//#if defined(CONFIG_ETHWAN) || defined(CONFIG_RTL_MULTI_LAN_DEV)
extern const char* ELANVIF[];
extern const char* SW_LAN_PORT_IF[];
//#endif

extern const char BRIF[];
extern const char VC_BR[];
extern const char LLC_BR[];
extern const char VC_RT[];
extern const char BLANK[];
extern const char LLC_RT[];
extern const char PORT_DHCP[];
extern const char ARG_ADD[];
extern const char ARG_CHANGE[];
extern const char ARG_DEL[];
extern const char ARG_ENCAPS[];
extern const char ARG_QOS[];
extern const char ARG_255x4[];
extern const char ARG_0x4[];
extern const char ARG_BKG[];
extern const char ARG_I[];
extern const char ARG_O[];
extern const char ARG_T[];
extern const char ARG_TCP[];
extern const char ARG_UDP[];
extern const char ARG_NO[];
#ifdef NEW_IP_QOS_SUPPORT
extern const char ARG_TCPUDP[];
#endif
extern const char ARG_ICMP[];
extern const char FW_BLOCK[];
extern const char FW_INACC[];
extern const char FW_IPFILTER[];
extern const char PORT_FW[];
extern const char IPTABLE_DMZ[];
extern const char IPTABLE_IPFW[];
extern const char IPTABLE_IPFW2[];
#ifdef NEW_PORTMAPPING
extern const char FW_DHCPS_DIS[];
#endif
extern const char FW_MACFILTER[];
extern const char FW_IPQ_MANGLE_DFT[];
extern const char FW_IPQ_MANGLE_USER[];
extern const char FW_DROP[];
extern const char FW_ACCEPT[];
extern const char FW_RETURN[];
extern const char FW_FORWARD[];
extern const char FW_INPUT[];
extern const char FW_PREROUTING[];
extern const char FW_DPORT[];
extern const char FW_SPORT[];
extern const char FW_ADD[];
extern const char FW_DEL[];
extern const char FW_INSERT[];
#ifdef PORT_FORWARD_ADVANCE
extern const char FW_PPTP[];
extern const char FW_L2TP[];
extern const char *PFW_Gategory[];
extern const char *PFW_Rule[];
int config_PFWAdvance( int action_type );
#endif
extern const char *strItf[];
extern const char RMACC_MARK[];
extern const char CONFIG_HEADER[];
extern const char CONFIG_TRAILER[];
extern const char CONFIG_XMLFILE[];
extern const char CONFIG_XMLENC[];
extern const char CONFIG_BIN[];
extern const char PPP_SYSLOG[];
extern const char PPP_DEBUG_LOG[];
extern const char PPP_CONF[];
extern const char PPP_PID[];
extern const char PPPOE_CONF[];
extern const char PPPOA_CONF[];
extern const char BACKUP_DIRNAME[];

extern const char ADSLCTRL[];
extern const char IFCONFIG[];
extern const char IWPRIV[];
extern const char BRCTL[];
extern const char MPOAD[];
extern const char MPOACTL[];
extern const char DHCPD[];
extern const char DHCPC[];
extern const char DNSRELAY[];
extern const char DNSRELAYPID[];
extern const char SPPPD[];
extern const char SPPPCTL[];
extern const char WEBSERVER[];
extern const char SNMPD[];
extern const char ROUTE[];
extern const char IPTABLES[];
extern const char RESOLV[];
extern const char DNSMASQ_CONF[];

#ifdef CONFIG_IPV6
extern const char IP6TABLES[];
extern const char FW_IPV6FILTER[];
extern const char FW_IPV6REMOTEACC[];
extern const char ARG_ICMPV6[];
#endif
extern const char EMPTY_MAC[MAC_ADDR_LEN];

/*ql 20081114 START need ebtables support*/
extern const char EBTABLES[];
extern const char ZEBRA[];
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
extern const char OSPFD[];
#endif
extern const char RIPD[];
extern const char ROUTED[];
extern const char IGMPROXY[];
extern const char TC[];
extern const char NETLOGGER[];
#ifdef TIME_ZONE
extern const char SNTPC[];
extern const char SNTPC_PID[];
#endif
#ifdef CONFIG_USER_DDNS
extern const char DDNSC_PID[];
#endif

extern const char PROC_DYNADDR[];
extern const char PROC_IPFORWARD[];
extern const char PROC_FORCE_IGMP_VERSION[];
extern const char PPPD_FIFO[];
extern const char MPOAD_FIFO[];
extern const char STR_DISABLE[];
extern const char STR_ENABLE[];
extern const char STR_UNNUMBERED[];
extern const char rebootWord0[];
extern const char rebootWord1[];
extern const char rebootWord2[];
extern const char errGetEntry[];
extern const char MER_GWINFO[];

extern const char *n0to7[];
extern const char *prioLevel[];
extern const int priomap[];;
extern const char *ipTos[];
//alex
#ifdef CONFIG_8021P_PRIO
extern const char *set1ptable[];
#ifdef NEW_IP_QOS_SUPPORT
extern const char *setpredtable[];
#endif
#endif

#if defined(CONFIG_RTL8681_PTM)
extern const char PTMIF[];
#endif

#ifdef CONFIG_USB_ETH
extern const char USBETHIF[];
#endif //CONFIG_USB_ETH

#ifdef CONFIG_USER_CUPS
int getPrinterList(char *str, size_t size);
#endif // CONFIG_USER_CUPS

extern const char STR_NULL[];
extern const char DHCPC_PID[];
extern const char DHCPC_ROUTERFILE[];
extern const char DHCPC_SCRIPT[];
extern const char DHCPC_SCRIPT_NAME[];
extern const char DHCPD_CONF[];
extern const char DHCPD_LEASE[];
extern const char DHCPSERVERPID[];
extern const char DHCPRELAYPID[];

#ifdef XOR_ENCRYPT
//Jenny, Configuration file encryption
extern const char XOR_KEY[];
void xor_encrypt(char *inputfile, char *outputfile);
#endif

extern const char PW_HOME_DIR[];
extern const char PW_CMD_SHELL[];

#if defined CONFIG_IPV6 || defined CONFIG_RTK_RG_INIT
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
extern const char DHCPDV6_CONF_AUTO[];
extern const char DHCPDV6_CONF[];
extern const char DHCPDV6_LEASES[];
extern const char DHCPDV6[];
extern const char DHCREALYV6[];
extern const char DHCPSERVER6PID[];
extern const char DHCPRELAY6PID[];
extern const char DHCPCV6SCRIPT[];
extern const char DHCPCV6[];
extern const char DHCPCV6STR[];
#endif
struct ipv6_ifaddr
{
	int			valid;
	struct in6_addr		addr;
	unsigned int		prefix_len;
	unsigned int		flags;
	unsigned int		scope;
};
#endif // of CONFIG_IPV6

extern const char *ppp_auth[];

#if defined(CONFIG_RTL_IGMP_SNOOPING)
void __dev_setupIGMPSnoop(int flag);	// enable/disable IGMP snooping
#endif

#if defined(CONFIG_RTL_MLD_SNOOPING)
void __dev_setupMLDSnoop(int flag);
#endif

void __dev_setupDirectBridge(int flag);
#ifdef CONFIG_USER_IP_QOS
int hwnat_wanif_rule(void);
int setWanIF1PMark(void);
#endif
void __dev_setupIPQoS(int flag);
#ifdef QOS_DIFFSERV
void cleanupDiffservRule(int idx);
int setupDiffServ(void);
#endif
int get_domain_ifinfo(struct itfInfo *info, int len, int ifdomain);

int do_cmd(const char *filename, char *argv [], int dowait);
int va_cmd(const char *cmd, int num, int dowait, ...);  //return 0:OK, other:fail
int va_cmd_no_echo(const char *cmd, int num, int dowait, ...);  //return 0:OK, other:fail
int call_cmd(const char *filename, int num, int dowait, ...);	//return 0:OK, other:fail
void write_to_pppd(struct data_to_pass_st *);
int write_to_mpoad(struct data_to_pass_st *);
int startDhcpc(char *inf, MIB_CE_ATM_VC_Tp pEntry);
void config_AddressMap(int action);
int startIP(char *inf, MIB_CE_ATM_VC_Tp pEntry, CHANNEL_MODE_T ipEncap);
void stopPPP(void);
int startPPP(char *inf, MIB_CE_ATM_VC_Tp pEntry, char *qos, CHANNEL_MODE_T pppEncap);
int find_ppp_from_conf(char *pppif);
int _get_classification_mark(int entryNo, MIB_CE_IP_QOS_T *p);
int get_classification_mark(int entryNo);
int startConnection(MIB_CE_ATM_VC_Tp pEntry, int);
void stopConnection(MIB_CE_ATM_VC_Tp pEntry);
#ifdef CONFIG_NO_REDIAL
void startReconnect(void);
#endif
#ifdef CONFIG_USER_RTK_SYSLOG
char *log_severity[8];
int stopLog(void);
int startLog(void);
#endif
#if defined(BB_FEATURE_SAVE_LOG) || defined(CONFIG_USER_RTK_SYSLOG)
void writeLogFileHeader(FILE * fp);
#endif
#ifdef DEFAULT_GATEWAY_V2
int ifExistedDGW(void);
#endif
int msgProcess(struct mymsgbuf *qbuf);
#if !defined(CONFIG_MTD_NAND) && defined(CONFIG_BLK_DEV_INITRD)
static inline int flashdrv_filewrite(FILE * fp, int size, void *dstP)
{
	return 0;
}
#else
int flashdrv_filewrite(FILE * fp, int size, void *dstP);
#endif


#ifdef CONFIG_USER_IGMPPROXY
int startIgmproxy(void);
#ifdef CONFIG_IGMPPROXY_MULTIWAN
int setting_Igmproxy(void);
#endif
#endif
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_ECMH
int startMLDproxy(void);		// Mason Yu. MLD Proxy
#endif
#ifdef CONFIG_USER_RADVD
void setup_radvd_conf(int resync);
void init_radvd_conf_mib(void);   // Added by Mason Yu for p2r_test
#endif
#if defined(CONFIG_USER_DHCPV6_ISC_DHCP411) || defined(CONFIG_USER_RADVD)
extern const char RADVD_CONF[];
extern const char RADVD_PID[];
#endif
void route_v6_cfg_modify(MIB_CE_IPV6_ROUTE_T *pRoute, int del);
#endif
void addStaticRoute(void);
void deleteStaticRoute(void);
int setupMacFilter(void);
#ifdef LAYER7_FILTER_SUPPORT
int setupAppFilter(void);
#endif
#ifdef PARENTAL_CTRL
int parent_ctrl_table_init(void);
int parent_ctrl_table_add(MIB_PARENT_CTRL_T *addedEntry);
int parent_ctrl_table_del(MIB_PARENT_CTRL_T *addedEntry);
int parent_ctrl_table_rule_update(void);
#endif
int setupDMZ(void);

#ifdef CONFIG_DEV_xDSL
char adsl_drv_get(unsigned int id, void *rValue, unsigned int len);
#ifdef CONFIG_VDSL
char dsl_msg_set_array(int msg, int *pval);
char dsl_msg_set(int msg, int val);
char dsl_msg_get_array(int msg, int *pval);
char dsl_msg_get(int msg, int *pval);
#endif /*CONFIG_VDSL*/

typedef struct _xdsl_op_
{
	int id;
	char (*xdsl_drv_get)(unsigned int id, void *rValue, unsigned int len);
#ifdef CONFIG_VDSL
	char (*xdsl_msg_set_array)(int msg, int *pval);
	char (*xdsl_msg_get_array)(int msg, int *pval);
	char (*xdsl_msg_set)(int msg, int val);
	char (*xdsl_msg_get)(int msg, int *pval);
#endif /*CONFIG_VDSL*/
	int  (*xdsl_get_info)(int id, char *buf, int len);
} XDSL_OP;
XDSL_OP *xdsl_get_op(int id);
int setupDsl(void);
#endif

int getLeasesInfo(const char *fname, DLG_INFO_Tp pInfo);
int getMIB2Str(unsigned int id, char *str);
int getSYS2Str(SYSID_T id, char *str);
int ifWanNum(const char *type); /* type="all", "rt", "br" */
#ifdef REMOTE_ACCESS_CTL
void remote_access_modify(MIB_CE_ACC_T accEntry, int enable);
void filter_set_remote_access(int enable);
#endif
#ifdef IP_ACL
void filter_set_acl(int enable);
#endif
#ifdef NAT_CONN_LIMIT
int restart_connlimit(void);
void set_conn_limit(void);
#endif
#ifdef TCP_UDP_CONN_LIMIT
int restart_connlimit(void);
void set_conn_limit(void);
#endif
#ifdef URL_BLOCKING_SUPPORT
void filter_set_url(int enable);
int restart_urlblocking(void);
#ifdef URL_ALLOWING_SUPPORT
void set_url(int enable);
int restart_url(void);
#endif
#endif
void itfcfg(char *if_name, int up_flag);
#ifdef DOMAIN_BLOCKING_SUPPORT
void filter_set_domain(int enable);
int restart_domainBLK(void);
#endif
#if defined(CONFIG_USER_ROUTED_ROUTED) || defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD)
int startRip(void);
#endif
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
int startOspf(void);
#endif
#ifdef SUPPORT_DHCP_RESERVED_IPADDR
int clearDHCPReservedIPAddrByInstNum(unsigned int instnum);
#endif
int setupDhcpd(void);
int startDhcpRelay(void);

/*ql:20080729 START: use icVer to record the version of IC*/
#ifdef MULTI_IC_SUPPORT
#define IC8671				1
//#define IC8671P				2
#define IC8672				3
#define IC8671B				4
#define IC8671B_costdown	5
#endif
/*ql:20080729 END*/

typedef struct {
	unsigned int	key;		/* magic key */

#define BOOT_IMAGE             0xB0010001
#define CONFIG_IMAGE           0xCF010002
/*ql:20080729 START: different IC with different IMG KEY*/
#ifndef MULTI_IC_SUPPORT
#define APPLICATION_IMAGE      0xA0000003
#ifdef CONFIG_RTL8686
#define APPLICATION_UBOOT      	0xA0000103	/*uboot only*/
#define APPLICATION_UIMAGE      0xA0000203	/*uimage only*/
#define APPLICATION_ROOTFS      0xA0000403	/*rootfs only*/
#endif
#else
#define APPLICATION_IMG_8671       0xA0000003
//#define APPLICATION_IMG_8671P      0xA0000002
#define APPLICATION_IMG_8672       0xA0000004
#define APPLICATION_IMG_8671B      0xA0000008
#define APPLICATION_IMG_8671B_CD   0xA0000010
/*ql:20080729 START: if sachem register read fail, then don't check image key*/
#define APPLICATION_IMG_ALL        0xA0000000
/*ql:20080729 END*/
#endif
/*ql:20080729 END*/
#define BOOTPTABLE             0xB0AB0004


	unsigned int	address;	/* image loading DRAM address */
	unsigned int	length;		/* image length */
	unsigned int	entry;		/* starting point of program */
	unsigned short	chksum;		/* chksum of */

	unsigned char	type;
#define KEEPHEADER    0x01   /* set save header to flash */
#define FLASHIMAGE    0x02   /* flash image */
#define COMPRESSHEADER    0x04       /* compress header */
#define MULTIHEADER       0x08       /* multiple image header */
#define IMAGEMATCH        0x10       /* match image name before upgrade */


	unsigned char	   date[25];  /* sting format include 24 + null */
	unsigned char	   version[16];
	unsigned int  *flashp;  /* pointer to flash address */

} IMGHDR;

//ql_xu ---signature header
#define SIG_LEN			20
typedef struct {
	unsigned int sigLen;	//signature len
	unsigned char sigStr[SIG_LEN];	//signature content
	unsigned short chksum;	//chksum of imghdr and img
}SIGHDR;

struct wstatus_info {
	unsigned int ifIndex;
	char ifname[IFNAMSIZ];
	char devname[IFNAMSIZ];
	char ifDisplayName[IFNAMSIZ];
	unsigned int tvpi;
	unsigned int tvci;
	int cmode;
	char encaps[8];
	char protocol[10];
	char ipver;	// IPv4 or IPv6
	char ipAddr[20];
	char remoteIp[20];
	char *strStatus;
	char uptime[20];
	char totaluptime[20];
	char vpivci[12];
	int pppDoD;
	int itf_state;
	int link_state;
};

unsigned short ipchksum(unsigned char *ptr, int count, unsigned short resid);

struct file_pipe {
	unsigned char *buffer;
	size_t bufsize;
	void (*func)(unsigned char *buffer, size_t *bufsize);
};
#ifdef _PRMT_USBRESTORE
int usbRestore(void);
#endif
void encode(unsigned char *buf, size_t * buflen);
void decode(unsigned char *buf, size_t *buflen);
int file_copy_pipe(const char *inputfile, const char *outputfile, struct file_pipe *pipe);
int usb_filter(const struct dirent *dirent);
int isUSBMounted(void);

#ifdef PORT_FORWARD_GENERAL
void clear_dynamic_port_fw(int (*upnp_delete_redirection)(unsigned short eport, const char * protocol));
int setupPortFW(void);
void portfw_modify( MIB_CE_PORT_FW_T *p, int del );
#endif
#ifdef TIME_ZONE
int startNTP(void);
int stopNTP(void);
#endif

#ifdef CONFIG_USER_MINIDLNA
void startMiniDLNA(void);
void stopMiniDLNA(void);
#endif

// Mason Yu. combine_1p_4p_PortMapping
#if (defined( ITF_GROUP_1P) && defined(ITF_GROUP)) || (defined( ITF_GROUP_4P) && defined(ITF_GROUP))
void setupEth2pvc(void);
#endif
#ifdef IP_QOS
int setupUserIPQoSRule(int enable);
#endif
#if defined(IP_QOS) | defined(CONFIG_USER_IP_QOS_3)
int stopIPQ(void);
int setupIPQ(void);
#endif
#ifdef CONFIG_USER_IP_QOS_3
void take_qos_effect_v3(void);
#endif
/*ql:20081114 START: support GRED*/
#define UNDOWAIT 0
#define DOWAIT 1
#define MAX_SPACE_LEGNTH 1024

#define DOCMDINIT \
		char cmdargvs[MAX_SPACE_LEGNTH]={0};\
		int argvs_index=1;\
		char *_argvs[32];

#define DOCMDARGVS(cmd,dowait,format,args...) \
		argvs_index=1;\
		memset(cmdargvs,0,sizeof(cmdargvs));\
		memset(_argvs,0,sizeof(_argvs));\
		snprintf(cmdargvs,sizeof(cmdargvs),format , ##args);\
		fprintf(stderr,"%s %s\n",cmd,cmdargvs);\
		_argvs[argvs_index]=strtok(cmdargvs," ");\
		while(_argvs[argvs_index]){\
			_argvs[++argvs_index]=strtok(NULL," ");\
		}\
		do_cmd(cmd,_argvs,dowait);
/*ql:20081114 END*/

#ifdef CONFIG_XFRM
#define SETKEY_CONF "/tmp/setkey.conf"
#define RACOON_CONF "/tmp/racoon.conf"
#define RACOON_PID "/var/run/racoon.pid"
#define PSK_FILE "/tmp/psk.txt"
#define DHGROUP_INDEX(x)	((x >> 24) & 0xff)
#define ENCRYPT_INDEX(x)	((x >> 16) & 0xff)
#define AHAUTH_INDEX(x)	((x >> 8) & 0xff)
#define AUTH_INDEX(x)	(x & 0xff)
struct IPSEC_PROP_ST
{
	char name[MAX_NAME_LEN];
	unsigned int algorithm; //dhGroup|espEncryption|ahAuth|espAuth
};

void ipsec_take_effect();
extern struct IPSEC_PROP_ST ikeProps[];
extern struct IPSEC_PROP_ST saProps[];
#endif

#ifdef CONFIG_USER_L2TPD_LNS
int applyL2TP(MIB_L2TP_T *pentry, int enable, int l2tp_index);
void l2tp_take_effect(void);
#endif

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_PPTPD_PPTPD
void applyPptpAccount(MIB_VPN_ACCOUNT_T *pentry, int enable);
void pptpd_take_effect(void);
#endif
void applyPPtP(MIB_PPTP_T *pentry, int enable, int pptp_index);
void pptp_take_effect(void);
#endif
#ifdef CONFIG_USER_L2TPD_LNS
void applyL2tpAccount(MIB_VPN_ACCOUNT_T *pentry, int enable);
#endif
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
int startSnmp(void);
int restart_snmp(int flag);
#endif
#if defined(CONFIG_USER_CWMP_TR069) || defined(APPLY_CHANGE)
void off_tr069(void);
#endif
#if defined(CONFIG_USER_CWMP_TR069) || defined(IP_ACL)
int restart_acl(void);
#endif
int restart_dnsrelay(void); //Jenny
int restart_dhcp(void);
int restart_lanip(void);
#ifdef CONFIG_USER_ROUTED_ROUTED
int delRipTable(unsigned int ifindex);
#endif
int delPPPoESession(unsigned int ifindex);
MIB_CE_ATM_VC_T *getATMVCEntryByIfIndex(unsigned int ifIndex, MIB_CE_ATM_VC_T *p);

#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
int delPortForwarding( unsigned int ifindex );
int updatePortForwarding( unsigned int old_id, unsigned int new_id );
int delRoutingTable( unsigned int ifindex );
int updateRoutingTable( unsigned int old_id, unsigned int new_id );
unsigned int findMaxConDevInstNum(MEDIA_TYPE_T mType);
unsigned int findConDevInstNumByPVC(unsigned char vpi, unsigned short vci);
unsigned int findMaxPPPConInstNum(MEDIA_TYPE_T mType, unsigned int condev_inst);
unsigned int findMaxIPConInstNum(MEDIA_TYPE_T mType, unsigned int condev_inst);
/*start use_fun_call_for_wan_instnum*/
int resetWanInstNum(MIB_CE_ATM_VC_Tp entry);
int updateWanInstNum(MIB_CE_ATM_VC_Tp entry);
#define dumpWanInstNum(p, s) do{}while(0)
/*end use_fun_call_for_wan_instnum*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
int delDhcpcOption( unsigned int ifindex );
unsigned int findMaxDHCPOptionInstNum( unsigned int usedFor, unsigned int dhcpConSPInstNum);
int getDHCPOptionByOptInstNum( unsigned int dhcpOptNum, unsigned int dhcpSPNum, unsigned int usedFor, MIB_CE_DHCP_OPTION_T *p, unsigned int *id );
int getDHCPClientOptionByOptInstNum( unsigned int dhcpOptNum, unsigned int ifIndex, unsigned int usedFor, MIB_CE_DHCP_OPTION_T *p, unsigned int *id );
unsigned int findMaxDHCPClientOptionInstNum(int usedFor, unsigned int ifIndex);
unsigned int findDHCPOptionNum(int usedFor, unsigned int ifIndex);
unsigned int findMaxDHCPReqOptionOrder(unsigned int ifIndex);
unsigned int findMaxDHCPConSPInsNum(void );
unsigned int findMaxDHCPConSPOrder(void );
int getDHCPConSPByInstNum( unsigned int dhcpspNum,  DHCPS_SERVING_POOL_T *p, unsigned int *id );
void clearOptTbl(unsigned int instnum);
unsigned int getSPDHCPOptEntryNum(unsigned int usedFor, unsigned int instnum);
int getSPDHCPRsvOptEntryByCode(unsigned int instnum, unsigned char optCode, MIB_CE_DHCP_OPTION_T *optEntry ,int *id);
void initSPDHCPOptEntry(DHCPS_SERVING_POOL_T *p);
#endif
#if defined(IP_QOS) || defined(CONFIG_USER_IP_QOS_3)
unsigned int getQoSQueueNum(void);
#endif
MIB_CE_ATM_VC_T *getATMVCByInstNum( unsigned int devnum, unsigned int ipnum, unsigned int pppnum, MIB_CE_ATM_VC_T *p, unsigned int *chainid );
int startCWMP(void);
#ifdef E8B_GET_OUI
void getOUIfromMAC(char *ouiname);
#endif
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
int proc_write_for_mwband(void);
#endif
void set_endpoint(char *newurl, char *acsurl); //star: remove "http://" from acs url string
#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
void setQosfortr069(int mode, char *urlvalue);
void setTr069QosFlag(int var);
int getTr069QosFlag(void);
#endif

#ifdef CONFIG_MIDDLEWARE
int sendSetDefaultRetMsg2MidIntf();
int sendSetDefaultFlagMsg2MidProcess();
int apply_Midware( int action_type, int id, void *olddata );	/*note: cann't restart cwmp process for apply immediately*/
void setapplicationtype_mw(MIB_CE_ATM_VC_T *pEntry, int mode, char *mwaddr);
void setMidwareRouteFW(int clearold);
#endif

void storeOldACS(void);
int getOldACS(char *acsurl);
#ifdef _PRMT_X_CT_COM_PORTALMNT_
#define TR069_FILE_PORTALMNT  "/proc/fp_tr069"
void setPortalMNT(void);
#endif
#endif //_CWMP_MIB_

int restart_ddns(void);
int getDisplayWanName(MIB_CE_ATM_VC_T *pEntry, char* name);
int getWanEntrybyindex(MIB_CE_ATM_VC_T *pEntry, unsigned int ifIndex);
int getWanEntrybyMedia(MIB_CE_ATM_VC_T *pEntry, MEDIA_TYPE_T mType);
unsigned int getWanIfMapbyMedia(MEDIA_TYPE_T mType);
int isValidMedia(unsigned int ifIndex);
unsigned int if_find_index(int cmode, unsigned int map);

int setWanName(char *str, int applicationtype);
int generateWanName(MIB_CE_ATM_VC_T * entry, char *wanname);
int getWanName(MIB_CE_ATM_VC_T * pEntry, char *name);
int getifIndexByWanName(const char *name);

int create_icmp_socket(void);
int in_cksum(unsigned short *buf, int sz);
int utilping(char *str);
int defaultGWAddr(char *gwaddr);
int pdnsAddr(char *dnsaddr);
int getATMEntrybyVPIVCIUsrPswd(MIB_CE_ATM_VC_T* Entry, int vpi, int vci, char* username, char* password, char* ifname);

int getNameServers(char *buf);
int setNameServers(char *buf);
#ifdef ACCOUNT_CONFIG
int getAccPriv(char *user);
#endif
int isValidIpAddr(char *ipAddr);
int isValidHostID(char *ip, char *mask);
int isValidNetmask(char *mask, int checkbyte);
int isSameSubnet(char *ipAddr1, char *ipAddr2, char *mask);
int isValidMacString(char *MacStr);
int isValidMacAddr(unsigned char *macAddr);

struct ppp_policy_route_info {
	u_char	if_name[IFNAMSIZ];
	u_long	hisip;
	u_long	myip;
	u_long	primary_dns;
	u_long	second_dns;
};
extern int set_ppp_source_route(struct ppp_policy_route_info *ppp_info);

#ifdef QOS_SPEED_LIMIT_SUPPORT
int mib_qos_speed_limit_existed(int speed,int prior);
#endif
int restart_ethernet(int instnum);
#ifdef ELAN_LINK_MODE
int setupLinkMode(void);
#endif

#ifdef _PRMT_TR143_
struct TR143_UDPEchoConfig
{
	unsigned char	Enable;
	unsigned char	EchoPlusEnabled;
	unsigned short	UDPPort;
	unsigned char	Interface[IFNAMSIZ];
	unsigned char	SourceIPAddress[4];
};
void UDPEchoConfigSave(struct TR143_UDPEchoConfig *p);
int UDPEchoConfigStart( struct TR143_UDPEchoConfig *p );
int UDPEchoConfigStop( struct TR143_UDPEchoConfig *p );
#endif //_PRMT_TR143_

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
enum eTStatus
{
	eTStatusDisabled,
	eTStatusUnsynchronized,
	eTStatusSynchronized,
	eTStatusErrorFailed,/*Error_FailedToSynchronize*/
	eTStatusError
};
#endif
/*ping_zhang:20081217 END*/

//void pppoe_session_update(void *p);
//void save_pppoe_sessionid(void *p);
// Added by Magician for external use
int deleteConnection(int configAll, MIB_CE_ATM_VC_Tp pEntry);
#ifdef CONFIG_RTL8672_SAR
int RegisterPVCnumber(void);
#endif
int startWan(int configAll, MIB_CE_ATM_VC_Tp pEntry);
#define CONFIGONE	0
#define CONFIGALL 	1

// Mason Yu
#define WEB_REDIRECT_BY_MAC_INTERVAL	12*60*60	/* Polling DHCP release table every 12 hours */
#define TIMEOUT(fun, arg1, arg2, handle) 	timeout(fun,arg1,arg2, &handle)
#define UNTIMEOUT(fun, arg, handle)		untimeout(&handle)
#ifdef WEB_REDIRECT_BY_MAC
extern struct callout landingPage_ch;
void clearLandingPageRule(void *dummy);
#endif
#ifdef AUTO_DETECT_DMZ
extern struct callout autoDMZ_ch;
#endif

// Mason Yu. Timer for auto search PVC
#ifdef CONFIG_RTL8672_SAR
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
extern int autoHunt_found;
extern int fin_AutoSearchPVC;
extern struct callout autoSearchPVC_ch;
extern int sok;
void succ_AutoSearchPVC(void);
int setVC0Connection(unsigned int vpi, unsigned int vci);
void stopAutoSearchPVC(void *dummy);
int startAutoSearchPVC(void);
#define INTERVAL_AUTOPVC 10//45
#define COUNT_AUTOPVC	0//4 If COUNT_AUTOPVC is 0, the AutoPvcSearch will repeat until it find the PVC.
#endif
#endif
#ifdef MULTI_IC_SUPPORT
int getImgKey(void);
#endif
int pppdbg_get(int unit);
struct sysinfo * updateLinkTime(unsigned char update);
#ifdef CONFIG_USER_XDSL_SLAVE
struct sysinfo * updateSlvLinkTime(unsigned char update);
#endif /*CONFIG_USER_XDSL_SLAVE*/
#ifdef CONFIG_ATM_CLIP
void sendAtmInARPRep(unsigned char update);
#endif

void poll_autoDMZ(void *dummy);
void restartWAN(int configAll, MIB_CE_ATM_VC_Tp pEntry);
void resolveServiceDependency(unsigned int idx);
#ifdef IP_PASSTHROUGH
void restartIPPT(struct ippt_para para);
#endif
#ifdef DOS_SUPPORT
void setup_dos_protection(void);
#endif

#ifdef CONFIG_RTK_OAM_V1
extern struct callout eponoamd_ch;
void notify_eponoamd(void *dummy);
#endif

#ifdef CONFIG_LED_INDICATOR_TIMER
extern struct callout ledsched_ch;
void led_schedule(void);
#endif

#ifdef CONFIG_RG_SLEEPMODE_TIMER
extern struct callout sleepmode_ch;
void sleepmode_schedule(void);
#endif

#ifdef COMMIT_IMMEDIATELY
void Commit(void);
#endif

int checkRoute(MIB_CE_IP_ROUTE_T, int);
void setup_ipforwarding(int enable);
void route_cfg_modify(MIB_CE_IP_ROUTE_T *, int, int entryID);
void route_ppp_ifup(unsigned long pppGW);
char *ifGetName(int, char *, unsigned int);
int getIfIndexByName(char *pIfname);
int getNameByIP(char *ip, char *buffer, unsigned int len);
#if defined(ITF_GROUP_1P) && defined(ITF_GROUP)
int setVlan(struct ifreq *ifr);
#endif


/* WAPI */
#define WAPI_TMP_CERT "/var/tmp/tmp.cert"
#define WAPI_AP_CERT "/var/myca/ap.cert"
#define WAPI_CA_CERT "/var/myca/CA.cert"
#define WAPI_CA4AP_CERT "/var/myca/ca4ap.cert"
#define WAPI_AP_CERT_SAVE "/var/config/ap.cert"
#define WAPI_CA_CERT_SAVE "/var/config/CA.cert"
#define WAPI_CA4AP_CERT_SAVE "/var/config/ca4ap.cert"
void wapi_cert_link_one(const char *name, const char *lnname);

#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
#ifndef CONFIG_RTL8672NIC
#define ETHWAN_PORT 0//in virtual view
#define PTMWAN_PORT 5//in virtual view
#else
#define ETHWAN_PORT 3
#endif
int init_ethwan_config(MIB_CE_ATM_VC_T *pEntry);
#endif

#ifdef CONFIG_USER_WT_146
int wt146_dbglog_get(unsigned char *ifname);
void wt146_create_wan(MIB_CE_ATM_VC_Tp pEntry, int reset_bfd_only );
void wt146_del_wan(MIB_CE_ATM_VC_Tp pEntry);
void wt146_set_default_config(MIB_CE_ATM_VC_Tp pEntry);
void wt146_copy_config(MIB_CE_ATM_VC_Tp pto, MIB_CE_ATM_VC_Tp pfrom);
#endif //CONFIG_USER_WT_146

// Magician: This function is for checking the validation of whole config file.
char checkConfigFile(char *config_file);

// Magician: This function can show memory usage change on the fly.
#if DEBUG_MEMORY_CHANGE
extern char last_memsize[128], last_file[32], last_func[32]; // Use to indicate last position where you put ShowMemChange().
extern int last_line;  // Use to indicate last position where you put ShowMemChange().
int ShowMemChange(char *file, char *func, int line);
#endif

#ifndef CONFIG_RTL8672NIC
void setupIPQoSflag(int flag);
#endif

int restart_IPFilter_DMZ_MACFilter(void);
#ifdef CONFIG_IP_NF_ALG_ONOFF
int setupAlgOnOff(void);
#endif
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
int start_captiveportal(void);
int stop_captiveportal(void);
#endif

//Kevin:Check whether to enable/disable upstream ip fastpath
void UpdateIpFastpathStatus(void);

#define BIT_IS_SET(a, no)  (a & (0x1 << no))

#ifdef NEW_PORTMAPPING
//define it when debug newportmapping
#define NEWPORTMAPPING_DBG
#ifdef NEWPORTMAPPING_DBG
#define AUG_PRT(fmt,args...)  printf("\033[1;33;46m<%s %d %s> \033[m"fmt, __FILE__, __LINE__, __func__ , ##args)
#else
#define AUG_PRT(fmt,args...)  do{}while(0)
#endif

#define PMAP_DEFAULT_TBLID	252
struct pmap_s {
	int valid;
	unsigned int ifIndex;	// resv | media | ppp | vc
	unsigned int applicationtype;
	unsigned short itfGroup;
	unsigned short fgroup;
};

extern struct pmap_s pmap_list[MAX_VC_NUM];
int get_pmap_fgroup(struct pmap_s *pmap_p, int num);
int check_itfGroup(MIB_CE_ATM_VC_Tp pEntry, MIB_CE_ATM_VC_Tp pOldEntry);
int exec_portmp();
void setupnewEth2pvc();

int caculate_tblid(uint32_t ifid);

void setup_wan_pmap_lanmember(MEDIA_TYPE_T mType, unsigned int Index);


#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
void modPolicyRouteTable(const char *pptp_ifname, struct in_addr *real_addr);
#ifdef CONFIG_IPV6_VPN
void modIPv6PolicyRouteTable(const char *pptp_ifname, struct in6_addr *real_addr);
#endif
#endif
#endif

#ifdef CONFIG_HWNAT
void setup_hwnat_eth_member(int port, int mbr, char enable);
#ifdef CONFIG_PTMWAN
void setup_hwnat_ptm_member(int port, int mbr, char enable);
#endif
#endif
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
int Init_RTK_RG_Device(void);
int Flush_RG_static_route(void);
int check_RG_static_route(void);
int check_RG_static_route_PPP(MIB_CE_ATM_VC_T *vc_entry);
void rg_add_route(MIB_CE_IP_ROUTE_T *entry, int entryID);
#endif

struct net_device_stats
{
	unsigned long	rx_packets;		/* total packets received	*/
	unsigned long	tx_packets;		/* total packets transmitted	*/
	unsigned long	rx_bytes;		/* total bytes received 	*/
	unsigned long	tx_bytes;		/* total bytes transmitted	*/
	unsigned long	rx_errors;		/* bad packets received		*/
	unsigned long	tx_errors;		/* packet transmit problems	*/
	unsigned long	rx_dropped;		/* no space in linux buffers	*/
	unsigned long	tx_dropped;		/* no space available in linux	*/
	unsigned long	multicast;		/* multicast packets received	*/
	unsigned long	collisions;

	/* detailed rx_errors: */
	unsigned long	rx_length_errors;
	unsigned long	rx_over_errors;		/* receiver ring buff overflow	*/
	unsigned long	rx_crc_errors;		/* recved pkt with crc error	*/
	unsigned long	rx_frame_errors;	/* recv'd frame alignment error */
	unsigned long	rx_fifo_errors;		/* recv'r fifo overrun		*/
	unsigned long	rx_missed_errors;	/* receiver missed packet	*/

	/* detailed tx_errors */
	unsigned long	tx_aborted_errors;
	unsigned long	tx_carrier_errors;
	unsigned long	tx_fifo_errors;
	unsigned long	tx_heartbeat_errors;
	unsigned long	tx_window_errors;

	/* for cslip etc */
	unsigned long	rx_compressed;
	unsigned long	tx_compressed;
};

/*
 *	Refer to linux/ethtool.h
 */
struct net_link_info
{
	unsigned long	supported;	/* Features this interface supports: ports, link modes, auto-negotiation */
	unsigned long	advertising;	/* Features this interface advertises: link modes, pause frame use, auto-negotiation */
	unsigned short	speed;		/* The forced speed, 10Mb, 100Mb, gigabit */
	unsigned char	duplex;		/* Duplex, half or full */
	unsigned char	phy_address;
	unsigned char	transceiver;	/* Which transceiver to use */
	unsigned char	autoneg;	/* Enable or disable autonegotiation */
};

#define _PATH_PROCNET_DEV "/proc/net/dev"
int list_net_device_with_flags(short flags, int nr_names,
				char (* const names)[IFNAMSIZ]);
int get_net_device_stats(const char *ifname, struct net_device_stats *nds);
enum {
	RX_OCTETS,
	RX_DISCARDS,
	RX_UCAST_PACKETS,
	RX_MCAST_PACKETS,
	RX_BCAST_PACKETS,
	TX_OCTETS,
	TX_DISCARDS,
	TX_UCAST_PACKETS,
	TX_MCAST_PACKETS,
	TX_BCAST_PACKETS,
};
struct ethtool_stats * ethtool_gstats(const char *ifname);
int get_net_link_status(const char *ifname);
int get_net_link_info(const char *ifname, struct net_link_info *info);

#define WAN_MODE GetWanMode()
enum e_wan_mode {MODE_ATM = 1, MODE_Ethernet = 2, MODE_PTM = 4, MODE_BOND = 8, MODE_Wlan = 0x10};
int GetWanMode(void);
int isInterfaceMatch(unsigned int);
#define WAN_MODE_MASK (GET_MODE_ATM|GET_MODE_ETHWAN|GET_MODE_PTM|GET_MODE_WLAN)

#ifdef CONFIG_DEV_xDSL
#define GET_MODE_ATM 0x1
#else
#define GET_MODE_ATM 0x0
#endif

#ifdef CONFIG_ETHWAN
#define GET_MODE_ETHWAN 0x2
#else
#define GET_MODE_ETHWAN 0x0
#endif

#ifdef CONFIG_PTMWAN
#define GET_MODE_PTM 0x4
#else
#define GET_MODE_PTM 0
#endif

#ifdef CONFIG_PTM_BONDING
#define GET_MODE_BOND 0x8
#else
#define GET_MODE_BOND 0
#endif

#ifdef WLAN_WISP
#define GET_MODE_WLAN 0x10
#else
#define GET_MODE_WLAN 0
#endif

int reset_cs_to_default(int flag); // 0: short reset; 1: long reset

#ifdef CONFIG_USER_SAMBA
int startSamba(void);
int stopSamba(void);
#endif // CONFIG_USER_SAMBA

#ifdef CONFIG_TR_064
#define TR064_STATUS GetTR064Status()
int GetTR064Status(void);
#endif

#ifdef CONFIG_USER_CWMP_TR069
#define PRE_CWMP_WAN_INTF "/var/pre_cwmp_wan_intf"
enum {Old_ACS_URL = 1, Cur_ACS_URL};
int SetTR069WANInterface(void); // Magician: Bind WAN interface to TR-069
int SetTR069WANInterfacePPP(void);
int SetTR069WANInterfaceDHCP(void);
int DelTR069WANInterface(char);
#endif

// Mason Yu. Specify IP Address
struct ddns_info {    				/* Used as argument to ddnsC() */
	int		ipversion;        		/* IPVersion . 1:IPv4, 2:IPv6, 3:IPv4 and IPv6 */
	char 	ifname[IFNAMSIZ];        /* Interface name */
};

int update_hosts(char *, struct addrinfo *);
struct addrinfo *hostname_to_ip(char *, IP_PROTOCOL);

#define PMAP_VC_START	1
#define PMAP_PPP_START	0x10
#define PMAP_NAS_START 0x20
#define PMAP_NAS_PPP_START 0x30
#ifdef CONFIG_PTMWAN
#define PMAP_PTM_START 0x40
#define PMAP_PTM_PPP_START 0x50
#endif
#define ITF_SOURCE_ROUTE_VC_START	0x60
#define ITF_SOURCE_ROUTE_NAS_START	0x70
#define ITF_SOURCE_ROUTE_PTM_START	0x80
#define ITF_SOURCE_ROUTE_PPP_START	0x90

/***********************************************************************/
/* parameter structure & utility functions. */
/***********************************************************************/
/*error code*/
#define ERR_9000	-9000	/*Method not supported*/
#define ERR_9001	-9001	/*Request denied*/
#define ERR_9002	-9002	/*Internal error*/
#define ERR_9003	-9003	/*Invalid arguments*/
#define ERR_9004	-9004	/*Resources exceeded*/
#define ERR_9005	-9005	/*Invalid parameter name*/
#define ERR_9006	-9006	/*Invalid parameter type*/
#define ERR_9007	-9007	/*Invalid parameter value*/
#define ERR_9008	-9008	/*Attempt to set a non-writable parameter*/
#define ERR_9009	-9009	/*Notification request rejected*/
#define ERR_9010	-9010	/*Download failure*/
#define ERR_9011	-9011	/*Upload failure*/
#define ERR_9012	-9012	/*File transfer server authentication failure*/
#define ERR_9013	-9013	/*Unsupported protocol for file transfer*/

#if defined(NEW_IP_QOS_SUPPORT)
int setMIBforQosMode(unsigned char modeflag);
//int cr2reg(int pcr);
#endif //IP_QOS

/* mac to string function */
int hex(unsigned char);
void convert_mac(char *);

/*star:20080807 START add for ct qos model*/
#define NONEMODE  0
#define INTERNETMODE 1
#define TR069MODE     2
#define IPTVMODE       4
#define  VOIPMODE      8
#define  OTHERMODE    16
/*star:20080807 END*/

int getWanEntrybyindex(MIB_CE_ATM_VC_T *, unsigned int);

// Magician: E8B Security
typedef struct _WAN_STATE_ST_ {
	char xDSLMode;	//xDSL mode: route, route&bridge
	char wanItf;	//0-MER only; 1- PPPoE only;  2- MER&PPPoE
	char ifNum;		//num of route interface
	char null;
} WAN_STATE_T;

typedef struct _VC_STATE_ST_ {
	char ifName[IFNAMSIZ];	//interface name
	char chMode;	//channel mode of current PVC: CHANNEL_MODE_IPOE ...
	char fstPvc;	//show if is the first configured wan interface
	char dfGW;		//show if interface with default GW
	char null;
} VC_STATE_T, *VC_STATE_Pt;

//use fw_state_t to reserve the set state of firewall grade
typedef struct _FW_GRADE_INIT_ST_{
	char linkDownInit;	//set firewall grade while link down
	char linkUpInit;	//set firewall grade while link up.
	char preFwGrade;	//previous firewall grade
	char null;
} FW_GRADE_INIT_St;

#define EBTABLES_ENABLED 1
#define EBTABLES_DISABLED 0
#define MAC_FILTER_BRIDGE_RULES 16
#define MAC_FILTER_ROUTER_RULES 16

#define DOS_ENABLE		0x01
#define SYSFLOODSYN		0x02
#define SYSFLOODFIN		0x04
#define SYSFLOODUDP		0x08
#define SYSFLOODICMP	0x10
#define IPFLOODSYN		0x20
#define IPFLOODFIN		0x40
#define IPFLOODUDP		0x80
#define IPFLOODICMP		0x100
#define TCPUDPPORTSCAN	0x200
#define ICMPSMURFENABLED	0x400
#define IPLANDENABLED		0x800
#define IPSPOOFENABLED		0x1000
#define IPTEARDROPENABLED	0x2000
#define PINGOFDEATHENABLED	0x4000
#define TCPSCANENABLED		0x8000
#define TCPSynWithDataEnabled	0x10000
#define UDPBombEnabled			0x20000
#define UDPEchoChargenEnabled	0x40000
#define sourceIPblock		0x400000

int changeFwGrade(int);
int setupDos();
// End Magician: E8B Security
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
int set_ctcom_alarm(unsigned int alarm_num);
int clear_ctcom_alarm(unsigned int alarm_num);
#endif
void compact_reqoption_order();

#ifdef CONFIG_USB_SUPPORT
struct usb_info{
                char disk_type[64];
                char disk_status[64];
                char disk_fs[64];
                unsigned long disk_used;
                unsigned long disk_available;
                char disk_mounton[256];
};
void getUSBDeviceInfo(int *disk_sum,struct usb_info* disk1,struct usb_info *disk2);
#endif
#ifdef CONFIG_HTTP_DOWNLOAD_TEST
extern struct callout httptest_ch;
#endif

#ifdef SUPPORT_WAN_BANDWIDTH_INFO
extern struct callout bandwidth_ch;
void poll_bandwidth(void *dummy);
int wan_bandwidth_set(unsigned char enable, unsigned int interval);
int wan_bandwidth_get(int rg_wan_idx, int * uploadrate, int * downloadrate);
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
extern struct callout macinfo_ch;
void poll_macinfo(void);
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
extern struct callout lanbandwidth_ch;
void poll_lanbandwidth(void *dummy);
void startLanPortMibInfoTimer(void);
#endif
#ifdef SUPPORT_WEB_REDIRECT
void welcomeRedirectCheck();
int webRedirectSetRGRule(int add);
#endif

void compact_reqoption_order(unsigned int ifIndex);

int check_user_is_registered(void);
int sync_itfGroup(int ifidx);	//for vlan binding

/***************************************************************************/
#ifdef SUPPORT_ACCESS_RIGHT
#define DEFAULT_INTERNET_ACCESS_RIGHT	1
#define DEFAULT_STORAGE_ACCESS_RIGHT	1
#endif
#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
#define DEFAULT_MAX_US_BANDWIDTH		1024*1024
#define DEFAULT_MAX_DS_BANDWIDTH		1024*1024
#endif

#ifdef CONFIG_USER_LANNETINFO
#define MAX_LANNET_DEV_NAME_LENGTH			32
#define MAX_LANNET_BRAND_NAME_LENGTH		16
#define MAX_LANNET_OS_NAME_LENGTH			16

#define MAX_LAN_HOST_NUM		256

typedef struct lanHostInfo_s
{
	unsigned char	mac[MAC_ADDR_LEN];
	char			devName[MAX_LANNET_DEV_NAME_LENGTH];
	unsigned char	devType;	/* 0-phone 1-pad 2-PC 3-STB 4-other  0xff-unknown */
	unsigned int	ip;
	unsigned char	connectionType;	/* 0- wired 1-wireless */
	unsigned char	port;	/* 0-wifi, 1- lan1, 2-lan2, 3-lan3, 4-lan4 */
	char			brand[MAX_LANNET_BRAND_NAME_LENGTH];
	char			os[MAX_LANNET_OS_NAME_LENGTH];
	unsigned int	onLineTime;
} lanHostInfo_t;

typedef struct hgDevInfo_s
{
	unsigned char	mac[MAC_ADDR_LEN];
	char			devName[MAX_LANNET_DEV_NAME_LENGTH];
} hgDevInfo_t;

#endif

#endif // INCLUDE_UTILITY_H

/***************************************************************************/

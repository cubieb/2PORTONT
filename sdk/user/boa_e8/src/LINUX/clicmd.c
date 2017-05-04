/*
 *	clicmd.c
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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
#include <pwd.h>
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#include "mib.h"
#include "adsl_drv.h"
#include "utility.h"
#ifdef USE_LIBMD5
#include <libmd5wrapper.h>
#else
#include "../md5.h"
#endif //USE_LIBMD5
#include <crypt.h>
#include <time.h>
#include <sys/stat.h>
#include "cfgutility.h"


typedef enum {
	SECURITY_ROOT,
	SECURITY_SUPERUSER,
	SECURITY_USER
} SECURITY_LEVEL;

typedef enum {
	ARG_START = 0,
	NUMBER1 = 1,
	NUMBER2,
	NUMBER3,
	NUMBER4,
	NUMBER5,
	NUMBER6,
	NUMBER7,
	NUMBER8,
	NUMBER9,
	NUMBER10,
	IPADDR1,
	IPADDR2,
	IPADDR3,
	IPADDR4,
	IPADDR5,
	IPADDR6,
	IPMASK1,
	IPMASK2,
	IPMASK3,
	IPMASK4,
	IPMASK5,
	IPMASK6,
	MACADDR1,
	MACADDR2,
	MACADDR3,
	MACADDR4,
	MACADDR5,
	MACADDR6,
	STRING1,
	STRING2,
	STRING3,
	STRING4,
	STRING5,
	STRING6,
	STRING7,
	STRING8,
	RANGE1,
	RANGE2,
	RANGE3,

	ARG_TAG,
	ARG_END
} ARG_ID;

typedef enum {
	CMD_LINUX_SHELL = 1,
	CMD_EXIT_CLI,
	CMD_SHOW_SYSTEM_STATUS,
	CMD_SHOW_INTERFACE,
	CMD_SHOW_INTERFACE_STATISTICS,
	CMD_SHOW_LAN_SETTINGS,
	CMD_SHOW_INTERFACE_WAN,
	CMD_CFG_LAN_IP,
	CMD_REBOOT_SYSTEM,
	CMD_SAVE_CONFIG,
	CMD_RESTORE_DEFAULT,
	CMD_SHOW_ARP_TBL,
	CMD_SHOW_FDB_TBL,
	CMD_SHOW_ROUTE_TBL,
	CMD_SHOW_STATIC_ROUTES,
	CMD_DHCP_MODE,
	CMD_ADD_ASSIGN_IP,
	CMD_DELETE_ASSIGN_IP,
	CMD_DHCP_RELAY_SETTING,
	CMD_DHCP_SERVER_SETTING,
	CMD_SHOW_DCHP_ASSIGNED_IP,
	CMD_SHOW_DCHP_CLIENT,
	CMD_SHOW_DCHP_SETTING,
	CMD_DHCP_DNS_RELAY,
	CMD_DHCP_DNS_MANUAL,
	CMD_SET_DNS_SERVER,
	CMD_DELETE_WAN_INTERFACE,
	CMD_ADD_1483BRG_PVC,
	CMD_ADD_1483MER_PVC,
	CMD_ADD_1483RT_PVC,
	CMD_ADD_PPPOA_PVC,
	CMD_ADD_PPPOE_PVC,
	CMD_ENABLE_AUTO_PVC,
	CMD_DISABLE_AUTO_PVC,
	CMD_ADD_AUTO_PVC,
	CMD_DELETE_AUTO_PVC,
	CMD_SHOW_AUTO_PVC,
	CMD_MODIFY_ATM_SETTING,
	CMD_CFG_ADSL_SETTING,
	CMD_ENABLE_IGMP_PROXY,
	CMD_DISABLE_IGMP_PROXY,
	CMD_DISABLE_UPNP,
	CMD_ENABLE_UPNP,
	CMD_SHOW_ATM_VC_TBL,
	CMD_SHOW_RIP_STATUS_TBL,
	CMD_SHOW_BRIDGING,
	CMD_SHOW_UPNP,
	CMD_SHOW_IGMP_PROXY,
	CMD_SHOW_DNS_SERVER,
	CMD_ADD_RIP_INTERFACE,
	CMD_DELETE_RIP_INTERFACE,
	CMD_DISABLE_RIP,
	CMD_ENABLE_RIP,
	CMD_SHOW_IP_PORT_FILTERING,
	CMD_ADD_IPPORT_FILTERING,
	CMD_DELETE_IPPORT_FILTERING,
	CMD_IPPORT_FILTER_DEFAULT_ACTION,
	CMD_ADD_MAC_FILTERING,
	CMD_DELETE_MAC_FILTERING,
	CMD_MAC_FILTER_DEFAULT_ACTION,
	CMD_SHOW_MAC_FILTERING,
	CMD_SHOW_PORT_FORWARDING,
	CMD_ADD_PORT_FORWARDING,
	CMD_DELETE_PORT_FORWARDING,
	CMD_DISABLE_PORT_FORWARDING,
	CMD_ENABLE_PORT_FORWARDING,
	CMD_ADD_URL_BLOCKING,
	CMD_DELETE_URL_BLOCKING,
	CMD_DISABLE_URL_BLOCKING,
	CMD_ENABLE_URL_BLOCKING,
	CMD_SHOW_URL_BLOCKING,
	CMD_SHOW_DOMAIN_BLOCKING,
	CMD_ADD_DOMAIN_BLOCKING,
	CMD_DELETE_DOMAIN_BLOCKING,
	CMD_DISABLE_DOMAIN_BLOCKING,
	CMD_ENABLE_DOMAIN_BLOCKING,
	CMD_DISABLE_DMZ,
	CMD_ENABLE_DMZ,
	CMD_SHOW_DMZ,
	CMD_SET_BRIDGING,
	CMD_ADD_ROUTE,
	CMD_DELETE_ROUTE,
	CMD_SHOW_SNMP_SETTINGS,
	CMD_ENABLE_SNMP,
	CMD_DISABLE_SNMP,
	CMD_CONFIG_SNMP,
	CMD_DISABLE_PORT_MAPPING,
	CMD_ENABLE_PORT_MAPPING,
	CMD_ADD_GROUP1_INTERFACE,
	CMD_DELETE_GROUP1_INTERFACE,
	CMD_ADD_GROUP2_INTERFACE,
	CMD_DELETE_GROUP2_INTERFACE,
	CMD_ADD_GROUP3_INTERFACE,
	CMD_DELETE_GROUP3_INTERFACE,
	CMD_ADD_GROUP4_INTERFACE,
	CMD_DELETE_GROUP4_INTERFACE,
	CMD_SHOW_PORT_MAPPING,
	CMD_SHOW_IP_QOS,
	CMD_SET_DEFAULT_QOS,
	CMD_ADD_IPQOS_RULE,
	CMD_DELETE_IPQOS_RULE,
	CMD_DISABLE_IP_QOS,
	CMD_ENABLE_IP_QOS,
	CMD_SET_TELNET_ACCESS,
	CMD_SET_FTP_ACCESS,
	CMD_SET_TFTP_ACCESS,
	CMD_SET_WEB_ACCESS,
	CMD_SET_SNMP_ACCESS,
	CMD_SET_PING_ACCESS,
	CMD_SHOW_REMOTE_ACCESS,
	CMD_SET_PASSWORD,
	CMD_SHOW_ACL_CONFIG,
	CMD_ADD_ACL,
	CMD_DELETE_ACL,
	CMD_DISABLE_ACL,
	CMD_ENABLE_ACL,
	CMD_SHOW_TR069,
	CMD_CFG_TR069_ACS,
	CMD_CFG_TR069_CONNECT_REQUEST,
	CMD_DISABLE_TR069,
	CMD_ENABLE_TR069,
	CMD_DHCP_DNS,
	
#if defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL_IGMP_SNOOPING)
	CMD_ENABLE_IGMP_SNOOPING,
	CMD_DISABLE_IGMP_SNOOPING,
#endif
	CMD_PRINT_HELP_INFO,
	CMD_SHOW_ADSL_SETTING,
	CMD_SHOW_ADSL_STATISTICS,
	
	
	COMMAND_END
} CLI_COMMAND;

struct arg_range{
	int start;
	int end;
};

struct arg_table{
	int arg_id;
	void *arg_pnt;
	char *arg_helpname; 
};

typedef struct cmd_arg{
	char *arg_name; 
	unsigned int arg_id;
	unsigned int arg_flag;
	char *arg_help;
}CMD_ARG;

typedef struct cmd_node{
	char	*cmd_str;
	int	cmd_flag;
	int	cmd_type;
	struct cmd_node *cmd_fwd;	/*note: cmd_fwd and cmd_args can not exist simultaneously*/
	struct cmd_arg  *cmd_args;
	void	(*cmd_func)(int , struct cmd_arg *); 
	void	(*err_func)(struct cmd_node *, char *); 
	char * cmd_help;
}CMD_NODE;


#define FALSE 0
#define TRUE  1

#define END			0x1		/*denotes that there should be one cmd_func for this command, and com_args can be added*/
#define SUSERONLY	0x2		/*denotes that this command can be executed by suser user only*/

#define CLI_RUNFILE	"/var/run/cli.pid"
#define MAX_ARGC		32

/*argument flags defination*/
#define ARG_SET				0x1		/*denotes that this argument has been inputted, otherwise it has not been inputted*/
#define ARG_SELECT_START		0x2		/*arg tag, see START_SELECT_ARG*/
#define ARG_SELECT_DETACH	0x4		/*arg tag, see DETACH_SELECT_ARG*/
#define ARG_SELECT_END		0x8		/*arg tag, see END_SELECT_ARG*/
#define ARG_NAME				0x10		/*denotes that argument's name need to be inputted when inputting command arguments*/
#define ARG_VALUE			0x20		/*denotes that argument's value need to be inputted when inputting command arguments*/	
#define ARG_OPTIONAL			0x40		/*denotes that this argument is optional. argument name is necessary for this argument inputting! In help information, this argument will be braced by "[" and "]"*/
#define ARG_BOTH				(ARG_NAME | ARG_VALUE)		/*both name and value are needed to be inputted*/
#define ARG_NAME_OPTIONAL	(ARG_NAME | ARG_OPTIONAL)		/*optional argument,but name is necessary when inputting argument*/
#define ARG_BOTH_OPTIONAL	(ARG_BOTH | ARG_OPTIONAL)		/*optional argument,both name and value are necessary when inputting argument*/



#define START_SELECT_ARG		{" {",ARG_TAG,ARG_SELECT_START,0},		/*denotes that a selection group start. In help information, a "{" will be added.*/
#define DETACH_SELECT_ARG	{"|",ARG_TAG,ARG_SELECT_DETACH,0},	/*denotes that a selection group detach. In help information, a "}|{" will be added.*/
#define END_SELECT_ARG		{"}",ARG_TAG,ARG_SELECT_END,0},		/*denotes that a selection group end. In help information, a "}" will be added.*/

static const char NOT_AUTHORIZED[] = "\nNot Authorized!\n";
static const char Argerr_String[] = "get command argument error!\n";
static SECURITY_LEVEL loginLevel;	// security level: each login user has its own security level
static int exstat=1;
static int login_flag=0; // Jenny, login_flag=1 from console
static int console_flag=0; // cli forked by console

static int number1;
static int number2;
static int number3;
static int number4;
static int number5;
static int number6;
static int number7;
static int number8;
static int number9;
static int number10;
static struct in_addr ipaddr1;
static struct in_addr ipaddr2;
static struct in_addr ipaddr3;
static struct in_addr ipaddr4;
static struct in_addr ipaddr5;
static struct in_addr ipaddr6;
static struct in_addr ipmask1;
static struct in_addr ipmask2;
static struct in_addr ipmask3;
static struct in_addr ipmask4;
static struct in_addr ipmask5;
static struct in_addr ipmask6;
static char macaddr1[6];
static char macaddr2[6];
static char macaddr3[6];
static char macaddr4[6];
static char macaddr5[6];
static char macaddr6[6];
static char string1[260];
static char string2[260];
static char string3[260];
static char string4[260];
static char string5[260];
static char string6[260];
static char string7[260];
static char string8[260];
static struct arg_range range1;
static struct arg_range range2;
static struct arg_range range3;


/*note: the order of cmdArgTable must be same as ARG_ID defination*/
static struct arg_table cmdArgTable[] = {
	{ARG_START,0,0}, 		/*this empty array is needed for NUMBER1 is 1 */
	{NUMBER1,(void *)&number1,"number1"},	/*index number: NUMBER1-1 = 0*/
	{NUMBER2,(void *)&number2,"number2"},	/*index number: NUMBER2-1 = 1*/
	{NUMBER3,(void *)&number3,"number3"},
	{NUMBER4,(void *)&number4,"number4"},
	{NUMBER5,(void *)&number5,"number5"},
	{NUMBER6,(void *)&number6,"number6"},
	{NUMBER7,(void *)&number7,"number7"},
	{NUMBER8,(void *)&number8,"number8"},
	{NUMBER9,(void *)&number9,"number9"},
	{NUMBER10,(void *)&number10,"number10"},
	{IPADDR1,(void *)&ipaddr1,"ipaddr1"},
	{IPADDR2,(void *)&ipaddr2,"ipaddr2"},
	{IPADDR3,(void *)&ipaddr3,"ipaddr3"},
	{IPADDR4,(void *)&ipaddr4,"ipaddr4"},
	{IPADDR5,(void *)&ipaddr5,"ipaddr5"},
	{IPADDR6,(void *)&ipaddr6,"ipaddr6"},
	{IPMASK1,(void *)&ipmask1,"ipmask1"},
	{IPMASK2,(void *)&ipmask2,"ipmask2"},
	{IPMASK3,(void *)&ipmask3,"ipmask3"},
	{IPMASK4,(void *)&ipmask4,"ipmask4"},
	{IPMASK5,(void *)&ipmask5,"ipmask5"},
	{IPMASK6,(void *)&ipmask6,"ipmask6"},
	{MACADDR1,(void *)macaddr1,"macaddr1"},
	{MACADDR2,(void *)macaddr2,"macaddr2"},
	{MACADDR3,(void *)macaddr3,"macaddr3"},
	{MACADDR4,(void *)macaddr4,"macaddr4"},
	{MACADDR5,(void *)macaddr5,"macaddr5"},
	{MACADDR6,(void *)macaddr6,"macaddr6"},
	{STRING1,(void *)string1,"string1"},
	{STRING2,(void *)string2,"string2"},
	{STRING3,(void *)string3,"string3"},
	{STRING4,(void *)string4,"string4"},
	{STRING5,(void *)string5,"string5"},
	{STRING6,(void *)string6,"string6"},
	{STRING7,(void *)string7,"string7"},
	{STRING8,(void *)string8,"string8"},
	{RANGE1,(void *)&range1,"Number1:Number2"},	/*such as "1234:5678"*/
	{RANGE2,(void *)&range2,"Number3:Number4"},
	{RANGE3,(void *)&range3,"Number5:Number6"}
};

/*******************************function statement************************/

//static void leave(void) __attribute__ ((noreturn));	/* abort cli shell */
static void leave(void) ;	/* abort cli shell */
static int check_access(SECURITY_LEVEL level);
static void processInputCommand(char *cmdBuf);
static int getArgValue(struct cmd_arg *pCmdArg, char * argName, void * pVal);
static void listRootLevelNodes(struct cmd_node * pCmdNode, char *pMatchStr);
static void listNextLevelNodes(struct cmd_node * pCmdNode, char *pMatchStr);
static void listCommandArgs(struct cmd_node * pCmdNode, char *pMatchStr);
static void cmdEnterLinuxShell(int cmdType, struct cmd_arg *pCmdArg);
static void cmdExitCli(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowSystemStatus(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowInterface(int cmdType, struct cmd_arg *pCmdArg);
#if defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL_IGMP_SNOOPING)
static void cmdSetIgmpSnooping(int cmdType, struct cmd_arg *pCmdArg);
#endif
static void cmdCfgLanIp(int cmdType, struct cmd_arg *pCmdArg);
static void cmdPrintHelpInformation(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowInterfaceStatistics(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowAdslSetting(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowAdslStatistics(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowInterfaceLan(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowInterfaceWan(int cmdType, struct cmd_arg *pCmdArg);
static void cmdRebootSystem(int cmdType, struct cmd_arg *pCmdArg);
static void cmdSaveConfig(int cmdType, struct cmd_arg *pCmdArg);
static void cmdRestoreDefault(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowArpTbl(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowFdbTbl(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowRouteTbl(int cmdType, struct cmd_arg *pCmdArg);
#ifdef ROUTING
static void cmdShowStaticRoutes(int cmdType, struct cmd_arg *pCmdArg);
#endif
#ifdef CONFIG_USER_DHCP_SERVER
static void cmdSetDhcpMode(int cmdType, struct cmd_arg *pCmdArg);
static void cmdSetDhcpIpAssignment(int cmdType, struct cmd_arg *pCmdArg);
static void cmdDhcpRelaySetting(int cmdType, struct cmd_arg *pCmdArg);
static void cmdDhcpServerSetting(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowDhcpAssignedIp(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowDhcpClient(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowDhcpSetting(int cmdType, struct cmd_arg *pCmdArg);
#endif
#if 0
#ifdef DHCPS_DNS_OPTIONS
static void cmdSetDhcpDnsOption(int cmdType, struct cmd_arg *pCmdArg);
#endif
#endif
static void cmdSetDnsServer(int cmdType, struct cmd_arg *pCmdArg);
static void cmdDeleteWanInterface(int cmdType, struct cmd_arg *pCmdArg);
static void cmdAddWanInterface(int cmdType, struct cmd_arg *pCmdArg);
static void cmdSetAutoPvc(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowAutoPvcTbl(int cmdType, struct cmd_arg *pCmdArg);
static void cmdModifyAtmSetting(int cmdType, struct cmd_arg *pCmdArg);
static void cmdCfgAdslSetting(int cmdType, struct cmd_arg *pCmdArg);
#ifdef CONFIG_USER_IGMPPROXY
static void cmdSetIgmpproxy(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowIgmpproxy(int cmdType, struct cmd_arg *pCmdArg);
#endif
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
static void cmdSetUPnP(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowUpnp(int cmdType, struct cmd_arg *pCmdArg);
#endif
static void cmdShowAtmVcTbl(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowBridging(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowDnsServer(int cmdType, struct cmd_arg *pCmdArg);
#ifdef CONFIG_USER_ROUTED_ROUTED
static void cmdSetRip(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowRipStatusTbl(int cmdType, struct cmd_arg *pCmdArg);
#endif
static void cmdShowIpPortFiltering(int cmdType, struct cmd_arg *pCmdArg);
#ifdef IP_PORT_FILTER
static void cmdAddIpPortFiltering(int cmdType, struct cmd_arg *pCmdArg);
static void cmdDeleteIpPortFiltering(int cmdType, struct cmd_arg *pCmdArg);
static void cmdDefaultIpPortFiltering(int cmdType, struct cmd_arg *pCmdArg);
#endif
#ifdef MAC_FILTER
static void cmdSetMacFiltering(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowMacFiltering(int cmdType, struct cmd_arg *pCmdArg);
#endif
#ifdef PORT_FORWARD_GENERAL
static void cmdShowPortForwarding(int cmdType, struct cmd_arg *pCmdArg);
static void cmdAddPortForwarding(int cmdType, struct cmd_arg *pCmdArg);
static void cmdDeletePortForwarding(int cmdType, struct cmd_arg *pCmdArg);
static void cmdSetPortForwarding(int cmdType, struct cmd_arg *pCmdArg);
#endif
#ifdef URL_BLOCKING_SUPPORT
static void cmdSetUrlBlocking(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowUrlBlocking(int cmdType, struct cmd_arg *pCmdArg);
#endif
#ifdef DOMAIN_BLOCKING_SUPPORT
static void cmdShowDomainBlocking(int cmdType, struct cmd_arg *pCmdArg);
static void cmdSetDomainBlocking(int cmdType, struct cmd_arg *pCmdArg);
#endif
#ifdef DMZ
static void cmdSetDmz(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowDmz(int cmdType, struct cmd_arg *pCmdArg);
#endif
static void cmdSetBridging(int cmdType, struct cmd_arg *pCmdArg);
#ifdef ROUTING
static void cmdSetRoute(int cmdType, struct cmd_arg *pCmdArg);
#endif
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
static void cmdSetSnmp(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowSnmpSettings(int cmdType, struct cmd_arg *pCmdArg);
#endif
#if defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
static void cmdShowPortMapping(int cmdType, struct cmd_arg *pCmdArg);
static void cmdSetPortMapping(int cmdType, struct cmd_arg *pCmdArg);
static void cmdAddGroupInterface(int cmdType, struct cmd_arg *pCmdArg);
static void cmdDeleteGroupInterface(int cmdType, struct cmd_arg *pCmdArg);
#endif
#if defined(IP_QOS)
static void cmdSetIpQos(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowIpQos(int cmdType, struct cmd_arg *pCmdArg);
#endif
#ifdef REMOTE_ACCESS_CTL
static void cmdSetRemoteAccess(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowRemoteAccess(int cmdType, struct cmd_arg *pCmdArg);
#endif
static void cmdSetPassword(int cmdType, struct cmd_arg *pCmdArg);
#ifdef IP_ACL
static void cmdSetAcl(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowAclCfg(int cmdType, struct cmd_arg *pCmdArg);
#endif
#ifdef CONFIG_USER_CWMP_TR069
static void cmdSetTr069(int cmdType, struct cmd_arg *pCmdArg);
static void cmdShowTr069(int cmdType, struct cmd_arg *pCmdArg);
#endif



/****************help information********************/
static char encapsulation_help[] = "\"llc\" or \"vcmux\"";
static char enable_help[] = "\"disable\" or \"enable\"";
static char onoff_help[] = "\"on\" or \"off\"";
static char allow_help[] = "\"deny\" or \"allow\"";
static char dhcp_pool_help[] = "\"1\"(primary LAN) or \"2\"(Secondary LAN)";
static char rip_recv_mode_help[] = "\"none\", \"rip1\", \"rip2\" or \"both\"";
static char rip_send_mode_help[] = "\"none\", \"rip1\", \"rip2\" or \"rip1compat\"";
static char filtering_action_help[] = "\"deny\" or \"allow\"";
static char ipport_protocol_help[] = "\"tcp\", \"udp\" or \"icmp\"";
static char direction_help[] = "\"incoming\" or \"outgoing\"";
static char portForward_protocol_help[] = "\"tcp\", \"udp\" or \"both\"";
static char wan_interface_help[] = "WAN interface's name, or input \"any\" for all WAN interfaces";
static char interface_help[] = "interface's name, or input \"any\" for all interfaces";
static char stp_help[] = "\"on\"(enable) or \"off\"(disable)";
static char physical_port_help[] = ""
#if (defined(CONFIG_EXT_SWITCH)  && defined (IP_QOS_VPORT))
"\"lan1\", \"lan2\", \"lan3\", \"lan4\","
#else
// Mason Yu. combine_1p_4p_PortMapping
#if (defined( ITF_GROUP_1P) && defined(ITF_GROUP))
"\"eth0\", "
#else
"\"lan0\", "
#endif
#endif
#ifdef WLAN_SUPPORT
"\"wlan0\", "
#ifdef WLAN_MBSSID
"\"vap0\", \"vap1\", \"vap2\", \"vap3\", "
#endif	
#endif
#ifdef CONFIG_USB_ETH
"\"usb0\", "
#endif 
;
static char out_priority_help[] = "\"0\"(highest priority), \"1\", \"2\" or \"3\"(lowest priority)";
static char tag_8021p_help[] = "from \"0\" to \"7\"";
static char tag_precedence_help[] = "from \"0\" to \"7\"";
static char tos_help[] = "\"0\"(normal service), \"1\"(minimize cost), \"2\"(maximize reliability), \"3\"(maximize throughput) or \"4\"(minimize delay)";
static char acl_interface_help[] = "\"lan\" or \"wan\"";
static char atmqos_help[] = "\"ubr\", \"cbr\", \"rt-vbr\" or \"nrt-vbr\"";
static char portmap_interfacename_help[] = ""
#if (defined(CONFIG_EXT_SWITCH)  && defined (IP_QOS_VPORT))
"\"lan1\", \"lan2\", \"lan3\", \"lan4\", "
#else
// Mason Yu. combine_1p_4p_PortMapping
#if (defined( ITF_GROUP_1P) && defined(ITF_GROUP))
"\"eth0\", "
#else
"\"lan0\", "
#endif
#endif
#ifdef WLAN_SUPPORT
"\"wlan0\", "
#ifdef WLAN_MBSSID
"\"vap0\", \"vap1\", \"vap2\", \"vap3\", "
#endif	
#endif
#ifdef CONFIG_USB_ETH
"\"usb0\", "
#endif 
"or WAN interface's name";
static char wan_interfacename_help[] = "WAN interface's name";
static char rip_interfacename_help[] = "\"br0\" or WAN interface's name";


/****************command arguments*********************/

static struct cmd_arg lan_ip_args[] = {
	{"ip",			IPADDR1,		ARG_VALUE,			0},
	{"mask",			IPMASK1,		ARG_BOTH,			0},
#ifdef CONFIG_SECONDARY_IP
	{"secondip",		IPADDR2,		ARG_BOTH_OPTIONAL,	0},
	{"secondmask",		IPMASK2,		ARG_BOTH_OPTIONAL,	0},
	{"dhcppool",		NUMBER1,		ARG_BOTH_OPTIONAL,	dhcp_pool_help},
#endif
	{0,0,0,0}
};

static struct cmd_arg ipassign_add_args[] = {
	{"ip",			IPADDR1,		ARG_VALUE,			0},
	{"mac",			MACADDR1,	ARG_BOTH,			0},
	{0,0,0,0}
};

static struct cmd_arg ipassign_delete_args[] = {
	{"ip",			IPADDR1,		ARG_VALUE,			0},
	{0,0,0,0}
};

static struct cmd_arg dhcp_server_args[] = {
#ifdef DHCPS_POOL_COMPLETE_IP
	{"poolstart",		IPADDR1,		ARG_BOTH_OPTIONAL,	0},
	{"poolend",		IPADDR2,		ARG_BOTH_OPTIONAL,	0},
#else
	{"poolstart",		NUMBER1,		ARG_BOTH_OPTIONAL,	0},
	{"poolend",		NUMBER2,		ARG_BOTH_OPTIONAL,	0},
#endif
	{"gateway",		IPADDR3,		ARG_BOTH_OPTIONAL,	0},
	{"leasetime",		NUMBER3,		ARG_BOTH_OPTIONAL,	0},
	{"domainname",		STRING1,		ARG_BOTH_OPTIONAL,	0},
#ifdef DHCPS_DNS_OPTIONS
START_SELECT_ARG
	{"relaydns",		STRING2,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"manualdns",		STRING3,		ARG_NAME,			0},
	{"dns1",			IPADDR4,		ARG_VALUE,			0},
	{"dns2",			IPADDR5,		ARG_VALUE,			0},
	{"dns3",			IPADDR6,		ARG_VALUE,			0},
END_SELECT_ARG
#endif
	{0,0,0,0}
};

static struct cmd_arg dhcp_relay_args[] = {
	{"server",			IPADDR1,		ARG_BOTH,			0},
	{0,0,0,0}
};

static struct cmd_arg portmap_interface_args[] = {
	{"interface",		STRING1,		ARG_VALUE,			portmap_interfacename_help},
	{0,0,0,0}
};

static struct cmd_arg wan_interface_args[] = {
	{"interface",		STRING1,		ARG_VALUE,			wan_interfacename_help},
	{0,0,0,0}
};

/*
#ifdef CONFIG_USER_ROUTED_ROUTED
static struct cmd_arg rip_delete_args[] = {
	{"interface",		STRING1,		ARG_VALUE,			rip_interfacename_help},
	{0,0,0,0}
};
#endif
*/

static struct cmd_arg add_1483brg_args[] = {
	{"vpi",			NUMBER1,		ARG_BOTH,			0},
	{"vci",			NUMBER2,		ARG_BOTH,			0},
	{"encapsulation",	STRING1,		ARG_BOTH,			encapsulation_help},
START_SELECT_ARG
	{"enabled",		STRING2,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"disabled",		STRING2,		ARG_NAME,			0},
END_SELECT_ARG
	{0,0,0,0}
};

static struct cmd_arg add_1483rt_args[] = {
	{"vpi",			NUMBER1,		ARG_BOTH,			0},
	{"vci",			NUMBER2,		ARG_BOTH,			0},
	{"encapsulation",	STRING1,		ARG_BOTH,			encapsulation_help},
	{"napt",			STRING2,		ARG_BOTH_OPTIONAL,	onoff_help},
START_SELECT_ARG
	{"unnumbered",		STRING3,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"staticip",		IPADDR1,		ARG_BOTH,			0},
	{"mask",			IPMASK1,		ARG_BOTH,			0},
	{"remoteip",		IPADDR2,		ARG_BOTH,			0},
END_SELECT_ARG
#ifdef DEFAULT_GATEWAY_V1
	{"defaultgtw",		STRING4,		ARG_BOTH_OPTIONAL,	onoff_help},
#endif
START_SELECT_ARG
	{"enabled",		STRING5,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"disabled",		STRING5,		ARG_NAME,			0},
END_SELECT_ARG
	{0,0,0,0}
};

static struct cmd_arg add_mer_args[] = {
	{"vpi",			NUMBER1,		ARG_BOTH,			0},
	{"vci",			NUMBER2,		ARG_BOTH,			0},
	{"encapsulation",	STRING1,		ARG_BOTH,			encapsulation_help},
	{"napt",			STRING2,		ARG_BOTH_OPTIONAL,	onoff_help},
START_SELECT_ARG
	{"dhcp",			STRING3,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"staticip",		IPADDR1,		ARG_BOTH,			0},
	{"mask",			IPMASK1,		ARG_BOTH,			0},
	{"remoteip",		IPADDR2,		ARG_BOTH,			0},
END_SELECT_ARG
#ifdef DEFAULT_GATEWAY_V1
	{"defaultgtw",		STRING4,		ARG_BOTH_OPTIONAL,	onoff_help},
#endif
START_SELECT_ARG
	{"enabled",		STRING5,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"disabled",		STRING5,		ARG_NAME,			0},
END_SELECT_ARG
	{0,0,0,0}
};

static struct cmd_arg add_ppp_args[] = {
	{"vpi",			NUMBER1,		ARG_BOTH,			0},
	{"vci",			NUMBER2,		ARG_BOTH,			0},
	{"encapsulation",	STRING1,		ARG_BOTH,			encapsulation_help},
	{"napt",			STRING2,		ARG_BOTH_OPTIONAL,	onoff_help},
	{"username",		STRING3,		ARG_BOTH,			0},
	{"password",		STRING4,		ARG_BOTH,			0},
START_SELECT_ARG
	{"continuous",		STRING5,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"ondemand",		STRING5,		ARG_NAME,			0},
	{"idletime",		NUMBER3,		ARG_BOTH,			0},
DETACH_SELECT_ARG
	{"manual",			STRING5,		ARG_NAME,			0},
END_SELECT_ARG
#ifdef DEFAULT_GATEWAY_V1
	{"defaultgtw",		STRING6,		ARG_BOTH_OPTIONAL,	onoff_help},
#endif
START_SELECT_ARG
	{"enabled",		STRING7,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"disabled",		STRING7,		ARG_NAME,			0},
END_SELECT_ARG
	{0,0,0,0}
};
/*
static struct cmd_arg autopvc_set_args[] = {
	{"vpi",NUMBER1,0,0},
	{"vci",NUMBER2,0,0},
	{0,0,0,0}
};
*/
static struct cmd_arg wan_atm_args[] = {
	{"vpi",			NUMBER1,		ARG_BOTH,			0},
	{"vci",			NUMBER2,		ARG_BOTH,			0},
	{"qos",			STRING1,		ARG_BOTH,			atmqos_help},
	{"pcr",			NUMBER3,		ARG_BOTH,			0},
	{"cdvt",			NUMBER4,		ARG_BOTH,			0},
	{"scr",			NUMBER5,		ARG_BOTH_OPTIONAL,	0},
	{"mbs",			NUMBER6,		ARG_BOTH_OPTIONAL,	0},
	{0,0,0,0}
};

static struct cmd_arg wan_adsl_args[] = {
	{"g.lite",			NUMBER1,		ARG_NAME_OPTIONAL,	0},
	{"g.dmt",			NUMBER2,		ARG_NAME_OPTIONAL,	0},
	{"t1.413",			NUMBER3,		ARG_NAME_OPTIONAL,	0},
	{"adsl2",			NUMBER4,		ARG_NAME_OPTIONAL,	0},
	{"adsl2+",			NUMBER5,		ARG_NAME_OPTIONAL,	0},
	{"annexl",			NUMBER6,		ARG_NAME_OPTIONAL,	0},
	{"annexm",			NUMBER7,		ARG_NAME_OPTIONAL,	0},
	{"bitswap",		NUMBER8,		ARG_NAME_OPTIONAL,	0},
	{"sra",			NUMBER9,		ARG_NAME_OPTIONAL,	0},
	{0,0,0,0}
};

#ifdef CONFIG_USER_ROUTED_ROUTED
static struct cmd_arg rip_add_args[] = {
	{"interface",		STRING1,		ARG_VALUE,			rip_interfacename_help},
	{"recvmode",		STRING2,		ARG_BOTH,			rip_recv_mode_help},
	{"sendmode",		STRING3,		ARG_BOTH,			rip_send_mode_help},
	{0,0,0,0}
};
#endif

#ifdef IP_PORT_FILTER
static struct cmd_arg ipfilter_add_args[] = {
	{"direction",		STRING1,		ARG_BOTH,			direction_help},
	{"srcip",			IPADDR1,		ARG_BOTH_OPTIONAL,	0},
	{"srcmask",		IPMASK1,		ARG_BOTH_OPTIONAL,	0},
	{"dstip",			IPADDR2,		ARG_BOTH_OPTIONAL,	0},
	{"dstmask",		IPMASK2,		ARG_BOTH_OPTIONAL,	0},
	{"protocol",		STRING2,		ARG_BOTH,			ipport_protocol_help},
	{"srcport",		RANGE1,		ARG_BOTH_OPTIONAL,	0},
	{"dstport",		RANGE2,		ARG_BOTH_OPTIONAL,	0},
	{"action",			STRING3,		ARG_BOTH,			filtering_action_help},
	{0,0,0,0}
};
#endif

static struct cmd_arg index_delete_args[] = {
	{"index",			NUMBER1,		ARG_BOTH,			0},
	{0,0,0,0}
};

#ifdef IP_PORT_FILTER
static struct cmd_arg ipfilter_default_args[] = {
	{"incoming",		STRING1,		ARG_BOTH,			filtering_action_help},
	{"outgoing",		STRING2,		ARG_BOTH,			filtering_action_help},
	{0,0,0,0}
};
#endif

#ifdef MAC_FILTER
static struct cmd_arg macfilter_add_args[] = {
	{"direction",		STRING1,		ARG_BOTH,			direction_help},
	{"action",			STRING2,		ARG_BOTH,			filtering_action_help},
	{"srcmac",			MACADDR1,	ARG_BOTH_OPTIONAL,	0},
	{"dstmac",			MACADDR2,	ARG_BOTH_OPTIONAL,	0},
	{0,0,0,0}
};

static struct cmd_arg macfilter_default_args[] = {
	{"incoming",		STRING1,		ARG_BOTH,			filtering_action_help},
	{"outgoing",		STRING2,		ARG_BOTH,			filtering_action_help},
	{0,0,0,0}
};
#endif

#ifdef PORT_FORWARD_GENERAL
static struct cmd_arg portforward_add_args[] = {
	{"protocol",		STRING1,		ARG_BOTH,			portForward_protocol_help},
	{"localip",		IPADDR1,		ARG_BOTH,			0},
	{"localport",		RANGE1,		ARG_BOTH_OPTIONAL,	0},
	{"publicip",		IPADDR2,		ARG_BOTH_OPTIONAL,	0},
	{"publicport",		RANGE2,		ARG_BOTH_OPTIONAL,	0},
	{"interface",		STRING2,		ARG_BOTH_OPTIONAL,	wan_interface_help},
	{"comment",		STRING3,		ARG_BOTH_OPTIONAL,	0},
START_SELECT_ARG
	{"enabled",		STRING4,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"disabled",		STRING4,		ARG_NAME,			0},
END_SELECT_ARG
	{0,0,0,0}
};
#endif

#ifdef URL_BLOCKING_SUPPORT
static struct cmd_arg url_blocking_args[] = {
	{"fqdn",			STRING1,		ARG_BOTH_OPTIONAL,	0},
	{"keyword",		STRING2,		ARG_BOTH_OPTIONAL,	0},
	{0,0,0,0}
};
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
static struct cmd_arg domain_blocking_args[] = {
	{"domain",			STRING1,		ARG_VALUE,			0},
	{0,0,0,0}
};
#endif

#ifdef DMZ
static struct cmd_arg dmz_host_args[] = {
	{"ip",			IPADDR1,		ARG_VALUE,			0},
	{0,0,0,0}
};
#endif

static struct cmd_arg cfg_bridging_args[] = {
	{"aging",			NUMBER1,		ARG_BOTH_OPTIONAL,	0},
	{"stp",			STRING1,		ARG_BOTH_OPTIONAL,	stp_help},
	{0,0,0,0}
};

#ifdef ROUTING
static struct cmd_arg route_add_args[] = {
	{"destination",		IPADDR1,		ARG_VALUE,			0},
	{"mask",			IPMASK1,		ARG_BOTH,			0},
	{"nexthop",		IPADDR2,		ARG_BOTH_OPTIONAL,	0},
	{"metric",			NUMBER1,		ARG_BOTH_OPTIONAL,	0},
	{"interface",		STRING1,		ARG_BOTH_OPTIONAL,	interface_help},
START_SELECT_ARG
	{"enabled",		STRING2,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"disabled",		STRING2,		ARG_NAME,			0},
END_SELECT_ARG
	{0,0,0,0}
};

static struct cmd_arg route_delete_args[] = {
	{"destination",		IPADDR1,		ARG_VALUE,			0},
	{"mask",			IPMASK1,		ARG_BOTH,			0},
	{0,0,0,0}
};
#endif

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
static struct cmd_arg cfg_snmp_args[] = {
	{"description",		STRING1,		ARG_BOTH_OPTIONAL,	0},
	{"contact",		STRING2,		ARG_BOTH_OPTIONAL,	0},
	{"systemname",		STRING3,		ARG_BOTH_OPTIONAL,	0},
	{"location",		STRING4,		ARG_BOTH_OPTIONAL,	0},
	{"objectid",		STRING5,		ARG_BOTH_OPTIONAL,	0},
	{"trapip",			IPADDR1,		ARG_BOTH_OPTIONAL,	0},
	{"readcommunity",	STRING6,		ARG_BOTH_OPTIONAL,	0},
	{"writecommunity",	STRING7,		ARG_BOTH_OPTIONAL,	0},
	{0,0,0,0}
};
#endif

#if defined(IP_QOS)
static struct cmd_arg ipqos_add_args[] = {
	{"srcip",			IPADDR1,		ARG_BOTH_OPTIONAL,	0},
	{"srcmask",		IPMASK1,		ARG_BOTH_OPTIONAL,	0},
	{"dstip",			IPADDR2,		ARG_BOTH_OPTIONAL,	0},
	{"dstmask",		IPMASK2,		ARG_BOTH_OPTIONAL,	0},
	{"protocol",		STRING1,		ARG_BOTH_OPTIONAL,	ipport_protocol_help},
	{"srcport",		NUMBER1,		ARG_BOTH_OPTIONAL,	0},
	{"dstport",		NUMBER2,		ARG_BOTH_OPTIONAL,	0},
	{"phyport",		STRING2,		ARG_BOTH_OPTIONAL,	physical_port_help},
	{"priority",		NUMBER3,		ARG_BOTH,			out_priority_help},
	{"802.1p",			NUMBER4,		ARG_BOTH_OPTIONAL,	tag_8021p_help},
	{"precedence",		NUMBER5,		ARG_BOTH_OPTIONAL,	tag_precedence_help},
	{"tos",			NUMBER6,		ARG_BOTH_OPTIONAL,	tos_help},
	{0,0,0,0}
};

#endif

static struct cmd_arg remote_access_port_args[] = {
	{"lan",			STRING1,		ARG_BOTH,			allow_help},
	{"wan",			STRING2,		ARG_BOTH,			allow_help},
	{"wanport",		NUMBER1,		ARG_BOTH_OPTIONAL,	0},
	{0,0,0,0}
};

static struct cmd_arg remote_access_args[] = {
	{"lan",			STRING1,		ARG_BOTH,			allow_help},
	{"wan",			STRING2,		ARG_BOTH,			allow_help},
	{0,0,0,0}
};

static struct cmd_arg cfg_password_args[] = {
	{"user",			STRING1,		ARG_BOTH,			0},
	{"oldpasswd",		STRING2,		ARG_BOTH,			0},
	{"newpasswd",		STRING3,		ARG_BOTH,			0},
	{0,0,0,0}
};

#ifdef IP_ACL
static struct cmd_arg acl_add_args[] = {
	{"ip",			IPADDR1,		ARG_BOTH,			0},
	{"mask",			IPMASK1,		ARG_BOTH,			0},
	{"interface",		STRING1,		ARG_BOTH,			acl_interface_help},
START_SELECT_ARG
	{"enabled",		STRING2,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"disabled",		STRING3,		ARG_NAME,			0},
END_SELECT_ARG
	{0,0,0,0}
};

#endif

#ifdef CONFIG_USER_CWMP_TR069
static struct cmd_arg tr069_acs_args[] = {
	{"url",			STRING1,		ARG_BOTH_OPTIONAL,	0},
	{"username",		STRING2,		ARG_BOTH_OPTIONAL,	0},
	{"password",		STRING3,		ARG_BOTH_OPTIONAL,	0},
	{"inform",			STRING4,		ARG_BOTH_OPTIONAL,	enable_help},
	{"interval",		NUMBER2,		ARG_BOTH_OPTIONAL,	0},
	{0,0,0,0}
};

static struct cmd_arg tr069_request_args[] = {
	{"username",		STRING1,		ARG_BOTH_OPTIONAL,	0},
	{"password",		STRING2,		ARG_BOTH_OPTIONAL,	0},
	{"path",			STRING3,		ARG_BOTH_OPTIONAL,	0},
	{"port",			NUMBER1,		ARG_BOTH_OPTIONAL,	0},
	{0,0,0,0}
};
#endif

#if 0
#ifdef DHCPS_DNS_OPTIONS
static struct cmd_arg dhcp_dns_args[] = {
START_SELECT_ARG
	{"relay",			STRING1,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"dns1",			IPADDR1,		ARG_VALUE,			0},
	{"dns2",			IPADDR2,		ARG_VALUE,			0},
	{"dns3",			IPADDR3,		ARG_VALUE,			0},
END_SELECT_ARG
	{0,0,0,0}
};
#endif
#endif

static struct cmd_arg dhcp_mode_args[] = {
START_SELECT_ARG
	{"none",			STRING1,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"relay",			STRING1,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"server",			STRING1,		ARG_NAME,			0},
END_SELECT_ARG
	{0,0,0,0}
};

static struct cmd_arg dns_server_args[] = {
START_SELECT_ARG
	{"auto",			STRING1,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"dns1",			IPADDR1,		ARG_VALUE,			0},
	{"dns2",			IPADDR2,		ARG_VALUE,			0},
	{"dns3",			IPADDR3,		ARG_VALUE,			0},
END_SELECT_ARG
	{0,0,0,0}
};

#if defined(IP_QOS)
static struct cmd_arg ipqos_default_args[] = {
START_SELECT_ARG
	{"802.1p",			STRING1,		ARG_NAME,			0},
DETACH_SELECT_ARG
	{"ippred",			STRING1,		ARG_NAME,			0},
END_SELECT_ARG
	{0,0,0,0}
};
#endif	

/**********************command nodes***********************/

static struct cmd_node show_interface_nodes[] = {
	{"statistics",	END, CMD_SHOW_INTERFACE_STATISTICS,	0,	0,	cmdShowInterfaceStatistics,		0,	"Interface statistics"},
	{0,0,0,0,0,0,0,0}
};

static struct cmd_node show_adsl_nodes[] = {
	{"setting",	END, CMD_SHOW_ADSL_SETTING,		0,	0,	cmdShowAdslSetting,	0,	"Adsl setting"},
	{"statistics",	END, CMD_SHOW_ADSL_STATISTICS,	0,	0,	cmdShowAdslStatistics,	0,	"Adsl statistics"},
	{0,0,0,0,0,0,0,0}
};

#ifdef ROUTING
static struct cmd_node show_routes_nodes[] = {
	{"static",		END, CMD_SHOW_STATIC_ROUTES,		0,	0,	cmdShowStaticRoutes,	0,	"Static routes"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef CONFIG_USER_DHCP_SERVER
static struct cmd_node show_dhcp_nodes[] = {
	{"ipreserve",	END, CMD_SHOW_DCHP_ASSIGNED_IP,	0,	0,	cmdShowDhcpAssignedIp,	0,	"Assigned ip"},
	{"client",		END, CMD_SHOW_DCHP_CLIENT,		0,	0,	cmdShowDhcpClient,		0,	"Dhcp clients"},
	{"setting",	END, CMD_SHOW_DCHP_SETTING,		0,	0,	cmdShowDhcpSetting,		0,	"Dhcp setting"},
	{0,0,0,0,0,0,0,0}
};
#endif

static struct cmd_node show_wan_nodes[] = {
	{"adsl",		0, 	0,						show_adsl_nodes,	0, 0,						listNextLevelNodes, "Adsl settings"},
	{"atm",		END,	CMD_SHOW_ATM_VC_TBL,	0,				0, cmdShowAtmVcTbl,		0,				 "ATM VC table"},
//	{"autopvc",	END,	CMD_SHOW_AUTO_PVC,			0,				0, cmdShowAutoPvcTbl,		0,				 "Auto-pvc search table"},
	{"interface",	END,	CMD_SHOW_INTERFACE_WAN,	0,				0, cmdShowInterfaceWan,	0,				 "WAN interface"},
	{0,0,0,0,0,0,0,0}
};

static struct cmd_node show_bridge_nodes[] = {
	{"macs",	END, CMD_SHOW_FDB_TBL,	0,	0,	cmdShowFdbTbl,	0,	"Bridge forwarding table"},
	{0,0,0,0,0,0,0,0}
};

#if defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL_IGMP_SNOOPING)
static struct cmd_node lan_igmp_nodes[] = {
	{"disable",	END,	CMD_DISABLE_IGMP_SNOOPING,	0,	0,	cmdSetIgmpSnooping,	0,	0},
	{"enable",		END,	CMD_ENABLE_IGMP_SNOOPING,	0,	0,	cmdSetIgmpSnooping,	0,	0},
	{0,0,0,0,0,0,0,0}
};
#endif

static struct cmd_node cfg_lan_nodes[] = {
#if defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL_IGMP_SNOOPING)
	{"igmpsnoop",	0, 	0,				lan_igmp_nodes,	0,			0,			listNextLevelNodes,	"Igmp snooping setting"},
#endif
	{"ip",		END,	CMD_CFG_LAN_IP,	0,				lan_ip_args,	cmdCfgLanIp,	listCommandArgs,		"IP/mask address"},
	{0,0,0,0,0,0,0,0}
};

static struct cmd_node channel_add_nodes[] = {
	{"br2684",		END,	CMD_ADD_1483BRG_PVC,	0,	add_1483brg_args,	cmdAddWanInterface,	listCommandArgs,	"Create 1483 bridged pvc"},
	{"mer",		END,	CMD_ADD_1483MER_PVC,	0,	add_mer_args,		cmdAddWanInterface,	listCommandArgs,	"Create 1483 mer pvc"},
	{"rt2684",		END,	CMD_ADD_1483RT_PVC,		0,	add_1483rt_args,		cmdAddWanInterface,	listCommandArgs,	"Create 1483 routed pvc"},
	{"pppoa",		END,	CMD_ADD_PPPOA_PVC,		0,	add_ppp_args,		cmdAddWanInterface,	listCommandArgs,	"Create PPPoA pvc"},
	{"pppoe",		END,	CMD_ADD_PPPOE_PVC,		0,	add_ppp_args,		cmdAddWanInterface,	listCommandArgs,	"Create PPPoE pvc"},
	{0,0,0,0,0,0,0,0}
};
/*
static struct cmd_node channel_autopvc_nodes[] = {
	{"add",	END, CMD_ADD_AUTO_PVC,		0,autopvc_set_args,	cmdSetAutoPvc,	listCommandArgs,	"Add auto-pvc"},
	{"delete",	END, CMD_DELETE_AUTO_PVC,		0,autopvc_set_args,	cmdSetAutoPvc,	listCommandArgs,	"Delete auto-pvc"},
	{"disable",END, CMD_DISABLE_AUTO_PVC,	0,0,				cmdSetAutoPvc,	0,				"Disable auto-pvc"},
	{"enable",	END, CMD_ENABLE_AUTO_PVC,	0,0,				cmdSetAutoPvc,	0,				"Enable auto-pvc"},
	{0,0,0,0,0,0,0,0}
};
*/
static struct cmd_node wan_channel_nodes[] = {
	{"add",		0, 	0,							channel_add_nodes,	0,					0,						listNextLevelNodes,	"Add interface"},
//	{"autopvc",	0, 	0,							channel_autopvc_nodes,	0,					0,						listNextLevelNodes,		"Auto-pvc search"},
	{"delete",		END,	CMD_DELETE_WAN_INTERFACE,	0,					wan_interface_args,	cmdDeleteWanInterface,	listCommandArgs,		"Delete interface"},
	{0,0,0,0,0,0,0,0}
};

static struct cmd_node cfg_wan_nodes[] = {
	{"adsl",		END, CMD_CFG_ADSL_SETTING,	0,					wan_adsl_args,	cmdCfgAdslSetting,	listCommandArgs,	"Adsl settings"},
	{"atm",		END, CMD_MODIFY_ATM_SETTING,	0,					wan_atm_args,	cmdModifyAtmSetting,	listCommandArgs,	"Atm settings"},
	{"interface",	0, 	0,						wan_channel_nodes,	0,				0,					listNextLevelNodes,"Channel configuration"},
	{0,0,0,0,0,0,0,0}
};


static struct cmd_node dhcp_ipassign_nodes[] = {
	{"add",	END, CMD_ADD_ASSIGN_IP,		0,	ipassign_add_args,		cmdSetDhcpIpAssignment,	listCommandArgs,	"Assign ip"},
	{"delete",	END, CMD_DELETE_ASSIGN_IP,		0,	ipassign_delete_args,	cmdSetDhcpIpAssignment,	listCommandArgs,	"Delete assigned ip"},
	{0,0,0,0,0,0,0,0}
};

#ifdef CONFIG_USER_DHCP_SERVER
static struct cmd_node cfg_dhcp_nodes[] = {
	{"mode",		END,	CMD_DHCP_MODE,			0,					dhcp_mode_args,	cmdSetDhcpMode,		listCommandArgs,		"DHCP mode"},
	{"relay",		END,	CMD_DHCP_RELAY_SETTING,	0,					dhcp_relay_args,	cmdDhcpRelaySetting,	listCommandArgs,		"DHCP relay setting"},
	{"server",		END,	CMD_DHCP_SERVER_SETTING,	0,					dhcp_server_args,	cmdDhcpServerSetting,	listCommandArgs,		"DHCP server setting"},
	{"ipreserve",	0,	0,						dhcp_ipassign_nodes,	0,				0,					listNextLevelNodes,	"DHCP ip assignment"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef DMZ
static struct cmd_node cfg_dmz_nodes[] = {
 	{"disable",	END,	CMD_DISABLE_DMZ,		0,	0,				cmdSetDmz,	0,				"Disable DMZ"},
	{"enable",		END,	CMD_ENABLE_DMZ,		0,	dmz_host_args,	cmdSetDmz,	listCommandArgs,	"Enable DMZ"},

 	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef CONFIG_USER_IGMPPROXY
static struct cmd_node cfg_igmpprxoy_nodes[] = {
 	{"disable",	END,	CMD_DISABLE_IGMP_PROXY,			0,	0,					cmdSetIgmpproxy,		0,				"Disable igmp_proxy"},
	{"enable",		END,	CMD_ENABLE_IGMP_PROXY,			0,	wan_interface_args,	cmdSetIgmpproxy,		listCommandArgs,	"Enable igmp_proxy"},

	{0,0,0,0,0,0,0,0}
};
#endif

#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
static struct cmd_node cfg_upnp_nodes[] = {
 	{"disable",	END,	CMD_DISABLE_UPNP,		0,	0,					cmdSetUPnP,	0,				"Disable UPnP"},
	{"enable",		END,	CMD_ENABLE_UPNP,			0,	wan_interface_args,	cmdSetUPnP,	listCommandArgs,	"Enable UPnP"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef CONFIG_USER_ROUTED_ROUTED
static struct cmd_node cfg_rip_nodes[] = {
	{"add",		END,	CMD_ADD_RIP_INTERFACE,	0,	rip_add_args,			cmdSetRip,	listCommandArgs,	"Add RIP interface"},
	{"delete",		END,	CMD_DELETE_RIP_INTERFACE,	0,	index_delete_args,		cmdSetRip,	listCommandArgs,	"Delete RIP interface"},
 	{"disable",	END,	CMD_DISABLE_RIP,			0,	0,					cmdSetRip,	0,				"Disable RIP"},
	{"enable",		END,	CMD_ENABLE_RIP,			0,	0,					cmdSetRip,	0,				"Enable RIP"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef ROUTING
static struct cmd_node cfg_route_nodes[] = {
	{"add",	END,	CMD_ADD_ROUTE,		0,	route_add_args,		cmdSetRoute,	listCommandArgs,	"Add route"},
	{"delete",	END,	CMD_DELETE_ROUTE,	0,	route_delete_args,		cmdSetRoute,	listCommandArgs,	"Delete route"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef IP_PORT_FILTER
static struct cmd_node cfg_ipfiltering_nodes[] = {
 	{"add",		END,	CMD_ADD_IPPORT_FILTERING,			0,	ipfilter_add_args,		cmdAddIpPortFiltering,		listCommandArgs,	"Add ip port filtering rule"},
 	{"delete",		END,	CMD_DELETE_IPPORT_FILTERING,		0,	index_delete_args,		cmdDeleteIpPortFiltering,	listCommandArgs,	"Delete ip port filtering rule"},
 	{"default",	END,	CMD_IPPORT_FILTER_DEFAULT_ACTION,	0,	ipfilter_default_args,	cmdDefaultIpPortFiltering,	listCommandArgs,	"Ip port filtering default action"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef MAC_FILTER
static struct cmd_node cfg_macfiltering_nodes[] = {
 	{"add",		END,	CMD_ADD_MAC_FILTERING,			0,	macfilter_add_args,	cmdSetMacFiltering,		listCommandArgs,	"Add MAC filtering rule"},
 	{"delete",		END,	CMD_DELETE_MAC_FILTERING,			0,	index_delete_args,		cmdSetMacFiltering,		listCommandArgs,	"Delete MAC filtering rule"},
 	{"default",	END,	CMD_MAC_FILTER_DEFAULT_ACTION,	0,	macfilter_default_args,	cmdSetMacFiltering,		listCommandArgs,	"MAC filtering default action"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef PORT_FORWARD_GENERAL
static struct cmd_node cfg_portforwarding_nodes[] = {
 	{"add",		END,	CMD_ADD_PORT_FORWARDING,	0,	portforward_add_args,		cmdAddPortForwarding,		listCommandArgs,	"Add port forwarding rule"},
 	{"delete",		END,	CMD_DELETE_PORT_FORWARDING,	0,	index_delete_args,			cmdDeletePortForwarding,	listCommandArgs,	"Delete port forwarding rule"},
 	{"disable",	END,	CMD_DISABLE_PORT_FORWARDING,	0,	0,						cmdSetPortForwarding,		0,				"Disable port forwarding"},
	{"enable",		END,	CMD_ENABLE_PORT_FORWARDING,	0,	0,						cmdSetPortForwarding,		0,				"Enable port forwarding"},
	{0,0,0,0,0,0,0,0}
};
#endif

#if defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
static struct cmd_node portmap_group1_nodes[] = {
 	{"add",		END,	CMD_ADD_GROUP1_INTERFACE,	0,	portmap_interface_args,	cmdAddGroupInterface,		listCommandArgs,	"Add interface to group1"},
 	{"delete",		END,	CMD_DELETE_GROUP1_INTERFACE,	0,	portmap_interface_args,	cmdDeleteGroupInterface,	listCommandArgs,	"Delete interface from group1"},
	{0,0,0,0,0,0,0,0}
};

static struct cmd_node portmap_group2_nodes[] = {
 	{"add",		END,	CMD_ADD_GROUP2_INTERFACE,	0,	portmap_interface_args,	cmdAddGroupInterface,		listCommandArgs,	"Add interface to group2"},
 	{"delete",		END,	CMD_DELETE_GROUP2_INTERFACE,	0,	portmap_interface_args,	cmdDeleteGroupInterface,	listCommandArgs,	"Delete interface from group2"},
	{0,0,0,0,0,0,0,0}
};

static struct cmd_node portmap_group3_nodes[] = {
 	{"add",		END,	CMD_ADD_GROUP3_INTERFACE,	0,	portmap_interface_args,	cmdAddGroupInterface,		listCommandArgs,	"Add interface to group3"},
 	{"delete",		END,	CMD_DELETE_GROUP3_INTERFACE,	0,	portmap_interface_args,	cmdDeleteGroupInterface,	listCommandArgs,	"Delete interface from group3"},
	{0,0,0,0,0,0,0,0}
};

static struct cmd_node portmap_group4_nodes[] = {
 	{"add",		END,	CMD_ADD_GROUP4_INTERFACE,	0,	portmap_interface_args,	cmdAddGroupInterface,		listCommandArgs,	"Add interface to group4"},
 	{"delete",		END,	CMD_DELETE_GROUP4_INTERFACE,	0,	portmap_interface_args,	cmdDeleteGroupInterface,	listCommandArgs,	"Delete interface from group4"},
	{0,0,0,0,0,0,0,0}
};

static struct cmd_node cfg_portmapping_nodes[] = {
 	{"disable",	END,	CMD_DISABLE_PORT_MAPPING,	0,						0,	cmdSetPortMapping,	0,					"Disable port mapping"},
	{"enable",		END,	CMD_ENABLE_PORT_MAPPING,		0,						0,	cmdSetPortMapping,	0,					"Enable port mapping"},
	{"group1",		0,	0,							portmap_group1_nodes,	0,	0,					listNextLevelNodes,	"Set group1"},
	{"group2",		0,	0,							portmap_group2_nodes,	0,	0,					listNextLevelNodes,	"Set group2"},
	{"group3",		0,	0,							portmap_group3_nodes,	0,	0,					listNextLevelNodes,	"Set group3"},
	{"group4",		0,	0,							portmap_group4_nodes,	0,	0,					listNextLevelNodes,	"Set group4"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef URL_BLOCKING_SUPPORT
static struct cmd_node cfg_urlblocking_nodes[] = {
 	{"add",		END,	CMD_ADD_URL_BLOCKING,	0,	url_blocking_args,	cmdSetUrlBlocking,		listCommandArgs,	"Add URL blocking rule"},
 	{"delete",		END,	CMD_DELETE_URL_BLOCKING,	0,	url_blocking_args,	cmdSetUrlBlocking,		listCommandArgs,	"Delete URL blocking rule"},
 	{"disable",	END,	CMD_DISABLE_URL_BLOCKING,0,	0,				cmdSetUrlBlocking,		0,				"Disable URL blocking"},
	{"enable",		END,	CMD_ENABLE_URL_BLOCKING,	0,	0,				cmdSetUrlBlocking,		0,				"Enable URL blocking"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
static struct cmd_node cfg_domainblocking_nodes[] = {
 	{"add",		END,	CMD_ADD_DOMAIN_BLOCKING,	0,	domain_blocking_args,	cmdSetDomainBlocking,		listCommandArgs,	"Add domain blocking rule"},
 	{"delete",		END,	CMD_DELETE_DOMAIN_BLOCKING,	0,	domain_blocking_args,	cmdSetDomainBlocking,		listCommandArgs,	"Delete domain blocking rule"},
 	{"disable",	END,	CMD_DISABLE_DOMAIN_BLOCKING,	0,	0,					cmdSetDomainBlocking,		0,				"Disable domain blocking"},
	{"enable",		END,	CMD_ENABLE_DOMAIN_BLOCKING,	0,	0,					cmdSetDomainBlocking,		0,				"Enable domain blocking"},
	{0,0,0,0,0,0,0,0}
};
#endif

#if defined(IP_QOS)
static struct cmd_node cfg_ipqos_nodes[] = {
	{"default",	END,	CMD_SET_DEFAULT_QOS,		0,	ipqos_default_args,	cmdSetIpQos,		listCommandArgs,		"Set default qos rule"},
	{"disable",	END,	CMD_DISABLE_IP_QOS,		0,	0,					cmdSetIpQos,		0,					"Disable IP QoS"},
	{"enable",		END,	CMD_ENABLE_IP_QOS,		0,	0,					cmdSetIpQos,		0,					"Enable IP QoS"},
	{"add",		END,	CMD_ADD_IPQOS_RULE,		0,	ipqos_add_args,		cmdSetIpQos,		listCommandArgs,		"Add IP QoS rule"},
 	{"delete",		END,	CMD_DELETE_IPQOS_RULE,	0,	index_delete_args,		cmdSetIpQos,		listCommandArgs,		"Delete IP QoS rule"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef REMOTE_ACCESS_CTL
static struct cmd_node cfg_remote_access_nodes[] = {
 	{"ftp",	END,	CMD_SET_FTP_ACCESS,		0,	remote_access_port_args,	cmdSetRemoteAccess,	listCommandArgs,	"FTP access"},
 	{"http",	END,	CMD_SET_WEB_ACCESS,		0,	remote_access_port_args,	cmdSetRemoteAccess,	listCommandArgs,	"Http access"},
 	{"ping",	END,	CMD_SET_PING_ACCESS,		0,	remote_access_args,		cmdSetRemoteAccess,	listCommandArgs,	"Ping access"},
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	{"snmp",	END,	CMD_SET_SNMP_ACCESS,	0,	remote_access_args,		cmdSetRemoteAccess,	listCommandArgs,	"SNMP access"},
#endif
#ifdef CONFIG_USER_TELNETD_TELNETD
 	{"telnet",	END,	CMD_SET_TELNET_ACCESS,	0,	remote_access_port_args,	cmdSetRemoteAccess,	listCommandArgs,	"Telnet access"},
#endif
#ifdef CONFIG_USER_TFTPD_TFTPD
	{"tftp",	END,	CMD_SET_TFTP_ACCESS,		0,	remote_access_args,		cmdSetRemoteAccess,	listCommandArgs,	"TFTP access"},
#endif
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef IP_ACL
static struct cmd_node cfg_acl_nodes[] = {
 	{"add",		END,	CMD_ADD_ACL,	0,	acl_add_args,			cmdSetAcl,	listCommandArgs,	"Add access control list"},
 	{"delete",		END,	CMD_DELETE_ACL,	0,	index_delete_args,		cmdSetAcl,	listCommandArgs,	"Delete access control list"},
 	{"disable",	END,	CMD_DISABLE_ACL,	0,	0,					cmdSetAcl,	0,				"Disable access control list"},
	{"enable",		END,	CMD_ENABLE_ACL,	0,	0,					cmdSetAcl,	0,				"Enable access control list"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
static struct cmd_node cfg_snmp_nodes[] = {
 	{"disable",	END,	CMD_DISABLE_SNMP,	0,	0,					cmdSetSnmp,		0,				"Disable SNMP"},
	{"enable",		END,	CMD_ENABLE_SNMP,	0,	0,					cmdSetSnmp,		0,				"Enable SNMP"},
 	{"setting",	END,	CMD_CONFIG_SNMP,	0,	cfg_snmp_args,		cmdSetSnmp,		listCommandArgs,	"Set SNMP parameters"},
	{0,0,0,0,0,0,0,0}
};
#endif

#ifdef CONFIG_USER_CWMP_TR069
static struct cmd_node cfg_tr069_nodes[] = {
 	{"acs",		END,	CMD_CFG_TR069_ACS,				0,	tr069_acs_args,		cmdSetTr069,		listCommandArgs,	"Set TR069 ACS"},
 	{"request",	END,	CMD_CFG_TR069_CONNECT_REQUEST,	0,	tr069_request_args,	cmdSetTr069,		listCommandArgs,	"Set TR069 connect request"},
 	{"disable",	END,	CMD_DISABLE_TR069,				0,	0,					cmdSetTr069,		0,				"Disable TR069"},
	{"enable",		END,	CMD_ENABLE_TR069,				0,	0,					cmdSetTr069,		0,				"Enable TR069"},
	{0,0,0,0,0,0,0,0}
};
#endif

static struct cmd_node cfg_account_nodes[] = {
	{"password",		END, 	CMD_SET_PASSWORD,	0,	cfg_password_args,	cmdSetPassword,		listCommandArgs,		"Set password"},
	{0,0,0,0,0,0,0,0}
};


static struct cmd_node cmd_config_nodes[] = {
	{"account",		SUSERONLY, 		0,					cfg_account_nodes,		0,					0,					listNextLevelNodes,	"Set account"},
#ifdef IP_ACL
	{"acl",			SUSERONLY,		0,					cfg_acl_nodes,			0,					0,					listNextLevelNodes,	"Config access control list"},
#endif
 	{"bridge",			SUSERONLY | END,	CMD_SET_BRIDGING,	0,						cfg_bridging_args,		cmdSetBridging,		listCommandArgs,		"Set bridging ageing time and STP"},
#ifdef CONFIG_USER_DHCP_SERVER
	{"dhcp",			SUSERONLY,		0,					cfg_dhcp_nodes,			0,					0,					listNextLevelNodes,	"Config DHCP"},
#endif
	{"dmz",			SUSERONLY,		0,					cfg_dmz_nodes,			0,					0,					listNextLevelNodes,	"Config DMZ"},
	{"dns",			SUSERONLY | END,	CMD_SET_DNS_SERVER,	0,						dns_server_args,		cmdSetDnsServer,		listCommandArgs,		"Config DNS server"},
#ifdef DOMAIN_BLOCKING_SUPPORT
	{"domainblock",		SUSERONLY,		0,					cfg_domainblocking_nodes,	0,					0,					listNextLevelNodes,	"Domain blocking"},
#endif
#ifdef CONFIG_USER_IGMPPROXY
	{"igmpproxy",		SUSERONLY, 		0,					cfg_igmpprxoy_nodes,		0,					0,					listNextLevelNodes,	"Config igmp_proxy"},
#endif
#ifdef IP_PORT_FILTER
	{"ipportfilter",	SUSERONLY, 		0,					cfg_ipfiltering_nodes,		0,					0,					listNextLevelNodes,	"Ip port filtering"},
#endif
#if defined(IP_QOS)
	{"ipqos",			SUSERONLY, 		0,					cfg_ipqos_nodes,			0,					0,					listNextLevelNodes,	"Config IP QoS"},
#endif
	{"lan",			SUSERONLY, 		0,					cfg_lan_nodes,			0,					0,					listNextLevelNodes,	"Config LAN interface"},
#ifdef MAC_FILTER
	{"macfilter",		SUSERONLY, 		0,					cfg_macfiltering_nodes,		0,					0,					listNextLevelNodes,	"Set MAC filtering"},
#endif
#if defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
	{"portbind",		SUSERONLY, 		0,					cfg_portmapping_nodes,	0,					0,					listNextLevelNodes,	"Set port mapping"},
#endif
#ifdef PORT_FORWARD_GENERAL
	{"portforward",		SUSERONLY, 		0,					cfg_portforwarding_nodes,	0,					0,					listNextLevelNodes,	"Set port forwarding"},
#endif
#ifdef REMOTE_ACCESS_CTL
	{"remoteaccess",	SUSERONLY, 		0,					cfg_remote_access_nodes,	0,					0,					listNextLevelNodes,	"Config remote access"},
#endif
	{"reset",			SUSERONLY | END,	CMD_RESTORE_DEFAULT,	0,						0,					cmdRestoreDefault,	0,					"Restore to default setting"},	
#ifdef CONFIG_USER_ROUTED_ROUTED
	{"rip",			SUSERONLY, 		0,					cfg_rip_nodes,			0,					0,					listNextLevelNodes,	"Config RIP"},
#endif
#ifdef ROUTING
	{"route",			SUSERONLY, 		0,					cfg_route_nodes,			0,					0,					listNextLevelNodes,	"Add/Delete routes"},
#endif
	{"save",			SUSERONLY | END, 	CMD_SAVE_CONFIG,		0,						0,					cmdSaveConfig,		0,					"Save configuration"},
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	{"snmp",			SUSERONLY , 		0,					cfg_snmp_nodes,			0,					0,					listNextLevelNodes,	"Config SNMP"},
#endif
#ifdef CONFIG_USER_CWMP_TR069
	{"tr069",			SUSERONLY, 		0,					cfg_tr069_nodes,			0,					0,					listNextLevelNodes,	"Config TR069"},
#endif
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	{"upnp",			SUSERONLY, 		0,					cfg_upnp_nodes,			0,					0,					listNextLevelNodes,	"Config UPnP"},
#endif
#ifdef URL_BLOCKING_SUPPORT
	{"urlblock",		SUSERONLY, 		0,					cfg_urlblocking_nodes,		0,					0,					listNextLevelNodes,	"Set URL blocking"},
#endif
	{"wan",			SUSERONLY, 		0,					cfg_wan_nodes,			0,					0,					listNextLevelNodes,	"Config WAN interface"},
	{0,0,0,0,0,0,0,0}
};

static struct cmd_node cmd_debug_nodes[] = {
	{0,0,0,0,0,0,0,0}
};

static struct cmd_node cmd_show_nodes[] = {
#ifdef IP_ACL
	{"acl",			END,	CMD_SHOW_ACL_CONFIG,		0,					0,	cmdShowAclCfg,			0,					"Access control list"},
#endif
	{"arp",			END,	CMD_SHOW_ARP_TBL,			0,					0,	cmdShowArpTbl,			0,					"Arp table"},
	{"bridge",			END,	CMD_SHOW_BRIDGING,			show_bridge_nodes,	0,	cmdShowBridging,			listNextLevelNodes,	"bridge settings"},
#ifdef CONFIG_USER_DHCP_SERVER
	{"dhcp",			0, 	0,							show_dhcp_nodes,		0,	0,						listNextLevelNodes,	"Dhcp status"},
#endif
	{"dmz",			END,	CMD_SHOW_DMZ,				0,					0,	cmdShowDmz,			0,					"DMZ setting"},
	{"dns",			END,	CMD_SHOW_DNS_SERVER,		0,					0,	cmdShowDnsServer,		0,					"DNS server settings"},
#ifdef DOMAIN_BLOCKING_SUPPORT
	{"domainblock",		END,	CMD_SHOW_DOMAIN_BLOCKING,	0,					0,	cmdShowDomainBlocking,	0,					"Domain blocking"},
#endif
#ifdef CONFIG_USER_IGMPPROXY
	{"igmpproxy",		END,	CMD_SHOW_IGMP_PROXY,		0,					0,	cmdShowIgmpproxy,		0,					"Igmp proxy settings"},
#endif
	{"interface",		END,	CMD_SHOW_INTERFACE,			show_interface_nodes,	0,	cmdShowInterface,		listNextLevelNodes,	"Interface information"},
#if defined(IP_QOS)
	{"ipqos",			END,	CMD_SHOW_IP_QOS,			0,					0,	cmdShowIpQos,			0,					"IP QoS"},
#endif
#ifdef IP_PORT_FILTER
	{"ipportfilter",	END,	CMD_SHOW_IP_PORT_FILTERING,	0,					0,	cmdShowIpPortFiltering,	0,					"IP port filtering"},
#endif
	{"lan",			END,	CMD_SHOW_LAN_SETTINGS,		0,					0,	cmdShowInterfaceLan,		0,					"LAN settings"},
#ifdef MAC_FILTER
	{"macfilter",		END,	CMD_SHOW_MAC_FILTERING,		0,					0,	cmdShowMacFiltering,		0,					"MAC filtering"},
#endif
#if defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
	{"portbind",		END,	CMD_SHOW_PORT_MAPPING,		0,					0,	cmdShowPortMapping,		0,					"Port mapping"},
#endif
	{"portforward",		END,	CMD_SHOW_PORT_FORWARDING,	0,					0,	cmdShowPortForwarding,	0,					"Port forwarding"},
#ifdef REMOTE_ACCESS_CTL
	{"remoteaccess",	END,	CMD_SHOW_REMOTE_ACCESS,	0,					0,	cmdShowRemoteAccess,	0,					"Remote access status"},
#endif
#ifdef CONFIG_USER_ROUTED_ROUTED
	{"rip",			END,	CMD_SHOW_RIP_STATUS_TBL,		0,					0,	cmdShowRipStatusTbl,		0,					"RIP status and config table"},
#endif
#ifdef ROUTING
	{"route",			END,	CMD_SHOW_ROUTE_TBL,			show_routes_nodes,	0,	cmdShowRouteTbl,			listNextLevelNodes,	"Ip route table"},
#endif
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	{"snmp",			END,	CMD_SHOW_SNMP_SETTINGS,		0,					0,	cmdShowSnmpSettings,		0,					"SNMP settings"},
#endif
	{"status",			END,	CMD_SHOW_SYSTEM_STATUS,		0,					0,	cmdShowSystemStatus,	0,					"System status"},
#ifdef CONFIG_USER_CWMP_TR069
	{"tr069",			END,	CMD_SHOW_TR069,				0,					0,	cmdShowTr069,			0,					"Tr069 settings"},
#endif
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	{"upnp",			END,	CMD_SHOW_UPNP,				0,					0,	cmdShowUpnp,			0,					"upnp settings"},
#endif
#ifdef URL_BLOCKING_SUPPORT
	{"urlblock",		END,	CMD_SHOW_URL_BLOCKING,		0,					0,	cmdShowUrlBlocking,		0,					"URL blocking"},
#endif
	{"wan",			0,	0,							show_wan_nodes,		0,	0,						listNextLevelNodes,	"WAN settings"},
	{0,0,0,0,0,0,0,0}
};

/*END flag means that there should be one cmd_func can be executed*/
/*and cmd node with cmd_func or cmd_args must has END flag*/
static struct cmd_node cmd_root[] = {
	{"config",		0, 				0,					cmd_config_nodes,	0,	0,						listNextLevelNodes,	"Configure system"},
	{"debug",		SUSERONLY, 		0,					cmd_debug_nodes,	0,	0,						listNextLevelNodes,	"Debug system"},
	{"exit",		END,				CMD_EXIT_CLI,			0,					0,	cmdExitCli,				0,					"Exit command line interface"},
	{"help",		END,				CMD_PRINT_HELP_INFO,	0,					0,	cmdPrintHelpInformation,	0,					"Help information"},
	{"reboot",		END,				CMD_REBOOT_SYSTEM,	0,					0,	cmdRebootSystem,		0,					"Reboot system"},
	{"sh",		SUSERONLY | END,	CMD_LINUX_SHELL,		0,					0,	cmdEnterLinuxShell,		0,					"Enter linux shell"},
	{"show",		0,				0,					cmd_show_nodes,		0,	0,						listNextLevelNodes,	"Show system information"},
	{0,0,0,0,0,0,0,0}
};




/**********************************command execute functions********************************/

#ifdef CONFIG_USER_CWMP_TR069
static void cmdSetTr069(int cmdType, struct cmd_arg *pCmdArg)
{
	char acsUrl[260], usrname[260],passwd[260],reqPath[260];
	char buf[32];
	int informEn,port;
	unsigned int informItvl;
	CWMP_ACS trAcs;
	CWMP_CONN_REQ trConnReq;
	
	switch(cmdType){
		case CMD_DISABLE_TR069:
			if(setTr069Capability(0) == -1){
				printf("Disable tr069 error!\n");
			}
			break;
		case CMD_ENABLE_TR069:
			if(setTr069Capability(1) == -1){
				printf("Enable tr069 error!\n");
			}
			break;
		case CMD_CFG_TR069_ACS:
			memset(&trAcs,0,sizeof(trAcs));
			if(getArgValue(pCmdArg,"url",(void *)acsUrl) == 1){
				trAcs.pUrl = acsUrl;
			}else{
				trAcs.pUrl = NULL;
			}
			if(getArgValue(pCmdArg,"username",(void *)usrname) == 1){
				trAcs.pUsrname= usrname;
			}else{
				trAcs.pUsrname = NULL;
			}
			if(getArgValue(pCmdArg,"password",(void *)passwd) == 1){
				trAcs.pPwd= passwd;
			}else{
				trAcs.pPwd = NULL;
			}
			if(getArgValue(pCmdArg,"inform",(void *)buf) == 1){
				if(!strncmp("disable",buf,strlen(buf)) || !strncmp("DISABLE",buf,strlen(buf))){
					informEn = 0;
				}else{
					informEn = 1;
				}
				trAcs.pIfmEn = &informEn;
			}else{
				trAcs.pIfmEn= NULL;
			}

			if(getArgValue(pCmdArg,"interval",(void *)&informItvl) == 1){
				trAcs.pIfmItvl = &informItvl;
			}else{
				trAcs.pIfmItvl= NULL;
			}
			if(setTr069Acs(&trAcs) == -1){
				printf("Set tr069 acs error!\n");
			}
			break;
		case CMD_CFG_TR069_CONNECT_REQUEST:
			memset(&trConnReq,0,sizeof(trConnReq));
			if(getArgValue(pCmdArg,"username",(void *)usrname) == 1){
				trConnReq.pUsrname= usrname;
			}else{
				trConnReq.pUsrname = NULL;
			}
			if(getArgValue(pCmdArg,"password",(void *)passwd) == 1){
				trConnReq.pPwd= passwd;
			}else{
				trConnReq.pPwd = NULL;
			}
			if(getArgValue(pCmdArg,"path",(void *)reqPath) == 1){
				trConnReq.pPath= reqPath;
			}else{
				trConnReq.pPath= NULL;
			}
			if(getArgValue(pCmdArg,"port",(void *)&port) == 1){
				trConnReq.pPort= &port;
			}else{
				trConnReq.pPort= NULL;
			}
			if(setTr069ConnectReq(&trConnReq) == -1){
				printf("Set tr069 connect request error!\n");
			}
			break;
	}
}

static void cmdShowTr069(int cmdType, struct cmd_arg *pCmdArg)
{
	showTr069Settings();
}

#endif


#ifdef IP_ACL
static void cmdSetAcl(int cmdType, struct cmd_arg *pCmdArg)
{
	struct in_addr ip,mask;
	int enabled,index,intf;
	char buf[32];
	
	switch(cmdType){
		case CMD_DISABLE_ACL:
			setAclCapability(0);
			break;
		case CMD_ENABLE_ACL:
			setAclCapability(1);
			break;
		case CMD_ADD_ACL:
			if(getArgValue(pCmdArg,"ip",(void *)&ip) != 1){
				printf("please input ip address\n");
				goto arg_err;
			}
			if(getArgValue(pCmdArg,"mask",(void *)&mask) != 1){
				printf("please input netmask\n");
				goto arg_err;
			}
			if(getArgValue(pCmdArg,"interface",(void *)buf) != 1){
				printf("please input interface name\n");
				goto arg_err;
			}
			if(!strncmp("LAN",buf,strlen(buf)) || !strncmp("lan",buf,strlen(buf))){
				intf = 0;
			}else{
				intf = 1;
			}
			if(getArgValue(pCmdArg,"disabled",(void *)buf) == 1){
				enabled = 0;
			}else{
				enabled = 1;
			}
			
			addAclEntry(&ip, &mask, intf, enabled);
			break;
		case CMD_DELETE_ACL:
			if(getArgValue(pCmdArg,"index",(void *)&index) != 1){
				printf("please input acl entry's index number\n");
				goto arg_err;
			}

			delAclEntry(index);
			break;
		default:
			return;
	}
	
	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowAclCfg(int cmdType, struct cmd_arg *pCmdArg)
{
	showAclCfg();
}
#endif

static void cmdSetPassword(int cmdType, struct cmd_arg *pCmdArg)
{
	char user[64];
	char oldpasswd[64];
	char newpasswd[64];
	
	if(getArgValue(pCmdArg,"user",(void *)user) != 1){
		printf("please input username\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"oldpasswd",(void *)oldpasswd) != 1){
		printf("please input old password\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"newpasswd",(void *)newpasswd) != 1){
		printf("please input new password\n");
		goto arg_err;
	}

	if(setAccountPassword(user,oldpasswd,newpasswd) == -1){
		printf("Set password error!\n");
	}

	return;
	
arg_err:
	printf("%s",Argerr_String);

}

#ifdef REMOTE_ACCESS_CTL
static void cmdSetRemoteAccess(int cmdType, struct cmd_arg *pCmdArg)
{
	char buf[64];
	int port,lanAct,wanAct;
	int rmtType;
	int havePort = 0;

	switch(cmdType){
		case CMD_SET_FTP_ACCESS:
			rmtType = RMT_FTP;
			havePort = 1;
			break;
		case CMD_SET_WEB_ACCESS:
			rmtType = RMT_WEB;
			havePort = 1;
			break;
#ifdef CONFIG_USER_TELNETD_TELNETD
		case CMD_SET_TELNET_ACCESS:
			rmtType = RMT_TELNET;
			havePort = 1;
			break;
#endif
		case CMD_SET_PING_ACCESS:
			rmtType = RMT_PING;
			break;
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP			
		case CMD_SET_SNMP_ACCESS:
			rmtType = RMT_SNMP;
			break;
#endif
#ifdef CONFIG_USER_TFTPD_TFTPD
		case CMD_SET_TFTP_ACCESS:
			rmtType = RMT_TFTP;
			break;
#endif
	}

	if(getArgValue(pCmdArg,"lan",(void *)buf) == 1){
		if(!strncmp("deny",buf,strlen(buf))){
			lanAct = 0;
		}else if(!strncmp("allow",buf,strlen(buf))){
			lanAct = 1;
		}else{
			printf("Unknown lan action! please input \"deny\" or \"allow\"\n");
			goto arg_err;
		}
	}else{
		printf("please input lan action!\n");
		goto arg_err;
	}
			
	if(getArgValue(pCmdArg,"wan",(void *)buf) == 1){
		if(!strncmp("deny",buf,strlen(buf))){
			wanAct = 0;
		}else if(!strncmp("allow",buf,strlen(buf))){
			wanAct = 1;
		}else{
			printf("Unknown wan action! please input \"deny\" or \"allow\"\n");
			goto arg_err;
		}
	}else{
		printf("please input wan action!\n");
		goto arg_err;
	}

	if( havePort ){
		if(getArgValue(pCmdArg,"wanport",(void *)&port) != 1){
			port = 0;
		}
	}

	if(setRmtAccess(rmtType,lanAct,wanAct,port) == -1){
		printf("Set remote access error!\n");
		return;
	}

	return;
arg_err:
	printf("%s",Argerr_String);

}

static void cmdShowRemoteAccess(int cmdType, struct cmd_arg *pCmdArg)
{
	showRmtAccess();
}
#endif

#if defined(IP_QOS)
static int getQosArguments(struct cmd_arg *pCmdArg,MIB_CE_IP_QOS_Tp pEntry)
{
	char buf[32];
	int intVal;
	struct in_addr ipaddr,netmask;
	struct in_addr * pAddr;
	unsigned long mbit;
	int i,type, ifnum = 0, itfid, itfdomain;	
	struct itfInfo itfs[16];
	
#ifdef _CWMP_MIB_
	pEntry->enable = 1;
#endif

	if(getArgValue(pCmdArg,"srcip",(void *)&ipaddr) == 1){
		if(getArgValue(pCmdArg,"srcmask",(void *)&netmask) != 1){
			printf("please input srcmask!\n");
			return -1;
		}
		pAddr = (struct in_addr *)pEntry->sip;
		pAddr->s_addr = ipaddr.s_addr;
		mbit=0;
		while (1) {
			if (netmask.s_addr&0x80000000) {
				mbit++;
				netmask.s_addr <<= 1;
			}else{
				break;
			}
		}
		pEntry->smaskbit = mbit;
	}

	if(getArgValue(pCmdArg,"dstip",(void *)&ipaddr) == 1){
		if(getArgValue(pCmdArg,"dstmask",(void *)&netmask) != 1){
			printf("please input dstmask!\n");
			return -1;
		}
		pAddr = (struct in_addr *)pEntry->dip;
		pAddr->s_addr = ipaddr.s_addr;
		mbit=0;
		while (1) {
			if (netmask.s_addr&0x80000000) {
				mbit++;
				netmask.s_addr <<= 1;
			}else{
				break;
			}
		}
		pEntry->dmaskbit = mbit;
	}

	if(getArgValue(pCmdArg,"protocol",(void *)buf) == 1){
		if(!strcasecmp("tcp",buf)){
			pEntry->protoType = PROTO_TCP;
		}else if(!strcasecmp("udp",buf)){
			pEntry->protoType = PROTO_UDP;
		}else if(!strcasecmp("icmp",buf)){
			pEntry->protoType = PROTO_ICMP;
		}else{
			printf("Unknown protocol! please input \"icmp\" ,\"tcp\" or \"udp\"\n");
			return -1;
		}
	}

	if(pEntry->protoType == PROTO_TCP || pEntry->protoType == PROTO_UDP){
		if(getArgValue(pCmdArg,"srcport",(void *)&intVal) == 1){
			pEntry->sPort = intVal;
		}
		if(getArgValue(pCmdArg,"dstport",(void *)&intVal) == 1){
			pEntry->dPort = intVal;
		}
	}

	if(getArgValue(pCmdArg,"phyport",(void *)buf) == 1){
#ifdef CONFIG_USB_ETH
		type = (DOMAIN_ELAN|DOMAIN_WLAN|DOMAIN_ULAN);
#else
		type = (DOMAIN_ELAN|DOMAIN_WLAN);
#endif 
		ifnum = get_domain_ifinfo(itfs, 16, type);
		if (ifnum == 0)
			return -1;
		for(i=0;i<ifnum;i++){
			if(!strcasecmp(buf,itfs[i].name)){
				itfid = IF_ID(itfs[i].ifdomain, itfs[i].ifid);
				itfdomain = IF_DOMAIN(itfid);
				itfid = itfid & 0x0ff;
				if (itfdomain == DOMAIN_ELAN)
					pEntry->phyPort = itfid;
#ifdef WLAN_SUPPORT
				else if (itfdomain == DOMAIN_WLAN){
#ifdef WLAN_MBSSID
					pEntry->phyPort = 5 + itfid;
#else
					pEntry->phyPort = 5;	// wlan0
#endif		
				}
#endif
#ifdef CONFIG_USB_ETH
				else if (itfdomain == DOMAIN_ULAN)
					pEntry->phyPort = IFUSBETH_PHYNUM;	// usb0
#endif 
				break;
			}
		}
		if(i >= ifnum){
			printf("not find this physical port!\n");
			return -1;
		}
	}else{
		pEntry->phyPort = 0xff;
	}

	if(getArgValue(pCmdArg,"priority",(void *)&intVal) == 1){
		if(intVal <0 || intVal >=IPQOS_NUM_PRIOQ){
			printf("error priority value!\n");
			return -1;
		}
		pEntry->prior = intVal;
	}else{
		printf("Please input priority value!\n");
		return -1;
	}

	if(getArgValue(pCmdArg,"802.1p",(void *)&intVal) == 1){
		if(intVal < 0 || intVal > 7){
			printf("error 802.1p tag!\n");
			return -1;
		}
		pEntry->m_1p = intVal+1;
	}

	if(getArgValue(pCmdArg,"precedence",(void *)&intVal) == 1){
		if(intVal < 0 || intVal > 7){
			printf("error precedence!\n");
			return -1;
		}
		pEntry->m_ipprio = intVal+1;
	}

	if(getArgValue(pCmdArg,"tos",(void *)&intVal) == 1){
		if(intVal < 0 || intVal > 4){
			printf("error tos!\n");
			return -1;
		}
		pEntry->m_iptos = (unsigned char)(1 << intVal);
	}else{
		pEntry->m_iptos = 0xff;
	}

	return 0;
}

static void cmdSetIpQos(int cmdType, struct cmd_arg *pCmdArg)
{
	char vChar;
	char buf[32];
	int index;
	MIB_CE_IP_QOS_T entry;
	
	switch(cmdType){
		case CMD_SET_DEFAULT_QOS:
			if(getArgValue(pCmdArg,"ippred",(void *)buf) == 1){
				vChar = 0;
			}else if(getArgValue(pCmdArg,"802.1p",(void *)buf) == 1){
				vChar = 1;
			}else{
				printf("Please input default qos: 802.1p or ippred\n");
				goto arg_err;
			}
			if(setDefaultQoS(vChar) == -1){
				printf("Set default QoS error!\n");
			}
			break;
		case CMD_DISABLE_IP_QOS:
			if(setQosCapability(0) == -1){
				printf("Disable QoS error!\n");
			}
			break;
		case CMD_ENABLE_IP_QOS:
			if(setQosCapability(1) == -1){
				printf("Disable QoS error!\n");
			}
			break;
		case CMD_ADD_IPQOS_RULE:
			memset(&entry, 0, sizeof(entry));
			if(getQosArguments(pCmdArg,&entry) == -1){
				goto arg_err;
			}
			if(ipQosAdd(&entry) == -1){
				printf("Add ip QoS rule error!\n");
			}
			break;
		case CMD_DELETE_IPQOS_RULE:
			if(getArgValue(pCmdArg,"index",(void *)&index) != 1){
				printf("Get selected rule's index number error!\n");
				goto arg_err;
			}
			if(ipQosDel(index) == -1){
				printf("Delete ip QoS rule error!\n");
			}
			break;
	}

	return;
	
arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowIpQos(int cmdType, struct cmd_arg *pCmdArg)
{
	showIpQoS();
}
#endif


#if defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
static void cmdShowPortMapping(int cmdType, struct cmd_arg *pCmdArg)
{
	showPortmapping();
}

static void cmdAddGroupInterface(int cmdType, struct cmd_arg *pCmdArg)
{
	char ifname[16];
	enum PortMappingGrp grp;
	
	switch(cmdType){
		case CMD_ADD_GROUP1_INTERFACE:
			grp = PM_GROUP1;
			break;
		case CMD_ADD_GROUP2_INTERFACE:
			grp = PM_GROUP2;
			break;
		case CMD_ADD_GROUP3_INTERFACE:
			grp = PM_GROUP3;
			break;
		case CMD_ADD_GROUP4_INTERFACE:
			grp = PM_GROUP4;
			break;
	}

	if(getArgValue(pCmdArg,"interface",(void *)ifname) == 1){
		if(addGroupInterface(grp,ifname) == -1){
			printf("Add interface to group error!\n");
		}
	}else{
		printf("Please input interface's name!\n");
		goto arg_err;
	}

	return;
	
arg_err:
	printf("%s",Argerr_String);

}

static void cmdDeleteGroupInterface(int cmdType, struct cmd_arg *pCmdArg)
{
	char ifname[16];
	enum PortMappingGrp grp;

	switch(cmdType){
		case CMD_DELETE_GROUP1_INTERFACE:
			grp = PM_GROUP1;
			break;
		case CMD_DELETE_GROUP2_INTERFACE:
			grp = PM_GROUP2;
			break;
		case CMD_DELETE_GROUP3_INTERFACE:
			grp = PM_GROUP3;
			break;
		case CMD_DELETE_GROUP4_INTERFACE:
			grp = PM_GROUP4;
			break;
	}
	
	if(getArgValue(pCmdArg,"interface",(void *)ifname) == 1){
		if(delGroupInterface(grp,ifname) == -1){
			printf("Delete interface from group error!\n");
		}
	}else{
		printf("Please input interface's name!\n");
		goto arg_err;
	}

	return;
	
arg_err:
	printf("%s",Argerr_String);
}

static void cmdSetPortMapping(int cmdType, struct cmd_arg *pCmdArg)
{
	switch(cmdType){
		case CMD_DISABLE_PORT_MAPPING:
			if(setPortMappingCapability(0) == -1){
				printf("Disable port mapping error!\n");
			}
			break;
		case CMD_ENABLE_PORT_MAPPING:
			if(setPortMappingCapability(1) == -1){
				printf("Enable port mapping error!\n");
			}
			break;
	}
}
#endif

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
static void cmdSetSnmp(int cmdType, struct cmd_arg *pCmdArg)
{
	char snmpSysDescr[65], snmpSysContact[65], snmpSysName[65];
	char snmpSysLocation[65], snmpSysObjectID[65], snmpCommunityRO[65], snmpCommunityRW[65];
	struct in_addr trapIp;
	SNMP_PARA snmpPara;
	
	switch(cmdType){
		case CMD_DISABLE_SNMP:
			if(setSnmpCapability(0) == -1){
				printf("Disable SNMP error!\n");	
			}
			break;
		case CMD_ENABLE_SNMP:
			if(setSnmpCapability(1) == -1){
				printf("Enable SNMP error!\n");	
			}
			break;
		case CMD_CONFIG_SNMP:
			memset(&snmpPara,0,sizeof(snmpPara));
			if(getArgValue(pCmdArg,"description",(void *)snmpSysDescr) == 1){
				snmpPara.pSysDescription = snmpSysDescr;
			}else{
				snmpPara.pSysDescription = NULL;
			}
			if(getArgValue(pCmdArg,"contact",(void *)snmpSysContact) == 1){
				snmpPara.pSysContact = snmpSysContact;
			}else{
				snmpPara.pSysContact = NULL;
			}
			if(getArgValue(pCmdArg,"systemname",(void *)snmpSysName) == 1){
				snmpPara.pSysName = snmpSysName;
			}else{
				snmpPara.pSysName = NULL;
			}
			if(getArgValue(pCmdArg,"location",(void *)snmpSysLocation) == 1){
				snmpPara.pSysLocation = snmpSysLocation;
			}else{
				snmpPara.pSysLocation = NULL;
			}
			if(getArgValue(pCmdArg,"objectid",(void *)snmpSysObjectID) == 1){
				snmpPara.pSysObjectID = snmpSysObjectID;
			}else{
				snmpPara.pSysObjectID = NULL;
			}
			if(getArgValue(pCmdArg,"trapip",(void *)&trapIp) == 1){
				snmpPara.pTrapIp = &trapIp;
			}else{
				snmpPara.pTrapIp = NULL;
			}
			if(getArgValue(pCmdArg,"readcommunity",(void *)snmpCommunityRO) == 1){
				snmpPara.pROCommunity = snmpCommunityRO;
			}else{
				snmpPara.pROCommunity = NULL;
			}
			if(getArgValue(pCmdArg,"writecommunity",(void *)snmpCommunityRW) == 1){
				snmpPara.pRWCommunity = snmpCommunityRW;
			}else{
				snmpPara.pRWCommunity = NULL;
			}
			if(cfgSnmpSetting(&snmpPara) == -1){
				printf("Set SNMP parameter error!\n");	
			}
			break;
	}
}

static void cmdShowSnmpSettings(int cmdType, struct cmd_arg *pCmdArg)
{
	showSnmpSettings();
}
#endif

static void cmdSetBridging(int cmdType, struct cmd_arg *pCmdArg)
{
	unsigned char enabled;
	unsigned short time;
	unsigned int aging;
	unsigned char buf[32];
	
	if(getArgValue(pCmdArg,"aging",(void *)&aging) == 1){
		time = aging;
		setBridgeAgingTime(time);
	}

	if(getArgValue(pCmdArg,"stp",(void *)buf) == 1){
		if(!strcasecmp("on",buf))
			enabled = 1;
		else 
			enabled = 0;
		setBridgeStp(enabled);
	}
}

static void cmdShowBridging(int cmdType, struct cmd_arg *pCmdArg)
{
	showBridgeCfg();
}

#ifdef DMZ
static void cmdSetDmz(int cmdType, struct cmd_arg *pCmdArg)
{
	struct in_addr ip;

	switch(cmdType){
		case CMD_DISABLE_DMZ:
			if(setDmzCapability(0,0) == -1){
				printf("Disable DMZ error!\n");
			}
			break;
		case CMD_ENABLE_DMZ:
			if(getArgValue(pCmdArg,"ip",(void *)&ip) != 1){
				printf("please input dmz host ip!\n");
				goto arg_err;
			}
			if(setDmzCapability(1,&ip) == -1){
				printf("Enable DMZ error!\n");
			}
			break;
	}

	return;
	
arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowDmz(int cmdType, struct cmd_arg *pCmdArg)
{
	showDmzSetting();
}
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
static void cmdSetDomainBlocking(int cmdType, struct cmd_arg *pCmdArg)
{
	char buf[256];
	
	switch(cmdType){
		case CMD_ADD_DOMAIN_BLOCKING:
			if(getArgValue(pCmdArg,"domain",(void *)buf) != 1){
				printf("please input domain name!\n");
				goto arg_err;
			}
			if(addDomainBlockingEntry(buf) == -1){
				printf("Add domain blocking entry error!\n");
			}
			break;
		case CMD_DELETE_DOMAIN_BLOCKING:
			if(getArgValue(pCmdArg,"domain",(void *)buf) != 1){
				printf("please input domain name!\n");
				goto arg_err;
			}
			if(deleteDomainBlockingEntry(buf) == -1){
				printf("Delete domain blocking entry error!\n");
			}
			break;
		case CMD_DISABLE_DOMAIN_BLOCKING:
			if(setDomainBlockingCapablility(0) == -1){
				printf("Disable domain blocking error!\n");
			}
			break;
		case CMD_ENABLE_DOMAIN_BLOCKING:
			if(setDomainBlockingCapablility(1) == -1){
				printf("Enable domain blocking error!\n");
			}
			break;
	}

	return ;
	
arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowDomainBlocking(int cmdType, struct cmd_arg *pCmdArg)
{
	showDomainBlocking();
}
#endif

#ifdef URL_BLOCKING_SUPPORT
static void cmdSetUrlBlocking(int cmdType, struct cmd_arg *pCmdArg)
{
	char enabled;
	char buf[256];
	int action;
	
	switch(cmdType){
		case CMD_ADD_URL_BLOCKING:
		case CMD_DELETE_URL_BLOCKING:
			action = cmdType == CMD_ADD_URL_BLOCKING ? URL_BLOCK_ADD : URL_BLOCK_DEL;
			if(getArgValue(pCmdArg,"fqdn",(void *)buf) == 1){
				if(setUrlBlockingEntry(action,MIB_URL_FQDN_TBL,buf) == -1){
					printf("set url blocking fqdn entry error!\n");
				}
			}
			if(getArgValue(pCmdArg,"keyword",(void *)buf) == 1){
				if(setUrlBlockingEntry(action,MIB_KEYWD_FILTER_TBL,buf) == -1){
					printf("set url blocking keyword entry error!\n");
				}
			}
			break;
		case CMD_DISABLE_URL_BLOCKING:
		case CMD_ENABLE_URL_BLOCKING:
			enabled = (cmdType == CMD_DISABLE_URL_BLOCKING) ? 0 : 1;
			if(setUrlBlockingCapability(enabled) == -1){
				printf("set url blocking capability error!\n");
			}
			break;
	}
}

static void cmdShowUrlBlocking(int cmdType, struct cmd_arg *pCmdArg)
{
	showUrlBlocking();
}
#endif

#ifdef PORT_FORWARD_GENERAL
static void cmdAddPortForwarding(int cmdType, struct cmd_arg *pCmdArg)
{
	char buf[128];
	struct in_addr locip,pubip;
	struct arg_range locport,pubport;
	MIB_CE_PORT_FW_T entry;
	struct in_addr * pAddr;
	
	/*initial variable*/
	memset(&entry, 0x00, sizeof(entry));
	
	/*get argument values*/
	if(getArgValue(pCmdArg,"protocol",(void *)buf) == 1){
		if(!strcasecmp("both",buf)){
			entry.protoType = PROTO_UDPTCP;
		}else if(!strcasecmp("tcp",buf)){
			entry.protoType = PROTO_TCP;
		}else if(!strcasecmp("udp",buf)){
			entry.protoType = PROTO_UDP;
		}else{
			printf("Unknown protocol! please input \"both\" ,\"tcp\" or \"udp\"\n");
			goto arg_err;
		}
	}else{
		printf("please input protocol!\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"localip",(void *)&locip) == 1){
		pAddr = (struct in_addr *)entry.ipAddr;
		pAddr->s_addr = locip.s_addr;
	}else{
		printf("please input local ip!\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"localport",(void *)&locport) == 1){
		entry.fromPort =locport.start;
		entry.toPort = locport.end;
	}

	if(getArgValue(pCmdArg,"publicip",(void *)&pubip) == 1){
		pAddr = (struct in_addr *)entry.remotehost;
		pAddr->s_addr = pubip.s_addr;
	}

	if(getArgValue(pCmdArg,"publicport",(void *)&pubport) == 1){
		entry.externalfromport =pubport.start;
		entry.externaltoport = pubport.end;
	}

	if(getArgValue(pCmdArg,"interface",(void *)buf) == 1){
		if(!strcasecmp("any",buf)){
			entry.ifIndex = 0xff;
		}else if((entry.ifIndex = getIfIndexByName(buf)) == -1){
			printf("Unknown WAN interface's name!\n");
			goto arg_err;
		}
	}else{
		entry.ifIndex = 0xff;
	}

	if(getArgValue(pCmdArg,"comment",(void *)buf) == 1){
		if(strlen(buf) >= COMMENT_LEN){
			printf("Comment string is too long! it should be less than %d !\n",COMMENT_LEN);
			goto arg_err;
		}
		strcpy(entry.comment,buf);
	}

	if(getArgValue(pCmdArg,"disabled",(void *)buf) == 1){
		entry.enable = 0;
	}else{
		entry.enable = 1;
	}

	if(addPortForwarding(&entry) == -1){
		printf("Add port forwarding error!\n");
	}
	
	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdDeletePortForwarding(int cmdType, struct cmd_arg *pCmdArg)
{
	int index;
	
	if(getArgValue(pCmdArg,"index",(void *)&index) != 1){
		printf("Get selected rule's index number error!\n");
		goto arg_err;
	}

	if(deletePortForwarding(index) == -1){
		printf("Delete port forwarding by index error!\n");
	}
	
	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdSetPortForwarding(int cmdType, struct cmd_arg *pCmdArg)
{
	switch(cmdType){
		case CMD_DISABLE_PORT_FORWARDING:
			if(setPortForwardingCapability(0) == -1){
				printf("Disable port forwarding error!\n");
			}
			break;
		case CMD_ENABLE_PORT_FORWARDING:
			if(setPortForwardingCapability(1) == -1){
				printf("Enable port forwarding error!\n");
			}
			break;
	}
}

static void cmdShowPortForwarding(int cmdType, struct cmd_arg *pCmdArg)
{
	showPortForwarding();
}
#endif

#ifdef MAC_FILTER
static void cmdSetMacFiltering(int cmdType, struct cmd_arg *pCmdArg)
{
	char action[32];
	char buf[32];
	char inact,outact,macSet=0;
	int i,index;
	MIB_CE_MAC_FILTER_T macEntry;
	
	switch(cmdType){
		case CMD_ADD_MAC_FILTERING:
			memset(&macEntry, 0x00, sizeof(macEntry));
			if(getArgValue(pCmdArg,"direction",(void *)buf) == 1){
				if(!strncmp("outgoing",buf,strlen(buf))){
					macEntry.dir = DIR_OUT;
				}else if(!strncmp("incoming",buf,strlen(buf))){
					macEntry.dir = DIR_IN;
				}else{
					printf("Unknown direction! please input \"incoming\" or \"outgoing\"\n");
					goto arg_err;
				}
			}else{
				printf("please input direction!\n");
				goto arg_err;
			}
			
			if(getArgValue(pCmdArg,"action",(void *)buf) == 1){
				if(!strncmp("deny",buf,strlen(buf))){
					macEntry.action = 0;
				}else if(!strncmp("allow",buf,strlen(buf))){
					macEntry.action = 1;
				}else{
					printf("Unknown action! please input \"deny\" or \"allow\"\n");
					goto arg_err;
				}
			}else{
				printf("please input action!\n");
				goto arg_err;
			}

			if(getArgValue(pCmdArg,"srcmac",(void *)macEntry.srcMac) == 1){
				macSet = 1;
			}
			if(getArgValue(pCmdArg,"dstmac",(void *)macEntry.dstMac) == 1){
				macSet = 1;
			}
			if(macSet == 0){
				printf("Please input srcmac or dstmac!\n");
				goto arg_err;
			}
			if(addMacFiltering(&macEntry) == -1){
				printf("Add MAC filtering error!\n");
			}
			break;
		case CMD_DELETE_MAC_FILTERING:
			if(getArgValue(pCmdArg,"index",(void *)&index) != 1){
				printf("Get selected rule's index number error!\n");
				goto arg_err;
			}
			if(deleteMacFiltering(index) == -1){
				printf("Delete MAC filtering rule error!\n");
			}
			break;
		case CMD_MAC_FILTER_DEFAULT_ACTION:
			if(getArgValue(pCmdArg,"incoming",(void *)action) == 1){
				if(!strncmp("allow",action,strlen(action))){
					inact = 1;
				}else if(!strncmp("deny",action,strlen(action))){
					inact = 0;
				}else{
					printf("Unknown incoming action! please input \"allow\" or \"deny\"\n");
					goto arg_err;
				}
			}else{
				printf("please input incoming action!\n");
				goto arg_err;
			}
			if(getArgValue(pCmdArg,"outgoing",(void *)action) == 1){
				if(!strncmp("allow",action,strlen(action))){
					outact = 1;
				}else if(!strncmp("deny",action,strlen(action))){
					outact = 0;
				}else{
					printf("Unknown outgoing action! please input \"allow\" or \"deny\"\n");
					goto arg_err;
				}
			}else{
				printf("please input outgoing action!\n");
				goto arg_err;
			}

			if(setDefaultMacFilteringAction(inact,outact) == -1){
				printf("Set default MAC filtering action error!\n");
			}
			break;
	}
	
	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowMacFiltering(int cmdType, struct cmd_arg *pCmdArg)
{
	showMACFiltering();
}
#endif

#ifdef IP_PORT_FILTER
static void cmdAddIpPortFiltering(int cmdType, struct cmd_arg *pCmdArg)
{
	char buf[32];
	struct in_addr srcip,srcmask,dstip,dstmask;
	struct arg_range srcport,dstport;
	MIB_CE_IP_PORT_FILTER_T filterEntry;
	struct in_addr * pAddr;
	unsigned long mbit;
	
	/*initial variable*/
	memset(&filterEntry, 0x00, sizeof(filterEntry));
	
	/*get argument values*/
	if(getArgValue(pCmdArg,"direction",(void *)buf) == 1){
		if(!strncmp("outgoing",buf,strlen(buf))){
			filterEntry.dir = DIR_OUT;
		}else if(!strncmp("incoming",buf,strlen(buf))){
			filterEntry.dir = DIR_IN;
		}else{
			printf("Unknown direction! please input \"incoming\" or \"outgoing\"\n");
			goto arg_err;
		}
	}else{
		printf("please input direction!\n");
		goto arg_err;
	}
	
	if(getArgValue(pCmdArg,"protocol",(void *)buf) == 1){
		if(!strcasecmp("icmp",buf)){
			filterEntry.protoType = PROTO_ICMP;
		}else if(!strcasecmp("tcp",buf)){
			filterEntry.protoType = PROTO_TCP;
		}else if(!strcasecmp("udp",buf)){
			filterEntry.protoType = PROTO_UDP;
		}else{
			printf("Unknown protocol! please input \"icmp\" ,\"tcp\" or \"udp\"\n");
			goto arg_err;
		}
	}else{
		printf("please input protocol!\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"action",(void *)buf) == 1){
		if(!strncmp("deny",buf,strlen(buf))){
			filterEntry.action = 0;
		}else if(!strncmp("allow",buf,strlen(buf))){
			filterEntry.action = 1;
		}else{
			printf("Unknown action! please input \"deny\" or \"allow\"\n");
			goto arg_err;
		}
	}else{
		printf("please input action!\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"srcip",(void *)&srcip) == 1){
		if(getArgValue(pCmdArg,"srcmask",(void *)&srcmask) != 1){
			printf("please input srcmask!\n");
			goto arg_err;
		}
		pAddr = (struct in_addr *)filterEntry.srcIp;
		pAddr->s_addr = srcip.s_addr;
		mbit=0;
		while (1) {
			if (srcmask.s_addr&0x80000000) {
				mbit++;
				srcmask.s_addr <<= 1;
			}else{
				break;
			}
		}
		filterEntry.smaskbit = mbit;
	}

	if(getArgValue(pCmdArg,"dstip",(void *)&dstip) == 1){
		if(getArgValue(pCmdArg,"dstmask",(void *)&dstmask) != 1){
			printf("please input dstmask!\n");
			goto arg_err;
		}
		pAddr = (struct in_addr *)filterEntry.dstIp;
		pAddr->s_addr = dstip.s_addr;
		mbit=0;
		while (1) {
			if (dstmask.s_addr&0x80000000) {
				mbit++;
				dstmask.s_addr <<= 1;
			}else{
				break;
			}
		}
		filterEntry.dmaskbit = mbit;
	}

	if(filterEntry.protoType == PROTO_TCP || filterEntry.protoType == PROTO_UDP){
		if(getArgValue(pCmdArg,"srcport",(void *)&srcport) == 1){
			filterEntry.srcPortFrom =srcport.start;
			filterEntry.srcPortTo = srcport.end;
		}
		if(getArgValue(pCmdArg,"dstport",(void *)&dstport) == 1){
			filterEntry.dstPortFrom =dstport.start;
			filterEntry.dstPortTo = dstport.end;
		}
	}

	if(ipPortFilterAdd(&filterEntry) == -1){
		printf("Add ip port filtering rule error!\n");
	}

	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdDeleteIpPortFiltering(int cmdType, struct cmd_arg *pCmdArg)
{
	int index;

	if(getArgValue(pCmdArg,"index",(void *)&index) != 1){
		printf("Get selected rule's index number error!\n");
		goto arg_err;
	}
	
	if(ipPortFilterDel(index) == -1){
		printf("Delete ip port filtering rule error!\n");
	}

	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdDefaultIpPortFiltering(int cmdType, struct cmd_arg *pCmdArg)
{
	char action[32];
	char inact,outact;
	
	if(getArgValue(pCmdArg,"incoming",(void *)action) == 1){
		if(!strncmp("allow",action,strlen(action))){
			inact = 1;
		}else if(!strncmp("deny",action,strlen(action))){
			inact = 0;
		}else{
			printf("Unknown incoming action! please input \"allow\" or \"deny\"\n");
			goto arg_err;
		}
	}else{
		printf("please input incoming action!\n");
		goto arg_err;
	}
	
	if(getArgValue(pCmdArg,"outgoing",(void *)action) == 1){
		if(!strncmp("allow",action,strlen(action))){
			outact = 1;
		}else if(!strncmp("deny",action,strlen(action))){
			outact = 0;
		}else{
			printf("Unknown outgoing action! please input \"allow\" or \"deny\"\n");
			goto arg_err;
		}
	}else{
		printf("please input outgoing action!\n");
		goto arg_err;
	}

	if(setDefaultIpPortFilterAction(inact,outact) == -1){
		printf("Set default ip port filtering action error!\n");
	}

	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowIpPortFiltering(int cmdType, struct cmd_arg *pCmdArg)
{
	showIPPortFilteringTable();
}
#endif

#ifdef CONFIG_USER_ROUTED_ROUTED
static void cmdSetRip(int cmdType, struct cmd_arg *pCmdArg)
{
	char ifname[64];
	char sendmode[64];
	char recvmode[64];
	int index;
	MIB_CE_RIP_T ripEntry;
	
	switch(cmdType){
		case CMD_ADD_RIP_INTERFACE:
			memset(&ripEntry,0,sizeof(MIB_CE_RIP_T));
			if(getArgValue(pCmdArg,"interface",(void *)ifname) == 1){
				if(!strcasecmp(BRIF,ifname)){
					ripEntry.ifIndex = 0xff;
				}else{
					ripEntry.ifIndex = getIfIndexByName(ifname);
				}
			}else{
				printf("please input interface name!\n");
				goto arg_err;
			}
			if(getArgValue(pCmdArg,"recvmode",(void *)recvmode) == 1){
				if(!strcasecmp("rip1",recvmode)){
					ripEntry.receiveMode = 1;
				}else if(!strcasecmp("rip2",recvmode)){
					ripEntry.receiveMode = 2;
				}else if(!strcasecmp("both",recvmode)){
					ripEntry.receiveMode = 3;
				}
			}else{
				printf("please input rip receive mode!\n");
				goto arg_err;
			}
			if(getArgValue(pCmdArg,"sendmode",(void *)sendmode) == 1){
				if(!strcasecmp("rip1",sendmode)){
					ripEntry.sendMode = 1;
				}else if(!strcasecmp("rip2",sendmode)){
					ripEntry.sendMode = 2;
				}else if(!strncmp("rip1compat",sendmode,strlen(sendmode)) || !strncmp("RIP1COMPAT",sendmode,strlen(sendmode))){
					ripEntry.sendMode = RIP_V1_COMPAT;
				}
			}else{
				printf("please input rip receive mode!\n");
				goto arg_err;
			}
			if(addRipInterface(&ripEntry) == -1){
				printf("Add rip interface error!\n");
			}
			break;
		case CMD_DELETE_RIP_INTERFACE:
			if(getArgValue(pCmdArg,"index",(void *)&index) != 1){
				printf("Get selected rule's index number error!\n");
				goto arg_err;
			}
			if(delRipInterface(index) == -1){
				printf("Delete rip interface error!\n");
			}
			break;
		case CMD_ENABLE_RIP:
			if(setRipCapability(1) == -1){
				printf("Enable rip error!\n");
			}
			break;
		case CMD_DISABLE_RIP:
			if(setRipCapability(0) == -1){
				printf("Disable rip error!\n");
			}
			break;
	}

	return;

arg_err:
	printf("%s",Argerr_String);
}
static void cmdShowRipStatusTbl(int cmdType, struct cmd_arg *pCmdArg)
{
	showRipStatusTbl();
}

#endif

#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
static void cmdSetUPnP(int cmdType, struct cmd_arg *pCmdArg)
{
	char ifname[64];
	unsigned char ifIndex;

	switch(cmdType){
		case CMD_DISABLE_UPNP:
			if(setUpnpCapability(0,0) == -1){
				printf("Disable UPnP error!\n");
			}
			break;
		case CMD_ENABLE_UPNP:
			if(getArgValue(pCmdArg,"interface",(void *)ifname) != 1){
				printf("Please input interface name!\n");
				goto arg_err;
			}
			if((ifIndex = getIfIndexByName(ifname)) < 0){
				printf("No such interface!\n");
				goto arg_err;
			}
			if(setUpnpCapability(1,ifIndex) == -1){
				printf("Enable UPnP error!\n");
			}
			break;
	}

	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowUpnp(int cmdType, struct cmd_arg *pCmdArg)
{
	showUpnp();
}
#endif

#ifdef CONFIG_USER_IGMPPROXY
static void cmdSetIgmpproxy(int cmdType, struct cmd_arg *pCmdArg)
{
	char ifname[64];

	switch(cmdType){
		case CMD_DISABLE_IGMP_PROXY:
			if(setIgmpProxyCapability(0,NULL) == -1){
				printf("Disable igmp proxy error!\n");
			}
			break;
		case CMD_ENABLE_IGMP_PROXY:
			if(getArgValue(pCmdArg,"interface",(void *)ifname) != 1){
				printf("please input interface name!\n");
				goto arg_err;
			}
			if(setIgmpProxyCapability(1,ifname) == -1){
				printf("Enable igmp proxy error!\n");
			}
			break;
	}

	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowIgmpproxy(int cmdType, struct cmd_arg *pCmdArg)
{
	showIgmpProxy();
}
#endif

static void cmdCfgAdslSetting(int cmdType, struct cmd_arg *pCmdArg)
{
	unsigned short mode=0;
	char olr = 0;
	int intVal;
	
	if(getArgValue(pCmdArg,"g.lite",(void *)&intVal) == 1){	
		mode |= ADSL_MODE_GLITE;
	}
	if(getArgValue(pCmdArg,"g.dmt",(void *)&intVal) == 1){	
		mode |= ADSL_MODE_GDMT;
	}
	if(getArgValue(pCmdArg,"t1.413",(void *)&intVal) == 1){	
		mode |= ADSL_MODE_T1413;
	}
	if(getArgValue(pCmdArg,"adsl2",(void *)&intVal) == 1){	
		mode |= ADSL_MODE_ADSL2;
	}
	if(getArgValue(pCmdArg,"adsl2+",(void *)&intVal) == 1){	
		mode |= ADSL_MODE_ADSL2P;
	}
	if(getArgValue(pCmdArg,"annexl",(void *)&intVal) == 1){	
		mode |= ADSL_MODE_ANXL;
	}
	if(getArgValue(pCmdArg,"annexm",(void *)&intVal) == 1){	
		mode |= ADSL_MODE_ANXM;
	}
#ifdef ENABLE_ADSL_MODE_GINP
	if(getArgValue(pCmdArg,"g.inp",(void *)&intVal) == 1){	
		mode |= ADSL_MODE_GINP;
	}
#endif
	if(getArgValue(pCmdArg,"bitswap",(void *)&intVal) == 1){	
		olr |= 1;
	}
	if(getArgValue(pCmdArg,"sra",(void *)&intVal) == 1){	
		olr |= 2;
	}

	if(setAdslSettings(mode,olr) == -1){
		printf("Set ADSL error!\n");
	}
}

static void cmdShowAdslSetting(int cmdType, struct cmd_arg *pCmdArg)
{
	showADSLSettings();
}

static void cmdShowAdslStatistics(int cmdType, struct cmd_arg *pCmdArg)
{
	showAdslStatistics();
}

static void cmdModifyAtmSetting(int cmdType, struct cmd_arg *pCmdArg)
{
	char buf[256];
	int intVal;
	ATM_PARA atm_para;

	memset(&atm_para,0,sizeof(atm_para));
	if(getArgValue(pCmdArg,"vpi",(void *)&intVal) == 1){	
		if(intVal < 0 || intVal > 255){
			printf("Invalid vpi value! it should be from 0 to 255.\n");
			goto arg_err;
		}
		atm_para.vpi = intVal;
	}else{
		printf("Please input vpi value!\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"vci",(void *)&intVal) == 1){	
		if(intVal < 0 || intVal > 65535){
			printf("Invalid vci value! it should be from 0 to 65535.\n");
			goto arg_err;
		}
		atm_para.vci = intVal;
	}else{
		printf("Please input vci value!\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"qos",(void *)buf) == 1){	
		if(!strncmp("ubr",buf,strlen(buf)) || !strncmp("UBR",buf,strlen(buf))){
			atm_para.qos = 0;
		}else if(!strncmp("cbr",buf,strlen(buf)) || !strncmp("CBR",buf,strlen(buf))){
			atm_para.qos = 1;
		}else if(!strncmp("nrt-vbr",buf,strlen(buf)) || !strncmp("NRT-VBR",buf,strlen(buf))){
			atm_para.qos = 2;
		}else if(!strncmp("rt-vbr",buf,strlen(buf)) || !strncmp("RT-VBR",buf,strlen(buf))){
			atm_para.qos = 3;
		}else{
			printf("Unknown qos type! it should be \"ubr\", \"cbr\", \"rt-vbr\" or \"nrt-vbr\"\n");
			goto arg_err;
		}
	}else{
		printf("Please input qos type!\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"pcr",(void *)&intVal) == 1){	
		if(intVal < 0 || intVal > 65535){
			printf("Invalid pcr value! it should be from 0 to 65535.\n");
			goto arg_err;
		}
		atm_para.pcr = intVal;
	}else{
		printf("Please input pcr value!\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"cdvt",(void *)&intVal) == 1){	
		atm_para.cdvt = (unsigned int)intVal;
	}else{
		printf("Please input cdvt value!\n");
		goto arg_err;
	}

	if(atm_para.qos == 2 || atm_para.qos == 3){
		if(getArgValue(pCmdArg,"scr",(void *)&intVal) == 1){	
			if(intVal < 0 || intVal > 65535){
				printf("Invalid scr value! it should be from 0 to 65535.\n");
				goto arg_err;
			}
			atm_para.scr = intVal;
		}else{
			printf("Please input scr value!\n");
			goto arg_err;
		}
		if(getArgValue(pCmdArg,"mbs",(void *)&intVal) == 1){	
			if(intVal < 0 || intVal > 65535){
				printf("Invalid mbs value! it should be from 0 to 65535.\n");
				goto arg_err;
			}
			atm_para.mbs = intVal;
		}else{
			printf("Please input mbs value!\n");
			goto arg_err;
		}
	}

	if(modifyAtmSettings(&atm_para) == -1){
		printf("Modify ATM settings error!\n");
	}
	
	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowAtmVcTbl(int cmdType, struct cmd_arg *pCmdArg)
{
	showAtmSettings();
}

static void cmdSetAutoPvc(int cmdType, struct cmd_arg *pCmdArg)
{

}

static void cmdShowAutoPvcTbl(int cmdType, struct cmd_arg *pCmdArg)
{

}

static void cmdAddWanInterface(int cmdType, struct cmd_arg *pCmdArg)
{
	char *temp, strIp[20], strGW[20], strMask[20];
	char buf[256];
	int intVal,ipset=0;
	MIB_CE_ATM_VC_T entry; 

	memset(&entry, 0x00, sizeof(entry));
	entry.pcr = 7600;   
	switch(cmdType){
		case CMD_ADD_1483BRG_PVC:
			entry.cmode = CHANNEL_MODE_BRIDGE;
			break;
		case CMD_ADD_1483MER_PVC:
			entry.cmode = CHANNEL_MODE_IPOE;
			break;
		case CMD_ADD_PPPOE_PVC:
			entry.cmode = CHANNEL_MODE_PPPOE;
			break;
		case CMD_ADD_PPPOA_PVC:
			entry.cmode = CHANNEL_MODE_PPPOA;
			break;
		case CMD_ADD_1483RT_PVC:
			entry.cmode = CHANNEL_MODE_RT1483;
			break;
	}
	if(getArgValue(pCmdArg,"vpi",(void *)&intVal) == 1){	
		if(intVal < 0 || intVal > 255){
			printf("Invalid vpi value! it should be from 0 to 255.\n");
			goto arg_err;
		}
		entry.vpi = intVal;
	}else{
		printf("Please input vpi value!\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"vci",(void *)&intVal) == 1){	
		if(intVal < 0 || intVal > 65535){
			printf("Invalid vci value! it should be from 0 to 65535.\n");
			goto arg_err;
		}
		entry.vci = intVal;
	}else{
		printf("Please input vci value!\n");
		goto arg_err;
	}

	if(getArgValue(pCmdArg,"encapsulation",(void *)buf) == 1){	
		if(!strncmp("llc",buf,strlen(buf)) || !strncmp("LLC",buf,strlen(buf))){
			entry.encap = 1;
		}else if(!strncmp("vcmux",buf,strlen(buf)) || !strncmp("VCMUX",buf,strlen(buf))){
			entry.encap = 0;
		}else{
			printf("Unknown encapsulation type! it should be \"llc\" or \"vcmux\".\n");
			goto arg_err;
		}
	}else{
		printf("Please input encapsulation type!\n");
		goto arg_err;
	}

	entry.napt = 0;
	if(entry.cmode != CHANNEL_MODE_BRIDGE){
		if(getArgValue(pCmdArg,"napt",(void *)buf) == 1){	
			if(!strcmp("on",buf) || !strcmp("ON",buf)){
				entry.napt = 1;
			}
		}
	}

#ifdef DEFAULT_GATEWAY_V1
	entry.dgw = 0;
	if(entry.cmode != CHANNEL_MODE_BRIDGE){
		if(getArgValue(pCmdArg,"defaultgtw",(void *)buf) == 1){	
			if(!strcmp("on",buf) || !strcmp("ON",buf)){
				entry.dgw = 1;
			}
		}
	}
#endif

	entry.enable = 1;
	if(getArgValue(pCmdArg,"disabled",(void *)buf) == 1){	
		entry.enable = 0;
	}

	entry.brmode = 1;

	// 1483 bridged
	if (entry.cmode == CHANNEL_MODE_BRIDGE)
	{
		entry.brmode = 0;
		entry.ipAddr[0]=0;
		entry.ipAddr[1]=0;
		entry.ipAddr[2]=0;
		entry.ipAddr[3]=0;
		entry.remoteIpAddr[0]=0;
		entry.remoteIpAddr[1]=0;
		entry.remoteIpAddr[2]=0;
		entry.remoteIpAddr[3]=0;
		entry.netMask[0]=0;
		entry.netMask[1]=0;
		entry.netMask[2]=0;
		entry.netMask[3]=0;
#ifdef DEFAULT_GATEWAY_V1
		entry.dgw=0;
#endif
		entry.cdvt=0;
	} else if (entry.cmode == CHANNEL_MODE_PPPOE || entry.cmode == CHANNEL_MODE_PPPOA){ 	// PPP connection
		if(getArgValue(pCmdArg,"username",(void *)buf) == 1){	
			if(strlen(buf) > MAX_PPP_NAME_LEN)
			{
				printf("Username is too long!\n");
				goto arg_err;
			}else{
				strcpy(entry.pppUsername,buf);
			}
		}else{
			printf("Please input username!\n");
			goto arg_err;
		}

		if(getArgValue(pCmdArg,"password",(void *)buf) == 1){	
			if(strlen(buf) > MAX_NAME_LEN-1)
			{
				printf("password is too long!\n");
				goto arg_err;
			}else{
				strcpy(entry.pppPassword,buf);
			}
		}else{
			printf("Please input password!\n");
			goto arg_err;
		}

		if(getArgValue(pCmdArg,"continuous",(void *)buf) == 1){	
			entry.pppCtype = CONTINUOUS;
		}else if(getArgValue(pCmdArg,"manual",(void *)buf) == 1){
			entry.pppCtype = MANUAL;
		}else if(getArgValue(pCmdArg,"ondemand",(void *)buf) == 1){
			entry.pppCtype = CONNECT_ON_DEMAND;
			if(getArgValue(pCmdArg,"idletime",(void *)&intVal) == 1){
				entry.pppIdleTime = intVal;
			}else{
				printf("Please input idle time!\n");
				goto arg_err;
			}
		}else{
			printf("Please input ppp connect type!\n");
			goto arg_err;
		}
	}else{ // mer or 1483rt
		if(getArgValue(pCmdArg,"staticip",(void *)entry.ipAddr) == 1){
			if(getArgValue(pCmdArg,"mask",(void *)entry.netMask) != 1 ){
				printf("Please input mask!\n");
				goto arg_err;
			}
			if(getArgValue(pCmdArg,"remoteip",(void *)entry.remoteIpAddr) != 1 ){
				printf("Please input remote ip address!\n");
				goto arg_err;
			}
			entry.ipDhcp = DHCP_DISABLED;
			temp = inet_ntoa(*((struct in_addr *)entry.ipAddr));
			strcpy(strIp, temp);
			temp = inet_ntoa(*((struct in_addr *)entry.remoteIpAddr));
			strcpy(strGW, temp);
			temp = inet_ntoa(*((struct in_addr *)entry.netMask));
			strcpy(strMask, temp);
			if (!isValidHostID(strIp, strMask)) {
				printf("Invalid IP/Subnet Mask combination!!\n");
				goto arg_err;
			}
			if (!isSameSubnet(strIp, strGW, strMask)) {
				printf("Invalid IP address! It should be located in the same subnet.\n");
				goto arg_err;
			}
			ipset = 1;
		}else{
			if(entry.cmode == CHANNEL_MODE_RT1483){
				if(getArgValue(pCmdArg,"unnumbered",(void *)buf) == 1){
					entry.ipunnumbered = 1;
					ipset = 1;
				}
			}else{//mer
				if(getArgValue(pCmdArg,"dhcp",(void *)buf) == 1){
					entry.ipDhcp = DHCP_CLIENT;
					ipset = 1;
				}
			}
		}

		if(!ipset){
			printf("Please input ip settings!\n");
			goto arg_err;
		}
	}

	if(addWanInterface(&entry) == -1){
		printf("Add WAN interface error!\n");
	}
	
	return ;
	
arg_err:
	printf("%s",Argerr_String);

}

static void cmdDeleteWanInterface(int cmdType, struct cmd_arg *pCmdArg)
{
	char ifname[256];

	if(getArgValue(pCmdArg,"interface",(void *)ifname) != 1){	
		printf("Please input WAN interface's name!\n");
		goto arg_err;
	}

	if(delWanInterface(ifname) == -1){
		printf("Delete WAN interface error!\n");
	}

	return ;
	
arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowInterfaceWan(int cmdType, struct cmd_arg *pCmdArg)
{
	showWanInterface();
}

static void cmdSetDnsServer(int cmdType, struct cmd_arg *pCmdArg)
{
	struct in_addr dns1,dns2,dns3;
	struct in_addr *pDns1=NULL, *pDns2=NULL, *pDns3=NULL;
	char buf[32];

	if(getArgValue(pCmdArg,"auto",(void *)buf) == 1){	/*relay*/
		if(setDnsServer(DNS_AUTO,NULL,NULL,NULL) == -1){
			printf("Set DNS mode error!\n");
		}
	}else{	/*manually*/
		if(getArgValue(pCmdArg,"dns1",(void *)&dns1) == 1){
			pDns1 = &dns1;
		}

		if(getArgValue(pCmdArg,"dns2",(void *)&dns2) == 1){
			pDns2 = &dns2;
		}
		
		if(getArgValue(pCmdArg,"dns3",(void *)&dns3) == 1){
			pDns3 = &dns3;
		}
		
		if(setDnsServer(DNS_MANUAL,pDns1,pDns2,pDns3) == -1){
			printf("Set DNS mode error!\n");
		}
	}
}

static void cmdShowDnsServer(int cmdType, struct cmd_arg *pCmdArg)
{
	showDnsSetting();
}

#ifdef CONFIG_USER_DHCP_SERVER
static void cmdSetDhcpMode(int cmdType, struct cmd_arg *pCmdArg)
{
	unsigned char mode;
	char buf[256];

	if(getArgValue(pCmdArg,"server",(void *)buf) == 1){
		mode = DHCP_LAN_SERVER;
	}else if(getArgValue(pCmdArg,"relay",(void *)buf) == 1){
		mode = DHCP_LAN_RELAY;
	}else if(getArgValue(pCmdArg,"none",(void *)buf) == 1){
		mode = DHCP_LAN_NONE;
	}else{
		printf("Unkonwn dhcp mode!\n");
		goto arg_err;
	}

	if(setDHCPMode(mode) == -1){
		printf("Set DHCP mode error!\n");
	}
	
	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdSetDhcpIpAssignment(int cmdType, struct cmd_arg *pCmdArg)
{
	struct in_addr ip;
	char mac[6];
	
	switch(cmdType)
	{
		case CMD_ADD_ASSIGN_IP:
			if(getArgValue(pCmdArg,"ip",(void *)&ip) != 1){
				printf("Please input IP address!\n");
				goto arg_err;
			}
			if(getArgValue(pCmdArg,"mac",(void *)mac) != 1){
				printf("Please input MAC address!\n");
				goto arg_err;
			}
			if(addDhcpReservedIp(&ip,mac) == -1){
				printf("Add reserved ip error!\n");
			}
			break;
		case CMD_DELETE_ASSIGN_IP:
			if(getArgValue(pCmdArg,"ip",(void *)&ip) != 1){
				printf("Please input IP address!\n");
				goto arg_err;
			}
			if(deleteDhcpReservedIp(&ip) == -1){
				printf("Delete reserved ip error!\n");
			}
			break;
	}

	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdDhcpRelaySetting(int cmdType, struct cmd_arg *pCmdArg)
{
	struct in_addr serverIp;
	
	if(getArgValue(pCmdArg,"server",(void *)&serverIp) != 1){
		printf("Please input relay server address!\n");
		goto arg_err;
	}

	if(setDhcpRelayServer(&serverIp) == -1){
		printf("Set dhcp relay server error!\n");
	}

	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdDhcpServerSetting(int cmdType, struct cmd_arg *pCmdArg)
{
	unsigned char dnsMode;
	char buf[64];
	char domainName[260];
	int leaseTime;
	int start_flag,end_flag;
#ifdef DHCPS_POOL_COMPLETE_IP
	struct in_addr start_ip,end_ip;
#else
	int start_host,end_host;
	unsigned char start,end;
#endif
	struct in_addr gateway,dns1,dns2,dns3;
	DHCP_SERVER_PARA dhcp_server_para;
	
	memset(&dhcp_server_para,0,sizeof(dhcp_server_para));
#ifdef DHCPS_POOL_COMPLETE_IP
	start_flag = getArgValue(pCmdArg,"poolstart",(void *)&start_ip);
	end_flag = getArgValue(pCmdArg,"poolend",(void *)&end_ip);
	if(start_flag == 1 && end_flag == 1){
		dhcp_server_para.pPoolstart = &start_ip;
		dhcp_server_para.pPoolend= &end_ip;
	}else if(start_flag == 1){
		printf("poolend is needed!\n");
		goto arg_err;
	}else if(end_flag == 1){
		printf("poolstart is needed!\n");
		goto arg_err;
	}
#else
	start_flag = getArgValue(pCmdArg,"poolstart",(void *)&start_host);
	end_flag = getArgValue(pCmdArg,"poolend",(void *)&end_host);
	if(start_flag == 1 && end_flag == 1){
		start = start_host;
		end = end_host;
		dhcp_server_para.pPoolstart = &start;
		dhcp_server_para.pPoolend= &end;
	}else if(start_flag == 1){
		printf("poolend is needed!\n");
		goto arg_err;
	}else if(end_flag == 1){
		printf("poolstart is needed!\n");
		goto arg_err;
	}
#endif

	if(getArgValue(pCmdArg,"gateway",(void *)&gateway) == 1){
		dhcp_server_para.pGateway = &gateway;
	}

	if(getArgValue(pCmdArg,"leasetime",(void *)&leaseTime) == 1){
		dhcp_server_para.pLeaseTime = &leaseTime;
	}

	if(getArgValue(pCmdArg,"domainname",(void *)domainName) == 1){
		dhcp_server_para.pDomain = domainName;
	}

#ifdef DHCPS_DNS_OPTIONS
	if(getArgValue(pCmdArg,"relaydns",(void *)buf) == 1){
		dnsMode = 0;
		dhcp_server_para.pDnsMode = &dnsMode;
	}else if(getArgValue(pCmdArg,"manualdns",(void *)buf) == 1){
		dnsMode = 1;
		dhcp_server_para.pDnsMode = &dnsMode;
		if(getArgValue(pCmdArg,"dns1",(void *)&dns1) == 1){
			dhcp_server_para.pDns1 = &dns1;
		}
		if(getArgValue(pCmdArg,"dns2",(void *)&dns2) == 1){
			dhcp_server_para.pDns2 = &dns2;
		}
		if(getArgValue(pCmdArg,"dns3",(void *)&dns3) == 1){
			dhcp_server_para.pDns3 = &dns3;
		}
	}
#endif

	if(setDhcpServer(&dhcp_server_para) == -1){
		printf("Set dhcp server error!\n");
	}
	
	return;

arg_err:
	printf("%s",Argerr_String);
}
#endif

#if 0
static void cmdDhcpServerSetting(int cmdType, struct cmd_arg *pCmdArg)
{
	int number;
	char string[256];
	struct in_addr ip;
	int start_flag,end_flag;
#ifdef DHCPS_POOL_COMPLETE_IP
	struct in_addr start_ip,end_ip;
#else
	int start_host,end_host;
	unsigned char start,end;
#endif

#ifdef DHCPS_POOL_COMPLETE_IP
	start_flag = getArgValue(pCmdArg,"poolstart",(void *)&start_ip);
	end_flag = getArgValue(pCmdArg,"poolend",(void *)&end_ip);
	if(start_flag == 1 && end_flag == 1){
		setDhcpsPoolRange(&start_ip,&end_ip);
	}else if(start_flag == 1){
		printf("poolend is needed!\n");
		goto arg_err;
	}else if(end_flag == 1){
		printf("poolstart is needed!\n");
		goto arg_err;
	}
#else
	start_flag = getArgValue(pCmdArg,"poolstart",(void *)&start_host);
	end_flag = getArgValue(pCmdArg,"poolend",(void *)&end_host);
	if(start_flag == 1 && end_flag == 1){
		start = start_host;
		end = end_host;
		setDhcpsPoolRange(start,end);
	}else if(start_flag == 1){
		printf("poolend is needed!\n");
		goto arg_err;
	}else if(end_flag == 1){
		printf("poolstart is needed!\n");
		goto arg_err;
	}
#endif
	
	if(getArgValue(pCmdArg,"gateway",(void *)&ip) == 1){
		setDhcpsGateway(&ip);
	}

	if(getArgValue(pCmdArg,"leasetime",(void *)&number) == 1){
		setDhcpsLeaseTime(number);
	}

	if(getArgValue(pCmdArg,"domainname",(void *)string) == 1){
		setDhcpsDomainName(string);
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_dhcp();
#endif

	return;

arg_err:
	printf("%s",Argerr_String);
}

#ifdef DHCPS_DNS_OPTIONS
static void cmdSetDhcpDnsOption(int cmdType, struct cmd_arg *pCmdArg)
{
	struct in_addr ip;
	char buf[32];

	if(getArgValue(pCmdArg,"relay",(void *)buf) == 1){	/*relay*/
		setDhcpsDnsOption(0);
	}else{	/*manually*/
		setDhcpsDnsOption(1);
		if(getArgValue(pCmdArg,"dns1",(void *)&ip) == 1){
			setDhcpsDns(MIB_DHCPS_DNS1,&ip);
			
			if(getArgValue(pCmdArg,"dns2",(void *)&ip) == 1){
				setDhcpsDns(MIB_DHCPS_DNS2,&ip);
			}
		
			if(getArgValue(pCmdArg,"dns3",(void *)&ip) == 1){
				setDhcpsDns(MIB_DHCPS_DNS3,&ip);
			}
		}
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_dhcp();
#endif
}
#endif
#endif

#ifdef CONFIG_USER_DHCP_SERVER
static void cmdShowDhcpAssignedIp(int cmdType, struct cmd_arg *pCmdArg)
{
	showDhcpsReservedIp();
}

static void cmdShowDhcpClient(int cmdType, struct cmd_arg *pCmdArg)
{
	showDhcpsClient();
}

static void cmdShowDhcpSetting(int cmdType, struct cmd_arg *pCmdArg)
{
	showDhcpsSetting();
}
#endif

static void cmdRestoreDefault(int cmdType, struct cmd_arg *pCmdArg)
{
	reset_cs_to_default(1);
	cmd_reboot();
}

static void cmdEnterLinuxShell(int cmdType, struct cmd_arg *pCmdArg)
{
	if (check_access(SECURITY_SUPERUSER))
		va_cmd("/bin/sh", 0, 1);
}

static void cmdRebootSystem(int cmdType, struct cmd_arg *pCmdArg)
{
	cmd_reboot();
}

static void cmdSaveConfig(int cmdType, struct cmd_arg *pCmdArg)
{
	saveMibAll();
}

static void cmdPrintHelpInformation(int cmdType, struct cmd_arg *pCmdArg)
{
	printf("The followings are available commands:\n");
	listRootLevelNodes(NULL,NULL);
}

static void cmdExitCli(int cmdType, struct cmd_arg *pCmdArg)
{
#ifdef EMBED
	if (!console_flag)
		unlink(CLI_RUNFILE);
#endif

	exit(0);
}

static void cmdShowSystemStatus(int cmdType, struct cmd_arg *pCmdArg)
{
	showStatus();
}

static void cmdShowArpTbl(int cmdType, struct cmd_arg *pCmdArg)
{
	showARPTable();
}

static void cmdShowFdbTbl(int cmdType, struct cmd_arg *pCmdArg)
{
	showMACs();
}

#ifdef ROUTING
static void cmdSetRoute(int cmdType, struct cmd_arg *pCmdArg)
{
	char ifname[64];
	int extPortAssigned = 0;
	struct in_addr dstip,netmask;
	MIB_CE_IP_ROUTE_T entry;

	switch(cmdType){
		case CMD_ADD_ROUTE:
			memset(&entry,0,sizeof(MIB_CE_IP_ROUTE_T));
			if(getArgValue(pCmdArg,"destination",(void *)entry.destID) != 1){
				printf("Please input destination ip address!\n");
				goto arg_err;
			}
			if(getArgValue(pCmdArg,"mask",(void *)entry.netMask) != 1){
				printf("Please input network mask!\n");
				goto arg_err;
			}
			if(getArgValue(pCmdArg,"nexthop",(void *)entry.nextHop) == 1){
				extPortAssigned = 1;
			}
			if(getArgValue(pCmdArg,"metric",(void *)&entry.FWMetric) != 1){
				entry.FWMetric = 1;
			}
			if(getArgValue(pCmdArg,"interface",(void *)ifname) == 1){
				if(!strcasecmp("any",ifname)){
					entry.ifIndex = 0xff;
				}else if((entry.ifIndex = getIfIndexByName(ifname)) != -1){
					extPortAssigned = 1;
				}else{
					printf("Not find this interface!\n");
					goto arg_err;
				}
			}else{
				entry.ifIndex = 0xff;
			}
			if(getArgValue(pCmdArg,"disabled",(void *)ifname) == 1){
				entry.Enable = 0;
			}else{
				entry.Enable = 1;
			}
			if(extPortAssigned == 0){
				printf("Please designate next hop or interface!\n");
				goto arg_err;
			}
			if(staticRouteAdd(&entry) == -1){
				printf("Add static route error!\n");
			}
			break;
		case CMD_DELETE_ROUTE:
			if(getArgValue(pCmdArg,"destination",(void *)&dstip) != 1){
				printf("Please input destination ip address!\n");
				goto arg_err;
			}
			if(getArgValue(pCmdArg,"mask",(void *)&netmask) != 1){
				printf("Please input network mask!\n");
				goto arg_err;
			}
			if(staticRouteDel(&dstip,&netmask) == -1){
				printf("Delete static route error!\n");
			}
			break;
	}

	return;
	
arg_err:
	printf("%s",Argerr_String);

}
#endif

static void cmdShowRouteTbl(int cmdType, struct cmd_arg *pCmdArg)
{
	showRoutes();
}

#ifdef ROUTING
static void cmdShowStaticRoutes(int cmdType, struct cmd_arg *pCmdArg)
{
	showStaticRoutes();
}
#endif

static void cmdShowInterface(int cmdType, struct cmd_arg *pCmdArg)
{
	FILE * fp;
	if(fp = fopen("/bin/ifconfig","r")){
		fclose(fp);
		va_cmd("/bin/ifconfig", 0, 1);
	}else{
		showLanInterface();
		showWanInterface();
	}
}

static void cmdShowInterfaceStatistics(int cmdType, struct cmd_arg *pCmdArg)
{
	showInterfaceStatistics();
}

#if defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL_IGMP_SNOOPING)
static void cmdSetIgmpSnooping(int cmdType, struct cmd_arg *pCmdArg)
{
	int enable;
	
	enable = (cmdType == CMD_ENABLE_IGMP_SNOOPING) ? 1 : 0;
	if(setLanIgmpSnooping(enable) == -1){
		printf("Set igmpsnooping error!\n");
	}
}
#endif

static void cmdCfgLanIp(int cmdType, struct cmd_arg *pCmdArg)
{
	struct in_addr ip1,mask1;
#ifdef CONFIG_SECONDARY_IP
	struct in_addr ip2,mask2;
	int dhcppool_num;
	int set_second_ip = 0;
#endif
	if(getArgValue(pCmdArg,"ip",(void *)&ip1) != 1){
		printf("Please input ip address!\n");
		goto arg_err;
	}
	if(getArgValue(pCmdArg,"mask",(void *)&mask1) != 1){
		printf("Please input mask!\n");
		goto arg_err;
	}
#ifdef CONFIG_SECONDARY_IP
	if(getArgValue(pCmdArg,"secondip",(void *)&ip2) == 1){
		set_second_ip = 1;
	}else{
		goto arg_ok;
	}
	if(getArgValue(pCmdArg,"secondmask",(void *)&mask2) != 1){	//if second ip is set, mask must be set
		printf("Please input second ip mask!\n");
		goto arg_err;
	}
	if(getArgValue(pCmdArg,"dhcppool",(void *)&dhcppool_num) != 1){	//if second ip is set, dhcp_pool must be set
		printf("Please input dhcppool number if second ip is set!\n");
		goto arg_err;
	}else if(dhcppool_num != 1 && dhcppool_num != 2){
		printf("dhcp_pool must be 1(primary LAN) or 2(Secondary LAN)!\n");
		goto arg_err;
	}
#endif

arg_ok:

#ifdef CONFIG_SECONDARY_IP
	if(setLanIpAddr(&ip1,&mask1,set_second_ip,&ip2,&mask2,dhcppool_num-1) == -1)
#else
	if(setLanIpAddr(&ip1,&mask1) == -1)
#endif
		printf("Set LAN ip error!\n");

	return;

arg_err:
	printf("%s",Argerr_String);
}

static void cmdShowInterfaceLan(int cmdType, struct cmd_arg *pCmdArg)
{
	showLanInterface();
}

/***************************command parse function******************************/

/* return value:
			1: get value;
			0: don't get value, but this argument is optional
			-1:don't get value, and this argument is required
*/
static int getArgValue(struct cmd_arg *pCmdArg, char * argName, void * pVal)
{
	struct cmd_arg * pArg = pCmdArg;
	int arg_flag;
	char *pMac;
	
	while(pArg->arg_id){
		if(!strcmp(argName,pArg->arg_name)){	//find
			arg_flag = pArg->arg_flag;
			if(arg_flag & ARG_SET){
				printf("arg_name:%s  \n",argName);
				if(arg_flag & ARG_VALUE){	//value needed
					switch(pArg->arg_id){
						case NUMBER1:
						case NUMBER2:
						case NUMBER3:
						case NUMBER4:
						case NUMBER5:
						case NUMBER6:
						case NUMBER7:
						case NUMBER8:
						case NUMBER9:
						case NUMBER10:
							printf("arg_value: %d\n",*(int *)(cmdArgTable[pArg->arg_id].arg_pnt));
							*(int *)pVal = *(int *)(cmdArgTable[pArg->arg_id].arg_pnt);
							break;
						case IPADDR1:
						case IPADDR2:
						case IPADDR3:
						case IPADDR4:
						case IPADDR5:
						case IPADDR6:
						case IPMASK1:
						case IPMASK2:
						case IPMASK3:
						case IPMASK4:
						case IPMASK5:
						case IPMASK6:
							printf("arg_value: %x\n",((struct in_addr *)(cmdArgTable[pArg->arg_id].arg_pnt))->s_addr);
							((struct in_addr *)pVal)->s_addr = ((struct in_addr *)(cmdArgTable[pArg->arg_id].arg_pnt))->s_addr;
							break;
						case MACADDR1:
						case MACADDR2:
						case MACADDR3:
						case MACADDR4:
						case MACADDR5:
						case MACADDR6:
							pMac = (char *)cmdArgTable[pArg->arg_id].arg_pnt;
							printf("arg_value: %02x:%02x:%02x:%02x:%02x:%02x\n",pMac[0],pMac[1],pMac[2],pMac[3],pMac[4],pMac[5]);
							memcpy(pVal,(void *)cmdArgTable[pArg->arg_id].arg_pnt,6);
							break;
						case STRING1:
						case STRING2:
						case STRING3:
						case STRING4:
						case STRING5:
						case STRING6:
						case STRING7:
						case STRING8:
							printf("arg_value: %s\n",(char *)(cmdArgTable[pArg->arg_id].arg_pnt));
							strcpy(pVal,cmdArgTable[pArg->arg_id].arg_pnt);
							break;
						case RANGE1:
						case RANGE2:
						case RANGE3:
							printf("arg_value: %d:%d\n",((struct arg_range *)(cmdArgTable[pArg->arg_id].arg_pnt))->start,((struct arg_range *)(cmdArgTable[pArg->arg_id].arg_pnt))->end);
							((struct arg_range *)pVal)->start = ((struct arg_range *)(cmdArgTable[pArg->arg_id].arg_pnt))->start;
							((struct arg_range *)pVal)->end = ((struct arg_range *)(cmdArgTable[pArg->arg_id].arg_pnt))->end;
							break;
						default:
							return -1;
					}
				}
				return 1;
			}else if(arg_flag & ARG_OPTIONAL){
				return 0;
			}else{
				//printf("get %s value error!\n",argName);
				return -1;
			}
		}
		pArg++;
	}

	printf("not find %s in arg list!\n",argName);
	return -1;		//not find
}

static void listRootLevelNodes(struct cmd_node * pCmdNode, char *pMatchStr)
{
	if (pCmdNode == NULL)
		pCmdNode = cmd_root;

	printf("\n");
	while(pCmdNode->cmd_str){
		if(pCmdNode->cmd_flag & SUSERONLY){
			if (loginLevel > SECURITY_SUPERUSER){
				pCmdNode++;
				continue;
			}
		}

		if(pMatchStr != NULL && pMatchStr[0] != 0){
			if(strlen(pMatchStr) <= strlen(pCmdNode->cmd_str)){
				if(!strncmp(pCmdNode->cmd_str,pMatchStr,strlen(pMatchStr))){
					goto displayCmd;
				}
				pCmdNode++;
				continue;
			}
		}
		
displayCmd:		
		if(pCmdNode->cmd_help)
			printf("    %-24.24s  %-64.64s\n",pCmdNode->cmd_str,pCmdNode->cmd_help);
		else
			printf("    %-24.24s\n",pCmdNode->cmd_str);
		pCmdNode++;
	}
	printf("\n");
}


static void listNextLevelNodes(struct cmd_node * pCmdNode, char *pMatchStr)
{
	struct cmd_node * pNode = pCmdNode->cmd_fwd;

	if(pNode){
		printf("\nCommands after \"%s\" are:\n\n",pCmdNode->cmd_str);
		while(pNode->cmd_str){
			if(pNode->cmd_flag & SUSERONLY){
				if (loginLevel > SECURITY_SUPERUSER){
					pNode++;
					continue;
				}
			}

			if(pMatchStr != NULL && pMatchStr[0] != 0){
				if(strlen(pMatchStr) <= strlen(pNode->cmd_str)){
					if(!strncmp(pNode->cmd_str,pMatchStr,strlen(pMatchStr))){
						goto displayCmd;
					}
				}
				pNode++;
				continue;
			}
displayCmd:			
			if(pNode->cmd_help)
				printf("    %-24.24s  %-64.64s\n",pNode->cmd_str,pNode->cmd_help);
			else
				printf("    %-24.24s\n",pNode->cmd_str);
			pNode++;
		}
	}
	printf("\n");
}

static void listCommandArgs(struct cmd_node * pCmdNode,char * pMatchStr)
{
	struct cmd_arg *pArg = pCmdNode->cmd_args;
	int haveArgHelp = 0;
	int haveNumberArg = 0;
	int haveIpMaskArg = 0;
	int haveMacArg = 0;
	int haveStringArg = 0;
	int haveOptionalArg = 0;
	int haveSelectionTag = 0;
	int haveRangeArg = 0;
	int arg_flag;
	
	if(pArg){
		printf("\nPlease input command arguments, as following:\n\n");
		
		while(pArg->arg_id){

			if(pArg->arg_help)
				haveArgHelp = 1;

			arg_flag = pArg->arg_flag;
			
			if(arg_flag & ARG_OPTIONAL){
				haveOptionalArg = 1;
				printf("[");
			}
			
			switch(pArg->arg_id){
				case NUMBER1:
				case NUMBER2:
				case NUMBER3:
				case NUMBER4:
				case NUMBER5:
				case NUMBER6:
				case NUMBER7:
				case NUMBER8:
				case NUMBER9:
				case NUMBER10:
					printf(" ");
					if(arg_flag & ARG_NAME)
						printf("%s",pArg->arg_name);
					if((arg_flag & ARG_BOTH) == ARG_BOTH)
						printf(" ");
					if(arg_flag & ARG_VALUE){
						printf("\"%s\"",cmdArgTable[pArg->arg_id].arg_helpname);
						haveNumberArg = 1;
					}
					printf(" ");
					break;
							
				case IPADDR1:
				case IPADDR2:
				case IPADDR3:
				case IPADDR4:
				case IPADDR5:
				case IPADDR6:
				case IPMASK1:
				case IPMASK2:
				case IPMASK3:
				case IPMASK4:
				case IPMASK5:
				case IPMASK6:
					printf(" ");
					if(arg_flag & ARG_NAME)
						printf("%s",pArg->arg_name);
					if((arg_flag & ARG_BOTH) == ARG_BOTH)
						printf(" ");
					if(arg_flag & ARG_VALUE){
						printf("\"***.***.***.***\"");
						haveIpMaskArg = 1;
					}
					printf(" ");
					break;
							
				case MACADDR1:
				case MACADDR2:
				case MACADDR3:
				case MACADDR4:
				case MACADDR5:
				case MACADDR6:
					printf(" ");
					if(arg_flag & ARG_NAME)
						printf("%s",pArg->arg_name);
					if((arg_flag & ARG_BOTH) == ARG_BOTH)
						printf(" ");
					if(arg_flag & ARG_VALUE){
						haveMacArg = 1;
						printf("\"##:##:##:##:##:##\"");	
					}
					printf(" ");
					break;
							
				case STRING1:
				case STRING2:
				case STRING3:
				case STRING4:
				case STRING5:
				case STRING6:
				case STRING7:
				case STRING8:
					printf(" ");
					if(arg_flag & ARG_NAME)
						printf("%s",pArg->arg_name);
					if((arg_flag & ARG_BOTH) == ARG_BOTH)
						printf(" ");
					if(arg_flag & ARG_VALUE){
						haveStringArg = 1;
						printf("\"%s\"",cmdArgTable[pArg->arg_id].arg_helpname);
					}
					printf(" ");
					break;
				case RANGE1:
				case RANGE2:
				case RANGE3:
					printf(" ");
					if(arg_flag & ARG_NAME)
						printf("%s",pArg->arg_name);
					if((arg_flag & ARG_BOTH) == ARG_BOTH)
						printf(" ");
					if(arg_flag & ARG_VALUE){
						printf("\"%s\"",cmdArgTable[pArg->arg_id].arg_helpname);
						haveRangeArg = 1;
					}
					printf(" ");
					break;
				case ARG_TAG:
					haveSelectionTag = 1;
					printf("%s",pArg->arg_name);
					break;
				default:
					printf("unknown argument type!\n");
					return;
			}

			if(arg_flag & ARG_OPTIONAL){
				printf("]");
			}
			
			pArg++;
		}

		printf("\n\nnote:\n");
		if(haveNumberArg || haveRangeArg)
			printf(" \"number\" and \"Number\" are decimal\n");
		if(haveIpMaskArg)
			printf(" \"*\" is decimal\n");
		if(haveMacArg)
			printf(" \"#\" is hexadecimal\n");
		if(haveStringArg)
			printf(" \"string\" has 256 characters maximal length\n");
		if(haveOptionalArg)
			printf(" The argument braced by \"[\" and \"]\" is optional\n");
		if(haveSelectionTag)
			printf("{...|...|...} means there are some selections to be choosed\n");
		if(haveArgHelp){
			printf("\n The followings are argument details:\n");
			pArg=pCmdNode->cmd_args;
			while(pArg->arg_id){
				if(pArg->arg_help){
					printf("  %s:  %s\n",cmdArgTable[pArg->arg_id].arg_helpname,pArg->arg_help);
				}
				pArg++;
			}
		}
		printf("\n");
	}
}

/*
 * generic routine to parse string into argv/argc format
 * the max 32 tokens can be handled.
 */
static char * stringparse(char *line)
{
	char *cp = line;
	unsigned int num;

	while ( *line != '\0' && *line != '\"' ) {
		if ( *line == '\\' ) {
			line++;
			switch ( *line++ ) {
			case 'n':
				*cp++ = '\n';
				break;
			case 't':
				*cp++ = '\t';
				break;
			case 'v':
				*cp++ = '\v';
				break;
			case 'b':
				*cp++ = '\b';
				break;
			case 'r':
				*cp++ = '\r';
				break;
			case 'f':
				*cp++ = '\f';
				break;
			case 'a':
				*cp++ = '\a';
				break;
			case '\\':
				*cp++ = '\\';
				break;
			case '\?':
				*cp++ = '\?';
				break;
			case '\'':
				*cp++ = '\'';
				break;
			case '\"':
				*cp++ = '\"';
				break;
			case 'x':
			  --line;
				num = strtoul( line, &line, 16 );
				*cp++ = (char) num;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			   --line;
				num = strtoul( line, &line, 8 );
				*cp++ = (char) num;
				break;
			case '\0':
				return 0;
			default:
				*cp++ = *(line - 1);
				break;
			};
		} else {
			*cp++ = *line++;
		}
	}

	if ( *line == '\"' )
		line++; 	/* skip final quote */
	*cp = '\0';		/* terminate string */
	return line;
}

static int cmdparse(char *line, char *argv[], int *argc0)
{
	char *cp;
	int qflag = FALSE;
	int argc;
	/* Remove cr/lf */

	for(argc = 0;argc < MAX_ARGC;){
		/* Skip leading white space */
		while(*line == ' ' || *line == '\t')	line++;
		if(*line == '\0')			break;
		/* return if comment character first non-white */
		if ( argc == 0  &&  *line == '#' )	return 0;
		/* Check for quoted token */
		if(*line == '"'){
			line++;	/* Suppress quote */
			qflag = TRUE;
		}
		argv[argc++] = line;	/* Beginning of token */

		if(qflag){
			/* Find terminating delimiter */
			if((line = stringparse(line)) == 0){
				return -1;
			}
			qflag = FALSE; // reset qflag after stringparse
		} else {
			/* Find space or tab. If not present,
			 * then we've already found the last
			 * token.
			 */
			if((cp = strchr(line,' ')) == 0
			 && (cp = strchr(line,'\t')) == 0){
				break;
			}
			*cp++ = '\0';
			line = cp;
		}
	}
	if (argc < 1) {		/* empty command line */
		argc = 1;
		argv[0] = "";
	}
	(*argc0)=argc;  /* update arg count */
	argv[argc]=0;   /* initial rest of field */

	return (0);
}

/*parse input string to argc/argv*/
static int parseCmdString2ArgcArgv(char *cmdBuf, int *argc, char *argv[])
{
	int len;
	
	if (cmdBuf[0]==0) 
		return 0;
	
	while ( *cmdBuf == ' ' ) 
		cmdBuf++; /* skip white space */

	len = strlen(cmdBuf) - 1;/*skip white space from the end*/
	for (; len >= 0; len--){
		if (cmdBuf[len] != ' ') 
			break;
	}
  	cmdBuf[len+1] = '\0';
	
	if(cmdparse(cmdBuf, argv, argc) < 0)
		return 0;
	
	if ( argv[0] == 0)
 		return 0;
	
	return 1;

}

static int setCommandArgument(char *pStr, struct cmd_arg  *pArg)
{
	int number;
	char tmpBuf[260];
	char * pString;
	unsigned int mac2[6];

	switch(pArg->arg_id){
		case NUMBER1:
		case NUMBER2:
		case NUMBER3:
		case NUMBER4:
		case NUMBER5:
		case NUMBER6:
		case NUMBER7:
		case NUMBER8:
		case NUMBER9:
		case NUMBER10:
			if (1 != sscanf(pStr, "%d", &number)){
				printf(" %s:%s is invalid!\n",pArg->arg_name,pStr);
				return 0;
			}
			*(int *)(cmdArgTable[pArg->arg_id].arg_pnt) = number;
			break;
					
		case IPADDR1:
		case IPADDR2:
		case IPADDR3:
		case IPADDR4:
		case IPADDR5:
		case IPADDR6:
			if(!isValidIpAddr(pStr) || (!inet_aton(pStr, cmdArgTable[pArg->arg_id].arg_pnt))){
				printf(" %s:%s is invalid!\n",pArg->arg_name,pStr);
				return 0;
			}
			break;
					
		case IPMASK1:
		case IPMASK2:
		case IPMASK3:
		case IPMASK4:
		case IPMASK5:
		case IPMASK6:
			if(!isValidNetmask(pStr, 1) || (!inet_aton(pStr, cmdArgTable[pArg->arg_id].arg_pnt))){
				printf(" %s:%s is invalid!\n",pArg->arg_name,pStr);
				return 0;
			}
			break;
					
		case MACADDR1:
		case MACADDR2:
		case MACADDR3:
		case MACADDR4:
		case MACADDR5:
		case MACADDR6:
			if(!isValidMacString(pStr)){
				printf(" %s: %s is invalid! it should be ##:##:##:##:##:##,\"#\" is 0~9 and a~f\n",pArg->arg_name,pStr);
				return 0;
			}
			if(mac17ToMac6(pStr, (char *)cmdArgTable[pArg->arg_id].arg_pnt) == -1){
				printf(" %s: %s is invalid! \n",pArg->arg_name,pStr);
				return 0;
			}
			break;
					
		case STRING1:
		case STRING2:
		case STRING3:
		case STRING4:
		case STRING5:
		case STRING6:
		case STRING7:
		case STRING8:
			if(strlen(pStr) > 256){
				printf("The string: \"%s\" is too long!\n",pStr);
			}
			strcpy(cmdArgTable[pArg->arg_id].arg_pnt,pStr);
			break;
			
		case RANGE1:
		case RANGE2:
		case RANGE3:
			strcpy(tmpBuf,pStr);
			pString = strtok(tmpBuf,":");	//start
			if(pString){
				if (1 != sscanf(pString, "%d", &number)){
					goto range_err;
				}
				((struct arg_range *)(cmdArgTable[pArg->arg_id].arg_pnt))->start = number;

				pString = strtok(NULL,":");	//end
				if(pString){
					if (1 != sscanf(pString, "%d", &number)){
						goto range_err;
					}
					((struct arg_range *)(cmdArgTable[pArg->arg_id].arg_pnt))->end = number;

					pString = strtok(NULL,":");	//recheck
					if(!pString){
						return 1;
					}
				}
			}
range_err:
			printf(" %s:%s is invalid! its format should be number:number\n",pArg->arg_name,pStr);
			return 0;

			break;
		default:
			return 0;
	}
	
	return 1;
}

static struct cmd_node * findMostLikelyNode(char *pStr, struct cmd_node *pNode, int *pNum)
{
	struct cmd_node *pMatchNode;
	int matchNum=0;

	while(pNode->cmd_str){
		if(strlen(pStr) <= strlen(pNode->cmd_str)){
			if(pNode->cmd_flag & SUSERONLY){
				if (loginLevel > SECURITY_SUPERUSER){
					pNode++;
					continue;
				}
			}
			if(!strcmp(pStr,pNode->cmd_str)){	/*string completely match*/
				return pNode;		
			}
			if(!strncmp(pNode->cmd_str,pStr,strlen(pStr))){
				matchNum++;
				pMatchNode = pNode;
			}
		}
		pNode++;
	}

	if(pNum)
		*pNum = matchNum;
	
	if(matchNum == 1){
		return pMatchNode;
	}else{
		return NULL;
	}
}

static struct cmd_arg  *findMatchSelection(struct cmd_arg  *pCmdArg, char *pStr)
{
	struct cmd_arg * pResult = NULL;
	
	while(pCmdArg->arg_id){
		if(pCmdArg->arg_flag & ARG_SELECT_END){
			break;
		}else if(pCmdArg->arg_flag & (ARG_SELECT_START | ARG_SELECT_DETACH)){
			pCmdArg++;	//next argument
			if(!pCmdArg->arg_id)
				break;
			/*the first argument after ARG_SELECT_START or ARG_SELECT_DETACH must not be optional */
			if((pCmdArg->arg_flag & ARG_NAME) && !strncmp(pCmdArg->arg_name,pStr,strlen(pStr))){
				return pCmdArg;
			}else if((pCmdArg->arg_flag & ARG_BOTH) == ARG_VALUE){ //only value, select it
				pResult = pCmdArg;
			}
		}
		pCmdArg++;
		
		/*find next argument with ARG_TAG*/
		while(pCmdArg->arg_id){
			if(pCmdArg->arg_flag & (ARG_SELECT_START | ARG_SELECT_END | ARG_SELECT_DETACH)){
				break;
			}
			pCmdArg++;
		}
	}

	return pResult;
}

static struct cmd_arg  *findArgAfterSelection(struct cmd_arg  *pCmdArg)
{
	int found = 0;
	
	if(!(pCmdArg->arg_flag & ARG_SELECT_END)){
		pCmdArg++;
		while(pCmdArg->arg_id){
			if(pCmdArg->arg_flag & ARG_SELECT_END){
				found = 1;
				break;
			}
			pCmdArg++;
		}
	}else{
		found = 1;
	}
	
	if(found){
		pCmdArg++;
		return (pCmdArg);
	}else{
		return NULL;
	}
}

static int noOtherMatchArgName(struct cmd_arg  *pCmdArg, char *pStr)
{
	pCmdArg++;
	while(pCmdArg->arg_id){
		if(!strncmp(pCmdArg->arg_name,pStr,strlen(pStr)))
			return 0;
		pCmdArg++;
	}

	return 1;
}


static int parseCommandArguments(struct cmd_arg  *pCmdArg, int argc, char *argv[])
{
	int i;
	struct cmd_arg  *pArg;

	pArg = pCmdArg;
		
	/*clear ARG_SET flags at first*/
	while(pArg->arg_id){
		pArg->arg_flag &= ~ARG_SET; 
		pArg++;
	}

	pArg = pCmdArg;
	for(i=0;i<argc;i++)
	{
arg_check:	
		if(!pArg || !pArg->arg_id){
			goto arg_err;
		}

		if((pArg->arg_flag & (ARG_SELECT_START | ARG_SELECT_END | ARG_SELECT_DETACH))){
			if(pArg->arg_flag & ARG_SELECT_START){
				pArg = findMatchSelection(pArg,argv[i]);
			}else{
				pArg = findArgAfterSelection(pArg);
			}
			goto arg_check;
		}
		
		if(pArg->arg_flag & ARG_OPTIONAL){	//optional argument must cpmpare name 
			if(!strcmp(pArg->arg_name, argv[i]) || (!strncmp(pArg->arg_name,argv[i],strlen(argv[i])) && noOtherMatchArgName(pArg,argv[i]))){
				if(pArg->arg_flag & ARG_VALUE){
					if(++i >= argc){
						goto arg_err;
					}
					if(setCommandArgument(argv[i],pArg) != 1)
						goto arg_err;
				}
				pArg->arg_flag |= ARG_SET;
				pArg++;
				continue;
			}else{
				pArg++;
				goto arg_check;
			}
		}else{		//required argument
			if((pArg->arg_flag & ARG_BOTH) == ARG_NAME){		//only name is needed
				if(!strncmp(pArg->arg_name,argv[i],strlen(argv[i]))){
					pArg->arg_flag |= ARG_SET;
					pArg++;
					continue;
				}else{
					goto arg_err;
				}
			}else if((pArg->arg_flag & ARG_BOTH) == ARG_VALUE){//only value is needed
				if(setCommandArgument(argv[i],pArg) != 1)
					goto arg_err;
				pArg->arg_flag |= ARG_SET;
				pArg++;
				continue;
			}else{	//both name and value are needed
				if(!strncmp(pArg->arg_name,argv[i],strlen(argv[i]))){
					if(++i >= argc){
						goto arg_err;
					}
					if(setCommandArgument(argv[i],pArg) != 1)
						goto arg_err;
					pArg->arg_flag |= ARG_SET;
					pArg++;
					continue;
				}else{
					goto arg_err;
				}
			}
		}
	}
#if 0
	/*check whether necessary argument is absent*/
	while(pArg && pArg->arg_id){
		if((pArg->arg_flag & ARG_OPTIONAL) == 0 && (pArg->arg_flag & ARG_BOTH) != 0){
			printf("Argument is uncomplete! please input %s\n",pArg->arg_name);
			return 0;
		}
		pArg++;
	}
#endif

	return 1;

arg_err:
	
	printf("can not recognise arguments\n");
	return 0;
}
static void processInputCommand(char *cmdBuf)
{
	int i,found=0;
	struct cmd_node *pCmdNode = cmd_root;
	struct cmd_node *pNode;
	struct cmd_arg  *pCmdArg = NULL;
	void	(*pCmdFunc)(int , struct cmd_arg *) = NULL; 
	void	(*pErrFunc)(struct cmd_node *,char *) = listRootLevelNodes; 
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	if(strlen(cmdBuf) == 0)	/*no command is inputted*/
		return;
	
	if(parseCmdString2ArgcArgv(cmdBuf,&argc,argv) == 0)
		goto cmd_err;

	//command debug
/*	for(i=0;i<argc;i++){
		printf("%s ",argv[i]);
	}
	printf("\n");
*/
	pNode = pCmdNode;
	
	/*find the cmdNode and command func*/
	for(i=0;i<argc;i++){
		pNode = findMostLikelyNode(argv[i],pNode,NULL);
		if(pNode == NULL){
			goto cmd_err;
		}else{
			pCmdNode = pNode;

			if(pNode->cmd_flag & END){
				pCmdFunc = pNode->cmd_func;
			}else{
				pCmdFunc = NULL;
			}
					
			if(pNode->err_func){
				pErrFunc = pNode->err_func;
			}

			if((i+1) == argc){
				if((pNode->cmd_flag & END) && (pNode->cmd_args == NULL))
					goto cmd_exe;
				else
					goto cmd_err;
			}else if(pNode->cmd_fwd){
				pNode = pNode->cmd_fwd;	// goto next level cmd string
				continue;
			}else if(pNode->cmd_args && (pNode->cmd_flag & END)){ 
				pCmdArg = pNode->cmd_args;	//start getting arguments
				i++;
				goto get_args;
			}else{
				goto cmd_err;	//command string length is error!
			}
		}
	}

get_args:
	/*get the command args*/
	if(pCmdArg){
		if(parseCommandArguments(pCmdArg,argc-i,&argv[i]) != 1)
			goto cmd_err;
	}

cmd_exe:
	/*execute command function; if no command function, do cmd_err*/
	if(pCmdFunc){
		pCmdFunc(pCmdNode->cmd_type,pCmdNode->cmd_args);
		return;
	}
	
cmd_err:
	printf("command error!\n");
	if(pErrFunc)
		pErrFunc(pCmdNode,NULL);
}

#ifdef CONFIG_CLI_TAB_FEATURE
int listNextCmdNodeOrArgs(char *cmdBuf)
{
	char matchStr[256];
	int i,cmdLen,spaceEnded=0, matchNum=0;
	struct cmd_node *pCmdNode = cmd_root;
	struct cmd_node *pNode;
	void	(*pErrFunc)(struct cmd_node *, char *) = listRootLevelNodes; 
	int argc = 0;
	char *argv[MAX_ARGC] = {0};

	memset(matchStr,0,256);
	
	if((cmdLen = strlen(cmdBuf)) == 0){	/*no command is inputted*/
		goto cmd_err;
	}

	if(cmdBuf[cmdLen-1] == ' ')
		spaceEnded = 1;
	
	if(parseCmdString2ArgcArgv(cmdBuf,&argc,argv) == 0)
		return -1;

	pNode = pCmdNode;
	
	/*find the cmdNode and command func*/
	for(i=0;i<argc;i++){
		pNode = findMostLikelyNode(argv[i],pNode,&matchNum);
		if(pNode == NULL){
			if((i+1) == argc){
				if(matchNum > 0){
					strcpy(matchStr,argv[i]);
					goto cmd_err;
				}
				else
					return -1;
			}
			else
				return -1;
		}else{
			pCmdNode = pNode;

			if(pNode->err_func){
				pErrFunc = pNode->err_func;
			}

			if((i+1) == argc){
				if(strcmp(argv[i],pNode->cmd_str) && spaceEnded == 0){	//complete the last command string
					strcpy(cmdBuf,(pNode->cmd_str+strlen(argv[i])));
					strcat(cmdBuf," ");
					return(strlen(cmdBuf));
				}
				if(pErrFunc == listRootLevelNodes){
					return -1;
				}
				goto cmd_err;
			}else if(pNode->cmd_fwd){
				pNode = pNode->cmd_fwd;	// goto next level cmd string
			}else if(pNode->cmd_args){
				goto cmd_err;
			}else{
				return -1;
			}
		}
	}
	
cmd_err:
	
	if(pErrFunc)
		pErrFunc(pCmdNode,matchStr);
	
	return 0;
}
	
#endif


/***********************************login function***********************************/

static int auth_cli(const char *name, const char *passwd)
{
	char *xpasswd;
	struct passwd *p;

	p = getpwnam (name);
	if (p == NULL)
		return 1;

	if (p->pw_uid != 0)
		return 1;
		
#if defined(HAVE_GETSPNAM) && defined(HAVE_SHADOW_H)
	if (p->pw_passwd == NULL || strlen (p->pw_passwd) == 1)
	{
		struct  spwd *spw;

		setspent ();
		spw = getspnam (p->pw_name);
		if (spw != NULL)
			p->pw_passwd = spw->sp_pwdp;
		endspent ();
	}
#endif 
	xpasswd = crypt (passwd, p->pw_passwd);
	return  (!xpasswd || strcmp (xpasswd, p->pw_passwd) != 0);
}

static int check_access(SECURITY_LEVEL level)
{
	if (loginLevel > level) {
		printf(NOT_AUTHORIZED);
		return 0;
	}
	return 1;
}

static struct passwd *getrealpass(const char *user) {
	struct passwd *pwp;
	
	pwp = getpwnam(user);
	if (pwp == NULL)
		return NULL;
	return pwp;
}

static void leave(void)
{
	_exit(exstat);
	/* NOTREACHED */
}

#ifdef EMBED
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = CLI_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}
#endif

static void sigexit(int dummy)
{
	if (!console_flag)
		unlink(CLI_RUNFILE);
	exit(0);
}

int main(int argc, char *argv[])
{
	int auth_denied=1; //auth_denied=1: Login Incorrect, auth_denied=0: Login Success
	int flag, len, login=0;
	unsigned char uname[10], *upasswd;
	struct passwd *pwp;
	char loginUser[MAX_NAME_LEN];
	char cmdBuf[BUFSIZ];
	
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, sigexit);
	signal(SIGHUP, sigexit);

	// check the login option
	// Usage: cli -l -c -u loginName
	while ((flag = getopt(argc, argv, "lcu:")) != EOF) {
		switch (flag) {
			case 'l':
				login = 1;
				break;
			case 'c':
				console_flag = 1;
				break;
			case 'u':
				if (optarg == NULL)
					break;
				strncpy(loginUser, optarg, MAX_NAME_LEN);
				break;
			default:
				break;
		}
	}
	
	if (!console_flag)
#ifdef EMBED
		log_pid();
#endif
	// Kaohj, get the login priviledge
	pwp = getrealpass(loginUser);
	//printf("uid=%d\n", pwp->pw_uid);
	if (pwp && pwp->pw_uid == 0)
		loginLevel = SECURITY_ROOT;
	else
		loginLevel = SECURITY_USER;
	
	login_flag = login;
	while (1)
	{
		if (login && auth_denied) {
			uname[0] = '\0';
			while (strlen(uname) == 0) {
				if (0 == getInputString("User: ", uname, sizeof(uname)))
					printf("!! Input User Name !!\n");
			}
				
			upasswd = getpass("Password: ");
			if ((auth_denied = auth_cli(uname, upasswd)) != 0)
			{
				printf("Login Incorrect!\n\n");
				auth_denied = 1;
			}
		}
		else
		{
			if (exstat==0)
				leave();
#ifdef CONFIG_CLI_CMD_EDIT
			cmdedit_read_input(">",cmdBuf);
#else
			getInputString(">",cmdBuf,sizeof(cmdBuf));
#endif
			processInputCommand(cmdBuf);
		}
	}
	return 0;
}


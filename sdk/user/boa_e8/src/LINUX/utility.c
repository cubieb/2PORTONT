/*
 *      Utiltiy function to communicate with TCPIP stuffs
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: utility.c,v 1.845 2013/01/25 07:31:11 kaohj Exp $
 *
 */

/*-- System inlcude files --*/
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <ctype.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/sysinfo.h>

#include <netinet/if_ether.h>
#include <linux/if.h>
#include <linux/sockios.h>
#ifdef EMBED
#include <linux/config.h>
#endif
#ifdef CONFIG_DEV_xDSL
#include <linux/atmdev.h>
#endif

/*-- Local include files --*/
#include "mib.h"
#include "utility.h"
#include "adsl_drv.h"

/* for open(), lseek() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.h"

#include "wireless.h"
#ifdef CONFIG_ATM_CLIP
#include <linux/atmclip.h>
#include <linux/atmarp.h>
#endif

#include <linux/if_bridge.h>
#include <netdb.h>
#include <netinet/ip_icmp.h>
// Kaohj -- get_net_link_status() and get_net_link_info()
#include <linux/ethtool.h>
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
#include "rtusr_rg_api.h"
#endif

#ifdef CONFIG_HWNAT
#include "hwnat_ioctl.h"
#endif

#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif

#if defined(SUPPORT_MCAST_TEST) && defined(CONFIG_GPON_FEATURE)
#if defined(CONFIG_RTK_L34_ENABLE)
#include <rtk_rg_liteRomeDriver.h>
#else
#include "rtk/ponmac.h"
#include "rtk/gpon.h"
#include "rtk/epon.h"
#endif
#endif

#define PR_VC_START	1
#define	PR_PPP_START	16

#ifdef POLICY_ROUTING_DNSV4RELAY
static int g_vc_entry = 0;
#endif

#if defined CONFIG_IPV6
#include "ipv6_info.h"
#endif

const char LANIF[] = "br0";
const char LAN_ALIAS[] = "br0:0";	// alias for secondary IP
const char LAN_IPPT[] = "br0:1";	// alias for IP passthrough
const char ELANIF[] = "eth0";
#ifdef CONFIG_RTL_MULTI_LAN_DEV
const char* ELANVIF[] = {ALIASNAME_ELAN0, ALIASNAME_ELAN1, ALIASNAME_ELAN2, ALIASNAME_ELAN3};
const char* SW_LAN_PORT_IF[] = {ALIASNAME_ELAN0, ALIASNAME_ELAN1, ALIASNAME_ELAN2, ALIASNAME_ELAN3}; // used for iptables
#else
const char* ELANVIF[] = {"eth0"};
const char* SW_LAN_PORT_IF[] = {ALIASNAME_ELAN0, ALIASNAME_ELAN1, ALIASNAME_ELAN2, ALIASNAME_ELAN3}; // used for iptables
#endif
const char BRIF[] = "br0";
#if defined(CONFIG_RTL8681_PTM) && !defined(CONFIG_RTL867X_PACKET_PROCESSOR)
const char PTMIF[] = "ptm0";
#else
const char PTMIF[] = "ptm0_pp";
#endif
#ifdef CONFIG_USB_ETH
const char USBETHIF[] = "usb0";
#endif //CONFIG_USB_ETH
//the name of wlan
#if defined(CONFIG_SLAVE_WLAN1_ENABLE) && !defined(CONFIG_MASTER_WLAN0_ENABLE)
const char*  wlan[]   = {"wlan1", "wlan1-vap0", "wlan1-vap1", "wlan1-vap2", "wlan1-vap3", ""};
const char*  wlan_itf[] = {"WLAN", "WLAN-AP1", "WLAN-AP2", "WLAN-AP3", "WLAN-AP4", ""};
#else
const char*  wlan[]   = {"wlan0", "wlan0-vap0", "wlan0-vap1", "wlan0-vap2", "wlan0-vap3",
						 "wlan1", "wlan1-vap0", "wlan1-vap1", "wlan1-vap2", "wlan1-vap3", ""};
const char*  wlan_itf[] = {"WLAN0", "WLAN0-AP1", "WLAN0-AP2", "WLAN0-AP3", "WLAN0-AP4",
						   "WLAN1", "WLAN1-AP1", "WLAN1-AP2", "WLAN1-AP3", "WLAN1-AP4", ""};

#endif
#ifdef WLAN_SUPPORT
const int wlan_en[] = {1 //wlan0
#ifdef WLAN_MBSSID
	,1, 1, 1, 1 //wlan0-vap0 ~ wlan0-vap3
#else
	,0, 0, 0, 0
#endif
#ifdef WLAN_DUALBAND_CONCURRENT
	,1 //wlan1
#ifdef WLAN_MBSSID
	,1, 1, 1, 1 //wlan1-vap0 ~ wlan1-vap3
#else
	,0, 0, 0, 0
#endif
#else
	,0, 0, 0, 0, 0
#endif
};
#else
const int wlan_en[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif
const char VC_BR[] = "0";
const char LLC_BR[] = "1";
const char VC_RT[] = "3";
const char LLC_RT[] = "4";
#ifdef CONFIG_ATM_CLIP
const char LLC_1577[] = "6";
#endif
const char PORT_DNS[] = "53";
const char PORT_DHCP[] = "67";
const char BLANK[] = "";
const char ARG_ADD[] = "add";
const char ARG_CHANGE[] = "change";
const char ARG_DEL[] = "del";
const char ARG_ENCAPS[] = "encaps";
const char ARG_QOS[] = "qos";
const char ARG_255x4[] = "255.255.255.255";
const char ARG_0x4[] = "0.0.0.0";
const char ARG_BKG[] = "&";
const char ARG_I[] = "-i";
const char ARG_O[] = "-o";
const char ARG_T[] = "-t";
const char ARG_TCP[] = "TCP";
const char ARG_UDP[] = "UDP";
#ifdef NEW_IP_QOS_SUPPORT
const char ARG_TCPUDP[] = "TCP/UDP";
#endif
const char ARG_NO[] = "!";
const char ARG_ICMP[] = "ICMP";
const char FW_BLOCK[] = "block";
const char FW_INACC[] = "inacc";
const char FW_IPFILTER[] = "ipfilter";
const char FW_BR_WAN[] = "br_wan";
const char FW_BR_WAN_OUT[] = "br_wan_out";
#ifdef NEW_PORTMAPPING
const char FW_DHCPS_DIS[] = "dhcps_disable";
#endif
const char FW_BR_PPPOE[] = "br_pppoe";
//const char FW_MACFILTER[] = "macfilter";
const char FW_MACFILTER[] = "macfilter_b";
const char FW_MACFILTER_ROUTER[] = "macfilter_r";
const char FW_APPFILTER[] = "appfilter";
const char FW_APP_P2PFILTER[] = "appp2pfilter";
const char FW_IPQ_MANGLE_DFT[] = "m_ipq_dft";
const char FW_IPQ_MANGLE_USER[] = "m_ipq_user";

const char FW_INTERNET_ACCESSRIGHT_BRIDGE[] = "internet_accessright_b";
const char FW_INTERNET_ACCESSRIGHT_ROUTER[] = "internet_accessright_r";
const char FW_STORAGE_ACCESSRIGHT_ROUTER[] = "storage_accessright_r";

#ifdef PORT_FORWARD_ADVANCE
const char FW_PPTP[] = "pptp";
const char FW_L2TP[] = "l2tp";
#endif
const char PORT_FW[] = "portfw";
const char IPTABLE_DMZ[] = "dmz";
const char IPTABLE_IPFW[] = "ipfw";
const char IPTABLE_IPFW2[] = "ipfw_PostRouting";
const char IPTABLES_PORTTRIGGER[] = "portTrigger";
const char IPTABLE_TR069[] = "tr069";
const char FW_DROP[] = "DROP";
const char FW_ACCEPT[] = "ACCEPT";
const char FW_RETURN[] = "RETURN";
const char FW_FORWARD[] = "FORWARD";
const char FW_INPUT[] = "INPUT";
const char FW_OUTPUT[] = "OUTPUT";
const char FW_PREROUTING[] = "PREROUTING";
const char FW_POSTROUTING[] = "POSTROUTING";
const char IPTABLE_VTLSUR[]="virtual_server";
const char FW_DPORT[] = "--dport";
const char FW_SPORT[] = "--sport";
const char FW_ADD[] = "-A";
const char FW_DEL[] = "-D";
const char FW_INSERT[] = "-I";
const char RMACC_MARK[] = "0x1000";
const char CONFIG_HEADER[] = "<Config_Information_File_8671>";
const char CONFIG_TRAILER[] = "</Config_Information_File_8671>";
const char CONFIG_XMLFILE[] = "/tmp/config.xml";
const char CONFIG_XMLENC[] = "/tmp/config.enc";
const char CONFIG_BIN[] = "/tmp/config.bin";
const char PPP_SYSLOG[] = "/tmp/ppp_syslog";
const char PPP_DEBUG_LOG[] = "/tmp/ppp_debug_log";
const char BACKUP_DIRNAME[] = "e8_Config_Backup";

const char ADSLCTRL[] = "/bin/adslctrl";
const char IFCONFIG[] = "/bin/ifconfig";
const char BRCTL[] = "/bin/brctl";
const char MPOAD[] = "/bin/mpoad";
const char MPOACTL[] = "/bin/mpoactl";
const char DHCPD[] = "/bin/udhcpd";
const char DHCPC[] = "/bin/udhcpc";
const char DNSRELAY[] = "/bin/dnsmasq";
const char DNSRELAYPID[] = "/var/run/dnsmasq.pid";
const char SPPPD[] = "/bin/spppd";
const char SPPPCTL[] = "/bin/spppctl";
const char WEBSERVER[] = "/bin/boa";
const char SNMPD[] = "/bin/snmpd";
const char ROUTE[] = "/bin/route";
const char IPTABLES[] = "/bin/iptables";
#ifdef CONFIG_IPV6
const char IP6TABLES[] = "/bin/ip6tables";
const char FW_IPV6FILTER[] = "ipv6filter";
const char FW_IPV6REMOTEACC[] = "ipv6remoteacc";
const char ARG_ICMPV6[] = "ICMPV6";
#endif
const char EMPTY_MAC[MAC_ADDR_LEN] = {0};
/*ql 20081114 START need ebtables support*/
const char EBTABLES[] = "/bin/ebtables";
const char ZEBRA[] = "/bin/zebra";
const char RIPD[] = "/bin/ripd";
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
const char OSPFD[] = "/bin/ospfd";
#endif
const char ROUTED[] = "/bin/routed";
const char IGMPROXY[] = "/bin/igmpproxy";
const char MLDPROXY[] = "/bin/ecmh";		// Mason Yu. MLD Proxy
#if defined(CONFIG_RTL867X_NETLOG) && defined(CONFIG_USER_NETLOGGER_SUPPORT)
const char NETLOGGER[]="/bin/netlogger";
#endif
const char TC[] = "/bin/tc";
#ifdef TIME_ZONE
const char SNTPC[] = "/bin/vsntp";
const char SNTPC_PID[] = "/var/run/vsntp.pid";
#endif
#ifdef CONFIG_USER_DDNS
const char DDNSC_PID[] = "/var/run/updatedd.pid";
#endif
const char ROUTED_PID[] = "/var/run/routed.pid";
#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
const char ZEBRA_PID[] = "/var/run/zebra.pid";
const char OSPFD_PID[] = "/var/run/ospfd.pid";
#endif
const char IGMPPROXY_PID[] = "/var/run/igmp_pid";
const char MLDPROXY_PID[] = "/var/run/ecmh.pid";	// Mason Yu. MLD Proxy

const char PROC_DYNADDR[] = "/proc/sys/net/ipv4/ip_dynaddr";
const char PROC_IPFORWARD[] = "/proc/sys/net/ipv4/ip_forward";
#ifdef CONFIG_IPV6
#if defined(CONFIG_USER_DHCPV6_ISC_DHCP411) || defined(CONFIG_USER_RADVD)
const char RADVD_CONF[] = "/var/radvd.conf";
const char RADVD_PID[] = "/var/run/radvd.pid";
#endif
const char PROC_IP6FORWARD[] = "/proc/sys/net/ipv6/conf/all/forwarding";
const char PROC_MC6FORWARD[] = "/proc/sys/net/ipv6/conf/all/mc_forwarding";
#endif // of CONFIG_IPV6
const char PROC_NET_ATM_PVC[] = "/proc/net/atm/pvc";
#ifdef _LINUX_2_6_
const char PROC_FORCE_IGMP_VERSION[] = "/proc/sys/net/ipv4/conf/default/force_igmp_version";
#endif
const char PPP_CONF[] = "/var/ppp/ppp.conf";
const char PPPD_FIFO[] = "/tmp/ppp_serv_fifo";
const char MPOAD_FIFO[] = "/tmp/serv_fifo";
const char DHCPC_PID[] = "/var/run/udhcpc.pid";
const char DHCPC_ROUTERFILE[] = "/var/udhcpc/router";
const char DHCPC_SCRIPT[] = "/etc/scripts/udhcpc.sh";
const char DHCPC_SCRIPT_NAME[] = "/var/udhcpc/udhcpc";
const char DHCPD_CONF[] = "/var/udhcpd/udhcpd.conf";
const char DHCPD_LEASE[] = "/var/udhcpd/udhcpd.leases";
const char DHCPSERVERPID[] = "/var/run/udhcpd.pid";
const char DHCPRELAYPID[] = "/var/run/dhcrelay.pid";
const char IPOA_IPINFO[] = "/tmp/IPoAHalfBridge";
const char MER_GWINFO[] = "/tmp/MERgw";
#ifdef CONFIG_USER_CUPS
const char CUPSDPRINTERCONF[] = "/var/cups/conf/printers.conf";
#endif // CONFIG_USER_CUPS
#define MINI_UPNPDPID  "/var/run/mini_upnpd.pid"	//cathy
#define MINIUPNPDPID  "/var/run/linuxigd.pid"
const char MINIDLNAPID[] = "/var/run/minidlna.pid";
const char STR_DISABLE[] = "Disabled";
const char STR_ENABLE[] = "Enabled";
const char STR_AUTO[] = "Auto";
const char STR_MANUAL[] = "Manual";
const char STR_UNNUMBERED[] = "unnumbered";
const char STR_ERR[] = "err";
const char STR_NULL[] = "null";
const char rebootWord0[] = "The System is Restarting ...";
const char rebootWord1[] = "This device has been configured and is rebooting.";
const char rebootWord2[] = "Close the Configuration window and wait"
			" for a minute before reopening your web browser."
			" If necessary, reconfigure your PC's IP address to match"
			" your new configuration.";
// TR-111: ManufacturerOUI, ProductClass
const char MANUFACTURER_OUI[] = DEF_MANUFACTUREROUI_STR;//"00E04C";
const char PRODUCT_CLASS[] = DEF_PRODUCTCLASS_STR;//"IGD";

#ifdef CONFIG_BOA_WEB_E8B_CH
const char PW_HOME_DIR[] = "/mnt";
#else
const char PW_HOME_DIR[] = "/tmp";
#endif
#ifdef CONFIG_USER_CLI
const char PW_CMD_SHELL[] = "/bin/cli";
#else
const char PW_CMD_SHELL[] = "/bin/sh";
#endif
#ifdef CONFIG_USER_SAMBA
const char SMBDPID[] = "/var/lock/smbd.pid";
#ifdef CONFIG_USER_NMBD
const char NMBDPID[] = "/var/lock/nmbd.pid";
#endif
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
const char *n0to7[] = {
	"n/a", "0", "1", "2", "3", "4", "5", "6", "7"
};

// Note: size of prioLevel depends on the IPQOS_NUM_PKT_PRIO
const char *prioLevel[] = {
	"p0", "p1", "p2", "p3", "p4", "p5", "p6", "p7"
};

//alex_huang
#ifdef CONFIG_8021P_PRIO
const char *set1ptable[]={"set1ptbl0","set1ptbl1","set1ptbl2","set1ptbl3","set1ptbl4","set1ptbl5","set1ptbl6","set1ptbl7"};
#ifdef NEW_IP_QOS_SUPPORT
const char *setpredtable[]={"setpredtbl0", "setpredtbl1", "setpredtbl2", "setpredtbl3", "setpredtbl4", "setpredtbl5", "setpredtbl6", "setpredtbl7"};
#endif
#endif
// priority mapping of packet priority against priority queue
// ex, priomap[3] == 2 means packet priority 3 is mapping to priority queue_2 by default
//const int priomap[8] = {3, 3, 2, 2, 2, 1, 1, 1};
//const int priomap[8] = {4, 4, 3, 3, 2, 2, 1, 1};
const int priomap[8] = {3, 4, 4, 3, 2, 2, 1, 1}; //cathy
const char *ipTos[] = {
	"n/a", "Normal Service", "Minimize Cost", "Maximize Reliability",
	"Maximize Throughput", "Minimize Delay"
};
#endif

const char *ppp_auth[] = {
	"AUTO", "PAP", "CHAP", "NONE"
};

const char errGetEntry[] = "Get table entry error!";

int startSSDP()
{
#ifdef CONFIG_USER_MINI_UPNPD
	char *argv[10];
	int i = 0, pid = 0;

	pid = read_pid((char *)MINI_UPNPDPID);
	if (pid > 0)
	{
		kill(pid, 9);
		unlink(MINI_UPNPDPID);
	}

	argv[i++]="/bin/mini_upnpd";
#ifdef WLAN_SUPPORT
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
	char wscd_pid_name[32];
	getWscPidName(wscd_pid_name);
	if (read_pid(wscd_pid_name) > 0){
		argv[i++]="-wsc";
		argv[i++]="/tmp/wscd_config";
	}
#endif
#endif

#ifdef CONFIG_USER_MINIUPNPD
	if (read_pid((char *)MINIUPNPDPID) > 0){
		argv[i++]="-igd";
		argv[i++]="/tmp/igd_config";
	}
#endif
	argv[i++]="&";
	argv[i]=NULL;
	do_cmd( argv[0], argv, 0 );
#endif
	return 0;
}

/*
vc0 return 1, vc1 return 2 ...
ppp0 return 11, ppp1 return 12 ...
*/
inline int caculate_tblid_ITF_SourceRoute(uint32_t ifid)
{
	int tbl_id;

	uint32_t ifindex;

	ifindex = ifid;

	//ifindex of vc* is 0xff01, 0xff02, ...
	//ifindex 0f ppp* is 0x00, 0x0101, 0x0202 ...
#ifdef CONFIG_ETHWAN
	//if(CHECK_NAS_IDX(ifindex))
	if( MEDIA_INDEX(ifindex)==MEDIA_ETH )
	{
		if(PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
			tbl_id = ETH_INDEX(ifindex) + ITF_SOURCE_ROUTE_NAS_START;
		else
			tbl_id = PPP_INDEX(ifindex) + ITF_SOURCE_ROUTE_PPP_START;
	}
	else
#endif /*CONFIG_ETHWAN*/
#ifdef CONFIG_PTMWAN
	if( MEDIA_INDEX(ifindex)==MEDIA_PTM )
	{
		if(PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
			tbl_id = PTM_INDEX(ifindex) + ITF_SOURCE_ROUTE_PTM_START;
		else
			tbl_id = PPP_INDEX(ifindex) + ITF_SOURCE_ROUTE_PPP_START;
	}
	else
#endif /*CONFIG_PTMWAN*/
	{
	//ifindex of vc* is 0xff01, 0xff02, ...
	//ifindex of ppp* is 0x00, 0x0101, 0x0202 ...
	if (PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
		tbl_id = VC_INDEX(ifindex) + ITF_SOURCE_ROUTE_VC_START;
	else
		tbl_id = PPP_INDEX(ifindex) + ITF_SOURCE_ROUTE_PPP_START;
	}

	// Mason Yu.
	//printf("ifindex=0x%x, tbl_id=0x%x\n", ifindex, tbl_id);
	return tbl_id;
}

//rule_mark[0] = "0x20000"  which means:  mark the 0 as 0x20000/0xe0000
const char*  rule_mark[] = {"0x20000/0xe0000",    "0x40000/0xe0000",
						   "0x60000/0xe0000",
						   "0x80000/0xe0000",
						   ""};

const char*  rule_mark_ppp[] = {"0xa0000/0xe0000", "0xc0000/0xe0000", "0xe0000/0xe0000",""};

#define ITF_SourceRoute_DHCP_MARK_NUM	4
#define ITF_SourceRoute_PPP_MARK_NUM	3

//eason
#ifdef _PRMT_USB_ETH_
//0:ok, -1:error
int getUSBLANMacAddr( char *p )
{
	unsigned char *hwaddr;
	struct ifreq ifr;
	strcpy(ifr.ifr_name, USBETHIF);
	do_ioctl(SIOCGIFHWADDR, &ifr);
	hwaddr = (unsigned char *)ifr.ifr_hwaddr.sa_data;
//	printf("The result of SIOCGIFHWADDR is type %d  "
//	       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x.\n",
//	       ifr.ifr_hwaddr.sa_family, hwaddr[0], hwaddr[1],
//	       hwaddr[2], hwaddr[3], hwaddr[4], hwaddr[5]);
	memcpy( p, hwaddr, 6 );
	return 0;
}

//0:high, 1:full, 2:low
int getUSBLANRate( void )
{
	struct ifreq ifr;
	strcpy(ifr.ifr_name, USBETHIF);
	do_ioctl( SIOCUSBRATE, &ifr );
//	fprintf( stderr, "getUSBLANRate=%d\n", *(int*)(&ifr.ifr_ifindex) ); //ifr_ifru.ifru_ivalue );
	return ifr.ifr_ifindex;//ifr_ifru.ifru_ivalue;
}

//0:up, 1:nolink
int getUSBLANStatus(void )
{
	struct ifreq ifr;
	strcpy(ifr.ifr_name, USBETHIF);
	do_ioctl( SIOCUSBSTAT, &ifr );
//	fprintf( stderr, "getUSBLANStatus=%d\n", *(int*)(&ifr.ifr_ifindex) ); //ifr_ifru.ifru_ivalue );
	return ifr.ifr_ifindex; //ifr_ifru.ifru_ivalue;
}
#endif

static char adslDevice[] = "/dev/adsl0";

static FILE* adslFp = NULL;

static const char *dhcp_mode[] = {
	"None", "DHCP Relay", "DHCP Server"
};

#ifdef IP_PASSTHROUGH
static void set_IPPT_LAN_access();
#endif

int virtual_port_enabled;
const int virt2user[] = {
	1, 2, 3, 4, 5
};

// Timer for auto search PVC
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
int autoHunt_found = 0;
int fin_AutoSearchPVC = 0;
#endif

// Mason Yu
#ifdef PORT_FORWARD_ADVANCE
const char *PFW_Gategory[] = {"VPN", "Game"};
const char *PFW_Rule[] = {"PPTP", "L2TP"};
#endif

//keep the same order with ITF_T
//if want to convert lan device name to br0, call LANDEVNAME2BR0() macro
const char *strItf[] = {
	"",		//ITF_ALL
	"",		//ITF_WAN
	"br0",		//ITF_LAN

	"eth0",		//ITF_ETH0
	"eth0_sw0",	//ITF_ETH0_SW0
	"eth0_sw1",	//ITF_ETH0_SW1
	"eth0_sw2",	//ITF_ETH0_SW2
	"eth0_sw3",	//ITF_ETH0_SW3

	"wlan0",	//ITF_WLAN0
	"wlan0-vap0",	//ITF_WLAN0_VAP0
	"wlan0-vap1",	//ITF_WLAN0_VAP1
	"wlan0-vap2",	//ITF_WLAN0_VAP2
	"wlan0-vap3",	//ITF_WLAN0_VAP3

	"wlan1",	//ITF_WLAN0
	"wlan1-vap0",	//ITF_WLAN0_VAP0
	"wlan1-vap1",	//ITF_WLAN0_VAP1
	"wlan1-vap2",	//ITF_WLAN0_VAP2
	"wlan1-vap3",	//ITF_WLAN0_VAP3

	"usb0",		//ITF_USB0

	""		//ITF_END
};

int IfName2ItfId(char *s)
{
	int i;
	if( !s || s[0]==0 ) return ITF_ALL;
	if( (strncmp(s, "ppp", 3)==0) || (strncmp(s, "vc", 2)==0)
		|| (strncmp(s, "nas", 3)==0) || (strncmp(s, "ptm", 3)==0))
		return ITF_WAN;

	for( i=0;i<ITF_END;i++ )
	{
		if( strcmp( strItf[i],s )==0 ) return i;
	}

	return -1;
}

// Mason Yu. 090903
int do_ioctl(unsigned int cmd, struct ifreq *ifr)
{
	int skfd, ret;

	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return (-1);
	}

	ret = ioctl(skfd, cmd, ifr);
	close(skfd);
	return ret;
}

/*
 *	Check if a host is direct connected to local interfaces.
 *	pEntry: check with local interface pEntry; null to check all local interfaces.
 */
int isDirectConnect(struct in_addr *haddr, MIB_CE_ATM_VC_Tp pEntry)
{
	char buff[256];
	int flgs;
	struct in_addr dest, mask;
	FILE *fp;

	if (pEntry) {
		dest = *((struct in_addr *)pEntry->ipAddr);
		mask = *((struct in_addr *)pEntry->netMask);
		if ((dest.s_addr & mask.s_addr) == (haddr->s_addr & mask.s_addr)) {
			//printf("dest=0x%x, mask=0x%x\n", dest.s_addr, mask.s_addr);
			return 1;
		}
	}
	else { // pEntry == NULL
		if (!(fp = fopen("/proc/net/route", "r"))) {
			printf("Error: cannot open /proc/net/route - continuing...\n");
			return 0;
		}
		fgets(buff, sizeof(buff), fp);
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			if (sscanf(buff, "%*s%x%*x%x%*d%*d%*d%x", &dest, &flgs, &mask) != 3) {
				printf("Unsuported kernel route format\n");
				fclose(fp);
				return 0;
			}
			if ((flgs & RTF_UP) && mask.s_addr != 0) {
				if ((dest.s_addr & mask.s_addr) == (haddr->s_addr & mask.s_addr)) {
					//printf("dest=0x%x, mask=0x%x\n", dest.s_addr, mask.s_addr);
					fclose(fp);
					return 1;
				}
			}
		}
		fclose(fp);
	}
	return 0;
}

/*
 * Get Interface Addr (MAC, IP, Mask)
 */
int getInAddr(char *interface, ADDR_T type, void *pAddr)
{
	struct ifreq ifr;
	int found=0;
	struct sockaddr_in *addr;

	strcpy(ifr.ifr_name, interface);
	if (do_ioctl(SIOCGIFFLAGS, &ifr) < 0)
		return (0);

	if (type == HW_ADDR) {
		if (do_ioctl(SIOCGIFHWADDR, &ifr) >= 0) {
			memcpy(pAddr, &ifr.ifr_hwaddr, sizeof(struct sockaddr));
			found = 1;
		}
	}
	else if (type == IP_ADDR) {
		if (do_ioctl(SIOCGIFADDR, &ifr) == 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
			found = 1;
		}
	}
	else if (type == DST_IP_ADDR) {
		if (do_ioctl(SIOCGIFDSTADDR, &ifr) == 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
			found = 1;
		}
	}
	else if (type == SUBNET_MASK) {
		if (do_ioctl(SIOCGIFNETMASK, &ifr) >= 0) {
			addr = ((struct sockaddr_in *)&ifr.ifr_addr);
			*((struct in_addr *)pAddr) = *((struct in_addr *)&addr->sin_addr);
			found = 1;
		}
	}
	return found;
}

int getInFlags(char *interface, int *flags)
{
	struct ifreq ifr;
	int found=0;

#ifdef EMBED
	strcpy(ifr.ifr_name, interface);

	if (do_ioctl(SIOCGIFFLAGS, &ifr) == 0) {
		if (flags)
			*flags = ifr.ifr_flags;
		found = 1;
	}
#endif
	return found;
}

int setInFlags(char *interface, int flags)
{
	struct ifreq ifr;
	int ret=0;

#ifdef EMBED
	strcpy(ifr.ifr_name, interface);
	ifr.ifr_flags = flags;

	if (do_ioctl(SIOCSIFFLAGS, &ifr) == 0)
		ret = 1;
#endif
	return ret;
}

int INET_resolve(char *name, struct sockaddr *sa)
{
	struct sockaddr_in *s_in = (struct sockaddr_in *)sa;

	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;

	/* Default is special, meaning 0.0.0.0. */
	if (strcmp(name, "default")==0) {
		s_in->sin_addr.s_addr = INADDR_ANY;
		return 1;
	}
	/* Look to see if it's a dotted quad. */
	if (inet_aton(name, &s_in->sin_addr)) {
		return 0;
	}
	/* guess not.. */
	return -1;
}

enum { ROUTE_ADD, ROUTE_DEL };
/*
 *	Add a route
 */
static int route_modify(struct rtentry *rt, int action)
{
	int skfd;

	if (rt==0)
		return -1;

	rt->rt_flags = RTF_UP;
	if (((struct sockaddr_in *)(&rt->rt_gateway))->sin_addr.s_addr)
		rt->rt_flags |= RTF_GATEWAY;

	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	// Added by Mason Yu. According the netmask, we input the correct dst address.
	((struct sockaddr_in *)(&rt->rt_dst))->sin_addr.s_addr =
	(((struct sockaddr_in *)(&rt->rt_dst))->sin_addr.s_addr & ((struct sockaddr_in *)(&rt->rt_genmask))->sin_addr.s_addr );

	if (action == ROUTE_ADD) {
	/* Tell the kernel to accept this route. */
		if (ioctl(skfd, SIOCADDRT, rt) < 0) {
			perror("SIOCADDRT");
			close(skfd);
			return -1;
		}
	} else {
		if (ioctl(skfd, SIOCDELRT, rt) < 0) {
			perror("SIOCDELRT");
			close(skfd);
			return -1;
		}
	}

	/* Close the socket. */
	(void) close(skfd);
	return 0;

}

static inline int INET_addroute(struct rtentry *rt)
{
	return route_modify(rt, ROUTE_ADD);
}

/*
*	Del a route
*/
static inline int INET_delroute(struct rtentry *rt)
{
	return route_modify(rt, ROUTE_DEL);
}

void setup_ipforwarding(int enable)
{
	FILE *fp;
	int ipv6forward;

	if (enable != 0)
		enable = 1;
	fp = fopen(PROC_IPFORWARD, "w");
	if(fp)
	{
		fprintf(fp, "%d\n", enable);
		fclose(fp);
	}
#ifdef CONFIG_IPV6
	//  Because the /proc/.../conf/all/forwarding value, will affect /proc/.../conf/vc0/forwarding value.
	fp = fopen(PROC_IP6FORWARD, "r+");
	if(fp)
	{
		fscanf(fp,"%d",&ipv6forward);
		if(ipv6forward !=enable)
			fprintf(fp, "%d\n", enable);
		fclose(fp);
	}
	if (fp = fopen(PROC_MC6FORWARD, "w"))
	{
		fprintf(fp, "%d\n", enable);
		fclose(fp);
	}
#endif
}

// update corresponding field of rtentry from MIB_CE_IP_ROUTE_T
static void updateRtEntry(MIB_CE_IP_ROUTE_T *pSrc, struct rtentry *pDst)
{
	struct sockaddr_in *s_in;

	//pDst->rt_flags = RTF_UP;
	s_in = (struct sockaddr_in *)&pDst->rt_dst;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)pSrc->destID;

	s_in = (struct sockaddr_in *)&pDst->rt_genmask;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)pSrc->netMask;

	//if (pSrc->nextHop[0]&&pSrc->nextHop[1]&&pSrc->nextHop[2]&&pSrc->nextHop[3]) {
	if (pSrc->nextHop[0] || pSrc->nextHop[1] || pSrc->nextHop[2] ||pSrc->nextHop[3]) {
		s_in = (struct sockaddr_in *)&pDst->rt_gateway;
		s_in->sin_family = AF_INET;
		s_in->sin_port = 0;
		s_in->sin_addr = *(struct in_addr *)pSrc->nextHop;
	}
}

/*del=>  1: delete the route entry,
         0: add the route entry(skip ppp part),
        -1: add the route entry*/
void route_cfg_modify(MIB_CE_IP_ROUTE_T *pRoute, int del, int entryID)
{
	struct rtentry rt;
	char ifname[IFNAMSIZ];

	if( pRoute==NULL ) return;
	if(!pRoute->Enable) return;

	memset(&rt, 0, sizeof(rt));
	updateRtEntry(pRoute, &rt);
	rt.rt_dev = ifGetName(pRoute->ifIndex, ifname, sizeof(ifname));
	if (pRoute->FWMetric > -1)
		rt.rt_metric = pRoute->FWMetric + 1;


	if (del>0) {
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
		// rg delete wan
		RG_del_static_route(pRoute);
#endif
		INET_delroute(&rt);
		return;
	}
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
	if(!strncmp(ifname,"ppp",3)){
		//for pppoe connection.
		struct in_addr inAddr;
		if(getInAddr(ifname,IP_ADDR,(void *)&inAddr)){
		//check if pppoe interface up!
			int wan_ifIndex=-1;
			int ret=-1;
			MIB_CE_ATM_VC_T vc_entry = {0};
			wan_ifIndex = getIfIndexByName(ifname);
			if (wan_ifIndex == -1){
				printf("%s-%d error get wan ifIndex",__func__,__LINE__);
				goto SKIP_RG_STATIC_ROUTE;
			}
			if(!getATMVCEntryByIfIndex(wan_ifIndex, &vc_entry)){
				printf("%s-%d error get wan ATMVC Entry",__func__,__LINE__);
				goto SKIP_RG_STATIC_ROUTE;
			}
			RG_add_static_route_PPP(pRoute,&vc_entry,entryID);
		}
	}else{
	rg_add_route(pRoute, entryID);
	}
	SKIP_RG_STATIC_ROUTE:
#endif
	INET_addroute(&rt);
}

#ifdef CONFIG_IPV6
void route_v6_cfg_modify(MIB_CE_IPV6_ROUTE_T *pRoute, int del)
{
	char ifname[IFNAMSIZ];
	char metric[5];

	if( pRoute==NULL ) return;
	if(!pRoute->Enable) return;

	ifGetName(pRoute->DstIfIndex, ifname, sizeof(ifname));
	sprintf(metric,"%d",pRoute->FWMetric);

	if(pRoute->DstIfIndex== DUMMY_IFINDEX)
	{
		if (del>0) {
			va_cmd("/bin/route", 8, 1,
					"-A", "inet6", "del", pRoute->Dstination, "gw",  pRoute->NextHop, "metric", metric);
			return;
		}

		ifGetName(pRoute->DstIfIndex, ifname, sizeof(ifname));
		//route -A inet6 add  2003::/3 gw 2003::1 metric 10 dev nas0_0
		va_cmd("/bin/route", 8, 1,
				"-A", "inet6", "add", pRoute->Dstination, "gw",  pRoute->NextHop, "metric", metric);
	}
	else
	{
		if (del>0) {
			va_cmd("/bin/route", 10, 1,
					"-A", "inet6", "del", pRoute->Dstination, "gw",  pRoute->NextHop, "metric", metric,"dev", ifname);
			return;
		}

		ifGetName(pRoute->DstIfIndex, ifname, sizeof(ifname));
		va_cmd("/bin/route", 10, 1,
				"-A", "inet6", "add", pRoute->Dstination, "gw",  pRoute->NextHop, "metric", metric,"dev", ifname);
	}
}
#endif

void route_ppp_ifup(unsigned long pppGW)
{
	unsigned int entryNum, i;
	char ifname0[IFNAMSIZ];//, *ifname;
	MIB_CE_IP_ROUTE_T Entry;
	struct rtentry rt;
#ifdef CONFIG_USER_CWMP_TR069
	int cwmp_msgid;
	struct cwmp_message cwmpmsg;
#endif

	entryNum = mib_chain_total(MIB_IP_ROUTE_TBL);

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
		{
			continue;
		}
		if( !Entry.Enable ) continue;

		memset(&rt, 0, sizeof(rt));

		rt.rt_dev = ifGetName(Entry.ifIndex, ifname0, sizeof(ifname0));

		if (rt.rt_dev && !strncmp(rt.rt_dev, "ppp", 3)) {
			updateRtEntry(&Entry, &rt);
			rt.rt_metric = Entry.FWMetric + 1;
			route_modify(&rt, ROUTE_ADD);
		}
		else if (Entry.ifIndex == DUMMY_IFINDEX) {	// Interface "any"
			struct in_addr *addr;
			addr = (struct in_addr *)&Entry.nextHop;
			if (addr->s_addr == pppGW) {
				updateRtEntry(&Entry, &rt);
				rt.rt_metric = Entry.FWMetric + 1;
				route_modify(&rt, ROUTE_ADD);
			}
		}
	}

#ifdef CONFIG_USER_CWMP_TR069
	SetTR069WANInterfacePPP();
#endif
#ifdef CONFIG_USER_CWMP_TR069
	if((cwmp_msgid = msgget((key_t)1234, 0)) > 0 )
	{
		cwmpmsg.msg_type = MSG_ACTIVE_NOTIFY;
		msgsnd(cwmp_msgid, (void *)&cwmpmsg, MSG_SIZE, 0);
	}
#endif
}

/*
* Convert ifIndex to system interface name, e.g. eth0,vc0...
*/
char *ifGetName(int ifindex, char *buffer, unsigned int len)
{
	MEDIA_TYPE_T mType;

	if ( ifindex == DUMMY_IFINDEX )
		return 0;
	if (PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
	{
		mType = MEDIA_INDEX(ifindex);
		if (mType == MEDIA_ATM)
#ifdef CONFIG_RTL_MULTI_PVC_WAN
			snprintf( buffer, len,  "vc%u_%u", VC_MAJOR_INDEX(ifindex),  VC_MINOR_INDEX(ifindex));
#else
			snprintf( buffer, len,  "vc%u", VC_INDEX(ifindex) );
#endif
		else if (mType == MEDIA_ETH)
#ifdef CONFIG_RTL_MULTI_ETH_WAN
			snprintf( buffer, len, "%s%d", ALIASNAME_MWNAS, ETH_INDEX(ifindex));
#else
			snprintf( buffer, len,  "%s%u", ALIASNAME_NAS, ETH_INDEX(ifindex) );
#endif
#ifdef CONFIG_PTMWAN
		else if (mType == MEDIA_PTM)
			snprintf( buffer, len, "%s%d", ALIASNAME_MWPTM, PTM_INDEX(ifindex));
#endif /*CONFIG_PTMWAN*/
		else if (mType == MEDIA_IPIP)    // Mason Yu. Add VPN ifIndex
			snprintf( buffer, len, "ipip%u", IPIP_INDEX(ifindex));
		else
			return 0;

	}else{
		snprintf( buffer, len,  "ppp%u", PPP_INDEX(ifindex) );

	}
	return buffer;
}


int isAnyPPPoEWan()
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		if(Entry.cmode == CHANNEL_MODE_PPPOE)
			return 1;
	}
	return 0;
}

int getNameByIP(char *ip, char *buffer, unsigned int len)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];
	struct in_addr inAddr;
	char *itfIP;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("getNameByIP: Get chain record error!\n");
			return 0;
		}

		if (Entry.enable == 0)
			continue;

		ifGetName(Entry.ifIndex,ifname,sizeof(ifname));
		if (getInAddr(ifname, IP_ADDR, (void *)&inAddr) == 1) {
			itfIP = inet_ntoa(inAddr);
			if(!strcmp(itfIP, ip)){
				strncpy(buffer, ifname, len);
				buffer[len-1]='\0';
				//printf("getNameByIP: The %s IPv4 address is %s. Found\n", ifname, itfIP);
				break;
			}
		}
	}

	if(i>= entryNum){
		printf("getNameByIP: not find this interface!\n");
		return 0;
	}

	return 1;
}

int getIfIndexByName(char *pIfname)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		ifGetName(Entry.ifIndex,ifname,sizeof(ifname));

		if(!strcmp(ifname,pIfname)){
			break;
		}
	}

	if(i>= entryNum){
		//printf("not find this interface!\n");
		return -1;
	}

	return(Entry.ifIndex);
}

/*ericchung add, get applicationtype by interface name */
int getapplicationtypeByName(char *pIfname)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		ifGetName(Entry.ifIndex,ifname,sizeof(ifname));

		if(!strcmp(ifname,pIfname)){
			break;
		}
	}

	if(i> entryNum){
		printf("not find this interface!\n");
		return -1;
	}

	return(Entry.applicationtype);
}

// Mason Yu. combine_1p_4p_PortMapping
#if defined(ITF_GROUP_1P) && defined(ITF_GROUP)
int setVlan(struct ifreq *ifr)
{
	int ret=0;

#ifdef EMBED
	if (do_ioctl(SIOCS8305VLAN, ifr) == 0)
		ret = 1;
#endif

	return ret;
}
#endif

#ifdef CONFIG_HWNAT
static inline int setEthPortMapping(struct ifreq *ifr)
{
	int ret=0;

#ifdef EMBED
	if (do_ioctl(SIOCSITFGROUP, ifr) == 0)
		ret = 1;

#endif
	return ret;
}
#endif

#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
void handleIGMPandMLD(int isIGMPenable, int isMLDenable)
{
	/*
	 * romDriver version after from 1984.
	 *
	 * ?ï¿½ï¿½?romeDriverä¿ï??ï¿½ï¿½??ï¿?gmpSnoopingç¸½ï¿½??ï¿½ï¿½?
	 *
	 * ?ï¿½ï¿½?å¤?°å?ï¿???proc/rg/mcast_protocolï¼?
	 * echo 1 > /proc/rg/mcast_protocol => IGMP_only
	 * echo 2 > /proc/rg/mcast_protocol => MLD_only
	 */

	printf("%s: isIGMPenable:%d, isMLDenable:%d\n",__func__,isIGMPenable,isMLDenable);

	if(!isIGMPenable && !isMLDenable){
		system("/bin/echo 0 > /proc/rg/igmpSnooping");
		printf("Both IGMP and MLD snooping are disabled!\n");
		return;
	}

	if( isIGMPenable && isMLDenable){
		system("/bin/echo 0 > /proc/rg/mcast_protocol");
		system("/bin/echo 1 > /proc/rg/igmpSnooping");
		printf("Both IGMP and MLD snooping are enabled!\n");
		return;
	}

	if( isIGMPenable ){
		system("/bin/echo 1 > /proc/rg/mcast_protocol");
		system("/bin/echo 1 > /proc/rg/igmpSnooping");
		printf("Only IGMP snopping is enabled!\n");
		return;
	}

	if( isMLDenable ){
		system("/bin/echo 2 > /proc/rg/mcast_protocol");
		system("/bin/echo 1 > /proc/rg/igmpSnooping");
		printf("Only MLD snooping is enabled!\n");
		return;
	}

	printf("Error! Should not goto here!\n");
}

static unsigned int hextol(unsigned char *hex)
{
printf("%02x %02x %02x %02x\n",hex[0], hex[1], hex[2], hex[3]);
	return ( (hex[0] << 24) | (hex[1] << 16) | (hex[2] << 8) | (hex[3]));
}
void rg_add_route(MIB_CE_IP_ROUTE_T *entry, int entryID)
{
	int i, n;
	FILE *fp;
	int flags;
	uint32_t mask, gateway;
	uint32_t dest, tmask, nH;
	struct IFADDRLIST *al;
	char buf[256], tdevice[256], device[256];
	struct in_addr gw_addr;
	struct sockaddr hwaddr;
	char mac_str[20]={0};
	char *str;
	fp = fopen("/proc/net/route", "r");
	n = 0;
	mask = 0;
	device[0] = '\0';
	nH = hextol(entry->nextHop);
//printf("%s-%d nH=%x\n",__func__,__LINE__,nH);
	while (fgets(buf, sizeof(buf), fp) != NULL) {
		++n;
		if (n == 1 && strncmp(buf, "Iface", 5) == 0)
			continue;
		i = sscanf(buf, "%255s %lx %lx %x %*s %*s %*s %x",
					tdevice, &dest, &gateway, &flags, &tmask);
//printf("tdevice:%s, dest=%x, gateway=%x, tmask=%x\n",tdevice,dest,gateway,tmask);
		if ((nH & tmask) == dest
		 && (tmask > mask || mask == 0) && (flags & RTF_UP) && (dest != 0x7f000000) && (dest !=0)
		) {
			mask = tmask;
			strcpy(device, tdevice);
//printf("[%s-%d] n:%d tdevice:%s, dest=%x, gateway=%x, tmask=%x\n",__func__,__LINE__,n,tdevice,dest,gateway,tmask);
			break;
		}
	}
	fclose(fp);
	if (device[0] == '\0')
		printf("can't find interface");
	else{
		getInAddr(device, HW_ADDR, &hwaddr);
		sprintf( mac_str, "%02X:%02X:%02X:%02X:%02X:%02X",
		(unsigned char)hwaddr.sa_data[0], (unsigned char)hwaddr.sa_data[1], (unsigned char)hwaddr.sa_data[2],
		(unsigned char)hwaddr.sa_data[3], (unsigned char)hwaddr.sa_data[4], (unsigned char)hwaddr.sa_data[5] );
//		printf("%s-%d mac_str:%s\n",__func__,__LINE__,mac_str);
		RG_add_static_route(entry,(char*)hwaddr.sa_data,entryID);
	}
}
/*use in reboot time to add static route*/
int Flush_RG_static_route(void)
{
 	unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); /* get chain record size */
	MIB_CE_IP_ROUTE_T Entry;
	int i;
	//AUG_PRT("%s-%d totalEntry=%d\n",__func__,__LINE__,totalEntry);
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
				return 0;
	//AUG_PRT("%s-%d Entry.ifIndex=%x\n",__func__,__LINE__,Entry.ifIndex);
			RG_del_static_route(&Entry);
			mib_chain_update(MIB_IP_ROUTE_TBL,(void *)&Entry,i);
		}
	return 0;
}
int check_RG_static_route(void)
{
 	unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); /* get chain record size */
	MIB_CE_IP_ROUTE_T Entry;
	int i;

	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
			return 0;
		rg_add_route(&Entry, i);
	}

	return 1;
}
int check_RG_static_route_PPP(MIB_CE_ATM_VC_T *vc_entry)
{
 	unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); /* get chain record size */
	MIB_CE_IP_ROUTE_T Entry;
	int i;
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
			return 0;
		//if user assign specify out interface (pppoe)
		if(Entry.ifIndex == vc_entry->ifIndex){
			//printf("%s-%d vc_entry->ifIndex=0x%x\n",__func__,__LINE__,vc_entry->ifIndex);
			RG_add_static_route_PPP(&Entry,vc_entry,i);
		}
	}
	return 1;
}
#endif

#if defined(CONFIG_RTL_IGMP_SNOOPING)
// enable/disable IGMP snooping
void __dev_setupIGMPSnoop(int flag)
{
	struct ifreq ifr;
	struct ifvlan ifvl;
	unsigned char mode;

#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
		mib_get(MIB_MPMODE, (void *)&mode);
		handleIGMPandMLD(flag, ((mode&MP_MLD_MASK)==MP_MLD_MASK)?1:0);
#else
	if(flag){
		system("/bin/echo 1 > /proc/br_igmpsnoop");
		system("/bin/echo 1 > /proc/br_igmpquery");
	}
	else{
		system("/bin/echo 0 > /proc/br_igmpsnoop");
		system("/bin/echo 0 > /proc/br_igmpquery");
	}
#endif
	printf("IGMP Snooping: %s\n", flag?"enabled":"disabled");
}
#endif

// Mason Yu. MLD snooping
//#ifdef CONFIG_IPV6
#if defined(CONFIG_RTL_MLD_SNOOPING)
// enable/disable MLD snooping
void __dev_setupMLDSnoop(int flag)
{
	unsigned char mode;

#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
	mib_get(MIB_MPMODE, (void *)&mode);
	handleIGMPandMLD(((mode&MP_IGMP_MASK)==MP_IGMP_MASK)?1:0, flag);
#else
	if(flag){
		system("/bin/echo 1 > /proc/br_mldsnoop");
		system("/bin/echo 1 > /proc/br_mldquery");
	}
	else{
		system("/bin/echo 0 > /proc/br_mldsnoop");
		system("/bin/echo 0 > /proc/br_mldquery");
	}
#endif
	printf("MLD Snooping: %s\n", flag?"enabled":"disabled");
}
#endif

// ioctl for direct bridge mode, jiunming
void __dev_setupDirectBridge(int flag)
{
	struct ifreq ifr;
	int ret;

	strcpy(ifr.ifr_name, ELANIF);
	ifr.ifr_ifru.ifru_ivalue = flag;
#ifdef EMBED
	if( do_ioctl(SIOCDIRECTBR, &ifr)==0 ) {
		ret = 1;
		//printf("Set direct bridge mode %s!\n", flag?"enable":"disable" );
	}
	else {
		ret = 0;
		//printf("Set direct bridge mode error!\n");
	}
#endif

}

// Mason Yu, For Set IPQOS
//ql 20081117 START for ip qos
#ifdef CONFIG_USER_IP_QOS
#ifdef CONFIG_HWNAT
int hwnat_wanif_rule(void)
{
	hwnat_ioctl_cmd hifr;
	struct hwnat_ioctl_qos_cmd hiqc;
	struct rtl867x_hwnat_qos_rule *qos_rule;
	MIB_CE_ATM_VC_T Entry;
	int vcTotal, i;
	char ipWanif[IFNAMSIZ];
	memset(&hiqc, 0, sizeof(hiqc));
	hifr.type = HWNAT_IOCTL_QOS_TYPE;
	hifr.data = &hiqc;
	hiqc.type = QOSRULE_CMD;
	qos_rule = &(hiqc.u.qos_rule.qos_rule_pattern);
	unsigned int portmask;


	vcTotal = mib_chain_total(MIB_ATM_VC_TBL);

	for (i = 0; i < vcTotal; i++){

		portmask=0;
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);
		if(Entry.vlan==0)
			continue;
		if(Entry.vprio==0)
			continue;
		if(Entry.cmode != CHANNEL_MODE_BRIDGE)
			continue;
		ifGetName(Entry.ifIndex, ipWanif, sizeof(ipWanif));
		{
		int var;
		for(var = 0; var < 9; var ++)
		portmask|=Entry.itfGroup & (0x1 << var) ;
		}
		memcpy(qos_rule->outIfname, ipWanif, strlen(ipWanif));
		qos_rule->rule_type = RTL867x_IPQos_Format_srcPort_setMask;
		hiqc.u.qos_rule.qos_rule_remark_8021p = Entry.vprio-1;	// 802.1p //bcz 0 is meaning disalbe
		if(portmask==0)
			portmask=0xf;
		qos_rule->Format_srcPort_setMask_LanPortMask = (portmask<<1);
		//q_index
		hiqc.u.qos_rule.qos_rule_queue_index = 0;
		send_to_hwnat(hifr);
	}



}

static int hwnat_single_queue_setup(void)
{
	hwnat_ioctl_cmd hifr;
	struct hwnat_ioctl_qos_cmd hiqc;
	struct rtl867x_hwnat_qos_queue *qos_queue;

	memset(&hiqc, 0, sizeof(hiqc));
	hifr.type = HWNAT_IOCTL_QOS_TYPE;
	hifr.data = &hiqc;
	hiqc.type = OUTPUTQ_CMD;
	qos_queue = &(hiqc.u.qos_queue);
	qos_queue->action = QDISC_ENABLE;
	qos_queue->sp_num = 1;
	//qos_queue->bandwidth = -1;
	qos_queue->default_queue = 0;
	send_to_hwnat(hifr);
	return 0;
}

int setWanIF1PMark(void)
{
	int vcTotal, i;

	MIB_CE_ATM_VC_T Entry;
	vcTotal = mib_chain_total(MIB_ATM_VC_TBL);
	int idx;
	unsigned int HWQosEnable=0;
	unsigned int WanIF1PEnable=0;

	for (i = 0; i < vcTotal; i++)
	{
		mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);

		if(Entry.vlan==0)
			continue;
		if(Entry.vprio==0)
			continue;
		WanIF1PEnable=1;
		if(Entry.enableIpQos)
			HWQosEnable=1;
	}
	if(WanIF1PEnable)
	if(!HWQosEnable){
		//AUG_PRT("HWQOSDisable\n");
		hwnat_single_queue_setup();
		hwnat_wanif_rule();
	}
	return 1;
}
#else
int hwnat_wanif_rule(void) {}
int setWanIF1PMark(void) {}
#endif
#endif

// enable/disable IPQoS
void __dev_setupIPQoS(int flag)
{
//ql 20081117 START for ip qos
#ifdef CONFIG_USER_IP_QOS
	if ( flag == 0 )
		va_cmd("/bin/ethctl", 2, 1, "setipqos", "0");
	else if (flag == 1)
		va_cmd("/bin/ethctl", 2, 1, "setipqos", "1");
#endif
}

#ifdef QOS_DIFFSERV
static char* proto2layer2[] = {
    [0]" ",
    [1]"6",
    [2]"17",
    [3]"1",
};

static char* strPolicing[] = {
    [0]" ",
    [1]"drop",
    [2]"continue",
};

void cleanupDiffservRule(int idx)
{
	unsigned int num, i;
	MIB_CE_IP_QOS_T qEntry;
	char wanif[IFNAMSIZ];

	mib_chain_get(MIB_IP_QOS_TBL, idx, (void *)&qEntry);
	ifGetName(qEntry.ifIndex, wanif, sizeof(wanif));

	// tc qdisc del dev vc0 root
	va_cmd(TC, 5, 1, "qdisc", (char *)ARG_DEL, "dev", wanif, "root");
}

#if 0
int deleteDiffservEntry()
{
	unsigned int totalEntry, i;
	MIB_CE_IP_QOS_T entry;

	totalEntry = mib_chain_total(MIB_IP_QOS_TBL); /* get chain record size */
	// Delete all existed diffserv entry
	for (i = totalEntry - 1; i >= 0; i ++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&entry)) {
			printf("%s\n", errGetEntry);
			return 1;
		}
		if (entry.enDiffserv == 1) {
			// delete from chain record
			if (mib_chain_delete(MIB_IP_QOS_TBL, i) != 1) {
				printf("Delete MIB_IP_QOS_TBLchain record error!\n");
				return 1;
			}
		}
	}

	return 0;
}
#endif

static void diffserv_filter_rule(MIB_CE_IP_QOS_Tp qEntry, char *prio, char *classid)
{
	char *argv[60], wanif[IFNAMSIZ];
	char saddr[20], daddr[20], sport[6], dport[6], strdscp[6] = {0}, strUpLinkRate[10];
	char *psaddr, *pdaddr;
	int idx, i;

	ifGetName(qEntry->ifIndex, wanif, sizeof(wanif));

	// source ip, mask
	snprintf(saddr, 20, "%s", inet_ntoa(*((struct in_addr *)qEntry->sip)));
	if (strcmp(saddr, ARG_0x4) == 0)
		psaddr = 0;
	else {
		if (qEntry->smaskbit!=0)
			snprintf(saddr, 20, "%s/%d", saddr, qEntry->smaskbit);
		psaddr = saddr;
	}
	// destination ip, mask
	snprintf(daddr, 20, "%s", inet_ntoa(*((struct in_addr *)qEntry->dip)));
	if (strcmp(daddr, ARG_0x4) == 0)
		pdaddr = 0;
	else {
		if (qEntry->dmaskbit!=0)
			snprintf(daddr, 20, "%s/%d", daddr, qEntry->dmaskbit);
		pdaddr = daddr;
	}
	snprintf(sport, 6, "%d", qEntry->sPort);
	snprintf(dport, 6, "%d", qEntry->dPort);

	// Classifier setup for 1:0
	// tc filter add dev vc0 parent 1:0 protocol ip prio 1 u32
	//	match ip src 192.168.1.3/32 match ip dst 192.168.8.11/32
	//	match ip tos 0x38 0xff match ip protocol 6 0xff
	//	match ip sport 1090 0xff match ip dport 21 0xff
	//	police rate 500kbit burst 10k drop classid 1:1
	argv[1] = "filter";
	argv[2] = (char *)ARG_ADD;
	argv[3] = "dev";
	argv[4] = wanif;
	argv[5] = "parent";
	argv[6] = "1:0";
	argv[7] = "protocol";
	argv[8] = "ip";
	argv[9] = "prio";
	argv[10] = prio;
	argv[11] = "u32";
	idx = 12;

	// match filter
	// src ip
	if (psaddr != 0) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "src";
		argv[idx++] = psaddr;
	}
	// dst ip
	if (pdaddr != 0) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "dst";
		argv[idx++] = pdaddr;
	}
	//dscp match
#ifdef QOS_DSCP_MATCH
	if (0 != qEntry->qosDscp) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "tos";
		snprintf(strdscp, 6, "0x%x", (qEntry->qosDscp-1)&0xFF);
		argv[idx++] = strdscp;
		argv[idx++] = "0xff";
	}
#endif
	// protocol
	if (qEntry->protoType != PROTO_NONE) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "protocol";
		argv[idx++] = proto2layer2[qEntry->protoType];
		argv[idx++] = "0xff";
	}
	// src port
	if ((qEntry->protoType==PROTO_TCP ||
		qEntry->protoType==PROTO_UDP) &&
		qEntry->sPort != 0) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "sport";
		argv[idx++] = sport;
		argv[idx++] = "0xff";
	}
	// dst port
	if ((qEntry->protoType==PROTO_TCP ||
		qEntry->protoType==PROTO_UDP) &&
		qEntry->dPort != 0) {
		argv[idx++] = "match";
		argv[idx++] = "ip";
		argv[idx++] = "dport";
		argv[idx++] = dport;
		argv[idx++] = "0xff";
	}

	// police
	if (0 != qEntry->limitSpeed) {
		argv[idx++] = "police";
		argv[idx++] = "rate";
		snprintf(strUpLinkRate, 10, "%dKbit", qEntry->limitSpeed);
		argv[idx++] = strUpLinkRate;
		argv[idx++] = "burst";
		argv[idx++] = "10k";
		argv[idx++] = strPolicing[qEntry->policing];
	}

	argv[idx++] = "classid";
	argv[idx++] = classid;
	argv[idx++] = NULL;

	printf("%s", TC);
	for (i=1; i<idx-1; i++)
		printf(" %s", argv[i]);
	printf("\n");
	do_cmd(TC, argv, 1);
}

static void diffserv_HTB_bw_div(MIB_CE_IP_QOS_Tp qEntry, char *classid)
{
	unsigned short ceil, rateBE;
	char wanif[IFNAMSIZ], strCeil[10], strRate[10], strRateBE[10];
	int htbrate = 0;

	ifGetName(qEntry->ifIndex, wanif, sizeof(wanif));

	//patch: actual bandwidth maybe a little greater than configured limit value, so I minish 7% of the configured limit value ahead.
	//ceil = qEntry->totalBandwidth / 100 * 93;
	ceil = qEntry->totalBandwidth;
	htbrate = (qEntry->htbRate>=qEntry->totalBandwidth)?qEntry->totalBandwidth-100:qEntry->htbRate;
	rateBE = ceil - htbrate;
	snprintf(strCeil, 10, "%dKbit", ceil);
	snprintf(strRate, 10, "%dKbit", htbrate);
	snprintf(strRateBE, 10, "%dKbit", rateBE);

	// tc qdisc add dev vc0 parent 1:0 handle 2:0 htb
	va_cmd(TC, 9, 1, "qdisc", (char *)ARG_ADD, "dev", wanif, "parent", "1:0", "handle", "2:0", "htb");
	// tc class add dev vc0 parent 2:0 classid 2:1 htb rate 1Mbit ceil 1Mbit
	va_cmd(TC, 13, 1, "class", (char *)ARG_ADD, "dev", wanif, "parent", "2:0", "classid", "2:1", "htb", "rate", strCeil, "ceil", strCeil);
	// tc class add dev vc0 parent 2:1 classid 2:2 htb rate 1Mbit ceil 1Mbit
	va_cmd(TC, 13, 1, "class", (char *)ARG_ADD, "dev", wanif, "parent", "2:1", "classid", "2:2", "htb", "rate", strRate, "ceil", strCeil);
	// tc class add dev vc0 parent 2:1 classid 2:3 htb rate 1Mbit ceil 1Mbit
	va_cmd(TC, 13, 1, "class", (char *)ARG_ADD, "dev", wanif, "parent", "2:1", "classid", classid, "htb", "rate", strRateBE, "ceil", strCeil);
}

static void calculate_RED(MIB_CE_IP_QOS_Tp qEntry, char *strLimit, char *strMin, char *strMax, char *strBurst)
{
	int max, min, limit, burst;

	max = (int)(qEntry->totalBandwidth / 8 * qEntry->latency / 1000);
	min = (int)(max / 3);
	limit = (int)(8 * max);
	burst = (int)(( 2 * min * 1000 + max * 1000 ) / ( 3* 1000 ));

	snprintf(strLimit, 10, "%dKB", limit);
	snprintf(strMin, 10, "%dKB", min);
	snprintf(strMax, 10, "%dKB", max);
	snprintf(strBurst, 6, "%d", burst);
}

static void diffserv_be_queue(MIB_CE_IP_QOS_Tp qEntry, char *classid)
{
	char wanif[IFNAMSIZ], strMax[10], strMin[10], strLimit[10], strBurst[6], strBw[10];

	ifGetName(qEntry->ifIndex, wanif, sizeof(wanif));
	snprintf(strBw, 10, "%dKbit", qEntry->totalBandwidth);
	calculate_RED(qEntry, strLimit, strMin, strMax, strBurst);

	// tc qdisc add dev vc0 parent 2:3 red limit 6KB min 1.5KB max 4.5KB burst 20 avpkt 1000 bandwidth 1Mbit probability 0.4
	printf("%s qdisc %s dev %s parent %s red limit %s min %s max %s burst %s avpkt 1000 bandwidth %s probability 0.4\n", TC
		, (char *)ARG_ADD, wanif, classid, strLimit, strMin, strMax, strBurst, strBw);
	va_cmd(TC, 21, 1, "qdisc", (char *)ARG_ADD, "dev", wanif, "parent", classid,
		"red", "limit", strLimit, "min", strMin, "max", strMax, "burst", strBurst,
		"avpkt", "1000", "bandwidth", strBw, "probability", "0.4");
}

static int setupEFTraffic(int efindex)
{
	MIB_CE_IP_QOS_T qEntry;
	char wanif[IFNAMSIZ], prio[2], classid[6], strdscp[6];
	int phb;

	mib_chain_get(MIB_IP_QOS_TBL, efindex, (void *)&qEntry);
	phb = qEntry.m_ipprio << 3 | qEntry.m_iptos << 1;
	ifGetName(qEntry.ifIndex, wanif, sizeof(wanif));

	// Create the egress root "dsmark" qdisc on wan interface
	// tc qdisc add dev vc0 root handle 1:0 dsmark indices 64 default_index 2
	va_cmd(TC, 12, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
		"root", "handle", "1:0", "dsmark", "indices", "64", "default_index", "2");

	snprintf(prio, 2, "1");
	snprintf(classid, 6, "1:1");

	// Classifier setup for 1:0
	diffserv_filter_rule(&qEntry, prio, classid);

	// Classes to specify DSCPs
	// tc class change dev vc0 parent 1:0 classid 1:1 dsmark mask 0x3 value 0xb8
	snprintf(strdscp, 6, "0x%x", (phb << 2)&0xFF);
	va_cmd(TC, 13, 1, "class", "change", "dev", wanif,
		"parent", "1:0", "classid", classid, "dsmark", "mask", "0x3", "value", strdscp);
	// BE
	va_cmd(TC, 13, 1, "class", "change", "dev", wanif, "parent", "1:0", "classid", "1:2", "dsmark", "mask", "0x3", "value", "0x0");

	// bandwidth division with HTB
	snprintf(classid, 6, "2:3");
	diffserv_HTB_bw_div(&qEntry, classid);

	// queue setup
	// tc qdisc add dev vc0 parent 2:2 pfifo limit 5
	va_cmd(TC, 9, 1, "qdisc", (char *)ARG_ADD, "dev", wanif, "parent", "2:2", "pfifo", "limit", "5");
	diffserv_be_queue(&qEntry, classid);

	// classifier setup for 2:1
	// tc filter add dev vc0 parent 2:0 protocol ip prio 1 handle 1 tcindex classid 2:2
	va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif, "parent", "2:0", "protocol", "ip",
		"prio", "1", "handle", "1", "tcindex", "classid", "2:2");
	// tc filter add dev vc0 parent 2:0 protocol ip prio 2 handle 2 tcindex classid 2:3
	va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif, "parent", "2:0", "protocol", "ip",
		"prio", "2", "handle", "2", "tcindex", "classid", classid);

	return 0;
}

static int setupAFTraffic(int *afindex, int aftotal, char *wanif)
{
	MIB_CE_IP_QOS_T qEntry;
	char prio[2], classid[6], strdscp[6];
	int i, phb;
	//int htbrate = 0;
	char strMax[10], strMin[10], strLimit[10], strBurst[6], strBw[10], strDP[2], strProbability[6];

	// Create the root "dsmark" qdisc on wan interface
	// tc qdisc add dev vc0 handle 1:0 root dsmark indices 64
	va_cmd(TC, 10, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
		"handle", "1:0", "root", "dsmark", "indices", "64");

	// dsmark classes to specify DSCPs
	// tc class change dev vc0 parent 1:0 classid 1:11 dsmark mask 0x3 value 0x28
	for (i = 0; i < aftotal; i ++) {
		mib_chain_get(MIB_IP_QOS_TBL, afindex[i], (void *)&qEntry);
		phb = qEntry.m_ipprio << 3 | qEntry.m_iptos << 1;
		snprintf(strdscp, 6, "0x%x", (phb << 2)&0xFF);
		snprintf(classid, 6, "1:%d%d", qEntry.m_ipprio, qEntry.m_iptos);
		va_cmd(TC, 13, 1, "class", "change", "dev", wanif,
			"parent", "1:0", "classid", classid, "dsmark", "mask", "0x3", "value", strdscp);
	}
	// BE
	va_cmd(TC, 13, 1, "class", "change", "dev", wanif, "parent", "1:0", "classid", "1:5", "dsmark", "mask", "0x3", "value", "0x0");

	// Classifier setup for 1:0
	for (i = 0; i < aftotal; i ++) {
		mib_chain_get(MIB_IP_QOS_TBL, afindex[i], (void *)&qEntry);
		snprintf(prio, 2, "%d", i + 1);
		snprintf(classid, 6, "1:%d%d", qEntry.m_ipprio, qEntry.m_iptos);
		//htbrate += qEntry.limitSpeed;
		diffserv_filter_rule(&qEntry, prio, classid);
	}
	// BE: tc filter add dev vc0 parent 1:0 protocol ip prio 5 u32 match ip protocol 0 0 flowid 1:5
	va_cmd(TC, 18, 1, "filter", (char *)ARG_ADD, "dev", wanif,
		"parent", "1:0", "protocol", "ip", "prio", "5", "u32", "match", "ip", "protocol", "0", "0", "flowid", "1:5");

	// bandwidth division with HTB
	snprintf(classid, 6, "2:6");
	//htbrate = (htbrate>qEntry.totalBandwidth)?qEntry.totalBandwidth-100:htbrate;
	diffserv_HTB_bw_div(&qEntry, classid);

	// queue setup
	// GRED setup for AF class
	// tc qdisc add dev vc0 parent 2:2 gred setup DPs 3 default 2 grio
	va_cmd(TC, 13, 1, "qdisc", (char *)ARG_ADD, "dev", wanif,
		"parent", "2:2", "gred", "setup", "DPs", "3", "default", "2", "grio");
	// tc qdisc change dev vc0 parent 2:2 gred limit 6KB min 1.5KB max 4.5KB burst 20 avpkt 1000 bandwidth 1Mbit DP 1 probability 0.02 prio 2
	snprintf(strBw, 10, "%dKbit", qEntry.totalBandwidth);
	for (i = 0; i < 3; i ++) {
		calculate_RED(&qEntry, strLimit, strMin, strMax, strBurst);
		snprintf(strDP, 2, "%d", i + 1);
		snprintf(strProbability, 6, "0.0%d", 2 * (i + 1));
		snprintf(prio, 2, "%d", i + 2);
		va_cmd(TC, 25, 1, "qdisc", "change", "dev", wanif, "parent", "2:2",
			"gred", "limit", strLimit, "min", strMin, "max", strMax, "burst", strBurst,
			"avpkt", "1000", "bandwidth", strBw, "DP", strDP, "probability", strProbability, "prio", prio);
		printf("%s qdisc change dev %s parent 2:2 gred limit %s min %s max %s burst %s avpkt 1000 bandwidth %s DP %s probability %s prio %s\n"
			, TC, wanif, strLimit, strMin, strMax, strBurst, strBw, strDP, strProbability, prio);
	}
	// RED setup for BE
	diffserv_be_queue(&qEntry, classid);

	// classifier setup for 2:1
	// tc filter add dev vc0 parent 2:0 protocol ip prio 1 handle 17 tcindex classid 2:2
	for (i = 0; i < aftotal; i ++) {
		mib_chain_get(MIB_IP_QOS_TBL, afindex[i], (void *)&qEntry);
		snprintf(strdscp, 6, "%d", qEntry.m_ipprio * 16 + qEntry.m_iptos);
		va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif, "parent", "2:0", "protocol", "ip",
			"prio", "1", "handle", strdscp, "tcindex", "classid", "2:2");
	}
	// tc filter add dev vc0 parent 2:0 protocol ip prio 1 handle 5 tcindex classid 2:3
	va_cmd(TC, 15, 1, "filter", (char *)ARG_ADD, "dev", wanif, "parent", "2:0", "protocol", "ip",
		"prio", "1", "handle", "5", "tcindex", "classid", classid);

	return 0;
}

int setupDiffServ(void)
{
	unsigned int num, i;
	MIB_CE_IP_QOS_T qEntry;
	char wanif[IFNAMSIZ];
	unsigned char phbclass;
	int efIndex, afIndex[3], afcount = 0;

	mib_get(MIB_DIFFSERV_PHBCLASS, (void *)&phbclass);
	num = mib_chain_total(MIB_IP_QOS_TBL);
	for (i = 0; i < num; i ++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&qEntry))
			continue;
		if (qEntry.enDiffserv == 0) // non-Diffserv entry
			continue;
		//if (qEntry.m_ipprio == 5 && qEntry.m_iptos == 3) {	// EF
		if (phbclass == qEntry.m_ipprio) {
			if (phbclass == 5) {	// EF
				efIndex = i;
				break;
			}
			else {	// AF
				afIndex[afcount] = i;
				afcount ++;
				ifGetName(qEntry.ifIndex, wanif, sizeof(wanif));
			}
		}
	}
	if (num > 0) {
		if (afcount > 0)
			setupAFTraffic(afIndex, afcount, wanif);
		else
			setupEFTraffic(efIndex);
	}

	return 0;
}
#endif // #ifdef QOS_DIFFSERV

/*------------------------------------------------------------------
 * Get a list of interface info. (itfInfo) of the specified ifdomain.
 * where,
 * info: a list of interface info entries
 * len: max length of the info list
 * ifdomain: interface domain
 *-----------------------------------------------------------------*/
int get_domain_ifinfo(struct itfInfo *info, int len, int ifdomain)
{
	unsigned int swNum, vcNum;
	int i, num;
	int mib_wlan_num;
	char mygroup;
	char strlan[]="LAN0";
	char wanif[IFNAMSIZ];
	MIB_CE_ATM_VC_T pvcEntry;
	num=0;

#ifdef WLAN_SUPPORT
	int ori_wlan_idx;
#endif

	if (ifdomain&DOMAIN_ELAN) {
		// LAN ports
#ifdef IP_QOS_VPORT
		swNum = SW_LAN_PORT_NUM;
		for (i=0; i<swNum; i++) {
			strlan[3] = '0' + virt2user[i];	// user index
			info[num].ifdomain = DOMAIN_ELAN;
			info[num].ifid=i;	// virtual index
			strncpy(info[num].name, strlan, 8);
			num++;
			if (num > len)
				break;
		}
#else
		info[num].ifdomain = DOMAIN_ELAN;
		info[num].ifid=0;
		strncpy(info[num].name, strlan, 8);
		num++;
#endif
	}

#ifdef WLAN_SUPPORT
	if (ifdomain&DOMAIN_WLAN) {
		ori_wlan_idx = wlan_idx;

		for( wlan_idx = 0; wlan_idx < NUM_WLAN_INTERFACE; wlan_idx++ )
		{
			// wlan0
			mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
			info[num].ifdomain = DOMAIN_WLAN;
			info[num].ifid = 0 + wlan_idx * 10;  // Magician: ifid of wlan0 = 0; ifid of wlan1 = 10
			strncpy(info[num].name, getWlanIfName(), 8);
			num++;

//jim luo add it to support QoS on Virtual AP...
#ifdef WLAN_MBSSID
			for (i=1; i<IFGROUP_NUM; i++) {
				mib_get(MIB_WLAN_VAP0_ITF_GROUP + ((i-1)<<1), (void *)&mygroup);
				info[num].ifdomain = DOMAIN_WLAN;
				info[num].ifid = i + wlan_idx * 10;  // Magician: ifid of wlan0-vap0~3 = 1~4; ifid of wlan1-vap0~3 = 11~14
				sprintf(info[num].name, "wlan%d-vap%d", wlan_idx, i-WLAN_VAP_ITF_INDEX);
				num++;
			}
#endif  // WLAN_MBSSID
		}
		wlan_idx = ori_wlan_idx;
	}
#endif  // WLAN_SUPPORT

#ifdef CONFIG_USB_ETH
	if (ifdomain&DOMAIN_ULAN) {
		// usb0
		mib_get(MIB_USBETH_ITF_GROUP, (void *)&mygroup);
		info[num].ifdomain = DOMAIN_ULAN;
		info[num].ifid=0;
		sprintf(info[num].name, "%s", USBETHIF );
		num++;
	}
#endif //CONFIG_USB_ETH

	if (ifdomain&DOMAIN_WAN) {
		// vc
		vcNum = mib_chain_total(MIB_ATM_VC_TBL);

		for (i=0; i<vcNum; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			{
  				//boaError(wp, 400, "Get chain record error!\n");
  				printf("Get chain record error!\n");
				return -1;
			}

			if (pvcEntry.enable == 0)
				continue;

			info[num].ifdomain = DOMAIN_WAN;
			info[num].ifid=pvcEntry.ifIndex;
			ifGetName(pvcEntry.ifIndex, wanif, sizeof(wanif));
			strncpy(info[num].name, wanif, 8);
			num++;

			if (num > len)
				break;
		}
	}

	return num;
}

int read_pid(const char *filename)
{
	FILE *fp;
	int pid;

	if ((fp = fopen(filename, "r")) == NULL)
		return -1;
	fscanf(fp, "%d", &pid);
	fclose(fp);

	return pid;
}

// Added by Kaohj
//return 0:OK, other:fail
int do_cmd(const char *filename, char *argv[], int dowait)
{
	pid_t pid, wpid;
	int ret, status;
	sigset_t tmpset, origset;

	sigfillset(&tmpset);
	sigprocmask(SIG_BLOCK, &tmpset, &origset);
	pid = vfork();
	sigprocmask(SIG_SETMASK, &origset, NULL);

	if (pid == 0) {
		/* the child */
#ifndef CONFIG_LUNA
		char *env[3];
#endif

		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
#ifndef CONFIG_LUNA
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;
#endif

#ifndef CONFIG_LUNA
		execve(filename, argv, env);
#else
		execv(filename, argv);
#endif

		fprintf(stderr, "exec %s failed\n", filename);
		_exit(EXIT_FAILURE);
	} else if (pid > 0) {
		if (!dowait)
			ret = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = waitpid(pid,&status,0)) != pid) {
				if (wpid == -1 && errno == ECHILD) {	/* see wait(2) manpage */
					break;
				}
			}

			ret = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
		}
	} else if (pid < 0) {
		fprintf(stderr, "fork of %s failed\n", filename);
		ret = -1;
	}

	return ret;
}

//return 0:OK, other:fail
int va_cmd(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[24];
	int status;

	TRACE(STA_SCRIPT, "%s ", cmd);
	va_start(ap, dowait);

	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
		TRACE(STA_SCRIPT|STA_NOTAG, "%s ", s);
	}

	TRACE(STA_SCRIPT|STA_NOTAG, "\n");
	argv[k+1] = NULL;
	status = do_cmd(cmd, argv, dowait);
	va_end(ap);

	return status;
}

//return 0:OK, other:fail
/*same function as va_cmd(). Execute silently.
  No print out command string in console.
*/
int va_cmd_no_echo(const char *cmd, int num, int dowait, ...)
{
	va_list ap;
	int k;
	char *s;
	char *argv[24];
	int status;

	va_start(ap, dowait);

	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
	}

	argv[k+1] = NULL;
	status = do_cmd(cmd, argv, dowait);
	va_end(ap);

	return status;
}

//return 0:OK, other:fail
int call_cmd(const char *filename, int num, int dowait, ...)
{
	va_list ap;
	char *s;
	char *argv[24];
	int status=0, st, k;
	pid_t pid, wpid;

	va_start(ap, dowait);

	for (k=0; k<num; k++)
	{
		s = va_arg(ap, char *);
		argv[k+1] = s;
	}

	argv[k+1] = NULL;
	if((pid = vfork()) == 0) {
		/* the child */
		char *env[3];

		signal(SIGINT, SIG_IGN);
		argv[0] = (char *)filename;
		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);

		printf("exec %s failed\n", filename);
		_exit(2);
	} else if(pid > 0) {
		if (!dowait)
			status = 0;
		else {
			/* parent, wait till rc process dies before spawning */
			while ((wpid = wait(&st)) != pid)
				if (wpid == -1 && errno == ECHILD) { /* see wait(2) manpage */
					break;
				}
		}
	} else if(pid < 0) {
		printf("fork of %s failed\n", filename);
		status = -1;
	}
	if (wpid>0)
		if (WIFEXITED(st))
			status = WEXITSTATUS(st);
	va_end(ap);

	return status;
}

void write_to_pppd(struct data_to_pass_st *pmsg)
{
#if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
	//Chuck: use system call to trigger spppd main.c pause(), fit for ql_xu performance tune fix
	system(pmsg->data);
#else
	int pppd_fifo_fd=-1;

	pppd_fifo_fd = open(PPPD_FIFO, O_WRONLY);
	if (pppd_fifo_fd == -1)
		fprintf(stderr, "Sorry, no spppd server\n");
	else
	{
		write(pppd_fifo_fd, pmsg, sizeof(*pmsg));
		close(pppd_fifo_fd);
	}
#endif
}

// return value:
// 0  : successful
// -1 : failed
int write_to_mpoad(struct data_to_pass_st *pmsg)
{
	int mpoad_fifo_fd=-1;
	int status=0;
	int ret=0;
#ifdef CONFIG_RTL8672_SAR
	mpoad_fifo_fd = open(MPOAD_FIFO, O_WRONLY);
	if (mpoad_fifo_fd == -1) {
		fprintf(stderr, "Sorry, no mpoad server\n");
		status = -1;
	} else
	{
		// Modified by Mason Yu
		//write(mpoad_fifo_fd, pmsg, sizeof(*pmsg));
REWRITE:
		ret = write(mpoad_fifo_fd, pmsg, sizeof(*pmsg));
		if(ret<0 && errno==EPIPE)
			 goto REWRITE;
#ifdef _LINUX_2_6_
		//sleep more time for mpoad to wake up
		usleep(100*1000);
#else
		usleep(30*1000);
#endif

		close(mpoad_fifo_fd);
		// wait server to consume it
#ifdef _LINUX_2_6_
		//sleep more time for mpoad to wake up
		usleep(100*1000);
#else
		usleep(1000);
#endif
	}
#endif // CONFIG_RTL8672_SAR
	return status;
}

static void WRITE_DHCPC_FILE(int fh, unsigned char *buf)
{
	if ( write(fh, buf, strlen(buf)) != strlen(buf) ) {
		printf("Write udhcpc script file error!\n");
		close(fh);
	}
}

static void write_to_dhcpc_script(char *fname, MIB_CE_ATM_VC_Tp pEntry)
{
	int fh;
	int mark;
	char buff[64];
#ifdef POLICY_ROUTING_DNSV4RELAY
	char buff2[128];
	int32_t tbl_id;
	char str_tblid[10]="";
	unsigned char Dnsmode;
	char iproutecmd2[80];	// Mason Yu. for ACS and NTP
#endif
#ifdef DEFAULT_GATEWAY_V2
	unsigned int dgw;
#endif
#ifdef IP_POLICY_ROUTING
	int i, num, found;
	MIB_CE_IP_QOS_T qEntry;
#endif
#ifdef NEW_PORTMAPPING
	char iproutecmd[80];
	int  tableId;
#endif
	char ifname[30];

	ifGetName(pEntry->ifIndex, ifname, sizeof(ifname));

	fh = open(fname, O_RDWR|O_CREAT|O_TRUNC, S_IXUSR);

	if (fh == -1) {
		printf("Create udhcpc script file %s error!\n", fname);
		return;
	}

	WRITE_DHCPC_FILE(fh, "#!/bin/sh\n");
	snprintf(buff, 64, "RESOLV_CONF=\"/var/udhcpc/resolv.conf.%s\"\n", ifname);
	WRITE_DHCPC_FILE(fh, buff);
	WRITE_DHCPC_FILE(fh, "[ \"$broadcast\" ] && BROADCAST=\"broadcast $broadcast\"\n");
	WRITE_DHCPC_FILE(fh, "[ \"$subnet\" ] && NETMASK=\"netmask $subnet\"\n");
	WRITE_DHCPC_FILE(fh, "MER_GW_INFO=\"/tmp/MERgw.\"$interface\n");	// Jenny, write MER1483 gateway info
	WRITE_DHCPC_FILE(fh, "echo $router > $MER_GW_INFO\n");
	WRITE_DHCPC_FILE(fh, "ifconfig $interface 0.0.0.0\n");
        WRITE_DHCPC_FILE(fh, "ifconfig $interface $ip $BROADCAST $NETMASK -pointopoint\n");

#ifdef DEFAULT_GATEWAY_V2
	mib_get( MIB_ADSL_WAN_DGW_ITF, (void *)&dgw);	// Jenny, check default gateway
	if ((dgw == pEntry->ifIndex)
#ifdef AUTO_PPPOE_ROUTE
		 || (dgw == DGW_AUTO && (CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_IPOE)
#endif
	)
#else
	if (pEntry->dgw)
#endif
	{
		WRITE_DHCPC_FILE(fh, "if [ \"$router\" ]; then\n");
#ifdef DEFAULT_GATEWAY_V2
		if (ifExistedDGW() == 1)	// Jenny, delete existed default gateway first
			WRITE_DHCPC_FILE(fh, "\troute del default\n");
#endif
		WRITE_DHCPC_FILE(fh, "\twhile route del -net default gw 0.0.0.0 dev $interface\n");
		WRITE_DHCPC_FILE(fh, "\tdo :\n");
		WRITE_DHCPC_FILE(fh, "\tdone\n\n");
		WRITE_DHCPC_FILE(fh, "\tfor i in $router\n");
		WRITE_DHCPC_FILE(fh, "\tdo\n");
		WRITE_DHCPC_FILE(fh, "\troute add -net default gw $i dev $interface\n");
#ifdef NEW_PORTMAPPING
		tableId = caculate_tblid(pEntry->ifIndex);
		snprintf(iproutecmd, 80, "\tip route add default dev $interface via $i table %d\n", tableId);
		WRITE_DHCPC_FILE(fh, iproutecmd);
		snprintf(iproutecmd, 80, "\tip route add default dev $interface via $i table %d\n", PMAP_DEFAULT_TBLID);
		WRITE_DHCPC_FILE(fh, iproutecmd);
#endif
		WRITE_DHCPC_FILE(fh, "\tdone\n");
		WRITE_DHCPC_FILE(fh, "fi\n");
	}
	else {
#ifdef DEFAULT_GATEWAY_V2	// Jenny, assign default gateway by remote WAN IP
		unsigned char dgwip[16];
		if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0) {
			if (dgw == DGW_NONE && getMIB2Str(MIB_ADSL_WAN_DGW_IP, dgwip) == 0) {
				if (ifExistedDGW() == 1)
					WRITE_DHCPC_FILE(fh, "\troute del default\n");
				// route add default gw remotip
				snprintf(buff, 64, "\troute add default gw %s\n", dgwip);
				WRITE_DHCPC_FILE(fh, buff);
			}
		}
#endif
#ifdef NEW_PORTMAPPING
		tableId = caculate_tblid(pEntry->ifIndex);
		WRITE_DHCPC_FILE(fh, "if [ \"$router\" ]; then\n");
		snprintf(iproutecmd, 80, "\tip route del default table %d\n", tableId);
		WRITE_DHCPC_FILE(fh, iproutecmd);
		snprintf(iproutecmd, 80, "\tip route add default dev $interface via $i table %d\n", tableId);
		WRITE_DHCPC_FILE(fh, "\tfor i in $router\n");
		WRITE_DHCPC_FILE(fh, "\tdo\n");
		WRITE_DHCPC_FILE(fh, iproutecmd);
		WRITE_DHCPC_FILE(fh, "\tdone\n");
		WRITE_DHCPC_FILE(fh, "fi\n");
#endif
	}

#ifdef CONFIG_USER_RTK_WAN_CTYPE
	if (pEntry->applicationtype&X_CT_SRV_TR069)
	{
		char ifname[10];
		char cmd[64];

		//snprintf(ifname,sizeof(ifname),"vc%d",VC_INDEX(pEntry->ifIndex));
		ifGetName(pEntry->ifIndex, ifname, sizeof(ifname));
		sprintf(buff,"%s.%s", DHCPC_ROUTERFILE, ifname);
		WRITE_DHCPC_FILE(fh, "if [ \"$router\" ]; then\n");
		WRITE_DHCPC_FILE(fh, "\tfor i in $router\n");
		WRITE_DHCPC_FILE(fh, "\tdo\n");
		unlink(buff);
		sprintf(cmd,"\techo $router > %s\n",buff);
		WRITE_DHCPC_FILE(fh, cmd);
		WRITE_DHCPC_FILE(fh, "\tdone\n");
		WRITE_DHCPC_FILE(fh, "fi\n");
	}
#endif

	/**************************** Important *****************************/
	/* Should set policy route before write resolv.conf, or there are   */
	/* still some packets may go to wrong pvc.                          */
	/*************************************** ****************************/
	set_dhcp_source_route(fh, pEntry);

	WRITE_DHCPC_FILE(fh, "if [ \"$dns\" ]; then\n");
	WRITE_DHCPC_FILE(fh, "\trm $RESOLV_CONF\n");
	WRITE_DHCPC_FILE(fh, "\tfor i in $dns\n");
	WRITE_DHCPC_FILE(fh, "\tdo\n");
	WRITE_DHCPC_FILE(fh, "\techo 'DNS=' $i\n");
	//WRITE_DHCPC_FILE(fh, "\techo nameserver $i >> $RESOLV_CONF\n");
	// echo 192.168.88.21@192.168.99.100
	snprintf(buff, 64, "\techo $i@$ip >> $RESOLV_CONF\n");
	WRITE_DHCPC_FILE(fh, buff);
	WRITE_DHCPC_FILE(fh, "\tdone\n");
	WRITE_DHCPC_FILE(fh, "fi\n");

#ifdef IP_POLICY_ROUTING
	num = mib_chain_total(MIB_IP_QOS_TBL);
	found = 0;
	// set advanced-routing rule
	for (i=0; i<num; i++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&qEntry))
			continue;
#ifdef QOS_DIFFSERV
		if (qEntry.enDiffserv == 1) // Diffserv entry
			continue;
#endif
		if (qEntry.outif == pEntry->ifIndex) {

			found = 1;
			mark = get_classification_mark(i);
			if (mark != 0) {
				WRITE_DHCPC_FILE(fh, "if [ \"$router\" ]; then\n");
				snprintf(buff, 64, "\tip ru add fwmark %x table %d\n", mark, VC_INDEX(pEntry->ifIndex)+PR_VC_START);
				WRITE_DHCPC_FILE(fh, buff);
			}
		}
	}
	if (found) {
		snprintf(buff, 64, "\tip ro add default via $router dev $interface table %d\n", VC_INDEX(pEntry->ifIndex)+PR_VC_START);
		WRITE_DHCPC_FILE(fh, buff);
		WRITE_DHCPC_FILE(fh, "fi\n");
	}
#endif

#ifdef CONFIG_CWMP_TR181_SUPPORT
	// Collect DHCP Information for Device.DHCPv4.Client.{i}.
	WRITE_DHCPC_FILE(fh, "\nINFO_FILE=\"/tmp/udhcpc_info.$interface\"\n");
	WRITE_DHCPC_FILE(fh, "rm $INFO_FILE\n");

	// IP Address
	WRITE_DHCPC_FILE(fh, "if [ \"$ip\" ]; then\n");
	WRITE_DHCPC_FILE(fh, "	echo ip=$ip >> $INFO_FILE\n");
	WRITE_DHCPC_FILE(fh, "fi\n");

	// DHCP Server
	WRITE_DHCPC_FILE(fh, "if [ \"$siaddr\" ]; then\n");
	WRITE_DHCPC_FILE(fh, "	echo siaddr=$siaddr >> $INFO_FILE\n");
	WRITE_DHCPC_FILE(fh, "fi\n");

	// Gateway info
	WRITE_DHCPC_FILE(fh, "if [ \"$router\" ]; then\n");
	WRITE_DHCPC_FILE(fh, "	echo router=$router >> $INFO_FILE\n");
	WRITE_DHCPC_FILE(fh, "fi\n");

	// DNS Servers
	WRITE_DHCPC_FILE(fh, "DNS=''\n");
	WRITE_DHCPC_FILE(fh, "if [ \"$dns\" ]; then\n");
	WRITE_DHCPC_FILE(fh, "	for i in $dns\n");
	WRITE_DHCPC_FILE(fh, "	do\n");
	WRITE_DHCPC_FILE(fh, "		DNS=\"$DNS$i,\"\n");
	WRITE_DHCPC_FILE(fh, "	done\n");
	WRITE_DHCPC_FILE(fh, "	echo dns=$DNS >> $INFO_FILE\n");
	WRITE_DHCPC_FILE(fh, "fi\n");

	// Expire time
	WRITE_DHCPC_FILE(fh, "if [ \"$expire\" ]; then\n");
	WRITE_DHCPC_FILE(fh, "	echo expire=$expire >> $INFO_FILE\n");
	WRITE_DHCPC_FILE(fh, "fi\n");
#endif
	close(fh);
}

int set_dhcp_source_route(int fh, MIB_CE_ATM_VC_Tp pEntry)
{
	char buff[64];
	char buff2[128];
	int32_t tbl_id;
	char str_tblid[10]="";
	char iproutecmd[80];	// Mason Yu. for ACS and NTP

	//This interface is not default route
	if ( pEntry->dgw== 0)
	{
		tbl_id = caculate_tblid_ITF_SourceRoute(pEntry->ifIndex);
		snprintf(str_tblid, sizeof(str_tblid), "%d", tbl_id);

		//(1) ip route flush table 64
		snprintf(buff, 64, "\tip route flush table %s\n", str_tblid);
		WRITE_DHCPC_FILE(fh, buff);

		// (2) ip rule del fwmark 1 table 200
		snprintf(buff, 64, "\tip rule del table %s\n", str_tblid);
		WRITE_DHCPC_FILE(fh, buff);

		// (3) Add default route for policy route table
		WRITE_DHCPC_FILE(fh, "if [ \"$router\" ]; then\n");
		//ip route add default dev vc0 via 192.168.1.254 table 64
		snprintf(iproutecmd, 80, "\tip route add default dev $interface via $i table %s\n", str_tblid);
		WRITE_DHCPC_FILE(fh, "\tfor i in $router\n");
		WRITE_DHCPC_FILE(fh, "\tdo\n");
		WRITE_DHCPC_FILE(fh, iproutecmd);
		WRITE_DHCPC_FILE(fh, "\tdone\n");
		WRITE_DHCPC_FILE(fh, "fi\n");

		// (4) Add net route for this policy route table
		// ip route add 192.168.8.0/24 via 192.168.8.1 table 200
		snprintf(buff2, 128, "\tip route add $netip/$mask via $ip table %s\n", str_tblid);
		WRITE_DHCPC_FILE(fh, buff2);

		// (5) Add source IP rule
		snprintf(buff2, 128, "\tip rule add from $ip table %s\n", str_tblid);
		WRITE_DHCPC_FILE(fh, buff2);
	}
	return 0;
}

int set_static_source_route(MIB_CE_ATM_VC_Tp pEntry)
{
	int32_t tbl_id;
	char str_tblid[10]="";
	char ifname[10];
	char *temp;
	char ipAddr[20], remoteIp[20];
	unsigned long wanmask, wanmbit, wanip, wansubnet;
	unsigned char wansubnetStr[20];
	char wansubnetStr_tmp[30];
	char wanipStr[20];

	ifGetName(pEntry->ifIndex, ifname, sizeof(ifname));
	// If this interface set DNS IP manually, and this interface is not default route
	if ( ((pEntry->cmode == CHANNEL_MODE_RT1483) || (pEntry->cmode == CHANNEL_MODE_IPOE)) && ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) && (pEntry->dgw== 0))
	{
		tbl_id = caculate_tblid_ITF_SourceRoute(pEntry->ifIndex);
		snprintf(str_tblid, sizeof(str_tblid), "%d", tbl_id);

		// (1) flush source route
		//( ip route flush table 64
		va_cmd("/bin/ip", 4, 1, "route", "flush", "table", str_tblid);

		// (2) Get some WAN IP address
		temp = inet_ntoa(*((struct in_addr *)pEntry->ipAddr));
		if (strcmp(temp, "0.0.0.0"))
		{
			strcpy(ipAddr, temp);
		}

		wanip = *(unsigned long *)(pEntry->ipAddr);
		wanmask = *(unsigned long *)(pEntry->netMask);
		wansubnet = (wanip&wanmask);
		//printf("***** wanip=0x%x, wanmask=0x%x, wansubnet=0x%x\n", wanip, wanmask, wansubnet);
		strncpy(wansubnetStr, inet_ntoa(*((struct in_addr *)&wansubnet)), 16);
		wansubnetStr[15] = '\0';
		strncpy(wanipStr, inet_ntoa(*((struct in_addr *)&wanip)), 16);
		wanipStr[15] = '\0';

		wanmbit=0;
		while (1) {
			if (wanmask&0x80000000) {
				wanmbit++;
				wanmask <<= 1;
			}
			else
				break;
		}
		snprintf(wansubnetStr_tmp,30,"%s/%d",wansubnetStr,wanmbit);

		// (3) Add default route for policy route table
		temp = inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr));
		if (strcmp(temp, "0.0.0.0"))
		{
			strcpy(remoteIp, temp);
			//ip route add default dev vc0 via 192.168.1.254 table 64
			va_cmd("/bin/ip", 9, 1, "route", "add", "default", "dev", ifname, "via", remoteIp, "table", str_tblid);
		}

		// (4) Add net route for this policy route table
		// ip route add 192.168.8.0/24 via 192.168.8.1 table 200
		if (pEntry->cmode != CHANNEL_MODE_RT1483) // p-to-p link do not need interface route.
			va_cmd("/bin/ip", 7, 1, "route", "add", wansubnetStr_tmp, "via", ipAddr, "table", str_tblid);

		// (5) Add source rule from this interface
		// ip rule del table 64
		// delete rule before adding it.
		va_cmd("/bin/ip", 4, 1, "rule", "del", "table", str_tblid);
		va_cmd("/bin/ip", 6, 1, "rule", "add", "from", wanipStr, "table", str_tblid);
	}
	return 0;
}

int set_ppp_source_route(struct ppp_policy_route_info *ppp_info)
{
	int32_t tbl_id;
	unsigned int ifIndex;
	char str_tblid[10]="";
	char SourceRouteIp[20]={0};
	char sourceroute_tmp[50];
	int applicationtype;

	ifIndex = getIfIndexByName(ppp_info->if_name);
	applicationtype=getapplicationtypeByName(ppp_info->if_name);
	tbl_id = caculate_tblid_ITF_SourceRoute(ifIndex);
	snprintf(str_tblid, sizeof(str_tblid), "%d", tbl_id);

	//ip route flush table 64
	va_cmd("/bin/ip", 4, 1, "route", "flush", "table", str_tblid);
	// ip rule del table 64
	va_cmd("/bin/ip", 4, 1, "rule", "del", "table", str_tblid);

	strcpy(SourceRouteIp,inet_ntoa(*((struct in_addr *)&ppp_info->myip)) );
	//snprintf(sourceroute_tmp,50,"%s/%d",SourceRouteIp,24);
	// Add source rule
	va_cmd("/bin/ip", 6, 1, "rule", "add", "from", SourceRouteIp,"table", str_tblid);
	// ip route add default dev ppp0 table 200
	va_cmd("/bin/ip", 7, 1, "route", "add", "default", "dev", ppp_info->if_name, "table", str_tblid);

	return 0;
}

// DHCP client configuration
// return value:
// 1  : successful
int startDhcpc(char *inf, MIB_CE_ATM_VC_Tp pEntry)
{
	unsigned char value[32], value2[32];
	char devName[MAX_NAME_LEN];
	FILE *fp;
	DNS_TYPE_T dnsMode;
	char * argv[16];

	unsigned int i, vcTotal, resolvopt;

	mib_get(MIB_DEVICE_NAME, (void *)devName);

	argv[1] = inf;
	argv[2] = "up";
	argv[3] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s\n", IFCONFIG, argv[1], argv[2]);
	do_cmd(IFCONFIG, argv, 1);

	// udhcpc -i vc0 -p pid -s script
	argv[1] = (char *)ARG_I;
	argv[2] = inf;
	argv[3] = "-p";
	snprintf(value2, 32, "%s.%s", (char*)DHCPC_PID, inf);
	argv[4] = (char *)value2;
	argv[5] = "-s";
	snprintf(value, 32, "%s.%s", (char *)DHCPC_SCRIPT_NAME, inf);
	write_to_dhcpc_script(value, pEntry);
	argv[6] = (char *)DHCPC_SCRIPT;
	// Add option 12 Host Name
	argv[7] = "-H";
	argv[8] = (char *)devName;
	argv[9] = NULL;

	if (strcmp(inf, LANIF) == 0)
	{
		// LAN interface
		// enable Microsoft auto IP configuration
		argv[9] = "-a";
		argv[10] = NULL;
	}

	TRACE(STA_SCRIPT, "%s %s %s %s ", DHCPC, argv[1], argv[2], argv[3]);
	TRACE(STA_SCRIPT, "%s %s %s\n", argv[4], argv[5], argv[6]);
	do_cmd(DHCPC, argv, 0);

	return 1;
}

int reWriteAllDhcpcScript()
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char wanif[IFNAMSIZ];
	unsigned char value[32];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entryNum; i++) {
		/* Retrieve entry */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			printf("reWriteAllDhcpcScript: cannot get ATM_VC_TBL(ch=%d) entry\n", i);
			return 0;
		}

		if ((DHCP_T)Entry.ipDhcp == DHCP_CLIENT)
		{
			ifGetName(PHY_INTF(Entry.ifIndex),wanif,sizeof(wanif));
			snprintf(value, 32, "%s.%s", (char *)DHCPC_SCRIPT_NAME, wanif);
			write_to_dhcpc_script(value, &Entry);
		}

	}
}

int clean_SourceRoute(MIB_CE_ATM_VC_Tp pEntry)
{
	//char buff2[128];
	int32_t tbl_id;
	char str_tblid[10]="";

	if ( ((pEntry->cmode == CHANNEL_MODE_RT1483) || (pEntry->cmode == CHANNEL_MODE_IPOE) || (pEntry->cmode == CHANNEL_MODE_PPPOE) || (pEntry->cmode == CHANNEL_MODE_PPPOA))
	    && (pEntry->dgw== 0))
	{
		tbl_id = caculate_tblid_ITF_SourceRoute(pEntry->ifIndex);
		snprintf(str_tblid, sizeof(str_tblid), "%d", tbl_id);

		//ip route flush table 64
		va_cmd("/bin/ip", 4, 1, "route", "flush", "table", str_tblid);
		// ip rule del table 64 for fwmark
		va_cmd("/bin/ip", 4, 1, "rule", "del", "table", str_tblid);
	}
	return 0;
}

int cleanAll_SourceRoute()
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entryNum; i++) {
		/* Retrieve entry */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			printf("cleanAll_SourceRoute: cannot get ATM_VC_TBL(ch=%d) entry\n", i);
			return 0;
		}

		clean_SourceRoute(&Entry);
	}
}

#define BRNF_MODULE_MAC_FILTER	(0x01 << 0)
#define BRNF_MODULE_URLBLOCK		(0x01 << 1)
/*
  * Unified Control functio for Brctl brnf on/off
  * trigger : 0-> off,  1 -> on
  * module, the module identifier, must be in format of (0x1 << x)
*/
static void brnfctrl(int trigger, int module)
{
	static unsigned int br_nf_flag = 0;
	int total = 0;
	int	orig;

	orig = br_nf_flag;
	total = ifWanNum("br");

	if (total == 0)//no WAN in bridge mode
	{
		if (br_nf_flag!=0)
			va_cmd("/bin/brctl", 2, 0, "brnf", "off");
		br_nf_flag = 0;
		return;
	}


	if (trigger == 0){
		br_nf_flag = br_nf_flag & (~module);
		if ((orig != 0)&&(br_nf_flag == 0))
			va_cmd("/bin/brctl", 2, 0, "brnf", "off");
	}
	else {
		br_nf_flag = br_nf_flag | module;
		if ((orig == 0)&& (br_nf_flag != 0))
			va_cmd("/bin/brctl", 2, 0, "brnf", "on");
	}


}

#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
static int GetIP_AddressMap(MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T *entry, ADDRESSMAP_IP_T *ip_info)
{
	unsigned char value[32];

        // Get Local Start IP
	if (((struct in_addr *)entry->lsip)->s_addr != 0)
	{
		strncpy(ip_info->lsip, inet_ntoa(*((struct in_addr *)entry->lsip)), 16);
		ip_info->lsip[15] = '\0';
	}
	else
		ip_info->lsip[0] = '\0';


	// Get Local End IP
	if (((struct in_addr *)entry->leip)->s_addr != 0)
	{
		strncpy(ip_info->leip, inet_ntoa(*((struct in_addr *)entry->leip)), 16);
		ip_info->leip[15] = '\0';
	}
	else
		ip_info->leip[0] = '\0';

	// Get Global Start IP
	if (((struct in_addr *)entry->gsip)->s_addr != 0)
	{
		strncpy(ip_info->gsip, inet_ntoa(*((struct in_addr *)entry->gsip)), 16);
		ip_info->gsip[15] = '\0';
	}
	else
		ip_info->gsip[0] = '\0';

	// Get Global End IP
	if (((struct in_addr *)entry->geip)->s_addr != 0)
	{
		strncpy(ip_info->geip, inet_ntoa(*((struct in_addr *)entry->geip)), 16);
		ip_info->geip[15] = '\0';
	}
	else
		ip_info->geip[0] = '\0';

	sprintf(ip_info->srcRange, "%s-%s", ip_info->lsip, ip_info->leip);
	sprintf(ip_info->globalRange, "%s-%s", ip_info->gsip, ip_info->geip);
//	printf( "\r\nGetIP_AddressMap %s-%s", ip_info->lsip, ip_info->leip);
//	printf( "\r\nGetIP_AddressMap %s-%s", ip_info->gsip, ip_info->geip);
	return 1;
}
#else //!MULTI_ADDRESS_MAPPING
static int GetIP_AddressMap(ADDRESSMAP_IP_T *ip_info)
{
	unsigned char value[32];

        // Get Local Start IP
        if (mib_get(MIB_LOCAL_START_IP, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(ip_info->lsip, inet_ntoa(*((struct in_addr *)value)), 16);
			ip_info->lsip[15] = '\0';
		}
		else
			ip_info->lsip[0] = '\0';
	}

	// Get Local End IP
        if (mib_get(MIB_LOCAL_END_IP, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(ip_info->leip, inet_ntoa(*((struct in_addr *)value)), 16);
			ip_info->leip[15] = '\0';
		}
		else
			ip_info->leip[0] = '\0';
	}

	// Get Global Start IP
	if (mib_get(MIB_GLOBAL_START_IP, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(ip_info->gsip, inet_ntoa(*((struct in_addr *)value)), 16);
			ip_info->gsip[15] = '\0';
		}
		else
			ip_info->gsip[0] = '\0';
	}

	// Get Global End IP
	if (mib_get(MIB_GLOBAL_END_IP, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(ip_info->geip, inet_ntoa(*((struct in_addr *)value)), 16);
			ip_info->geip[15] = '\0';
		}
		else
			ip_info->geip[0] = '\0';
	}

	sprintf(ip_info->srcRange, "%s-%s", ip_info->lsip, ip_info->leip);
	sprintf(ip_info->globalRange, "%s-%s", ip_info->gsip, ip_info->geip);

	return 1;
}
#endif //end of !MULTI_ADDRESS_MAPPING
#endif

#ifdef CONFIG_HWNAT
static int send_extip_to_hwnat(char action, char *name, unsigned int ip)
{
	struct _hwnat_ioctl_cmd ioctl_arg;
	struct hwnat_ioctl_extip_cmd cmd_arg;

	cmd_arg.extip_rule.name = name;
	cmd_arg.extip_rule.action = action;
	cmd_arg.extip_rule.ip = ip;

	ioctl_arg.type = HWNAT_IOCTL_EXTIP_TYPE;
	ioctl_arg.data = &cmd_arg;

	send_to_hwnat(ioctl_arg);
	return 0;
}
#endif

// Setup one NAT rule for a specfic interface
static int startAddressMap(MIB_CE_ATM_VC_Tp pEntry)
{
	char wanif[IFNAMSIZ];
	char myip[16];
#ifdef ADDRESS_MAPPING
	ADDRESSMAP_IP_T ip_info;
#ifdef MULTI_ADDRESS_MAPPING
	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T		entry;
	int		i,entryNum;


#else // ! MULTI_ADDRESS_MAPPING
	char vChar;
	ADSMAP_T mapType;

	GetIP_AddressMap(&ip_info);

	mib_get( MIB_ADDRESS_MAP_TYPE,  (void *)&vChar);
        mapType = (ADSMAP_T)vChar;
#endif  //end of !MULTI_ADDRESS_MAPPING
#endif

	if ( !pEntry->enable || ((DHCP_T)pEntry->ipDhcp != DHCP_DISABLED)
		|| ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_BRIDGE)
		|| ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOE)
		|| ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOA)
		|| (!pEntry->napt)
		)
		return -1;

	//snprintf(wanif, 6, "vc%u", VC_INDEX(pEntry->ifIndex));
	ifGetName( PHY_INTF(pEntry->ifIndex), wanif, sizeof(wanif));
	strncpy(myip, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), 16);
	myip[15] = '\0';

#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
	entryNum = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);

	for (i = 0; i<entryNum; i++)
	{
		mib_chain_get(MULTI_ADDRESS_MAPPING_LIMIT_TBL, i, &entry);
		GetIP_AddressMap(&entry, &ip_info);

		// add customized mapping
		if ( entry.addressMapType == ADSMAP_ONE_TO_ONE ) {
			va_cmd(IPTABLES, 12, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-s", ip_info.lsip,
				ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.gsip);

		}
		else if ( entry.addressMapType == ADSMAP_MANY_TO_ONE ) {
			va_cmd(IPTABLES, 14, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-m", "iprange", "--src-range", ip_info.srcRange,
				ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.gsip);

		}
		else if ( entry.addressMapType == ADSMAP_MANY_TO_MANY ) {
			va_cmd(IPTABLES, 14, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-m", "iprange", "--src-range", ip_info.srcRange,
				ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.globalRange);

		}
		else if ( entry.addressMapType == ADSMAP_ONE_TO_MANY ) {
			va_cmd(IPTABLES, 12, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-s", ip_info.lsip,
				ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.globalRange);
		}

	}
#else //!MULTI_ADDRESS_MAPPING
	// add customized mapping
	if ( mapType == ADSMAP_ONE_TO_ONE ) {
		va_cmd(IPTABLES, 12, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-s", ip_info.lsip,
			ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.gsip);

	}
	else if ( mapType == ADSMAP_MANY_TO_ONE ) {
		va_cmd(IPTABLES, 14, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-m", "iprange", "--src-range", ip_info.srcRange,
			ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.gsip);

	}
	else if ( mapType == ADSMAP_MANY_TO_MANY ) {
		va_cmd(IPTABLES, 14, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-m", "iprange", "--src-range", ip_info.srcRange,
			ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.globalRange);

	}
	// Mason Yu on True
#if 1
	else if ( mapType == ADSMAP_ONE_TO_MANY ) {
		va_cmd(IPTABLES, 12, 1, "-t", "nat", FW_ADD, "POSTROUTING", "-s", ip_info.lsip,
			ARG_O, wanif, "-j", "SNAT", "--to-source", ip_info.globalRange);

	}
#endif
#endif //end of !MULTI_ADDRESS_MAPPING
#endif
	// add default mapping
	va_cmd(IPTABLES, 10, 1, "-t", "nat", FW_ADD, "POSTROUTING",
		ARG_O, wanif, "-j", "SNAT", "--to-source", myip);
}

// Delete one NAT rule for a specfic interface
static void stopAddressMap(MIB_CE_ATM_VC_Tp pEntry)
{
	char *argv[16];
	char wanif[IFNAMSIZ];
	char myip[16];

#ifdef ADDRESS_MAPPING
	ADDRESSMAP_IP_T ip_info;
#ifdef MULTI_ADDRESS_MAPPING
	MIB_CE_MULTI_ADDR_MAPPING_LIMIT_T		entry;
	int		i,entryNum;


#else //MULTI_ADDRESS_MAPPING
	char vChar;
	ADSMAP_T mapType;

	GetIP_AddressMap(&ip_info);

	mib_get( MIB_ADDRESS_MAP_TYPE,  (void *)&vChar);
        mapType = (ADSMAP_T)vChar;
#endif //!MULTI_ADDRESS_MAPPING
#endif

	if ( !pEntry->enable || ((DHCP_T)pEntry->ipDhcp != DHCP_DISABLED)
		|| ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_BRIDGE)
		|| ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOE)
		|| ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOA)
		|| (!pEntry->napt)
		)
		return;

	//snprintf(wanif, 6, "vc%u", VC_INDEX(pEntry->ifIndex));
	ifGetName( PHY_INTF(pEntry->ifIndex), wanif, sizeof(wanif));
	strncpy(myip, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), 16);
	myip[15] = '\0';

	if (pEntry->cmode != CHANNEL_MODE_BRIDGE && pEntry->napt == 1)
	{	// Enable NAPT
		argv[1] = "-t";
		argv[2] = "nat";
		argv[3] = (char *)FW_DEL;
		argv[4] = "POSTROUTING";

		// remove default mapping
		argv[5] = "-o";
		argv[6] = wanif;
		argv[7] = "-j";
		if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
			strncpy(myip, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), 16);
			myip[15] = '\0';
			argv[8] = "SNAT";
			argv[9] = "--to-source";
			argv[10] = myip;
			argv[11] = NULL;
		}
		else {
			argv[8] = "MASQUERADE";
			argv[9] = NULL;
		}
		do_cmd(IPTABLES, argv, 1);

#ifdef ADDRESS_MAPPING
#ifdef MULTI_ADDRESS_MAPPING
		entryNum = mib_chain_total(MULTI_ADDRESS_MAPPING_LIMIT_TBL);

		for (i = 0; i<entryNum; i++)
		{
			mib_chain_get(MULTI_ADDRESS_MAPPING_LIMIT_TBL, i, &entry);
			GetIP_AddressMap(&entry, &ip_info);

			// remove customized mapping
			if ( entry.addressMapType  == ADSMAP_ONE_TO_ONE ) {
				argv[5] = "-s";
				argv[6] = ip_info.lsip;
				argv[7] = "-o";
				argv[8] = wanif;
				argv[9] = "-j";
				if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
					argv[10] = "SNAT";
					argv[11] = "--to-source";
					argv[12] = ip_info.gsip;
					argv[13] = NULL;
				}
				else {
					argv[8] = "MASQUERADE";
					argv[9] = NULL;
				}

			} else if ( entry.addressMapType  == ADSMAP_MANY_TO_ONE ) {

				argv[5] = "-m";
				argv[6] = "iprange";
				argv[7] = "--src-range";
				argv[8] = ip_info.srcRange;
				argv[9] = "-o";
				argv[10] = wanif;
				argv[11] = "-j";

				if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
					argv[12] = "SNAT";
					argv[13] = "--to-source";
					argv[14] = ip_info.gsip;
					argv[15] = NULL;
				}
				else {
					argv[8] = "MASQUERADE";
					argv[9] = NULL;
				}

			} else if ( entry.addressMapType  == ADSMAP_MANY_TO_MANY ) {
				argv[5] = "-m";
				argv[6] = "iprange";
				argv[7] = "--src-range";
				argv[8] = ip_info.srcRange;
				argv[9] = "-o";
				argv[10] = wanif;
				argv[11] = "-j";
				if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
					argv[12] = "SNAT";
					argv[13] = "--to-source";
					argv[14] = ip_info.globalRange;
					argv[15] = NULL;
				}
				else {
					argv[8] = "MASQUERADE";
					argv[9] = NULL;
				}

			}

			else if ( entry.addressMapType  == ADSMAP_ONE_TO_MANY ) {
				argv[5] = "-s";
				argv[6] = ip_info.lsip;
				argv[7] = "-o";
				argv[8] = wanif;
				argv[9] = "-j";
				if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
					argv[10] = "SNAT";
					argv[11] = "--to-source";
					argv[12] = ip_info.globalRange;
					argv[13] = NULL;
				}
				else {
					argv[8] = "MASQUERADE";
					argv[9] = NULL;
				}
			}

			else
				return;
			TRACE(STA_SCRIPT, "%s %s %s %s %s ", IPTABLES, argv[1], argv[2], argv[3], argv[4]);
			TRACE(STA_SCRIPT, "%s %s %s %s\n", argv[5], argv[6], argv[7], argv[8]);
			do_cmd(IPTABLES, argv, 1);
		}
#else //!MULTI_ADDRESS_MAPPING
		// remove customized mapping
		if ( mapType == ADSMAP_ONE_TO_ONE ) {
			argv[5] = "-s";
			argv[6] = ip_info.lsip;
			argv[7] = "-o";
			argv[8] = wanif;
			argv[9] = "-j";
			if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
				argv[10] = "SNAT";
				argv[11] = "--to-source";
				argv[12] = ip_info.gsip;
				argv[13] = NULL;
			}
			else {
				argv[8] = "MASQUERADE";
				argv[9] = NULL;
			}

		} else if ( mapType == ADSMAP_MANY_TO_ONE ) {
			argv[5] = "-m";
			argv[6] = "iprange";
			argv[7] = "--src-range";
			argv[8] = ip_info.srcRange;
			argv[9] = "-o";
			argv[10] = wanif;
			argv[11] = "-j";
			if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
				argv[12] = "SNAT";
				argv[13] = "--to-source";
				argv[14] = ip_info.gsip;
				argv[15] = NULL;
			}
			else {
				argv[8] = "MASQUERADE";
				argv[9] = NULL;
			}

		} else if ( mapType == ADSMAP_MANY_TO_MANY ) {
			argv[5] = "-m";
			argv[6] = "iprange";
			argv[7] = "--src-range";
			argv[8] = ip_info.srcRange;
			argv[9] = "-o";
			argv[10] = wanif;
			argv[11] = "-j";
			if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
				argv[12] = "SNAT";
				argv[13] = "--to-source";
				argv[14] = ip_info.globalRange;
				argv[15] = NULL;
			}
			else {
				argv[8] = "MASQUERADE";
				argv[9] = NULL;
			}

		}

		// Msason Yu on True
#if 1
		else if ( mapType == ADSMAP_ONE_TO_MANY ) {
			argv[5] = "-s";
			argv[6] = ip_info.lsip;
			argv[7] = "-o";
			argv[8] = wanif;
			argv[9] = "-j";
			if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
				argv[10] = "SNAT";
				argv[11] = "--to-source";
				argv[12] = ip_info.globalRange;
				argv[13] = NULL;
			}
			else {
				argv[8] = "MASQUERADE";
				argv[9] = NULL;
			}
		}
#endif
		else
			return;
		TRACE(STA_SCRIPT, "%s %s %s %s %s ", IPTABLES, argv[1], argv[2], argv[3], argv[4]);
		TRACE(STA_SCRIPT, "%s %s %s %s\n", argv[5], argv[6], argv[7], argv[8]);
		do_cmd(IPTABLES, argv, 1);

#endif //end of !MULTI_ADDRESS_MAPPING
#endif // of ADDRESS_MAPPING
	}

}

// Config all NAT rules.
// If action= ACT_STOP, delete all NAT rules.
// If action= ACT_START, setup all NAT rules.
void config_AddressMap(int action)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char wanif[6];
	char myip[16];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entryNum; i++) {
		/* Retrieve entry */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			printf("restartAddressMap: cannot get ATM_VC_TBL(ch=%d) entry\n", i);
			return;
		}

		if (Entry.enable == 0)
			continue;
		if ( action == ACT_STOP )
			stopAddressMap(&Entry);
		else if ( action == ACT_START) {
			startAddressMap(&Entry);
		}
	}

	if ( action == ACT_START) {
		va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
	}

}

static int startIP_v4(char *inf, MIB_CE_ATM_VC_Tp pEntry, CHANNEL_MODE_T ipEncap)
{
	char myip[16], remoteip[16], netmask[16];
#ifdef IP_PASSTHROUGH
	unsigned int ippt_itf;
	int ippt=0;
	struct in_addr net;
	char netip[16];
#endif
#ifdef DEFAULT_GATEWAY_V2
	unsigned int dgw;
	int isdgw = 0;
#endif
	FILE *fp;
	// Mason Yu
	unsigned long broadcastIpAddr;
	char broadcast[16];

#ifdef IP_PASSTHROUGH
	// check IP passthrough
	if (mib_get(MIB_IPPT_ITF, (void *)&ippt_itf) != 0)
	{
		if (ippt_itf == pEntry->ifIndex)
			ippt = 1; // this interface enable the IP passthrough
	}
#endif

#ifdef DEFAULT_GATEWAY_V2
	// Jenny, check default gateway
	if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0)
	{
		if (dgw == pEntry->ifIndex)
			isdgw = 1; // this interface is default gateway
	}
#endif

	if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED)
	{
		if (!get_net_link_status(inf))
			return 0;
		// ifconfig vc0 ipaddr
		strncpy(myip, inet_ntoa(*((struct in_addr *)pEntry->ipAddr)), 16);
		myip[15] = '\0';
		strncpy(remoteip, inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr)), 16);
		remoteip[15] = '\0';
		strncpy(netmask, inet_ntoa(*((struct in_addr *)pEntry->netMask)), 16);
		netmask[15] = '\0';
		// Mason Yu
		broadcastIpAddr = ((struct in_addr *)pEntry->ipAddr)->s_addr | ~(((struct in_addr *)pEntry->netMask)->s_addr);
		strncpy(broadcast, inet_ntoa(*((struct in_addr *)&broadcastIpAddr)), 16);
		broadcast[15] = '\0';
#ifdef CONFIG_ATM_CLIP
		if (ipEncap == CHANNEL_MODE_RT1483 || ipEncap == CHANNEL_MODE_RT1577)
#else
		if (ipEncap == CHANNEL_MODE_RT1483)
#endif
		{
#ifdef IP_PASSTHROUGH
			if (!ippt)
			{
#endif
				if (pEntry->ipunnumbered)	// Jenny, for IP unnumbered determination temporarily
					va_cmd(IFCONFIG, 8, 1, inf, "10.0.0.1", "-arp",
						"-broadcast", "pointopoint", "10.0.0.2",
						"netmask", ARG_255x4);
				else
					// ifconfig vc0 myip -arp -broadcast pointopoint
					//   remoteip netmask 255.255.255.255
					va_cmd(IFCONFIG, 8, 1, inf, myip, "-arp",
						"-broadcast", "pointopoint", remoteip,
//						"netmask", netmask);	// Jenny, subnet mask added
						"netmask", ARG_255x4);
#ifdef IP_PASSTHROUGH
			}
			else	// IP passthrough
			{
				// ifconfig vc0 10.0.0.1 -arp -broadcast pointopoint
				//   10.0.0.2 netmask 255.255.255.255
				va_cmd(IFCONFIG, 8, 1, inf, "10.0.0.1", "-arp",
					"-broadcast", "pointopoint", "10.0.0.2",				// Jenny, for IP passthrough determination temporarily
					"netmask", netmask);
//				va_cmd(IFCONFIG, 8, 1, inf, "10.0.0.1", "-arp",
//					"-broadcast", "pointopoint", "10.0.0.2",
//					"netmask", ARG_255x4);
				// ifconfig br0:1 remoteip
				va_cmd(IFCONFIG, 2, 1, LAN_IPPT, remoteip);

				// Mason Yu. Add route for Public IP
               			net.s_addr = (((struct in_addr *)pEntry->remoteIpAddr)->s_addr) & (((struct in_addr *)pEntry->netMask)->s_addr);
               			strncpy(netip, inet_ntoa(net), 16);
				netip[15] = '\0';
				va_cmd(ROUTE, 7, 1, ARG_DEL, "-net", netip, "netmask", netmask, "dev", LANIF);
				va_cmd(ROUTE, 5, 1, ARG_ADD, "-host", myip, "dev", LANIF);

				// write ip to file for DHCP server
				if (fp = fopen(IPOA_IPINFO, "w"))
				{
					fwrite( pEntry->ipAddr, 4, 1, fp);
					fwrite( pEntry->remoteIpAddr, 4, 1, fp);
					fwrite( pEntry->netMask, 4, 1, fp);
					fclose(fp);
				}
			}
#endif
#ifdef DEFAULT_GATEWAY_V2
			if (isdgw)
#else
			if (pEntry->dgw)
#endif
			{
#ifdef DEFAULT_GATEWAY_V2
				if (ifExistedDGW() == 1)	// Jenny, delete existed default gateway first
					va_cmd(ROUTE, 2, 1, ARG_DEL, "default");
#endif
				// route add default vc0
				va_cmd(ROUTE, 3, 1, ARG_ADD, "default", inf);
				//va_cmd(ROUTE, 4, 1, ARG_ADD, "default", "gw", remoteip);
			}
		}
		else
		{
			// Mason Yu. Set netmask and broadcast
			// ifconfig vc0 myip
			//va_cmd(IFCONFIG, 2, 1, inf, myip);

			va_cmd(IFCONFIG, 6, 1, inf, myip, "netmask", netmask, "broadcast",  broadcast);

#ifdef DEFAULT_GATEWAY_V2
			if (isdgw)
#else
			if (pEntry->dgw)
#endif
			{
#ifdef DEFAULT_GATEWAY_V2
					if (ifExistedDGW() == 1)	// Jenny, delete existed default gateway first
						va_cmd(ROUTE, 2, 1, ARG_DEL, "default");
#endif
				// route add default gw remotip
				va_cmd(ROUTE, 4, 1, ARG_ADD, "default", "gw", remoteip);
			}
			if (ipEncap == CHANNEL_MODE_IPOE) {
				unsigned char value[32];
				snprintf(value, 32, "%s.%s", (char *)MER_GWINFO, inf);
				if (fp = fopen(value, "w"))
				{
					fprintf(fp, "%s\n", remoteip);
					fclose(fp);
				}
			}
		}
	}
	else
	{
		int dhcpc_pid;
		unsigned char value[32];
		// Enabling support for a dynamically assigned IP (ISP DHCP)...
		va_cmd(IPTABLES, 16, 1, FW_ADD, FW_INPUT, ARG_I, inf, "-p",
			ARG_UDP, FW_DPORT, "69", "-d", ARG_255x4, "-m",
			"state", "--state", "NEW", "-j", FW_ACCEPT);

		if (fp = fopen(PROC_DYNADDR, "w"))
		{
			fprintf(fp, "1\n");
			fclose(fp);
		}
		else
		{
			printf("Open file %s failed !\n", PROC_DYNADDR);
		}

		snprintf(value, 32, "%s.%s", (char*)DHCPC_PID, inf);
		dhcpc_pid = read_pid((char*)value);
		if (dhcpc_pid > 0)
			kill(dhcpc_pid, SIGTERM);
		if (startDhcpc(inf, pEntry) == -1)
		{
			printf("start DHCP client failed !\n");
		}
	}

#ifdef CONFIG_HWNAT
	if (pEntry->napt == 1){
		if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED){
			send_extip_to_hwnat(ADD_EXTIP_CMD, inf, *(uint32 *)pEntry->ipAddr);
		}
		else{
			send_extip_to_hwnat(ADD_EXTIP_CMD, inf, 0);
		}
	}
#endif

#ifdef ROUTING
	// When interface IP reset, the static route will also be reseted.
	deleteStaticRoute();
	addStaticRoute();
#endif
	set_static_source_route(pEntry);

	return 1;
}
// IP interface: 1483-r or MER
// return value:
// 1  : successful
int startIP(char *inf, MIB_CE_ATM_VC_Tp pEntry, CHANNEL_MODE_T ipEncap)
{
	unsigned char buffer[7];

	// Set MTU for 1483-r or MER
	sprintf(buffer, "%u", pEntry->mtu);
	va_cmd(IFCONFIG,3,1,inf, "mtu", buffer);

#ifdef CONFIG_IPV6
	if (pEntry->IpProtocol & IPVER_IPV4) {
#endif
		startIP_v4(inf, pEntry, ipEncap);
#ifdef CONFIG_IPV6
	}
	if (pEntry->IpProtocol & IPVER_IPV6)
		startIP_for_V6(pEntry);
#endif
	return 1;
}

#ifdef CONFIG_NO_REDIAL
void startReconnect()
{
	unsigned char vChar;
	struct data_to_pass_st msg;
#ifdef _PRMT_X_CT_COM_RECON_
	mib_get( CWMP_CT_RECON_ENABLE, (void *)&vChar);
	if(vChar==1){
		snprintf(msg.data, BUF_SIZE, "spppctl noredial 0");
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
	}else{
		snprintf(msg.data, BUF_SIZE, "spppctl noredial 1");
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
	}
#endif
}
#endif //CONFIG_NO_REDIAL

// Jenny, stop PPP
void stopPPP(void)
{
	int i;
	char tp[10];
	struct data_to_pass_st msg;
#ifdef CONFIG_USER_PPPOMODEM
	for (i=0; i<(MAX_PPP_NUM+MAX_MODEM_PPPNUM); i++)
#else
	for (i=0; i<MAX_PPP_NUM; i++)
#endif //CONFIG_USER_PPPOMODEM
	{
		sprintf(tp, "ppp%d", i);
		if (find_ppp_from_conf(tp)) {
			snprintf(msg.data, BUF_SIZE, "spppctl del %u", i);
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
	}
#ifdef CONFIG_USER_PPPOE_PROXY
	for (i=9; i<=13; i++)  //maximum pppoe session count is 5
	{
		sprintf(tp, "ppp%d", i);
		if (find_ppp_from_conf(tp)) {
			snprintf(msg.data, BUF_SIZE, "spppctl delproxyunit %d", i);
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
	}
#endif
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	for (i=9; i<=10; i++)
	{
		sprintf(tp, "ppp%d", i);
		if (find_ppp_from_conf(tp)) {
			snprintf(msg.data, BUF_SIZE, "spppctl del %d pptp 0", i);
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
	}
#endif
#ifdef CONFIG_USER_L2TPD_L2TPD
	for (i=11; i<=12; i++)
	{
		sprintf(tp, "ppp%d", i);
		if (find_ppp_from_conf(tp)) {
			snprintf(msg.data, BUF_SIZE, "spppctl del %u l2tp 0", i);
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
	}
#endif
}

// PPP connection
// Input: inf == "vc0","vc1", ...
// pppEncap: 0 : PPPoE, 1 : PPPoA
// return value:
// 1  : successful
int startPPP(char *inf, MIB_CE_ATM_VC_Tp pEntry, char *qos, CHANNEL_MODE_T pppEncap)
{
	char ifIdx[3], pppif[6], stimeout[7];
#ifdef IP_PASSTHROUGH
	unsigned int ippt_itf;
	int ippt=0;
#endif
	struct data_to_pass_st msg;
#ifdef DEFAULT_GATEWAY_V2
	unsigned int dgw;
	int isdgw = 0;
#endif
	int lastArg, pppinf, pppdbg = 0;

#ifdef IP_PASSTHROUGH
	// check IP passthrough
	if (mib_get(MIB_IPPT_ITF, (void *)&ippt_itf) != 0)
	{
		if (ippt_itf == pEntry->ifIndex)
			ippt = 1; // this interface enable the IP passthrough
	}
#endif

#ifdef DEFAULT_GATEWAY_V2
	// Jenny, check default gateway
	if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0) {
#ifdef AUTO_PPPOE_ROUTE
		if (DGW_AUTO == dgw)
			isdgw = 1;
#endif
		if (dgw == pEntry->ifIndex)
			isdgw = 1; // this interface is default gateway
	}
#endif

	snprintf(ifIdx, 3, "%u", PPP_INDEX(pEntry->ifIndex));
	snprintf(pppif, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
	if (pppEncap == CHANNEL_MODE_PPPOE)	// PPPoE
	{
		// Kaohj -- set pppoe txqueuelen to 0 to make it no queue as the queueing is
		//		just in the underlying vc interface
		// ifconfig ppp0 txqueuelen 0
		// tc qdisc replace dev ppp0 root pfifo
		// tc qdisc del dev ppp0 root
		va_cmd(IFCONFIG, 3, 1, pppif, "txqueuelen", "0");
		// workaround to remove default qdisc if any.
		va_cmd(TC, 6, 1, "qdisc", "replace", "dev", pppif, "root", "pfifo");
		va_cmd(TC, 5, 1, "qdisc", (char *)ARG_DEL, "dev", pppif, "root");
//#ifdef CONFIG_USER_PPPOE_PROXY
#if 0
              printf("enable pppoe proxy %d \n",pEntry->PPPoEProxyEnable);
              printf("maxuser = %d ...",pEntry->PPPoEProxyMaxUser);
             if(pEntry->PPPoEProxyEnable)
	    {
                FILE  *fp_pap;
                pppoe_proxy pp_proxy;
	         if ((fp_pap = fopen(PPPD_PAPFILE, "a+")) == NULL)
		  {
			printf("Open file %s failed !\n", PPPD_PAPFILE);
			return -1;
		  }
		  fprintf(fp_pap, "%s * \"%s\"  *\n", pEntry->pppUsername, pEntry->pppPassword);
		  fclose(fp_pap);

		  if(!has_pppoe_init){
				pp_proxy.cmd =PPPOE_PROXY_ENABLE;
				ppp_proxy_ioctl(&pp_proxy,SIOCPPPOEPROXY);
				has_pppoe_init =1;
		   }
		    pp_proxy.wan_unit =PPP_INDEX(pEntry->ifIndex) ;
		    pp_proxy.cmd =PPPOE_WAN_UNIT_SET;
		    strcpy(pp_proxy.user,pEntry->pppUsername);
		    strcpy(pp_proxy.passwd,pEntry->pppPassword);
		    pp_proxy.maxShareNum = pEntry->PPPoEProxyMaxUser;
		    ppp_proxy_ioctl(&pp_proxy,SIOCPPPOEPROXY);

         	    if (pEntry->napt == 1)
        	   {	// Enable NAPT
		      va_cmd(IPTABLES, 8, 1, "-t", "nat", FW_ADD, "POSTROUTING",
			 "-o", pppif, "-j", "MASQUERADE");
        	    }


		   	return 1;
             }
#endif

		if (pEntry->pppCtype != MANUAL){	// Jenny
			// spppctl add 0 pppoe vc0 username USER password PASSWORD
			//         gw 1 mru xxxx acname xxx
			snprintf(msg.data, BUF_SIZE,
				"spppctl add %s pppoe %s username %s password %s"
				" gw %d mru %d", ifIdx,
				inf, pEntry->pppUsername, pEntry->pppPassword,
#ifdef DEFAULT_GATEWAY_V2
				isdgw, pEntry->mtu);
#else
				pEntry->dgw, pEntry->mtu);
#endif
			if (strlen(pEntry->pppACName))
				snprintf(msg.data, BUF_SIZE, "%s acname %s", msg.data, pEntry->pppACName);
		}
		else {
			snprintf(msg.data, BUF_SIZE,
				"spppctl new %s pppoe %s username %s password %s"
				" gw %d mru %d", ifIdx,
				inf, pEntry->pppUsername, pEntry->pppPassword,
#ifdef DEFAULT_GATEWAY_V2
				isdgw, pEntry->mtu);
#else
				pEntry->dgw, pEntry->mtu);
#endif
			if (strlen(pEntry->pppACName))
				snprintf(msg.data, BUF_SIZE, "%s acname %s", msg.data, pEntry->pppACName);
		}
#ifdef CONFIG_SPPPD_STATICIP
		// Jenny, set PPPoE static IP
		if (pEntry->pppIp) {
			unsigned long addr;
			addr = *((unsigned long *)pEntry->ipAddr);
			if (addr)
				snprintf(msg.data, BUF_SIZE, "%s staticip %x", msg.data, addr);
		}
#endif
#ifdef _CWMP_MIB_
		// Set Service Name
		if (strlen(pEntry->pppServiceName))
			snprintf(msg.data, BUF_SIZE, "%s servicename %s", msg.data, pEntry->pppServiceName);
#endif
#ifdef CONFIG_USER_PPPOE_PROXY
		snprintf(msg.data, BUF_SIZE, "%s proxy %d maxuser %d itfgroup %d", msg.data, pEntry->PPPoEProxyEnable, pEntry->PPPoEProxyMaxUser, pEntry->itfGroup);
#ifdef CONFIG_RTK_RG_INIT //Add acl rule for PPPoE proxy usage
		if(pEntry->PPPoEProxyEnable){
			printf("Add acl rule for PPPoE proxy usage.\n");
			RTK_RG_PPPoEProxy_ACL_Rule_Set();
			if(pEntry->itfGroup > 0){
				system("echo 1 > /proc/rg/pppoe_proxy_only_for_binding_packet");
				printf("echo 1 > /proc/rg/pppoe_proxy_only_for_binding_packet\n");
			}else{
				system("echo 0 > /proc/rg/pppoe_proxy_only_for_binding_packet");
				printf("echo 0 > /proc/rg/pppoe_proxy_only_for_binding_packet\n");
			}
		}
#endif
#endif
	}
	#ifdef CONFIG_RTL8672_SAR
	else	// PPPoA
	{
		if (pEntry->pppCtype != MANUAL)	// Jenny
			// spppctl add 0 pppoa vpi.vci encaps ENCAP qos xxx
			//         username USER password PASSWORD gw 1 mru xxxx
			snprintf(msg.data, BUF_SIZE,
				"spppctl add %s pppoa %u.%u encaps %d qos %s "
				"username %s password %s gw %d mru %d",
				ifIdx, pEntry->vpi, pEntry->vci, pEntry->encap,	qos,
#ifdef DEFAULT_GATEWAY_V2
				pEntry->pppUsername, pEntry->pppPassword, isdgw, pEntry->mtu);
#else
				pEntry->pppUsername, pEntry->pppPassword, pEntry->dgw, pEntry->mtu);
#endif
		else
			snprintf(msg.data, BUF_SIZE,
				"spppctl new %s pppoa %u.%u encaps %d qos %s "
				"username %s password %s gw %d mru %d",
				ifIdx, pEntry->vpi, pEntry->vci, pEntry->encap,	qos,
#ifdef DEFAULT_GATEWAY_V2
				pEntry->pppUsername, pEntry->pppPassword, isdgw, pEntry->mtu);
#else
				pEntry->pppUsername, pEntry->pppPassword, pEntry->dgw, pEntry->mtu);
#endif
	}
	#endif // CONFIG_RTL8672_SAR

	// Set Authentication Method
	if ((PPP_AUTH_T)pEntry->pppAuth >= PPP_AUTH_PAP && (PPP_AUTH_T)pEntry->pppAuth <= PPP_AUTH_CHAP)
		snprintf(msg.data, BUF_SIZE, "%s auth %s", msg.data, ppp_auth[pEntry->pppAuth]);

#ifdef IP_PASSTHROUGH
	// Set IP passthrough
	snprintf(msg.data, BUF_SIZE, "%s ippt %d", msg.data, ippt);
#endif

	// set PPP debug
	pppdbg = pppdbg_get(PPP_INDEX(pEntry->ifIndex));
	snprintf(msg.data, BUF_SIZE, "%s debug %d", msg.data, pppdbg);

#ifdef _CWMP_MIB_
	// Set Auto Disconnect Timer
	if (pEntry->autoDisTime > 0)
		snprintf(msg.data, BUF_SIZE, "%s disctimer %d", msg.data, pEntry->autoDisTime);
	// Set Warn Disconnect Delay
	if (pEntry->warnDisDelay > 0)
		snprintf(msg.data, BUF_SIZE, "%s discdelay %d", msg.data, pEntry->warnDisDelay);
#endif

#ifdef CONFIG_IPV6
	snprintf(msg.data, BUF_SIZE, "%s ipt %u", msg.data, pEntry->IpProtocol - 1);
#endif

	if (pEntry->pppCtype == CONTINUOUS)	// Continuous
	{
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
		// set the ppp keepalive timeout
		snprintf(msg.data, BUF_SIZE,
			"spppctl katimer 100");
		TRACE(STA_SCRIPT, "%s\n", msg.data);

		write_to_pppd(&msg);
		printf("PPP Connection (Continuous)...\n");
	}
	else if (pEntry->pppCtype == CONNECT_ON_DEMAND)	// On-demand
	{
		snprintf(msg.data, BUF_SIZE, "%s timeout %u", msg.data, pEntry->pppIdleTime);
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
		printf("PPP Connection (On-demand)...\n");
	}
	else if (pEntry->pppCtype == MANUAL)	// Manual
	{
		// Jenny, for PPP connecting/disconnecting manually
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
		printf("PPP Connection (Manual)...\n");
	}

	if (pEntry->napt == 1)
	{
#ifdef CONFIG_HWNAT
		/*QL: pppoe interface should do HW SNAT as well as mer interface*/
		send_extip_to_hwnat(ADD_EXTIP_CMD, pppif, 0);
#endif
	}

#ifdef CONFIG_NO_REDIAL
	startReconnect();
#endif
	return 1;
}

// find if pppif exists in /var/ppp/ppp.conf
int find_ppp_from_conf(char *pppif)
{
	char buff[256];
	FILE *fp;
	char strif[6];
	int found=0;

	if (!(fp=fopen(PPP_CONF, "r"))) {
		printf("%s not exists.\n", PPP_CONF);
	}
	else {
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL ) {
			if(sscanf(buff, "%s", strif)!=1) {
				found=0;
				printf("Unsuported ppp configuration format\n");
				break;
			}

			if ( !strcmp(pppif, strif) ) {
				found = 1;
				break;
			}
		}
		fclose(fp);
	}
	return found;
}

#ifdef CONFIG_RTL8672_SAR
// find if vc exists in /proc/net/atm/pvc
static int find_pvc_from_running(int vpi, int vci)
{
	char buff[256];
	FILE *fp;
	int tvpi, tvci;
	int found=0;

	if (!(fp=fopen(PROC_NET_ATM_PVC, "r"))) {
		fclose(fp);
		printf("%s not exists.\n", PROC_NET_ATM_PVC);
	}
	else {
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL ) {
			if(sscanf(buff, "%*d%d%d", &tvpi, &tvci)!=2) {
				found=0;
				printf("Unsuported pvc configuration format\n");
				break;
			}
			if ( tvpi==vpi && tvci==vci ) {
				found = 1;
				break;
			}
		}
		fclose(fp);
	}
	return found;
}
#endif//CONFIG_RTL8672_SAR

// calculate the 15-0(bits) Cell Rate register value (PCR or SCR)
// return its corresponding register value
int cr2reg(int pcr)
{
#ifdef CONFIG_RTL8672
	return pcr;
#else
	int k, e, m, pow2, reg;

	k = pcr;
	e=0;

	while (k>1) {
		k = k/2;
		e++;
	}

	//printf("pcr=%d, e=%d\n", pcr,e);
	pow2 = 1;
	for (k = 1; k <= e; k++)
		pow2*=2;

	//printf("pow2=%d\n", pow2);
	//m = ((pcr/pow2)-1)*512;
	k = 0;
	while (pcr >= pow2) {
		pcr -= pow2;
		k++;
	}
	m = (k-1)*512 + pcr*512/pow2;
	//printf("m=%d\n", m);
	reg = (e<<9 | m );
	//printf("reg=%d\n", reg);
	return reg;
#endif
}

/*
 *	get the mark value for a traffic classification
 */

int _get_classification_mark( int entryNo, MIB_CE_IP_QOS_T *p )
{
	int mark=0;

	if(p==NULL) return 0;

	// mark the packet:  8-bits(high) |   8-bits(low)
	//                    class id    |  802.1p (if any)
	mark = ((entryNo+1) << 8);	// class id
	#if 0
	#ifdef QOS_SPEED_LIMIT_SUPPORT
	//use the first 3 bit
	if(p->limitSpeedEnabled)
		{
			int tmpmark=p->limitSpeedRank;

		  	mark|=tmpmark<<12;
			printf("limitSpeedRank=%d,mark=%d\n",p->limitSpeedRank,mark);
		}

	#endif
	#endif
	if (p->m_1p != 0)
		mark |= (p->m_1p-1);	// 802.1p

	return mark;
}

int get_classification_mark(int entryNo)
{
	MIB_CE_IP_QOS_T qEntry;
	int i, num, mark;

	mark = 0;
	num = mib_chain_total(MIB_IP_QOS_TBL);
	if (entryNo >= num)
		return 0;
	// get fwmark
	if (!mib_chain_get(MIB_IP_QOS_TBL, entryNo, (void *)&qEntry))
		return 0;

#if 1
	mark =  _get_classification_mark( entryNo, &qEntry );
#else
	// mark the packet:  8-bits(high) |   8-bits(low)
	//                    class id    |  802.1p (if any)
	mark = ((entryNo+1) << 8);	// class id
	if (qEntry.m_1p != 0)
		mark |= (qEntry.m_1p-1);	// 802.1p
#endif

	return mark;
}

/*
 *	generate the ifup_ppp(vc)x script for WAN interface
 */
static int generate_ifup_script(unsigned int ifIndex, MIB_CE_ATM_VC_Tp pEntry)
{
	int mark, ret;
	FILE *fp;
	char wanif[8], ifup_path[32];
#ifdef IP_POLICY_ROUTING
	int i, num, found;
	MIB_CE_IP_QOS_T qEntry;
#endif
	char ipv6Enable =-1;
	char devname[IFNAMSIZ];

	ret = 0;

	ifGetName(ifIndex, wanif, sizeof(wanif));

	if (PPP_INDEX(ifIndex) != DUMMY_PPP_INDEX) {
		// PPP interface

		snprintf(ifup_path, 32, "/var/ppp/ifup_%s", wanif);
		if (fp=fopen(ifup_path, "w+") ) {
			fprintf(fp, "#!/bin/sh\n\n");
#ifdef IP_POLICY_ROUTING
			num = mib_chain_total(MIB_IP_QOS_TBL);
			found = 0;
			// set advanced-routing rule
			for (i=0; i<num; i++) {
				if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&qEntry))
					continue;
#ifdef QOS_DIFFSERV
				if (qEntry.enDiffserv == 1) // Diffserv entry
					continue;
#endif
				if (qEntry.outif == ifIndex) {

					found = 1;
					mark = get_classification_mark(i);
					if (mark != 0) {
						// Don't forget to point out that fwmark with
						// ipchains/iptables is a decimal number, but that
						// iproute2 uses hexadecimal number.
						fprintf(fp, "ip ru add fwmark %x table %d\n", mark, PPP_INDEX(ifIndex)+PR_PPP_START);
					}
				}
			}
			if (found) {
				fprintf(fp, "ip ro add default dev %s table %d\n", wanif, PPP_INDEX(ifIndex)+PR_PPP_START);
			}
#endif
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
			// Added by Mason Yu for Start upnpd.
			fprintf(fp, "/bin/upnpctrl sync\n");
#endif
#ifdef NEW_PORTMAPPING
			//august: 2012 March 3rd for fixing the pppoe re-up portmapping bugs
			fprintf(fp, "ip ro flush table %d\n", caculate_tblid(ifIndex));
			fprintf(fp, "ip ro add default dev %s table %d\n", wanif, caculate_tblid(ifIndex));

			// default table
			if (pEntry->applicationtype&X_CT_SRV_INTERNET) {
				fprintf(fp, "ip ro flush table %d\n", PMAP_DEFAULT_TBLID);
				ifGetName(PHY_INTF(ifIndex), devname, sizeof(devname));
				fprintf(fp, "ip ro add default dev %s table %d\n", wanif, PMAP_DEFAULT_TBLID);

				if(pEntry->PPPoEProxyEnable){
					fprintf(fp, "route add default %s \n", wanif);
					fprintf(fp, "iptables -I  FORWARD  1 -i ppp+ -o %s -j ACCEPT", wanif);
				}
			}
#endif

			fclose(fp);
			chmod(ifup_path, 484);

			// Added by Mason Yu. For IPV6
#ifdef CONFIG_IPV6
			mib_get(MIB_V6_IPV6_ENABLE, (void *)&ipv6Enable);
			if(ipv6Enable == 1)
			{
				snprintf(ifup_path, 32, "/var/ppp/ifupv6_%s", wanif);
				if (fp=fopen(ifup_path, "w+") ) {
					unsigned char Ipv6AddrStr[48], RemoteIpv6AddrStr[48];
					unsigned char pidfile[30], leasefile[30];
					unsigned char value[256];

					fprintf(fp, "#!/bin/sh\n\n");
					if ((pEntry->AddrMode & IPV6_WAN_AUTO) != IPV6_WAN_AUTO) {
						// SLAAC
						fprintf(fp, "/bin/echo 0 > /proc/sys/net/ipv6/conf/%s/autoconf\n", wanif);
					}
					// Set forwarding=0 to do Slaac
					else {
						fprintf(fp, "/bin/echo 0 > /proc/sys/net/ipv6/conf/%s/forwarding\n", wanif);
					}

					if ( ((pEntry->AddrMode & IPV6_WAN_STATIC)) == IPV6_WAN_STATIC ) {
						unsigned char zero_ip[IP6_ADDR_LEN] = {0};
						char dns_addr[48] = {0};
						int dns_set = 0;

						inet_ntop(PF_INET6, (struct in6_addr *)pEntry->Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
						inet_ntop(PF_INET6, (struct in6_addr *)pEntry->RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));

						// Add WAN static IP
						snprintf(Ipv6AddrStr, 48, "%s/%d", Ipv6AddrStr, pEntry->Ipv6AddrPrefixLen);
						fprintf(fp, "/bin/ifconfig %s add %s\n", wanif, Ipv6AddrStr);

						// Add default gw
						if (pEntry->dgw) {
							// route -A inet6 add ::/0 gw 3ffe::0200:00ff:fe00:0100 dev ppp0
							fprintf(fp, "/bin/route -A inet6  add ::/0 gw %s dev %s\n", RemoteIpv6AddrStr, wanif);
						}

						if(memcmp(zero_ip, pEntry->Ipv6Dns1, IP6_ADDR_LEN) != 0)
						{
							inet_ntop(AF_INET6, pEntry->Ipv6Dns1, dns_addr, 64);
							fprintf(fp, "echo %s > /var/resolv6.conf.%s\n", dns_addr, wanif);
							fprintf(fp, "/bin/route -A inet6 add %s dev %s\n", dns_addr, wanif);
							dns_set = 1;
						}
						if(memcmp(zero_ip, pEntry->Ipv6Dns2, IP6_ADDR_LEN) != 0)
						{
							inet_ntop(AF_INET6, pEntry->Ipv6Dns2, dns_addr, 64);
							fprintf(fp, "echo %s >> /var/resolv6.conf.%s\n", dns_addr, wanif);
							fprintf(fp, "/bin/route -A inet6 add %s dev %s\n", dns_addr, wanif);
							dns_set = 1;
						}

						if(dns_set)
							fprintf(fp, "/bin/kill -SIGUSR1 `cat /var/run/systemd.pid`\n");
					}

#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
					// Start DHCPv6 client
					// dhclient -6 -sf /var/dhclient-script -lf /var/dhclient6-leases -pf /var/run/dhclient6.pid ppp0 -d -q -N -P
					if ( (pEntry->Ipv6Dhcp == 1) || ((pEntry->Ipv6DhcpRequest & 0x2) == 0x2) ) {
						snprintf(leasefile, 30, "/var/%s%s.leases", DHCPCV6STR, wanif);
						snprintf(pidfile, 30, "/var/run/%s%s.pid", DHCPCV6STR, wanif);
						snprintf(value, sizeof(value), "/bin/dhclient -6 -sf /etc/dhclient-script -lf %s -pf %s %s -d -q", leasefile, pidfile, wanif);
						// Request Address
						if ( (pEntry->Ipv6DhcpRequest & 0x1) == 0x1 ) {
							snprintf(value, sizeof(value), "%s -N", value);
						}

						// Request Prefix
						if ( (pEntry->Ipv6DhcpRequest & 0x2) == 0x2 ) {
							snprintf(value, sizeof(value), "%s -P", value);
						}

						// If dsliet enabled, using dhcpv6 option 64 to request AFTR.
						if((pEntry->dslite_enable == 1) && (pEntry->dslite_aftr_mode == IPV6_DSLITE_MODE_AUTO)){
							char fconf[64] = {0};
							sprintf(fconf, "/var/dhclient6_%s.conf", wanif);

							FILE *fp = fopen(fconf, "w");

							if(fp)
							{

								if(pEntry->dslite_enable){
									fprintf(fp, "option dhcp6.dslite code 64 = domain-list; \n");
									fprintf(fp, "also request dhcp6.dslite; \n");
								}

								fclose(fp);
							}

							snprintf(value, sizeof(value), "%s -cf %s ", value, fconf);

						}
						else{
							//If dslite_aftr_mode is static, send signal to let systemd be prepared to setup dslite
							if(pEntry->dslite_enable &&
							 (pEntry->dslite_aftr_mode == IPV6_DSLITE_MODE_STATIC ) &&
							 (pEntry->dslite_aftr_hostname!=NULL)){
									fprintf(fp, "/bin/kill -SIGUSR2 `cat /var/run/systemd.pid`\n");
							}
						}

						snprintf(value, sizeof(value), "%s &\n", value);
						fprintf(fp, value);
					}
#endif

#ifdef NEW_PORTMAPPING
					fprintf(fp, "ip -6 ro flush table %d\n", caculate_tblid(ifIndex));
					fprintf(fp, "ip -6 ro add default dev %s table %d\n", wanif, caculate_tblid(ifIndex));

					// default table
					if (pEntry->applicationtype&X_CT_SRV_INTERNET) {
						fprintf(fp, "ip -6 ro flush table %d\n", PMAP_DEFAULT_TBLID);
						ifGetName(PHY_INTF(ifIndex), devname, sizeof(devname));
						fprintf(fp, "ip -6 ro add default dev %s table %d\n", wanif, PMAP_DEFAULT_TBLID);
					}
#endif
					fclose(fp);
					chmod(ifup_path, 484);
				}
			}  //End of if(ipv6Enable == 1)
#endif

		}
		else
			ret = -1;
	}
	else {
		// not supported till now
		return -1;
	}

	return ret;
}

/*
 *	generate the ifdown_ppp(vc)x script for WAN interface
 */
static int generate_ifdown_script(unsigned int ifIndex, MIB_CE_ATM_VC_Tp pEntry)
{
	int mark, ret;
	FILE *fp;
	char wanif[6], ifdown_path[32];
	char devname[IFNAMSIZ];
	unsigned char vChar;

	ret = 0;

	if (PPP_INDEX(ifIndex) != DUMMY_PPP_INDEX) {
		// PPP interface
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(ifIndex));
		snprintf(ifdown_path, 32, "/var/ppp/ifdown_%s", wanif);
		if (fp=fopen(ifdown_path, "w+") ) {
			fprintf(fp, "#!/bin/sh\n\n");
#ifdef NEW_PORTMAPPING
			if (pEntry->cmode == CHANNEL_MODE_PPPOE) {
				ifGetName(PHY_INTF(ifIndex), devname, sizeof(devname));
				fprintf(fp, "ip ro change default dev %s table %d\n", devname, caculate_tblid(ifIndex));
				// default table
				if (pEntry->applicationtype&X_CT_SRV_INTERNET) {
					fprintf(fp, "ip ro change default dev %s table %d\n", devname, PMAP_DEFAULT_TBLID);
				}
			}
#endif

#if defined(CONFIG_IPV6) && defined(CONFIG_BHS) && defined(CONFIG_USER_DHCPV6_ISC_DHCP411)
            /* For Telefonica IPv6 Test Plan, Test Case ID 20178, if PPPoE session is down,			   *
			 * LAN PC global IPv6 address must change the state from "Prefered" to "Obsolete".		   *
			 * So here, if PPPoE connection is down, and DHCPv6 is auto mode, kill the DHCPv6 process  */

			mib_get( MIB_DHCPV6_MODE, (void *)&vChar);
			if (vChar==3) {  // 3:auto mode
				fprintf(fp, "kill `cat /var/run/dhcpd6.pid`\n");
            }
#endif
			fclose(fp);
			chmod(ifdown_path, 484);
		}
		else
			ret = -1;
	}
	else {
		// not supported till now
		return -1;
	}

	return ret;
}

/*
 *	setup the policy-routing for static link
 */
#ifdef IP_POLICY_ROUTING
static int setup_static_PolicyRouting(unsigned int ifIndex)
{
	int i, num, mark, ret, found;
	char str_mark[8], str_rtable[8], ipAddr[32], wanif[IFNAMSIZ];
	struct in_addr inAddr;
	MIB_CE_IP_QOS_T qEntry;

	ret = -1;

	if (ifIndex == DUMMY_IFINDEX)
		return ret;
	else {
		if (PPP_INDEX(ifIndex) != DUMMY_PPP_INDEX) {
			// PPP interface
			return ret;
		}
		else {
			// vc interface
			ifGetName(ifIndex,wanif,sizeof(wanif));
			snprintf(str_rtable, 8, "%d", VC_INDEX(ifIndex)+PR_VC_START);
			if (getInAddr( wanif, DST_IP_ADDR, (void *)&inAddr) == 1)
			{
				strcpy(ipAddr, inet_ntoa(inAddr));
			}
			else
				return ret;
		}

		if (!getInFlags(wanif, 0))
			return ret;	// interface not found
	}

	num = mib_chain_total(MIB_IP_QOS_TBL);
	found = 0;
	// set advanced-routing rule
	for (i=0; i<num; i++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&qEntry))
			continue;
#ifdef QOS_DIFFSERV
		if (qEntry.enDiffserv == 1) // Diffserv entry
			continue;
#endif
		if (qEntry.outif == ifIndex) {

			found = 1;
			mark = get_classification_mark(i);
			if (mark != 0) {
				snprintf(str_mark, 8, "%x", mark);
				// Don't forget to point out that fwmark with
				// ipchains/iptables is a decimal number, but that
				// iproute2 uses hexadecimal number.
				// ip ru add fwmark xxx table 3
				va_cmd("/bin/ip", 6, 1, "ru", "add", "fwmark", str_mark, "table", str_rtable);
				// ip ro add default via ipaddr dev vc0 table 3
			}
		}
	}
	if (found) {
		// ip ro add default via ipaddr dev vc0 table 3
		va_cmd("/bin/ip", 9, 1, "ro", "add", "default", "via", ipAddr, "dev", wanif, "table", str_rtable);
		ret = 0;
	}

	return ret;
}
#endif


#ifdef CONFIG_USER_WT_146
#define BFD_SERVER_FIFO_NAME "/tmp/bfd_serv_fifo"
#define BFD_CONF_PREFIX "/var/bfd/bfdconf_"
static void wt146_write_to_bfdmain(struct data_to_pass_st *pmsg)
{
	int bfdmain_fifo_fd=-1;

	bfdmain_fifo_fd = open(BFD_SERVER_FIFO_NAME, O_WRONLY);
	if (bfdmain_fifo_fd == -1)
		fprintf(stderr, "Sorry, no bfdmain server\n");
	else
	{
		write(bfdmain_fifo_fd, pmsg, sizeof(*pmsg));
		close(bfdmain_fifo_fd);
	}
}

#define BFDCFG_DBGLOG		"/var/bfd/bfd_dbg_log"
int wt146_dbglog_get(unsigned char *ifname)
{
	char buff[256];
	FILE *fp;
	unsigned char name[32];
	int dbgflag=0;

	//printf( "%s> enter\n", __FUNCTION__ );
	if(ifname)
	{
		//printf( "%s> open %s, search for %s\n", __FUNCTION__, BFDCFG_DBGLOG, ifname );
		fp = fopen(BFDCFG_DBGLOG, "r");
		if(fp)
		{
			while(fgets(buff, sizeof(buff), fp)!=NULL)
			{
				//printf( "%s> got %s", __FUNCTION__, buff );
				int flag;
				if (sscanf(buff, "%s %d", name, &flag) != 2)
					continue;
				else{
					if( strcmp( name, ifname )==0 )
					{
						dbgflag=flag?1:0;
						break;
					}
				}
			}
			fclose(fp);
		}
	}

	//printf( "%s> exit, dbgflag=%d\n", __FUNCTION__, dbgflag );
	return dbgflag;
}

void wt146_create_wan(MIB_CE_ATM_VC_Tp pEntry, int reset_bfd_only )
{
	if(pEntry)
	{
		char wanif[5], conf_name[32];
		FILE *fp;

		if(pEntry->enable==0)
			return;

		if( (pEntry->cmode!=CHANNEL_MODE_IPOE) &&
			(pEntry->cmode!=CHANNEL_MODE_RT1483) )
			return;

		if( pEntry->bfd_enable==0 ) return 0;

		snprintf(wanif, 5, "vc%d", VC_INDEX(pEntry->ifIndex));
		snprintf(conf_name, 32, "%s%s", BFD_CONF_PREFIX, wanif);
		fp=fopen( conf_name, "w" );
		if(fp)
		{
			int bfddebug;
			if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED)
			{
				fprintf( fp, "LocalIP=%s\n",  inet_ntoa(*((struct in_addr *)pEntry->ipAddr)) );
				fprintf( fp, "RemoteIP=%s\n",  inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr)) );
			}

			fprintf( fp, "OpMode=%u\n", pEntry->bfd_opmode );
			fprintf( fp, "Role=%u\n", pEntry->bfd_role );
			fprintf( fp, "DetectMult=%u\n", pEntry->bfd_detectmult );
			fprintf( fp, "MinTxInt=%u\n", pEntry->bfd_mintxint );
			fprintf( fp, "MinRxInt=%u\n", pEntry->bfd_minrxint );
			fprintf( fp, "MinEchoRxInt=%u\n", pEntry->bfd_minechorxint );
			fprintf( fp, "AuthType=%u\n", pEntry->bfd_authtype );
			if( pEntry->bfd_authtype!=BFD_AUTH_NONE )
			{
				int authkeylen=0;
				fprintf( fp, "AuthKeyID=%u\n", pEntry->bfd_authkeyid );
				fprintf( fp, "AuthKey=" );
					while( authkeylen<pEntry->bfd_authkeylen  )
					{
						fprintf( fp, "%02x", pEntry->bfd_authkey[authkeylen] );
						authkeylen++;
					}
					fprintf( fp, "\n" );
			}
			fprintf( fp, "DSCP=%u\n", pEntry->bfd_dscp );
			if(pEntry->cmode==CHANNEL_MODE_IPOE)
				fprintf( fp, "EthPrio=%u\n", pEntry->bfd_ethprio );
			bfddebug=wt146_dbglog_get( wanif );
			fprintf( fp, "debug=%d\n", bfddebug );
			fclose(fp);

			if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED)
			{
				struct data_to_pass_st msg;
				snprintf(msg.data, BUF_SIZE,"bfdctl add %s file %s", wanif, conf_name);
				TRACE(STA_SCRIPT, "%s\n", msg.data);
				//printf( "%s> %s\n",  __FUNCTION__, msg.data);
				wt146_write_to_bfdmain(&msg);
			}else{
				//only re-init bfd, not re-init wanconnection
				if(reset_bfd_only)
				{
					int dhcpc_pid;
					dhcpc_pid = read_pid((char*)DHCPC_PID);
					if (dhcpc_pid > 0)
						kill(dhcpc_pid, SIGHUP);
				}
			}
		}

		//printf( "%s> create %s bfd session\n", __FUNCTION__, wanif );
	}
}

void wt146_del_wan(MIB_CE_ATM_VC_Tp pEntry)
{
	if(pEntry)
	{
		if(pEntry->enable==0)
			return;

		if( (pEntry->cmode!=CHANNEL_MODE_IPOE) &&
			(pEntry->cmode!=CHANNEL_MODE_RT1483) )
			return;

		//if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED)
		{
				struct data_to_pass_st msg;
				char wanif[5], conf_name[32];

				snprintf(wanif, 5, "vc%d", VC_INDEX(pEntry->ifIndex));
				snprintf(conf_name, 32, "%s%s", BFD_CONF_PREFIX, wanif);
				unlink( conf_name);

				snprintf(msg.data, BUF_SIZE,"bfdctl del %s", wanif);
				TRACE(STA_SCRIPT, "%s\n", msg.data);
				//printf( "%s> %s\n",  __FUNCTION__, msg.data);
				wt146_write_to_bfdmain(&msg);

				//printf( "%s> delete %s bfd session\n", __FUNCTION__, wanif );
		}
	}
}

void wt146_set_default_config(MIB_CE_ATM_VC_Tp pEntry)
{
	if(pEntry)
	{
		if( (pEntry->cmode==CHANNEL_MODE_IPOE) ||
			(pEntry->cmode==CHANNEL_MODE_RT1483) )
			pEntry->bfd_enable=1;
		else
			pEntry->bfd_enable=0;

		pEntry->bfd_opmode=BFD_DEMAND_MODE;
		pEntry->bfd_role=BFD_PASSIVE_ROLE;
		pEntry->bfd_detectmult=3;
		pEntry->bfd_mintxint=1000000;
		pEntry->bfd_minrxint=1000000;
		pEntry->bfd_minechorxint=0;
		pEntry->bfd_authtype=BFD_AUTH_NONE;
		pEntry->bfd_authkeyid=0;
		pEntry->bfd_authkeylen=0;
		memset( pEntry->bfd_authkey, 0, sizeof(pEntry->bfd_authkey) );
		pEntry->bfd_dscp=0;
		pEntry->bfd_ethprio=0;
	}
}

void wt146_copy_config(MIB_CE_ATM_VC_Tp pto, MIB_CE_ATM_VC_Tp pfrom)
{
	if(pto && pfrom)
	{
		//printf( "\n\nwt146_copy_config\n" );
		pto->bfd_enable=pfrom->bfd_enable;
		pto->bfd_opmode=pfrom->bfd_opmode;
		pto->bfd_role=pfrom->bfd_role;
		pto->bfd_detectmult=pfrom->bfd_detectmult;
		pto->bfd_mintxint=pfrom->bfd_mintxint;
		pto->bfd_minrxint=pfrom->bfd_minrxint;
		pto->bfd_minechorxint=pfrom->bfd_minechorxint;
		pto->bfd_authtype=pfrom->bfd_authtype;
		pto->bfd_authkeyid=pfrom->bfd_authkeyid;
		pto->bfd_authkeylen=pfrom->bfd_authkeylen;
		memcpy( pto->bfd_authkey, pfrom->bfd_authkey, sizeof(pto->bfd_authkey) );
		pto->bfd_dscp=pfrom->bfd_dscp;
		pto->bfd_ethprio=pfrom->bfd_ethprio;
	}
}
#endif //CONFIG_USER_WT_146

//Inform kernel the number of routing interface.
// flag = 1 to increase 1
// flag = 0 to decrease 1
static void internetLed_route_if(int flag)
{
	FILE *fp;

	fp = fopen ("/proc/IntnetLED", "w");
	if (fp)
	{
		if (flag)
			fprintf(fp,"+");
		else
			fprintf(fp,"-");
		fclose(fp);
	}
}

static int setup_ethernet_config(MIB_CE_ATM_VC_Tp pEntry, char *wanif)
{
	char *argv[8];
	int status=0;
	unsigned char devAddr[MAC_ADDR_LEN];
	char macaddr[MAC_ADDR_LEN*2+1];

#if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
#ifdef CONFIG_ATM_CLIP
		if(pEntry->cmode != CHANNEL_MODE_RT1577)
#endif
		{
			snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
				pEntry->MacAddr[0], pEntry->MacAddr[1], pEntry->MacAddr[2], pEntry->MacAddr[3], pEntry->MacAddr[4], pEntry->MacAddr[5]);

			argv[1]=wanif;
			argv[2]="hw";
			argv[3]="ether";
			argv[4]=macaddr;
			argv[5]=NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s %s\n", IFCONFIG, argv[1], argv[2], argv[3], argv[4]);
			status|=do_cmd(IFCONFIG, argv, 1);
		}
#else	// #if defined(CONFIG_LUNA) && defined(GEN_WAN_MAC)
#ifdef CONFIG_ATM_CLIP
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr) != 0 && pEntry->cmode != CHANNEL_MODE_RT1577)
#else
		if (mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr) != 0)
#endif
		{
			snprintf(macaddr, 13, "%02x%02x%02x%02x%02x%02x",
				devAddr[0], devAddr[1], devAddr[2],
#ifdef CONFIG_USER_IPV6READYLOGO_ROUTER
				//Set vc and nas mac with br0's mac plus 1
				devAddr[3], devAddr[4], devAddr[5]+1);
#else
				devAddr[3], devAddr[4], devAddr[5]);
#endif
			argv[1]=wanif;
			argv[2]="hw";
			argv[3]="ether";
			argv[4]=macaddr;
			argv[5]=NULL;
			TRACE(STA_SCRIPT, "%s %s %s %s %s\n", IFCONFIG, argv[1], argv[2],
						argv[3], argv[4]);
			status|=do_cmd(IFCONFIG, argv, 1);
		}
#endif

#ifdef CONFIG_RTK_RG_INIT

		char sysbuf[128];
		int wanPhyPort;
#if 0
		if(mib_get(MIB_WAN_PHY_PORT,(void *)&wanPhyPort)){
			sprintf( sysbuf, "/bin/echo %d nas0 > /proc/rtl8686gmac/dev_port_mapping",wanPhyPort );
		}
#else
		wanPhyPort = RG_get_wan_phyPortId();
		if(wanPhyPort!=-1){
			sprintf( sysbuf, "/bin/echo %d nas0 > /proc/rtl8686gmac/dev_port_mapping",wanPhyPort );
		}
#endif

		/*
		#ifdef CONFIG_RG_API_RLE0371
		sprintf( sysbuf, "/bin/echo 5 nas0 > /proc/rtl8686gmac/dev_port_mapping" );
		#else
		sprintf( sysbuf, "/bin/echo 3 nas0 > /proc/rtl8686gmac/dev_port_mapping" );
		#endif
		*/
		printf( "system(): %s\n", sysbuf );
		system(sysbuf);

		//Temporary solution for netlink event down/up that will let spppd has ubnormal behavior.
		sleep(1);
#endif
		// ifconfig vc0 txqueuelen 10
		argv[1] = wanif;
		argv[2] = "txqueuelen";
#ifdef NEW_IP_QOS_SUPPORT
#ifdef MINIMIZE_RAM_USAGE
		/*ql:20081118 START: use TC to implement IP QoS, then device txqueuelen should not be 0*/
		//argv[3] = "0";
		argv[3] = "100";
		/*ql:20081118 END*/
#else
		argv[3] = "100";
#endif
#else
                #ifdef MINIMIZE_RAM_USAGE
		argv[3] = "0";
                #else
                argv[3] = "10";
                #endif
#endif
		argv[4] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s %s\n", IFCONFIG, argv[1], argv[2], argv[3]);
		status|=do_cmd(IFCONFIG, argv, 1);
#ifdef CONFIG_IPV6
		// Disable ipv6 in bridge
		if (pEntry->cmode == CHANNEL_MODE_BRIDGE)
			setup_disable_ipv6(wanif, 1);
		else { // enable ipv6 if applicable
			if (pEntry->IpProtocol & IPVER_IPV6)
				setup_disable_ipv6(wanif, 0);
			else
				setup_disable_ipv6(wanif, 1);
		}
#endif
		// ifconfig vc0 up
		argv[2] = "up";
		argv[3] = NULL;
		TRACE(STA_SCRIPT, "%s %s %s\n", IFCONFIG, argv[1], argv[2]);
		status|=do_cmd(IFCONFIG, argv, 1);

		return status;
}

#ifdef CONFIG_RTL8672_SAR
#ifdef CONFIG_RTL_MULTI_PVC_WAN
static void addVCVirtualDev(MIB_CE_ATM_VC_Tp pEntry)
{
	MEDIA_TYPE_T mType;
	char ifname[IFNAMSIZ];  // virtual vc device name
	char wanif[IFNAMSIZ];   // major vc device name

	// Get virtual vc device name
	ifGetName(PHY_INTF(pEntry->ifIndex), ifname, sizeof(ifname));
	// Get major vc device name
	snprintf(wanif, IFNAMSIZ,  "vc%u", VC_MAJOR_INDEX(pEntry->ifIndex));
	mType = MEDIA_INDEX(pEntry->ifIndex);

	//add new vcwan dev here.
	if (mType == MEDIA_ATM && (WAN_MODE & MODE_ATM)) {
		int tmp_group;
		const char smux_brg_cmd[]="/bin/ethctl addsmux bridge %s %s";
		const char smux_ipoe_cmd[]="/bin/ethctl addsmux ipoe %s %s";
		const char smux_pppoe_cmd[]="/bin/ethctl addsmux pppoe %s %s";
		char cmd_str[100];

		//va_cmd(IFCONFIG, 2, 1, "vc0", "up");
		if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_BRIDGE)
			snprintf(cmd_str, 100, smux_brg_cmd, wanif, ifname);
		else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_IPOE)
			snprintf(cmd_str, 100, smux_ipoe_cmd, wanif, ifname);
		else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOE)
			snprintf(cmd_str, 100, smux_pppoe_cmd, wanif, ifname);

		if (pEntry->napt)
			strncat(cmd_str, " napt", 100);

		if (pEntry->vlan) {
			unsigned int vlantag;
			vlantag = (pEntry->vid|((pEntry->vprio) << 13));
			snprintf(cmd_str, 100, "%s vlan %d", cmd_str, vlantag );
		}
		printf("TRACE: %s\n", cmd_str);
		system(cmd_str);
	}
}
#endif

/*
 *	ATM channel: RFC2684, RFC1577, PPPoA
 */
static int setup_ATM_channel(MIB_CE_ATM_VC_Tp pEntry)
{
	struct data_to_pass_st msg;
	char *aal5Encap, qosParms[64];
	char wanif[IFNAMSIZ];
	int pcreg, screg;

	if (MEDIA_INDEX(pEntry->ifIndex) != MEDIA_ATM)
		return 0;

	// get the aal5 encapsulation
	if (pEntry->encap == ENCAP_VCMUX)
	{
#ifdef CONFIG_ATM_CLIP
		if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_RT1483 || (CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_RT1577)
#else
		if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_RT1483)
#endif
			aal5Encap = (char *)VC_RT;
		else
			aal5Encap = (char *)VC_BR;
	}
	else	// LLC
	{
		if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_RT1483)
			aal5Encap = (char *)LLC_RT;
#ifdef CONFIG_ATM_CLIP
		else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_RT1577)
			aal5Encap = (char *)LLC_1577;
#endif
		else
			aal5Encap = (char *)LLC_BR;
	}

	// calculate the 15-0(bits) PCR CTLSTS value
	pcreg = cr2reg(pEntry->pcr);

	if ((ATM_QOS_T)pEntry->qos == ATMQOS_CBR)
	{
		snprintf(qosParms, 64, "cbr:pcr=%u", pcreg);
	}
	else if ((ATM_QOS_T)pEntry->qos == ATMQOS_VBR_NRT)
	{
		screg = cr2reg(pEntry->scr);
		snprintf(qosParms, 64, "nrt-vbr:pcr=%u,scr=%u,mbs=%u",
			pcreg, screg, pEntry->mbs);
	}
	else if ((ATM_QOS_T)pEntry->qos == ATMQOS_VBR_RT)
	{
		screg = cr2reg(pEntry->scr);
		snprintf(qosParms, 64, "rt-vbr:pcr=%u,scr=%u,mbs=%u",
			pcreg, screg, pEntry->mbs);
	}
	else //if ((ATM_QOS_T)pEntry->qos == ATMQOS_UBR)
	{
		snprintf(qosParms, 64, "ubr:pcr=%u", pcreg);
	}
	// Mason Yu Test
	//printf("qosParms=%s\n", qosParms);
#ifdef CONFIG_RTL_MULTI_PVC_WAN
	snprintf( wanif, IFNAMSIZ,  "vc%u", VC_MAJOR_INDEX(pEntry->ifIndex));
#else
	ifGetName(PHY_INTF(pEntry->ifIndex), wanif, sizeof(wanif));
#endif
	// start this vc
	if (pEntry->cmode != CHANNEL_MODE_PPPOA && !find_pvc_from_running(pEntry->vpi, pEntry->vci))
	{
		unsigned char devAddr[MAC_ADDR_LEN];
		char macaddr[MAC_ADDR_LEN*2+1];

		// mpoactl add vc0 pvc 0.33 encaps 4 qos xxx brpppoe x

#ifdef CONFIG_ATM_CLIP
		if (pEntry->cmode != CHANNEL_MODE_RT1577)
#endif

		// 20130426 W.H. Hung: briged PPPoE setup has been replaced
		// by ebtables rules.
		snprintf(msg.data, BUF_SIZE,
			"mpoactl add %s pvc %u.%u encaps %s qos %s", wanif,
			pEntry->vpi, pEntry->vci, aal5Encap, qosParms);
#ifdef CONFIG_ATM_CLIP
		else
			snprintf(msg.data, BUF_SIZE,
				"mpoactl addclip %s pvc %u.%u encaps %s qos %s", wanif,
				pEntry->vpi, pEntry->vci, aal5Encap, qosParms);
#endif
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_mpoad(&msg);

#ifdef _LINUX_2_6_
		{
			int repeat_i=0;
			//make sure the channel is created (ifconfig hw ether may fail because the vc# is not ready)
#ifdef CONFIG_RTL_MULTI_PVC_WAN
			while( (repeat_i<10) )
#else
			while( (repeat_i<10) && (!find_pvc_from_running(pEntry->vpi, pEntry->vci)) )
#endif
			{
				usleep(50000);
				repeat_i++;
			}
		}
#endif //_LINUX_2_6_
#ifdef CONFIG_RTL_MULTI_PVC_WAN
		// start root vc for multi-wan PVC
		setup_ethernet_config(pEntry, wanif);
#endif
	}

	if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOA)
	{
		// PPPoA
		printf("PPPoA\n");
		if (startPPP(0, pEntry, qosParms, CHANNEL_MODE_PPPOA) == -1)
		{
			printf("start PPPoA failed !\n");
			return 0;
		}
	}
	#ifdef CONFIG_ATM_CLIP
	else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_RT1577)
	{
		// rfc1577-routed
		printf("1577 routed\n");
		unsigned long addr;
		struct data_to_pass_st msg;

		if (startIP(wanif, pEntry, CHANNEL_MODE_RT1577) == -1)
		{
			printf("start 1577-routed failed !\n");
			return 0;
		}
		addr = *((unsigned long *)pEntry->ipAddr);
		snprintf(msg.data, BUF_SIZE, "mpoactl set %s cip %lu", wanif, addr);
		write_to_mpoad(&msg);
	}
	#endif
	if(pEntry->cmode != CHANNEL_MODE_BRIDGE)
	{
		unsigned char cmode[4], vpi[4], vci[4];
//		printf("ch %d (vc_index), cmode %d (%d. %d)\n", VC_INDEX(pEntry->ifIndex), pEntry->cmode, pEntry->vpi, pEntry->vci);
                sprintf(cmode, "%d", pEntry->cmode);
		//transfer vpi,vci for setting PPPoA correct CH index
		sprintf(vpi, "%d", pEntry->vpi);
		sprintf(vci, "%d", pEntry->vci);
		va_cmd("/bin/sarctl",4,1,"set_header", cmode,vpi,vci);
	}
	va_cmd("/bin/sarctl",1,1,"set_pkta");

#ifdef CONFIG_RTL_MULTI_PVC_WAN
	// create virtual interface, ex. vc0_0
	addVCVirtualDev(pEntry);
#endif
	return 1;
}
#endif // CONFIG_RTL8672_SAR


#ifdef CONFIG_HWNAT
void setup_hwnat_eth_member(int port, int mbr, char enable)
{
	struct ifreq ifr;
	struct ifvlan ifvl;

#ifndef CONFIG_RTL_MULTI_ETH_WAN
	strcpy(ifr.ifr_name, ALIASNAME_NAS0);
#else
	sprintf(ifr.ifr_name, "%s%d", ALIASNAME_MWNAS, ETH_INDEX(port));
#endif
	ifr.ifr_data = (char *)&ifvl;
	ifvl.enable=enable;
	ifvl.port=port;
	ifvl.member=mbr;
	if(setEthPortMapping(&ifr)!=1)
		printf("set hwnat port mapping error!\n");
	return;
}
#ifdef CONFIG_PTMWAN
void setup_hwnat_ptm_member(int port, int mbr, char enable)
{
	struct ifreq ifr;
	struct ifvlan ifvl;

	sprintf(ifr.ifr_name, "%s%d", ALIASNAME_MWPTM, PTM_INDEX(port));
	ifr.ifr_data = (char *)&ifvl;
	ifvl.enable=enable;
	ifvl.port=port;
	ifvl.member=mbr;
	if(setEthPortMapping(&ifr)!=1)
		printf("set ptm hwnat port mapping error!\n");
	return;
}
#endif /*CONFIG_PTMWAN*/
#endif

#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
int Init_RTK_RG_Device(void)
{
	char wanif[IFNAMSIZ];
	int i;

	va_cmd(BRCTL, 2, 1, "addbr", BRIF );

	for(i=1;i<6;i++)
	{
		snprintf( wanif, sizeof(wanif), "eth0.%d", i);
		va_cmd(IFCONFIG, 2, 1, wanif, "down");
	}
	snprintf( wanif, sizeof(wanif), "eth0");
	va_cmd(BRCTL, 3, 1, "addif", BRIF, wanif);

//	snprintf( wanif, sizeof(wanif), "nas0");
//	va_cmd(BRCTL, 3, 1, "delif", BRIF, wanif);

/*
	for(i=1;i<6;i++)
	{
		snprintf( wanif, sizeof(wanif), "eth0.%d", i);
		va_cmd(BRCTL, 3, 1, "delif", BRIF, wanif);
	}
*/
	snprintf( wanif, sizeof(wanif), "eth0");
	va_cmd(IFCONFIG, 2, 1, wanif, "down");

	snprintf( wanif, sizeof(wanif), "nas0");
	va_cmd(IFCONFIG, 2, 1, wanif, "down");

	snprintf( wanif, sizeof(wanif), "br0");
	va_cmd(IFCONFIG, 2, 1, wanif, "down");
}
#endif

// return value:
// 0  : successful
// -1 : failed
int startConnection(MIB_CE_ATM_VC_Tp pEntry, int mib_vc_idx)
{
	struct data_to_pass_st msg;
	char *aal5Encap, qosParms[64], wanif[IFNAMSIZ];
	int brpppoe;
	int pcreg, screg;
	int status=0;
	MEDIA_TYPE_T mType;
	char vChar=-1;
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	unsigned char upnpdEnable;
	unsigned int upnpItf;
	char ext_ifname[IFNAMSIZ];
#endif

	if(pEntry == NULL || pEntry->enable == 0)
	{
		return 0;
	}

	mType = MEDIA_INDEX(pEntry->ifIndex);
#ifdef CONFIG_RTL8672_SAR
	if (mType == MEDIA_ATM)
		setup_ATM_channel(pEntry);
#else
	if (mType == MEDIA_ATM)
		return 0;
#endif
	if (pEntry->cmode != CHANNEL_MODE_BRIDGE)
		internetLed_route_if(1);//+1

#ifdef NEW_PORTMAPPING
		get_pmap_fgroup(pmap_list, MAX_VC_NUM);
		setup_wan_pmap_lanmember(mType, pEntry->ifIndex);
#endif

	//snprintf(wanif, 5, "vc%d", VC_INDEX(pEntry->ifIndex));
	ifGetName(PHY_INTF(pEntry->ifIndex),wanif,sizeof(wanif));
	//printf("%s true_%s, major dev=%s\n", __func__, MEDIA_INDEX(pEntry->ifIndex)?"ETH":"ATM", wanif);

	if (pEntry->cmode != CHANNEL_MODE_PPPOA)
	{
		setup_ethernet_config(pEntry, wanif);
	}

// Mason Yu. enable_802_1p_090722
#ifdef CONFIG_RTL8672_SAR
#ifdef ENABLE_802_1Q
	// This vlan tag function is mutual exclusive with multi-wan pvc.
#ifndef CONFIG_RTL_MULTI_PVC_WAN
	// mpoactl set vc0 vlan 1 pvid 1
	snprintf(msg.data, BUF_SIZE,
		"mpoactl set %s vlan %d vid %d vprio %d", wanif,
		pEntry->vlan, pEntry->vid, pEntry->vprio);
	if (pEntry->cmode == CHANNEL_MODE_BRIDGE || pEntry->cmode == CHANNEL_MODE_IPOE ||
		pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_6RD) {
		if (mType == MEDIA_ATM) {
			// mpoactl set vc0 vlan 1 pvid 1
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			status|=write_to_mpoad(&msg);
		}
	}
#endif
#endif
#endif

	if (pEntry->cmode == CHANNEL_MODE_BRIDGE)
	{
  	//shlee turns on DIRECT_BRIDGE_MODE only bridge mode pvc
  	//Avoiding pppoe passthrough will enter direct bridge
		if (mType == MEDIA_ATM) {
			unsigned char ch_no[32];
			sprintf(ch_no,"%d",VC_INDEX(pEntry->ifIndex));
	  		va_cmd("/bin/sarctl",2,1,"direct_bridge",ch_no);	// Turns on direct bridge mode
		}

		// rfc1483-bridged
		// brctl addif br0 vc0
		printf("1483 bridged\n");
		// Mason Yu
		if ( pEntry->brmode != BRIDGE_DISABLE )
			status|=va_cmd(BRCTL, 3, 1, "addif", BRIF, wanif);
	}
	else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_IPOE)
	{
		// MAC Encapsulated Routing
		printf("1483 MER\n");

		if (startIP(wanif, pEntry, CHANNEL_MODE_IPOE) == -1)
		{
			printf("start MER failed !\n");
			status=-1;
		}
//forwarding Engine will do it.
#if defined(PPPOE_PASSTHROUGH) && !defined(CONFIG_RTK_RG_INIT)
		//if (pEntry->brmode == BRIDGE_PPPOE)	// enable PPPoE pass-through
		if (pEntry->brmode != BRIDGE_DISABLE)	// enable bridge
			// brctl addif br0 vc0
			status|=va_cmd(BRCTL, 3, 1, "addif", BRIF, wanif);
#endif
		if ( pEntry->ipDhcp == DHCP_DISABLED ) {
#ifdef IP_POLICY_ROUTING
			setup_static_PolicyRouting(pEntry->ifIndex);
#endif
		}
	}
	else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOE)
	{
		// PPPoE
		printf("PPPoE\n");

		if (startPPP(wanif, pEntry, 0, CHANNEL_MODE_PPPOE) == -1)
		{
			printf("start PPPoE failed !\n");
			status=-1;
		}
//forwarding Engine will do it.
#if defined(PPPOE_PASSTHROUGH) && !defined(CONFIG_RTK_RG_INIT)
		//if (pEntry->brmode == BRIDGE_PPPOE)	// enable PPPoE pass-through
		if (pEntry->brmode != BRIDGE_DISABLE)	// enable bridge
			// brctl addif br0 vc0
			status|=va_cmd(BRCTL, 3, 1, "addif", BRIF, wanif);
#endif

	}
#ifdef CONFIG_RTL8672_SAR
	else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_RT1483)
	{
		// rfc1483-routed
		printf("1483 routed\n");
		if (startIP(wanif, pEntry, CHANNEL_MODE_RT1483) == -1)
		{
			printf("start 1483-routed failed !\n");
			status=-1;
		}
#ifdef IP_POLICY_ROUTING
		setup_static_PolicyRouting(pEntry->ifIndex);
#endif
	}
#endif
#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
	else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_6RD)
	{
		// 6RD
		printf("6RD\n");
		if (startIP(wanif, pEntry, CHANNEL_MODE_IPOE) == -1)
		{
			printf("start 6RD failed !\n");
			status=-1;
		}
	}
#endif

// Magician: UPnP Daemon Start
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
	if(mib_get(MIB_UPNP_DAEMON, (void *)&upnpdEnable) && upnpdEnable)
	{
		if (mib_get(MIB_UPNP_EXT_ITF, (void *)&upnpItf) && upnpItf == pEntry->ifIndex)
		{
			ifGetName(upnpItf, ext_ifname, sizeof(ext_ifname));

			va_cmd("/bin/upnpctrl", 1, 1, "sync");
			va_cmd("/bin/upnpctrl", 3, 1, "up", ext_ifname, BRIF);
		}
	}
#endif
// The end of UPnP Daemon Start

#ifdef CONFIG_USER_WT_146
	wt146_create_wan(pEntry, 0);
#endif //CONFIG_USER_WT_146

#ifdef CONFIG_IPV6
/* Martin zhu add for bridge WAN support ipv4/ipv6 2015.6.29 */
	setupBridgeIPv4IPv6Filter(pEntry,1);
#endif

	if (status >= 0) {
		generate_ifup_script(pEntry->ifIndex, pEntry);
		generate_ifdown_script(pEntry->ifIndex, pEntry);
	}

	return status;
}

#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
static void addEthWANdev(MIB_CE_ATM_VC_Tp pEntry)
{
	MEDIA_TYPE_T mType;
	char ifname[IFNAMSIZ];
	int flag=0;

	ifGetName(PHY_INTF(pEntry->ifIndex), ifname, sizeof(ifname));
	mType = MEDIA_INDEX(pEntry->ifIndex);

	//add new ethwan dev here.
	if( ((mType == MEDIA_ETH) && (WAN_MODE & MODE_Ethernet))
	     #ifdef CONFIG_PTMWAN
	     ||((mType == MEDIA_PTM) && (WAN_MODE & MODE_PTM))
	     #endif /*CONFIG_PTMWAN*/
	){
		int tmp_group;
		const char smux_brg_cmd[]="/bin/ethctl addsmux bridge %s %s";
		const char smux_ipoe_cmd[]="/bin/ethctl addsmux ipoe %s %s";
		const char smux_pppoe_cmd[]="/bin/ethctl addsmux pppoe %s %s";

		char cmd_str[100];
		unsigned char *rootdev=NULL;

		#ifdef CONFIG_PTMWAN
		if(mType == MEDIA_PTM)
			rootdev=ALIASNAME_PTM0;
		else
		#endif /*CONFIG_PTMWAN*/
			rootdev=ALIASNAME_NAS0;

		va_cmd(IFCONFIG, 2, 1, rootdev, "up");
		if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_BRIDGE)
			snprintf(cmd_str, 100, smux_brg_cmd, rootdev, ifname);
		else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_IPOE || (CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_6RD)
			snprintf(cmd_str, 100, smux_ipoe_cmd, rootdev, ifname);
		else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOE)
			snprintf(cmd_str, 100, smux_pppoe_cmd, rootdev, ifname);

		if (pEntry->napt)
			strncat(cmd_str, " napt", 100);

		if(pEntry->brmode != BRIDGE_DISABLE)
			strncat(cmd_str, " brpppoe", 100);

		if (pEntry->vlan) {
			unsigned int vlantag;
			vlantag = (pEntry->vid|((pEntry->vprio) << 13));
			snprintf(cmd_str, 100, "%s vlan %d", cmd_str, vlantag );
		}
		printf("TRACE: %s\n", cmd_str);
		system(cmd_str);

		while(getInFlags(ifname, &flag)==0) { //wait the device created successly, Iulian Wu
			printf("Ethernet WAN not ready!\n");
			sleep(10);
		}
	}
}
#endif

#ifdef CONFIG_RTL_MULTI_PVC_WAN
static int find_latest_used_vc(MIB_CE_ATM_VC_Tp pEntry)
{
	int entryNum=0, i, num=0;
	MIB_CE_ATM_VC_T Entry;

	entryNum =  mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entryNum; i++) {
		/* Retrieve entry */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;
		if (VC_MAJOR_INDEX(Entry.ifIndex) == VC_MAJOR_INDEX(pEntry->ifIndex)) {
		    num = VC_MINOR_INDEX(Entry.ifIndex);
		}
	}
	return num;
}
#endif

void stopConnection(MIB_CE_ATM_VC_Tp pEntry)
{
	struct data_to_pass_st msg;
	char wandev[IFNAMSIZ];
	char wanif[IFNAMSIZ];
	char myip[16];
	int i;
#ifdef CONFIG_RTL_MULTI_PVC_WAN
	char ifname[IFNAMSIZ];   // major vc device name
	int num;
#endif
#ifdef CONFIG_USER_MINIUPNPD
	unsigned char upnpdEnable;
	unsigned int upnpItf;
	char ext_ifname[IFNAMSIZ];
#endif

	clean_SourceRoute(pEntry);

#ifdef CONFIG_USER_WT_146
	wt146_del_wan(pEntry);
#endif //CONFIG_USER_WT_146

// Magician: UPnP Daemon Stop
#ifdef CONFIG_USER_MINIUPNPD
	if (mib_get(MIB_UPNP_DAEMON, &upnpdEnable) && upnpdEnable) {
		if (mib_get(MIB_UPNP_EXT_ITF, &upnpItf)
		    && upnpItf == pEntry->ifIndex) {
			ifGetName(upnpItf, ext_ifname, sizeof(ext_ifname));

			va_cmd("/bin/upnpctrl", 3, 1, "down", ext_ifname, BRIF);
		}
	}
#endif
// The end of UPnP Daemon Stop

	// delete this vc
	if (pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA)
	{
		int i,entryNum;
		MIB_CE_ATM_VC_T Entry;

		// spppctl del 0
		snprintf(msg.data, BUF_SIZE,
			"spppctl del %u", PPP_INDEX(pEntry->ifIndex));
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
		printf("send command [%s]\n",msg.data);

		/* Andrew, need to remove all PPP using the same underlying VC or kernel will have NULL references */
		entryNum =  mib_chain_total(MIB_ATM_VC_TBL);
		for (i = 0; i < entryNum; i++) {
			/* Retrieve entry */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				continue;

			if ((Entry.cmode != CHANNEL_MODE_PPPOE) && (Entry.cmode != CHANNEL_MODE_PPPOA))
				continue;
			if (VC_INDEX(Entry.ifIndex) != VC_INDEX(pEntry->ifIndex))
				continue;
			if (MEDIA_INDEX(pEntry->ifIndex) != MEDIA_INDEX(Entry.ifIndex))
				continue;

			snprintf(msg.data, BUF_SIZE, "spppctl del %u", PPP_INDEX(Entry.ifIndex));
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
	}
	else{
		//snprintf(wanif, 6, "vc%u", VC_INDEX(pEntry->ifIndex));
		ifGetName( PHY_INTF(pEntry->ifIndex), wanif, sizeof(wanif));
	}

	ifGetName( PHY_INTF(pEntry->ifIndex), wandev, sizeof(wandev));

#ifdef CONFIG_USER_PPPOE_PROXY
		if (pEntry->PPPoEProxyEnable==1)
		{
			char tp[10];
			for (i=9; i<=13; i++)	//maximum pppoe session count is 5
			{
				sprintf(tp, "ppp%d", i);
				if (find_ppp_from_conf(tp)) {
					snprintf(msg.data, BUF_SIZE, "spppctl delproxyunit %d", i);
					TRACE(STA_SCRIPT, "%s\n", msg.data);
					write_to_pppd(&msg);
					printf("send command [%s]\n",msg.data);
				}
			}

#ifdef CONFIG_RTK_RG_INIT
			printf("Flush PPPoE proxy ACL rule.\n");
			RTK_RG_PPPoEProxy_ACL_Rule_Flush();
#endif
		}
#endif

#ifdef CONFIG_IPV6
	stopIP_PPP_for_V6(pEntry);
#endif

#ifdef CONFIG_IPV6
	/* Martin zhu add for bridge WAN support ipv4/ipv6 Filter 2015.6.29 */
	setupBridgeIPv4IPv6Filter(pEntry,0);
#endif

	/******************************************************************************************************************/
	/* Ruby Note: WAN pppx must be deleted first, then  WAN Interface cound down, or it may happen that like ptm0_0   */
	/*       is disappeared , and the socket in pppx will access incorrect data in kernel pppoe_flush_dev.            */
	/******************************************************************************************************************/
	sleep(2);
	va_cmd(BRCTL, 3, 1, "delif", BRIF, wandev);

	if (pEntry->cmode != CHANNEL_MODE_BRIDGE)
		internetLed_route_if(0);//-1

	/* Kevin, stop root Qdisc before ifdown interface, for 0412 hw closing IPQoS */
#ifdef CONFIG_RTL_MULTI_PVC_WAN
	// Get major vc device name
	snprintf(ifname, IFNAMSIZ,  "vc%u", VC_MAJOR_INDEX(pEntry->ifIndex));
#endif
	va_cmd(TC, 5, 1, "qdisc", (char *)ARG_DEL, "dev", wandev, "root");

	if (pEntry->cmode != CHANNEL_MODE_PPPOA) {
		//ifconfig vc0 0.0.0.0
		va_cmd(IFCONFIG, 2, 1, wanif, "0.0.0.0");
		// ifconfig vc0 down
		va_cmd(IFCONFIG, 2, 1, wanif, "down");
		// wait for sar to process the queueing packets
		//usleep(1000);
		for (i=0; i<10000000; i++);

		#ifdef CONFIG_RTL8672_SAR
		if (MEDIA_INDEX(pEntry->ifIndex) == MEDIA_ATM) {
#ifdef CONFIG_RTL_MULTI_PVC_WAN
			// ethctl remsumx vcX vcX_X
			//printf("** Remove smux device %s(root %s)\n", wanif, ifname);
			if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_BRIDGE)
			{
				va_cmd("/bin/ethctl", 4, 1, "remsmux", "bridge", ifname, wandev);
			}
			else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_IPOE)
			{
				va_cmd("/bin/ethctl", 4, 1, "remsmux", "ipoe", ifname, wandev);
			}
			else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOE)
			{
				va_cmd("/bin/ethctl", 4, 1, "remsmux", "pppoe", ifname, wandev);
			}
#endif

			// mpoactl del vc0
#ifdef CONFIG_RTL_MULTI_PVC_WAN
			num = find_latest_used_vc(pEntry);
			if ( num == VC_MINOR_INDEX(pEntry->ifIndex)) {
				// delete major vc name
				//printf("**stopConnection: delete major vc name=%s\n", ifname);
				//ifconfig vc0 0.0.0.0
				va_cmd(IFCONFIG, 2, 1, ifname, "0.0.0.0");
				// ifconfig vc0 down
				va_cmd(IFCONFIG, 2, 1, ifname, "down");
				// wait for sar to process the queueing packets
				//usleep(1000);
				for (i=0; i<10000000; i++);

				snprintf(msg.data, BUF_SIZE, "mpoactl del vc%u", VC_MAJOR_INDEX(pEntry->ifIndex));
				TRACE(STA_SCRIPT, "%s\n", msg.data);
				write_to_mpoad(&msg);

				// make sure this vc been deleted completely
				while (find_pvc_from_running(pEntry->vpi, pEntry->vci));
			}
#else
			snprintf(msg.data, BUF_SIZE, "mpoactl del vc%u", VC_INDEX(pEntry->ifIndex));
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_mpoad(&msg);

			// make sure this vc been deleted completely
			while (find_pvc_from_running(pEntry->vpi, pEntry->vci));
#endif
		}
		#endif
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
		//del ethwan dev here.
		if( (MEDIA_INDEX(pEntry->ifIndex) == MEDIA_ETH)
		     #ifdef CONFIG_PTMWAN
		     || (MEDIA_INDEX(pEntry->ifIndex) == MEDIA_PTM)
		     #endif /*CONFIG_PTMWAN*/
		){
			unsigned char *rootdev;

			#ifdef CONFIG_PTMWAN
			if(MEDIA_INDEX(pEntry->ifIndex) == MEDIA_PTM)
				rootdev=ALIASNAME_PTM0;
			else
			#endif /*CONFIG_PTMWAN*/
				rootdev=ALIASNAME_NAS0;

#ifdef CONFIG_IPV6
			if (pEntry->IpProtocol & IPVER_IPV4 && (DHCP_T)pEntry->ipDhcp != DHCP_DISABLED)
			{
				int dhcpc_pid = 0;
				char filename[100] = {0};

				sprintf(filename, "%s.%s", (char*)DHCPC_PID, wandev);
				dhcpc_pid = read_pid(filename);
				if (dhcpc_pid > 0)
					kill(dhcpc_pid, SIGHUP);
			}
#endif

			printf("%s remove smux device %s\n", __func__, wandev);
			if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_BRIDGE)
			{
				va_cmd("/bin/ethctl", 4, 1, "remsmux", "bridge", rootdev, wandev);
			}
			else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_IPOE)
			{
				va_cmd("/bin/ethctl", 4, 1, "remsmux", "ipoe", rootdev, wandev);
			}
			else if ((CHANNEL_MODE_T)pEntry->cmode == CHANNEL_MODE_PPPOE)
			{
				va_cmd("/bin/ethctl", 4, 1, "remsmux", "pppoe", rootdev, wandev);
			}
		}
#endif
	}
	#ifdef CONFIG_RTL8672_SAR
	else {
		int repeat_i=0;
		//ifconfig vc0 0.0.0.0
		va_cmd(IFCONFIG, 2, 1, wanif, "0.0.0.0");
		va_cmd(IFCONFIG, 2, 1, wanif, "down");
		for (i=0; i<10000000; i++);
		while( (repeat_i<10) && (find_pvc_from_running(pEntry->vpi, pEntry->vci)) )
		{
			usleep(50000);
			repeat_i++;
		}
	}
	#endif

	// delete one NAT rule for the specific interface
	stopAddressMap(pEntry);

#ifdef CONFIG_HWNAT
	if (pEntry->napt == 1){
		if ((DHCP_T)pEntry->ipDhcp == DHCP_DISABLED) {
			send_extip_to_hwnat(DEL_EXTIP_CMD, wandev, *(uint32 *)pEntry->ipAddr);
		}
		else{
			send_extip_to_hwnat(DEL_EXTIP_CMD, wandev, 0);
		}
	}
#endif
#ifdef CONFIG_RTK_RG_INIT
	RG_WAN_Interface_Del(pEntry->rg_wan_idx);
	check_RG_static_route();
#endif
}

#ifdef PORT_FORWARD_ADVANCE
int config_PFWAdvance( int action_type )
{
	switch( action_type )
	{
	case ACT_START:
		startPFWAdvance();
		break;

	case ACT_RESTART:
		stopPFWAdvance();
		startPFWAdvance();
		break;

	case ACT_STOP:
		stopPFWAdvance();
		break;

	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef CONFIG_USER_RTK_SYSLOG
char *log_severity[] =
{
	"Emergency",
	"Alert",
	"Critical",
	"Error",
	"Warning",
	"Notice",
	"Informational",
	"Debug"
};

#define SLOGDPID  "/var/run/syslogd.pid"
#define KLOGDPID  "/var/run/klogd.pid"
#define SLOGDLINK1 "/var/tmp/messages"
#define SLOGDLINK2 "/var/tmp/messages.old"

static int stopSlogD(void)
{
	int slogDid=0;
	int status=0;

	slogDid = read_pid((char*)SLOGDPID);
	if(slogDid > 0) {
		kill(slogDid, 9);
		unlink(SLOGDPID);
		unlink(SLOGDLINK1);
		unlink(SLOGDLINK2);
	}
	return 1;

}

static int stopKlogD(void)
{
	int klogDid=0;
	int status=0;

	klogDid = read_pid((char*)KLOGDPID);
	if(klogDid > 0) {
		kill(klogDid, 9);
		unlink(KLOGDPID);
	}
	return 1;

}

int stopLog(void)
{

	// Kill SlogD
	stopSlogD();
#ifdef USE_BUSYBOX_KLOGD
	// Kill KlogD
	stopKlogD();
#endif

	return 1;
}

int startLog(void)
{
	unsigned char vChar;
	unsigned int vInt;
	char *argv[30], loglen[8], loglevel[2];
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
	char serverip[30], serverport[6];
#endif
	int idx;

	mib_get(MIB_MAXLOGLEN, &vInt);
	snprintf(loglen, sizeof(loglen), "%u", vInt);
	mib_get(MIB_SYSLOG_LOG_LEVEL, &vChar);
	snprintf(loglevel, sizeof(loglevel), "%hhu", vChar);
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
	mib_get(MIB_SYSLOG_MODE, &vChar);
#endif
	argv[1] = "-n";
	argv[2] = "-s";
	argv[3] = loglen;
	argv[4] = "-l";
	argv[5] = loglevel;
	idx = 6;
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
	/* 1: Local, 2: Remote, 3: Both */
	/* Local or Both */
	if (vChar & 1)
		argv[idx++] = "-L";
	/* Remote or Both */
	if (vChar & 2) {
		getMIB2Str(MIB_SYSLOG_SERVER_IP, serverip);
		getMIB2Str(MIB_SYSLOG_SERVER_PORT, serverport);
		snprintf(serverip, sizeof(serverip), "%s:%s", serverip, serverport);
		argv[idx++] = "-R";
		argv[idx++] = serverip;
	}
#endif
#ifdef BB_FEATURE_SAVE_LOG
	argv[idx++] = "-w";
#endif
	argv[idx] = NULL;

	mib_get(MIB_ADSL_DEBUG, &vChar);
	if(vChar==1)
		va_cmd("/bin/adslctrl",2,0,"debug","10");

	mib_get(MIB_SYSLOG, (void *)&vChar);
	if(vChar==1){
		TRACE(STA_SCRIPT, "/bin/slogd %s %s %s %s %s ...\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
		printf("/bin/slogd %s %s %s %s %s ...\n", argv[1], argv[2], argv[3], argv[4], argv[5]);
		do_cmd("/bin/slogd", argv, 0);
#ifdef USE_BUSYBOX_KLOGD
		va_cmd("/bin/klogd",1,0,"-n");
#endif
	}


	return 1;
}
#endif
#if defined(BB_FEATURE_SAVE_LOG) || defined(CONFIG_USER_RTK_SYSLOG)
void writeLogFileHeader(FILE * fp)
{
	unsigned char dev_id_buf[256], log_ip[INET_ADDRSTRLEN];
	unsigned char *bufptr;
/*star:20081017 modify for some macro is changed to mib entry*/
#ifdef _CWMP_MIB_
	unsigned char ipAddr[IP_ADDR_LEN];
#ifdef E8B_GET_OUI
	getOUIfromMAC(dev_id_buf);
#else
	strcpy(dev_id_buf, DEF_MANUFACTUREROUI_STR);
#endif
	bufptr = dev_id_buf + strlen(dev_id_buf);
	*bufptr = '-';
	bufptr++;
	mib_get(MIB_HW_SERIAL_NUMBER, bufptr);
	mib_get(MIB_ADSL_LAN_IP, ipAddr);
	strcpy(log_ip, inet_ntoa(*(struct in_addr *)ipAddr));
#else
	strcpy(dev_id_buf, "devId");
	strcpy(log_ip, "192.168.1.1");
#endif
	fprintf(fp,
		"Manufacturer:%s;\nProductClass:%s;\nSerialNumber:%s;\nIP:%s;\nHWVer:%s;\nSWVer:%s;\n\n",
		DEF_MANUFACTURER_STR, DEVICE_MODEL_STR, dev_id_buf, log_ip,
		HARDWARE_VERSION_STR, SOFTWARE_VERSION_STR);
}
#endif

#ifdef DEFAULT_GATEWAY_V2
int ifExistedDGW(void)
{
	char buff[256];
	int flgs;
	struct in_addr dest, mask;
	FILE *fp;
	if (!(fp = fopen("/proc/net/route", "r"))) {
		printf("Error: cannot open /proc/net/route - continuing...\n");
		return -1;
	}
	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (sscanf(buff, "%*s%x%*x%x%*d%*d%*d%x", &dest, &flgs, &mask) != 3) {
			printf("Unsuported kernel route format\n");
			fclose(fp);
			return -1;
		}
		if ((flgs & RTF_UP) && dest.s_addr == 0 && mask.s_addr == 0) {
			fclose(fp);
			return 1;
		}
	}
	fclose(fp);
	return 0;
}
#endif

// Mason Yu. MLD Proxy
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_ECMH
// MLD proxy configuration
// return value:
// 1  : successful
// 0  : function not enable
// -1 : startup failed
int startMLDproxy()
{
	unsigned char mldproxyEnable=0;
	unsigned int mldproxyItf;
	char ifname[IFNAMSIZ];
	int mldproxy_pid;
	MIB_CE_ATM_VC_T Entry;
	unsigned int entryNum, i;

	// Kill old IGMP proxy
	mldproxy_pid = read_pid((char *)MLDPROXY_PID);
	if (mldproxy_pid >= 1) {
		// kill it
		if (kill(mldproxy_pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'", mldproxy_pid);
		}
	}

	// check if MLD proxy enabled ?
	if (mib_get(MIB_MLD_PROXY_DAEMON, (void *)&mldproxyEnable) != 0)
	{
		if (mldproxyEnable != 1){
			printf("%s(%d) MLD proxy not enable\n",__func__,__LINE__);
			system("/bin/echo 0 > /proc/br_mldproxy");
			return 0;	// MLD proxy not enable
		}
	}
	if(mldproxyEnable)
	{
		printf("%s(%d) MLD proxy enable\n",__func__,__LINE__);
		system("/bin/echo 1 > /proc/br_mldproxy");
	}
	if (mib_get(MIB_MLD_PROXY_EXT_ITF, (void *)&mldproxyItf) != 0)
	{
		if (!ifGetName(mldproxyItf, ifname, sizeof(ifname)))
		{
			printf("Error: MLD proxy interface not set !\n");
			return 0;
		}
	}
#if defined(CONFIG_MCAST_VLAN) && defined(CONFIG_RTK_RG_INIT)
	RTK_RG_ACL_Flush_mVlan();
	RTK_RG_ACL_Add_mVlan();
#endif
	// check for bridge&IPv4 WAN should not start ecmh process
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
			printf("error get atm vc entry\n");
			return -1;
		}
		if (mldproxyItf == Entry.ifIndex ) break;
	}

	if (Entry.IpProtocol == IPVER_IPV4 || Entry.cmode == CHANNEL_MODE_BRIDGE )
	{
		printf("Error: MLD proxy interface is invalid, stop the %s !\n",MLDPROXY);
	} else {
		va_cmd(MLDPROXY, 4, 0, "-i", ifname, "-o", (char *)LANIF);
	}
	return 1;
}
#endif
#endif

#ifdef CONFIG_USER_IGMPPROXY
// IGMP proxy configuration
// return value:
// 1  : successful
// 0  : function not enable
// -1 : startup failed
int startIgmproxy(void)
{
	unsigned char igmpEnable;
	unsigned int igmpItf;
	char ifname[IFNAMSIZ];
	int igmpproxy_pid;

	// Kill old IGMP proxy
	igmpproxy_pid = read_pid((char *)IGMPPROXY_PID);
	if (igmpproxy_pid >= 1) {
		// kill it
		if (kill(igmpproxy_pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'", igmpproxy_pid);
		}
	}

	// check if IGMP proxy enabled ?
	if (mib_get(MIB_IGMP_PROXY, (void *)&igmpEnable) != 0)
	{
		if (igmpEnable != 1)
			return 0;	// IGMP proxy not enable
	}
	if (mib_get(MIB_IGMP_PROXY_ITF, (void *)&igmpItf) != 0)
	{
		if (!ifGetName(igmpItf, ifname, sizeof(ifname)))
		{
			printf("Error: IGMP proxy interface not set !\n");
			return 0;
		}
	}


	va_cmd(IGMPROXY, 6, 0, "-c","1","-d", (char *)LANIF,"-u",ifname);
	return 1;
}

#ifdef CONFIG_IGMPPROXY_MULTIWAN
int setting_Igmproxy(void)
{
	int igmpproxy_pid;
	MIB_CE_ATM_VC_T Entry;
	unsigned int entryNum, i;
	char igmpproxy_wan[100];
	char igmpproxy_cmd[120];
	int igmpenable =0;
	char ifname[IFNAMSIZ];

	// Kill old IGMP proxy
	igmpproxy_pid = read_pid((char *)IGMPPROXY_PID);
	if (igmpproxy_pid >= 1) {
		// kill it
		if (kill(igmpproxy_pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'", igmpproxy_pid);
		}
	}
	igmpproxy_wan[0]='\0';

	// Mason Yu. IPTV_Intf is set to "" for sar driver
#ifdef CONFIG_IP_NF_UDP
	va_cmd("/bin/sarctl",2,1,"iptv_intf", "");
#endif
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0; i<entryNum; i++)
	{
		 if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		 {
  			printf("error get atm vc entry\n");
			return -1;
		 }

		// check if IGMP proxy enabled ?
		if(Entry.enable && Entry.enableIGMP)
		{
			char iptv_intf_str[10];
			igmpenable =1;
			if (ifGetName(Entry.ifIndex, ifname, sizeof(ifname))) {

				//multiple WAN interfaces, seperated by ','
               if(igmpproxy_wan[0]=='\0')
					snprintf(igmpproxy_wan, 100, "%s",ifname);
               else
					snprintf(igmpproxy_wan, 100, "%s,%s", igmpproxy_wan, ifname);
#ifdef CONFIG_IP_NF_UDP
				va_cmd("/bin/sarctl",2,1,"iptv_intf", ifname);
#endif
			}
			else
			{
				printf("Error: IGMP proxy interface not set !\n");
				return 0;
			}
		}
        }
	if(igmpenable){
		sprintf(igmpproxy_cmd,"%s -c 1 -d br0 -u %s",(char *)IGMPROXY, igmpproxy_wan);
		system(igmpproxy_cmd);
	}

	//Kevin, tell 0412 igmp snooping that the proxy is enabled/disabled
#ifndef CONFIG_RTL8672NIC
	if(igmpenable)
		system("/bin/echo 1 > /proc/br_igmpProxy");
	else
		system("/bin/echo 0 > /proc/br_igmpProxy");
#endif

	// Mason Yu. Kill all session
#ifdef CONFIG_IP_NF_UDP
#ifdef CONFIG_RTL8672_SAR
	itfcfg("sar", 0);
#endif
	va_cmd(IFCONFIG, 2, 1, ELANIF, "down");
	va_cmd(IFCONFIG, 2, 1, BRIF, "down");
	va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
	va_cmd(IFCONFIG, 2, 1, ELANIF, "up");
	va_cmd(IFCONFIG, 2, 1, BRIF, "up");
	itfcfg("sar", 1);

#endif
#if defined(CONFIG_MCAST_VLAN) && defined(CONFIG_RTK_RG_INIT)
	RTK_RG_ACL_Flush_mVlan();
	RTK_RG_ACL_Add_mVlan();
#endif

	return 1;
}
#endif
#endif // of CONFIG_USER_IGMPPROXY

void addStaticRoute(void)
{
	unsigned int entryNum, i;
	MIB_CE_IP_ROUTE_T Entry;
	//struct rtentry rt;
	//struct sockaddr_in *inaddr;
	//char	ifname[17];

	/* Clean out the RTREQ structure. */
	//memset((char *) &rt, 0, sizeof(struct rtentry));
	entryNum = mib_chain_total(MIB_IP_ROUTE_TBL);

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
		{
			return;
		}
#if !defined(CONFIG_LUNA) && !defined(CONFIG_RTK_RG_INIT)
		route_cfg_modify(&Entry, 0, i);
#endif
	}
}

void deleteStaticRoute(void)
{
	unsigned int entryNum, i;
	MIB_CE_IP_ROUTE_T Entry;
	//struct rtentry rt;
	//struct sockaddr_in *inaddr;
	//char	ifname[17];

	/* Clean out the RTREQ structure. */
	//memset((char *) &rt, 0, sizeof(struct rtentry));
	entryNum = mib_chain_total(MIB_IP_ROUTE_TBL);

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
		{
			return;
		}
		route_cfg_modify(&Entry, 1, i);
	}
}

#ifdef CONFIG_IPV6
static void addStaticV6Route()
{
	unsigned int entryNum, i;
	MIB_CE_IPV6_ROUTE_T Entry;

	entryNum = mib_chain_total(MIB_IPV6_ROUTE_TBL);

	for (i=0; i<entryNum; i++)
	{
		if (!mib_chain_get(MIB_IPV6_ROUTE_TBL, i, (void *)&Entry))
			return;
		route_v6_cfg_modify(&Entry, 0);
	}
}

static void deleteV6StaticRoute()
{
	unsigned int entryNum, i;
	MIB_CE_IPV6_ROUTE_T Entry;
	entryNum = mib_chain_total(MIB_IPV6_ROUTE_TBL);

	for (i=0; i<entryNum; i++)
	{
		if (!mib_chain_get(MIB_IPV6_ROUTE_TBL, i, (void *)&Entry))
			return;
		route_v6_cfg_modify(&Entry, 1);
	}
}
#endif

#ifdef CONFIG_USER_PPPOMODEM
static void _wan3g_start_each( MIB_WAN_3G_T *p, unsigned char pppidx)
{
    //printf( "%s: enter\n", __FUNCTION__ );
    //DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

    if(p && p->enable)
    {
	struct data_to_pass_st msg;
	int pppdbg;
	char ifIdx[3], pppif[6];

	snprintf(ifIdx, 3, "%u", pppidx);
	snprintf(pppif, 6, "ppp%u", pppidx);

	if(p->ctype!=MANUAL)
		snprintf(msg.data, BUF_SIZE, "spppctl add %s", ifIdx);
	else
		snprintf(msg.data, BUF_SIZE, "spppctl new %s", ifIdx);

	//set device
	//snprintf(msg.data, BUF_SIZE, "%s pppomodem /dev/ttyUSB0", msg.data);
	snprintf(msg.data, BUF_SIZE, "%s pppomodem auto", msg.data);

	//set pin code
	if( p->pin!=NO_PINCODE )
		snprintf(msg.data, BUF_SIZE, "%s simpin %04u", msg.data, p->pin);

	//set apn
	if( strlen(p->apn) )
		snprintf(msg.data, BUF_SIZE, "%s apn %s", msg.data, p->apn);

	//set dial
	snprintf(msg.data, BUF_SIZE, "%s dial %s", msg.data, p->dial);

	// Set Authentication Method
	//printf( "p->auth=%d, %d, %d\n", p->auth, PPP_AUTH_NONE, p->auth==PPP_AUTH_NONE );
	if(p->auth==PPP_AUTH_NONE)
	{
		//skip or???
	}else{
		snprintf(msg.data, BUF_SIZE, "%s auth %s", msg.data, ppp_auth[p->auth]);
		snprintf(msg.data, BUF_SIZE, "%s username %s password %s", msg.data, p->username, p->password);
	}

	//set default gateway/ mtu
	snprintf(msg.data, BUF_SIZE, "%s gw %d mru %u",	msg.data, p->dgw, p->mtu);

	// set PPP debug
	pppdbg = pppdbg_get(pppidx);
	snprintf(msg.data, BUF_SIZE, "%s debug %d", msg.data, pppdbg);
	//snprintf(msg.data, BUF_SIZE, "%s debug 1", msg.data);

	//paula, set 3g backup PPP
	snprintf(msg.data, BUF_SIZE, "%s backup %d", msg.data, p->backup);

	if(p->backup)
		snprintf(msg.data, BUF_SIZE, "%s backup_timer %d", msg.data, p->backup_timer);

	if(p->ctype == CONTINUOUS) // Continuous
	{
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		//printf("\ncmd=%s\n",msg.data);
		write_to_pppd(&msg);

		// set the ppp keepalive timeout
		snprintf(msg.data, BUF_SIZE,"spppctl katimer 100");
		TRACE(STA_SCRIPT, "%s\n", msg.data);

		write_to_pppd(&msg);
		printf("PPP Connection (Continuous)...\n");
	}else if(p->ctype==CONNECT_ON_DEMAND) // On-demand
	{
		snprintf(msg.data, BUF_SIZE, "%s timeout %u", msg.data, p->idletime);
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
		printf("PPP Connection (On-demand)...\n");
	}
	else if(p->ctype==MANUAL) // Manual
	{
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
		printf("PPP Connection (Manual)...\n");
	}

	if(p->napt)
	{	// Enable NAPT
		va_cmd(IPTABLES, 8, 1, "-t", "nat", FW_ADD, "POSTROUTING",
			"-o", pppif, "-j", "MASQUERADE");
	}

    }

    return;
}

static void _wan3g_stop_each( MIB_WAN_3G_T *p, unsigned char pppidx)
{
    //printf( "%s: enter\n", __FUNCTION__ );
    //DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

    if(p && p->enable)
    {
	struct data_to_pass_st msg;
    	char pppif[6];

	snprintf(pppif, 6, "ppp%u", pppidx);
	// spppctl del 0
	snprintf(msg.data, BUF_SIZE, "spppctl del %u", pppidx);
	TRACE(STA_SCRIPT, "%s\n", msg.data);
	write_to_pppd(&msg);

	//down interface
	va_cmd(IFCONFIG, 2, 1, pppif, "down");

	if(p->napt==1)
	{	// Disable NAPT
		va_cmd(IPTABLES, 8, 1, "-t", "nat", FW_DEL, "POSTROUTING",
			"-o", pppif, "-j", "MASQUERADE");
	}
    }

    return;
}

static MIB_WAN_3G_T mib_wan_3g_default_table[] = {
	{
	 /*enable */ 0,
	 /*auth */ PPP_AUTH_NONE,
	 /*ctype */ CONTINUOUS,
	 /*napt */ 1,
	 /*pin */ NO_PINCODE,
	 /*idletime */ 60,
	 /*mtu */ 1500,
	 /*dgw */ 1,
	 /*apn */ "internet",
	 /*dial */ "*99#",
	 /*username */ "",
	 /*password */ "",
	 //paula, 3g backup PPP
	 /*backup */ 0,
	 /*backup timer */ 60
	 }
};

/*
 *	Return value:
 *		0: disabled
 *		1: enabled
 */

int wan3g_start(void)
{
	unsigned char pppidx;
	MIB_WAN_3G_T entry, *p = &entry;

	pppidx = 0;
	if (!mib_chain_get(MIB_WAN_3G_TBL, pppidx, (void *)p)) {
		//printf("No entry in MIB_WAN_3G, add one\n");
		p = &mib_wan_3g_default_table[0];
		mib_chain_add(MIB_WAN_3G_TBL, (void *)p);
	}

	_wan3g_start_each(p, MODEM_PPPIDX_FROM + pppidx);

	if (p->enable) {
		//setup_ipforwarding(1);
		return 1;
	}

	return 0;
}

/*
 *	Return value:
 *		0: disabled
 *		1: enabled
 */
int wan3g_enable(void)
{
	unsigned char pppidx;
	MIB_WAN_3G_T entry, *p = &entry;

	pppidx = 0;
	if (!mib_chain_get(MIB_WAN_3G_TBL, pppidx, (void *)p)) {
		//printf("No entry in MIB_WAN_3G, add one\n");
		p = &mib_wan_3g_default_table[0];
		mib_chain_add(MIB_WAN_3G_TBL, (void *)p);
	}

	if (p->enable) {
		//setup_ipforwarding(1);
		return 1;
	}

	return 0;
}

void wan3g_stop(void)
{
	unsigned char pppidx;
	MIB_WAN_3G_T entry, *p = &entry;

	pppidx = 0;
	if (!mib_chain_get(MIB_WAN_3G_TBL, pppidx, p)) {
		mib_chain_add(MIB_WAN_3G_TBL, mib_wan_3g_default_table);
	}

	_wan3g_stop_each(p, MODEM_PPPIDX_FROM + pppidx);

	return;
}
#endif //CONFIG_USER_PPPOMODEM

// Added by Mason Yu
// configAll = CONFIGALL,  pEntry = NULL  : delete all WAN connections(include VC, ETHWAN, PTMWAN, VPN, 3g).
//									  It means that we want to restart all WAN channel.
// configAll = CONFIGONE, pEntry != NULL : delete specified VC, ETHWAN, PTMWAN connection and VPN, 3g connections.
// 									  It means that we delete or modify an old VC, ETHWAN, PTMWAN channel.
// configAll = CONFIGONE, pEntry = NULL  : delete VPN, 3g connections. It means that we add a new VC, ETHWAN, PTMWAN channel.
int deleteConnection(int configAll, MIB_CE_ATM_VC_Tp pEntry)
{
	unsigned int entryNum, i, idx;
	MIB_CE_ATM_VC_T Entry;

	//close all tunnel before delete wan interface.
#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	MIB_PPTP_T pptp;

	entryNum = mib_chain_total(MIB_PPTP_TBL);
	for (i=0; i<entryNum; i++)
	{
		if ( !mib_chain_get(MIB_PPTP_TBL, i, (void *)&pptp) )
			return;

		if(configAll == CONFIGALL)
		{
			applyPPtP(&pptp, 0, i);
		}
		else if(configAll == CONFIGONE)
		{
			 if(pEntry && pptp.ifIndex == pEntry->ifIndex)
			 {
				applyPPtP(&pptp, 0, i);
				break;
			 }
		}
	}
#endif
#ifdef CONFIG_USER_L2TPD_L2TPD
	MIB_L2TP_T l2tp;
	entryNum = mib_chain_total(MIB_L2TP_TBL);
	for (i=0; i<entryNum; i++)
	{
		if ( !mib_chain_get(MIB_L2TP_TBL, i, (void *)&l2tp) )
			return;

		if(configAll == CONFIGALL)
		{
			applyL2TP(&l2tp, 0, i);
		}
		else if(configAll == CONFIGONE)
		{
			 if(pEntry && l2tp.ifIndex == pEntry->ifIndex)
			 {
				applyL2TP(&l2tp, 0, i);
				break;
			 }
		}
	}
#endif

		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i = 0; i < entryNum; i++) {
			/* Retrieve entry */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
				printf("deleteConnection: cannot get ATM_VC_TBL(ch=%d) entry\n", i);
				return 0;
			}

		if (configAll == CONFIGALL)
		{
			if(Entry.enable) {
				/* remove connection on driver*/
				stopConnection(&Entry);
			}
		}
	else if (configAll == CONFIGONE) {
			if(pEntry && Entry.ifIndex == pEntry->ifIndex) {
				if(Entry.enable) {
				/* remove connection on driver*/
					stopConnection(&Entry);
					break;
				}
			}
		}
	}

#ifdef CONFIG_USER_PPPOMODEM
	if((configAll == CONFIGALL) || (pEntry && (TO_IFINDEX(MEDIA_3G,  MODEM_PPPIDX_FROM, 0) == pEntry->ifIndex)))
	{
	wan3g_stop();
	}
#endif //CONFIG_USER_PPPOMODEM
}

static void cleanAllFirewallRule()
{
	// Added by Mason Yu. Clean all Firewall rules.
	va_cmd(IPTABLES, 1, 1, "-F");
	va_cmd(EBTABLES, 1, 1, "-F");
	va_cmd(IPTABLES, 3, 1, "-t", "nat", "-F");
#ifdef CONFIG_IPV6
	va_cmd(IP6TABLES, 1, 1, "-F");
#endif
#ifdef IP_ACL
	va_cmd(IPTABLES, 2, 1, "-X", "aclblock");
#endif
#if defined(NAT_CONN_LIMIT) || defined(TCP_UDP_CONN_LIMIT)
	va_cmd(IPTABLES, 2, 1, "-X", "connlimit");
#endif

	va_cmd(EBTABLES, 2, 1, "-X", (char *)FW_BR_WAN);
	va_cmd(EBTABLES, 2, 1, "-X", (char *)FW_BR_WAN_OUT);

#ifdef PPPOE_PASSTHROUGH
	va_cmd(EBTABLES, 6, 1, "-t", "broute", "-D", "BROUTING", "-j", (char *)FW_BR_PPPOE);
	va_cmd(EBTABLES, 4, 1, "-t", "broute", "-X", (char *)FW_BR_PPPOE);
	va_cmd(EBTABLES, 2, 1, "-X", (char *)FW_BR_PPPOE);
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
	va_cmd(IPTABLES, 2, 1, "-X", "domainblk");
#endif
#ifdef PORT_FORWARD_GENERAL
	va_cmd(IPTABLES, 2, 1, "-X", (char *)PORT_FW);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)PORT_FW);
#endif

	va_cmd(IPTABLES, 2, 1, "-X", (char *)IPTABLE_DMZ);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)IPTABLE_DMZ);

#ifdef NATIP_FORWARDING
	va_cmd(IPTABLES, 2, 1, "-X", (char *)IPTABLE_IPFW);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)IPTABLE_IPFW);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)IPTABLE_IPFW2);
#endif

#ifdef PORT_FORWARD_ADVANCE
	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_PPTP);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)FW_PPTP);
	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_L2TP);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)FW_L2TP);
#endif

#ifdef PORT_TRIGGERING
	va_cmd(IPTABLES, 2, 1, "-X", (char *)IPTABLES_PORTTRIGGER);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)IPTABLES_PORTTRIGGER);
#endif

#ifdef REMOTE_ACCESS_CTL
	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_INACC);
#endif
	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_IPFILTER);
#ifdef CONFIG_IPV6
	va_cmd(IP6TABLES, 2, 1, "-X", (char *)FW_IPV6FILTER);
	va_cmd(IP6TABLES, 2, 1, "-X", (char *)FW_IPV6REMOTEACC);
#endif

#ifdef MAC_FILTER
	//va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_MACFILTER);
#ifndef MAC_FILTER_SRC_ONLY
	va_cmd(EBTABLES, 2, 1, "-X", (char *)FW_MACFILTER);
#else
	va_cmd(EBTABLES, 2, 1, "-X", (char *)FW_MACFILTER_ROUTER);
#endif
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)FW_MACFILTER_ROUTER);
#endif

#ifdef URL_BLOCKING_SUPPORT
	va_cmd(IPTABLES, 2, 1, "-X", "urlblock");
#endif

#ifdef URL_ALLOWING_SUPPORT
	va_cmd(IPTABLES, 2, 1, "-X", "urlallow");
#endif

#ifdef WEB_REDIRECT_BY_MAC
	va_cmd(IPTABLES, 2, 1, "-t", "nat", "-X", "WebRedirectByMAC");
#endif

#ifdef VIRTUAL_SERVER_SUPPORT
	va_cmd(IPTABLES, 2, 1, "-X", (char *)IPTABLE_VTLSUR);
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", (char *)IPTABLE_VTLSUR);
#endif

#if 0
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	va_cmd(IPTABLES, 2, 1, "-t", "nat", "-X", "CaptivePortal");
#endif
#endif

#ifdef CONFIG_USER_CWMP_TR069
	va_cmd(IPTABLES, 2, 1, "-X", IPTABLE_TR069);
#endif
	filter_set_remote_access(0);

#ifdef CONFIG_USER_FON
	va_cmd(IPTABLES, 2, 1, "-X", "fongw");
	va_cmd(IPTABLES, 2, 1, "-X", "fongw_fwd");
#endif

#ifdef SUPPORT_ACCESS_RIGHT
	apply_accessRight(0);
#endif
#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
	apply_maxBandwidth(0);
#endif

}

static int setupIPFilter()
{
	char *argv[20];
	unsigned char value[32], byte;
	int vInt, i, total;
	MIB_CE_IP_PORT_FILTER_T IpEntry;
	char *policy, *filterSIP, *filterDIP, srcPortRange[12], dstPortRange[12];
	char  srcip[20], dstip[20];
#ifdef IP_RANGE_FILTER_SUPPORT
	// jim support ip range options " iptables -m iprange --src-range xx.xx.xx.xx-yy.yy.yy.yy --dst-range a.a.a.a-b.b.b.b ..."
	//char srcip2[20]={0}, dstip2[20]={0};
	char srcip2[55]={0}, dstip2[55]={0};
	//char SIPRange[40]={0};
	char SIPRange[110]={0};
	//char DIPRange[40]={0};
	char DIPRange[110]={0};
	char *filterSIPRange=NULL;
	char *filterDIPRange=NULL;
	filterSIP=filterDIP=NULL;
#endif
	unsigned char ipportfilter_state;

	// Delete ipfilter rule
	va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_IPFILTER);

#ifdef CONFIG_RTK_RG_INIT
	FlushRTK_RG_ACL_Filters();
	RTK_RG_ACL_IPPort_Filter_Allow_LAN_to_GW();
#endif

	// packet filtering
	// ip filtering
	// Add chain for ip filtering
	// iptables -N ipfilter
	//va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_IPFILTER);

	// accept related
	// iptables -A ipfilter -m state --state ESTABLISHED,RELATED -j RETURN
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_IPFILTER, "-m", "state",
		"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_RETURN);
	// iptables -A ipfilter -d 224.0.0.0/4 -j RETURN
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_IPFILTER, "-d",
		"224.0.0.0/4", "-j", (char *)FW_RETURN);

	mib_get(MIB_IPFILTER_ON_OFF, (void*)&ipportfilter_state);

	if(ipportfilter_state == 0) return 0;

	total = mib_chain_total(MIB_IP_PORT_FILTER_TBL);
	for (i = 0; i < total; i++)
	{
		int idx=0;
		int udp_tcp_idx=-1;
		/*
		 *	srcPortRange: src port
		 *	dstPortRange: dst port
		 */
		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, i, (void *)&IpEntry))
			return -1;

#ifdef CONFIG_RTK_RG_INIT
#ifdef CONFIG_IPV6
		if(IpEntry.IpProtocol == IPVER_IPV4) //ipv4
#endif
		AddRTK_RG_ACL_IPPort_Filter(&IpEntry);
#endif

		if (IpEntry.action == 0)
			policy = (char *)FW_DROP;
		else
			policy = (char *)FW_RETURN;

		// source port
		if (IpEntry.srcPortFrom == 0)
			srcPortRange[0]='\0';
		else if (IpEntry.srcPortTo == 0)
			snprintf(srcPortRange, 12, "%u", IpEntry.srcPortFrom);
		else
			snprintf(srcPortRange, 12, "%u:%u",
				IpEntry.srcPortFrom, IpEntry.srcPortTo);

		// destination port
		if (IpEntry.dstPortFrom == 0)
			dstPortRange[0]='\0';
		else if (IpEntry.dstPortTo == 0)
			snprintf(dstPortRange, 12, "%u", IpEntry.dstPortFrom);
		else
			snprintf(dstPortRange, 12, "%u:%u",
				IpEntry.dstPortFrom, IpEntry.dstPortTo);

		// source ip, mask
	#ifdef IP_RANGE_FILTER_SUPPORT
		if(IpEntry.srcIp2[0] == 0)    // normal ip filter, no iprange supported
		{
	#endif
			strncpy(srcip, inet_ntoa(*((struct in_addr *)IpEntry.srcIp)), 16);
			if (strcmp(srcip, ARG_0x4) == 0)
				filterSIP = 0;
			else {
				if (IpEntry.smaskbit!=0)
					snprintf(srcip, 20, "%s/%d", srcip, IpEntry.smaskbit);
				filterSIP = srcip;
			}
	#ifdef IP_RANGE_FILTER_SUPPORT
		}
		else
		{
			strncpy(srcip, inet_ntoa(*((struct in_addr *)IpEntry.srcIp)), 16);
			strncpy(srcip2, inet_ntoa(*((struct in_addr *)IpEntry.srcIp2)),16);

			if(strcmp(srcip, ARG_0x4) ==0 || strcmp(srcip2, ARG_0x4) ==0)
				filterSIPRange=0;
			else
			{
				snprintf(SIPRange, 40, "%s-%s", srcip, srcip2);
				filterSIPRange=SIPRange;
			}
		}
	#endif

		// destination ip, mask
	#ifdef IP_RANGE_FILTER_SUPPORT
		if(IpEntry.dstIp2[0] == 0)    // normal ip filter, no iprange supported
		{
	#endif
			strncpy(dstip, inet_ntoa(*((struct in_addr *)IpEntry.dstIp)), 16);
			if (strcmp(dstip, ARG_0x4) == 0)
				filterDIP = 0;
			else {
				if (IpEntry.dmaskbit!=0)
					snprintf(dstip, 20, "%s/%d", dstip, IpEntry.dmaskbit);
				filterDIP = dstip;
			}
	#ifdef IP_RANGE_FILTER_SUPPORT
		}
		else
		{
			strncpy(dstip, inet_ntoa(*((struct in_addr *)IpEntry.dstIp)), 16);
			strncpy(dstip2, inet_ntoa(*((struct in_addr *)IpEntry.dstIp2)),16);

			if(strcmp(dstip, ARG_0x4) ==0 || strcmp(dstip2, ARG_0x4) ==0)
				filterDIPRange=0;
			else
			{
				snprintf(DIPRange, 40, "%s-%s", dstip, dstip2);
				filterDIPRange=DIPRange;
			}
		}
	#endif

		// interface
		argv[1] = (char *)FW_ADD;
		argv[2] = (char *)FW_IPFILTER;
		argv[3] = (char *)ARG_I;

		if (IpEntry.dir == DIR_OUT) {
			argv[4] = (char *)LANIF;
			idx = 5;
		}
		else {
			argv[4] = "!";
			argv[5] = (char *)LANIF;
			idx = 6;
		}

		// protocol
		if (IpEntry.protoType != PROTO_NONE) {
			argv[idx++] = "-p";
			if (IpEntry.protoType == PROTO_TCP)
				argv[idx++] = (char *)ARG_TCP;
			else if (IpEntry.protoType == PROTO_UDP)
				argv[idx++] = (char *)ARG_UDP;
			else if (IpEntry.protoType == PROTO_ICMP)
				argv[idx++] = (char *)ARG_ICMP;
			else if(IpEntry.protoType == PROTO_UDPTCP){
				//add udp rule first for udp/tcp protocol
				udp_tcp_idx = idx;
				argv[idx++] = (char *)ARG_UDP;
			}
		}

		// src ip
		if (filterSIP != 0)
		{
			argv[idx++] = "-s";
			argv[idx++] = filterSIP;
		}

		// src port
		if ((IpEntry.protoType==PROTO_TCP ||
			IpEntry.protoType==PROTO_UDP ||
			IpEntry.protoType==PROTO_UDPTCP) &&
			srcPortRange[0] != 0) {
			argv[idx++] = (char *)FW_SPORT;
			argv[idx++] = srcPortRange;
		}

		// dst ip
		if (filterDIP != 0)
		{
			argv[idx++] = "-d";
			argv[idx++] = filterDIP;
		}
		// dst port
		if ((IpEntry.protoType==PROTO_TCP ||
			IpEntry.protoType==PROTO_UDP ||
			IpEntry.protoType==PROTO_UDPTCP) &&
			dstPortRange[0] != 0) {
			argv[idx++] = (char *)FW_DPORT;
			argv[idx++] = dstPortRange;
		}

#ifdef IP_RANGE_FILTER_SUPPORT
	if(filterSIPRange || filterDIPRange)
	{
		argv[idx++] = strdup("-m");
		argv[idx++] = strdup("iprange");
		if(filterSIPRange)
		{
			argv[idx++] = strdup("--src-range");
			argv[idx++] = strdup(filterSIPRange);
		}
		if(filterDIPRange)
		{
			argv[idx++] = strdup("--dst-range");
			argv[idx++] = strdup(filterDIPRange);
		}
	}
#endif

		// target/jump
		argv[idx++] = "-j";
		argv[idx++] = policy;
		argv[idx++] = NULL;

		/* Debug printing.
		idx = 1;
		printf("iptables ");
		while(argv[idx])
		{
			printf("%s ", argv[idx++]);
		}
		putchar(10);
		/* End debug printing. */

		do_cmd(IPTABLES, argv, 1);
		if(udp_tcp_idx!=-1){
			//add tcp rule for udp/tcp protocol
			argv[udp_tcp_idx] = (char *)ARG_TCP;
			do_cmd(IPTABLES, argv, 1);
		}
	}

//#ifdef  CONFIG_USER_PPPOE_PROXY
#if 0
    char ppp_proxyif[6];
	char ppp_proxynum[2];
	int proxy_index;

	for(proxy_index = WAN_PPP_INTERFACE; proxy_index< WAN_PPP_INTERFACE+LAN_PPP_INTERFACE;proxy_index++){
		snprintf(ppp_proxyif, 6, "ppp%u", proxy_index);
		va_cmd (IPTABLES, 6, 1, (char *) FW_ADD, (char *) FW_IPFILTER, (char *)
           ARG_I, ppp_proxyif, "-j", (char *) FW_RETURN);
		}
#endif

	// allow DMZ to pass
	// Mason Yu
	#if 0
	if ((mib_get(MIB_DMZ_ENABLE, (void *)&byte) != 0) && byte) {
		if (mib_get(MIB_DMZ_IP, (void *)value) != 0)
		{
			char *ipaddr = srcip;
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
			ipaddr[15] = '\0';

			total = mib_chain_total (MIB_IP_PORT_FILTER_TBL) + 3;
			snprintf(value, 8, "%d", total);
			// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -d $DMZ_IP -j RETURN
			va_cmd(IPTABLES, 12, 1, "-I",
				(char *)FW_IPFILTER, value, (char *)ARG_I, "!",
				(char *)LANIF, (char *)ARG_O,
				(char *)LANIF, "-d", ipaddr, "-j", (char *)FW_RETURN);
		}
	}
	#endif
	// iptables -A FORWARD -j ipfilter
	//va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_IPFILTER);

	// Kill all conntrack (to kill the established conntrack when change iptables rules)
	va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
}

static int block_br_wan()
{
	int total;
	int i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ] = {0};

	// INPUT
	va_cmd(EBTABLES, 2, 1, "-N", FW_BR_WAN);
	va_cmd(EBTABLES, 3, 1, "-P", FW_BR_WAN, "RETURN");
	// OUTPUT
	va_cmd(EBTABLES, 2, 1, "-N", FW_BR_WAN_OUT);
	va_cmd(EBTABLES, 3, 1, "-P", FW_BR_WAN_OUT, "RETURN");

	// check all wan interfaces
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;

		if(!Entry.enable)
			continue;

		ifGetName(PHY_INTF(Entry.ifIndex), ifname, sizeof(ifname));

		if(Entry.cmode == CHANNEL_MODE_BRIDGE)
		{
			va_cmd(EBTABLES, 6, 1, "-A", FW_BR_WAN, "-i", ifname, "-j", "DROP");
			va_cmd(EBTABLES, 6, 1, "-A", FW_BR_WAN_OUT, "-o", ifname, "-j", "DROP");
		}
	}

	// add to INPUT chain
	va_cmd(EBTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", FW_BR_WAN);
	// add to OUTPUT chain
	va_cmd(EBTABLES, 4, 1, (char *)FW_ADD, (char *)FW_OUTPUT, "-j", FW_BR_WAN_OUT);

	return 0;
}

#ifdef PPPOE_PASSTHROUGH
static int setupBrPppoe(void)
{
	int total;
	int i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ] = {0};

	// check all wan interfaces
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;

		if(!Entry.enable)
			continue;

		ifGetName(PHY_INTF(Entry.ifIndex), ifname, sizeof(ifname));

		if (Entry.brmode != BRIDGE_DISABLE)
		{
			struct sockaddr hwaddr;
			char mac_str[20]={0};
			getInAddr(ifname, HW_ADDR, &hwaddr);
			sprintf( mac_str, "%02X:%02X:%02X:%02X:%02X:%02X",
				(unsigned char)hwaddr.sa_data[0], (unsigned char)hwaddr.sa_data[1], (unsigned char)hwaddr.sa_data[2],
				(unsigned char)hwaddr.sa_data[3], (unsigned char)hwaddr.sa_data[4], (unsigned char)hwaddr.sa_data[5] );

			// route packets if the packet is from WAN and the destination is for myself
			//eg.: ebtables -t broute -A BROUTING -i vc0 -d 00:E0:4C:86:70:02 -j DROP
			va_cmd(EBTABLES, 10, 1, "-t", "broute", "-A", FW_BR_PPPOE, "-i", ifname,
				"-d", mac_str, "-j", "DROP");
		}

		if (Entry.brmode == BRIDGE_PPPOE)
		{
			char vid[10] = {0};

#if defined(CONFIG_RTL_MULTI_WAN) || defined(CONFIG_RTL_MULTI_PVC_WAN)
			//bridge PPPoE session/discover packets on this WAN interface
			if(Entry.vlan)
			{
				sprintf(vid, "%u", Entry.vid);

				va_cmd(EBTABLES, 10, 1, "-A", FW_BR_PPPOE, "-i", ifname,
				"--proto", "0x8100", "--vlan-encap", "0x8863", "-j", "RETURN");

				va_cmd(EBTABLES, 10, 1, "-A", FW_BR_PPPOE, "-i", ifname,
				"--proto", "0x8100", "--vlan-encap", "0x8864", "-j", "RETURN");
			}
			else
#endif
			{
				va_cmd(EBTABLES, 8, 1, "-A", FW_BR_PPPOE, "-i", ifname,
				"--proto", "0x8863", "-j", "RETURN");
				va_cmd(EBTABLES, 8, 1, "-A", FW_BR_PPPOE, "-i", ifname,
				"--proto", "0x8864", "-j", "RETURN");
			}

			va_cmd(EBTABLES, 8, 1, "-A", FW_BR_PPPOE, "-o", ifname,
				"--proto", "0x8863", "-j", "RETURN");
			va_cmd(EBTABLES, 8, 1, "-A", FW_BR_PPPOE, "-o", ifname,
				"--proto", "0x8864", "-j", "RETURN");

			//drop other packets on this WAN interface
			va_cmd(EBTABLES, 6, 1, "-A", FW_BR_PPPOE, "-i", ifname,
				"-j", "DROP");
			va_cmd(EBTABLES, 6, 1, "-A", FW_BR_PPPOE, "-o", ifname,
				"-j", "DROP");
		}
	}

	return 0;
}
#endif

int setupMacFilter(void)
{
	int i, total;
	char *policy;
	char srcmacaddr[18], dstmacaddr[18];
	char smacParm[64]={0};
	char dmacParm[64]={0};
#ifdef MAC_FILTER
	MIB_CE_MAC_FILTER_T MacEntry;
	char mac_in_dft, mac_out_dft;
	char eth_mac_ctrl=0, wlan_mac_ctrl=0;
#endif

	DOCMDINIT;

#ifdef CONFIG_HWNAT
	int mac_filter_in_permit  = 1;
	int mac_filter_out_permit = 1;
#endif

	// Delete all Macfilter rule
	va_cmd(EBTABLES, 2, 1, "-F", (char *)FW_MACFILTER);
#ifdef CONFIG_RTK_RG_INIT
	FlushRTK_RG_MAC_Filters();
#endif
#ifdef PARENTAL_CTRL
	parent_ctrl_table_init();
	parent_ctrl_table_rule_update();
#endif
#ifdef MAC_FILTER
	mib_get(MIB_MACF_OUT_ACTION, (void *)&mac_out_dft);
	mib_get(MIB_MACF_IN_ACTION, (void *)&mac_in_dft);
	mib_get(MIB_ETH_MAC_CTRL, (void *)&eth_mac_ctrl);
	mib_get(MIB_WLAN_MAC_CTRL, (void *)&wlan_mac_ctrl);
#endif

#ifdef MAC_FILTER
	total = mib_chain_total(MIB_MAC_FILTER_TBL);

	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_MAC_FILTER_TBL, i, (void *)&MacEntry))
			return -1;

#ifdef CONFIG_RTK_RG_INIT
		AddRTK_RG_MAC_Filter(&MacEntry);
		continue;
#endif

		smacParm[0]=dmacParm[0]='\0';

		if (MacEntry.action == 0)
			policy = (char *)FW_DROP;
		else
			policy = (char *)FW_RETURN;

		if(memcmp(MacEntry.srcMac,"\x00\x00\x00\x00\x00\x00",MAC_ADDR_LEN)) {
			snprintf(srcmacaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				MacEntry.srcMac[0], MacEntry.srcMac[1],
				MacEntry.srcMac[2], MacEntry.srcMac[3],
				MacEntry.srcMac[4], MacEntry.srcMac[5]);
			snprintf(smacParm, sizeof(smacParm),"-s %s", srcmacaddr);
		}

		if(memcmp(MacEntry.dstMac,"\x00\x00\x00\x00\x00\x00",MAC_ADDR_LEN)) {
			snprintf(dstmacaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				MacEntry.dstMac[0], MacEntry.dstMac[1],
				MacEntry.dstMac[2], MacEntry.dstMac[3],
				MacEntry.dstMac[4], MacEntry.dstMac[5]);
			snprintf(dmacParm, sizeof(dmacParm),"-d %s", dstmacaddr);
		}

		if (MacEntry.dir == DIR_OUT) {
			if (!strlen(dmacParm)) { // compatible with TR069 -- smac outgoing
				if(mac_out_dft) {
					// ebtables -A macfilter -i eth0+  -s 00:xx:..:xx -j ACCEPT/DROP
					DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ %s %s -j %s", (char *)FW_MACFILTER,
						(char *)ELANIF,smacParm,dmacParm,policy);
#ifdef WLAN_SUPPORT
					// ebtables -A macfilter -i wlan0+  -s 00:xx:..:xx -j ACCEPT/DROP
					DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ %s %s -j %s", (char *)FW_MACFILTER,
						(char *)WLANIF[0],smacParm,dmacParm,policy);
#endif
				}
				else {
					if (eth_mac_ctrl) {
						// ebtables -A macfilter -i eth0+  -s 00:xx:..:xx -j ACCEPT/DROP
						DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ %s %s -j %s", (char *)FW_MACFILTER,
							(char *)ELANIF,smacParm,dmacParm,policy);
					}
					if (wlan_mac_ctrl) {
#ifdef WLAN_SUPPORT
						// ebtables -A macfilter -i wlan0+  -s 00:xx:..:xx -j ACCEPT/DROP
						DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ %s %s -j %s", (char *)FW_MACFILTER,
							(char *)WLANIF[0],smacParm,dmacParm,policy);
#endif
					}
				}
			}
			else {
				// ebtables -A macfilter -i eth0+  -s 00:xx:..:xx -j ACCEPT/DROP
				DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ %s %s -j %s", (char *)FW_MACFILTER,
					(char *)ELANIF, smacParm,dmacParm,policy);
#ifdef WLAN_SUPPORT
				// ebtables -A macfilter -i wlan0+  -s 00:xx:..:xx -j ACCEPT/DROP
				DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ %s %s -j %s", (char *)FW_MACFILTER,
					(char *)WLANIF[0],smacParm,dmacParm,policy);
#endif
			}
		}
		else { // DIR_IN
#ifdef CONFIG_RTL8672_SAR
			// ebtables -A macfilter -i vc+  -s 00:xx:..:xx -j ACCEPT/DROP
			DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i vc+ %s %s -j %s", (char *)FW_MACFILTER,
				smacParm,dmacParm,policy);
#endif
#ifdef CONFIG_ETHWAN
			// ebtables -A macfilter -i nas0+  -s 00:xx:..:xx -j ACCEPT/DROP
			DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ %s %s -j %s", (char *)FW_MACFILTER, ALIASNAME_NAS0,
				smacParm,dmacParm,policy);
#endif
#ifdef CONFIG_PTMWAN
			// ebtables -A macfilter -i ptm0+  -s 00:xx:..:xx -j ACCEPT/DROP
			DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ %s %s -j %s", (char *)FW_MACFILTER, ALIASNAME_PTM0,
				smacParm,dmacParm,policy);
#endif
		}
	}

#ifdef CONFIG_RTK_RG_INIT
	//RTK_RG_MAC_Filter_Default_Policy(mac_out_dft, mac_in_dft);
	return 0;
#endif

	// default action
	DOCMDARGVS(EBTABLES,DOWAIT,"-P %s RETURN", (char *)FW_MACFILTER);
	if(mac_out_dft == 0) { // DROP
#ifdef CONFIG_HWNAT
		mac_filter_out_permit = 0;
#endif
		if (eth_mac_ctrl) {
			// ebtables -A macfilter -i eth0+ -j DROP
			DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ -j DROP", (char *)FW_MACFILTER, (char *)ELANIF);
		}
#ifdef WLAN_SUPPORT
		if (wlan_mac_ctrl) {
			// ebtables -A macfilter -i wlan0+ -j DROP
			DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ -j DROP", (char *)FW_MACFILTER, (char *)WLANIF[0]);
		}
#endif
	}
	if(mac_in_dft == 0) { // DROP
#ifdef CONFIG_HWNAT
		mac_filter_in_permit = 0;
#endif
#ifdef CONFIG_RTL8672_SAR
		// ebtables -A macfilter -i vc+ -j DROP
		DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i vc+ -j DROP", (char *)FW_MACFILTER);
#endif
#ifdef CONFIG_ETHWAN
		// ebtables -A macfilter -i nas0+ -j DROP
		DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ -j DROP", (char *)FW_MACFILTER, ALIASNAME_NAS0);
#endif
#ifdef CONFIG_PTMWAN
		// ebtables -A macfilter -i ptm0+ -j DROP
		DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s+ -j DROP", (char *)FW_MACFILTER, ALIASNAME_PTM0);
#endif
	}

#if defined CONFIG_HWNAT && defined CONFIG_RTL8676_Static_ACL
	if( mac_filter_in_permit==1 && mac_filter_out_permit==1 )
	{
		TRACE(STA_SCRIPT,"/bin/echo all_permit> /proc/rtl865x/acl_mac_filter_mode\n");
		system("/bin/echo all_permit> /proc/rtl865x/acl_mac_filter_mode");
	}
	else if( mac_filter_in_permit==0 && mac_filter_out_permit==1 )
	{
		TRACE(STA_SCRIPT,"/bin/echo in_drop_out_permit> /proc/rtl865x/acl_mac_filter_mode\n");
		system("/bin/echo in_drop_out_permit> /proc/rtl865x/acl_mac_filter_mode");
	}
	else if( mac_filter_in_permit==1 && mac_filter_out_permit==0 )
	{
		TRACE(STA_SCRIPT,"/bin/echo in_permit_out_drop> /proc/rtl865x/acl_mac_filter_mode\n");
		system("/bin/echo in_permit_out_drop> /proc/rtl865x/acl_mac_filter_mode");
	}
	else if( mac_filter_in_permit==0 && mac_filter_out_permit==0 )
	{
		TRACE(STA_SCRIPT,"/bin/echo all_drop> /proc/rtl865x/acl_mac_filter_mode\n");
		system("/bin/echo all_drop> /proc/rtl865x/acl_mac_filter_mode");
	}
#endif

#endif // of MAC_FILTER

	//Kevin, clear bridge fastpath table
	TRACE(STA_SCRIPT,"/bin/echo 2 > /proc/fastbridge\n");
	system("/bin/echo 2 > /proc/fastbridge");
#ifdef CONFIG_HWNAT
	TRACE(STA_SCRIPT,"/bin/echo clean_L2 > /proc/rtl865x/acl_mode\n");
	system("/bin/echo clean_L2 > /proc/rtl865x/acl_mode");
#endif

	return 0;
}

#ifdef LAYER7_FILTER_SUPPORT
int setupAppFilter(void)
{
	int entryNum,i;
	LAYER7_FILTER_T Entry;

	// Delete all Appfilter rule
	va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_APPFILTER);
	va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_APP_P2PFILTER);
	va_cmd(IPTABLES, 4, 1, ARG_T, "mangle", "-F", (char *)FW_APPFILTER);

	entryNum = mib_chain_total(MIB_LAYER7_FILTER_TBL);

	for(i=0;i<entryNum;i++)
	{
		if (!mib_chain_get(MIB_LAYER7_FILTER_TBL, i, (void *)&Entry))
			return -1;

		if(!strcmp("smtp",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 25 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "25", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("pop3",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 110 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "110", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("bittorrent",Entry.appname)){
			//iptables -A appp2pfilter -m ipp2p --bit -j DROP
			va_cmd(IPTABLES, 7, 1, (char *)FW_ADD, (char *)FW_APP_P2PFILTER, "-m", "ipp2p",
				"--bit", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("chinagame",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 8000 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "8000", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("gameabc",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 5100 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "5100", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("haofang",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 1201 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "1201", "-j", (char *)FW_DROP);
			continue;
		}
		else if(!strcmp("ourgame",Entry.appname)){
			//iptables -A appfilter -i br0 -p TCP --dport 2000 -j DROP
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_APPFILTER, ARG_I, LANIF,
				"-p", ARG_TCP, FW_DPORT, "2000", "-j", (char *)FW_DROP);
		}


		// iptables -t mangle -A appfilter -m layer7 --l7proto qq -j DROP
		va_cmd(IPTABLES, 10, 1, ARG_T, "mangle", (char *)FW_ADD, (char *)FW_APPFILTER, "-m", "layer7",
			"--l7proto", Entry.appname, "-j", (char *)FW_DROP);

	}

	return 0;
}
#endif
#ifdef PARENTAL_CTRL
//Uses Global variable for keep watching timeout
static MIB_PARENT_CTRL_T parentctrltable[MAX_PARENTCTRL_USER_NUM] = {0};
/********************************
 *
 *	Initialization. load from flash
 *
 ********************************/
//return if this mac should be filtered now!
static int parent_ctrl_check(MIB_PARENT_CTRL_T *entry)
{
	time_t tm;
	struct tm the_tm;
	int		tmp1,tmp2,tmp3;;

	time(&tm);
	memcpy(&the_tm, localtime(&tm), sizeof(the_tm));

	if (((entry->controlled_day) & (1 << the_tm.tm_wday))!=0)
	{
		tmp1 = entry->start_hr * 60 +  entry->start_min;
		tmp2 = entry->end_hr * 60 +  entry->end_min;
		tmp3 = the_tm.tm_hour *60 + the_tm.tm_min;
		if ((tmp3 >= tmp1) && (tmp3 <= tmp2) )
			return 1;
	}

		return 0;

}

int parent_ctrl_table_init(void)
{
	int total,i;
	MIB_PARENT_CTRL_T	entry;

	memset(&parentctrltable[0],0, sizeof(MIB_PARENT_CTRL_T)*MAX_PARENTCTRL_USER_NUM);
	total = mib_chain_total(MIB_PARENTAL_CTRL_TBL);
	if (total >= MAX_PARENTCTRL_USER_NUM)
	{
		total = MAX_PARENTCTRL_USER_NUM -1;
		printf("ERROR, CHECK!");
	}
	 for ( i=0; i<total; i++)
	 {
		mib_chain_get(MIB_PARENTAL_CTRL_TBL, i, &parentctrltable[i]);
	 }
}

int parent_ctrl_table_add(MIB_PARENT_CTRL_T *addedEntry)
{
	int	i;

	for (i = 0; i < MAX_PARENTCTRL_USER_NUM; i++)
	{
		if (strlen(parentctrltable[i].username) == 0)
		{
			break;
		}
	}
	addedEntry->cur_state = 0;
	memcpy (&parentctrltable[i],addedEntry, sizeof(MIB_PARENT_CTRL_T));

	parent_ctrl_table_rule_update();
}

int parent_ctrl_table_del(MIB_PARENT_CTRL_T *addedEntry)
{
	int	i;
	char macaddr[20];

	for (i = 0; i < MAX_PARENTCTRL_USER_NUM; i++)
	{
		if ( !strcmp(parentctrltable[i].username, addedEntry->username))
		{
			if (parentctrltable[i].cur_state == 1)
			{
				//del the entry
					snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
					parentctrltable[i].mac[0], parentctrltable[i].mac[1],
					parentctrltable[i].mac[2], parentctrltable[i].mac[3],
					parentctrltable[i].mac[4], parentctrltable[i].mac[5]);
				va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)getWlanIfName(), "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");
				va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)ELANIF, "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");
			}
			//remove the iptable here if it exists
			memset(&parentctrltable[i],0, sizeof(MIB_PARENT_CTRL_T));
			break;
		}
	}
}

int parent_ctrl_table_delall(void)
{
	int	i;
	char macaddr[20];

	for (i = 0; i < MAX_PARENTCTRL_USER_NUM; i++)
	{
		if (parentctrltable[i].cur_state == 1)
		{
			//del the entry
			snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				parentctrltable[i].mac[0], parentctrltable[i].mac[1],
				parentctrltable[i].mac[2], parentctrltable[i].mac[3],
				parentctrltable[i].mac[4], parentctrltable[i].mac[5]);
			va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
					(char *)ARG_I, (char *)getWlanIfName(), "-m", "mac",
					"--mac-source",  macaddr, "-j", "DROP");
			va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
					(char *)ARG_I, (char *)ELANIF, "-m", "mac",
					"--mac-source",  macaddr, "-j", "DROP");
		}

	}
	memset(&parentctrltable[0],0, sizeof(MIB_PARENT_CTRL_T)*MAX_PARENTCTRL_USER_NUM);
}

// update the rules to iptables according to current time
int parent_ctrl_table_rule_update(void)
{
	int i, check;
	char macaddr[20];

	for (i = 0; i < MAX_PARENTCTRL_USER_NUM; i++)
	{
		if (strlen(parentctrltable[i].username) > 0)
		{

			check = parent_ctrl_check(&parentctrltable[i]);

			if (( check == 1) &&
				(parentctrltable[i].cur_state == 0))
			{
				parentctrltable[i].cur_state = 1;

				//add the entry
				snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				parentctrltable[i].mac[0], parentctrltable[i].mac[1],
				parentctrltable[i].mac[2], parentctrltable[i].mac[3],
				parentctrltable[i].mac[4], parentctrltable[i].mac[5]);

				//for debug
				va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)getWlanIfName(), "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");
				va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)ELANIF, "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");
			}
			else if ((check == 0) &&
				(parentctrltable[i].cur_state == 1))
			{
				parentctrltable[i].cur_state = 0;
				//del the entry
				snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
				parentctrltable[i].mac[0], parentctrltable[i].mac[1],
				parentctrltable[i].mac[2], parentctrltable[i].mac[3],
				parentctrltable[i].mac[4], parentctrltable[i].mac[5]);

				va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)getWlanIfName(), "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");
				va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_MACFILTER,
						(char *)ARG_I, (char *)ELANIF, "-m", "mac",
						"--mac-source",  macaddr, "-j", "DROP");
			}

		}
	}
}
#endif

#ifdef CONFIG_USER_RTK_VOIP
#include "web_voip.h"
/*----------------------------------------------------------------------------
 * Name:
 *      voip_setup_iptable
 * Descriptions:
 *      Creat an iptable rule to allow incoming VoIP calls.
 * return:              none
 *---------------------------------------------------------------------------*/
static void voip_setup_iptable(void)
{
        char portbuff[10];
	voipCfgParam_t * pCfg;
	voipCfgPortParam_t *VoIPport;
	int i,val;

	val=voip_flash_get( &pCfg);
	if (val != 0)
	{
		for ( i=5060; i<=5061; i++)	//default value
		{
			sprintf(portbuff, "%d",i);
  			va_cmd(IPTABLES, 11, 1, "-I", (char *)FW_INPUT,
             	   "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
	                (char *)FW_DPORT,portbuff, "-j", (char* ) "ACCEPT");
		}
	}
	else
	{
        for ( i=0; i<VOIP_PORTS; i++)
        {
                VoIPport = &pCfg->ports[i];
	        sprintf(portbuff, "%d",VoIPport->sip_port);
      		  va_cmd(IPTABLES, 11, 1, "-I", (char *)FW_INPUT,
                "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
                (char *)FW_DPORT,portbuff, "-j", (char* ) "ACCEPT");
        }
	}
}

#ifdef _PRMT_X_CT_COM_DHCP_
#define DEBUG_CHN_TEL(format, ...)
//#define DEBUG_CHN_TEL  printf

#include <openssl/des.h>
#include <openssl/md5.h>
#include <voip_flash.h>

void output_hex(const char *prefix, const unsigned char *data, size_t size)
{
	int i;

	DEBUG_CHN_TEL("%s", prefix);
	for(i=0 ; i < size ; i++)
	{
		DEBUG_CHN_TEL("%02X ", data[i]);
	}
	DEBUG_CHN_TEL("\n");
}

/** Utilities to encrypt VoIP username/password **/
/** For DHCP option 60 & DHCPv6 option 16 **/
static void get_key(const unsigned char *in, const unsigned char *R, const unsigned char *TS, unsigned char *out)
{
	unsigned char tmp[80];
	unsigned char *cur = tmp;

	memcpy(cur, R, 8);
	cur += 8;

	memcpy(cur, in, strlen(in));
	cur += strlen(in);

	memcpy(cur, TS, 8);
	cur += 8;

	MD5(tmp, strlen(in)+16, out);
}

static void do_3des(const unsigned char *key, const unsigned char *in, unsigned char *out, size_t len)
{
	DES_key_schedule ks1, ks2, ks3;
	DES_cblock block_key;
	DES_cblock tmp1, tmp2;
	int cnt, i;

	memcpy(&block_key, key, 8);
	DES_set_key_unchecked(&block_key, &ks1);

	memcpy(&block_key, key + 8, 8);
    DES_set_key_unchecked(&block_key, &ks2);

	memcpy(&block_key, key + 16, 8);
    DES_set_key_unchecked(&block_key, &ks3);

	cnt = len / 8;

	for(i = 0; i < cnt ; i++)
	{
		memcpy(tmp1, in + 8*i, 8);
		DES_ecb3_encrypt(&tmp1, &tmp2, &ks1, &ks2, &ks3, DES_ENCRYPT);
		memcpy(out + 8*i, &tmp2, 8);
	}
}

static int get_encrypt_text(char *user, char *R, char *TS, char *buf)
{
	char des3_key[24] = {0};
	int user_len = strlen(user);
	int i;
	char *plain_text;
	int plain_len;
	unsigned char pkcs7;


	output_hex("R = ", R, 8);
	output_hex("TS = ", TS, 8);

	// key for 3DES
	memcpy(des3_key, R, 8);
	memcpy(des3_key + 8, TS, 8);
	//output_hex("3des_key = ", des3_key, 24);

	// Prepare plain text for 3DES
	pkcs7 = (user_len % 8 == 0) ? 8 : 8 - (user_len % 8);
	plain_len = user_len + pkcs7;
	plain_text = malloc(plain_len);
	strcpy(plain_text, user);
	for( i = 0 ; i < pkcs7 ; i++)
		*(plain_text + user_len + i) = pkcs7;

	//output_hex("plain_text = ", plain_text, plain_len);

	//clear output buffer
	memset(buf, 0, plain_len);

	do_3des(des3_key, plain_text, buf, plain_len);

	output_hex("encrypted text = ", buf, plain_len);
	free(plain_text);

	return plain_len;
}

/*----------------------------------------------------------------------------
 * Name:
 *      get_ctc_voip_encrypted_data
 * Descriptions:
 *      Generate DHCP option 60 or DHCPv6 option 16 field type 34 values.
 * return:
 *      Length of encrypted data.
 *---------------------------------------------------------------------------*/
static int get_ctc_voip_encrypted_data(char *out, int len)
{
	char buf[80];
	char *cur = out;
	int buf_len;
	char serial[64] = {0};
	char oui[32] = {0};

#if 0	//test data 1
	// Result of C: A4 ED BA BF 98 50 1E D1 6C BF DE 0D 0E A5 0F F9
	// Result of Key: 53 BE 31 31 0F 1F 5F C8 53 8E 67 70 77 32 19 DE
	char user[] = "ad66133512@iptv";
	char pass[] = "123456";
	char R[8] = {0x0A, 0xDA, 0xA1, 0x3C, 0x13, 0x0D, 0x33, 0x6A};
	char TS[8] = {0x0E, 0};
#endif

#if 0	//test data 2
	// Result of C: 1A B2 45 FF B0 CF B4 47 52 29 C8 67 10 6B E0 E3
	// Result of Key: 26 57 07 02 2B 15 B6 BC 3A C6 50 1B 69 46 29 FA
	char user[] = "ad66133512@iptv";
	char pass[] = "123456";
	char R[8] = {0x3E, 0x1C, 0x2D, 0x45, 0x13, 0xDB, 0x38, 0x15};
	char TS[8] = {0xFB, 0x16, 0x57, 0x4E, 0};
#endif

#if 1	//disable this code block and enable above test data for testing
	char username[DNS_LEN] = {0};
	char password[DNS_LEN] = {0};
	char R[8] = {0};	//8 bytes random number
	char TS[8] = {0};	//8 bytes time stamp with leading zero
	long temp;
	voipCfgParam_t * pCfg;
	voipCfgPortParam_t *VoIPport;
	int i,val;

	if(voip_flash_get( &pCfg) == 0)
	{
		strcpy(username, pCfg->ports[0].proxies[0].login_id);
		strcpy(password, pCfg->ports[0].proxies[0].password);
	}

	temp = time(NULL);
	memcpy(TS, &temp, 4);

	srandom(temp);
	temp = random();
	memcpy(R, &temp, 4);
	temp = random();
	memcpy(R+4, &temp, 4);
#endif

	mib_get(MIB_HW_SERIAL_NUMBER, serial);
	mib_set(RTK_DEVID_OUI, "");
	getOUIfromMAC(oui);

	DEBUG_CHN_TEL("Username: %s\n", username);
	DEBUG_CHN_TEL("Password: %s\n", password);
	DEBUG_CHN_TEL("OUI: %s\n", oui);
	DEBUG_CHN_TEL("Serial Number: %s\n", serial);

	buf_len = strlen(oui);
	memcpy(cur, oui, buf_len);
	cur += buf_len;

	memcpy(cur, "-", 1);
	cur += 1;

	buf_len = strlen(serial);
	memcpy(cur, serial, buf_len);
	cur += buf_len;


	if(strlen(username) == 0 || strlen(password) == 0)
	{
		printf("Use default: admin/admin\n");
		strcpy(cur, "admin/admin");
		cur += strlen("admin/admin");
		return (cur-out);
	}

	// First byte is encryption type;
	*cur = 1;
	cur += 1;

	// concat random number
	memcpy(cur, R, 8);
	cur += 8;

	// concat time stamp
	memcpy(cur, TS, 8);
	cur += 8;

	// length of MD5 result is 16
	get_key(password, R, TS, buf);
	output_hex("Key = ", buf, 16);
	memcpy(cur, buf, 16);
	cur += 16;

	buf_len = get_encrypt_text(username, R, TS, buf);
	memcpy(cur, buf, buf_len);
	cur += buf_len;

	output_hex("Result: ", out, cur - out);

	return (cur - out);
}
#endif //_PRMT_X_CT_COM_DHCP_
#endif //CONFIG_USER_RTK_VOIP

#ifdef _PRMT_X_CT_COM_DHCP_
int gen_ctcom_dhcp_opt(unsigned char type, char *output, int out_len)
{
	int len;

	if(output == NULL)
		return 0;

	switch(type)
	{
	case 1:
		strncpy(output, "Realtek", out_len);
		return strlen(output);
	case 2:
		strncpy(output, "HGW", out_len);
		return strlen(output);
	case 3:
		strncpy(output, DEVICE_MODEL_STR, out_len);
		return strlen(output);
	case 4:
		snprintf(output, out_len, "%s/%s", HARDWARE_VERSION_STR, SOFTWARE_VERSION_STR);
		return strlen(output);
#ifdef CONFIG_USER_RTK_VOIP
	case 34:
		return get_ctc_voip_encrypted_data(output, out_len);
#endif
	default:
		return -1;
	}
}
#endif //_PRMT_X_CT_COM_DHCP_


#ifdef VIRTUAL_SERVER_SUPPORT
int setupVtlsvr(int type)
{

	int i, total;
	struct vtlsvr_entryx Entry;
	total = mib_chain_total(MIB_VIRTUAL_SVR_TBL);
	DOCMDINIT;
/*
#ifdef IP_FILTER_PRIORITY_HIGHEST
	int position=mib_chain_total(MIB_IP_PORT_FILTER_TBL)+3;
#else
	int position=3;
#endif
*/
	//clear the IPTABLE_VTLSUR rules in the filter tables
	DOCMDARGVS(IPTABLES, DOWAIT, "-F %s", IPTABLE_VTLSUR);

	//clear the IPTABLE_VTLSUR rules in the nat tables
	DOCMDARGVS(IPTABLES, DOWAIT, "-t nat -F %s", IPTABLE_VTLSUR);

#ifdef CONFIG_RTK_RG_INIT
	FlushRTK_RG_Virtual_Server();
#endif

	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_VIRTUAL_SVR_TBL, i, (void *)&Entry))
			return -1;

#ifdef CONFIG_RTK_RG_INIT
		if(type == VIRTUAL_SERVER_ADD)
			RTK_RG_Virtual_Server_Set(&Entry);
#endif
		if (Entry.protoType == PROTO_TCP || Entry.protoType == 0)//0--TCP/UDP
		{
		/*ping_zhang:20080809 START:replace 'br0' with 'br+' in order to involve pppoe proxy connection*/
	 	#ifdef CONFIG_PPPOE_PROXY_IF_NAME
			// iptables -t nat -A virtual_server -i ! $LAN_IF -p TCP --dport srcPortRange -j DNAT --to-destination ipaddr --to-ports 8080
			DOCMDARGVS(IPTABLES, DOWAIT, "-t nat %s %s -i ! br+ -p TCP --dport %u:%u -j DNAT --to-destination %s:%u ",VIRTUAL_SERVER_ACTION_APPEND(type),IPTABLE_VTLSUR,
				Entry.wanStartPort, Entry.wanEndPort,	inet_ntoa(*((struct in_addr *)&(Entry.serverIp))),Entry.lanPort);
			// iptables -A virtual_server 3 -i ! $LAN_IF -o $LAN_IF -p TCP --dport dstPortRange -j ACCEPT
			DOCMDARGVS(IPTABLES, DOWAIT, "%s %s -i ! br+ -o br+ -p TCP -d %s --dport %u -j ACCEPT",VIRTUAL_SERVER_ACTION_INSERT(type),IPTABLE_VTLSUR,inet_ntoa(*((struct in_addr *)&(Entry.serverIp))),Entry.lanPort);
		#else
			// iptables -t nat -A virtual_server -i ! $LAN_IF -p TCP --dport srcPortRange -j DNAT --to-destination ipaddr --to-ports 8080
			DOCMDARGVS(IPTABLES, DOWAIT, "-t nat %s %s -i ! br0 -p TCP --dport %u:%u -j DNAT --to-destination %s:%u ",VIRTUAL_SERVER_ACTION_APPEND(type),IPTABLE_VTLSUR,
				Entry.wanStartPort, Entry.wanEndPort,	inet_ntoa(*((struct in_addr *)&(Entry.serverIp))),Entry.lanPort);
			// iptables -A virtual_server -i ! $LAN_IF -o $LAN_IF -p TCP --dport dstPortRange -j ACCEPT
			DOCMDARGVS(IPTABLES, DOWAIT, "%s %s -i ! br0 -o br0 -p TCP -d %s --dport %u -j ACCEPT",VIRTUAL_SERVER_ACTION_INSERT(type),IPTABLE_VTLSUR,inet_ntoa(*((struct in_addr *)&(Entry.serverIp))),Entry.lanPort);
		#endif
	 	/*ping_zhang:20080809 END*/
		}

		if (Entry.protoType == PROTO_UDP || Entry.protoType == 0)
		{
	 	/*ping_zhang:20080809 START:replace 'br0' with 'br+' in order to involve pppoe proxy connection*/
	 	#ifdef CONFIG_PPPOE_PROXY_IF_NAME
			// iptables -t nat -A virtual_server -i ! $LAN_IF -p TCP --dport srcPortRange -j DNAT --to-destination ipaddr --to-ports 8080
			DOCMDARGVS(IPTABLES, DOWAIT, "-t nat %s %s -i ! br+ -p UDP --dport %u:%u -j DNAT --to-destination %s:%u ",VIRTUAL_SERVER_ACTION_APPEND(type),IPTABLE_VTLSUR,
				Entry.wanStartPort, Entry.wanEndPort,	inet_ntoa(*((struct in_addr *)&(Entry.serverIp))),Entry.lanPort);
			// iptables -A virtual_server -i ! $LAN_IF -o $LAN_IF -p TCP --dport dstPortRange -j ACCEPT
			DOCMDARGVS(IPTABLES, DOWAIT, "%s %s  -i ! br+ -o br+ -p UDP -d %s --dport %u -j ACCEPT",VIRTUAL_SERVER_ACTION_INSERT(type),IPTABLE_VTLSUR,inet_ntoa(*((struct in_addr *)&(Entry.serverIp))),Entry.lanPort);
	 	#else
			// iptables -t nat -A virtual_server -i ! $LAN_IF -p TCP --dport srcPortRange -j DNAT --to-destination ipaddr --to-ports 8080
			DOCMDARGVS(IPTABLES, DOWAIT, "-t nat %s %s -i ! br0 -p UDP --dport %u:%u -j DNAT --to-destination %s:%u ",VIRTUAL_SERVER_ACTION_APPEND(type),IPTABLE_VTLSUR,
				Entry.wanStartPort, Entry.wanEndPort,	inet_ntoa(*((struct in_addr *)&(Entry.serverIp))),Entry.lanPort);
			// iptables -A virtual_server -i ! $LAN_IF -o $LAN_IF -p TCP --dport dstPortRange -j ACCEPT
			DOCMDARGVS(IPTABLES, DOWAIT, "%s %s  -i ! br0 -o br0 -p UDP -d %s --dport %u -j ACCEPT", VIRTUAL_SERVER_ACTION_INSERT(type), IPTABLE_VTLSUR, inet_ntoa(*((struct in_addr *)&(Entry.serverIp))),Entry.lanPort);
	 	#endif
	 	/*ping_zhang:20080809 END*/
		}
	}

	return 1;
}
#endif

#ifdef PORT_FORWARD_GENERAL
void clear_dynamic_port_fw(int (*upnp_delete_redirection)(unsigned short eport, const char * protocol))
{
	int i, total;
	MIB_CE_PORT_FW_T port_entity;

	total = mib_chain_total(MIB_PORT_FW_TBL);

	for (i = total - 1; i >= 0; i--) {
		if (!mib_chain_get(MIB_PORT_FW_TBL, i, &port_entity))
			continue;

		if (port_entity.dynamic) {
			/* created by UPnP */
			mib_chain_delete(MIB_PORT_FW_TBL, i);

			if (upnp_delete_redirection)
				upnp_delete_redirection(port_entity.externalfromport,
					port_entity.protoType == PROTO_UDP ? "UDP" : "TCP");
		}
	}
}

// Mason Yu
int setupPortFW(void)
{
	int vInt, i, total;
	unsigned char value[32];
	MIB_CE_PORT_FW_T PfEntry;

	// Clean all rules
	// iptables -F portfw
	va_cmd(IPTABLES, 2, 1, "-F", (char *)PORT_FW);
	// iptables -t nat -F portfw
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", (char *)PORT_FW);

#ifdef CONFIG_RTK_RG_INIT
	FlushRTK_RG_Vertual_Server();
#endif

	vInt = 0;
	if (mib_get(MIB_PORT_FW_ENABLE, (void *)value) != 0)
		vInt = (int)(*(unsigned char *)value);

	if (vInt == 1)
	{
		int negate=0, hasRemote=0;
		char * proto = 0;
		char intPort[32], extPort[32];

		total = mib_chain_total(MIB_PORT_FW_TBL);

		for (i = 0; i < total; i++)
		{
			if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&PfEntry))
				return -1;
			if (PfEntry.dynamic == 1)
				continue;
#ifdef CONFIG_RTK_RG_INIT
			RTK_RG_Vertual_Server_Set(&PfEntry);
#endif
			portfw_modify( &PfEntry, 0 );
		}
	}//if (vInt == 1)

	return 1;
}
#endif

#ifdef PORT_FORWARD_ADVANCE
static int setupPFWAdvance(void)
{
	// iptables -N pptp
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_PPTP);
	// iptables -A FORWARD -j pptp
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_PPTP);
	// iptables -t nat -N pptp
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)FW_PPTP);
	// iptables -t nat -A PREROUTING -j pptp
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)FW_PPTP);


	// iptables -N l2tp
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_L2TP);
	// iptables -A FORWARD -j l2tp
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_L2TP);
	// iptables -t nat -N l2tp
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)FW_L2TP);
	// iptables -t nat -A PREROUTING -j l2tp
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)FW_L2TP);

	config_PFWAdvance(ACT_START);

	return 0;
}

static int stopPFWAdvance(void)
{
	unsigned int entryNum, i;
	MIB_CE_PORT_FW_ADVANCE_T Entry;
	int pptp_enable=1;
	int l2tp_enable=1;

	entryNum = mib_chain_total(MIB_PFW_ADVANCE_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_PFW_ADVANCE_TBL, i, (void *)&Entry))
		{
  			printf("stopPFWAdvance: Get chain record error!\n");
			return 1;
		}

		if ( strcmp("PPTP", PFW_Rule[(PFW_RULE_T)Entry.rule]) == 0 && pptp_enable == 1) {
			// iptables -F pptp
			va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_PPTP);
			// iptables -t nat -F pptp
			va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", (char *)FW_PPTP);
			pptp_enable = 0;
		}

		if ( strcmp("L2TP", PFW_Rule[(PFW_RULE_T)Entry.rule]) == 0 && l2tp_enable == 1) {
			// iptables -F l2tp
			va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_L2TP);
			// iptables -t nat -F l2tp
			va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", (char *)FW_L2TP);
			l2tp_enable = 0;
		}
	}
	return 0;
}

static int startPFWAdvance(void)
{
	unsigned int entryNum, i;
	MIB_CE_PORT_FW_ADVANCE_T Entry;
	char interface_name[IFNAMSIZ], lanIP[16], ip_port[32];
	struct in_addr dest;
	int pf_enable;
	unsigned char value[32];
	int pptp_enable=0;
	int l2tp_enable=0;

	entryNum = mib_chain_total(MIB_PFW_ADVANCE_TBL);


	pf_enable = 0;
	if (mib_get(MIB_PORT_FW_ENABLE, (void *)value) != 0)
		pf_enable = (int)(*(unsigned char *)value);

	if ( pf_enable != 1 ) {
		printf("Port Forwarding is disable and stop to setup Port Forwarding Advance!\n");
		return 1;
	}

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_PFW_ADVANCE_TBL, i, (void *)&Entry))
		{
  			printf("startPFWAdvance: Get chain record error!\n");
			return 1;
		}

		if ( strcmp("PPTP", PFW_Rule[(PFW_RULE_T)Entry.rule]) == 0 && pptp_enable == 0) {
			// LAN IP Address
			dest.s_addr = *(unsigned long *)Entry.ipAddr;
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strcpy(lanIP, inet_ntoa(dest));
			lanIP[15] = '\0';
			sprintf(ip_port,"%s:%d",lanIP, 1723);

			// interface

			// iptables -A pptp -p tcp --destination-port 1723 --dst $LANIP -j ACCEPT
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_PPTP, "-p", (char *)ARG_TCP, "--destination-port", "1723", "--dst", lanIP, "-j", (char *)FW_ACCEPT);

			// iptables -A pptp -p 47 --dst $LANIP -j ACCEPT
			va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_PPTP, "-p", "47", "--dst", lanIP, "-j", (char *)FW_ACCEPT);


			if (ifGetName(Entry.ifIndex, interface_name, sizeof(interface_name))) {
				// iptables -t nat -A pptp -i ppp0 -p tcp --dport 1723 -j DNAT --to-destination $LANIP:1723
				va_cmd(IPTABLES, 14, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PPTP, "-i", interface_name, "-p", (char *)ARG_TCP, "--dport", "1723", "-j", "DNAT", "--to-destination", ip_port);

				// iptables -t nat -A pptp -i ppp0 -p 47 -j DNAT --to-destination $LANIP
				va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PPTP, "-i", interface_name, "-p", "47", "-j", "DNAT", "--to-destination", lanIP);
			} else {
				// iptables -t nat -A pptp -p tcp --dport 1723 -j DNAT --to-destination $LANIP:1723
				va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PPTP, "-p", (char *)ARG_TCP, "--dport", "1723", "-j", "DNAT", "--to-destination", ip_port);

				// iptables -t nat -A pptp -p 47 -j DNAT --to-destination $LANIP
				va_cmd(IPTABLES, 10, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PPTP, "-p", "47", "-j", "DNAT", "--to-destination", lanIP);
			}
			pptp_enable = 1;
		}

		if ( strcmp("L2TP", PFW_Rule[(PFW_RULE_T)Entry.rule]) == 0 && l2tp_enable == 0) {
			// LAN IP Address
			dest.s_addr = *(unsigned long *)Entry.ipAddr;
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strcpy(lanIP, inet_ntoa(dest));
			lanIP[15] = '\0';
			sprintf(ip_port,"%s:%d",lanIP, 1701);

			// interface

			// iptables -A l2tp -p udp --destination-port 1701 --dst $LANIP -j ACCEPT
			va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_L2TP, "-p", (char *)ARG_UDP, "--destination-port", "1701", "--dst", lanIP, "-j", (char *)FW_ACCEPT);

			// iptables -t nat -A l2tp -i ppp0 -p udp --dport 1701 -j DNAT --to-destination $LANIP:1701
			if (!ifGetName(Entry.ifIndex, interface_name, sizeof(interface_name))) {
				va_cmd(IPTABLES, 12, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_L2TP, "-p", (char *)ARG_UDP, "--dport", "1701", "-j", "DNAT", "--to-destination", ip_port);
			} else {
				va_cmd(IPTABLES, 14, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_L2TP, "-i", interface_name, "-p", (char *)ARG_UDP, "--dport", "1701", "-j", "DNAT", "--to-destination", ip_port);
			}
			l2tp_enable = 1;
		}
	}
	return 0;
}
#endif

static void clearDMZ(void)
{
	// iptables -F dmz
	va_cmd(IPTABLES, 2, 1, "-F", (char *)IPTABLE_DMZ);
	// iptables -t nat -F dmz
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", (char *)IPTABLE_DMZ);
}

static void setDMZ(char *ip)
{
	//snmp don't forward to DMZ
	// iptables -t nat -A dmz -i ! $LAN_IF -p UDP --dport 161:162 -j ACCEPT
	va_cmd(IPTABLES, 13, 1, "-t", "nat", (char *)FW_ADD, (char *)IPTABLE_DMZ, "!", (char *)ARG_I,
		(char *)LANIF, "-p", (char *)ARG_UDP, (char *)FW_DPORT, "161:162", "-j", (char *)FW_ACCEPT);
#ifdef _CWMP_MIB_
	va_cmd(IPTABLES, 13, 1, "-t", "nat", (char *)FW_ADD, (char *)IPTABLE_DMZ, "!", (char *)ARG_I,
		(char *)LANIF, "-p", (char *)ARG_TCP, (char *)FW_DPORT, "9999", "-j", (char *)FW_ACCEPT);
#endif
	// Kaohj -- Multicast not involved
	// iptables -t nat -A dmz -i ! $LAN_IF -d 224.0.0.0/4 -j ACCEPT
	va_cmd(IPTABLES, 11, 1, "-t", "nat", (char *)FW_ADD, (char *)IPTABLE_DMZ, "!", (char *)ARG_I,
		(char *)LANIF, "-d", "224.0.0.0/4", "-j", (char *)FW_ACCEPT);
	// Remote access not go DMZ
	va_cmd(IPTABLES, 13, 1, "-t", "nat", (char *)FW_ADD, (char *)IPTABLE_DMZ, "!", (char *)ARG_I,
		(char *)LANIF, "-m", "mark", "--mark", RMACC_MARK, "-j", FW_RETURN);
	// iptables -t nat -A dmz -i ! $LAN_IF -j DNAT --to-destination $DMZ_IP
	va_cmd(IPTABLES, 11, 1, "-t", "nat", (char *)FW_ADD, (char *)IPTABLE_DMZ, "!", (char *)ARG_I,
		(char *)LANIF, "-j", "DNAT", "--to-destination", ip);
	// iptables -A dmz -i ! $LAN_IF -o $LAN_IF -d $DMZ_IP -j ACCEPT
	va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, (char *)IPTABLE_DMZ, "!", (char *)ARG_I,
		(char *)LANIF, (char *)ARG_O, (char *)LANIF, "-d", ip, "-j", (char *)FW_ACCEPT);
}

// Mason Yu
int setupDMZ(void)
{
	int vInt;
	unsigned char value[32];
	char ipaddr[32];

	clearDMZ();

	vInt = 0;
	if (mib_get(MIB_DMZ_ENABLE, (void *)value) != 0)
		vInt = (int)(*(unsigned char *)value);

	if (mib_get(MIB_DMZ_IP, (void *)value) != 0)
	{
#ifdef CONFIG_RTK_RG_INIT
		RTK_RG_DMZ_Set(vInt, *(in_addr_t *)&value);
#endif

		if (vInt == 1)
		{
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
			ipaddr[15] = '\0';
			setDMZ(ipaddr);
		}
	}
}

#ifdef NATIP_FORWARDING
static void fw_setupIPForwarding(void)
{
	int i, total;
	char local[16], remote[16], enable;
	MIB_CE_IP_FW_T Entry;

	// Add New chain on filter and nat
	// iptables -N ipfw
	va_cmd(IPTABLES, 2, 1, "-N", (char *)IPTABLE_IPFW);
	// iptables -A FORWARD -j ipfw
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)IPTABLE_IPFW);
	// iptables -t nat -N ipfw
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)IPTABLE_IPFW);
	// iptables -t nat -A PREROUTING -j ipfw
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)IPTABLE_IPFW);
	// iptables -t nat -N ipfw2
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)IPTABLE_IPFW2);
	// iptables -t nat -A POSTROUTING -j ipfw2
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_POSTROUTING, "-j", (char *)IPTABLE_IPFW2);


	mib_get(MIB_IP_FW_ENABLE, (void *)&enable);
	if (!enable)
		return;

	total = mib_chain_total(MIB_IP_FW_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_IP_FW_TBL, i, (void *)&Entry))
			continue;
		strncpy(local, inet_ntoa(*((struct in_addr *)Entry.local_ip)), 16);
		strncpy(remote, inet_ntoa(*((struct in_addr *)Entry.remote_ip)), 16);
		// iptables -t nat -A ipfw -d remoteip -i ! $LAN_IF -j DNAT --to-destination localip
		va_cmd(IPTABLES, 13, 1, "-t", "nat",
			(char *)FW_ADD,	(char *)IPTABLE_IPFW, "-d", remote,
			 "!", (char *)ARG_I, (char *)LANIF, "-j",
			"DNAT", "--to-destination", local);
		// iptables -t nat -A ipfw2 -s localip -o ! br0 -j SNAT --to-source remoteip
		va_cmd(IPTABLES, 13, 1, "-t", "nat", FW_ADD, (char *)IPTABLE_IPFW2,

			(char *)ARG_O, "!", (char *)LANIF, "-s", local, "-j", "SNAT", "--to-source", remote);

		// iptables -A ipfw2 -d localip -i ! $LAN_IF -o $LAN_IF -j RETURN
		va_cmd(IPTABLES, 11, 1, (char *)FW_ADD,
			(char *)IPTABLE_IPFW, "-d", local, "!", (char *)ARG_I,
			(char *)LANIF, (char *)ARG_O,
			(char *)LANIF, "-j", (char *)FW_ACCEPT);
	}
}
#endif

static int setup_default_IPFilter(void)
{
	// Set default action for ipfilter
	unsigned char value[32];
	int vInt;
	unsigned char ipportfilter_state;

	mib_get(MIB_IPFILTER_ON_OFF, (void*)&ipportfilter_state);

	if(ipportfilter_state == 0) return 0;

	if (mib_get(MIB_IPF_OUT_ACTION, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned char *)value);

		if (vInt == 0)	// DROP
		{
			// iptables -A ipfilter -i $LAN_IF -j DROP
			va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_IPFILTER, (char *)ARG_I, (char *)LANIF, "-j", (char *)FW_DROP);
		}
	}

	if (mib_get(MIB_IPF_IN_ACTION, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned char *)value);

		if (vInt == 0)	// DROP
		{
			// iptables -A ipfilter -i ! $LAN_IF -j DROP
				va_cmd(IPTABLES, 7, 1, (char *)FW_ADD, (char *)FW_IPFILTER, "!", (char *)ARG_I, (char *)LANIF, "-j", (char *)FW_DROP);
		}
	}

	return 1;
}

#ifdef CONFIG_RTK_RG_INIT
static int setup_RG_IPPort_Filter_Default_Policy()
{
	char vChar;
	int in_policy, out_policy;
	unsigned char ipportfilter_state;

	mib_get(MIB_IPFILTER_ON_OFF, (void*)&ipportfilter_state);

	if(ipportfilter_state == 0) return 0;

	if (mib_get(MIB_IPF_OUT_ACTION, &vChar) != 0)
		out_policy = vChar;

	if (mib_get(MIB_IPF_IN_ACTION, &vChar) != 0)
		in_policy = vChar;

	RTK_RG_ACL_IPPort_Filter_Default_Policy(out_policy, in_policy);
}
#endif

#if defined(WIFI_TIMER_SCHEDULE) || defined(CONFIG_LED_INDICATOR_TIMER) || defined(CONFIG_RG_SLEEPMODE_TIMER)
void updateScheduleCrondFile(char *pathname, int startup)
{
	char *strVal;
	int totalnum, i, j, first, crond_pid;
	FILE *fp, *getPIDS;
	char tmpbuf[100], day_string[20], kill_cmd[100], filename[100];
#ifdef WIFI_TIMER_SCHEDULE
	MIB_CE_WIFI_TIMER_EX_T Entry_ex;
	MIB_CE_WIFI_TIMER_T Entry;
	unsigned char startHour, startMinute, endHour, endMinute;
#endif
#ifdef CONFIG_LED_INDICATOR_TIMER
	MIB_CE_DAY_SCHED_T ledEntry;
#endif
#ifdef CONFIG_RG_SLEEPMODE_TIMER
	MIB_CE_RG_SLEEPMODE_SCHED_T sleepEntry;
#endif
	int schedFileExist = 0;

	if(startup){
		snprintf(tmpbuf, 100, "mkdir -p %s", pathname);
		system(tmpbuf);
	}
	if ( !mib_get(MIB_SUSER_NAME, (void *)tmpbuf) ) {
		printf("ERROR: Get superuser name from MIB database failed.\n");
		return;
	}
	snprintf(filename, 100, "%s/%s", pathname, tmpbuf);

	crond_pid = read_pid("/var/run/crond.pid");
	if(crond_pid > 0){
		kill(crond_pid, 9);
		unlink("/var/run/crond.pid");
	}				
	
	fp = fopen(filename, "w");
	
#ifdef WIFI_TIMER_SCHEDULE
	totalnum = mib_chain_total(MIB_WIFI_TIMER_EX_TBL);
	for(i=0; i<totalnum; i++){
		mib_chain_get(MIB_WIFI_TIMER_EX_TBL, i, &Entry_ex);
		
		first = 1;
		if(Entry_ex.day & (1<<7)){
			if(first){
				snprintf(day_string, 20, "0");
				first = 0;
			}
			else
				snprintf(day_string, 20,"%s,0", day_string);
		}
		for(j=1;j<7;j++){
			if(Entry_ex.day & (1<<j)){
				if(first){
					snprintf(day_string, 20, "%d", j);
					first = 0;
				}
				else
					snprintf(day_string, 20,"%s,%d", day_string, j);
			}
		}
		if(Entry_ex.enable){
			sscanf(Entry_ex.Time, "%hhu:%hhu", &startHour, &startMinute);
			snprintf(tmpbuf, 100, "%hhu %hhu * * %s /bin/config_wlan %d\n", startMinute, startHour, day_string, Entry_ex.onoff);
			fputs(tmpbuf,fp);
			if(schedFileExist == 0)
				schedFileExist = 1;
		}
	}

	totalnum = mib_chain_total(MIB_WIFI_TIMER_TBL);
	for(i=0; i<totalnum; i++){
		mib_chain_get(MIB_WIFI_TIMER_TBL, i, &Entry);
		if(Entry.enable){
			sscanf(Entry.startTime, "%hhu:%hhu", &startHour, &startMinute);
			snprintf(tmpbuf, 100, "%hhu %hhu */%hhu * * /bin/config_wlan 1\n", startMinute, startHour, Entry.controlCycle);
			fputs(tmpbuf,fp);
			sscanf(Entry.endTime, "%hhu:%hhu", &endHour, &endMinute);
			snprintf(tmpbuf, 100, "%hhu %hhu */%hhu * * /bin/config_wlan 0\n", endMinute, endHour, Entry.controlCycle);
			fputs(tmpbuf,fp);
			if(schedFileExist == 0)
				schedFileExist = 1;
		}
	}
#endif

#ifdef CONFIG_LED_INDICATOR_TIMER
	totalnum = mib_chain_total(MIB_LED_INDICATOR_TIMER_TBL);
	for(i=0; i<totalnum; i++)
	{
		mib_chain_get(MIB_LED_INDICATOR_TIMER_TBL, i, &ledEntry);
		if(ledEntry.enable)
		{
			//set initial state.
			time_t curTime;   
			struct tm curTm;
			int curState = 0;

			time(&curTime);   
			memcpy(&curTm, localtime(&curTime), sizeof(curTm));
			if(ledEntry.startHour == ledEntry.endHour)
			{
				if((curTm.tm_hour == ledEntry.startHour) 
					&& (ledEntry.startMin <= curTm.tm_min) && (ledEntry.endMin > curTm.tm_min))
				{
					curState = 1;
				}
			}
			else if((ledEntry.startHour <= curTm.tm_hour) && (ledEntry.endHour >= curTm.tm_hour))
			{
				if((ledEntry.startHour == curTm.tm_hour) && (ledEntry.startMin <= curTm.tm_min))
				{
					curState = 1;
				}
				else if((ledEntry.endHour == curTm.tm_hour) && (ledEntry.endMin > curTm.tm_min))
				{
					curState = 1;
				}
				else if((ledEntry.startHour < curTm.tm_hour) && (ledEntry.endHour > curTm.tm_hour))
				{
					curState = 1;
				}
			}
			if(1 == curState)
			{
				system("/bin/config_led 1");
			}
			else
			{
				system("/bin/config_led 0");
			}
			snprintf(tmpbuf, 100, "%hhu %hhu */%hhu * * /bin/config_led 1\n", ledEntry.startMin, ledEntry.startHour, ledEntry.ctlCycle);
			fputs(tmpbuf,fp);
			snprintf(tmpbuf, 100, "%hhu %hhu */%hhu * * /bin/config_led 0\n", ledEntry.endMin, ledEntry.endHour, ledEntry.ctlCycle);
			fputs(tmpbuf,fp);
			if(schedFileExist == 0)
				schedFileExist = 1;
		}
	}
#endif

#ifdef CONFIG_RG_SLEEPMODE_TIMER
	totalnum = mib_chain_total(MIB_SLEEP_MODE_SCHED_TBL);
	for(i=0; i<totalnum; i++)
	{
		mib_chain_get(MIB_SLEEP_MODE_SCHED_TBL, i, &sleepEntry);
		if(sleepEntry.enable)
		{
			if(0 == sleepEntry.day)
			{
				//action rightnow!
				snprintf(tmpbuf, 100, "/bin/config_sleepmode %d", sleepEntry.onoff);
				system(tmpbuf);
			}
			else
			{
				snprintf(tmpbuf, 100, "%hhu %hhu * * %d /bin/config_sleepmode %d\n", sleepEntry.minute, sleepEntry.hour, sleepEntry.day-1, sleepEntry.onoff);
				fputs(tmpbuf,fp);
			
				if(schedFileExist == 0)
					schedFileExist = 1;
			}
		}
	}
#endif

	fclose(fp);

	if(schedFileExist) {
		// file is not empty
		va_cmd("/bin/crond", 0, 1);
	}
	else
		unlink(filename);
}
#endif

#if 0
#ifdef CONFIG_LED_INDICATOR_TIMER
struct callout ledsched_ch;
unsigned int led_scheduled_work = 0;

int check_led_should_schedule(struct tm * pTime, MIB_CE_DAY_SCHED_T* pEntry)
{
	int result = 0;

	//ignore the ctlCycle NOW!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	/*
	if((pEntry->ctlCycle&(1<<pTime->tm_wday))==0)
	{
		//weekday does not match. pEntry->ctlCycle: bit 0: Sunday, bit 1: Monday, ..., bit 6: Saturday
		return 0;
	}
	*/
	
	if(pEntry->startHour == pEntry->endHour)
	{
		if((pTime->tm_hour == pEntry->startHour) 
			&& (pEntry->startMin <= pTime->tm_min) && (pEntry->endMin > pTime->tm_min))
		{
			result = 1;
		}
	}
	else if((pEntry->startHour <= pTime->tm_hour) && (pEntry->endHour >= pTime->tm_hour))
	{
		if((pEntry->startHour == pTime->tm_hour) && (pEntry->startMin <= pTime->tm_min))
		{
			result = 1;
		}
		else if((pEntry->endHour == pTime->tm_hour) && (pEntry->endMin> pTime->tm_min))
		{
			result = 1;
		}
		else if((pEntry->startHour < pTime->tm_hour) && (pEntry->endHour > pTime->tm_hour))
		{
			result = 1;
		}
	}

	return result;
	
}

void led_schedule(void)
{
	int total;
	unsigned int shouldSchedule = 0;
	MIB_CE_DAY_SCHED_T entry;
	unsigned char status;
	time_t curTime;   
	struct tm curTm;

	time(&curTime);   
	memcpy(&curTm, localtime(&curTime), sizeof(curTm));
	
	total = mib_chain_total(MIB_LED_INDICATOR_TIMER_TBL);
	if(0 == total)
	{
		//no schedule here.
		UNTIMEOUT(led_schedule, 0, ledsched_ch);
		system("/bin/mpctl led restore");
		return;
	}
	else
	{
		memset(&entry, 0, sizeof(entry));
		if(!mib_chain_get(MIB_LED_INDICATOR_TIMER_TBL, 0, (void *)&entry))
		{
			//Chain Get Error
			goto end;
		}
		else
		{
			if(0 == entry.enable)
			{
				//schedule not enable now
				UNTIMEOUT(led_schedule, 0, ledsched_ch);
				system("/bin/mpctl led restore");
				return;
			}
			else
			{
				shouldSchedule = check_led_should_schedule(&curTm, &entry);
			}
		}
	}

	if(!mib_get(MIB_LED_STATUS, (void *)&status))
	{
		//Mib Get Error!
		goto end;
	}
	printf("[%s %d]shouldSchedule=%d, led_scheduled_work=%d, led status=%d\n", __func__, __LINE__, shouldSchedule, led_scheduled_work, status);
	/* Start to Check the Schedule */
	if((shouldSchedule && (led_scheduled_work == 0)) 
		|| ((shouldSchedule == 0) && (led_scheduled_work || status==1)))
	{
		if(shouldSchedule && (led_scheduled_work == 0))
		{
			//Take all LED ON
			led_scheduled_work = 1;
			if(!mib_get(MIB_LED_STATUS, (void *)&status))
			{
				//Mib Get Error!
				goto end;
			}
			if(status == 0)
			{
				status = 1;
				mib_set(MIB_LED_STATUS, (void *)&status);
				system("/bin/mpctl led on");
			}
		}
		else
		{
			//Take all LED OFF
			led_scheduled_work = 0;
			if(!mib_get(MIB_LED_STATUS, (void *)&status))
			{
				//Mib Get Error!
				goto end;
			}
			if(status == 1)
			{
				status = 0;
				mib_set(MIB_LED_STATUS, (void *)&status);
				system("/bin/mpctl led off");
			}
		}
		#ifdef COMMIT_IMMEDIATELY
		Commit();
		#endif
	}

end:
	TIMEOUT(led_schedule, 0, 60, ledsched_ch);	
}
#endif

#ifdef CONFIG_RG_SLEEPMODE_TIMER
struct callout sleepmode_ch;
unsigned int rgsleep_scheduled_work = 0;

int check_sleep_should_schedule(struct tm * pTime, MIB_CE_RG_SLEEPMODE_SCHED_T * pEntry)
{
	//ignore the ctlCycle NOW!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	int result = 0;

	if(pEntry->day == 0)
	{
		//0 implys enable/disable psmode immediately
		return 1;
	}

	if((pEntry->day&(1<<pTime->tm_wday))==0)
	{
		//weekday does not match. pEntry->day: bit 0: Sunday, bit 1: Monday, ..., bit 6: Saturday
		return 0;
	}
	
	if(pEntry->hour > pTime->tm_hour)
	{
		result = 1;
	}
	else if((pEntry->hour == pTime->tm_hour) && (pEntry->minute >= pTime->tm_min))
	{
		result = 1;
	}

	return result;
}	

void sleepmode_schedule(void)
{
	int i, total;
	unsigned int shouldSchedule = 0;
	MIB_CE_RG_SLEEPMODE_SCHED_T entry;
	time_t curTime;   
	struct tm curTm;

	time(&curTime);   
	memcpy(&curTm, localtime(&curTime), sizeof(curTm));
	
	total = mib_chain_total(MIB_SLEEP_MODE_SCHED_TBL);
	if(0 == total)
	{
		//no schedule here.
		UNTIMEOUT(sleepmode_schedule, 0, sleepmode_ch);
		return;
	}
	else
	{
		for (i = 0; i < total; i++)
		{
			memset(&entry, 0, sizeof(entry));
			if(!mib_chain_get(MIB_SLEEP_MODE_SCHED_TBL, 0, (void *)&entry))
			{
				//Chain Get Error
				goto end;
			}
			else
			{
				if(0 == entry.enable)
				{
					//schedule not enable now
					continue;
				}
				else
				{
					shouldSchedule = check_sleep_should_schedule(&curTm, &entry);
				}

				/* Start to Check the Schedule */
				if((shouldSchedule && (rgsleep_scheduled_work == 0)) 
					|| ((shouldSchedule == 0) && rgsleep_scheduled_work))
				{
					if(shouldSchedule && (rgsleep_scheduled_work == 0))
					{
						//Enter sleep mode
						config_sleepmode(1);
					}
					else
					{
						//Leave sleep mode
						config_sleepmode(0);
					}
				}
			}
		}
	}
	
end:
	TIMEOUT(sleepmode_schedule, 0, 60, sleepmode_ch);	
}
#endif
#endif

#ifndef CONFIG_RTL8672NIC
void setupIPQoSflag(int flag)
{
	if(flag)
		system("ethctl setipqos 1");
	else
		system("ethctl setipqos 0");
}
#endif

int restart_IPFilter_DMZ_MACFilter(void)
{
	setupIPFilter();
	// iptables -A filter -j dmz
	va_cmd(IPTABLES, 4, 1, "-A", (char *)FW_IPFILTER, "-j", (char *)IPTABLE_DMZ);
	setup_default_IPFilter();
	//setupMacFilter();
#ifndef MAC_FILTER_SRC_ONLY
	setupMacFilterEbtables();
#endif
	setupMacFilterTables();
#ifdef CONFIG_IPV6
	restart_IPV6Filter();
#endif
#ifdef CONFIG_RTK_RG_INIT
	setup_RG_IPPort_Filter_Default_Policy();
#endif
	return 1;
}

#ifdef PORT_TRIGGERING
static void parse_and_add_triggerPort(char *inRange, PROTO_TYPE_T prot, char *ip)
{
	int parseLen, j;
	char tempStr1[10]={0},tempStr2[10]={0};
	char *p, dstPortRange[32];
	int idx=0,shift=0;

	parseLen = strlen(inRange);
	if (prot == PROTO_TCP)
		p = (char *)ARG_TCP;
	else
		p = (char *)ARG_UDP;

	for(j=0;j<GAMING_MAX_RANGE;j++)
	{
		if(((inRange[j]>='0')&&(inRange[j]<='9')))
		{
			if(shift>=9) continue;
			if(idx==0)
				tempStr1[shift++]=inRange[j];
			else
				tempStr2[shift++]=inRange[j];

		}
		else if((inRange[j]==',')||
				(inRange[j]=='-')||
				(inRange[j]==0))
		{
			if(idx==0)
				tempStr1[shift]=0;
			else
				tempStr2[shift]=0;

			shift=0;
			if((inRange[j]==',')||
				(inRange[j]==0))
			{
				/*
				uint16 inStart,inFinish;
				inStart=atoi(tempStr1);
				if(idx==0)
					inFinish=inStart;
				else
					inFinish=atoi(tempStr2);
				*/
				dstPortRange[0] = '\0';
				if (idx==0) // single port number
					strncpy(dstPortRange, tempStr1, 32);
				else {
					if (strlen(tempStr1)!=0 && strlen(tempStr2)!=0)
						snprintf(dstPortRange, 32, "%s:%s", tempStr1, tempStr2);
				}

				idx=0;

				// iptables -t nat -A PREROUTING -i ! $LAN_IF -p TCP --dport dstPortRange -j DNAT --to-destination ipaddr
				va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)FW_ADD,	(char *)IPTABLES_PORTTRIGGER,
					 "!", (char *)ARG_I, (char *)LANIF,
					"-p", p,
					(char *)FW_DPORT, dstPortRange, "-j",
					"DNAT", "--to-destination", ip);

				// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -p TCP --dport dstPortRange -j RETURN
				va_cmd(IPTABLES, 13, 1, (char *)FW_ADD,
					(char *)IPTABLES_PORTTRIGGER, "!", (char *)ARG_I,
					(char *)LANIF, (char *)ARG_O,
					(char *)LANIF, "-p", p,
					(char *)FW_DPORT, dstPortRange, "-j",
					(char *)FW_ACCEPT);
				/*
				//make inFinish always bigger than inStart
				if(inStart>inFinish)
				{
					uint16 temp;
					temp=inFinish;
					inFinish=inStart;
					inStart=temp;
				}

				if(!((inStart==0)||(inFinish==0)))
				{
					rtl8651a_addTriggerPortRule(
					dsid, //dsid
					inType,inStart,inFinish,
					outType,
					outStart,
					outFinish,localIp);
//					printf("inRange=%d-%d\n inType=%d\n",inStart,inFinish,inType);
				}
				*/
			}
			else
			{
				idx++;
			}
			if(inRange[j]==0)
				break;
		}
	}
}

static void setupPortTriggering(void)
{
	int i, total;
	char ipaddr[16];
	MIB_CE_PORT_TRG_T Entry;

	total = mib_chain_total(MIB_PORT_TRG_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_PORT_TRG_TBL, i, (void *)&Entry))
			continue;
		if(!Entry.enable)
			continue;

		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)Entry.ip)), 16);

		parse_and_add_triggerPort(Entry.tcpRange, PROTO_TCP, ipaddr);
		parse_and_add_triggerPort(Entry.udpRange, PROTO_UDP, ipaddr);
	}
}
#endif // of PORT_TRIGGERING

/*
 *	Setup TCPMSS for ppp in FORWARD chain
 */
static int filter_set_tcpmss(void)
{
	int i, total, min_mtu;
	char mmtu[5]="1492", mmtu_s[11];
	MIB_CE_ATM_VC_T Entry;

	min_mtu=1492;
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<total; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;
		if (Entry.enable == 0)
			continue;
		if(min_mtu > Entry.mtu)
			min_mtu = Entry.mtu;
	}
	sprintf(mmtu, "%d", min_mtu-40);
	// Added by Mason Yu
	sprintf(mmtu_s, "%d:1536", min_mtu-40);

	// for PPPoE Black Hole
	// iptables -A FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu
	{ /* Since we should NOT modify the MSS of the inbound SYN packet, we should add this rule ONLY on PPP interface. */
		va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char *)FW_FORWARD,
			"-p", "tcp", "-o", "ppp+", "--tcp-flags", "SYN,RST", "SYN", "-j",
			"TCPMSS", "--clamp-mss-to-pmtu");

		/* Because we don't want to change the br0's MTU, thus, we must patch ISP's MSS on the inbound packet. */
//		va_cmd(IPTABLES, 13, 1, (char *)FW_ADD, (char *)FW_FORWARD,
//			"-p", "tcp", "-i", "ppp+", "--tcp-flags", "SYN,RST", "SYN", "-j",
//			"TCPMSS", "--set-mss", "1452");

		// Mason Yu.
		// When the receive interface is ppp,
		// if the incoming packet's mss is greater than WAN's min mss, the incoming packet's mss must be changed to WAN's min mss,
		// if the incoming packet's mss is smaller than WAN's min mss, the incoming packet's mss need not be changed.
		va_cmd(IPTABLES, 17, 1, (char *)FW_ADD, (char *)FW_FORWARD,
			"-p", "tcp", "-i", "ppp+", "--tcp-flags", "SYN,RST", "SYN", "-m" , "tcpmss", "--mss", mmtu_s, "-j",
			"TCPMSS", "--set-mss", mmtu);
	}
}

// Execute firewall rules
// return value:
// 1  : successful
static int setupFirewall(void)
{
	char *argv[20];
	unsigned char value[32];
	int vInt, i, total, vcNum;
	MIB_CE_IP_PORT_FILTER_T IpEntry;
#ifdef MAC_FILTER
	MIB_CE_MAC_FILTER_T MacEntry;
	char mac_in_dft, mac_out_dft;
	char eth_mac_ctrl=0, wlan_mac_ctrl=0;
#endif
	MIB_CE_ATM_VC_T Entry;
	char *policy, *filterSIP, *filterDIP, srcPortRange[12], dstPortRange[12];
	char ipaddr[32], srcip[20], dstip[20];
	char ifname[16], extra[32];
	char srcmacaddr[18], dstmacaddr[18];
#ifdef IP_PASSTHROUGH
	unsigned int ippt_itf;
#endif
	int spc_enable, spc_ip;
	// Added by Mason Yu for ACL
	unsigned char aclEnable, domainEnable;
	unsigned char dhcpvalue[32];
	unsigned char vChar;
	int dhcpmode;
	char str_vc[] = "vc0";
	// Added by Mason Yu for URL Blocking
#if defined( URL_BLOCKING_SUPPORT)||defined( URL_ALLOWING_SUPPORT)
	unsigned char urlEnable;
#endif
#ifdef NAT_CONN_LIMIT
	unsigned char connlimitEn;
#endif

#ifdef CONFIG_RTK_RG_INIT
	char sysbuf[128];
	//iptables -F
	//iptables -P INPUT ACCEPT
	sprintf(sysbuf,"/bin/echo 1 > /proc/sys/net/ipv4/ip_forward");
	printf("system(): %s\n",sysbuf);
	system(sysbuf);
#endif

	//-------------------------------------------------
	// Filter table
	//-------------------------------------------------
	//--------------- set policies --------------
	// iptables -P INPUT DROP
	va_cmd(IPTABLES, 3, 1, "-P", (char *)FW_INPUT, (char *)FW_DROP);

	// iptables -P FORWARD ACCEPT
	va_cmd(IPTABLES, 3, 1, "-P", (char *)FW_FORWARD, (char *)FW_ACCEPT);

	// accept related
	// iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-m", "state",
		"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_ACCEPT);

	// It must be in the beginning of FORWARD chain.
	filter_set_tcpmss();
#ifdef CONFIG_LUNA_DUAL_LINUX
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_INPUT, "-i", "vwlan", "-j", (char *)FW_ACCEPT);
#endif
#ifdef CONFIG_USER_FON
	setFonFirewall();
#endif
	// Kaohj -- allowing RIP (in case of ACL blocking)
	// iptables -A INPUT -p udp --dport 520 -j ACCEPT
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-p",
		"udp", (char *)FW_DPORT, "520", "-j", (char *)FW_ACCEPT);

	// Drop UPnP SSDP from the WAN side
	va_cmd(IPTABLES, 9, 1, (char *)FW_ADD, (char *)FW_INPUT, "!", (char *)ARG_I, (char *)LANIF, "-d", "239.255.255.250", "-j", (char *)FW_DROP);

	// Allowing multicast access, ie. IGMP, RIPv2
	// iptables -A INPUT -d 224.0.0.0/4 -j ACCEPT
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_INPUT, "-d",
		"224.0.0.0/4", "-j", (char *)FW_ACCEPT);
#ifdef IP_ACL
	//  Add Chain(aclblock) for ACL
	va_cmd(IPTABLES, 2, 1, "-N", "aclblock");
	mib_get(MIB_ACL_CAPABILITY, (void *)&aclEnable);
	if (aclEnable == 1)  // ACL Capability is enabled
		filter_set_acl(1);
	//iptables -A INPUT -j aclblock
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", "aclblock");
#endif

#ifdef NAT_CONN_LIMIT
	//Add Chain(connlimit) for NAT conn limit
	va_cmd(IPTABLES, 2, 1, "-N", "connlimit");

	mib_get(MIB_NAT_CONN_LIMIT, (void *)&connlimitEn);
	if (connlimitEn == 1)
		set_conn_limit();

	//iptables -A FORWARD -j connlimit
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "connlimit");
#endif
#ifdef TCP_UDP_CONN_LIMIT
	//Add Chain(connlimit) for NAT conn limit
	va_cmd(IPTABLES, 2, 1, "-N", "connlimit");

	mib_get(MIB_CONNLIMIT_ENABLE, (void *)&vChar);
	if (vChar == 1)
		set_conn_limit();
	//iptables -A FORWARD -j connlimit
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "connlimit");
#endif

	block_br_wan();

#ifdef CONFIG_USER_RTK_VOIP
	voip_setup_iptable();
#endif

	// setup chain for wlan blocking
	va_cmd(EBTABLES, 4, 1, "-N", "wlan_block", "-P", "RETURN");
	va_cmd(EBTABLES, 4, 1, "-A", "FORWARD", "-j", "wlan_block");

#ifdef PPPOE_PASSTHROUGH
	//  W.H. Hung: Add Chain br_pppoe for bridged PPPoE
	va_cmd(EBTABLES, 2, 1, "-N", FW_BR_PPPOE);
	va_cmd(EBTABLES, 3, 1, "-P", FW_BR_PPPOE, "RETURN");
	va_cmd(EBTABLES, 4, 1, "-t", "broute", "-N", FW_BR_PPPOE);
	va_cmd(EBTABLES, 5, 1, "-t", "broute", "-P", FW_BR_PPPOE, "RETURN");
	setupBrPppoe();
	// and to FORWORD chain
	va_cmd(EBTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", FW_BR_PPPOE);
	va_cmd(EBTABLES, 6, 1, "-t", "broute", (char *)FW_ADD, "BROUTING", "-j", FW_BR_PPPOE);
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
	//  Add Chain(domainblk) for ACL
	va_cmd(IPTABLES, 2, 1, "-N", "domainblk");
	mib_get(MIB_DOMAINBLK_CAPABILITY, (void *)&domainEnable);
	if (domainEnable == 1)  // Domain blocking Capability is enabled
		filter_set_domain(1);
	//iptables -A INPUT -j domainblk
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", "domainblk");

	//iptables -A FORWARD -j domainblk
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "domainblk");

	//iptables -A OUTPUT -j domainblk
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, "OUTPUT", "-j", "domainblk");
#endif

#ifdef IP_PASSTHROUGH
	// IP Passthrough, LAN access
	set_IPPT_LAN_access();
#endif

#ifdef CONFIG_USER_CWMP_TR069
	va_cmd(IPTABLES, 2, 1, "-N", (char *)IPTABLE_TR069);
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, "INPUT", "-j", (char *)IPTABLE_TR069);
#endif

#ifdef CONFIG_MIDDLEWARE
	//setMidwareRouteFW(0);
#endif

#ifdef REMOTE_ACCESS_CTL
	// Add chain for remote access
	// iptables -N inacc
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_INACC);
	va_cmd(IPTABLES, 11, 1, "-A", (char *)FW_INACC,
		 "!", (char *)ARG_I, LANIF, "-m", "mark", "--mark", RMACC_MARK, "-j", FW_ACCEPT);

#ifdef CONFIG_IPV6
	va_cmd(IP6TABLES, 2, 1, "-N", (char *)FW_IPV6REMOTEACC);
#endif

	filter_set_remote_access(1);

	// Added by Mason Yu for dhcp Relay. Open DHCP Relay Port for Incoming Packets.
	if (mib_get(MIB_DHCP_MODE, (void *)dhcpvalue) != 0)
	{
		dhcpmode = (unsigned int)(*(unsigned char *)dhcpvalue);
		if (dhcpmode == DHCP_LAN_SERVER || dhcpmode == DHCP_LAN_RELAY){
			// iptables -A inacc -i ! br0 -p udp --dport 67 -j ACCEPT
			va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, (char *)FW_INACC, "!", (char *)ARG_I, BRIF, "-p", "udp", (char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_ACCEPT);
		}
	}

#ifdef CONFIG_USER_ROUTED_ROUTED
	// Added by Mason Yu. Open RIP Port for Incoming Packets.
	if (mib_get( MIB_RIP_ENABLE, (void *)&vChar) != 0)
	{
		if (1 == vChar)
		{
			// iptables -A inacc -i ! br0 -p udp --dport 520 -j ACCEPT
			va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, (char *)FW_INACC, "!", "-i", BRIF, "-p", "udp", (char *)FW_DPORT, "520", "-j", (char *)FW_ACCEPT);
		}
	}
#endif

	// iptables -A INPUT -j inacc
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", (char *)FW_INACC);
#endif // of REMOTE_ACCESS_CTL

	// Added by Mason Yu for accept packet with ip(127.0.0.1)
	// Magician: Modify this rule to allow to ping self. Merge 192.168.1.1-to-192.168.1.1 accepted rule and to-127.0.0.1 accpeted rule.
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_INPUT, "-i", "lo", "-j", (char *)FW_ACCEPT);

	// Single PC mode
	if (mib_get(MIB_SPC_ENABLE, (void *)value) != 0)
	{
		if (value[0]) // Single PC mode enabled
		{
			spc_enable = 1;
			mib_get(MIB_SPC_IPTYPE, (void *)value);
			if (value[0] == 0) // single private IP
			{
				spc_ip = 0;
				mib_get(MIB_ADSL_LAN_IP, (void *)value);

				// IP pool start address
				// Kaohj
				#ifndef DHCPS_POOL_COMPLETE_IP
				mib_get(MIB_ADSL_LAN_CLIENT_START, (void *)&value[3]);
				strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
				#else
				mib_get(MIB_DHCP_POOL_START, (void *)value);
				strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
				#endif
				ipaddr[15] = '\0';
				// iptables -A FORWARD -i $LANIF -s ! ipaddr -j DROP
				va_cmd(IPTABLES, 9, 1, (char *)FW_ADD,
					(char *)FW_FORWARD, (char *)ARG_I,
					(char *)LANIF, "!", "-s", ipaddr,
					"-j", (char *)FW_DROP);
			}
			else // single IP passthrough
			{
				spc_ip = 1;
			}
		}
		else
			spc_enable = 0;
	}

#ifdef IP_PASSTHROUGH
	// check vc
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;

		if (spc_enable && (spc_ip == 1)) // single IP passthrough (public IP)
		{
			if ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_RT1483 &&
				ippt_itf == Entry.ifIndex)
			{	// ippt WAN interface (1483-r)
				if ((DHCP_T)Entry.ipDhcp == DHCP_DISABLED)
				{
					strncpy(ipaddr, inet_ntoa(*((struct in_addr *)Entry.ipAddr)), 16);
					ipaddr[15] = '\0';
					// iptables -A FORWARD -i $LANIF -s ! ipaddr -j DROP
					va_cmd(IPTABLES, 9, 1, (char *)FW_ADD,
						(char *)FW_FORWARD, (char *)ARG_I,
						(char *)LANIF, "!", "-s", ipaddr,
						"-j", (char *)FW_DROP);
				}
			}
		}
	}
#endif

#if defined(URL_BLOCKING_SUPPORT)||defined(URL_ALLOWING_SUPPORT)
 mib_get(MIB_URL_CAPABILITY, (void *)&urlEnable);
#endif

#ifdef URL_BLOCKING_SUPPORT
// Added by Mason Yu for URL Blocking
//  Add Chain(urlblock) for URL
 va_cmd(IPTABLES, 2, 1, "-N", "urlblock");
 if (urlEnable == 1)  // URL Capability enabled
   filter_set_url(1);
  //iptables -A FORWARD -j urlblock
   va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "urlblock");
 #endif

 #ifdef URL_ALLOWING_SUPPORT
   va_cmd(IPTABLES,2,1 ,"-N","urlallow");
   if(urlEnable==2)
		filter_set_url(2); //alex

 	//iptables -A FORWARD -j urlallow
   va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "urlallow");
#endif

#ifdef LAYER7_FILTER_SUPPORT
	//App Filter
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_APPFILTER);
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_APP_P2PFILTER);
	va_cmd(IPTABLES, 4, 1, ARG_T, "mangle", "-N", (char *)FW_APPFILTER);
	setupAppFilter();
	// iptables -A FORWARD -j appfilter
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_APPFILTER);
	// iptables -t mangle -A POSTROUTING -j appfilter
	va_cmd(IPTABLES, 6, 1, ARG_T, "mangle", (char *)FW_ADD, (char *)FW_POSTROUTING, "-j", (char *)FW_APPFILTER);
#endif

#ifdef PORT_FORWARD_GENERAL
	// port forwarding
	// Add New chain on filter and nat
	// iptables -N portfw
	va_cmd(IPTABLES, 2, 1, "-N", (char *)PORT_FW);
	// iptables -A FORWARD -j portfw
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)PORT_FW);
	// iptables -t nat -N portfw
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)PORT_FW);
	// iptables -t nat -A PREROUTING -j portfw
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)PORT_FW);
	setupPortFW();
#endif

#ifdef PORT_FORWARD_ADVANCE
	setupPFWAdvance();
#endif

#ifdef NATIP_FORWARDING
	fw_setupIPForwarding();
#endif

#ifdef PORT_TRIGGERING
	// Add New chain on filter and nat
	// iptables -N portTrigger
	va_cmd(IPTABLES, 2, 1, "-N", (char *)IPTABLES_PORTTRIGGER);
	// iptables -A FORWARD -j portTrigger
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)IPTABLES_PORTTRIGGER);
	// iptables -t nat -N portTrigger
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)IPTABLES_PORTTRIGGER);
	// iptables -t nat -A PREROUTING -j portTrigger
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)IPTABLES_PORTTRIGGER);
	setupPortTriggering();
#endif
	// IP Filter
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_IPFILTER);
	setupIPFilter();
	// iptables -A FORWARD -j ipfilter
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_IPFILTER);

	// DMZ
	// Add New chain on filter and nat
	// iptables -N dmz
	va_cmd(IPTABLES, 2, 1, "-N", (char *)IPTABLE_DMZ);
	// iptables -t nat -N dmz
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)IPTABLE_DMZ);
	setupDMZ();
	// iptables -A filter -j dmz
	va_cmd(IPTABLES, 4, 1, "-A", (char *)FW_IPFILTER, "-j", (char *)IPTABLE_DMZ);
	// iptables -t nat -A PREROUTING -j dmz
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)IPTABLE_DMZ);

	// Set IP filter default action
	setup_default_IPFilter();

	// Mac Filter
	va_cmd(EBTABLES, 2, 1, "-N", (char *)FW_MACFILTER);
	//setupMacFilter();

#ifndef MAC_FILTER_SRC_ONLY
	// ebtables -A FORWARD -j macfilter
	va_cmd(EBTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_MACFILTER);
	setupMacFilterEbtables();
#endif

	// Mac Filter SRC
#ifdef MAC_FILTER_SRC_ONLY
	va_cmd(EBTABLES, 2, 1, "-N", (char *)FW_MACFILTER_ROUTER);
	va_cmd(EBTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_MACFILTER_ROUTER);
#endif
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)FW_MACFILTER_ROUTER);
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_INSERT, (char *)FW_PREROUTING, "-j", (char *)FW_MACFILTER_ROUTER);
	setupMacFilterTables();

#ifdef SUPPORT_ACCESS_RIGHT
	apply_accessRight(1);
#endif
#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
	apply_maxBandwidth(1);
#endif

#if defined CONFIG_RTK_RG_INIT && defined CONFIG_APOLLO_ROMEDRIVER
	// ebtables -P FORWARD DROP, Rome driver will do it for Kernel.
	va_cmd(EBTABLES, 3, 1, "-P", (char *)FW_FORWARD, (char *)FW_DROP);
	va_cmd(EBTABLES, 2, 1, "-N", "disBCMC");
	//we want to drop RA from bridge internet WAN, we trap icmpv6 packet to PS.
	//but the kernel ebtables will drop forward path due Rome driver.
	//the mld queries is also icmpv6 which type is 130, we let it pass in here!
	
	va_cmd(EBTABLES, 10, 1, (char *)FW_ADD, "disBCMC", "-p", "IPv6", "--ip6-proto","58","--ip6-icmp-type","130","-j", FW_RETURN);
	va_cmd(EBTABLES, 6, 1, (char *)FW_ADD, "disBCMC", "-d", "Broadcast", "-j", FW_DROP);
	va_cmd(EBTABLES, 6, 1, (char *)FW_ADD, "disBCMC", "-d", "Multicast", "-j", FW_DROP);
	va_cmd(EBTABLES, 4, 1, (char *)FW_INSERT, (char *)FW_FORWARD, "-j", "disBCMC");
	va_cmd(EBTABLES, 3, 1, "-P", "disBCMC", (char *)FW_RETURN);
#endif

#ifdef NEW_PORTMAPPING
	//Drop DHCP requests from LAN if DHCP is disabled on WAN
	va_cmd(EBTABLES, 2, 1, "-N", FW_DHCPS_DIS);
	va_cmd(EBTABLES, 3, 1, "-P", FW_DHCPS_DIS, "RETURN");
	//Check packets from DHCP client
	va_cmd(EBTABLES, 16, 1, (char *)FW_ADD, (char *)FW_INPUT, "-p", "ip"
		, "--ip-protocol", "17", "--ip-sport", "68", "--ip-dport", "67"
		, "--ip-src", "0.0.0.0", "--ip-dst", "255.255.255.255", "-j", FW_DHCPS_DIS);
#endif

#ifdef LAYER7_FILTER_SUPPORT
	// iptables -A FORWARD -j appp2pfilter
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_APP_P2PFILTER);
#endif

#ifdef VIRTUAL_SERVER_SUPPORT
	//execute virtual server FORWARD rules
	va_cmd(IPTABLES, 2, 1, "-N", (char *)IPTABLE_VTLSUR);
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)IPTABLE_VTLSUR);

	//execute virtual server NAT prerouting rules
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", (char *)IPTABLE_VTLSUR);
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", (char *)IPTABLE_VTLSUR);

	setupVtlsvr(VIRTUAL_SERVER_ADD);
#endif

 	// for multicast
#if 0 // move ahead to ipfilter chain
	// iptables -A FORWARD -d 224.0.0.0/4 -j ACCEPT
	va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-d",
		"224.0.0.0/4", "-j", (char *)FW_ACCEPT);
#endif

	// Kaohj --- only the LAN ports in the same group with IGMP proxy upstream interface
	// be able to accept IGMP messages
#if (defined(ITF_GROUP) && defined(CONFIG_USER_IGMPPROXY))
#ifdef CONFIG_IGMPPROXY_MULTIWAN
	mib_get(MIB_MPMODE, (void *)&value[0]);
	if(value[0]&MP_PMAP_MASK){ //Port-mapping enable
		total = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				continue;
			if (!Entry.enable)
				continue;
			if (Entry.enableIGMP){ // igmpproxy upstream interface
                      		int k;
				int swtotal;
				MIB_CE_SW_PORT_T lanPort;
				swtotal = mib_chain_total(MIB_SW_PORT_TBL);
				for (k=0; k<swtotal; k++) {
					if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&lanPort))
						continue;
					if (lanPort.itfGroup != Entry.itfGroup){
						 //disable LAN IGMP input for other groups
						char vp[] = "1";
						vp[0] = '1' + i;
						//				printf("inport i = %d  phy %s \n",i,phy);
						// iptables -A INPUT -d 224.0.0.0/4 -i eth0_sw0 -j DROP
						// Note: the mark(nfmark) is tagged at NIC driver(re8670.c)
						//					va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-d",
						//					"224.0.0.0/4", (char *)ARG_I,  phy, "-j", (char *)FW_DROP);
						//                                iptables -A INPUT -d 224.0.0.0/4 -m mark --mark port+1 -j DROP
						// Note: the mark(nfmark) is tagged at NIC driver(re8670.c)
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INPUT, "-d",
						"224.0.0.0/4", "-m", "mark", "--mark", vp, "-j", (char *)FW_DROP);
					}
				}
			}

	   }
	}
#else
	mib_get(MIB_MPMODE, (void *)&value[0]);
	mib_get(MIB_IGMP_PROXY, (void *)&value[1]);
	if ((value[0]&MP_PMAP_MASK) && (value[1] == 1)) { // Port-mapping && igmp proxy enabled
		mib_get(MIB_IGMP_PROXY_ITF, (void *)&vInt);
		total = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				continue;
			if (!Entry.enable)
				continue;
			if (Entry.ifIndex == vInt)	// igmpproxy upstream interface
				break;
		}
		if (i!=total) {	// found igmpproxy upstream interface
			MIB_CE_SW_PORT_T lanPort;
			total = mib_chain_total(MIB_SW_PORT_TBL);
			for (i=0; i<total; i++) {
				if (!mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&lanPort))
					continue;
				if (lanPort.itfGroup != Entry.itfGroup) {
#if 0
					// disable LAN IGMP input for other groups
					char vp[] = "1";
					vp[0] = '1' + i;
					// iptables -A INPUT -d 224.0.0.0/4 -m mark --mark port+1 -j DROP
					// Note: the mark(nfmark) is tagged at NIC driver(re8670.c)
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INPUT, "-d",
					"224.0.0.0/4", "-m", "mark", "--mark", vp, "-j", (char *)FW_DROP);
#endif
                             //disable LAN IGMP input for other groups
					char phy[]="eth0_sw0";
					phy[7] = '0'+ i;
	//				printf("inport i = %d  phy %s \n",i,phy);
					// iptables -A INPUT -d 224.0.0.0/4 -i eth0_sw0 -j DROP
					// Note: the mark(nfmark) is tagged at NIC driver(re8670.c)
					va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-d",
					"224.0.0.0/4", (char *)ARG_I,  phy, "-j", (char *)FW_DROP);


				}
			}
		}
	}
#endif
#endif // of CONFIG_IGMPPROXY_MULTIWAN
	// iptables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
	// andrew. moved to the 1st rule, or returning PING/DNS relay will be blocked.
	//va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-m", "state",
	//	"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_ACCEPT);

	// iptables -A INPUT -m state --state NEW -i $LAN_INTERFACE -j ACCEPT
	va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INPUT, "-m", "state",
		"--state", "NEW", (char *)ARG_I, (char *)LANIF,
		"-j", (char *)FW_ACCEPT);

#if 0
	/*--------------------------------------------------------------------
	 * The following are the default action and should be final rules
	 -------------------------------------------------------------------*/
	// iptables -N block
	va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_BLOCK);

	// default action
	if (mib_get(MIB_IPF_OUT_ACTION, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned char *)value);
		if (vInt == 1)	// ACCEPT
		{
			// iptables -A block -i $LAN_IF -j ACCEPT
			va_cmd(IPTABLES, 6, 1, (char *)FW_ADD,
				(char *)FW_BLOCK, (char *)ARG_I,
				(char *)LANIF, "-j", (char *)FW_ACCEPT);
		}
	}

	if (mib_get(MIB_IPF_IN_ACTION, (void *)value) != 0)
	{
		vInt = (int)(*(unsigned char *)value);
		if (vInt == 1)	// ACCEPT
		{
			// iptables -A block -i ! $LAN_IF -j ACCEPT
			va_cmd(IPTABLES, 7, 1, (char *)FW_ADD,
				(char *)FW_BLOCK, "!", (char *)ARG_I,
				(char *)LANIF, "-j", (char *)FW_ACCEPT);
		}
	}

	// iptables -A block -m state --state ESTABLISHED,RELATED -j ACCEPT
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_BLOCK, "-m", "state",
		"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_ACCEPT);

	/*
	// iptables -A block -m state --state NEW -i $LAN_INTERFACE -j ACCEPT
	va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_BLOCK, "-m", "state",
		"--state", "NEW", (char *)ARG_I, (char *)LANIF,
		"-j", (char *)FW_ACCEPT);
	*/

	// iptables -A block -j DROP
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, "block", "-j", (char *)FW_DROP);

	/*
	// iptables -A INPUT -j block
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", "block");
	*/

	// iptables -A FORWARD -j block
	va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", "block");
#endif

#ifdef CONFIG_IPV6
	// INPUT default DROP , ip6table -P INPUT  DROP
	va_cmd(IP6TABLES, 3, 1, "-P", (char *)FW_INPUT, (char *)FW_DROP);

	// ip6tables -A INPUT -p icmpv6 --icmpv6-type router-advertisement -j ACCEPT
	va_cmd(IP6TABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-p", "icmpv6",
		"--icmpv6-type","router-advertisement","-j", (char *)FW_ACCEPT);

    // ip6tables -I INPUT  -p icmpv6 --icmpv6-type neighbour-solicitation -j ACCEPT
    va_cmd(IP6TABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-p", "icmpv6",
            "--icmpv6-type","neighbour-solicitation","-j", (char *)FW_ACCEPT);

    // ip6tables -I INPUT  -p icmpv6 --icmpv6-type neighbour-advertisement -j ACCEPT
    va_cmd(IP6TABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-p", "icmpv6",
            "--icmpv6-type","neighbour-advertisement","-j", (char *)FW_ACCEPT);

	// ip6tables -A INPUT -p icmpv6 -i $LAN_INTERFACE -j ACCEPT
	va_cmd(IP6TABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-p", "icmpv6",
	(char *)ARG_I, (char *)LANIF, "-j", (char *)FW_ACCEPT);

	// ip6tables -A INPUT -m state --state NEW -i $LAN_INTERFACE -j ACCEPT
	va_cmd(IP6TABLES, 10, 1, (char *)FW_ADD, (char *)FW_INPUT, "-m", "state",
		"--state", "NEW", (char *)ARG_I, (char *)LANIF,
		"-j", (char *)FW_ACCEPT);

	// ip6tables -A INPUT -m state --state ESTABLISHED,RELATED -j ACCEPT
	va_cmd(IP6TABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-m", "state",
			"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_ACCEPT);

	// ip6tables  -A INPUT -p udp --dport  546 -j ACCEPT
	va_cmd(IP6TABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, "-p", "udp",
			"--dport", "546", "-j", (char *)FW_ACCEPT);

	// ip6tables  -A INPUT -i $LAN_INTERFACE -p udp --dport  547 -j ACCEPT
	va_cmd(IP6TABLES, 8, 1, (char *)FW_ADD, (char *)FW_INPUT, (char *)ARG_I, (char *)LANIF,
			"-p", "udp", "--dport", "547", "-j", (char *)FW_ACCEPT);

	// IPv6 Filter
	va_cmd(IP6TABLES, 2, 1, "-N", (char *)FW_IPV6FILTER);
	setupIPV6Filter();
	// ip6tables -A FORWARD -j ipv6filter
	va_cmd(IP6TABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_IPV6FILTER);

	// Set IPV6 filter default action
	setup_default_IPV6Filter();

	// ip6tables -I INPUT -j ipv6remoteacc
	va_cmd(IP6TABLES, 4, 1, (char *)FW_INSERT, (char *)FW_INPUT, "-j", (char *)FW_IPV6REMOTEACC);
#endif
#ifdef CONFIG_RTK_RG_INIT
	setup_RG_IPPort_Filter_Default_Policy();
#endif

	return 1;
}
#ifdef CONFIG_IP_NF_ALG_ONOFF
//add by ramen return 1--sucessful
// Mason Yu. alg_onoff_20101023
int setupAlgOnOff(void)
{
	unsigned char value=0;
#ifndef CONFIG_RTK_RG_INIT
#ifdef CONFIG_NF_CONNTRACK_FTP
	if(mib_get(MIB_IP_ALG_FTP,&value)&& value==0)
		system("/bin/echo 0 > /proc/algonoff_ftp");
	else
		system("/bin/echo 1 > /proc/algonoff_ftp");
#endif
#ifdef CONFIG_NF_CONNTRACK_H323
	if(mib_get(MIB_IP_ALG_H323,&value)&& value==0)
		system("/bin/echo 0 >/proc/algonoff_h323");
	else
		system("/bin/echo 1 >/proc/algonoff_h323");
#endif
#ifdef CONFIG_NF_CONNTRACK_IRC
	if(mib_get(MIB_IP_ALG_IRC,&value)&& value==0)
		system("/bin/echo 0 >/proc/algonoff_irc");
	else
		system("/bin/echo 1 >/proc/algonoff_irc");
#endif
#ifdef CONFIG_NF_CONNTRACK_RTSP
	if(mib_get(MIB_IP_ALG_RTSP,&value)&& value==0)
		system("/bin/echo 0 >/proc/algonoff_rtsp");
	else
		system("/bin/echo 1 >/proc/algonoff_rtsp");
#endif
#ifdef CONFIG_NF_CONNTRACK_QUAKE3
	if(mib_get(MIB_IP_ALG_QUAKE3,&value)&& value==0)
		system("/bin/echo 0 > /proc/algonoff_quake3");
	else
		system("/bin/echo 1 > /proc/algonoff_quake3");
#endif
#ifdef CONFIG_NF_CONNTRACK_CUSEEME
	if(mib_get(MIB_IP_ALG_CUSEEME,&value)&& value==0)
		system("/bin/echo 0 > /proc/algonoff_cuseeme");
	else
		system("/bin/echo 1 > /proc/algonoff_cuseeme");
#endif
#ifdef CONFIG_NF_CONNTRACK_L2TP
	if(mib_get(MIB_IP_ALG_L2TP,&value)&& value==0)
		system("/bin/echo 0 > /proc/algonoff_l2tp");
	else
		system("/bin/echo 1 > /proc/algonoff_l2tp");
#endif
#ifdef CONFIG_NF_CONNTRACK_IPSEC
	if(mib_get(MIB_IP_ALG_IPSEC,&value)&& value==0)
		system("/bin/echo 0 > /proc/algonoff_ipsec");
	else
		system("/bin/echo 1 > /proc/algonoff_ipsec");
#endif
#ifdef CONFIG_NF_CONNTRACK_SIP
	if(mib_get(MIB_IP_ALG_SIP,&value)&& value==0)
		system("/bin/echo 0 > /proc/algonoff_sip");
	else
		system("/bin/echo 1 > /proc/algonoff_sip");
#endif
#ifdef CONFIG_NF_CONNTRACK_PPTP
	if(mib_get(MIB_IP_ALG_PPTP,&value)&& value==0)
		system("/bin/echo 0 > /proc/algonoff_pptp");
	else
		system("/bin/echo 1 > /proc/algonoff_pptp");
#endif
#endif // !def CONFIG_RTK_RG_INIT
#ifdef CONFIG_RTK_RG_INIT
	RTK_RG_FLUSH_ALG_FILTER_RULE();
	RTK_RG_ALG_Set();
#endif
	return;
}
#endif


#ifdef WEB_REDIRECT_BY_MAC
static int start_web_redir_by_mac(void)
{
	int status=0;
	char tmpbuf[MAX_URL_LEN];
	char ipaddr[16], ip_port[32], redir_server[33];
	int  def_port=WEB_REDIR_BY_MAC_PORT;

	ipaddr[0]='\0'; ip_port[0]='\0';redir_server[0]='\0';
	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		ipaddr[15] = '\0';
		sprintf(ip_port,"%s:%d",ipaddr,def_port);
	}//else ??

	// Enable Bridge Netfiltering
	//status|=va_cmd("/bin/brctl", 2, 0, "brnf", "on");

	//iptables -t nat -N WebRedirectByMAC
	status|=va_cmd(IPTABLES, 4, 1, "-t", "nat","-N","WebRedirectByMAC");

	//iptables -t nat -A WebRedirectByMAC -d 192.168.1.1 -j RETURN
	status|=va_cmd(IPTABLES, 8, 1, "-t", "nat","-A","WebRedirectByMAC",
		"-d", ipaddr, "-j", (char *)FW_RETURN);

	//iptables -t nat -A WebRedirectByMAC -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:8080
	status|=va_cmd(IPTABLES, 12, 1, "-t", "nat","-A","WebRedirectByMAC",
		"-p", "tcp", "--dport", "80", "-j", "DNAT",
		"--to-destination", ip_port);

	//iptables -t nat -A PREROUTING -i eth0 -p tcp --dport 80 -j WebRedirectByMAC
	status|=va_cmd(IPTABLES, 12, 1, "-t", "nat","-A","PREROUTING",
		"-i", ELANIF,
		"-p", "tcp", "--dport", "80", "-j", "WebRedirectByMAC");


{
	int num,i;
	unsigned char tmp2[18];
	MIB_WEB_REDIR_BY_MAC_T	wrm_entry;

	num = mib_chain_total( MIB_WEB_REDIR_BY_MAC_TBL );
	//printf( "\nnum=%d\n", num );
	for(i=0;i<num;i++)
	{
		if( !mib_chain_get( MIB_WEB_REDIR_BY_MAC_TBL, i, (void*)&wrm_entry ) )
			continue;

		sprintf( tmp2, "%02X:%02X:%02X:%02X:%02X:%02X",
				wrm_entry.mac[0], wrm_entry.mac[1], wrm_entry.mac[2], wrm_entry.mac[3], wrm_entry.mac[4], wrm_entry.mac[5] );
		//printf( "add one mac: %s \n", tmp2 );
		// iptables -A macfilter -i eth0  -m mac --mac-source $MAC -j ACCEPT/DROP
		status|=va_cmd("/bin/iptables", 10, 1, "-t", "nat", "-I", "WebRedirectByMAC", "-m", "mac", "--mac-source", tmp2, "-j", "RETURN");
	}
}

	return 0;
}
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
int start_captiveportal(void)
{
	int status = 0;
	char tmpbuf[MAX_URL_LEN];
	char lan_ipaddr[16], lan_ip_port[32];
	char ip_mask[24];
	int  def_port = CAPTIVEPORTAL_PORT, i, num;
	FILE *fp;

	lan_ipaddr[0] = '\0';
	lan_ip_port[0] = '\0';

	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
	{
		strncpy(lan_ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		lan_ipaddr[15] = '\0';
		sprintf(lan_ip_port, "%s:%d", lan_ipaddr, def_port);
	}
	else
		return -1;

	//iptables -t nat -A PREROUTING -p tcp -d 192.168.1.1 --dport 80 -j RETURN
	status |= va_cmd(IPTABLES, 12, 1, "-t", "nat", "-A", "PREROUTING",	"-p", "tcp", "-d", lan_ipaddr, "--dport", "80", "-j", (char *)FW_RETURN);

	//iptables -t nat -A PREROUTING -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:18182
	status |= va_cmd(IPTABLES, 12, 1, "-t", "nat", "-A", "PREROUTING", "-p", "tcp", "--dport", "80", "-j", "DNAT", "--to-destination", lan_ip_port);

	CWMP_CAPTIVEPORTAL_ALLOWED_LIST_T ccal_entry;

	if(!(fp = fopen("/var/cp_allow_ip", "w")))
	{
		fprintf(stderr, "Open file cp_allow_ip failed!");
		return -1;
	}

	num = mib_chain_total(CWMP_CAPTIVEPORTAL_ALLOWED_LIST);

	for( i = 0; i < num; i++ )
	{
		if(!mib_chain_get(CWMP_CAPTIVEPORTAL_ALLOWED_LIST, i, (void*)&ccal_entry))
			continue;

		if( ccal_entry.mask < 32 ) // Valid subnet mask
			sprintf(ip_mask, "%s/%d", inet_ntoa(*(struct in_addr *)&ccal_entry.ip_addr), ccal_entry.mask);
		else	// Invalid subnet mask or unset subnet mask.
			sprintf(ip_mask, "%s", inet_ntoa(*(struct in_addr *)&ccal_entry.ip_addr));

		// iptables -t nat -I PREROUTING -p tcp -d 209.85.175.104/24 --dport 80 -j RETURN
		status |= va_cmd(IPTABLES, 12, 1, "-t", "nat", "-I", "PREROUTING", "-p", "tcp", "-d", ip_mask, "--dport", "80", "-j", "RETURN");
		fprintf(fp, "%s\n", ip_mask);
	}

	fclose(fp);

	return status;
}

int stop_captiveportal(void)
{
	int status = 0;
	char tmpbuf[MAX_URL_LEN];
	char lan_ipaddr[16], lan_ip_port[32];
	char ip_mask[24];
	int  def_port = CAPTIVEPORTAL_PORT, i;
	FILE *fp;

	if(fp = fopen("/var/cp_allow_ip", "r"))
	{
		char *tmp;

		while(fgets(ip_mask, 24, fp))
		{
			tmp = strchr(ip_mask, '\n');
			*tmp = '\0';

			// iptables -t nat -D PREROUTING -p tcp -d 209.85.175.104/24 --dport 80 -j RETURN
			va_cmd(IPTABLES, 12, 1, "-t", "nat", "-D", "PREROUTING", "-p", "tcp", "-d", ip_mask, "--dport", "80", "-j", "RETURN");
		}

		fclose(fp);
		unlink("/var/cp_allow_ip");
	}

	lan_ipaddr[0] = '\0';
	lan_ip_port[0] = '\0';

	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
	{
		strncpy(lan_ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		lan_ipaddr[15] = '\0';
		sprintf(lan_ip_port, "%s:%d", lan_ipaddr, def_port);
	}
	else
		return -1;

	//iptables -t nat -D PREROUTING -p tcp -d 192.168.1.1 --dport 80 -j RETURN
	status |= va_cmd(IPTABLES, 12, 1, "-t", "nat", "-D", "PREROUTING", "-p", "tcp", "-d", lan_ipaddr, "--dport", "80", "-j", (char *)FW_RETURN);

	//iptables -t nat -D PREROUTING -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:18182
	status |= va_cmd(IPTABLES, 12, 1, "-t", "nat", "-D", "PREROUTING", "-p", "tcp", "--dport", "80", "-j", "DNAT", "--to-destination", lan_ip_port);

	return status;
}
#endif

#ifdef SUPPORT_MCAST_TEST
#define IPTV_BUSSINESS_OK   0
#define NO_IPTV_CONNECTION -1
#define IPTV_DISCONNECT    -2
#define INVALID_MULTIVLAN  -3
#define IPTV_BUSSINESS_NOK -4

#define ROMEDRIVER_IGMP_PROC "/proc/rg/igmpSnooping"
#define ROMEDRIVER_IGMP_INFO "address"

/*return value: 0 test success; 1 test fail*/
int multicastTest(int * diagResult)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[IFNAMSIZ];
	int flags, flags_found;
	FILE *fp;
	char buf[200] ={0};
	char cmd[100];
	int isBridgeUp = 0, hasMcVlan = 0, hasMcGroup = 0;
	int ret = 1;

#ifdef CONFIG_GPON_FEATURE
	rtk_gpon_fsm_status_t onu;
	#if defined(CONFIG_RTK_L34_ENABLE)
			rtk_rg_gpon_ponStatus_get(&onu);
	#else
			rtk_gpon_ponStatus_get(&onu);
	#endif
#endif

	/*check if there is a linkup bridge wan*/
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 1; i < entryNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return;

		if (Entry.cmode == CHANNEL_MODE_BRIDGE) 
		{
			ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
			flags_found = getInFlags(ifname, &flags);

			if (flags_found) 
			{
				if (flags & IFF_UP) 
				{
#ifdef CONFIG_GPON_FEATURE
					if (onu == 5)
						isBridgeUp = 1;
#else
					isBridgeUp = 1;
#endif
				}
			}
			/*check if entry has multicast vlan*/
			if(Entry.mVid > 0)
				hasMcVlan = 1;
			break;
		}
	}
	
	/*check if there is multicast group info*/
	//need to enable rg proc pipe function, rg proc pipe function is disable by default
	sprintf(cmd, "echo 1 > /proc/rg/proc_to_pipe");
	va_cmd("/bin/sh", 2, 1, "-c", cmd);
	
	fp = fopen(ROMEDRIVER_IGMP_PROC,"r");
	if(fp)
	{
		while (fgets(buf, 200, fp) != NULL) 
		{
			if(strstr(buf, ROMEDRIVER_IGMP_INFO))
			{
				hasMcGroup = 1;
				break;					
			}
		}
		fclose(fp);
	}
	sprintf(cmd, "echo 0 > /proc/rg/proc_to_pipe");
	va_cmd("/bin/sh", 2, 1, "-c", cmd);
	
	if(i >= entryNum)
	{
		*diagResult = NO_IPTV_CONNECTION;
	}
	else if(isBridgeUp == 0)
	{
		*diagResult = IPTV_DISCONNECT;
	}
	else if(hasMcVlan == 0)
	{
		*diagResult = INVALID_MULTIVLAN;
	}
	else if(hasMcGroup == 0)
	{
		*diagResult = IPTV_BUSSINESS_NOK;
	}
	else
	{
		*diagResult = IPTV_BUSSINESS_OK;
		ret = 0;
	}
	
	printf("%s isBridgeUp[%d],hasMcVlan[%d],hasMcGroup[%d] \n",__func__,isBridgeUp,hasMcVlan,hasMcGroup);
	return ret;
}
#endif

#ifdef SUPPORT_WAN_BANDWIDTH_INFO
//siyuan 2016-1-8 calculate wan bandwidth rate
typedef struct wan_bandwidth_info{
	unsigned int ifIndex;
	int rg_wan_idx;
	struct  timeval    oldtv;
	unsigned long long oldUploadCnt;   /* save upload counter last time */
	unsigned long long oldDownloadCnt; /* save download counter last time */
	int uploadRate;   /* in unit of kbps */
	int downloadRate; /* in unit of kbps */
}WAN_BANDWIDTH_INFO;

int bandwidthInit = 0;
int bandwidthNum = 0;
struct callout bandwidth_ch;
WAN_BANDWIDTH_INFO bandwidthInfo[MAX_VC_NUM];

void initWanBandwidthInfo()
{
	int i, entryNum;
	MIB_CE_ATM_VC_T entry;
	WAN_BANDWIDTH_INFO * bandwidth;
	
	memset(bandwidthInfo, 0, sizeof(bandwidthInfo));
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &entry)) {
			printf("get MIB chain error\n");
			break;
		}
		bandwidth = &bandwidthInfo[i];
		
		bandwidth->ifIndex = entry.ifIndex;
		bandwidth->rg_wan_idx = entry.rg_wan_idx;

		gettimeofday(&bandwidth->oldtv,NULL);
		RG_get_interface_counter(bandwidth->rg_wan_idx, &bandwidth->oldUploadCnt, &bandwidth->oldDownloadCnt);		
	}
	bandwidthNum = entryNum;
}

int calWanBandwidth(WAN_BANDWIDTH_INFO * bandwidth)
{
	unsigned long long uploadcnt, downloadcnt;
	struct  timeval  tv;
	int msecond;
	int ret;

	gettimeofday(&tv,NULL);

	ret = RG_get_interface_counter(bandwidth->rg_wan_idx, &uploadcnt, &downloadcnt);
	if(ret < 0)
	{
		return -1;
	}

	/* calculate upload and download rate */
	if((uploadcnt <= bandwidth->oldUploadCnt) || (downloadcnt <= bandwidth->oldDownloadCnt))
	{
		bandwidth->uploadRate = 0;
		bandwidth->downloadRate = 0;
	}
	else
	{	
		msecond = (tv.tv_sec - bandwidth->oldtv.tv_sec) * 1000 + 
				  ((int)(tv.tv_usec) - (int)(bandwidth->oldtv.tv_usec)) / 1000;

		bandwidth->uploadRate = ((uploadcnt - bandwidth->oldUploadCnt) * 1000 / msecond) >> 7;
		bandwidth->downloadRate = ((downloadcnt - bandwidth->oldDownloadCnt) * 1000 / msecond) >> 7;
	}
	
	bandwidth->oldUploadCnt = uploadcnt;
	bandwidth->oldDownloadCnt = downloadcnt;
	bandwidth->oldtv.tv_sec = tv.tv_sec;
	bandwidth->oldtv.tv_usec = tv.tv_usec;
	return 0;
}

/* check if any wan interface has changed*/
int checkWanChange()
{
	int i, entryNum;
	MIB_CE_ATM_VC_T entry;
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	if(entryNum != bandwidthNum)
		return -1;

	for (i = 0; i < entryNum; i++) 
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &entry)) {
			printf("get MIB chain error\n");
			return -1;
		}

		if((entry.ifIndex != bandwidthInfo[i].ifIndex) ||
		   (entry.rg_wan_idx != bandwidthInfo[i].rg_wan_idx))
			return -1;
	}

	return 0;
}

void calAllWanBandwidth()
{
	int i;

	if(checkWanChange() < 0)
	{
		initWanBandwidthInfo();
		return;
	}
	
	for (i = 0; i < bandwidthNum; i++) 
	{
		calWanBandwidth(&bandwidthInfo[i]);
	}
}

void poll_bandwidth(void *dummy)
{
	unsigned char enable;
	unsigned int interval;
	
	mib_get(MIB_WAN_BANDWIDTH_ENABLE, (void *)&enable);
	if(enable == 0)
	{
		//no schedule here.
		UNTIMEOUT(poll_bandwidth, 0, bandwidth_ch);
		return;
	}

	mib_get(MIB_WAN_BANDWIDTH_INTERVAL, (void *)&interval);
	
	if(bandwidthInit == 0)
	{
		initWanBandwidthInfo();
		bandwidthInit = 1;
	}
	else
	{
		calAllWanBandwidth();
	}
	
	TIMEOUT(poll_bandwidth, 0, interval, bandwidth_ch);
}

int wan_bandwidth_get(int rg_wan_idx, int * uploadrate, int * downloadrate)
{
	int i;
	unsigned char enable;

	mib_get(MIB_WAN_BANDWIDTH_ENABLE, (void *)&enable);
	if(enable == 0)
	{
		goto end;
	}
	
	for (i = 0; i < bandwidthNum; i++) 
	{
		if(bandwidthInfo[i].rg_wan_idx == rg_wan_idx)
		{
			*uploadrate = bandwidthInfo[i].uploadRate;
			*downloadrate = bandwidthInfo[i].downloadRate;
			return 0;
		}
	}

end:
	*uploadrate = 0;
	*downloadrate = 0;
	return -1;
}

int wan_bandwidth_set(unsigned char enable, unsigned int interval)
{
	unsigned char oldEnable;
	
	mib_get(MIB_WAN_BANDWIDTH_ENABLE, (void *)&oldEnable);

	mib_set(MIB_WAN_BANDWIDTH_ENABLE, (void *)&enable);
	mib_set(MIB_WAN_BANDWIDTH_INTERVAL, (void *)&interval);

	//printf("%s enable[%d] interval[%d] \n", __func__,enable,interval);
	if((oldEnable == 0) && (enable == 1))
	{
		bandwidthInit = 0;
		TIMEOUT(poll_bandwidth, 0, interval, bandwidth_ch);
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	return 0;
}
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
struct callout lanbandwidth_ch;
#if 0
typedef struct lan_bandwidth_info{
	rtk_mac_t mac;
	unsigned int port_idx;
	struct  timeval    oldtv;
	unsigned long long oldUpCnt;   /* save upload counter last time */
	unsigned long long oldDownCnt; /* save download counter last time */
	unsigned int uploadRate;   /* in unit of kbps */
	unsigned int downloadRate; /* in unit of kbps */
}LAN_BANDWIDTH_INFO;

int lanbandwidthInit = 0;
int lanbandwidthNum = 0;
LAN_BANDWIDTH_INFO lanbandwidthInfo[16];
#endif
typedef struct lanport_mib_info{
	struct  timeval    pretv;
	unsigned long long upCnt;   /* save upload counter last time */
	unsigned long long downCnt; /* save download counter last time */
	unsigned int upRate;   /* in unit of kbps */
	unsigned int downRate; /* in unit of kbps */
}LANPORT_MIB_INFO;

LANPORT_MIB_INFO lanPortMibInfo[4];

int getRealRatebyPort(unsigned int phyport, unsigned int* upRate, unsigned int* downRate)
{
	if(phyport > 3)
	{
		//printf("[%s %d]phyport[%d] error only support 0 - 3\n", __func__, __LINE__, phyport);
		return -1;
	}

	*upRate = lanPortMibInfo[phyport].upRate;
	*downRate = lanPortMibInfo[phyport].downRate;
	return RT_ERR_RG_OK;
}

void poll_lanbandwidth(void *dummy)
{
	int i = 0;
	unsigned char enable;
	unsigned int interval;
	unsigned long long curUpCnt;
	unsigned long long curDownCnt;
	struct  timeval  tv;
	int msecond;
	
	mib_get(MIB_LANHOST_BANDWIDTH_CONTROL_ENABLE, (void *)&enable);
	if(enable == 0)
	{
		//no schedule here.
		printf("[%s %d]MIB_LANHOST_BANDWIDTH_CONTROL disable.\n", __func__, __LINE__);
		UNTIMEOUT(poll_lanbandwidth, 0, lanbandwidth_ch);
		return;
	}

	//Get mib date, calculate upRate&downRate
	for(i = 0; i<4; i++)
	{
		gettimeofday(&tv,NULL);
		rtk_stat_port_get(i, IF_IN_OCTETS_INDEX, &curUpCnt);
		rtk_stat_port_get(i, IF_OUT_OCTETS_INDEX, &curDownCnt);

		if((curUpCnt <= lanPortMibInfo[i].upCnt) || (curDownCnt <= lanPortMibInfo[i].downCnt))
		{
			lanPortMibInfo[i].upRate = 0;
			lanPortMibInfo[i].downRate = 0;
		}
		else
		{	
			msecond = (tv.tv_sec - lanPortMibInfo[i].pretv.tv_sec) * 1000 + 
					  ((int)(tv.tv_usec) - (int)(lanPortMibInfo[i].pretv.tv_usec)) / 1000;

			lanPortMibInfo[i].upRate = ((curUpCnt - lanPortMibInfo[i].upCnt) * 1000 / msecond) >> 7;
			lanPortMibInfo[i].downRate = ((curDownCnt - lanPortMibInfo[i].downCnt) * 1000 / msecond) >> 7;
		}
		
		lanPortMibInfo[i].upCnt = curUpCnt;
		lanPortMibInfo[i].downCnt = curDownCnt;
		lanPortMibInfo[i].pretv.tv_sec = tv.tv_sec;
		lanPortMibInfo[i].pretv.tv_usec = tv.tv_usec;
		//printf("[%s %d]port[%d], upRate=%d, downRate=%d\n", __func__, __LINE__, i, lanPortMibInfo[i].upRate, lanPortMibInfo[i].downRate);
	}

	mib_get(MIB_LANHOST_BANDWIDTH_INTERVAL, (void *)&interval);
	//printf("[%s %d]start lanbandwidth timer, interval=%d.\n\n", __func__, __LINE__, interval);
	TIMEOUT(poll_lanbandwidth, 0, interval, lanbandwidth_ch);
}

void startLanPortMibInfoTimer()
{
	int i = 0;
	unsigned char enable;
	unsigned int interval;
	unsigned long long curUpCnt;
	unsigned long long curDownCnt;
	struct  timeval  tv;

	mib_get(MIB_LANHOST_BANDWIDTH_CONTROL_ENABLE, (void *)&enable);
	if(enable == 0)
	{
		printf("[%s %d]MIB_LANHOST_BANDWIDTH_CONTROL disable.\n", __func__, __LINE__);
		UNTIMEOUT(poll_lanbandwidth, 0, lanbandwidth_ch);
		return;
	}
	
	memset(lanPortMibInfo, 0, sizeof(lanPortMibInfo));
	for(i = 0; i<4; i++)
	{
		gettimeofday(&tv,NULL);
		rtk_stat_port_get(i, IF_IN_OCTETS_INDEX, &curUpCnt);
		rtk_stat_port_get(i, IF_OUT_OCTETS_INDEX, &curDownCnt);

		lanPortMibInfo[i].upCnt = curUpCnt;
		lanPortMibInfo[i].downCnt = curDownCnt;
		lanPortMibInfo[i].pretv.tv_sec = tv.tv_sec;
		lanPortMibInfo[i].pretv.tv_usec = tv.tv_usec;
	}

	mib_get(MIB_LANHOST_BANDWIDTH_INTERVAL, (void *)&interval);
	//printf("[%s %d]start lanbandwidth timer, interval=%d.\n", __func__, __LINE__, interval);
	TIMEOUT(poll_lanbandwidth, 0, interval, lanbandwidth_ch);
}
#endif

#ifdef SUPPORT_WEB_PUSHUP
static struct callout upgradeWebPushUpTimer;

static char HTTP_FIRMWARE_UPGRADE_PUSHUP[] = 
{
	"HTTP/1.0 200 OK\r\n"
	"Connection: keep-alive\r\n"
	"Content-Type: text/html\r\n"
	"\r\n"
	"<!doctype html>\r\n"
    "<html>\r\n"
    "<head>\r\n"
	"<meta http-equiv=\"Content-Type\" content=\"text/html\" charset=\"utf-8\">\r\n"
    "<title></title>\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "<script>\r\n"
	"function openwin(u, w, h) {\r\n"
	"	var l = (screen.width - w)/2;\r\n"
	"	var t = (screen.height - h) / 2;\r\n"
	"	var s = 'width=' + w + ', height=' + h + ', top=' + t + ', left=' + l +"
	"      ', toolbar=no, scrollbars=no, menubar=no, location=no, resizable=no';\r\n"
	"	window.open(u, \"_blank\", s);\r\n"
	"}\r\n"
	"openwin(\"http://192.168.1.1/upgrade_pop.asp\", 400, 100);\r\n"
	"window.location.href=\"http://%s\";\r\n"
    "</script>\r\n"
    "</body>\r\n"
    "</html>"
};

int upgradeWebSet(int enable)
{	
	char content[1024] = {0};
	
	if(enable)
	{
		memset(content, 0, sizeof(content));
		strcpy(content, HTTP_FIRMWARE_UPGRADE_PUSHUP);
		printf("content:\n%s\n", content);
		RG_set_redirect_http_all(2, content, strlen(content));
	}
	else
	{
		RG_set_redirect_http_all(0, content, 0);
	}

	return 1;
}

static int isValidImageFile(const char *fname)
{
	int ret;
	char buf[256];

	// todo: validate the image file
	snprintf(buf, sizeof(buf), "/bin/tar tf %s md5.txt > /dev/null", fname);
	ret = system(buf);

	return !ret;
}

void startUpgradeFirmware(void)
{
	char filename[100];
	char userStr[100], passwdStr[100];
	char pathStr[100];
	char portStr[100];
	char downUrl[100];
	struct stat statbuf;
	int fd;
	int urlLen, i;
	
	snprintf(userStr, 100, "user=%s", "anonymous");
	snprintf(passwdStr, 100, "passwd=%s", "anonymous");
	snprintf(pathStr, 100, "path=%s", "/tmp");
	snprintf(portStr, 100, "port=%d", 21);
	mib_get(MIB_FIRMWARE_DOWNURL, (void *)downUrl);
	
	va_cmd("/bin/wget_manage", 5, 1, userStr, passwdStr, pathStr, portStr, downUrl);

	/* get filename */
	urlLen = strlen(downUrl);
	if (downUrl[urlLen-1] == '/') downUrl[urlLen-1] = 0;
	
	for (i=urlLen-1; i>=0; i--)
	{
		if (downUrl[i] == '/')
			break;
	}
	if (i>=0)//finename found
	{
		snprintf(filename, 100, "/tmp/%s", &downUrl[i+1]);
	}
	if (!isValidImageFile(filename)) {
		printf("Incorrect image file\n");
		unlink(filename);
		return;
	}
	
	// Save file for upgrade Firmware
	fd = open(filename, O_RDWR);
	fstat(fd, &statbuf);
	close(fd);
	cmd_upload(filename, 0, statbuf.st_size);
}

static void upgradeWebPushupCheck(void)
{
	upgradeWebSet(1);
}

void startPushwebTimer(unsigned int time)//time unit: seconds
{
	TIMEOUT(upgradeWebPushupCheck, 0, time, upgradeWebPushUpTimer);
}

int firmwareUpgradeConfigSet(char *downUrl, int mode, int method, int upgradeID)
{
	/*sanity check for parameter*/
	if (NULL == downUrl)
	{
		printf("downURL should not be NULL.\n");
		return 0;
	}
	
	if (strncmp(downUrl, "ftp://", 6) &&
		strncmp(downUrl, "http://", 7))
	{
		printf("wrong downURL!\n");
		return 0;
	}

	if ((mode < 0) || (mode > 5))
	{
		printf("unknown mode, should be 0~5.\n");
		return 0;
	}

	if ((method != 0) && (method != 1))
	{
		printf("unknow method, should be 0 or 1!\n");
		return 0;
	}

	/* save the config */
	mib_set(MIB_FIRMWARE_DOWNURL, (void *)downUrl);
	mib_set(MIB_UPGRADE_MODE, (void *)&mode);
	mib_set(MIB_UPGRADE_METHOD, (void *)&method);
	mib_set(MIB_UPGRADE_ID, (void *)&upgradeID);

	/* take effect now */
	UNTIMEOUT(upgradeWebPushupCheck, 0, upgradeWebPushUpTimer);
	
	if (0 == mode)//upgrade firmware
	{
		if (0 == method)
		{//proposal for upgrade
			upgradeWebSet(1);
		}
		else if (1 == method)
		{//force to upgrade
			startUpgradeFirmware();
		}
	}
	else
	{
		//TODO: upgrade OSGI framework, java machine, NOS, etc...
	}
}
#endif//end of SUPPORT_WEB_PUSHUP

#ifdef SUPPORT_WEB_REDIRECT
//siyuan 2016-1-11 add pushweb and web redirect
struct pushweb{
	char url[128];
	int width;
	int height;
	int top;
	int left;
};

char HTTP_HEADER[] =
{
	"HTTP/1.0 200 OK\r\n"
	"Connection: close\r\n"
	"Content-Type: text/html\r\n"
	"Content-Length: %d\r\n"
	"\r\n"
	"%s"
};

char HTTP_BODY_PUSHWEB[] = 
{
	"<!doctype html>"
    "<html>"
    "<head>"
	"<meta http-equiv=\"Content-Type\" content=\"text/html\" charset=\"utf-8\">"
    "<title></title>"
    "</head>"
    "<body>"
    "<script>"
    "window.open(\"%s\", \"_blank\", \"height=%d,width=%d,top=%d,left=%d\");"
    "</script>"
    "</body>"
    "</html>"
};

static struct pushweb pushWebCfg;
struct callout pushWeb_ch;

static int welcomeInit = 0;
static int welcomeDay;
struct callout welcome_ch;
static int redirectInit = 0;

static void pushWebGenerateHttp(struct pushweb * cfg, char * content, int size)
{
	char buff[512] = {0};
	int filesize;
	
	snprintf(buff, 512, HTTP_BODY_PUSHWEB, cfg->url,
			 cfg->height,cfg->width, cfg->top, cfg->left);
	filesize = strlen(buff);
		
	snprintf(content, size, HTTP_HEADER, filesize, buff);
}

static void pushWebEnable(int enable)
{
	char content[728] = {0};
	
	if(enable)
	{
		pushWebGenerateHttp(&pushWebCfg, content, 1024);
		RG_set_redirect_http_all(1, content, strlen(content));
	}
	else 
	{
		RG_set_redirect_http_all(0, content, 0);
	}
}

/* timeout, should disable pushweb */
static void pushWebCheck()
{	
	pushWebEnable(0);
	UNTIMEOUT(pushWebCheck, 0, pushWeb_ch);
}

int pushWebSet(int enable, int width, int height, int top, int left, 
					char * url, int second)
{	
	memset((void*)&pushWebCfg, 0 , sizeof(pushWebCfg));

	if(url == NULL || strlen(url) == 0)	
		return -1;

	if((strlen(url) >= MAX_URL_LEN) || strncmp(url, "http://", 7 )) 			
			return -1;

	strncpy(pushWebCfg.url, url, sizeof(pushWebCfg.url));
	pushWebCfg.width = width;
	pushWebCfg.height = height;
	pushWebCfg.top = top;
	pushWebCfg.left = left;
	
	pushWebEnable(enable);
	if(enable)
		TIMEOUT(pushWebCheck, 0, second, pushWeb_ch);
	
	return 0;
}


int webRedirectSetRGRule(int add)
{
	int i;
	int num;
	MIB_REDIRECT_URL_LIST_T redirectUrl;
	MIB_REDIRECT_WHITE_LIST_T whiteUrl;

	num = mib_chain_total(MIB_REDIRECT_URL_LIST);
	
	for( i = 0; i < num; i++ )
	{
		if(!mib_chain_get(MIB_REDIRECT_URL_LIST, i, (void*)&redirectUrl))
			continue;
	
		if(add)
			RG_add_redirectHttpURL(&redirectUrl);
		else
			RG_del_redirectHttpURL(&redirectUrl);
	}

	num = mib_chain_total(MIB_REDIRECT_WHITE_LIST);
	
	for( i = 0; i < num; i++ )
	{
		if(!mib_chain_get(MIB_REDIRECT_WHITE_LIST, i, (void*)&whiteUrl))
			continue;

		if(add)
			RG_add_redirectWhiteUrl(&whiteUrl);
		else
			RG_del_redirectWhiteUrl(&whiteUrl);
	}

	return 0;
}

/* strip prefix "http://" from url content */
static void stripHttpFromUrl(char * url, int len)
{
	int offset;
	char buff[MAX_URL_LEN];

	if(strncmp(url, "http://", 7 ) == 0) 
	{
		offset = 7;			
	}
	else if(strncmp(url, "https://", 8 ) == 0) 
	{
		offset = 8;
	}
	else
		return;

	memmove(url, url+offset, len-offset);
	url[len-offset] = '\0';
}

int webRedirectSetRule(MIB_REDIRECT_URL_LIST_T * redirect, int redirectNum, 
							 MIB_REDIRECT_WHITE_LIST_T * white, int whiteNum)
{
	int i;
	int num;
		
	if(redirectInit == 0)
	{
		redirectInit = 1;
	}
	else
	{/* clear old rg redirect rule */
		webRedirectSetRGRule(0);
	}
	
	mib_chain_clear(MIB_REDIRECT_URL_LIST);
	num = (redirectNum <= MAX_REDIRECT_URL_LIST) ? redirectNum : MAX_REDIRECT_URL_LIST;
	for(i = 0; i < num; i++)
	{
		stripHttpFromUrl(redirect[i].srcUrl, strlen(redirect[i].srcUrl));
		stripHttpFromUrl(redirect[i].dstUrl, strlen(redirect[i].dstUrl));
		mib_chain_add(MIB_REDIRECT_URL_LIST, (void*)&redirect[i]);
	}

	num = (whiteNum <= MAX_REDIRECT_WHITE_LIST) ? whiteNum : MAX_REDIRECT_WHITE_LIST;
	mib_chain_clear(MIB_REDIRECT_WHITE_LIST);
	for(i = 0; i < num; i++)
	{
		stripHttpFromUrl(white[i].url, strlen(white[i].url));
		mib_chain_add(MIB_REDIRECT_WHITE_LIST, (void*)&white[i]);
	}

	/* add rg redirect rule */
	webRedirectSetRGRule(1);

	return 0;
}

void welcomeRedirectSet(int enable)
{	
	char url[MAX_URL_LEN] = {0};
	char buff[MAX_URL_LEN] = {0};
	unsigned int second;
	
	if(enable)
	{
		mib_get( MIB_REDIRECT_WELCOME_URL, (void *)url);
		mib_get( MIB_REDIRECT_WELCOME_TIME, (void *)&second);

		snprintf(buff, MAX_URL_LEN, "%s?length=%d&url=", url, second);
		RG_set_welcome_redirect(1, buff);
	}	
	else
	{
		RG_set_welcome_redirect(0, url);
	}
}

void welcomeRedirectCheck()
{
	unsigned char enable;
	int welcomeSet = 0;
	struct tm curTm;
	time_t curTime;   
	
	mib_get(MIB_REDIRECT_WELCOME_ENABLE, (void *)&enable);
	if(enable == 0)
	{
		UNTIMEOUT(welcomeRedirectCheck, 0, welcome_ch);
		return;
	}

	time(&curTime);
	memcpy(&curTm, localtime(&time), sizeof(curTm));
	
	if(welcomeInit == 0)
	{
		welcomeDay = curTm.tm_yday;
		welcomeInit = 1;
		welcomeSet = 1;
	}
	else
	{		
		/* check if a new day comes */
		if (welcomeDay != curTm.tm_yday)
		{
			welcomeSet = 1;
			welcomeDay = curTm.tm_yday;
		}
	}

	if(welcomeSet)
		welcomeRedirectSet(1);

	TIMEOUT(welcomeRedirectCheck, 0, 3600, welcome_ch);
}

int webRedirectSet(unsigned char welcome, char * url, unsigned int second, 
						MIB_REDIRECT_URL_LIST_T * redirect, int redirectNum, 
						MIB_REDIRECT_WHITE_LIST_T * white, int whiteNum)
{
	int i;
	int num;
	unsigned char oldEnable;
	
	mib_get(MIB_REDIRECT_WELCOME_ENABLE, (void *)&oldEnable);
	mib_set(MIB_REDIRECT_WELCOME_ENABLE, (void *)&welcome);

	if(url == NULL || strlen(url) == 0)
	{
		mib_set( MIB_REDIRECT_WELCOME_URL, (void *)"");
	}
	else
	{	
		stripHttpFromUrl(url, strlen(url));		
		mib_set( MIB_REDIRECT_WELCOME_URL, (void *)url);
	}

	mib_set(MIB_REDIRECT_WELCOME_TIME, (void *)&second);

	webRedirectSetRule(redirect, redirectNum, white, whiteNum);
	
	if(welcome)
	{
		welcomeInit = 0;
		welcomeRedirectCheck();		
	}
	else if((welcome == 0) && oldEnable)
	{
		UNTIMEOUT(welcomeRedirectCheck, 0, welcome_ch);
		welcomeRedirectSet(0);
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	return 0;
}

int updateRedirectIPAddr(unsigned int ipAddr)
{
	if ((0 == ipAddr) || (0xFFFFFFFF == ipAddr))
		return 0;

	mib_set(MIB_REDIRECT_IP, (void *)ipAddr);
	return 1;
}

int updateITMSAddr(unsigned int ipAddr)
{
	if ((0 == ipAddr) || (0xFFFFFFFF == ipAddr))
		return 0;

	mib_set(MIB_ITMS_ADDR, (void *)ipAddr);
	return 1;
}
#endif

#ifdef CONFIG_RTK_RG_INIT
#include "rtusr_rg_api.h"
#endif

static int startWanServiceDependency(int ipEnabled)
{
	char vChar=-1;
	int ret;

#ifdef DEFAULT_GATEWAY_V2
	unsigned char dgwip[16];
	unsigned int dgw;
	if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0) {
		if (dgw == DGW_NONE && getMIB2Str(MIB_ADSL_WAN_DGW_IP, dgwip) == 0) {
			if (ifExistedDGW() == 1)
				va_cmd(ROUTE, 2, 1, ARG_DEL, "default");
			// route add default gw remotip
			va_cmd(ROUTE, 4, 1, ARG_ADD, "default", "gw", dgwip);
		}
	}
#endif

	// Add static routes
	// Mason Yu. Init hash table for all routes on RIP
	addStaticRoute();
#ifdef CONFIG_IPV6
	addStaticV6Route();
#endif

#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
	//Kill originally running RADVD daemon.
	if(!ipEnabled)
	{
		int pid = read_pid((char *)RADVD_PID);
		if ( pid > 0)
		{
			printf("Bridge mode only, kill the original radvd deamon.\n");
			kill(pid,SIGTERM);
			va_cmd("/bin/rm", 1, 0, (char *)RADVD_PID);
		}
	}
	else{
		restartRadvd();
		}
#endif

    if (ipEnabled!=1)
	{
		setup_ipforwarding(0);
	}

#ifdef CONFIG_USER_ROUTED_ROUTED
	if (startRip() == -1)
	{
		printf("start RIP failed !\n");
		ret = -1;
	}
#endif

#ifdef TIME_ZONE
	if (startNTP())
	{
		printf("start SNTP failed !\n");
		ret = -1;
	}
#endif

#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
	if (startOspf() == -1)
	{
		printf("start OSPF failed !\n");
		ret = -1;
	}
#endif

#ifdef CONFIG_USER_IGMPPROXY
#ifdef CONFIG_IGMPPROXY_MULTIWAN
		if (setting_Igmproxy() == -1)
		{
			printf("start IGMP proxy failed !\n");
			ret = -1;
		}
#else
		if (startIgmproxy() == -1)
		{
			printf("start IGMP proxy failed !\n");
			ret = -1;
		}
#endif // of CONFIG_IGMPPROXY_MULTIWAN
#endif // of CONFIG_USER_IGMPPROXY

// Mason Yu. MLD Proxy
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_ECMH
		if (startMLDproxy() == -1)
		{
			printf("start MLD proxy failed !\n");
			ret = -1;
		}
#endif // of CONFIG_USER_ECMH
#endif
	// execute firewall rules
	if (setupFirewall() == -1)
	{
		printf("execute firewall rules failed !\n");
		ret = -1;
	}
#ifdef CONFIG_IP_NF_ALG_ONOFF
	if(setupAlgOnOff()==-1)
	{
		printf("execute ALG on-off failed!\n");
	}
#endif

#if 0 //eason path for pvc add/del and conntrack killall
	// Kaohj --- remove all conntracks
	va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
#endif

#ifdef WEB_REDIRECT_BY_MAC
	if( -1==start_web_redir_by_mac() )
		ret=-1;
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	unsigned char cp_enable;
	char cp_url[MAX_URL_LEN];

	mib_get(MIB_CAPTIVEPORTAL_ENABLE, (void *)&cp_enable);
	mib_get(MIB_CAPTIVEPORTAL_URL, (void *)cp_url);

	if(cp_enable && cp_url[0])
		if( -1 == start_captiveportal() )
			ret = -1;
#endif
#ifndef CONFIG_RTK_RG_INIT
#ifdef CONFIG_USER_CWMP_TR069
	SetTR069WANInterface();
#endif
#endif
	return ret;
}

//--------------------------------------------------------
// WAN startup
// return value:
// 1  : successful
// -1 : failed
// configAll = CONFIGALL,  pEntry = NULL  : start all WAN connections(include VC, ETHWAN, PTMWAN, VPN, 3g).
// configAll = CONFIGONE, pEntry != NULL : start specified VC, ETHWAN, PTMWAN connection and VPN, 3g connections.
// configAll = CONFIGONE, pEntry = NULL  : start VPN, 3g connections.
int startWan(int configAll, MIB_CE_ATM_VC_Tp pEntry)
{
	int vcTotal, i, ret;
	MIB_CE_ATM_VC_T Entry;
	int ipEnabled;
	FILE *fp;
	char vcNum[16];
	char ifname[IFNAMSIZ];
	char vChar=-1;
#ifdef CONFIG_DEV_xDSL
	Modem_LinkSpeed vLs;
	vLs.upstreamRate=0;
#endif
	ret = 1;
	vcTotal = mib_chain_total(MIB_ATM_VC_TBL);

	ipEnabled = 0;

	// Mason Yu.
	//If it is a router modem , we should config forwarding first.
	// Because the /proc/.../conf/all/forwarding value, will affect /proc/.../conf/vc0/forwarding value.
	setup_ipforwarding(1);

#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)

//#ifdef NEW_PORTMAPPING
	// Get port-mapping bitmapped LAN ports (fgroup)
	//get_pmap_fgroup(pmap_list, MAX_VC_NUM);
//#endif

#if 0//defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
	if(Init_RG_ELan(UntagCPort, RoutingWan)!=SUCCESS){
		printf("Init_RG_ELan failed!!! \n");
			return -1;
	}
#endif

	for (i = 0; i < vcTotal; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				return -1;

		if (Entry.enable == 0)
			continue;

		if(configAll == CONFIGALL)
		{
#ifdef CONFIG_RTK_RG_INIT
			// Call RG_add_wan before addEthWANdev because addEthWANdev() will trigger link change events.
			RG_add_wan(&Entry, i);
#endif
			addEthWANdev(&Entry);
		}
		else if(configAll == CONFIGONE)
		{
			 if(pEntry && Entry.ifIndex == pEntry->ifIndex)
			 {
#ifdef CONFIG_RTK_RG_INIT
				// Call RG_add_wan before addEthWANdev because addEthWANdev() will trigger link change events.
				RG_add_wan(&Entry, i);
#endif
				addEthWANdev(&Entry);
				break;
			 }
		}
	}
#endif	//defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)

	// If we set forwarding=1 for all, then the every interafce's forwarding will be modified to 1.
	// So we must set forwarding=1 for all first, then set every interafce's forwarding.
	// Mason Yu.
	setup_ipforwarding(1);
#ifdef CONFIG_USER_PPPOMODEM
	// wan3g run as routed mode
	ipEnabled = wan3g_start();
#endif //CONFIG_USER_PPPOMODEM

	for (i = 0; i < vcTotal; i++)
	{
		/* get the specified chain record */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;

		if(!isInterfaceMatch(Entry.ifIndex))  // Magician: Only raise interfaces that was set by wan mode.
			continue;

		if (Entry.enable == 0)
			continue;

		if (configAll == CONFIGALL) 		// config for ALL
			ret|=startConnection(&Entry, i);
		else	 if (configAll == CONFIGONE)	// config for one
		{
			if (pEntry != NULL) {
				if ( Entry.ifIndex == pEntry->ifIndex) {
					ret|=startConnection(&Entry, i);
				}
				else
					continue;
			}
			else
				break;		// If pEntry == NULL, It mean we want to delete this channel.
		}

		if ((CHANNEL_MODE_T)Entry.cmode != CHANNEL_MODE_BRIDGE)
			ipEnabled = 1;

#if defined(CONFIG_ATM_CLIP)&&defined(CONFIG_RTL8672_SAR)
#ifdef CONFIG_DEV_xDSL
		if (adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) && vLs.upstreamRate != 0) {
#endif
			if (Entry.cmode == CHANNEL_MODE_RT1577) {
				struct data_to_pass_st msg;
				char wanif[5];
				unsigned long addr = *((unsigned long *)Entry.remoteIpAddr);
				snprintf(wanif, 5, "vc%d", VC_INDEX(Entry.ifIndex));
				snprintf(msg.data, BUF_SIZE, "mpoactl set %s inarprep %lu", wanif, addr);
				write_to_mpoad(&msg);
			}
#ifdef CONFIG_DEV_xDSL
		}
#endif
#endif
		//if (configAll == CONFIGONE)
		//	break;
	}

	startSNAT();
	ret = startWanServiceDependency(ipEnabled);
	return ret;
}

#ifdef E8B_NEW_DIAGNOSE
int findpvcfileid(const pvc_status_entry oldentry)
{
	int i;
	char buf[32];
	FILE *fp;
	pvc_status_entry entry;

	memset(&entry, 0, sizeof(entry));
	for (i = 1; ; i++) {
		sprintf(buf, "%s%d", PVC_FILE, i);
		fp = fopen(buf, "rb+");
		if (fp) {
			fread(&entry, 1, sizeof(entry), fp);
			fclose(fp);
			if (entry.vpi == oldentry.vpi && entry.vci == oldentry.vci) {
				break;
			}
		} else {
			break;
		}
	}

	return i;
}

int findmatchpvcfileid(pvc_status_entry *pvcentry)
{
	int i;
	char buf[32];
	FILE *fp;
	pvc_status_entry entry;

	memset(&entry, 0, sizeof(entry));
	for (i = 1; ; i++) {
		sprintf(buf, "%s%d", PVC_FILE, i);
		fp = fopen(buf, "rb+");
		if (fp) {
			fread(&entry, 1, sizeof(entry), fp);
			fclose(fp);
			if (entry.vpi == pvcentry->vpi && entry.vci == pvcentry->vci) {
				memcpy(pvcentry, &entry, sizeof(entry));
				return i;
			}
		} else {
			return 0;
		}
	}
}

void clearpvcfile()
{
	int i;
	char buf[32];
	FILE *fp;

	for (i = 1; ; i++) {
		sprintf(buf, "%s%d", PVC_FILE, i);
		fp = fopen(buf, "rb+");
		if (fp) {
			fclose(fp);
			unlink(buf);
		} else {
			break;
		}
	}
}

void cleardelpvcfile()
{
	int i;
	char buf[32];
	FILE *fp;
	pvc_status_entry entry;

	for (i = 1; ; i++) {
		sprintf(buf, "%s%d", PVC_FILE, i);
		fp = fopen(buf, "rb+");
		if (fp) {
			fread(&entry, 1, sizeof(entry), fp);
			fclose(fp);
			if (!strcmp(entry.action, "del"))
				unlink(buf);
		} else {
			break;
		}
	}
}

void clearssidfile()
{
	int i;
	char buf[32];
	FILE *fp;
	pvc_status_entry entry;

	for (i = 1; i <= 5; i++) {
		sprintf(buf, "%s%d", SSID_FILE, i);
		unlink(buf);
	}
}

void clearremotefile()
{
	clearpvcfile();
	clearssidfile();
	unlink(USERLIMIT_FILE);
	unlink(QOS_FILE);
	unlink(NEW_SETTING);
}

void check_inform_status(int firstflag)
{
	char tmpbuf[256 + 1];
	FILE *fp;
	unsigned int regResult;

	if (firstflag) {
		fp = fopen(CONNREQ_STATUS_FILE, "w");
		if (fp) {
			fprintf(fp, "%d", NO_REQUEST);
			fclose(fp);
		}
	}

	mib_get(CWMP_USERINFO_RESULT, &regResult);
	if (regResult == SET_FAULT) {
		fp = fopen(NEW_SETTING, "r");
		if (fp) {
			fgets(tmpbuf, sizeof(tmpbuf), fp);
			fclose(fp);
		}
		if (!strcmp(tmpbuf, PVC_FILE)) {
			clearpvcfile();
		} else if (!strcmp(tmpbuf, SSID_FILE)) {
			clearssidfile();
		} else if (!strcmp(tmpbuf, USERLIMIT_FILE)) {
			unlink(USERLIMIT_FILE);
		} else if (!strcmp(tmpbuf, QOS_FILE)) {
			unlink(QOS_FILE);
		}
	}
}
#endif	//E8B_NEW_DIAGNOSE

#ifdef CONFIG_MIDDLEWARE
void mwSetFirewall(int set)
{
	char *act;
	if(set)
		act = FW_ADD;
	else
		act = FW_DEL;
	va_cmd(IPTABLES, 8, 1, (char *)act, (char *)FW_INPUT, "-p",
		"tcp", (char *)FW_DPORT, "9080", "-j", (char *)FW_ACCEPT);
	va_cmd(IPTABLES, 8, 1, (char *)act, (char *)FW_INPUT, "-p",
		"tcp", (char *)FW_DPORT, "30005", "-j", (char *)FW_ACCEPT);
}

// cxy 2015-8-29: no need to add route for middleware server,
//				ctadmin binds localip passed by middleware process to socket connecting to middleware server.
#if 0
void setapplicationtype_mw(MIB_CE_ATM_VC_T *pEntry, int mode, char *pAddr )
{
	char vStr[256+1];
	char addrStr[256+1];
	struct hostent *host;
	struct in_addr mwaddr;
	char* argv[7];
	char *operationMode[2]={"add","del"};
	char *iptablesopt[2]={"-A","-D"};
	char vChar;
/*star:20090120*/
	mib_get( CWMP_TR069_ENABLE, (void*)&vChar);
	if(vChar==1)
		return;

	if(pEntry->cmode!=0){
		if(pAddr==NULL){
			mib_get(CWMP_MIDWARE_SERVER_ADDR,(void*)vStr);
			set_endpoint(addrStr,vStr);
		}else{
			//strcpy(addrStr,pAddr);
			set_endpoint(addrStr,pAddr);
		}
		host=gethostbyname(addrStr);

		if(host==NULL){
			printf("Can't get host by host name.\n");
			return;
		}
		memcpy((char *) &(mwaddr.s_addr), host->h_addr_list[0], host->h_length);

		if(pEntry->applicationtype==2 || pEntry->applicationtype==3 )//TR_069 or BOTH
		{
				char interfaceName[8]={0};
				char DstIp[20]={0};
				//route del -host 192.168.2.241 dev vc0
				argv[1]=operationMode[mode];
				argv[2]="-host";
				strcpy(DstIp,inet_ntoa(mwaddr));
				argv[3]=DstIp;
				argv[4]="dev";
				if(pEntry->cmode!=CHANNEL_MODE_PPPOE && pEntry->cmode!=CHANNEL_MODE_PPPOA )
					snprintf(interfaceName,sizeof(interfaceName),"vc%d",VC_INDEX(pEntry->ifIndex));
				else
					snprintf(interfaceName,sizeof(interfaceName),"ppp%d",PPP_INDEX(pEntry->ifIndex));
				argv[5]=interfaceName;
				argv[6]=NULL;
			if(mode==0){
				if((strstr(interfaceName,"ppp")!=0)){
					do_cmd("/bin/route",argv,1);
					return;
				}
				if(pEntry->ipDhcp==1)
				{
					char gwip[20];
					FILE *fp;
					char buffer[50];
					sprintf(buffer,"%s.%s",DHCPC_ROUTERFILE,interfaceName);
					fp=fopen(buffer,"r");
					if(!fp)
						return;
					fscanf(fp,"%s",gwip);
					fclose(fp);
					va_cmd("/bin/route",7,1,"add","-host",DstIp,"gw",gwip,"dev",interfaceName);
					printf("/bin/route %s %s %s %s %s %s %s","add","-host",DstIp,"gw",gwip,"dev",interfaceName);
					return;
				}
				if(pEntry->cmode==CHANNEL_MODE_IPOE && pEntry->ipDhcp==0)
				{
					char gwip[20];
					if(pEntry->remoteIpAddr[0]!=0){
						strcpy(gwip, inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr)));
						va_cmd("/bin/route",7,1,"add","-host",DstIp,"gw",gwip,"dev",interfaceName);
						printf("/bin/route %s %s %s %s %s %s %s","add","-host",DstIp,"gw",gwip,"dev",interfaceName);
					}
					return;
				}
				return;
			}
			printf("/bin/route %s %s %s %s  %s!\n",argv[1],argv[2],argv[3],argv[4],argv[5]);
			do_cmd("/bin/route",argv,1);

		}else if(pEntry->applicationtype==1){
			char DstIp[20]={0};
			char interfaceName[8]={0};

			strcpy(DstIp,inet_ntoa(mwaddr));
			if(pEntry->cmode!=CHANNEL_MODE_PPPOE && pEntry->cmode!=CHANNEL_MODE_PPPOA )
				snprintf(interfaceName,sizeof(interfaceName),"vc%d",VC_INDEX(pEntry->ifIndex));
			else
				snprintf(interfaceName,sizeof(interfaceName),"ppp%d",PPP_INDEX(pEntry->ifIndex));
			//iptables -A OUTPUT -o vc0 -d acsip -j DROP
			printf("\n/bin/iptables %s %s %s %s %s %s %s %s\n",iptablesopt[mode], "OUTPUT", (char *)ARG_O, interfaceName,
				"-d", DstIp, "-j", (char *)FW_DROP);
			va_cmd(IPTABLES, 8, 1, iptablesopt[mode], "OUTPUT", (char *)ARG_O, interfaceName,
				"-d", DstIp, "-j", (char *)FW_DROP);
		}
	}
	return;

}

void delMidwareRouteFW()
{
	MIB_CE_ATM_VC_T Entry;
	int total,i;
	char oldaddr[256+1];
	char newaddr[256+1];

	mib_get(CWMP_MIDWARE_SERVER_ADDR,(void*)newaddr);
	mib_get(CWMP_MIDWARE_SERVER_ADDR_OLD,(void*)oldaddr);

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<total;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&Entry)!=1)
			continue;
		if((Entry.applicationtype==X_CT_SRV_INTERNET)||(Entry.applicationtype==X_CT_SRV_TR069) ||(Entry.applicationtype==(X_CT_SRV_TR069|X_CT_SRV_INTERNET) )){
			setapplicationtype_mw(&Entry,1,oldaddr); // del old midware url route rule
			setapplicationtype_mw(&Entry,1,newaddr); // del new midware url route rule
		}
	}
	return;
}
void setMidwareRouteFW(int clearold) //clearold: 1: used in situation of reboot cwmp
									//         0:used in situation of reboot firewall
{
	MIB_CE_ATM_VC_T Entry;
	int total,i;
	char oldaddr[256+1];
	char newaddr[256+1];

	mib_get(CWMP_MIDWARE_SERVER_ADDR,(void*)newaddr);
	if(clearold==0)
		strcpy(oldaddr,newaddr);
	else
		mib_get(CWMP_MIDWARE_SERVER_ADDR_OLD,(void*)oldaddr);
	total = mib_chain_total(MIB_ATM_VC_TBL);

	for(i=0;i<total;i++){
		if(mib_chain_get(MIB_ATM_VC_TBL,i,&Entry)!=1)
			continue;
		if((Entry.applicationtype==X_CT_SRV_INTERNET)||(Entry.applicationtype==X_CT_SRV_TR069) ||(Entry.applicationtype==(X_CT_SRV_TR069|X_CT_SRV_INTERNET) )){
			setapplicationtype_mw(&Entry,1,oldaddr); // del old acs url route rule
			setapplicationtype_mw(&Entry,1,newaddr); // del old acs url route rule
			setapplicationtype_mw(&Entry,0,newaddr); // add new acs url route rule
		}
	}
	return;
}
#endif

int sendMsg2MidProcess(struct mwMsg * pMsg)
{
	int spid;
	FILE * spidfile;
	int msgid;
	msgid = msgget((key_t)1357,  0666);
	if(msgid <= 0){
	//	fprintf(stdout,"Saved: get cwmp msgqueue error in %s!\n",__FUNCTION__);
		return -1;
	}

	/* get midware interface pid*/
	if ((spidfile = fopen(CWMP_MIDPROC_RUNFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}else{
		fprintf(stdout,"Saved: midware midprocess pidfile not exists in %s\n",__FUNCTION__);
		return -1;
	}

	pMsg->msg_type = spid;
	pMsg->msg_datatype = MSG_MIDWARE;
	if(msgsnd(msgid, (void *)pMsg, MW_MSG_SIZE, 0) < 0){
		fprintf(stdout,"Saved: send message to midprocess error in %s!\n",__FUNCTION__);
		return -1;
	}

	return 0;
}

int sendSetDefaultFlagMsg2MidProcess()
 {
	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;

	*(sendBuf) = OP_SetDefaultFlag;
	return(sendMsg2MidProcess(&sendMsg));

 }

int sendInformEventMsg2MidProcess( unsigned char eventCode )
{
	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;
	*(sendBuf) = OP_informEvent;
	*(sendBuf+1) = eventCode;
	*(sendBuf+2) = 0;
	return (sendMsg2MidProcess(&sendMsg));
}

 static int sendMsg2MidIntf(struct mwMsg * pMsg)
{
	int spid;
	FILE * spidfile;
	int msgid;

	msgid = msgget((key_t)1357,  0666);
	if(msgid <= 0){
		fprintf(stdout,"Saved: get cwmp msgqueue error in %s!\n",__FUNCTION__);
		return -1;
	}

	/* get midware interface pid*/
	if ((spidfile = fopen(MW_INTF_RUNFILE, "r"))) {
		fscanf(spidfile, "%d\n", &spid);
		fclose(spidfile);
	}else{
		fprintf(stdout,"Saved: midware interface pidfile not exists in %s\n",__FUNCTION__);
		return -1;
	}

	pMsg->msg_type = spid;
	pMsg->msg_datatype = PACKET_OK;
	if(msgsnd(msgid, (void *)pMsg, MW_MSG_SIZE, 0) < 0){
		fprintf(stdout,"Saved: send message to midwareintf error in %s!\n",__FUNCTION__);
		return -1;
	}

	return 0;
}

int sendSetDefaultRetMsg2MidIntf()
{
	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;

	*sendBuf = OP_SetDefaultRet;		/*Opcode*/
	*(sendBuf+1) = 0;				/*N*/
	return(sendMsg2MidIntf(&sendMsg));
}


/* send Exit cmd:boa process -> midProcess(tr069) -> midware_inf process -> ctadmin
 * ctadmin will exit itself
 */
 int sendMWExit2Midproc()
 {
	struct mwMsg sendMsg;
	char * sendBuf = sendMsg.msg_data;

	*(sendBuf) = OP_MWExit;
	return(sendMsg2MidProcess(&sendMsg));
 }

int apply_Midware( int action_type, int id, void *olddata )	/*note: cann't restart cwmp process for apply immediately*/
{
	unsigned char oldValue,vChar;

	oldValue = *((unsigned char *)olddata);
	mib_get(CWMP_TR069_ENABLE,(void *)&vChar);

	if(oldValue == 1 && vChar == 2){	/*start midware*/
		/*midprocess thread will be started in cwmp_main process*/
		//setMidwareRouteFW(1);
		mwSetFirewall(1);
		if(read_pid((char*)MWINTF_MAIN_RUNFILE) <= 0){ //midware interface is not started
			va_cmd( "/bin/midware_intf", 0, 0 );
		}
		//start midware bootprocess
		//va_cmd( "/usr/bin/ctadmin_realtek", 1, 0,"s=2" );	//call ct_midware bootstrap
		va_cmd( "/usr/local/ct/ctadmin", 1, 0,"s=2" );	//call ct_midware bootstrap
	}

	if(oldValue == 2 && vChar == 1){	/*close midware*/
		int loopcount = MW_EXIT_WAIT_TIME;
		int midware_intf_pid = 0;
		/*midprocess thread will be closed by itself*/
		/*wait ct_midware Exit*/
		while(loopcount){
			midware_intf_pid = read_pid((char*)MWINTF_MAIN_RUNFILE);
			if(midware_intf_pid > 0)
				loopcount--;
			else
				break;
			sleep(1);
		}

		mwSetFirewall(0);
		//delMidwareRouteFW();//delete MW route entry
		midware_intf_pid = read_pid((char*)MWINTF_MAIN_RUNFILE);
		if(midware_intf_pid > 0)
			kill(midware_intf_pid,SIGTERM);
	}

	return 0;
}
#endif


#ifdef _CWMP_MIB_ /*jiunming, mib for cwmp-tr069*/
int startCWMP(void)
{
	char vChar=0;
	char strPort[16];
	unsigned int conreq_port=0;

	//lan interface enable or disable
	mib_get(CWMP_LAN_IPIFENABLE, (void *)&vChar);
	if(vChar==0)
	{
		va_cmd(IFCONFIG, 2, 1, BRIF, "down");
		printf("Disable br0 interface\n");
	}
	//eth0 interface enable or disable
	mib_get(CWMP_LAN_ETHIFENABLE, (void *)&vChar);
	if(vChar==0)
	{
		va_cmd(IFCONFIG, 2, 1, ELANIF, "down");
		printf("Disable eth0 interface\n");
	}

	/*add a wan port to pass */
	mib_get(CWMP_CONREQ_PORT, (void *)&conreq_port);
	if(conreq_port==0) conreq_port=7547;
	sprintf(strPort, "%u", conreq_port );
	va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", "-A", (char *)FW_PREROUTING,
		 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

#ifdef CONFIG_RTK_RG_INIT
	mib_get(MIB_DMZ_ENABLE, &vChar);
	if(vChar)
	{
		Flush_RTK_RG_gatewayService();
		RTK_RG_gatewayService_add();
	}
#endif

	/*start the cwmpClient program*/
	mib_get(CWMP_FLAG, (void *)&vChar);
	if( vChar&CWMP_FLAG_AUTORUN )
	{
		va_cmd( "/bin/cwmpClient", 0, 0 );
	}

#ifdef CONFIG_MIDDLEWARE
	mib_get(CWMP_TR069_ENABLE,(void *)&vChar);	//CWMP_TR069_ENABLE: 0-run midware;1-run normal tr069; 2 run midware and normal tr069
	if(vChar == 0 || vChar == 2){
		//setMidwareRouteFW(1);
		if(read_pid((char*)MWINTF_MAIN_RUNFILE) <= 0){ //midware interface is not started
			va_cmd( "/bin/midware_intf", 0, 0 );
		}
		//start midware bootprocess
		if(vChar == 0)
			va_cmd( "/usr/local/ct/ctadmin", 1, 0,"s=0" );	//call midware bootstrap
		else
		va_cmd( "/usr/local/ct/ctadmin", 1, 0,"s=2" );	//call midware bootstrap
		mwSetFirewall(1);
	}
#endif
#ifdef E8B_NEW_DIAGNOSE
	check_inform_status(1);
#endif

#ifdef CONFIG_USER_CWMP_UPNP_DM
	// Start UPnP DM proxy
	va_cmd( "/bin/upnpmd_cp", 0, 0 );
#endif
	return 0;
}
#endif

#ifdef E8B_GET_OUI
void getOUIfromMAC(char *oui)
{
	unsigned char buffer[MAC_ADDR_LEN];
	int i;

	if (mib_get(MIB_ELAN_MAC_ADDR, buffer)) {
		sprintf(oui, "%02x%02x%02x", buffer[0], buffer[1], buffer[2]);
		for (i = 0; i < strlen(oui); i++) {
			oui[i] = toupper(oui[i]);
		}
		oui[i] = '\0';
	} else {
		strcpy(oui, DEF_MANUFACTUREROUI_STR);
	}
}
#endif

#ifdef CONFIG_DEV_xDSL
static char open_adsl_drv(void)
{
	if ((adslFp = fopen(adslDevice, "r")) == NULL) {
		printf("ERROR: failed to open %s, error(%s)\n", adslDevice,
		       strerror(errno));
		return 0;
	}
	return 1;
}

static void close_adsl_drv(void)
{
	if (adslFp) {
		fclose(adslFp);
		adslFp = NULL;
	}
}

char adsl_drv_get(unsigned int id, void *rValue, unsigned int len)
{
	int ret = 0;
#ifdef EMBED
	if (open_adsl_drv()) {
		obcif_arg myarg;
		myarg.argsize = (int)len;
		myarg.arg = (int)rValue;

		if (ioctl(fileno(adslFp), id, &myarg) >= 0)
			ret = 1;

		close_adsl_drv();
		return ret;
	}
#endif
	return ret;
}

#ifdef CONFIG_VDSL
/*pval: must be an int[4]-arrary pointer*/
static char dsl_msg(unsigned int id, int msg, int *pval)
{
	MSGTODSL msg2dsl;
	char ret=0;

	msg2dsl.message=msg;
	msg2dsl.intVal=pval;
	ret=adsl_drv_get(id, &msg2dsl, sizeof(MSGTODSL));

	return ret;
}

char dsl_msg_set_array(int msg, int *pval)
{
	char ret=0;

	if(pval)
	{
		ret=dsl_msg(RLCM_UserSetDslData, msg, pval);
	}
	return ret;
}

char dsl_msg_set(int msg, int val)
{
	int tmpint[4];
	char ret=0;

	tmpint[0]=val;
	ret=dsl_msg_set_array(msg, tmpint);
	return ret;
}

char dsl_msg_get_array(int msg, int *pval)
{
	char ret=0;

	if(pval)
	{
		ret=dsl_msg(RLCM_UserGetDslData, msg, pval);
	}
	return ret;
}

char dsl_msg_get(int msg, int *pval)
{
	int tmpint[4];
	char ret=0;

	if(pval)
	{
		ret=dsl_msg_get_array(msg, tmpint);
		if(ret) *pval=tmpint[0];
	}
	return ret;
}
#endif /*#CONFIG_VDSL*/

static XDSL_OP xdsl0_op =
{
	0,
	adsl_drv_get,
#ifdef CONFIG_VDSL
	dsl_msg_set_array,
	dsl_msg_get_array,
	dsl_msg_set,
	dsl_msg_get,
#endif /*CONFIG_VDSL*/
	getAdslInfo,
};

#ifdef CONFIG_USER_XDSL_SLAVE
static XDSL_OP xdsl1_op =
{
	1,
	adsl_slv_drv_get,
#ifdef CONFIG_VDSL
	dsl_slv_msg_set_array,
	dsl_slv_msg_get_array,
	dsl_slv_msg_set,
	dsl_slv_msg_get,
#endif /*CONFIG_VDSL*/
	getAdslSlvInfo,
};
#endif /*CONFIG_USER_XDSL_SLAVE*/

static XDSL_OP *xdsl_op[]=
{
	&xdsl0_op,
#ifdef CONFIG_USER_XDSL_SLAVE
	&xdsl1_op
#endif /*CONFIG_USER_XDSL_SLAVE*/
};

XDSL_OP *xdsl_get_op(int id)
{
#ifdef CONFIG_USER_XDSL_SLAVE
	if(id)
	{
		if(id!=1) printf( "%s: error id=%d\n", __FUNCTION__, id );
		return xdsl_op[1];
	}
#endif /*CONFIG_USER_XDSL_SLAVE*/

	if(id!=0) printf( "%s: error id=%d\n", __FUNCTION__, id );
	return xdsl_op[0];
}

int setupDsl(void)
{
	char tone[64];
	unsigned char init_line, olr;
	unsigned short dsl_mode;
	int val;
	int ret=1;

	// check INIT_LINE
	if (mib_get(MIB_INIT_LINE, (void *)&init_line) != 0)
	{
		if (init_line == 1)
		{
#ifdef CONFIG_VDSL
			unsigned int mode;
			unsigned short profile;

			//disable modem,20130203
			printf("xDSL disable modem\n");
			adsl_drv_get(RLCM_PHY_DISABLE_MODEM, NULL, 0);

			//Pmd mode
			mode=0;
			mib_get(MIB_ADSL_MODE, (void *)&dsl_mode);
			if(dsl_mode&ADSL_MODE_GDMT)		mode |= MODE_GDMT;
			if(dsl_mode&ADSL_MODE_GLITE)	mode |= MODE_GLITE;
			if(dsl_mode&ADSL_MODE_ADSL2)	mode |= MODE_ADSL2;
			if(dsl_mode&ADSL_MODE_ADSL2P)	mode |= MODE_ADSL2PLUS;
			if(dsl_mode&ADSL_MODE_VDSL2)	mode |= MODE_VDSL2;

			if(dsl_mode&ADSL_MODE_ANXB)
			{
				// Annex B
				mode |= MODE_ANX_B;
				if(dsl_mode&ADSL_MODE_T1413)	mode |= MODE_ETSI;
			}else{
				// Annex A
				mode |= MODE_ANX_A;
				if(dsl_mode&ADSL_MODE_T1413)	mode |= MODE_ANSI;
				if(dsl_mode&ADSL_MODE_ANXL)		mode |= MODE_ANX_L;
				if(dsl_mode&ADSL_MODE_ANXM)		mode |= MODE_ANX_M;
			}
			printf("SetPmdMode=0x%08x (dsl_mode=0x%04x)\n",  mode, dsl_mode);
			dsl_msg_set(SetPmdMode, mode);

			//vdsl2 profile
			mib_get(MIB_VDSL2_PROFILE, (void *)&profile);
			printf("SetVdslProfile=0x%08x\n", (unsigned int)profile);
			dsl_msg_set(SetVdslProfile, (unsigned int)profile);


			// G.INP,	 default by dsplib
#ifdef ENABLE_ADSL_MODE_GINP
			val=0;
			if (dsl_mode & ADSL_MODE_GINP)	// G.INP
				val=3;
			printf("SetGInp=0x%08x\n", val);
			dsl_msg_set(SetGInp, val);
#endif /*ENABLE_ADSL_MODE_GINP*/

			// set OLR Type
			mib_get(MIB_ADSL_OLR, (void *)&olr);
			val = (int)olr;
			// SRA (should include bitswap)
			if(val == 2) val = 3;
			printf("SetOlr=0x%08x (olr=0x%02x)\n", val, olr);
			dsl_msg_set(SetOlr, val);

			// Start handshaking; should be the last command
			mode=0;
			adsl_drv_get(RLCM_SET_ADSL_MODE, (void *)&mode, 4);


			//enable modem,20130203
			printf("xDSL enable modem\n");
			adsl_drv_get(RLCM_PHY_ENABLE_MODEM, NULL, 0);

			ret = 1;
#else /*CONFIG_VDSL*/
			char mode[3], inp;
			int xmode,adslmode, axB, axM, axL;

			// start adsl
			mib_get(MIB_ADSL_MODE, (void *)&dsl_mode);

			adslmode=(int)(dsl_mode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT));	// T1.413 & G.dmt
			#if 0
			adsl_drv_get(RLCM_PHY_START_MODEM, (void *)&adslmode, 4);
			#endif

			if (dsl_mode & ADSL_MODE_ANXB) {	// Annex B
				axB = 1;
				axL = 0;
				axM = 0;
			}
			else {	// Annex A
				axB = 0;
				if (dsl_mode & ADSL_MODE_ANXL)	// Annex L
					axL = 3; // Wide-Band & Narrow-Band Mode
				else
					axL = 0;
				if (dsl_mode & ADSL_MODE_ANXM)	// Annex M
					axM = 1;
				else
					axM = 0;
			}

			adsl_drv_get(RLCM_SET_ANNEX_B, (void *)&axB, 4);
			adsl_drv_get(RLCM_SET_ANNEX_L, (void *)&axL, 4);
			adsl_drv_get(RLCM_SET_ANNEX_M, (void *)&axM, 4);

#ifdef ENABLE_ADSL_MODE_GINP
			if (dsl_mode & ADSL_MODE_GINP)	// G.INP
				xmode = DSL_FUNC_GINP;
			else
				xmode = 0;
			adsl_drv_get(RLCM_SET_DSL_FUNC, (void *)&xmode, 4);
#endif

			xmode=0;
			if (dsl_mode & (ADSL_MODE_GLITE|ADSL_MODE_T1413|ADSL_MODE_GDMT))
				xmode |= 1;	// ADSL1
			if (dsl_mode & ADSL_MODE_ADSL2)
				xmode |= 2;	// ADSL2
			if (dsl_mode & ADSL_MODE_ADSL2P)
				xmode |= 4;	// ADSL2+
			adsl_drv_get(RLCM_SET_XDSL_MODE, (void *)&xmode, 4);

			// set OLR Type
			mib_get(MIB_ADSL_OLR, (void *)&olr);

			val = (int)olr;
			if (val == 2) // SRA (should include bitswap)
				val = 3;

			adsl_drv_get(RLCM_SET_OLR_TYPE, (void *)&val, 4);

			// set Tone mask
			mib_get(MIB_ADSL_TONE, (void *)tone);
			adsl_drv_get(RLCM_LOADCARRIERMASK, (void *)tone, GET_LOADCARRIERMASK_SIZE);

			mib_get(MIB_ADSL_HIGH_INP, (void *)&inp);
			xmode = inp;
			adsl_drv_get(RLCM_SET_HIGH_INP, (void *)&xmode, 4);

			// new_hibrid
			mib_get(MIB_DEVICE_TYPE, (void *)&inp);
			xmode = inp;
			switch(xmode) {
				case 1:
					xmode = 1052;
					break;
				case 2:
					xmode = 2099;
					break;
				case 3:
					xmode = 3099;
					break;
				case 4:
					xmode = 4052;
					break;
				case 5:
					xmode = 5099;
					break;
				case 9:
					xmode = 9099;
					break;
				default:
					xmode = 9099;
			}
			adsl_drv_get(RLCM_SET_HYBRID, (void *)&xmode, 4);
			// Start handshaking; should be the last command.
			adsl_drv_get(RLCM_SET_ADSL_MODE, (void *)&adslmode, 4);
			ret = 1;
#endif /*CONFIG_VDSL*/
		}
		else
			ret = 0;
	}
	else
		ret = -1;

#ifdef FIELD_TRY_SAFE_MODE
	unsigned char mode;
	SafeModeData vSmd;

	memset((void *)&vSmd, 0, sizeof(vSmd));
	mib_get(MIB_ADSL_FIELDTRYSAFEMODE, (void *)&mode);
	vSmd.FieldTrySafeMode = (int)mode;
	mib_get(MIB_ADSL_FIELDTRYTESTPSDTIMES, (void *)&vSmd.FieldTryTestPSDTimes);
	mib_get(MIB_ADSL_FIELDTRYCTRLIN, (void *)&vSmd.FieldTryCtrlIn);
	adsl_drv_get(RLCM_SET_SAFEMODE_CTRL, (void *)&vSmd, SAFEMODE_DATA_SIZE);
#endif

	return ret;
}
#endif // of CONFIG_DEV_xDSL

// return 1: found Prefix Delegation.
// return 0: can not find Prefix Delegation.
int getLeasesInfo(const char *fname, DLG_INFO_Tp pInfo)
{
	FILE *fp;
	char temps[0x100];
	char *str, *endStr;
	int offset, RNTime, RBTime, PLTime, MLTime;
	struct in6_addr addr6;

	int ret=0;

	if ((fp = fopen(fname, "r")) == NULL)
	{
		printf("Open file %s fail !\n", fname);
		return 0;
	}

	while (fgets(temps,0x100,fp))
	{
		if (temps[strlen(temps)-1]=='\n')
			temps[strlen(temps)-1] = 0;

		if (str=strstr(temps, "ia-pd"))
		{
			ret = 1;
			fgets(temps,0x100,fp);

			// Get renew
			fgets(temps,0x100,fp);
			if ( str=strstr(temps, "renew") ) {
				offset = strlen("renew")+1;
				if ( endStr=strchr(str+offset, ';')) {
					*endStr=0;
					sscanf(str+offset, "%u", &RNTime);
					pInfo->RNTime = RNTime;
				}
			}

			// Get rebind
			fgets(temps,0x100,fp);
			if ( str=strstr(temps, "rebind")) {
				offset = strlen("rebind")+1;
				if ( endStr=strchr(str+offset, ';')) {
					*endStr=0;
					sscanf(str+offset, "%u", &RBTime);
					pInfo->RBTime = RBTime;
				}
			}

			// Get prefix
			fgets(temps,0x100,fp);
			if ( str=strstr(temps, "iaprefix")) {
				offset = strlen("iaprefix")+1;
				if ( endStr=strchr(str+offset, ' ')) {
					*endStr=0;

					endStr=strchr(str+offset, '/');
					*endStr=0;

					// PrefixIP
					inet_pton(PF_INET6, (str+offset), &addr6);
					memcpy(pInfo->prefixIP, &addr6, IP6_ADDR_LEN);

					// Prefix Length
					//sscanf((endStr+1), "%d", &(Info.prefixLen));
					pInfo->prefixLen = (char)atoi((endStr+1));
				}

			}


			fgets(temps,0x100,fp);

			// Get preferred-life
			fgets(temps,0x100,fp);
			if ( str=strstr(temps, "preferred-life")) {
				offset = strlen("preferred-life")+1;
				if ( endStr=strchr(str+offset, ';') ) {
					*endStr=0;
					sscanf(str+offset, "%u", &PLTime);
					pInfo->PLTime = PLTime;
				}
			}

			// Get max-life
			fgets(temps,0x100,fp);
			if( str=strstr(temps, "max-life")) {
				offset = strlen("max-life")+1;
				if ( endStr=strchr(str+offset, ';')) {
					*endStr=0;
					sscanf(str+offset, "%u", &MLTime);
					pInfo->MLTime = MLTime;
				}
			}
		}
		if ( str=strstr(temps, "dhcp6.name-servers")) {
			offset = strlen("dhcp6.name-servers")+1;
			if ( endStr=strchr(str+offset, ';')) {
				*endStr=0;
				//printf("Name server=%s\n", str+offset);
				memcpy(pInfo->nameServer, (unsigned char *)(str+offset),  256);
			}
		}
	}
	fclose(fp);

	return ret;
}

int getMIB2Str(unsigned int id, char *strbuf)
{
	unsigned char buffer[64];

	if (!strbuf)
		return -1;

	switch (id) {
		// INET address
		case MIB_ADSL_LAN_IP:
		case MIB_ADSL_LAN_SUBNET:
		case MIB_ADSL_LAN_IP2:
		case MIB_ADSL_LAN_SUBNET2:
		case MIB_ADSL_WAN_DNS1:
		case MIB_ADSL_WAN_DNS2:
		case MIB_ADSL_WAN_DNS3:
		case MIB_ADSL_WAN_DHCPS:
		case MIB_DMZ_IP:
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
		case MIB_SNMP_TRAP_IP:
#endif
		case MIB_DHCP_POOL_START:
		case MIB_DHCP_POOL_END:
		case MIB_DHCPS_DNS1:
		case MIB_DHCPS_DNS2:
		case MIB_DHCPS_DNS3:
		case MIB_DHCP_SUBNET_MASK:
#ifdef AUTO_PROVISIONING
		case MIB_HTTP_SERVER_IP:
#endif
		case MIB_ADSL_LAN_DHCP_GATEWAY:
#if 1
#if defined(_PRMT_X_CT_COM_DHCP_)||defined(IP_BASED_CLIENT_TYPE)
		case CWMP_CT_STB_MINADDR:
		case CWMP_CT_STB_MAXADDR:
		case CWMP_CT_PHN_MINADDR:
		case CWMP_CT_PHN_MAXADDR:
		case CWMP_CT_CMR_MINADDR:
		case CWMP_CT_CMR_MAXADDR:
		case CWMP_CT_PC_MINADDR:
		case CWMP_CT_PC_MAXADDR:
		case CWMP_CT_HGW_MINADDR:
		case CWMP_CT_HGW_MAXADDR:
#endif //_PRMT_X_CT_COM_DHCP_
#endif
		//ql 20090122 add
#ifdef IMAGENIO_IPTV_SUPPORT
		case MIB_IMAGENIO_DNS1:
		case MIB_IMAGENIO_DNS2:
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
		case MIB_OPCH_ADDRESS:
#endif
/*ping_zhang:20090930 END*/
#endif

#ifdef ADDRESS_MAPPING
#ifndef MULTI_ADDRESS_MAPPING
		case MIB_LOCAL_START_IP:
		case MIB_LOCAL_END_IP:
		case MIB_GLOBAL_START_IP:
		case MIB_GLOBAL_END_IP:
#endif //end of !MULTI_ADDRESS_MAPPING
#endif
#ifdef DEFAULT_GATEWAY_V2
		case MIB_ADSL_WAN_DGW_IP:
#endif
#ifdef CONFIG_USER_RTK_SYSLOG
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
		case MIB_SYSLOG_SERVER_IP:
#endif
#ifdef SEND_LOG
		case MIB_LOG_SERVER_IP:
#endif
#endif
#ifdef _PRMT_TR143_
		case TR143_UDPECHO_SRCIP:
#endif //_PRMT_TR143_
			if(!mib_get( id, (void *)buffer))
				return -1;
			// Mason Yu
			if ( ((struct in_addr *)buffer)->s_addr == INADDR_NONE ) {
				sprintf(strbuf, "%s", "");
			} else {
				sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)buffer)));
			}
			break;
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
		// INET6 address
		case MIB_DHCPV6S_RANGE_START:
		case MIB_DHCPV6S_RANGE_END:
			if(!mib_get( id, (void *)buffer))
				return -1;
			inet_ntop(PF_INET6, buffer, strbuf, 48);
			break;
		case MIB_DHCPV6S_MIN_ADDRESS:
		case MIB_DHCPV6S_MAX_ADDRESS:
		case MIB_IPV6_LAN_PREFIX:
			if(!mib_get( id, (void *)strbuf))
				return -1;
			break;
#endif
		case MIB_ADSL_WAN_DNSV61:
		case MIB_ADSL_WAN_DNSV62:
		case MIB_ADSL_WAN_DNSV63:
			if(!mib_get( id, (void *)buffer))
				return -1;
			inet_ntop(PF_INET6, buffer, strbuf, 48);
			break;
#endif
		// Ethernet address
		case MIB_ELAN_MAC_ADDR:
		case MIB_WLAN_MAC_ADDR:
			if(!mib_get( id,  (void *)buffer))
				return -1;
			sprintf(strbuf, "%02x%02x%02x%02x%02x%02x", buffer[0], buffer[1],
				buffer[2], buffer[3], buffer[4], buffer[5]);
			break;
		// Char
		case MIB_ADSL_LAN_CLIENT_START:
		case MIB_ADSL_LAN_CLIENT_END:
#ifdef WLAN_SUPPORT
		case MIB_WLAN_CHAN_NUM:
		case MIB_WLAN_CHANNEL_WIDTH:
		case MIB_WLAN_NETWORK_TYPE:
		case MIB_WIFI_TEST:

#ifdef WLAN_UNIVERSAL_REPEATER
		case MIB_REPEATER_ENABLED1:
#endif
#ifdef WLAN_WDS
		case MIB_WLAN_WDS_ENABLED:
#endif

#endif
#ifdef CONFIG_USER_RTK_SYSLOG
		case MIB_SYSLOG_LOG_LEVEL:
		case MIB_SYSLOG_DISPLAY_LEVEL:
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
		case MIB_SYSLOG_MODE:
#endif
#endif
#if 0
#if defined(_PRMT_X_CT_COM_DHCP_)||defined(IP_BASED_CLIENT_TYPE)
		case CWMP_CT_STB_MINADDR:
		case CWMP_CT_STB_MAXADDR:
		case CWMP_CT_PHN_MINADDR:
		case CWMP_CT_PHN_MAXADDR:
		case CWMP_CT_CMR_MINADDR:
		case CWMP_CT_CMR_MAXADDR:
		case CWMP_CT_PC_MINADDR:
		case CWMP_CT_PC_MAXADDR:
		case CWMP_CT_HGW_MINADDR:
		case CWMP_CT_HGW_MAXADDR:
#endif //_PRMT_X_CT_COM_DHCP_
#endif
#if defined(CONFIG_IPV6) && defined (CONFIG_USER_DHCPV6_ISC_DHCP411)
		case MIB_DHCPV6S_PREFIX_LENGTH:
#endif
#ifdef CONFIG_RTK_RG_INIT
		case MIB_MAC_BASED_TAG_DECISION:
#endif
#if defined(CONFIG_IPV6)
		case MIB_LAN_DNSV6_MODE:
		case MIB_PREFIXINFO_PREFIX_MODE:
#endif
		case CWMP_CONFIGURABLE:
			if(!mib_get(id,  (void *)buffer))
				return -1;
	   		sprintf(strbuf, "%u", *(unsigned char *)buffer);
	   		break;
	   	// Short
	   	case MIB_BRCTL_AGEINGTIME:
#ifdef WLAN_SUPPORT
	   	case MIB_WLAN_FRAG_THRESHOLD:
	   	case MIB_WLAN_RTS_THRESHOLD:
	   	case MIB_WLAN_BEACON_INTERVAL:
#endif
#ifdef CONFIG_USER_RTK_SYSLOG
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
		case MIB_SYSLOG_SERVER_PORT:
#endif
#endif
		//ql 20090122 add
#ifdef IMAGENIO_IPTV_SUPPORT
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
		case MIB_OPCH_PORT:
#endif
/*ping_zhang:20090930 END*/
#endif
			if(!mib_get( id,  (void *)buffer))
				return -1;
			sprintf(strbuf, "%u", *(unsigned short *)buffer);
			break;
	   	// Interger
		case MIB_ADSL_LAN_DHCP_LEASE:
			// Mason Yu
			if(!mib_get( id,  (void *)buffer))
				return -1;
			// if MIB_ADSL_LAN_DHCP_LEASE=0xffffffff, it indicate an infinate lease
			if ( *(unsigned long *)buffer == 0xffffffff )
				sprintf(strbuf, "-1");
			else
				sprintf(strbuf, "%u", *(unsigned int *)buffer);
			break;
		// Interger
#ifdef WEB_REDIRECT_BY_MAC
		case MIB_WEB_REDIR_BY_MAC_INTERVAL:
#endif
		case MIB_IGMP_PROXY_ITF:
#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
		case MIB_UPNP_EXT_ITF:
#endif
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_ECMH		// Mason Yu. MLD Proxy
		case MIB_MLD_PROXY_EXT_ITF:
#endif
#endif

#ifdef IP_PASSTHROUGH
		case MIB_IPPT_ITF:
		case MIB_IPPT_LEASE:
#endif
#ifdef DEFAULT_GATEWAY_V2
		case MIB_ADSL_WAN_DGW_ITF:
#endif
		case MIB_MAXLOGLEN:
#ifdef _CWMP_MIB_
		case CWMP_INFORM_INTERVAL:
#endif
#ifdef DOS_SUPPORT
		case MIB_DOS_SYSSYN_FLOOD:
		case MIB_DOS_SYSFIN_FLOOD:
		case MIB_DOS_SYSUDP_FLOOD:
		case MIB_DOS_SYSICMP_FLOOD:
		case MIB_DOS_PIPSYN_FLOOD:
		case MIB_DOS_PIPFIN_FLOOD:
		case MIB_DOS_PIPUDP_FLOOD:
		case MIB_DOS_PIPICMP_FLOOD:
		case MIB_DOS_BLOCK_TIME:
#endif
#ifdef TCP_UDP_CONN_LIMIT
		case MIB_CONNLIMIT_UDP:
		case MIB_CONNLIMIT_TCP:
#endif
#ifdef _CWMP_MIB_
		case CWMP_CONREQ_PORT:
		case CWMP_WAN_INTERFACE:
#endif
#ifdef CONFIG_MIDDLEWARE
		case CWMP_MIDWARE_SERVER_PORT:
#endif
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
		case MIB_DHCPV6S_DEFAULT_LEASE:
		case MIB_DHCPV6S_PREFERRED_LIFETIME:
		case MIB_DHCPV6S_RENEW_TIME:
		case MIB_DHCPV6S_REBIND_TIME:
#endif
#endif
#ifdef CONFIG_RTK_RG_INIT
		case MIB_LAN_VLAN_ID1:
		case MIB_LAN_VLAN_ID2:
#endif
		case MIB_WAN_MODE:
#ifdef TIME_ZONE
		case MIB_NTP_TIMEZONE_DB_INDEX:
#endif
#ifdef _PRMT_X_CT_COM_USERINFO_
		case CWMP_USERINFO_STATUS:
		case CWMP_USERINFO_RESULT:
#endif

#if defined(CONFIG_IPV6)
		case MIB_PREFIXINFO_DELEGATED_WANCONN:
		case MIB_DNSINFO_WANCONN:
#endif
			if(!mib_get( id,  (void *)buffer))
				return -1;
			sprintf(strbuf, "%u", *(unsigned int *)buffer);
			break;
		// String
		case MIB_ADSL_LAN_DHCP_DOMAIN:
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
		case MIB_SNMP_SYS_DESCR:
		case MIB_SNMP_SYS_CONTACT:
		case MIB_SNMP_SYS_LOCATION:
		case MIB_SNMP_SYS_OID:
		case MIB_SNMP_COMM_RO:
		case MIB_SNMP_COMM_RW:
#endif
		case MIB_SNMP_SYS_NAME:
#ifdef WLAN_SUPPORT
#ifdef WLAN_UNIVERSAL_REPEATER
		case MIB_REPEATER_SSID1:
#endif
#ifdef WLAN_WDS
		case MIB_WLAN_WDS_PSK:
#endif
#endif
#ifdef AUTO_PROVISIONING
		case MIB_CONFIG_VERSION:
#endif
#ifdef TIME_ZONE
		case MIB_NTP_ENABLED:
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
		case MIB_NTP_SERVER_HOST1:
		case MIB_NTP_SERVER_HOST2:
#endif
/*ping_zhang:20081217*/
#ifdef _CWMP_MIB_
		case CWMP_PROVISIONINGCODE:
		case CWMP_ACS_URL:
		case CWMP_ACS_USERNAME:
		case CWMP_ACS_PASSWORD:
		case CWMP_CONREQ_USERNAME:
		case CWMP_CONREQ_PASSWORD:
		case CWMP_CONREQ_PATH:
		case CWMP_LAN_CONFIGPASSWD:
		case CWMP_SERIALNUMBER:
		case CWMP_DL_COMMANDKEY:
		case CWMP_RB_COMMANDKEY:
		case CWMP_ACS_PARAMETERKEY:
		case CWMP_CERT_PASSWORD:
#ifdef _PRMT_USERINTERFACE_
		case UIF_AUTOUPDATESERVER:
		case UIF_USERUPDATESERVER:
#endif
		case CWMP_SI_COMMANDKEY:
		case CWMP_ACS_KICKURL:
		case CWMP_ACS_DOWNLOADURL:
#ifdef _PRMT_X_CT_COM_PORTALMNT_
		case CWMP_CT_PM_URL4PC:
		case CWMP_CT_PM_URL4STB:
		case CWMP_CT_PM_URL4MOBILE:
#endif
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG //WPS
		case MIB_WSC_PIN:
		case MIB_WSC_SSID:
#endif
#ifdef CONFIG_USER_RTK_SYSLOG
#ifdef SEND_LOG
		case MIB_LOG_SERVER_NAME:
#endif
#endif
#if defined(CONFIG_IPV6) && defined(CONFIG_USER_RADVD)
		case MIB_V6_MAXRTRADVINTERVAL:
		case MIB_V6_MINRTRADVINTERVAL:
		case MIB_V6_ADVCURHOPLIMIT:
		case MIB_V6_ADVDEFAULTLIFETIME:
		case MIB_V6_ADVREACHABLETIME:
		case MIB_V6_ADVRETRANSTIMER:
		case MIB_V6_ADVLINKMTU:
		case MIB_V6_PREFIX_IP:
		case MIB_V6_PREFIX_LEN:
		case MIB_V6_VALIDLIFETIME:
		case MIB_V6_PREFERREDLIFETIME:
		case MIB_V6_RDNSS1:
		case MIB_V6_RDNSS2:
		case MIB_V6_ULAPREFIX_ENABLE:
		case MIB_V6_ULAPREFIX:
		case MIB_V6_ULAPREFIX_LEN:
		case MIB_V6_ULAPREFIX_VALID_TIME:
		case MIB_V6_ULAPREFIX_PREFER_TIME:
#endif
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
		case MIB_DHCPV6S_CLIENT_DUID:
#endif
#endif
		case MIB_SUSER_NAME:
		case MIB_USER_NAME:
#ifdef CONFIG_USER_SAMBA
#ifdef CONFIG_USER_NMBD
		case MIB_SAMBA_NETBIOS_NAME:
#endif
		case MIB_SAMBA_SERVER_STRING:
#endif
			if(!mib_get( id,  (void *)strbuf)){
				return -1;
			}
			break;

#ifdef CONFIG_RTL_WAPI_SUPPORT
		case MIB_WLAN_WAPI_MCAST_REKEYTYPE:
		case MIB_WLAN_WAPI_UCAST_REKETTYPE:
			if(!mib_get( id,  (void *)buffer)){
				return -1;
			}
			sprintf(strbuf, "%d", buffer[0]);
			break;
		case MIB_WLAN_WAPI_MCAST_TIME:
		case MIB_WLAN_WAPI_MCAST_PACKETS:
		case MIB_WLAN_WAPI_UCAST_TIME:
		case MIB_WLAN_WAPI_UCAST_PACKETS:
			if(!mib_get( id,  (void *)buffer)){
				return -1;
			}
			sprintf(strbuf, "%u", *(unsigned int *)buffer);
			break;
#endif
                // below are version information for tr069 inform packet
                case RTK_DEVID_MANUFACTURER:
                case RTK_DEVID_OUI:
                case RTK_DEVID_PRODUCTCLASS:
                case RTK_DEVINFO_HWVER:
                case RTK_DEVINFO_SWVER:
                case RTK_DEVINFO_SPECVER:
                case MIB_HW_SERIAL_NUMBER:
#if defined(CONFIG_GPON_FEATURE)
                case MIB_GPON_SN:
#endif

#ifdef CONFIG_MIDDLEWARE
				case CWMP_MIDWARE_SERVER_ADDR:
#endif

		if(!mib_get( id,  (void *)strbuf))
			return -1;
		break;

		default:
			return -1;
	}

	return 0;
}

int getSYS2Str(SYSID_T id, char *strbuf)
{
	unsigned char buffer[128], vChar;
	struct sysinfo info;
	int updays, uphours, upminutes, len, i;
	time_t tm;
	struct tm tm_time, *ptm_time;
	FILE *fp;
	unsigned char tmpBuf[64], *pStr;
	unsigned short vUShort;
	unsigned int vUInt;
#ifdef CONFIG_IPV6
	struct ipv6_ifaddr ip6_addr[6];
#endif
	struct stat f_status;
#ifdef WLAN_SUPPORT
	MIB_CE_MBSSIB_T Entry;
#endif

	if (!strbuf)
		return -1;

	strbuf[0] = '\0';

	switch (id) {
		case SYS_UPTIME:
			sysinfo(&info);
			updays = (int) info.uptime / (60*60*24);
			if (updays)
				sprintf(strbuf, "%d day%s, ", updays, (updays != 1) ? "s" : "");
			len = strlen(strbuf);
			upminutes = (int) info.uptime / 60;
			uphours = (upminutes / 60) % 24;
			upminutes %= 60;
			if(uphours)
				sprintf(&strbuf[len], "%2d:%02d", uphours, upminutes);
			else
				sprintf(&strbuf[len], "%d min", upminutes);
			break;
		case SYS_DATE:
	 		time(&tm);
			memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
			strftime(strbuf, 200, "%a %b %e %H:%M:%S %Z %Y", &tm_time);
			break;
		case SYS_YEAR:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_year+ 1900));
			break;
		case SYS_MONTH:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_mon+ 1));
			break;
		case SYS_DAY:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_mday));
			break;
		case SYS_HOUR:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_hour));
			break;
		case SYS_MINUTE:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_min));
			break;
		case SYS_SECOND:
	 		time(&tm);
			ptm_time = localtime(&tm);
			snprintf(strbuf, 64, "%d", (ptm_time->tm_sec));
			break;
		case SYS_FWVERSION:
#ifdef EMBED
			tmpBuf[0]=0;
			pStr = 0;
			fp = fopen("/etc/version", "r");
			if (fp!=NULL) {
				fgets(tmpBuf, sizeof(tmpBuf), fp);  //main version
				fclose(fp);
				pStr = strchr(tmpBuf, ' ');
				*pStr=0;
			};
			sprintf(strbuf, "%s", tmpBuf);
#else
			sprintf(strbuf, "%s.%s", "Realtek", "1");
#endif
			break;
		case SYS_LAN_DHCP:
			if ( !mib_get( MIB_DHCP_MODE, (void *)buffer) )
				return -1;
			if (DHCP_LAN_SERVER == buffer[0])
				strcpy(strbuf, STR_ENABLE);
			else
				strcpy(strbuf, STR_DISABLE);
			break;
		case SYS_DHCP_LAN_IP:
#if defined(CONFIG_SECONDARY_IP) && !defined(DHCPS_POOL_COMPLETE_IP)
			mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
			if (vChar)
				mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar);
#else
			vChar = 0;
#endif
			if (vChar == 0) // primary LAN
				getMIB2Str(MIB_ADSL_LAN_IP, strbuf);
			else // secondary LAN
				getMIB2Str(MIB_ADSL_LAN_IP2, strbuf);
			break;
		case SYS_DHCP_LAN_SUBNET:
#if defined(CONFIG_SECONDARY_IP) && !defined(DHCPS_POOL_COMPLETE_IP)
			mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
			if (vChar)
				mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar);
#else
			vChar = 0;
#endif
			if (vChar == 0) // primary LAN
				getMIB2Str(MIB_ADSL_LAN_SUBNET, strbuf);
			else // secondary LAN
				getMIB2Str(MIB_ADSL_LAN_SUBNET2, strbuf);
			break;
			// Kaohj
		case SYS_DHCPS_IPPOOL_PREFIX:
#if defined(CONFIG_SECONDARY_IP) && !defined(DHCPS_POOL_COMPLETE_IP)
			mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
			if (vChar)
				mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar);
#else
			vChar = 0;
#endif
			if (vChar == 0) // primary LAN
				mib_get(MIB_ADSL_LAN_IP, (void *)&tmpBuf[0]) ;
			else // secondary LAN
				mib_get(MIB_ADSL_LAN_IP2, (void *)&tmpBuf[0]) ;
			pStr = tmpBuf;
			sprintf(strbuf, "%d.%d.%d.", pStr[0], pStr[1], pStr[2]);
			break;
#ifdef WLAN_SUPPORT
		case SYS_WLAN:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if (0 == Entry.wlanDisabled)
				strcpy(strbuf, STR_ENABLE);
			else
				strcpy(strbuf, STR_DISABLE);
			break;
		case SYS_WLAN_SSID:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			strcpy(strbuf, Entry.ssid);
			break;
		case SYS_WLAN_DISABLED:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf,"%u", Entry.wlanDisabled);
			break;
		case SYS_WLAN_HIDDEN_SSID:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf,"%u", Entry.hidessid);
			break;
		case SYS_WLAN_MODE_VAL:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf,"%u", Entry.wlanMode);
			break;
		case SYS_WLAN_BCASTSSID:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if (0 == Entry.hidessid)
				strcpy(strbuf, STR_ENABLE);
			else
				strcpy(strbuf, STR_DISABLE);
			break;
		case SYS_WLAN_ENCRYPT_VAL:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf,"%u", Entry.encrypt);
			break;
		case SYS_WLAN_WPA_CIPHER_SUITE:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf,"%u", Entry.unicastCipher);
			break;
		case SYS_WLAN_WPA2_CIPHER_SUITE:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf,"%u", Entry.wpa2UnicastCipher);
			break;
		case SYS_WLAN_WPA_AUTH:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf,"%u", Entry.wpaAuth);
			break;
		case SYS_WLAN_BAND:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			strcpy(strbuf, wlan_band[(BAND_TYPE_T)Entry.wlanBand]);
			break;
		case SYS_WLAN_AUTH:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if(Entry.authType >= 0 && Entry.authType <= 2)
				strcpy(strbuf, wlan_auth[(AUTH_TYPE_T)Entry.authType]);
			else
				strcpy(strbuf, "None");
			break;
		case SYS_WLAN_PREAMBLE:
			if ( !mib_get( MIB_WLAN_PREAMBLE_TYPE, (void *)buffer) )
				return -1;
			strcpy(strbuf, wlan_preamble[(PREAMBLE_T)buffer[0]]);
			break;
		case SYS_WLAN_ENCRYPT:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			switch(Entry.encrypt) {
				case WIFI_SEC_NONE:
				case WIFI_SEC_WEP:
				case WIFI_SEC_WPA:
					strcpy(strbuf, wlan_encrypt[(WIFI_SECURITY_T)Entry.encrypt]);
					break;
				case WIFI_SEC_WPA2:
					strcpy(strbuf, wlan_encrypt[3]);
					break;
				case WIFI_SEC_WPA2_MIXED:
					strcpy(strbuf, wlan_encrypt[4]);
					break;
				#ifdef CONFIG_RTL_WAPI_SUPPORT
				case WIFI_SEC_WAPI:
					strcpy(strbuf, wlan_encrypt[5]);
					break;
				#endif
				default:
					strcpy(strbuf, wlan_encrypt[0]);
			}
			break;
		// Mason Yu. 201009_new_security
		case SYS_WLAN_WPA_CIPHER:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if ( Entry.unicastCipher == 0 )
				strcpy(strbuf, "");
			else
				strcpy(strbuf, wlan_Cipher[Entry.unicastCipher-1]);
			break;
		case SYS_WLAN_WPA2_CIPHER:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if ( Entry.wpa2UnicastCipher == 0 )
				strcpy(strbuf, "");
			else
				strcpy(strbuf, wlan_Cipher[Entry.wpa2UnicastCipher-1]);
			break;
		case SYS_WLAN_PSKFMT:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			strcpy(strbuf, wlan_pskfmt[Entry.wpaPSKFormat]);
			break;
		case SYS_WLAN_PSKVAL:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			for (len=0; len<strlen(Entry.wpaPSK); len++)
				strbuf[len]='*';
			strbuf[len]='\0';
			break;
		case SYS_WLAN_WEP_KEYLEN:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			strcpy(strbuf, wlan_wepkeylen[Entry.wep]);
			break;
		case SYS_WLAN_WEP_KEYFMT:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			strcpy(strbuf, wlan_wepkeyfmt[Entry.wepKeyType]);
			break;
		case SYS_WLAN_WPA_MODE:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if (Entry.wpaAuth == WPA_AUTH_AUTO)
				strcpy(strbuf, "Enterprise (RADIUS)");
			else if (Entry.wpaAuth == WPA_AUTH_PSK)
				strcpy(strbuf, "Personal (Pre-Shared Key)");
			break;
		case SYS_WLAN_RSPASSWD:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			for (len=0; len<strlen(Entry.rsPassword); len++)
				strbuf[len]='*';
			strbuf[len]='\0';
			break;
		case SYS_WLAN_RS_PORT:
			if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf, "%u", Entry.rsPort);
			break;
		case SYS_WLAN_RS_IP:
			if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if ( ((struct in_addr *)Entry.rsIpAddr)->s_addr == INADDR_NONE ) {
				sprintf(strbuf, "%s", "");
			} else {
				sprintf(strbuf, "%s", inet_ntoa(*((struct in_addr *)Entry.rsIpAddr)));
			}
			break;
		case SYS_WLAN_RS_PASSWORD:
			if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf, "%s", Entry.rsPassword);
			break;
		case SYS_WLAN_ENABLE_1X:
			if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf, "%u", Entry.enable1X);
			break;
		// Added by Jenny
		case SYS_TX_POWER:
			if ( !mib_get( MIB_TX_POWER, (void *)&buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, "100%");
			else if(1 == buffer[0])
				strcpy(strbuf, "70%");
			else if(2 == buffer[0])
				strcpy(strbuf, "50%");
			else if(3 == buffer[0])
				strcpy(strbuf, "35%");
			else if(4 == buffer[0])
				strcpy(strbuf, "15%");
			break;
		case SYS_WLAN_MODE:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			strcpy(strbuf, wlan_mode[(WLAN_MODE_T)Entry.wlanMode]);
			break;
		case SYS_WLAN_TXRATE:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
					return -1;
			if (0 == Entry.rateAdaptiveEnabled){
				for (i=0; i<12; i++)
					if (1<<i == Entry.fixedTxRate)
						strcpy(strbuf, wlan_rate[i]);
			}
			else if (1 == Entry.rateAdaptiveEnabled)
				strcpy(strbuf, STR_AUTO);
			break;
		case SYS_WLAN_BLOCKRELAY:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if (0 == Entry.userisolation)
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ENABLE);
			break;
		case SYS_WLAN_BLOCK_ETH2WIR:
			if ( !mib_get( MIB_WLAN_BLOCK_ETH2WIR, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ENABLE);
			break;
		case SYS_WLAN_AC_ENABLED:
			if ( !mib_get( MIB_WLAN_AC_ENABLED, (void *)&buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_DISABLE);
			else if(1 == buffer[0])
				strcpy(strbuf, "Allow Listed");
			else if(2 == buffer[0])
				strcpy(strbuf, "Deny Listed");
			else
				strcpy(strbuf, STR_ERR);
			break;
		case SYS_WLAN_WDS_ENABLED:
			if ( !mib_get( MIB_WLAN_WDS_ENABLED, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ENABLE);
			break;
#ifdef WLAN_QoS
		case SYS_WLAN_QoS:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if (0 == Entry.wmmEnabled)
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ENABLE);
			break;
#endif
#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
		case SYS_WLAN_WPS_ENABLED:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if (0 == Entry.wsc_disabled)
				strcpy(strbuf, STR_ENABLE);
			else
				strcpy(strbuf, STR_DISABLE);
			break;
		case SYS_WLAN_WPS_STATUS:
			if( !mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			if (0 == Entry.wsc_configured)
				strcpy(strbuf, "Unconfigured");
			else
				strcpy(strbuf, "Configured");
			break;
		// for PIN brute force attack
		case SYS_WLAN_WPS_LOCKDOWN:
			if (stat("/tmp/wscd_lock_stat", &f_status) == 0) {
				//printf("[%s %d] %s exist\n",__FUNCTION__,__LINE__,WSCD_LOCK_STAT);
				strbuf[0]='1';
			}else{
				strbuf[0]='0';
			}

			strbuf[1] = '\0';
			break;
		case SYS_WSC_DISABLE:
			if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf, "%u", Entry.wsc_disabled);
			break;
		case SYS_WSC_AUTH:
			if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf, "%u", Entry.wsc_auth);
			break;
		case SYS_WSC_ENC:
			if(!mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry))
				return -1;
			sprintf(strbuf, "%u", Entry.wsc_enc);
			break;
#endif
#endif // #ifdef WLAN_SUPPORT
		case SYS_DNS_MODE:
			if ( !mib_get( MIB_ADSL_WAN_DNS_MODE, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, STR_AUTO);
			else
				strcpy(strbuf, STR_MANUAL);
			break;

		case SYS_DHCP_MODE:
			if(!mib_get( MIB_DHCP_MODE, (void *)buffer) )
				return -1;
			strcpy(strbuf, dhcp_mode[(DHCP_TYPE_T)buffer[0]]);
			break;

		case SYS_IPF_OUT_ACTION:
			if(!mib_get(MIB_IPF_OUT_ACTION, (void *)buffer) ){
				return -1;
			}
			if (0 == buffer[0])
				strcpy(strbuf, "Deny");
			else if(1 == buffer[0])
				strcpy(strbuf, "Allow");
			else
				strcpy(strbuf, "err");
			break;

		case SYS_DEFAULT_PORT_FW_ACTION:
			if(!mib_get(MIB_PORT_FW_ENABLE, (void *)buffer) ){
				return -1;
			}
			if (0 == buffer[0])
				strcpy(strbuf, "Disable");
			else if(1 == buffer[0])
				strcpy(strbuf, "Enable");
			else
				strcpy(strbuf, "err");
			break;
#if defined(CONFIG_RTL_IGMP_SNOOPING)
		// Added by Jenny
		case SYS_IGMP_SNOOPING:
			if(!mib_get( MIB_MPMODE, (void *)&vChar)) {
				return -1;
			}
			if (vChar&MP_IGMP_MASK)
				strcpy(strbuf, STR_ENABLE);
			else
				strcpy(strbuf, STR_DISABLE);
			break;
#endif
		case SYS_IP_QOS:
			if(!mib_get( MIB_MPMODE, (void *)&vChar)) {
				return -1;
			}
			if (vChar&MP_IPQ_MASK)
				strcpy(strbuf, STR_ENABLE);
			else
				strcpy(strbuf, STR_DISABLE);
			break;


		// Added by Mason Yu
		case SYS_IPF_IN_ACTION:
			if ( !mib_get( MIB_IPF_IN_ACTION, (void *)buffer) )
				return -1;
			if (0 == buffer[0])
				strcpy(strbuf, "Deny");
			else if(1 == buffer[0])
				strcpy(strbuf, "Allow");
			else
				strcpy(strbuf, "err");
			break;

#ifdef CONFIG_SECONDARY_IP
		case SYS_LAN_IP2:
			if (!mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar))
				return -1;
			if (vChar == 0)
				strcpy(strbuf, STR_DISABLE);
			else
				strcpy(strbuf, STR_ENABLE);
			break;

		case SYS_LAN_DHCP_POOLUSE:
			if (!mib_get( MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar))
				return -1;
			if (vChar == 0)
				strcpy(strbuf, "Primary LAN");
			else
				strcpy(strbuf, "Secondary LAN");
			break;
#endif

#ifdef URL_BLOCKING_SUPPORT
		case SYS_DEFAULT_URL_BLK_ACTION:
			if(!mib_get(MIB_URL_CAPABILITY, (void *)buffer) ){
				return -1;
			}
			if (0 == buffer[0])
				strcpy(strbuf, "Disable");
			else if(1 == buffer[0])
				strcpy(strbuf, "Enable");
			else
				strcpy(strbuf, "err");
			break;
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
		case SYS_DEFAULT_DOMAIN_BLK_ACTION:
			if(!mib_get(MIB_DOMAINBLK_CAPABILITY, (void *)buffer) ){
				return -1;
			}
			if (0 == buffer[0])
				strcpy(strbuf, "Disable");
			else if(1 == buffer[0])
				strcpy(strbuf, "Enable");
			else
				strcpy(strbuf, "err");
			break;
#endif
#ifdef CONFIG_DEV_xDSL
		case SYS_DSL_OPSTATE:
			getAdslInfo(ADSL_GET_MODE, buffer, 64);
			if (buffer[0] != '\0')
				sprintf(strbuf, "%s,", buffer);
			getAdslInfo(ADSL_GET_STATE, buffer, 64);
			strcat(strbuf, buffer);
			break;
#ifdef CONFIG_USER_XDSL_SLAVE
		case SYS_DSL_SLV_OPSTATE:
			getAdslSlvInfo(ADSL_GET_MODE, buffer, 64);
			if (buffer[0] != '\0')
				sprintf(strbuf, "%s,", buffer);
			getAdslSlvInfo(ADSL_GET_STATE, buffer, 64);
			strcat(strbuf, buffer);
			break;
#endif /*CONFIG_USER_XDSL_SLAVE*/
#endif
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
		case SYS_DHCPV6_MODE:
			if(!mib_get( MIB_DHCPV6_MODE, (void *)buffer) )
				return -1;
			strcpy(strbuf, dhcp_mode[(DHCP_TYPE_T)buffer[0]]);
			break;

		case SYS_DHCPV6_RELAY_UPPER_ITF:
			if(!mib_get( MIB_DHCPV6R_UPPER_IFINDEX, (void *)&vUInt) )
				return -1;

			if (vUInt != DUMMY_IFINDEX)
			{
				ifGetName(vUInt, strbuf, 8);
			}
			else
			{
				snprintf(strbuf, 6, "None");
				return 0;
			}
			break;
#endif
		case SYS_LAN_IP6_LL:
			i=getifip6(LANIF, IPV6_ADDR_LINKLOCAL, ip6_addr, 1);
			if (!i)
				strbuf[0] = 0;
			else
				inet_ntop(PF_INET6, &ip6_addr[0].addr, tmpBuf, 256);
				sprintf(strbuf, "%s/%d", tmpBuf, ip6_addr[0].prefix_len);
			break;
		case SYS_LAN_IP6_GLOBAL:
			i=getifip6(LANIF, IPV6_ADDR_UNICAST, ip6_addr, 6);
			strbuf[0] = 0;
			if (i) {
				for (len=0; len<i; len++) {
					inet_ntop(PF_INET6, &ip6_addr[len].addr, tmpBuf, 256);
					if (len == 0)
						sprintf(strbuf, "%s/%d", tmpBuf, ip6_addr[len].prefix_len);
					else
						sprintf(strbuf, "%s, %s/%d", strbuf, tmpBuf, ip6_addr[len].prefix_len);
				}
			}
			break;
#endif // of CONFIG_IPV6

		default:
			return -1;
	}

	return 0;
}

int ifWanNum(const char *name)
{
	int ifnum=0;
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	int type;
	unsigned char protocol = IPVER_IPV4_IPV6; // 1:IPv4, 2: IPv6, 3: Both. Mason Yu. MLD Proxy

	if ( !strcmp(name, "all") )
		type = 0;
	else if ( !strcmp(name, "rt") )
	{
		type = 1;	// route interface
		protocol = IPVER_IPV4;
	}
	else if ( !strcmp(name, "rtv6") )	// Mason Yu. MLD Proxy
	{
		type = 1;	// route interface
		protocol = IPVER_IPV6;
	}
	else if ( !strcmp(name, "br") )
		type = 2;	// bridge interface
	else
		type = 1;	// default to route

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
			return -1;
		}

		if (Entry.enable == 0)
			continue;

// Mason Yu. MLD Proxy
#ifdef CONFIG_IPV6
		if ( protocol == IPVER_IPV4_IPV6 || Entry.IpProtocol == IPVER_IPV4_IPV6 || protocol == Entry.IpProtocol ) {
			if (type == 2) {
				if (Entry.cmode == CHANNEL_MODE_BRIDGE)
				{
					ifnum++;
				}
			}
			else {  // rt or all (1 or 0)
				if (type == 1 && Entry.cmode == CHANNEL_MODE_BRIDGE)
					continue;

				ifnum++;
			}
		}
#else
		if (type == 2) {
			if (Entry.cmode == CHANNEL_MODE_BRIDGE)
			{
				ifnum++;
			}
		}
		else {  // rt or all (1 or 0)
			if (type == 1 && Entry.cmode == CHANNEL_MODE_BRIDGE)
				continue;

			ifnum++;
		}
#endif

	}

	return ifnum;
}

#ifdef REMOTE_ACCESS_CTL
void remote_access_modify(MIB_CE_ACC_T accEntry, int enable)
{
	char *act;
	char strPort[8];
	int ret;


	if (enable)
		act = (char *)"-I";
	else
		act = (char *)FW_DEL;

	// telnet service: bring up by inetd
	#ifdef CONFIG_USER_TELNETD_TELNETD
	if (!(accEntry.telnet & 0x02)) {	// not LAN access
		// iptables -A inacc -i $LAN_IF -p TCP --dport 23 -j DROP
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
		(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "23", "-j", (char *)FW_DROP);

	#ifdef CONFIG_IPV6
		//ip6tables -I ipv6remoteacc -i br0 -p TCP --dport 23 -j DROP
		va_cmd(IP6TABLES, 10, 1, act, (char *)FW_IPV6REMOTEACC, (char *)ARG_I, (char *)LANIF
				, "-p", (char *)ARG_TCP, (char *)FW_DPORT, "23", "-j", (char *)FW_DROP);
	#endif
	}
	if (accEntry.telnet & 0x01) {	// can WAN access
		snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.telnet_port);
		va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
		 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

		// redirect if this is not standard port
		if (accEntry.telnet_port != 23) {
			ret = va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					 "!", (char *)ARG_I, (char *)LANIF,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "23");
		}

		// iptables -A inacc -i ! $LAN_IF -p TCP --dport 23 -j ACCEPT
		#if 0
		va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
		 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "23", "-j", (char *)FW_ACCEPT);

		fprintf(stderr, "telnet = %d\n", accEntry.telnet_port);
		if (accEntry.telnet_port != 23) {
			ret = va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					 "!", (char *)ARG_I, (char *)LANIF,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "23");
			fprintf(stderr, "telnet cmd = %d\n", ret);
		}
		#endif

	#ifdef CONFIG_IPV6
		//block IPv6 WAN access
		//FIXME: There's no redirect rule support for IPv6, we can only block standard port.

		//ip6tables -I ipv6remoteacc -i ! br0 -p TCP --dport 23 -j ACCEPT
		va_cmd(IP6TABLES, 11, 1, act, (char *)FW_IPV6REMOTEACC, (char *)ARG_I, "!", (char *)LANIF
				, "-p", (char *)ARG_TCP, (char *)FW_DPORT, "23", "-j", (char *)FW_ACCEPT);
	#endif

	}
	#endif	//CONFIG_USER_TELNETD_TELNETD


	#ifdef CONFIG_USER_FTPD_FTPD
	// ftp service: bring up by inetd
	if (!(accEntry.ftp & 0x02)) {	// not LAN access
		// iptables -A inacc -i $LAN_IF -p TCP --dport 21 -j DROP
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
		(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "21", "-j", (char *)FW_DROP);

	#ifdef CONFIG_IPV6
		//ip6tables -I ipv6remoteacc -i br0 -p TCP --dport 21 -j DROP
		va_cmd(IP6TABLES, 10, 1, act, (char *)FW_IPV6REMOTEACC, (char *)ARG_I, (char *)LANIF
				, "-p", (char *)ARG_TCP, (char *)FW_DPORT, "21", "-j", (char *)FW_DROP);
	#endif
	}
	if (accEntry.ftp & 0x01) {	// can WAN access
		snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.ftp_port);
		va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
		 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

		// redirect if this is not standard port
		if (accEntry.ftp_port != 21) {
			ret = va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					 "!", (char *)ARG_I, (char *)LANIF,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "21");
		}

		#if 0
		// iptables -A inacc -i ! $LAN_IF -p TCP --dport 21 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
		 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
		(char *)FW_DPORT, "21", "-j", (char *)FW_ACCEPT);
		#endif
#ifdef CONFIG_IPV6
		//block IPv6 WAN access
		//FIXME: There's no redirect rule support for IPv6, we can only block standard port.

		//ip6tables -A ipv6remoteacc -i ! br0 -p TCP --dport 23 -j ACCEPT
		va_cmd(IP6TABLES, 11, 1, act, (char *)FW_IPV6REMOTEACC, (char *)ARG_I, "!", (char *)LANIF
				, "-p", (char *)ARG_TCP, (char *)FW_DPORT, "23", "-j", (char *)FW_ACCEPT);
#endif

	}
	#endif	//CONFIG_USER_FTPD_FTPD

	#ifdef CONFIG_USER_TFTPD_TFTPD
	// tftp service: bring up by inetd
	if (!(accEntry.tftp & 0x02)) {	// not LAN access
		// iptables -A inacc -i $LAN_IF -p UDP --dport 69 -j DROP
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
		(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
		(char *)FW_DPORT, "69", "-j", (char *)FW_DROP);
	}
	if (accEntry.tftp & 0x01) {	// can WAN access
		// iptables -A inacc -i ! $LAN_IF -p UDP --dport 69 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
		 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
		(char *)FW_DPORT, "69", "-j", (char *)FW_ACCEPT);
	}
	#endif

	// HTTP service
		if (!(accEntry.web & 0x02)) {	// not LAN access
			// iptables -A inacc -i $LAN_IF -p TCP --dport 80 -j DROP
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "80", "-j", (char *)FW_DROP);
		}
		if (accEntry.web & 0x01) {	// can WAN access
			snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.web_port);
			// iptables -A inacc -i ! $LAN_IF -p TCP --dport 80 -j ACCEPT
			va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
				 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
				(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);
			#if 0
			va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
			 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
			#endif
			if (accEntry.web_port != 80) {
				va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					 "!", (char *)ARG_I, (char *)LANIF,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "80");
			}

		}

  #ifdef CONFIG_USER_BOA_WITH_SSL
  	  //HTTPS service
		if (!(accEntry.https & 0x02)) {	// not LAN access
			// iptables -A inacc -i $LAN_IF -p TCP --dport 443 -j DROP
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "443", "-j", (char *)FW_DROP);
		}
		if (accEntry.https & 0x01) {	// can WAN access
			snprintf(strPort, sizeof(strPort)-1, "%u", accEntry.https_port);
			// iptables -t mangle  -I PREROUTING -i !$LAN_IF -p TCP --dport 443 -j MARK --set-mark 0x1000
			va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
				 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
				(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);

			if (accEntry.https_port != 443) {
				va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)act, (char *)FW_PREROUTING,
					 "!", (char *)ARG_I, (char *)LANIF,
					"-p", ARG_TCP,
					(char *)FW_DPORT, strPort, "-j",
					"REDIRECT", "--to-ports", "443");
			}

		}
  #endif

	#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	// snmp service
	//if (accEntry.snmp !=0) {	// have snmp server
		if (!(accEntry.snmp & 0x02)) {	// not LAN access
			// iptables -A inacc -i $LAN_IF -p UDP --dport 161:162 -j DROP
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
			(char *)FW_DPORT, "161:162", "-j", (char *)FW_DROP);
		}
		if (accEntry.snmp & 0x01) {	// can WAN access
			// iptables -A inacc -i ! $LAN_IF -p UDP --dport 161:162 -m limit
			//  --limit 100/s --limit-burst 500 -j ACCEPT
			va_cmd(IPTABLES, 17, 1, act, (char *)FW_INACC,
				 "!", (char *)ARG_I, (char *)LANIF, "-p",
				(char *)ARG_UDP, (char *)FW_DPORT, "161:162", "-m",
				"limit", "--limit", "100/s", "--limit-burst",
				"500", "-j", (char *)FW_ACCEPT);
		}
	#endif

	#ifdef CONFIG_USER_SSH_DROPBEAR
	// ssh service
		if (!(accEntry.ssh & 0x02)) {	// not LAN access
			// iptables -A inacc -i $LAN_IF -p TCP --dport 22 -j DROP
			va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "22", "-j", (char *)FW_DROP);
		}
		if (accEntry.ssh & 0x01) {	// can WAN access
			// iptables -A inacc -i ! $LAN_IF -p TCP --dport 22 -j ACCEPT
			va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
			 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "22", "-j", (char *)FW_ACCEPT);
		}
	#endif

	// ping service
	/*
	if (1) // can LAN access
	{
		// iptables -A INPUT -i $LAN_IF  -p ICMP --icmp-type echo-request -m limit
		//   --limit 1/s -j ACCEPT
		va_cmd(IPTABLES, 14, 1, act, (char *)FW_INPUT,
			(char *)ARG_I, (char *)LANIF, "-p", "ICMP",
			"--icmp-type", "echo-request", "-m", "limit",
			"--limit", "1/s", "-j", (char *)FW_ACCEPT);
	}
	*/

	if (accEntry.icmp & 0x01) // can WAN access
	{
		// iptables -A inacc -i ! $LAN_IF  -p ICMP --icmp-type echo-request -m limit
		//   --limit 1/s -j ACCEPT
		va_cmd(IPTABLES, 15, 1, act, (char *)FW_INACC,
			 "!", (char *)ARG_I, (char *)LANIF, "-p", "ICMP",
			"--icmp-type", "echo-request", "-m", "limit",
			"--limit", "1/s", "-j", (char *)FW_ACCEPT);
	}

	#ifdef CONFIG_USER_NETLOGGER_SUPPORT
	if (accEntry.netlog & 0x01) {	// can WAN access
		snprintf(strPort, sizeof(strPort)-1, "%u", 0x1234);
		va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", act, (char *)FW_PREROUTING,
		 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);
	}
	#endif //CONFIG_USER_NETLOGGER_SUPPORT

#ifdef CONFIG_USER_SAMBA
	if (!(accEntry.smb & 0x02)) {	// not LAN access
		// iptables -A inacc -i $LAN_IF -p TCP --dport 139 -j DROP
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "139", "-j", (char *)FW_DROP);
		// iptables -A inacc -i $LAN_IF -p TCP --dport 445 -j DROP
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "445", "-j", (char *)FW_DROP);
		// iptables -A inacc -i $LAN_IF -p UDP --dport 137 -j DROP
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
			(char *)FW_DPORT, "137", "-j", (char *)FW_DROP);
		// iptables -A inacc -i $LAN_IF -p UDP --dport 138 -j DROP
		va_cmd(IPTABLES, 10, 1, act, (char *)FW_INACC,
			(char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
			(char *)FW_DPORT, "138", "-j", (char *)FW_DROP);
	}
	if (accEntry.ssh & 0x01) {	// can WAN access
		// iptables -A inacc -i ! $LAN_IF -p TCP --dport 139 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
		 	"!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "139", "-j", (char *)FW_ACCEPT);
		// iptables -A inacc -i ! $LAN_IF -p TCP --dport 445 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
		 	"!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP,
			(char *)FW_DPORT, "445", "-j", (char *)FW_ACCEPT);
		// iptables -A inacc -i ! $LAN_IF -p UDP --dport 137 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
		 	"!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
			(char *)FW_DPORT, "137", "-j", (char *)FW_ACCEPT);
		// iptables -A inacc -i ! $LAN_IF -p UDP --dport 138 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, act, (char *)FW_INACC,
		 	"!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
			(char *)FW_DPORT, "138", "-j", (char *)FW_ACCEPT);
	}
#endif
}

void filter_set_remote_access(int enable)
{
	MIB_CE_ACC_T accEntry;
	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&accEntry))
		return;
	remote_access_modify( accEntry, enable );

	return;

}
#endif // of REMOTE_ACCESS_CTL

#ifdef IP_ACL
void filter_set_acl(int enable)
{
	int i, total;
	struct in_addr src;
	char ssrc[40];
	MIB_CE_ACL_IP_T Entry;

#ifdef ACL_IP_RANGE
	struct in_addr start,end;
	char sstart[16],send[16];
#endif

#ifdef MAC_ACL
	MIB_CE_ACL_MAC_T MacEntry;
	char macaddr[18];
#endif


	// Added by Mason Yu for ACL.
	// check if ACL Capability is enabled ?
	if (!enable)
		va_cmd(IPTABLES, 2, 1, "-F", "aclblock");
	else {
		// Add policy to aclblock chain
		total = mib_chain_total(MIB_ACL_IP_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_ACL_IP_TBL, i, (void *)&Entry))
			{
				return;
			}

//    		printf("\nstartip=%x,endip=%x\n",*(unsigned long*)Entry.startipAddr,*(unsigned long*)Entry.endipAddr);

			// Check if this entry is enabled
			if ( Entry.Enabled == 1 ) {
#ifdef ACL_IP_RANGE
				start.s_addr = *(unsigned long*)Entry.startipAddr;
				end.s_addr = *(unsigned long*)Entry.endipAddr;
				strcpy(sstart, inet_ntoa(start));
				strcpy(send, inet_ntoa(end));
				strcpy(ssrc,sstart);
				strcat(ssrc,"-");
				strcat(ssrc,send);

#else
				src.s_addr = *(unsigned long *)Entry.ipAddr;

				// inet_ntoa is not reentrant, we have to
				// copy the static memory before reuse it
				strcpy(ssrc, inet_ntoa(src));
				snprintf(ssrc, 20, "%s/%d", ssrc, Entry.maskbit);

#endif
		        	if ( Entry.Interface == IF_DOMAIN_LAN ) {

#ifdef ACL_IP_RANGE
					if(*(unsigned long*)Entry.startipAddr != *(unsigned long*)Entry.endipAddr)
					// iptables -A aclblock -m iprange --src-range x.x.x.x-1x.x.x.x -j RETURN
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "aclblock", "-i", BRIF, "-m", "iprange", "--src-range", ssrc, "-j", (char *)FW_RETURN);
					else
						va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, "aclblock", "-i", BRIF, "-s", sstart, "-j", (char *)FW_RETURN);
#else
					// iptables -A INPUT -s xxx.xxx.xxx.xxx
					va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, "aclblock", "-i", BRIF, "-s", ssrc, "-j", (char *)FW_RETURN);
#endif
				} else {
					// iptables -A INPUT -s xxx.xxx.xxx.xxx
					va_cmd(IPTABLES, 9, 1, (char *)FW_ADD, "aclblock", "!", "-i", BRIF, "-s", ssrc, "-j", (char *)FW_RETURN);
				}
			}
		}
#ifdef MAC_ACL
		total = mib_chain_total(MIB_ACL_MAC_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_ACL_MAC_TBL, i, (void *)&MacEntry))
			{
				return;
			}

			// Check if this entry is enabled
			if ( MacEntry.Enabled == 1 ) {
				snprintf(macaddr, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
					MacEntry.macAddr[0], MacEntry.macAddr[1],
					MacEntry.macAddr[2], MacEntry.macAddr[3],
					MacEntry.macAddr[4], MacEntry.macAddr[5]);

		        if ( MacEntry.Interface == IF_DOMAIN_LAN ) {
		 			// iptables -A aclblock -i br0  -m mac --mac-source $MAC -j RETURN
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "aclblock",
							(char *)ARG_I, BRIF, "-m", "mac",
							"--mac-source",  macaddr, "-j", (char *)FW_RETURN);
				} else {
					// iptables -A aclblock -i ! br0  -m mac --mac-source $MAC -j RETURN
					va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, "aclblock",
							 "!", (char *)ARG_I, BRIF, "-m", "mac",
							"--mac-source",  macaddr, "-j", (char *)FW_RETURN);
				}
			}
		}
#endif

		// allowing DNS request during ACL enabled
		// iptables -A aclblock -p udp -i br0 --dport 53 -j RETURN
		va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "aclblock", "-p", "udp", "-i", BRIF,(char*)FW_DPORT, (char *)PORT_DNS, "-j", (char *)FW_RETURN);
		// allowing DHCP request during ACL enabled
		// iptables -A aclblock -p udp -i br0 --dport 67 -j RETURN
		va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "aclblock", "-p", "udp", "-i", BRIF, (char*)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_RETURN);

		// iptables -A aclblock -i lo -j RETURN
		// Local Out DNS query will refer the /etc/resolv.conf(DNS server is 127.0.0.1). We should accept this DNS query when enable ACL.
		va_cmd(IPTABLES, 6, 1, (char *)FW_ADD, "aclblock", "-i", "lo", "-j", (char *)FW_RETURN);
		// iptables -A aclblock -j DROP
		va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, "aclblock", "-j", (char *)FW_DROP);
	}
}
#endif

#ifdef NAT_CONN_LIMIT
//ql_xu add
int restart_connlimit(void)
{
	unsigned char connlimitEn;

	va_cmd(IPTABLES, 2, 1, "-F", "connlimit");

	mib_get(MIB_NAT_CONN_LIMIT, (void *)&connlimitEn);
	if (connlimitEn == 1)
		set_conn_limit();
}

void set_conn_limit(void)
{
	int i, total;
	char ssrc[20];
	char connNum[10];
	MIB_CE_CONN_LIMIT_T Entry;

	// Add policy to connlimit chain
	//iptables -A connlimit -m state --state RELATED,ESTABLISHED -j RETURN
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, "connlimit", "-m", "state",
		"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_RETURN);

	total = mib_chain_total(MIB_CONN_LIMIT_TBL);
	for (i=0; i<total; i++) {
		if (!mib_chain_get(MIB_CONN_LIMIT_TBL, i, (void *)&Entry))
			continue;

		// Check if this entry is enabled
		if ( Entry.Enabled == 1 ) {
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strncpy(ssrc, inet_ntoa(*((struct in_addr *)&Entry.ipAddr)), 16);
			ssrc[15] = '\0';

			snprintf(connNum, 10, "%d", Entry.connNum);

			// iptables -A connlimit -i br0 -s xxx.xxx.xxx.xxx
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "connlimit", "-s", ssrc, "-m", "iplimit",
				"--iplimit-above", connNum, "--iplimit-mask", "255.255.255.255", "-j", "REJECT");
		}
	}
}
#endif
#ifdef TCP_UDP_CONN_LIMIT
int restart_connlimit(void)
{
	unsigned char connlimitEn;

	va_cmd(IPTABLES, 2, 1, "-F", "connlimit");

	mib_get(MIB_CONNLIMIT_ENABLE, (void *)&connlimitEn);
	if (connlimitEn == 1)
		set_conn_limit();
}

void set_conn_limit(void)
{
	int 					i, total;
	char 				ssrc[20];
	char 				connNum[10];
	MIB_CE_TCP_UDP_CONN_LIMIT_T Entry;

	// Add policy to connlimit chain, allow all established ~
	//iptables -A connlimit -m state --state RELATED,ESTABLISHED -j RETURN
	va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, "connlimit", "-m", "state",
		"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_RETURN);

	total = mib_chain_total(MIB_TCP_UDP_CONN_LIMIT_TBL);
	for (i=0; i<total; i++) {
		if (!mib_chain_get(MIB_TCP_UDP_CONN_LIMIT_TBL, i, (void *)&Entry))
			continue;
		// Check if this entry is enabled and protocol is TCP
		if (( Entry.Enabled == 1 ) && ( Entry.protocol == 0 )) {
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strncpy(ssrc, inet_ntoa(*((struct in_addr *)&Entry.ipAddr)), 16);
			ssrc[15] = '\0';
			snprintf(connNum, 10, "%d", Entry.connNum);

			// iptables -A connlimit -p tcp -s 192.168.1.99 -m iplimit --iplimit-above 2 -j REJECT
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "connlimit","-p","tcp", "-s", ssrc, "-m", "iplimit",
				"--iplimit-above", connNum, "-j", "REJECT");
		}
		// UDP
		else if  (( Entry.Enabled == 1 ) && ( Entry.protocol == 1 )) {
			strncpy(ssrc, inet_ntoa(*((struct in_addr *)&Entry.ipAddr)), 16);
			ssrc[15] = '\0';
			snprintf(connNum, 10, "%d", Entry.connNum);

			//iptables -A connlimit -p udp -s 192.168.1.99 -m udplimit --udplimit-above 2 -j REJECT
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "connlimit","-p","udp", "-s", ssrc, "-m", "udplimit",
				"--udplimit-above", connNum, "-j", "REJECT");
		}

	}

	//The Global rules goes last ~
	//TCP
	mib_get(MIB_CONNLIMIT_TCP, (void *)&i);
	if (i >0)
	{
		snprintf(connNum, 10, "%d", i);
		va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "connlimit","-p","tcp",  "-m", "iplimit",
					"--iplimit-above", connNum, "-j", "REJECT");
	}
	//UDP
	mib_get(MIB_CONNLIMIT_UDP, (void *)&i);
	if (i >0)
	{
		snprintf(connNum, 10, "%d", i);
		va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "connlimit","-p","udp",  "-m", "udplimit",
					"--udplimit-above", connNum, "-j", "REJECT");
	}

}
#endif//TCP_UDP_CONN_LIMIT

#ifdef URL_BLOCKING_SUPPORT
void filter_set_url(int mode)
{
	int i, total;
	MIB_CE_URL_FQDN_T entry;
	char portStr[6];

	//Kevin, check does it need to enable/disable IP fastpath status
	UpdateIpFastpathStatus();

	va_cmd(IPTABLES, 2, 1, "-F", "urlblock");
	va_cmd(IPTABLES, 2, 1, "-F", "urlallow");

#ifdef CONFIG_RTK_RG_INIT
	Flush_RTK_RG_URL_Filter();
#endif

#ifdef CONFIG_RTK_RG_INIT
	RTK_RG_URL_Filter_Set_By_Key(mode);
#endif

	if(1 == mode) {//blacklist
		printf("urlblock\n");
		// Add URL policy to urlblock chain
		total = mib_chain_total(MIB_URL_FQDN_TBL);

		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_URL_FQDN_TBL, i, (void *)&entry))
				return;

			snprintf(portStr, 6, "%d", entry.port);
			printf("port:%s key:%s", portStr, entry.key);
		 	// iptables -A urlblock -p tcp --dport 80 -m string --url "tw.yahoo.com" -j DROP
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "urlblock", "-p", "tcp", "--dport",
				portStr, "-m", "string", "--url", entry.key, "-j", (char *)FW_DROP);
		}
	}
	else if (2 == mode) {//whitelist
		printf("urlallow\n");
		// Add URL policy to urlblock chain
		total = mib_chain_total(MIB_URL_FQDN_TBL);

		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_URL_FQDN_TBL, i, (void *)&entry))
				return;

			snprintf(portStr, 6, "%d", entry.port);
			// iptables -A urlallow -p tcp --dport 80 -m string --url "tw.yahoo.com" -j ACCEPT
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "urlallow", "-p", "tcp", "--dport",
				portStr, "-m", "string", "--urlalw", entry.key, "-j", (char *)FW_ACCEPT);
		}
		//&endofurl& is flag for drop other urls
		va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "urlallow", "-p", "tcp", "-m", "string",
			"--urlalw", "endofurl" ,"-j", (char *)FW_DROP);
	}
}

int restart_urlblocking(void)
{
	unsigned char urlEnable;

	va_cmd(IPTABLES, 2, 1, "-F", "urlblock");

#ifdef CONFIG_RTK_RG_INIT
	Flush_RTK_RG_URL_Filter();
#endif

	mib_get(MIB_URL_CAPABILITY, (void *)&urlEnable);
	filter_set_url(urlEnable);

	return 0;
}
#ifdef URL_ALLOWING_SUPPORT
void set_url(int enable)
{
	int i, j, total, totalKeywd;
	MIB_CE_URL_FQDN_T Entry;

	// check if URL Capability is enabled ?

	if (!enable)

		va_cmd(IPTABLES, 2, 1, "-F", "urlallow");

	else {

		// Add URL policy to urlblock chain
		total = mib_chain_total(MIB_URL_ALLOW_FQDN_TBL);

		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_URL_ALLOW_FQDN_TBL, i, (void *)&Entry))
			{
				return;
			}

		 	// iptables -A urlallow -p tcp --dport 80 -m string --url "tw.yahoo.com" -j ACCEPT
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "urlallow", "-p", "tcp", "--dport", "80", "-m", "string", "--urlalw", Entry.fqdn, "-j", (char *)FW_ACCEPT);
		}
	//&endofurl& is flag for drop other urls
              va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "urlallow", "-p", "tcp", "--dport", "80", "-m", "string", "--urlalw", "&endofurl&" ,"-j", (char *)FW_DROP);
	}
}

int restart_url(void)
{
	unsigned char urlEnable;

	va_cmd(IPTABLES, 2, 1, "-F", "urlallow");
	va_cmd(IPTABLES, 2, 1, "-F", "urlblock");
	mib_get(MIB_URL_CAPABILITY, (void *)&urlEnable);
	if (urlEnable == 1)  // URL Capability enabled
		filter_set_url(1);
	else if(urlEnable == 2) //URL allow
		set_url(2);

	return 0;
}
#endif
#endif

#ifdef DOMAIN_BLOCKING_SUPPORT
// Added by Mason Yu for Domain Blocking
void filter_set_domain(int enable)
{
	int i, total;
	MIB_CE_DOMAIN_BLOCKING_T Entry;
	unsigned char sdest[MAX_DOMAIN_LENGTH];
	int j, k;
    	unsigned char *needle_tmp, *str;
    	char len[MAX_DOMAIN_GROUP];
    	unsigned char seg[MAX_DOMAIN_GROUP][MAX_DOMAIN_SUB_STRING];
    	unsigned char cmpStr[MAX_DOMAIN_LENGTH]="\0";

	//Kevin, check does it need to enable/disable IP fastpath status
    UpdateIpFastpathStatus();

	// check if Domain Blocking Capability is enabled ?
	if (!enable)
		va_cmd(IPTABLES, 2, 1, "-F", "domainblk");
	else {
		// Add policy to domainblk chain
		total = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&Entry))
			{
				return;
			}

			// Mason Yu
			// calculate domain sub string lengh and form the compare sub string.
			// Foe example, If the domain is aa.bbb.cccc, the compare sub string 1 is 0x02 0x61 0x61.
			// The compare sub string 2 is 0x03 0x62 0x62 0x62. The compare sub string 3 is 0x04 0x63 0x63 0x63 0x63.
			needle_tmp = Entry.domain;

			for (j=0; (str =strchr(needle_tmp, '.'))!= NULL; j++)
    			{
    				*str = '\0';

    				strncpy(seg[j]+1, needle_tmp, (MAX_DOMAIN_SUB_STRING - 1));
    				seg[j][MAX_DOMAIN_SUB_STRING - 1]='\0';
    				//printf(" seg[%d]= %s...(1)\n", j, seg[j]);

    				//seg[j][0]= len[j];
    				seg[j][0] = strlen(needle_tmp);
    				//printf(" seg[%d]= %s...(2)\n", j, seg[j]);

    				needle_tmp = str+1;
   			}

    			// calculate the laster domain sub string lengh and form the laster compare sub string
    			strncpy(seg[j]+1, needle_tmp, (MAX_DOMAIN_SUB_STRING - 1));
    			seg[j][MAX_DOMAIN_SUB_STRING - 1]='\0';

    			seg[j][0]= strlen(needle_tmp);
    			//printf(" seg[%d]= %s...(3)\n", j, seg[j]);

    			// Merge the all compare sub string into a final compare string.
    			for ( k=0; k<=j; k++) {
    				//printf(" seg[%d]= %s", k, seg[k]);
    				strcat(cmpStr, seg[k]);
    				//printf(" cmpStr=%s\n", cmpStr);
    			}
    			//printf("\n");

		 	// iptables -A domainblk -p udp --dport 53 -m string --domain yahoo.com -j DROP
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, "domainblk", "-p", "udp", "--dport", "53", "-m", "string", "--domain", cmpStr, "-j", (char *)FW_DROP);
			cmpStr[0] = '\0';
		}

	}
}

int restart_domainBLK(void)
{
	unsigned char domainEnable;

	va_cmd(IPTABLES, 2, 1, "-F", "domainblk");
	mib_get(MIB_DOMAINBLK_CAPABILITY, (void *)&domainEnable);
	if (domainEnable == 1)  // domain blocking Capability enabled
		filter_set_domain(1);
	else
		filter_set_domain(0);
	return 0;
}
#endif

#if defined(CONFIG_USER_ROUTED_ROUTED) || defined(CONFIG_USER_ZEBRA_OSPFD_OSPFD)
static const char ROUTED_CONF[] = "/var/run/routed.conf";

int startRip(void)
{
	FILE *fp;
	unsigned char ripOn, ripInf, ripVer;
	int rip_pid;

	unsigned int entryNum, i;
	MIB_CE_RIP_T Entry;
	char ifname[IFNAMSIZ], receive_mode[5], send_mode[5];

	if (mib_get(MIB_RIP_ENABLE, (void *)&ripOn) == 0)
		ripOn = 0;
	rip_pid = read_pid((char *)ROUTED_PID);
	if (rip_pid >= 1) {
		// kill it
		if (kill(rip_pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'", rip_pid);
		}
	}

	if (!ripOn)
		return 0;

	printf("start rip!\n");
	if ((fp = fopen(ROUTED_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", ROUTED_CONF);
		return -1;
	}

#if 0
	if (mib_get(MIB_RIP_VERSION, (void *)&ripVer) == 0)
		ripVer = 1;	// default version 2

	fprintf(fp, "version %u\n", ripVer+1);

	// LAN interface
	if (mib_get(MIB_ADSL_LAN_RIP, (void *)&ripOn) != 0)
	{
		if (ripOn)
			fprintf(fp, "network br0\n");
	}

	// WAN interface
	vcTotal = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0; i<vcTotal; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;

		if (Entry.enable == 0)
			continue;

		if (Entry.cmode != CHANNEL_MODE_BRIDGE && Entry.rip)
		{
			if (PPP_INDEX(Entry.ifIndex) != 0x0f)
			{	// PPP interface
				snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
			}
			else
			{	// vc interface
				snprintf(ifname, 6, "vc%u", VC_INDEX(Entry.ifIndex));
			}

			fprintf(fp, "network %s\n", ifname);
		}
	}
#endif

	entryNum = mib_chain_total(MIB_RIP_TBL);

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry))
		{
  			printf("Get MIB_RIP_TBL chain record error!\n");
			return;
		}

		if (!ifGetName(Entry.ifIndex, ifname, sizeof(ifname))) {
			strncpy(ifname, BRIF, strlen(BRIF));
			ifname[strlen(BRIF)] = '\0';
		}

		fprintf(fp, "network %s %d %d\n", ifname, Entry.receiveMode, Entry.sendMode);
	}

	fclose(fp);
	// Modified by Mason Yu for always as a supplier for RIP
	//va_cmd(ROUTED, 0, 0);
	va_cmd(ROUTED, 1, 0, "-s");
	return 1;
}
#endif	// of CONFIG_USER_ROUTED_ROUTED

#ifdef CONFIG_USER_ZEBRA_OSPFD_OSPFD
static const char ZEBRA_CONF[] = "/etc/config/zebra.conf";
static const char OSPFD_CONF[] = "/etc/config/ospfd.conf";

// OSPF server configuration
// return value:
// 0  : not enabled
// 1  : successful
// -1 : startup failed
int startOspf(void)
{
	FILE *fp;
	char *argv[6];
	int pid;
	unsigned char ospfOn;
	unsigned int entryNum, i, j;
	MIB_CE_OSPF_T Entry;
	char *netIp[20];
	unsigned int uMask;
	unsigned int uIp;

	if (mib_get(MIB_OSPF_ENABLE, (void *)&ospfOn) == 0)
		ospfOn = 0;
	//kill old zebra
	pid = read_pid((char *)ZEBRA_PID);
	if (pid >= 1) {
		if (kill(pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'\n", pid);
		}
	}
	//kill old ospfd
	pid = read_pid((char *)OSPFD_PID);
	if (pid >= 1) {
		if (kill(pid, SIGTERM) != 0)
			printf("Could not kill pid '%d'\n", pid);
	}

	if (!ospfOn)
		return 0;

	printf("start ospf!\n");
	//create zebra.conf
	if ((fp = fopen(ZEBRA_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", ZEBRA_CONF);
		return -1;
	}
	fprintf(fp, "hostname Router\n");
	fprintf(fp, "password zebra\n");
	fprintf(fp, "enable password zebra\n");
	fclose(fp);

	//create ospfd.conf
	if ((fp = fopen(OSPFD_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", OSPFD_CONF);
		return -1;
	}
	fprintf(fp, "hostname ospfd\n");
	fprintf(fp, "password zebra\n");
	fprintf(fp, "enable password zebra\n");
	fprintf(fp, "router ospf\n");
	//ql_xu test.
	//fprintf(fp, "network %s area 0\n", "192.168.2.0/24");
	entryNum = mib_chain_total(MIB_OSPF_TBL);

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_OSPF_TBL, i, (void *)&Entry))
		{
  			printf("Get MIB_OSPF_TBL chain record error!\n");
			return -1;
		}

		uIp = *(unsigned int *)Entry.ipAddr;
		uMask = *(unsigned int *)Entry.netMask;
		uIp = uIp & uMask;
		strcpy(netIp, inet_ntoa(*((struct in_addr *)&uIp)));

		for (j=0; j<32; j++)
			if ((uMask>>j) & 0x01)
				break;
		uMask = 32 - j;

		sprintf(netIp, "%s/%d", netIp, uMask);
		fprintf(fp, "network %s area 0\n", netIp);
	}
	fclose(fp);

	//start zebra
	argv[1] = "-d";
	argv[2] = "-k";
	argv[3] = NULL;
	TRACE(STA_SCRIPT, "%s %s %s\n", ZEBRA, argv[1], argv[2]);
	do_cmd(ZEBRA, argv, 0);

	//start ospfd
	argv[1] = "-d";
	argv[2] = NULL;
	TRACE(STA_SCRIPT, "%s %s\n", OSPFD, argv[1]);
	do_cmd(OSPFD, argv, 0);

	return 1;
}
#endif

#include <linux/atm.h>

//up_flag:0, itf down; non-zero: itf up
void itfcfg(char *if_name, int up_flag)
{
#ifdef EMBED
	int fd;
	struct ifreq ifr;
	char cmd;

	if (strncmp(if_name, "sar", sizeof("sar"))==0) {  //sar enable/disable
#ifdef CONFIG_RTL8672_SAR
		struct atmif_sioc mysio;

		if((fd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
			printf("socket open error\n");
			return;
		}
		mysio.number = 0;
		mysio.length = sizeof(struct SAR_IOCTL_CFG);
		mysio.arg = (void *)NULL;
		if (up_flag) ioctl(fd, SIOCDEVPRIVATE+2, &mysio);
		else         ioctl(fd, SIOCDEVPRIVATE+3, &mysio);
		if(fd!=(int)NULL)
		    close(fd);
#else
		printf( "%s: SAR interface not supportted.\n", __FUNCTION__ );
#endif
	}
	else if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
		int flags;
		if (getInFlags(if_name, &flags) == 1) {
			if (up_flag)
				flags |= IFF_UP;
			else
				flags &= ~IFF_UP;

			setInFlags(if_name, flags);
		}
		close(fd);
	}
#endif
}

#ifdef XOR_ENCRYPT
const char XOR_KEY[] = "tecomtec";
void xor_encrypt(char *inputfile, char *outputfile)
{
	FILE *input  = fopen(inputfile, "rb");
	FILE *output = fopen(outputfile, "wb");

	if(input != NULL && output != NULL) {
		unsigned char buffer[MAX_CONFIG_FILESIZE];
		size_t count, i, j = 0;
		do {
			count = fread(buffer, sizeof *buffer, sizeof buffer, input);
			for(i = 0; i<count; ++i) {
				buffer[i] ^= XOR_KEY[j++];
				if(XOR_KEY[j] == '\0')
					j = 0; /* restart at the beginning of the key */
			}
			fwrite(buffer, sizeof *buffer, count, output);
		} while (count == sizeof buffer);
		fclose(input);
		fclose(output);
	}
}
#endif

unsigned short
ipchksum(unsigned char *ptr, int count, unsigned short resid)
{
	register unsigned int sum = resid;
       if ( count==0)
       	return(sum);

	while(count > 1) {
		//sum += ntohs(*ptr);
		sum += (( ptr[0] << 8) | ptr[1] );
		if ( sum>>31)
			sum = (sum&0xffff) + ((sum>>16)&0xffff);
		ptr += 2;
		count -= 2;
	}

	if (count > 0)
		sum += (*((unsigned char*)ptr) << 8) & 0xff00;

	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);

	if (sum == 0xffff)
		sum = 0;
	return (unsigned short)sum;
}

int getLinkStatus(struct ifreq *ifr)
{
	int status=0;

#ifdef __mips__
	if (do_ioctl(SIOCGMEDIALS, ifr) == 1)
		status = 1;
#endif
	return status;
}

/***port forwarding APIs*******/
#ifdef PORT_FORWARD_GENERAL
/*move from startup.c:iptable_fw()*/
static void iptable_fw( int del, int negate, const char *ifname, const char *proto, const char *remoteIP, const char *extPort, const char *dstIP)
{
	char *act;

	if(del) act = (char *)FW_DEL;
	else act = (char *)FW_ADD;


//	if (negate && remoteIP) {
	if (negate && remoteIP && extPort) {
		va_cmd(IPTABLES, 17, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			 "!", (char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			"-s", (char *)remoteIP,
			(char *)FW_DPORT, extPort, "-j",
			"DNAT", "--to-destination", dstIP);
	} else if (negate && remoteIP) {
		va_cmd(IPTABLES, 15, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			 "!", (char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			"-s", (char *)remoteIP,
			"-j", "DNAT", "--to-destination", dstIP);
//	} else if (negate) {
	} else if (negate && extPort) {
		va_cmd(IPTABLES, 15, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			 "!", (char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			(char *)FW_DPORT, extPort, "-j",
			"DNAT", "--to-destination", dstIP);
//	} else if (remoteIP) {
	} else if (remoteIP && extPort) {
		va_cmd(IPTABLES, 16, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			"-s", (char *)remoteIP,
			(char *)FW_DPORT, extPort, "-j",
			"DNAT", "--to-destination", dstIP);
	} else if (negate) {
		va_cmd(IPTABLES, 13, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			 "!", (char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			"-j", "DNAT", "--to-destination", dstIP);
	} else if (remoteIP) {
		va_cmd(IPTABLES, 14, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			"-s", (char *)remoteIP,
			"-j", "DNAT", "--to-destination", dstIP);
//	} else {
	} else if (extPort) {
		va_cmd(IPTABLES, 14, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			(char *)FW_DPORT, extPort, "-j",
			"DNAT", "--to-destination", dstIP);
	} else {
		va_cmd(IPTABLES, 12, 1, "-t", "nat",
			(char *)act,	(char *)PORT_FW,
			(char *)ARG_I, (char *)ifname,
			"-p", (char *)proto,
			 "-j", "DNAT", "--to-destination", dstIP);
	}

}

/*move from startup.c:iptable_filter()*/
static void iptable_filter( int del, int negate, const char *ifname, const char *proto, const char *remoteIP, const char *intPort)
{
	char *strInsert="-I";
	char *strThird="3";
	char *act, *rulenum;

	if(del)
	{
		act = (char *)FW_DEL;

//		if (negate && remoteIP) {
		if (negate && remoteIP && intPort) {
			va_cmd(IPTABLES, 15, 1, act,
				(char *)PORT_FW, "!", (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-s", remoteIP,
				(char *)FW_DPORT, intPort,
				"-j",(char *)FW_ACCEPT);
		} else if (negate && remoteIP) {
			va_cmd(IPTABLES, 13, 1, act,
				(char *)PORT_FW, "!", (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-s", remoteIP,
				"-j",(char *)FW_ACCEPT);
//		} else if (negate) {
		} else if (negate && intPort) {
			va_cmd(IPTABLES, 13, 1, act,
				(char *)PORT_FW, "!", (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				(char *)FW_DPORT, intPort,
				"-j",(char *)FW_ACCEPT);
//		} else if (remoteIP) {
		} else if (remoteIP && intPort) {
			va_cmd(IPTABLES, 14, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-s", remoteIP,
				(char *)FW_DPORT, intPort,
				"-j",(char *)FW_ACCEPT);
		} else if (negate) {
			va_cmd(IPTABLES, 11, 1, act,
				(char *)PORT_FW, "!", (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-j",(char *)FW_ACCEPT);
		} else if (remoteIP) {
			va_cmd(IPTABLES, 12, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-s", remoteIP,
				"-j",(char *)FW_ACCEPT);
//		} else {
		} else if (intPort) {
			va_cmd(IPTABLES, 12, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				(char *)FW_DPORT, intPort, "-j",
				(char *)FW_ACCEPT);
		} else {
			va_cmd(IPTABLES, 10, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-j", (char *)FW_ACCEPT);
		}
	}else
	{
		//act = strInsert; //use insert, not append => need rulenum
		//rulenum = strThird;
 		act = (char *)FW_ADD;

//		if (negate && remoteIP) {
		if (negate && remoteIP && intPort) {
			va_cmd(IPTABLES, 15, 1, act,
				(char *)PORT_FW, "!", (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-s", remoteIP,
				(char *)FW_DPORT, intPort,
				"-j",(char *)FW_ACCEPT);
		} else if (negate && remoteIP) {
			va_cmd(IPTABLES, 13, 1, act,
				(char *)PORT_FW, "!", (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-s", remoteIP,
				"-j",(char *)FW_ACCEPT);
//		} else if (negate) {
		} else if (negate && intPort) {
			va_cmd(IPTABLES, 13, 1, act,
				(char *)PORT_FW, "!", (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				(char *)FW_DPORT, intPort,
				"-j",(char *)FW_ACCEPT);
//		} else if (remoteIP) {
		} else if (remoteIP && intPort) {
			va_cmd(IPTABLES, 14, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-s", remoteIP,
				(char *)FW_DPORT, intPort,
				"-j",(char *)FW_ACCEPT);
		} else if (negate) {
			va_cmd(IPTABLES, 11, 1, act,
				(char *)PORT_FW, "!", (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-j",(char *)FW_ACCEPT);
		} else if (remoteIP) {
			va_cmd(IPTABLES, 12, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-s", remoteIP,
				"-j",(char *)FW_ACCEPT);
//		} else {
		} else if (intPort) {
			va_cmd(IPTABLES, 12, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				(char *)FW_DPORT, intPort, "-j",
				(char *)FW_ACCEPT);
		} else {
			va_cmd(IPTABLES, 10, 1, act,
				(char *)PORT_FW, (char *)ARG_I, ifname,
				(char *)ARG_O,
				(char *)LANIF, "-p", (char *)proto,
				"-j", (char *)FW_ACCEPT);
		}
	}
}

/*move from startup.c ==> part of setupFirewall()*/
void portfw_modify( MIB_CE_PORT_FW_T *p, int del )
{
	int negate=0, hasRemote=0, hasLocalPort=0, hasExtPort=0;
	char * proto = 0;
	char intPort[32], extPort[32];
	char ipaddr[32], extra[32], ifname[IFNAMSIZ];

	if(p==NULL) return;

#if 0
	{
		fprintf( stderr,"<portfw_modify>\n" );
		fprintf( stderr,"\taction:%s\n", (del==0)?"ADD":"DEL" );
		fprintf( stderr,"\tifIndex:0x%x\n", p->ifIndex );
		fprintf( stderr,"\tenable:%u\n", p->enable );
		fprintf( stderr,"\tleaseduration:%u\n", p->leaseduration );
		fprintf( stderr,"\tremotehost:%s\n", inet_ntoa(*((struct in_addr *)p->remotehost))  );
		fprintf( stderr,"\texternalport:%u\n", p->externalport );
		fprintf( stderr,"\tinternalclient:%s\n", inet_ntoa(*((struct in_addr *)p->ipAddr)) );
		fprintf( stderr,"\tinternalport:%u\n", p->toPort );
		fprintf( stderr,"\tprotocol:%u\n", p->protoType ); /*PROTO_TCP=1, PROTO_UDP=2*/
		fprintf( stderr,"<end portfw_modify>\n" );
	}
#endif

	if( del==0 ) //add
	{
		char vCh=0;
		mib_get(MIB_PORT_FW_ENABLE, (void *)&vCh);
		if(vCh==0) return;

		if (!p->enable) return;
	}

/*	snprintf(intPort, 12, "%u", p->fromPort);

	if (p->externalport) {
		snprintf(extPort, sizeof(extPort), "%u", p->externalport);
		snprintf(intPort, sizeof(intPort), "%u", p->fromPort);
		snprintf(ipaddr, sizeof(ipaddr), "%s:%s", inet_ntoa(*((struct in_addr *)p->ipAddr)), intPort);
	} else {
		snprintf(intPort, sizeof(extPort), "%u", p->fromPort);
		strncpy(extPort, intPort, sizeof(intPort));
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)p->ipAddr)), 16);
	}
	*/
    if (p->fromPort)
    {
        if (p->fromPort == p->toPort)
        {
            snprintf(intPort, sizeof(intPort), "%u", p->fromPort);
        }
        else
        {
            /* "%u-%u" is used by port forwarding */
            snprintf(intPort, sizeof(intPort), "%u-%u", p->fromPort, p->toPort);
        }

        snprintf(ipaddr, sizeof(ipaddr), "%s:%s", inet_ntoa(*((struct in_addr *)p->ipAddr)), intPort);

        if (p->fromPort != p->toPort)
        {
            /* "%u:%u" is used by filter */
            snprintf(intPort, sizeof(intPort), "%u:%u", p->fromPort, p->toPort);
        }
        hasLocalPort = 1;
    }
    else
    {
        snprintf(ipaddr, sizeof(ipaddr), "%s", inet_ntoa(*((struct in_addr *)p->ipAddr)));
        hasLocalPort = 0;
    }

	if (p->externalfromport && p->externaltoport && (p->externalfromport != p->externaltoport)) {
		snprintf(extPort, sizeof(extPort), "%u:%u", p->externalfromport, p->externaltoport);
		hasExtPort = 1;
	} else if (p->externalfromport) {
		snprintf(extPort, sizeof(extPort), "%u", p->externalfromport);
		hasExtPort = 1;
	} else if (p->externaltoport) {
		snprintf(extPort, sizeof(extPort), "%u", p->externaltoport);
		hasExtPort = 1;
	} else {
		hasExtPort = 0;
	}
	//printf( "extPort:%s hasExtPort=%d\n",  extPort, hasExtPort);
	//printf( "entry.externalfromport:%d entry.externaltoport=%d\n",  p->externalfromport, p->externaltoport);

	if (ifGetName(p->ifIndex, ifname, sizeof(ifname)))
		negate = 0;
	else {
		strcpy(ifname, LANIF);
		negate = 1;
	}

	if (p->remotehost[0]) {
		snprintf(extra, sizeof(extra), "%s", inet_ntoa(*((struct in_addr *)p->remotehost)));
		hasRemote = 1;
	} else {
		hasRemote = 0;
	}

	//fprintf( stderr, "ipaddr:%s, intPort:%s\n", ipaddr, intPort );
	//check something
	//internalclient can't be zeroip
	if( strncmp(ipaddr,"0.0.0.0", 7)==0 ) return;
	//internalport can't be zero
//	if( strcmp(intPort,"0")==0 ) return;
	//fprintf( stderr, "Pass ipaddr:%s, intPort:%s\n", ipaddr, intPort );

	if (p->protoType == PROTO_TCP || p->protoType == PROTO_UDPTCP)
	{
		// iptables -t nat -A PREROUTING -i ! $LAN_IF -p TCP --dport dstPortRange -j DNAT --to-destination ipaddr
//		iptable_fw( del, negate, ifname, ARG_TCP, hasRemote ? extra : 0, extPort, ipaddr);
		iptable_fw( del, negate, ifname, ARG_TCP, hasRemote ? extra : 0, hasExtPort ? extPort : 0, ipaddr);
		// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -p TCP --dport dstPortRange -j RETURN
//		iptable_filter( del, negate, ifname, ARG_TCP, hasRemote ? extra : 0, intPort);
		iptable_filter( del, negate, ifname, ARG_TCP, hasRemote ? extra : 0, hasLocalPort ? intPort : 0);
	}

	if (p->protoType == PROTO_UDP || p->protoType == PROTO_UDPTCP)
	{
//		iptable_fw( del, negate, ifname, ARG_UDP, hasRemote ? extra : 0, extPort, ipaddr);
		iptable_fw( del, negate, ifname, ARG_UDP, hasRemote ? extra : 0, hasExtPort ? extPort : 0, ipaddr);
		// iptables -I ipfilter 3 -i ! $LAN_IF -o $LAN_IF -p UDP --dport dstPortRange -j RETURN
//		iptable_filter( del, negate, ifname, ARG_UDP, hasRemote ? extra : 0, intPort);
		iptable_filter( del, negate, ifname, ARG_UDP, hasRemote ? extra : 0, hasLocalPort ? intPort : 0);
	}
}
#endif // of PORT_FORWARD_GENERAL

/***end port forwarding APIs*******/

#ifdef SUPPORT_DHCP_RESERVED_IPADDR
static const char DHCPReservedIPAddrFile[] = "/var/udhcpd/DHCPReservedIPAddr.txt";
static int setupDHCPReservedIPAddr( void )
{
	FILE *fp;

	fp=fopen( DHCPReservedIPAddrFile, "w" );
	if(!fp) return -1;

	{//default pool
		unsigned int j,num;
		num = mib_chain_total( MIB_DHCP_RESERVED_IPADDR_TBL );
		fprintf( fp, "START %u\n", 0 );
		for( j=0;j<num;j++ )
		{
			MIB_DHCP_RESERVED_IPADDR_T entryip;
			if(!mib_chain_get( MIB_DHCP_RESERVED_IPADDR_TBL, j, (void*)&entryip ))
				continue;
			if( entryip.InstanceNum==0 )
			{
				fprintf( fp, "%s\n",  inet_ntoa(*((struct in_addr *)&(entryip.IPAddr))) );
			}
		}
		fprintf( fp, "END\n" );
	}

#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
{
	unsigned int i,numpool;
	numpool = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
	for( i=0; i<numpool;i++ )
	{
		unsigned int j,num;
		DHCPS_SERVING_POOL_T entrypool;

		if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)&entrypool ) )
			continue;

		//skip disable or relay pools
		if( entrypool.enable==0 || entrypool.localserved==0 )
			continue;

		num = mib_chain_total( MIB_DHCP_RESERVED_IPADDR_TBL );
		fprintf( fp, "START %u\n", entrypool.InstanceNum );
		for( j=0;j<num;j++ )
		{
			MIB_DHCP_RESERVED_IPADDR_T entryip;
			if(!mib_chain_get( MIB_DHCP_RESERVED_IPADDR_TBL, j, (void*)&entryip ))
				continue;
			if( entryip.InstanceNum==entrypool.InstanceNum )
			{
				fprintf( fp, "%s\n",  inet_ntoa(*((struct in_addr *)&(entryip.IPAddr))) );
			}
		}
		fprintf( fp, "END\n" );
	}
}
#endif //_PRMT_X_TELEFONICA_ES_DHCPOPTION_

	fclose(fp);
	return 0;
}

int clearDHCPReservedIPAddrByInstNum(unsigned int instnum)
{
	int j,num;
	num = mib_chain_total( MIB_DHCP_RESERVED_IPADDR_TBL );
	if( num>0 )
	{
		for( j=num-1;j>=0;j-- )
		{
			MIB_DHCP_RESERVED_IPADDR_T entryip;
			if(!mib_chain_get( MIB_DHCP_RESERVED_IPADDR_TBL, j, (void*)&entryip ))
				continue;
			if( entryip.InstanceNum==instnum )
			{
				mib_chain_delete( MIB_DHCP_RESERVED_IPADDR_TBL, j );
			}
		}
	}
	return 0;
}
#endif //SUPPORT_DHCP_RESERVED_IPADDR



// DHCP server configuration
// return value:
// 0  : not active
// 1  : active
// -1 : setup failed
int setupDhcpd(void)
{
	unsigned char value[32];
	unsigned int uInt, uLTime;
	DNS_TYPE_T dnsMode;
	FILE *fp, *fp2;
	char ipstart[16], ipend[16];
#ifdef IMAGENIO_IPTV_SUPPORT
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
	char opchaddr[16]={0}, stbdns1[16]={0}, stbdns2[16]={0};
	unsigned short opchport;
#endif
/*ping_zhang:20090930 END*/
#endif
	char subnet[16], ipaddr[16], ipaddr2[16];
	char dhcpsubnet[16];
	char dns1[16], dns2[16], dns3[16], dhcps[16];
	char domain[MAX_NAME_LEN];
	unsigned int entryNum, i, j;
	MIB_CE_MAC_BASE_DHCP_T Entry;
#ifdef IP_PASSTHROUGH
	int ippt;
	unsigned int ippt_itf;
#endif
	int spc_enable, spc_ip;
	struct in_addr myip;
	//ql: check if pool is in first IP subnet or in secondary IP subnet.
	int subnetIdx=0;
	struct in_addr lanip1, lanmask1, lanip2, lanmask2, inpoolstart;
	char serverip[16];

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	DHCPS_SERVING_POOL_T dhcppoolentry;
	int dhcppoolnum;
	unsigned char macempty[6]={0,0,0,0,0,0};
	char *ipempty="0.0.0.0";
#endif
	char macaddr[20];

/*ping_zhang:20080919 END*/

	// check if dhcp server on ?
	// Modified by Mason Yu for dhcpmode
	//if (mib_get(MIB_ADSL_LAN_DHCP, (void *)value) != 0)
	if (mib_get(MIB_DHCP_MODE, (void *)value) != 0)
	{
		uInt = (unsigned int)(*(unsigned char *)value);
		if (uInt != DHCP_LAN_SERVER)
		{
			return 0;	// dhcp Server not on
		}
	}

#ifdef CONFIG_SECONDARY_IP
	mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)value);
	if (value[0])
		mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)value);
#else
	value[0] = 0;
#endif

#ifdef DHCPS_POOL_COMPLETE_IP
	mib_get(MIB_DHCP_SUBNET_MASK, (void *)value);
	strncpy(dhcpsubnet, inet_ntoa(*((struct in_addr *)value)), 16);
	dhcpsubnet[15] = '\0';
	// IP Pool start
	mib_get(MIB_DHCP_POOL_START, (void *)value);
	strncpy(ipstart, inet_ntoa(*((struct in_addr *)value)), 16);
	// IP Pool end
	mib_get(MIB_DHCP_POOL_END, (void *)value);
	strncpy(ipend, inet_ntoa(*((struct in_addr *)value)), 16);
#else
	if (value[0] == 0) { // primary LAN
		if (mib_get(MIB_ADSL_LAN_SUBNET, (void *)value) != 0)
		{
			strncpy(dhcpsubnet, inet_ntoa(*((struct in_addr *)value)), 16);
			dhcpsubnet[15] = '\0';
		}
		else
			return -1;

		mib_get(MIB_ADSL_LAN_IP, (void *)value);
	}
	else { // secondary LAN
		if (mib_get(MIB_ADSL_LAN_SUBNET2, (void *)value) != 0)
		{
			strncpy(dhcpsubnet, inet_ntoa(*((struct in_addr *)value)), 16);
			dhcpsubnet[15] = '\0';
		}
		else
			return -1;

		mib_get(MIB_ADSL_LAN_IP2, (void *)value);
	}

	// IP pool start address
	mib_get(MIB_ADSL_LAN_CLIENT_START, (void *)&value[3]);
		strncpy(ipstart, inet_ntoa(*((struct in_addr *)value)), 16);
		ipstart[15] = '\0';
	// IP pool end address
	mib_get(MIB_ADSL_LAN_CLIENT_END, (void *)&value[3]);
		strncpy(ipend, inet_ntoa(*((struct in_addr *)value)), 16);
		ipend[15] = '\0';
	#endif

	// Added by Mason Yu for MAC base assignment. Start
	if ((fp2 = fopen("/var/dhcpdMacBase.txt", "w")) == NULL)
	{
		printf("***** Open file /var/dhcpdMacBase.txt failed !\n");
		goto dhcpConf;
	}

//star: reserve local ip for dhcp server
		MIB_CE_MAC_BASE_DHCP_T entry;

		strcpy(macaddr, "localmac");
		//printf("entry.macAddr = %s\n", entry.macAddr);
		if (mib_get(MIB_ADSL_LAN_IP, (void *)value) != 0)
		{
			//strncpy(entry.ipAddr, inet_ntoa(*((struct in_addr *)value)), 16);
			//entry.ipAddr[15] = '\0';
			fprintf(fp2, "%s: %s\n", macaddr, inet_ntoa(*((struct in_addr *)value)));
		}

#ifdef CONFIG_SECONDARY_IP
		mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)value);
		if (value[0] == 1) {
			strcpy(macaddr, "localsecmac");
			//printf("entry.macAddr = %s\n", entry.macAddr);
			if (mib_get(MIB_ADSL_LAN_IP2, (void *)value) != 0)
			{
				//strncpy(entry.ipAddr, inet_ntoa(*((struct in_addr *)value)), 16);
				//entry.ipAddr[15] = '\0';
				fprintf(fp2, "%s: %s\n", macaddr, inet_ntoa(*((struct in_addr *)value)) );
			}

		}

#endif

//star: check mactbl
	struct in_addr poolstart,poolend,macip;
	unsigned long v1;

	inet_aton(ipstart, &poolstart);
	inet_aton(ipend, &poolend);
check_mactbl:
	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry))
		{
  			printf("setupDhcpd:Get chain(MIB_MAC_BASE_DHCP_TBL) record error!\n");
		}
		//inet_aton(Entry.ipAddr, &macip);
		v1 = *((unsigned long *)Entry.ipAddr_Dhcp);
		if( v1 < poolstart.s_addr || v1 > poolend.s_addr )
		//if(macip.s_addr<poolstart.s_addr||macip.s_addr>poolend.s_addr)
		{
			if(mib_chain_delete(MIB_MAC_BASE_DHCP_TBL, i)!=1)
			{
				printf("setupDhcpd:Delete chain(MIB_MAC_BASE_DHCP_TBL) record error!\n");
				return -1;
			}
			break;
		}
	}
	if((int)i<((int)entryNum-1))
		goto check_mactbl;

	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry))
		{
  			printf("setupDhcpd:Get chain(MIB_MAC_BASE_DHCP_TBL) record error!\n");
		}

		snprintf(macaddr, 18, "%02x-%02x-%02x-%02x-%02x-%02x",
				Entry.macAddr_Dhcp[0], Entry.macAddr_Dhcp[1],
				Entry.macAddr_Dhcp[2], Entry.macAddr_Dhcp[3],
				Entry.macAddr_Dhcp[4], Entry.macAddr_Dhcp[5]);

		for (j=0; j<17; j++){
			if ( macaddr[j] != '-' ) {
				macaddr[j] = tolower(macaddr[j]);
			}
		}

		//fprintf(fp2, "%s: %s\n", Entry.macAddr, Entry.ipAddr);
		fprintf(fp2, "%s: %s\n", macaddr, inet_ntoa(*((struct in_addr *)Entry.ipAddr_Dhcp)) );
	}
	fclose(fp2);
	// Added by Mason Yu for MAC base assignment. End

dhcpConf:

#ifdef SUPPORT_DHCP_RESERVED_IPADDR
	setupDHCPReservedIPAddr();
#endif //SUPPORT_DHCP_RESERVED_IPADDR

#ifdef IMAGENIO_IPTV_SUPPORT
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
	if (mib_get(MIB_OPCH_ADDRESS, (void *)&value[0]) != 0) {
		strncpy(opchaddr, inet_ntoa(*(struct in_addr *)value), 16);
		opchaddr[15] = '\0';
	} else
		return -1;

	if (mib_get(MIB_IMAGENIO_DNS1, (void *)&value[0]) != 0) {
		strncpy(stbdns1, inet_ntoa(*(struct in_addr *)value), 16);
		stbdns1[15] = '\0';
	} else
		return -1;

	mib_get(MIB_IMAGENIO_DNS2, (void *)&myip);
	if ((myip.s_addr != 0xffffffff) && (myip.s_addr != 0)) { // not empty
		strncpy(stbdns2, inet_ntoa(myip), 16);
		stbdns2[15] = '\0';
	}

	if (!mib_get(MIB_OPCH_PORT, (void *)&opchport))
		return -1;
#endif
/*ping_zhang:20090930 END*/
#endif

	// IP max lease time
	if (mib_get(MIB_ADSL_LAN_DHCP_LEASE, (void *)&uLTime) == 0)
		return -1;

	if (mib_get(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)domain) == 0)
		return -1;

	// get DNS mode
	if (mib_get(MIB_ADSL_WAN_DNS_MODE, (void *)value) != 0)
	{
		dnsMode = (DNS_TYPE_T)(*(unsigned char *)value);
	}
	else
		return -1;

	// Commented by Mason Yu for LAN_IP as DNS Server
	if ((fp = fopen(DHCPD_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", DHCPD_CONF);
		return -1;
	}

	if (mib_get(MIB_SPC_ENABLE, (void *)value) != 0)
	{
		if (value[0])
		{
			spc_enable = 1;
			mib_get(MIB_SPC_IPTYPE, (void *)value);
			spc_ip = (unsigned int)(*(unsigned char *)value);
		}
		else
			spc_enable = 0;
	}

/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	fprintf(fp,"poolname default\n");
#endif
/*ping_zhang:20080919 END*/
	//ql: check if pool is in first IP subnet or in secondary IP subnet.
	mib_get(MIB_DHCP_POOL_START, (void *)&inpoolstart);
	mib_get(MIB_ADSL_LAN_IP, (void *)&lanip1);
	mib_get(MIB_ADSL_LAN_SUBNET, (void *)&lanmask1);
	if ((inpoolstart.s_addr & lanmask1.s_addr) == (lanip1.s_addr & lanmask1.s_addr))
		subnetIdx = 0;
	else {
#ifdef CONFIG_SECONDARY_IP
		mib_get(MIB_ADSL_LAN_IP2, (void *)&lanip2);
		mib_get(MIB_ADSL_LAN_SUBNET2, (void *)&lanmask2);
		if ((inpoolstart.s_addr & lanmask2.s_addr) == (lanip2.s_addr & lanmask2.s_addr))
			subnetIdx = 1;
		else
#endif //CONFIG_SECONDARY_IP
			subnetIdx = 0;
	}
	if (0 == subnetIdx)
		snprintf(serverip, 16, "%s", inet_ntoa(lanip1));
	else
		snprintf(serverip, 16, "%s", inet_ntoa(lanip2));

	fprintf(fp, "interface %s\n", LANIF);
	//ql add
	fprintf(fp, "server %s\n", serverip);
	// Mason Yu. For Test
#if 0
	fprintf(fp, "locallyserved no\n" );
	if (mib_get(MIB_ADSL_WAN_DHCPS, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(dhcps, inet_ntoa(*((struct in_addr *)value)), 16);
			dhcps[15] = '\0';
		}
		else
			dhcps[0] = '\0';
	}
	fprintf(fp, "dhcpserver %s\n", dhcps);
#endif

	if (spc_enable)
	{
		if (spc_ip == 0) { // single private ip
			fprintf(fp, "start %s\n", ipstart);
			fprintf(fp, "end %s\n", ipstart);
		}
		//else { // single shared ip
	}
	else
	{
		fprintf(fp, "start %s\n", ipstart);
		fprintf(fp, "end %s\n", ipend);
	}
#if 0
#ifdef IP_BASED_CLIENT_TYPE
	fprintf(fp, "pcstart %s\n", pcipstart);
	fprintf(fp, "pcend %s\n", pcipend);
	fprintf(fp, "cmrstart %s\n", cmripstart);
	fprintf(fp, "cmrend %s\n", cmripend);
	fprintf(fp, "stbstart %s\n", stbipstart);
	fprintf(fp, "stbend %s\n", stbipend);
	fprintf(fp, "phnstart %s\n", phnipstart);
	fprintf(fp, "phnend %s\n", phnipend);
	fprintf(fp, "hgwstart %s\n", hgwipstart);
	fprintf(fp, "hgwend %s\n", hgwipend);
	//ql 20090122 add
	fprintf(fp, "pcopt60 %s\n", pcopt60);
	fprintf(fp, "cmropt60 %s\n", cmropt60);
	fprintf(fp, "stbopt60 %s\n", stbopt60);
	fprintf(fp, "phnopt60 %s\n", phnopt60);
#endif
#endif

#if 0
/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
#ifndef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
#ifdef IP_BASED_CLIENT_TYPE
	{
		MIB_CE_IP_RANGE_DHCP_T Entry;
		int i, entrynum=mib_chain_total(MIB_IP_RANGE_DHCP_TBL);

		for (i=0; i<entrynum; i++)
		{
			if (!mib_chain_get(MIB_IP_RANGE_DHCP_TBL, i, (void *)&Entry))
				continue;

			strncpy(devipstart, inet_ntoa(*((struct in_addr *)Entry.startIP)), 16);
			strncpy(devipend, inet_ntoa(*((struct in_addr *)Entry.endIP)), 16);

/*ping_zhang:20090317 START:change len of option60 to 100*/
			//fprintf(fp, "range start=%s:end=%s:option=%s:devicetype=%d\n", devipstart, devipend, Entry.option60,Entry.deviceType);
			//fprintf(fp, "range start=%s:end=%s:option=%s:devicetype=%d:optCode=%d:optStr=%s\n", devipstart, devipend, Entry.option60,Entry.deviceType,Entry.optionCode,Entry.optionStr);
			//fprintf(fp, "range s=%s:e=%s:o=%s:t=%d:oC=%d:oS=%s\n",	devipstart, devipend, Entry.option60,Entry.deviceType,Entry.optionCode,Entry.optionStr);
			fprintf(fp, "range i=%d:s=%s:e=%s:o=%s:t=%d\n",i,devipstart, devipend, Entry.option60,Entry.deviceType);
			fprintf(fp, "range_optRsv i=%d:oC=%d:oS=%s\n",i,Entry.optionCode,Entry.optionStr);
/*ping_zhang:20090317 END*/
		}
	}
#endif
#endif
#endif //#if 0

/*ping_zhang:20090319 END*/
#ifdef IMAGENIO_IPTV_SUPPORT
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
	fprintf(fp, "opchaddr %s\n", opchaddr);
	fprintf(fp, "opchport %d\n", opchport);
	fprintf(fp, "stbdns1 %s\n", stbdns1);
	if (stbdns2[0])
		fprintf(fp, "stbdns2 %s\n", stbdns2);
#endif
/*ping_zhang:20090930 END*/
#endif

#ifdef IP_PASSTHROUGH
	ippt = 0;
	if (mib_get(MIB_IPPT_ITF, (void *)&ippt_itf) != 0)
		if (ippt_itf != DUMMY_IFINDEX) // IP passthrough
			ippt = 1;

	if (ippt)
	{
		fprintf(fp, "ippt yes\n");
		mib_get(MIB_IPPT_LEASE, (void *)&uInt);
		fprintf(fp, "ipptlt %d\n", uInt);
	}
#endif

	fprintf(fp, "opt subnet %s\n", dhcpsubnet);

	// Added by Mason Yu
	if (mib_get(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)value) != 0)
	{
		strncpy(ipaddr2, inet_ntoa(*((struct in_addr *)value)), 16);
		ipaddr2[15] = '\0';
	}
	else
		return -1;
	fprintf(fp, "opt router %s\n", ipaddr2);

	// Modified by Mason Yu for LAN_IP as DNS Server
#if 0
	if (dnsMode == DNS_AUTO)
	{
		fprintf(fp, "opt dns %s\n", ipaddr);
	}
	else	// DNS_MANUAL
	{
		if (dns1[0])
			fprintf(fp, "opt dns %s\n", dns1);
		if (dns2[0])
			fprintf(fp, "opt dns %s\n", dns2);
		if (dns3[0])
			fprintf(fp, "opt dns %s\n", dns3);
	}
#endif

	// Kaohj
#ifdef DHCPS_DNS_OPTIONS
	mib_get(MIB_DHCP_DNS_OPTION, (void *)value);
	if (value[0] == 0)
	fprintf(fp, "opt dns %s\n", ipaddr2);
	else { // check manual setting
		mib_get(MIB_DHCPS_DNS1, (void *)&myip);
		strncpy(ipaddr, inet_ntoa(myip), 16);
		ipaddr[15] = '\0';
		fprintf(fp, "opt dns %s\n", ipaddr);
		mib_get(MIB_DHCPS_DNS2, (void *)&myip);
		if (myip.s_addr != INADDR_NONE) { // not empty
			strncpy(ipaddr, inet_ntoa(myip), 16);
			ipaddr[15] = '\0';
			fprintf(fp, "opt dns %s\n", ipaddr);
			mib_get(MIB_DHCPS_DNS3, (void *)&myip);
			if (myip.s_addr != INADDR_NONE) { // not empty
				strncpy(ipaddr, inet_ntoa(myip), 16);
				ipaddr[15] = '\0';
				fprintf(fp, "opt dns %s\n", ipaddr);
			}
		}
	}
#else
	fprintf(fp, "opt dns %s\n", ipaddr2);
#endif
	fprintf(fp, "opt lease %u\n", uLTime);
	if (domain[0])
		fprintf(fp, "opt domain %s\n", domain);
	else
		// per TR-068, I-188
		fprintf(fp, "opt domain domain_not_set.invalid\n");

#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
{
	//format: opt venspec [enterprise-number] [sub-option code] [sub-option data] ...
	//opt vendspec: dhcp option 125 Vendor-Identifying Vendor-Specific
	//enterprise-number: 3561(ADSL Forum)
	//sub-option code: 4(GatewayManufacturerOUI)
	//sub-option code: 5(GatewaySerialNumber)
	//sub-option code: 6(GatewayProductClass)
	char opt125_sn[65];
	mib_get(MIB_HW_SERIAL_NUMBER, (void *)opt125_sn);
	fprintf( fp, "opt venspec 3561 4 %s 5 %s 6 %s\n", MANUFACTURER_OUI, opt125_sn, PRODUCT_CLASS );
}
#endif

/*write dhcp serving pool config*/
/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
	fprintf(fp, "poolend end\n");
	memset(&dhcppoolentry,0,sizeof(DHCPS_SERVING_POOL_T));

/*test code*
	dhcppoolentry.enable=1;
	dhcppoolentry.poolorder=1;
	strcpy(dhcppoolentry.poolname,"poolone");
	strcpy(dhcppoolentry.vendorclass,"MSFT 5.0");
	strcpy(dhcppoolentry.clientid,"");
	strcpy(dhcppoolentry.userclass,"");

	inet_aton("192.168.1.40",((struct in_addr *)(dhcppoolentry.startaddr)));
	inet_aton("192.168.1.50",((struct in_addr *)(dhcppoolentry.endaddr)));
	inet_aton("255.255.255.0",((struct in_addr *)(dhcppoolentry.subnetmask)));
	inet_aton("192.168.1.1",((struct in_addr *)(dhcppoolentry.iprouter)));
	inet_aton("172.29.17.10",((struct in_addr *)(dhcppoolentry.dnsserver1)));
	strcpy(dhcppoolentry.domainname,"poolone.com");
	dhcppoolentry.leasetime=86400;
	dhcppoolentry.dnsservermode=1;
	dhcppoolentry.InstanceNum=1;
	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
printf("\nentryNum=%d\n",entryNum);
	if(entryNum==0)
		mib_chain_add(MIB_DHCPS_SERVING_POOL_TBL,(void*)&dhcppoolentry);
****/

	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);

	for(i=0;i<entryNum;i++){
		memset(&dhcppoolentry,0,sizeof(DHCPS_SERVING_POOL_T));
		if(!mib_chain_get(MIB_DHCPS_SERVING_POOL_TBL,i,(void*)&dhcppoolentry))
			continue;
		if(dhcppoolentry.enable==1){
			strncpy(ipstart, inet_ntoa(*((struct in_addr *)(dhcppoolentry.startaddr))), 16);
			strncpy(ipend, inet_ntoa(*((struct in_addr *)(dhcppoolentry.endaddr))), 16);
			strncpy(dhcpsubnet, inet_ntoa(*((struct in_addr *)(dhcppoolentry.subnetmask))), 16);
			strncpy(ipaddr2, inet_ntoa(*((struct in_addr *)(dhcppoolentry.iprouter))), 16);

			if( dhcppoolentry.localserved ) //check only for locallyserved==true
			{
				if(!strcmp(ipstart,ipempty) ||
					!strcmp(ipend,ipempty) ||
					!strcmp(dhcpsubnet,ipempty))
					continue;
			}


			fprintf(fp, "poolname %s\n",dhcppoolentry.poolname);
			fprintf(fp, "cwmpinstnum %d\n",dhcppoolentry.InstanceNum);
			fprintf(fp, "poolorder %u\n",dhcppoolentry.poolorder);
			fprintf(fp, "interface %s\n", LANIF);
			//ql add
			fprintf(fp, "server %s\n", serverip);
			fprintf(fp, "start %s\n", ipstart);
			fprintf(fp, "end %s\n", ipend);

			fprintf(fp, "sourceinterface %u\n",dhcppoolentry.sourceinterface);
			if(dhcppoolentry.vendorclass[0]!=0){
				fprintf(fp, "vendorclass %s\n",dhcppoolentry.vendorclass);
				fprintf(fp, "vendorclassflag %u\n",dhcppoolentry.vendorclassflag);
				if(dhcppoolentry.vendorclassmode[0]!=0)
					fprintf(fp, "vendorclassmode %s\n",dhcppoolentry.vendorclassmode);
			}
			if(dhcppoolentry.clientid[0]!=0){
				fprintf(fp, "clientid %s\n",dhcppoolentry.clientid);
				fprintf(fp, "clientidflag %u\n",dhcppoolentry.clientidflag);
			}
			if(dhcppoolentry.userclass[0]!=0){
				fprintf(fp, "userclass %s\n",dhcppoolentry.userclass);
				fprintf(fp, "userclassflag %u\n",dhcppoolentry.userclassflag);
			}
			if(memcmp(dhcppoolentry.chaddr,macempty,6)){
				fprintf(fp, "chaddr %02x%02x%02x%02x%02x%02x\n",dhcppoolentry.chaddr[0],dhcppoolentry.chaddr[1],dhcppoolentry.chaddr[2],
					dhcppoolentry.chaddr[3],dhcppoolentry.chaddr[4],dhcppoolentry.chaddr[5]);
				if(memcmp(dhcppoolentry.chaddrmask,macempty,6))
					fprintf(fp, "chaddrmask %02x%02x%02x%02x%02x%02x\n",dhcppoolentry.chaddrmask[0],dhcppoolentry.chaddrmask[1],dhcppoolentry.chaddrmask[2],
						dhcppoolentry.chaddrmask[3],dhcppoolentry.chaddrmask[4],dhcppoolentry.chaddrmask[5]);
				fprintf(fp, "chaddrflag %u\n",dhcppoolentry.chaddrflag);
			}

#ifdef IP_PASSTHROUGH
			ippt = 0;
			if (mib_get(MIB_IPPT_ITF, (void *)&ippt_itf) != 0)
				if (ippt_itf != DUMMY_IFINDEX) // IP passthrough
					ippt = 1;

			if (ippt)
			{
				fprintf(fp, "ippt yes\n");
				mib_get(MIB_IPPT_LEASE, (void *)&uInt);
				fprintf(fp, "ipptlt %d\n", uInt);
			}
#endif
#ifdef IMAGENIO_IPTV_SUPPORT
			//fprintf(fp, "pcopt60 %s\n", pcopt60);
			//fprintf(fp, "cmropt60 %s\n", cmropt60);
			//fprintf(fp, "stbopt60 %s\n", stbopt60);
			//fprintf(fp, "phnopt60 %s\n", phnopt60);
/*ping_zhang:20090930 START:add for Telefonica new option 240*/
#if 0
			fprintf(fp, "opchaddr %s\n", opchaddr);
			fprintf(fp, "opchport %d\n", opchport);
			fprintf(fp, "stbdns1 %s\n", stbdns1);
			if (stbdns2[0])
				fprintf(fp, "stbdns2 %s\n", stbdns2);
#endif
/*ping_zhang:20090930 END*/
#endif

			if(strcmp(dhcpsubnet,ipempty))
				fprintf(fp, "opt subnet %s\n", dhcpsubnet);
			if(strcmp(ipaddr2,ipempty))
				fprintf(fp, "opt router %s\n", ipaddr2);

			// Kaohj
#ifdef DHCPS_DNS_OPTIONS
			if (dhcppoolentry.dnsservermode == 0)
				fprintf(fp, "opt dns %s\n", ipaddr2);
			else { // check manual setting
				strncpy(ipaddr, inet_ntoa(*((struct in_addr *)(dhcppoolentry.dnsserver1))), 16);
				ipaddr[15] = '\0';
				fprintf(fp, "opt dns %s\n", ipaddr);
				strncpy(ipaddr, inet_ntoa(*((struct in_addr *)(dhcppoolentry.dnsserver2))), 16);
				ipaddr[15] = '\0';
				if(strcmp(ipaddr,ipempty)) { // not empty
					fprintf(fp, "opt dns %s\n", ipaddr);
					strncpy(ipaddr, inet_ntoa(*((struct in_addr *)(dhcppoolentry.dnsserver3))), 16);
					ipaddr[15] = '\0';
					if(strcmp(ipaddr,ipempty)) { // not empty
						fprintf(fp, "opt dns %s\n", ipaddr);
					}
				}
			}
#else
			fprintf(fp, "opt dns %s\n", ipaddr2);
#endif
			fprintf(fp, "opt lease %u\n", (unsigned int)(dhcppoolentry.leasetime));
			if (dhcppoolentry.domainname[0])
				fprintf(fp, "opt domain %s\n", dhcppoolentry.domainname);
			else
				// per TR-068, I-188
				fprintf(fp, "opt domain domain_not_set.invalid\n");

#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
#ifdef _PRMT_X_CT_COM_DHCP_
		{
			//format: opt venspec [enterprise-number] [sub-option code] [sub-option data] ...
			//opt vendspec: dhcp option 125 Vendor-Identifying Vendor-Specific
			//enterprise-number: 0(e8 temprary enterprise code)
			//sub-option code: 2(DHCP server type)
			//sub-option code: 10(Internet VID)
			//sub-option code: 11(IPTV VID)
			//sub-option code: 12(TR069 VID)
			//sub-option code: 13(VoIP VID)
			unsigned char send_opt_125;
			mib_get(CWMP_CT_DHCPS_SEND_OPT_125, &send_opt_125);
			if(send_opt_125)
				fprintf( fp, "opt venspec 0 2 %s\n", "HGW-CT");
		}
#else
		{
			//format: opt venspec [enterprise-number] [sub-option code] [sub-option data] ...
			//opt vendspec: dhcp option 125 Vendor-Identifying Vendor-Specific
			//enterprise-number: 3561(ADSL Forum)
			//sub-option code: 4(GatewayManufacturerOUI)
			//sub-option code: 5(GatewaySerialNumber)
			//sub-option code: 6(GatewayProductClass)
			char opt125_sn[65];
			mib_get(MIB_HW_SERIAL_NUMBER, (void *)opt125_sn);
			fprintf( fp, "opt venspec 3561 4 %s 5 %s 6 %s\n", MANUFACTURER_OUI, opt125_sn, PRODUCT_CLASS );
		}
#endif
#endif
			//relayinfo
			if( dhcppoolentry.localserved==0 )
			{
				char dhcpserveripaddr[16];
				fprintf(fp, "locallyserved no\n" );
				strncpy(dhcpserveripaddr, inet_ntoa(*((struct in_addr *)(dhcppoolentry.dhcprelayip))), 16);
				dhcpserveripaddr[15]='\0';
				fprintf(fp, "dhcpserver %s\n", dhcpserveripaddr );
			}

			fprintf(fp, "poolend end\n");

		}
	}
#endif
/*ping_zhang:20080919 END*/

	fclose(fp);

//star: retain the lease file between during dhcdp restart
	if((fp = fopen(DHCPD_LEASE, "r")) == NULL)
	{
		if ((fp = fopen(DHCPD_LEASE, "w")) == NULL)
		{
			printf("Open file %s failed !\n", DHCPD_LEASE);
			return -1;
		}
		fprintf(fp, "\n");
		fclose(fp);
	}
	else
		fclose(fp);

	return 1;
}



// Added by Mason Yu for Dhcp Relay
// return value:
// 0  : not active
// 1  : active
// -1 : setup failed
int startDhcpRelay(void)
{
	unsigned char value[32];
	unsigned int dhcpmode;
	char dhcps[16];
	int status=0;


	if (mib_get(MIB_DHCP_MODE, (void *)value) != 0)
	{
		dhcpmode = (unsigned int)(*(unsigned char *)value);
		if (dhcpmode != DHCP_LAN_RELAY)
			return 0;	// dhcp Relay not on
	}

	// DHCP Relay is on
	if (dhcpmode == DHCP_LAN_RELAY) {

		//printf("DHCP Relay is on\n");

		if (mib_get(MIB_ADSL_WAN_DHCPS, (void *)value) != 0)
		{
			if (((struct in_addr *)value)->s_addr != 0)
			{
				strncpy(dhcps, inet_ntoa(*((struct in_addr *)value)), 16);
				dhcps[15] = '\0';
			}
			else
				dhcps[0] = '\0';
		}
		else
			return -1;

		//printf("dhcps = %s\n", dhcps);
#ifndef COMBINE_DHCPD_DHCRELAY
		status=va_cmd("/bin/dhcrelay", 1, 0, dhcps);
#else
		status=va_cmd(DHCPD, 2, 0, "-R", dhcps);
#endif
		status=(status==-1)?-1:1;

		return status;

	}

}

int delete_line(const char *filename, int delete_line)
{
	FILE *fileptr1, *fileptr2;
	char ch;
	int temp = 1;
	char *tmpFileName="/var/tmp/replica";

	//open file in read mode
	fileptr1 = fopen(filename, "r");
	fseek(fileptr1, 0, SEEK_SET);

	//open new file in write mode
	fileptr2 = fopen(tmpFileName, "w+");
	while ((ch = getc(fileptr1)) != EOF)
	{
		if (ch == '\n')
			temp++;

		//except the line to be deleted
		if (temp != delete_line)
		{
			//copy all lines in file replica.c
			putc(ch, fileptr2);
		}
	}
	fclose(fileptr1);
	fclose(fileptr2);
	remove(filename);
	//rename the file replica.c to original name
	rename(tmpFileName, filename);
	return 0;
}

int delete_hosts(unsigned char *yiaddr)
{
	FILE *fp;
	int dnsrelaypid=0;
	char temps[256+MAX_NAME_LEN]="", *pwd;
	char tmp1[20]="";
	int tmp_line = 0;

del_again:
	if ((fp = fopen(HOSTS, "r")) == NULL)
	{
		printf("Open file %s failed !\n", HOSTS);
		return 0;
	}

	fseek(fp, 0, SEEK_SET);
	temps[0] = '\0';
	tmp1[0] = '\0';
	tmp_line = 0;

	while (fgets(temps, 256+MAX_NAME_LEN, fp)) {
		tmp_line++;
		pwd = strstr(temps, yiaddr);
		if (pwd) {
			if(sscanf(temps, "%s%*", tmp1) == 1)
			{
				if (!strcmp(tmp1, yiaddr)) {
					fclose(fp);
					delete_line(HOSTS, tmp_line);
					goto del_again;
				}
			}
		}
	}
	fclose(fp);

	dnsrelaypid = read_pid((char*)DNSRELAYPID);
	if(dnsrelaypid > 0)
		kill(dnsrelaypid, SIGHUP);
	return 1;
}

int delete_dsldevice_on_hosts()
{
	unsigned char value[32];
	unsigned char IPStr[16]={0};

	// Get DNS entry "dsldevice" for its own address
	mib_get(MIB_ADSL_LAN_IP, (void *)value);
	strncpy(IPStr, inet_ntoa(*((struct in_addr *)value)), 16);
	delete_hosts(IPStr);
	return 1;
}

int add_dsldevice_on_hosts()
{
	unsigned char value, value2[32];
	FILE *fp;
	char domain[MAX_NAME_LEN];
	char devName[MAX_NAME_LEN];
	int dnsrelaypid=0;

#ifdef CONFIG_USER_DHCP_SERVER
	mib_get(MIB_DHCP_MODE, (void *)&value);
	if (value== DHCP_SERVER)
	{
		if ((fp = fopen(HOSTS, "a")) == NULL)
		{
			printf("Open file %s failed on DHCP_SERVER mode !\n", HOSTS);
			return -1;
		}
	}
	else
#endif
	{
		if ((fp = fopen(HOSTS, "w")) == NULL)
		{
			printf("Open file %s failed on Not DHCP_SERVER mode !\n", HOSTS);
			return -1;
		}
	}

	// add DNS entry "dsldevice" for its own address
	mib_get(MIB_ADSL_LAN_IP, (void *)value2);
	fprintf(fp, "%s\t", inet_ntoa(*((struct in_addr *)value2)));

	domain[0] = devName[0] = 0;
	mib_get(MIB_DEVICE_NAME, (void *)devName);

#ifdef CONFIG_USER_DHCP_SERVER
	if (value  == DHCP_SERVER)
	{	// add domain
		mib_get(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)domain);
		if (domain[0])
			fprintf(fp, "%s.%s ", devName, domain);
	}
#endif
	fprintf(fp, "%s\n", devName);
	fclose(fp);

	dnsrelaypid = read_pid((char*)DNSRELAYPID);
	if(dnsrelaypid > 0)
		kill(dnsrelaypid, SIGHUP);

	return 1;
}

const char RESOLV[] = "/var/resolv.conf";
const char DNSMASQ_CONF[] = "/var/dnsmasq.conf";
const char RESOLV_BACKUP[] = "/var/resolv_backup.conf";
const char MANUAL_RESOLV[] = "/var/resolv.manual.conf";
// Mason Yu. for copy the ppp & dhcp nameserver to the /var/resolv.conf
const char AUTO_RESOLV[] = "/var/resolv.auto.conf";	/*add resolv.auto.conf to distinguish DNS manual and auto config*/
const char DNS_RESOLV[] = "/var/udhcpc/resolv.conf";
const char DNS6_RESOLV[] = "/var/resolv6.conf";
const char PPP_RESOLV[] = "/etc/ppp/resolv.conf";
const char HOSTS[] = "/var/tmp/hosts";

// DNS relay server configuration
// return value:
// 1  : successful
// -1 : startup failed
int startDnsRelay(void)
{
	unsigned char value[32];
	FILE *fp;
	DNS_TYPE_T dnsMode;
	char *str;
	unsigned int i, vcTotal, resolvopt;
	char dns1[16], dns2[16], dns3[16];
	char dnsv61[48]={0};
	char dnsv62[48]={0};
	char dnsv63[48]={0};
	char domain[MAX_NAME_LEN];
	FILE *dnsfp=fopen(RESOLV,"w");		/* resolv.conf*/

	fprintf(dnsfp, "nameserver 127.0.0.1\n");
#ifdef CONFIG_IPV6
	fprintf(dnsfp, "nameserver ::1\n");
#endif
	fclose(dnsfp);
#ifdef OLD_DNS_MANUALLY
	if (mib_get(MIB_ADSL_WAN_DNS1, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != INADDR_NONE && ((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(dns1, inet_ntoa(*((struct in_addr *)value)), 16);
			dns1[15] = '\0';
		}
		else
			dns1[0] = '\0';
	}
	else
		return -1;

	if (mib_get(MIB_ADSL_WAN_DNS2, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != INADDR_NONE && ((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(dns2, inet_ntoa(*((struct in_addr *)value)), 16);
			dns2[15] = '\0';
		}
		else
			dns2[0] = '\0';
	}
	else
		return -1;

	if (mib_get(MIB_ADSL_WAN_DNS3, (void *)value) != 0)
	{
		if (((struct in_addr *)value)->s_addr != INADDR_NONE && ((struct in_addr *)value)->s_addr != 0)
		{
			strncpy(dns3, inet_ntoa(*((struct in_addr *)value)), 16);
			dns3[15] = '\0';
		}
		else
			dns3[0] = '\0';
	}
	else
		return -1;

#ifdef CONFIG_IPV6
	if (mib_get(MIB_ADSL_WAN_DNSV61, (void *)value) != 0)	{
		if (memcmp( ((struct in6_addr *)value)->s6_addr, dnsv61, 16))
		{
			inet_ntop(PF_INET6, &value, dnsv61, 48);
			dnsv61[47] = '\0';
		}
		else
			dnsv61[0] = '\0';
	}
	else
		return -1;

	if (mib_get(MIB_ADSL_WAN_DNSV62, (void *)value) != 0)	{
		if (memcmp( ((struct in6_addr *)value)->s6_addr, dnsv62, 16))
		{
			inet_ntop(PF_INET6, &value, dnsv62, 48);
			dnsv62[47] = '\0';
		}
		else
			dnsv62[0] = '\0';
	}
	else
		return -1;

	if (mib_get(MIB_ADSL_WAN_DNSV63, (void *)value) != 0)	{
		if (memcmp( ((struct in6_addr *)value)->s6_addr, dnsv63, 16))
		{
			inet_ntop(PF_INET6, &value, dnsv63, 48);
			dnsv63[47] = '\0';
		}
		else
			dnsv63[0] = '\0';
	}
	else
		return -1;
#endif

	// get DNS mode
	if (mib_get(MIB_ADSL_WAN_DNS_MODE, (void *)value) != 0)
	{
		dnsMode = (DNS_TYPE_T)(*(unsigned char *)value);
	}
	else
		return -1;

	if ((fp = fopen(MANUAL_RESOLV, "w")) == NULL)
	{
		printf("Open file %s failed !\n", MANUAL_RESOLV);
		return -1;
	}

	if (dns1[0])
		fprintf(fp, "nameserver %s\n", dns1);
	if (dns2[0])
		fprintf(fp, "nameserver %s\n", dns2);
	if (dns3[0])
		fprintf(fp, "nameserver %s\n", dns3);
#ifdef CONFIG_IPV6
	if (dnsv61[0])
		fprintf(fp, "nameserver %s\n", dnsv61);
	if (dnsv62[0])
		fprintf(fp, "nameserver %s\n", dnsv62);
	if (dnsv63[0])
		fprintf(fp, "nameserver %s\n", dnsv63);
#endif
	fclose(fp);

	if (dnsMode == DNS_MANUAL)
	{
		// dnsmasq -h -i LANIF -r MANUAL_RESOLV
		TRACE(STA_INFO, "get DNS from manual\n");
		str=(char *)MANUAL_RESOLV;
	}
	else	// DNS_AUTO
	{
#if 0
		MIB_CE_ATM_VC_T Entry;

		resolvopt = 0;
		vcTotal = mib_chain_total(MIB_ATM_VC_TBL);

		for (i = 0; i < vcTotal; i++)
		{
			/* get the specified chain record */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
				return -1;

			if (Entry.enable == 0)
				continue;

			if ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_PPPOE ||
				(CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_PPPOA)
				resolvopt = 1;
			else if ((DHCP_T)Entry.ipDhcp == DHCP_CLIENT)
				resolvopt = 2;
		}

#ifdef CONFIG_USER_PPPOMODEM
		{
			MIB_WAN_3G_T entry,*p;
			p=&entry;
			if(mib_chain_get( MIB_WAN_3G_TBL, 0, (void*)p))
			{
				if(p->enable) resolvopt = 1;
			}
		}
#endif //CONFIG_USER_PPPOMODEM

		if (resolvopt == 1) // get from PPP
		{
			// dnsmasq -h -i LANIF -r PPP_RESOLV
			TRACE(STA_INFO, "get DNS from PPP\n");
			str=(char *)PPP_RESOLV;
		}
		else if (resolvopt == 2) // get from DHCP client
		{
			// dnsmasq -h -i LANIF -r DNS_RESOLV
			TRACE(STA_INFO, "get DNS from DHCP client\n");
			str=(char *)DNS_RESOLV;
		}
		else	// get from manual
		{
			// dnsmasq -h -i LANIF -r MANUAL_RESOLV
			TRACE(STA_INFO, "get DNS from manual\n");
			str=(char *)MANUAL_RESOLV;
		}
#endif

		fp=fopen(AUTO_RESOLV,"r");
		if(fp)
			fclose(fp);
		else{
			fp=fopen(AUTO_RESOLV,"w");
			if(fp)
				fclose(fp);
		}
		str=(char *)AUTO_RESOLV;
	}
#endif

	// create hosts file
	delete_dsldevice_on_hosts();
	add_dsldevice_on_hosts();

	#if 0
	//va_cmd(DNSRELAY, 4, 0, (char *)ARG_I, (char *)LANIF, "-r", str);
	if (va_cmd(DNSRELAY, 2, 0, "-r", str))
	    return -1;
	#endif

#ifdef OLD_DNS_MANUALLY
//star: for the dns restart
	unlink(RESOLV);

	if (symlink(str, RESOLV)) {
			printf("failed to link %s --> %s\n", str, RESOLV);
			return -1;
	}
#endif

#ifdef SUPPORT_WEB_REDIRECT
	/* Do NOT cache failed DNS search results */
	if (va_cmd(DNSRELAY, 5, 0, "-C", DNSMASQ_CONF, "-r", RESOLV, "-N"))
	    return -1;
#else
	if (va_cmd(DNSRELAY, 4, 0, "-C", DNSMASQ_CONF, "-r", RESOLV))
	    return -1;
#endif

	return 1;
}

/*ql: 20081114 START: new IP QoS*/
#ifdef NEW_IP_QOS_SUPPORT
#define QOS_SETUP_DEBUG
static unsigned int qos_setup_debug = 3;

#ifdef QOS_SETUP_DEBUG
#define QOS_SETUP_PRINT_FUNCTION                    \
    do{if(qos_setup_debug&0x1) fprintf(stderr,"%s: %s  %d\n", __FILE__, __FUNCTION__,__LINE__);}while(0)
#else
#define QOS_SETUP_PRINT_FUNCTION do{}while(0)
#endif

enum qos_mode_t
{
	QOS_NONE=0,
	QOS_RULE,
	QOS_TC
};

static char* proto2str2layer[] = {
    [0]" ",
    [1]"--ip-proto 6",
    [2]"--ip-proto 17",
    [3]"--ip-proto 1",
};

static char* proto2str[] = {
    [0]" ",
    [1]"-p TCP",
    [2]"-p UDP",
    [3]"-p ICMP",
};

static const char QOS_CHAIN_EBT[] =  "ebt_rule";
static const char QOS_CHAIN_IPT[] =  "ipt_rule";

/****************************************
* getUpLinkRate:
* DESC: get upstream link rate.
****************************************/
static unsigned int getUpLinkRate(void)
{
#ifdef CONFIG_DEV_xDSL
	Modem_LinkSpeed vLs;
	unsigned char ret = 0;
	unsigned int total_bandwidth = 1024;//default to be 1Mbps

	ret = adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE);
	if (ret) {
		if(0 != vLs.upstreamRate)
			total_bandwidth = vLs.upstreamRate;
	}

	mib_set(MIB_QOS_UPRATE, (void *)&total_bandwidth);

	return total_bandwidth;
#else
	// Ethernet upLink: 100Mbps
	return 102400;
#endif
}

/************************************************************
* NAME: setupQosChain
* DESC: setup user defined chain in iptable tables or ebtables table
* RETURN: 0 - success; 1 - fail
************************************************************/
static int setupQoSChain(int enable)
{
	QOS_SETUP_PRINT_FUNCTION;

	if (enable) {
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-N", "ebt_rule");
		va_cmd(EBTABLES, 5, 1, "-t", "broute", "-P", "ebt_rule", "RETURN");
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-N", "ipt_rule");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-A", "PREROUTING", "-j", "ipt_rule");
#else
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-N", "ipt_rule");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-A", "FORWARD", "-j", "ipt_rule");
#endif
		//va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-N", "qos_rule");
		//va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-A", "FORWARD", "-j", "qos_rule");
	} else {
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-F", "ebt_rule");
		va_cmd(EBTABLES, 6, 1, "-t", "broute", "-D", "BROUTING", "-j", "ebt_rule");
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-X", "ebt_rule");

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-F", "ipt_rule");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-D", "PREROUTING", "-j", "ipt_rule");
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-X", "ipt_rule");
#else
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-F", "ipt_rule");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-D", "FORWARD", "-j", "ipt_rule");
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-X", "ipt_rule");
#endif
	}
	return 0;
}

/********************************************************************
* NAME: setupQosTcChain
* DESC: setup user defined chain in iptable tables or ebtables table for tc shaping
* RETURN: 0 - success; 1 - fail
********************************************************************/
static int setupQosTcChain(unsigned int enable)
{
	QOS_SETUP_PRINT_FUNCTION;

	if (enable)
	{
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-N", "ebt_rule");
		va_cmd(EBTABLES, 6, 1, "-t", "broute", "-A", "BROUTING", "-j", "ebt_rule");

		va_cmd(IPTABLES, 4, 1, "-t", "filter", "-N", "qos_filter");
		va_cmd(IPTABLES, 7, 1, "-t", "filter", "-I", "FORWARD", "1", "-j", "qos_filter");

		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-N", "qos_traffic");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-A", "POSTROUTING", "-j", "qos_traffic");
	} else {
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-F", "ebt_rule");
		va_cmd(EBTABLES, 6, 1, "-t", "broute", "-D", "BROUTING", "-j", "ebt_rule");
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-X", "ebt_rule");

		va_cmd(EBTABLES, 4, 1, "-t", "filter", "-F", "INPUT");

		va_cmd(IPTABLES, 4, 1, "-t", "filter", "-F", "qos_filter");
		va_cmd(IPTABLES, 6, 1, "-t", "filter", "-D", "FORWARD", "-j", "qos_filter");
		va_cmd(IPTABLES, 4, 1, "-t", "filter", "-X", "qos_filter");

		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-F", "qos_traffic");
		va_cmd(IPTABLES, 6, 1, "-t", "mangle", "-D", "POSTROUTING", "-j", "qos_traffic");
		va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-X", "qos_traffic");
	}

	return 1;
}

/*******************************************************
* enableIMQ:
* DESC: setup IMQ device and redirect all packet to IMQ queue.
********************************************************/
static int enableIMQ(void)
{
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;

	DOCMDARGVS(IFCONFIG, DOWAIT, "imq0 txqueuelen 100");
	DOCMDARGVS(IFCONFIG, DOWAIT, "imq0 up");

	DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle -A PREROUTING -i br+ -j IMQ --todev 0");

	return 0;
}

/*******************************************************
* cleanup_qos_setting:
* DESC: clean all tc rule and relevant iptables/ebtables rules.
********************************************************/
static void cleanupQdiscRule(void)
{
	MIB_CE_ATM_VC_T pvcEntry;
	int i = 0, vcNum = 0;
	char ifname[IFNAMSIZ];
	DOCMDINIT;

	//clear all tc rule on pvc...
	vcNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<vcNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry) || !pvcEntry.enable)
			continue;
		if (!pvcEntry.enableIpQos)
			continue;

		//interface
		ifGetName(pvcEntry.ifIndex, ifname, sizeof(ifname));

		DOCMDARGVS(TC, DOWAIT, "qdisc del dev %s root", ifname);
	}
}

static void cleanup_qos_setting(void)
{
	unsigned char qosmode;
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;

	mib_get(MIB_QOS_MODE, (void *)&qosmode);
	if (qosmode == QOS_NONE)
		return;
	else if (qosmode == QOS_TC) {
		setupQosTcChain(0);
		cleanupQdiscRule();
	}
	else if (qosmode == QOS_RULE) {
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle -D PREROUTING -i br+ -j IMQ --todev 0");
		DOCMDARGVS(IFCONFIG, DOWAIT, "imq0 down");
#endif

		setupQoSChain(0);

		//clear all tc rule on imq0.
#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		DOCMDARGVS(TC, DOWAIT, "qdisc del dev imq0 root");
#else
		cleanupQdiscRule();
#endif
	}
}

/******************************************************
* NAME: setup_filter_rule
* DESC: setup filter rule for relevant qdisc, and tag packet
                   according to qos rule.
* ARGS: policy 0 - PRIO; 1 - WRR
* RETURN: 0 - success; 1 - fail
******************************************************/
static int setup_filter_rule(unsigned char policy)
{
    MIB_CE_IP_QOS_T entry;
    int i = 0, EntryNum = 0;
	MIB_CE_ATM_VC_T pvcEntry;
	int j = 0, vcNum = 0;
    DOCMDINIT;

    EntryNum = mib_chain_total(MIB_IP_QOS_TBL);
	vcNum = mib_chain_total(MIB_ATM_VC_TBL);
    for(i=0; i<EntryNum; i++)
	{
		unsigned int mark=0;

		if(!mib_chain_get(MIB_IP_QOS_TBL, i, (void*)&entry)||!entry.enable)
		    continue;

		for (j=0; j<vcNum; j++)
		{
			char ifname[IFNAMSIZ];
			char phyPort[16]={0};
			char sport[48], dport[48], saddr[48], daddr[48], strmark[48];
			char strdscp[24] = {0};
			char *proto=NULL;
			char *eth_proto = NULL;
// Mason Yu. combine_1p_4p_PortMapping
#if (defined( ITF_GROUP_1P) && defined(ITF_GROUP))
			unsigned int wan_vlan_enable = 0;
#endif
			unsigned int bridge=0;
			unsigned char tos=0, dscp=0;

			if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry))
				continue;

			if (!pvcEntry.enable)
				continue;
			if (!pvcEntry.enableIpQos || pvcEntry.ifIndex != entry.outif)
				continue;

// Mason Yu. combine_1p_4p_PortMapping
#if (defined( ITF_GROUP_1P) && defined(ITF_GROUP))
			if (pvcEntry.vlan)
				wan_vlan_enable = 1;
#endif
			if (pvcEntry.cmode == CHANNEL_MODE_BRIDGE)
				bridge = 1;

			//interface
			ifGetName(pvcEntry.ifIndex, ifname, sizeof(ifname));

			//dscp match
#ifdef QOS_DSCP_MATCH
			if(0 != entry.qosDscp)
			{
				if (bridge)
					snprintf(strdscp, 24, "--ip-tos 0x%x", (entry.qosDscp-1)&0xFF);
				else
					snprintf(strdscp, 24, "-m dscp --dscp 0x%x", entry.qosDscp>>2);
			}else{
				strdscp[0]='\0';
			}
#endif

			//source address
			if(0 != entry.sip[0]) {
				if(0 != entry.smaskbit) {
					if (bridge)
						snprintf(saddr, 48, "--ip-source %s/%d",
							inet_ntoa(*(struct in_addr*)entry.sip), entry.smaskbit);
					else
						snprintf(saddr, 48, "-s %s/%d",
							inet_ntoa(*(struct in_addr*)entry.sip), entry.smaskbit);
				} else {
					if (bridge)
						snprintf(saddr, 48, "--ip-source %s", inet_ntoa(*(struct in_addr*)entry.sip));
					else
						snprintf(saddr, 48, "-s %s", inet_ntoa(*(struct in_addr*)entry.sip));
				}
			} else
				saddr[0]='\0';

			//dest address
			if(0 != entry.dip[0]) {
				if(0 != entry.dmaskbit) {
					if (bridge)
						snprintf(daddr, 48, "--ip-destination %s/%d",
							inet_ntoa(*(struct in_addr*)entry.dip), entry.dmaskbit);
					else
						snprintf(daddr, 48, "-d %s/%d",
							inet_ntoa(*(struct in_addr*)entry.dip), entry.dmaskbit);
				} else {
					if (bridge)
						snprintf(daddr, 48, "--ip-destination %s", inet_ntoa(*(struct in_addr*)entry.dip));
					else
						snprintf(daddr, 48, "-d %s", inet_ntoa(*(struct in_addr*)entry.dip));
				}
			} else
				daddr[0]='\0';

			//protocol
			if (bridge)
				proto = proto2str2layer[entry.protoType];//for ebtables
			else
				proto = proto2str[entry.protoType];//for iptables

			//source port (range)
			if((PROTO_NONE == entry.protoType) ||
				(PROTO_ICMP == entry.protoType) ||
				(0 == entry.sPort))
			{//if protocol is icmp or none or port not set, ignore the port
				sport[0] = '\0';
			}
			else
			{
				if (bridge)
					snprintf(sport, 48, "--ip-source-port %d", entry.sPort);
				else
					snprintf(sport, 48, "--sport %d", entry.sPort);
			}

			//dest port (range)
			if((PROTO_NONE == entry.protoType) ||
				(PROTO_ICMP == entry.protoType) ||
				(0 == entry.dPort))
			{//if protocol is icmp or none or port not set, ignore the port
				dport[0] = '\0';
			}
			else
			{
				if (bridge)
					snprintf(dport, 48, "--ip-destination-port %d", entry.dPort);
				else
					snprintf(dport, 48, "--dport %d", entry.dPort);
			}

			//lan port, USB, eth0_sw0-eth0_sw3, wlan
			if (entry.phyPort != 0xff) {
#if defined (IP_QOS_VPORT)
				if (entry.phyPort < SW_LAN_PORT_NUM)
					snprintf(phyPort, 16, "-i %s", SW_LAN_PORT_IF[entry.phyPort]);
#else
				if (entry.phyPort == 0)
					snprintf(phyPort, 16, "-i %s", ELANIF);
#endif
#ifdef CONFIG_USB_ETH
				else if (entry.phyPort == IFUSBETH_PHYNUM)
					snprintf(phyPort, 16, "-i %s", USBETHIF);
#endif
#ifdef WLAN_SUPPORT
				else {
					snprintf(phyPort, 16, "-i %s", getWlanIfName());
				}
#endif
			} else {
				if (bridge)
					phyPort[0] = '\0';
				else
					snprintf(phyPort, 16, "-i br0");
			}

			//lan 802.1p mark, 0-7 bit
			if(0 != entry.vlan1p) {
				if (bridge)
					snprintf(strmark, 48, "--vlan-prio %d", (entry.vlan1p-1)&0xff);
				else
					snprintf(strmark, 48, "-m mark --mark 0x%x", (entry.vlan1p-1)&0xff);
			} else
				strmark[0] = '\0';

			if (bridge) {
				if(strmark[0] != '\0')//vlan 802.1p priority, use 802.1Q ethernet protocol
				{
					eth_proto = "-p 0x8100";
				}else {//use ipv4 for ethernet protocol
					eth_proto = "-p 0x0800";
				}
			}

			//wan 802.1p mark
// Mason Yu. combine_1p_4p_PortMapping
#if defined( ITF_GROUP_1P) && defined(ITF_GROUP)
			if(wan_vlan_enable && (0 != entry.m_1p))
#else
			if (0 != entry.m_1p)
#endif
				mark = (entry.m_1p-1)&0xf;
			mark |= (i+1) << 8;

			//set the mark
			if (bridge) {
				DOCMDARGVS(EBTABLES, DOWAIT, "-t broute -A %s %s %s %s %s %s %s %s %s %s -j mark --set-mark 0x%x",
					QOS_CHAIN_EBT, phyPort, eth_proto, proto, saddr, sport, daddr, dport, strdscp, strmark, mark);
			} else {
				DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle -A %s %s %s %s %s %s %s %s %s -j MARK --set-mark 0x%x",
					QOS_CHAIN_IPT, phyPort, proto, saddr, sport, daddr, dport, strdscp, strmark, mark);
			}

			//reset match_mark
			if(strmark[0] != '\0') {
				if (bridge)
					snprintf(strmark, 48, "--vlan-prio %d", mark);
				else
					snprintf(strmark, 48, "-m mark --mark 0x%x", mark);
			}

			//set dscp
#ifdef QOS_DSCP
			if (entry.dscp) {//dscp target
				if (entry.m_dscp != 0) {
					if (bridge) {
						DOCMDARGVS(EBTABLES, DOWAIT, "-t broute -A %s %s %s %s %s %s %s %s %s %s -j ftos --set-ftos 0x%x",
							QOS_CHAIN_EBT, phyPort, eth_proto, proto, saddr, sport, daddr, dport, strdscp, strmark, (entry.m_dscp-1)&0xff);
					} else {
						DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle -A %s %s %s %s %s %s %s %s %s -j DSCP --set-dscp 0x%x",
							QOS_CHAIN_IPT, phyPort, proto, saddr, sport, daddr, dport, strdscp, strmark, entry.m_dscp>>2);
					}
				}
			} else  {
#endif
			if ((entry.m_ipprio != 0) || (entry.m_iptos != 0xff)) {
				if (entry.m_ipprio != 0)
					tos = (entry.m_ipprio-1) << 5;
				if (entry.m_iptos != 0xff)
					tos |= entry.m_iptos;

				if (bridge) {
					DOCMDARGVS(EBTABLES, DOWAIT, "-t broute -A %s %s %s %s %s %s %s %s %s %s -j ftos --set-ftos 0x%x",
						QOS_CHAIN_EBT, phyPort, eth_proto, proto, saddr, sport, daddr, dport, strdscp, strmark, tos);
				} else {
					DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle -A %s %s %s %s %s %s %s %s %s -j TOS --set-tos 0x%x",
						QOS_CHAIN_IPT, phyPort, proto, saddr, sport, daddr, dport, strdscp, strmark, tos);
				}
			}
#ifdef QOS_DSCP
			}
#endif

#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
			if(PLY_WRR == policy)//weighted round robin
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio 1 protocol ip handle 0x%x fw flowid 1:%d00",
					ifname, mark, entry.prior+1);
			}
			else if (PLY_PRIO == policy)//priority queue
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio %d protocol ip handle 0x%x fw flowid 1:%d",
					ifname, entry.prior+1, mark, entry.prior+1);
			}
#endif
		}

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		if(PLY_WRR == policy)//weighted round robin
		{
			DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio 1 protocol ip handle 0x%x fw flowid 1:%d00",
				mark, entry.prior+1);
		}
		else if (PLY_PRIO == policy)//priority queue
		{
			DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio %d protocol ip handle 0x%x fw flowid 1:%d",
				entry.prior+1, mark, entry.prior+1);
		}
#endif
	}

    return 0;
}

/****************************************************************
 *NAME:      setup_traffic_rule
 *DESC:      tc class add dev $DEV parent 10:1 handle 10:$SUBID htb rate $RATE ceil $CEIL
 *           tc filter add dev $DEV parent 10: protocol ip prio 0 handle $HANDLE fw classid 10:$SUBID
 *           iptables -p $PROTO -s $SADDR -d $DADDR --sport $SPORT --dport $DPORT -j MARK --set-mark $HANDLE
 *           setup traffic control for every configuration
 *ARGS:      index, start from 1
 *RETURN:    0 success, others  fail
 ****************************************************************/
static int setup_traffic_rule(MIB_CE_IP_TC_Tp entry)
{
	char ifname[IFNAMSIZ];
	char* tc_act = NULL, *fw_act=NULL;
	char* proto1 = NULL, *proto2 = NULL;
	char wanPort[16]={0};
	char  saddr[24], daddr[24], sport[16], dport[16];
	int upLinkRate=0, childRate=0;
	int mark;
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;

	tc_act = (char*)ARG_ADD;
	fw_act = (char*)FW_ADD;

	if(NULL == entry) {
		printf("Invalid traffic contolling rule!\n");
		goto error;
	}

	ifGetName(entry->ifIndex, ifname, sizeof(ifname));

	//wan interface
	snprintf(wanPort, 16, "-o %s", ifname);

	//source address and netmask
	if(0 != entry->srcip)
	{
		if(0 != entry->smaskbits) {
			snprintf(saddr, 24, "-s %s/%d", inet_ntoa(*((struct in_addr*)(&entry->srcip))), entry->smaskbits);
		} else {
			snprintf(saddr, 24, "-s %s", inet_ntoa(*((struct in_addr*)(&entry->srcip))));
		}
	}
	else {//if not specify the source ip
		saddr[0] = '\0';
	}

	//destination address and netmask
	if(0 != entry->dstip) {
		if(0 != entry->dmaskbits) {
			snprintf(daddr, 24, "-d %s/%d", inet_ntoa(*((struct in_addr*)(&entry->dstip))), entry->dmaskbits);
		} else {
			snprintf(daddr, 24, "-d %s", inet_ntoa(*((struct in_addr*)(&entry->dstip))));
		}
	} else {//if not specify the dest ip
		daddr[0] = '\0';
	}

	//source port
	if(0 != entry->sport) {
		snprintf(sport, 16, "--sport %d", entry->sport);
	} else {
		sport[0] = '\0';
	}

	//destination port
	if(0 != entry->dport) {
		snprintf(dport, 16, "--dport %d", entry->dport);
	} else {
		dport[0] = '\0';
	}

	//protocol
	if (((0 != entry->sport) || (0 != entry->dport)) &&
		(entry->protoType < 2))
		entry->protoType = 4;

	if(entry->protoType>4)//wrong protocol index
	{
		printf("Wrong protocol\n");
		goto error;
	} else {
		switch(entry->protoType)
		{
			case 0://NONE
				proto1 = " ";
				//filt_proto1 = " ";
				break;
			case 1://ICMP
				proto1 = "-p ICMP";
				//filt_proto1 = "match ip ptotocol 1 0xff";
				break;
			case 2://TCP
				proto1 = "-p TCP";
				//filt_proto1 = "match ip protocol 6 0xff";
				break;
			case 3://UDP
				proto1 = "-p UDP";
				//filt_proto1 = "match ip protocol 17 0xff";
				break;
			case 4://TCP/UDP
				proto1 = "-p TCP";
				proto2 = "-p UDP";
				//filt_proto1 = "match ip protocol 6 0xff";
				//filt_proto2 = "match ip protocol 17 0xff";
				break;
		}
	}

	upLinkRate = entry->limitSpeed;
	if(0 != upLinkRate)
	{
		//get mark
		mark = (entry->entryid<<12);

		//patch: true bandwidth will be a little greater than limit value, so I minish 7% of set limit value ahead.
		int ceil;
		ceil = upLinkRate/100 * 93;

		//childRate = (10 > upLinkRate)?upLinkRate:10;
		childRate = (10>ceil)?ceil:10;

		DOCMDARGVS(TC, DOWAIT, "class %s dev %s parent 1:1 classid 1:%d0 htb rate %dkbit ceil %dkbit mpu 64 overhead 4",
			tc_act, ifname, entry->entryid, childRate, ceil);

		DOCMDARGVS(TC, DOWAIT, "qdisc %s dev %s parent 1:%d0 handle %d1: pfifo",
			tc_act, ifname, entry->entryid, entry->entryid);

		DOCMDARGVS(TC, DOWAIT, "filter %s dev %s parent 1: protocol ip prio 0 handle 0x%x fw flowid 1:%d0",
			tc_act, ifname, mark, entry->entryid);

		DOCMDARGVS(IPTABLES, DOWAIT,  "-t mangle %s qos_traffic %s %s %s %s %s %s -j MARK --set-mark 0x%x",
			fw_act, wanPort, proto1, saddr, daddr, sport, dport, mark);

		/*TCP/UDP?*/
		if(proto2)//setup the other protocol
		{
			DOCMDARGVS(IPTABLES, DOWAIT, "-t mangle %s qos_traffic %s %s %s %s %s %s -j MARK --set-mark 0x%x",
				fw_act, wanPort, proto2, saddr, daddr, sport, dport, mark);
		}
	}
	else
	{//if uprate=0, forbid traffic matching the rules
		DOCMDARGVS(IPTABLES, DOWAIT, "-t filter %s qos_filter %s %s %s %s %s %s -j DROP",
			fw_act, wanPort, proto1, saddr, daddr, sport, dport);

		/*TCP/UDP again*/
		if(proto2)
		{
			DOCMDARGVS(IPTABLES, DOWAIT, "-t filter %s qos_filter %s %s %s %s %s %s -j DROP",
				fw_act, wanPort, proto2, saddr, daddr, sport, dport);
		}
	}

	return 0;
error:
	return 1;
}


/**************************************************************************
 * NAME:    setup_wrr_queue
 * DESC:    Using the htb qdisc to implement the wrr qdisc(surprised?), not
 *          CBQ, because the CBQ qdisc is so complicated and not very accurate.
 *          The skeleton of wrr(htb):
 *                                     HTB(root qdisc,1:)
 *                                      |
 *                                     HTB(root class,1:1)
 *                  ____________________|________________
 *                 |            |            |           |
 *                HTB          HTB          HTB         HTB
 *         (sub-cls,1:10) (sub-cls,1:20)(sub-cls,1:30)(sub-cls,1:40)
 *
 *         for example, bandwidth is 1024Kbit/s, there are three queues with
 *         priority 3:2:1, then these queues are allocated rate and ceil is
 *         1/2, 1/3, 1/6 of total bandwidth.
 *         This function is called when dsl synchronization is completed.
 *ARGS:
 *RETURN:  0 success, 1 fail.
**************************************************************************/
static int setup_wrr_queue(void)
{
#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	MIB_CE_ATM_VC_T vcEntry;
	int i, EntryNum;
	char ifname[IFNAMSIZ];
#endif
    int j, quantum;
	int rate = 0;
	unsigned int total_bandwidth = 0;
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;

	total_bandwidth = getUpLinkRate();

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	//tc qdisc add dev $DEV root handle 1: htb default 400
	DOCMDARGVS(TC,DOWAIT, "qdisc add dev imq0 root handle 1: htb default 400");

	//tc class add dev $DEV parent 1: classid 1:1 htb rate $RATE ceil $CEIL
	DOCMDARGVS(TC, DOWAIT, "class add dev imq0 parent 1: classid 1:1 htb rate %uKbit ceil %uKbit burst 15k",
		total_bandwidth, total_bandwidth);

	for(j=1; j<=4; j++)
	{
		/*ql:20080821 START: when line rate is low than 1Mbps, rate should be smaller...*/
		//rate = 700 * (5-j)/10;
		if (total_bandwidth > 950)
			rate = 70 * (5-j);
		else if (total_bandwidth > 790)
			rate = 60 * (5-j);
		else if (total_bandwidth > 650)
			rate = 50 * (5-j);
		else if (total_bandwidth > 540)
			rate = 40 * (5-j);
		else if (total_bandwidth > 300)
			rate = 30 * (5-j);
		else
			rate = 10 * (5-j);
		/*ql:20080821 END*/
		quantum = 1250 *(5-j);

		//if total bandwidth is too small, then reduce rate value
		/*ql:20080821 START: modify rate according to ceil*/
		//rate = (rate>=ceil)?(10*(5-j)):rate;
		if (rate > total_bandwidth)
		{
			rate = total_bandwidth * (5-j)/10;
		}
		/*ql:20080821 END*/
		//tc class add dev $DEV parent 10:1 classid 10:$SUBID htb rate $RATE ceil $RATE prio $PRIO
		DOCMDARGVS(TC, DOWAIT, "class add dev imq0 parent 1:1 classid 1:%d00 htb rate %dKbit ceil %uKbit prio 0 quantum %d",
			j, rate, total_bandwidth, quantum);
	}

	//set queue len
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:100 handle 100: pfifo limit 12");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:200 handle 200: pfifo limit 9");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:300 handle 300: pfifo limit 6");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:400 handle 400: pfifo limit 3");
#else
	EntryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<EntryNum; i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vcEntry)||!vcEntry.enable)
			continue;

		//get the interface name(?)
		ifGetName(vcEntry.ifIndex, ifname, sizeof(ifname));

		//tc qdisc add dev $DEV root handle 1: htb default 400
		DOCMDARGVS(TC,DOWAIT, "qdisc add dev %s root handle 1: htb default 400",ifname);

		//tc class add dev $DEV parent 1: classid 1:1 htb rate $RATE ceil $CEIL
		DOCMDARGVS(TC, DOWAIT, "class add dev %s parent 1: classid 1:1 htb rate %uKbit ceil %uKbit burst 15k",
			ifname, total_bandwidth, total_bandwidth);

		for(j=1; j<=4; j++)
		{
			/*ql:20080821 START: when line rate is low than 1Mbps, rate should be smaller...*/
			//rate = 700 * (5-j)/10;
			if (total_bandwidth > 950)
				rate = 70 * (5-j);
			else if (total_bandwidth > 790)
				rate = 60 * (5-j);
			else if (total_bandwidth > 650)
				rate = 50 * (5-j);
			else if (total_bandwidth > 540)
				rate = 40 * (5-j);
			else if (total_bandwidth > 300)
				rate = 30 * (5-j);
			else
				rate = 10 * (5-j);
			/*ql:20080821 END*/
			quantum = 1250 *(5-j);

			//if total bandwidth is too small, then reduce rate value
			/*ql:20080821 START: modify rate according to ceil*/
			//rate = (rate>=ceil)?(10*(5-j)):rate;
			if (rate > total_bandwidth)
			{
				rate = total_bandwidth * (5-j)/10;
			}
			/*ql:20080821 END*/
			//tc class add dev $DEV parent 10:1 classid 10:$SUBID htb rate $RATE ceil $RATE prio $PRIO
			DOCMDARGVS(TC, DOWAIT, "class add dev %s parent 1:1 classid 1:%d00 htb rate %dKbit ceil %uKbit prio 0 quantum %d",
				ifname, j, rate, total_bandwidth, quantum);
		}

		//set queue len
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:100 handle 100: pfifo limit 12", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:200 handle 200: pfifo limit 9", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:300 handle 300: pfifo limit 6", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:400 handle 400: pfifo limit 3", ifname);
	}
#endif

	//now, setup queue rules for wrr qdisc
	setup_filter_rule(PLY_WRR);

    return 0;
}

/*******************************************************************************
 *NAME:    setup_prio_queue
 *DESC:    if configurating policy to priority queue,
 *         create priority queues based on struct MIB_CE_IP_QUEUE_CFG_T setting,
 *         The default number of queue is four,1-4.
 *ARGS:    None
 *RETURN:  0 success, others fail
 *******************************************************************************/
static int setup_prio_queue(void)
{
#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	MIB_CE_ATM_VC_T vcEntry;
    int i, EntryNum;
	char ifname[IFNAMSIZ];
#endif
	DOCMDINIT;

    QOS_SETUP_PRINT_FUNCTION;

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
	//setup basic config for imq0
	DOCMDARGVS(TC,DOWAIT, "qdisc add dev imq0 root handle 1: prio bands 4 priomap 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3");

	//set queue len
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:1 handle 100: pfifo limit 10");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:2 handle 200: pfifo limit 10");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:3 handle 300: pfifo limit 10");
	DOCMDARGVS(TC, DOWAIT, "qdisc add dev imq0 parent 1:4 handle 400: pfifo limit 10");
#else
	EntryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<EntryNum; i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vcEntry) || !vcEntry.enable)
			continue;

		//get the interface name(?)
		ifGetName(vcEntry.ifIndex, ifname, sizeof(ifname));

		DOCMDARGVS(TC,DOWAIT, "qdisc add dev %s root handle 1: prio bands 4 priomap 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3",ifname);

		//set queue len
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:1 handle 100: pfifo limit 10", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:2 handle 200: pfifo limit 10", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:3 handle 300: pfifo limit 10", ifname);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:4 handle 400: pfifo limit 10", ifname);
	}
#endif

	//setup ip qos queue rules for pq
	setup_filter_rule(PLY_PRIO);

	return 0;
}

/*******************************************************************************
 * NAME:    setup_traffic_control
 * DESC:    main function to complte trafice controlling,
 *          setup the basic setting by calling setup_traffic_basic,
 *          and for every configuration by calling setup_traffic_cfg(),
 *          the basic setting includes one root qdisc and root
 *          class, the setting looks like below:
 *                              HTB(root qdisc, handle 10:)
 *                               |
 *                              HTB(root class, classid 10:1)
 *            ___________________|_____________________
 *            |         |        |          |          |
 *           HTB       HTB      HTB        HTB        HTB
 *(subclass id 10:10 rate Xkbit)........       (sub class id 10:N0 rate Ykbit)
 *ARGS:    none
 *RETURN:  0 success, others fail
 *******************************************************************************/
static int setup_traffic_control(void)
{
	MIB_CE_IP_TC_T  entry;
	MIB_CE_ATM_VC_T vcEntry;
	int i, entry_num =0, vcEntryNum = 0;
	char ifname[IFNAMSIZ];
	unsigned char totalBandWidthEn = 0;
	unsigned int bandwidth;
	unsigned short rate, ceil;
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;

	mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&totalBandWidthEn);
	entry_num = mib_chain_total(MIB_IP_QOS_TC_TBL);

	if (!totalBandWidthEn && (0==entry_num))
		return 1;

	if (totalBandWidthEn)
		mib_get(MIB_TOTAL_BANDWIDTH, (void *)&bandwidth);
	else
		bandwidth = getUpLinkRate();

	vcEntryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<vcEntryNum; i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vcEntry)||!vcEntry.enable)
			continue;

		//patch: actual bandwidth maybe a little greater than configured limit value, so I minish 7% of the configured limit value ahead.
		if (totalBandWidthEn)
			ceil = bandwidth/100 * 93;
		else
			ceil = bandwidth;

		ifGetName(vcEntry.ifIndex, ifname, sizeof(ifname));

		//tc qdisc add dev $DEV root handle 1: htb default 2
		DOCMDARGVS(TC,DOWAIT, "qdisc add dev %s root handle 1: htb default 2 r2q 1", ifname);

		//tc class add dev $DEV parent 1: classid 1:1 htb rate $RATE ceil $CEIL
		DOCMDARGVS(TC, DOWAIT, "class add dev %s parent 1: classid 1:1 htb rate %dKbit ceil %dKbit mpu 64 overhead 4 burst 15k",
			ifname, ceil, ceil);

		//patch with above
		rate = (ceil>10)?10:ceil;

		//tc class add dev $DEV parent 1:1 classid 1:2 htb rate $RATE ceil $CEIL
		DOCMDARGVS(TC, DOWAIT, "class add dev %s parent 1:1 classid 1:2 htb rate %dKbit ceil %dKbit mpu 64 overhead 4",
			ifname, rate, ceil);

		//DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:2 handle 2: tbf rate %ukbit latency 50ms burst 1540 mpu 64",
		//	ifname, total_bandwidth);
		DOCMDARGVS(TC, DOWAIT, "qdisc add dev %s parent 1:2 handle 2: pfifo limit 10", ifname);
	}

	for(i=0; i<entry_num; i++)
	{
		if(!mib_chain_get(MIB_IP_QOS_TC_TBL, i, (void*)&entry))
			continue;

		if (setup_traffic_rule(&entry))
			return 1;
	}

	return 0;
}

static void setup_default_rule(void)
{
#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	MIB_CE_ATM_VC_T vcEntry;
	int i, vcnum;
	char ifname[IFNAMSIZ];
#endif
	int k;
	unsigned char vChar, policy;
	DOCMDINIT;

	QOS_SETUP_PRINT_FUNCTION;

	mib_get(MIB_QOS_DOMAIN, (void *)&vChar);
	mib_get(MIB_QOS_POLICY, (void*)&policy);

#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	vcnum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<vcnum; i++) {
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vcEntry)||!vcEntry.enable)
			continue;

		//get the interface name(?)
		ifGetName(vcEntry.ifIndex, ifname, sizeof(ifname));

		if (vChar == (char)PRIO_IP) {
#ifdef CONFIG_8021P_PRIO
			unsigned char value[IPQOS_NUM_PKT_PRIO];
			if(mib_get(MIB_PRED_PRIO, (void *)value)== 0)
			{
				printf("Get 8021P_PROI  error!\n");
				return 0;
			}
#endif
			for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
				char pattern[]="0x00";
				int prio, flowid;

				prio = k<<1;
				if (prio<=9)
					pattern[2] += prio; // highest 3 bits
				else
					pattern[2] = 'a'+(prio-10);

#ifdef CONFIG_8021P_PRIO
				flowid = value[k]+1;
#else
				flowid = priomap[k];
#endif

				// match ip tos PATTERN MASK
				if(PLY_WRR == policy)//weighted round robin
				{
					DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio 2 protocol ip u32 match ip tos %s 0xe0 flowid 1:%d00",
						ifname, pattern, flowid);
				}
				else if (PLY_PRIO == policy)//priority queue
				{
					DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio 5 protocol ip u32 match ip tos %s 0xe0 flowid 1:%d",
						ifname, pattern, flowid);
				}
			}
		}
		else if(vChar == (char)PRIO_802_1p){ // PRIO_802_1p
#ifdef CONFIG_8021P_PRIO
			unsigned char value[IPQOS_NUM_PKT_PRIO];
			if(mib_get(MIB_8021P_PRIO, (void *)value)== 0)
			{
				printf("Get 8021P_PRIO  error!\n");
				return 0;
			}
#endif
			for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
				int flowid;

#ifdef CONFIG_8021P_PRIO
				flowid = value[k]+1;
#else
				flowid = priomap[k];
#endif

				if(PLY_WRR == policy)//weighted round robin
				{
					DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio 2 protocol ip handle %d fw flowid 1:%d00",
						ifname, k+1, flowid);
				}
				else if (PLY_PRIO == policy)//priority queue
				{
					DOCMDARGVS(TC, DOWAIT, "filter add dev %s parent 1: prio 5 protocol ip handle %d fw flowid 1:%d",
						ifname, k+1, flowid);
				}
			}
		}
	}
#else
	if (vChar == (char)PRIO_IP) {
#ifdef CONFIG_8021P_PRIO
		unsigned char value[IPQOS_NUM_PKT_PRIO];
		if(mib_get(MIB_PRED_PRIO, (void *)value)== 0)
		{
			printf("Get 8021P_PROI  error!\n");
			return 0;
		}
#endif
		for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
			char pattern[]="0x00";
			int prio, flowid;

			prio = k<<1;
			if (prio<=9)
				pattern[2] += prio; // highest 3 bits
			else
				pattern[2] = 'a'+(prio-10);

#ifdef CONFIG_8021P_PRIO
			flowid = value[k]+1;
#else
			flowid = priomap[k];
#endif
			// match ip tos PATTERN MASK
			if(PLY_WRR == policy)//weighted round robin
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio 2 protocol ip u32 match ip tos %s 0xe0 flowid 1:%d00",
					pattern, flowid);
			}
			else if (PLY_PRIO == policy)//priority queue
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio 5 protocol ip u32 match ip tos %s 0xe0 flowid 1:%d",
					pattern, flowid);
			}
		}
	}
	else if(vChar == (char)PRIO_802_1p){ // PRIO_802_1p
#ifdef CONFIG_8021P_PRIO
		unsigned char value[IPQOS_NUM_PKT_PRIO];
		if(mib_get(MIB_8021P_PRIO, (void *)value)== 0)
		{
			printf("Get 8021P_PRIO  error!\n");
			return 0;
		}
#endif
		for (k=0; k<=(IPQOS_NUM_PKT_PRIO-1); k++) {
			int flowid;

#ifdef CONFIG_8021P_PRIO
			flowid = value[k]+1;
#else
			flowid = priomap[k];
#endif

			if(PLY_WRR == policy)//weighted round robin
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio 2 protocol ip handle %d fw flowid 1:%d00",
					k+1, flowid);
			}
			else if (PLY_PRIO == policy)//priority queue
			{
				DOCMDARGVS(TC, DOWAIT, "filter add dev imq0 parent 1: prio 5 protocol ip handle %d fw flowid 1:%d",
					k+1, flowid);
			}
		}
	}
#endif
}

int setup_qos_setting(void)
{
	unsigned char policy;
	unsigned char vChar, qosmode;

	__dev_setupIPQoS(1);

	mib_get(MIB_MPMODE, (void *)&vChar);
	if (vChar & MP_IPQ_MASK)//qos priority
	{
		qosmode = QOS_RULE;

		//enable IP QoS on IMQ
		va_cmd("/bin/sarctl", 2, 1, "qos_imq", "1");

		mib_get(MIB_QOS_POLICY, (void *)&policy);

		setupQoSChain(1);

		if (policy == PLY_PRIO) {//for PRIO
			setup_prio_queue();
		}
		else if (policy == PLY_WRR) {//for WFQ
			setup_wrr_queue();
		}
		setup_default_rule();

#if defined(CONFIG_IMQ) || defined(CONFIG_IMQ_MODULE)
		enableIMQ();
#endif
	}//traffic shaping
	else
	{
		qosmode = QOS_TC;

		//disable IMQ
		va_cmd("/bin/sarctl", 2, 1, "qos_imq", "0");

		setupQosTcChain(1);

		if (setup_traffic_control()) {
			qosmode = QOS_NONE;
			setupQosTcChain(0);
			cleanupQdiscRule();
		}
	}
	mib_set(MIB_QOS_MODE, (void *)&qosmode);

	return 0;
}


/******************************************************
* NAME: stop_IPQoS
* DESC: when IP QoS stopped, traffic control will be effective.
******************************************************/
void stop_IPQoS(void)
{
	unsigned char mode=0;

	mib_get(MIB_MPMODE, (void *)&mode);
	mode &= ~MP_IPQ_MASK;
	mib_set(MIB_MPMODE, (void *)&mode);

	take_qos_effect();
}

#ifdef CONFIG_DEV_xDSL
static int change_queue(unsigned int upLinkRate)
{
    MIB_CE_ATM_VC_T entry;
    int i, j, vcnum=0;
	int rate, quantum;
    char ifname[IFNAMSIZ] = {0};
	unsigned char qosmode;
	DOCMDINIT;

	mib_get(MIB_QOS_MODE, (void *)&qosmode);

	if (qosmode == QOS_RULE)
	{
#if !defined(CONFIG_IMQ) && !defined(CONFIG_IMQ_MODULE)
	    vcnum = mib_chain_total(MIB_ATM_VC_TBL);
	    for(i=0; i<vcnum; i++)
	    {
			if((!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&entry)) || (!entry.enable))
			    continue;

			ifGetName(entry.ifIndex, ifname, sizeof(ifname));

			//tc class change dev $DEV parent 10: classid 10:1 htb rate $RATE ceil $CEIL prio 10
			DOCMDARGVS(TC, UNDOWAIT, "class change dev %s parent 1: classid 1:1 htb rate %dKbit ceil %dKbit burst 15k",
				ifname, upLinkRate, upLinkRate);

			for (j =1; j<=4; j++)
			{
				/*ql:20080821 START: when line rate is low than 1Mbps, rate should be smaller...*/
				//rate = 700 * (5-j)/10;
				if (upLinkRate > 950)
					rate = 70 * (5-j);
				else if (upLinkRate > 790)
					rate = 60 * (5-j);
				else if (upLinkRate > 650)
					rate = 50 * (5-j);
				else if (upLinkRate > 540)
					rate = 40 * (5-j);
				else if (upLinkRate > 300)
					rate = 30 * (5-j);
				else
					rate = 10 * (5-j);
				/*ql:20080821 END*/
				quantum = 1250 *(5-j);

				//if total bandwidth is too small, then reduce rate value
				if (rate > upLinkRate)
				{
					rate = upLinkRate * (5-j)/10;
				}
				//add subclass for one queue config
				//tc class add dev $DEV parent 10:1 classid 10:$SUBID htb rate $RATE ceil $RATE prio $PRIO
				DOCMDARGVS(TC, UNDOWAIT, "class change dev %s parent 1:1 classid 1:%d00 htb rate %dKbit ceil %dKbit prio 0 quantum %d",
					ifname, j, rate, upLinkRate, quantum);
			}
	    }
#else
		//tc class change dev $DEV parent 1: classid 1:1 htb rate $RATE ceil $CEIL
		DOCMDARGVS(TC, UNDOWAIT, "class change dev imq0 parent 1: classid 1:1 htb rate %dKbit ceil %dKbit burst 15k",
			upLinkRate, upLinkRate);

		for (j =1; j<=4; j++)
		{
				/*ql:20080821 START: when line rate is low than 1Mbps, rate should be smaller...*/
				//rate = 70 * (5-j);
				if (upLinkRate > 950)
					rate = 70 * (5-j);
				else if (upLinkRate > 790)
					rate = 60 * (5-j);
				else if (upLinkRate > 650)
					rate = 50 * (5-j);
				else if (upLinkRate > 540)
					rate = 40 * (5-j);
				else if (upLinkRate > 300)
					rate = 30 * (5-j);
				else
					rate = 10 * (5-j);
				/*ql:20080821 END*/
				quantum = 1250 * (5-j);
				/*ql:20080821 START: modify rate according to ceil*/
				//rate = (rate>=ceil)?(10*(5-j)):rate;
				if (rate > upLinkRate)
				{
					rate = upLinkRate * (5-j)/10;
				}
				/*ql:20080821 END*/

				//tc class add dev $DEV parent 10:1 classid 10:$SUBID htb rate $RATE ceil $RATE prio $PRIO
				DOCMDARGVS(TC, UNDOWAIT, "class change dev imq0 parent 1:1 classid 1:%d00 htb rate %dKbit ceil %dKbit prio 0 quantum %d",
					j, rate, upLinkRate, quantum);
		}
#endif
	}
	else if (qosmode == QOS_TC)
	{
	    vcnum = mib_chain_total(MIB_ATM_VC_TBL);
	    for(i=0; i<vcnum; i++)
	    {
			if((!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&entry)) || (!entry.enable))
			    continue;

			ifGetName(entry.ifIndex, ifname, sizeof(ifname));

			//tc class change dev $DEV parent 1: classid 1:1 htb rate $RATE ceil $CEIL
			DOCMDARGVS(TC,UNDOWAIT, "class change dev %s parent 1: classid 1:1 htb rate %dKbit ceil %dKbit",
				ifname, upLinkRate, upLinkRate);

			//tc class change dev $DEV parent 1:1 classid 1:2 htb rate $RATE ceil $CEIL
			DOCMDARGVS(TC,UNDOWAIT, "class change dev %s parent 1:1 classid 1:2 htb rate 10Kbit ceil %dKbit",
				ifname, upLinkRate);
	    }
	}

	mib_set(MIB_QOS_UPRATE, (void *)&upLinkRate);

	return 0;
}

int monitor_qos_setting(void)
{
    Modem_LinkSpeed vLs;
    unsigned char ret;
    unsigned char policy, mode, bandwidthlimit;
	unsigned int dsl_uprate;

	mib_get(MIB_QOS_POLICY, (void*)&policy);
	mib_get(MIB_QOS_UPRATE, (void *)&dsl_uprate);
	mib_get(MIB_QOS_MODE, (void *)&mode);
	mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&bandwidthlimit);

	if (((mode==QOS_RULE) && (policy == PLY_WRR)) ||
		((mode==QOS_TC) && !bandwidthlimit))
	{//wrr or traffical control with no totalbandwidth restrict
	    ret = adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE);
	    if (ret)
		{
			if(0 != vLs.upstreamRate)//setup
			{
			    if(dsl_uprate == vLs.upstreamRate)//need not setup
					return 0;
				else if((0 != dsl_uprate) && (dsl_uprate != vLs.upstreamRate))
				{
					change_queue(vLs.upstreamRate);
			    }
			}
		}
	}
    return 0;
}
#endif

#endif
/*ql: 20081114 END*/

#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD)
/*
 *	generate the ifup_ppp(vpc)x script for WAN interface
 */
static int generate_ifup_script_vpn(unsigned int ifIndex)
{
	int ret;
	FILE *fp;
	char wanif[8], ifup_path[32];

	ret = 0;

	ifGetName(ifIndex, wanif, sizeof(wanif));

	if (PPP_INDEX(ifIndex) != DUMMY_PPP_INDEX) {
		// PPP interface

		snprintf(ifup_path, 32, "/var/ppp/ifup_%s", wanif);
		if (fp=fopen(ifup_path, "w+") ) {
			fprintf(fp, "#!/bin/sh\n\n");

			fclose(fp);
			chmod(ifup_path, 484);
		}
#ifdef CONFIG_IPV6_VPN
		snprintf(ifup_path, 32, "/var/ppp/ifupv6_%s", wanif);
		if (fp=fopen(ifup_path, "w+") ) {
			fprintf(fp, "#!/bin/sh\n\n");

			fclose(fp);
			chmod(ifup_path, 484);
		}
#endif
	}
	else {
		// not supported till now
		return -1;
	}

	return ret;
}

/*
 *	generate the ifdown_ppp(vpn)x script for WAN interface
 */
static int generate_ifdown_script_vpn(unsigned int ifIndex)
{
	int ret;
	FILE *fp;
	char wanif[6], ifdown_path[32];
	char devname[IFNAMSIZ];

	ret = 0;

	if (PPP_INDEX(ifIndex) != DUMMY_PPP_INDEX) {
		// PPP interface
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(ifIndex));
		snprintf(ifdown_path, 32, "/var/ppp/ifdown_%s", wanif);
		if (fp=fopen(ifdown_path, "w+") ) {
			fprintf(fp, "#!/bin/sh\n\n");
			fclose(fp);
			chmod(ifdown_path, 484);
		}
#ifdef CONFIG_IPV6_VPN
		snprintf(ifdown_path, 32, "/var/ppp/ifdownv6_%s", wanif);
		if (fp=fopen(ifdown_path, "w+") ) {
			fprintf(fp, "#!/bin/sh\n\n");
			fclose(fp);
			chmod(ifdown_path, 484);
		}
#endif
	}
	else {
		// not supported till now
		return -1;
	}

	return ret;
}

/*
 *	remove the ifup_ppp(vpc)x script for WAN interface
 */
static int remove_ifup_script_vpn(unsigned int ifIndex)
{
	int ret;
	FILE *fp;
	char wanif[8], ifup_path[32];

	ret = 0;

	ifGetName(ifIndex, wanif, sizeof(wanif));

	if (PPP_INDEX(ifIndex) != DUMMY_PPP_INDEX) {
		// PPP interface

		snprintf(ifup_path, 32, "/var/ppp/ifup_%s", wanif);
		unlink(ifup_path);
#ifdef CONFIG_IPV6_VPN
		snprintf(ifup_path, 32, "/var/ppp/ifupv6_%s", wanif);
		unlink(ifup_path);
#endif
	}
	else {
		// not supported till now
		return -1;
	}

	return ret;
}

/*
 *	remove the ifdown_ppp(vpn)x script for WAN interface
 */
static int remove_ifdown_script_vpn(unsigned int ifIndex)
{
	int ret;
	FILE *fp;
	char wanif[6], ifdown_path[32];
	char devname[IFNAMSIZ];

	ret = 0;

	if (PPP_INDEX(ifIndex) != DUMMY_PPP_INDEX) {
		// PPP interface
		snprintf(wanif, 6, "ppp%u", PPP_INDEX(ifIndex));
		snprintf(ifdown_path, 32, "/var/ppp/ifdown_%s", wanif);
		unlink(ifdown_path);
#ifdef CONFIG_IPV6_VPN
		snprintf(ifdown_path, 32, "/var/ppp/ifdownv6_%s", wanif);
		unlink(ifdown_path);
#endif
	}
	else {
		// not supported till now
		return -1;
	}

	return ret;
}
#endif

#ifdef CONFIG_XFRM
struct IPSEC_PROP_ST ikeProps[] = {
	{"------------------------", 0x00000000},
	{"des-md5-group1", 0x01010001},//dhGroup|Encryption|ahAuth|Auth
	{"des-sha1-group1", 0x01010002},
	{"3des-md5-group2", 0x02020001},
	{"3des-sha1-group2", 0x02020002}
};

struct IPSEC_PROP_ST saProps[] = {
	{"------------------------", 0x00000000},
	{"esp-des-md5", 0x01010001},
	{"esp-des-sha1", 0x01010002},
	{"esp-3des-md5", 0x02020001},
	{"esp-3des-sha1", 0x02020002}
};

static char *encryptArraySetkey[] = {
	"null",
	"des-cbc",
	"3des-cbc",
	"aes-cbc",
	"simple-cbc",
	"blowfish-cbc",
	"cast128-cbc",
	"twofish-cbc"
};

static char *authArraySetkey[] = {
	"null",
	"hmac-md5",
	"hmac-sha1",
	"keyed-md5",
	"keyed-sha1",
	"hmac-sha2-256",
	"hmac-sha256",
	"tcp-md5"
};

static char *encryptArrayRacoon[] = {
	"",
	"des",
	"3des",
	"blowfish",
	"twofish"
};

static char *authArrayRacoon[] = {
	"",
	"md5",
	"sha1",
	"hmac_md5",
	"hmac_sha1",
	"hmac_sha2-256",
	"hmac_sha256"
};

static char *dhArray[] = {
	"",
	"modp768",
	"modp1024",
	"modp1536"
};

/*
return
	0: failed
	1: succeed
*/
static int applyIPsec(MIB_IPSEC_T *pEntry)
{
	char *strVal;
	int intVal;
	char local[20], remote[20], homeAddr[20], gwAddr[20];
	char *saProtocol[2]={"esp", "ah"};
	char *transportMode[2]={"tunnel", "transport"};
	char *ikeMode[2]={"main", "aggressive"};
	char *filterProtocol[4]={"any", "tcp", "udp", "icmp"};
	struct in_addr ipAddr;
	char filterPort[10];
	FILE *fpPSK, *fpSetKey, *fpRacoon;
	/*
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	*/
	if ((fpSetKey = fopen(SETKEY_CONF, "a"))== NULL){
		printf("ERROR: open file %s error!\n", PSK_FILE);
		return 0;
	}

	strVal = inet_ntoa(*(struct in_addr*)pEntry->remoteIP);
	strncpy(remote, strVal, 20);

	strVal = inet_ntoa(*(struct in_addr*)pEntry->localIP);
	strncpy(local, strVal, 20);

	strVal = inet_ntoa(*(struct in_addr*)pEntry->remoteTunnel);
	strncpy(gwAddr, strVal, 20);

	strVal = inet_ntoa(*(struct in_addr*)pEntry->localTunnel);
	strncpy(homeAddr, strVal, 20);

	/*
	if (getInAddr(pEntry->localWAN, IP_ADDR, (void *)&ipAddr) == 1){
		strVal = inet_ntoa(ipAddr);
		snprintf(homeAddr, 20, "%s", strVal);
	}else{
		printf("ERROR: can't get address of %s!\n", pEntry->localWAN);
		return 0;
	}

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0; i<entryNum; i++){
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)){
			return -1;
		}
		getDisplayWanName((void *)&Entry, Entry.WanName);
		if(!strncmp(Entry.WanName, pEntry->localWAN, sizeof(pEntry->localWAN)))
			break;
	}

	if(i >= entryNum)
		return -1;
	*/
	if(pEntry->filterPort != 0)
		snprintf(filterPort, 10, "%d", pEntry->filterPort);
	else
		snprintf(filterPort, 10, "%s", "any");

	if(pEntry->negotiationType == 1){
		//manual mode
		if((pEntry->encapMode&0x1) != 0x0){
			fprintf(fpSetKey, "add %s %s esp 0x%x -m %s ", local,remote,
				pEntry->espOUTSPI, transportMode[pEntry->transportMode]);

			if(pEntry->espEncrypt != 0)
				fprintf(fpSetKey, "-E %s 0x%s ", encryptArraySetkey[pEntry->espEncrypt], pEntry->espEncryptKey);
			if(pEntry->espAuth != 0)
				fprintf(fpSetKey, "-A %s 0x%s ", authArraySetkey[pEntry->espAuth], pEntry->espAuthKey);

			fprintf(fpSetKey, ";\n\n");

			fprintf(fpSetKey, "add %s %s esp 0x%x -m %s ", remote,local,
				pEntry->espINSPI, transportMode[pEntry->transportMode]);

			if(pEntry->espEncrypt != 0)
				fprintf(fpSetKey, "-E %s 0x%s ", encryptArraySetkey[pEntry->espEncrypt], pEntry->espEncryptKey);
			if(pEntry->espAuth != 0)
				fprintf(fpSetKey, "-A %s 0x%s ", authArraySetkey[pEntry->espAuth], pEntry->espAuthKey);

			fprintf(fpSetKey, ";\n\n");
		}

		if((pEntry->encapMode&0x2) != 0x0){
			fprintf(fpSetKey, "add %s %s ah 0x%x -m %s -A %s 0x%s ;\n\n", local, remote,
				pEntry->ahOUTSPI, transportMode[pEntry->transportMode], authArraySetkey[pEntry->ahAuth], pEntry->ahAuthKey);

			fprintf(fpSetKey, "add %s %s ah 0x%x -m %s -A %s 0x%s ;\n\n", remote, local,
				pEntry->ahINSPI, transportMode[pEntry->transportMode], authArraySetkey[pEntry->ahAuth], pEntry->ahAuthKey);

		}
	}

	//direction: in
	fprintf(fpSetKey, "spdadd %s/%d[%s] %s/%d[%s] %s -P in ipsec", remote, pEntry->remoteMask, filterPort,
		local, pEntry->localMask, filterPort, filterProtocol[pEntry->filterProtocol]);

	if(pEntry->transportMode == 0){
		if((pEntry->encapMode&0x1) != 0x0)
			fprintf(fpSetKey, " esp/tunnel/%s-%s/use", gwAddr, homeAddr);
		if((pEntry->encapMode&0x2) != 0x0)
			fprintf(fpSetKey, " ah/tunnel/%s-%s/use", gwAddr, homeAddr);
	}else{
		if((pEntry->encapMode&0x1) != 0x0)
			fprintf(fpSetKey, " esp/transport//use");
		if((pEntry->encapMode&0x2) != 0x0)
			fprintf(fpSetKey, " ah/transport//use");
	}

	fprintf(fpSetKey, ";\n\n");

	//direction: out
	fprintf(fpSetKey, "spdadd %s/%d[%s] %s/%d[%s] %s -P out ipsec", local, pEntry->localMask, filterPort,
		remote, pEntry->remoteMask, filterPort, filterProtocol[pEntry->filterProtocol]);

	if(pEntry->transportMode == 0){
		if((pEntry->encapMode&0x1) != 0x0)
			fprintf(fpSetKey, " esp/tunnel/%s-%s/require", homeAddr, gwAddr);
		if((pEntry->encapMode&0x2) != 0x0)
			fprintf(fpSetKey, " ah/tunnel/%s-%s/require", homeAddr, gwAddr);
	}else{
		if((pEntry->encapMode&0x1) != 0x0)
			fprintf(fpSetKey, " esp/transport//require");
		if((pEntry->encapMode&0x2) != 0x0)
			fprintf(fpSetKey, " ah/transport//require");
	}

	fprintf(fpSetKey, ";\n\n");
	fclose(fpSetKey);

	if(pEntry->negotiationType == 0){
		//ike mode
		if ((fpPSK = fopen(PSK_FILE, "a"))== NULL){
			printf("ERROR: open file %s error!\n", PSK_FILE);
			return 0;
		}
		if(access(RACOON_CONF, 0)!=0){
			if ((fpRacoon = fopen(RACOON_CONF, "a"))== NULL){
				printf("ERROR: open file %s error!\n", RACOON_CONF);
				return 0;
			}
			fprintf(fpRacoon, "path pre_shared_key \"%s\" ;\n", PSK_FILE);
		}else{
			if ((fpRacoon = fopen(RACOON_CONF, "a"))== NULL){
				printf("ERROR: open file %s error!\n", RACOON_CONF);
				return 0;
			}
		}

		fprintf(fpPSK, "%s %s\n", inet_ntoa(*(struct in_addr*)pEntry->remoteIP), pEntry->psk);
		fclose(fpPSK);

		fprintf(fpRacoon, "remote %s {\n", remote);
		fprintf(fpRacoon, "\texchange_mode %s ;\n", ikeMode[pEntry->ikeMode]);
		fprintf(fpRacoon, "\tlifetime time %d second ;\n", pEntry->ikeAliveTime);
		for(intVal = 0; intVal<4; intVal++){
			if(pEntry->ikeProposal[intVal]==0)
				continue;
			fprintf(fpRacoon, "\tproposal {\n");
			fprintf(fpRacoon, "\t\tencryption_algorithm %s ;\n", encryptArrayRacoon[ENCRYPT_INDEX(ikeProps[pEntry->ikeProposal[intVal]].algorithm)]);
			fprintf(fpRacoon, "\t\thash_algorithm %s ;\n", authArrayRacoon[AUTH_INDEX(ikeProps[pEntry->ikeProposal[intVal]].algorithm)]);
			fprintf(fpRacoon, "\t\tauthentication_method pre_shared_key ;\n");
			fprintf(fpRacoon, "\t\tdh_group %d ;\n", DHGROUP_INDEX(ikeProps[pEntry->ikeProposal[intVal]].algorithm));
			fprintf(fpRacoon, "\t}\n\n");
		}
		fprintf(fpRacoon, "}\n\n");

		for(intVal = 0; intVal<4; intVal++){
			if(pEntry->saProposal[intVal]==0)
				continue;
			fprintf(fpRacoon, "sainfo address %s/%d[%s] %s address %s/%d[%s] %s{\n", pEntry->transportMode==0?homeAddr:local, pEntry->localMask, filterPort,
					filterProtocol[pEntry->filterProtocol], pEntry->transportMode==0?gwAddr:remote, pEntry->localMask, filterPort, filterProtocol[pEntry->filterProtocol]);
			fprintf(fpRacoon, "\tpfs_group %s ;\n", dhArray[DHGROUP_INDEX(ikeProps[pEntry->ikeProposal[intVal]].algorithm)]);
			fprintf(fpRacoon, "\tlifetime time %d second ;\n", pEntry->saAliveTime);
			fprintf(fpRacoon, "\tencryption_algorithm %s ;\n", encryptArrayRacoon[ENCRYPT_INDEX(ikeProps[pEntry->ikeProposal[intVal]].algorithm)]);
			fprintf(fpRacoon, "\tauthentication_algorithm %s ;\n", authArrayRacoon[AUTH_INDEX(ikeProps[pEntry->ikeProposal[intVal]].algorithm)+2]);
			fprintf(fpRacoon, "\tcompression_algorithm deflate ;\n");
			fprintf(fpRacoon, "}\n\n");
		}
		fclose(fpRacoon);

	}
	return 1;
}

void ipsec_take_effect(void)
{

	int entryNum, i, newState;
	MIB_IPSEC_T Entry;
	char gwAddr[20];
	FILE *fp;
	char pid[10];

	va_cmd("/bin/setkey", 1, 1, "-F");
	va_cmd("/bin/setkey", 1, 1, "-FP");

	if(access(RACOON_PID, 0)==0){
		if ((fp = fopen(RACOON_PID, "r"))== NULL){
			printf("ERROR: open file %s error!\n", RACOON_PID);
			return;
		}
		fscanf(fp, "%d\n", &i);
		snprintf(pid, 10, "%d", i);
		va_cmd("/bin/kill", 1, 1,  pid);
		va_cmd("/bin/rm", 1, 1,  RACOON_PID);
	}

	if(access(PSK_FILE, 0)==0){
		if(remove(PSK_FILE))
			printf("Could not delete the file &s \n", PSK_FILE);
	}

	entryNum = mib_chain_total(MIB_IPSEC_TBL);
	for(i = 0; i<entryNum; i++){
		mib_chain_get(MIB_IPSEC_TBL, i, (void *)&Entry);

		if(Entry.enable ==1){
			newState = applyIPsec(&Entry);
			if(newState != Entry.state){
				Entry.state = newState;
				mib_chain_update(MIB_IPSEC_TBL, &Entry, i);
			}
		}
	}

	if(access(PSK_FILE, 0)==0)
		va_cmd("/bin/chmod", 2, 1, "600", PSK_FILE);

	if(access(SETKEY_CONF, 0)==0){
		va_cmd("/bin/setkey", 2, 1, "-f", SETKEY_CONF);
		if(remove(SETKEY_CONF))
			printf("Could not delete the file &s \n", SETKEY_CONF);
	}

	if(access(RACOON_CONF, 0)==0){
		va_cmd("/bin/racoon", 2, 1, "-f", RACOON_CONF);
		if(remove(RACOON_CONF))
			printf("Could not delete the file &s \n", RACOON_CONF);
	}

	return;
}
#endif

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_PPTPD_PPTPD
void applyPptpAccount(MIB_VPN_ACCOUNT_T *pentry, int enable)
{
	MIB_VPND_T server;
	int total, i;
	struct data_to_pass_st msg;
	char index[5];
	char auth[20];
	char enctype[20];
	char localip[20], peerip[20];

	if (1 == enable) {//add a pptp account

		if (0 == pentry->enable)
			return;

		//get server auth info
		total = mib_chain_total(MIB_VPN_SERVER_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_VPN_SERVER_TBL, i, &server))
				continue;

			if (VPN_PPTP == server.type)
				break;
		}

		if (i >= total) {
			printf("please configure pptp server info first.\n");
			return;
		}

		switch (server.authtype)
		{
		case 1://pap
			strcpy(auth, "pap");
			break;
		case 2://chap
			strcpy(auth, "chap");
			break;
		case 3://chapmsv2
			strcpy(auth, "chapms-v2");
			break;
		default:
			strcpy(auth, "auto");
			break;
		}

		switch (server.enctype)
		{
		case 1://MPPE
			strcpy(enctype, "+MPPE");
			break;
        case 2://MPPC
			strcpy(enctype, "+MPPC");
			break;
		case 3://MPPE&MPPC
			strcpy(enctype, "+BOTH");
			break;
		default:
			strcpy(enctype, "none");
			break;
		}

		snprintf(localip, 20, "%s", inet_ntoa(*(struct in_addr *)&server.localaddr));
		snprintf(peerip, 20, "%s", inet_ntoa(*(struct in_addr *)&server.peeraddr));

		snprintf(msg.data, BUF_SIZE, "spppctl addvpn %s type PPTP auth %s enctype %s username %s password %s localip %s peerip %s",
					pentry->name, auth, enctype, pentry->username, pentry->password, localip, peerip);

		printf("%s: %s\n", __func__, msg.data);

		write_to_pppd(&msg);
	}
	else {
		printf("/bin/spppctl delvpn %s\n", pentry->name);

		va_cmd("/bin/spppctl", 2, 1, "delvpn", pentry->name);
	}
}

void pptpd_take_effect(void)
{
	MIB_VPN_ACCOUNT_T entry;
	unsigned int entrynum, i;
	int enable;

	if ( !mib_get(MIB_PPTP_ENABLE, (void *)&enable) )
		return;

	entrynum = mib_chain_total(MIB_VPN_ACCOUNT_TBL);
	for (i=0; i<entrynum; i++)
	{
		if (!mib_chain_get(MIB_VPN_ACCOUNT_TBL, i, (void *)&entry))
			continue;
		if(VPN_PPTP == entry.type)
			applyPptpAccount(&entry, 0);
	}

	if (enable) {
		for (i=0; i<entrynum; i++)
		{
			if (!mib_chain_get(MIB_VPN_ACCOUNT_TBL, i, (void *)&entry))
				continue;
			if(VPN_PPTP == entry.type)
				applyPptpAccount(&entry, 1);
		}
	}
}
#endif

void applyPPtP(MIB_PPTP_T *pentry, int enable, int pptp_index)
{
	struct data_to_pass_st msg;
	char index[5];
	char auth[20];
	char enctype[20];
	// Mason Yu. Add VPN ifIndex
	// unit declarations for ppp  on if_sppp.h
	// (1) 0 ~ 7: pppoe/pppoa, (2) 8: 3G, (3) 9 ~ 10: PPTP, (4) 11 ~12: L2TP
	if (enable == 1) {	// add(persistent) or new(Dial-on-demand)
	
		switch (pentry->authtype)
		{
		case 1://pap
			strcpy(auth, "pap");
			break;
		case 2://chap
			strcpy(auth, "chap");
			break;
		case 3://chapmsv2
			strcpy(auth, "chapms-v2");
			break;
		default:
			strcpy(auth, "auto");
			break;
		}

		switch (pentry->enctype)
		{
		case 1://MPPE
			strcpy(enctype, "+MPPE");
			break;
        case 2://MPPC
			strcpy(enctype, "+MPPC");
			break;
		case 3://MPPE&MPPC
			strcpy(enctype, "+BOTH");
			break;
		default:
			strcpy(enctype, "none");
			break;
		}

		if (pentry->dgw) {
			snprintf(msg.data, BUF_SIZE,
				"spppctl add %d pptp auth %s username %s password %s server %s gw %d enctype %s", pentry->idx+9,
				auth, pentry->username, pentry->password, pentry->server, pentry->dgw, enctype);
		}
		else {
			snprintf(msg.data, BUF_SIZE,
				"spppctl add %d pptp auth %s username %s password %s server %s enctype %s", pentry->idx+9,
				auth, pentry->username, pentry->password, pentry->server, enctype);
		}

#ifdef CONFIG_IPV6_VPN
		snprintf(msg.data, BUF_SIZE, "%s ipt %u", msg.data, pEntry->IpProtocol - 1);
#endif

		printf("%s: %s\n", __func__, msg.data);

		generate_ifup_script_vpn(pentry->ifIndex);
		generate_ifdown_script_vpn(pentry->ifIndex);

		write_to_pppd(&msg);
#ifdef CONFIG_RTK_RG_INIT
		//RG add pptp wan!
		RG_add_pptp_wan(pentry,pptp_index);
#endif
	}
	else {
#ifdef CONFIG_RTK_RG_INIT
		//RG del pptp wan!
		if(pentry->rg_wan_idx>0){
			RG_WAN_Interface_Del(pentry->rg_wan_idx);
			pentry->rg_wan_idx=0;
			mib_chain_update(MIB_PPTP_TBL, pentry, pptp_index);
		}
#endif
		snprintf(index, 5, "%d", pentry->idx+9);
		va_cmd("/bin/spppctl", 4, 1, "del", index, "pptp", "0");
		remove_ifup_script_vpn(pentry->ifIndex);
		remove_ifdown_script_vpn(pentry->ifIndex);
	}
}

void pptp_take_effect(void)
{
	MIB_PPTP_T entry;
	unsigned int entrynum, i;
	int enable;

	if ( !mib_get(MIB_PPTP_ENABLE, (void *)&enable) )
		return;

	entrynum = mib_chain_total(MIB_PPTP_TBL);

	for (i=0; i<entrynum; i++)
	{
		if ( !mib_chain_get(MIB_PPTP_TBL, i, (void *)&entry) )
			return;

		applyPPtP(&entry, 0, i);
	}

	if (enable) {


		for (i=0; i<entrynum; i++)
		{
			if ( !mib_chain_get(MIB_PPTP_TBL, i, (void *)&entry) )
				return;

			applyPPtP(&entry, 1, i);
		}
	}
}
#endif

#ifdef CONFIG_USER_L2TPD_L2TPD
static const char *tunnel_auth[2] = {"none", "challenge"};
#endif
#ifdef CONFIG_USER_L2TPD_LNS
void applyL2tpAccount(MIB_VPN_ACCOUNT_T *pentry, int enable)
{
	MIB_VPND_T server;
	int total, i;
	struct data_to_pass_st msg;
	char index[5];
	char auth[20];
	char enctype[20];
	char localip[20], peerip[20];

	if (1 == enable) {//add a pptp account

		if (0 == pentry->enable)
			return;

		//get server auth info
		total = mib_chain_total(MIB_VPN_SERVER_TBL);
		for (i=0; i<total; i++) {
			if (!mib_chain_get(MIB_VPN_SERVER_TBL, i, &server))
				continue;

			if (VPN_L2TP == server.type)
				break;
		}

		if (i >= total) {
			printf("please configure l2tp server info first.\n");
			return;
		}

		switch (server.authtype)
		{
		case 1://pap
			strcpy(auth, "pap");
			break;
		case 2://chap
			strcpy(auth, "chap");
			break;
		case 3://chapmsv2
			strcpy(auth, "chapms-v2");
			break;
		default:
			strcpy(auth, "auto");
			break;
		}

		switch (server.enctype)
		{
		case 1://MPPE
			strcpy(enctype, "+MPPE");
			break;
        case 2://MPPC
			strcpy(enctype, "+MPPC");
			break;
		case 3://MPPE&MPPC
			strcpy(enctype, "+BOTH");
			break;
		default:
			strcpy(enctype, "none");
			break;
		}

		snprintf(localip, 20, "%s", inet_ntoa(*(struct in_addr *)&server.localaddr));
		snprintf(peerip, 20, "%s", inet_ntoa(*(struct in_addr *)&server.peeraddr));

		snprintf(msg.data, BUF_SIZE, "spppctl addvpn %s type L2TP auth %s enctype %s username %s password %s localip %s peerip %s",
					pentry->name, auth, enctype, pentry->username, pentry->password, localip, peerip);

		snprintf(msg.data, BUF_SIZE, "%s tunnel_auth %s", msg.data, tunnel_auth[server.tunnel_auth]);
		if (server.tunnel_auth) {
			snprintf(msg.data, BUF_SIZE, "%s secret %s", msg.data, server.tunnel_key);
		}

		printf("%s: %s\n", __func__, msg.data);

		write_to_pppd(&msg);
	}
	else {
		printf("/bin/spppctl delvpn %s\n", pentry->name);

		va_cmd("/bin/spppctl", 2, 1, "delvpn", pentry->name);
	}
}

void l2tpd_take_effect(void)
{
	MIB_VPN_ACCOUNT_T entry;
	unsigned int entrynum, i;
	int enable;

	if ( !mib_get(MIB_L2TP_ENABLE, (void *)&enable) )
		return;

	entrynum = mib_chain_total(MIB_VPN_ACCOUNT_TBL);
	for (i=0; i<entrynum; i++)
	{
		if (!mib_chain_get(MIB_VPN_ACCOUNT_TBL, i, (void *)&entry))
			continue;
		if(VPN_L2TP == entry.type)
			applyL2tpAccount(&entry, 0);
	}

	if (enable) {
		for (i=0; i<entrynum; i++)
		{
			if (!mib_chain_get(MIB_VPN_ACCOUNT_TBL, i, (void *)&entry))
				continue;
			if(VPN_L2TP == entry.type)
				applyL2tpAccount(&entry, 1);
		}
	}
}
#endif

#ifdef CONFIG_USER_L2TPD_L2TPD
static const char *l2tp_auth[] = {
	"auto", "pap", "chap", "chapms-v2"
};

static const char *l2tp_encryt[] = {
	"none", "+MPPE", "+MPPC", "+BOTH"
};

int applyL2TP(MIB_L2TP_T *pentry, int enable, int l2tp_index)
{
	struct data_to_pass_st msg;
	char index[5];

	// Mason Yu. Add VPN ifIndex
	// unit declarations for ppp  on if_sppp.h
	// (1) 0 ~ 7: pppoe/pppoa, (2) 8: 3G, (3) 9 ~ 10: PPTP, (4) 11 ~12: L2TP
	if (enable == 1) { 	// add(persistent) or new(Dial-on-demand)
		if (pentry->conntype != MANUAL) {
			snprintf(msg.data, BUF_SIZE,
				"spppctl add %d l2tp username %s password %s gw %d mru %d", pentry->idx+11,
				pentry->username, pentry->password, pentry->dgw, pentry->mtu);
		}
		else {
			snprintf(msg.data, BUF_SIZE,
				"spppctl new %d l2tp username %s password %s gw %d mru %d", pentry->idx+11,
				pentry->username, pentry->password, pentry->dgw, pentry->mtu);
		}

		snprintf(msg.data, BUF_SIZE, "%s auth %s", msg.data, l2tp_auth[pentry->authtype]);

		snprintf(msg.data, BUF_SIZE, "%s enctype %s", msg.data, l2tp_encryt[pentry->enctype]);

		snprintf(msg.data, BUF_SIZE, "%s server %s", msg.data, pentry->server);

		snprintf(msg.data, BUF_SIZE, "%s tunnel_auth %s", msg.data, tunnel_auth[pentry->tunnel_auth]);
		if (pentry->tunnel_auth) {
			snprintf(msg.data, BUF_SIZE, "%s secret %s", msg.data, pentry->secret);
		}

		if (pentry->conntype == CONNECT_ON_DEMAND)
			snprintf(msg.data, BUF_SIZE, "%s timeout %d", pentry->idletime);

#ifdef CONFIG_IPV6_VPN
		snprintf(msg.data, BUF_SIZE, "%s ipt %u", msg.data, pEntry->IpProtocol - 1);
#endif

		printf("%s: %s\n", __func__, msg.data);

		generate_ifup_script_vpn(pentry->ifIndex);
		generate_ifdown_script_vpn(pentry->ifIndex);
#ifdef CONFIG_RTK_RG_INIT
		//RG add pptp wan!
		RG_add_l2tp_wan(pentry,l2tp_index);
#endif

		write_to_pppd(&msg);
	}
	else if (enable == 2) {		// connect(up) for dial on-demand
		snprintf(msg.data, BUF_SIZE, "spppctl up %d", pentry->idx+11);
#ifdef CONFIG_RTK_RG_INIT
		//RG add pptp wan!
		RG_add_l2tp_wan(pentry,l2tp_index);
#endif
		write_to_pppd(&msg);
	}
	else if (enable == 3) {		// disconnect(down) for dial on-demand
		snprintf(msg.data, BUF_SIZE, "spppctl down %d", pentry->idx+11);
#ifdef CONFIG_RTK_RG_INIT
		//RG add pptp wan!
		RG_add_l2tp_wan(pentry,l2tp_index);
#endif
		write_to_pppd(&msg);
	}
	else {
#ifdef CONFIG_RTK_RG_INIT
		//RG del l2tp wan!
		if(pentry->rg_wan_idx>0){
			RG_WAN_Interface_Del(pentry->rg_wan_idx);
			pentry->rg_wan_idx=0;
			mib_chain_update(MIB_L2TP_TBL, pentry, l2tp_index);
		}
#endif
		snprintf(index, 5, "%d", pentry->idx+11);
		va_cmd("/bin/spppctl", 4, 1, "del", index, "l2tp", "0");
		remove_ifup_script_vpn(pentry->ifIndex);
		remove_ifdown_script_vpn(pentry->ifIndex);
	}

	return 1;
}

void l2tp_take_effect(void)
{
	MIB_L2TP_T entry;
	unsigned int entrynum, i;//, j;
	int enable;

	if ( !mib_get(MIB_L2TP_ENABLE, (void *)&enable) )
		return;

	entrynum = mib_chain_total(MIB_L2TP_TBL);


	//delete all firstly
	for (i=0; i<entrynum; i++)
	{
		if ( !mib_chain_get(MIB_L2TP_TBL, i, (void *)&entry) )
			return;

		applyL2TP(&entry, 0, i);
	}

	if (enable) {
		for (i=0; i<entrynum; i++)
		{
			if ( !mib_chain_get(MIB_L2TP_TBL, i, (void *)&entry) )
				return;

			applyL2TP(&entry, 1, i);
		}
	}
}
#endif

#ifdef _PRMT_X_CT_COM_PORTALMNT_
void setPortalMNT(void)
{
	char tmpstr[MAX_URL_LEN];
	char urlstr[MAX_URL_LEN + 16];
	unsigned char vChar;
	FILE *fp;
#ifdef CONFIG_RTK_RG_INIT
	int i, mib_id;
#endif

	mib_get(CWMP_CT_PM_ENABLE, &vChar);
	fp = fopen(TR069_FILE_PORTALMNT, "w");
	if(fp)
	{
		if (vChar == 1)
			fwrite("on", 2, 1, fp);
		else{
			fwrite("off", 3, 1, fp);
		}
		fclose(fp);
	}

#ifdef CONFIG_RTK_RG_INIT
	mib_id = CWMP_CT_PM_URL4PC;
	for(i=0; i<3; i++){
		/*
		/proc/rg/redirect_first_http_req_set_url
		command:
		a device_type url: add redirect url for specific device type, e.g. for PC (device type = 0) use "a 0 www.google.com"
		d device_type: delete redirect url for specific device type e.g. for STB (device type = 1) use "d 1"
		*/
		if(vChar == 1){
			mib_get(mib_id + i, tmpstr);
			if (tmpstr[0]){
				fp = fopen("/proc/rg/redirect_first_http_req_set_url", "w");
				fprintf(fp, "a %d %s", i, tmpstr);
				fclose(fp);
			}else{
				fp = fopen("/proc/rg/redirect_first_http_req_set_url", "w");
				fprintf(fp, "d %d", i);
				fclose(fp);
			}
		}
		else{
			fp = fopen("/proc/rg/redirect_first_http_req_set_url", "w");
			fprintf(fp, "d %d", i);
			fclose(fp);
		}
	}
#endif

	fp = fopen(TR069_FILE_PORTALMNT, "w");
	if(fp)
	{
		mib_get(CWMP_CT_PM_URL4PC, tmpstr);
		if (tmpstr[0]) {
			sprintf(urlstr, "PC_addr:%s", tmpstr);
			fwrite(urlstr, strlen(urlstr), 1, fp);
		}

		mib_get(CWMP_CT_PM_URL4STB, tmpstr);
		if (tmpstr[0]) {
			sprintf(urlstr, "STB_addr:%s", tmpstr);
			fwrite(urlstr, strlen(urlstr), 1, fp);
		}

		mib_get(CWMP_CT_PM_URL4MOBILE, tmpstr);
		if (tmpstr[0]) {
			sprintf(urlstr, "MOB_addr:%s", tmpstr);
			fwrite(urlstr, strlen(urlstr), 1, fp);
		}

		fclose(fp);
	}

}
#endif

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
// Added by Mason Yu for ILMI(PVC) community string
static const char PVCREADSTR[] = "ADSL";
static const char PVCWRITESTR[] = "ADSL";
// Added by Mason Yu for write community string
static const char SNMPCOMMSTR[] = "/var/snmpComStr.conf";
// return value:
// 1  : successful
// -1 : startup failed
int startSnmp(void)
{
	unsigned char value[16];
	unsigned char trapip[16];
	unsigned char commRW[100], commRO[100], enterOID[100];
	FILE 	      *fp;

	// Get SNMP Trap Host IP Address
	if(!mib_get( MIB_SNMP_TRAP_IP,  (void *)value)){
		printf("Can no read MIB_SNMP_TRAP_IP\n");
	}

	if (((struct in_addr *)value)->s_addr != 0)
	{
		strncpy(trapip, inet_ntoa(*((struct in_addr *)value)), 16);
		trapip[15] = '\0';
	}
	else
		trapip[0] = '\0';
	//printf("***** trapip = %s\n", trapip);

	// Get CommunityRO String
	if(!mib_get( MIB_SNMP_COMM_RO,  (void *)commRO)) {
		printf("Can no read MIB_SNMP_COMM_RO\n");
	}
	//printf("*****buffer = %s\n", commRO);


	// Get CommunityRW String
	if(!mib_get( MIB_SNMP_COMM_RW,  (void *)commRW)) {
		printf("Can no read MIB_SNMP_COMM_RW\n");
	}
	//printf("*****commRW = %s\n", commRW);


	// Get Enterprise OID
	if(!mib_get( MIB_SNMP_SYS_OID,  (void *)enterOID)) {
		printf("Can no read MIB_SNMP_SYS_OID\n");
	}
	//printf("*****enterOID = %s\n", enterOID);


	// Write community string to file
	if ((fp = fopen(SNMPCOMMSTR, "w")) == NULL)
	{
		printf("Open file %s failed !\n", SNMPCOMMSTR);
		return -1;
	}

	if (commRO[0])
		fprintf(fp, "readStr %s\n", commRO);
	if (commRW[0])
		fprintf(fp, "writeStr %s\n", commRW);

	// Add ILMI(PVC) community string
	fprintf(fp, "PvcReadStr %s\n", PVCREADSTR);
	fprintf(fp, "PvcWriteStr %s\n", PVCWRITESTR);
	fclose(fp);

	// Mason Yu
	// ZyXEL Remote management does not verify the comm string, so we can limit the comm string as "ADSL"
	if (va_cmd("/bin/snmpd", 8, 0, "-p", "161", "-c", PVCREADSTR, "-th", trapip, "-te", enterOID))
	    return -1;
	return 1;

}

#define SNMPPID  "/var/run/snmpd.pid"

int restart_snmp(int flag)
{
	unsigned char value[32];
	int snmppid=0;
	int status=0;

	snmppid = read_pid((char*)SNMPPID);

	//printf("\nsnmppid=%d\n",snmppid);

	if(snmppid > 0) {
		kill(snmppid, 9);
		unlink(SNMPPID);
	}

	if(flag==1){
		status = startSnmp();
	}
	return status;
}
#endif

#define CWMPPID  "/var/run/cwmp.pid"
#if defined(CONFIG_USER_CWMP_TR069) || defined(APPLY_CHANGE)
void off_tr069(void)
{
	int cwmppid=0;
	int status;

#ifdef CONFIG_MIDDLEWARE
	int midware_intf_pid = 0;
	mwSetFirewall(0);
	midware_intf_pid = read_pid((char*)MWINTF_MAIN_RUNFILE);
	if(midware_intf_pid > 0)
		kill(midware_intf_pid, SIGTERM);
#endif	//end of CONFIG_MIDDLEWARE

	cwmppid = read_pid((char*)CWMPPID);

	printf("\ncwmppid=%d\n",cwmppid);

	if(cwmppid > 0)
		kill(cwmppid, 15);

}
#endif

#if defined(IP_ACL) || defined(APPLY_CHANGE)
int restart_acl(void)
{
	unsigned char aclEnable;

	//va_cmd(IPTABLES, 2, 1, "-F", "aclblock");
	mib_get(MIB_ACL_CAPABILITY, (void *)&aclEnable);
	if (aclEnable == 1)  // ACL Capability is enabled
	{
		filter_set_acl(0);
		filter_set_acl(1);
	}
	else
		filter_set_acl(0);

	return 0;
}
#endif

//DNSRELAY

int restart_dnsrelay(void)
{
	unsigned char value[32];
	int dnsrelaypid=0, i;

	dnsrelaypid = read_pid((char*)DNSRELAYPID);

	//printf("\ndnsrelaypid=%d\n",dnsrelaypid);

	if(dnsrelaypid > 0)
		kill(dnsrelaypid, SIGTERM);

	// Kaohj -- wait for process termination
	for (i=0; i<10000; i++) {
		dnsrelaypid = read_pid((char*)DNSRELAYPID);
		if (dnsrelaypid<=0)
			break;
	}

	if (startDnsRelay() == -1)
	{
		printf("restart DNS relay failed !\n");
		return -1;
	}
	return 0;

}

int restart_dhcp(void)
{
	unsigned char value[32];
	unsigned int uInt;
	int dhcpserverpid=0,dhcprelaypid=0;
	int tmp_status, status=0;

	if (mib_get(MIB_DHCP_MODE, (void *)value) != 0)
	{
		uInt = (unsigned int)(*(unsigned char *)value);
//		if (uInt != 0 && uInt !=1 && uInt != 2 )
//			return -1;
	}

	dhcpserverpid = read_pid((char*)DHCPSERVERPID);
	dhcprelaypid = read_pid((char*)DHCPRELAYPID);

	//printf("\ndhcpserverpid=%d,dhcprelaypid=%d\n",dhcpserverpid,dhcprelaypid);

	if(dhcpserverpid > 0)
		kill(dhcpserverpid, 15);
	if(dhcprelaypid > 0)
		kill(dhcprelaypid, 15);
/*ping_zhang:20090319 START:fix 2 dhcp will appear bug*/
	//add to check if old udhcp is exit.
	while(read_pid((char*)DHCPSERVERPID)>0 ||read_pid((char*)DHCPRELAYPID)>0)
	{
		usleep(30000);
	}
/*ping_zhang:20090319 END*/
	if(uInt != DHCP_LAN_SERVER)
	{
		FILE *fp;
		//star:clean the lease file
		if ((fp = fopen(DHCPD_LEASE, "w")) == NULL)
		{
			printf("Open file %s failed !\n", DHCPD_LEASE);
			return -1;
		}
		fprintf(fp, "\n");
		fclose(fp);
	}


	if(uInt == DHCP_LAN_SERVER)
	{
		tmp_status = setupDhcpd();
		if (tmp_status == 1)
		{
			//printf("\nrestart dhcpserver!\n");
#ifdef COMBINE_DHCPD_DHCRELAY
			status = va_cmd(DHCPD, 2, 0, "-S", DHCPD_CONF);
#else
			status = va_cmd(DHCPD, 1, 0, DHCPD_CONF);
#endif

			while(read_pid((char*)DHCPSERVERPID) < 0)
				usleep(250000);

			restart_dnsrelay();
		}
		else if (tmp_status == -1)
	   	 	status = -1;
		return status;
	}
	else if(uInt == DHCP_LAN_RELAY)
	{
		startDhcpRelay();
		restart_dnsrelay();
		status=(status==-1)?-1:0;

		return status;
	}
	else
	{
		restart_dnsrelay();
		return 0;
	}



	return -1;
}

//Ip interface
int restart_lanip(void)
{
	char vChar=0;
	unsigned char ipaddr[32]="";
	unsigned char subnet[32]="";
	unsigned char value[6];
	int vInt;
	int status=0;
	FILE * fp;
#ifdef IP_PASSTHROUGH
	unsigned char ippt_addr[32]="";
	unsigned int ippt_itf;
	unsigned long myipaddr,hisipaddr;
	int ippt_flag = 0;
#endif

	itfcfg((char *)LANIF, 0);
#ifdef CONFIG_SECONDARY_IP
	itfcfg((char *)LAN_ALIAS, 0);
#endif
#ifdef IP_PASSTHROUGH
	itfcfg((char *)LAN_IPPT, 0);
	if (mib_get(MIB_IPPT_ITF, (void *)&ippt_itf) != 0) {
		if (ippt_itf != DUMMY_IFINDEX) {	// IP passthrough
			fp = fopen ("/tmp/PPPHalfBridge", "r");
			if (fp) {
				fread(&myipaddr, 4, 1, fp);
				// Added by Mason Yu. Access internet fail.
				fclose(fp);
				myipaddr+=1;
				strncpy(ippt_addr, inet_ntoa(*((struct in_addr *)(&myipaddr))), 16);
				ippt_addr[15] = '\0';
				ippt_flag =1;
			}

		}
	}
#endif

#ifdef _CWMP_MIB_
	mib_get(CWMP_LAN_IPIFENABLE, (void *)&vChar);
	if (vChar != 0)
#endif
	{
		if (mib_get(MIB_ADSL_LAN_IP, (void *)value) != 0)
		{
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
			ipaddr[15] = '\0';
		}
		if (mib_get(MIB_ADSL_LAN_SUBNET, (void *)value) != 0)
		{
			strncpy(subnet, inet_ntoa(*((struct in_addr *)value)), 16);
			subnet[15] = '\0';
		}

		vInt = 1500;
		snprintf(value, 6, "%d", vInt);
		// set LAN-side MRU
		status|=va_cmd(IFCONFIG, 6, 1, (char*)LANIF, ipaddr, "netmask", subnet, "mtu", value);


#ifdef CONFIG_SECONDARY_IP
		mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)value);
		if (value[0] == 1) {
			// ifconfig LANIF LAN_IP netmask LAN_SUBNET
			if (mib_get(MIB_ADSL_LAN_IP2, (void *)value) != 0)
			{
				strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
				ipaddr[15] = '\0';
			}
			if (mib_get(MIB_ADSL_LAN_SUBNET2, (void *)value) != 0)
			{
				strncpy(subnet, inet_ntoa(*((struct in_addr *)value)), 16);
				subnet[15] = '\0';
			}
			snprintf(value, 6, "%d", vInt);
			// set LAN-side MRU
			status|=va_cmd(IFCONFIG, 6, 1, (char*)LAN_ALIAS, ipaddr, "netmask", subnet, "mtu", value);
		}
#endif




#ifdef IP_PASSTHROUGH
               if(ippt_flag)
			   status|=va_cmd(IFCONFIG, 2, 1, (char*)LAN_IPPT,ippt_addr);
#endif

#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
		RG_reset_LAN();
#endif
		status|=restart_dhcp();

// Trigger igmpproxy to update interface info.
#ifdef CONFIG_USER_IGMPPROXY
		vInt = read_pid("/var/run/igmp_pid");
		if (vInt >= 1) {
			// Kick to sync the multicast virtual interfaces
			kill(vInt, SIGUSR1);
		}
#endif
		// Kaohj -- Disconnect network connection when LAN IP changes
		usleep(250000);
		cmd_killproc(PID_SHIFT(PID_TELNETD));
		return status;
	}
	return -1;
}

//DHCP

#ifdef EMBED
int getOneDhcpClient(char **ppStart, unsigned long *size, char *ip, char *mac, char *liveTime)
{
	struct dhcpOfferedAddr {
	u_int8_t chaddr[16];
	u_int32_t yiaddr;       /* network order */
	u_int32_t expires;      /* host order */
	u_int32_t interfaceType;
	u_int8_t hostName[64];
	};

	struct dhcpOfferedAddr entry;

	if ( *size < sizeof(entry) )
		return -1;

	entry = *((struct dhcpOfferedAddr *)*ppStart);
	*ppStart = *ppStart + sizeof(entry);
	*size = *size - sizeof(entry);

	if (entry.expires == 0)
		return 0;
//star: conflict ip addr will not be displayed on web
	if(entry.chaddr[0]==0&&entry.chaddr[1]==0&&entry.chaddr[2]==0&&entry.chaddr[3]==0&&entry.chaddr[4]==0&&entry.chaddr[5]==0)
		return 0;

	strcpy(ip, inet_ntoa(*((struct in_addr *)&entry.yiaddr)) );
	snprintf(mac, 20, "%02x:%02x:%02x:%02x:%02x:%02x",
			entry.chaddr[0],entry.chaddr[1],entry.chaddr[2],entry.chaddr[3],
			entry.chaddr[4], entry.chaddr[5]);

	snprintf(liveTime, 10, "%lu", (unsigned long)ntohl(entry.expires));

	return 1;
}
#endif

int delPortForwarding( unsigned int ifindex )
{
	int total,i;

	total = mib_chain_total( MIB_PORT_FW_TBL );
	//for( i=0;i<total;i++ )
	for( i=total-1;i>=0;i-- )
	{
		MIB_CE_PORT_FW_T *c, port_entity;
		c = &port_entity;
		if( !mib_chain_get( MIB_PORT_FW_TBL, i, (void*)c ) )
			continue;

		if(c->ifIndex==ifindex)
			mib_chain_delete( MIB_PORT_FW_TBL, i );
	}
	return 0;
}

int updatePortForwarding( unsigned int old_id, unsigned int new_id )
{
	unsigned int total,i;

	total = mib_chain_total( MIB_PORT_FW_TBL );
	for( i=0;i<total;i++ )
	{
		MIB_CE_PORT_FW_T *c, port_entity;
		c = &port_entity;
		if( !mib_chain_get( MIB_PORT_FW_TBL, i, (void*)c ) )
			continue;

		if(c->ifIndex==old_id)
		{
			c->ifIndex = new_id;
			mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)c, i );
		}
	}
	return 0;
}

#ifdef CONFIG_USER_ROUTED_ROUTED
int delRipTable(unsigned int ifindex)
{
	int total,i;
	MIB_CE_RIP_T Entry;

	total=mib_chain_total(MIB_RIP_TBL);

	for(i=total-1;i>=0;i--)	{
		mib_chain_get(MIB_RIP_TBL,i,&Entry);
		if (Entry.ifIndex!=ifindex) continue;
		mib_chain_delete(MIB_RIP_TBL,i);
	}
}
#endif

int delRoutingTable( unsigned int ifindex )
{
	int total,i;

	total = mib_chain_total( MIB_IP_ROUTE_TBL );
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
	Flush_RG_static_route();
#endif
	//for( i=0;i<total;i++ )
	for( i=total-1;i>=0;i-- )
	{
		MIB_CE_IP_ROUTE_T *c, entity;
		c = &entity;
		if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)c ) )
			continue;

		if(c->ifIndex==ifindex)
			mib_chain_delete( MIB_IP_ROUTE_TBL, i );
	}
	return 0;
}

int delPPPoESession(unsigned int ifindex)
{
	int totalEntry,i;
	MIB_CE_PPPOE_SESSION_T Entry;

	totalEntry = mib_chain_total(MIB_PPPOE_SESSION_TBL);
    for (i=0; i<totalEntry; i++) {
		if( !mib_chain_get(MIB_PPPOE_SESSION_TBL, i, &Entry) )
		{
  			printf("Get chain record error!\n");
			continue;
		}

		if (Entry.uifno == ifindex)
        {
			mib_chain_delete( MIB_PPPOE_SESSION_TBL, i );
        }
	}
	return 0;
}

/*ql:20080926 END*/
#ifdef NEW_IP_QOS_SUPPORT
int delIpQosTcRule(MIB_CE_ATM_VC_Tp pEntry)
{
	int total, i;
	MIB_CE_IP_TC_T entry;

	total = mib_chain_total(MIB_IP_QOS_TC_TBL);
	for (i=total-1; i>=0; i--)
	{
		mib_chain_get(MIB_IP_QOS_TC_TBL, i, &entry);
		if (entry.ifIndex != pEntry->ifIndex)
			continue;

		mib_chain_delete(MIB_IP_QOS_TC_TBL, i);
	}

	return(1);
}
#endif

int updateRoutingTable( unsigned int old_id, unsigned int new_id )
{
	unsigned int total,i;

	total = mib_chain_total( MIB_IP_ROUTE_TBL );
	for( i=0;i<total;i++ )
	{
		MIB_CE_IP_ROUTE_T *c, entity;
		c = &entity;
		if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)c ) )
			continue;

		if(c->ifIndex==old_id)
		{
			c->ifIndex = new_id;
			mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)c, i );
		}
	}
	return 0;
}

MIB_CE_ATM_VC_T *getATMVCEntryByIfIndex(unsigned int ifIndex, MIB_CE_ATM_VC_T *p)
{
	unsigned int i,num;

	if( (p==NULL) || (ifIndex==DUMMY_IFINDEX) ) return NULL;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;

		if( p->ifIndex==ifIndex )
		{
			return p;
		}
	}

	return NULL;
}

#ifdef _CWMP_MIB_

unsigned int findMaxConDevInstNum(MEDIA_TYPE_T mType)
{
	unsigned int ret=0, i,num;
	MIB_CE_ATM_VC_T *p,vc_entity;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		p = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;
		if (MEDIA_INDEX(p->ifIndex) != mType)
			continue;
		if( p->ConDevInstNum > ret )
			ret = p->ConDevInstNum;
	}

	return ret;
}

unsigned int findConDevInstNumByPVC(unsigned char vpi, unsigned short vci)
{
	unsigned int ret=0, i,num;
	MIB_CE_ATM_VC_T *p,vc_entity;

	if( (vpi==0) && (vci==0) ) return ret;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		p = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;
		if( p->vpi==vpi && p->vci==vci )
		{
			ret = p->ConDevInstNum;
			break;
		}
	}

	return ret;
}

unsigned int findMaxPPPConInstNum(MEDIA_TYPE_T mType, unsigned int condev_inst)
{
	unsigned int ret=0, i,num;
	MIB_CE_ATM_VC_T *p,vc_entity;

	if(condev_inst==0) return ret;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		p = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;
		if (MEDIA_INDEX(p->ifIndex) != mType)
			continue;
		if( p->ConDevInstNum == condev_inst )
		{
			if( (p->cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
			    ((p->cmode==CHANNEL_MODE_BRIDGE)&&(p->brmode==BRIDGE_PPPOE)) ||
#endif
			    (p->cmode==CHANNEL_MODE_PPPOA) )
			{
				if( p->ConPPPInstNum > ret )
					ret = p->ConPPPInstNum;
			}
		}
	}

	return ret;
}

unsigned int findMaxIPConInstNum(MEDIA_TYPE_T mType, unsigned int condev_inst)
{
	unsigned int ret=0, i,num;
	MIB_CE_ATM_VC_T *p,vc_entity;

	if(condev_inst==0) return ret;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		p = &vc_entity;
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;
		if (MEDIA_INDEX(p->ifIndex) != mType)
			continue;
		if( p->ConDevInstNum==condev_inst )
		{
			if( (p->cmode == CHANNEL_MODE_IPOE) ||
#ifdef PPPOE_PASSTHROUGH
			    ((p->cmode==CHANNEL_MODE_BRIDGE)&&(p->brmode!=BRIDGE_PPPOE)) ||
#else
			    (p->cmode == CHANNEL_MODE_BRIDGE) ||
#endif
			    (p->cmode == CHANNEL_MODE_RT1483) )
			{
				if( p->ConIPInstNum > ret )
					ret = p->ConIPInstNum;
			}
		}
	}

	return ret;
}


/*start use_fun_call_for_wan_instnum*/
int resetWanInstNum(MIB_CE_ATM_VC_Tp entry)
{
	if(entry==NULL) return -1;

	entry->connDisable=0;/*0: always for web/cli*/
	entry->ConDevInstNum=0;
	entry->ConIPInstNum=0;
	entry->ConPPPInstNum=0;
	return 0;
}

int updateWanInstNum(MIB_CE_ATM_VC_Tp entry)
{
	if(entry==NULL) return -1;

	if(entry->ConDevInstNum==0)
		entry->ConDevInstNum = 1 + findMaxConDevInstNum(MEDIA_INDEX(entry->ifIndex));

	if( (entry->cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
	    ((entry->cmode==CHANNEL_MODE_BRIDGE)&&(entry->brmode==BRIDGE_PPPOE)) ||
#endif
	    (entry->cmode==CHANNEL_MODE_PPPOA) )
	{
		if(entry->ConPPPInstNum==0)
			entry->ConPPPInstNum = 1 + findMaxPPPConInstNum(MEDIA_INDEX(entry->ifIndex), entry->ConDevInstNum );

		entry->ConIPInstNum = 0;
	}else{
		entry->ConPPPInstNum = 0;

		if(entry->ConIPInstNum==0)
			entry->ConIPInstNum = 1 + findMaxIPConInstNum(MEDIA_INDEX(entry->ifIndex), entry->ConDevInstNum);
	}

	return 0;
}
/*end use_fun_call_for_wan_instnum*/


/*ping_zhang:20080919 START:add for new telefonica tr069 request: dhcp option*/
#ifdef _PRMT_X_TELEFONICA_ES_DHCPOPTION_
unsigned int findMaxDHCPOptionInstNum( unsigned int usedFor, unsigned int dhcpConSPInstNum)
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPOption_entity;

	num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOption_entity;
		if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor!=usedFor || dhcpConSPInstNum != p->dhcpConSPInstNum)
			continue;
		if(p->dhcpOptInstNum>ret)
			ret=p->dhcpOptInstNum;
	}

	return ret;
}

int getDHCPOptionByOptInstNum( unsigned int dhcpOptNum, unsigned int dhcpSPNum, unsigned int usedFor, MIB_CE_DHCP_OPTION_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;

	if( (dhcpOptNum==0) || (p==NULL) || (id==NULL) )
		return ret;

	num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)p ) )
			continue;

		if( (p->usedFor==usedFor) && (p->dhcpOptInstNum==dhcpOptNum) && (p->dhcpConSPInstNum==dhcpSPNum) )
		{
			*id = i;
			ret = 0;
			break;
		}
	}
	return ret;
}

int getDHCPClientOptionByOptInstNum( unsigned int dhcpOptNum, unsigned int ifIndex, unsigned int usedFor, MIB_CE_DHCP_OPTION_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;

	if( (dhcpOptNum==0) || (p==NULL) || (id==NULL) )
		return ret;

	num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ) )
			continue;

		if( (p->usedFor==usedFor) && (p->dhcpOptInstNum==dhcpOptNum) &&(p->ifIndex==ifIndex) )
		{
			*id = i;
			ret = 0;
			break;
		}
	}
	return ret;
}

unsigned int findMaxDHCPClientOptionInstNum(int usedFor, unsigned int ifIndex)
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPOption_entity;

	num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOption_entity;
		if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor!=usedFor||p->ifIndex!=ifIndex)
			continue;
		if(p->dhcpOptInstNum>ret)
			ret=p->dhcpOptInstNum;
	}

	return ret;

}

unsigned int findDHCPOptionNum(int usedFor, unsigned int ifIndex)
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPOption_entity;

	num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOption_entity;
		if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor==usedFor && p->ifIndex==ifIndex)
			ret++;
	}

	return ret;

}

unsigned int findMaxDHCPReqOptionOrder(unsigned int ifIndex)
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPSP_entity;

	num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPSP_entity;
		if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor!=eUsedFor_DHCPClient_Req)
			continue;
		if(p->ifIndex != ifIndex)
			continue;
		if(p->order>ret)
			ret=p->order;
	}

	return ret;
}

unsigned int findMaxDHCPConSPInsNum(void )
{
	unsigned int ret=0, i,num;
	DHCPS_SERVING_POOL_T *p,DHCPSP_entity;

	num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPSP_entity;
		if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)p ))
			continue;
		if(p->InstanceNum>ret)
			ret=p->InstanceNum;
	}

	return ret;
}

unsigned int findMaxDHCPConSPOrder(void )
{
	unsigned int ret=0, i,num;
	DHCPS_SERVING_POOL_T *p,DHCPSP_entity;

	num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPSP_entity;
		if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)p ))
			continue;
		if(p->poolorder>ret)
			ret=p->poolorder;
	}

	return ret;
}

int getDHCPConSPByInstNum( unsigned int dhcpspNum,  DHCPS_SERVING_POOL_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;

	if( (dhcpspNum==0) || (p==NULL) || (id==NULL) )
		return ret;

	num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)p ) )
			continue;

		if( p->InstanceNum==dhcpspNum)
		{
			*id = i;
			ret = 0;
			break;
		}
	}
	return ret;
}

/*ping_zhang:20090319 START:replace ip range with serving pool of tr069*/
void clearOptTbl(unsigned int instnum)
{
	unsigned int  i,num,found;
	MIB_CE_DHCP_OPTION_T *p,DHCPOption_entity;

delOpt:
	num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOption_entity;
		if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor!=eUsedFor_DHCPServer_ServingPool)
			continue;
		if(p->dhcpConSPInstNum==instnum){
			mib_chain_delete(MIB_DHCP_SERVER_OPTION_TBL,i);
			break;
		}
	}
	if(i<num)
		goto delOpt;
	return;

}

unsigned int getSPDHCPOptEntryNum(unsigned int usedFor, unsigned int instnum)
{
	unsigned int ret=0, i,num;
	MIB_CE_DHCP_OPTION_T *p,DHCPOPT_entity;

	num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		p = &DHCPOPT_entity;
		if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)p ))
			continue;
		if(p->usedFor==usedFor && p->dhcpConSPInstNum==instnum)
			ret++;
	}

	return ret;
}

int getSPDHCPRsvOptEntryByCode(unsigned int instnum, unsigned char optCode, MIB_CE_DHCP_OPTION_T *optEntry ,int *id)
{
	unsigned int ret=-1, i,num;

	if(!optEntry)	return ret;

	num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)optEntry ))
			continue;
		if(optEntry->usedFor==eUsedFor_DHCPServer_ServingPool
			&& optEntry->dhcpConSPInstNum==instnum
			&& optEntry->tag==optCode)
		{
			*id = i;
			ret = 0;
			break;
		}
	}

	return ret;
}

void initSPDHCPOptEntry(DHCPS_SERVING_POOL_T *p)
{
	char origstr[GENERAL_LEN];
	char *origstrDomain=origstr;

	if(!p)
		return;

	memset( p, 0, sizeof( DHCPS_SERVING_POOL_T ) );
	p->enable = 1;
	p->poolorder = findMaxDHCPConSPOrder() + 1;
	strncpy(p->vendorclassmode,"Substring",MODE_LEN-1);
	p->vendorclassmode[MODE_LEN-1]=0;
	p->InstanceNum = findMaxDHCPConSPInsNum() +1;
	p->leasetime=86400;
	p->localserved = 1;//default: locallyserved=true;
	memset(p->chaddrmask,0xff,MAC_ADDR_LEN);//default to all 0xff
	mib_get(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)origstrDomain);
	strncpy(p->domainname,origstrDomain,GENERAL_LEN-1);
	p->domainname[GENERAL_LEN-1]=0;
	mib_get(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)p->iprouter);
#ifdef DHCPS_POOL_COMPLETE_IP
	mib_get(MIB_DHCP_SUBNET_MASK, (void *)p->subnetmask);
#else
	mib_get(MIB_ADSL_LAN_SUBNET, (void *)p->subnetmask);
#endif
}

int delDhcpcOption( unsigned int ifindex )
{
	MIB_CE_DHCP_OPTION_T entry;
	int i, entrynum;

	entrynum = mib_chain_total(MIB_DHCP_CLIENT_OPTION_TBL);
	for (i=entrynum-1; i>=0; i--)
	{
		if (!mib_chain_get(MIB_DHCP_CLIENT_OPTION_TBL, i, (void *)&entry))
			continue;

		if (entry.ifIndex == ifindex)
			mib_chain_delete(MIB_DHCP_CLIENT_OPTION_TBL, i);
	}
	return 0;
}

void compact_reqoption_order(unsigned int ifIndex)
{
	//int ret=-1;
	int num,i,j;
	int maxorder;
	MIB_CE_DHCP_OPTION_T *p,pentry;
	char *orderflag;

	while(1){
		p=&pentry;
		maxorder=findMaxDHCPReqOptionOrder(ifIndex);
		orderflag=(char*)malloc(maxorder+1);
		if(orderflag==NULL) return;
		memset(orderflag,0,maxorder+1);

		num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
		for( i=0; i<num;i++ )
		{

				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
					continue;
				if(p->usedFor!=eUsedFor_DHCPClient_Req)
					continue;
				if(p->ifIndex != ifIndex)
					continue;
				orderflag[p->order]=1;
		}
		for(j=1;j<=maxorder;j++){
			if(orderflag[j]==0)
				break;
		} //star: there only one 0 in orderflag array
		if(j==(maxorder+1))
		{
			if(orderflag)
			{
				free(orderflag);
				orderflag=NULL;
			}
			break;
		}
		for( i=0; i<num;i++ )
		{

				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)p ))
					continue;
				if(p->usedFor!=eUsedFor_DHCPClient_Req)
					continue;
				if(p->ifIndex != ifIndex)
					continue;
				if(p->order>j){
					(p->order)--;
					mib_chain_update(MIB_DHCP_CLIENT_OPTION_TBL,(void*)p,i);
				}
		}

		if(orderflag)
		{
			free(orderflag);
			orderflag=NULL;
		}
	}

}
/*ping_zhang:20090319 END*/
#endif
/*ping_zhang:20080919 END*/

MIB_CE_ATM_VC_T *getATMVCByInstNum( unsigned int devnum, unsigned int ipnum, unsigned int pppnum, MIB_CE_ATM_VC_T *p, unsigned int *chainid )
{
	unsigned int i,num;

	if( (p==NULL) || (chainid==NULL) ) return NULL;

	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
			continue;

		if( (p->ConDevInstNum==devnum) &&
		    (p->ConIPInstNum==ipnum) &&
		    (p->ConPPPInstNum==pppnum) )
		{
			*chainid=i;
			return p;
		}
	}

	return NULL;
}

#if defined(IP_QOS) || defined(CONFIG_USER_IP_QOS_3)
unsigned int getQoSQueueNum(void)
{
	unsigned int i,entryNum,num=0;
	MIB_CE_IP_QOS_QUEUE_T entry;

	entryNum = mib_chain_total(MIB_IP_QOS_QUEUE_TBL);
	for(i=0; i<entryNum; i++)
	{
		if( !mib_chain_get( MIB_IP_QOS_QUEUE_TBL, i, (void*)&entry ))
			continue;

		num++;
	}
	return num;
}
#endif

#ifdef _PRMT_TR143_
static const char gUDPEchoServerName[] = "/bin/udpechoserver";
static const char gUDPEchoServerPid[] = "/var/run/udpechoserver.pid";

void UDPEchoConfigSave(struct TR143_UDPEchoConfig *p)
{
	if(p)
	{
		unsigned char itftype;
		mib_get( TR143_UDPECHO_ENABLE, (void *)&p->Enable );
		mib_get( TR143_UDPECHO_SRCIP, (void *)p->SourceIPAddress );
		mib_get( TR143_UDPECHO_PORT, (void *)&p->UDPPort );
		mib_get( TR143_UDPECHO_PLUS, (void *)&p->EchoPlusEnabled );

		mib_get( TR143_UDPECHO_ITFTYPE, (void *)&itftype );
		if(itftype==ITF_WAN)
		{
			int total,i;
			MIB_CE_ATM_VC_T *pEntry, vc_entity;

			p->Interface[0]=0;
			total = mib_chain_total(MIB_ATM_VC_TBL);
			for( i=0; i<total; i++ )
			{
				pEntry = &vc_entity;
				if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
					continue;
				if(pEntry->TR143UDPEchoItf)
				{
					ifGetName(pEntry->ifIndex, p->Interface, sizeof(p->Interface));
				}
			}
		}else if(itftype<ITF_END)
		{
			strcpy( p->Interface, strItf[itftype] );
			LANDEVNAME2BR0(p->Interface);
		}else
			p->Interface[0]=0;

	}
	return;
}

int UDPEchoConfigStart( struct TR143_UDPEchoConfig *p )
{
	if(!p) return -1;

	if( p->Enable )
	{
		char strPort[16], strAddr[32];
		char *argv[10];
		int  i;

		if(p->UDPPort==0)
		{
			fprintf( stderr, "UDPEchoConfigStart> error p->UDPPort=0\n" );
			return -1;
		}
		sprintf( strPort, "%u", p->UDPPort );
		va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", FW_ADD, (char *)FW_PREROUTING,
			 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
			(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);


		i=0;
		argv[i]=(char *)gUDPEchoServerName;
		i++;
		argv[i]="-port";
		i++;
		argv[i]=strPort;
		i++;
		if( strlen(p->Interface) > 0 )
		{
			argv[i]="-i";
			i++;
			argv[i]=p->Interface;
			i++;
		}
		if( p->SourceIPAddress[0]!=0 ||
			p->SourceIPAddress[1]!=0 ||
			p->SourceIPAddress[2]!=0 ||
			p->SourceIPAddress[3]!=0  )
		{
			struct in_addr *pSIP = (struct in_addr *)p->SourceIPAddress;
			argv[i]="-addr";
			i++;
			sprintf( strAddr, "%s", inet_ntoa( *pSIP ) );
			argv[i]=strAddr;
			i++;
		}
		if( p->EchoPlusEnabled )
		{
			argv[i]="-plus";
			i++;
		}

		argv[i]=NULL;
		do_cmd( gUDPEchoServerName, argv, 0 );

	}

	return 0;
}

int UDPEchoConfigStop( struct TR143_UDPEchoConfig *p )
{
	char strPort[16];
	int pid;
	int status;

	if(!p) return -1;

	sprintf( strPort, "%u", p->UDPPort );
	va_cmd(IPTABLES, 15, 1, ARG_T, "mangle", FW_DEL, (char *)FW_PREROUTING,
		 "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_UDP,
		(char *)FW_DPORT, strPort, "-j", (char *)"MARK", "--set-mark", RMACC_MARK);


	pid = read_pid((char *)gUDPEchoServerPid);
	if (pid >= 1)
	{
		status = kill(pid, SIGTERM);
		if (status != 0)
		{
			printf("Could not kill UDPEchoServer's pid '%d'\n", pid);
			return -1;
		}
	}

	return 0;
}
#endif //_PRMT_TR143_

#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
static const char PROCFS_NAME[] = "/proc/ClientsMonitor";

int proc_write_for_mwband()
{
	FILE *fp;
	unsigned int vInt;
	unsigned char Value;
	unsigned char curbuffer[1024];
	unsigned char *curptr = curbuffer;
	int len =0;

#if !defined(CONFIG_LUNA)
	if ((fp = fopen(PROCFS_NAME, "w")) == NULL)
	{
		printf("Open file %s failed !\n", PROCFS_NAME);
		return -1;
	}
#endif	


	memset(curbuffer, 0, 1024);

	len=sprintf(curptr, "flagLimitOnAll "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_MODE, (void *)&vInt);
//	 printf("\nflagLimitOnAll=%d\n",vInt);
	 len=sprintf(curptr, "%d\n", vInt);
	 curptr+=len;

	 len=sprintf(curptr, "limitOnAll "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_NUMBER, (void *)&vInt);
//	 printf("\nlimitOnAll=%d\n",vInt);
	 len=sprintf(curptr, "%d\n", vInt);
	 curptr+=len;

	 len=sprintf(curptr, "computerLimitEnable "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_PC_ENABLE, (void *)&Value);
//	 printf("\ncomputerLimitEnable=%d\n",Value);
	 len=sprintf(curptr, "%d\n", Value);
	 curptr+=len;

	 len=sprintf(curptr, "limitOnComputer "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_PC_NUM, (void *)&vInt);
//	 printf("\nlimitOnComputer=%d\n",vInt);
	 len=sprintf(curptr, "%d\n", vInt);
	 curptr+=len;

	 len=sprintf(curptr, "cameraLimitEnable "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_CMR_ENABLE, (void *)&Value);
//	 printf("\ncameraLimitEnable=%d\n",Value);
	 len=sprintf(curptr, "%d\n", Value);
	 curptr+=len;

	 len=sprintf(curptr, "limitOnCamera "); // set ip to detect...
	 if(len < 0)
	  goto err;
	 curptr+=len;
	 mib_get( CWMP_CT_MWBAND_CMR_NUM, (void *)&vInt);
//	 printf("\nlimitOnCamera=%d\n",vInt);
	 len=sprintf(curptr, "%d\n", vInt);
	 curptr+=len;

    // set stb to detect...
    len = sprintf(curptr, "stbLimitEnable ");
    if (len < 0)
    {
        goto err;
    }
    curptr += len;
    mib_get(CWMP_CT_MWBAND_STB_ENABLE, (void *)&Value);
    len = sprintf(curptr, "%d\n", Value);
    curptr += len;

    len = sprintf(curptr, "limitOnStb ");
    if (len < 0)
    {
        goto err;
    }
    curptr += len;
    mib_get(CWMP_CT_MWBAND_STB_NUM, (void *)&vInt);
    len = sprintf(curptr, "%d\n", vInt);
    curptr += len;

    // set phone to detect...
    len = sprintf(curptr, "phnLimitEnable ");
    if (len < 0)
    {
        goto err;
    }
    curptr += len;
    mib_get(CWMP_CT_MWBAND_PHN_ENABLE, (void *)&Value);
    len = sprintf(curptr, "%d\n", Value);
    curptr += len;

    len = sprintf(curptr, "limitOnPhn ");
    if (len < 0)
    {
        goto err;
    }
    curptr += len;
    mib_get(CWMP_CT_MWBAND_PHN_NUM, (void *)&vInt);
    len = sprintf(curptr, "%d\n", vInt);
    curptr += len;
#if !defined(CONFIG_LUNA)
	fwrite(curbuffer, 1024, 1, fp);
	fclose(fp);
#endif
#ifdef CONFIG_RTK_RG_INIT
	RTK_RG_AccessWanLimit_Set();
#endif
	return 0;
err:
#if !defined(CONFIG_LUNA)
	fclose(fp);
#endif
	return -1;

}
#endif //CONFIG_CTC_E8_CLIENT_LIMIT
#endif //_CWMP_MIB_

int getDisplayWanName(MIB_CE_ATM_VC_T *pEntry, char* name)
{
	MEDIA_TYPE_T mType;

	if(pEntry==NULL || name==NULL)
		return 0;

	name[0] = '\0';
	mType = MEDIA_INDEX(pEntry->ifIndex);
	if (pEntry->cmode == CHANNEL_MODE_PPPOA) { // ppp0, ...
		snprintf(name, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
	}
	else if (pEntry->cmode == CHANNEL_MODE_PPPOE) { // ppp0_vc0, ...
		if (mType == MEDIA_ATM)
#ifdef CONFIG_RTL_MULTI_PVC_WAN
			snprintf(name, 16, "ppp%u_vc%u_%u", PPP_INDEX(pEntry->ifIndex), VC_MAJOR_INDEX(pEntry->ifIndex), VC_MINOR_INDEX(pEntry->ifIndex));
#else
			snprintf(name, 16, "ppp%u_vc%u", PPP_INDEX(pEntry->ifIndex), VC_INDEX(pEntry->ifIndex));
#endif
#ifdef CONFIG_PTMWAN
		else if (mType == MEDIA_PTM)
			snprintf(name, 16, "ppp%u_%s%u", PPP_INDEX(pEntry->ifIndex), ALIASNAME_MWPTM, PTM_INDEX(pEntry->ifIndex));
#endif /*CONFIG_PTMWAN*/
		else if (mType == MEDIA_ETH)
#ifdef CONFIG_RTL_MULTI_ETH_WAN
			snprintf(name, 16, "ppp%u_%s%u", PPP_INDEX(pEntry->ifIndex), ALIASNAME_MWNAS, ETH_INDEX(pEntry->ifIndex));
#else
			snprintf(name, 16, "ppp%u_%s%u", PPP_INDEX(pEntry->ifIndex), ALIASNAME_NAS, ETH_INDEX(pEntry->ifIndex));
#endif
		else
			return 0;
	}
	else { // vc0 ...
		if (mType == MEDIA_ATM)
#ifdef CONFIG_RTL_MULTI_PVC_WAN
			snprintf(name, 16, "vc%u_%u", VC_MAJOR_INDEX(pEntry->ifIndex), VC_MINOR_INDEX(pEntry->ifIndex));
#else
			snprintf(name, 5, "vc%u", VC_INDEX(pEntry->ifIndex));
#endif
#ifdef CONFIG_PTMWAN
		else if (mType == MEDIA_PTM)
			snprintf(name, 7, "%s%u", ALIASNAME_MWPTM, PTM_INDEX(pEntry->ifIndex));
#endif /*CONFIG_PTMWAN*/
		else if (mType == MEDIA_ETH)
#ifdef CONFIG_RTL_MULTI_ETH_WAN
			snprintf(name, 7, "%s%u", ALIASNAME_MWNAS, ETH_INDEX(pEntry->ifIndex));
#else
			snprintf(name, 5, "%s%u", ALIASNAME_NAS, ETH_INDEX(pEntry->ifIndex));
#endif

		else
			return 0;
	}
	return 1;
}

//jim: to get wan MIB by index... this index is combined index for ppp or vc...
int getWanEntrybyindex(MIB_CE_ATM_VC_T *pEntry, unsigned int ifIndex)
{
	if(pEntry==NULL)
		return -1;
	int mibtotal,i,num=0,totalnum=0;
	MIB_CE_ATM_VC_T Entry;
	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<mibtotal;i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;
		if(Entry.ifIndex == ifIndex)
			break;
	}
	if(i==mibtotal)
		return -1;
	memcpy(pEntry, &Entry, sizeof(Entry));
	return 0;
}

unsigned int getWanIfMapbyMedia(MEDIA_TYPE_T mType)
{
	int mibtotal,i;
	MIB_CE_ATM_VC_T Entry;
	unsigned int ifMap; // high half for PPP bitmap, low half for vc bitmap

	ifMap=0;//init
	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<mibtotal;i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;
		if (Entry.cmode == CHANNEL_MODE_PPPOE)
			ifMap |= (1 << 16) << PPP_INDEX(Entry.ifIndex);	// PPP map
		if(MEDIA_INDEX(Entry.ifIndex) == mType)
			ifMap |= 1 << VC_INDEX(Entry.ifIndex);
	}

	return ifMap;
}

// Kaohj --- first entry
int getWanEntrybyMedia(MIB_CE_ATM_VC_T *pEntry, MEDIA_TYPE_T mType)
{
	if(pEntry==NULL)
		return -1;
	int mibtotal,i,num=0,totalnum=0;
	MIB_CE_ATM_VC_T Entry;
	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0;i<mibtotal;i++)
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;
		if(MEDIA_INDEX(Entry.ifIndex) == mType)
			break;
	}
	if(i==mibtotal)
		return -1;
	memcpy(pEntry, &Entry, sizeof(Entry));
	return i;
}

static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";
#ifdef EMBED
static const char PROC_NET_ATM_BR[] = "/proc/net/atm/br2684";
#ifdef CONFIG_ATM_CLIP
static const char PROC_NET_ATM_CLIP[] = "/proc/net/atm/pvc";
#endif
const char PPPOE_CONF[] = "/var/ppp/pppoe.conf";
const char PPPOA_CONF[] = "/var/ppp/pppoa.conf";
const char PPP_PID[] = "/var/run/spppd.pid";
#endif
#undef FILE_LOCK
int getWanStatus(struct wstatus_info *sEntry, int max)
{
	unsigned int data, data2;
	char	buff[256], tmp1[20], tmp2[20], tmp3[20], tmp4[20];
	char	*temp;
	int in_turn=0, vccount=0, ifcount=0;
	int linkState, dslState=0, ethState=0;
	int i, ifindices[256], br_socket_fd, spid;
	FILE *fp;
#ifdef FILE_LOCK
	struct flock flpoe, flpoa;
	int fdpoe, fdpoa;
#endif
	Modem_LinkSpeed vLs;
	int entryNum;
	MIB_CE_ATM_VC_T tEntry;
	struct wstatus_info vcEntry[MAX_VC_NUM];

	memset(sEntry, 0, sizeof(struct wstatus_info)*max);
#ifdef EMBED
	// get spppd pid
	spid = 0;
	if ((fp = fopen(PPP_PID, "r"))) {
		fscanf(fp, "%d\n", &spid);
		fclose(fp);
	}
	else
		printf("spppd pidfile not exists\n");

	if (spid) {
		struct data_to_pass_st msg;
		snprintf(msg.data, BUF_SIZE, "spppctl pppstatus %d", spid);
		TRACE(STA_SCRIPT, "%s\n", msg.data);
		write_to_pppd(&msg);
	}
#endif
	in_turn = 0;
#ifdef EMBED
#ifdef CONFIG_ATM_BR2684
#ifdef CONFIG_RTL_MULTI_PVC_WAN
	if( WAN_MODE & MODE_ATM )
	{
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<entryNum; i++)
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tEntry))
				continue;

			if(MEDIA_INDEX(tEntry.ifIndex) != MEDIA_ATM)
				continue;
			//czpinging, skipped the entry with Disabled Admin status
			if(tEntry.enable==0)
				continue;

			vcEntry[vccount].ifIndex = tEntry.ifIndex;
			ifGetName(tEntry.ifIndex, vcEntry[vccount].ifname, IFNAMSIZ);
			ifGetName(PHY_INTF(tEntry.ifIndex), vcEntry[vccount].devname, IFNAMSIZ);
			vcEntry[vccount].tvpi = tEntry.vpi;
			vcEntry[vccount].tvci = tEntry.vci;

			switch(tEntry.encap) {
				case ENCAP_VCMUX:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].encaps, "VCMUX");
					break;
				case ENCAP_LLC:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].encaps, "LLC");
					break;
				default:
					break;
			}

			switch(tEntry.cmode) {
				case CHANNEL_MODE_IPOE:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "IPoE");
					break;
				case CHANNEL_MODE_BRIDGE:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "Bridged");
					break;
				case CHANNEL_MODE_PPPOE:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "PPPoE");
					break;
				case CHANNEL_MODE_PPPOA:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "PPPoA");
					break;
				case CHANNEL_MODE_RT1483:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "RT1483");
					break;
				case CHANNEL_MODE_RT1577:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "RT1577");
					break;
				default:
					break;
			}
			strcpy(vcEntry[vccount].vpivci, "---");
			vccount++;
		}
	}
#else
	if (!(fp=fopen(PROC_NET_ATM_BR, "r")))
		printf("%s not exists.\n", PROC_NET_ATM_BR);
	else {
		while ( fgets(buff, sizeof(buff), fp) != NULL ) {
			if (in_turn==0)
				if(sscanf(buff, "%*s%s", tmp1)!=1) {
					printf("Unsuported pvc configuration format\n");
					break;
				}
				else {
					vccount ++;
					tmp1[strlen(tmp1)-1]='\0';
					strcpy(vcEntry[vccount-1].ifname, tmp1);
					strcpy(vcEntry[vccount-1].devname, tmp1);
				}
			else
				if(sscanf(buff, "%*s%s%*s%s", tmp1, tmp2)!=2) {
					printf("Unsuported pvc configuration format\n");
					break;
				}
				else {
					sscanf(tmp1, "0.%u.%u:", &vcEntry[vccount-1].tvpi, &vcEntry[vccount-1].tvci);
					sscanf(tmp2, "%u,", &data);
					strcpy(vcEntry[vccount-1].protocol, "");
					if (data==1 || data == 4)
						strcpy(vcEntry[vccount-1].encaps, "LLC");
					else if (data==0 || data==3)
						strcpy(vcEntry[vccount-1].encaps, "VCMUX");
					if (data==3 || data==4)
						strcpy(vcEntry[vccount-1].protocol, "rt1483");
				}
			in_turn ^= 0x01;
		}
		fclose(fp);
	}
#endif
#endif
#ifdef CONFIG_ATM_CLIP
	if (!(fp=fopen(PROC_NET_ATM_CLIP, "r")))
		printf("%s not exists.\n", PROC_NET_ATM_CLIP);
	else {
		fgets(buff, sizeof(buff), fp);
		while ( fgets(buff, sizeof(buff), fp) != NULL ) {
			char *p = strstr(buff, "CLIP");
			if (p != NULL) {
				if (sscanf(buff, "%*d%u%u%*d%*d%*s%*d%*s%*s%s%s", &data, &data2, tmp1, tmp2) != 4) {
					printf("Unsuported 1577 configuration format\n");
					break;
				}
				else {
					vccount ++;
					sscanf(tmp1, "Itf:%s", tmp3);
					strcpy(vcEntry[vccount-1].ifname, strtok(tmp3, ","));
					strcpy(vcEntry[vccount-1].devname, vcEntry[vccount-1].ifname);
					sscanf(tmp2, "Encap:%s", tmp4);
					strcpy(vcEntry[vccount-1].encaps, strtok(tmp4, "/"));
					strcpy(vcEntry[vccount-1].protocol, "rt1577");
					vcEntry[vccount-1].tvpi = data;
					vcEntry[vccount-1].tvci = data2;
				}
			}
		}
		fclose(fp);
	}
#endif


#ifdef CONFIG_PTMWAN
	if( WAN_MODE & MODE_PTM )
	{
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<entryNum; i++)
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tEntry))
				continue;

			if(MEDIA_INDEX(tEntry.ifIndex) != MEDIA_PTM)
				continue;
			//czpinging, skipped the entry with Disabled Admin status
			if(tEntry.enable==0)
				continue;

			vcEntry[vccount].ifIndex = tEntry.ifIndex;
			ifGetName(tEntry.ifIndex, vcEntry[vccount].ifname, IFNAMSIZ);
			ifGetName(PHY_INTF(tEntry.ifIndex), vcEntry[vccount].devname, IFNAMSIZ);
			strcpy(vcEntry[vccount].encaps, "---");
			switch(tEntry.cmode) {
				case CHANNEL_MODE_IPOE:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "IPoE");
					break;
				case CHANNEL_MODE_BRIDGE:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "Bridged");
					break;
				case CHANNEL_MODE_PPPOE:
					strcpy(vcEntry[vccount].protocol, "PPPoE");
					break;
				default:
					break;
			}
			strcpy(vcEntry[vccount].vpivci, "---");
			vccount++;
		}
	}
#endif // CONFIG_PTMWAN


#ifdef CONFIG_ETHWAN
	if( WAN_MODE & MODE_Ethernet )
	{
#ifndef CONFIG_RTL_MULTI_ETH_WAN
		entryNum = getWanEntrybyMedia(&tEntry, MEDIA_ETH);
		if (entryNum>=0) {
			vcEntry[vccount].ifIndex = tEntry.ifIndex;
			ifGetName(tEntry.ifIndex, vcEntry[vccount].ifname, IFNAMSIZ);
			ifGetName(PHY_INTF(tEntry.ifIndex), vcEntry[vccount].devname, IFNAMSIZ);
			strcpy(vcEntry[vccount].encaps, "---");
			switch(tEntry.cmode) {
				case CHANNEL_MODE_IPOE:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "IPoE");
					break;
				case CHANNEL_MODE_BRIDGE:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "Bridged");
					break;
				case CHANNEL_MODE_PPPOE:
					strcpy(vcEntry[vccount].protocol, "PPPoE");
					break;
				default:
					break;
			}
			strcpy(vcEntry[vccount].vpivci, "---");
			vccount++;
		}
#else
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<entryNum; i++)
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&tEntry))
				continue;

			if(MEDIA_INDEX(tEntry.ifIndex) == MEDIA_ATM)
				continue;
			#ifdef CONFIG_PTMWAN
			if(MEDIA_INDEX(tEntry.ifIndex) == MEDIA_PTM)
				continue;
			#endif /*CONFIG_PTMWAN*/

			//czpinging, skipped the entry with Disabled Admin status
			if(tEntry.enable==0)
				continue;

			vcEntry[vccount].ifIndex = tEntry.ifIndex;
			ifGetName(tEntry.ifIndex, vcEntry[vccount].ifname, IFNAMSIZ);
			ifGetName(PHY_INTF(tEntry.ifIndex), vcEntry[vccount].devname, IFNAMSIZ);
			strcpy(vcEntry[vccount].encaps, "---");
			switch(tEntry.cmode) {
				case CHANNEL_MODE_IPOE:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "IPoE");
					break;
				case CHANNEL_MODE_BRIDGE:
					memset(vcEntry[vccount].protocol, 0, 10);
					strcpy(vcEntry[vccount].protocol, "Bridged");
					break;
				case CHANNEL_MODE_PPPOE:
					strcpy(vcEntry[vccount].protocol, "PPPoE");
					break;
				default:
					break;
			}
			strcpy(vcEntry[vccount].vpivci, "---");
			vccount++;
		}
#endif
	}
#endif // CONFIG_ETHWAN

#ifdef FILE_LOCK
	// file locking
	fdpoe = open(PPPOE_CONF, O_RDWR);
	fdpoa = open(PPPOA_CONF, O_RDWR);
	if ((fdpoe != -1) && (fdpoa != -1)) {
		flpoe.l_type = flpoa.l_type = F_RDLCK;
		flpoe.l_whence = flpoa.l_whence = SEEK_SET;
		flpoe.l_start = flpoa.l_start = 0;
		flpoe.l_len = flpoa.l_len = 0;
		flpoe.l_pid = flpoa.l_pid = getpid();
		if (fcntl(fdpoe, F_SETLKW, &flpoe) == -1)
			printf("pppoe read lock failed\n");
		if (fcntl(fdpoa, F_SETLKW, &flpoa) == -1)
			printf("pppoa read lock failed\n");
	}
#endif
	if (!(fp=fopen(PPPOA_CONF, "r")))
		printf("%s not exists.\n", PPPOA_CONF);
	else {
		fgets(buff, sizeof(buff), fp);
		while ( fgets(buff, sizeof(buff), fp) != NULL )
			if (sscanf(buff, "%s%u%u%*s%s%*s%*d%*d%*d%s%s", tmp1, &data, &data2, tmp2, tmp3, tmp4) != 6) {
				printf("Unsuported pppoa configuration format\n");
				break;
			}
			else {
				ifcount ++;
				// ifIndex --- ppp index(no vc index)
				sEntry[ifcount-1].ifIndex = TO_IFINDEX(MEDIA_ATM, tmp1[3]-'0', DUMMY_VC_INDEX);
				strcpy(sEntry[ifcount-1].ifname, tmp1);
				strcpy(sEntry[ifcount-1].encaps, tmp2);
				strcpy(sEntry[ifcount-1].protocol, "PPPoA");
				sEntry[ifcount-1].tvpi = data;
				sEntry[ifcount-1].tvci = data2;
				sprintf(sEntry[ifcount-1].vpivci, "%u/%u", sEntry[ifcount-1].tvpi, sEntry[ifcount-1].tvci);
				strcpy(sEntry[ifcount-1].uptime, tmp3);
				strcpy(sEntry[ifcount-1].totaluptime, tmp4);
			}
		fclose(fp);
	}

	if (!(fp=fopen(PPPOE_CONF, "r")))
		printf("%s not exists.\n", PPPOE_CONF);
	else {
		fgets(buff, sizeof(buff), fp);
		while ( fgets(buff, sizeof(buff), fp) != NULL )
			if(sscanf(buff, "%s%s%*s%*s%*s%s%s", tmp1, tmp2, tmp3, tmp4) != 4) {
				printf("Unsuported pppoe configuration format\n");
				break;
			}
			else
				for (i=0; i<vccount; i++)
#ifdef CONFIG_RTL_MULTI_PVC_WAN
					if (strcmp(vcEntry[i].ifname, tmp1) == 0)
#else
					if (strcmp(vcEntry[i].devname, tmp2) == 0)
#endif
					{
						ifcount ++;
						// ifIndex --- ppp index + vc index
						if (!strncmp(vcEntry[i].devname,"vc",2))
						{
#ifdef CONFIG_RTL_MULTI_PVC_WAN
							sEntry[ifcount-1].ifIndex = TO_IFINDEX(MEDIA_ATM, tmp1[3]-'0', ((((vcEntry[i].devname[2]-'0') << 4) & 0xf0) | ((vcEntry[i].devname[4]-'0') & 0x0f)) );
#else
							sEntry[ifcount-1].ifIndex = TO_IFINDEX(MEDIA_ATM, tmp1[3]-'0', vcEntry[i].devname[2]-'0');
#endif
							sEntry[ifcount-1].tvpi = vcEntry[i].tvpi;
							sEntry[ifcount-1].tvci = vcEntry[i].tvci;
							sprintf(sEntry[ifcount-1].vpivci, "%u/%u", sEntry[ifcount-1].tvpi, sEntry[ifcount-1].tvci);
							//printf("***** sEntry[ifcount-1].ifIndex=0x%x\n", sEntry[ifcount-1].ifIndex);
						}
#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
						else {
							sEntry[ifcount-1].ifIndex = vcEntry[i].ifIndex;
							strcpy(sEntry[ifcount-1].vpivci, "---");
						}
#endif
						strcpy(sEntry[ifcount-1].ifname, tmp1);
#ifdef CONFIG_RTL_MULTI_PVC_WAN
						strcpy(sEntry[ifcount-1].devname, tmp2);
#else
						strcpy(sEntry[ifcount-1].devname, vcEntry[i].devname);
#endif
						strcpy(sEntry[ifcount-1].encaps, vcEntry[i].encaps);
						strcpy(sEntry[ifcount-1].protocol, "PPPoE");
						strcpy(sEntry[ifcount-1].uptime, tmp3);
						strcpy(sEntry[ifcount-1].totaluptime, tmp4);
					}
		fclose(fp);
	}
#ifdef FILE_LOCK
	// file unlocking
	if ((fdpoe != -1) && (fdpoa != -1)) {
		flpoe.l_type = flpoa.l_type = F_UNLCK;
		if (fcntl(fdpoe, F_SETLK, &flpoe) == -1)
			printf("pppoe read unlock failed\n");
		if (fcntl(fdpoa, F_SETLK, &flpoa) == -1)
			printf("pppoa read unlock failed\n");
		close(fdpoe);
		close(fdpoa);
	}
#endif
	for (i=0; i<vccount; i++) {
		int j, vcfound=0;
		for (j=0; j<ifcount; j++) {
#ifdef CONFIG_RTL_MULTI_PVC_WAN
			if (strcmp(vcEntry[i].ifname, sEntry[j].ifname) == 0)  // PPPoE-used device
#else
			if (strcmp(vcEntry[i].devname, sEntry[j].devname) == 0)  // PPPoE-used device
#endif
			{
				vcfound = 1;
				break;
			}
		}
		if (!vcfound) {	// VC not used for PPPoE, add to list
			ifcount ++;
			// ifIndex --- vc index (no ppp index)
			if (!strncmp(vcEntry[i].devname,"vc",2)) {
#ifdef CONFIG_RTL_MULTI_PVC_WAN
				sEntry[ifcount-1].ifIndex = TO_IFINDEX(MEDIA_ATM, DUMMY_PPP_INDEX, ((((vcEntry[i].devname[2]-'0') << 4) & 0xf0) | ((vcEntry[i].devname[4]-'0') & 0x0f)) );
#else
				sEntry[ifcount-1].ifIndex = TO_IFINDEX(MEDIA_ATM, DUMMY_PPP_INDEX, vcEntry[i].ifname[2]-'0');
#endif
				sEntry[ifcount-1].tvpi = vcEntry[i].tvpi;
				sEntry[ifcount-1].tvci = vcEntry[i].tvci;
				sprintf(sEntry[ifcount-1].vpivci, "%u/%u", sEntry[ifcount-1].tvpi, sEntry[ifcount-1].tvci);
			}
#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
			else {
				sEntry[ifcount-1].ifIndex = vcEntry[i].ifIndex;
				strcpy(sEntry[ifcount-1].vpivci, "---");
			}
#endif
			strcpy(sEntry[ifcount-1].ifname, vcEntry[i].ifname);
			strcpy(sEntry[ifcount-1].encaps, vcEntry[i].encaps);
			strcpy(sEntry[ifcount-1].protocol, vcEntry[i].protocol);
		}
	}

#endif

#ifdef CONFIG_DEV_xDSL
	// check for xDSL link
	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		dslState = 0;
	else
		dslState = 1;
#endif
#if defined(CONFIG_ETHWAN) || defined(CONFIG_PTMWAN)
	// todo
	ethState = 1;
#endif

	if (ifcount > max)
		printf("WARNNING! status list overflow(%d).\n", ifcount);


	for (i=0; i<ifcount; i++) {
		struct in_addr inAddr;
		int flags;
		int totalNum, k;
		MIB_CE_ATM_VC_T entry;
		MEDIA_TYPE_T mType;

#ifdef EMBED
		// Kaohj --- interface name to be displayed
		totalNum = mib_chain_total(MIB_ATM_VC_TBL);

		for(k=0; k<totalNum; k++)
		{
			mib_chain_get(MIB_ATM_VC_TBL, k, (void *)&entry);

			if (sEntry[i].ifIndex == entry.ifIndex) {
				getDisplayWanName(&entry, sEntry[i].ifDisplayName);
				sEntry[i].cmode = entry.cmode;
#ifdef CONFIG_IPV6
				sEntry[i].ipver = entry.IpProtocol;
#endif
				break;
			}

		}
		if (getInAddr( sEntry[i].ifname, IP_ADDR, (void *)&inAddr) == 1)
		{
			temp = inet_ntoa(inAddr);
			if (getInFlags( sEntry[i].ifname, &flags) == 1)
				if ((strcmp(temp, "10.0.0.1") == 0) && flags & IFF_POINTOPOINT)	// IP Passthrough or IP unnumbered
					strcpy(sEntry[i].ipAddr, STR_UNNUMBERED);
				else if (strcmp(temp, "64.64.64.64") == 0)
					strcpy(sEntry[i].ipAddr, "");
				else
					strcpy(sEntry[i].ipAddr, temp);
		}
		else
#endif
			strcpy(sEntry[i].ipAddr, "");

#ifdef EMBED
		if (getInAddr( sEntry[i].ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
		{
			temp = inet_ntoa(inAddr);
			if (strcmp(temp, "10.0.0.2") == 0)
				strcpy(sEntry[i].remoteIp, STR_UNNUMBERED);
			else if (strcmp(temp, "64.64.64.64") == 0)
				strcpy(sEntry[i].remoteIp, "");
			else
				strcpy(sEntry[i].remoteIp, temp);
			if (getInFlags( sEntry[i].ifname, &flags) == 1)
				if (flags & IFF_BROADCAST) {
					unsigned char value[32];
					snprintf(value, 32, "%s.%s", (char *)MER_GWINFO, sEntry[i].ifname);
					if (fp = fopen(value, "r")) {
						fscanf(fp, "%s\n", sEntry[i].remoteIp);
						//strcpy(sEntry[i].protocol, "mer1483");
						fclose(fp);
					}
					else
						strcpy(sEntry[i].remoteIp, "");
				}
		}
		else
#endif
			strcpy(sEntry[i].remoteIp, "");

		if (!strcmp(sEntry[i].protocol, ""))
		{
			//get channel mode
			switch(sEntry[i].cmode) {
			case CHANNEL_MODE_IPOE:
				memset(sEntry[i].protocol, 0, 10);
				strcpy(sEntry[i].protocol, "mer1483");
				break;
			case CHANNEL_MODE_BRIDGE:
				memset(sEntry[i].protocol, 0, 10);
				strcpy(sEntry[i].protocol, "br1483");
				break;
			default:
				break;
			}
		}

		mType = MEDIA_INDEX(sEntry[i].ifIndex);
		if (mType == MEDIA_ATM)
			linkState = dslState;
		#ifdef CONFIG_PTMWAN
		else if (mType == MEDIA_PTM)
			linkState = dslState && ethState;//???
		#endif /*CONFIG_PTMWAN*/
		else if (mType == MEDIA_ETH)
			linkState = ethState;
		else
			linkState = 0;
		sEntry[i].link_state = linkState;
		// set status flag
		if (getInFlags( sEntry[i].ifname, &flags) == 1)
		{
			if (flags & IFF_UP) {
				if (!linkState) {
					sEntry[i].strStatus = (char *)IF_DOWN;
					sEntry[i].itf_state = 0;
				}
				else {
					if (sEntry[i].cmode == CHANNEL_MODE_BRIDGE) {
						sEntry[i].strStatus = (char *)IF_UP;
						sEntry[i].itf_state = 1;
					}
					else
						if (getInAddr(sEntry[i].ifname, IP_ADDR, (void *)&inAddr) == 1) {
							temp = inet_ntoa(inAddr);
							if (strcmp(temp, "64.64.64.64")) {
								sEntry[i].strStatus = (char *)IF_UP;
								sEntry[i].itf_state = 1;															}
							else {
								sEntry[i].strStatus = (char *)IF_DOWN;
								sEntry[i].itf_state = 0;
							}
						}
						else {
							sEntry[i].strStatus = (char *)IF_DOWN;
							sEntry[i].itf_state = 0;
						}
				}
			}
			else {
				sEntry[i].strStatus = (char *)IF_DOWN;
				sEntry[i].itf_state = 0;
			}
		}
		else {
			sEntry[i].strStatus = (char *)IF_NA;
			sEntry[i].itf_state = -1;
		}

		if (sEntry[i].cmode == CHANNEL_MODE_PPPOE || sEntry[i].cmode == CHANNEL_MODE_PPPOA) {
			if (sEntry[i].itf_state <= 0) {
				sEntry[i].ipAddr[0] = '\0';
				sEntry[i].remoteIp[0] = '\0';
			}
			if (entry.pppCtype == CONNECT_ON_DEMAND && entry.pppIdleTime != 0)
				sEntry[i].pppDoD = 1;
			else
				sEntry[i].pppDoD = 0;
		}

	}
	return ifcount;
}

int isValidMedia(unsigned int ifIndex)
{
	MEDIA_TYPE_T mType;

	mType = MEDIA_INDEX(ifIndex);
	if (1
	#ifdef CONFIG_RTL8672_SAR
		&& mType!=MEDIA_ATM
	#endif
	#ifdef CONFIG_PTMWAN
		&& mType!=MEDIA_PTM
	#endif /*CONFIG_PTMWAN*/
	#ifdef CONFIG_ETHWAN
		&& mType!=MEDIA_ETH
	#endif
	)
		return 0;
	return 1;
}

// Kaohj -- specific for pvc channel
// map: bit map of used interface, ppp index (0~15) is mapped into high 16 bits,
// while vc index (0~15) is mapped into low 16 bits.
// while major vc index (8 ~ 15) is mapped into hight 8 bits of vc index.
// while minor vc index (0 ~ 7) is mapped into low 8 bits of vc index.
// return: interface index, byte1 for PPP index and byte0 for vc index.
//		0xefff(NA_PPP): PPP not available
//		0xffff(NA_VC) : vc not available
unsigned int if_find_index(int cmode, unsigned int map)
{
	int i;
	unsigned int index, vc_idx, ppp_idx;
#ifdef CONFIG_RTL_MULTI_PVC_WAN
	int j;
#endif

	// find the first available vc index (mpoa interface)
	i = 0;
#ifndef CONFIG_RTL_MULTI_PVC_WAN
	for (i=0; i<MAX_VC_NUM; i++)
	{
		if (!((map>>i) & 1))
			break;
	}
#else
	for (i=0; i<MAX_VC_NUM; i++)
	{
		if (!(((map>>8)>>i) & 1))
			break;
	}

	j=0;
	for (j=0; j<MAX_VC_VIRTUAL_NUM; j++)
	{
		if (!((map>>j) & 1))
			break;
	}
#endif

#ifndef CONFIG_RTL_MULTI_PVC_WAN
	if (i != MAX_VC_NUM)
		vc_idx = i;
	else
		return NA_VC;
#else
	// Mason Yu
	//printf("if_find_index: major=%d, minor=%d\n", i, j);
	if (i != MAX_VC_NUM && j != MAX_VC_VIRTUAL_NUM) {
		vc_idx = (((i << 4) & 0xf0) | (j & 0x0f));
	}
	else
		return NA_VC;
#endif

	if (cmode == CHANNEL_MODE_PPPOE || cmode == CHANNEL_MODE_PPPOA)
	{
		// find an available PPP index
		map >>= 16;
		i = 0;
		while (map & 1)
		{
			map >>= 1;
			i++;
		}
		ppp_idx = i;
		if (ppp_idx<=(MAX_PPP_NUM-1))
			index = TO_IFINDEX(MEDIA_ATM, ppp_idx, vc_idx);
		else
			return NA_PPP;

		if (cmode == CHANNEL_MODE_PPPOA)
			//index |= 0x0f;	// PPPoA doesn't use mpoa interface, set to 0x0f (don't care)
			index = TO_IFINDEX(MEDIA_ATM, ppp_idx, DUMMY_VC_INDEX);
	}
	else
	{
		// don't care the PPP index
		//index |= 0xf0;
		index = TO_IFINDEX(MEDIA_ATM, DUMMY_PPP_INDEX, vc_idx);
	}
	return index;
}

#ifdef CONFIG_ETHWAN
int init_ethwan_config(MIB_CE_ATM_VC_T *pEntry)
{
	if (!pEntry)
		return 0;
	memset(pEntry, 0, sizeof(MIB_CE_ATM_VC_T));
	pEntry->ifIndex = TO_IFINDEX(MEDIA_ETH, DUMMY_PPP_INDEX, 0);
	pEntry->cmode = CHANNEL_MODE_BRIDGE;
	pEntry->enable = 1;
	return 1;
}
#endif
#ifdef CONFIG_PTMWAN
int init_ptmwan_config(MIB_CE_ATM_VC_T *pEntry)
{
	if (!pEntry)
		return 0;
	memset(pEntry, 0, sizeof(MIB_CE_ATM_VC_T));
	pEntry->ifIndex = TO_IFINDEX(MEDIA_PTM, DUMMY_PPP_INDEX, 0);
	pEntry->cmode = CHANNEL_MODE_BRIDGE;
	pEntry->enable = 1;
	return 1;
}
#endif /*CONFIG_PTMWAN*/
#ifdef CONFIG_HWNAT
int send_to_hwnat(hwnat_ioctl_cmd hifr)
{
	struct ifreq    ifr;
	int fd = -1;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if(fd< 0){
		printf("Error!Socket create fail.\n");
		return 0;
	}

	strncpy(ifr.ifr_name, ELANVIF[0], IFNAMSIZ);
	ifr.ifr_data = (void *)&hifr;

	if (ioctl(fd, RTL819X_IOCTL_HWNAT, &ifr) < 0) {
		printf("Error ioctl(RTL819X_IOCTL_HWNAT) in boa\n");
		close(fd);
		return 0;
	}
	close(fd);
	return 0;
}
#endif

//star: to get ppp index for wanname
static int getpppindex(MIB_CE_ATM_VC_T * pEntry)
{
	int ret = -1;
	int mibtotal, i, num = 0, totalnum = 0;
	unsigned int pppindex, tmpindex;
	MIB_CE_ATM_VC_T Entry;

	if (pEntry->cmode != CHANNEL_MODE_PPPOE && pEntry->cmode != CHANNEL_MODE_PPPOA)
		return ret;

	pppindex = PPP_INDEX(pEntry->ifIndex);
	if (pppindex == DUMMY_PPP_INDEX)
		return ret;

	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < mibtotal; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			continue;
		if (Entry.cmode != CHANNEL_MODE_PPPOE && Entry.cmode != CHANNEL_MODE_PPPOA)
			continue;
		tmpindex = PPP_INDEX(Entry.ifIndex);
		if (tmpindex == DUMMY_PPP_INDEX)
			continue;
		if (Entry.vpi == pEntry->vpi && Entry.vci == pEntry->vci) {
			totalnum++;
			if (tmpindex < pppindex)
				num++;
		}
	}

	if (totalnum > 1)
		ret = num;

	return ret;

}

int generateWanName(MIB_CE_ATM_VC_T *entry, char* wanname)
{
	char vpistr[6];
	char vcistr[6];
	char vid[16];
	int i, mibtotal;
	MIB_CE_ATM_VC_T tmpEntry;
	MEDIA_TYPE_T mType;

	mibtotal = mib_chain_total(MIB_ATM_VC_TBL);
	for(i=0; i< mibtotal; i++)
	{
		mib_chain_get(MIB_ATM_VC_TBL, i, &tmpEntry);
		if(tmpEntry.ifIndex == entry->ifIndex
		#ifdef BR_ROUTE_ONEPVC
			&&tmpEntry.cmode == entry->cmode
		#endif
			)
			break;
	}
	if(i==mibtotal)
		return -1;
	i++;

	mType = MEDIA_INDEX(entry->ifIndex);
	sprintf(wanname, "%d_", i);
	if(entry==NULL || wanname ==NULL)
		return -1;
	memset(vpistr, 0, sizeof(vpistr));
	memset(vcistr, 0, sizeof(vcistr));
	if (entry->applicationtype&X_CT_SRV_TR069)
		strcat(wanname, "TR069_");
#ifdef CONFIG_USER_RTK_VOIP
	if (entry->applicationtype&X_CT_SRV_VOICE)
		strcat(wanname, "VOICE_");
#endif
	if (entry->applicationtype&X_CT_SRV_INTERNET)
		strcat(wanname, "INTERNET_");
	if (entry->applicationtype&X_CT_SRV_OTHER)
		strcat(wanname, "Other_");
	if(entry->cmode == CHANNEL_MODE_BRIDGE)
		strcat(wanname, "B_");
	else
		strcat(wanname, "R_");
	if ((mType == MEDIA_ETH) || (mType == MEDIA_PTM)) {
		strcat(wanname, "VID_");
		if(entry->vlan==1){
			sprintf(vid, "%d", entry->vid);
			strcat(wanname, vid);
		}
	}
	else if (mType == MEDIA_ATM) {
		sprintf(vpistr, "%d", entry->vpi);
		sprintf(vcistr, "%d", entry->vci);
		strcat(wanname, vpistr);
		strcat(wanname, "_");
		strcat(wanname, vcistr);
	}
	return 0;
}

int setWanName(char* str, int applicationtype)
{
	if (applicationtype&X_CT_SRV_TR069)
		strcat(str, "TR069_");
#ifdef CONFIG_USER_RTK_VOIP
	if (applicationtype&X_CT_SRV_VOICE)
		strcat(str, "VOICE_");
#endif
	if (applicationtype&X_CT_SRV_INTERNET)
		strcat(str, "INTERNET_");
	if (applicationtype&X_CT_SRV_OTHER)
		strcat(str, "Other_");
	if (strlen(str)) // remove tailing '_'
		str[strlen(str)-1]='\0';
}

int getWanName(MIB_CE_ATM_VC_T * pEntry, char *name)
{
	if (pEntry == NULL || name == NULL)
		return 0;
#ifdef _CWMP_MIB_
	if (*(pEntry->WanName))
		strcpy(name, pEntry->WanName);
	else
#endif
	{			//if not set by ACS. then generate automaticly.
		generateWanName(pEntry, name);
	}
	return 1;
}

#define DEFDATALEN	56
#define PINGCOUNT 3
#define PINGINTERVAL	1	/* second */
#define MAXWAIT		5

static struct sockaddr_in pingaddr;
static int pingsock = -1;
static long ntransmitted = 0, nreceived = 0, nrepeats = 0;
static int myid = 0;
static int finished = 0;

int create_icmp_socket(void)
{
	struct protoent *proto;
	int sock;

	proto = getprotobyname("icmp");
	/* if getprotobyname failed, just silently force
	 * proto->p_proto to have the correct value for "icmp" */
	if ((sock = socket(AF_INET, SOCK_RAW,
			(proto ? proto->p_proto : 1))) < 0) {        /* 1 == ICMP */
		printf("cannot create raw socket\n");
	}

	return sock;
}

int in_cksum(unsigned short *buf, int sz)
{
	int nleft = sz;
	int sum = 0;
	unsigned short *w = buf;
	unsigned short ans = 0;

	while (nleft > 1) {
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1) {
		*(unsigned char *) (&ans) = *(unsigned char *) w;
		sum += ans;
	}

	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ans = ~sum;
	return (ans);
}

static void pingfinal()
{
	finished = 1;
}

static void sendping()
{
	struct icmp *pkt;
	int c;
	char packet[DEFDATALEN + 8];

	pkt = (struct icmp *) packet;
	pkt->icmp_type = ICMP_ECHO;
	pkt->icmp_code = 0;
	pkt->icmp_cksum = 0;
	pkt->icmp_seq = ntransmitted++;
	pkt->icmp_id = myid;
	pkt->icmp_cksum = in_cksum((unsigned short *) pkt, sizeof(packet));

	c = sendto(pingsock, packet, sizeof(packet), 0,
			   (struct sockaddr *) &pingaddr, sizeof(struct sockaddr_in));

	if (c < 0 || c != sizeof(packet)) {
		ntransmitted--;
		finished = 1;
		printf("sock: sendto fail !");
		return;
	}

	signal(SIGALRM, sendping);
	if (ntransmitted < PINGCOUNT) {	/* schedule next in 1s */
		alarm(PINGINTERVAL);
	} else {	/* done, wait for the last ping to come back */
		signal(SIGALRM, pingfinal);
		alarm(MAXWAIT);
	}
}

int utilping(char *str)
{
//	char *submitUrl;
	char tmpBuf[100];
	int c;
	struct hostent *h;
	struct icmp *pkt;
	struct iphdr *iphdr;
	char packet[DEFDATALEN + 8];
	int rcvdseq, ret=0;
	fd_set rset;
	struct timeval tv;

	if (str[0]) {
		if ((pingsock = create_icmp_socket()) < 0) {
			perror("socket");
			snprintf(tmpBuf, 100, "ping: socket create error");
			goto setErr_ping;
		}

		memset(&pingaddr, 0, sizeof(struct sockaddr_in));
		pingaddr.sin_family = AF_INET;

		if ((h = gethostbyname(str)) == NULL) {
			//herror("ping: ");
			//snprintf(tmpBuf, 100, "ping: %s: %s", str, hstrerror(h_errno));
			goto setErr_ping;
		}

		if (h->h_addrtype != AF_INET) {
			//strcpy(tmpBuf, "unknown address type; only AF_INET is currently supported.");
			goto setErr_ping;
		}

		memcpy(&pingaddr.sin_addr, h->h_addr, sizeof(pingaddr.sin_addr));

		printf("PING %s (%s): %d data bytes\n",
		   h->h_name,inet_ntoa(*(struct in_addr *) &pingaddr.sin_addr.s_addr),DEFDATALEN);

		myid = getpid() & 0xFFFF;
		ntransmitted = nreceived = nrepeats = 0;
		finished = 0;
		rcvdseq=ntransmitted-1;
		FD_ZERO(&rset);
		FD_SET(pingsock, &rset);
		/* start the ping's going ... */
		sendping();

		/* listen for replies */
		while (1) {
			struct sockaddr_in from;
			socklen_t fromlen = (socklen_t) sizeof(from);
			int c, hlen, dupflag;

			if (finished)
				break;

			tv.tv_sec = 1;
			tv.tv_usec = 0;

			if (select(pingsock+1, &rset, NULL, NULL, &tv) > 0) {
				if ((c = recvfrom(pingsock, packet, sizeof(packet), 0,
								  (struct sockaddr *) &from, &fromlen)) < 0) {

					printf("sock: recvfrom fail !");
					continue;
				}
			}
			else // timeout or error
				continue;

			if (c < DEFDATALEN+ICMP_MINLEN)
				continue;

			iphdr = (struct iphdr *) packet;
			hlen = iphdr->ihl << 2;
			pkt = (struct icmp *) (packet + hlen);	/* skip ip hdr */
			if (pkt->icmp_id != myid) {
//				printf("not myid\n");
				continue;
			}
			if (pkt->icmp_type == ICMP_ECHOREPLY) {
				++nreceived;
				if (pkt->icmp_seq == rcvdseq) {
					// duplicate
					++nrepeats;
					--nreceived;
					dupflag = 1;
				} else {
					rcvdseq = pkt->icmp_seq;
					dupflag = 0;
					if (nreceived < PINGCOUNT)
					// reply received, send another immediately
						sendping();
				}
				printf("%d bytes from %s: icmp_seq=%u", c,
					   inet_ntoa(*(struct in_addr *) &from.sin_addr.s_addr),
					   pkt->icmp_seq);
				if (dupflag) {
					printf(" (DUP!)");
				}
				printf("\n");
			}
			if (nreceived >= PINGCOUNT) {
				ret = 1;
				break;
			}
		}
		FD_CLR(pingsock, &rset);
		close(pingsock);
		pingsock = -1;
	}
	printf("\n--- ping statistics ---\n");
	printf("%ld packets transmitted, ", ntransmitted);
	printf("%ld packets received\n\n", nreceived);
	if (nrepeats)
		printf("%ld duplicates, ", nrepeats);
	printf("\n");
	return ret;
setErr_ping:
	if (pingsock >= 0) {
		close(pingsock);
		pingsock = -1;
	}
	printf("Ping error!!\n\n");
	return ret;
}

#ifdef CONFIG_DEV_xDSL
int testOAMLookback(MIB_CE_ATM_VC_Tp pEntry, unsigned char scope, unsigned char type)
{
#ifdef EMBED
	int skfd;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	time_t  curTime, preTime = 0;

	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}

	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));
	lbReq.Scope = scope;
	lbReq.vpi = pEntry->vpi;
	if (type == 5)
		lbReq.vci = pEntry->vci;
	else if (type == 4) {
		if (scope == 0)	// Segment
			lbReq.vci = 3;
		else if (scope == 1)	// End-to-end
			lbReq.vci = 4;
	}
	memset(lbReq.LocID, 0xff, 16);	// Loopback Location ID
	mysio.number = 0;	// ATM interface number
	mysio.arg = (void *)&lbReq;
	// Start the loopback test
	if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_START failed !");
		close(skfd);
		return -1;
	}

	finished = 0;
	time(&preTime);
	// Query the loopback status
	mysio.arg = (void *)&lbState;
	lbState.vpi = pEntry->vpi;
	if (type == 5)
		lbState.vci = pEntry->vci;
	else if (type == 4) {
		if (scope == 0)	// Segment
			lbState.vci = 3;
		else if (scope == 1)	// End-to-end
			lbState.vci = 4;
	}
	lbState.Tag = lbReq.Tag;

	while (1)
	{
		time(&curTime);
		if (curTime - preTime >= MAXWAIT)
		{
			//printf("OAMLB timeout!\n");
			finished = 1;
			break;	// break for timeout
		}

		if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0) {
			perror("ioctl: ATM_OAM_LB_STATUS failed !");
			mysio.arg = (void *)&lbReq;
			ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
			close(skfd);
			return -1;
		}

		if (lbState.count[0] > 0)
		{
			break;	// break for loopback success
		}
	}

	mysio.arg = (void *)&lbReq;
	// Stop the loopback test
	if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_STOP failed !");
		close(skfd);
		return -1;
	}
	close(skfd);

	if (!finished) {
		printf("\n--- Loopback cell received successfully ---\n");
		return 1;	// successful
	}
	else {
		printf("\n--- Loopback failed ---\n");
		return 0;	// failed
	}
#else
	return 1;
#endif
}
#endif	//CONFIG_DEV_xDSL

int defaultGWAddr(char *gwaddr)
{
	char buff[256], ifname[16];
	int flgs;
	//unsigned long int g;
	//struct in_addr gw;
	struct in_addr gw, dest, mask;
	FILE *fp;

	if (!(fp=fopen("/proc/net/route", "r"))) {
		printf("Error: cannot open /proc/net/route - continuing...\n");
		return -1;
	}

	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		//if (sscanf(buff, "%s%lx%*lx%X%", ifname, &g, &flgs) != 3) {
		if (sscanf(buff, "%s%x%x%x%*d%*d%*d%x", ifname, &dest, &gw, &flgs, &mask) != 5) {
			printf("Unsupported kernel route format\n");
			fclose(fp);
			return -1;
		}
		if(flgs & RTF_UP) {
			// default gateway
			if (dest.s_addr == 0 && mask.s_addr == 0) {
				if (gw.s_addr != 0) {
					sprintf(gwaddr, "%s", inet_ntoa(gw));
					fclose(fp);
					return 0;
				}
				else {
					if (getInAddr(ifname, DST_IP_ADDR, (void *)&gw) == 1)
						if (gw.s_addr != 0) {
							sprintf(gwaddr, "%s", inet_ntoa(gw));
							fclose(fp);
							return 0;
						}
				}
			}
		}
		/*if((g == 0) && (flgs & RTF_UP)) {
			if (getInAddr(ifname, DST_IP_ADDR, (void *)&gw) == 1)
				if (gw.s_addr != 0) {
					sprintf(gwaddr, "%s", inet_ntoa(gw));
					fclose(fp);
					return 0;
				}
		}
		if (sscanf(buff, "%*s%*lx%lx%X%", &g, &flgs) != 2) {
			printf("Unsupported kernel route format\n");
			fclose(fp);
			return -1;
		}
		if(flgs & RTF_UP) {
			gw.s_addr = g;
			if (gw.s_addr != 0) {
				sprintf(gwaddr, "%s", inet_ntoa(gw));
				fclose(fp);
				return 0;
			}
		}*/
	}
	fclose(fp);
	return -1;
}

int pdnsAddr(char *dnsaddr)
{

	FILE *fp;
	char buff[256];
	if ( (fp = fopen("/var/resolv.conf", "r")) == NULL ) {
		printf("Unable to open resolver file\n");
		return -1;
	}

	fgets(buff, sizeof(buff), fp);
	if (sscanf(buff, "nameserver %s", dnsaddr) != 1) {
		printf("Unsupported kernel route format\n");
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return 0;
}

int getNameServers(char *buf)
{
	FILE *fp;
	char line[128], addr[64];
	int count = 0;

	buf[0] = '\0';
	if ((fp = fopen(RESOLV, "r")) == NULL)
		return -1;

	while (fgets(line, sizeof(line), fp) != NULL) {
		if (sscanf(line, "nameserver %s", addr) != 1)
			continue;

		if (count == 0) {
			sprintf(buf, "%s", addr);
		} else {
			sprintf(line, ", %s", addr);
			strcat(buf, line);
		}
		count++;
	}

	fclose(fp);

	return 0;
}

int setNameServers(char *buf)
{
	FILE *fp;
	char line[128], *ptr;

	if ((fp = fopen(RESOLV, "w")) == NULL)
		return -1;

	ptr = strtok(buf, ", ");

	do {
		if (snprintf(line, sizeof(line), "nameserver %s\n", ptr) == 0)
			continue;
		fputs(line, fp);
	} while (ptr = strtok(NULL, ", "));

	fclose(fp);

	return 0;
}

#ifdef ACCOUNT_CONFIG
//Jenny, get user account privilege
int getAccPriv(char *user)
{
	int totalEntry, i;
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	char suName[MAX_NAME_LEN], usName[MAX_NAME_LEN];

	mib_get(MIB_SUSER_NAME, (void *)suName);
	mib_get(MIB_USER_NAME, (void *)usName);
	if (strcmp(suName, user) == 0)
		return (int)PRIV_ROOT;
	else if (strcmp(usName, user) == 0)
		return (int)PRIV_USER;
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
	for (i=0; i<totalEntry; i++)
	{
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry))
			continue;
		if (strcmp(Entry.userName, user) == 0)
			return Entry.privilege;
	}
	return -1;
}
#endif

//jim support dsl disconnection when firmware upgrade from Local Side.....
//retrun value: 1-local        0 - wan         -1 - error
static int isAccessFromLocal(unsigned int ip)
{
	unsigned int uLanIp;
	unsigned int uLanMask;
	char secondIpEn;
	unsigned int uSecondIp;
	unsigned int uSecondMask;

	if (!mib_get( MIB_ADSL_LAN_IP, (void *)&uLanIp ))
		return -1;
	if (!mib_get( MIB_ADSL_LAN_SUBNET, (void *)&uLanMask ))
		return -1;

	if ( (ip & uLanMask) == (uLanIp & uLanMask) ) {//in the same subnet with LAN port
		return 1;
	} else {
		if (!mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&secondIpEn ))
			return -1;

		if (secondIpEn == 1) {//second IP is enabled
			if (!mib_get( MIB_ADSL_LAN_IP2, (void *)&uSecondIp))
				return -1;
			if (!mib_get( MIB_ADSL_LAN_SUBNET2, (void *)&uSecondMask))
				return -1;

			if ( (ip & uSecondMask) == (uSecondIp & uSecondMask) )//in the same subnet with LAN port
				return 1;
		}
	}

	return 0;
}

// Jenny, for checking duplicated destination address
int checkRoute(MIB_CE_IP_ROUTE_T rtEntry, int idx)
{
	//unsigned char destNet[4];
	unsigned long destID, netMask, nextHop;
	unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); /* get chain record size */
	MIB_CE_IP_ROUTE_T Entry;
	int i;

	/*destNet[0] = rtEntry.destID[0] & rtEntry.netMask[0];
	destNet[1] = rtEntry.destID[1] & rtEntry.netMask[1];
	destNet[2] = rtEntry.destID[2] & rtEntry.netMask[2];
	destNet[3] = rtEntry.destID[3] & rtEntry.netMask[3];*/
	destID = *((unsigned long *)&rtEntry.destID);
	netMask = *((unsigned long *)&rtEntry.netMask);
	nextHop = *((unsigned long *)&rtEntry.nextHop);

	// check if route exists
	for (i=0; i<totalEntry; i++) {
		long pdestID, pnetMask, pnextHop;
		unsigned char pdID[4];
		char *temp;
		if (i == idx)
			continue;
		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
			return 0;

		pdID[0] = Entry.destID[0] & Entry.netMask[0];
		pdID[1] = Entry.destID[1] & Entry.netMask[1];
		pdID[2] = Entry.destID[2] & Entry.netMask[2];
		pdID[3] = Entry.destID[3] & Entry.netMask[3];
		temp = inet_ntoa(*((struct in_addr *)pdID));
		pdestID = ntohl(inet_addr(temp));
		pnetMask = *((unsigned long *)&Entry.netMask);
		pnextHop = *((unsigned long *)&Entry.nextHop);
		if (pdestID == destID && pnetMask == netMask && pnextHop == nextHop && rtEntry.ifIndex == Entry.ifIndex)
			return 0;
	}
	return 1;
}

int isValidIpAddr(char *ipAddr)
{
	long field[4];

	if (sscanf(ipAddr, "%ld.%ld.%ld.%ld", &field[0], &field[1], &field[2], &field[3]) != 4)
		return 0;

	if (field[0] < 1 || field[0] > 223 || field[0] == 127 || field[1] < 0 || field[1] > 255 || field[2] < 0 || field[2] > 255 || field[3] < 0 || field[3] > 254)
		return 0;

	if (inet_addr(ipAddr) == -1)
		return 0;

	return 1;
}

int isValidHostID(char *ip, char *mask)
{
	long hostIp, netMask, hid, mbit;
	int i, bit, bitcount = 0;

	inet_aton(mask, (struct in_addr *)&netMask);
	hostIp = ntohl(inet_addr(ip));

	hid = ~netMask & hostIp;
	if (hid == 0x0)
		return 0;
	mbit = 0;
	while (1) {
		if (netMask & 0x80000000) {
			mbit++;
			netMask <<= 1;
		}
		else
			break;
	}
	mbit = 32 - mbit;
	for (i=0; i<mbit; i++) {
		bit = hid & 1L;
		if (bit)
			bitcount ++;
		hid >>= 1;
	}
	if (bitcount == mbit)
		return 0;
	return 1;
}

int isValidNetmask(char *mask, int checkbyte)
{
	long netMask;
	int i, bit, isChanged = 0;

	netMask = ntohl(inet_addr(mask));

	// Check most byte (must be 255) and least significant bit (must be 0)
	if (checkbyte) {
		bit = (netMask & 0xFF000000L) >> 24;
		if (bit != 255)
			return 0;
	}

//	bit = netMask & 1L;
//	if (bit)
//		return 0;

	// make sure the bit pattern changes from 0 to 1 only once
	for (i=1; i<31; i++) {
		netMask >>= 1;
		bit = netMask & 1L;

		if (bit) {
			if (!isChanged)
				isChanged = 1;
		}
		else {
			if (isChanged)
				return 0;
		}
	}

	return 1;
}

// check whether an IP address is in the same subnet
int isSameSubnet(char *ipAddr1, char *ipAddr2, char *mask)
{
	long netAddr1, netAddr2, netMask;

	netAddr1 = inet_addr(ipAddr1);
	netAddr2 = inet_addr(ipAddr2);
	netMask = inet_addr(mask);

	if ((netAddr1 & netMask) != (netAddr2 & netMask))
		return 0;

	return 1;
}

int isValidMacString(char *MacStr)
{
	int i;

	if(strlen(MacStr) != 17){
		return 0;
	}

	for(i=0;i<17;i++){
		if((i+1)%3 == 0){
			if(MacStr[i] != ':')
				return 0;
		}else{
			if(!((MacStr[i] >= '0' && MacStr[i] <= '9')
				|| (MacStr[i] >= 'a' && MacStr[i] <= 'f')
				|| (MacStr[i] >= 'A' && MacStr[i] <= 'F'))){
				return 0;
			}
		}
	}
	return 1;
}

int isValidMacAddr(unsigned char *macAddr)
{
	// Check for bad, multicast, broadcast, or null address
	if ((macAddr[0] & 1) || (macAddr[0] & macAddr[1] & macAddr[2] & macAddr[3] & macAddr[4] & macAddr[5]) == 0xff
		|| (macAddr[0] | macAddr[1] | macAddr[2] | macAddr[3] | macAddr[4] | macAddr[5]) == 0x00)
		return 0;

	return 1;
}

#ifdef QOS_SPEED_LIMIT_SUPPORT
//return -1 --not existed
int mib_qos_speed_limit_existed(int speed,int prior)
{
	int entryTotalNum=mib_chain_total(MIB_QOS_SPEED_LIMIT);
	int i=0;
	MIB_CE_IP_QOS_SPEEDRANK_T entry;
	for(i=0;i<entryTotalNum;i++)
		{
		    mib_chain_get(MIB_QOS_SPEED_LIMIT, i, &entry);
		   if(entry.speed==speed&&entry.prior==prior)
		   	return entry.index;
		}
	return -1;
}
#endif

//Ethernet
#if defined(ELAN_LINK_MODE)
#include <linux/sockios.h>
struct mii_ioctl_data {
	unsigned short	phy_id;
	unsigned short	reg_num;
	unsigned short	val_in;
	unsigned short	val_out;
};
#endif

int restart_ethernet(int instnum)
{
	char vChar=0;
	int status=0;

#ifdef _CWMP_MIB_
	//eth0 interface enable or disable
	mib_get(CWMP_LAN_ETHIFENABLE, (void *)&vChar);

	if(vChar==0)
	{
		va_cmd(IFCONFIG, 2, 1, ELANIF, "down");
		printf("Disable eth0 interface\n");
		return 0;
	}
	else
	{
#endif
		va_cmd(IFCONFIG, 2, 1, ELANIF, "up");
		printf("Enable eth0 interface\n");

#ifdef ELAN_LINK_MODE_INTRENAL_PHY
		int skfd;
		struct ifreq ifr;
		unsigned char mode;
		//struct mii_ioctl_data *mii = (struct mii_data *)&ifr.ifr_data;
		//MIB_CE_SW_PORT_T Port;
		//int i, k, total;
		struct ethtool_cmd ecmd;


		strcpy(ifr.ifr_name, ELANIF);
		ifr.ifr_data = &ecmd;


		if (!mib_get(MIB_ETH_MODE, (void *)&mode))
			return -1;

		skfd = socket(AF_INET, SOCK_DGRAM, 0);
		if(skfd == -1)
		{
			fprintf(stderr, "Socket Open failed Error\n");
			return -1;
		}
		ecmd.cmd = ETHTOOL_GSET;
		if (ioctl(skfd, SIOCETHTOOL, &ifr) < 0) {
			fprintf(stderr, "ETHTOOL_GSET on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
			goto error;
		}

		ecmd.autoneg = AUTONEG_DISABLE;
		switch(mode) {
		case LINK_10HALF:
			ecmd.speed = SPEED_10;
			ecmd.duplex = DUPLEX_HALF;
			break;
		case LINK_10FULL:
			ecmd.speed = SPEED_10;
			ecmd.duplex = DUPLEX_FULL;
			break;
		case LINK_100HALF:
			ecmd.speed = SPEED_100;
			ecmd.duplex = DUPLEX_HALF;
			break;
		case LINK_100FULL:
			ecmd.speed = SPEED_100;
			ecmd.duplex = DUPLEX_FULL;
			break;
		default:
			ecmd.autoneg = AUTONEG_ENABLE;
		}

		ecmd.cmd = ETHTOOL_SSET;
		if (ioctl(skfd, SIOCETHTOOL, &ifr) < 0) {
			fprintf(stderr, "ETHTOOL_SSET on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
			goto error;
		}
		/*
		ecmd.cmd = ETHTOOL_NWAY_RST;
		if (ioctl(skfd, SIOCETHTOOL, &ifr) < 0) {
			fprintf(stderr, "ETHTOOL_SSET on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
		}
		*/

		error:

		close(skfd);
#endif
		return status;
#ifdef _CWMP_MIB_
	}
#endif

	return -1;

}

#ifdef ELAN_LINK_MODE
// return value:
// 0  : successful
// -1 : failed
int setupLinkMode(void)
{
	int skfd;
	struct ifreq ifr;
	struct mii_ioctl_data *mii = (struct mii_data *)&ifr.ifr_data;
	//MIB_CE_SW_PORT_Tp pPort;
	MIB_CE_SW_PORT_T Port;
	int i, k, total;
	int status=0;

	strcpy(ifr.ifr_name, ELANIF);
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	total = mib_chain_total(MIB_SW_PORT_TBL);

	for (i=0; i<total; i++) {
		if (!mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&Port))
			continue;
		mii->phy_id = i; // phy i
		mii->reg_num = 4; // register 4
		// set NWAY advertisement
		mii->val_in = 0x0401; // enable flow control capability and IEEE802.3
		if (Port.linkMode == LINK_10HALF || Port.linkMode == LINK_AUTO)
			mii->val_in |= (1<<(5+LINK_10HALF));
		if (Port.linkMode == LINK_10FULL || Port.linkMode == LINK_AUTO)
			mii->val_in |= (1<<(5+LINK_10FULL));
		if (Port.linkMode == LINK_100HALF || Port.linkMode == LINK_AUTO)
			mii->val_in |= (1<<(5+LINK_100HALF));
		if (Port.linkMode == LINK_100FULL || Port.linkMode == LINK_AUTO)
			mii->val_in |= (1<<(5+LINK_100FULL));

		if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
			fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
		}

		// restart
		mii->reg_num = 0; // register 0
		mii->val_in = 0x1200; // enable auto-negotiation and restart it
		if (ioctl(skfd, SIOCSMIIREG, &ifr) < 0) {
			fprintf(stderr, "SIOCSMIIREG on %s failed: %s\n", ifr.ifr_name,
				strerror(errno));
			status=-1;
		};
	}
	if(skfd!=-1) close(skfd);
	return status;
}
#endif // of ELAN_LINK_MODE

// Mason Yu
struct	callout {
    struct timeval	c_time;		/* time at which to call routine */
    void		*c_arg;		/* argument to routine */
    void		(*c_func) __P((void *)); /* routine */
    struct		callout *c_next;
};

void timeout(void (*func) __P((void *)), void *arg, int time, struct callout *handle);
void mtimeout(void (*func) __P((void *)), void *arg, int time, struct callout *handle);
void untimeout(struct callout *handle);
static struct callout *callout = NULL;	/* Callout list */
static struct timeval timenow;		/* Current time */

/*
 * timeout - Schedule a timeout.
 *
 * Note that this timeout takes the number of seconds, NOT hz (as in
 * the kernel).
 */
void
timeout(func, arg, time, handle)
    void (*func) __P((void *));
    void *arg;
    int time;
    struct callout *handle;
{
    struct callout *p, **pp;

    untimeout(handle);

    handle->c_arg = arg;
    handle->c_func = func;
    gettimeofday(&timenow, NULL);
    handle->c_time.tv_sec = timenow.tv_sec + time;
    handle->c_time.tv_usec = timenow.tv_usec;

    /*
     * Find correct place and link it in.
     */
    for (pp = &callout; (p = *pp); pp = &p->c_next)
	if (handle->c_time.tv_sec < p->c_time.tv_sec
	    || (handle->c_time.tv_sec == p->c_time.tv_sec
		&& handle->c_time.tv_usec < p->c_time.tv_usec))
	    break;
    handle->c_next = p;
    *pp = handle;
}


/*
 * untimeout - Unschedule a timeout.
 */
void
untimeout(handle)
struct callout *handle;
{
    struct callout **copp, *freep;

    /*
     * Find first matching timeout and remove it from the list.
     */
    for (copp = &callout; (freep = *copp); copp = &freep->c_next)
	if (freep == handle) {
	    *copp = freep->c_next;
	    break;
	}
}


/*
 * calltimeout - Call any timeout routines which are now due.
 */
void
calltimeout()
{
    struct callout *p;

    while (callout != NULL) {
		p = callout;
		if (gettimeofday(&timenow, NULL) < 0)
		    //fatal("Failed to get time of day: %m");
	    	printf("Failed to get time of day: %m");
		if (!(p->c_time.tv_sec < timenow.tv_sec
		      || (p->c_time.tv_sec == timenow.tv_sec
			  && p->c_time.tv_usec <= timenow.tv_usec)))
	    	break;		/* no, it's not time yet */
		callout = p->c_next;
		(*p->c_func)(p->c_arg);
    }
}


/*
 * timeleft - return the length of time until the next timeout is due.
 */
static struct timeval *
timeleft(tvp)
    struct timeval *tvp;
{
    if (callout == NULL)
	return NULL;

    gettimeofday(&timenow, NULL);
    tvp->tv_sec = callout->c_time.tv_sec - timenow.tv_sec;
    tvp->tv_usec = callout->c_time.tv_usec - timenow.tv_usec;
    if (tvp->tv_usec < 0) {
	tvp->tv_usec += 1000000;
	tvp->tv_sec -= 1;
    }
    if (tvp->tv_sec < 0)
	tvp->tv_sec = tvp->tv_usec = 0;

    return tvp;
}

#ifdef CONFIG_RTK_OAM_V1
#define EPONOAMD_PID_FILE		"/var/run/eponoamd.pid"
struct callout eponoamd_ch;

void notify_eponoamd(void *dummy)
{
	int pid, ret;
	unsigned char loid[30], passwd[30];

	pid = read_pid(EPONOAMD_PID_FILE);
	if (pid > 0)
	{
		ret = kill(pid,0);
		if((ret == -1) && (errno == ESRCH))
		{
			//eponoamd no exist
			TIMEOUT(notify_eponoamd, 0, 10, eponoamd_ch);
		}
		else
		{
			mib_get(MIB_LOID, loid);
			mib_get(MIB_LOID_PASSWD, passwd);

			va_cmd("/bin/oamcli", 6, 1, "set", "ctc", "loid", "0", loid, passwd);
		}
	}
	else
	{
		//printf("eponoamd not exist!\n");
		TIMEOUT(notify_eponoamd, 0, 10, eponoamd_ch);
	}
}
#endif


#ifdef WEB_REDIRECT_BY_MAC
struct callout landingPage_ch;
void clearLandingPageRule(void *dummy)
{
	int status=0;
	char ipaddr[16], ip_port[32];
	char tmpbuf[MAX_URL_LEN];
	int  def_port=WEB_REDIR_BY_MAC_PORT;
	int num, i;
	unsigned int uLTime;

	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", "WebRedirectByMAC");

	ipaddr[0]='\0'; ip_port[0]='\0';

	if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
	{
		strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
		ipaddr[15] = '\0';
		sprintf(ip_port,"%s:%d",ipaddr,def_port);
	}

	//iptables -t nat -A WebRedirectByMAC -d 192.168.1.1 -j RETURN
	status|=va_cmd(IPTABLES, 8, 1, "-t", "nat","-A","WebRedirectByMAC",
		"-d", ipaddr, "-j", (char *)FW_RETURN);

	//iptables -t nat -A WebRedirectByMAC -p tcp --dport 80 -j DNAT --to-destination 192.168.1.1:8080
	status|=va_cmd(IPTABLES, 12, 1, "-t", "nat","-A","WebRedirectByMAC",
		"-p", "tcp", "--dport", "80", "-j", "DNAT",
		"--to-destination", ip_port);

	num = mib_chain_total( MIB_WEB_REDIR_BY_MAC_TBL );
	for ( i=0; i<num; i++) {
		mib_chain_delete( MIB_WEB_REDIR_BY_MAC_TBL, i );
	}

	//update to the flash
	#if 0
	itfcfg("sar", 0);
	itfcfg(ELANIF, 0);
	#endif
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	#if 0
	itfcfg("sar", 1);
	itfcfg(ELANIF, 1);
	#endif

        mib_get(MIB_WEB_REDIR_BY_MAC_INTERVAL, (void *)&uLTime);
	TIMEOUT(clearLandingPageRule, 0, uLTime, landingPage_ch);
}
#endif

#ifdef AUTO_DETECT_DMZ
#define AUTO_DMZ_INTERVAL 30
static int getDhcpClientIP(char **ppStart, unsigned long *size, char *ip)
{
	struct dhcpOfferedAddr {
        	u_int8_t chaddr[16];
        	u_int32_t yiaddr;       /* network order */
        	u_int32_t expires;      /* host order */
	};
	struct dhcpOfferedAddr entry;
	if (*size < sizeof(entry))
		return -1;
	entry = *((struct dhcpOfferedAddr *)*ppStart);
	*ppStart = *ppStart + sizeof(entry);
	*size = *size - sizeof(entry);
	if (entry.expires == 0)
		return 0;
	if (entry.chaddr[0]==0&&entry.chaddr[1]==0&&entry.chaddr[2]==0&&entry.chaddr[3]==0&&entry.chaddr[4]==0&&entry.chaddr[5]==0)
		return 0;
	strcpy(ip, inet_ntoa(*((struct in_addr *)&entry.yiaddr)));
	return 1;
}

static int Get1stArp(char *dmzIP)
{
	FILE *fp;
	char  buf[256];
	char tmp0[32],tmp1[32],tmp2[32];
	int dmzFlags;
	dmzIP[0] = 0; // "" empty
	fp = fopen("/proc/net/arp", "r");
	if (fp == NULL)
		printf("read arp file fail!\n");
	else {
		fgets(buf, 256, fp);//first line!?
		while(fgets(buf, 256, fp) >0) {
			//sscanf(buf, "%s", dmzIP);
			sscanf(buf,"%s	%*s	0x%x %s %s %s ", dmzIP, &dmzFlags,tmp0,tmp1,tmp2);
			if ((dmzFlags == 0) || (strncmp(tmp2,"br",2)!=0))
				continue;
			return 1;
		}
		fclose(fp);
		return 0;
	}
	return 0;
}

struct callout autoDMZ_ch;

void poll_autoDMZ(void *dummy)
{
	static char autoDMZ = 0;
	// signal dhcp client to renew
	struct stat status;
	char dhcpIP[40], *buffer = NULL, *ptr;
	FILE *fp;
	char dmz_ip_str[20];


	if (autoDMZ == 0) // search 1st arp
	{
		unsigned long ulDmz, ulDhcp;
		unsigned char ucPreStat;
		int entryNum, ret;
		char s_entryNum[8], ip[32];
		struct in_addr ipAddr, dmzIp;

		fflush(stdout);
		if (Get1stArp(dmz_ip_str) == 1)
		{
			if (strlen(dmz_ip_str) == 0)
				goto end; // error

			autoDMZ = 1;
			mib_get(MIB_DMZ_ENABLE, (void *)&ucPreStat);
			mib_get(MIB_DMZ_IP, (void *)&dmzIp);
			ulDmz = *((unsigned long *)&dmzIp);
			strncpy(ip, inet_ntoa(dmzIp), 16); // ip -> old dmz
			if (strcmp(ip,dmz_ip_str) == 0)
				goto end; // no changed!

			inet_aton(dmz_ip_str, &ipAddr);
			ip[15] = '\0';
			entryNum = mib_chain_total (MIB_IP_PORT_FILTER_TBL) + 3;
			snprintf(s_entryNum, 8, "%d", entryNum);
			if (ucPreStat && ulDmz != 0)
				clearDMZ();

			if (mib_set(MIB_DMZ_IP, (void *)&ipAddr) == 0)
				printf("Set DMZ MIB error!\n");

				if (!mib_set(MIB_DMZ_ENABLE, (void *)&autoDMZ))
					printf("Set DMZ Capability error!\n");
				setDMZ(dmz_ip_str);
				goto end;

		}

	}
	else  // check dhcp and then arp
	{
		unsigned long ulDmz, ulDhcp;
		unsigned char ucPreStat;
		struct in_addr ipAddr, dmzIp;
		int entryNum, ret;
		char s_entryNum[8], ip[32];
		// siganl DHCP server to update lease file
		int pid = read_pid(DHCPSERVERPID);

		if (pid > 0)	// renew
			kill(pid, SIGUSR1);
		usleep(1000);

		if (stat(DHCPD_LEASE, &status) < 0)
			goto end;

		// read DHCP server lease file
		buffer = malloc(status.st_size);
		if (buffer == NULL)
			goto end;
		fp = fopen(DHCPD_LEASE, "r");
		if (fp == NULL)
			goto end;
		fread(buffer, 1, status.st_size, fp);
		fclose(fp);
		ptr = buffer;

		while (1) {
			if (getDhcpClientIP(&ptr, &status.st_size, dhcpIP) == 1)
			{
				mib_get(MIB_DMZ_ENABLE, (void *)&ucPreStat);
				mib_get(MIB_DMZ_IP, (void *)&dmzIp);
				ulDmz = *((unsigned long *)&dmzIp);
				strncpy(ip, inet_ntoa(dmzIp), 16);
				if (strcmp(ip, dhcpIP) ==0 )	//dhcp still in using..
				{
					goto end;
				}
			}
			else // find the 1st arp
			{
				// uses the 1st entry
				if (Get1stArp(dmz_ip_str) == 1)
				{
					if (strlen(dmz_ip_str) == 0)
						goto end; // error

					autoDMZ = 1;
					mib_get(MIB_DMZ_ENABLE, (void *)&ucPreStat);
					mib_get(MIB_DMZ_IP, (void *)&dmzIp);
					ulDmz = *((unsigned long *)&dmzIp);
					strncpy(ip, inet_ntoa(dmzIp), 16); // ip -> old dmz
					inet_aton(dmz_ip_str, &ipAddr);
					ip[15] = '\0';
					if (strcmp(ip,dmz_ip_str) == 0)
						goto end; // still the same one

					entryNum = mib_chain_total (MIB_IP_PORT_FILTER_TBL) + 3;
					snprintf(s_entryNum, 8, "%d", entryNum);
					if (ucPreStat && ulDmz != 0)
						clearDMZ();

					if (mib_set(MIB_DMZ_IP, (void *)&ipAddr) == 0)
						printf("Set DMZ MIB error!\n");

						if (!mib_set(MIB_DMZ_ENABLE, (void *)&autoDMZ))
							printf("Set DMZ Capability error!\n");
						setDMZ(dmz_ip_str);
						goto end;

				}
				else
				{
					// clear rules
					mib_get(MIB_DMZ_IP, (void *)&dmzIp);
					strncpy(ip, inet_ntoa(dmzIp), 16); // ip -> old dmz
					ip[15] = '\0';

					clearDMZ();
					*((unsigned long *)&dmzIp) = 0;
					autoDMZ = 0;
					mib_set(MIB_DMZ_ENABLE, (void *)&autoDMZ);
					mib_set(MIB_DMZ_IP, (void *)&dmzIp);
				}
				goto end;
			}
		}
	}


end:
	if (buffer)
		free(buffer);
	TIMEOUT(poll_autoDMZ, 0, AUTO_DMZ_INTERVAL, autoDMZ_ch);
}
#endif

#ifdef CONFIG_DEV_xDSL
static int doOAMLookback(MIB_CE_ATM_VC_Tp pEntry)
{
#ifdef EMBED
	int	skfd;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	int finished_OAMLB;
	time_t currTime, preTime;

	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}

	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));
	lbReq.Scope = 0;	// Segment
	lbReq.vpi = pEntry->vpi;
	lbReq.vci = pEntry->vci;
	memset(lbReq.LocID, 0xff, 16);	// Loopback Location ID
	mysio.number = 0;	// ATM interface number
	mysio.arg = (void *)&lbReq;
	// Start the loopback test
	if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_START failed !");
		close(skfd);
		return -1;
	}

	finished_OAMLB = 0;
	// Mason Yu. If receive a SIGALRM, the all timer will be disable.
	// So I use "time()" to schedule Timer.
	//signal(SIGALRM, lbTimeout);
	//alarm(MAXWAIT);

	// Mason Yu. I can not use "TIMEOUT()" to register timer.
	//TIMEOUT(oamLBTimeout, 0, 1, oamLookBK_ch);
	time(&currTime);
	preTime = currTime;

	// Query the loopback status
	mysio.arg = (void *)&lbState;
	lbState.vpi = pEntry->vpi;
	lbState.vci = pEntry->vci;
	lbState.Tag = lbReq.Tag;

	while (1)
	{
		// Mason Yu. Use time() to schedule Timer.
		time(&currTime);
	    	if (currTime - preTime >= 1) {
	    		printf("This OAMLB is time out!\n");
	    		finished_OAMLB = 1;
	    	}

		if (finished_OAMLB)
			break;	// break for timeout

		if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0) {
			perror("ioctl: ATM_OAM_LB_STATUS failed !");
			mysio.arg = (void *)&lbReq;
			ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
			close(skfd);
			return -1;
		}

		if (lbState.count[0] > 0)
		{
			break;	// break for loopback success
		}
	}

	mysio.arg = (void *)&lbReq;
	// Stop the loopback test
	if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_STOP failed !");
		close(skfd);
		return -1;
	}
	close(skfd);

	if (finished_OAMLB)
		return 0;	// failed
	else
		return 1;	// successful
#else
	return 1;
#endif
}

static int doOAMLookback_F5_EndToEnd(MIB_CE_ATM_VC_Tp pEntry)
{
#ifdef EMBED
	int	skfd;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	int finished_OAMLB_F5_EndToEnd;
	time_t currTime, preTime;

	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error(F5 End-To-End)");
		return -1;
	}

	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));
	lbReq.Scope = 1;	// End-To-End
	lbReq.vpi = pEntry->vpi;
	lbReq.vci = pEntry->vci;
	memset(lbReq.LocID, 0xff, 16);	// Loopback Location ID
	mysio.number = 0;	// ATM interface number
	mysio.arg = (void *)&lbReq;
	// Start the loopback test
	if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_START(F5 End-To-End) failed !");
		close(skfd);
		return -1;
	}

	finished_OAMLB_F5_EndToEnd = 0;
	// Mason Yu. If receive a SIGALRM, the all timer will be disable.
	// So I use "time()" to schedule Timer.
	//signal(SIGALRM, lbTimeout);
	//alarm(MAXWAIT);

	// Mason Yu. I can not use "TIMEOUT()" to register timer.
	//TIMEOUT(oamLBTimeout, 0, 1, oamLookBK_ch);
	time(&currTime);
	preTime = currTime;

	// Query the loopback status
	mysio.arg = (void *)&lbState;
	lbState.vpi = pEntry->vpi;
	lbState.vci = pEntry->vci;
	lbState.Tag = lbReq.Tag;

	while (1)
	{
		// Mason Yu. Use time() to schedule Timer.
		time(&currTime);
	    	if (currTime - preTime >= 1) {
	    		printf("This OAMLB(F5 End-To-End) is time out!\n");
	    		finished_OAMLB_F5_EndToEnd = 1;
	    	}

		if (finished_OAMLB_F5_EndToEnd)
			break;	// break for timeout

		if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0) {
			perror("ioctl: ATM_OAM_LB_STATUS(F5 End-To-End) failed !");
			mysio.arg = (void *)&lbReq;
			ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
			close(skfd);
			return -1;
		}

		if (lbState.count[0] > 0)
		{
			break;	// break for loopback success
		}
	}

	mysio.arg = (void *)&lbReq;
	// Stop the loopback test
	if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) {
		perror("ioctl: ATM_OAM_LB_STOP(F5 End-To-End) failed !");
		close(skfd);
		return -1;
	}
	close(skfd);

	if (finished_OAMLB_F5_EndToEnd)
		return 0;	// failed
	else
		return 1;	// successful
#else
	return 1;
#endif
}
#endif //CONFIG_DEV_xDSL

#ifdef CONFIG_RTL8672_SAR
// Timer for auto search PVC
#if defined(AUTO_PVC_SEARCH_TR068_OAMPING) || defined(AUTO_PVC_SEARCH_PURE_OAMPING) || defined(AUTO_PVC_SEARCH_AUTOHUNT)
struct callout autoSearchPVC_ch;
int sok=0;
#define MAX_PVC_SEARCH_PAIRS 16


#ifdef AUTO_PVC_SEARCH_TR068_OAMPING
typedef struct pvc_entry {
	unsigned long vpi;
	unsigned long vci;
} PVC_T;

PVC_T pvcList[] = {{0, 35}, {8, 35}, {0, 43}, {0, 51}, {0, 59},
	{8, 43}, {8, 51}, {8, 59}, {0}
};
#endif

void succ_AutoSearchPVC(void)
{
	fin_AutoSearchPVC = 1;
	UNTIMEOUT(stopAutoSearchPVC, 0, autoSearchPVC_ch);
	printf("***** Auto Search PVC is successful and stopped ! *****\n");
	syslog(LOG_INFO, "Auto Search PVC is successful and stopped !\n");
}

#if defined(AUTO_PVC_SEARCH_AUTOHUNT)
static void StopSarAutoPvcSearch(int vpi,int vci)
{
	FILE *fp;
	if (fp=fopen("/proc/AUTO_PVC_SEARCH", "w") )
	{
//		printf("StopSarAutoPvcSearch: received (%d,%d) inform SAR driver stop auto-pvc-search\n", vpi,vci);

		fprintf( fp, "0 %d,%d\n", vpi,vci);
		fclose(fp);
	} else {
		printf("Open /proc/AUTO_PVC_SEARCH failed! Can't stop SAR driver doing auto-pvc-search\n");
	}
}
#endif

// Mason Yu
#ifdef AUTO_PVC_SEARCH_TR068_OAMPING
static void startAutoPVCSearchTR068OAMPing_F5_Segment(MIB_CE_ATM_VC_Tp pEntry, unsigned long bitmap)
{
	int idx;
	unsigned long mask=1;
	unsigned long ovpi, ovci;
	unsigned char vChar;
	int i;

	if (!pEntry || !pEntry->enable)
		goto search_table_tr068;

        if (doOAMLookback(pEntry)!=1)
	{
		if(i==0)
			stopConnection(pEntry);

search_table_tr068:
		// start searching ...
		printf("TR068 OAMPing(F5 Segment):Default pvc failed, start searching ...\n");
		ovpi = pEntry->vpi;
		ovci = pEntry->vci;

		//Retrieve PVC list from Flash
		for (idx=0; pvcList[idx].vpi || pvcList[idx].vci; idx++) {
			if (bitmap & mask) {
				//stopConnection(pEntry);
				pEntry->vpi = pvcList[idx].vpi;
				pEntry->vci = pvcList[idx].vci;
				//startConnection(pEntry);
				if (doOAMLookback(pEntry)) {
					// That's it
					printf("TR068 OAMPing(F5 Segment):That's it! vpi=%d, vci=%d\n", pEntry->vpi, pEntry->vci);
					sok=1;
					break;
				}
			}
			mask <<= 1;
		}

		if (sok) { // search ok, set it
			setVC0Connection(pEntry->vpi, pEntry->vci);
			succ_AutoSearchPVC();
		}
		else { // search failed, back to original
			//stopConnection(pEntry);
			pEntry->vpi = ovpi;
			pEntry->vci = ovci;
			//startConnection(pEntry);
			printf("TR068 OAMPing(F5 Segment):Auto-search pvc failed !\n");
		}
	}
	else
	{
		printf("TR068 OAMPing(F5 Segment):first-pvc oam loopback ok!\n");
		sok=1;
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
		StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif
       		succ_AutoSearchPVC();
   }
}

static void startAutoPVCSearchTR068OAMPing_F5_EndToEnd(MIB_CE_ATM_VC_Tp pEntry, unsigned long bitmap)
{
	int idx;
	unsigned long mask=1;
	unsigned long ovpi, ovci;
	unsigned char vChar;
	int i;

	if (!pEntry || !pEntry->enable)
		goto search_table_tr068;

        if (doOAMLookback_F5_EndToEnd(pEntry)!=1)
	{
		if(i==0)
			stopConnection(pEntry);

search_table_tr068:
		// start searching ...
		printf("TR068 OAMPing(F5 End-To-End):Default pvc failed, start searching ...\n");
		ovpi = pEntry->vpi;
		ovci = pEntry->vci;

		//Retrieve PVC list from Flash
		for (idx=0; pvcList[idx].vpi || pvcList[idx].vci; idx++) {
			if (bitmap & mask) {
				//stopConnection(pEntry);
				pEntry->vpi = pvcList[idx].vpi;
				pEntry->vci = pvcList[idx].vci;
				//startConnection(pEntry);
				if (doOAMLookback_F5_EndToEnd(pEntry)) {
					// That's it
					printf("TR068 OAMPing(F5 End-To-End):That's it! vpi=%d, vci=%d\n", pEntry->vpi, pEntry->vci);
					sok=1;
					break;
				}
			}
			mask <<= 1;
		}

		if (sok) { // search ok, set it
			setVC0Connection(pEntry->vpi, pEntry->vci);
			succ_AutoSearchPVC();
		}
		else { // search failed, back to original
			//stopConnection(pEntry);
			pEntry->vpi = ovpi;
			pEntry->vci = ovci;
			//startConnection(pEntry);
			printf("TR068 OAMPing(F5 End-To-End):Auto-search pvc failed !\n");
		}
	}
	else
	{
		printf("TR068 OAMPing(F5 End-To-End):first-pvc oam loopback ok!\n");
		sok=1;
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
		StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif
       		succ_AutoSearchPVC();
   }
}
#endif

#ifdef AUTO_PVC_SEARCH_PURE_OAMPING
static void startAutoPVCSearchPureOAMPing_F5_Segment(MIB_CE_ATM_VC_Tp pEntry, unsigned long bitmap)
{
	int idx;
	unsigned long mask=1;
	unsigned long ovpi, ovci;
	unsigned char vChar;
	int i;

	//  read PVC table from flash
	unsigned int entryNum, Counter;
	//MIB_AUTO_PVC_SEARCH_Tp entryP;
	MIB_AUTO_PVC_SEARCH_T Entry;
	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	if(entryNum > MAX_PVC_SEARCH_PAIRS)
		entryNum = MAX_PVC_SEARCH_PAIRS;


	if (!pEntry || !pEntry->enable)
		goto search_table;

        if (doOAMLookback(pEntry)!=1)
	{
		//if(i==0)
			stopConnection(pEntry);
search_table:
		// start searching ...
		printf("Pure OAMping(F5 Segment):Default pvc failed, start searching ...\n");
		ovpi = pEntry->vpi;
		ovci = pEntry->vci;

		//Retrieve PVC list
		for(Counter=0; Counter< entryNum; Counter++)
		{
			if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, Counter, (void *)&Entry))
				continue;
			//stopConnection(pEntry);
			pEntry->vpi = Entry.vpi;
			pEntry->vci = Entry.vci;
			//startConnection(pEntry);
			if (doOAMLookback(pEntry)) {
				// That's it
				printf("Pure OAMping(F5 Segment):That's it! vpi=%d, vci=%d\n", pEntry->vpi, pEntry->vci);
				printf("Pure OAMping(F5 Segment):Inform SAR driver to stop pvc search\n");
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
				StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif
				sok=1;
				break;
			}

		}


		if (sok) { // search ok, set it
			setVC0Connection(pEntry->vpi, pEntry->vci);
			succ_AutoSearchPVC();
		}
		else { // search failed, back to original
			//stopConnection(pEntry);
			pEntry->vpi = ovpi;
			pEntry->vci = ovci;
			//startConnection(pEntry);
			printf("Pure OAMping(F5 Segment): Auto-search PVC failed !\n");
		}
	}
	else
	{
		printf("Pure OAMping(F5 Segment): first-pvc oam loopback ok!\n");
		sok=1;
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
		StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif
		succ_AutoSearchPVC();

   }
}

static void startAutoPVCSearchPureOAMPing_F5_EndToEnd(MIB_CE_ATM_VC_Tp pEntry, unsigned long bitmap)
{
	int idx;
	unsigned long mask=1;
	unsigned long ovpi, ovci;
	unsigned char vChar;
	int i;

	//  read PVC table from flash
	unsigned int entryNum, Counter;
	//MIB_AUTO_PVC_SEARCH_Tp entryP;
	MIB_AUTO_PVC_SEARCH_T Entry;
	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	if(entryNum > MAX_PVC_SEARCH_PAIRS)
		entryNum = MAX_PVC_SEARCH_PAIRS;


	if (!pEntry || !pEntry->enable)
		goto search_table;

        if (doOAMLookback_F5_EndToEnd(pEntry)!=1)
	{
		//if(i==0)
			stopConnection(pEntry);
search_table:
		// start searching ...
		printf("Pure OAMping(F5 End-To-End):Default pvc failed, start searching ...\n");
		ovpi = pEntry->vpi;
		ovci = pEntry->vci;

		//Retrieve PVC list
		for(Counter=0; Counter< entryNum; Counter++)
		{
			if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, Counter, (void *)&Entry))
				continue;
			//stopConnection(pEntry);
			pEntry->vpi = Entry.vpi;
			pEntry->vci = Entry.vci;
			//startConnection(pEntry);
			if (doOAMLookback_F5_EndToEnd(pEntry)) {
				// That's it
				printf("Pure OAMping(F5 End-To-End):That's it! vpi=%d, vci=%d\n", pEntry->vpi, pEntry->vci);
				printf("Pure OAMping(F5 End-To-End):Inform SAR driver to stop pvc search\n");
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
				StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif
				sok=1;
				break;
			}

		}


		if (sok) { // search ok, set it
			setVC0Connection(pEntry->vpi, pEntry->vci);
			succ_AutoSearchPVC();
		}
		else { // search failed, back to original
			//stopConnection(pEntry);
			pEntry->vpi = ovpi;
			pEntry->vci = ovci;
			//startConnection(pEntry);
			printf("Pure OAMping(F5 End-To-End): Auto-search PVC failed !\n");
		}
	}
	else
	{
		printf("Pure OAMping(F5 End-To-End): first-pvc oam loopback ok!\n");
		sok=1;
#ifdef AUTO_PVC_SEARCH_AUTOHUNT
		StopSarAutoPvcSearch(pEntry->vpi, pEntry->vci);
#endif
		succ_AutoSearchPVC();

   }
}
#endif


// Mason Yu
// Find ch1~7 except ch0 and check if the ch1~7's VPI/VCI are the same as Searched VPI/VCI by Auto PVC Search.
static void ifAutoPVCisExist(unsigned int vpi, unsigned vci)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	// find VC from ch1 not ch0. Because the ch0 is used for Auto PVC Search
	for (i = 1; i < entryNum; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return;

		if ( Entry.vpi == vpi && Entry.vci == vci ) {
			stopConnection(&Entry);

			if(mib_chain_delete(MIB_ATM_VC_TBL, i) != 1) {
				printf("ifAutoPVCisExist: Delete chain record error!");
			}
			break;
		}
	}
}

static MIB_CE_ATM_VC_T vc0_Entry;

// Mason Yu
// If the configuration has include ch0, we just update ch0 with Searched VPI/VCI.
// If the configuration has not include ch0, we add a new entry(ch0) with Searched VPI/VCI.
int setVC0Connection(unsigned int vpi, unsigned int vci)
{
	MIB_CE_ATM_VC_T Entry;

	/* Retrieve ch 0's entry */
	if (!mib_chain_get(MIB_ATM_VC_TBL, 0, (void *)&Entry)) {
		// The ch0 does not exist
		printf("setVC0Connection: The ch0 does not exist\n");

		// sarctl pvcnumber 1
		//va_cmd("/bin/sarctl",2,1,"pvcnumber","1");
		memset(&Entry, 0x00, sizeof(Entry));

		Entry.ifIndex = TO_IFINDEX(MEDIA_ATM, DUMMY_PPP_INDEX, 0);
		Entry.vpi = vpi;
		Entry.qos = 0;
		Entry.vci = vci;
		Entry.pcr = ATM_MAX_US_PCR;
		Entry.encap = 1;
		Entry.napt = 1;
		Entry.mtu = 1500;
		Entry.enable = 1;

		/* create a new connection */
		// Mason Yu. flash all atm_vc_tbl for AutoHunt.
		#if 0
		if(Entry.enable) {
			printf("setVC0Connection: start connection (%d/%d)\n", vpi, vci);
			startConnection(&Entry);
		}
		#endif

		if(mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)&Entry) != 1){
			printf("setVC0Connection:Error! Add MIB_ATM_VC_TBL chain record for Auto PVC Search.\n");
			return 0;
		}
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

	} else {
		// The ch0 exist
		// Check if the searched VC exist on configure?
		ifAutoPVCisExist(vpi, vci);

		/* remove the default connection */
		// Mason Yu. flash all atm_vc_tbl for AutoHunt.
		//stopConnection(&Entry);
		deleteConnection(CONFIGONE, &Entry);

		Entry.vpi = vpi;
		Entry.vci = vci;

		/* create a new connection */
		// Mason Yu. flash all atm_vc_tbl for AutoHunt.
		#if 0
		if(Entry.enable) {
			printf("setVC0Connection: start connection (%d/%d)\n", vpi, vci);
			startConnection(&Entry);
		}
		#endif

		mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, 0);
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	}
	// Mason Yu. flash all atm_vc_tbl for AutoHunt.
	restartWAN(CONFIGONE, &Entry);
}

static void ResetSarAutoHunt(int vpi,int vci, int encap)
{
	FILE *fp;
	if (fp=fopen("/proc/AUTO_PVC_SEARCH", "w") )
	{
//		printf("StopSarAutoPvcSearch: received (%d,%d) inform SAR driver stop auto-pvc-search\n", vpi,vci);

		fprintf( fp, "2 %d,%d,%d\n", vpi,vci,encap);
		fclose(fp);
	} else {
		printf("Open /proc/AUTO_PVC_SEARCH failed! Can't reset SAR Driver to default setting for autoHunt\n");
	}
}

#ifdef AUTO_PVC_SEARCH_AUTOHUNT
#define MAX_PVC_SEARCH_PAIRS 16
static void StartAutoHunt(void)
{
	FILE *fp;

	//MIB_AUTO_PVC_SEARCH_Tp entryP;
	MIB_AUTO_PVC_SEARCH_T Entry;
	unsigned int entryNum,i;
	unsigned char tmp[12], tmpBuf[MAX_PVC_SEARCH_PAIRS*12];

	entryNum = mib_chain_total(MIB_AUTO_PVC_SEARCH_TBL);
	memset(tmpBuf, 0, sizeof(tmpBuf));
	for(i=0;i<entryNum; i++) {
		memset(tmp, 0, 12);
		//mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry);
		//if (!Entry)
		//	continue;
		if (!mib_chain_get(MIB_AUTO_PVC_SEARCH_TBL, i, (void *)&Entry))
			continue;
		sprintf(tmp,"(%d %d)", Entry.vpi, Entry.vci);
		strcat(tmpBuf, tmp);

	}
	//printf("StartSarAutoPvcSearch: inform SAR %s\n", tmpBuf);


	if (fp=fopen("/proc/AUTO_PVC_SEARCH", "w") )
	{
		fprintf(fp, "1%s\n", tmpBuf);	//write pvc list stored in flash to SAR driver
//		printf("StartSarAutoPvcSearch: Inform SAR driver to start auto-pvc-search\n");

		fclose(fp);
	} else {
		printf("Open /proc/AUTO_PVC_SEARCH failed! Can't start SAR driver doing auto-pvc-search\n");
	}

}
#endif
// Mason Yu. 20130304
static int AutoSearchPVC_Count = 1;
void stopAutoSearchPVC(void *dummy)
{
	MIB_CE_ATM_VC_T Entry;
	FILE *fp;
	int oamPingResult, do_autoHunt;
	long i;

	printf("***** Auto Search PVC is fail and stopped ! *****\n");
	syslog(LOG_INFO, "Auto Search PVC is fail and stopped");
	AutoSearchPVC_Count++;

	fp = fopen("/tmp/autoPVC", "r");
	if(fp){
		fscanf(fp, "OAMPing:%d AutoHUNT:%d\n", &oamPingResult, &do_autoHunt);
		fclose(fp);
		unlink("/tmp/autoPVC");

		// Auto search PVC is finished.
		fin_AutoSearchPVC = 1;

	} else {
		//TIMEOUT(stopAutoSearchPVC, 0, INTERVAL_AUTOPVC, autoSearchPVC_ch);
		return;
	}

	// Mason Yu. Delay 10 sec to make sure that the AutoHunt is finished.
	//for(i=0; i<100000000; i++);

	if ( oamPingResult == 0 && autoHunt_found == 0) {
		// Mason Yu. 20130304
		if ((AutoSearchPVC_Count <= COUNT_AUTOPVC) || (COUNT_AUTOPVC==0)) {
			printf("***** Auto Search PVC is fail and do again ! *****\n");
			startAutoSearchPVC();
		}
		else {
		printf("Reset SAR Driver for Auto Search PVC!\n");
		if (!mib_chain_get(MIB_ATM_VC_TBL, 0, (void *)&Entry))
			return;

		if ( do_autoHunt == 1 ) {
			//ResetSarAutoHunt(5, 35, 1);
			ResetSarAutoHunt(Entry.vpi, Entry.vci, Entry.encap);
		}

		// Reset ch0
		//setVC0Connection(5, 35);
		setVC0Connection(Entry.vpi, Entry.vci);
	}
	}
}

int startAutoSearchPVC(void)
{
	unsigned char autosearch;
	int i;
	MIB_CE_ATM_VC_T Entry;

	if (mib_get(MIB_ATM_VC_AUTOSEARCH, (void *)&autosearch) != 0)
	{
		if (autosearch == 1)
		{
			unsigned long map = 0xffffffff;
			MIB_CE_ATM_VC_Tp pFirstEntry=NULL;
			Modem_LinkSpeed vLs;
			vLs.upstreamRate=0;
			unsigned int entryNum;
			// Timer for auto search PVC
			FILE *fp;

			printf("***** Auto Search PVC started *****\n");
			syslog(LOG_INFO, "Auto Search PVC started");
			TIMEOUT(stopAutoSearchPVC, 0, INTERVAL_AUTOPVC, autoSearchPVC_ch);

			#if 0
			// wait until showtime
			while (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs,
				RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0) {
				usleep(1000000);
			}
			usleep(2000000);	//AUTO-PVC-SEARCH
			#endif

			// do first-pvc auto search per [TR-068, I-88]
			// Modified by Mason Yu
			entryNum = mib_chain_total(MIB_ATM_VC_TBL);

			for (i = 0; i < entryNum; i++)
			{
				unsigned long mask;
				int k;

				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
					return -1;
				// Retrieve pvc list from flash
#ifdef AUTO_PVC_SEARCH_TR068_OAMPING
				mask = 1;
				for (k=0; pvcList[k].vpi || pvcList[k].vci; k++) {
					if (pvcList[k].vpi == Entry.vpi &&
						pvcList[k].vci == Entry.vci)
					{
						map ^= mask;	// set bit to zero
						break;
					}
					mask <<= 1;
				}
#endif
				if (pFirstEntry==NULL && Entry.enable) // get the first pvc
				{
					pFirstEntry = &Entry;
					// Added by Mason Yu
					break;
				}
			}

			// Modified by Mason Yu
			if (pFirstEntry==NULL) {
				pFirstEntry = &Entry;
			}

			//if (pFirstEntry)
#ifdef AUTO_PVC_SEARCH_TR068_OAMPING
				startAutoPVCSearchTR068OAMPing_F5_Segment(pFirstEntry, map);
				if ( sok == 0 ) {
					setVC0Connection(pFirstEntry->vpi, pFirstEntry->vci);
					startAutoPVCSearchTR068OAMPing_F5_EndToEnd(pFirstEntry, map);
				}
#elif defined(AUTO_PVC_SEARCH_PURE_OAMPING)
				startAutoPVCSearchPureOAMPing_F5_Segment(pFirstEntry, map);
				if ( sok == 0 ){
					setVC0Connection(pFirstEntry->vpi, pFirstEntry->vci);
					startAutoPVCSearchPureOAMPing_F5_EndToEnd(pFirstEntry, map);
				}
#else
				printf("Just only do AutoHunt to find PVC\n");
#endif
			// Timer for auto search PVC
			fp = fopen("/tmp/autoPVC", "w+");
			if(fp){
				fprintf(fp, "OAMPing:%d AutoHUNT:0\n", sok);
				fclose(fp);
			}


#ifdef AUTO_PVC_SEARCH_AUTOHUNT
			//OAM loopback not found, start SAR driver auto-pvc-search
			if(sok==0){
				printf("***** startRest: call SAR driver to start autoHunt ****\n");
				// Kaohj
				StartAutoHunt();
				//cmd_start_autohunt();

				// Timer for auto search PVC
				fp = fopen("/tmp/autoPVC", "w+");
				if(fp){
					fprintf(fp, "OAMPing:0 AutoHUNT:1\n");
					fclose(fp);
				}

		        }
		} else {

			StopSarAutoPvcSearch(0,0);
			return -1;
		}
#else
        	} else {
        		return -1;
        	}

#endif
	}
	return 1;
}

#endif
#endif // of CONFIG_RTL8672_SAR

/* Martin zhu add for bridge WAN support ipv4/ipv6 Filter 2015.6.29 */
#ifdef CONFIG_IPV6
void setupBridgeIPv4IPv6Filter( MIB_CE_ATM_VC_Tp pEntry, unsigned char isadd )
{
	int ret;

	if ( CHANNEL_MODE_BRIDGE == (CHANNEL_MODE_T)pEntry->cmode )
	{
		if ( isadd ) {// add ACL Rules
		#ifdef CONFIG_RTK_RG_INIT
			ret = AddRTK_RG_ACL_Bridge_IPv4IPv6_Filters( pEntry );
		#endif
		} else {// delete ACL Rules
		#ifdef CONFIG_RTK_RG_INIT
			ret = FlushRTK_RG_ACL_Bridge_IPv4IPv6_Filters( pEntry );
		#endif
		}
	}

	return;
}
#endif

void startSNAT()
{
	int vcTotal, i;
	MIB_CE_ATM_VC_T Entry;
	char wanif[IFNAMSIZ];

	vcTotal = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < vcTotal; i++)
	{
		/* get the specified chain record */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			return -1;

		if (Entry.enable == 0)
			continue;

		ifGetName(Entry.ifIndex,wanif,sizeof(wanif));
		if (((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_IPOE)  ||
		  ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_RT1483) ||
		  ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_RT1577) )
		{
#ifdef CONFIG_IPV6
			if (Entry.IpProtocol & IPVER_IPV4) {
#endif
				if ((DHCP_T)Entry.ipDhcp == DHCP_DISABLED)
				{
					if (Entry.napt == 1)
					{
						// Setup one NAT Rule for the specfic interface
						startAddressMap(&Entry);
					}
				}
				else
				{
					if (Entry.napt == 1)
					{	// Enable NAPT
						va_cmd(IPTABLES, 8, 1, "-t", "nat", FW_ADD, "POSTROUTING",
							ARG_O, wanif, "-j", "MASQUERADE");
					}
				}
#ifdef CONFIG_IPV6
			}
#endif
		}
		else if (((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_PPPOE) ||
		      ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_PPPOA))
		{
#ifdef CONFIG_IPV6
			if (Entry.IpProtocol & IPVER_IPV4) {
#endif
				if (Entry.napt == 1)
				{	// Enable NAPT
					va_cmd(IPTABLES, 8, 1, "-t", "nat", FW_ADD, "POSTROUTING",
						"-o", wanif, "-j", "MASQUERADE");
				}
#ifdef CONFIG_IPV6
			}
#endif
		}

	}
}

// configAll = CONFIGALL,  pEntry = NULL  : restart all WAN connections(include VC, ETHWAN, PTMWAN, VPN, 3g).
//									  It means that we add or modify all VC, ETHWAN, PTMWAN channel.
// configAll = CONFIGONE, pEntry != NULL : restart specified VC, ETHWAN, PTMWAN connection and VPN, 3g connections.
// 									  It means that we add or modify a VC, ETHWAN, PTMWAN channel.
// configAll = CONFIGONE, pEntry = NULL  : restart VPN, 3g connections. It means that we delete a VC, ETHWAN, PTMWAN channel.
void restartWAN(int configAll, MIB_CE_ATM_VC_Tp pEntry)
{
	int dhcrelay_pid, i;
	char vChar;
	unsigned char tmpBuf[64];

#ifdef CONFIG_RTL8672_SAR
	itfcfg("sar", 0);
#endif

#if !defined(CONFIG_RTK_RG_INIT)
//eason path for pvc add/del and conntrack killall
// Mason Yu. Before flush all the conntrack table, we should close the eth0 and br0 interface.
// In order to into a loop in ip_conntrack_flush();
#if	!defined(CONFIG_RTL_MULTI_LAN_DEV) && !defined(CONFIG_RTL8686)
	va_cmd(IFCONFIG, 2, 1, ELANIF, "down");
#endif
	va_cmd(IFCONFIG, 2, 1, BRIF, "down");
	va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
#if	!defined(CONFIG_RTL_MULTI_LAN_DEV) && !defined(CONFIG_RTL8686)
	va_cmd(IFCONFIG, 2, 1, ELANIF, "up");
#endif
	va_cmd(IFCONFIG, 2, 1, BRIF, "up");
#endif

	cleanAllFirewallRule();

#if defined(CONFIG_RTK_RG_INIT)
	/* Turn switch WAN port down */
	//RG_wan_phy_force_power_down(1);
#endif

#ifdef CONFIG_RTK_RG_INIT
{
	int EntryID, ret;
	MIB_CE_ATM_VC_T tmp;
	ret = RG_check_Droute(configAll,pEntry,&EntryID);
	if(ret == 3){
	/*remove, modify D route case!!!!*/
//AUG_PRT("%s-%d EntryID=%d\n",__func__,__LINE__,EntryID);
		if (!mib_chain_get(MIB_ATM_VC_TBL, EntryID, (void*)&tmp))
			return -1;
//AUG_PRT("%s-%d tmp.enable=%d\n",__func__,__LINE__,tmp.enable);
		deleteConnection(CONFIGONE, &tmp);// Modify or remove D route
		startWan(CONFIGONE, &tmp);
	}
//AUG_PRT("%s-%d\n",__func__,__LINE__);
}
#endif
	if (configAll == CONFIGALL)
		startWan(CONFIGALL, NULL);
	else if (configAll == CONFIGONE)
		startWan(CONFIGONE, pEntry);

#if defined(CONFIG_RTK_RG_INIT)
	/* Turn switch WAN port up */
	//RG_wan_phy_force_power_down(0);
#endif

	mib_get(MIB_MPMODE, (void *)&vChar);

#ifdef NEW_PORTMAPPING
	setupnewEth2pvc();
#endif

	// setup IP QoS
#ifdef IP_QOS
	if (vChar&MP_IPQ_MASK) {
		stopIPQ();
		setupIPQ();
	}
#endif

// Mason Yu. SIGRTMIN for DHCP Relay.catch SIGRTMIN(090605)
#ifdef COMBINE_DHCPD_DHCRELAY
	dhcrelay_pid = read_pid(DHCPSERVERPID);
#else
	dhcrelay_pid = read_pid("/var/run/dhcrelay.pid");
#endif
	if (dhcrelay_pid > 0) {
		printf("restartWAN1: kick dhcrelay(%d) to Re-discover all network interface\n", dhcrelay_pid);
		// Mason Yu. catch SIGRTMIN(It is a real time signal and number is 32) for re-sync all interface on DHCP Relay
		// Kaohj -- SIGRTMIN is not constant, so we use 32.
		//kill(dhcrelay_pid, SIGRTMIN);
		kill(dhcrelay_pid, 32);
	}

	// Mason Yu.
#ifdef CONFIG_IPV6

	if (mib_get(MIB_IPV6_LAN_IP_ADDR, (void *)tmpBuf) != 0)
	{
		char cmdBuf[100]={0};
		sprintf(cmdBuf, "%s/%d", tmpBuf, 64);
		va_cmd(IFCONFIG, 3, 1, LANIF, ARG_ADD, cmdBuf);
	}

#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
	if(configAll || ( pEntry && (pEntry->IpProtocol & IPVER_IPV6)))
	{
		restartDHCPV6Server();
	}
#endif
#endif

#ifdef IP_QOS
	update_qos_tbl();
#endif

	//ql 20081118 START restart IP QoS
#ifdef NEW_IP_QOS_SUPPORT
	take_qos_effect();
#endif
#ifdef CONFIG_USER_IP_QOS_3
	take_qos_effect_v3();
#endif
#ifdef CONFIG_RTL8672_SAR
	itfcfg("sar", 1);
#endif

#ifdef CONFIG_USER_XFRM
	ipsec_take_effect();
#endif

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
#ifdef CONFIG_USER_PPTPD_PPTPD
	pptpd_take_effect();
#endif
#endif

#ifdef CONFIG_USER_L2TPD_LNS
	l2tpd_take_effect();
#endif

#ifndef MAC_FILTER_SRC_ONLY
	setupMacFilterEbtables();
#endif
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
{
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE) || defined(CONFIG_FIBER_FEATURE)
	int pon_mode=0, acl_default=0;
	if (mib_get(MIB_PON_MODE, (void *)&pon_mode) != 0)
	{
#ifdef CONFIG_RTL9602C_SERIES
		acl_default = 1;
#endif
		if ((pon_mode != GPON_MODE) || acl_default == 1)
		{
			RG_del_All_Acl_Rules();
			RG_add_default_Acl_Qos();
		}
	}
#else
	/*use for 8696*/
	RG_del_All_Acl_Rules();
	RG_add_default_Acl_Qos();
#endif
}
#endif
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
	RG_del_PPPoE_Acl();
	RG_add_PPPoE_RB_passthrough_Acl();
	Flush_RTK_RG_IPv4_IPv6_Vid_Binding_ACL();
	RTK_RG_Set_IPv4_IPv6_Vid_Binding_ACL();
	RTK_RG_FLUSH_Bridge_DHCP_ACL_FILE();
	RTK_RG_Set_ACL_Bridge_DHCP_Filter();
#endif
	setupMacFilterTables();
}

/*
 *	Deal with configuration dependency when a WAN channel has been deleted.
 */
void resolveServiceDependency(unsigned int idx)
{
	MIB_CE_ATM_VC_T Entry;
#ifdef CONFIG_USER_IGMPPROXY
	unsigned int igmp_proxy_itf;
	unsigned char igmp_enable;
#endif
#ifdef IP_PASSTHROUGH
	unsigned int ippt_itf;
	unsigned int ippt_lease;
#endif
#ifdef CONFIG_USER_MINIUPNPD
	unsigned char upnpdEnable;
	unsigned int upnpItf;
#endif
	struct data_to_pass_st msg;
	int k;
#ifdef TIME_ZONE
	unsigned int if_wan = DUMMY_IFINDEX;
#endif

	/* get the specified chain record */
	if (!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&Entry))
	{
		return;
	}

#ifdef CONFIG_USER_IGMPPROXY
	// resolve IGMP proxy dependency
	if(!mib_get( MIB_IGMP_PROXY_ITF,  (void *)&igmp_proxy_itf))
		return;
	if (Entry.ifIndex == igmp_proxy_itf)
	{ // This interface is IGMP proxy interface
		igmp_proxy_itf = DUMMY_IFINDEX;	// set to default
		mib_set(MIB_IGMP_PROXY_ITF, (void *)&igmp_proxy_itf);
		igmp_enable = 0;	// disable IGMP proxy
		mib_set(MIB_IGMP_PROXY, (void *)&igmp_enable);
	}
#endif

#ifdef IP_PASSTHROUGH
	// resolve IP passthrough dependency
	if(!mib_get( MIB_IPPT_ITF,  (void *)&ippt_itf))
		return;
	if (Entry.ifIndex == ippt_itf)
	{ // This interface is IP passthrough interface
		ippt_itf = DUMMY_IFINDEX;	// set to default
		mib_set(MIB_IPPT_ITF, (void *)&ippt_itf);
		ippt_lease = 600;	// default to 10 min.
		mib_set(MIB_IPPT_LEASE, (void *)&ippt_lease);
	}
#endif

#ifdef CONFIG_USER_MINIUPNPD
	if (mib_get(MIB_UPNP_DAEMON, &upnpdEnable) && upnpdEnable) {
		if (mib_get(MIB_UPNP_EXT_ITF, &upnpItf)
				&& upnpItf == Entry.ifIndex) {
			upnpdEnable = 0;
			mib_set(MIB_UPNP_DAEMON, &upnpdEnable);
		}
	}
#endif

	delPortForwarding( Entry.ifIndex );
	delRoutingTable( Entry.ifIndex );
#ifdef CONFIG_USER_ROUTED_ROUTED
	delRipTable(Entry.ifIndex);
#endif
#ifdef TIME_ZONE
	mib_get(MIB_NTP_IF_WAN, &if_wan);
	if (if_wan == Entry.ifIndex) {
		stopNTP();
		if_wan = DUMMY_IFINDEX;
		mib_set(MIB_NTP_IF_WAN, &if_wan);
	}
#endif
	delPPPoESession(Entry.ifIndex);
#ifdef CONFIG_MIDDLEWARE
	//setapplicationtype_mw(&Entry,1,NULL); // star: del route rule for tr069 type wan connection
#endif

}

#ifdef IP_PASSTHROUGH
static void set_IPPT_LAN_access()
{
	unsigned int ippt_itf;
	unsigned char value[32];

	// IP Passthrough, LAN access
	if (mib_get(MIB_IPPT_ITF, (void *)&ippt_itf) != 0)
		if (ippt_itf != DUMMY_IFINDEX) {	// IP passthrough
			mib_get(MIB_IPPT_LANACC, (void *)value);
			if (value[0] == 0)	// disable LAN access
				// iptables -A FORWARD -i $LAN_IF -o $LAN_IF -j DROP
				va_cmd(IPTABLES, 8, 1, (char *)FW_ADD,
					(char *)FW_FORWARD, (char *)ARG_I,
					(char *)LANIF, (char *)ARG_O,
					(char *)LANIF, "-j", (char *)FW_DROP);
		}
}

static void clean_IPPT_LAN_access()
{
	// iptables -D FORWARD -i $LAN_IF -o $LAN_IF -j DROP
	va_cmd(IPTABLES, 8, 1, (char *)FW_DEL,
		(char *)FW_FORWARD, (char *)ARG_I,
		(char *)LANIF, (char *)ARG_O,
		(char *)LANIF, "-j", (char *)FW_DROP);
}

// Mason Yu
void restartIPPT(struct ippt_para para)
{
	unsigned int entryNum, i, idx, vcIndex, selected;
	MIB_CE_ATM_VC_T Entry;
	FILE *fp;
	int restar_dhcpd_flag=0, isPPPoE=0;
	unsigned long myipaddr, hisipaddr;
	char pppif[6], globalIP_str[16];
	MEDIA_TYPE_T mType;

	//printf("Take effect for IPPT and old_ippt_itf=%d new_ippt_itf=%d\n", para.old_ippt_itf, para.new_ippt_itf);
	//printf("Take effect for IPPT and old_ippt_lease=%d new_ippt_lease=%d\n", para.old_ippt_lease, para.new_ippt_lease);
	//printf("Take effect for IPPT and old_ippt_lanacc=%d new_ippt_lanacc=%d\n", para.old_ippt_lanacc, para.new_ippt_lanacc);

       	// Stop IPPT
       	// If old_ippt_itf != 255 and new_ippt_itf != old_ippt_itf, it is that the IPPT is enabled. We should clear some configurations.
	if ( para.old_ippt_itf != DUMMY_IFINDEX  && para.new_ippt_itf != para.old_ippt_itf) {
		// (1)  set restart DHCP server flag with 1.
		restar_dhcpd_flag = 1;  // on restart DHCP server flag

		// (2) Delete /tmp/PPPHalfBridge file for DHCP Server
       		fp = fopen("/tmp/PPPHalfBridge", "r");
       		if (fp) {
       			fread(&myipaddr, 4, 1, fp);
	 	       	fread(&hisipaddr, 4, 1, fp);
       			unlink("/tmp/PPPHalfBridge");
       			fclose(fp);
       		}

       		// (3) Delete /tmp/IPoAHalfBridge file for DHCP Server
       		fp = fopen("/tmp/IPoAHalfBridge", "r");
       		if (fp) {
       			fread(&myipaddr, 4, 1, fp);
	 	       	fread(&hisipaddr, 4, 1, fp);
       			unlink("/tmp/IPoAHalfBridge");
       			fclose(fp);
       		}

		// Change Public IP to string
		snprintf(globalIP_str, 16, "%d.%d.%d.%d", (int)(myipaddr>>24)&0xff, (int)(myipaddr>>16)&0xff, (int)(myipaddr>>8)&0xff, (int)(myipaddr)&0xff);

		// (4) Delete LAN IPPT interface
		va_cmd(IFCONFIG, 2, 1, (char*)LAN_IPPT,"down");

		// (5) Delete a public IP's route
               	va_cmd(ROUTE, 5, 1, ARG_DEL, "-host", globalIP_str, "dev", LANIF);

		// (6) Restart the previous IPPT WAN connection.
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i = 0; i < entryNum; i++) {
			/* Retrieve entry */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
				printf("restartIPPT: cannot get ATM_VC_TBL-1(ch=%d) entry\n", i);
				return;
			}

			/* remove connection on driver*/
			if (para.old_ippt_itf == Entry.ifIndex) {
				stopConnection(&Entry);

				// If this connection is PPPoE/oA, we should kill the old NAPT rule in POSTROUTING chain.
				// When channel mode is rt1483, NAPT rule will be deleted by stopConnection().
				if (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA) {
					snprintf(pppif, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
        				va_cmd(IPTABLES, 10, 1, "-t", "nat", FW_DEL, "POSTROUTING",
			 			"-o", pppif, "-j", "SNAT", "--to-source", globalIP_str);
				}

				if (Entry.cmode == CHANNEL_MODE_PPPOE) {
					isPPPoE = 1;
					vcIndex = VC_INDEX(Entry.ifIndex);
					mType = MEDIA_INDEX(Entry.ifIndex);
					selected = i;
				}

#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
				addEthWANdev(&Entry);
#endif
				startConnection(&Entry, i);
				break;
			}
		}
	}

	if (isPPPoE) {
		for (i=0; i<entryNum; i++) {
			if (i == selected)
				continue;
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
				printf("restartIPPT: cannot get ATM_VC_TBL-1(ch=%d) entry\n", i);
				return;
			}
			if (mType == MEDIA_INDEX(Entry.ifIndex) && vcIndex == VC_INDEX(Entry.ifIndex)) {	// Jenny, for multisession PPPoE support
				startConnection(&Entry, i);
			}
		}
	}

	// Start New IPPT
	if ( para.new_ippt_itf != DUMMY_IFINDEX && para.new_ippt_itf != para.old_ippt_itf) {
		// (1)  set restart DHCP server flag with 1.
		restar_dhcpd_flag = 1;  // on restart DHCP server flag

		// (2) Config WAN interface and reconnect to DSL.
		entryNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i = 0; i < entryNum; i++) {
			/* Retrieve entry */
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
				printf("restartIPPT: cannot get ATM_VC_TBL-2(ch=%d) entry\n", i);
				return;
			}

			if (para.new_ippt_itf == Entry.ifIndex) {
				// If this connection is PPPoE/oA, we should kill the old NAPT rule in POSTROUTING chain.
				// When channel mode is rt1483, NAPT rule will be deleted by stopConnection().
				if ( (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA)) {
					snprintf(pppif, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
					va_cmd(IPTABLES, 8, 1, "-t", "nat", FW_DEL, "POSTROUTING",
			 			"-o", pppif, "-j", "MASQUERADE");
				}
				stopConnection(&Entry);
				if (Entry.cmode == CHANNEL_MODE_PPPOE) {
					isPPPoE = 1;
					vcIndex = VC_INDEX(Entry.ifIndex);
					mType = MEDIA_INDEX(Entry.ifIndex);
					selected = i;
				}
#if defined(CONFIG_RTL_MULTI_ETH_WAN) || defined(CONFIG_PTMWAN)
				addEthWANdev(&Entry);
#endif
				startConnection(&Entry, i);
				break;
			}
		}
		if (isPPPoE) {
			for (i=0; i<entryNum; i++) {
				if (i == selected)
					continue;
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
					printf("restartIPPT: cannot get ATM_VC_TBL-1(ch=%d) entry\n", i);
					return;
				}
				if (mType == MEDIA_INDEX(Entry.ifIndex) && vcIndex == VC_INDEX(Entry.ifIndex)) { // Jenny, for multisession PPPoE support
					startConnection(&Entry, i);
				}
			}
		}

	}  //  if ( new_ippt_itf != 255 )

	// Check IPPT Lease Time
	// Here we just concern about IPPT is enable.
	if ( para.old_ippt_lease != para.new_ippt_lease && para.new_ippt_itf != DUMMY_IFINDEX) {
		restar_dhcpd_flag = 1;  // on restart DHCP server flag
		//printf("change IPPT Lease Time\n");
	}

	// Check IPPT LAN Access
	if ( para.new_ippt_itf != DUMMY_IFINDEX || para.old_ippt_itf != DUMMY_IFINDEX) {
		if ( para.old_ippt_lanacc == 0 && para.old_ippt_itf != DUMMY_IFINDEX)
			clean_IPPT_LAN_access();
		set_IPPT_LAN_access();
	}

	// Restart DHCP Server
	if ( restar_dhcpd_flag == 1 ) {
		restar_dhcpd_flag = 0;  // off restart DHCP server flag
		restart_dhcp();
	}

}
#endif

#ifdef DOS_SUPPORT
void setup_dos_protection(void)
{
	unsigned int dos_enable;
	unsigned int dos_syssyn_flood;
	unsigned int dos_sysfin_flood;
	unsigned int dos_sysudp_flood;
	unsigned int dos_sysicmp_flood;
	unsigned int dos_pipsyn_flood;
	unsigned int dos_pipfin_flood;
	unsigned int dos_pipudp_flood;
	unsigned int dos_pipicmp_flood;
	unsigned int dos_block_time;
	unsigned char buffer[256],dosstr[256];
	int dostmpip[4],dostmpmask[4];
	int dosip,dosmask;
	FILE *dosfp;

	if (!mib_get( MIB_DOS_ENABLED,  (void *)&dos_enable)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_SYSSYN_FLOOD,  (void *)&dos_syssyn_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_SYSFIN_FLOOD,  (void *)&dos_sysfin_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_SYSUDP_FLOOD,  (void *)&dos_sysudp_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_SYSICMP_FLOOD,  (void *)&dos_sysicmp_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_PIPSYN_FLOOD,  (void *)&dos_pipsyn_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_PIPFIN_FLOOD,  (void *)&dos_pipfin_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_PIPUDP_FLOOD,  (void *)&dos_pipudp_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_PIPICMP_FLOOD,  (void *)&dos_pipicmp_flood)){
		printf("DOS parameter failed!\n");
	}
	if (!mib_get( MIB_DOS_BLOCK_TIME,  (void *)&dos_block_time)){
		printf("DOS parameter failed!\n");
	}

	//get ip
	if(!mib_get( MIB_ADSL_LAN_IP, (void *)buffer))
		return -1;
	sscanf(inet_ntoa(*((struct in_addr *)buffer)),"%d.%d.%d.%d",&dostmpip[0],&dostmpip[1],&dostmpip[2],&dostmpip[3]);
	dosip= dostmpip[0]<<24 | dostmpip[1]<<16 | dostmpip[2]<<8 | dostmpip[3];
	//get mask
	if(!mib_get( MIB_ADSL_LAN_SUBNET, (void *)buffer))
		return -1;
	sscanf(inet_ntoa(*((struct in_addr *)buffer)),"%d.%d.%d.%d",&dostmpmask[0],&dostmpmask[1],&dostmpmask[2],&dostmpmask[3]);
	dosmask= dostmpmask[0]<<24 | dostmpmask[1]<<16 | dostmpmask[2]<<8 | dostmpmask[3];
	dosip &= dosmask;
	sprintf(dosstr,"1 %x %x %d %d %d %d %d %d %d %d %d %d\n",
		dosip,dosmask,dos_enable,dos_syssyn_flood,dos_sysfin_flood,dos_sysudp_flood,
		dos_sysicmp_flood,dos_pipsyn_flood,dos_pipfin_flood,dos_pipudp_flood,
		dos_pipicmp_flood,dos_block_time);
	//printf("dosstr:%s\n\n",dosstr);
	dosstr[strlen(dosstr)]=NULL;
	if (dosfp = fopen("/proc/enable_dos", "w"))
	{
			fprintf(dosfp, "%s",dosstr);
			fclose(dosfp);
	}
}
#endif

#ifdef MULTI_IC_SUPPORT
/*
* ql:20080729 START: get register value(4 bytes)
* parameter: addr - register address
*/
static int getRegValue(unsigned int addr)
{
	int skfd;
	struct atmif_sioc mysio;
	unsigned int ptr[3];

	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return -1;
	}

	ptr[0] = addr;
	ptr[1] = 4;
	mysio.number = 0;
	mysio.length = sizeof(struct SAR_IOCTL_CFG);
	mysio.arg = (void *)ptr;
	if ((ioctl(skfd, SAR_READ_MEM, &mysio))<0) {
		printf("read reg %08X failed!\n", addr);
		close(skfd);
		return -1;
	}

	close(skfd);

	return(ptr[2]);
}

/*
* ql: 20080729 START: use getImgKey() to gain the right key of relevant IC version.
*/
int getImgKey(void)
{
	unsigned int key;
	unsigned int reg_clk;
	unsigned short sachem_ver_reg;
	int icVer;

	sachem_ver_reg = (unsigned short)getRegValue(0xb8600020);
	if (-1 == sachem_ver_reg)
		return 0;

	if (!sachem_ver_reg) {//I think it must be 8671, read value to ensure
		sachem_ver_reg = (unsigned short)getRegValue(0xb8000020);
		switch(sachem_ver_reg) {
			case 0x1a08:
			case 0x081a:
			case 0x1a0a:
			case 0x0a1a:
			case 0x1a1a:
			case 0x1a2a:
			case 0x2a1a:
				icVer = IC8671;
				break;
			default:
				printf("unknown sachem version!\n");
				icVer = 0;
				break;
		}
	} else {
		unsigned int reg_clk;

		reg_clk = getRegValue(0xB8003200);
		if (-1 == reg_clk)
			icVer = 0;
		else if ((reg_clk & 0x00001f00) == 0)
			icVer = IC8672;
		else if ((reg_clk & 0x00100000) == 0x00100000)
			icVer = IC8671B;
		else
			icVer = IC8671B_costdown;
	}

	switch(icVer)
	{
		case IC8671:
			key = APPLICATION_IMG_8671;
			break;
		//case IC8671P:
		//	key = APPLICATION_IMG_8671P;
		//	break;
		case IC8672:
			key = APPLICATION_IMG_8672;
			break;
		case IC8671B:
			key = APPLICATION_IMG_8671B;
			break;
		case IC8671B_costdown:
			key = APPLICATION_IMG_8671B_CD;
			break;
		default:
			/*ql:20080729 START: if sachem register read fail, then don't check image key*/
			//key = 0;
			key = APPLICATION_IMG_ALL;
			/*ql:20080729 END*/
			break;
	}
	printf("%s:%08X\n", __FUNCTION__, key);

	return(key);
}
#endif

/*ql: 20080729 END*/

int pppdbg_get(int unit)
{
	char buff[256];
	FILE *fp;
	int pppinf, pppdbg = 0;

	if (fp = fopen(PPP_DEBUG_LOG, "r")) {
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			if (sscanf(buff, "%d:%d", &pppinf, &pppdbg) != 2)
				break;
			else {
				if (pppinf == unit)
					break;
			}
		}
		fclose(fp);
	}
	return pppdbg;
}

//cathy, update pvc link time
struct sysinfo * updateLinkTime(unsigned char update)
{
	static struct sysinfo info;

	if(update) {	// down --> up
		sysinfo(&info);
	}

	return &info;
}

#ifdef CONFIG_USER_XDSL_SLAVE
struct sysinfo * updateSlvLinkTime(unsigned char update)
{
	static struct sysinfo info;

	if(update) {	// down --> up
		sysinfo(&info);
	}

	return &info;
}
#endif /*CONFIG_USER_XDSL_SLAVE*/

#ifdef CONFIG_ATM_CLIP
void sendAtmInARPRep(unsigned char update)
{
	int i, totalEntry;
	MIB_CE_ATM_VC_T Entry;
	struct data_to_pass_st msg;
	char wanif[5];
	unsigned long addr;

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	if (update) {	// down --> up
		for (i=0; i<totalEntry; i++) {
			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry);
			if (Entry.enable == 0)
				continue;
			if (Entry.cmode == CHANNEL_MODE_RT1577) {
				addr = *((unsigned long *)Entry.remoteIpAddr);
				snprintf(wanif, 5, "vc%d", VC_INDEX(Entry.ifIndex));
				snprintf(msg.data, BUF_SIZE, "mpoactl set %s inarprep %lu", wanif, addr);
				write_to_mpoad(&msg);
			}
		}
	}
}
#endif

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
void Commit()
{
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
}
#endif // of #if COMMIT_IMMEDIATELY

#ifdef CONFIG_USER_SAMBA
int startSamba(void)
{
	unsigned char samba_enable;
#ifdef CONFIG_MULTI_SMBD_ACCOUNT
	char devName[MAX_NAME_LEN];
	char cmdStr[200];
#endif
	char *argv[10],
#ifdef CONFIG_USER_NMBD
	     nbn[MAX_SAMBA_NETBIOS_NAME_LEN], nbn_opt[MAX_SAMBA_NETBIOS_NAME_LEN + 32],
#endif
	     ss[MAX_SAMBA_SERVER_STRING_LEN], ss_opt[MAX_SAMBA_SERVER_STRING_LEN + 32];
	int i = -1, ret = 0;

	mib_get(MIB_SAMBA_ENABLE, &samba_enable);
	if (!samba_enable) {
		return -1;
	}

	//QL 20160127: set hostname for samba daemon
#ifdef CONFIG_MULTI_SMBD_ACCOUNT
	mib_get(MIB_DEVICE_NAME, (void *)devName);
	snprintf(cmdStr, 200, "echo %s > /proc/sys/kernel/hostname", devName);
	va_cmd("/bin/sh", 2, 1, "-c", cmdStr);
#endif//end of CONFIG_MULTI_SMBD_ACCOUNT

#ifdef CONFIG_USER_NMBD
	mib_get(MIB_SAMBA_NETBIOS_NAME, nbn);
#endif
	mib_get(MIB_SAMBA_SERVER_STRING, ss);

	/* set NetBIOS Name and Server String via command line arguments */
#ifdef CONFIG_USER_NMBD
	sprintf(nbn_opt, "--option=netbios name=%s", nbn);
#endif
	sprintf(ss_opt, "--option=server string=%s", ss);

	argv[++i] = "/bin/nmbd";
#ifdef CONFIG_USER_NMBD
	argv[++i] = nbn_opt;
#endif
	argv[++i] = ss_opt;
	argv[++i] = NULL;

#ifdef CONFIG_USER_NMBD
	ret = do_cmd(argv[0], argv, 0);
#endif

	argv[0] = "/bin/smbd";
	printf("%s %d\n", __func__, __LINE__);
	ret |= do_cmd(argv[0], argv, 0);

	return ret;
}

int stopSamba(void)
{
	pid_t pid;

#ifdef CONFIG_USER_NMBD
	pid = read_pid(NMBDPID);
	if (pid > 0) {
		/* nmbd is running */
		kill(pid, 9);
		unlink(NMBDPID);
	}
#endif

	pid = read_pid(SMBDPID);
	if (pid > 0) {
		/* smbd is running */
		/* using -pid would kill all processes whose process group ID is pid */
		kill(-pid, 9);
		unlink(SMBDPID);
	}

	return 0;
}
#endif // CONFIG_USER_SAMBA

#ifdef CONFIG_USER_CUPS
int getPrinterList(char *str, size_t size)
{
	char strbuf[BUF_SIZE], serverIP[INET_ADDRSTRLEN], *substr, *chr;
	FILE *fp;
	int offset;
	struct in_addr inAddr;

	if (getInAddr((char *)LANIF, IP_ADDR, &inAddr) == 1) {
		strncpy(serverIP, inet_ntoa(inAddr), sizeof(serverIP));
	} else {
		getMIB2Str(MIB_ADSL_LAN_IP, serverIP);
	}

	offset = 0;
	fp = fopen(CUPSDPRINTERCONF, "r");

	while (fgets(strbuf, sizeof(strbuf), fp)) {
		chr = strchr(strbuf, '#');

		/* search the pattern '<DefaultPrinter lp0>' or '<Printer lp0>' */
		if (substr = strstr(strbuf, "Printer ")) {
			if (chr && chr < substr) {
				/* in comment */
				continue;
			}

			/*
			 * the length of "Printer " is 8,
			 * now substr points to the start of the printer name
			 */
			substr += 8;

			if (chr = strchr(substr, '>'))
				*chr = '\0';

			offset += snprintf(str + offset, size - offset,
					"http://%s:631/printers/%s\n",
					serverIP, substr);
		}
	}

	fclose(fp);

	return offset;
}
#endif // CONFIG_USER_CUPS

#ifdef CONFIG_USER_MINIDLNA
// success: return 1;
// fail: return 0;
int get_dlna_db_dir(char *db_dir)
{
	FILE *fp;
	char buf[256]="";
	int ret=0;

	fp = fopen("/proc/mounts", "r");
	if (fp) {
		while (fgets(buf, sizeof(buf), fp)) {
			if (strstr(buf, "/dev/sd")) {
				sscanf(buf,"%*s %s", db_dir);
				sprintf(db_dir, "%s/minidlna", db_dir);
				//warn("get_db_dir: db_dir=%s\n", db_dir);
				// Format is : /dev/sdb /var/mnt/sdb vfat .....................
				ret = 1;
				break;
			}
		}
		fclose(fp);
	}

	return ret;
}

static void createMiniDLNAconf(char *name, char *directory)
{
	FILE *fp;

	fp = fopen(name, "w");
	if(fp) {
		fputs("port=8200\n",fp);
		fputs("network_interface=br0\n", fp);
		fprintf(fp, "media_dir=/mnt\n");
		fprintf(fp, "db_dir=%s\n",directory);
		fputs("friendly_name=My DLNA Server\n", fp);
		fputs("album_art_names=Cover.jpg/cover.jpg/AlbumArtSmall.jpg/albumartsmall.jpg/AlbumArt.jpg/albumart.jpg/Album.jpg/album.jpg/Folder.jpg/folder.jpg/Thumb.jpg/thumb.jpg\n", fp);
		fputs("inotify=yes\n", fp);
		fputs("enable_tivo=no\n", fp);
		fputs("strict_dlna=no\n", fp);
		fputs("notify_interval=10\n", fp);	// 900
		fputs("serial=12345678\n", fp);
		fputs("model_number=1\n", fp);
		fclose(fp);
	}
	return;
}

void startMiniDLNA(void)
{
	char *argv[10];
	int i = 0, pid = 0;
	//MIB_DMS_T entry,*p;
	unsigned int enable;
	char db_dir[32]="";

	if(read_pid((char *)MINIDLNAPID)>0){
		printf( "%s: already start, line=%d\n", __FUNCTION__, __LINE__ );
		return;
	}

	if (!get_dlna_db_dir(db_dir))
		return;;

	// Mason Yu. use table not chain
	mib_get(MIB_DMS_ENABLE, (void *)&enable);
	if(!enable) {
		printf( "%s: is diaabled, line=%d\n", __FUNCTION__, __LINE__ );
		return;
	}
	createMiniDLNAconf("/var/minidlna.conf", db_dir);

	argv[i++]="/bin/minidlna";
//	argv[i++]="-d";
	argv[i++]="-R";
	argv[i++]="-f";
	argv[i++]="/var/minidlna.conf";
	argv[i]=NULL;
	do_cmd( argv[0], argv, 0 );

	while(read_pid((char*)MINIDLNAPID) < 0)
	{
		//printf("MINIDLNA is not running. Please wait!\n");
		usleep(300000);
	}
}

void stopMiniDLNA(void)
{
	int i, pid = 0;
	pid = read_pid((char *)MINIDLNAPID);
	if(pid <= 0){
		printf( "%s: already stop, line=%d\n", __FUNCTION__, __LINE__ );
		return;
	}

	//killpg(pid, 9);
	kill(pid, 15);
}
#endif

#ifdef CONFIG_IPV6

int ifPrefixUsingPD()
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return 0;
		}

		if ((Entry.Ipv6DhcpRequest&0x2) == 0x2){
			printf("Will request IPv6 prefix, 0x%x\n",Entry.ifIndex);
			return 1;
		}
	}

	return 0;
}

int ifIPv6LanPrefixConfigured()
{
	unsigned char str[MAX_RADVD_CONF_PREFIX_LEN];
	unsigned char vChar;

	if ( !mib_get(MIB_IPV6_LAN_PREFIX, (void *)str)) {
		return 0;
	}
	if ( !mib_get(MIB_IPV6_LAN_PREFIX_LEN, (void *)&vChar)) {
		return 0;
	}

	if(str[0] && (vChar!=0))
		return 1;

	return 0;
}

#ifdef CONFIG_USER_RADVD

// Added by Mason Yu for p2r_test
void init_radvd_conf_mib(void)
{
	unsigned char vChar;

	// MaxRtrAdvIntervalAct
	mib_set(MIB_V6_MAXRTRADVINTERVAL, "");

	// MinRtrAdvIntervalAct
	mib_set(MIB_V6_MINRTRADVINTERVAL, "");

	// AdvCurHopLimitAct
	mib_set(MIB_V6_ADVCURHOPLIMIT, "");

	// AdvDefaultLifetime
	mib_set(MIB_V6_ADVDEFAULTLIFETIME, "");

	// AdvReachableTime
	mib_set(MIB_V6_ADVREACHABLETIME, "");

	// AdvRetransTimer
	mib_set(MIB_V6_ADVRETRANSTIMER, "");

	// AdvLinkMTU
	mib_set(MIB_V6_ADVLINKMTU, "");

	// AdvSendAdvert
	vChar = 1;
	mib_set( MIB_V6_SENDADVERT, (void *)&vChar);      // on

	// AdvManagedFlag
	vChar = 2;
	mib_set( MIB_V6_MANAGEDFLAG, (void *)&vChar );     // ignore

	// AdvOtherConfigFlag
	vChar = 2;
	mib_set( MIB_V6_OTHERCONFIGFLAG, (void *)&vChar ); // ignore

    // RDNSS
	mib_set(MIB_V6_RDNSS1, "");
	mib_set(MIB_V6_RDNSS2, "");

	// Prefix
	mib_set( MIB_V6_PREFIX_MODE, "1");    // It must have Prefix IP

	mib_set(MIB_V6_PREFIX_IP, "3ffe:501:ffff:100::");
	mib_set(MIB_V6_PREFIX_LEN, "64");

	// AdvValidLifetime
	mib_set(MIB_V6_VALIDLIFETIME, "2592000");

	// AdvPreferredLifetime
	mib_set(MIB_V6_PREFERREDLIFETIME, "604800");

	// AdvOnLink
	vChar = 2;
	mib_set( MIB_V6_ONLINK, (void *)&vChar );    // ignore

	// AdvAutonomous
	vChar = 2;
	mib_set( MIB_V6_AUTONOMOUS, (void *)&vChar ); // ignore
}

int setup_radvd_conf_new(DNS_V6_INFO_Tp pDnsV6Info, PREFIX_V6_INFO_Tp pPrefixInfo)
{
	FILE *fp;
	unsigned char str[MAX_RADVD_CONF_PREFIX_LEN];
	unsigned char str2[MAX_RADVD_CONF_PREFIX_LEN];
	unsigned char vChar,vChar2;
	int radvdpid;

	if(!pDnsV6Info || !pPrefixInfo){
		printf("Error! NULL parameter input %s\n");
		return -1;
	}

	if ((fp = fopen(RADVD_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", RADVD_CONF);
		return -1;
	}

	fprintf(fp, "interface br0\n");
	fprintf(fp, "{\n");

	// AdvSendAdvert
	if ( !mib_get( MIB_V6_SENDADVERT, (void *)&vChar) )
		printf("Get MIB_V6_SENDADVERT error!");
	if (0 == vChar)
		fprintf(fp, "\tAdvSendAdvert off;\n");
	else if (1 == vChar)
		fprintf(fp, "\tAdvSendAdvert on;\n");

	// MaxRtrAdvIntervalAct
	if ( !mib_get(MIB_V6_MAXRTRADVINTERVAL, (void *)str)) {
		printf("Get MaxRtrAdvIntervalAct mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tMaxRtrAdvInterval %s;\n", str);
	}

	// MinRtrAdvIntervalAct
	if ( !mib_get(MIB_V6_MINRTRADVINTERVAL, (void *)str)) {
		printf("Get MinRtrAdvIntervalAct mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tMinRtrAdvInterval %s;\n", str);
	}

	// AdvCurHopLimitAct
	if ( !mib_get(MIB_V6_ADVCURHOPLIMIT, (void *)str)) {
		printf("Get AdvCurHopLimitAct mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tAdvCurHopLimit %s;\n", str);
	}

	// AdvDefaultLifetime
	if ( !mib_get(MIB_V6_ADVDEFAULTLIFETIME, (void *)str)) {
		printf("Get AdvDefaultLifetime mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tAdvDefaultLifetime %s;\n", str);
	}

	// AdvReachableTime
	if ( !mib_get(MIB_V6_ADVREACHABLETIME, (void *)str)) {
		printf("Get AdvReachableTime mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tAdvReachableTime %s;\n", str);
	}

	// AdvRetransTimer
	if ( !mib_get(MIB_V6_ADVRETRANSTIMER, (void *)str)) {
		printf("Get AdvRetransTimer mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tAdvRetransTimer %s;\n", str);
	}

	// AdvLinkMTU
	if ( !mib_get(MIB_V6_ADVLINKMTU, (void *)str)) {
		printf("Get AdvLinkMTU mib error!");
	}
	if (str[0]) {
		int mtu = atoi(str);
		//Modify due to e8 Avalanche test , if MTU is specified (Like 1492), throughput will be bad.
		//But if there is PPPoE WAN, LAN PC should set the MTU, to prevent packet too big in IPv6.
		if( mtu>0 && isAnyPPPoEWan())
		fprintf(fp, "\tAdvLinkMTU %s;\n", str);
		else
			fprintf(fp, "\tAdvLinkMTU 0;\n");

	}

	// AdvManagedFlag
	if ( !mib_get( MIB_V6_MANAGEDFLAG, (void *)&vChar) )
		printf("Get MIB_V6_MANAGEDFLAG error!");
	if (0 == vChar)
		fprintf(fp, "\tAdvManagedFlag off;\n");
	else if (1 == vChar)
		fprintf(fp, "\tAdvManagedFlag on;\n");

	// AdvOtherConfigFlag
	if ( !mib_get( MIB_V6_OTHERCONFIGFLAG, (void *)&vChar) )
		printf("Get MIB_V6_OTHERCONFIGFLAG error!");
	if (0 == vChar)
		fprintf(fp, "\tAdvOtherConfigFlag off;\n");
	else if (1 == vChar)
		fprintf(fp, "\tAdvOtherConfigFlag on;\n");

	//NOTE: in radvd.conf
	//      Prefix/clients/route/RDNSS configurations must be given in exactly this order.
	// ULA Prefix
	mib_get (MIB_V6_ULAPREFIX_ENABLE, (void *)&vChar);
	if (vChar!=0) {
		unsigned char validtime[MAX_RADVD_CONF_PREFIX_LEN];
		unsigned char preferedtime[MAX_RADVD_CONF_PREFIX_LEN];

		if ( !mib_get(MIB_V6_ULAPREFIX, (void *)str)       ||
				!mib_get(MIB_V6_ULAPREFIX_LEN, (void *)str2)  ||
				!mib_get(MIB_V6_ULAPREFIX_VALID_TIME, (void *)validtime)  ||
				!mib_get(MIB_V6_ULAPREFIX_PREFER_TIME, (void *)preferedtime)
		   )
		{
			printf("Get ULAPREFIX mib error!");
		}
		else
		{
			unsigned char ip6Addr[IP6_ADDR_LEN];
			unsigned char devAddr[MAC_ADDR_LEN];
			unsigned char meui64[8];
			unsigned char value[64];
			int i;
#ifdef CONFIG_RTK_RG_INIT
			struct ipv6_ifaddr ip6_addr[6];
			char cur_ip6_addr[64];
#endif

			fprintf(fp, "\t\n");
			fprintf(fp, "\tprefix %s/%s\n", str, str2);
			fprintf(fp, "\t{\n");
			fprintf(fp, "\t\tAdvOnLink on;\n");
			fprintf(fp, "\t\tAdvAutonomous on;\n");
			fprintf(fp, "\t\tAdvValidLifetime %s;\n",validtime);
			fprintf(fp, "\t\tAdvPreferredLifetime %s;\n",preferedtime);
			fprintf(fp, "\t};\n");

#ifdef CONFIG_RTK_RG_INIT
			mib_get(MIB_LAN_IPV6_MODE1, (void *)&vChar);
			if(vChar == 0 ) // LAN IPv6 address mode is auto.
			{
#endif
				inet_pton(PF_INET6, str, (void *)ip6Addr);

				//setup LAN ULA v6 IP address according the ULA prefix + EUI64.
				mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
				mac_meui64(devAddr, meui64);
				for (i=0; i<8; i++)
					ip6Addr[i+8] = meui64[i];
				inet_ntop(PF_INET6, &ip6Addr, value, sizeof(value));
				sprintf(value, "%s/%s", value, str2);
				printf("Set LAN ULA %s\n",value);
#ifdef CONFIG_RTK_RG_INIT
			}
			else
			{
				mib_get(MIB_LAN_IPV6_ADDR1, (void *)ip6Addr);
				inet_ntop(PF_INET6, ip6Addr, value, sizeof(value));
				mib_get(MIB_LAN_IPV6_PREFIX_LEN1, (void *)&vChar2);
				sprintf(value, "%s/%d", value, vChar2);
			}

			getifip6(LANIF, IPV6_ADDR_UNICAST, ip6_addr, 6);
			inet_ntop(PF_INET6, &ip6_addr[0].addr, cur_ip6_addr, sizeof(cur_ip6_addr));
			sprintf(cur_ip6_addr, "%s/%d", cur_ip6_addr, ip6_addr[0].prefix_len);

			va_cmd(IFCONFIG, 3, 1, LANIF, "del", cur_ip6_addr);
			va_cmd(IFCONFIG, 3, 1, LANIF, "add", value);
#else
			va_cmd(IFCONFIG, 3, 1, LANIF, "del", value);
			va_cmd(IFCONFIG, 3, 1, LANIF, "add", value);
#endif
		}
	}

	// Prefix
	if(pPrefixInfo->prefixIP[0] && pPrefixInfo->prefixLen)
	{
		struct  in6_addr ip6Addr;
		unsigned char devAddr[MAC_ADDR_LEN];
		unsigned char meui64[8];
		unsigned char value[64];
		unsigned char prefixBuf[100]={0};
		int i;

		inet_ntop(PF_INET6,pPrefixInfo->prefixIP, prefixBuf, sizeof(prefixBuf));
		fprintf(fp, "\t\n");
		fprintf(fp, "\tprefix %s/%d\n", prefixBuf, pPrefixInfo->prefixLen);
		fprintf(fp, "\t{\n");

		memcpy(ip6Addr.s6_addr,pPrefixInfo->prefixIP,8);
		mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
		mac_meui64(devAddr, meui64);
		for (i=0; i<8; i++)
			ip6Addr.s6_addr[i+8] = meui64[i];
		inet_ntop(PF_INET6, &ip6Addr, value, sizeof(value));
		sprintf(value, "%s/%d", value, pPrefixInfo->prefixLen);
		va_cmd(IFCONFIG, 3, 1, LANIF, "del", value);
		va_cmd(IFCONFIG, 3, 1, LANIF, "add", value);

		// AdvOnLink
		if ( !mib_get( MIB_V6_ONLINK, (void *)&vChar) )
			printf("Get MIB_V6_ONLINK error!");
		if (0 == vChar)
			fprintf(fp, "\t\tAdvOnLink off;\n");
		else if (1 == vChar)
			fprintf(fp, "\t\tAdvOnLink on;\n");

		// AdvAutonomous
		if ( !mib_get( MIB_V6_AUTONOMOUS, (void *)&vChar) )
			printf("Get MIB_V6_AUTONOMOUS error!");
		if (0 == vChar)
			fprintf(fp, "\t\tAdvAutonomous off;\n");
		else if (1 == vChar)
			fprintf(fp, "\t\tAdvAutonomous on;\n");

		// AdvValidLifetime
		if ( !mib_get(MIB_V6_VALIDLIFETIME, (void *)str)) {
			printf("Get AdvValidLifetime mib error!");
		}
		if (str[0]) {
			fprintf(fp, "\t\tAdvValidLifetime %s;\n", str);
		}

		// AdvPreferredLifetime
		if ( !mib_get(MIB_V6_PREFERREDLIFETIME, (void *)str)) {
			printf("Get AdvPreferredLifetime mib error!");
		}
		if (str[0]) {
			fprintf(fp, "\t\tAdvPreferredLifetime %s;\n", str);
		}

			fprintf(fp, "\t\tDeprecatePrefix on;\n");
		fprintf(fp, "\t};\n");
	}

	//set RDNSS according to DNSv6 server setting
	if(pDnsV6Info && pDnsV6Info->nameServer){
		char *ptr=NULL;
		unsigned char nameServer[IPV6_BUF_SIZE_256];

		memcpy(nameServer,pDnsV6Info->nameServer,sizeof(nameServer));

		//  Replace ',' in string to meet RADVD CONF format
		//	RDNSS ip [ip] [ip] {    list of rdnss specific options
		//	};
		ptr=strchr(nameServer,',');
		if(ptr)
			ptr[0]=' ';

		fprintf(fp, "\n\tRDNSS %s\n", nameServer);
		fprintf(fp, "\t{\n");
		fprintf(fp, "\t\tAdvRDNSSPreference 8;\n");
		fprintf(fp, "\t\tAdvRDNSSOpen off;\n");
		fprintf(fp, "\t};\n");
	}

	fprintf(fp, "};\n");
	fclose(fp);
	return 0;

}

void setup_radvd_conf(int resync)
{
	FILE *fp;
	unsigned char str[MAX_RADVD_CONF_PREFIX_LEN];
	unsigned char str2[MAX_RADVD_CONF_PREFIX_LEN];
	unsigned char vChar,vChar2;
	int radvdpid;


	if ((fp = fopen(RADVD_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", RADVD_CONF);
		return;
	}

	fprintf(fp, "interface br0\n");
	fprintf(fp, "{\n");

	// AdvSendAdvert
	if ( !mib_get( MIB_V6_SENDADVERT, (void *)&vChar) )
		printf("Get MIB_V6_SENDADVERT error!");
	if (0 == vChar)
		fprintf(fp, "\tAdvSendAdvert off;\n");
	else if (1 == vChar)
		fprintf(fp, "\tAdvSendAdvert on;\n");

	// MaxRtrAdvIntervalAct
	if ( !mib_get(MIB_V6_MAXRTRADVINTERVAL, (void *)str)) {
		printf("Get MaxRtrAdvIntervalAct mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tMaxRtrAdvInterval %s;\n", str);
	}

	// MinRtrAdvIntervalAct
	if ( !mib_get(MIB_V6_MINRTRADVINTERVAL, (void *)str)) {
		printf("Get MinRtrAdvIntervalAct mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tMinRtrAdvInterval %s;\n", str);
	}

	// AdvCurHopLimitAct
	if ( !mib_get(MIB_V6_ADVCURHOPLIMIT, (void *)str)) {
		printf("Get AdvCurHopLimitAct mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tAdvCurHopLimit %s;\n", str);
	}

	// AdvDefaultLifetime
	if ( !mib_get(MIB_V6_ADVDEFAULTLIFETIME, (void *)str)) {
		printf("Get AdvDefaultLifetime mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tAdvDefaultLifetime %s;\n", str);
	}

	// AdvReachableTime
	if ( !mib_get(MIB_V6_ADVREACHABLETIME, (void *)str)) {
		printf("Get AdvReachableTime mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tAdvReachableTime %s;\n", str);
	}

	// AdvRetransTimer
	if ( !mib_get(MIB_V6_ADVRETRANSTIMER, (void *)str)) {
		printf("Get AdvRetransTimer mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tAdvRetransTimer %s;\n", str);
	}

	// AdvLinkMTU
	if ( !mib_get(MIB_V6_ADVLINKMTU, (void *)str)) {
		printf("Get AdvLinkMTU mib error!");
	}
	if (str[0]) {
		fprintf(fp, "\tAdvLinkMTU %s;\n", str);
	}

	// AdvManagedFlag
	if ( !mib_get( MIB_V6_MANAGEDFLAG, (void *)&vChar) )
		printf("Get MIB_V6_MANAGEDFLAG error!");
	if (0 == vChar)
		fprintf(fp, "\tAdvManagedFlag off;\n");
	else if (1 == vChar)
		fprintf(fp, "\tAdvManagedFlag on;\n");

	// AdvOtherConfigFlag
	if ( !mib_get( MIB_V6_OTHERCONFIGFLAG, (void *)&vChar) )
		printf("Get MIB_V6_OTHERCONFIGFLAG error!");
	if (0 == vChar)
		fprintf(fp, "\tAdvOtherConfigFlag off;\n");
	else if (1 == vChar)
		fprintf(fp, "\tAdvOtherConfigFlag on;\n");

	//NOTE: in radvd.conf
	//      Prefix/clients/route/RDNSS configurations must be given in exactly this order.
	// ULA Prefix
	mib_get (MIB_V6_ULAPREFIX_ENABLE, (void *)&vChar);
	if (vChar!=0) {
		unsigned char validtime[MAX_RADVD_CONF_PREFIX_LEN];
		unsigned char preferedtime[MAX_RADVD_CONF_PREFIX_LEN];

		if ( !mib_get(MIB_V6_ULAPREFIX, (void *)str)       ||
				!mib_get(MIB_V6_ULAPREFIX_LEN, (void *)str2)  ||
				!mib_get(MIB_V6_ULAPREFIX_VALID_TIME, (void *)validtime)  ||
				!mib_get(MIB_V6_ULAPREFIX_PREFER_TIME, (void *)preferedtime)
		   )
		{
			printf("Get ULAPREFIX mib error!");
		}
		else
		{
			unsigned char ip6Addr[IP6_ADDR_LEN];
			unsigned char devAddr[MAC_ADDR_LEN];
			unsigned char meui64[8];
			unsigned char value[64];
			int i;
#ifdef CONFIG_RTK_RG_INIT
			struct ipv6_ifaddr ip6_addr[6];
			char cur_ip6_addr[64];
#endif

			fprintf(fp, "\t\n");
			fprintf(fp, "\tprefix %s/%s\n", str, str2);
			fprintf(fp, "\t{\n");
			fprintf(fp, "\t\tAdvOnLink on;\n");
			fprintf(fp, "\t\tAdvAutonomous on;\n");
			fprintf(fp, "\t\tAdvValidLifetime %s;\n",validtime);
			fprintf(fp, "\t\tAdvPreferredLifetime %s;\n",preferedtime);
			fprintf(fp, "\t};\n");

#ifdef CONFIG_RTK_RG_INIT
			mib_get(MIB_LAN_IPV6_MODE1, (void *)&vChar);
			if(vChar == 0 ) // LAN IPv6 address mode is auto.
			{
#endif
				inet_pton(PF_INET6, str, (void *)ip6Addr);

				//setup LAN ULA v6 IP address according the ULA prefix + EUI64.
				mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
				mac_meui64(devAddr, meui64);
				for (i=0; i<8; i++)
					ip6Addr[i+8] = meui64[i];
				inet_ntop(PF_INET6, &ip6Addr, value, sizeof(value));
				sprintf(value, "%s/%s", value, str2);
				printf("Set LAN ULA %s\n",value);
#ifdef CONFIG_RTK_RG_INIT
			}
			else
			{
				mib_get(MIB_LAN_IPV6_ADDR1, (void *)ip6Addr);
				inet_ntop(PF_INET6, ip6Addr, value, sizeof(value));
				mib_get(MIB_LAN_IPV6_PREFIX_LEN1, (void *)&vChar2);
				sprintf(value, "%s/%d", value, vChar2);
			}

			getifip6(LANIF, IPV6_ADDR_UNICAST, ip6_addr, 6);
			inet_ntop(PF_INET6, &ip6_addr[0].addr, cur_ip6_addr, sizeof(cur_ip6_addr));
			sprintf(cur_ip6_addr, "%s/%d", cur_ip6_addr, ip6_addr[0].prefix_len);

			va_cmd(IFCONFIG, 3, 1, LANIF, "del", cur_ip6_addr);
			va_cmd(IFCONFIG, 3, 1, LANIF, "add", value);
#else
			va_cmd(IFCONFIG, 3, 1, LANIF, "del", value);
			va_cmd(IFCONFIG, 3, 1, LANIF, "add", value);
#endif
		}
	}

	// Prefix
	if ( !mib_get(MIB_IPV6_LAN_PREFIX, (void *)str)) {
		printf("Get Prefix_IP mib error!");
	}
	if ( !mib_get(MIB_IPV6_LAN_PREFIX_LEN, (void *)&vChar)) {
		printf("Get Prefix_Length mib error!");
	}

	if(str[0] && vChar!=0)
	{
		struct  in6_addr ip6Addr;
		unsigned char devAddr[MAC_ADDR_LEN];
		unsigned char meui64[8];
		unsigned char value[64];
		int i;

		fprintf(fp, "\t\n");
		fprintf(fp, "\tprefix %s/%d\n", str, vChar);
		fprintf(fp, "\t{\n");

		inet_pton(AF_INET6,str,&ip6Addr);
		mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
		mac_meui64(devAddr, meui64);
		for (i=0; i<8; i++)
			ip6Addr.s6_addr[i+8] = meui64[i];
		inet_ntop(PF_INET6, &ip6Addr, value, sizeof(value));
		sprintf(value, "%s/%d", value, vChar);
		va_cmd(IFCONFIG, 3, 1, LANIF, "del", value);
		va_cmd(IFCONFIG, 3, 1, LANIF, "add", value);

		// AdvOnLink
		if ( !mib_get( MIB_V6_ONLINK, (void *)&vChar) )
			printf("Get MIB_V6_ONLINK error!");
		if (0 == vChar)
			fprintf(fp, "\t\tAdvOnLink off;\n");
		else if (1 == vChar)
			fprintf(fp, "\t\tAdvOnLink on;\n");

		// AdvAutonomous
		if ( !mib_get( MIB_V6_AUTONOMOUS, (void *)&vChar) )
			printf("Get MIB_V6_AUTONOMOUS error!");
		if (0 == vChar)
			fprintf(fp, "\t\tAdvAutonomous off;\n");
		else if (1 == vChar)
			fprintf(fp, "\t\tAdvAutonomous on;\n");

		// AdvValidLifetime
		if ( !mib_get(MIB_V6_VALIDLIFETIME, (void *)str)) {
			printf("Get AdvValidLifetime mib error!");
		}
		if (str[0]) {
			fprintf(fp, "\t\tAdvValidLifetime %s;\n", str);
		}

		// AdvPreferredLifetime
		if ( !mib_get(MIB_V6_PREFERREDLIFETIME, (void *)str)) {
			printf("Get AdvPreferredLifetime mib error!");
		}
		if (str[0]) {
			fprintf(fp, "\t\tAdvPreferredLifetime %s;\n", str);
		}

		fprintf(fp, "\t};\n");
	}
	//RDNSS
	if ( !mib_get(MIB_IPV6_LAN_IP_ADDR, (void *)str)) {
		printf("Get RDNSS1 mib error!");
	}
	else if (str[0]) {
		fprintf(fp, "\n\tRDNSS %s\n", str);
		fprintf(fp, "\t{\n");
		fprintf(fp, "\t\tAdvRDNSSPreference 8;\n");
		fprintf(fp, "\t\tAdvRDNSSOpen off;\n");
		fprintf(fp, "\t};\n");
	}

	fprintf(fp, "};\n");
	fclose(fp);

	// Resync radvd.conf
	if ( resync == 1 ) {
		radvdpid = read_pid((char *)RADVD_PID);
		if (radvdpid > 0) {
			kill(radvdpid, SIGHUP);
		}
	}

	return;

}
#endif // of CONFIG_USER_RADVD
#endif // of CONFIG_IPV6

#ifdef _CWMP_MIB_
void set_endpoint(char *newurl, char *acsurl) //star: remove "http://" from acs url string
{
	register const char *s;
	register size_t i, n;
	newurl[0] = '\0';

	if (!acsurl || !*acsurl)
	  return;
	s = strchr(acsurl, ':');
	if (s && s[1] == '/' && s[2] == '/')
	  s += 3;
	else
	  s = acsurl;
	n = strlen(s);
	if(n>256)
		n=256;

	for (i = 0; i < n; i++)
	{ newurl[i] = s[i];
	  if (s[i] == '/' || s[i] == ':')
	    break;
	}

	newurl[i] = '\0';
}

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
void setQosfortr069(int mode, char *urlvalue)
{
	char vStr[256+1];
	char acsurl[256+1];
	struct hostent *host;
	struct in_addr acsaddr;
	char classId[]="0x00:0x00";
	char* argv[15];
	char *operationMode[2]={"add","del"};
	char *iptablesopt[2]={"-A","-D"};

	{
		set_endpoint(acsurl,urlvalue);
		host=gethostbyname(acsurl);
		if(host==NULL)
			return;
		memcpy((char *) &(acsaddr.s_addr), host->h_addr_list[0], host->h_length);

		{
			char interfaceName[8]={0};
			char DstIp[20]={0};
			int k;
			//iptables -t mangle -A OUTPUT -d 172.21.146.44 -p tcp -j CLASSIFY --set-class 0x00:0x06
			argv[1]="-t";
			argv[2]="mangle";
			argv[3]=iptablesopt[mode];
			argv[4]="OUTPUT";
			argv[5]="-d";
			strcpy(DstIp,inet_ntoa(acsaddr));
			argv[6]=DstIp;
			argv[7]="-p";
			argv[8]="tcp";
			argv[9]="-j";
			argv[10]="CLASSIFY";
			argv[11]="--set-class";
			for (k=0; k<8; k++) {
				if (priomap[k] == 1) {
					classId[8]+=k;
					break;
				}
			}
			argv[12]=classId;
			argv[13]=NULL;
			printf("\niptables");
			int i;
			for(i=1;i<12;i++){
				printf(" %s",argv[i]);
			}
			printf("\n");
			do_cmd("/bin/iptables",argv,1);

			//telefonica requests to add this rule to chain "INPUT" and "FORWARD", for possible tr069 Device in Lan side
			//Magician: Setting this rule in INPUT is useless, this is just for Telefonica's demand.
			//argv[4]="INPUT";
			//do_cmd("/bin/iptables",argv,1);

			argv[4]="FORWARD";
			do_cmd("/bin/iptables",argv,1);
		}
	}
	return;

}

static int SetcwmpQosflag = 0;

void setTr069QosFlag(int var)
{
	SetcwmpQosflag = var;
}

int getTr069QosFlag(void)
{
	return SetcwmpQosflag;
}
#endif

static const char OLDACSFILE[] = "/var/oldacs";
static const char OLDACSFILE_FOR_WAN[] = "/var/oldacs_wan";  // Magician: Used for TR-069 WAN interface.

void storeOldACS(void)
{
	FILE* fp;
	char acsurl[256+1]={0};

	mib_get(CWMP_ACS_URL, acsurl);
	if(strlen(acsurl))
	{
		if(fp = fopen(OLDACSFILE, "w"))
		{
			fprintf(fp, "%s",acsurl);
			fclose(fp);
		}

		if(fp = fopen(OLDACSFILE_FOR_WAN, "w"))
		{
			fprintf(fp, "%s",acsurl);
			fclose(fp);
		}
	}
}

int getOldACS(char *acsurl)
{
	FILE* fp;

	acsurl[0]=0;
	fp=fopen(OLDACSFILE,"r");
	if(fp){
		fscanf(fp,"%s",acsurl);
		fclose(fp);
		unlink(OLDACSFILE);
	}
	if(strlen(acsurl))
		return 1;
	else
		return 0;
}
/*star:20100305 END*/

static int getOldACSforWAN(char *acsurl)
{
	FILE* fp;

	acsurl[0]=0;
	fp=fopen(OLDACSFILE_FOR_WAN,"r");
	if(fp){
		fscanf(fp,"%s",acsurl);
		fclose(fp);
		unlink(OLDACSFILE_FOR_WAN);
	}
	if(strlen(acsurl))
		return 1;
	else
		return 0;
}
#endif

// Magician: This function is for checking the validation of whole config file.
char checkConfigFile(char *config_file)
{
	FILE *fp;
	char strbuf[512], *pstr;
	int linenum = 1, len;
	char inChain = 0, isEnd = 0, status = 1;

	if(!(fp = fopen(config_file, "r")))
	{
		printf("Open config file failed: %s\n", strerror(errno));
		return 0;
	}

	len = strlen(fgets(strbuf, 512, fp));
	if(strbuf[len-1] != '\n')
	{
		printf("Miss a newline at line %d!\n", linenum);
		fclose(fp);
		return 0;
	}

	if(strbuf[len-2] == '\r' && strbuf[len-1] == '\n') // Remove the CRLF(0x0D0A) which is at the last of a line.
		strbuf[len-2] = 0;
	else if(strbuf[len-1] == '\n') // Remove the LF(0x0A) which is at the last of a line.
		strbuf[len-1] = 0;

	if(strcmp(strbuf, CONFIG_HEADER))
	{
		printf("Invalid header: %s\n", strbuf);
		fclose(fp);
		return 0;
	}

	while(fgets(strbuf, 512, fp))
	{
		linenum++;  // The counter of current line on handling.
		len = strlen(strbuf);
		//printf("%d: %d: %s", linenum, len, strbuf);

		if(strbuf[len-1] != '\n')   // If this line does not end with a newline, return an error.
		{
			printf("Miss a newline at line %d!\n", linenum);
			status = 0;
			break;
		}

		if(strbuf[len-2] == '\r' && strbuf[len-1] == '\n') // Remove the CRLF(0x0D0A) which is at the last of a line.
			strbuf[len-2] = 0;
		else if(strbuf[len-1] == '\n') // Remove the LF(0x0A) which is at the last of a line.
			strbuf[len-1] = 0;

		if(isEnd)  // It should be at the end of this config file.
		{
			printf("Invalid end of config file at line %d\n", linenum);
			status = 0;
			break;
		}

		if(!strncmp(strbuf, "<Value Name=\"", 13))  // Check the validation of common <Value Name.... line.
		{
			if(!(pstr = strchr(strbuf+13, '\"')))
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			pstr++;

			if(strncmp(pstr, " Value=\"", 8))
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			if(!(pstr = strchr(pstr+8, '\"')))
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			pstr++;

			if(strncmp(pstr, "/>", 2))
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			pstr += 2;

			if(*pstr != '\0')
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			continue;
		}

		if(!strncmp(strbuf, "<chain chainName=\"", 18))  // Enter in a chain table.
		{
			if(inChain) // It has been in another chain table currently.
			{
				printf("Invalid structure at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			if(!(pstr = strchr(strbuf+18, '\"')))
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			pstr++;

			if(*pstr != '>')
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			pstr++;

			if(*pstr != '\0')
			{
				printf("Invalid format at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}

			inChain = 1;
			continue;
		}

		if(!strncmp(strbuf, "</chain>", 8))  // Leave from a chain table.
		{
			if(inChain)
			{
				inChain = 0;
				continue;
			}
			else
			{
				printf("Invalid structure at line %d: %s\n", linenum, strbuf);
				status = 0;
				break;
			}
		}

		if(!strcmp(strbuf, CONFIG_TRAILER))  // Reach the end of config file.
		{
			isEnd = 1;
			continue;
		}

		printf("Unknown format at line %d: %s\n", linenum, strbuf);
		status = 0;
		break;
	}

	fclose(fp);
	return status;
}

// Magician: This function is used for memory changing watch. You can put it anywhere to detect if memory usage changes.
// Be sure you assign all these functions in THE SAME process.
#if DEBUG_MEMORY_CHANGE
char last_memsize[128], last_file[32], last_func[32]; // Use to indicate last position where you put ShowMemChange().
int last_line;  // Use to indicate last position where you put ShowMemChange().
int ShowMemChange(char *file, char *func, int line)
{
	FILE *fp, *logfp;
	char buf[128];
	int i;
	char isPntOnChg = 1;  // Print message when memory size changed.
	char isLog = 1;  // Log results in /tmp/memlog
	char status = 0;  // return 1 when memory usage does change or otherwise return 0.

	if(isLog && !(logfp = fopen("/tmp/memlog", "a")))
	{
		perror("/tmp/memlog");
		return -1;
	}

	sprintf(buf, "/proc/%d/status", getpid());

	if(fp = fopen(buf, "r"))
	{
		for( i = 0; i < 11; i++ )
			fgets(buf, 128, fp);

		fclose(fp);

		if(!isPntOnChg || strcmp(buf, last_memsize))
		{
			putchar('\n');

			if(isPntOnChg)
			{
				printf("===== Last Memory size info (%s:%s:%d) =====\n", last_file, last_func, last_line);
				printf(last_memsize);

				if(isLog)
				{
					fprintf(logfp, "===== Last Memory size info (%s:%s:%d) =====\n", last_file, last_func, last_line);
					fprintf(logfp, last_memsize);
				}
			}

			printf("===== Memory size info (%s:%s:%d) =====\n", file, func, line);
			printf(buf);

			if(isLog)
			{
				fprintf(logfp, "===== Memory size info (%s:%s:%d) =====\n", file, func, line);
				fprintf(logfp, "%s\n", buf);
			}

			status = 1;
		}

		strncpy(last_memsize, buf, 128);
		strncpy(last_file, file, 32);
		strncpy(last_func, func, 32);
		last_line = line;
	}

	fclose(logfp);
	return status;
}
#endif
//Kevin:Check whether to enable/disable upstream ip fastpath
void UpdateIpFastpathStatus(void)
{
    /* If any one of the folllowing functions is enabled,
           we have to disable upstream ip fastpath.
               (1) IP Qos
               (2) URL blocking
               (3) Domain blocking
           Otherwise, keep ip fastpath up/downstream both to enhance throughput.
       */

    unsigned char mode=0;
#ifdef IP_QOS
	mib_get(MIB_MPMODE, (void *)&mode);
	if (mode & MP_IPQ_MASK)
	{
		//printf("(%s)IP Qos V1!\n",__func__);
		system("/bin/echo 1 > /proc/FastPath");
		return;
	}
#endif

#if defined(CONFIG_USER_IP_QOS_3) || defined(NEW_IP_QOS_SUPPORT)
	unsigned int qosEnable;
	unsigned int qosRuleNum, carRuleNum;
	unsigned char totalBandWidthEn;

	qosEnable = getQosEnable();
	qosRuleNum = getQosRuleNum();
	mib_get(MIB_TOTAL_BANDWIDTH_LIMIT_EN, (void *)&totalBandWidthEn);
	carRuleNum = mib_chain_total(MIB_IP_QOS_TC_TBL);
	//if ((qosEnable && qosRuleNum) || totalBandWidthEn || carRuleNum) {
	if ((qosEnable && qosRuleNum)) {
		system("/bin/echo 1 > /proc/FastPath");
		system("/bin/echo 0 > /proc/fastbridge");
		return;
	}
	else if (totalBandWidthEn || carRuleNum) {
		system("/bin/echo 1 > /proc/FastPath");
		return;
	}
#endif

	mode=0;
	mib_get(MIB_URL_CAPABILITY, (void *)&mode);
	if (mode)
	{
		//printf("(%s)URL blocking!\n",__func__);
		system("/bin/echo 1 > /proc/FastPath");
		return;
	}

	mode=0;
	mib_get(MIB_DOMAINBLK_CAPABILITY, (void *)&mode);
	if (mode)
	{
		//printf("(%s)Domain blocking!\n",__func__);
		system("/bin/echo 1 > /proc/FastPath");
		return;
	}

	//printf("(%s)none!\n",__func__);
	system("/bin/echo 2 > /proc/FastPath");
}

#ifdef RESERVE_KEY_SETTING
/*
 * flag
 * 0: for short reset
 * 1: for long reset
 * 2: TR-069 FactoryReset
 */
static int reserve_critical_setting(int flag)
{
	//first backup current setting
	mib_backup(CONFIG_MIB_ALL);

	// restore current to default
#ifdef CONFIG_USER_XMLCONFIG
	va_cmd("/bin/sh",2,1, "/etc/scripts/config_xmlconfig.sh", "-d");
#else
	mib_sys_to_default(CURRENT_SETTING);
#endif
	//now retrieve the key parameters.

	mib_retrive_table(MIB_LOID);
	mib_retrive_table(MIB_LOID_PASSWD);
	mib_retrive_table(MIB_LOID_OLD);
	mib_retrive_table(MIB_LOID_PASSWD_OLD);
#if defined(CONFIG_GPON_FEATURE)
	mib_retrive_table(MIB_GPON_PLOAM_PASSWD);
#endif

	if(flag == 2){
#ifdef CONFIG_YUEME
#ifdef CONFIG_USER_L2TPD_L2TPD
		mib_retrive_chain(MIB_L2TP_TBL);
#endif
#ifdef WLAN_SUPPORT
#ifdef WIFI_TIMER_SCHEDULE
		mib_retrive_chain(MIB_WIFI_TIMER_EX_TBL);
		mib_retrive_chain(MIB_WIFI_TIMER_TBL);
#endif
#endif
#ifdef CONFIG_LED_INDICATOR_TIMER
		mib_retrive_chain(MIB_LED_INDICATOR_TIMER_TBL);
#endif
#ifdef CONFIG_RG_SLEEPMODE_TIMER
		mib_retrive_chain(MIB_SLEEP_MODE_SCHED_TBL);
#endif
#endif
		return 0;
	}

#ifdef _CWMP_MIB_
	mib_retrive_table(CWMP_ACS_URL);
	mib_retrive_table(CWMP_ACS_URL_OLD);
	mib_retrive_table(CWMP_ACS_USERNAME);
	mib_retrive_table(CWMP_ACS_PASSWORD);
	mib_retrive_table(CWMP_CONREQ_USERNAME);
	mib_retrive_table(CWMP_CONREQ_PASSWORD);
#endif
	mib_retrive_table(MIB_SUSER_NAME);
	mib_retrive_table(MIB_SUSER_PASSWORD);
	mib_retrive_chain(MIB_ATM_VC_TBL);
	mib_retrive_chain(MIB_IP_ROUTE_TBL);

	if(flag == 1)
	{
		// Remove not TR-069 WAN
		MIB_CE_ATM_VC_T entry;
		int total, rmid = 0, i;

		total = mib_chain_total(MIB_ATM_VC_TBL);

		for(i=0 ; i < total ; i++)
		{
			mib_chain_get(MIB_ATM_VC_TBL, rmid, (void *)&entry);

			if(entry.applicationtype & X_CT_SRV_TR069)
				rmid++;
			else
				mib_chain_delete(MIB_ATM_VC_TBL, rmid);
		}
	}
	else
	{
#ifdef _CWMP_MIB_
		mib_retrive_table(CWMP_FLAG2);
#endif
#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
		mib_retrive_chain(MIB_PORT_BINDING_TBL);
#endif
		mib_retrive_chain(MIB_DHCPS_SERVING_POOL_TBL);

#ifdef WLAN_SUPPORT
		MIB_CE_MBSSIB_T Entry;
		mib_chain_get(MIB_MBSSIB_TBL, 0, (void *)&Entry);
		mib_retrive_chain(MIB_MBSSIB_TBL);
		mib_chain_update(MIB_MBSSIB_TBL,(void *)&Entry, 0); //reset to default for SSID-1
#endif

#ifdef VOIP_SUPPORT
		mib_retrive_chain(MIB_VOIP_CFG_TBL);
#endif
#ifdef _PRMT_X_CT_COM_USERINFO_
		mib_retrive_table(CWMP_USERINFO_STATUS);
		mib_retrive_table(CWMP_USERINFO_LIMIT);
		mib_retrive_table(CWMP_USERINFO_TIMES);
		mib_retrive_table(CWMP_USERINFO_RESULT);
#endif
#ifdef _PRMT_X_CT_COM_PING_
		mib_retrive_chain(CWMP_CT_PING_TBL);
#endif
#ifdef TIME_ZONE
		mib_retrive_table(MIB_NTP_ENABLED);
		mib_retrive_table(MIB_NTP_TIMEZONE_DB_INDEX);
		mib_retrive_table(MIB_DST_ENABLED);
		mib_retrive_table(MIB_NTP_SERVER_HOST1);
		mib_retrive_table(MIB_NTP_SERVER_HOST2);
		mib_retrive_table(MIB_NTP_IF_TYPE);
		mib_retrive_table(MIB_NTP_IF_WAN);
		mib_retrive_table(MIB_NTP_INTERVAL);
#endif
#ifdef VIRTUAL_SERVER_SUPPORT
		mib_retrive_chain(MIB_VIRTUAL_SVR_TBL);
#endif
#ifdef _PRMT_X_CT_COM_MWBAND_
		mib_retrive_table(CWMP_CT_MWBAND_MODE);
		mib_retrive_table(CWMP_CT_MWBAND_STB_ENABLE);
		mib_retrive_table(CWMP_CT_MWBAND_CMR_ENABLE);
		mib_retrive_table(CWMP_CT_MWBAND_PC_ENABLE);
		mib_retrive_table(CWMP_CT_MWBAND_PHN_ENABLE);
		mib_retrive_table(CWMP_CT_MWBAND_NUMBER);
		mib_retrive_table(CWMP_CT_MWBAND_STB_NUM);
		mib_retrive_table(CWMP_CT_MWBAND_CMR_NUM);
		mib_retrive_table(CWMP_CT_MWBAND_PC_NUM);
		mib_retrive_table(CWMP_CT_MWBAND_PHN_NUM);
#endif

#ifdef CONFIG_YUEME
#ifdef CONFIG_USER_L2TPD_L2TPD
		mib_retrive_chain(MIB_L2TP_TBL);
#endif
#endif
	}

	mib_retrive_table(MIB_BRCTL_AGEINGTIME);
	mib_retrive_table(MIB_BRCTL_STP);

	return 0;
}
#endif

static char *get_name(char *name, char *p)
{
	while (isspace(*p))
		p++;
	while (*p) {
		if (isspace(*p))
			break;
		if (*p == ':') {	/* could be an alias */
			char *dot = p, *dotname = name;
			*name++ = *p++;
			while (isdigit(*p))
				*name++ = *p++;
			if (*p != ':') {	/* it wasn't, backup */
				p = dot;
				name = dotname;
			}
			if (*p == '\0')
				return NULL;
			p++;
			break;
		}
		*name++ = *p++;
	}
	*name = '\0';

	return p;
}

static int procnetdev_version(char *buf)
{
	if (strstr(buf, "compressed"))
		return 2;
	if (strstr(buf, "bytes"))
		return 1;
	return 0;
}

static const char *const ss_fmt[] = {
	"%n%lu%lu%lu%lu%lu%n%n%n%lu%lu%lu%lu%lu%lu",
	"%lu%lu%lu%lu%lu%lu%n%n%lu%lu%lu%lu%lu%lu%lu",
	"%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu%lu"
};

static void get_dev_fields(char *bp, struct net_device_stats *nds, int procnetdev_vsn)
{
	memset(nds, 0, sizeof(*nds));

	sscanf(bp, ss_fmt[procnetdev_vsn],
		   &nds->rx_bytes, /* missing for 0 */
		   &nds->rx_packets,
		   &nds->rx_errors,
		   &nds->rx_dropped,
		   &nds->rx_fifo_errors,
		   &nds->rx_frame_errors,
		   &nds->rx_compressed, /* missing for <= 1 */
		   &nds->multicast, /* missing for <= 1 */
		   &nds->tx_bytes, /* missing for 0 */
		   &nds->tx_packets,
		   &nds->tx_errors,
		   &nds->tx_dropped,
		   &nds->tx_fifo_errors,
		   &nds->collisions,
		   &nds->tx_carrier_errors,
		   &nds->tx_compressed /* missing for <= 1 */
		   );

	if (procnetdev_vsn <= 1) {
		if (procnetdev_vsn == 0) {
			nds->rx_bytes = 0;
			nds->tx_bytes = 0;
		}
		nds->multicast = 0;
		nds->rx_compressed = 0;
		nds->tx_compressed = 0;
	}
}

/**
 * list_net_device_with_flags - list network devices with the specified flags
 * @flags: input argument, the network device flags
 * @nr_names: input argument, number of elements in @names
 * @names: output argument, constant pointer to the array of network device names
 *
 * Returns the number of resulted elements in @names for success
 * or negative errno values for failure.
 */
int list_net_device_with_flags(short flags, int nr_names,
				char (* const names)[IFNAMSIZ])
{
	FILE *fh;
	char buf[512];
	struct ifreq ifr;
	int nr_result, skfd;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0)
		goto out;

	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh)
		goto out_close_skfd;
	fgets(buf, sizeof(buf), fh);	/* eat line */
	fgets(buf, sizeof(buf), fh);

	nr_result = 0;
	while (fgets(buf, sizeof(buf), fh) && nr_result < nr_names) {
		char name[128];

		get_name(name, buf);

		strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name));
		if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0)
			goto out_close_fh;

		if (ifr.ifr_flags & flags) {
			strncpy(names[nr_result++], name, ARRAY_SIZE(names[0]));
		}
	}

	if (ferror(fh))
		goto out_close_fh;

	fclose(fh);
	close(skfd);

	return nr_result;

out_close_fh:
	fclose(fh);
out_close_skfd:
	close(skfd);
out:
	warn("%s():%d", __FUNCTION__, __LINE__);

	return -errno;
}

/**
 * get_net_device_stats - get the statistics of the specified network device
 * @ifname: input argument, constant pointer to the network device name
 * @nds: output argument, pointer to network device statistics
 *
 * Returns 0 if not found, 1 if found
 * or negative errno values for failure.
 *
 */
int get_net_device_stats(const char *ifname, struct net_device_stats *nds)
{
	FILE *fh;
	char buf[512];
	int procnetdev_vsn, found;

	fh = fopen(_PATH_PROCNET_DEV, "r");
	if (!fh)
		goto out;
	fgets(buf, sizeof(buf), fh);	/* eat line */
	fgets(buf, sizeof(buf), fh);

	procnetdev_vsn = procnetdev_version(buf);

	memset(nds, 0, sizeof(*nds));
	found = 0;
	while (fgets(buf, sizeof(buf), fh)) {
		char *s, name[128];
		int n;

		s = get_name(name, buf);

		if (!strcmp(name, ifname)) {
			get_dev_fields(s, nds, procnetdev_vsn);
			found = 1;
			break;
		}
	}

	if (ferror(fh))
		goto out_close_fh;

	fclose(fh);

	return found;

out_close_fh:
	fclose(fh);
out:
	warn("%s():%d %s", __FUNCTION__, __LINE__, _PATH_PROCNET_DEV);

	return -errno;
}

/**
 * ethtool_gstats - get the statistics of the specified network device using ethtool
 * @ifname: input argument, constant pointer to the network device name
 *
 * Returns NULL if an error occurs, non-NULL otherwise
 */
struct ethtool_stats * ethtool_gstats(const char *ifname)
{
	struct ifreq ifr;
	int fd, err;
	struct ethtool_drvinfo drvinfo;
	struct ethtool_stats *stats = NULL;
	unsigned int n_stats;

	/* Setup our control structures. */
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, ifname);

	/* Open control socket. */
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("Cannot get control socket");
		goto out;
	}

	drvinfo.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = &drvinfo;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err < 0) {
		perror("Cannot get driver information");
		goto out_close_fd;
	}

	n_stats = drvinfo.n_stats;
	if (n_stats < 1) {
		fprintf(stderr, "no stats available\n");
		goto out_close_fd;
	}

	stats = calloc(1, n_stats * sizeof(uint64_t) + sizeof(struct ethtool_stats));
	if (!stats) {
		fprintf(stderr, "no memory available\n");
		goto out_close_fd;
	}

	stats->cmd = ETHTOOL_GSTATS;
	stats->n_stats = n_stats;
	ifr.ifr_data = stats;
	err = ioctl(fd, SIOCETHTOOL, &ifr);
	if (err < 0) {
		perror("Cannot get stats information");
		free(stats);
		stats = NULL;
		goto out_close_fd;
	}

out_close_fd:
	close(fd);
out:
	return stats;
}

// Kaohj
/*
 *	Get the link status about device.
 *	Return:
 *		-1 on error
 *		0 on link down
 *		1 on link up
 */
int get_net_link_status(const char *ifname)
{
	struct ifreq ifr;
	struct ethtool_value edata;
	int ret;
#ifdef CONFIG_DEV_xDSL
	Modem_LinkSpeed vLs;
#endif

#ifdef CONFIG_DEV_xDSL
	if (!strcmp(ifname, ALIASNAME_DSL0)) {
       		if ( adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs,
			RLCM_GET_LINK_SPEED_SIZE) && vLs.upstreamRate != 0)
			ret = 1;
		else
			ret = 0;
		return ret;
	}
#endif
	strcpy(ifr.ifr_name, ifname);
	edata.cmd = ETHTOOL_GLINK;
	ifr.ifr_data = (caddr_t)&edata;

	ret = do_ioctl(SIOCETHTOOL, &ifr);
	if (ret == 0)
		ret = edata.data;
	return ret;
}

/*
 *	Get the link information about device.
 *	Return:
 *		-1 on error
 *		0 on success
 */
int get_net_link_info(const char *ifname, struct net_link_info *info)
{
	struct ifreq ifr;
	struct ethtool_cmd ecmd;
	int ret;

	memset(info, 0, sizeof(struct net_link_info));
	strcpy(ifr.ifr_name, ifname);
	ecmd.cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t)&ecmd;

	ret = do_ioctl(SIOCETHTOOL, &ifr);
	if (ret == 0) {
		info->supported = ecmd.supported; // ports, link modes, auto-negotiation
		info->advertising = ecmd.advertising; // link modes, pause frame use, auto-negotiation
		info->speed = ecmd.speed; // 10Mb, 100Mb, gigabit
		info->duplex = ecmd.duplex; // Half, Full, Unknown
		info->phy_address = ecmd.phy_address;
		info->transceiver = ecmd.transceiver;
		info->autoneg = ecmd.autoneg;
	}
	return ret;
}

int GetWanMode(void)
{
	unsigned int wanmode = 0;

	if(!mib_get(MIB_WAN_MODE, (void *)&wanmode))
		fprintf(stderr, "Get mib value MIB_WAN_MODE failed!\n");

	return wanmode & WAN_MODE_MASK;
}

int isInterfaceMatch(unsigned int ifindex)
{
	if(WAN_MODE & MODE_ATM && MEDIA_ATM == MEDIA_INDEX(ifindex))
		return 1;
	else if(WAN_MODE & MODE_Ethernet && MEDIA_ETH == MEDIA_INDEX(ifindex))
		return 1;
	else if(WAN_MODE & MODE_PTM && MEDIA_PTM == MEDIA_INDEX(ifindex))
		return 1;

	return 0;
}

/*
 * flag
 * 0: for short reset
 * 1: for long reset
 * 2: TR-069 FactoryReset
 */
int reset_cs_to_default(int flag)
{
#ifdef RESERVE_KEY_SETTING
	reserve_critical_setting(flag);
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
#else
	return mib_flash_to_default(CURRENT_SETTING);
#endif
	return 1;
}

#ifdef CONFIG_TR_064
int GetTR064Status(void)
{
	unsigned char tr064_st = 0;

	if(!mib_get(MIB_TR064_ENABLED, (void *)&tr064_st))
		fprintf(stderr, "Get mib value MIB_TR064_ENABLED failed!\n");

	return tr064_st;
}
#endif

#ifdef CONFIG_USER_CWMP_TR069
int SetTR069WANInterface(void) // Magician: Bind WAN interface to TR-069
{
	unsigned int cwmp_ifidx, total, i, pre_cwmp_ifidx, v_uint;
	unsigned char cwmp_flag;
	MIB_CE_ATM_VC_T entry;
	char dst_ip[20], intf_name[10], vStr[256], acsurl[256], *argv[7], dport[10];
	struct hostent *host;
	struct in_addr acsaddr;
	FILE *fp;

	va_cmd(IPTABLES, 2, 1, "-F", (char *)IPTABLE_TR069);

	if(mib_get(CWMP_FLAG, (void *)&cwmp_flag))
	{
		if(!(cwmp_flag & CWMP_FLAG_AUTORUN))  // Return if TR-069 is disabled.
			return 0;
	}

	if(!mib_get(CWMP_WAN_INTERFACE, (void *)&cwmp_ifidx))
	{
		fprintf(stderr, "Get mib value CWMP_WAN_INTERFACE failed!\n");
		return -1;
	}

	if(!mib_get(CWMP_ACS_URL, (void*)vStr))
	{
		fprintf(stderr, "Get mib value CWMP_ACS_URL failed!\n");
		return -1;
	}
	set_endpoint(acsurl, vStr);

	if(!(host = gethostbyname(acsurl)))
	{
		fprintf(stderr, "ACS URL gethostbyname failed!\n");
		return -1;
	}

	memcpy((char *) &(acsaddr.s_addr), host->h_addr_list[0], host->h_length);
	strcpy(dst_ip, inet_ntoa(acsaddr));

	total = mib_chain_total(MIB_ATM_VC_TBL);

	argv[1] = "del";
	argv[2] = "-host";
	argv[3] = dst_ip;
	argv[4] = "dev";
	argv[5] = intf_name;
	argv[6] = NULL;

	if(fp = fopen(PRE_CWMP_WAN_INTF, "r"))
	{
		fscanf(fp, "%u", &pre_cwmp_ifidx);
		fclose(fp);
		unlink(PRE_CWMP_WAN_INTF);

		if(pre_cwmp_ifidx != DUMMY_IFINDEX)
		{
			ifGetName(pre_cwmp_ifidx, intf_name, sizeof(intf_name));
			do_cmd("/bin/route", argv, 1);  // Delete pre-configured interface route rule.
		}
	}

	if( cwmp_ifidx != DUMMY_IFINDEX )
	{
		if(!mib_get(CWMP_CONREQ_PORT, (void *)&v_uint))
		{
			fprintf(stderr, "Get CWMP_CONREQ_PORT failed!\n");
			return -1;
		}

		sprintf(dport, "%u", v_uint);
		ifGetName(cwmp_ifidx, intf_name, sizeof(intf_name));
		// iptables -A tr069 ! -i vc0 -s 192.168.2.44 -p tcp --dport 7547 -j DROP
		va_cmd(IPTABLES, 13, 1, "-A", IPTABLE_TR069, "!", (char *)ARG_I, intf_name, "-s", dst_ip, "-p", "tcp", "--dport", dport, "-j", (char *)FW_DROP);

		for( i = 0; i < total; i++ )
		{
			if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry) != 1)
				continue;

			ifGetName(entry.ifIndex, intf_name, sizeof(intf_name));

			if(entry.ifIndex == cwmp_ifidx)
			{
				if(entry.cmode == CHANNEL_MODE_PPPOA || entry.cmode == CHANNEL_MODE_PPPOE || entry.cmode == CHANNEL_MODE_RT1483)
				{
					argv[1] = "add";
					do_cmd("/bin/route", argv, 1);  // Add route for ppp.
				}
				else if(entry.ipDhcp == 1)
				{
					char gwip[20];
					char buffer[50];
					sprintf(buffer, "%s.%s", DHCPC_ROUTERFILE, intf_name);

					if(!(fp = fopen(buffer, "r"))) return -1;

					fscanf(fp, "%s", gwip);
					fclose(fp);
					va_cmd("/bin/route", 7, 1, "add", "-host", dst_ip, "gw", gwip, "dev", intf_name);
				}
				else if(entry.cmode == CHANNEL_MODE_IPOE && entry.ipDhcp == 0 )
				{
					char gwip[20];
					/* Kaohj -- If ACS server is direct-connected with TR069-enabled interface(pEntry), */
					/*	we don't need to add a host route. */

					if(isDirectConnect(&acsaddr, &entry))
					{
						printf("ACS server is direct Connected.\n");
						return 0;
					}

					if(entry.remoteIpAddr[0] != 0)
					{
						strcpy(gwip, inet_ntoa(*((struct in_addr *)entry.remoteIpAddr)));
						va_cmd("/bin/route", 7, 1, "add", "-host", dst_ip, "gw", gwip, "dev", intf_name);
					}
				}
				break;
			}
		}
	}

	return 1;
}

int SetTR069WANInterfacePPP(void) // Magician: Bind WAN interface to TR-069. Special user when PPP is up.
{
	unsigned int cwmp_ifidx;
	unsigned char cwmp_flag;
	char dst_ip[20], intf_name[10], vStr[256], acsurl[256], *argv[7];
	struct hostent *host;
	struct in_addr acsaddr;

	if(mib_get(CWMP_FLAG, (void *)&cwmp_flag))
	{
		if(!(cwmp_flag & CWMP_FLAG_AUTORUN))  // Return if TR-069 is disabled.
			return 0;
	}

	if(!mib_get(CWMP_WAN_INTERFACE, (void *)&cwmp_ifidx))
	{
		fprintf(stderr, "Get mib value CWMP_WAN_INTERFACE failed!\n");
		return -1;
	}

	if( cwmp_ifidx != DUMMY_IFINDEX )
	{
		if(!mib_get(CWMP_ACS_URL, (void*)vStr))
		{
			fprintf(stderr, "Get mib value CWMP_ACS_URL failed!\n");
			return -1;
		}
		set_endpoint(acsurl, vStr);

		if(!(host = gethostbyname(acsurl)))
		{
			fprintf(stderr, "ACS URL gethostbyname failed!\n");
			return -1;
		}

		memcpy((char *) &(acsaddr.s_addr), host->h_addr_list[0], host->h_length);
		strcpy(dst_ip, inet_ntoa(acsaddr));

		argv[1] = "add";
		argv[2] = "-host";
		argv[3] = dst_ip;
		argv[4] = "dev";
		argv[5] = intf_name;
		argv[6] = NULL;

		ifGetName(cwmp_ifidx, intf_name, sizeof(intf_name));

		if((strstr(intf_name, ALIASNAME_PPP) != 0))
			do_cmd("/bin/route", argv, 1);  // Add route for ppp.
	}

	return 1;
}

int SetTR069WANInterfaceDHCP(void) // Magician: Bind WAN interface to TR-069. Special use when DHCP IP had got.
{
	unsigned int cwmp_ifidx, total, i;
	unsigned char cwmp_flag;
	MIB_CE_ATM_VC_T entry;
	char dst_ip[20], intf_name[10], vStr[256], acsurl[256], *argv[7];
	struct hostent *host;
	struct in_addr acsaddr;
	FILE *fp;

	if(mib_get(CWMP_FLAG, (void *)&cwmp_flag))
	{
		if(!(cwmp_flag & CWMP_FLAG_AUTORUN))  // Return if TR-069 is disabled.
			return 0;
	}

	if(!mib_get(CWMP_WAN_INTERFACE, (void *)&cwmp_ifidx))
	{
		fprintf(stderr, "Get mib value CWMP_WAN_INTERFACE failed!\n");
		return -1;
	}

	if( cwmp_ifidx != DUMMY_IFINDEX )
	{
		if(!mib_get(CWMP_ACS_URL, (void*)vStr))
		{
			fprintf(stderr, "Get mib value CWMP_ACS_URL failed!\n");
			return -1;
		}
		set_endpoint(acsurl, vStr);

		if(!(host = gethostbyname(acsurl)))
		{
			fprintf(stderr, "ACS URL gethostbyname failed!\n");
			return -1;
		}

		memcpy((char *) &(acsaddr.s_addr), host->h_addr_list[0], host->h_length);
		strcpy(dst_ip, inet_ntoa(acsaddr));

		total = mib_chain_total(MIB_ATM_VC_TBL);

		argv[1] = "del";
		argv[2] = "-host";
		argv[3] = dst_ip;
		argv[4] = "dev";
		argv[5] = intf_name;
		argv[6] = NULL;

		for( i = 0; i < total; i++ )
		{
			if(mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry) != 1)
				continue;

			ifGetName(entry.ifIndex, intf_name, sizeof(intf_name));

			if(entry.ifIndex == cwmp_ifidx)
			{
				if(entry.ipDhcp == 1)
				{
					char gwip[20];
					char buffer[50];
					sprintf(buffer, "%s.%s", DHCPC_ROUTERFILE, intf_name);

					if(!(fp = fopen(buffer, "r"))) return -1;

					fscanf(fp, "%s", gwip);
					fclose(fp);
					va_cmd("/bin/route", 7, 1, "add", "-host", dst_ip, "gw", gwip, "dev", intf_name);
				}
				break;
			}
		}
	}

	return 1;
}

int DelTR069WANInterface(char acsurl_type) // Magician: Delete WAN interface routing.
{
	unsigned int cwmp_ifidx;
	char dst_ip[20], intf_name[10], vStr[256], acsurl[256], *argv[7];
	struct hostent *host;
	struct in_addr acsaddr;

	if(!mib_get(CWMP_WAN_INTERFACE, (void *)&cwmp_ifidx))
	{
		fprintf(stderr, "Get mib value CWMP_WAN_INTERFACE failed!\n");
		return -1;
	}

	if( cwmp_ifidx != DUMMY_IFINDEX )
	{
		if(acsurl_type == Cur_ACS_URL )
		{
			if(!mib_get(CWMP_ACS_URL, (void*)vStr))
			{
				fprintf(stderr, "Get mib value CWMP_ACS_URL failed!\n");
				return -1;
			}
		}
		else if(acsurl_type == Old_ACS_URL )
			getOldACSforWAN(vStr);
		else
		{
			fprintf(stderr, "Wrong type of ACS URL!\n");
			return -1;
		}

		set_endpoint(acsurl, vStr);

		if(!(host = gethostbyname(acsurl)))
		{
			fprintf(stderr, "ACS URL gethostbyname failed!\n");
			return -1;
		}

		memcpy((char *) &(acsaddr.s_addr), host->h_addr_list[0], host->h_length);
		strcpy(dst_ip, inet_ntoa(acsaddr));

		argv[1] = "del";
		argv[2] = "-host";
		argv[3] = dst_ip;
		argv[4] = "dev";
		argv[5] = intf_name;
		argv[6] = NULL;

		ifGetName(cwmp_ifidx, intf_name, sizeof(intf_name));
		do_cmd("/bin/route", argv, 1);  // Del route.
	}

	return 1;
}
#endif  // #ifdef CONFIG_USER_CWMP_TR069

// Mason Yu. Support ddns status file.
#ifdef CONFIG_USER_DDNS
int stop_all_ddns()
{
	unsigned int entryNum, i;
	MIB_CE_DDNS_T Entry;
	char pidName[128];
	int pid = 0;

	entryNum = mib_chain_total(MIB_DDNS_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_DDNS_TBL, i, (void *)&Entry))
		{
			printf("stop_all_ddns:Get chain record error!\n");
			continue;
		}

		snprintf(pidName, 128, "%s.%s.%s.%s", (char *)DDNSC_PID, Entry.provider, Entry.interface, Entry.hostname);
		pid = read_pid((char *)pidName);
		if (pid > 0) {
			kill(pid, SIGKILL);
			unlink(pidName);
		}
	}
}

int restart_ddns(void)
{
	// Mason Yu.  create DDNS thread dynamically
	//printf("restart_ddns\n");
	// Mason Yu. Specify IP Address
#ifdef CONFIG_IPV6
	va_cmd("/bin/updateddctrl", 2, 1, "all", "3");
#else
	va_cmd("/bin/updateddctrl", 2, 1, "all", "1");
#endif

	return 0;
}

void remove_ddns_status(void)
{
	unsigned int entryNum, i;
	MIB_CE_DDNS_T Entry;
	FILE *fp;
	unsigned char filename[100];

	entryNum = mib_chain_total(MIB_DDNS_TBL);
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_DDNS_TBL, i, (void *)&Entry))
		{
  			printf("remove_ddns_status:Get chain record error!\n");
			continue;
		}

		// Check all variables that updatedd need
		if ( strlen(Entry.username) == 0 ) {
			printf("remove_ddns_status: username/email is NULL!!\n");
			continue;
		}

		if ( strlen(Entry.password) == 0 ) {
			printf("remove_ddns_status: password/key is NULL!!\n");
			continue;
		}

		if ( strlen(Entry.hostname) == 0 ) {
			printf("remove_ddns_status: Hostname is NULL!!\n");
			continue;
		}

		if ( strlen(Entry.interface) == 0 ) {
			printf("remove_ddns_status: Interface is NULL!!\n");
			continue;
		}

		if ( Entry.Enabled != 1 ) {
			printf("remove_ddns_status: The account is disabled!!\n");
			continue;
		}

		if ( strcmp(Entry.provider, "dyndns") == 0 || strcmp(Entry.provider, "tzo") == 0) {
			// open a status file
			sprintf(filename, "/var/%s.%s.%s.txt", Entry.provider, Entry.username, Entry.password);
			if((fp=fopen(filename,"r")) ==NULL)
				continue;
			fclose(fp);
			unlink(filename);
		}else {
			//sprintf(account, "%s:%s", Entry.email, Entry.key);
			printf("remove_ddns_status: Not support this provider\n");
			syslog(LOG_INFO, "remove_ddns_status: Not support this provider %s\n", Entry.provider);
			continue;
		}
	}
}
#endif

// ysleu: To show diagshell RG DHCP request information.
#if defined(CONFIG_LUNA) && defined(CONFIG_RTK_RG_INIT)
void write_to_dhcpc_info(unsigned long requested_ip,unsigned long subnet_mask,unsigned long gw_addr)
{
	int fh;
	int mark;
	char buff[64];

	fh = open("/var/udhcpc/udhcpc.info", O_RDWR|O_CREAT|O_TRUNC, S_IXUSR);

	if (fh == -1) {
		printf("Create udhcpc script file %s error!\n");
		return;
	}

 	WRITE_DHCPC_FILE(fh, "\n##########################################\n");
	WRITE_DHCPC_FILE(fh, "DHCP Request Information:\n");
	sprintf(buff, "[ip_addr]:%d.%d.%d.%d\n",(requested_ip>>24)&0xff,(requested_ip>>16)&0xff,(requested_ip>>8)&0xff,(requested_ip>>0)&0xff);
	WRITE_DHCPC_FILE(fh, buff);
	sprintf(buff, "[ip_network_mask]:%d.%d.%d.%d\n",(subnet_mask>>24)&0xff,(subnet_mask>>16)&0xff,(subnet_mask>>8)&0xff,(subnet_mask>>0)&0xff);
	WRITE_DHCPC_FILE(fh, buff);
	sprintf(buff, "[gateway_ipv4_addr]:%d.%d.%d.%d\n",(gw_addr>>24)&0xff,(gw_addr>>16)&0xff,(gw_addr>>8)&0xff,(gw_addr>>0)&0xff);
	WRITE_DHCPC_FILE(fh, buff);
 	WRITE_DHCPC_FILE(fh, "##########################################\n");

	close(fh);
}
#endif

#if defined(CTC_WAN_NAME) || defined(CONFIG_USER_RTK_WAN_CTYPE)
enum RouteOP{ADD_Route, DEL_Route};
void setapplicationtype_tr069_uti(MIB_CE_ATM_VC_T *pEntry, int mode, char *urlvalue)
{
	char vStr[256+1];
	char acsurl[256+1];
	struct hostent *host;
	struct in_addr acsaddr;
	char* argv[7];
	char *operationMode[2]={"add","del"};
	char *iptablesopt[2]={"-A","-D"};
	FILE *fp;
	const char ACSIP_PATH[] = "/var/acsip";

	if(pEntry->cmode!=0)
	{
		if(urlvalue==NULL)
		{
			mib_get(CWMP_ACS_URL,(void*)vStr);
			set_endpoint(acsurl,vStr);
		}
		else
			set_endpoint(acsurl,urlvalue);

		// Read ACS IP by gethostbyname()
		if(mode==ADD_Route)
		{
			if(!(host = gethostbyname(acsurl)))
			{
				unlink(ACSIP_PATH);
				printf("Get ACS IP failed\n");
				return;
			}
			else
			{
				memcpy((char *) &(acsaddr.s_addr), host->h_addr_list[0], host->h_length);

				if(!(fp = fopen(ACSIP_PATH, "w")))
				{
					//printf("Open ACS IP file failed: %s\n", strerror(errno));
					return;
				}

				fwrite(host->h_addr_list[0], host->h_length, 1, fp);
				fclose(fp);
			}
		}

		if(mode==DEL_Route)
		{
			if(!(fp = fopen(ACSIP_PATH, "r")))
			{
				//printf("Open ACS IP file failed: %s\n", strerror(errno));
				return;
			}
			fread(&acsaddr.s_addr, sizeof(acsaddr.s_addr), 1, fp);
			fclose(fp);
			unlink(ACSIP_PATH);
		}

		if (pEntry->applicationtype&X_CT_SRV_TR069)
		{
			char interfaceName[8]={0};
			char DstIp[20]={0};

			//route del -host 192.168.2.241 dev vc0
			argv[1]=operationMode[mode];
			argv[2]="-host";
			strcpy(DstIp, inet_ntoa(acsaddr));
			argv[3]=DstIp;
			argv[4]="dev";
			ifGetName(pEntry->ifIndex, interfaceName, sizeof(interfaceName));
			argv[5]=interfaceName;
			argv[6]=NULL;

			//if(mode==ADD_Route)
			{
				// Kaohj -- If ACS server is direct-connected with
				//	TR069-enabled interface(pEntry),
				//	we don't need to add a host route.
				if((strstr(interfaceName,ALIASNAME_PPP)!=0) || (pEntry->ipDhcp==1) ) {
					if(mode==ADD_Route) {
						if (isDirectConnect(&acsaddr, NULL)) {
							printf("ACS server is direct Connected(1).\n");
							return;
						}
					}
				}

				if((strstr(interfaceName,ALIASNAME_PPP)!=0))
//				if((strstr(interfaceName,"ppp")!=0))
				{
					//DBPRINT("/bin/route %s %s %s %s %s\n",argv[1],argv[2],argv[3],argv[4],argv[5]);
					do_cmd("/bin/route",argv,1);
					return;
				}

				if(pEntry->ipDhcp==1)
				{
					char gwip[20];
					FILE *fp;
					char buffer[50];
					sprintf(buffer,"%s.%s", DHCPC_ROUTERFILE,interfaceName);
					fp=fopen(buffer,"r");

					if(!fp)
						return;

					fscanf(fp,"%s",gwip);
					fclose(fp);

					va_cmd("/bin/route",7,1,operationMode[mode],"-host",DstIp,"gw",gwip,"dev",interfaceName);
					//DBPRINT("/bin/route %s %s %s %s %s %s %s","add","-host",DstIp,"gw",gwip,"dev",interfaceName);
					return;
				}

				if(pEntry->cmode==CHANNEL_MODE_IPOE && pEntry->ipDhcp==0)
				{
					char gwip[20];
					// Kaohj -- If ACS server is direct-connected with
					//	TR069-enabled interface(pEntry),
					//	we don't need to add a host route.#if 0
					if(mode==ADD_Route) {
						if (isDirectConnect(&acsaddr, pEntry)) {
							printf("ACS server is direct Connected(2).\n");
							return;
						}
					}

					if(pEntry->remoteIpAddr[0]!=0)
					{
						strcpy(gwip, inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr)));
						va_cmd("/bin/route",7,1,operationMode[mode],"-host",DstIp,"gw",gwip,"dev",interfaceName);
						//DBPRINT("/bin/route %s %s %s %s %s %s %s","add","-host",DstIp,"gw",gwip,"dev",interfaceName);
						//printf("/bin/route %s %s %s %s %s %s %s","add","-host",DstIp,"gw",gwip,"dev",interfaceName);
					}
					return;
				}
				return;
			}
			//DBPRINT("/bin/route %s %s %s %s %s\n",argv[1],argv[2],argv[3],argv[4],argv[5]);
			//do_cmd("/bin/route",argv,1);
		}
		else
		{
			char DstIp[20]={0};
			char interfaceName[8]={0};

			strcpy(DstIp,inet_ntoa(acsaddr));

			ifGetName(pEntry->ifIndex, interfaceName, sizeof(interfaceName));


			//iptables -A OUTPUT -o vc0 -d acsip -j DROP
			printf("/bin/iptables %s %s %s %s %s %s %s %s\n",iptablesopt[mode], "OUTPUT", (char *)ARG_O, interfaceName,
				"-d", DstIp, "-j", (char *)FW_DROP);
			va_cmd(IPTABLES, 8, 1, iptablesopt[mode], "OUTPUT", (char *)ARG_O, interfaceName,	"-d", DstIp, "-j", (char *)FW_DROP);
		}
	}
	return;
}
#endif

/**************************************************************************
 * NAME:    usbRestore
 * DESC:    check if USB is mounted and config file exist. If yes, then go ahead and use it
 *          for restore configuration.
 *ARGS:     NONE
 *RETURN:   NONE
 **************************************************************************/
#ifdef _PRMT_USBRESTORE
int usbRestore(void)
{
	char buffer[16], dstname[64], usb_cfg_filename[32];
	int rv = 0, i, n;
	struct dirent **namelist;
	struct file_pipe pipe;
	unsigned char cpbuf[256];
	unsigned char vChar;
#ifdef CONFIG_USE_RAW_DATA
	struct stat st;
	char *buf;
	FILE *fp;
	size_t nRead;
#endif

	mib_get(MIB_USBRESTORE, &vChar);
	if (vChar == 0)
		return 0;

	fprintf(stderr, "ENTER %s\n", __FUNCTION__);
	fprintf(stderr, "check usbRestore...\n");

	// wait for usbmount successfully
	for (i = 0; i < 5; i++) {
		sleep(1);
		n = scandir("/mnt", &namelist, usb_filter, alphasort);

		/* no match */
		if (n <= 0) {
			if (n == 0)
				free(namelist);
			continue;
		} else
			break;
	}

	if (n <= 0) {
		return 0;
	}

	// make usb config filename (dst file)
	snprintf(usb_cfg_filename, sizeof(usb_cfg_filename), "ctce8_%s.cfg", DEVICE_MODEL_STR);

	// wait for usbmount successfully
	sleep(3);
	pipe.buffer = cpbuf;
	pipe.bufsize = sizeof(cpbuf);
	pipe.func = decode;
	for (i = 0; i < n; i++) {
		snprintf(dstname, sizeof(dstname), "/mnt/%s/%s/%s",
			 namelist[i]->d_name, BACKUP_DIRNAME, usb_cfg_filename);
		/* if not exist */
		if (access(dstname, F_OK))
			continue;

		fprintf(stderr, "USB: found %s\n", dstname);

#if defined(CONFIG_USER_XMLCONFIG) || defined(CONFIG_USE_XML)
		rv = file_copy_pipe(dstname, CONFIG_XMLFILE, &pipe);
#else
		rv = file_copy_pipe(dstname, CONFIG_BIN, &pipe);
#endif
		if (rv != 0) {
			fprintf(stderr, "copy failed (%d)\n", rv);
			rv = 0;
			goto out;
		}
		// do restore here...
		fprintf(stderr, "USB: start restore...\n");
#ifdef CONFIG_USER_XMLCONFIG
		rv = call_cmd("/bin/xmlconfig", 3, 1, "-if", CONFIG_XMLFILE, "-nodef");
		if (rv) {
			fprintf(stderr, "failed! (%d)\n", rv);
			goto out;
		}
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
#elif CONFIG_USE_XML
		rv = call_cmd("/bin/loadconfig", 0, 1);
		if (rv) {
			fprintf(stderr, "failed! (%d)\n", rv);
			goto out;
		}
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
#else
		stat(CONFIG_BIN, &st);
		buf = malloc(st.st_size);
		if (!buf)
			goto out;

		fp = fopen(CONFIG_BIN, "r");
		if (!fp)
			goto out;

		nRead = fread(buf, 1, st.st_size, fp);
		fclose(fp);
		if (nRead == st.st_size) {
			printf("write to %d bytes from %08x\n", nRead, buf);
		} else {
			free(buf);
			goto out;
		}
		if (mib_update_from_raw(buf, nRead) != 1) {
			fprintf(stderr, "Flash error!\n");
			free(buf);
			goto out;
		}
		free(buf);
#endif
		fprintf(stderr, "\n\n\nUSB: restore done\n");
		sync();
		break;
	}

out:
	for (i = 0; i < n; i++) {
		free(namelist[i]);
	}
	free(namelist);

	return rv;
}
#endif

static unsigned long sn_matrix[8][8] = {
{0x111C3D60, 0x0B5D60DC, 0x05C94DB0, 0x08A9F558, 0x154848D0, 0x001504A2, 0x0D0C652A, 0x141BE95C},
{0x034EAC58, 0x3B079DAA, 0x09FFFBFA, 0x003F9E94, 0x11618D09, 0x0D5E558A, 0x2577BE5C, 0x003E7AF8},
{0x008AE4C9, 0x0855C770, 0x34143744, 0x1AF6F0A8, 0x05686380, 0x11BCF974, 0x1F96A973, 0x0D81EFE7},
{0x2DB9F622, 0x26218B68, 0x16F02844, 0x09B62188, 0x2524312E, 0x0AF9AB28, 0x2360D510, 0x0018C501},
{0x024CCE7F, 0x0568C990, 0x03AEA55A, 0x0B6AD558, 0x0FDA28A2, 0x12F9E4FC, 0x24252765, 0x111D6B0F},
{0x15D4E175, 0x10AA5B93, 0x13C6CDA0, 0x1473DF94, 0x1AAEE3E0, 0x258EDC1B, 0x19F410CF, 0x000654B8},
{0x04D697F6, 0x141C44B1, 0x0E32F287, 0x0622CA9C, 0x1186804E, 0x0031FD64, 0x101ED862, 0x20197942},
{0x0030AC6E, 0x2A8F8EBD, 0x01918510, 0x02BDF1FC, 0x0C551F68, 0x0C330868, 0x0C0434B6, 0x08CD55A6}};

void encode(unsigned char *buf, size_t * buflen)
{
	int pd = 0;
	unsigned char *p = buf;

	if (p == NULL || buflen == 0)
		return;
	while (pd < *buflen) {
		*p ^= ((unsigned char *)sn_matrix)[pd % 256];
		p++;
		pd++;
	}
}

void decode(unsigned char *buf, size_t *buflen)
{
	encode(buf, buflen);
}

int file_copy_pipe(const char *inputfile, const char *outputfile, struct file_pipe *pipe)
{
	FILE *input, *output;
	size_t b_read, b_write;
	int quit = 0, rv = 0;

	input = fopen(inputfile, "rb");
	if (input == NULL)
		return errno;

	output = fopen(outputfile, "wb");
	if (output == NULL)
		return errno;

	do {
		b_read = fread(pipe->buffer, 1, pipe->bufsize, input);
		if (b_read < pipe->bufsize) {
			quit = 1;
		}
		if (pipe->func)
			pipe->func(pipe->buffer, &b_read);

		b_write = fwrite(pipe->buffer, 1, b_read, output);
		if (b_read != b_write) {
			rv = ferror(output);
			quit = 1;
		}
	} while (!quit);

	fflush(output);
	fclose(input);
	fclose(output);

	return rv;
}

int usb_filter(const struct dirent *dirent)
{
	return !strncmp(dirent->d_name, "usb", 3);
}

int isUSBMounted(void)
{
	struct dirent **namelist;
	int i, n;

	n = scandir("/mnt", &namelist, usb_filter, alphasort);

	/* no match */
	if (n < 0)
		return -1;

	for (i = 0; i < n; i++) {
		free(namelist[i]);
	}
	free(namelist);

	return n;
}

//20080825 add by ramen to fix the special char in the html
char * fixSpecialChar(char *str,char *srcstr,int length)
{
   int i=0,j=0;
   if(!str||!srcstr)  return NULL;
   int strlength=strlen(srcstr);
   if(length<strlength) return NULL;
   char *tempstr=(char*)malloc(strlength*2);
   memset(tempstr,0,2*strlength);
    for(i=0,j=0;i<=strlength;i++)
   	{
   	  if(srcstr[i]=='"'||srcstr[i]=='\\')
   	  	{
   	  	   	  	tempstr[j++]='\\';
		}
	  	tempstr[j++]=srcstr[i];
   	}
   tempstr[j++]='\0';
   if(j>length) goto ERR;
    memcpy(str,tempstr,j);
   ERR:
    free(tempstr);
    return str;
}

// Mason Yu. 2630-e8b
int getProcessPid(const char *pidfile)
{
    	FILE *f;
	int pid=0;
	//fprintf(stderr,"pidfile=%s\n",pidfile);
	if((f = fopen(pidfile, "r")) == NULL)
		return 0;
	fscanf(f, "%d\n", &pid);
	fclose(f);
	//fprintf(stderr,"pid=%d\n",pid);
	return pid;
}

#ifdef TIME_ZONE

unsigned int check_ntp_if()
{
	unsigned char if_type = 0;
	unsigned char wan_type = 0;
	unsigned int if_wan, ifIndex = DUMMY_IFINDEX;
	MIB_CE_ATM_VC_T entry = {0};
	int total, i;

	mib_get(MIB_NTP_IF_WAN, &if_wan);
	mib_get(MIB_NTP_IF_TYPE, &if_type);
	switch(if_type)
	{
	case 0: //INTERNET
		wan_type = X_CT_SRV_INTERNET;
		break;
	case 1: //VOICE
		wan_type = X_CT_SRV_VOICE;
		break;
	case 2: //TR069
		wan_type = X_CT_SRV_TR069;
		break;
	case 3:
		wan_type = X_CT_SRV_OTHER;
		break;
	}

	total = mib_chain_total(MIB_ATM_VC_TBL);

	for(i = 0 ; i < total ; i++)
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, i, &entry) == 0)
			continue;

		if(entry.applicationtype & wan_type)
		{
			// select first match
			if(ifIndex == DUMMY_IFINDEX)
				ifIndex = entry.ifIndex;

			if(entry.ifIndex == if_wan)
			{
				ifIndex = if_wan;
				break;	//specified is OK
			}
		}
	}

	//update NTP WAN
	if(ifIndex != if_wan)
		mib_set(MIB_NTP_IF_WAN, &ifIndex);

	return ifIndex;
}

// Mason Yu. 2630-e8b
int setupNtp(int type)
{
	unsigned char ntpEnabled;
	unsigned char ntpServerHost1[MAX_NAME_LEN];
	unsigned char ntpServerHost2[MAX_NAME_LEN];
	unsigned int if_wan;
	unsigned int interval;
	int pid, num, i, status = 0, sys_pid = -1;
	char ifname[IFNAMSIZ];
	MIB_CE_ATM_VC_T vc_entry;
	char argv_fmt[128];
	FILE *fp;
	unsigned int index = 0;
	unsigned char dst_enabled = 1;

	DOCMDINIT;
	switch (type) {
	case SNTP_DISABLED:
RETRY:
		pid = getProcessPid(SNTPC_PID);
		if (pid) {
			status = DOCMDARGVS("/bin/kill", DOWAIT, "%d", pid);
			goto RETRY;
		}
		break;
	case SNTP_ENABLED:
		pid = getProcessPid(SNTPC_PID);
		mib_get(MIB_NTP_ENABLED, &ntpEnabled);
		if (pid == 0 && ntpEnabled) {
			if (mib_get(MIB_NTP_TIMEZONE_DB_INDEX, &index)) {
				if ((fp = fopen("/etc/TZ", "w")) != NULL) {
					status |= !mib_get(MIB_DST_ENABLED, &dst_enabled);
					fprintf(fp, "%s\n", get_tz_string(index, dst_enabled));
					fclose(fp);
				} else {
					status |= 1;
				}
			} else {
				status |= 1;
			}

			status |= !mib_get(MIB_NTP_SERVER_HOST1, ntpServerHost1);
			status |= !mib_get(MIB_NTP_SERVER_HOST2, ntpServerHost2);

			status |= !mib_get(MIB_NTP_INTERVAL, &interval);
			if (interval == 0)
				interval = 86400;

			if (ntpServerHost2[0])
				sprintf(argv_fmt, "-s %s -s %s -i %d", ntpServerHost1, ntpServerHost2, interval);
			else
				sprintf(argv_fmt, "-s %s -i %d", ntpServerHost1, interval);

			//get interface name by if_wan
			if_wan = check_ntp_if();

			sys_pid = read_pid("/var/run/systemd.pid");
			if(sys_pid > 0)
				kill(sys_pid, SIGUSR1);	//update DNS info
			sleep(1);	//wait a second for DNS updating
			memset(ifname, 0, sizeof(ifname));
			if (ifGetName(if_wan, ifname, IFNAMSIZ)) {
				strcat(argv_fmt, " -d %s");
				DOCMDARGVS(SNTPC, UNDOWAIT, argv_fmt, ifname);
			} else {
				// Only run SNTPC if interface is found.
				//DOCMDARGVS(SNTPC, UNDOWAIT, argv_fmt);
			}
		} else {
			status = 0;
		}
		break;
	}

	return status;
}

// return value:
// 1  : successful
// -1 : failed
inline int startNTP(void)
{
	return setupNtp(SNTP_ENABLED);
}

// return value:
// 1  : successful
// -1 : failed
inline int stopNTP(void)
{
	return setupNtp(SNTP_DISABLED);
}
#endif //TIME_ZONE

// Magician: E8B security
//ql: set firewall grade
//not take effect immediately while modify/add/del pvc
WAN_STATE_T wanState;
//use fw_state_t to reserve the set state of firewall grade
FW_GRADE_INIT_St fw_state_t;

/* mac to string function */
int hex(unsigned char ch)
{
	if (ch >= 'a' && ch <= 'f')
		return ch-'a'+10;
	if (ch >= '0' && ch <= '9')
		return ch-'0';
	if (ch >= 'A' && ch <= 'F')
		return ch-'A'+10;
	return -1;
}

//mac's size must be >=17, content format:"00-16-76-D9-A6-AF"
void convert_mac(char *mac)
{
	char temp[ETH_ALEN];
	int i;
	for(i = 0; i < ETH_ALEN; i++) {
		temp[i] = hex(mac[i * 3]) * 16 + hex(mac[i * 3 + 1]);
	}
	memcpy(mac, temp, ETH_ALEN);
}

int getifIndexByWanName(const char *name)
{
	int index=0;
	int cnt=mib_chain_total(MIB_ATM_VC_TBL);
	MIB_CE_ATM_VC_T pvcEntry;
	for(index=0;index<cnt;index++)
		{
			char itfname[256]={0};
			mib_chain_get(MIB_ATM_VC_TBL,index,&pvcEntry);
			getWanName(&pvcEntry, itfname);
			if(!strncmp(name,itfname,strlen(name)))
				return pvcEntry.ifIndex;
		}
     return -1;
}

char *getEbtablesProtocol(int type)
{
	static char *ebproto[]={
		"",
		"PPP_DISC",
		"IPv4",
		"IPv6",
		"ATALK",
		"IPX",
		"NetBEUI",
		"IGMP"
		};
	return ebproto[type];
}

void convertMacFormat(char *str, unsigned char *mac)
{
	char tmpBuf[4];
	int i, j = 0;

	for (i = 0; i < 17; i += 3)
	{
		tmpBuf[0] = str[i];
		tmpBuf[1] = str[i+1];
		tmpBuf[2] = 0;

		if (!isxdigit(tmpBuf[0]) || !isxdigit(tmpBuf[1]))
			return 0;

		mac[j++] = (unsigned char)strtol(tmpBuf, (char**)NULL, 16);
	}
	//printf("%x %x %x %x %x %x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int setupRouteEbtables()
{
	struct routemac_entry entry;
	int cnt,index, k;
	cnt=mib_chain_total(MIB_MAC_FILTER_ROUTER_TBL);
	DOCMDINIT;
	DOCMDARGVS(EBTABLES,DOWAIT,"-F %s",(char *)FW_MACFILTER_ROUTER);
	for(index=0;index<cnt;index++)
	{
		mib_chain_get(MIB_MAC_FILTER_ROUTER_TBL,index,&entry);
		#if defined(CONFIG_ETHWAN) || defined(CONFIG_RTL_MULTI_LAN_DEV)
		for(k=0;k<ELANVIF_NUM;k++) {
			//printf("ITF is %s\n", (char *)ELANVIF[k]);
			DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s -s %s -j DROP", (char *)FW_MACFILTER_ROUTER, (char *)ELANVIF[k], entry.mac);
		}
		#else
		DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i %s -s %s -j DROP", (char *)FW_MACFILTER_ROUTER, ELANIF, entry.mac);
		#endif
		DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -i wlan0 -s %s -j DROP", (char *)FW_MACFILTER_ROUTER, entry.mac);
	}
}

int setupMacFilterTables()
{
	unsigned char macFilterEnable;
	struct routemac_entry entry;
	int cnt,index, k;
#ifdef	CONFIG_RTK_RG_INIT
	unsigned char smac[MAC_ADDR_LEN];
#endif
	cnt=mib_chain_total(MIB_MAC_FILTER_ROUTER_TBL);
#ifdef MAC_FILTER_SRC_ONLY
	DOCMDINIT;
	DOCMDARGVS(EBTABLES,DOWAIT,"-F %s",(char *)FW_MACFILTER_ROUTER);
#endif
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", (char *)FW_MACFILTER_ROUTER);

#ifdef	CONFIG_RTK_RG_INIT
	FlushRTK_RG_MAC_Filters();
#endif

	mib_get(MIB_MAC_FILTER_SRC_ENABLE, &macFilterEnable);

	if(macFilterEnable)
	{
		for(index=0;index<cnt;index++)
		{
			mib_chain_get(MIB_MAC_FILTER_ROUTER_TBL,index,&entry);
#ifdef	CONFIG_RTK_RG_INIT
			convertMacFormat(entry.mac, smac);
			AddRTK_RG_RT_MAC_Filter(smac);
#endif
#ifdef MAC_FILTER_SRC_ONLY
			DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -s %s -j DROP", (char *)FW_MACFILTER_ROUTER, entry.mac);
#endif
			va_cmd(IPTABLES, 10, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_MACFILTER_ROUTER, "-m", "mac", "--mac-source", entry.mac, "-j", (char *)FW_DROP);
		}
	}
}
int convertProtocaltoEthertype(unsigned char protoType)
{

	static int ethertype_arr[]={
		0,
		0x8863, //PPP_DISC
		0x0800, //IPv4
		0x86DD, //IPv6
		0x809B, //ATALK
		0x8137, //IPX
		0x8191  //NetBEUI
		};
	return ethertype_arr[protoType];
}
int setupMacFilterEbtables()
{
	unsigned char macFilterMode=0;
	unsigned char macFilterEnable=0;
	struct brgmac_entry  entry;
	int cnt;
	int index=0;
#ifdef	CONFIG_RTK_RG_INIT
	unsigned char smac[MAC_ADDR_LEN];
	unsigned char dmac[MAC_ADDR_LEN];
	int ethertype;
	FlushRTK_RG_MAC_Filters_in_ACL();
#endif
	DOCMDINIT;
	DOCMDARGVS(EBTABLES, DOWAIT, "-F %s", (char *)FW_MACFILTER);

	mib_get(MIB_MAC_FILTER_EBTABLES_ENABLE, &macFilterEnable);
	mib_get(MIB_MAC_FILTER_EBTABLES_MODE, &macFilterMode);

	DBPRINT(0, "macFilterEnable=%d,macFilterMode=%d\n", macFilterEnable, macFilterMode);
	if(macFilterEnable)
	{
		cnt=mib_chain_total(MIB_MAC_FILTER_EBTABLES_TBL);
		for(index=0;index<cnt;index++)
		{
			char protoParm[256]={0};
			char smacParm[256]={0};
			char dmacParm[256]={0};
			char smac2Parm[256]={0};
			char dmac2Parm[256]={0};

			memset(&entry,0,sizeof(entry));
			mib_chain_get(MIB_MAC_FILTER_EBTABLES_TBL,index,&entry);

#ifdef	CONFIG_RTK_RG_INIT
			convertMacFormat(entry.smac, smac);
			convertMacFormat(entry.dmac, dmac);
			ethertype = convertProtocaltoEthertype(entry.protoType);
			AddRTK_RG_BG_MAC_Filter(&entry, smac, dmac, macFilterMode, ethertype);
#endif

			if(entry.protoType)
				snprintf(protoParm,sizeof(protoParm),"-p %s", getEbtablesProtocol(entry.protoType));

			if(strlen(entry.smac)) {
				snprintf(smacParm,sizeof(smacParm),"-s %s",entry.smac);
				snprintf(dmac2Parm,sizeof(smacParm),"-d %s",entry.smac);
			}

			if(strlen(entry.dmac)) {
				snprintf(dmacParm,sizeof(dmacParm),"-d %s",entry.dmac);
				snprintf(smac2Parm,sizeof(dmacParm),"-s %s",entry.dmac);
			}

			int itfindex=0;
			for(itfindex=0;itfindex<entry.portNum;itfindex++)
			{
				char itfName[8];
				char itfParm[16]={0};
				ifGetName(PHY_INTF(entry.ifIndex[itfindex]), itfName, sizeof(itfName));
				switch(entry.direction)
				{
					case 0://lan-->wan
						snprintf(itfParm,sizeof(itfParm),"-o %s",itfName);
						DOCMDARGVS(EBTABLES,DOWAIT,"-A %s %s %s  %s %s -j %s", (char *)FW_MACFILTER,
						itfParm,protoParm,smacParm,dmacParm,(macFilterMode?"RETURN":"DROP"));
						break;
					case 1://wan-->lan
						snprintf(itfParm,sizeof(itfParm),"-i %s",itfName);
						DOCMDARGVS(EBTABLES,DOWAIT,"-A %s %s %s  %s %s -j %s", (char *)FW_MACFILTER,
						itfParm,protoParm,smacParm,dmacParm,(macFilterMode?"RETURN":"DROP"));
						break;
					case 2://lan<-->wan
						snprintf(itfParm,sizeof(itfParm),"-o %s",itfName);
						DOCMDARGVS(EBTABLES,DOWAIT,"-A %s %s %s  %s %s -j %s", (char *)FW_MACFILTER,
						itfParm,protoParm,smacParm,dmacParm,(macFilterMode?"RETURN":"DROP"));
						memset(itfParm,0,sizeof(itfParm));
						snprintf(itfParm,sizeof(itfParm),"-i %s",itfName);
						DOCMDARGVS(EBTABLES,DOWAIT,"-A %s %s %s  %s %s -j %s", (char *)FW_MACFILTER,
						itfParm,protoParm,dmac2Parm,smac2Parm,(macFilterMode?"RETURN":"DROP"));
						break;
				}
			}
		}
#ifdef	CONFIG_RTK_RG_INIT
		RTK_RG_MAC_Filter_Default_Policy(macFilterMode? 0:1, macFilterMode? 0:1);
#endif
		DOCMDARGVS(EBTABLES,DOWAIT,"-P %s %s",(char *)FW_MACFILTER, (macFilterMode?"DROP":"RETURN"));
	}
	else {
		DOCMDARGVS(EBTABLES,DOWAIT,"-F %s", (char *)FW_MACFILTER);
		DOCMDARGVS(EBTABLES,DOWAIT,"-P %s RETURN", (char *)FW_MACFILTER);
	}

#ifdef CONFIG_RTL8672_BRIDGE_FASTPATH
	TRACE(STA_SCRIPT,"/bin/echo 2 > /proc/fastbridge\n");
	system("/bin/echo 2 > /proc/fastbridge");
#endif

}

int addLowGradeRule(VC_STATE_Pt pEntry, int enable)
{
	if (enable) {
		//iptables -I INPUT 1 -i vc0 -p udp --dport 69 -j ACCEPT
		//va_cmd(IPTABLES, 11, 1, (char *)FW_INSERT, "fwinput", "1", (char *)ARG_I,
		//	pEntry->ifName, "-p", "udp", "--dport", "69", "-j", (char *)FW_ACCEPT);
		//iptables -I INPUT 2 -i vc0 -p tcp --dport 23 -j ACCEPT
		//va_cmd(IPTABLES, 11, 1, (char *)FW_INSERT, "fwinput", "2", (char *)ARG_I,
		//	pEntry->ifName, "-p", "tcp", "--dport", "23", "-j", (char *)FW_ACCEPT);
		//iptables -I INPUT 3 -i vc0 -p icmp --icmp-type 8 -j ACCEPT
		va_cmd(IPTABLES, 9, 1, (char *)FW_INSERT, "fwinput", "3", "-p", "icmp", "--icmp-type", "8", "-j", (char *)FW_ACCEPT);
		//iptables -I INPUT 4 -i vc0 -p tcp --dport 80 -j ACCEPT
		//va_cmd(IPTABLES, 11, 1, (char *)FW_INSERT, "fwinput", "4", (char *)ARG_I,
		//	pEntry->ifName, "-p", "tcp", "--dport", "80", "-j", (char *)FW_ACCEPT);
	} else {
		//iptables -D INPUT -i vc0 -p udp --dport 69 -j ACCEPT
		//va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, "fwinput", (char *)ARG_I,
		//	pEntry->ifName, "-p", "udp", "--dport", "69", "-j", (char *)FW_ACCEPT);
		//iptables -D INPUT -i vc0 -p tcp --dport 23 -j ACCEPT
		//va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, "fwinput", (char *)ARG_I,
		//	pEntry->ifName, "-p", "tcp", "--dport", "23", "-j", (char *)FW_ACCEPT);
		//iptables -D INPUT -i vc0 -p icmp --icmp-type 8 -j ACCEPT
		va_cmd(IPTABLES,8, 1, (char *)FW_DEL, "fwinput","-p", "icmp", "--icmp-type", "8", "-j", (char *)FW_ACCEPT);
		//iptables -D INPUT -i vc0 -p tcp --dport 80 -j ACCEPT
		//va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, "fwinput", (char *)ARG_I,
		//	pEntry->ifName, "-p", "tcp", "--dport", "80", "-j", (char *)FW_ACCEPT);
	}

	return 1;
}

int changeFwGrade(int currGrade)
{
	int enable;
	int mibCnt;
	int totalNum;
	MIB_CE_ATM_VC_T entry;
	VC_STATE_T vcState;
	Modem_LinkSpeed vLs;

	#ifdef CONFIG_RTK_RG_INIT
	if(currGrade == (char)FW_HIGH)
		RG_tcp_stateful_tracking(1);
	else if(currGrade == (char)FW_LOW)
		RG_tcp_stateful_tracking(0);
	else // wrong ?! enable stateful tracking
		RG_tcp_stateful_tracking(1);
	#endif

	if (fw_state_t.preFwGrade == (char)FW_HIGH) {
		if (currGrade != (char)FW_HIGH)//HIGH->LOW
			enable = 1;
		else
			return 0;
	}
	else if (currGrade == (char)FW_HIGH) {
		if (fw_state_t.preFwGrade != FW_HIGH)//LOW->High
			enable = 0;
		else
			return 0;
	} else//don't change
		return 0;

#if 0
#ifdef CONFIG_DEV_xDSL
	//first, check for xDSL link
	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		return 0;//if link down, do nothing
#endif

	if (((char)WAN_INVALID==wanState.xDSLMode) || ((char)WAN_BRG==wanState.xDSLMode))
		return 0;
#endif
	totalNum = mib_chain_total(MIB_ATM_VC_TBL);
	for(mibCnt=0; mibCnt<totalNum; mibCnt++)
	{
		memset(&vcState, 0, sizeof(VC_STATE_T));

		if (!mib_chain_get(MIB_ATM_VC_TBL, mibCnt, (void *)&entry))
		{
			printf("get MIB chain error\n");
			return -1;
		}
		if ((entry.cmode == CHANNEL_MODE_BRIDGE) || !entry.enable)
			continue;


		ifGetName(entry.ifIndex, vcState.ifName, sizeof(vcState.ifName));

		addLowGradeRule(&vcState, enable);
	}

	fw_state_t.preFwGrade = currGrade;

	return 1;
}

#define DOS_ENABLE_ALL	(DOS_ENABLE|SYSFLOODSYN|SYSFLOODFIN|SYSFLOODUDP|SYSFLOODICMP|IPFLOODSYN|\
						IPFLOODFIN|IPFLOODUDP|IPFLOODICMP|TCPUDPPORTSCAN|ICMPSMURFENABLED|\
						IPLANDENABLED|IPSPOOFENABLED|IPTEARDROPENABLED|PINGOFDEATHENABLED|\
						TCPSCANENABLED|TCPSynWithDataEnabled|UDPBombEnabled|UDPEchoChargenEnabled|\
						sourceIPblock)
//#define DOS_ENABLE_E8B	DOS_ENABLE_ALL
#define DOS_ENABLE_E8B	(DOS_ENABLE|SYSFLOODSYN|SYSFLOODFIN|SYSFLOODUDP|SYSFLOODICMP|IPFLOODSYN|\
						IPFLOODFIN|IPFLOODUDP|IPFLOODICMP|TCPUDPPORTSCAN|ICMPSMURFENABLED|\
						IPLANDENABLED|IPSPOOFENABLED|IPTEARDROPENABLED|PINGOFDEATHENABLED|\
						TCPSCANENABLED|TCPSynWithDataEnabled|UDPBombEnabled|sourceIPblock)

int setupDos()
{
	unsigned int dos_enable=0;
	unsigned char buffer[256],dosstr[256];
	int dostmpip[4],dostmpmask[4];
	int dosip,dosmask;
	FILE *dosfp;

	if (!mib_get(MIB_DOS_ENABLED,  (void *)&dos_enable)){
		printf("DOS parameter failed!\n");
	}

	//get ip
	if(!mib_get(MIB_ADSL_LAN_IP, (void *)buffer)) {
		printf("get LAN IP failed!\n");
		return -1;
	}
	sscanf(inet_ntoa(*((struct in_addr *)buffer)),"%d.%d.%d.%d",&dostmpip[0],&dostmpip[1],&dostmpip[2],&dostmpip[3]);
	dosip= dostmpip[0]<<24 | dostmpip[1]<<16 | dostmpip[2]<<8 | dostmpip[3];
	//get mask
	if(!mib_get(MIB_ADSL_LAN_SUBNET, (void *)buffer)) {
		printf("get LAN SUBNET failed!\n");
		return -1;
	}
	sscanf(inet_ntoa(*((struct in_addr *)buffer)),"%d.%d.%d.%d",&dostmpmask[0],&dostmpmask[1],&dostmpmask[2],&dostmpmask[3]);
	dosmask= dostmpmask[0]<<24 | dostmpmask[1]<<16 | dostmpmask[2]<<8 | dostmpmask[3];
	dosip &= dosmask;
	if (dos_enable) {
		//printf("dos enable:%d\n", dos_enable);
		dos_enable = DOS_ENABLE_E8B;
	}

#ifdef CONFIG_DOS
	sprintf(dosstr,"1 %x %x %d 100 100 100 100 100 100 100 100 300\n", dosip,dosmask,dos_enable);
	//sprintf(dosstr,"1 %x %x %d 1 1 1 1 1 1 1 1 30\n", dosip,dosmask,dos_enable);
	//printf("dosstr:%s\n\n",dosstr);
	dosstr[strlen(dosstr)] = 0;
	if (dosfp = fopen("/proc/enable_dos", "w"))
	{
		fprintf(dosfp, "%s",dosstr);
		fclose(dosfp);
	}
#endif

#ifdef CONFIG_RTK_RG_INIT
	//setup dos
	RTK_RG_DoS_Set(dos_enable);
#endif

	return 1;
}
// End Magician: E8B security

#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
static int find_alarm_num(unsigned int alarm_num)
{
	int total = mib_chain_total(CWMP_CT_ALARM_TBL);
	CWMP_CT_ALARM_T alarm;
	int i;

	for(i = 0 ; i < total ; i++)
	{
		mib_chain_get(CWMP_CT_ALARM_TBL, i, &alarm);
		if(alarm.alarmNumber == alarm_num)
			break;
	}

	if(i >= total)
		i = -1;

	return i;
}

int set_ctcom_alarm(unsigned int alarm_num)
{
	if(find_alarm_num(alarm_num) == -1)
	{
		CWMP_CT_ALARM_T alarm = {0};

		alarm.alarmNumber = alarm_num;
		alarm.mode = ALARM_MODE_NONE;

		mib_chain_add(CWMP_CT_ALARM_TBL, (void *)&alarm);
		mib_update(CURRENT_SETTING, CONFIG_MIB_CHAIN);
	}
}

int clear_ctcom_alarm(unsigned int alarm_num)
{
	int num = find_alarm_num(alarm_num);

	if(num != -1)
	{
		mib_chain_delete(CWMP_CT_ALARM_TBL, num);
		mib_update(CURRENT_SETTING, CONFIG_MIB_CHAIN);
	}
}
#endif //_PRMT_X_CT_COM_ALARM_MONITOR_

#ifdef CONFIG_USER_RTK_LBD
void setupLBD(void)
{
	unsigned char enable = 0;
#ifdef CONFIG_EPON_FEATURE
	int i;
	char cmd[512];
	int loopdetect_period=10;
	int loopdrecover_time=300;
	unsigned short ether_type=0xfffa;
	MIB_CE_LBD_VLAN_T loopdetect_vlan;
	int totalvlan;
	unsigned int pon_mode=0;
#endif
	int pid;
	
	mib_get(MIB_LBD_ENABLE, &enable);
#ifdef CONFIG_EPON_FEATURE
	mib_get(MIB_PON_MODE, &pon_mode);
	if(pon_mode == EPON_MODE)
	{
		sprintf(cmd,"oamcli set ctc loopdetect");
		mib_get(MIB_LBD_ETHER_TYPE, &ether_type);
		mib_get(MIB_LBD_EXIST_PERIOD, &loopdetect_period);
		mib_get(MIB_LBD_CANCEL_PERIOD, &loopdrecover_time);
		sprintf(cmd,"%s %d 0x%x %d %d",cmd,enable, ether_type,loopdetect_period,loopdrecover_time);
		totalvlan=mib_chain_total(MIB_LBD_VLAN_TBL);
		for(i=0; i<totalvlan; i++)
		{
			if(mib_chain_get(MIB_LBD_VLAN_TBL,i,&loopdetect_vlan))
			{
				if(i==0)
					sprintf(cmd, "%s %d", cmd, loopdetect_vlan.vid);
				else
					sprintf(cmd, "%s,%d", cmd, loopdetect_vlan.vid);
			}
		}
		printf("%s\n",cmd);
		system(cmd);
		return;
	}
#endif
	pid = read_pid("/var/run/loopback.pid");
	if(pid > 0)
	{
		kill(pid, SIGTERM);
		while(read_pid("/var/run/loopback.pid") > 0)
			usleep(200);
	}

	if(enable)
		va_cmd("/bin/loopback", 0, 0);
}
#endif

int update_hosts(char *hostname, struct addrinfo *servinfo)
{
	if(!hostname || !servinfo) return 0;

	struct addrinfo *p;
	struct sockaddr_in *h;
	struct sockaddr_in6 *h6;
	struct in_addr ina;
	struct in6_addr ina6;

	FILE *fp;
	int isV4Existed = 0, isV6Existed = 0;
	char line[80], ip[64];

	if(inet_pton(AF_INET6, hostname, &ina6) > 0 || inet_pton(AF_INET, hostname, &ina) > 0)
		return 0;

	fp = fopen(HOSTS, "a+");
	while(fgets(line, 80, fp) != NULL)
	{
		char *ip,*name,*saveptr1;
		ip = strtok_r(line," \n", &saveptr1);
		name = strtok_r(NULL," \n", &saveptr1);

		if(name && !strcmp(name, hostname))
		{
			if(strchr(ip, '.') != NULL)
				isV4Existed = 1;
			else
				isV6Existed = 1;
		}
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next)
	{
		if(p->ai_family == AF_INET)
		{
			h = (struct sockaddr_in *) p->ai_addr;
			strcpy(ip, inet_ntoa(h->sin_addr));

			if(!isV4Existed)
				fprintf(fp, "%-15s %s\n", ip, hostname);
		}
		else if(p->ai_family == AF_INET6)
		{
			h6 = (struct sockaddr_in6 *) p->ai_addr;
			inet_ntop(AF_INET6, &h6->sin6_addr, ip, sizeof(ip));

			if(!isV6Existed)
	 			fprintf(fp, "%-15s %s\n", ip, hostname);
		}
	}

	fclose(fp);
	return 1;
}

// IMPORTANGE NOTE: caller of this API must use freeaddrinfo() to free the returned memory.
struct addrinfo *hostname_to_ip(char *hostname, IP_PROTOCOL IPVer)
{
	struct addrinfo hints, *servinfo;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = IPVer == IPVER_IPV4? AF_INET: IPVer == IPVER_IPV6? AF_INET6: AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(hostname , NULL, &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return NULL;
	}

	update_hosts(hostname, servinfo);

	return servinfo;
}

#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
int query_aftr(char *aftr,  char *aftr_dst, char *aftr_addr_str)
{
	struct addrinfo hints, *res, *p;
	int status;
	char ipstr[INET6_ADDRSTRLEN];
	void *addr;

	if(!aftr||!aftr_dst||!aftr_addr_str){
		printf("%s Error! Null Input! \n",__func__);
		return 2;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;

	printf("aftr=[%s]\n",aftr);
	if ((status = getaddrinfo(aftr, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "getaddrinfo:%s %s\n", aftr, gai_strerror(status));
		return 2;
	}

	printf("%s: IP addresses for [%s]:", __func__,aftr);

	p = res;
	if (p &&p->ai_family == AF_INET6) { // IPv6
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
		addr = &(ipv6->sin6_addr);
	}

	memcpy(aftr_dst,addr,sizeof(struct in6_addr));

	// convert the IP to a string and print it:
	inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
	printf("[%s]\n",ipstr);

	strcpy(aftr_addr_str,ipstr);

	freeaddrinfo(res);

	return 0;
}
#endif

//return 1: registered, return 0: failed.
int check_user_is_registered()
{
	unsigned int regStatus = 99;

	mib_get(CWMP_USERINFO_STATUS, &regStatus);

	if(regStatus == 0 || regStatus == 5)
		return 1;

	return 0;
}


/*
 *	LAN port ifidx has been changed to VLAN-based port mapping.
 *	This routine resolve the possible port-mapping mode conflict.
 */
int sync_itfGroup(int ifidx)
{
	MIB_CE_ATM_VC_T vcEntry;
	int total, i;

	total = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0; i<total; i++) {
		mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vcEntry);
		if (vcEntry.itfGroup & (1<<ifidx)) {
			// Remove this port from port-based mapping as it is vlan-based mode now.
			vcEntry.itfGroup &= ~(1<<ifidx);
			mib_chain_update(MIB_ATM_VC_TBL, (void*)&vcEntry, i);
		}
	}
	return 0;
}

#ifdef CONFIG_HTTP_DOWNLOAD_TEST
struct callout httptest_ch;
#endif

/*************************************************************************/
#ifdef SUPPORT_ACCESS_RIGHT

void flush_internet_access_right()
{
	DOCMDINIT;
	
	DOCMDARGVS(EBTABLES,DOWAIT,"-F %s",(char *)FW_INTERNET_ACCESSRIGHT_BRIDGE);
	va_cmd(EBTABLES, 4, 1, (char *)FW_DEL, (char *)FW_FORWARD, "-j", (char *)FW_INTERNET_ACCESSRIGHT_BRIDGE);
	va_cmd(EBTABLES, 2, 1, "-X", (char *)FW_INTERNET_ACCESSRIGHT_BRIDGE);
		
	va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_INTERNET_ACCESSRIGHT_ROUTER);
	va_cmd(IPTABLES, 4, 1, (char *)FW_DEL, (char *)FW_FORWARD, "-j", (char *)FW_INTERNET_ACCESSRIGHT_ROUTER);
	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_INTERNET_ACCESSRIGHT_ROUTER);	

#ifdef	CONFIG_RTK_RG_INIT
	FlushRTK_RG_RT_INTERNET_ACCESS_RIGHT();
#endif
}

void setup_internet_access_right(void)
{
	
	int totalNum,index;
	unsigned char macString[32] = {0};
	MIB_LAN_HOST_ACCESS_RIGHT_T entry;
	DOCMDINIT;
	
	totalNum = mib_chain_total(MIB_LAN_HOST_ACCESS_RIGHT_TBL);
	for(index=0; index<totalNum; index++)
	{
		mib_chain_get(MIB_LAN_HOST_ACCESS_RIGHT_TBL,index,&entry);
		if( entry.internetAccessRight == 0 )
		{
			if( index == 0 )
			{
				va_cmd(EBTABLES, 2, 1, "-N", (char *)FW_INTERNET_ACCESSRIGHT_BRIDGE);
				va_cmd(EBTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)FW_INTERNET_ACCESSRIGHT_BRIDGE);

				va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_INTERNET_ACCESSRIGHT_ROUTER);
				va_cmd(IPTABLES, 4, 1, (char *)FW_INSERT, (char *)FW_FORWARD, "-j", (char *)FW_INTERNET_ACCESSRIGHT_ROUTER);
			}
#ifdef	CONFIG_RTK_RG_INIT
			AddRTK_RG_RT_INTERNET_ACCESS_RIGHT(entry.mac);
#endif
			sprintf(macString, "%02x:%02x:%02x:%02x:%02x:%02x", entry.mac[0], entry.mac[1], entry.mac[2], entry.mac[3], entry.mac[4], entry.mac[5]);
			
			DOCMDARGVS(EBTABLES,DOWAIT,"-A %s -s %s -j DROP", (char *)FW_INTERNET_ACCESSRIGHT_BRIDGE, macString);
			va_cmd(IPTABLES, 8, 1, (char *)FW_ADD, (char *)FW_INTERNET_ACCESSRIGHT_ROUTER, "-m", "mac", "--mac-source", macString, "-j", (char *)FW_DROP);
		}
	}
}

void flush_storage_access_right()
{
	va_cmd(IPTABLES, 2, 1, "-F", (char *)FW_STORAGE_ACCESSRIGHT_ROUTER);
	va_cmd(IPTABLES, 4, 1, (char *)FW_DEL, (char *)FW_INPUT, "-j", (char *)FW_STORAGE_ACCESSRIGHT_ROUTER);
	va_cmd(IPTABLES, 2, 1, "-X", (char *)FW_STORAGE_ACCESSRIGHT_ROUTER);			
}
void setup_storage_access_right(void)
{
	
	int totalNum,index;
	unsigned char macString[32] = {0};
	MIB_LAN_HOST_ACCESS_RIGHT_T entry;
	totalNum = mib_chain_total(MIB_LAN_HOST_ACCESS_RIGHT_TBL);
	
	for(index=0; index<totalNum; index++)
	{
		mib_chain_get(MIB_LAN_HOST_ACCESS_RIGHT_TBL,index,&entry);
		if( entry.storageAccessRight == 0 )
		{
			if(index == 0)
			{
				va_cmd(IPTABLES, 2, 1, "-N", (char *)FW_STORAGE_ACCESSRIGHT_ROUTER);
				va_cmd(IPTABLES, 4, 1, (char *)FW_INSERT, (char *)FW_INPUT, "-j", (char *)FW_STORAGE_ACCESSRIGHT_ROUTER);
			}
			
			sprintf(macString, "%02x:%02x:%02x:%02x:%02x:%02x", entry.mac[0], entry.mac[1], entry.mac[2], entry.mac[3], entry.mac[4], entry.mac[5]);
			
			/* iptables -A FW_STORAGE_ACCESSRIGHT_ROUTER -m mac --mac-source xx:xx:xx:xx:xx:xx -p tcp --dport 21 -j DROP */
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char*)FW_STORAGE_ACCESSRIGHT_ROUTER, "-m", "mac", "--mac-source", macString, "-p", "tcp", "--dport", "21", "-j", (char *)FW_DROP);

			/* iptables -A FW_STORAGE_ACCESSRIGHT_ROUTER -m mac --mac-source xx:xx:xx:xx:xx:xx -p tcp --dport 139/445 -j DROP */
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char*)FW_STORAGE_ACCESSRIGHT_ROUTER, "-m", "mac", "--mac-source", macString, "-p", "tcp", "--dport", "139", "-j", (char *)FW_DROP);
			va_cmd(IPTABLES, 12, 1, (char *)FW_ADD, (char*)FW_STORAGE_ACCESSRIGHT_ROUTER, "-m", "mac", "--mac-source", macString, "-p", "tcp", "--dport", "445", "-j", (char *)FW_DROP);
		}
	}
}

void apply_accessRight(uint8 enable)
{
	flush_internet_access_right();
	if(enable)
		setup_internet_access_right();

	flush_storage_access_right();
	if(enable)	
		setup_storage_access_right();
}
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
typedef struct lanport_ctlrate_info{
	unsigned int upRate;   /* in unit of kbps */
	unsigned int downRate; /* in unit of kbps */
}LANPORT_CTLRATE_INFO;

struct callout macinfo_ch;
LANPORT_CTLRATE_INFO lanPortCtlRateInfo[4];

/*
Return: 
	0 - no change in  lanPortCtlRateInfo.
	1 - policy changed in lanPortCtlRateInfo.
	others - error occured.
 */
int reCalculateLanPortRateLimitInfo(void)
{
	int i, totalNum, macidx;
	unsigned int ratelimit;
	MIB_LAN_HOST_BANDWIDTH_T entry;
	rtk_rg_macEntry_t macEntry;
	LANPORT_CTLRATE_INFO oldInfo[4];

	memcpy(oldInfo, lanPortCtlRateInfo , sizeof(lanPortCtlRateInfo));
	memset(lanPortCtlRateInfo, 0, sizeof(lanPortCtlRateInfo));
	memset(&entry, 0 , sizeof(MIB_LAN_HOST_BANDWIDTH_T));
	totalNum = mib_chain_total(MIB_LAN_HOST_BANDWIDTH_TBL);
	for(i=0; i<totalNum; i++)
	{
		if (!mib_chain_get(MIB_LAN_HOST_BANDWIDTH_TBL, i, (void *)&entry)) 
		{
			printf("[%s %d]get LAN_HOST_BANDWIDTHl chain entry error!\n", __func__, __LINE__);
			return -1;
		}

		macidx = -1;
		memset(&macEntry, 0, sizeof(rtk_rg_macEntry_t));	
		strncpy(macEntry.mac.octet, entry.mac, sizeof(macEntry.mac.octet));
		if(RT_ERR_RG_OK == rtk_rg_macEntry_find(&macEntry,&macidx))
		{
			//Upstream
			if((0 == lanPortCtlRateInfo[macEntry.port_idx].upRate)
				||((entry.maxUsBandwidth!=0)&&(entry.maxUsBandwidth < lanPortCtlRateInfo[macEntry.port_idx].upRate)))
			{
				lanPortCtlRateInfo[macEntry.port_idx].upRate = entry.maxUsBandwidth;
			}

			//Downstream
			if((0 == lanPortCtlRateInfo[macEntry.port_idx].downRate)
				||((entry.maxDsBandwidth!=0)&&(entry.maxDsBandwidth < lanPortCtlRateInfo[macEntry.port_idx].downRate)))
			{
				lanPortCtlRateInfo[macEntry.port_idx].downRate = entry.maxDsBandwidth;
			}
		}
		else
		{
			//printf("[%s %d]get macEntry[0x%02x-%02x-%02x-%02x-%02x-%02x] failed.\n", 
				//__func__, __LINE__, entry.mac[0], entry.mac[1], entry.mac[2], entry.mac[3], entry.mac[4], entry.mac[5]);
			return -1;
		}
	}

	if(!memcmp(oldInfo, lanPortCtlRateInfo, sizeof(lanPortCtlRateInfo)))
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

void apply_lanPortRateLimit(void)
{
	int port = 0;

	for(port=0; port<4; port++)
	{
		if(RT_ERR_RG_OK == rtk_rg_portIgrBandwidthCtrlRate_set(port, lanPortCtlRateInfo[port].upRate))
		{
			//printf("[%s %d]Set portIgrBandwidthCtrlRate, port=%d, rate=%d\n", __func__, __LINE__, port, lanPortCtlRateInfo[port].upRate);
		}
		else
		{
			printf("[%s %d]Failed to Set portIgrBandwidthCtrlRate, port=%d, rate=%d\n", __func__, __LINE__, port, lanPortCtlRateInfo[port].upRate);
			return;
		}

		if(RT_ERR_RG_OK == rtk_rg_portEgrBandwidthCtrlRate_set(port, lanPortCtlRateInfo[port].downRate))
		{
			//printf("[%s %d]Set portEgrBandwidthCtrlRate, port=%d, rate=%d\n", __func__, __LINE__, port, lanPortCtlRateInfo[port].downRate);
		}
		else
		{
			printf("[%s %d]Failed to Set portEgrBandwidthCtrlRate, port=%d, rate=%d\n", __func__, __LINE__, port, lanPortCtlRateInfo[port].downRate);
			return;
		}
	}
}

void clear_lanPortRateLimit(void)
{
	int port = 0;

	for(port=0; port<4; port++)
	{
		if(RT_ERR_RG_OK == rtk_rg_portIgrBandwidthCtrlRate_set(port, 0))
		{
			//printf("[%s %d]Clear portIgrBandwidthCtrlRate, port=%d\n", __func__, __LINE__, port);
		}
		else
		{
			printf("[%s %d]Failed to Clear portIgrBandwidthCtrlRate, port=%d\n", __func__, __LINE__, port);
			return;
		}

		if(RT_ERR_RG_OK == rtk_rg_portEgrBandwidthCtrlRate_set(port, 0))
		{
			//printf("[%s %d]Clear portEgrBandwidthCtrlRate, port=%d\n", __func__, __LINE__, port);
		}
		else
		{
			printf("[%s %d]Failed to Clear portEgrBandwidthCtrlRate, port=%d\n", __func__, __LINE__, port);
			return;
		}
	}
}

void flush_maxBandwidth(void)
{
	FlushRTK_RG_RT_MAX_US_BANDWIDTH();
	FlushRTK_RG_RT_MAX_DS_BANDWIDTH();
}

/* setup maxDsBandwidth */
void setup_maxBandwidth(void)
{
	unsigned char lanHostBandwidthControlEnable=0;
	
	/* check if this function is on */
	mib_get(MIB_LANHOST_BANDWIDTH_CONTROL_ENABLE, (void *)&lanHostBandwidthControlEnable);
	if(lanHostBandwidthControlEnable == 1)
	{
		AddRTK_RG_RT_MAX_US_BANDWIDTH();
		AddRTK_RG_RT_MAX_DS_BANDWIDTH();	
	}
}

void poll_macinfo(void)
{
	unsigned char lanHostBandwidthControlEnable=0;

	mib_get(MIB_LANHOST_BANDWIDTH_CONTROL_ENABLE, (void *)&lanHostBandwidthControlEnable);
	if((1==lanHostBandwidthControlEnable)&&(1 == reCalculateLanPortRateLimitInfo()))
	{
		apply_lanPortRateLimit();
	}

	TIMEOUT(poll_macinfo, 0, 1, macinfo_ch);
}

void apply_maxBandwidth(uint8 enable)
{
#if 0
	//by martin_zhu
	flush_maxBandwidth();

	if(enable)
		setup_maxBandwidth();
#else
	unsigned char lanHostBandwidthControlEnable=0;

	mib_get(MIB_LANHOST_BANDWIDTH_CONTROL_ENABLE, (void *)&lanHostBandwidthControlEnable);
	//printf("[%s %d]enable=%d, lanHostBandwidthControlEnable=%d\n", __func__, __LINE__, enable, lanHostBandwidthControlEnable);
	
	reCalculateLanPortRateLimitInfo();
	if(lanHostBandwidthControlEnable)
		apply_lanPortRateLimit();
	else
		clear_lanPortRateLimit();
#endif
}
#endif

#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
void apply_lanBandWidthMonitor()
{
	//printf("[%s %d]Enter.\n", __func__, __LINE__);
	UNTIMEOUT(poll_lanbandwidth, 0, lanbandwidth_ch);
	startLanPortMibInfoTimer();
}
#endif

unsigned int isMacAddrInvalid(unsigned char *pMac) 
{
	if (pMac == NULL)
		return 1;
	
	if(pMac[0]&0x01)
		return 1;

	if(0x00 == (pMac[0]|pMac[1]|pMac[2]|pMac[3]|pMac[4]|pMac[5]) )
		return 1;
	
	return 0;
}

unsigned int macAddrCmp(unsigned char *addr1, unsigned char *addr2) 
{
	unsigned short *a = (unsigned short *)addr1;
	unsigned short *b = (unsigned short *)addr2;
	
	return ((a[0] ^ b[0]) | (a[1] ^ b[1]) | (a[2] ^ b[2])) != 0;
}

#ifdef CONFIG_USER_LANNETINFO
/*
 * FUNCTION:GET_LAN_NET_INFO
 *
 * INPUT:
 *
 * RETURN:
 *	successful--RT_ERR_CTC_IGD1_OK
 */
int flock_set(int fd, int type)  
{  
	struct flock lock;	
	lock.l_whence = SEEK_SET;  
	lock.l_start = 0;  
	lock.l_len = 0;  
	lock.l_type = type;  
	lock.l_pid = getpid();
	  
	if((fcntl(fd, F_SETLKW, &lock)) < 0)  
	{  
		printf("Lock failed:type = %d\n", lock.l_type);  
		return 1;  
	}  
	return 0;  
}

#define LANNETINFOFILE	"/var/lannetinfo"
#define LANNETINFO_RUNFILE	"/var/run/lannetinfo.pid"
int get_lan_net_info(lanHostInfo_t **ppLANNetInfoData, unsigned int *pCount)
{
	struct stat status;
	int fd, i, read_size, pid;
	lanHostInfo_t *pLANNetInfoData = NULL;
	
	if( (ppLANNetInfoData==NULL)||(pCount==NULL) )
		return -1;
	
	pid = read_pid(LANNETINFO_RUNFILE);
	if( pid>0 )
		kill(pid, SIGUSR2);
		
	usleep(30000);//30ms
	
	if ( stat(LANNETINFOFILE, &status) < 0 )
		return -3;
		
	fd = open(LANNETINFOFILE, O_RDONLY); 
	if(fd < 0)
		return -3;
		
	read_size = (unsigned long)(status.st_size);
	*pCount = read_size / sizeof(lanHostInfo_t);
	
	pLANNetInfoData = (lanHostInfo_t *) malloc(read_size);
		
	if(pLANNetInfoData == NULL)
		return -3;
	
	*ppLANNetInfoData = pLANNetInfoData;
	memset(pLANNetInfoData, 0, read_size);
	
	if(!flock_set(fd, F_RDLCK))
	{
		read_size = read(fd, (void*)pLANNetInfoData, read_size);
		flock_set(fd, F_UNLCK);
	}
	
	close(fd);
	return 0;	
}
#endif

/*
 * FUNCTION:SET_ATTACH_DEVICE_RIGHT
 *
 * INPUT:
 *	macAddr: 
 *		target device's MAC address
 *	internetAccessRight: 
 *		internet access right, 1-ON, 0-OFF
 *  storageAccessRight:
 *		storage access right, 1-ON, 0-OFF
 *
 * RETURN:
 *	0  successful
 *	-1 Input error
 *  -2 device not find
 *  -3 Other error
 */
#ifdef SUPPORT_ACCESS_RIGHT
int32 set_attach_device_right(unsigned char *pMacAddr, unsigned char internetAccessRight, unsigned char storageAccessRight)
{
	int ret, i, totalNum;
	MIB_LAN_HOST_ACCESS_RIGHT_T entry;
	
	/* check input first */
	if( isMacAddrInvalid(pMacAddr) )
		return -1;

	/* get Host by macAddr */
	 totalNum = mib_chain_total(MIB_LAN_HOST_ACCESS_RIGHT_TBL);

	memset(&entry, 0 , sizeof(MIB_LAN_HOST_ACCESS_RIGHT_T));
	for(i=0; i<totalNum; i++)
	{
		if(!mib_chain_get(MIB_LAN_HOST_ACCESS_RIGHT_TBL, i, (void*)&entry))
		{
			printf("Get lan host access right mib entry error.\n");
			return -3;
		}
		
		if( !macAddrCmp(pMacAddr, entry.mac))
		{
			if( (internetAccessRight==DEFAULT_INTERNET_ACCESS_RIGHT) || (storageAccessRight==DEFAULT_INTERNET_ACCESS_RIGHT) )//delete mib entry
			{
				printf("Delete the target mib entry.\n");
				mib_chain_delete(MIB_LAN_HOST_ACCESS_RIGHT_TBL, i);
				goto apply;
			}
			/* Set internetAccessRight and storageAccessRight by macAddr */
			entry.internetAccessRight = internetAccessRight;
			entry.storageAccessRight = storageAccessRight;
			mib_chain_update(MIB_LAN_HOST_ACCESS_RIGHT_TBL, (void*)&entry, i);
			goto apply;
		}	
	}
	
	if( (DEFAULT_INTERNET_ACCESS_RIGHT == entry.internetAccessRight)&&(DEFAULT_INTERNET_ACCESS_RIGHT == entry.storageAccessRight) )
	{
		printf("InternetAccessRight and StorageAccessRight is default value, so return direct.\n");
		goto end;
	}
	/* add an entry */
	memset(&entry, 0 , sizeof(MIB_LAN_HOST_ACCESS_RIGHT_T));
	memcpy(entry.mac, pMacAddr, MAC_ADDR_LEN);

	entry.internetAccessRight = internetAccessRight;
	entry.storageAccessRight = storageAccessRight;

	ret = mib_chain_add(MIB_LAN_HOST_ACCESS_RIGHT_TBL, (void*)&entry);
	if(ret == 0)
		return -3;
	
apply:
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY	
	apply_accessRight(1);

end:	
	return 0;
}
#endif


/*
 * FUNCTION:QUERY_ATTACH_DEVICE_REALRATE
 *
 * INPUT:
 *	macAddr: 
 *		target device's MAC address
 *
 * RETURN:
 *	0  -- successful
 *	-1 -- Input error--RT_ERR_CTC_IGD1_INPUT
 *  -2 -- device not found
 *  -3 -- other
 */
#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
int query_attach_device_realrate(unsigned char *pMacAddr, unsigned int *pUsBandWidth, unsigned int *pDsBandWidth)
{
	/* get current Us/Ds Bandwidth */
	unsigned char lanHostBandwidthMonitorEnable=0;
	rtk_rg_macEntry_t macEntry;
	int index = -1;

	/* check input first */
	if( isMacAddrInvalid(pMacAddr) )
	{
		return -1;
	}
	
	mib_get(MIB_LANHOST_BANDWIDTH_MONITOR_ENABLE, (void*)&lanHostBandwidthMonitorEnable);
	if(lanHostBandwidthMonitorEnable == 0)
	{
		printf("Func is off.\n");
		return	-2;
	}
	
	/* 9600 series not support this function, so we use port instead. */
	memset(&macEntry, 0x0, sizeof(rtk_rg_macEntry_t));
	memcpy(macEntry.mac.octet, pMacAddr, sizeof(macEntry.mac.octet));
	if(RT_ERR_RG_OK == rtk_rg_macEntry_find(&macEntry,&index))
	{
		if(RT_ERR_RG_OK == getRealRatebyPort(macEntry.port_idx, pUsBandWidth, pDsBandWidth))
		{
			return 0;
		}
		else
		{
			return -3;
		}
	}
	else
	{
		printf("[%s %d]get macEntry failed.\n", __func__, __LINE__);
		return -3;
	}
	return -3;
}
#endif

/*
 * FUNCTION:SET_ATTACH_DEVICE_BANDWIDTH
 *
 * INPUT:
 *	macAddr: 
 *		target device's MAC address
 *	usBandWidth:
 *		Upstream bandwidth
 *	dsBandWidth:
 *		Downstream bandwidth
 * RETURN:
 *	0  -- successful--RT_ERR_CTC_IGD1_OK
 *	-1 -- Input error
 *  -2 -- device not find
 *  -3 -- other
 */
#ifdef CONFIG_USER_LAN_BANDWIDTH_CONTROL
int set_attach_device_maxbandwidth(unsigned char *pMacAddr, unsigned int maxUsBandWidth, unsigned int maxDsBandWidth)
{
	int ret, i, totalNum;
	unsigned char lanHostBandwidthControlEnable=0;
	MIB_LAN_HOST_BANDWIDTH_T entry;
	
	/* check if this function is on */
	mib_get(MIB_LANHOST_BANDWIDTH_CONTROL_ENABLE, (void*)&lanHostBandwidthControlEnable);
	if(lanHostBandwidthControlEnable == 0)
		return	-3;
	
	/* check input first */
	if(isMacAddrInvalid(pMacAddr))
		return -1;
	
	if( (maxUsBandWidth>=DEFAULT_MAX_US_BANDWIDTH)||(maxUsBandWidth>=DEFAULT_MAX_US_BANDWIDTH) )
		return -1;
	
	totalNum = mib_chain_total(MIB_LAN_HOST_BANDWIDTH_TBL);
	memset(&entry, 0 , sizeof(MIB_LAN_HOST_BANDWIDTH_T));
	for(i=0; i<totalNum; i++)
	{
		if(!mib_chain_get(MIB_LAN_HOST_BANDWIDTH_TBL, i, (void*)&entry))
		{	
			printf("Get lan host bandwidth table error.\n");
			return -3;
		}
		
		if( !macAddrCmp(pMacAddr, entry.mac))
		{
			/* Set usBandwidth and dsBandwidth by macAddr */
			entry.maxUsBandwidth = maxUsBandWidth;
			entry.maxDsBandwidth = maxDsBandWidth;
			
			mib_chain_update(MIB_LAN_HOST_BANDWIDTH_TBL, (void*)&entry, i);
			goto apply;
		}	
	}

	/* add an entry */
	memset(&entry, 0 , sizeof(MIB_LAN_HOST_BANDWIDTH_T));
	memcpy( entry.mac, pMacAddr, MAC_ADDR_LEN );
	entry.maxUsBandwidth = maxUsBandWidth;
	entry.maxDsBandwidth = maxDsBandWidth;	
	
	ret = mib_chain_add(MIB_LAN_HOST_BANDWIDTH_TBL, (void*)&entry);
	if (ret == 0)
		return -3;
apply:
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif // of #if COMMIT_IMMEDIATELY
	apply_maxBandwidth(1);
	return 0;
}


/*
 * FUNCTION:GET_ATTACH_DEVICE_BANDWIDTH
 *
 * INPUT:
 *	macAddr: 
 *		target device's MAC address
 *	pUsBandWidth:
 *		save upstream bandwidth
 *	pDsBandWidth:
 *		save downstream bandwidth
 * RETURN:
 *	0  -- successful--RT_ERR_CTC_IGD1_OK
 *	-1 -- Input error
 *  -2 -- device not find
 *  -3 -- other
 */
int get_attach_device_maxbandwidth(unsigned char *pMacAddr, unsigned int *pUsBandWidth, unsigned int *pDsBandWidth)
{
	int i, totalNum;
	unsigned char lanHostBandwidthControlEnabel;
	MIB_LAN_HOST_BANDWIDTH_T entry;

	/* check if this function is on */
	mib_get(MIB_LANHOST_BANDWIDTH_CONTROL_ENABLE, &lanHostBandwidthControlEnabel);
	if(lanHostBandwidthControlEnabel == 0){
		printf("Func is off.\n");
		return	-3;
	}
	/* check input first */
	if( (isMacAddrInvalid(pMacAddr)) ||(pUsBandWidth == NULL) || (pDsBandWidth == NULL) )
		return -1;
	
	if( ( totalNum = mib_chain_total(MIB_LAN_HOST_BANDWIDTH_TBL) ) <= 0 )
	{
		printf("Lan host mib table is empty.\n");
		return -2;
	}
	
	memset(&entry, 0, sizeof(MIB_LAN_HOST_BANDWIDTH_T));
	for(i=0; i<totalNum; i++)
	{
		if(!mib_chain_get(MIB_LAN_HOST_BANDWIDTH_TBL,i, (void*)&entry))
			continue;
		
		if( !macAddrCmp(pMacAddr, entry.mac))
			break;
	}

	if( i==totalNum )
	{
		return -2;
	}
	/* Get usBandwidth and dsBandwidth by macAddr */
	*pUsBandWidth = entry.maxUsBandwidth;
	*pDsBandWidth = entry.maxDsBandwidth;
	
	return 0;
}
#endif

/*
 * FUNCTION:SET_ATTCH_DEVICE_NAME
 *
 * INPUT:
 *	macAddr: 
 *		target device's MAC address
 *	pDevName:
 *		devName to set
 *	
 * RETURN:
 *	0 -- successful
 *	-1 -- Input error
 *  -2 -- device not found
 *  -3 -- other
 */
#ifdef CONFIG_USER_LANNETINFO
int set_attach_device_name(unsigned char *pMacAddr, char *pDevName)
{
	int i, totalNum;
	MIB_LAN_DEV_NAME_T entry;
	
	if( (pMacAddr==NULL)||(pDevName==NULL) )
		return -1;
	
	if( ( totalNum = mib_chain_total(MIB_LAN_DEV_NAME_TBL) ) <= 0 )
	{
		printf("Lan host mib table is empty.\n");
		goto add;
	}
	
	memset(&entry, 0, sizeof(MIB_LAN_DEV_NAME_T));
	for(i=0; i<totalNum; i++)
	{
		if(!mib_chain_get(MIB_LAN_DEV_NAME_TBL,i, (void*)&entry))
			continue;
		
		if( !macAddrCmp(pMacAddr, entry.mac))
			break;
	}


	if( i!=totalNum )
	{
		strncpy(entry.devName, pDevName, MAX_LANNET_DEV_NAME_LENGTH-1);
		entry.devName[MAX_LANNET_DEV_NAME_LENGTH-1] = 0;
		if(!mib_chain_update(MIB_LAN_DEV_NAME_TBL,(void*)&entry, i))
			return -3;
		goto end;
	}
	
add:
	memcpy(entry.mac, pMacAddr, MAC_ADDR_LEN);
	strncpy(entry.devName, pDevName, MAX_LANNET_DEV_NAME_LENGTH-1);
	entry.devName[MAX_LANNET_DEV_NAME_LENGTH-1] = 0;
	if(!mib_chain_add(MIB_LAN_DEV_NAME_TBL,(void*)&entry))
		return -3;
end:
	return 0;
}

int get_attach_device_name(unsigned char *pMacAddr, char *pDevName)
{
	int i, totalNum;
	MIB_LAN_DEV_NAME_T entry;

	if( (pMacAddr==NULL)||(pDevName==NULL) )
		return -1;
		
	if( ( totalNum = mib_chain_total(MIB_LAN_DEV_NAME_TBL) ) <= 0 )
	{
		printf("Lan host mib table is empty.\n");
		return -2;
	}
		
	memset(&entry, 0, sizeof(MIB_LAN_DEV_NAME_T));
	for(i=0; i<totalNum; i++)
	{
		if(!mib_chain_get(MIB_LAN_DEV_NAME_TBL,i, (void*)&entry))
			continue;
			
		if( !macAddrCmp(pMacAddr, entry.mac))
			break;
	}

	if(i!=totalNum)
	{// find 
		strncpy(pDevName, entry.devName, MAX_LANNET_DEV_NAME_LENGTH-1);
		pDevName[MAX_LANNET_DEV_NAME_LENGTH-1] = 0;
		return 0;
	}

	return -2;
}

int get_hg_dev_name(hgDevInfo_t **ppHGDevData, unsigned int *pCount)
{
	int32 ret, i, size=0;
	lanHostInfo_t *pLANHostData = NULL;
	hgDevInfo_t *pHGDevData =NULL;

	if((ppHGDevData==NULL)||(pCount == NULL))
		return -1;
	
	/* should get lan Client table */
	ret = get_lan_net_info(&pLANHostData, pCount);
	if(ret)
		return ret;
	++(*pCount);
	
	size = (*pCount) * sizeof(hgDevInfo_t);	
	pHGDevData = (hgDevInfo_t *)malloc( size );
	if(pHGDevData==NULL)
		return -3;
	*ppHGDevData = pHGDevData;
	
	memset(pHGDevData, 0, size);
	/* get gateway MacAddr */
	if ( !mib_get(MIB_ELAN_MAC_ADDR, (void *)pHGDevData[0].mac))
		return -3;

	/* get gateway DevName */
	strcpy(pHGDevData[0].devName, DEVICE_MODEL_STR);

	/* get all lan dev name */
	for(i=1; i<(*pCount);i++)
	{
			memcpy(pHGDevData[i].mac, pLANHostData[i-1].mac, MAC_ADDR_LEN);
			strncpy(pHGDevData[i].devName, pLANHostData[i-1].devName, MAX_LANNET_DEV_NAME_LENGTH-1);
			pHGDevData[i].devName[MAX_LANNET_DEV_NAME_LENGTH-1]=0;
			
			get_attach_device_name(pHGDevData[i].mac, pHGDevData[i].devName);
	}

	free(pLANHostData);
	return 0;
}

/****************************************************************************/
#endif



#ifdef CONFIG_USB_SUPPORT
int getUsbDeviceLabel(const char *device, char *label, char *type)
{
        FILE *fp;
        char command[100], buff[256], tmpBuf[256], *pStr;
        int i;

        snprintf(command, 100, "/bin/blkid %s > /tmp/blkid_file", device);
        system(command);


        if ((fp = fopen("/tmp/blkid_file", "r")) == NULL) {
                printf("open /tmp/blkid_file failed.\n");
                return 0;
        }
        else {
                if ( fgets(buff, sizeof(buff), fp) != NULL )
                {
                        memcpy(tmpBuf, buff, 256);
                        if ((pStr = strstr(tmpBuf, "LABEL=")) != NULL) {
                                pStr += strlen("LABEL=");

                                /* LABEL="LISI" */
                                pStr++;
                                i = 0;
                                while (pStr[i] != '"')
                                        i++;
                                pStr[i] = '\0';

                                strcpy(label, pStr);
                        }
                        else
                                strcpy(label, "Unknown");

                        memcpy(tmpBuf, buff, 256);
                        if ((pStr = strstr(tmpBuf, "TYPE=")) != NULL) {
                                pStr += strlen("TYPE=");

                                pStr++;
                                i=0;
                                while (pStr[i] != '"')
                                        i++;
                                pStr[i] = '\0';
                                strcpy(type, pStr);
                        }
                        else
                                strcpy(type, "Unknown");
                }

                fclose(fp);
                unlink("/tmp/blkid_file");
        }
        return 1;
}
#if 0
static int usb_filter(const struct dirent *dirent)
{
        char *name = dirent->d_name;
        struct stat statbuff;
        char path[32];

        if((strlen(name) == 4) && (name[0] == 's') && (name[1] == 'd')
           && (name[2] >= 'a') && (name[2] <= 'z')
           && (name[3] >= '0') && (name[3] <= '9'))
                return 1;
        else if((strlen(name) == 3) &&(name[0] == 's') && (name[1] == 'r')
                && (name[2] >= '0') && (name[2] <= '9'))
                return 1;
        else if ((strlen(name) == 3) &&(name[0] == 's') && (name[1] == 'd')
                && (name[2] >= 'a') && (name[2] <= 'z')) {
                sprintf(path, "/mnt/%s", name);
                if (stat(path, (struct stat *)&statbuff) == -1)
                        return 0;
                else if (0 == statbuff.st_blocks)
                        return 0;
                return 1;
        }

        return 0;
}
#endif
int umountUSBDevie(void)
{
        int errcode = 1;
        struct dirent **namelist;
        char device[100];
        int i, n;

        n = scandir("/mnt", &namelist, usb_filter, alphasort);

        if (n <= 0)
                return 1;

        for (i = 0; i < n; i++)
        {
                if (namelist[i]->d_name[0] == 's')
                {
                        snprintf(device, 100, "/dev/%s", namelist[i]->d_name);
                        va_cmd("/bin/umount", 1, 1, (char *)device);
                }

                free(namelist[i]);
        }
        free(namelist);

        return 1;
}


void getUSBDeviceInfo(int *disk_sum, struct usb_info * disk1,struct usb_info * disk2)
{
        struct dirent **namelist;
        char tmpLabel[100], tmpFSType[10];
        char device[2][20], type[2][100], mounton[2][512];
        int  usb_storage_num[2];
        char usb_storage_dev[2][10];
        char usb_storage_serial[2][100];
        char invert=0;
        char tmpstr1[100], tmpstr2[100];
        char usb1_serial[100], usb2_serial[100];
        char usb1_dev[10], usb2_dev[10];
        char cmd[100];
        char line[512] = {0};
        FILE *pf = NULL;
        int i, n, diskSum=0;
        unsigned long used[2],avail[2];

        n = scandir("/mnt", &namelist, usb_filter, alphasort);
        memset(type, 0, sizeof(type));
        memset(mounton,0,sizeof(mounton));
        memset(used,0,sizeof(used));
        memset(avail,0,sizeof(avail));

        /* no match */
        if (n > 0)
        {
                for (i=0; i<n; i++) {
                        if ((namelist[i]->d_name[0] == 's') && (diskSum < 2)) {
                                if (strlen(namelist[i]->d_name) == 3)
                                        namelist[i]->d_name[3] = '\0';
                                else
                                        namelist[i]->d_name[4] = '\0';
                                snprintf(device[diskSum], 20, "/dev/%s", namelist[i]->d_name);
                                memset(tmpLabel, 0, sizeof(tmpLabel));
                                memset(tmpFSType, 0, sizeof(tmpFSType));
                                getUsbDeviceLabel(device[diskSum], tmpLabel, tmpFSType);
                                snprintf(type[diskSum], 100, "%s", tmpFSType);
                                snprintf(cmd,sizeof(cmd),"df %s > /tmp/usbinfo",device[diskSum]);
                                system(cmd);
                                pf=fopen("/tmp/usbinfo","r");
                                if(pf)
                                {
                                        fgets(line,sizeof(line),pf);
                                        if(fgets(line,sizeof(line),pf))
                                        {
                                                sscanf(line,"%*s %*d %d %d %*s %s\n",&used[diskSum],&avail[diskSum],mounton[diskSum]);
                                        }
                                        fclose(pf);
                                        unlink("/tmp/usbinfo");
                                }
                                diskSum++;
                        }
                        free(namelist[i]);
                }

                free(namelist);
        }
        *disk_sum = diskSum;

        usb_storage_num[0] = usb_storage_num[1] = -1;
        memset(usb_storage_dev, 0, sizeof(usb_storage_dev));
        memset(usb_storage_serial, 0, sizeof(usb_storage_serial));

        snprintf(cmd, 100, "ls /proc/scsi/usb-storage/  > /tmp/storage.tmp 2>&1");
        system(cmd);

        pf = fopen("/tmp/storage.tmp", "r");
        if(pf) {
                i = 0;
                while(fgets(line, sizeof(line), pf)!=NULL) {
                        //printf("%s:%s", __func__, line);
                        sscanf(line, "%d", &usb_storage_num[i]);

                        i++;
                        if (i >= 2)
                                break;
                }
                //printf("usb_storage_num %d %d\n", usb_storage_num[0], usb_storage_num[1]);
                fclose(pf);
        }

        for (i=0; i<2; i++)
        {
                if (usb_storage_num[i] != -1) {
                        snprintf(cmd, 100, "ls /sys/class/scsi_device/%d:0:0:0/device/block/ > /tmp/storage.tmp 2>&1",
                                        usb_storage_num[i]);
                        system(cmd);

                        pf = fopen("/tmp/storage.tmp", "r");
                        if(pf) {
                                fgets(line, sizeof(line), pf);
                                //printf("%s\n", line);
                                sscanf(line, "%s", usb_storage_dev[i]);
                                fclose(pf);
                        }

                        snprintf(cmd, 100, "cat /proc/scsi/usb-storage/%d > /tmp/storage.tmp 2>&1", usb_storage_num[i]);
                        system(cmd);

                        pf = fopen("/tmp/storage.tmp", "r");
                        if(pf) {
                                while(fgets(line, sizeof(line), pf)!=NULL) {
                                        //printf("%s\n", line);
                                        sscanf(line, "%s %*s %s", tmpstr1, tmpstr2);
                                        if (strstr(tmpstr1, "Serial")) {
                                                strcpy(usb_storage_serial[i], tmpstr2);
                                                //printf("%s:storage_serial[%d]:%s\n", __func__, i, usb_storage_serial[i]);
                                                break;
                                        }
                                }
                                fclose(pf);
                        }
                }
        }

        memset(usb1_serial, 0, sizeof(usb1_serial));
        memset(usb2_serial, 0, sizeof(usb2_serial));
        memset(usb1_dev, 0, sizeof(usb1_dev));
        memset(usb2_dev, 0, sizeof(usb2_dev));

        pf = fopen("/sys/bus/usb/devices/2-1/serial", "r");
        if (!pf)
                pf = fopen("/sys/bus/usb/devices/2-2/serial", "r");
        if (pf) {
                fgets(line, sizeof(line), pf);
                sscanf(line, "%s", usb1_serial);
                //printf("%s:usb2_serial:%s", __func__, usb1_serial);
                fclose(pf);
        }

        pf = fopen("/sys/bus/usb/devices/1-2/serial", "r");
        if (!pf)
                pf = fopen("/sys/bus/usb/devices/1-1/serial", "r");
        if (pf) {
                fgets(line, sizeof(line), pf);
                sscanf(line, "%s", usb2_serial);
                //printf("%s:usb1_serial:%s\n", __func__, usb2_serial);
                fclose(pf);
        }

        unlink("/tmp/storage.tmp");

        if (usb1_serial[0]) {//usb1 running(USB2.0)
                for (i=0; i<2; i++) {
                        if (usb_storage_serial[i][0] && !strcmp(usb_storage_serial[i], usb1_serial)) {
                                strcpy(usb1_dev, usb_storage_dev[i]);
                                break;
                        }
                }
                //debug
                if (i >= 2)
                        printf("unknown usb1 dev name\n");
                else {
                        if (strstr(device[0], usb1_dev))
                                invert = 0;
                        else if (strstr(device[1], usb1_dev))
                                invert = 1;
                }
        }

        if (usb2_serial[0]) {//usb2 running(USB3.0)
                for (i=0; i<2; i++) {
                        if (usb_storage_serial[i][0] && !strcmp(usb_storage_serial[i], usb2_serial)) {
                                strcpy(usb2_dev, usb_storage_dev[i]);
                                break;
                        }
                }
                //debug
                if (i >= 2)
                        printf("unknown usb2 dev name\n");
                else {
                        if (strstr(device[0], usb2_dev))
                                invert = 1;
                        else if (strstr(device[1], usb2_dev))
                                invert = 0;
                }
        }

        if(usb1_dev[0])
        {
                strcpy(disk1->disk_type, usb1_dev);
                strcpy(disk1->disk_status, "Mounted");
                if(invert)
                {
                        sprintf(disk1->disk_fs, "%s", type[1]);
                        disk1->disk_used=used[1];
                        disk1->disk_available=avail[1];
                        strcpy(disk1->disk_mounton, mounton[1]);
                }
                else
                {
                        sprintf(disk1->disk_fs, "%s", type[0]);
                        disk1->disk_used=used[0];
                        disk1->disk_available=avail[0];
                        strcpy(disk1->disk_mounton,  mounton[0]);
                }
        }
        else
        {
                strcpy(disk1->disk_type, "No Device");
                strcpy(disk1->disk_status, "Disconnect");
                strcpy(disk1->disk_fs,"");
                strcpy(disk1->disk_mounton,"");
                disk1->disk_used=0;
                disk1->disk_available=0;
        }

        if(usb2_dev[0])
        {
                strcpy(disk2->disk_type, usb2_dev);
                strcpy(disk2->disk_status, "Mounted");
                if(invert)
                {
                        sprintf(disk2->disk_fs, "%s", type[0]);
                        disk2->disk_used=used[0];
                        disk2->disk_available=avail[0];
                        strcpy(disk2->disk_mounton, mounton[0]);
                }
                else
                {
                        sprintf(disk2->disk_fs, "%s", type[1]);
                        disk2->disk_used=used[1];
                        disk2->disk_available=avail[1];
                        strcpy(disk2->disk_mounton, mounton[1]);
                }
        }
        else
        {
                strcpy(disk2->disk_type, "No Device");
                strcpy(disk2->disk_status, "Disconnect");
                strcpy(disk2->disk_fs,"");
                strcpy(disk2->disk_mounton,"");
                disk2->disk_used=0;
                disk2->disk_available=0;
        }

}
#endif//end of CONFIG_USB_SUPPORT

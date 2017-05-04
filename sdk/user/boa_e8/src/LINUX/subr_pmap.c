/*
 *      System routines for Port-mapping
 *
 */

#include <string.h>
#include "debug.h"
#include "utility.h"
#include "wireless.h"
#ifdef CONFIG_RTK_RG_INIT
#include "rtusr_rg_api.h"
#endif
void attach_mark_to_vlanpck();
void reset_vlanmap();
int vlanmap_addpair(const char *ifname, unsigned short lVid, unsigned short wVid);
static int convert_mark_to_string(unsigned int mark, char *string, uint32_t num);
int vlanmap_deldev(const char *ifname);


#define MAX_IP_RULE_NUM 36
typedef struct _tag_ip_rule_set{
uint32_t num;
uint32_t fwmark[MAX_IP_RULE_NUM];
}IP_RULE_SET;

/*
 * The bitmap for interface should follow this convention commonly shared
 * by sar driver (sar_send())
 * Bit-map:
 *  bit16|bit15|bit14|bit13|bit12|(bit11)|bit10|bit9|bit8|bit7|bit6 |bit5 |bit4  |bit3|bit2|bit1|bit0
 *  vap3 |vap2 |vap1 |vap0 |wlan1|(usb0) |vap3 |vap2|vap1|vap0|wlan0|resvd|device|lan3|lan2|lan1|lan0
 * If usb(switch port) add:
 *  bit16|bit15|bit14|bit13|bit12|(bit11)|bit10|bit9|bit8|bit7|bit6 |bit5  |bit4     |bit3|bit2|bit1|bit0
 *  vap3 |vap2 |vap1 |vap0 |wlan1|(usb0) |vap3 |vap2|vap1|vap0|wlan0|device|lan4(usb)|lan3|lan2|lan1|lan0
 */

/* =============================================================================================================*/


#ifdef NEW_PORTMAPPING
//#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define RESET_DECONFIG_SCRIPT()	\
	do {	\
		pmap_fid=open("/var/tmp/pmap_deconfig", O_RDWR|O_CREAT|O_TRUNC, S_IXUSR);	\
		write_to_dconfig(1, "#!/bin/sh\n");	\
	} while (0)
#define CLOSE_DECONFIG_SCRIPT()	close(pmap_fid)
const char PMAP_DECONFIG_SCRIPT[] = "/var/tmp/pmap_deconfig";
int pmap_fid;

#ifdef CONFIG_IPV6
/** 
 * We need priority value because "ip -6 rule" 
 * always use 16383 priority of new rules.
 */
int ip6_priority_i = 32764;
char ip6_priority[6] = "32764";
#endif

struct pmap_s pmap_list[MAX_VC_NUM];

/*
 *	Get the port-mapping bitmap (fgroup) for bridged WAN.
 *	-1	failed
 *	0	successful
 */
int get_pmap_fgroup(struct pmap_s *pmap_p, int num)
{
	unsigned int total;
	unsigned short final_itfgroup;
	int i, k;
	MIB_CE_ATM_VC_T Entry;

	memset(pmap_p, 0, sizeof(struct pmap_s)*num);
	total = mib_chain_total(MIB_ATM_VC_TBL);
	if (total > num)
		return -1;

	for(i = 0; i < total; i++) {
		mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&Entry);
		if (!Entry.enable || !isValidMedia(Entry.ifIndex))
			continue;
		pmap_p[i].ifIndex = Entry.ifIndex;
		pmap_p[i].valid = 1;
		pmap_p[i].applicationtype = Entry.applicationtype;
		pmap_p[i].itfGroup = Entry.itfGroup;
	}

	// Get fgroup
	for(i = 0; i < total; i++) {
		if (pmap_p[i].applicationtype&X_CT_SRV_INTERNET) {
			pmap_p[i].fgroup = 0x01ff; // all

			for (k=0; k<total; k++) {
				if (k==i) // ignore itself
					continue;
				// strip mapped ports
AUG_PRT("strip mapped ports i:%d k:%d itfGroup=0x%x\n",i,k,pmap_p[k].itfGroup);
				pmap_p[i].fgroup &= (~(pmap_p[k].itfGroup));
			}
		}
		else if (pmap_p[i].applicationtype&X_CT_SRV_OTHER) {
			pmap_p[i].fgroup = pmap_p[i].itfGroup;
		}
		else
			pmap_p[i].fgroup = 0;
	}

	return 0;
}

/*
 *	Resolve the overlapped ports.
 */
int check_itfGroup(MIB_CE_ATM_VC_Tp pEntry, MIB_CE_ATM_VC_Tp pOldEntry)
{
	uint32_t i;
	uint16_t cur_itfgroup;
	int32_t totalEntry;
	MIB_CE_ATM_VC_T temp_Entry;
#ifdef CONFIG_RTK_RG_INIT
	return 0;
#endif
	AUG_PRT("the pEntry->vlan, pEntry->vid : %d\n", pEntry->vlan, pEntry->vid);

	cur_itfgroup = pEntry->itfGroup;

	if (pEntry->applicationtype&X_CT_SRV_INTERNET) {
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		for(i = 0; i < totalEntry; ++i)
		{
			if(mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&temp_Entry))
			{
				uint16_t sameport;

				if(pOldEntry && pOldEntry->ifIndex == temp_Entry.ifIndex)
					continue;

				sameport = (uint16_t)(cur_itfgroup & temp_Entry.itfGroup);
				if(sameport) { // strip overlapped ports
					temp_Entry.itfGroup &= (uint16_t)(~(sameport));
					mib_chain_update(MIB_ATM_VC_TBL, (void*)&temp_Entry, i);
				}

			}//end if(mib_chain_get)
			else //something wrong
				return -1;
		}//end for()
	}
	else if (pEntry->applicationtype&X_CT_SRV_OTHER) {
		if(cur_itfgroup)
		{
			totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
			for(i = 0; i < totalEntry; ++i)
			{
				if(mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&temp_Entry))
				{
					uint16_t sameport;

					if(pOldEntry && pOldEntry->ifIndex == temp_Entry.ifIndex)
						continue;

					sameport = (uint16_t)(cur_itfgroup & temp_Entry.itfGroup);
					if(sameport) { // strip overlapped ports
						temp_Entry.itfGroup &=  (uint16_t)(~(sameport));
						mib_chain_update(MIB_ATM_VC_TBL, (void*)&temp_Entry, i);
					}

				}
				else
				{
					//something wrong
					return -1;
				}

			}//end for()
		}
	}

	return 0;
}

/*
 *	deconfig script of port-mapping.
 *	Contains rules to delete all ip rules for port-mapping.
 *	This script should be called before reconfigure port-mapping.
 */
int write_to_dconfig(int num, ...)
{
	va_list ap;
	int k;
	char *s;
	char buf[80];

	va_start(ap, num);

	buf[0] = '\0';
	for (k=0; k<num; k++) {
		s = va_arg(ap, char *);
		if (strlen(buf))
			sprintf(buf, "%s %s", buf, s);
		else
			sprintf(buf, "%s", s);
	}
	sprintf(buf, "%s\n", buf);
	write(pmap_fid, buf, strlen(buf));
	return 0;
}

//the name of wlan
//const char*  wlan[]   = {ALIASNAME_WLAN0,ALIASNAME_WLAN0_VAP0, ALIASNAME_WLAN0_VAP1, ALIASNAME_WLAN0_VAP2, ALIASNAME_WLAN0_VAP3, ""};
//const char*  wlan[]   = {"wlan0", "wlan0_vap0", "wlan0_vap1", "wlan0_vap2", "wlan0_vap3", ""};


/*
//pol_mark[PMAP_ETH0_SW0] = "0x1"  which means:  mark the eth0_sw0 as 0x100000/0xf00000
const char*  pol_mark[] = {"0x100000/0xf00000", "0x200000/0xf00000",
													"0x300000/0xf00000",
													"0x400000/0xf00000",
													"0x500000/0xf00000",
													"0x600000/0xf00000",
													"0x700000/0xf00000",
													"0x800000/0xf00000",
													"0x900000/0xf00000", ""};
*/

const char  tonull[] = "> /dev/null";

#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)

#define BROUTE_CHAIN_PREFIX "broute_chain_"

char broute_chain_nic[5][20] = { BROUTE_CHAIN_PREFIX, BROUTE_CHAIN_PREFIX,
									BROUTE_CHAIN_PREFIX, BROUTE_CHAIN_PREFIX, ""};

char broute_chain_wlan[6][32] = {BROUTE_CHAIN_PREFIX, BROUTE_CHAIN_PREFIX,
									BROUTE_CHAIN_PREFIX, BROUTE_CHAIN_PREFIX, BROUTE_CHAIN_PREFIX, ""};
#endif


//the const strings
const char   BIN_IP[] = "/bin/ip";
const char 	 LANIF_MARK[] = "lan_mark_in_mangle";
const char	 PORTMAP_EBTBL[] = "portmapping";
const char   LANIF_BROUTE[] = "lan_mark_in_broute";


//pol_mark[PMAP_ETH0_SW0] = "0x1"  which means:  mark the eth0_sw0 as 0x1000000/0xff000000
const char*  pol_mark[] = {"0x1000000/0xff000000", "0x2000000/0xff000000",
													"0x3000000/0xff000000",
													"0x4000000/0xff000000",
													"0x5000000/0xff000000",
													"0x6000000/0xff000000",
													"0x7000000/0xff000000",
													"0x8000000/0xff000000",
													"0x9000000/0xff000000",
													"0xa000000/0xff000000",
													"0xb000000/0xff000000",
													"0xc000000/0xff000000",
													"0xd000000/0xff000000",
													"0xe000000/0xff000000", ""};

// change the digit to the string
// 1 ===> "1"
static int aug_itoa(int digital, char* string, uint32_t num)
{
	int rt;

	if(NULL  == string)
		return -1;

	if(0 == num || num > 10)
		return -2;

	rt = snprintf(string, num, "%d", digital);

	return rt;
}


/*
vc0 return 1, vc1 return 2 ...
ppp0 return 0x10, ppp1 return 0x11 ...
nas0_0 return 0x20, nas0_1 return 0x20, ...
ppp0_over_nas0 return 0x30, ...
*/
#define PMAP_VC_START	1
#define PMAP_PPP_START	0x10
#define PMAP_NAS_START 0x20
#define PMAP_NAS_PPP_START 0x30
#ifdef CONFIG_PTMWAN
#define PMAP_PTM_START 0x40
#define PMAP_PTM_PPP_START 0x50
#endif /*CONFIG_PTMWAN*/

//#define CHECK_NAS_IDX(a) (a >> 16)
//#define DUMMY_NAS_PPP_INDEX 0x1ff

int caculate_tblid(uint32_t ifid)
{
	int tbl_id;

	uint32_t ifindex;

	ifindex = ifid;

	//ifindex of nas0_* is 0x1ff01, 0x1ff02
	//ifindex of ppp* over nas0 is 0x10000, 0x10001
#ifdef CONFIG_ETHWAN
	//if(CHECK_NAS_IDX(ifindex))
	if( MEDIA_INDEX(ifindex)==MEDIA_ETH )
	{
		if(PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
			tbl_id = ETH_INDEX(ifindex) + PMAP_NAS_START;
		else
			tbl_id = PPP_INDEX(ifindex) + PMAP_NAS_PPP_START;
	}
	else
#endif /*CONFIG_ETHWAN*/
#ifdef CONFIG_PTMWAN
	if( MEDIA_INDEX(ifindex)==MEDIA_PTM )
	{
		if(PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
			tbl_id = PTM_INDEX(ifindex) + PMAP_PTM_START;
		else
			tbl_id = PPP_INDEX(ifindex) + PMAP_PTM_PPP_START;
	}
	else
#endif /*CONFIG_PTMWAN*/
	{
	//ifindex of vc* is 0xff01, 0xff02, ...
	//ifindex of ppp* is 0x00, 0x0101, 0x0202 ...
	if (PPP_INDEX(ifindex) == DUMMY_PPP_INDEX)
		tbl_id = VC_INDEX(ifindex) + PMAP_VC_START;
	else
		tbl_id = PPP_INDEX(ifindex) + PMAP_PPP_START;
	}

	return tbl_id;
}

static void set_ebtbl(MIB_CE_ATM_VC_Tp pEntry)
{
	uint16_t fgroup;
	unsigned int ifIndex;

	char str_tblid[10];
	char ifname[IFNAMSIZ];

	int32_t j;
	int32_t tbl_id;
	uint16_t itfGroup;
	int idx;

	get_pmap_fgroup(pmap_list, MAX_VC_NUM);
	idx = -1;
	for (j=0; j<MAX_VC_NUM; j++) {
		if (pmap_list[j].valid && pmap_list[j].ifIndex == pEntry->ifIndex) {
			idx = j;
			break;
		}
	}
	if (idx < 0)
		return;
	itfGroup	= pEntry->itfGroup;
	fgroup		= pmap_list[idx].fgroup;
	ifIndex		= pEntry->ifIndex;

	ifGetName(ifIndex, ifname, sizeof(ifname));

	for(j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < SW_LAN_PORT_NUM; ++j)
	{
		if(BIT_IS_SET(fgroup, j))
		{
			va_cmd(EBTABLES, 9, 1, "-I", PORTMAP_EBTBL, "1",
								"-i", SW_LAN_PORT_IF[j],
								"-o", ifname,
								"-j", "RETURN");
			va_cmd(EBTABLES, 9, 1, "-I", PORTMAP_EBTBL, "1",
								"-i", ifname,
								"-o", SW_LAN_PORT_IF[j],
								"-j", "RETURN");

			// Drop DHCP requests from bounded LAN
			if(pEntry->disableLanDhcp)
				va_cmd(EBTABLES, 6, 1, FW_ADD, FW_DHCPS_DIS, "-i", SW_LAN_PORT_IF[j], "-j", FW_DROP);

			//if one lanport is bound to bridge pvc, it should NOT be routed!
			if(BIT_IS_SET(itfGroup, j))
			{
				va_cmd(BIN_IP, 5, 1, "rule", "add", "fwmark", pol_mark[j], "prohibit");
				write_to_dconfig(6, BIN_IP, "rule", "del", "fwmark", pol_mark[j], "prohibit");
#ifdef CONFIG_IPV6
				va_cmd(BIN_IP, 8, 1, "-6", "rule", "add", "priority", ip6_priority, "fwmark", pol_mark[j], "prohibit");
				write_to_dconfig(9, BIN_IP, "-6", "rule", "del", "priority", ip6_priority, "fwmark", pol_mark[j], "prohibit");
				snprintf(ip6_priority, 6, "%d", --ip6_priority_i);
#endif

			}
		}
	}
#ifdef WLAN_SUPPORT
	for(j = PMAP_WLAN0; j <= PMAP_WLAN1_VAP3; ++j)
	{
	    if(BIT_IS_SET(fgroup, j))
	    {
			if(wlan_en[j-PMAP_WLAN0] == 0)
				continue;

			va_cmd(EBTABLES, 9, 1, "-I", PORTMAP_EBTBL, "1",
								"-i", wlan[j - PMAP_WLAN0],
								"-o", ifname,
								"-j", "RETURN");
			va_cmd(EBTABLES, 9, 1, "-I", PORTMAP_EBTBL, "1",
								"-i", ifname,
								"-o", wlan[j - PMAP_WLAN0],
								"-j", "RETURN");

			// Drop DHCP requests from bounded LAN
			if(pEntry->disableLanDhcp)
				va_cmd(EBTABLES, 6, 1, FW_ADD, FW_DHCPS_DIS, "-i", wlan[j - PMAP_WLAN0], "-j", FW_DROP);

			//if one lanport is bound to bridge pvc, it should NOT be routed!
			if(BIT_IS_SET(itfGroup, j))
			{
				va_cmd(BIN_IP, 5, 1, "rule", "add", "fwmark", pol_mark[j], "prohibit");
				write_to_dconfig(6, BIN_IP, "rule", "del", "fwmark", pol_mark[j], "prohibit");
#ifdef CONFIG_IPV6
				va_cmd(BIN_IP, 8, 1, "-6", "rule", "add", "priority", ip6_priority, "fwmark", pol_mark[j], "prohibit");
				write_to_dconfig(9, BIN_IP, "-6", "rule", "del", "priority", ip6_priority, "fwmark", pol_mark[j], "prohibit");
				snprintf(ip6_priority, 6, "%d", --ip6_priority_i);
#endif

			}
		}
	}
#endif
}

//august: this func is not perfect! >_<
static int getNetAddrwithMask(MIB_CE_ATM_VC_Tp pEntry, char* pAddr, int length)
{
	//august: I am so lazy that the length  won't be checked!
	//so the length is useless,

	int netmask_bits;

	uint32_t _in_netmask;
	uint32_t _ipaddr;

	struct in_addr in_net_addr;

	char tmp[8];

	int j;

	if(length < 20)
		return -1;

	_in_netmask = (*((struct in_addr *)pEntry->netMask)).s_addr;
	_ipaddr = (*((struct in_addr *)pEntry->ipAddr)).s_addr;

	in_net_addr.s_addr = _ipaddr & _in_netmask;

	strcpy(pAddr, (const char *)inet_ntoa(in_net_addr));

	for(j = 0; j < 25; ++j)
	{
		if((_in_netmask & 0x1) == 0)
		{
			_in_netmask = _in_netmask >> 1;
		}
		else
			break;
	}

	netmask_bits = 32 - j;

	sprintf(tmp, "/%d", netmask_bits);

	strcat(pAddr, tmp);

	return 0;

}

//do the command of policy routing which is "ip route " and "ip rule"
static int set_policyrt(MIB_CE_ATM_VC_Tp pEntry)
{
	uint16_t itfGroup;
	uint32_t ifIndex;

	int32_t tbl_id;
        int iFlags;

	char ifname[IFNAMSIZ], devname[IFNAMSIZ];
	char str_tblid[10], str_dft_tblid[8];
	char ipAddr[16];

	char netAddr[20];

	int32_t j;

	itfGroup = pEntry->itfGroup;

	ifIndex = pEntry->ifIndex;

	ifGetName(ifIndex, ifname, sizeof(ifname));

	//ifindex ==>  table index
	//if vc0's  ifindex is 0xff00 ==> table 0x1
	//if ppp0's ifindex is 0xff01 ==> table 0x11
	tbl_id = caculate_tblid(ifIndex);

	aug_itoa(tbl_id, str_tblid, sizeof(str_tblid));
	aug_itoa(PMAP_DEFAULT_TBLID, str_dft_tblid, sizeof(str_dft_tblid));

	AUG_PRT("the itfGroup is 0x%x. ifIndex is 0x%x. ifname is %s\n", itfGroup, ifIndex, ifname);

	// Kaohj -- We should have default route to restrict to port-mapping even if not connected.
	// 	Be noted that default route for dhcped and ppp interfaces must be set
	//	dynamically whenever interface is up
	if (pEntry->cmode == CHANNEL_MODE_IPOE) {
		if ((pEntry->IpProtocol == IPVER_IPV6)&& (pEntry->AddrMode == IPV6_WAN_DSLITE)) { 
			va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", "tun0", "table", str_tblid);
			if (pEntry->applicationtype&X_CT_SRV_INTERNET) 
				va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", "tun0", "table", str_dft_tblid);
		}
		else if (pEntry->ipDhcp == DHCP_DISABLED) { // static MER
			strcpy(ipAddr, (const char *)inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr)));
			va_cmd(BIN_IP, 9, 1, "route", "add", "default", "via", ipAddr, "dev", ifname, "table", str_tblid);

			getNetAddrwithMask(pEntry, (char *)netAddr, 20);

			va_cmd(BIN_IP, 7, 1, "route", "add", (char *)netAddr, "dev", ifname, "table", str_tblid);

			if (pEntry->applicationtype&X_CT_SRV_INTERNET) {
				va_cmd(BIN_IP, 9, 1, "route", "add", "default", "via", ipAddr, "dev", ifname, "table", str_dft_tblid);
				va_cmd(BIN_IP, 7, 1, "route", "add", (char *)netAddr, "dev", ifname, "table", str_dft_tblid);
			}
		}
		else { // dhcped MER
			va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", ifname, "table", str_tblid);
			if (pEntry->applicationtype&X_CT_SRV_INTERNET)
				va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", ifname, "table", str_dft_tblid);
		}
	}
	else { // PPP
		if (getInFlags(ifname, &iFlags) == 1 && (iFlags & IFF_UP)) {
			va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", ifname, "table", str_tblid);
			if (pEntry->applicationtype&X_CT_SRV_INTERNET)
				va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", ifname, "table", str_dft_tblid);
		}
		else {
			ifGetName(PHY_INTF(ifIndex), devname, sizeof(devname));
			va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", devname, "table", str_tblid);
			if (pEntry->applicationtype&X_CT_SRV_INTERNET)
				va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", devname, "table", str_dft_tblid);
		}
	}

	for(j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < SW_LAN_PORT_NUM; ++j)
	{
		if(BIT_IS_SET(itfGroup, j))
		{
			va_cmd(BIN_IP, 6, 1, "rule", "add", "fwmark", pol_mark[j],
									"table", str_tblid);
			write_to_dconfig(7, BIN_IP, "rule", "del", "fwmark", pol_mark[j],
									"table", str_tblid);
		}

	}
#ifdef WLAN_SUPPORT
	for(j = PMAP_WLAN0; j <= PMAP_WLAN1_VAP3; ++j)	
	{
	    if(BIT_IS_SET(itfGroup, j))
	    {
			if(wlan_en[j-PMAP_WLAN0]==0)
				continue;
			
			va_cmd(BIN_IP, 6, 1, "rule", "add", "fwmark", pol_mark[j],
									"table", str_tblid);
			write_to_dconfig(7, BIN_IP, "rule", "del", "fwmark", pol_mark[j],
									"table", str_tblid);
		}
	}
#endif
}

#ifdef CONFIG_IPV6
//do the command of policy routing which is "ip -6 route " and "ip -6 rule"
static int set_ip6_policyrt(MIB_CE_ATM_VC_Tp pEntry)
{
	uint16_t itfGroup;
	uint32_t ifIndex;

	int32_t tbl_id;
	int iFlags;

	char ifname[IFNAMSIZ], devname[IFNAMSIZ];
	char str_tblid[10], str_dft_tblid[8];
	char ipAddr[MAX_V6_IP_LEN];
	char str_prefix[MAX_V6_IP_LEN];
	char netAddr[45] = {0};
	struct in6_addr prefix = {0};

	int32_t j;

	if(!(pEntry->IpProtocol & IPVER_IPV6))
		return 0;	//ipv4 only

	itfGroup = pEntry->itfGroup;
	ifIndex = pEntry->ifIndex;

	ifGetName(ifIndex, ifname, sizeof(ifname));

	//ifindex ==>  table index
	//if vc0's  ifindex is 0xff00 ==> table 0x1
	//if ppp0's ifindex is 0xff01 ==> table 0x11
	tbl_id = caculate_tblid(ifIndex);

	aug_itoa(tbl_id, str_tblid, sizeof(str_tblid));
	aug_itoa(PMAP_DEFAULT_TBLID, str_dft_tblid, sizeof(str_dft_tblid));

	AUG_PRT("[IPv6] the itfGroup is 0x%x. ifIndex is 0x%x. ifname is %s\n", itfGroup, ifIndex, ifname);

	// Kaohj -- We should have default route to restrict to port-mapping even if not connected.
	// 	Be noted that default route for dhcped and ppp interfaces must be set
	//	dynamically whenever interface is up
	if (pEntry->cmode == CHANNEL_MODE_IPOE)
	{
		if(pEntry->AddrMode & IPV6_WAN_STATIC)
		{
			inet_ntop(AF_INET6, pEntry->RemoteIpv6Addr, ipAddr, sizeof(ipAddr));
			va_cmd(BIN_IP, 10, 1, "-6", "route", "add", "default", "via", ipAddr, "dev", ifname, "table", str_tblid);

			ip6toPrefix(pEntry->Ipv6Addr, pEntry->Ipv6AddrPrefixLen, &prefix);
			inet_ntop(AF_INET6, &prefix, str_prefix, sizeof(str_prefix));
			snprintf(netAddr, 45, "%s/%d", str_prefix, pEntry->Ipv6AddrPrefixLen);
			va_cmd(BIN_IP, 8, 1, "-6", "route", "add", (char *)netAddr, "dev", ifname, "table", str_tblid);

			if (pEntry->applicationtype&X_CT_SRV_INTERNET)
			{
				va_cmd(BIN_IP, 10, 1, "-6", "route", "add", "default", "via", ipAddr, "dev", ifname, "table", str_dft_tblid);
				va_cmd(BIN_IP, 8, 1, "-6", "route", "add", (char *)netAddr, "dev", ifname, "table", str_dft_tblid);
			}
		}

		if(pEntry->AddrMode & (IPV6_WAN_AUTO | IPV6_WAN_DHCP))
		{
			va_cmd(BIN_IP, 8, 1, "-6", "route", "add", "default", "dev", ifname, "table", str_tblid);
			if (pEntry->applicationtype&X_CT_SRV_INTERNET)
				va_cmd(BIN_IP, 8, 1, "-6", "route", "add", "default", "dev", ifname, "table", str_dft_tblid);
		}
	}
	else
	{
		// PPP
		if (getInFlags(ifname, &iFlags) == 1 && (iFlags & IFF_UP)) {
			va_cmd(BIN_IP, 8, 1, "-6", "route", "add", "default", "dev", ifname, "table", str_tblid);
			if (pEntry->applicationtype&X_CT_SRV_INTERNET)
				va_cmd(BIN_IP, 8, 1, "-6", "route", "add", "default", "dev", ifname, "table", str_dft_tblid);
		}
		else {
			ifGetName(PHY_INTF(ifIndex), devname, sizeof(devname));
			va_cmd(BIN_IP, 8, 1, "-6", "route", "add", "default", "dev", devname, "table", str_tblid);
			if (pEntry->applicationtype&X_CT_SRV_INTERNET)
				va_cmd(BIN_IP, 8, 1, "-6", "route", "add", "default", "dev", devname, "table", str_dft_tblid);
		}
	}

	for(j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < SW_LAN_PORT_NUM; ++j)
	{
		if(BIT_IS_SET(itfGroup, j))
		{
			va_cmd(BIN_IP, 9, 1, "-6", "rule", "add", "priority", ip6_priority, "fwmark", pol_mark[j],
									"table", str_tblid);
			write_to_dconfig(10, BIN_IP, "-6", "rule", "del", "priority", ip6_priority, "fwmark", pol_mark[j],
									"table", str_tblid);
			snprintf(ip6_priority, 6, "%d", --ip6_priority_i);
		}

	}
#ifdef WLAN_SUPPORT
	for(j = PMAP_WLAN0; j <= PMAP_WLAN1_VAP3; ++j)
	{
	    if(BIT_IS_SET(itfGroup, j))
	    {
			if(wlan_en[j-PMAP_WLAN0]==0)
				continue;

			va_cmd(BIN_IP, 9, 1, "-6", "rule", "add", "priority", ip6_priority, "fwmark", pol_mark[j],
									"table", str_tblid);
			write_to_dconfig(10, BIN_IP, "-6", "rule", "del", "priority", ip6_priority, "fwmark", pol_mark[j],
									"table", str_tblid);
			snprintf(ip6_priority, 6, "%d", --ip6_priority_i);
		}
	}
#endif
}
#endif

//execute the port-mapping commands
int exec_portmp()
{
	int32_t totalEntry;
	MIB_CE_ATM_VC_T temp_Entry;
	int iptbl_done_once, ebtbl_done_once;
	int32_t i, j;	
	MIB_CE_PORT_BINDING_T pbEntry;
	
	iptbl_done_once = ebtbl_done_once = 0;

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);

	for(i = 0; i < totalEntry; ++i)
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&temp_Entry))
		{
			if (!temp_Entry.enable)
				continue;			
			
			if((temp_Entry.cmode == CHANNEL_MODE_BRIDGE ||
				temp_Entry.cmode == CHANNEL_MODE_IPOE ||
				temp_Entry.cmode == CHANNEL_MODE_PPPOE) &&
				temp_Entry.brmode != BRIDGE_DISABLE)
			{
				if(!ebtbl_done_once)
				{
					va_cmd(EBTABLES, 8, 1, "-A", PORTMAP_EBTBL, "-i", "eth+", "-o", "eth+", "-j", "RETURN");
					va_cmd(EBTABLES, 8, 1, "-A", PORTMAP_EBTBL, "-i", "eth+", "-o", "wlan+", "-j", "RETURN");
					va_cmd(EBTABLES, 8, 1, "-A", PORTMAP_EBTBL, "-i", "wlan+", "-o", "eth+", "-j", "RETURN");
					va_cmd(EBTABLES, 8, 1, "-A", PORTMAP_EBTBL, "-i", "wlan+", "-o", "wlan+", "-j", "RETURN");

					va_cmd(EBTABLES, 6, 1, "-A", PORTMAP_EBTBL, "-i", "eth0+", "-j", "DROP");
					va_cmd(EBTABLES, 6, 1, "-A", PORTMAP_EBTBL, "-i", "wlan+", "-j", "DROP");

					ebtbl_done_once = 111;
				}
				set_ebtbl(&temp_Entry);
			}

			if(temp_Entry.cmode == CHANNEL_MODE_IPOE || temp_Entry.cmode == CHANNEL_MODE_PPPOE
					|| temp_Entry.cmode == CHANNEL_MODE_RT1483)
			{
				if(!iptbl_done_once)
				{

					for(j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < SW_LAN_PORT_NUM; ++j)
					{						
						mib_chain_get(MIB_PORT_BINDING_TBL, j, (void*)&pbEntry);
						if (pbEntry.pb_mode == PORT_BAESD_MODE)
						{
							va_cmd(IPTABLES, 10, 1, "-t", "mangle", "-A", LANIF_MARK,
										"-i", SW_LAN_PORT_IF[j],
										"-j", "MARK",
										"--set-mark", pol_mark[j]);

#ifdef CONFIG_IPV6
							va_cmd(IP6TABLES, 10, 1, "-t", "mangle", "-A", LANIF_MARK,
										"-i", SW_LAN_PORT_IF[j],
										"-j", "MARK",
										"--set-mark", pol_mark[j]);
#endif
						}
					}
#ifdef WLAN_SUPPORT
					for(j = PMAP_WLAN0; j <= PMAP_WLAN1_VAP3; ++j)
					{
						if(wlan_en[j-PMAP_WLAN0]==0)
							continue;

						mib_chain_get(MIB_PORT_BINDING_TBL, j, (void*)&pbEntry);
						if (pbEntry.pb_mode == PORT_BAESD_MODE)
						{
							va_cmd(IPTABLES, 10, 1, "-t", "mangle", "-A", LANIF_MARK,
										"-i", wlan[j - PMAP_WLAN0],
										"-j", "MARK",
										"--set-mark", pol_mark[j]);
#ifdef CONFIG_IPV6
							va_cmd(IP6TABLES, 10, 1, "-t", "mangle", "-A", LANIF_MARK,
										"-i", SW_LAN_PORT_IF[j],
										"-j", "MARK",
										"--set-mark", pol_mark[j]);
#endif
						}
					}
#endif
					iptbl_done_once = 111;
				}

				set_policyrt(&temp_Entry);
#ifdef CONFIG_IPV6
				set_ip6_policyrt(&temp_Entry);
#endif
			}
		} //endif if(mib_chain...)
	}//end for
	return 0;
}

int set_vc_fgroup()
{
	int i;
	int vcIdx;
	struct data_to_pass_st msg;
	char wanif[16];
	MEDIA_TYPE_T mType;

	get_pmap_fgroup(pmap_list, MAX_VC_NUM);

	for (i=0; i<MAX_VC_NUM; i++) {
		if (!pmap_list[i].valid)
			continue;
		mType = MEDIA_INDEX(pmap_list[i].ifIndex);
		if ( mType == MEDIA_ATM)
		{
			/*august:20120330
			we integrate the ethwan and adsl portmap

			the fgroup and the entry.itfGroup is always the following sequence

			bit 0 ===> frist lan device, e.g. eth0.2
			bit n ===> the num n lan or wlan device, e.g. eth0.(2 + n);
			*/
			unsigned short pass_kernel_fgroup = pmap_list[i].fgroup << 1;

			vcIdx = VC_INDEX(pmap_list[i].ifIndex);
			snprintf(wanif, 16, "vc%d", vcIdx);
			//AUG_PRT("The tmp_Entry.fgroup is 0x%x\n", pass_kernel_fgroup);
			snprintf(msg.data, BUF_SIZE, "mpoactl set %s fgroup %d", wanif, pass_kernel_fgroup);
			//AUG_PRT("%s\n", msg.data);

			write_to_mpoad(&msg);

		}
		else
			continue;
	}
}

void setup_vc_pmap_lanmember(int vcIndex, unsigned short fgroup)
{
	char wanif[16];
	int vcIdx;
	struct data_to_pass_st msg;

	vcIdx = VC_INDEX(vcIndex);
	snprintf(wanif, 16, "vc%d", vcIdx);

	snprintf(msg.data, BUF_SIZE, "mpoactl set %s fgroup %d", wanif, fgroup);

	AUG_PRT("%s \n", msg.data);

	write_to_mpoad(&msg);
}

void setup_wan_pmap_lanmember(MEDIA_TYPE_T mType, unsigned int Index)
{
	int i;

	get_pmap_fgroup(pmap_list, MAX_VC_NUM);

	for(i=0; i<MAX_VC_NUM; i++)
	{
		if(pmap_list[i].ifIndex != Index)
			continue;

		if (mType == MEDIA_ETH && (WAN_MODE & MODE_Ethernet))
		{
			int tmp_group;
#ifdef CONFIG_HWNAT
			// bit-0 used for wan in driver
			tmp_group = ((pmap_list[i].fgroup)<<1)|0x1;
			//AUG_PRT("set intf:%x   member:%x!\n", Index, tmp_group);
			setup_hwnat_eth_member(Index, tmp_group, 1);
#endif
		}
#ifdef CONFIG_PTMWAN
		else if (mType == MEDIA_PTM && (WAN_MODE & MODE_PTM))
		{
			int tmp_group;
#ifdef CONFIG_HWNAT
			// bit-0 used for wan in driver
			tmp_group = ((pmap_list[i].fgroup)<<1)|0x1;
			//AUG_PRT("set intf:%x   member:%x!\n", Index, tmp_group);
			setup_hwnat_ptm_member(Index, tmp_group, 1);
#endif
		}
#endif /*CONFIG_PTMWAN*/
#if 0
		else if (mType == MEDIA_ATM && (WAN_MODE & MODE_ATM))
		{
			unsigned short fgroup;
			// bit-0 is set to 0 to diff vc from nas
			fgroup = ((pmap_list[i].fgroup)<<1)|0x0;
			//AUG_PRT("set intf:%x   member:%x!\n", Index, fgroup);
			setup_vc_pmap_lanmember(Index, fgroup);
		}
#endif
	}
}

static void reset_pmap()
{
	int i;

	// Kaohj --- use chain LANIF_MARK for LAN interface marking.
	va_cmd(IPTABLES, 8, 1, "-t", "mangle", "-D", (char *)FW_PREROUTING, "-i", (char *)LANIF, "-j", LANIF_MARK);
	va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-N", LANIF_MARK);
	va_cmd(IPTABLES, 4, 1, "-t", "mangle", "-F", LANIF_MARK);
	va_cmd(IPTABLES, 8, 1, "-t", "mangle", "-A", (char *)FW_PREROUTING, "-i", (char *)LANIF, "-j", LANIF_MARK);

#ifdef CONFIG_IPV6
	// Clear ip6tables rules
	va_cmd(IP6TABLES, 8, 1, "-t", "mangle", "-D", (char *)FW_PREROUTING, "-i", (char *)LANIF, "-j", LANIF_MARK);
	va_cmd(IP6TABLES, 4, 1, "-t", "mangle", "-N", LANIF_MARK);
	va_cmd(IP6TABLES, 4, 1, "-t", "mangle", "-F", LANIF_MARK);
	va_cmd(IP6TABLES, 8, 1, "-t", "mangle", "-A", (char *)FW_PREROUTING, "-i", (char *)LANIF, "-j", LANIF_MARK);
	ip6_priority_i = 32764;
	snprintf(ip6_priority, 6, "%d", ip6_priority_i);
#endif

	//
	va_cmd(EBTABLES, 4, 1, (char *)FW_DEL, (char *)FW_FORWARD, "-j", (char *)PORTMAP_EBTBL);
	//ebtables -N portmapping
	va_cmd(EBTABLES, 2, 1, "-N", (char *)PORTMAP_EBTBL);
	//ebtables -A FORWARD -j portmapping
	va_cmd(EBTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)PORTMAP_EBTBL);

	for (i=PMAP_ETH0_SW0; i<PMAP_ITF_END; i++) {
		va_cmd(BIN_IP, 4, 1, "rule", "del", "fwmark", pol_mark[i]);
#ifdef CONFIG_IPV6
		va_cmd(BIN_IP, 5, 1, "-6", "rule", "del", "fwmark", pol_mark[i]);
#endif
	}

	//The reason looping 9 times is that the max port num is 9;
	for (i = PMAP_ETH0_SW0; i < PMAP_ITF_END; ++i)
	{
		va_cmd(BIN_IP, 3, 1, "rule", "del", "prohibit");
#ifdef CONFIG_IPV6
		va_cmd(BIN_IP, 4, 1, "-6", "rule", "del", "prohibit");
#endif
	}

	va_cmd(EBTABLES, 2, 1, "-F", FW_DHCPS_DIS);
}

int reset_ip_rule()
{
	// Kaohj -- deconfig script for port-mapping; Remove all ip rules for port-mapping.
	va_cmd(PMAP_DECONFIG_SCRIPT, 0, 1);

	#if 0
	//AUG_PRT("Flush the ip rule and re-add the table main/default!\n");
	va_cmd(BIN_IP, 2,  1, "rule", "flush");
	va_cmd(BIN_IP, 6,  1, "rule", "add", "table", "main", "pref", "32766");
	va_cmd(BIN_IP, 6,  1, "rule", "add", "table", "default", "pref", "32767");
	#endif
}

/*
 *	Hook rule for default route for unbinded traffic.
 */
int hook_default_rtable()
{
	char str_dft_tblid[8];
	
	aug_itoa(PMAP_DEFAULT_TBLID, str_dft_tblid, sizeof(str_dft_tblid));
	va_cmd(BIN_IP, 8,  1, "rule", "add", "iif", ALIASNAME_BR0, "table", str_dft_tblid, "pref", "32765");
	write_to_dconfig(7, BIN_IP, "rule", "del", "iif", ALIASNAME_BR0, "table", str_dft_tblid);

#ifdef CONFIG_IPV6
	va_cmd(BIN_IP, 9,  1, "-6", "rule", "add", "iif", ALIASNAME_BR0, "table", str_dft_tblid, "pref", "32765");
	write_to_dconfig(8, BIN_IP, "-6", "rule", "del", "iif", ALIASNAME_BR0, "table", str_dft_tblid);
#endif
}

void setupnewEth2pvc()
{
	reset_ip_rule();

	/*NOTE: if there is vlan-mapping
			be sure it is before pmap*/
#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
	reset_vlanmap();
#endif

	reset_pmap();

	RESET_DECONFIG_SCRIPT();

	hook_default_rtable();

#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
	attach_mark_to_vlanpck();
	//re-sync omci wan-info!!!! check vlan port mapping, port binding
	#if defined(CONFIG_RTK_RG_INIT) || defined(CONFIG_GPON_FEATURE)
	RTK_RG_Sync_OMCI_WAN_INFO();
	#endif
#endif

	//set_vc_fgroup();

	exec_portmp();

	CLOSE_DECONFIG_SCRIPT();
}
#if defined(CONFIG_RTL867X_VLAN_MAPPING) || defined(CONFIG_APOLLO_ROMEDRIVER)
#define SIOCSIFVLANMAP	0x89c2

enum re_vlan_ioctl_cmds {
	RE_VLAN_ADD_VLANPAIR_CMD = 1,
	RE_VLAN_DEL_VLANPAIR_CMD,
	RE_VLAN_DEL_VLANDEV_CMD,
	RE_VLAN_SHOW_ALL_CMD
};

struct re_vlan_ioctl_args {
	enum re_vlan_ioctl_cmds cmd;
	char ifname[IFNAMSIZ];
	unsigned short lanVid;
	unsigned short wanVid;
};

struct vlan_pair {
	unsigned short vid_a;
	unsigned short vid_b;
#ifdef CONFIG_RTK_RG_INIT
	unsigned short rg_vbind_entryID;
#endif	
};


const char	 VLANMAP_EBTBL[] = "vlanmapping";
const char	 VLANMAP_BROUTE[] = "broute_vlanmapping";
const char	 VLAN_BIND[] = "vlanbinding";

static int find_wanif_by_vlanid(unsigned short latvid, MIB_CE_ATM_VC_T* vc_Entry)
{
	int j, totalEntry;

	int ifIndex;

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for(j = 0; j < totalEntry; ++j)
	{
		mib_chain_get(MIB_ATM_VC_TBL, j, (void*)vc_Entry);		
		if(vc_Entry->vlan && vc_Entry->vid == latvid)
		{
			ifIndex	= (int)vc_Entry->ifIndex;

			return ifIndex;
		}
	}
	return -1;
}


/*
vc0 return 0x01, vc1 return 0x02 ...
ppp0 return 0x10, ppp1 return 0x11 ...
nas0_0 return 0x20, nas0_1 return 0x21, ...
ppp0_over_nas0 return 0x30, ...
*/
static uint32_t convert_ifindex_to_mark(int ifindex)
{
	int32_t tbl_id;

	uint32_t mark;

	tbl_id = caculate_tblid(ifindex);

	//AUG_PRT("tbl_id: 0x%x\n",tbl_id);

	mark = tbl_id;

	return mark;
}

/**
(int)51 ===> (char)"0x51000000"
**/
static int convert_mark_to_string(unsigned int mark, char *string, uint32_t num)
{
	int rt;

	char temp_str[4];

	if(NULL  == string)
		return -1;

	if(0 == num || num < 24)
		return -2;

	strncpy(string, "0x", 3);

	rt = snprintf(temp_str, 4, "%x", mark);

	strncat(string, temp_str, 4);

	strncat(string, "000000", 7);

	return rt;

}
/*
(int)51 ===> (char)"0x51000000/0xFF000000"
*/
static int convert_mark_to_string_with_mask(unsigned int mark, char *string, uint32_t num)
{
	int rt;

	char temp_str[4];

	if(NULL  == string)
		return -1;

	if(0 == num || num < 24)
		return -2;

	strncpy(string, "0x", 3);

	rt = snprintf(temp_str, 4, "%x", mark);

	strncat(string, temp_str, 4);

	strncat(string, "000000/0xFF000000", 18);

	return rt;
}

void vlanmp_untreat_brgwan()
{
	int j, totalEntry;

	int ifindex;
	char ifname[IFNAMSIZ];

	MIB_CE_ATM_VC_T vc_Entry;

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for(j = 0; j < totalEntry; ++j)
	{
		mib_chain_get(MIB_ATM_VC_TBL, j, (void*)&vc_Entry);
		if(CHANNEL_MODE_BRIDGE == vc_Entry.cmode)
		{
			ifindex = vc_Entry.ifIndex;
			ifGetName(ifindex, ifname, sizeof(ifname));
			va_cmd(EBTABLES, 6, 1, "-I", VLANMAP_EBTBL, "-i", ifname, "-j", "RETURN");
		}
	}
}

/**
	In the vlan mapping, we should add a chain for every port
**/

void attach_mark_to_vlanpck()
{
	int ifindex, totalPortbd;
	uint32_t mark;

	int port;

	int every_port_do_once;
	int	vlanmp_ebtbl_allport_do_once;

	int32_t tbl_id;

	char str_tblid[10];

	char str_mark[24];
	char str_mark_mask[24];
	char str_vid_a[6];
	char ifname[IFNAMSIZ];

	MIB_CE_PORT_BINDING_T pbEntry;
	IP_RULE_SET vlanmp_iprule_set;

	memset(&vlanmp_iprule_set, 0, sizeof vlanmp_iprule_set);

	vlanmp_ebtbl_allport_do_once = 0;

	totalPortbd = mib_chain_total(MIB_PORT_BINDING_TBL);
	//polling every lanport vlan-mapping entry
	for (port = 0; port < totalPortbd; ++port)
	{
		every_port_do_once = 0;
		//get the number 'port' pbentry!
		mib_chain_get(MIB_PORT_BINDING_TBL, port, (void*)&pbEntry);

		//AUG_PRT("pbEntry.pb_mode: %d\n", pbEntry.pb_mode);
		//is it vlan-mapping lan-port?
		if((unsigned char)VLAN_BASED_MODE == pbEntry.pb_mode)
		{
			struct vlan_pair *vid_pair;
			int k;

			vid_pair = (struct vlan_pair *)&pbEntry.pb_vlan0_a;

			// because there are only 4 pairs~
			for (k=0; k<4; k++)
			{
				//Be sure the content of vlan-mapping exsit!
				if (vid_pair[k].vid_a)
				{
					MIB_CE_ATM_VC_T vc_Entry;
#ifndef CONFIG_APOLLO_ROMEDRIVER
//-------------------------call cathy's func--------------------------------------------------
					/*Pass to the kernel*/
					if(port <= PMAP_ETH0_SW3 && port < SW_LAN_PORT_NUM)
						vlanmap_addpair(SW_LAN_PORT_IF[port], vid_pair[k].vid_a, vid_pair[k].vid_b);
#ifdef WLAN_SUPPORT
					else if((port > PMAP_ETH0_SW3) && (port < PMAP_ITF_END))
						vlanmap_addpair(wlan[port - PMAP_WLAN0], vid_pair[k].vid_a, vid_pair[k].vid_b);
#endif

//--------------------------------------------------------------------------------------------
#endif
					ifindex = find_wanif_by_vlanid(vid_pair[k].vid_b, &vc_Entry);					

					//this vid_pair does not match any wan interface!
					if(ifindex < 0)
					{
						/*Drop the packet ???*/

						continue;
					}
#ifdef CONFIG_RTK_RG_INIT
					AUG_PRT("ifindex : 0x%x, vid_pair[%d].vid_b=%d port=%d\n", ifindex, k, vid_pair[k].vid_b,port);
					/*RG don't support wifi vlan mapping right now~*/
					if(port <= PMAP_ETH0_SW3 && port < SW_LAN_PORT_NUM)
						RG_add_vlanBinding(&vc_Entry,k,vid_pair[k].vid_a,port);
#ifdef WLAN_SUPPORT
					else if((port > PMAP_ETH0_SW3) && (port < PMAP_ITF_END))
						RG_add_vlanBinding(&vc_Entry,k,vid_pair[k].vid_a,port);						
#endif
#endif

					//From now on, the vlan pair find his wanif;

					if(!every_port_do_once)
					{
						every_port_do_once = 1;						
						if(port <= PMAP_ETH0_SW3 && port < SW_LAN_PORT_NUM)
						{
							//va_cmd(EBTABLES, 6, 1, "-t", "broute", "-A", (char *)VLANMAP_BROUTE, "-j", (char *)SW_LAN_PORT_IF[port - PMAP_WLAN0]);
							va_cmd(EBTABLES, 8,  1, "-t", "broute", "-I", VLANMAP_BROUTE,
														"-i", SW_LAN_PORT_IF[port], "-j", broute_chain_nic[port]);
						}
#ifdef WLAN_SUPPORT
						else
						{
							//va_cmd(EBTABLES, 6, 1, "-t", "broute", "-A", (char *)VLANMAP_BROUTE, "-j", (char *)SW_LAN_PORT_IF[port - PMAP_WLAN0]);
							va_cmd(EBTABLES, 8,  1, "-t", "broute", "-I", VLANMAP_BROUTE, "-i", wlan[port - PMAP_WLAN0],
													"-j", broute_chain_wlan[port - PMAP_WLAN0]);
						}
#endif
					}

					tbl_id = caculate_tblid(ifindex);

					aug_itoa(tbl_id, str_tblid, sizeof(str_tblid));

					//AUG_PRT("str_tblid: %s\n", str_tblid);

					//according ifindex, get ifname, e.g, ifindex of vc* is 0xff01, 0xff02,
					//so 0xff01 ===> "vc0"
					ifGetName(ifindex, ifname, sizeof(ifname));

					/*
					vc0 return 0x1, vc1 return 0x2 ...
					ppp0 return 0x10, ppp1 return 0x11 ...
					nas0_0 return 0x20, nas0_1 return 0x21, ...
					ppp0_over_nas0 return 0x30, ...
					*/
					mark = convert_ifindex_to_mark(ifindex);

					vlanmp_iprule_set.fwmark[vlanmp_iprule_set.num] = mark;
					vlanmp_iprule_set.num++;

					//AUG_PRT("mark: 0x%x\n", mark);

					//(int)0x51 ===> (char *)"0x51000000"
					convert_mark_to_string(mark, str_mark, sizeof(str_mark));

					//AUG_PRT("str_mark: %s\n", str_mark);

					//(int)0x51 ===> (char *)"0x51000000/0xFF000000"
					convert_mark_to_string_with_mask(mark, str_mark_mask, sizeof(str_mark_mask));

					aug_itoa(vid_pair[k].vid_a, str_vid_a, sizeof(str_vid_a));

					//the wanif is bridged.  So ,target is Accept
					if(CHANNEL_MODE_BRIDGE == vc_Entry.cmode)
					{
						if(!vlanmp_ebtbl_allport_do_once)
						{
							vlanmp_ebtbl_allport_do_once = 1;
							//AUG_PRT("ebtables -A vlanmapping -p ! 8021Q -j ACCEPT\n");
							//ebtables -A vlanmapping -p ! 8021Q -j ACCEPT
							va_cmd(EBTABLES, 7, 1, "-A", VLANMAP_EBTBL, "-p", "!", "8021Q",
													"-j", "ACCEPT");
						}

						if(port <= PMAP_ETH0_SW3 && port < SW_LAN_PORT_NUM)
						{
#ifdef CONFIG_RTK_RG_INIT
							va_cmd(EBTABLES, 12, 1,"-t","broute","-A",VLAN_BIND, "-p", "8021q", "--vlan-id", str_vid_a, "-i",SW_LAN_PORT_IF[port], "-j", "DROP");
#endif
							va_cmd(EBTABLES, 14, 1, "-t", "broute", "-A", broute_chain_nic[port],
													"-p", "8021q", "--vlan-id", str_vid_a,
													"-j", "mark", "--mark-or", str_mark, "--mark-target", "CONTINUE");

							va_cmd(EBTABLES, 12, 1, "-A", VLANMAP_EBTBL,
												"-p", "8021q", "--vlan-id", str_vid_a,
												"-i", SW_LAN_PORT_IF[port], "-o", ifname, "-j", "ACCEPT");
						}
#ifdef WLAN_SUPPORT
						else
						{
#ifdef CONFIG_RTK_RG_INIT
							va_cmd(EBTABLES, 12, 1,"-t","broute","-A",VLAN_BIND, "-p", "8021q", "--vlan-id", str_vid_a, "-i",ELANIF, "-j", "DROP");
#endif						
							va_cmd(EBTABLES, 14, 1, "-t", "broute", "-A", broute_chain_wlan[port - PMAP_WLAN0],
													"-p", "8021q", "--vlan-id", str_vid_a,
													"-j", "mark", "--mark-or", str_mark, "--mark-target", "CONTINUE");

							va_cmd(EBTABLES, 12, 1, "-A", VLANMAP_EBTBL,
												"-p", "8021q", "--vlan-id", str_vid_a,
												"-i", wlan[port - PMAP_WLAN0], "-o", ifname, "-j", "ACCEPT");
						}
#endif

					}

					//the wanif is routed or pppoe. So target is Drop
					if(CHANNEL_MODE_IPOE == vc_Entry.cmode || CHANNEL_MODE_PPPOE == vc_Entry.cmode)
					{
						int i;
						int dupflag = 0;					
						
						//In the broute chain, the target "DROP" does NOT means get rid of the packet!
						//It actually means that the packet should be routed!
						if(port <= PMAP_ETH0_SW3 && port < SW_LAN_PORT_NUM)
						{
#ifdef CONFIG_RTK_RG_INIT
							va_cmd(EBTABLES, 12, 1,"-t","broute","-A",VLAN_BIND, "-p", "8021q", "--vlan-id", str_vid_a, "-i",SW_LAN_PORT_IF[port], "-j", "DROP");
#endif						
							va_cmd(EBTABLES, 14, 1, "-t", "broute", "-A", broute_chain_nic[port],
													"-p", "8021q", "--vlan-id", str_vid_a,
													"-j", "mark", "--mark-or", str_mark, "--mark-target", "CONTINUE");
						}
#ifdef WLAN_SUPPORT
						else
						{
#ifdef CONFIG_RTK_RG_INIT
							va_cmd(EBTABLES, 12, 1,"-t","broute","-A",VLAN_BIND, "-p", "8021q", "--vlan-id", str_vid_a, "-i",ELANIF, "-j", "DROP");
#endif	
							va_cmd(EBTABLES, 14, 1, "-t", "broute", "-A", broute_chain_wlan[port - PMAP_WLAN0],
													"-p", "8021q", "--vlan-id", str_vid_a,
													"-j", "mark", "--mark-or", str_mark, "--mark-target", "CONTINUE");
						}
#endif
						for(i = 0; i < vlanmp_iprule_set.num; ++i)
						{
							if(mark  == vlanmp_iprule_set.fwmark[i])
								dupflag++;
						}

						if(dupflag > 1)
							continue;

						va_cmd(BIN_IP, 6, 1, "rule", "add", "fwmark", str_mark_mask,
												"table", str_tblid);
						write_to_dconfig(7, BIN_IP, "rule", "del", "fwmark", str_mark_mask,
												"table", str_tblid);
#ifdef CONFIG_IPV6
						va_cmd(BIN_IP, 7, 1, "-6", "rule", "add", "fwmark", str_mark_mask,
												"table", str_tblid);
						write_to_dconfig(8, BIN_IP, "-6", "rule", "del", "fwmark", str_mark_mask,
												"table", str_tblid);
#endif
						// if the itfGroup of this wanif is 0, we should set the ip route table of it
						if(vc_Entry.itfGroup == 0)
						{
							char netAddr[20];
							int iFlags;
							char ipAddr[16];

							if (vc_Entry.cmode == CHANNEL_MODE_IPOE) {
									if (vc_Entry.ipDhcp == DHCP_DISABLED) { // static MER
										strcpy(ipAddr, (const char *)inet_ntoa(*((struct in_addr *)vc_Entry.remoteIpAddr)));
										va_cmd(BIN_IP, 9, 1, "route", "add", "default", "via", ipAddr, "dev", ifname, "table", str_tblid);

										getNetAddrwithMask(&vc_Entry, (char *)netAddr, sizeof(netAddr));

										va_cmd(BIN_IP, 7, 1, "route", "add", (char *)netAddr, "dev", ifname, "table", str_tblid);
									}
									else { // dhcped MER
										va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", ifname, "table", str_tblid);
									}
								}
								else { // PPP
									if (getInFlags(ifname, &iFlags) == 1 && (iFlags & IFF_UP)) {
										va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", ifname, "table", str_tblid);
									}
									else {
										ifGetName(PHY_INTF(ifindex), ifname, sizeof(ifname));
										va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", ifname, "table", str_tblid);
									}
								}
						}

					}

				}
			}
		}
		else if((unsigned char)PORT_BAESD_MODE == pbEntry.pb_mode)
		{
			/*do something*/
			//ebtables -I vlanmapping -i eth0.2 -j portmapping
			if(port <= PMAP_ETH0_SW3 && port < SW_LAN_PORT_NUM)
				va_cmd(EBTABLES, 6, 1, "-I", VLANMAP_EBTBL,
												"-i", SW_LAN_PORT_IF[port],
												"-j", "RETURN");
#ifdef WLAN_SUPPORT
			else if((port > PMAP_ETH0_SW3) && (port < PMAP_ITF_END))
				va_cmd(EBTABLES, 6, 1, "-I", VLANMAP_EBTBL,
												"-i", wlan[port - PMAP_WLAN0],
												"-j", "RETURN");
#endif

		}
	}

	vlanmp_untreat_brgwan();

	return;
}


void reset_vlanmap()
{
	int j;

	//ebtables  -D FORWARD -j vlanmapping
	va_cmd(EBTABLES, 4, 1, (char *)FW_DEL, FW_FORWARD, "-j", (char *)VLANMAP_EBTBL);
	//ebtables  -N vlanmapping
	va_cmd(EBTABLES, 2, 1, "-N", (char *)VLANMAP_EBTBL);
	//ebtables -P vlanmapping DROP
	va_cmd(EBTABLES, 3, 1, "-P", (char *)VLANMAP_EBTBL, "DROP");
	//ebtables -F vlanmapping
	va_cmd(EBTABLES, 2, 1, "-F", (char *)VLANMAP_EBTBL);
	//ebtables -A FORWARD -j vlanmapping
	va_cmd(EBTABLES, 4, 1, "-A", FW_FORWARD, "-j", (char *)VLANMAP_EBTBL);

	//ebtables -t broute -D BROUTING -j broute_vlanmapping
	va_cmd(EBTABLES, 6, 1, "-t", "broute", (char *)FW_DEL, "BROUTING", "-j", (char *)VLANMAP_BROUTE);
	//ebtables -t broute -N broute_vlanmapping
	va_cmd(EBTABLES, 4, 1, "-t", "broute", "-N", (char *)VLANMAP_BROUTE);
#ifdef CONFIG_RTK_RG_INIT
	//ebtables -t broute -D BROUTING -j broute_vlanmapping
	va_cmd(EBTABLES, 6, 1, "-t", "broute", (char *)FW_DEL, "BROUTING", "-j", (char *)VLAN_BIND);
	//ebtables -t broute -N vlanbinding
	va_cmd(EBTABLES, 4, 1, "-t", "broute", "-N", (char *)VLAN_BIND);
	//ebtables -t broute -P vlanbinding RETURN
	va_cmd(EBTABLES, 5, 1, "-t", "broute", "-P", (char *)VLAN_BIND, "RETURN");
	//ebtables -t broute -F vlanbinding
	va_cmd(EBTABLES, 4, 1, "-t", "broute", "-F", (char *)VLAN_BIND);
	//ebtables -t broute -I  BROUTING -j vlanbinding
	va_cmd(EBTABLES, 6, 1, "-t", "broute", "-I", "BROUTING", "-j", (char *)VLAN_BIND);
#endif
	//ebtables -t broute -P broute_vlanmapping RETURN
	va_cmd(EBTABLES, 5, 1, "-t", "broute", "-P", (char *)VLANMAP_BROUTE, "RETURN");
	//ebtables -t broute -I  BROUTING -j broute_vlanmapping
	//ebtables -t broute -F broute_vlanmapping
	va_cmd(EBTABLES, 4, 1, "-t", "broute", "-F", (char *)VLANMAP_BROUTE);
	//ebtables -t broute -I  BROUTING -j broute_vlanmapping
	va_cmd(EBTABLES, 6, 1, "-t", "broute", "-I", "BROUTING", "-j", (char *)VLANMAP_BROUTE);

	for(j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < SW_LAN_PORT_NUM; ++j)
	{
		//ebtables -t broute -D broute_vlanmapping -j brt_ALIASNAME_ETH0DOT2
		//va_cmd(EBTABLES, 6, 1, "-t", "broute", "-D", (char *)VLANMAP_BROUTE, "-j", (char *)broute_chain_nic[j]);
		//ebtables -N vlanmapping
		memcpy(broute_chain_nic[j], BROUTE_CHAIN_PREFIX, sizeof(broute_chain_nic[j]));
		strncat(broute_chain_nic[j], SW_LAN_PORT_IF[j], 12);
		va_cmd(EBTABLES, 6, 1, "-t", "broute", "-N", (char *)broute_chain_nic[j], "-P", "RETURN");
		//ebtables -t broute -F vlanmapping
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-F", (char *)broute_chain_nic[j]);
		//ebtables -I 1 FORWARD -j vlanmapping
		//va_cmd(EBTABLES, 6, 1, "-t", "broute", "-A", (char *)VLANMAP_BROUTE, "-j", (char *)SW_LAN_PORT_IF[j]);

#ifdef CONFIG_RTK_RG_INIT
		RG_flush_vlanBinding(j);

		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-F", (char *)VLAN_BIND, "-i", (char*)SW_LAN_PORT_IF[j]);
#endif
	}
#ifdef WLAN_SUPPORT
	for(j = PMAP_WLAN0; j <= PMAP_WLAN0_VAP3; ++j)
	{
		//ebtables -D FORWARD -j wlan0
		//va_cmd(EBTABLES, 6, 1, "-t", "broute", "-D", (char *)VLANMAP_BROUTE, "-j", (char *)broute_chain_wlan[j - PMAP_WLAN0]);
		//ebtables -N vlanmapping
		memcpy(broute_chain_wlan[j - PMAP_WLAN0], BROUTE_CHAIN_PREFIX, sizeof(broute_chain_wlan[j - PMAP_WLAN0]));
		strncat(broute_chain_wlan[j - PMAP_WLAN0], wlan[j - PMAP_WLAN0], 12);
		va_cmd(EBTABLES, 6, 1, "-t", "broute", "-N", (char *)broute_chain_wlan[j - PMAP_WLAN0], "-P", "RETURN");
		//ebtables -F vlanmapping
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-F", (char *)broute_chain_wlan[j - PMAP_WLAN0]);
		//ebtables -I 1 FORWARD -j vlanmapping
		//va_cmd(EBTABLES, 6, 1, "-t", "broute", "-A", (char *)VLANMAP_BROUTE, "-j", (char *)wlan[j - PMAP_WLAN0]);
#ifdef CONFIG_RTK_RG_INIT
		RG_flush_vlanBinding(j);
		va_cmd(EBTABLES, 4, 1, "-t", "broute", "-F", (char *)VLAN_BIND, "-i", (char *)broute_chain_wlan[j - PMAP_WLAN0]);
#endif
	}
#endif

//----------------------------------Call Cathy's func-------------------------------------------------------
	//reset kernel vlan-mapping table	
#ifndef CONFIG_APOLLO_ROMEDRIVER 	
	for(j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < SW_LAN_PORT_NUM; ++j)
		vlanmap_deldev(SW_LAN_PORT_IF[j]);
#endif	
//----------------------------------------------------------------------------------------------------------
	return;
}

/*
 *	Add vid pair (lan-vid, wan-vid) of VLAN binding for LAN interface 'ifname'.
 *	ifname: LAN interface name, eth0.2, eth0.3, ...
 *	lVid  : LAN side VLAN vid
 *	wVid  : WAN side VLAN vid
 *	Return value:
 *		0: fail
 *		1: successful
 */
int vlanmap_addpair(const char *ifname, unsigned short lVid, unsigned short wVid)
{
	int fd;
	struct re_vlan_ioctl_args arg;

	arg.cmd = RE_VLAN_ADD_VLANPAIR_CMD;
	strcpy(arg.ifname, ifname);
	arg.lanVid = lVid;
	arg.wanVid = wVid;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return 0;
	}

	if (ioctl(fd, SIOCSIFVLANMAP, &arg) < 0) {
		perror("ioctl");
		close(fd);
		return 0;
	}
	close(fd);
	return 1;
}

/*
 *	Delete vid pair (lan-vid, wan-vid) of VLAN binding for LAN interface 'ifname'.
 *	ifname: LAN interface name, eth0.2, eth0.3, ...
 *	lVid  : LAN side VLAN vid
 *	wVid  : WAN side VLAN vid
 *	Return value:
 *		0: fail
 *		1: successful
 */
int vlanmap_delpair(const char *ifname, unsigned short lVid, unsigned short wVid)
{
	int fd;
	struct re_vlan_ioctl_args arg;

	arg.cmd = RE_VLAN_DEL_VLANPAIR_CMD;
	strcpy(arg.ifname, ifname);
	arg.lanVid = lVid;
	arg.wanVid = wVid;
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return 0;
	}

	if (ioctl(fd, SIOCSIFVLANMAP, &arg) < 0) {
		perror("ioctl");
		close(fd);
		return 0;
	}
	close(fd);
	return 1;
}

/*
 *	Flush vid pairs (lan-vid, wan-vid) of VLAN binding for LAN interface 'ifname'.
 *	ifname: LAN interface name, eth0.2, eth0.3, ...
 *	Return value:
 *		0: fail
 *		1: successful
 */
int vlanmap_deldev(const char *ifname)
{
	int fd;
	struct re_vlan_ioctl_args arg;

	arg.cmd = RE_VLAN_DEL_VLANDEV_CMD;
	strcpy(arg.ifname, ifname);
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return 0;
	}

	if (ioctl(fd, SIOCSIFVLANMAP, &arg) < 0) {
		perror("ioctl");
		close(fd);
		return 0;
	}
	close(fd);
	return 1;
}
#endif // of CONFIG_RTL867X_VLAN_MAPPING

#if defined(CONFIG_USER_PPTP_CLIENT_PPTP) || defined(CONFIG_USER_L2TPD_L2TPD) || defined(CONFIG_NET_IPIP)
#include <linux/sockios.h>
#ifdef CONFIG_IPV6_VPN
#include <ifaddrs.h>
#endif

unsigned int get_ip(char * szIf)
{
	int sockfd;
	struct ifreq req;
	struct sockaddr_in *host;

	if(-1 == (sockfd = socket(PF_INET, SOCK_STREAM, 0)))
	{
		perror( "socket" );
		return -1;
	}

	bzero(&req, sizeof(struct ifreq));
	strcpy(req.ifr_name, szIf);
	ioctl(sockfd, SIOCGIFADDR, &req);
	host = (struct sockaddr_in*)&req.ifr_addr;
	close(sockfd);

	return host->sin_addr.s_addr;
}

#ifdef CONFIG_IPV6_VPN
static int ipv6_addr_equal(const struct in6_addr *a1,
				  const struct in6_addr *a2)
{
	return (((a1->s6_addr32[0] ^ a2->s6_addr32[0]) |
		 (a1->s6_addr32[1] ^ a2->s6_addr32[1]) |
		 (a1->s6_addr32[2] ^ a2->s6_addr32[2]) |
		 (a1->s6_addr32[3] ^ a2->s6_addr32[3])) == 0);
}

/**
 * If head file  is not existed in your system, you could get information of IPv6 address
 * in file /proc/net/if_inet6.
 *
 * Contents of file "/proc/net/if_inet6" like as below:
 * 00000000000000000000000000000001 01 80 10 80       lo
 * fe8000000000000032469afffe08aa0f 08 40 20 80     ath0
 * fe8000000000000032469afffe08aa0f 07 40 20 80    wifi0
 * fe8000000000000032469afffe08aa0f 05 40 20 80     eth1
 * fe8000000000000032469afffe08aa0f 03 40 20 80      br0
 * fe8000000000000032469afffe08aa10 04 40 20 80     eth0
 *
 * +------------------------------+ ++ ++ ++ ++    +---+
 * |                                |  |  |  |     |
 * 1                                2  3  4  5     6
 *
 * There are 6 row items parameters:
 * 1 => IPv6 address without ':'
 * 2 => Interface index
 * 3 => Length of prefix
 * 4 => Scope value (see kernel source "include/net/ipv6.h" and "net/ipv6/addrconf.c")
 * 5 => Interface flags (see kernel source "include/linux/rtnetlink.h" and "net/ipv6/addrconf.c")
 * 6 => Device name
 *
 * Note that all values of row 1~5 are hexadecimal string
 */
static int isIfIn6Addr(char * szIf, struct in6_addr *ifaddr6)
{
#define IF_INET6 "/proc/net/if_inet6"
	struct in6_addr addr6;
	char str[128], address[64];
	char *addr, *index, *prefix, *scope, *flags, *name;
	char *delim = " \t\n", *p, *q;
	FILE *fp;
	int count;

	if (NULL == (fp = fopen(IF_INET6, "r"))) {
		perror("fopen error");
		return -1;
	}

	while (fgets(str, sizeof(str), fp)) {
		addr = strtok(str, delim);
		index = strtok(NULL, delim);
		prefix = strtok(NULL, delim);
		scope = strtok(NULL, delim);
		flags = strtok(NULL, delim);
		name = strtok(NULL, delim);

		if (strcmp(name, szIf))
			continue;

		memset(address, 0x00, sizeof(address));
		p = addr;
		q = address;
		count = 0;
		while (*p != ' ') {
				if (count == 4) {
						*q++ = ':';
						count = 0;
				}
				*q++ = *p++;
				count++;
		}

		inet_pton(AF_INET6, address, &addr6);

		if (ipv6_addr_equal(&addr6, ifaddr6)) {
			fclose(fp);
			return 1;
		}
	}

	fclose(fp);
	return 0;
}
#endif

/*
 * Author          : QL
 * Date             : 2011-12-16
 * FUNC_NAME : modPolicyRouteTable
 * PARAMS        : pptp_ifname                pptp interface name(ppp9, ppp10, ...)
 *                        real_addr                    real interface address(vc0, nas0_0, ppp0, ...)
 * Description   : when PPtP Interface is on with def gw, maybe we should add default route to policy route table
 */
void modPolicyRouteTable(const char *pptp_ifname, struct in_addr *real_addr)
{
	MIB_CE_ATM_VC_T entry;
	int32_t totalEntry;
	uint32_t ifIndex, tbl_id, i;
	uint16_t itfGroup;
	char ifname[IFNAMSIZ];
	char str_tblid[10];
	unsigned int devAddr;

	printf("%s %d Enter (pptp:%s real:%x).\n", __func__, __LINE__, pptp_ifname, real_addr->s_addr);

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i = 0; i < totalEntry; ++i)
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&entry))
		{
#ifdef CONFIG_IPV6_VPN
			if (entry.IpProtocol == IPVER_IPV6)
				continue;
#endif

			//only care route mode interface.
			if(entry.cmode != CHANNEL_MODE_BRIDGE)
			{
				ifIndex = entry.ifIndex;
				ifGetName(ifIndex, ifname, sizeof(ifname));

				devAddr = get_ip(ifname);
				if (devAddr != real_addr->s_addr)
					continue;

				tbl_id = caculate_tblid(ifIndex);
				snprintf(str_tblid, 10, "%d", tbl_id);
				//delete original default route.
				printf("ip route del default table %d\n", tbl_id);
				va_cmd(BIN_IP, 5, 1, "route", "del", "default", "table", str_tblid);

				printf("ip route add default dev %s table %d\n", pptp_ifname, tbl_id);
				va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", pptp_ifname, "table", str_tblid);

				break;
			}
		}
	}
}

#ifdef CONFIG_IPV6_VPN
void modIPv6PolicyRouteTable(const char *pptp_ifname, struct in6_addr *real_addr)
{
	MIB_CE_ATM_VC_T entry;
	int32_t totalEntry;
	uint32_t ifIndex, tbl_id, i;
	uint16_t itfGroup;
	char ifname[IFNAMSIZ];
	char str_tblid[10];
	struct in6_addr devAddr;
	unsigned char buff[48];

	printf("%s %d Enter (pptp:%s real:%s).\n", __func__, __LINE__, pptp_ifname,
		inet_ntop(AF_INET6, &real_addr, buff, 48));

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for(i = 0; i < totalEntry; ++i)
	{
		if(mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&entry))
		{
			if (entry.IpProtocol == IPVER_IPV4)
				continue;

			//only care route mode interface.
			if(entry.cmode != CHANNEL_MODE_BRIDGE)
			{
				ifIndex = entry.ifIndex;
				ifGetName(ifIndex, ifname, sizeof(ifname));

				if (!isIfIn6Addr(ifname, real_addr))
					continue;

				tbl_id = caculate_tblid(ifIndex);
				snprintf(str_tblid, 10, "%d", tbl_id);
				//delete original default route.
				printf("ip -6 route del ::/0 table %d\n", tbl_id);
				va_cmd(BIN_IP, 6, 1, "-6", "route", "del", "::/0", "table", str_tblid);

				printf("ip -6 route add ::/0 dev %s table %d\n", pptp_ifname, tbl_id);
				va_cmd(BIN_IP, 8, 1, "-6", "route", "add", "::/0", "dev", pptp_ifname, "table", str_tblid);

				break;
			}
		}
	}
}
#endif
#endif//endof CONFIG_USER_PPTP_CLIENT_PPTP || CONFIG_USER_L2TPD_L2TPD || CONFIG_NET_IPIP
#endif//endof NEW_PORTMAPPING


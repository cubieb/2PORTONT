/*
 *      System routines for Port-mapping
 *
 */

#include <string.h>
#include "debug.h"
#include "utility.h"
#include "wireless.h"

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

#ifdef NEW_PORTMAPPING

struct pmap_s pmap_list[MAX_VC_NUM];

/*
 *	Get the port-mapping bitmap (fgroup); member ports(mapped+unmapped) for WAN interfaces.
 *	-1	failed
 *	0	successful
 */
int get_pmap_fgroup(struct pmap_s *pmap_p, int num)
{
	unsigned int total;
	unsigned short mapped_itfgroup, unmapped_itfgroup;
	int i, k;
	MIB_CE_ATM_VC_T Entry;

	memset(pmap_p, 0, sizeof(struct pmap_s)*num);
	total = mib_chain_total(MIB_ATM_VC_TBL);
	if (total > num)
		return -1;
	mapped_itfgroup = 0;

	for(i = 0; i < total; i++) {
		mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&Entry);
		if (!Entry.enable || !isValidMedia(Entry.ifIndex))
			continue;
		pmap_p[i].ifIndex = Entry.ifIndex;
		pmap_p[i].valid = 1;
		pmap_p[i].itfGroup = Entry.itfGroup;
		mapped_itfgroup |= Entry.itfGroup;
	}
	unmapped_itfgroup = ((~mapped_itfgroup) & 0xFFFF);

	// Get fgroup
	for(i = 0; i < total; i++) {
		// mapped + unmapped
		pmap_p[i].fgroup = pmap_p[i].itfGroup | unmapped_itfgroup;
	}

	return 0;
}

//this func make sure that the different apptype pvcs do not share same lanports
int check_itfGroup(MIB_CE_ATM_VC_Tp pEntry, MIB_CE_ATM_VC_Tp pOldEntry)
{
	uint32_t i;
	uint16_t cur_itfgroup;
	int32_t totalEntry;
	int ret;

	MIB_CE_ATM_VC_T temp_Entry;

	cur_itfgroup = pEntry->itfGroup;

	if(cur_itfgroup)
	{
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		for(i = 0; i < totalEntry; ++i)
		{
			uint16_t sameport;
			ret = mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&temp_Entry);

			if(!ret)
				return -1;

			//if(pEntry->ifIndex == temp_Entry.ifIndex)
			if(pOldEntry && pOldEntry->ifIndex == temp_Entry.ifIndex)
				continue;

			sameport = (uint16_t)(cur_itfgroup & temp_Entry.itfGroup);
			if(sameport) {
				temp_Entry.itfGroup &=  (uint16_t)( ~(sameport));
				mib_chain_update(MIB_ATM_VC_TBL, (void*)&temp_Entry, i);
			}
		}  //end for()
	}
	return 0;
}

//the const strings
const char   BIN_IP[] = "/bin/ip";
const char 	 LANIF_MARK[] = "lan_mark";
const char	 PORTMAP_EBTBL[] = "portmapping";

//pol_mark[ETH0_SW0] = "0x1"  which means:  mark the eth0_sw0 as 0x1000000/0xff000080
//2014/2/27 modify mask 0xff000000 to 0xff000080 because route don't need VLAN header.
//          clear bit 7 VLAN Enable ,set in ebtables.
const char*  pol_mark[] = {"0x1000000/0xff000080", "0x2000000/0xff000080",
													"0x3000000/0xff000080",
													"0x4000000/0xff000080",
													"0x5000000/0xff000080",
													"0x6000000/0xff000080",
													"0x7000000/0xff000080",
													"0x8000000/0xff000080",
													"0x9000000/0xff000080",
													"0xa000000/0xff000080",
													"0xb000000/0xff000080",
													"0xc000000/0xff000080",
													"0xd000000/0xff000080",
													"0xe000000/0xff000080",""};


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

	AUG_PRT("netAddr : %s. tmp : %s\n", pAddr, tmp);

	strcat(pAddr, tmp);

	return 0;
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

	ifGetName( PHY_INTF(ifIndex), ifname, sizeof(ifname));

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

			//if one lanport is bound to bridge pvc, it should NOT be routed!
			if(BIT_IS_SET(itfGroup, j)) {
				va_cmd(BIN_IP, 5, 1, "rule", "add", "fwmark", pol_mark[j], "prohibit");

				// don't touch my dhcp server if mapping to IPTV type
				// iptables -A portmapping_dhcp -i $LAN_IF -p UDP --dport 67 -j DROP				
				if ((pEntry->cmode == CHANNEL_MODE_BRIDGE)) {
					//printf("***** Set drop dhcp rule for %s\n", SW_LAN_PORT_IF[j]);
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)PORTMAP_IPTBL,
						(char *)ARG_I, SW_LAN_PORT_IF[j], "-p", (char *)ARG_UDP,
						(char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
				}
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
								"-i", wlan[j-PMAP_WLAN0],
								"-o", ifname,
								"-j", "RETURN");
			va_cmd(EBTABLES, 9, 1, "-I", PORTMAP_EBTBL, "1",
								"-i", ifname,
								"-o", wlan[j-PMAP_WLAN0],
								"-j", "RETURN");

			//if one lanport is bound to bridge pvc, it should NOT be routed!
			if(BIT_IS_SET(itfGroup, j)) {
				va_cmd(BIN_IP, 5, 1, "rule", "add", "fwmark", pol_mark[j], "prohibit");

				// don't touch my dhcp server if mapping to IPTV type
				// iptables -A portmapping_dhcp -i $WLAN_IF -p UDP --dport 67 -j DROP				
				if ((pEntry->cmode == CHANNEL_MODE_BRIDGE)) {
					//printf("***** Set drop dhcp rule for %s\n", wlan[j - PMAP_WLAN0]);
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)PORTMAP_IPTBL,
						(char *)ARG_I, wlan[j-PMAP_WLAN0], "-p", (char *)ARG_UDP,
						(char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
				}
			}
		}
	}
#endif
	va_cmd(EBTABLES, 6, 1, "-A", PORTMAP_EBTBL, "-i", ifname, "-j", "DROP");
}


//do the command of policy routing which is "ip route " and "ip rule"
static int set_policyrt(MIB_CE_ATM_VC_Tp pEntry)
{
	uint16_t itfGroup;
	uint32_t ifIndex;

	int32_t tbl_id;
        int iFlags;

	char ifname[IFNAMSIZ], devname[IFNAMSIZ];
	char str_tblid[10];
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

	AUG_PRT("the cmode %d itfGroup is 0x%x. ifIndex is 0x%x. ifname is %s\n", pEntry->cmode, itfGroup, ifIndex, ifname);

	// Kaohj -- We should have default route to restrict to port-mapping even if not connected.
	// 	Be noted that default route for dhcped and ppp interfaces must be set
	//	dynamically whenever interface is up
	if (pEntry->cmode == CHANNEL_MODE_IPOE) {
		if (pEntry->ipDhcp == DHCP_DISABLED) { // static MER
			strcpy(ipAddr, (const char *)inet_ntoa(*((struct in_addr *)pEntry->remoteIpAddr)));
			va_cmd(BIN_IP, 7, 1, "route", "add", "default", "via", ipAddr, "table", str_tblid);

			getNetAddrwithMask(pEntry, (char *)netAddr, 20);

			va_cmd(BIN_IP, 7, 1, "route", "add", (char *)netAddr, "dev", ifname, "table", str_tblid);
		}
		else { // dhcped MER
			va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", ifname, "table", str_tblid);
		}
	}
	else { // PPP
		//for ppp, the route should be added in spppd
		#if 0
		if (getInFlags(ifname, &iFlags) == 1 && (iFlags & IFF_UP)) {
			va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", ifname, "table", str_tblid);
		}
		else {
			ifGetName(PHY_INTF(ifIndex), devname, sizeof(devname));
			va_cmd(BIN_IP, 7, 1, "route", "add", "default", "dev", devname, "table", str_tblid);
		}
		#endif
	}

	for(j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < SW_LAN_PORT_NUM; ++j)
	{
		if(BIT_IS_SET(itfGroup, j))
		{
			va_cmd(BIN_IP, 6, 1, "rule", "add", "fwmark", pol_mark[j],
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
		}
	}
#endif

}

//execute the port-mapping commands
int exec_portmp()
{
	int32_t totalEntry;
	MIB_CE_ATM_VC_T temp_Entry;
	int iptbl_done_once, ebtbl_done_once;
	int32_t i, j;
	
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
						va_cmd(IPTABLES, 10, 1, "-t", "mangle", "-A", LANIF_MARK,
									"-i", SW_LAN_PORT_IF[j],
									"-j", "MARK",
									"--set-mark", pol_mark[j]);
					}
#ifdef WLAN_SUPPORT
					for(j = PMAP_WLAN0; j <= PMAP_WLAN1_VAP3; ++j)					
					{
						if(wlan_en[j-PMAP_WLAN0]==0)
							continue;
						
						va_cmd(IPTABLES, 10, 1, "-t", "mangle", "-A", LANIF_MARK,
									"-i", wlan[j-PMAP_WLAN0],
									"-j", "MARK",
									"--set-mark", pol_mark[j]);
					}
#endif

					iptbl_done_once = 111;
				}

				set_policyrt(&temp_Entry);
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
		else if (mType == MEDIA_ATM && (WAN_MODE & MODE_ATM))
		{
			unsigned short fgroup;
			// bit-0 is set to 0 to diff vc from nas
			fgroup = ((pmap_list[i].fgroup)<<1)|0x0;
			//AUG_PRT("set intf:%x   member:%x!\n", Index, fgroup);
			setup_vc_pmap_lanmember(Index, fgroup);
		}

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

	//
	va_cmd(EBTABLES, 4, 1, (char *)FW_DEL, (char *)FW_FORWARD, "-j", (char *)PORTMAP_EBTBL);
	//ebtables -N portmapping
	va_cmd(EBTABLES, 2, 1, "-N", (char *)PORTMAP_EBTBL);
	//ebtables -A FORWARD -j portmapping
	va_cmd(EBTABLES, 4, 1, (char *)FW_ADD, (char *)FW_FORWARD, "-j", (char *)PORTMAP_EBTBL);

	for (i=PMAP_ETH0_SW0; i<PMAP_ITF_END; i++) {
		va_cmd(BIN_IP, 4, 1, "rule", "del", "fwmark", pol_mark[i]);
	}

	//The reason looping 9 times is that the max port num is 9;
	for (i = PMAP_ETH0_SW0; i < PMAP_ITF_END; ++i)
	{
		va_cmd(BIN_IP, 3, 1, "rule", "del", "prohibit");
	}

	// iptables -F portmapping_dhcp
	va_cmd(IPTABLES, 2, 1, "-F", (char *)PORTMAP_IPTBL);
}

void setupnewEth2pvc()
{
	reset_pmap();

	//set_vc_fgroup();

	exec_portmp();
}

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

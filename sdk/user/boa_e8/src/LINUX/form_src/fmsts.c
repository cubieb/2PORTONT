/*
 *      Web server handler routines for Status
 *      Authors:
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>
#include <semaphore.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "autoconf.h"
#endif

/* for ioctl */

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_bridge.h>
#include "adsl_drv.h"
#include <stdio.h>
#include <fcntl.h>
#include "signal.h"
#include "../defs.h"
#include "../boa.h"
#include "fmdefs.h"
#ifdef CONFIG_GPON_FEATURE
#if defined(CONFIG_RTK_L34_ENABLE)
#include <rtk_rg_liteRomeDriver.h>
#else
#include "rtk/ponmac.h"
#include "rtk/gpon.h"
#include "rtk/epon.h"
#endif
#endif

static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";

#define __PME(entry,name)               #name, entry.name

struct wan_status_info {
	char protocol[10];
	char ipAddr[INET_ADDRSTRLEN];
	char *strStatus;
	char servName[MAX_WAN_NAME_LEN];
	unsigned short vlanId;
	unsigned char igmpEnbl;
	char qosEnbl;
	char servType[20];
	char encaps[8];
	char ipv6Addr[256];
};
#ifdef CONFIG_MCAST_VLAN
struct iptv_mcast_info {
	unsigned int ifIndex;	
	char servName[MAX_WAN_NAME_LEN];
	unsigned short vlanId;
	unsigned char enable;
};

int listWanName(int eid, request * wp, int argc, char **argv)
{
	char ifname[IFNAMSIZ];
	int i, entryNum;
	MIB_CE_ATM_VC_T entry;
	struct iptv_mcast_info mEntry[MAX_VC_NUM + MAX_PPP_NUM] = { 0 };
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &entry)) {
			printf("get MIB chain error\n");
			return -1;
		}
		ifGetName(entry.ifIndex, ifname, sizeof(ifname));
		getWanName(&entry, mEntry[i].servName);
		mEntry[i].vlanId = entry.mVid;
		boaWrite(wp,
			 "links.push(new it_nr(\"%d\"" _PTS _PTI "));\n", i,
			 __PME(mEntry[i], servName), __PME(mEntry[i], vlanId));		
	}


}
#endif

int listWanConfig(int eid, request * wp, int argc, char **argv)
{
	char ifname[IFNAMSIZ], *str_ipv4;
	int flags, i, entryNum, flags_found, isPPP;
#ifdef EMBED
	int spid;
#endif
	MIB_CE_ATM_VC_T entry;
	struct in_addr inAddr;
#ifdef CONFIG_DEV_xDSL
	Modem_LinkSpeed vLs;
	int adslflag;
	MEDIA_TYPE_T mType;
#endif

	struct wan_status_info sEntry[MAX_VC_NUM + MAX_PPP_NUM] = { 0 };

#ifdef CONFIG_GPON_FEATURE
	rtk_gpon_fsm_status_t onu;
	#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_gpon_ponStatus_get(&onu);
	#else
		rtk_gpon_ponStatus_get(&onu);
	#endif
#endif

#ifdef EMBED
	if ((spid = read_pid(PPP_PID)) > 0)
		kill(spid, SIGUSR2);
	else
		fprintf(stderr, "spppd pidfile not exists\n");
#endif

#ifdef CONFIG_DEV_xDSL
	// check for xDSL link
	if (!adsl_drv_get
	    (RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE)
	    || vLs.upstreamRate == 0)
		adslflag = 0;
	else
		adslflag = 1;
#endif
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i = 0; i < entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &entry)) {
			printf("get MIB chain error\n");
			return -1;
		}
#ifdef CONFIG_IPV6
		if ((entry.IpProtocol & IPVER_IPV4) == 0)
			continue;	// not IPv4 capable
#endif
		ifGetName(entry.ifIndex, ifname, sizeof(ifname));
		flags_found = getInFlags(ifname, &flags);

		switch (entry.cmode) {
		case CHANNEL_MODE_BRIDGE:
			strcpy(sEntry[i].protocol, "br1483");
			isPPP = 0;
			break;
		case CHANNEL_MODE_IPOE:
			strcpy(sEntry[i].protocol, "IPoE");
			isPPP = 0;
			break;
		case CHANNEL_MODE_PPPOE:	//patch for pppoe proxy
			strcpy(sEntry[i].protocol, "PPPoE");
			isPPP = 1;
			break;
		case CHANNEL_MODE_PPPOA:
			strcpy(sEntry[i].protocol, "PPPoA");
			isPPP = 1;
			break;
		default:
			isPPP = 0;
			break;
		}

		strcpy(sEntry[i].ipAddr, "");
#ifdef EMBED
		if (flags_found && getInAddr(ifname, IP_ADDR, &inAddr) == 1) {
			str_ipv4 = inet_ntoa(inAddr);
			// IP Passthrough or IP unnumbered
			if (flags & IFF_POINTOPOINT && (strcmp(str_ipv4, "10.0.0.1") == 0))
				strcpy(sEntry[i].ipAddr, STR_UNNUMBERED);
			else
				strcpy(sEntry[i].ipAddr, str_ipv4);
		}
#endif

		// set status flag
		if (flags_found) {
			if (flags & IFF_UP) {
#ifdef CONFIG_DEV_xDSL
				mType = MEDIA_INDEX(entry.ifIndex);

				if (!adslflag && 
#ifdef CONFIG_PTMWAN
				(	
#endif
				mType == MEDIA_ATM	
#ifdef CONFIG_PTMWAN
				|| mType == MEDIA_PTM)
#endif
						)
					sEntry[i].strStatus = (char *)IF_DOWN;
				else {
#endif
#ifdef CONFIG_GPON_FEATURE
					if ((onu == 5)
					    || getInAddr(ifname, IP_ADDR, &inAddr) == 1)
#else
					if (strcmp(sEntry[i].protocol, "br1483") == 0
						|| getInAddr(ifname, IP_ADDR, &inAddr) == 1)
#endif
						sEntry[i].strStatus =
						    (char *)IF_UP;
					else
						sEntry[i].strStatus =
						    (char *)IF_DOWN;
#ifdef CONFIG_DEV_xDSL
				}
#endif
			} else
				sEntry[i].strStatus = (char *)IF_DOWN;
		} else
			sEntry[i].strStatus = (char *)IF_NA;

		if (isPPP && strcmp(sEntry[i].strStatus, (char *)IF_UP)) {
			sEntry[i].ipAddr[0] = '\0';
		}
		getWanName(&entry, sEntry[i].servName);

#if defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL_MULTI_ETH_WAN) || (defined(ITF_GROUP_1P) && defined(ITF_GROUP))
		sEntry[i].vlanId = entry.vid;
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
		sEntry[i].igmpEnbl = entry.enableIGMP;
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
		sEntry[i].qosEnbl = entry.enableIpQos;
#endif

		if (entry.qos == 0) {
			if (entry.svtype == 0) {
				strcpy(sEntry[i].servType, "UBR Without PCR");
			} else {
				strcpy(sEntry[i].servType, "UBR With PCR");
			}
		} else if (entry.qos == 1) {
			strcpy(sEntry[i].servType, "CBR");
		} else if (entry.qos == 2) {
			strcpy(sEntry[i].servType, "Non Realtime VBR");
		} else if (entry.qos == 3) {
			strcpy(sEntry[i].servType, "Realtime VBR");
		}

		if (entry.encap == 1) {
			strcpy(sEntry[i].encaps, "LLC");
		} else {
			strcpy(sEntry[i].encaps, "VCMUX");
		}
		//found in mit

#ifdef BR_ROUTE_ONEPVC
		if (entry.cmode == CHANNEL_MODE_BRIDGE && entry.br_route_flag == 1) {
			strcpy(sEntry[i].protocol, "br1483");
			sEntry[i].igmpEnbl = 0;
			strcpy(sEntry[i].ipAddr, "");
		}
#endif
		boaWrite(wp,
			 "links.push(new it_nr(\"%d\"" _PTS _PTS _PTS _PTS
			 _PTS _PTI _PTI _PTI _PTS "));\n", i,
			 __PME(sEntry[i], servName), __PME(sEntry[i], encaps),
			 __PME(sEntry[i], servType), __PME(sEntry[i], protocol),
			 __PME(sEntry[i], ipAddr), __PME(sEntry[i], vlanId),
			 __PME(sEntry[i], igmpEnbl), __PME(sEntry[i], qosEnbl),
			 __PME(sEntry[i], strStatus)
		    );
	}

	return 0;
}

#ifdef CONFIG_IPV6
int listWanConfigIpv6(int eid, request * wp, int argc, char ** argv)
{
	char ifname[IFNAMSIZ], str_ipv6[INET6_ADDRSTRLEN];
	int flags, i, j, k, entryNum, flags_found, isPPP;
#ifdef EMBED
	int spid;
#endif
	struct ipv6_ifaddr ipv6_addr[6];
	MIB_CE_ATM_VC_T entry;
	struct in_addr inAddr;
#ifdef CONFIG_DEV_xDSL
	Modem_LinkSpeed vLs;
	int adslflag;
	MEDIA_TYPE_T mType;
#endif

	struct wan_status_info sEntry[MAX_VC_NUM + MAX_PPP_NUM] = { 0 };
#ifdef CONFIG_GPON_FEATURE
	rtk_gpon_fsm_status_t onu;
	#if defined(CONFIG_RTK_L34_ENABLE)
		rtk_rg_gpon_ponStatus_get(&onu);
	#else
		rtk_gpon_ponStatus_get(&onu);
	#endif
#endif

#ifdef EMBED
	if ((spid = read_pid(PPP_PID)) > 0)
		kill(spid, SIGUSR2);
	else
		fprintf(stderr, "spppd pidfile not exists\n");
#endif

#ifdef CONFIG_DEV_xDSL
	// check for xDSL link
	if (!adsl_drv_get
	    (RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE)
	    || vLs.upstreamRate == 0)
		adslflag = 0;
	else
		adslflag = 1;
#endif

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i = 0; i < entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&entry)) {
			printf("get MIB chain error\n");
			return -1;
		}

		if ((entry.IpProtocol & IPVER_IPV6) == 0)
			continue;	// not IPv6 capable

		ifGetName(entry.ifIndex, ifname, sizeof(ifname));

		switch (entry.cmode) {
		case CHANNEL_MODE_BRIDGE:
			strcpy(sEntry[i].protocol, "br1483");
			isPPP = 0;
			break;
		case CHANNEL_MODE_IPOE:
			strcpy(sEntry[i].protocol, "IPoE");
			isPPP = 0;
			break;
		case CHANNEL_MODE_PPPOE:	//patch for pppoe proxy
			strcpy(sEntry[i].protocol, "PPPoE");
			isPPP = 1;
			break;
		case CHANNEL_MODE_PPPOA:
			strcpy(sEntry[i].protocol, "PPPoA");
			isPPP = 1;
			break;
		default:
			isPPP = 0;
			break;
		}

		k = getifip6(ifname, IPV6_ADDR_UNICAST, ipv6_addr, 6);
		sEntry[i].ipv6Addr[0] = 0;
		if (k) {
			for (j = 0; j < k; j++) {
				inet_ntop(AF_INET6, &ipv6_addr[j].addr, str_ipv6,
					  INET6_ADDRSTRLEN);
				if (j == 0)
					sprintf(sEntry[i].ipv6Addr, "%s/%d",
						str_ipv6,
						ipv6_addr[j].prefix_len);
				else
					sprintf(sEntry[i].ipv6Addr, "%s, %s/%d",
						sEntry[i].ipv6Addr, str_ipv6,
						ipv6_addr[j].prefix_len);
			}
		}

		// set status flag
		if (getInFlags(ifname, &flags) == 1) {
			if (flags & IFF_UP) {
#ifdef CONFIG_DEV_xDSL
				mType = MEDIA_INDEX(entry.ifIndex);

				if (!adslflag && 
#ifdef CONFIG_PTMWAN
				(	
#endif
				mType == MEDIA_ATM	
#ifdef CONFIG_PTMWAN
				|| mType == MEDIA_PTM)
#endif
						)
					sEntry[i].strStatus = (char *)IF_DOWN;
				else {
#endif
#ifdef CONFIG_GPON_FEATURE
					if ((onu == 5) || k!=0)
#else
					if (strcmp(sEntry[i].protocol, "br1483") == 0 || k!=0)
#endif
						sEntry[i].strStatus =
						    (char *)IF_UP;
					else
						sEntry[i].strStatus =
						    (char *)IF_DOWN;
#ifdef CONFIG_DEV_xDSL
				}
#endif
			} else
				sEntry[i].strStatus = (char *)IF_DOWN;
		} else
			sEntry[i].strStatus = (char *)IF_NA;

		if (isPPP && strcmp(sEntry[i].strStatus, (char *)IF_UP)) {
			sEntry[i].ipv6Addr[0] = '\0';
		}
		getWanName(&entry, sEntry[i].servName);

#if defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL_MULTI_ETH_WAN) || (defined(ITF_GROUP_1P) && defined(ITF_GROUP))
		sEntry[i].vlanId = entry.vid;
#endif

#ifdef CONFIG_IGMPPROXY_MULTIWAN
		sEntry[i].igmpEnbl = entry.enableIGMP;
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
		sEntry[i].qosEnbl = entry.enableIpQos;
#endif

		if (entry.qos == 0) {
			if (entry.svtype == 0) {
				strcpy(sEntry[i].servType, "UBR Without PCR");
			} else {
				strcpy(sEntry[i].servType, "UBR With PCR");
			}
		} else if (entry.qos == 1) {
			strcpy(sEntry[i].servType, "CBR");
		} else if (entry.qos == 2) {
			strcpy(sEntry[i].servType, "Non Realtime VBR");
		} else if (entry.qos == 3) {
			strcpy(sEntry[i].servType, "Realtime VBR");
		}

		if (entry.encap == 1) {
			strcpy(sEntry[i].encaps, "LLC");
		} else {
			strcpy(sEntry[i].encaps, "VCMUX");
		}

		//found in mit
#ifdef BR_ROUTE_ONEPVC
		if (entry.cmode == CHANNEL_MODE_BRIDGE && entry.br_route_flag == 1) {
			strcpy(sEntry[i].protocol, "br1483");
			sEntry[i].igmpEnbl = 0;
			strcpy(sEntry[i].ipv6Addr, "");
		}
#endif
		boaWrite(wp,
			  "links.push(new it_nr(\"%d\"" _PTS _PTS _PTS _PTS
			    _PTS _PTI _PTI _PTI _PTS "));\n", i,
			  __PME(sEntry[i], servName), __PME(sEntry[i], encaps),
			  __PME(sEntry[i], servType), __PME(sEntry[i], protocol),
			  __PME(sEntry[i], ipv6Addr), __PME(sEntry[i], vlanId),
			  __PME(sEntry[i], igmpEnbl), __PME(sEntry[i], qosEnbl),
			  __PME(sEntry[i], strStatus)
		    );
	}

	return 0;
}
#endif

#ifdef SUPPORT_WAN_BANDWIDTH_INFO
int listWanBandwidth(int eid, request * wp, int argc, char **argv)
{
	char ifname[IFNAMSIZ];
	int i, entryNum;
	MIB_CE_ATM_VC_T entry;
	char servName[MAX_WAN_NAME_LEN];
	int uploadRate, downloadRate;	
	
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i = 0; i < entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &entry)) {
			printf("get MIB chain error\n");
			return -1;
		}

		ifGetName(entry.ifIndex, ifname, sizeof(ifname));
		
		getWanName(&entry, servName);
		
		if(wan_bandwidth_get(entry.rg_wan_idx, &uploadRate, &downloadRate) < 0)
		{
			uploadRate = -1;
			downloadRate = -1;
		}
		
		boaWrite(wp,
			 "links.push(new it_nr('%d',new it('servName', '%s'), new it('upload', '%d'), new it('download', '%d')));\n", 
			 i,servName, uploadRate, downloadRate);
	}
}
#endif

/*****************************
** ethers stats list
*/
int E8BPktStatsList(int eid, request * wp, int argc, char ** argv)
{
	int i, nBytesSent = 0;
	struct net_device_stats nds;

#ifndef CONFIG_RTK_RG_INIT
	for (i = 0; i < ELANVIF_NUM; i++) {
		get_net_device_stats(ELANVIF[i], &nds);
		nBytesSent += boaWrite(wp, "ethers.push(new it_nr(\"%d\""
				", new it(\"%s\", \"¶Ë¿Ú_%d\")" _PTUL _PTUL
				_PTUL _PTUL _PTUL
				_PTUL _PTUL _PTUL "));\n",
			  i, "ifname", i+1,
			  "rx_packets", nds.rx_packets,
			  "rx_bytes", nds.rx_bytes,
			  "rx_errors", nds.rx_errors,
			  "rx_dropped", nds.rx_dropped,
			  "tx_packets", nds.tx_packets,
			  "tx_bytes", nds.tx_bytes,
			  "tx_errors", nds.tx_errors,
			  "tx_dropped", nds.tx_dropped);
	}
#else // use rg api to retrive switch level packet counter
	for(i = 0 ; i < ELANVIF_NUM; i++)
	{
		unsigned long tx_pkts,tx_drops,tx_errs,rx_pkts,rx_drops,rx_errs;
		unsigned long long int tx_bytes,rx_bytes;

		if(RG_get_portCounter(i,&tx_bytes,&tx_pkts,&tx_drops,&tx_errs,&rx_bytes,&rx_pkts,&rx_drops,&rx_errs) == 0 ){
			// get fail , assign all counter to 0
			tx_pkts = tx_drops = tx_errs = rx_pkts = rx_drops = rx_errs = 0;
			tx_bytes = rx_bytes = 0;
		}

		nBytesSent += boaWrite(wp, "ethers.push(new it_nr(\"%d\""
				", new it(\"%s\", \"¶Ë¿Ú_%d\")" _PTUL _PTULL
				_PTUL _PTUL _PTUL
				_PTULL _PTUL _PTUL "));\n",
			  i, "ifname", i+1,
			  "rx_packets", rx_pkts,
			  "rx_bytes", rx_bytes,
			  "rx_errors", rx_errs,
			  "rx_dropped", rx_drops,
			  "tx_packets", tx_pkts,
			  "tx_bytes", tx_bytes,
			  "tx_errors", tx_errs,
			  "tx_dropped", tx_drops);
	}
#endif


	return nBytesSent;
}

#ifdef CONFIG_USER_LAN_BANDWIDTH_MONITOR
int initPageLanBandwidthMonitor(int eid, request * wp, int argc, char ** argv)
{
	unsigned int cur_usBandwidth = 0;
	unsigned int cur_dsBandwidth = 0;
	lanHostInfo_t *pLanNetInfo=NULL;
	unsigned char macString[32]={0};
	unsigned int count=0;
	int ret=-1, idx;

	ret = get_lan_net_info(&pLanNetInfo, &count);
	if(ret<0)
		goto end;

	for(idx=0; idx<count; idx++)
	{
		memset(macString, 0, 32);
		changeMacToString(pLanNetInfo[idx].mac, macString);
		fillcharZeroToMacString(macString);

		cur_usBandwidth = 0;
		cur_dsBandwidth = 0;
		query_attach_device_realrate(pLanNetInfo[idx].mac, &cur_usBandwidth, &cur_dsBandwidth);

		boaWrite (wp, "push(new it_nr(\"%d\"" _PTS _PTI _PTI"));\n", idx, 
			"mac", macString, "cur_usBand", cur_usBandwidth, "cur_dsBand", cur_dsBandwidth);
	}
	
end:
	if(pLanNetInfo)
		free(pLanNetInfo);
	
	return ret;
}

#endif

#ifdef CONFIG_USER_LANNETINFO

#define LANNETINFOFILE	"/var/lannetinfo"
#define LANNETINFO_RUNFILE	"/var/run/lannetinfo.pid"

#define LAN_HOST_NAME_LENGTH	32
#define VENDOR_NAME_LENGTH		16
#define OS_NAME_LENGTH			16
typedef struct lanNetInfo_s
{
	unsigned char		mac[MAC_ADDR_LEN];
	unsigned char		devName[LAN_HOST_NAME_LENGTH];
	unsigned char		devType;	/* 0-phone 1-pad 2-PC 3-STB 4-other  0xff-unknown */
	unsigned int		ip;
	unsigned char		connectionType;	/* 0- wired 1-wireless */
	unsigned char		port;	/* 0-wifi, 1- lan1, 2-lan2, 3-lan3, 4-lan4 */
	unsigned char		brand[VENDOR_NAME_LENGTH];
	unsigned char		os[OS_NAME_LENGTH];
	unsigned int		onLineTime;
} lanNetInfo_t;


unsigned char *devTypeString[5] = {
	"OTHER",
	"Phone",
	"PC",
	"Pad",
	"STB",
};

unsigned char *connectionTypeString[2] = {
	"Ethernet",
	"Wifi"
};

int initPageLanNetInfo(int eid, request * wp, int argc, char ** argv)
{
	lanNetInfo_t *pLanNetInfo=NULL;
	unsigned char macString[32]={0};
	struct in_addr lanIP;
	unsigned int count=0;
	int ret=-1, idx;

	ret = get_lan_net_info(&pLanNetInfo, &count);
	if(ret<0)
		goto end;

	for(idx=0; idx<count; idx++)
	{
		memset(macString, 0, 32);
		changeMacToString(pLanNetInfo[idx].mac, macString);
		fillcharZeroToMacString(macString);
		lanIP.s_addr = pLanNetInfo[idx].ip;

		boaWrite (wp, "push(new it_nr(\"%d\""_PTS _PTS _PTS _PTS _PTI _PTS _PTS _PTS _PTI"));\n", idx, 
			"devName", pLanNetInfo[idx].devName, "devType", devTypeString[pLanNetInfo[idx].devType],"brand", pLanNetInfo[idx].brand,
			"OS", pLanNetInfo[idx].os, "port", pLanNetInfo[idx].port, "mac", macString, "ip", inet_ntoa(lanIP), "connectionType", connectionTypeString[pLanNetInfo[idx].connectionType],
			"onlineTime", pLanNetInfo[idx].onLineTime);
	}
	
end:
	if(pLanNetInfo)
		free(pLanNetInfo);
	
	return ret;
}

#endif

/*****************************
** devices list
*/
int E8BDhcpClientList(int eid, request * wp, int argc, char ** argv)
{
#ifdef EMBED
	char ipAddr[INET_ADDRSTRLEN], macAddr[20], liveTime[10], devname[MAX_NAME_LEN], *buf = NULL, *ptr;
	DHCPS_SERVING_POOL_T dhcppoolentry;
	FILE *fp;
	int i, entryNum, ret, pid, cnt;
	struct stat status;
	unsigned int ipVal = 0;
	unsigned long leaseFileSize;

	// siganl DHCP server to update lease file
	pid = read_pid(DHCPSERVERPID);
	if (pid > 0) {
		kill(pid, SIGUSR1);
		usleep(1000);
	}

	if (stat(DHCPD_LEASE, &status) < 0)
		goto err;

	// read DHCP server lease file
	leaseFileSize = (unsigned long)(status.st_size);
	buf = malloc(leaseFileSize);
	if (buf == NULL)
		goto err;

	fp = fopen(DHCPD_LEASE, "r");

	if (fp == NULL)
		goto err;

	fread(buf, leaseFileSize, 1, fp);
	fclose(fp);
	ptr = buf;

	entryNum = mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL);
	cnt = 0;
	memset(&devname, 0, sizeof(devname));

	while (1) {
		ret = getOneDhcpClient(&ptr, &leaseFileSize, ipAddr, macAddr, liveTime);
		/* lijian: 20080904 END */

		if (ret < 0)
			break;
		if (ret == 0)
			continue;

		inet_aton(ipAddr, (struct in_addr *)&ipVal);

		for (i = 0; i < entryNum; i++) {
			if (!mib_chain_get
			    (MIB_DHCPS_SERVING_POOL_TBL, i,
			     (void *)&dhcppoolentry))
				continue;

			if (ipVal >= *(unsigned int *)dhcppoolentry.startaddr
			    && ipVal <=
			    *(unsigned int *)dhcppoolentry.endaddr) {
				strcpy(devname, dhcppoolentry.poolname);
				break;
			}
		}

		boaWrite(wp, "clts.push(new it_nr(\"%d\"" _PTS _PTS _PTS _PTS "));\n",
			  cnt, _PMEX(devname), _PMEX(macAddr), _PMEX(ipAddr), _PMEX(liveTime));
		cnt++;
	}

err:
	if (buf)
		free(buf);

	return 0;
#else
	return 0;
#endif
}


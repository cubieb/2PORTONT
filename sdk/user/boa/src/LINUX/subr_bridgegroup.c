/*
 *      System routines for Port-mapping
 *
 */

#include <string.h>
#include "debug.h"
#include "utility.h"
#include "wireless.h"

#if defined(CONFIG_USER_BRIDGE_GROUPING)
#ifdef CONFIG_RTK_RG_INIT // Rostelecom, Port Binding function
#include <rtk/rtusr_rg_api.h>
#define OMCI_WAN_INFO "/proc/omci/wanInfo"

void set_port_binding_rule(int enable)
{
	int i = 0, j = 0, num = 0;
	MIB_CE_ATM_VC_T pvcEntry;
	num = mib_chain_total(MIB_ATM_VC_TBL);

	if (enable)
	{
		// don't touch the DHCP server if mapping to bridge WAN
		for (i = 0; i < num; i++)
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
				continue;

			if (pvcEntry.enable == 0)
				continue;

			if (pvcEntry.cmode == CHANNEL_MODE_BRIDGE && pvcEntry.itfGroup != 0)
			{
				for (j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < SW_LAN_PORT_NUM; ++j)
				{
					if (BIT_IS_SET(pvcEntry.itfGroup, j)) {
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INACC, (char *)ARG_I, SW_LAN_PORT_IF[j], "-p", (char *)ARG_UDP, (char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
				}

#ifdef WLAN_SUPPORT
				for (j = PMAP_WLAN0; j <= PMAP_WLAN1_VAP3; ++j)
				{
					if (BIT_IS_SET(pvcEntry.itfGroup, j)) {
						va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, (char *)FW_INACC, (char *)ARG_I, wlan[j - PMAP_WLAN0], "-p", (char *)ARG_UDP, (char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
				}
#endif // WLAN_SUPPORT
			}
		}
	}
	else
	{
		// don't touch the DHCP server if mapping to bridge WAN
		for (i = 0; i < num; i++)
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
				continue;

			if (pvcEntry.enable == 0)
				continue;

			if (pvcEntry.cmode == CHANNEL_MODE_BRIDGE && pvcEntry.itfGroup != 0)
			{
				for (j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < SW_LAN_PORT_NUM; ++j)
				{
					if (BIT_IS_SET(pvcEntry.itfGroup, j)) {
						va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_INACC, (char *)ARG_I, SW_LAN_PORT_IF[j], "-p", (char *)ARG_UDP, (char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
				}

#ifdef WLAN_SUPPORT
				for (j = PMAP_WLAN0; j <= PMAP_WLAN1_VAP3; ++j)
				{
					if (BIT_IS_SET(pvcEntry.itfGroup, j)) {
						va_cmd(IPTABLES, 10, 1, (char *)FW_DEL, (char *)FW_INACC, (char *)ARG_I, wlan[j - PMAP_WLAN0], "-p", (char *)ARG_UDP, (char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
					}
				}
#endif // WLAN_SUPPORT
			}
		}
	}
}

int get_DefaultGWGroup()
{
	int i = 0, num = 0;
	MIB_CE_ATM_VC_T pvcEntry;
	num = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < num; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;

		if (pvcEntry.dgw == 1) // default gateway
		{
			return pvcEntry.itfGroupNum;
		}
	}

	return 0;
}

int rg_set_port_binding_mask(unsigned int set_wanlist)
{
	int i = 0, num = 0;
	MIB_CE_ATM_VC_T pvcEntry;
	char cmdStr[64] = {0};

	num = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < num; i++)
	{
		if ((set_wanlist & (1 << i)) != 0)
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
				continue;

			if (pvcEntry.enable == 0)
				continue;

			// reset omci wan info
			snprintf(cmdStr, sizeof(cmdStr), "echo %d %d %d %d %d %d %d > %s", pvcEntry.rg_wan_idx, 0, 0, 0, 0, 0, 0, OMCI_WAN_INFO);
			system(cmdStr);

			// only update wan_info.port_binding_mask.portmask and wan_info.wlan0_dev_binding_mask
			if (RG_add_wan(&pvcEntry, i) == -1)
			{
				fprintf(stderr, "[%s:%s@%d] RG_add_wan ERROR\n", __FILE__, __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}

	return 0;
}

int set_port_binding_mask(unsigned int *wanlist)
{
	int i = 0, j = 0, num = 0, ifnum = 0, DefaultGWGroup = 0, change_num = 0, find_group_first_wan = 0, all_in_one_group = 0;
	unsigned int port_binding_mask = 0;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];
	struct availableItfInfo itfList[MAX_NUM_OF_ITFS];
	MIB_CE_SW_PORT_T swportEntry;

	DefaultGWGroup = get_DefaultGWGroup();
	int Available_num = get_AvailableInterface(itfList, MAX_NUM_OF_ITFS);
	*wanlist = 0;

	set_port_binding_rule(0);

	for (i = 0; i < IFGROUP_NUM; i++)
	{
		memset(itfs, 0, sizeof(itfs));
		ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, i);

		if (ifnum == 0)
			continue;

		if (ifnum == Available_num)
			all_in_one_group = 1;

		port_binding_mask = 0;
		find_group_first_wan = 0;

		// LAN Port
		for (j = PMAP_ETH0_SW0; j <= PMAP_ETH0_SW3 && j < SW_LAN_PORT_NUM; ++j)
		{
			if (!mib_chain_get(MIB_SW_PORT_TBL, j, &swportEntry))
				continue;

			if (swportEntry.itfGroup == i)
			{
				port_binding_mask |= (1 << j);
			}
		}

#ifdef WLAN_SUPPORT
		unsigned char vUChar = 0, wlgroup = 0;
		MIB_CE_MBSSIB_T mbssidEntry;
		int ori_wlan_idx;

		ori_wlan_idx = wlan_idx;
		wlan_idx = 0;

		for (j = PMAP_WLAN0; j <= PMAP_WLAN1_VAP3; j++)
		{
			if (wlan_en[j - PMAP_WLAN0] == 0)
				continue;

			if (j == PMAP_WLAN0)
			{
				mib_get(MIB_WLAN_DISABLED, &vUChar);
				if (vUChar == 0) {
					mib_get(MIB_WLAN_ITF_GROUP, &wlgroup);

					if (wlgroup == i)
					{
						port_binding_mask |= (1 << j);
					}
				}
			}

#ifdef WLAN_MBSSID
			if (j >= PMAP_WLAN0_VAP0 && j <= PMAP_WLAN0_VAP3)
			{
				if (!mib_chain_get(MIB_MBSSIB_TBL, (j - PMAP_WLAN0), &mbssidEntry) || mbssidEntry.wlanDisabled)
					continue;

				mib_get(MIB_WLAN_VAP0_ITF_GROUP + ((j - PMAP_WLAN0_VAP0) << 1), &wlgroup);
				if (wlgroup == i)
				{
					port_binding_mask |= (1 << j);
				}
			}
#endif

#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
			if (j == PMAP_WLAN1)
			{
				mib_get(MIB_WLAN1_DISABLED, &vUChar);
				if (vUChar == 0) {
					mib_get(MIB_WLAN1_ITF_GROUP, &wlgroup);

					if (wlgroup == i)
					{
						port_binding_mask |= (1 << j);
					}
				}
			}

#ifdef WLAN_MBSSID
			if (j >= PMAP_WLAN1_VAP0 && j <= PMAP_WLAN1_VAP3)
			{
				if (!mib_chain_get(MIB_WLAN1_MBSSIB_TBL, (j - PMAP_WLAN1), &mbssidEntry) || mbssidEntry.wlanDisabled)
					continue;

				mib_get(MIB_WLAN1_VAP0_ITF_GROUP + ((j - PMAP_WLAN1_VAP0) << 1), &wlgroup);
				if (wlgroup == i)
				{
					port_binding_mask |= (1 << j);
				}
			}
#endif
#endif
		}
		wlan_idx = ori_wlan_idx;
#endif // WLAN_SUPPORT

		//fprintf(stderr, "\033[1;31m[%s:%s@%d]\033[0m  [Group%d] port_binding_mask=%d\n", __FILE__, __FUNCTION__, __LINE__, i, port_binding_mask);

		MIB_CE_ATM_VC_T pvcEntry;
		num = mib_chain_total(MIB_ATM_VC_TBL);
		for (j = 0; j < num; j++)
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry))
				continue;

			if (pvcEntry.itfGroupNum == i && all_in_one_group == 1) // all interface in same group, set itfGroup to 0
			{
				if (pvcEntry.itfGroup != 0)
				{
					pvcEntry.itfGroup = 0;
					mib_chain_update(MIB_ATM_VC_TBL, (void *)&pvcEntry, j);
					change_num++;
					*wanlist |= (1 << j);
				}
				continue;
			}

			if (pvcEntry.itfGroupNum == DefaultGWGroup && pvcEntry.dgw == 0) // default gateway group, only set default gateway itfGroup
			{
				if (pvcEntry.itfGroup != 0)
				{
					pvcEntry.itfGroup = 0;
					mib_chain_update(MIB_ATM_VC_TBL, (void *)&pvcEntry, j);
					change_num++;
					*wanlist |= (1 << j);
				}
				continue;
			}

			if (pvcEntry.itfGroupNum == i)
			{
				if (find_group_first_wan == 0)
				{
					find_group_first_wan = 1;
					if (pvcEntry.itfGroup != port_binding_mask)
					{
						pvcEntry.itfGroup = port_binding_mask;
						mib_chain_update(MIB_ATM_VC_TBL, (void *)&pvcEntry, j);
						change_num++;
						*wanlist |= (1 << j);
					}
				}
				else
				{
					if (pvcEntry.itfGroup != 0)
					{
						pvcEntry.itfGroup = 0;
						mib_chain_update(MIB_ATM_VC_TBL, (void *)&pvcEntry, j);
						change_num++;
						*wanlist |= (1 << j);
					}
				}
			}
		}
	}

	set_port_binding_rule(1);

	return change_num;
}
#endif

int get_AvailableInterface(struct availableItfInfo *info, int len)
{
#if defined(ITF_GROUP_4P) || defined(CONFIG_RTL_8676HWNAT) || defined(CONFIG_APOLLO_ROMEDRIVER)
	unsigned int swNum;
	char strlan[] = "LAN0";
	MIB_CE_SW_PORT_T Entry;
#endif
	unsigned int vcNum;
	int i, num;
	unsigned char mygroup;
	MIB_CE_ATM_VC_T pvcEntry;
#if defined(WLAN_SUPPORT) || defined(WLAN_MBSSID)
	int ori_wlan_idx;
#endif
#ifdef WLAN_SUPPORT
	unsigned char vUChar;
#endif
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_T mbssidEntry;
#endif

	num = 0;
#if defined(ITF_GROUP_4P) || defined(CONFIG_RTL_8676HWNAT) || defined(CONFIG_APOLLO_ROMEDRIVER)
	// LAN ports
	swNum = mib_chain_total(MIB_SW_PORT_TBL);
	for (i = 0; i < ELANVIF_NUM; i++) {
		if (!mib_chain_get(MIB_SW_PORT_TBL, i, &Entry))
			continue;

		strlan[3] = '0' + virt2user[i];	// LAN1, LAN2, LAN3, LAN4
		info[num].ifdomain = DOMAIN_ELAN;
		info[num].ifid = i;
		strncpy(info[num].name, strlan, sizeof(info[num].name));
		info[num].itfGroup = Entry.itfGroup;
		if (++num >= len)
			return num;

#ifdef CONFIG_USER_VLAN_ON_LAN		
		strlan[3] = '0' + virt2user[i];	// LAN1, LAN2, LAN3, LAN4
		info[num].ifdomain = DOMAIN_ELAN;
		info[num].ifid = i + swNum;
		snprintf(info[num].name, sizeof(info[num].name), "%s.%hu", strlan, Entry.vid);
		info[num].itfGroup = Entry.vlan_on_lan_itfGroup;
		if (++num >= len)
			return num;
#endif
	}
#endif

#ifdef WLAN_SUPPORT
	// wlan0
	ori_wlan_idx = wlan_idx;
	wlan_idx = 0;		// Magician: mib_get will add wlan_idx to mib id. Therefore wlan_idx must be set to 0.

	mib_get(MIB_WLAN_DISABLED, &vUChar);
	if (vUChar == 0) {
		mib_get(MIB_WLAN_ITF_GROUP, &mygroup);
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid = 0;
		strncpy(info[num].name, (char *)WLANIF[0], sizeof(info[num].name));
		info[num].itfGroup = mygroup;
		if (++num >= len)
			return num;
	}

	// wlan1
#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	mib_get(MIB_WLAN1_DISABLED, &vUChar);
	if (vUChar == 0) {
		mib_get(MIB_WLAN1_ITF_GROUP, &mygroup);
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid = 10;
		strncpy(info[num].name, (char *)WLANIF[1], sizeof(info[num].name));
		info[num].itfGroup = mygroup;
		if (++num >= len)
			return num;
	}
#endif

#ifdef WLAN_MBSSID
	for (i = 0; i < IFGROUP_NUM - 1; i++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i + 1, &mbssidEntry) ||
				mbssidEntry.wlanDisabled)
			continue;

		mib_get(MIB_WLAN_VAP0_ITF_GROUP + (i << 1), &mygroup);
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid = i + 1;
		sprintf(info[num].name, "wlan0-vap%d", i);
		info[num].itfGroup = mygroup;
		if (++num >= len)
			return num;
	}

#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	for (i = 0; i < IFGROUP_NUM - 1; i++) {
		if (!mib_chain_get(MIB_WLAN1_MBSSIB_TBL, i + 1, &mbssidEntry) ||
				mbssidEntry.wlanDisabled)
			continue;

		mib_get(MIB_WLAN1_VAP0_ITF_GROUP + (i << 1), &mygroup);
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid = i + 11;
		sprintf(info[num].name, "wlan1-vap%d", i);
		info[num].itfGroup = mygroup;
		if (++num >= len)
			return num;
	}
#endif
#endif // WLAN_MBSSID
	wlan_idx = ori_wlan_idx;
#endif // WLAN_SUPPORT

#ifdef CONFIG_USB_ETH
	// usb0
	mib_get(MIB_USBETH_ITF_GROUP, &mygroup);
	info[num].ifdomain = DOMAIN_ULAN;
	info[num].ifid = 0;
	strncpy(info[num].name, (char *)USBETHIF, sizeof(info[num].name));
	info[num].itfGroup = mygroup;
	if (++num >= len)
		return num;
#endif

	// vc
	vcNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i = 0; i < vcNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &pvcEntry)) {
			printf("Get chain record error!\n");
			continue;
		}

		if (pvcEntry.enable == 0
		    || !isValidMedia(pvcEntry.ifIndex)
#ifdef CONFIG_RTK_RG_INIT // Rostelecom, Port Binding function
#else
		    || ( pvcEntry.cmode != CHANNEL_MODE_BRIDGE && pvcEntry.brmode == BRIDGE_DISABLE)
#endif
		    )
			continue;

		info[num].ifdomain = DOMAIN_WAN;
		info[num].ifid = pvcEntry.ifIndex;
		ifGetName(pvcEntry.ifIndex, info[num].name, sizeof(info[num].name));
		info[num].itfGroup = pvcEntry.itfGroupNum;
		if (++num >= len)
			return num;
	}

	return num;
}

int setup_bridge_grouping(int mode)
{
	unsigned char grpnum;
	int i, ifnum, ret, status = 0;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];
	char brif[] = "br0";
	unsigned char vUChar;
	unsigned short vUShort;
	char str[16];
	MIB_CE_ATM_VC_T Entry;

#ifdef CONFIG_RTK_RG_INIT // Rostelecom, Port Binding function
	// do nothing
#else
	if (mode == ADD_RULE) {
		for (grpnum = 0; grpnum <= 4; grpnum++) {
			ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, grpnum);
			if (ifnum <= 0)
				continue;

			// default group (grpnum == 0) should not be "addbr" here
			if (grpnum == 0)
				goto step_3;

			brif[2] = '0' + grpnum;

			// (1) use brctl to create bridge interface
			// brctl addbr br[1-4]
			status |= va_cmd(BRCTL, 2, 1, "addbr", brif);

			// ifconfig br[1-4] hw ether "macaddr"
			if (getMIB2Str(MIB_ELAN_MAC_ADDR, str) == 0)
				status |= va_cmd(IFCONFIG, 4, 1, brif, "hw", "ether", str);

			if (mib_get(MIB_BRCTL_STP, &vUChar)) {
				if (vUChar) {
					// stp on
					// brctl stp br[1-4] on
					status |= va_cmd(BRCTL, 3, 1, "stp",
						   brif, "on");
				} else {
					// stp off
					// brctl stp br[1-4] off
					status |= va_cmd(BRCTL, 3, 1, "stp",
						   brif, "off");

					// brctl setfd br[1-4] 1
					// if forwarding_delay=0, fdb_get may fail in serveral seconds after booting
					status |= va_cmd(BRCTL, 3, 1, "setfd",
						   brif, "1");
				}
			}

			// brctl setageing br[1-4] ageingtime
			if (mib_get(MIB_BRCTL_AGEINGTIME, &vUShort)) {
				snprintf(str, sizeof(str), "%hu", vUShort);
				status |= va_cmd(BRCTL, 3, 1, "setageing",
					   brif, str);
			}

			// (2) use ifconfig to up interface
			// ifconfig br[1-4] up
			status |= va_cmd(IFCONFIG, 2, 1, brif, "up");
step_3:
			// (3) for all interfaces in grpnum, remove from br0, add to br[0-4]
			for (i = 0; i < ifnum; i++) {
				if (itfs[i].ifdomain == DOMAIN_WAN
					&& getATMVCEntryByIfIndex(itfs[i].ifid, &Entry)) {
				// skip when brpnum is 0 (br0) and itfs[i] is non-bridged-mode WAN
					//if (grpnum == 0 && Entry.cmode != ADSL_BR1483)
					if (grpnum == 0 && (Entry.cmode != CHANNEL_MODE_BRIDGE && Entry.brmode == BRIDGE_DISABLE))					
						continue;

					// convert ppp0 to vc0 (if ppp0 is over vc0)
					ifGetName(PHY_INTF(Entry.ifIndex), itfs[i].name, sizeof(itfs[i].name));
				}

				if (itfs[i].ifdomain == DOMAIN_ELAN) {
				// convert LAN1 to eth0.2
				ret = sscanf(itfs[i].name, "LAN%hhu.%hu", &vUChar, &vUShort);
				if (ret == 1)
					snprintf(itfs[i].name, sizeof(itfs[i].name), "%s", ELANVIF[vUChar - 1]);
				else if (ret == 2)
					snprintf(itfs[i].name, sizeof(itfs[i].name), "%s.%hu",
							ELANVIF[vUChar - 1], vUShort);
				}

				// brctl delif br0 eth0.2
				status |= va_cmd(BRCTL, 3, 1, "delif", (char *)BRIF, itfs[i].name);

				// brctl addif br[0-4] eth0.2
				status |= va_cmd(BRCTL, 3, 1, "addif", brif, itfs[i].name);

			}
		}
	} else if (mode == DEL_RULE) {
		for (grpnum = 1; grpnum <= 4; grpnum++) {
			// default group (grpnum == 0) should not be "delbr" here

			ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, grpnum);
			if (ifnum <= 0)
				continue;

			brif[2] = '0' + grpnum;

			// (1) down br[1-4]
			// ifconfig br[1-4] down
			status |= va_cmd(IFCONFIG, 2, 1, brif, "down");

			// (2) del all interfaces on br[1-4]
			// brctl delbr br[1-4]              
			status |= va_cmd(BRCTL, 2, 1, "delbr", brif);
		}
	}
#endif

	return status;
}

void setgroup(char *list, unsigned char grpnum)
{
	int ifdomain, ifid;
	int i, num;
	char *arg0, *token;
	MIB_CE_ATM_VC_T Entry;	
		
	arg0 = list;
	while ((token = strtok(arg0, ",")) != NULL) {
		ifid = atoi(token);
		ifdomain = IF_DOMAIN(ifid);
		ifid = IF_INDEX(ifid);	
		
#if defined(ITF_GROUP_4P) || defined(CONFIG_RTL_8676HWNAT) || defined(CONFIG_APOLLO_ROMEDRIVER)
		if (ifdomain == DOMAIN_ELAN) {
			MIB_CE_SW_PORT_T Entry;

			num = mib_chain_total(MIB_SW_PORT_TBL);
			if (ifid < num) {
				if (!mib_chain_get
				    (MIB_SW_PORT_TBL, ifid, &Entry))
					return;
				Entry.itfGroup = grpnum;				
				mib_chain_update(MIB_SW_PORT_TBL, &Entry, ifid);
			} else if (ifid >= num && ifid < num * 2) {
				if (!mib_chain_get
				    (MIB_SW_PORT_TBL, ifid - num, &Entry))
					return;
				Entry.vlan_on_lan_itfGroup = grpnum;				
				mib_chain_update(MIB_SW_PORT_TBL, &Entry, ifid - num);
			}
		}
#endif

#ifdef WLAN_SUPPORT
		else if (ifdomain == DOMAIN_WLAN) {
			int ori_wlan_idx = wlan_idx;

			wlan_idx = 0;

			if (ifid == 0) {
				mib_set(MIB_WLAN_ITF_GROUP, &grpnum);
				goto next_token;
			}

#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
			if (ifid == 10) {
				mib_set(MIB_WLAN1_ITF_GROUP, &grpnum);
				goto next_token;
			}
#endif

			// Added by Mason Yu
#ifdef WLAN_MBSSID
			// wlan0_vap0
			if (ifid >= 1 && ifid < IFGROUP_NUM) {
				mib_set(MIB_WLAN_VAP0_ITF_GROUP + ((ifid - 1) << 1), &grpnum);
				goto next_token;
			}
#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
			// wlan1_vap0
			if (ifid >= 11 && ifid < IFGROUP_NUM + 10) {
				mib_set(MIB_WLAN1_VAP0_ITF_GROUP + ((ifid - 11) << 1), &grpnum);
				goto next_token;
			}
#endif
#endif // WLAN_MBSSID
			wlan_idx = ori_wlan_idx;
		}
#endif // WLAN_SUPPORT

#ifdef CONFIG_USB_ETH
		else if (ifdomain == DOMAIN_ULAN) {
			if (ifid == 0)
				mib_set(MIB_USBETH_ITF_GROUP, &grpnum);
		}
#endif

		else if (ifdomain == DOMAIN_WAN) {

			num = mib_chain_total(MIB_ATM_VC_TBL);
			for (i = 0; i < num; i++) {
				if (!mib_chain_get
				    (MIB_ATM_VC_TBL, i, (void *)&Entry))
					return;
				if (Entry.enable && isValidMedia(Entry.ifIndex)
				    && Entry.ifIndex == ifid) {
					Entry.itfGroupNum = grpnum;
					mib_chain_update(MIB_ATM_VC_TBL,
							 (void *)&Entry, i);
				}
			}
		}
next_token:
		arg0 = NULL;
	}
}

/*------------------------------------------------------------------
 * Get a list of interface info. (itfInfo) of group.
 * where,
 * info: a list of interface info entries
 * len: max length of the info list
 * grpnum: group id
 *-----------------------------------------------------------------*/
int get_group_ifinfo(struct itfInfo *info, int len, unsigned char grpnum)
{
#if defined(ITF_GROUP_4P) || defined(CONFIG_RTL_8676HWNAT) || defined(CONFIG_APOLLO_ROMEDRIVER)
	unsigned int swNum;
	char strlan[] = "LAN0";
	MIB_CE_SW_PORT_T Entry;
#endif
	unsigned int vcNum;
	int i, num;
	unsigned char mygroup;
	MIB_CE_ATM_VC_T pvcEntry;
#if defined(WLAN_SUPPORT) || defined(WLAN_MBSSID)
	int ori_wlan_idx;
#endif
#ifdef WLAN_SUPPORT
	unsigned char vUChar;
#endif
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_T mbssidEntry;
#endif

	num = 0;
#if defined(ITF_GROUP_4P) || defined(CONFIG_RTL_8676HWNAT) || defined(CONFIG_APOLLO_ROMEDRIVER)
	// LAN ports
	swNum = mib_chain_total(MIB_SW_PORT_TBL);
	for (i = 0; i < ELANVIF_NUM; i++) {
		if (!mib_chain_get(MIB_SW_PORT_TBL, i, &Entry))
			return -1;
		if (Entry.itfGroup == grpnum) {
			strlan[3] = '0' + virt2user[i];	// LAN1, LAN2, LAN3, LAN4
			info[num].ifdomain = DOMAIN_ELAN;
			info[num].ifid = i;
			strncpy(info[num].name, strlan, sizeof(info[num].name));
			if (++num >= len)
				return num;
		}

#ifdef CONFIG_USER_VLAN_ON_LAN		
		if (Entry.vlan_on_lan_enabled && Entry.vlan_on_lan_itfGroup == grpnum) {
			strlan[3] = '0' + virt2user[i];	// LAN1, LAN2, LAN3, LAN4
			info[num].ifdomain = DOMAIN_ELAN;
			info[num].ifid = i + swNum;
			snprintf(info[num].name, sizeof(info[num].name), "%s.%hu", strlan, Entry.vid);
			if (++num >= len)
				return num;
		}
#endif
	}
#endif

#ifdef WLAN_SUPPORT
	// wlan0
	ori_wlan_idx = wlan_idx;
	wlan_idx = 0;		// Magician: mib_get will add wlan_idx to mib id. Therefore wlan_idx must be set to 0.

	mib_get(MIB_WLAN_DISABLED, &vUChar);
	if (vUChar == 0) {
		mib_get(MIB_WLAN_ITF_GROUP, &mygroup);
		if (mygroup == grpnum) {
			info[num].ifdomain = DOMAIN_WLAN;
			info[num].ifid = 0;
			strncpy(info[num].name, (char *)WLANIF[0], sizeof(info[num].name));
			if (++num >= len)
				return num;
		}
	}
// wlan1
#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	mib_get(MIB_WLAN1_DISABLED, &vUChar);
	if (vUChar == 0) {
		mib_get(MIB_WLAN1_ITF_GROUP, &mygroup);
		if (mygroup == grpnum) {
			info[num].ifdomain = DOMAIN_WLAN;
			info[num].ifid = 10;
			strncpy(info[num].name, (char *)WLANIF[1], sizeof(info[num].name));
			if (++num >= len)
				return num;
		}
	}
#endif

#ifdef WLAN_MBSSID
	for (i = 0; i < IFGROUP_NUM - 1; i++) {
		if (!mib_chain_get(MIB_MBSSIB_TBL, i + 1, &mbssidEntry) ||
				mbssidEntry.wlanDisabled)
			continue;

		mib_get(MIB_WLAN_VAP0_ITF_GROUP + (i << 1), &mygroup);
		if (mygroup == grpnum) {
			info[num].ifdomain = DOMAIN_WLAN;
			info[num].ifid = i + 1;
			sprintf(info[num].name, "wlan0-vap%d", i);
			if (++num >= len)
				return num;
		}
	}

#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	for (i = 0; i < IFGROUP_NUM - 1; i++) {
		if (!mib_chain_get(MIB_WLAN1_MBSSIB_TBL, i + 1, &mbssidEntry) ||
				mbssidEntry.wlanDisabled)
			continue;

		mib_get(MIB_WLAN1_VAP0_ITF_GROUP + (i << 1), &mygroup);
		if (mygroup == grpnum) {
			info[num].ifdomain = DOMAIN_WLAN;
			info[num].ifid = i + 11;
			sprintf(info[num].name, "wlan1-vap%d", i);
			if (++num >= len)
				return num;
		}
	}
#endif
#endif // WLAN_MBSSID
	wlan_idx = ori_wlan_idx;
#endif // WLAN_SUPPORT

#ifdef CONFIG_USB_ETH
	// usb0
	mib_get(MIB_USBETH_ITF_GROUP, &mygroup);
	if (mygroup == grpnum) {
		info[num].ifdomain = DOMAIN_ULAN;
		info[num].ifid = 0;
		strncpy(info[num].name, (char *)USBETHIF, sizeof(info[num].name));
		if (++num >= len)
			return num;
	}
#endif

	// vc
	vcNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i = 0; i < vcNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, &pvcEntry)) {
			printf("Get chain record error!\n");
			return -1;
		}

		if (pvcEntry.enable == 0 || pvcEntry.itfGroupNum != grpnum
		    || !isValidMedia(pvcEntry.ifIndex)
#ifdef CONFIG_RTK_RG_INIT // Rostelecom, Port Binding function
#else
		    || ( pvcEntry.cmode != CHANNEL_MODE_BRIDGE && pvcEntry.brmode == BRIDGE_DISABLE)
#endif
		    )
			continue;

		info[num].ifdomain = DOMAIN_WAN;
		info[num].ifid = pvcEntry.ifIndex;
		ifGetName(pvcEntry.ifIndex, info[num].name,
			  sizeof(info[num].name));
		if (++num >= len)
			return num;
	}

	return num;
}

#endif

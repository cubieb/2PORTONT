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

/* =============================================================================================================*/

#if defined(VLAN_GROUP)
// enable/disable VLAN-Grouping
void __dev_setupVlanGrouping(int flag)
{
	struct ifreq ifr;
	struct ifvlan ifvl;

	strcpy(ifr.ifr_name, ELANIF);
	ifr.ifr_data = (char *)&ifvl;
	ifvl.cmd=VLAN_SETVLANGROUPING;
	ifvl.enable=flag;
	setVlan(&ifr);
	printf("Port/VLAN mapping: %s\n", flag?"enabled":"disabled");
}

void setupEth2pvc_vlan()
{
#ifdef EMBED
	struct ifreq ifr;
	struct ifvlan ifvl;
	int i, k, mbr, maskbit;
	MIB_CE_VLAN_T vlan_entry;
	MIB_CE_SW_PORT_T sw_entry;
	unsigned char mode;

	mib_get(MIB_MPMODE, (void *)&mode);
#ifdef IP_QOS_VPORT
	//if (mode&0x07) // IPQ, PortMapping and IGMP-snooping need to register virtual LAN ports by only once
	if (mode&(MP_PMAP_MASK | MP_IPQ_MASK | MP_IGMP_MASK)) // IPQ, PortMapping and IGMP-snooping need to register virtual LAN ports by only once
#else
	//if (mode&0x05)	// PortMapping and IGMP-snooping need to register virtual LAN ports
	if (mode&(MP_PMAP_MASK | MP_IGMP_MASK)) // PortMapping and IGMP-snooping need to register virtual LAN ports by only once
#endif
	if (!virtual_port_enabled)
		__dev_register_swport();

	if (!(mode&MP_PMAP_MASK))
		return;
	strcpy(ifr.ifr_name, ELANIF);
	// set VLAN G(group1), H(group2), I(group3), J(group4)
	// VLANF(group0) is default group
	// VLAN A, B, C, D, E are used for LAN port identification
	ifvl.cmd=VLAN_SETINFO;
	//------------ set default group ---------------
	ifvl.vlanIdx=SW_PORT_NUM+1; // don't touch sw_port and cpu port
	ifvl.vid=0;
	ifvl.member = 0xffffffff;
	ifr.ifr_data = (char *)&ifvl;
	setVlan(&ifr);
	//------------ set user-defined group ----------
	for (i=1; i<VLAN_NUM; i++) {
		mib_chain_get(MIB_VLAN_TBL, i, (void *)&vlan_entry);
		ifvl.vlanIdx=SW_PORT_NUM+1+i; // don't touch sw_port and cpu port
		// VID 0 is used to identify priority frames and VID 4095(0xFFF) is reserved
		ifvl.vid=vlan_entry.tag;
		ifvl.member = vlan_entry.member; //mbr; // member ports:
		ifvl.member |= 0x10; // Set CPU as a member of each vlan.
		ifr.ifr_data = (char *)&ifvl;
		//printf("vlanidx: %d, vid: %d, mbr: 0x%x\n", ifvl.vlanIdx, ifvl.vid, ifvl.member);
		setVlan(&ifr);
	}

	//------------ Set LAN pvid ---------------
	ifvl.cmd=VLAN_SETPVIDX;
	for (i=0; i<SW_PORT_NUM; i++) {
		// set sw-port 0~3 vlan idex.
		mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&sw_entry);
		ifvl.port=SW_PORT_NUM+1+i; // don't touch sw_port and cpu port
		ifvl.vlanIdx=SW_PORT_NUM+1+sw_entry.pvid;
		ifr.ifr_data = (char *)&ifvl;
		//printf("port: %d, pvid: %d\n", ifvl.port, ifvl.vlanIdx);
		setVlan(&ifr);
	}

	//------------ enable vlan-grouping -----------
	__dev_setupVlanGrouping(1);
	//------------ enable 8305 vlan -----------
	setup_8305_vlan_enable();
#endif	// of EMBED
}
void setupEth2pvc_vlan_disable()
{
	unsigned char vChar=0;

	// disable VLAN groupping
	__dev_setupVlanGrouping(0);

	// disable 8305 vlan
	mib_get( MIB_MPMODE, (void *)&vChar);
	// If one the IGMP Snooping, IPQoS and Port Mapping is enabled, we  can not disable VLAN.
	if ( vChar == 0) {
		printf("Disable switch VLAN.\n");
		setup_8305_vlan_disable();
		__dev_setupVirtualPort(0);
	}

	printf("Port/VLAN mapping stopped\n");
}

// membership bit-map:
// | resvd | vc7 vc6 vc5 vc4 | vc3 vc2 vc1 vc0 |
//  vap3 | vap2 vap1 vap0 wlan0 | resvd | LAN4 LAN3 LAN2 LAN1
void setgroup(char *list, int grpnum, int vlan_vid, int vlan_dhcps, int vlan_igmp)
{
	int itfid, itfdomain;
	int i, vlan_num, vc_num;
	char *arg0, *token;
	unsigned int member, maskbit;
	MIB_CE_VLAN_T vlan_entry;
	MIB_CE_ATM_VC_T vc_entry;
	MIB_CE_SW_PORT_T sw_entry;

	vlan_num = mib_chain_total(MIB_VLAN_TBL);
	if (grpnum <= 0 && grpnum >=vlan_num)
		return;
	mib_chain_get(MIB_VLAN_TBL, grpnum, (void *)&vlan_entry);
	// reset pvid
	maskbit = 1;
	for (i=0; i<4; i++) {
		if (vlan_entry.member & maskbit) {
			mib_chain_get(MIB_SW_PORT_TBL, i, (void *)&sw_entry);
			sw_entry.pvid = 0;
			mib_chain_update(MIB_SW_PORT_TBL, (void *)&sw_entry, i);
		}
		maskbit<<=1;
	}
	vlan_entry.member = 0;
	vlan_entry.tag = vlan_vid;
	vlan_entry.dhcps = vlan_dhcps;
	vlan_entry.igmp = vlan_igmp;
	vc_num = mib_chain_total(MIB_ATM_VC_TBL);

	member = 0;
	arg0=list;
	while ((token=strtok(arg0,","))!=NULL) {
		itfid = atoi(token);
		itfdomain = IF_DOMAIN(itfid);
		itfid = IF_INDEX(itfid);

		if (itfdomain == DOMAIN_ELAN) {
			// VLAN membershipt
			vlan_entry.member |= 1<<itfid;
			// set pvid
			mib_chain_get(MIB_SW_PORT_TBL, itfid, (void *)&sw_entry);
			sw_entry.pvid = grpnum;
			mib_chain_update(MIB_SW_PORT_TBL, (void *)&sw_entry, itfid);
		}
		else if (itfdomain == DOMAIN_WAN) {
			for (i=0; i<vc_num; i++) {
				mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&vc_entry);
				if (vc_entry.enable && isValidMedia(vc_entry.ifIndex) && VC_INDEX(vc_entry.ifIndex)==VC_INDEX(itfid)) {
					if (vc_entry.cmode == CHANNEL_MODE_BRIDGE ||
						vc_entry.cmode == CHANNEL_MODE_IPOE ||
						vc_entry.cmode == CHANNEL_MODE_PPPOE)
						vlan_entry.member |= (1<<16)<<VC_INDEX(itfid);
				}
			}
		}
#ifdef WLAN_SUPPORT
		else if (itfdomain == DOMAIN_WLAN) {
			vlan_entry.member |= 0x0100<<itfid;
		}
#endif
		arg0=0;
	}
	//printf("vlan_entry %d, member=0x%x, tag=%d\n", grpnum, vlan_entry.member, vlan_entry.tag);
	mib_chain_update(MIB_VLAN_TBL, (void *)&vlan_entry, grpnum);
}

/*------------------------------------------------------------------
 * Get a list of interface info. (itfInfo) of group.
 * where,
 * info: a list of interface info entries
 * len: max length of the info list
 * grpnum: group id
 *-----------------------------------------------------------------*/
int get_group_ifinfo(struct itfInfo *info, int len, int grpnum)
{
	unsigned int swNum, vcNum;
	int i, k, num;
	unsigned int dft_member, maskbit;
	char mygroup;
	char strlan[]="LAN0", strPPP[]="ppp0", strvc[]="vc0";
	MIB_CE_ATM_VC_T pvcEntry;
	MIB_CE_VLAN_T vlan_entry;

	if (grpnum == 0) { // default group
		#ifdef WLAN_SUPPORT
		dft_member = 0x010f;
		#else
		dft_member = 0x0f;
		#endif
		// LAN -- get default LAN members
		for (k=1; k<VLAN_NUM; k++) {
			maskbit = 1;
			mib_chain_get(MIB_VLAN_TBL, k, (void *)&vlan_entry);
			// ELAN
			for (i=0; i<4; i++) {
				if (vlan_entry.member & maskbit) {
					dft_member &= (~maskbit);
				}
				maskbit<<=1;
			}
			#ifdef WLAN_SUPPORT
			// WLAN
			maskbit = 0x0100;
			if (vlan_entry.member & maskbit)
				dft_member &= (~maskbit);
			#endif
		}
		// assign default LAN info.
		num = 0;
		// ELAN
		maskbit = 1;
		for (i=0; i<4; i++) {
			if (num > len)
				break;
			if (dft_member & maskbit) {
				info[num].ifdomain = DOMAIN_ELAN;
				info[num].ifid=i;
				strlan[3] = '0'+virt2user[i];
				strncpy(info[num].name, strlan, 8);
				num++;
			}
			maskbit<<=1;
		}
		#ifdef WLAN_SUPPORT
		// WLAN
		maskbit = 0x0100;
		if (dft_member & maskbit) {
			info[num].ifdomain = DOMAIN_WLAN;
			info[num].ifid=0;
			strncpy(info[num].name, (char *)WLANIF[0], 8);
			num++;
		}
		#endif

		// WAN -- get default WAN members
		vcNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (i=0; i<vcNum; i++) {
			mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry);
			if (pvcEntry.enable && isValidMedia(pvcEntry.ifIndex)) {
				if (pvcEntry.cmode == CHANNEL_MODE_BRIDGE ||
					pvcEntry.cmode == CHANNEL_MODE_IPOE ||
					pvcEntry.cmode == CHANNEL_MODE_PPPOE)
					dft_member |= (1<<16)<<VC_INDEX(pvcEntry.ifIndex);
			}
		}
		// assign default WAN info.
		maskbit = 0x10000;
		for (i=0; i<8; i++) {
			if (num > len)
				break;
			if (dft_member & maskbit) {
				info[num].ifdomain = DOMAIN_WAN;
				info[num].ifid=(0xf0|i);
				strvc[2] = '0'+i;
				strncpy(info[num].name, strvc, 8);
				num++;
			}
			maskbit<<=1;
		}

		return num;
	}
	if (grpnum <=0 && grpnum >=VLAN_NUM)
		return 0;

	mib_chain_get(MIB_VLAN_TBL, grpnum, (void *)&vlan_entry);
	num = 0;
	// LAN
	maskbit = 1;
	for (i=0; i<4; i++) {
		if (num > len)
			break;
		if (vlan_entry.member & maskbit) {
			info[num].ifdomain = DOMAIN_ELAN;
			info[num].ifid=i;
			strlan[3] = '0'+virt2user[i];
			strncpy(info[num].name, strlan, 8);
			num++;
		}
		maskbit<<=1;
	}
	#ifdef WLAN_SUPPORT
	// WLAN
	maskbit = 0x0100;
	if (vlan_entry.member & maskbit) {
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid=0;
		strncpy(info[num].name, (char *)WLANIF[0], 8);
		num++;
	}
	#endif
	#ifdef WLAN_MBSSID
	#endif

	// WAN
	maskbit = 0x10000;
	for (i=0; i<8; i++) {
		if (num > len)
			break;
		if (vlan_entry.member & maskbit) {
			info[num].ifdomain = DOMAIN_WAN;
			info[num].ifid=(0xf0|i);
			strvc[2] = '0'+i;
			strncpy(info[num].name, strvc, 8);
			num++;
		}
		maskbit<<=1;
	}

	return num;
}

void vg_setup_iptables()
{
	MIB_CE_VLAN_T vlan_entry;
	int i, k;
	unsigned int maskbit;
	unsigned char mode;

	va_cmd(IPTABLES, 2, 1, "-F", "itfgroup");
	va_cmd(EBTABLES, 2, 1, "-F", "itfgroup_in");
	va_cmd(EBTABLES, 2, 1, "-F", "itfgroup_fw");
	mib_get(MIB_MPMODE, (void *)&mode);
	if (!(mode&MP_PMAP_MASK))
		return;

	for (i=1; i<VLAN_NUM; i++) {
		mib_chain_get(MIB_VLAN_TBL, i, (void *)&vlan_entry);
		if (!vlan_entry.dhcps) {
			// Disable dhcp service for LAN port in this group.
			maskbit = 1;
			for (k=0; k<SW_LAN_PORT_NUM; k++) {
				// find the virtual LAN ports in this group
				if (vlan_entry.member & maskbit) {
					// iptables -A itfgroup -i eth0_sw0 -p udp --dport 67 -j DROP
					va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "itfgroup", (char *)ARG_I, (char *)SW_LAN_PORT_IF[k], "-p", "udp", (char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
				}
				maskbit<<=1;
			}
			// Disable dhcp service for Wlan port in this group.
			#ifdef WLAN_SUPPORT
			maskbit = 0x0100;
			if (vlan_entry.member & maskbit) {
				// iptables -A itfgroup -i wlan0 -p udp --dport 67 -j DROP
				va_cmd(IPTABLES, 10, 1, (char *)FW_ADD, "itfgroup", (char *)ARG_I, "wlan0", "-p", "udp", (char *)FW_DPORT, (char *)PORT_DHCP, "-j", (char *)FW_DROP);
			}
			#endif
		}
		else { // just in case
			// Device support dhcp service, so block dhcp packet from bridging.
			maskbit = 1;
			for (k=0; k<SW_LAN_PORT_NUM; k++) {
				// find the virtual LAN ports in this group
				if (vlan_entry.member & maskbit) {
					// ebtables -A itfgroup_fw -i eth0_sw0 -p IPv4 --ip-proto 17 --ip-dport 67 -j DROP
					va_cmd(EBTABLES, 12, 1, (char *)FW_ADD, "itfgroup_fw", (char *)ARG_I, (char *)SW_LAN_PORT_IF[k], "-p", "IPv4", "--ip-proto", "17", "--ip-dport", "67", "-j", (char *)FW_DROP);
				}
				maskbit<<=1;
			}
			#ifdef WLAN_SUPPORT
			maskbit = 0x0100;
			if (vlan_entry.member & maskbit) {
				// ebtables -A itfgroup_fw -i wlan0 -p IPv4 --ip-proto 17 --ip-dport 67 -j DROP
				va_cmd(EBTABLES, 12, 1, (char *)FW_ADD, "itfgroup_fw", (char *)ARG_I, "wlan0", "-p", "IPv4", "--ip-proto", "17", "--ip-dport", "67", "-j", (char *)FW_DROP);
			}
			#endif
		}
		if (!vlan_entry.igmp) {
			// Block IGMP packet for LAN port in the group.
			maskbit = 1;
			for (k=0; k<SW_LAN_PORT_NUM; k++) {
				// find the virtual LAN ports in this group
				if (vlan_entry.member & maskbit) {
					// ebtables -A itfgroup_in -i eth0_sw0 -p IPv4 --ip-proto 2 -j DROP
					va_cmd(EBTABLES, 10, 1, (char *)FW_ADD, "itfgroup_in", (char *)ARG_I, (char *)SW_LAN_PORT_IF[k], "-p", "IPv4", "--ip-proto", "2", "-j", (char *)FW_DROP);
					// ebtables -A itfgroup_fw -i eth0_sw0 -p IPv4 --ip-proto 2 -j DROP
					va_cmd(EBTABLES, 10, 1, (char *)FW_ADD, "itfgroup_fw", (char *)ARG_I, (char *)SW_LAN_PORT_IF[k], "-p", "IPv4", "--ip-proto", "2", "-j", (char *)FW_DROP);
				}
				maskbit<<=1;
			}
			// Block IGMP packet for WLAN port in the group.
			#ifdef WLAN_SUPPORT
			maskbit = 0x0100;
			if (vlan_entry.member & maskbit) {
				// ebtables -A itfgroup_in -i wlan0 -p IPv4 --ip-proto 2 -j DROP
				va_cmd(EBTABLES, 10, 1, (char *)FW_ADD, "itfgroup_in", (char *)ARG_I, "wlan0", "-p", "IPv4", "--ip-proto", "2", "-j", (char *)FW_DROP);
				// ebtables -A itfgroup_fw -i wlan0 -p IPv4 --ip-proto 2 -j DROP
				va_cmd(EBTABLES, 10, 1, (char *)FW_ADD, "itfgroup_fw", (char *)ARG_I, "wlan0", "-p", "IPv4", "--ip-proto", "2", "-j", (char *)FW_DROP);
			}
			#endif
		}
	}
}
#endif // of VLAN_GROUP

/*
 *      System routines for Interface Group
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

#ifdef ITF_GROUP
#ifdef CONFIG_USB_ETH
#define SIOCSITFGROUP	0x89b6      // Mason Yu 090903

static void setup_usb_group_member(int mbr)
{
	struct ifreq ifr;
	printf( "setup_usb_group_member> mbr=0x%x\n", mbr );
	strcpy(ifr.ifr_name, USBETHIF);
	ifr.ifr_ifru.ifru_ivalue = mbr;
	do_ioctl( SIOCSITFGROUP, &ifr );
	return;
}
#endif

#ifdef WLAN_SUPPORT
#define SIOSIWRTLITFGROUP		0x8B90

static void setup_wlan_group_member(int itf, int mbr)
{
	int skfd;
	struct iwreq wrq;
	int k;
#ifdef WLAN_MBSSID
	char s_ifname[16];
#endif

	skfd = socket(AF_INET, SOCK_DGRAM, 0);

#ifndef WLAN_MBSSID
	/* Set device name */
	strncpy(wrq.ifr_name, getWlanIfName(), IFNAMSIZ);
	wrq.u.data.flags = 0;
#else
	if( itf == 0 )
	{
		strncpy(wrq.ifr_name, getWlanIfName(), IFNAMSIZ);
		wrq.u.data.flags = 0;
	}
	else
	{
		snprintf(s_ifname, 16, "wlan%d-vap%d", wlan_idx, itf-WLAN_VAP_ITF_INDEX);  // ex: wlan0-vap0
		strncpy(wrq.ifr_name, s_ifname, IFNAMSIZ);
	}

	wrq.u.data.flags = itf;

#endif

	wrq.u.data.pointer = (caddr_t)&mbr;
	wrq.u.data.length = sizeof(mbr);

	ioctl(skfd, SIOSIWRTLITFGROUP, &wrq);
	close( skfd );
}
#endif // of WLAN_SUPPORT


/*------------------------------------------------------------------------
 *	Setup RTL8305 Vlan membership.
 *	Enable port-based Vlan and set the vlan membership with
 *	the following pre-requisite:
 *
 *	port vlanIdx 	Vlan	VID
 *	 0     0       A    0
 *	 1     1       B    1
 *	 2     2       C    2
 *	 3     3       D    3
 *
 *------------------------------------------------------------------------*/
static void setup_8305_vlan_eth_member(int port, int mbr)
{
	int k;
	struct ifreq ifr;
	struct ifvlan ifvl;

	strcpy(ifr.ifr_name, ELANIF);
	ifr.ifr_data = (char *)&ifvl;

	// set 8305 port-based vlan members
	// port		vlanIdx		Vlan	VID
	// 0		0		A	0
	// 1		1		B	1
	// 2		2		C	2
	// 3		3		D	3
	ifvl.cmd=VLAN_SETINFO;

	// This port is a member ?
	#ifdef CONFIG_ETHWAN
	if (port == ETHWAN_PORT || mbr & (1<<port)) {
	#else
	if (mbr & (1<<port)) {
	#endif
		// set the membership
		ifvl.vlanIdx=port;
		// VID 0 is used to identify priority frames and VID 4095(0xFFF) is reserved
		ifvl.vid=port+1;
		ifvl.member=mbr;

#ifdef CONFIG_USB_ETH
		ifvl.member|=( mbr&(1<<IFUSBETH_SHIFT) );
#endif //CONFIG_USB_ETH

		setVlan(&ifr);
		TRACE(STA_SCRIPT, "RTL8305 VLAN: vlanIdx=%d, vid=%d, member=0x%.2x\n", ifvl.vlanIdx, ifvl.vid, ifvl.member);
	} else {
		// Added by Mason Yu
		// set the membership
		ifvl.vlanIdx=port;
		// VID 0 is used to identify priority frames and VID 4095(0xFFF) is reserved
		ifvl.vid=port+1;

#ifdef CONFIG_USBCLIENT
		ifvl.member=0x3f;
#else
		ifvl.member=0x1f;
#endif
		setVlan(&ifr);
	}
}

void __dev_setupPortMapping(int flag)
{
	struct ifreq ifr;
	struct ifvlan ifvl;

	strcpy(ifr.ifr_name, ELANIF);
	ifr.ifr_data = (char *)&ifvl;
	ifvl.cmd=VLAN_SETPORTMAPPING;
	ifvl.enable=flag;
	setVlan(&ifr);
	//printf("Port Mapping: %s\n", flag?"enabled":"disabled");
}

void setgroup(char *list, int grpnum)
{
	int itfid, itfdomain;
	int i, num;
	char *arg0, *token;
	int mib_wlan_num;

	arg0=list;
	while ((token=strtok(arg0,","))!=NULL) {
		itfid = atoi(token);
		itfdomain = IF_DOMAIN(itfid);
		itfid = IF_INDEX(itfid);

#if defined(ITF_GROUP_4P) || !defined(CONFIG_RTL8672NIC)
		if (itfdomain == DOMAIN_ELAN) {
			MIB_CE_SW_PORT_T Entry;

			num = mib_chain_total(MIB_SW_PORT_TBL);
			if (itfid < num) {
				if (!mib_chain_get(MIB_SW_PORT_TBL, itfid, (void *)&Entry))
					return;
				Entry.itfGroup = grpnum;
				mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, itfid);
			}
		}
#elif defined(ITF_GROUP_1P)
		if (itfdomain == DOMAIN_ELAN) {
			char mygroup;
			mygroup = (char)grpnum;
			if (itfid == 0)
				mib_set(MIB_ETH_ITF_GROUP, (void *)&mygroup);
		}
#endif

#ifdef WLAN_SUPPORT
		else if (itfdomain == DOMAIN_WLAN) {
			char mygroup;
			int ori_wlan_idx = wlan_idx;

			mygroup = (char)grpnum;
			wlan_idx = 0;

			if (itfid == 0)
				mib_set(MIB_WLAN_ITF_GROUP, (void *)&mygroup);

#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
			if (itfid == 10)
				mib_set(MIB_WLAN1_ITF_GROUP, (void *)&mygroup);
#endif

			// Added by Mason Yu
#ifdef WLAN_MBSSID
			for ( i=1; i<IFGROUP_NUM; i++) {
				if (itfid == i) {// wlan0_vap0
					mib_set(MIB_WLAN_VAP0_ITF_GROUP + ((i-1)<<1), (void *)&mygroup);

				}
			}

#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
			for ( i = 11; i < 10+IFGROUP_NUM; i++)
				if (itfid == i) // wlan1_vap0
					mib_set(MIB_WLAN1_VAP0_ITF_GROUP + ((i-11)<<1), (void *)&mygroup);
#endif
#endif // WLAN_MBSSID
			wlan_idx = ori_wlan_idx;
		}
#endif // WLAN_SUPPORT

#ifdef CONFIG_USB_ETH
		else if (itfdomain == DOMAIN_ULAN) {
			char mygroup;
			mygroup = (char)grpnum;
			if (itfid == 0)
				mib_set(MIB_USBETH_ITF_GROUP, (void *)&mygroup);
		}
#endif

		else if (itfdomain == DOMAIN_WAN) {
			MIB_CE_ATM_VC_T Entry;

			num = mib_chain_total(MIB_ATM_VC_TBL);
			for (i=0; i<num; i++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
					return;
				if (Entry.enable && isValidMedia(Entry.ifIndex) && Entry.ifIndex==itfid) {
					Entry.itfGroup = grpnum;
					mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
				}
			}
		}
		arg0=0;
	}
}

/*
 * Get interface group membership(bit mapped)
 */
static void get_group_membership(int *mbr)
{
	MIB_CE_SW_PORT_T Port;
	int i, k, total, num;
	char grpid;
	int ori_wlan_idx;
#ifndef CONFIG_RTL8672NIC
	MIB_CE_ATM_VC_T pvcEntry;
	MEDIA_TYPE_T mType;
#endif

	// flush membership
	for (k=0; k<IFGROUP_NUM; k++)
		mbr[k] = 0;

	// Ethernet LAN
#if defined(ITF_GROUP_4P) || !defined(CONFIG_RTL8672NIC)
	num = mib_chain_total(MIB_SW_PORT_TBL);
	for (k=0; k<num; k++) {
#if !defined(CONFIG_RTL8672NIC) && defined(CONFIG_ETHWAN)
		if(k == ETHWAN_PORT)//port 0 is unused for lan switch
			continue;
#endif
		if (!mib_chain_get(MIB_SW_PORT_TBL, k, (void *)&Port))
			continue;
		if (Port.itfGroup < IFGROUP_NUM)
			mbr[Port.itfGroup] |= (1<<k);
	}
#else
	mib_get(MIB_ETH_ITF_GROUP, (void *)&grpid);
	if (grpid < IFGROUP_NUM)
		mbr[grpid] |= 1;
#endif

#ifndef CONFIG_RTL8672NIC
	/* set membership into vc */
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<total; i++) {
		int vcIdx;

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;
		if (!pvcEntry.enable || !isValidMedia(pvcEntry.ifIndex))
			continue;
		#ifdef CONFIG_ETHWAN
		mType = MEDIA_INDEX(pvcEntry.ifIndex);
		if (mType == MEDIA_ETH) {/*QL if multi eth wan enable, wan port should be distinguished. */
			mbr[pvcEntry.itfGroup]|=(1<<ETHWAN_PORT);
		}
		#endif
	}
#endif

	// Wireless
	#ifdef WLAN_SUPPORT
	ori_wlan_idx = wlan_idx;
	wlan_idx = 0;
	mib_get(MIB_WLAN_ITF_GROUP, (void *)&grpid);
	if (grpid < IFGROUP_NUM)
		mbr[grpid] |= (1<<IFWLAN_SHIFT);

	#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	mib_get(MIB_WLAN1_ITF_GROUP, (void *)&grpid);
	if (grpid < IFGROUP_NUM)
		mbr[grpid] |= (1<<IFWLAN1_SHIFT);
	#endif

	#ifdef WLAN_MBSSID
	for ( k=1; k<IFGROUP_NUM; k++ ) {
		mib_get(MIB_WLAN_VAP0_ITF_GROUP + ((k-1)<<1), (void *)&grpid);
		if (grpid < IFGROUP_NUM)
			mbr[grpid] |= (1<<(IFWLAN_SHIFT+k));
	}

	#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	for ( k=1; k<IFGROUP_NUM; k++ )
	{
		mib_get(MIB_WLAN1_VAP0_ITF_GROUP + ((k-1)<<1), (void *)&grpid);
		if (grpid < IFGROUP_NUM)
			mbr[grpid] |= (1<<(IFWLAN1_SHIFT+k));
	}
	#endif
	#endif // WLAN_MBSSID
	wlan_idx = ori_wlan_idx;
	#endif // WLAN_SUPPORT

	// USB-Eth
	#ifdef CONFIG_USB_ETH
	mib_get(MIB_USBETH_ITF_GROUP, (void *)&grpid);
	if (grpid < IFGROUP_NUM) {
		mbr[grpid] |= (1<<IFUSBETH_SHIFT);
	}  // Mason Yu 090903
	#endif

	// device is mandatory
	for (k=0; k<IFGROUP_NUM; k++)
		mbr[k] |= (1<<DEVICE_SHIFT);
}

static void set_group_membership(int *mbr)
{
	MIB_CE_ATM_VC_T pvcEntry;
	struct data_to_pass_st msg;
	char wanif[5];
	int i, k, total;
#ifdef WLAN_SUPPORT
	int ori_wlan_idx;
#endif
	MEDIA_TYPE_T mType;

	/* set membership into vc */
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<total; i++) {
		int vcIdx;

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;
		if (!pvcEntry.enable || !isValidMedia(pvcEntry.ifIndex))
			continue;

		mType = MEDIA_INDEX(pvcEntry.ifIndex);
		if ( mType == MEDIA_ATM) {
#ifdef CONFIG_RTL8672_SAR
			vcIdx = VC_INDEX(pvcEntry.ifIndex);
			snprintf(wanif, 5, "vc%d", vcIdx);
			// mpoactl set vc0 group 1 member 3
			snprintf(msg.data, BUF_SIZE,
				"mpoactl set %s group 1 member %d",
				wanif, mbr[pvcEntry.itfGroup]);
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_mpoad(&msg);
#endif
		}
		#ifdef CONFIG_ETHWAN
		else if (mType == MEDIA_ETH) {
#ifdef CONFIG_HWNAT
#ifndef CONFIG_RTL_MULTI_ETH_WAN
			setup_hwnat_eth_member(ETHWAN_PORT, mbr[pvcEntry.itfGroup], 1);
#else
			setup_hwnat_eth_member(pvcEntry.ifIndex, mbr[pvcEntry.itfGroup], 1);
#endif
			printf("set wan port 0 member: %x\n", mbr[pvcEntry.itfGroup]);
#else
			setup_8305_vlan_eth_member(ETHWAN_PORT, mbr[pvcEntry.itfGroup]);
#endif
		}
		#endif
		else
			continue;
	}

#if defined(ITF_GROUP_4P) || !defined(CONFIG_RTL8672NIC)
	total = mib_chain_total(MIB_SW_PORT_TBL);
#else
	total = 1;
#endif
	for (i=0; i<IFGROUP_NUM; i++)
	{
		for (k=0; k<MAX_NUM_OF_ITFS; k++)
		{
			if (mbr[i]&(1<<k)) // port k in group i
			{

				if (k >= 0 && k < total){ // eth LAN port
#ifndef CONFIG_RTL_MULTI_ETH_WAN
#ifdef CONFIG_ETHWAN
					if(k != ETHWAN_PORT) //wan port has been set in MIB_ATM_VC_TBL
#endif
#ifdef CONFIG_HWNAT
					setup_hwnat_eth_member(k, mbr[i], 1);
#else
					setup_8305_vlan_eth_member(k, mbr[i]);
#endif
#endif
				}

				#ifdef WLAN_SUPPORT
				ori_wlan_idx = wlan_idx;

				if(k >= IFWLAN_SHIFT && k <= (IFWLAN_SHIFT+WLAN_MBSSID_NUM)) // wlan port
				{
					//printf("group=%d, ", i);
					wlan_idx = 0;
					setup_wlan_group_member(k-IFWLAN_SHIFT, mbr[i]);
				}
				else if(k >= IFWLAN1_SHIFT && k <= (IFWLAN1_SHIFT+WLAN_MBSSID_NUM))
				{
					//printf("group=%d, ", i);
					wlan_idx = 1;
					setup_wlan_group_member(k-IFWLAN1_SHIFT, mbr[i]);
				}

				wlan_idx = ori_wlan_idx;
				#endif

				#ifdef CONFIG_USB_ETH
				if (k == IFUSBETH_SHIFT) // usb-eth port
					setup_usb_group_member( mbr[i] );
				#endif
			}
		}
	}
}

/*
 * Setup interface group membership(bit mapped)
 * The bitmap for interface should follow this convention commonly shared
 * by sar driver (sar_send())
 * Bit-map:
 *  bit16|bit15|bit14|bit13|bit12|(bit11)|bit10|bit9|bit8|bit7|bit6 |bit5 |bit4  |bit3|bit2|bit1|bit0
 *  vap3 |vap2 |vap1 |vap0 |wlan1|(usb0) |vap3 |vap2|vap1|vap0|wlan0|resvd|device|lan3|lan2|lan1|lan0
 * If usb(switch port) add:
 *  bit16|bit15|bit14|bit13|bit12|(bit11)|bit10|bit9|bit8|bit7|bit6 |bit5  |bit4     |bit3|bit2|bit1|bit0
 *  vap3 |vap2 |vap1 |vap0 |wlan1|(usb0) |vap3 |vap2|vap1|vap0|wlan0|device|lan4(usb)|lan3|lan2|lan1|lan0
  * If 8676HW NAT add:
 *  bit16|bit15|bit14|bit13|bit12|(bit11)|bit10|bit9|bit8|bit7|bit6 |bit5  |bit4     |bit3|bit2|bit1|bit0
 *  vap3 |vap2 |vap1 |vap0 |wlan1|(usb0) |vap3 |vap2|vap1|vap0|wlan0|device|lan4|lan3|lan2|lan1|lan0

 */
void setupEth2pvc()
{
	int mbr[IFGROUP_NUM];

	get_group_membership(mbr);
	set_group_membership(&mbr[0]);
	// enable Port Mapping
	__dev_setupPortMapping(1);
	// enable switch vlan
#ifdef ITF_GROUP_4P
	setup_8305_vlan_enable();
#endif
	printf("Port Mapping: Ethernet to pvc mapping started\n");
}

void setupEth2pvc_disable()
{
	MIB_CE_ATM_VC_T pvcEntry;
	int i, k, total, num, member;
	struct data_to_pass_st msg;
	char wanif[5];
	unsigned char vChar=0;
	MEDIA_TYPE_T mType;

	// Mason Yu
	// If we disable Port Mapping, we should set all interface's membership to '0'.
	member=0;

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<total; i++) {
		int vcIdx;

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
			continue;
		if (!pvcEntry.enable || !isValidMedia(pvcEntry.ifIndex))
			continue;

		mType = MEDIA_INDEX(pvcEntry.ifIndex);
		if ( mType == MEDIA_ATM) {
#ifdef CONFIG_RTL8672_SAR
			/* set membership into vc */
			vcIdx = VC_INDEX(pvcEntry.ifIndex);
			snprintf(wanif, 5, "vc%d", vcIdx);
			// mpoactl set vc0 group 1 member 3
			snprintf(msg.data, BUF_SIZE,
				"mpoactl set %s group 1 member %d",
				wanif, member);
			TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_mpoad(&msg);
#endif
		}
#ifdef CONFIG_ETHWAN
		else if (mType == MEDIA_ETH) {
#ifndef CONFIG_RTL_MULTI_ETH_WAN
			setup_8305_vlan_eth_member(ETHWAN_PORT, member);
#else
#ifdef CONFIG_HWNAT
			setup_hwnat_eth_member(pvcEntry.ifIndex, 0, 0);
#endif
#endif
		}
#endif
		else
			continue;
	}

	/* set membership 0 into LAN ports */
	// Mason Yu. combine_1p_4p_PortMapping
#ifdef CONFIG_HWNAT
#ifndef CONFIG_RTL_MULTI_ETH_WAN
	setup_hwnat_eth_member(0, 0, 0);
#endif
#else
#ifdef ITF_GROUP_4P
	num = mib_chain_total(MIB_SW_PORT_TBL);
#ifdef CONFIG_ETHWAN
	num--;//skip wan port
#endif

	for (k=0; k<num; k++) {
		setup_8305_vlan_eth_member(k, member);
	}
#elif defined(ITF_GROUP_1P)
	setup_8305_vlan_eth_member(0, member);
#endif
#endif

	#ifdef WLAN_SUPPORT
	for (k=0; k<=WLAN_MBSSID_NUM; k++)
		setup_wlan_group_member(k, member);
	#endif

	#ifdef CONFIG_USB_ETH
	/* set membership 0 into usb0 */
	setup_usb_group_member( member );
	#endif //CONFIG_USB_ETH

	// disable Port Mapping
	__dev_setupPortMapping(0);


	// disable 8305 vlan
	// Mason Yu. combine_1p_4p_PortMapping
	#ifdef ITF_GROUP_4P
	mib_get( MIB_MPMODE, (void *)&vChar);
	// If one the IGMP Snooping, IPQoS and Port Mapping is enabled, we  can not disable VLAN.
	if ( vChar == 0) {
		setup_8305_vlan_disable();
		__dev_setupVirtualPort(0);
	}
	#endif
	printf("Port Mapping: Ethernet to pvc mapping stopped\n");
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
	int i, num;
	int mib_wlan_num;
	char mygroup;
	char strlan[]="LAN0", strPPP[]="ppp0", strvc[]="vc0";
	MIB_CE_SW_PORT_T Entry;
	MIB_CE_ATM_VC_T pvcEntry;
	int ori_wlan_idx;

#if defined(ITF_GROUP_4P) || !defined(CONFIG_RTL8672NIC)
	// LAN ports
	swNum = mib_chain_total(MIB_SW_PORT_TBL);
	num=0;
	for (i=swNum; i>0; i--) {
		if (!mib_chain_get(MIB_SW_PORT_TBL, i-1, (void *)&Entry))
			return -1;
		if (Entry.itfGroup == grpnum) {
			strlan[3] = '0'+virt2user[i-1];  // LAN1, LAN2, LAN3, LAN4
			info[num].ifdomain = DOMAIN_ELAN;
			info[num].ifid=i-1;
			#ifdef CONFIG_ETHWAN
			if((i - 1) == ETHWAN_PORT){
				continue;
			}
			#endif
			strncpy(info[num].name, strlan, 8);
			num++;
		}
		if (num > len)
			break;
	}
#elif defined(ITF_GROUP_1P)
	//eth0
	// Mason Yu
	num=0;
	mib_get(MIB_ETH_ITF_GROUP, (void *)&mygroup);
	if (mygroup == grpnum) {
		info[num].ifdomain = DOMAIN_ELAN;
		info[num].ifid=0;
		strcpy(info[num].name, (char *)ELANIF);
		num++;
	}
#endif

#ifdef WLAN_SUPPORT
	// wlan0
	ori_wlan_idx = wlan_idx;
	wlan_idx = 0;  // Magician: mib_get will add wlan_idx to mib id. Therefore wlan_idx must be set to 0.
	mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
	if (mygroup == grpnum) {
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid=0;
		strncpy(info[num].name, (char *)WLANIF[wlan_idx], 8);
		num++;
	}

// wlan1
#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	wlan_idx = 1;
	mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);

	if (mygroup == grpnum)
	{
		info[num].ifdomain = DOMAIN_WLAN;
		info[num].ifid = 10;
		strncpy(info[num].name, (char *)WLANIF[wlan_idx], 8);
		num++;
	}
#endif

	wlan_idx = ori_wlan_idx;
#endif // WLAN_SUPPORT

#ifdef WLAN_MBSSID
	ori_wlan_idx = wlan_idx;
	wlan_idx = 0;  // Magician: mib_get will add wlan_idx to mib id. Therefore wlan_idx must be set to 0.
	for (i=1; i<IFGROUP_NUM; i++) {
		mib_get(MIB_WLAN_VAP0_ITF_GROUP + ((i-1)<<1), (void *)&mygroup);
		if (mygroup == grpnum) {
			info[num].ifdomain = DOMAIN_WLAN;
			info[num].ifid=i;
			sprintf(info[num].name, "%s-vap%d", (char *)WLANIF[wlan_idx] ,i-1);
			num++;
		}
	}

#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
	wlan_idx = 1;
	for (i=11; i<10+IFGROUP_NUM; i++)
	{
		mib_get(MIB_WLAN_VAP0_ITF_GROUP + ((i-11)<<1), (void *)&mygroup);
		if (mygroup == grpnum) {
			info[num].ifdomain = DOMAIN_WLAN;
			info[num].ifid=i;
			sprintf(info[num].name, "%s-vap%d", (char *)WLANIF[wlan_idx], i-11);
			num++;
		}
	}
#endif

	wlan_idx = ori_wlan_idx;
#endif // WLAN_MBSSID

#ifdef CONFIG_USB_ETH
	// usb0
	mib_get(MIB_USBETH_ITF_GROUP, (void *)&mygroup);
	if (mygroup == grpnum) {
		info[num].ifdomain = DOMAIN_ULAN;
		info[num].ifid=0;
		strcpy(info[num].name, (char *)USBETHIF);
		num++;
	}
#endif

	// vc
	vcNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0; i<vcNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&pvcEntry))
		{
  			//boaError(wp, 400, "Get chain record error!\n");
  			printf("Get chain record error!\n");
			return -1;
		}

		if (pvcEntry.enable == 0 || pvcEntry.itfGroup!=grpnum || !isValidMedia(pvcEntry.ifIndex))
			continue;

		ifGetName(pvcEntry.ifIndex, info[num].name, sizeof(info[num].name));
		info[num].ifdomain = DOMAIN_WAN;
		info[num].ifid=pvcEntry.ifIndex;
		num++;
		if (num > len)
			break;
	}

	return num;
}
#endif // of ITF_GROUP

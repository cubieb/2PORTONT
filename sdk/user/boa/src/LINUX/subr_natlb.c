
#include <string.h>
#include "debug.h"
#include "utility.h"
#include <string.h>

extern const char FW_PREROUTING[];
extern const char FW_POSTROUTING[];

#if defined(PORT_FORWARD_GENERAL) || defined(DMZ)
#ifdef NAT_LOOPBACK
#ifdef PORT_FORWARD_GENERAL
static int iptable_fw_natLB_dynamic( char *ifname, const char *proto, const char *extPort, const char *dstIP, const char *myip, char *nat_pre_name, char *nat_post_name, int fh)
{	
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	unsigned char ipStr[20], subnetStr[20];
	unsigned long mask, mbit, ip, subnet;	
	char lan_ipaddr[32], dstIP2[32], *pStr, buff[128];	
	
	// (1) Get LAN IP and mask
	mib_get(MIB_ADSL_LAN_IP, (void *)&ip);
	strncpy(ipStr, inet_ntoa(*((struct in_addr *)&ip)), 16);
	ipStr[15] = '\0';

	mib_get(MIB_ADSL_LAN_SUBNET, (void *)&mask);
	subnet = (ip&mask);	
	strncpy(subnetStr, inet_ntoa(*((struct in_addr *)&subnet)), 16);
	subnetStr[15] = '\0';

	mbit=0;
	while (1) {
		if (mask&0x80000000) {
			mbit++;
			mask <<= 1;
		}
		else
			break;
	}
	snprintf(lan_ipaddr, sizeof(lan_ipaddr), "%s/%d", subnetStr, mbit);
	
	strncpy(dstIP2, dstIP, sizeof(dstIP2));
	pStr = strstr(dstIP2, ":");
	if (pStr != NULL)
		pStr[0] = '\0';
	
	// (2) Set rule	 for PPP
	if ( myip != NULL && fh==0)	// It is for PPP connection
	{
		// (2.1) Set prerouting rule
		// iptables -t nat -A portfwPreNatLB_ppp0 -s 192.168.1.0/24 -d 192.168.8.1 -p TCP --dport 21 -j DNAT --to-destination 192.168.1.20:21
		if (extPort) {
			va_cmd(IPTABLES, 16, 1, "-t", "nat",
				"-A", nat_pre_name,
				"-s", lan_ipaddr,
				"-d", myip,
				"-p", (char *)proto,
				(char *)FW_DPORT, extPort, "-j",
				"DNAT", "--to-destination", dstIP);
		} else {
			va_cmd(IPTABLES, 14, 1, "-t", "nat",
				"-A", nat_pre_name,
				"-s", lan_ipaddr,
				"-d", myip,
				"-p", (char *)proto,
				"-j", "DNAT", "--to-destination", dstIP);
		}
		
		// (2.2) Set postrouting rule
		//iptables -t nat -A portfwPostNatLB_ppp0 -s 192.168.1.0/24 -d 192.168.1.20 -p TCP --dport 21 -j MASQUERADE	
		if (extPort) {
			va_cmd(IPTABLES, 14, 1, "-t", "nat",
				"-A", nat_post_name,
				"-s", lan_ipaddr,
				"-d", dstIP2,
				"-p", (char *)proto,
				(char *)FW_DPORT, extPort, 				
				"-j",	"MASQUERADE");
		} else {
			va_cmd(IPTABLES, 12, 1, "-t", "nat",
				"-A", nat_post_name,
				"-s", lan_ipaddr,
				"-d", dstIP2,
				"-p", (char *)proto,				
				"-j", "MASQUERADE");
		}
	}
	
	// (3) set rule for DHCP
	if ( myip == NULL && fh != 0)	// It is for DHCP dynamic connection
	{
		// (3.1) Set prerouting rule
		// iptables -t nat -A portfwPreNatLB_ptm0_0 -s 192.168.1.0/24 -d 192.168.8.1 -p TCP --dport 21 -j DNAT --to-destination 192.168.1.20:21
		if (extPort) {			
			snprintf(buff, 128, "\tiptables -t nat -A %s -s %s -d $ip -p %s --dport %s -j DNAT --to-destination %s\n", nat_pre_name, lan_ipaddr, (char *)proto, extPort, dstIP);	
			WRITE_DHCPC_FILE(fh, buff);
		} else {			
			snprintf(buff, 128, "\tiptables -t nat -A %s -s %s -d $ip -p %s -j DNAT --to-destination %s\n", nat_pre_name, lan_ipaddr, (char *)proto, dstIP);	
			WRITE_DHCPC_FILE(fh, buff);
		}
		
		// (3.2) Set postrouting rule
		//iptables -t nat -A portfwPostNatLB_ptm0_0 -s 192.168.1.0/24 -d 192.168.1.20 -p TCP --dport 21 -j MASQUERADE	
		if (extPort) {			
			snprintf(buff, 128, "\tiptables -t nat -A %s -s %s -d %s -p %s --dport %s -j MASQUERADE\n", nat_post_name, lan_ipaddr, dstIP2, (char *)proto, extPort);
			WRITE_DHCPC_FILE(fh, buff);
		} else {			
			snprintf(buff, 128, "\tiptables -t nat -A %s -s %s -d %s -p %s -j MASQUERADE\n", nat_post_name, lan_ipaddr, dstIP2, (char *)proto);
			WRITE_DHCPC_FILE(fh, buff);
		}
	}
	return 0;

}

static void portfw_NATLB_dynamic( MIB_CE_PORT_FW_T *p, char *ifname, char *myip, char *nat_pre_name, char *nat_post_name, int fh)
{
	int hasExtPort=0;
	char * proto = 0;
	char intPort[32], extPort[32];
	char ipaddr[32], extra[32];	
	
	if(p==NULL) return;

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
    }
    else
    {
        snprintf(ipaddr, sizeof(ipaddr), "%s", inet_ntoa(*((struct in_addr *)p->ipAddr)));       
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
	//printf( "portfw_NATLB_dynamic: extPort is %s hasExtPort=%d, myip is %s\n",  extPort, hasExtPort, myip);
	//printf( "entry.externalfromport:%d entry.externaltoport=%d\n",  p->externalfromport, p->externaltoport);	
	
	//check something
	//internalclient can't be zeroip
	if( strncmp(ipaddr,"0.0.0.0", 7)==0 ) return;

	if (p->protoType == PROTO_TCP || p->protoType == PROTO_UDPTCP)
	{		
		iptable_fw_natLB_dynamic( ifname, ARG_TCP, hasExtPort ? extPort : 0, ipaddr, myip, nat_pre_name, nat_post_name, fh);
	}

	if (p->protoType == PROTO_UDP || p->protoType == PROTO_UDPTCP)
	{	
		iptable_fw_natLB_dynamic( ifname, ARG_UDP, hasExtPort ? extPort : 0, ipaddr, myip, nat_pre_name, nat_post_name, fh);
	}
}
#endif

#ifdef DMZ
static int iptable_dmz_natLB_dynamic(char *ifname, char *dstIP, char *myip, char *nat_pre_name, char *nat_post_name, int fh)
{	
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	unsigned char ipStr[20], subnetStr[20];
	unsigned long mask, mbit, ip, subnet;	
	char lan_ipaddr[32], buff[128];	
	
	// (1) Get LAN IP and mask
	mib_get(MIB_ADSL_LAN_IP, (void *)&ip);
	strncpy(ipStr, inet_ntoa(*((struct in_addr *)&ip)), 16);
	ipStr[15] = '\0';

	mib_get(MIB_ADSL_LAN_SUBNET, (void *)&mask);
	subnet = (ip&mask);	
	strncpy(subnetStr, inet_ntoa(*((struct in_addr *)&subnet)), 16);
	subnetStr[15] = '\0';

	mbit=0;
	while (1) {
		if (mask&0x80000000) {
			mbit++;
			mask <<= 1;
		}
		else
			break;
	}
	snprintf(lan_ipaddr, sizeof(lan_ipaddr), "%s/%d", subnetStr, mbit);	
	
	// (2) Set rule	
	if ( myip != NULL && fh==0)	// It is for PPP connection
	{
		// (2.1) Set prerouting rule
		// iptables -t nat -A dmzPreNatLB_ppp0 -s 192.168.1.0/24 -d 192.168.8.1 -j DNAT --to-destination 192.168.1.20		
		va_cmd(IPTABLES, 12, 1, "-t", "nat",
			"-A", nat_pre_name,
			"-s", lan_ipaddr,
			"-d", myip,			
			"-j",	"DNAT", "--to-destination", dstIP);
			
		// (2.2) Set postrouting rule
		//iptables -t nat -A dmzPostNatLB_ppp0 -s 192.168.1.0/24 -d 192.168.1.20  -j  MASQUERADE	
		va_cmd(IPTABLES, 10, 1, "-t", "nat",
			"-A", nat_post_name,
			"-s", lan_ipaddr,
			"-d", dstIP,			
			"-j",	"MASQUERADE");	
	}
	
	// (3) set rule for DHCP
	if ( myip == NULL && fh != 0)	// It is for DHCP dynamic connection
	{
		// (3.1) Set prerouting rule
		// iptables -t nat -A dmzPreNatLB_ptm0_0 -s 192.168.1.0/24 -d 192.168.8.1 -j DNAT --to-destination 192.168.1.20		
		snprintf(buff, 128, "\tiptables -t nat -A %s -s %s -d $ip -j DNAT --to-destination %s\n", nat_pre_name, lan_ipaddr, dstIP);	
		WRITE_DHCPC_FILE(fh, buff);
		
		// (3.2) Set postrouting rule
		//iptables -t nat -A dmzPostNatLB_ptm0_0 -s 192.168.1.0/24 -d 192.168.1.20 -j MASQUERADE		
		snprintf(buff, 128, "\tiptables -t nat -A %s -s %s -d %s -j MASQUERADE\n", nat_post_name, lan_ipaddr, dstIP);
		WRITE_DHCPC_FILE(fh, buff);
	}
}

static int set_dmz_natLB_dynamic(char *ifname, char *myip, char *nat_pre_name, char *nat_post_name, int fh)
{
	int vInt;
	unsigned char value[32];
	char ipaddr[32];	

	vInt = 0;
	if (mib_get(MIB_DMZ_ENABLE, (void *)value) != 0)
		vInt = (int)(*(unsigned char *)value);

	if (mib_get(MIB_DMZ_IP, (void *)value) != 0)
	{
		if (vInt == 1)
		{
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)value)), 16);
			ipaddr[15] = '\0';
			iptable_dmz_natLB_dynamic(ifname, ipaddr, myip, nat_pre_name, nat_post_name, fh);
		}
	}
}
#endif

static int del_NATLB_dynamic(char *nat_pre_name, char *nat_post_name)
{
	// iptables -t nat -F portfwPreNatLB_ppp0
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", nat_pre_name);
	// iptables -t nat -A PREROUTING -j portfwPreNatLB_pppo
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_DEL, (char *)FW_PREROUTING, "-j", nat_pre_name);
	// iptables -t nat -X portfwPreNatLB_ppp0
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", nat_pre_name);
	
	// iptables -t nat -N portfwPostNatLB_ppp0
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-F", nat_post_name);
	// iptables -t nat -A POSTROUTING -j portfwPostNatLB_ppp0
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_DEL, (char *)FW_POSTROUTING, "-j", nat_post_name);
	// iptables -t nat -X portfwPostNatLB_ppp0
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-X", nat_post_name);
	return 0;
}

int cleanOneEntry_NATLB_rule_dynamic_link(MIB_CE_ATM_VC_Tp pEntry, int mode)
{	
	char ifname[10];
	char nat_pre_name[30], nat_post_name[30];

	ifGetName(pEntry->ifIndex, ifname, sizeof(ifname));	
	
	if ((DHCP_T)pEntry->ipDhcp == DHCP_CLIENT || pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA)
	{
#ifdef PORT_FORWARD_GENERAL
		// (1) del NATLB of portfw rule
		if ( mode == DEL_ALL_NATLB_DYNAMIC || mode == DEL_PORTFW_NATLB_DYNAMIC)
		{
			snprintf(nat_pre_name, 30, "%s_%s", PORT_FW_PRE_NAT_LB, ifname);
			snprintf(nat_post_name, 30, "%s_%s", PORT_FW_POST_NAT_LB, ifname);
			del_NATLB_dynamic(nat_pre_name, nat_post_name);
		}
#endif
		
#ifdef DMZ
		// (2) del NATLB of dmz rule
		if ( mode == DEL_ALL_NATLB_DYNAMIC || mode == DEL_DMZ_NATLB_DYNAMIC)
		{
			snprintf(nat_pre_name, 30, "%s_%s", IPTABLE_DMZ_PRE_NAT_LB, ifname);
			snprintf(nat_post_name, 30, "%s_%s", IPTABLE_DMZ_POST_NAT_LB, ifname);
			del_NATLB_dynamic(nat_pre_name, nat_post_name);
		}
#endif
	}
	return 0;
}

int cleanALLEntry_NATLB_rule_dynamic_link(int mode)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entryNum; i++) {
		/* Retrieve entry */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			printf("cleanAll_NATLB_rule_dynamic_link: cannot get ATM_VC_TBL(ch=%d) entry\n", i);
			return 0;
		}

		cleanOneEntry_NATLB_rule_dynamic_link(&Entry, mode);
	}	
}

static int set_dhcp_NATLB_chain(int fh, char *nat_pre_name, char *nat_post_name)
{
	char buff[64];
	
	// iptables -t nat -F portfwPreNatLB_ptm0_0
	snprintf(buff, 64, "\tiptables -t nat -F %s\n", nat_pre_name);
	WRITE_DHCPC_FILE(fh, buff);	
	// iptables -t nat -D PREROUTING -j portfwPreNatLB_ptm0_0
	snprintf(buff, 64, "\tiptables -t nat -D PREROUTING -j %s\n", nat_pre_name);	
	WRITE_DHCPC_FILE(fh, buff);
	// iptables -t nat -X portfwPreNatLB_ptm0_0
	snprintf(buff, 64, "\tiptables -t nat -X %s\n", nat_pre_name);
	WRITE_DHCPC_FILE(fh, buff);
	
	// iptables -t nat -F portfwPostNatLB_ptm0_0	
	snprintf(buff, 64, "\tiptables -t nat -F %s\n", nat_post_name);
	WRITE_DHCPC_FILE(fh, buff);
	// iptables -t nat -D POSTROUTING -j portfwPostNatLB_ptm0_0	
	snprintf(buff, 64, "\tiptables -t nat -D POSTROUTING -j %s\n", nat_post_name);	
	WRITE_DHCPC_FILE(fh, buff);
	// iptables -t nat -X portfwPostNatLB_ptm0_0	
	snprintf(buff, 64, "\tiptables -t nat -X %s\n", nat_post_name);
	WRITE_DHCPC_FILE(fh, buff);
	
	// iptables -t nat -N portfwPreNatLB_ptm0_0	
	snprintf(buff, 64, "\tiptables -t nat -N %s\n", nat_pre_name);
	WRITE_DHCPC_FILE(fh, buff);	
	// iptables -t nat -A PREROUTING -j portfwPreNatLB_ptm0_0	
	snprintf(buff, 64, "\tiptables -t nat -A PREROUTING -j %s\n", nat_pre_name);	
	WRITE_DHCPC_FILE(fh, buff);
	
	// iptables -t nat -N portfwPostNatLB_ptm0_0	
	snprintf(buff, 64, "\tiptables -t nat -N %s\n", nat_post_name);
	WRITE_DHCPC_FILE(fh, buff);
	// iptables -t nat -A POSTROUTING -j portfwPostNatLB_ptm0_0	
	snprintf(buff, 64, "\tiptables -t nat -A POSTROUTING -j %s\n", nat_post_name);	
	WRITE_DHCPC_FILE(fh, buff);
	return 0;
}

int set_dhcp_NATLB(int fh, MIB_CE_ATM_VC_Tp pEntry)
{
	int vInt, i, total;
	unsigned char value[32];
#ifdef PORT_FORWARD_GENERAL
	MIB_CE_PORT_FW_T PfEntry;
#endif
	char ifname[10];
	char nat_pre_name[30], nat_post_name[30];
	
	ifGetName(pEntry->ifIndex, ifname, sizeof(ifname));	
	snprintf(nat_pre_name, 30, "%s_%s", PORT_FW_PRE_NAT_LB, ifname);
	snprintf(nat_post_name, 30, "%s_%s", PORT_FW_POST_NAT_LB, ifname);		

#ifdef PORT_FORWARD_GENERAL
	// (1) set NATLB rule for portfw
	vInt = 0;
	if (mib_get(MIB_PORT_FW_ENABLE, (void *)value) != 0)
		vInt = (int)(*(unsigned char *)value);

	if (vInt == 1)
	{
		int negate=0, hasRemote=0;
		char * proto = 0;
		char intPort[32], extPort[32];		
		
		total = mib_chain_total(MIB_PORT_FW_TBL);
		
		if (total > 0)
			set_dhcp_NATLB_chain(fh, nat_pre_name, nat_post_name);
			
		for (i = 0; i < total; i++)
		{
			if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&PfEntry))
				return -1;
				
			portfw_NATLB_dynamic( &PfEntry, ifname, NULL, nat_pre_name, nat_post_name, fh);
		}
	}
#endif
	
#ifdef DMZ
	// (2) set NATLB rule for dmz
	snprintf(nat_pre_name, 30, "%s_%s", IPTABLE_DMZ_PRE_NAT_LB, ifname);
	snprintf(nat_post_name, 30, "%s_%s", IPTABLE_DMZ_POST_NAT_LB, ifname);
	
	vInt = 0;
	if (mib_get(MIB_DMZ_ENABLE, (void *)value) != 0)
		vInt = (int)(*(unsigned char *)value);
		
	if (mib_get(MIB_DMZ_IP, (void *)value) != 0)
	{
		if (vInt == 1)
		{
			set_dhcp_NATLB_chain(fh, nat_pre_name, nat_post_name);	
			set_dmz_natLB_dynamic(ifname, NULL, nat_pre_name, nat_post_name, fh);	
		}
	}	
#endif

	return 0;
}

static int set_ppp_NATLB_chain(char *nat_pre_name, char *nat_post_name)
{
	del_NATLB_dynamic(nat_pre_name, nat_post_name);
	
	// iptables -t nat -N portfwPreNatLB_ppp0
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", nat_pre_name);
	// iptables -t nat -A PREROUTING -j portfwPreNatLB_pppo
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_PREROUTING, "-j", nat_pre_name);
	// iptables -t nat -N portfwPostNatLB_ppp0
	va_cmd(IPTABLES, 4, 1, "-t", "nat", "-N", nat_post_name);
	// iptables -t nat -A POSTROUTING -j portfwPostNatLB_ppp0
	va_cmd(IPTABLES, 6, 1, "-t", "nat", (char *)FW_ADD, (char *)FW_POSTROUTING, "-j", nat_post_name);
	return 0;
}

int set_ppp_NATLB(char *ifname, char *myip)
{
	int vInt, i, total;
	unsigned char value[32];
	MIB_CE_PORT_FW_T PfEntry;
	char nat_pre_name[30], nat_post_name[30];
	
	snprintf(nat_pre_name, 30, "%s_%s", PORT_FW_PRE_NAT_LB, ifname);
	snprintf(nat_post_name, 30, "%s_%s", PORT_FW_POST_NAT_LB, ifname);	
	
#ifdef PORT_FORWARD_GENERAL
	// (1) set NATLB rule for portfw
	vInt = 0;
	if (mib_get(MIB_PORT_FW_ENABLE, (void *)value) != 0)
		vInt = (int)(*(unsigned char *)value);

	if (vInt == 1)
	{
		int negate=0, hasRemote=0;
		char * proto = 0;
		char intPort[32], extPort[32];

		total = mib_chain_total(MIB_PORT_FW_TBL);
		
		if ( total > 0)
			set_ppp_NATLB_chain(nat_pre_name, nat_post_name);
			
		for (i = 0; i < total; i++)
		{
			if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&PfEntry))
				return -1;
				
			portfw_NATLB_dynamic( &PfEntry, ifname, myip, nat_pre_name, nat_post_name, 0);
		}
	}
#endif

#ifdef DMZ
	// (2) set NATLB rule for dmz
	snprintf(nat_pre_name, 30, "%s_%s", IPTABLE_DMZ_PRE_NAT_LB, ifname);
	snprintf(nat_post_name, 30, "%s_%s", IPTABLE_DMZ_POST_NAT_LB, ifname);

	vInt = 0;
	if (mib_get(MIB_DMZ_ENABLE, (void *)value) != 0)
		vInt = (int)(*(unsigned char *)value);
		
	if (mib_get(MIB_DMZ_IP, (void *)value) != 0)
	{
		if (vInt == 1)
		{
			set_ppp_NATLB_chain(nat_pre_name, nat_post_name);	
			set_dmz_natLB_dynamic(ifname, myip, nat_pre_name, nat_post_name, 0);		
		}
	}	
#endif
	
	return 1;
}

#ifdef DMZ
int iptable_dmz_natLB(int del, char *dstIP)
{	
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	unsigned char ipStr[20], subnetStr[20];
	unsigned long mask, mbit, ip, subnet;	
	char ipaddr[32], lan_ipaddr[32];	
	char *act;
	
	if(del) act = (char *)FW_DEL;
	else act = (char *)FW_ADD;
	
	// (1) Get LAN IP and mask
	mib_get(MIB_ADSL_LAN_IP, (void *)&ip);
	strncpy(ipStr, inet_ntoa(*((struct in_addr *)&ip)), 16);
	ipStr[15] = '\0';

	mib_get(MIB_ADSL_LAN_SUBNET, (void *)&mask);
	subnet = (ip&mask);	
	strncpy(subnetStr, inet_ntoa(*((struct in_addr *)&subnet)), 16);
	subnetStr[15] = '\0';

	mbit=0;
	while (1) {
		if (mask&0x80000000) {
			mbit++;
			mask <<= 1;
		}
		else
			break;
	}
	snprintf(lan_ipaddr, sizeof(lan_ipaddr), "%s/%d", subnetStr, mbit);	
	
	//(2) Set rule
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entryNum; i++) 
	{
		/* Retrieve entry */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) 
		{
			printf("setStaticNATLoopBack: cannot get ATM_VC_TBL(ch=%d) entry\n", i);
			return 0;
		}
		
		// Set rule for static link only
		if ( !Entry.enable || ((DHCP_T)Entry.ipDhcp != DHCP_DISABLED)
			|| ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_BRIDGE)
			|| ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_PPPOE)
			|| ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_PPPOA)
			|| (!Entry.napt)
			)
			continue;
			
		snprintf(ipaddr, sizeof(ipaddr), "%s", inet_ntoa(*((struct in_addr *)Entry.ipAddr)));
		
#ifdef DMZ
		// (2.1) Set prerouting rule
		// iptables -t nat -A dmzPreNatLB -s 192.168.1.0/24 -d 192.168.8.1 -j DNAT --to-destination 192.168.1.20		
		va_cmd(IPTABLES, 12, 1, "-t", "nat",
			(char *)act,	(char *)IPTABLE_DMZ_PRE_NAT_LB,
			"-s", lan_ipaddr,
			"-d", ipaddr,			
			"-j",	"DNAT", "--to-destination", dstIP);
			
		// (2.2) Set postrouting rule
		//iptables -t nat -A dmzPostNatLB -s 192.168.1.0/24 -d 192.168.1.20 -j SNAT --to-source 192.168.8.1		
		va_cmd(IPTABLES, 12, 1, "-t", "nat",
			(char *)act,	(char *)IPTABLE_DMZ_POST_NAT_LB,
			"-s", lan_ipaddr,
			"-d", dstIP,			
			"-j", "SNAT", "--to-source", ipaddr);		
#endif
	}
}
#endif

#ifdef PORT_FORWARD_GENERAL
int iptable_fw_natLB( int del, unsigned int ifIndex, const char *proto, const char *extPort, const char *dstIP)
{	
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	unsigned char ipStr[20], subnetStr[20];
	unsigned long mask, mbit, ip, subnet;
	char *act;
	char ipaddr[32], lan_ipaddr[32], dstIP2[32], *pStr;
	
	if(del) act = (char *)FW_DEL;
	else act = (char *)FW_ADD;
	
	// (1) Get LAN IP and mask
	mib_get(MIB_ADSL_LAN_IP, (void *)&ip);
	strncpy(ipStr, inet_ntoa(*((struct in_addr *)&ip)), 16);
	ipStr[15] = '\0';

	mib_get(MIB_ADSL_LAN_SUBNET, (void *)&mask);
	subnet = (ip&mask);	
	strncpy(subnetStr, inet_ntoa(*((struct in_addr *)&subnet)), 16);
	subnetStr[15] = '\0';

	mbit=0;
	while (1) {
		if (mask&0x80000000) {
			mbit++;
			mask <<= 1;
		}
		else
			break;
	}
	snprintf(lan_ipaddr, sizeof(lan_ipaddr), "%s/%d", subnetStr, mbit);
	
	strncpy(dstIP2, dstIP, sizeof(dstIP2));
	pStr = strstr(dstIP2, ":");
	if (pStr != NULL)
		pStr[0] = '\0';
	
	//(2) Set rule
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < entryNum; i++) 
	{
		/* Retrieve entry */
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) 
		{
			printf("setStaticNATLoopBack: cannot get ATM_VC_TBL(ch=%d) entry\n", i);
			return 0;
		}
		
		// Set rule for static link only
		if ( !Entry.enable || ((DHCP_T)Entry.ipDhcp != DHCP_DISABLED)
			|| ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_BRIDGE)
			|| ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_PPPOE)
			|| ((CHANNEL_MODE_T)Entry.cmode == CHANNEL_MODE_PPPOA)
			|| (!Entry.napt)
			)
			continue;			
		
		if (ifIndex != DUMMY_IFINDEX)
		{
			if (Entry.ifIndex != ifIndex)
				continue;
		}
		snprintf(ipaddr, sizeof(ipaddr), "%s", inet_ntoa(*((struct in_addr *)Entry.ipAddr)));		
		
		// (2.1) Set prerouting rule
		// iptables -t nat -A portfwPreNatLB -s 192.168.1.0/24 -d 192.168.8.1 -p TCP --dport 21 -j DNAT --to-destination 192.168.1.20:21			
		if (extPort) {
			va_cmd(IPTABLES, 16, 1, "-t", "nat",
				(char *)act,	(char *)PORT_FW_PRE_NAT_LB,
				"-s", lan_ipaddr,
				"-d", ipaddr,
				"-p", (char *)proto,
				(char *)FW_DPORT, extPort, "-j",
				"DNAT", "--to-destination", dstIP);
		} else {
			va_cmd(IPTABLES, 14, 1, "-t", "nat",
				(char *)act,	(char *)PORT_FW_PRE_NAT_LB,
				"-s", lan_ipaddr,
				"-d", ipaddr,
				"-p", (char *)proto,
				"-j", "DNAT", "--to-destination", dstIP);
		}

		// (2.2) Set postrouting rule
		//iptables -t nat -A portfwPostNatLB -s 192.168.1.0/24 -d 192.168.1.20 -p TCP --dport 21 -j SNAT --to-source 192.168.8.1 
		if (extPort) {
			va_cmd(IPTABLES, 16, 1, "-t", "nat",
				(char *)act,	(char *)PORT_FW_POST_NAT_LB,
				"-s", lan_ipaddr,
				"-d", dstIP2,
				"-p", (char *)proto,
				(char *)FW_DPORT, extPort, "-j",
				"SNAT", "--to-source", ipaddr);
		} else {
			va_cmd(IPTABLES, 14, 1, "-t", "nat",
				(char *)act,	(char *)PORT_FW_POST_NAT_LB,
				"-s", lan_ipaddr,
				"-d", dstIP2,
				"-p", (char *)proto,
				"-j", "SNAT", "--to-source", ipaddr);
		}
		
		if (ifIndex != DUMMY_IFINDEX)			
			break;		
	}
	return 0;
}
#endif
#endif
#endif

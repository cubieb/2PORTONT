#include <string.h>
#include <unistd.h>
#include "mib.h"
#include "utility.h"
 
#ifdef EMBED
#include <linux/config.h>
#include <config/autoconf.h>
#else
#include "../../../../include/linux/autoconf.h"
#include "../../../../config/autoconf.h"
#endif

#if defined(CONFIG_USER_DHCPV6_ISC_DHCP411) || defined (CONFIG_IPV6)
const char DHCPDV6_CONF_AUTO[]="/var/dhcpd6_auto.conf";
const char DHCPDV6_CONF[]="/var/dhcpd6.conf";
const char DHCPDV6_LEASES[]="/var/dhcpd6.leases";
const char DHCPDV6[]="/bin/dhcpd";
const char DHCREALYV6[]="/bin/dhcrelayV6";
const char DHCPSERVER6PID[]="/var/run/dhcpd6.pid";
const char DHCPRELAY6PID[]="/var/run/dhcrelay6.pid";
const char DHCPCV6SCRIPT[]="/etc/dhclient-script";
const char DHCPCV6[]="/bin/dhclient";
const char DHCPCV6STR[]="dhcpcV6";
#endif

#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
int option_name_server(FILE *fp)
{
	unsigned int entryNum, i;
	MIB_DHCPV6S_NAME_SERVER_T Entry;
	unsigned char strAll[(MAX_V6_IP_LEN+2)*MAX_DHCPV6_CHAIN_ENTRY]="";
	
	entryNum = mib_chain_total(MIB_DHCPV6S_NAME_SERVER_TBL);

	for (i=0; i<entryNum; i++) {
		unsigned char buf[MAX_V6_IP_LEN+2]="";
		
		if (!mib_chain_get(MIB_DHCPV6S_NAME_SERVER_TBL, i, (void *)&Entry))
		{
  			printf("option_name_server: Get Name Server chain record error!\n");
			return;
		}
		
		if ( i< (entryNum-1) )
		{
			sprintf(buf, "%s, ", Entry.nameServer);
		} else
			sprintf(buf, "%s", Entry.nameServer);
		strcat(strAll, buf);		
	}
	
	if ( entryNum > 0 )
	{
		//printf("strAll=%s\n", strAll);
		fprintf(fp, "option dhcp6.name-servers %s;\n", strAll);
	}
	
	return 0;
}

int option_domain_search(FILE *fp)
{
	unsigned int entryNum, i;
	MIB_DHCPV6S_DOMAIN_SEARCH_T Entry;
	unsigned char strAll[(MAX_DOMAIN_LENGTH+4)*MAX_DHCPV6_CHAIN_ENTRY]="";
	
	entryNum = mib_chain_total(MIB_DHCPV6S_DOMAIN_SEARCH_TBL);

	for (i=0; i<entryNum; i++) {
		unsigned char buf[MAX_DOMAIN_LENGTH+4]="";
		
		if (!mib_chain_get(MIB_DHCPV6S_DOMAIN_SEARCH_TBL, i, (void *)&Entry))
		{
  			printf("option_name_server: Get domain search chain record error!\n");
			return;
		}
		
		if ( i< (entryNum-1) )
		{
			sprintf(buf, "\"%s\", ", Entry.domain);
		} else
			sprintf(buf, "\"%s\"", Entry.domain);
		strcat(strAll, buf);		
	}
	
	if ( entryNum > 0 )
	{
		//printf("strAll(domain)=%s\n", strAll);
		fprintf(fp, "option dhcp6.domain-search %s;\n", strAll);
	}
	
	return 0;
}

int setup_dhcpdv6_conf()
{
	FILE *fp;
	unsigned int DLTime, PFTime, RNTime, RBTime;
	unsigned char vChar;
	struct in6_addr ip6Addr, ip6Prefix;
	int k;
	unsigned char value[MAX_V6_IP_LEN];
	unsigned char value2[MAX_V6_IP_LEN];
	unsigned char value3[MAX_DUID_LEN];
	 
	if ((fp = fopen(DHCPDV6_CONF, "w")) == NULL)
	{
		printf("Open file %s failed !\n", DHCPDV6_CONF);
		return;
	}	

	// default-lease-time	
	if (mib_get(MIB_DHCPV6S_DEFAULT_LEASE, (void *)&DLTime) == 0)
		return -1;
	fprintf(fp, "default-lease-time %u;\n", DLTime);
	
	// preferred-lifetime	
	if (mib_get(MIB_DHCPV6S_PREFERRED_LIFETIME, (void *)&PFTime) == 0)
		return -1;
	fprintf(fp, "preferred-lifetime %u;\n", PFTime);
	
	// option dhcp-renewal-time	
	if (mib_get(MIB_DHCPV6S_RENEW_TIME, (void *)&RNTime) == 0)
		return -1;
	fprintf(fp, "option dhcp-renewal-time %u;\n", RNTime);
	
	// option dhcp-rebinding-time	
	if (mib_get(MIB_DHCPV6S_REBIND_TIME, (void *)&RBTime) == 0)
		return -1;
	fprintf(fp, "option dhcp-rebinding-time %u;\n", RBTime);
	
	// Option dhcp6.name-servers
	option_name_server(fp);
	
	// Option dhcp6.domain-search
	option_domain_search(fp);
	
	// option dhcp6.client-id 00:01:00:01:00:04:93:e0:00:00:00:00:a2:a2;
	if ( !mib_get(MIB_DHCPV6S_CLIENT_DUID, (void *)value3)) {
		printf("Get MIB_DHCPV6S_CLIENT_DUID mib error!");
		return -1;
	}
	if (value3[0]) {
		fprintf(fp, "option dhcp6.client-id %s;\n", value3);
	}
	
	// subnet6 3ffe:501:ffff:100::/64 {
	mib_get(MIB_DHCPV6S_PREFIX_LENGTH, (void *)&vChar);
	mib_get(MIB_DHCPV6S_RANGE_START, (void *)ip6Addr.s6_addr);
	k = (int)vChar;
	if (k<=0 || k > 128) {
		printf("WARNNING! Prefix Length == %d\n", k);
		k = 64;
	}
	ip6toPrefix(&ip6Addr, k, &ip6Prefix);
	inet_ntop(PF_INET6, &ip6Prefix, value, sizeof(value));
	fprintf(fp, "subnet6 %s/%d {\n", value, k);
	
	getMIB2Str(MIB_DHCPV6S_RANGE_START, value);
	getMIB2Str(MIB_DHCPV6S_RANGE_END, value2);
	if (value[0] && value2[0]) {
		fprintf(fp, "\trange6 %s %s;\n", value, value2);
	}	
	fprintf(fp, "}\n");
	fclose(fp);
	
	if ((fp = fopen(DHCPDV6_LEASES, "w")) == NULL)
	{
		printf("Open file %s failed !\n", DHCPDV6_LEASES);
		return;
	}
	fclose(fp);
	
	return 1;	
	
}

int startDhcpv6Relay()
{	
	char upper_ifname[8];
	unsigned int upper_if;
	
	if ( !mib_get(MIB_DHCPV6R_UPPER_IFINDEX, (void *)&upper_if)) {
		printf("Get MIB_DHCPV6R_UPPER_IFINDEX mib error!");
		return -1;
	}		
	
	if ( upper_if != DUMMY_IFINDEX )
	{
		ifGetName(upper_if, upper_ifname, sizeof(upper_ifname));
	}
	else
	{
		printf("Error: The upper interface of dhcrelayV6 not set !\n");
		return 0;
	}
	
	// dhcrelayV6 -6 -l br0 -u vc0
	va_cmd(DHCREALYV6, 5, 0, "-6", "-l", BRIF, "-u", upper_ifname);
	return 0;
}

int start_dhcpv6(int enable)
{
	unsigned char value[64];
	unsigned char vChar;
	int tmp_status, status=0;
	unsigned int uInt, i;
	DHCP_TYPE_T mode;
	int dhcpserverpid=0,dhcprelaypid=0;
	struct in6_addr ip6Addr, targetIp;
	unsigned char devAddr[MAC_ADDR_LEN];
	unsigned char meui64[8];
	
	mib_get(MIB_V6_IPV6_ENABLE, (void *)&vChar);
	if (vChar == 0)
		return;

	mode = DHCP_LAN_NONE;
	if (mib_get(MIB_DHCPV6_MODE, (void *)value) != 0)
	{
		mode = (DHCP_TYPE_T)(*(unsigned char *)value);
	}
		
	dhcpserverpid = read_pid((char*)DHCPSERVER6PID);
	dhcprelaypid = read_pid((char*)DHCPRELAY6PID);
	
	if(dhcpserverpid > 0) {
		kill(dhcpserverpid, 15);
		//unlink((char*)DHCPSERVER6PID);
	}	
	if(dhcprelaypid > 0) {
		kill(dhcprelaypid, 15);
		//unlink((char*)DHCPRELAY6PID);
	}
	
	while(read_pid((char*)DHCPSERVER6PID)>0 ||read_pid((char*)DHCPRELAY6PID)>0)
	{
		usleep(30000);
	}	
	
	if (mode == DHCP_LAN_SERVER) {
		mib_get(MIB_DHCPV6S_PREFIX_LENGTH, (void *)&vChar);
		mib_get(MIB_DHCPV6S_RANGE_START, (void *)ip6Addr.s6_addr);
		uInt = (int)vChar;
		if (uInt<=0 || uInt > 128) {
			printf("WARNNING! Prefix Length == %d\n", uInt);
			uInt = 64;
		}
		ip6toPrefix(&ip6Addr, uInt, &targetIp);
		mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
		mac_meui64(devAddr, meui64);
		for (i=0; i<8; i++)
			targetIp.s6_addr[i+8] = meui64[i];
		inet_ntop(PF_INET6, &targetIp, value, sizeof(value));
		sprintf(value, "%s/%d", value, uInt);
		va_cmd(IFCONFIG, 3, 1, LANIF, "del", value);
		if (enable)
			va_cmd(IFCONFIG, 3, 1, LANIF, "add", value);
	}
	
	if (!enable)
		return 1;
	
	if(mode == DHCP_LAN_SERVER)
	{
		tmp_status = setup_dhcpdv6_conf();
		if (tmp_status == 1)
		{			
			// dhcpd -6 -cf /var/dhcpdv6.conf -lf /var/dhcpd6.leases br0
			status = va_cmd(DHCPDV6, 6, 0, "-6", "-cf", DHCPDV6_CONF, "-lf", DHCPDV6_LEASES, BRIF);

			//while(read_pid((char*)DHCPSERVER6PID) < 0)
			//	usleep(250000);
		}
		else if (tmp_status == -1)
	   	 	status = -1;
		return status;
	}
	else if(mode == DHCP_LAN_RELAY)
	{
		startDhcpv6Relay();
		status=(status==-1)?-1:0;

		return status;
	}
	else if(mode == DHCP_LAN_SERVER_AUTO)
	{
		//When AUTO mode, should wait until Configuration file is ready then start the DHCPv6 Server.
		//(After connection is ready and getting prefix delegation.)
		if(access(DHCPDV6_CONF_AUTO, F_OK)==0)
			status = va_cmd(DHCPDV6, 6, 0, "-6", "-cf", DHCPDV6_CONF_AUTO, "-lf", DHCPDV6_LEASES, BRIF);
    }
	else
	{
		return 0;
	}
	return -1;
}


#endif
#endif // #ifdef CONFIG_IPV6

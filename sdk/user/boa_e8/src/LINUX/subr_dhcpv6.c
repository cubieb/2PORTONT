#include <string.h>
#include <unistd.h>
#include "mib.h"
#include "utility.h"
#include "ipv6_info.h"
 
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

int setup_dhcpdv6_conf_new(DNS_V6_INFO_Tp pDnsV6Info, PREFIX_V6_INFO_Tp pPrefixInfo)
{
	FILE *fp;
	unsigned int DLTime, PFTime, RNTime, RBTime;
	unsigned char vChar;
	struct in6_addr ip6Addr, ip6Prefix;
	int k;
	unsigned char value[MAX_V6_IP_LEN];
	unsigned char value2[MAX_V6_IP_LEN];
	unsigned char value3[MAX_DUID_LEN];
	unsigned char value4[MAX_V6_IP_LEN];
	char* strVal;
	 
	if(!pDnsV6Info || !pPrefixInfo){
		printf("Error! Input data is NULL %s\n",__func__);
		return -1;
	}

	if ((fp = fopen(DHCPDV6_CONF, "w")) == NULL) {
		printf("Open file %s failed !\n", DHCPDV6_CONF);
		return -1;
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
	fprintf(fp, "option dhcp6.name-servers %s;\n", pDnsV6Info->nameServer);
	
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

	//subnet6 2001:b010:7030:1a01::/64 {
	//        range6 2001:b010:7030:1a01::/64;
	//	 (or) range6 low-address high-address;
	//        }	

	//Add min_address & max_address
	mib_get(MIB_DHCPV6S_MIN_ADDRESS, (void *)&value2);
	mib_get(MIB_DHCPV6S_MAX_ADDRESS, (void *)&value4);
	
	printf("[%s:%d]\n",__func__,__LINE__);	
	if (pPrefixInfo->prefixIP[0] && pPrefixInfo->prefixLen) {
		struct  in6_addr ip6Addr, ip6Addr2;
		unsigned char minaddr[IP6_ADDR_LEN]={0},maxaddr[IP6_ADDR_LEN]={0};
		unsigned char devAddr[MAC_ADDR_LEN]={0};
		unsigned char meui64[8]={0};
		unsigned char prefixBuf[100]={0},tmpBuf[100]={0},tmpBuf2[100]={0};
		int i;
		
		inet_ntop(PF_INET6,pPrefixInfo->prefixIP, prefixBuf, sizeof(prefixBuf));
		printf("prefix is %s/%d\n",prefixBuf, pPrefixInfo->prefixLen);

		fprintf(fp, "subnet6 %s/%d {\n", prefixBuf, pPrefixInfo->prefixLen);
		//fprintf(fp, "\trange6 %s/%d;\n", value, vChar);

		//Merge the prefix & min/max address 
		sprintf(tmpBuf,"::%s",value2);
		inet_pton(PF_INET6,tmpBuf,minaddr);

		sprintf(tmpBuf,"::%s",value4);
		inet_pton(PF_INET6,tmpBuf,maxaddr);
		
		for (i=0; i<8; i++){
			minaddr[i] = pPrefixInfo->prefixIP[i];
			maxaddr[i] = pPrefixInfo->prefixIP[i];
		}

		inet_ntop(PF_INET6,minaddr, tmpBuf, sizeof(tmpBuf));
		inet_ntop(PF_INET6,maxaddr, tmpBuf2, sizeof(tmpBuf2));

		printf("range6 %s%s;\n",tmpBuf,tmpBuf2);
		fprintf(fp, "\trange6 %s %s;\n", tmpBuf,tmpBuf2);  
		fprintf(fp, "}\n");

		//setup br0 IPv6 IP according to this prefix
		memcpy(ip6Addr.s6_addr,pPrefixInfo->prefixIP,sizeof(ip6Addr.s6_addr));

		mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
		mac_meui64(devAddr, meui64);
		for (i=0; i<8; i++)
			ip6Addr.s6_addr[i+8] = meui64[i];

		inet_ntop(PF_INET6, &ip6Addr, value, sizeof(value));
		sprintf(value, "%s/%d", value,pPrefixInfo->prefixLen);
		va_cmd(IFCONFIG, 3, 1, LANIF, "del", value);
		va_cmd(IFCONFIG, 3, 1, LANIF, "add", value);
	}	
	fclose(fp);

	if ((fp = fopen(DHCPDV6_LEASES, "w")) == NULL)
	{
		printf("Open file %s failed !\n", DHCPDV6_LEASES);
		return;
	}
	fclose(fp);
	
	return 1;	
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
	
	// Option dhcp6.name-servers: (e8 spec require DNS relay)
	if (mib_get(MIB_IPV6_LAN_IP_ADDR, (void *)value) == 0)
		return -1;
	fprintf(fp, "option dhcp6.name-servers %s;\n", value);
	
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

	//subnet6 2001:b010:7030:1a01::/64 {
	//        range6 2001:b010:7030:1a01::/64;
	//        }	
	mib_get(MIB_IPV6_LAN_PREFIX, (void *)&value);
	mib_get(MIB_IPV6_LAN_PREFIX_LEN, (void *)&vChar);
	if (value[0] && vChar) {

		struct  in6_addr ip6Addr;
		unsigned char devAddr[MAC_ADDR_LEN];
		unsigned char meui64[8];
		int i;

		fprintf(fp, "subnet6 %s/%d {\n", value, vChar);
		fprintf(fp, "\trange6 %s/%d;\n", value, vChar);
		fprintf(fp, "}\n");

		//setup br0 IPv6 IP according to this prefix
		inet_pton(AF_INET6,value,&ip6Addr);
		mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
		mac_meui64(devAddr, meui64);
		for (i=0; i<8; i++)
			ip6Addr.s6_addr[i+8] = meui64[i];
		inet_ntop(PF_INET6, &ip6Addr, value, sizeof(value));
		sprintf(value, "%s/%d", value, vChar);
		va_cmd(IFCONFIG, 3, 1, LANIF, "del", value);
		va_cmd(IFCONFIG, 3, 1, LANIF, "add", value);
	}	
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
	
	if (!enable || (mode==DHCP_LAN_NONE))
		return 1;

	//If all the WAN interface NOT request Prefix Delegation AND IPv6 LAN prefix is configured
	//Then start DHCPv6 Server manual
	if(!ifPrefixUsingPD() && ifIPv6LanPrefixConfigured())
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
	else if(ifPrefixUsingPD())
	{
		//When AUTO mode, should wait until Configuration file is ready then start the DHCPv6 Server.
		//(After connection is ready and getting prefix delegation.)
		if(access(DHCPDV6_CONF_AUTO, F_OK)==0)
			status = va_cmd(DHCPDV6, 6, 0, "-6", "-cf", DHCPDV6_CONF_AUTO, "-lf", DHCPDV6_LEASES, BRIF);
    }
	return -1;
}


int start_dhcpv6_new(DNS_V6_INFO_Tp pDnsV6Info, PREFIX_V6_INFO_Tp pPrefixInfo)
{
	unsigned char value[64];
	unsigned char vChar;
	int tmp_status, status=0;
	unsigned int uInt, i;
	DHCP_TYPE_T mode;
	int dhcpserverpid=0;
	
	mib_get(MIB_V6_IPV6_ENABLE, (void *)&vChar);
	if (vChar == 0)
		return;

	mode = DHCP_LAN_NONE;
	if (mib_get(MIB_DHCPV6_MODE, (void *)value) != 0)
	{
		mode = (DHCP_TYPE_T)(*(unsigned char *)value);
	}
		
	dhcpserverpid = read_pid((char*)DHCPSERVER6PID);
	if(dhcpserverpid > 0) {
		kill(dhcpserverpid, 15);
		//unlink((char*)DHCPSERVER6PID);
	}	
	
	while(read_pid((char*)DHCPSERVER6PID)>0)
	{
		usleep(30000);
	}	
	
	if (mode==DHCP_LAN_NONE)
		return 1;

	tmp_status = setup_dhcpdv6_conf_new(pDnsV6Info,pPrefixInfo);
	if (tmp_status == 1) {		
		// dhcpd -6 -cf /var/dhcpdv6.conf -lf /var/dhcpd6.leases br0
		status = va_cmd(DHCPDV6, 6, 0, "-6", "-cf", DHCPDV6_CONF, "-lf", DHCPDV6_LEASES, BRIF);
		return status;
	}

	return -1;
}

#endif
#endif // #ifdef CONFIG_IPV6

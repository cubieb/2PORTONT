/*
 *      IPv6 basic routines
 *
 */

#include <string.h>
#include "debug.h"
#include "utility.h"
#include "ipv6_info.h"
#include <signal.h>

#if defined CONFIG_IPV6 || defined CONFIG_RTK_RG_INIT
/*
 *	convert ipv6 address to ipv6 prefix
 *	ip6:	ipv6 address
 *	plen:	prefix length
 *	prefix: target ipv6 prefix
 *	Return: 1 Success; 0 fail
 */
int ip6toPrefix(void *ip6, int plen, void *prefix)
{
	struct in6_addr *src, *dst;
	int i, k, m;
	unsigned char mask, tmask;

	if (plen <=1 || plen > 128)
		return 0;

	src = (struct in6_addr *)ip6;
	dst = (struct in6_addr *)prefix;
	*dst = in6addr_any;
	k = plen/8;
	for (i=0; i<k; i++)
		dst->s6_addr[i] = src->s6_addr[i];
	m = plen - k*8;
	if (m) {
		mask = 0;
		tmask = 0x80;
		for (i=0; i<m; i++) {
			mask |= tmask;
			tmask = tmask>>1;
		}
	}
	dst->s6_addr[k] &= mask;
	return 1;
}

/*
 *	convert Ethernet address to modified EUI-64
 *	src(6 octects):	mac address
 *	dst(8 octects):	target MEUI-64
 *	Return: 1 Success; 0 fail
 */
int mac_meui64(char *src, char *dst)
{
	int i;

	memset(dst, 0, 8);
	memcpy(dst, src, 3);
	memcpy(dst + 5, src + 3, 3);
	dst[3] = 0xff;
	dst[4] = 0xfe;
	dst[0] ^= 0x02;
	return 1;
}

/*
 *	Get IPv6 addresses of interface.
 *	addr_scope: net/ipv6.h
 *		IPV6_ADDR_ANY		0x0000U
 *		IPV6_ADDR_UNICAST      	0x0001U
 *		IPV6_ADDR_MULTICAST    	0x0002U
 *		IPV6_ADDR_LOOPBACK	0x0010U
 *		IPV6_ADDR_LINKLOCAL	0x0020U
 *		IPV6_ADDR_SITELOCAL	0x0040U
 *		IPV6_ADDR_COMPATv4	0x0080U
 *		IPV6_ADDR_SCOPE_MASK	0x00f0U
 *		IPV6_ADDR_MAPPED	0x1000U
 *		IPV6_ADDR_RESERVED	0x2000U
 *	addr_lst: address list
 *	num: max number of address
 *	Return: number of addresses
 */
int getifip6(char *ifname, unsigned int addr_scope, struct ipv6_ifaddr *addr_lst, int num)
{
	FILE *fp;
	struct in6_addr		addr;
	unsigned int		ifindex = 0;
	unsigned int		prefixlen, scope, flags;
	unsigned char		scope_value;
	char			devname[IFNAMSIZ];
	char 			buf[1024];
	int			k = 0;

	memset(addr_lst, 0, sizeof(struct ipv6_ifaddr)*num);
	/* Get link local addresses from /proc/net/if_inet6 */
	fp = fopen("/proc/net/if_inet6", "r");
	if (!fp)
		return 0;

	scope_value = (addr_scope == IPV6_ADDR_UNICAST) ? IPV6_ADDR_ANY : addr_scope;
	/* Format "fe80000000000000029027fffe24bbab 02 0a 20 80     eth0" */
	while (fgets(buf, sizeof(buf), fp))
	{
		//printf("buf= %s\n", buf);
		if (21 != sscanf( buf,
			"%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx %x %x %x %x %8s",
			&addr.s6_addr[ 0], &addr.s6_addr[ 1], &addr.s6_addr[ 2], &addr.s6_addr[ 3],
			&addr.s6_addr[ 4], &addr.s6_addr[ 5], &addr.s6_addr[ 6], &addr.s6_addr[ 7],
			&addr.s6_addr[ 8], &addr.s6_addr[ 9], &addr.s6_addr[10], &addr.s6_addr[11],
			&addr.s6_addr[12], &addr.s6_addr[13], &addr.s6_addr[14], &addr.s6_addr[15],
			&ifindex, &prefixlen, &scope, &flags, devname))
		{
			printf("/proc/net/if_inet6 has a broken line, ignoring");
			continue;
		}

		if (!strcmp(ifname, devname) && (addr_scope == IPV6_ADDR_ANY || scope_value == scope)) {
			if (k>=num)
				break;
			else {
				addr_lst[k].valid = 1;
				memcpy(&addr_lst[k].addr, &addr, sizeof(struct in6_addr));
				addr_lst[k].prefix_len = prefixlen;
				addr_lst[k].flags = flags;
				addr_lst[k].scope = scope;
			}
			k++;
		}
		//inet_ntop(PF_INET6, &addr, buf, 1024);
		//printf("IPv6: %s scope=0x%x\n", buf, scope);
	}

	fclose(fp);
	return k;
}

#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
static void make6RD_prefix(MIB_CE_ATM_VC_Tp pEntry, unsigned char *ip6buf, int ip6buf_size)
{
	unsigned int ipAddr;
	unsigned char B1,B2,B3,B4;
	struct in6_addr ip6Addr;
	unsigned char devAddr[MAC_ADDR_LEN];
	unsigned char meui64[8];
	int i;
	int v4addr_offset = pEntry->SixrdPrefixLen/8;

	inet_pton(PF_INET6, pEntry->SixrdPrefix, (void *) ip6Addr.s6_addr);
	ipAddr = (*(unsigned int *)(pEntry->ipAddr))<<pEntry->SixrdIPv4MaskLen;

	if(( pEntry->SixrdPrefixLen % 8 ) ==0 )
	{
		B1 = ipAddr>>24;
		B2 = (ipAddr<<8)>>24;
		B3 = (ipAddr<<16)>>24;
		B4 = (ipAddr<<24)>>24;
		ip6Addr.s6_addr[v4addr_offset] = B1;
		ip6Addr.s6_addr[v4addr_offset+1] = B2;
		ip6Addr.s6_addr[v4addr_offset+2] = B3;
		ip6Addr.s6_addr[v4addr_offset+3] = B4;

	}
	else
	{   //SixrdPrefixLen is not multiple of 8, will be more complicated to handle

		int prefix_lastbits= pEntry->SixrdPrefixLen % 8;                        //Ex:41%8=1 , last bit is 1
		unsigned int v4IP_B1B2B2B4_shifted  =  ipAddr <<(8-prefix_lastbits);    //            shift left for (8-1) = 7 bits

		B1 = v4IP_B1B2B2B4_shifted>>24;
		B2 = (v4IP_B1B2B2B4_shifted<<8)>>24;
		B3 = (v4IP_B1B2B2B4_shifted<<16)>>24;
		B4 = (v4IP_B1B2B2B4_shifted<<24)>>24;

		ip6Addr.s6_addr[v4addr_offset] =  (ip6Addr.s6_addr[v4addr_offset]>>(8-prefix_lastbits)) <<(8-prefix_lastbits);
		                                  //This stip is for EX: 6rd prefix is 2001:1001:F000 but prefix lenght is 34
		                                  //Then the bit 33,34 will be recorded. others will be shifted out.
		ip6Addr.s6_addr[v4addr_offset] |= ipAddr>>(24+prefix_lastbits);
		ip6Addr.s6_addr[v4addr_offset+1] = B1;
		ip6Addr.s6_addr[v4addr_offset+2] = B2;
		ip6Addr.s6_addr[v4addr_offset+3] = B3;
		ip6Addr.s6_addr[v4addr_offset+4] = B4;
	}

	mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
	mac_meui64(devAddr, meui64);
	for (i=0; i<8; i++)
		ip6Addr.s6_addr[i+8] = meui64[i];

	inet_ntop(PF_INET6, &ip6Addr, ip6buf, ip6buf_size);
}
#endif

#ifdef _PRMT_X_CT_COM_DHCP_
static inline void to_hex(unsigned char *buf, unsigned char *value, size_t size)
{
	int i;

	for(i = 0 ; i < size ; i++)
	{
		if(i != 0)
		{
			buf[0] = ':';
			buf++;
		}
		sprintf(buf, "%02x", value[i]);
		buf += 2;
	}
}

int option_vendor_class_identify(FILE *fp, MIB_CE_ATM_VC_Tp pEntry)
{
	unsigned int i;

	/**
	 * opt16 is an encoded hex string, format: "aa:bb:cc:dd....."
	 * Start with CTCOM enterprise number 0 (4 bytes in DHCPv6) temprarily.
	 * Value 0 is specified by spec.
	 * max size is 253 * 3 bytes.
	 */ 
	char opt16[1024] = "00:00:00:00:";
	char opt16_data[512] = {0};	// option16 binary data
	char *cur = opt16_data;
	unsigned short total_len = 0;

	//define option 16
	fprintf(fp, "option dhcp6.vendor-class code 16 = string;\n");
	
	// create binary option 16 data
	for (i = 0; i < 4; i++)
	{
		if(pEntry->dhcpv6_opt16_enable[i])
		{
			unsigned char field_len = 0;

			if(pEntry->dhcpv6_opt16_type[i] == 34 && pEntry->dhcpv6_opt16_value_mode[i] != 2)
				continue;

			if(pEntry->dhcpv6_opt16_value_mode[i] == 0)
			{
				field_len = (unsigned char) strlen(pEntry->dhcpv6_opt16_value[i]);

				if(field_len > 0)
				{
					// (2) 1 bytes for Field type
					memcpy(cur, &pEntry->dhcpv6_opt16_type[i], 1);
					total_len +=1;
					cur +=1;

					// (3) 1 bytes for Field Length
					memcpy(cur, &field_len, 1);
					total_len +=1;
					cur +=1;
				
					// (4) n bytes for Field Value
					memcpy(cur, pEntry->dhcpv6_opt16_value[i], field_len);
					total_len += field_len;
					cur += field_len;
				}
			}
			else if((pEntry->dhcpv6_opt16_type[i] == 34 && pEntry->dhcpv6_opt16_value_mode[i] == 2) ||
				(pEntry->dhcpv6_opt16_type[i] != 34 && pEntry->dhcpv6_opt16_value_mode[i] == 1))
			{
				char buf[80] = {0};

				field_len = gen_ctcom_dhcp_opt(pEntry->dhcpv6_opt16_type[i], buf, 80);

				if(field_len > 0)
				{
					// (2) 1 bytes for Field type
					memcpy(cur, &pEntry->dhcpv6_opt16_type[i], 1);
					total_len +=1;
					cur +=1;

					// (3) 1 bytes for Field Length
					memcpy(cur, &field_len, 1);
					total_len +=1;
					cur +=1;
				
					// (4) n bytes for Field Value
					memcpy(cur, buf, field_len);
					total_len += field_len;
					cur += field_len;
				}
			}
		}
	}

	if(total_len > 0)
	{
		char inf[IFNAMSIZ] = {0};
		unsigned char buf[1024] = {0};
		char *tmp = (char *)&total_len;

		// append data
		sprintf(buf, "%02x:%02x:", tmp[0], tmp[1]);
		strcat(opt16, buf);
		printf("total data len=%d\n", total_len);
		to_hex(buf, opt16_data, total_len);
		strcat(opt16, buf);
		printf("DHCPv6 opt16=%s\n", opt16);

		// Get interface nname
		if (pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA)
		{
			snprintf(inf, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
		}
		else{
			ifGetName( PHY_INTF(pEntry->ifIndex), inf, sizeof(inf));
		}

		//send option 16
		fprintf(fp, "interface \"%s\" {\n"
				   "    send dhcp6.vendor-class %s;\n"
				   "}\n\n", inf, opt16);
	}
	
	return 0;
}
#endif	//_PRMT_X_CT_COM_DHCP_


int startIP_for_V6(MIB_CE_ATM_VC_Tp pEntry)
{
	unsigned char value[64], pidfile[30], leasefile[30];
	unsigned char 	Ipv6AddrStr[48], RemoteIpv6AddrStr[48], RemoteIpv6EndPointAddrStr[48];
	char *argv[20];
	int idx=0;
	char inf[IFNAMSIZ], infTun[10], v6NetRoute[10];
	char vChar=-1;
#ifdef _PRMT_X_CT_COM_DHCP_
	char fconf[64] = {0};
#endif

	mib_get(MIB_V6_IPV6_ENABLE, (void *)&vChar);
	if (vChar == 0)
		return;

	if (pEntry->IpProtocol & IPVER_IPV6) {
		// Get interface nname
		if (pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA)
		{
			snprintf(inf, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
		}
		else{
			ifGetName( PHY_INTF(pEntry->ifIndex), inf, sizeof(inf));
		}

		if ( pEntry->cmode == CHANNEL_MODE_IPOE || pEntry->cmode == CHANNEL_MODE_RT1483 || pEntry->cmode == CHANNEL_MODE_6RD ) {

			// Stop Slaac
			if ((pEntry->AddrMode & 0x1) != 0x1) {
				snprintf(value, 64, "/bin/echo 0 > /proc/sys/net/ipv6/conf/%s/autoconf", inf);
				system(value);
				snprintf(value, 64, "/bin/echo 0 > /proc/sys/net/ipv6/conf/%s/forwarding", inf);
				system(value);
			}
			// Set forwarding=0 to do Slaac
			else {
				snprintf(value, 64, "/bin/echo 0 > /proc/sys/net/ipv6/conf/%s/forwarding", inf);
				system(value);
			}

			if ( ((pEntry->AddrMode & 0x2)) == 0x2 ) {
				char file[64] = {0};
				FILE *infdns = NULL;

				inet_ntop(PF_INET6, (struct in6_addr *)pEntry->Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
				inet_ntop(PF_INET6, (struct in6_addr *)pEntry->RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));

				// Add WAN IP for MER
				snprintf(Ipv6AddrStr, 48, "%s/%d", Ipv6AddrStr, pEntry->Ipv6AddrPrefixLen);
				va_cmd(IFCONFIG, 3, 1, inf, ARG_ADD, Ipv6AddrStr);

				// Add default gw
				if (pEntry->dgw) {
					// route -A inet6 add ::/0 gw 3ffe::0200:00ff:fe00:0100 dev vc0
					va_cmd(ROUTE, 7, 1, FW_ADD, "inet6", ARG_ADD, "::/0", "gw", RemoteIpv6AddrStr, inf);
				}

				// Write DNS servers to /var/resolv6.conf.{interface}
				snprintf(file, 64, "%s.%s", (char *)DNS6_RESOLV, inf);
				infdns=fopen(file,"w");

				if(infdns)
				{
					unsigned char zero_ip[IP6_ADDR_LEN] = {0};
					char dns_addr[48] = {0};

					if(memcmp(zero_ip, pEntry->Ipv6Dns1, IP6_ADDR_LEN) != 0)
					{
						inet_ntop(AF_INET6, pEntry->Ipv6Dns1, dns_addr, 48);
						fprintf(infdns, "%s\n", dns_addr);
						va_cmd(ROUTE, 6, 1, "-A",  "inet6", "add", dns_addr, "dev", inf);
					}
					if(memcmp(zero_ip, pEntry->Ipv6Dns2, IP6_ADDR_LEN) != 0)
					{
						inet_ntop(AF_INET6, pEntry->Ipv6Dns2, dns_addr, 48);
						fprintf(infdns, "%s\n", dns_addr);
						va_cmd(ROUTE, 6, 1, "-A",  "inet6", "add", dns_addr, "dev", inf);
					}

					fclose(infdns);
				}
			}

#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
			// Start DHCPv6 client
			// dhclient -6 -sf /var/dhclient-script -lf /var/dhclient6-leases -pf /var/run/dhclient6.pid -d -q -N -P -cf /var/dhclient6.conf vc0
			if ( (pEntry->Ipv6Dhcp == 1)|| ((pEntry->Ipv6DhcpRequest & 0x2) == 0x2) ) {
				int dhcpcpid;

				argv[1] = "-6";
				argv[2] = "-sf";
				argv[3] = (char *)DHCPCV6SCRIPT;
				argv[4] = "-lf";
				snprintf(leasefile, 30, "/var/%s%s.leases", DHCPCV6STR, inf);
				argv[5] = leasefile;
				argv[6] = "-pf";
				snprintf(pidfile, 30, "/var/run/%s%s.pid", DHCPCV6STR, inf);
				argv[7] = pidfile;
				argv[8] = "-d";
				argv[9] = "-q";
				idx = 10;

				// Request Address
				if ( (pEntry->Ipv6DhcpRequest & 0x1) == 0x1 ) {
					argv[idx++] = "-N";
				}

				// Request Prefix
				if ( (pEntry->Ipv6DhcpRequest & 0x2) == 0x2 ) {
					argv[idx++] = "-P";
				}

#ifdef _PRMT_X_CT_COM_DHCP_
				{
					unsigned int ctcom_id = 0x0;

					sprintf(fconf, "/var/dhclient6_%s.conf", inf);

					FILE *fp = fopen(fconf, "w");

					if(fp)
					{
						fprintf(fp, "select-timeout 3;\n");

						fprintf(fp, "also request dhcp6.vendor-opts;\n");
						fprintf(fp, "require dhcp6.vendor-opts;\n");

#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
						//If dslite_aftr_mode is AUTO, set the conf of dhclient v6
						if(pEntry->dslite_enable && pEntry->dslite_aftr_mode == IPV6_DSLITE_MODE_AUTO ){
							fprintf(fp, "option dhcp6.dslite code 64 = domain-list; \n");
							fprintf(fp, "also request dhcp6.dslite; \n");
						}
						else{
							//If dslite_aftr_mode is static, send signal to let systemd be prepared to setup dslite
							if(pEntry->dslite_enable && 
							 (pEntry->dslite_aftr_mode == IPV6_DSLITE_MODE_STATIC ) &&
							 (pEntry->dslite_aftr_hostname!=NULL)){
								unsigned int pid;
								pid = read_pid("/var/run/systemd.pid");
								if ( pid > 0){
									printf("Send signal USR2 to systemd pid %d\n",pid);
									kill(pid, SIGUSR2);	
								}
							}
						}
#endif

						//send option 16
						option_vendor_class_identify(fp, pEntry);

						fclose(fp);
					}
				}
				argv[idx++] = "-cf";
				argv[idx++] = fconf;
#endif

				argv[idx++] = inf;
				argv[idx] = NULL;

				TRACE(STA_SCRIPT, "%s %s %s %s %s %s %s ...\n", DHCPCV6, argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
				do_cmd(DHCPCV6, argv, 0);
			}
#endif

			// Start DS-Lite
#ifdef DUAL_STACK_LITE
			if ( (pEntry->AddrMode & 0x4) == 0x4 ) {
				inet_ntop(PF_INET6, (struct in6_addr *)pEntry->Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
				inet_ntop(PF_INET6, (struct in6_addr *)pEntry->RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
				inet_ntop(PF_INET6, (struct in6_addr *)pEntry->RemoteIpv6EndPointAddr, RemoteIpv6EndPointAddrStr, sizeof(RemoteIpv6EndPointAddrStr));

				// (1) Add WAN Local IP
				snprintf(Ipv6AddrStr, 48, "%s/128", Ipv6AddrStr);
				va_cmd(IFCONFIG, 3, 1, inf, ARG_ADD, Ipv6AddrStr);

				// (2) Set tunnel
				// ip -6 tunnel add tun0 mode ipip6 local 2001:240:63f:ff00::3 remote 2001:240:63f:ff01::1 dev nas0_0
				snprintf(infTun, sizeof(infTun), "tun%u", VC_INDEX(pEntry->ifIndex));
				va_cmd("/bin/ip", 12, 1, "-6", "tunnel", "add", infTun, "mode", "ipip6", "local", Ipv6AddrStr, "remote", RemoteIpv6EndPointAddrStr, "dev", inf);

				// (3) Up tunnel
				// ip link set dev tun0 up
				va_cmd("/bin/ip", 5, 1, "link", "set", "dev", infTun, "up");

				// (4) Set IPv4 default route
				// ip route add default dev tun0
				if(pEntry->dgw == 1)
					va_cmd("/bin/ip", 5, 1, "route", "add", "default", "dev", infTun);

				// Set IPv6 route
				// (5) ip -6 route add 2001::/4 dev nas0_0
				snprintf(v6NetRoute, sizeof(v6NetRoute), "%c001::/4", Ipv6AddrStr[0]);
				va_cmd("/bin/ip", 6, 1, "-6", "route", "add", v6NetRoute, "dev", inf);
				// (6) ip -6 route add 2001:240:63f:ff01::1 via 2001:240:63f:ff00::1 dev nas0_0
				va_cmd("/bin/ip", 8, 1, "-6", "route", "add", RemoteIpv6EndPointAddrStr, "via", RemoteIpv6AddrStr, "dev", inf);
			}
#endif
			// Start 6rd
#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
			if ( (pEntry->AddrMode & 0x8) == 0x8 ) {
				unsigned char ipAddrStr[INET_ADDRSTRLEN];
				unsigned char SixrdBRv4IP[INET_ADDRSTRLEN];
				unsigned char buf[128];
				unsigned char ip6buf[48];
				char *tun6rdif ="tun6rd\0";

				printf("Start 6rd config\n");

				if( (pEntry->SixrdPrefixLen+(32-pEntry->SixrdIPv4MaskLen)) >64)
				{
					printf("Invalid 6RD setting, PrefixLen and IPv4 address > 64!!! Please check the setting!\n");
					return -1;
                }

				inet_ntop(PF_INET,  (struct in_addr *)pEntry->ipAddr, ipAddrStr, sizeof(ipAddrStr));
				inet_ntop(PF_INET,  (struct in_addr *)pEntry->SixrdBRv4IP, SixrdBRv4IP, sizeof(SixrdBRv4IP));

				//Setup tunnel
				//(1) ip tunnel add tun6rd mode sit local 10.2.2.2 ttl 64
				printf("Add 6rd tunnel\n");
				va_cmd("/bin/ip", 9, 1, "tunnel", "add", tun6rdif, "mode", "sit", "local", ipAddrStr, "ttl", "64");

				//(2) ip tunnel 6rd dev tun6rd 6rd-prefix 2001:db8::/32 6rd-relay_prefix 10.0.0.0/8
				printf("Setup 6rd tunnel\n");
				if(pEntry->SixrdIPv4MaskLen)
				{
					unsigned char IPv4Mask[INET_ADDRSTRLEN];
					unsigned int relay_mask = (*(unsigned int *)(pEntry->SixrdBRv4IP))&(0xffffffff<<(32-pEntry->SixrdIPv4MaskLen));
					unsigned buf2[128];

					inet_ntop(PF_INET,  &relay_mask,  IPv4Mask, sizeof(IPv4Mask));
					snprintf(buf, sizeof(buf), "%s/%d", pEntry->SixrdPrefix,pEntry->SixrdPrefixLen);
					snprintf(buf2, sizeof(buf), "%s/%d", IPv4Mask,pEntry->SixrdIPv4MaskLen);
					va_cmd("/bin/ip", 8, 1, "tunnel", "6rd", "dev", tun6rdif, "6rd-prefix", buf, "6rd-relay_prefix",buf2);
				}
				else
				{
					snprintf(buf, sizeof(buf), "%s::/%d", pEntry->SixrdPrefix,pEntry->SixrdPrefixLen);
					va_cmd("/bin/ip", 6, 1, "tunnel", "6rd", "dev", tun6rdif, "6rd-prefix", buf);
				}

				//(3) ip link set tun6rd up
				printf("6rd tunnel up\n");
				va_cmd("/bin/ip", 4, 1, "link", "set", tun6rdif, "up");

				//IP address and Routing
				//(4) ip -6 addr add 2001:db8:a02:202:EUI64/64 dev br0
				printf("Setup 6rd Address and routing\n");
				make6RD_prefix(pEntry,ip6buf,sizeof(ip6buf));
				snprintf(buf, sizeof(buf), "%s/64", ip6buf);
				va_cmd("/bin/ip", 6, 1, "-6", "addr", "add",  buf, "dev", BRIF);

				//(5) ip -6 addr add 2001:db8:a02:202:EUI64/32 dev tun6rd
				snprintf(buf, sizeof(buf), "%s/%d", ip6buf,pEntry->SixrdPrefixLen);
				va_cmd("/bin/ip", 6, 1, "-6", "addr", "add",  buf, "dev", tun6rdif);

				//(6) ip -6 route add ::/0 via ::10.1.1.1 dev tun6rd
				snprintf(buf, sizeof(buf), "::%s", SixrdBRv4IP);
				va_cmd("/bin/ip", 8, 1, "-6", "route", "add", "::/0", "via", buf, "dev", tun6rdif);
			}
#endif
		}
	}
}

int stopIP_PPP_for_V6(MIB_CE_ATM_VC_Tp pEntry)
{
	unsigned char value[64], pidfile[30], leasefile[30];
	unsigned char 	Ipv6AddrStr[48], RemoteIpv6AddrStr[48], RemoteIpv6EndPointAddrStr[48];
	char *argv[20];
	char inf[IFNAMSIZ], infTun[10], v6NetRoute[10];
	int dhcpcpid;

	if (pEntry->IpProtocol & IPVER_IPV6) {
		// Get interface nname
		if (pEntry->cmode == CHANNEL_MODE_PPPOE || pEntry->cmode == CHANNEL_MODE_PPPOA)
		{
			snprintf(inf, 6, "ppp%u", PPP_INDEX(pEntry->ifIndex));
		}
		else{
			ifGetName( PHY_INTF(pEntry->ifIndex), inf, sizeof(inf));
		}

		// Start Slaac
		snprintf(value, 64, "/bin/echo 1 > /proc/sys/net/ipv6/conf/%s/autoconf", inf);
		system(value);

		// Set default value (forwarding=1)
		//snprintf(value, 64, "/bin/echo 1 > /proc/sys/net/ipv6/conf/%s/forwarding", inf);
		//system(value);

		if ( ((pEntry->AddrMode & 0x2)) == 0x2 ) {
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));

			// delete WAN IP for MER
			snprintf(Ipv6AddrStr, 48, "%s/%d", Ipv6AddrStr, pEntry->Ipv6AddrPrefixLen);
			va_cmd(IFCONFIG, 3, 1, inf, ARG_DEL, Ipv6AddrStr);

			// delete default gw
			if (pEntry->dgw) {
				// route -A inet6 del ::/0 gw 3ffe::0200:00ff:fe00:0100 dev vc0
				va_cmd(ROUTE, 7, 1, FW_ADD, "inet6", ARG_DEL, "::/0", "gw", RemoteIpv6AddrStr, inf);
			}
		}

#ifdef CONFIG_USER_DHCPV6_ISC_DHCP411
		// Stop DHCPv6 client
		if ( (pEntry->Ipv6Dhcp == 1)|| ((pEntry->Ipv6DhcpRequest & 0x2) == 0x2)  ) {
			snprintf(pidfile, 30, "/var/run/%s%s.pid", DHCPCV6STR, inf);
			dhcpcpid = read_pid(pidfile);


			if(dhcpcpid > 0) {
				kill(dhcpcpid, 15);
				snprintf(leasefile, 30, "/var/%s%s.leases", DHCPCV6STR, inf);
				unlink(pidfile);
				unlink(leasefile);
			}
		}
#endif

#if defined(CONFIG_IPV6) && defined(DUAL_STACK_LITE)
		// Stop DS-Lite
		if ( pEntry->dslite_enable==1 ) {
			char cmd[100];
			sprintf(cmd,"ip  -6 tunnel del tun1\n");	
			printf("cmd:%s\n",cmd);
			system(cmd);

			unlink("/var/dslite_info");
/*
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->Ipv6Addr, Ipv6AddrStr, sizeof(Ipv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->RemoteIpv6Addr, RemoteIpv6AddrStr, sizeof(RemoteIpv6AddrStr));
			inet_ntop(PF_INET6, (struct in6_addr *)pEntry->RemoteIpv6EndPointAddr, RemoteIpv6EndPointAddrStr, sizeof(RemoteIpv6EndPointAddrStr));

			// del IPv6 route
			// (1) ip -6 route del 2001::/4 dev nas0_0
			snprintf(v6NetRoute, sizeof(v6NetRoute), "%c001::/4", Ipv6AddrStr[0]);
			va_cmd("/bin/ip", 6, 1, "-6", "route", "del", v6NetRoute, "dev", inf);

			// (2) ip -6 route del 2001:240:63f:ff01::1 via 2001:240:63f:ff00::1 dev nas0_0
			va_cmd("/bin/ip", 8, 1, "-6", "route", "del", RemoteIpv6EndPointAddrStr, "via", RemoteIpv6AddrStr, "dev", inf);

			snprintf(infTun, sizeof(infTun), "tun%u", VC_INDEX(pEntry->ifIndex));
			// (3) del IPv4 default route
			// ip route del default dev tun0
			va_cmd("/bin/ip", 5, 1, "route", "del", "default", "dev", infTun);

			// (4) Clean tunnel
			// ip -6 tunnel del tun0 mode ipip6 local 2001:240:63f:ff00::3 remote 2001:240:63f:ff01::1 dev nas0_0
			va_cmd("/bin/ip", 12, 1, "-6", "tunnel", "del", infTun, "mode", "ipip6", "local", Ipv6AddrStr, "remote", RemoteIpv6EndPointAddrStr, "dev", inf);

			// (5) Del WAN Local IP
			snprintf(Ipv6AddrStr, 48, "%s/128", Ipv6AddrStr);
			va_cmd(IFCONFIG, 3, 1, inf, ARG_DEL, Ipv6AddrStr);
*/
		}
#endif
		// Stop 6rd
#if defined(CONFIG_IPV6) && defined(CONFIG_IPV6_SIT_6RD)
		if ( (pEntry->AddrMode & 0x8) == 0x8 ) {
			unsigned char ip6buf[48];
			unsigned char buf[128];
			char *tun6rdif ="tun6rd\0";

			printf("Stop 6rd config\n");

			//(1) delete tunnel : ip tunnel del tun6rd
			va_cmd("/bin/ip", 3 ,1 ,"tunnel", "del", tun6rdif);

			//(2) delete default route : ip route del default
			va_cmd("/bin/ip", 3, 1, "route", "del", "default");

			//(3) ip -6 addr del 2001:db8:a02:202:EUI64/64 dev br0
			make6RD_prefix(pEntry,ip6buf,sizeof(ip6buf));
			snprintf(buf, sizeof(buf), "%s/64", ip6buf);
			va_cmd("/bin/ip", 6, 1, "-6", "addr", "del",  buf, "dev", BRIF);

			//(4) ip -6 route del ::/0
			va_cmd("/bin/ip", 8, 1, "-6", "route", "del", "::/0");
		}
#endif
	}
}

void setup_disable_ipv6(char *itf, int disable)
{
	char buf[64];
	char vChar=-1;

	mib_get(MIB_V6_IPV6_ENABLE, (void *)&vChar);
	if (vChar==0)
	{
		snprintf(buf, 64, "/bin/echo %d > /proc/sys/net/ipv6/conf/all/disable_ipv6", 1);
		system(buf);
	}
	else
	{
		snprintf(buf, 64, "/bin/echo %d > /proc/sys/net/ipv6/conf/all/disable_ipv6", 0);
		system(buf);
		snprintf(buf, 64, "/bin/echo %d > /proc/sys/net/ipv6/conf/%s/disable_ipv6", disable, itf);
		system(buf);
	}
}


int setupIPV6Filter()
{
	char *argv[20];
	unsigned char value[32], byte;
	int vInt, i, total;
	MIB_CE_IP_PORT_FILTER_T IpEntry;
	char *policy, *filterSIP, *filterDIP, srcPortRange[12], dstPortRange[12];
	char  srcip[55], dstip[55], srcip2[55], dstip2[55];
	char SIPRange[110]={0};
	char DIPRange[110]={0};
	char *filterSIPRange=NULL;
	char *filterDIPRange=NULL;
	unsigned char ipportfilter_state;
	filterSIP=filterDIP=NULL;
	// Delete ipfilter rule
	va_cmd(IP6TABLES, 2, 1, "-F", (char *)FW_IPV6FILTER);

	// packet filtering
	// ip filtering
	total = mib_chain_total(MIB_IP_PORT_FILTER_TBL);

	// accept related
	// ip6tables -A ipv6filter -m state --state ESTABLISHED,RELATED -j RETURN
	va_cmd(IP6TABLES, 8, 1, (char *)FW_ADD, (char *)FW_IPV6FILTER, "-m", "state",
			"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_RETURN);
	// ip6tables -A ipv6filter -d ff00::/8 -j RETURN
	//va_cmd(IP6TABLES, 6, 1, (char *)FW_ADD, (char *)FW_IPV6FILTER, "-d",
	//	"ff00::/8", "-j", (char *)FW_RETURN);
#ifdef CONFIG_RTK_RG_INIT
	FlushRTK_RG_ACL_IPv6Port_Filters();
#endif

	mib_get(MIB_IPFILTER_ON_OFF, (void*)&ipportfilter_state);

	if(ipportfilter_state == 0) return 0;

	for (i = 0; i < total; i++)
	{
		int idx=0;
		/*
		 *	srcPortRange: src port
		 *	dstPortRange: dst port
		 */
		filterSIPRange=filterDIPRange=NULL;
		filterSIP=filterDIP=NULL;
		memset(argv,0,sizeof(argv));

		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, i, (void *)&IpEntry))
			return -1;

		if (IpEntry.IpProtocol != IPVER_IPV6) // 1: IPv4, 2:IPv6
			continue;

		if (IpEntry.action == 0)
			policy = (char *)FW_DROP;
		else
			policy = (char *)FW_RETURN;

		// source port
		if (IpEntry.srcPortFrom == 0)
			srcPortRange[0]='\0';
		else if (IpEntry.srcPortFrom == IpEntry.srcPortTo)
			snprintf(srcPortRange, 12, "%u", IpEntry.srcPortFrom);
		else
			snprintf(srcPortRange, 12, "%u:%u",
					IpEntry.srcPortFrom, IpEntry.srcPortTo);

		// destination port
		if (IpEntry.dstPortFrom == 0)
			dstPortRange[0]='\0';
		else if (IpEntry.dstPortFrom == IpEntry.dstPortTo)
			snprintf(dstPortRange, 12, "%u", IpEntry.dstPortFrom);
		else
			snprintf(dstPortRange, 12, "%u:%u",
					IpEntry.dstPortFrom, IpEntry.dstPortTo);

		// source ip, prefixLen
		if(IpEntry.sip6End[0] == 0)    // normal ip filter, no iprange supported
		{
			inet_ntop(PF_INET6, (struct in6_addr *)IpEntry.sip6Start, srcip, 48);
			if (strcmp(srcip, "::") == 0)
				filterSIP = 0;
			else
			{
				if (IpEntry.sip6PrefixLen!=0)
					snprintf(srcip, sizeof(srcip), "%s/%d", srcip, IpEntry.sip6PrefixLen);

				filterSIP = srcip;
			}
		}
		else
		{
			inet_ntop(PF_INET6, (struct in6_addr *)IpEntry.sip6Start, srcip, 48);
			inet_ntop(PF_INET6, (struct in6_addr *)IpEntry.sip6End, srcip2, 48);

			if(strcmp(srcip, "::") ==0 || strcmp(srcip2, "::") ==0)
				filterSIPRange=0;
			else
			{
				snprintf(SIPRange, sizeof(SIPRange), "%s-%s", srcip, srcip2);
				filterSIPRange=SIPRange;
			}
		}

		// destination ip, mask
		if(IpEntry.dip6End[0] == 0)    // normal ip filter, no iprange supported
		{
			inet_ntop(PF_INET6, (struct in6_addr *)IpEntry.dip6Start, dstip, 48);
			if (strcmp(dstip, "::") == 0)
				filterDIP = 0;
			else
			{
				if (IpEntry.dip6PrefixLen!=0)
					snprintf(dstip, sizeof(dstip), "%s/%d", dstip, IpEntry.dip6PrefixLen);

				filterDIP = dstip;
			}
		}
		else
		{
			inet_ntop(PF_INET6, (struct in6_addr *)IpEntry.dip6Start, dstip, 48);
			inet_ntop(PF_INET6, (struct in6_addr *)IpEntry.dip6End, dstip2, 48);

			if(strcmp(dstip, "::") ==0 || strcmp(dstip2, "::") ==0)
				filterDIPRange=0;
			else
			{
				snprintf(DIPRange, sizeof(DIPRange), "%s-%s", dstip, dstip2);
				filterDIPRange=DIPRange;
			}
		}

		// interface
		argv[1] = (char *)FW_ADD;
		argv[2] = (char *)FW_IPV6FILTER;
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
			else //if (IpEntry.protoType == PROTO_ICMPV6)
				argv[idx++] = (char *)ARG_ICMPV6;
		}

		// src ip
		if (filterSIP != 0)
		{
			argv[idx++] = "-s";
			argv[idx++] = filterSIP;

		}

		// src port
		if ((IpEntry.protoType==PROTO_TCP ||
					IpEntry.protoType==PROTO_UDP) &&
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

		// iprange
		if(filterSIPRange || filterDIPRange)
		{
			argv[idx++] = "-m";
			argv[idx++] = "iprange";
			if(filterSIPRange)
			{
				argv[idx++] = "--src-range";
				argv[idx++] = filterSIPRange;
			}
			if(filterDIPRange)
			{
				argv[idx++] = "--dst-range";
				argv[idx++] = filterDIPRange;
			}
		}

		// dst port
		if ((IpEntry.protoType==PROTO_TCP ||
					IpEntry.protoType==PROTO_UDP) &&
				dstPortRange[0] != 0) {
			argv[idx++] = (char *)FW_DPORT;
			argv[idx++] = dstPortRange;
		}

		// target/jump
		argv[idx++] = "-j";
		argv[idx++] = policy;
		argv[idx++] = NULL;

		//printf("idx=%d\n", idx);
		TRACE(STA_SCRIPT, "%s %s %s %s %s %s %s ...\n", IP6TABLES, argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]);
		do_cmd(IP6TABLES, argv, 1);
#ifdef CONFIG_RTK_RG_INIT
		if(IpEntry.IpProtocol == IPVER_IPV6) //ipv6
			//AddRTK_RG_ACL_IPv6Port_Filter(&IpEntry);
			AddRTK_RG_ACL_IPPort_Filter_IPv6(&IpEntry);
#endif
	}

	// Added by Mason Yu. for Telefonica ICMPV6 of firewall
	// ip6tables -A ipv6filter -p ICMPV6 -j RETURN
	va_cmd(IP6TABLES, 6, 1, (char *)FW_ADD, (char *)FW_IPV6FILTER, "-p", (char *)ARG_ICMPV6, "-j", (char *)FW_RETURN);

	// Kill all conntrack (to kill the established conntrack when change ip6tables rules)
	va_cmd("/bin/ethctl", 2, 1, "conntrack", "killall");
	return 1;
}

int setup_default_IPV6Filter()
{
	// Set default action for ipv6filter
	unsigned char value;
	unsigned char ipportfilter_state;

	mib_get(MIB_IPFILTER_ON_OFF, (void*)&ipportfilter_state);

	if(ipportfilter_state == 0) return 0;

	if (mib_get(MIB_V6_IPF_OUT_ACTION, (void *)&value) != 0)
	{	
		if (value == 0)	// DROP
		{
			// ip6tables -A ipv6filter -i $LAN_IF -j DROP
			va_cmd(IP6TABLES, 6, 1, (char *)FW_ADD,
					(char *)FW_IPV6FILTER, (char *)ARG_I,
					(char *)LANIF, "-j", (char *)FW_DROP);
		}
	}

	if (mib_get(MIB_V6_IPF_IN_ACTION, (void *)&value) != 0)
	{
		if (value == 0)	// DROP
		{
			// ip6tables -A ipv6filter -i ! $LAN_IF -j DROP
			va_cmd(IP6TABLES, 7, 1, (char *)FW_ADD,
					(char *)FW_IPV6FILTER, (char *)ARG_I, "!",
					(char *)LANIF, "-j", (char *)FW_DROP);

		}
	}

	return 1;
}

int restart_IPV6Filter()
{
	setupIPV6Filter();
	setup_default_IPV6Filter();
	return 1;
}

void restartRadvd()
{
	DNS_V6_INFO_T dnsV6Info={0};
	PREFIX_V6_INFO_T prefixInfo={0};
	unsigned IPv6Enable=0,radvdEnable=0;
	int radvdpid;

	mib_get(MIB_V6_IPV6_ENABLE, (void *)&IPv6Enable);
	if(IPv6Enable){

		mib_get(MIB_V6_RADVD_ENABLE, (void *)&radvdEnable);

		get_dnsv6_info(&dnsV6Info);
		get_prefixv6_info(&prefixInfo);
		
		if(radvdEnable){
			setup_radvd_conf_new(&dnsV6Info,&prefixInfo);
			radvdpid=read_pid((char *)RADVD_PID);
			if(radvdpid>0){ //TERM it, let previous prefix be deprecated.
				kill(radvdpid, SIGTERM);
				unlink(RADVD_PID); //remove pid file, since if the pid file still existing will impact next new radvd process
			}
			va_cmd( "/bin/radvd", 3, 0, "-s", "-C", (char *)RADVD_CONF );
		}
	}
}

void restartLanV6Server()
{
	DNS_V6_INFO_T dnsV6Info={0};
	PREFIX_V6_INFO_T prefixInfo={0};
	unsigned IPv6Enable=0,radvdEnable=0;
	int radvdpid;

	mib_get(MIB_V6_IPV6_ENABLE, (void *)&IPv6Enable);
	if(IPv6Enable){

		mib_get(MIB_V6_RADVD_ENABLE, (void *)&radvdEnable);

		get_dnsv6_info(&dnsV6Info);
		get_prefixv6_info(&prefixInfo);
		
		if(radvdEnable){
			setup_radvd_conf_new(&dnsV6Info,&prefixInfo);
			radvdpid=read_pid((char *)RADVD_PID);
			if(radvdpid>0){ //TERM it, let previous prefix be deprecated.
				kill(radvdpid, SIGTERM);
				unlink(RADVD_PID); //remove pid file, since if the pid file still existing will impact next new radvd process
			}
			va_cmd( "/bin/radvd", 3, 0, "-s", "-C", (char *)RADVD_CONF );
		}
		start_dhcpv6_new(&dnsV6Info,&prefixInfo);
	}
}

void restartDHCPV6Server()
{
	DNS_V6_INFO_T dnsV6Info={0};
	PREFIX_V6_INFO_T prefixInfo={0};
	unsigned IPv6Enable=0,dhcpV6Enable=0;

	get_dnsv6_info(&dnsV6Info);
	get_prefixv6_info(&prefixInfo);
	start_dhcpv6_new(&dnsV6Info,&prefixInfo);
}



//Helper function for DNSv6 mode
int get_dnsv6_info(DNS_V6_INFO_Tp dnsV6Info)
{
	unsigned char ipv6DnsMode=0;
	unsigned char tmpBuf[100]={0},dnsv6_1[64]={0},dnsv6_2[64]={0} ;
	unsigned char leasefile[30];
	unsigned int wanconn=0;
	DLG_INFO_T dlgInfo={0};

	if(!dnsV6Info){
		printf("Error! NULL input dnsV6Info\n");
		goto setErr_ipv6;
	}
	if ( !mib_get(MIB_LAN_DNSV6_MODE, (void *)&ipv6DnsMode)) {
		printf("Error!! get LAN IPv6 DNS Mode fail!");
		goto setErr_ipv6;
	}

	dnsV6Info->mode = ipv6DnsMode;
	switch(dnsV6Info->mode)
	{ 
		case IPV6_DNS_HGWPROXY:
			if (!mib_get(MIB_IPV6_LAN_IP_ADDR, (void *)tmpBuf)) {
				printf("Error!! Get LAN IPv6 Address fail!");
				goto setErr_ipv6;
			}
			if(tmpBuf[0])
				strcpy(dnsV6Info->nameServer,tmpBuf);

			printf("IPV6_DNS_HGWPROXY,with nameServer %s\n",dnsV6Info->nameServer);		
			break;
		case IPV6_DNS_WANCONN:
			if (!mib_get(MIB_DNSINFO_WANCONN, (void *)&wanconn)) {
				printf("Error!! Get DNS WANCONN fail!");
				goto setErr_ipv6;
			}

			if(wanconn)
				dnsV6Info->wanconn = wanconn;

			printf("wanconn is %d=0x%x\n",wanconn,wanconn);
			ifGetName(dnsV6Info->wanconn, tmpBuf, sizeof(tmpBuf));
			snprintf(leasefile, 30, "/var/%s%s.leases", DHCPCV6STR, tmpBuf);
			if(getLeasesInfo(leasefile,&dlgInfo)){
				strcpy(dnsV6Info->nameServer,dlgInfo.nameServer);
				strncpy(dnsV6Info->leaseFile, leasefile, IPV6_BUF_SIZE_128);
			}
			else{
				printf("Error! Could not get delegation info from wanconn %s, file %s!\n", tmpBuf, leasefile);
				goto setErr_ipv6;
			}
			printf("IPV6_DNS_WANCONN,with nameServer %s\n",dnsV6Info->nameServer);		
			break;
		case IPV6_DNS_STATIC:
			if (!mib_get(MIB_ADSL_WAN_DNSV61, (void *)dnsv6_1)) {
				printf("Error!! Get DNS Server Address 1 fail!");
				goto setErr_ipv6;
			}

			//DNSV61,DNSV62 is in IA_6 format
			inet_ntop(PF_INET6,dnsv6_1, tmpBuf, sizeof(tmpBuf));
			strcpy(dnsv6_1,tmpBuf);

			if (!mib_get(MIB_ADSL_WAN_DNSV62, (void *)dnsv6_2)) {
				printf("Error!! Get DNS Server Address 2 fail!");
				goto setErr_ipv6;
			}
			inet_ntop(PF_INET6,dnsv6_2, tmpBuf, sizeof(tmpBuf));
			strcpy(dnsv6_2,tmpBuf);

			if(dnsv6_1[0]&&dnsv6_2[0])
				snprintf(dnsV6Info->nameServer,IPV6_BUF_SIZE_256,"%s,%s",dnsv6_1,dnsv6_2);
			else if(dnsv6_1[0])
				snprintf(dnsV6Info->nameServer,IPV6_BUF_SIZE_256,"%s",dnsv6_1);

			printf("IPV6_DNS_STATIC,with nameServer %s\n",dnsV6Info->nameServer);		
			break;
		default:
			printf("Error! Should not go to here!!\n");
			goto setErr_ipv6;
	}

	return 0;

setErr_ipv6:
	return -1;
}

//Helper function for PrefixV6 mode
int get_prefixv6_info(PREFIX_V6_INFO_Tp prefixInfo)
{
	unsigned char ipv6PrefixMode=0, prefixLen;
	unsigned char tmpBuf[100]={0};
	unsigned char leasefile[30];
	unsigned int wanconn=0;
	DLG_INFO_T dlgInfo={0};

	if(!prefixInfo){
		printf("Error! NULL input prefixV6Info\n");
		goto setErr_ipv6;
	}
	if ( !mib_get(MIB_PREFIXINFO_PREFIX_MODE, (void *)&ipv6PrefixMode)) {
		printf("Error!! get LAN IPv6 Prefix Mode fail!");
		goto setErr_ipv6;
	}

	prefixInfo->mode = ipv6PrefixMode;

	switch(prefixInfo->mode){
		case IPV6_PREFIX_DELEGATION:
			if (!mib_get(MIB_PREFIXINFO_DELEGATED_WANCONN, (void *)&wanconn)) {
				printf("Error!! Get PREFIXINFO_DELEGATED WANCONN fail!");
				goto setErr_ipv6;
			}
			if(wanconn)
				prefixInfo->wanconn = wanconn;

			printf("wanconn is %d=0x%x\n",wanconn,wanconn);
			ifGetName(prefixInfo->wanconn, tmpBuf, sizeof(tmpBuf));
			snprintf(leasefile, 30, "/var/%s%s.leases", DHCPCV6STR, tmpBuf);
			if(getLeasesInfo(leasefile,&dlgInfo)){
				memcpy(prefixInfo->prefixIP,dlgInfo.prefixIP,sizeof(prefixInfo->prefixIP));
				prefixInfo->RNTime = dlgInfo.RNTime;
				prefixInfo->RBTime = dlgInfo.RBTime;
				prefixInfo->PLTime = dlgInfo.PLTime;
				prefixInfo->MLTime = dlgInfo.MLTime;
				prefixInfo->prefixLen = dlgInfo.prefixLen;
				//     IPv6 network  may give prefix with length 56 by prefix delegation, 
				//     but only prefix length = 64, SLAAC will work.
				//
				//Ref: rfc4862: Section 5.5.3.  Router Advertisement Processing
				//     If the sum of the prefix length and interface identifier length
				//     does not equal 128 bits, the Prefix Information option MUST be
				//     ignored. 
				if( prefixInfo->prefixLen!=64 )
					prefixInfo->prefixLen=64;
				strncpy(prefixInfo->leaseFile, leasefile, IPV6_BUF_SIZE_128);
			}
			else{
				printf("Error! Could not get delegation info from wanconn %s, file %s!\n", tmpBuf, leasefile);
				goto setErr_ipv6;
			}
			inet_ntop(PF_INET6,prefixInfo->prefixIP, tmpBuf, sizeof(tmpBuf));
			printf("IPV6_PREFIX_DELEGATION, with prefix %s/%d\n",tmpBuf,prefixInfo->prefixLen);		
			break;
		case IPV6_PREFIX_STATIC:
			if (!mib_get(MIB_IPV6_LAN_PREFIX, (void *)tmpBuf)) { //STRING_T
				printf("Error!! Get MIB_IPV6_LAN_PREFIX fail!");
				goto setErr_ipv6;
			}

			if (!mib_get(MIB_IPV6_LAN_PREFIX_LEN, (void *)&prefixLen)) {
				printf("Error!! Get MIB_IPV6_LAN_PREFIX_LEN fail!");
				goto setErr_ipv6;
			}
			if(tmpBuf[0]){
				if ( !inet_pton(PF_INET6, tmpBuf, prefixInfo->prefixIP) ) 
					goto setErr_ipv6;
			}

			// AdvValidLifetime
			if ( !mib_get(MIB_V6_VALIDLIFETIME, (void *)tmpBuf)) {
				printf("Get AdvValidLifetime mib error!");
				goto setErr_ipv6;
			}

			if(tmpBuf[0])
				prefixInfo->MLTime=atoi(tmpBuf);
				
			// AdvPreferredLifetime
			if ( !mib_get(MIB_V6_PREFERREDLIFETIME, (void *)tmpBuf)) {
				printf("Get AdvPreferredLifetime mib error!");
				goto setErr_ipv6;
			}
			if(tmpBuf[0])
				prefixInfo->PLTime=atoi(tmpBuf);

			prefixInfo->prefixLen = prefixLen;
			inet_ntop(PF_INET6,prefixInfo->prefixIP, tmpBuf, sizeof(tmpBuf));
			printf("IPV6_PREFIX_STATIC, with prefix %s::/%d\n",tmpBuf,prefixInfo->prefixLen);		
			break;
		case IPV6_PREFIX_PPPOE:
		case IPV6_PREFIX_NONE:
		default:
			printf("Error! Not support this mode %d!\n");
	}
	return 0;

setErr_ipv6:
	return -1;
}

int addIPv6RemoteAccRule(char *ifname, struct in6_addr *ip6addr)
{
	char v6addrbuf[64]={0};
	char cmd[256]={0};

	if(!ifname || !ip6addr){
		printf("%s Error! Input is NULL!\n",__func__);
		return 0;
	}

	inet_ntop(PF_INET6, ip6addr, v6addrbuf, sizeof(v6addrbuf));

	// ip6tables -A ipv6remoteacc -m state --state ESTABLISHED,RELATED -j RETURN
	va_cmd(IP6TABLES, 8, 1, (char *)FW_ADD, (char *)FW_IPV6REMOTEACC, "-m", "state",
			"--state", "ESTABLISHED,RELATED", "-j", (char *)FW_RETURN);

#if 0 //Maybe could not ping from outside is better, so mark it. but leave for  future may change policy.

	//ip6tables -A ipv6remoteacc -i ppp0 -p ipv6-icmp  -d 2001:b011:7006:1b95:237:7ff:fe11:2233  -j ACCEPT
	va_cmd(IP6TABLES, 10, 1, (char *)FW_ADD, (char *)FW_IPV6REMOTEACC, (char *)ARG_I, (char *)ifname,
			"-p", "ipv6-icmp",  "-d", v6addrbuf,   "-j", (char *)FW_ACCEPT);
#endif
	
	//ip6tables -A ipv6remoteacc -i ppp0  -d 2001:b011:7006:1b95:237:7ff:fe11:2233 -j DROP
	va_cmd(IP6TABLES, 8, 1, (char *)FW_ADD, (char *)FW_IPV6REMOTEACC, (char *)ARG_I, (char *)ifname,
		 "-d", v6addrbuf,   "-j", (char *)FW_DROP);

	return 1;
}

#endif //#ifdef CONFIG_IPV6

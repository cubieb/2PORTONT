/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>

#include "../webs.h"
#include "webform.h"
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

#ifdef CONFIG_IPV6
static const char RADVD_NEW_CONF[] ="/var/radvd2.conf";
static const char DHCPDV6_NEW_CONF[] ="/var/dhcpd6_new_conf";
static const char KEYWORD1[]= "RDNSS ";                    //RADVD use, the last character(space) is necessary
static const char KEYWORD2[]= "option dhcp6.name-servers"; //DHCPv6 Server use!

static void kickRAChangeRDNSS(char *RDNSS)
{
	FILE *fp=NULL;
	FILE *nfp=NULL;
	char buf[512];
	int radvdpid;

	if ((fp = fopen(RADVD_CONF, "r")) == NULL) {
		printf("Open file %s Error!\n", RADVD_CONF);
		return;
	}

	if ((nfp = fopen(RADVD_NEW_CONF, "w")) == NULL) {
		printf("Open file %s Error!\n", RADVD_NEW_CONF);
		fclose(fp);
		return;
	}

	while(fgets(buf,sizeof(buf),fp)>0) {
		if(strstr(buf,KEYWORD1))
			fprintf(nfp,"\t%s %s\n",KEYWORD1,RDNSS);
		else
			fprintf(nfp,"%s",buf);
	}

	fclose(fp);
	fclose(nfp);

	sprintf(buf,"cp %s %s\n",RADVD_NEW_CONF, RADVD_CONF);
	system(buf);

	radvdpid = read_pid((char *)RADVD_PID);
	if (radvdpid > 0) {
		kill(radvdpid, SIGHUP);
	}
}

static void replaceDHCPv6Conf(char *oldFileName, char *newFileName, char *keywordStr, char *replaceStr)
{
	FILE *fp=NULL;
	FILE *nfp=NULL;
	char buf[512];

	if ((fp = fopen(oldFileName, "r")) == NULL) {
		printf("Open file %s Error!\n", oldFileName);
		return;
	}

	if ((nfp = fopen(newFileName, "w")) == NULL) {
		printf("Open file %s Error!\n", newFileName);
		fclose(fp);
		return;
	}

	while(fgets(buf,sizeof(buf),fp)>0) {
		if(strstr(buf, keywordStr))
			fprintf(nfp,"%s %s;\n",keywordStr,replaceStr);
		else
			fprintf(nfp,"%s",buf);
	}

	fclose(fp);
	fclose(nfp);

	sprintf(buf,"cp %s %s\n",newFileName, oldFileName);
	system(buf);

}

static void kickDHCPv6ChangeRDNSS(char *RDNSS)
{
	replaceDHCPv6Conf(DHCPDV6_CONF_AUTO, DHCPDV6_NEW_CONF, KEYWORD2, RDNSS);
	replaceDHCPv6Conf(DHCPDV6_CONF, DHCPDV6_NEW_CONF, KEYWORD2, RDNSS);

	start_dhcpv6(1);	
}

static void kickDNSRelayChangeOnTheFly()
{
	int dnsv6RelayPid = read_pid((char *)DNSRELAYPID);
	if ( dnsv6RelayPid> 0) {
		kill(dnsv6RelayPid, SIGTERM);
		unlink(DNSRELAYPID);
	}

	if (va_cmd(DNSRELAY, 2, 0, "-r", RESOLV))
	    return -1;
}

void formlanipv6(request * wp, char *path, char *query)
{
	char *submitUrl;
	static char tmpBuf[100];
	static char cmdBuf[100];
	char *lanIPv6AddrStr=NULL;

	if (mib_get(MIB_IPV6_LAN_IP_ADDR, (void *)tmpBuf) != 0) {
		char cmdBuf[48];
		sprintf(cmdBuf, "%s/%d", tmpBuf, 64);
		va_cmd(IFCONFIG, 3, 1, LANIF, ARG_DEL, cmdBuf);
	}
	lanIPv6AddrStr = boaGetVar(wp, "lanIpv6addr", "");

	if(!lanIPv6AddrStr) {
		strcpy(tmpBuf, "不合法的LAN IPv6地址!"); //Invalid LAN IPv6 address!
		goto setErr_ipv6;
	}

	if ( !mib_set(MIB_IPV6_LAN_IP_ADDR, (void *)lanIPv6AddrStr)) {
		strcpy(tmpBuf, "LAN IPv6地址设定错误!"); //set LAN IPv6 address fail!
		goto setErr_ipv6;
	}

	sprintf(cmdBuf, "%s/%d", lanIPv6AddrStr, 64);
	va_cmd(IFCONFIG, 3, 1, LANIF, ARG_ADD, cmdBuf);

	sleep(3);
	kickRAChangeRDNSS(lanIPv6AddrStr);
	kickDHCPv6ChangeRDNSS(lanIPv6AddrStr);
	kickDNSRelayChangeOnTheFly();

setOk_ipv6:

#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif

	submitUrl = boaGetVar(wp, "submit-url", "/");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
	return;

setErr_ipv6:
	ERR_MSG(tmpBuf);
}


void formlanipv6dns(request * wp, char *path, char *query)
{
	char *str_extif,*submitUrl,*str_dns1,*str_dns2;
	static char tmpBuf[100];
	static char cmdBuf[100];
	char *lanIPv6DnsModeStr=NULL;
	unsigned char ipv6DnsMode=0;
	unsigned int ext_if;
	struct in6_addr dnsv61, dnsv62, dnsv63;

	lanIPv6DnsModeStr = boaGetVar(wp, "ipv6landnsmode", "");
	str_extif = boaGetVar(wp, "ext_if", "");

	if(!lanIPv6DnsModeStr) {
		goto setErr_ipv6;
	}

	ipv6DnsMode = lanIPv6DnsModeStr[0]-'0';

	if ( !mib_set(MIB_LAN_DNSV6_MODE, (void *)&ipv6DnsMode)) {
		strcpy(tmpBuf, "Error!! set LAN IPv6 DNS Mode fail!");
		goto setErr_ipv6;
	}

	switch(ipv6DnsMode){
		case IPV6_DNS_HGWPROXY:
			break;
		case IPV6_DNS_WANCONN:
			if(str_extif[0])
				ext_if = (unsigned int)atoi(str_extif);
			else
				ext_if = DUMMY_IFINDEX;  // No interface selected.

			printf("DNS WANConnect at 0x%x\n",ext_if);
			if ( !mib_set(MIB_DNSINFO_WANCONN, (void *)&ext_if)) {
				strcpy(tmpBuf, "Error!! set LAN IPv6 DNS WAN Conn fail!");
				goto setErr_ipv6;
			}

			break;
		case IPV6_DNS_STATIC:
			memset(dnsv61.s6_addr, 0, 16);
			memset(dnsv62.s6_addr, 0, 16);
			str_dns1 = boaGetVar(wp, "Ipv6Dns1", "");
			if(str_dns1) {
				if ( !inet_pton(PF_INET6, str_dns1, &dnsv61) ) {
					strcpy(tmpBuf, Tinvalid_DNS_address);
					goto setErr_ipv6;
				}
				if ( !mib_set(MIB_ADSL_WAN_DNSV61, (void *)&dnsv61)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_ipv6;
				}
			}

			str_dns2 = boaGetVar(wp, "Ipv6Dns2", "");
			if(str_dns2) {
				if ( !inet_pton(PF_INET6, str_dns2, &dnsv62) ) {
					strcpy(tmpBuf, Tinvalid_DNS_address);
					goto setErr_ipv6;
				}
				if ( !mib_set(MIB_ADSL_WAN_DNSV62, (void *)&dnsv62)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_ipv6;
				}
			}

			break;
		default:
			break;
	}

setOk_ipv6:

#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif

	//TODO: re-modify dhcpd6, radvd conf and restart
	restartLanV6Server();


	submitUrl = boaGetVar(wp, "submit-url", "/");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
	return;

setErr_ipv6:
	ERR_MSG(tmpBuf);
}

void formlanipv6prefix(request * wp, char *path, char *query)
{
	char *submitUrl, *str_extif;
	static char tmpBuf[100];
	char *lanIPv6PrefixStr=NULL,*lanIPv6PrefixModeStr=NULL;
	char *tok=NULL, *prefixStr=NULL, *saveptr1=NULL;
	char len=0;
	char lanIPv6PrefixMode;
	unsigned int ext_if;

	lanIPv6PrefixModeStr = boaGetVar(wp, "ipv6lanprefixmode", "");
	lanIPv6PrefixMode = lanIPv6PrefixModeStr[0]-'0';
	printf("[%s] lanIPv6PrefixMode=%d\n",__func__,lanIPv6PrefixMode);
	if ( !mib_set(MIB_PREFIXINFO_PREFIX_MODE, (void *)&lanIPv6PrefixMode)) {
		strcpy(tmpBuf, "LAN IPv6前缀模式设定错误!"); //set LAN IPv6 prefix mode fail!
		goto setErr_ipv6;
	}

	switch(lanIPv6PrefixMode){
		case IPV6_PREFIX_DELEGATION:
			//MIB_PREFIXINFO_DELEGATED_WANCONN
			str_extif = boaGetVar(wp, "ext_if", "");
		if(str_extif[0])
			ext_if = (unsigned int)atoi(str_extif);
		else
			ext_if = DUMMY_IFINDEX;  // No interface selected.

		printf("Prefix is delegated WAN at %d\n",ext_if);
		if(!mib_set(MIB_PREFIXINFO_DELEGATED_WANCONN, (void *)&ext_if))
		{
			strcpy(tmpBuf, "Set PrefixInfo Delegated wanconn error!");
			goto setErr_ipv6;
		}

		break;

		case IPV6_PREFIX_STATIC:
	lanIPv6PrefixStr = boaGetVar(wp, "lanIpv6prefix", "");

	if(!lanIPv6PrefixStr) {
		strcpy(tmpBuf, "不合法的LAN IPv6前缀!"); //Invalid LAN IPv6 Prefix!
		goto setErr_ipv6;
	}

	//Prefix will be like 2001:2222::/64 form	
	prefixStr = strtok_r(lanIPv6PrefixStr,"/",&saveptr1);
	tok = strtok_r(NULL,"/",&saveptr1);
	len = atoi(tok);

	if(!prefixStr) {
		strcpy(tmpBuf, "不合法的LAN IPv6前缀!"); //Invalid LAN IPv6 Prefix!
		goto setErr_ipv6;
	}
	
	if((len>64) || (len<48) ) {
		strcpy(tmpBuf, "不合法的LAN IPv6前缀长度!"); //Invalid LAN IPv6 Prefix Len!
		goto setErr_ipv6;
	}

	if ( !mib_set(MIB_IPV6_LAN_PREFIX, (void *)prefixStr)) {
		strcpy(tmpBuf, "LAN IPv6前缀设定错误!"); //set LAN IPv6 prefix fail!
		goto setErr_ipv6;
	}

	if ( !mib_set(MIB_IPV6_LAN_PREFIX_LEN, (void *)&len )) {
		strcpy(tmpBuf, "LAN IPv6前缀长度设定错误!"); //set LAN IPv6 prefix length fail!
		goto setErr_ipv6;
	}

			break;
	}

setOk_ipv6:

#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif

	restartLanV6Server();

	submitUrl = boaGetVar(wp, "submit-url", "/");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
	return;

setErr_ipv6:
	ERR_MSG(tmpBuf);
}

#endif  //#ifdef CONFIG_IPV6


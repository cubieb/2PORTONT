/*-- System inlcude files --*/
#include <string.h>
#include <signal.h>
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "multilang.h"


#ifdef CONFIG_IPV6
///////////////////////////////////////////////////////////////////
//

const char RADVD_NEW_CONF[] ="/var/radvd2.conf";
const char KEYWORD1[]= "AdvValidLifetime";
const char KEYWORD2[]= "AdvPreferredLifetime";

void modifyRAConf(void)
{
#ifdef CONFIG_USER_RADVD
	FILE *fp=NULL;
	FILE *nfp=NULL;
	char buf[512];
	int radvdpid;

	if ((fp = fopen(RADVD_CONF, "r")) == NULL)
	{
		printf("Open file %s Error!\n", RADVD_CONF);
		return;
	}

	if ((nfp = fopen(RADVD_NEW_CONF, "w")) == NULL)
	{
		printf("Open file %s Error!\n", RADVD_NEW_CONF);
		fclose(fp);
		return;
	}

	while(fgets(buf,sizeof(buf),fp)>0)
	{
		if(strstr(buf,KEYWORD1))
			fprintf(nfp,"\t\t%s 0;\n",KEYWORD1);
		else if (strstr(buf,KEYWORD2))
			fprintf(nfp,"\t\t%s 0;\n",KEYWORD2);
		else
			fprintf(nfp,"%s",buf);
	}

	fclose(fp);
	fclose(nfp);

	sprintf(buf,"cp %s %s\n",RADVD_NEW_CONF, RADVD_CONF);
	system(buf);
	radvdpid = read_pid((char *)RADVD_PID);
	if (radvdpid > 0) {
		printf("Resync RADVD.\n");
		kill(radvdpid, SIGHUP);
	}
#endif
}


void doIPv6Restart(unsigned char is_enabled)
{
	printf("IPv6 enable status: %d\n",is_enabled);

	printf("Now delect All Connection\n");
	deleteConnection(CONFIGALL, NULL);

	if(is_enabled==0)
	{
		//If WAN PPPoE connection is down, LAN PC's IPv6 address need to 
		//change from  from "Prefered" to "Obsolete"
		//So lifetime of previous prefix is need to be set to 0.
		printf("Now preparing to disable IPV6...re setup radvd..\n");
		modifyRAConf();

		printf("Now kill dhcpd6\n");
		system("kill `cat /var/run/dhcpd6.pid`");
		system("kill `cat /var/run/radvd.pid`");
		system("rm /var/run/dhcpd6.pid");
		system("rm /var/run/radvd.pid");

		system("/bin/echo 1 > /proc/sys/net/ipv6/conf/all/disable_ipv6");
	}
	else
	{
		system("/bin/echo 0 > /proc/sys/net/ipv6/conf/all/disable_ipv6");
	}
	
	printf("Now restart WAN\n");	
	restartWAN(CONFIGALL, NULL);
}


void formIPv6EnableDisable(request * wp, char *path, char *query)
{
	char	*str_enb, *str_extif, *submitUrl;
	char tmpBuf[100];
	FILE *fp;
	char * argv[8];
	char ifname[6];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char is_enabled, pre_enabled;
	unsigned int ext_if, pre_ext_if;
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif

	str_enb = boaGetVar(wp, "ipv6switch", "");

	if(str_enb[0])
	{
		if (str_enb[0] == '0')
			is_enabled = 0;
		else
			is_enabled = 1;

		printf("IPv6 enable status: %d\n",is_enabled);
		if(!mib_set(MIB_V6_IPV6_ENABLE, (void *)&is_enabled))
		{
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_igmp;
		}
	}

	printf("IPv6 enable status: %d\n",is_enabled);
	doIPv6Restart(is_enabled);
	va_cmd(IFCONFIG, 2, 1, "br0", "down");
	va_cmd(IFCONFIG, 2, 1, "br0", "up");

#ifdef CONFIG_USER_RADVD
	if(is_enabled == 1)
	{
		unsigned char str[MAX_RADVD_CONF_PREFIX_LEN];
		unsigned char str2[MAX_RADVD_CONF_PREFIX_LEN];
		unsigned char vChar;
		mib_get (MIB_V6_ULAPREFIX_ENABLE, (void *)&vChar);
		if (vChar!=0) {
			if ( !mib_get(MIB_V6_ULAPREFIX, (void *)str)       ||
					!mib_get(MIB_V6_ULAPREFIX_LEN, (void *)str2)  )
			{
				printf("Get ULAPREFIX mib error!");
			}
			else
			{
				unsigned char ip6Addr[IP6_ADDR_LEN];
				unsigned char devAddr[MAC_ADDR_LEN];
				unsigned char meui64[8];
				unsigned char value[64];
				int i;

				inet_pton(PF_INET6, str, (void *) ip6Addr);
				//setup LAN ULA v6 IP address according the ULA prefix + EUI64.
				mib_get(MIB_ELAN_MAC_ADDR, (void *)devAddr);
				mac_meui64(devAddr, meui64);
				for (i=0; i<8; i++)
					ip6Addr[i+8] = meui64[i];
				inet_ntop(PF_INET6, &ip6Addr, value, sizeof(value));
				sprintf(value, "%s/%s", value, str2);
				printf("Set LAN ULA %s\n",value);
				va_cmd(IFCONFIG, 3, 1, LANIF, "del", value);
				va_cmd(IFCONFIG, 3, 1, LANIF, "add", value);
			}
		}
	}
#endif
   
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG(submitUrl);
	return;

setErr_igmp:
	ERR_MSG(tmpBuf);
}

#endif

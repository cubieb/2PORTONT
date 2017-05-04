/*
 *      Web server handler routines for DNS stuffs
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <net/route.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"

void formDns(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100];
	char vChar;
	int dns_changed=0;
#ifndef NO_ACTION
	int pid;
#endif
#ifdef DNS_BIND_PVC_SUPPORT
	DnsBindPvcRoute(DELDNSROUTE);
#endif
#ifdef CONFIG_IPV6
#ifdef DNSV6_BIND_PVC_SUPPORT
	Dnsv6BindPvcRoute(DELDNSROUTE);
#endif
#endif
	str = boaGetVar(wp, "dnsMode", "");
	if ( str[0] ) {
		DNS_TYPE_T dns, dns_old;

		if (!strcmp(str, "dnsAuto"))
			dns = DNS_AUTO;
		else if (!strcmp(str, "dnsManual"))
			dns = DNS_MANUAL;
		else {
			strcpy(tmpBuf, Tinvalid_DNS_mode);
			goto setErr_dns;
		}

		if ( !mib_get(MIB_ADSL_WAN_DNS_MODE, (void *)&vChar)) {
	  		strcpy(tmpBuf, TDNS_mib_get_error);
			goto setErr_dns;
		}
		dns_old = (DNS_TYPE_T)vChar;
		
		//jim strict check dns changed....
		if ( !mib_get(MIB_ADSL_WAN_DNS1, (void *)&vChar)) {
	  		strcpy(tmpBuf, TDNS_mib_get_error);
			goto setErr_dns;
		}

		// Set DNS to MIB
		vChar = (unsigned char) dns;
		if ( !mib_set(MIB_ADSL_WAN_DNS_MODE, (void *)&vChar)) {
	  		strcpy(tmpBuf, TDNS_mib_set_error);
			goto setErr_dns;
		}

		if (dns != dns_old) {
			dns_changed = 1;
			cleanAll_SourceRoute();
			reWriteAllDhcpcScript();
		}
		
		if ( dns == DNS_MANUAL ) {
			struct in_addr dns1, dns2, dns3;
			//jim
			struct in_addr dns1_old, dns2_old, dns3_old;
			dns1.s_addr=0;
			dns2.s_addr=0;
			dns3.s_addr=0;
			// If DHCP server is enabled in LAN, update dhcpd.conf
			str = boaGetVar(wp, "dns1", "");
			if ( str[0] ) {
				if ( !inet_aton(str, &dns1) ) {
					strcpy(tmpBuf, Tinvalid_DNS_address);
					goto setErr_dns;
				}
				//jim
				if ( !mib_get(MIB_ADSL_WAN_DNS1, (void *)&dns1_old)) {
	  				strcpy(tmpBuf, TDNS_mib_get_error);
					goto setErr_dns;
				}
				if(dns1.s_addr!=dns1_old.s_addr)
					dns_changed = 1;

				if ( !mib_set(MIB_ADSL_WAN_DNS1, (void *)&dns1)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_dns;
				}
				#ifdef DNS_BIND_PVC_SUPPORT
				str = boaGetVar(wp, "enableDnsBind", "");
				if(str[0]){//dns bind enabled
				   unsigned char enableDnsBind=1;
				   if(!mib_set(MIB_DNS_BIND_PVC_ENABLE,(void *)&enableDnsBind))
				   	{
				   	printf("set MIB_DNS_BIND_PVC_ENABLE error!\n");
					strcpy(tmpBuf, TDNS_mib_set_error);
							goto setErr_dns;
				   	}
				    str=boaGetVar(wp,"dnspvc1","");
				    printf("dns1 interface %d\n",atoi(str));
				    unsigned int dns1itfIndex=( unsigned int )atoi(str);
				if ( !mib_set(MIB_DNS_BIND_PVC1, (void *)&dns1itfIndex)) {
			  				strcpy(tmpBuf, TDNS_mib_set_error);
							goto setErr_dns;
					}
				}else{
				   unsigned char enableDnsBind=0;
				   if(!mib_set(MIB_DNS_BIND_PVC_ENABLE,&enableDnsBind))
				   	{
				   	printf("set MIB_DNS_BIND_PVC_ENABLE error!\n");
					strcpy(tmpBuf, TDNS_mib_set_error);
							goto setErr_dns;
				   	}
					}
				#endif
			}
			else {
				goto setOk_dns;
			}
			str = boaGetVar(wp, "dns2", "");
			if ( str[0] ) {
				if ( !inet_aton(str, &dns2) ) {
					strcpy(tmpBuf, Tinvalid_DNS_address);
					goto setErr_dns;
				}
				//jim
				if ( !mib_get(MIB_ADSL_WAN_DNS2, (void *)&dns2_old)) {
	  				strcpy(tmpBuf, TDNS_mib_get_error);
					goto setErr_dns;
				}
				if(dns2.s_addr!=dns2_old.s_addr)
					dns_changed = 1;

				if ( !mib_set(MIB_ADSL_WAN_DNS2, (void *)&dns2)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_dns;
				}
				#ifdef DNS_BIND_PVC_SUPPORT
				str = boaGetVar(wp, "enableDnsBind", "");
				if(str[0]){//dns bind enabled
				    str=boaGetVar(wp,"dnspvc2","");
				    printf("dns2 interface %d\n",atoi(str));
				    unsigned int dns2itfIndex=( unsigned int )atoi(str);
				if ( !mib_set(MIB_DNS_BIND_PVC2, (void *)&dns2itfIndex)) {
			  				strcpy(tmpBuf, TDNS_mib_set_error);
							goto setErr_dns;
					}
				}
				#endif
			}
			else {
				//jim
				if ( !mib_get(MIB_ADSL_WAN_DNS2, (void *)&dns2_old)) {
	  				strcpy(tmpBuf, TDNS_mib_get_error);
					goto setErr_dns;
				}
				if(dns2.s_addr!=dns2_old.s_addr)
					dns_changed = 1;

				if ( !mib_set(MIB_ADSL_WAN_DNS2, (void *)&dns2)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_dns;
				}
			}
			str = boaGetVar(wp, "dns3", "");
			if ( str[0] ) {
				if ( !inet_aton(str, &dns3) ) {
					strcpy(tmpBuf, Tinvalid_DNS_address);
					goto setErr_dns;
				}
				//jim
				if ( !mib_get(MIB_ADSL_WAN_DNS3, (void *)&dns3_old)) {
	  				strcpy(tmpBuf, TDNS_mib_get_error);
					goto setErr_dns;
				}
				if(dns3.s_addr!=dns3_old.s_addr)
					dns_changed = 1;
				if ( !mib_set(MIB_ADSL_WAN_DNS3, (void *)&dns3)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_dns;
				}
				#ifdef DNS_BIND_PVC_SUPPORT
				str = boaGetVar(wp, "enableDnsBind", "");
				if(str[0]){//dns bind enabled
				    str=boaGetVar(wp,"dnspvc3","");
				    printf("dns3 interface %d\n",atoi(str));
				    unsigned int dns3itfIndex=( unsigned int )atoi(str);
				if ( !mib_set(MIB_DNS_BIND_PVC3, (void *)&dns3itfIndex)) {
			  				strcpy(tmpBuf, TDNS_mib_set_error);
							goto setErr_dns;
					}
				}
				#endif
			}
			else {
				//jim
				if ( !mib_get(MIB_ADSL_WAN_DNS3, (void *)&dns3_old)) {
	  				strcpy(tmpBuf, TDNS_mib_get_error);
					goto setErr_dns;
				}
				if(dns3.s_addr!=dns3_old.s_addr)
					dns_changed = 1;

				if ( !mib_set(MIB_ADSL_WAN_DNS3, (void *)&dns3)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_dns;
				}
			}
		#ifdef DNS_BIND_PVC_SUPPORT
		 DnsBindPvcRoute(ADDDNSROUTE);
		#endif
		}
	}

#ifdef CONFIG_IPV6
	str = boaGetVar(wp, "dnsMode", "");
	if ( str[0] ) {
		DNS_TYPE_T dns, dns_old;

		if (!strcmp(str, "dnsAuto"))
			dns = DNS_AUTO;
		else if (!strcmp(str, "dnsManual"))
			dns = DNS_MANUAL;
		else {
			strcpy(tmpBuf, Tinvalid_DNS_mode);
			goto setErr_dns;
		}

		if ( !mib_get(MIB_ADSL_WAN_DNS_MODE, (void *)&vChar)) {
	  		strcpy(tmpBuf, TDNS_mib_get_error);
			goto setErr_dns;
		}
		dns_old = (DNS_TYPE_T)vChar;
		if (dns != dns_old)
			dns_changed = 1;
		//jim strict check dns changed....
		if ( !mib_get(MIB_ADSL_WAN_DNS1, (void *)&vChar)) {
	  		strcpy(tmpBuf, TDNS_mib_get_error);
			goto setErr_dns;
		}

		// Set DNS to MIB
		vChar = (unsigned char) dns;
		if ( !mib_set(MIB_ADSL_WAN_DNS_MODE, (void *)&vChar)) {
	  		strcpy(tmpBuf, TDNS_mib_set_error);
			goto setErr_dns;
		}

		if ( dns == DNS_MANUAL ) {
			struct in6_addr dnsv61, dnsv62, dnsv63;
			//jim
			struct in6_addr dnsv61_old, dnsv62_old, dnsv63_old;
			memset(dnsv61.s6_addr, 0, 16);
			memset(dnsv62.s6_addr, 0, 16);
			memset(dnsv63.s6_addr, 0, 16);

			// If DHCP server is enabled in LAN, update dhcpd.conf
			str = boaGetVar(wp, "dnsv61", "");
			if ( str[0] ) {
				if ( !inet_pton(PF_INET6, str, &dnsv61) ) {
					strcpy(tmpBuf, Tinvalid_DNS_address);
					goto setErr_dns;
				}
				//jim
				if ( !mib_get(MIB_ADSL_WAN_DNSV61, (void *)&dnsv61_old)) {
	  				strcpy(tmpBuf, TDNS_mib_get_error);
					goto setErr_dns;
				}
				//if(dns1.s_addr!=dns1_old.s_addr)
				if ( memcmp( dnsv61.s6_addr, dnsv61_old.s6_addr, 16) == 0 )
					dns_changed = 1;

				if ( !mib_set(MIB_ADSL_WAN_DNSV61, (void *)&dnsv61)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_dns;
				}
				#ifdef DNSV6_BIND_PVC_SUPPORT
				str = boaGetVar(wp, "enableDnsv6Bind", "");
				if(str[0]){//dns bind enabled
				   unsigned char enableDnsv6Bind=1;
				   if(!mib_set(MIB_DNSV6_BIND_PVC_ENABLE,(void *)&enableDnsv6Bind))
				   	{
						printf("set MIB_DNSV6_BIND_PVC_ENABLE error!\n");
						strcpy(tmpBuf, TDNS_mib_set_error);
						goto setErr_dns;
				   	}
				    str=boaGetVar(wp,"dnsv6pvc1","");
				    printf("dnsv61 interface %d\n",atoi(str));
				    unsigned int dnsv61itfIndex=( unsigned int )atoi(str);
					if ( !mib_set(MIB_DNSV6_BIND_PVC1, (void *)&dnsv61itfIndex)) {
			  			strcpy(tmpBuf, TDNS_mib_set_error);
						goto setErr_dns;
					}
				}else{
				   unsigned char enableDnsv6Bind=0;
				   if(!mib_set(MIB_DNSV6_BIND_PVC_ENABLE,&enableDnsv6Bind))
				   	{
				   	printf("set MIB_DNSV6_BIND_PVC_ENABLE error!\n");
					strcpy(tmpBuf, TDNS_mib_set_error);
							goto setErr_dns;
				   	}
					}
				#endif
			}
			else {
				goto setOk_dns;
			}

			str = boaGetVar(wp, "dnsv62", "");
			if ( str[0] ) {
				if ( !inet_pton(PF_INET6, str, &dnsv62) ) {
					strcpy(tmpBuf, Tinvalid_DNS_address);
					goto setErr_dns;
				}
				//jim
				if ( !mib_get(MIB_ADSL_WAN_DNSV62, (void *)&dnsv62_old)) {
	  				strcpy(tmpBuf, TDNS_mib_get_error);
					goto setErr_dns;
				}
				//if(dns2.s_addr!=dns2_old.s_addr)
				if ( memcmp( dnsv62.s6_addr, dnsv62_old.s6_addr, 16) == 0 )
					dns_changed = 1;

				if ( !mib_set(MIB_ADSL_WAN_DNSV62, (void *)&dnsv62)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_dns;
				}
				#ifdef DNSV6_BIND_PVC_SUPPORT
				str = boaGetVar(wp, "enableDnsv6Bind", "");
				if(str[0]){//dns bind enabled
				    str=boaGetVar(wp,"dnsv6pvc2","");
				    printf("dnsv62 interface %d\n",atoi(str));
				    unsigned int dnsv62itfIndex=( unsigned int )atoi(str);
					if ( !mib_set(MIB_DNSV6_BIND_PVC2, (void *)&dnsv62itfIndex)) {
			  				strcpy(tmpBuf, TDNS_mib_set_error);
							goto setErr_dns;
					}
				}
				#endif
			}
			else {
				//jim
				if ( !mib_get(MIB_ADSL_WAN_DNSV62, (void *)&dnsv62_old)) {
	  				strcpy(tmpBuf, TDNS_mib_get_error);
					goto setErr_dns;
				}
				//if(dns2.s_addr!=dns2_old.s_addr)
				if ( memcmp( dnsv62.s6_addr, dnsv62_old.s6_addr, 16) == 0 )
					dns_changed = 1;

				if ( !mib_set(MIB_ADSL_WAN_DNSV62, (void *)&dnsv62)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_dns;
				}
			}

			str = boaGetVar(wp, "dnsv63", "");
			if ( str[0] ) {
				if ( !inet_pton(PF_INET6, str, &dnsv63) ) {
					strcpy(tmpBuf, Tinvalid_DNS_address);
					goto setErr_dns;
				}
				//jim
				if ( !mib_get(MIB_ADSL_WAN_DNSV63, (void *)&dnsv63_old)) {
	  				strcpy(tmpBuf, TDNS_mib_get_error);
					goto setErr_dns;
				}
				//if(dns3.s_addr!=dns3_old.s_addr)
				if ( memcmp( dnsv63.s6_addr, dnsv63_old.s6_addr, 16) == 0 )
					dns_changed = 1;
				if ( !mib_set(MIB_ADSL_WAN_DNSV63, (void *)&dnsv63)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_dns;
				}
				#ifdef DNSV6_BIND_PVC_SUPPORT
				str = boaGetVar(wp, "enableDnsv6Bind", "");
				if(str[0]){//dns bind enabled
				    str=boaGetVar(wp,"dnsv6pvc3","");
				    printf("dnsv63 interface %d\n",atoi(str));
				    unsigned int dnsv63itfIndex=( unsigned int )atoi(str);
					if ( !mib_set(MIB_DNSV6_BIND_PVC3, (void *)&dnsv63itfIndex)) {
			  				strcpy(tmpBuf, TDNS_mib_set_error);
							goto setErr_dns;
					}
				}
				#endif
			}
			else {
				//jim
				if ( !mib_get(MIB_ADSL_WAN_DNSV63, (void *)&dnsv63_old)) {
	  				strcpy(tmpBuf, TDNS_mib_get_error);
					goto setErr_dns;
				}
				//if(dns3.s_addr!=dns3_old.s_addr)
				if ( memcmp( dnsv63.s6_addr, dnsv63_old.s6_addr, 16) == 0 )
					dns_changed = 1;

				if ( !mib_set(MIB_ADSL_WAN_DNSV63, (void *)&dnsv63)) {
	  				strcpy(tmpBuf, TDNS_mib_set_error);
					goto setErr_dns;
				}
			}
		#ifdef CONFIG_IPV6
		#ifdef DNSV6_BIND_PVC_SUPPORT
		Dnsv6BindPvcRoute(ADDDNSROUTE);
		#endif
		#endif
		}
	}
#endif

setOk_dns:

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

#ifndef NO_ACTION
	pid = fork();
	if (pid)
		waitpid(pid, NULL, 0);
	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
		exit(1);
	}
#endif

#ifdef APPLY_CHANGE
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
	if (dns_changed) {
		//jim valid immediately....
		restart_dnsrelay();
	}
#endif
#endif
	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG(submitUrl);
 	return;

setErr_dns:
	ERR_MSG(tmpBuf);
}

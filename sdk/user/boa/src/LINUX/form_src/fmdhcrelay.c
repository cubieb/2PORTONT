
/*
 *      Web server handler routines for DNS stuffs
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <net/if.h>
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
#include "multilang.h"

void formDhcrelay(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100];
	char vChar;
	DHCP_TYPE_T curDhcp;
	int dhcprelayflag=0;

	str = boaGetVar(wp, "save", "");
	if (str[0]) {
		struct in_addr dhcps;
		struct in_addr olddhcps;

		if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) ) {
			strcpy(tmpBuf, strGetDhcpModeerror);
			goto setErr_dhcrelay;
		}
		curDhcp = (DHCP_TYPE_T) vChar;

		if(curDhcp == DHCP_LAN_RELAY)
		{
			str = boaGetVar(wp, "dhcps", "");
			if ( str[0] ) {
				if ( !inet_aton(str, &dhcps) ) {
					sprintf(tmpBuf, "%s (DHCPS address)", strInvalidValue);
					goto setErr_dhcrelay;
				}
				if(!mib_get(MIB_ADSL_WAN_DHCPS,(void*)&olddhcps)){
					sprintf(tmpBuf, " %s (DHCPS MIB).",Tset_mib_error);  
					goto setErr_dhcrelay;
				}
				if(olddhcps.s_addr != dhcps.s_addr){
					dhcprelayflag = 1;
					if ( !mib_set(MIB_ADSL_WAN_DHCPS, (void *)&dhcps)) {
						sprintf(tmpBuf, " %s (DHCPS MIB).",Tset_mib_error);  
						goto setErr_dhcrelay;
					}
				}
				goto setOk_dhcrelay;
			}
			else {
				/*
				if ( !mib_get(MIB_ADSL_WAN_DNS1, (void *)&dns1) ) {
					sprintf(tmpBuf, "%s DNS1 MIB error!",Tget_mib_error);
					goto setErr_dns;
				}
				*/
				goto setOk_dhcrelay;
			}
		}
	}


setOk_dhcrelay:
	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

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

//star: for take effect
	if(dhcprelayflag == 1)
	{
		restart_dhcp();
		submitUrl = boaGetVar(wp, "submit-url", "");
		OK_MSG(submitUrl);
	}else{
		submitUrl = boaGetVar(wp, "submit-url", "");
		if (submitUrl[0])
			boaRedirect(wp, submitUrl);
		else
			boaDone(wp, 200);
	}
	return;

setErr_dhcrelay:
	ERR_MSG(tmpBuf);
}

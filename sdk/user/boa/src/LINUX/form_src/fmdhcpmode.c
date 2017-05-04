
/*
 *      Web server handler routines for DHCP Mode stuffs
 *
 */


/*-- System inlcude files --*/
#include <string.h>
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"



///////////////////////////////////////////////////////////////////
void formDhcpMode(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	char tmpBuf[100];
	DHCP_TYPE_T dtmode,olddtmode;
	unsigned char vChar;
#ifndef NO_ACTION
	int pid;
#endif
	int dhcpmodeflag=0;

	// Commit
	str = boaGetVar(wp, "save", "");
	if (str[0]) {
		int time;

		str = boaGetVar(wp, "dhcpMode", "");

		if (str[0]) {
			if ( str[0] == '0' )
				dtmode = DHCP_LAN_NONE;
			else if ( str[0] == '1' )
				dtmode = DHCP_LAN_RELAY;
			else if ( str[0] == '2' )
				dtmode = DHCP_LAN_SERVER;
			else {
				strcpy(tmpBuf, "Invalid dhcp mode value!");
				goto setErr_reboot;
			}
			if ( mib_get(MIB_DHCP_MODE, (void *)&vChar) == 0) {
				strcpy(tmpBuf, "Set dhcp mode MIB error!");
				goto setErr_reboot;
			}
			olddtmode = vChar;
			if(olddtmode != dtmode)
			{
				dhcpmodeflag = 1;
				vChar = (unsigned char) dtmode;
				if ( mib_set(MIB_DHCP_MODE, (void *)&vChar) == 0) {
					strcpy(tmpBuf, "Set dhcp mode MIB error!");
					goto setErr_reboot;
				}
			}
		}


	}



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
	if(dhcpmodeflag == 1)
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
  	return;

setErr_reboot:
	ERR_MSG(tmpBuf);
}

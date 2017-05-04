/*-- System inlcude files --*/
#include <string.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <signal.h>
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#ifdef CONFIG_RTK_RG_INIT
#include "rtusr_rg_api.h"
#endif

#ifdef CONFIG_USER_MINIUPNPD

///////////////////////////////////////////////////////////////////
void formUpnp(request *wp, char *path, char *query)
{
	char	*str_enb, *str_extif, *submitUrl;
	char tmpBuf[100];
	char ifname[IFNAMSIZ];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char is_enabled, pre_enabled;
	unsigned int ext_if, pre_ext_if;

	str_enb = boaGetVar(wp, "daemon", "");
	str_extif = boaGetVar(wp, "ext_if", "");

	if(str_enb[0])
	{
		is_enabled = str_enb[0] - '0';

		if(str_extif[0])
			ext_if = (unsigned int)atoi(str_extif);
		else
			ext_if = DUMMY_IFINDEX;  // No interface selected.

		// Magician: UPnP daemon start or shutdown
		if(mib_get(MIB_UPNP_DAEMON, (void *)&pre_enabled) && mib_get(MIB_UPNP_EXT_ITF, (void *)&pre_ext_if))
		{
			if(is_enabled)  // UPnP is enabled
			{
				if(!mib_set(MIB_UPNP_DAEMON, (void *)&is_enabled))
				{
					strcpy(tmpBuf, "设定UPNP错误!"); //Set UPNP error!
					goto setErr_igmp;
				}

				if(pre_enabled && (ext_if != pre_ext_if))  // UPnP was set enabled previous time, and the bound interface was changed this time.)
				{
					ifGetName(pre_ext_if, ifname, sizeof(ifname));

					va_cmd("/bin/upnpctrl", 3, 1, "down", ifname, "br0");  // Shutdown UPnP first.

					if(!mib_set(MIB_UPNP_EXT_ITF, (void *)&ext_if))
					{
						printf("Set UPNP Binded WAN interface index error(1)\n");
						strcpy(tmpBuf, "设定UPNP绑定WAN接口索引错误!"); //Set UPNP Binded WAN interface index error!
						goto setErr_igmp;
					}

					ifGetName(ext_if, ifname, sizeof(ifname));

					va_cmd("/bin/upnpctrl", 3, 1, "up", ifname, "br0");  // restart UPnP daemon
				}
				else  if(!pre_enabled)  // UPnP is set disabled previous time.
				{
					if(!mib_set(MIB_UPNP_EXT_ITF, (void *)&ext_if))
					{
						printf("Set UPNP Binded WAN interface index error(1)\n");
						strcpy(tmpBuf, "设定UPNP绑定WAN接口索引错误!"); //Set UPNP Binded WAN interface index error!
						goto setErr_igmp;
					}

					ifGetName(ext_if, ifname, sizeof(ifname));

					va_cmd("/bin/upnpctrl", 3, 1, "up", ifname, "br0");
				}
			}
			else if(pre_enabled)  // UPnP is set from enabled to disabled.
			{
				ifGetName(pre_ext_if, ifname, sizeof(ifname));

				va_cmd("/bin/upnpctrl", 3, 1, "down", ifname, "br0");

				if(!mib_set(MIB_UPNP_DAEMON, (void *)&is_enabled))
				{
					strcpy(tmpBuf, "Set UPNP error!");
					goto setErr_igmp;
				}

				if(!mib_set(MIB_UPNP_EXT_ITF, (void *)&ext_if))
				{
					printf("Set UPNP Binded WAN interface index error(1)\n");
					strcpy(tmpBuf, "设定UPNP绑定WAN接口索引错误!"); //Set UPNP Binded WAN interface index error!
					goto setErr_igmp;
				}
#ifdef CONFIG_RTK_RG_INIT				
			FLUSH_RTK_RG_UPnP_Entry();
#endif				
			}
		}
		else
		{
			strcpy(tmpBuf, "UPNP错误: 读取mib table失败!"); //UPNP error: get mib table failed!
			goto setErr_igmp;
		}
	}

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

#ifndef NO_ACTION
	pid = fork();
	if (pid)
		waitpid(pid, NULL, 0);
	else if (pid == 0)
	{
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _CONFIG_SCRIPT_PROG);
#ifdef HOME_GATEWAY
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "gw", "bridge", NULL);
#else
		execl( tmpBuf, _CONFIG_SCRIPT_PROG, "ap", "bridge", NULL);
#endif
		exit(1);
	}
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");
	//OK_MSG(submitUrl);
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);

	return;
setErr_igmp:
	ERR_MSG(tmpBuf);
}
#endif

/*-- System inlcude files --*/
#include <string.h>
#include <signal.h>
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"

#ifdef CONFIG_USER_MINIUPNPD
void formUpnp(request * wp, char *path, char *query)
{
	char *str_enb, *str_extif, *submitUrl;
#ifdef CONFIG_TR_064
	char *tr064_en;
	unsigned char is_tr064_en, is_tr064_changed = 0;
#endif
	char tmpBuf[100];
	char ifname[IFNAMSIZ];
#ifndef NO_ACTION
	int pid;
#endif
	unsigned char is_enabled, pre_enabled;
	unsigned int ext_if, pre_ext_if;
#ifdef EMBED
	unsigned char if_num;
	int igmp_pid;
#endif

#ifdef CONFIG_TR_064
	tr064_en = boaGetVar(wp, "tr_064_sw", "");

	if(tr064_en[0])
	{
		is_tr064_en = tr064_en[0] - '0';

		if(is_tr064_en != TR064_STATUS)
		{
			if(!mib_set(MIB_TR064_ENABLED, (void *)&is_tr064_en))
			{
				sprintf(tmpBuf, "%s:%s", Tset_mib_error, MIB_TR064_ENABLED);
				goto setErr_igmp;
			}

			is_tr064_changed = 1;
		}
	}
#endif

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
					strcpy(tmpBuf, multilang(LANG_SET_UPNP_ERROR));
					goto setErr_igmp;
				}

				// UPnP was set enabled previous time, and the bound interface was changed this time. Or TR-064 status was changed.
				if((pre_enabled && (ext_if != pre_ext_if))
#ifdef CONFIG_TR_064
						|| is_tr064_changed
#endif
				) {
					ifGetName(pre_ext_if, ifname, sizeof(ifname));

					va_cmd("/bin/upnpctrl", 3, 1, "down", ifname, BRIF);  // Shutdown UPnP first.

					if(!mib_set(MIB_UPNP_EXT_ITF, (void *)&ext_if))
					{
						printf("Set UPNP Binded WAN interface index error(1)\n");
						strcpy(tmpBuf, multilang(LANG_SET_UPNP_BINDED_WAN_INTERFACE_INDEX_ERROR));
						goto setErr_igmp;
					}

					ifGetName(ext_if, ifname, sizeof(ifname));

					va_cmd("/bin/upnpctrl", 3, 1, "up", ifname, BRIF);  // restart UPnP daemon
				}
				else  if(!pre_enabled)  // UPnP is set disabled previous time.
				{
					if(!mib_set(MIB_UPNP_EXT_ITF, (void *)&ext_if))
					{
						printf("Set UPNP Binded WAN interface index error(1)\n");
						strcpy(tmpBuf, multilang(LANG_SET_UPNP_BINDED_WAN_INTERFACE_INDEX_ERROR));
						goto setErr_igmp;
					}

					ifGetName(ext_if, ifname, sizeof(ifname));

					va_cmd("/bin/upnpctrl", 3, 1, "up", ifname, BRIF);
				}
			}
			else if(pre_enabled)  // UPnP is set from enabled to disabled.
			{
				ifGetName(pre_ext_if, ifname, sizeof(ifname));

				va_cmd("/bin/upnpctrl", 3, 1, "down", ifname, BRIF);

				if(!mib_set(MIB_UPNP_DAEMON, (void *)&is_enabled))
				{
					strcpy(tmpBuf, multilang(LANG_SET_UPNP_ERROR));
					goto setErr_igmp;
				}

				if(!mib_set(MIB_UPNP_EXT_ITF, (void *)&ext_if))
				{
					printf("Set UPNP Binded WAN interface index error(1)\n");
					strcpy(tmpBuf, multilang(LANG_SET_UPNP_BINDED_WAN_INTERFACE_INDEX_ERROR));
					goto setErr_igmp;
				}
			}
		}
		else
		{
			printf(tmpBuf, "%s:%s", Tget_mib_error, multilang(MIB_UPNP_DAEMON));
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
	OK_MSG(submitUrl);
	return;

setErr_igmp:
	ERR_MSG(tmpBuf);
}
#endif

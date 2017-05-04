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

// Mason Yu. MLD Proxy
#ifdef CONFIG_IPV6
#ifdef CONFIG_USER_ECMH

///////////////////////////////////////////////////////////////////
void formMLDProxy(request * wp, char *path, char *query)
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

	str_enb = boaGetVar(wp, "daemon", "");
	str_extif = boaGetVar(wp, "ext_if", "");

	if(str_enb[0])
	{
		if (str_enb[0] == '0')
			is_enabled = 0;
		else
			is_enabled = 1;

		if(str_extif[0])
			ext_if = (unsigned int)atoi(str_extif);
		else
			ext_if = DUMMY_IFINDEX;  // No interface selected.

		if(!mib_set(MIB_MLD_PROXY_DAEMON, (void *)&is_enabled))
		{
			strcpy(tmpBuf, "Set UPNP error!");
			goto setErr_igmp;
		}

		if(!mib_set(MIB_MLD_PROXY_EXT_ITF, (void *)&ext_if))
		{
			printf("Set UPNP Binded WAN interface index error(1)\n");
			strcpy(tmpBuf, "UPNP Binded WAN interface index …Ë∂®¥ÌŒÛ!");  //Set UPNP Binded WAN interface index error!
			goto setErr_igmp;
		}
	}

	startMLDproxy();

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

// Mason Yu. MLD snooping for e8b
void formMLDSnooping(request * wp, char *path, char *query)
{
	char	*str, *submitUrl, *strSnoop;
	char tmpBuf[100], mode;
#ifndef NO_ACTION
	int pid;
#endif

#if defined(CONFIG_RTL_IGMP_SNOOPING)
	char origmode = 0;
	strSnoop = boaGetVar(wp, "snoop", "");
	if ( strSnoop[0] ) {
		// bitmap for virtual lan port function
		// Port Mapping: bit-0
		// QoS : bit-1
		// IGMP snooping: bit-2
		// MLD snooping: bit-3
		mib_get(MIB_MPMODE, (void *)&mode);
		origmode = mode;
		strSnoop = boaGetVar(wp, "snoop", "");
		if ( strSnoop[0] == '1' ) {
			mode |= 0x08;
			//if(origmode != mode)
			//	igmp_changed_flag = 1;
			// take effect immediately
#if defined(CONFIG_RTL_MLD_SNOOPING)
			__dev_setupMLDSnoop(1);
#endif
		}
		else {
			mode &= 0xf7;
			//if(origmode != mode)
			//	igmp_changed_flag = 1;
#if defined(CONFIG_RTL_MLD_SNOOPING)
			__dev_setupMLDSnoop(0);
#endif
		}
		mib_set(MIB_MPMODE, (void *)&mode);
	}
#endif
#if defined(CONFIG_MCAST_VLAN) && defined(CONFIG_RTK_RG_INIT)
RTK_RG_ACL_Flush_mVlan();
RTK_RG_ACL_Add_mVlan();
#endif	

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
	return;
}
#endif

#include "webform.h"
#include "../asp_page.h"
#include "multilang.h"

void formSamba(request * wp, char *path, char *query)
{
	char *str, *submitUrl;
	unsigned char samba_enable;
	char tmpBuf[64];

	str = boaGetVar(wp, "sambaCap", "");
	if (str[0]) {
		samba_enable = str[0] - '0';
		if (!mib_set(MIB_SAMBA_ENABLE, &samba_enable)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto formSamba_err;
		}
	}

#ifdef CONFIG_USER_NMBD
	str = boaGetVar(wp, "netBIOSName", "");
	if (str[0]) {
		if (!mib_set(MIB_SAMBA_NETBIOS_NAME, str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto formSamba_err;
		}
	}
#endif

	str = boaGetVar(wp, "serverString", "");
	if (str[0]) {
		if (!mib_set(MIB_SAMBA_SERVER_STRING, str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto formSamba_err;
		}
	}

formSamba_end:
#ifdef APPLY_CHANGE
	stopSamba();
	startSamba();
#endif

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG(submitUrl);

	return;

formSamba_err:
	ERR_MSG(tmpBuf);
}

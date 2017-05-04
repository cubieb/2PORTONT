#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>
#include <net/route.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <net/if.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"


int fmDMS_checkWrite(int eid, request * wp, int argc, char **argv)
{
	char *name;
	unsigned int enable;

   	if (boaArgs(argc, argv, "%s", &name) < 1) {
   		boaError(wp, 400, "Insufficient args\n");
		//printf( "%s: error, line=%d\n", __FUNCTION__, __LINE__ );
   		return -1;
   	}

	// Mason Yu. use table not chain
	mib_get(MIB_DMS_ENABLE, (void *)&enable);

	if(!strcmp(name, "fmDMS-enable-dis")) {
		if(enable==0) boaWrite(wp, "checked");
		return 0;
	}
	else if(!strcmp(name, "fmDMS-enable-en")) {
		if(enable) boaWrite(wp, "checked");
		return 0;
	}

	return 0;
}


void formDMSConf(request * wp, char *path, char *query)
{
	// Mason Yu. use table not chain
	char *strVal;
	int enable;
	char tmpBuf[100];

	// enable/disable DMS
	strVal = boaGetVar(wp, "enableDMS", "");

	if ( strVal[0] == '1' ) {//enable
		enable = 1;
	}
	else//disable
		enable = 0;

	mib_set(MIB_DMS_ENABLE, (void *)&enable);

	if(enable)
		startMiniDLNA();
	else
		stopMiniDLNA();

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	strVal = boaGetVar(wp, "submit-url", "");
	OK_MSG(strVal);
	return;

setErr_dms:
	ERR_MSG(tmpBuf);
	return;

}


/*
 *      Web server handler routines for Commit/reboot stuffs
 *
 */


/*-- System inlcude files --*/
/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
//alex_huang
#ifdef EMBED
#include <linux/config.h>
#include <config/autoconf.h>
#else
#include "../../../../include/linux/autoconf.h"
#include "../../../../config/autoconf.h"
#endif

void formReboot(request * wp, char *path, char *query)
{
	char tmpBuf[100];
	unsigned char vChar;
	int k;
#ifndef NO_ACTION
	int pid;
#endif
	boaHeader(wp);
	//--- Add timer countdown
        boaWrite(wp, "<head><style>\n" \
        "#cntdwn{ border-color: white;border-width: 0px;font-size: 12pt;color: red;text-align:left; font-weight:bold; font-family: Courier;}\n" \
        "</style><script language=javascript>\n" \
        "var h=60;\n" \
        "function stop() { clearTimeout(id); }\n"\
        "function start() { h--; if (h >= 0) { frm.time.value = h; frm.textname.value='System rebooting, Please wait ...'; id=setTimeout(\"start()\",1000); }\n" \
        "if (h == 0) { window.open(\"/status.asp\",target=\"view\"); }}\n" \
        "</script></head>");
        boaWrite(wp,
        "<body bgcolor=white  onLoad=\"start();\" onUnload=\"stop();\"><blockquote>" \
        "<form name=frm><B><font color=red><INPUT TYPE=text NAME=textname size=40 id=\"cntdwn\">\n" \
        "<INPUT TYPE=text NAME=time size=5 id=\"cntdwn\"></font></form></blockquote></body>" );
        //--- End of timer countdown
   	boaWrite(wp, "<body><blockquote><h4>\n");
   	/*
   	boaWrite(wp, "The System is Restarting ...</h4>\n");
   	boaWrite(wp, "The DSL Router has been configured and is rebooting.<br><br>\n");
   	boaWrite(wp, "Close the DSL Router Configuration window and wait"
   		" for 2 minutes before reopening your web browser."
   		" If necessary, reconfigure your PC's IP address to match"
   		" your new configuration.\n");
   	*/
   	boaWrite(wp, "%s</h4>\n", rebootWord0);
   	boaWrite(wp, "%s<br><br>\n", rebootWord1);
   	boaWrite(wp, "%s\n", rebootWord2);
   	boaWrite(wp, "</blockquote></body>");
   	boaFooter(wp);
	boaDone(wp, 200);

#ifdef EMBED
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
#endif
	cmd_reboot();

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

  	return;

setErr_reboot:
	ERR_MSG(tmpBuf);
}

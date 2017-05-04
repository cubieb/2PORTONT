#include <stdio.h>
#include "web_voip.h"

#define _PATH_CALL_LOGS_1	"/tmp/call_log_p0"
#define _PATH_CALL_LOGS_2	"/tmp/call_log_p2"


void asp_voip_LogSet(request * wp, char *path, char *query)
{
	char *strValue, *submitUrl;

	submitUrl = boaGetVar(wp, "call_log_refesh", "");   // hidden page

	if (submitUrl[0])
		boaRedirect(wp, submitUrl);


}


#ifdef CONFIG_APP_BOA
int asp_voip_LogGet(request * wp, int argc, char **argv)
#else
int asp_voip_LogGet(int ejid, request * wp, int argc, char **argv)
#endif
{

    FILE *fp;
	char  buf[150];
	int nBytesSent=0;

	boaWrite(wp,"Port 1<br><table border=\"1\" width=500 >");
	boaWrite(wp, "<tr><td bgColor=#aaddff>DateTime</td><td bgColor=#aaddff>From</td><td bgColor=#aaddff>To</td><td bgColor=#aaddff>Type</td><td bgColor=#aaddff>Status</td><td bgColor=#aaddff>Duration</td></tr>\n");

     fp = fopen(_PATH_CALL_LOGS_1, "r");
       if (fp){              
	        while(fgets(buf,150,fp)){
			nBytesSent += boaWrite(wp, "%s", buf);
	       	}
			fclose(fp);
        }
	boaWrite(wp,"</table>");

#if CONFIG_RTK_VOIP_CON_CH_NUM > 1
	

	boaWrite(wp,"<br>Port 2<br><table border=\"1\" width=500>");


	boaWrite(wp, "<tr><td bgColor=#aaddff>DateTime</td><td bgColor=#aaddff>From</td><td bgColor=#aaddff>To</td><td bgColor=#aaddff>Type</td><td bgColor=#aaddff>Status</td><td bgColor=#aaddff>Duration</td></tr>\n");

	fp = fopen(_PATH_CALL_LOGS_2, "r");
	if (fp){			  
		while(fgets(buf,150,fp)){
		nBytesSent += boaWrite(wp, "%s", buf);
		}
		fclose(fp);
	}
	boaWrite(wp,"</table>");

#endif

	return 0;
}


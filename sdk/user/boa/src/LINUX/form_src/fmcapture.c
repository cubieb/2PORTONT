/*
 *      Web server handler routines for Packet Capture stuffs
 *
 */

/*-- System inlcude files --*/
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "../defs.h"

#ifdef CONFIG_USER_TCPDUMP_WEB
// static request * gwp;

void kill_tcpdump(void);

void kill_tcpdump()
{
	FILE *fp;
	char temp_pid[10];
	int tcpdump_pid;
	fp = fopen("/var/run/tcpdump.pid","rt");
	if (fp)
   	{
		fgets(temp_pid,sizeof(temp_pid)-1,fp);
		fclose(fp);
		tcpdump_pid = atoi(temp_pid);
		if (tcpdump_pid > 0)
			kill(tcpdump_pid,SIGKILL);
	}
}

void formCapture(request * wp, char *path, char *query)
{
	char	*str;
    int status=0;
    char tmpbuf[MAX_URL_LEN];
    char ipaddr[16], ip_port[32];

	boaHeader(wp);
	boaWrite(wp, "<body><blockquote>");

	str = boaGetVar(wp, "dostart", "");

	if (strcmp(str,"yes") == 0) // start capture
	{
		str = boaGetVar(wp, "tcpdumpArgs", "");

		boaWrite(wp, "<br>Your capture arguments: ");
		boaWrite(wp, str);

		boaWrite(wp, "<p>\n");

		sprintf(tmpbuf,"/bin/tcpdump -g %s &",str);
		kill_tcpdump();
		system(tmpbuf);
		sleep(1);
		boaWrite(wp, "Packet dump has started!<br>\n");

		ipaddr[0]='\0'; ip_port[0]='\0';
		if (mib_get(MIB_ADSL_LAN_IP, (void *)tmpbuf) != 0)
		{
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpbuf)), 16);
			ipaddr[15] = '\0';
			sprintf(tmpbuf,"http://%s:%d/tcpdump.cap",ipaddr,19222);
		}

		boaWrite(wp, "Please click <a href='");
		boaWrite(wp, tmpbuf);
		boaWrite(wp, "'>here</a> once now to start the download of the capture file.<p>\n");
		boaWrite(wp, "To stop it please return to the Diagnostics / Packet Dump page and click 'Stop'.\n<br>");
		boaWrite(wp, "Do not stop the download by the browser's download manager.\n");
	}
	else
	{
		kill_tcpdump();
        boaWrite(wp, "<br>Capture file closed.");
	}

	boaWrite(wp, "</blockquote>");
	boaFooter(wp);
	boaDone(wp, 200);

  	return;

}
#endif // of CONFIG_USER_TCPDUMP_WEB


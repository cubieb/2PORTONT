/*
 *      Web server handler routines for OAM Loopback diagnostic stuffs
 *
 */


/*-- System inlcude files --*/
#include <string.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <stdint.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <sys/time.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "../defs.h"
#include "multilang.h"

#define MAX_WAIT 1000000 // microseconds

/* common routines */

///////////////////////////////////////////////////////////////////
void formOamLb(request * wp, char *path, char *query)
{
#ifdef EMBED
	char	*str, *submitUrl;
	char tmpBuf[100];
	int	skfd,i,j, entryNum;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	unsigned int uVc;
	int curidx, len, rpt_cnt, suc_cnt = 0, fail_cnt = 0, max_rpt = 0, min_rpt = MAX_WAIT, mean_rpt = 0;
	char *tmpStr;
	unsigned char *tmpValue;
	MIB_CE_ATM_VC_T Entry;
	struct timeval curTime, preTime;
	time_t sec_diff;
	suseconds_t usec_diff;

#ifndef NO_ACTION
	int pid;
#endif

	rpt_cnt = 5;

	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));

	str = boaGetVar(wp, "oam_flow", "");
	if(str[0])
		lbReq.Scope = str[0] - '0';	// Segment

	str = boaGetVar(wp, "oam_VC", "");
	if (str==NULL) {
		strcpy(tmpBuf, Tno_conn);
		goto setErr_oamlb;
	}

	//get PVC number
	sscanf(str,"%u",&uVc);
	if (!mib_chain_get(MIB_ATM_VC_TBL, uVc, (void *)&Entry))
	{
		strcpy(tmpBuf, multilang(LANG_PVC_CONFIGURATION_ERROR));
		goto setErr_oamlb;
	}
	lbReq.vpi = Entry.vpi;

	if(lbReq.Scope > 2 ) { // F4: 3 for Segment; 4 for End-to-End
		lbReq.vci = lbReq.Scope;
		lbReq.Scope -= 3; // scope: 0 for Segment; 1 for End-to-End
	}
	else
		lbReq.vci = Entry.vci;
	str = boaGetVar(wp, "oam_llid", "");
	// convert max of 32 hex decimal string into its 16 octets value
	len = strlen(str);
	curidx = 16;
	for (i=0; i<32; i+=2)
	{
		// Loopback Location ID
		curidx--;
		tmpValue = (unsigned char *)&lbReq.LocID[curidx];
		if (len > 0)
		{
			len -= 2;
			if (len < 0)
				len = 0;
			tmpStr = str + len;
			*tmpValue = strtoul(tmpStr, 0, 16);
			*tmpStr='\0';
		}
		else
			*tmpValue = 0;
	}

//	printf("Loopback Location ID:\n");
//	for (i=0; i<16; i++)
//		printf("%.02x", lbReq.LocID[i]);
//	printf("\n");
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket open error");
		exit(1);
	}

	for( i = 0; i < rpt_cnt; i++ )
	{
		mysio.number = 0;	// ATM interface number
		mysio.arg = (void *)&lbReq;
		// Start the loopback test

		if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
			strcpy(tmpBuf, multilang(LANG_IOCTL_ATM_OAM_LB_START_FAILED));
			close(skfd);
			goto setErr_oamlb;
		}

		// Query the loopback status
		lbState.vpi = Entry.vpi;
		lbState.vci = lbReq.vci;

		lbState.Tag = lbReq.Tag;

		mysio.arg = (void *)&lbState;

		gettimeofday(&preTime, NULL);

		while (1)
		{
			gettimeofday(&curTime, NULL);

			sec_diff = curTime.tv_sec - preTime.tv_sec;
			usec_diff = curTime.tv_usec - preTime.tv_usec + sec_diff * 1000000;

			if(usec_diff >= MAX_WAIT)
			{
				fail_cnt++;
				break;	// break for timeout
			}

			if(ioctl(skfd, ATM_OAM_LB_STATUS, &mysio) < 0)
			{
				strcpy(tmpBuf, multilang(LANG_IOCTL_ATM_OAM_LB_STATUS_FAILED));
				mysio.arg = (void *)&lbReq;
				ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
				close(skfd);
				goto setErr_oamlb;
			}

			if(lbState.count[0] > 0)
			{
				gettimeofday(&curTime, NULL);

				sec_diff = curTime.tv_sec - preTime.tv_sec;
				usec_diff = curTime.tv_usec - preTime.tv_usec + sec_diff * 1000000;
				suc_cnt++;

				if(usec_diff > max_rpt )
					max_rpt = usec_diff;

				if(usec_diff < min_rpt )
					min_rpt = usec_diff;

				mean_rpt += usec_diff;
				break;	// break for loopback success
			}
		}

		mysio.arg = (void *)&lbReq;
		// Stop the loopback test
		if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio) < 0)
		{
			strcpy(tmpBuf, multilang(LANG_IOCTL_ATM_OAM_LB_STOP_FAILED));
			close(skfd);
			goto setErr_oamlb;
		}
	}

	close(skfd);

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

	if(suc_cnt == 0)
		mean_rpt = min_rpt = max_rpt = 0;
	else
		mean_rpt /= suc_cnt;

	boaHeader(wp);
	boaWrite(wp, "<body><blockquote><h3>ATM Loopback Diagnostic Results<br></h4>\n");
	boaWrite(wp, "<table border=0 width=600>");
	boaWrite(wp, "<tr>");
	boaWrite(wp, "<td width=300>Repetitions Count:</td><td align=left>%d</td>", rpt_cnt);
	boaWrite(wp, "</tr>");
	boaWrite(wp, "<tr>");
	boaWrite(wp, "<td width=300>Repetitions Timeout:</td><td align=left>%d ms</td>", MAX_WAIT/1000);
	boaWrite(wp, "</tr>");
	boaWrite(wp, "<tr>");
	boaWrite(wp, "<td width=300 style='color: green'>Success Response Count:</td><td align=left>%d</td>", suc_cnt);
	boaWrite(wp, "</tr>");
	boaWrite(wp, "<tr>");
	boaWrite(wp, "<td width=300 style='color: red'>Failure Response Count:</td><td align=left>%d</td>", fail_cnt);
	boaWrite(wp, "</tr>");
	boaWrite(wp, "<tr>");

	if(suc_cnt > 0 && mean_rpt < 1000 )
		boaWrite(wp, "<td width=300>Average Response Time:</td><td align=left><1 ms</td>");
	else
		boaWrite(wp, "<td width=300>Average Response Time:</td><td align=left>%d ms</td>", mean_rpt/1000);

	boaWrite(wp, "</tr>");
	boaWrite(wp, "<tr>");

	if(suc_cnt > 0 && min_rpt < 1000 )
		boaWrite(wp, "<td width=300>Minimum Response Time:</td><td align=left><1 ms</td>");
	else
		boaWrite(wp, "<td width=300>Minimum Response Time:</td><td align=left>%d ms</td>", min_rpt/1000);

	boaWrite(wp, "</tr>");
	boaWrite(wp, "<tr>");

	if(suc_cnt > 0 && max_rpt < 1000 )
		boaWrite(wp, "<td width=300>Maximum Response Time:</td><td align=left><1 ms</td>");
	else
		boaWrite(wp, "<td width=300>Maximum Response Time:</td><td align=left>%d ms</td>", max_rpt/1000);

	boaWrite(wp, "</tr>");
	boaWrite(wp, "</table><br>");

	submitUrl = boaGetVar(wp, "submit-url", "");
	boaWrite(wp, "<form><input type=button value=\"%s\" OnClick=window.location.replace(\"%s\")></form></blockquote></body>", Tback, submitUrl);
	boaFooter(wp);
	boaDone(wp, 200);

	return;

setErr_oamlb:
	ERR_MSG(tmpBuf);
#endif
}

int oamSelectList(int eid, request * wp, int argc, char **argv)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	int first_write=0;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	boaWrite(wp, "<BR>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;");
	for (i=0;i<entryNum;i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0 || MEDIA_INDEX(Entry.ifIndex) != MEDIA_ATM)
			continue;

		if (first_write==0) {
			first_write=1;
		    boaWrite(wp, "<input type=\"radio\" value=\"%d\" name=\"oam_VC\" checked>%d/%d\n",
				i, Entry.vpi, Entry.vci);
		} else
		    boaWrite(wp, "<input type=\"radio\" value=\"%d\" name=\"oam_VC\">%d/%d\n",
				i, Entry.vpi, Entry.vci);
	}
	return 0;
}


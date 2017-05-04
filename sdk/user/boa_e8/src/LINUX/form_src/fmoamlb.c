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

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "../defs.h"

#define MAXWAIT		5

static int finished = 0;

/* common routines */

///////////////////////////////////////////////////////////////////
void formOamLb(request *wp, char *path, char *query)
{
#ifdef EMBED
	char *str, *submitUrl;
	char tmpBuf[100];
	int	skfd,i,j, entryNum;
	struct atmif_sioc mysio;
	ATMOAMLBReq lbReq;
	ATMOAMLBState lbState;
	unsigned int uVc;
	int curidx, len;
	char *tmpStr;
	unsigned char *tmpValue;
	MIB_CE_ATM_VC_T Entry;
	time_t  curTime, preTime = 0;
	
#ifndef NO_ACTION
	int pid;
#endif

	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		exit(1);
	}
	
	memset(&lbReq, 0, sizeof(ATMOAMLBReq));
	memset(&lbState, 0, sizeof(ATMOAMLBState));
	str = boaGetVar(wp, "oam_flow", "");
	if (str[0] == '0')
		lbReq.Scope = 0;	// Segment
	else if (str[0] == '1')
		lbReq.Scope = 1;	// End-to-End
	
	str = boaGetVar(wp, "oam_VC", "");
	if (str==NULL) {
		strcpy(tmpBuf, Tno_conn);
		goto setErr_oamlb;
	}
	
	//get PVC number
	sscanf(str,"%u",&uVc);
	if (!mib_chain_get(MIB_ATM_VC_TBL, uVc, (void *)&Entry))
	{
		strcpy(tmpBuf, "PVC×éÌ¬´íÎó!"); //PVC configuration error!
		goto setErr_oamlb;
	}
	lbReq.vpi = Entry.vpi;
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
	
	mysio.number = 0;	// ATM interface number
	mysio.arg = (void *)&lbReq;
	// Start the loopback test
	if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0) {
		strcpy(tmpBuf, "ioctl: ATM_OAM_LB_START Ê§°Ü !"); //ioctl: ATM_OAM_LB_START failed !
		close(skfd);
		goto setErr_oamlb;
	}
	
	finished = 0;
	time(&preTime);
	
	// Query the loopback status
	mysio.arg = (void *)&lbState;
	lbState.vpi = Entry.vpi;
	lbState.vci = Entry.vci;
	lbState.Tag = lbReq.Tag;
	
	while (1)
	{
		time(&curTime);
		if (curTime - preTime >= MAXWAIT)
		{
			//printf("OAMLB timeout!\n");
			finished = 1;
			break;	// break for timeout
		}
		
		if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0) {
			strcpy(tmpBuf, "ioctl: ATM_OAM_LB_STATUS Ê§°Ü !"); //ioctl: ATM_OAM_LB_STATUS failed !
			mysio.arg = (void *)&lbReq;
			ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
			close(skfd);
			goto setErr_oamlb;
		}
		
		if (lbState.count[0] > 0)
		{
			break;	// break for loopback success
		}
	}
		
	mysio.arg = (void *)&lbReq;
	// Stop the loopback test
	if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0) {
		strcpy(tmpBuf, "ioctl: ATM_OAM_LB_STOP Ê§°Ü !"); //ioctl: ATM_OAM_LB_STOP failed !
		close(skfd);
		goto setErr_oamlb;
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

	boaHeader(wp);
	if (!finished)
	{
		boaWrite(wp, "<body><blockquote><h4><font color='green'>\"%s\"<br><br>", Trecv_cell_suc);
		printf("\n--- Loopback cell received successfully ---\n");
	}
	else
	{
		boaWrite(wp, "<body><blockquote><h4><font color='red'>%s<br><br>", Trecv_cell_fail);
		printf("\n--- Loopback failed ---\n");
	}
	boaWrite(wp, "</h4>\n");
	printf("\n");
	submitUrl = boaGetVar(wp, "submit-url", "");
	boaWrite(wp, "<form><input type=button value=\"%s\" OnClick=window.location.replace(\"%s\")></form></blockquote></body>", Tback, submitUrl);
	boaFooter(wp);
	boaDone(wp, 200);
	
  	return;

setErr_oamlb:
	ERR_MSG(tmpBuf);
#endif
}

int oamSelectList(int eid, request *wp, int argc, char **argv)
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



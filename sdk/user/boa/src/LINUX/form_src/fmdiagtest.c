/*
 *      Web server handler routines for diagnostic tests
 *
 */

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include <string.h>
#include <linux/if_bridge.h>
#include <stdio.h>
#include <sys/signal.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <net/route.h>
#include	<netdb.h>
#include "../defs.h"
#include "multilang.h"

#ifdef DIAGNOSTIC_TEST
#define PINGCOUNT	3
#define DEFDATALEN	56
#define PINGINTERVAL	1	/* second */
#define MAXWAIT		5
static const char R_PASS[] = " color='green'><b>PASS";
static const char R_FAIL[] = " color='red'><b>FAIL";
static int cmode = CHANNEL_MODE_BRIDGE;
int eth=0, adslflag=0, pppserver=0, auth=0, ipup=0, lb5s=0, lb5e=0, lb4s=0, lb4e=0, dgw=0, pdns=0;

static void processDiagTest(request * wp)
{
	int inf=-1, i, pppif;
	MIB_CE_ATM_VC_T Entry;
	unsigned int entryNum;
	FILE *fp;
	char buff[10], ifname[6];
	MIB_CE_ATM_VC_Tp pEntry;

#ifdef CONFIG_DEV_xDSL
	Modem_LinkSpeed vLs;

	if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		adslflag = 0;
	else
		adslflag = 1;
#else
	adslflag = 1;
#endif

	if (fp = fopen("/tmp/diaginf", "r")) {
		fscanf(fp, "%d", &inf);
		fclose(fp);
	}
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);

	for (i=0;i<entryNum;i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			boaError(wp, 400, "Get chain record error!\n");
			exit(-1);
		}
		if (Entry.enable == 0)
			continue;
		if (inf == -1)
			inf = Entry.ifIndex;
		if (Entry.ifIndex == inf) {
			if (Entry.cmode != CHANNEL_MODE_BRIDGE) {
				struct in_addr inAddr;
				int flags;
				cmode = CHANNEL_MODE_IPOE;
				if (PPP_INDEX(Entry.ifIndex) != DUMMY_PPP_INDEX) {	// PPP Interface
					int pppflag;
					cmode = CHANNEL_MODE_PPPOE;
					sprintf(ifname, "ppp%d", PPP_INDEX(Entry.ifIndex));
					if (fp = fopen("/tmp/ppp_diag_log", "r")) {
						while (fgets(buff, sizeof(buff), fp) != NULL) {
							sscanf(buff, "%d:%d", &pppif, &pppflag);
							if (pppif == PPP_INDEX(Entry.ifIndex))
									break;
						}
						fclose(fp);
					}
					switch(pppflag)
					{
						case 1:
							pppserver = 1;
							auth = ipup = 0;
							break;
						case 2:
							pppserver = auth = 1;
							ipup = 0;
							break;
						case 3:
							pppserver = auth = ipup = 1;
							break;
						case 0:
						default:
							pppserver = auth = ipup = 0;
							break;
					}
				}
				else
					sprintf(ifname, "vc%u", VC_INDEX(Entry.ifIndex));
			}
			else
				cmode = CHANNEL_MODE_BRIDGE;
			break;
		}
	}
	pEntry = &Entry;
	if (adslflag) {
#ifdef CONFIG_DEV_xDSL
		lb5s = testOAMLookback(pEntry, 0, 5);
		lb5e = testOAMLookback(pEntry, 1, 5);
		lb4s = testOAMLookback(pEntry, 0, 4);
		lb4e = testOAMLookback(pEntry, 1, 4);
#endif
		if (cmode > CHANNEL_MODE_BRIDGE) {
			char pingaddr[16];
			memset(pingaddr, 0x00, 16);
			if (defaultGWAddr(pingaddr))
				dgw = 0;
			else
//				dgw = testPing(pingaddr);
				dgw = utilping(pingaddr);
			memset(pingaddr, 0x00, 16);
			if (pdnsAddr(pingaddr))
				pdns = 0;
			else
//				pdns = testPing(pingaddr);
				pdns = utilping(pingaddr);
		}
	}
}

int startflag = 0;
void formDiagTest(request * wp, char *path, char *query)
{
	char *strSubmit, *submitUrl;
	unsigned int wan_itf;
	FILE *fp;

	strSubmit = boaGetVar(wp, "wan_if", "");
	if (strSubmit[0]) {
		wan_itf = (unsigned int)atoi(strSubmit);
		fp = fopen("/tmp/diaginf", "w");
		fprintf(fp, "%d", wan_itf);
		fclose(fp);
	}

//	strSubmit = boaGetVar(wp, "start", "");
//	if (strSubmit[0]) {
		// start diagnostic test here
		startflag = 1;
//	}

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;
}

// Test Ethernet LAN connection table
int lanTest(int eid, request * wp, int argc, char **argv)
{
	int fd, nBytesSent=0, flags;
	int mii_reg, i;
	struct ifreq ifrq;
	ushort *data = (ushort *)(&ifrq.ifr_data);
	unsigned int *data32 = (unsigned int *)(&ifrq.ifr_data);
	unsigned phy_id=1;
	unsigned char new_ioctl_nums = 0;
	ushort mii_val[32];

	if (startflag) {
		strncpy(ifrq.ifr_name, ELANIF, sizeof(ifrq.ifr_name));
		ifrq.ifr_name[ sizeof(ifrq.ifr_name)-1] = 0;
		eth = getLinkStatus(&ifrq);
		nBytesSent += boaWrite(wp, "<tr><td width=100%% colspan=\"2\" bgcolor=\"#808080\">"
		"<font color=\"#FFFFFF\" size=2><b>%s</b></font></td></tr>", Tlan_conn_chk);
		nBytesSent += boaWrite(wp, "<tr>"
		"<td width=\"80%%\" bgcolor=\"#DDDDDD\"><font size=2><b>%s</b></td>\n"
		"<td width=\"20%%\" bgcolor=\"#EEEEEE\"><font size=2%s</td></tr>\n", Ttest_eth_conn, (eth)?R_PASS: R_FAIL);
	}
	return nBytesSent;
}

// Test ADSL service provider connection table
int adslTest(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	adslflag = pppserver = auth = ipup = lb5s = lb5e = lb4s = lb4e = dgw = pdns=0;
	if (startflag) {
		processDiagTest(wp);
		nBytesSent += boaWrite(wp, "<tr><td width=100%% colspan=\"2\" bgcolor=\"#808080\">"
		"<font color=\"#FFFFFF\" size=2><b>%s</b></font></td></tr>", Tadsl_conn_chk);
		nBytesSent += boaWrite(wp, "<tr>"
		"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</td>"
		"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n", Ttest_adsl_syn, (adslflag)?R_PASS: R_FAIL);
		nBytesSent += boaWrite(wp, "<tr>"
		"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
		"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n", Ttest_oam_f5_seg, (lb5s)?R_PASS: R_FAIL);
		nBytesSent += boaWrite(wp, "<tr>"
		"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
		"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n", Ttest_oam_f5_end, (lb5e)?R_PASS: R_FAIL);
		nBytesSent += boaWrite(wp, "<tr>"
		"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
		"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n", Ttest_oam_f4_seg, (lb4s)?R_PASS: R_FAIL);
		nBytesSent += boaWrite(wp, "<tr>"
		"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
		"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n", Ttest_oam_f4_end, (lb4e)?R_PASS: R_FAIL);
	}
	return nBytesSent;
}

// Test Internet service provider connection table
int internetTest(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	unsigned int entryNum;

	if (startflag) {
		if (cmode > CHANNEL_MODE_BRIDGE) {
			nBytesSent += boaWrite(wp, "<tr><td width=100%% colspan=\"2\" bgcolor=\"#808080\">"
			"<font color=\"#FFFFFF\" size=2><b>%s</b></font></td></tr>", Tint_conn_chk);
			if (cmode == CHANNEL_MODE_PPPOE || cmode == CHANNEL_MODE_PPPOA) {
				nBytesSent += boaWrite(wp, "<tr>"
				"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
				"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n", Ttest_ppps_conn, (pppserver)?R_PASS: R_FAIL);
				nBytesSent += boaWrite(wp, "<tr>"
				"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</td>"
				"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n", Ttest_auth, (auth==1)?R_PASS: R_FAIL);
				nBytesSent += boaWrite(wp, "<tr>"
				"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
				"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n", Ttest_assigned_ip, (ipup)?R_PASS: R_FAIL);
			}
			nBytesSent += boaWrite(wp, "<tr>"
			"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
			"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n", Tping_def_gw, (dgw)?R_PASS: R_FAIL);
			nBytesSent += boaWrite(wp, "<tr>"
			"<td width=80%% bgcolor=#DDDDDD><font size=2><b>%s</b></td>"
			"<td width=20%% bgcolor=#EEEEEE><font size=2%s</td></tr>\n", Tping_pri_dnss, (pdns)?R_PASS: R_FAIL);
		}
		startflag = 0;
	}
	return nBytesSent;
}
#endif

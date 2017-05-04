/*
 *      Web server handler routines for System status
 *
 */

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"
#include "debug.h"
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_bridge.h>
#include "adsl_drv.h"
#include <stdio.h>
#include <fcntl.h>
#include "signal.h"
#include "../defs.h"
#include "multilang.h"

static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";
#ifdef EMBED
#ifdef CONFIG_USER_PPPOMODEM
const char PPPOM_CONF[] = "/var/ppp/pppom.conf";
#endif //CONFIG_USER_PPPOMODEM
#endif

#if defined(CONFIG_RTL_8676HWNAT)
void formLANPortStatus(request * wp, char *path, char *query)
{
	char *submitUrl, *strSubmitR;
	strSubmitR = boaGetVar(wp, "refresh", "");
	// Refresh
	if (strSubmitR[0]) {
		goto setOk_filter;
	}

setOk_filter:
	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;
}
void showLANPortStatus(int eid, request * wp, int argc, char **argv)
{
	int i;
	unsigned char strbuf[256];
	for(i=0; i<SW_LAN_PORT_NUM; i++){
		getLANPortStatus(i, strbuf);
		boaWrite(wp,"<tr bgcolor=\"#EEEEEE\">\n"
	    "<td width=40%%><font size=2><b>LAN%d</b></td>\n"
	    "<td width=60%%><font size=2>%s</td>\n</tr>", i+1, strbuf);
	}
}
#endif

void formStatus(request * wp, char *path, char *query)
{
	char *submitUrl, *strSubmitR, tmpBuf[100];
#ifdef CONFIG_PPP
	char *strSubmitP;
	struct data_to_pass_st msg;
	char buff[256];
	unsigned int i, flag, inf;
	FILE *fp;
#endif
#ifdef CONFIG_USER_PPPOMODEM
	unsigned int cflag[MAX_PPP_NUM+MAX_MODEM_PPPNUM]={0};
#else
	unsigned int cflag[MAX_PPP_NUM]={0};
#endif //CONFIG_USER_PPPOMODEM

#ifdef CONFIG_PPP
	// Added by Jenny, for PPP connecting/disconnecting
#ifdef CONFIG_USER_PPPOMODEM
	for (i=0; i<(MAX_PPP_NUM+MAX_MODEM_PPPNUM); i++)
#else
	for (i=0; i<MAX_PPP_NUM; i++)
#endif //CONFIG_USER_PPPOMODEM
	{
		char tmp[15], tp[10];

		sprintf(tp, "ppp%d", i);
		if (find_ppp_from_conf(tp)) {
			if (fp=fopen("/tmp/ppp_up_log", "r")) {
				while ( fgets(buff, sizeof(buff), fp) != NULL ) {
					if(sscanf(buff, "%d %d", &inf, &flag) != 2)
						break;
					else {
						if (inf == i)
							cflag[i] = flag;
					}
				}
				fclose(fp);
			}
			sprintf(tmp, "submitppp%d", i);
			strSubmitP = boaGetVar(wp, tmp, "");
			if ( strSubmitP[0] ) {
				if ((strcmp(strSubmitP, "Connect") == 0)) {
					if (cflag[i]) {
						snprintf(msg.data, BUF_SIZE, "spppctl up %u", i);
						usleep(3000000);
						TRACE(STA_SCRIPT, "%s\n", msg.data);
						write_to_pppd(&msg);
							//add by ramen to resolve for clicking "connect" button twice.
					}
				} else if (strcmp(strSubmitP, "Disconnect") == 0) {

					snprintf(msg.data, BUF_SIZE, "spppctl down %u", i);
					TRACE(STA_SCRIPT, "%s\n", msg.data);
					write_to_pppd(&msg);
						//add by ramen to resolve for clicking "disconnect" button twice.
				} else {
					strcpy(tmpBuf, multilang(LANG_INVALID_PPP_ACTION));
					goto setErr_filter;
				}
			}
		}
	}
#endif

	strSubmitR = boaGetVar(wp, "refresh", "");
	// Refresh
	if (strSubmitR[0]) {
		goto setOk_filter;
	}

setOk_filter:
	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

setErr_filter:
	ERR_MSG(tmpBuf);
}

void formDate(request * wp, char *path, char *query)
{
	char *strVal, *submitUrl;
	time_t tm;
	struct tm tm_time;

	time(&tm);
	memcpy(&tm_time, localtime(&tm), sizeof(tm_time));

	strVal = boaGetVar(wp, "sys_month", "");
	if (strVal[0])
		tm_time.tm_mon = atoi(strVal);

	strVal = boaGetVar(wp, "sys_day", "");
	if (strVal[0])
		tm_time.tm_mday = atoi(strVal);

	strVal = boaGetVar(wp, "sys_year", "");
	if (strVal[0])
		tm_time.tm_year = atoi(strVal) - 1900;

	strVal = boaGetVar(wp, "sys_hour", "");
	if (strVal[0])
		tm_time.tm_hour = atoi(strVal);

	strVal = boaGetVar(wp, "sys_minute", "");
	if (strVal[0])
		tm_time.tm_min = atoi(strVal);

	strVal = boaGetVar(wp, "sys_second", "");
	if (strVal[0])
		tm_time.tm_sec = atoi(strVal);

	tm = mktime(&tm_time);

	if (stime(&tm) < 0) {
		perror("cannot set date");
	}

	OK_MSG1(multilang(LANG_SYSTEM_DATE_HAS_BEEN_MODIFIED_SUCCESSFULLY_PLEASE_REFLESH_YOUR_STATUS_PAGE), NULL);
	return;
}

#ifdef CONFIG_USER_PPPOMODEM
#undef FILE_LOCK
int wan3GTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	nBytesSent += boaWrite(wp, "<br>\n");
	nBytesSent += boaWrite(wp, "<table width=600 border=0>\n");
	nBytesSent += boaWrite(wp, "  <tr>\n");
	nBytesSent += boaWrite(wp, "    <td width=100%% colspan=5 bgcolor=\"#008000\">\n"
	                              "      <font color=\"#FFFFFF\" size=2><b>3G %s</b></font>\n"
	                              "    </td>\n", multilang(LANG_CONFIGURATION));
	nBytesSent += boaWrite(wp, "  </tr>\n");

	nBytesSent += boaWrite(wp, "  <tr bgcolor=\"#808080\">\n"
                              	      "    <td width=\"15%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"15%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"25%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"25%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"20%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "  </tr>\n",
				multilang(LANG_INTERFACE), multilang(LANG_PROTOCOL), multilang(LANG_IP_ADDRESS),
				multilang(LANG_GATEWAY), multilang(LANG_STATUS));

	{
		MIB_WAN_3G_T entry,*p;
		p=&entry;
		if( mib_chain_get( MIB_WAN_3G_TBL, 0, (void*)p) && p->enable )
		{
			int mppp_idx;
			char mppp_ifname[IFNAMSIZ];
			char mppp_protocol[10];
			char mppp_ipaddr[20];
			char mppp_remoteip[20];
			char *mppp_status;
			char mppp_uptime[20]="";
			char mppp_totaluptime[20]="";
			struct in_addr inAddr;
			int flags;
			char *temp;
			int pppConnectStatus, pppDod;
			FILE *fp;
			#ifdef FILE_LOCK
			struct flock flpom;
			int fdpom;
			#endif

			mppp_idx=MODEM_PPPIDX_FROM;
			sprintf( mppp_ifname, "ppp%d", mppp_idx );
			strcpy( mppp_protocol, "PPP" );

			if (getInAddr( mppp_ifname, IP_ADDR, (void *)&inAddr) == 1)
			{
				sprintf( mppp_ipaddr, "%s",   inet_ntoa(inAddr) );
				if (strcmp(mppp_ipaddr, "64.64.64.64") == 0)
					strcpy(mppp_ipaddr, "");
			}else
				strcpy( mppp_ipaddr, "" );

			if (getInAddr( mppp_ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
			{
				struct in_addr gw_in;
				char gw_tmp[20];
				gw_in.s_addr=htonl(0x0a404040+mppp_idx);
				sprintf( gw_tmp, "%s",    inet_ntoa(gw_in) );

				sprintf( mppp_remoteip, "%s",   inet_ntoa(inAddr) );
				if( strcmp(mppp_remoteip, gw_tmp)==0 )
					strcpy(mppp_remoteip, "");
				else if (strcmp(mppp_remoteip, "64.64.64.64") == 0)
					strcpy(mppp_remoteip, "");
			}else
				strcpy( mppp_remoteip, "" );


			if (getInFlags( mppp_ifname, &flags) == 1)
			{
				if (flags & IFF_UP) {
					if (getInAddr(mppp_ifname, IP_ADDR, (void *)&inAddr) == 1) {
						temp = inet_ntoa(inAddr);
						if (strcmp(temp, "64.64.64.64"))
							mppp_status = (char *)IF_UP;
						else
							mppp_status = (char *)IF_DOWN;
					}else
						mppp_status = (char *)IF_DOWN;
				}else
					mppp_status = (char *)IF_DOWN;
			}else
				mppp_status = (char *)IF_NA;

			if (strcmp(mppp_status, (char *)IF_UP) == 0)
				pppConnectStatus = 1;
			else{
				pppConnectStatus = 0;
				mppp_ipaddr[0] = '\0';
				mppp_remoteip[0] = '\0';
			}

			if(p->backup || p->ctype==CONNECT_ON_DEMAND && p->idletime!=0) //added by paula, 3g backup PPP
				pppDod=1;
			else
				pppDod=0;

			#ifdef FILE_LOCK
			//file locking
			fdpom = open(PPPOM_CONF, O_RDWR);
			if (fdpom != -1) {
				flpom.l_type = F_WRLCK;
				flpom.l_whence = SEEK_SET;
				flpom.l_start = 0;
				flpom.l_len = 0;
				flpom.l_pid = getpid();
				if (fcntl(fdpom, F_SETLKW, &flpom) == -1)
					printf("pppom write lock failed\n");
				//printf( "wan3GTable: pppom write lock successfully\n" );
			}
			#endif
			if (!(fp=fopen(PPPOM_CONF, "r")))
				printf("%s not exists.\n", PPPOM_CONF);
			else {
				char	buff[256], tmp1[20], tmp2[20], tmp3[20], tmp4[20];

				fgets(buff, sizeof(buff), fp);
				if( fgets(buff, sizeof(buff), fp) != NULL )
				{
					if (sscanf(buff, "%s%*s%s%s", tmp1, tmp2, tmp3) != 3)
					{
						printf("Unsuported pppoa configuration format\n");
					}else {
						if( !strcmp(mppp_ifname,tmp1) )
						{
							strcpy(mppp_uptime, tmp2);
							strcpy(mppp_totaluptime, tmp3);
						}
					}
				}
				fclose(fp);
			}
			#ifdef FILE_LOCK
			//file unlocking
			if (fdpom != -1) {
				flpom.l_type = F_UNLCK;
				if (fcntl(fdpom, F_SETLK, &flpom) == -1)
					printf("pppom write unlock failed\n");
				close(fdpom);
				//printf( "wan3GTable: pppom write unlock successfully\n" );
			}
			#endif

			nBytesSent += boaWrite(wp, "  <tr bgcolor=\"#EEEEEE\">\n"
			                              "    <td align=center width=\"15%%\"><font size=2>%s</td>\n"
			                              "    <td align=center width=\"15%%\"><font size=2>%s</td>\n"
			                              "    <td align=center width=\"25%%\"><font size=2>%s</td>\n"
			                              "    <td align=center width=\"25%%\"><font size=2>%s</td>\n",
			                              mppp_ifname, mppp_protocol, mppp_ipaddr, mppp_remoteip);

			nBytesSent += boaWrite(wp, "    <td align=center width=\"20%%\"><font size=2>%s %s / %s ",
								mppp_status, mppp_uptime, mppp_totaluptime );
			if(!pppDod)
			{
				nBytesSent += boaWrite(wp, "<input type=\"submit\" value=\"%s\" name=\"submit%s\">\n",
							(pppConnectStatus==1) ?
							multilang(LANG_DISCONNECT) : multilang(LANG_CONNECT), mppp_ifname);
			}
			nBytesSent += boaWrite(wp, "	</td>\n");
			nBytesSent += boaWrite(wp, "  </tr>\n");
		}
	}

	nBytesSent += boaWrite(wp, "</table>");
	return nBytesSent;
}
#else
int wan3GTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	return nBytesSent;
}
#endif //CONFIG_USER_PPPOMODEM

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
int wanPPTPTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	MIB_PPTP_T Entry;
	unsigned int entryNum, i;

	nBytesSent += boaWrite(wp, "<br>\n");
	nBytesSent += boaWrite(wp, "<table width=600 border=0>\n");
	nBytesSent += boaWrite(wp, "  <tr>\n");
	nBytesSent += boaWrite(wp, "    <td width=100%% colspan=5 bgcolor=\"#008000\">\n"
	                              "      <font color=\"#FFFFFF\" size=2><b>PPTP %s</b></font>\n"
	                              "    </td>\n", multilang(LANG_CONFIGURATION));
	nBytesSent += boaWrite(wp, "  </tr>\n");

	nBytesSent += boaWrite(wp, "  <tr bgcolor=\"#808080\">\n"
                              	      "    <td width=\"15%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"15%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"25%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"25%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"20%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "  </tr>\n",
				multilang(LANG_INTERFACE), multilang(LANG_PROTOCOL), multilang(LANG_IP_ADDRESS),
				multilang(LANG_GATEWAY), multilang(LANG_STATUS));

	entryNum = mib_chain_total(MIB_PPTP_TBL);
	for (i=0; i<entryNum; i++)
	{
		char mppp_ifname[IFNAMSIZ];
		char mppp_protocol[10];
		char mppp_ipaddr[20];
		char mppp_remoteip[20];
		char *mppp_status;
		struct in_addr inAddr;
		int flags;

		if (!mib_chain_get(MIB_PPTP_TBL, i, (void *)&Entry))
		{
			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		ifGetName(Entry.ifIndex, mppp_ifname, sizeof(mppp_ifname));
		strcpy( mppp_protocol, "PPP" );

		if (getInAddr( mppp_ifname, IP_ADDR, (void *)&inAddr) == 1)
		{
			sprintf( mppp_ipaddr, "%s",   inet_ntoa(inAddr) );
		}else
			strcpy( mppp_ipaddr, "" );

		if (getInAddr( mppp_ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
		{
			sprintf( mppp_remoteip, "%s",   inet_ntoa(inAddr) );
		}else
			strcpy( mppp_remoteip, "" );

		if (getInFlags( mppp_ifname, &flags) == 1)
		{
			if (flags & IFF_UP) {
				mppp_status = (char *)IF_UP;
			}else
				mppp_status = (char *)IF_DOWN;
		}else
			mppp_status = (char *)IF_NA;

		nBytesSent += boaWrite(wp, "  <tr bgcolor=\"#EEEEEE\">\n"
		                              "    <td align=center width=\"15%%\"><font size=2>%s</td>\n"
		                              "    <td align=center width=\"15%%\"><font size=2>%s</td>\n"
		                              "    <td align=center width=\"25%%\"><font size=2>%s</td>\n"
		                              "    <td align=center width=\"25%%\"><font size=2>%s</td>\n",
		                              mppp_ifname, mppp_protocol, mppp_ipaddr, mppp_remoteip);

		nBytesSent += boaWrite(wp, "    <td align=center width=\"20%%\"><font size=2>%s ",
							mppp_status );

		nBytesSent += boaWrite(wp, "	</td>\n");
		nBytesSent += boaWrite(wp, "  </tr>\n");

	}
	nBytesSent += boaWrite(wp, "</table>");
	return nBytesSent;
}
#else
int wanPPTPTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	return nBytesSent;
}
#endif //CONFIG_USER_PPTP_CLIENT_PPTP

#ifdef CONFIG_USER_L2TPD_L2TPD
int wanL2TPTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	MIB_L2TP_T Entry;
	unsigned int entryNum, i;

	nBytesSent += boaWrite(wp, "<br>\n");
	nBytesSent += boaWrite(wp, "<table width=600 border=0>\n");
	nBytesSent += boaWrite(wp, "  <tr>\n");
	nBytesSent += boaWrite(wp, "    <td width=100%% colspan=5 bgcolor=\"#008000\">\n"
	                              "      <font color=\"#FFFFFF\" size=2><b>L2TP %s</b></font>\n"
	                              "    </td>\n", multilang(LANG_CONFIGURATION));
	nBytesSent += boaWrite(wp, "  </tr>\n");

	nBytesSent += boaWrite(wp, "  <tr bgcolor=\"#808080\">\n"
                              	      "    <td width=\"15%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"15%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"25%%\" align=center><font size=2><b>%s %s</b></font></td>\n"
	                              "    <td width=\"25%%\" align=center><font size=2><b>%s %s</b></font></td>\n"
	                              "    <td width=\"20%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "  </tr>\n",
		multilang(LANG_INTERFACE), multilang(LANG_PROTOCOL), multilang(LANG_LOCAL),
		multilang(LANG_IP_ADDRESS), multilang(LANG_REMOTE), multilang(LANG_IP_ADDRESS),
		multilang(LANG_STATUS));

	entryNum = mib_chain_total(MIB_L2TP_TBL);
	for (i=0; i<entryNum; i++)
	{
		char mppp_ifname[IFNAMSIZ];
		char mppp_protocol[10];
		char mppp_ipaddr[20];
		char mppp_remoteip[20];
		char *mppp_status;
		struct in_addr inAddr;
		int flags;

		if (!mib_chain_get(MIB_L2TP_TBL, i, (void *)&Entry))
		{
			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		ifGetName(Entry.ifIndex, mppp_ifname, sizeof(mppp_ifname));
		strcpy( mppp_protocol, "PPP" );

		if (getInAddr( mppp_ifname, IP_ADDR, (void *)&inAddr) == 1)
		{
			sprintf( mppp_ipaddr, "%s",   inet_ntoa(inAddr) );
		}else
			strcpy( mppp_ipaddr, "" );

		if (getInAddr( mppp_ifname, DST_IP_ADDR, (void *)&inAddr) == 1)
		{
			sprintf( mppp_remoteip, "%s",   inet_ntoa(inAddr) );
		}else
			strcpy( mppp_remoteip, "" );

		if (getInFlags( mppp_ifname, &flags) == 1)
		{
			if (flags & IFF_UP) {
				mppp_status = (char *)IF_UP;
			}else
				mppp_status = (char *)IF_DOWN;
		}else
			mppp_status = (char *)IF_NA;

		nBytesSent += boaWrite(wp, "  <tr bgcolor=\"#EEEEEE\">\n"
		                              "    <td align=center width=\"15%%\"><font size=2>%s</td>\n"
		                              "    <td align=center width=\"15%%\"><font size=2>%s</td>\n"
		                              "    <td align=center width=\"25%%\"><font size=2>%s</td>\n"
		                              "    <td align=center width=\"25%%\"><font size=2>%s</td>\n",
		                              mppp_ifname, mppp_protocol, mppp_ipaddr, mppp_remoteip);

		nBytesSent += boaWrite(wp, "    <td align=center width=\"20%%\"><font size=2>%s ",
							mppp_status );

		nBytesSent += boaWrite(wp, "	</td>\n");
		nBytesSent += boaWrite(wp, "  </tr>\n");

	}
	nBytesSent += boaWrite(wp, "</table>");
	return nBytesSent;
}
#else
int wanL2TPTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	return nBytesSent;
}
#endif //CONFIG_USER_L2TPD_L2TPD

#ifdef CONFIG_NET_IPIP
int wanIPIPTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	MIB_IPIP_T Entry;
	unsigned int entryNum, i;

	nBytesSent += boaWrite(wp, "<br>\n");
	nBytesSent += boaWrite(wp, "<table width=600 border=0>\n");
	nBytesSent += boaWrite(wp, "  <tr>\n");
	nBytesSent += boaWrite(wp, "    <td width=100%% colspan=5 bgcolor=\"#008000\">\n"
	                              "      <font color=\"#FFFFFF\" size=2><b>IPIP %s</b></font>\n"
	                              "    </td>\n", multilang(LANG_CONFIGURATION));
	nBytesSent += boaWrite(wp, "  </tr>\n");

	nBytesSent += boaWrite(wp, "  <tr bgcolor=\"#808080\">\n"
                              	      "    <td width=\"15%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"15%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"25%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"25%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "    <td width=\"20%%\" align=center><font size=2><b>%s</b></font></td>\n"
	                              "  </tr>\n",
				multilang(LANG_INTERFACE), multilang(LANG_PROTOCOL), multilang(LANG_IP_ADDRESS),
				multilang(LANG_GATEWAY), multilang(LANG_STATUS));

	entryNum = mib_chain_total(MIB_IPIP_TBL);
	for (i=0; i<entryNum; i++)
	{
		char mppp_ifname[IFNAMSIZ];
		char mppp_protocol[10];
		char mppp_ipaddr[20];
		char mppp_remoteip[20];
		char *mppp_status;
		struct in_addr inAddr;
		int flags;

		if (!mib_chain_get(MIB_IPIP_TBL, i, (void *)&Entry))
		{
			boaError(wp, 400, "Get chain record error!\n");
			return -1;
		}

		snprintf(mppp_ipaddr, 20, "%s", inet_ntoa(*((struct in_addr *)&Entry.saddr)));
		snprintf(mppp_remoteip, 20, "%s", inet_ntoa(*((struct in_addr *)&Entry.daddr)));
		ifGetName(Entry.ifIndex, mppp_ifname, sizeof(mppp_ifname));
		strcpy( mppp_protocol, "IPinIP" );

		if (getInFlags( mppp_ifname, &flags) == 1)
		{
			if (flags & IFF_UP) {
				mppp_status = (char *)IF_UP;
			}else
				mppp_status = (char *)IF_DOWN;
		}else
			mppp_status = (char *)IF_DOWN;

		nBytesSent += boaWrite(wp, "  <tr bgcolor=\"#EEEEEE\">\n"
		                              "    <td align=center width=\"15%%\"><font size=2>%s</td>\n"
		                              "    <td align=center width=\"15%%\"><font size=2>%s</td>\n"
		                              "    <td align=center width=\"25%%\"><font size=2>%s</td>\n"
		                              "    <td align=center width=\"25%%\"><font size=2>%s</td>\n",
		                              mppp_ifname, mppp_protocol, mppp_ipaddr, mppp_remoteip);

		nBytesSent += boaWrite(wp, "    <td align=center width=\"20%%\"><font size=2>%s ",
							mppp_status );

		nBytesSent += boaWrite(wp, "	</td>\n");
		nBytesSent += boaWrite(wp, "  </tr>\n");

	}
	nBytesSent += boaWrite(wp, "</table>");
	return nBytesSent;
}
#else
int wanIPIPTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	return nBytesSent;
}
#endif //CONFIG_NET_IPIP

// Jenny, current status
int wanConfList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	int in_turn=0, ifcount=0;
	int i;
	struct wstatus_info sEntry[MAX_VC_NUM+MAX_PPP_NUM];

	ifcount = getWanStatus(sEntry, MAX_VC_NUM+MAX_PPP_NUM);
	nBytesSent += boaWrite(wp, "<tr bgcolor=\"#808080\">"
	"<td width=\"8%%\" align=center><font size=2><b>%s</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>%s</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>%s</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>%s</b></font></td>\n"
	"<td width=\"22%%\" align=center><font size=2><b>%s</b></font></td>\n"
	"<td width=\"22%%\" align=center><font size=2><b>%s</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>%s</b></font></td></tr>\n",
	multilang(LANG_INTERFACE), multilang(LANG_VPI_VCI), multilang(LANG_ENCAPSULATION),
	multilang(LANG_PROTOCOL), multilang(LANG_IP_ADDRESS), multilang(LANG_GATEWAY),
	multilang(LANG_STATUS));
	in_turn = 0;
	for (i=0; i<ifcount; i++) {
		if (in_turn == 0)
			nBytesSent += boaWrite(wp, "<tr bgcolor=\"#EEEEEE\">\n");
		else
			nBytesSent += boaWrite(wp, "<tr bgcolor=\"#DDDDDD\">\n");

		in_turn ^= 0x01;
		nBytesSent += boaWrite(wp,
		"<td align=center width=\"5%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"5%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"5%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"5%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"10%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"10%%\"><font size=2>%s</td>\n"
		"<td align=center width=\"23%%\"><font size=2>%s\n",
		sEntry[i].ifDisplayName, sEntry[i].vpivci, sEntry[i].encaps,
		sEntry[i].protocol, sEntry[i].ipAddr, sEntry[i].remoteIp, sEntry[i].strStatus);
		if (sEntry[i].cmode == CHANNEL_MODE_PPPOE || sEntry[i].cmode == CHANNEL_MODE_PPPOA) { // PPP mode
			nBytesSent += boaWrite(wp, " %s / %s ", sEntry[i].uptime, sEntry[i].totaluptime);
			if (sEntry[i].link_state && !sEntry[i].pppDoD)
				if (sEntry[i].cmode == CHANNEL_MODE_PPPOE)
					nBytesSent += boaWrite(wp,
					"<input type=\"submit\" id=\"%s\" value=\"%s\" name=\"submit%s\" onClick=\"disButton('%s')\">",
					sEntry[i].ifname, (sEntry[i].itf_state==1) ? "Disconnect" : "Connect",
					sEntry[i].ifname,sEntry[i].ifname);
				else
					nBytesSent += boaWrite(wp,
					"<input type=\"submit\" value=\"%s\" name=\"submit%s\">"
					, (sEntry[i].itf_state==1) ? "Disconnect" : "Connect",
					sEntry[i].ifname);
		}
		nBytesSent += boaWrite(wp, "</td></tr>\n");
	}
	return nBytesSent;
}

int DHCPSrvStatus(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
#ifndef CONFIG_SFU
	char vChar = 0;
#ifdef CONFIG_USER_DHCP_SERVER
	if ( !mib_get( MIB_DHCP_MODE, (void *)&vChar) )
		return -1;
#endif

	nBytesSent += boaWrite(wp,"<tr bgcolor=\"#EEEEEE\">\n"
				"<td width=\"40%%\">\n"
				"<font size=2><b>DHCP %s</b></td>\n<td width=\"60%%\">\n"
    				"<font size=2>%s</td></tr>\n",
      				multilang(LANG_SERVER),DHCP_LAN_SERVER == vChar?multilang(LANG_ENABLED): multilang(LANG_DISABLED));
#endif

	return nBytesSent;
}

#ifdef CONFIG_RTL8672_SAR
#define FM_DSL_VER \
"<tr bgcolor=\"#DDDDDD\">" \
"<td width=40%%><font size=2><b>%s</b></td>" \
"<td width=60%%><font size=2>%s</td>" \
"</tr>"

int DSLVer(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent = 0;
	char s_ver[64];

	if(!(WAN_MODE & MODE_ATM) && !(WAN_MODE & MODE_PTM))
		return 0;

	getAdslInfo(ADSL_GET_VERSION, s_ver, 64);
	nBytesSent += boaWrite(wp, FM_DSL_VER, multilang(LANG_DSP_VERSION) , s_ver);

#ifdef CONFIG_USER_XDSL_SLAVE
	getAdslSlvInfo(ADSL_GET_VERSION, s_ver, 64);
	nBytesSent += boaWrite(wp, FM_DSL_VER, multilang(LANG_DSP_SLAVE_VERSION), s_ver);
#endif /*CONFIG_USER_XDSL_SLAVE*/

  return nBytesSent;
}

int DSLStatus(int eid, request * wp, int argc, char **argv)
{
	const char FM_DSL_STATUS[] =
		"<tr>\n"
		"<td width=100%% colspan=\"2\" bgcolor=\"#008000\"><font color=\"#FFFFFF\" size=2><b>%s</b></font></td>\n"
		"</tr>\n"
		"<tr bgcolor=\"#EEEEEE\">\n"
		"<td width=40%%><font size=2><b>Operational Status</b></td>\n"
		"<td width=60%%><font size=2>%s</td>\n"
		"</tr>\n"
		"<tr bgcolor=\"#DDDDDD\">\n"
		"<td width=40%%><font size=2><b>Upstream Speed</b></td>\n"
		"<td width=60%%><font size=2>%s&nbsp;kbps&nbsp;</td>\n"
		"</tr>\n"
		"<tr bgcolor=\"#EEEEEE\">\n"
		"<td width=40%%><font size=2><b>Downstream Speed</b></td>\n"
		"<td width=60%%><font size=2>%s&nbsp;kbps&nbsp;</td>\n"
		"</tr>\n";

	int nBytesSent = 0;
	char o_status[64], u_speed[16], d_speed[16];

	if(!(WAN_MODE & MODE_ATM) && !(WAN_MODE & MODE_PTM))
		return 0;

	getSYS2Str(SYS_DSL_OPSTATE, o_status);
	getAdslInfo(ADSL_GET_RATE_US, u_speed, 16);
	getAdslInfo(ADSL_GET_RATE_DS, d_speed, 16);

	nBytesSent += boaWrite(wp, FM_DSL_STATUS, "DSL", o_status, u_speed, d_speed);

#ifdef LOOP_LENGTH_METER
		char distance[16];
		const char FM_DSL_STATUS_LLM[] = "<tr bgcolor=\"#DDDDDD\">\n"
		"<td width=40%%><font size=2><b>Distance Measurement</b></td>\n"
		"<td width=60%%><font size=2>%s&nbsp;(m)&nbsp;</td>\n"
		"</tr>\n";

	getAdslInfo(ADSL_GET_LOOP_LENGTH_METER, distance, 16);
	nBytesSent += boaWrite(wp, FM_DSL_STATUS_LLM, distance);
#endif

#ifdef CONFIG_USER_XDSL_SLAVE
	getSYS2Str(SYS_DSL_SLV_OPSTATE, o_status);
	getAdslSlvInfo(ADSL_GET_RATE_US, u_speed, 16);
	getAdslSlvInfo(ADSL_GET_RATE_DS, d_speed, 16);

	nBytesSent += boaWrite(wp, FM_DSL_STATUS, "DSL Slave", o_status, u_speed, d_speed);
#endif /*CONFIG_USER_XDSL_SLAVE*/

  return nBytesSent;
}
#endif


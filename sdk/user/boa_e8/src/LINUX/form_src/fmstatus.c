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
#include "boa.h"
#include "fmdefs.h"		// Mason Yu. 2630-e8b

static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";
#ifdef EMBED
#ifdef CONFIG_USER_PPPOMODEM
const char PPPOM_CONF[] = "/var/ppp/pppom.conf";
#endif //CONFIG_USER_PPPOMODEM
#endif

void formStatus(request * wp, char *path, char *query)
{
	char *submitUrl, *strSubmitR, *strSubmitP;
	struct data_to_pass_st msg;
	char tmpBuf[100], buff[256];
	unsigned int i,flag, inf;
	FILE *fp;
#ifdef CONFIG_USER_PPPOMODEM
	unsigned int cflag[MAX_PPP_NUM+MAX_MODEM_PPPNUM]={0};
#else
	unsigned int cflag[MAX_PPP_NUM]={0};
#endif //CONFIG_USER_PPPOMODEM

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
				if ((strcmp(strSubmitP, "Connect") == 0))
				{
					if (cflag[i]) {
						snprintf(msg.data, BUF_SIZE, "spppctl up %u", i);
						usleep(3000000);
						TRACE(STA_SCRIPT, "%s\n", msg.data);
						write_to_pppd(&msg);
							//add by ramen to resolve for clicking "connect" button twice.
					}
				}
				else if (strcmp(strSubmitP, "Disconnect") == 0)
				{

					snprintf(msg.data, BUF_SIZE, "spppctl down %u", i);
					TRACE(STA_SCRIPT, "%s\n", msg.data);
					write_to_pppd(&msg);
						//add by ramen to resolve for clicking "disconnect" button twice.
				}
				else {
					strcpy(tmpBuf, "不合法的PPP action!"); //不合法的PPP action!
					goto setErr_filter;
				}
			}
		}
	}

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

	OK_MSG1("系统时间已修改成功<p>"
		"请重整你的\"Status\"页面", NULL); //System Date has been modified successfully. Please reflesh your \"Status\" page.
	return;
}

//jim china telecom
#ifdef CTC_WAN_NAME
//jim we define it in utility.c
/*
static int setWanName(char* str, int applicationtype)
{
	switch(applicationtype)
	{
		//Internet
		case 0:
			strcpy(str, "Internet_");
			break;
		//TR069_Internet
		case 1:
			strcpy(str, "TR069_Internet_");
			break;
			//TR069
		case 2:
			strcpy(str, "TR069_");
			break;
		//Others
		case 3:
			strcpy(str, "Other_");
			break;
		default:
			strcpy(str, "Internet_");
			break;
	}
}*/
int getATMEntrybyVPIVCIUsrPswd(MIB_CE_ATM_VC_T* Entry, int vpi, int vci, char* username, char* password, char *ifname)
{
	int entryNum;
	int mibcnt;
	int ret=0;
	char tmpifname[IFNAMSIZ];
	if(Entry==NULL )
	{
		return -1;
	}
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for(mibcnt=0; mibcnt<entryNum; mibcnt++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, (void *)Entry))
		{
			return -1;
		}

		ifGetName(Entry->ifIndex, tmpifname, sizeof(tmpifname));

		if(username==NULL || password==NULL)
		{
			//ignore PPP
			if((Entry->cmode!=CHANNEL_MODE_PPPOE &&Entry->cmode!=CHANNEL_MODE_PPPOA) &&
					Entry->vpi==vpi && Entry->vci==vci && !strcmp(tmpifname, ifname))
				return 0; //found...
		}
		else if((Entry->cmode==CHANNEL_MODE_PPPOE ||Entry->cmode==CHANNEL_MODE_PPPOA)&& Entry->vpi==vpi && Entry->vci==vci &&
			!strcmp(Entry->pppUsername, username)    && !strcmp(Entry->pppPassword, password) && !strcmp(tmpifname, ifname))
		{
			return 0; //found
		}
	}
	return -1; //not found
}
#endif

#ifdef CONFIG_USER_PPPOMODEM
int wan3GTable(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	nBytesSent += boaWrite(wp, "<br>\n");
	nBytesSent += boaWrite(wp, "<table width=600 border=0>\n");
	nBytesSent += boaWrite(wp, "  <tr>\n");
	nBytesSent += boaWrite(wp, "    <td width=100%% colspan=5 bgcolor=\"#008000\">\n"
	                              "      <font color=\"#FFFFFF\" size=2><b>3G Configuration</b></font>\n"
	                              "    </td>\n");
	nBytesSent += boaWrite(wp, "  </tr>\n");

	nBytesSent += boaWrite(wp, "  <tr bgcolor=\"#808080\">\n"
                              	      "    <td width=\"15%%\" align=center><font size=2><b>Interface</b></font></td>\n"
	                              "    <td width=\"15%%\" align=center><font size=2><b>Protocol</b></font></td>\n"
	                              "    <td width=\"25%%\" align=center><font size=2><b>IP Address</b></font></td>\n"
	                              "    <td width=\"25%%\" align=center><font size=2><b>Gateway</b></font></td>\n"
	                              "    <td width=\"20%%\" align=center><font size=2><b>Status</b></font></td>\n"
	                              "  </tr>\n");

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
			struct flock flpom;
			int fdpom;

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
			//file unlocking
			if (fdpom != -1) {
				flpom.l_type = F_UNLCK;
				if (fcntl(fdpom, F_SETLK, &flpom) == -1)
					printf("pppom write unlock failed\n");
				close(fdpom);
				//printf( "wan3GTable: pppom write unlock successfully\n" );
			}

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
									(pppConnectStatus==1)?"Disconnect":"Connect", mppp_ifname);
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

// Jenny, current status
int wanConfList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	int in_turn=0, ifcount=0;
	int i;
	struct wstatus_info sEntry[MAX_VC_NUM+MAX_PPP_NUM];

	ifcount = getWanStatus(sEntry, MAX_VC_NUM+MAX_PPP_NUM);
	nBytesSent += boaWrite(wp, "<tr bgcolor=\"#808080\">"
	"<td width=\"8%%\" align=center><font size=2><b>Interface</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>VPI/VCI</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>Encap</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>Protocol</b></font></td>\n"
	"<td width=\"22%%\" align=center><font size=2><b>IP Address</b></font></td>\n"
	"<td width=\"22%%\" align=center><font size=2><b>Gateway</b></font></td>\n"
	"<td width=\"12%%\" align=center><font size=2><b>Status</b></font></td></tr>\n");
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
					sEntry[i].ifname, (sEntry[i].itf_state==1)?"Disconnect":"Connect", sEntry[i].ifname,sEntry[i].ifname);
				else
					nBytesSent += boaWrite(wp,
					"<input type=\"submit\" value=\"%s\" name=\"submit%s\">"
					, (sEntry[i].itf_state==1)?"Disconnect":"Connect", sEntry[i].ifname);
		}
		nBytesSent += boaWrite(wp, "</td></tr>\n");
	}
	return nBytesSent;
}

// Mason Yu. 2630-e8b
#ifdef TIME_ZONE
//add by chenzhuoxin
void formTimezone(request * wp, char *path, char *query)
{
	char *strVal;
	unsigned int index;
	unsigned char dstEnabled, ntpEnabled;
	char *serverHost1;
	char *serverHost2;
	unsigned char if_type;
	unsigned int interval, if_wan;

	/*
	 *Add code by Realtek
	 */
	printf("enter formTimezone\n");
	strVal = boaGetVar(wp, "tmzone", "");
	if (strVal[0])
		index = strtoul(strVal, NULL, 0);
	printf("tmzone=%u\n", index);

	strVal = boaGetVar(wp, "dstEnabled", "");
	if (0 == strcmp(strVal, "on"))
		dstEnabled = 1;
	else
		dstEnabled = 0;

	strVal = boaGetVar(wp, "ntpEnabled", "");
	if (0 == strcmp(strVal, "on"))
		ntpEnabled = 1;
	else
		ntpEnabled = 0;
	printf("ntpEnabled=%d\n", ntpEnabled);

	strVal = boaGetVar(wp, "server1", "");
	if (strVal[0]) {
		printf("server1 = %s\n", strVal);
		serverHost1 = strVal;
	} else
		serverHost1 = "";

	strVal = boaGetVar(wp, "server2", "");
	if (strVal[0]) {
		printf("server2 = %s\n", strVal);
		serverHost2 = strVal;
	} else
		serverHost2 = "";

	strVal = boaGetVar(wp, "if_type", "0");
	if_type = strVal[0] - '0';
	printf("if_type=%d\n", if_type);

	strVal = boaGetVar(wp, "if_wan", "65535");
	if_wan = strtoul(strVal, NULL, 0);
	printf("if_wan=%d\n", if_wan);

	strVal = boaGetVar(wp, "interval", "86400");
	interval = atoi(strVal);
	printf("interval=%d\n", interval);

	/*
	 *Add code by Realtek
	 */
	mib_set(MIB_NTP_TIMEZONE_DB_INDEX, &index);
	mib_set(MIB_DST_ENABLED, &dstEnabled);

	setupNtp(SNTP_DISABLED);
	mib_set(MIB_NTP_ENABLED, &ntpEnabled);
	if (ntpEnabled) {
		mib_set(MIB_NTP_SERVER_HOST1, serverHost1);
		mib_set(MIB_NTP_SERVER_HOST2, serverHost2);
		mib_set(MIB_NTP_IF_TYPE, &if_type);
		mib_set(MIB_NTP_IF_WAN, &if_wan);
		mib_set(MIB_NTP_INTERVAL, &interval);
	}
	setupNtp(SNTP_ENABLED);

	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);

	_COND_REDIRECT;
	return;

}

//add by chenzhuoxin
int init_sntp_page(int eid, request * wp, int argc, char **argv)
{
	unsigned char vUChar;
	char ip[MAX_NAME_LEN];
	char fixip[MAX_NAME_LEN * 2];
	unsigned char if_type;	//0: INTERNET, 1: VOICE, 2: TR069, 3: OTHER
	unsigned int interval, if_wan;

	/*
	 *Add code by Realtek
	 */

	//Debug
	printf("enter init_sntp_page\n");

	mib_get(MIB_DST_ENABLED, &vUChar);
	if (vUChar == 1)
		boaWrite(wp, "dstEnabled.checked = true;\n");

	mib_get(MIB_NTP_ENABLED, &vUChar);
	if (vUChar == 1)
		boaWrite(wp, "ntpEnabled.checked = true;\n");

	//strcpy(ip, "clock.ngc.he.net");  //for debug
	mib_get(MIB_NTP_SERVER_HOST1, ip);
	boaWrite(wp, "server1.value = \"%s\";\n",
		 fixSpecialChar(fixip, ip, sizeof(fixip)));

	//strcpy(ip, "none");        //for debug
	mib_get(MIB_NTP_SERVER_HOST2, ip);
	//printf("ip=%s ip[0]=%d\n", ip, ip[0]);
	if (ip[0] != '\0')
		boaWrite(wp, "server2.value = \"%s\";\n",
			 fixSpecialChar(fixip, ip, sizeof(fixip)));

	mib_get(MIB_NTP_IF_TYPE, &if_type);
	boaWrite(wp, "if_type.selectedIndex = %hhu;\n",
			if_type);

	boaWrite(wp, "if_typeChange();\n", if_wan);
	mib_get(MIB_NTP_IF_WAN, &if_wan);
	boaWrite(wp, "if_wan.value = %u;\n", if_wan);

	mib_get(MIB_NTP_INTERVAL, &interval);
	boaWrite(wp, "interval.value = %u;\n",
			interval);

	/*
	 *Add code by Realtek
	 */
	printf("leave %s", __FUNCTION__);
	return 0;

}
#endif

int addHttpNoCache(int eid, request * wp, int argc, char ** argv)
{
	boaWrite(wp, "<HEAD>\n<META HTTP-EQUIV=\"Pragma\" CONTENT=\"no-cache\">\n</HEAD>\n");
}

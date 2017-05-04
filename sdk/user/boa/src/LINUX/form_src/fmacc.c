/*
 *      Web server handler routines for service access control
 *
 */

/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>

#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "multilang.h"

#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif

#ifdef REMOTE_ACCESS_CTL

void formAcc(request * wp, char *path, char *query)
{
	char *submitUrl, *strVal;
	char tmpBuf[100];
	unsigned int i;
	MIB_CE_ACC_T Entry;
	MIB_CE_ACC_T entry;
	unsigned char vChar;
	unsigned int  uPort;
#ifndef NO_ACTION
	int pid;
#endif

//#ifdef APPLY_CHANGE
	// remove entries
	filter_set_remote_access(0);
//#endif
	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry)) {
		memset(&entry, '\0', sizeof(MIB_CE_ACC_T));
		mib_chain_add(MIB_ACC_TBL, (unsigned char*)&entry);
		mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry);
	}

	memset(&Entry, '\0', sizeof(MIB_CE_ACC_T));

	// telnet
	#ifdef CONFIG_USER_TELNETD_TELNETD
	strVal = boaGetVar(wp, "l_telnet", "");
	if (strVal[0]=='1')
		Entry.telnet |= 0x02;
	strVal = boaGetVar(wp, "w_telnet", "");
	if (strVal[0]=='1')
		Entry.telnet |= 0x01;

	uPort = 0;
	strVal = boaGetVar(wp, "w_telnet_port", "");
	if (strVal[0]) {
		sscanf(strVal, "%u", &uPort);
		Entry.telnet_port = uPort;
	}
	if (!Entry.telnet_port)
		Entry.telnet_port = 23;
	#endif

	// ftp
	#ifdef CONFIG_USER_FTPD_FTPD
	strVal = boaGetVar(wp, "l_ftp", "");
	if (strVal[0]=='1')
		Entry.ftp |= 0x02;
	strVal = boaGetVar(wp, "w_ftp", "");
	if (strVal[0]=='1')
		Entry.ftp |= 0x01;

	uPort = 0;
	strVal = boaGetVar(wp, "w_ftp_port", "");
	if (strVal[0]) {
		sscanf(strVal, "%u", &uPort);
		Entry.ftp_port = uPort;
	}
	if (!Entry.ftp_port)
		Entry.ftp_port = 21;
        #endif

	// tftp
	#ifdef CONFIG_USER_TFTPD_TFTPD
	strVal = boaGetVar(wp, "l_tftp", "");
	#ifdef CONFIG_USER_WLAN_QCSAPI
	Entry.tftp |= 0x02;
	#else
	if (strVal[0]=='1')
		Entry.tftp |= 0x02;
	#endif
	strVal = boaGetVar(wp, "w_tftp", "");
	if (strVal[0]=='1')
		Entry.tftp |= 0x01;
	#endif

	// web
	strVal = boaGetVar(wp, "l_web", "");
	if (strVal[0]=='1')
		Entry.web |= 0x02;
	strVal = boaGetVar(wp, "w_web", "");
	if (strVal[0]=='1')
		Entry.web |= 0x01;
	uPort = 0;
	strVal = boaGetVar(wp, "w_web_port", "");
	if (strVal[0]) {
		sscanf(strVal, "%u", &uPort);
		Entry.web_port = uPort;
	}
	if (!Entry.web_port)
		Entry.web_port = 80;

	//https
	#ifdef CONFIG_USER_BOA_WITH_SSL
	strVal = boaGetVar(wp, "l_https", "");
	printf("get from web :%s",strVal);
	if (strVal[0]=='1')
		Entry.https |= 0x02;
	strVal = boaGetVar(wp, "w_https", "");
	printf("get from web :%s",strVal);
	if (strVal[0]=='1')
		Entry.https |= 0x01;
	uPort = 0;
	strVal = boaGetVar(wp, "w_https_port", "");
	printf("get from web :%s",strVal);
	if (strVal[0]) {
		sscanf(strVal, "%u", &uPort);
		Entry.https_port = uPort;
	}
	if (!Entry.https_port)
		Entry.https_port = 443;
	#endif //end of CONFIG_USER_BOA_WITH_SSL

	// snmp
	#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	strVal = boaGetVar(wp, "l_snmp", "");
	if (strVal[0]=='1')
		Entry.snmp |= 0x02;
	strVal = boaGetVar(wp, "w_snmp", "");
	if (strVal[0]=='1')
		Entry.snmp |= 0x01;
	#endif

	// ssh
	#ifdef CONFIG_USER_SSH_DROPBEAR
	strVal = boaGetVar(wp, "l_ssh", "");
	if (strVal[0]=='1')
		Entry.ssh |= 0x02;
	strVal = boaGetVar(wp, "w_ssh", "");
	if (strVal[0]=='1')
		Entry.ssh |= 0x01;
	#endif

	// icmp
	// LAN side access is always enabled
	strVal = boaGetVar(wp, "w_icmp", "");
	Entry.icmp |= 0x02;
	if (strVal[0]=='1')
		Entry.icmp |= 0x01;

setOk_acc:
	// log message
	mib_chain_update(MIB_ACC_TBL, (void *)&Entry, 0);
	// Apply to changes
	filter_set_remote_access(1);

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

#ifndef NO_ACTION
	pid = fork();
	if (pid) {
		waitpid(pid, NULL, 0);
	}
	else if (pid == 0) {
		snprintf(tmpBuf, 100, "%s/%s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG);
		execl( tmpBuf, _FIREWALL_SCRIPT_PROG, NULL);
		exit(1);
	}
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
 	return;

setErr_acc:
	ERR_MSG(tmpBuf);
}

// Post the services access control configuration at web page.
//
int accPost(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;
	unsigned int i, k;
	MIB_CE_ACC_T Entry;
	char strDocAcc[]="document.acc.";

	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry))
		return 0;
	#ifdef CONFIG_USER_TELNETD_TELNETD
	if (Entry.telnet & 0x02)
		nBytesSent += boaWrite(wp, "%sl_telnet.checked=true;\n", strDocAcc);
	if (Entry.telnet & 0x01)
		nBytesSent += boaWrite(wp, "%sw_telnet.checked=true;\n", strDocAcc);
	#endif
	#ifdef CONFIG_USER_FTPD_FTPD
	if (Entry.ftp & 0x02)
		nBytesSent += boaWrite(wp, "%sl_ftp.checked=true;\n", strDocAcc);
	if (Entry.ftp & 0x01)
		nBytesSent += boaWrite(wp, "%sw_ftp.checked=true;\n", strDocAcc);
	#endif
	#ifdef CONFIG_USER_TFTPD_TFTPD
	if (Entry.tftp & 0x02)
		nBytesSent += boaWrite(wp, "%sl_tftp.checked=true;\n", strDocAcc);
	if (Entry.tftp & 0x01)
		nBytesSent += boaWrite(wp, "%sw_tftp.checked=true;\n", strDocAcc);
	#endif
	if (Entry.web & 0x02)
		nBytesSent += boaWrite(wp, "%sl_web.checked=true;\n", strDocAcc);
	if (Entry.web & 0x01)
		nBytesSent += boaWrite(wp, "%sw_web.checked=true;\n", strDocAcc);
	#ifdef CONFIG_USER_BOA_WITH_SSL
	if (Entry.https & 0x02)
		nBytesSent += boaWrite(wp, "%sl_https.checked=true;\n", strDocAcc);
	if (Entry.https & 0x01)
		nBytesSent += boaWrite(wp, "%sw_https.checked=true;\n", strDocAcc);
	#endif //end of CONFIG_USER_BOA_WITH_SSL
	#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	if (Entry.snmp & 0x02)
		nBytesSent += boaWrite(wp, "%sl_snmp.checked=true;\n", strDocAcc);
	if (Entry.snmp & 0x01)
		nBytesSent += boaWrite(wp, "%sw_snmp.checked=true;\n", strDocAcc);
	#endif
	#ifdef CONFIG_USER_SSH_DROPBEAR
	if (Entry.ssh & 0x02)
		nBytesSent += boaWrite(wp, "%sl_ssh.checked=true;\n", strDocAcc);
	if (Entry.ssh & 0x01)
		nBytesSent += boaWrite(wp, "%sw_ssh.checked=true;\n", strDocAcc);
	#endif
	// LAN side access is always enabled
	if (Entry.icmp & 0x01)
		nBytesSent += boaWrite(wp, "%sw_icmp.checked=true;\n", strDocAcc);
	#ifdef CONFIG_USER_TELNETD_TELNETD
	nBytesSent += boaWrite(wp, "%sw_telnet_port.value=%d;\n", strDocAcc, Entry.telnet_port);
	#endif
	#ifdef CONFIG_USER_FTPD_FTPD
	nBytesSent += boaWrite(wp, "%sw_ftp_port.value=%d;\n", strDocAcc, Entry.ftp_port);
	#endif
	nBytesSent += boaWrite(wp, "%sw_web_port.value=%d;\n", strDocAcc, Entry.web_port);
	#ifdef CONFIG_USER_BOA_WITH_SSL
	nBytesSent += boaWrite(wp, "%sw_https_port.value=%d;\n", strDocAcc, Entry.https_port);
	#endif
	return nBytesSent;
}

int accItem(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent = 0;

#ifdef CONFIG_USER_TELNETD_TELNETD
	nBytesSent += boaWrite(wp, "<tr>\n"
		"<td align=left class=tdkind>%s</td>\n"
		"<td align=center><input type=checkbox name=l_telnet value=1></td>\n"
		"<td align=center><input type=checkbox name=w_telnet value=1></td>\n"
		"<td align=center><input type=text size=5 name=w_telnet_port></td>\n</tr>\n",
		multilang(LANG_TELNET));
#else
	nBytesSent += boaWrite(wp, "<input type=hidden name=w_telnet_port value=65536>\n");
#endif

#ifdef CONFIG_USER_FTPD_FTPD
	nBytesSent += boaWrite(wp, "<tr>\n"
		"<td align=left class=tdkind>%s</td>\n"
		"<td align=center><input type=checkbox name=l_ftp value=1></td>\n"
		"<td align=center><input type=checkbox name=w_ftp value=1></td>\n"
		"<td align=center><input type=text size=5 name=w_ftp_port></td>\n</tr>\n",
		multilang(LANG_FTP));
#else
	nBytesSent += boaWrite(wp, "<input type=hidden name=w_ftp_port value=65537>\n");
#endif

#ifdef CONFIG_USER_TFTPD_TFTPD
	nBytesSent += boaWrite(wp, "<tr>\n"
		"<td align=left class=tdkind>%s</td>\n"
		#ifdef CONFIG_USER_WLAN_QCSAPI
		"<td align=center><input type=checkbox name=l_tftp checked disabled></td>\n"
		#else
		"<td align=center><input type=checkbox name=l_tftp value=1></td>\n"
		#endif
		"<td align=center><input type=checkbox name=w_tftp value=1></td>\n</tr>\n",
		multilang(LANG_TFTP));
#endif

	nBytesSent += boaWrite(wp, "<tr>\n"
	       "<td align=left class=tdkind>%s</td>\n"
		"<td align=center><input type=checkbox name=l_web value=1></td>\n"
		"<td align=center><input type=checkbox name=w_web value=1></td>\n"
		"<td align=center><input type=text size=5 name=w_web_port></td>\n</tr>\n",
		multilang(LANG_HTTP));

#ifdef CONFIG_USER_BOA_WITH_SSL
	nBytesSent += boaWrite(wp, "<tr>\n"
		"<td align=left class=tdkind>HTTPS</td>\n"
		"<td align=center><input type=checkbox name=l_https value=1></td>\n"
		"<td align=center><input type=checkbox name=w_https value=1></td>\n"
		"<td align=center><input type=text size=5 name=w_https_port></td>\n</tr>\n");
#else //end of CONFIG_USER_BOA_WITH_SSL
	nBytesSent += boaWrite(wp, "<input type=hidden name=w_https_port value=65538>\n");
#endif

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	nBytesSent += boaWrite(wp, "<tr>\n"
		"<td align=left class=tdkind>SNMP</td>\n"
		"<td align=center><input type=checkbox name=l_snmp value=1></td>\n"
		"<td align=center><input type=checkbox name=w_snmp value=1></td>\n</tr>\n");
#endif

#ifdef CONFIG_USER_SSH_DROPBEAR
	nBytesSent += boaWrite(wp, "<tr>\n"
		"<td align=left class=tdkind>Secure Shell(SSH)</td>\n"
		"<td align=center><input type=checkbox name=l_ssh value=1></td>\n"
		"<td align=center><input type=checkbox name=w_ssh value=1></td>\n</tr>\n");
#endif

	nBytesSent += boaWrite(wp, "<tr>\n"
		"<td align=left class=tdkind>%s</td>\n"
		"<td align=center><input type=checkbox name=l_icmp checked disabled></td>\n"
		"<td align=center><input type=checkbox name=w_icmp value=1></td>\n</tr>\n",
		multilang(LANG_PING));

	return nBytesSent;
}
#endif // of REMOTE_ACCESS_CTL


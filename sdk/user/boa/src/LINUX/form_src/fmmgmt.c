/*
 *      Web server handler routines for management (password, save config, f/w update)
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *      Authors: Dick Tam	<dicktam@realtek.com.tw>
 *
 */

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <crypt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <signal.h>
#include <stdint.h>
#include <linux/atm.h>
#include <linux/atmdev.h>

#include "../webs.h"
#include "../um.h"
#include "mib.h"
#include "webform.h"
#include "adsl_drv.h"
#include "utility.h"
#include "rtl_flashdrv.h"

//xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
#include <syslog.h>
#include "boa.h"
#endif

//ql_xu add
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

// Mason Yu
#ifdef EMBED
#include <linux/config.h>
#include <linux/sockios.h>	//cathy
#else
#include "../../../../include/linux/autoconf.h"
#endif


//xl_yue
#ifdef ACCOUNT_LOGIN_CONTROL
#include <syslog.h>
#include "boa.h"
#endif
#include "../defs.h"
#include "multilang.h"
#include "multilang_set.h" // Added by davian_kuo.
#ifdef TIME_ZONE
#include "tz.h"
#endif

#ifdef CONFIG_SFU
#include <rtk/stat.h>
#endif

#define DEFAULT_GROUP		"administrators"
#define ACCESS_URL			"/"
#define _PATH_PROCNET_DEV	"/proc/net/dev"
#define MAX_DSL_TONE		512

static int srandomCalled = 0;
char g_rUserName[MAX_NAME_LEN];
char g_rUserPass[MAX_NAME_LEN];
bool_t g_remoteUpdate;

short *snr;
short *qln;
short *hlog;
static int diagflag=1;

//Added by Andrew
static unsigned char psd_bit_en;
static unsigned char psd_tone[8];
static float psd_us[8];
static char psd_measure;


// Added by Mason Yu
extern int g_filesize;
#ifdef ENABLE_SIGNATURE_ADV
extern int upgrade;
#endif
extern int g_upgrade_firmware;
#ifdef USE_LOGINWEB_OF_SERVER
// Mason Yu on True
unsigned char g_login_username[MAX_NAME_LEN];
#endif

#ifdef CONFIG_USER_XMLCONFIG
extern const char *shell_name;
#endif

static void write_etcPassword()
{
	FILE *fp;
	char userName[MAX_NAME_LEN], userPass[MAX_NAME_LEN];
	char *xpass;
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T entry;
	unsigned int totalEntry;
#endif
	int i;

	fp = fopen("/var/passwd", "w+");
#ifdef ACCOUNT_CONFIG
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&entry)) {
			printf("ERROR: Get account configuration information from MIB database failed.\n");
			return;
		}
		strcpy(userName, entry.userName);
		strcpy(userPass, entry.userPassword);
		xpass = crypt(userPass, "$1$");
		if (xpass) {
			if (entry.privilege == (unsigned char)PRIV_ROOT)
				fprintf(fp, "%s:%s:0:0::%s:%s\n", userName, xpass, PW_HOME_DIR, PW_CMD_SHELL);
			else
				fprintf(fp, "%s:%s:1:0::%s:%s\n", userName, xpass, PW_HOME_DIR, PW_CMD_SHELL);
		}
	}
#endif
	mib_get( MIB_SUSER_NAME, (void *)userName );
	mib_get( MIB_SUSER_PASSWORD, (void *)userPass );
	xpass = crypt(userPass, "$1$");
	if (xpass)
		fprintf(fp, "%s:%s:0:0::%s:%s\n", userName, xpass, PW_HOME_DIR, PW_CMD_SHELL);

	// Added by Mason Yu for others user
	mib_get( MIB_SUPER_NAME, (void *)userName );
	mib_get( MIB_SUPER_PASSWORD, (void *)userPass );
	xpass = crypt(userPass, "$1$");
	if (xpass)
		fprintf(fp, "%s:%s:0:0::%s:%s\n", userName, xpass, PW_HOME_DIR, PW_CMD_SHELL);

	mib_get( MIB_USER_NAME, (void *)userName );
	if (userName[0]) {
		mib_get( MIB_USER_PASSWORD, (void *)userPass );
		xpass = crypt(userPass, "$1$");
		if (xpass)
			fprintf(fp, "%s:%s:1:0::%s:%s\n", userName, xpass, PW_HOME_DIR, PW_CMD_SHELL);
	}

	fclose(fp);
	chmod(PW_HOME_DIR, 0x1fd);	// let owner and group have write access
}

#ifdef ACCOUNT_CONFIG
extern char suName[MAX_NAME_LEN];
extern char usName[MAX_NAME_LEN];
// Jenny, user account configuration
/////////////////////////////////////////////////////////////////////////////
int accountList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent=0;

	unsigned int entryNum, i;
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	char	*priv;
	char upasswd[MAX_NAME_LEN];

	nBytesSent += boaWrite(wp, "<tr><font size=2>"
	"<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=2>%s</td>\n"
	"<td align=center width=\"50%%\" bgcolor=\"#808080\"><font size=2>%s%s</td>\n"
	"<td align=center width=\"30%%\" bgcolor=\"#808080\"><font size=2>%s</td></font></tr>\n",
	multilang(LANG_SELECT), multilang(LANG_USER), multilang(LANG_NAME_2), multilang(LANG_PRIVILEGE));

	/*if (!mib_get(MIB_SUSER_PASSWORD, (void *)upasswd)) {
		printf("ERROR: Get superuser password from MIB database failed.\n");
		return;
	}*/
	nBytesSent += boaWrite(wp, "<tr>"
	"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//	" value=\"s0\" onClick=\"postEntry('%s', %d, '%s')\"></td>\n"),
//	suName, PRIV_ROOT, upasswd);
	" value=\"s0\" onClick=\"postEntry('%s', %d)\"></td>\n",
	suName, PRIV_ROOT);
	nBytesSent += boaWrite(wp,
	"<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
	"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>Admin</b></font></td></tr>\n",
	suName);

	/*if (!mib_get(MIB_USER_PASSWORD, (void *)upasswd)) {
		printf("ERROR: Get user password from MIB database failed.\n");
		return;
	}*/
	nBytesSent += boaWrite(wp, "<tr>"
	"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//	" value=\"s1\" onClick=\"postEntry('%s', %d, '%s')\"></td>\n"),
//	usName, PRIV_USER, upasswd);
	" value=\"s1\" onClick=\"postEntry('%s', %d)\"></td>\n",
	usName, PRIV_USER);
	nBytesSent += boaWrite(wp,
	"<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
	"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>User</b></font></td></tr>\n",
	usName);

	entryNum = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry)) {
  			boaError(wp, 400, strGetChainerror);
			return -1;
		}

		priv = 0;
		if (Entry.privilege == PRIV_ROOT)
			priv = multilang(LANG_ADMIN);
		else if (Entry.privilege == PRIV_ENG)
			priv = multilang(LANG_SUPPORT);
		else if (Entry.privilege == PRIV_USER)
			priv = multilang(LANG_USER);

		nBytesSent += boaWrite(wp, "<tr>"
		"<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"radio\" name=\"select\""
//		" value=\"s%d\" onClick=\"postEntry('%s', %d, '%s')\"></td>\n"),
//		i+2, Entry.userName, Entry.privilege, Entry.userPassword);
		" value=\"s%d\" onClick=\"postEntry('%s', %d)\"></td>\n",
		i+2, Entry.userName, Entry.privilege);
		nBytesSent += boaWrite(wp,
		"<td align=center width=\"50%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
		"<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
		Entry.userName, priv);
	}

	return nBytesSent;
}

void formAccountConfig(request * wp, char *path, char *query)
{
	char *str, *strUser, *submitUrl, *strOldPassword, *strPassword, *strConfPassword, *strPriv;
	MIB_CE_ACCOUNT_CONFIG_T entry, Entry;
	char tmpBuf[100];
	strUser = boaGetVar(wp, "username", "");
	strPriv = boaGetVar(wp, "privilege", "");
	strOldPassword = boaGetVar(wp, "oldpass", "");
	strPassword = boaGetVar(wp, "newpass", "");
	strConfPassword = boaGetVar(wp, "confpass", "");
	/* Retrieve next page URL */
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	int totalEntry, i, selected;

	// Delete
	str = boaGetVar(wp, "deluser", "");
	if (str[0]) {
		unsigned int i;
		unsigned int idx;
		totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
		str = boaGetVar(wp, "select", "");

		if (str[0]) {
			if (!strncmp(str, "s0", 2) || !strncmp(str, "s1", 2)) {
				strcpy(tmpBuf, multilang(LANG_SORRY_THE_ACCOUNT_CANNOT_BE_DELETED));
				goto setErr_user;
			}
			for (i=0; i<totalEntry; i++) {
				idx = totalEntry - i + 1;
				snprintf(tmpBuf, 4, "s%d", idx);
				if (!gstrcmp(str, tmpBuf)) {
					/* get the specified chain record */
					if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, idx - 2, (void *)&Entry)) {
						strcpy(tmpBuf, errGetEntry);
						goto setErr_user;
					}
					// delete from chain record
					if(mib_chain_delete(MIB_ACCOUNT_CONFIG_TBL, idx - 2) != 1) {
						strcpy(tmpBuf, Tdelete_chain_error);
						goto setErr_user;
					}
					goto setOk_user;
				}
			}
		}
		else {
			strcpy(tmpBuf, multilang(LANG_THERE_IS_NO_ITEM_SELECTED_TO_DELETE));
			goto setErr_user;
		}
	}

	if (!strUser[0]) {
		strcpy(tmpBuf, strUserNameempty);
		goto setErr_user;
	}
	else {
		strncpy(entry.userName, strUser, MAX_NAME_LEN-1);
		entry.userName[MAX_NAME_LEN-1] = '\0';
		//entry.userName[MAX_NAME_LEN] = '\0';
	}

	if (!strPassword[0]) {
		strcpy(tmpBuf, WARNING_EMPTY_NEW_PASSWORD);
		goto setErr_user;
	}
	else {
		strncpy(entry.userPassword, strPassword, MAX_NAME_LEN-1);
		entry.userPassword[MAX_NAME_LEN-1] = '\0';
		//entry.userPassword[MAX_NAME_LEN] = '\0';
	}

	if (!strConfPassword[0]) {
		strcpy(tmpBuf, WARNING_EMPTY_CONFIRMED_PASSWORD);
		goto setErr_user;
	}

	if (strcmp(strPassword, strConfPassword) != 0 ) {
		strcpy(tmpBuf, WARNING_UNMATCHED_PASSWORD);
		goto setErr_user;
	}

	if (strPriv[0])
		entry.privilege = strPriv[0] - '0';

	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
	// Add
	str = boaGetVar(wp, "adduser", "");
	if (str[0]) {
		int intVal;
		/* Check if user name exists */
		if (strcmp(suName, strUser) == 0 || strcmp(usName, strUser) == 0) {
			strcpy(tmpBuf, multilang(LANG_ERROR_USER_ALREADY_EXISTS));
			goto setErr_user;
		}
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry)) {
  				boaError(wp, 400, strGetChainerror);
				return;
			}

			if (strcmp(Entry.userName, strUser) == 0) {
				strcpy(tmpBuf, multilang(LANG_ERROR_USER_ALREADY_EXISTS));
				goto setErr_user;
			}
		}

		intVal = mib_chain_add(MIB_ACCOUNT_CONFIG_TBL, (unsigned char*)&entry);
		if (intVal == 0) {
			strcpy(tmpBuf, strAddChainerror);
			goto setErr_user;
		}
		else if (intVal == -1) {
			strcpy(tmpBuf, strTableFull);
			goto setErr_user;
		}
	}

	// Modify
	str = boaGetVar(wp, "modify", "");
	if (str[0]) {
		selected = -1;
		str = boaGetVar(wp, "select", "");
		if (str[0]) {
			for (i=0; i<totalEntry+2; i++) {
				snprintf(tmpBuf, 4, "s%d", i);
				if (!gstrcmp(str, tmpBuf)) {
					selected = i;
					break;
				}
			}
			if (selected >= 2) {
				if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, selected - 2, (void *)&Entry)) {
					strcpy(tmpBuf, strGetChainerror);
					goto setErr_user;
				}
				if (strcmp(Entry.userPassword, strOldPassword) != 0) {
					strcpy(tmpBuf, WARNING_WRONG_PASSWORD);
					goto setErr_user;
				}
				mib_chain_update(MIB_ACCOUNT_CONFIG_TBL, (void *)&entry, selected - 2);
			}
			else if (selected == 0) {
				if (!mib_get(MIB_SUSER_PASSWORD, (void *)tmpBuf)) {
					strcpy(tmpBuf, WARNING_GET_PASSWORD);
					goto setErr_user;
				}
				if (strcmp(tmpBuf, strOldPassword) != 0) {
					strcpy(tmpBuf, WARNING_WRONG_PASSWORD);
					goto setErr_user;
				} else if (!mib_set(MIB_SUSER_PASSWORD, (void *)strPassword)) {
					strcpy(tmpBuf, WARNING_SET_PASSWORD);
					goto setErr_user;
				}
				if (!mib_set(MIB_SUSER_NAME, (void *)strUser)) {
					strcpy(tmpBuf, Tset_mib_error);
					goto setErr_user;
				}
				mib_get(MIB_SUSER_NAME, (void *)suName);
			}
			else if (selected == 1) {
				if (!mib_get(MIB_USER_PASSWORD, (void *)tmpBuf)) {
					strcpy(tmpBuf, WARNING_GET_PASSWORD);
					goto setErr_user;
				}
				if (strcmp(tmpBuf, strOldPassword) != 0) {
					strcpy(tmpBuf, WARNING_WRONG_PASSWORD);
					goto setErr_user;
				} else if (!mib_set(MIB_USER_PASSWORD, (void *)strPassword)) {
					strcpy(tmpBuf, WARNING_SET_PASSWORD);
					goto setErr_user;
				}
				if (!mib_set(MIB_USER_NAME, (void *)strUser)) {
					strcpy(tmpBuf, Tset_mib_error);
					goto setErr_user;
				}
				mib_get(MIB_USER_NAME, (void *)usName);
			}
		}
	}

setOk_user:
#ifdef EMBED
	// for take effect on real time
	writePasswdFile();
	write_etcPassword();	// Jenny
#endif


	OK_MSG(submitUrl);
	return;

setErr_user:
	OK_MSG1(tmpBuf, submitUrl);
}
#endif

/////////////////////////////////////////////////////////////////////////////
void formPasswordSetup(request * wp, char *path, char *query)
{
	char *str, *submitUrl, *strPassword, *strOldPassword, *strConfPassword;
	char tmpBuf[100];
	char userName[MAX_NAME_LEN];

	str = boaGetVar(wp, "userMode", "");
	//strUser = boaGetVar(wp, "username", "");
	strOldPassword = boaGetVar(wp, "oldpass", "");
	strPassword = boaGetVar(wp, "newpass", "");
	strConfPassword = boaGetVar(wp, "confpass", "");

	if ( !strOldPassword[0] ) {
		strcpy(tmpBuf, WARNING_EMPTY_OLD_PASSWORD);
		goto setErr_pass;
	}

	if ( !strPassword[0] ) {
		strcpy(tmpBuf, WARNING_EMPTY_NEW_PASSWORD);
		goto setErr_pass;
	}

	if ( !strConfPassword[0] ) {
		strcpy(tmpBuf, WARNING_EMPTY_CONFIRMED_PASSWORD);
		goto setErr_pass;
	}

	if (strcmp(strPassword, strConfPassword) != 0 ) {
		strcpy(tmpBuf, WARNING_UNMATCHED_PASSWORD);
		goto setErr_pass;
	}

	if (str[0]) {
		if ( str[0] == '0' ) {       // superuser ==> cht
			if ( !mib_get(MIB_SUSER_PASSWORD, (void *)tmpBuf)) {
				strcpy(tmpBuf, WARNING_GET_PASSWORD);
				goto setErr_pass;
			}

			if ( strcmp(tmpBuf, strOldPassword) != 0 ) {
				strcpy(tmpBuf, WARNING_WRONG_PASSWORD);
				goto setErr_pass;
			}else if ( !mib_set(MIB_SUSER_PASSWORD, (void *)strPassword) ) {
				strcpy(tmpBuf, WARNING_SET_PASSWORD);
				goto setErr_pass;
			}
		}
		else if ( str[0] == '1' ) {  // normal user ==> user
			if ( !mib_get(MIB_USER_PASSWORD, (void *)tmpBuf)) {
				strcpy(tmpBuf, WARNING_GET_PASSWORD);
				goto setErr_pass;
			}

			if ( strcmp(tmpBuf, strOldPassword) != 0 ) {
				strcpy(tmpBuf, WARNING_WRONG_PASSWORD);
				goto setErr_pass;
			}else if ( !mib_set(MIB_USER_PASSWORD, (void *)strPassword) ) {
				strcpy(tmpBuf, WARNING_SET_PASSWORD);
				goto setErr_pass;
			}
		}
		else {
			strcpy(tmpBuf, WARNING_WRONG_USER);
			goto setErr_pass;
		}
	}

#ifdef EMBED
	// Added by Mason Yu for take effect on real time
	writePasswdFile();
	write_etcPassword();	// Jenny
#endif

	/*
	if (mib_update(HW_SETTING) == 0) {
		printf("Warning : Commit hs fail(formPasswordSetup()) !\n");
	}
	*/

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	/* Init user management */
	// Commented By Mason Yu
	//set_user_profile();

	/* Retrieve next page URL */
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	OK_MSG(submitUrl);
	return;

setErr_pass:
	ERR_MSG(tmpBuf);
}

// Added by Mason Yu for 2 level web page
/////////////////////////////////////////////////////////////////////////////
void formUserPasswordSetup(request * wp, char *path, char *query)
{
	char *str, *submitUrl, *strPassword, *strOldPassword, *strConfPassword;
	char tmpBuf[100];
	char userName[MAX_NAME_LEN];
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	int totalEntry, i, selected = -1;
#endif

	//str = boaGetVar(wp, "userMode", "");
	//strUser = boaGetVar(wp, "username", "");
	strOldPassword = boaGetVar(wp, "oldpass", "");
	strPassword = boaGetVar(wp, "newpass", "");
	strConfPassword = boaGetVar(wp, "confpass", "");

	if ( !strOldPassword[0] ) {
		strcpy(tmpBuf, multilang(LANG_WARNING_EMPTY_OLD_PASSWORD));
		goto setErr_pass;
	}

	if ( !strPassword[0] ) {
		strcpy(tmpBuf, multilang(LANG_WARNING_EMPTY_NEW_PASSWORD));
		goto setErr_pass;
	}

	if ( !strConfPassword[0] ) {
		strcpy(tmpBuf, multilang(LANG_WARNING_EMPTY_CONFIRMED_PASSWORD));
		goto setErr_pass;
	}

	if (strcmp(strPassword, strConfPassword) != 0 ) {
		strcpy(tmpBuf, multilang(LANG_WARNING_UNMATCHED_PASSWORD));
		goto setErr_pass;
	}


#ifdef ACCOUNT_CONFIG
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry))
			continue;
		if (Entry.privilege == (unsigned char)PRIV_ROOT)
			continue;
		#ifdef USE_LOGINWEB_OF_SERVER
		if(!strcmp(g_login_username, Entry.userName))
		#else
		if (strcmp(wp->user, Entry.userName) == 0)
		#endif
		{
			selected = i;
			break;
		}
	}
	if (selected != -1) {
		if (strcmp(Entry.userPassword, strOldPassword) != 0) {
			strcpy(tmpBuf, WARNING_WRONG_PASSWORD);
			goto setErr_pass;
		} else {
			strncpy(Entry.userPassword, strPassword, MAX_NAME_LEN-1);
			Entry.userPassword[MAX_NAME_LEN-1] = '\0';
			//Entry.userPassword[MAX_NAME_LEN] = '\0';
		}
		Entry.privilege = (unsigned char)getAccPriv(Entry.userName);
		mib_chain_update(MIB_ACCOUNT_CONFIG_TBL, (void *)&Entry, selected);
	}
	else {
#endif
	if ( !mib_get(MIB_USER_PASSWORD, (void *)tmpBuf)) {
		strcpy(tmpBuf, Tget_mib_error);
		goto setErr_pass;
	}

	if ( strcmp(tmpBuf, strOldPassword) != 0 ) {
		strcpy(tmpBuf, multilang(LANG_ERROR_INPUT_OLD_USER_PASSWORD_ERROR));
		goto setErr_pass;
	}else if ( !mib_set(MIB_USER_PASSWORD, (void *)strPassword) ) {
		strcpy(tmpBuf, Tset_mib_error);
		goto setErr_pass;
	}
#ifdef ACCOUNT_CONFIG
	}
#endif

#ifdef EMBED
	// Added by Mason Yu for take effect on real time
	writePasswdFile();
	write_etcPassword();	// Jenny
#endif

	/*
	if (mib_update(HW_SETTING) == 0) {
		printf("Warning : Commit hs fail(formPasswordSetup()) !\n");
	}
	*/

	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);

	/* Init user management */
	// Commented By Mason Yu
	//set_user_profile();

	/* Retrieve next page URL */
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	OK_MSG(submitUrl);
	return;

setErr_pass:
	ERR_MSG(tmpBuf);
}

////////////////////////////////////////////////////////////////////
void set_user_profile(void)
{
	char superName[MAX_NAME_LEN], superPass[MAX_NAME_LEN];
	char userName[MAX_NAME_LEN], userPass[MAX_NAME_LEN];
	char dport[10];
//	char *user, *nextUser, *group;

	/* first time load, get mib */
	if ( !mib_get( MIB_SUPER_NAME, (void *)superName ) ||
		!mib_get( MIB_SUSER_PASSWORD, (void *)superPass ) ||
			!mib_get( MIB_USER_NAME, (void *)userName ) ||
				!mib_get( MIB_USER_PASSWORD, (void *)userPass ) ) {
		error(E_L, E_LOG, "Get user account MIB failed");
		return;
	}

	/* Delete all user account belonging to DEFAULT_GROUP */
	/*
	user = umGetFirstUser();
	while (user) {
//		printf("boaDeleteUser (user=%s).\n", user);
		nextUser = umGetNextUser(user);
		group = umGetUserGroup(user);
		if (gstrcmp(DEFAULT_GROUP, group) == 0) {
			if ( boaDeleteUser(user) ) {
				printf("ERROR: Unable to delete user account (user=%s).\n", user);
				return;
			}
		}

		user = nextUser;
		//user = umGetFirstUser();
	}
	*/

	boaDeleteAccessLimit(ACCESS_URL);
	boaDeleteGroup(DEFAULT_GROUP);

	if ( userName[0] ) {
		/* Create supervisor */
		if ( !boaGroupExists(DEFAULT_GROUP) )
			if ( boaAddGroup(DEFAULT_GROUP, (short)PRIV_ADMIN, AM_BASIC, FALSE, FALSE) ) {
				error(E_L, E_LOG, "ERROR: Unable to add group.");
				return;
			}
		if ( !boaAccessLimitExists(ACCESS_URL) ) {
			if ( boaAddAccessLimit(ACCESS_URL, AM_FULL, (short)0, DEFAULT_GROUP) ) {
				error(E_L, E_LOG, "ERROR: Unable to add access limit.");
				return;
			}
		}

		/* Create user */
		if ( boaAddUser(superName, superPass, DEFAULT_GROUP, FALSE, FALSE) ) {
			error(E_L, E_LOG, "ERROR: Unable to add supervisor account.");
			return;
		}

		/* Create user */
		if ( boaAddUser(userName, userPass, DEFAULT_GROUP, FALSE, FALSE) ) {
			error(E_L, E_LOG, "ERROR: Unable to add user account.");
			return;
		}
	}
	else {
		if (g_remoteConfig) {	// remote config not allowed
			char ipaddr[20], tmpStr[5];

			if (g_rUserName[0]) {
				if ( boaDeleteUser(g_rUserName) ) {
					printf("ERROR: Unable to delete user account (user=%s).\n", g_rUserName);
					return;
				}
				g_rUserName[0] = '\0';
			}

			mib_get(MIB_ADSL_LAN_IP, (void *)tmpStr);
			strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpStr)), 16);
			ipaddr[15] = '\0';
			snprintf(ipaddr, 20, "%s:80", ipaddr);
			sprintf(dport, "%d", g_remoteAccessPort);
			// iptables -D INPUT -i ! br0 -p TCP --dport 80 -j ACCEPT
			va_cmd(IPTABLES, 11, 1, (char *)FW_DEL, (char *)FW_INPUT, "!", ARG_I, LANIF, "-p", ARG_TCP, FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
			// iptables -t nat -D PREROUTING -i ! $LAN_IF -p TCP --dport 51003 -j DNAT --to-destination ipaddr:80
			va_cmd(IPTABLES, 15, 1, "-t", "nat", (char *)FW_DEL,	"PREROUTING", "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP, (char *)FW_DPORT,
			dport, "-j", "DNAT", "--to-destination", ipaddr);
			g_remoteConfig = 0;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// search token szKey from string szString
// if find, return its value, else return null
char* SearchKeyValue(char* szString, char* szKey)
{
	char *szDuplicate;
	char *key, *lp, *cp, *value;

	//duplicate the string, avoid the original string to be modefied
	szDuplicate = strdup(szString);

	for (lp = szDuplicate ; lp && *lp; )
	{
		cp = lp;
		if ((lp = gstrchr(lp, ';')) != NULL)
		{
			lp++;
		}

		if ((key = gstrtok(cp, "= \t;")) == NULL)
		{
			continue;
		}

		if ((value = gstrtok(NULL, ";")) == NULL)
		{
			value = "";
		}

		while (gisspace(*value))
		{
			value++;
		}

		if(strcmp(key, szKey) == 0)
		{
			return value;
		}
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
// find key szKey form string szString
// start from nStart to nEnd of szString
// if found, return the first index of the matched string
// if not found, return -1
static int FindKeyIndex(char *szKey, char * szString, int nStart, int nEnd)
{
	int nKeyLen = strlen(szKey);
	char *szSearch = szString + nStart;
	char *szSearchEnd = szString + nEnd;
	int nIndex = 0;
	bool bFind = false;
	while(szSearch != szSearchEnd)
	{
		if(memcmp(szSearch, szKey, nKeyLen) ==0)
		{
			bFind = true;
			break;
		}
		else
		{
			nIndex++;
			szSearch++;
		}
	}

	if(bFind == true)
		return (nIndex + nStart);
	else
		return -1;

}

/*
 * Strip head and tail of http file to form a real content file.
 */
static int strip_http(const char *fname, unsigned int offset, unsigned int nLen)
{
	FILE *src_fp, *dst_fp;
	char buf[64];
	size_t count, ret;
	unsigned int len;
	
	src_fp=fopen(fname, "rb");
	if(src_fp==NULL)
		return 0;
	fseek(src_fp, offset, SEEK_SET);
	dst_fp=fopen(fname, "r+b");
	if(dst_fp==NULL) {
		fclose(src_fp);
		return 0;
	}
	
	len = nLen;
	// Move data content to head of file
	while (len > 0) {
		count = (sizeof(buf) < len) ? sizeof(buf) : len;
		ret = fread(buf, 1, count, src_fp);

		count = ret;
		ret = fwrite(buf, 1, count, dst_fp);

		len -= ret;
	}
	
	fclose(src_fp);
	fclose(dst_fp);
	// shrink the size of file to content size
	truncate(fname, nLen);
	return 1;
}

#ifdef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
static int isValidImageFile(const char *fname)
{
	int ret;
	char buf[256];
	
	// todo: validate the image file
	snprintf(buf, sizeof(buf), "/bin/tar tf %s md5.txt > /dev/null", fname);
	ret = system(buf);

	return !ret;
}
#else
static int isValidImageFile(const char *fname) {
	IMGHDR imgHdr;
	unsigned int csum;
	int size, remain, nRead, block;
	unsigned char buf[64];
	FILE *fp;
#ifdef CONFIG_RTL8686
	int err=-1;
#endif
#ifdef ENABLE_SIGNATURE
	SIGHDR sigHdr;
	unsigned int hdrChksum;
	int i;
#endif
	/*ql: 20080729 START: check image key according to IC version*/
#ifdef MULTI_IC_SUPPORT
	unsigned int key;
#endif
	/*ql: 20080729 END*/

	fp=fopen(fname, "rb");
	if(fp==NULL)
		goto ERROR1;
#if defined(ENABLE_SIGNATURE)
	//ql_xu add: check if the img signature is right
	memset(&sigHdr, 0, sizeof(SIGHDR));
	if (1 != fread(&sigHdr, sizeof(sigHdr), 1, fp)) {
		printf("failed to read signature header\n");
		goto ERROR1;
	}
#endif
	if (1!=fread(&imgHdr, sizeof(imgHdr), 1, fp)) {
		printf("Failed to read header\n");
		goto ERROR1;
	}
#ifndef ENABLE_SIGNATURE_ADV
#ifdef ENABLE_SIGNATURE
	printf("sig len: %d\n", sigHdr.sigLen);
	if (sigHdr.sigLen > SIG_LEN) {
		printf("signature length error\n");
		goto ERROR1;
	}
	for (i=0; i<sigHdr.sigLen; i++)
		sigHdr.sigStr[i] = sigHdr.sigStr[i] - 10;
	if (strcmp(sigHdr.sigStr, SIGNATURE)) {
		printf("signature error\n");
		goto ERROR1;
	}

	hdrChksum = sigHdr.chksum;
	hdrChksum = ipchksum(&imgHdr, sizeof(imgHdr), hdrChksum);
	if (hdrChksum) {
		printf("Checksum failed(fmmgmt isValidImageFile), size=%d, csum=%04xh\n", sigHdr.sigLen, hdrChksum);
		goto ERROR1;
	}
#endif
#endif

	/*ql: 20080729 START: get sachem version, determine the IC version and then get correct img key.*/
#ifdef MULTI_IC_SUPPORT
	key = getImgKey();

	if ((key != (imgHdr.key & key)) || (((imgHdr.key>>28)&0xf) != ((key>>28)&0xf))) {
		printf("Unknown header\n");
		goto ERROR1;
	}
#else
#ifdef CONFIG_RTL8686
	switch(imgHdr.key){
		case APPLICATION_UBOOT:
		case APPLICATION_UIMAGE:
		case APPLICATION_ROOTFS:
			printf("%s-%d, got header::%x\n",__func__,__LINE__,imgHdr.key);
			err = 0;
			break;
		default:
			printf("%s-%d, Unknown header::%x\n",__func__,__LINE__,imgHdr.key);
			err = 1;
			break;
	}
	if(err)
		goto ERROR1;
#else
	if (imgHdr.key != APPLICATION_IMAGE) {
		printf("Unknown header\n");
		goto ERROR1;
	}
#endif
#endif
	/*ql: 20080729 END*/

	csum = imgHdr.chksum;
	size = imgHdr.length;
	remain = size;

	while (remain > 0) {
		block = (remain > sizeof(buf)) ? sizeof(buf) : remain;
		nRead = fread(buf, 1, block, fp);
		if (nRead <= 0) {
			printf("read too short (remain=%d, block=%d)\n", remain, block);
			goto ERROR1;
		}
		remain -= nRead;
		csum = ipchksum(buf, nRead,csum);
	}

	if (csum) {
		printf("Checksum failed(fmmgmt isValidImageFIle2), size=%d, csum=%04xh\n", size, csum);
		goto ERROR1;
	}

	return 1;
ERROR1:
	return 0;
}
#endif

// find the start and end of the upload file.
FILE * _uploadGet(request *wp, unsigned int *startPos, unsigned *endPos) {

	FILE *fp=NULL;
	struct stat statbuf;
	unsigned char c, *buf;

	if (wp->method == M_POST)
	{
		fstat(wp->post_data_fd, &statbuf);
		lseek(wp->post_data_fd, SEEK_SET, 0);

		//printf("file size=%d\n",statbuf.st_size);
		fp=fopen(wp->post_file_name,"rb");
		if(fp==NULL) goto error;
	}
	else goto error;

	do
	{
		if(feof(fp))
		{
			printf("Cannot find start of file\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		c= fgetc(fp);
		if (c!=0xd)
			continue;
		c= fgetc(fp);
		if (c!=0xa)
			continue;
		break;
	}while(1);
	(*startPos)=ftell(fp);

	do
	{
		if(feof(fp))
		{
			printf("Cannot find the end of the file!\n");
			goto error;
		}
		c= fgetc(fp);
		if (c!='-')
			continue;
		c= fgetc(fp);
		if (c!='-')
			continue;
		c= fgetc(fp);
		if (c!='-')
			continue;
		c= fgetc(fp);
		if (c!='-')
			continue;
		break;
	}while(1);
	(*endPos)=ftell(fp);
	*endPos -= 6;  // Magician

	return fp;
error:
	return NULL;
}

#ifdef WEB_UPGRADE
// Added by Mason Yu
void displayUploadMessage(request * wp, int status)
{
	//printf("Popout web page\n");
	boaHeader(wp);
	boaWrite(wp, "<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n");
	boaWrite(wp, "<body><blockquote><h4>\n");
	boaWrite(wp, "Upgrade Firmware failed ! ");
	switch (status) {
		case ERROR_FILESIZE:
			boaWrite(wp, "(file size exceeded)");
			break;
		case ERROR_FORMAT:
			boaWrite(wp, "(file format error)");
			break;
		case ERROR_INVALID:
		default:
			boaWrite(wp, "(Invalid file)");
			break;
	}
	boaWrite(wp, "</h4>\n");
	boaWrite(wp, "%s<br><br>\n", rebootWord0);
	boaWrite(wp, "%s\n", rebootWord2);
	boaWrite(wp, "</blockquote></body>");
	boaFooter(wp);
	boaDone(wp, 200);
}

#ifdef UPGRADE_V1
///////////////////////////////////////////////////////////////////////////////
void formUpload(request * wp, char *path, char *query)
{
	unsigned int startPos, endPos, nLen;
	char tmpBuf[100] /*, *submitUrl */ ;
	FILE *fp = NULL;

	/* find the start and end positive of run time image */
	tmpBuf[0] = '\0';
	//printf("\nTry to get file size of new firmware\n");

#ifdef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT	
	/* Toggle the LED for firmupgarde */
	system("echo 3 > /proc/power_flag");
#endif

#ifdef ENABLE_SIGNATURE_ADV
	if (upgrade != 2) {	//signature Err
		displayUploadMessage(wp, ERROR_INVALID);
		goto end;
	}
#endif

	if (g_filesize >= g_max_upload_size) {
		displayUploadMessage(wp, ERROR_FILESIZE);
		goto end;
	}

	if ((fp = _uploadGet(wp, &startPos, &endPos)) == NULL) {
		strcpy(tmpBuf, FILEOPENFAILED);
		fclose(fp);
		goto fail;
	}
	fclose(fp);

	/* check header and checksum of this image */
	printf("endPos=%u startPos=%u\n", endPos, startPos);
	nLen = endPos - startPos;

#ifdef EMBED
	// write to flash
	{
		int writeflashtime;

		strip_http(wp->post_file_name, startPos, nLen);
		if (!isValidImageFile(wp->post_file_name)) {
			printf("Incorrect image file\n");
			displayUploadMessage(wp, ERROR_FORMAT);
			goto end;
		}
		// Save file for upgrade Firmware
		g_upgrade_firmware = TRUE;
		cmd_upload(wp->post_file_name, 0, nLen);

#ifdef CONFIG_DEV_xDSL
		Modem_LinkSpeed vLs;
		vLs.upstreamRate = 0;

		if (adsl_drv_get
		    (RLCM_GET_LINK_SPEED, (void *)&vLs,
		     RLCM_GET_LINK_SPEED_SIZE) && vLs.upstreamRate != 0)
			writeflashtime = g_filesize / 17400 / 3;	//star: flash can wirte about 17k in 1 sec with the adsl line up
		else
#endif
			writeflashtime = g_filesize / 21000 / 3;	//star: flash can wirte about 21k in 1 sec
#ifdef CONFIG_LUNA_FIRMWARE_UPGRADE_SUPPORT
		writeflashtime += 10;
#endif
		// Added by Mason Yu
		boaWrite(wp, "<html><head>\n"
			 "<style>#cntdwn{ border-color: white;border-width: 0px;font-size: 12pt;color: red;text-align:center; font-weight:bold; font-family: Courier;}\n"
			 "body { background-repeat: no-repeat; background-attachment: fixed;	 background-position: center;	text-align:left;}\n"
			 "div.msg { font-size: 20px; top: 100px; width: 470px; text-align:center;}\n"
			 "#progress-boader{ border: 2px #ccc solid;border-radius: 10px;height: 25; top: 10px; width :420px; text-align:center ;left: 30px}\n"
			 "</style>\n"
			 "<script language=javascript src=\"/nprogress.js\"></script>\n"
			 "<link rel=stylesheet href=\"/style/nprogress.css\">\n"
			 "<script language=javascript>\n"
			 "NProgress.configure({ minimum: 0.01, trickle: false, showSpinner: false, parent: '#progress-boader' });\n"
			 "var h=total=(%d+10), index=0, percent=0;\n"
			 "function stop() { clearTimeout(id); NProgress.done(); }\n"
			 "function start() { h--; index ++; percent=(Math.round((index/total)*100))/100;\n"
			 "if (h >= 40) {	frm.textname.value='%s'; id=setTimeout(\"start()\",1000); NProgress.set(percent);}\n"
			 "if (h >= 0 && h < 40) {  frm.textname.value='%s'; id=setTimeout(\"start()\",1000); NProgress.set(percent);}\n"
			 "if (h == 0) { window.open(\"/status.asp\",target=\"view\"); }}\n"
			 "</script></head>\n"
			 "<div class=msg id=\"progress-boader\"></div>\n"
			 "<body bgcolor=white  onLoad=\"start();\" onUnload=\"stop();\">"
			 "<blockquote><form action=/boaform/formStopUpload method=post name=frm><b><font color=red>\n"
			 "<input type=text name=textname size=40 id=\"cntdwn\">\n"
#ifdef CONFIG_DOUBLE_IMAGE
			 "<input type=submit name=cancel value=\"Cancel and Reboot\">\n"
#endif
			 "</font></b></form>\n"
			 "<h4>%s</h4>\n"
			 "</blockquote></body></html>", writeflashtime,multilang(LANG_FIRMWARE_UPGRADING_PLEASE_WAIT),
			 multilang(LANG_SYSTEM_RESTARTING_PLEASE_WAIT),multilang(LANG_PAGE_DESC_UPGRADE_NOTICE));

	}
	return;
#endif


fail:
	OK_MSG1(tmpBuf, "/upgrade.asp");
end:
#ifdef EMBED
	cmd_reboot();
#endif

	return;
}
#endif // of UPGRADE_V1
#ifdef CONFIG_DOUBLE_IMAGE
void formStopUpload(request * wp, char * path, char * query)
{
	formReboot(wp, path, query);
	cmd_upload(NULL, 0, 0);  //stop fw_upload
}
#endif
#endif // of WEB_UPGRADE

///////////////////////////////////////////////////////////////////////////////
/*
 *	Tag: load, Value: Upload - upload configuration file
 *	Tag: save, Value: Save... - save configuration file
 *	Tag: reset, Value: Rest - reset configuration to default
 */

void formSaveConfig(request * wp, char *path, char *query)
{
	char *strRequest;
	unsigned int maxFileSector;

	char tmpBuf[100], *submitUrl;

	CONFIG_DATA_T action_type = UNKNOWN_SETTING;

	wp->buffer_end=0; // clear header
   	tmpBuf[0] = '\0';

	if (g_filesize > MIN_UPLOAD_FILESIZE) {
		boaHeader(wp);
		boaWrite(wp, "<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n");
		boaWrite(wp, "<body><blockquote><h4>\n");
		boaWrite(wp, "Restore settings from config file failed! Uploaded file size out of constraint!<br>");
		boaWrite(wp, "%s</h4>\n", rebootWord0);
		boaWrite(wp, "<br>%s\n", rebootWord2);
		boaWrite(wp, "</blockquote></body>");
		boaFooter(wp);
		boaDone(wp, 200);
		goto end;
	}
	else if (g_filesize >= MAX_CONFIG_FILESIZE) {
		strcpy(tmpBuf, multilang(LANG_ERROR_INPUT_OLD_USER_PASSWORD_ERROR));
		goto fail;
	}

	strRequest = boaGetVar(wp, "save_ds", "");
	if (strRequest[0])
	{
		action_type = DEFAULT_SETTING;
		maxFileSector = DEFAULT_SETTING_MAX_LEN/10;
	}

	strRequest = boaGetVar(wp, "save_cs", "");
	if (strRequest[0])
	{
		action_type = CURRENT_SETTING;
		maxFileSector = CURRENT_SETTING_MAX_LEN/10;
	}

	strRequest = boaGetVar(wp, "save_hs", "");
	if (strRequest[0])
	{
		action_type = HW_SETTING;
		maxFileSector = HW_SETTING_MAX_LEN/10;
	}

	if (action_type != UNKNOWN_SETTING)
	{	// save configuration file
		PARAM_HEADER_T header;
		unsigned char *ptr;
		unsigned int fileSize,filelen;
		unsigned int fileSector;
		unsigned char *buf;
#ifdef CONFIG_USER_XMLCONFIG
		FILE *fp;
		int ret=-1;

		//create configuration file for backup
		printf("formSaveConfig(Backup): Backup configuration file... \n");
		ret = va_cmd(shell_name, 3, 1, "/etc/scripts/config_xmlconfig.sh", "-s", "/tmp/config.xml");
#else

#ifdef CONFIG_USE_XML
		FILE *fp;
		int ret=-1;
		//create config file
		//ret = call_cmd("/bin/CreatexmlConfig", 0, 1);
		ret = call_cmd("/bin/saveconfig", 0, 1);
	#ifdef XOR_ENCRYPT
		xor_encrypt("/tmp/config.xml", "/tmp/config_xor.xml");
	#endif
#else
		if(mib_read_header(action_type, &header) != 1)
		{
			error(E_L, E_LOG, "ERROR: Flash read fail");
			strcpy(tmpBuf, multilang(LANG_ERROR_FLASH_READ_FAIL));
			goto fail;
		}

		fileSize = sizeof(PARAM_HEADER_T) + header.len;
		buf = malloc(fileSize);
		if ( buf == NULL ) {
			strcpy(tmpBuf, multilang(LANG_ALLOCATE_BUFFER_FAILED));
			goto fail;
		}

		printf("fileSize=%d\n",fileSize);
		if(mib_read_to_raw(action_type, buf, fileSize) != 1)
		{
			free(buf);

			error(E_L, E_LOG, "ERROR: Flash read fail");
			strcpy(tmpBuf, multilang(LANG_ERROR_FLASH_READ_FAIL));
			goto fail;
		}
		ENCODE_DATA(buf + sizeof(PARAM_HEADER_T), header.len);
#endif

#endif	/*CONFIG_USER_XMLCONFIG*/

		boaWrite(wp, "HTTP/1.0 200 OK\n");
		boaWrite(wp, "Content-Type: application/octet-stream;\n");

		if(action_type == CURRENT_SETTING)
#if defined(CONFIG_USE_XML) || defined(CONFIG_USER_XMLCONFIG)
		boaWrite(wp, "Content-Disposition: attachment;filename=\"config.xml\" \n");
#else
		boaWrite(wp, "Content-Disposition: attachment;filename=\"adsl-config.bin\" \n");
#endif
		else if(action_type == DEFAULT_SETTING)
		boaWrite(wp, "Content-Disposition: attachment;filename=\"adsl-config-ds.bin\" \n");
		else if(action_type == HW_SETTING)
		boaWrite(wp, "Content-Disposition: attachment;filename=\"adsl-config-hs.bin\" \n");
#ifdef 	SERVER_SSL
		// IE bug, we can't sent file with no-cache through https
#else
		boaWrite(wp, "Pragma: no-cache\n");
		boaWrite(wp, "Cache-Control: no-cache\n");
#endif
		boaWrite(wp, "\n");

#if defined(CONFIG_USE_XML) || defined(CONFIG_USER_XMLCONFIG)
	#ifdef XOR_ENCRYPT
		fp=fopen("/tmp/config_xor.xml","r");
	#else
		fp=fopen("/tmp/config.xml","r");
	#endif
		//decide the file size
		fseek(fp, 0, SEEK_END);
		filelen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fileSize=filelen;
		maxFileSector=0x1000;
		buf = malloc(maxFileSector);
		if ( buf == NULL ) {
			strcpy(tmpBuf, multilang(LANG_ALLOCATE_BUFFER_FAILED));
			goto fail;
		}
		while(fileSize>0)
		{
			int nRead;

//			printf("write %d %d\n",maxFileSector, fileSize);
			fileSector = (fileSize>maxFileSector)?maxFileSector:fileSize;
			nRead = fread((void *)buf, 1, fileSector, fp);
			boaWriteDataNonBlock(wp, buf, nRead);

			fileSize -= fileSector;
		}
		free(buf);
		fclose(fp);
#else
		ptr = buf;
		while(fileSize>0)
		{
//			printf("write %d %d %08x\n",maxFileSector, fileSize, (unsigned int )ptr);
			fileSector = (fileSize>maxFileSector)?maxFileSector:fileSize;
			boaWriteDataNonBlock(wp, ptr, fileSector);

			fileSize -= fileSector;
			ptr += fileSector;
 			sleep(1);
		}
		free(buf);
#endif
		//boaDone(wp, 200);
		//OK_MSG("/saveconf.asp");
		return;
	}

	strRequest = boaGetVar(wp, "reset", "");
	printf("%s\n", strRequest);
	if (strRequest[0])
	{	// reset configuration to default
		char *submitUrl=0;

		//submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

		//OK_MSG(submitUrl);
		// Modified by Mason Yu. for not use default setting
		/*
		mib_load(DEFAULT_SETTING, CONFIG_MIB_ALL);
		formReboot(wp, NULL, NULL);
		*/

		// Jenny, add reboot messages when reset to default
		boaHeader(wp);
		boaWrite(wp, "<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n");
		boaWrite(wp, "<body><blockquote><h4>\n");
		boaWrite(wp, "%s</h4>\n", rebootWord0);
		boaWrite(wp, "%s<br><br>\n", rebootWord1);
		boaWrite(wp, "%s\n", rebootWord2);
		boaWrite(wp, "</blockquote></body>");
		boaFooter(wp);
		boaDone(wp, 200);

#ifdef EMBED
		//Mason Yu,  LED flash while factory reset
		system("echo 2 > /proc/load_default");
		reset_cs_to_default(1);
		cmd_reboot();
#endif
		return;
	}

#ifdef CONFIG_USER_XMLCONFIG
	{
		FILE	*fp=NULL,*fp_input;
		unsigned char *buf;
		unsigned int startPos,endPos,nLen,nRead;
		int ret=-1;

		if ((fp = _uploadGet(wp, &startPos, &endPos)) == NULL) {
			strcpy(tmpBuf, multilang(LANG_ERROR_FIND_THE_START_AND_END_OF_THE_UPLOAD_FILE_FAILED));
			goto fail;
		}

		/* check header and checksum of this image */
		nLen = endPos - startPos;
		printf("Config file size is %d\n", nLen);
		buf = malloc(nLen);
		if (!buf) {
			fclose(fp);
			goto end;
		}

		fseek(fp, startPos, SEEK_SET);
		nRead = fread((void *)buf, 1, nLen, fp);
		fclose(fp);
		if (nRead != nLen)
			printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

		fp_input=fopen("/tmp/config.xml","w");
		if (!fp_input)
			printf("Get config file fail!\n");

		fwrite((void *)buf, 1, nLen, fp_input);
		printf("create file config.xml\n");
		free(buf);
		fclose(fp_input);

		if(va_cmd(shell_name, 3, 1, "/etc/scripts/config_xmlconfig.sh", "-c", "/tmp/config.xml"))
		{
			boaHeader(wp);
			boaWrite(wp, "<body><blockquote>\n");
			boaWrite(wp, "<h3>Invalid config file! \n</h3>");
			boaWrite(wp, "</blockquote></body>");
			boaFooter(wp);
			boaDone(wp, 200);
			unlink("/tmp/config.xml");
			goto end;
		}

		printf("formSaveConfig(Restore): Upload the configuration file. \n");
		ret = va_cmd(shell_name,3,1, "/etc/scripts/config_xmlconfig.sh", "-l", "/tmp/config.xml");

		if (ret == 0) {
			boaHeader(wp);
			boaWrite(wp, "<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n");
			boaWrite(wp, "<body><blockquote><h4>\n");
			boaWrite(wp, "Restore settings from config file successful! \n<br>");
			boaWrite(wp, "%s</h4>\n", rebootWord0);
			boaWrite(wp, "%s<br><br>\n", rebootWord1);
			boaWrite(wp, "%s\n", rebootWord2);
			boaWrite(wp, "</blockquote></body>");
			boaFooter(wp);
			boaDone(wp, 200);
			cmd_reboot();
		}
		else {
			strcpy(tmpBuf, multilang(LANG_ERROR_RESTORE_CONFIG_FILE_FAILED_INVALID_CONFIG_FILE));
			goto fail;
		}
	}
#else	/*CONFIG_USER_XMLCONFIG*/

#ifdef CONFIG_USE_XML
	{
		FILE	*fp=NULL,*fp_input;
		unsigned char *buf;
		unsigned int startPos,endPos,nLen,nRead;
		int ret=-1;

		if ((fp = _uploadGet(wp, &startPos, &endPos)) == NULL) {
			strcpy(tmpBuf, multilang(LANG_ERROR_FIND_THE_START_AND_END_OF_THE_UPLOAD_FILE_FAILED));
			goto fail;
		}

		/* check header and checksum of this image */
		//printf("endPos=%u startPos=%u\n",endPos,startPos);
		nLen = endPos - startPos;
		printf("Config file size is %d\n", nLen);
		buf = malloc(nLen);
		if (!buf) {
			fclose(fp);
			goto end;
		}

		fseek(fp, startPos, SEEK_SET);
		nRead = fread((void *)buf, 1, nLen, fp);
		fclose(fp);
		if (nRead != nLen)
			printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

	#ifdef XOR_ENCRYPT
		fp_input=fopen("/tmp/config_xor.xml","w");
	#else
		fp_input=fopen("/tmp/config.xml","w");
	#endif
		if (!fp_input)
			printf("Get config file fail!\n");
	#ifdef XOR_ENCRYPT
		fwrite((void *)buf, 1, nLen, fp_input);
	#else
		fwrite((void *)buf, 1, nLen, fp_input);
	#endif
		printf("create file config.xml\n");
		free(buf);
		fclose(fp_input);

	// Magician: Check the content of config file if it is valid.
	#ifdef XOR_ENCRYPT
	if(!checkConfigFile("/tmp/config_xor.xml"))
	{
		boaHeader(wp);
		boaWrite(wp, "<body><blockquote>\n");
		boaWrite(wp, "<h3>Invalid config file! \n</h3>");
		boaWrite(wp, "</blockquote></body>");
		boaFooter(wp);
		boaDone(wp, 200);
		unlink("/tmp/config_xor.xml");
		goto end;
	}
	#else
	if(!checkConfigFile("/tmp/config.xml"))
	{
		boaHeader(wp);
		boaWrite(wp, "<body><blockquote>\n");
		boaWrite(wp, "<h3>Invalid config file! \n</h3>");
		boaWrite(wp, "</blockquote></body>");
		boaFooter(wp);
		boaDone(wp, 200);
		unlink("/tmp/config.xml");
		goto end;
	}
	#endif
	// Magician: End

	#ifdef XOR_ENCRYPT
		xor_encrypt("/tmp/config_xor.xml", "/tmp/config.xml");
		unlink("/tmp/config_xor.xml");
	#endif
		//ret = call_cmd("/bin/LoadxmlConfig", 0, 1);
		ret = call_cmd("/bin/loadconfig", 0, 1);

		if (ret == 0) {
			boaHeader(wp);
			boaWrite(wp, "<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n");
			boaWrite(wp, "<body><blockquote><h4>\n");
			boaWrite(wp, "Restore settings from config file successful! \n<br>");
			boaWrite(wp, "%s</h4>\n", rebootWord0);
			boaWrite(wp, "%s<br><br>\n", rebootWord1);
			boaWrite(wp, "%s\n", rebootWord2);
			boaWrite(wp, "</blockquote></body>");
			boaFooter(wp);
			boaDone(wp, 200);
			mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
			cmd_reboot();
		}
		else {
			strcpy(tmpBuf, multilang(LANG_ERROR_RESTORE_CONFIG_FILE_FAILED_INVALID_CONFIG_FILE));
			goto fail;
		}
	}
#else
	{
		unsigned int startPos,endPos,nLen,nRead;
		FILE	*fp=NULL;
		//struct stat statbuf;
		unsigned char *buf;

		/* find the start and end positive of run time image */
		printf("\nTry to get file size of new firmware\n");

		if ((fp = _uploadGet(wp, &startPos, &endPos)) == NULL)
			goto end;

		/* check header and checksum of this image */
		printf("endPos=%u startPos=%u\n",endPos,startPos);

		nLen = endPos - startPos;
		//printf("filesize is %d\n", nLen);
		buf = malloc(nLen);
		if (!buf)
			goto end;

		fseek(fp, startPos, SEEK_SET);
		nRead = fread((void *)buf, 1, nLen, fp);
		if (nRead != nLen)
			printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

		printf("write to %d bytes from %08x\n", nLen, buf);
		DECODE_DATA(buf + sizeof(PARAM_HEADER_T), nLen - sizeof(PARAM_HEADER_T));
		if(mib_update_from_raw(buf, nLen) != 1)
		{
			strcpy(tmpBuf, multilang(LANG_FLASH_ERROR));
			free(buf);
			goto end;
		}
		//submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
		//printf("submitted val: %s\n", submitUrl);
		// Mason Yu. Auto Reboot for raw data config
		//OK_MSG("/saveconf.asp");
		free(buf);
		// Mason Yu. Auto Reboot for raw data config
		boaHeader(wp);
		boaWrite(wp, "<META HTTP-EQUIV=Refresh CONTENT=\"60; URL=/status.asp\">\n");
		boaWrite(wp, "<body><blockquote><h4>\n");
		boaWrite(wp, "Restore settings from config file successful! \n<br>");
		boaWrite(wp, "%s</h4>\n", rebootWord0);
		boaWrite(wp, "%s<br><br>\n", rebootWord1);
		boaWrite(wp, "%s\n", rebootWord2);
		boaWrite(wp, "</blockquote></body>");
		boaFooter(wp);
		boaDone(wp, 200);
		//mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
		cmd_reboot();
	}
#endif

#endif	/*CONFIG_USER_XMLCONFIG*/

	return;

fail:
	OK_MSG1(tmpBuf, "/admin/saveconf.asp");
end:
	cmd_reboot();
 	return;
}

void formImportOMCIShell(request * wp, char *path, char *query)
{
	char *strRequest;
	unsigned int maxFileSector;
	char tmpBuf[100], *submitUrl;
	struct stat statbuf;

	wp->buffer_end=0; // clear header
   	tmpBuf[0] = '\0';
	FILE	*fp=NULL,*fp_input;
	unsigned char *buf, c;
	unsigned int startPos,endPos,nLen,nRead;
	int ret=-1;
	
	if (wp->method == M_POST)
	{
		fstat(wp->post_data_fd, &statbuf);
		lseek(wp->post_data_fd, SEEK_SET, 0);
		fp=fopen(wp->post_file_name,"rb");
		if(fp==NULL) goto fail;
	}
	else goto fail;
	
	do
	{
		if(feof(fp))
		{
			printf("Cannot find start of file\n");
			goto fail;
		}
		c= fgetc(fp);
		if (c!=0xd) continue;
		c= fgetc(fp);
		if (c!=0xa) continue;
		c= fgetc(fp);
		if (c!=0xd) continue;
		c= fgetc(fp);
		if (c!=0xa) continue;
		break;
	}while(1);
	startPos=ftell(fp);
	if(fseek(fp,statbuf.st_size-0x100,SEEK_SET)<0)
		goto fail;
	do
	{
		if(feof(fp))
		{
			printf("Cannot find the end of the file!\n");
			goto fail;
		}
		c= fgetc(fp);
		if (c!='-') continue;
		c= fgetc(fp);
		if (c!='-') continue;
		c= fgetc(fp);
		if (c!='-') continue;
		c= fgetc(fp);
		if (c!='-') continue;
		break;
	}while(1);
	endPos=ftell(fp);
	endPos -= 6;  // Magician
	
	nLen = endPos - startPos;
	buf = malloc(nLen);
	if (!buf)
	{
		fclose(fp);
		goto end;
	}

	fseek(fp, startPos, SEEK_SET);
	nRead = fread((void *)buf, 1, nLen, fp);
	fclose(fp);
	if (nRead != nLen)
		printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

	fp_input=fopen("/tmp/omcishell","w");
	if (!fp_input)
		printf("Get config file fail!\n");

	fwrite((void *)buf, 1, nLen, fp_input);
	printf("create file omcishell\n");
	free(buf);
	fclose(fp_input);
	system("/bin/sh /tmp/omcishell");
	strcpy(tmpBuf, multilang(LANG_OK));
	OK_MSG1(tmpBuf, "/vermod.asp");
	return;
fail:
	OK_MSG1(tmpBuf, "/vermod.asp");
end:
	
 	return;
}


void formExportOMCIlog(request * wp, char *path, char *query)
{
	char *strRequest;

	char tmpBuf[100], *submitUrl;
	PARAM_HEADER_T header;
	unsigned int maxFileSector;
	FILE *fp;

	unsigned char *ptr;
	unsigned int fileSize,filelen;
	unsigned int fileSector;
	unsigned char *buf;
	int ret;


	wp->buffer_end=0; // clear header
   	tmpBuf[0] = '\0';

	system("/bin/diag gpon deactivate");
	sleep(1);
	system("/bin/omcicli mib_reset");
	sleep(1);
	system("/bin/omcicli set logfile 3 ffffffff");
	system("/bin/diag gpon activate init-state o1");

	ret = sleep(60);
	do
	{
		ret = sleep(ret);	
	}while(ret > 0);
	
	system("/bin/omcicli set logfile 0");  
	system("/bin/tar -cf /tmp/omcilog.tar /tmp/omcilog /tmp/omcilog.par");

	ret=-1;
			
	boaWrite(wp, "HTTP/1.0 200 OK\n");
	boaWrite(wp, "Content-Type: application/octet-stream;\n");
		
	boaWrite(wp, "Content-Disposition: attachment;filename=\"omcilog.tar\" \n");
#ifdef 	SERVER_SSL
	// IE bug, we can't sent file with no-cache through https
#else
	boaWrite(wp, "Pragma: no-cache\n");
	boaWrite(wp, "Cache-Control: no-cache\n");
#endif
	boaWrite(wp, "\n");
		
	
	fp=fopen("/tmp/omcilog.tar","r");
	
	//decide the file size
	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fileSize=filelen;
	buf = malloc(0x1000);
	if ( buf == NULL ) {
		strcpy(tmpBuf, multilang(LANG_ALLOCATE_BUFFER_FAILED));
		return;
	}
	while(fileSize>0)
	{
		int nRead;
		fileSector = (fileSize>maxFileSector)?maxFileSector:fileSize;
		nRead = fread((void *)buf, 1, fileSector, fp);
		boaWriteDataNonBlock(wp, buf, nRead);

		fileSize -= fileSector;
		ptr += fileSector;
	}
			
	free(buf);
	fclose(fp);
 	return;
}



#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
void formSnmpConfig(request * wp, char *path, char *query)
{
	char *str, *submitUrl;
	struct in_addr trap_ip;
	static char tmpBuf[100];

//star: for take effect
	unsigned int snmpchangeflag=0;
	unsigned char vChar;
	char origstr[128];
	unsigned char origip[16];
	unsigned char snmpVal, oid_snmpVal;

	// Enable/Disable SNMPD
	str = boaGetVar(wp, "save", "");
	if (str[0]) {
		str = boaGetVar(wp, "snmp_enable", "");
		if (str[0]) {
			if (str[0] == '0')
				snmpVal = 0;
			else
				snmpVal = 1;

			mib_get(MIB_SNMPD_ENABLE, (void *)&oid_snmpVal);
			if ( oid_snmpVal != snmpVal ) {
				snmpchangeflag = 1;
			}

			if ( !mib_set(MIB_SNMPD_ENABLE, (void *)&snmpVal)) {
				strcpy(tmpBuf, Tset_mib_error);
				goto setErr_pass;
			}
		}
 	}

	str = boaGetVar(wp, "snmpSysDescr", "");
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_DESCR, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_DESCR, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_pass;
		}
	}

	str = boaGetVar(wp, "snmpSysContact", "");
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_CONTACT, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_CONTACT, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_pass;
		}
	}

	str = boaGetVar(wp, "snmpSysName", "");
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_NAME, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_NAME, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_pass;
		}
	}

	str = boaGetVar(wp, "snmpSysLocation", "");
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_LOCATION, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_LOCATION, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_pass;
		}
	}

	str = boaGetVar(wp, "snmpSysObjectID", "");
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_OID, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_OID, (void *)str)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_pass;
		}
	}

	str = boaGetVar(wp, "snmpCommunityRO", "");
	if (str[0]) {
		mib_get(MIB_SNMP_COMM_RO, (void*)origstr);

		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_COMM_RO, (void *)str)) {
			strcpy(tmpBuf, strSetcommunityROerror);
			goto setErr_pass;
		}
	}
	str = boaGetVar(wp, "snmpCommunityRW", "");
	if (str[0]) {
		mib_get(MIB_SNMP_COMM_RW, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_COMM_RW, (void *)str)) {
			strcpy(tmpBuf, strSetcommunityRWerror);
			goto setErr_pass;
		}
	}
	str = boaGetVar(wp, "snmpTrapIpAddr", "");
	if ( str[0] ) {
		if ( !inet_aton(str, &trap_ip) ) {
			strcpy(tmpBuf, strInvalTrapIp);
			goto setErr_pass;
		}
		mib_get(MIB_SNMP_TRAP_IP, (void*)&origip);
		if(((struct in_addr*)origip)->s_addr != trap_ip.s_addr)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_TRAP_IP, (void *)&trap_ip)) {
			strcpy(tmpBuf, strSetTrapIperror);
			goto setErr_pass;
		}
	}

	/* upgdate to flash */
//	mib_update(CURRENT_SETTING);
	if(snmpchangeflag == 1)
	{
// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
		Commit();
#endif

//		RECONNECT_MSG(strIp);
//		req_flush(wp);
#if defined(APPLY_CHANGE)
		if ( snmpVal == 1 )	// on
			restart_snmp(1);
		else
			restart_snmp(0);  // off
#endif
		submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
		OK_MSG(submitUrl);
		return;
	}

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;

 setErr_pass:
	ERR_MSG(tmpBuf);
}
#endif

#ifdef CONFIG_DEV_xDSL
void formSetAdslTone(request * wp, char *path, char *query)
{
	char *submitUrl;
	short mode;
	int i, chan;
	char tmpBuf[100];
	char *strVal;
	unsigned char tone[64];	  // Added by Mason Yu for correct Tone Mib Type
	char *strApply, *strMaskAll, *strUnmaskAll;

	memset(tone, 0, sizeof(tone));

	strApply = boaGetVar(wp, "apply", "");
	strMaskAll = boaGetVar(wp, "maskAll", "");
	strUnmaskAll = boaGetVar(wp, "unmaskAll", "");


	// get the channel number
	mib_get(MIB_ADSL_MODE, (void *)&mode);
	if (mode & ADSL_MODE_ADSL2P)
		chan = 512;	// ADSL2+
	else
		chan = 256;	// ADSL, ADSL2

	if (strApply[0]) {
		for (i=0; i<chan; i++) {
			snprintf(tmpBuf, 20, "select%d", i);
			strVal = boaGetVar(wp, tmpBuf, "");

				if ( !gstrcmp(strVal, "ON") ) {
					//tone[i/8] = tone[i/8] | (1 << (i%8));
					tone[i/8] = (tone[i/8] << 1) | 1 ;
				}else {
					//tone[i/8] = tone[i/8] | (0 << (i%8));
					tone[i/8] = (tone[i/8] << 1) | 0 ;
				}
		}
//#ifdef APPLY_CHANGE
		// set Tone mask
		adsl_drv_get(RLCM_LOADCARRIERMASK, (void *)tone, GET_LOADCARRIERMASK_SIZE);
//#endif
		goto setOk_tone;
	}



	if (strMaskAll[0]) {
		for (i=0; i<chan; i++) {
			//tone[i/8] = tone[i/8] | (1 << (i%8));
			tone[i/8] = (tone[i/8] << 1) | 1 ;
		}
		goto setOk_tone;
	}


	if (strUnmaskAll[0]) {
		for (i=0; i<chan; i++) {
			//tone[i/8] = tone[i/8] | (0 << (i%8));
			tone[i/8] = (tone[i/8] << 1) | 0 ;
		}
		goto setOk_tone;
	}

setOk_tone:

	if ( !mib_set(MIB_ADSL_TONE, (void *)tone) ) {
		strcpy(tmpBuf, Tset_mib_error);
		goto setErr_tone;
	}

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	OK_MSG(submitUrl);

  	return;

setErr_tone:
	ERR_MSG(tmpBuf);

}
#endif

#ifdef CONFIG_DEV_xDSL
#ifdef CONFIG_DSL_VTUO
#if 0
#define VTUO_PRT printf
#else
#define VTUO_PRT(...) while(0){}
#endif

static int FTOI(double d)
{
	char stri[33];
	snprintf( stri, 32, "%.0f", d );
	stri[32]=0;
	return atoi(stri);
}

static void vtuo_chan_init( request * wp )
{
	char *p_page="document.set_vtuo_chan";
	char *sGinpStat[]={"Forbidden", "Preferred", "Forced", "Test" };
	unsigned int tmpUInt;
	unsigned char tmpUChar;
	boaWrite(wp, "<script>\n");

	mib_get(VTUO_CHAN_DS_NDR_MAX, (void *)&tmpUInt);
	boaWrite(wp, "%s.DSRateMax.value=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_CHAN_DS_NDR_MIN, (void *)&tmpUInt);
	boaWrite(wp, "%s.DSRateMin.value=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_CHAN_DS_MAX_DELAY, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSDelay.value=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_CHAN_DS_MIN_INP, (void *)&tmpUChar);
	if(tmpUChar==0) tmpUInt=0;
	else if(tmpUChar==17) tmpUInt=1;
	else tmpUInt= tmpUChar+1;
	boaWrite(wp, "%s.DSINP.selectedIndex=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_CHAN_DS_MIN_INP8, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSINP8.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_CHAN_DS_SOS_MDR, (void *)&tmpUInt);
	boaWrite(wp, "%s.DSSOSRate.value=%u;\n", p_page, tmpUInt);

	mib_get(VTUO_CHAN_US_NDR_MAX, (void *)&tmpUInt);
	boaWrite(wp, "%s.USRateMax.value=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_CHAN_US_NDR_MIN, (void *)&tmpUInt);
	boaWrite(wp, "%s.USRateMin.value=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_CHAN_US_MAX_DELAY, (void *)&tmpUChar);
	boaWrite(wp, "%s.USDelay.value=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_CHAN_US_MIN_INP, (void *)&tmpUChar);
	if(tmpUChar==0) tmpUInt=0;
	else if(tmpUChar==17) tmpUInt=1;
	else tmpUInt= tmpUChar+1;
	boaWrite(wp, "%s.USINP.selectedIndex=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_CHAN_US_MIN_INP8, (void *)&tmpUChar);
	boaWrite(wp, "%s.USINP8.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_CHAN_US_SOS_MDR, (void *)&tmpUInt);
	boaWrite(wp, "%s.USSOSRate.value=%d;\n", p_page, tmpUInt);

	mib_get(VTUO_GINP_DS_MODE, (void *)&tmpUChar);
	boaWrite(wp, "document.getElementById('ChanGinpDs').innerHTML='%s';\n", sGinpStat[tmpUChar]);
	mib_get(VTUO_GINP_US_MODE, (void *)&tmpUChar);
	boaWrite(wp, "document.getElementById('ChanGinpUs').innerHTML='%s';\n", sGinpStat[tmpUChar]);

	boaWrite(wp, "</script>\n");
	return;
}

static void vtuo_ginp_init( request * wp )
{
	char *p_page="document.set_vtuo_ginp";
	unsigned char tmpUChar;
	unsigned int tmpUInt;
	unsigned short tmpUShort;
	boaWrite(wp, "<script>\n");

	mib_get(VTUO_GINP_DS_MODE, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSGinpMode.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_DS_ET_MAX, (void *)&tmpUInt);
	boaWrite(wp, "%s.DSThroMax.value=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_GINP_DS_ET_MIN, (void *)&tmpUInt);
	boaWrite(wp, "%s.DSThroMin.value=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_GINP_DS_NDR_MAX, (void *)&tmpUInt);
	boaWrite(wp, "%s.DSNDRMax.value=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_GINP_DS_SHINE_RATIO, (void *)&tmpUShort);
	boaWrite(wp, "%s.DSRatio.value=%u;\n", p_page, tmpUShort);
	mib_get(VTUO_GINP_DS_LEFTR_THRD, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSThres.value=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_DS_MAX_DELAY, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSMaxDelay.value=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_DS_MIN_DELAY, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSMinDelay.value=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_DS_MIN_INP, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSINP.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_DS_REIN_SYM, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSReinSym.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_DS_REIN_FREQ, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSReinFreq.selectedIndex=%u;\n", p_page, tmpUChar);

	mib_get(VTUO_GINP_US_MODE, (void *)&tmpUChar);
	boaWrite(wp, "%s.USGinpMode.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_US_ET_MAX, (void *)&tmpUInt);
	boaWrite(wp, "%s.USThroMax.value=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_GINP_US_ET_MIN, (void *)&tmpUInt);
	boaWrite(wp, "%s.USThroMin.value=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_GINP_US_NDR_MAX, (void *)&tmpUInt);
	boaWrite(wp, "%s.USNDRMax.value=%u;\n", p_page, tmpUInt);
	mib_get(VTUO_GINP_US_SHINE_RATIO, (void *)&tmpUShort);
	boaWrite(wp, "%s.USRatio.value=%u;\n", p_page, tmpUShort);
	mib_get(VTUO_GINP_US_LEFTR_THRD, (void *)&tmpUChar);
	boaWrite(wp, "%s.USThres.value=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_US_MAX_DELAY, (void *)&tmpUChar);
	boaWrite(wp, "%s.USMaxDelay.value=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_US_MIN_DELAY, (void *)&tmpUChar);
	boaWrite(wp, "%s.USMinDelay.value=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_US_MIN_INP, (void *)&tmpUChar);
	boaWrite(wp, "%s.USINP.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_US_REIN_SYM, (void *)&tmpUChar);
	boaWrite(wp, "%s.USReinSym.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_GINP_US_REIN_FREQ, (void *)&tmpUChar);
	boaWrite(wp, "%s.USReinFreq.selectedIndex=%u;\n", p_page, tmpUChar);

	boaWrite(wp, "</script>\n");
	return;
}

static void vtuo_line_init( request * wp )
{
	char *p_page="document.set_vtuo_line";
	char *sra_mode[4]={"Manual","AdaptInit","Dynamic","SOS"};
	unsigned char tmpUChar;
	unsigned int tmpUInt;
	unsigned short tmpUShort;
	short tmpShort;
	double tmpD;
	int num;
	boaWrite(wp, "<script>\n");

	mib_get(VTUO_LINE_VDSL2_PROFILE, (void *)&tmpUShort);
	boaWrite(wp, "%s.Vd2Prof30a.checked=%s;\n", p_page, (tmpUShort&VDSL2_PROFILE_30A)?"true":"false" );
	boaWrite(wp, "%s.Vd2Prof17a.checked=%s;\n", p_page, (tmpUShort&VDSL2_PROFILE_17A)?"true":"false" );
	boaWrite(wp, "%s.Vd2Prof12a.checked=%s;\n", p_page, (tmpUShort&VDSL2_PROFILE_12A)?"true":"false" );
	boaWrite(wp, "%s.Vd2Prof12b.checked=%s;\n", p_page, (tmpUShort&VDSL2_PROFILE_12B)?"true":"false" );
	boaWrite(wp, "%s.Vd2Prof8a.checked=%s;\n", p_page, (tmpUShort&VDSL2_PROFILE_8A)?"true":"false" );
	boaWrite(wp, "%s.Vd2Prof8b.checked=%s;\n", p_page, (tmpUShort&VDSL2_PROFILE_8B)?"true":"false" );
	boaWrite(wp, "%s.Vd2Prof8c.checked=%s;\n", p_page, (tmpUShort&VDSL2_PROFILE_8C)?"true":"false" );
	boaWrite(wp, "%s.Vd2Prof8d.checked=%s;\n", p_page, (tmpUShort&VDSL2_PROFILE_8D)?"true":"false" );
	/*
	mib_get(VTUO_LINE_DS_MAX_SNR_NOLMT, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSMaxSNRNoLmt.checked=%s;\n", p_page, tmpUChar?"true":"false" );
	*/
	mib_get(VTUO_LINE_DS_MAX_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.DSMaxSNR.value='%.1f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_DS_TARGET_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.DSTargetSNR.value='%.1f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_DS_MIN_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.DSMinSNR.value='%.1f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_DS_BITSWAP, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSBitswap[%u].checked=true;\n", p_page, tmpUChar);
	/*
	mib_get(VTUO_LINE_DS_MAX_TXPWR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.DSMaxTxPwr.value='%.1f';\n", p_page, tmpD );

 	mib_get(VTUO_LINE_DS_MIN_OH_RATE, (void *)&tmpUShort);
	boaWrite(wp, "%s.DSMinOhRate.value=%u;\n", p_page, tmpUShort );

	mib_get(VTUO_LINE_US_MAX_SNR_NOLMT, (void *)&tmpUChar);
	boaWrite(wp, "%s.USMaxSNRNoLmt.checked=%s;\n", p_page, tmpUChar?"true":"false");
	*/
	mib_get(VTUO_LINE_US_MAX_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.USMaxSNR.value='%.1f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_US_TARGET_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.USTargetSNR.value='%.1f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_US_MIN_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.USMinSNR.value='%.1f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_US_BITSWAP, (void *)&tmpUChar);
	boaWrite(wp, "%s.USBitswap[%u].checked=true;\n", p_page, tmpUChar);
	/*
	mib_get(VTUO_LINE_US_MAX_RXPWR_NOLMT, (void *)&tmpUChar);
	boaWrite(wp, "%s.USMaxRxPwrNoLmt.checked=%s;\n", p_page, tmpUChar?"true":"false");
	mib_get(VTUO_LINE_US_MAX_RXPWR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.USMaxRxPwr.value='%.1f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_US_MAX_TXPWR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.USMaxTxPwr.value='%.1f';\n", p_page, tmpD );

 	mib_get(VTUO_LINE_US_MIN_OH_RATE, (void *)&tmpUShort);
	boaWrite(wp, "%s.USMinOhRate.value=%u;\n", p_page, tmpUShort );
	mib_get(VTUO_LINE_TRANS_MODE, (void *)&tmpUChar);
	boaWrite(wp, "%s.TransMode.selectedIndex=%u;\n", p_page, tmpUChar);
	*/
	mib_get(VTUO_LINE_ADSL_PROTOCOL, (void *)&tmpUInt);
	boaWrite(wp, "%s.AdProtANSI.checked=%s;\n", p_page, (tmpUInt&MODE_ANSI)?"true":"false" );
	boaWrite(wp, "%s.AdProtETSI.checked=%s;\n", p_page, (tmpUInt&MODE_ETSI)?"true":"false" );
	boaWrite(wp, "%s.AdProtG9921.checked=%s;\n", p_page, (tmpUInt&MODE_GDMT)?"true":"false" );
	boaWrite(wp, "%s.AdProtG9922.checked=%s;\n", p_page, (tmpUInt&MODE_GLITE)?"true":"false" );
	boaWrite(wp, "%s.AdProtG9923.checked=%s;\n", p_page, (tmpUInt&MODE_ADSL2)?"true":"false" );
	boaWrite(wp, "%s.AdProtG9925.checked=%s;\n", p_page, (tmpUInt&MODE_ADSL2PLUS)?"true":"false" );
	/*
	mib_get(VTUO_LINE_CLASS_MASK, (void *)&tmpUChar);
	boaWrite(wp, "%s.ClassMask.selectedIndex=%u;\n", p_page, tmpUChar);
	*/
	mib_get(VTUO_LINE_LIMIT_MASK, (void *)&tmpUChar);
	boaWrite(wp, "%s.LimitMask.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_LINE_US0_MASK, (void *)&tmpUChar);
	boaWrite(wp, "%s.US0Mask.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_LINE_UPBO_ENABLE, (void *)&tmpUChar);
	boaWrite(wp, "%s.UPBO[%u].checked=true;\n", p_page, tmpUChar);
	mib_get(VTUO_LINE_UPBOKL, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.UPBOKL.value='%.1f';\n", p_page, tmpD );

	mib_get(VTUO_LINE_UPBO_1A, (void *)&tmpShort);
	tmpD=(double)tmpShort/100;
	boaWrite(wp, "%s.USBand1a.value='%.2f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_UPBO_2A, (void *)&tmpShort);
	tmpD=(double)tmpShort/100;
	boaWrite(wp, "%s.USBand2a.value='%.2f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_UPBO_3A, (void *)&tmpShort);
	tmpD=(double)tmpShort/100;
	boaWrite(wp, "%s.USBand3a.value='%.2f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_UPBO_4A, (void *)&tmpShort);
	tmpD=(double)tmpShort/100;
	boaWrite(wp, "%s.USBand4a.value='%.2f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_UPBO_1B, (void *)&tmpShort);
	tmpD=(double)tmpShort/100;
	boaWrite(wp, "%s.USBand1b.value='%.2f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_UPBO_2B, (void *)&tmpShort);
	tmpD=(double)tmpShort/100;
	boaWrite(wp, "%s.USBand2b.value='%.2f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_UPBO_3B, (void *)&tmpShort);
	tmpD=(double)tmpShort/100;
	boaWrite(wp, "%s.USBand3b.value='%.2f';\n", p_page, tmpD );
 	mib_get(VTUO_LINE_UPBO_4B, (void *)&tmpShort);
	tmpD=(double)tmpShort/100;
	boaWrite(wp, "%s.USBand4b.value='%.2f';\n", p_page, tmpD );
	/*
 	mib_get(VTUO_LINE_RT_MODE, (void *)&tmpUChar);
	boaWrite(wp, "%s.RTMode.checked=%s;\n", p_page, tmpUChar?"true":"false" );
	*/
	mib_get(VTUO_LINE_US0_ENABLE, (void *)&tmpUChar);
	boaWrite(wp, "%s.US0[%u].checked=true;\n", p_page, tmpUChar);

	mib_get(VTUO_SRA_DS_RA_MODE, (void *)&tmpUChar);
	boaWrite(wp, "document.getElementById('LineSRAModeDs').innerHTML='DS: %s';\n", sra_mode[tmpUChar]);
	mib_get(VTUO_SRA_US_RA_MODE, (void *)&tmpUChar);
	boaWrite(wp, "document.getElementById('LineSRAModeUs').innerHTML='US: %s';\n", sra_mode[tmpUChar]);

	num=mib_chain_total( MIB_VTUO_PSD_DS_TBL );
	boaWrite(wp, "document.getElementById('LineMibPsdDs').innerHTML='DS: %d Break Point';\n", num);
	num=mib_chain_total( MIB_VTUO_PSD_US_TBL );
	boaWrite(wp, "document.getElementById('LineMibPsdUs').innerHTML='US: %d Break Point';\n", num);
	/*
	mib_get(VTUO_VN_DS_ENABLE, (void *)&tmpUChar);
	boaWrite(wp, "document.getElementById('LineVNDs').innerHTML='DS: %s';\n", tmpUChar?"Enable":"Disable");
	mib_get(VTUO_VN_US_ENABLE, (void *)&tmpUChar);
	boaWrite(wp, "document.getElementById('LineVNUs').innerHTML='US: %s';\n", tmpUChar?"Enable":"Disable");
	*/
	mib_get(VTUO_DPBO_ENABLE, (void *)&tmpUChar);
	boaWrite(wp, "document.getElementById('LineDpboEnable').innerHTML='%s';\n", tmpUChar?"Enable":"Disable");
	/*
	num=mib_chain_total( MIB_VTUO_RFI_TBL );
	boaWrite(wp, "document.getElementById('LineRfi').innerHTML='%d band';\n", num);
	*/
	boaWrite(wp, "</script>\n");
	return;
}

static void vtuo_inm_init( request * wp )
{
	char *p_page="document.set_vtuo_inm";
	unsigned char tmpUChar;
	short tmpShort;
	double tmpD;
	boaWrite(wp, "<script>\n");

	mib_get(VTUO_INM_NE_INP_EQ_MODE, (void *)&tmpUChar);
	boaWrite(wp, "%s.NEInpEqMode.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_INM_NE_INMCC, (void *)&tmpUChar);
	boaWrite(wp, "%s.NEInmCc.value=%u;\n", p_page, tmpUChar );
	mib_get(VTUO_INM_NE_IAT_OFFSET, (void *)&tmpUChar);
	boaWrite(wp, "%s.NEIatOff.value=%u;\n", p_page, tmpUChar );
	mib_get(VTUO_INM_NE_IAT_SETUP, (void *)&tmpUChar);
	boaWrite(wp, "%s.NEIatSet.value=%u;\n", p_page, tmpUChar );
	/*
	mib_get(VTUO_INM_NE_ISDD_SEN, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.NEIsddSen.value='%.1f';\n", p_page, tmpD );
	*/
	mib_get(VTUO_INM_FE_INP_EQ_MODE, (void *)&tmpUChar);
	boaWrite(wp, "%s.FEInpEqMode.selectedIndex=%u;\n", p_page, tmpUChar);
	mib_get(VTUO_INM_FE_INMCC, (void *)&tmpUChar);
	boaWrite(wp, "%s.FEInmCc.value=%u;\n", p_page, tmpUChar );
	mib_get(VTUO_INM_FE_IAT_OFFSET, (void *)&tmpUChar);
	boaWrite(wp, "%s.FEIatOff.value=%u;\n", p_page, tmpUChar );
	mib_get(VTUO_INM_FE_IAT_SETUP, (void *)&tmpUChar);
	boaWrite(wp, "%s.FEIatSet.value=%u;\n", p_page, tmpUChar );
	/*
	mib_get(VTUO_INM_FE_ISDD_SEN, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.FEIsddSen.value='%.1f';\n", p_page, tmpD );
	*/
	boaWrite(wp, "</script>\n");
	return;
}

static void vtuo_sra_init( request * wp )
{
	char *p_page="document.set_vtuo_sra";
	unsigned char tmpUChar;
	unsigned short tmpUShort;
	short tmpShort;
	double tmpD;
	boaWrite(wp, "<script>\n");

	mib_get(VTUO_SRA_DS_RA_MODE, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSRateAdapt[%u].checked=true;\n", p_page, tmpUChar);
	mib_get(VTUO_SRA_DS_DYNAMIC_DEPTH, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSDynDep[%u].checked=true;\n", p_page, tmpUChar);
	mib_get(VTUO_SRA_DS_USHIFT_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.DSUpShiftSNR.value='%.1f';\n", p_page, tmpD );
	mib_get(VTUO_SRA_DS_USHIFT_TIME, (void *)&tmpUShort);
	boaWrite(wp, "%s.DSUpShiftTime.value=%u;\n", p_page, tmpUShort );
	mib_get(VTUO_SRA_DS_DSHIFT_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.DSDownShiftSNR.value='%.1f';\n", p_page, tmpD );
	mib_get(VTUO_SRA_DS_DSHIFT_TIME, (void *)&tmpUShort);
	boaWrite(wp, "%s.DSDownShiftTime.value=%u;\n", p_page, tmpUShort );

	mib_get(VTUO_SRA_DS_SOS_TIME, (void *)&tmpUShort);
	boaWrite(wp, "%s.DSSosTime.value=%u;\n", p_page, tmpUShort );
	mib_get(VTUO_SRA_DS_SOS_CRC, (void *)&tmpUShort);
	boaWrite(wp, "%s.DSSosCrc.value=%u;\n", p_page, tmpUShort );
	mib_get(VTUO_SRA_DS_SOS_NTONE, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSSosnTones.value=%u;\n", p_page, tmpUChar );
	mib_get(VTUO_SRA_DS_SOS_MAX, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSSosMax.value=%u;\n", p_page, tmpUChar );
	/*
	mib_get(VTUO_SRA_DS_SOS_MSTEP_TONE, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSSosMultiStep.selectedIndex=%u;\n", p_page, tmpUChar);
	*/
	mib_get(VTUO_SRA_DS_ROC_ENABLE, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSRocEnable[%u].checked=true;\n", p_page, tmpUChar);
	mib_get(VTUO_SRA_DS_ROC_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.DSRocSNR.value='%.1f';\n", p_page, tmpD );
	mib_get(VTUO_SRA_DS_ROC_MIN_INP, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSRocMinINP.selectedIndex=%u;\n", p_page, tmpUChar);



	mib_get(VTUO_SRA_US_RA_MODE, (void *)&tmpUChar);
	boaWrite(wp, "%s.USRateAdapt[%u].checked=true;\n", p_page, tmpUChar);
	mib_get(VTUO_SRA_US_DYNAMIC_DEPTH, (void *)&tmpUChar);
	boaWrite(wp, "%s.USDynDep[%u].checked=true;\n", p_page, tmpUChar);
	mib_get(VTUO_SRA_US_USHIFT_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.USUpShiftSNR.value='%.1f';\n", p_page, tmpD );
	mib_get(VTUO_SRA_US_USHIFT_TIME, (void *)&tmpUShort);
	boaWrite(wp, "%s.USUpShiftTime.value=%u;\n", p_page, tmpUShort );
	mib_get(VTUO_SRA_US_DSHIFT_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.USDownShiftSNR.value='%.1f';\n", p_page, tmpD );
	mib_get(VTUO_SRA_US_DSHIFT_TIME, (void *)&tmpUShort);
	boaWrite(wp, "%s.USDownShiftTime.value=%u;\n", p_page, tmpUShort );

	mib_get(VTUO_SRA_US_SOS_TIME, (void *)&tmpUShort);
	boaWrite(wp, "%s.USSosTime.value=%u;\n", p_page, tmpUShort );
	mib_get(VTUO_SRA_US_SOS_CRC, (void *)&tmpUShort);
	boaWrite(wp, "%s.USSosCrc.value=%u;\n", p_page, tmpUShort );
	mib_get(VTUO_SRA_US_SOS_NTONE, (void *)&tmpUChar);
	boaWrite(wp, "%s.USSosnTones.value=%u;\n", p_page, tmpUChar );
	mib_get(VTUO_SRA_US_SOS_MAX, (void *)&tmpUChar);
	boaWrite(wp, "%s.USSosMax.value=%u;\n", p_page, tmpUChar );
	/*
	mib_get(VTUO_SRA_US_SOS_MSTEP_TONE, (void *)&tmpUChar);
	boaWrite(wp, "%s.USSosMultiStep.selectedIndex=%u;\n", p_page, tmpUChar);
	*/
	mib_get(VTUO_SRA_US_ROC_ENABLE, (void *)&tmpUChar);
	boaWrite(wp, "%s.USRocEnable[%u].checked=true;\n", p_page, tmpUChar);
	mib_get(VTUO_SRA_US_ROC_SNR, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.USRocSNR.value='%.1f';\n", p_page, tmpD );
	mib_get(VTUO_SRA_US_ROC_MIN_INP, (void *)&tmpUChar);
	boaWrite(wp, "%s.USRocMinINP.selectedIndex=%u;\n", p_page, tmpUChar);


	boaWrite(wp, "</script>\n");
	return;
}

static void vtuo_dpbo_init( request * wp )
{
	char *p_page="document.set_vtuo_dpbo";
	unsigned char tmpUChar;
	unsigned short tmpUShort;
	short tmpShort;
	int tmpInt;
	double tmpD;

	mib_get(VTUO_DPBO_ENABLE, (void *)&tmpUChar);
	boaWrite(wp, "%s.DpboEnable[%u].checked=true;\n", p_page, tmpUChar);
	mib_get(VTUO_DPBO_ESEL, (void *)&tmpShort);
	tmpD= (double)tmpShort/10;
	boaWrite(wp, "%s.DpboESel.value='%.1f';\n", p_page, tmpD );
	mib_get(VTUO_DPBO_ESCMA, (void *)&tmpInt);
	tmpD=(double)tmpInt/10000;
	boaWrite(wp, "%s.DpboEScma.value='%.4f';\n", p_page, tmpD );
	mib_get(VTUO_DPBO_ESCMB, (void *)&tmpInt);
	tmpD=(double)tmpInt/10000;
	boaWrite(wp, "%s.DpboEScmb.value='%.4f';\n", p_page, tmpD );
	mib_get(VTUO_DPBO_ESCMC, (void *)&tmpInt);
	tmpD=(double)tmpInt/10000;
	boaWrite(wp, "%s.DpboEScmc.value='%.4f';\n", p_page, tmpD );
	mib_get(VTUO_DPBO_MUS, (void *)&tmpShort);
	tmpD=(double)tmpShort/10;
	boaWrite(wp, "%s.DpboMus.value='%.1f';\n", p_page, tmpD );
	mib_get(VTUO_DPBO_FMIN, (void *)&tmpUShort);
	boaWrite(wp, "%s.DpboFMin.value='%u';\n", p_page, tmpUShort );
	mib_get(VTUO_DPBO_FMAX, (void *)&tmpUShort);
	boaWrite(wp, "%s.DpboFMax.value='%u';\n", p_page, tmpUShort );

	return;
}

static void vtuo_dpbo_init_tbl( request * wp )
{
	char *p_page="document.set_vtuo_dpbo";
	MIB_CE_VTUO_DPBO_T entry, *p=&entry;
	int num, i;
	double tmpD;

	num=mib_chain_total( MIB_VTUO_DPBO_TBL );
	for(i=0; i<num; i++)
	{
		if (!mib_chain_get(MIB_VTUO_DPBO_TBL, i, p))
			continue;

		boaWrite(wp, "%s.DpboToneId%d.value='%u';\n", p_page, i+1, p->ToneId );
		tmpD=(double)p->PsdLevel/10;
		boaWrite(wp, "%s.DpboPsd%d.value='%.1f';\n", p_page, i+1, tmpD );
	}

	return;
}

static void vtuo_psd_init_tbl( request * wp )
{
	char *p_page="document.set_vtuo_psd";
	MIB_CE_VTUO_PSD_T entry, *p=&entry;
	int num, i;
	double tmpD;

	num=mib_chain_total( MIB_VTUO_PSD_DS_TBL );
	for(i=0; i<num; i++)
	{
		if (!mib_chain_get(MIB_VTUO_PSD_DS_TBL, i, p))
			continue;

		boaWrite(wp, "%s.ToneIdDs%d.value='%u';\n", p_page, i+1, p->ToneId );
		tmpD=(double)p->PsdLevel/10;
		boaWrite(wp, "%s.PsdDs%d.value='%.1f';\n", p_page, i+1, tmpD );
	}

	num=mib_chain_total( MIB_VTUO_PSD_US_TBL );
	for(i=0; i<num; i++)
	{
		if (!mib_chain_get(MIB_VTUO_PSD_US_TBL, i, p))
			continue;

		boaWrite(wp, "%s.ToneIdUs%d.value='%u';\n", p_page, i+1, p->ToneId );
		tmpD=(double)p->PsdLevel/10;
		boaWrite(wp, "%s.PsdUs%d.value='%.1f';\n", p_page, i+1, tmpD );
	}

	return;
}
/*
static void vtuo_vn_init_tbl( request * wp )
{
	char *p_page="document.set_vtuo_psd";
	MIB_CE_VTUO_VN_T entry, *p=&entry;
	int num, i;
	double tmpD;

	num=mib_chain_total( MIB_VTUO_VN_DS_TBL );
	for(i=0; i<num; i++)
	{
		if (!mib_chain_get(MIB_VTUO_VN_DS_TBL, i, p))
			continue;

		boaWrite(wp, "%s.ToneIdDs%d.value='%u';\n", p_page, i+1, p->ToneId );
		tmpD=(double)p->NoiseLevel/10;
		boaWrite(wp, "%s.PsdDs%d.value='%.1f';\n", p_page, i+1, tmpD );
	}

	num=mib_chain_total( MIB_VTUO_VN_US_TBL );
	for(i=0; i<num; i++)
	{
		if (!mib_chain_get(MIB_VTUO_VN_US_TBL, i, p))
			continue;

		boaWrite(wp, "%s.ToneIdUs%d.value='%u';\n", p_page, i+1, p->ToneId );
		tmpD=(double)p->NoiseLevel/10;
		boaWrite(wp, "%s.PsdUs%d.value='%.1f';\n", p_page, i+1, tmpD );
	}

	return;
}

static void vtuo_vn_init( request * wp )
{
	char *p_page="document.set_vtuo_psd";
	unsigned char tmpUChar;

	mib_get(VTUO_VN_DS_ENABLE, (void *)&tmpUChar);
	boaWrite(wp, "%s.DSVnEnable[%u].checked=true;\n", p_page, tmpUChar);

	mib_get(VTUO_VN_US_ENABLE, (void *)&tmpUChar);
	boaWrite(wp, "%s.USVnEnable[%u].checked=true;\n", p_page, tmpUChar);

	return;
}

static void vtuo_rfi_init_tbl( request * wp )
{
	char *p_page="document.set_vtuo_rfi";
	MIB_CE_VTUO_RFI_T entry, *p=&entry;
	int num, i;

	num=mib_chain_total( MIB_VTUO_RFI_TBL );
	for(i=0; i<num; i++)
	{
		if (!mib_chain_get(MIB_VTUO_RFI_TBL, i, p))
			continue;

		boaWrite(wp, "%s.ToneId%d.value='%u';\n", p_page, i+1, p->ToneId );
		boaWrite(wp, "%s.ToneIdEnd%d.value='%u';\n", p_page, i+1, p->ToneIdEnd );
	}

	return;
}
*/
static void vtuo_stat_band( request * wp )
{
#define FMT_STAT_BAND_START "<tr align=center bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s</th>\n"
#define FMT_STAT_BAND_END "\n</tr>\n"

	XDSL_OP *d=xdsl_get_op(0);
	int msgval[9], i;
	double tmpD;

	memset( msgval, 0, sizeof(msgval) );
	d->xdsl_msg_get_array(GetSNRpb, msgval);
	boaWrite(wp, FMT_STAT_BAND_START, "SNRM (dB)" );
	for(i=0; i<9; i++)
	{
		if(msgval[i]==-512)
		{
			boaWrite(wp, "<td>n/a</td>");
		}else{
			tmpD=(double)msgval[i]/10;
			boaWrite(wp, "<td>%.1f</td>", tmpD);
		}
	}
	boaWrite(wp, FMT_STAT_BAND_END);

	memset( msgval, 0, sizeof(msgval) );
	d->xdsl_msg_get_array(GetSATNpb, msgval);
	boaWrite(wp, FMT_STAT_BAND_START, "SATN (dB)" );
	for(i=0; i<9; i++)
	{
		if(msgval[i]==1023)
		{
			boaWrite(wp, "<td>n/a</td>");
		}else{
			tmpD=(double)msgval[i]/10;
			boaWrite(wp, "<td>%.1f</td>", tmpD);
		}
	}
	boaWrite(wp, FMT_STAT_BAND_END);

	memset( msgval, 0, sizeof(msgval) );
	d->xdsl_msg_get_array(GetLATNpb, msgval);
	boaWrite(wp, FMT_STAT_BAND_START, "LATN (dB)" );
	for(i=0; i<9; i++)
	{
		if(msgval[i]==1023)
		{
			boaWrite(wp, "<td>n/a</td>");
		}else{
			tmpD=(double)msgval[i]/10;
			boaWrite(wp, "<td>%.1f</td>", tmpD);
		}
	}
	boaWrite(wp, FMT_STAT_BAND_END);
}

#if 1

static int vtuo_stat_time_fmt(unsigned int uptime, char *buf, int webfmt)
{
	unsigned int updays, upsec, upminutes, uphours;

	if(buf)
	{
		updays = uptime / (60*60*24);
		if (updays)
			sprintf(buf, "%u day%s,%s", updays, (updays != 1) ? "s" : "", webfmt?"&nbsp;":" ");
		upsec = uptime % 60;
		upminutes = uptime / 60;
		uphours = (upminutes / 60) % 24;
		upminutes %= 60;
		sprintf(buf, "%02u:%02u:%02u\n", uphours, upminutes, upsec);

		return 0;
	}

	return -1;
}

static void vtuo_stat_perform( request * wp )
{
#define FMT_STAT_PFM_START "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s</th>\n"
#define FMT_STAT_PFM_END "\n</tr>\n"
	int i,j;
	XDSL_OP *d=xdsl_get_op(0);
	int tmp2dsl[6][27];//sincelink:0=ds/1=us, 15min:2=ds/3=us, 1day:4=ds/5=us
	char *tbuf[]={"ES", "SES", "LOSs", "LOFs", "UAS",
			"CV/CRC", "Corrected", "Uncorrected", "Rtx", "RtxCorrected",
			"RtxUncorrected", "LEFTRs", "MinEFTR", "ErrFreeBits", "LOL",
			"LPR", "Time Elapsed" };
	char buf[32];

	memset(tmp2dsl, 0, sizeof(tmp2dsl) );
	tmp2dsl[0][0]=0;
	d->xdsl_msg_get_array( GetEvCntDs_e127, &tmp2dsl[0][0] );
	tmp2dsl[1][0]=0;
	d->xdsl_msg_get_array( GetEvCntUs_e127, &tmp2dsl[1][0] );
	tmp2dsl[2][0]=0;
	d->xdsl_msg_get_array( GetEvCnt15MinDs_e127, &tmp2dsl[2][0] );
	tmp2dsl[3][0]=0;
	d->xdsl_msg_get_array( GetEvCnt15MinUs_e127, &tmp2dsl[3][0] );
	tmp2dsl[4][0]=0;
	d->xdsl_msg_get_array( GetEvCnt1DayDs_e127, &tmp2dsl[4][0] );
	tmp2dsl[5][0]=0;
	d->xdsl_msg_get_array( GetEvCnt1DayUs_e127, &tmp2dsl[5][0] );

	boaWrite(wp, "<tr bgcolor=#f0f0f0>\n");
	boaWrite(wp, "<th align=left bgcolor=#c0c0c0></th>\n");
	boaWrite(wp, "<th colspan=2>Since Link</th>\n");
	boaWrite(wp, "<th colspan=2>Curr 15 Min</th>\n");
	boaWrite(wp, "<th colspan=2>Curr 1 Day</th>\n");
	boaWrite(wp, "</tr>\n");

	boaWrite(wp, "<tr bgcolor=#f0f0f0>\n");
	boaWrite(wp, "<th align=left bgcolor=#c0c0c0><font size=2>Full Inits</th>\n");
	boaWrite(wp, "<td colspan=2>n/a</td>\n");
	boaWrite(wp, "<td colspan=2>%u</td>\n", tmp2dsl[2][1] );
	boaWrite(wp, "<td colspan=2>%u</td>\n", tmp2dsl[4][1] );
	boaWrite(wp, "</tr>\n");

	boaWrite(wp, "<tr bgcolor=#f0f0f0>\n");
	boaWrite(wp, "<th align=left bgcolor=#c0c0c0><font size=2>Failed Full Inits</th>\n");
	boaWrite(wp, "<td colspan=2>n/a</td>\n");
	boaWrite(wp, "<td colspan=2>%u</td>\n", tmp2dsl[2][2] );
	boaWrite(wp, "<td colspan=2>%u</td>\n", tmp2dsl[4][2] );
	boaWrite(wp, "</tr>\n");

	boaWrite(wp, "<tr align=center bgcolor=#f0f0f0>\n");
	boaWrite(wp, "<th bgcolor=#c0c0c0></th>\n");
	for(j=0; j<6; j++)
		boaWrite(wp, "<td>VTU%s</td>", (j&0x1)?"O":"R" );
	boaWrite(wp, "</tr>\n");

	for(i=0; i<14; i++)
	{
		/* skip some elements */
		if (i >= 6 && i <= 10)
			continue;
		boaWrite(wp, FMT_STAT_PFM_START, tbuf[i] );
		for(j=0; j<6; j++)
			boaWrite(wp, "<td>%u</td>", tmp2dsl[j][i+4] );
		boaWrite(wp, FMT_STAT_PFM_END);
	}

	/*LOL*/
	boaWrite(wp, FMT_STAT_PFM_START, tbuf[14] );
	for(j=0; j<6; j++)
		boaWrite(wp, "<td>%u</td>", tmp2dsl[j][19]);
 	boaWrite(wp, FMT_STAT_PFM_END);

	/*LPR*/
	boaWrite(wp, FMT_STAT_PFM_START, tbuf[15] );
	for(j=0; j<6; j++)
		boaWrite(wp, "<td>%u</td>", tmp2dsl[j][21]);
	boaWrite(wp, FMT_STAT_PFM_END);

	/*TimeElapsed*/
	boaWrite(wp, FMT_STAT_PFM_START, tbuf[16] );
	for(j=0; j<3; j++)
	{
		vtuo_stat_time_fmt( tmp2dsl[j * 2][22], buf, 1 );
		boaWrite(wp, "<td colspan=2>%s</td>", buf);
	}
	boaWrite(wp, FMT_STAT_PFM_END);

	/*boaWrite(wp, "<h4>* Impulse Noise Moniter Current Counter</h4>\n");*/

	memset(tmp2dsl, 0, sizeof(tmp2dsl) );
	tmp2dsl[0][0]=0;
	d->xdsl_msg_get_array( GetInmCntDs_e127, &tmp2dsl[0][0] );
	tmp2dsl[1][0]=0;
	d->xdsl_msg_get_array( GetInmCntUs_e127, &tmp2dsl[1][0] );
	tmp2dsl[2][0]=0;
	d->xdsl_msg_get_array( GetInmCnt15MinDs_e127, &tmp2dsl[2][0] );
	tmp2dsl[3][0]=0;
	d->xdsl_msg_get_array( GetInmCnt15MinUs_e127, &tmp2dsl[3][0] );
	tmp2dsl[4][0]=0;
	d->xdsl_msg_get_array( GetInmCnt1DayDs_e127, &tmp2dsl[4][0] );
	tmp2dsl[5][0]=0;
	d->xdsl_msg_get_array( GetInmCnt1DayUs_e127, &tmp2dsl[5][0] );

	for (i = 1; i <= 26; i++) {
		if (i < 18)
			sprintf(buf, "INMEQ%u", i);
		else if (i < 26)
			sprintf(buf, "IAT%u", i - 18);
		else
			sprintf(buf, "Symbol");
		boaWrite(wp, FMT_STAT_PFM_START, buf);
		for (j = 0; j < 6; j++)
			boaWrite(wp, "<td>%u</td>", tmp2dsl[j][i]);
		boaWrite(wp, FMT_STAT_PFM_END);
	}

	return;
}


#else
static void vtuo_stat_perform( request * wp )
{
#define FMT_STAT_PFM_START "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s</th>\n"
#define FMT_STAT_PFM_END "\n</tr>\n"

	XDSL_OP *d=xdsl_get_op(0);
	int msgval[4], tmpval[4];
	char buf[32];

	/*FECS*/
	boaWrite(wp, FMT_STAT_PFM_START, "FECS" );
	d->xdsl_msg_get_array(GetFECS, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*ES*/
	boaWrite(wp, FMT_STAT_PFM_START, "ES" );
	getAdslDrvInfo( "es-ds", buf, sizeof(buf) );
	boaWrite(wp, "<td>%s</td>", buf);
	getAdslDrvInfo( "es-us", buf, sizeof(buf) );
	boaWrite(wp, "<td>%s</td>", buf);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*SES*/
	boaWrite(wp, FMT_STAT_PFM_START, "SES" );
	getAdslDrvInfo( "ses-ds", buf, sizeof(buf) );
	boaWrite(wp, "<td>%s</td>", buf);
	getAdslDrvInfo( "ses-us", buf, sizeof(buf) );
	boaWrite(wp, "<td>%s</td>", buf);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*LOSs*/
	boaWrite(wp, FMT_STAT_PFM_START, "LOSs" );
	d->xdsl_msg_get_array(GetLOSs, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*LOFs*/
	boaWrite(wp, FMT_STAT_PFM_START, "LOFs" );
	d->xdsl_msg_get_array(GetLOFs, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*UAS*/
	boaWrite(wp, FMT_STAT_PFM_START, "UAS" );
	getAdslDrvInfo( "uas-ds", buf, sizeof(buf) );
	boaWrite(wp, "<td>%s</td>", buf);
	getAdslDrvInfo( "uas-us", buf, sizeof(buf) );
	boaWrite(wp, "<td>%s</td>", buf);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*Code Violation*/
	boaWrite(wp, FMT_STAT_PFM_START, "Code Violation" );
	getAdslDrvInfo( "crc-ds", buf, sizeof(buf) );
	boaWrite(wp, "<td>%s</td>", buf);
	getAdslDrvInfo( "crc-us", buf, sizeof(buf) );
	boaWrite(wp, "<td>%s</td>", buf);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*Corrected*/
	boaWrite(wp, FMT_STAT_PFM_START, "Corrected" );
	d->xdsl_msg_get_array(GetRSCorr, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*Uncorrected*/
	boaWrite(wp, FMT_STAT_PFM_START, "Uncorrected" );
	d->xdsl_msg_get_array(GetRSUnCorr, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*Rtx*/
	boaWrite(wp, FMT_STAT_PFM_START, "Rtx" );
	d->xdsl_msg_get_array(GetReTxRtx, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*RtxCorrected*/
	boaWrite(wp, FMT_STAT_PFM_START, "RtxCorrected" );
	d->xdsl_msg_get_array(GetReTxRtxCorr, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*RtxUncorrected*/
	boaWrite(wp, FMT_STAT_PFM_START, "RtxUncorrected" );
	d->xdsl_msg_get_array(GetReTxRtxUnCorr, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*LEFTRs*/
	boaWrite(wp, FMT_STAT_PFM_START, "LEFTRs" );
	d->xdsl_msg_get_array(GetReTxLEFTRs, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*MinEFTR*/
	boaWrite(wp, FMT_STAT_PFM_START, "MinEFTR" );
	d->xdsl_msg_get_array(GetReTxMinEFTR, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*ErrFreeBits*/
	boaWrite(wp, FMT_STAT_PFM_START, "ErrFreeBits" );
	d->xdsl_msg_get_array(GetReTxErrFreeBits, msgval);
	boaWrite(wp, "<td>%u</td>", msgval[0]);
	boaWrite(wp, "<td>%u</td>", msgval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*LOLs/LOL*/
	boaWrite(wp, FMT_STAT_PFM_START, "LOLs/LOL" );
	d->xdsl_msg_get_array(GetLOLs, msgval);
	d->xdsl_msg_get_array(GetLOL, tmpval);
	boaWrite(wp, "<td>%u/%u</td>", msgval[0], tmpval[0]);
	boaWrite(wp, "<td>%u/%u</td>", msgval[1], tmpval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	/*LPRs/LPR*/
	boaWrite(wp, FMT_STAT_PFM_START, "LPRs/LPR" );
	d->xdsl_msg_get_array(GetLPRs, msgval);
	d->xdsl_msg_get_array(GetLPR, tmpval);
	boaWrite(wp, "<td>%u/%u</td>", msgval[0], tmpval[0]);
	boaWrite(wp, "<td>%u/%u</td>", msgval[1], tmpval[1]);
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, "<td></td><td></td>");
	boaWrite(wp, FMT_STAT_PFM_END);

	return;
}
#endif
static void vtuo_stat_medley_psd( request * wp )
{
#define FMT_STAT_PSD_START "<tr align=center bgcolor=#f0f0f0>\n"
#define FMT_STAT_PSD_MID_1 "<th align=center bgcolor=#c0c0c0><font size=2>"
#define FMT_STAT_PSD_MID_2 "</th>\n"
#define FMT_STAT_PSD_END "\n</tr>\n"
#define VTUO_MEDLEY_PSD_DS_MAXSIZE	48
#define VTUO_MEDLEY_PSD_US_MAXSIZE	32
	XDSL_OP *d=xdsl_get_op(0);
	int ds_msg[1+2*VTUO_MEDLEY_PSD_DS_MAXSIZE], us_msg[1+2*VTUO_MEDLEY_PSD_US_MAXSIZE];
	int i, isShowtime;
	double tmpD;
	char buf[64];

	isShowtime=0;
	memset( buf, 0, sizeof(buf) );
	d->xdsl_drv_get(RLCM_REPORT_MODEM_STATE, (void *)buf, sizeof(buf));
	if( strncmp(buf, "SHOWTIME", 8)==0 )	isShowtime=1;

	if(isShowtime)
	{
		memset( ds_msg, 0, sizeof(ds_msg) );
		ds_msg[0]=VTUO_MEDLEY_PSD_DS_MAXSIZE;
		d->xdsl_msg_get_array(GetPSD_MD_Ds, ds_msg);
		memset( us_msg, 0, sizeof(us_msg) );
		us_msg[0]=VTUO_MEDLEY_PSD_US_MAXSIZE;
		d->xdsl_msg_get_array(GetPSD_MD_Us, us_msg);
	}

	i=0;
	while( (i<VTUO_MEDLEY_PSD_DS_MAXSIZE)||(i<VTUO_MEDLEY_PSD_US_MAXSIZE) )
	{
		boaWrite(wp, FMT_STAT_PSD_START, i );

		if(i<VTUO_MEDLEY_PSD_DS_MAXSIZE)
		{
			boaWrite(wp, FMT_STAT_PSD_MID_1);
			boaWrite(wp, "%d", i+1);
			boaWrite(wp, FMT_STAT_PSD_MID_2);
			if( (isShowtime) && (i<ds_msg[0]) )
			{
				tmpD=(double)ds_msg[2+i*2]/10;
				boaWrite(wp, "<td>%u</td><td>%.1f</td>", ds_msg[1+i*2], tmpD);
			}else{
				boaWrite(wp, "<td>---</td><td>---</td>");
			}
		}else{
			boaWrite(wp, FMT_STAT_PSD_MID_1);
			boaWrite(wp, FMT_STAT_PSD_MID_2);
			boaWrite(wp, "<td></td><td></td>");
		}

		if(i<VTUO_MEDLEY_PSD_US_MAXSIZE)
		{
			boaWrite(wp, FMT_STAT_PSD_MID_1);
			boaWrite(wp, "%d", i+1);
			boaWrite(wp, FMT_STAT_PSD_MID_2);
			if( (isShowtime) && (i<us_msg[0]) )
			{
				tmpD=(double)us_msg[2+i*2]/10;
				boaWrite(wp, "<td>%u</td><td>%.1f</td>", us_msg[1+i*2], tmpD);
			}else{
				boaWrite(wp, "<td>---</td><td>---</td>");
			}
		}else{
			boaWrite(wp, FMT_STAT_PSD_MID_1);
			boaWrite(wp, FMT_STAT_PSD_MID_2);
			boaWrite(wp, "<td></td><td></td>");
		}

		boaWrite(wp, FMT_STAT_PSD_END);

		i++;
	}

	return;
}

int vtuo_checkWrite(int eid, request * wp, int argc, char **argv)
{
	char *name;
	unsigned char tmpUChar;

	//printf( "%s: enter\n", __FUNCTION__ );
   	if (boaArgs(argc, argv, "%s", &name) < 1) {
   		boaError(wp, 400, "Insufficient args\n");
		printf( "%s: error, line=%d\n", __FUNCTION__, __LINE__ );
   		return -1;
   	}
	//printf( "%s: name=%s\n", __FUNCTION__, name );

	if(!strcmp(name, "chan-init")) {
		vtuo_chan_init(wp);
		return 0;
	}else if(!strcmp(name, "ginp-init")) {
		vtuo_ginp_init(wp);
		return 0;
	}else if(!strcmp(name, "line-init")) {
		vtuo_line_init(wp);
		return 0;
	}else if(!strcmp(name, "inm-init")) {
		vtuo_inm_init(wp);
		return 0;
	}else if(!strcmp(name, "sra-init")) {
		vtuo_sra_init(wp);
		return 0;
	}else if(!strcmp(name, "dpbo-init")) {
		vtuo_dpbo_init(wp);
		return 0;
	}else if(!strcmp(name, "dpbo-init-tbl")) {
		vtuo_dpbo_init_tbl(wp);
		return 0;
	}else if(!strcmp(name, "psd-init-tbl")) {
		vtuo_psd_init_tbl(wp);
		return 0;
	/*
	}else if(!strcmp(name, "vn-init-tbl")) {
		vtuo_vn_init_tbl(wp);
		return 0;
	}else if(!strcmp(name, "vn-init")) {
		vtuo_vn_init(wp);
		return 0;
	}else if(!strcmp(name, "rfi-init-tbl")) {
		vtuo_rfi_init_tbl(wp);
		return 0;
	*/
	}else if(!strcmp(name, "stat-band")) {
		vtuo_stat_band(wp);
		return 0;
	}else if(!strcmp(name, "stat-perform")) {
		vtuo_stat_perform(wp);
		return 0;
	}else if(!strcmp(name, "stat-medley-psd")) {
		vtuo_stat_medley_psd(wp);
		return 0;
	}else{
		printf( "%s: error, line=%d, name=%s\n", __FUNCTION__, __LINE__, name );
		return -1;
	}

	return 0;
}

static void formSetVTUO_Chan(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strVal;
	unsigned int tmpUInt;
	unsigned char tmpUChar;

	VTUO_PRT( "%s> enter\n", __FUNCTION__ );

	/*downstream*/
	strVal = boaGetVar(wp, "DSRateMax", "");
	if (strVal[0])
	{
		tmpUInt=atoi( strVal );
		VTUO_PRT( "DSRateMax=%u\n", tmpUInt );
		mib_set(VTUO_CHAN_DS_NDR_MAX, (void *)&tmpUInt);
	}

	strVal = boaGetVar(wp, "DSRateMin", "");
	if (strVal[0])
	{
		tmpUInt=atoi( strVal );
		VTUO_PRT( "DSRateMin=%u\n", tmpUInt );
		mib_set(VTUO_CHAN_DS_NDR_MIN, (void *)&tmpUInt);
	}

	strVal = boaGetVar(wp, "DSDelay", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSDelay=%u\n", tmpUChar );
		mib_set(VTUO_CHAN_DS_MAX_DELAY, (void *)&tmpUChar);
	}

	strVal = boaGetVar(wp, "DSINP", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSINP=%u\n", tmpUChar );
		mib_set(VTUO_CHAN_DS_MIN_INP, (void *)&tmpUChar);
	}

	strVal = boaGetVar(wp, "DSINP8", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSINP8=%u\n", tmpUChar );
		mib_set(VTUO_CHAN_DS_MIN_INP8, (void *)&tmpUChar);
	}

	strVal = boaGetVar(wp, "DSSOSRate", "");
	if (strVal[0])
	{
		tmpUInt=atoi( strVal );
		VTUO_PRT( "DSSOSRate=%u\n", tmpUInt );
		mib_set(VTUO_CHAN_DS_SOS_MDR, (void *)&tmpUInt);
	}


	/*upstream*/
	strVal = boaGetVar(wp, "USRateMax", "");
	if (strVal[0])
	{
		tmpUInt=atoi( strVal );
		VTUO_PRT( "USRateMax=%u\n", tmpUInt );
		mib_set(VTUO_CHAN_US_NDR_MAX, (void *)&tmpUInt);
	}

	strVal = boaGetVar(wp, "USRateMin", "");
	if (strVal[0])
	{
		tmpUInt=atoi( strVal );
		VTUO_PRT( "USRateMin=%u\n", tmpUInt );
		mib_set(VTUO_CHAN_US_NDR_MIN, (void *)&tmpUInt);
	}

	strVal = boaGetVar(wp, "USDelay", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USDelay=%u\n", tmpUChar );
		mib_set(VTUO_CHAN_US_MAX_DELAY, (void *)&tmpUChar);
	}

	strVal = boaGetVar(wp, "USINP", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USINP=%u\n", tmpUChar );
		mib_set(VTUO_CHAN_US_MIN_INP, (void *)&tmpUChar);
	}

	strVal = boaGetVar(wp, "USINP8", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USINP8=%u\n", tmpUChar );
		mib_set(VTUO_CHAN_US_MIN_INP8, (void *)&tmpUChar);
	}

	strVal = boaGetVar(wp, "USSOSRate", "");
	if (strVal[0])
	{
		tmpUInt=atoi( strVal );
		VTUO_PRT( "USSOSRate=%u\n", tmpUInt );
		mib_set(VTUO_CHAN_US_SOS_MDR, (void *)&tmpUInt);
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef APPLY_CHANGE
	VTUOSetupChan();
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);

}

static void formSetVTUO_Ginp(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strVal;
	unsigned int tmpUInt;
	unsigned short tmpUShort;
	unsigned char tmpUChar;

	VTUO_PRT( "%s> enter\n", __FUNCTION__ );

	/*downstream*/
	strVal = boaGetVar(wp, "DSGinpMode", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSGinpMode=%u\n", tmpUChar );
		mib_set(VTUO_GINP_DS_MODE, (void *)&tmpUChar);
	}else
		tmpUChar=0;

	if(tmpUChar!=0) //enable
	{
		strVal = boaGetVar(wp, "DSThroMax", "");
		if (strVal[0])
		{
			tmpUInt=atoi( strVal );
			VTUO_PRT( "DSThroMax=%u\n", tmpUInt );
			mib_set(VTUO_GINP_DS_ET_MAX, (void *)&tmpUInt);
		}
		strVal = boaGetVar(wp, "DSThroMin", "");
		if (strVal[0])
		{
			tmpUInt=atoi( strVal );
			VTUO_PRT( "DSThroMin=%u\n", tmpUInt );
			mib_set(VTUO_GINP_DS_ET_MIN, (void *)&tmpUInt);
		}
		strVal = boaGetVar(wp, "DSNDRMax", "");
		if (strVal[0])
		{
			tmpUInt=atoi( strVal );
			VTUO_PRT( "DSNDRMax=%u\n", tmpUInt );
			mib_set(VTUO_GINP_DS_NDR_MAX, (void *)&tmpUInt);
		}
		strVal = boaGetVar(wp, "DSRatio", "");
		if (strVal[0])
		{
			tmpUShort=atoi( strVal );
			VTUO_PRT( "DSRatio=%u\n", tmpUShort );
			mib_set(VTUO_GINP_DS_SHINE_RATIO, (void *)&tmpUShort);
		}
		strVal = boaGetVar(wp, "DSThres", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "DSThres=%u\n", tmpUChar );
			mib_set(VTUO_GINP_DS_LEFTR_THRD, (void *)&tmpUChar);
		}
		strVal = boaGetVar(wp, "DSMaxDelay", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "DSMaxDelay=%u\n", tmpUChar );
			mib_set(VTUO_GINP_DS_MAX_DELAY, (void *)&tmpUChar);
		}
		strVal = boaGetVar(wp, "DSMinDelay", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "DSMinDelay=%u\n", tmpUChar );
			mib_set(VTUO_GINP_DS_MIN_DELAY, (void *)&tmpUChar);
		}
		strVal = boaGetVar(wp, "DSINP", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "DSINP=%u\n", tmpUChar );
			mib_set(VTUO_GINP_DS_MIN_INP, (void *)&tmpUChar);
		}
		strVal = boaGetVar(wp, "DSReinSym", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "DSReinSym=%u\n", tmpUChar );
			mib_set(VTUO_GINP_DS_REIN_SYM, (void *)&tmpUChar);
		}
		strVal = boaGetVar(wp, "DSReinFreq", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "DSReinFreq=%u\n", tmpUChar );
			mib_set(VTUO_GINP_DS_REIN_FREQ, (void *)&tmpUChar);
		}
	}

	/*Upstream*/
	strVal = boaGetVar(wp, "USGinpMode", "");
	if (strVal[0])
	{
		tmpUChar=(char)atoi( strVal );
		VTUO_PRT( "USGinpMode=%u\n", tmpUChar );
		mib_set(VTUO_GINP_US_MODE, (void *)&tmpUChar);
	}else
		tmpUChar=0;

	if(tmpUChar!=0) //enable
	{
		strVal = boaGetVar(wp, "USThroMax", "");
		if (strVal[0])
		{
			tmpUInt=atoi( strVal );
			VTUO_PRT( "USThroMax=%u\n", tmpUInt );
			mib_set(VTUO_GINP_US_ET_MAX, (void *)&tmpUInt);
		}
		strVal = boaGetVar(wp, "USThroMin", "");
		if (strVal[0])
		{
			tmpUInt=atoi( strVal );
			VTUO_PRT( "USThroMin=%u\n", tmpUInt );
			mib_set(VTUO_GINP_US_ET_MIN, (void *)&tmpUInt);
		}
		strVal = boaGetVar(wp, "USNDRMax", "");
		if (strVal[0])
		{
			tmpUInt=atoi( strVal );
			VTUO_PRT( "USNDRMax=%u\n", tmpUInt );
			mib_set(VTUO_GINP_US_NDR_MAX, (void *)&tmpUInt);
		}
		strVal = boaGetVar(wp, "USRatio", "");
		if (strVal[0])
		{
			tmpUShort=atoi( strVal );
			VTUO_PRT( "USRatio=%u\n", tmpUShort );
			mib_set(VTUO_GINP_US_SHINE_RATIO, (void *)&tmpUShort);
		}
		strVal = boaGetVar(wp, "USThres", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "USThres=%u\n", tmpUChar );
			mib_set(VTUO_GINP_US_LEFTR_THRD, (void *)&tmpUChar);
		}
		strVal = boaGetVar(wp, "USMaxDelay", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "USMaxDelay=%u\n", tmpUChar );
			mib_set(VTUO_GINP_US_MAX_DELAY, (void *)&tmpUChar);
		}
		strVal = boaGetVar(wp, "USMinDelay", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "USMinDelay=%u\n", tmpUChar );
			mib_set(VTUO_GINP_US_MIN_DELAY, (void *)&tmpUChar);
		}
		strVal = boaGetVar(wp, "USINP", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "USINP=%u\n", tmpUChar );
			mib_set(VTUO_GINP_US_MIN_INP, (void *)&tmpUChar);
		}
		strVal = boaGetVar(wp, "USReinSym", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "USReinSym=%u\n", tmpUChar );
			mib_set(VTUO_GINP_US_REIN_SYM, (void *)&tmpUChar);
		}
		strVal = boaGetVar(wp, "USReinFreq", "");
		if (strVal[0])
		{
			tmpUChar=atoi( strVal );
			VTUO_PRT( "USReinFreq=%u\n", tmpUChar );
			mib_set(VTUO_GINP_US_REIN_FREQ, (void *)&tmpUChar);
		}
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef APPLY_CHANGE
	VTUOSetupGinp();
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);

}

static void formSetVTUO_Line(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strVal;
	unsigned char tmpUChar;
	unsigned int tmpUInt;
	unsigned short tmpUShort;
	short tmpShort;
	double tmpD;

	VTUO_PRT( "%s> enter\n", __FUNCTION__ );

	tmpUShort=0;
	strVal = boaGetVar(wp, "Vd2Prof30a", "");
	if (strVal[0]) tmpUShort|=VDSL2_PROFILE_30A;
	strVal = boaGetVar(wp, "Vd2Prof17a", "");
	if (strVal[0]) tmpUShort|=VDSL2_PROFILE_17A;
	strVal = boaGetVar(wp, "Vd2Prof12a", "");
	if (strVal[0]) tmpUShort|=VDSL2_PROFILE_12A;
	strVal = boaGetVar(wp, "Vd2Prof12b", "");
	if (strVal[0]) tmpUShort|=VDSL2_PROFILE_12B;
	strVal = boaGetVar(wp, "Vd2Prof8a", "");
	if (strVal[0]) tmpUShort|=VDSL2_PROFILE_8A;
	strVal = boaGetVar(wp, "Vd2Prof8b", "");
	if (strVal[0]) tmpUShort|=VDSL2_PROFILE_8B;
	strVal = boaGetVar(wp, "Vd2Prof8c", "");
	if (strVal[0]) tmpUShort|=VDSL2_PROFILE_8C;
	strVal = boaGetVar(wp, "Vd2Prof8d", "");
	if (strVal[0]) tmpUShort|=VDSL2_PROFILE_8D;
	VTUO_PRT( "Vd2Prof=0x%04x\n", tmpUShort );
	mib_set(VTUO_LINE_VDSL2_PROFILE, (void *)&tmpUShort);
	/*
	strVal = boaGetVar(wp, "DSMaxSNRNoLmt", "");
	if (strVal[0]) tmpUChar=1;
	else tmpUChar=0;
	VTUO_PRT( "DSMaxSNRNoLmt=%u\n", tmpUChar );
	mib_set(VTUO_LINE_DS_MAX_SNR_NOLMT, (void *)&tmpUChar);
	*/
	strVal = boaGetVar(wp, "DSMaxSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "DSMaxSNR=%d\n", tmpShort );
		mib_set(VTUO_LINE_DS_MAX_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "DSTargetSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "DSTargetSNR=%d\n", tmpShort );
		mib_set(VTUO_LINE_DS_TARGET_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "DSMinSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "DSMinSNR=%d\n", tmpShort );
		mib_set(VTUO_LINE_DS_MIN_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "DSBitswap", "");
	if (strVal[0])
	{
		tmpUChar=atoi(strVal);
		VTUO_PRT( "DSBitswap=%d\n", tmpUChar );
		mib_set(VTUO_LINE_DS_BITSWAP, (void *)&tmpUChar);
	}
	/*
	strVal = boaGetVar(wp, "DSMaxTxPwr", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "DSMaxTxPwr=%d\n", tmpShort );
		mib_set(VTUO_LINE_DS_MAX_TXPWR, (void *)&tmpShort);
	}

	strVal = boaGetVar(wp, "DSMinOhRate", "");
	if (strVal[0])
	{
		tmpUShort=atoi(strVal);
		VTUO_PRT( "DSMinOhRate=%d\n", tmpUShort );
		mib_set(VTUO_LINE_DS_MIN_OH_RATE, (void *)&tmpUShort);
	}

	strVal = boaGetVar(wp, "USMaxSNRNoLmt", "");
	if (strVal[0]) tmpUChar=1;
	else tmpUChar=0;
	VTUO_PRT( "USMaxSNRNoLmt=%u\n", tmpUChar );
	mib_set(VTUO_LINE_US_MAX_SNR_NOLMT, (void *)&tmpUChar);
	*/
	strVal = boaGetVar(wp, "USMaxSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "USMaxSNR=%d\n", tmpShort );
		mib_set(VTUO_LINE_US_MAX_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USTargetSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "USTargetSNR=%d\n", tmpShort );
		mib_set(VTUO_LINE_US_TARGET_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USMinSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "USMinSNR=%d\n", tmpShort );
		mib_set(VTUO_LINE_US_MIN_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USBitswap", "");
	if (strVal[0])
	{
		tmpUChar=atoi(strVal);
		VTUO_PRT( "USBitswap=%d\n", tmpUChar );
		mib_set(VTUO_LINE_US_BITSWAP, (void *)&tmpUChar);
	}
	/*
	strVal = boaGetVar(wp, "USMaxRxPwrNoLmt", "");
	if (strVal[0]) tmpUChar=1;
	else tmpUChar=0;
	VTUO_PRT( "USMaxRxPwrNoLmt=%u\n", tmpUChar );
	mib_set(VTUO_LINE_US_MAX_RXPWR_NOLMT, (void *)&tmpUChar);
	strVal = boaGetVar(wp, "USMaxRxPwr", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "USMaxRxPwr=%d\n", tmpShort );
		mib_set(VTUO_LINE_US_MAX_RXPWR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USMaxTxPwr", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "USMaxTxPwr=%d\n", tmpShort );
		mib_set(VTUO_LINE_US_MAX_TXPWR, (void *)&tmpShort);
	}

	strVal = boaGetVar(wp, "USMinOhRate", "");
	if (strVal[0])
	{
		tmpUShort=atoi(strVal);
		VTUO_PRT( "USMinOhRate=%d\n", tmpUShort );
		mib_set(VTUO_LINE_US_MIN_OH_RATE, (void *)&tmpUShort);
	}
	strVal = boaGetVar(wp, "TransMode", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "TransMode=%u\n", tmpUChar );
		mib_set(VTUO_LINE_TRANS_MODE, (void *)&tmpUChar);
	}
	*/

	tmpUInt=0;
	strVal = boaGetVar(wp, "AdProtANSI", "");
	if (strVal[0]) tmpUInt|=MODE_ANSI;
	strVal = boaGetVar(wp, "AdProtETSI", "");
	if (strVal[0]) tmpUInt|=MODE_ETSI;
	strVal = boaGetVar(wp, "AdProtG9921", "");
	if (strVal[0]) tmpUInt|=MODE_GDMT;
	strVal = boaGetVar(wp, "AdProtG9922", "");
	if (strVal[0]) tmpUInt|=MODE_GLITE;
	strVal = boaGetVar(wp, "AdProtG9923", "");
	if (strVal[0]) tmpUInt|=MODE_ADSL2;
	strVal = boaGetVar(wp, "AdProtG9925", "");
	if (strVal[0]) tmpUInt|=MODE_ADSL2PLUS;
	VTUO_PRT( "AdProt=0x%08x\n", tmpUInt );
	mib_set(VTUO_LINE_ADSL_PROTOCOL, (void *)&tmpUInt);

	/*
	strVal = boaGetVar(wp, "ClassMask", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "ClassMask=%u\n", tmpUChar );
		mib_set(VTUO_LINE_CLASS_MASK, (void *)&tmpUChar);
	}
	*/
	strVal = boaGetVar(wp, "LimitMask", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "LimitMask=%u\n", tmpUChar );
		mib_set(VTUO_LINE_LIMIT_MASK, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "US0Mask", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "US0Mask=%u\n", tmpUChar );
		mib_set(VTUO_LINE_US0_MASK, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "UPBO", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "UPBO=%u\n", tmpUChar );
		mib_set(VTUO_LINE_UPBO_ENABLE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "UPBOKL", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "UPBOKL=%d\n", tmpShort );
		mib_set(VTUO_LINE_UPBOKL, (void *)&tmpShort);
	}

	strVal = boaGetVar(wp, "USBand1a", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*100);
		VTUO_PRT( "USBand1a=%d\n", tmpShort );
		mib_set(VTUO_LINE_UPBO_1A, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USBand2a", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*100);
		VTUO_PRT( "USBand2a=%d\n", tmpShort );
		mib_set(VTUO_LINE_UPBO_2A, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USBand3a", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*100);
		VTUO_PRT( "USBand3a=%d\n", tmpShort );
		mib_set(VTUO_LINE_UPBO_3A, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USBand4a", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*100);
		VTUO_PRT( "USBand4a=%d\n", tmpShort );
		mib_set(VTUO_LINE_UPBO_4A, (void *)&tmpShort);
	}

	strVal = boaGetVar(wp, "USBand1b", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*100);
		VTUO_PRT( "USBand1b=%d\n", tmpShort );
		mib_set(VTUO_LINE_UPBO_1B, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USBand2b", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*100);
		VTUO_PRT( "USBand2b=%d\n", tmpShort );
		mib_set(VTUO_LINE_UPBO_2B, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USBand3b", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*100);
		VTUO_PRT( "USBand3b=%d\n", tmpShort );
		mib_set(VTUO_LINE_UPBO_3B, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USBand4b", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*100);
		VTUO_PRT( "USBand4b=%d\n", tmpShort );
		mib_set(VTUO_LINE_UPBO_4B, (void *)&tmpShort);
	}
	/*
	strVal = boaGetVar(wp, "RTMode", "");
	if (strVal[0]) tmpUChar=1;
	else tmpUChar=0;
	VTUO_PRT( "RTMode=%u\n", tmpUChar );
	mib_set(VTUO_LINE_RT_MODE, (void *)&tmpUChar);
	*/
	strVal = boaGetVar(wp, "US0", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "US0=%u\n", tmpUChar );
		mib_set(VTUO_LINE_US0_ENABLE, (void *)&tmpUChar);
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef APPLY_CHANGE
	VTUOSetupLine();
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);

}

static void formSetVTUO_Inm(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strVal;
	unsigned char tmpUChar;
	short tmpShort;
	double tmpD;

	VTUO_PRT( "%s> enter\n", __FUNCTION__ );

	strVal = boaGetVar(wp, "NEInpEqMode", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "NEInpEqMode=%u\n", tmpUChar );
		mib_set(VTUO_INM_NE_INP_EQ_MODE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "NEInmCc", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "NEInmCc=%u\n", tmpUChar );
		mib_set(VTUO_INM_NE_INMCC, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "NEIatOff", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "NEIatOff=%u\n", tmpUChar );
		mib_set(VTUO_INM_NE_IAT_OFFSET, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "NEIatSet", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "NEIatSet=%u\n", tmpUChar );
		mib_set(VTUO_INM_NE_IAT_SETUP, (void *)&tmpUChar);
	}
	/*
	strVal = boaGetVar(wp, "NEIsddSen", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "NEIsddSen=%d\n", tmpShort );
		mib_set(VTUO_INM_NE_ISDD_SEN, (void *)&tmpShort);
	}
	*/
	strVal = boaGetVar(wp, "FEInpEqMode", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "FEInpEqMode=%u\n", tmpUChar );
		mib_set(VTUO_INM_FE_INP_EQ_MODE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "FEInmCc", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "FEInmCc=%u\n", tmpUChar );
		mib_set(VTUO_INM_FE_INMCC, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "FEIatOff", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "FEIatOff=%u\n", tmpUChar );
		mib_set(VTUO_INM_FE_IAT_OFFSET, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "FEIatSet", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "FEIatSet=%u\n", tmpUChar );
		mib_set(VTUO_INM_FE_IAT_SETUP, (void *)&tmpUChar);
	}
	/*
	strVal = boaGetVar(wp, "FEIsddSen", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "FEIsddSen=%d\n", tmpShort );
		mib_set(VTUO_INM_FE_ISDD_SEN, (void *)&tmpShort);
	}
	*/
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef APPLY_CHANGE
	VTUOSetupInm();
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);

}

static void formSetVTUO_Sra(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strVal;
	unsigned char tmpUChar;
	unsigned short tmpUShort;
	short tmpShort;
	double tmpD;

	VTUO_PRT( "%s> enter\n", __FUNCTION__ );

	strVal = boaGetVar(wp, "DSRateAdapt", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSRateAdapt=%u\n", tmpUChar );
		mib_set(VTUO_SRA_DS_RA_MODE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "DSDynDep", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSDynDep=%u\n", tmpUChar );
		mib_set(VTUO_SRA_DS_DYNAMIC_DEPTH, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "DSUpShiftSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "DSUpShiftSNR=%d\n", tmpShort );
		mib_set(VTUO_SRA_DS_USHIFT_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "DSUpShiftTime", "");
	if (strVal[0])
	{
		tmpUShort=atoi( strVal );
		VTUO_PRT( "DSUpShiftTime=%u\n", tmpUShort );
		mib_set(VTUO_SRA_DS_USHIFT_TIME, (void *)&tmpUShort);
	}
	strVal = boaGetVar(wp, "DSDownShiftSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "DSDownShiftSNR=%d\n", tmpShort );
		mib_set(VTUO_SRA_DS_DSHIFT_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "DSDownShiftTime", "");
	if (strVal[0])
	{
		tmpUShort=atoi( strVal );
		VTUO_PRT( "DSDownShiftTime=%u\n", tmpUShort );
		mib_set(VTUO_SRA_DS_DSHIFT_TIME, (void *)&tmpUShort);
	}

	strVal = boaGetVar(wp, "DSSosTime", "");
	if (strVal[0])
	{
		tmpUShort=atoi( strVal );
		VTUO_PRT( "DSSosTime=%u\n", tmpUShort );
		mib_set(VTUO_SRA_DS_SOS_TIME, (void *)&tmpUShort);
	}
	strVal = boaGetVar(wp, "DSSosCrc", "");
	if (strVal[0])
	{
		tmpUShort=atoi( strVal );
		VTUO_PRT( "DSSosCrc=%u\n", tmpUShort );
		mib_set(VTUO_SRA_DS_SOS_CRC, (void *)&tmpUShort);
	}
	strVal = boaGetVar(wp, "DSSosnTones", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSSosnTones=%u\n", tmpUChar );
		mib_set(VTUO_SRA_DS_SOS_NTONE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "DSSosMax", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSSosMax=%u\n", tmpUChar );
		mib_set(VTUO_SRA_DS_SOS_MAX, (void *)&tmpUChar);
	}
	/*
	strVal = boaGetVar(wp, "DSSosMultiStep", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSSosMultiStep=%u\n", tmpUChar );
		mib_set(VTUO_SRA_DS_SOS_MSTEP_TONE, (void *)&tmpUChar);
	}
	*/
	strVal = boaGetVar(wp, "DSRocEnable", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSRocEnable=%u\n", tmpUChar );
		mib_set(VTUO_SRA_DS_ROC_ENABLE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "DSRocSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "DSRocSNR=%d\n", tmpShort );
		mib_set(VTUO_SRA_DS_ROC_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "DSRocMinINP", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSRocMinINP=%u\n", tmpUChar );
		mib_set(VTUO_SRA_DS_ROC_MIN_INP, (void *)&tmpUChar);
	}



	strVal = boaGetVar(wp, "USRateAdapt", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USRateAdapt=%u\n", tmpUChar );
		mib_set(VTUO_SRA_US_RA_MODE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "USDynDep", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USDynDep=%u\n", tmpUChar );
		mib_set(VTUO_SRA_US_DYNAMIC_DEPTH, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "USUpShiftSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "USUpShiftSNR=%d\n", tmpShort );
		mib_set(VTUO_SRA_US_USHIFT_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USUpShiftTime", "");
	if (strVal[0])
	{
		tmpUShort=atoi( strVal );
		VTUO_PRT( "USUpShiftTime=%u\n", tmpUShort );
		mib_set(VTUO_SRA_US_USHIFT_TIME, (void *)&tmpUShort);
	}
	strVal = boaGetVar(wp, "USDownShiftSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "USDownShiftSNR=%d\n", tmpShort );
		mib_set(VTUO_SRA_US_DSHIFT_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USDownShiftTime", "");
	if (strVal[0])
	{
		tmpUShort=atoi( strVal );
		VTUO_PRT( "USDownShiftTime=%u\n", tmpUShort );
		mib_set(VTUO_SRA_US_DSHIFT_TIME, (void *)&tmpUShort);
	}

	strVal = boaGetVar(wp, "USSosTime", "");
	if (strVal[0])
	{
		tmpUShort=atoi( strVal );
		VTUO_PRT( "USSosTime=%u\n", tmpUShort );
		mib_set(VTUO_SRA_US_SOS_TIME, (void *)&tmpUShort);
	}
	strVal = boaGetVar(wp, "USSosCrc", "");
	if (strVal[0])
	{
		tmpUShort=atoi( strVal );
		VTUO_PRT( "USSosCrc=%u\n", tmpUShort );
		mib_set(VTUO_SRA_US_SOS_CRC, (void *)&tmpUShort);
	}
	strVal = boaGetVar(wp, "USSosnTones", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USSosnTones=%u\n", tmpUChar );
		mib_set(VTUO_SRA_US_SOS_NTONE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "USSosMax", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USSosMax=%u\n", tmpUChar );
		mib_set(VTUO_SRA_US_SOS_MAX, (void *)&tmpUChar);
	}
	/*
	strVal = boaGetVar(wp, "USSosMultiStep", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USSosMultiStep=%u\n", tmpUChar );
		mib_set(VTUO_SRA_US_SOS_MSTEP_TONE, (void *)&tmpUChar);
	}
	*/
	strVal = boaGetVar(wp, "USRocEnable", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USRocEnable=%u\n", tmpUChar );
		mib_set(VTUO_SRA_US_ROC_ENABLE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "USRocSNR", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "USRocSNR=%d\n", tmpShort );
		mib_set(VTUO_SRA_US_ROC_SNR, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "USRocMinINP", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USRocMinINP=%u\n", tmpUChar );
		mib_set(VTUO_SRA_US_ROC_MIN_INP, (void *)&tmpUChar);
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef APPLY_CHANGE
	VTUOSetupSra();
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);

}


static void formSetVTUO_Dpbo(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strVal;
	unsigned char tmpUChar;
	unsigned short tmpUShort;
	short tmpShort;
	int tmpInt;
	double tmpD;

	VTUO_PRT( "%s> enter\n", __FUNCTION__ );

	strVal = boaGetVar(wp, "DpboEnable", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DpboEnable=%u\n", tmpUChar );
		mib_set(VTUO_DPBO_ENABLE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "DpboESel", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "DpboESel=%d\n", tmpShort );
		mib_set(VTUO_DPBO_ESEL, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "DpboEScma", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpInt= FTOI(tmpD*10000);
		VTUO_PRT( "DpboEScma=%d\n", tmpInt );
		mib_set(VTUO_DPBO_ESCMA, (void *)&tmpInt);
	}
	strVal = boaGetVar(wp, "DpboEScmb", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpInt= FTOI(tmpD*10000);
		VTUO_PRT( "DpboEScmb=%d\n", tmpInt );
		mib_set(VTUO_DPBO_ESCMB, (void *)&tmpInt);
	}
	strVal = boaGetVar(wp, "DpboEScmc", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpInt= FTOI(tmpD*10000);
		VTUO_PRT( "DpboEScmc=%d\n", tmpInt );
		mib_set(VTUO_DPBO_ESCMC, (void *)&tmpInt);
	}
	strVal = boaGetVar(wp, "DpboMus", "");
	if (strVal[0])
	{
		tmpD=atof(strVal);
		tmpShort= FTOI(tmpD*10);
		VTUO_PRT( "DpboMus=%d\n", tmpShort );
		mib_set(VTUO_DPBO_MUS, (void *)&tmpShort);
	}
	strVal = boaGetVar(wp, "DpboFMin", "");
	if (strVal[0])
	{
		tmpUShort=atoi( strVal );
		VTUO_PRT( "DpboFMin=%u\n", tmpUShort );
		mib_set(VTUO_DPBO_FMIN, (void *)&tmpUShort);
	}
	strVal = boaGetVar(wp, "DpboFMax", "");
	if (strVal[0])
	{
		tmpUShort=atoi( strVal );
		VTUO_PRT( "DpboFMax=%u\n", tmpUShort );
		mib_set(VTUO_DPBO_FMAX, (void *)&tmpUShort);
	}


{
	int	i, max=16;
	MIB_CE_VTUO_DPBO_T entry, *p=&entry;
	unsigned short pre=0;

	mib_chain_clear(MIB_VTUO_DPBO_TBL);
	for(i=0; i<max; i++)
	{
		char name[16];

		sprintf( name, "DpboToneId%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		p->ToneId=atoi( strVal );
		VTUO_PRT( "%s=%u\n", name, p->ToneId );

		sprintf( name, "DpboPsd%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		tmpD=atof(strVal);
		p->PsdLevel= FTOI(tmpD*10);
		VTUO_PRT( "%s=%d\n", name, p->PsdLevel );

		if( (p->ToneId==0) && (p->PsdLevel==0) )
			break;
		if( (i!=0) && (pre>=p->ToneId) )
			break;
		mib_chain_add(MIB_VTUO_DPBO_TBL, p);
		pre=p->ToneId;
	}

}


#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef APPLY_CHANGE
	VTUOSetupDpbo();
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);

}


static void formSetVTUO_Psd(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strVal;
	double tmpD;
	int	i, max;
	MIB_CE_VTUO_PSD_T entry, *p=&entry;
	unsigned short pre=0;

	mib_chain_clear(MIB_VTUO_PSD_DS_TBL);
	max=32;
	for(i=0; i<max; i++)
	{
		char name[16];

		sprintf( name, "ToneIdDs%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		p->ToneId=atoi( strVal );
		VTUO_PRT( "%s=%u\n", name, p->ToneId );

		sprintf( name, "PsdDs%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		tmpD=atof(strVal);
		p->PsdLevel= FTOI(tmpD*10);
		VTUO_PRT( "%s=%d\n", name, p->PsdLevel );

		if( (p->ToneId==0) && (p->PsdLevel==0) )
			break;
		if( (i!=0) && (pre>=p->ToneId) )
			break;
		mib_chain_add(MIB_VTUO_PSD_DS_TBL, p);
		pre=p->ToneId;
	}

	mib_chain_clear(MIB_VTUO_PSD_US_TBL);
	max=16;
	for(i=0; i<max; i++)
	{
		char name[16];

		sprintf( name, "ToneIdUs%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		p->ToneId=atoi( strVal );
		VTUO_PRT( "%s=%u\n", name, p->ToneId );

		sprintf( name, "PsdUs%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		tmpD=atof(strVal);
		p->PsdLevel= FTOI(tmpD*10);
		VTUO_PRT( "%s=%d\n", name, p->PsdLevel );

		if( (p->ToneId==0) && (p->PsdLevel==0) )
			break;
		if( (i!=0) && (pre>=p->ToneId) )
			break;
		mib_chain_add(MIB_VTUO_PSD_US_TBL, p);
		pre=p->ToneId;
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef APPLY_CHANGE
	VTUOSetupPsd();
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);

}

/*
static void formSetVTUO_Vn(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strVal;
	unsigned char tmpUChar;
	double tmpD;
	int	i, max;
	MIB_CE_VTUO_VN_T entry, *p=&entry;
	unsigned short pre=0;

	strVal = boaGetVar(wp, "DSVnEnable", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "DSVnEnable=%u\n", tmpUChar );
		mib_set(VTUO_VN_DS_ENABLE, (void *)&tmpUChar);
	}
	strVal = boaGetVar(wp, "USVnEnable", "");
	if (strVal[0])
	{
		tmpUChar=atoi( strVal );
		VTUO_PRT( "USVnEnable=%u\n", tmpUChar );
		mib_set(VTUO_VN_US_ENABLE, (void *)&tmpUChar);
	}

	mib_chain_clear(MIB_VTUO_VN_DS_TBL);
	max=32;
	for(i=0; i<max; i++)
	{
		char name[16];

		sprintf( name, "ToneIdDs%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		p->ToneId=atoi( strVal );
		VTUO_PRT( "%s=%u\n", name, p->ToneId );

		sprintf( name, "PsdDs%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		tmpD=atof(strVal);
		p->NoiseLevel= FTOI(tmpD*10);
		VTUO_PRT( "%s=%d\n", name, p->NoiseLevel );

		if( (p->ToneId==0) && (p->NoiseLevel==0) )
			break;
		if( (i!=0) && (pre>=p->ToneId) )
			break;
		mib_chain_add(MIB_VTUO_VN_DS_TBL, p);
		pre=p->ToneId;
	}

	mib_chain_clear(MIB_VTUO_VN_US_TBL);
	max=16;
	for(i=0; i<max; i++)
	{
		char name[16];

		sprintf( name, "ToneIdUs%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		p->ToneId=atoi( strVal );
		VTUO_PRT( "%s=%u\n", name, p->ToneId );

		sprintf( name, "PsdUs%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		tmpD=atof(strVal);
		p->NoiseLevel= FTOI(tmpD*10);
		VTUO_PRT( "%s=%d\n", name, p->NoiseLevel );

		if( (p->ToneId==0) && (p->NoiseLevel==0) )
			break;
		if( (i!=0) && (pre>=p->ToneId) )
			break;
		mib_chain_add(MIB_VTUO_VN_US_TBL, p);
		pre=p->ToneId;
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef APPLY_CHANGE
	VTUOSetupVn();
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);

}

static void formSetVTUO_Rfi(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strVal;
	double tmpD;
	int	i, max;
	MIB_CE_VTUO_RFI_T entry, *p=&entry;
	unsigned short pre=0;

	mib_chain_clear(MIB_VTUO_RFI_TBL);
	max=16;
	for(i=0; i<max; i++)
	{
		char name[16];

		sprintf( name, "ToneId%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		p->ToneId=atoi( strVal );
		VTUO_PRT( "%s=%u\n", name, p->ToneId );

		sprintf( name, "ToneIdEnd%d", i+1 );
		strVal = boaGetVar(wp, name, "");
		p->ToneIdEnd=atoi( strVal );
		VTUO_PRT( "%s=%d\n", name, p->ToneIdEnd );

		if( (p->ToneId==0) && (p->ToneIdEnd==0) )
			break;
		if( p->ToneId > p->ToneIdEnd )
			break;
		if( (i!=0) && (pre>=p->ToneId) )
			break;
		mib_chain_add(MIB_VTUO_RFI_TBL, p);
		pre=p->ToneIdEnd;
	}

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef APPLY_CHANGE
	VTUOSetupRfi();
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);

}
*/

#define FMT_VTUO_HEAD "<head><title>VTU-O DSL Statistics</title></head>\n"
#define FMT_VTUO_BODY "<body>\n"
#define FMT_VTUO_BODY_END "</body>\n"
static void formSetVTUO_Status_History(request * wp, int oneday)
{
	int i,j,i_loop;
	XDSL_OP *d=xdsl_get_op(0);
	int tmp2dsl[27];

	boaHeader(wp);
	boaWrite(wp, FMT_VTUO_HEAD);
	boaWrite(wp, FMT_VTUO_BODY);
	boaWrite(wp, "<h4>* %s Interval</h4>\n", oneday?"One-Day":"15-Min");

	boaWrite(wp, "<table border=0 cellspacing=4 cellpadding=0>\n");
	boaWrite(wp, "<tr align=center bgcolor=#c0c0c0>\n");
	boaWrite(wp, "<th colspan=3></th>\n");
	boaWrite(wp, "<th bgcolor=#f0f0f0>&nbsp;&nbsp;</th>\n");
	boaWrite(wp, "<th colspan=11>VTUR</th>\n");
	boaWrite(wp, "<th bgcolor=#f0f0f0>&nbsp;&nbsp;</th>\n");
	boaWrite(wp, "<th colspan=11>VTUO</th>\n");
	boaWrite(wp, "<th bgcolor=#f0f0f0>&nbsp;&nbsp;</th>\n");
	boaWrite(wp, "<th></th>\n");
	boaWrite(wp, "</tr>\n\n");

	boaWrite(wp, "<tr align=center bgcolor=#c0c0c0>\n");
	boaWrite(wp, "<th>Index</th>\n");
	boaWrite(wp, "<th>Full Inits</th>\n");
	boaWrite(wp, "<th>Failed Full<br>Inits</th>\n");
	for(i=0;i<2;i++)
	{
		char *tbuf[]={"ES", "SES", "LOSs", "LOFs", "UAS",
				"CV/CRC", "LEFTRs", "MinEFTR", "ErrFreeBits", "LOL",
				"LPR", NULL };
		boaWrite(wp, "<th bgcolor=#f0f0f0>&nbsp;&nbsp;</th>\n");
		for(j=0;tbuf[j]!=NULL;j++)
			boaWrite(wp, "<th>%s</th>\n", tbuf[j]);
	}
	boaWrite(wp, "<th bgcolor=#f0f0f0>&nbsp;&nbsp;</th>\n");
	boaWrite(wp, "<th>Index</th>\n");
	boaWrite(wp, "</tr>\n\n");

	if(oneday) i_loop=7;
	else i_loop=96;
	for(i=0;i<i_loop;i++)
	{
		boaWrite(wp, "<tr align=center bgcolor=#f0f0f0>\n");
		boaWrite(wp, "<td>%d</td>\n", i+1);

		//ds
		memset(tmp2dsl, 0, sizeof(tmp2dsl) );
		tmp2dsl[0]=i;
		if(oneday) d->xdsl_msg_get_array( GetEvCnt1DayDs_e127, tmp2dsl );
		else d->xdsl_msg_get_array( GetEvCnt15MinDs_e127, tmp2dsl );
		boaWrite(wp, "<td>%u</td>\n", tmp2dsl[1]);
		boaWrite(wp, "<td>%u</td>\n", tmp2dsl[2]);
		boaWrite(wp, "<td></td>\n");
		for(j=4;j<18;j++) {
			/* skip some elements */
			if (j >= 10 && j <= 14)
				continue;
			boaWrite(wp, "<td>%u</td>\n", tmp2dsl[j]);
		}
		boaWrite(wp, "<td>%u</td>\n", tmp2dsl[19]);
		boaWrite(wp, "<td>%u</td>\n", tmp2dsl[21]);

		//up
		memset(tmp2dsl, 0, sizeof(tmp2dsl) );
		tmp2dsl[0]=i;
		if(oneday) d->xdsl_msg_get_array( GetEvCnt1DayUs_e127, tmp2dsl );
		else d->xdsl_msg_get_array( GetEvCnt15MinUs_e127, tmp2dsl );
		boaWrite(wp, "<td></td>\n");
		for(j=4;j<18;j++) {
			/* skip some elements */
			if (j >= 10 && j <= 14)
				continue;
			boaWrite(wp, "<td>%u</td>\n", tmp2dsl[j]);
		}
		boaWrite(wp, "<td>%u</td>\n", tmp2dsl[19]);
		boaWrite(wp, "<td>%u</td>\n", tmp2dsl[21]);

		boaWrite(wp, "<td></td>\n");
		boaWrite(wp, "<td>%d</td>\n", i+1);
		boaWrite(wp, "</tr>\n\n");
	}

	boaWrite(wp, "</table>\n");
	boaWrite(wp, "<br><br>\n");

	boaWrite(wp, "<h4>* Impulse Noise Moniter %s Interval</h4>\n", oneday?"One-Day":"15-Min");
	boaWrite(wp, "<table border=0 cellspacing=4 cellpadding=0>\n");
	boaWrite(wp, "<tr align=center bgcolor=#c0c0c0>\n");
	boaWrite(wp, "<th bgcolor=#f0f0f0></th>\n");
	boaWrite(wp, "<th colspan=26>VTUR</th>\n");
	boaWrite(wp, "<th bgcolor=#f0f0f0></th>\n");
	boaWrite(wp, "<th colspan=26>VTUO</th>\n");
	boaWrite(wp, "<th bgcolor=#f0f0f0></th>\n");
	boaWrite(wp, "</tr>\n\n");

	boaWrite(wp, "<tr align=center bgcolor=#c0c0c0>\n");
	for(i=0;i<2;i++)
	{
		boaWrite(wp, "<th bgcolor=#f0f0f0>Index</th>\n");
		for(j=1;j<18;j++) boaWrite(wp, "<th>INMEQ%d</th>\n", j);
		for(j=0;j<8;j++) boaWrite(wp, "<th>IAT%d</th>\n", j);
		boaWrite(wp, "<th>Symbol</th>\n");
	}
	boaWrite(wp, "<th bgcolor=#f0f0f0>Index</th>\n");
	boaWrite(wp, "</tr>\n\n");

	if(oneday) i_loop=7;
	else i_loop=96;
	for(i=0;i<i_loop;i++)
	{
		boaWrite(wp, "<tr align=center bgcolor=#f0f0f0>\n");

		boaWrite(wp, "<td>%d</td>\n", i+1);
		memset(tmp2dsl, 0, sizeof(tmp2dsl) );
		tmp2dsl[0]=i;
		if(oneday) d->xdsl_msg_get_array( GetInmCnt1DayDs_e127, tmp2dsl );
		else d->xdsl_msg_get_array( GetInmCnt15MinDs_e127, tmp2dsl );
		for(j=1;j<27;j++) boaWrite(wp, "<td>%u</td>\n", tmp2dsl[j]);

		boaWrite(wp, "<td>%d</td>\n", i+1);
		memset(tmp2dsl, 0, sizeof(tmp2dsl) );
		tmp2dsl[0]=i;
		if(oneday) d->xdsl_msg_get_array( GetInmCnt1DayUs_e127, tmp2dsl );
		else d->xdsl_msg_get_array( GetInmCnt15MinUs_e127, tmp2dsl );
		for(j=1;j<27;j++) boaWrite(wp, "<td>%u</td>\n", tmp2dsl[j]);

		boaWrite(wp, "<td>%d</td>\n", i+1);
		boaWrite(wp, "</tr>\n\n");
	}

	boaWrite(wp, "</table>\n");

	boaWrite(wp, FMT_VTUO_BODY_END);
	boaFooter(wp);
	boaDone(wp, 200);
}
/*
static void formSetVTUO_Status_SubCarrier(request * wp, int item, int ds )
{
	XDSL_OP *d=xdsl_get_op(0);
	int chan, i, isVDSL2, ret, offset;
	int start_tone, end_tone;
	int mval=0, okflag=1;
	short *sbuf;
	int sbuf_size;

	// get the channel number
	chan=256;
	isVDSL2=0;
	if(d->xdsl_msg_get(GetPmdMode,&mval))
	{
		if(mval&MODE_VDSL2)
			isVDSL2=1;
		else
			isVDSL2=0;

		if(mval<MODE_ADSL2PLUS)
			chan=256;
		else
			chan=512;
	}

	if(isVDSL2==0)
	{
		int tmp2dsl[11];
		memset( tmp2dsl, 0, sizeof(tmp2dsl) );
		if(ds)
			d->xdsl_msg_get_array( GetDsBand, tmp2dsl );
		else
			d->xdsl_msg_get_array( GetUsBand, tmp2dsl );

		start_tone= tmp2dsl[1];
		end_tone= tmp2dsl[2];
		printf( "%s: n=%d, start=%d, end=%d\n", __FUNCTION__, tmp2dsl[0], start_tone, end_tone );
	}

	sbuf_size=sizeof(short)*MAX_DSL_TONE*2;
	sbuf = malloc(sbuf_size);
	if(sbuf)
	{
		memset( sbuf, 0,  sbuf_size );
		ret=0;
		if(item==0)
			ret = d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_HLOG, (void *)sbuf, sbuf_size);
		else if(item==1)
			ret = d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_QLN, (void *)sbuf, sbuf_size);
		else if(item==2)
			ret = d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_SNR, (void *)sbuf, sbuf_size);
		if(!ret)
		{
			printf( "%s: RLCM_GET_VDSL2_DIAG_HLOG/QLN/SNR failed\n", __FUNCTION__ );
			okflag=0;
		}
	}else{
		printf( "%s: malloc failed\n", __FUNCTION__ );
		okflag=0;
	}

	if(ds)
		offset=MAX_DSL_TONE;
	else
		offset=0;

	boaHeader(wp);
	boaWrite(wp, FMT_VTUO_HEAD);
	boaWrite(wp, FMT_VTUO_BODY);
	boaWrite(wp, "<h4>* ");
	if(item==0) boaWrite(wp, "Hlog ");
	else if(item==1) boaWrite(wp, "QLN ");
	else if(item==2) boaWrite(wp, "SNR ");
	if(ds) boaWrite(wp, "Downstream");
	else boaWrite(wp, "Upstream");
	boaWrite(wp, "</h4>\n");
	boaWrite(wp, "<table border=0 cellspacing=4 cellpadding=0>\n");

	i=0;
	while(i<chan)
	{
		int j;
		boaWrite(wp, "<tr align=center bgcolor=#f0f0f0>\n");
		boaWrite(wp, "<th width=10%% align=right bgcolor=#c0c0c0>(%d)</th>\n", i+1);

		for(j=i; j<(i+10); j++)
		{
			double tmpD;

			boaWrite(wp, "<td width=9%%>");
			if( okflag&&(j<chan) )
			{
				tmpD=0;
				if(isVDSL2==0)
				{
					if( (j>=start_tone)&&(j<=end_tone) )
						tmpD=(double)sbuf[j]/10;
				}else
					tmpD=(double)sbuf[offset+j]/10;

				boaWrite(wp, "%.1f", tmpD);
			}
			boaWrite(wp, "</td>");
		}
		boaWrite(wp, "</tr>\n\n");

		i=i+10;
	}


	boaWrite(wp, "</table>\n");
	boaWrite(wp, "<br><br>\n");
	boaWrite(wp, FMT_VTUO_BODY_END);
	boaFooter(wp);
	boaDone(wp, 200);

	if(sbuf) free(sbuf);
}
*/
static void formSetVTUO_Status_BitAlloc(request * wp, int ds)
{
	int i, ret;
	XDSL_OP *d=xdsl_get_op(0);
	int okflag=1, maxtone=4096;
	unsigned char *cbuf;
	int cbuf_size;

	cbuf_size=sizeof(char)*maxtone;
	cbuf = malloc(cbuf_size);
	if(cbuf)
	{
		memset( cbuf, 0,  cbuf_size );
		ret=0;
		if(ds)
			ret = d->xdsl_msg_get_array(GetBitPerToneDs, (int*)cbuf);
		else
			ret = d->xdsl_msg_get_array(GetBitPerToneUs, (int*)cbuf);
		if(!ret)
		{
			printf( "%s: GetBitPerToneDs/Us failed\n", __FUNCTION__ );
			okflag=0;
		}
	}else{
		printf( "%s: malloc failed\n", __FUNCTION__ );
		okflag=0;
	}

	boaHeader(wp);
	boaWrite(wp, FMT_VTUO_HEAD);
	boaWrite(wp, FMT_VTUO_BODY);
	boaWrite(wp, "<h4>* BitAlloc %s</h4>\n", ds?"Downstream":"Upstream");
	boaWrite(wp, "<table border=0 cellspacing=4 cellpadding=0>\n");

	i=0;
	while(i<maxtone)
	{
		int j;
		boaWrite(wp, "<tr align=center bgcolor=#f0f0f0>\n");
		boaWrite(wp, "<th width=10%% align=right bgcolor=#c0c0c0>(%d)</th>\n", i+1);

		for(j=i; j<i+10; j++)
		{
			boaWrite(wp, "<td width=9%%>");
			if(okflag&&(j<maxtone)) boaWrite(wp, "%u", cbuf[j] );
			boaWrite(wp, "</td>");
		}

		boaWrite(wp, "</tr>\n\n");
		i=i+10;
	}


	boaWrite(wp, "</table>\n");
	boaWrite(wp, "<br><br>\n");
	boaWrite(wp, FMT_VTUO_BODY_END);
	boaFooter(wp);
	boaDone(wp, 200);

	if(cbuf) free(cbuf);
}

static void formSetVTUO_Status_GainAlloc(request * wp, int ds)
{
	int i, ret=0;
	XDSL_OP *d=xdsl_get_op(0);
	int okflag=1, maxtone=4096;
	unsigned short *sbuf;
	int sbuf_size;

	sbuf_size=sizeof(short)*maxtone;
	sbuf = malloc(sbuf_size);
	if(sbuf)
	{
		memset( sbuf, 0,  sbuf_size );
		if(ds)
			ret = d->xdsl_msg_get_array(GetGainPerToneDs, (int*)sbuf);
		else
			ret = d->xdsl_msg_get_array(GetGainPerToneUs, (int*)sbuf);
		if(!ret)
		{
			printf( "%s: GetGainPerToneDs/Us failed\n", __FUNCTION__ );
			okflag=0;
		}
	}else{
		printf( "%s: malloc failed\n", __FUNCTION__ );
		okflag=0;
	}

	boaHeader(wp);
	boaWrite(wp, FMT_VTUO_HEAD);
	boaWrite(wp, FMT_VTUO_BODY);
	boaWrite(wp, "<h4>* GainAlloc %s</h4>\n", ds?"Downstream":"Upstream");
	boaWrite(wp, "<table border=0 cellspacing=4 cellpadding=0>\n");

	i=0;
	while(i<maxtone)
	{
		int j;
		boaWrite(wp, "<tr align=center bgcolor=#f0f0f0>\n");
		boaWrite(wp, "<th width=10%% align=right bgcolor=#c0c0c0>(%d)</th>\n", i+1);

		for(j=i; j<i+10; j++)
		{
			boaWrite(wp, "<td width=9%%>");
			if(okflag&&(j<maxtone)) boaWrite(wp, "%u", sbuf[j] );
			boaWrite(wp, "</td>");
		}

		boaWrite(wp, "</tr>\n\n");
		i=i+10;
	}

	boaWrite(wp, "</table>\n");
	boaWrite(wp, "<br><br>\n");
	boaWrite(wp, FMT_VTUO_BODY_END);
	boaFooter(wp);
	boaDone(wp, 200);

	if(sbuf) free(sbuf);
}

static void formSetVTUO_Status(request * wp, char *path, char *query)
{
	char *strApply;

	strApply = boaGetVar(wp, "StatusPage", "");
	VTUO_PRT( "StatusPage=%s\n", strApply);

	if(!strcmp(strApply, "15min")) {
		formSetVTUO_Status_History(wp, 0);
	}else if(!strcmp(strApply, "oneday")) {
		formSetVTUO_Status_History(wp, 1);
	/*
	}else if(!strcmp(strApply, "hlog_ds")) {
		formSetVTUO_Status_SubCarrier(wp, 0, 1);
	}else if(!strcmp(strApply, "hlog_us")) {
		formSetVTUO_Status_SubCarrier(wp, 0, 0);
	}else if(!strcmp(strApply, "qln_ds")) {
		formSetVTUO_Status_SubCarrier(wp, 1, 1);
	}else if(!strcmp(strApply, "qln_us")) {
		formSetVTUO_Status_SubCarrier(wp, 1, 0);
	}else if(!strcmp(strApply, "snr_ds")) {
		formSetVTUO_Status_SubCarrier(wp, 2, 1);
	}else if(!strcmp(strApply, "snr_us")) {
		formSetVTUO_Status_SubCarrier(wp, 2, 0);
	*/
	}else if(!strcmp(strApply, "bit_ds")) {
		formSetVTUO_Status_BitAlloc(wp, 1);
	}else if(!strcmp(strApply, "bit_us")) {
		formSetVTUO_Status_BitAlloc(wp, 0);
	}else if(!strcmp(strApply, "gain_ds")) {
		formSetVTUO_Status_GainAlloc(wp, 1);
	}else if(!strcmp(strApply, "gain_us")) {
		formSetVTUO_Status_GainAlloc(wp, 0);
	}else{
		boaHeader(wp);
		boaWrite(wp, "<body><h4>StatusPage=%s is not ready!!!</body></h4>\n", strApply);
		boaFooter(wp);
		boaDone(wp, 200);
	}
}

void formSetVTUO(request * wp, char *path, char *query)
{
	char *strApply;

	strApply = boaGetVar(wp, "ChanProfile", "");
	if (strApply[0]) {
		formSetVTUO_Chan( wp, path, query );
	}

	strApply = boaGetVar(wp, "GinpSetup", "");
	if (strApply[0]) {
		formSetVTUO_Ginp( wp, path, query );
	}

	strApply = boaGetVar(wp, "LineProfile", "");
	if (strApply[0]) {
		formSetVTUO_Line( wp, path, query );
	}

	strApply = boaGetVar(wp, "InmSetup", "");
	if (strApply[0]) {
		formSetVTUO_Inm( wp, path, query );
	}

	strApply = boaGetVar(wp, "SraSetup", "");
	if (strApply[0]) {
		formSetVTUO_Sra( wp, path, query );
	}

	strApply = boaGetVar(wp, "DpboSetup", "");
	if (strApply[0]) {
		formSetVTUO_Dpbo( wp, path, query );
	}

	strApply = boaGetVar(wp, "PsdSetup", "");
	if (strApply[0]) {
		formSetVTUO_Psd( wp, path, query );
	}
	/*
	strApply = boaGetVar(wp, "VnSetup", "");
	if (strApply[0]) {
		formSetVTUO_Vn( wp, path, query );
	}

	strApply = boaGetVar(wp, "RfiSetup", "");
	if (strApply[0]) {
		formSetVTUO_Rfi( wp, path, query );
	}
	*/
	strApply = boaGetVar(wp, "StatusPage", "");
	if (strApply[0]) {
		formSetVTUO_Status( wp, path, query );
	}

	return;
}
#endif /*CONFIG_DSL_VTUO*/


void formSetAdsl(request * wp, char *path, char *query)
{
	char *submitUrl;
	char *strVal, *strApply;
	char olr;
	short mode;
	int xmode;


	strApply = boaGetVar(wp, "psdm", "");
	if (strApply[0]) {
		#if SUPPORT_TR105
		int id;
		if (!gstrcmp(strApply, "Enable")) {
			psd_measure = 1;
			id = RLCM_ENABLE_NODROPLINEFLAG;
		} else if (!gstrcmp(strApply, "Disable")) {
			psd_measure = 0;
			id = RLCM_DISABLE_NODROPLINEFLAG;
		} else
			goto done;

		adsl_drv_get(id, 0, 0);
		#ifdef CONFIG_USER_XDSL_SLAVE
		adsl_slv_drv_get(id, 0, 0);
		#endif /*CONFIG_USER_XDSL_SLAVE*/
		#endif
		goto done;
	}

	mib_get(MIB_ADSL_MODE, (void *)&mode);
	mode &= ADSL_MODE_ANXB;

	strVal = boaGetVar(wp, "glite", "");
	if (strVal[0]=='1')
		mode |= ADSL_MODE_GLITE;
	strVal = boaGetVar(wp, "t1413", "");
	if (strVal[0]=='1')
		mode |= ADSL_MODE_T1413;
	strVal = boaGetVar(wp, "gdmt", "");
	if (strVal[0]=='1')
		mode |= ADSL_MODE_GDMT;
	strVal = boaGetVar(wp, "adsl2", "");
	if (strVal[0]=='1')
		mode |= ADSL_MODE_ADSL2;
	strVal = boaGetVar(wp, "anxl", "");
	if (strVal[0]=='1')
		mode |= ADSL_MODE_ANXL;
	strVal = boaGetVar(wp, "anxm", "");
	if (strVal[0]=='1')
		mode |= ADSL_MODE_ANXM;
	strVal = boaGetVar(wp, "adsl2p", "");
	if (strVal[0]=='1')
		mode |= ADSL_MODE_ADSL2P;
#ifdef ENABLE_ADSL_MODE_GINP
	strVal = boaGetVar(wp, "ginp", "");
	if (strVal[0]=='1')
		mode |= ADSL_MODE_GINP;
#endif
#ifdef CONFIG_VDSL
	strVal = boaGetVar(wp, "vdsl2", "");
	if (strVal[0]=='1')
		mode |= ADSL_MODE_VDSL2;
#endif /*CONFIG_VDSL*/

	mib_set(MIB_ADSL_MODE, (void *)&mode);

#ifdef CONFIG_VDSL
	//VDSL2 profile
	strVal = boaGetVar(wp, "vdsl2", "");
	if (strVal[0]=='1')
	{
		unsigned short vd2p=0;

		strVal = boaGetVar(wp, "vdsl2p8a", "");
		if (strVal[0]=='1')
			vd2p |= VDSL2_PROFILE_8A;
		strVal = boaGetVar(wp, "vdsl2p8b", "");
		if (strVal[0]=='1')
			vd2p |= VDSL2_PROFILE_8B;
		strVal = boaGetVar(wp, "vdsl2p8c", "");
		if (strVal[0]=='1')
			vd2p |= VDSL2_PROFILE_8C;
		strVal = boaGetVar(wp, "vdsl2p8d", "");
		if (strVal[0]=='1')
			vd2p |= VDSL2_PROFILE_8D;
		strVal = boaGetVar(wp, "vdsl2p12a", "");
		if (strVal[0]=='1')
			vd2p |= VDSL2_PROFILE_12A;
		strVal = boaGetVar(wp, "vdsl2p12b", "");
		if (strVal[0]=='1')
			vd2p |= VDSL2_PROFILE_12B;
		strVal = boaGetVar(wp, "vdsl2p17a", "");
		if (strVal[0]=='1')
			vd2p |= VDSL2_PROFILE_17A;
		strVal = boaGetVar(wp, "vdsl2p30a", "");
		if (strVal[0]=='1')
			vd2p |= VDSL2_PROFILE_30A;

		mib_set(MIB_VDSL2_PROFILE, (void *)&vd2p);
	}
#endif /*CONFIG_VDSL*/


	// OLR type
	olr = 0;
	strVal = boaGetVar(wp, "bswap", "");
	if (strVal[0]=='1')
		olr |= 1;

	strVal = boaGetVar(wp, "sra", "");
	if (strVal[0]=='1')
		olr |= 2;

	mib_set(MIB_ADSL_OLR, (void *)&olr);

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	//added by xlyue
	//syslog(LOG_INFO, "ADSL Setting -- ADSL_MODE: 0x%x, ADSL_ORL: 0x%x\n",mode,olr);

#ifdef APPLY_CHANGE
#ifdef CONFIG_USER_XDSL_SLAVE
	mib_slv_sync_dsl();
	sys_slv_init( UC_STR_DSL_SETUP );
#endif /*CONFIG_USER_XDSL_SLAVE*/
	setupDsl();
#ifdef CONFIG_VDSL
	/*when ad<->vd, need retrain*/
	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
#endif /*CONFIG_VDSL*/
#endif
done:
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	OK_MSG(submitUrl);

	return;

}

static void _formDiagAdsl(request * wp, XDSL_OP *d)
{
	char *str, *submitUrl;
	int mode;

//	str = boaGetVar(wp, "start", "");
	// Display diagnostic messages
	boaHeader(wp);
	submitUrl = boaGetVar(wp, "submit-url", "");
	boaWrite(wp, "<META HTTP-EQUIV=Refresh CONTENT=\"180; URL=%s?act=1\">\n", submitUrl );
   	boaWrite(wp, "<body><blockquote><br><br>\n");
   	boaWrite(wp, "%s<br>\n", Tadsl_diag_wait);
   	boaWrite(wp, "</blockquote></body>");
   	boaFooter(wp);
	boaDone(wp, 200);

//	if (str[0]) {
		// start diagnose here
#ifdef _USE_NEW_IOCTL_FOR_DSLDIAG_
		//fprintf( stderr, "use RLCM_ENABLE_DIAGNOSTIC to start dsldiag\n" );
		mode=0;
		d->xdsl_drv_get(RLCM_ENABLE_DIAGNOSTIC, (void *)&mode, sizeof(int));//Lupin
#else
		mode = 41;
		d->xdsl_drv_get(RLCM_DEBUG_MODE, (void *)&mode, sizeof(int));
#endif
		d->xdsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
//	}
	//submitUrl = boaGetVar(wp, "submit-url", "");
	//if (submitUrl[0])
	//	boaRedirect(wp, submitUrl);
}

void formDiagAdsl(request * wp, char *path, char *query)
{
	XDSL_OP *d;
#ifdef CONFIG_USER_XDSL_SLAVE
	char *id;
	id = boaGetVar(wp, "slaveid", "");
	//printf( "%s: id=%s\n", __FUNCTION__, id );
	if(id[0])
	{
		d=xdsl_get_op(1);
	}else
#endif /*CONFIG_USER_XDSL_SLAVE*/
	{
		d=xdsl_get_op(0);
	}

	_formDiagAdsl(wp, d);
}


void formStatAdsl(request * wp, char *path, char *query)
{
	char *submitUrl;

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
}
#endif

#ifdef CONFIG_DEV_xDSL
static int _adslToneDiagTbl(request * wp, XDSL_OP *d)
{
	char mode;
	static int chan=256;
	int nBytesSent=0;
	char str[16], hlin_ds[32], hlin_us[32];
	char latt_ds[16], latt_us[16], satt_ds[16], satt_us[16];
	char snr_ds[16], snr_us[16], attr_ds[16], attr_us[16];
	char txpw_ds[16], txpw_us[16];
	char *act;
	int ldstate;
#ifdef CONFIG_VDSL
	int mval=0;
#endif /*CONFIG_VDSL*/

	act = boaGetVar(wp, "act", "");
	if (act && act[0]=='1') {
		d->xdsl_drv_get(RLCM_GET_LD_STATE, (void *)&ldstate, 4);
		if (ldstate != 0)
			nBytesSent += boaWrite(wp, "<tr>\n<b><font color='green'>%s</b></tr>\n", Tadsl_diag_suc);
		else
			nBytesSent += boaWrite(wp, "<tr>\n<b><font color='red'>%s</b></tr>\n", Tadsl_diag_fail);
	}

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0 width=\"120\"></th>\n");
	nBytesSent += boaWrite(wp, "<th width=\"100\"><font size=2>%s</th><th width=\"100\"><font size=2>%s</th>\n</tr>\n", multilang(LANG_DOWNSTREAM), multilang(LANG_UPSTREAM));

#ifdef CONFIG_VDSL
	//reset
	hlin_ds[0] = hlin_us[0] = latt_ds[0] = latt_us[0] = 0;
	satt_ds[0] = satt_us[0] = snr_ds[0] = snr_us[0] = 0;
	attr_ds[0] = attr_us[0] = txpw_ds[0] = txpw_us[0] = 0;

	// get the channel number
	if(d->xdsl_msg_get(GetPmdMode,&mval))
	{
		short *pother;
		VDSL2DiagOthers vd2other;

		pother=(short*)&vd2other;
		memset( &vd2other, 0, sizeof(vd2other) );
		if( d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_OTHER, (void *)&vd2other, sizeof(vd2other)) )
		{
			if(mval<MODE_VDSL1)
			{
				// Hlinear scale
				snprintf(hlin_ds, 16, "%d", (unsigned short)pother[1]);
				snprintf(hlin_us, 16, "%d", (unsigned short)pother[0]);
				// loop Attenuation
				snprintf(latt_ds, 16, "%d.%d", pother[3]/10, abs(pother[3]%10));
				snprintf(latt_us, 16, "%d.%d", pother[2]/10, abs(pother[2]%10));
				// signal Attenuation
				snprintf(satt_ds, 16, "%d.%d", pother[5]/10, abs(pother[5]%10));
				snprintf(satt_us, 16, "%d.%d", pother[4]/10, abs(pother[4]%10));
				// SNR Margin
				snprintf(snr_ds, 16, "%d.%d", pother[7]/10, abs(pother[7]%10));
				snprintf(snr_us, 16, "%d.%d", pother[6]/10, abs(pother[6]%10));
				// Attainable Rate
				snprintf(attr_ds, 16, "%d", pother[9]);
				snprintf(attr_us, 16, "%d", pother[8]);
				// tx power
				snprintf(txpw_ds, 16, "%d.%d", pother[11]/10, abs(pother[11]%10));
				snprintf(txpw_us, 16, "%d.%d", pother[10]/10, abs(pother[10]%10));
			}else if(mval&MODE_VDSL2){
				//printf( "%s:%d> it's VDSL2\n", __FUNCTION__, __LINE__ );
				// Hlinear scale
				snprintf(hlin_ds, 16, "%d", (unsigned short)vd2other.HlinScale_ds);
				snprintf(hlin_us, 16, "%d", (unsigned short)vd2other.HlinScale_us);
				#if 0
				// loop Attenuation
				snprintf(latt_ds, 16, "%d.%d", 0, 0);
				snprintf(latt_us, 16, "%d.%d", 0, 0);
				// signal Attenuation
				snprintf(satt_ds, 16, "%d.%d", 0, 0);
				snprintf(satt_us, 16, "%d.%d", 0, 0);
				#endif
				// SNR Margin
				snprintf(snr_ds, 16, "%d.%d", vd2other.SNRMds/10, abs(vd2other.SNRMds%10));
				snprintf(snr_us, 16, "%d.%d", vd2other.SNRMus/10, abs(vd2other.SNRMus%10));
				// Attainable Rate
				snprintf(attr_ds, 16, "%d", vd2other.ATTNDRds);
				snprintf(attr_us, 16, "%d", vd2other.ATTNDRus);
				// tx power
				snprintf(txpw_ds, 16, "%d.%d", vd2other.ACTATPds/10, abs(vd2other.ACTATPds%10));
				snprintf(txpw_us, 16, "%d.%d", vd2other.ACTATPus/10, abs(vd2other.ACTATPus%10));
			}
		}else{
			//printf( "%s:%d> RLCM_GET_VDSL2_DIAG_OTHER failed\n", __FUNCTION__, __LINE__ );
		}
	}else{
		//printf( "%s:%d> GetPmdMode failed\n", __FUNCTION__, __LINE__ );
	}

#else /*CONFIG_VDSL*/

	// get the channel number
	if(d->xdsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&mode, 1)) {
		//ramen to clear the first 3 bit
		mode&=0x1F;
		if (mode < 5) //adsl1/adsl2
			chan = 256;
		else
			chan = 512;
	}

	//hlog = malloc(sizeof(short)*(chan*3+HLOG_ADDITIONAL_SIZE));
	hlog = malloc(sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE));
	//if (d->xdsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)hlog, sizeof(short)*(chan*3+HLOG_ADDITIONAL_SIZE))) {
	if (d->xdsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)hlog, sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE))) {
		// Hlinear scale
		snprintf(hlin_ds, 16, "%d", (unsigned short)hlog[chan*3+1]);
		snprintf(hlin_us, 16, "%d", (unsigned short)hlog[chan*3]);
		// loop Attenuation
		snprintf(latt_ds, 16, "%d.%d", hlog[chan*3+3]/10, abs(hlog[chan*3+3]%10));
		snprintf(latt_us, 16, "%d.%d", hlog[chan*3+2]/10, abs(hlog[chan*3+2]%10));
		// signal Attenuation
		snprintf(satt_ds, 16, "%d.%d", hlog[chan*3+5]/10, abs(hlog[chan*3+5]%10));
		snprintf(satt_us, 16, "%d.%d", hlog[chan*3+4]/10, abs(hlog[chan*3+4]%10));
		// SNR Margin
		snprintf(snr_ds, 16, "%d.%d", hlog[chan*3+7]/10, abs(hlog[chan*3+7]%10));
		snprintf(snr_us, 16, "%d.%d", hlog[chan*3+6]/10, abs(hlog[chan*3+6]%10));
		// Attainable Rate
		snprintf(attr_ds, 16, "%d", hlog[chan*3+9]);
		snprintf(attr_us, 16, "%d", hlog[chan*3+8]);
		// tx power
		snprintf(txpw_ds, 16, "%d.%d", hlog[chan*3+11]/10, abs(hlog[chan*3+11]%10));
		snprintf(txpw_us, 16, "%d.%d", hlog[chan*3+10]/10, abs(hlog[chan*3+10]%10));
	}
	else {
		hlin_ds[0] = hlin_us[0] = latt_ds[0] = latt_us[0] = 0;
		satt_ds[0] = satt_us[0] = snr_ds[0] = snr_us[0] = 0;
		attr_ds[0] = attr_us[0] = txpw_ds[0] = txpw_us[0] = 0;
	}
	if(hlog) free(hlog);
#endif /*CONFIG_VDSL*/

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s</th>\n", multilang(LANG_HLIN_SCALE));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", hlin_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", hlin_us);

#ifdef CONFIG_VDSL
if(mval<MODE_VDSL1)
{
#endif /*CONFIG_VDSL*/
	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dB)</th>\n", multilang(LANG_LOOP_ATTENUATION));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", latt_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", latt_us);

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dB)</th>\n", multilang(LANG_SIGNAL_ATTENUATION));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", satt_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", satt_us);
#ifdef CONFIG_VDSL
}
#endif /*CONFIG_VDSL*/

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dB)</th>\n", multilang(LANG_SNR_MARGIN));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", snr_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", snr_us);

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(Kbps)</th>\n", multilang(LANG_ATTAINABLE_RATE));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", attr_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", attr_us);

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dBm)</th>\n", multilang(LANG_OUTPUT_POWER));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", txpw_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", txpw_us);

	return nBytesSent;
}

int adslToneDiagTbl(int eid, request * wp, int argc, char **argv)
{
	XDSL_OP *d;
#ifdef CONFIG_USER_XDSL_SLAVE
	//printf( "\n%s: %s\n", __FUNCTION__, argc?"slave":"" );
	if(argc)
	{
		d=xdsl_get_op(1);
	}else
#endif /*CONFIG_USER_XDSL_SLAVE*/
	{
		d=xdsl_get_op(0);
	}

	return _adslToneDiagTbl(wp, d);
}


static int _vdslBandStatusTbl(request * wp, XDSL_OP *d)
{
	int nBytesSent=0;
#ifdef CONFIG_VDSL
	int mval=0;

	if(d->xdsl_msg_get(GetPmdMode,&mval))
	{
		if(mval&MODE_VDSL2)
		{
			int flag,i;
			VDSL2DiagOthers vd2other;

			memset( &vd2other, 0, sizeof(vd2other) );
			if( d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_OTHER, (void *)&vd2other, sizeof(vd2other))==0 )
			{
				flag=0;
				//printf( "%s:%d> RLCM_GET_VDSL2_DIAG_OTHER VDSL2 failed\n", __FUNCTION__, __LINE__ );
			}else{
				flag=1;
			}

#define FMT_BANDSTAT0 "<th width=9%% bgcolor=#c0c0c0><font size=2>%s%d</font></th>\n"
#define FMT_BANDSTAT1 "<th bgcolor=#c0c0c0><font size=2>%s</font></th>\n"
#define FMT_BANDSTAT2 "<td align=center bgcolor=#f0f0f0><font size=2>%g</font></td>\n"
#define FMT_BANDSTAT3 "<td align=center bgcolor=#f0f0f0></td>\n"

			nBytesSent += boaWrite(wp, "<table border=0 width=500 cellspacing=4 cellpadding=0>\n");
			nBytesSent += boaWrite(wp, "<tr>\n");
			nBytesSent += boaWrite(wp, "<th width=19%% bgcolor=#c0c0c0><font size=2>Band Status</font></th>\n");
			for(i=0;i<5;i++)
				nBytesSent += boaWrite(wp, FMT_BANDSTAT0, "U", i);
			for(i=0;i<4;i++)
				nBytesSent += boaWrite(wp, FMT_BANDSTAT0, "D", i+1);
			nBytesSent += boaWrite(wp, "</tr>\n");

			//LATN
			nBytesSent += boaWrite(wp, "<tr>\n");
			nBytesSent += boaWrite(wp, FMT_BANDSTAT1, "LATN");
			if(flag)
			{
				for(i=0;i<5;i++)
					nBytesSent += boaWrite(wp, FMT_BANDSTAT2, ((float)vd2other.LATNpbus[i])/10 );
				for(i=0;i<4;i++)
					nBytesSent += boaWrite(wp, FMT_BANDSTAT2, ((float)vd2other.LATNpbds[i])/10 );
			}else{
				for(i=0;i<9;i++)
					nBytesSent += boaWrite(wp, FMT_BANDSTAT3);
			}
			nBytesSent += boaWrite(wp, "</tr>\n");

			//SATN
			nBytesSent += boaWrite(wp, "<tr>\n");
			nBytesSent += boaWrite(wp, FMT_BANDSTAT1, "SATN");
			if(flag)
			{
				for(i=0;i<5;i++)
					nBytesSent += boaWrite(wp, FMT_BANDSTAT2, ((float)vd2other.SATNpbus[i])/10 );
				for(i=0;i<4;i++)
					nBytesSent += boaWrite(wp, FMT_BANDSTAT2, ((float)vd2other.SATNpbds[i])/10 );
			}else{
				for(i=0;i<9;i++)
					nBytesSent += boaWrite(wp, FMT_BANDSTAT3);
			}
			nBytesSent += boaWrite(wp, "</tr>\n");

			//SNRM
			nBytesSent += boaWrite(wp, "<tr>\n");
			nBytesSent += boaWrite(wp, FMT_BANDSTAT1, "SNRM");
			if(flag)
			{
				for(i=0;i<5;i++)
					nBytesSent += boaWrite(wp, FMT_BANDSTAT2, ((float)vd2other.SNRMpbus[i])/10 );
				for(i=0;i<4;i++)
					nBytesSent += boaWrite(wp, FMT_BANDSTAT2, ((float)vd2other.SNRMpbds[i])/10 );
			}else{
				for(i=0;i<9;i++)
					nBytesSent += boaWrite(wp, FMT_BANDSTAT3);
			}
			nBytesSent += boaWrite(wp, "</tr>\n");


			nBytesSent += boaWrite(wp, "</table>\n");
			nBytesSent += boaWrite(wp, "<p>\n");
		}
	}
#endif /*CONFIG_VDSL*/
	return nBytesSent;
}

int vdslBandStatusTbl(int eid, request * wp, int argc, char **argv)
{
	XDSL_OP *d;
#ifdef CONFIG_USER_XDSL_SLAVE
	//printf( "\n%s: %s\n", __FUNCTION__, argc?"slave":"" );
	if(argc)
	{
		d=xdsl_get_op(1);
	}else
#endif /*CONFIG_USER_XDSL_SLAVE*/
	{
		d=xdsl_get_op(0);
	}

	return _vdslBandStatusTbl(wp, d);
}

#ifdef CONFIG_VDSL
static int _adslToneDiagList(request * wp, XDSL_OP *d)
{
	char mode;
	int chan;
	int i;
	int nBytesSent=0;
	int ival, intp, fp;
	char str[16];
	int mval=0;
	int isVDSL2, vd2loop;
	ComplexShort *hlin;
	VDSL2DiagOthers vd2other;

	// get the channel number
	chan=256;
	isVDSL2=0;
	if(d->xdsl_msg_get(GetPmdMode,&mval))
	{
		if(mval&MODE_VDSL2)
			isVDSL2=1;
		else
			isVDSL2=0;

		if(mval<MODE_ADSL2PLUS)
			chan=256;
		else
			chan=512;
	}

	intp=fp=0;
	str[0] = 0;

	snr = malloc(sizeof(short)*MAX_DSL_TONE*2);
	qln = malloc(sizeof(short)*MAX_DSL_TONE*2);
	hlog = malloc(sizeof(short)*MAX_DSL_TONE*2);
	hlin = malloc(sizeof(ComplexShort)*MAX_DSL_TONE*2);
	if( !snr || !qln || !hlog || !hlin )
	{
		printf( "%s:%d>malloc failed, snr=%x, qln=%x, hlog=%x, hlin=%x\n",
			__FUNCTION__, __LINE__, snr, qln, hlog, hlin );
		diagflag = 0;
	}else{
		//reset
		memset( hlog, 0,  sizeof(short)*MAX_DSL_TONE*2 );
		memset( snr, 0,  sizeof(short)*MAX_DSL_TONE*2 );
		memset( qln, 0,  sizeof(short)*MAX_DSL_TONE*2 );
		memset( hlin, 0,  sizeof(ComplexShort)*MAX_DSL_TONE*2 );
		memset( &vd2other, 0, sizeof(vd2other) );

		if( d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_HLOG, (void *)hlog, sizeof(short)*MAX_DSL_TONE*2))
		{
			d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_SNR, (void *)snr, sizeof(short)*MAX_DSL_TONE*2);
			d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_QLN, (void *)qln, sizeof(short)*MAX_DSL_TONE*2);
			d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_HLIN, (void *)hlin, sizeof(ComplexShort)*MAX_DSL_TONE*2);
			d->xdsl_drv_get(RLCM_GET_VDSL2_DIAG_OTHER, (void *)&vd2other, sizeof(vd2other));
			diagflag = 1;
		}else{
			diagflag = 0;
			//printf( "%s:%d> RLCM_GET_VDSL2_DIAG_HLOG VDSL2 failed\n", __FUNCTION__, __LINE__ );
		}
	}

	for(vd2loop=0; vd2loop<=isVDSL2; vd2loop++)
	{
		int offset;

		if(vd2loop==1)
		{
			offset=MAX_DSL_TONE;
			nBytesSent += boaWrite(wp, "<tr></tr><tr></tr>\n");
			nBytesSent += boaWrite(wp, "<tr><th bgcolor=#c0c0c0 colspan=6><font size=2>Downstream (Group Number=%u)</font></th></tr>\n", vd2other.SNRGds);
		}else{ //vd2loop=0
			offset=0;
			if(isVDSL2) nBytesSent += boaWrite(wp, "<tr><th bgcolor=#c0c0c0 colspan=6><font size=2>Upstream (Group Number=%u)</font></th></tr>\n", vd2other.SNRGus);
		}

		nBytesSent += boaWrite(wp, "<tr>\n<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang(LANG_TONE_NUMBER));
		nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang(LANG_H_REAL));
		nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang(LANG_H_IMAGE));
		nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang(LANG_SNR));
		nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang(LANG_QLN));
		nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n</tr>\n", multilang(LANG_HLOG));

		for (i = 0; i < chan; i++)
		{
			nBytesSent += boaWrite(wp, "<tr>\n<th bgcolor=#c0c0c0><font size=2>%d</font></th>\n", i);

			// H.Real
			if (diagflag) {
				intp = hlin[offset+i].real/1000;
				fp = hlin[offset+i].real%1000;
				if (fp<0) {
					fp = -fp;
					if (intp == 0)
						snprintf(str, 16, "-0.%03d", fp);
					else
						snprintf(str, 16, "%d.%03d", intp, fp);
				}
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n", str);

			// H.Image
			if (diagflag) {
				intp = hlin[offset+i].imag/1000;
				fp = hlin[offset+i].imag%1000;
				if (fp<0) {
					fp = -fp;
					if (intp == 0)
						snprintf(str, 16, "-0.%03d", fp);
					else
						snprintf(str, 16, "%d.%03d", intp, fp);
				}
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n", str);

			// snr
			if (diagflag) {
				intp = snr[offset+i]/10;
				fp = abs(snr[offset+i]%10);
				snprintf(str, 16, "%d.%d", intp, fp);
			}
			nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n", str);

			// qln
			if (diagflag) {
				intp = qln[offset+i]/10;
				fp = qln[offset+i]%10;
				if (fp<0) {
					if (intp != 0)
						snprintf(str, 16, "%d.%d", intp, -fp);
					else
						snprintf(str, 16, "-%d.%d", intp, -fp);
				}
				else
					snprintf(str, 16, "%d.%d", intp, fp);
			}
			nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n", str);

			// hlog
			if (diagflag) {
				intp = hlog[offset+i]/10;
				fp = hlog[offset+i]%10;
				if (fp<0) {
					if (intp != 0)
						snprintf(str, 16, "%d.%d", intp, -fp);
					else
						snprintf(str, 16, "-%d.%d", intp, -fp);
				}
				else
					snprintf(str, 16, "%d.%d", intp, fp);
			}
			nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n</tr>\n", str);
		}
	}

	if(snr) free(snr);
	if(qln) free(qln);
	if(hlog) free(hlog);
	if(hlin) free(hlin);

	return nBytesSent;
}
#else
static int _adslToneDiagList(request * wp, XDSL_OP *d)
{
	char mode;
	static int chan=256;
	int i;
	int nBytesSent=0;
	int ival, intp, fp;
	char str[16];

	// get the channel number
	if(d->xdsl_drv_get(RLCM_GET_SHOWTIME_XDSL_MODE, (void *)&mode, 1)) {
		//ramen to clear the first 3 bit.
		mode&=0x1f;
		if (mode < 5) //adsl1/adsl2
			chan = 256;
		else
			chan = 512;
	}

	intp=fp=0;
	str[0] = 0;

	/*
	snr = malloc(sizeof(short)*chan);
	qln = malloc(sizeof(short)*chan);
	hlog = malloc(sizeof(short)*(chan*3+HLOG_ADDITIONAL_SIZE));
	*/
	snr = malloc(sizeof(short)*MAX_DSL_TONE);
	qln = malloc(sizeof(short)*MAX_DSL_TONE);
	hlog = malloc(sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE));

	/*
	if (adsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)hlog, sizeof(short)*(chan*3+HLOG_ADDITIONAL_SIZE))) {
		adsl_drv_get(RLCM_GET_DIAG_SNR, (void *)snr, sizeof(short)*chan);
		adsl_drv_get(RLCM_GET_DIAG_QLN, (void *)qln, sizeof(short)*chan);
		diagflag = 1;
	}
	*/
	if (d->xdsl_drv_get(RLCM_GET_DIAG_HLOG, (void *)hlog, sizeof(short)*(MAX_DSL_TONE*3+HLOG_ADDITIONAL_SIZE))) {
		d->xdsl_drv_get(RLCM_GET_DIAG_SNR, (void *)snr, sizeof(short)*MAX_DSL_TONE);
		d->xdsl_drv_get(RLCM_GET_DIAG_QLN, (void *)qln, sizeof(short)*MAX_DSL_TONE);
		diagflag = 1;
	}
	else
		diagflag = 0;

	nBytesSent += boaWrite(wp, "<tr>\n<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang(LANG_TONE_NUMBER));
	nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang(LANG_H_REAL));
	nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang(LANG_H_IMAGE));
	nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang(LANG_SNR));
	nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang(LANG_QLN));
	nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n</tr>\n", multilang(LANG_HLOG));

	for (i = 0; i < chan; i++) {

		nBytesSent += boaWrite(wp, "<tr>\n<th bgcolor=#c0c0c0><font size=2>%d</font></th>\n", i);
		// H.Real = H.Real*Hlin.Scale/32768
		if (diagflag) {
			/*
			if (i <= 31) { // upstream
				fp=(hlog[i+chan]*hlog[chan*3]*100)/32768;
			}
			else { // downstream
				fp=(hlog[i+chan]*hlog[chan*3+1]*100)/32768;
			}
			*/
			intp = hlog[i+chan]/1000;
			fp = hlog[i+chan]%1000;
			if (fp<0) {
				fp = -fp;
				if (intp == 0)
					snprintf(str, 16, "-0.%03d", fp);
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			else
				snprintf(str, 16, "%d.%03d", intp, fp);
		}

		nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n", str);
		// H.Image = H.Image*Hlin.Scale/32768
		if (diagflag) {
			/*
			if (i <= 31) { // upstream
				fp=(hlog[i+chan*2]*hlog[chan*3]*100)/32768;
			}
			else { // downstream
				fp=(hlog[i+chan*2]*hlog[chan*3+1]*100)/32768;
			}
			*/
			intp = hlog[i+chan*2]/1000;
			fp = hlog[i+chan*2]%1000;
			if (fp<0) {
				fp = -fp;
				if (intp == 0)
					snprintf(str, 16, "-0.%03d", fp);
				else
					snprintf(str, 16, "%d.%03d", intp, fp);
			}
			else
				snprintf(str, 16, "%d.%03d", intp, fp);
		}

		nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n", str);
		//nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0>%d</td>\n", 0);
		// snr = -32+(snr/2)
		if (diagflag) {
			/*
			ival = snr[i] * 5; // *10/2
			intp = ival/10-32;
			fp = ival % 10;
			*/
			intp = snr[i]/10;
			fp = abs(snr[i]%10);
			snprintf(str, 16, "%d.%d", intp, fp);
		}
		nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n", str);
		// qln = -23-(qln/2)
		if (diagflag) {
			/*
			ival = qln[i] * 5;
			intp = ival/10+23;
			fp = ival % 10;
			*/
			intp = qln[i]/10;
			fp = qln[i]%10;
			if (fp<0) {
				if (intp != 0)
					snprintf(str, 16, "%d.%d", intp, -fp);
				else
					snprintf(str, 16, "-%d.%d", intp, -fp);
			}
			else
				snprintf(str, 16, "%d.%d", intp, fp);
		}
		nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n", str);
		// hlog = 6-(hlog/10)
		if (diagflag) {
			/*
			ival = hlog[i]/10;
			if (ival >= 6) {// negative value
				intp = ival - 6;
				fp = hlog[i] % 10;
				nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0>-%d.%d</td>\n</tr>\n", intp, fp);
			}
			else { //positive value
				intp = 6- ival;
				ival = hlog[i] % 10;
				if (ival != 0)
					intp--;
				fp = 10 - ival;
				nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0>%d.%d</td>\n</tr>\n", intp, fp);
			}
			*/
			intp = hlog[i]/10;
			fp = hlog[i]%10;
			if (fp<0) {
				if (intp != 0)
					snprintf(str, 16, "%d.%d", intp, -fp);
				else
					snprintf(str, 16, "-%d.%d", intp, -fp);
			}
			else
				snprintf(str, 16, "%d.%d", intp, fp);
		}
		/*
		else
			nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0>%d.%d</td>\n</tr>\n", intp, fp);
		*/
		nBytesSent += boaWrite(wp, "<td align=center bgcolor=#f0f0f0><font size=2>%s</font></td>\n</tr>\n", str);
	}

	free(snr);
	free(qln);
	free(hlog);

	return nBytesSent;
}
#endif /*CONFIG_VDSL*/

int adslToneDiagList(int eid, request * wp, int argc, char **argv)
{
	XDSL_OP *d;
#ifdef CONFIG_USER_XDSL_SLAVE
	//printf( "\n%s: %s\n", __FUNCTION__, argc?"slave":"" );
	if(argc)
	{
		d=xdsl_get_op(1);
	}else
#endif /*CONFIG_USER_XDSL_SLAVE*/
	{
		d=xdsl_get_op(0);
	}

	return _adslToneDiagList(wp, d);
}

int adslToneConfDiagList(int eid, request * wp, int argc, char **argv)
{
	short mode;
	int i, chan;
	int nBytesSent=0;
	unsigned char tone[64];    // Added by Mason Yu for correct Tone Mib Type
	int onbit;


	memset(tone, 0, sizeof(tone));

	// get the channel number
	mib_get(MIB_ADSL_MODE, (void *)&mode);
	if (mode & ADSL_MODE_ADSL2P)
		chan = 512;	// ADSL2+
	else
		chan = 256;	// ADSL, ADSL2

	if ( !mib_get(MIB_ADSL_TONE, (void *)tone)) {
		printf(strGetToneerror);
	}

	nBytesSent += boaWrite(wp, "<tr>\n<th width=25%% bgcolor=#c0c0c0>%s</th>\n",
			multilang(LANG_TONE_NUMBER));
	nBytesSent += boaWrite(wp, "<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
			multilang(LANG_SELECT));

	for (i = 0; i < chan; i++) {
		//onbit =(tone[i/8] >> (i%8) ) & 0x01;
		onbit =(tone[i/8] >> (7-(i%8)) ) & 0x01;

		nBytesSent += boaWrite(wp, "<tr>\n<th bgcolor=#c0c0c0>%d</th>\n", i);
		if (onbit == 1)
			nBytesSent += boaWrite(wp, "<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\" checked></td></tr>\n", i);
		else
			nBytesSent += boaWrite(wp, "<td align=center width=\"20%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"select%d\" value=\"ON\"></td></tr>\n", i);

	}

	return nBytesSent;
}

int adslPSDMaskTbl(int eid, request * wp, int argc, char **argv)
{
	int i;
	int nBytesSent=0;
	char *strChecked;
	char strTone[16];
	char strUS[16];


	/* send header */
	nBytesSent += boaWrite(wp, "<tr>\n<th align=center width=\"10%%\" bgcolor=\"#B0B0B0\">%s</th>\n", multilang(LANG_ENABLE));
	nBytesSent += boaWrite(wp, "<th align=center width=\"10%%\" bgcolor=\"#B0B0B0\">%s (0-63)</th>\n", multilang(LANG_TONE));
	nBytesSent += boaWrite(wp, "<th align=center width=\"70%%\" bgcolor=\"#B0B0B0\">dBm/Hz</th></tr>\n");

	for (i = 0; i < 8; i++) {

		if (psd_bit_en & (1<<i)) {
			strChecked = "checked";
		} else {
			strChecked = "";
		}
		snprintf(strTone, sizeof(strTone), "value=%d", psd_tone[i]);
		snprintf(strUS, sizeof(strUS), "value=%.1f", psd_us[i]);

		nBytesSent += boaWrite(wp, "<tr><td align=center width=\"10%%\" bgcolor=\"#C0C0C0\"><input type=\"checkbox\" name=\"s%d\" value=\"1\" %s></td>\n",  i, strChecked);
		nBytesSent += boaWrite(wp, "<td align=center width=\"30%%\" bgcolor=\"#C0C0C0\"><input type=\"text\" maxLength=2 size=\"4\" name=\"t%d\" %s></td>\n", i, strTone);
		nBytesSent += boaWrite(wp, "<td align=center width=\"60%%\" bgcolor=\"#C0C0C0\"><input type=\"text\" size=\"8\" name=\"f%d\" %s></td></tr>\n", i, strUS);
	}

	return nBytesSent;
}

void formSetAdslPSD(request * wp, char *path, char *query)
{
	char *submitUrl;
	int i;
	char tmpBuf[100];
	unsigned char bits = 0;
	int tone = 0;
	float us;
	UsPSDData PSD;

	char *strVal=0, *strApply;

	strApply = boaGetVar(wp, "apply", "");

	/* user clicked apply*/
	if (strApply[0]) {
		for (i=0; i<8; i++) {
			snprintf(tmpBuf, sizeof(tmpBuf), "s%d", i);
			strVal = boaGetVar(wp, tmpBuf, "");

			if (strVal && !gstrcmp(strVal, "1") ) {
				if (tone < 0) {
					snprintf(tmpBuf, sizeof(tmpBuf), multilang(LANG_S_SHOULD_BE_0_63), strVal);
					goto setErr_tone;
				}

				bits |= (1<<i);
			}

			tone = -1;

			snprintf(tmpBuf, sizeof(tmpBuf), "t%d", i);
			strVal = boaGetVar(wp, tmpBuf, "");
			if ((bits & (1 << i)) && strVal) {
				if (1 != sscanf(strVal, "%d", &tone)) {
					snprintf(tmpBuf, sizeof(tmpBuf), multilang(LANG_TD_S_NOT_A_NUMBER), i, strVal);
					goto setErr_tone;
				}

				if ((tone > 63) || (tone < 0)) {
					snprintf(tmpBuf, sizeof(tmpBuf), multilang(LANG_S_SHOULD_BE_0_63), strVal);
					goto setErr_tone;
				}

				psd_tone[i] = tone;
			}

			if (!(bits & (1 << i)))
				continue;

			snprintf(tmpBuf, sizeof(tmpBuf), "f%d", i);
			strVal = boaGetVar(wp, tmpBuf, "");
			if (strVal && (sscanf(strVal, "%f", &us)==1)) {
				psd_us[i] = us;
			}

		}

		psd_bit_en = bits;
		goto setOk_tone;
	}

setOk_tone:

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	/*
	fprintf(stderr, "psd_bit_en: %x  psd_measure: %d\n", psd_bit_en, psd_measure);
	for (i=0;i<8;i++) {
		fprintf(stderr, "psd_tone[%d]=%d, psd_us[%d]=%.1f\n", i,psd_tone[i],i,psd_us[i]);
	}
	*/
	#if SUPPORT_TR105
	for (i=0;i<8;i++) {
		if (bits & (1 << i)) {
			PSD.breakFreq_array_us[i] = psd_tone[i];
			PSD.MIB_PSD_us[i] = psd_us[i];
		} else {
			PSD.breakFreq_array_us[i] = 513; // means disabled.
		}
	}
	if (adsl_drv_get(RLCM_WEB_SET_USPSD, (void *)&PSD, sizeof(PSD))) {
	}
	#endif

	OK_MSG(submitUrl);

  	return;

setErr_tone:
	ERR_MSG(tmpBuf);
}

int adslPSDMeasure(int eid, request * wp, int argc, char **argv) {
	int nBytesSent=0;
	char *Value;

	Value = multilang((psd_measure) ? LANG_DISABLE : LANG_ENABLE);

	nBytesSent += boaWrite(wp, "<input type=submit value=\"%s\" name=\"psdm\">", Value);
	return nBytesSent;
}


//cathy
static void _DSLuptime(request * wp, XDSL_OP *d)
{
	Modem_LinkSpeed vLs;
	unsigned char adslflag;
	struct sysinfo info, *up;
	int updays, uphours, upminutes, upsec;

	// check for xDSL link
	if (!d->xdsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
		adslflag = 0;
	else
		adslflag = 1;

	if( adslflag ) {

		#ifdef CONFIG_USER_XDSL_SLAVE
		if(d->id)
			up = updateSlvLinkTime(0);
		else
		#endif /*CONFIG_USER_XDSL_SLAVE*/
			up = updateLinkTime(0);

		sysinfo(&info);
		info.uptime = info.uptime - up->uptime;
		updays = (int) info.uptime / (60*60*24);
		if (updays)
			boaWrite (wp, "%d day%s,&nbsp;", updays, (updays != 1) ? "s" : "");
		upsec = (int) info.uptime % 60;
		upminutes = (int) info.uptime / 60;
		uphours = (upminutes / 60) % 24;
		upminutes %= 60;
		boaWrite (wp, "%02d:%02d:%02d\n", uphours, upminutes, upsec);
	}
	else {
		boaWrite(wp, "&nbsp;");
	}
}

void DSLuptime(int eid, request * wp, int argc, char **argv)
{
	_DSLuptime(wp, xdsl_get_op(0) );
}

#ifdef CONFIG_USER_XDSL_SLAVE
void DSLSlvuptime(int eid, request * wp, int argc, char **argv)
{
	_DSLuptime(wp, xdsl_get_op(1) );
}
#endif /*CONFIG_USER_XDSL_SLAVE*/
#endif // of CONFIG_DEV_xDSL


/////////////////////////////////////////////////////////////////////////////
void formStats(request * wp, char *path, char *query)
{
	char *strValue, *submitUrl;
	#ifdef CONFIG_RTK_RG_INIT
	int i;
	#endif
	strValue = boaGetVar(wp, "reset", "");	//cathy, reset stats
	if(strValue[0]) {
		if(strValue[0] - '0') {	//reset
#ifdef EMBED
		system("echo all > /proc/realtek/netdev_reset");
		#ifdef CONFIG_RTK_RG_INIT
		for (i = 0; i < ELANVIF_NUM; i++) {
			RG_clear_portCounter(i);
		}
		#endif
#ifdef WLAN_QTN
		va_cmd("/bin/qcsapi_sockrpc", 4, 1, "reset_all_stats", "wifi0", "0", "0");
#endif
#endif
#ifdef CONFIG_SFU
		rtk_stat_init();
#endif
		}
	}

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
}

// List the packet statistics for all interfaces at web page.
int pktStatsList(int eid, request * wp, int argc, char **argv)
{
	//ql_xu add: for auto generate wan name
	int entryNum;
	int mibcnt;
	MIB_CE_ATM_VC_T Entry;
	int i;

	char *names[16];
	char ifname[IFNAMSIZ];
	char ifDisplayName[IFNAMSIZ];
	int num_itf;
	struct net_device_stats nds;

	num_itf=0;
	for (i=0; i<ELANVIF_NUM; i++) {
		names[i] = (char *)ELANVIF[i];
		num_itf++;
	}
#ifdef CONFIG_USB_ETH
	names[num_itf++] = (char *)USBETHIF;
#endif
#ifdef WLAN_SUPPORT
	names[num_itf++] = "wlan0";
#ifdef WLAN_QTN
	names[num_itf++] = "wlan1";
#endif
#endif

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">%s</td></font></tr>\n",
	multilang(LANG_INTERFACE), multilang(LANG_RX_PKT), multilang(LANG_RX_ERR), multilang(LANG_RX_DROP),
	multilang(LANG_TX_PKT), multilang(LANG_TX_ERR), multilang(LANG_TX_DROP));

	// LAN statistics
	#ifndef CONFIG_RTK_RG_INIT
	#ifdef CONFIG_SFU
	rtk_stat_port_cntr_t portcnt;
	rtk_stat_port_getAll(0, &portcnt);
	boaWrite(wp, "<tr>"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
			"LAN",
			 (portcnt.ifInUcastPkts + portcnt.ifInMulticastPkts + portcnt.ifInBroadcastPkts), 
			 (portcnt.dot3StatsSymbolErrors + portcnt.dot3ControlInUnknownOpcodes), 
			 portcnt.dot1dTpPortInDiscards,
			(portcnt.ifOutUcastPkts + portcnt.ifOutMulticastPkts + portcnt.ifOutBrocastPkts), 
			0,
			portcnt.ifOutDiscards);

	#else
	for (i = 0; i < num_itf; i++) {
		get_net_device_stats(names[i], &nds);

		boaWrite(wp, "<tr>"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
			names[i],
			nds.rx_packets, nds.rx_errors, nds.rx_dropped,
			nds.tx_packets, nds.tx_errors, nds.tx_dropped);
	}
	#endif//!CONFIG_SFU
	#else // use rg api to retrive switch level packet counter
	for(i = 0 ; i < num_itf; i ++)
	{
		unsigned long tx_pkts,tx_drops,tx_errs,rx_pkts,rx_drops,rx_errs;

		if(!strncmp(names[i],"wlan",4) || ! strncmp(names[i],"usb",3)) // wlan or usb interface
		{
#ifdef WLAN_QTN
#ifdef WLAN1_QTN
		if(!strncmp(names[i],"wlan1",5))
			
#else
		if(!strncmp(names[i],"wlan0",5))
#endif
			rt_qcsqpi_get_net_device_stats("wifi0", &nds);
		else
#endif
			get_net_device_stats(names[i], &nds);
			rx_pkts = nds.rx_packets;
			rx_errs = nds.rx_errors;
			rx_drops = nds.rx_dropped;
			tx_pkts = nds.tx_packets;
			rx_errs = nds.tx_errors;
			tx_drops = nds.tx_dropped;
		}
		else
		{
			if(RG_get_portCounter(i,&tx_pkts,&tx_drops,&tx_errs,&rx_pkts,&rx_drops,&rx_errs) == 0 )
			// get fail , assign all counter to 0
				tx_pkts = tx_drops = tx_errs = rx_pkts = rx_drops = rx_errs = 0;	
		}
		
		boaWrite(wp, "<tr>"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
			names[i],
			rx_pkts, rx_errs, rx_drops,
			tx_pkts, tx_errs, tx_drops);
		}
	#endif

#ifdef EMBED
#ifdef CONFIG_RTL8672_SAR
if(WAN_MODE & MODE_ATM)
{
	for (mibcnt = 0; mibcnt < entryNum; mibcnt++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, &Entry)) {
			printf("MULTI_ATM_WAN: get MIB chain error\n");
			continue;
		}

		if (Entry.enable == 0)
			continue;

		if(MEDIA_INDEX(Entry.ifIndex) != MEDIA_ATM)
			continue;

		//generate wan name
		ifGetName(Entry.ifIndex, ifname, sizeof(ifname));

		get_net_device_stats(ifname, &nds);
		getDisplayWanName(&Entry, ifDisplayName);

		boaWrite(wp, "<tr>"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
			ifDisplayName, nds.rx_packets,
			nds.rx_errors, nds.rx_dropped,
			nds.tx_packets, nds.tx_errors,
			nds.tx_dropped);
	}
}
#endif	// CONFIG_RTL8672_SAR

#ifdef CONFIG_PTMWAN
if(WAN_MODE & MODE_PTM)
{
	for (mibcnt = 0; mibcnt < entryNum; mibcnt++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, &Entry)) {
			printf("MULTI_PTM_WAN: get MIB chain error\n");
			continue;
		}

		if (Entry.enable == 0)
			continue;

		if(MEDIA_INDEX(Entry.ifIndex) != MEDIA_PTM)
			continue;

		//generate wan name
		ifGetName(Entry.ifIndex, ifname, sizeof(ifname));

		get_net_device_stats(ifname, &nds);
		getDisplayWanName(&Entry, ifDisplayName);

		boaWrite(wp, "<tr>"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
			ifDisplayName, nds.rx_packets,
			nds.rx_errors, nds.rx_dropped,
			nds.tx_packets, nds.tx_errors,
			nds.tx_dropped);
	}
}
#endif	// CONFIG_PTMWAN


#if defined(CONFIG_ETHWAN) && !defined(CONFIG_SFU) 
if(WAN_MODE & MODE_Ethernet)
{
	for (mibcnt = 0; mibcnt < entryNum; mibcnt++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, &Entry)) {
			printf("MULTI_ETH_WAN: get MIB chain error\n");
			continue;
		}

		if (Entry.enable == 0)
			continue;

		if(MEDIA_INDEX(Entry.ifIndex) != MEDIA_ETH)
			continue;

		//generate wan name
		ifGetName(Entry.ifIndex, ifname, sizeof(ifname));

		get_net_device_stats(ifname, &nds);
		getDisplayWanName(&Entry, ifDisplayName);

		boaWrite(wp, "<tr>"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
			ifDisplayName, nds.rx_packets,
			nds.rx_errors, nds.rx_dropped,
			nds.tx_packets, nds.tx_errors,
			nds.tx_dropped);
	}
}
#endif	// CONFIG_ETHWAN
#endif	// EMBED

#ifdef CONFIG_USER_PPPOMODEM
	{
		MIB_WAN_3G_T entry, *p;

		p = &entry;
		if (mib_chain_get(MIB_WAN_3G_TBL, 0, p)) {
			if (p->enable) {
				sprintf(ifname, "ppp%d", MODEM_PPPIDX_FROM);
				get_net_device_stats(ifname, &nds);

				boaWrite(wp, "<tr>"
					"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>3G_%s</b></font></td>\n"
					"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
					"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
					"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
					"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
					"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
					"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
					ifname, nds.rx_packets,
					nds.rx_errors, nds.rx_dropped,
					nds.tx_packets, nds.tx_errors,
					nds.tx_dropped);
			}
		}
	}
#endif //CONFIG_USER_PPPOMODEM

#ifdef CONFIG_USER_PPTP_CLIENT_PPTP
	{
		MIB_PPTP_T Entry;
		unsigned int entryNum, i;

		entryNum = mib_chain_total(MIB_PPTP_TBL);
		for (i=0; i<entryNum; i++)
		{
			char mppp_ifname[IFNAMSIZ];

			if (!mib_chain_get(MIB_PPTP_TBL, i, (void *)&Entry))
			{
				printf("Get MIB_PPTP_TBL chain record error!\n");
				continue;
			}

			ifGetName(Entry.ifIndex, mppp_ifname, sizeof(mppp_ifname));
			snprintf(ifname, 15, "%s_pptp%d", mppp_ifname, Entry.idx);
			get_net_device_stats(mppp_ifname, &nds);

			boaWrite(wp, "<tr>"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
				ifname, nds.rx_packets,
				nds.rx_errors, nds.rx_dropped,
				nds.tx_packets, nds.tx_errors,
				nds.tx_dropped);
		}
	}
#endif

#ifdef CONFIG_USER_L2TPD_L2TPD
	{
		MIB_L2TP_T Entry;
		unsigned int entryNum, i;

		entryNum = mib_chain_total(MIB_L2TP_TBL);
		for (i=0; i<entryNum; i++)
		{
			char mppp_ifname[IFNAMSIZ];

			if (!mib_chain_get(MIB_L2TP_TBL, i, (void *)&Entry))
			{
				printf("Get MIB_L2TP_TBL chain record error!\n");
				continue;
			}

			ifGetName(Entry.ifIndex, mppp_ifname, sizeof(mppp_ifname));
			snprintf(ifname, 15, "%s_l2tp%d", mppp_ifname, Entry.idx);
			get_net_device_stats(mppp_ifname, &nds);

			boaWrite(wp, "<tr>"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
				ifname, nds.rx_packets,
				nds.rx_errors, nds.rx_dropped,
				nds.tx_packets, nds.tx_errors,
				nds.tx_dropped);
		}
	}
#endif

#ifdef CONFIG_NET_IPIP
	{
		MIB_IPIP_T Entry;
		unsigned int entryNum, i;

		entryNum = mib_chain_total(MIB_IPIP_TBL);
		for (i=0; i<entryNum; i++)
		{
			char ifname[IFNAMSIZ];

			if (!mib_chain_get(MIB_IPIP_TBL, i, (void *)&Entry))
			{
				printf("Get MIB_IPIP_TBL chain record error!\n");
				continue;
			}

			ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
			get_net_device_stats(ifname, &nds);

			boaWrite(wp, "<tr>"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
				ifname, nds.rx_packets,
				nds.rx_errors, nds.rx_dropped,
				nds.tx_packets, nds.tx_errors,
				nds.tx_dropped);
		}
	}
#endif

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
void formRconfig(request * wp, char *path, char *query)
{
	char *strValue, *strSubmit, *uid, *upwd;
	char tmpBuf[100];
	char ipaddr[20], dport[10];
	char userName[MAX_NAME_LEN];

	strSubmit = boaGetVar(wp, "yes", "");
	mib_get(MIB_ADSL_LAN_IP, (void *)tmpBuf);
	strncpy(ipaddr, inet_ntoa(*((struct in_addr *)tmpBuf)), 16);
	ipaddr[15] = '\0';
	snprintf(ipaddr, 20, "%s:80", ipaddr);

	// enable
	if (strSubmit[0]) {
		mib_get( MIB_USER_NAME, (void *)userName );
		if (userName[0] == '\0') {
			strcpy(tmpBuf, multilang(LANG_PROHIBIT_ADMINISTRATORS_PASSWORD_NOT_SET));
			goto setErr_rconf;
		}

		strValue = boaGetVar(wp, "writable", "");
		if (strValue[0])	// allow update
			g_remoteUpdate = TRUE;
		else	// read only
			g_remoteUpdate = FALSE;

		strValue = boaGetVar(wp, "portFlag", "");
		if (strValue[0]) {	// default port 51003
			g_remoteAccessPort = 51003;
		}
		else {	// use randomly selected port
			if (!srandomCalled) {
				srandom(time(0));
				srandomCalled = 1;
			}
			g_remoteAccessPort = 50000 + (random()&0x00000fff);
		}

		sprintf(dport, "%d", g_remoteAccessPort);

		uid = boaGetVar(wp, "uid", "");
		upwd = boaGetVar(wp, "pwd", "");
		if (uid[0] != '\0' && upwd[0] != '\0') {
			/* Create user */
			if ( boaAddUser(uid, upwd, DEFAULT_GROUP, FALSE, FALSE) ) {
				error(E_L, E_LOG, "ERROR: Unable to add user account.");
				return;
			}
			else {
				strcpy(g_rUserName, uid);
				strcpy(g_rUserPass, upwd);
				// The remote access session MUST be started
				// within REMOTE_PASS_LIFE seconds.
				g_rexpire = time(0) + REMOTE_PASS_LIFE;
				g_rSessionStart = FALSE;
			}
		}
		else {
			g_rUserName[0] = '\0';
			g_rUserPass[0] = '\0';
		}

		// iptables -D INPUT -j block
		va_cmd(IPTABLES, 4, 1, (char *)FW_DEL, (char *)FW_INPUT, "-j", "block");
		// iptables -A INPUT -i ! br0 -p TCP --dport 80 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, (char *)FW_INPUT, "!", ARG_I, LANIF, "-p", ARG_TCP, FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
		// iptables -A INPUT -j block
		va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", "block");

		// iptables -t nat -A PREROUTING -i ! $LAN_IF -p TCP --dport 51003 -j DNAT --to-destination ipaddr:80
		va_cmd(IPTABLES, 15, 1, "-t", "nat", (char *)FW_ADD,	"PREROUTING", "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP, (char *)FW_DPORT,
		dport, "-j", "DNAT", "--to-destination", ipaddr);
		g_remoteConfig = 1;
	}

	strSubmit = boaGetVar(wp, "no", "");
	// disable
	if (strSubmit[0]) {
		sprintf(dport, "%d", g_remoteAccessPort);

		// delete original user
		if (g_rUserName[0]) {
			if ( boaDeleteUser(g_rUserName) ) {
				printf("ERROR: Unable to delete user account (user=%s).\n", g_rUserName);
				return;
			}
			g_rUserName[0] = '\0';
		}

		// iptables -D INPUT -i ! br0 -p TCP --dport 80 -j ACCEPT
		va_cmd(IPTABLES, 11, 1, (char *)FW_DEL, (char *)FW_INPUT, "!", ARG_I, LANIF, "-p", ARG_TCP, FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
		// iptables -t nat -D PREROUTING -i ! $LAN_IF -p TCP --dport 51003 -j DNAT --to-destination ipaddr:80
		va_cmd(IPTABLES, 15, 1, "-t", "nat", (char *)FW_DEL, "PREROUTING", "!", (char *)ARG_I, (char *)LANIF, "-p", (char *)ARG_TCP, (char *)FW_DPORT,
		dport, "-j", "DNAT", "--to-destination", ipaddr);
		g_remoteConfig = 0;
		g_remoteUpdate = FALSE;
	}

	strSubmit = boaGetVar(wp, "submit-url", "");   // hidden page
	boaRedirect(wp, strSubmit);
	return;

setErr_rconf:
	ERR_MSG(tmpBuf);
}

//for rc4 encryption
/****************************************/
/* rc4_encrypt()                        */
/****************************************/
#define RC4_INT unsigned int
#define MAX_MESSAGE_LENGTH 2048
unsigned char en_cipherstream[MAX_MESSAGE_LENGTH+1];
void xor_block(int length, unsigned char *a, unsigned char *b, unsigned char *out)
{
    int i;
    for (i=0;i<length; i++)
    {
        out[i] = a[i] ^ b[i];
    }
}
static __inline__ void swap(unsigned char *a, unsigned char *b)
{
    unsigned char tmp;

    tmp = *a;
    *a = *b;
    *b = tmp;
}
void rc4(
            unsigned char *key,
            int key_length,
            int cipherstream_length,
            unsigned char *cipherstream)
{
    int i, j, x;

    unsigned char s[256];
    unsigned char k[256];

    /* Create Key Stream */
    for (i=0; i<256; i++)
        k[i] = key[i % key_length];

    /* Initialize SBOX */
    for (j=0; j<256; j++)
        s[j] = j;

    /* Seed the SBOX */
    i = 0;
    for (j=0; j<256; j++)
    {
        i = (i + s[j] + k[j]) & 255;
        swap(&s[j], &s[i]);
    }

    /* Generate the cipherstream */
    j = 0;
    i = 0;

    for (x=0; x<cipherstream_length; x++)
    {
        j = (j + 1) & 255;
        i = (i + s[j]) & 255;
        swap(&s[j], &s[i]);
        cipherstream[x] = s[(s[j] + s[i]) & 255];
    };
}

void rc4_encrypt(
            unsigned char *key,
            int key_length,
            unsigned char *data,
            int data_length,
            unsigned char *ciphertext)
{

    rc4(key, key_length, data_length, en_cipherstream);
    xor_block(data_length, en_cipherstream, data, ciphertext);
}

int rc4_encry()
{
	// for rc4 ecncryption
	FILE *fp_in, *fp_out;
	char ciphertext[256];
	unsigned char rc4_key[256];
	int key_length;
	char LINE[256];
	int data_len;

	rc4_key[0] = 0;
	strcpy(rc4_key, "realteksd5adsl");
	key_length = strlen(rc4_key);
	rc4_key[key_length] = '\0';

	fp_in = fopen("/var/log/messages", "r");
	fp_out = fopen("/tmp/log.txt", "w");
	while (!feof(fp_in)) {
		data_len = fread(LINE, sizeof(char), 255, fp_in);
		LINE[data_len] = 0;
		//encryption
		rc4_encrypt(rc4_key, key_length, LINE, data_len, ciphertext);
		ciphertext[data_len] = 0;
		fwrite(ciphertext, sizeof(char), data_len, fp_out);
	}
	fclose(fp_out);
	fclose(fp_in);

	return 0;
}


#define _PATH_SYSCMD_LOG "/tmp/syscmd.log"
unsigned char first_time=1;
unsigned char dbg_enable=0;
void formSysCmd(request * wp, char *path, char *query)
{
	char  *submitUrl, *sysCmd,*strVal,*strRequest;
	unsigned char adsldbg;
	unsigned int maxFileSector;
//#ifndef NO_ACTION
	char tmpBuf[100];
//#endif

	if(first_time==1){
		mib_get(MIB_ADSL_DEBUG, (void *)&adsldbg);
		if(adsldbg==1)
			dbg_enable=1;
		first_time=0;
	}
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	sysCmd = boaGetVar(wp, "sysCmd", "");   // hidden page

	strRequest = boaGetVar(wp, "save", "");
	if (strRequest[0])
	{
		PARAM_HEADER_T header;
		unsigned char *ptr;
		unsigned int fileSize,filelen;
		unsigned int fileSector;

		wp->buffer_end=0; // clear header
		FILE *fp;
		//create config file
		rc4_encry();
		boaWrite(wp, "HTTP/1.0 200 OK\n");
		boaWrite(wp, "Content-Type: application/octet-stream;\n");

		boaWrite(wp, "Content-Disposition: attachment;filename=\"log\" \n");


		boaWrite(wp, "Pragma: no-cache\n");
		boaWrite(wp, "Cache-Control: no-cache\n");
		boaWrite(wp, "\n");

		fp=fopen("/tmp/log.txt","r");
		//decide the file size
		fseek(fp, 0, SEEK_END);
		filelen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fileSize=filelen;

		while(fileSize>0)
		{
			char buf[0x1000];
			int nRead;
			maxFileSector=0x500;
//			printf("write %d %d %08x\n",maxFileSector, fileSize, (unsigned int )ptr);
			fileSector = (fileSize>maxFileSector)?maxFileSector:fileSize;
			nRead = fread((void *)buf, 1, fileSector, fp);
			buf[nRead]=0;
			boaWriteDataNonBlock(wp, buf, nRead);
			//printf("%s",buf);
			fileSize -= fileSector;
			ptr += fileSector;
 			//sleep(1);
		}
		fclose(fp);
		//boaDone(wp, 200);
		//OK_MSG("/saveconf.asp");
		return;
	}
//#ifndef NO_ACTION
	if(sysCmd[0]){
		snprintf(tmpBuf, 100, "%s 2>&1 > %s",sysCmd,  _PATH_SYSCMD_LOG);
		system(tmpBuf);
	}
//#endif


	strVal = boaGetVar(wp, "adsldbg", "");
	if(strVal[0]!=0){
		if (strVal[0]) {
			if (strVal[0] == '0'){
				adsldbg = 0;
			}
			else{
				adsldbg = 1;
			}
			if ( !mib_set(MIB_ADSL_DEBUG, (void *)&adsldbg)) {
				strcpy(tmpBuf, Tset_mib_error);
			}
		}
	}

	boaRedirect(wp, submitUrl);
	return;
}

int sysCmdLog(int eid, request * wp, int argc, char **argv)
{
        FILE *fp;
	char  buf[150];
	int nBytesSent=0;

        fp = fopen(_PATH_SYSCMD_LOG, "r");
        if ( fp == NULL )
                goto err1;
        while(fgets(buf,150,fp)){
		nBytesSent += boaWrite(wp, "%s", buf);
        }
	fclose(fp);
	unlink(_PATH_SYSCMD_LOG);
err1:
	return nBytesSent;
}
int lanSetting(int eid, request * wp, int argc, char **argv)
{
char *parm;

	if (boaArgs(argc, argv, "%s", &parm) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return -1;
	}
 if( parm[0] == '1' ){
 	boaWrite(wp, "menu.addItem(\"Lan Setting\");"
			"lansetting = new MTMenu();"
			"lansetting.addItem(\"LAN Interface\", \"tcpiplan.asp\", \"\", \"LAN Interface Configuration\");"
			"lansetting.addItem(\"DHCP Mode\", \"dhcpmode.asp\", \"\", \"DHCP Mode Configuration\");"
			"lansetting.addItem(\"DHCP Server\", \"dhcpd.asp\", \"\", \"DHCP Server Configuration\");");
	boaWrite(wp, "menu.makeLastSubmenu(lansetting);");
 	}
 else
  {
  boaWrite(wp, "<tr><td><b>Lan Setting</b></td></tr>\n"
                           "<tr><td>&nbsp;&nbsp;<a href=\"tcpiplan.asp\" target=\"view\">LAN Interface</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcpmode.asp\" target=\"view\">DHCP Mode</a></td></tr>\n"
			"<tr><td>&nbsp;&nbsp;<a href=\"dhcpd.asp\" target=\"view\">DHCP Server</a></td></tr>\n");
 	}

   return 0;
}

static int process_msg(char *msg, int is_wlan_only)
{
	char *p1, *p2;

	p2 = strstr(p1, "wlan");
	if (p2 && p2[5]==':')
		memcpy(p1, p2, strlen(p2)+1);
	else {
		if (is_wlan_only)
			return 0;

		p2 = strstr(p1, "klogd: ");
		if (p2 == NULL)
			return 0;
		memcpy(p1, p2+7, strlen(p2)-7+1);
	}
	return 1;
}

#ifdef FINISH_MAINTENANCE_SUPPORT
//added by xl_yue for supporting inform ITMS after finishing maintenance
void formFinishMaintenance(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	pid_t tr069_pid;

	str = boaGetVar(wp, "finish", "");
	if (str[0]) {
		// signal tr069 to inform ITMS that maintenance is finished
		tr069_pid = read_pid("/var/run/cwmp.pid");
		if ( tr069_pid > 0)
			kill(tr069_pid, SIGUSR1);
		else
			goto setErr_Signal;
	}

	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG1(multilang(LANG_OK_START_TO_INFORM_ITMS_THAT_MAINTENANCE_IS_OVER),submitUrl);
  	return;

setErr_Signal:
	ERR_MSG(multilang(LANG_ERROR_CAN_NOT_FIND_TR069_PCOCESS));

}
#endif

//added by xl_yue
#ifdef USE_LOGINWEB_OF_SERVER

#ifdef USE_BASE64_MD5_PASSWD
void calPasswdMD5(char *pass, char *passMD5);
#endif

void formLogin(request * wp, char *path, char *query)
{
	char	*str,*username,*password, *submitUrl;
	char	suPasswd[MAX_NAME_LEN], usPasswd[MAX_NAME_LEN];
#ifdef USE_BASE64_MD5_PASSWD
	char md5_pass[32];
#endif
	struct user_info * pUser_info;
#ifdef LOGIN_ERR_TIMES_LIMITED
	struct errlogin_entry * pErrlog_entry = NULL;
#endif
	char usrName[MAX_NAME_LEN];
	char supName[MAX_NAME_LEN];
	//xl_yue:1:bad password;2:invalid user;3:login error for three times,forbidden;4:other has login;
	int denied = 1;
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	int totalEntry, i;
#endif

	str = boaGetVar(wp, "save", "");
	if (str[0]) {
		pUser_info = search_login_list(wp);
		if(pUser_info){
			denied = 5;
			goto setErr_Signal;
		}
		username = boaGetVar(wp, "username", "");
		// Mason Yu on True
		//printf("username=%s\n", username);
		strcpy(g_login_username, username);
		if (!username[0] ) {
			denied = 2;
			goto setErr_Signal;
		}
		password = boaGetVar(wp, "password", "");
		if (!password[0] ) {
			denied = 1;
			goto setErr_Signal;
		}
	}else{
		denied = 10;
		goto setErr_Signal;
	}

#ifdef ACCOUNT_CONFIG
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry)) {
			denied = 10;
			goto setErr_Signal;
		}
		if (Entry.privilege == (unsigned char)PRIV_ROOT)
			strcpy(supName, Entry.userName);
		else
			strcpy(usrName, Entry.userName);
		if (strcmp(username, Entry.userName) == 0) {
#ifdef USE_BASE64_MD5_PASSWD
			calPasswdMD5(Entry.userPassword, md5_pass);
			if (strcmp(password, md5_pass))
#else
			if (strcmp(password,Entry.userPassword))
#endif
			{
				denied = 1;
				goto setErr_Signal;
			}
			denied = 0;
			goto pass_check;
		}
	}
#endif
	if ( !mib_get(MIB_USER_NAME, (void *)usrName) ) {
		denied = 10;
		goto setErr_Signal;
	}

	if(strcmp(usrName, username)==0){
		if ( !mib_get(MIB_USER_PASSWORD, (void *)usPasswd) ) {
			denied = 10;
			goto setErr_Signal;
		}
#ifdef USE_BASE64_MD5_PASSWD
		calPasswdMD5(usPasswd, md5_pass);
		if(strcmp(password,md5_pass))
#else
		if(strcmp(password,usPasswd))
#endif
		{
			denied = 1;
			goto setErr_Signal;
		}
		denied = 0;
		goto pass_check;
	}

	if ( !mib_get(MIB_SUSER_NAME, (void *)supName) ) {
		denied = 10;
		goto setErr_Signal;
	}

	if(strcmp(supName, username)==0){
		if ( !mib_get(MIB_SUSER_PASSWORD, (void *)suPasswd) ){
			denied = 10;
			goto setErr_Signal;
		}
#ifdef USE_BASE64_MD5_PASSWD
		calPasswdMD5(suPasswd, md5_pass);
		if(strcmp(password,md5_pass))
#else
		if(strcmp(password,suPasswd))
#endif
		{
			denied = 1;
			goto setErr_Signal;
		}
		denied = 0;
		goto pass_check;
	}

	if(denied){
		denied = 2;
		goto setErr_Signal;
	}

pass_check:

#ifdef ONE_USER_LIMITED
	if(!strcmp(usrName, username) && usStatus.busy){
		if(strcmp(usStatus.remote_ip_addr, wp->remote_ip_addr)){
			denied = 4;
			goto setErr_Signal;
		}
	}else if(!strcmp(supName, username) && suStatus.busy){
		if(strcmp(suStatus.remote_ip_addr, wp->remote_ip_addr)){
			denied = 4;
			goto setErr_Signal;
		}
	}
#endif

	pUser_info = search_login_list(wp);
	if(!pUser_info){
		pUser_info = malloc(sizeof(struct user_info));
		pUser_info->last_time = time_counter;
//		pUser_info->login_status = STATUS_LOGIN;
		strncpy(pUser_info->remote_ip_addr, wp->remote_ip_addr, sizeof(pUser_info->remote_ip_addr));
		if(strcmp(usrName, username)==0){
			pUser_info->directory = strdup("index_user.html");
#ifdef ONE_USER_LIMITED
			pUser_info->paccount = &usStatus;
			pUser_info->paccount->busy = 1;
			strncpy(pUser_info->paccount->remote_ip_addr, wp->remote_ip_addr, sizeof(pUser_info->paccount->remote_ip_addr));
#endif
		}
		else{
#ifdef CONFIG_VDSL
			pUser_info->directory = strdup("/index_vdsl.html");
#else
			pUser_info->directory = strdup("/index.html");
#endif
#ifdef ONE_USER_LIMITED
			pUser_info->paccount = &suStatus;
			pUser_info->paccount->busy = 1;
			strncpy(pUser_info->paccount->remote_ip_addr, wp->remote_ip_addr, sizeof(pUser_info->paccount->remote_ip_addr));
#endif
		}
		//list it to user_login_list
		pUser_info->next = user_login_list;
		user_login_list = pUser_info;

		syslog(LOG_INFO, "login successful for %s from %s\n", username, wp->remote_ip_addr);
	}else{
//		if(pUser_info->login_status != STATUS_FORBIDDEN){
			pUser_info->last_time = time_counter;
//			pUser_info->login_status = STATUS_LOGIN;
//		}
	}

#ifdef LOGIN_ERR_TIMES_LIMITED
	free_from_errlog_list(wp);
#endif

	boaRedirect(wp, "/");

//	submitUrl = boaGetVar(wp, "submit-url", "");
//	OK_MSG1(multilang(LANG_OK_LOGIN_SUCCESSFULLY),submitUrl);
  	return;

setErr_Signal:

#ifdef LOGIN_ERR_TIMES_LIMITED
	if(denied == 1 || denied == 2){
		pErrlog_entry = search_errlog_list(wp);
		if(pErrlog_entry){
			pErrlog_entry->last_time = time_counter;
			pErrlog_entry->login_count++;
			if(pErrlog_entry->login_count >= MAX_LOGIN_NUM)
				denied = 3;
		}else{
			pErrlog_entry = malloc(sizeof(struct errlogin_entry));
			pErrlog_entry->last_time = time_counter;
			pErrlog_entry->login_count = 1;
			strncpy(pErrlog_entry->remote_ip_addr, wp->remote_ip_addr, sizeof(pErrlog_entry->remote_ip_addr));
			pErrlog_entry->next = errlogin_list;
			errlogin_list = pErrlog_entry;
		}
	}
#endif

	switch(denied){
		case 1:
			ERR_MSG2(multilang(LANG_ERROR_BAD_PASSWORD));
			syslog(LOG_ERR, "login error from %s for bad password \n",wp->remote_ip_addr);
			break;
		case 2:
			ERR_MSG2(multilang(LANG_ERROR_INVALID_USERNAME));
			syslog(LOG_ERR, "login error from %s for invalid username \n",wp->remote_ip_addr);
			break;
#ifdef LOGIN_ERR_TIMES_LIMITED
		case 3:
			ERR_MSG2(multilang(LANG_ERROR_YOU_HAVE_LOGINED_ERROR_THREE_TIMES_PLEASE_RELOGIN_1_MINUTE_LATER));
			syslog(LOG_ERR, "login error from %s for having logined error three times \n",wp->remote_ip_addr);
			break;
#endif
#ifdef ONE_USER_LIMITED
		case 4:
			ERR_MSG2(multilang(LANG_ERROR_ANOTHER_USER_HAVE_LOGINED_IN_USING_THIS_ACCOUNTONLY_ONE_USER_CAN_LOGIN_USING_THIS_ACCOUNT_AT_THE_SAME_TIME));
			syslog(LOG_ERR, "login error from %s for using the same account with another user at the same time\n",wp->remote_ip_addr);
			break;
#endif
		case 5:
			ERR_MSG2(multilang(LANG_ERROR_YOU_HAVE_LOGINED_PLEASE_LOGOUT_AT_FIRST_AND_THEN_LOGIN));
			syslog(LOG_ERR, "login error from %s for having logined\n",wp->remote_ip_addr);
			break;
		default:
			ERR_MSG2(multilang(LANG_ERROR_WEB_AUTHENTICATION_ERRORPLEASE_CLOSE_THIS_WINDOW_AND_REOPEN_YOUR_WEB_BROWSER_TO_LOGIN));
			syslog(LOG_ERR, "web authentication error!\n");
			break;
		}
}

void formLogout(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	struct user_info * pUser_info;

	str = boaGetVar(wp, "save", "");
	if (str[0]) {
		if(!free_from_login_list(wp)){
			syslog(LOG_ERR, "logout error from %s\n",wp->remote_ip_addr);
			goto setErr_Signal;
		}
		syslog(LOG_INFO, "logout successful from %s\n",wp->remote_ip_addr);
	}

	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG1(multilang(LANG_OK_LOGOUT_SUCCESSFULLY),submitUrl);
  	return;

setErr_Signal:
	ERR_MSG(multilang(LANG_ERROR_LOGOUT_FAILEDPERHAPS_YOU_HAVE_LOGOUTED));

}

int passwd2xmit(int eid, request * wp, int argc, char **argv)
{
#ifdef USE_BASE64_MD5_PASSWD
	boaWrite(wp, "document.cmlogin.password.value = b64_md5(document.cmlogin.password.value);");
#endif
}

// Multi-lingual for login, added by davian_kuo.
void formLoginMultilang(request *wp, char *path, char *query) {
	char *strVal;
	char *submitUrl;

	strVal = (char *)boaGetVar (wp, "loginSelinit", "");
	if (strVal) {
		g_language_state = atoi (strVal);
		if (g_language_state != g_language_state_prev) {
			//printf ("Switch to lang_%d\n", g_language_state);
			g_language_state_prev = g_language_state;

#if MULTI_LANG_DL == 1
			if (dl_handle != NULL) dlclose (dl_handle);

			char *lib_name = (char *) malloc (sizeof(char) * 25);
			if (lib_name == NULL) {
				fprintf (stderr, "lib_name malloc failed!\n"); return;
			}
			char *strtbl_name = (char *) malloc (sizeof(char) * 15);
			if (strtbl_name == NULL) {
				fprintf (stderr, "strtbl_name malloc failed!\n"); return;
			}
			snprintf (lib_name, 25, "libmultilang_%s.so", lang_set[g_language_state].langType);
			snprintf (strtbl_name, 15, "strtbl_%s", lang_set[g_language_state].langType);

			dl_handle = dlopen (lib_name, RTLD_LAZY);
			strtbl = (const char **) dlsym (dl_handle, strtbl_name);

			free (lib_name);
			free (strtbl_name);
#else
			strtbl = strtbl_name[g_language_state];
#endif
			// Save mib to xmlconfig.
			if (!mib_set (MIB_MULTI_LINGUAL, (void *)lang_set[g_language_state].langType)) {
				ERR_MSG (strSetMultiLangError);
				return;
			}
			#ifdef COMMIT_IMMEDIATELY
			Commit ();
			#endif
		}
	}
}

#endif


#ifdef ACCOUNT_LOGIN_CONTROL
//added by xl_yue
/*
extern struct account_info su_account;
extern char suName[MAX_NAME_LEN];
*/
void formAdminLogout(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;

	str = boaGetVar(wp, "adminlogout", "");
	if (str[0]) {
		su_account.account_busy = 0;
		su_account.account_timeout = 1;
		syslog(LOG_INFO, "Account: %s logout from %s\n", suName, su_account.remote_ip_addr);
	}
	else
		goto err_logout;

	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG1(multilang(LANG_LOGOUT_SUCCESSFULLY),submitUrl);
  	return;

err_logout:
	ERR_MSG(multilang(LANG_LOGOUT_ERROR));
}

//added by xl_yue
/*
extern struct account_info us_account;
extern char usName[MAX_NAME_LEN];
*/
void formUserLogout(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;

	str = boaGetVar(wp, "userlogout", "");
	if (str[0]) {
		us_account.account_busy = 0;
		us_account.account_timeout = 1;
		syslog(LOG_INFO, "Account: %s logout from %s\n", usName, us_account.remote_ip_addr);
	}
	else
		goto err_logout;

	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG1(multilang(LANG_LOGOUT_SUCCESSFULLY),submitUrl);
  	return;

err_logout:
	ERR_MSG(multilang(LANG_LOGOUT_ERROR));
}

#endif


#ifdef CONFIG_USER_RTK_SYSLOG
int sysLogList(int eid, request * wp, int argc, char **argv)
{
	FILE *fp;
	char buff[256], *facstr, *pristr, *timestr, *tmp, *msgstr;
	int nBytesSent=0;
	int enabled;
	unsigned char vChar;
	int pri;

	if (first_time==1) {
		mib_get(MIB_ADSL_DEBUG, (void *)&vChar);
		if(vChar==1)
			dbg_enable=1;
		first_time=0;
	}

	if (dbg_enable==0) {
		nBytesSent += boaWrite(wp, "<tr>"
			"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>%s/%s</b></font></td>\n"
			"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td>\n"
			"<td align=center bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
		multilang(LANG_DATE), multilang(LANG_TIME), multilang(LANG_FACILITY),
		multilang(LANG_LEVEL), multilang(LANG_MESSAGE));

		fp = fopen("/var/log/messages.old", "r");
		if (fp) {
			fgets(buff, sizeof(buff), fp);
			while (fgets(buff, sizeof(buff), fp) != NULL) {
				tmp = strtok(buff, "|");
				timestr = strtok(NULL, "|");
				facstr = strtok(NULL, ".");
				pristr = strtok(NULL, "|");
				msgstr = strtok(NULL, "\n");
				mib_get(MIB_SYSLOG_DISPLAY_LEVEL, (void *)&vChar);
				pri = atoi(tmp);
				if((pri & 0x07) <= vChar)
					nBytesSent += boaWrite(wp, "<tr>"
						"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
						"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
						"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
						"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n", timestr, facstr, pristr, msgstr);
			}
			fclose(fp);
		}

		if (!(fp = fopen("/var/log/messages", "r"))) {
			//printf("Error: cannot open /var/log/messages - continuing...\n");
			goto err1;
		}
		fgets(buff, sizeof(buff), fp);
		while (fgets(buff, sizeof(buff), fp) != NULL) {
			tmp = strtok(buff, "|");
			timestr = strtok(NULL, "|");
			facstr = strtok(NULL, ".");
			pristr = strtok(NULL, "|");
			msgstr = strtok(NULL, "\n");
			mib_get(MIB_SYSLOG_DISPLAY_LEVEL, (void *)&vChar);
			pri = atoi(tmp);
			if((pri & 0x07) <= vChar)
				nBytesSent += boaWrite(wp, "<tr>"
					"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
					"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
					"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td>\n"
					"<td bgcolor=\"#C0C0C0\"><font size=\"2\">%s</td></tr>\n", timestr, facstr, pristr, msgstr);
		}
		fclose(fp);
	}
err1:
	return nBytesSent;
}

static void saveLogFile(request * wp, FILE *fp)
{
	unsigned char *ptr;
	unsigned int fileSize,filelen;
	unsigned int fileSector;
	unsigned int maxFileSector;

	//decide the file size
	fseek(fp, 0, SEEK_END);
	filelen = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	fileSize=filelen;

	while (fileSize>0) {
		char buf[0x100];
		maxFileSector = 0x50;
		int nRead;

		fileSector = (fileSize > maxFileSector) ? maxFileSector : fileSize;
		nRead = fread((void *)buf, 1, fileSector, fp);

		boaWriteDataNonBlock(wp, buf, nRead);

		fileSize -= fileSector;
		ptr += fileSector;
	}
}

void formSysLog(request * wp, char *path, char *query)
{
	char  *submitUrl,*strVal,*str;
	unsigned char logcap;
	char tmpBuf[100];
	struct in_addr inIp;

	//max msg. length
	str = boaGetVar(wp, "maxloglen", "");
	if (str[0]) {
		unsigned int len;
		len = (unsigned int) strtol(str, (char**)NULL, 10);
		if ( mib_set(MIB_MAXLOGLEN, (void *)&len) == 0) {
			strcpy(tmpBuf, Tset_mib_error);
			goto formSysLog_err;
		}
	}

	// Set ACL Capability
	strVal = boaGetVar(wp, "apply", "");
	if (strVal[0]) {
		struct in_addr inIpAddr;
		strVal = boaGetVar(wp, "logcap", "");
		if (strVal[0]) {
			if (strVal[0] == '0')
				logcap = 0;
			else
				logcap = 1;
			if ( !mib_set(MIB_SYSLOG, (void *)&logcap)) {
				strcpy(tmpBuf, Tset_mib_error);
				goto formSysLog_err;
			}
		}

#ifdef WEB_ENABLE_PPP_DEBUG
		strVal = boaGetVar(wp, "pppcap", "");
		if (strVal[0]) {
			unsigned char pppcap;
			struct data_to_pass_st msg;
			pppcap = strVal[0] - '0';
			snprintf(msg.data, BUF_SIZE, "spppctl syslog %d", pppcap);
			write_to_pppd(&msg);
		}
#endif

		strVal = boaGetVar(wp, "levelLog", "");
		if (strVal[0]) {
			logcap = strVal[0] - '0' ;
			if (mib_set(MIB_SYSLOG_LOG_LEVEL, (void *)&logcap) == 0) {
				strcpy(tmpBuf, Tset_mib_error);
				goto formSysLog_err;
			}
		}
		strVal = boaGetVar(wp, "levelDisplay", "");
		if (strVal[0]) {
			logcap = strVal[0] - '0' ;
			if (mib_set(MIB_SYSLOG_DISPLAY_LEVEL, (void *)&logcap) == 0) {
				strcpy(tmpBuf, Tset_mib_error);
				goto formSysLog_err;
			}
		}
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
		strVal = boaGetVar(wp, "logMode", "");
		if (strVal[0]) {
			logcap = strVal[0] - '0' ;
			if (mib_set(MIB_SYSLOG_MODE, (void *)&logcap) == 0) {
				strcpy(tmpBuf, Tset_mib_error);
				goto formSysLog_err;
			}
		}
		strVal = boaGetVar(wp, "logAddr", "");
		if (strVal[0]) {
			if (!inet_aton(strVal, &inIpAddr)) {
				strcpy(tmpBuf, strWrongIP);
				goto formSysLog_err;
			}
			if (!mib_set(MIB_SYSLOG_SERVER_IP, (void *)&inIpAddr)) {
				strcpy(tmpBuf, strSetIPerror);
				goto formSysLog_err;
			}
		}
		strVal = boaGetVar(wp, "logPort", "");
		if (strVal && strVal[0] ) {
			unsigned short intVal = atoi(strVal);
			if (!mib_set(MIB_SYSLOG_SERVER_PORT, (void *)&intVal)) {
				strcpy(tmpBuf, Tsvr_port);
				goto formSysLog_err;
			}
		}
#endif
 	}

	// Set Log Server IP
#ifdef SEND_LOG
	strVal = boaGetVar(wp, "ip", "");
	if (strVal[0]) {
		if ( !inet_aton(strVal, &inIp) ) {
			strcpy(tmpBuf, strWrongIP);
			goto formSysLog_err;
		}

		if ( !mib_set( MIB_LOG_SERVER_IP, (void *)&inIp)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto formSysLog_err;
		}
 	}

	// Set User name for Log Server
	strVal = boaGetVar(wp, "username", "");
	if (strVal[0]) {
		if ( !mib_set(MIB_LOG_SERVER_NAME, (void *)strVal)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto formSysLog_err;
		}
	}

	// Set Passeord for Log Server
	strVal = boaGetVar(wp, "passwd", "");
	if (strVal[0]) {
		if ( !mib_set(MIB_LOG_SERVER_PASSWORD, (void *)strVal)) {
			strcpy(tmpBuf, Tset_mib_error);
			goto formSysLog_err;
		}
	}
#endif

	// Save log File
	strVal = boaGetVar(wp, "save_log", "");
	if (strVal[0])
	{
		/*unsigned char *ptr;
		unsigned int fileSize,filelen;
		unsigned int fileSector;
		unsigned int maxFileSector;*/
		FILE *fp, *fp2;

		fp=fopen("/var/log/messages","r");
		if ( fp == NULL ) {
			strcpy(tmpBuf, multilang(LANG_SYSTEM_LOG_NOT_EXISTS));
			goto formSysLog_err;
		}

		wp->buffer_end=0; // clear header
		boaWrite(wp, "HTTP/1.0 200 OK\n");
		boaWrite(wp, "Content-Type: application/octet-stream;\n");
		boaWrite(wp, "Content-Disposition: attachment;filename=\"messages.txt\" \n");
		boaWrite(wp, "Pragma: no-cache\n");
		boaWrite(wp, "Cache-Control: no-cache\n");
		boaWrite(wp, "\n");

		fp2 = fopen("/var/log/messages.old", "r");
		if (fp2) {
			saveLogFile(wp, fp2);
			fclose(fp2);
		}
		/*
		//decide the file size
		fseek(fp, 0, SEEK_END);
		filelen = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fileSize=filelen;

		while(fileSize>0)
		{
			char buf[0x100];
			maxFileSector=0x50;
			int nRead;

//			printf("write %d %d %08x\n",maxFileSector, fileSize, (unsigned int )ptr);
			fileSector = (fileSize>maxFileSector)?maxFileSector:fileSize;
			//fileSector = fileSize;
			nRead = fread((void *)buf, 1, fileSector, fp);

			boaWriteDataNonBlock(wp, buf, nRead);

			fileSize -= fileSector;
			ptr += fileSector;
			//wrong free....
			//free(buf);
 			//sleep(1);
		}
		*/
		saveLogFile(wp, fp);
		fclose(fp);
		return;

	}

	// Clear log file
	str = boaGetVar(wp, "clear_log", "");
	if (str[0]) {
		unlink("/var/log/messages.old");
		unlink("/var/log/messages");
		submitUrl = boaGetVar(wp, "submit-url", "");
		if (submitUrl[0])
			boaRedirect(wp, submitUrl);
		else
			boaDone(wp, 200);
	  	return;
	}

formSysLog_end:
#if defined(APPLY_CHANGE)
	// Mason Yu. Take effect in real time.
	stopLog();
	startLog();
#endif

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);
	return;
formSysLog_err:
	ERR_MSG(tmpBuf);
}

#ifdef WEB_ENABLE_PPP_DEBUG
void ShowPPPSyslog(int eid, request * wp, int argc, char **argv)
{
	boaWrite(wp, "<tr>\n\t<td width=\"25%%\"><font size=2><b>Show PPP Debug Message&nbsp;:</b></td>\n");
	boaWrite(wp, "\t<td width=\"30%%\"><font size=2>\n");
	boaWrite(wp, "\t\t<input type=\"radio\" value=\"0\" name=\"pppcap\">Disable&nbsp;&nbsp;");
	boaWrite(wp, "\n\t\t<input type=\"radio\" value=\"1\" name=\"pppcap\">Enable");
	boaWrite(wp, "\n\t</td>\n</tr>\n");
}
#endif

void RemoteSyslog(int eid, request * wp, int argc, char **argv)
{
	char *name;

	if (boaArgs(argc, argv, "%s", &name) < 1) {
		boaError(wp, 400, "Insufficient args\n");
		return;
	}

	if (!strncmp(name, "syslog-mode", 11)) {
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
		boaWrite(wp, "<tr>\n\t<td><font size=2><b>Mode&nbsp;:</b></td>\n");
		boaWrite(wp, "\t<td><select name='logMode' size=\"1\" onChange='cbClick(this)'>\n");
		checkWrite(eid, wp, argc, argv);
#else
		boaWrite(wp, "<input type=\"hidden\" name=\"logMode\">\n");
#endif
	}

	if (!strncmp(name, "server-info", 11)) {
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
		boaWrite(wp, "\n\t</select></td>\n</tr>\n"
				"\t<td><font size=2><b>Server IP Address&nbsp;:</b></td>\n"
				"\t<td><input type='text' name='logAddr' maxlength=\"15\"></td>\n"
				"</tr>\n<tr>\n"
				"\t<td><font size=2><b>Server UDP Port&nbsp;:</b></td>\n"
				"\t<td><input type='text' name='logPort' maxlength=\"15\"></td>\n"
				"</tr>\n");
#else
		boaWrite(wp, "<input type=\"hidden\" name=\"logAddr\">\n");
		boaWrite(wp, "<input type=\"hidden\" name=\"logPort\">\n");
#endif
	}

	if (!strncmp(name, "check-ip", 8)) {
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
		boaWrite(wp, "\tif (document.forms[0].logAddr.disabled == false && !checkIP(document.formSysLog.logAddr))\n");
		boaWrite(wp, "\t\treturn false;\n");
#endif
	}
}
#endif	// of CONFIG_USER_RTK_SYSLOG

#ifdef TIME_ZONE
void formNtp(request * wp, char *path, char *query)
{
	char *submitUrl,*strVal, *tmpStr;
	char tmpBuf[100];
	char enabled=0, ntpServerIdx;
	unsigned int ext_if;

	strVal = boaGetVar(wp, "save", "");

	if(strVal[0]){
		struct tm tm_time;
		time_t tm;
		unsigned char dst_enabled = 1;

		tm_time.tm_isdst = -1;  /* Be sure to recheck dst. */
		strVal = boaGetVar(wp, "year", "");
		tm_time.tm_year = atoi(strVal) - 1900;
		strVal = boaGetVar(wp, "month", "");
		tm_time.tm_mon = atoi(strVal)-1;
		strVal = boaGetVar(wp, "day", "");
		tm_time.tm_mday = atoi(strVal);
		strVal = boaGetVar(wp, "hour", "");
		tm_time.tm_hour = atoi(strVal);
		strVal = boaGetVar(wp, "minute", "");
		tm_time.tm_min = atoi(strVal);
		strVal = boaGetVar(wp, "second", "");
		tm_time.tm_sec = atoi(strVal);
		tm = mktime(&tm_time);
		if(tm < 0){
			sprintf(tmpBuf, multilang(LANG_SET_TIME_ERROR));
			goto setErr_end;
		}
		if(stime(&tm) < 0){
			sprintf(tmpBuf, multilang(LANG_SET_TIME_ERROR));
			goto setErr_end;
		}

		tmpStr = boaGetVar(wp, "dst_enabled", "");
		if (!strcmp(tmpStr, "ON"))
			dst_enabled = 1;
		else
			dst_enabled = 0;
		if (mib_set(MIB_DST_ENABLED, &dst_enabled) == 0) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_end;
		}

		tmpStr = boaGetVar(wp, "timeZone", "");
		if (tmpStr[0]) {
			FILE *fp;
			unsigned int index;

			index = strtoul(tmpStr, NULL, 0);

			if (mib_set(MIB_NTP_TIMEZONE_DB_INDEX, &index) == 0) {
				strcpy(tmpBuf, Tset_mib_error);
				goto setErr_end;
			}

			if ((fp = fopen("/etc/TZ", "w")) != NULL) {
				fprintf(fp, "%s\n", get_tz_string(index, dst_enabled));
				fclose(fp);
			}
		}

		tmpStr = boaGetVar(wp, "enabled", "");
		if(!strcmp(tmpStr, "ON"))
			enabled = 1 ;
		else
			enabled = 0 ;
		if ( mib_set( MIB_NTP_ENABLED, (void *)&enabled) == 0) {
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_end;
		}

		tmpStr = boaGetVar(wp, "ext_if", "");
		if(tmpStr[0])
			ext_if = (unsigned int)atoi(tmpStr);
		else
			ext_if = DUMMY_IFINDEX;  // No interface selected.
		if(!mib_set(MIB_NTP_EXT_ITF, (void *)&ext_if))
		{
			strcpy(tmpBuf, Tset_mib_error);
			goto setErr_end;
		}

		tmpStr = boaGetVar(wp, "ntpServerId", "");
		if(tmpStr[0]){
			ntpServerIdx = tmpStr[0] - '0' ;
			if ( mib_set(MIB_NTP_SERVER_ID, (void *)&ntpServerIdx) == 0) {
				strcpy(tmpBuf, Tset_mib_error);
				goto setErr_end;
			}
		}

		tmpStr = boaGetVar(wp, "ntpServerHost1", "");
		if(tmpStr[0]){
			if ( mib_set(MIB_NTP_SERVER_HOST1, (void *)tmpStr) == 0) {
				strcpy(tmpBuf, Tset_mib_error);
				goto setErr_end;
			}
		}

		tmpStr = boaGetVar(wp, "ntpServerHost2", "");
		if(tmpStr[0]){
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
			if ( mib_set(MIB_NTP_SERVER_HOST2, (void *)tmpStr) == 0) {
				strcpy(tmpBuf, Tset_mib_error);
				goto setErr_end;
			}
/*ping_zhang:20081217 END*/

		}
	}
	if (enabled == 0)
		goto  set_ntp_end;

set_ntp_end:
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
	cmd_set_dns_config(NULL);
	restart_dnsrelay();
#endif
#if defined(APPLY_CHANGE)
	if (enabled) {
		stopNTP();
		startNTP();
	} else {
		stopNTP();
	}
#endif

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	OK_MSG(submitUrl);
	return;

setErr_end:
	ERR_MSG(tmpBuf);
}

int timeZoneList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent = 0;
	unsigned int i, selected = 0;

	mib_get(MIB_NTP_TIMEZONE_DB_INDEX, &selected);

	for (i = 0; i < nr_tz; i++) {
		nBytesSent += boaWrite(wp, "<option value=\"%u\"%s>%s (UTC%s)</option>",
				i, (i == selected) ? " selected" : "",
				get_tz_location(i, FOR_WEB), get_tz_utc_offset(i));
	}

	return nBytesSent;
}
#endif

#ifdef DOS_SUPPORT
void formDosCfg(request * wp, char *path, char *query)
{
	char	*submitUrl, *tmpStr;
	char	tmpBuf[100];
	unsigned int	floodCount=0,blockTimer=0;
	unsigned int	enabled = 0;

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	mib_get(MIB_DOS_ENABLED, (void *)&enabled);

	tmpStr = boaGetVar(wp, "dosEnabled", "");
	if(!strcmp(tmpStr, "ON")) {
		enabled |= 1;

		tmpStr = boaGetVar(wp, "sysfloodSYN", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 2;
			tmpStr = boaGetVar(wp, "sysfloodSYNcount", "");
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_SYSSYN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, strSetDosSYSSYNFLOODErr);
				goto setErr;
			}
		}
		else{
			enabled &= ~2;
		}
		tmpStr = boaGetVar(wp, "sysfloodFIN", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 4;
			tmpStr = boaGetVar(wp, "sysfloodFINcount", "");
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_SYSFIN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, strSetDosSYSFINFLOODErr);
				goto setErr;
			}
		}
		else{
			enabled &= ~4;
		}
		tmpStr = boaGetVar(wp, "sysfloodUDP", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 8;
			tmpStr = boaGetVar(wp, "sysfloodUDPcount", "");
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_SYSUDP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, strSetDosSYSUDPFLOODErr);
				goto setErr;
			}
		}
		else{
			enabled &= ~8;
		}
		tmpStr = boaGetVar(wp, "sysfloodICMP", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x10;
			tmpStr = boaGetVar(wp, "sysfloodICMPcount", "");
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_SYSICMP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, strSetDosSYSICMPFLOODErr);
				goto setErr;
			}
		}
		else{
			enabled &= ~0x10;
		}
		tmpStr = boaGetVar(wp, "ipfloodSYN", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x20;
			tmpStr = boaGetVar(wp, "ipfloodSYNcount", "");
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_PIPSYN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, strSetDosPIPSYNFLOODErr);
				goto setErr;
			}
		}
		else{
			enabled &= ~0x20;
		}
		tmpStr = boaGetVar(wp, "ipfloodFIN", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x40;
			tmpStr = boaGetVar(wp, "ipfloodFINcount", "");
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_PIPFIN_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, strSetDosPIPFINFLOODErr);
				goto setErr;
			}
		}
		else{
			enabled &= ~0x40;
		}
		tmpStr = boaGetVar(wp, "ipfloodUDP", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x80;
			tmpStr = boaGetVar(wp, "ipfloodUDPcount", "");
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_PIPUDP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, strSetDosPIPUDPFLOODErr);
				goto setErr;
			}
		}
		else{
			enabled &= ~0x80;
		}
		tmpStr = boaGetVar(wp, "ipfloodICMP", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x100;
			tmpStr = boaGetVar(wp, "ipfloodICMPcount", "");
			string_to_dec(tmpStr,&floodCount);
			if ( mib_set(MIB_DOS_PIPICMP_FLOOD, (void *)&floodCount) == 0) {
				strcpy(tmpBuf, strSetDosPIPICMPFLOODErr);
				goto setErr;
			}
		}
		else{
			enabled &= ~0x100;
		}
		tmpStr = boaGetVar(wp, "TCPUDPPortScan", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x200;

			tmpStr = boaGetVar(wp, "portscanSensi", "");
			if( tmpStr[0]=='1' ) {
				enabled |= 0x800000;
			}
			else{
				enabled &= ~0x800000;
			}
		}
		else{
			enabled &= ~0x200;
		}
		tmpStr = boaGetVar(wp, "ICMPSmurfEnabled", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x400;
		}
		else{
			enabled &= ~0x400;
		}
		tmpStr = boaGetVar(wp, "IPLandEnabled", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x800;
		}
		else{
			enabled &= ~0x800;
		}
		tmpStr = boaGetVar(wp, "IPSpoofEnabled", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x1000;
		}
		else{
			enabled &= ~0x1000;
		}
		tmpStr = boaGetVar(wp, "IPTearDropEnabled", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x2000;
		}
		else{
			enabled &= ~0x2000;
		}
		tmpStr = boaGetVar(wp, "PingOfDeathEnabled", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x4000;
		}
		else{
			enabled &= ~0x4000;
		}
		tmpStr = boaGetVar(wp, "TCPScanEnabled", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x8000;
		}
		else{
			enabled &= ~0x8000;
		}
		tmpStr = boaGetVar(wp, "TCPSynWithDataEnabled", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x10000;
		}
		else{
			enabled &= ~0x10000;
		}
		tmpStr = boaGetVar(wp, "UDPBombEnabled", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x20000;
		}
		else{
			enabled &= ~0x20000;
		}
		tmpStr = boaGetVar(wp, "UDPEchoChargenEnabled", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x40000;
		}
		else{
			enabled &= ~0x40000;
		}
		tmpStr = boaGetVar(wp, "sourceIPblock", "");
		if(!strcmp(tmpStr, "ON")) {
			enabled |= 0x400000;
			tmpStr = boaGetVar(wp, "IPblockTime", "");
			string_to_dec(tmpStr,&blockTimer);
			if ( mib_set(MIB_DOS_BLOCK_TIME, (void *)&blockTimer) == 0) {
				strcpy(tmpBuf, strSetDosIPBlockTimeErr);
				goto setErr;
			}
		}
		else{
			enabled &= ~0x400000;
		}
	}
	else
		enabled = 0;

	if ( mib_set(MIB_DOS_ENABLED, (void *)&enabled) == 0) {
		strcpy(tmpBuf, strSetDosEnableErr);
		goto setErr;
	}

	//apmib_update(CURRENT_SETTING);
#if defined(APPLY_CHANGE)
	setup_dos_protection();
#endif

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

#ifndef NO_ACTION
	run_init_script("all");
#endif

	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page
	OK_MSG(submitUrl);

	return;
setErr:
	ERR_MSG(tmpBuf);
}
#endif

#ifdef WEB_REDIRECT_BY_MAC
void formLanding(request * wp, char *path, char *query)
{
	char *submitUrl, *strLTime;
	unsigned int uLTime;

	strLTime = boaGetVar(wp, "interval", "");
	if ( strLTime[0] ) {
		sscanf(strLTime, "%u", &uLTime);
	}

	mib_set(MIB_WEB_REDIR_BY_MAC_INTERVAL, (void *)&uLTime);

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		//boaRedirect(wp, submitUrl);
		OK_MSG(submitUrl);
	return;
}
#endif
void formVersionMod(request *wp, char *path, char *query)
{
	char *strData;

	// manufacturer
	strData = boaGetVar(wp,"txt_mft","");
	if (strData[0])
	{
		if (!mib_set(RTK_DEVID_MANUFACTURER, strData)) {
			goto setErr_tr069;
		}
		else printf("Update Manufacturer to %s \n" , strData);
	}

	// OUI
	strData = boaGetVar(wp,"txt_oui","");
 	if (strData[0])
	{
		if (!mib_set(RTK_DEVID_OUI, strData)) {
			goto setErr_tr069;
		}
		else printf("Update OUI to %s \n" , strData);
	}
		
	// Product Class
	strData = boaGetVar(wp,"txt_proclass","");
	if (strData[0])
	{
		if (!mib_set( RTK_DEVID_PRODUCTCLASS, strData)) {
			goto setErr_tr069;
		}
		else printf("Update Product Class to %s \n" , strData);
	}

	// HW Serial Number
	strData = boaGetVar(wp,"txt_serialno","");
	if (strData[0])
	{
		if (!mib_set(MIB_HW_SERIAL_NUMBER, (void *)strData)) {
			goto setErr_tr069;
		}
		else printf("Update Serial Number to %s \n" , strData);
	}

	// Provisioning Code
#ifdef CONFIG_USER_CWMP_TR069
	strData = boaGetVar(wp,"txt_provisioningcode","");
	if (strData[0])
	{
		if (!mib_set(CWMP_PROVISIONINGCODE, (void *)strData)) {
			goto setErr_tr069;
		}
		else printf("Update Provisioning Code to %s \n" , strData);
	}
#endif

	// Spec. Version
	strData = boaGetVar(wp,"txt_specver","");
	if (strData[0])
	{
		if (!mib_set(RTK_DEVINFO_SPECVER, (void *)strData)) {
			goto setErr_tr069;
		}
		else printf("Update Spec. Version to %s \n" , strData);
	}

	// Software Version
	strData = boaGetVar(wp,"txt_swver","");
	if (strData[0])
	{
		if (!mib_set(RTK_DEVINFO_SWVER, (void *)strData)) {
			goto setErr_tr069;
		}
		else printf("Update Software Version to %s \n" , strData);
	}

	// Hardware Version
	strData = boaGetVar(wp,"txt_hwver","");
	if (strData[0])
	{
		if (!mib_set(RTK_DEVINFO_HWVER, (void *)strData)) {
			goto setErr_tr069;
		}
		else printf("Update Hardware Version to %s \n" , strData);
	}
#if defined(CONFIG_GPON_FEATURE)
	//GPON SN
	strData = boaGetVar(wp,"txt_gponsn","");
	if (strData[0])
	{
		if (!mib_set(MIB_GPON_SN, (void *)strData)) {
			goto setErr_tr069;
		}
		else printf("Update GPON SN to %s \n" , strData);
	}
#endif
	// ELAN MAC Address
	strData = boaGetVar(wp,"txt_elanmac","");
	if (strData[0])
	{
		unsigned char mac[6];
		if ( !string_to_hex(strData, mac, 12)) {
			goto setErr_tr069;
		}
		if (!mib_set(MIB_ELAN_MAC_ADDR, (void *)mac)) {
			goto setErr_tr069;
		}
		else printf("Update ELAN MAC Address to %s \n" , strData);
	}


	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	mib_update(HW_SETTING, CONFIG_MIB_ALL);


	strData = boaGetVar(wp, "submit-url", "");
	if (strData[0])
		boaRedirect(wp, strData);

	return;
				
setErr_tr069:
	ERR_MSG(multilang(LANG_ERROR_SETTING));
}

#ifdef USER_WEB_WIZARD
void formWebWizardMenu(request * wp, char *path, char *query)
{
	char *submitUrl;

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
		//OK_MSG(submitUrl);
	return;
}

void formWebWizard1(request * wp, char *path, char *query)
{
	char *submitUrl, *strPassword, *strConfPassword;
	char tmpBuf[100];

	strPassword = boaGetVar(wp, "newpass", "");
	strConfPassword = boaGetVar(wp, "confirmpass", "");

	if ( !strPassword[0] ) {
		strcpy(tmpBuf, WARNING_EMPTY_NEW_PASSWORD);
		goto setErr_pass;
	}

	if ( !strConfPassword[0] ) {
		strcpy(tmpBuf, WARNING_EMPTY_CONFIRMED_PASSWORD);
		goto setErr_pass;
	}

	if (strcmp(strPassword, strConfPassword) != 0 ) {
		strcpy(tmpBuf, WARNING_UNMATCHED_PASSWORD);
		goto setErr_pass;
	}

	if (!mib_set(MIB_SUSER_PASSWORD, (void *)strPassword)) {
		strcpy(tmpBuf, WARNING_SET_PASSWORD);
		goto setErr_pass;
	}

#ifdef EMBED
	// Added by Mason Yu for take effect on real time
	writePasswdFile();
	write_etcPassword();	// Jenny
#endif

// Magician: Commit immediately
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

	/* Init user management */
	// Commented By Mason Yu
	//set_user_profile();

	/* Retrieve next page URL */
	submitUrl = boaGetVar(wp, "submit-url", "");   // hidden page

	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	//OK_MSG(submitUrl);
	return;

setErr_pass:
	ERR_MSG(tmpBuf);
}

void formWebWizard4(request * wp, char *path, char *query)
{
	char *username, *passwd, *submitUrl, *strValue;
	char tmpBuf[100];
	MIB_CE_ATM_VC_T Entry;
	unsigned int totalEntry;
	int i;

	// user name
	username = boaGetVar(wp, "pppusername", "");
	if ( username[0] ) {
		if (strlen(username) > MAX_PPP_NAME_LEN) {
			strcpy(tmpBuf, strUserNametoolong);
			goto setErr_ppp;
		}
	}
	else {
		strcpy(tmpBuf, strUserNameempty);
		goto setErr_ppp;
	}

	// password
	passwd = boaGetVar(wp, "ppppassword", "");
	if ( passwd[0] ) {
		if (strlen(passwd) > MAX_NAME_LEN-1) {
			strcpy(tmpBuf, strPasstoolong);
			goto setErr_ppp;
		}
	}
	else {
		strcpy(tmpBuf, strPassempty);
		goto setErr_ppp;
	}

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<totalEntry; i++) {
		struct data_to_pass_st msg;
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
  			printf(strGetChainerror);
			return;
		}
		if (Entry.enable == 0)
			continue;
		if (Entry.cmode == CHANNEL_MODE_PPPOE) {
			strncpy(Entry.pppUsername, username, MAX_PPP_NAME_LEN);
			Entry.pppUsername[strlen(username)]='\0';
			strncpy(Entry.pppPassword, passwd, MAX_NAME_LEN-1);
			Entry.pppPassword[strlen(passwd)]='\0';
			strValue = boaGetVar(wp, "ppp_vlan", "");
			Entry.vid = (unsigned int) strtol(strValue, (char**)NULL, 10);
			if (Entry.vid > 0)
				Entry.vlan = 1;
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
		}
		else
			continue;
	}

	submitUrl = boaGetVar(wp, "submit-url", "");
	if (submitUrl[0])
		boaRedirect(wp, submitUrl);
	else
		boaDone(wp, 200);
  	return;
  
setErr_ppp:
	ERR_MSG(tmpBuf);
}

void ShowWebWizardPage(int eid, request * wp, int argc, char **argv)
{
#ifdef WLAN_SUPPORT
	boaWrite(wp, "<input type=\"hidden\" value=\"/web_wizard_5.asp\" name=\"submit-url\">\n");
#else
	boaWrite(wp, "<input type=\"hidden\" value=\"/web_wizard_6.asp\" name=\"submit-url\">\n");
#endif
return;
}

#ifdef WLAN_SUPPORT
static void setWlan(request * wp,char disable,char* ssid,WIFI_SECURITY_T encryption,char* key)
{
	char curssid[32],tmpBuf[100];
	MIB_CE_MBSSIB_T Entry;

	if (disable == 1)
		return;

	wlan_getEntry(&Entry, 0);
	Entry.wlanDisabled = disable;
	strcpy(Entry.ssid, ssid);
	Entry.encrypt = encryption;

	Entry.wpaAuth = WPA_AUTH_PSK;
	Entry.unicastCipher = WPA_CIPHER_TKIP;
	Entry.wpa2UnicastCipher = WPA_CIPHER_AES;
	Entry.wpaPSKFormat = 0;//passphrase
	strcpy(Entry.wpaPSK, key);
	wlan_setEntry(&Entry, 0);
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
}
#endif

void formWebWizard5(request * wp, char *path, char *query)
{
	char *strValue,*ssid;
	char *strSubmit;
	char tmpBuf[100], disable;
	unsigned char encryption;
	int tempWlanIdx;

	strValue = boaGetVar(wp,"wlanDisabled","");
	if (!gstrcmp(strValue, "ON"))
		disable = 1;
	else
		disable = 0;

#ifdef WLAN_SUPPORT
	strValue = boaGetVar(wp,"psk","");
	encryption = WIFI_SEC_WPA2_MIXED;

	tempWlanIdx = wlan_idx;

	wlan_idx = 0;
	ssid = boaGetVar(wp,"ssid5g","");
	setWlan(wp, disable, ssid, (WIFI_SECURITY_T)encryption, strValue);

	wlan_idx = 1;
	ssid = boaGetVar(wp,"ssid2g","");
	setWlan(wp, disable, ssid, (WIFI_SECURITY_T)encryption, strValue);

	wlan_idx = tempWlanIdx;
#endif

setOK:
	strSubmit = boaGetVar(wp,"submit-url","");
	if(strSubmit[0])
		boaRedirect(wp, strSubmit);
	else
		boaDone(wp, 200);
	return;
}

void formWebWizard6(request * wp, char *path, char *query)
{
	char *strSubmit;
	unsigned char webwizard_flag = 0;

	mib_set(MIB_USER_WEB_WIZARD_FLAG, (void *)&webwizard_flag);
#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif
#ifdef WLAN_SUPPORT
	config_WLAN(ACT_RESTART);
#endif

setOK:
	//printf("[%s:%d] webwizard_flag=%d\n", __func__, __LINE__, webwizard_flag);
	strSubmit=boaGetVar(wp,"submit-url","");
	//printf("[%s:%d] strSubmit=%s\n", __func__, __LINE__, strSubmit);
	if(strSubmit[0])
		boaRedirect(wp, strSubmit);
	else
		boaDone(wp, 200);
	return;
}

#endif


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
#include <sys/mount.h>
#include <sys/socket.h>
#include <signal.h>
#include <linux/if.h>
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
#include "fmdefs.h"
#ifdef CONFIG_MIDDLEWARE
#include <rtk/midwaredefs.h>
#endif

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


#include "../defs.h"
#ifdef TIME_ZONE
#include "tz.h"
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

// Mason Yu. t123
#if 0
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
	multilang_bpas("Select"), multilang_bpas("User"), multilang_bpas(" Name"), multilang_bpas("Privilege"));

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
			priv = multilang_bpas("Admin");
		else if (Entry.privilege == PRIV_ENG)
			priv = multilang_bpas("Support");
		else if (Entry.privilege == PRIV_USER)
			priv = multilang_bpas("User");

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
				strcpy(tmpBuf, "Sorry, the account cannot be deleted!");
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
						strcpy(tmpBuf, "Delete chain record error!");
						goto setErr_user;
					}
					goto setOk_user;
				}
			}
		}
		else {
			strcpy(tmpBuf, "There is no item selected to delete!");
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
			strcpy(tmpBuf, "ERROR: user already exists!");
			goto setErr_user;
		}
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry)) {
  				boaError(wp, 400, strGetChainerror);
				return;
			}

			if (strcmp(Entry.userName, strUser) == 0) {
				strcpy(tmpBuf, "ERROR: user already exists!");
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
					strcpy(tmpBuf, "ERROR: Set Super user name to MIB database failed.");
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
					strcpy(tmpBuf, "ERROR: Set user name to MIB database failed.");
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
		strcpy(tmpBuf, "ERROR: Old Password cannot be empty.");
		goto setErr_pass;
	}

	if ( !strPassword[0] ) {
		strcpy(tmpBuf, "ERROR: New Password cannot be empty.");
		goto setErr_pass;
	}

	if ( !strConfPassword[0] ) {
		strcpy(tmpBuf, "ERROR: Confirmed Password cannot be empty.");
		goto setErr_pass;
	}

	if (strcmp(strPassword, strConfPassword) != 0 ) {
		strcpy(tmpBuf, "ERROR: New Password is not the same as Confirmed Password.");
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
		strcpy(tmpBuf, "ERROR: Get user password MIB error!");
		goto setErr_pass;
	}

	if ( strcmp(tmpBuf, strOldPassword) != 0 ) {
		strcpy(tmpBuf, "ERROR: Input Old user password error!");
		goto setErr_pass;
	}else if ( !mib_set(MIB_USER_PASSWORD, (void *)strPassword) ) {
		strcpy(tmpBuf, "ERROR: Set user password to MIB database failed.");
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
			va_cmd(IPTABLES, 11, 1, (char *)FW_DEL, (char *)FW_INPUT, ARG_I,
			"!", LANIF, "-p", ARG_TCP, FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
			// iptables -t nat -D PREROUTING -i ! $LAN_IF -p TCP --dport 51003 -j DNAT --to-destination ipaddr:80
			va_cmd(IPTABLES, 15, 1, "-t", "nat",
						(char *)FW_DEL,	"PREROUTING",
						(char *)ARG_I, "!", (char *)LANIF,
						"-p", (char *)ARG_TCP,
						(char *)FW_DPORT, dport, "-j",
						"DNAT", "--to-destination", ipaddr);
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

#endif
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
	boaWrite(wp, "固件升级失败 ! ");
	switch (status) {
		case ERROR_FILESIZE:
			boaWrite(wp, "(档案过大)");
			break;
		case ERROR_FORMAT:
			boaWrite(wp, "(档案格式错误)");
			break;
		case ERROR_INVALID:
		default:
			boaWrite(wp, "(无效档案)");
			break;
	}
	boaWrite(wp, "</h4>\n");
	boaWrite(wp, "%s<br><br>\n", rebootWord0);
	boaWrite(wp, "%s\n", rebootWord2);
	boaWrite(wp, "</blockquote></body>");
	boaFooter(wp);
	boaDone(wp, 200);
}

#ifdef SUPPORT_WEB_PUSHUP
#define SEVEN_DAY	(7*24*60*60)

extern int upgradeWebSet(int enable);

void formUpgradePop(request * wp, char * path, char * query)
{
	struct	timeval    tv;
	char *strRequest;

	/* get reply from client, stop pushup web at once! */
	upgradeWebSet(0);

	memset(&tv, 0, sizeof(tv));
	
	strRequest = boaGetVar(wp, "doit", "");
	if (strRequest[0])
	{
		/* get firmware right now */
		startUpgradeFirmware();

		/* clear push time to cancel periodly push job */
		mib_set(MIB_UPGRADE_WEB_PUSH_TIME, (void *)&tv.tv_sec);
	}

	strRequest = boaGetVar(wp, "nodo", "");
	if (strRequest[0])
	{
		/* do nothing */
		
		/* clear push time to cancel periodly push job */
		mib_set(MIB_UPGRADE_WEB_PUSH_TIME, (void *)&tv.tv_sec);
	}

	strRequest = boaGetVar(wp, "holdover", "");
	if (strRequest[0])
	{
		/* do it again after 7 days */
		gettimeofday(&tv, NULL);
		mib_set(MIB_UPGRADE_WEB_PUSH_TIME, (void *)&tv.tv_sec);
		
		startPushwebTimer(SEVEN_DAY);
	}
}
#endif

#ifdef UPGRADE_V1
///////////////////////////////////////////////////////////////////////////////
void formUpload(request * wp, char *path, char *query)
{
	unsigned int startPos, endPos, nLen;
	FILE *fp = NULL;

	/* find the start and end positive of run time image */
	//printf("\nTry to get file size of new firmware\n");

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
		displayUploadMessage(wp, ERROR_INVALID);
		fclose(fp);
		goto end;
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
		boaWrite(wp, "<html><head><META http-equiv=content-type content=\"text/html; charset=gbk\"><style>\n"
			 "#cntdwn{ border-color: white;border-width: 0px;font-size: 12pt;color: red;text-align:left; font-weight:bold; font-family: Courier;}\n"
			 "</style><script language=javascript>\n"
			 "var h=(%d+10);\n"
			 "function stop() { clearTimeout(id); }\n"
			 "function start() { h--; if (h >= 40) { frm.time.value = h; frm.textname.value='固件升级, 请稍等 ...'; id=setTimeout(\"start()\",1000); }\n"
			 "if (h >= 0 && h < 40) { frm.time.value = h; frm.textname.value='系统重启中, 请稍等 ...'; id=setTimeout(\"start()\",1000); }\n"
			 "if (h == 0) { window.open(\"/status.asp\",target=\"view\"); }}\n"
			 "</script></head><body bgcolor=white  onLoad=\"start();\" onUnload=\"stop();\">"
			 "<blockquote><form action=/boaform/formStopUpload method=post name=frm><b><font color=red><input type=text name=textname size=40 id=\"cntdwn\">\n"
			 "<input type=text name=time size=5 id=\"cntdwn\">\n"
#ifdef CONFIG_DOUBLE_IMAGE
			 "<input type=submit name=cancel value=\"Cancel and Reboot\">\n"
#endif
			 "</font></b></form>\n"
			 "<h4>在上传时请不要将本机断电以免造成系统毁坏.</h4>\n"
			 "</blockquote></body></html>", writeflashtime);
	}
	return;
#endif


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
		strcpy(tmpBuf, "ERROR: Restore Config file failed! Uploaded file size out of constraint!\n");
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
		ret = va_cmd("/bin/sh",3,1, "/etc/scripts/config_xmlconfig.sh", "-s", "/tmp/config.xml");
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
			strcpy(tmpBuf, "ERROR: Flash read fail.");
			goto fail;
		}

		fileSize = sizeof(PARAM_HEADER_T) + header.len;
		buf = malloc(fileSize);
		if ( buf == NULL ) {
			strcpy(tmpBuf, "Allocate buffer failed!");
			goto fail;
		}

		printf("fileSize=%d\n",fileSize);
		if(mib_read_to_raw(action_type, buf, fileSize) != 1)
		{
			free(buf);

			error(E_L, E_LOG, "ERROR: Flash read fail");
			strcpy(tmpBuf, "ERROR: Flash read fail.");
			goto fail;
		}
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
		buf = malloc(0x1000);
		if ( buf == NULL ) {
			strcpy(tmpBuf, "Allocate buffer failed!");
			goto fail;
		}
		while(fileSize>0)
		{
			int nRead;

//			printf("write %d %d %08x\n",maxFileSector, fileSize, (unsigned int )ptr);
			fileSector = (fileSize>maxFileSector)?maxFileSector:fileSize;
			nRead = fread((void *)buf, 1, fileSector, fp);
			boaWriteDataNonBlock(wp, buf, nRead);

			fileSize -= fileSector;
			ptr += fileSector;
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
			strcpy(tmpBuf, "ERROR: find the start and end of the upload file failed!");
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

		if(va_cmd("/bin/sh",3,1, "/etc/scripts/config_xmlconfig.sh", "-c", "/tmp/config.xml"))
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
		ret = va_cmd("/bin/sh",3,1, "/etc/scripts/config_xmlconfig.sh", "-l", "/tmp/config.xml");

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
			strcpy(tmpBuf, "ERROR: Restore Config file failed! Invalid config file!");
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
			strcpy(tmpBuf, "ERROR: find the start and end of the upload file failed!");
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
			strcpy(tmpBuf, "ERROR: Restore Config file failed! Invalid config file!");
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
		if(mib_update_from_raw(buf, nLen) != 1)
		{
			strcpy(tmpBuf, "Flash error!");
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

#if 0
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
void formSnmpConfig(request * wp, char *path, char *query)
{
	char *str, *submitUrl;
	struct in_addr trap_ip;
	static char tmpBuf[100];

//star: for take effect
	unsigned int snmpchangeflag=0;
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
				strcpy(tmpBuf, "formSnmpConfig: set MIB_SNMPD_ENABLE fail!");
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
			strcpy(tmpBuf, "Set snmpSysDescr mib error!");
			goto setErr_pass;
		}
	}

	str = boaGetVar(wp, "snmpSysContact", "");
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_CONTACT, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_CONTACT, (void *)str)) {
			strcpy(tmpBuf, "Set snmpSysContact mib error!");
			goto setErr_pass;
		}
	}

	str = boaGetVar(wp, "snmpSysName", "");
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_NAME, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_NAME, (void *)str)) {
			strcpy(tmpBuf, "Set snmpSysName mib error!");
			goto setErr_pass;
		}
	}

	str = boaGetVar(wp, "snmpSysLocation", "");
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_LOCATION, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_LOCATION, (void *)str)) {
			strcpy(tmpBuf, "Set snmpSysLocation mib error!");
			goto setErr_pass;
		}
	}

	str = boaGetVar(wp, "snmpSysObjectID", "");
	if (str[0]) {
		mib_get(MIB_SNMP_SYS_OID, (void*)origstr);
		if(strcmp(origstr,str)!=0)
			snmpchangeflag = 1;
		if ( !mib_set(MIB_SNMP_SYS_OID, (void *)str)) {
			strcpy(tmpBuf, "Set snmpSysObjectID mib error!");
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
		strcpy(tmpBuf, "ERROR: Set ADSL Tone to MIB database failed.");
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
	nBytesSent += boaWrite(wp, "<th width=\"100\"><font size=2>%s</th><th width=\"100\"><font size=2>%s</th>\n</tr>\n", multilang_bpas(Tdownstream), multilang_bpas(Tupstream));

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

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s</th>\n", multilang_bpas(Thlin_scale));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", hlin_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", hlin_us);

#ifdef CONFIG_VDSL
if(mval<MODE_VDSL1)
{
#endif /*CONFIG_VDSL*/
	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dB)</th>\n", multilang_bpas(Tloop_annu));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", latt_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", latt_us);

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dB)</th>\n", multilang_bpas(Tsig_annu));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", satt_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", satt_us);
#ifdef CONFIG_VDSL
}
#endif /*CONFIG_VDSL*/

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dB)</th>\n", multilang_bpas(Tsnr_marg));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", snr_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", snr_us);

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(Kbps)</th>\n", multilang_bpas(Tattain_rate));
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n", attr_ds);
	nBytesSent += boaWrite(wp, "<td align=center><font size=2>%s</font></td>\n</tr>\n", attr_us);

	nBytesSent += boaWrite(wp, "<tr bgcolor=#f0f0f0>\n<th align=left bgcolor=#c0c0c0><font size=2>%s(dBm)</th>\n", multilang_bpas(Tout_power));
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

		nBytesSent += boaWrite(wp, "<tr>\n<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang_bpas(Ttone_num));
		nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang_bpas("H.Real"));
		nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang_bpas("H.Image"));
		nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang_bpas("SNR"));
		nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang_bpas("QLN"));
		nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n</tr>\n", multilang_bpas("Hlog"));

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

	nBytesSent += boaWrite(wp, "<tr>\n<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang_bpas(Ttone_num));
	nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang_bpas("H.Real"));
	nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang_bpas("H.Image"));
	nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang_bpas("SNR"));
	nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n", multilang_bpas("QLN"));
	nBytesSent += boaWrite(wp, "<th width=15%% bgcolor=#c0c0c0><font size=2>%s</font></th>\n</tr>\n", multilang_bpas("Hlog"));

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
			multilang_bpas("Tone Number"));
	nBytesSent += boaWrite(wp, "<td align=center width=\"20%%\" bgcolor=\"#808080\"><font size=\"2\"><b>%s</b></font></td></tr>\n",
			multilang_bpas("Select"));

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
	nBytesSent += boaWrite(wp, "<tr>\n<th align=center width=\"10%%\" bgcolor=\"#B0B0B0\">%s</th>\n", multilang_bpas("Enable"));
	nBytesSent += boaWrite(wp, "<th align=center width=\"10%%\" bgcolor=\"#B0B0B0\">%s (0-63)</th>\n", multilang_bpas("Tone"));
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
					snprintf(tmpBuf, sizeof(tmpBuf), "%s should be (0~63)", strVal);
					goto setErr_tone;
				}

				bits |= (1<<i);
			}

			tone = -1;

			snprintf(tmpBuf, sizeof(tmpBuf), "t%d", i);
			strVal = boaGetVar(wp, tmpBuf, "");
			if ((bits & (1 << i)) && strVal) {
				if (1 != sscanf(strVal, "%d", &tone)) {
					snprintf(tmpBuf, sizeof(tmpBuf), "t%d %s not a number", i, strVal);
					goto setErr_tone;
				}

				if ((tone > 63) || (tone < 0)) {
					snprintf(tmpBuf, sizeof(tmpBuf), "%s should be (0~63)", strVal);
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

	Value = multilang_bpas((psd_measure) ? "Disable" : "Enable");

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

	strValue = boaGetVar(wp, "reset", "");	//cathy, reset stats
	if(strValue[0]) {
		if(strValue[0] - '0') {	//reset
#ifdef EMBED
			int skfd;
			struct ifreq ifr;
			if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
				perror("socket error");
			}
			else {
				if (ioctl(skfd, SIOCRESETSTAT, &ifr) < 0) {
					printf("ioctl SIOCRESETSTAT error\n");
				}
				close(skfd);
			}
			// reset sar statistics
			va_cmd("/bin/sarctl", 1, 1, "resetstats");
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
//#ifdef CTC_WAN_NAME
	//ql_xu add: for auto generate wan name
	int entryNum;
	int mibcnt;
	MIB_CE_ATM_VC_T Entry;
//#endif
	int i;
#ifdef EMBED
	int skfd;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	struct ch_stat stat;
#endif

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
#endif

//#ifdef CTC_WAN_NAME
	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
//#endif
boaWrite(wp, "<tr><font size=1>"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">%s</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\">Rx pkt</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Rx err</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Rx drop</td>\n"
	"<td align=center width=\"15%%\" bgcolor=\"#808080\">Tx pkt</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Tx err</td>\n"
	"<td align=center width=\"8%%\" bgcolor=\"#808080\">Tx drop</td></font></tr>\n",
	multilang_bpas("Interface"));

	// LAN statistics
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

#ifdef EMBED
#ifdef CONFIG_RTL8672_SAR
	// pvc statistics
	if ((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0) {
		perror("socket open error");
		return;
	}

	mysio.number = 0;

	for (i = 0; i < MAX_VC_NUM; i++) {
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if (ioctl(skfd, ATM_SAR_GETSTAT, &mysio) < 0) {
			(void)close(skfd);
			return;
		}

		if (cfg.created == 0)
			continue;

#ifdef CTC_WAN_NAME
		//check mib chain to see the channel mode of the current channel.
		int found = 0;
		char applicationname[MAX_WAN_NAME_LEN];

		for (mibcnt = 0; mibcnt < entryNum; mibcnt++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, &Entry)) {
				printf("get MIB chain error\n");
				continue;
			}

			if (Entry.enable == 0)
				continue;

			if (Entry.vpi != cfg.vpi || Entry.vci != cfg.vci)
				continue;

			//found entry in mibs, get channel mode
			cfg.rfc = Entry.cmode;

			memset(applicationname, 0, sizeof(applicationname));
			setWanName(applicationname, Entry.applicationtype);
			if (cfg.rfc == CHANNEL_MODE_BRIDGE) {
				//bridge mode
				strcat(applicationname, "B_");
			} else if (cfg.rfc == CHANNEL_MODE_PPPOE || cfg.rfc == CHANNEL_MODE_PPPOA) {
				snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
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

				found++;

				continue;
			} else {	//rt1483 rt1577
				strcat(applicationname, "R_");
			}

			sprintf(applicationname, "%s%d_%d", applicationname, cfg.vpi, cfg.vci);
#ifdef _CWMP_MIB_
			if (*Entry.WanName)
				strcpy(applicationname, Entry.WanName);
#endif

			boaWrite(wp, "<tr>"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%s</b></font></td>\n"
				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
				"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
				applicationname,
				cfg.stat.rx_pkt_cnt, cfg.stat.rx_pkt_fail,
				cfg.stat.rx_crc_error, cfg.stat.tx_desc_ok_cnt,
				cfg.stat.tx_pkt_fail_cnt, cfg.stat.send_desc_lack);

			found++;

			break;
		}

		if (found == 0) {
			printf("in pktStatsList: not found mib entry for this interface\n");
			//set to default application name....
		}
#else
		for (mibcnt = 0; mibcnt < entryNum; mibcnt++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, mibcnt, &Entry)) {
				printf("get MIB chain error\n");
				continue;
			}

			if (Entry.enable == 0)
				continue;

			if (Entry.vpi != cfg.vpi || Entry.vci != cfg.vci)
				continue;

			//found entry in mibs, get channel mode
			cfg.rfc = Entry.cmode;
			if (cfg.rfc == CHANNEL_MODE_PPPOE || cfg.rfc == CHANNEL_MODE_PPPOA) {
				snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
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
					continue;
			}

			boaWrite(wp, "<tr>"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%u_%u</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"15%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td>\n"
			"<td align=center width=\"8%%\" bgcolor=\"#C0C0C0\"><font size=\"2\"><b>%lu</b></font></td></tr>\n",
			cfg.vpi, cfg.vci, cfg.stat.rx_pkt_cnt, cfg.stat.rx_pkt_fail,
			cfg.stat.rx_crc_error, cfg.stat.tx_desc_ok_cnt,
			cfg.stat.tx_pkt_fail_cnt, cfg.stat.send_desc_lack);
			break;
		}
#endif
	}

	close(skfd);
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
		if (Entry.cmode != CHANNEL_MODE_PPPOE)
		{
			getWanName(&Entry, ifname);
		}else		// PPPoE interface
			snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));

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


#ifdef CONFIG_ETHWAN
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
		if (Entry.cmode != CHANNEL_MODE_PPPOE) {
#ifdef CONFIG_RTL_MULTI_ETH_WAN
			getWanName(&Entry, ifname);
#else
			sprintf(ifname, ALIASNAME_NAS0);
#endif
		}
		else		// PPPoE interface
			snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));

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
			strcpy(tmpBuf, "PROHIBIT: Administrator's password not set!");
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
		va_cmd(IPTABLES, 11, 1, (char *)FW_ADD, (char *)FW_INPUT, ARG_I,
		"!", LANIF, "-p", ARG_TCP, FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
		// iptables -A INPUT -j block
		va_cmd(IPTABLES, 4, 1, (char *)FW_ADD, (char *)FW_INPUT, "-j", "block");

		// iptables -t nat -A PREROUTING -i ! $LAN_IF -p TCP --dport 51003 -j DNAT --to-destination ipaddr:80
		va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)FW_ADD,	"PREROUTING",
					(char *)ARG_I, "!", (char *)LANIF,
					"-p", (char *)ARG_TCP,
					(char *)FW_DPORT, dport, "-j",
					"DNAT", "--to-destination", ipaddr);
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
		va_cmd(IPTABLES, 11, 1, (char *)FW_DEL, (char *)FW_INPUT, ARG_I,
		"!", LANIF, "-p", ARG_TCP, FW_DPORT, "80", "-j", (char *)FW_ACCEPT);
		// iptables -t nat -D PREROUTING -i ! $LAN_IF -p TCP --dport 51003 -j DNAT --to-destination ipaddr:80
		va_cmd(IPTABLES, 15, 1, "-t", "nat",
					(char *)FW_DEL,	"PREROUTING",
					(char *)ARG_I, "!", (char *)LANIF,
					"-p", (char *)ARG_TCP,
					(char *)FW_DPORT, dport, "-j",
					"DNAT", "--to-destination", ipaddr);
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
				strcpy(tmpBuf, "Set LOG Capability error!");
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
#endif // #if 0 // Mason Yu. t123

//added by xl_yue for supporting inform ITMS after finishing maintenance
void formFinishMaintenance(request * wp, char *path, char *query)
{
	char	*str, *submitUrl;
	pid_t tr069_pid;
#ifdef CONFIG_MIDDLEWARE
	pid_t tmp_pid;
	unsigned char vChar;
	mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
	if(!vChar)
	{
		//martin_zhu:send MIB_MIDWARE_INFORM_EVENT to MidProcess
		vChar = CTEVENT_ACCOUNTCHANGE;
		sendInformEventMsg2MidProcess( vChar );
	}else
#endif

	// signal tr069 to inform ITMS that maintenance is finished
	tr069_pid = read_pid("/var/run/cwmp.pid");
	if ( tr069_pid > 0) {
#ifdef CONFIG_MIDDLEWARE
		vChar = CTEVENT_ACCOUNTCHANGE;
		mib_set(MIB_MIDWARE_INFORM_EVENT,(void*)&vChar);
#endif
		kill(tr069_pid, SIGUSR1);
	} else
		goto setErr_Signal;

	submitUrl = boaGetVar(wp, "submit-url", "");
	OK_MSG1("成功:通知ITMS维护已经结束!",submitUrl); //OK:start to inform ITMS that maintenance is over!
  	return;

setErr_Signal:
	ERR_MSG("错误:找不到TR069程序!"); //ERROR:can not find TR069 pcocess!

}

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
#ifdef CTC_TELECOM_ACCOUNT
	unsigned char vChar;
#endif
	char usrName[MAX_NAME_LEN];
	char supName[MAX_NAME_LEN];
	//char tuserName[MAX_NAME_LEN]; // if pass check, use tuserName to determine priv
	//xl_yue:1:bad password;2:invalid user;3:login error for three times,forbidden;4:other has login;
	int denied = 1;
	int login_by_uspwd = -1;
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	int totalEntry, i;
#endif

	// Mason Yu. t123
	/*
	str = boaGetVar(wp, "save", "");
	if (str[0]) {
	*/
		pUser_info = search_login_list(wp);
		if(pUser_info){
			denied = 5;
			goto setErr_Signal;
		}
		username = boaGetVar(wp, "username", "");
		// Mason Yu on True
		//printf("username=%s\n", username);
		strcpy(g_login_username, username);
		#if 0
		if (!username[0] ) {
			denied = 2;
			goto setErr_Signal;
		}
		#endif
		password = boaGetVar(wp, "psd", "");
		if (!password[0] ) {
			denied = 1;
			goto setErr_Signal;
		}
	// Mason Yu. t123
	/*
	}else{
		denied = 10;
		goto setErr_Signal;
	}
	*/

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

//	if(strcmp(usrName, username)==0){
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
			//goto setErr_Signal; //check if password = suser_password
		}
		else{
			denied = 0;
			//memset(tuserName, '\0', MAX_NAME_LEN);
			//strcpy(tuserName, usrName); //save tuserName as usrName
			login_by_uspwd = 1;
			goto pass_check;
		}
//	}

#ifdef CTC_TELECOM_ACCOUNT
	if(!mib_get(MIB_CTC_ACCOUNT_ENABLE, (void *)&vChar)){
		denied = 10;
		goto setErr_Signal;
	}
	if(!vChar){
		//denied = 2;
		//if((strcmp(usrName, username)==0))
			denied = 1;
		goto setErr_Signal;
	}
#endif

	if ( !mib_get(MIB_SUSER_NAME, (void *)supName) ) {
		denied = 10;
		goto setErr_Signal;
	}

//	if(strcmp(supName, username)==0){
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
			if(strcmp(supName, username) && strcmp(usrName, username))
				denied = 2;
			goto setErr_Signal;
		}
		denied = 0;
		//memset(tuserName, '\0', MAX_NAME_LEN);
		//strcpy(tuserName, supName); //save tuserName as supName if use suserpassword
		login_by_uspwd = 0;
		goto pass_check;
//	}

	if(denied){
		denied = 2;
		goto setErr_Signal;
	}

pass_check:

#ifdef ONE_USER_LIMITED
	if(usStatus.busy){
		if(strcmp(usStatus.remote_ip_addr, wp->remote_ip_addr)){
			// if no action lasting for 5 minutes, logout
			if (time_counter - usStatus.pUser_info->last_time > 300) {
				free_from_login_list_by_ip_addr(usStatus.remote_ip_addr);
			} else {
				denied = 4;
				goto setErr_Signal;
			}
		}
	}
	if(suStatus.busy){
		if(strcmp(suStatus.remote_ip_addr, wp->remote_ip_addr)){
			// if no action lasting for 5 minutes, logout
			if (time_counter - suStatus.pUser_info->last_time > 300) {
				free_from_login_list_by_ip_addr(suStatus.remote_ip_addr);
			} else {
				denied = 4;
				goto setErr_Signal;
			}
		}
	}
#endif

	pUser_info = search_login_list(wp);
	if(!pUser_info){
		pUser_info = malloc(sizeof(struct user_info));
		pUser_info->last_time = time_counter;
		strncpy(pUser_info->remote_ip_addr, wp->remote_ip_addr, sizeof(pUser_info->remote_ip_addr));
		if(login_by_uspwd == 1){
			pUser_info->directory = strdup("/admin/index_user.html");
			pUser_info->priv = 0;//normal
#ifdef ONE_USER_LIMITED
			pUser_info->paccount = &usStatus;
			pUser_info->paccount->busy = 1;
			pUser_info->paccount->pUser_info = pUser_info;
			strncpy(pUser_info->paccount->remote_ip_addr, wp->remote_ip_addr, sizeof(pUser_info->paccount->remote_ip_addr));
#endif
		}
		else{
#ifdef CONFIG_VDSL
			pUser_info->directory = strdup("/index_vdsl.html");
#else
			pUser_info->directory = strdup("/index.html");
#endif
			pUser_info->priv = 1;//admin
#ifdef ONE_USER_LIMITED
			pUser_info->paccount = &suStatus;
			pUser_info->paccount->busy = 1;
			pUser_info->paccount->pUser_info = pUser_info;
			strncpy(pUser_info->paccount->remote_ip_addr, wp->remote_ip_addr, sizeof(pUser_info->paccount->remote_ip_addr));
#endif
		}
		//list it to user_login_list
		pUser_info->next = user_login_list;
		user_login_list = pUser_info;

		syslog(LOG_INFO, "login successful for %s from %s\n", username, wp->remote_ip_addr);

#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
		clear_ctcom_alarm(CTCOM_ALARM_LOGING_TRY_LIMIT);
#endif
	}else{
			pUser_info->last_time = time_counter;
	}

#ifdef LOGIN_ERR_TIMES_LIMITED
	free_from_errlog_list(wp);
#endif

	boaRedirectTemp(wp, "/");

//	submitUrl = boaGetVar(wp, "submit-url", "");
//	OK_MSG1("OK:login successfully!",submitUrl);
  	return;

setErr_Signal:

#ifdef LOGIN_ERR_TIMES_LIMITED
	if(denied == 1 || denied == 2){
		pErrlog_entry = search_errlog_list(wp);
		if(pErrlog_entry){
			pErrlog_entry->last_time = time_counter;
			pErrlog_entry->login_count++;
			if(pErrlog_entry->login_count % MAX_LOGIN_NUM == 0)
				denied = 3;
			if(pErrlog_entry->login_count > 10)
				syslog(LOG_ERR, "104032 logined error > 10 times\n");
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
			ERR_MSG2("错误: 密码错误!"); //ERROR:bad password!
			syslog(LOG_ERR, "login error from %s for bad password \n",wp->remote_ip_addr);
			break;
		case 2:
			ERR_MSG2("错误: 不存在的用户名"); //ERROR:invalid username!
			syslog(LOG_ERR, "login error from %s for invalid username \n",wp->remote_ip_addr);
			break;
#ifdef LOGIN_ERR_TIMES_LIMITED
		case 3:
			ERR_MSG2("错误: 连续登入错误3次, 请于1分钟后重新登入!"); //ERROR:you have logined error 3 consecutive times, please relogin 1 minute later!
			syslog(LOG_ERR, "login error from %s for having logined error three consecutive times \n",wp->remote_ip_addr);
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
			set_ctcom_alarm(CTCOM_ALARM_LOGING_TRY_LIMIT);
#endif
			break;
#endif
#ifdef ONE_USER_LIMITED
		case 4:
			ERR_MSG1("错误: 其他用户已登入, 同时间只允许一个用户登入!", "/"); //ERROR:another user have logined in using this account!only one user can login using this account at the same time!
			syslog(LOG_ERR, "login error from %s for using the same account with another user at the same time\n",wp->remote_ip_addr);
			break;
#endif
		case 5:
			ERR_MSG1("错误: 你已经登入, 请登出后再次登入!", "/"); //ERROR:you have logined! please logout at first and then login!
			syslog(LOG_ERR, "login error from %s for having logined\n",wp->remote_ip_addr);
			break;
		default:
			ERR_MSG2("错误: 网页认证错误, 请关闭视窗启重起浏览器再登入!"); //ERROR:web authentication error!please close this window and reopen your web browser to login!
			syslog(LOG_ERR, "web authentication error!\n");
			break;
		}
}

void formLogout(request * wp, char *path, char *query)
{
	if (!free_from_login_list(wp)) {
		syslog(LOG_ERR, "logout error from %s\n", wp->remote_ip_addr);
		printf("logout error\n");
	} else {
		syslog(LOG_INFO, "logout successful from %s\n",
		       wp->remote_ip_addr);
		printf("logout\n");
	}

	boaRedirect(wp, "/admin/login.asp");
}

int passwd2xmit(int eid, request * wp, int argc, char **argv)
{
#ifdef USE_BASE64_MD5_PASSWD
	boaWrite(wp, "document.cmlogin.password.value = b64_md5(document.cmlogin.password.value);");
#endif
}

#endif


// Mason Yu. t123
#if 0
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
#endif

void formPasswordSetup(request * wp, char *path, char *query)
{
	char issu = 0;
	char tmpBuf[100];
	struct user_info *pUser_info = NULL;
	//新用户名:
	char *pnewUserName = NULL;
	//旧密码:
	char *poldPasswd = NULL;
	//新密码:
	char *pnewPasswd = NULL;
	//确认密码:
	char *paffirmPasswd = NULL;
	char *stemp = NULL;
	int lineno = __LINE__;
	char suname[64];
	_TRACE_CALL;

	pUser_info = search_login_list(wp);
	issu = pUser_info && pUser_info->priv;
	if (!issu) {
		_GET_PSTR(oldPasswd, _NEED);
	}
	_GET_PSTR(newPasswd, _NEED);
	_GET_PSTR(affirmPasswd, _NEED);
	strcpy(tmpBuf, "设定错误!");
	if (strcmp(pnewPasswd, paffirmPasswd) != 0) {
		lineno = __LINE__;
		goto check_err;
	}

	if (!mib_get(MIB_USER_PASSWORD, (void *)tmpBuf)) {
		goto check_err;
	}

	if (!issu && strcmp(tmpBuf, poldPasswd) != 0) {
		strcpy(tmpBuf, "旧密码错误!");
		goto check_err;
	} else if (!mib_set(MIB_USER_PASSWORD, (void *)pnewPasswd)) {
		goto check_err;
	}
	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
#ifdef EMBED
	// Added by Mason Yu for take effect on real time
	writePasswdFile();
#endif

	_COND_REDIRECT;
check_err:
	_TRACE_LEAVEL;
	ERR_MSG(tmpBuf);
	return;
}

#ifdef CONFIG_USER_RTK_SYSLOG
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

void formSysLog(request * wp, char * path, char * query)
{
	char *stemp = "";
	int lineno = __LINE__;
	unsigned char syslogEnable;
	FILE *fp;

	_TRACE_CALL;

	FETCH_INVALID_OPT(stemp, "action", _NEED);

	if (strcmp(stemp, "clr") == 0) {	//clear all log
		/************Place your code here, do what you want to do! ************/
		mib_get(MIB_SYSLOG, &syslogEnable);
		if (syslogEnable) {
			fp = fopen("/var/config/syslogd.txt", "w");
			if (fp) {
				writeLogFileHeader(fp);
				fclose(fp);
			}
#ifdef CONFIG_USER_FLATFSD_XXX
			va_cmd("/bin/flatfsd", 1, 1, "-s");
#endif
		}
		/************Place your code here, do what you want to do! ************/
	} else if (strcmp(stemp, "saveLog") == 0){

		fp=fopen("/var/config/syslogd.txt","r");
		if ( fp == NULL ) {
				printf("System Log not exists!\n");
				goto check_err;
		}

		wp->buffer_end=0; // clear header
		boaWrite(wp, "HTTP/1.0 200 OK\n");
		boaWrite(wp, "Content-Type: application/octet-stream;\n");
		boaWrite(wp, "Content-Disposition: attachment;filename=\"messages.txt\" \n");
		boaWrite(wp, "Pragma: no-cache\n");
		boaWrite(wp, "Cache-Control: no-cache\n");
		boaWrite(wp, "\n");

		saveLogFile(wp, fp);
		fclose(fp);
		return;
	} else {
		lineno = __LINE__;
		goto check_err;
	}

	_COND_REDIRECT;
check_err:
	_TRACE_LEAVEL;
	return;
}

static char *fixupLoginfo(char *info)
{
	static char loginfo[4096] = { 0 };
	int i = 0, j = 0;
	int info_length = strlen(info);

	memset(loginfo, 0, sizeof(loginfo));

	for (i = 0; i < info_length; i++) {
		if (info[i] == '"' || info[i] == '\\') {
			loginfo[j++] = '\\';
		}
		loginfo[j++] = info[i];
	}

	return loginfo;
}

int sysLogList(int eid, request * wp, int argc, char ** argv)
{
	char *tmp1, *tmp2, tmpbuf[1024];
	FILE *fp;
	int nBytesSent = 0;
	struct access_syslog_entry entry;
	unsigned char displayLevel;
	unsigned int s, security;

	_TRACE_CALL;

	//add by ramen
	if ((fp = fopen("/var/config/syslogd.txt", "r")) == NULL)
		goto check_err;

	/* Pass the log file header */
	while (fgets(tmpbuf, sizeof(tmpbuf), fp)) {
		if (1 == strlen(tmpbuf)) {
			/* empty line, next is log */
			break;
		}
	}

	/* test whether called from mgm_log_view_sec.asp
	 * or mgm_log_view_access.asp */
	security = strstr(wp->pathname, "mgm_log_view_sec.asp") ? 1 : 0;
	mib_get(MIB_SYSLOG_DISPLAY_LEVEL, &displayLevel);

	_TRACE_POINT;
	while (fgets(tmpbuf, sizeof(tmpbuf), fp)) {
		tmpbuf[strlen(tmpbuf) - 1] = '\0';
		if (security && !strstr(tmpbuf, "500001:Access-logged"))
			continue;
		memset(&entry, 0, sizeof(entry));

		/* get dateTime */
		memcpy(entry.dateTime, tmpbuf, sizeof("YYYY-MM-DD HH:MM:SS") - 1);

		/* get severity */
		tmp1 = strstr(tmpbuf, "[") + 1;
		tmp2 = strstr(tmp1, "]");
		memcpy(entry.severity, tmp1, tmp2 - tmp1);

		if (!security) {
			for (s = 0; s < ARRAY_SIZE(log_severity); s++) {
				if (0 == strcmp(log_severity[s], entry.severity)) {
					break;
				}
			}

			if (s >= ARRAY_SIZE(log_severity) || s > displayLevel) {
				continue;
			}
		}

		/* get msg */
		entry.msg = fixupLoginfo(tmp2 + 2);

		nBytesSent += boaWrite(wp, "rcs.push(new Array(\"%s\", \"%s\", \"%s\"));\n",
				entry.dateTime, entry.severity, entry.msg);
	}

check_err:
	_TRACE_LEAVEL;
	return nBytesSent;
}

void formSysLogConfig(request * wp, char *path, char *query)
{
	//记录启用:
	unsigned char syslogEnable = 0;	//1- 启用;  0- 禁用
	//记录等级:
	unsigned char recordLevel = 0;	//0- Emergency;  1- Alert; …; 7- Debugging
	//显示等级:
	unsigned char dispLevel = 0;	//0- Emergency;  1- Alert; …; 7- Debugging
	//模式:
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
	unsigned char sysMode = 1;	//1- Local;  2- Remote;  3- Both
	unsigned long logAddr = 0;
	unsigned short logPort = 0;
#endif
	char *stemp;
	int lineno = __LINE__;

	_GET_INT(syslogEnable, _NEED);
	if (syslogEnable > 1) {
		goto check_err;
	}
	if (syslogEnable) {
		_GET_INT(recordLevel, _NEED);
		if (recordLevel > 7) {
			goto check_err;
		}
		_GET_INT(dispLevel, _NEED);
		if (dispLevel > 7) {
			goto check_err;
		}
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
		_GET_INT(sysMode, _NEED);
		if (sysMode > 3) {
			goto check_err;
		}

		if (sysMode >= 2) {
			_GET_IP(logAddr, _NEED);
			if (logAddr == 0 || logAddr == 0xFFFFFFFF) {
				goto check_err;
			}
			_GET_INT(logPort, _NEED);
			if (logPort == 0) {
				goto check_err;
			}
		}
#endif
	}

	/************Place your code here, do what you want to do! ************/
	mib_set(MIB_SYSLOG, &syslogEnable);
	mib_set(MIB_SYSLOG_LOG_LEVEL, &recordLevel);
	mib_set(MIB_SYSLOG_DISPLAY_LEVEL, &dispLevel);
#ifdef CONFIG_USER_RTK_SYSLOG_REMOTE
	mib_set(MIB_SYSLOG_MODE, &sysMode);
	if (sysMode >= 2) {
		mib_set(MIB_SYSLOG_SERVER_IP, &logAddr);
		mib_set(MIB_SYSLOG_SERVER_PORT, &logPort);
	}
#endif
	stopLog();
	if (syslogEnable)
		startLog();
	/************Place your code here, do what you want to do! ************/

	_COND_REDIRECT;

#ifdef COMMIT_IMMEDIATELY
	Commit();
#endif

check_err:
	return;
}
#endif	// of CONFIG_USER_RTK_SYSLOG

#ifdef TIME_ZONE
int timeZoneList(int eid, request * wp, int argc, char **argv)
{
	int nBytesSent = 0;
	unsigned int i, selected = 0;

	mib_get(MIB_NTP_TIMEZONE_DB_INDEX, &selected);

	for (i = 0; i < nr_tz; i++) {
		nBytesSent += boaWrite(wp, "<option value=\"%u\"%s>%s (UTC%s)</option>",
				i, (i == selected) ? " selected" : "",
				get_tz_location(i), get_tz_utc_offset(i));
	}

	return nBytesSent;
}
#endif

#if 0
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
#endif  // #if 0  // Mason Yu. t123

#ifdef E8B_NEW_DIAGNOSE
void formTr069Diagnose(request *wp, char *path, char *query)
{
	{
		/************Place your code here, do what you want to do! ************/
		pid_t  tr069_pid;
		unsigned int events;

#ifdef CONFIG_MIDDLEWARE
		pid_t tmp_pid;
		unsigned char vChar;
		mib_get(CWMP_TR069_ENABLE, (void *)&vChar);
		if (!vChar) {
		//Martin_zhu:send MIB_MIDWARE_INFORM_EVENT to MidProcess
			vChar = CTEVENT_SEND_INFORM;
			sendInformEventMsg2MidProcess( vChar );
		}
		else
#endif
		{
			tr069_pid = read_pid("/var/run/cwmp.pid");
			if ( tr069_pid > 0) {
#ifdef E8B_NEW_DIAGNOSE
				FILE *fp = NULL;
				fp = fopen(INFORM_STATUS_FILE, "w");
				if (fp) {
					fprintf(fp, "%d", INFORMING);
					fclose(fp);
				}
#endif
#ifdef CONFIG_MIDDLEWARE
				vChar = CTEVENT_SEND_INFORM;
				if(mib_set(MIB_MIDWARE_INFORM_EVENT, (void*)&vChar))
					kill(tr069_pid, SIGUSR1);
#else
				mib_get(CWMP_INFORM_EVENTCODE, &events);
				#define EC_PERIODIC 0x000004	// Must match the define in cwmp_rpc.h
				events |= EC_PERIODIC;
				mib_set(CWMP_INFORM_EVENTCODE, &events);
#endif
			}
		}
		/************Place your code here, do what you want to do! ************/
	}

	_COND_REDIRECT;
}
#endif

/*****************************
** 用户管理
*/
int initPageMgmUser(int eid, request * wp, int argc, char **argv)
{
	char issu = 0;
	struct user_info *pUser_info = NULL;
	int lineno = __LINE__;

	_TRACE_CALL;

	pUser_info = search_login_list(wp);
	issu = ((pUser_info && pUser_info->priv) ? 1 : 0);

	_PUT_BOOL(issu);

	_TRACE_LEAVEL;
	return 0;
}

void formReboot(request * wp, char * path, char * query)
{
	int lineno = __LINE__;
	char *strReset;

	_TRACE_CALL;

	boaHeader(wp);
	//--- Add timer countdown
        boaWrite(wp, "<head><META http-equiv=content-type content=\"text/html; charset=gbk\"><style>\n" \
        "#cntdwn{ border-color: white; border-width: 0px; font-size: 12pt; color: red; text-align:left; font-weight:bold; font-family: Courier;}\n" \
        "</style><script language=javascript>\n" \
        "var h = 40;\n" \
        "function stop() { clearTimeout(id); }\n"\
        "function start() { h--; if (h >= 0) { frm.time.value = h; frm.textname.value='设备重启中, 请稍候 ...'; id = setTimeout(\"start()\", 1000); }\n" \
        "if (h == 0) { window.location.reload(true); }}\n" \
        "</script></head>");
        boaWrite(wp,
        "<body bgcolor=white onLoad=\"start();\" onUnload=\"stop();\"><blockquote>" \
        "<form name=frm><b><font color=red><input type=text name=textname size=40 id=\"cntdwn\">\n" \
        "<input type=text name=time size=5 id=\"cntdwn\"></font></form></blockquote></body>" );
        //--- End of timer countdown
   	boaFooter(wp);
	boaDone(wp, 200);

	strReset = boaGetVar(wp, "reset", "");
#ifdef EMBED
	if (strReset[0]) {
#ifdef CONFIG_MIDDLEWARE
	unsigned char vChar;
	mib_get(CWMP_TR069_ENABLE, (void*)&vChar);
	if ( (vChar == 0)||(vChar == 2) ) {
		if( (sendSetDefaultFlagMsg2MidProcess() == 0)&&(sendSetDefaultRetMsg2MidIntf() == 0) )
		{
			sleep(10);	//wait reboot command from middleware
		}
	}
#endif	//end of CONFIG_MIDDLEWARE
		reset_cs_to_default(0); //fix to short reset
	}
#endif
	cmd_reboot();

check_err:
	_TRACE_LEAVEL;
	return;
}

/*****************************
** 日常应用
*/
/// This setting should sync with usbmount user tool
int listUsbDevices(int eid, request *wp, int argc, char ** argv)
{
	int errcode = 1, lineno = __LINE__;
	struct dirent **namelist;
	int i, n;

	_TRACE_CALL;

	n = scandir("/mnt", &namelist, usb_filter, alphasort);

	/* no match */
	if (n < 0)
		goto check_err;

	for (i = 0; i < n; i++) {
		boaWrite(wp, "push(new it_nr(\"%c\"" _PTS "));\n",
				namelist[i]->d_name[3],
				"path", namelist[i]->d_name);
		free(namelist[i]);
	}
	free(namelist);

check_err:
	_TRACE_LEAVEL;
	return 0;
}

/*****************************
**USB备份配置
*/
void formUSBbak(request * wp, char *path, char *query)
{
	//USB分区
	char *pusbdev = NULL, *stemp = NULL, dstname[64], usb_cfg_filename[32];
	int errcode = 1, lineno = __LINE__;
	int forcebackup = 0;	//jim used for remove exist config file in usb disk...
	int rv = 0;
	struct file_pipe pipe;
	unsigned char cpbuf[256];

	_TRACE_CALL;

	FETCH_INVALID_OPT(stemp, "action", _OPT);

	if (stemp && stemp[0]) {
		if (0 == strcmp(stemp, "en")) {
			/* setting config fast restore */
#ifdef _PRMT_USBRESTORE
			unsigned char cfgFastRestoreEnable;

			_GET_BOOL(cfgFastRestoreEnable, _NEED);

			if (mib_set
			    (MIB_USBRESTORE, (void *)&cfgFastRestoreEnable)) {
				mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
			} else {
				printf("set config fast restore failed!");
			}
#endif
			_COND_REDIRECT;
			return;
		}
	}

	if (isUSBMounted() <= 0)
		goto err_dir;

	_GET_PSTR(usbdev, _NEED);
	/************Place your code here, do what you want to do! ************/
	_GET_INT(forcebackup, _OPT);

	// create dir
	snprintf(dstname, sizeof(dstname), "/mnt/%s/%s/", pusbdev, BACKUP_DIRNAME);
	if (mkdir(dstname, 0755) && errno != EEXIST)
		goto err_dir;

	// make usb config filename (dst file)
	snprintf(usb_cfg_filename, sizeof(usb_cfg_filename), "ctce8_%s.cfg", DEVICE_MODEL_STR);
	strncat(dstname, usb_cfg_filename, sizeof(usb_cfg_filename));

	// check if dst file exist..
	if (!access(dstname, F_OK)) {
		if (!forcebackup) {
			goto err_exist;
		} else {
			printf("%s: config file exist: %s\n", __FUNCTION__,
			       dstname);
			unlink(dstname);
		}
	}
	// prepare config file..
#ifdef CONFIG_USER_XMLCONFIG
	call_cmd("/bin/xmlconfig", 2, 1, "-of", CONFIG_XMLFILE);
#elif CONFIG_USE_XML
	call_cmd("/bin/saveconfig", 0, 1);
#else
	call_cmd("/bin/saveconfig", 4, 1, "-t", "raw", "-f", CONFIG_BIN);
#endif
	pipe.buffer = cpbuf;
	pipe.bufsize = sizeof(cpbuf);
	pipe.func = encode;

#if defined(CONFIG_USER_XMLCONFIG) || defined(CONFIG_USE_XML)
	rv = file_copy_pipe(CONFIG_XMLFILE, dstname, &pipe);
	unlink(CONFIG_XMLFILE);
#else
	rv = file_copy_pipe(CONFIG_BIN, dstname, &pipe);
	unlink(CONFIG_BIN);
#endif
	if (rv != 0) {
		goto err_file;
	}
	chmod(dstname, S_IRUSR);
	sync();			/* lijian: 20080716: flush inode of config file after chmod */

	errcode = 0;
	OK_MSG1("保存成功", "/mgm_dev_usbbak.asp")

check_err:
	_TRACE_LEAVEL;
	return;

err_exist:
	printf("file already exist, remove it first\n");

	boaWrite(wp, "<html>\
<head>\
<meta http-equiv=cache-control content=\"no-cache, must-revalidate\">\
<meta http-equiv=content-type content=\"text/html; charset=gbk\">\
<meta http-equiv=content-script-type content=text/javascript>\
<style type=text/css>\
@import url(/style/default.css);\
</style>\
<script language=\"javascript\" src=\"common.js\"></script>\
<body>\
<form action=\"/boaform/admin/formUSBbak\" method=\"post\">\
<table align=\"center\"><tr><td><font color=\"red\" size=2>配置文件备份已存在!</font></td></tr></table>\
<table align=center>\
<tr align=center>\
<td colspan=2 class=actionbuttons>\
<input type=\"hidden\" name=\"forcebackup\" value=\"1\" >\
<input type=\"hidden\" name=\"usbdev\" value=\"%s\" >\
<input type=\"submit\" value=\"删除后备份\"></td>\
<td colspan=2 class=actionbuttons><input type='button' onClick='history.back()' value='退出备份'></td>\
</tr>\
</table>\
</form>\
</body>\
</html>", pusbdev);
	boaDone(wp, 200);
	return;

err_file:
	printf("fail to copy, disk full? code %d\n", rv);
	snprintf(dstname, sizeof dstname, "空间不够");
	_ERR_MSG(dstname);
	return;

err_dir:
	snprintf(dstname, sizeof dstname, "保存失败");
	_ERR_MSG(dstname);
	return;
}

/*****************************
**USB卸载
*/
void formUSBUmount(request * wp, char *path, char *query)
{
	char *pusbdev = NULL, dstname[64];
	int errcode = 1, lineno = __LINE__, ret;

	if (isUSBMounted() <= 0)
		goto err_dir;

	_GET_PSTR(usbdev, _NEED);

	// create dir
	snprintf(dstname, sizeof(dstname), "/mnt/%s", pusbdev);

	ret = umount2(dstname, MNT_DETACH);
	ret |= rmdir(dstname);

	if (ret)
		goto err_dir;

	OK_MSG1("卸载成功", "/mgm_dev_usb_umount.asp");
	return;

check_err:
err_dir:
	_ERR_MSG("卸载失败");
}

void formVersionMod(request *wp, char *path, char *query)
{
	char *strData;
	unsigned char str;
	unsigned int cnt;

	// manufacturer
	strData = boaGetVar(wp,"txt_mft","");
	if (strData[0])
	{
		if (!mib_set(RTK_DEVID_MANUFACTURER, strData)) {
			goto setErr;
		}
		else printf("Update Manufacturer to %s \n" , strData);
	}

	// OUI
	strData = boaGetVar(wp,"txt_oui","");
	if (strData[0])
	{
		if (!mib_set(RTK_DEVID_OUI, strData)) {
			goto setErr;
		}
		else printf("Update OUI to %s \n" , strData);
	}

	// Product Class
	strData = boaGetVar(wp,"txt_proclass","");
	if (strData[0])
	{
		if (!mib_set( RTK_DEVID_PRODUCTCLASS, strData)) {
			goto setErr;
		}
		else printf("Update Product Class to %s \n" , strData);
	}

	// HW Serial Number
	strData = boaGetVar(wp,"txt_serialno","");
	if (strData[0])
	{
		if (!mib_set(MIB_HW_SERIAL_NUMBER, (void *)strData)) {
			goto setErr;
		}
		else printf("Update Serial Number to %s \n" , strData);
	}

#ifdef CONFIG_USER_CWMP_TR069
	// Provisioning Code
	strData = boaGetVar(wp,"txt_provisioningcode","");
	if (strData[0])
	{
		if (!mib_set(CWMP_PROVISIONINGCODE, (void *)strData)) {
			goto setErr;
		}
		else printf("Update Provisioning Code to %s \n" , strData);
	}
#endif
	// Spec. Version
	strData = boaGetVar(wp,"txt_specver","");
	if (strData[0])
	{
		if (!mib_set(RTK_DEVINFO_SPECVER, (void *)strData)) {
			goto setErr;
		}
		else printf("Update Spec. Version to %s \n" , strData);
	}

	// Software Version
	strData = boaGetVar(wp,"txt_swver","");
	if (strData[0])
	{
		if (!mib_set(RTK_DEVINFO_SWVER, (void *)strData)) {
			goto setErr;
		}
		else printf("Update Software Version to %s \n" , strData);
	}

	// Hardware Version
	strData = boaGetVar(wp,"txt_hwver","");
	if (strData[0])
	{
		if (!mib_set(RTK_DEVINFO_HWVER, (void *)strData)) {
			goto setErr;
		}
		else printf("Update Hardware Version to %s \n" , strData);
	}
#if defined(CONFIG_GPON_FEATURE)
	//GPON SN
	strData = boaGetVar(wp,"txt_gponsn","");
	if (strData[0])
	{
		if (!mib_set(MIB_GPON_SN, (void *)strData)) {
			goto setErr;
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
			goto setErr;
		}
		if (!mib_set(MIB_ELAN_MAC_ADDR, (void *)mac)) {
			goto setErr;
		}
		else printf("Update ELAN MAC Address to %s \n" , strData);
	}

	// WAN LIMIT
	strData = boaGetVar(wp,"txt_wanlimit","");
	if(strData[0])
	{
		int limit = 0, enable = 1;

		limit = atoi(strData);
		if(limit == 0)
		{
			if (!mib_set(CWMP_CT_MWBAND_MODE, (void *)&limit))
			{
				goto setErr;
			}
			printf("Disable WAN Limit\n");
		}
		else
		{
			if (!mib_set(CWMP_CT_MWBAND_MODE, (void *)&enable))
				goto setErr;
			if (!mib_set(CWMP_CT_MWBAND_NUMBER, (void *)&limit))
				goto setErr;

			printf("Enable WAN Limit, num=%d\n", limit);
		}
	}

	// LOID Register Status
	strData = boaGetVar(wp,"txt_reg_status","");
	if (strData[0])
	{
		cnt = strtoul(strData,NULL,10);
		if (!mib_set(CWMP_USERINFO_STATUS, (void *)&cnt)) {
			goto setErr;
		}
		else printf("Update LOID Status to %d \n" , cnt);
	}

	// LOID Register Result
	strData = boaGetVar(wp,"txt_reg_result","");
	if (strData[0])
	{
		cnt = strtoul(strData,NULL,10);
		if (!mib_set(CWMP_USERINFO_RESULT, (void *)&cnt)) {
			goto setErr;
		}
		else printf("Update LOID Result to %d \n" , cnt);
	}

	// CWMP_CONFIGURABLE
	strData = boaGetVar(wp,"txt_cwmp_conf","");
	if (strData[0])
	{
		unsigned char enable = 0;

		enable = atoi(strData);
		if (!mib_set(CWMP_CONFIGURABLE, (void *)&enable)) {
			goto setErr;
		}
		else printf("Update CWMP_CONFIGURABLE to %d \n" , enable);
	}


	mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	mib_update(HW_SETTING, CONFIG_MIB_ALL);


	strData = boaGetVar(wp, "submit-url", "");
	if (strData[0])
		boaRedirect(wp, strData);
	else
		boaDone(wp, 200);


	return;

setErr:
	ERR_MSG("Error Setting...");
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
	strcpy(tmpBuf, "OK");
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
	system("/bin/omcicli mib reset");
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
		strcpy(tmpBuf, "Allocate buffer failed!");
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

#include "mibtbl.h"
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/file.h>
#include <rtk/utility.h>
#ifdef EMBED
#include <config/autoconf.h>
#else
#include "../../../../config/autoconf.h"
#endif


#define FTP_ACCOUNT_FILE	"/var/passwd.ftp"
#define SMBD_ACCOUNT_FILE	"/var/passwd.smb"
#define SMBD_PASSWD_FILE	"/etc/samba/smbpasswd"
#define SMBD_CONF_FILE		"/etc/smb.conf"

#ifdef CONFIG_MULTI_FTPD_ACCOUNT
/*
 * /etc/passwd.ftp format same to /etc/passwd
 */
void ftpd_account_change(void)
{
	FILE *fp;
	MIB_CE_FTP_ACCOUNT_T entry;
	int mibtotal=0, i;
	char userPass[MAX_NAME_LEN];
	char *xpass;
	unsigned char anonymous_exist=0;

	fp = fopen(FTP_ACCOUNT_FILE, "w");
	if (!fp)
		return;
	
	mibtotal = mib_chain_total(MIB_FTP_ACCOUNT_TBL);
	for (i=0; i<mibtotal; i++)
	{
		if (!mib_chain_get(MIB_FTP_ACCOUNT_TBL, i, (void *)&entry))
			continue;

		if (!entry.username[0])//account is invalid
			continue;
		
		memset(userPass, 0, sizeof(userPass));
		memcpy(userPass, entry.password, MAX_NAME_LEN);

		xpass = crypt(userPass, "$1$");
		
		if (xpass)
			fprintf(fp, "%s:%s:0:0::%s:%s\n", entry.username, xpass, PW_HOME_DIR, PW_CMD_SHELL);
	}

	mib_get(MIB_FTP_ANNONYMOUS, (void *)&anonymous_exist);
	if (anonymous_exist) {
		xpass = crypt("~!@#$%^&*()_+abcdefg", "$1$");
		if (xpass)
			fprintf(fp, "%s:%s:0:0::%s:%s\n", "anonymous", xpass, PW_HOME_DIR, PW_CMD_SHELL);
	}
	
	fclose(fp);
	chmod(PW_HOME_DIR, 0x1fd);	// let owner and group have write access
}
#endif//CONFIG_MULTI_FTPD_ACCOUNT

#ifdef CONFIG_MULTI_SMBD_ACCOUNT
void smbd_account_change(void)
{
	FILE *fp;
	MIB_CE_SMB_ACCOUNT_T entry;
	int mibtotal=0, i;
	char userPass[MAX_NAME_LEN];
	char *xpass;
	unsigned char anonymous_exist=0;
	char cmdStr[100];

	/* 1. create passwd for smbd */
	fp = fopen(SMBD_ACCOUNT_FILE, "w");
	if (!fp)
		return;
	
	mibtotal = mib_chain_total(MIB_SMBD_ACCOUNT_TBL);
	for (i=0; i<mibtotal; i++)
	{
		if (!mib_chain_get(MIB_SMBD_ACCOUNT_TBL, i, (void *)&entry))
			continue;

		if (!entry.username[0])//account is invalid
			continue;
		
		memset(userPass, 0, sizeof(userPass));
		memcpy(userPass, entry.password, MAX_NAME_LEN);

		xpass = crypt(userPass, "$1$");
		
		if (xpass)
			fprintf(fp, "%s:%s:0:0::%s:%s\n", entry.username, xpass, PW_HOME_DIR, PW_CMD_SHELL);
	}
	fprintf(fp, "%s:%s:0:0::/tmp:/dev/null\n", "nobody", "x");
	fclose(fp);

	/* 2. create /etc/smb.conf */
	fp = fopen(SMBD_CONF_FILE, "w");
	if (!fp)
		return;

	mib_get(MIB_SMB_ANNONYMOUS, (void *)&anonymous_exist);
	fprintf(fp, "#\n");
	fprintf(fp, "# Realtek Semiconductor Corp.\n");
	fprintf(fp, "#\n");
	fprintf(fp, "# Tony Wu (tonywu@realtek.com)\n");
	fprintf(fp, "# Jan. 10, 2011\n");
	fprintf(fp, "\n");
	fprintf(fp, "[global]\n");
	fprintf(fp, "\t# netbios name = Realtek\n");
	fprintf(fp, "\t# server string = Realtek Samba Server\n");
	fprintf(fp, "\tsyslog = 10\n");
	fprintf(fp, "\tsocket options = TCP_NODELAY\n");
	fprintf(fp, "\t# unix charset = ISO-8859-1\n");
	fprintf(fp, "\t# preferred master = no\n");
	fprintf(fp, "\t# domain master = no\n");
	fprintf(fp, "\t# local master = yes\n");
	fprintf(fp, "\t# os level = 20\n");
	if (anonymous_exist)
		fprintf(fp, "\tsecurity = share\n");
	else
	{
		fprintf(fp, "\tsecurity = user\n");
		fprintf(fp, "\tencrypt passwords = true\n");
		fprintf(fp, "\tpassdb backend = smbpasswd\n");
		fprintf(fp, "\tsmb passwd file = %s\n", SMBD_PASSWD_FILE);
	}
	fprintf(fp, "\t# guest account = admin\n");
	fprintf(fp, "\tdeadtime = 15\n");
	fprintf(fp, "\tstrict sync = no\n");
	fprintf(fp, "\tsync always = no\n");
	fprintf(fp, "\tdns proxy = no\n");
	fprintf(fp, "\tinterfaces = lo, br0\n");
	if (!anonymous_exist)
		fprintf(fp, "\tusershare allow guests = no\n");
	fprintf(fp, "\n");
	
	if (anonymous_exist)
	{
		fprintf(fp, "[mnt]\n");
		fprintf(fp, "\tcomment = File Server\n");
		fprintf(fp, "\tpath = /mnt\n");
		fprintf(fp, "\tpublic = yes\n");
		fprintf(fp, "\twritable = yes\n");
		fprintf(fp, "\tprintable = no\n");
		fprintf(fp, "\tcreate mask = 0644\n");
		fprintf(fp, "\tguest ok = yes\n");
	}
	else
	{
		for (i=0; i<mibtotal; i++)
		{
			if (!mib_chain_get(MIB_SMBD_ACCOUNT_TBL, i, (void *)&entry))
				continue;
		
			if (!entry.username[0])//account is invalid
				continue;
			
			fprintf(fp, "[%s]\n", entry.username);
			fprintf(fp, "\tcomment = %s\n", entry.username);
			fprintf(fp, "\tpath = /mnt\n");
			fprintf(fp, "\tvalid users = %s\n", entry.username);
			fprintf(fp, "\tpublic = yes\n");
			fprintf(fp, "\twritable = yes\n");
			fprintf(fp, "\tprintable = no\n");
			fprintf(fp, "\tguest ok = no\n");
			fprintf(fp, "\n");
		}
	}
	fclose(fp);

	/* 3. create passwd file, /etc/samba/smbpasswd */
	fp = fopen(SMBD_PASSWD_FILE, "w");
	if (!fp)
		return;
	fprintf(fp, "nobody:0:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX:[UD         ]:LCT-00000000:nobody\n");
	fclose(fp);

	mibtotal = mib_chain_total(MIB_SMBD_ACCOUNT_TBL);
	for (i=0; i<mibtotal; i++)
	{
		if (!mib_chain_get(MIB_SMBD_ACCOUNT_TBL, i, (void *)&entry))
			continue;

		if (!entry.username[0])//account is invalid
			continue;
		
		snprintf(cmdStr, 100, "smbpasswd %s %s", entry.username, entry.password);
		va_cmd("/bin/sh", 2, 1, "-c", cmdStr);
	}
}
#endif//end of CONFIG_MULTI_SMBD_ACCOUNT

#ifdef CONFIG_MULTI_FTPD_ACCOUNT
/*
 * FUNCTION : enable_ftpd
 * AUTHOR   : ql_xu
 * DATE     : 20151209
 * PARAMETER: enable   0: close
 *                     1: local open
 *                     2: remote open
 *                     1: both local and remote open
 * RETURN   : -1 - mib chain get fail
 *            -2 - unknown parameter
 *             0 - ftpd not support
 *             1 - success
 */
int enable_ftpd(int enable)
{
#ifdef CONFIG_USER_FTPD_FTPD
	MIB_CE_ACC_T accEntry;

	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&accEntry))
		return -1;

	switch (enable)
	{
	case 0:
		accEntry.ftp = 0;
		break;
	case 1:
		accEntry.ftp = 0x2;
		break;
	case 2:
		accEntry.ftp = 0x1;
		break;
	case 3:
		accEntry.ftp = 0x3;
		break;
	default:
		return -2;
	}

	remote_access_modify( accEntry, 0 );
	remote_access_modify( accEntry, 1 );

	return 1;
#else
	return 0;
#endif
}

/*
 * FUNCTION : get_ftpd_capability
 * AUTHOR   : ql_xu
 * DATE     : 20151209
 * PARAMETER: none
 * RETURN   : 0 - disabled
 *            1 - local access enabled
 *            2 - remote access enabled
 *            3 - both local and remote access enabled
 *           -1 - failed to get ftpd capability
 */
int get_ftpd_capability(void)
{
	MIB_CE_ACC_T accEntry;

	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&accEntry))
		return -1;

	if ((accEntry.ftp & 0x3) == 0)
		return 0;
	if ((accEntry.ftp & 0x3) == 0x1)
		return 2;
	if ((accEntry.ftp & 0x3) == 0x2)
		return 1;
	if ((accEntry.ftp & 0x3) == 0x3)
		return 3;
}

/*
 * FUNCTION : enable_anonymous_ftpd_account
 * AUTHOR   : ql_xu
 * DATE     : 20151209
 * PARAMETER: enable  0: not allow    1: allow
 * RETURN   : 0 - fail
 *            1 - success
 */
int enable_anonymous_ftpd_account(int enable)
{
	unsigned char vChar;

	vChar = !!enable;
	if (!mib_set(MIB_FTP_ANNONYMOUS, (void *)&vChar))
		return 0;

	ftpd_account_change();
	
	return 1;
}

/*
 * FUNCTION : add_ftpd_account
 * AUTHOR   : ql_xu
 * DATE     : 20151209
 * PARAMETER: username/passwd
 * RETURN   : 0 - fail
 *            1 - success
 */
int add_ftpd_account(char *username, char *passwd)
{
	MIB_CE_FTP_ACCOUNT_T entry;
	int mibtotal=0, i;

	if (!username || !passwd)//username should not be NULL
		return 0;
	
	mibtotal = mib_chain_total(MIB_FTP_ACCOUNT_TBL);
	for (i=0; i<mibtotal; i++)
	{
		if (!mib_chain_get(MIB_FTP_ACCOUNT_TBL, i, (void *)&entry))
			continue;

		if (!strcmp(entry.username, username))//account already exist
			return 0;
	}

	memset(&entry, 0, sizeof(entry));
	snprintf(entry.username, MAX_NAME_LEN, "%s", username);
	snprintf(entry.password, MAX_NAME_LEN, "%s", passwd);

	if (!mib_chain_add(MIB_FTP_ACCOUNT_TBL, (void *)&entry))
		return 0;

	ftpd_account_change();

	return 1;
}

/*
 * FUNCTION : clear_ftpd_account
 * AUTHOR   : ql_xu
 * DATE     : 20151209
 * PARAMETER: none
 * RETURN   : 0 - fail
 *            1 - success
 */
int clear_ftpd_account(void)
{
	if (!mib_chain_clear(MIB_FTP_ACCOUNT_TBL))
		return 0;

	ftpd_account_change();
	
	return 1;
}
#endif//end of CONFIG_MULTI_FTPD_ACCOUNT

#ifdef CONFIG_MULTI_SMBD_ACCOUNT
/*
 * FUNCTION : enable_smbd
 * AUTHOR   : ql_xu
 * DATE     : 20151210
 * PARAMETER: enable   0: close
 *                     1: local open
 *                     2: remote open
 *                     1: both local and remote open
 * RETURN   : -1 - mib chain get fail
 *            -2 - unknown parameter
 *             0 - smbd not support
 *             1 - success
 */
int enable_smbd(int enable)
{
#ifdef CONFIG_USER_SAMBA
	MIB_CE_ACC_T accEntry;

	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&accEntry))
		return -1;

	switch (enable)
	{
	case 0:
		accEntry.smb = 0;
		break;
	case 1:
		accEntry.smb = 0x2;
		break;
	case 2:
		accEntry.smb = 0x1;
		break;
	case 3:
		accEntry.smb = 0x3;
		break;
	default:
		return -2;
	}

	remote_access_modify( accEntry, 0 );
	remote_access_modify( accEntry, 1 );

	return 1;
#else
	return 0;
#endif
}

/*
 * FUNCTION : get_smbd_capability
 * AUTHOR   : ql_xu
 * DATE     : 20151210
 * PARAMETER: none
 * RETURN   : 0 - disabled
 *            1 - local access enabled
 *            2 - remote access enabled
 *            3 - both local and remote access enabled
 *           -1 - failed to get smbd capability
 */
int get_smbd_capability(void)
{
	MIB_CE_ACC_T accEntry;

	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&accEntry))
		return -1;

	if ((accEntry.smb & 0x3) == 0)
		return 0;
	if ((accEntry.smb & 0x3) == 0x1)
		return 2;
	if ((accEntry.smb & 0x3) == 0x2)
		return 1;
	if ((accEntry.smb & 0x3) == 0x3)
		return 3;
}

/*
 * FUNCTION : enable_anonymous_smbd_account
 * AUTHOR   : ql_xu
 * DATE     : 20151210
 * PARAMETER: enable  0: not allow    1: allow
 * RETURN   : 0 - fail
 *            1 - success
 */
int enable_anonymous_smbd_account(int enable)
{
	unsigned char vChar;

	vChar = !!enable;
	if (!mib_set(MIB_SMB_ANNONYMOUS, (void *)&vChar))
		return 0;

	smbd_account_change();
	
	return 1;
}

/*
 * FUNCTION : add_smbd_account
 * AUTHOR   : ql_xu
 * DATE     : 20151210
 * PARAMETER: username/passwd
 * RETURN   : 0 - fail
 *            1 - success
 */
int add_smbd_account(char *username, char *passwd)
{
	MIB_CE_SMB_ACCOUNT_T entry;
	int mibtotal=0, i;

	if (!username || !passwd)//username should not be NULL
		return 0;
	
	mibtotal = mib_chain_total(MIB_SMBD_ACCOUNT_TBL);
	for (i=0; i<mibtotal; i++)
	{
		if (!mib_chain_get(MIB_SMBD_ACCOUNT_TBL, i, (void *)&entry))
			continue;

		if (!strcmp(entry.username, username))//account already exist
			return 0;
	}

	memset(&entry, 0, sizeof(entry));
	snprintf(entry.username, MAX_NAME_LEN, "%s", username);
	snprintf(entry.password, MAX_NAME_LEN, "%s", passwd);

	if (!mib_chain_add(MIB_SMBD_ACCOUNT_TBL, (void *)&entry))
		return 0;

	smbd_account_change();

	return 1;
}

/*
 * FUNCTION : clear_smbd_account
 * AUTHOR   : ql_xu
 * DATE     : 20151210
 * PARAMETER: none
 * RETURN   : 0 - fail
 *            1 - success
 */
int clear_smbd_account(void)
{
	if (!mib_chain_clear(MIB_SMBD_ACCOUNT_TBL))
		return 0;

	smbd_account_change();
	
	return 1;
}
#endif//end of CONFIG_MULTI_SMBD_ACCOUNT

/*
 * FUNCTION : enable_httpd
 * AUTHOR   : ql_xu
 * DATE     : 20151214
 * PARAMETER: enable   0: close
 *                     1: local open
 *                     2: remote open
 *                     1: both local and remote open
 * RETURN   : -1 - mib chain get fail
 *            -2 - unknown parameter
 *             0 - smbd not support
 *             1 - success
 */
int enable_httpd(int enable)
{
	MIB_CE_ACC_T accEntry;

	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&accEntry))
		return -1;

	switch (enable)
	{
	case 0:
		accEntry.web = 0;
		break;
	case 1:
		accEntry.web = 0x2;
		break;
	case 2:
		accEntry.web = 0x1;
		break;
	case 3:
		accEntry.web = 0x3;
		break;
	default:
		return -2;
	}

	remote_access_modify( accEntry, 0 );
	remote_access_modify( accEntry, 1 );

	return 1;
}

/*
 * FUNCTION : set_httpd_password
 * AUTHOR   : ql_xu
 * DATE     : 20151214
 * PARAMETER: username/passwd
 * RETURN   : 0 - fail
 *            1 - success
 */
int set_httpd_password(char *passwd)
{
	if (!passwd)//password should not be NULL
		return 0;
	
	mib_set(MIB_USER_PASSWORD, (void *)passwd);

#ifdef EMBED
	// Added by Mason Yu for take effect on real time
	//writePasswdFile();
#endif

	return 1;
}


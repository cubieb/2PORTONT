#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <linux/if_bridge.h>
#include <sys/param.h>
#include <net/route.h>
#include <pwd.h>
#ifdef HAVE_SHADOW_H
#include <shadow.h>
#endif

#include "mib.h"
#include "adsl_drv.h"
#include "utility.h"
#include "../defs.h"
#ifdef USE_LIBMD5
#include <libmd5wrapper.h>
#else
#include "../md5.h"
#endif //USE_LIBMD5
#include <crypt.h>
#include <time.h>
#include <sys/stat.h>
#include "cfgutility.h"


static char strbuf[256];
static const char *emptystr = "";
static const char *menuBar = "------------------------------------------------------------";
static const char *messageBar = "-------------------------------------------------------------------------";
static const char IF_UP[] = "up";
static const char IF_DOWN[] = "down";
static const char IF_NA[] = "n/a";
static const char IF_DISABLE[] = "Disable";
static const char IF_ENABLE[]= "Enable";
static const char IF_ON[] = "On";
static const char IF_OFF[] = "Off";
static const char *strQos[] = { "UBR", "CBR", "nrt-VBR", "rt-VBR" };
static const char *g_pszMode[] = { "br1483", "mer1483", "PPPoE", "PPPoA", "rt1483"
                                   #ifdef CONFIG_ATM_CLIP
                                   , "rt1577"
                                   #endif
                                  };


#define MENU_LINE	printf("%s\n", menuBar);
#define MSG_LINE		printf("%s\n", messageBar);

/***************************************************************************/

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int i, j = 0;

	for (i = 0; i < len; i += 2)
	{
		tmpBuf[0] = string[i];
		tmpBuf[1] = string[i+1];
		tmpBuf[2] = 0;

		if (!isxdigit(tmpBuf[0]) || !isxdigit(tmpBuf[1]))
			return 0;

		key[j++] = (unsigned char)strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}

char *getMibInfo(int id) {
	if (getMIB2Str(id, strbuf) == 0)
		return strbuf;

	return (char *)emptystr;
}

int getInputNum()
{
	char buf[128];
	int num;

	fgets((char *)buf, 127, stdin);
	if ( buf[0]==0) return(0);
	//printf("Enter buf= %s\n", buf);
	if(sscanf( buf, "%d", &num)==0) return(0);
	return( num);
}

int getInputStr( char *value, int maxlen, char *pcIsInputInvalid)
{
	char buf[128];
	int num;

	value[0]=0;
	fgets((char *)buf, 127, stdin);
	buf[127] = 0;
	if (buf[strlen(buf)-1]=='\n')
		buf[strlen(buf)-1] = 0;

	if (( buf[0]==0) || ( strlen(buf) > maxlen))
	{
		if (NULL != pcIsInputInvalid)
		{
			*pcIsInputInvalid = 1;
		}

		printf("Invalid value!\n");
		return(0);
	}

	strncpy(value, buf, maxlen);

	if (NULL != pcIsInputInvalid)
	{
		*pcIsInputInvalid = 0;
	}

	return( strlen(value));
}

int getTypedInputDefault(int type, char *prompt, void *rv, void *arg1, void *arg2) {
	while (1) {

		printf(prompt);
		strbuf[0] = 0;
		if (INPUT_TYPE_STRING == type) {
			char *pc;
			fgets((char *)rv, (int)arg1, stdin);
			pc = (char *)rv;
			pc[strlen(pc)-1] = 0; // remove the ending LF
			return 1;
		}
		fgets(strbuf, sizeof(strbuf) - 1, stdin);
		strbuf[strlen(strbuf)-1] = 0; // remove the ending LF
		//printf("input len %d\n", strlen(strbuf));
		if (strlen(strbuf) == 0)
			return -2;

		switch (type) {
		case INPUT_TYPE_IPADDR:
			if (inet_aton(strbuf, rv))
				return 1;
			break;

		case INPUT_TYPE_IPMASK:
			if (inet_aton(strbuf, rv))
				return 1;
			break;

		case INPUT_TYPE_UINT:
			do {
				unsigned int num;
				unsigned int min, max;
				if (1 != sscanf(strbuf, "%u", &num))
					break;
				if (arg1) {
					min = *(unsigned int *)arg1;
					if (num < min)
						break;
				}

				if (arg2) {
					max = *(unsigned int *)arg2;
					if (num > max)
						break;
				}
				*(unsigned int *)rv = num;
				return 1;
			} while (0);
			break;

		default:
			return (-1);
		}
	} // while

	return 0;
}
int getTypedInput(int type, char *prompt, void *rv, void *arg1, void *arg2) {
	while (1) {

		printf(prompt);
		strbuf[0] = 0;
		if (INPUT_TYPE_STRING == type) {
			char *pc;
			int len;

			len = (int)arg1;
			fgets((char *)rv, len, stdin);
			pc = (char *)rv;
			pc[len-1] = 0;
			if (pc[strlen(pc)-1]=='\n')
				pc[strlen(pc)-1] = 0;
			if (strlen(pc) == 0)
				return 0;
			return 1;
		}
		fgets(strbuf, sizeof(strbuf) - 1, stdin);
		strbuf[strlen(strbuf)-1] = 0; // remove the ending LF
		//printf("input len %d\n", strlen(strbuf));
		//if (strlen(strbuf) == 0)
		if (strlen(strbuf) == 0  && type != INPUT_TYPE_INT)
			return 0;

		switch (type) {
		case INPUT_TYPE_IPADDR:
			if (!isValidIpAddr(strbuf)) {
				printf("Invalid IP address %s.\n", strbuf);
				return 0;
			}
			if (inet_aton(strbuf, rv))
				return 1;
			break;

		case INPUT_TYPE_IPMASK:
			if (!isValidNetmask(strbuf, 1)) {
				printf("Invalid subnet mask %s.\n", strbuf);
				return 0;
			}
			if (inet_aton(strbuf, rv))
				return 1;
			break;

		case INPUT_TYPE_UINT:
			do {
				unsigned int num;
				unsigned int min, max;
				if (1 != sscanf(strbuf, "%u", &num))
					break;
				if (arg1) {
					min = *(unsigned int *)arg1;
					if (num < min)
						break;
				}

				if (arg2) {
					max = *(unsigned int *)arg2;
					if (num > max)
						break;
				}
				*(unsigned int *)rv = num;
				return 1;
			} while (0);
			break;

			case INPUT_TYPE_INT:
				do {
					int num;
					int min, max;
					if (strlen(strbuf) == 0)
						return -1;
					if (1 != sscanf(strbuf, "%d", &num))
						break;
					if (arg1) {
						min = *(int *)arg1;
						if (num < min)
							break;
					}

					if (arg2) {
						max = *(int *)arg2;
						if (num > max)
							break;
					}
					*(int *)rv = num;
					return 1;
				} while (0);
				break;

		default:
			return (-1);
		}
	} // while

	return 0;
}

int getInputIpAddr(char *prompt, struct in_addr *rv) {
	return (getTypedInput(INPUT_TYPE_IPADDR, prompt, (void *)rv, 0, 0));
}

int getInputIpMask(char *prompt, struct in_addr *rv) {
	return (getTypedInput(INPUT_TYPE_IPMASK, prompt, (void *)rv, 0, 0));
}

int getInputString(char *prompt, char *rv, int len) {
	return (getTypedInput(INPUT_TYPE_STRING, prompt, (void *)rv, (void *)len, 0));
}

int getInputUint(char *prompt, unsigned int *rv, unsigned int *min, unsigned int *max) {
	return (getTypedInput(INPUT_TYPE_UINT, prompt, (void *)rv, min, max));
}

int getInputInt(char *prompt, int *rv, int *min, int *max) {
	return (getTypedInput(INPUT_TYPE_INT, prompt, (void *)rv, min, max));
}

int getInputOption(unsigned int *rv, unsigned int min0, unsigned int max0) {
	char buf[32];
	int min=min0, max=max0;

	sprintf( buf, "Enter the option(%d-%d): ", min, max);
	return (getInputUint( buf, rv, &min, &max));
}

int mac17ToMac6(char *mac17, char *mac6)
{
	int i;

	for (i=0; i<17; i++){
		if ((i+1)%3 != 0)
			mac17[i-(i+1)/3] = mac17[i];
	}
	mac17[12] = '\0';
	if (strlen(mac17) != 12  || !string_to_hex(mac17, mac6, 12) || !isValidMacAddr(mac6)) {
		printf(strInvdMACAddr);
		return -1;
	}

	return 0;
}

static char base64chars[64] = "abcdefghijklmnopqrstuvwxyz"
                              "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
/*
 * Name: base64encode()
 *
 * Description: Encodes a buffer using BASE64.
 */
void base64encode(unsigned char *from, char *to, int len)
{
	while (len) {
		unsigned long k;
		int c;

		c = (len < 3) ? len : 3;
		k = 0;
		len -= c;
		while (c--)
			k = (k << 8) | *from++;
		*to++ = base64chars[ (k >> 18) & 0x3f ];
		*to++ = base64chars[ (k >> 12) & 0x3f ];
		*to++ = base64chars[ (k >> 6) & 0x3f ];
		*to++ = base64chars[ k & 0x3f ];
	}
	*to++ = 0;
}

void calPasswdMD5(char *pass, char *passMD5)
{
	char temps[0x100],*pwd;
	struct MD5Context mc;
 	unsigned char final[16];
	char encoded_passwd[0x40];
	//char *pass="user";

  	/* Encode password ('pass') using one-way function and then use base64
	 encoding. */

	MD5Init(&mc);
	{

	//printf("calPasswdMD5: pass=%s\n", pass);
	MD5Update(&mc, pass, strlen(pass));
	}
	MD5Final(final, &mc);
	strcpy(encoded_passwd,"$1$");
	base64encode(final, encoded_passwd+3, 16);
        //printf("encoded_passwd=%s for %s\n",encoded_passwd, pass);

        strcpy(passMD5, encoded_passwd);

}

#ifdef EMBED
#ifndef MULTI_USER_PRIV
// Added by Mason Yu(2 level)
void writePasswdFile()
{
	FILE *fp, *fp2;
	char suPasswd[MAX_NAME_LEN], usPasswd[MAX_NAME_LEN];
	char suPasswdMD5[50], usPasswdMD5[50];
	char suName[MAX_NAME_LEN];
	char usName[MAX_NAME_LEN];
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T entry;
	unsigned int totalEntry;
	int i;

	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
#endif

	// Added by Mason Yu for write superUser into Current Setting
	if ( !mib_get(MIB_SUSER_NAME, (void *)suName) ) {
		printf("ERROR: Get superuser name from MIB database failed.\n");
		return;
	}

	if ( !mib_get(MIB_USER_NAME, (void *)usName) ) {
		printf("ERROR: Get user name from MIB database failed.\n");
		return;
	}

	if ( !mib_get(MIB_SUSER_PASSWORD, (void *)suPasswd) ) {
		printf("ERROR: Set superuser password to MIB database failed.\n");
		return;
	}

	if ( !mib_get(MIB_USER_PASSWORD, (void *)usPasswd) ) {
		printf("ERROR: Set superuser password to MIB database failed.\n");
		return;
	}

	calPasswdMD5(suPasswd, suPasswdMD5);
	calPasswdMD5(usPasswd, usPasswdMD5);
	//calPasswdMD5("realtek", PasswdMD5);
	//printf("PasswdMD5=%s\n", PasswdMD5);


	// boa.passwd  "/var/snmpComStr.conf"
	if ((fp = fopen("/var/boaUser.passwd", "w")) == NULL)
	{
		printf("***** Open file /var/boaUser.passwd failed !\n");
		return;
	}

	if ((fp2 = fopen("/var/boaSuper.passwd", "w")) == NULL)
	{
		printf("***** Open file /var/boaSuper.passwd failed !\n");
		return;
	}

	fprintf(fp, "%s:%s\n", usName, usPasswdMD5);
	fprintf(fp, "%s:%s\n", suName, suPasswdMD5);
	fprintf(fp2, "%s:%s\n", suName, suPasswdMD5);

#ifdef ACCOUNT_CONFIG
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&entry)) {
			printf("ERROR: Get account configuration information from MIB database failed.\n");
			fclose(fp);
			fclose(fp2);
			return;
		}
		calPasswdMD5(entry.userPassword, usPasswdMD5);

		switch(entry.privilege) {
			case PRIV_USER:
				fprintf(fp, "%s:%s\n", entry.userName, usPasswdMD5);
				break;

			case PRIV_ENG:
				break;

			case PRIV_ROOT:
				fprintf(fp, "%s:%s\n", entry.userName, usPasswdMD5);
				fprintf(fp2, "%s:%s\n", entry.userName, usPasswdMD5);
				break;

			default:
				printf("Wrong privilege!\n");
				break;
		}
	}
#endif

	fclose(fp);
	fclose(fp2);

	#ifdef SUPPORT_AUTH_DIGEST
	if(!(fp = fopen("/var/DigestUser.passwd", "w")))
	{
		printf("***** Open file /var/DigestUser.passwd failed !\n");
		return;
	}
	fprintf(fp, "%s\n%s\n%s\n%s\n", usName, usPasswd, suName, suPasswd);
	fclose(fp);

	if(!(fp = fopen("/var/DigestSuper.passwd", "w")))
	{
		printf("***** Open file /var/DigestSuper.passwd failed!\n");
		return;
	}
	fprintf(fp, "%s\n%s\n", suName, suPasswd);
	fclose(fp);
	#endif


	#if 0 //def _CWMP_MIB_
	{
		char PasswdMD5[256+1];
		char *tmpStr = PasswdMD5;
		if ((fp = fopen("/var/digest.passwd", "w")) == NULL)
		{
			printf("***** Open file /var/digest.passwd failed !\n");
			return;
		}
		mib_get(CWMP_CONREQ_USERNAME, (void *)tmpStr);
		fprintf(fp, "%s\n", tmpStr);
		mib_get(CWMP_CONREQ_PASSWORD, (void *)tmpStr);
		fprintf(fp, "%s\n", tmpStr);
		fclose(fp);
        }
	#endif
}
#else
void writePasswdFile()
{
	FILE *fp;
#ifdef SUPPORT_AUTH_DIGEST
	FILE *fpd;
#endif
	char suName[MAX_NAME_LEN];
	char usName[MAX_NAME_LEN];
	char suPasswd[MAX_NAME_LEN], usPasswd[MAX_NAME_LEN];
	char suPasswdMD5[50], usPasswdMD5[50];
	MIB_CE_ACCOUNT_CONFIG_T entry;
	unsigned int totalEntry;
	int i;

	if (!mib_get(MIB_SUSER_NAME, (void *)suName)) {
		printf("ERROR: Get superuser name from MIB database failed.\n");
		return;
	}
	if (!mib_get(MIB_USER_NAME, (void *)usName)) {
		printf("ERROR: Get user name from MIB database failed.\n");
		return;
	}
	if (!mib_get(MIB_SUSER_PASSWORD, (void *)suPasswd)) {
		printf("ERROR: Set superuser password to MIB database failed.\n");
		return;
	}
	if (!mib_get(MIB_USER_PASSWORD, (void *)usPasswd)) {
		printf("ERROR: Set user password to MIB database failed.\n");
		return;
	}
	calPasswdMD5(suPasswd, suPasswdMD5);
	calPasswdMD5(usPasswd, usPasswdMD5);
#ifdef SUPPORT_AUTH_DIGEST
	if(!(fpd = fopen("/var/DigestUser.passwd", "w")))
	{
		printf("***** Open file /var/DigestUser.passwd failed !\n");
		return;
	}
	fprintf(fpd, "%s\n%s\n%s\n%s\n", usName, usPasswd, suName, suPasswd);
#endif
	if ((fp = fopen("/var/boaUser.passwd", "w")) == NULL) {
		printf("***** Open file /var/boaUser.passwd failed !\n");
		return;
	}
	fprintf(fp, "%s:%s\n", usName, usPasswdMD5);
	fprintf(fp, "%s:%s\n", suName, suPasswdMD5);

	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&entry)) {
			printf("ERROR: Get account configuration information from MIB database failed.\n");
			fclose(fp);
			return;
		}
		calPasswdMD5(entry.userPassword, usPasswdMD5);
#ifdef SUPPORT_AUTH_DIGEST
		fprintf(fpd, "%s\n%s\n", entry.userName, entry.userPassword);
#endif
		fprintf(fp, "%s:%s\n", entry.userName, usPasswdMD5);
	}
#ifdef SUPPORT_AUTH_DIGEST
	fclose(fpd);
#endif
	fclose(fp);

	#if 0
	//#ifdef _CWMP_MIB_
	{
		char PasswdMD5[256];
		char *tmpStr = PasswdMD5;
		if ((fp = fopen("/var/digest.passwd", "w")) == NULL)
		{
			printf("***** Open file /var/digest.passwd failed !\n");
			return;
		}
		mib_get(CWMP_CONREQ_USERNAME, (void *)tmpStr);
		fprintf(fp, "%s\n", tmpStr);
		mib_get(CWMP_CONREQ_PASSWORD, (void *)tmpStr);
		fprintf(fp, "%s\n", tmpStr);
		fclose(fp);
        }
        #endif
}
#endif
#endif

void write_etcPassword()
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

#if 0 // anonymous ftp
	// added for anonymous ftp
	fprintf(fp, "%s:%s:10:10::/tmp:/dev/null\n", "ftp", "x");
#endif
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

int getDGW(char *buf)
{
	char buff[256];
	int flgs;
	struct in_addr gw, dest, mask, inAddr;
	char ifname[16], dgw[16];
	FILE *fp;

	memset(buf, 0x00, 256);
	if (!(fp=fopen("/proc/net/route", "r"))) {
		printf("Error: cannot open /proc/net/route - continuing...\n");
		return -1;
	}

	fgets(buff, sizeof(buff), fp);
	while (fgets(buff, sizeof(buff), fp) != NULL) {
		if (sscanf(buff, "%s%x%x%x%*d%*d%*d%x", ifname, &dest, &gw, &flgs, &mask) != 5) {
			printf("Unsuported kernel route format\n");
			fclose(fp);
			return -1;
		}
		if(flgs & RTF_UP) {
			if (getInAddr(ifname, IP_ADDR, (void *)&inAddr) == 1) {
				if (inAddr.s_addr == 0x40404040) {
					sprintf(buf, "");
					fclose(fp);
					return 0;
				}
			}
			// default gateway
			if (dest.s_addr == 0 && mask.s_addr == 0) {
				if (gw.s_addr != 0) {
					strncpy(dgw,  inet_ntoa(gw), 16);
					sprintf(buf, "%s", dgw);
					fclose(fp);
					return 0;
				}
				else {
					sprintf(buf, "%s", ifname);
					fclose(fp);
					return 0;
				}
			}
		}
	}
	fclose(fp);
	return 0;
}

int wanConnList(int bShowIndex)
{
#ifndef CONFIG_DEV_xDSL
	printf("To be implemented\n");
	return -1;
#else
    unsigned int uiEntryNum, i;
    MIB_CE_ATM_VC_T stEntry;
    char szVcc[12], *szAal5Encap, szIpAddr[20], szGateway[20], *pszStatus, *pszTemp;
    const char *pszMode;
    int iAdslflag;
    Modem_LinkSpeed stLs;

    if (bShowIndex)
    {
        printf("%-4s%-10s%-8s%-7s%-9s%-16s%-16s%-7s\n","Idx","Interface","VPI/VCI","Encap","Protocol","IP Address","Gateway","Status");
    }
    else
    {
        printf("%-10s%-8s%-7s%-9s%-16s%-16s%-7s\n","Interface","VPI/VCI","Encap","Protocol","IP Address","Gateway","Status");
    }

    MSG_LINE;

    uiEntryNum = mib_chain_total(MIB_ATM_VC_TBL);
    if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&stLs, RLCM_GET_LINK_SPEED_SIZE) || stLs.upstreamRate == 0)
    {
        iAdslflag = 0;
    }
    else
    {
        iAdslflag = 1;
    }

    for (i = 0; i < uiEntryNum; i++)
    {
        char szIfname[6];
        struct in_addr stInAddr;
        int iFlags;

        if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&stEntry))
        {
            return -1;
        }

        if (stEntry.enable == 0)
            continue;

        /* Initialize */
        szIpAddr[0]  = '\0';
        szGateway[0] = '\0';

        pszMode = 0;
#ifdef CONFIG_ATM_CLIP
        if (stEntry.cmode <= CHANNEL_MODE_RT1577)
#else
        if (stEntry.cmode <= CHANNEL_MODE_RT1483)
#endif
        {
            pszMode = g_pszMode[stEntry.cmode];
        }

        snprintf(szVcc, 12, "%u/%u", stEntry.vpi, stEntry.vci);

        if (stEntry.encap == 0)
        {
            szAal5Encap = "VCMUX";
        }
        else
        {
            szAal5Encap = "LLC";
        }

        if ((CHANNEL_MODE_PPPOE == stEntry.cmode) || (CHANNEL_MODE_PPPOA == stEntry.cmode))
        {
            snprintf(szIfname, 6, "ppp%u", PPP_INDEX(stEntry.ifIndex));

#ifdef EMBED
            if (getInAddr(szIfname, IP_ADDR, (void *)&stInAddr) == 1)
            {
                pszTemp = inet_ntoa(stInAddr);
                if (getInFlags(szIfname, &iFlags) == 1)
                {
                    // Jenny,  buglist B063, check if IP Passthrough or IP unnumbered
                    if ((strcmp(pszTemp, "10.0.0.1") == 0) && iFlags & IFF_POINTOPOINT)
                    {
                        strcpy(szIpAddr, STR_UNNUMBERED);
                    }
			else if (strcmp(pszTemp, "64.64.64.64") == 0)
				strcpy(szIpAddr, "");
                    else
                    {
                        strcpy(szIpAddr, pszTemp);
                    }
                }
            }

            if (getInAddr(szIfname, DST_IP_ADDR, (void *)&stInAddr) == 1)
            {
                pszTemp = inet_ntoa(stInAddr);
                if (strcmp(pszTemp, "10.0.0.2") == 0)
                // Jenny,  buglist B063, check if IP Passthrough or IP unnumbered
                {
                    strcpy(szGateway, STR_UNNUMBERED);
                }
		else if (strcmp(pszTemp, "64.64.64.64") == 0)
			strcpy(szGateway, "");
                else
                {
                    strcpy(szGateway, pszTemp);
                }
            }
#endif

             // set status flag
            if (getInFlags(szIfname, &iFlags) == 1)
            {
                if (iFlags & IFF_UP)
                {
			if (getInAddr(szIfname, IP_ADDR, (void *)&stInAddr) == 1) {
				if (stInAddr.s_addr == 0x40404040)
					pszStatus = (char *)IF_DOWN;
				else
					pszStatus = (char *)IF_UP;
			}
                }
                else
                {
                    //cathy, for  bug B011
                    pszStatus = (char *)IF_DOWN;
                    szIpAddr[0]  = '\0';
                    szGateway[0] = '\0';
                }
            }
            else
            {
                pszStatus = (char *)IF_NA;
            }
        }
        else
        {
            snprintf(szIfname, 5, "vc%u", VC_INDEX(stEntry.ifIndex));

#ifdef EMBED
            // Jenny, sync cli IP status with web page
            if (getInAddr(szIfname, IP_ADDR, (void *)&stInAddr) == 1)
            {
                pszTemp = inet_ntoa(stInAddr);
                if (getInFlags(szIfname, &iFlags) == 1)
                {
                    if ((strcmp(pszTemp, "10.0.0.1") == 0) && iFlags & IFF_POINTOPOINT)
                    // IP Passthrough or IP unnumbered
                    {
                        strcpy(szIpAddr, STR_UNNUMBERED);
                    }
                    else
                    {
                        strcpy(szIpAddr, pszTemp);
                    }
                }
            }

            if (getInAddr(szIfname, DST_IP_ADDR, (void *)&stInAddr) == 1)
            {
                pszTemp = inet_ntoa(stInAddr);
                if (strcmp(pszTemp, "10.0.0.2") == 0)
                {
                    strcpy(szGateway, STR_UNNUMBERED);
                }
                else
                {
                    strcpy(szGateway, pszTemp);
                }

                if (getInFlags(szIfname, &iFlags) == 1)
                {
                    if (iFlags & IFF_BROADCAST)
                    {
                        unsigned char aucValue[32];
                        FILE *pstFile;

                        snprintf(aucValue, 32, "%s.%s", (char *)MER_GWINFO, szIfname);
                        if (pstFile = fopen(aucValue, "r"))
                        {
                            fscanf(pstFile, "%s\n", szGateway);
                            fclose(pstFile);
                        }
                        else
                        {
                            szGateway[0] = '\0';
                        }
                    }
                }
            }
#endif

            // set status flag
            if (getInFlags(szIfname, &iFlags) == 1)
            {
                if ((iFlags & IFF_UP) && iAdslflag)
                {
                    //cathy, for  bug B011
                    pszStatus = (char *)IF_UP;
                }
                else
                {
                    pszStatus = (char *)IF_DOWN;
                    szIpAddr[0]  = '\0';
                    szGateway[0] = '\0';
                }
            }
            else
            {
                pszStatus = (char *)IF_NA;
            }
        }

        if (bShowIndex)
        {
            printf("%-4d%-10s%-8s%-7s%-9s%-16s%-16s%-7s\n", i, szIfname, szVcc, szAal5Encap,
                    pszMode, szIpAddr, szGateway, pszStatus);
        }
        else
        {
            printf("%-10s%-8s%-7s%-9s%-16s%-16s%-7s\n", szIfname, szVcc, szAal5Encap,
                    pszMode, szIpAddr, szGateway, pszStatus);
        }
    }
    return uiEntryNum;
#endif // defined CONFIG_DEV_xDSL
}

void showStatus()
{
	printf("\n");
	MSG_LINE;
	printf("                            ADSL Router Status                           \n");
	MSG_LINE;
	printf("This page shows the current status and some basic settings of the device.\n");
	MSG_LINE;
	printf("System\n");
	printf("Alias Name\t\t: %s\n", getMibInfo(MIB_SNMP_SYS_NAME));
	getSYS2Str(SYS_UPTIME, strbuf);
	printf("Uptime\t\t\t: %s\n", strbuf);
	getSYS2Str(SYS_DATE, strbuf);
	printf("Date/Time\t\t: %s\n", strbuf);
	#ifdef CONFIG_DEV_xDSL
	getSYS2Str(SYS_FWVERSION, strbuf);
	printf("Firmware Version\t: %s\n", strbuf);
	getAdslInfo(ADSL_GET_VERSION, strbuf, 256);
	printf("DSP Version\t\t: %s\n", strbuf);
	#endif // CONFIG_DEV_xDSL
#ifdef WLAN_SUPPORT
	getWlVersion(WLANIF[0], strbuf);
	printf("Wireless Version\t: %s\n", strbuf);
#endif
	getNameServers(strbuf);
	printf("Name Servers\t\t: %s\n", strbuf);
	getDGW(strbuf);
	printf("Default Gateway\t\t: %s\n", strbuf);
	#ifdef CONFIG_DEV_xDSL
	printf("\nDSL\n");
	printf("Operational Status\t: ");
	getAdslInfo(ADSL_GET_MODE, strbuf, 256);
	if (strbuf[0])
		printf("%s, ", strbuf);
	getAdslInfo(ADSL_GET_STATE, strbuf, 256);
	printf("%s\n", strbuf);
	getAdslInfo(ADSL_GET_RATE_US, strbuf, 256);
	printf("Upstream Speed\t\t: %s kbps\n", strbuf);
	getAdslInfo(ADSL_GET_RATE_DS, strbuf, 256);
	printf("Downstream Speed\t: %s kbps", strbuf);
	getAdslInfo(ADSL_GET_LATENCY, strbuf, 256);
	printf("\nChannel mode\t\t: %s\n", strbuf);
	#endif //CONFIG_DEV_xDSL
	printf("\nLAN Configuration\n");
	printf("IP Address\t\t: %s\n", getMibInfo(MIB_ADSL_LAN_IP));
	printf("Subnet Mask\t\t: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET));
#ifdef CONFIG_USER_DHCP_SERVER
	getSYS2Str(SYS_LAN_DHCP, strbuf);
	printf("DHCP Server\t\t: %s\n", strbuf);
#endif
	printf("MAC Address\t\t: %s\n", getMibInfo(MIB_ELAN_MAC_ADDR));
	printf("\nWAN Configuration\n");
	wanConnList(0);
}

#ifdef CONFIG_SECONDARY_IP
int setLanIpAddr(struct in_addr *pIp1,struct in_addr *pMask1,int enIp2,struct in_addr *pIp2,struct in_addr *pMask2,int dhcp_pool)
#else
int setLanIpAddr(struct in_addr *pIp1,struct in_addr *pMask1)
#endif
{
#ifdef CONFIG_SECONDARY_IP
	unsigned char vChar;
#endif

	if ( !mib_set( MIB_ADSL_LAN_IP, (void *)pIp1)) {
		printf("%s\n", strSetIPerror);
		return -1;
	}

	if ( !mib_set(MIB_ADSL_LAN_SUBNET, (void *)pMask1)) {
		printf("%s\n", strSetMaskerror);
		return -1;
	}

#ifdef CONFIG_SECONDARY_IP
	if (enIp2 == 1)
		vChar = 1;
	else
		vChar = 0;
	if (!mib_set(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar)){
		printf("Set secondary IP error!");
		return -1;
	}

	if (enIp2 == 1) {
		if (!mib_set( MIB_ADSL_LAN_IP2, (void *)pIp2)){
			printf("%s\n", strSetIPerror);
			return -1;
		}

		if (!mib_set(MIB_ADSL_LAN_SUBNET2, (void *)pMask2)){
			printf("%s\n", strSetMaskerror);
			return -1;
		}

		if (dhcp_pool == 1)
			vChar = 1;
		else
			vChar = 0;
		if (!mib_set(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)&vChar)){
			printf("Set DHCP pool error!");
			return -1;
		}
	}
#endif

#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_lanip();
#endif

	return 0;
}


#if defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL_IGMP_SNOOPING)
int setLanIgmpSnooping(int enable)
{
	char mode;

	// bitmap for virtual lan port function
	// Port Mapping: bit-0
	// QoS : bit-1
	// IGMP snooping: bit-2
	mib_get(MIB_MPMODE, (void *)&mode);

	if (enable) {
		mode |= 0x04;
		// take effect immediately
		__dev_setupIGMPSnoop(1);
	}
	else {
		mode &= 0xfb;
		__dev_setupIGMPSnoop(0);
	#if defined(CONFIG_EXT_SWITCH)
		if (mode==0)
			__dev_setupVirtualPort(0); // disable virtual port
	#endif
	}

	if (!mib_set(MIB_MPMODE, (void *)&mode)){
		printf("Set IGMP Snooping error!");
		return -1;
	}

	return 0;
}
#endif

void showInterfaceStatistics()
{
	int i;
#ifdef EMBED
	int skfd;
	struct atmif_sioc mysio;
	struct SAR_IOCTL_CFG cfg;
	struct ch_stat stat;
#endif

	char *names[16];
	int num_itf;
	struct net_device_stats nds;

	printf("\n");
	MSG_LINE;
	printf("                        Interface Statistics                           \n");
	MSG_LINE;
	printf("\nItf\tRx_pkt\tRx_err\tRx_drop\tTx_pkt\tTx_err\tTx_drop\n");
	MSG_LINE;

	// Ethernet statistics
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
#ifdef CONFIG_ETHWAN
	names[num_itf++] = ALIASNAME_NAS0;
#endif
	// LAN statistics
	for (i = 0; i < num_itf; i++) {
		get_net_device_stats(names[i], &nds);
		printf("%s\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
			names[i], nds.rx_packets, nds.rx_errors,
			nds.rx_dropped, nds.tx_packets,
			nds.tx_errors, nds.tx_dropped);
	}

#ifdef EMBED
	// pvc statistics
	if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
		perror("socket open error");
		return;
	}

	mysio.number = 0;

	for (i=0; i < MAX_VC_NUM; i++)
	{
		cfg.ch_no = i;
		mysio.arg = (void *)&cfg;
		if(ioctl(skfd, ATM_SAR_GETSTAT, &mysio)<0)
		{
			(void)close(skfd);
			return;
		}

		if (cfg.created == 0)
			continue;

		printf("%u/%u\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n",
      			cfg.vpi, cfg.vci, cfg.stat.rx_pkt_cnt, cfg.stat.rx_pkt_fail,
			cfg.stat.rx_crc_error, cfg.stat.tx_desc_ok_cnt,
			cfg.stat.tx_pkt_fail_cnt, cfg.stat.send_desc_lack);
	}
	(void)close(skfd);
#endif

	MSG_LINE;
}

void showADSLSettings()
{
	unsigned char vChar;
	unsigned short mode;

	mib_get( MIB_ADSL_MODE, (void *)&mode);
	printf("ADSL modulation:\n");	// ADSL modulation
	if (mode & ADSL_MODE_GLITE)
		printf("\tG.Lite:\t  Enable\n");
	else
		printf("\tG.Lite:\t  Disable\n");
	if (mode & ADSL_MODE_GDMT)
		printf("\tG.Dmt:\t  Enable\n");
	else
		printf("\tG.Dmt:\t  Disable\n");
	if (mode & ADSL_MODE_T1413)
		printf("\tT1.413:\t  Enable\n");
	else
		printf("\tT1.413:\t  Disable\n");
	if (mode & ADSL_MODE_ADSL2)
		printf("\tADSL2:\t  Enable\n");
	else
		printf("\tADSL2:\t  Disable\n");
	if (mode & ADSL_MODE_ADSL2P)
		printf("\tADSL2+:\t  Enable\n");
	else
		printf("\tADSL2+:\t  Disable\n");
	printf("AnnexL Option:\n");	// AnnexL Option
	if (mode & ADSL_MODE_ANXL)
		printf("\tAnnexL:\t  Enable\n");
	else
		printf("\tAnnexL:\t  Disable\n");
	printf("AnnexM Option:\n");	// AnnexM Option
	if (mode & ADSL_MODE_ANXM)
		printf("\tAnnexM:\t  Enable\n");
	else
		printf("\tAnnexM:\t  Disable\n");
#ifdef ENABLE_ADSL_MODE_GINP
	printf("G.INP Option:\n");	// G.INP Option
	if (mode & ADSL_MODE_GINP)
		printf("\tG.INP:\t  Enable\n");
	else
		printf("\tG.INP:\t  Disable\n");
#endif

	mib_get( MIB_ADSL_OLR, (void *)&vChar);
	printf("ADSL Capability:\n");	// OLR type
	if (vChar & 1)
		printf("\tBitswap:  Enable\n");
	else
		printf("\tBitswap:  Disable\n");
	if (vChar & 2)
		printf("\tSRA:\t  Enable\n");
	else
		printf("\tSRA:\t  Disable\n");
}

void showAdslStatistics()
{
	printf("\n");
	#ifdef CONFIG_DEV_xDSL
	MSG_LINE;
	printf("                        ADSL Statistics                          \n");
	MSG_LINE;

	getAdslInfo(ADSL_GET_MODE, strbuf, 256);
	printf("Mode\t\t\t: %s\n", strbuf);
	getAdslInfo(ADSL_GET_LATENCY, strbuf, 256);
	printf("Latency\t\t\t: %s\n", strbuf);
	getAdslInfo(ADSL_GET_TRELLIS, strbuf, 256);
	printf("Trellis Coding\t\t: %s\n", strbuf);
	getAdslInfo(ADSL_GET_STATE, strbuf, 256);
	printf("Status\t\t\t: %s\n", strbuf);
	getAdslInfo(ADSL_GET_POWER_LEVEL, strbuf, 256);
	printf("Power Level\t\t: %s\n", strbuf);
	printf("\n");

	printf("\t\t\t\t\t\tDownstream");
	printf("\tUptream\n");
	getAdslInfo(ADSL_GET_SNR_DS, strbuf, 256);
	printf("SNR Margin (dB)\t\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_SNR_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_LPATT_DS, strbuf, 256);
	printf("Attenuation (dB)\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_LPATT_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_POWER_DS, strbuf, 256);
	printf("Power (dBm)\t\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_POWER_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_ATTRATE_DS, strbuf, 256);
	printf("Attainable Rate (Kbps)\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_ATTRATE_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_RATE_DS, strbuf, 256);
	printf("Rate (Kbps)\t\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_RATE_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_K_DS, strbuf, 256);
	printf("K (number of bytes in DMT frame)\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_K_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_R_DS, strbuf, 256);
	printf("R (number of check bytes in RS code word)\t: %s", strbuf);
	getAdslInfo(ADSL_GET_R_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_S_DS, strbuf, 256);
	printf("S (RS code word size in DMT frame)\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_S_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_D_DS, strbuf, 256);
	printf("D (interleaver depth)\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_D_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_DELAY_DS, strbuf, 256);
	printf("Delay (msec)\t\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_DELAY_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_FEC_DS, strbuf, 256);
	printf("FEC\t\t\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_FEC_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_CRC_DS, strbuf, 256);
	printf("CRC\t\t\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_CRC_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_ES_DS, strbuf, 256);
	printf("Total ES\t\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_ES_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_SES_DS, strbuf, 256);
	printf("Total SES\t\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_SES_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	getAdslInfo(ADSL_GET_UAS_DS, strbuf, 256);
	printf("Total UAS\t\t\t\t\t: %s", strbuf);
	getAdslInfo(ADSL_GET_UAS_US, strbuf, 256);
	printf("\t\t%s\n", strbuf);

	printf("\n");
	#endif  // CONFIG_DEV_xDSL
}

void showLanInterface()
{
	unsigned char vChar;

	printf("\nLAN Interface:\n");
	printf("	Interface Name\t: br0\n");
	printf("	IP Address\t: %s\n",getMibInfo(MIB_ADSL_LAN_IP));
	printf("	Subnet Mask\t: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET));

#ifdef CONFIG_SECONDARY_IP
	getSYS2Str(SYS_LAN_IP2, strbuf);
	printf("	Secondary IP\t: %s\n", strbuf);
	mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&vChar);
	if (vChar != 0) {
		printf("	IP Address\t: %s\n",getMibInfo(MIB_ADSL_LAN_IP2));
		printf("	Subnet Mask\t: %s\n", getMibInfo(MIB_ADSL_LAN_SUBNET2));
		getSYS2Str(SYS_LAN_DHCP_POOLUSE, strbuf);
		printf("	DHCP pool\t: %s\n", strbuf);
	}
#endif

#if defined(CONFIG_EXT_SWITCH) || defined(CONFIG_RTL_IGMP_SNOOPING)
	getSYS2Str(SYS_IGMP_SNOOPING, strbuf);
	printf("	IGMP Snooping\t: %s\n\n", strbuf);
#endif

}

/* Display wan configuration */
/* int bShowIndex: indicates whether show the index */
int _wanConfList(int bShowIndex)
{
	#ifndef CONFIG_DEV_xDSL
	printf("To be implemented\n");
	return 0;
	#else
    unsigned int entryNum, i;
    MIB_CE_ATM_VC_T Entry;
    char vcc[12], *aal5Encap;
    const char *mode;
    char *pszNapt, ipAddr[20], szMask[20], remoteIp[20], szUserName[MAX_PPP_NAME_LEN+1];
#ifdef DEFAULT_GATEWAY_V1
    const char *strDroute;
#endif
    char    *strStatus, *temp;
    int in_turn = 0, adslflag;

    if (bShowIndex)
    {
        printf("%-4s", "Idx");
    }

#ifdef DEFAULT_GATEWAY_V1
    printf("%-6s%-7s%-7s%-8s%-16s%-16s%-16s%-5s%-30s%-8s%-7s\n","Inf","PVC","Encap","Mode","IPAddr","RemoteIP","Mask","NAPT","UserName","Status","DRoute");
#else
    printf("%-6s%-7s%-7s%-8s%-16s%-16s%-16s%-5s%-30s%-8s\n","Inf","PVC","Encap","Mode","IPAddr","RemoteIP","Mask","NAPT","UserName","Status");
#endif

    MSG_LINE;

    entryNum = mib_chain_total(MIB_ATM_VC_TBL);
    //cathy, for  bug B011
    Modem_LinkSpeed vLs;
    // check for xDSL link
    if (!adsl_drv_get(RLCM_GET_LINK_SPEED, (void *)&vLs, RLCM_GET_LINK_SPEED_SIZE) || vLs.upstreamRate == 0)
        adslflag = 0;
    else
        adslflag = 1;

    for (i=0; i<entryNum; i++)
    {
        char ifname[6];
        struct in_addr inAddr;
        int flags;

        if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
        {
            return -1;
        }

        // set status flag
        if (Entry.enable == 0)
        {
            strStatus = (char *)IF_DISABLE;
        }
        else
        {
            strStatus = (char *)IF_ENABLE;
        }
#if 0
		if (Entry.enable == 0)
			continue;
#endif

        /* Initialize */
        ipAddr[0]     = '\0';
        szMask[0]     = '\0';
        remoteIp[0]   = '\0';
        szUserName[0] = '\0';
        pszNapt       = "";
#ifdef DEFAULT_GATEWAY_V1
        strDroute     = "";
#endif

        mode = 0;
#ifdef CONFIG_ATM_CLIP
        if (Entry.cmode <= CHANNEL_MODE_RT1577)
#else
        if (Entry.cmode <= CHANNEL_MODE_RT1483)
#endif
        {
            mode = g_pszMode[Entry.cmode];
        }

        snprintf(vcc, 12, "%u/%u", Entry.vpi, Entry.vci);

        aal5Encap = 0;
        if (Entry.encap == 0)
        {
            aal5Encap = "VCMUX";
        }
        else
        {
            aal5Encap = "LLC";
        }

        if (Entry.napt == 0)
        {
            pszNapt = (char*)IF_OFF;
        }
        else
        {
            pszNapt = (char*)IF_ON;
        }

        if (Entry.cmode == CHANNEL_MODE_PPPOE || Entry.cmode == CHANNEL_MODE_PPPOA)
        {
            strncpy(szUserName, Entry.pppUsername, MAX_PPP_NAME_LEN);
            szUserName[MAX_NAME_LEN] = '\0';

            snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));

#ifdef CONFIG_SPPPD_STATICIP
            if (Entry.cmode == CHANNEL_MODE_PPPOE && Entry.pppIp)
            {
                temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
                strcpy(ipAddr, temp);

                temp = inet_ntoa(*((struct in_addr *)Entry.netMask));
                strcpy(szMask, temp);
            }
#endif

        }
        else
        {
            snprintf(ifname, 5, "vc%u", VC_INDEX(Entry.ifIndex));

            if ((Entry.ipDhcp == (char)DHCP_DISABLED)
                && (!Entry.ipunnumbered)
                && (Entry.cmode != CHANNEL_MODE_BRIDGE))
            {
                // static IP address
                temp = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
                strcpy(ipAddr, temp);

                temp = inet_ntoa(*((struct in_addr *)Entry.remoteIpAddr));
                strcpy(remoteIp, temp);

                temp = inet_ntoa(*((struct in_addr *)Entry.netMask)); // Jenny, subnet mask
                strcpy(szMask, temp);
            }
        }

#ifdef DEFAULT_GATEWAY_V1
        if (Entry.dgw == 0) // Jenny, default route
            strDroute = IF_OFF;
        else
            strDroute = IF_ON;
#endif

        in_turn ^= 0x01;

        if (bShowIndex)
        {
            printf("%-4d", i);
        }

#ifdef DEFAULT_GATEWAY_V1
        printf("%-6s%-7s%-7s%-8s%-16s%-16s%-16s%-5s%-30s%-8s%-7s\n", ifname, vcc, aal5Encap,
                mode, ipAddr, remoteIp, szMask, pszNapt, szUserName, strStatus, strDroute);
#else
        printf("%-6s%-7s%-7s%-8s%-16s%-16s%-16s%-5s%-30s%-8s\n", ifname, vcc, aal5Encap,
                mode, ipAddr, remoteIp, szMask, pszNapt, szUserName, strStatus);
#endif
    }

    return entryNum;
	#endif //CONFIG_DEV_xDSL
}

void showWanInterface()
{
	printf("\nWAN Interface:\n");
	_wanConfList(1);
}

void showARPTable()
{
	FILE *fp;
	char arg1[20], arg2[20];
	int arg3;

	printf("\n");
	printf("-----------------------------------\n");
	printf("             ARP Table             \n");
	printf("-----------------------------------\n");
	printf("IP Address          MAC Address\n");
	printf("-----------------------------------\n");

	fp = fopen("/proc/net/arp", "r");
	if (fp == NULL){
		printf("read arp file fail!\n");
		return;
	}
	fgets(strbuf, 256, fp);
	while (fgets(strbuf, 256, fp)) {
		sscanf(strbuf, "%s%*x 0x%x%s", arg1, &arg3, arg2);
		if (!arg3)
			continue;
		printf("%-20s%s\n", arg1, arg2);
	}

	fclose(fp);
}

void showRoutes()
{
	char buff[256];
	int flgs;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16], iface[6];
	FILE *fp;

	printf("\n");
	printf("------------------------------------------------------------\n");
	printf("                       IP Route Table                       \n");
	printf("------------------------------------------------------------\n");
	printf("Destination      Subnet Mask      NextHop          Iface\n");
	printf("------------------------------------------------------------\n");

	if (!(fp=fopen("/proc/net/route", "r"))) {
		fclose(fp);
		printf("Error: cannot open /proc/net/route !!\n");
		return;
	}


	fgets(buff, sizeof(buff), fp);

	while( fgets(buff, sizeof(buff), fp) != NULL ) {
		if(sscanf(buff, "%s%lx%lx%X%*d%*d%*d%lx",
		   iface, &d, &g, &flgs, &m)!=5) {
			printf("Error: Unsuported kernel route format !!\n");
			return;
		}

		if(flgs & RTF_UP) {
			dest.s_addr = d;
			gw.s_addr   = g;
			mask.s_addr = m;
			// inet_ntoa is not reentrant, we have to
			// copy the static memory before reuse it
			strcpy(sdest, inet_ntoa(dest));
			strcpy(sgw,  (gw.s_addr==0   ? "*" : inet_ntoa(gw)));

			printf("%-17s%-17s%-17s%s\n",sdest, inet_ntoa(mask), sgw, iface);
		}
	}

	fclose(fp);
	return;
}

static void __dump_fdb_entry(struct __fdb_entry *f)
{
	unsigned long long tvusec;
	int sec,usec;

	// jiffies to tv
	tvusec = (1000000ULL*f->ageing_timer_value)/HZ;
	sec = tvusec/1000000;
	usec = tvusec - 1000000 * sec;

	printf("%-9i%.2x:%.2x:%.2x:%.2x:%.2x:%.2x  %-11s%4i.%.2i\n",
		f->port_no, f->mac_addr[0], f->mac_addr[1], f->mac_addr[2],
		f->mac_addr[3], f->mac_addr[4], f->mac_addr[5],
		f->is_local?"yes":"no", sec, usec/10000);
}

void showMACs()
{
	int br_socket_fd;
	struct bridge *br;
	struct __fdb_entry fdb[256];
	int offset;
	unsigned long args[4];
	struct ifreq ifr;

   	printf("\n");
   	printf("------------------------------------------------------\n");
   	printf("          Bridge Forwarding Database Table            \n");
   	printf("------------------------------------------------------\n");
   	printf("Port No    MAC Address      Is Local?   Ageing Timer\n");
   	printf("------------------------------------------------------\n");

	offset = 0;
	args[0] = BRCTL_GET_FDB_ENTRIES;
	args[1] = (unsigned long)fdb;
	args[2] = 256;
	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket not avaiable !!\n");
		return;
	}
//	memcpy(ifr.ifr_name, br->ifname, IFNAMSIZ);
	memcpy(ifr.ifr_name, BRIF, IFNAMSIZ);
	((unsigned long *)(&ifr.ifr_data))[0] = (unsigned long)args;
	while (1) {
		int i;
		int num;

		args[3] = offset;
		num = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

		if (num <= 0)
		{
			if (num < 0)
				printf("br0 interface not exists !!\n");
			break;
		}

		for (i=0;i<num;i++)
			__dump_fdb_entry(fdb+i);

		offset += num;
	}
	close(br_socket_fd);

}

#ifdef ROUTING
void showStaticRoutes()
{
	unsigned int entryNum, i;
	MIB_CE_IP_ROUTE_T Entry;
	unsigned long int d,g,m;
	struct in_addr dest;
	struct in_addr gw;
	struct in_addr mask;
	char sdest[16], sgw[16];
	char interface_name[30];

	entryNum = mib_chain_total(MIB_IP_ROUTE_TBL);

	printf("\n-----------------------------------------------------------------------------------\n");
	printf("                              Static Route Table                                  \n");
	printf("---------------------------------------------------------------------------------------\n");
	//printf("Idx  Subnet Mask      NextHop          Iface\n");
	printf("Idx  State    Destination      Subnet Mask      NextHop          Metric   Interface\n");
	printf("-----------------------------------------------------------------------------------\n");

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_IP_ROUTE_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}

		dest.s_addr = *(unsigned long *)Entry.destID;
		gw.s_addr   = *(unsigned long *)Entry.nextHop;
		mask.s_addr = *(unsigned long *)Entry.netMask;
		// inet_ntoa is not reentrant, we have to
		// copy the static memory before reuse it
		strcpy(sdest, inet_ntoa(dest));
		strcpy(sgw, inet_ntoa(gw));
		if (Entry.ifIndex == 0xff)
			strcpy( interface_name, "---" );
		else if ((Entry.ifIndex & 0xf0) == 0xf0)
			sprintf(interface_name, "vc%u", VC_INDEX(Entry.ifIndex));
		else
			sprintf(interface_name, "ppp%u", PPP_INDEX(Entry.ifIndex));
		//printf("%-5d%-17s%-17s%s\n",i+1, sdest, inet_ntoa(mask), sgw);
		printf("%-5d%-9s%-17s%-17s%-17s%-9d%s\n", i+1, Entry.Enable ? "Enable" : "Disable", sdest, inet_ntoa(mask), sgw, Entry.FWMetric, interface_name);
	}

	return;
}
#endif

#ifdef CONFIG_USER_DHCP_SERVER
int setDHCPMode(unsigned char mode)
{
	if ( mib_set(MIB_DHCP_MODE, (void *)&mode) == 0)
	{
		printf("Set MIB_DHCP_MODE error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_dhcp();
#endif

	return 0;
}

int setDhcpRelayServer(struct in_addr *serverIp)
{
	if ( !mib_set(MIB_ADSL_WAN_DHCPS, (void *)serverIp)) {
	  	printf("Set MIB_ADSL_WAN_DHCPS error!");
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_dhcp();
#endif

	return 0;
}

int setDhcpServer(DHCP_SERVER_PARA_P pDhcpServerPara)
{
#ifdef DHCPS_POOL_COMPLETE_IP
	if(pDhcpServerPara->pPoolstart && pDhcpServerPara->pPoolend){
		if(pDhcpServerPara->pPoolstart->s_addr > pDhcpServerPara->pPoolend->s_addr){
			printf("Invalid DHCP client range!\n");
			return -1;
		}
		if(!mib_set( MIB_DHCP_POOL_START, (void *)pDhcpServerPara->pPoolstart) || !mib_set( MIB_DHCP_POOL_END, (void *)pDhcpServerPara->pPoolend)){
			printf("set dhcp pool error!\n");
			return -1;
		}
	}
#else
	if(pDhcpServerPara->pPoolstart && pDhcpServerPara->pPoolend){
		unsigned char start = *(pDhcpServerPara->pPoolstart);
		unsigned char end = *(pDhcpServerPara->pPoolend);
		if(start > 254 || start < 1){
			printf("pool start host error! it must be less than 255 and larger than 1!\n");
			return -1;
		}
		if(end > 254 || end < 1){
			printf("pool end host error! it must be less than 255 and larger than 1!\n");
			return -1;
		}

		if(start >  end){
			printf("Invalid DHCP client range!\n");
			return -1;
		}

		if(!mib_set( MIB_ADSL_LAN_CLIENT_START, (void *)&start) ||!mib_set( MIB_ADSL_LAN_CLIENT_END, (void *)&end)){
			printf("set dhcp pool error!\n");
			return -1;
		}
	}
#endif

	if(pDhcpServerPara->pGateway){
		if ( !mib_set( MIB_ADSL_LAN_DHCP_GATEWAY, (void *)pDhcpServerPara->pGateway)) {
			printf("Set DHCP Server Gateway Address error!\n");
			return -1;
		}
	}

	if(pDhcpServerPara->pLeaseTime){
		if ( !mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)pDhcpServerPara->pLeaseTime)) {
			printf("Set DHCP lease time error!\n");
			return -1;
		}
	}

	if(pDhcpServerPara->pDomain){
		if ( !mib_set(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)pDhcpServerPara->pDomain)) {
			printf("Set DHCP Domain Name error!\n");
			return -1;
		}
	}

#ifdef DHCPS_DNS_OPTIONS
	if(pDhcpServerPara->pDnsMode){
		unsigned char mode = *(pDhcpServerPara->pDnsMode);
		if(!mib_set(MIB_DHCP_DNS_OPTION, (void *)&mode)){
			printf("set dhcp server dns option error!\n");
			return -1;
		}
		if(mode){
			if(pDhcpServerPara->pDns1){
				if(!mib_set(MIB_DHCPS_DNS1, (void *)pDhcpServerPara->pDns1)){
					printf("set dhcp server dns1 error!\n");
					return -1;
				}
			}
			if(pDhcpServerPara->pDns2){
				if(!mib_set(MIB_DHCPS_DNS2, (void *)pDhcpServerPara->pDns2)){
					printf("set dhcp server dns2 error!\n");
					return -1;
				}
			}
			if(pDhcpServerPara->pDns3){
				if(!mib_set(MIB_DHCPS_DNS3, (void *)pDhcpServerPara->pDns3)){
					printf("set dhcp server dns3 error!\n");
					return -1;
				}
			}
		}
	}
#endif

#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_dhcp();
#endif

	return 0;
}
#endif

#if 0
#ifdef DHCPS_POOL_COMPLETE_IP
int setDhcpsPoolRange(struct in_addr * pStartIp, struct in_addr *pEndIp)
{
	if(pStartIp && pEndIp){
		if(pStartIp->s_addr > pEndIp->s_addr){
			printf("Invalid DHCP client range!\n");
			return -1;
		}

		if(!mib_set( MIB_DHCP_POOL_START, (void *)pStartIp) || !mib_set( MIB_DHCP_POOL_END, (void *)pEndIp)){
			printf("set dhcp pool error!\n");
			return -1;
		}
	}

	return 0;
}
#else
int setDhcpsPoolRange(unsigned char start, unsigned char end)
{
	if(start > 254 || start < 1){
		printf("pool start host error! it must be less than 255 and larger than 1!\n");
		return -1;
	}
	if(end > 254 || end < 1){
		printf("pool end host error! it must be less than 255 and larger than 1!\n");
		return -1;
	}

	if(start > end){
		printf("Invalid DHCP client range!\n");
		return -1;
	}

	if(!mib_set( MIB_ADSL_LAN_CLIENT_START, (void *)&start) ||!mib_set( MIB_ADSL_LAN_CLIENT_END, (void *)&end)){
		printf("set dhcp pool error!\n");
		return -1;
	}

	return 0;
}
#endif

#ifdef DHCPS_DNS_OPTIONS
int setDhcpsDnsOption(unsigned char option)
{
	if(!mib_set(MIB_DHCP_DNS_OPTION, (void *)&option)){
		printf("set dhcp server dns option error!\n");
		return -1;
	}
	return 0;
}

int setDhcpsDns(int mib_id, struct in_addr * ip)
{
	if(!mib_set(mib_id, (void *)ip)){
		printf("set dhcp server dns error!\n");
		return -1;
	}
	return 0;
}
#endif

int setDhcpsGateway(struct in_addr * ip)
{
	if ( !mib_set( MIB_ADSL_LAN_DHCP_GATEWAY, (void *)ip)) {
		printf("Set DHCP Server Gateway Address error!\n");
		return -1;
	}
	return 0;
}

int setDhcpsLeaseTime(unsigned int time)
{
	if ( !mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)&time)) {
		printf("Set DHCP lease time error!\n");
		return -1;
	}
	return 0;
}

int setDhcpsDomainName(char * pName)
{
	if ( !mib_set(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)pName)) {
		printf("Set DHCP Domain Name error!\n");
		return -1;
	}
	return 0;
}
#endif

#ifdef IP_ACL
void showAclCfg()
{
	unsigned char vChar;
	unsigned int entryNum, i;
	MIB_CE_ACL_IP_T Entry;
	struct in_addr dest;
	char sdest[16];

	if ( !mib_get( MIB_ACL_CAPABILITY, (void *)&vChar) )
		return;
	if (0 == vChar)
		printf("1. ACL Capability: Disabled\n");
	else
		printf("1. ACL Capability: Enabled\n");

	printf("2. ACL Table\n");
	printf("   Idx  State   Interface  IP Address\n");
	printf("---------------------------------------------\n");

	entryNum = mib_chain_total(MIB_ACL_IP_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ACL_IP_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
		}

		dest.s_addr = *(unsigned long *)Entry.ipAddr;

		// inet_ntoa is not reentrant, we have to
		// copy the static memory before reuse it
		strcpy(sdest, inet_ntoa(dest));
		snprintf(sdest, 20, "%s/%d", sdest, Entry.maskbit);
		printf("   %-5d%-8s%-11s%s\n", i+1, Entry.Enabled ? "Enable" : "Disable", (Entry.Interface == IF_DOMAIN_LAN)? "LAN" : "WAN", sdest);
		//printf("(%d) IP Address: %s\n", i+1, sdest);
	}
}

int setAclCapability(int enable)
{
	unsigned char vChar;

	vChar = (enable == 0)? 0 : 1;
	if(!mib_set(MIB_ACL_CAPABILITY, (void *)&vChar)){
		printf("set acl capability error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	restart_acl();
#endif

	return 0;
}

/*interface 0: LAN; 1: WAN*/
int addAclEntry(struct in_addr *ip, struct in_addr *mask, int interface, int enabled)
{
	MIB_CE_ACL_IP_T Entry;
	unsigned long mbit,netmask=*((unsigned long*)mask);
	int intVal;

	mbit=0;
	while (1) {
		if (netmask&0x80000000) {
			mbit++;
			netmask <<= 1;
		}
		else
			break;
	}
	((struct in_addr *)&Entry.ipAddr[0])->s_addr = ip->s_addr;
	Entry.maskbit = mbit;
	Entry.Interface = (interface == 0 ? IF_DOMAIN_LAN : IF_DOMAIN_WAN);
	Entry.Enabled = enabled? 1 : 0;

	intVal = mib_chain_add(MIB_ACL_IP_TBL, (unsigned char*)&Entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return -1;
	}else if (intVal == -1){
		printf(strTableFull);
		return -1;
	}

#if defined(APPLY_CHANGE)
	restart_acl();
#endif

	return 0;
}

int delAclEntry(int index)
{
	unsigned int totalEntry;

	totalEntry = mib_chain_total(MIB_ACL_IP_TBL); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return -1;
	}

	if(mib_chain_delete(MIB_ACL_IP_TBL, index-1) != 1){
		printf("Delete chain record error!");
		return -1;
	}

#if defined(APPLY_CHANGE)
	restart_acl();
#endif

	return 0;
}
#endif

 void showBridgeCfg()
{
	unsigned short time;
	unsigned char stp;

	mib_get(MIB_BRCTL_AGEINGTIME, (void *)&time);
	printf("Bridge Ageing Time: %d(seconds)\n",time);
	mib_get(MIB_BRCTL_STP, (void *)&stp);
	printf("802.1d Spanning Tree: %s\n",stp==1?"Enabled" : "Disabled");
}

int setBridgeAgingTime(unsigned short time)
{
#if defined(APPLY_CHANGE)
	char *argv[5];
	char str[10];
	//unsigned char str_cmd[100];
#endif

	if (mib_set(MIB_BRCTL_AGEINGTIME, (void *)&time) == 0) {
		printf("Set bridge ageing time MIB error!");
		return -1;
	}

#if defined(APPLY_CHANGE)
	sprintf(str,"%u",time);
	//printf("str=%s\n", str);
	argv[1]="setageing";
	argv[2]=(char*)BRIF;
	argv[3]=str;
	argv[4]=NULL;
	//sprintf(str_cmd, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
	//printf("str_cmd=%s\n", str_cmd);
	do_cmd(BRCTL, argv, 1);
#endif

	return 0;
}

int setBridgeStp(unsigned char enabled)
{
#if defined(APPLY_CHANGE)
	char *argv[5];
	char str[10];
	//unsigned char str_cmd[100];
#endif

	if ( !mib_set(MIB_BRCTL_STP, (void *)&enabled)) {
		printf("Set STP mib error!");
		return -1;
	}

#if defined(APPLY_CHANGE)
	if (enabled == 1)	// on
	{	// brctl setfd br0 20
		argv[1]="setfd";
		argv[2]=(char*)BRIF;
		argv[3]="20";
		argv[4]=NULL;
		//sprintf(str_cmd, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
		//printf("str_cmd=%s\n", str_cmd);
		do_cmd(BRCTL, argv, 1);
	}

	argv[1]="stp";
	argv[2]=(char*)BRIF;

	if (enabled == 0)
		argv[3]="off";
	else
		argv[3]="on";

	argv[4]=NULL;
	//sprintf(str_cmd, "%s %s %s %s\n", BRCTL, argv[1], argv[2], argv[3]);
	//printf("str_cmd=%s\n", str_cmd);
	do_cmd(BRCTL, argv, 1);
#endif

	return 0;
}

int setAccountPassword(char * user, char *oldpasswd, char *newpasswd)
{
	char usName[MAX_NAME_LEN];
	char oldMIBpass[MAX_NAME_LEN];
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T Entry;
	int i,entryNum = 0;
#endif
	int selnum = 0;

	mib_get(MIB_SUSER_NAME, (void *)usName);
	if(!strcmp(user,usName)){
		selnum = 1;
		goto check_oldpwd;
	}
	mib_get(MIB_USER_NAME, (void *)usName);
	if(!strcmp(user,usName)){
		selnum = 2;
		goto check_oldpwd;
	}

#ifdef ACCOUNT_CONFIG
	entryNum = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&Entry)){
  			printf ("Get chain record error!\n");
			return -1;
		}
		if(!strcmp(user,Entry.userName)){
			selnum = i+3;
			goto check_oldpwd;
		}
	}
#endif

check_oldpwd:
	if(selnum == 0){
		printf("username is not correct!\n");
		return -1;
	}

	if (selnum == 1) {
   		if (!mib_get(MIB_SUSER_PASSWORD, (void *)oldMIBpass)){
			printf("Get super user password from MIB database error!\n");
			return -1;
   		}
	}else if (selnum == 2) {
   		if (!mib_get(MIB_USER_PASSWORD, (void *)oldMIBpass)){
			printf("Get user password from MIB database failed.\n");
			return -1;
   		}
	}
#ifdef ACCOUNT_CONFIG
	else{
		strcpy(oldMIBpass,Entry.userPassword);
	}
#endif

	/*compare old password*/
   	if (strcmp(oldpasswd, oldMIBpass) != 0) {
   		printf("old password is not correct!\n");
		return -1;
   	}

	/*set new password*/
	if (selnum == 1) {
		if (!mib_set(MIB_SUSER_PASSWORD, (void *)newpasswd)){
			printf("Set Super user password to MIB database failed.\n");
			return -1;
		}
	} else if (selnum == 2) {
		if (!mib_set(MIB_USER_PASSWORD, (void *)newpasswd)){
			printf("Set user password to MIB database failed.\n");
			return -1;
		}
	}
#ifdef ACCOUNT_CONFIG
	else {
		strncpy(Entry.userPassword, newpasswd, MAX_NAME_LEN-1);
		Entry.userPassword[MAX_NAME_LEN-1] = '\0';
		Entry.privilege = (unsigned char)getAccPriv(Entry.userName);
		mib_chain_update(MIB_ACCOUNT_CONFIG_TBL, (void *)&Entry, selnum -3);
	}
#endif

#if defined(APPLY_CHANGE)
	writePasswdFile();
	write_etcPassword();
#endif

	return 0;
}

#ifdef CONFIG_USER_DHCP_SERVER
int addDhcpReservedIp(struct in_addr *ip,char *pMac6)
{
	unsigned char mode;
	MIB_CE_MAC_BASE_DHCP_T entry, Entry;
	int mibtotal, i, intVal;
	unsigned long v1,v2;

	mib_get(MIB_DHCP_MODE, (void *)&mode);
	if(mode != DHCP_LAN_SERVER){
		printf("Reserve ip is only valid in server mode! Please set DHCP to server mode at first!\n");
		return -1;
	}

	memset(&entry, 0, sizeof(entry));
	memcpy((void *)entry.macAddr_Dhcp,(void *)pMac6,MAC_ADDR_LEN);
	memcpy((void *)entry.ipAddr_Dhcp,(void *)ip,IP_ADDR_LEN);

	mibtotal = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	for (i=0; i<mibtotal; i++) {
		mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void*)&Entry);
		v1 = *((unsigned long *)Entry.ipAddr_Dhcp);
		v2 = *((unsigned long *)entry.ipAddr_Dhcp);
		if ( (	Entry.macAddr_Dhcp[0]==entry.macAddr_Dhcp[0] && Entry.macAddr_Dhcp[1]==entry.macAddr_Dhcp[1] && Entry.macAddr_Dhcp[2]==entry.macAddr_Dhcp[2] &&
	     		Entry.macAddr_Dhcp[3]==entry.macAddr_Dhcp[3] && Entry.macAddr_Dhcp[4]==entry.macAddr_Dhcp[4] && Entry.macAddr_Dhcp[5]==entry.macAddr_Dhcp[5] ) || (v1==v2)  ) {
			printf(strStaticipexist);
			return -1;
		}
	}

	intVal = mib_chain_add(MIB_MAC_BASE_DHCP_TBL, (void *)&entry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return -1;
	}else if (intVal == -1) {
		printf(strTableFull);
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_dhcp();
#endif

	return 0;
}

int deleteDhcpReservedIp(struct in_addr *ip)
{
	unsigned char mode;
	MIB_CE_MAC_BASE_DHCP_T Entry;
	int mibtotal, i;
	unsigned long v1,v2;

	mib_get(MIB_DHCP_MODE, (void *)&mode);
	if(mode != DHCP_LAN_SERVER){
		printf("Reserve ip is only valid in server mode! Please set DHCP to server mode at first!\n");
		return -1;
	}

	mibtotal = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	for (i=0; i<mibtotal; i++) {
		mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void*)&Entry);
		v1 = *((unsigned long *)Entry.ipAddr_Dhcp);
		v2 = *((unsigned long *)ip);
		if(v1 == v2){
			if(mib_chain_delete(MIB_MAC_BASE_DHCP_TBL, i) != 1) {
				printf("Delete chain record error!\n");
				return -1;
			}
			break;
		}
	}

	if(i>= mibtotal){
		printf("Not find this ip in reserved ip table!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_dhcp();
#endif

	return 0;
}

void showDhcpsReservedIp()
{
	char macAddr[20];
	char ipAddr[20];
	unsigned int entryNum, i;
	MIB_CE_MAC_BASE_DHCP_T Entry;

	entryNum = mib_chain_total(MIB_MAC_BASE_DHCP_TBL);
	printf("MAC-Based Assignment Table:\n");
	printf("Idx  Host MAC Address         Assigned IP Address\n");
	printf("-------------------------------------------------------------------------\n");
	if (!entryNum)
		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_MAC_BASE_DHCP_TBL, i, (void *)&Entry)) {
  			printf("Get chain record error!\n");
			return;
		}
		sprintf(macAddr,"%02x:%02x:%02x:%02x:%02x:%02x",Entry.macAddr_Dhcp[0],Entry.macAddr_Dhcp[1],Entry.macAddr_Dhcp[2],Entry.macAddr_Dhcp[3],Entry.macAddr_Dhcp[4],Entry.macAddr_Dhcp[5]);
		sprintf(ipAddr,"%s",inet_ntoa(*(struct in_addr *)Entry.ipAddr_Dhcp));
		printf("%-5d%-25s%s\n", i+1, macAddr, ipAddr);
	}
}

void showDhcpsClient()
{
	struct stat status;
	int element=0, ret, pid;
	char ipAddr[40], macAddr[40], liveTime[80], *buf=NULL, *ptr;
	FILE *fp;

	printf("\n");
	printf("Active DHCP Client Table\n");
	printf("--------------------------------------------------------------------------\n");
	printf("This table shows the assigned IP address, MAC address and time expired for \n");
	printf("each DHCP leased client. \n");
	printf("--------------------------------------------------------------------------\n");
	printf("IP Address          MAC Address         Time Expired (sec)\n");
	printf("--------------------------------------------------------------------------\n");
	// siganl DHCP server to update lease file
	pid = read_pid(DHCPSERVERPID);
	if (pid > 0)
		kill(pid, SIGUSR1);
	usleep(1000);
	if (stat(DHCPD_LEASE, &status) < 0)
		goto end_dhcp;

	// read DHCP server lease file
	buf = malloc(status.st_size);
	if (buf == NULL)
		goto end_dhcp;
	fp = fopen(DHCPD_LEASE, "r");
	if (fp == NULL)
		goto end_dhcp;
	fread(buf, 1, status.st_size, fp);
	fclose(fp);

	ptr = buf;
	while (1) {
		ret = getOneDhcpClient(&ptr, &status.st_size, ipAddr, macAddr, liveTime);
		if (ret < 0)
			break;
		if (ret == 0)
			continue;
		printf("%-20s%-20s%s\n", ipAddr, macAddr, liveTime);
		element ++;
	}
	if (!element)
		printf("No data!\n\n");
end_dhcp:
	if (buf)
		free(buf);
}


void showDhcpsSetting()
{
	unsigned char vChar;
	struct in_addr ip;
	unsigned char buffer[32];
	struct in_addr lanIp, lanMask;
	unsigned char ipprefix[16];
#ifdef DHCPS_DNS_OPTIONS
	char *pDns;
	unsigned char dnsopt;
#endif

	mib_get(MIB_DHCP_MODE, (void *)&vChar);
	switch(vChar){
		case DHCP_LAN_NONE:
			printf("DHCP mode: NONE\n");
			break;
		case DHCP_LAN_RELAY:
			printf("DHCP mode: RELAY\n");
			printf("relay server: %s\n",getMibInfo(MIB_ADSL_WAN_DHCPS));
			break;
		case DHCP_LAN_SERVER:
			printf("DHCP mode: SERVER\n");
#ifdef CONFIG_SECONDARY_IP
			mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&buffer );
			if (buffer[0])
				mib_get(MIB_ADSL_LAN_DHCP_POOLUSE, (void *)buffer);
#else
			buffer[0] = 0;
#endif
			if (buffer[0] == 0){ // primary LAN
				if(!mib_get(MIB_ADSL_LAN_IP, (void *)&lanIp) || !mib_get(MIB_ADSL_LAN_SUBNET, (void *)&lanMask))
					return;
			}else{ // secondary LAN
				if(!mib_get(MIB_ADSL_LAN_IP2, (void *)&lanIp) || !mib_get(MIB_ADSL_LAN_SUBNET2, (void *)&lanMask))
					return;
			}

#ifndef DHCPS_POOL_COMPLETE_IP
			getSYS2Str(SYS_DHCPS_IPPOOL_PREFIX, ipprefix);
#endif

			printf("LAN IP Address: %s\t", inet_ntoa(lanIp));
			printf("Subnet Mask: %s\n", inet_ntoa(lanMask));
			getSYS2Str(SYS_LAN_DHCP, strbuf);
			printf("DHCP Server: %s\n", strbuf);
			mib_get( MIB_ADSL_LAN_IP, (void *)buffer);
#ifndef DHCPS_POOL_COMPLETE_IP
			printf("IP Pool Range: %s%s - ", ipprefix, getMibInfo(MIB_ADSL_LAN_CLIENT_START));
			printf("%s%s\n",  ipprefix, getMibInfo(MIB_ADSL_LAN_CLIENT_END));
#else
			printf("IP Pool Range: %s - ", getMibInfo(MIB_DHCP_POOL_START));
			printf("%s\n", getMibInfo(MIB_DHCP_POOL_END));
#endif
			printf("Max Lease Time: %s\n", getMibInfo(MIB_ADSL_LAN_DHCP_LEASE));
			printf("Domain Name: %s\n", getMibInfo(MIB_ADSL_LAN_DHCP_DOMAIN));
			printf("Gateway Address: %s\n", getMibInfo(MIB_ADSL_LAN_DHCP_GATEWAY));
#ifdef DHCPS_DNS_OPTIONS
			mib_get(MIB_DHCP_DNS_OPTION, (void *)&dnsopt);
			printf("DNS Options: %s\n", dnsopt==0?"Use DNS Relay":"Use Manual Setting");
			if (dnsopt == 1) { // manual setting
				printf("  DNS1: %s\n", getMibInfo(MIB_DHCPS_DNS1));
				pDns = getMibInfo(MIB_DHCPS_DNS2);
				if (!strcmp(pDns, "0.0.0.0"))
					printf("  DNS2: \n");
				else
					printf("  DNS2: %s\n", pDns);
				pDns = getMibInfo(MIB_DHCPS_DNS3);
				if (!strcmp(pDns, "0.0.0.0"))
					printf("  DNS3: \n");
				else
					printf("  DNS3: %s\n", pDns);
			}
#endif
			break;
	}
}
#endif

#ifdef DMZ
int setDmzCapability(char enabled, struct in_addr *ip)
{
	struct in_addr ipAddr, curIpAddr, curSubnet, secondIpAddr, secondSubnet;
	unsigned long v1, v2, v3;
#ifdef CONFIG_SECONDARY_IP
	unsigned long v4, v5;
	char ip2Enable;
#endif

	if(enabled){
		memcpy(&ipAddr,ip,4);
		//inet_aton(ip, (struct in_addr *)&ipAddr);
		mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
		mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);
#ifdef CONFIG_SECONDARY_IP
		mib_get( MIB_ADSL_LAN_ENABLE_IP2, (void *)&ip2Enable );
		mib_get( MIB_ADSL_LAN_IP2, (void *)&secondIpAddr );
		mib_get( MIB_ADSL_LAN_SUBNET2, (void *)&secondSubnet);
#endif

		v1 = *((unsigned long *)&ipAddr);
		v2 = *((unsigned long *)&curIpAddr);
		v3 = *((unsigned long *)&curSubnet);
#ifdef CONFIG_SECONDARY_IP
		v4 = *((unsigned long *)&secondIpAddr);
		v5 = *((unsigned long *)&secondSubnet);
#endif

		if (v1) {
			if ( (((v1 & v3) != (v2 & v3))
#ifdef CONFIG_SECONDARY_IP
			 && !ip2Enable) ||(((v1 & v5) != (v4 & v5)) && ((v1 & v3) != (v2 & v3))
#endif
			 )){
				printf("Invalid IP address! It should be set within the current subnet.\n");
				return -1;
			}
		}
		if ( mib_set(MIB_DMZ_IP, (void *)&ipAddr) == 0) {
			printf("Set DMZ MIB error!\n");
			return -1;
		}
	}

	if ( mib_set(MIB_DMZ_ENABLE, (void *)&enabled) == 0) {
		printf("Set MIB_DMZ_ENABLE error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	setupDMZ();
#endif

	return 0;
}

void showDmzSetting()
{
	char vChar;
	struct in_addr ipAddr;

	mib_get(MIB_DMZ_ENABLE, (void *)&vChar);
	if(vChar == 1){
		printf("DMZ is enabled\n");
		mib_get(MIB_DMZ_IP, (void *)&ipAddr);
		printf("DMZ host: %s\n",inet_ntoa(ipAddr));
	}else{
		printf("DMZ is disabled\n");
	}
}
#endif

int setDnsServer(unsigned char mode,struct in_addr *dns1,struct in_addr *dns2,struct in_addr *dns3)
{
	if(mode == DNS_MANUAL){
		if(dns1){
			if(!mib_set(MIB_ADSL_WAN_DNS1, (void *)dns1)){
				printf("set MIB_ADSL_WAN_DNS1 error!\n");
				return -1;
			}
		}
		if(dns2){
			if(!mib_set(MIB_ADSL_WAN_DNS2, (void *)dns2)){
				printf("set MIB_ADSL_WAN_DNS2 error!\n");
				return -1;
			}
		}
		if(dns3){
			if(!mib_set(MIB_ADSL_WAN_DNS3, (void *)dns3)){
				printf("set MIB_ADSL_WAN_DNS3 error!\n");
				return -1;
			}
		}
	}else{
		mode = DNS_AUTO;
	}

	if(!mib_set(MIB_ADSL_WAN_DNS_MODE, (void *)&mode)){
		printf("set MIB_ADSL_WAN_DNS_MODE error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
	restart_dnsrelay();
#endif
#endif

	return 0;
}


void showDnsSetting()
{
	unsigned char vChar;
	char * pDns;

	mib_get(MIB_ADSL_WAN_DNS_MODE, (void *)&vChar);
	printf("DNS Mode: %s\n", vChar == DNS_AUTO ? "auto" : "manually" );
	if(vChar == DNS_MANUAL){
		printf("  DNS1: %s\n", getMibInfo(MIB_ADSL_WAN_DNS1));
		pDns = getMibInfo(MIB_ADSL_WAN_DNS2);
		if (!strcmp(pDns, "0.0.0.0"))
			printf("  DNS2: \n");
		else
			printf("  DNS2: %s\n", pDns);
		pDns = getMibInfo(MIB_ADSL_WAN_DNS3);
		if (!strcmp(pDns, "0.0.0.0"))
			printf("  DNS3: \n");
		else
			printf("  DNS3: %s\n", pDns);
	}
}

#ifdef DOMAIN_BLOCKING_SUPPORT
int addDomainBlockingEntry(char *domainName)
{
	MIB_CE_DOMAIN_BLOCKING_T entry;
	int i, intVal;
	unsigned int totalEntry;

	totalEntry = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);
	for (i = 0 ; i < totalEntry; i++) {
		if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&entry)) {
			printf(errGetEntry);
			return -1;
		}
		if (!strcmp(entry.domain, domainName)){
			printf(strMACInList);
			return -1;
		}
	}
	strcpy(entry.domain, domainName);
	intVal = mib_chain_add(MIB_DOMAIN_BLOCKING_TBL, (unsigned char*)&entry);
	if (intVal == 0) {
		printf("Add Domain Blocking chain record error.\n");
		return -1;
	}else if (intVal == -1) {
		printf(strTableFull);
		return -1;
	}

	return 0;
}
int deleteDomainBlockingEntry(char *domainName)
{
	unsigned int totalEntry;
	unsigned int i;
	MIB_CE_DOMAIN_BLOCKING_T entry;

	totalEntry = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL); /* get chain record size */
	if (totalEntry==0) {
		printf("Empty table!\n");
		return -1;
	}

	for(i=0;i<totalEntry;i++)
	{
		if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&entry)) {
			printf(errGetEntry);
			return -1;
		}
		if (!strcmp(entry.domain, domainName)){
			if(mib_chain_delete(MIB_DOMAIN_BLOCKING_TBL, i) != 1){
				printf("Delete chain record error!");
				return -1;
			}
			break;
		}
	}

	return 0;
}
int setDomainBlockingCapablility(unsigned char enabled)
{
	if (mib_set(MIB_DOMAINBLK_CAPABILITY, (void *)&enabled) == 0){
		printf("Set domain blocking enabled flag error!\n");
		return -1;
	}

	return 0;
}
void showDomainBlocking()
{
	unsigned char vChar;

	mib_get(MIB_DOMAINBLK_CAPABILITY, (void *)&vChar);
	printf("Domain blocking capability is %s\n",vChar==0?"disabled":"enabled");
	if(vChar){
		unsigned int entryNum, i;
		MIB_CE_DOMAIN_BLOCKING_T Entry;

		entryNum = mib_chain_total(MIB_DOMAIN_BLOCKING_TBL);
		printf("Domain Block Table:\n");
		printf("Idx     Domain\n");
   		printf("-------------------------------------------------------------------------\n");
   		if (!entryNum)
   			printf("No data!\n\n");

		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(MIB_DOMAIN_BLOCKING_TBL, i, (void *)&Entry)) {
  				printf("Get chain record error!\n");
				return;
			}
			printf("%-8d%s\n", i+1, Entry.domain);
		}
	}
}
#endif

int getIfModeByName(char *pIfname)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[6];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		if (PPP_INDEX(Entry.ifIndex) != 0x0f)
		{	// PPP interface
			snprintf(ifname,6,"ppp%u",PPP_INDEX(Entry.ifIndex));
		}else{	// vc interface
			snprintf(ifname,5,"vc%u",VC_INDEX(Entry.ifIndex));
		}

		if(!strcasecmp(ifname,pIfname)){
			break;
		}
	}

	if(i>= entryNum){
		printf("not find this interface!\n");
		return -1;
	}

	return(Entry.cmode);
}

int getIfModeByIndex(int ifIndex)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		if(Entry.ifIndex == ifIndex){
			break;
		}
	}

	if(i>= entryNum){
		printf("not find this interface!\n");
		return -1;
	}

	return(Entry.cmode);
}

int getIfIndexByName(char *pIfname)
{
	unsigned int entryNum, i;
	MIB_CE_ATM_VC_T Entry;
	char ifname[6];

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (Entry.enable == 0)
			continue;

		if (PPP_INDEX(Entry.ifIndex) != 0x0f)
		{	// PPP interface
			snprintf(ifname,6,"ppp%u",PPP_INDEX(Entry.ifIndex));
		}else{	// vc interface
			snprintf(ifname,5,"vc%u",VC_INDEX(Entry.ifIndex));
		}

		if(!strcasecmp(ifname,pIfname)){
			break;
		}
	}

	if(i>= entryNum){
		printf("not find this interface!\n");
		return -1;
	}

	return(Entry.ifIndex);
}

#ifdef CONFIG_USER_IGMPPROXY
#ifdef EMBED
#if defined(APPLY_CHANGE)
#define IGMP_RUNFILE "/var/run/igmp_pid"
int resetIGMProxy(int restart, char * pIfname)
{
	char * argv[8];
	int igmp_pid;

	/*check interface name at first*/
	if(restart){
		if(getIfModeByName(pIfname) <= CHANNEL_MODE_BRIDGE){
			printf("no such route interface!\n");
			return -1;
		}
	}

	igmp_pid = read_pid(IGMP_RUNFILE);
	if (igmp_pid >= 1) {
		// kill it
		if (kill(igmp_pid, SIGTERM) != 0) {
			printf("Could not kill pid '%d'", igmp_pid);
			return -1;
		}
	}

	if (restart) {
		//igmpproxy -c 1 -d br0 -u nas0_0,vc0 -D
		argv[1] = "-c";
		argv[2] = "1";
		argv[3] = "-d";
		argv[4] = (char *)LANIF;
		argv[5] = "-u";
		argv[6] = pIfname;
		argv[7] = NULL;
		do_cmd(IGMPROXY, argv, 0);
	}

	return 0;
}
#endif
#endif

int setIgmpProxyCapability(char enabled, char * pIfname)
{
	unsigned char ifIndex;

	if(enabled){
		if((ifIndex = getIfIndexByName(pIfname)) < 0){
			printf("No such interface!\n");
			return -1;
		}
		if(getIfModeByIndex(ifIndex) <= CHANNEL_MODE_BRIDGE){
			printf("No such route interface!\n");
			return -1;
		}
		if ( !mib_set(MIB_IGMP_PROXY_ITF, (void *)&ifIndex)) {
			printf("Set MIB_IGMP_PROXY_ITF error!\n");
			return -1;
		}
	}
	if ( !mib_set(MIB_IGMP_PROXY, (void *)&enabled)) {
		printf("Set MIB_IGMP_PROXY error!\n");
		return -1;;
	}

#if defined(APPLY_CHANGE)
	resetIGMProxy(enabled, pIfname);
#endif

	return 0;
}

void showIgmpProxy()
{
	unsigned char proxy;
	unsigned char ifIndex;
	char ifname[6];

	mib_get(MIB_IGMP_PROXY, (void *)&proxy);
	printf("IGMPProxy is %s\n",proxy==0?"disabled":"enabled");
	if(proxy){
		mib_get(MIB_IGMP_PROXY_ITF, (void *)&ifIndex);

		if (PPP_INDEX(ifIndex) != 0x0f)
		{	// PPP interface
			snprintf(ifname,6,"ppp%u",PPP_INDEX(ifIndex));
		}else{	// vc interface
			snprintf(ifname,5,"vc%u",VC_INDEX(ifIndex));
		}

		printf("IGMPProxy interface: %s\n",ifname);
	}

}
#endif

#ifdef URL_BLOCKING_SUPPORT
int setUrlBlockingEntry(int action, int mibTblId, char *urlstr)
{
	MIB_CE_URL_FQDN_T entryFQDN;
	MIB_CE_KEYWD_FILTER_T entryKeyword;
	int i, intVal;
	unsigned int totalEntry;

	totalEntry = mib_chain_total(mibTblId); /* get chain record size */
	if (mibTblId == MIB_URL_FQDN_TBL){
		for (i = 0 ; i < totalEntry; i++){
			if(!mib_chain_get(mibTblId, i, (void *)&entryFQDN)){
				printf(errGetEntry);
				printf("\n");
				return -1;
			}
			if(!strcmp(entryFQDN.fqdn, urlstr)){
				if(action == URL_BLOCK_DEL){
					if(mib_chain_delete(mibTblId, i) != 1){
						printf("Delete chain record error!\n");
						return -1;
					}else{
						return 0;
					}
				}else{
					printf("This FQDN has been in table!\n");
					return -1;
				}
			}
		}
		if(action == URL_BLOCK_ADD){
			strcpy(entryFQDN.fqdn, urlstr);
			intVal = mib_chain_add(mibTblId, (unsigned char*)&entryFQDN);
			if (intVal == 0) {
				printf("Add URL chain record error.\n");
				return -1;
			}else if(intVal == -1) {
				printf(strTableFull);
				return -1;
			}
		}else{
			printf("not found this FQDN in url table!\n");
			return -1;
		}
	}else if (mibTblId == MIB_KEYWD_FILTER_TBL){
		for (i = 0 ; i < totalEntry; i++){
			if(!mib_chain_get(mibTblId, i, (void *)&entryKeyword)){
				printf(errGetEntry);
				printf("\n");
				return -1;
			}
			if (!strcmp(entryKeyword.keyword, urlstr)){
				if(action == URL_BLOCK_DEL){
					if(mib_chain_delete(mibTblId, i) != 1){
						printf("Delete chain record error!\n");
						return -1;
					}else{
						return 0;
					}
				}else{
					printf("This keyword has been in table!\n");
					return -1;
				}
			}
		}
		if(action == URL_BLOCK_ADD){
			strcpy(entryKeyword.keyword, urlstr);
			intVal = mib_chain_add(mibTblId, (unsigned char*)&entryKeyword);
			if (intVal == 0) {
				printf("Add URL chain record error.\n");
				return -1;
			}else if(intVal == -1) {
				printf(strTableFull);
				return -1;
			}
		}else{
			printf("not found this keyword in url table!\n");
			return -1;
		}
	}

#if defined(APPLY_CHANGE)
	restart_urlblocking();
#endif

	return 0;
}

int setUrlBlockingCapability(char enabled)
{
	if(mib_set(MIB_URL_CAPABILITY, (void *)&enabled) == 0){
		printf("Set enabled flag error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	restart_urlblocking();
#endif

	return 0;
}

void showURLBlkTable(int mibTblId){
	unsigned int entryNum, i;
	MIB_CE_URL_FQDN_T EntryFQDN;
	MIB_CE_KEYWD_FILTER_T EntryKeyword;

	entryNum = mib_chain_total(mibTblId);

	printf("\n");
	if (mibTblId == MIB_URL_FQDN_TBL) {
   		printf("URL Blocking Table:\n");
		printf("Idx     FQDN\n");
	}
	else if (mibTblId == MIB_KEYWD_FILTER_TBL) {
		printf("Keyword Filtering Table:\n");
		printf("Idx 	Filtered Keyword\n");
	}
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	if (mibTblId == MIB_URL_FQDN_TBL) {
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(mibTblId, i, (void *)&EntryFQDN)) {
	  			printf("Get chain record error!\n");
				return;
			}
			printf("%-8d%s\n", i+1, EntryFQDN.fqdn);
		}
	}
	else if (mibTblId == MIB_KEYWD_FILTER_TBL) {
		for (i=0; i<entryNum; i++) {
			if (!mib_chain_get(mibTblId, i, (void *)&EntryKeyword)) {
	  			printf("Get chain record error!\n");
				return;
			}
			printf("%-8d%s\n", i+1, EntryKeyword.keyword);
		}
	}
}

void showUrlBlocking()
{
	unsigned char vChar;

	mib_get(MIB_URL_CAPABILITY, (void *)&vChar);
	printf("URL blocking is %s\n",vChar == 0? "disabled" : "enabled");
	if(vChar){
		showURLBlkTable(MIB_URL_FQDN_TBL);
		showURLBlkTable(MIB_KEYWD_FILTER_TBL);
	}
}
#endif

#if defined(CONFIG_USER_UPNPD)||defined(CONFIG_USER_MINIUPNPD)
int setUpnpCapability(unsigned char enabled,unsigned char ifIndex)
{
	if(enabled){
		if(getIfModeByIndex(ifIndex) <= CHANNEL_MODE_BRIDGE){
			printf("no such route interface!\n");
			return -1;
		}
		if ( !mib_set(MIB_UPNP_EXT_ITF, (void *)&ifIndex)) {
			printf("Set UPnP interface index error!\n");
			return -1;
		}
	}

	if ( !mib_set(MIB_UPNP_DAEMON, (void *)&enabled)) {
		printf("Set UPnP error!\n");
		return -1;;
	}

	return 0;
}

void showUpnp()
{
	unsigned char enabled;
	unsigned char ifIndex;
	char ifname[6];

	mib_get(MIB_UPNP_DAEMON, (void *)&enabled);
	printf("UPnP is %s\n",enabled==0?"disabled":"enabled");
	if(enabled){
		mib_get(MIB_UPNP_EXT_ITF, (void *)&ifIndex);

		if (PPP_INDEX(ifIndex) != 0x0f)
		{	// PPP interface
			snprintf(ifname,6,"ppp%u",PPP_INDEX(ifIndex));
		}else{	// vc interface
			snprintf(ifname,5,"vc%u",VC_INDEX(ifIndex));
		}

		printf("UPnP interface: %s\n",ifname);
	}

}
#endif

#ifdef IP_PORT_FILTER
int setDefaultIpPortFilterAction(char inact, char outact)
{
	if ( mib_set( MIB_IPF_OUT_ACTION, (void *)&outact) == 0) {
		printf("Set Default Filter Action error!\n");
		return -1;
	}

	if ( mib_set( MIB_IPF_IN_ACTION, (void *)&inact) == 0) {
		printf("Set Default Filter Action error!\n");
		return -1;
	}

#if defined(IP_PORT_FILTER) || defined(MAC_FILTER) || defined(DMZ)
#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_IPFilter_DMZ_MACFilter();
#endif
#endif

	return 0;
}

int ipPortFilterAdd(MIB_CE_IP_PORT_FILTER_Tp pFilterEntry)
{
	int intVal;

	if(pFilterEntry->srcPortFrom > pFilterEntry->srcPortTo){
		printf("Invalid source port range!\n");
		return -1;
	}

	if(pFilterEntry->dstPortFrom > pFilterEntry->dstPortTo){
		printf("Invalid destination port range!\n");
		return -1;
	}

	intVal = mib_chain_add(MIB_IP_PORT_FILTER_TBL, (unsigned char*)pFilterEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return -1;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		return -1;
	}

#if defined(IP_PORT_FILTER) || defined(MAC_FILTER) || defined(DMZ)
#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_IPFilter_DMZ_MACFilter();
#endif
#endif

	return 0;
}

int ipPortFilterDel(int index)
{
	unsigned int totalEntry;

	totalEntry = mib_chain_total(MIB_IP_PORT_FILTER_TBL); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return -1;
	}

	if(index > totalEntry || index <= 0){
		printf("index is error!\n");
		return -1;
	}

	if(mib_chain_delete(MIB_IP_PORT_FILTER_TBL, index-1) != 1) {
		printf("Delete chain record error!\n");
		return -1;
	}

#if defined(IP_PORT_FILTER) || defined(MAC_FILTER) || defined(DMZ)
#if defined(APPLY_CHANGE)
	// Take effect in real time
	restart_IPFilter_DMZ_MACFilter();
#endif
#endif

	return 0;
}

void showIPPortFilteringTable()
{
	unsigned int entryNum, i;
	MIB_CE_IP_PORT_FILTER_T Entry;
	char *dir, *ract;
	char *type, *sip, *dip;
	char sipaddr[20],dipaddr[20], sportRange[20], dportRange[20];
	unsigned char vChar;

	// Mason Yu
	printf("Default Action:\n");
	printf("----------------------------------------------\n");
	if ( !mib_get( MIB_IPF_OUT_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar)
		printf("Outgoing Default Action: Deny\n");
	else
		printf("Outgoing Default Action: Allow\n");

	if ( !mib_get( MIB_IPF_IN_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar)
		printf("InComing Default Action: Deny\n");
	else
		printf("InComing Default Action: Allow\n");

	// IP/Filter Rule Table
	entryNum = mib_chain_total(MIB_IP_PORT_FILTER_TBL);

   	printf("\n");
   	printf("Current Filter Table:\n");
	//printf("Idx  Local IP Address  Port Range  Port Type  Protocol Rule  Action  Comment \n");
	printf("Idx Direction Protocol SrcAddress         SrcPort     DstAddress         DstPort     Rule_Action\n");
   	printf("--------------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_IP_PORT_FILTER_TBL, i, (void *)&Entry))
		{
  			printf("showIPPortFilteringTable: Get chain record error!\n");
			return;
		}

		if (Entry.dir == DIR_OUT)
			dir = "Outgoing";
		else
			dir = "Incoming";

		// Modified by Mason Yu for Block ICMP packet
		if ( Entry.protoType == PROTO_ICMP )
		{
			type = (char *)ARG_ICMP;
		}
		else if ( Entry.protoType == PROTO_TCP )
			type = (char *)ARG_TCP;
		else
			type = (char *)ARG_UDP;

		sip = inet_ntoa(*((struct in_addr *)Entry.srcIp));
		//printf("Entry.srcIp=%x\n", *(unsigned long *)(Entry.srcIp));

		if ( !strcmp(sip, "0.0.0.0"))
			sip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(sipaddr, 20, "%s", sip);
			else
				snprintf(sipaddr, 20, "%s/%d", sip, Entry.smaskbit);
			sip = sipaddr;
		}


		if ( Entry.srcPortFrom == 0)
			strcpy(sportRange, BLANK);
		else if ( Entry.srcPortFrom == Entry.srcPortTo )
			snprintf(sportRange, 20, "%d", Entry.srcPortFrom);
		else
			snprintf(sportRange, 20, "%d-%d", Entry.srcPortFrom, Entry.srcPortTo);

		dip = inet_ntoa(*((struct in_addr *)Entry.dstIp));
		if ( !strcmp(dip, "0.0.0.0"))
			dip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(dipaddr, 20, "%s", dip);
			else
				snprintf(dipaddr, 20, "%s/%d", dip, Entry.dmaskbit);
			dip = dipaddr;
		}

		if ( Entry.dstPortFrom == 0)
			strcpy(dportRange, BLANK);
		else if ( Entry.dstPortFrom == Entry.dstPortTo )
			snprintf(dportRange, 20, "%d", Entry.dstPortFrom);
		else
			snprintf(dportRange, 20, "%d-%d", Entry.dstPortFrom, Entry.dstPortTo);

		if ( Entry.action == 0 )
			ract = "Deny";
		else
			ract = "Allow";

	 	printf("%-4d%-10s%-9s%-19s%-12s%-19s%-12s%s\n",i+1, dir, type, sip, sportRange, dip, dportRange, ract);
	}
}
#endif


#if defined(IP_QOS)
int isIpQosEnabled()
{
	unsigned char mode=0;

	mib_get(MIB_MPMODE, (void *)&mode);
	if (mode&0x02)
		return 1;
	else
		return 0;
}

int setDefaultQoS(unsigned char value)
{
	if(isIpQosEnabled())
		stopIPQ();

	if(mib_set(MIB_QOS_DOMAIN, (void *)&value) == 0){
		printf("Set MIB_QOS_DOMAIN error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	if(isIpQosEnabled())
		setupIPQ();
#endif

	return 0;
}

int setQosCapability(unsigned char enabled)
{
	unsigned char mode=0;

	mib_get(MIB_MPMODE, (void *)&mode);
	if (mode&0x02) stopIPQ();

	if (enabled)
		mode |= 0x02;
	else
		mode &= 0xfd;

	if(mib_set(MIB_MPMODE, (void *)&mode) == 0){
		printf("Set MIB_MPMODE error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	if(enabled)
		setupIPQ();
#endif

	return 0;
}

int ipQosAdd(MIB_CE_IP_QOS_Tp pEntry)
{
	int intVal;
	int totalEntry = mib_chain_total(MIB_IP_QOS_TBL);

	if (totalEntry >= MAX_QOS_RULE)
	{
		printf("Error: Maximum number of Qos rule exceeds !\n");
		return -1;
	}

	if(isIpQosEnabled())
		stopIPQ();

	intVal = mib_chain_add(MIB_IP_QOS_TBL, (void *)pEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return -1;
	}else if (intVal == -1) {
		printf(strTableFull);
		return -1;
	}

#if defined(APPLY_CHANGE)
	if(isIpQosEnabled())
		setupIPQ();
#endif

	return 0;
}

int ipQosDel(int index)
{
	if(isIpQosEnabled())
		stopIPQ();

	if(mib_chain_delete(MIB_IP_QOS_TBL, index-1) != 1) {
		printf("Delete chain record error!");
		return -1;
	}

#if defined(APPLY_CHANGE)
	if(isIpQosEnabled())
		setupIPQ();
#endif

	return 0;
}

void showIpQoS()
{
	unsigned char vChar;
	unsigned int entryNum, i, k;
	MIB_CE_IP_QOS_T Entry;
	char *psip, *pdip, sip[20], dip[20];
	const char *type;
	char sport[10], dport[10];
	unsigned int mask, smask, dmask;
	char strPhy[]="LAN0", *pPhy;
	const char *pPrio, *pIPrio, *pTos, *p1p;

	if(!isIpQosEnabled()){
		printf("IP QoS is disabled!\n");
		return;
	}else
		printf("IP QoS is enabled!\n");

	mib_get( MIB_QOS_DOMAIN, (void *)&vChar);
	printf("Default IP QoS base on %s\n",vChar? "802.1p":"IP Pred");

	entryNum = mib_chain_total(MIB_IP_QOS_TBL);
	printf("\nThe following is IP QoS rule table:\n");
  	printf("Idx  Src IP               Src Port  Des IP               Des Port  Protocol Lan Port Priority IP Precd IP Tos               Wan 802.1p\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_IP_QOS_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return ;
		}
		//source ip
		psip = inet_ntoa(*((struct in_addr *)Entry.sip));
		if ( !strcmp(psip, "0.0.0.0"))
			psip = (char *)BLANK;
		else {
			if (Entry.smaskbit==0)
				snprintf(sip, 20, "%s", psip);
			else
				snprintf(sip, 20, "%s/%d", psip, Entry.smaskbit);
			psip = sip;
		}
		// source port
		if (Entry.sPort == 0)
			strcpy(sport, BLANK);
		else
			snprintf(sport, 10, "%d", Entry.sPort);

		// destination ip
		pdip = inet_ntoa(*((struct in_addr *)Entry.dip));
		if ( !strcmp(pdip, "0.0.0.0"))
			pdip = (char *)BLANK;
		else {
			if (Entry.dmaskbit==0)
				snprintf(dip, 20, "%s", pdip);
			else
				snprintf(dip, 20, "%s/%d", pdip, Entry.dmaskbit);
			pdip = dip;
		}
		// destination port
		if (Entry.dPort == 0)
			strcpy(dport, BLANK);
		else
			snprintf(dport, 10, "%d", Entry.dPort);

		if ( Entry.protoType == PROTO_TCP )
			type = ARG_TCP;
		else if ( Entry.protoType == PROTO_UDP )
			type = ARG_UDP;
		else if ( Entry.protoType == PROTO_ICMP )
			type = ARG_ICMP;
		else
			type = (char *)BLANK;

		// Assign Priority
		if (Entry.prior <= (IPQOS_NUM_PRIOQ-1))
			pPrio = prioLevel[Entry.prior];
		else
			// should not be here !!
			pPrio = prioLevel[2];

		if (Entry.m_ipprio == 0)
			pIPrio = BLANK;
		else
			pIPrio = n0to7[Entry.m_ipprio];

		if (Entry.m_iptos == 0xff)
			pTos = BLANK;
		else {
			int mask, i;
			mask = i = 1;
			while (i <= 5) {
				if (Entry.m_iptos & mask)
					break;
				else {
					i++;
					mask<<=1;
				}
			}
			if (i >= 6)
				i = 1;
			pTos = ipTos[i];
		}

		if (Entry.m_1p == 0)
			p1p = BLANK;
		else
			p1p = n0to7[Entry.m_1p];

		pPhy = strPhy;
		if (Entry.phyPort == 0xff)
			pPhy = (char *)BLANK;
#if (defined(CONFIG_EXT_SWITCH)  && defined(IP_QOS_VPORT))
		else if (Entry.phyPort < SW_PORT_NUM)
			strPhy[3] = '0' + virt2user[Entry.phyPort];
#else
		else if (Entry.phyPort == 0)
			strPhy[3] = '0';
#endif
#ifdef CONFIG_USB_ETH
		else if (Entry.phyPort == IFUSBETH_PHYNUM)
			pPhy = (char *)USBETHIF;
#endif //CONFIG_USB_ETH
#ifdef WLAN_SUPPORT
		else
			pPhy = (char *)WLANIF[0];
#endif
		printf("%-5d%-21s%-10s%-21s%-10s%-9s%-9s%-9s%-9s%-21s%s\n",
			i+1, psip, sport, pdip, dport, type, pPhy, pPrio, pIPrio, pTos, p1p);
	}

	return;
}
#endif

#ifdef MAC_FILTER
int setDefaultMacFilteringAction(char inact, char outact)
{
	if(mib_set( MIB_MACF_OUT_ACTION, (void *)&outact) == 0){
		printf("Set MIB_MACF_OUT_ACTION error!\n");
		return -1;
	}
	outact = !outact;
	if(mib_set( MIB_ETH_MAC_CTRL, (void *)&outact) == 0){
		printf("Set MIB_ETH_MAC_CTRL error!\n");
		return -1;
	}
	if(mib_set( MIB_WLAN_MAC_CTRL, (void *)&outact) == 0){
		printf("Set MIB_WLAN_MAC_CTRL error!\n");
		return -1;
	}

	if(mib_set( MIB_MACF_IN_ACTION, (void *)&inact) == 0){
		printf("Set MIB_MACF_IN_ACTION error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
	setupMacFilter();
#endif

	return 0;
}

int addMacFiltering(MIB_CE_MAC_FILTER_Tp pEntry)
{
	int intVal;

	intVal = mib_chain_add(MIB_MAC_FILTER_TBL, (void *)pEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return -1;
	}
	else if (intVal == -1) {
		printf(strTableFull);
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
	setupMacFilter();
#endif

	return 0;
}

int deleteMacFiltering(int index)
{
	unsigned int totalEntry;

	totalEntry = mib_chain_total(MIB_MAC_FILTER_TBL); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return -1;
	}
	if(index>totalEntry || index<=0){
		printf("Error index!\n");
		return -1;
	}
	if(mib_chain_delete(MIB_MAC_FILTER_TBL, index-1) != 1) {
		printf("Delete chain record error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
	setupMacFilter();
#endif

	return 0;
}

void showMACFiltering()
{
	unsigned int entryNum, i;
	MIB_CE_MAC_FILTER_T Entry;
	char ract[6];
	char tmpbuf[100], tmpbuf2[100];
	char *dir, *outAct, *inAct;
	unsigned char vChar;

	entryNum = mib_chain_total(MIB_MAC_FILTER_TBL);

	printf("\nOutgoing and Incoming Default Action :\n");
	printf("-------------------------------------------------------------------------\n");
	if ( !mib_get( MIB_MACF_OUT_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar)
		outAct = "Deny";
	else
		outAct = "Allow";
	printf("Outgoing : %s\n", outAct);

	if ( !mib_get( MIB_MACF_IN_ACTION, (void *)&vChar) )
		return;
	if (0 == vChar)
		inAct = "Deny";
	else
		inAct = "Allow";
	printf("Incoming : %s\n", inAct);

   	printf("\n");
   	printf("Current Filter Table:\n");
	printf("Idx  Direction  Src MAC Address     Dst MAC Address      Rule Action  \n");
   	printf("-------------------------------------------------------------------------\n");
   	if (!entryNum)
   		printf("No data!\n\n");

	for (i=0; i<entryNum; i++) {
		if (!mib_chain_get(MIB_MAC_FILTER_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}

		if (Entry.dir == DIR_OUT)
			dir = "Outgoing";
		else
			dir = "Incoming";

		if ( Entry.action == 0 )
			strcpy(ract,"Deny");
		else
			strcpy(ract,"Allow");

		if (   Entry.srcMac[0]==0 && Entry.srcMac[1]==0
		    && Entry.srcMac[2]==0 && Entry.srcMac[3]==0
		    && Entry.srcMac[4]==0 && Entry.srcMac[5]==0 ) {
			strcpy(tmpbuf, "------");
		}else {
			snprintf(tmpbuf, 100, ("%02x:%02x:%02x:%02x:%02x:%02x"),
				Entry.srcMac[0], Entry.srcMac[1], Entry.srcMac[2],
				Entry.srcMac[3], Entry.srcMac[4], Entry.srcMac[5]);
		}

		if (   Entry.dstMac[0]==0 && Entry.dstMac[1]==0
		    && Entry.dstMac[2]==0 && Entry.dstMac[3]==0
		    && Entry.dstMac[4]==0 && Entry.dstMac[5]==0 ) {
			strcpy(tmpbuf2, "------");
		}else {
			snprintf(tmpbuf2, 100, ("%02x:%02x:%02x:%02x:%02x:%02x"),
				Entry.dstMac[0], Entry.dstMac[1], Entry.dstMac[2],
				Entry.dstMac[3], Entry.dstMac[4], Entry.dstMac[5]);
		}

		printf("%-5d%-11s%-20s%-21s%-10s\n",i+1,dir,tmpbuf,tmpbuf2,ract);
	}
}
#endif

#if defined(CONFIG_EXT_SWITCH) && defined(ITF_GROUP)
int isPortMappingEnabled()
{
	unsigned char mode=0;

	mib_get(MIB_MPMODE, (void *)&mode);
	if(mode&0x01)
		return 1;
	else
		return 0;
}

int setPortMappingCapability(int enabled)
{
	unsigned char mode;

	mib_get(MIB_MPMODE, (void *)&mode);
	if (enabled) {
		mode |= 0x01;
	}else
		mode &= 0xfe;
	if(mib_set(MIB_MPMODE, (void *)&mode) == 0){
		printf("Set MIB_MPMODE error!\n");
		return -1;
	}

	if(mode&0x01){
		setupEth2pvc();
	}else {
		setupEth2pvc_disable();
	}

	return 0;
}

int addGroupInterface(enum PortMappingGrp grp, char *pIfname)
{
	char select_ifid[64];
	char other_ifid[128];
	int i, ifnum;
	int find =0,othernum = 0;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];

	if(isPortMappingEnabled() == 0){
		printf("PortMapping is disabled! Please enable portMapping at first!\n");
		return -1;
	}

	memset(select_ifid,0,64);
	memset(other_ifid,0,100);
	ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, PM_DEFAULTGRP);
	if(ifnum == 0){
		printf("No available interface in default group!\n");
		return -1;
	}
	for(i=0;i<ifnum;i++){
		if(!strcasecmp(pIfname,itfs[i].name)){
			snprintf( select_ifid, 64, "%d", IF_ID(itfs[i].ifdomain, itfs[i].ifid));
			find = 1;
		}else{
			if(othernum == 0){
				snprintf( other_ifid, 64, "%d", IF_ID(itfs[i].ifdomain, itfs[i].ifid));
			}else{
				snprintf( other_ifid, 64, "%s,%d", other_ifid, IF_ID(itfs[i].ifdomain, itfs[i].ifid));
			}
			othernum++;
		}
	}
	if(find == 0){
		printf("This interface is not in default group! Please select interface from default group!\n");
		return -1;
	}

	setgroup(select_ifid, grp);
	setgroup(other_ifid, PM_DEFAULTGRP);

	// Take effect
       	setupEth2pvc();

	return 0;
}

int delGroupInterface(enum PortMappingGrp grp, char *pIfname)
{
	char select_ifid[64];
	char other_ifid[128];
	int i, ifnum;
	int find =0,othernum = 0;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];

	if(isPortMappingEnabled() == 0){
		printf("PortMapping is disabled! Please enable portMapping at first!\n");
		return -1;
	}

	memset(select_ifid,0,64);
	memset(other_ifid,0,100);
	ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, grp);
	if(ifnum == 0){
		printf("No interface in group%d !\n",grp);
		return -1;
	}
	for(i=0;i<ifnum;i++){
		if(!strcasecmp(pIfname,itfs[i].name)){
			snprintf( select_ifid, 64, "%d", IF_ID(itfs[i].ifdomain, itfs[i].ifid));
			find = 1;
		}else{
			if(othernum == 0){
				snprintf( other_ifid, 64, "%d", IF_ID(itfs[i].ifdomain, itfs[i].ifid));
			}else{
				snprintf( other_ifid, 64, "%s,%d", other_ifid, IF_ID(itfs[i].ifdomain, itfs[i].ifid));
			}
			othernum++;
		}
	}
	if(find == 0){
		printf("This interface is not in group%d !\n",grp);
		return -1;
	}

	if(othernum != 0)
		setgroup(other_ifid, grp);
	setgroup(select_ifid, PM_DEFAULTGRP);

	// Take effect
	setupEth2pvc();

	return 0;
}

void showPortmapping()
{
	int i,j, ifnum;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];

	printf("PortMapping is %s. \n", isPortMappingEnabled()?"Enabled":"Disabled");
	printf("Group interfaces:\n");
	for(i=0;i<5;i++){
		if(i == 0)
			printf("    Default group: ");
		else
			printf("    Group%d: ",i);
		ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, i);
		for(j=0;j<ifnum;j++){
			printf("%s, ",itfs[j].name);
		}
		printf("\n");
	}
}
#endif

#ifdef PORT_FORWARD_GENERAL
int setPortForwardingCapability(unsigned char enabled)
{
	if ( mib_set( MIB_PORT_FW_ENABLE, (void *)&enabled) == 0) {
		printf("Set port forwarding error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	setupPortFW();
#endif

	return 0;
}

int addPortForwarding(MIB_CE_PORT_FW_Tp pEntry)
{
	int intVal;
	struct in_addr curIpAddr, curSubnet;
	unsigned long v1, v2, v3;

	mib_get( MIB_ADSL_LAN_IP,  (void *)&curIpAddr);
	mib_get( MIB_ADSL_LAN_SUBNET,  (void *)&curSubnet);

	v1 = *((unsigned long *)(pEntry->ipAddr));
	v2 = *((unsigned long *)&curIpAddr);
	v3 = *((unsigned long *)&curSubnet);

	if ( (v1 & v3) != (v2 & v3) ) {
		printf("Invalid IP address! It should be set within the current subnet.\n");
		return -1;
	}

	if ( pEntry->fromPort > pEntry->toPort )
	{
		printf("Error! Invalid local port range.\n");
		return -1;
	}

	if ( pEntry->externalfromport > pEntry->externaltoport )
	{
		printf("Error! Invalid public port range.\n");
		return -1;
	}

	if(pEntry->ifIndex != 0xff){
		if(getIfModeByIndex(pEntry->ifIndex) <= CHANNEL_MODE_BRIDGE){
			printf("no such route interface!\n");
			return -1;
		}
	}

	intVal = mib_chain_add(MIB_PORT_FW_TBL, (unsigned char*)pEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return -1;
	}else if (intVal == -1) {
		printf(strTableFull);
		return -1;
	}

#if defined(APPLY_CHANGE)
	setupPortFW();
#endif

	return 0;
}

int deletePortForwarding(int index)
{
	unsigned int totalEntry;
	MIB_CE_PORT_FW_T Entry;

	totalEntry = mib_chain_total(MIB_PORT_FW_TBL); /* get chain record size */
	if(totalEntry==0){
		printf("Empty table!\n");
		return -1;
	}

	if(index>totalEntry || index<=0){
		printf("Error selected index!\n");
		return -1;
	}

	if (!mib_chain_get(MIB_PORT_FW_TBL, index - 1, &Entry))
	{
		printf("Get chain record error!\n");
		return -1;
	}

	if (Entry.dynamic)
	{
		printf("Created by UPnP error!\n");
		return -1;
	}

	if(mib_chain_delete(MIB_PORT_FW_TBL, index - 1) != 1)
	{
		printf("Delete chain record error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	setupPortFW();
#endif

    	return 0;
}

void showPortForwarding()
{
	unsigned char enabled;
	unsigned int entryNum, i;
	MIB_CE_PORT_FW_T Entry;
	char	type[8], portRange[20], *ip, szLocalIP[20], *pszStatus, szRemotHost[20], szPublicPortRange[20], szIfName[16];

	mib_get( MIB_PORT_FW_ENABLE, (void *)&enabled);
	printf("Port forwarding is %s.\n",enabled?"enabled":"disabled");
	printf("Current Port Forwarding Table:\n");
	printf("%-5s%-18s%-10s%-15s%-15s%-9s%-16s%-15s%-10s\n", "Idx",  "Local IP Address",  "Protocol",
           "Local Port", "Comment", "Enable", "Remote Host", "Public Port", "Interface");
	printf("-------------------------------------------------------------------------------------------------------\n");

	entryNum = mib_chain_total(MIB_PORT_FW_TBL);
	if (!entryNum)
		printf("No data!\n");

	for (i = 0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_PORT_FW_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}

		ip = inet_ntoa(*((struct in_addr *)Entry.ipAddr));
		strcpy(szLocalIP, ip);
		if ( !strcmp(szLocalIP, "0.0.0.0"))
		{
			strcpy(szLocalIP ,"----");
		}

		if ( Entry.protoType == PROTO_UDPTCP )
		{
			strcpy(type,"TCP+UDP");
		}
		else if ( Entry.protoType == PROTO_TCP )
		{
			strcpy(type,"TCP");
		}
		else
		{
			strcpy(type,"UDP");
		}

		if ( Entry.fromPort == 0)
		{
			strcpy(portRange, "----");
		}
		else if ( Entry.fromPort == Entry.toPort )
		{
			snprintf(portRange, 20, "%d", Entry.fromPort);
		}
		else
		{
			snprintf(portRange, 20, "%d-%d", Entry.fromPort, Entry.toPort);
		}

		if (Entry.enable){
			pszStatus = (char *)IF_ENABLE;
		}else{
			pszStatus = (char *)IF_DISABLE;
		}

		ip = inet_ntoa(*((struct in_addr *)Entry.remotehost));
        	strcpy(szRemotHost, ip);
		if ( !strcmp(szRemotHost, "0.0.0.0"))
		{
			strcpy(szRemotHost ,"----");
		}

		if ( Entry.externalfromport == 0)
		{
			strcpy(szPublicPortRange, "----");
		}
		else if ( Entry.externalfromport == Entry.externaltoport )
		{
			snprintf(szPublicPortRange, 20, "%d", Entry.externalfromport);
		}
		else
		{
			snprintf(szPublicPortRange, 20, "%d-%d", Entry.externalfromport, Entry.externaltoport);
		}

		if (Entry.ifIndex == 0xff)
		{
			strcpy( szIfName, "----" );
		}else if((Entry.ifIndex & 0xf0) == 0xf0){
			sprintf(szIfName, "vc%u", VC_INDEX(Entry.ifIndex));
		}else{
			sprintf(szIfName, "ppp%u", PPP_INDEX(Entry.ifIndex));
		}

		printf("%-5d%-18s%-10s%-15s%-15s%-9s%-16s%-15s%-10s\n", i + 1, szLocalIP, type, portRange,
			Entry.comment, pszStatus, szRemotHost, szPublicPortRange, szIfName);
	}
}
#endif

#ifdef REMOTE_ACCESS_CTL
int setRmtAccess(int rmtType,int lanAct,int wanAct,int port)
{
	char action;
	MIB_CE_ACC_T Entry;
	MIB_CE_ACC_T entry;

	/*check whether MIB_ACC_TBL chain is empty or not*/
	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry)) {
		memset(&entry, '\0', sizeof(MIB_CE_ACC_T));
		mib_chain_add(MIB_ACC_TBL, (unsigned char*)&entry);
		if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry))
			return -1;
	}

	filter_set_remote_access(0);

	action = (wanAct ? 0x01 : 0x00) | (lanAct ? 0x02 : 0x00);

	switch(rmtType){
		case RMT_FTP:
			Entry.ftp = action;
			if(port == 0){
				if(Entry.ftp_port == 0)
					Entry.ftp_port = 21;
			}else{
				Entry.ftp_port = port;
			}
			break;
#ifdef CONFIG_USER_TFTPD_TFTPD
		case RMT_TFTP:
			Entry.tftp = action;
			break;
#endif
#ifdef CONFIG_USER_TELNETD_TELNETD
		case RMT_TELNET:
			Entry.telnet = action;
			if(port == 0){
				if(Entry.telnet_port == 0)
					Entry.telnet_port = 23;
			}else{
				Entry.telnet_port = port;
			}
			break;
#endif
#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
		case RMT_SNMP:
			Entry.snmp = action;
			break;
#endif
		case RMT_WEB:
			Entry.web = action;
			if(port == 0){
				if(Entry.web_port == 0)
					Entry.web_port = 80;
			}else{
				Entry.web_port = port;
			}
			break;
		case RMT_PING:
			Entry.icmp = action | 0x02; /*lan ping is always enabled*/
			break;
	}

	mib_chain_update(MIB_ACC_TBL, (void *)&Entry, 0);

	filter_set_remote_access(1);

	return 0;
}

void showRmtAccess()
{
	MIB_CE_ACC_T Entry;

	if (!mib_chain_get(MIB_ACC_TBL,0, (void *)&Entry)) {
		printf("Get MIB fail!\n");
		return;
	}

	//TELNET
#ifdef CONFIG_USER_TELNETD_TELNETD
	if (Entry.telnet & 0x02)
		printf("Telnet  LAN:  Enabled\n");
	else
		printf("Telnet  LAN:  Disabled\n");
	if (Entry.telnet & 0x01)
		if (Entry.telnet_port == 23)
			printf("Telnet  WAN:  Enabled\n");
		else
			printf("Telnet  WAN:  Enabled on port %d\n", Entry.telnet_port);
	else
		printf("Telnet  WAN:  Disabled\n");
#endif

#ifdef CONFIG_USER_FTP_FTP_FTP
	//FTP
	if (Entry.ftp & 0x02)
		printf("FTP     LAN:  Enabled\n");
	else
		printf("FTP     LAN:  Disabled\n");
	if (Entry.ftp & 0x01)
		if (Entry.ftp_port == 21)
			printf("FTP     WAN:  Enabled\n");
		else
			printf("FTP     WAN:  Enabled on port %d\n", Entry.ftp_port);
	else
		printf("FTP     WAN:  Disabled\n");
#endif

#ifdef CONFIG_USER_TFTPD_TFTPD
	//TFTP
	if (Entry.tftp & 0x02)
		printf("TFTP    LAN:  Enabled\n");
	else
		printf("TFTP    LAN:  Disabled\n");
	if (Entry.tftp & 0x01)
		printf("TFTP    WAN:  Enabled\n");
	else
		printf("TFTP    WAN:  Disabled\n");
#endif

	//HTTP
	if (Entry.web & 0x02)
		printf("WEB     LAN:  Enabled\n");
	else
		printf("WEB     LAN:  Disabled\n");
	if (Entry.web & 0x01)
		if (Entry.web_port == 80)
			printf("WEB     WAN:  Enabled\n");
		else
			printf("WEB     WAN:  Enabled on port %d\n", Entry.web_port);
	else
		printf("WEB     WAN:  Disabled\n");

//HTTPS
#ifdef CONFIG_USER_BOA_WITH_SSL
	if (Entry.https & 0x02)
		printf("HTTPS   LAN:  Enabled\n");
	else
		printf("HTTPS   LAN:  Disabled\n");
	if (Entry.https & 0x01)
		if (Entry.https_port == 443)
			printf("HTTPS   WAN:  Enabled\n");
		else
			printf("HTTPS   WAN:  Enabled on port %d\n", Entry.https_port);
	else
		printf("HTTPS   WAN:  Disabled\n");
#endif //end of CONFIG_USER_BOA_WITH_SSL

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
	//SNMP
	if (Entry.snmp & 0x02)
		printf("SNMP    LAN:  Enabled\n");
	else
		printf("SNMP    LAN:  Disabled\n");
	if (Entry.snmp & 0x01)
		printf("SNMP    WAN:  Enabled\n");
	else
		printf("SNMP    WAN:  Disabled\n");
#endif

#ifdef CONFIG_USER_SSH_DROPBEAR
	//SSH
	if (Entry.ssh & 0x02)
		printf("SSH     LAN:  Enabled\n");
	else
		printf("SSH     LAN:  Disabled\n");
	if (Entry.ssh & 0x01)
		printf("SSH     WAN:  Enabled\n");
	else
		printf("SSH     WAN:  Disabled\n");
#endif

	//ICMP
	printf("ICMP    LAN:  Enabled\n");
	if (Entry.icmp & 0x01)
		printf("ICMP    WAN:  Enabled\n");
	else
		printf("ICMP    WAN:  Disabled\n");

}
#endif

#ifdef CONFIG_USER_ROUTED_ROUTED
int setRipCapability(unsigned char enabled)
{
	if (!mib_set(MIB_RIP_ENABLE, (void *)&enabled)) {
		printf("Set RIP error!");
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
      	startRip();
#endif

	return 0;
}

int addRipInterface(MIB_CE_RIP_Tp pEntry)
{
	int entryNum, i, intVal;
	MIB_CE_RIP_T ripEntry;

	if(pEntry->ifIndex != 0xff){
		if((pEntry->ifIndex == -1) || (getIfModeByIndex(pEntry->ifIndex) <= CHANNEL_MODE_BRIDGE)){
			printf("no such route interface!\n");
			return -1;
		}
	}

	// check RIP table
	entryNum = mib_chain_total(MIB_RIP_TBL);
	for (i=0; i<entryNum; i++) {
		mib_chain_get(MIB_RIP_TBL, i, (void *)&ripEntry);
		if (ripEntry.ifIndex == pEntry->ifIndex) {
			printf("Entry already exists!\n");
			return -1;
		}
	}

	intVal = mib_chain_add(MIB_RIP_TBL, (unsigned char*)pEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return -1;
	}else if (intVal == -1) {
		printf(strTableFull);
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
      	startRip();
#endif

	return 0;
}

int delRipInterface(int index)
{
	unsigned int totalEntry;

	totalEntry = mib_chain_total(MIB_RIP_TBL); /* get chain record size */
	if(totalEntry == 0){
		printf("Empty table!\n");
		return -1;
	}

	if(index > totalEntry || index <= 0){
		printf("Error selected index!\n");
		return -1;
	}

	if(mib_chain_delete(MIB_RIP_TBL, index - 1) != 1)
	{
		printf("Delete chain record error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
      	startRip();
#endif

	return 0;
}

void showRipStatusTbl()
{
	unsigned char enabled;
	unsigned int entryNum, i;
	MIB_CE_RIP_T Entry;
	char ifname[IFNAMSIZ];
	char *receive_mode, *send_mode;
	char mode_none[] = "None";
	char mode_rip1[] = "RIP1";
	char mode_rip2[] = "RIP2";
	char mode_both[] = "Both";
	char mode_rip1comp[] = "RIP1COMPAT";

	mib_get(MIB_RIP_ENABLE, (void *)&enabled);
	if(enabled == 0){
		printf("RIP is disabled!\n");
		return;
	}else{
		printf("RIP is enabled!\n");
	}

	printf("The following is RIP interface table:\n");
	entryNum = mib_chain_total(MIB_RIP_TBL);
	printf("Index\tIface\tReceive\tSend\n");
	MSG_LINE;
	if (!entryNum) {
		printf("No data!\n\n");
		return;
	}

	for (i=0; i<entryNum; i++) {
		mib_chain_get(MIB_RIP_TBL, i, (void *)&Entry);

		if( Entry.ifIndex == 0xff) {
			strncpy(ifname, BRIF, 6);
			ifname[5] = '\0';
		} else {
			ifGetName(Entry.ifIndex, ifname, sizeof(ifname));
		}

		if ( Entry.receiveMode == RIP_NONE ) {
			receive_mode = mode_none;
		} else if ( Entry.receiveMode == RIP_V1 ) {
			receive_mode = mode_rip1;
		} else if ( Entry.receiveMode == RIP_V2 ) {
			receive_mode = mode_rip2;
		} else if ( Entry.receiveMode == RIP_V1_V2 ) {
			receive_mode = mode_both;
		} else {
			receive_mode = mode_none;
		}

		if ( Entry.sendMode == RIP_NONE ) {
			send_mode = mode_none;
		} else if ( Entry.sendMode == RIP_V1 ) {
			send_mode = mode_rip1;
		} else if ( Entry.sendMode == RIP_V2 ) {
			send_mode = mode_rip2;
		} else if ( Entry.sendMode == RIP_V1_COMPAT ) {
			send_mode = mode_rip1comp;
		} else {
			send_mode = mode_none;
		}

		printf("%d\t%s\t%s\t%s\n", i+1, ifname, receive_mode, send_mode);
	}
}
#endif

#ifdef ROUTING
int staticRouteAdd(MIB_CE_IP_ROUTE_Tp pEntry)
{
	unsigned char destNet[4];
	int i, intVal;
	unsigned int totalEntry =0;

	for (i=0; i<4; i++) {
		destNet[i] = pEntry->destID[i] & pEntry->netMask[i];
		if (destNet[i] != pEntry->destID[i]) {
			printf("The specified mask parameter is invalid! (Destination & Mask) != Destination.\n");
			return -1;
		}
	}

	if (!checkRoute(*pEntry, -1)) {
		printf(Tinvalid_rule);
		return -1;
	}

	if(pEntry->ifIndex != 0xff){
		if(getIfModeByIndex(pEntry->ifIndex) <= CHANNEL_MODE_BRIDGE){
			printf("no such route interface!\n");
			return -1;
		}
	}

	intVal = mib_chain_add(MIB_IP_ROUTE_TBL, (void *)pEntry);
	if (intVal == 0) {
		printf(strAddChainerror);
		return -1;
	}else if (intVal == -1) {
		printf(strTableFull);
		return -1;
	}
	/* get chain record size */
	totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); 
	route_cfg_modify(pEntry, 0, totalEntry-1);

	return 0;
}

int staticRouteDel(struct in_addr *dstip, struct in_addr *netmask)
{
	int skfd;
	unsigned int idx;
	MIB_CE_IP_ROUTE_T Entry;
	struct rtentry rt;
	unsigned int totalEntry = mib_chain_total(MIB_IP_ROUTE_TBL); /* get chain record size */
	struct sockaddr_in *s_in;

	/* get the specified chain record */
	for(idx=0;idx<totalEntry;idx++){
		if (!mib_chain_get(MIB_IP_ROUTE_TBL, idx, (void *)&Entry)){
			printf("Get mib chain error!\n");
			return -1;
		}
		if((dstip->s_addr == ((struct in_addr *)(Entry.destID))->s_addr) &&
			(netmask->s_addr == ((struct in_addr *)(Entry.netMask))->s_addr)){
			break;
		}
	}
	if(idx >= totalEntry){
		printf("Not find this route entry!\n");
		return -1;
	}

	/* Clean out the RTREQ structure. */
	memset((char *) &rt, 0, sizeof(struct rtentry));
	rt.rt_flags = RTF_UP;
	s_in = (struct sockaddr_in *)&rt.rt_dst;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)Entry.destID;

	s_in = (struct sockaddr_in *)&rt.rt_genmask;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)Entry.netMask;

	s_in = (struct sockaddr_in *)&rt.rt_gateway;
	s_in->sin_family = AF_INET;
	s_in->sin_port = 0;
	s_in->sin_addr = *(struct in_addr *)Entry.nextHop;

	rt.rt_flags |= RTF_GATEWAY;

	// delete from chain record
	if(mib_chain_delete(MIB_IP_ROUTE_TBL, idx) != 1) {
		printf("Delete chain record error!\n");
		return -1;
	}

	/* Create a socket to the INET kernel. */
	if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Create socket error!\n");
		return -1;
	}

	/* Tell the kernel to delete this route. */
	if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
		printf("kernel delete route error!\n");
		close(skfd);
		return -1;
	}

	/* Close the socket. */
	close(skfd);

	return 0;
}
#endif

#ifdef CONFIG_USER_SNMPD_SNMPD_V2CTRAP
int isSnmpEnabled()
{
	char enabled;

	mib_get(MIB_SNMPD_ENABLE, (void *)&enabled);
	if(enabled == 0)
		return 0;
	else
		return 1;
}

int setSnmpCapability(char enabled)
{
	if(mib_set(MIB_SNMPD_ENABLE, (void *)&enabled) == 0){
		printf("Set MIB_SNMPD_ENABLE error!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	if (enabled)
		restart_snmp(1);
	else
		restart_snmp(0);
#endif

	return 0;
}

static int validateKey(char *str)
{
	int i,len;
	len=strlen(str);
   	for (i=0; i<len; i++) {
		if ( (str[i] >= '0' && str[i] <= '9') ||(str[i] == '.' ) )
			continue;
		return 0;
  	}
  	return 1;
}

int cfgSnmpSetting(SNMP_PARA_P pSnmpPara)
{
	char *str;
	struct in_addr *pIp;

	str = pSnmpPara->pSysDescription;
	if(str){
		if ( !mib_set(MIB_SNMP_SYS_DESCR, (void *)str)) {
			printf("Set snmpSysDescr mib error!\n");
			return -1;
		}
	}

	str = pSnmpPara->pSysContact;
	if(str){
		if ( !mib_set(MIB_SNMP_SYS_CONTACT, (void *)str)) {
			printf("Set snmpSysContact mib error!\n");
			return -1;
		}
	}

	str = pSnmpPara->pSysName;
	if(str){
		if ( !mib_set(MIB_SNMP_SYS_NAME, (void *)str)) {
			printf("Set snmpSysName mib error!\n");
			return -1;
		}
	}

	str = pSnmpPara->pSysLocation;
	if(str){
		if ( !mib_set(MIB_SNMP_SYS_LOCATION, (void *)str)) {
			printf("Set snmpSysLocation mib error!\n");
			return -1;
		}
	}

	str = pSnmpPara->pSysObjectID;
	if(str){
		if(!validateKey(str)){
			printf("Invalid Object ID value. It should be fill with OID string.\n");
			return -1;
		}
		if ( !mib_set(MIB_SNMP_SYS_OID, (void *)str)) {
			printf("Set snmpSysObjectID mib error!\n");
			return -1;
		}
	}

	str = pSnmpPara->pROCommunity;
	if(str){
		if ( !mib_set(MIB_SNMP_COMM_RO, (void *)str)) {
			printf("Set snmpCommunityRO mib error!\n");
			return -1;
		}
	}

	str = pSnmpPara->pRWCommunity;
	if(str){
		if ( !mib_set(MIB_SNMP_COMM_RW, (void *)str)) {
			printf("Set snmpCommunityRW mib error!\n");
			return -1;
		}
	}

	pIp = pSnmpPara->pTrapIp;
	if(pIp){
		if ( !mib_set(MIB_SNMP_TRAP_IP, (void *)pIp)) {
			printf("Set snmpTrapIpAddr mib error!\n");
			return -1;
		}
	}

#if defined(APPLY_CHANGE)
	if (isSnmpEnabled())
		restart_snmp(1);
	else
		restart_snmp(0);
#endif

	return 0;
}

void showSnmpSettings()
{
	char str[256];
	struct in_addr trap_ip;
	unsigned char enable;

	if ( !mib_get( MIB_SNMPD_ENABLE, (void *)&enable)) {
		printf("Get snmpdEnable error!\n");
	}
	printf("SNMP is %s.\n", (enable==1)? "enabled":"disabled");

	if ( !mib_get(MIB_SNMP_SYS_DESCR, (void *)str)) {
		printf("Get snmpSysDescr mib error!\n");
	}
	printf("System Description: %s\n",str);

	if ( !mib_get(MIB_SNMP_SYS_CONTACT, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Contact: %s\n",str);

	if ( !mib_get(MIB_SNMP_SYS_NAME, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Name: %s\n",str);

	if ( !mib_get(MIB_SNMP_SYS_LOCATION, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Location: %s\n",str);

	if ( !mib_get(MIB_SNMP_SYS_OID, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("System Object ID: %s\n",str);

	if ( !mib_get(MIB_SNMP_TRAP_IP, (void *)&trap_ip)) {
		printf("Get mib error!\n");
	}
	printf("Trap IP Address: %s\n",inet_ntoa(trap_ip));

	if ( !mib_get(MIB_SNMP_COMM_RO, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("Community name (read-only): %s\n",str);

	if ( !mib_get(MIB_SNMP_COMM_RW, (void *)str)) {
		printf("Get mib error!\n");
	}
	printf("Community name (read-write): %s\n\n",str);
}
#endif

#ifdef CONFIG_USER_CWMP_TR069
int setTr069Capability(char enabled)
{
	char changeflag=0;
	unsigned char cwmp_flag=0;
	char cwmp_flag_value=0;

	if( enabled == 0) {
		if ( cwmp_flag & CWMP_FLAG_AUTORUN )
			changeflag = 1;
		cwmp_flag = cwmp_flag & (~CWMP_FLAG_AUTORUN);
		cwmp_flag_value = 0;
	}else {
		if ( !(cwmp_flag & CWMP_FLAG_AUTORUN) )
			changeflag = 1;
		cwmp_flag = cwmp_flag | CWMP_FLAG_AUTORUN;
		cwmp_flag_value = 1;
	}

	if ( !mib_set( CWMP_FLAG, (void *)&cwmp_flag)) {
		printf("Set CWMP_FLAG error!\n");
		return -1;
	}

#ifdef APPLY_CHANGE
	if ( changeflag ) {
		if ( cwmp_flag_value == 0 ) {  // disable TR069
			off_tr069();
		} else {                       // enable TR069
			off_tr069();
			if (-1==startCWMP()){
				printf("Start tr069 Fail(CLI)\n");
				return -1;
			}
		}
	}
#endif

	return 0;
}

int setTr069Acs(CWMP_ACS_P pAcs)
{
	char vChar;
	char old_vChar;
	char changeflag=0;
	unsigned int vInt;
	unsigned int informInterv;
	unsigned char cwmp_flag=0;
	char cwmp_flag_value=0;

	mib_get( CWMP_FLAG, (void *)&cwmp_flag );
	if ( cwmp_flag & CWMP_FLAG_AUTORUN )
		cwmp_flag_value = 1;
	else
		cwmp_flag_value = 0;

	// ACS URL
	if(pAcs->pUrl){
		if(!mib_set(CWMP_ACS_URL, (void *)pAcs->pUrl)){
			printf("Set CWMP_ACS_URL error!\n");
			return -1;
		}
	}

	// ACS Username
	if(pAcs->pUsrname){
		if(!mib_set(CWMP_ACS_USERNAME, (void *)pAcs->pUsrname)){
			printf("Set CWMP_ACS_USERNAME error!\n");
			return -1;
		}
	}

	// ACS Password
	if(pAcs->pPwd){
		if(!mib_set(CWMP_ACS_PASSWORD, (void *)pAcs->pPwd)){
			printf("Set CWMP_ACS_PASSWORD error!\n");
			return -1;
		}
	}

	// Inform
	if(pAcs->pIfmEn){
		vChar = *(pAcs->pIfmEn)? 1 : 0;
		mib_get( CWMP_INFORM_ENABLE, (void*)&old_vChar);
		if(old_vChar != vChar){
			changeflag = 1;
			if ( !mib_set( CWMP_INFORM_ENABLE, (void *)&vChar)) {
				printf("Set CWMP_INFORM_ENABLE error!\n");
				return -1;
			}
		}
	}

	// Inform Interval
	if(pAcs->pIfmItvl){
		vInt = *(pAcs->pIfmItvl);
		mib_get( CWMP_INFORM_INTERVAL, (void*)&informInterv);
		if(vInt != informInterv){
			changeflag = 1;
			if ( !mib_set( CWMP_INFORM_INTERVAL, (void *)&vInt)) {
				printf("Set CWMP_INFORM_INTERVAL error!\n");
				return -1;
			}
		}
	}

	// Mason Yu
#ifdef APPLY_CHANGE
	if ( changeflag ) {
		if ( cwmp_flag_value == 0 ) {  // disable TR069
			off_tr069();
		} else {                       // enable TR069
			off_tr069();
			if (-1==startCWMP()){
				printf("Start tr069 Fail(CLI)\n");
				return -1;
			}
		}
	}
#endif

	return 0;
}

int setTr069ConnectReq(CWMP_CONN_REQ_P pConnReq)
{
	unsigned int vInt;
	char changeflag=0;
	char strPath[256+1];
	int old_port;
	unsigned char cwmp_flag=0;
	char cwmp_flag_value=0;

	// Get TR069 adminStatus(Disable or Enable)
	mib_get( CWMP_FLAG, (void *)&cwmp_flag );
	if ( cwmp_flag & CWMP_FLAG_AUTORUN )
		cwmp_flag_value = 1;
	else
		cwmp_flag_value = 0;

	// Username
	if(pConnReq->pUsrname){
		if(!mib_set(CWMP_CONREQ_USERNAME, (void *)pConnReq->pUsrname)){
			printf("Set CWMP_CONREQ_USERNAME error!\n");
			return -1;
		}
	}

	// Password
	if(pConnReq->pPwd){
		if(!mib_set(CWMP_CONREQ_PASSWORD, (void *)pConnReq->pPwd)){
			printf("Set CWMP_CONREQ_PASSWORD error!\n");
			return -1;
		}
	}

	// Path
	if(pConnReq->pPath){
		mib_get( CWMP_CONREQ_PATH, (void *)strPath);
		if (strcmp(pConnReq->pPath,strPath)!=0){
			changeflag = 1;
			if ( !mib_set( CWMP_CONREQ_PATH, (void *)pConnReq->pPath)) {
				printf("Set CWMP_CONREQ_PATH error!\n");
				return -1;
			}
		}
	}

	// Port
	if(pConnReq->pPort){
		vInt = *(pConnReq->pPort);
		mib_get( CWMP_CONREQ_PORT, (void *)&old_port);
		if ( vInt != old_port ) {
			changeflag = 1;
			if ( !mib_set( CWMP_CONREQ_PORT, (void *)&vInt)) {
				printf("Set CWMP_CONREQ_PORT error!\n");
				return -1;
			}
		}

	}

#ifdef APPLY_CHANGE
	if ( changeflag ) {
		if ( cwmp_flag_value == 0 ) {  // disable TR069
			off_tr069();
		} else {                       // enable TR069
			off_tr069();
			if (-1==startCWMP()){
				printf("Start tr069 Fail(CLI)\n");
				return -1;
			}
		}
	}
#endif

	return 0;
}


void showTr069Settings()
{
	char tmpStr[256+1];
	char vChar;
	int vInt;

	mib_get( CWMP_FLAG, (void *)&vChar );
	printf("TR069 is %s.\n", (vChar & CWMP_FLAG_AUTORUN)? "Enabled":"Disabled");
	MENU_LINE
	mib_get(CWMP_ACS_URL, (void *)tmpStr);
	printf("ACS URL: %s\n", tmpStr);
	mib_get(CWMP_ACS_USERNAME, (void *)tmpStr);
	printf("ACS Username: %s\n", tmpStr);
	mib_get(CWMP_ACS_PASSWORD, (void *)tmpStr);
	printf("ACS Password: %s\n", tmpStr);
	mib_get(CWMP_INFORM_ENABLE, (void *)&vChar);
	printf("Inform: %s\n", vChar ? "Enabled" : "Disabled");
	if (vChar) {
		mib_get(CWMP_INFORM_INTERVAL, (void *)&vInt);
		printf("Inform Interval: %d\n", vInt);
	}
	MENU_LINE
	printf("Connection Request\n");
	mib_get(CWMP_CONREQ_USERNAME, (void *)tmpStr);
	printf("Username: %s\n", tmpStr);
	mib_get(CWMP_CONREQ_PASSWORD, (void *)tmpStr);
	printf("Password: %s\n", tmpStr);
	mib_get(CWMP_CONREQ_PATH, (void *)tmpStr);
	printf("PATH: %s\n", tmpStr);
	mib_get(CWMP_CONREQ_PORT, (void *)&vInt);
	printf("Port: %d\n", vInt);
}
#endif

int saveMibAll()
{
	if(!mib_update(CURRENT_SETTING, CONFIG_MIB_ALL)){
		printf("mib update current setting error!\n");
		return -1;
	}

	return 0;
}

int setAdslSettings(unsigned short mode, char olr)
{
	#ifndef CONFIG_DEV_xDSL
	printf("To be implemented\n");
	return -1;
	#else 
	int xmode;
	unsigned short oldMode;

	// annex B and annex A are incompatible,
	mib_get(MIB_ADSL_MODE, (void *)&oldMode);
	if(oldMode &= ADSL_MODE_ANXB){
		mode &= (~(ADSL_MODE_ANXL|ADSL_MODE_ANXM));
		mode |= ADSL_MODE_ANXB;
	}

	if(!mib_set(MIB_ADSL_MODE, (void *)&mode)){
		printf("Set MIB_ADSL_MODE error!\n");
		return -1;
	}
	if(!mib_set(MIB_ADSL_OLR, (void *)&olr)){
		printf("Set MIB_ADSL_OLR error!\n");
		return -1;
	}

	adsl_drv_get(RLCM_MODEM_RETRAIN, NULL, 0);
	xmode=0;
	if (mode & (ADSL_MODE_GDMT|ADSL_MODE_T1413))
		xmode |= 1;	// ADSL1
	if (mode & ADSL_MODE_ADSL2)
		xmode |= 2;	// ADSL2
	if (mode & ADSL_MODE_ADSL2P)
		xmode |= 4;	// ADSL2+
	adsl_drv_get(RLCM_SET_XDSL_MODE, (void *)&xmode, 4);

	xmode = mode & (ADSL_MODE_T1413|ADSL_MODE_GDMT);	//  1: ansi, 2: g.dmt, 8:g.lite
	adsl_drv_get(RLCM_SET_ADSL_MODE, (void *)&xmode, 4);

	if (mode & ADSL_MODE_ANXL)	// Annex L
		xmode = 3; // Wide-Band & Narrow-Band Mode
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_ANNEX_L, (void *)&xmode, 4);

	if (mode & ADSL_MODE_ANXM)	// Annex M
		xmode = 1;
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_ANNEX_M, (void *)&xmode, 4);

#ifdef ENABLE_ADSL_MODE_GINP
	if (mode & ADSL_MODE_GINP)	// G.INP
		xmode = DSL_FUNC_GINP;
	else
		xmode = 0;
	adsl_drv_get(RLCM_SET_DSL_FUNC, (void *)&xmode, 4);
#endif

	xmode = (int)olr;
	if (xmode == 2)// SRA (should include bitswap)
		xmode = 3;
	adsl_drv_get(RLCM_SET_OLR_TYPE, (void *)&xmode, 4);

	return 0;
	#endif //CONFIG_DEV_xDSL
}

int modifyAtmSettings(ATM_PARA_P pAtmPara)
{
	int i, totalEntry,found=0, mib_vc_idx;
	MIB_CE_ATM_VC_T Entry;
	MIB_CE_ATM_VC_T entry;

	totalEntry = mib_chain_total(MIB_ATM_VC_TBL); /* get chain record size */
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry)) {
			printf(errGetEntry);
			return -1;
		}
		if(Entry.vpi == pAtmPara->vpi && Entry.vci == pAtmPara->vci){
#if defined(APPLY_CHANGE)
			if(found == 0){
				// Take effect in real time
				stopConnection(&Entry);
			}
#endif
			Entry.qos = pAtmPara->qos;
			Entry.pcr = pAtmPara->pcr;
			Entry.cdvt = pAtmPara->cdvt;
			Entry.scr = pAtmPara->scr;
			Entry.mbs = pAtmPara->mbs;
			mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, i);
			if(found == 0){
				found = 1;
				mib_vc_idx = i;
				memcpy(&entry,&Entry,sizeof(entry));
			}
		}
	}

	if(found == 0){
		printf("Not find PVC with inputted vpi and vci!\n");
		return -1;
	}

#if defined(APPLY_CHANGE)
	// Take effect in real time
	startConnection(&entry, mib_vc_idx);
#endif

	return 0;
}

void showAtmSettings()
{
	unsigned int entryNum, i, k;
	MIB_CE_ATM_VC_T Entry;
	char	vpi[6], vci[6], *qos, pcr[6], scr[6], mbs[6];
	char cdvt[12];
	char *temp;
	int vcList[MAX_VC_NUM], found;

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	printf("Idx\tPVC\tQoS\tPCR\tCDVT\tSCR\tMBS\n");
	MSG_LINE;
	if (!entryNum) {
   		printf("No data!\n\n");
   		return;
   	}
	memset(vcList, 0, MAX_VC_NUM*4);

	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return;
		}

		// skip duplicate vc display
		found = 0;
		for (k=0; k<MAX_VC_NUM && vcList[k] != 0; k++) {
			if (vcList[k] == VC_INDEX(Entry.ifIndex)) {
				found = 1;
				break;
			}
		}
		if (found)
			continue;
		else
			vcList[k] = VC_INDEX(Entry.ifIndex);

		snprintf(vpi, 6, "%u", Entry.vpi);
		snprintf(vci, 6, "%u", Entry.vci);
		snprintf(pcr, 6, "%u", Entry.pcr);
		snprintf(cdvt, 12, "%u", Entry.cdvt);

		if (Entry.qos <= 3)
			qos = (char *)strQos[Entry.qos];

		if(Entry.qos > 1) {
			snprintf(scr, 6, "%u", Entry.scr);
			snprintf(mbs, 6, "%u", Entry.mbs);
		} else {
			strcpy(scr, "---");
			strcpy(mbs, "---");
		}

		printf("%d\t%s/%s\t%s\t%s\t%s\t%s\t%s\n", i,vpi,vci,qos,pcr,cdvt,scr,mbs);
	}
}

int addWanInterface(MIB_CE_ATM_VC_Tp pEntry)
{
	deleteConnection(CONFIGALL, NULL);

	// now do the actual set.
	do {
		unsigned int ifMap;	// high half for PPP bitmap, low half for vc bitmap
#ifdef DEFAULT_GATEWAY_V1
		//int drouted=-1;
		int drouted = 0;
#endif
		int i, cnt, pIdx, intVal;
		unsigned int totalEntry;
		MIB_CE_ATM_VC_T Entry;
		//MIB_CE_ATM_VC_Tp pmyEntry;

		ifMap = 0;
		cnt=0;
		totalEntry = mib_chain_total(MIB_ATM_VC_TBL);
		if (totalEntry >= MAX_VC_NUM) {
			printf(strMaxVc);
			printf("\n");
			restartWAN(CONFIGALL, NULL);
			return -1;
		}

		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
			{
	  			printf("Get chain record error!\n");
				restartWAN(CONFIGALL, NULL);
				return -1;
			}

			if (Entry.vpi == pEntry->vpi && Entry.vci == pEntry->vci)
			{
				cnt++;
				pIdx = i;
				//pmyEntry = &Entry;
			}

			ifMap |= 1 << (Entry.ifIndex & 0x0f);	// vc map
			ifMap |= (1 << 16) << ((Entry.ifIndex >> 4) & 0x0f);	// PPP map

#ifdef DEFAULT_GATEWAY_V1
			//if ((drouted == -1) && Entry.dgw && (Entry.cmode != CHANNEL_MODE_BRIDGE))	// default route entry
			//	drouted = i;
			if (Entry.cmode != CHANNEL_MODE_BRIDGE)
				if (Entry.dgw)
					drouted = 1;
#endif
		}

		if (cnt == 0)	// pvc not exists
		{
			pEntry->ifIndex = if_find_index(pEntry->cmode, ifMap);
			if (pEntry->ifIndex == 0xff)
			{
				printf("Error: Maximum number of VC exceeds !\n");
				restartWAN(CONFIGALL, NULL);
				return -1;
			}
			else if (pEntry->ifIndex == 0xef)
			{
				printf("Error: Maximum number of PPP exceeds !\n");
				restartWAN(CONFIGALL, NULL);
				return -1;
			}
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
			pEntry->ConDevInstNum = 1 + findMaxConDevInstNum(MEDIA_INDEX(pEntry->ifIndex));
			if( (pEntry->cmode==CHANNEL_MODE_PPPOE) ||
#ifdef PPPOE_PASSTHROUGH
			    ((pEntry->cmode==CHANNEL_MODE_BRIDGE)&&(pEntry->brmode==BRIDGE_PPPOE)) ||
#endif
			    (pEntry->cmode==CHANNEL_MODE_PPPOA) )
				pEntry->ConPPPInstNum = 1;
			else
				pEntry->ConIPInstNum = 1;
#endif
		}else	// pvc exists
		{
			if (!mib_chain_get(MIB_ATM_VC_TBL, pIdx, (void *)&Entry)) {	// for multisession PPPoE, get existed pvc config
				printf(errGetEntry);
				printf("\n");
				restartWAN(CONFIGALL, NULL);
				return -1;
			}
			//if (pmyEntry->cmode == CHANNEL_MODE_PPPOE)
			if (Entry.cmode == CHANNEL_MODE_PPPOE && pEntry->cmode == CHANNEL_MODE_PPPOE)
			{
				if (cnt<MAX_POE_PER_VC)
				{	// get the pvc info.
					//pEntry->qos = pmyEntry->qos;
					//pEntry->pcr = pmyEntry->pcr;
					//pEntry->encap = pmyEntry->encap;
					pEntry->qos = Entry.qos;
					pEntry->pcr = Entry.pcr;
					pEntry->encap = Entry.encap;
					ifMap &= 0xffff0000; // don't care the vc part
					pEntry->ifIndex = if_find_index(pEntry->cmode, ifMap);
					if (pEntry->ifIndex == 0xef)
					{
						printf("Error: Maximum number of PPP exceeds !\n");
						restartWAN(CONFIGALL, NULL);
						return -1;
					}
					pEntry->ifIndex &= 0xf0;
					//pEntry->ifIndex |= (pmyEntry->ifIndex&0x0f);
					pEntry->ifIndex |= (Entry.ifIndex&0x0f);
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
					//pEntry->ConDevInstNum = pmyEntry->ConDevInstNum;
					//pEntry->ConPPPInstNum = 1 + findMaxPPPConInstNum( pmyEntry->ConDevInstNum );
					pEntry->ConDevInstNum = Entry.ConDevInstNum;
					pEntry->ConPPPInstNum = 1 + findMaxPPPConInstNum(MEDIA_INDEX(Entry.ifIndex), Entry.ConDevInstNum);
#endif
				}
				else
				{
					printf("Maximum number of PPPoE connections exceeds in this vc!\n");
					restartWAN(CONFIGALL, NULL);
					return -1;
				}
			}
			else
			{
				printf("Connection already exists!\n");
				restartWAN(CONFIGALL, NULL);
				return -1;
			}
		}

#ifdef DEFAULT_GATEWAY_V1
		if (pEntry->cmode != CHANNEL_MODE_BRIDGE)
			if (drouted && pEntry->dgw) {
				printf(strDrouteExist);
				printf("\n");
				restartWAN(CONFIGALL, NULL);
				return -1;
			}
/*		if (pEntry->cmode != CHANNEL_MODE_BRIDGE)
			if (pEntry->dgw && drouted != -1) {	// default route already existed
				if (!mib_chain_get(MIB_ATM_VC_TBL, drouted, (void *)&Entry)) {
					Entry.dgw = 0;
					mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, drouted);
				}
			}
*/
      		// set default
//		pEntry->dgw = 1;
#endif
		if (pEntry->cmode == CHANNEL_MODE_PPPOE)
			pEntry->mtu = 1492;
		else
			pEntry->mtu = 1500;

#ifdef CONFIG_EXT_SWITCH
		// VLAN
		pEntry->vlan = 0; // disable
		pEntry->vid = 0; // VLAN tag
		pEntry->vprio = 0; // priority bits (0 ~ 7)
		pEntry->vpass = 0; // no pass-through
#endif

		intVal = mib_chain_add(MIB_ATM_VC_TBL, (unsigned char*)pEntry);
		if (intVal == 0) {
			printf(strAddChainerror);
			printf("\n");
			restartWAN(CONFIGALL, NULL);
			return -1;
		}else if (intVal == -1) {
			printf(strTableFull);
			printf("\n");
			restartWAN(CONFIGALL, NULL);
			return -1;
		}
		restartWAN(CONFIGALL, NULL);
	} while (0);

	return 0;
}

int delWanInterface(char *pIfname)
{
	char ifname[8];
	int idx,entryNum;
	MIB_CE_ATM_VC_T entry;
#ifdef WLAN_SUPPORT
	int ori_wlan_idx;
#endif

	entryNum = mib_chain_total(MIB_ATM_VC_TBL);
	for(idx=0;idx<entryNum;idx++){
		if (!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&entry))
		{
			printf(errGetEntry);
			printf("\n");
			return -1;
		}
		if (entry.cmode == CHANNEL_MODE_PPPOE || entry.cmode == CHANNEL_MODE_PPPOA){
			snprintf(ifname, 6, "ppp%u", PPP_INDEX(entry.ifIndex));
		}else{
			snprintf(ifname, 5, "vc%u", VC_INDEX(entry.ifIndex));
		}
		if(!strcmp(ifname,pIfname)){
			break;
		}
	}

	if(idx >= entryNum){
		printf("Not find this interface!\n");
		return -1;
	}

	deleteConnection(CONFIGALL, NULL);
	resolveServiceDependency(idx);
#ifdef _CWMP_MIB_ /*jiunming, for cwmp-tr069*/
	{
		MIB_CE_ATM_VC_T cwmp_entry;
		if (mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&cwmp_entry))
		{
#ifdef PORT_FORWARD_GENERAL
			delPortForwarding( cwmp_entry.ifIndex );
#endif
#ifdef ROUTING
			delRoutingTable( cwmp_entry.ifIndex );
#endif
		}
	}
#endif
	if(mib_chain_delete(MIB_ATM_VC_TBL, idx) != 1) {
		printf("Delete chain record error!\n");
		restartWAN(CONFIGALL, NULL);
		return -1;
	}
#ifdef CONFIG_EXT_SWITCH
#ifdef ITF_GROUP
	{
		int wanPortNum;
		unsigned int swNum, vcNum;
		MIB_CE_SW_PORT_T Entry;
		MIB_CE_ATM_VC_T pvcEntry;
		int j, grpnum;
		char mygroup;

		vcNum = mib_chain_total(MIB_ATM_VC_TBL);
		for (grpnum=1; grpnum<=4; grpnum++) {
			wanPortNum = 0;

			for (j=0; j<vcNum; j++) {
				if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&pvcEntry)) {
					printf("Get chain record error!\n");
					restartWAN(CONFIGALL, NULL);
					return -1;
			}
			if (pvcEntry.enable == 0 || pvcEntry.itfGroup!=grpnum)
				continue;
				wanPortNum++;
			}

			if (0 == wanPortNum) {
				//release LAN ports
				swNum = mib_chain_total(MIB_SW_PORT_TBL);
				for (j=swNum; j>0; j--) {
					if (!mib_chain_get(MIB_SW_PORT_TBL, j-1, (void *)&Entry)) {
						printf("Get chain record error!\n");
						restartWAN(CONFIGALL, NULL);
						return -1;
					}
					if (Entry.itfGroup == grpnum) {
						Entry.itfGroup = 0;
						mib_chain_update(MIB_SW_PORT_TBL, (void *)&Entry, j-1);
					}
				}
#ifdef WLAN_SUPPORT
				ori_wlan_idx = wlan_idx;
				wlan_idx = 0;
				//release wlan0
				mib_get(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
				if (mygroup == grpnum) {
					mygroup = 0;
					mib_set(MIB_WLAN_ITF_GROUP, (void *)&mygroup);
				}

#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
				// release wlan1
				mib_get(MIB_WLAN1_ITF_GROUP, (void *)&mygroup);
				if (mygroup == grpnum) {
					mygroup = 0;
					mib_set(MIB_WLAN1_ITF_GROUP, (void *)&mygroup);
				}

#endif
#ifdef WLAN_MBSSID
				//release MBSSID
				for (j=1; j<IFGROUP_NUM; j++) {
					mib_get(MIB_WLAN_VAP0_ITF_GROUP + ((j-1)<<1), (void *)&mygroup);
					if (mygroup == grpnum) {
						mygroup = 0;
						mib_set(MIB_WLAN_VAP0_ITF_GROUP + ((j-1)<<1), (void *)&mygroup);
					}
				}
#if defined(CONFIG_RTL_92D_DMDP) || defined(WLAN_DUALBAND_CONCURRENT)
				// release MBSSID for wlan1
				for (j=1; j<IFGROUP_NUM; j++) {
					mib_get(MIB_WLAN1_VAP0_ITF_GROUP + ((j-1)<<1), (void *)&mygroup);
					if (mygroup == grpnum) {
						mygroup = 0;
						mib_set(MIB_WLAN1_VAP0_ITF_GROUP + ((j-1)<<1), (void *)&mygroup);
					}
				}
#endif
#endif
			wlan_idx = ori_wlan_idx;
#endif // WLAN_SUPPORT
				setgroup("", grpnum);
				break;
			}
		}
	}//end
#endif	// of ITF_GROUP
#endif
	restartWAN(CONFIGALL, NULL);

	return 0;
}



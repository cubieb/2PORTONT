#include "cli_utility.h"

void cli_show_menu(char *strlist[], int size)
{
	int i;

	for( i = 0; i < size; i++ )
	{
		printf("(%d) %-33s", i+1, strlist[i]);
		if(i%2) putchar('\n');
	}
	if(i%2) putchar('\n');
}

int cli_show_dyn_menu(const char *menu[])
{
	int i;

	for( i = 0; menu[i]; i++ )
	{
		printf("(%d) %-33s", i+1, menu[i]);

		if(i%2) putchar('\n');
	}
	if(i%2) putchar('\n');

	return i;
}

int getInputNum()
{
	char buf[128];
	int num;

	fgets((char *)buf, 127, stdin);
	if ( buf[0]==0) return(0);
	//printf("Enter buf= %s\n", buf);
	if(sscanf( buf, "%d", &num)==0) return(0);
	//CLIDEBUG("Enter num=%d\n", num);
	return(num);
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
	//CLIDEBUG("Enter value=%s len=%d\n", value, strlen(value));

	if (NULL != pcIsInputInvalid)
	{
		*pcIsInputInvalid = 0;
	}

	return( strlen(value));
}

int checkString(char *str)
{
	int i;
	for (i=0; i<strlen(str); i++) {
		//if ((str[i] >= '0' && str[i] <= '9') || (str[i] >= 'A' && str[i] <= 'Z') || (str[i] >= 'a' && str[i] <= 'z') ||
		//  (str[i] == '.') || (str[i] == ':') || (str[i] == '-') || (str[i] == '_') || (str[i] == ' ') || (str[i] == '/') || (str[i] == '@'))
		if((str[i]>=32) && (str[i]<=126))
			continue;
		return 0;
	}
	return 1;
}

int getTypedInputDefault(int type, char *prompt, void *rv, void *arg1, void *arg2)
{
	char strbuf[256];

	while (1) {
		clearerr(stdin);

		printf(prompt);
		strbuf[0] = 0;
		if (INPUT_TYPE_STRING == type) {
			char *pc;
			fgets((char *)rv, (int)arg1, stdin);
			pc = (char *)rv;
			pc[strlen(pc)-1] = 0; // remove the ending LF
			//tylo, check string
			if(checkString(pc) == 0){
				pc[0]=0;
			}
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

		#ifdef CONFIG_IPV6
		case INPUT_TYPE_IP6ADDR:
			if (inet_pton(PF_INET6, strbuf, rv))
				return 1;
			break;
		#endif

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

int getTypedInput(int type, char *prompt, void *rv, void *arg1, void *arg2)
{
	char strbuf[256];

	while (1)
	{
		clearerr(stdin);

		printf(prompt);
		strbuf[0] = 0;

		if (INPUT_TYPE_STRING == type)
		{
			char *pc;
			int len;
			char *tmpp;
			char tmp[16];
			int empty;

			len = (int)arg1;
			fgets((char *)rv, len, stdin);
			pc = (char *)rv;
			pc[len-1] = 0;

			if (pc[strlen(pc)-1]=='\n')
				pc[strlen(pc)-1] = 0;
			else { // flush stdin
				empty = 0;

			while (!empty)
			{
				tmpp = fgets(tmp, 16, stdin);

				if (strlen(tmpp) < 15 || tmpp[14] == '\n')
					empty = 1;
				}
			}

			if (strlen(pc) == 0)
				return 0;

			//tylo, check string
			if (checkString(pc) == 0)
			{
				printf("Invalid string \"%s\"\n", pc);
				printWaitStr();
				return 0;
			}

			return 1;
		}

		fgets(strbuf, sizeof(strbuf) - 1, stdin);
		strbuf[strlen(strbuf)-1] = 0; // remove the ending LF

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

		#ifdef CONFIG_IPV6
		case INPUT_TYPE_IP6ADDR:
			if (inet_pton(PF_INET6, strbuf, rv))
				return 1;
			else
				printf("Invalid IPv6 address %s.\n", strbuf);
			break;
		#endif

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

#ifdef CONFIG_IPV6
int getInputIp6Addr(char *prompt, struct in6_addr *rv) {
	return (getTypedInput(INPUT_TYPE_IP6ADDR, prompt, (void *)rv, 0, 0));
}
#endif

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

void printWaitStr()
{
	char buf[128];
	getInputString("\nPress Enter key to continue...", buf, sizeof(buf)-1);
}

// Jenny, check CLI user/passwd
int auth_cli(const char *name, const char *passwd)
{
	char *xpasswd;
	struct passwd *p;

	p = getpwnam (name);
	if (p == NULL)
		return 1;

	if (p->pw_uid != 0)
		return 1;

#if defined(HAVE_GETSPNAM) && defined(HAVE_SHADOW_H)
	if (p->pw_passwd == NULL || strlen (p->pw_passwd) == 1)
	{
		struct  spwd *spw;

		setspent ();
		spw = getspnam (p->pw_name);
		if (spw != NULL)
			p->pw_passwd = spw->sp_pwdp;
		endspent ();
	}
#endif
	xpasswd = crypt (passwd, p->pw_passwd);
	return  (!xpasswd || strcmp (xpasswd, p->pw_passwd) != 0);
}

int isStrIncludedSpace( char *str )
{
	if(str)
	{
		int i;
		for( i=0; i<strlen(str); i++)
			if( isspace(str[i]) ) return 1;
	}
	return 0;
}

int isPhoneNumber( char *str )
{
	if(str)
	{
		int i;
		for( i=0; i<strlen(str); i++)
			if( isdigit(str[i])==0 && str[i]!='*' && str[i]!='#' ) return 0;
	}
	return 1;
}

void printspace(unsigned int x)
{
	while (x--)
	{
		printf(" ");
	}
}

/*
 * Name: base64encode()
 *
 * Description: Encodes a buffer using BASE64.
 */
void base64encode(unsigned char *from, char *to, int len)
{
	char base64chars[64] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";

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

  	/* Encode password ('pass') using one-way function and then use base64	 encoding. */

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
}
#else // #ifndef MULTI_USER_PRIV
void writePasswdFile()
{
	FILE *fp;
#ifdef SUPPORT_AUTH_DIGEST
	FILE *fpd;
#endif
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
}
#endif // #else MULTI_USER_PRIV
#endif  // #ifdef EMBED

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

char *getMibInfo(int id)
{
	static char strbuf[256], emptystr[] = "";

	if (getMIB2Str(id, strbuf) == 0)
		return strbuf;

	return (char *)emptystr;
}

/* Display WAN configurations */
/* int check_type:	indicates whether to use mType or not */
/* MEDIA_TYPE_T mType:	the accepted WAN type */
/* int bShowIndex:	indicates whether show the index */
/* int *map:		an array mapping from printed index to recordNum in ATM_VC_TBL */
int wanConnList(int check_type, MEDIA_TYPE_T mType, int bShowIndex, int *map)
{
	int i, k, ifcount = 0;
	struct wstatus_info sEntry[MAX_VC_NUM + MAX_PPP_NUM];

	ifcount = getWanStatus(sEntry, MAX_VC_NUM + MAX_PPP_NUM);

	if (bShowIndex)
		printf("%-6s", "Index");

	printf("%-15s%-8s%-7s%-9s%-16s%-16s%-7s\n", "Interface", "VPI/VCI",
	       "Encap", "Protocol", "IP Address", "Gateway", "Status");

	MSG_LINE;

	for (i = 0, k = 0; i < ifcount; i++) {
		if (check_type && MEDIA_INDEX(sEntry[i].ifIndex) != mType)
			continue;

		if (bShowIndex) {
			if (map) {
				map[k] = sEntry[i].recordNum;
			}
			printf("%-6d", k++);
		}

		printf("%-15s%-8s%-7s%-9s%-16s%-16s%-7s\n",
		       sEntry[i].ifDisplayName, sEntry[i].vpivci,
		       sEntry[i].encaps, sEntry[i].protocol, sEntry[i].ipAddr,
		       sEntry[i].remoteIp, sEntry[i].strStatus);
	}

	return ifcount;
}

int cli_atmVcList(int *map)
{
	//int nBytesSent=0;

	unsigned int entryNum, i, k, idx;
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
   		return 0;
   	}
	memset(vcList, 0, MAX_VC_NUM*4);

	idx = 0;
	for (i=0; i<entryNum; i++) {

		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void *)&Entry))
		{
  			printf("Get chain record error!\n");
			return -1;
		}

		if (MEDIA_INDEX(Entry.ifIndex) != MEDIA_ATM)
			continue;
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

		printf("%d\t%s/%s\t%s\t%s\t%s\t%s\t%s\n", idx,vpi,vci,qos,pcr,cdvt,scr,mbs);
		if (map)
			map[idx] = i;
		idx++;
	}

	return idx;
}

// Jenny, get default gateway information
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

int validateKey(char *str) //tylo
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

#ifdef ITF_GROUP
// int grp=0: default group
// int grp=1: Group 1
// int grp=2: Group 2
// int grp=3: Group 3
// int grp=4: Group 4
char *get_all_IFID(int grp)
{
	int i, ifnum;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];
	char *availval;

	ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, grp);
	availval = (char *)malloc(128);
	availval[0]=0;
	if (ifnum>=1) {
		snprintf(availval, 64, "%d", IF_ID(itfs[0].ifdomain, itfs[0].ifid));

		ifnum--;
		for (i=0; i<ifnum; i++) {
			snprintf(availval, 64, "%s,%d", availval, IF_ID(itfs[i+1].ifdomain, itfs[i+1].ifid));
		}
	}
	//printf("The group %d include [%s]\n", grp, availval);
	return (char *)availval;

}

// int grp=0: default group
// int grp=1: Group 1
// int grp=2: Group 2
// int grp=3: Group 3
// int grp=4: Group 4
char *get_specify_one_IFID(int grp, int sel)
{
	int i, ifnum;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];
	char *availval;

	ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, grp);
	availval = (char *)malloc(10);
	availval[0]=0;
	if (ifnum>=1) {
		for (i=0; i<ifnum; i++) {
			if ( i == (sel-1)) {
				snprintf( availval, 64, "%d", IF_ID(itfs[i].ifdomain, itfs[i].ifid));
				//printf("The No%d IFID in group %d is [%s]\n", sel, grp, availval);
				return (char *)availval;
			}

		}
	}
	return (char *)availval;

}

// int grp=0: default group
// int grp=1: Group 1
// int grp=2: Group 2
// int grp=3: Group 3
// int grp=4: Group 4
int del_IFID_from_group(int grp, int sel)
{
	int i, ifnum;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];
	char *availval;
	char del_availval[64];

	ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, grp);
	availval = (char *)malloc(128);
	availval[0]=0;
	if (ifnum>=1) {
		for (i=0; i<ifnum; i++) {
			if ( (sel-1) != i ) {
				if ( availval[0] == 0 )
					snprintf( availval, 64, "%d", IF_ID(itfs[i].ifdomain, itfs[i].ifid));
				else
					snprintf( availval, 64, "%s,%d", availval, IF_ID(itfs[i].ifdomain, itfs[i].ifid));
			} else
				snprintf( del_availval, 64, "%d", IF_ID(itfs[i].ifdomain, itfs[i].ifid));
		}

	}
	//printf("The remain IFID in group %d is [%s]\n", grp, availval);

	// Set Port Mapping
	if ( availval[0]==0 ) {
		//printf("del_availval=%s\n", del_availval);
		setgroup(del_availval, 0);
	} else {
		setgroup(availval, grp);
		setgroup(del_availval, 0);
	}

	free(availval);
	return 1;
}

int get_port_mapping_status()
{
	unsigned char mode=0;
#ifdef CONFIG_EXT_SWITCH
	mib_get(MIB_MPMODE, (void *)&mode);
	if(mode&MP_PMAP_MASK)
		return 1;
	else
		return 0;
#endif
}
#endif // #ifdef ITF_GROUP

#ifdef TIME_ZONE
#ifdef APPLY_CHANGE
int restartSNTPC()
{
	unsigned char ValGet;

	mib_get(MIB_NTP_ENABLED, (void *)&ValGet);
	if(ValGet==1) {
		stopNTP();
		startNTP();
	} else {
		stopNTP();
	}
}
#endif
#endif

void bin_to_str(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		sprintf(tmpbuf, "%02x", bin[i]);
		strcat(out, tmpbuf);
	}
}

void RegeneratePIN(char *PIN)
{
	int i, accum = 0, ran_num;

	srand(time(NULL));

	for( i = 0; i < 7; i++ )
	{
		PIN[i] = rand() % 10;
		accum += (1 + ((i+1) & 1) * 2) * PIN[i];
		PIN[i] += '0';
	}

	accum %= 10;
	PIN[i] = ((10 - accum) % 10) + '0';

	PIN[++i] = 0;
}

int ValidatePINCode(char *code)
{
	int i, accum = 0;

	for( i = 0; i < 8; i++ )
		accum += (1 + ((i+1) & 1) * 2) * (code[i] - '0');

	return accum % 10? -3: 0;
}

int CheckPINCode(char *PIN)
{
	int i, code_len = 0;

	code_len = strlen(PIN);

	if(code_len != 8 && code_len != 4)
		return -1;

	for( i = 0; i < code_len; i++ )
	{
		if( PIN[i] < '0' || PIN[i] > '9' )
			return -2;
	}

	if(code_len == 8)
		return ValidatePINCode(PIN);
	else
		return 0;
}

void InitDynamicMenu()
{
	int wanmode = WAN_MODE;
	int i = 0, j = 0;
	int wan_bitmap = WAN_MODE_MASK;
	int idx, count = 0;

	for(idx = 0; idx < 5; idx++)
	{
		if(1 & (wan_bitmap >> idx))
			count++;
	}

	/* WAN Menu */
	if(count > 1)
	{
		WANMenu[i] = strSetWANInterface[SET_WAN_ITF_WAN_MODE];
		wan_menu_idx[i] = i;
		i++;
	}

	if(wanmode & MODE_Ethernet)
	{
		WANMenu[i] = strSetWANInterface[SET_WAN_ITF_ETH_CONFIG];
		wan_menu_idx[i] = SET_WAN_ITF_ETH_CONFIG;
		i++;
	}

	if(wanmode & MODE_PTM)
	{
		WANMenu[i] = strSetWANInterface[SET_WAN_ITF_PTM_CONFIG];
		wan_menu_idx[i] = SET_WAN_ITF_PTM_CONFIG;
		i++;
	}

	if(wanmode & MODE_ATM || wanmode & MODE_PTM)
	{
		for( j = SET_WAN_ITF_DSL_CONFIG; j <= SET_WAN_ITF_ADSL_SETTINGS; j++ )
		{
			WANMenu[i] = strSetWANInterface[j];
			wan_menu_idx[i] = j;
			i++;
		}
	}

	for( j = SET_WAN_ITF_ADSL_SETTINGS + 1; j < SET_WAN_ITF_IDX_END; j++ )
	{
		WANMenu[i] = strSetWANInterface[j];
		wan_menu_idx[i] = j;
		i++;
	}

	WANMenu[i] = NULL;
	/* WAN Menu End*/

	/* Diagnostics Menu */
	i = 0;
	DiagnosticsMenu[i] = itemsDiagnosticsMenu[i];
	diag_menu_idx[i] = i;
	i++;

	if(wanmode & MODE_ATM)
	{
		for( j = MENU_DIAG_ATM_LOOPBACK; j <= MENU_DIAG_ADSL_CONNECTION; j++ )
		{
			DiagnosticsMenu[i] = itemsDiagnosticsMenu[j];
			diag_menu_idx[i] = j;
			i++;
		}
	}

	for( j = MENU_DIAG_ADSL_CONNECTION + 1; j < MENU_DIAG_END; j++ )
	{
		DiagnosticsMenu[i] = itemsDiagnosticsMenu[j];
		diag_menu_idx[i] = j;
		i++;
	}

	DiagnosticsMenu[i] = NULL;
	/* Diagnostics Menu End */

	/* Statistics Menu */
	for(i = 0, j = 0; j < MENU_STA_END; j++ )
	{
		if( j == MENU_STA_ADSL && !(wanmode & MODE_ATM) )
			continue;

		StatisticsMenu[i] = itemsStatisticsMenu[j];
		sta_menu_idx[i] = j;
		i++;
	}
	/* Statistics Menu End */
}

#ifdef CONFIG_PPP
int pppConnect(int idx, int isConnect)
{
	MIB_CE_ATM_VC_T Entry;
	char ifname[6];
	int havePPP=0;
	struct data_to_pass_st msg;
	char qosParms[32];

	if (!mib_chain_get(MIB_ATM_VC_TBL, idx, (void *)&Entry))
		return (-1);

	if (Entry.cmode != CHANNEL_MODE_PPPOE && Entry.cmode != CHANNEL_MODE_PPPOA) {
		printf("not allowed for this connection !\n");
		return (-1);
	}

	snprintf(ifname, 6, "ppp%u", PPP_INDEX(Entry.ifIndex));
	havePPP = find_ppp_from_conf(ifname);

	if (isConnect) {
		if (!havePPP) {
			// create a PPP connection
			printf("create %s here\n", ifname);
			if (Entry.cmode == CHANNEL_MODE_PPPOE)	// PPPoE
			{
				// spppctl add 0 pppoe vc0 username USER password PASSWORD gw 1
				snprintf(msg.data, BUF_SIZE, "spppctl add %u pppoe vc%u username %s password %s gw 1",
					PPP_INDEX(Entry.ifIndex), VC_INDEX(Entry.ifIndex), Entry.pppUsername, Entry.pppPassword);
			}
			else	// PPPoA
			{
				if ((ATM_QOS_T)Entry.qos == ATMQOS_UBR)
				{
					snprintf(qosParms, 32, "ubr:pcr=%u", Entry.pcr);
				}
				else if ((ATM_QOS_T)Entry.qos == ATMQOS_UBR)
				{
					snprintf(qosParms, 32, "cbr:pcr=%u", Entry.pcr);
				}
				else	// rt-vbr or nrt-vbr
				{
					snprintf(qosParms, 32, "vbr:pcr=%u,scr=%u,mbs=%u", Entry.pcr, Entry.scr, Entry.mbs);
				}

				snprintf(msg.data, BUF_SIZE, "spppctl add %u pppoa %u.%u encaps %d qos %s username %s password %s gw 1",
					PPP_INDEX(Entry.ifIndex), Entry.vpi, Entry.vci, Entry.encap, qosParms, Entry.pppUsername, Entry.pppPassword);
			}
			//TRACE(STA_SCRIPT, "%s\n", msg.data);
			write_to_pppd(&msg);
		}
		else
		{
			/* printf("%s already exists\n", ifname); */
			/* Up this interface */
			printf("Connecting %s\n", ifname);
			// spppctl up ppp
			snprintf(msg.data, BUF_SIZE, "spppctl up %u", PPP_INDEX(Entry.ifIndex));
			write_to_pppd(&msg);
		}

		//goto setOk_filter;
		}
		else {
			if (havePPP)
			{
				/* Disconnect ppp */
				printf("Disconnecting %s\n", ifname);
				// spppctl down ppp
				snprintf(msg.data, BUF_SIZE, "spppctl down %u", PPP_INDEX(Entry.ifIndex));
				write_to_pppd(&msg);
			}
			else
				printf("%s not exists\n", ifname);
		}
   return 0;
}
#endif

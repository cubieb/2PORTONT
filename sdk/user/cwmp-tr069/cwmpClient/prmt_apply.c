#include <crypt.h>
#include "prmt_igd.h"
#ifdef CONFIG_CWMP_TR181_SUPPORT
#include "tr181/prmt_ppp.h"
#include "tr181/prmt_ip_if.h"
#endif

#ifdef USE_LIBMD5
#include <libmd5wrapper.h>
#else
/*copy from boa: md5.c*/
typedef unsigned long uint32;
struct MD5Context {
	uint32 buf[4];
	uint32 bits[2];
	unsigned char in[64];
};

void MD5Transform(uint32 buf[4], uint32 const in[16]);

#ifdef EMBED
#define HIGHFIRST
#endif

#ifndef HIGHFIRST
#define byteReverse(buf, len)	/* Nothing */
#else
void byteReverse(unsigned char *buf, unsigned longs);

/*
 * Note: this code is harmless on little-endian machines.
 */
void byteReverse(unsigned char *buf, unsigned longs)
{
    uint32 t;
    do {
	t = (uint32) ((unsigned) buf[3] << 8 | buf[2]) << 16 |
	    ((unsigned) buf[1] << 8 | buf[0]);
	*(uint32 *) buf = t;
	buf += 4;
    } while (--longs);
}
#endif //HIGHFIRST

/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void MD5Init(struct MD5Context *ctx)
{
    ctx->buf[0] = 0x67452301;
    ctx->buf[1] = 0xefcdab89;
    ctx->buf[2] = 0x98badcfe;
    ctx->buf[3] = 0x10325476;

    ctx->bits[0] = 0;
    ctx->bits[1] = 0;
}

/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void MD5Update(struct MD5Context *ctx, unsigned char const *buf, unsigned len)
{
    uint32 t;

    /* Update bitcount */

    t = ctx->bits[0];
    if ((ctx->bits[0] = t + ((uint32) len << 3)) < t)
	ctx->bits[1]++;		/* Carry from low to high */
    ctx->bits[1] += len >> 29;

    t = (t >> 3) & 0x3f;	/* Bytes already in shsInfo->data */

    /* Handle any leading odd-sized chunks */

    if (t) {
	unsigned char *p = (unsigned char *) ctx->in + t;

	t = 64 - t;
	if (len < t) {
	    memcpy(p, buf, len);
	    return;
	}
	memcpy(p, buf, t);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (uint32 *) ctx->in);
	buf += t;
	len -= t;
    }
    /* Process data in 64-byte chunks */

    while (len >= 64) {
	memcpy(ctx->in, buf, 64);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (uint32 *) ctx->in);
	buf += 64;
	len -= 64;
    }

    /* Handle any remaining bytes of data. */

    memcpy(ctx->in, buf, len);
}

/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void MD5Final(unsigned char digest[16], struct MD5Context *ctx)
{
    unsigned count;
    unsigned char *p;

    /* Compute number of bytes mod 64 */
    count = (ctx->bits[0] >> 3) & 0x3F;

    /* Set the first char of padding to 0x80.  This is safe since there is
       always at least one byte free */
    p = ctx->in + count;
    *p++ = 0x80;

    /* Bytes of padding needed to make 64 bytes */
    count = 64 - 1 - count;

    /* Pad out to 56 mod 64 */
    if (count < 8) {
	/* Two lots of padding:  Pad the first block to 64 bytes */
	memset(p, 0, count);
	byteReverse(ctx->in, 16);
	MD5Transform(ctx->buf, (uint32 *) ctx->in);

	/* Now fill the next block with 56 bytes */
	memset(ctx->in, 0, 56);
    } else {
	/* Pad block to 56 bytes */
	memset(p, 0, count - 8);
    }
    byteReverse(ctx->in, 14);

    /* Append length in bits and transform */
    ((uint32 *) ctx->in)[14] = ctx->bits[0];
    ((uint32 *) ctx->in)[15] = ctx->bits[1];

    MD5Transform(ctx->buf, (uint32 *) ctx->in);
    byteReverse((unsigned char *) ctx->buf, 4);
    memcpy(digest, ctx->buf, 16);
    memset((char *) ctx, 0, sizeof(ctx));	/* In case it's sensitive */
}

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
	( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  MD5Update blocks
 * the data and converts bytes into longwords for this routine.
 */
void MD5Transform(uint32 buf[4], uint32 const in[16])
{
    register uint32 a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;
}
#endif //USE_LIBMD5

/*copy from boa => util.c:base64encode()*/
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

/*copy from boa.c:calPasswdMD5()*/
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

/*update the boa's username & password.
  copy from boa.c:writePasswdFile()*/
int writePasswdFile()
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
		fclose(fp);
		return;
	}

	fprintf(fp, "%s:%s\n", usName, usPasswdMD5);
	fprintf(fp, "%s:%s\n", suName, suPasswdMD5);
	fprintf(fp2, "%s:%s", suName, suPasswdMD5);

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

	return 0;
}

/*copy from startup.c:sys_setup()*/
/*it should be written in the utility.c,
  but this function need to link with -lcrypt.(crypt).
  increse the memory size about 300kbytes */
void updateUserAccount(void)
{
	FILE *fp;
	char userName[MAX_NAME_LEN], userPass[MAX_NAME_LEN];
	char *xpass;
#ifdef ACCOUNT_CONFIG
	MIB_CE_ACCOUNT_CONFIG_T entry;
	unsigned int totalEntry;
	int i;
#endif

	fp = fopen("/var/passwd", "w+");
#ifdef ACCOUNT_CONFIG
	totalEntry = mib_chain_total(MIB_ACCOUNT_CONFIG_TBL); /* get chain record size */
	for (i=0; i<totalEntry; i++) {
		if (!mib_chain_get(MIB_ACCOUNT_CONFIG_TBL, i, (void *)&entry)) {
			printf("ERROR: Get account configuration information from MIB database failed.\n");
			fclose(fp);
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

int apply_UserAccount( int action_type, int id, void *olddata )
{
	//for UserAccount, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_RESTART:
	case CWMP_START:
		updateUserAccount();
		writePasswdFile();
		break;
	case CWMP_STOP:
		break;
	default:
		return -1;
	}
	return 0;
}

#ifdef TIME_ZONE
//NTP
int apply_NTP( int action_type, int id, void *olddata )
{
	//for NTP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_RESTART:
		stopNTP();
	case CWMP_START:
		startNTP();
		break;
	case CWMP_STOP:
		stopNTP();
		break;
	default:
		return -1;
	}
	return 0;
}
#endif //TIME_ZONE


// DNS
int apply_DNS( int action_type, int id, void *olddata )
{
	//for RIP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
#if defined(CONFIG_USER_DNSMASQ_DNSMASQ) || defined(CONFIG_USER_DNSMASQ_DNSMASQ245)
		restart_dnsrelay();
#endif
		break;
	default:
		return -1;
	}
	return 0;
}

// DHCP
int apply_DHCP( int action_type, int id, void *olddata )
{
	//for RIP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
#ifdef CONFIG_USER_DHCP_SERVER
		restart_dhcp();
#endif
		break;
	default:
		return -1;
	}
	return 0;
}

#ifdef ROUTING
//ROUTE
int apply_Layer3Forwarding( int action_type, int id, void *olddata )
{
	MIB_CE_IP_ROUTE_T *pOldRoute=olddata;
	MIB_CE_IP_ROUTE_T RouteEntry, *pNewRoute=NULL;

	//got the lastest entry
	if( mib_chain_get( MIB_IP_ROUTE_TBL, id,  (void*)&RouteEntry )!=0 ) //0:error
		pNewRoute =  &RouteEntry;

	switch( action_type )
	{
	case CWMP_RESTART:
		if( pOldRoute &&
		    pOldRoute->Enable &&
		    !( pOldRoute->ifIndex==DUMMY_IFINDEX &&
		      pOldRoute->nextHop[0]==0 &&
		      pOldRoute->nextHop[1]==0 &&
		      pOldRoute->nextHop[2]==0 &&
		      pOldRoute->nextHop[3]==0 )
		  )
			route_cfg_modify( pOldRoute, 1, id);
	case CWMP_START:
		if( pNewRoute &&
		    pNewRoute->Enable &&
		    !( pNewRoute->ifIndex==DUMMY_IFINDEX &&
		      pNewRoute->nextHop[0]==0 &&
		      pNewRoute->nextHop[1]==0 &&
		      pNewRoute->nextHop[2]==0 &&
		      pNewRoute->nextHop[3]==0 )
		  )
			route_cfg_modify( pOldRoute, 1, id);
		break;
	case CWMP_STOP:
		if( pOldRoute &&
		    pOldRoute->Enable &&
		    !( pOldRoute->ifIndex==DUMMY_IFINDEX &&
		      pOldRoute->nextHop[0]==0 &&
		      pOldRoute->nextHop[1]==0 &&
		      pOldRoute->nextHop[2]==0 &&
		      pOldRoute->nextHop[3]==0 )
		  )
			route_cfg_modify( pOldRoute, 1, id);
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

//RIP
int apply_RIP( int action_type, int id, void *olddata )
{
	//for RIP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
#ifdef CONFIG_USER_ROUTED_ROUTED
		startRip();
#endif
		break;
	default:
		return -1;
	}
	return 0;
}

#ifdef NEW_PORTMAPPING
//PortMapping
int apply_PortMapping( int action_type, int id, void *olddata )
{

// set debug mode,jiunming test
//DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

	//for PortMapping, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_RESTART:
		//stop portmapping? but for tr069, once enable, always enable!
	case CWMP_START:
		setupnewEth2pvc();
		break;
	case CWMP_STOP:
		break;
	default:
		return -1;
	}
	return 0;
}
#endif //NEW_PORTMAPPING

int apply_DefaultRoute( int action_type, int id, void *olddata )
{
	//for DefaultRoute, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_RESTART:
		va_cmd(ROUTE, 2, 1, "delete", "default");
	case CWMP_START:
		{
#ifdef DEFAULT_GATEWAY_V2
			unsigned int dgw;
			if (mib_get(MIB_ADSL_WAN_DGW_ITF, (void *)&dgw) != 0)
			{
#ifdef AUTO_PPPOE_ROUTE
				if (dgw != DGW_AUTO)
#endif
				{
					char ifname[IFNAMSIZ] = "";
					if (ifGetName(dgw, ifname, sizeof(ifname)) != NULL)
					{
#ifdef DEFAULT_GATEWAY_V2
						if (ifExistedDGW() == 1)	// Jenny, delete existed default gateway first
							va_cmd(ROUTE, 2, 1, ARG_DEL, "default");
#endif
						va_cmd(ROUTE, 3, 1, ARG_ADD, "default", ifname);
					}
				}
			}
#else
			MIB_CE_ATM_VC_T *pEntry, vc_entity;
			int total,i;
			total = mib_chain_total(MIB_ATM_VC_TBL);
			for( i=0; i<total; i++ )
			{
				pEntry = &vc_entity;
				if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
					continue;

				if( (pEntry->dgw==1) && (pEntry->enable==1) )
				{
					char ifname[IFNAMSIZ]="";
					if( ifGetName(pEntry->ifIndex, ifname, sizeof(ifname))!=NULL )
						va_cmd(ROUTE, 3, 1, ARG_ADD, "default", ifname );
				}
			}
#endif
		}
		break;
	case CWMP_STOP:
		va_cmd(ROUTE, 2, 1, "delete", "default");
		break;
	default:
		return -1;
	}
	return 0;
}

#ifdef VIRTUAL_SERVER_SUPPORT
int apply_PortForwarding( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case ACT_RESTART:  //CWMP_RESTART:
	case ACT_START:    //CWMP_START:
	case ACT_STOP:     //CWMP_STOP:
		setupVtlsvr(VIRTUAL_SERVER_DELETE);
		setupVtlsvr(VIRTUAL_SERVER_ADD);
		break;
	default:
		return -1;
	}
	return 0;
}
#elif defined(PORT_FORWARD_GENERAL)
// Added by Mason Yu for take effect in real time.
int apply_PortForwarding( int action_type, int id, void *olddata )
{
	MIB_CE_PORT_FW_T *pOldPort=olddata;
	MIB_CE_PORT_FW_T PortEntry, *pNewPort=NULL;

	//got the lastest entry
	if( mib_chain_get( MIB_PORT_FW_TBL, id,  (void*)&PortEntry )!=0 ) //0:error
		pNewPort =  &PortEntry;

	switch( action_type )
	{
	case ACT_RESTART:  //CWMP_RESTART:
		if(pOldPort) portfw_modify( pOldPort, 1 );
	case ACT_START:    //CWMP_START:
		if(pNewPort) portfw_modify( pNewPort, 0 );
		break;
	case ACT_STOP:     //CWMP_STOP:
		if(pOldPort)
			portfw_modify( pOldPort, 1 );
		else if(pNewPort)
			portfw_modify( pNewPort, 1 );
		break;
	default:
		return -1;
	}
	return 0;
}
#endif


#ifdef WLAN_SUPPORT
//WIRELESS
int apply_WLAN( int action_type, int id, void *olddata )
{
	//for RIP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		#ifdef CONFIG_WIFI_SIMPLE_CONFIG // WPS
		update_wps_configured(0);
		#endif
		config_WLAN(ACT_RESTART);
		break;
	default:
		return -1;
	}
	return 0;
}
#endif	// of WLAN_SUPPORT

//Ethernet LAN
int apply_ETHER( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restart_ethernet(id);
		break;
	default:
		return -1;
	}
	return 0;

}

// LANIP
int apply_LANIP( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restart_lanip();
		break;
	default:
		return -1;
	}
	return 0;

}

#ifdef MAC_FILTER
int apply_MACFILTER( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		printf("\nRestart macfilter!\n");
		return setupMacFilter();
		break;
	default:
		return -1;
	}
	return 0;

}
#endif

//DDNS
#ifdef CONFIG_USER_DDNS
int apply_DDNS( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restart_ddns();
		break;
	default:
		return -1;
	}
	return 0;

}
#endif

#ifdef REMOTE_ACCESS_CTL
int apply_RemoteAccess( int action_type, int id, void *olddata )
{
	MIB_CE_ACC_T *pOldAcc=olddata;
	MIB_CE_ACC_T AccEntry, *pNewAcc=NULL;

	//got the lastest entry
	if (mib_chain_get(MIB_ACC_TBL, 0, (void *)&AccEntry)) //0:error
		pNewAcc = &AccEntry;

	//void remote_access_modify(  MIB_CE_ACC_T accEntry, int enable)
	switch( action_type )
	{
	case CWMP_RESTART:
		if(pOldAcc) remote_access_modify( *pOldAcc, 0 );
	case CWMP_START:
		if(pNewAcc) remote_access_modify( *pNewAcc, 1 );
		break;
	case CWMP_STOP:
		if(pOldAcc) remote_access_modify( *pOldAcc, 0 );
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

#if defined(CONFIG_USER_IGMPPROXY) && defined(CONFIG_IGMPPROXY_MULTIWAN)
int apply_IGMPProxy( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_RESTART:
	case CWMP_START:
	case CWMP_STOP:
		setting_Igmproxy();
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef _PRMT_X_CT_COM_IPTV_
int apply_IGMPSnooping( int action_type, int id, void *olddata )
{
	char vChar;
	//for RIP, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		mib_get( MIB_MPMODE, (void *)&vChar);
		if(vChar & MP_IGMP_MASK) // igmp snooping
			__dev_setupIGMPSnoop(1);
		else
			__dev_setupIGMPSnoop(0);
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef _PRMT_TR143_
int apply_UDPEchoConfig( int action_type, int id, void *olddata )
{
	struct TR143_UDPEchoConfig newdata;
	int ret=0;

	UDPEchoConfigSave(&newdata);
	switch( action_type )
	{
	case CWMP_RESTART:
		if(olddata) UDPEchoConfigStop(olddata);
	case CWMP_START:
		UDPEchoConfigStart(&newdata);
		break;
	case CWMP_STOP:
		if(olddata) UDPEchoConfigStop(olddata);
		break;
	default:
		ret=-1;
	}

	return ret;
}
#endif

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
int Apply_CaptivePortal(int action_type, int id, void *olddata)
{
	switch(action_type)
	{
		case CWMP_RESTART:
			stop_captiveportal();
			start_captiveportal();
			break;
		case CWMP_START:
			start_captiveportal();
			break;
		case CWMP_STOP:
			stop_captiveportal();
			break;
		default:
			return -1;
	}
	return 0;
}
#endif

#ifdef CONFIG_IP_NF_ALG_ONOFF
int apply_ALGONOFF( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		setupAlgOnOff();
		break;
	default:
		return -1;
	}
	return 0;

}
#endif

#if defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)
int apply_IPQoS( int action_type, int id, void *olddata )
{

// set debug mode,jiunming test
//DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

	//for IPQoS, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_RESTART:
#ifndef NEW_IP_QOS_SUPPORT
		stopIPQ();
#endif
	case CWMP_START:
		{
			unsigned char mode=0;
			mib_get(MIB_MPMODE, (void *)&mode);
			//ql 20081117 START modify qos function
#ifndef NEW_IP_QOS_SUPPORT
			if (mode&MP_IPQ_MASK) setupIPQ();
#else
			if (mode & MP_IPQ_MASK) take_qos_effect();
#endif
		}
		break;
	case CWMP_STOP:
#ifndef NEW_IP_QOS_SUPPORT
		stopIPQ();
#else
		stop_IPQoS();
#endif
		break;
	default:
		return -1;
	}
	return 0;
}
#endif //defined(IP_QOS) || defined(NEW_IP_QOS_SUPPORT)

#ifdef IP_QOS		/* For IPQoS v1*/
int apply_IPQoSRule( int action_type, int id, void *olddata )
{
	MIB_CE_IP_QOS_T *pOldQoS=olddata;

// set debug mode,jiunming test
//DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

	//for IPQoSRule,
	//id=-1 => take action for all rules,
	//id>-1 => modify the specific entry
	switch( action_type )
	{
	case CWMP_RESTART:
		if(id>-1)
			if(pOldQoS) _setupIPQoSRule(0, id, pOldQoS);
		setupUserIPQoSRule(0);
	case CWMP_START:
		{
			unsigned char mode=0;
			mib_get(MIB_MPMODE, (void *)&mode);
			if (mode&MP_IPQ_MASK) setupUserIPQoSRule(1);
		}
		break;
	case CWMP_STOP:
		if(id>-1)
		{
			if(pOldQoS) _setupIPQoSRule(0, id, pOldQoS);
		}
		else
			setupUserIPQoSRule(0);
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef NEW_IP_QOS_SUPPORT	/* For IPQoS v2*/
int apply_IPQoSRule( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_RESTART:
	case CWMP_START:
	case CWMP_STOP:
		take_qos_effect();
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef CONFIG_USER_IP_QOS_3
int apply_IPQoS( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_RESTART:
		stopIPQ();
	case CWMP_START:
		setupIPQ();
		break;
	case CWMP_STOP:
		stopIPQ();
		break;
	default:
		return -1;
	}
	return 0;
}

int apply_IPQoSRule( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_RESTART:
	case CWMP_START:
	case CWMP_STOP:
		take_qos_effect_v3();
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

#if defined(CONFIG_USER_UPNPD) || defined(CONFIG_USER_MINIUPNPD)
#ifdef _PRMT_X_CT_COM_UPNP_
int apply_UPnP(int action_type, int id, void *olddata)
{
	unsigned char is_enabled;
	unsigned int ext_if;
	char ifname[IFNAMSIZ];

	mib_get(MIB_UPNP_EXT_ITF, (void *)&ext_if);
	ifGetName(ext_if, ifname, sizeof(ifname));

	switch (action_type) {
	case CWMP_RESTART:
        va_cmd("/bin/upnpctrl", 3, 1, "down", ifname, "br0");  // stop miniupnpd
	case CWMP_START:
	    va_cmd("/bin/upnpctrl", 3, 1, "up", ifname, "br0");  // start miniupnpd
		break;
	case CWMP_STOP:
        va_cmd("/bin/upnpctrl", 3, 1, "down", ifname, "br0");  // stop miniupnpd
		break;
	default:
		return -1;
	}

	return 0;
}
#endif /* _PRMT_X_CT_COM_UPNP_ */
#endif /* defined(CONFIG_USER_UPNPD) || defined(CONFIG_USER_MINIUPNPD) */

#ifdef CONFIG_USER_MINIDLNA
int apply_DLNA( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
		startMiniDLNA();
		break;
	case CWMP_RESTART:
		stopMiniDLNA();
		startMiniDLNA();
		break;
	case CWMP_STOP:
		stopMiniDLNA();
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef CONFIG_NO_REDIAL
int apply_Reconnect( int action_type, int id, void *olddata )
{
// set debug mode,jiunming test
//DEBUGMODE(STA_INFO|STA_SCRIPT|STA_WARNING|STA_ERR);

	//for IPQoS, id and olddata have no meaning
	switch( action_type )
	{
	case CWMP_RESTART:
	case CWMP_START:
	case CWMP_STOP:
		startReconnect();
		break;
	default:
		return -1;
	}
}
#endif

#if defined(CONFIG_USER_RADVD) && defined(CONFIG_E8B)
int apply_radvd( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restartRadvd();
	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef CONFIG_USER_TR104
int restart_voip(){

	int h_pipe, res=0;
    static char FIFO_SOLAR[] = "/var/run/solar_control.fifo";


    h_pipe = open(FIFO_SOLAR, O_WRONLY | O_NONBLOCK);
    if (h_pipe == -1)
    {
    	fprintf(stderr, "open %s failed\n", FIFO_SOLAR);
        return -1;
    }
#if 1 /* Added by Alex, 20111027, if support fifo 'X' commands*/
    res = write(h_pipe, "X\n", 2);          // restart solar
#else
    res = write(h_pipe, "x\n", 2);          // restart solar
#endif
    if (res == -1) {
        fprintf(stderr, "write %s failed\n", FIFO_SOLAR);
        close(h_pipe);
        return -1;
    }

    close(h_pipe);
}

int apply_voip( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
    case CWMP_STOP:
        //Do Nothing
        break;
	case CWMP_RESTART:
		restart_voip();
		break;
	default:
		return -1;
	}
	return 0;

}
#endif /* CONFIG_USER_TR104*/


#ifdef CONFIG_CWMP_TR181_SUPPORT
//Apply setting and set applied parameter values to NULL.
int lower_layers_changed(int action_type, int id, void *olddata)
{
	struct CWMP_LINKNODE *ip_if_node = gIPIfEntityObjList->sibling;	//skip LAN
	struct CWMP_LINKNODE *ppp_if_node = gPPPIfEntityObjList;

	while(ip_if_node)
	{
		struct IPIfEntityData *ip_if_data = ip_if_node->obj_data;
		struct PPPIfObjData *ppp_if_data = ppp_if_node->obj_data;
		char ifname[IFNAMSIZ] = {0};
		MIB_CE_ATM_VC_T vc_entity = {0};
		int chainidx = -1;

		unsigned int new_ifindex;
		unsigned int old_ifindex;

		if(get_ip_if_info(ip_if_node->instnum, &vc_entity, &chainidx, ifname) < 0)
			goto next;

		old_ifindex = vc_entity.ifIndex;

		if(ip_if_data->LowerLayers)
		{
			unsigned char changed = 0;

			if(strcmp(ip_if_data->LowerLayers, "") == 0)
			{
				//TODO: 6rd & DSLite
				if((vc_entity.cmode==CHANNEL_MODE_BRIDGE))
					goto next;

				new_ifindex = getNewIfIndex(CHANNEL_MODE_BRIDGE, vc_entity.ConDevInstNum, MEDIA_INDEX(vc_entity.ifIndex), chainidx);
				vc_entity.ifIndex = new_ifindex;
				vc_entity.cmode = CHANNEL_MODE_BRIDGE;
				vc_entity.brmode=BRIDGE_ETHERNET;

				CWMPDBG(1, (stderr, "Change to Bridge, old_ifIndex=%08x, new_ifIndex=%08x, real_ifIndex=%08x\n", old_ifindex, new_ifindex, vc_entity.ifIndex));
				changed = 1;
			}
			else if ( ip_if_node->instnum == getInstNum(ip_if_data->LowerLayers, "PPP.Interface"))
			{
				if(ppp_if_data->LowerLayers)
				{
					if ( ppp_if_node->instnum == getInstNum(ppp_if_data->LowerLayers, "Ethernet.Link"))
					{
						if((vc_entity.cmode==CHANNEL_MODE_PPPOE))
							goto next;

						new_ifindex = getNewIfIndex(CHANNEL_MODE_PPPOE, vc_entity.ConDevInstNum, MEDIA_INDEX(vc_entity.ifIndex), chainidx);
						if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) )
							return -1;
						vc_entity.cmode = CHANNEL_MODE_PPPOE;
						vc_entity.ifIndex = TO_IFINDEX(MEDIA_INDEX(vc_entity.ifIndex), PPP_INDEX(new_ifindex), VC_INDEX(vc_entity.ifIndex));
						vc_entity.mtu = 1492;
						vc_entity.brmode=BRIDGE_DISABLE;

						CWMPDBG(1, (stderr, "Change to PPPoE, old_ifIndex=%08x, new_ifIndex=%08x, real_ifIndex=%08x\n", old_ifindex, new_ifindex, vc_entity.ifIndex));
						changed = 1;

						free(ppp_if_data->LowerLayers);
						ppp_if_data->LowerLayers = NULL;
					}
					else if ( ppp_if_node->instnum == getInstNum(ppp_if_data->LowerLayers, "ATM.Link"))
					{
						int old_ifindex = vc_entity.ifIndex;

						if(MEDIA_INDEX(vc_entity.ifIndex) != MEDIA_ATM)
						{
							CWMPDBG(1, (stderr, "Only DSL connection can change to PPPoA mode.\n", old_ifindex, new_ifindex, vc_entity.ifIndex));
							goto next;
						}

						if((vc_entity.cmode==CHANNEL_MODE_PPPOA))
						    goto next;

						new_ifindex = getNewIfIndex(CHANNEL_MODE_PPPOA, vc_entity.ConDevInstNum, MEDIA_INDEX(vc_entity.ifIndex), chainidx);
						if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) )
							return -1;
						vc_entity.cmode = CHANNEL_MODE_PPPOA;
						vc_entity.ifIndex = TO_IFINDEX(MEDIA_ATM, PPP_INDEX(new_ifindex), VC_INDEX(vc_entity.ifIndex));
						vc_entity.brmode=BRIDGE_DISABLE;

						CWMPDBG(1, (stderr, "Change to PPPoA, old_ifIndex=%08x, new_ifIndex=%08x, real_ifIndex=%08x\n", old_ifindex, new_ifindex, vc_entity.ifIndex));
						changed = 1;

						free(ppp_if_data->LowerLayers);
						ppp_if_data->LowerLayers = NULL;
					}
					else
					{
						CWMPDBG(1, (stderr, "<%s:%d> Wrong PPP.Interface.%d.LowerLayer value\n", __FUNCTION__, __LINE__, ppp_if_node->instnum));
						return -1;
					}
				}
			}
			else if ( ip_if_node->instnum == getInstNum(ip_if_data->LowerLayers, "Ethernet.Link"))
			{
				if(vc_entity.cmode==CHANNEL_MODE_IPOE)
					goto next;

				new_ifindex = getNewIfIndex(CHANNEL_MODE_IPOE, vc_entity.ConDevInstNum, MEDIA_INDEX(vc_entity.ifIndex), chainidx);
				vc_entity.ifIndex = new_ifindex;
				vc_entity.cmode = CHANNEL_MODE_IPOE;
				vc_entity.brmode=BRIDGE_DISABLE;
				vc_entity.napt=1;

				CWMPDBG(1, (stderr, "Change to IPoE, old_ifIndex=%08x, new_ifIndex=%08x, real_ifIndex=%08x\n", old_ifindex, new_ifindex, vc_entity.ifIndex));
				changed = 1;
			}
			else if ( ip_if_node->instnum - 1 == getInstNum(ip_if_data->LowerLayers, "ATM.Link"))
			{
				if(vc_entity.cmode == CHANNEL_MODE_RT1483)
					goto next;

				new_ifindex = getNewIfIndex( CHANNEL_MODE_RT1483, vc_entity.ConDevInstNum, MEDIA_INDEX(vc_entity.ifIndex), chainidx);
				if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) )
					return -1;

				vc_entity.cmode = CHANNEL_MODE_RT1483;
				vc_entity.ifIndex = new_ifindex;
				vc_entity.mtu = 1500;

				CWMPDBG(1, (stderr, "Change to IPoA, old_ifIndex=%08x, new_ifIndex=%08x, real_ifIndex=%08x\n", old_ifindex, new_ifindex, vc_entity.ifIndex));
				changed = 1;
			}
			else
			{
				CWMPDBG(1, (stderr, "<%s:%d> Wrong IP.Interface.%d.LowerLayer value\n", __FUNCTION__, __LINE__, ip_if_node->instnum));
				free(ip_if_data->LowerLayers);
				ip_if_data->LowerLayers = NULL;
				return -1;
			}

			if(changed)
			{
				free(ip_if_data->LowerLayers);
				ip_if_data->LowerLayers = NULL;
				mib_chain_update(MIB_ATM_VC_TBL, &vc_entity, chainidx);
			}
			else
			{
				return -1;	// no changes, return error to skip reboot
			}
		}

next:
		ip_if_node = ip_if_node->sibling;
		ppp_if_node = ppp_if_node->sibling;
	}
	return 0;
}
#endif

#ifdef _PRMT_X_CT_COM_PORTALMNT_
int apply_PortalMNT(int action_type, int id, void *olddata)
{
	switch (action_type) {
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		setPortalMNT();
		break;
	default:
		return -1;
	}

	return 0;
}
#endif /* _PRMT_X_CT_COM_PORTALMNT_ */


#ifdef _PRMT_X_CT_COM_DHCP_
int apply_DHCPv6S(int action_type, int id, void *olddata)
{
	switch (action_type) {
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restartDHCPV6Server();
	default:
		return -1;
	}

	return 0;
}
#endif /* _PRMT_X_CT_COM_PORTALMNT_ */

#ifdef _PRMT_X_CT_COM_SYSLOG_
int apply_Syslog( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
#ifdef CONFIG_USER_RTK_SYSLOG
		stopLog();
		startLog();
#endif
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef _PRMT_X_CT_COM_IPv6_
int apply_ip6_lan_addr( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		{
			char *old_addr = (char *)olddata;
			char cmdBuf[64];
			char ip6_str[40]= {0};

			sprintf(cmdBuf, "%s/%d", old_addr, 64);
			va_cmd(IFCONFIG, 3, 1, LANIF, ARG_DEL, cmdBuf);

			mib_get(MIB_IPV6_LAN_IP_ADDR, ip6_str);
			sprintf(cmdBuf, "%s/%d", ip6_str, 64);
			va_cmd(IFCONFIG, 3, 1, LANIF, ARG_ADD, cmdBuf);
		}
		break;
	default:
		return -1;
	}
	return 0;
}

int apply_lanv6_server( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		restartLanV6Server();
	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef _PRMT_X_CT_COM_LBD_
int apply_lbd( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
		setupLBD();
		break;
	default:
		return -1;
	}
	return 0;
}
#endif

#ifdef CONFIG_MCAST_VLAN
int apply_mcast_vlan( int action_type, int id, void *olddata )
{
	switch( action_type )
	{
	case CWMP_START:
	case CWMP_RESTART:
	case CWMP_STOP:
#ifdef CONFIG_RTK_RG_INIT
		RTK_RG_ACL_Flush_mVlan();
		RTK_RG_ACL_Add_mVlan();
#endif
		break;
	default:
		return -1;
	}
	return 0;
}
#endif


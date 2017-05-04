/*
 *  Boa, an http server
 *  Copyright (C) 1995 Paul Phillips <psp@well.com>
 *  Authorization "module" (c) 1998,99 Martin Hinner <martin@tdp.cz>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 1, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

extern char *mgmtUserName(void);
extern char *mgmtPassword(void);

#include <stdio.h>
#include <fcntl.h>
#ifdef __UC_LIBC__
#include <unistd.h>
#else
#include <crypt.h>
#endif
#include "syslog.h"
#ifdef USE_LIBMD5
#include <libmd5wrapper.h>
#else
#include "md5.h"
#endif //USE_LIBMD5
#include "boa.h"
#ifdef SHADOW_AUTH
#include <shadow.h>
#endif
#ifdef OLD_CONFIG_PASSWORDS
#include <crypt_old.h>
#endif
#ifdef EMBED
#include <sys/types.h>
#include <pwd.h>
#include <config/autoconf.h>
#else
#include "../../../config/autoconf.h"
#endif

#ifdef SECURITY_COUNTS
#include "../../login/logcnt.c"
#endif

#ifdef EMBED
// Added by Mason Yu for 2 level web page
#include "./LINUX/mib.h"

// Added by Mason Yu
//extern char usName[MAX_NAME_LEN];
#endif

// Added by davian_kuo.
#include "./LINUX/form_src/multilang.h"

//add by xl_yue
#ifdef ACCOUNT_LOGIN_CONTROL
/*
extern char suName[MAX_NAME_LEN];
extern struct account_info su_account;
extern struct account_info us_account;
extern time_t time_counter;
extern struct errlogin_entry * errlogin_list;
*/
#define MAX_LOG_NUM 3
#endif

#ifdef USE_AUTH

#define DBG_DIGEST 0

struct _auth_dir_ {
	char *directory;
	FILE *authfile;
	int dir_len;
#if SUPPORT_AUTH_DIGEST
	int authtype; // 0:basic, 1:digest
	char *realm;
#endif
	struct _auth_dir_ *next;
};

typedef struct _auth_dir_ auth_dir;

static auth_dir *auth_hashtable [AUTH_HASHTABLE_SIZE];
#ifdef OLD_CONFIG_PASSWORDS
char auth_old_password[16];
#endif


// simple function to strip leading space and ending space/LF/CR
// Magician (2007.12.27): Modify to trim all white spaces, and solve some possible secure problems.
char *trim(char *input)
{
	char *tmp, *ret;
	int i, len;

	tmp = input;

	len = strlen(input);
	for( i = 0; i < len; i++ )  // Trim leading spaces.
	{
		if(isspace(*tmp))
			tmp++;
		else
			break;
	}

	len = strlen(tmp);
	for( i = len - 1; i >= 0; i-- )  // Trim trailing spaces.
	{
		if( isspace(tmp[i]) )
			tmp[i] = '\0';
		else
			break;
	}

	return tmp;
}

int istrimed(char chr, char *trimchr)
{
	int i, len;

	if(isspace(chr))
		return 1;

	len = strlen(trimchr);
	for( i = 0; i < len; i++ )
		if( trimchr[i] == chr )
			return 1;

	return 0;
}

//Magician 2007/12/27: Extended for trim function.
char *trimEx(char *input, char *tmchr)
{
	char *tmp, *ret;
	int i, len;

	tmp = input;

	len = strlen(input);
	for( i = 0; i < len; i++ )  // Trim leading spaces.
	{
		if(istrimed(*tmp, tmchr))
			tmp++;
		else
			break;
	}

	len = strlen(tmp);
	for( i = len - 1; i >= 0; i-- )  // Trim trailing spaces.
	{
		if( istrimed(tmp[i], tmchr) )
			tmp[i] = '\0';
		else
			break;
	}

	return tmp;
}

#if SUPPORT_AUTH_DIGEST

//static struct http_session *digest_session, digest_session0;
static struct http_session session_array[HTTP_SESSION_MAX]; // support 2 session only

static struct http_session * http_session_get() {
	int i;
	for (i=0; i< HTTP_SESSION_MAX; i++) {
		if (session_array[i].in_use)
			continue;

		session_array[i].in_use = 1;
		return &session_array[i];
	}
	return 0;
}

static void http_session_free(struct http_session *s) {
	s->in_use = 0;
}

#define soap_random rand()
static void http_da_calc_nonce(char nonce[HTTP_DA_NONCELEN])
{
  static short count = 0xCA53;
  sprintf(nonce, "%8.8x%4.4hx%8.8x", (int)time(NULL), count++, soap_random);
}

static void http_da_calc_opaque(char opaque[HTTP_DA_OPAQUELEN])
{
  sprintf(opaque, "%8.8x", soap_random);
}

static void s2hex(const unsigned char *src, char *dst, int len) {
	int i;
	for (i=0; i < len; i++) {
		sprintf(dst, "%02x", src[i]);
		dst += 2;
	}
	*dst = 0;
}

static void http_da_calc_HA1(struct MD5Context *pCtx, char *alg, char *userid, char *realm, char *passwd, char *nonce, char *cnonce, char HA1hex[33])
{

	char HA1[16];

	MD5Init(pCtx);
	MD5Update(pCtx, userid, strlen(userid));
	MD5Update(pCtx, ":", 1);
	MD5Update(pCtx, realm, strlen(realm));
 	MD5Update(pCtx, ":", 1);
	MD5Update(pCtx, passwd, strlen(passwd));
 	MD5Final(HA1, pCtx);

	if (alg && strncasecmp(alg, "MD5-session", 11)) {
		#if DBG_DIGEST
		fprintf(stderr, "alg = %s\n", alg);
		#endif
		MD5Init(pCtx);
		MD5Update(pCtx, HA1, 16);
		MD5Update(pCtx, ":", 1);
		MD5Update(pCtx, nonce, strlen(nonce));
 		MD5Update(pCtx, ":", 1);
		MD5Update(pCtx, cnonce, strlen(cnonce));
 		MD5Final(HA1, pCtx);
	}

	s2hex(HA1, HA1hex, 16);
	#if DBG_DIGEST
	fprintf(stderr, "HA1: MD5(%s:%s:%s)=%s\n", userid, realm, passwd, HA1hex);
	#endif
};

static void http_da_calc_response(struct MD5Context *pCtx, char HA1hex[33], char *nonce, char *ncount, char *cnonce, char *qop, char *method, char *uri, char entityHAhex[33], char response[33])
{
	char HA2[16], HA2hex[33], responseHA[16];

	MD5Init(pCtx);
	MD5Update(pCtx, method, strlen(method));
	MD5Update(pCtx, ":", 1);
	MD5Update(pCtx, uri, strlen(uri));
	if (!strncasecmp(qop, "auth-int", 8))
	{
		MD5Update(pCtx, ":", 1);
		MD5Update(pCtx, entityHAhex, 32);
	}
 	MD5Final(HA2, pCtx);
	s2hex(HA2, HA2hex, 16);

	MD5Init(pCtx);
	MD5Update(pCtx, HA1hex, 32);
	MD5Update(pCtx, ":", 1);
	MD5Update(pCtx, nonce, strlen(nonce));
	MD5Update(pCtx, ":", 1);

  	if (qop && *qop)
  	{
  		MD5Update(pCtx, ncount, strlen(ncount));
		MD5Update(pCtx, ":", 1);
		MD5Update(pCtx, cnonce, strlen(cnonce));
		MD5Update(pCtx, ":", 1);
		MD5Update(pCtx, qop, strlen(qop));
		MD5Update(pCtx, ":", 1);
    	}
	MD5Update(pCtx, HA2hex, 32);
	MD5Final(responseHA, pCtx);



  	s2hex(responseHA, response, 16);
	#if DBG_DIGEST
	fprintf(stderr, "HA2: MD5(%s:%s)=%s\n", method, uri, HA2hex);
	fprintf(stderr, "Response: MD5(%s:%s:%s:%s:%s:%s)=%s\n", HA1hex, nonce, ncount, cnonce, qop, HA2hex, response);
	#endif
}



static void http_da_session_cleanup()
{
	struct http_session *s;
	time_t now = time(NULL);

	//MUTEX_LOCK(http_da_session_lock);
	int i;
	for (i = 0; i < HTTP_SESSION_MAX; i++) {
		s = &session_array[i];

		if (!s->in_use)
			continue;

		// not expired yet.
		if (s->modified + HTTP_DA_EXPIRY_TIME > now)
			continue;

		http_session_free(s);
	}
  //MUTEX_UNLOCK(http_da_session_lock);
}

static struct http_session * http_da_session_start(const char *realm, const char *nonce, const char *opaque)
{

	struct http_session *session;
	time_t now = time(NULL);
	static int count = 0;

	if((count++ % 10) == 0) /* don't do this all the time to improve efficiency */
 		http_da_session_cleanup();

  	//MUTEX_LOCK(http_da_session_lock);

  	session = http_session_get();
	if (session)
  {
  	//session->next = http_da_session;
		session->modified = now;
		if (nonce)
			strncpy(session->nonce, nonce, sizeof(session->nonce));
		else
			http_da_calc_nonce(session->nonce);

		if (opaque)
			strncpy(session->opaque, opaque, sizeof(session->opaque));
		else
			http_da_calc_opaque(session->opaque);

		strncpy(session->realm,realm, sizeof(session->realm));
    		session->ncount = 0;
    		//http_da_session = session;
  	}

	return session;
  	//MUTEX_UNLOCK(http_da_session_lock);
}

static struct http_session * http_da_session_update(const char *realm, const char *nonce, const char *opaque, const char *cnonce, const char *ncount)
{
	int i;
	struct http_session *s;
#if VERIFY_OPAQUE
  	if (!realm || !nonce || !opaque || !cnonce || !ncount)
    		return 0;
#else
  	if (!realm || !nonce || !cnonce || !ncount)
    		return 0;
#endif
  //MUTEX_LOCK(http_da_session_lock);
	for (i = 0; i < HTTP_SESSION_MAX; i++)
	{
		s = &session_array[i];
    if (!s->in_use)
      continue;

		#if DBG_DIGEST
    	fprintf(stderr, "session nonce=%s; client resend nonce=%s\n", s->nonce, nonce);
		#endif

		#if VERIFY_OPAQUE
   		if (!strcmp(s->realm, realm) && !strcmp(s->nonce, nonce) && !strcmp(s->opaque, opaque))
   			break;
 		#else
   		if (!strcmp(s->realm, realm) && !strcmp(s->nonce, nonce))
   			break;
 		#endif
	}

	#if DBG_DIGEST
		fprintf(stderr, "session ncount=%d; client ncount=%d\n", s->ncount, strtoul(ncount, NULL, 16));
	#endif

	if (i < HTTP_SESSION_MAX)
	{
		unsigned long nc = strtoul(ncount, NULL, 16);

// Magician 01/14/2008
/* For non-sequential sending by FireFox, ignore ncount number comparison.
		if (s->ncount >= nc)
		{
			s->modified = 0;
			http_session_free(s);
		}
		else
		{
*/
			s->ncount = nc;
			s->modified = time(NULL);
//		}
	}
	else
		return 0;

	return s;
}

static void _auth_find_value(const char *input, const char * const token, char **ppVal, int *pSize)
{
	char del1[] = ",", del2[] = "=", *tmp, *tok, *value, *toks[11];
	char tmchr[] = "\"';=-()[]*&^%$#@!~`{}[]?,.+";
	int i = 0;

	tmp = strdup(strstr(input, "username"));

	for( tok = strtok(tmp, del1); tok != NULL && i < 10; tok = strtok(NULL, del1), i++ )
		toks[i] = tok;

	toks[i] = NULL;

	for( i = 0; i < 11; i++ )
	{
		if( !toks[i] )
			break;

		if((tok = strtok(toks[i], del2)) && (value = strtok(NULL, del2)) && !strcmp(trim(tok), token))
		{
			value = trimEx(value, tmchr);
			*ppVal = value;
			*pSize = strlen(value);
			break;
		}
	}

	free(tmp);
}


// return length of the value. -1 if token is not in input.
static int auth_find_value(const char *input, const char * const token, char *buf, int len) {
	char *pc;
	int tmp;
	memset(buf, 0, len);
	_auth_find_value(input, token, &pc, &tmp);
	if (pc) {
		strncpy(buf, pc, tmp);
		return tmp;
	}
	return -1;
}


#endif

/*
 * Name: get_auth_hash_value
 *
 * Description: adds the ASCII values of the file letters
 * and mods by the hashtable size to get the hash value
 */
inline int get_auth_hash_value(char *dir)
{
#ifdef EMBED
	return 0;
#else
	unsigned int hash = 0;
	unsigned int index = 0;
	unsigned char c;

	hash = dir[index++];
	while ((c = dir[index++]) && c != '/')
		hash += (unsigned int) c;

	return hash % AUTH_HASHTABLE_SIZE;
#endif
}

/*
 * Name: auth_add
 *
 * Description: adds
 */
void * auth_add(char *directory, char *file)
{
	auth_dir *new_a, *old;

	old = auth_hashtable[get_auth_hash_value(directory)];
	while (old)
	{
		if (!strcmp(directory, old->directory))
			return 0;
		old = old->next;
	}

	new_a = (auth_dir *)malloc(sizeof(auth_dir));
	/* success of this call will be checked later... */
	new_a->authfile = fopen(file, "rt");
	new_a->directory = strdup(directory);
	new_a->dir_len = strlen(directory);
	new_a->next = auth_hashtable [get_auth_hash_value(directory)];
	auth_hashtable [get_auth_hash_value(directory)] = new_a;

	return (void *)new_a;
}

#if SUPPORT_AUTH_DIGEST
void auth_add_digest(char *directory, char *file, char *realm) {

	auth_dir *dir;
	dir = (auth_dir *)auth_add(directory,file);
	if (dir) {
		dir->authtype = HTTP_AUTH_DIGEST;
		dir->realm = strdup(realm);

		fprintf(stderr, "Added Digest: %s, %s, %s\n", directory, file, realm);
	}
}
#endif

void auth_check()
{
	int hash;
	auth_dir *cur;

	for (hash=0;hash<AUTH_HASHTABLE_SIZE;hash++)
	{
  	cur = auth_hashtable [hash];
	  while (cur)
		{
			if (!cur->authfile)
			{
				log_error_time();
				fprintf(stderr,"Authentication password file for %s not found!\n",
						cur->directory);
			}
			cur = cur->next;
		}
	}
}

#ifdef LDAP_HACK
#include <lber.h>
#include <ldap.h>
#include <sg_configdd.h>
#include <sg_confighelper.h>
#include <sg_users.h>

/* Return a positive, negative or not possible result to the LDAP auth for
 * the specified user.
 */
static int ldap_auth(const char *const user, const char *const pswd) {
	static time_t last;
	static char *prev_user;
	LDAP *ld;
	int ldap_ver, r;
	char f[256];
	ConfigHandleType *c = config_load(amazon_ldap_config_dd);

	/* Don't repeat query too often if the user name hasn't changed */
	if (last && prev_user &&
			time(NULL) < (last + config_get_int(c, AMAZON_LDAP_CACHET)) &&
			strcmp(prev_user, user) == 0) {
		config_free(c);
		last = time(NULL);
		return 1;
	}
	if (prev_user != NULL)   { free(prev_user);	prev_user = NULL;   }
	last = 0;

	if ((ld = ldap_init(config_get_string(c, AMAZON_LDAP_HOST),
			config_get_int(c, AMAZON_LDAP_PORT))) == NULL) {
		syslog(LOG_ERR, "unable to initialise LDAP");
		config_free(c);
		return 0;
	}
	if (ldap_set_option(ld, LDAP_OPT_REFERRALS, LDAP_OPT_OFF) != LDAP_SUCCESS) {
		syslog(LOG_ERR, "unable to set LDAP referrals off");
		config_free(c);
		ldap_unbind(ld);
		return 0;
	}
	ldap_ver = config_get_int(c, AMAZON_LDAP_VERSION);
	if (ldap_ver > 0 && ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldap_ver) != LDAP_SUCCESS) {
		syslog(LOG_ERR, "unable to set LDAP version %d", ldap_ver);
		config_free(c);
		ldap_unbind(ld);
		return 0;
	}
	snprintf(f, sizeof f, config_get_string(c, AMAZON_LDAP_BIND_DN), user);
	r = ldap_simple_bind_s(ld, f, pswd);
	if (r != LDAP_SUCCESS) {
		syslog(LOG_ERR, "unable to connect to LDAP (%s)", ldap_err2string(r));
		config_free(c);
		ldap_unbind(ld);
		return (r==LDAP_INVALID_CREDENTIALS)?-1:0;
	}
	config_free(c);
	ldap_unbind(ld);
	/* Caching timing details for next time through */
	prev_user = strdup(user);
	last = time(NULL);
	return 1;
}
static unsigned char ldap_succ;
#endif



/*
 * Name: auth_check_userpass
 *
 * Description: Checks user's password. Returns 0 when sucessful and password
 * 	is ok, else returns nonzero; As one-way function is used RSA's MD5 w/
 *  BASE64 encoding.
#ifdef EMBED
 * On embedded environments we use crypt(), instead of MD5.
#endif
 */
int auth_check_userpass(char *user, char *pass, FILE *authfile, request * req)
{
#ifdef LDAP_HACK

	/* Yeah, code before declarations will fail on older compilers... */
	switch (ldap_auth(user, pass)) {
	case -1:	ldap_succ = 0;				return 1;
	case 0:		ldap_succ = strcmp(user, "root")?0:1;	break;
	case 1:
		ldap_succ = 1;
		if (start_user_update(0) == 0)
			done_user_update(set_user_password(user, pass, 0)==0?1:0);
		return 0;
	}
#endif
#ifdef SHADOW_AUTH
	struct spwd *sp;

	sp = getspnam(user);
	if (!sp)
		return 2;

	if (!strcmp(crypt(pass, sp->sp_pwdp), sp->sp_pwdp))
		return 0;

#else

//#ifndef EMBED
#if 1

#ifdef EMBED
	char temps[0x100],*pwd;
	struct MD5Context mc;
 	unsigned char final[16];
	char encoded_passwd[0x40];
	int userflag = 0;
	// Added by Mason Yu (2 leval)
	FILE *fp;
	char disabled;

   	/* Encode password ('pass') using one-way function and then use base64
	encoding. */

	// Added by Mason Yu(2 level)
	//if ( strcmp("user", user)==0 && strcmp(req->request_uri, "/")==0 ) {

	// Added by Mason Yu for boa memory leak
	// User access web every time, the boa will auth once. And use strdup to allocate memory for directory_index.
	// So We should free the memory space of older directory_index to avoid memory leak.
#ifndef MULTI_USER_PRIV
	if (directory_index) free(directory_index);

#ifndef ACCOUNT_CONFIG
	if (strcmp(usName, user)==0)
#else
	if (getAccPriv(user) == (int)PRIV_USER)
#endif
	{
#ifdef CONFIG_VDSL
		directory_index = strdup("index_user_vdsl.html");
#else
		directory_index = strdup("index_user.html");
#endif

		if (strcmp(req->request_uri, "/")==0)
		{
			fp = fopen("/var/boaUser.passwd", "r");
			// Jenny
			//authfile = fp;
			userflag = 1;
		}
	}

	else if (strcmp("adsl", user) == 0){
		char Passwd[50];

		if ( mib_get(MIB_SUPER_PASSWORD, (void *)Passwd) == 0) {
  			printf("Get Engineer Password error!\n");
  			return 1;
		}
#ifdef CONFIG_VDSL
		directory_index = strdup("index_vdsl.html");
#else
		directory_index = strdup("index.html");
#endif
		if (strcmp(Passwd, pass) == 0)
			return 0;
		else
			return 1;
	}
	else
#ifdef CONFIG_VDSL
		directory_index = strdup("index_vdsl.html");
#else
		directory_index = strdup("index.html");
#endif

#else
	if (strcmp("adsl", user) == 0) {
		char Passwd[50];

		if (mib_get(MIB_SUPER_PASSWORD, (void *)Passwd) == 0) {
			printf("Get Engineer Password error!\n");
			return 1;
		}

		if (strcmp(Passwd, pass) == 0)
			return 0;
		else
			return 1;
	}
#endif //#ifndef MULTI_USER_PRIV

	MD5Init(&mc);
	{
	//char *pass="admin";
	MD5Update(&mc, pass, strlen(pass));
	}
	MD5Final(final, &mc);
	strcpy(encoded_passwd,"$1$");
	base64encode(final, encoded_passwd+3, 16);

	DBG(printf("auth_check_userpass(%s,%s,...);\n",user,pass);)

	if (userflag) {
		fseek(fp, 0, SEEK_SET);
		while (fgets(temps, 0x100, fp)) {
			if (temps[strlen(temps)-1]=='\n')
				temps[strlen(temps)-1] = 0;
			pwd = strchr(temps,':');
			if (pwd) {
				*pwd++=0;
				if (!strcmp(temps,user)) {
					if (!strcmp(pwd,encoded_passwd)) {
						fclose(fp);
						userflag = 0;
						return 0;
					}
				} else
					continue;
			}
		}
	}
	else {
	fseek(authfile, 0, SEEK_SET);
	while (fgets(temps,0x100,authfile))
	{
		if (temps[strlen(temps)-1]=='\n')
			temps[strlen(temps)-1] = 0;
		pwd = strchr(temps,':');
		if (pwd)
		{
			*pwd++=0;
			if (!strcmp(temps,user))
			{
				if (!strcmp(pwd,encoded_passwd)) {
					/*
					// Added by Mason Yu for web page via serverhost
					if ((strcmp(req->request_uri, "/")==0) && (!strcmp(usName, user)
					#ifdef ACCOUNT_CONFIG
						|| getAccPriv(user) == (int)PRIV_USER
					#endif
					))
						fclose(fp);
					*/
					return 0;
				}
			} else {
				// Modified by Mason Yu for multi user with passwd file.
				continue;
				//return 2;
			}
		}
	}
	}

	if (userflag) {
		fclose(fp);
		userflag = 0;
	}
	// Added by Mason Yu for web page via serverhost
	/*
	if ((strcmp(req->request_uri, "/")==0) && (!strcmp(usName, user)
	#ifdef ACCOUNT_CONFIG
		|| getAccPriv(user) == (int)PRIV_USER
	#endif
	))
		fclose(fp);
	*/
#else
//printf("user=%s, pass=%s name=%s pass2=%s\n",user,pass,mgmtUserName(),mgmtPassword());
	//if((strcmp(mgmtUserName(),user)==0)&&(strcmp(mgmtPassword(),pass)==0)) return 0;
	//else return 2;
	return 0;
#endif

#else
	struct passwd *pwp;

	pwp = getpwnam(user);
	if (pwp != NULL) {
		if (strcmp(crypt(pass, pwp->pw_passwd), pwp->pw_passwd) == 0)
			return 0;
	} else
#ifdef OLD_CONFIG_PASSWORDS
	/* For backwards compatibility we allow the global root password to work too */
	if ((auth_old_password[0] != '\0') &&
			((*user == '\0') || (strcmp(user, "root") == 0))) {
		if (strcmp(crypt_old(pass,auth_old_password),auth_old_password) == 0 ||
				strcmp(crypt(pass,auth_old_password),auth_old_password) == 0) {
			strcpy(user, "root");
			return 0;
		}
	} else
#endif	/* OLD_CONFIG_PASSWORDS */
		return 2;

#endif	/* ! EMBED */
#endif	/* SHADOW_AUTH */
	return 1;
}

#ifdef ACCOUNT_LOGIN_CONTROL
//added by xl_yue
struct errlogin_entry * scan_errlog_list(request * req)
{
	struct errlogin_entry * perrlog;

	for(perrlog = errlogin_list;perrlog;perrlog = perrlog->next){
		if(!strcmp(perrlog->remote_ip_addr, req->remote_ip_addr))
			break;
	}

	return perrlog;
}
//added by xl_yue
int check_errlogin_list(request * req)
{
	struct errlogin_entry * perrlog;
	struct errlogin_entry * berrlog = NULL;

	for(perrlog = errlogin_list;perrlog;berrlog = perrlog,perrlog = perrlog->next){
		if(!strcmp(perrlog->remote_ip_addr, req->remote_ip_addr))
			break;
	}

	if(!perrlog)
		return 0;

	if(perrlog->login_count >= MAX_LOG_NUM){
		return -1;
	}
	else{//unlist and free
		if(perrlog == errlogin_list)
			errlogin_list = perrlog->next;
		else
			berrlog->next = perrlog->next;

		free(perrlog);
	}
	return 0;
}
#endif

static void Send_Unauthorized(auth_dir *current, request *req) {
#if SUPPORT_AUTH_DIGEST
	if (current->authtype == HTTP_AUTH_DIGEST) {
		struct http_session *s;
		if ((s = http_da_session_start(current->realm, 0, 0)) != NULL) {

			send_r_unauthorized_digest(req, s);
			return;
		}
		// no more session available
		fprintf(stderr, "No more session available\n");
		send_r_bad_request(req);
	}
	else
#endif
	send_r_unauthorized(req,server_name);
}

int auth_authorize(request * req)
{
	int i, denied = 1;
	auth_dir *current;
 	int hash;
	char *pwd, *method;
#ifndef ACCOUNT_LOGIN_CONTROL
	static char current_client[20];
#endif
	char auth_userpass[0x80];
	//added by xl_yue
#ifdef ACCOUNT_LOGIN_CONTROL
	struct account_info * current_account_info = NULL;
	struct errlogin_entry * err_login;
	int do_succeed_log = 0;
#endif

//xl_yue
#ifdef USE_LOGINWEB_OF_SERVER
	struct user_info * pUser_info;
#ifdef LOGIN_ERR_TIMES_LIMITED
	struct errlogin_entry * pErrlog_entry = NULL;
#endif
#endif

	DBG(printf("auth_authorize\n");)

	hash = get_auth_hash_value(req->request_uri);
	current = auth_hashtable[hash];

	while (current) {
  		if (!memcmp(req->request_uri, current->directory,
								current->dir_len)) {
			if (current->directory[current->dir_len - 1] != '/' &&
				        req->request_uri[current->dir_len] != '/' &&
								req->request_uri[current->dir_len] != '\0') {
				break;
			}
//xl_yue add
#ifndef USE_LOGINWEB_OF_SERVER
			if (req->authorization)
			{
				if (current->authfile==0)
				{
					send_r_error(req);
					return 0;
				}

			#if SUPPORT_AUTH_DIGEST
				if (!strncasecmp(req->authorization,"Digest ",7))
				{
					struct MD5Context md5ctx;
//					char username[33],realm[34],nonce[33],qop[33];
					char realm[34],nonce[33],qop[33];
					char cnonce[33],response[34],nc[33],opaque[33];
					char HA1hex[33], entityHAhex[33], myresponse[33];
					struct http_session *session;

//					auth_find_value(req->authorization + 7, "username", username, sizeof(username));
					auth_find_value(req->authorization + 7, "username", auth_userpass, sizeof(auth_userpass));
					auth_find_value(req->authorization + 7, "realm", realm, sizeof(realm));
					auth_find_value(req->authorization + 7, "nonce", nonce, sizeof(nonce));
					auth_find_value(req->authorization + 7, "qop", qop, sizeof(qop));
					auth_find_value(req->authorization + 7, "cnonce", cnonce, sizeof(cnonce));
					auth_find_value(req->authorization + 7, "response", response, sizeof(response));
					auth_find_value(req->authorization + 7, "opaque", opaque, sizeof(opaque));
					auth_find_value(req->authorization + 7, "nc", nc, sizeof(nc));

					#if DBG_DIGEST
						char uri[256];
						auth_find_value(req->authorization + 7, "uri", uri, sizeof(uri));
						fprintf(stderr,"Client inputs:\n");
//						fprintf(stderr,"  username=%s\n", username);
						fprintf(stderr,"  auth_userpass=%s\n", auth_userpass);
						fprintf(stderr,"  realm=%s\n", realm);
						fprintf(stderr,"  nonce=%s\n", nonce);
						fprintf(stderr,"  qop=%s\n", qop);
						fprintf(stderr,"  cnonce=%s\n", cnonce);
						fprintf(stderr,"  response=%s\n", response);
						fprintf(stderr,"  opaque=%s\n", opaque);
						fprintf(stderr,"  uri=%s\n", uri);
						fprintf(stderr,"  nc=%s\n", nc);
					#endif

					if((session = http_da_session_update(realm, nonce, opaque, cnonce, nc)))
					{
						char tmpStr[64];
						char *str;

#ifndef MULTI_USER_PRIV
//						if(!strcmp(username, usName))
						if(!strcmp(auth_userpass, usName))
						{
							fclose(current->authfile);
							current->authfile = fopen("/var/DigestUser.passwd", "r");

							if (directory_index)
								free(directory_index);
#ifdef CONFIG_VDSL
							directory_index = strdup("index_user_vdsl.html");
#else
							directory_index = strdup("index_user.html");
#endif
						}
//						else if(!strcmp(username, suName))
						else if(!strcmp(auth_userpass, suName))
						{
							fclose(current->authfile);
							current->authfile = fopen("/var/DigestSuper.passwd", "r");

							if (directory_index)
								free(directory_index);

#ifdef CONFIG_VDSL								
							directory_index = strdup("index_vdsl.html");
#else
							directory_index = strdup("index.html");
#endif
						}
//						else if(!strcmp(username, "adsl"))
						else if(!strcmp(auth_userpass, "adsl"))
						{
							fclose(current->authfile);
							current->authfile = fopen("/var/DigestSuper.passwd", "r");

							if (directory_index)
								free(directory_index);
#ifdef CONFIG_VDSL
							directory_index = strdup("index_vdsl.html");
#else
							directory_index = strdup("index.html");
#endif
						}
#endif

						#if DBG_DIGEST
							fprintf(stderr, "DEBUG: enter http_da_session_update session\n");
						#endif

						fseek(current->authfile, 0, SEEK_SET);
						while (fgets(tmpStr, sizeof(tmpStr), current->authfile))
						{
							str = trim(tmpStr);

							#if DBG_DIGEST
//								fprintf(stderr, "DEBUG: tmpStr(username)=%s\n", tmpStr);
								fprintf(stderr, "DEBUG: tmpStr(auth_userpass)=%s\n", tmpStr);
							#endif

//							if (!strcmp(username, tmpStr) || !strcmp(username, "adsl"))
							if (!strcmp(auth_userpass, tmpStr) || !strcmp(auth_userpass, "adsl"))
							{
								fgets(tmpStr, sizeof(tmpStr),current->authfile);
								str = trim(tmpStr);

//								if(!strcmp(username, "adsl"))
								if(!strcmp(auth_userpass, "adsl"))
									str = strdup("realtek");

								#if DBG_DIGEST
									fprintf(stderr, "DEBUG: tmpStr(password)=%s\n", tmpStr);
								#endif

								switch(req->method)
								{
									case M_GET:
										method = strdup("GET");
										break;
									case M_HEAD:
										method = strdup("HEAD");
										break;
									case M_PUT:
										method = strdup("PUT");
										break;
									case M_POST:
										method = strdup("POST");
										break;
									case M_DELETE:
										method = strdup("DELETE");
										break;
									case M_LINK:
										method = strdup("LINK");
										break;
									case M_UNLINK:
										method = strdup("UNLINK");
										break;
									default:
										method = strdup("");
								}

//								http_da_calc_HA1(&md5ctx, NULL, username, session->realm, str, session->nonce, cnonce, HA1hex);
								http_da_calc_HA1(&md5ctx, NULL, auth_userpass, session->realm, str, session->nonce, cnonce, HA1hex);
								http_da_calc_response(&md5ctx, HA1hex, session->nonce, nc, cnonce, qop, method, req->request_uri, entityHAhex, myresponse);

								#if DBG_DIGEST
									fprintf(stderr, "DEBUG:   reponse=%s\n", response);
									fprintf(stderr, "DEBUG: myreponse=%s\n\n", myresponse);
								#endif

								free(method);

								if(!strcmp(response, myresponse))
								{
									denied = 0;
									break;
								}
							}
							else
								fgets(tmpStr, sizeof(tmpStr),current->authfile);
						}
					}
				}
				else
				{
			#endif  // SUPPORT_AUTH_DIGEST

					if (strncasecmp(req->authorization,"Basic ",6))
					{
						syslog(LOG_ERR, "Can only handle Basic auth\n");
						send_r_bad_request(req);
						return 0;
					}

					#if SUPPORT_AUTH_DIGEST
					if (current->authtype == HTTP_AUTH_DIGEST)
					{
						Send_Unauthorized(current, req);
						return 0;
					}
					#endif

					base64decode(auth_userpass, req->authorization+6, sizeof(auth_userpass));

					if ( (pwd = strchr(auth_userpass,':')) == 0 )
					{
						syslog(LOG_ERR, "No user:pass in Basic auth\n");
						send_r_bad_request(req);
						return 0;
					}

					*pwd++=0;
					// Modified by Mason Yu
					//denied = auth_check_userpass(auth_userpass,pwd,current->authfile);
					denied = auth_check_userpass(auth_userpass,pwd,current->authfile, req);
			#if SUPPORT_AUTH_DIGEST
				} // digest check
			#endif
#ifdef SECURITY_COUNTS
				if (strncmp(get_mime_type(req->request_uri),"image/",6))
					access__attempted(denied, auth_userpass);
#endif

#ifdef ACCOUNT_LOGIN_CONTROL
				//added by xl_yue
				if(!denied){
					//check if this ip in errlogin list
					if(check_errlogin_list(req)){
						denied = 5;
						goto pass_check;
					}

					//for user
					if (strcmp(usName, auth_userpass)==0){
						current_account_info = &us_account;
//						printf("select us_account\n");
					}
					//for super user
					else if(strcmp(suName, auth_userpass)==0){
						current_account_info = &su_account;
//						printf("select su_account\n");
					}
					else{
						current_account_info = NULL;
//						printf("select none account\n");
					}

					if(current_account_info){
						if (strcmp(current_account_info->remote_ip_addr, req->remote_ip_addr) == 0){
//							printf("the same ip\n");
							if(current_account_info->account_timeout){			//need login again for no acction for 5 minutes
								current_account_info->account_timeout = 0;
								current_account_info->last_time = time_counter;
								denied = 3;
//								printf("account timeout\n");
							}else{
//								printf("account not timeout\n");
								current_account_info->last_time = time_counter;
								current_account_info->account_busy = 1;
								//req->paccount_info = current_account_info;
								//current_account_info->refcnt++;
							}
						}else{
//							printf("not same ip\n");
							//only one user can login using the same account at the same time
							if(current_account_info->account_busy){
//								printf("account busy\n");
								denied = 4;
							}else{
//								printf("account not busy\n");
								current_account_info->account_timeout = 0;
								current_account_info->last_time = time_counter;
								current_account_info->account_busy = 1;
								do_succeed_log = 1;
								strncpy(current_account_info->remote_ip_addr, req->remote_ip_addr, sizeof(current_account_info->remote_ip_addr));
								//req->paccount_info = current_account_info;
								//current_account_info->refcnt++;
							}
						}
					}
				}else{
					/*login error three times,should wait 1 minutes */
					if((err_login = scan_errlog_list(req)) == NULL){
						err_login = (struct errlogin_entry*) malloc(sizeof(struct errlogin_entry));
						if (!err_login)
							die(OUT_OF_MEMORY);
						err_login->login_count = 0;
						strncpy(err_login->remote_ip_addr, req->remote_ip_addr, sizeof(err_login->remote_ip_addr));
						//list err_login to errlogin_list
						err_login->next = errlogin_list;
						errlogin_list = err_login;
					}
					if((++(err_login->login_count)) >= MAX_LOG_NUM){
						//if have login,then exit
						if(!strcmp(su_account.remote_ip_addr, req->remote_ip_addr)){
							su_account.account_timeout = 0;
							su_account.account_busy = 0;
							su_account.remote_ip_addr[0] = '\0';
						}
						if(!strcmp(us_account.remote_ip_addr, req->remote_ip_addr)){
							us_account.account_timeout = 0;
							us_account.account_busy = 0;
							us_account.remote_ip_addr[0] = '\0';
						}

						if(err_login->login_count == MAX_LOG_NUM)
							err_login->last_time = time_counter;

						denied = 5;
					}else{
						err_login->last_time = time_counter;
					}
				}
#endif

pass_check:
				if (denied) {
					switch (denied) {
						case 1:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: Bad Password\n", auth_userpass, req->remote_ip_addr);
							#if DBG_DIGEST
								fprintf(stderr, "DEBUG %d: Authentication attempt failed for %s from %s because: Bad Password\n", denied, auth_userpass, req->remote_ip_addr);
							#endif
							break;
						case 2:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: Invalid Username\n",	auth_userpass, req->remote_ip_addr);
							#if DBG_DIGEST
								fprintf(stderr, "DEBUG %d: Authentication attempt failed for %s from %s because: Invalid Username\n",	denied, auth_userpass, req->remote_ip_addr);
							#endif
							break;
#ifdef ACCOUNT_LOGIN_CONTROL
						//added by xl_yue
						case 3:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because:login has timeouted \n", auth_userpass, req->remote_ip_addr);
							#if DBG_DIGEST
								fprintf(stderr, "DEBUG %d: Authentication attempt failed for %s from %s because:login has timeouted \n", denied, auth_userpass, req->remote_ip_addr);
							#endif
							break;
						//added by xl_yue
						case 4:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: another has logined\n", auth_userpass, req->remote_ip_addr);
							#if DBG_DIGEST
								fprintf(stderr, "DEBUG %d: Authentication attempt failed for %s from %s because: another has logined\n", denied, auth_userpass, req->remote_ip_addr);
							#endif
							send_r_forbidden(req);
							return 0;
							break;
						//added by xl_yue
						case 5:
							syslog(LOG_ERR, "Authentication attempt failed for %s from %s because: have logined error for three times\n", auth_userpass, req->remote_ip_addr);
							#if DBG_DIGEST
								fprintf(stderr, "DEBUG %d: Authentication attempt failed for %s from %s because: have logined error for three times\n", denied, auth_userpass, req->remote_ip_addr);
							#endif
							send_r_forbidden(req);
							return 0;
							break;
#endif
						}
#ifndef ACCOUNT_LOGIN_CONTROL
					bzero(current_client, sizeof(current_client));
#endif
					Send_Unauthorized(current, req);
					return 0;


				}
#ifdef ACCOUNT_LOGIN_CONTROL
				if(do_succeed_log){
					syslog(LOG_INFO, "Authentication successful for %s from %s\n", auth_userpass, req->remote_ip_addr);
				}
#else
				if (strcmp(current_client, req->remote_ip_addr) != 0) {
					strncpy(current_client, req->remote_ip_addr, sizeof(current_client));
					syslog(LOG_INFO, "Authentication successful for %s from %s\n", auth_userpass, req->remote_ip_addr);
				}
#endif
				/* Copy user's name to request structure */
#ifdef LDAP_HACK
				if (!ldap_succ) {
					strcpy(req->user, "noldap");
					syslog(LOG_INFO, "Access granted as noldap");
				} else
#endif
				strncpy(req->user, auth_userpass, 15);
				req->user[15] = '\0';
				return 1;
			}else
			{
				/* No credentials were supplied. Tell them that some are required */
#ifdef ACCOUNT_LOGIN_CONTROL
				//added by xl_yue:if have login error for three times, forbid logining until 1 minute later
				if((err_login = scan_errlog_list(req)) != NULL){
					if(err_login->login_count >= MAX_LOG_NUM){
						send_r_forbidden(req);
						return 0;
					}
				}
#endif
				Send_Unauthorized(current, req);
				return 0;
			}
#else
//xl_yue
				pUser_info = search_login_list(req);
				if(pUser_info){
					//user account can not access admin account directory
#ifdef CONFIG_VDSL
					if((strcmp(req->request_uri,"/")) && (!strcmp(current->directory,"/")) && (!strcmp(pUser_info->directory,"index_user_vdsl.html"))){
#else
					if((strcmp(req->request_uri,"/")) && (!strcmp(current->directory,"/")) && (!strcmp(pUser_info->directory,"index_user.html"))){
#endif
						send_r_forbidden(req);
						return 0;
					}

					directory_index = pUser_info->directory;
					pUser_info->last_time = time_counter;
					return 1;
				}else{
#ifdef LOGIN_ERR_TIMES_LIMITED
					pErrlog_entry = search_errlog_list(req);
					if(pErrlog_entry){
						if(pErrlog_entry->login_count >= MAX_LOGIN_NUM){
							send_r_forbidden(req);
							return 0;
						}
					}
#endif
					if(!strcmp(req->request_uri,"/admin/login.asp")
						|| !strcmp(req->request_uri,"/admin/code.asp")
						|| !strcmp(req->request_uri,"/admin/md5.js")
						|| !strcmp(req->request_uri,"/admin/code_user.asp")
						|| !strcmp(req->request_uri,"/boaform/admin/formLogin")
						|| !strcmp(req->request_uri,"/boaform/admin/formLoginMultilang") // added by davian_kuo
						|| !strcmp(req->request_uri,"/admin/LoginFiles/realtek.jpg")
						|| !strcmp(req->request_uri,"/admin/LoginFiles/locker.gif"))
						return 1;

					directory_index = NULL;

					if(strcmp(req->request_uri,"/")){
/*
						req->response_status = R_FORBIDDEN;
						req_write(req, "<HTML><HEAD><TITLE>Forbidden</TITLE></HEAD>\n <BODY><H1>Forbidden </H1>\n");
						req_write(req, " Your requested URL is forbidden to access, maybe because you have not logined.\n");
 						req_write(req, " Please close this window and reopen your web browser to login.\n</BODY></HTML>\n");
						req_flush(req);
*/
						req->buffer_end=0;
						req->response_status = R_MOVED_PERM;
//						req_write(req, "<HTML><HEAD><TITLE>Login</TITLE></HEAD>\n"
//									 "<BODY><BLOCKQUOTE>\n <h2><font color=\"#0000FF\">Login</font></h2>"
//									 "You have not logined,please\n");
						boaWrite(req, "<HTML><HEAD><TITLE>Login</TITLE></HEAD>\n"
									 "<BODY><BLOCKQUOTE>\n <h2><font color=\"#0000FF\">%s</font></h2>"
									 "%s\n", multilang(LANG_LOGIN), multilang(LANG_RE_LOGIN_NOTICE));
						req_write(req, "<A HREF=\"/admin/login.asp\" target=_top>login</A>.\n</BLOCKQUOTE></BODY></HTML>\n");
						req_flush(req);
						return 0;
					}

					//boaRedirect(req, "/admin/login.asp");
					boaRedirectTemp(req,"/admin/login.asp"); //avoid caching
					return 0;
				}
#endif
		}
	  current = current->next;
  }

	return 1;
}

void dump_auth(void)
{
	int i;
	auth_dir *temp;

	for (i = 0; i < AUTH_HASHTABLE_SIZE; ++i) {
		if (auth_hashtable[i]) {
			temp = auth_hashtable[i];
			while (temp) {
				auth_dir *temp_next;

				if (temp->directory)
					free(temp->directory);
				if (temp->authfile)
					fclose(temp->authfile);
				temp_next = temp->next;
				free(temp);
				temp = temp_next;
			}
			auth_hashtable[i] = NULL;
		}
	}
}

#endif

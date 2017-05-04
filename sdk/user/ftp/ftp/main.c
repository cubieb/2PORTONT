/*
 * Copyright (c) 1985, 1989 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

char copyright[] =
  "@(#) Copyright (c) 1985, 1989 Regents of the University of California.\n"
  "All rights reserved.\n";

/*
 * from: @(#)main.c	5.18 (Berkeley) 3/1/91
 */
char main_rcsid[] = 
  "$Id: main.c,v 1.9 2012/10/24 14:12:57 tsaitc Exp $";


/*
 * FTP User Program -- Command Interface.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

/* #include <arpa/ftp.h>	<--- unused? */

#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <pwd.h>
#ifdef	__USE_READLINE__
#include <readline/readline.h>
#include <readline/history.h>
#endif

#ifdef __UCLIBC__
extern char *getlogin(void);
#endif

#define Extern
#include "ftp_var.h"
int traceflag = 0;
const char *home = "/";

extern FILE *cout;
extern int data;
extern struct cmd cmdtab[];
extern int NCMDS;

void intr(int);
void lostpeer(int);
void help(int argc, char *argv[]);

static void cmdscanner(int top);
static char *slurpstring(void);

static
void
usage(void)
{
	// Kaohj --- add '-f scriptFile'
	//printf("\n\tUsage: { ftp | pftp } [-pinegvtd] [hostname]\n");
	printf("\n\tUsage: { ftp | pftp } [-pinegvtd] {[-f scriptFile] | [hostname]}\n");
#ifdef CONFIG_USER_TR143
	printf("\t\t\t{-TR143TestMode}{-TestFileLength length}\n");
	printf("\t\t\t{-Interface name}{-DSCP value}\n");
#endif //CONFIG_USER_TR143
	printf("\t   -p: enable passive mode (default for pftp)\n");
	printf("\t   -i: turn off prompting during mget\n");
	printf("\t   -n: inhibit auto-login\n");
	printf("\t   -e: disable readline support, if present\n");
	printf("\t   -g: disable filename globbing\n");
	printf("\t   -v: verbose mode\n");
	printf("\t   -t: enable packet tracing [nonfunctional]\n");
	printf("\t   -d: enable debugging\n");
#ifdef CONFIG_USER_TR143
	printf("\t   -TR143TestMode: enable TR143 test mode\n");
	printf("\t   -TestFileLength: set TR143 upload file length\n");
	printf("\t   -Interface: set interface name(ex. ppp0...)\n");
	printf("\t   -DSCP: set DSCP value(0~63)\n");
#endif //CONFIG_USER_TR143
	printf("\n");
}

int
main(volatile int argc, char **volatile argv)
{
	register char *cp;
	int top;
	struct passwd *pw = NULL;
	char homedir[MAXPATHLEN];

#ifdef CONFIG_USER_REMOTE_MANAGEMENT
	// Kaohj --- init rmStatus
	rmStatus.operStatus = OPER_NORMAL;
	rmStatus.doneSize = 0;
	rmStatus.elapseTime = 0;
#endif
	tick = 0;
#ifdef CONFIG_USER_TR143
	memset( &gFtpTR143Diag, 0, sizeof(gFtpTR143Diag) );
#endif //CONFIG_USER_TR143

	memset(ifname, 0, sizeof(ifname));
	ftp_port = "21";
	doglob = 1;
	interactive = 1;
	autologin = 1;
	passivemode = 0;
#ifdef IPV6
	family = AF_UNSPEC;
#else
	family = AF_INET;
#endif

        cp = strrchr(argv[0], '/');
        cp = (cp == NULL) ? argv[0] : cp+1;
        if (strcmp(cp, "pftp") == 0)
            passivemode = 1;

#ifdef __USE_READLINE__
	/* 
	 * Set terminal type so libreadline can parse .inputrc correctly
	 */
	rl_terminal_name = getenv("TERM");
#endif

	argc--, argv++;
	while (argc > 0 && **argv == '-') {
		// Kaohj --- add '-f scriptFile'
		cp = *argv + 1;
		if (*cp && *cp == 'f') { // specify the ftp script file
			if (*(cp+1)=='\0') {
				// It's legal to get the script file
				argc--; argv++;
				if (argc > 0) {
					FILE *fp;
					// associate the script file to stdin
					fp = freopen(*argv, "r", stdin);
					if (fp == NULL) {
						//printf("ftp(%s): %s\n", *argv, strerror(errno));
						exit(0);
					}
#ifdef CONFIG_USER_TR143
					{
						char *prf=gFtpTR143Diag.Resultfile;
						int   rfsize=sizeof(gFtpTR143Diag.Resultfile);
						snprintf( prf, rfsize-1, "%s.result", *argv  );
						prf[rfsize-1]=0;
						fprintf( stderr, "gFtpTR143Diag.Resultfile=%s\n", gFtpTR143Diag.Resultfile );
					}
#endif //CONFIG_USER_TR143
					argc--; argv++;
				}
				else{
					usage();
					exit(0);
				}
				continue;
			}
		}else if( *cp && strcmp( cp, "Interface" )==0 )
		{
			argc--; argv++;

			if(argc && (strlen(*argv)<sizeof(ifname)))
			{
				strncpy(ifname, *argv, sizeof(ifname));
				fprintf( stderr, "ifname=%s\n", ifname);
			}else{
				usage();
				exit(0);
			}
#ifdef CONFIG_USER_TR143
		}else if( *cp && strcmp( cp, "TR143TestMode" )==0 )
		{
			gFtpTR143Diag.Enable=1;
			fprintf( stderr, "gFtpTR143Diag.Enable=%d\n", gFtpTR143Diag.Enable );
		}else if( *cp && strcmp( cp, "DSCP" )==0 )
		{
			argc--; argv++;
			if(argc)
			{
				gFtpTR143Diag.DSCP=atoi(*argv);
				fprintf( stderr, "gFtpTR143Diag.DSCP=%d\n", gFtpTR143Diag.DSCP );
			}else{
				usage();
				exit(0);
			}
		}else if( *cp && strcmp( cp, "TestFileLength" )==0 )
		{
			argc--; argv++;
			if(argc)
			{
				gFtpTR143Diag.TestFileLength=atoi(*argv);
				fprintf( stderr, "gFtpTR143Diag.TestFileLength=%u\n", gFtpTR143Diag.TestFileLength );
			}else{
				usage();
				exit(0);
		}
#endif //CONFIG_USER_TR143
		}else
		{
		for (cp = *argv + 1; *cp; cp++)
			switch (*cp) {

			case 'd':
				options |= SO_DEBUG;
				debug++;
				break;
			
			case 'v':
				verbose++;
				break;

			case 't':
				traceflag++;
				break;

			case 'i':
				interactive = 0;
				break;

			case 'n':
				autologin = 0;
				break;

			case 'p':
				passivemode = 1;
				break;

			case 'g':
				doglob = 0;
				break;
				
			case 'e':
				rl_inhibit = 1;
				break;
				
			case 'h':
				usage();
				exit(0);

			default:
				fprintf(stdout,
				  "ftp: %c: unknown option\n", *cp);
				exit(1);
			}
		}
		argc--, argv++;
	}
	fromatty = isatty(fileno(stdin));
	if (fromatty)
		verbose++;
	cpend = 0;	/* no pending replies */
	proxy = 0;	/* proxy not active */
	crflag = 1;	/* strip c.r. on ascii gets */
	sendport = -1;	/* not using ports */
	/*
	 * Set up the home directory in case we're globbing.
	 */
	cp = getlogin();
	if (cp != NULL) {
		pw = getpwnam(cp);
	}
	if (pw == NULL)
		pw = getpwuid(getuid());
	if (pw != NULL) {
		strlcpy(homedir, pw->pw_dir, sizeof(homedir));
		homedir[sizeof(homedir)-1] = 0;
		home = homedir;
	}
	if (argc > 0) {
		if (sigsetjmp(toplevel, 1))
			exit(0);
		(void) signal(SIGINT, intr);
		(void) signal(SIGPIPE, lostpeer);
		setpeer(argc + 1, argv - 1);
	}
	top = sigsetjmp(toplevel, 1) == 0;
	if (top) {
		(void) signal(SIGINT, intr);
		(void) signal(SIGPIPE, lostpeer);
	}
	for (;;) {
		cmdscanner(top);
		top = 1;
	}
}

void
intr(int ignore)
{
	(void)ignore;
	siglongjmp(toplevel, 1);
}

void
lostpeer(int ignore)
{
	(void)ignore;

	if (connected) {
		if (cout != NULL) {
			shutdown(fileno(cout), 1+1);
			fclose(cout);
			cout = NULL;
		}
		if (data >= 0) {
			shutdown(data, 1+1);
			close(data);
			data = -1;
		}
		connected = 0;
	}
	pswitch(1);
	if (connected) {
		if (cout != NULL) {
			shutdown(fileno(cout), 1+1);
			fclose(cout);
			cout = NULL;
		}
		connected = 0;
	}
	proxflag = 0;
	pswitch(0);
}

/*char *
tail(filename)
	char *filename;
{
	register char *s;
	
	while (*filename) {
		s = rindex(filename, '/');
		if (s == NULL)
			break;
		if (s[1])
			return (s + 1);
		*s = '\0';
	}
	return (filename);
}
*/

static char *get_input_line(char *buf, int buflen)
{
#ifdef __USE_READLINE__
	if (fromatty && !rl_inhibit) {
		char *lineread = readline("ftp> ");
		if (!lineread) return NULL;
		strlcpy(buf, lineread, buflen);
		buf[buflen-1] = 0;
		if (lineread[0]) add_history(lineread);
		free(lineread);
		return buf;
	}
#endif
	if (fromatty) {
		printf("ftp> ");
		fflush(stdout);
	}
	return fgets(buf, buflen, stdin);
}


/*
 * Command parser.
 */
static void
cmdscanner(int top)
{
	int margc;
	char *marg;
	char **margv;
	register struct cmd *c;
	register int l;

	if (!top)
		(void) putchar('\n');
	for (;;) {
		if (!get_input_line(line, sizeof(line))) {
			quit();
		}
		l = strlen(line);
		if (l == 0)
			break;
		if (line[--l] == '\n') {
			if (l == 0)
				break;
			line[l] = '\0';
		} 
		else if (l == sizeof(line) - 2) {
			printf("sorry, input line too long\n");
			while ((l = getchar()) != '\n' && l != EOF)
				/* void */;
			break;
		} /* else it was a line without a newline */
		margv = makeargv(&margc, &marg);
		if (margc == 0) {
			continue;
		}
		c = getcmd(margv[0]);
		if (c == (struct cmd *)-1) {
			printf("?Ambiguous command\n");
			continue;
		}
		if (c == NULL) {
			printf("?Invalid command\n");
			continue;
		}
		if (c->c_conn && !connected) {
			printf("Not connected.\n");			
			continue;
		}
		if (c->c_handler_v) c->c_handler_v(margc, margv);
		else if (c->c_handler_0) c->c_handler_0();
		else c->c_handler_1(marg);

		if (bell && c->c_bell) putchar('\007');
		if (c->c_handler_v != help)
			break;
	}
	(void) signal(SIGINT, intr);
	(void) signal(SIGPIPE, lostpeer);
}

struct cmd *
getcmd(const char *name)
{
	const char *p, *q;
	struct cmd *c, *found;
	int nmatches, longest;

	longest = 0;
	nmatches = 0;
	found = 0;
	for (c = cmdtab; (p = c->c_name) != NULL; c++) {
		for (q = name; *q == *p++; q++)
			if (*q == 0)		/* exact match? */
				return (c);
		if (!*q) {			/* the name was a prefix */
			if (q - name > longest) {
				longest = q - name;
				nmatches = 1;
				found = c;
			} else if (q - name == longest)
				nmatches++;
		}
	}
	if (nmatches > 1)
		return ((struct cmd *)-1);
	return (found);
}

/*
 * Slice a string up into argc/argv.
 */

int slrflag;

char **
makeargv(int *pargc, char **parg)
{
	static char *rargv[20];
	int rargc = 0;
	char **argp;

	argp = rargv;
	stringbase = line;		/* scan from first of buffer */
	argbase = argbuf;		/* store from first of buffer */
	slrflag = 0;
	while ((*argp++ = slurpstring())!=NULL)
		rargc++;

	*pargc = rargc;
	if (parg) *parg = altarg;
	return rargv;
}

/*
 * Parse string into argbuf;
 * implemented with FSM to
 * handle quoting and strings
 */
static
char *
slurpstring(void)
{
	static char excl[] = "!", dols[] = "$";

	int got_one = 0;
	register char *sb = stringbase;
	register char *ap = argbase;
	char *tmp = argbase;		/* will return this if token found */

	if (*sb == '!' || *sb == '$') {	/* recognize ! as a token for shell */
		switch (slrflag) {	/* and $ as token for macro invoke */
			case 0:
				slrflag++;
				stringbase++;
				return ((*sb == '!') ? excl : dols);
				/* NOTREACHED */
			case 1:
				slrflag++;
				altarg = stringbase;
				break;
			default:
				break;
		}
	}

S0:
	switch (*sb) {

	case '\0':
		goto OUT;

	case ' ':
	case '\t':
		sb++; goto S0;

	default:
		switch (slrflag) {
			case 0:
				slrflag++;
				break;
			case 1:
				slrflag++;
				altarg = sb;
				break;
			default:
				break;
		}
		goto S1;
	}

S1:
	switch (*sb) {

	case ' ':
	case '\t':
	case '\0':
		goto OUT;	/* end of token */

	case '\\':
		sb++; goto S2;	/* slurp next character */

	case '"':
		sb++; goto S3;	/* slurp quoted string */

	default:
		*ap++ = *sb++;	/* add character to token */
		got_one = 1;
		goto S1;
	}

S2:
	switch (*sb) {

	case '\0':
		goto OUT;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S1;
	}

S3:
	switch (*sb) {

	case '\0':
		goto OUT;

	case '"':
		sb++; goto S1;

	default:
		*ap++ = *sb++;
		got_one = 1;
		goto S3;
	}

OUT:
	if (got_one)
		*ap++ = '\0';
	argbase = ap;			/* update storage pointer */
	stringbase = sb;		/* update scan pointer */
	if (got_one) {
		return(tmp);
	}
	switch (slrflag) {
		case 0:
			slrflag++;
			break;
		case 1:
			slrflag++;
			altarg = NULL;
			break;
		default:
			break;
	}
	return NULL;
}

#define HELPINDENT ((int) sizeof ("directory"))

/*
 * Help command.
 * Call each command handler with argc == 0 and argv[0] == name.
 */
void
help(int argc, char *argv[])
{
	struct cmd *c;

	if (argc == 1) {
		int i, j, w;
		unsigned k;
		int columns, width = 0, lines;

		printf("Commands may be abbreviated.  Commands are:\n\n");
		for (c = cmdtab; c < &cmdtab[NCMDS]; c++) {
			int len = strlen(c->c_name);

			if (len > width)
				width = len;
		}
		width = (width + 8) &~ 7;
		columns = 80 / width;
		if (columns == 0)
			columns = 1;
		lines = (NCMDS + columns - 1) / columns;
		for (i = 0; i < lines; i++) {
			for (j = 0; j < columns; j++) {
				c = cmdtab + j * lines + i;
				if (c->c_name && (!proxy || c->c_proxy)) {
					printf("%s", c->c_name);
				}
				else if (c->c_name) {
					for (k=0; k < strlen(c->c_name); k++) {
						(void) putchar(' ');
					}
				}
				if (c + lines >= &cmdtab[NCMDS]) {
					printf("\n");
					break;
				}
				w = strlen(c->c_name);
				while (w < width) {
					w = (w + 8) &~ 7;
					(void) putchar('\t');
				}
			}
		}
		return;
	}
	while (--argc > 0) {
		register char *arg;
		arg = *++argv;
		c = getcmd(arg);
		if (c == (struct cmd *)-1)
			printf("?Ambiguous help command %s\n", arg);
		else if (c == NULL)
			printf("?Invalid help command %s\n", arg);
		else
			printf("%-*s\t%s\n", HELPINDENT,
				c->c_name, c->c_help);
	}
}

#ifdef CONFIG_USER_TR143
/********************************************************************************************************/
void FtpTR143UpdateResultFile(void)
{
	if(gFtpTR143Diag.Enable && strlen(gFtpTR143Diag.Resultfile) )
	{
		FILE	*fd;
		unsigned int byterecv=0,bytesent=0;

		// In this way we can handle overflow once becasue variables are unsigned int
		byterecv=gFtpTR143Diag.TotalBytesReceivedEnd-gFtpTR143Diag.TotalBytesReceivedStart;
		bytesent=gFtpTR143Diag.TotalBytesSentEnd-gFtpTR143Diag.TotalBytesSentStart;
			
		fd=fopen( gFtpTR143Diag.Resultfile, "w" );
		if(fd)
		{
			fprintf( fd, "%d %u %u %u %ld.%ld %ld.%ld %ld.%ld %ld.%ld %ld.%ld",
					gFtpTR143Diag.DiagnosticsState,
					gFtpTR143Diag.TestBytesReceived,
					byterecv,
					bytesent,
					gFtpTR143Diag.ROMTime.tv_sec,gFtpTR143Diag.ROMTime.tv_usec,
					gFtpTR143Diag.BOMTime.tv_sec,gFtpTR143Diag.BOMTime.tv_usec,
					gFtpTR143Diag.EOMTime.tv_sec,gFtpTR143Diag.EOMTime.tv_usec,
					gFtpTR143Diag.TCPOpenRequestTime.tv_sec,gFtpTR143Diag.TCPOpenRequestTime.tv_usec,
					gFtpTR143Diag.TCPOpenResponseTime.tv_sec,gFtpTR143Diag.TCPOpenResponseTime.tv_usec );
			fclose(fd);
		}
	}
}

void FtpTR143DiagDone(void)
{
	if(gFtpTR143Diag.Enable)
	{
		FtpTR143UpdateResultFile();
		quit();
	}
}

int FtpTR143GetEnable(void)
{
	return gFtpTR143Diag.Enable?1:0;
}

void FtpTR143SetState(int state)
{
	if(gFtpTR143Diag.Enable)
	{
		fprintf( stderr, "set gFtpTR143Diag.DiagnosticsState=%d\n", state );
		gFtpTR143Diag.DiagnosticsState=state;
		FtpTR143UpdateResultFile();	
	}
}

void FtpTR143SetFileSize(long long size)
{
	if(gFtpTR143Diag.Enable)
	{
		fprintf( stderr, "set gFtpTR143Diag.TestFileLength=%lld", size );
		if(size > 4294967295LL)
			fprintf( stderr, ", Overflow!!.\n");
		else
			fprintf( stderr, "\n");
		gFtpTR143Diag.TestFileLength=size;
		//FtpTR143UpdateResultFile();	
	}
}

void FtpTR143SetROMTime(void)
{
	if(gFtpTR143Diag.Enable)
	{
		struct timeval t;
		gettimeofday(&t, NULL);
		fprintf( stderr, "set gFtpTR143Diag.ROMTime=%lu.%lu\n", t.tv_sec, t.tv_usec  );
		gFtpTR143Diag.ROMTime=t;
		FtpTR143UpdateResultFile();
	}
}

void FtpTR143SetBOMTime(void)
{
	if(gFtpTR143Diag.Enable)
	{
		struct timeval t;
		gettimeofday(&t, NULL);
		fprintf( stderr, "set gFtpTR143Diag.BOMTime=%lu.%lu\n", t.tv_sec, t.tv_usec  );
		gFtpTR143Diag.BOMTime=t;
		FtpTR143UpdateResultFile();
	}
}

void FtpTR143SetEOMTime(void)
{
	if(gFtpTR143Diag.Enable)
	{
		struct timeval t;
		gettimeofday(&t, NULL);
		fprintf( stderr, "set gFtpTR143Diag.EOMTime=%lu.%lu\n", t.tv_sec, t.tv_usec  );
		gFtpTR143Diag.EOMTime=t;
		FtpTR143UpdateResultFile();
	}
}

void FtpTR143SetTCPOpenRequestTime(void)
{
	if(gFtpTR143Diag.Enable)
	{
		struct timeval t;
		gettimeofday(&t, NULL);
		fprintf( stderr, "set gFtpTR143Diag.TCPOpenRequestTime=%lu.%lu\n", t.tv_sec, t.tv_usec  );
		gFtpTR143Diag.TCPOpenRequestTime=t;
		FtpTR143UpdateResultFile();
	}
}

void FtpTR143SetTCPOpenResponseTime(void)
{
	if(gFtpTR143Diag.Enable)
	{
		struct timeval t;
		gettimeofday(&t, NULL);
		fprintf( stderr, "set gFtpTR143Diag.TCPOpenResponseTime=%lu.%lu\n", t.tv_sec, t.tv_usec  );
		gFtpTR143Diag.TCPOpenResponseTime=t;
		FtpTR143UpdateResultFile();
	}
}

void FtpTR143SetTestBytesReceived(long long size)
{
	if(gFtpTR143Diag.Enable)
	{
		fprintf( stderr, "set gFtpTR143Diag.TestBytesReceived=%lld", size );
		if(size > 4294967295LL)
			fprintf( stderr, ", Overflow!!.\n");
		else
			fprintf( stderr, "\n");
		gFtpTR143Diag.TestBytesReceived=size;
		FtpTR143UpdateResultFile();	
	}
}

#include <sys/types.h>
#include <ifaddrs.h>
static int FtpTR143FindItfNamebyAddr(union sockunion *su, char *n)
{
	struct ifaddrs *ifaddr, *ifa;
	int s, family;

	if(!su || !n) return -1;

	if (getifaddrs(&ifaddr) == -1)
	{
		perror("getifaddrs\n");
		return 0;
	}

	for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
	{
		char hostbuf[INET6_ADDRSTRLEN] = {0};
		struct sockaddr_in *inet_addr;
		struct sockaddr_in6 *inet6_addr;

		if (ifa->ifa_addr == NULL)
			continue;

		family = ifa->ifa_addr->sa_family;

		if(family != su->su_family)
			continue;

		fprintf( stderr, "findItfNamebyAddr>got name=%s\n", ifa->ifa_name);
		getnameinfo(ifa->ifa_addr,
			(family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
				hostbuf, sizeof(hostbuf) - 1, NULL, 0, NI_NUMERICHOST);
		fprintf( stderr, "findItfNamebyAddr>got family=%d, %s\n", family, hostbuf);

		union sockunion *c = (union sockunion *)ifa->ifa_addr;
		if ((family == AF_INET6 && 
			memcmp(&c->su_sin6.sin6_addr, &su->su_sin6.sin6_addr, 
				sizeof(struct in6_addr)) == 0) || 
		(family == AF_INET &&
		 	memcmp(&c->su_sin.sin_addr, &su->su_sin.sin_addr, 
				sizeof(struct in_addr)) == 0)) {
			strcpy( n, ifa->ifa_name );
			fprintf( stderr, "findItfNamebyAddr>got match\n" );
			break;
		}
	}
	freeifaddrs(ifaddr);

	return 0;
}

void FtpTR143FindInterfaceBySockFd(int sockfd)
{
	if( gFtpTR143Diag.Enable && (strlen(ifname)==0) )
	{
		union sockunion myaddr;
		socklen_t myaddrlen=sizeof(myaddr);
		if( getsockname( sockfd, (struct sockaddr *)&myaddr, &myaddrlen )< 0 )
			perror( "getsockname" );
		else{
			char hostbuf[INET6_ADDRSTRLEN];
			getnameinfo((struct sockaddr *)&myaddr, sizeof(myaddr),
					hostbuf, sizeof(hostbuf) - 1, NULL, 0,
					NI_NUMERICHOST);
			fprintf( stderr, "my out addr=%s\n", hostbuf );
			FtpTR143FindItfNamebyAddr( &myaddr, ifname );
		}
	}
}

static char *get_name(char *name, char *p)
{
    while (isspace(*p))
	p++;
    while (*p) {
	if (isspace(*p))
	    break;
	if (*p == ':') {	/* could be an alias */
	    char *dot = p, *dotname = name;
	    *name++ = *p++;
	    while (isdigit(*p))
		*name++ = *p++;
	    if (*p != ':') {	/* it wasn't, backup */
		p = dot;
		name = dotname;
	    }
	    if (*p == '\0')
		return NULL;
	    p++;
	    break;
	}
	*name++ = *p++;
    }
    *name++ = '\0';
    return p;
}

static int getInterfaceStat(
	char *ifname,
	unsigned long *bs,
	unsigned long *br,
	unsigned long *ps,
	unsigned long *pr )
{
	int	ret=-1;
	FILE 	*fh;
	char 	buf[512];

	if( (bs==NULL) || (br==NULL) || (ps==NULL) || (pr==NULL) )	return ret;
	*bs=0; *br=0; *ps=0; *pr=0;
	
	fh = fopen("/proc/net/dev", "r");
	if (!fh) return ret;
	
	fgets(buf, sizeof buf, fh);	/* eat line */
	fgets(buf, sizeof buf, fh);
	while (fgets(buf, sizeof buf, fh))
	{
		char *s, name[128];
		s = get_name(name, buf);    
		if (!strcmp(ifname, name)) {
			sscanf(s,
			"%lu %lu %*lu %*lu %*lu %*lu %*lu %*lu %lu %lu %*lu %*lu %*lu %*lu %*lu %*lu",
			br, pr, bs, ps);
			ret=0;
			break;
		}
	}
	fclose(fh);
	return ret;
}

void FtpTR143SetItfStatsForStart(void)
{
	if(gFtpTR143Diag.Enable && (strlen(ifname)>0))
	{
		unsigned long bs,br,ps,pr;
		if( getInterfaceStat( ifname, &bs, &br, &ps, &pr )==0 )
		{
			gFtpTR143Diag.TotalBytesReceivedStart=br;
			gFtpTR143Diag.TotalBytesSentStart=bs;
		}
		fprintf( stderr, "set gFtpTR143Diag.TotalBytesReceivedStart=%u\n", gFtpTR143Diag.TotalBytesReceivedStart );
		fprintf( stderr, "set gFtpTR143Diag.TotalBytesSentStart=%u\n", gFtpTR143Diag.TotalBytesSentStart );
	}
}

void FtpTR143SetItfStatsForEnd(void)
{
	if(gFtpTR143Diag.Enable && (strlen(ifname)>0))
	{
		unsigned long bs,br,ps,pr;
		if( getInterfaceStat( ifname, &bs, &br, &ps, &pr )==0 )
		{
			gFtpTR143Diag.TotalBytesReceivedEnd=br;
			gFtpTR143Diag.TotalBytesSentEnd=bs;
		}
		fprintf( stderr, "set gFtpTR143Diag.TotalBytesReceivedEnd=%u\n", gFtpTR143Diag.TotalBytesReceivedEnd );
		fprintf( stderr, "set gFtpTR143Diag.TotalBytesSentEnd=%u\n", gFtpTR143Diag.TotalBytesSentEnd );
	}
}

int FtpTR143SetSockOption(int socket)
{
	if(gFtpTR143Diag.Enable)
	{
		int keepalive = 1;
	
		if( gFtpTR143Diag.DSCP )
		{
			int tos_value=(gFtpTR143Diag.DSCP<<2);
			fprintf( stderr, "FtpTR143SetSockOption> set IP_TOS=%d\n", tos_value );
			if (setsockopt(socket, IPPROTO_IP, IP_TOS, &tos_value, sizeof(int)))
			{
				perror( "setsockopt:IP_TOS" );
				return -1;
			}
		}
		// enable TCP keep alive to prvent control connection lost
		if(setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive)))
		{
			perror( "setsockopt:SO_KEEPALIVE" );
			return -1;
		}
	}
	return 0;
}
/********************************************************************************************************/
#endif //CONFIG_USER_TR143


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

/* 
 * From: @(#)ftp.c	5.38 (Berkeley) 4/22/91
 */
char ftp_rcsid[] = 
  "$Id: ftp.c,v 1.12 2012/10/24 14:12:57 tsaitc Exp $";

#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/file.h>

#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <pwd.h>
#include <stdarg.h>
// Kaohj
#include <rtk/sysconfig.h>

#include "ftp_var.h"
#include "cmds.h"

#ifdef __UC_LIBC__
char *getlogin(void) { return((char *) NULL); }
#endif

#include "../version.h"
//Mason Yu
#include <rtk/sysconfig.h>
#include <rtk/options.h>
#include <rtk/utility.h>

#ifdef CONFIG_USER_REMOTE_MANAGEMENT
// Kaohj
#define RMBYTES 102400
struct rcvst rmStatus;
#endif
#ifndef CONFIG_LUNA
// Mason Yu
int updateType;
#define UPD_IMAGE	0
#define UPD_CONF	1
#define UPD_NONE	5
#endif

int data = -1;
off_t restart_point = 0;

static union sockunion hisctladdr;
static union sockunion data_addr;
static union sockunion myctladdr;
static int ptflag = 0;
static sigjmp_buf recvabort;
static sigjmp_buf sendabort;
static sigjmp_buf ptabort;
static int ptabflg = 0;
static int abrtflag = 0;

void lostpeer(int);
extern int connected;

static char *gunique(char *);
static void proxtrans(const char *cmd, char *local, char *remote);
static int initconn(void);
static void ptransfer(const char *direction, long long bytes, 
		      const struct timeval *t0, 
		      const struct timeval *t1);
static void tvsub(struct timeval *tdiff, 
		  const struct timeval *t1, 
		  const struct timeval *t0);
static void abort_remote(FILE *din);

FILE *cin, *cout;
static FILE *dataconn(const char *);

#ifdef CONFIG_USER_REMOTE_MANAGEMENT
// Kaohj --- update the RM ftp status
void
update_rmstatus(struct rcvst *status)
{
	FILE *fp;
	
	if ((fp = fopen("/var/ftpStatus.txt", "w")) == NULL)
	{
		printf("***** Open file /var/ftpStatus.txt failed !\n");
	}
	else {
		fprintf(fp, "OperStatus: %d\n", status->operStatus);
		fprintf(fp, "TotalSize: %d\n", status->doneSize);
		fprintf(fp, "ElapseTime: %d\n", status->elapseTime);	
		fclose(fp);
	}
}
#endif

char *
hookup(const char *host, const char *port)
{
	int s = -1, error;
	struct addrinfo hints, *res, *res0;
	static char hostnamebuf[MAXHOSTNAMELEN];
	socklen_t len;

	memset((char *)&hisctladdr, 0, sizeof (hisctladdr));
	memset((char *)&myctladdr, 0, sizeof (myctladdr));
	memset(&hints, 0, sizeof(hints));

	/* This flag will cause segfault on LUNA Linux 2.6.30 */
#if !defined(CONFIG_LUNA) || !defined(CONFIG_KERNEL_2_6_30)
	hints.ai_flags = AI_CANONNAME;
#endif
	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
	error = getaddrinfo(host, port, &hints, &res0);
	if (error) {
		fprintf(stderr, "ftp: can't lookup `[%s]:%s'", host, port);
		code = -1;
		return (0);
	}

	if (res0->ai_canonname)
		(void)strlcpy(hostnamebuf, res0->ai_canonname,
		    sizeof(hostnamebuf));
	else
		(void)strlcpy(hostnamebuf, host, sizeof(hostnamebuf));
	hostname = hostnamebuf;

	for (res = res0; res; res = res->ai_next) {
		char hname[NI_MAXHOST], sname[NI_MAXSERV];

		ai_unmapped(res);
		if (getnameinfo(res->ai_addr, res->ai_addrlen,
		    hname, sizeof(hname), sname, sizeof(sname),
		    NI_NUMERICHOST | NI_NUMERICSERV) != 0) {
			strlcpy(hname, "?", sizeof(hname));
			strlcpy(sname, "?", sizeof(sname));
		}
		if (verbose && res0->ai_next) {
				/* if we have multiple possibilities */
			fprintf(stderr, "ftp: trying [%s]:%s ...\n", hname, sname);
		}
		s = socket(res->ai_family, SOCK_STREAM, res->ai_protocol);
		if (s < 0) {
			fprintf(stderr, "ftp: can't create socket for connection to `[%s]:%s'",
			    hname, sname);
			continue;
		}
		if( strlen(ifname) )
		{
			struct ifreq ifr = {0};
			strcpy(ifr.ifr_name, ifname);
			fprintf( stderr, "set SO_BINDTODEVICE=%s\n", ifname);
			if (setsockopt(s, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)))
			{
				perror( "setsockopt:SO_BINDTODEVICE" );
				return -1;
			}
		}

#ifdef CONFIG_USER_TR143
		if(FtpTR143SetSockOption(s)<0) return (0); 
#endif //CONFIG_USER_TR143

		if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
			close(s);
			s = -1;
			continue;
		}

		/* finally we got one */
		break;
	}
	if (s < 0) {
		fprintf(stderr, "ftp: can't connect to `[%s]:%s'", host, port);

#ifdef CONFIG_USER_REMOTE_MANAGEMENT
		// Added by Mason Yu for Remote Management. Start
		rmStatus.operStatus = OPER_CONNECT_FAILURE;
		rmStatus.doneSize = 0;
		rmStatus.elapseTime = 0;
		update_rmstatus(&rmStatus);
		// Added by Mason Yu for Remote Management. End
#endif

		code = -1;
		freeaddrinfo(res0);
		goto bad;
	}
	memcpy(&hisctladdr, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res0);
	res0 = res = NULL;

	len = sizeof(myctladdr);
	if (getsockname(s, (struct sockaddr *)&myctladdr, &len) < 0) {
		perror("ftp: getsockname");
		code = -1;
		goto bad;
	}

#ifdef IP_TOS
	if (hisctladdr.su_family == AF_INET) {
		int tos = IPTOS_LOWDELAY;
		if (setsockopt(s, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)) == -1) {
				perror("ftp: setsockopt TOS (ignored)");
		}
	}
#endif
	cin = fdopen(s, "r");
	cout = fdopen(s, "w");
	if (cin == NULL || cout == NULL) {
		fprintf(stderr, "ftp: fdopen failed.\n");
		if (cin)
			(void) fclose(cin);
		if (cout)
			(void) fclose(cout);
		code = -1;
		goto bad;
	}
	if (verbose)
		printf("Connected to %s.\n", hostname);
	if (getreply(0) > 2) { 	/* read startup message from server */
		if (cin)
			(void) fclose(cin);
		if (cout)
			(void) fclose(cout);
		code = -1;
		goto bad;
	}
#ifdef SO_OOBINLINE
	{
		int on = 1;

		if (setsockopt(s, SOL_SOCKET, SO_OOBINLINE, &on, 
					sizeof(on)) < 0 && debug) {
			perror("ftp: setsockopt");
		}
	}
#endif /* SO_OOBINLINE */

	
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
	// Added by Mason Yu for Remote Management. Start
	rmStatus.operStatus = OPER_CONNECT_SUCCESS;
	rmStatus.doneSize = 0;
	rmStatus.elapseTime = 0;
	update_rmstatus(&rmStatus);
	// Added by Mason Yu for Remote Management. End
#endif

	
	return (hostname);
bad:
	(void) close(s);
	return ((char *)0);
}

int
dologin(const char *host)
{
	char tmp[80];
	char *luser, *pass, *zacct;
	int n, aflag = 0;

	luser = pass = zacct = 0;
	if (xruserpass(host, &luser, &pass, &zacct) < 0) {
		code = -1;
		return(0);
	}
	while (luser == NULL) {
		char *myname = getlogin();

		if (myname == NULL) {
			struct passwd *pp = getpwuid(getuid());

			if (pp != NULL)
				myname = pp->pw_name;
		}
		if (myname)
			printf("Name (%s:%s): ", host, myname);
		else
			printf("Name (%s): ", host);
#ifdef EMBED
		fflush(stdout);
#endif
		if (fgets(tmp, sizeof(tmp) - 1, stdin)==NULL) {
			fprintf(stderr, "\nLogin failed.\n");
			return 0;
		}
		tmp[strlen(tmp) - 1] = '\0';
		if (*tmp == '\0')
			luser = myname;
		else
			luser = tmp;
	}
	n = command("USER %s", luser);
	if (n == CONTINUE) {
		if (pass == NULL) {
			/* fflush(stdout); */
			pass = getpass("Password:");
		}
		n = command("PASS %s", pass);
	}
	if (n == CONTINUE) {
		aflag++;
		/* fflush(stdout); */
		zacct = getpass("Account:");
		n = command("ACCT %s", zacct);
	}
	if (n != COMPLETE) {
		fprintf(stderr, "Login failed.\n");
		return (0);
	}
	if (!aflag && zacct != NULL)
		(void) command("ACCT %s", zacct);
	if (proxy)
		return(1);
	for (n = 0; n < macnum; ++n) {
		if (!strcmp("init", macros[n].mac_name)) {
			int margc;
			char **margv;
			strcpy(line, "$init");
			margv = makeargv(&margc, NULL);
			domacro(margc, margv);
			break;
		}
	}
	return (1);
}


static void
cmdabort(int ignore)
{
	(void)ignore;

	printf("\n");
	fflush(stdout);
	abrtflag++;
	if (ptflag) siglongjmp(ptabort,1);
}

int
command(const char *fmt, ...)
{
	va_list ap;
	int r;
	void (*oldintr)(int);

	abrtflag = 0;
	if (debug) {
		printf("---> ");
		va_start(ap, fmt);
		if (strncmp("PASS ", fmt, 5) == 0)
			printf("PASS XXXX");
		else 
			vfprintf(stdout, fmt, ap);
		va_end(ap);
		printf("\n");
		(void) fflush(stdout);
	}
	if (cout == NULL) {
		perror ("No control connection for command");
		code = -1;
		return (0);
	}
	oldintr = signal(SIGINT, cmdabort);
	va_start(ap, fmt);
	vfprintf(cout, fmt, ap);
	va_end(ap);
	fprintf(cout, "\r\n");
	(void) fflush(cout);
	cpend = 1;
	r = getreply(!strcmp(fmt, "QUIT"));
	if (abrtflag && oldintr != SIG_IGN)
		(*oldintr)(SIGINT);
	(void) signal(SIGINT, oldintr);
	return(r);
}

char reply_string[BUFSIZ];		/* last line of previous reply */

#include <ctype.h>

int
getreply(int expecteof)
{
	register int c, n;
	register int dig;
	register char *cp;
	int originalcode = 0, continuation = 0;
	void (*oldintr)(int);
	int pflag = 0;
	size_t px = 0;
	size_t psize = sizeof(pasv);

	oldintr = signal(SIGINT, cmdabort);
	for (;;) {
		dig = n = code = 0;
		cp = reply_string;
		while ((c = getc(cin)) != '\n') {
			if (c == IAC) {     /* handle telnet commands */
				switch (c = getc(cin)) {
				case WILL:
				case WONT:
					c = getc(cin);
					fprintf(cout, "%c%c%c", IAC, DONT, c);
					(void) fflush(cout);
					break;
				case DO:
				case DONT:
					c = getc(cin);
					fprintf(cout, "%c%c%c", IAC, WONT, c);
					(void) fflush(cout);
					break;
				default:
					break;
				}
				continue;
			}
			dig++;
			if (c == EOF) {
				if (expecteof) {
					(void) signal(SIGINT,oldintr);
					code = 221;
					return (0);
				}
				lostpeer(0);
				if (verbose) {
					printf("421 Service not available, remote server has closed connection\n");
					(void) fflush(stdout);
				}
				code = 421;
				return(4);
			}
			if (c != '\r' && (verbose > 0 ||
			    (verbose > -1 && n == '5' && dig > 4))) {
				if (proxflag &&
				   (dig == 1 || (dig == 5 && verbose == 0)))
					printf("%s:",hostname);
				(void) putchar(c);
			}
			if (dig < 4 && isdigit(c))
				code = code * 10 + (c - '0');
			if (!pflag && code == 227)
				pflag = 1;
			else if (!pflag && code == 229)
				pflag = 100;
			if (dig > 4 && pflag == 1 && isdigit(c))
				pflag = 2;
			if (pflag == 2) {
				if (c != '\r' && c != ')') {
					if (px < psize-1) pasv[px++] = c;
				}
				else {
					pasv[px] = '\0';
					pflag = 3;
				}
			}
			if (pflag == 100 && c == '(')
				pflag = 2;
			if (dig == 4 && c == '-') {
				if (continuation)
					code = 0;
				continuation++;
			}
			if (n == 0)
				n = c;
			if (cp < &reply_string[sizeof(reply_string) - 1])
				*cp++ = c;
		}
		if (verbose > 0 || (verbose > -1 && n == '5')) {
			(void) putchar(c);
			(void) fflush (stdout);
		}
		if (continuation && code != originalcode) {
			if (originalcode == 0)
				originalcode = code;
			continue;
		}
		*cp = '\0';
		//fprintf(stderr, "<%s:%d>Reply got: code=%d, reply=%s\n", __func__, __LINE__, code, reply_string);
		if (n != '1')
			cpend = 0;
		(void) signal(SIGINT,oldintr);
		if (code == 421 || originalcode == 421)
			lostpeer(0);
		if (abrtflag && oldintr != cmdabort && oldintr != SIG_IGN)
			(*oldintr)(SIGINT);
		return (n - '0');
	}
}

static int
empty(fd_set *mask, int hifd, int sec)
{
	struct timeval t;

	t.tv_sec = (long) sec;
	t.tv_usec = 0;
	return(select(hifd+1, mask, (fd_set *) 0, (fd_set *) 0, &t));
}

static void
abortsend(int ignore)
{
	(void)ignore;

	mflag = 0;
	abrtflag = 0;
	printf("\nsend aborted\nwaiting for remote to finish abort\n");
	(void) fflush(stdout);
	siglongjmp(sendabort, 1);
}

#define HASHBYTES 1024

void
sendrequest(const char *cmd, char *local, char *remote, int printnames)
{
	struct stat st;
	struct timeval start, stop;
	register int c, d;
	FILE *volatile fin, *volatile dout = 0;
	int (*volatile closefunc)(FILE *);
	void (*volatile oldintr)(int);
	void (*volatile oldintp)(int);
	volatile long long bytes = 0, hashbytes = HASHBYTES;
	char buf[BUFSIZ], *bufp;
	const char *volatile lmode;	
	int reply_code;
	
#ifdef CONFIG_USER_TR143
	FtpTR143SetState( eTR143_Error_NoResponse );
#endif //CONFIG_USER_TR143
	if (verbose && printnames) {
		if (local && *local != '-')
			printf("local: %s ", local);
		if (remote)
			printf("remote: %s\n", remote);
	}
	if (proxy) {
		proxtrans(cmd, local, remote);
#ifdef CONFIG_USER_TR143
		FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
		return;
	}
	if (curtype != type)
		changetype(type, 0);
	closefunc = NULL;
	oldintr = NULL;
	oldintp = NULL;
	lmode = "w";
	if (sigsetjmp(sendabort, 1)) {
		while (cpend) {
			(void) getreply(0);
		}
		if (data >= 0) {
			(void) close(data);
			data = -1;
		}
		if (oldintr)
			(void) signal(SIGINT,oldintr);
		if (oldintp)
			(void) signal(SIGPIPE,oldintp);
		code = -1;
#ifdef CONFIG_USER_TR143
		FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
		return;
	}
	oldintr = signal(SIGINT, abortsend);
	if (strcmp(local, "-") == 0)
		fin = stdin;
	else if (*local == '|') {
		oldintp = signal(SIGPIPE,SIG_IGN);
		fin = popen(local + 1, "r");
		if (fin == NULL) {
			perror(local + 1);
			(void) signal(SIGINT, oldintr);
			(void) signal(SIGPIPE, oldintp);
			code = -1;
#ifdef CONFIG_USER_TR143
			FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
			return;
		}
		closefunc = pclose;
	} else {
		fin = fopen(local, "r");
		if (fin == NULL) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			(void) signal(SIGINT, oldintr);
			code = -1;
#ifdef CONFIG_USER_TR143
			FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
			return;
		}
		closefunc = fclose;
		if (fstat(fileno(fin), &st) < 0 ||
		    (st.st_mode&S_IFMT) != S_IFREG) {
			fprintf(stdout, "%s: not a plain file.\n", local);
			(void) signal(SIGINT, oldintr);
			fclose(fin);
			code = -1;
#ifdef CONFIG_USER_TR143
			FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
			return;
		}
	}
	if (initconn()) {
		(void) signal(SIGINT, oldintr);
		if (oldintp)
			(void) signal(SIGPIPE, oldintp);
		code = -1;
		if (closefunc != NULL)
			(*closefunc)(fin);
#ifdef CONFIG_USER_TR143
		FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
		return;
	}
#ifdef CONFIG_USER_TR143
	FtpTR143SetState( eTR143_Error_NoSTOR );
#endif //CONFIG_USER_TR143
	if (sigsetjmp(sendabort, 1))
		goto abort;

	if (restart_point &&
	    (strcmp(cmd, "STOR") == 0 || strcmp(cmd, "APPE") == 0)) {
		if (fseek(fin, (long) restart_point, 0) < 0) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			restart_point = 0;
			if (closefunc != NULL)
				(*closefunc)(fin);
#ifdef CONFIG_USER_TR143
			FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
			return;
		}
		if (command("REST %ld", (long) restart_point)
			!= CONTINUE) {
			restart_point = 0;
			if (closefunc != NULL)
				(*closefunc)(fin);
#ifdef CONFIG_USER_TR143
			FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
			return;
		}
		restart_point = 0;
		lmode = "r+w";
	}
	if (remote) {
#ifdef CONFIG_USER_TR143
		FtpTR143SetROMTime();
#endif //CONFIG_USER_TR143
		if (command("%s %s", cmd, remote) != PRELIM) {
			(void) signal(SIGINT, oldintr);
			if (oldintp)
				(void) signal(SIGPIPE, oldintp);
			if (closefunc != NULL)
				(*closefunc)(fin);
#ifdef CONFIG_USER_TR143
			FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
			return;
		}
	} else
		if (command("%s", cmd) != PRELIM) {
			(void) signal(SIGINT, oldintr);
			if (oldintp)
				(void) signal(SIGPIPE, oldintp);
			if (closefunc != NULL)
				(*closefunc)(fin);
			return;
		}
	dout = dataconn(lmode);
	if (dout == NULL)
		goto abort;
	(void) gettimeofday(&start, (struct timezone *)0);
	oldintp = signal(SIGPIPE, SIG_IGN);
	switch (curtype) {

	case TYPE_I:
	case TYPE_L:
		errno = d = 0;
#ifdef CONFIG_USER_TR143
		if( FtpTR143GetEnable() )
		{
			unsigned int sentsize=gFtpTR143Diag.TestFileLength;
			memset( buf, 'A', sizeof (buf) );
			FtpTR143SetItfStatsForStart();
			FtpTR143SetBOMTime();
			while(sentsize)
			{
				if(sentsize>sizeof (buf))
					c=sizeof (buf);
				else
					c=sentsize;
				sentsize -=c;
				bytes += c;
				for (bufp = buf; c > 0; c -= d, bufp += d)
					if ((d = write(fileno(dout), bufp, c)) <= 0)
						break;
				if (hash) {
					while (bytes >= hashbytes) {
						(void) putchar('#');
						hashbytes += HASHBYTES;
					}
					(void) fflush(stdout);
				}
				if (tick && (bytes >= hashbytes)) {
					printf("\rBytes transferred: %lld", bytes);
					(void) fflush(stdout);
					while (bytes >= hashbytes)
						hashbytes += TICKBYTES;
				}				
			}
		}else
#endif //CONFIG_USER_TR143
		{
			while ((c = read(fileno(fin), buf, sizeof (buf))) > 0) {
				bytes += c;
				for (bufp = buf; c > 0; c -= d, bufp += d)
					if ((d = write(fileno(dout), bufp, c)) <= 0)
						break;
				if (hash) {
					while (bytes >= hashbytes) {
						(void) putchar('#');
						hashbytes += HASHBYTES;
					}
					(void) fflush(stdout);
				}
				if (tick && (bytes >= hashbytes)) {
					printf("\rBytes transferred: %lld", bytes);
					(void) fflush(stdout);
					while (bytes >= hashbytes)
						hashbytes += TICKBYTES;
				}
			}
		}
		if (hash && (bytes > 0)) {
			if (bytes < HASHBYTES)
				(void) putchar('#');
			(void) putchar('\n');
			(void) fflush(stdout);
		}
		if (tick) {
			(void) printf("\rBytes transferred: %lld\n", bytes);
			(void) fflush(stdout);
		}
		if (c < 0)
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
		if (d < 0) {
			if (errno != EPIPE) 
				perror("netout");
			bytes = -1;
		}
		break;

	case TYPE_A:
		while ((c = getc(fin)) != EOF) {
			if (c == '\n') {
				while (hash && (bytes >= hashbytes)) {
					(void) putchar('#');
					(void) fflush(stdout);
					hashbytes += HASHBYTES;
				}
				if (tick && (bytes >= hashbytes)) {
					(void) printf("\rBytes transferred: %lld",
						bytes);
					(void) fflush(stdout);
					while (bytes >= hashbytes)
						hashbytes += TICKBYTES;
				}
				if (ferror(dout))
					break;
				(void) putc('\r', dout);
				bytes++;
			}
			(void) putc(c, dout);
			bytes++;
	/*		if (c == '\r') {			  	*/
	/*		(void)	putc('\0', dout);  (* this violates rfc */
	/*			bytes++;				*/
	/*		}                          			*/     
		}
		if (hash) {
			if (bytes < hashbytes)
				(void) putchar('#');
			(void) putchar('\n');
			(void) fflush(stdout);
		}
		if (tick) {
			(void) printf("\rBytes transferred: %lld\n", bytes);
			(void) fflush(stdout);
		}
		if (ferror(fin))
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
		if (ferror(dout)) {
			if (errno != EPIPE)
				perror("netout");
			bytes = -1;
		}
		break;
	}
	(void) gettimeofday(&stop, (struct timezone *)0);
	if (closefunc != NULL)
		(*closefunc)(fin);
	(void) fclose(dout);
	/* closes data as well, so discard it */
	data = -1;
	reply_code=getreply(0);
#ifdef CONFIG_USER_TR143
	FtpTR143SetItfStatsForEnd();
	FtpTR143SetEOMTime();
	if( reply_code==COMPLETE )
		FtpTR143SetState( eTR143_Completed );
	else
		FtpTR143SetState( eTR143_Error_NoTransferComplete );
#endif //CONFIG_USER_TR143
	(void) signal(SIGINT, oldintr);
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	if (bytes > 0)
		ptransfer("sent", bytes, &start, &stop);
	return;
abort:
	
	(void) gettimeofday(&stop, (struct timezone *)0);
	(void) signal(SIGINT, oldintr);
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	if (!cpend) {
		code = -1;
#ifdef CONFIG_USER_TR143
		FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
		return;
	}
	if (dout) {
		(void) fclose(dout);
	}
	if (data >= 0) {
		/* if it just got closed with dout, again won't hurt */
		(void) close(data);
		data = -1;
	}
	(void) getreply(0);
	code = -1;
	if (closefunc != NULL && fin != NULL)
		(*closefunc)(fin);
	if (bytes > 0)
		ptransfer("sent", bytes, &start, &stop);
#ifdef CONFIG_USER_TR143
	FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
}

static void
abortrecv(int ignore)
{
	(void)ignore;

	mflag = 0;
	abrtflag = 0;
	printf("\nreceive aborted\nwaiting for remote to finish abort\n");
	(void) fflush(stdout);
	siglongjmp(recvabort, 1);
}

#ifndef CONFIG_LUNA
// Kaohj -- verify file header for configuration file
static int isConfile(char *buf)
{
#ifdef CONFIG_USE_XML
	int i, j, hlen;
	char cnfHdr[128];
	
	j = 0;
	hlen = strlen(CONFIG_HEADER);
	if (hlen > 128)
		hlen = 128;
	for (i=0; i<hlen; i++) {
#ifdef XOR_ENCRYPT
		cnfHdr[i] = buf[i]^XOR_KEY[j++];
		if (XOR_KEY[j] == '\0')
			j = 0;
#else
		cnfHdr[i] = buf[i];
#endif
	}
	cnfHdr[hlen] = 0;
	if (strcmp(cnfHdr, CONFIG_HEADER))
		return 0;
	return 1;
#else
	return !strncmp(buf, "ADSL-CS-", 8);
#endif
}
#endif

void
recvrequest(const char *cmd, 
	    char *volatile local, char *remote, 
	    const char *lmode, int printnames)
{
	FILE *volatile fout, *volatile din = 0;
	int reply_code;
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
	int rmftp_mode;
	struct timeval td;
	volatile long rmbytes = HASHBYTES;
	char str_remote[64];
#endif
	int (*volatile closefunc)(FILE *);
	void (*volatile oldintp)(int);
	void (*volatile oldintr)(int);
	volatile int is_retr, tcrflag, bare_lfs = 0;
	static unsigned bufsize=0;
	static char *buf;
	volatile long long bytes = 0, hashbytes = HASHBYTES;
	register int c, d;
	struct timeval start, stop;
	struct stat st;		
#ifndef CONFIG_LUNA
	// Mason Yu
	int checked = 0;	
	IMGHDR *phdr;
	/*ql: 20080729 START: check image key according to IC version*/
#ifdef MULTI_IC_SUPPORT
	unsigned int key=0;
#endif
	/*ql: 20080729 END*/
#endif
	
#ifdef CONFIG_USER_TR143
	FtpTR143SetState( eTR143_Error_NoResponse );
#endif //CONFIG_USER_TR143
	is_retr = strcmp(cmd, "RETR") == 0;
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
	// Kaohj --- Remote Management
	if (is_retr && rmStatus.operStatus == OPER_CONNECT_SUCCESS) {
		// Added by Mason Yu. Save downloading status.
		rmStatus.operStatus = OPER_DOWNLOADING;
		rmStatus.doneSize = 0;
		rmStatus.elapseTime = 0;
		update_rmstatus(&rmStatus);
		// Added by Mason Yu for Remote Management. End
	}
#endif
	if (is_retr && verbose && printnames) {
		if (local && *local != '-')
			printf("local: %s ", local);
		if (remote)
			printf("remote: %s\n", remote);
	}
	if (proxy && is_retr) {
		proxtrans(cmd, local, remote);
#ifdef CONFIG_USER_TR143
		FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
		return;
	}
	closefunc = NULL;
	oldintr = NULL;
	oldintp = NULL;
	tcrflag = !crflag && is_retr;
	if (sigsetjmp(recvabort, 1)) {
		while (cpend) {
			(void) getreply(0);
		}
		if (data >= 0) {
			(void) close(data);
			data = -1;
		}
		if (oldintr)
			(void) signal(SIGINT, oldintr);
		code = -1;
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
		if (is_retr && rmStatus.operStatus == OPER_DOWNLOADING) {
			rmStatus.operStatus = OPER_DOWNLOAD_FAILURE;
			rmStatus.doneSize = 0;
			rmStatus.elapseTime = 0;
			update_rmstatus(&rmStatus);
		}
#endif
#ifdef CONFIG_USER_TR143
		FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
		return;
	}
	oldintr = signal(SIGINT, abortrecv);
	if (strcmp(local, "-") && *local != '|') {
		if (access(local, W_OK) < 0) {
			char *dir = rindex(local, '/');

			if (errno != ENOENT && errno != EACCES) {
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				(void) signal(SIGINT, oldintr);
				code = -1;
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
				// Kaohj
				if (is_retr) {
					rmStatus.operStatus = OPER_DOWNLOAD_FAILURE;
					update_rmstatus(&rmStatus);
				}
#endif
#ifdef CONFIG_USER_TR143
				FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
				return;
			}
			if (dir != NULL)
				*dir = 0;
			d = access(dir ? local : ".", W_OK);
			if (dir != NULL)
				*dir = '/';
			if (d < 0) {
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				(void) signal(SIGINT, oldintr);
				code = -1;
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
				// Kaohj
				if (is_retr) {
					rmStatus.operStatus = OPER_DOWNLOAD_FAILURE;
					update_rmstatus(&rmStatus);
				}
#endif
#ifdef CONFIG_USER_TR143
				FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
				return;
			}
			if (!runique && errno == EACCES &&
			    chmod(local, 0600) < 0) {
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				/*
				 * Believe it or not, this was actually
				 * repeated in the original source.
				 */
				(void) signal(SIGINT, oldintr);
				/*(void) signal(SIGINT, oldintr);*/
				code = -1;
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
				// Kaohj
				if (is_retr) {
					rmStatus.operStatus = OPER_DOWNLOAD_FAILURE;
					update_rmstatus(&rmStatus);
				}
#endif
#ifdef CONFIG_USER_TR143
				FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
				return;
			}
			if (runique && errno == EACCES &&
			   (local = gunique(local)) == NULL) {
				(void) signal(SIGINT, oldintr);
				code = -1;
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
				// Kaohj
				if (is_retr) {
					rmStatus.operStatus = OPER_DOWNLOAD_FAILURE;
					update_rmstatus(&rmStatus);
				}
#endif
#ifdef CONFIG_USER_TR143
				FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
				return;
			}
		}
		else if (runique && (local = gunique(local)) == NULL) {
			(void) signal(SIGINT, oldintr);
			code = -1;
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
			// Kaohj
			if (is_retr) {
				rmStatus.operStatus = OPER_DOWNLOAD_FAILURE;
				update_rmstatus(&rmStatus);
			}
#endif
#ifdef CONFIG_USER_TR143
			FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
			return;
		}
	}
	if (!is_retr) {
		if (curtype != TYPE_A)
			changetype(TYPE_A, 0);
	} 
	else if (curtype != type) {
		changetype(type, 0);
	}
	if (initconn()) {
		(void) signal(SIGINT, oldintr);
		code = -1;
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
		// Kaohj
		if (is_retr) {
			rmStatus.operStatus = OPER_DOWNLOAD_FAILURE;
			update_rmstatus(&rmStatus);
		}
#endif
#ifdef CONFIG_USER_TR143
		FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
		return;
	}
#ifdef CONFIG_USER_TR143
	FtpTR143SetState( eTR143_Error_Timeout );
#endif //CONFIG_USER_TR143
	if (sigsetjmp(recvabort, 1))
		goto abort;
	if (is_retr && restart_point &&
	    command("REST %ld", (long) restart_point) != CONTINUE)
	{
#ifdef CONFIG_USER_TR143
		FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
		return;
	}
	if (remote) {
#ifdef CONFIG_USER_TR143
		FtpTR143SetROMTime();
#endif //CONFIG_USER_TR143
		if (command("%s %s", cmd, remote) != PRELIM) {
			(void) signal(SIGINT, oldintr);
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
			// Kaohj
			if (is_retr) {
				rmStatus.operStatus = OPER_DOWNLOAD_FAILURE;
				update_rmstatus(&rmStatus);
			}
#endif
#ifdef CONFIG_USER_TR143
			FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
			return;
		}
	} 
	else {
		if (command("%s", cmd) != PRELIM) {
			(void) signal(SIGINT, oldintr);
#ifdef CONFIG_USER_TR143
			FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
			return;
		}
	}
	din = dataconn("r");
	if (din == NULL)
		goto abort;
	if (strcmp(local, "-") == 0)
		fout = stdout;
	else if (*local == '|') {
		oldintp = signal(SIGPIPE, SIG_IGN);
		fout = popen(local + 1, "w");
		if (fout == NULL) {
			perror(local+1);
			goto abort;
		}
		closefunc = pclose;
	} 
	else {
		fout = fopen(local, lmode);
		if (fout == NULL) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			goto abort;
		}
		closefunc = fclose;
	}
	if (fstat(fileno(fout), &st) < 0 || st.st_blksize == 0)
		st.st_blksize = BUFSIZ;
	if (st.st_blksize > bufsize) {
		if (buf)
			(void) free(buf);
		buf = malloc((unsigned)st.st_blksize);
		if (buf == NULL) {
			perror("malloc");
			bufsize = 0;
			goto abort;
		}
		bufsize = st.st_blksize;
	}
	(void) gettimeofday(&start, (struct timezone *)0);
	switch (curtype) {

	case TYPE_I:
	case TYPE_L:
		if (restart_point &&
		    lseek(fileno(fout), (long) restart_point, L_SET) < 0) {
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
			if (closefunc != NULL)
				(*closefunc)(fout);
#ifdef CONFIG_USER_TR143
			FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
			return;
		}
		errno = d = 0;
#ifdef CONFIG_USER_TR143
		FtpTR143SetItfStatsForStart();
#endif //CONFIG_USER_TR143
		while ((c = read(fileno(din), buf, bufsize)) > 0) {
#ifdef CONFIG_USER_TR143
			if(gFtpTR143Diag.Enable)
			{
				if(bytes==0) FtpTR143SetBOMTime();
				d=c;//skip write for tr143 test
			}else
#endif //CONFIG_USER_TR143
#ifndef CONFIG_LUNA
			// Mason Yu			
			if (!checked) {
				checked = 1;
				phdr = (IMGHDR *)buf;
				updateType = UPD_NONE;
				if ( isConfile(buf))
					updateType = UPD_CONF;				
				/*ql:20080729 START: different ic matched with different image key*/
#ifdef MULTI_IC_SUPPORT
				key = getImgKey();
				
				if ((key == (phdr->key & key)) && (((phdr->key>>28)&0xf) == ((key>>28)&0xf))) {
					// Telnet_CLI will use ftp, not kill
					cmd_killproc(ALL_PID & ~(1 << PID_CLI | 1 << PID_TELNETD | 1 << PID_CWMP));
					updateType = UPD_IMAGE;
				}
#else
				if ( phdr->key == APPLICATION_IMAGE) {
					// Telnet_CLI will use ftp, not kill
					cmd_killproc(ALL_PID & ~(1 << PID_CLI | 1 << PID_TELNETD | 1 << PID_CWMP));
					updateType = UPD_IMAGE;
				}
#endif
				/*ql:20080729 END*/
				if (updateType == UPD_NONE) {
					printf("FTPC: Invalid File !\n");
					break;
					//goto abort;
				}
			}
#endif
			
			if ((d = write(fileno(fout), buf, c)) != c)
				break;
			bytes += c;
			if (hash && is_retr) {
				while (bytes >= hashbytes) {
					(void) putchar('#');
					hashbytes += HASHBYTES;
				}
				(void) fflush(stdout);
			}
			if (tick && (bytes >= hashbytes) && is_retr) {
				(void) printf("\rBytes transferred: %lld",
					bytes);
				(void) fflush(stdout);
				while (bytes >= hashbytes)
					hashbytes += TICKBYTES;
			}
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
			// Kaohj
			// Remote management
			if ((bytes >= rmbytes) && is_retr) {
				// log receiving status
				struct timeval td;
					
				(void) gettimeofday(&stop, (struct timezone *)0);
				tvsub(&td, &stop, &start);
				
				rmStatus.operStatus = OPER_DOWNLOADING;
				rmStatus.doneSize = bytes/1024;
				rmStatus.elapseTime = td.tv_sec;
				update_rmstatus(&rmStatus);
				while (bytes >= rmbytes)
					rmbytes += RMBYTES;
			}
#endif
		}
#ifdef CONFIG_USER_TR143
		FtpTR143SetEOMTime();
		FtpTR143SetItfStatsForEnd();
		if(bytes>0) FtpTR143SetTestBytesReceived(bytes);
		if(bytes==gFtpTR143Diag.TestFileLength)
			FtpTR143SetState( eTR143_Completed );
		else
			FtpTR143SetState( eTR143_Error_TransferFailed );
#endif //CONFIG_USER_TR143
		if (hash && bytes > 0) {
			if (bytes < HASHBYTES)
				(void) putchar('#');
			(void) putchar('\n');
			(void) fflush(stdout);
		}
		if (tick && is_retr) {
			(void) printf("\rBytes transferred: %lld\n", bytes);
			(void) fflush(stdout);
		}
		if (c < 0) {
			if (errno != EPIPE)
				perror("netin");
			bytes = -1;
		}
		if (d < c) {
			if (d < 0)
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
			else
				fprintf(stderr, "%s: short write\n", local);
		}
		break;

	case TYPE_A:
		if (restart_point) {
			register int i, n, ch;

			if (fseek(fout, 0L, L_SET) < 0)
				goto done;
			n = restart_point;
			for (i = 0; i++ < n;) {
				if ((ch = getc(fout)) == EOF)
					goto done;
				if (ch == '\n')
					i++;
			}
			if (fseek(fout, 0L, L_INCR) < 0) {
done:
				fprintf(stderr, "local: %s: %s\n", local,
					strerror(errno));
				if (closefunc != NULL)
					(*closefunc)(fout);
				return;
			}
		}
		while ((c = getc(din)) != EOF) {
			if (c == '\n')
				bare_lfs++;
			while (c == '\r') {
				while (hash && (bytes >= hashbytes)
					&& is_retr) {
					(void) putchar('#');
					(void) fflush(stdout);
					hashbytes += HASHBYTES;
				}
				if (tick && (bytes >= hashbytes) && is_retr) {
					printf("\rBytes transferred: %lld",
						bytes);
					fflush(stdout);
					while (bytes >= hashbytes)
						hashbytes += TICKBYTES;
				}
				bytes++;
				if ((c = getc(din)) != '\n' || tcrflag) {
					if (ferror(fout))
						goto break2;
					(void) putc('\r', fout);
					if (c == '\0') {
						bytes++;
						goto contin2;
					}
					if (c == EOF)
						goto contin2;
				}
			}
			(void) putc(c, fout);
			bytes++;
	contin2:	;
		}
break2:
		if (hash && is_retr) {
			if (bytes < hashbytes)
				(void) putchar('#');
			(void) putchar('\n');
			(void) fflush(stdout);
		}
		if (tick && is_retr) {
			(void) printf("\rBytes transferred: %lld\n", bytes);
			(void) fflush(stdout);
		}
		if (bare_lfs) {
			printf("WARNING! %d bare linefeeds received in ASCII mode\n", bare_lfs);
			printf("File may not have transferred correctly.\n");
		}
		if (ferror(din)) {
			if (errno != EPIPE)
				perror("netin");
			bytes = -1;
		}
		if (ferror(fout))
			fprintf(stderr, "local: %s: %s\n", local,
				strerror(errno));
		break;
	}
	if (closefunc != NULL)
		(*closefunc)(fout);
	(void) signal(SIGINT, oldintr);
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	(void) gettimeofday(&stop, (struct timezone *)0);
	(void) fclose(din);
	/* closes data as well, so discard it */
	data = -1;
	// Kaohj
	//(void) getreply(0);
	reply_code = getreply(0);
	//printf("code=%d, ret=%d\n", code, reply_code);
	if (bytes > 0 && is_retr)
		ptransfer("received", bytes, &start, &stop);
	// Kaohj --- Remote Management
	//printf("reply: %s\n", reply_string);
	//printf("download ok (local:%s, remote:%s)\n", local, remote);
#ifdef CONFIG_USER_REMOTE_MANAGEMENT
	// Added by Mason Yu for Remote Management. Start
	if (is_retr) {
		tvsub(&td, &stop, &start);
		if (reply_code == 2) { // download success
			if (!strcmp(local, "/dev/null")) {
				// ftpTest
				rmftp_mode = 3;
				rmStatus.operStatus = OPER_DOWNLOAD_SUCCESS;
			}
			else {
				// ftp upgrade
				rmftp_mode = 2;
				sprintf(str_remote, "/tmp/%s", remote);
#ifndef CONFIG_LUNA
				if (!cmd_check_image(str_remote, 0)) {
					//fprintf(stderr, "Image Checksum Failed\n");
					rmStatus.operStatus = OPER_UPGRADE_FAILURE;
					rmftp_mode = 1; // stop
				}
				else {
#endif
					rmStatus.operStatus = OPER_SAVING;
#ifndef CONFIG_LUNA
   				}
#endif
			}
		}
		else { // download fail
			rmftp_mode = 1;
			rmStatus.operStatus = OPER_DOWNLOAD_FAILURE;
		}
		rmStatus.doneSize = bytes/1024;
		rmStatus.elapseTime = td.tv_sec;
		update_rmstatus(&rmStatus);
		// Added by Mason Yu for Remote Management. End
		if (rmftp_mode == 2) {
			FILE *fp;
			struct stat st;
			// ftp upgrade
			//printf("upgrade here\n");
			
			if ((fp = fopen(str_remote, "rb")) == NULL) {
				printf("File %s open fail\n", str_remote);
				return;
			}

			if (fstat(fileno(fp), &st) < 0) {
				printf("File %s get status fail\n", str_remote);
				fclose(fp);
				return;	
			}		
	
			if (st.st_size <= 0) {
				printf("File %s size error\n", str_remote);
				fclose(fp);
				return;
			}
			fclose(fp);			
			cmd_upload(str_remote, 0, st.st_size);
			
			rmStatus.operStatus = OPER_UPGRADE_SUCCESS;
			rmStatus.doneSize = bytes/1024;
			rmStatus.elapseTime = td.tv_sec;
			update_rmstatus(&rmStatus);
		}
	}
#endif
	return;
abort:

#ifdef CONFIG_USER_REMOTE_MANAGEMENT
/* abort using RFC959 recommended IP,SYNC sequence  */	
	(void) gettimeofday(&stop, (struct timezone *)0);
	tvsub(&td, &stop, &start);
	rmStatus.operStatus = OPER_DOWNLOAD_FAILURE;
	rmStatus.doneSize = bytes/1024;
	rmStatus.elapseTime = td.tv_sec;
	update_rmstatus(&rmStatus);
#endif
	if (oldintp)
		(void) signal(SIGPIPE, oldintp);
	(void) signal(SIGINT, SIG_IGN);
	if (!cpend) {
		code = -1;
		(void) signal(SIGINT, oldintr);
#ifdef CONFIG_USER_TR143
		FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
		return;
	}

	abort_remote(din);
	code = -1;
	if (closefunc != NULL && fout != NULL)
		(*closefunc)(fout);
	if (din) {
		(void) fclose(din);
	}
	if (data >= 0) {
		/* if it just got closed with din, again won't hurt */
		(void) close(data);
		data = -1;
	}
	if (bytes > 0)
		ptransfer("received", bytes, &start, &stop);
	(void) signal(SIGINT, oldintr);
#ifdef CONFIG_USER_TR143
	FtpTR143DiagDone();
#endif //CONFIG_USER_TR143
}

/*
 * Need to start a listen on the data channel before we send the command,
 * otherwise the server's connect may fail.
 */
static int
initconn(void)
{
	int result, tmpno = 0;
	socklen_t len;
	int on = 1;
	u_long a1, a2, a3, a4, p1, p2;

	if (passivemode) {
#ifdef CONFIG_USER_TR143
		FtpTR143SetState( eTR143_Error_NoPASV );
#endif //CONFIG_USER_TR143
		data = socket(myctladdr.su_family, SOCK_STREAM, 0);
		if (data < 0) {
			perror("ftp: socket");
			return(1);
		}
		if (options & SO_DEBUG &&
		    setsockopt(data, SOL_SOCKET, SO_DEBUG, &on, sizeof (on)) < 0)
			perror("ftp: setsockopt (ignored)");

		if (myctladdr.su_family == AF_INET6) {
			char delim[4];

			if (command("EPSV") != COMPLETE) {
				printf("Extended passive mode refused.\n");
				return(1);
			}

			if (sscanf(pasv, "%c%c%c%lu%c", &delim[0], &delim[1], 
						&delim[2], &p1, &delim[3]) != 5) {
				printf("Extended passive mode address scan failure. "
								"Shouldn't happen!\n");
				return(1);
			}

			if (delim[0] != delim[1] || delim[0] != delim[2]
						 || delim[0] != delim[3]) {
				printf("Extended passive mode parse error.\n");
				return(1);
			}
			data_addr = hisctladdr;
			data_addr.su_port = htons(p1);
		} else {
			if (command("PASV") != COMPLETE) {
				printf("Passive mode refused.\n");
				return(1);
			}

			/*
			 * What we've got at this point is a string of comma separated
			 * one-byte unsigned integer values, separated by commas.
			 * The first four are the an IP address. The fifth is the MSB
			 * of the port number, the sixth is the LSB. From that we'll
			 * prepare a sockaddr_in.
			 */

			if (sscanf(pasv,"%ld,%ld,%ld,%ld,%ld,%ld",
					&a1, &a2, &a3, &a4, &p1, &p2) != 6) {
				printf("Passive mode address scan failure."
							" Shouldn't happen!\n");
				return(1);
			}

			data_addr.su_family = AF_INET;
			data_addr.su_sin.sin_addr.s_addr = 
				htonl((a1 << 24) | (a2 << 16) | (a3 << 8) | a4);
			data_addr.su_port = htons((p1 << 8) | p2);
		}

#ifdef CONFIG_USER_TR143
		FtpTR143SetState( eTR143_Error_NoResponse );
		if(FtpTR143SetSockOption(data)<0) return (1); 
		FtpTR143SetTCPOpenRequestTime();
#endif //CONFIG_USER_TR143
		if (connect(data, (struct sockaddr *)&data_addr, sizeof(data_addr)) < 0) {
			perror("ftp: connect");
			return(1);
		}
#ifdef CONFIG_USER_TR143
		FtpTR143SetTCPOpenResponseTime();
		FtpTR143FindInterfaceBySockFd(data);
#endif //CONFIG_USER_TR143
#ifdef IP_TOS
		if (data_addr.su_family == AF_INET) {
			int tos = IPTOS_THROUGHPUT;
			if (setsockopt(data, IPPROTO_IP, IP_TOS,
					&tos, sizeof(tos)) < 0)
				perror("ftp: setsockopt TOS (ignored)");
		}
#endif
		return(0);
	}

noport:
	data_addr = myctladdr;
	if (sendport)
		data_addr.su_port = 0;	/* let system pick one */ 
	if (data != -1)
		(void) close(data);
	data = socket(data_addr.su_family, SOCK_STREAM, 0);
	if (data < 0) {
		perror("ftp: socket");
		if (tmpno)
			sendport = 1;
		return (1);
	}
	if (!sendport)
		if (setsockopt(data, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
			perror("ftp: setsockopt (reuse address)");
			goto bad;
		}
	if (bind(data, (struct sockaddr *)&data_addr, sizeof (data_addr)) < 0) {
		perror("ftp: bind");
		goto bad;
	}
	if (options & SO_DEBUG &&
	    setsockopt(data, SOL_SOCKET, SO_DEBUG, &on, sizeof(on)) < 0)
		perror("ftp: setsockopt (ignored)");
	len = sizeof (data_addr);
	if (getsockname(data, (struct sockaddr *)&data_addr, &len) < 0) {
		perror("ftp: getsockname");
		goto bad;
	}
	if (listen(data, 1) < 0)
		perror("ftp: listen");
	if (sendport) {
#define	UC(b)	(((int)b)&0xff)
		if (data_addr.su_family == AF_INET6) {
			char hostbuf[INET6_ADDRSTRLEN];
			getnameinfo((struct sockaddr *)&data_addr, sizeof(data_addr),
					hostbuf, sizeof(hostbuf) - 1,
					NULL, 0, NI_NUMERICHOST);
			result = command("EPRT |2|%s|%hu|", hostbuf,
						ntohs(data_addr.su_port));
		} else {
			register char *p, *a;
			a = (char *)&data_addr.su_sin.sin_addr;
			p = (char *)&data_addr.su_port;
			result = command("PORT %d,%d,%d,%d,%d,%d",
						UC(a[0]), UC(a[1]), UC(a[2]), UC(a[3]),
						UC(p[0]), UC(p[1]));
		}

		if (result == ERROR && sendport == -1) {
			sendport = 0;
			tmpno = 1;
			goto noport;
		}
		return (result != COMPLETE);
	}
	if (tmpno)
		sendport = 1;
#ifdef IP_TOS
	if (data_addr.su_family == AF_INET) {
		int tos = IPTOS_THROUGHPUT;
		if (setsockopt(data, IPPROTO_IP, IP_TOS, &tos,
					sizeof(tos)) < 0)
			perror("ftp: setsockopt TOS (ignored)");
	}
#endif
	return (0);
bad:
	(void) close(data), data = -1;
	if (tmpno)
		sendport = 1;
	return (1);
}

static FILE *
dataconn(const char *lmode)
{
	union sockunion from;
	int s;
	socklen_t fromlen = sizeof(from);

        if (passivemode)
            return (fdopen(data, lmode));

	s = accept(data, (struct sockaddr *) &from, &fromlen);
	if (s < 0) {
		perror("ftp: accept");
		(void) close(data), data = -1;
		return (NULL);
	}
	(void) close(data);
	data = s;
#ifdef IP_TOS
	if (from.su_family == AF_INET) {
		int tos = IPTOS_THROUGHPUT;
		if (setsockopt(s, IPPROTO_IP, IP_TOS, &tos,
					sizeof(tos)) < 0)
			perror("ftp: setsockopt TOS (ignored)");
	}
#endif
	return (fdopen(data, lmode));
}

static void
ptransfer(const char *direction, long long bytes, 
	  const struct timeval *t0, 
	  const struct timeval *t1)
{
	struct timeval td;
#ifdef __UCLIBC__
	/* Use all ints for embedded targets */
	unsigned long s, bs;

	if (verbose) {
		tvsub(&td, t1, t0);
		s = (td.tv_sec * 1000) + (td.tv_usec / 1000);
#define	nz(x)	((x) == 0 ? 1 : (x))
		bs = bytes / nz(s);		
		printf("%lld bytes %s in %lu secs (%lu Kbytes/sec)\n",
		    bytes, direction, (s/1000), bs);
	}
#else
	float s, bs;

	if (verbose) {
		tvsub(&td, t1, t0);
		s = td.tv_sec + (td.tv_usec / 1000000.);
#define	nz(x)	((x) == 0 ? 1 : (x))
		bs = bytes / nz(s);		
		printf("%ld bytes %s in %.3g secs (%.2g Kbytes/sec)\n",
		    bytes, direction, s, bs / 1024.0);
	}
#endif
}

#if 0
tvadd(tsum, t0)
	struct timeval *tsum, *t0;
{

	tsum->tv_sec += t0->tv_sec;
	tsum->tv_usec += t0->tv_usec;
	if (tsum->tv_usec > 1000000)
		tsum->tv_sec++, tsum->tv_usec -= 1000000;
}
#endif

static void
tvsub(struct timeval *tdiff, 
      const struct timeval *t1, 
      const struct timeval *t0)
{

	tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
	tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
	if (tdiff->tv_usec < 0)
		tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}

static 
void
psabort(int ignore)
{
	(void)ignore;
	abrtflag++;
}

void
pswitch(int flag)
{
	void (*oldintr)(int);
	static struct comvars {
		int connect;
		char name[MAXHOSTNAMELEN];
		union sockunion mctl;
		union sockunion hctl;
		FILE *in;
		FILE *out;
		int tpe;
		int curtpe;
		int cpnd;
		int sunqe;
		int runqe;
		int mcse;
		int ntflg;
		char nti[17];
		char nto[17];
		int mapflg;
		char mi[MAXPATHLEN];
		char mo[MAXPATHLEN];
	} proxstruct, tmpstruct;
	struct comvars *ip, *op;

	abrtflag = 0;
	oldintr = signal(SIGINT, psabort);
	if (flag) {
		if (proxy)
			return;
		ip = &tmpstruct;
		op = &proxstruct;
		proxy++;
	} 
	else {
		if (!proxy)
			return;
		ip = &proxstruct;
		op = &tmpstruct;
		proxy = 0;
	}
	ip->connect = connected;
	connected = op->connect;
	if (hostname) {
		(void) strlcpy(ip->name, hostname, sizeof(ip->name) - 1);
		ip->name[strlen(ip->name)] = '\0';
	} 
	else {
		ip->name[0] = 0;
	}
	hostname = op->name;
	ip->hctl = hisctladdr;
	hisctladdr = op->hctl;
	ip->mctl = myctladdr;
	myctladdr = op->mctl;
	ip->in = cin;
	cin = op->in;
	ip->out = cout;
	cout = op->out;
	ip->tpe = type;
	type = op->tpe;
	ip->curtpe = curtype;
	curtype = op->curtpe;
	ip->cpnd = cpend;
	cpend = op->cpnd;
	ip->sunqe = sunique;
	sunique = op->sunqe;
	ip->runqe = runique;
	runique = op->runqe;
	ip->mcse = mcase;
	mcase = op->mcse;
	ip->ntflg = ntflag;
	ntflag = op->ntflg;
	(void) strlcpy(ip->nti, ntin, 16);
	(ip->nti)[strlen(ip->nti)] = '\0';
	(void) strcpy(ntin, op->nti);
	(void) strlcpy(ip->nto, ntout, 16);
	(ip->nto)[strlen(ip->nto)] = '\0';
	(void) strcpy(ntout, op->nto);
	ip->mapflg = mapflag;
	mapflag = op->mapflg;
	(void) strlcpy(ip->mi, mapin, MAXPATHLEN - 1);
	(ip->mi)[strlen(ip->mi)] = '\0';
	(void) strcpy(mapin, op->mi);
	(void) strlcpy(ip->mo, mapout, MAXPATHLEN - 1);
	(ip->mo)[strlen(ip->mo)] = '\0';
	(void) strcpy(mapout, op->mo);
	(void) signal(SIGINT, oldintr);
	if (abrtflag) {
		abrtflag = 0;
		(*oldintr)(SIGINT);
	}
}

static
void
abortpt(int ignore)
{
	(void)ignore;
	printf("\n");
	fflush(stdout);
	ptabflg++;
	mflag = 0;
	abrtflag = 0;
	siglongjmp(ptabort, 1);
}

static void
proxtrans(const char *cmd, char *local, char *remote)
{
	void (*volatile oldintr)(int);
	volatile int secndflag = 0, prox_type, nfnd;
	const char *volatile cmd2;
	fd_set mask;

	if (strcmp(cmd, "RETR"))
		cmd2 = "RETR";
	else
		cmd2 = runique ? "STOU" : "STOR";
	if ((prox_type = type) == 0) {
		if (unix_server && unix_proxy)
			prox_type = TYPE_I;
		else
			prox_type = TYPE_A;
	}
	if (curtype != prox_type)
		changetype(prox_type, 1);
	if (command("PASV") != COMPLETE) {
		printf("proxy server does not support third party transfers.\n");
		return;
	}
	pswitch(0);
	if (!connected) {
		printf("No primary connection\n");
		pswitch(1);
		code = -1;
		return;
	}
	if (curtype != prox_type)
		changetype(prox_type, 1);
	if (command("PORT %s", pasv) != COMPLETE) {
		pswitch(1);
		return;
	}
	if (sigsetjmp(ptabort, 1))
		goto abort;
	oldintr = signal(SIGINT, abortpt);
	if (command("%s %s", cmd, remote) != PRELIM) {
		(void) signal(SIGINT, oldintr);
		pswitch(1);
		return;
	}
	sleep(2);
	pswitch(1);
	secndflag++;
	if (command("%s %s", cmd2, local) != PRELIM)
		goto abort;
	ptflag++;
	(void) getreply(0);
	pswitch(0);
	(void) getreply(0);
	(void) signal(SIGINT, oldintr);
	pswitch(1);
	ptflag = 0;
	printf("local: %s remote: %s\n", local, remote);
	return;
abort:
	(void) signal(SIGINT, SIG_IGN);
	ptflag = 0;
	if (strcmp(cmd, "RETR") && !proxy)
		pswitch(1);
	else if (!strcmp(cmd, "RETR") && proxy)
		pswitch(0);
	if (!cpend && !secndflag) {  /* only here if cmd = "STOR" (proxy=1) */
		if (command("%s %s", cmd2, local) != PRELIM) {
			pswitch(0);
			if (cpend)
				abort_remote((FILE *) NULL);
		}
		pswitch(1);
		if (ptabflg)
			code = -1;
		(void) signal(SIGINT, oldintr);
		return;
	}
	if (cpend)
		abort_remote((FILE *) NULL);
	pswitch(!proxy);
	if (!cpend && !secndflag) {  /* only if cmd = "RETR" (proxy=1) */
		if (command("%s %s", cmd2, local) != PRELIM) {
			pswitch(0);
			if (cpend)
				abort_remote((FILE *) NULL);
			pswitch(1);
			if (ptabflg)
				code = -1;
			(void) signal(SIGINT, oldintr);
			return;
		}
	}
	if (cpend)
		abort_remote((FILE *) NULL);
	pswitch(!proxy);
	if (cpend) {
		FD_ZERO(&mask);
		FD_SET(fileno(cin), &mask);
		if ((nfnd = empty(&mask, fileno(cin), 10)) <= 0) {
			if (nfnd < 0) {
				perror("abort");
			}
			if (ptabflg)
				code = -1;
			lostpeer(0);
		}
		(void) getreply(0);
		(void) getreply(0);
	}
	if (proxy)
		pswitch(0);
	pswitch(1);
	if (ptabflg)
		code = -1;
	(void) signal(SIGINT, oldintr);
}

void
reset(void)
{
	fd_set mask;
	int nfnd = 1;

	FD_ZERO(&mask);
	while (nfnd > 0) {
		FD_SET(fileno(cin), &mask);
		if ((nfnd = empty(&mask, fileno(cin), 0)) < 0) {
			perror("reset");
			code = -1;
			lostpeer(0);
		}
		else if (nfnd) {
			(void) getreply(0);
		}
	}
}

static char *
gunique(char *local)
{
	static char new[MAXPATHLEN];
	char *cp = rindex(local, '/');
	int d, count=0;
	char ext = '1';

	if (cp)
		*cp = '\0';
	d = access(cp ? local : ".", W_OK);
	if (cp)
		*cp = '/';
	if (d < 0) {
		fprintf(stderr, "local: %s: %s\n", local, strerror(errno));
		return((char *) 0);
	}
	(void) strcpy(new, local);
	cp = new + strlen(new);
	*cp++ = '.';
	while (!d) {
		if (++count == 100) {
			printf("runique: can't find unique file name.\n");
			return((char *) 0);
		}
		*cp++ = ext;
		*cp = '\0';
		if (ext == '9')
			ext = '0';
		else
			ext++;
		if ((d = access(new, F_OK)) < 0)
			break;
		if (ext != '0')
			cp--;
		else if (*(cp - 2) == '.')
			*(cp - 1) = '1';
		else {
			*(cp - 2) = *(cp - 2) + 1;
			cp--;
		}
	}
	return(new);
}

static void
abort_remote(FILE *din)
{
	char buf[BUFSIZ];
	int nfnd, hifd;
	fd_set mask;

	/*
	 * send IAC in urgent mode instead of DM because 4.3BSD places oob mark
	 * after urgent byte rather than before as is protocol now
	 */
	snprintf(buf, sizeof(buf), "%c%c%c", IAC, IP, IAC);
	if (send(fileno(cout), buf, 3, MSG_OOB) != 3)
		perror("abort");
	fprintf(cout,"%cABOR\r\n", DM);
	(void) fflush(cout);
	FD_ZERO(&mask);
	FD_SET(fileno(cin), &mask);
	hifd = fileno(cin);
	if (din) { 
		FD_SET(fileno(din), &mask);
		if (hifd < fileno(din)) hifd = fileno(din);
	}
	if ((nfnd = empty(&mask, hifd, 10)) <= 0) {
		if (nfnd < 0) {
			perror("abort");
		}
		if (ptabflg)
			code = -1;
		lostpeer(0);
	}
	if (din && FD_ISSET(fileno(din), &mask)) {
		while (read(fileno(din), buf, BUFSIZ) > 0)
			/* LOOP */;
	}
	if (getreply(0) == ERROR && code == 552) {
		/* 552 needed for nic style abort */
		(void) getreply(0);
	}
	(void) getreply(0);
}

/*
 * Ensure that ai->ai_addr is NOT an IPv4 mapped address.
 * IPv4 mapped address complicates too many things in FTP
 * protocol handling, as FTP protocol is defined differently
 * between IPv4 and IPv6.
 *
 * This may not be the best way to handle this situation,
 * since the semantics of IPv4 mapped address is defined in
 * the kernel.  There are configurations where we should use
 * IPv4 mapped address as native IPv6 address, not as
 * "an IPv6 address that embeds IPv4 address" (namely, SIIT).
 *
 * More complete solution would be to have an additional
 * getsockopt to grab "real" peername/sockname.  "real"
 * peername/sockname will be AF_INET if IPv4 mapped address
 * is used to embed IPv4 address, and will be AF_INET6 if
 * we use it as native.  What a mess!
 */
void
ai_unmapped(struct addrinfo *ai)
{
#ifdef IPV6
	struct sockaddr_in6 *sin6;
	struct sockaddr_in sin;
	socklen_t len;

	if (ai->ai_family != AF_INET6)
		return;
	if (ai->ai_addrlen != sizeof(struct sockaddr_in6) ||
	    sizeof(sin) > ai->ai_addrlen)
		return;
	sin6 = (struct sockaddr_in6 *)ai->ai_addr;
	if (!IN6_IS_ADDR_V4MAPPED(&sin6->sin6_addr))
		return;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	len = sizeof(struct sockaddr_in);
	memcpy(&sin.sin_addr, &sin6->sin6_addr.s6_addr[12],
	    sizeof(sin.sin_addr));
	sin.sin_port = sin6->sin6_port;

	ai->ai_family = AF_INET;
	memcpy(ai->ai_addr, &sin, len);
	ai->ai_addrlen = len;
#endif
}


/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

//ql_xu
#include <rtk/options.h>
#include <rtk/utility.h>

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)tftpd.c	5.8 (Berkeley) 6/18/88";
#endif /* not lint */

/*
 * Trivial file transfer protocol server.
 *
 * This version includes many modifications by Jim Guyton <guyton@rand-unix>
 */

#define __USE_BSD 1

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <arpa/tftp.h>

#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
#include <setjmp.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#ifdef EMBED
#include <unistd.h>
#include <linux/autoconf.h>
#include <rtk/sysconfig.h>
#include <rtk/utility.h>
#endif
#include "tftpd.h"

#define RTL_UPGRADE_FW 1

#define	TIMEOUT		5

#define UPD_IMAGE	0
#define UPD_CONF	1
#define UPD_NONE	5

// [2009.05.13]Magician: Max allowed file size for different flash size.
#if defined(CONFIG_4M_FLASH)
	#define MAX_FILE_SIZE 3700000
#elif defined(CONFIG_8M_FLASH)
	#define MAX_FILE_SIZE 7700000
#elif defined(CONFIG_16M_FLASH)
	#define MAX_FILE_SIZE 15700000
#elif defined(CONFIG_32M_FLASH)
	#define MAX_FILE_SIZE 31700000
#else
	#define MAX_FILE_SIZE 2000000
#endif

extern	int errno;
union	sockunion myaddr;
int	peer;
int	rexmtval = TIMEOUT;
int	maxtimeout = 5*TIMEOUT;

#define	PKTSIZE	SEGSIZE+4
char	buf[PKTSIZE];
char	ackbuf[PKTSIZE];
union	sockunion from;
int	fromlen;

//ql_xu add
unsigned int uFileSize=0;
int updateType;

// Brian
//FILE *myfp;
//const char tftpFile[]="/var/tftplog";
// Brian

#ifdef RTL_UPGRADE_FW
// Kaohj -- verify file header for configuration file
static int isConfile(char *buf)
{
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
}

static int tftpUpdateConfig(const char *name)
{
	int ret = -1;

	cmd_file2xml(name, "/tmp/config.xml");
	//ret = call_cmd("/bin/LoadxmlConfig", 0, 1);
	ret = call_cmd("/bin/loadconfig", 0, 1);
	if (!ret) {
		/* upgdate to flash */
		mib_update(CURRENT_SETTING, CONFIG_MIB_ALL);
	}
	printf("Rebooting ...\n");
	cmd_reboot();

	return ret;
}
#endif // of RTL_UPGRADE_FW

// Kaohj -- write pid file
#ifdef EMBED
static char tftpd_pidfile[] = "/var/run/tftpd.pid";
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = tftpd_pidfile;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

static void quit_signal(int sig)
{
	unlink(tftpd_pidfile);
	exit(1);
}
#endif

main()
{
	register struct tftphdr *tp;
	register int n;
	int on = 1;
	// Kaohj
#ifdef EMBED
	FILE *fp;

	// Kaohj --- check daemon
	fp = fopen(tftpd_pidfile, "r");
	if (fp) {
		// Allow only one tftp connection
		fclose(fp);
		exit(0);
	}
#endif

	openlog("tftpd", LOG_PID, LOG_DAEMON);
	ioctl(0, FIONBIO, &on);
/*	if (ioctl(0, FIONBIO, &on) < 0) {
		syslog(LOG_ERR, "ioctl(FIONBIO): %m\n");
		exit(1);
	}
*/
	fromlen = sizeof(from);
	n = recvfrom(0, buf, sizeof (buf), 0, (struct sockaddr *)&from, &fromlen);
	if (n < 0) {
		syslog(LOG_ERR, "recvfrom: %m\n");
		exit(1);
	}

#ifndef RTL_UPGRADE_FW
	/*
	 * Now that we have read the message out of the UDP
	 * socket, we fork and exit.  Thus, inetd will go back
	 * to listening to the tftp port, and the next request
	 * to come in will start up a new instance of tftpd.
	 *
	 * We do this so that inetd can run tftpd in "wait" mode.
	 * The problem with tftpd running in "nowait" mode is that
	 * inetd may get one or more successful "selects" on the
	 * tftp port before we do our receive, so more than one
	 * instance of tftpd may be started up.  Worse, if tftpd
	 * break before doing the above "recvfrom", inetd would
	 * spawn endless instances, clogging the system.
	 */
	{
		int pid;
		int i, j;

		for (i = 1; i < 20; i++) {
			pid = fork();
			if (pid < 0) {
				sleep(i);
				/*
				* flush out to most recently sent request.
				*
				* This may drop some request, but those
				* will be resent by the clients when
				* they timeout.  The positive effect of
				* this flush is to (try to) prevent more
				* than one tftpd being started up to service
				* a single request from a single client.
				*/
				j = sizeof(from);
				i = recvfrom(0, buf, sizeof (buf), 0,	(struct sockaddr *)&from, &j);
				if (i > 0) {
					n = i;
					fromlen = j;
				}
			} else {
				break;
			}
		}
		if (pid < 0) {
			syslog(LOG_ERR, "fork: %m\n");
			exit(1);
		} else if (pid != 0) {
			exit(0);
		}
	}
#endif

#ifdef RTL_UPGRADE_FW   //brian
	//myfp = fopen (tftpFile, "w+");  //brian
	//fprintf(myfp, "tftp\n");
	//fflush(myfp);
#endif
	alarm(0);
	close(0);
	close(1);
	peer = socket(from.su_family, SOCK_DGRAM, 0);
	if (peer < 0) {
		syslog(LOG_ERR, "socket: %m\n");
		exit(1);
	}
	memset(&myaddr, 0, sizeof(myaddr));
	myaddr.su_family = from.su_family;
	if (bind(peer, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		syslog(LOG_ERR, "bind: %m\n");
		exit(1);
	}
	if (connect(peer, (struct sockaddr *)&from, sizeof(from)) < 0) {
		syslog(LOG_ERR, "connect: %m\n");
		exit(1);
	}
	tp = (struct tftphdr *)buf;
	tp->th_opcode = ntohs(tp->th_opcode);
	if (tp->th_opcode == RRQ || tp->th_opcode == WRQ) {
		//ql_xu -- get write size
		//search "tsize"
		if (tp->th_opcode == WRQ) {
			int i;
			char filesize[20];
			FILE *fp;

			//test
			//fp = fopen("/tmp/tftpd_log", "w");
			//printf("tftpd(0): receive size=%d\n", n);
			//printf("tftpd(0.0): buf=%s\n", buf);
			//fflush(NULL);

			for (i=0; i<n; i++)
				if (!strcmp(buf + i, "tsize"))
					break;

			if (i < n) {
				memcpy(filesize, buf+i+6, n-i-6);
				sscanf(filesize, "%u", &uFileSize);
			}
		}
		tftp(tp, n);
	}
	exit(1);
}

int	validate_access();
int	sendfile(), recvfile();

struct formats {
	char	*f_mode;
	int	(*f_validate)();
	int	(*f_send)();
	int	(*f_recv)();
	int	f_convert;
} formats[] = {
	{ "netascii",	validate_access,	sendfile,	recvfile, 1 },
	{ "octet",	validate_access,	sendfile,	recvfile, 0 },
#ifdef notdef
	{ "mail",	validate_user,		sendmail,	recvmail, 1 },
#endif
	{ 0 }
};

/*
 * Handle initial connection protocol.
 */
tftp(tp, size)
	struct tftphdr *tp;
	int size;
{
	register char *cp;
	int first = 1, ecode;
	register struct formats *pf;
	char *filename, *mode;

#ifdef EMBED
	signal (SIGTERM, quit_signal);
	log_pid();
#endif
	filename = cp = tp->th_stuff;
again:
	while (cp < buf + size) {
		if (*cp == '\0')
			break;
		cp++;
	}
	if (*cp != '\0') {
		nak(EBADOP);
		// Kaohj
#ifdef EMBED
		unlink(tftpd_pidfile);
#endif
		exit(1);
	}
	if (first) {
		mode = ++cp;
		first = 0;
		goto again;
	}
	for (cp = mode; *cp; cp++)
		if (isupper(*cp))
			*cp = tolower(*cp);
	for (pf = formats; pf->f_mode; pf++)
		if (strcmp(pf->f_mode, mode) == 0)
			break;
	if (pf->f_mode == 0) {
		nak(EBADOP);
		// Kaohj
#ifdef EMBED
		unlink(tftpd_pidfile);
#endif
		exit(1);
	}
	ecode = (*pf->f_validate)(filename, tp->th_opcode);
	if (ecode) {
		nak(ecode);
		// Kaohj
#ifdef EMBED
		unlink(tftpd_pidfile);
#endif
		exit(1);
	}
	if (tp->th_opcode == WRQ) {
		(*pf->f_recv)(pf);
	} else
		(*pf->f_send)(pf);
	// Kaohj
#ifdef EMBED
	unlink(tftpd_pidfile);
#endif
	exit(0);
}


FILE *file;
char pathFilename[32];//brian
// Kaohj
const char *TFTP_ROOT = "/tmp";
int filesize=0;//brian
//int do_upgrade;
#ifdef ENABLE_SIGNATURE_ADV
int upgrade=0;
#endif

#ifdef RTL_UPGRADE_FW   //brian
#define UPLOAD_PREFIX		"vm"

void tftpUpdateImage(const char *name, int len)
{
//   fprintf(myfp, "tftpUpdateImage: name=%s len=%d\n", name, len);
//   fflush(myfp);
	FILE *fp;
	struct stat st;

#ifndef CONFIG_LUNA
	if (cmd_check_image(name, 0)) {
#endif
		if ((fp = fopen(name, "rb")) == NULL) {
			printf("File %s open fail\n", name);
			return;
		}

		if (fstat(fileno(fp), &st) < 0) {
			printf("File %s get status fail\n", name);
			fclose(fp);
			return;	
		}		
	
		if (st.st_size <= 0) {
			printf("File %s size error\n", name);
			fclose(fp);
			return;
		}
		fclose(fp);		
		cmd_upload(name, 0, st.st_size);
#ifndef CONFIG_LUNA
	}
#endif
}

void tftpKillProcess()
{
   //fprintf(myfp, "tftpKillProcess\n");
   //fflush(myfp);
   cmd_killproc(ALL_PID & ~(1<<PID_TFTPD));
}
#endif

/*
 * Validate file access.  Since we
 * have no uid or gid, for now require
 * file to exist and be publicly
 * readable/writable.
 * Note also, full path name must be
 * given as we have no login directory.
 */
validate_access(filename, mode)
	char *filename;
	int mode;
{
	struct stat stbuf;
	int	fd;
	int	k, len;

// Comment by Mason Yu
#if 0
#ifndef CONFIG_NETtel
	if (*filename != '/')
		return (EACCESS);
	if (stat(filename, &stbuf) < 0)
		return (errno == ENOENT ? ENOTFOUND : EACCESS);
	if (mode == RRQ) {
		if ((stbuf.st_mode&(S_IREAD >> 6)) == 0)
			return (EACCESS);
	} else {
		if ((stbuf.st_mode&(S_IWRITE >> 6)) == 0)
			return (EACCESS);
	}
#endif /*!CONFIG_NETTEL*/
	fd = open(filename, mode == RRQ ? 0 : 1);
	if (fd < 0)
		return (errno + 100);
	file = fdopen(fd, (mode == RRQ)? "r":"w");
	if (file == NULL) {
		return errno+100;
	}
#endif
	// Added by Mason Yu
	if (*filename != '/')
		snprintf(pathFilename, 31, "%s/%s", TFTP_ROOT, filename);
	else
		snprintf(pathFilename, 31, "%s", filename);

	len = strlen(pathFilename);
	k = len-1;
	while (k >= 0) {
		if (pathFilename[k] == '/') {
			k++;
			break;
		}
		else
			k--;
	}
	if (k < 0)
		k = 0;
	//if (!strncmp(&pathFilename[k], UPLOAD_PREFIX, strlen(UPLOAD_PREFIX)))
	//	do_upgrade = 1;
	//else
	//	do_upgrade = 0;
#ifdef CONFIG_USER_NFBI_MASTER
	file = fopen(pathFilename, (mode == RRQ)? "r":"w+");
#else /*CONFIG_USER_NFBI_MASTER*/
	file = fopen(pathFilename, (mode == RRQ)? "r+":"w+");
#endif /*CONFIG_USER_NFBI_MASTER*/
	if (file == NULL)
		return errno+100;

	return (0);
}

int	timeout;
jmp_buf	timeoutbuf;

void
timer(int signum)
{

	timeout += rexmtval;
	#ifdef CONFIG_USER_WLAN_QCSAPI
	//exit program directly if timer timeout
	#ifdef EMBED
        unlink(tftpd_pidfile);
        #endif
        exit(1);
	#else
	if (timeout >= maxtimeout)
	{
		#ifdef EMBED
	        unlink(tftpd_pidfile);
		#endif
		exit(1);
	}
	#endif
	longjmp(timeoutbuf, 1);
}

/*
 * Send the requested file.
 */
sendfile(pf)
	struct formats *pf;
{
	struct tftphdr *dp, *r_init();
	register struct tftphdr *ap;    /* ack packet */
	register int block = 1, size, n;

	signal(SIGALRM, timer);
	dp = r_init();
	ap = (struct tftphdr *)ackbuf;
	do {
		size = readit(file, &dp, pf->f_convert);
		if (size < 0) {
			nak(errno + 100);
			goto abort;
		}
		dp->th_opcode = htons((u_short)DATA);
		dp->th_block = htons((u_short)block);
		timeout = 0;
		(void) setjmp(timeoutbuf);

send_data:
		if (send(peer, dp, size + 4, 0) != size + 4) {
			syslog(LOG_ERR, "tftpd: write: %m\n");
			goto abort;
		}
		read_ahead(file, pf->f_convert);
		for ( ; ; ) {
			alarm(rexmtval);        /* read the ack */
			n = recv(peer, ackbuf, sizeof (ackbuf), 0);
			alarm(0);
			if (n < 0) {
				syslog(LOG_ERR, "tftpd: read: %m\n");
				goto abort;
			}
			ap->th_opcode = ntohs((u_short)ap->th_opcode);
			ap->th_block = ntohs((u_short)ap->th_block);

			if (ap->th_opcode == ERROR)
				goto abort;

			if (ap->th_opcode == ACK) {
				if (ap->th_block == block) {
					break;
				}
				/* Re-synchronize with the other side */
				(void) synchnet(peer);
				if (ap->th_block == (block -1)) {
					goto send_data;
				}
			}

		}
		block++;
	} while (size == SEGSIZE);
abort:
	(void) fclose(file);
}

void
justquit(int signum)
{
#ifdef EMBED
	unlink(tftpd_pidfile);
#endif
	exit(0);
}

/*
 * Receive a file.
 */
recvfile(pf)
	struct formats *pf;
{
	struct tftphdr *dp, *w_init();
	register struct tftphdr *ap;    /* ack buffer */
	register int block = 0, n, size;
#ifdef ENABLE_SIGNATURE_ADV
	IMGHDR imgHdr;
	SIGHDR sigHdr;
	unsigned int hdrChksum;
	unsigned int i=0;
#endif
	/*ql: 20080729 START: check image key according to IC version*/
#ifdef MULTI_IC_SUPPORT
	int key;
#endif
	/*ql: 20080729 END*/


#ifdef RTL_UPGRADE_FW   //brian
	//fprintf(myfp, "tftp: pathFilename=%s\n", pathFilename);
	//fflush(myfp);
	//ql commented: if signature error, system should work well
//#ifndef ENABLE_SIGNATURE_ADV
//	if (do_upgrade)
//		tftpKillProcess();
//#endif
#endif
	filesize=0;
	signal(SIGALRM, timer);
	dp = w_init();
	ap = (struct tftphdr *)ackbuf;
	do {
		timeout = 0;
		ap->th_opcode = htons((u_short)ACK);
		ap->th_block = htons((u_short)block);
		block++;
		(void) setjmp(timeoutbuf);
send_ack:
		if (send(peer, ackbuf, 4, 0) != 4) {
			syslog(LOG_ERR, "tftpd: write: %m\n");
			goto abort;
		}
		write_behind(file, pf->f_convert);

#ifdef RTL_UPGRADE_FW
		//ql--when upgrade the image, if received over 300 bytes, check the signature.
		//if signature OK, then kill some process to save memory, otherwise stop upgrade.
#ifdef ENABLE_SIGNATURE_ADV
		if (/*do_upgrade && */!upgrade) {
			if (filesize > 300) {//check signature
				fseek(file, 0, SEEK_SET);
#ifdef ENABLE_SIGNATURE
				if (1 != fread(&sigHdr, sizeof(sigHdr), 1, file)) {
					goto FAIL;
				}
#endif
				if (1 != fread(&imgHdr, sizeof(imgHdr), 1, file)) {
					goto FAIL;
				}
				fseek(file, 0, SEEK_END);

#ifdef ENABLE_SIGNATURE
				if (sigHdr.sigLen > SIG_LEN) {
					goto FAIL;
				}
				for (i=0; i<sigHdr.sigLen; i++)
					sigHdr.sigStr[i] = sigHdr.sigStr[i] - 10;
				if (strcmp(sigHdr.sigStr, SIGNATURE)) {
					goto FAIL;
				}

				hdrChksum = sigHdr.chksum;
				hdrChksum = ipchksum(&imgHdr, sizeof(imgHdr), hdrChksum);
				if (hdrChksum) {
					goto FAIL;
				}
#else
				// Kaohj
				updateType = UPD_NONE;
				if (isConfile((char *)&imgHdr))
					updateType = UPD_CONF;
				/*ql:20080729 START: different ic matched with different image key*/
#ifdef MULTI_IC_SUPPORT
				printf("recvfile(0):\n");
				key = getImgKey();

				if ((key == (imgHdr.key & key)) && (((imgHdr.key>>28)&0xf) == ((key>>28)&0xf)))
					updateType = UPD_IMAGE;
#else
				if (imgHdr.key == APPLICATION_IMAGE)
				{
					updateType = UPD_IMAGE;
				}
#endif
				/*ql:20080729 END*/
				if (updateType == UPD_NONE)
					goto FAIL;
				//if (imgHdr.key != APPLICATION_IMAGE) {
				//	goto FAIL;
				//}
#endif
				upgrade = 2;	//check OK
				tftpKillProcess();
				goto SUCC;
		FAIL:
				upgrade = 1;
				break;
		SUCC:
				;
			}
		}
#endif
#endif // of RTL_UPGRADE_FW

		for ( ; ; ) {
			alarm(rexmtval);
			n = recv(peer, dp, PKTSIZE, 0);
			alarm(0);
			if (n < 0) {            /* really? */
				syslog(LOG_ERR, "tftpd: read: %m\n");
				goto abort;
			}
			dp->th_opcode = ntohs((u_short)dp->th_opcode);
			dp->th_block = ntohs((u_short)dp->th_block);
			if (dp->th_opcode == ERROR)
				goto abort;
			if (dp->th_opcode == DATA) {
				if (dp->th_block == block) {
					break;   /* normal */
				}
				/* Re-synchronize with the other side */
				(void) synchnet(peer);
				if (dp->th_block == (block-1))
					goto send_ack;          /* rexmit */
			}
		}
		/*  size = write(file, dp->th_data, n - 4); */
		size = writeit(file, &dp, n - 4, pf->f_convert);
		if (size != (n-4)) {                    /* ahem */
			if (size < 0) nak(errno + 100);
			else nak(ENOSPACE);
			goto abort;
		}
		filesize +=size;

	// Kaohj -- check MAX_FILE_SIZE
	//} while (size == SEGSIZE);
	//ql-- check image size
	}	while ( (uFileSize?(filesize < uFileSize):1) &&
						(!uFileSize?(size == SEGSIZE):1) &&
						(filesize < MAX_FILE_SIZE));

	write_behind(file, pf->f_convert);
	(void) fclose(file);            /* close data file */

	ap->th_opcode = htons((u_short)ACK);    /* send the "final" ack */
	ap->th_block = htons((u_short)(block));
	(void) send(peer, ackbuf, 4, 0);

#ifdef RTL_UPGRADE_FW  //brian
	//fprintf(myfp, "pathFilename=%s filesize(%d)\n", pathFilename, filesize);
	//fflush(myfp);
//#ifndef ENABLE_SIGNATURE_ADV
//	if (do_upgrade)
//#else
	printf("recvfile(0): updateType=%d\n", updateType);
#ifdef ENABLE_SIGNATURE_ADV
	if (/*do_upgrade && */upgrade==2)
#endif
//#endif
		// Kaohj
	{
  		if (updateType == UPD_CONF) {
  			tftpUpdateConfig(pathFilename);
  		}
  		else if (updateType == UPD_IMAGE) {
			tftpUpdateImage(pathFilename, filesize);
		}
	}
#endif
	signal(SIGALRM, justquit);      /* just quit on timeout */
	alarm(rexmtval);
	n = recv(peer, buf, sizeof (buf), 0); /* normally times out and quits */
	alarm(0);
	if (n >= 4 &&                   /* if read some data */
	    dp->th_opcode == DATA &&    /* and got a data block */
	    block == dp->th_block) {	/* then my last ack was lost */
		(void) send(peer, ackbuf, 4, 0);     /* resend final ack */
	}
abort:
	return;
}

struct errmsg {
	int		e_code;
	const char	*e_msg;
} errmsgs[] = {
	{ EUNDEF,	"Undefined error code" },
	{ ENOTFOUND,	"File not found" },
	{ EACCESS,	"Access violation" },
	{ ENOSPACE,	"Disk full or allocation exceeded" },
	{ EBADOP,	"Illegal TFTP operation" },
	{ EBADID,	"Unknown transfer ID" },
	{ EEXISTS,	"File already exists" },
	{ ENOUSER,	"No such user" },
	{ -1,		0 }
};

/*
 * Send a nak packet (error message).
 * Error code passed in is one of the
 * standard TFTP codes, or a UNIX errno
 * offset by 100.
 */
nak(error)
	int error;
{
	register struct tftphdr *tp;
	int length;
	register struct errmsg *pe;

	tp = (struct tftphdr *)buf;
	tp->th_opcode = htons((u_short)ERROR);
	tp->th_code = htons((u_short)error);
	for (pe = errmsgs; pe->e_code >= 0; pe++)
		if (pe->e_code == error)
			break;
	if (pe->e_code < 0) {
		#ifdef CONFIG_UCLINUX
		pe->e_msg = sys_errlist[error - 100];
		#else	// mmu
		//11/08/05' hrchen, modify to fit in uClibc
		pe->e_msg = strerror(error - 100);
		#endif
		tp->th_code = EUNDEF;   /* set 'undef' errorcode */
	}
	strcpy(tp->th_msg, pe->e_msg);
	length = strlen(pe->e_msg);
	tp->th_msg[length] = '\0';
	length += 5;
	if (send(peer, buf, length, 0) != length)
		syslog(LOG_ERR, "nak: %m\n");
}

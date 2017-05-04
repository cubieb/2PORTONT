/* Synchronize the time periodically. */
/*
   Copyright (C) 2003-2004 imacat.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   版權所有 (c) 2003-2004 依瑪貓。

   本程式是自由軟體，您可以遵照自由軟體基金會（ Free Software Foundation
   ）出版的 GNU 通用公共許可證條款（ GNU General Public License ）第二版
   來修改和重新發佈這一程式，或者自由選擇使用任何更新的版本。

   發佈這一程式的目的是希望它有用，但沒有任何擔保。甚至沒有適合特定目的
   而隱含的擔保。更詳細的情況請參閱 GNU 通用公共許可證。

   您應該已經和程式一起收到一份 GNU 通用公共許可證的副本。如果還沒有，寫
   信給： Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

/* Filename:	synctime.c
   Description:	Synchronize the time periodically
   Author:	imacat <imacat@mail.imacat.idv.tw>
   Date:	2003-12-22
   Copyright:	(c) 2003 imacat */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* Headers */
#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/atm.h>
#include <net/if.h>
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#include <rtk/options.h>
/*ping_zhang:20081217 END*/
extern int h_errno;
#ifdef EMBED
#include "../../config/autoconf.h"
#endif
/* Configuration */
#define AUTHOR "Realtek"
#define AUTHORMAIL "imacat@mail.imacat.idv.tw"
#define EXEC_USER "root"
#define PIDDIR "/var/run"
#define DEFAULT_INTERVAL 900
#define MAX_NETWORK_ERROR 1200
#define SNTP_PORT 123
#define VERSION "1.0"


// Commented by Mason Yu
#if 0
#define VERSTR "\
%s v%s, Copyright (C) 2003-2004 %s\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
\n"
#endif

#define VERSTR "\
%s v%s, Copyright (C) 2004-2005 %s\n\
\n"

#define LONGHELP "\
Usage: %s [-i n] [-d ifname] [-p file] server\n\
Synchronize the system time against a time server periodically.  It\n\
uses RFC 1361 SNTP service UDP port 123 to synchronize the time.  You\n\
may need to be root to synchronize the system time.\n\
\n\
  -i,--interval n    Set the synchronization interval to n seconds. (%d)\n\
  -d,--device ifname Set the interface name used to synchronize. (%s)\n\
  -p,--pidfile file  The PID file location. (%s)\n\
  -h,--help          Display this help.\n\
  -v,--version       Display version information.\n\
  server             Time server to synchronize against.\n\
\n"

#define EXIT_OK 0
#define EXIT_ARGERR 1
#define EXIT_NETERR 2
#define EXIT_SYSERR 127

#define EPOCH_DIFF ((unsigned long) 86400 * (365 * 70 + 17))
#define syslog(a,format,argv...) printf(format,##argv)
/* Prototypes */
void set_this_file (char *argv0);
void parse_args (int argc, char *argv[]);
void check_priv (void);
void xexit (int status)
    __attribute__((noreturn));
void verror (int status, char *message, va_list ap)
    __attribute__((noreturn));
void error (int status, char *message, ...)
    __attribute__((noreturn, format(printf, 2, 3)));
void vwarn (char *message, va_list ap);
void warn (char *message, ...)
    __attribute__((format(printf, 1, 2)));
void neterror (int firstsync, time_t errstart, char *message, ...)
    __attribute__((format(printf, 3, 4)));
void sigterm_exit (int signum)
    __attribute__((noreturn));
void sigusr1(int);
void daemonize (void);
void makepid (void);
void setsigterm (void);
// Kaohj -- let user process kick to sync the time
void setsigusr1 (void);
#if 0
double synctime (void);
#else
int synctime (void);
#endif
/*ping_zhang:20081223 START:add to make each ntp server is used.*/
#ifdef SNTP_MULTI_SERVER
void init_server_used(void);
unsigned int check_server_all_used(void);
#endif
/*ping_zhang:20081223 END*/

void fork2background (void);
void closeall (void);
void settime (time_t t);
unsigned long fromnetnum (const char *oct);
const char *tonetnum (unsigned long num);
unsigned long usec2frac (long usec);
long frac2usec (unsigned long frac);

void *xmalloc (size_t size);
char *xstrcpy (const char *src);
char *xstrcat (int n, ...);
time_t xtime (time_t *t);
void xgettimeofday (struct timeval *tv, struct timezone *tz);
void xsettimeofday (const struct timeval *tv , const struct timezone *tz);
void xchdir (const char *path);
pid_t xfork (void);
pid_t xsetsid (void);
long xsysconf (int name, const char *confname);
FILE *xfopen (const char *path, const char *mode);
void xfclose (FILE *stream);
int xfprintf (FILE *stream, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

/* Variables */
char *this_file = NULL, *pidfile = NULL;
#ifdef CONFIG_BOA_WEB_E8B_CH
#define SNTP_MULTI_SERVER
#endif
#ifdef SNTP_MULTI_SERVER
#define MAX_MUTLI_SERVER_NUM 2
char *server_name[MAX_MUTLI_SERVER_NUM] = {0};
/*ping_zhang:20081223 START:add to make each ntp server is used.*/
unsigned int server_used[MAX_MUTLI_SERVER_NUM] = {0};
/*ping_zhang:20081223 END*/
int udp;
#else
char *server_name = NULL;
#endif
int interval = -1, daemonized = 0;
char *interface = NULL;
struct sockaddr_in server;
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
enum eTStatus
{
	eTStatusDisabled,
	eTStatusUnsynchronized,
	eTStatusSynchronized,
	eTStatusErrorFailed,/*Error_FailedToSynchronize*/
	eTStatusError
};
#endif
/*ping_zhang:20081217 END*/

/* set_this_file: Get the name of this file
     return: none. */
void
set_this_file (char *argv0)
{
  char *p;
  p = rindex (argv0, '/');
  if (p == NULL)
    this_file = xstrcpy (argv0);
  else
    this_file = xstrcpy (++p);
  return;
}

// Kaohj
// Return: 0 -> fail
//         1 -> successful
#ifdef SNTP_MULTI_SERVER
static int get_server(const char* servername)
#else
static int get_server()
#endif
{
	int r;
	struct in_addr server_addr;
	struct hostent *server_hostent;
	#if 0
	void *p;
	size_t len;
	#endif
#ifdef SNTP_MULTI_SERVER
	r = inet_aton (servername, &server_addr);
	if (r == 0)
	{
		/* Try DNS look up */
		server_hostent = gethostbyname (servername);
		if (server_hostent == NULL)
		{
			//error (EXIT_ARGERR, "%s: %s.", server_name, hstrerror (h_errno));
			printf ("%s: %s.", servername, hstrerror (h_errno));
			return 0;
		}
		/* Obtain the IP number, reverse the byte order */
		#if 0
		server_addr.s_addr =
		    ((server_hostent->h_addr_list[0][3] << 24) & 0xFF000000) |
		    ((server_hostent->h_addr_list[0][2] << 16) & 0x00FF0000) |
		    ((server_hostent->h_addr_list[0][1] << 8)  & 0x0000FF00) |
		    (server_hostent->h_addr_list[0][0]         & 0x000000FF);
		#endif
		server_addr.s_addr = *((unsigned long *)server_hostent->h_addr);
		#if 0
		/* Modify the server name for logging */
		len = strlen (servername) + 20;
		p = (void *) servername;
		servername = (char *) xmalloc (len);
		snprintf ((char *)servername, len, "%s (%hhu.%hhu.%hhu.%hhu)", (char *) p,
			server_hostent->h_addr_list[0][0],
			server_hostent->h_addr_list[0][1],
			server_hostent->h_addr_list[0][2],
			server_hostent->h_addr_list[0][3]);
		free (p);
		#endif
	}
#else
	r = inet_aton (server_name, &server_addr);
	if (r == 0)
	{
		/* Try DNS look up */
		server_hostent = gethostbyname (server_name);
		if (server_hostent == NULL)
		{
			//error (EXIT_ARGERR, "%s: %s.", server_name, hstrerror (h_errno));
			printf ("%s: %s.", server_name, hstrerror (h_errno));
			return 0;
		}
		/* Obtain the IP number, reverse the byte order */
		#if 0
		server_addr.s_addr =
		    ((server_hostent->h_addr_list[0][3] << 24) & 0xFF000000) |
		    ((server_hostent->h_addr_list[0][2] << 16) & 0x00FF0000) |
		    ((server_hostent->h_addr_list[0][1] << 8)  & 0x0000FF00) |
		    (server_hostent->h_addr_list[0][0]         & 0x000000FF);
		#endif
		server_addr.s_addr = *((unsigned long *)server_hostent->h_addr);
		/* Modify the server name for logging */
		#if 0
		len = strlen (server_name) + 20;
		p = (void *) server_name;
		server_name = (char *) xmalloc (len);
		snprintf (server_name, len, "%s (%hhu.%hhu.%hhu.%hhu)", (char *) p,
			server_hostent->h_addr_list[0][0],
			server_hostent->h_addr_list[0][1],
			server_hostent->h_addr_list[0][2],
			server_hostent->h_addr_list[0][3]);
		free (p);
		#endif
	}
#endif
	/* Save the server infomation */
	server.sin_family = AF_INET;
	server.sin_addr = server_addr;
	server.sin_port = htons (SNTP_PORT);
	return 1;
}

/* parse_args: Parse the arguments.
     return: none. */
void
parse_args (int argc, char *argv[])
{
  static struct option longopts[] = {
    {"interval", 1, NULL, 'i'},
    {"device", 1, NULL, 'd'},
    {"pidfile", 1, NULL, 'p'},
    {"help", 0, NULL, 'h'},
    {"version", 0, NULL, 'v'},
  #ifdef SNTP_MULTI_SERVER
   {"server",1,NULL,'s'},
  #endif
    {0, 0, 0, 0}
  };
  int i, r, c, c0, longindex = 0;
// Kaohj
#if 0
  struct in_addr server_addr;
  struct hostent *server_hostent;
  void *p;
  size_t len;
#endif

  /* Set the default value */
  interval = DEFAULT_INTERVAL;
  interface = NULL;
  pidfile = xstrcat (4, PIDDIR, "/", this_file, ".pid");

  while (1)
   {
  #ifdef SNTP_MULTI_SERVER
    c = getopt_long (argc, argv, "i:d:p:s:hv", longopts, &longindex);
  #else
    c = getopt_long (argc, argv, "i:d:p:hv", longopts, &longindex);
  #endif
    if (c == -1)
      break;

    switch (c)
     {
      case 'i':
        r = sscanf (optarg, "%5d%c", &interval, &c0);
        if (r != 1 || interval <= 0)
            error (EXIT_ARGERR, "invalid interval: %s.", optarg);
        break;

      case 'd':
		if(interface)
            free(interface);
        interface = xstrcpy (optarg);
        break;

      case 'p':
        if (pidfile != NULL)
          free (pidfile);
      	pidfile = xstrcpy (optarg);
      	break;

      case 'h':
        printf (LONGHELP, this_file, interval, pidfile);
        exit (EXIT_OK);

      case 'v':
        // Commened by Mason Yu
        printf (VERSTR, this_file, VERSION, AUTHOR);
        exit (EXIT_OK);
#ifdef SNTP_MULTI_SERVER
      case 's':
	  	for(i=0;i<MAX_MUTLI_SERVER_NUM;i++)
	  	{
	  	if(!server_name[i])
			{
			server_name[i]= xstrcpy (optarg);
			break;
	  		}
	  	}
		break;
#endif
      default:
        exit (EXIT_ARGERR);
     }
   }
#ifndef SNTP_MULTI_SERVER
  /* Process each argument */
  for (i = optind; i < argc; i++)
    switch (i - optind)
     {
      case 0:
        server_name = xstrcpy (argv[i]);
        break;
      default:
        error (EXIT_ARGERR, "Too many argument: %s.", argv[i]);
     }
  #endif
  /* Process the interval */
  if (interval == -1)
    interval = DEFAULT_INTERVAL;

  /* Process the PID file */
  /* Compose the default PID file path */
  if (pidfile == NULL)
    pidfile = xstrcat (4, PIDDIR, "/", this_file, ".pid");

  // Mason Yu. Kill process in real time.
  /* Make the PID file */
  //makepid ();

#ifndef SNTP_MULTI_SERVER
  /* Process the time server */
  if (server_name == NULL)
    error (EXIT_ARGERR, "Please specify the time server.");

  // Kaohj
  r = get_server();
 #endif
  #if 0
  r = inet_aton (server_name, &server_addr);
  if (r == 0)
   {
    /* Try DNS look up */
    server_hostent = gethostbyname (server_name);
    if (server_hostent == NULL)
    {
      //error (EXIT_ARGERR, "%s: %s.", server_name, hstrerror (h_errno));
      printf ("%s: %s.", server_name, hstrerror (h_errno));
      return;
    }
    /* Obtain the IP number, reverse the byte order */
    // Kaohj
    #if 0
    server_addr.s_addr =
        ((server_hostent->h_addr_list[0][3] << 24) & 0xFF000000) |
        ((server_hostent->h_addr_list[0][2] << 16) & 0x00FF0000) |
        ((server_hostent->h_addr_list[0][1] << 8)  & 0x0000FF00) |
        (server_hostent->h_addr_list[0][0]         & 0x000000FF);
    #endif
    server_addr.s_addr = *((unsigned long *)server_hostent->h_addr);
    /* Modify the server name for logging */
    len = strlen (server_name) + 20;
    p = (void *) server_name;
    server_name = (char *) xmalloc (len);
    snprintf (server_name, len, "%s (%hhu.%hhu.%hhu.%hhu)", (char *) p,
        server_hostent->h_addr_list[0][0],
        server_hostent->h_addr_list[0][1],
        server_hostent->h_addr_list[0][2],
        server_hostent->h_addr_list[0][3]);
    free (p);
   }
  /* Save the server infomation */
  server.sin_family = AF_INET;
  server.sin_addr = server_addr;
  server.sin_port = htons (SNTP_PORT);
  #endif

  return;
}

/* xexit: Properly handle the exit.
     return: none. */
void
xexit (int status)
{
  int r;

  /* Proper exit in daemon mode */
  if (daemonized)
   {
    /* Remove the PID file */
    r = unlink (pidfile);
    if (r == -1)
      syslog (LOG_ERR, "unlink %s: %s at %s line %d.",
        pidfile, strerror (errno), __FILE__, __LINE__);
    /* Close the syslog */
    closelog ();
   }

  // Mason Yu. Kill process in real time.
  if(pidfile) {
  	unlink(pidfile);
  }
	// free memory
#ifdef SNTP_MULTI_SERVER
	for(r=0; r<MAX_MUTLI_SERVER_NUM; r++)
	{
		if(server_name[r])
			free(server_name[r]);
	}
#else
	if (server_name)
		free(server_name);
#endif
  exit (status);
}

/* verror: Issue an error with variable argument list */
void
verror (int status, char *message, va_list ap)
{
  /* Issue the error message */
  if (daemonized)
   {
    vsyslog (LOG_ERR, message, ap);
    syslog (LOG_ERR, "Exited upon unrecoverable network error.");
   }
  else
   {
    vfprintf (stderr, message, ap);
    fprintf (stderr, "\n");
   }

  xexit (status);
}

/* error: Issue an error */
void
error (int status, char *message, ...)
{
  va_list ap;

  /* Handle errors with verror */
  va_start (ap, message);
  verror (status, message, ap);
  va_end (ap);

  /* No return */
}

/* vwarn: Issue a warning with variable argument list */
void
vwarn (char *message, va_list ap)
{
  /* Issue the warning message */
  if (daemonized)
    vsyslog (LOG_WARNING, message, ap);
  else
    vfprintf (stderr, message, ap);

  return;
}

/* warn: Issue a warning */
void
warn (char *message, ...)
{
  va_list ap;

  /* Handle warnings with vwarn */
  va_start (ap, message);
  vwarn (message, ap);
  va_end (ap);

  return;
}

/* neterror: Issue a network error */
void
neterror (int firstsync, time_t errstart, char *message, ...)
{
  va_list ap;
  time_t now;

  va_start (ap, message);

  /* Don't pass it if we can't even synchronize the first time */
  if (firstsync)
    verror (EXIT_NETERR, message, ap);

  /* Warn it */
  vwarn (message, ap);
  now = xtime (NULL);
  /* First error */
  if (errstart == 0)
    errstart = now;
  /* Errors lasted for too long */
  else if (now - errstart > MAX_NETWORK_ERROR)
    error (EXIT_NETERR, "Exited upon network error exceeding %d seconds.", MAX_NETWORK_ERROR);

  return;
}

/* sigterm: End the program */
void
sigterm_exit (int signum)
{
  /* Log the exit */
  syslog (LOG_INFO, "Exited upon TERM signal.\n");
  // Mason Yu. Kill process in real time.
  //jim remove pid file
  #if 0
  if(pidfile) {
  	unlink(pidfile);
  }
  printf("%s: %s removed.\n", __FUNCTION__, pidfile);
  #endif

  /* Exit normally */
  xexit (EXIT_OK);
}

void sigusr1(int dummy)

{
	syslog(LOG_INFO, "vsntp: Kicked to wake-up by SIGUSR1\n");
}
/* daemonize: Daemonize the process */
/* http://www.erlenstar.demon.co.uk/unix/faq_2.html#SEC16 */
void
daemonize (void)
{
  /* Fork to be a new process group leader */
  fork2background ();
  /* Become a new session group leader, to get rid of the
     controlling terminal */
  xsetsid ();
  /* Fork again to get rid of this new session */
  fork2background ();
  /* chdir to "/", to avoid staying on any mounted file system */
  xchdir ("/");
  /* Avoid inheriting umasks */
  umask (0);
  /* Close all opened file descriptors */
  closeall ();

  /* Set the flag */
  daemonized = 1;

  /* Start the syslog */
  openlog (this_file, LOG_PID, LOG_DAEMON);
  /* Log the start */
  syslog (LOG_INFO, "Start synchronization with %s.", server_name);

  return;
}

/* makepid: Make the PID file */
void
makepid (void)
{
  pid_t pid;
  FILE *fp;
  int skfd;
  struct atmif_sioc mysio;
  // Mason Yu. Kill process in real time.
  int pid_old;
  FILE *fp2;

  /* Record the PID */
  pid = getpid ();

#ifdef CONFIG_DEV_xDSL
  if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0){
  	perror("socket open error");
  	//exit(1);
  }
  else {
  	mysio.number = 0;
  	mysio.length = sizeof(struct SAR_IOCTL_CFG);
  	mysio.arg = (void *)&pid;
  	ioctl(skfd, SENT_SNTP_PID, &mysio);
  	close(skfd);
  }
#endif

  // Mason Yu. Kill process in real time.
  fp2 = fopen(pidfile, "r");
  if ( fp2 != NULL ) {
  	fscanf(fp2, "%d", &pid_old);
  	//printf("Kill old vsntp process\n");
  	kill(pid_old, SIGTERM);
  	unlink(pidfile);
	fclose(fp2);
	// wait until old process exit ...
	sleep(1);
  }

  /* Save to the file */
  fp = xfopen (pidfile, "w");
  xfprintf (fp, "%d\n", pid);
  xfclose (fp);

  return;
}

/* setsigterm: Configure the way we exit */
void
setsigterm (void)
{
  struct sigaction action;

  // Mason Yu. Kill process in real time.
  action.sa_flags = 0;
  /* Set the TERM signal handler */
  action.sa_handler = sigterm_exit;
  /* Block further signals */
  sigemptyset (&action.sa_mask);
  // Mason Yu. Kill process in real time.
  sigaddset(&action.sa_mask, SIGTERM);
  /* Set the TERM signal handler */
  sigaction (SIGTERM, &action, NULL);
  return;
}

/* setsigusr1: Configure the way that user process can request
	to sync the time */
void
setsigusr1 (void)
{
	struct sigaction sa;

	sa.sa_flags = 0;

	sigemptyset(&sa.sa_mask);
	sigaddset(&sa.sa_mask, SIGUSR1);
	sa.sa_handler = sigusr1;
	sigaction(SIGUSR1, &sa, NULL);
}
#ifdef SNTP_MULTI_SERVER
int try_server_valid()
{
fprintf(stderr,"%s\n",__FUNCTION__);
int i,r;
struct ifreq req;

for(i=0;i<MAX_MUTLI_SERVER_NUM;i++)
 {
 	if(!server_name[i]) continue;
	if(server_used[i]) continue; /*ping_zhang:20081223 add to check whether the ntp_server[i] has been tried.*/
	server_used[i]=1;			/*ping_zhang:20081223 mark the ntp_server[i] has been tried.*/
 	if(!get_server(server_name[i])) continue;
	 /* Create the UDP socket */
	  udp = socket (PF_INET, SOCK_DGRAM, 0);

      if(interface)
      {
        strncpy(req.ifr_ifrn.ifrn_name, interface, 16);
        if (setsockopt(udp, SOL_SOCKET, SO_BINDTODEVICE,(char *)&req, sizeof(req)) < 0)
        {
          close(udp);
          warn ("Bind to interface error: %s.", strerror(errno));
        }
      }

	  /* Initialize the connection */
	  r = connect (udp, (struct sockaddr*) &server, sizeof (server));
	  if (r < 0)
	   {
	   	// Kaohj
	   	printf("connect %s: %s\n", server_name[i], strerror (errno));
	   	/*
	    neterror (firstsync, errstart, "connect %s: %s at %s line %d.",
	      server_name, strerror (errno), __FILE__, __LINE__);
	      */
	    /* Close the opened socket */
	    r = close (udp);
	    if (r != 0)
	      warn ("close udp: %s at %s line %d.",
	        strerror (errno), __FILE__, __LINE__);
	   udp=0;
	   }
	  else
	  {
	  	break;
	  }
	}
return udp;
}

/*ping_zhang:20081223 START:add to make each ntp server is used.*/
void init_server_used(void)
{
	int i;

	for(i=0; i<MAX_MUTLI_SERVER_NUM; i++)
	{
		if(server_name[i])
			server_used[i] = 0; /*useable and unused*/
		else
			server_used[i] = 2; /*unuseable*/
	}
	return;
}

/*check if all server are tried
	return 0: yes
		   1: no*/
unsigned int check_server_all_used(void)
{
	int i;

	for(i=0; i<MAX_MUTLI_SERVER_NUM; i++)
	{
		if(server_used[i]==0)
			return 1;
	}
	return 0;
}
/*ping_zhang:20081223 END*/
#endif
#if 0
/* synctime: Synchronize the time.  See RFC 1361.
     return: Time offset that is synchronized.*/
double
synctime (void)
{
  int i, r, udp;
  static int firstsync = 1;
  char buf[61];
  ssize_t len;
  static time_t errstart = 0;
  static struct timeval tv1, tv2, tv3, tv4, tvnew;
  struct timezone tz;
  double t1, t2, t3, t4, toff, tnew;

  /* Create the UDP socket */
  udp = socket (PF_INET, SOCK_DGRAM, 0);
  /* Initialize the connection */
  r = connect (udp, (struct sockaddr*) &server, sizeof (server));
  if (r == -1)
   {
    neterror (firstsync, errstart, "connect %s: %s at %s line %d.",
      server_name, strerror (errno), __FILE__, __LINE__);
    /* Close the opened socket */
    r = close (udp);
    if (r != 0)
      warn ("close udp: %s at %s line %d.",
        strerror (errno), __FILE__, __LINE__);
    return 0;
   }

  /* Send to the server */
  /* Pad zeroes */
  for (i = 0; i < 61; i++)
    buf[i] = 0;
  /* 00 001 011 - leap, ntp ver, client.  See RFC 1361. */
  buf[0] = (0 << 6) | (1 << 3) | 3;
  /* Get the local sent time - Originate Timestamp */
  xgettimeofday (&tv1, &tz);

  t1 = (double) tv1.tv_sec + (double) tv1.tv_usec / 1000000;
  /* Send to the server */
  memcpy (&buf[40], tonetnum ((unsigned long) tv1.tv_sec + EPOCH_DIFF), 4);
  memcpy (&buf[44], tonetnum (usec2frac (tv1.tv_usec)), 4);
  len = send (udp, buf, 48, 0);
  if (len == -1)
   {
    neterror (firstsync, errstart, "send %s: %s at %s line %d.",
      server_name, strerror (errno), __FILE__, __LINE__);
    /* Close the opened socket */
    r = close (udp);
    if (r != 0)
      warn ("close udp: %s at %s line %d.",
        strerror (errno), __FILE__, __LINE__);
    return 0;
   }

  /* Read from the server */
  len = recv (udp, &buf, 60, 0);
  if (len == -1)
   {
    neterror (firstsync, errstart, "recv %s: %s at %s line %d.",
      server_name, strerror (errno), __FILE__, __LINE__);
    /* Close the opened socket */
    r = close (udp);
    if (r != 0)
      warn ("close udp: %s at %s line %d.",
        strerror (errno), __FILE__, __LINE__);
    return 0;
   }

  /* Close the socket */
  r = close (udp);
  if (r != 0)
    warn ("close udp: %s at %s line %d.",
      strerror (errno), __FILE__, __LINE__);

  /* Get the local received time */
  xgettimeofday (&tv4, &tz);

  t4 = (double) tv4.tv_sec + (double) tv4.tv_usec / 1000000;
  /* Get the remote Receive Timestamp */
  tv2.tv_sec = fromnetnum (&buf[32]) - EPOCH_DIFF;
  tv2.tv_usec = frac2usec (fromnetnum (&buf[36]));
  t2 = (double) tv2.tv_sec + (double) tv2.tv_usec / 1000000;
  /* Get the remote Transmit Timestamp */
  tv3.tv_sec = fromnetnum (&buf[40]) - EPOCH_DIFF;
  tv3.tv_usec = frac2usec (fromnetnum (&buf[44]));
  t3 = (double) tv3.tv_sec + (double) tv3.tv_usec / 1000000;

  /* Calculate the time offset */
  toff = (t2 + t3 - t1 - t4) / 2;
  /* Calculate the new time */
  tnew = t4 + toff;
  tvnew.tv_usec = (long long) (tnew * 1000000) % 1000000;
  tvnew.tv_sec = ((long long) (tnew * 1000000) - tvnew.tv_usec) / 1000000;

  /* Set the time */
  // Mason Yu
  //tz.tz_minuteswest = 480;
  //tz.tz_dsttime = 0;
  //const struct timezone tzz = { 28800, 0 };
  //xsettimeofday (&tvnew, &tzz);
  //tvnew.tv_sec += 8*60*60;  // Taiwan
  xsettimeofday (&tvnew, &tz);

  /* Re-initialize the error timer */
  errstart = 0;
  /* Remove the first time flag */
  firstsync = 0;

  return toff;
}
#else
int
synctime (void)
{
#ifdef SNTP_MULTI_SERVER
  int i, r;
#else
  int i, r, udp;
#endif
  static int firstsync = 1;
  char buf[61];
  ssize_t len;
  static time_t errstart = 0;
  static struct timeval tv1, tv2, tv3, tv4, tvnew;
  struct timezone tz;
  unsigned long t1, t2, t3, t4, toff, tnew;
  // Kaohj
  fd_set rfd;
  struct timeval to;
  int loop;
    FILE *fp;
  struct ifreq req;


#ifdef SNTP_MULTI_SERVER
try_next_server:
if(!udp&&!try_server_valid())
{
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	   fp=fopen("/tmp/timeStatus","w");
	   if(fp){
		fprintf(fp,"%d",eTStatusErrorFailed);
		fclose(fp);
	   }
#endif
	/*ping_zhang:20081217 END*/
/*ping_zhang:20081223 START:check if each ntp server is tried.*/
	if(check_server_all_used()==1)
	{
		udp=0;
		goto try_next_server;
	}
/*ping_zhang:20081223 END*/
	return 0;
}
#else
  // Kaohj
  if (server.sin_family == 0)
  {
     if (!get_server())
	  		return 0;
}
  /* Create the UDP socket */
  udp = socket (PF_INET, SOCK_DGRAM, 0);

  if(interface)
  {
    strncpy(req.ifr_ifrn.ifrn_name, interface, 16);
    if (setsockopt(udp, SOL_SOCKET, SO_BINDTODEVICE,(char *)&req, sizeof(req)) < 0)
    {
      close(udp);
      warn ("Bind to interface error: %s.", strerror(errno));
    }
  }

  /* Initialize the connection */
  r = connect (udp, (struct sockaddr*) &server, sizeof (server));
  if (r == -1)
   {
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
   fp=fopen("/tmp/timeStatus","w");
   if(fp){
	fprintf(fp,"%d",eTStatusErrorFailed);
	fclose(fp);
   }
#endif
/*ping_zhang:20081217 END*/
   	// Kaohj
   	printf("connect %s: %s\n", server_name, strerror (errno));
   	/*
    neterror (firstsync, errstart, "connect %s: %s at %s line %d.",
      server_name, strerror (errno), __FILE__, __LINE__);
      */
    /* Close the opened socket */
    r = close (udp);
    if (r != 0)
      warn ("close udp: %s at %s line %d.",
        strerror (errno), __FILE__, __LINE__);
    return 0;
   }
#endif


  // Kaohj
  loop = 0;
retry:
  /* Send to the server */
  /* Pad zeroes */
  for (i = 0; i < 61; i++)
    buf[i] = 0;
  /* 00 001 011 - leap, ntp ver, client.  See RFC 1361. */
  buf[0] = (0 << 6) | (1 << 3) | 3;
  /* Get the local sent time - Originate Timestamp */
  xgettimeofday (&tv1, &tz);

  // Kaohj
  //t1 = (double) tv1.tv_sec + (double) tv1.tv_usec / 1000000;
  t1 = tv1.tv_sec + tv1.tv_usec / 1000000;
  //printf("tv1: %d, %d, t1=%d\n", (unsigned long)tv1.tv_sec, (unsigned long)tv1.tv_usec, t1);
  /* Send to the server */
  memcpy (&buf[40], tonetnum ((unsigned long) tv1.tv_sec + EPOCH_DIFF), 4);
  memcpy (&buf[44], tonetnum (usec2frac (tv1.tv_usec)), 4);
  len = send (udp, buf, 48, 0);
  if (len < 0)
   {
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
   fp=fopen("/tmp/timeStatus","w");
   if(fp){
	fprintf(fp,"%d",eTStatusErrorFailed);
	fclose(fp);
   }
#endif
/*ping_zhang:20081217 END*/
   	if (errno != EINTR)
   		printf("send %s: %s at %s line %d.\n",
   		  server_name, strerror (errno), __FILE__, __LINE__);
   	/*
    neterror (firstsync, errstart, "send %s: %s at %s line %d.",
      server_name, strerror (errno), __FILE__, __LINE__);
      */
    /* Close the opened socket */
    r = close (udp);
    if (r != 0)
      warn ("close udp: %s at %s line %d.",
        strerror (errno), __FILE__, __LINE__);
/*ping_zhang:20081223 START:check if each ntp server is tried.*/
#ifdef SNTP_MULTI_SERVER
    if(check_server_all_used()==1)
    {
	udp=0;
	goto try_next_server;
    }
#endif
/*ping_zhang:20081223*/
    return 0;
   }

  /* Read from the server */
  // kaohj -- retry if necessary
  to.tv_sec = 1;
  to.tv_usec = 0;
  FD_ZERO(&rfd);
  FD_SET(udp, &rfd);

  if ((select(udp+1, &rfd, NULL, NULL, &to)) > 0) {
  	len = recv (udp, &buf, 60, 0);
  }
  else {
  	loop++;
  	if (loop <= 5)
  		goto retry;
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
   fp=fopen("/tmp/timeStatus","w");
   if(fp){
	fprintf(fp,"%d",eTStatusErrorFailed);
	fclose(fp);
   }
#endif
/*ping_zhang:20081217 END*/
  	close (udp);
/*ping_zhang:20081223 START:check if each ntp server is tried.*/
#ifdef SNTP_MULTI_SERVER
	if(check_server_all_used()==1)
	{
		udp=0;
		goto try_next_server;
	}
#endif
/*ping_zhang:20081223 END*/
  	return 0;
  }

  if (len < 0)
   {
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
   fp=fopen("/tmp/timeStatus","w");
   if(fp){
	fprintf(fp,"%d",eTStatusErrorFailed);
	fclose(fp);
   }
#endif
/*ping_zhang:20081217 END*/
   	if (errno != EINTR)
   		printf("recv %s: %s at %s line %d.\n",
   		  server_name, strerror (errno), __FILE__, __LINE__);
   	/*
    neterror (firstsync, errstart, "recv %s: %s at %s line %d.",
      server_name, strerror (errno), __FILE__, __LINE__);
      */
    /* Close the opened socket */
    r = close (udp);
    if (r != 0)
      warn ("close udp: %s at %s line %d.",
        strerror (errno), __FILE__, __LINE__);
/*ping_zhang:20081223 START:check if each ntp server is tried.*/
#ifdef SNTP_MULTI_SERVER
   if(check_server_all_used()==1)
   {
		udp=0;
		goto try_next_server;
   }
#endif
/*ping_zhang:20081223 END*/
    return 0;
   }

  /* Close the socket */
  r = close (udp);
  if (r != 0)
    warn ("close udp: %s at %s line %d.",
      strerror (errno), __FILE__, __LINE__);

  /* Get the local received time */
  xgettimeofday (&tv4, &tz);

  // Kaohj
  //t4 = (double) tv4.tv_sec + (double) tv4.tv_usec / 1000000;
  t4 = tv4.tv_sec + tv4.tv_usec / 1000000;
  //printf("tv4: %d, %d, t4=%d\n", (unsigned long)tv4.tv_sec, (unsigned long)tv4.tv_usec, t4);
  /* Get the remote Receive Timestamp */
  tv2.tv_sec = fromnetnum (&buf[32]) - EPOCH_DIFF;
  tv2.tv_usec = frac2usec (fromnetnum (&buf[36]));
  // Kaohj
  //t2 = (double) tv2.tv_sec + (double) tv2.tv_usec / 1000000;
  t2 = tv2.tv_sec + tv2.tv_usec / 1000000;
  //printf("tv2: %d, %d, t2=%d\n", tv2.tv_sec, tv2.tv_usec, t2);
  /* Get the remote Transmit Timestamp */
  tv3.tv_sec = fromnetnum (&buf[40]) - EPOCH_DIFF;
  tv3.tv_usec = frac2usec (fromnetnum (&buf[44]));
  // Kaohj
  //t3 = (double) tv3.tv_sec + (double) tv3.tv_usec / 1000000;
  t3 = tv3.tv_sec + tv3.tv_usec / 1000000;
  //printf("tv3: %d, %d, t3=%d\n", tv3.tv_sec, tv3.tv_usec, t3);

  /* Calculate the time offset */
  toff = (t2 + t3 - t1 - t4) / 2;
  /* Calculate the new time */
  // Kaohj
  //tnew = t4 + toff;
  tnew = (t2 + t3 - t1 + t4) / 2;
  //printf("toff=%d, tnew=%d\n", toff, tnew);
  //tvnew.tv_usec = (long long) (tnew * 1000000) % 1000000;
  tvnew.tv_usec = 0;
  // Kaohj
  //tvnew.tv_sec = ((long long) (tnew * 1000000) - tvnew.tv_usec) / 1000000;
  tvnew.tv_sec = tnew;
  //printf("tvnew: %d, %d\n", tvnew.tv_sec, tvnew.tv_usec);

  /* Set the time */
  xsettimeofday (&tvnew, &tz);
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
   fp=fopen("/tmp/timeStatus","w");
   if(fp){
	fprintf(fp,"%d",eTStatusSynchronized);
	fclose(fp);
   }
#endif
/*ping_zhang:20081217 END*/
  /* Re-initialize the error timer */
  errstart = 0;
  /* Remove the first time flag */
  firstsync = 0;

  return toff;
}

#endif

/* fork2background: Fork to the background.
     return: none. */
void
fork2background (void)
{
  pid_t pid;
  /* Fork */
  pid = xfork ();
  /* Exit the parent process */
  if (pid != 0)
    exit (0);
  return;
}

/* closeall: Close all the opened file descriptors,
             especially: stdin, stdout and stderr.
     return: none. */
void
closeall (void)
{
  int i;
  long openmax;
  openmax = xsysconf (_SC_OPEN_MAX, "_SC_OPEN_MAX");
  for (i = 0; i < openmax; i++)
    close (i);
  return;
}

/* fromnetnum: Convert from a network number to a C number.
     return: the number in unsigned long. */
unsigned long
fromnetnum (const char *oct)
{
  return ((unsigned char) oct[0] << 24 | (unsigned char) oct[1] << 16 | (unsigned char) oct[2] << 8 | (unsigned char) oct[3]);
}

/* tonetnum: Convert from a C number to a network number.
     return: the number in network octet.  */
const char *tonetnum (unsigned long num)
{
  static char oct[5] = "0000";
  oct[0] = (num >> 24) & 255;
  oct[1] = (num >> 16) & 255;
  oct[2] = (num >> 8) & 255;
  oct[3] = num & 255;
  return oct;
}

/* usec2frac: Convert from microsecond to fraction of a second.
     return: Fraction of a second. */
unsigned long
usec2frac (long usec)
{
  return (unsigned long) (((long long) usec << 32) / 1000000);
}

/* usec2frac: Convert from fraction of a second to microsecond
     return: microsecond. */
long
frac2usec (unsigned long frac)
{
  return (long) (((long long) frac * 1000000) >> 32);
}

/* xstrcpy: allocate enough memory, make a copy of the string, and handle errors.
     return: pointer to the destination string. */
char *
xstrcpy (const char *src)
{
  char *dest;
  dest = (char *) xmalloc (strlen (src) + 1);
  strcpy (dest, src);
  return dest;
}

/* xstrcat: allocate enough memory, concatenate the strings, and handle errors.
     return: pointer to the destination string. */
char *
xstrcat (int n, ...)
{
  int i;
  size_t len;
  va_list ap;
  char *s;

  /* Calculate the result size */
  va_start (ap, n);
  for (i = 0, len = 0; i < n; i++)
    len += strlen (va_arg (ap, char *));
  va_end (ap);

  /* Allocate the memory */
  s = (char *) xmalloc (len + 1);

  /* Concatenate the strings */
  va_start (ap, n);
  strcpy (s, va_arg (ap, char *));
  for (i = 1; i < n; i++)
    strcat (s, va_arg (ap, char *));
  va_end (ap);

  return s;
}

/* xmalloc: malloc() and handle errors.
     return: pointer to the allocated memory block. */
void *
xmalloc (size_t size)
{
  void *ptr;
  ptr = malloc (size);
  if (ptr == NULL)
    error (EXIT_SYSERR, "malloc: %s at %s line %d.",
      strerror (errno), __FILE__, __LINE__);
  return ptr;
}

/* xtime: time() and handle errors.
     return: current time. */
time_t
xtime (time_t *t)
{
  time_t r;
  r = time (t);
  if (r == -1)
    error (EXIT_SYSERR, "time: %s at %s line %d.",
      strerror (errno), __FILE__, __LINE__);
  return r;
}

/* xgettimeofday: gettimeofday() and handle errors.
     return: none. */
void
xgettimeofday (struct timeval *tv, struct timezone *tz)
{
  int r;
  r = gettimeofday (tv, tz);
  if (r == -1)
    error (EXIT_SYSERR, "gettimeofday: %s at %s line %d.",
      strerror (errno), __FILE__, __LINE__);
  return;
}

/* xsettimeofday: settimeofday() and handle errors.
     return: none. */
void
xsettimeofday (const struct timeval *tv , const struct timezone *tz)
{
  int r;
  r = settimeofday (tv, tz);
  if (r == -1)
    error (EXIT_SYSERR, "settimeofday: %s at %s line %d.",
      strerror (errno), __FILE__, __LINE__);
  return;
}

/* xchdir: chdir() and handle errors.
     return: none. */
void
xchdir (const char *path)
{
  int r;
  r = chdir (path);
  if (r == -1)
    error (EXIT_SYSERR, "chdir %s: %s at %s line %d.",
      path, strerror (errno), __FILE__, __LINE__);
  return;
}

/* xfork: fork() and handle errors.
     return: none. */
pid_t
xfork (void)
{
  pid_t pid;
  pid = fork ();
  if (pid == -1)
    error (EXIT_SYSERR, "fork: %s at %s line %d.", strerror (errno), __FILE__, __LINE__);
  return pid;
}

/* xsetsid: setsid() and handle errors.
     return: none. */
pid_t
xsetsid (void)
{
  pid_t pid;
  pid = setsid ();
  if (pid == -1)
    error (EXIT_SYSERR, "setsid: %s at %s line %d.",
      strerror (errno), __FILE__, __LINE__);
  return pid;
}

/* xsysconf: sysconf() and handle errors.
     return: none. */
long
xsysconf (int name, const char *confname)
{
  long r;
  r = sysconf (name);
  if (r == -1)
    error (EXIT_SYSERR, "sysconf %s: %s at %s line %d.",
      confname, strerror (errno), __FILE__, __LINE__);
  return r;
}

/* xfopen: fopen() and handle errors.
     return: file handler pointer. */
FILE *
xfopen (const char *path, const char *mode)
{
  FILE *fp;
  fp = fopen (path, mode);
  if (fp == NULL)
    error (EXIT_SYSERR, "fopen %s: %s at %s line %d.",
      path, strerror (errno), __FILE__, __LINE__);
  return fp;
}

/* xfclose: fclose() and handle errors.
     return: none. */
void
xfclose (FILE *stream)
{
  int r;
  r = fclose (stream);
  if (r == EOF)
    error (EXIT_SYSERR, "fclose: %s at %s line %d.",
      strerror (errno), __FILE__, __LINE__);
  return;
}

/* xfprintf: fprintf() and handle errors.
     return: length print so far. */
int
xfprintf (FILE *stream, const char *format, ...)
{
  int len;
  va_list ap;
  va_start (ap, format);
  len = vfprintf (stream, format, ap);
  va_end (ap);
  if (len < 0)
    error (EXIT_SYSERR, "vfprintf: %s at %s line %d.",
      strerror (errno), __FILE__, __LINE__);
  return len;
}

/* Main program */
int
main (int argc, char *argv[])
{
  time_t now, next;
  unsigned int wait, ret;
  // Kaohj
  //double toff;
  int toff;

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
   FILE *fp;
   fp=fopen("/tmp/timeStatus","w");
   if(fp){
	fprintf(fp,"%d",eTStatusUnsynchronized);
	fclose(fp);
   }
#endif
/*ping_zhang:20081217 END*/
  /* Find this file's name */
  set_this_file (argv[0]);
  parse_args (argc, argv);

  /* Synchronize for the first time, to ensure no obvious network errors */

/*ping_zhang:20081223 START:add to make each ntp server is used.*/
#ifdef SNTP_MULTI_SERVER
  udp=0;
  init_server_used();
#endif
/*ping_zhang:20081223 END*/
  toff = synctime ();
  /* Daemonize */
  // Kaohj
  //daemonize ();

  /*ping_zhang:20081223 START:remove makepid() from parse_args() to avoid vsntp can't start up.*/
  makepid();
  /*ping_zhang:20081223 END*/

  /* Set the way we exit */
  setsigterm ();
  setsigusr1 ();

  /* Report the first synchronization */
  // Kaohj
  //syslog (LOG_INFO, "Time adjusted %.6f seconds.", toff);
  //syslog (LOG_INFO, "Time adjusted %.6d seconds.", toff);
  /* Calculate the next time to synchronize */
  now = xtime (NULL);
  wait = interval - (now % interval);
  next = now + wait;
  /* Wait until that time */
  sleep (wait);

  /* Enter endless loop of synchronization */
  while (1) {
  /*ping_zhang:20081223 START:add to make each ntp server is used.*/
  #ifdef SNTP_MULTI_SERVER
  udp=0;
  init_server_used();
  #endif
  /*ping_zhang:20081223 END*/
    /* Synchronize the time */
    synctime ();
    now = xtime (NULL);
    /* Find the next time line */
    do next += interval;
      while (next <= now);
    wait = next - now;
    /* Wait until that time */
    ret = sleep (wait);
    if (ret != 0)
    	sleep(10); // interrupted by SIGUSR1, wait for interface up
  }

  return 0;
}


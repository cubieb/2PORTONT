#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <syslog.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#ifdef HAVE_TCPD_H
#include <tcpd.h>
#endif

#include "extern.h"

static void reapchild __P ((int));

#define DEFPORT 21

#ifdef FULL_FTPD
#ifdef WITH_WRAP

int allow_severity = LOG_INFO;
int deny_severity = LOG_NOTICE;

static int
check_host (struct sockaddr *sa)
{
  struct hostent *hp;
  char addr[INET6_ADDRSTRLEN];

#ifdef IPV6
  if (sa->sa_family != AF_INET6)
#else
  if (sa->sa_family != AF_INET)
#endif
    return 1;

  // Kao --- gethostbyaddr will try for a long time if not successful
  //hp = gethostbyaddr ((char *)&sin->sin_addr,
	//	      sizeof (struct in_addr), AF_INET);
  hp = NULL;

  getnameinfo(sa, sizeof(*sa), addr, sizeof(addr) - 1,
		  NULL, 0, NI_NUMERICHOST);
  if (hp)
    {
      if (!hosts_ctl ("ftpd", hp->h_name, addr, STRING_UNKNOWN))
	{
	  syslog (LOG_NOTICE, "tcpwrappers rejected: %s [%s]",
		  hp->h_name, addr);
	  return 0;
	}
    }
  else
    {
      if (!hosts_ctl ("ftpd", STRING_UNKNOWN, addr, STRING_UNKNOWN))
	{
	  syslog (LOG_NOTICE, "tcpwrappers rejected: [%s]", addr);
	  return 0;
	}
    }
  return (1);
}
#endif
#endif

static void
reapchild (int signo)
{
  int save_errno = errno;

  (void)signo;
  while (waitpid (-1, NULL, WNOHANG) > 0)
    ;
  errno = save_errno;
}

#ifndef __uClinux__
// Kaohj, not support daemon mode
int
server_mode (const char *pidfile, union sockunion *phis_addr)
{
  int ctl_sock, fd;
  struct servent *sv;
  int port;
  static union sockunion server_addr;  /* Our address.  */

  /* Become a daemon.  */
  if (daemon(1,1) < 0)
    {
      syslog (LOG_ERR, "failed to become a daemon");
      return -1;
    }
  (void) signal (SIGCHLD, reapchild);

  /* Get port for ftp/tcp.  */
  sv = getservbyname ("ftp", "tcp");
  port = (sv == NULL) ? DEFPORT : ntohs(sv->s_port);

  /* Open socket, bind and start listen.  */
  ctl_sock = socket (phis_addr->su_family, SOCK_STREAM, 0);
  if (ctl_sock < 0)
    {
      syslog (LOG_ERR, "control socket: %m");
      return -1;
    }

  /* Enable local address reuse.  */
  {
    int on = 1;
    if (setsockopt (ctl_sock, SOL_SOCKET, SO_REUSEADDR,
		    (char *)&on, sizeof(on)) < 0)
      syslog (LOG_ERR, "control setsockopt: %m");
  }

  memset (&server_addr, 0, sizeof server_addr);
  server_addr.su_family = AF_INET6;
  server_addr.su_port = htons (port);

  if (bind (ctl_sock, (struct sockaddr *)&server_addr, sizeof server_addr))
    {
      syslog (LOG_ERR, "control bind: %m");
      return -1;
    }
  if (listen (ctl_sock, 32) < 0)
    {
      syslog (LOG_ERR, "control listen: %m");
      return -1;
    }

#ifdef FULL_FTPD
  /* Stash pid in pidfile.  */
  {
    FILE *pid_fp = fopen (pidfile, "w");
    if (pid_fp == NULL)
      syslog (LOG_ERR, "can't open %s: %m", PATH_FTPDPID);
    else
      {
	fprintf (pid_fp, "%d\n", getpid());
	fchmod (fileno(pid_fp), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	fclose (pid_fp);
      }
  }
#endif

  /* Loop forever accepting connection requests and forking off
     children to handle them.  */
  while (1)
    {
      int addrlen = sizeof (*phis_addr);
      fd = accept (ctl_sock, (struct sockaddr *)phis_addr, &addrlen);
      if (fork () == 0) /* child */
	{
	  (void) dup2 (fd, 0);
	  (void) dup2 (fd, 1);
	  close (ctl_sock);
	  break;
	}
      close (fd);
    }

#ifdef FULL_FTPD
#ifdef WITH_WRAP
  /* In the child.  */
  if (!check_host ((struct sockaddr *)phis_addr))
    return -1;
#endif
#endif
  return fd;
}
#endif	// of __uClinux__

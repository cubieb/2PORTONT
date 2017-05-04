/*
    rshd.c - remote shell server
    Copyright (C) 2003  Guus Sliepen <guus@sliepen.eu.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as published
    by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <security/pam_appl.h>
#include <pty.h>
#include <utmp.h>
#include <grp.h>
#include <paths.h>

static char *argv0;

static void usage(void) {
	syslog(LOG_NOTICE, "Usage: %s", argv0);
}

/* Read until a NULL byte is encountered */

static ssize_t readtonull(int fd, char *buf, size_t count) {
	int len = 0, result;
	
	while(count) {
		result = read(fd, buf, 1);
		
		if(result <= 0)
			return result;

		len++;
		count--;
				
		if(!*buf++)
			return len;
	}
	
	errno = ENOBUFS;
	return -1;
}

/* PAM conversation function */

static int conv_h(int msgc, const struct pam_message **msgv, struct pam_response **res, void *app) {
	syslog(LOG_ERR, "PAM requires conversation");
	return PAM_CONV_ERR;
}

int main(int argc, char **argv) {
	struct sockaddr_storage peer_sa;
	struct sockaddr *peer = (struct sockaddr *)&peer_sa;
	socklen_t peerlen = sizeof peer_sa;
	
	char user[1024];
	char luser[1024];
	char command[1024];
	char env[1024];
		
	struct passwd *pw;
	
	int err;
	
	int opt;

	char host[NI_MAXHOST];
	char addr[NI_MAXHOST];
	char port[NI_MAXSERV];
	char eport[NI_MAXSERV];
	char lport[NI_MAXSERV];
	int portnr, eportnr;
	
	struct addrinfo hint, *ai, *lai;
	int esock;
		
	int i;

	pam_handle_t *handle;		
	struct pam_conv conv = {conv_h, NULL};
	const void *item;
	char *pamuser;
	
	char *shellname;
	
	argv0 = argv[0];
	
	/* Process options */
			
	while((opt = getopt(argc, argv, "+")) != -1) {
		switch(opt) {
			default:
				syslog(LOG_ERR, "Unknown option!");
				usage();
				return 1;
		}
	}
	
	if(optind != argc) {
		syslog(LOG_ERR, "Too many arguments!");
		usage();
		return 1;
	}
	
	/* Check source of connection */
	
	if(getpeername(0, peer, &peerlen)) {
		syslog(LOG_ERR, "Can't get address of peer: %m");
		return 1;
	}
	
	/* Unmap V4MAPPED addresses */
	
	if(peer->sa_family == AF_INET6 && IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6 *)peer)->sin6_addr)) {
		((struct sockaddr_in *)peer)->sin_addr.s_addr = ((struct sockaddr_in6 *)peer)->sin6_addr.s6_addr32[3];
		peer->sa_family = AF_INET;
	}

	/* Lookup hostname */
	
	if((err = getnameinfo(peer, peerlen, host, sizeof host, NULL, 0, 0))) {
		syslog(LOG_ERR, "Error resolving address: %s", gai_strerror(err));
		return 1;
	}
	
	if((err = getnameinfo(peer, peerlen, addr, sizeof addr, port, sizeof port, NI_NUMERICHOST | NI_NUMERICSERV))) {
		syslog(LOG_ERR, "Error resolving address: %s", gai_strerror(err));
		return 1;
	}
	
	/* Check if connection comes from a privileged port */
	
	portnr = atoi(port);
	
	if(portnr < 512 || portnr >= 1024) {
		syslog(LOG_ERR, "Connection from %s on illegal port %d.", host, portnr);
		return 1;
	}
	
	/* Read port number for stderr socket */
	
	if(readtonull(0, eport, sizeof eport) <= 0) {
		syslog(LOG_ERR, "Error while receiving stderr port number from %s: %m", host);
		return 1;
	}
	
	eportnr = atoi(eport);
	
	if(eportnr) {
		memset(&hint, '\0', sizeof hint);
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_family = peer->sa_family;
		
		err = getaddrinfo(addr, eport, &hint, &ai);
		if(err || !ai) {
			syslog(LOG_ERR, "Error looking up host: %s", gai_strerror(err));
			return 1;
		}

		esock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
		
		if(esock == -1) {
			syslog(LOG_ERR, "socket() failed: %m");
			return 1;
		}

		hint.ai_flags = AI_PASSIVE;
		
		for(i = 1023; i >= 512; i--) {
			snprintf(lport, sizeof lport, "%d", i);
			err = getaddrinfo(NULL, lport, &hint, &lai);
			if(err || !ai) {
				syslog(LOG_ERR, "Error looking up localhost: %s", gai_strerror(err));
				return 1;
			}
			
			err = bind(esock, lai->ai_addr, lai->ai_addrlen);
			
			freeaddrinfo(lai);
			
			if(err)
				continue;
			else
				break;
		}
		
		if(err) {
			syslog(LOG_ERR, "Could not bind to privileged port: %m");
			return 1;
		}
		
		if(connect(esock, ai->ai_addr, ai->ai_addrlen)) {
			syslog(LOG_ERR, "Connecting to stderr port %d on %s failed: %m", eportnr, host);
			return 1;
		}
		
		freeaddrinfo(ai);
		
		if(esock != 2) {
			if(dup2(esock, 2) == -1) {
				syslog(LOG_ERR, "dup2() failed: %m");
				return 1;
			}
			close(esock);
		}
	}

	/* Read usernames and terminal info */
	
	if(readtonull(0, user, sizeof user) <= 0 || readtonull(0, luser, sizeof luser) <= 0) {
		syslog(LOG_ERR, "Error while receiving usernames from %s: %m", host);
		return 1;
	}
	
	if(readtonull(0, command, sizeof command) <= 0) {
		syslog(LOG_ERR, "Error while receiving command from %s: %m", host);
		return 1;
	}
	
	syslog(LOG_NOTICE, "Connection from %s@%s for %s", user, host, luser);
	
	/* Start PAM */
	
	if((err = pam_start("rsh", luser, &conv, &handle)) != PAM_SUCCESS) {
		write(1, "Authentication failure\n", 23);
		syslog(LOG_ERR, "PAM error: %s", pam_strerror(handle, err));
		return 1;
	}
		
	pam_set_item(handle, PAM_USER, luser);
	pam_set_item(handle, PAM_RUSER, user);
	pam_set_item(handle, PAM_RHOST, host);

	/* Write NULL byte to client so we can give a login prompt if necessary */
	
	if(write(1, "", 1) <= 0) {
		syslog(LOG_ERR, "Unable to write NULL byte: %m");
		return 1;
	}
	
	/* Try to authenticate */
	
	err = pam_authenticate(handle, 0);
	
	/* PAM might ask for a new password */
	
	if(err == PAM_NEW_AUTHTOK_REQD) {
		err = pam_chauthtok(handle, PAM_CHANGE_EXPIRED_AUTHTOK);
		if(err == PAM_SUCCESS)
			err = pam_authenticate(handle, 0);
	}
	
	if(err != PAM_SUCCESS) {
		write(1, "Authentication failure\n", 23);
		syslog(LOG_ERR, "PAM error: %s", pam_strerror(handle, err));
		return 1;
	}

	/* Check account */
	
	err = pam_acct_mgmt(handle, 0);
	
	if(err != PAM_SUCCESS) {
		write(1, "Authentication failure\n", 23);
		syslog(LOG_ERR, "PAM error: %s", pam_strerror(handle, err));
		return 1;
	}

	/* PAM can map the user to a different user */
	
	err = pam_get_item(handle, PAM_USER, &item);
	
	if(err != PAM_SUCCESS) {
		syslog(LOG_ERR, "PAM error: %s", pam_strerror(handle, err));
		return 1;
	}
	
	pamuser = strdup((char *)item);
	
	if(!pamuser || !*pamuser) {
		syslog(LOG_ERR, "PAM didn't return a username?!");
		return 1;
	}

	pw = getpwnam(pamuser);

	if (!pw) {
		syslog(LOG_ERR, "PAM_USER does not exist?!");
		return 1;
	}
	
	if (setgid(pw->pw_gid)) {
		syslog(LOG_ERR, "setgid() failed: %m");
		return 1;
	}
	
	if (initgroups(pamuser, pw->pw_gid)) {
		syslog(LOG_ERR, "initgroups() failed: %m");
		return 1;
	}
	
	err = pam_setcred(handle, PAM_ESTABLISH_CRED);
	
	if(err != PAM_SUCCESS) {
		syslog(LOG_ERR, "PAM error: %s", pam_strerror(handle, err));
		return 1;
	}
	
	/* Authentication succeeded */
	
	if(setuid(pw->pw_uid)) {
		syslog(LOG_ERR, "setuid() failed: %m");
		return 1;
	}
	
	if(!pw->pw_shell || !*pw->pw_shell) {
		syslog(LOG_ERR, "No shell for %s", pamuser);
		return 1;
	}
	
	/* Set some environment variables PAM doesn't set */
	
	snprintf(env, sizeof env, "USER=%s", pamuser);
	pam_putenv(handle, env);
	snprintf(env, sizeof env, "SHELL=%s", pw->pw_shell);
	pam_putenv(handle, env);
	snprintf(env, sizeof env, "PATH=%s", _PATH_DEFPATH);
	pam_putenv(handle, env);
	snprintf(env, sizeof env, "HOME=%s", pw->pw_dir);
	pam_putenv(handle, env);
	
	/* Run command */
	
	if(chdir(pw->pw_dir) && chdir("/")) {
		syslog(LOG_DEBUG, "chdir() failed: %m");
		return 1;
	}
	
	shellname = strrchr(pw->pw_shell, '/');
	if(shellname)
		shellname++;
	else
		shellname = pw->pw_shell;				
	
	if(strrchr(pw->pw_shell, '/'))
	
	execle(pw->pw_shell, shellname, "-c", command, NULL, pam_getenvlist(handle));
	
	syslog(LOG_ERR, "Failed to spawn shell: %m");
	return 1;
}

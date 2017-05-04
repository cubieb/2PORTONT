/*
    rlogind.c - remote login server
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
#include <sys/poll.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <termios.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <security/pam_appl.h>
#include <pty.h>
#include <utmp.h>
#include <grp.h>
#include <syslog.h>

static char *argv0;

static void usage(void) {
	syslog(LOG_NOTICE, "Usage: %s", argv0);
}

/* Make sure everything gets written */

static ssize_t safewrite(int fd, const void *buf, size_t count) {
	int result;
	
	while(count) {
		result = write(fd, buf, count);
		if(result <= 0)
			return -1;
		buf += result;
		count -= result;
	}
	
	return count;
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

static ssize_t conv_read(int infd, int outfd, char *buf, size_t count, int echo) {
	int len = 0, result;
	
	while(count) {
		result = read(infd, buf, 1);
		
		if(result <= 0)
			return result;

		if(!*buf)
			continue;
		
		len++;
		count--;
		
		if(*buf == '\r') {
			if(write(outfd, "\n\r", 2) <= 0)
				return -1;
			*buf = '\0';
			return len;
		}
		
		if(echo)
			if(write(outfd, buf, 1) <= 0)
				return -1;
		
		buf++;
	}
	
	errno = ENOBUFS;	
	return -1;
}

static int conv_h(int msgc, const struct pam_message **msgv, struct pam_response **res, void *app) {
	int i, err;
	char reply[1024];
	
	*res = malloc(sizeof *reply * msgc);
	if(!*res)
		return PAM_CONV_ERR;
	memset(*res, '\0', sizeof *reply * msgc);
	
	for(i = 0; i < msgc; i++) {
		switch(msgv[i]->msg_style) {
			case PAM_PROMPT_ECHO_OFF:
				if(safewrite(1, msgv[i]->msg, strlen(msgv[i]->msg)) == -1)
					return PAM_CONV_ERR;
				err = conv_read(0, 1, reply, sizeof reply, 0);
				if(err <= 0)
					return PAM_CONV_ERR;
				res[i]->resp = strdup(reply);
				break;
			case PAM_PROMPT_ECHO_ON:
				if(safewrite(1, msgv[i]->msg, strlen(msgv[i]->msg)) == -1)
					return PAM_CONV_ERR;
				err = conv_read(0, 1, reply, sizeof reply, 1);
				if(err <= 0)
					return PAM_CONV_ERR;
				res[i]->resp = strdup(reply);
				break;
			case PAM_ERROR_MSG:
				if(safewrite(1, msgv[i]->msg, strlen(msgv[i]->msg)) == -1)
					return PAM_CONV_ERR;
				if(safewrite(1, "\n", 1) == -1)
					return PAM_CONV_ERR;
				break;
			case PAM_TEXT_INFO:
				if(safewrite(1, msgv[i]->msg, strlen(msgv[i]->msg)) <= 0)
					return PAM_CONV_ERR;
				if(safewrite(1, "\n", 1) == -1)
					return PAM_CONV_ERR;
				break;
			default:
				return PAM_CONV_ERR;
		}
	}
	
	return PAM_SUCCESS;
}

int main(int argc, char **argv) {
	struct sockaddr_storage peer_sa;
	struct sockaddr *peer = (struct sockaddr *)&peer_sa;
	socklen_t peerlen = sizeof peer_sa;
	
	char user[1024];
	char luser[1024];
	char term[1024];
		
	int portnr;
	
	struct passwd *pw;
	
	int err;
	
	int opt;

	char host[NI_MAXHOST];
	char addr[NI_MAXHOST];
	char port[NI_MAXSERV];
	
	char buf[4096];
	int len;
	
	struct pollfd pfd[3];
	
	struct winsize winsize;
	uint16_t winbuf[4];
	int i;
	
	int master, slave;
	char *tty, *ttylast;

	pam_handle_t *handle;		
	struct pam_conv conv = {conv_h, NULL};
	const void *item;
	char *pamuser;
	
	int pid;
	
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
	
	/* Wait for NULL byte */
	
	if(read(0, buf, 1) != 1 || *buf) {
		syslog(LOG_ERR, "Didn't receive NULL byte from %s: %m\n", host);
		return 1;
	}

	/* Read usernames and terminal info */
	
	if(readtonull(0, user, sizeof user) <= 0 || readtonull(0, luser, sizeof luser) <= 0) {
		syslog(LOG_ERR, "Error while receiving usernames from %s: %m", host);
		return 1;
	}
	
	if(readtonull(0, term, sizeof term) <= 0) {
		syslog(LOG_ERR, "Error while receiving terminal from %s: %m", host);
		return 1;
	}
	
	syslog(LOG_NOTICE, "Connection from %s@%s for %s", user, host, luser);
	
	/* We need to have a pty before we can use PAM */
	
	if(openpty(&master, &slave, 0, 0, &winsize) != 0) {
		syslog(LOG_ERR, "Could not open pty: %m");
		return 1;
	}
	
	tty = ttyname(slave);

	/* Start PAM */
	
	if((err = pam_start("rlogin", luser, &conv, &handle)) != PAM_SUCCESS) {
		safewrite(1, "Authentication failure\n", 23);
		syslog(LOG_ERR, "PAM error: %s", pam_strerror(handle, err));
		return 1;
	}
		
	pam_set_item(handle, PAM_USER, luser);
	pam_set_item(handle, PAM_RUSER, user);
	pam_set_item(handle, PAM_RHOST, host);
	pam_set_item(handle, PAM_TTY, tty);

	/* Write NULL byte to client so we can give a login prompt if necessary */
	
	if(safewrite(1, "", 1) == -1) {
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
		safewrite(1, "Authentication failure\n", 23);
		syslog(LOG_ERR, "PAM error: %s", pam_strerror(handle, err));
		return 1;
	}

	/* Check account */
	
	err = pam_acct_mgmt(handle, 0);
	
	if(err != PAM_SUCCESS) {
		safewrite(1, "Authentication failure\n", 23);
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
	
	pam_end(handle, PAM_SUCCESS);

	if((pid = fork()) < 0) {
		syslog(LOG_ERR, "fork() failed: %m");
		return 1;
	}
	
	if(send(1, "\x80", 1, MSG_OOB) <= 0) {
		syslog(LOG_ERR, "Unable to write OOB \x80: %m");
		return 1;
	}
	
	if(pid) {
		/* Parent process, still the rlogin server */
		
		close(slave);

		/* Process input/output */

		pfd[0].fd = 0;
		pfd[0].events = POLLIN | POLLERR | POLLHUP;
		pfd[1].fd = master;
		pfd[1].events = POLLIN | POLLERR | POLLHUP;
		
		for(;;) {
			errno = 0;

			if(poll(pfd, 2, -1) == -1) {
				if(errno == EINTR)
					continue;
				break;
			}

			if(pfd[0].revents) {
				len = read(0, buf, sizeof buf);
				if(len <= 0)
					break;

				/* Scan for control messages. Yes this is evil and should be done differently. */
				
				for(i = 0; i < len - 11;) {
					if(buf[i++] == (char)0xFF)
					if(buf[i++] == (char)0xFF)
					if(buf[i++] == 's')
					if(buf[i++] == 's') {
						memcpy(winbuf, buf + i, 8);
						winsize.ws_row = ntohs(winbuf[0]);
						winsize.ws_col = ntohs(winbuf[1]);
						winsize.ws_xpixel = ntohs(winbuf[2]);
						winsize.ws_ypixel = ntohs(winbuf[3]);
						if(ioctl(master, TIOCSWINSZ, &winsize) == -1)
							break;
						memcpy(buf + i - 4, buf + i + 8, len - i - 8);
						i -= 4;
						len -= 12;
					}
				}
				
				if(safewrite(master, buf, len) == -1)
					break;
				pfd[0].revents = 0;
			}

			if(pfd[1].revents) {
				len = read(master, buf, sizeof buf);
				if(len <= 0) {
					errno = 0;
					break;
				}
				if(safewrite(1, buf, len) == -1)
					break;
				pfd[1].revents = 0;
			}
		}

		/* The end */
		
		if(errno) {
			syslog(LOG_NOTICE, "Closing connection with %s@%s: %m", user, host);
			err = 1;
		} else {
			syslog(LOG_NOTICE, "Closing connection with %s@%s", user, host);
			err = 0;
		}
		
		ttylast = tty + 5;

		if(logout(ttylast))
			logwtmp(ttylast, "", "");
			
		close(master);
	} else {
		/* Child process, will become the shell */
		
		char *speed;
		struct termios tios;
		char *envp[2];

		/* Prepare tty for login */

		close(master);
		if(login_tty(slave)) {
			syslog(LOG_ERR, "login_tty() failed: %m");
			return 1;
		}

		/* Fix terminal type and speed */
		
		tcgetattr(0, &tios);

		if((speed = strchr(term, '/'))) {
			*speed++ = '\0';
			cfsetispeed(&tios, atoi(speed));
			cfsetospeed(&tios, atoi(speed));
		}
		
		tcsetattr(0, TCSADRAIN, &tios);

		/* Create environment */

		asprintf(&envp[0], "TERM=%s", term);
		envp[1] = NULL;

		/* Spawn login process */
		
		execle("/bin/login", "login", "-p", "-h", host, "-f", pamuser, NULL, envp);

		syslog(LOG_ERR, "Failed to spawn login process: %m");
		return 1;
	}

	return err;
}

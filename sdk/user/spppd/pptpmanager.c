/*
 * pptpmanager.c
 *
 * Manages the PoPToP sessions.
 *
 * $Id: pptpmanager.c,v 1.2 2012/09/10 08:03:25 knight_peng Exp $
 */


#ifdef __linux__
#define _GNU_SOURCE 1		/* broken arpa/inet.h */
#endif

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>


#include "pptpdefs.h"
#include "pptpmanager.h"
#include "if_sppp.h"

extern fd_set pptpConnSet;
extern int maxPptpConnFd;
extern int pptp_host_socket;
extern struct sppp *spppq;


int pptp_manager(void)
{
	struct sppp *sq, *sp;
	PPTPD_DRV_CTRL *p;
	socklen_t addrsize;
	int send_packet;
	int outCallRecvd = 0;
	struct timeval tv;
	unsigned char packet[PPTP_MAX_CTRL_PCKT_SIZE];
	unsigned char rply_packet[PPTP_MAX_CTRL_PCKT_SIZE];
	ssize_t rply_size;		/* Reply packet size */
	fd_set in;

	in = pptpConnSet;

	tv.tv_sec = 0;
	tv.tv_usec = 10000;

	if (select(maxPptpConnFd + 1, &in, NULL, NULL, &tv) <= 0) {
		return 0;
	}

	if (FD_ISSET(pptp_host_socket, &in)) {	/* A call came! */
		int clientSocket;
		struct sockaddr_in client_addr;

		/* Accept call and launch PPTPCTRL */
		addrsize = sizeof(client_addr);
		clientSocket = accept(pptp_host_socket, (struct sockaddr *) &client_addr, &addrsize);
		if (clientSocket == -1) {
			/* accept failed, but life goes on... */
		} else if (clientSocket != 0) {
			fd_set rfds;
			struct timeval tv;
			struct pptp_header ph;
			int len;

			/*
			 * DOS protection: get a peek at the first packet
			 * and do some checks on it before we continue.
			 * A 10 second timeout on the first packet seems reasonable
			 * to me,  if anything looks sus,  throw it away.
			 */

			FD_ZERO(&rfds);
			FD_SET(clientSocket, &rfds);
			tv.tv_sec = 10;
			tv.tv_usec = 0;
			if (select(clientSocket + 1, &rfds, NULL, NULL, &tv) <= 0) {
				goto dos_exit;
			}

			len = recv(clientSocket, &ph, sizeof(ph), MSG_PEEK);
			if (len == 0) {
				goto dos_exit;
			}
			if (len != sizeof(ph)) {
				goto dos_exit;
			}

			ph.length = htons(ph.length);
			ph.pptp_type = ntohs(ph.pptp_type);
			ph.magic = ntohl(ph.magic);
			ph.ctrl_type = ntohs(ph.ctrl_type);

			if (ph.length <= 0 || ph.length > PPTP_MAX_CTRL_PCKT_SIZE) {
				printf("MGR: initial packet length %d outside "
						"(0 - %d)",  ph.length, PPTP_MAX_CTRL_PCKT_SIZE);
				goto dos_exit;
			}

			if (ph.magic != PPTP_MAGIC_COOKIE) {
				printf("MGR: initial packet bad magic");
				goto dos_exit;
			}

			if (ph.pptp_type != PPTP_CTRL_MESSAGE) {
				printf("MGR: initial packet has bad type");
				goto dos_exit;
			}

			if (ph.ctrl_type != START_CTRL_CONN_RQST) {
				printf("MGR: initial packet has bad ctrl type "
						"0x%x", ph.ctrl_type);
	dos_exit:
				printf("close clientSocket.\n");
				close(clientSocket);
				goto NEXT1;
			}

			if ((sp= add_pppd(SPPP_PPTPD)) != NULL)
			{
				if(-1 == start_pppd(sp, clientSocket))
					return 0;
				if (clientSocket > maxPptpConnFd)
					maxPptpConnFd = clientSocket;
				FD_SET(clientSocket, &pptpConnSet);
				/* clientSocket is active now, so set in current FdSet */
				FD_SET(clientSocket, &in);
			}
		}		/* FD_ISSET(hostSocket, &connSet) */
	}

NEXT1:
	sq = spppq;
	while(sp = sq) {
		sq = sq->pp_next;
		
		if (sp->over != SPPP_PPTPD) {
			continue;
		}

		p = sp->pp_lowerp;
		if (NULL == p) {
			continue;
		}

		send_packet = FALSE;
		outCallRecvd = FALSE;
		
		if (FD_ISSET(p->ctrl_sock, &in))
		{
			//printf("receive message, sock = %d.\n",p->ctrl_sock);
			
			send_packet = TRUE;
			switch (read_pptp_packet(sp, packet, rply_packet, &rply_size)) {
			case 0:
				syslog(LOG_ERR, "CTRL: CTRL read failed");
				goto DEL_PPPD;

			case -1:
				send_packet = FALSE;
				break;

			case STOP_CTRL_CONN_RQST:
				send_pptp_packet(sp, rply_packet, rply_size);
				goto DEL_PPPD;

			case CALL_CLR_RQST:
				printf("CTRL: Received CALL CLR request (closing call)\n");
				if (sp->fd != -1) {
					remove_fd(sp->fd);
					close(sp->fd);
					sp->fd = -1;
				}
				NOTE_VALUE(PAC, p->call_id_pair, htons(-1));
				NOTE_VALUE(PNS, p->call_id_pair, htons(-1));
				break;

			case OUT_CALL_RQST:
				/* for killing off the link later (ugly) */
				NOTE_VALUE(PAC, p->call_id_pair, ((struct pptp_out_call_rply *) (rply_packet))->call_id);
				NOTE_VALUE(PNS, p->call_id_pair, ((struct pptp_out_call_rply *) (rply_packet))->call_id_peer);
				/* Start the call */
				printf("CTRL: Starting call (launching pppd, opening GRE)\n");
				pptp_gre_init(sp);
				outCallRecvd = TRUE;
				break;

			case ECHO_RPLY:
				if (p->echo_wait == TRUE && ((struct pptp_echo_rply *) (packet))->identifier == p->echo_count)
					p->echo_wait = FALSE;
				else
					printf("CTRL: Unexpected ECHO REPLY packet\n");
				/* FALLTHRU */
			case SET_LINK_INFO:
				send_packet = FALSE;
				break;

			case CALL_DISCONN_NTFY:
			case STOP_CTRL_CONN_RPLY:
				/* These don't generate replies.  Also they come from things we don't send in this section. */
				syslog(LOG_WARNING, "CTRL: Got a reply to a packet we didn't send");
				send_packet = FALSE;
				break;

			/* Otherwise, the already-formed reply will do fine, so send it */
			}

		}
		
		/* send reply packet - this may block, but it should be very rare */
		if (send_packet == TRUE && send_pptp_packet(sp, rply_packet, rply_size) < 0) {
DEL_PPPD:
			printf("%s del pppd %d\n", __func__, sp->unit);
			del_pppd(sp->unit);
			continue;
		}

		if (TRUE == outCallRecvd) {
			/* start pppd now */
			start_pppd_real(sp);
		}
	}
}				/* pptp_manager() */

/*
 * Author: Kevin Thayer
 * 
 * This creates a socket to listen on, sets the max # of pending connections and 
 * various other options.
 * 
 * Returns the fd of the host socket.
 * 
 * The function return values are:
 * 0 for sucessful
 * -1 for bad socket creation
 * -2 for bad socket options
 * -3 for bad bind
 * -4 for bad listen
 */
int createPptpHostSocket(int *hostSocket)
{
	int opt = 1;
	int flags;
	struct sockaddr_in address;

	/* create the master socket and check it worked */
	if ((*hostSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	fcntl(*hostSocket, F_SETFD, fcntl(*hostSocket, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
	/* set master socket to allow daemon to be restarted with connections active  */
	if (setsockopt(*hostSocket, SOL_SOCKET, SO_REUSEADDR,
		       (char *) &opt, sizeof(opt)) < 0)
		return -2;

#if 0
	flags = fcntl(*hostSocket, F_GETFL);
	if (flags == -1 || fcntl(*hostSocket, F_SETFL, flags | O_NONBLOCK) == -1)
	   	printf("Couldn't set vpn host socket to nonblock\n");
#endif

	/* set up socket */
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(PPTP_PORT);

	/* bind the socket to the pptp port */
	if (bind(*hostSocket, (struct sockaddr *) &address, sizeof(address)) < 0)
		return -3;

	/* minimal backlog to avoid DoS */
	if (listen(*hostSocket, 3) < 0)
		return -4;

	printf("%s hostSocket %d for PPTP server.\n", __func__, *hostSocket);

	return 0;
}


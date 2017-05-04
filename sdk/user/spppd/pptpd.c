#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <time.h>
#include <memory.h>
#include <utmp.h>
#include <mntent.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <unistd.h>

#include <asm/types.h>		/* glibc 2 conflicts with linux/types.h */
#include <linux/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <netinet/if_ether.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <linux/ppp_defs.h>
#include <linux/if_ppp.h>
#include <linux/version.h>
#include <linux/kdev_t.h>

#include <netdb.h>
#include "if_sppp.h"
#include "pptpmanager.h"


int pptp_server_init(struct sppp *sp, int clientSocket)
{
	PPTPD_DRV_CTRL *p;
	struct sockaddr_in addr;
	socklen_t addrlen;
	int flags;

	printf("%s enter.\n", __func__);
	
	p = (PPTPD_DRV_CTRL *)malloc(sizeof(PPTPD_DRV_CTRL));
	if (!p)
		return -1;
	memset (p, 0, sizeof(PPTPD_DRV_CTRL));
	p->sppp = (void *)sp;
	
	p->fd = -1;
	p->ctrl_sock = -1;
	p->account_idx = -1;
			
	sp->pp_lowerp = (void *)p;
	
	p->ctrl_sock = clientSocket;

	/* Create I/O buffers */
	p->read_size = 0;
	p->read_alloc= 512;
	p->read_buffer = malloc(sizeof(*(p->read_buffer)) * p->read_alloc);

	addrlen = sizeof(addr);
	if (getsockname(p->ctrl_sock, (struct sockaddr *) &addr, &addrlen) != 0) {
		return -1;
	}
	p->inetaddrs[0] = addr.sin_addr;

	addrlen = sizeof(addr);
	if (getpeername(p->ctrl_sock, (struct sockaddr *) &addr, &addrlen) != 0) {
		return -1;
	}
	p->inetaddrs[1] = addr.sin_addr;

	/* Set non-blocking */
	if ((flags = fcntl(p->ctrl_sock, F_GETFL)) == -1 ||
			fcntl(p->ctrl_sock, F_SETFL, flags|O_NONBLOCK) == -1) {
		return -1;
	}

	p->fd = socket(AF_PPPOX,SOCK_STREAM,PX_PROTO_PPTP);
	if (p->fd < 0)	{
		printf("Failed to create PPTPD socket\n");
		return -1;
	}

	flags = fcntl(p->fd, F_GETFL);	// Jenny
	if (flags == -1 || fcntl(p->fd, F_SETFL, flags | O_NONBLOCK) == -1)
		printf("Couldn't set PPTPD socket to nonblock\n");

	printf("%s exit.\n", __func__);

	return p->fd;
}


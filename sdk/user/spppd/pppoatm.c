// NOTE // NOTE // NOTE // NOTE // NOTE // NOTE // NOTE // NOTE // NOTE //
//
// This is only meant as an example of the new pppd plugins right now...
// it won't work with the current PPPoATM code for the kernel (as
// distributed by Jens Axboe), it is meant to work with a new version
// of the in-kernel PPPoATM backend which I am working on.  As such
// it will neither compile nor work in this present form.
//				-- mitch@sfgoth.com
//
// NOTE // NOTE // NOTE // NOTE // NOTE // NOTE // NOTE // NOTE // NOTE //

/* pppoatm.c - pppd plugin to implement PPPoATM protocol.
 *
 * Copyright 2000 Mitchell Blank Jr.
 * Based in part on work from Jens Axboe and Paul Mackerras.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 */
#include "pppoa.h"
#include <unistd.h>
#include <fcntl.h>


int pppoatm_init(struct pppoa_param_s *poar)
{
	int fd;
	struct atm_backend_ppp be;
	int ret;

	//printf("PPPoATM - open device\n");	
	
  	fd = socket(AF_ATMPVC, SOCK_DGRAM, 0);
  	if (fd < 0)
    	printf("failed to create socket: %m");

	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC); // FD_CLOEXEC: Let child process won't inherit this fd
  	if (setsockopt(fd, SOL_ATM, SO_ATMQOS, &poar->qos, sizeof(struct atm_qos)) < 0)
    	printf("setsockopt(SO_ATMQOS): %m");

	ret = connect(fd, (struct sockaddr *) &poar->addr,
    	sizeof(struct sockaddr_atmpvc));
  	if (ret)
    	printf("connect(pppoatm): %m %d", ret);

  
	be.backend_num = ATM_BACKEND_PPP;
	if (poar->encaps==0)	// VC_MUX
		be.encaps = PPPOATM_ENCAPS_VC;
	else // 1: LLC
		be.encaps = PPPOATM_ENCAPS_LLC;
  	if (ioctl(fd, ATM_SETBACKEND, &be) < 0)
    	printf("ioctl(ATM_SETBACKEND): %m");

	//printf("PPPoATM - device got set\n");
	/* backup fd, since reopen /dev/ppp will get a new fd */
	poar->fd = fd;
	
	return fd;
}

int pppoatm_close(struct pppoa_param_s *poar)
{
	close(poar->fd);
	poar->fd = -1;
	return 0;
}

#ifndef _PPPOA_H_
#define _PPPOA_H_

#include "atm.h"
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <sys/stat.h>
#include <linux/if.h>
#include <sys/ioctl.h>

#define SPPP_PPPOATM	0

#define pppoatm_overhead(x) (x ? 6 : 2)

struct pppoa_param_s {
	int fd;
	int encaps;
	struct sockaddr_atmpvc	addr;
	struct atm_qos			qos;
};

int pppoatm_init(struct pppoa_param_s *poar);

#endif

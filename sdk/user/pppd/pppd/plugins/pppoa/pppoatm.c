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
#include "pppd.h"
#include "paths.h"
#include "fsm.h" /* Needed for lcp.h to include cleanly */
#include "lcp.h"
#include <atm.h>
#include <linux/atmdev.h>
#include <linux/atmppp.h>
#include <sys/stat.h>
#include <net/if.h>
#include <sys/ioctl.h>

static struct sockaddr_atmpvc pvcaddr;
static char *qosstr = NULL;
static int pppoatm_accept = 0;
static bool llc_encaps = 0;
static bool vc_encaps = 0;
static int device_got_set = 0;
static int device_opened = 0;
static int pppoatm_max_mtu, pppoatm_max_mru;
static int pppoatm_fd = 0;

static int setdevname_pppoatm(const char *cp);

static option_t my_options[] = {
#if 0
	{ "accept", o_bool, &pppoatm_accept,
	  "set PPPoATM socket to accept incoming connections", 1 },
#endif
	/*
	{ "pvc", o_special, (void *) &setdevname_pppoatm,
	  "PPPoA device name",
	  OPT_PRIVFIX | OPT_NOARG  | OPT_STATIC,
	  devnam},
	*/  
	{ "llc-encaps", o_bool, &llc_encaps,
	  "use LLC encapsulation for PPPoATM", 1},
	{ "vc-encaps", o_bool, &vc_encaps,
	  "use VC multiplexing for PPPoATM (default)", 1},
	{ "qos", o_string, &qosstr,
	  "set QoS for PPPoATM connection", 1},
	{ NULL }
};

/* returns:
 *  -1 if there's a problem with setting the device
 *   0 if we can't parse "cp" as a valid name of a device
 *   1 if "cp" is a reasonable thing to name a device
 * Note that we don't actually open the device at this point
 * We do need to fill in:
 *   devnam: a string representation of the device
 *   devstat: a stat structure of the device.  In this case
 *     we're not opening a device, so we just make sure
 *     to set up S_ISCHR(devstat.st_mode) != 1, so we
 *     don't get confused that we're on stdin.
 */
static int setdevname_pppoatm(const char *cp)
{
	struct sockaddr_atmpvc addr;
	extern struct stat devstat;
	printf("PPPoATM setdevname_pppoatm. (get vpi.vci) %s\n", cp);

	if (device_got_set)
    	return 0;

	memset(&addr, 0, sizeof addr);
	if (text2atm(cp, (struct sockaddr *) &addr, sizeof(addr), T2A_PVC | T2A_NAME) < 0)
		return 1;

	memcpy(&pvcaddr, &addr, sizeof pvcaddr);
	strlcpy(devnam, cp, sizeof devnam);
	devstat.st_mode = S_IFSOCK;

	printf("PPPoATM setdevname_pppoatm - SUCCESS\n");

	device_got_set = 1;

	return 0;
}

static int setspeed_pppoatm(const char *cp)
{
	return 0;
}

static int options_for_pppoatm(void)
{
	//return options_from_devfile(_PATH_ATMOPT, devnam);
	return 0;
}

#define pppoatm_overhead() (llc_encaps ? 6 : 2)

static int open_device_pppoatm(void)
{
  int fd;
  struct atm_qos qos;
  int ret;

  printf("PPPoATM - open device\n");
  if(device_opened)
  	fd = pppoatm_fd;
  else
  {
  	
  if (!device_got_set)
    no_device_given_pppoatm();

  fd = socket(AF_ATMPVC, SOCK_DGRAM, 0);
  if (fd < 0)
    fatal("failed to create socket: %m");
  }

  memset(&qos, 0, sizeof qos);
  qos.txtp.traffic_class = qos.rxtp.traffic_class = ATM_UBR;
  /* TODO: support simplified QoS setting */
  if (qosstr != NULL)
    if (text2qos(qosstr, &qos, 0))
      fatal("Can't parse QoS: \"%s\"");
  qos.txtp.max_sdu = 1500 + pppoatm_overhead();
  qos.rxtp.max_sdu = 1500 + pppoatm_overhead();
  qos.aal = ATM_AAL5;

  if (setsockopt(fd, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0)
    fatal("setsockopt(SO_ATMQOS): %m");

	ret = connect(fd, (struct sockaddr *) &pvcaddr,
    	sizeof(struct sockaddr_atmpvc));
  if (ret)
    fatal("connect(%s): %m %d", devnam, ret);
  pppoatm_max_mtu = 1500;
  pppoatm_max_mru = 1500;

  set_line_discipline_pppoatm(fd);
  pppoatm_fd = fd;
	device_opened = 1;  
printf("PPPoATM - device got set\n");
  return fd;
}

static void post_open_setup_pppoatm(void)
{
	/* NOTHING */
}

static void pre_close_restore_pppoatm(void)
{
	/* NOTHING */
}

static void no_device_given_pppoatm(void)
{
	fatal("No vpi.vci specified");
}

static void disconnect_device_pppoatm(void)
{
  /* NOTHING */
  if(device_opened) {
	  close(pppoatm_fd);
	  device_opened = 0;
  	printf("PPPOATM: disconnect_device_pppoatm\n");
  }
}


static void set_line_discipline_pppoatm(int fd)
{
  struct atm_backend_ppp be;
  
  be.backend_num = ATM_BACKEND_PPP;
  if (!llc_encaps)
    be.encaps = PPPOATM_ENCAPS_VC;
  else if (!vc_encaps)
    be.encaps = PPPOATM_ENCAPS_LLC;
  else
    be.encaps = PPPOATM_ENCAPS_AUTODETECT;
  if (ioctl(fd, ATM_SETBACKEND, &be) < 0)
    fatal("ioctl(ATM_SETBACKEND): %m");
}

static void reset_line_discipline_pppoatm(int fd)
{
  atm_backend_t be = ATM_BACKEND_RAW;
  /* 2.4 doesn't support this yet */
  (void) ioctl(fd, ATM_SETBACKEND, &be);
}

static void send_config_pppoatm(int unit, int mtu, u_int32_t asyncmap, int pcomp, int accomp)
{
	int sock;
	struct ifreq ifr;
	if (mtu > pppoatm_max_mtu)
		error("Couldn't increase MTU to %d", mtu);
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0)
		fatal("Couldn't create IP socket: %m");
	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	ifr.ifr_mtu = mtu;
	if (ioctl(sock, SIOCSIFMTU, (caddr_t) &ifr) < 0)
		fatal("ioctl(SIOCSIFMTU): %m");
	(void) close (sock);
}

static void recv_config_pppoatm(int unit, int mru, u_int32_t asyncmap, int pcomp, int accomp)
{
	if (mru > pppoatm_max_mru)
		error("Couldn't increase MRU to %d", mru);
}

static void set_xaccm_pppoatm(int unit, ext_accm accm)
{
	/* NOTHING */
}

#ifdef EMBED
void pppoa_plugin_init(void)
#else
void plugin_init(void)
#endif
{

#if _linux_
	extern int new_style_driver;	/* From sys-linux.c */
#endif
	static char *bad_options[] = {
		"noaccomp", "-ac",
		"default-asyncmap", "-am", "asyncmap", "-as", "escape",
		"receive-all",
		"crtscts", "-crtscts", "nocrtscts",
		"cdtrcts", "nocdtrcts",
		"xonxoff",
		"modem", "local", "sync",
		NULL };

#ifndef EMBED
#if _linux_
    if (!new_style_driver)
	fatal("Kernel doesn't support ppp_generic needed for PPPoE");
#else
    fatal("No PPPoE support on this OS");
#endif
#endif /*EMBED*/
	
	printf("PPPoATM plugin_init\n");
	add_options(my_options);

	tty_open_hook = open_device_pppoatm;
	tty_close_hook = disconnect_device_pppoatm;
	setdevname_hook = &setdevname_pppoatm;
	establish_ppp_hook = &generic_establish_ppp;
	disestablish_ppp_hook = &generic_disestablish_ppp;
	ppp_send_config_hook = &send_config_pppoatm;
	ppp_recv_config_hook = &recv_config_pppoatm;

	{
		char **a;
		for (a = bad_options; *a != NULL; a++)
			remove_option(*a);
	}
	
	modem = 0;
	lcp_wantoptions[0].neg_accompression = 0;
	lcp_allowoptions[0].neg_accompression = 0;
	lcp_wantoptions[0].neg_asyncmap = 0;
	lcp_allowoptions[0].neg_asyncmap = 0;
	lcp_wantoptions[0].neg_pcompression = 0;
}

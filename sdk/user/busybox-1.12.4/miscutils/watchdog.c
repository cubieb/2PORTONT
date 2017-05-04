/* vi: set sw=4 ts=4: */
/*
 * Mini watchdog implementation for busybox
 *
 * Copyright (C) 2003  Paul Mundt <lethal@linux-sh.org>
 * Copyright (C) 2006  Bernhard Fischer <busybox@busybox.net>
 *
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 */

// Mason Yu. Patch from old busybox
#if 1  // This is a old watchdog from old busybox
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "busybox.h"

#ifdef _LINUX_2_6_
#define SIOCETHTEST	0x89b1
#else
#define SIOCETHTEST	0x89a1
#endif
struct arg{
	unsigned char cmd; 
	unsigned int cmd2;
	unsigned int cmd3;
	unsigned int cmd4;
}pass_arg;

void usage(void)
{
	printf("watchdog usage:\n");
	printf("  start n:start watchdog and kick every n seconds.\n");
	printf("  on:     watchdog enable\n");
	printf("  off:    watchdog disable\n");
	printf("  kick n:  set watchdog to kick every n seconds\n");
	printf("  timeout n:  system reset if watchdog task not kick in n seconds\n");
	printf("  status: get watchdog status\n");
}

int send_cmd(int fd, struct ifreq *ifr, int cmd2, int cmd3)
{
    pass_arg.cmd=14;  //watchdog command
    pass_arg.cmd2=cmd2;
    pass_arg.cmd3=cmd3;
	return ioctl(fd, SIOCETHTEST, ifr);
}

#define WDG_RUNFILE	"/var/run/wdg.pid"
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = WDG_RUNFILE;
	
	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

extern int watchdog_main(int argc, char **argv)
{
  struct ifreq		ifr;
  int fd=0;
  int duration;
    
	if ((argc != 2)&&(argc != 3)) {
		usage();
	    goto watchdog_fail;
	}

	fd = socket(AF_INET, SOCK_DGRAM, 0);
  	if(fd< 0){
		printf("Watchdog control fail!\n");
		goto watchdog_fail;
  	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ifr.ifr_data = (void *)&pass_arg;
	if (strcmp(argv[1],"start")==0) {
	    if (argc != 3) {
	    	usage();
	    	goto watchdog_fail;
	    };
	    //enable hw watchdog
	    duration = atoi(argv[2]);
	    if (send_cmd(fd, &ifr, 2, duration) < 0) {
	        printf("Watchdog enable fail!\n");
		    goto watchdog_fail;
	    };
	    pass_arg.cmd3 = duration;  //init sleep time
	    log_pid();
	    while (1) {
	        //kick watchdog
	        //pass_arg.cmd3 is changed in kernel if user use "set n"
	        //to change kick cycle time
	        if (send_cmd(fd, &ifr, 1, pass_arg.cmd3) < 0) {
		        printf("Watchdog kick fail!\n");
		        break;
	        };
		    sleep(pass_arg.cmd3);
	    }
	}
	if (strcmp(argv[1],"on")==0) {  //enable hw watchdog
	    if (send_cmd(fd, &ifr, 2, 0) < 0) {
	        printf("Watchdog enable fail!\n");
		    goto watchdog_fail;
	    };
	}
	if (strcmp(argv[1],"off")==0) {  //disable hw watchdog
	    if (send_cmd(fd, &ifr, 3, 0) < 0) {
	        printf("Watchdog disable fail!\n");
		    goto watchdog_fail;
	    };
	}
	if (strcmp(argv[1],"kick")==0) {  //set kick cycle time
	    if (argc != 3) {
	    	usage();
	    	goto watchdog_fail;
	    };
	    //set kick cycle time
	    duration = atoi(argv[2]);
	    if (send_cmd(fd, &ifr, 4, duration) < 0) {
	        printf("Watchdog set cycle time fail!\n");
		    goto watchdog_fail;
	    };
	}
	if (strcmp(argv[1],"status")==0) {  //print status
	    if (send_cmd(fd, &ifr, 5, 0) < 0) {
	        printf("Watchdog get status fail!\n");
		    goto watchdog_fail;
	    };
	}
	if (strcmp(argv[1],"timeout")==0) {  //set timeout time
	    if (argc != 3) {
	    	usage();
	    	goto watchdog_fail;
	    };
	    //set timeout time
	    duration = atoi(argv[2]);
	    if (send_cmd(fd, &ifr, 6, duration) < 0) {
	        printf("Watchdog set timeout time fail!\n");
		    goto watchdog_fail;
	    };
	}
		
watchdog_fail:
	return EXIT_FAILURE;
}

#else  // Mason Yu. Patch from old busybox
// Mason Yu. This is a original watchdog on busybox-1.12.4

#include "libbb.h"

#define OPT_FOREGROUND 0x01
#define OPT_TIMER      0x02

static void watchdog_shutdown(int sig UNUSED_PARAM)
{
	static const char V = 'V';

	write(3, &V, 1);	/* Magic, see watchdog-api.txt in kernel */
	if (ENABLE_FEATURE_CLEAN_UP)
		close(3);
	exit(EXIT_SUCCESS);
}

int watchdog_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int watchdog_main(int argc, char **argv)
{
	unsigned opts;
	unsigned timer_duration = 30000; /* Userspace timer duration, in milliseconds */
	char *t_arg;

	opt_complementary = "=1"; /* must have 1 argument */
	opts = getopt32(argv, "Ft:", &t_arg);

	if (opts & OPT_TIMER) {
		static const struct suffix_mult suffixes[] = {
			{ "ms", 1 },
			{ "", 1000 },
			{ }
		};
		timer_duration = xatou_sfx(t_arg, suffixes);
	}

	if (!(opts & OPT_FOREGROUND)) {
		bb_daemonize_or_rexec(DAEMON_CHDIR_ROOT, argv);
	}

	bb_signals(BB_FATAL_SIGS, watchdog_shutdown);

	/* Use known fd # - avoid needing global 'int fd' */
	xmove_fd(xopen(argv[argc - 1], O_WRONLY), 3);

// TODO?
//	if (!(opts & OPT_TIMER)) {
//		if (ioctl(fd, WDIOC_GETTIMEOUT, &timer_duration) == 0)
//			timer_duration *= 500;
//		else
//			timer_duration = 30000;
//	}

	while (1) {
		/*
		 * Make sure we clear the counter before sleeping, as the counter value
		 * is undefined at this point -- PFM
		 */
		write(3, "", 1); /* write zero byte */
		usleep(timer_duration * 1000L);
	}
	return EXIT_SUCCESS; /* - not reached, but gcc 4.2.1 is too dumb! */
}
#endif

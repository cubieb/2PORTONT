/* vi: set sw=4 ts=4: */
/*
 * Mini watchdog implementation for busybox
 *
 * Copyright (C) 2000  spoon <spoon@ix.netcom.com>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/* getopt not needed */
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

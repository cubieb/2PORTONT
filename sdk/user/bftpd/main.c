/*

bftpd Copyright (C) 1999-2003 Max-Wilhelm Bruker

This program is is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2 of the
License as published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

*/

#include <config.h>
#include <stdio.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_ASM_SOCKET_H
#include <asm/socket.h>
#endif
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#ifdef HAVE_WAIT_H
# include <wait.h>
#else
# ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
# endif
#endif

#include "main.h"
#include "cwd.h"
#include "mystring.h"
#include "logging.h"
#include "dirlist.h"
#include "bftpdutmp.h"
#include "options.h"
#include "login.h"
#include "list.h"
#include "debug.h"

int global_argc;
char **global_argv;
char **my_argv_list;   // jesse
struct sockaddr_in name;
int isparent = 1;
int listensocket=-1, sock;
FILE *passwdfile = NULL, *groupfile = NULL, *devnull;
struct sockaddr_in remotename;
char *remotehostname;
int control_timeout, data_timeout;
int alarm_type = 0;

struct bftpd_list_element *child_list;

/* Command line parameters */
char *configpath = PATH_BFTPD_CONF;
int daemonmode = 0;

void print_file(int number, char *filename)
{
	FILE *phile;
	char foo[256];
	phile = fopen(filename, "r");
	if (phile) {
		while (fgets(foo, sizeof(foo), phile)) {
			foo[strlen(foo) - 1] = '\0';
			control_printf(SL_SUCCESS, "%i-%s", number, foo);
		}
		fclose(phile);
	}
}

void end_child()
{
	if (passwdfile)
		fclose(passwdfile);
	if (groupfile)
		fclose(groupfile);
	config_end();
	bftpd_log("Quitting.\n");
    bftpd_statuslog(1, 0, "quit");
    bftpdutmp_end();
	log_end();
	login_end();
	bftpd_cwd_end();
	if (daemonmode) {
		close(sock);
		close(0);
		close(1);
		close(2);
	}
}



/*
This function causes the program to 
re-read parts of the config file.

-- Jesse
*/
void handler_sighup(int sig)
{
   bftpd_log("Caught HUP signal. Re-reading config file.\n");
   Reread_Config_File();
   signal(sig, handler_sighup);
}

/* lijian : 20080716 bug-id 3649 : kill all child processes before terminated */
void handler_sigterm_p(int signum)
{
	int i;
	struct bftpd_childpid *childpid; 
    
	signal(SIGCHLD, SIG_IGN);
    
	/* Notify all child process to exit */
	for (i = 0; i < bftpd_list_count(child_list); i++)
	{
		childpid = bftpd_list_get(child_list, i);
		kill(childpid->pid, SIGTERM);
	}

	exit(0);
}


void handler_sigchld(int sig)
{
	pid_t pid;
	int i;
	struct bftpd_childpid *childpid; 

        /* Get the child's return code so that the zombie dies */
	pid = wait(NULL);   
	for (i = 0; i < bftpd_list_count(child_list); i++) {
		childpid = bftpd_list_get(child_list, i);
		if (childpid->pid == pid) {
			close(childpid->sock);
			bftpd_list_del(&child_list, i);
			free(childpid);
                        /* make sure the child is removed from the log */
                        bftpdutmp_remove_pid(pid);
		}
	}
}

void handler_siguser1(int sig)
{
	int i,num;
	struct bftpd_childpid *childpid; 

	/* Notify all child process to exit usb directory*/
	for (i = 0; i < bftpd_list_count(child_list); i++)
	{
		childpid = bftpd_list_get(child_list, i);
		kill(childpid->pid, SIGUSR1);
	}
	
#if 0
	signal(SIGCHLD, SIG_IGN);
	num = bftpd_list_count(child_list);
        /* Get the child's return code so that the zombie dies */
	for (i = 0; i < num; i++) {
		if(!child_list)	break;
		childpid = bftpd_list_get(child_list, 0);
		if(childpid){
			kill(childpid->pid, SIGTERM);
			close(childpid->sock);
	                /* make sure the child is removed from the log */
	                bftpdutmp_remove_pid(childpid->pid);
			free(childpid);
			bftpd_list_del(&child_list, 0);
		}
	}
	signal(SIGCHLD, handler_sigchld);
#endif
}

void childHandler_singuser1(int sig)
{
	chdir("/");
}
void handler_sigterm(int signum)
{
        bftpdutmp_end();
	exit(0);	/* Force normal termination so that end_child() is called */
}

void handler_sigalrm(int signum)
{
    /* Log user out. -- Jesse <slicer69@hotmail.com> */
    bftpdutmp_end();

    if (alarm_type) {
        close(alarm_type);
        bftpd_log("Kicked from the server due to data connection timeout.\n");
        control_printf(SL_FAILURE, "421 Kicked from the server due to data connection timeout.");
        exit(0);
    } else {
        bftpd_log("Kicked from the server due to control connection timeout.\n");
        control_printf(SL_FAILURE, "421 Kicked from the server due to control connection timeout.");
        exit(0);
    }
}

void init_everything()
{
	if (!daemonmode) {
		config_init();
        hidegroups_init();
    }
	log_init();
    bftpdutmp_init();
	login_init();
}

pid_t self_pid;

char *get_config_opt(FILE *fp_config, char *buf, const char *opt_name)
{
	char *ret;
	ret =NULL;
	if (fp_config == NULL)
	{
		DEBUG_PRINT("fp_config=NULL\n");
		return ret;
	}

	fseek(fp_config, 0, SEEK_SET);

	while ( fgets(buf, 40, fp_config) )
	{
		DEBUG_PRINT("fgets:%s\n", buf);
		if ( (strncmp(buf, opt_name, strlen(opt_name)) == 0) && (buf[strlen(opt_name)] == '=') )
		{
			ret = &(buf[strlen(opt_name)+1]);
			if ( (*ret) == 0x0a )
			{
				ret = NULL;
			}
			else
			{
				DEBUG_PRINT("get the opt: %s=%s\n", opt_name, ret);
			}
			break;
		}
	}

	return ret;
}

int main(int argc, char **argv)
{
	char str[MAXCMD + 1];
	static struct hostent *he;
	int i = 1, port;
	int retval;
        socklen_t my_length;
	int bindCount;
	int fp;

	/*xl_yue: 20080728 resovle bug-bugID:0003705. open some file at first if stdin/stdout/stderr is closed*/
	do{
		fp = open("/dev/null", O_RDWR);
	}while(fp < 3);
	close(fp);
	/*xl_yue: 20080728*/
	
	cdh_debug_init();
	DEBUG_PRINT("bftpd main...");
        my_argv_list = argv;
        signal(SIGHUP, handler_sighup);

	while (((retval = getopt(argc, argv, "c:hdDin"))) > -1) {
		switch (retval) {
			case 'h':
				printf(
					"Usage: %s [-h] [-i|-d|-D] [-c <filename>|-n]\n"
					"-h print this help\n"
					"-i (default) run from inetd\n"
					"-d daemon mode: fork() and run in TCP listen mode\n"
					"-D run in TCP listen mode, but don't pre-fork()\n"
					"-c read the config file named \"filename\" instead of " PATH_BFTPD_CONF "\n"
					"-n no config file, use defaults\n", argv[0]);
				return 0;
			case 'i': daemonmode = 0; break;
			case 'd': daemonmode = 1; break;
			case 'D': daemonmode = 2; break;
			case 'c': configpath = strdup(optarg); break;
			case 'n': configpath = NULL; break;
		}
	}
	if (daemonmode) {
		struct sockaddr_in myaddr, new;
		if (daemonmode == 1) {
			if (fork())
				exit(0);  /* Exit from parent process */
			setsid();
			if (fork())
				return 0;
		}
		signal(SIGCHLD, handler_sigchld);

		/*xl_yue:20080730 kill all child process if SIGUSR1 received */
		signal(SIGUSR1, handler_siguser1);
        
		/* lijian : 20080716 bug-id 3649 : kill all child processes before terminated */
		signal(SIGTERM, handler_sigterm_p);
        
		config_init();
		chdir("/");
                hidegroups_init();
				
		listensocket = socket(AF_INET, SOCK_STREAM, 0);
		
		DEBUG_PRINT("listensocket = %d\n",listensocket);

#ifdef SO_REUSEADDR
		setsockopt(listensocket, SOL_SOCKET, SO_REUSEADDR, (void *) &i,
				   sizeof(i));
#endif
#ifdef SO_REUSEPORT
		setsockopt(listensocket, SOL_SOCKET, SO_REUSEPORT, (void *) &i,
				   sizeof(i));
#endif
		memset((void *) &myaddr, 0, sizeof(myaddr));
        if (!((port = strtoul(config_getoption("PORT"), NULL, 10))))
            port = DEFAULT_PORT;
		myaddr.sin_port = htons(port);
		DEBUG_PRINT("port = %d\n", port);
		// write the pid to a file
		{
			FILE *fp_config;
			char strtemp[64];
			long pos;

			fp_config = fopen("/var/run/bftpd/ftpdconfig", "r+");
			if (fp_config == NULL)
			{
				DEBUG_PRINT("Fail to open \"/var/run/bftpd/ftpdpassword\"!\n");
				return -1;
			}

			self_pid = getpid();

			DEBUG_PRINT("self_pid = %d\n",self_pid);

			pos = ftell(fp_config);
			while ( fgets(strtemp, 62, fp_config) )
			{
				if ( ! strncmp(strtemp, "bftpd_pid=", 10) )
				{
					fseek(fp_config, pos, SEEK_SET);
					fprintf(fp_config, "bftpd_pid=%d\n", self_pid);
					DEBUG_PRINT("change bftpd_pid=%d\n", self_pid);
					pos = -1;
					break;
				}
				pos = ftell(fp_config);
			}

			if (pos != -1)
			{
				fseek(fp_config, 0, SEEK_END);
				fprintf(fp_config, "bftpd_pid=%d\n", self_pid);
				DEBUG_PRINT("bftpd_pid=%d\n", self_pid);
			}

			/*xl_yue:20080730 support port config by ITMS*/
			{
				char * pPort;
				int ftp_port;
				char tmpBuf[42];
				
				pPort = get_config_opt(fp_config, tmpBuf, "ftp_port");
				if(sscanf(pPort, "%d", &ftp_port) >= 0){
					myaddr.sin_port = htons(ftp_port);
				}
			}
			/*xl_yue:20080730*/
			
			fclose(fp_config);
		}
		if (!strcasecmp(config_getoption("BIND_TO_ADDR"), "any")
			|| !config_getoption("BIND_TO_ADDR")[0])
			myaddr.sin_addr.s_addr = INADDR_ANY;
		else
			myaddr.sin_addr.s_addr = inet_addr(config_getoption("BIND_TO_ADDR"));
		
		DEBUG_PRINT("start bind...\n");

		bindCount = 0;
		do{
			retval = bind(listensocket, (struct sockaddr *) &myaddr, sizeof(myaddr));
			bindCount++;
			if(bindCount > 4)
				break;
			usleep(1000000);
		}while(retval < 0);
		
		if (retval < 0) {
			fprintf(stderr, "Bind failed: %s\n", strerror(errno));
			DEBUG_PRINT("bind error!\n");
			exit(1);
		}
#if 0		
		if (retval = bind(listensocket, (struct sockaddr *) &myaddr, sizeof(myaddr)) < 0) {
			fprintf(stderr, "Bind failed: %s\n", strerror(errno));
			exit(1);
		}
#endif

		DEBUG_PRINT("start listen...\n");

		if (retval=listen(listensocket, 5)) {
			fprintf(stderr, "Listen failed: %s\n", strerror(errno));
			DEBUG_PRINT("listen error!\n");
			exit(1);
		}
                
		DEBUG_PRINT("start accept...\n");

		for (i = 0; i < 3; i++) {
			close(i);		/* Remove fd pointing to the console */
			open("/dev/null", O_RDWR);	/* Create fd pointing nowhere */
		}
		my_length = sizeof(new);

		while ((sock = accept(listensocket, (struct sockaddr *) &new, &my_length))) {
			pid_t pid;
			/* If accept() becomes interrupted by SIGCHLD, it will return -1.
			 * So in order not to create a child process when that happens,
			 * we have to check if accept() returned an error.
			 */
			if (sock > 0) {
				pid = fork();
				if (!pid) {       /* child */
					close(0);
					close(1);
					close(2);
					isparent = 0;
					dup2(sock, fileno(stdin));
					dup2(sock, fileno(stderr));
					break;
				} else {          /* parent */
					struct bftpd_childpid *tmp_pid = malloc(sizeof(struct bftpd_childpid));
					tmp_pid->pid = pid;
					tmp_pid->sock = sock;
					bftpd_list_add(&child_list, tmp_pid);
				}
			}
		}
	}
        
        /* Child only. From here on... */

	devnull = fopen("/dev/null", "w");
	global_argc = argc;
	global_argv = argv;
	init_everything();
	atexit(end_child);
	signal(SIGTERM, handler_sigterm);
	signal(SIGALRM, handler_sigalrm);

	signal(SIGUSR1,childHandler_singuser1);

    /* If we do not have getpwnam() for some reason, then
       we must use FILE_AUTH or exit. */
    #ifdef NO_GETPWNAM
    {
        char *file_auth_option;

        file_auth_option = config_getoption("FILE_AUTH");
        if (! file_auth_option[0] )
        {
           bftpd_log("Exiting, becasue we have no way to authorize clients.\n");
           exit(0);
        }
    }
    #endif

    control_timeout = strtoul(config_getoption("CONTROL_TIMEOUT"), NULL, 0);
    if (!control_timeout)
        control_timeout = CONTROL_TIMEOUT;
    data_timeout = strtoul(config_getoption("DATA_TIMEOUT"), NULL, 0);
    if (!data_timeout)
        data_timeout = DATA_TIMEOUT;
    xfer_bufsize = strtoul(config_getoption("XFER_BUFSIZE"), NULL, 0);
    if (!xfer_bufsize)
        xfer_bufsize = XFER_BUFSIZE;

    /* get scripts to run pre and post write */
    pre_write_script = config_getoption("PRE_WRITE_SCRIPT");
    if (! pre_write_script[0])
       pre_write_script = NULL;
    post_write_script = config_getoption("POST_WRITE_SCRIPT");
    if (! post_write_script[0])
       post_write_script = NULL;


	my_length = sizeof(remotename);
    if (getpeername(fileno(stderr), (struct sockaddr *) &remotename, &my_length)) {
		control_printf(SL_FAILURE, "421-Could not get peer IP address.\r\n421 %s.",
		               strerror(errno));
		return 0;
	}
	i = 1;
	setsockopt(fileno(stdin), SOL_SOCKET, SO_OOBINLINE, (void *) &i,
			   sizeof(i));
	setsockopt(fileno(stdin), SOL_SOCKET, SO_KEEPALIVE, (void *) &i,
			   sizeof(i));
	/* If option is set, determine the client FQDN */
	if (!strcasecmp((char *) config_getoption("RESOLVE_CLIENT_IP"), "yes")) {
		if ((he = gethostbyaddr((char *) &remotename.sin_addr,
								sizeof(struct in_addr), AF_INET)))
			remotehostname = strdup(he->h_name);
		else
			remotehostname = strdup(inet_ntoa(remotename.sin_addr));
	} else
		remotehostname = strdup(inet_ntoa(remotename.sin_addr));
	bftpd_log("Incoming connection from %s.\n", remotehostname);
    bftpd_statuslog(1, 0, "connect %s", remotehostname);
	my_length = sizeof(name);
	getsockname(fileno(stdin), (struct sockaddr *) &name, &my_length);
	print_file(220, config_getoption("MOTD_GLOBAL"));
	/* Parse hello message */
	strcpy(str, (char *) config_getoption("HELLO_STRING"));
	replace(str, "%v", VERSION);
	if (strstr(str, "%h")) {
		if ((he = gethostbyaddr((char *) &name.sin_addr, sizeof(struct in_addr), AF_INET)))
			replace(str, "%h", he->h_name);
		else
			replace(str, "%h", (char *) inet_ntoa(name.sin_addr));
	}
	replace(str, "%i", (char *) inet_ntoa(name.sin_addr));
	control_printf(SL_SUCCESS, "220 %s", str);

        /* We might not get any data, so let's set an alarm before the
           first read. -- Jesse <slicer69@hotmail.com> */
        alarm(control_timeout);
        
	/* Read lines from client and execute appropriate commands */
	while (fgets(str, sizeof(str), stdin)) {
        alarm(control_timeout);
        str[strlen(str) - 2] = 0;
        bftpd_statuslog(2, 0, "%s", str);
#if 1 //def DEBUG //chenduanhai
		bftpd_log("Processing command: %s\n", str);
#endif
		parsecmd(str);
	}

        if (remotehostname) free(remotehostname);
	return 0;
}

#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <linux/config.h>
#include "uc_udp.h"
#include "uc_cmd.h"
#include "uc_mib.h"
#include "uc_sys.h"
#ifdef WITH_MDIO
#include "uc_mdio.h"
#endif /*WITH_MDIO*/

//wait child process to terminate
static void sigchld_handler(int signo)
{
	pid_t pid;
	int stat;
	
	while( (pid=waitpid(-1, &stat, WNOHANG))>0 )
	{
		//printf("child %d termniated\n", pid);
	}
}

static int ser_handle_cmd(unsigned short cmd, char* mesg, int len, int maxlen)
{
	int ret;

	if( (len<0) ||
		((len>0)&&(mesg==NULL)) ) 
		return -1;
	ret=uc_cmd_handle( (int)cmd, mesg, len, maxlen);
	return ret;
}


int ser_handle_action(void)
{
	return uc_cmd_action();
}


int ser_handle_udp(char* mesg, int len, int maxlen)
{
	int ret;

	if( (len<0) ||
		((len>0)&&(mesg==NULL)) ) 
		return -1;

	ret=uc_udp_handle_mesg( mesg, len, maxlen, ser_handle_cmd);
	return ret;
}

static int uc_server_start(void)
{
#ifdef CONFIG_NET
	int s;

	printf( "%s(): with UDP server\n", __FUNCTION__ );
	s=uc_udp_bind(NULL);
	if(s<0) return -1;
	uc_udp_loop( s, ser_handle_udp, ser_handle_action);
#else /*CONFIG_NET*/	
	printf( "%s(): without UDP server\n", __FUNCTION__ );
	while(1) pause();
#endif /*CONFIG_NET*/

	return 0;
}

int main(int argc, char **argv)
{
	// become daemon
	if (daemon(0,1) == -1) 
	{
		perror("daemon()");
		return -1;
	}

	signal(SIGCHLD, sigchld_handler);

	//init mib
	uc_mib_init();


#ifdef WITH_MDIO
	//init mdio
	signal(SIGUSR1, mdio_event_handler);	
	if( mdio_open()<0 ) return -1;
	if( mdio_set_host_pid()<0 ) return -1;
	
#ifndef _USE_NFBI_WAY_
{
	unsigned char *p;
	p=mdio_wait_netinfo(5);
	if(p)
	{
		printf( "use net settings from master\n" );
		uc_mib_set( UC_MIB_ELAN_MAC_ADDR, p );
		uc_mib_set( UC_MIB_ADSL_LAN_IP, p+6 );
		uc_mib_set( UC_MIB_ADSL_LAN_SUBNET, p+10 );
	}else
		printf( "use default net settings\n" );
}
#endif /*_USE_NFBI_WAY_*/
#endif /*WITH_MDIO*/


#ifndef _USE_NFBI_WAY_
	//init eth0
	uc_sys_eth_init();
#endif /*_USE_NFBI_WAY_*/


	//start server
	return uc_server_start();
}


#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include "uc_udp.h"
#ifdef _USE_NFBI_WAY_ 
#include "linux-2.6.x/drivers/char/rtl867x_nfbi/rtl867x_nfbi.h"
#define NFBI_DEV_NAME  ("/dev/rtl_nfbi")
static int nfbi_fd;
#else  /*_USE_NFBI_WAY_*/
static unsigned char ser_ip[32];
#endif /*_USE_NFBI_WAY_*/


#define UC_ERR_CON_SRV	0xffff
#define TRY_TIME		10


enum UCDSTAT{
	eUCD_DEAD=0,
	eUCD_ALIVE=1
};
static int ucd_status=eUCD_ALIVE;
static time_t t_failed;


////////////////////////////////////////////////////////////////////////
static int uc_relay_resp_fail(char *m, int len)
{
	int ret;
	UC_HDR *ph;

	if( (m==NULL) || (len<=0) ) return -1;
	if( len<UC_HDR_SIZE) return -1;
	ph=(UC_HDR *)m;
	if(ph->magic!=UC_HDR_MAGIC) return -1;
	ph->cmd|=UC_CMD_RES_ERR;
	ph->code=UC_ERR_CON_SRV;
	ph->len=0;
	ret=UC_HDR_SIZE;
	return ret;
}


////////////////////////////////////////////////////////////////////////
#ifdef _USE_NFBI_WAY_ 
static int uc_nfbi_write(char *data, int len)
{
	int ret;
	ret = write(nfbi_fd, data, len);
	//printf( "uc_nfbi_write: write %d/%d\n", ret, len );
	if (ret!=len) 
	{
		if (errno == ETIME) 
			printf("uc_nfbi_write(): write timeout\n");
		else
			printf("uc_nfbi_write(): write error\n");
		return -1;
	}

	return ret;
}


static int uc_nfbi_read(char *data, int maxlen)
{
	int ret;
	
	ret = read(nfbi_fd, data, maxlen);
	//printf( "uc_nfbi_read: read %d/%d\n", ret, maxlen );
	if(ret<=0) 
	{
		if (errno == ETIME)
			printf("uc_nfbi_read(): read timeout\n");
		else
			printf("uc_nfbi_read(): read error\n");
		return -1;
	}

	return ret;
}

static int uc_nfbi_sendrecv(char *data, int len, int maxlen)
{
	int ret;
	ret=uc_nfbi_write( data, len );
	if(ret==len)
	{
		ret=uc_nfbi_read( data, maxlen );
	}
	
	return ret;
}
#endif /*_USE_NFBI_WAY_*/


////////////////////////////////////////////////////////////////////////
static int uc_relay(char *m, int len, int maxlen)
{
	int ret;

	if( (m==NULL) || (len<=0) ) return -1;

	if( (ucd_status==eUCD_DEAD)&&(t_failed+TRY_TIME>time(NULL)) )
	{
		ret=uc_relay_resp_fail( m, len );
	}else{
		UC_HDR *ph;
		int isNoResp=0;

		ph=(UC_HDR *)m;
		if( (len>UC_HDR_SIZE) &&
			(ph->magic==UC_HDR_MAGIC) &&
			((ph->cmd&UC_CMD_NO_RES_MASK)==UC_CMD_NO_RES) )
		{
			isNoResp=1;
#ifdef _USE_NFBI_WAY_
			printf( "uc_relay: not support sendonly\n" );
			ret=-1;
#else  /*_USE_NFBI_WAY_*/
			ret=uc_udp_real_sendonly( ser_ip, m, len);
#endif /*_USE_NFBI_WAY_*/

		}else{

#ifdef _USE_NFBI_WAY_
			ret=uc_nfbi_sendrecv( m, len, maxlen );
#else  /*_USE_NFBI_WAY_*/		
			ret=uc_udp_real_sendrecv( ser_ip, m, len, maxlen );
#endif /*_USE_NFBI_WAY_*/

		}
		
		if( ((isNoResp==0)&&(ret<=0)) ||
			((isNoResp==1)&&(ret<0))   )
		{	
			//failed
			if(ucd_status==eUCD_ALIVE)
			{
				ucd_status=eUCD_DEAD;
				printf( "uc_relay(): ucd_status=%d\n", ucd_status );
			}
			t_failed=time(NULL);
			ret=uc_relay_resp_fail( m, len );
		}else{
			//ok
			if(ucd_status==eUCD_DEAD)
			{
				ucd_status=eUCD_ALIVE;
				printf( "uc_relay(): ucd_status=%d\n", ucd_status );
			}
		}
	}
	
	return ret;	
}


static int uc_relay_start(void)
{
	int s;

	s=uc_udp_bind(NULL);
	if(s<0)
	{
		printf( "%s(): call uc_udp_bind() failed!\n", __FUNCTION__ );
		return -1;
	}
	uc_udp_loop( s, uc_relay, NULL );
	return 0;		
}


////////////////////////////////////////////////////////////////////////
#define UCRELAY_RUNFILE	"/var/run/ucrelay.pid"
static void uc_relay_log_pid(void)
{
	FILE *f;
	pid_t pid;
	char *pidfile = UCRELAY_RUNFILE;

	pid = getpid();
	f = fopen(pidfile, "w");
	if( f == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

static void uc_relay_clr_pid(void)
{
	FILE *f;
	char *pidfile = UCRELAY_RUNFILE;

	f = fopen(pidfile, "r");
	if( f != NULL){
		unlink(pidfile);
		fclose(f);
	}
}

static void uc_relay_handle_term(int s)
{
	uc_relay_clr_pid();
	exit(0);
}

static void uc_relay_handle_reset(int s)
{
	ucd_status=eUCD_ALIVE;
	printf( "%s(): reset ucd_status=%d\n", __FUNCTION__, ucd_status );
}


////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
#ifdef _USE_NFBI_WAY_
	nfbi_fd = open(NFBI_DEV_NAME, O_RDWR); 
	if(-1 == nfbi_fd)
	{
		perror("uc_relay: open nfbi driver failed");
		return -1;
	}
#else  /*_USE_NFBI_WAY_*/
	struct in_addr inaddr;
	
	if(argc!=2)
	{	
		printf( "ucrelay: error argument, specify ucd's ip address\n" );
		return -1;
	}

	if( inet_pton(AF_INET, argv[1], &inaddr)<=0 )
	{
		printf("ucrelay: inet_pton error, ip=%s\n", argv[1] );
		return -1;
	}
	strcpy( ser_ip, argv[1] );
#endif /*_USE_NFBI_WAY_*/

	if (daemon(0,1) == -1) 
	{
		perror("ucrelay: daemon()");
		return -1;
	}

	uc_relay_log_pid();
	signal( SIGTERM, uc_relay_handle_term );
	signal( SIGUSR1, uc_relay_handle_reset );

	return uc_relay_start();
}


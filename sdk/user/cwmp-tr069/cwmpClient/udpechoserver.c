#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <linux/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#include "udpechoserverlib.h"

struct ECHOSET
{
	char		*interface;
	struct in_addr	cliaddr;
	unsigned short	port;
	char		echoplus;
};

struct ECHOSET gEchoSet;
struct ECHORESULT *gEchoResult=NULL;


int handle_echo( struct ECHOSET *pset )
{
	int			sockfd;
	struct sockaddr_in	servaddr;
	int			ret;
	int			n = 1;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if( sockfd<0 )
	{
		printf("handle_echo> socket error ret=%d", sockfd);
		return -1;
	}

	if(pset->interface)
	{
		ret = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, pset->interface, strlen(pset->interface)+1);
		if(ret)
		{
			printf("handle_echo> setsockopt error interface=%s, ret=%d\n", pset->interface, ret);
			//return -1;
		}
	}

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n))<0)
		perror( "handle_echo> SO_REUSEADDR" );

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(pset->port);

	{
		int bindloop=10;
		while(bindloop--)
		{
			int bindsleep=3;
			ret = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
			if( ret<0 )
				perror( "handle_echo> bind" );
			else
				break;

			while(bindsleep!=0) bindsleep=sleep(bindsleep);
		}
		if(bindloop==0)
		{
			printf("handle_echo> bind error");
			close(sockfd);
			return -1;
		}
	}

	for(;;)
	{
		struct sockaddr_in	cliaddr;
		int			n;
		char			mesg[MAXLINE];
		socklen_t		fromlen;
		struct timeval		tv_recv, tv_reply;

		fromlen = sizeof(cliaddr);
		n = recvfrom(sockfd, mesg, MAXLINE, 0, (struct sockaddr *)&cliaddr, &fromlen );
		if(n<=0)
		{
			printf("handle_echo> recvfrom error ret=%d", n);
			continue;
		}
		//printf( "handle_echo> Get from %s:%d, len=%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), n );

		if(pset->cliaddr.s_addr)
		{
			if( pset->cliaddr.s_addr!=cliaddr.sin_addr.s_addr )
			{
				continue;
			}
		}

		gettimeofday( &tv_recv, NULL );
		if(gEchoResult->TimeFirstPacketReceived.tv_sec==0&&gEchoResult->TimeFirstPacketReceived.tv_usec==0)
			gEchoResult->TimeFirstPacketReceived=tv_recv;
		gEchoResult->TimeLastPacketReceived=tv_recv;
		gEchoResult->PacketsReceived++;
		gEchoResult->BytesReceived+=n;

		if(pset->echoplus)
		{
			struct ECHOPLUS *pplus;
			if( n>=sizeof(struct ECHOPLUS) )
			{
				pplus=(struct ECHOPLUS *)mesg;
				pplus->TestRespSN=htonl(gEchoResult->TestRespSN);
				pplus->TestRespRecvTimeStamp=htonl( tv_recv.tv_sec*1000000+tv_recv.tv_usec );
				pplus->TestRespReplyFailureCount=htonl( gEchoResult->TestRespReplyFailureCount );
				gettimeofday( &tv_reply, NULL );
				pplus->TestRespReplyTimeStamp=htonl( tv_reply.tv_sec*1000000+tv_reply.tv_usec );
#if 0
			}else{
				gEchoResult->TestRespReplyFailureCount++;
				continue;
#endif
			}
		}

		ret = sendto(sockfd, mesg, n, 0, (struct sockaddr *)&cliaddr, fromlen);
		if( ret!=n )
		{
			gEchoResult->TestRespReplyFailureCount++;
			printf("handle_echo> sendto error ret=%d\n", ret);
		}else{
			gEchoResult->TestRespSN++;
			gEchoResult->PacketsResponded++;
			gEchoResult->BytesResponded+=ret;
		}
	}

	return 0;
}

int parseArg( int argc, char **argv )
{
	char		*interface=NULL;
	struct in_addr	cliaddr;
	unsigned int	port;
	int		echoplus=0;
	int		cnt, got_inf, got_cliaddr, got_port, got_echoplus;

	cnt=1;
	got_inf=0;
	got_cliaddr=0;
	got_port=0;
	got_echoplus=0;
	while( cnt<argc )
	{
		if( strcmp( "-i", argv[cnt] )==0 )
		{
			if( cnt+1<argc )
			{
				cnt++;
				interface=argv[cnt];
				got_inf=1;
			}else
				return -1;
		}else if( strcmp( "-addr", argv[cnt] )==0 )
		{
			if( cnt+1<argc )
			{
				char *a=argv[cnt+1];
				if( inet_aton( a, &cliaddr ) )
				{
					got_cliaddr=1;
					cnt++;
				}else
					return -1;
			}else
				return -1;
		}else if( strcmp( "-port", argv[cnt] )==0 )
		{
			if( cnt+1<argc )
			{
				long int tmp_port;
				char *s=argv[cnt+1];
				tmp_port = strtol(s, (char **)NULL, 10);
				if( tmp_port==LONG_MIN || tmp_port==LONG_MAX )
					return -1;
				if( tmp_port<=0 || tmp_port>=65535 )
					return -1;
				port=tmp_port;
				got_port=1;
				cnt++;
			}else
				return -1;
		}else if( strcmp( "-plus", argv[cnt] )==0 )
		{
			got_echoplus=1;
			echoplus=1;
		}else{
			return -1;
		}

		cnt++;
	}

	if( !got_port ) return -1;
	memset( &gEchoSet, 0, sizeof(gEchoSet) );
	if(got_inf)
	{
		gEchoSet.interface=strdup( interface );
		printf( "got interface %s\n", gEchoSet.interface );
	}
	if(got_cliaddr)
	{
		memcpy( &gEchoSet.cliaddr, &cliaddr, sizeof(cliaddr) );
		printf( "got cliaddr %s\n", inet_ntoa(gEchoSet.cliaddr) );
	}
	if(got_port)
	{
		gEchoSet.port=port;
		printf( "got port %d\n", gEchoSet.port );
	}
	if(got_echoplus)
	{
		gEchoSet.echoplus=echoplus;
		printf( "got enable echo plus %d\n", gEchoSet.echoplus );
	}

	return 0;
}

#define ECHOSERVER_RUNFILE "/var/run/udpechoserver.pid"
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = ECHOSERVER_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}
static void clr_pid()
{
	FILE *f;
	char *pidfile = ECHOSERVER_RUNFILE;

	if((f = fopen(pidfile, "r")) != NULL){
		fclose(f);
		unlink(pidfile);
	}
}

void handle_term()
{
	clr_pid();
	exit(0);
}

int main(int argc, char **argv)
{
	log_pid();

	signal( SIGTERM,handle_term);
	signal( SIGHUP,SIG_IGN);
	signal( SIGINT,SIG_IGN);
	signal( SIGPIPE,SIG_IGN);
	signal( SIGALRM,SIG_IGN);
	signal( SIGUSR1,SIG_IGN);
	signal( SIGUSR2,SIG_IGN);

	if( parseArg( argc, argv ) < 0 )
	{
		printf( "\nudpechoserver [-i <interface>] [-addr <client address>] -port <port number> [-plus]\n" );
		return -1;
	}

	if( initShmem( (void**)&gEchoResult, sizeof(struct ECHORESULT), ECHOTOK )<0 )
	{
		printf( "initShmem() error\n" );
		return -1;
	}
	//printf( "gEchoResult=0x%x\n", gEchoResult );

	return handle_echo( &gEchoSet );
}

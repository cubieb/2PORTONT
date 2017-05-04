#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>
#include "uc_udp.h"

static unsigned int uc_udp_get_random(void)
{
        struct timeval tod;

        gettimeofday(&tod , NULL);
        srand(tod.tv_usec);
        return rand();
}

static unsigned short uc_udp_get_random16(void)
{
	return (unsigned short)(uc_udp_get_random()&0xffff);
}

static void uc_udp_dump_header(char *b, int len)
{
	if(b&&(len>=UC_HDR_SIZE))
	{
		UC_HDR *ph=(UC_HDR *)b;

		printf( "HDR ");
		printf( "MAGIC=0x%08x ", ph->magic );
		printf( "CMD=0x%04x ", ph->cmd);
		printf( "SEQ=0x%04x ", ph->seq);
		printf( "CODE=%u ", ph->code);
		printf( "LEN=%u ", ph->len);
		printf( "DATA=... ");
		printf( "\n");
	}
}

static int uc_udp_open(void)
{
	int sockfd;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		perror("uc_udp_open(): socket");
		return -1;
	}

	return sockfd;
}

static void uc_udp_close( int s)
{
	close(s);
}

static int uc_udp_select(int rfd, int wfd, int t)
{
	struct timeval timeout;
	fd_set rfdset, wfdset, efdset, *pr, *pw, *pe;
	int maxfd=-1, ret=-1;

	if( (rfd==-1)&&(wfd==-1) ) return -1;

	FD_ZERO(&rfdset);
	FD_ZERO(&wfdset);
	FD_ZERO(&efdset);
	pe=NULL;
	if(rfd!=-1)
	{
		FD_SET( rfd, &rfdset);
		pr=&rfdset;
		FD_SET( rfd, &efdset);
		pe=&efdset;
	}else
		pr=NULL;

	if(wfd!=-1)
	{
		FD_SET( wfd, &wfdset);
		pw=&wfdset;
		FD_SET( wfd, &efdset);
		pe=&efdset;
	}else
		pw=NULL;

	if(rfd>wfd) 
		maxfd=rfd+1;
	else
		maxfd=wfd+1;

	timeout.tv_sec = t;
	timeout.tv_usec = 0;


	for (;;)
	{
		int r;
		r = select( maxfd, pr, pw, pe, &timeout);
		if(r>0)
		{
			ret=r;
			break;
		}else if(r==0){ 
			ret=0;
			break;
		}else if(errno!=EINTR){
			perror("uc_udp_select");
			ret=-1;
			break;
		}
	}

	
	return ret;
}


/**************************************************************************************/
/* server */
/**************************************************************************************/
int uc_udp_handle_mesg(char *m, int len, int maxlen, uc_cmd_cb cmdcb)
{
	UC_HDR *ph;
	int retlen, ret;
	
	if((m==NULL)||(cmdcb==NULL)) return -1;
	if(len<UC_HDR_SIZE)
	{
		printf( "%s(): recv len(%d) is too short!\n", __FUNCTION__, len  );
		return -1;
	}

	//printf( "before handling:\n" );
	//uc_udp_dump_header(m, len);
	ph=(UC_HDR *)m;
	if(ph->magic!=UC_HDR_MAGIC)
	{
		printf( "%s(): check magic error!\n", __FUNCTION__);
		return -1;
	}
	if((ph->len+UC_HDR_SIZE)!=len)
	{
		printf( "%s(): check len error!\n", __FUNCTION__);
		return -1;
	}
	
	retlen=cmdcb( ph->cmd&UC_CMD_MASK, m+UC_HDR_SIZE, len-UC_HDR_SIZE, maxlen-UC_HDR_SIZE );
	if((ph->cmd&UC_CMD_NO_RES_MASK)==UC_CMD_NEED_RES)
	{
		if(retlen>=0)
		{
			ph->cmd|=UC_CMD_RES_OK;
			ph->code=0;
			ph->len=retlen;
			ret=retlen+UC_HDR_SIZE;
		}else{
			ph->cmd|=UC_CMD_RES_ERR;
			ph->code=(unsigned short)((~retlen+1)&0xffff);
			ph->len=0;
			ret=UC_HDR_SIZE;
		}
		//printf( "after handling:\n" );
		//uc_udp_dump_header(m, ret);
	}else
		ret=0; //skip to send response

	return ret;	
}

int uc_udp_bind(char *itf)
{
	int			sockfd;
	struct sockaddr_in	servaddr;
	int			ret, n;
	
	sockfd = uc_udp_open();
	if( sockfd<0 )
	{
		printf( "%s(): call uc_udp_open() failed!\n", __FUNCTION__ );
		return -1;
	}

	if(itf)
	{
		ret = setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, itf, strlen(itf)+1);
		if(ret)
		{
			printf("%s(): setsockopt error interface=%s, ret=%d, errno=%d\n", __FUNCTION__, itf, ret, errno);
		}
	}

	n=1;	
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n))<0)
		perror( "uc_udp_bind(): SO_REUSEADDR" );

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(UC_PORT);

{
	int bindloop=10;
	while(bindloop--)
	{
		int bindsleep=3;
		ret = bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if( ret<0 )
			perror( "uc_udp_bind(): bind" );
		else
			break;
			
		while(bindsleep!=0) bindsleep=sleep(bindsleep);
	}
	if(bindloop==0)
	{
		printf("uc_udp_bind(): bind error\n");
		uc_udp_close(sockfd);
		return -1;
	}
}

	return sockfd;
}

int uc_udp_loop(int sockfd, uc_udp_cb udpcb, uc_act_cb actcb)
{

	if(udpcb==NULL) return -1;

	for(;;)
	{
		struct sockaddr_in	cliaddr;
		socklen_t	fromlen;
		int			n, ret;
		char		mesg[UC_MAX_SIZE];
		struct timeval t, t_end;

		
		fromlen = sizeof(cliaddr);
		n = recvfrom(sockfd, mesg, UC_MAX_SIZE, 0, (struct sockaddr *)&cliaddr, &fromlen );
		if(n==0)
		{ 
			printf("uc_udp_loop(): recvfrom error ret=%d, errno=%d", n, errno);
			continue;
		}
		if(n<0)
		{ 
			if(errno!=EINTR)
				printf("uc_udp_loop(): recvfrom error ret=%d, errno=%d", n, errno);
			continue;
		}
		//printf( "uc_udp_loop(): got from %s:%d, len=%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), n );	

		n=udpcb( mesg, n, sizeof(mesg) );
		if(n<=0) continue;

		ret=uc_udp_select(-1, sockfd, UC_TIMEOUT);
		if( ret<=0 )
		{
			printf("%s(): call uc_udp_select() failed(%d)\n", __FUNCTION__, ret );
			continue;
		}
		gettimeofday(&t_end, NULL);
		t_end.tv_sec+=UC_TIMEOUT;
send_again:
		ret = sendto(sockfd, mesg, n, 0, (struct sockaddr *)&cliaddr, fromlen);
		if(ret<0)
		{
			if(errno==EINTR)
			{
				gettimeofday(&t, NULL);
				if( timercmp(&t, &t_end, <) ) goto send_again;
			}
			
			perror( "uc_udp_loop(): sendto" );
		}else if( ret!=n )
		{
			printf("uc_udp_loop(): only sendto %d bytes, not %d\n", ret, n);
		}

		if(actcb) actcb();
	}	
		
	return 0;
}


/**************************************************************************************/
/* client */
/**************************************************************************************/
static int uc_udp_send(int sockfd, char *sip, char *buff, int len)
{
	int n;
	struct timeval t, t_end;
	struct sockaddr_in servaddr;

	if( (buff==NULL) || (len<=0) ) return -1;

	if(!sip)
	{
		printf( "%s(): source ip==NULL\n", __FUNCTION__ );
		return -1;
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port   = htons(UC_PORT);
	if (inet_pton(AF_INET, sip, &servaddr.sin_addr) <= 0)
	{
		printf("%s(): inet_pton error, ip=%s\n", __FUNCTION__, sip );
		return -1;
	}

	n=uc_udp_select(-1, sockfd, UC_TIMEOUT);
	if( n<=0 )
	{
		//printf("%s(): call uc_select() failed(%d)\n", __FUNCTION__, n );
		return -1;
	}
		
	gettimeofday(&t_end, NULL);
	t_end.tv_sec+=UC_TIMEOUT;	
sendagain:
	n=sendto(sockfd, buff, len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if((n==-1)&&(errno==EINTR))
	{
		gettimeofday(&t, NULL);
		if( timercmp(&t, &t_end, <) ) goto sendagain;
	}
	if(n<0)
	{
		perror( "uc_udp_send(): sendto" );
		return -1;
	}else if( n!=len )
	{
		printf("%s(): only send n=%d, %d bytes\n", __FUNCTION__, n, len);
		return -1;
	}

	return n;
}

static int uc_udp_recv(int sockfd, char *buff, int len)
{
	int ret;
	struct timeval t, t_end;

	if( (buff==NULL) || (len<=0) ) return -1;

	ret=uc_udp_select(sockfd, -1, UC_TIMEOUT);
	if( ret<=0 )
	{
		//printf("%s(): call uc_select() failed(%d)\n", __FUNCTION__, ret );
		return -1;
	}

	gettimeofday(&t_end, NULL);
	t_end.tv_sec+=UC_TIMEOUT;	
recvagain:
	ret = recvfrom(sockfd, buff, len, 0, NULL, NULL);
	if((ret==-1)&&(errno==EINTR))
	{
		gettimeofday(&t, NULL);
		if( timercmp(&t, &t_end, <) ) goto recvagain;
	}
	if(ret<0)
	{
		perror( "uc_udp_recv(): recvfrom" );
		return -1;
	}

	return ret;
}

//-1: failed, "return >=0" means "recv bytes"
int uc_udp_real_sendrecv(char *sip, char *data, int len, int maxlen)
{
	int n, ret=-1;
	int sockfd;

	sockfd=uc_udp_open();
	if(sockfd<0) return ret;

	n=uc_udp_send(sockfd, sip, data, len);
	if(n==len)
	{
		n=uc_udp_recv(sockfd, data, maxlen);
		ret=n;
	}

	uc_udp_close(sockfd);	
	return ret;
}

//0:ok, -1: failed
int uc_udp_real_sendonly(char *sip, char *data, int len)
{
	int n, ret=-1;
	int sockfd;

	sockfd=uc_udp_open();
	if(sockfd<0) return ret;

	n=uc_udp_send(sockfd, sip, data, len);
	if(n==len) ret=0;

	uc_udp_close(sockfd);	
	return ret;
}

int uc_udp_sendrecv(char *sip, unsigned short cmd, char *buff, int len, int maxlen)
{
	UC_HDR hdr, *ph;
	int totalsize, n, ret=-1;

	if(!sip)
	{
		printf( "%s(): source ip==NULL\n", __FUNCTION__ );
		return -1;
	}
	if(buff==NULL)
	{
		printf( "%s(): buff==NULL\n", __FUNCTION__ );
		return -1;
	}
	if(len<0)
	{
		printf( "%s(): len=%d error\n", __FUNCTION__, len );
		return -1;
	}
	

	//hdr
	memset( &hdr, 0, UC_HDR_SIZE );
	hdr.magic=UC_HDR_MAGIC;
	hdr.cmd=cmd;
	hdr.seq=uc_udp_get_random16();
	hdr.len=len;
	memcpy( buff, &hdr, UC_HDR_SIZE );
	totalsize=UC_HDR_SIZE+len;
	//uc_udp_dump_header(buff,totalsize);


	if( (hdr.cmd&UC_CMD_NO_RES_MASK)==UC_CMD_NO_RES )
		return uc_udp_real_sendonly( sip, buff, totalsize );	
	n=uc_udp_real_sendrecv( sip, buff, totalsize, maxlen );
	if(n>0) //check hdr
	{
		//uc_udp_dump_header(buff,n);
		ph=(UC_HDR *)buff;
		if(n>=UC_HDR_SIZE)
		{
			if((ph->magic==hdr.magic) &&
				((ph->cmd&UC_CMD_MASK)==hdr.cmd) &&
				(ph->seq==hdr.seq) &&
				(ph->len==(n-UC_HDR_SIZE)) )
			{
				if( (ph->cmd&UC_CMD_RES_MASK)==UC_CMD_RES_OK )
				{
					n=n-UC_HDR_SIZE;
					if(n<=maxlen)
					{
						ret=n;
					}else{
						printf("%s(): buff size from upper layer is too small\n", __FUNCTION__);
					}
				}else{
					if( (cmd!=UC_CMD_DSL_IOC)&&(ph->code!=0xffff) )
						printf("%s(): cmd fail, code=%u(%d)\n", __FUNCTION__, ph->code, (short)(~ph->code+1) );
				}
			}else{
				printf("%s(): check header error!\n", __FUNCTION__);
				printf("%s(): ori=> magic=0x%08x cmd=%u, seq=%u\n", __FUNCTION__, hdr.magic, hdr.cmd, hdr.seq);
				printf("%s(): got=> magic=0x%08x cmd=%u, seq=%u\n", __FUNCTION__, ph->magic, ph->cmd, ph->seq);
				printf("%s(): got=> len=%u, recv len=%u\n", __FUNCTION__, ph->len, n-UC_HDR_SIZE );
			}
		}else{
			printf("%s(): recv len(%d) too short!\n", __FUNCTION__, n);
		}
	}
	return ret;
}
/**************************************************************************************/



#include <stdio.h>
#include <unistd.h>
#include "bfdlib.h"
#include "bfdsocket.h"
#include "bfdtimeout.h"

#define BFDECHO_MIN_TX_INT 100000 //0.1 sec
#define BFDECHO_DETECTMULT 5

/*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                       My Discriminator                        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                       Sequence Number                         |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct bfdechomsg
{
	unsigned int mydisc;
	unsigned int seq;
};

/*******************************************************************************/
static void bfd_echo_cal_new_txinterval(unsigned char detectmult, struct timeval *oldtv, struct timeval *newtv)
{
	unsigned long long int txint;
	unsigned int percent;

	if( !oldtv || !newtv ) return;

	//   The interval between transmitted BFD Echo packets MUST NOT be less than
	//   the value advertised by the remote system in Required Min Echo RX
	//   Interval, except as follows:
	//      A 25% jitter MAY be applied to the rate of transmission, such that
	//      the actual interval MAY be between 75% and 100% of the advertised
	//      value.  A single BFD Echo packet MAY be transmitted between
	//      normally scheduled Echo transmission intervals.
	BFD_TIMEVAL2LONGUS( oldtv, &txint );
	percent = rand() % 26; /*75%~100%*/
	txint = txint*(75+percent)/100;
	BFD_LONGUS2TIMEVAL( &txint, newtv );
	return;
}

static void bfd_echo_get_timevalue(struct bfdsessiondata *pb, struct timeval *tv)
{
	unsigned int percent;

	if(!pb || !tv) return;
	bfd_echo_cal_new_txinterval( pb->EchoDetectMult, &pb->EchoTxInterval, tv);

	if(BFDDBG(pb)) 
		printf( BFDDBG_FMT "echo tx interval={%d.%06d} => new {%d.%06d} \n", BFDDBG_ARG(pb), 
			pb->EchoTxInterval.tv_sec, pb->EchoTxInterval.tv_usec,
			tv->tv_sec, tv->tv_usec );
	return;
}

static void bfd_echo_timeoutcb(void *arg)
{
	struct bfdsessiondata	*pb=arg;
	struct bfdsession	*pbs=&pb->session;
	struct timeval tv;
	int sendmsg=1;

	if(pb->EchoEnable==0)
	{
		sendmsg=0;
	}
	if(pb->EchoSendRawSock<0)
	{
		sendmsg=0;
	}

	if(sendmsg)
	{
		unsigned char buf[32];
		int len;
		struct bfdechomsg *m;

		m=(struct bfdechomsg *)buf;
		m->mydisc=htonl(pbs->LocalDiscr);
		m->seq=htonl(pb->EchoTxSeq);
		pb->EchoTxSeq++;
		len=sizeof(struct bfdechomsg);

		if(BFDDBG(pb))
		{
			struct timeval pkttv;
			printf( "\n");
			gettimeofday(&pkttv, NULL);
			printf( BFDDBG_FMT "(ECHO OUT:time=%d.%06d)\n", BFDDBG_ARG(pb), pkttv.tv_sec, pkttv.tv_usec );
			printf( BFDDBG_FMT "send out an echo packet\n",  BFDDBG_ARG(pb) );
			bfd_dump_data( buf, len );
		}
		
		if(bfd_raw_sendto(	pb->EchoSendRawSock, 
					buf, len,  
					pb->EchoSourceIP, pb->EchoSourcePort, 
					pb->LocalIP, pb->EchoPort, 
					pb->EchoRemoteMacAddr, pb->EchoIfIndex,
					pb->dscp, pb->ttl)<0)
			printf( BFDDBG_FMT "call bfd_raw_sendto() failed\n", BFDDBG_ARG(pb) );
		//else
		//	printf( BFDDBG_FMT "call bfd_raw_sendto() ok\n", BFDDBG_ARG(pb) );

		if(pb->EchoPacketSent==0)
		{
			struct timeval tv;
			pb->EchoPacketSent=1;
			gettimeofday( &tv, NULL );
			timeradd( &tv, &pb->EchoDetectInterval, &pb->EchoDetectTimeout );
		}
	}
	
	bfd_echo_get_timevalue( pb, &tv );
	bfd_timeout(&bfd_echo_timeoutcb, pb, tv, &pb->EchoTxTime);
}

static void bfd_echo_handle_recv(struct bfdsessiondata *pb)
{
	fd_set fd;
	int maxfd;
	int r;
	struct timeval to;

	if(!pb->EchoEnable) return;
	if(pb->EchoSock<0) return;
	
        FD_ZERO(&fd);
        FD_SET(pb->EchoSock, &fd);
        maxfd=pb->EchoSock;
        to.tv_sec=0;
        to.tv_usec=0;
        //r = select( maxfd+1, &fd, NULL, &fd, &to);
        r = select( maxfd+1, &fd, NULL, NULL, &to);
        if(r<0)
        	perror( "bfd_echo_handle_recv() select error" );
        else if(r>0)
        {
        	//if(BFDDBG(pb)) printf( BFDDBG_FMT "select return r=%d\n", BFDDBG_ARG(pb), r );
        	if( FD_ISSET(pb->EchoSock, &fd) )
        	{
        		struct sockaddr_in cliaddr;
			char	buf[512];
			int	rlen;

			//if(BFDDBG(pb)) printf( BFDDBG_FMT "FD_ISSET(pb->EchoSock)\n", BFDDBG_ARG(pb) );
			if(BFDDBG(pb)) printf( "\n");
			rlen = bfd_recvfrom_with_srcinfo( pb->EchoSock, buf, sizeof(buf), &cliaddr );
			if(rlen>=0)
			{
				struct bfdechomsg *m=(struct bfdechomsg *)buf;
				int ishandled=0;
				
				if(BFDDBG(pb)) 
				{
					struct timeval pkttv;
					gettimeofday(&pkttv, NULL);
					printf( BFDDBG_FMT "(ECHO IN:time=%d.%06d)\n", BFDDBG_ARG(pb), pkttv.tv_sec, pkttv.tv_usec);
					printf( BFDDBG_FMT "call bfd_recvfrom_with_srcinfo() ok, len=%d\n", BFDDBG_ARG(pb), rlen );
				}

				//match local echo format, set ishandled
				//count the missing packets
				if( (rlen==sizeof(struct bfdechomsg)) &&
				    (m->mydisc==htonl(pb->session.LocalDiscr)) )
				{
					if(BFDDBG(pb))
					{
						printf( BFDDBG_FMT "get a loop-back echo packet!\n", BFDDBG_ARG(pb) );
						bfd_dump_data( buf, rlen );
					}
					ishandled=1;
					gettimeofday( &pb->EchoLastRecvTime, NULL );
					timeradd( &pb->EchoLastRecvTime, &pb->EchoDetectInterval, &pb->EchoDetectTimeout );
				}

				if(ishandled==0)
				{
					if(BFDDBG(pb))
					{
						printf( BFDDBG_FMT "skip the received echo packet!\n", BFDDBG_ARG(pb) );
						bfd_dump_data( buf, rlen );
					}
				}
			}else
				if(BFDDBG(pb)) printf( BFDDBG_FMT "call bfd_recvfrom_with_srcinfo() failed, rlen=%d\n", BFDDBG_ARG(pb), rlen );
        	}
        }
}

static int bfd_echo_update_parameters(struct bfdsessiondata *p)
{
	struct bfdsession	*pbs=&p->session;
	unsigned int txint;
	struct timeval tv;

	if(!p) return -1;

	txint=pbs->RemoteEchoRxInterval;
	if( (txint>0)&&(txint<BFDECHO_MIN_TX_INT) ) txint=BFDECHO_MIN_TX_INT;

	//handle the echo txinterval's value
	BFD_US2TIMEVAL( &txint, &tv );
	if( txint&&(p->EchoState==BFD_ON) )
	{
		if(timercmp(&tv,&p->EchoTxInterval, ==)==0)
		{
			struct timeval tdiff, tnew, tnewdiff;
			if(timercmp(&tv,&p->EchoTxInterval, >))
			{
				//delay the schedule tx
				timersub( &tv, &p->EchoTxInterval, &tdiff );
				//tdiff=tdiff*percentage
				bfd_echo_cal_new_txinterval( p->EchoDetectMult, &tdiff, &tnewdiff );
				timeradd( &p->EchoTxTime.c_time, &tnewdiff, &tnew);
				p->EchoTxTime.c_time=tnew;
				if(BFDDBG(p))
					printf( BFDDBG_FMT "delay the echo tx interval, tdiff={%d.%06d} => tnewdiff={%d.%06d}\n", BFDDBG_ARG(p),
							tdiff.tv_sec, tdiff.tv_usec, tnewdiff.tv_sec, tnewdiff.tv_usec );
			}else{ //<
				//hurry the schedule tx
				timersub( &p->EchoTxInterval, &tv, &tdiff );
				//tdiff=tdiff*percentage
				bfd_echo_cal_new_txinterval( p->EchoDetectMult, &tdiff, &tnewdiff );
				timersub( &p->EchoTxTime.c_time, &tnewdiff, &tnew);
				p->EchoTxTime.c_time=tnew;
				if(BFDDBG(p))
					printf( BFDDBG_FMT "hurry the echo tx interval, tdiff={%d.%06d} => tnewdiff={%d.%06d}\n", BFDDBG_ARG(p),
							tdiff.tv_sec, tdiff.tv_usec, tnewdiff.tv_sec, tnewdiff.tv_usec );
			}
		}//else txint does not change
	}
	p->EchoTxInterval=tv;

	//handle the echo detectiontime's value
	BFD_TIMEVALMULT( &p->EchoTxInterval, &p->EchoDetectMult, &tv );
	if( txint&&(p->EchoState==BFD_ON)&&p->EchoPacketSent )
	{
		if( timercmp( &tv, &p->EchoDetectInterval, == )==0 )
		{
			struct timeval tdiff, tnew;
			if( timercmp(&tv, &p->EchoDetectInterval, > ) )
			{
				timersub( &tv, &p->EchoDetectInterval, &tdiff );
				timeradd( &p->EchoDetectTimeout, &tdiff, &tnew );
				p->EchoDetectTimeout=tnew;
				if(BFDDBG(p))
					printf( BFDDBG_FMT "delay the echo detect interval, tdiff={%d.%06d}\n", BFDDBG_ARG(p),
						tdiff.tv_sec, tdiff.tv_usec);
			}else{ // <
			   #if 1
				//use the old echo detect interval (longer time) 
				//=> do not need to modify the p->EchoDetectTimeout
			   #else
				timersub( &p->EchoDetectInterval, &tnew, &tdiff );
				timersub( &p->EchoDetectTimeout, &tdiff, &tnew );
				p->EchoDetectTimeout=tnew;
				if(BFDDBG(p)) 			
					printf( BFDDBG_FMT "hurry the echo detect interval, tdiff={%d.%06d}\n", BFDDBG_ARG(p),
						tdiff.tv_sec, tdiff.tv_usec);
			   #endif
			}
		}//else detectiontime does not change
	}
	p->EchoDetectInterval=tv;


	//when SessionState==ST_UP, but RemoteSessionState may not in ST_UP
	//the echo packets may not be looped back => timeout
	//if( (pbs->SessionState==ST_UP)&&(txint!=0) )
	/*when both peers are in the ST_UP state, start to tx echo packets*/
	if(	(p->EchoAdminEnable!=0)&&
		(pbs->RemoteSessionState==ST_UP)&&
		(pbs->SessionState==ST_UP)&&
		(txint!=0) )
	{
		//enable tx echo packets to the remote system
		p->EchoEnable=1;
	}else{
		//disable tx echo packets to the remote system
		p->EchoEnable=0;
	}


	return 0;
}

static int bfd_echo_update(struct bfdsessiondata *p)
{
	if(!p) return -1;

	//update parameters
	bfd_echo_update_parameters(p);

	if( p->EchoEnable==0)
	{
		//close
		if(p->EchoSendRawSock>=0)
		{
			if(BFDDBG(p)) printf( BFDDBG_FMT "close EchoSendRawSock\n",  BFDDBG_ARG(p) );
			bfd_close(p->EchoSendRawSock);
			p->EchoSendRawSock=-1;
		}

		if(p->EchoSock>=0)
		{
			if(BFDDBG(p)) printf( BFDDBG_FMT "close EchoSock\n",  BFDDBG_ARG(p) );
			bfd_close(p->EchoSock);
			p->EchoSock=-1;
		}

		if( p->EchoState==BFD_ON )
		{
			if(BFDDBG(p)) printf( BFDDBG_FMT "disable local echo\n",  BFDDBG_ARG(p) );
			bfd_untimeout(&p->EchoTxTime);
			p->EchoState=BFD_OFF;
			p->EchoPacketSent=0;
		}
		
	}else{
		//open
		if(p->EchoSock<0)
		{
			int ifindex, ifflags;
			if( bfd_get_ifindex( p->Interface, &ifindex )<0 )
			{
				printf( BFDDBG_FMT "bfd_get_ifindex() error\n", BFDDBG_ARG(p) );
				return -1;
			}
			p->EchoIfIndex=ifindex;		
			if(BFDDBG(p)) printf( BFDDBG_FMT "%s's ifindex=%d\n", BFDDBG_ARG(p), p->Interface, p->EchoIfIndex);

			if( bfd_get_ifflags( p->Interface, &ifflags )<0 )
			{
				printf( BFDDBG_FMT "bfd_get_ifflags() error\n", BFDDBG_ARG(p) );
				return -1;
			}
			if(BFDDBG(p)) printf( BFDDBG_FMT "%s's ifflags=%d\n", BFDDBG_ARG(p), p->Interface, ifflags );

			if( ifflags&IFF_NOARP )
			{
				if(BFDDBG(p)) printf( BFDDBG_FMT "%s does not support ARP\n", BFDDBG_ARG(p), p->Interface );
				p->EchoIfHasARP=0;
			}else{
				p->EchoIfHasARP=1;
				if(BFDDBG(p)) printf( BFDDBG_FMT "%s supports ARP\n", BFDDBG_ARG(p), p->Interface );
				if( bfd_get_arp(p->Interface, p->EchoRemoteIP, p->EchoRemoteMacAddr)<0 )
				{
					printf( BFDDBG_FMT "bfd_get_arp() return error.\n ", BFDDBG_ARG(p) ); 
					return -1;
				}
				p->EchoRemoteMacAddrGot=1;
			}
			
			p->EchoSendRawSock=bfd_raw_bind( p->EchoIfIndex, p->ethprio );
			if( p->EchoSendRawSock<0 )
			{
				printf( BFDDBG_FMT "bind p->EchoSendRawSock error\n", BFDDBG_ARG(p) );
				return -1;
			}

			p->EchoSock = bfd_bind(p->EchoPort, p->Interface);
			if(p->EchoSock<0)
			{
				printf( BFDDBG_FMT "bind p->EchoSock error\n", BFDDBG_ARG(p) );
				if(p->EchoSendRawSock>=0)
				{	
					bfd_close(p->EchoSendRawSock);
					p->EchoSendRawSock=-1;
				}
				return -1;
			}
			//bfd_setsockopt( p->EchoSock, p->dscp, p->ttl, p->ethprio );
		}
		
		if(p->EchoState==BFD_OFF)
		{
			struct timeval tv={0,0};
			if(BFDDBG(p)) printf( BFDDBG_FMT "enable local echo\n",  BFDDBG_ARG(p) );
			bfd_timeout(&bfd_echo_timeoutcb, p, tv, &p->EchoTxTime);
			p->EchoState=BFD_ON;
			p->EchoPacketSent=0;
		}

	}
	return 0;
};

int bfd_echo_timeout( struct bfdsessiondata *pbsd )
{
	int istimeout=0;
	if(pbsd)
	{
		if(	(pbsd->EchoEnable!=0) && 
			(pbsd->EchoState==BFD_ON) &&
			(pbsd->EchoPacketSent!=0) )
		{
			struct timeval tv;

			gettimeofday( &tv, NULL );
			if( timercmp(&tv, &pbsd->EchoDetectTimeout, >) )
			{
				istimeout=1;
			}
		}
	}
	return istimeout;
}

void bfd_echo_process(struct bfdsessiondata *pb)
{
	bfd_echo_update(pb);
	bfd_echo_handle_recv( pb );
}

int bfd_echo_init(struct bfdsessiondata *p)
{
	if(!p) return -1;
	srand((unsigned int)time(NULL));

	p->EchoSock=-1;
	p->EchoPort=3785;
	p->EchoEnable=0;
	p->EchoState=BFD_OFF;
	
	p->EchoTxSeq = (unsigned int)(rand() & 0xffff);
	p->EchoDetectMult=BFDECHO_DETECTMULT;
	p->EchoPacketSent=0;
	p->EchoAdminEnable=1;

	p->EchoSendRawSock=-1;
	p->EchoIfIndex=0;
	p->EchoIfHasARP=0;
	memset( p->EchoRemoteIP, 0, sizeof(p->EchoRemoteIP) );
	memset( p->EchoRemoteMacAddr, 0, sizeof(p->EchoRemoteMacAddr) );
	p->EchoRemoteMacAddrGot=0;
	p->EchoSourcePort=43785;
	strcpy( p->EchoSourceIP, "192.168.101.50" );
	
	return 0;
}


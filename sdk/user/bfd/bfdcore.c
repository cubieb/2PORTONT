#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "bfdlib.h"
#include "bfdsocket.h"
#include "bfdtimeout.h"

/**************************************************************************************/
//poll
void bfd_pollseq_increase( struct bfdsessiondata *pb )
{
	if(!pb) return;

	if(pb->PollSeqSent)
	{
		if(pb->PollSeq<=1) pb->PollSeq++;
	}else{
		if(pb->PollSeq==0) pb->PollSeq++;
	}

	return;
}

void bfd_pollseq_get_finalbit( struct bfdsessiondata *pb )
{
	if(!pb) return;

	if(pb->PollSeqSent)
	{
		struct bfdsession *pbs=&pb->session;
		
		//the poll sequence must be terminated
		pb->PollSeqSent=0;
		if(pb->PollSeq) pb->PollSeq--;
		if(pb->PollSeq==0)
		{
			if(pbs->OldDesiredMinTxInterval) pbs->OldDesiredMinTxInterval=0;
			if(pbs->OldRequiredMinRxInterval) pbs->OldRequiredMinRxInterval=0;
			if(pbs->OldLocalEchoRxInterval) pbs->OldLocalEchoRxInterval=0;
		}
	}

	return;
}

#ifdef PERIODIC_POLL_WHEN_DEMAND
/**************************************************************************************/
//periodic poll callback
static void bfd_periodicpollcb(void *arg)
{
	struct bfdsessiondata *pb=arg;
	struct timeval pptv;

	bfd_pollseq_increase(pb);

	BFD_US2TIMEVAL( &pb->PeriodicPollInterval, &pptv );
	bfd_timeout(&bfd_periodicpollcb, pb, pptv, &pb->PeriodicPollTime);	

	if(BFDDBG(pb)) printf( BFDDBG_FMT "Increase pollseq=%u, PeriodicPollTime={%d.%06d}\n", BFDDBG_ARG(pb), 
				pb->PollSeq, pptv.tv_sec, pptv.tv_usec );
};
#endif //PERIODIC_POLL_WHEN_DEMAND

/**************************************************************************************/
//periodic tx
static void bfdtx_cal_new_txinterval(unsigned char detectmult, struct timeval *oldtv, struct timeval *newtv)
{
	unsigned long long int txint;
	unsigned int percent;

	if( !oldtv || !newtv ) return;

	//   The periodic transmission of BFD Control packets SHOULD be jittered
	//   by up to 25%, that is, the interval SHOULD be reduced by a random
	//   value of 0 to 25%, in order to avoid self-synchronization.  Thus, the
	//   average interval between packets may be up to 12.5% less than that
	//   negotiated.
	//   If bfd.DetectMult is equal to 1, the interval between transmitted BFD
	//   Control packets MUST be no more than 90% of the negotiated
	//   transmission interval, and MUST be no less than 75% of the negotiated
	//   transmission interval.  This is to ensure that, on the remote system,
	//   the calculated DetectTime does not pass prior to the receipt of the
	//   next BFD Control packet.
	BFD_TIMEVAL2LONGUS( oldtv, &txint );
	if( detectmult==1 )
		percent = rand() % 16; /*75%~90%*/	
	else
		percent = rand() % 26; /*75%~100%*/
	txint = txint*(75+percent)/100;
	BFD_LONGUS2TIMEVAL( &txint, newtv );
	return;
}

static void bfdtx_get_timevalue(struct bfdsessiondata *pb, struct timeval *tv)
{
	if(!pb || !tv) return;
	bfdtx_cal_new_txinterval( pb->session.DetectMult, &pb->PeriodicTxInterval, tv );

	if(BFDDBG(pb)) 
		printf( BFDDBG_FMT "tx interval={%d.%06d} => new {%d.%06d} \n", BFDDBG_ARG(pb), 
			pb->PeriodicTxInterval.tv_sec, pb->PeriodicTxInterval.tv_usec,
			tv->tv_sec, tv->tv_usec );
	return;
}

static void bfdtx_timeoutcb(void *arg)
{
	struct bfdsessiondata *pb=arg;
	struct bfdsession	*pbs=&pb->session;
	struct timeval tv;
	char	buf[128];
	int sendmsg=1;

	//if(BFDDBG(pb)) printf( BFDDBG_FMT "enter\n", BFDDBG_ARG(pb) );
	if( (pbs->RemoteDiscr==0)&&(pb->Role==BFD_PASSIVE_ROLE) )
	{
		sendmsg=0;
	}
	if( pbs->RemoteMinRxInterval==0 )
	{
		sendmsg=0;
	}
	if( (pbs->RemoteDemandMode) &&
	    (pbs->SessionState==ST_UP) &&
	    (pbs->RemoteSessionState==ST_UP) &&
	    (pb->PollSeq==0) )
	{
		sendmsg=0;
	}

	if(sendmsg)
	{
		int len;
		bfd_create_basic_msg( pb, buf, sizeof(buf) );
		if(pb->PollSeq)
		{
			 if(pb->PollSeqSent==0)
			 {
			 	pb->PollSeqSent=1;
			 	if(pb->CurDemandMode)
			 	{
				 	struct timeval tv;
				 	gettimeofday( &tv, NULL );
					timeradd( &tv, &pb->DetectInterval, &pb->DetectTimeout );
			 	}
			 }
			
			bfd_set_poll_bit( buf, 1 );
			bfd_set_final_bit( buf, 0 );		
		}
		bfd_msg_add_auth_part( pb, buf, sizeof(buf) );
		len=bfd_get_msg_len(buf);

		if(BFDDBG(pb)) 
		{
			struct timeval pkttv;
			printf( "\n");
			gettimeofday(&pkttv, NULL);
			printf( BFDDBG_FMT "(OUT:time=%d.%06d)\n", BFDDBG_ARG(pb), pkttv.tv_sec, pkttv.tv_usec );
			printf( BFDDBG_FMT "send out a BFD control packet.\n", BFDDBG_ARG(pb) );
			bfd_dump_data( buf, len );
			bfd_msg_dump( buf, len );
		}
		if( bfd_sendto( pb->LocalSendSock, buf, len, pb->RemoteIP, pb->RemotePort )<0 )
			printf( BFDDBG_FMT "call bfd_sendto() failed\n", BFDDBG_ARG(pb) );
		//else
		//	printf( BFDDBG_FMT "call bfd_sendto() ok\n", BFDDBG_ARG(pb) );
	}

	bfdtx_get_timevalue( pb, &tv );
	bfd_timeout(&bfdtx_timeoutcb, pb, tv, &pb->PeriodicTxTime);
}

void bfdtx_update_interval(struct bfdsessiondata *pbsd)
{
	unsigned int txint;
	struct timeval tv;
	unsigned char useOldValue=0;
	

	if(pbsd==NULL) return;

	//   If bfd.DesiredMinTxInterval is increased and bfd.SessionState is Up,
	//   the actual transmission interval used MUST NOT change until the Poll
	//   Sequence described above has terminated. 
	if(pbsd->session.OldDesiredMinTxInterval)
	{
		if(pbsd->session.SessionState==ST_UP)
			useOldValue=1;
		else
			pbsd->session.OldDesiredMinTxInterval=0;
	}
	if(useOldValue)
	{
		//if(BFDDBG(pbsd))
		//	printf( BFDDBG_FMT "use OldDesiredMinTxInterval(%u) to count tx interval!!!\n",
		//		BFDDBG_ARG(pbsd), pbsd->session.OldDesiredMinTxInterval  );
		txint=BFD_MAX( pbsd->session.OldDesiredMinTxInterval, pbsd->session.RemoteMinRxInterval );
	}else{
		txint=BFD_MAX( pbsd->session.DesiredMinTxInterval, pbsd->session.RemoteMinRxInterval );
	}
	BFD_US2TIMEVAL( &txint, &tv );

	//If the local system reduces its transmit interval due to
	//bfd.RemoteMinRxInterval being reduced (the remote system has
	//advertised a reduced value in Required Min RX Interval), and the
	//remote system is not in Demand mode, the local system MUST honor the
	//new interval immediately. 
#if 0
	if( (txint==pbsd->session.RemoteMinRxInterval) &&
		(timercmp(&tv,&pbsd->PeriodicTxInterval, <)) &&
		(pbsd->session.RemoteDemandMode==0) && 
		(pbsd->PeriodicTxState==BFD_ON) )
	{
		struct timeval tleft, tdiff, tnew;
		//if(BFDDBG(pbsd)) printf( BFDDBG_FMT "\n", BFDDBG_ARG(pbsd)  );

		if( bfd_timeleft( &pbsd->PeriodicTxTime, &tleft  )==0 )
		{
			timersub( &pbsd->PeriodicTxInterval,  &tv, &tdiff );
			if( timercmp(&tleft,&tdiff, <) )
			{
				//the new interval has already passed
				tnew.tv_sec=0;
				tnew.tv_usec=0;
			}else{
				timersub( &tleft, &tdiff, &tnew );
			}
			//bfd_untimeout(&pbsd->PeriodicTxTime);
			bfd_timeout(&bfdtx_timeoutcb, pbsd, tnew, &pbsd->PeriodicTxTime);
			if(BFDDBG(pbsd))
			{
				printf( BFDDBG_FMT "tx interval is reduced due to RemoteMinRxInterval => update immediately\n", BFDDBG_ARG(pbsd)  );
				printf( BFDDBG_FMT "tleft={%d.%06d}, tdiff={%d.%06d}, tnew={%d.%06d}\n", BFDDBG_ARG(pbsd),
						tleft.tv_sec, tleft.tv_usec, tdiff.tv_sec, tdiff.tv_usec, tnew.tv_sec, tnew.tv_usec);
			}
		}
	}else 
#endif
	/*the codes below should include the case above*/
	//timing parameter changes must be effected immediately
	if(timercmp(&tv,&pbsd->PeriodicTxInterval, ==)==0)
	{

		if(pbsd->PeriodicTxState==BFD_ON)
		{
			struct timeval tdiff, tnew, tnewdiff;
			if(timercmp(&tv,&pbsd->PeriodicTxInterval, >))
			{
				//delay the schedule tx
				timersub( &tv, &pbsd->PeriodicTxInterval, &tdiff );
				//tdiff=tdiff*percentage
				bfdtx_cal_new_txinterval( pbsd->session.DetectMult, &tdiff, &tnewdiff );
				timeradd( &pbsd->PeriodicTxTime.c_time, &tnewdiff, &tnew);
				pbsd->PeriodicTxTime.c_time=tnew;
				if(BFDDBG(pbsd))
					printf( BFDDBG_FMT "delay the tx interval, tdiff={%d.%06d} => tnewdiff={%d.%06d}\n", BFDDBG_ARG(pbsd),
							tdiff.tv_sec, tdiff.tv_usec, tnewdiff.tv_sec, tnewdiff.tv_usec );
			}else{ //<
				//hurry the schedule tx
				timersub( &pbsd->PeriodicTxInterval, &tv, &tdiff );
				//tdiff=tdiff*percentage
				bfdtx_cal_new_txinterval( pbsd->session.DetectMult, &tdiff, &tnewdiff );
				timersub( &pbsd->PeriodicTxTime.c_time, &tnewdiff, &tnew);
				pbsd->PeriodicTxTime.c_time=tnew;
				if(BFDDBG(pbsd))
					printf( BFDDBG_FMT "hurry the tx interval, tdiff={%d.%06d} => tnewdiff={%d.%06d}\n", BFDDBG_ARG(pbsd),
							tdiff.tv_sec, tdiff.tv_usec, tnewdiff.tv_sec, tnewdiff.tv_usec );
			}
		}
	}

	pbsd->PeriodicTxInterval=tv;
}

void bfdtx_update_enable(struct bfdsessiondata *pbsd)
{
	struct bfdsession *pbs;

	if(pbsd==NULL) return;
	
	pbs=&pbsd->session;
	if( (pbs->RemoteDemandMode==1) &&
	    (pbs->SessionState==ST_UP) &&
	    (pbs->RemoteSessionState==ST_UP) )
	{
		//Demand mode is active on the remote system and 
		//the local system MUST cease the periodic transmission of BFD Control packets
		if(pbsd->PeriodicTxEnable)
		{
			pbsd->PeriodicTxEnable=0;
			if(BFDDBG(pbsd)) printf( BFDDBG_FMT "stop the periodic transmission of BFD Control packets\n", BFDDBG_ARG(pbsd) );
		}
	}

	if( (pbs->RemoteDemandMode==0) ||
	    (pbs->SessionState!=ST_UP) ||
	    (pbs->RemoteSessionState!=ST_UP) )
	{
		//Demand mode is not active on the remote system and 
		//the local system MUST send periodic BFD Control packets
		if(pbsd->PeriodicTxEnable==0)
		{
			pbsd->PeriodicTxEnable=1;
			if(BFDDBG(pbsd)) printf( BFDDBG_FMT "start the periodic transmission of BFD Control packets\n", BFDDBG_ARG(pbsd) );
		}
	}
}

void bfdtx_onoff(struct bfdsessiondata *pb)
{
	if(pb==NULL) return;

	if(pb->PeriodicTxState==BFD_OFF)
	{
		if(pb->PeriodicTxEnable || pb->PollSeq)
		{
			struct timeval tv={0,0};
			bfd_timeout(&bfdtx_timeoutcb, pb, tv, &pb->PeriodicTxTime);
			pb->PeriodicTxState=BFD_ON;
		}
	}else{
		if( (pb->PeriodicTxEnable==0)&&(pb->PollSeq==0) )
		{
			bfd_untimeout(&pb->PeriodicTxTime);
			pb->PeriodicTxState=BFD_OFF;
		}			
	}
}

void bfdtx_update(struct bfdsessiondata *pb)
{
	bfdtx_update_interval(pb);
	bfdtx_update_enable(pb);
	bfdtx_onoff(pb);	
}

/**************************************************************************************/
//detection time 
void bfd_detect_update_interval(struct bfdsessiondata *pbsd)
{
	unsigned int txint;
	unsigned char useOldRxValue=0;

	if(pbsd==NULL) return;

	/*for asynchronous mode*/
	//   If bfd.RequiredMinRxInterval is reduced and bfd.SessionState is Up,
	//   the previous value of bfd.RequiredMinRxInterval MUST be used when
	//   calculating the Detection Time for the remote system until the Poll
	//   Sequence described above has terminated.
	if(pbsd->session.OldRequiredMinRxInterval)
	{
		if(pbsd->session.SessionState==ST_UP)
			useOldRxValue=1;
		else
			pbsd->session.OldRequiredMinRxInterval=0;
	}
	if(useOldRxValue)
	{
		//if(BFDDBG(pbsd))
		//	printf( "%s %s> use OldRequiredMinRxInterval(%u) to count async detection time!!!\n", 
		//		BFD_GET_INF_NAME(pbsd), __FUNCTION__, pbsd->session.OldRequiredMinRxInterval );
		txint=BFD_MAX( pbsd->session.OldRequiredMinRxInterval, pbsd->session.RemoteMinTxInterval );
	}else{
		txint=BFD_MAX( pbsd->session.RequiredMinRxInterval, pbsd->session.RemoteMinTxInterval );
	}
	BFD_US2TIMEVAL( &txint, &pbsd->DetectIntervalAsynBase );


	/*for demand mode*/
	txint=BFD_MAX( pbsd->session.DesiredMinTxInterval, pbsd->session.RemoteMinRxInterval );
	BFD_US2TIMEVAL( &txint, &pbsd->DetectIntervalDemdBase );
}

void bfd_active_demand_mode(struct bfdsessiondata *pbsd)
{
	struct bfdsession *pbs;
	unsigned char old_CurDemandMode;
	struct timeval tnew, tdiff, t;

	if(pbsd==NULL) return;
	bfd_detect_update_interval(pbsd);
	
	pbs=&pbsd->session;
	old_CurDemandMode=pbsd->CurDemandMode;
	if( (pbs->DemandMode==1) &&
	    (pbs->SessionState==ST_UP) &&
	    (pbs->RemoteSessionState==ST_UP) )
	{
		if(!pbsd->CurDemandMode)
		{
			pbsd->CurDemandMode=1;
			if(BFDDBG(pbsd)) printf( BFDDBG_FMT "demand mode is active\n", BFDDBG_ARG(pbsd) );
		}
	}else{
		if(pbsd->CurDemandMode)
		{
			//when demandmode changes from active to un-active
			//the DetectTimeout may have passed
			struct timeval tv;
			gettimeofday( &tv, NULL );
			timeradd( &tv, &pbsd->DetectInterval, &pbsd->DetectTimeout );

			pbsd->CurDemandMode=0;
			if(BFDDBG(pbsd)) printf( BFDDBG_FMT "demand mode is NOT active\n", BFDDBG_ARG(pbsd) );
		}
	}

	if(old_CurDemandMode!=pbsd->CurDemandMode)
	{
		bfd_pollseq_increase(pbsd);//pbsd->PollSeq++;
	}

#ifdef PERIODIC_POLL_WHEN_DEMAND
	if(pbsd->CurDemandMode)
	{
		if(	(pbsd->PeriodicPollAdminEnable) &&
			(pbsd->PeriodicPollState==BFD_OFF) &&
			(pbsd->PeriodicPollInterval>0) )
		{
			struct timeval pptv;
			BFD_US2TIMEVAL( &pbsd->PeriodicPollInterval, &pptv );
			bfd_timeout(&bfd_periodicpollcb, pbsd, pptv, &pbsd->PeriodicPollTime);
			pbsd->PeriodicPollState=BFD_ON;
			if(BFDDBG(pbsd)) printf( BFDDBG_FMT "Enable PeriodicPoll, PeriodicPollTime={%d.%06d}\n", BFDDBG_ARG(pbsd),
						pptv.tv_sec, pptv.tv_usec );
		}
	}else{
		if(pbsd->PeriodicPollState==BFD_ON)
		{
			bfd_untimeout(&pbsd->PeriodicPollTime);
			pbsd->PeriodicPollState=BFD_OFF;
			if(BFDDBG(pbsd)) printf( BFDDBG_FMT "Disable PeriodicPoll\n", BFDDBG_ARG(pbsd) );
		}
	}
#endif //PERIODIC_POLL_WHEN_DEMAND


	//In Asynchronous mode, the Detection Time calculated in the local
	//system is equal to the value of Detect Mult received from the remote
	//system, multiplied by the agreed transmit interval of the remote
	//system (the greater of bfd.RequiredMinRxInterval and the last
	//received Desired Min TX Interval.)
	
	//In Demand mode, the Detection Time calculated in the local system is
	//equal to bfd.DetectMult, multiplied by the agreed transmit interval
	//of the local system (the greater of bfd.DesiredMinTxInterval and
	//bfd.RemoteMinRxInterval.)
	if(pbsd->CurDemandMode)
	{
		BFD_TIMEVALMULT( &pbsd->DetectIntervalDemdBase, &pbsd->session.DetectMult, &tnew );
	}else{
		BFD_TIMEVALMULT( &pbsd->DetectIntervalAsynBase, &pbsd->session.RemoteDetectMult, &tnew );
	}

	//timing parameter changes must be effected immediately
	if( timercmp( &tnew, &pbsd->DetectInterval, == )==0 )
	{
		if( timercmp(&tnew, &pbsd->DetectInterval, > ) )
		{
			timersub( &tnew, &pbsd->DetectInterval, &tdiff );
			timeradd( &pbsd->DetectTimeout, &tdiff, &t );
			pbsd->DetectTimeout=t;
			if(BFDDBG(pbsd))
				printf( BFDDBG_FMT "delay the detect interval, tdiff={%d.%06d}\n", BFDDBG_ARG(pbsd),
					tdiff.tv_sec, tdiff.tv_usec);
		}else{ // <
			timersub( &pbsd->DetectInterval, &tnew, &tdiff );
			timersub( &pbsd->DetectTimeout, &tdiff, &t );
			pbsd->DetectTimeout=t;
			if(BFDDBG(pbsd)) 			
				printf( BFDDBG_FMT "hurry the detect interval, tdiff={%d.%06d}\n", BFDDBG_ARG(pbsd),
					tdiff.tv_sec, tdiff.tv_usec);
		}
	}

	pbsd->DetectInterval=tnew;
	return;
}

static int bfd_detect_timeout( struct bfdsessiondata *pbsd )
{
	int istimeout=0;
	if(pbsd)
	{
		struct timeval tv;

		gettimeofday( &tv, NULL );
		if(pbsd->CurDemandMode==0) //In Asynchronous mode
		{
			if( timercmp(&tv, &pbsd->DetectTimeout, >) )
			{
				istimeout=1;
			}
		}else //In Demand mode
		{
			if( pbsd->PollSeqSent && timercmp(&tv, &pbsd->DetectTimeout, >) )
			{
				istimeout=1;
			}			
		}
	}
	return istimeout;
}

/**************************************************************************************/
void bfd_handle_statemachine( struct bfdsessiondata *pbsd, int event)
{
	struct bfdsession *pbs;
	
	if(!pbsd) return;

	pbs=&pbsd->session;
	if(BFDDBG(pbsd)) 
		printf( BFDDBG_FMT "<<<state=%d(%s), event=%d(%s) ==> ", BFDDBG_ARG(pbsd), 
			pbs->SessionState, bfd_getstate_str(pbs->SessionState), 
			event,  bfd_getevent_str(event) );
	switch(pbs->SessionState)
	{
	case ST_ADMINDOWN:
		switch(event)
		{
		case EVENT_ENABLE_SESSION:
			pbs->SessionState=ST_DOWN;
			break;
		}
		break;
	case ST_DOWN:
		switch(event)
		{
		case EVENT_DOWN:
			pbs->SessionState=ST_INIT;
			break;
		case EVENT_INIT:
			pbs->SessionState=ST_UP;
			break;
		case EVENT_DISABLE_SESSION:
			pbs->SessionState=ST_ADMINDOWN;
			pbs->LocalDiag=DC_ADMIN_DOWN;
			break;
		}
		break;
	case ST_INIT:
		switch(event)
		{
		case EVENT_INIT:
		case EVENT_UP:
			pbs->SessionState=ST_UP;
			break;
		case EVENT_ADMINDOWN:
			pbs->LocalDiag=DC_NSSD;
			pbs->SessionState=ST_DOWN;
			break;
		case EVENT_DETECT_TIMEOUT:
			pbs->LocalDiag=DC_TIME_EXPIRED;
			pbs->SessionState=ST_DOWN;
			break;
		case EVENT_ECHO_TIMEOUT:
			pbs->LocalDiag=DC_ECHO_FAILED;
			pbs->SessionState=ST_DOWN;
			break;
		case EVENT_DISABLE_SESSION:
			pbs->SessionState=ST_ADMINDOWN;
			pbs->LocalDiag=DC_ADMIN_DOWN;
			break;
		}
		break;
	case ST_UP:
		switch(event)
		{
		case EVENT_DOWN:
			pbs->LocalDiag=DC_NSSD;
			pbs->SessionState=ST_DOWN;
			break;
		case EVENT_ADMINDOWN:
			pbs->LocalDiag=DC_NSSD;
			pbs->SessionState=ST_DOWN;
			break;
		case EVENT_DETECT_TIMEOUT:
			pbs->LocalDiag=DC_TIME_EXPIRED;
			pbs->SessionState=ST_DOWN;
			break;
		case EVENT_ECHO_TIMEOUT:
			pbs->LocalDiag=DC_ECHO_FAILED;
			pbs->SessionState=ST_DOWN;
			break;
		case EVENT_DISABLE_SESSION:
			pbs->SessionState=ST_ADMINDOWN;
			pbs->LocalDiag=DC_ADMIN_DOWN;
			break;
		}
		break;
	default:
		break;
	}
	if(BFDDBG(pbsd)) 
		printf( "new state=%d(%s), diag=%d(%s)>>>\n", 
			pbs->SessionState,bfd_getstate_str(pbs->SessionState), 
			pbs->LocalDiag, bfd_getdiagcode_str(pbs->LocalDiag)  );

}
/**************************************************************************************/
static void bfd_handle_timeout( struct bfdsessiondata *pbsd)
{
	if(pbsd)
	{
		int istimeout=0;
		int eventcode;

		if( bfd_detect_timeout(pbsd) )
		{
			istimeout=1;
			eventcode=EVENT_DETECT_TIMEOUT;
		}else if( bfd_echo_timeout(pbsd) )
		{
			istimeout=1;
			eventcode=EVENT_ECHO_TIMEOUT;
		}

		if(istimeout)
		{			
			struct timeval tv;

			//timeout
			bfd_handle_statemachine( pbsd, eventcode);

			//clear the bfd.RemoteDiscr
			pbsd->session.RemoteDiscr=0;
			//reset bfd.RemoteMinRxInterval to a small value
			pbsd->session.RemoteMinRxInterval=1;
			pbsd->session.RemoteMinTxInterval=1;
			//clear the bfd.AuthSeqKnown (fixme, should twice the detection time)
			pbsd->session.AuthSeqKnown=0;
			//reset poll-related
			if(pbsd->session.OldDesiredMinTxInterval) pbsd->session.OldDesiredMinTxInterval=0;
			if(pbsd->session.OldRequiredMinRxInterval) pbsd->session.OldRequiredMinRxInterval=0;
			if(pbsd->session.OldLocalEchoRxInterval) pbsd->session.OldLocalEchoRxInterval=0;
			pbsd->PollSeq=0;
			pbsd->PollSeqSent=0;
	
			//set new timeout for detect time
			gettimeofday( &tv, NULL );
			timeradd( &tv, &pbsd->DetectInterval, &pbsd->DetectTimeout );
		}
	}
}

/**************************************************************************************/
static void bfd_handle_recv(struct bfdsessiondata *pb)
{
	fd_set fd;
	int maxfd;
	int r;
	struct timeval to;

        FD_ZERO(&fd);
        FD_SET(pb->LocalRecvSock, &fd);
        maxfd=pb->LocalRecvSock;
        to.tv_sec=0;
        to.tv_usec=1000;
        //r = select( maxfd+1, &fd, NULL, &fd, &to);
        r = select( maxfd+1, &fd, NULL, NULL, &to);
        if(r<0)
        	perror( "bfd_handle_recv> select error" );
        else if(r>0)
        {
        	//if(BFDDBG(pb)) printf( "select return r=%d\n", r );
        	if( FD_ISSET(pb->LocalRecvSock, &fd) )
        	{
			struct sockaddr_in cliaddr;
			char	buf[512];
			int	rlen;
			unsigned int dscp, ttl, ethprio;
			
			if(BFDDBG(pb))
			{
				printf( "\n");
				//printf( BFDDBG_FMT "FD_ISSET(pb->LocalRecvSock)\n", BFDDBG_ARG(pb) );
			}

			dscp=0;
			ttl=0;
			ethprio=0;
			rlen = bfd_recvmsg( pb->LocalRecvSock, buf, sizeof(buf), &cliaddr, &dscp, &ttl, &ethprio );
			//if(BFDDBG(pb)) printf( BFDDBG_FMT "got a packet with dscp=%u, ttl=%u\n", 
			//		BFDDBG_ARG(pb), dscp, ttl);
#ifdef _CHECK_TTL_
			if(ttl!=255)
			{
				//auth_none => must, other authtype => may discard the received packet
				//if(pb->session.AuthType==AUTH_NONE)
				{
					if(BFDDBG(pb)) printf( BFDDBG_FMT "got a packet with ttl=%u, not 255 => discard it !!!\n", 
							BFDDBG_ARG(pb), ttl);					
					return;
				}
			}
#endif //_CHECK_TTL_

			if(rlen>=0)
			{
				int ret;
				if( strcmp(pb->RemoteIP, inet_ntoa(cliaddr.sin_addr)) )
				{
					if(BFDDBG(pb)) printf( BFDDBG_FMT "got a packet from %s, not from %s => discard it !!!\n", 
							BFDDBG_ARG(pb), inet_ntoa(cliaddr.sin_addr), pb->RemoteIP);
					return;
				}
					
				if(BFDDBG(pb))
				{ 
					struct timeval pkttv;
					gettimeofday(&pkttv, NULL);
					printf( BFDDBG_FMT "(IN:time=%d.%06d)\n", BFDDBG_ARG(pb), pkttv.tv_sec, pkttv.tv_usec);
					printf( BFDDBG_FMT "call bfd_recvfrom() ok, len=%d.\n", 	BFDDBG_ARG(pb), rlen );
					bfd_dump_data( buf, rlen );
					bfd_msg_dump( buf, rlen );
				}
				ret=bfd_process_recv_msg( pb, buf, rlen );
				if( ret<0 )
				{
					if(BFDDBG(pb)) printf( BFDDBG_FMT "discard the received packet (ret=%d) !!!\n", BFDDBG_ARG(pb), ret );
					//if(BFDDBG(pb)) bfd_sessiondata_dump(pb);
				}else{
					//if(BFDDBG(pb)) bfd_sessiondata_dump(pb);
					if(pb->RecvPollSeq)
					{
						int len;
						if(BFDDBG(pb)) 
						{
							printf( "\n");
							printf( BFDDBG_FMT "receive P bit=> send back with F bit\n", BFDDBG_ARG(pb) );
						}
						bfd_create_basic_msg( pb, buf, sizeof(buf) );
						bfd_set_poll_bit( buf, 0 );
						bfd_set_final_bit( buf, 1 );
						bfd_msg_add_auth_part( pb, buf, sizeof(buf) );
						len=bfd_get_msg_len(buf);

						if(BFDDBG(pb)) 
						{
							struct timeval pkttv;
							gettimeofday(&pkttv, NULL);
							printf( BFDDBG_FMT "(OUT:time=%d.%06d)\n", BFDDBG_ARG(pb), pkttv.tv_sec, pkttv.tv_usec );
							printf( BFDDBG_FMT "send out a BFD control packet with F bit.(dscp=%d, ethprio=%d)\n", 
										BFDDBG_ARG(pb), dscp, ethprio);
							bfd_dump_data( buf, len );
							bfd_msg_dump( buf, len );
						}
						
						bfd_setsockopt( pb->LocalSendFinalSock, dscp, pb->ttl, ethprio );
						if( bfd_sendto( pb->LocalSendFinalSock, buf, len, pb->RemoteIP, pb->RemotePort )<0 )
							printf( BFDDBG_FMT "call bfd_sendto() failed\n", BFDDBG_ARG(pb) );
						//else
						//	printf( BFDDBG_FMT "call bfd_sendto ok\n", BFDDBG_ARG(pb) );
						
						pb->RecvPollSeq=0;
					}
				}
			}else{
				if(BFDDBG(pb)) printf( BFDDBG_FMT "call bfd_recvfrom() failed, rlen=%d\n", BFDDBG_ARG(pb), rlen );
			}
        	}
        }
        
        return;
}

/**************************************************************************************/
void bfd_checktxinterval(struct bfdsessiondata *pb)
{
	if(pb)
	{
		//   When bfd.SessionState is not Up, the system MUST set
		//   bfd.DesiredMinTxInterval to a value of not less than one second
		if(pb->session.UserDesiredMinTxInterval<(BFD_ONESECOND)) 
		{
			if(	(pb->session.SessionState==ST_UP) &&
				(pb->session.DesiredMinTxInterval!=pb->session.UserDesiredMinTxInterval) )
			{
				//decrease DesiredMinTxInterval case, and sessionstate is up
				pb->session.DesiredMinTxInterval=pb->session.UserDesiredMinTxInterval;
				bfd_pollseq_increase(pb);//pb->PollSeq++;
				if(BFDDBG(pb)) printf( BFDDBG_FMT "set DesiredMinTxInterval to UserDesiredMinTxInterval(%u)\n", 
									BFDDBG_ARG(pb), pb->session.DesiredMinTxInterval );
			}else if (	(pb->session.SessionState!=ST_UP) &&
						(pb->session.DesiredMinTxInterval!=(BFD_ONESECOND)) )
			{
				//increase DesiredMinTxInterval case, but sessionstate is not up
				pb->session.DesiredMinTxInterval=BFD_ONESECOND;//one second			
				if(BFDDBG(pb)) printf( BFDDBG_FMT "set DesiredMinTxInterval to one second(%u)\n", 
									BFDDBG_ARG(pb), pb->session.DesiredMinTxInterval );
			}
		}
	}
}

void bfd_detect_wt146(struct bfdsessiondata *pb, unsigned char preSessionState)
{
	if(pb)
	{
		//wt-146 case 1:Prompt DHCP client to transition into Init-Reboot state for DHCP initiated IP subscribers, 
		//or send a user-defined signal to the operator console for static IP subscribers.
		//wt-146 case 2:The RG MUST enter DHCP Init-Reboot state 
		//upon detecting that BFD transitioned into "Down" state
		if( (pb->GotGSNotify) ||
			((pb->session.SessionState==ST_DOWN)&&(preSessionState!=pb->session.SessionState)) )
		{
			if(pb->dhcpc_pid)
			{
				if(BFDDBG(pb)) printf( BFDDBG_FMT "send SIGHUP signal to dhcpc pid=%d(GS=%d)\n", 
									BFDDBG_ARG(pb), pb->dhcpc_pid, pb->GotGSNotify );
				kill( pb->dhcpc_pid, SIGHUP );
			}
			if(pb->GotGSNotify) pb->GotGSNotify=0;
		}
	}
}

void bfd_process(struct bfdsessiondata *pb)
{
	if(pb)
	{
		unsigned char oriSessionState;
		oriSessionState=pb->session.SessionState;

		bfd_checktxinterval(pb);
		bfd_handle_recv(pb);
		bfd_active_demand_mode(pb);
		bfdtx_update(pb);
		bfd_echo_process(pb);
		bfd_handle_timeout(pb);
		bfd_detect_wt146(pb, oriSessionState);
	}
}

void bfd_loop(struct bfdsessiondata *pb)
{
	struct timeval tv;

	bfd_sessiondata_dump(pb);
	
	//bfd_detect_update_interval(pb);
	bfd_active_demand_mode(pb);
	gettimeofday( &tv, NULL );
	timeradd( &tv, &pb->DetectInterval, &pb->DetectTimeout );
	
	while(1)
	{
#ifdef _HANDLE_CONSOLE_		
		bfd_handle_console(pb);
#endif /*_HANDLE_CONSOLE_*/
		bfd_process(pb);
		bfd_calltimeout();
	}
}

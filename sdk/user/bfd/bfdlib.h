#ifndef _BFDLIB_H_
#define _BFDLIB_H_

#include <asm/byteorder.h>
#include <sys/time.h>
#include "bfdtimeout.h"

#define BFD_ONESECOND	(1000000) /*us, microsecond*/

#define PERIODIC_POLL_WHEN_DEMAND 1
#define PERIODIC_POLL_DEFAULT_INTERVAL (1*60*BFD_ONESECOND) /*1 min*/
	
/*The version number:*/
#define BFD_VER		1

/*BFD diagnostic code:
        0 -- No Diagnostic
        1 -- Control Detection Time Expired
        2 -- Echo Function Failed
        3 -- Neighbor Signaled Session Down
        4 -- Forwarding Plane Reset
        5 -- Path Down
        6 -- Concatenated Path Down
        7 -- Administratively Down
        8 -- Reverse Concatenated Path Down
        9-31 -- Reserved for future use
*/
#define DC_NO_DIAG		0
#define	DC_TIME_EXPIRED		1
#define	DC_ECHO_FAILED		2
#define	DC_NSSD			3
#define DC_FORWARD_RESET	4
#define DC_PATH_DOWN		5
#define DC_CONPATH_DOWN		6
#define DC_ADMIN_DOWN		7
#define DC_RCONPATH_DOWN	8

/*BFD session state:
        0 -- AdminDown
        1 -- Down
        2 -- Init
        3 -- Up
*/
#define ST_ADMINDOWN	0
#define ST_DOWN		1
#define ST_INIT		2
#define ST_UP		3

/* BFD event */
#define EVENT_ADMINDOWN		0
#define EVENT_DOWN		1
#define EVENT_INIT		2
#define EVENT_UP		3
#define EVENT_DETECT_TIMEOUT	4
#define EVENT_ECHO_TIMEOUT	5
#define EVENT_ENABLE_SESSION	6
#define EVENT_DISABLE_SESSION	7
#define EVENT_UNKNOWN		8



/*The authentication type:
        0 - Reserved
        1 - Simple Password
        2 - Keyed MD5
        3 - Meticulous Keyed MD5
        4 - Keyed SHA1
        5 - Meticulous Keyed SHA1
        6-255 - Reserved for future use
*/
#define AUTH_NONE		0
#define AUTH_PASSWORD		1
#define AUTH_KEYED_MD5		2
#define AUTH_METI_KEYED_MD5	3
#define AUTH_KEYED_SHA1		4
#define AUTH_METI_KEYED_SHA1	5

/*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Auth Type   |   Auth Len    |  Auth Key ID  |  Password...  |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                              ...                              |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct bfd_auth_password
{
	unsigned char	type;
	unsigned char	len;
	unsigned char	keyid;
	unsigned char	password[0];
};

/*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Auth Type   |   Auth Len    |  Auth Key ID  |   Reserved    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        Sequence Number                        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                      Auth Key/Digest...                       |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                              ...                              |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct bfd_auth_md5
{
	unsigned char	type;
	unsigned char	len;
	unsigned char	keyid;
	unsigned char	res;
	unsigned int	seq;
	unsigned char	digest[0];
};

/*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Auth Type   |   Auth Len    |  Auth Key ID  |   Reserved    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                        Sequence Number                        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                       Auth Key/Hash...                        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                              ...                              |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct bfd_auth_sha1
{
	unsigned char	type;
	unsigned char	len;
	unsigned char	keyid;
	unsigned char	res;
	unsigned int	seq;
	unsigned char	hash[0];
};

/*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |   Auth Type   |   Auth Len    |    Authentication Data...     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct bfd_auth
{
	unsigned char	type;
	unsigned char	len;
	unsigned char	data[0];
};

/*
     0                   1                   2                   3
     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |Vers |  Diag   |Sta|P|F|C|A|D|M|  Detect Mult  |    Length     |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                       My Discriminator                        |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                      Your Discriminator                       |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                    Desired Min TX Interval                    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                   Required Min RX Interval                    |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                 Required Min Echo RX Interval                 |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/
struct bfdmsg
{
#ifdef __BIG_ENDIAN_BITFIELD
	unsigned char	vers:3;
	unsigned char	diag:5;
#else
	unsigned char	diag:5;
	unsigned char	vers:3;
#endif /*__BIG_ENDIAN_BITFIELD*/

#ifdef __BIG_ENDIAN_BITFIELD
	unsigned char	sta:2;
	unsigned char	flag_p:1;
	unsigned char	flag_f:1;
	unsigned char	flag_c:1;
	unsigned char	flag_a:1;
	unsigned char	flag_d:1;
	unsigned char	flag_m:1;
#else
	unsigned char	flag_m:1;
	unsigned char	flag_d:1;
	unsigned char	flag_a:1;
	unsigned char	flag_c:1;
	unsigned char	flag_f:1;
	unsigned char	flag_p:1;
	unsigned char	sta:2;
#endif /*__BIG_ENDIAN_BITFIELD*/

	unsigned char	detect_mult;
	unsigned char	length;
	unsigned int	mydisc;
	unsigned int	yourdisc;
	unsigned int	txinterval;
	unsigned int	rxinterval;
	unsigned int	echorxinterval;
	unsigned char	authdata[0];
};


/*bfd operation mode*/
#define BFD_ASYNC_MODE		0
#define BFD_DEMAND_MODE		1
/*BFD role*/
#define BFD_ACTIVE_ROLE		0
#define BFD_PASSIVE_ROLE	1

/*BFD state variables*/
struct bfdsession
{
	/*state variables defined by spec*/
	unsigned char SessionState;
	unsigned char RemoteSessionState;
	unsigned char LocalDiag;
	unsigned char DetectMult;
	unsigned int  LocalDiscr;
	unsigned int  RemoteDiscr;
	unsigned int  DesiredMinTxInterval;
	unsigned int  RequiredMinRxInterval;
	unsigned int  RemoteMinRxInterval;
	unsigned char DemandMode;
	unsigned char RemoteDemandMode;
	unsigned char AuthType;
	unsigned char AuthSeqKnown;
	unsigned int  RcvAuthSeq;
	unsigned int  XmitAuthSeq;
	
	/*other*/
	/*echo rx interval*/
	unsigned int LocalEchoRxInterval;
	unsigned int OldLocalEchoRxInterval;
	unsigned int RemoteEchoRxInterval;
	/*remote min tx interval*/
	unsigned int  RemoteMinTxInterval;
	/*remote DetectMult*/
	unsigned char RemoteDetectMult;
	unsigned int UserDesiredMinTxInterval;
	//the old DesiredMinTxInterval (for increasing case)
	unsigned int OldDesiredMinTxInterval;
	//the old RequiredMinRxInterval (for reducing case)
	unsigned int OldRequiredMinRxInterval;
};

#define BFD_OFF		0
#define BFD_ON		1

struct bfdsessiondata
{
	struct bfdsessiondata	*next;

	struct bfdsession	session;
	
	unsigned char	Interface[16];
	unsigned char	LocalIP[16];
	unsigned char	RemoteIP[16];
	unsigned short	LocalRecvPort;
	unsigned short	LocalSendPort;
	unsigned short	LocalSendFinalPort;
	unsigned short	RemotePort;
	int		LocalRecvSock;
	int		LocalSendSock;
	int		LocalSendFinalSock;

	/*role*/
	unsigned char	Role;

	//periodic tx
	struct bfd_callout PeriodicTxTime;
	struct timeval	PeriodicTxInterval;
	unsigned char	PeriodicTxEnable;
	unsigned char	PeriodicTxState;
	
	/*save current DemandMode*/
	unsigned char	CurDemandMode;
	
	//detection time
	struct timeval	DetectIntervalAsynBase;/*asynchronous mode*/
	struct timeval	DetectIntervalDemdBase;/*demand mode*/
	struct timeval	DetectInterval;
	struct timeval	LastRecvTime;
	struct timeval	DetectTimeout;
	
	/*poll sequence*/
	unsigned char	PollSeq;
	unsigned char	PollSeqSent;
	unsigned char	RecvPollSeq;

	/*auth's ID & PSWD*/
	unsigned char	AuthKeyID;
	unsigned char	AuthKeyLen;
	unsigned char	AuthKey[20];//password's len=1~16,md5=16,sha1=20

/*start echo******************************************/
	int		EchoSock;
	unsigned short	EchoPort;
	unsigned char	EchoEnable;
	unsigned char	EchoState;
	struct bfd_callout EchoTxTime;
	struct timeval	EchoTxInterval;
	struct timeval	EchoDetectInterval;
	struct timeval	EchoLastRecvTime;
	struct timeval	EchoDetectTimeout;
	unsigned int	EchoTxSeq;
	unsigned char	EchoDetectMult;
	unsigned char	EchoPacketSent;
	unsigned char	EchoAdminEnable;
	/*echo-related variables for sending echo packets by using of a raw socket*/
	int		EchoSendRawSock;
	unsigned char	EchoIfHasARP;
	int		EchoIfIndex;
	unsigned char	EchoRemoteIP[16];
	unsigned char	EchoRemoteMacAddr[6];
	unsigned char	EchoRemoteMacAddrGot;
	unsigned char	EchoSourceIP[16];
	unsigned short	EchoSourcePort;
/*end echo********************************************/

#ifdef PERIODIC_POLL_WHEN_DEMAND
	//periodic poll
	unsigned char		PeriodicPollAdminEnable;
	unsigned char		PeriodicPollState;
	unsigned int		PeriodicPollInterval;
	struct bfd_callout	PeriodicPollTime;
#endif //PERIODIC_POLL_WHEN_DEMAND

	//others
	unsigned char	debug;//debug
	unsigned char	GotGSNotify;//wt-146, got the graceful-shutdown notify
	int		dhcpc_pid;//dhcp pid
	unsigned int	ttl;//IP TTL
	unsigned int	dscp;//IP DSCP
	unsigned int	ethprio;//ethernet priority

};


struct bfdcfg
{
	struct bfdsessiondata	bfddata;
	int			bfdcmd;
};


static inline char *BFD_GET_INF_NAME(struct bfdsessiondata *p)
{
	if(p&&p->Interface[0]) return (char *)p->Interface;
	return "unknown";
}
#define BFDDBG_FMT	"[%s]%s> "
#define BFDDBG_ARG(x)	BFD_GET_INF_NAME(x),__FUNCTION__
#define BFDDBG(x)	((x)&&(x)->debug)

#define BFD_MAX(a,b)	((a)>(b))?(a):(b)
#define BFD_MIN(a,b)	((a)>(b))?(b):(a)

static inline void BFD_US2TIMEVAL(unsigned int *pus, struct timeval *ptv)
{
	if(pus && ptv)
	{
		ptv->tv_sec=(*pus)/BFD_ONESECOND;
		ptv->tv_usec=(*pus)%BFD_ONESECOND;
	}
}

static inline void BFD_TIMEVAL2US(struct timeval *ptv, unsigned int *pus)
{
	if(ptv && pus)
	{
		*pus=ptv->tv_sec*BFD_ONESECOND+ptv->tv_usec;
	}
}

static inline void BFD_LONGUS2TIMEVAL(unsigned long long int *pus, struct timeval *ptv)
{
	if(pus && ptv)
	{
		ptv->tv_sec=(*pus)/BFD_ONESECOND;
		ptv->tv_usec=(*pus)%BFD_ONESECOND;
	}
}

static inline void BFD_TIMEVAL2LONGUS(struct timeval *ptv, unsigned long long int *pus)
{
	if(ptv && pus)
	{
		*pus=ptv->tv_sec*BFD_ONESECOND+ptv->tv_usec;
	}
}

static inline void BFD_TIMEVALMULT(struct timeval *ptv, unsigned char *pmult, struct timeval *pnewtv )
{
	if(ptv && pmult && pnewtv)
	{
		pnewtv->tv_sec=ptv->tv_sec*(*pmult);
		pnewtv->tv_usec=ptv->tv_usec*(*pmult);
		if(pnewtv->tv_usec>=BFD_ONESECOND)
		{
			pnewtv->tv_sec=pnewtv->tv_sec+(pnewtv->tv_usec/BFD_ONESECOND);
			pnewtv->tv_usec=pnewtv->tv_usec%BFD_ONESECOND;
		}
	}
}

/*function*/
extern void bfd_dump_data( unsigned char *d, unsigned int len );
extern void bfd_sessiondata_dump( struct bfdsessiondata *b );

extern char * bfd_getstate_str( int s );
extern char * bfd_getdiagcode_str( int s );
extern char * bfd_getevent_str( int s );

extern int bfd_session_init( struct bfdsessiondata *pbsd );
extern int bfd_create_basic_msg( struct bfdsessiondata *pbsd, unsigned char *b, unsigned int len );
extern int bfd_msg_add_auth_part( struct bfdsessiondata *pbsd, unsigned char *b, unsigned int len );
extern int bfd_get_msg_len( unsigned char *b );
extern int bfd_set_poll_bit( unsigned char *b, unsigned char v );
extern int bfd_set_final_bit( unsigned char *b, unsigned char v );

//echo
extern int bfd_echo_init(struct bfdsessiondata *p);
extern void bfd_echo_process(struct bfdsessiondata *pb);
extern int bfd_echo_timeout( struct bfdsessiondata *pbsd );

//periodic tx
extern void bfdtx_update_interval(struct bfdsessiondata *pbsd);
extern void bfdtx_update_enable(struct bfdsessiondata *pbsd);
extern void bfdtx_onoff(struct bfdsessiondata *pb);
extern void bfdtx_update(struct bfdsessiondata *pb);

//detection time
extern void bfd_detect_update_interval(struct bfdsessiondata *pbsd);

//state machine
extern void bfd_handle_statemachine( struct bfdsessiondata *pbsd, int event);

//process bfd session
extern void bfd_process(struct bfdsessiondata *pb);
extern void bfd_loop(struct bfdsessiondata *pb);

//poll sequence
extern void bfd_pollseq_increase( struct bfdsessiondata *pb );
extern void bfd_pollseq_get_finalbit( struct bfdsessiondata *pbsd );

#ifdef _HANDLE_CONSOLE_
extern void bfd_handle_console(struct bfdsessiondata *pb);
#endif //_HANDLE_CONSOLE_

#endif /*_BFDLIB_H_*/

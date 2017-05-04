#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bfdlib.h"
#include "libmd5.h"
#include "libsha1.h"


//md5-related
int bfd_md5hash( unsigned char *b, unsigned int len, unsigned char *h )
{
	struct LibMD5Context m;
	
	if(b==NULL ||  h==NULL) return -1;

	LibMD5Init(&m);
	LibMD5Update(&m, b, len);
	LibMD5Final(h, &m);
	return 0;
}

void bfd_test_md5hash(void)
{
	{
		unsigned char md5hash[16];
		
		#define TESTA   "abc"
		bfd_md5hash( TESTA, strlen(TESTA), md5hash );
		printf( "bfd_test_md5hash: count the md5hash for %s\n",  TESTA);
		bfd_dump_data( md5hash, 16 );
		printf("Should match:\n");
		printf("\t90015098 3cd24fb0 d6963f7d 28e17f72\n");
	}

	{
		//  key =         0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b
		//  key_len =     16 bytes
		//  data =        "Hi There"
		//  data_len =    8  bytes
		//  digest =      0x9294727a3638bb1c13f48ef8158bfc9d
		unsigned char key[16];
		unsigned int key_len=16;
		unsigned char data[]="Hi There";
		unsigned char data_len=8;
		unsigned char md5hash[16];
		memset( key, 0x0b, 16 );
		libhmac_md5(data,data_len,key,key_len,md5hash);
		printf( "bfd_test_md5hash: count libhmac_md5 test 1\n");
		bfd_dump_data( md5hash, 16 );
		printf("Should match:\n");
		printf("\t9294727a 3638bb1c 13f48ef8 158bfc9d\n");		
	}

	{
		//  key =         "Jefe"
		//  data =        "what do ya want for nothing?"
		//  data_len =    28 bytes
		//  digest =      0x750c783e6ab0b503eaa86e310a5db738
  		unsigned char key[]="Jefe";
		unsigned int key_len=4;
		unsigned char data[]="what do ya want for nothing?";
		unsigned char data_len=28;
		unsigned char md5hash[16];
		//memset( key, 0x0b, 16 );
		libhmac_md5(data,data_len,key,key_len,md5hash);
		printf( "bfd_test_md5hash: count libhmac_md5 test 2\n");
		bfd_dump_data( md5hash, 16 );
		printf("Should match:\n");
		printf("\t750c783e 6ab0b503 eaa86e31 0a5db738\n");		
	}

	{
		//  key =         0xAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
		//  key_len       16 bytes
		//  data =        0xDDDDDDDDDDDDDDDDDDDD...
		//                ..DDDDDDDDDDDDDDDDDDDD...
		//                ..DDDDDDDDDDDDDDDDDDDD...
		//                ..DDDDDDDDDDDDDDDDDDDD...
		//                ..DDDDDDDDDDDDDDDDDDDD
		//  data_len =    50 bytes
		//  digest =      0x56be34521d144c88dbb8c733f0e8b3f6
  		unsigned char key[16];
		unsigned int key_len=16;
		unsigned char data[50];
		unsigned char data_len=50;
		unsigned char md5hash[16];
		memset( key, 0xAA, 16 );
		memset( data, 0xDD, 50 );
		libhmac_md5(data,data_len,key,key_len,md5hash);
		printf( "bfd_test_md5hash: count libhmac_md5 test 3\n");
		bfd_dump_data( md5hash, 16 );
		printf("Should match:\n");
		printf("\t56be3452 1d144c88 dbb8c733 f0e8b3f6\n");
	}
}

//sha1-related
int bfd_sha1hash( unsigned char *b, unsigned int len, unsigned char *h )
{
	SHA1Context s;
	
	if(b==NULL ||  h==NULL) return -1;

	SHA1Reset(&s);
	SHA1Input(&s, b, len);
	SHA1Result(&s, h);
	return 0;
}

void bfd_test_sha1hash(void)
{
	unsigned char sha1hash[20];
	
#define TESTA   "abc"
	bfd_sha1hash( TESTA, strlen(TESTA), sha1hash );
	printf( "bfd_test_sha1hash: count the sha1hash for %s\n",  TESTA);
	bfd_dump_data( sha1hash, 20 );
	printf("Should match:\n");
	printf("\tA9993E36 4706816A BA3E2571 7850C26C 9CD0D89D\n");
}

void bfd_dump_data( unsigned char *d, unsigned int len )
{
	unsigned int i;
	
	//printf("\n");
	for( i=0; i<len; i++ )
	{
		if((i&0xf)==0) printf("%u\t ", i);
		printf( "%02x", d[i] );
		if((i&0xf)==0xf) printf("\n");
		else if((i&0x7)==0x7) printf("   ");
		else printf(" ");
	}
	if((i&0xf)!=0x0) printf("\n");
}

static char *bfdstatestr[] =
{
	"AdminDown",
	"Down",
	"Init",
	"Up",
	NULL
};

char * bfd_getstate_str( int s )
{
	switch(s)
	{
	case ST_ADMINDOWN: 
	case ST_DOWN:
	case ST_INIT:
	case ST_UP:
		return bfdstatestr[s];
	}
	
	return "UnknownState";
}

static char *bfdeventstr[] =
{
	"ADMINDOWN",
	"DOWN",
	"INIT",
	"UP",
	"DETECT_TIMEOUT",
	"ECHO_TIMEOUT",
	"ENABLE_SESSION",
	"DISABLE_SESSION",
	"UNKNOWN",
	NULL
};

char *bfd_getevent_str( int s )
{
	switch(s)
	{
	case EVENT_ADMINDOWN:
	case EVENT_DOWN:
	case EVENT_INIT:
	case EVENT_UP:
	case EVENT_DETECT_TIMEOUT:
	case EVENT_ECHO_TIMEOUT:
	case EVENT_ENABLE_SESSION:
	case EVENT_DISABLE_SESSION:
	case EVENT_UNKNOWN:
		return bfdeventstr[s];
	}
	return "UnknownEvent";
}

static char *bfddiagcodestr[] =
{
        "No Diagnostic",
        "Control Detection Time Expired",
        "Echo Function Failed",
        "Neighbor Signaled Session Down",
        "Forwarding Plane Reset",
        "Path Down",
        "Concatenated Path Down",
        "Administratively Down",
        "Reverse Concatenated Path Down",
        NULL
};

char * bfd_getdiagcode_str( int s )
{
	switch(s)
	{
	case DC_NO_DIAG:
	case DC_TIME_EXPIRED:
	case DC_ECHO_FAILED:
	case DC_NSSD:
	case DC_FORWARD_RESET:
	case DC_PATH_DOWN:
	case DC_CONPATH_DOWN:
	case DC_ADMIN_DOWN:
	case DC_RCONPATH_DOWN:
		return bfddiagcodestr[s];
	}
	
	return "UnknownCode";
}

static char *bfdauthtypestr[] =
{
        "None",
        "Simple Password",
        "Keyed MD5",
        "Meticulous Keyed MD5",
        "Keyed SHA1",
        "Meticulous Keyed SHA1",
        NULL
};

static char * bfd_getauthtype_str( int s )
{
	switch(s)
	{
	case AUTH_NONE:
	case AUTH_PASSWORD:
	case AUTH_KEYED_MD5:
	case AUTH_METI_KEYED_MD5:
	case AUTH_KEYED_SHA1:
	case AUTH_METI_KEYED_SHA1:
		return bfdauthtypestr[s];
	}
	
	return "UnknownAuthType";
}
static void bfd_session_dump( struct bfdsession *s )
{
	if(s)
	{
		printf( "\t============================================================\n" );
		printf( "\tSessionState=%u(%s), RemoteSessionState=%u(%s)\n", 
				s->SessionState, bfd_getstate_str(s->SessionState),
				s->RemoteSessionState, bfd_getstate_str(s->RemoteSessionState) );
		printf( "\tLocalDiag=%u(%s)\n", 
				s->LocalDiag, bfd_getdiagcode_str(s->LocalDiag) );
		printf( "\tDetectMult=%u, RemoteDetectMult=%u\n", 
				s->DetectMult, s->RemoteDetectMult );
		printf( "\tLocalDiscr=0x%08x, RemoteDiscr=0x%08x\n", 
				s->LocalDiscr, s->RemoteDiscr );
		printf( "\tDesiredMinTxInterval=%u, RemoteMinTxInterval=%u\n", 
				s->DesiredMinTxInterval, s->RemoteMinTxInterval );
		printf( "\tRequiredMinRxInterval=%u, RemoteMinRxInterval=%u\n", 
				s->RequiredMinRxInterval, s->RemoteMinRxInterval );
		printf( "\tDemandMode=%u, RemoteDemandMode=%u\n", 
				s->DemandMode, s->RemoteDemandMode );
		printf( "\tAuthType=%u(%s), XmitAuthSeq=0x%08x\n", 
				s->AuthType, bfd_getauthtype_str(s->AuthType), s->XmitAuthSeq );
		printf( "\tAuthSeqKnown=%u, RcvAuthSeq=0x%08x\n", 
				s->AuthSeqKnown, s->RcvAuthSeq );
		printf( "\tLocalEchoRxInterval=%u, RemoteEchoRxInterval=%u\n", 
				s->LocalEchoRxInterval, s->RemoteEchoRxInterval  );
		printf( "\tOldLocalEchoRxInterval=%u, UserDesiredMinTxInterval=%u\n", 
				s->OldLocalEchoRxInterval, s->UserDesiredMinTxInterval );
		printf( "\tOldDesiredMinTxInterval=%u, OldRequiredMinRxInterval=%u\n", 
				s->OldDesiredMinTxInterval, s->OldRequiredMinRxInterval  );
		printf( "\t============================================================\n" );
	}
}

static void bfd_echodata_dump( struct bfdsessiondata *b )
{
	if(b)
	{
		printf( "\t============================================================\n" );
		printf( "\tEchoPort=%u, EchoSourcePort=%u, EchoSock=%d, EchoSendRawSock=%d\n", 
				b->EchoPort, b->EchoSourcePort, b->EchoSock, b->EchoSendRawSock );
		printf( "\tEchoEnable=%u, EchoState=%u\n", 
				b->EchoEnable, b->EchoState );
		printf( "\tEchoAdminEnable=%u, EchoDetectMult=%u\n", 
				b->EchoAdminEnable, b->EchoDetectMult ); 
		printf( "\tEchoTxSeq=0x%08x, EchoPacketSent=%u\n", b->EchoTxSeq, b->EchoPacketSent ); 
		printf( "\tEchoTxInterval={%d.%06d}, EchoTxTime.c_time={%d.%06d}\n", 
				b->EchoTxInterval.tv_sec, b->EchoTxInterval.tv_usec,
				b->EchoTxTime.c_time.tv_sec, b->EchoTxTime.c_time.tv_usec);
		printf( "\tEchoDetectInterval={%d.%06d}\n", 
				b->EchoDetectInterval.tv_sec, b->EchoDetectInterval.tv_usec);
		printf( "\tEchoLastRecvTime={%d.%06d}, EchoDetectTimeout={%d.%06d}\n", 
				b->EchoLastRecvTime.tv_sec, b->EchoLastRecvTime.tv_usec,
				b->EchoDetectTimeout.tv_sec, b->EchoDetectTimeout.tv_usec);
		printf( "\tEchoIfIndex=%d, EchoIfHasARP=%u, EchoRemoteMacAddrGot=%u\n",
				b->EchoIfIndex, b->EchoIfHasARP, b->EchoRemoteMacAddrGot);
		printf( "\tEchoRemoteIP=%s, EchoRemoteMacAddr=%02x%02x%02x%02x%02x%02x\n", 
				b->EchoRemoteIP, b->EchoRemoteMacAddr[0], b->EchoRemoteMacAddr[1], 
				b->EchoRemoteMacAddr[2], b->EchoRemoteMacAddr[3], b->EchoRemoteMacAddr[4],
				b->EchoRemoteMacAddr[5] );
		printf( "\tEchoSourcePort=%u, EchoSourceIP=%s\n", 
				b->EchoSourcePort, b->EchoSourceIP );
		printf( "\t============================================================\n" );
	}
}

static void _bfd_sessiondata_dump( struct bfdsessiondata *b )
{
	if(b)
	{
		struct timeval tv;
		int i;

		printf( "\t============================================================\n" );
		printf( "\tInterface=%s, Role=%s\n", b->Interface, (b->Role==BFD_PASSIVE_ROLE)?"Passive Role":"Active Role" );
		printf( "\tLocalIP=%s, LocalRecvPort=%u\n", 
				b->LocalIP, b->LocalRecvPort );
		printf( "\tLocalSendPort=%u, LocalSendFinalPort=%u\n", 
				b->LocalSendPort, b->LocalSendFinalPort );
		printf( "\tRemoteIP=%s, RemotePort=%u\n", b->RemoteIP, b->RemotePort );
		printf( "\tLocalRecvSock=%d, LocalSendSock=%d, LocalSendFinalSock=%d\n",
				b->LocalRecvSock, b->LocalSendSock, b->LocalSendFinalSock );
		//periodic tx
		printf( "\tPeriodicTxEnable=%u, PeriodicTxState=%u\n", b->PeriodicTxEnable, b->PeriodicTxState );
		printf( "\tPeriodicTxInterval={%d.%06d}, PeriodicTxTime={%d.%06d}\n", 
				b->PeriodicTxInterval.tv_sec, b->PeriodicTxInterval.tv_usec,
				b->PeriodicTxTime.c_time.tv_sec, b->PeriodicTxTime.c_time.tv_usec);
		printf( "\tCurDemandMode=%u\n", b->CurDemandMode );
		//detection time
		printf( "\tDetectIntervalAsynBase={%d.%06d}, DetectIntervalDemdBase={%d.%06d}\n", 
				b->DetectIntervalAsynBase.tv_sec, b->DetectIntervalAsynBase.tv_usec,
				b->DetectIntervalDemdBase.tv_sec, b->DetectIntervalDemdBase.tv_usec);
		printf( "\tDetectInterval={%d.%06d}\n", 
				b->DetectInterval.tv_sec, b->DetectInterval.tv_usec);
		printf( "\tLastRecvTime={%d.%06d}, DetectTimeout={%d.%06d}\n", 
				b->LastRecvTime.tv_sec, b->LastRecvTime.tv_usec,
				b->DetectTimeout.tv_sec, b->DetectTimeout.tv_usec);

		printf( "\tPollSeq=%u, PollSeqSent=%u, RecvPollSeq=%u\n", b->PollSeq, b->PollSeqSent, b->RecvPollSeq );
#ifdef PERIODIC_POLL_WHEN_DEMAND
		printf( "\tPeriodicPollAdminEnable=%u, PeriodicPollState=%u\n", 
				b->PeriodicPollAdminEnable, b->PeriodicPollState );
		printf( "\tPeriodicPollInterval=%u, PeriodicPollTime={%d.%06d}\n", 
				b->PeriodicPollInterval, b->PeriodicPollTime.c_time.tv_sec, b->PeriodicPollTime.c_time.tv_usec );
#endif //PERIODIC_POLL_WHEN_DEMAND
		printf( "\tAuthKeyID=%u, AuthKeyLen=%u, AuthKey=", b->AuthKeyID, b->AuthKeyLen );
			for(i=0;i<b->AuthKeyLen;i++)
			{
				printf( "%02x", b->AuthKey[i] );
				if( (i&0x3)==0x3 ) printf(" ");
			}
			printf(" (");
			for(i=0;i<b->AuthKeyLen;i++)
			{
				if(isprint(b->AuthKey[i])) 
					printf( "%c", b->AuthKey[i] );
				else
					printf( "." );
				if( (i&0x3)==0x3 ) printf(" ");
			}
			printf(")\n");

		printf( "\tdebug=%u, dhcpc_pid=%d, GotGSNotify=%u\n", b->debug, b->dhcpc_pid, b->GotGSNotify );
		printf( "\tttl=%u, dscp=%u, ethprio=%u\n", b->ttl, b->dscp, b->ethprio );

		gettimeofday( &tv, NULL );
		printf( "\tCurrentTime={%d.%06d}\n", tv.tv_sec, tv.tv_usec);
		printf( "\t============================================================\n" );
	}
}

void bfd_sessiondata_dump( struct bfdsessiondata *b )
{
	if(b)
	{
		printf( "*******************************************************************************\n" );	
		printf( "Dump %s information:\n",  b->Interface );
		bfd_session_dump( &b->session );
		bfd_echodata_dump(b);
		_bfd_sessiondata_dump(b);
		printf( "*******************************************************************************\n" );	
	}
}

void bfd_msg_dump(unsigned char *b, int len)
{
	struct bfdmsg	*m=(struct bfdmsg *)b;
	
	if( b && len )
	{
		printf( "dump BFD message (len=%d)=================================\n", len );
		printf( "\tvers=%u, diag=%u(%s), sta=%u(%s)\n",
				m->vers, 
				m->diag, bfd_getdiagcode_str(m->diag),
				m->sta, bfd_getstate_str(m->sta) );
		printf( "\tflag=(%s%s%s%s%s%s), detect=%u, len=%u\n",
				m->flag_p?"P":"",
				m->flag_f?"F":"",
				m->flag_c?"C":"",
				m->flag_a?"A":"",
				m->flag_d?"D":"",
				m->flag_m?"M":"",				
				m->detect_mult, m->length);
		printf( "\tmydisc=%08x, yourdisc=%08x\n", ntohl(m->mydisc), ntohl(m->yourdisc) );
		printf( "\tmin tx=%u, min rx=%u, min echo rx=%u\n", 
				ntohl(m->txinterval), ntohl(m->rxinterval), ntohl(m->echorxinterval) );

		if( m->flag_a && (m->length>24) )
		{
			struct bfd_auth *a=(struct bfd_auth *)m->authdata;
			int i, j;

			switch(a->type)
			{
			case AUTH_PASSWORD:
				{
				struct bfd_auth_password *ap=(struct bfd_auth_password *)a;
				printf( "\tauth type=%u, len=%u, keyid=%u\n", 
						ap->type, ap->len, ap->keyid);
				printf( "\tauth password=" );
				i=ap->len-3;
				if(i>0)
				{
					if(i>16) i=16;
					for( j=0; j<i; j++)
					{
						printf( "%02x", ap->password[j] );
					}
					printf( "(" );
					for( j=0; j<i; j++)
					{
						if( isprint(ap->password[j]) ) printf( "%c", ap->password[j] );
						else printf( "." );
					}
					printf( ")\n" );
				}else{
					printf( "auth len error!!!\n" );
				}
				
				}
				break;
			case AUTH_KEYED_MD5:
			case AUTH_METI_KEYED_MD5:
				{
				struct bfd_auth_md5 *am=(struct bfd_auth_md5 *)a;
				printf( "\tauth type=%u, len=%u, keyid=%u, res=%u, seq=%08x\n", 
						am->type, am->len, am->keyid, am->res, ntohl(am->seq) );
				printf( "\tauth digest=");
				for(i=0; i<16; i++)
				{
					printf( "%02x", am->digest[i] );
					if((i&0x3)==0x3) printf( " " );
				}
				printf( "\n" );
				}
				break;
			case AUTH_KEYED_SHA1:
			case AUTH_METI_KEYED_SHA1:
				{
				struct bfd_auth_sha1 *as=(struct bfd_auth_sha1 *)a;
				printf( "\tauth type=%u, len=%u, keyid=%u, res=%u, seq=%08x\n", 
						as->type, as->len, as->keyid, as->res, ntohl(as->seq) );
				printf( "\tauth digest=");
				for(i=0; i<20; i++)
				{
					printf( "%02x", as->hash[i] );
					if((i&0x3)==0x3) printf( " " );
				}
				printf( "\n" );

				}
				break;
			default:
				printf("\tunknown auth type=%u\n", a->type );
				break;
			}
		}
		printf( "end dump BFD message=====================================\n", len );
	}
	return;	
}

int bfd_translate2event(int st)
{
	int ret;

	switch(st)
	{
	case ST_ADMINDOWN:
		ret=EVENT_ADMINDOWN;
		break;
	case ST_DOWN:
		ret=EVENT_DOWN;
		break;
	case ST_INIT:
		ret=EVENT_INIT;
		break;
	case ST_UP:
		ret=EVENT_UP;
		break;
	default:
		ret=EVENT_UNKNOWN;
		break;
	}

	return ret;
}

int bfd_session_init( struct bfdsessiondata *pbsd )
{
	struct bfdsession *s;

	srand((unsigned int)time(NULL));
	
	if(pbsd)
	{
		memset( pbsd, 0, sizeof(struct bfdsessiondata) );
		//default: active-role and asynchronous-mode bfd session
		pbsd->LocalRecvPort=3784;
		pbsd->LocalSendPort=49512;
		pbsd->LocalSendFinalPort=49513;
		pbsd->RemotePort=3784;
		pbsd->LocalRecvSock=-1;
		pbsd->LocalSendSock=-1;
		pbsd->LocalSendFinalSock=-1;
		pbsd->PeriodicTxEnable=1;
		pbsd->PeriodicTxState=BFD_OFF;
		pbsd->Role=BFD_ACTIVE_ROLE;
		pbsd->debug=BFD_OFF;
		pbsd->GotGSNotify=0;
		pbsd->ttl=255;
		pbsd->dscp=0;
		pbsd->ethprio=0;
#ifdef PERIODIC_POLL_WHEN_DEMAND
		pbsd->PeriodicPollAdminEnable=1;
		pbsd->PeriodicPollState=BFD_OFF;
		pbsd->PeriodicPollInterval=PERIODIC_POLL_DEFAULT_INTERVAL;
#endif //PERIODIC_POLL_WHEN_DEMAND

		//session part
		s=&pbsd->session;
		s->SessionState=ST_DOWN;
		s->RemoteSessionState=ST_DOWN;
		s->LocalDiscr=(unsigned int)rand();
		s->RemoteDiscr=0;
		s->LocalDiag=DC_NO_DIAG;
		s->DesiredMinTxInterval=1*BFD_ONESECOND;
		s->UserDesiredMinTxInterval=s->DesiredMinTxInterval;
		s->RequiredMinRxInterval=1*BFD_ONESECOND;
		s->RemoteMinRxInterval=1;
		s->RemoteMinTxInterval=1;
		s->DemandMode=0;
		s->RemoteDemandMode=0;
		s->DetectMult=3;
		s->AuthType=AUTH_NONE;
		s->AuthSeqKnown=0;
		//s->RcvAuthSeq
		s->XmitAuthSeq=(unsigned int)(rand() & 0xffff);
		s->LocalEchoRxInterval=0;
		s->OldLocalEchoRxInterval=0;
		s->RemoteEchoRxInterval=0;
		s->RemoteDetectMult=3;		
		s->OldDesiredMinTxInterval=0;
		s->OldRequiredMinRxInterval=0;
		return 0;
	}
	return -1;
}


int bfd_create_basic_msg( struct bfdsessiondata *pbsd, unsigned char *b, unsigned int len )
{
	struct bfdmsg	*pbm;
	struct bfdsession *pbs;
	
	if( !pbsd || !b || len<sizeof(struct bfdmsg) )
		return -1;

	pbm=(struct bfdmsg *)b;
	pbs=&pbsd->session;
	memset( b, 0, len );
	pbm->vers=BFD_VER;
	pbm->diag=pbs->LocalDiag;
	pbm->sta=pbs->SessionState;
	
	pbm->flag_p=0;
	pbm->flag_f=0;
	pbm->flag_c=0;
	pbm->flag_a=0; //bfd_msg_add_auth_part() will handle auth part
	if((pbs->SessionState==ST_UP)&&(pbs->RemoteSessionState==ST_UP))
		pbm->flag_d=pbs->DemandMode;
	else
		pbm->flag_d=0;
	pbm->flag_m=0;	
	
	pbm->detect_mult=pbs->DetectMult;
	pbm->length=24;

	pbm->mydisc=htonl( pbs->LocalDiscr );
	pbm->yourdisc=htonl( pbs->RemoteDiscr );
	pbm->txinterval=htonl( pbs->DesiredMinTxInterval );
	pbm->rxinterval=htonl( pbs->RequiredMinRxInterval );
	pbm->echorxinterval=htonl( pbs->LocalEchoRxInterval );
	return 0;
}

int bfd_msg_add_auth_part( struct bfdsessiondata *pbsd, unsigned char *b, unsigned int len )
{
	struct bfdmsg	*pbm;
	struct bfdsession *pbs;
	
	if( !pbsd || !b || len<sizeof(struct bfdmsg) )
		return -1;

	pbm=(struct bfdmsg *)b;
	pbs=&pbsd->session;
	switch( pbs->AuthType )
	{
	case AUTH_NONE:
		pbm->flag_a=0;
		pbm->length=24;
		break;
	case AUTH_PASSWORD:
		{
		struct bfd_auth_password *pbap;
		//basic session
		pbm->flag_a=1;
		pbm->length=24+3+pbsd->AuthKeyLen;
		//auth session
		pbap=(struct bfd_auth_password *)pbm->authdata;
		pbap->type=AUTH_PASSWORD;
		pbap->len=3+pbsd->AuthKeyLen;
		pbap->keyid=pbsd->AuthKeyID;
		memcpy( pbap->password, pbsd->AuthKey, pbsd->AuthKeyLen);
		}
		break;
	case AUTH_KEYED_MD5:
	case AUTH_METI_KEYED_MD5:
		{
		struct bfd_auth_md5 *pbam;
		unsigned char md5hash[16];
		//basic session
		pbm->flag_a=1;
		pbm->length=24+24;
		//auth session
		pbam=(struct bfd_auth_md5 *)pbm->authdata;	
		pbam->type=pbs->AuthType;
		pbam->len=24;
		pbam->keyid=pbsd->AuthKeyID;
		pbam->res=0;
		pbam->seq=htonl(pbs->XmitAuthSeq);
		pbs->XmitAuthSeq++;//??
		memcpy( pbam->digest, pbsd->AuthKey, pbsd->AuthKeyLen );
		memset( md5hash, 0, 16 );
		bfd_md5hash( (unsigned char*)pbm, 24+8+pbsd->AuthKeyLen, md5hash );
		memcpy( pbam->digest, md5hash, 16 );
		//if(BFDDBG(pbsd)) 
		//{
		//	printf( "dump counted md5hash:\n" );
		//	bfd_dump_data( md5hash,16 );
		//}
		}
		break;
	case AUTH_KEYED_SHA1:
	case AUTH_METI_KEYED_SHA1:
		{
		struct bfd_auth_sha1*pbas;
		unsigned char sha1hash[20];
		//basic session
		pbm->flag_a=1;
		pbm->length=24+28;
		//auth session
		pbas=(struct bfd_auth_sha1 *)pbm->authdata;	
		pbas->type=pbs->AuthType;
		pbas->len=28;
		pbas->keyid=pbsd->AuthKeyID;
		pbas->res=0;
		pbas->seq=htonl(pbs->XmitAuthSeq);
		pbs->XmitAuthSeq++;//??
		memcpy( pbas->hash, pbsd->AuthKey, pbsd->AuthKeyLen );
		memset( sha1hash, 0, 20 );
		bfd_sha1hash( (unsigned char *)pbm, 24+8+pbsd->AuthKeyLen, sha1hash );
		memcpy( pbas->hash, sha1hash, 20 );
		//if(BFDDBG(pbsd)) 
		//{
		//	printf( "dump counted sha1hash:\n" );
		//	bfd_dump_data( sha1hash,20 );
		//}
		}
		break;
	default:
		break;
	}
	return 0;
}

//ok=0, failed:<0
int bfd_msg_check_auth_part( struct bfdsessiondata *pbsd, unsigned char *b, unsigned int len )
{
	struct bfdmsg	*pbm;
	struct bfdsession *pbs;

	//if(BFDDBG(pbsd)) printf( BFDDBG_FMT "enter\n", BFDDBG_ARG(pbsd) );
	if( !pbsd || !b || len<sizeof(struct bfdmsg) )
		return -100;

	pbm=(struct bfdmsg *)b;
	pbs=&pbsd->session;
	switch( pbs->AuthType )
	{
	case AUTH_NONE:
		//nothing to check
		break;
	case AUTH_PASSWORD:
		{
		struct bfd_auth_password *pbap;
		pbap=(struct bfd_auth_password *)pbm->authdata;
		if(	(len<(24+3+pbsd->AuthKeyLen)) ||
			(pbm->length!=(24+3+pbsd->AuthKeyLen)) || 
			(pbap->type!=AUTH_PASSWORD) ) 
			return -120;
		if( pbap->keyid!=pbsd->AuthKeyID ) return -121;
		if( pbap->len!=(3+pbsd->AuthKeyLen) ) return -122;
		if( memcmp(pbap->password, pbsd->AuthKey, pbsd->AuthKeyLen) ) return -123;
		}
		break;
	case AUTH_KEYED_MD5:
	case AUTH_METI_KEYED_MD5:
		{
		struct bfd_auth_md5 *pbam;
		unsigned char md5hash[16], orimd5hash[16];
		pbam=(struct bfd_auth_md5 *)pbm->authdata;
		if(	(len<(24+24)) ||
			(pbm->length!=(24+24)) || 
			(pbam->type!=pbs->AuthType) ) 
			return -140;
		if( pbam->keyid!=pbsd->AuthKeyID ) return -141;
		if( pbam->len!=24 ) return -142;
		if( pbs->AuthSeqKnown==1 )
		{
			unsigned int maxseq, minseq, tempseq;
			unsigned char outrange=0;
			if(pbs->AuthType==AUTH_KEYED_MD5)
			{
				minseq=pbs->RcvAuthSeq;
				maxseq=pbs->RcvAuthSeq+(3*pbm->detect_mult);
			}else{ //AUTH_METI_KEYED_MD5
				minseq=pbs->RcvAuthSeq+1;
				maxseq=pbs->RcvAuthSeq+(3*pbm->detect_mult);
			}
			tempseq=ntohl(pbam->seq);
			if( minseq<maxseq )
			{
				if( (tempseq<minseq) || (tempseq>maxseq) )
					outrange=1;
			}else{ //minseq>=maxseq
				if( (tempseq<minseq) && (tempseq>maxseq) )
					outrange=1;
			}
			if(outrange)
			{	
				if(BFDDBG(pbsd)) printf( BFDDBG_FMT "sequence num lies outside of the range\n", BFDDBG_ARG(pbsd) );
				return -143;
			}				
		}else{
			pbs->AuthSeqKnown=1;
		}
		pbs->RcvAuthSeq=ntohl(pbam->seq);


		memcpy( orimd5hash, pbam->digest, 16 );
		memcpy( pbam->digest, pbsd->AuthKey, pbsd->AuthKeyLen );
		memset( md5hash, 0, 16 );
		bfd_md5hash( (unsigned char *)pbm, 24+8+pbsd->AuthKeyLen, md5hash );
		memcpy( pbam->digest, orimd5hash, 16 );//restore
		if( memcmp( md5hash, orimd5hash, 16 ) )
		{
			if(BFDDBG(pbsd))
			{
				printf( BFDDBG_FMT "md5 hash not match\n", BFDDBG_ARG(pbsd) );
				printf( "dump ori md5hash:\n" );
				bfd_dump_data( orimd5hash,16 );
				printf( "dump new md5hash:\n" );
				bfd_dump_data( md5hash,16 );
			}
			return -144;		
		}
		
		}
		break;
	case AUTH_KEYED_SHA1:
	case AUTH_METI_KEYED_SHA1:
		{
		struct bfd_auth_sha1 *pbas;
		unsigned char sha1hash[20], orisha1hash[20];
		pbas=(struct bfd_auth_sha1 *)pbm->authdata;
		if(	(len<(24+28)) ||
			(pbm->length!=(24+28)) || 
			(pbas->type!=pbs->AuthType) ) 
			return -160;
		if( pbas->keyid!=pbsd->AuthKeyID ) return -161;
		if( pbas->len!=28 ) return -162;
		if( pbs->AuthSeqKnown==1 )
		{
			unsigned int maxseq, minseq, tempseq;
			unsigned char outrange=0;
			if(pbs->AuthType==AUTH_KEYED_SHA1)
			{
				minseq=pbs->RcvAuthSeq;
				maxseq=pbs->RcvAuthSeq+(3*pbm->detect_mult);
			}else{ //AUTH_METI_KEYED_SHA1
				minseq=pbs->RcvAuthSeq+1;
				maxseq=pbs->RcvAuthSeq+(3*pbm->detect_mult);
			}			
			tempseq=ntohl(pbas->seq);
			if( minseq<maxseq )
			{
				if( (tempseq<minseq) || (tempseq>maxseq) )
					outrange=1;
			}else{ //minseq>=maxseq
				if( (tempseq<minseq) && (tempseq>maxseq) )
					outrange=1;
			}
			if(outrange)
			{	
				if(BFDDBG(pbsd)) printf( BFDDBG_FMT "sequence num lies outside of the range\n", BFDDBG_ARG(pbsd) );
				return -163;
			}
		}else{
			pbs->AuthSeqKnown=1;
		}
		pbs->RcvAuthSeq=ntohl(pbas->seq);

		memcpy( orisha1hash, pbas->hash, 20 );
		memcpy( pbas->hash, pbsd->AuthKey, pbsd->AuthKeyLen );
		memset( sha1hash, 0, 20 );
		bfd_sha1hash( (unsigned char *)pbm, 24+8+pbsd->AuthKeyLen, sha1hash );
		memcpy( pbas->hash, orisha1hash, 20 );//restore
		if( memcmp( sha1hash, orisha1hash, 20 ) )
		{
			if(BFDDBG(pbsd))
			{
				printf( BFDDBG_FMT "sha1 hash not match\n", BFDDBG_ARG(pbsd) );
				printf( "dump ori sha1hash:\n" );
				bfd_dump_data( orisha1hash,20 );
				printf( "dump new sha1hash:\n" );
				bfd_dump_data( sha1hash, 20 );
			}
			return -164;		
		}
		
		}
		break;
	default:
		if(BFDDBG(pbsd)) printf( BFDDBG_FMT "unsupport auth type=%u\n", BFDDBG_ARG(pbsd),  pbs->AuthType );
		return -101;
		break;
	}

	//if(BFDDBG(pbsd)) printf( BFDDBG_FMT "exit with 0\n", BFDDBG_ARG(pbsd) );
	return 0;
}

int bfd_set_poll_bit( unsigned char *b, unsigned char v )
{	
	if(b)
	{
		struct bfdmsg	*pbm=(struct bfdmsg *)b;
		pbm->flag_p=v?1:0;		
		return 0;
	}
	return -1;
}

int bfd_set_final_bit( unsigned char *b, unsigned char v )
{	
	if(b)
	{
		struct bfdmsg	*pbm=(struct bfdmsg *)b;
		pbm->flag_f=v?1:0;		
		return 0;
	}
	return -1;
}

int bfd_get_msg_len( unsigned char *b )
{
	if(b)
	{
		struct bfdmsg	*pbm=(struct bfdmsg *)b;	
		return (int)pbm->length;
	}
	return 0;
}

int bfd_process_recv_msg( struct bfdsessiondata *pbsd, unsigned char *b, unsigned int len )
{
	struct bfdmsg	*pbm;
	struct bfdsession *pbs;
	int ret;
	
	if( !pbsd || !b || len<sizeof(struct bfdmsg) )
		return -1;

	pbm=(struct bfdmsg *)b;
	pbs=&pbsd->session;
	
	if(pbm->vers!=BFD_VER) return -1;
	
	if(pbm->length<24) return -2;
	if(pbm->flag_a && (pbm->length<26)) return -3;
	if(pbm->length>len) return -4;
	
	if(pbm->detect_mult==0) return -5;
	if(pbm->flag_m) return -6;
	if(pbm->mydisc==0) return -7;
	if(pbm->yourdisc!=0)
	{
		if(pbm->yourdisc!=htonl(pbs->LocalDiscr))
	    		return -8;
	}
	if(pbm->yourdisc==0)
	{
		if( (pbm->sta!=ST_DOWN)&&(pbm->sta!=ST_ADMINDOWN) )
			return -9;
	}
	
	if(pbm->yourdisc==0)
	{
		//select the session based on some combination of other fields
		//source addressing information, my disc, and the interface
		//if a matching session is not found, a new session may be created or 
		//the packet may be discarded.
	}
	
	if(pbm->flag_a&&(pbs->AuthType==AUTH_NONE)) return -10;
	if(!pbm->flag_a&&(pbs->AuthType!=AUTH_NONE)) return -11;
	
	//check auth part
	ret=bfd_msg_check_auth_part( pbsd, b, len );
	if( ret<0 )
	{
		if(BFDDBG(pbsd)) printf( BFDDBG_FMT "call bfd_msg_check_auth_part() failed (ret=%d) !!!\n", BFDDBG_ARG(pbsd), ret ); 
		return ret;
	}
	
	pbs->RemoteDiscr=ntohl(pbm->mydisc);
	pbs->RemoteSessionState=pbm->sta;
	pbs->RemoteDemandMode=pbm->flag_d;
	pbs->RemoteDetectMult=pbm->detect_mult;
	pbs->RemoteMinRxInterval=ntohl(pbm->rxinterval);
	pbs->RemoteMinTxInterval=ntohl(pbm->txinterval);
	pbs->RemoteEchoRxInterval=ntohl(pbm->echorxinterval);

	if(pbm->flag_f) bfd_pollseq_get_finalbit( pbsd );

	//update the transmit interval
	bfdtx_update_interval(pbsd);
	
	//update the detection time
	bfd_detect_update_interval(pbsd);
	
	//update the state machine
	bfd_handle_statemachine( pbsd, bfd_translate2event(pbm->sta) );
	if(pbs->SessionState==ST_ADMINDOWN)
		return -13;

	//detect the special case for wt-146
	//When BFD session on RG receives a poll with a Diag code set to "Path Down",
	//it MUST perform the following actions
	if((pbm->sta==ST_ADMINDOWN)&&(pbm->flag_p)&&(pbm->diag==DC_PATH_DOWN))
	{
		if(BFDDBG(pbsd)) printf( BFDDBG_FMT "got the graceful-shutdown notify\n", BFDDBG_ARG(pbsd) ); 
		pbsd->GotGSNotify=1;
	}

	//check if demand mode should become active or not.
	bfd_active_demand_mode(pbsd);

	//check if periodic tx should become active or not
	bfdtx_update(pbsd);
	
	if(pbm->flag_p)
	{
		//send a BFD Control packet to the remote system 
		//with the Poll (P) bit clear, and the Final (F) bit set
		pbsd->RecvPollSeq=1;
	}
	
	//If the packet was not discarded, it has been received for purposes
	//of the Detection Time expiration rules in section 6.8.4.
	gettimeofday( &pbsd->LastRecvTime, NULL );
	timeradd( &pbsd->LastRecvTime, &pbsd->DetectInterval, &pbsd->DetectTimeout );
			
	return 0;
}


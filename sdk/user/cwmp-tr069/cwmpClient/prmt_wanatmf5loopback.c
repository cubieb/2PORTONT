#include "cwmpc_utility.h"
#include "prmt_wanatmf5loopback.h"
#include <linux/atm.h>
#include <linux/atmdev.h>
#include <sys/ioctl.h>
#include <config/autoconf.h>


extern unsigned int getWANDevInstNum( char *name );
extern unsigned int getWANConDevInstNum( char *name );
extern int getFirstATMVCEntryByInstNum( unsigned int wandevnum, unsigned int condevnum, MIB_CE_ATM_VC_T *p, unsigned int *id );


#define F5LB_NUMOFREP	1
#define F5LB_TIMEOUT	5000

enum{ F5LB_NONE=0, F5LB_REQUEST, F5LB_COMPLETE };

struct WANATMF5Loopback gWANATMF5LB={ F5LB_NONE,0,0,F5LB_NUMOFREP,F5LB_TIMEOUT,0,0,0,0,0 };
int gStartATMF5LB=0;
void *WANATMF5LB_thread(void *data);
void reset_WANATMF5LB( struct WANATMF5Loopback *p );
extern void cwmpDiagnosticDone();
extern void cwmpSetCpeHold(int holdit);


struct CWMP_OP tWANATMF5LBLeafOP = { getWANATMF5LB, setWANATMF5LB };
struct CWMP_PRMT tWANATMF5LBLeafInfo[] =
{
/*(name,			type,		flag,					op)*/
{"DiagnosticsState",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ|CWMP_DENY_ACT,	&tWANATMF5LBLeafOP},
{"NumberOfRepetitions",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,			&tWANATMF5LBLeafOP},
{"Timeout",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,			&tWANATMF5LBLeafOP},
{"SuccessCount",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWANATMF5LBLeafOP},
{"FailureCount",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWANATMF5LBLeafOP},
{"AverageResponseTime",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWANATMF5LBLeafOP},
{"MinimumResponseTime",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWANATMF5LBLeafOP},
{"MaximumResponseTime",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWANATMF5LBLeafOP}
};

struct CWMP_LEAF tWANATMF5LBLeaf[] =
{
{ &tWANATMF5LBLeafInfo[eF5_DiagnosticsState] },
{ &tWANATMF5LBLeafInfo[eF5_NumberOfRepetitions] },
{ &tWANATMF5LBLeafInfo[eF5_Timeout] },
{ &tWANATMF5LBLeafInfo[eF5_SuccessCount] },
{ &tWANATMF5LBLeafInfo[eF5_FailureCount] },
{ &tWANATMF5LBLeafInfo[eF5_AverageResponseTime] },
{ &tWANATMF5LBLeafInfo[eF5_MinimumResponseTime] },
{ &tWANATMF5LBLeafInfo[eF5_MaximumResponseTime] },
{ NULL }
};

int getWANATMF5LB(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	unsigned int	wandevnum, condevnum;	
	unsigned int	chainid;
	MIB_CE_ATM_VC_T	*pEntry, vc_entity;
	int		showdefault=1;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

#ifdef CONFIG_CWMP_TR181_SUPPORT
	
	if(gUseTR181)
	{
		//TR-181 has only 1 instance
		showdefault = 0;
	}
	else
#endif
	{
		//TR-181 already set vpi,vci by "Interface" parameter
		wandevnum = getWANDevInstNum( name );
		condevnum = getWANConDevInstNum( name );
		if( (wandevnum==0)||(condevnum==0) ) return ERR_9005;
		pEntry = &vc_entity;
		if( getFirstATMVCEntryByInstNum( wandevnum, condevnum, pEntry, &chainid )<0 )
			return ERR_9005;

		if( (gWANATMF5LB.vpi==pEntry->vpi) &&
		    (gWANATMF5LB.vci==pEntry->vci) )
			showdefault=0;
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "DiagnosticsState" )==0 )
	{
		if(showdefault==1)
			*data = strdup( "None" );
		else if( gWANATMF5LB.DiagState==F5LB_REQUEST )
			*data = strdup( "Requested" );
		else if( gWANATMF5LB.DiagState==F5LB_COMPLETE )
			*data = strdup( "Complete" );
		else
			*data = strdup( "None" );
	}else if( strcmp( lastname, "NumberOfRepetitions" )==0 )
	{	
		*data = uintdup( gWANATMF5LB.NumOfRep );
	}else if( strcmp( lastname, "Timeout" )==0 )
	{	
		*data = uintdup( gWANATMF5LB.Timeout );
	}else if( strcmp( lastname, "SuccessCount" )==0 )
	{	
		if(showdefault==1)
			*data = uintdup( 0 );
		else
			*data = uintdup( gWANATMF5LB.SuccessCount );
	}else if( strcmp( lastname, "FailureCount" )==0 )
	{	
		if(showdefault==1)
			*data = uintdup( 0 );
		else
			*data = uintdup( gWANATMF5LB.FailureCount );
	}else if( strcmp( lastname, "AverageResponseTime" )==0 )
	{	
		if(showdefault==1)
			*data = uintdup( 0 );
		else
			*data = uintdup( gWANATMF5LB.AvaRespTime );
	}else if( strcmp( lastname, "MinimumResponseTime" )==0 )
	{	
		if(showdefault==1)
			*data = uintdup( 0 );
		else
			*data = uintdup( gWANATMF5LB.MinRespTime );
	}else if( strcmp( lastname, "MaximumResponseTime" )==0 )
	{	
		if(showdefault==1)
			*data = uintdup( 0 );
		else
			*data = uintdup( gWANATMF5LB.MaxRespTime );

	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setWANATMF5LB(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char		*lastname = entity->info->name;
	unsigned int	*pUint=data;
	unsigned int	wandevnum,condevnum;	
	unsigned int	chainid;
	MIB_CE_ATM_VC_T	*pEntry, vc_entity;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

#ifdef CONFIG_CWMP_TR181_SUPPORT
	if(!gUseTR181)
#endif
	{
		wandevnum = getWANDevInstNum( name );
		condevnum = getWANConDevInstNum( name );
		if( (wandevnum==0)||(condevnum==0) ) return ERR_9005;
		pEntry = &vc_entity;
		if( getFirstATMVCEntryByInstNum( wandevnum, condevnum, pEntry, &chainid )<0 )
			return ERR_9005;
	}

	if( strcmp( lastname, "DiagnosticsState" )==0 )
	{
		char *buf=data;
		
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "Requested" )!=0 ) return ERR_9007;

		//create a pthread to start diagnostics
		if( gWANATMF5LB.DiagState==F5LB_REQUEST ) return ERR_9001;
		reset_WANATMF5LB( &gWANATMF5LB );
		gStartATMF5LB=1;
		gWANATMF5LB.DiagState=F5LB_REQUEST;

#ifdef CONFIG_CWMP_TR181_SUPPORT
		if(!gUseTR181)
#endif
		{
			gWANATMF5LB.vpi=pEntry->vpi;
			gWANATMF5LB.vci=pEntry->vci;
		}
	}else if( strcmp( lastname, "NumberOfRepetitions" )==0 )
	{	
		if(pUint==NULL) return ERR_9007;
		if(*pUint<1) return ERR_9007;
		gWANATMF5LB.NumOfRep = *pUint;
	}else if( strcmp( lastname, "Timeout" )==0 )
	{	
		if(pUint==NULL) return ERR_9007;
		if(*pUint<1) return ERR_9007;
		gWANATMF5LB.Timeout = *pUint;
	}else{
		return ERR_9005;
	}
	
	return 0;
}


/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
/*refer to  boa/src/LINUX/fmmgmt.c*/
void reset_WANATMF5LB( struct WANATMF5Loopback *p )
{
	p->DiagState = F5LB_NONE;
	p->SuccessCount = 0;
	p->FailureCount = 0;
	p->AvaRespTime = 0;
	p->MinRespTime = 0;
	p->MaxRespTime = 0;
}

#ifdef CONFIG_MIDDLEWARE
extern int sendOpertionDoneMsg2MidIntf(char opertion);
#endif

void *WANATMF5LB_thread(void *data)
{
	int numofrep=0;
	unsigned int totalresptime=0;
	
	CWMPDBG( 0, ( stderr, "<%s:%d>Start WANATMF5LB_thread\n", __FUNCTION__, __LINE__ ) );
	CWMPDBG( 1, ( stderr, "<%s:%d>vpi:%d, vci:%d, numofrep:%d, timeout:%d(ms)\n", 
			__FUNCTION__, __LINE__,gWANATMF5LB.vpi, gWANATMF5LB.vci, gWANATMF5LB.NumOfRep, gWANATMF5LB.Timeout ) );
	
	numofrep=gWANATMF5LB.NumOfRep;
	while(numofrep>0)
	{
		struct atmif_sioc mysio;
		ATMOAMLBReq lbReq;
		ATMOAMLBState lbState;
		int skfd;
		unsigned int resptime=0;

		numofrep--;
		if((skfd = socket(PF_ATMPVC, SOCK_DGRAM, 0)) < 0)
		{
			CWMPDBG( 0, ( stderr, "<%s:%d>open socket error\n", __FUNCTION__, __LINE__ ) );
			goto F5LB_NEXTLOOP;
		}


		memset(&lbReq, 0, sizeof(ATMOAMLBReq));
		lbReq.Scope = 0; //0:Segment, 1:End-to-End
		lbReq.vpi = gWANATMF5LB.vpi;
		lbReq.vci = gWANATMF5LB.vci;
		{	// convert max of 32 hex decimal string into its 16 octets value
			char LocationID[33]="FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
			unsigned char *tmpValue;
			char *tmpStr;
			int curidx,len,i;

			len = strlen(LocationID);
			curidx = 16;
			for (i=0; i<32; i+=2)
			{
				// Loopback Location ID
				curidx--;
				tmpValue = (unsigned char *)&lbReq.LocID[curidx];
				if (len > 0)
				{
					len -= 2;
					if (len < 0)
						len = 0;
					tmpStr = LocationID + len;
					*tmpValue = strtoul(tmpStr, 0, 16);
					*tmpStr='\0';
				}
				else
					*tmpValue = 0;
			}			
		}
		
		mysio.number = 0;// ATM interface number
		mysio.arg = (void *)&lbReq;
		// Start the loopback test
		if (ioctl(skfd, ATM_OAM_LB_START, &mysio)<0)
		{
			CWMPDBG( 0, ( stderr, "<%s:%d>ioctl(ATM_OAM_LB_START) error\n", __FUNCTION__, __LINE__ ) );
			close(skfd);
			goto F5LB_NEXTLOOP;
		}
		
		
		memset(&lbState, 0, sizeof(ATMOAMLBState));
		lbState.vpi = gWANATMF5LB.vpi;
		lbState.vci = gWANATMF5LB.vci;
		lbState.Tag = lbReq.Tag;
		// Query the loopback status
		mysio.arg = (void *)&lbState;
		//while loop
		{
			struct timeval st_tv, end_tv, stop_tv;
			
			gettimeofday( &st_tv, NULL );
			stop_tv.tv_sec = st_tv.tv_sec + gWANATMF5LB.Timeout/1000;
			stop_tv.tv_usec = st_tv.tv_usec + (gWANATMF5LB.Timeout%1000)*1000;
			end_tv.tv_sec=0;
			end_tv.tv_usec=0;
			while (1)
			{
				if (ioctl(skfd, ATM_OAM_LB_STATUS, &mysio)<0)
				{
					CWMPDBG( 0, ( stderr, "<%s:%d>ioctl(ATM_OAM_LB_STATUS) error\n", __FUNCTION__, __LINE__ ) );
					// Stop the loopback test
					mysio.arg = (void *)&lbReq;
					ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
					close(skfd);
					goto F5LB_NEXTLOOP;
				}

				gettimeofday( &end_tv, NULL );
				resptime= (end_tv.tv_sec-st_tv.tv_sec)*1000+(end_tv.tv_usec-st_tv.tv_usec)/1000;
				//CWMPDBG( 0, ( stderr, "<%s:%d>st_tv: %d %d\n", __FUNCTION__, __LINE__, st_tv.tv_sec, st_tv.tv_usec ) );
				//CWMPDBG( 0, ( stderr, "<%s:%d>stop_tv: %d %d\n", __FUNCTION__, __LINE__, stop_tv.tv_sec, stop_tv.tv_usec ) );
				//CWMPDBG( 0, ( stderr, "<%s:%d>end_tv: %d %d\n", __FUNCTION__, __LINE__, end_tv.tv_sec, end_tv.tv_usec ) );
				if (lbState.count[0] > 0)
				{
					CWMPDBG( 0, ( stderr, "<%s:%d>loopback successfully\n", __FUNCTION__, __LINE__ ) );
					break;
				}

				if( (end_tv.tv_sec>stop_tv.tv_sec) ||
				    ( (end_tv.tv_sec==stop_tv.tv_sec) &&
				      (end_tv.tv_usec>=stop_tv.tv_usec) ) )
				{
					//timeout;
					CWMPDBG( 0, ( stderr, "<%s:%d>timeout error\n", __FUNCTION__, __LINE__ ) );
					// Stop the loopback test
					mysio.arg = (void *)&lbReq;
					ioctl(skfd, ATM_OAM_LB_STOP, &mysio);
					close(skfd);
					goto F5LB_NEXTLOOP;
				}

			}
		}
		
		
		
		// Stop the loopback test
		mysio.arg = (void *)&lbReq;
		if (ioctl(skfd, ATM_OAM_LB_STOP, &mysio)<0)
		{
			CWMPDBG( 0, ( stderr, "<%s:%d>ioctl(ATM_OAM_LB_STOP) error\n", __FUNCTION__, __LINE__ ) );
			close(skfd);
			goto F5LB_NEXTLOOP;
		}
		close(skfd);

		gWANATMF5LB.SuccessCount++;
		totalresptime+=resptime;
		if(resptime>gWANATMF5LB.MaxRespTime)
			gWANATMF5LB.MaxRespTime=resptime;
		if( (gWANATMF5LB.MinRespTime==0) || ( resptime<gWANATMF5LB.MinRespTime) )
			gWANATMF5LB.MinRespTime=resptime;
		continue;

F5LB_NEXTLOOP:
		gWANATMF5LB.FailureCount++;

	}


//F5LB_END:
	gWANATMF5LB.DiagState=F5LB_COMPLETE;
	if(gWANATMF5LB.SuccessCount)
		gWANATMF5LB.AvaRespTime=totalresptime/gWANATMF5LB.SuccessCount;
	CWMPDBG( 0, ( stderr, "<%s:%d>Finished WANATMF5LB_thread\n", __FUNCTION__, __LINE__ ) );

#ifdef CONFIG_MIDDLEWARE
	sendOpertionDoneMsg2MidIntf('2');
#endif
	
	//send event to the acs
	cwmpDiagnosticDone();
	cwmpSetCpeHold(0);
	return NULL;
	
}

void cwmpStartATMF5LB(void)
{
	pthread_t pid;
	
	if( pthread_create( &pid, NULL, WANATMF5LB_thread ,NULL  ) )
	{
		gWANATMF5LB.DiagState=F5LB_COMPLETE;
		return;
	}
	pthread_detach(pid);
		
	return;
}

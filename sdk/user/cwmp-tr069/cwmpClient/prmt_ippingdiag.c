#include "prmt_ippingdiag.h"
#include "prmt_wancondevice.h"
#ifdef CONFIG_CWMP_TR181_SUPPORT
#include "tr181/prmt_ip_if.h"
#endif
#include <pthread.h>
#include <config/autoconf.h>
#include <libcwmp.h>


int gStartPing=0;

struct CWMP_OP tIPPingDiagnosticsLeafOP = { getIPPingDiag, setIPPingDiag };
struct CWMP_PRMT tIPPingDiagnosticsLeafInfo[] =
{
/*(name,		type,		flag,				op)*/
{"DiagnosticsState",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ|CWMP_DENY_ACT,&tIPPingDiagnosticsLeafOP},
{"Interface",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tIPPingDiagnosticsLeafOP},
{"Host",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tIPPingDiagnosticsLeafOP},
{"NumberOfRepetitions",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tIPPingDiagnosticsLeafOP},
{"Timeout",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tIPPingDiagnosticsLeafOP},
{"DataBlockSize",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tIPPingDiagnosticsLeafOP},
{"DSCP",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tIPPingDiagnosticsLeafOP},
{"SuccessCount",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIPPingDiagnosticsLeafOP},
{"FailureCount",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIPPingDiagnosticsLeafOP},
{"AverageResponseTime",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIPPingDiagnosticsLeafOP},
{"MinimumResponseTime",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIPPingDiagnosticsLeafOP},
{"MaximumResponseTime",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIPPingDiagnosticsLeafOP}
};
enum eIPPingDiagnosticsLeaf
{
	eIPDiagnosticsState,
	eIPInterface,
	eIPHost,
	eIPNumberOfRepetitions,
	eIPTimeout,
	eIPDataBlockSize,
	eIPDSCP,
	eIPSuccessCount,
	eIPFailureCount,
	eIPAverageResponseTime,
	eIPMinimumResponseTime,
	eIPMaximumResponseTime
};
struct CWMP_LEAF tIPPingDiagnosticsLeaf[] =
{
{ &tIPPingDiagnosticsLeafInfo[eIPDiagnosticsState] },
{ &tIPPingDiagnosticsLeafInfo[eIPInterface] },
{ &tIPPingDiagnosticsLeafInfo[eIPHost] },
{ &tIPPingDiagnosticsLeafInfo[eIPNumberOfRepetitions] },
{ &tIPPingDiagnosticsLeafInfo[eIPTimeout] },
{ &tIPPingDiagnosticsLeafInfo[eIPDataBlockSize] },
{ &tIPPingDiagnosticsLeafInfo[eIPDSCP] },
{ &tIPPingDiagnosticsLeafInfo[eIPSuccessCount] },
{ &tIPPingDiagnosticsLeafInfo[eIPFailureCount] },
{ &tIPPingDiagnosticsLeafInfo[eIPAverageResponseTime] },
{ &tIPPingDiagnosticsLeafInfo[eIPMinimumResponseTime] },
{ &tIPPingDiagnosticsLeafInfo[eIPMaximumResponseTime] },
{ NULL }
};




enum {
	STATE_NONE,
	STATE_REQUESTED,
	STATE_COMPLETE,
	STATE_ERROR_HOSTNAME
};

static const char * strDiagnosticState[] = { "None", "Requested", "Complete", "Error_ConnotResolveHostName", 0 };
// Diagnostic Variables
static int iDiagnosticState;
static char *DiagnosticState = "";
static char Interface[256+1];
static char Host[256+1];
static unsigned int NumberOfRepitition = 1;
static unsigned int Timeout = 1000;
static unsigned int DataBlockSize = 64;
static unsigned int DSCP;
static unsigned int SuccessCount;
static unsigned int FailureCount;
static unsigned int AverageResponseTime;
static unsigned int MinimumResponseTime;
static unsigned int MaxmumResponseTime;
static int pingThread = 0;
extern int icmp_test(char *intf, char *host, unsigned int count, unsigned int timeout, unsigned int datasize, unsigned char tos,
	unsigned int *cntOK, unsigned int *cntFail, unsigned int *timeAvg, unsigned int *timeMin, unsigned int *timeMax) ;

#ifdef CONFIG_MIDDLEWARE
extern int sendOpertionDoneMsg2MidIntf(char opertion);
#endif

static void *ipping_thread(void *arg) {
	int res;
	char ifname[IFNAMSIZ];
	char *pifname = NULL;

	// reset counters
	SuccessCount = FailureCount = 0;
	AverageResponseTime = MinimumResponseTime = MaxmumResponseTime = 0;

#ifdef CONFIG_CWMP_TR181_SUPPORT
	if(gUseTR181)
	{
		if(get_ip_if_ifname(Interface, ifname) == 0)
		{
			printf("convert %s failed\n", Interface);
			pifname = NULL;
		}
		else
			pifname = ifname;
	}
	else
	{
		// TR-098
		if (transfer2IfName(Interface, ifname))
		{
			printf("convert %s failed\n", Interface);
			pifname = NULL;
		}
		else
		{
			LANDEVNAME2BR0( ifname );
			pifname = ifname;
		}
	}
#else	//CONFIG_CWMP_TR181_SUPPORT
	if (transfer2IfName(Interface, ifname))
	{
		printf("convert %s failed\n", Interface);
		pifname = NULL;
	}
	else
	{
		LANDEVNAME2BR0( ifname );
		pifname = ifname;
	}
#endif	//CONFIG_CWMP_TR181_SUPPORT

	printf("converted %s->%s\n", Interface, ifname);

	//DSCP is bit 2~7 of TOS field
	res = icmp_test(pifname, Host, NumberOfRepitition, Timeout, DataBlockSize, DSCP << 2, &SuccessCount,
		&FailureCount, &AverageResponseTime, &MinimumResponseTime, &MaxmumResponseTime);

	if (res == -2) {
		iDiagnosticState = STATE_ERROR_HOSTNAME;
	} else {
		iDiagnosticState = STATE_COMPLETE;
	}

#ifdef CONFIG_MIDDLEWARE
	sendOpertionDoneMsg2MidIntf('1');
#endif

	cwmpDiagnosticDone();
	printf("ping test return %d\n", res);
END:
	pingThread = 0;

	return NULL;
}

int getIPPingDiag(char *name, struct CWMP_LEAF *entity, int *type, void **data) {

	// ASSERT
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	switch(getIndexOf(tIPPingDiagnosticsLeaf, entity->info->name)) {
	case 0: //DiagnosticState
		*data = strdup( strDiagnosticState[iDiagnosticState] );
		break;
	case 1: //Interface
		*data = strdup(Interface);
		break;
	case 2: // Host
		*data = strdup(Host);
		break;
	case 3: // #ofRepetitions
		*data = uintdup(NumberOfRepitition);
		break;
	case 4: // timeout
		*data = uintdup(Timeout);
		break;
	case 5: // datablock size
		*data = uintdup(DataBlockSize);
		break;
	case 6: // DSCP
		*data = uintdup(DSCP);
		break;
	case 7: // successCount
		*data = uintdup(SuccessCount);
		break;
	case 8: // Failcount
		*data = uintdup(FailureCount);
		break;
	case 9: // avgRespTime
		*data = uintdup(AverageResponseTime);
		break;
	case 10: // minRespTime
		*data = uintdup(MinimumResponseTime);
		break;
	case 11: // maxRespTime
		*data = uintdup(MaxmumResponseTime);
		break;
	default:
		return ERR_9005;

	}

	return 0;

}

void cwmpStartPingDiag() {
	pthread_t ping_pid;
	//printf("\n\nPing commence!!\n");
	if (pingThread) {
		printf("ping in progress, try again later=\n");
		return;
	}

	pingThread = 1;
	if( pthread_create( &ping_pid, NULL, ipping_thread, 0 ) != 0 )
	{
		iDiagnosticState = STATE_NONE;
		pingThread = 0;
		return;
	}
	pthread_detach(ping_pid);

}

int setIPPingDiag(char *name, struct CWMP_LEAF *entity, int type, void *data) {

	unsigned int *pNum;

	// sanity check
	if( (name==NULL) || (entity==NULL))
		return -1;
	if( entity->info->type!=type ) return ERR_9006;

	switch(getIndexOf(tIPPingDiagnosticsLeaf, entity->info->name)) {
	case 0: //DiagnosticState
		if (getStrIndexOf(strDiagnosticState, (char *)data) != STATE_REQUESTED)
			return ERR_9007;

		// already working
		if (STATE_REQUESTED == iDiagnosticState)
			return ERR_9004;

		iDiagnosticState = STATE_REQUESTED;
		gStartPing = 1;
		break;

	case 1: //Interface
		if (strlen((char *)data) > 256)
			return ERR_9007;
		strcpy(Interface, (char *)data);
		break;
	case 2: // Host
		if (strlen((char *)data) > 256)
			return ERR_9007;
		strcpy(Host, (char *)data);
		break;
	case 3: // #ofRepetitions
		pNum = (unsigned int *)data;

		if (!pNum ||( (*pNum) < 1))
			return ERR_9007;

		NumberOfRepitition = *pNum;
		break;
	case 4: // timeout
		pNum = (unsigned int *)data;

		if (!pNum ||( (*pNum) < 1))
			return ERR_9007;

		Timeout = *pNum;
		break;
	case 5: // datablock size
		pNum = (unsigned int *)data;

		if (!pNum ||( (*pNum) < 1) ||( (*pNum) > 65535))
			return ERR_9007;

		DataBlockSize = *pNum;
		break;
	case 6: // DSCP
		pNum = (unsigned int *)data;

		if (!pNum ||( (*pNum) > 64))
			return ERR_9007;

		DSCP = *pNum;
		break;

	default:
		return ERR_9005;

	}


	/*jiunming,default: return 0 if no errors */
	return 0;

}


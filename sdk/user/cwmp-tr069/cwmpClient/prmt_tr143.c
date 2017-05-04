#include "prmt_tr143.h"
#include <rtk/utility.h>
#include <sys/time.h>
#include "udpechoserverlib.h"
#ifdef CONFIG_USER_FTP_FTP_FTP
#ifdef _USE_RSDK_WRAPPER_
#include <sys/types.h>
#include <sys/wait.h>
#endif //_USE_RSDK_WRAPPER_
#include <signal.h>
#endif //CONFIG_USER_FTP_FTP_FTP
#ifdef _PRMT_TR143_


int clearWanUDPEchoItf( void );
int setWanUDPEchoItf( unsigned int ifindex );
int getWanUDPEchoItf( unsigned int *pifindex);
/*****************************************************************************************************/
char *strTR143State[] =
{
	"None",
	"Requested",
	"Completed",
	"Error_InitConnectionFailed",
	"Error_NoResponse",
	"Error_PasswordRequestFailed",
	"Error_LoginFailed",
	"Error_NoTransferMode",
	"Error_NoPASV",
	//download
	"Error_TransferFailed",
	"Error_IncorrectSize",
	"Error_Timeout",
	//upload
	"Error_NoCWD",
	"Error_NoSTOR",
	"Error_NoTransferComplete",

	"" /*eTR143_End, last one*/
};

struct TR143_Diagnostics gDownloadDiagnostics =
{
	DLWAY_DOWN,	/*Way*/
	eTR143_None,	/*DiagnosticsState*/
	NULL,		/*pInterface*/
	"",		/*IfName*/
	NULL,		/*pURL*/
	0,		/*DSCP*/
	0,		/*EthernetPriority*/
	0,		/*TestFileLength*/
	{0,0},		/*ROMTime*/
	{0,0},		/*BOMTime*/
	{0,0},		/*EOMTime*/
	0,		/*TestBytesReceived*/
	0,		/*TotalBytesReceived*/
	0,		/*TotalBytesSent*/
	{0,0},		/*TCPOpenRequestTime*/
	{0,0},		/*TCPOpenResponseTime*/
	0,		/*http_pid*/
	0		/*ftp_pid*/
};
int gStartTR143DownloadDiag=0;

struct TR143_Diagnostics gUploadDiagnostics =
{
	DLWAY_UP,	/*Way*/
	eTR143_None,	/*DiagnosticsState*/
	NULL,		/*pInterface*/
	"",		/*IfName*/
	NULL,		/*pURL*/
	0,		/*DSCP*/
	0,		/*EthernetPriority*/
	0,		/*TestFileLength*/
	{0,0},		/*ROMTime*/
	{0,0},		/*BOMTime*/
	{0,0},		/*EOMTime*/
	0,		/*TestBytesReceived*/
	0,		/*TotalBytesReceived*/
	0,		/*TotalBytesSent*/
	{0,0},		/*TCPOpenRequestTime*/
	{0,0},		/*TCPOpenResponseTime*/
	0,		/*http_pid*/
	0		/*ftp_pid*/
};
int gStartTR143UploadDiag=0;

/******UDPEchoConfig***************************************************************************/
struct CWMP_OP tUDPEchoConfigLeafOP = { getUDPEchoConfig, setUDPEchoConfig };
struct CWMP_PRMT tUDPEchoConfigLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tUDPEchoConfigLeafOP},
{"Interface",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tUDPEchoConfigLeafOP},
{"SourceIPAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tUDPEchoConfigLeafOP},
{"UDPPort",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tUDPEchoConfigLeafOP},
{"EchoPlusEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tUDPEchoConfigLeafOP},
{"EchoPlusSupported",		eCWMP_tBOOLEAN,	CWMP_READ,		&tUDPEchoConfigLeafOP},
{"PacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tUDPEchoConfigLeafOP},
{"PacketsResponded",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tUDPEchoConfigLeafOP},
{"BytesReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tUDPEchoConfigLeafOP},
{"BytesResponded",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tUDPEchoConfigLeafOP},
{"TimeFirstPacketReceived",	eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,&tUDPEchoConfigLeafOP},
{"TimeLastPacketReceived",	eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,&tUDPEchoConfigLeafOP},
};

struct CWMP_LEAF tUDPEchoConfigLeaf[] =
{
{ &tUDPEchoConfigLeafInfo[eUEC_Enable] },
{ &tUDPEchoConfigLeafInfo[eUEC_Interface] },
{ &tUDPEchoConfigLeafInfo[eUEC_SourceIPAddress] },
{ &tUDPEchoConfigLeafInfo[eUEC_UDPPort] },
{ &tUDPEchoConfigLeafInfo[eUEC_EchoPlusEnabled] },
{ &tUDPEchoConfigLeafInfo[eUEC_EchoPlusSupported] },
{ &tUDPEchoConfigLeafInfo[eUEC_PacketsReceived] },
{ &tUDPEchoConfigLeafInfo[eUEC_PacketsResponded] },
{ &tUDPEchoConfigLeafInfo[eUEC_BytesReceived] },
{ &tUDPEchoConfigLeafInfo[eUEC_BytesResponded] },
{ &tUDPEchoConfigLeafInfo[eUEC_TimeFirstPacketReceived] },
{ &tUDPEchoConfigLeafInfo[eUEC_TimeLastPacketReceived] },
{ NULL }
};

/******UploadDiagnostics***************************************************************************/
struct CWMP_OP tUploadDiagnosticsLeafOP = { getUploadDiagnostics, setUploadDiagnostics };
struct CWMP_PRMT tUploadDiagnosticsLeafInfo[] =
{
/*(name,			type,		flag,					op)*/
{"DiagnosticsState",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ|CWMP_DENY_ACT,	&tUploadDiagnosticsLeafOP},
{"Interface",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,			&tUploadDiagnosticsLeafOP},
{"UploadURL",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,			&tUploadDiagnosticsLeafOP},
{"DSCP",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,			&tUploadDiagnosticsLeafOP},
{"EthernetPriority",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,			&tUploadDiagnosticsLeafOP},
{"TestFileLength",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,			&tUploadDiagnosticsLeafOP},
{"ROMTime",			eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,		&tUploadDiagnosticsLeafOP},
{"BOMTime",			eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,		&tUploadDiagnosticsLeafOP},
{"EOMTime",			eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,		&tUploadDiagnosticsLeafOP},
{"TotalBytesSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tUploadDiagnosticsLeafOP},
{"TCPOpenRequestTime",		eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,		&tUploadDiagnosticsLeafOP},
{"TCPOpenResponseTime",		eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,		&tUploadDiagnosticsLeafOP},
};

struct CWMP_LEAF tUploadDiagnosticsLeaf[] =
{
{ &tUploadDiagnosticsLeafInfo[eUD_DiagnosticsState] },
{ &tUploadDiagnosticsLeafInfo[eUD_Interface] },
{ &tUploadDiagnosticsLeafInfo[eUD_UploadURL] },
{ &tUploadDiagnosticsLeafInfo[eUD_DSCP] },
{ &tUploadDiagnosticsLeafInfo[eUD_EthernetPriority] },
{ &tUploadDiagnosticsLeafInfo[eUD_TestFileLength] },
{ &tUploadDiagnosticsLeafInfo[eUD_ROMTime] },
{ &tUploadDiagnosticsLeafInfo[eUD_BOMTime] },
{ &tUploadDiagnosticsLeafInfo[eUD_EOMTime] },
{ &tUploadDiagnosticsLeafInfo[eUD_TotalBytesSent] },
{ &tUploadDiagnosticsLeafInfo[eUD_TCPOpenRequestTime] },
{ &tUploadDiagnosticsLeafInfo[eUD_TCPOpenResponseTime] },
{ NULL }
};

/******DownloadDiagnostics***************************************************************************/
struct CWMP_OP tDownloadDiagnosticsLeafOP = { getDownloadDiagnostics, setDownloadDiagnostics };
struct CWMP_PRMT tDownloadDiagnosticsLeafInfo[] =
{
/*(name,			type,		flag,					op)*/
{"DiagnosticsState",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ|CWMP_DENY_ACT,	&tDownloadDiagnosticsLeafOP},
{"Interface",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,			&tDownloadDiagnosticsLeafOP},
{"DownloadURL",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,			&tDownloadDiagnosticsLeafOP},
{"DSCP",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,			&tDownloadDiagnosticsLeafOP},
{"EthernetPriority",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,			&tDownloadDiagnosticsLeafOP},
{"ROMTime",			eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,		&tDownloadDiagnosticsLeafOP},
{"BOMTime",			eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,		&tDownloadDiagnosticsLeafOP},
{"EOMTime",			eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,		&tDownloadDiagnosticsLeafOP},
{"TestBytesReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tDownloadDiagnosticsLeafOP},
{"TotalBytesReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tDownloadDiagnosticsLeafOP},
{"TCPOpenRequestTime",		eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,		&tDownloadDiagnosticsLeafOP},
{"TCPOpenResponseTime",		eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,		&tDownloadDiagnosticsLeafOP},
};

struct CWMP_LEAF tDownloadDiagnosticsLeaf[] =
{
{ &tDownloadDiagnosticsLeafInfo[eDD_DiagnosticsState] },
{ &tDownloadDiagnosticsLeafInfo[eDD_Interface] },
{ &tDownloadDiagnosticsLeafInfo[eDD_DownloadURL] },
{ &tDownloadDiagnosticsLeafInfo[eDD_DSCP] },
{ &tDownloadDiagnosticsLeafInfo[eDD_EthernetPriority] },
{ &tDownloadDiagnosticsLeafInfo[eDD_ROMTime] },
{ &tDownloadDiagnosticsLeafInfo[eDD_BOMTime] },
{ &tDownloadDiagnosticsLeafInfo[eDD_EOMTime] },
{ &tDownloadDiagnosticsLeafInfo[eDD_TestBytesReceived] },
{ &tDownloadDiagnosticsLeafInfo[eDD_TotalBytesReceived] },
{ &tDownloadDiagnosticsLeafInfo[eDD_TCPOpenRequestTime] },
{ &tDownloadDiagnosticsLeafInfo[eDD_TCPOpenResponseTime] },
{ NULL }
};

/******PerformanceDiagnostic***************************************************************************/
struct CWMP_OP tPerformanceDiagnosticLeafOP = { getPerformanceDiagnostic, NULL };
struct CWMP_PRMT tPerformanceDiagnosticLeafInfo[] =
{
/*(name,			type,		flag,		op)*/
{"DownloadTransports",		eCWMP_tSTRING,	CWMP_READ,	&tPerformanceDiagnosticLeafOP},
{"UploadTransports",		eCWMP_tSTRING,	CWMP_READ,	&tPerformanceDiagnosticLeafOP},
};
enum ePerformanceDiagnosticLeaf
{
	eDownloadTransports,
	eUploadTransports
};
struct CWMP_LEAF tPerformanceDiagnosticLeaf[] =
{
{ &tPerformanceDiagnosticLeafInfo[eDownloadTransports] },
{ &tPerformanceDiagnosticLeafInfo[eUploadTransports] },
{ NULL }
};
/*****************************************************************************************************/
int getPerformanceDiagnostic(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
#ifdef CONFIG_USER_FTP_FTP_FTP
	char	*protocol_name="HTTP,FTP";
#else
	char	*protocol_name="HTTP";
#endif

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "DownloadTransports" )==0 )
	{
		 *data=strdup( protocol_name );
	}else if( strcmp( lastname, "UploadTransports" )==0 )
	{
		*data = strdup( protocol_name );
	}else{
		return ERR_9005;
	}

	return 0;
}

/*****************************************************************************************************/
int getDownloadDiagnostics(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "DiagnosticsState" )==0 )
	{
		 *data=strdup( strTR143State[gDownloadDiagnostics.DiagnosticsState] );
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		if( gDownloadDiagnostics.pInterface )
			*data = strdup( gDownloadDiagnostics.pInterface );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "DownloadURL" )==0 )
	{
		if( gDownloadDiagnostics.pURL )
			*data = strdup( gDownloadDiagnostics.pURL );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "DSCP" )==0 )
	{
		*data=uintdup( gDownloadDiagnostics.DSCP );
	}else if( strcmp( lastname, "EthernetPriority" )==0 )
	{
		*data=uintdup( gDownloadDiagnostics.EthernetPriority );
	}else if( strcmp( lastname, "ROMTime" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		*data=timevaldup( gDownloadDiagnostics.ROMTime );
	}else if( strcmp( lastname, "BOMTime" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		*data=timevaldup( gDownloadDiagnostics.BOMTime );
	}else if( strcmp( lastname, "EOMTime" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		*data=timevaldup( gDownloadDiagnostics.EOMTime );
	}else if( strcmp( lastname, "TestBytesReceived" )==0 )
	{
		*data=uintdup( gDownloadDiagnostics.TestBytesReceived );
	}else if( strcmp( lastname, "TotalBytesReceived" )==0 )
	{
		*data=uintdup( gDownloadDiagnostics.TotalBytesReceived );
	}else if( strcmp( lastname, "TCPOpenRequestTime" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		*data=timevaldup( gDownloadDiagnostics.TCPOpenRequestTime );
	}else if( strcmp( lastname, "TCPOpenResponseTime" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		*data=timevaldup( gDownloadDiagnostics.TCPOpenResponseTime );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setDownloadDiagnostics(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int *vUInt=data;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "DiagnosticsState" )==0 )
	{
		if( buf && strcmp( buf, "Requested" )==0 )
		{
			StopTR143DownloadDiag();
			gDownloadDiagnostics.DiagnosticsState=eTR143_Requested;
			gStartTR143DownloadDiag=1;
			return 0;
		}
		return ERR_9007;
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		char ifname[32];
		if( (buf==NULL) || strlen(buf)==0 )
		{
			if( gDownloadDiagnostics.pInterface )
			{
				free( gDownloadDiagnostics.pInterface );
				gDownloadDiagnostics.pInterface = NULL;
			}
			strcpy( gDownloadDiagnostics.IfName, "" );
			fprintf( stderr, "( set Interface:%s,%s)\n", "", buf?buf:"" );
			StopDownloadAndResetState();
			return 0;
		}else
		{
			if( transfer2IfName( buf, ifname )==0 )
			{
				LANDEVNAME2BR0(ifname);
				if( gDownloadDiagnostics.pInterface )
				{
					free( gDownloadDiagnostics.pInterface );
					gDownloadDiagnostics.pInterface = NULL;
				}
				gDownloadDiagnostics.pInterface = strdup( buf );
				strcpy( gDownloadDiagnostics.IfName, ifname );
				fprintf( stderr, "( set Interface:%s,%s)\n", ifname, buf );
				StopDownloadAndResetState();
				return 0;
			}
		}
		return ERR_9007;
	}else if( strcmp( lastname, "DownloadURL" )==0 )
	{
		if( gDownloadDiagnostics.pURL )
		{
			free( gDownloadDiagnostics.pURL );
			gDownloadDiagnostics.pURL = NULL;
		}
		if(buf) gDownloadDiagnostics.pURL = strdup( buf );
		StopDownloadAndResetState();
		return 0;
	}else if( strcmp( lastname, "DSCP" )==0 )
	{
		if( vUInt==NULL ) return ERR_9007;
		if( *vUInt>63 ) return ERR_9007;
		gDownloadDiagnostics.DSCP = *vUInt;
		StopDownloadAndResetState();
		return 0;
	}else if( strcmp( lastname, "EthernetPriority" )==0 )
	{
		if( vUInt==NULL ) return ERR_9007;
		if( *vUInt>7 ) return ERR_9007;
		gDownloadDiagnostics.EthernetPriority = *vUInt;
		StopDownloadAndResetState();
		return 0;
	}else{
		return ERR_9005;
	}

	return 0;
}

/*****************************************************************************************************/
int getUploadDiagnostics(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "DiagnosticsState" )==0 )
	{
		 *data=strdup( strTR143State[gUploadDiagnostics.DiagnosticsState] );
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		if( gUploadDiagnostics.pInterface )
			*data = strdup( gUploadDiagnostics.pInterface );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "UploadURL" )==0 )
	{
		if( gUploadDiagnostics.pURL )
			*data = strdup( gUploadDiagnostics.pURL );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, "DSCP" )==0 )
	{
		*data=uintdup( gUploadDiagnostics.DSCP );
	}else if( strcmp( lastname, "EthernetPriority" )==0 )
	{
		*data=uintdup( gUploadDiagnostics.EthernetPriority );
	}else if( strcmp( lastname, "TestFileLength" )==0 )
	{
		*data=uintdup( gUploadDiagnostics.TestFileLength );
	}else if( strcmp( lastname, "ROMTime" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		*data=timevaldup( gUploadDiagnostics.ROMTime );
	}else if( strcmp( lastname, "BOMTime" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		*data=timevaldup( gUploadDiagnostics.BOMTime );
	}else if( strcmp( lastname, "EOMTime" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		*data=timevaldup( gUploadDiagnostics.EOMTime );
	}else if( strcmp( lastname, "TotalBytesSent" )==0 )
	{
		*data=uintdup( gUploadDiagnostics.TotalBytesSent );
	}else if( strcmp( lastname, "TCPOpenRequestTime" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		*data=timevaldup( gUploadDiagnostics.TCPOpenRequestTime );
	}else if( strcmp( lastname, "TCPOpenResponseTime" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		*data=timevaldup( gUploadDiagnostics.TCPOpenResponseTime );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setUploadDiagnostics(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int *vUInt=data;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "DiagnosticsState" )==0 )
	{
		if( buf && strcmp( buf, "Requested" )==0 )
		{
			StopTR143UploadDiag();
			gUploadDiagnostics.DiagnosticsState=eTR143_Requested;
			gStartTR143UploadDiag=1;
			return 0;
		}
		return ERR_9007;
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		char ifname[32];
		if( (buf==NULL) || strlen(buf)==0 )
		{
			if( gUploadDiagnostics.pInterface )
			{
				free( gUploadDiagnostics.pInterface );
				gUploadDiagnostics.pInterface = NULL;
			}
			strcpy( gUploadDiagnostics.IfName, "" );
			fprintf( stderr, "( set Interface:%s,%s)\n", "", buf?buf:"" );
			StopUploadAndResetState();
			return 0;
		}else
		{
			if( transfer2IfName( buf, ifname )==0 )
			{
				LANDEVNAME2BR0(ifname);
				if( gUploadDiagnostics.pInterface )
				{
					free( gUploadDiagnostics.pInterface );
					gUploadDiagnostics.pInterface = NULL;
				}
				gUploadDiagnostics.pInterface = strdup( buf );
				strcpy( gUploadDiagnostics.IfName, ifname );
				fprintf( stderr, "( set Interface:%s,%s)\n", ifname, buf );
				StopUploadAndResetState();
				return 0;
			}
		}
		return ERR_9007;
	}else if( strcmp( lastname, "UploadURL" )==0 )
	{
		if( gUploadDiagnostics.pURL )
		{
			free( gUploadDiagnostics.pURL );
			gUploadDiagnostics.pURL = NULL;
		}
		if(buf) gUploadDiagnostics.pURL = strdup( buf );
		StopUploadAndResetState();
		return 0;
	}else if( strcmp( lastname, "DSCP" )==0 )
	{
		if( vUInt==NULL ) return ERR_9007;
		if( *vUInt>63 ) return ERR_9007;
		gUploadDiagnostics.DSCP = *vUInt;
		StopUploadAndResetState();
		return 0;
	}else if( strcmp( lastname, "EthernetPriority" )==0 )
	{
		if( vUInt==NULL ) return ERR_9007;
		if( *vUInt>7 ) return ERR_9007;
		gUploadDiagnostics.EthernetPriority = *vUInt;
		StopUploadAndResetState();
		return 0;
	}else if( strcmp( lastname, "TestFileLength" )==0 )
	{
		if( vUInt==NULL ) return ERR_9007;
		gUploadDiagnostics.TestFileLength = *vUInt;
		StopUploadAndResetState();
		return 0;
	}else{
		return ERR_9005;
	}

	return 0;
}

/*****************************************************************************************************/
int getUDPEchoConfig(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned short vShort=0;
	unsigned char buff[256]={0};
	struct ECHORESULT *result;
	struct timeval t={0,0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( TR143_UDPECHO_ENABLE, (void *)&vChar );
		*data=booldup( vChar!=0 );
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		mib_get( TR143_UDPECHO_ITFTYPE, (void *)&vChar );
		if( vChar==ITF_ALL )
		{
			*data = strdup( "" );
		}else if( vChar==ITF_WAN )
		{
			unsigned int ifindex;
			if( getWanUDPEchoItf(&ifindex)==0 )
				transfer2PathName( ifindex, buff );
			*data = strdup( buff );
		}else if( vChar<ITF_END )
		{
			transfer2PathNamefromItf( strItf[vChar], buff );
			*data = strdup( buff );
		}else
			*data = strdup( "" ); //or return ERR_9002
	}else if( strcmp( lastname, "SourceIPAddress" )==0 )
	{
		mib_get( TR143_UDPECHO_SRCIP, (void *)buff );
		if( buff[0]==0 && buff[1]==0 && buff[2]==0 && buff[3]==0 )
		{
			*data = strdup( "" );
		}else{
			struct in_addr *pSIP = (struct in_addr *)buff;
			*data = strdup( inet_ntoa(*pSIP) );
		}
	}else if( strcmp( lastname, "UDPPort" )==0 )
	{
		mib_get( TR143_UDPECHO_PORT, (void *)&vShort );
		*data=uintdup( vShort );
	}else if( strcmp( lastname, "EchoPlusEnabled" )==0 )
	{
		mib_get( TR143_UDPECHO_PLUS, (void *)&vChar );
		*data=booldup( vChar!=0 );
	}else if( strcmp( lastname, "EchoPlusSupported" )==0 )
	{
		*data=booldup( 1 );
	}else if( strcmp( lastname, "PacketsReceived" )==0 )
	{
		if( getShmem( (void**)&result, sizeof(struct ECHORESULT), ECHOTOK )<0 )
			*data=uintdup( 0 );
		else{
			*data=uintdup( result->PacketsReceived );
			detachShmem( result );
		}
	}else if( strcmp( lastname, "PacketsResponded" )==0 )
	{
		if( getShmem( (void**)&result, sizeof(struct ECHORESULT), ECHOTOK )<0 )
			*data=uintdup( 0 );
		else{
			*data=uintdup( result->PacketsResponded );
			detachShmem( result );
		}
	}else if( strcmp( lastname, "BytesReceived" )==0 )
	{
		if( getShmem( (void**)&result, sizeof(struct ECHORESULT), ECHOTOK )<0 )
			*data=uintdup( 0 );
		else{
			*data=uintdup( result->BytesReceived );
			detachShmem( result );
		}
	}else if( strcmp( lastname, "BytesResponded" )==0 )
	{
		if( getShmem( (void**)&result, sizeof(struct ECHORESULT), ECHOTOK )<0 )
			*data=uintdup( 0 );
		else{
			*data=uintdup( result->BytesResponded );
			detachShmem( result );
		}
	}else if( strcmp( lastname, "TimeFirstPacketReceived" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		if( getShmem( (void**)&result, sizeof(struct ECHORESULT), ECHOTOK )<0 )
			*data=timevaldup( t );
		else{
			*data=timevaldup( result->TimeFirstPacketReceived );
			detachShmem( result );
		}
	}else if( strcmp( lastname, "TimeLastPacketReceived" )==0 )
	{
		*type=eCWMP_tMICROSECTIME;
		if( getShmem( (void**)&result, sizeof(struct ECHORESULT), ECHOTOK )<0 )
			*data=timevaldup( t );
		else{
			*data=timevaldup( result->TimeLastPacketReceived );
			detachShmem( result );
		}
	}else{
		return ERR_9005;
	}

	return 0;
}

int setUDPEchoConfig(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char		*lastname = entity->info->name;
	char		*buf=data;
	int		*vInt=data;
	unsigned int	*vUInt=data;
	unsigned char	vChar;
	struct TR143_UDPEchoConfig olddata;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	UDPEchoConfigSave( &olddata );
	if( strcmp( lastname, "Enable" )==0 )
	{
		if(vInt)
		{
			vChar=(*vInt)?1:0;
			mib_set( TR143_UDPECHO_ENABLE, (void *)&vChar );
		}

		apply_add( CWMP_PRI_N, apply_UDPEchoConfig, CWMP_RESTART, 0, &olddata, sizeof(struct TR143_UDPEchoConfig ) );
		return 0;
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		char ifname[32];
		if(buf)
		{
			if( transfer2IfName( buf, ifname )==0 )
			{
				int	ifid;

				ifid = IfName2ItfId( ifname );
				if(ifid==-1) return ERR_9007;

				if(ifid==ITF_WAN)
				{
					unsigned int wanifindex;
					wanifindex=transfer2IfIndex(buf);
					if(wanifindex!=DUMMY_IFINDEX)
					{
						clearWanUDPEchoItf();
						setWanUDPEchoItf( wanifindex );
					}else
						return ERR_9007;
				}

				vChar=(unsigned char)ifid;
				mib_set( TR143_UDPECHO_ITFTYPE, (void *)&vChar );
			}else
				return ERR_9007;
		}

		apply_add( CWMP_PRI_N, apply_UDPEchoConfig, CWMP_RESTART, 0, &olddata, sizeof(struct TR143_UDPEchoConfig ) );
		return 0;
	}else if( strcmp( lastname, "SourceIPAddress" )==0 )
	{
		struct in_addr vInAddr;
		if(!buf) return ERR_9007;
		if(strlen(buf)==0)
			memset( &vInAddr, 0, sizeof(vInAddr) );
		else if(inet_aton(buf, &vInAddr)==0) //error
			return ERR_9007;
		mib_set( TR143_UDPECHO_SRCIP, (void *)&vInAddr );

		apply_add( CWMP_PRI_N, apply_UDPEchoConfig, CWMP_RESTART, 0, &olddata, sizeof(struct TR143_UDPEchoConfig ) );
		return 0;
	}else if( strcmp( lastname, "UDPPort" )==0 )
	{
		unsigned short vShort=0;

		if( vUInt==NULL ) return ERR_9007;
		if( *vUInt==0 || *vUInt>65535 ) return ERR_9007;
		vShort = *vUInt;
		mib_set( TR143_UDPECHO_PORT, (void *)&vShort );

		apply_add( CWMP_PRI_N, apply_UDPEchoConfig, CWMP_RESTART, 0, &olddata, sizeof(struct TR143_UDPEchoConfig ) );
		return 0;
	}else if( strcmp( lastname, "EchoPlusEnabled" )==0 )
	{
		if(vInt)
		{
			vChar = (*vInt)?1:0;
			mib_set( TR143_UDPECHO_PLUS, (void *)&vChar );
		}

		apply_add( CWMP_PRI_N, apply_UDPEchoConfig, CWMP_RESTART, 0, &olddata, sizeof(struct TR143_UDPEchoConfig ) );
		return 0;
	}else{
		return ERR_9005;
	}

	return 0;
}

/*****************************************************************************************************/
int clearWanUDPEchoItf( void )
{
	int total,i;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->TR143UDPEchoItf)
		{
			pEntry->TR143UDPEchoItf=0;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
		}
	}
	return 0;
}

int setWanUDPEchoItf( unsigned int ifindex )
{
	int total,i;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		if(pEntry->ifIndex==ifindex)
		{
			pEntry->TR143UDPEchoItf=1;
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
			return 0;
		}
	}
	return -1;
}

int getWanUDPEchoItf( unsigned int *pifindex)
{
	int total,i;
	MIB_CE_ATM_VC_T *pEntry, vc_entity;

	if(pifindex==NULL) return -1;
	*pifindex=DUMMY_IFINDEX;

	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;
		if(pEntry->TR143UDPEchoItf)
		{
			*pifindex=pEntry->ifIndex;
			return 0;
		}
	}
	return -1;
}

#ifdef CONFIG_USER_FTP_FTP_FTP
#define FTPDIAG_DOWNLOADFILE		"/tmp/ftpdiagdown.txt"
#define FTPDIAG_DOWNLOADFILE_RESULT	"/tmp/ftpdiagdown.txt.result"
#define FTPDIAG_UPLOADFILE		"/tmp/ftpdiagup.txt"
#define FTPDIAG_UPLOADFILE_RESULT	"/tmp/ftpdiagup.txt.result"
static int getFtpDiagResut( char *filename, struct TR143_Diagnostics *p )
{
	FILE *fd;
	int	ret;
	int	DiagnosticsState;
	unsigned int	TestBytesReceived;
	unsigned int	TotalBytesReceived;
	unsigned int	TotalBytesSent;
	struct timeval	ROMTime;
	struct timeval	BOMTime;
	struct timeval	EOMTime;
	struct timeval	TCPOpenRequestTime;
	struct timeval	TCPOpenResponseTime;

	if( filename==NULL || p==NULL ) return -1;
	fd=fopen( filename, "r" );
	if(fd==NULL) goto resulterror ;

	ret=fscanf( fd, "%d %u %u %u %u.%u %u.%u %u.%u %u.%u %u.%u",
			&DiagnosticsState,
			&TestBytesReceived,
			&TotalBytesReceived,
			&TotalBytesSent,
			&ROMTime.tv_sec,&ROMTime.tv_usec,
			&BOMTime.tv_sec,&BOMTime.tv_usec,
			&EOMTime.tv_sec,&EOMTime.tv_usec,
			&TCPOpenRequestTime.tv_sec,&TCPOpenRequestTime.tv_usec,
			&TCPOpenResponseTime.tv_sec,&TCPOpenResponseTime.tv_usec );
	fclose(fd);
	if(ret!=14)  goto resulterror ;
	if(DiagnosticsState<=eTR143_None || DiagnosticsState>=eTR143_End) goto resulterror;

	p->DiagnosticsState=DiagnosticsState;
	p->TestBytesReceived=TestBytesReceived;
	p->TotalBytesReceived=TotalBytesReceived;
	p->TotalBytesSent=TotalBytesSent;
	p->ROMTime.tv_sec=ROMTime.tv_sec;
	p->ROMTime.tv_usec=ROMTime.tv_usec;
	p->BOMTime.tv_sec=BOMTime.tv_sec;
	p->BOMTime.tv_usec=BOMTime.tv_usec;
	p->EOMTime.tv_sec=EOMTime.tv_sec;
	p->EOMTime.tv_usec=EOMTime.tv_usec;
	p->TCPOpenRequestTime.tv_sec=TCPOpenRequestTime.tv_sec;
	p->TCPOpenRequestTime.tv_usec=TCPOpenRequestTime.tv_usec;
	p->TCPOpenResponseTime.tv_sec=TCPOpenResponseTime.tv_sec;
	p->TCPOpenResponseTime.tv_usec=TCPOpenResponseTime.tv_usec;
	return 0;

resulterror:
	p->DiagnosticsState=eTR143_Error_InitConnectionFailed;
	return -1;
}

void checkPidforFTPDiag( pid_t  pid )
{
	int st;

	if(gDownloadDiagnostics.ftp_pid > 0)
		pid = waitpid(gDownloadDiagnostics.ftp_pid, &st, WNOHANG);
	else if(gUploadDiagnostics.ftp_pid > 0)
		pid = waitpid(gUploadDiagnostics.ftp_pid, &st, WNOHANG);

	//if(pid!=-1)
	if(pid>0)
	{
		if(pid==gDownloadDiagnostics.ftp_pid)
		{
			gDownloadDiagnostics.ftp_pid=0;
			getFtpDiagResut( FTPDIAG_DOWNLOADFILE_RESULT, &gDownloadDiagnostics );
			cwmpDiagnosticDone();
		}

		if(pid==gUploadDiagnostics.ftp_pid)
		{
			gUploadDiagnostics.ftp_pid=0;
			getFtpDiagResut( FTPDIAG_UPLOADFILE_RESULT, &gUploadDiagnostics );
			cwmpDiagnosticDone();
		}
	}
}

static int ExecFTPbyScript( char *scriptname, struct TR143_Diagnostics *p)
{
	pid_t pid;

	if( scriptname==NULL || p==NULL ) return -1;
	pid = vfork();
	if(pid==0)
	{
		/* the child */
		char *env[2];
		char *argv[16];
		char *filename="/bin/ftp";
		int i=0;
		char strDSCP[32],strFileLen[32];

		sprintf( strDSCP, "%u", p->DSCP );
		sprintf( strFileLen, "%u", p->TestFileLength );

		signal(SIGINT, SIG_IGN);
		argv[i++] = filename;
		argv[i++] = "-inv";
		argv[i++] = "-TR143TestMode";
		argv[i++] = "-f";
		argv[i++] = scriptname;
		argv[i++] = "-DSCP";
		argv[i++] = strDSCP;
		if( p->IfName && strlen(p->IfName)>0 )
		{
			argv[i++] = "-Interface";
			argv[i++] = p->IfName;
		}
		if(p->Way==DLWAY_UP)
		{
			argv[i++] = "-TestFileLength";
			argv[i++] = strFileLen;
		}
		argv[i++] = NULL;


		env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		env[1] = NULL;

		execve(filename, argv, env);
		perror( "execve" );
		printf("exec %s failed\n", filename);
		_exit(2);
	}else if(pid < 0)
	{
		perror( "vfork" );
		return -1;
	}

	p->ftp_pid=pid;
	return pid;
}

static int TR143StartFtpDiag(struct TR143_Diagnostics *p)
{
	char ftpdiagname[32]="";
	FILE *fd;
	int pid;
	char *pUrl=NULL, *pHost, *pPort, *pPath, *pFile, *pTmp;

	{//parser url
		if(p->pURL==NULL) goto ftperror;
		pHost=NULL; pPort=NULL; pPath=NULL; pFile=NULL;
		pUrl=strdup( p->pURL );
		if(pUrl==NULL) goto ftperror;
		if(strncmp(pUrl, "ftp://", 6)!=0) goto ftperror;
		pHost=pUrl+6;
		pPath=strchr( pHost, '/');
		if(pPath==NULL) goto ftperror;
		pPath[0]=0;
		pPath++;
		pFile=strrchr( pPath, '/');
		if(pFile==NULL)
		{
			pFile=pPath;
			pPath=NULL;
		}else{
			pFile[0]=0;
			pFile++;
		}

		pTmp=strchr( pHost, '@');
		if(pTmp)
		{
			pTmp[0]=0;
			pTmp++;
			//skip user&pwd???(tr143 uses anonymous to login)
			pHost=pTmp;
		}

		if(pHost[0] == '[')
		{
			char *tmp;

			//ipv6 address with port
			pPort = strrchr(pHost, ':');

			pHost++;			//skip '['
			tmp = strchr(pHost, ']');
			if(tmp == NULL)
				goto ftperror;
			tmp[0] = '\0';	//replace ']' with NULL

			if(tmp >= pPort)
				pPort = NULL;

			if(pPort)
			{
				pPort++;
				if( strlen(pPort)==0 ) pPort=NULL;
			}
		}
		else
		{
			pPort = strrchr( pHost, ':');
			if((pPort) && pPort == strchr( pHost, ':'))
			{
				//only one ':', IPv4 with port
				pPort[0]=0;
				pPort++;
				if( strlen(pPort)==0 ) pPort=NULL;
			}
			else
				pPort = NULL;	
		}

		fprintf( stderr, "parser url=%s\n", p->pURL );
		fprintf( stderr, "\t pHost=%s\n", pHost?pHost:"" );
		fprintf( stderr, "\t pPort=%s\n", pPort?pPort:"" );
		fprintf( stderr, "\t pPath=%s\n", pPath?pPath:"" );
		fprintf( stderr, "\t pFile=%s\n", pFile?pFile:"" );

		if( pHost==NULL || strlen(pHost)==0 ||
			pFile==NULL || strlen(pFile)==0 )
			goto ftperror;
	}

	{//write script
		if( p->Way==DLWAY_UP )
			strcpy( ftpdiagname, FTPDIAG_UPLOADFILE );
		else
			strcpy( ftpdiagname, FTPDIAG_DOWNLOADFILE );

		fd=fopen( ftpdiagname, "w" );
		if(fd==NULL)
		{
			perror( "fopen" );
			goto ftperror;
		}

		if(pPort)
			fprintf( fd, "open %s %s\n", pHost, pPort );
		else
			fprintf( fd, "open %s\n", pHost);
		fprintf( fd, "user anonymous dummypwd@\n" );
		fprintf( fd, "binary\n" );
		fprintf( fd, "passive\n" );
		if( p->Way==DLWAY_UP )
		{
			if(pPath) fprintf( fd, "cd %s/\n", pPath );
			fprintf( fd, "put /bin/ftp %s\n", pFile );
		}else{
			if(pPath)
			{
				fprintf( fd, "size %s/%s\n", pPath, pFile );
				fprintf( fd, "get %s/%s /dev/null\n", pPath, pFile );
			}else{
				fprintf( fd, "size %s\n", pFile );
				fprintf( fd, "get %s /dev/null\n", pFile );
			}
		}
		fprintf( fd, "quit\n" );
		fclose(fd);
	}

	pid=ExecFTPbyScript( ftpdiagname, p );
	if(pid==-1) goto ftperror;
	if(pUrl) free(pUrl);
	return 0;

ftperror:
	p->DiagnosticsState=eTR143_Error_InitConnectionFailed;
	if(pUrl) free(pUrl);
	cwmpDiagnosticDone();
	return -1;
}

static int TR143StopFtpDiag(struct TR143_Diagnostics *p)
{
	pid_t pid;

	if(!p) return -1;
	pid=p->ftp_pid;
	p->ftp_pid=0;
	if(pid) kill(pid, SIGTERM);
	return 0;
}
#endif //CONFIG_USER_FTP_FTP_FTP

static void ResetTR143ResultValues(struct TR143_Diagnostics *p)
{
	if(p)
	{
		p->TestBytesReceived=0;
		p->TotalBytesReceived=0;
		p->TotalBytesSent=0;
		memset( &p->ROMTime, 0, sizeof(p->ROMTime) );
		memset( &p->BOMTime, 0, sizeof(p->BOMTime) );
		memset( &p->EOMTime, 0, sizeof(p->EOMTime) );
		memset( &p->TCPOpenRequestTime, 0, sizeof(p->TCPOpenRequestTime) );
		memset( &p->TCPOpenResponseTime, 0, sizeof(p->TCPOpenResponseTime) );
		p->http_pid=0;
		p->ftp_pid=0;

		if( p->pInterface && p->pInterface[0] && (transfer2IfName( p->pInterface, p->IfName )==0) )
		{
			LANDEVNAME2BR0(p->IfName);
		}else
			p->IfName[0]=0;
	}
}

void StopTR143DownloadDiag(void)
{
	TR143StopHttpDiag( &gDownloadDiagnostics );
#ifdef CONFIG_USER_FTP_FTP_FTP
	TR143StopFtpDiag( &gDownloadDiagnostics );
	unlink(FTPDIAG_DOWNLOADFILE);
	unlink(FTPDIAG_DOWNLOADFILE_RESULT);
#endif //CONFIG_USER_FTP_FTP_FTP
}

void StartTR143DownloadDiag(void)
{
	StopTR143DownloadDiag();
	ResetTR143ResultValues(&gDownloadDiagnostics);
#ifdef CONFIG_USER_FTP_FTP_FTP
	if( (gDownloadDiagnostics.pURL!=NULL)
		&& (strncmp(gDownloadDiagnostics.pURL, "ftp://", 6)==0) )
		TR143StartFtpDiag( &gDownloadDiagnostics );
	else
#endif //CONFIG_USER_FTP_FTP_FTP
		TR143StartHttpDiag( &gDownloadDiagnostics );
}

void StopTR143UploadDiag(void)
{
	TR143StopHttpDiag( &gUploadDiagnostics );
#ifdef CONFIG_USER_FTP_FTP_FTP
	TR143StopFtpDiag( &gUploadDiagnostics );
	unlink(FTPDIAG_UPLOADFILE);
	unlink(FTPDIAG_UPLOADFILE_RESULT);
#endif //CONFIG_USER_FTP_FTP_FTP
}

void StartTR143UploadDiag(void)
{
	StopTR143UploadDiag();
	ResetTR143ResultValues(&gUploadDiagnostics);
#ifdef CONFIG_USER_FTP_FTP_FTP
	if( (gUploadDiagnostics.pURL!=NULL)
		&& (strncmp(gUploadDiagnostics.pURL, "ftp://", 6)==0) )
		TR143StartFtpDiag( &gUploadDiagnostics );
	else
#endif //CONFIG_USER_FTP_FTP_FTP
		TR143StartHttpDiag( &gUploadDiagnostics );
}

#endif /*_PRMT_TR143_*/

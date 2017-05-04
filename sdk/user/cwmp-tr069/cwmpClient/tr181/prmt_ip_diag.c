#include <stdio.h>
#include <stdlib.h>

#include <parameter_api.h>
#include <rtk/options.h>
#include <rtk/utility.h>

#include "../cwmpc_utility.h"
#include "prmt_ip_diag.h"
#include "prmt_ip_if.h"

//Reuse Tr-098 definitions
#include "../prmt_tr143.h"
#include "../prmt_ippingdiag.h"
#include "../prmt_traceroute.h"

/****** Device.IP.Diagnostics.UDPEcho ****************************************/
struct CWMP_OP tTR181UDPEchoConfigLeafOP = { getTR181UDPEchoConfig, setTR181UDPEchoConfig };
struct CWMP_PRMT tTR181UDPEchoConfigLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Interface",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tTR181UDPEchoConfigLeafOP},
};

enum eTR181UDPEchoConfigLeaf
{
	eTR181UEC_Interface,
};

struct CWMP_LEAF tTR181UDPEchoConfigLeaf[] =
{
{ &tUDPEchoConfigLeafInfo[eUEC_Enable] },
{ &tTR181UDPEchoConfigLeafInfo[eTR181UEC_Interface] },
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

/****** Device.IP.Diagnostics.DownloadDiagnostics ****************************/
struct CWMP_OP tTR181DownloadDiagnosticsLeafOP = { getTR181DownloadDiagnostics, setTR181DownloadDiagnostics };
struct CWMP_PRMT tTR181DownloadDiagnosticsLeafInfo[] =
{
/*(name,			type,		flag,					op)*/
{"Interface",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tTR181DownloadDiagnosticsLeafOP},
{"DownloadTransports",	eCWMP_tSTRING,	CWMP_READ,				&tTR181DownloadDiagnosticsLeafOP},
};

enum eTR181DownloadDiagnosticsLeaf
{
	eTR181DD_Interface,
	eTR181DD_DownloadTransports,
};

struct CWMP_LEAF tTR181DownloadDiagnosticsLeaf[] =
{
{ &tDownloadDiagnosticsLeafInfo[eDD_DiagnosticsState] },
{ &tTR181DownloadDiagnosticsLeafInfo[eTR181DD_Interface] },
{ &tDownloadDiagnosticsLeafInfo[eDD_DownloadURL] },
{ &tTR181DownloadDiagnosticsLeafInfo[eTR181DD_DownloadTransports] },	
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

/****** Device.IP.Diagnostics.UploadDiagnostics ******************************/
struct CWMP_OP tTR181UploadDiagnosticsLeafOP = { getTR181UploadDiagnostics, setTR181UploadDiagnostics };
struct CWMP_PRMT tTR181UploadDiagnosticsLeafInfo[] =
{
/*(name,			type,		flag,					op)*/
{"Interface",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tTR181UploadDiagnosticsLeafOP},
{"UploadTransports",	eCWMP_tSTRING,	CWMP_READ,				&tTR181UploadDiagnosticsLeafOP},
};

enum eTR181UploadDiagnosticsLeaf
{
	eTR181UD_Interface,
	eTR181UD_UploadTransports,
};

struct CWMP_LEAF tTR181UploadDiagnosticsLeaf[] =
{
{ &tUploadDiagnosticsLeafInfo[eUD_DiagnosticsState] },
{ &tTR181UploadDiagnosticsLeafInfo[eTR181UD_Interface] },
{ &tUploadDiagnosticsLeafInfo[eUD_UploadURL] },
{ &tTR181UploadDiagnosticsLeafInfo[eTR181UD_UploadTransports] },
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

/****** Device.IP.Diagnostics ************************************************/
struct CWMP_PRMT tIPDiagObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"IPPing",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
{"TraceRoute",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
{"DownloadDiagnostics",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"UploadDiagnostics", 	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"UDPEchoConfig",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eIPDiagObject
{
	eIPPing,
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
	eTraceRoute,
#endif
	eDownloadDiagnostics,
	eUploadDiagnostics,
	eUDPEchoConfig,
};

struct CWMP_NODE tIPDiagObject[] =
{
/*info,  				leaf,			next)*/
{&tIPDiagObjectInfo[eIPPing],	tIPPingDiagnosticsLeaf, NULL},
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
{&tIPDiagObjectInfo[eTraceRoute],	tTraceRouteDiagLeaf, tTraceRouteDiagObject},
#endif
{&tIPDiagObjectInfo[eDownloadDiagnostics],	tTR181DownloadDiagnosticsLeaf, NULL},
{&tIPDiagObjectInfo[eUploadDiagnostics],	tTR181UploadDiagnosticsLeaf, NULL},
{&tIPDiagObjectInfo[eUDPEchoConfig],		tTR181UDPEchoConfigLeaf, NULL},
{NULL,					NULL,			NULL}
};

/******* Operations **********************************************************/
int getTR181DownloadDiagnostics(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Interface" )==0 )
	{
		if( gDownloadDiagnostics.pInterface )
			*data = strdup( gDownloadDiagnostics.pInterface );
		else
			*data = strdup( "" );
	}
	else if( strcmp( lastname, "DownloadTransports" )==0 )
	{
#ifdef CONFIG_USER_FTP_FTP_FTP
		*data = strdup("HTTP,FTP");
#else
		*data = strdup("HTTP");
#endif
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setTR181DownloadDiagnostics(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int *vUInt=data;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Interface" )==0 )
	{
		char ifname[IFNAMSIZ];
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
		}
		else
		{
			if( get_ip_if_ifname( buf, ifname ) == 0 )
			{
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
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int getTR181UploadDiagnostics(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Interface" )==0 )
	{
		if( gUploadDiagnostics.pInterface )
			*data = strdup( gUploadDiagnostics.pInterface );
		else
			*data = strdup( "" );
	}
	else if( strcmp( lastname, "UploadTransports" )==0 )
	{
#ifdef CONFIG_USER_FTP_FTP_FTP
		*data = strdup("HTTP,FTP");
#else
		*data = strdup("HTTP");
#endif
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setTR181UploadDiagnostics(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int *vUInt=data;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Interface" )==0 )
	{
		char ifname[IFNAMSIZ];
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
		}
		else
		{
			if( get_ip_if_ifname( buf, ifname ) == 0)
			{
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
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int getTR181UDPEchoConfig(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned char buff[1024]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Interface" )==0 )
	{
		mib_get( TR143_UDPECHO_ITFTYPE, (void *)&vChar );
		if( vChar==ITF_ALL )
		{
			*data = strdup( "" );
		}
		else if( vChar==ITF_WAN )
		{
			unsigned int ifindex;
			if( getWanUDPEchoItf(&ifindex)==0 )
				ifIdex_to_ip_if_path( ifindex, buff );
			*data = strdup( buff );
		}
		else if( vChar<ITF_END )
		{
			ifname_to_ip_if_path(strItf[vChar], buff);
			*data = strdup( buff );
		}
		else
			*data = strdup( "" ); //or return ERR_9002
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setTR181UDPEchoConfig(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char		*lastname = entity->info->name;
	char		*buf=data;
	unsigned char	vChar;
	struct TR143_UDPEchoConfig olddata;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	UDPEchoConfigSave( &olddata );

	if( strcmp( lastname, "Interface" )==0 )
	{
		char ifname[32];
		if(buf)
		{
			if( get_ip_if_ifname(buf, ifname) == 0 )
			{
				int	ifid;

				ifid = IfName2ItfId( ifname );
				if(ifid == -1)
					return ERR_9007;

				if(ifid == ITF_WAN)
				{
					unsigned int wanifindex;
					wanifindex = ip_if_path_to_ifIndex(buf);
					if(wanifindex!=DUMMY_IFINDEX)
					{
						clearWanUDPEchoItf();
						setWanUDPEchoItf( wanifindex );
					}
					else
						return ERR_9007;
				}

				vChar=(unsigned char)ifid;
				mib_set( TR143_UDPECHO_ITFTYPE, (void *)&vChar );
			}
			else
				return ERR_9007;
		}
		apply_add( CWMP_PRI_N, apply_UDPEchoConfig, CWMP_RESTART, 0, &olddata, sizeof(struct TR143_UDPEchoConfig ) );
		return 0;
	}
	else{
		return ERR_9005;
	}

	return 0;
}


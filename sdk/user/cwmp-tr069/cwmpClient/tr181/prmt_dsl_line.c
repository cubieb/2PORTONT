#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>

#include <rtk/adsl_drv.h>
#include <rtk/adslif.h>
#include <rtk/utility.h>
#include <parameter_api.h>

#include "../cwmpc_utility.h"
#include "prmt_dsl_line.h"

#include "../prmt_wandevice.h"
#include "../prmt_wandsldiagnostics.h"


/****** Device.DSL.Line.1.Stat.Total *****************************************/
struct CWMP_OP tDSLLineStatsShowtimeLeafOP = { getDSLLineStatsShowtime, NULL};

struct CWMP_PRMT tDSLLineStatsShowTimeLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ErroredSecs",	eCWMP_tUINT, CWMP_READ,		&tDSLLineStatsShowtimeLeafOP},
{"SeverelyErroredSecs",	eCWMP_tUINT, CWMP_READ,		&tDSLLineStatsShowtimeLeafOP},
};

enum eDSLLineStatsShowtimeLeaf
{
	eDSLineStatsShowtimeErroredSecs,
	eDSLineStatsShowtimeSeverelyErroredSecs,
};

struct CWMP_LEAF tDSLLineStatsShowtimeLeaf[] =
{
{ &tDSLLineStatsShowTimeLeafInfo[eDSLineStatsShowtimeErroredSecs]  },
{ &tDSLLineStatsShowTimeLeafInfo[eDSLineStatsShowtimeSeverelyErroredSecs]  },
{ NULL	}
};

/****** Device.DSL.Line.1.Stat.Total *****************************************/
struct CWMP_OP tDSLLineStatsTotalLeafOP = { getDSLLineStatsTotal, NULL};

struct CWMP_PRMT tDSLLineStatsTotalLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ErroredSecs",			eCWMP_tUINT, CWMP_READ|CWMP_DENY_ACT,	&tDSLLineStatsTotalLeafOP},
{"SeverelyErroredSecs",	eCWMP_tUINT, CWMP_READ|CWMP_DENY_ACT,	&tDSLLineStatsTotalLeafOP},
};

enum eDSLLineStatsTotalLeaf
{
	eDSLineStatsTotalErroredSecs,
	eDSLineStatsTotalSeverelyErroredSecs,
};

struct CWMP_LEAF tDSLLineStatsTotalLeaf[] =
{
{ &tDSLLineStatsTotalLeafInfo[eDSLineStatsTotalErroredSecs]  },
{ &tDSLLineStatsTotalLeafInfo[eDSLineStatsTotalSeverelyErroredSecs]  },
{ NULL	}
};

/****** Device.DSL.Line.1.Stat ***********************************************/
struct CWMP_OP tDSLLineStatsLeafOP = { getDSLLineStats, NULL};

struct CWMP_PRMT tDSLLineStatsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"BytesSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tDSLLineStatsLeafOP},
{"BytesReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tDSLLineStatsLeafOP},
{"PacketsSent",		eCWMP_tULONG, 	CWMP_READ|CWMP_DENY_ACT, 	&tDSLLineStatsLeafOP},
{"PacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tDSLLineStatsLeafOP},
{"ErrorsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT, 	&tDSLLineStatsLeafOP},
{"ErrorsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT, 	&tDSLLineStatsLeafOP},
{"TotalStart",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tDSLLineStatsLeafOP},
{"ShowtimeStart",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tDSLLineStatsLeafOP},
/* LastShowtimeStart */
/* CurrentDayStart */
/* QuarterHourStart */
};

enum eDSLLineStatsLeaf
{
	eDSLLineStatsBytesSent,
	eDSLLineStatsBytesReceived,
	eDSLLineStatsPacketsSent,
	eDSLLineStatsPacketsReceived,
	eDSLLineStatsErrorsSent,
	eDSLLineStatsErrorsReceived,
	eDSLLineStatsTotalStart,
	eDSLLineStatsShowtimeStart,
};

struct CWMP_LEAF tDSLLineStatsLeaf[] =
{
{ &tDSLLineStatsLeafInfo[eDSLLineStatsBytesSent]  },
{ &tDSLLineStatsLeafInfo[eDSLLineStatsBytesReceived]  },
{ &tDSLLineStatsLeafInfo[eDSLLineStatsPacketsSent]  },
{ &tDSLLineStatsLeafInfo[eDSLLineStatsPacketsReceived]  },
{ &tDSLLineStatsLeafInfo[eDSLLineStatsErrorsSent]  },
{ &tDSLLineStatsLeafInfo[eDSLLineStatsErrorsReceived]  },
{ &tDSLLineStatsLeafInfo[eDSLLineStatsTotalStart]  },
{ &tDSLLineStatsLeafInfo[eDSLLineStatsShowtimeStart]  },
{ NULL	}
};

struct CWMP_PRMT tDSLLineStatsObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Total",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Showtime",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eDSLLineStatsObject
{
	eDSLLineStatsTotal,
	eDSLLineStatsShowtime,
};

struct CWMP_NODE tDSLLineStatsObject[] =
{
/*info,  				leaf,			next)*/
{&tDSLLineStatsObjectInfo[eDSLLineStatsTotal],		tDSLLineStatsTotalLeaf,	NULL},
{&tDSLLineStatsObjectInfo[eDSLLineStatsShowtime],	tDSLLineStatsShowtimeLeaf, NULL},
{NULL,					NULL,			NULL}
};

/****** Device.DSL.Line.1.TestParams. ****************************************/
// defined in "../prmt_wandevice.c"
extern struct CWMP_PRMT tWANDSLIFTestParamsLeafInfo[];
struct  CWMP_LEAF tDSLLineTestParamsLeaf[] =
{
#ifdef CONFIG_VDSL
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGGds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGGus] },
#endif /*CONFIG_VDSL*/
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGpsds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGpsus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGMTds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_HLOGMTus] },
#ifdef CONFIG_VDSL
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNGds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNGus] },
#endif /*CONFIG_VDSL*/
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNpsds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNpsus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNMTds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_QLNMTus] },
#ifdef CONFIG_VDSL
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRGds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRGus] },
#endif /*CONFIG_VDSL*/
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRpsds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRpsus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRMTds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SNRMTus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_LATNds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_LATNus] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SATNds] },
{ &tWANDSLIFTestParamsLeafInfo[eTP_SATNus] },
{ NULL }
};

/****** Device.DSL.Line.1 ****************************************************/
struct CWMP_OP tDSLLineEntityLeafOP = { getDSLLineEntity, setDSLLineEntity};

struct CWMP_PRMT tDSLLineEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,	&tDSLLineEntityLeafOP},
//{"Alias",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tDSLLineEntityLeafOP},
{"Name",		eCWMP_tSTRING,	CWMP_READ, 	&tDSLLineEntityLeafOP},
{"LastChange",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tDSLLineEntityLeafOP},
{"LowerLayers",	eCWMP_tSTRING,	CWMP_READ, 	&tDSLLineEntityLeafOP},
{"Upstream",	eCWMP_tBOOLEAN,	CWMP_READ, 	&tDSLLineEntityLeafOP},
{"FirmwareVersion",		eCWMP_tSTRING,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"LinkStatus",			eCWMP_tSTRING,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"StandardsSupported",	eCWMP_tSTRING,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"StandardUsed",		eCWMP_tSTRING,	CWMP_READ,	&tDSLLineEntityLeafOP},
#ifdef CONFIG_VDSL
{"AllowedProfiles",		eCWMP_tSTRING,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"CurrentProfile",		eCWMP_tSTRING,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"UPBOKLE",				eCWMP_tUINT,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"TRELLISds",		eCWMP_tINT, 	CWMP_READ,	&tDSLLineEntityLeafOP},
{"TRELLISus",		eCWMP_tINT, 	CWMP_READ,	&tDSLLineEntityLeafOP},
{"ACTSNRMODEds",	eCWMP_tUINT,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"ACTSNRMODEus",	eCWMP_tUINT,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"ACTUALCE",		eCWMP_tUINT,	CWMP_READ,	&tDSLLineEntityLeafOP},
#endif
{"LineEncoding",			eCWMP_tSTRING,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"UpstreamMaxBitRate",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tDSLLineEntityLeafOP},
{"DownstreamMaxBitRate",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tDSLLineEntityLeafOP},
{"UpstreamNoiseMargin",		eCWMP_tINT,		CWMP_READ|CWMP_DENY_ACT, 	&tDSLLineEntityLeafOP},
{"DownstreamNoiseMargin",	eCWMP_tINT,		CWMP_READ|CWMP_DENY_ACT, 	&tDSLLineEntityLeafOP},
#ifdef CONFIG_VDSL
{"SNRMpbds",		eCWMP_tSTRING,	CWMP_READ,	&tDSLLineEntityLeafOP},
{"SNRMpbus",		eCWMP_tSTRING,	CWMP_READ,	&tDSLLineEntityLeafOP},
#endif
{"UpstreamAttenuation",		eCWMP_tINT, 	CWMP_READ|CWMP_DENY_ACT, 	&tDSLLineEntityLeafOP},
{"DownstreamAttenuation",	eCWMP_tINT, 	CWMP_READ|CWMP_DENY_ACT, 	&tDSLLineEntityLeafOP},
{"UpstreamPower",			eCWMP_tINT, 	CWMP_READ|CWMP_DENY_ACT, 	&tDSLLineEntityLeafOP},
{"DownstreamPower",			eCWMP_tINT, 	CWMP_READ|CWMP_DENY_ACT, 	&tDSLLineEntityLeafOP},
{"XTURVendor",	eCWMP_tHEXBIN, CWMP_READ, 	&tDSLLineEntityLeafOP},
{"XTURCountry",	eCWMP_tHEXBIN, CWMP_READ, 	&tDSLLineEntityLeafOP},
{"XTUCVendor",	eCWMP_tHEXBIN, CWMP_READ, 	&tDSLLineEntityLeafOP},
{"XTUCCountry",	eCWMP_tHEXBIN, CWMP_READ, 	&tDSLLineEntityLeafOP},
};
enum eDSLLineEntityLeaf
{
	eEnable,
	eStatus,
	//eAlias,
	eName,
	eLastChange,
	eLowerLayers,
	eUpstream,
	eFirmwareVersion,
	eLinkStatus,
	eStandardsSupported,
	eStandardUsed,
#ifdef CONFIG_VDSL
	eAllowedProfiles, 
	eCurrentProfile,
	eUPBOKLE,
	eTRELLISds,
	eTRELLISus,
	eACTSNRMODEds,
	eACTSNRMODEus,
	eACTUALCE,
#endif
	eLineEncoding,
	eUpstreamMaxBitRate,
	eDownstreamMaxBitRate,
	eUpstreamNoiseMargin,
	eDownstreamNoiseMargin,
#ifdef CONFIG_VDSL
	eSNRMpbds,
	eSNRMpbus,
#endif
	eUpstreamAttenuation,
	eDownstreamAttenuation,
	eUpstreamPower,
	eDownstreamPower,
	eXTURVendor,
	eXTURCountry,
	eXTUCVendor,
	eXTUCCountry,
};

struct CWMP_LEAF tDSLLineEntityLeaf[] =
{
{ &tDSLLineEntityLeafInfo[eEnable]  },
{ &tDSLLineEntityLeafInfo[eStatus]  },
//{ &tDSLLineEntityLeafInfo[eAlias]  },
{ &tDSLLineEntityLeafInfo[eName]  },
{ &tDSLLineEntityLeafInfo[eLastChange]  },
{ &tDSLLineEntityLeafInfo[eLowerLayers]  },
{ &tDSLLineEntityLeafInfo[eUpstream]  },
{ &tDSLLineEntityLeafInfo[eFirmwareVersion]	},
{ &tDSLLineEntityLeafInfo[eLinkStatus]  },
{ &tDSLLineEntityLeafInfo[eStandardsSupported]	},
{ &tDSLLineEntityLeafInfo[eStandardUsed]  },
#ifdef CONFIG_VDSL
{ &tDSLLineEntityLeafInfo[eAllowedProfiles]	},
{ &tDSLLineEntityLeafInfo[eCurrentProfile]	},
{ &tDSLLineEntityLeafInfo[eUPBOKLE]  },
{ &tDSLLineEntityLeafInfo[eTRELLISds]  },
{ &tDSLLineEntityLeafInfo[eTRELLISus]	},
{ &tDSLLineEntityLeafInfo[eACTSNRMODEds]  },
{ &tDSLLineEntityLeafInfo[eACTSNRMODEus]  },
{ &tDSLLineEntityLeafInfo[eACTUALCE]  },
#endif
{ &tDSLLineEntityLeafInfo[eLineEncoding]  },
{ &tDSLLineEntityLeafInfo[eUpstreamMaxBitRate]  },
{ &tDSLLineEntityLeafInfo[eDownstreamMaxBitRate]  },
{ &tDSLLineEntityLeafInfo[eUpstreamNoiseMargin]  },
{ &tDSLLineEntityLeafInfo[eDownstreamNoiseMargin]  },
#ifdef CONFIG_VDSL
{ &tDSLLineEntityLeafInfo[eSNRMpbds]  },
{ &tDSLLineEntityLeafInfo[eSNRMpbus]  },
#endif
{ &tDSLLineEntityLeafInfo[eUpstreamAttenuation]  },
{ &tDSLLineEntityLeafInfo[eDownstreamAttenuation]  },
{ &tDSLLineEntityLeafInfo[eUpstreamPower]  },
{ &tDSLLineEntityLeafInfo[eDownstreamPower]  },
{ &tDSLLineEntityLeafInfo[eXTURVendor]  },
{ &tDSLLineEntityLeafInfo[eXTURCountry]  },
{ &tDSLLineEntityLeafInfo[eXTUCVendor]  },
{ &tDSLLineEntityLeafInfo[eXTUCCountry]  },
{ NULL	}
};

struct CWMP_PRMT tDSLLineEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"TestParams",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eDSLLineEntityObject
{
	eDSLLineStats,
	eDSLLineTestParams,
};

struct CWMP_NODE tDSLLineEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tDSLLineEntityObjectInfo[eDSLLineStats],		tDSLLineStatsLeaf,	tDSLLineStatsObject},
{&tDSLLineEntityObjectInfo[eDSLLineTestParams],	tDSLLineTestParamsLeaf,	NULL},
{NULL,					NULL,			NULL}
};
/****** Device.DSL.Line ******************************************************/
struct CWMP_PRMT tDSLLineObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"1",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eDSLLineObject
{
	eDSLLINE1,
};

struct CWMP_NODE tDSLLineObject[] =
{
/*info,  				leaf,			next)*/
{&tDSLLineObjectInfo[eDSLLINE1],		tDSLLineEntityLeaf,	tDSLLineEntityObject},
{NULL,					NULL,			NULL}
};

/***** Operations ************************************************************/
int getDSLLineStatsTotal(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	Modem_DSLConfigStatus MDS;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if(adsl_drv_get(RLCM_GET_DSL_STAT_TOTAL, (void *)&MDS, TR069_STAT_SIZE)==0)
	{
#if 0
		return ERR_9002;
#else
		memset( &MDS, 0, sizeof(MDS) );
#endif
	}
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ErroredSecs" )==0 )
	{
		*data = uintdup(MDS.ES);
	}
	else if( strcmp( lastname, "SeverelyErroredSecs" )==0 )
	{
		*data = uintdup(MDS.ES);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int getDSLLineStatsShowtime(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	Modem_DSLConfigStatus MDS;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if(adsl_drv_get(RLCM_GET_DSL_STAT_SHOWTIME, (void *)&MDS, TR069_STAT_SIZE)==0)
	{
#if 0
		return ERR_9002;
#else
		memset( &MDS, 0, sizeof(MDS) );
#endif
	}
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ErroredSecs" )==0 )
	{
		*data = uintdup(MDS.ES);
	}
	else if( strcmp( lastname, "SeverelyErroredSecs" )==0 )
	{
		*data = uintdup(MDS.ES);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int getDSLLineStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	struct net_device_stats nds;
	unsigned long bs=0,br=0,ps=0,pr=0;
	XDSL_OP *op = xdsl_get_op(0);
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	memset( &nds, 0, sizeof(nds) );
	if(get_DSLWANStat( &nds )!=0)
		fprintf(stderr, "<%s:%d> Get DSL statistics error!\n");

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "BytesSent" )==0 )
	{
		*data = (void *)ulongdup(nds.tx_bytes);
	}
	else if( strcmp( lastname, "BytesReceived" )==0 )
	{
		*data = (void *)ulongdup(nds.rx_bytes);
	}
	else if( strcmp( lastname, "PacketsSent" )==0 )
	{
		*data = (void *)ulongdup(nds.tx_packets);
	}
	else if( strcmp( lastname, "PacketsReceived" )==0 )
	{
		*data = (void *)ulongdup(nds.rx_packets);
	}
	else if( strcmp( lastname, "ErrorsSent" )==0 )
	{
		*data = uintdup(nds.tx_errors);
	}
	else if( strcmp( lastname, "ErrorsReceived" )==0 )
	{
		*data = uintdup(nds.rx_errors);
	}
	else if( strcmp( lastname, "TotalStart" )==0 )
	{
		struct sysinfo info;
		sysinfo(&info);
		*data = uintdup( info.uptime );
	}
	else if( strcmp( lastname, "ShowtimeStart" )==0 )
	{
		unsigned int vUint[3];
		if(adsl_drv_get(RLCM_GET_DSL_ORHERS, (void *)vUint, TR069_DSL_OTHER_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vUint[0] );
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int getDSLLineEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char buf[256] = {0};
	double vd=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = uintdup(1);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		getAdslInfo(ADSL_GET_STATE, buf, 256);
		if( strncmp( buf, "HANDSHAKING", 11 )==0 )
			*data = strdup( "Dormant" );
		else if( strncmp( buf, "SHOWTIME", 8 )==0 )
			*data = strdup( "Up" );
		else if( (strncmp( buf, "ACTIVATING", 10 )==0) ||
		         (strncmp( buf, "IDLE", 4 )==0) )
			*data = strdup( "Dormant" );
		else
			*data = strdup( "DOWN" );//or Error, Disabled
	}
	//TODO
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		*data = uintdup(1);
	}*/
	//TODO
	else if( strcmp( lastname, "Name" )==0 )
	{
		*data = strdup("vc0");	// Not sure
	}
	//TODO
	else if( strcmp( lastname, "LastChange" )==0 )
	{
		*data = uintdup(1);
	}
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		*data = strdup("");	//no lower layers
	}
	else if( strcmp( lastname, "Upstream" )==0 )
	{
		*data = booldup(1);	//WAN device
	}
	else if( strcmp( lastname, "FirmwareVersion" )==0 )
	{
		getAdslInfo( ADSL_GET_VERSION, buf, 256 );
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "LinkStatus" )==0 )
	{
		getAdslInfo(ADSL_GET_STATE, buf, 256);
		if( strncmp( buf, "HANDSHAKING", 11 )==0 )
			*data = strdup( "Initializing" );
		else if( strncmp( buf, "SHOWTIME", 8 )==0 )
			*data = strdup( "Up" );
		else if( (strncmp( buf, "ACTIVATING", 10 )==0) || (strncmp( buf, "IDLE", 4 )==0) )
			*data = strdup( "Down" );
		else
			*data = strdup( "Unavailable" );
	}
	else if( strcmp( lastname, "StandardsSupported" )==0 )
	{
		Modem_ADSL2WANConfig vMA2WC;

		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			int std_idx;
			//fprintf( stderr, "StandardsSupported=0x%x\n", vMA2WC.StandardsSuported );
			strcpy( buf, "" );
			std_idx=0;
			while( strAdsl2WanStd[std_idx] != NULL )
			{
				if( vMA2WC.StandardsSuported & (1<<std_idx) )
				{
					if( (strlen(buf)+strlen(strAdsl2WanStd[std_idx])+1)>=sizeof(buf) )
					{
						fprintf( stderr, "(%s:%d)buf is too small!!\n", __FUNCTION__, __LINE__ );
						break;
					}
					if(buf[0]) strcat( buf, "," );
					strcat( buf, strAdsl2WanStd[std_idx] );
				}
				std_idx++;
			}
			*data = strdup( buf );
		}
	}
	else if( strcmp( lastname, "StandardUsed" )==0 )
	{
		Modem_ADSL2WANConfig vMA2WC;

		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			int std_idx;
			//fprintf( stderr, "StandardUsed=0x%x\n", vMA2WC.StandardUsed );
			strcpy( buf, "" );
			std_idx=0;
			while( strAdsl2WanStd[std_idx] != NULL )
			{
				if( vMA2WC.StandardUsed & (1<<std_idx) )
				{
					strcpy( buf, strAdsl2WanStd[std_idx] );
					break;
				}
				std_idx++;
			}
			*data = strdup( buf );
		}
	}
	else if( strcmp( lastname, "LineEncoding" )==0 )
	{
		*data = strdup( "DMT" );
	}
	else if( strcmp( lastname, "UpstreamMaxBitRate" )==0 )
	{
		unsigned int vUint[3];
		if(adsl_drv_get(RLCM_GET_DSL_ORHERS, (void *)vUint, TR069_DSL_OTHER_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vUint[2] );
	}
	else if( strcmp( lastname, "DownstreamMaxBitRate" )==0 )
	{
		unsigned int vUint[3];
		if(adsl_drv_get(RLCM_GET_DSL_ORHERS, (void *)vUint, TR069_DSL_OTHER_SIZE)==0)
			*data = uintdup( 0 );
		else
			*data = uintdup( vUint[1] );
	}
	else if( strcmp( lastname, "UpstreamNoiseMargin" )==0 )
	{
		getAdslInfo(ADSL_GET_SNR_US, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}
	else if( strcmp( lastname, "DownstreamNoiseMargin" )==0 )
	{
		getAdslInfo(ADSL_GET_SNR_DS, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}
	else if( strcmp( lastname, "UpstreamAttenuation" )==0 )
	{
		double vd=0;

		getAdslInfo(ADSL_GET_LPATT_US, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}
	else if( strcmp( lastname, "DownstreamAttenuation" )==0 )
	{
		getAdslInfo(ADSL_GET_LPATT_DS, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}
	else if( strcmp( lastname, "UpstreamPower" )==0 )
	{
		getAdslInfo(ADSL_GET_POWER_US, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}
	else if( strcmp( lastname, "DownstreamPower" )==0 )
	{
		getAdslInfo(ADSL_GET_POWER_DS, buf, 256);
		vd = atof(buf);
		vd = vd * 10;
		*data = intdup( (int)vd );
	}
	else if( strcmp( lastname, "XTURVendor" )==0 )
	{
		Modem_Identification vMId = {0};
		struct xsd__hexBinary tmp = {0};
		char zeroCode[4] = {0};

		if(adsl_drv_get(RLCM_MODEM_NEAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
		{
			tmp.__ptr = zeroCode;
			tmp.__size = sizeof(zeroCode);
			*data = (void *)hexBinarydup(tmp);
		}
		else
		{
			tmp.__ptr = (char *) &vMId.ITU_VendorId.vendorCode;
			tmp.__size = sizeof(vMId.ITU_VendorId.vendorCode);
			*data = (void *)hexBinarydup(tmp);
		}
	}
	else if( strcmp( lastname, "XTURCountry" )==0 )
	{
		Modem_Identification vMId = {0};
		struct xsd__hexBinary tmp = {0};
		char zeroCode[2] = {0};

		if(adsl_drv_get(RLCM_MODEM_NEAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
		{
			tmp.__ptr = zeroCode;
			tmp.__size = sizeof(zeroCode);
			*data = (void *)hexBinarydup(tmp);
		}
		else
		{
			tmp.__ptr = (char *) &vMId.ITU_VendorId.countryCode;
			tmp.__size = sizeof(vMId.ITU_VendorId.countryCode);
			*data = (void *)hexBinarydup(tmp);
		}
	}
	else if( strcmp( lastname, "XTUCVendor" )==0 )
	{
		Modem_Identification vMId = {0};
		struct xsd__hexBinary tmp = {0};
		char zeroCode[4] = {0};

		if(adsl_drv_get(RLCM_MODEM_FAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
		{
			tmp.__ptr = zeroCode;
			tmp.__size = sizeof(zeroCode);
			*data = (void *)hexBinarydup(tmp);
		}
		else
		{
			tmp.__ptr = (char *) &vMId.ITU_VendorId.vendorCode;
			tmp.__size = sizeof(vMId.ITU_VendorId.vendorCode);
			*data = (void *)hexBinarydup(tmp);
		}
	}
	else if( strcmp( lastname, "XTUCCountry" )==0 )
	{
		Modem_Identification vMId = {0};
		struct xsd__hexBinary tmp = {0};
		char zeroCode[2] = {0};

		if(adsl_drv_get(RLCM_MODEM_FAR_END_ID_REQ, (void *)&vMId, RLCM_MODEM_ID_REQ_SIZE)==0)
		{
			tmp.__ptr = zeroCode;
			tmp.__size = sizeof(zeroCode);
			*data = (void *)hexBinarydup(tmp);
		}
		else
		{
			tmp.__ptr = (char *) &vMId.ITU_VendorId.countryCode;
			tmp.__size = sizeof(vMId.ITU_VendorId.countryCode);
			*data = (void *)hexBinarydup(tmp);
		}
	}
#ifdef CONFIG_VDSL
	else if( strcmp( lastname, "AllowedProfiles" )==0 )
	{
		int msgval[4];

		buf[0]=0;
		if(dsl_msg_get_array(GetHskXdslMode, msgval))
		{
			if( msgval[0]&MODE_VDSL2 )
			{
				int pval=msgval[1];
				if(pval&VDSL2_PROFILE_8A) strcat(buf, "8a,");
				if(pval&VDSL2_PROFILE_8B) strcat(buf, "8b,");
				if(pval&VDSL2_PROFILE_8C) strcat(buf, "8c,");
				if(pval&VDSL2_PROFILE_8D) strcat(buf, "8d,");
				if(pval&VDSL2_PROFILE_12A) strcat(buf, "12a,");
				if(pval&VDSL2_PROFILE_12B) strcat(buf, "12b,");
				if(pval&VDSL2_PROFILE_17A) strcat(buf, "17a,");
				if(pval&VDSL2_PROFILE_30A) strcat(buf, "30a,");
				if(buf[0]) buf[strlen(buf)-1]=0;
			}
		}
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "CurrentProfile" )==0 )
	{
		int mval=0, pval=0;

		buf[0]=0;
		if( dsl_msg_get(GetPmdMode,&mval) &&
			dsl_msg_get(GetVdslProfile,&pval) )
		{
			if(mval&MODE_VDSL2)
			{
				if(pval&VDSL2_PROFILE_8A) strcpy(buf, "8a");
				else if(pval&VDSL2_PROFILE_8B) strcpy(buf, "8b");
				else if(pval&VDSL2_PROFILE_8C) strcpy(buf, "8c");
				else if(pval&VDSL2_PROFILE_8D) strcpy(buf, "8d");
				else if(pval&VDSL2_PROFILE_12A) strcpy(buf, "12a");
				else if(pval&VDSL2_PROFILE_12B) strcpy(buf, "12b");
				else if(pval&VDSL2_PROFILE_17A) strcpy(buf, "17a");
				else if(pval&VDSL2_PROFILE_30A) strcpy(buf, "30a");
			}
		}
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "UPBOKLE" )==0 )
	{
		int msgval[4];

		if(dsl_msg_get_array(GetUPBOKLE, msgval))
		{
			*data = uintdup( msgval[0] );
		}else
			*data = uintdup( 0 );
	}
	else if( strcmp( lastname, "TRELLISds" )==0 )
	{
		int msgval[4];

		if(dsl_msg_get_array(GetTrellis, msgval))
		{
			if(msgval[0]) *data = intdup( 1 );
			else *data = intdup( 0 );
		}else
			*data = intdup( -1 );
	}
	else if( strcmp( lastname, "TRELLISus" )==0 )
	{
		int msgval[4];

		if(dsl_msg_get_array(GetTrellis, msgval))
		{
			if(msgval[1]) *data = intdup( 1 );
			else *data = intdup( 0 );
		}else
			*data = intdup( -1 );
	}
	else if( strcmp( lastname, "ACTSNRMODEds" )==0 )
	{
		int msgval[4];

		if(dsl_msg_get_array(GetACTSNRMODE, msgval))
		{
			*data = uintdup( msgval[0] );
		}else
			*data = uintdup( 0 );
	}
	else if( strcmp( lastname, "ACTSNRMODEus" )==0 )
	{
		int msgval[4];

		if(dsl_msg_get_array(GetACTSNRMODE, msgval))
		{
			*data = uintdup( msgval[1] );
		}else
			*data = uintdup( 0 );
	}
	else if( strcmp( lastname, "ACTUALCE" )==0 )
	{
		int msgval[4];

		if(dsl_msg_get_array(GetACTUALCE, msgval))
		{
			*data = uintdup( msgval[0] );
		}else
			*data = uintdup( 99 );
	}
	else if( strcmp( lastname, "SNRMpbus" )==0 )
	{
		char *pChar;
		if( getDSLParameterValue( GET_DSL_SNRMpbus, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}
	else if( strcmp( lastname, "SNRMpbds" )==0 )
	{
		char *pChar;
		if( getDSLParameterValue( GET_DSL_SNRMpbds, &pChar )<0 )
			*data = strdup( "" );
		else
			*data = strdup( pChar );
	}
#endif /*CONFIG_VDSL*/
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setDSLLineEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (entity==NULL)) return -1;

	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		if( *i!= 1 ) return ERR_9001;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


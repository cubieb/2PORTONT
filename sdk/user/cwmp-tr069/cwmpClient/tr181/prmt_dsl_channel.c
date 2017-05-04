#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>

#include <rtk/adsl_drv.h>
#include <rtk/adslif.h>
#include <rtk/utility.h>
#include <parameter_api.h>

#include "prmt_dsl_line.h"	//Becuase we use same Stats object
#include "prmt_dsl_channel.h"

/****** Device.DSL.Channel.1.Stat.Total **************************************/
struct CWMP_OP tDSLChannelStatsShowtimeLeafOP = { getDSLChannelStatsShowtime, NULL};

struct CWMP_PRMT tDSLChannelStatsShowTimeLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"XTURFECErrors",	eCWMP_tUINT, CWMP_READ,		&tDSLChannelStatsShowtimeLeafOP},
{"XTUCFECErrors",	eCWMP_tUINT, CWMP_READ,		&tDSLChannelStatsShowtimeLeafOP},
{"XTURHECErrors",	eCWMP_tUINT, CWMP_READ,		&tDSLChannelStatsShowtimeLeafOP},
{"XTUCHECErrors",	eCWMP_tUINT, CWMP_READ, 	&tDSLChannelStatsShowtimeLeafOP},
{"XTURCRCErrors",	eCWMP_tUINT, CWMP_READ, 	&tDSLChannelStatsShowtimeLeafOP},
{"XTUCCRCErrors",	eCWMP_tUINT, CWMP_READ, 	&tDSLChannelStatsShowtimeLeafOP},
};

enum eDSLChannelStatsShowtimeLeaf
{
	eDSLChannelStatsShowtimeXTURFECErrors,
	eDSLChannelStatsShowtimeXTUCFECErrors,
	eDSLChannelStatsShowtimeXTURHECErrors,
	eDSLChannelStatsShowtimeXTUCHECErrors,
	eDSLChannelStatsShowtimeXTURCRCErrors,
	eDSLChannelStatsShowtimeXTUCCRCErrors,
};

struct CWMP_LEAF tDSLChannelStatsShowtimeLeaf[] =
{
{ &tDSLChannelStatsShowTimeLeafInfo[eDSLChannelStatsShowtimeXTURFECErrors]  },
{ &tDSLChannelStatsShowTimeLeafInfo[eDSLChannelStatsShowtimeXTUCFECErrors]  },
{ &tDSLChannelStatsShowTimeLeafInfo[eDSLChannelStatsShowtimeXTURHECErrors]  },
{ &tDSLChannelStatsShowTimeLeafInfo[eDSLChannelStatsShowtimeXTUCHECErrors]  },
{ &tDSLChannelStatsShowTimeLeafInfo[eDSLChannelStatsShowtimeXTURCRCErrors]  },
{ &tDSLChannelStatsShowTimeLeafInfo[eDSLChannelStatsShowtimeXTUCCRCErrors]  },
{ NULL	}
};

/****** Device.DSL.Channel.1.Stat.Total **************************************/
struct CWMP_OP tDSLChannelStatsTotalLeafOP = { getDSLChannelStatsTotal, NULL};

struct CWMP_PRMT tDSLChannelStatsTotalLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"XTURFECErrors",	eCWMP_tUINT, CWMP_READ,		&tDSLChannelStatsTotalLeafOP},
{"XTUCFECErrors",	eCWMP_tUINT, CWMP_READ,		&tDSLChannelStatsTotalLeafOP},
{"XTURHECErrors",	eCWMP_tUINT, CWMP_READ,		&tDSLChannelStatsTotalLeafOP},
{"XTUCHECErrors",	eCWMP_tUINT, CWMP_READ,		&tDSLChannelStatsTotalLeafOP},
{"XTURCRCErrors",	eCWMP_tUINT, CWMP_READ,		&tDSLChannelStatsTotalLeafOP},
{"XTUCCRCErrors",	eCWMP_tUINT, CWMP_READ,		&tDSLChannelStatsTotalLeafOP},
};

enum eDSLChannelStatsTotalLeaf
{
	eDSLChannelStatsTotalXTURFECErrors,
	eDSLChannelStatsTotalXTUCFECErrors,
	eDSLChannelStatsTotalXTURHECErrors,
	eDSLChannelStatsTotalXTUCHECErrors,
	eDSLChannelStatsTotalXTURCRCErrors,
	eDSLChannelStatsTotalXTUCCRCErrors,
};

struct CWMP_LEAF tDSLChannelStatsTotalLeaf[] =
{
{ &tDSLChannelStatsTotalLeafInfo[eDSLChannelStatsTotalXTURFECErrors]	},
{ &tDSLChannelStatsTotalLeafInfo[eDSLChannelStatsTotalXTUCFECErrors]	},
{ &tDSLChannelStatsTotalLeafInfo[eDSLChannelStatsTotalXTURHECErrors]	},
{ &tDSLChannelStatsTotalLeafInfo[eDSLChannelStatsTotalXTUCHECErrors]	},
{ &tDSLChannelStatsTotalLeafInfo[eDSLChannelStatsTotalXTURCRCErrors]	},
{ &tDSLChannelStatsTotalLeafInfo[eDSLChannelStatsTotalXTUCCRCErrors]	},
{ NULL	}
};

/****** Device.DSL.Channel.1.Stat ********************************************/
struct CWMP_PRMT tDSLChannelStatsObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Total",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Showtime",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eDSLChannelStatsObject
{
	eDSLCHANNELSTATSTOTAL,
	eDSLCHANNELSTATSSHOWTIME,
};

struct CWMP_NODE tDSLChannelStatsObject[] =
{
/*info,  				leaf,			next)*/
{&tDSLChannelStatsObjectInfo[eDSLCHANNELSTATSTOTAL],	tDSLChannelStatsTotalLeaf,	NULL},
{&tDSLChannelStatsObjectInfo[eDSLCHANNELSTATSSHOWTIME],	tDSLChannelStatsShowtimeLeaf, NULL},
{NULL,					NULL,			NULL}
};


/****** Device.DSL.Channel.1 ****************************************************/
struct CWMP_OP tDSLChannelEntityLeafOP = { getDSLChannelEntity, setDSLChannelEntity};

struct CWMP_PRMT tDSLChannelEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDSLChannelEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,				&tDSLChannelEntityLeafOP},
//{"Alias",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDSLChannelEntityLeafOP},
{"Name",		eCWMP_tSTRING,	CWMP_READ, 				&tDSLChannelEntityLeafOP},
{"LastChange",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDSLChannelEntityLeafOP},
{"LowerLayers",	eCWMP_tSTRING,	CWMP_READ,				&tDSLChannelEntityLeafOP},
{"LinkEncapsulationSupported",	eCWMP_tSTRING,	CWMP_READ,	&tDSLChannelEntityLeafOP},
{"LinkEncapsulationUsed",		eCWMP_tSTRING,	CWMP_READ,	&tDSLChannelEntityLeafOP},
{"UpstreamCurrRate",			eCWMP_tUINT, CWMP_READ|CWMP_DENY_ACT,	&tDSLChannelEntityLeafOP},
{"DownstreamCurrRate",			eCWMP_tUINT, CWMP_READ|CWMP_DENY_ACT,	&tDSLChannelEntityLeafOP},
};
enum eDSLChannelEntityLeaf
{
	eEnable,
	eStatus,
	//eAlias,
	eName,
	eLastChange,
	eLowerLayers,
	eLinkEncapsulationSupported,
	eLinkEncapsulationUsed,
	eUpstreamCurrRate,
	eDownstreamCurrRate,
};

struct CWMP_LEAF tDSLChannelEntityLeaf[] =
{
{ &tDSLChannelEntityLeafInfo[eEnable]  },
{ &tDSLChannelEntityLeafInfo[eStatus]  },
//{ &tDSLChannelEntityLeafInfo[eAlias]  },
{ &tDSLChannelEntityLeafInfo[eName]  },
{ &tDSLChannelEntityLeafInfo[eLastChange]  },
{ &tDSLChannelEntityLeafInfo[eLowerLayers]  },
{ &tDSLChannelEntityLeafInfo[eLinkEncapsulationSupported]  },
{ &tDSLChannelEntityLeafInfo[eLinkEncapsulationUsed]  },
{ &tDSLChannelEntityLeafInfo[eUpstreamCurrRate]	},
{ &tDSLChannelEntityLeafInfo[eDownstreamCurrRate]  },
{ NULL	}
};

struct CWMP_PRMT tDSLChannelEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eDSLLineEntityObject
{
	eDSLChannelStatS,
};

extern struct CWMP_LEAF tDSLLineStatsLeaf[]; //Use the same object with DSL.Line.Stats
struct CWMP_NODE tDSLChannelEntityObject[] =
{
/*info,  				leaf,			next)*/
//Use the same leaf with DSL.Line.Stats because we only have only one line and channel
{&tDSLChannelEntityObjectInfo[eDSLChannelStatS],		tDSLLineStatsLeaf,	tDSLChannelStatsObject},
{NULL,					NULL,			NULL}
};

/****** Device.DSL.CHANNEL ***************************************************/
struct CWMP_PRMT tDSLChannelObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"1",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eDSLCHANNELObject
{
	eDSLChannel1,
};

struct CWMP_NODE tDSLChannelObject[] =
{
/*info,  				leaf,			next)*/
{&tDSLChannelObjectInfo[eDSLChannel1],		tDSLChannelEntityLeaf,	tDSLChannelEntityObject},
{NULL,					NULL,			NULL}
};

/***** Operations ************************************************************/
int getDSLChannelStatsTotal(char *name, struct CWMP_LEAF *entity, int *type, void **data)
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
	if( strcmp( lastname, "XTURFECErrors" )==0 )
	{
		*data = uintdup(MDS.FEC);
	}
	else if( strcmp( lastname, "XTUCFECErrors" )==0 )
	{
		*data = uintdup(MDS.AtucFEC);
	}
	else if( strcmp( lastname, "XTURHECErrors" )==0 )
	{
		*data = uintdup(MDS.HEC);
	}
	else if( strcmp( lastname, "XTUCHECErrors" )==0 )
	{
		*data = uintdup(MDS.AtucHEC);
	}
	else if( strcmp( lastname, "XTURCRCErrors" )==0 )
	{
		*data = uintdup(MDS.CRC);
	}
	else if( strcmp( lastname, "XTUCCRCErrors" )==0 )
	{
		*data = uintdup(MDS.AtucCRC);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int getDSLChannelStatsShowtime(char *name, struct CWMP_LEAF *entity, int *type, void **data)
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
	if( strcmp( lastname, "XTURFECErrors" )==0 )
	{
		*data = uintdup(MDS.FEC);
	}
	else if( strcmp( lastname, "XTUCFECErrors" )==0 )
	{
		*data = uintdup(MDS.AtucFEC);
	}
	else if( strcmp( lastname, "XTURHECErrors" )==0 )
	{
		*data = uintdup(MDS.HEC);
	}
	else if( strcmp( lastname, "XTUCHECErrors" )==0 )
	{
		*data = uintdup(MDS.AtucHEC);
	}
	else if( strcmp( lastname, "XTURCRCErrors" )==0 )
	{
		*data = uintdup(MDS.CRC);
	}
	else if( strcmp( lastname, "XTUCCRCErrors" )==0 )
	{
		*data = uintdup(MDS.AtucCRC);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int getDSLChannelEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
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
		*data = strdup("Device.DSL.Line.1");
	}
	else if( strcmp( lastname, "LinkEncapsulationSupported" )==0 )
	{
		Modem_ADSL2WANConfig vMA2WC = {0};

		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			//fprintf( stderr, "LinkEncapsulationSupported=0x%x\n", vMA2WC.LinkEncapSupported );
			buf[0]=0;;
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_992_3_ANNEX_K_ATM))
				strcat( buf, "G.992.3_Annex_K_ATM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_992_3_ANNEX_K_PTM))
				strcat( buf, "G.992.3_Annex_K_PTM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_993_2_ANNEX_K_ATM))
				strcat( buf, "G.993.2_Annex_K_ATM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_993_2_ANNEX_K_PTM))
				strcat( buf, "G.993.2_Annex_K_PTM," );
			if(vMA2WC.LinkEncapSupported & (1<<LE_G_994_1))
				strcat( buf, "G.994.1," );
			if(buf[0]) buf[ strlen(buf)-1 ]=0;
			*data = strdup( buf );
		}
	}
	else if( strcmp( lastname, "LinkEncapsulationUsed" )==0 )
	{
		Modem_ADSL2WANConfig vMA2WC = {0};

		if(adsl_drv_get(RLCM_GET_ADSL2WAN_IFCFG, (void *)&vMA2WC, TR069_ADSL2WANCFG_SIZE)==0)
			*data = strdup("");
		else{
			//fprintf( stderr, "LinkEncapsulationUsed=0x%x\n", vMA2WC.LinkEncapUsed );
			if(vMA2WC.LinkEncapUsed & (1<<LE_G_992_3_ANNEX_K_ATM))
				strcpy( buf, "G.992.3_Annex_K_ATM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_992_3_ANNEX_K_PTM))
				strcpy( buf, "G.992.3_Annex_K_PTM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_993_2_ANNEX_K_ATM))
				strcpy( buf, "G.993.2_Annex_K_ATM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_993_2_ANNEX_K_PTM))
				strcpy( buf, "G.993.2_Annex_K_PTM" );
			else if(vMA2WC.LinkEncapUsed & (1<<LE_G_994_1))
				strcpy( buf, "G.994.1" );
			else
				strcpy( buf, "" );
			*data = strdup( buf );
		}
	}
	else if( strcmp( lastname, "UpstreamCurrRate" )==0 )
	{
		getAdslInfo(ADSL_GET_RATE_US, buf, 256);
		*data = uintdup( atoi(buf) );
	}
	else if( strcmp( lastname, "DownstreamCurrRate" )==0 )
	{
		getAdslInfo(ADSL_GET_RATE_DS, buf, 256);
		*data = uintdup( atoi(buf) );
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setDSLChannelEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
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


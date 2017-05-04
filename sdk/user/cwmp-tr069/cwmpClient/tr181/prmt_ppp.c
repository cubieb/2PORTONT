#include <stdio.h>
#include <stdlib.h>

#include <linux/ethtool.h>
#include <config/autoconf.h>
#include <parameter_api.h>
#include <libcwmp.h>

#include "../cwmpc_utility.h"
#include "../prmt_apply.h"
#include "prmt_ppp.h"

/****** Device.PPP.Interface.{i}.Stats ***************************************/
struct CWMP_OP tPPPIfStatsLeafOP = { getPPPIfStats, NULL};

struct CWMP_PRMT tPPPIfStatsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"BytesSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"BytesReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"PacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"PacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"ErrorsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"ErrorsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"UnicastPacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"UnicastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"DiscardPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"DiscardPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"MulticastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"MulticastPacketsReceived",eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"BroadcastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfStatsLeafOP},
{"BroadcastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPPPIfStatsLeafOP},
{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tPPPIfStatsLeafOP},
};

enum ePPPIfStatsLeaf
{
	eStatsBytesSent,
	eStatsBytesReceived,
	eStatsPacketsSent,
	eStatsPacketsReceived,
	eStatsErrorsSent,
	eStatsErrorsReceived,
	eStatsUnicastPacketsSent,
	eStatsUnicastPacketsReceived,
	eStatsDiscardPacketsSent,
	eStatsDiscardPacketsReceived,
	eStatsMulticastPacketsSent,
	eStatsMulticastPacketsReceived,
	eStatsBroadcastPacketsSent,
	eStatsBroadcastPacketsReceived,
	eStatsUnknownProtoPacketsReceived,
};

struct CWMP_LEAF tPPPIfStatsLeaf[] =
{
{ &tPPPIfStatsLeafInfo[eStatsBytesSent]  },
{ &tPPPIfStatsLeafInfo[eStatsBytesReceived]  },
{ &tPPPIfStatsLeafInfo[eStatsPacketsSent]  },
{ &tPPPIfStatsLeafInfo[eStatsPacketsReceived]  },
{ &tPPPIfStatsLeafInfo[eStatsErrorsSent]  },
{ &tPPPIfStatsLeafInfo[eStatsErrorsReceived]  },
{ &tPPPIfStatsLeafInfo[eStatsUnicastPacketsSent]  },
{ &tPPPIfStatsLeafInfo[eStatsUnicastPacketsReceived]  },
{ &tPPPIfStatsLeafInfo[eStatsDiscardPacketsSent]  },
{ &tPPPIfStatsLeafInfo[eStatsDiscardPacketsReceived]  },
{ &tPPPIfStatsLeafInfo[eStatsMulticastPacketsSent]  },
{ &tPPPIfStatsLeafInfo[eStatsMulticastPacketsReceived]  },
{ &tPPPIfStatsLeafInfo[eStatsBroadcastPacketsSent]  },
{ &tPPPIfStatsLeafInfo[eStatsBroadcastPacketsReceived]  },
{ &tPPPIfStatsLeafInfo[eStatsUnknownProtoPacketsReceived]  },
{ NULL	}
};

/****** Device.PPP.Interface.{i}.PPPoE ***************************************/
struct CWMP_OP tPPPoELeafOP = { getPPPoE, NULL};

struct CWMP_PRMT tPPPoELeafInfo[] =
{
/*(name,			type,		flag,		op)*/
{"SessionID",	eCWMP_tULONG,	CWMP_READ,	&tPPPoELeafOP},
{"ACName",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tPPPoELeafOP},
{"ServiceName",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tPPPoELeafOP},
};

enum ePPPoELeaf
{
	ePPPoESessionID,
	ePPPoEACName,
	ePPPoEServiceName,
};

struct CWMP_LEAF tPPPoELeaf[] =
{
{ &tPPPoELeafInfo[ePPPoESessionID]  },
{ &tPPPoELeafInfo[ePPPoEACName]  },
{ &tPPPoELeafInfo[ePPPoEServiceName]  },
{ NULL	}
};


/****** Device.PPP.Interface.{i} *********************************************/
struct CWMP_LINKNODE *gPPPIfEntityObjList = NULL;

struct CWMP_OP tPPPIfEntityLeafOP = { getPPPIfEntity, setPPPIfEntity };

struct CWMP_PRMT tPPPIfEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,		&tPPPIfEntityLeafOP},
/*{"Alias",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},*/
{"Name",		eCWMP_tSTRING,	CWMP_READ,		&tPPPIfEntityLeafOP},
{"LastChange",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tPPPIfEntityLeafOP},
{"LowerLayers",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},
{"Reset",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},
{"ConnectionStatus",	eCWMP_tSTRING,	CWMP_READ,		&tPPPIfEntityLeafOP},
{"LastConnectionError",	eCWMP_tSTRING,	CWMP_READ,		&tPPPIfEntityLeafOP},
{"AutoDisconnectTime",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},
{"IdleDisconnectTime",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},
{"WarnDisconnectDelay",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},
{"Username",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},
{"Password",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},
{"EncryptionProtocol",	eCWMP_tSTRING,	CWMP_READ,		&tPPPIfEntityLeafOP},
{"CompressionProtocol",	eCWMP_tSTRING,	CWMP_READ,		&tPPPIfEntityLeafOP},
{"AuthenticationProtocol",	eCWMP_tSTRING,	CWMP_READ,	&tPPPIfEntityLeafOP},
{"MaxMRUSize",		eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},
{"CurrentMRUSize",	eCWMP_tUINT,	CWMP_READ,	&tPPPIfEntityLeafOP},
{"ConnectionTrigger",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tPPPIfEntityLeafOP},
{"LCPEcho", 		eCWMP_tUINT,	CWMP_READ,	&tPPPIfEntityLeafOP},
{"LCPEchoRetry",	eCWMP_tUINT,	CWMP_READ,	&tPPPIfEntityLeafOP},
};

enum ePPPIfEntityLeaf
{
	eIfEntityEnable,
	eIfEntityStatus,
//	eIfEntityAlias,
	eIfEntityName,
	eIfEntityLastChange,
	eIfEntityLowerLayers,
	eIfEntityReset,
	eIfEntityConnectionStatus,
	eIfEntityLastConnectionError,
	eIfEntityAutoDisconnectTime,
	eIfEntityIdleDisconnectTime,
	eIfEntityWarnDisconnectDelay,
	eIfEntityUsername,
	eIfEntityPassword,
	eIfEntityEncryptionProtocol,
	eIfEntityCompressionProtocol,
	eIfEntityAuthenticationProtocol,
	eIfEntityMaxMRUSize,
	eIfEntityCurrentMRUSize,
	eIfEntityConnectionTrigger,
	eIfEntityLCPEcho,
	eIfEntityLCPEchoRetry,
};

struct CWMP_LEAF tPPPIfEntityLeaf[] =
{
{ &tPPPIfEntityLeafInfo[eIfEntityEnable]  },
{ &tPPPIfEntityLeafInfo[eIfEntityStatus]  },
//{ &tPPPIfEntityLeafInfo[eIfEntityAlias]  },
{ &tPPPIfEntityLeafInfo[eIfEntityName]  },
{ &tPPPIfEntityLeafInfo[eIfEntityLastChange]  },
{ &tPPPIfEntityLeafInfo[eIfEntityLowerLayers]  },
{ &tPPPIfEntityLeafInfo[eIfEntityReset]  },
{ &tPPPIfEntityLeafInfo[eIfEntityConnectionStatus]  },
{ &tPPPIfEntityLeafInfo[eIfEntityLastConnectionError]	},
{ &tPPPIfEntityLeafInfo[eIfEntityAutoDisconnectTime]	},
{ &tPPPIfEntityLeafInfo[eIfEntityIdleDisconnectTime]	},
{ &tPPPIfEntityLeafInfo[eIfEntityWarnDisconnectDelay]	},
{ &tPPPIfEntityLeafInfo[eIfEntityUsername]  },
{ &tPPPIfEntityLeafInfo[eIfEntityPassword]  },
{ &tPPPIfEntityLeafInfo[eIfEntityEncryptionProtocol]	},
{ &tPPPIfEntityLeafInfo[eIfEntityCompressionProtocol]	},
{ &tPPPIfEntityLeafInfo[eIfEntityAuthenticationProtocol]	},
{ &tPPPIfEntityLeafInfo[eIfEntityMaxMRUSize]	},
{ &tPPPIfEntityLeafInfo[eIfEntityCurrentMRUSize]	},
{ &tPPPIfEntityLeafInfo[eIfEntityConnectionTrigger]  },
{ &tPPPIfEntityLeafInfo[eIfEntityLCPEcho]  },
{ &tPPPIfEntityLeafInfo[eIfEntityLCPEchoRetry]  },
{ NULL	}
};

struct CWMP_PRMT tPPPIfEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"PPPoE",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum ePPPIfEntityObject
{
	ePPPIfEntityPPPoE,
	ePPPIfEntityStats,
};

struct CWMP_NODE tPPPIfEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tPPPIfEntityObjectInfo[ePPPIfEntityPPPoE],	tPPPoELeaf,	NULL},
{&tPPPIfEntityObjectInfo[ePPPIfEntityStats],	tPPPIfStatsLeaf,	NULL},
{NULL,					NULL,			NULL}
};

/****** Device.PPP.Interface *********************************************/
struct CWMP_PRMT tPPPIfObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum ePPPIfObject
{
	ePPPIf0,
};

struct CWMP_LINKNODE tPPPIfObject[] =
{
/*info, 						leaf,				next,				sibling,	instnum)*/
{&tPPPIfObjectInfo[ePPPIf0], tPPPIfEntityLeaf, tPPPIfEntityObject, NULL, 0},
};

/****** Device.PPP ***********************************************************/
struct CWMP_OP tPPPLeafOP = { getPPP, NULL};
struct CWMP_OP tPPPIfObjectOP = { NULL, objPPPIf};


struct CWMP_PRMT tPPPLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"InterfaceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tPPPLeafOP},
};

enum ePPPLeaf
{
	eInterfaceNumberOfEntries,
};

struct CWMP_LEAF tPPPLeaf[] =
{
{ &tPPPLeafInfo[eInterfaceNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tPPPObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Interface",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tPPPIfObjectOP},
};

enum ePPPObject
{
	eInterface,
};

struct CWMP_NODE tPPPObject[] =
{
/*info,  				leaf,			next)*/
{&tPPPObjectInfo[eInterface],			NULL, NULL},
{NULL,					NULL,			NULL}
};

/***** Private Utilities *****************************************************/
unsigned int inline get_ppp_if_cnt()
{
	return mib_chain_total(MIB_ATM_VC_TBL);
}

static int get_ppp_if_info(int num, MIB_CE_ATM_VC_T *pEntry, int *index)
{
	int numofentries = mib_chain_total(MIB_ATM_VC_TBL);
	unsigned int cnt = 0;
	int i;

	if(pEntry == NULL || index == NULL)
		return -1;

	for(i = 0 ; i < numofentries ; i++)
	{
		if(!mib_chain_get( MIB_ATM_VC_TBL, i, (void *)pEntry))
			continue;

		if(num == pEntry->ConDevInstNum + 1)
		{
			*index = i;
			return 0;
		}
	}
	return -1;
}

void free_ppp_if_obj_data(void *obj_data)
{
	struct PPPIfObjData *data = (struct PPPIfObjData *)obj_data;

	if(data->LowerLayers == NULL)
		return;

	if(data->LowerLayers)
		free(data->LowerLayers);

	free(data);
}

struct PPPIfObjData *get_ppp_if_obj_data(int num)
{
	struct CWMP_LINKNODE *node = find_SiblingEntity(&gPPPIfEntityObjList, num);
	struct PPPIfObjData *data = NULL;

	if(node)
		data = (struct PPPIfObjData *)node->obj_data;

	return data;
}

static int set_ppp_if_lower(int num, MIB_CE_ATM_VC_T* pEntry, char *lower)
{
	int target = 0;
	struct PPPIfObjData *data = NULL;

	// No need to check num is valid and lower is not NULL.
	if(strcmp(lower, "") == 0 )
		goto ok;

	target = getInstNum(lower, "Ethernet.Link");
	if(target == num)
		goto ok;

	target = getInstNum(lower, "ATM.Link");
	if(target == num && MEDIA_INDEX(pEntry->ifIndex) == MEDIA_ATM)
		goto ok;

	return ERR_9007;
	
ok:
	data = get_ppp_if_obj_data(num);
	if(data == NULL)
	{
		fprintf(stderr, "Cannot find PPP.Interface.%d\n", num);
		return -1;
	}
	if(data && data->LowerLayers)
		free(data->LowerLayers);
	data->LowerLayers = strdup(lower);
	return 0;
}

/***** Operations ************************************************************/
int getPPPIfEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char ifname[IFNAMSIZ] = {0};
	int if_num = 0, index = -1;
	MIB_CE_ATM_VC_T vc_entity = {0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if_num = getInstNum(name, "PPP.Interface");
	if(if_num < 1)
		return ERR_9005;

	if(get_ppp_if_info(if_num, &vc_entity, &index) != 0)
		return ERR_9005;

	if(index != -1 && PPP_INDEX(vc_entity.ifIndex) != DUMMY_PPP_INDEX)
		snprintf(ifname, IFNAMSIZ, "ppp%d", PPP_INDEX(vc_entity.ifIndex));
		
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		if(vc_entity.enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int flags;

		if(vc_entity.enable==0)
			*data = strdup( "Down" );
		else if(getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_RUNNING)
				*data = strdup( "Up" );
			else
				*data = strdup( "Down" );
		}else
			*data = strdup( "Down" );
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		*data = strdup("");
	}*/
	else if( strcmp( lastname, "Name" )==0 )
	{
		*data = strdup(ifname);
	}
	else if( strcmp( lastname, "LastChange" )==0 )
	{
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		char buf[1024] = {0};	//empty string by default

		struct PPPIfObjData *obj = get_ppp_if_obj_data(if_num);

		if(obj && obj->LowerLayers)
		{
			*data = strdup(obj->LowerLayers);
		}
		else
		{
			if(vc_entity.cmode == CHANNEL_MODE_PPPOE)
				sprintf(buf, "Device.Ethernet.Link.%d", vc_entity.ConDevInstNum + 1);
			else if(vc_entity.cmode == CHANNEL_MODE_PPPOA)
				sprintf(buf, "Device.ATM.Link.%d", vc_entity.ConDevInstNum + 1);

			*data = strdup(buf);
		}
	}
	else if( strcmp( lastname, "Reset" )==0 )
	{
		// alway return false
		*data = booldup(0);
	}
	else if( strcmp( lastname, "ConnectionStatus" )==0 )
	{
		char pppstatus[32]={0};
#ifdef PPPOE_PASSTHROUGH
		int flags;

		if(vc_entity.cmode==CHANNEL_MODE_BRIDGE)
		{
			if(vc_entity.enable==0)
				*data = strdup( "Disconnected" );
			else if(getInFlags( ifname, &flags) == 1)
			{
				if (flags & IFF_RUNNING)
					*data = strdup( "Connected" );
				else
					*data = strdup( "Disconnected" );
			}else
				*data = strdup( "Disconnected" );
		}
		else
#endif
		if(vc_entity.cmode != CHANNEL_MODE_PPPOA && vc_entity.cmode != CHANNEL_MODE_PPPOE)
			*data = strdup("Unconfigured");
#ifdef CONFIG_PPP
		else if( getPPPConStatus( ifname, pppstatus )==0 )
			*data = strdup( pppstatus );
		else
#endif
			*data = strdup( "Disconnected" );
	}
	else if( strcmp( lastname, "LastConnectionError" )==0 )
	{
#ifdef PPPOE_PASSTHROUGH
		if(vc_entity.cmode==CHANNEL_MODE_BRIDGE)
			*data = strdup( "ERROR_NONE" );
		else
#endif
			*data = strdup(getLastConnectionError(vc_entity.ifIndex));
	}
	else if( strcmp( lastname, "AutoDisconnectTime" )==0 )
	{
		*data = uintdup( vc_entity.autoDisTime );
	}
	else if( strcmp( lastname, "IdleDisconnectTime" )==0 )
	{
		*data = uintdup( vc_entity.pppIdleTime );
	}
	else if( strcmp( lastname, "WarnDisconnectDelay" )==0 )
	{
		*data = uintdup( vc_entity.warnDisDelay );
	}
	else if( strcmp( lastname, "Username" )==0 )
	{
		*data = strdup( (char*)vc_entity.pppUsername );
	}
	else if( strcmp( lastname, "Password" )==0 )
	{
		*data = strdup("");
	}
	else if( strcmp( lastname, "EncryptionProtocol" )==0 )	// Jenny
	{
		*data = strdup( "None" );
	}
	else if( strcmp( lastname, "CompressionProtocol" )==0 )	// Jenny
	{
		*data = strdup( "None" );
	}
	else if( strcmp( lastname, "AuthenticationProtocol" )==0 )
	{
		if(vc_entity.pppAuth==PPP_AUTH_PAP)
			*data = strdup( "PAP" );
		else if(vc_entity.pppAuth==PPP_AUTH_CHAP)
			*data = strdup( "CHAP" );
		else if(vc_entity.pppAuth==PPP_AUTH_AUTO)
			*data = strdup( "PAPandCHAP" );
		else
			return ERR_9002;
	}
	else if( strcmp( lastname, "MaxMRUSize" )==0 )	// Jenny
	{
		*data = uintdup( vc_entity.mtu );
	}
	else if( strcmp( lastname, "CurrentMRUSize" )==0 )	// Jenny
	{
#ifdef CONFIG_PPP
		unsigned int cmru;
		
		if( getPPPCurrentMRU( ifname, vc_entity.cmode, &cmru )==0 )
			*data = uintdup( cmru );
		else
#endif
			*data = uintdup( vc_entity.mtu );
	}
	else if( strcmp( lastname, "ConnectionTrigger" )==0 )
	{
#ifdef PPPOE_PASSTHROUGH
		if( vc_entity.cmode==CHANNEL_MODE_BRIDGE )
			*data = strdup( "AlwaysOn" );
		else
#endif
		if( vc_entity.pppCtype==CONTINUOUS )
			*data = strdup( "AlwaysOn" );
		else if( vc_entity.pppCtype==CONNECT_ON_DEMAND )
			*data = strdup( "OnDemand" );
		else
			*data = strdup( "Manual" );
	}
	else if( strcmp( lastname, "LCPEcho" )==0 )	// Jenny
	{
#ifdef CONFIG_PPP
		unsigned int echo;
		if( getPPPLCPEcho( ifname, vc_entity.cmode, &echo )==0 )
			*data = uintdup( echo );
		else
#endif
			*data = uintdup( 0 );
	}
	else if( strcmp( lastname, "LCPEchoRetry" )==0 )	// Jenny
	{
#ifdef CONFIG_PPP
		unsigned int retry;
		if( getPPPEchoRetry( ifname, vc_entity.cmode, &retry )==0 )
			*data = uintdup( retry );
		else
#endif
			*data = uintdup( 0 );
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setPPPIfEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf=data;
	int if_num;
	char ifname[IFNAMSIZ] = {0};
	MIB_CE_ATM_VC_T vc_entity = {0};
	int chainidx = 0;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	if_num = getInstNum(name, "Interface");
	if(if_num < 1)
		return ERR_9005;

	if(get_ppp_if_info(if_num, &vc_entity, &chainidx) != 0)
		return ERR_9005;

	gWanBitMap |= (1U << chainidx);

	if(PPP_INDEX(vc_entity.ifIndex) != DUMMY_PPP_INDEX)
		snprintf(ifname, IFNAMSIZ, "ppp%d", PPP_INDEX(vc_entity.ifIndex));

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if(i==NULL) return ERR_9007;
		vc_entity.enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_ATM_VC_TBL, &vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		return ERR_9001;
	}*/
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		char *lower = data;
		struct PPPIfObjData *data = NULL;
		int ret;

		ret = set_ppp_if_lower(if_num, &vc_entity, lower);

		// Failed
		if(ret != 0)
			return ret;

		apply_add( CWMP_PRI_SL, lower_layers_changed, CWMP_RESTART, 0, NULL, 0 );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Reset" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i==1)
		{
			resetChainID = chainidx;
			gStartReset = 1;
		}
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "AutoDisconnectTime" )==0 )	// Jenny
	{
		unsigned int *disconntime = data;

		if(disconntime==NULL) return ERR_9007;
		vc_entity.autoDisTime = (unsigned short) (*disconntime);
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "IdleDisconnectTime" )==0 )
	{
		unsigned int *idletime = data;

		if(idletime==NULL) return ERR_9007;
		vc_entity.pppIdleTime = (unsigned short) (*idletime);
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "WarnDisconnectDelay" )==0 )	// Jenny
	{
		unsigned int *disconndelay = data;

		if(disconndelay==NULL) return ERR_9007;
		vc_entity.warnDisDelay = (unsigned short) (*disconndelay);
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Username" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;
		strncpy( (char*)vc_entity.pppUsername, buf, MAX_NAME_LEN-1 );
		vc_entity.pppUsername[MAX_NAME_LEN-1]=0;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Password" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;
		strncpy( (char*)vc_entity.pppPassword, buf, MAX_NAME_LEN-1 );
		vc_entity.pppPassword[MAX_NAME_LEN-1]=0;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "MaxMRUSize" )==0 )	// Jenny
	{
		unsigned int *mru = data;

		if (*mru<1)	*mru = 1;
		else if (*mru>1540)	*mru = 1540;
		vc_entity.mtu = *mru;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "ConnectionTrigger" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;
#ifdef PPPOE_PASSTHROUGH
		if( vc_entity.cmode==CHANNEL_MODE_BRIDGE )
		{
			if( strcmp( buf, "AlwaysOn" )==0 )
				return CWMP_APPLIED;
			else
				return ERR_9001;
		}
#endif
		if( strcmp( buf, "AlwaysOn" )==0 )
			vc_entity.pppCtype=CONTINUOUS;
		else if( strcmp( buf, "OnDemand" )==0 )
			vc_entity.pppCtype=CONNECT_ON_DEMAND;
		else if( strcmp( buf, "Manual" )==0 )
			vc_entity.pppCtype=MANUAL;
		else
			return ERR_9007;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else
	{
		return ERR_9005;
	}

	return CWMP_APPLIED;
}


int objPPPIf(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);
	
	switch( type )
	{
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
		{
			int total,i;
			struct CWMP_LINKNODE *old_table;
			int has_new=0;
			struct CWMP_LINKNODE *tmp_entity=NULL;
			MIB_CE_ATM_VC_T *p,vc_entity;
			struct node *old_list;

			total = mib_chain_total( MIB_ATM_VC_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			p = &vc_entity;
			
			for( i = 0 ; i < total ; i++ )
			{
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
					continue;

				tmp_entity = remove_SiblingEntity( &old_table, p->ConDevInstNum + 1);
				if( tmp_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, tmp_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, p->ConDevInstNum + 1 )==NULL )
					{
						unsigned int MaxInstNum = p->ConDevInstNum + 1;

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tPPPIfObject, sizeof(tPPPIfObject), &MaxInstNum );
						has_new = 1;

						tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, p->ConDevInstNum + 1 );
						if(tmp_entity)
						{
							tmp_entity->obj_data = malloc(sizeof(struct PPPIfObjData));
							if(tmp_entity->obj_data == NULL)
							{
								fprintf(stderr, "<%s:%d> malloc failed!\n", __FUNCTION__, __LINE__);
								return -1;
							}
							tmp_entity->free_obj_data = free_ppp_if_obj_data;
							memset(tmp_entity->obj_data, 0, sizeof(struct PPPIfObjData));
						}
					}//else already in next_table
				}
			}

			gPPPIfEntityObjList = (struct CWMP_LINKNODE *)entity->next;

			if( old_table )
			{
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );
			}

			if(has_new)
			{
				notify_set_wan_changed();
				return 0;
			}
		}
	}
	
	return -1;
}

int getPPPoE(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char ifname[IFNAMSIZ] = {0};
	int if_num = 0;
	int total_if = get_ppp_if_cnt();
	MIB_CE_ATM_VC_T vc_entity = {0};
	int index = -1;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if_num = getInstNum(name, "PPP.Interface");
	if(if_num < 1)
		return ERR_9005;

	if(get_ppp_if_info(if_num, &vc_entity, &index) != 0)
		return ERR_9005;

	if(index != -1 && PPP_INDEX(vc_entity.ifIndex) != DUMMY_PPP_INDEX)
		snprintf(ifname, IFNAMSIZ, "ppp%d", PPP_INDEX(vc_entity.ifIndex));

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "SessionID" )==0 )
	{
		unsigned char totalEntry;
		MIB_CE_PPPOE_SESSION_T Entry;
		unsigned int i,sessionID=0;//,found=0;

		totalEntry = mib_chain_total(MIB_PPPOE_SESSION_TBL); /* get chain record size */
		for (i=0; i<totalEntry; i++) {
			if (!mib_chain_get(MIB_PPPOE_SESSION_TBL, i, (void *)&Entry)) {
				return ERR_9002;
			}
			if (Entry.uifno == vc_entity.ifIndex)
			{
				sessionID = Entry.sessionId;
				break;
			}
		}
		if(sessionID < 1)
			*data = uintdup(0);
		else
			*data = uintdup(sessionID);
	}
	else if( strcmp( lastname, "ACName" )==0 )
	{
		if( vc_entity.cmode == CHANNEL_MODE_PPPOE )
			*data = strdup( (char*)vc_entity.pppACName );
		else
			*data = strdup( "" );
	}
	else if( strcmp( lastname, "ServiceName" )==0 )
	{
		if( vc_entity.cmode==CHANNEL_MODE_PPPOE )
			*data = strdup( (char*)vc_entity.pppServiceName );
		else
			*data = strdup( "" );
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setPPPoE(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf=data;
	int if_num;
	char ifname[IFNAMSIZ] = {0};
	MIB_CE_ATM_VC_T vc_entity = {0};
	int index = -1;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	if_num = getInstNum(name, "PPP.Interface");
	if(if_num < 1)
		return ERR_9005;

	if(get_ppp_if_info(if_num, &vc_entity, &index) != 0)
		return ERR_9005;

	gWanBitMap |= (1U << index);

	if(index != -1 && PPP_INDEX(vc_entity.ifIndex) != DUMMY_PPP_INDEX)
		snprintf(ifname, IFNAMSIZ, "ppp%d", PPP_INDEX(vc_entity.ifIndex));

	if( strcmp( lastname, "ACName" )==0 )
	{
		if( (vc_entity.cmode==CHANNEL_MODE_PPPOE) && (strlen( buf )<MAX_NAME_LEN) )
			strcpy( (char*)vc_entity.pppACName, buf );
		else
			return ERR_9001;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, if_num - 1 );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "ServiceName" )==0 )
	{
		if( (vc_entity.cmode==CHANNEL_MODE_PPPOE) && (strlen( buf )<MAX_NAME_LEN) )	// Jenny
			strcpy( (char*)vc_entity.pppServiceName, buf );
		else
			return ERR_9001;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, if_num - 1 );
		return CWMP_NEED_RESTART_WAN;
	}
	else
	{
		return ERR_9005;
	}

	return CWMP_APPLIED;
}


int getPPPIfStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	ifname[16];
	struct net_device_stats nds = {0};
	int if_num;
	MIB_CE_ATM_VC_T vc_entity = {0};
	int index = -1;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if_num = getInstNum(name, "PPP.Interface");
	if(if_num < 1)
		return ERR_9005;
	
	if(get_ppp_if_info(if_num, &vc_entity, &index) != 0)
		return ERR_9005;

	if(index != -1 && PPP_INDEX(vc_entity.ifIndex) != DUMMY_PPP_INDEX)
	{
		snprintf(ifname, IFNAMSIZ, "ppp%d", PPP_INDEX(vc_entity.ifIndex));
		get_net_device_stats(ifname, &nds);
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "BytesSent" )==0 )
	{
		*data = (void*)ulongdup(nds.tx_bytes);
	}
	else if( strcmp( lastname, "BytesReceived" )==0 )
	{
		*data = (void*)ulongdup(nds.rx_bytes);
	}
	else if( strcmp( lastname, "PacketsSent" )==0 )
	{
		*data = (void*)ulongdup(nds.tx_packets);
	}
	else if( strcmp( lastname, "PacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(nds.rx_packets);
	}
	else if( strcmp( lastname, "ErrorsSent" )==0 )
	{
		*data = uintdup(nds.tx_errors);
	}
	else if( strcmp( lastname, "ErrorsReceived" )==0 )
	{
		*data = uintdup(nds.rx_errors);
	}
	else if( strcmp( lastname, "UnicastPacketsSent" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "UnicastPacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "DiscardPacketsSent" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "DiscardPacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "MulticastPacketsSent" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "MulticastPacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "BroadcastPacketsSent" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "BroadcastPacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "UnknownProtoPacketsReceived" )==0 )
	{
		*data = uintdup(0);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int getPPP(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "InterfaceNumberOfEntries" )==0 )
	{
		*data = uintdup(get_ppp_if_cnt());
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


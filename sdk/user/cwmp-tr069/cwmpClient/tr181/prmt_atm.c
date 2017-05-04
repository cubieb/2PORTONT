#include <stdio.h>
#include <stdlib.h>

#include <linux/atm.h>
#include <rtk/mib.h>
#include <rtk/utility.h>

#include <parameter_api.h>

#include "../prmt_wanatmf5loopback.h"
#include "prmt_atm.h"

/****** Device.ATM.Diagnostics.F5Loopback. ***********************************/
struct CWMP_OP tATMF5LBLeafOP = { getATMF5LB, setATMF5LB };
struct CWMP_PRMT tATMF5LBLeafInfo[] =
{
/*(name,			type,		flag,					op)*/
{"Interface",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tATMF5LBLeafOP},
};

enum eATMF5LBLeaf
{
	eF5_Interface,
};

// defined in ../prmt_wanatmf5loopback.c
extern struct CWMP_PRMT tWANATMF5LBLeafInfo[];
struct CWMP_LEAF tATMF5LBLeaf[] =
{
{ &tWANATMF5LBLeafInfo[eF5_DiagnosticsState] },
{ &tATMF5LBLeafInfo[eF5_Interface] },
{ &tWANATMF5LBLeafInfo[eF5_NumberOfRepetitions] },
{ &tWANATMF5LBLeafInfo[eF5_Timeout] },
{ &tWANATMF5LBLeafInfo[eF5_SuccessCount] },
{ &tWANATMF5LBLeafInfo[eF5_FailureCount] },
{ &tWANATMF5LBLeafInfo[eF5_AverageResponseTime] },
{ &tWANATMF5LBLeafInfo[eF5_MinimumResponseTime] },
{ &tWANATMF5LBLeafInfo[eF5_MaximumResponseTime] },
{ NULL }
};

/****** Device.ATM.Diagnostics. **********************************************/
struct CWMP_PRMT tATMDiagObjectInfo[] =
{
/*(name,		type,		flag,		op)*/
{"F5Loopback",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eATMDiagObject
{
	eF5Loopback,
};

struct CWMP_NODE tATMDiagObject[] =
{
/*info,  				leaf,			next)*/
{&tATMDiagObjectInfo[eF5Loopback],		tATMF5LBLeaf,	NULL},
{NULL,					NULL,			NULL}
};

/****** Device.ATM.Link.{i}.Stats ********************************************/
struct CWMP_OP tATMLinkStatsLeafOP = { getATMLinkStats, NULL};

struct CWMP_PRMT tATMLinkStatsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"BytesSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"BytesReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"PacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"PacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"ErrorsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"ErrorsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"UnicastPacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"UnicastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"DiscardPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"DiscardPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"MulticastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"MulticastPacketsReceived",eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"BroadcastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"BroadcastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"TransmittedBlocks",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"ReceivedBlocks",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"CRCErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
{"HECErrors",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkStatsLeafOP},
};

enum eATMLinkStatsLeaf
{
	eLinkStatsBytesSent,
	eLinkStatsBytesReceived,
	eLinkStatsPacketsSent,
	eLinkStatsPacketsReceived,
	eLinkStatsErrorsSent,
	eLinkStatsErrorsReceived,
	eLinkStatsUnicastPacketsSent,
	eLinkStatsUnicastPacketsReceived,
	eLinkStatsDiscardPacketsSent,
	eLinkStatsDiscardPacketsReceived,
	eLinkStatsMulticastPacketsSent,
	eLinkStatsMulticastPacketsReceived,
	eLinkStatsBroadcastPacketsSent,
	eLinkStatsBroadcastPacketsReceived,
	eLinkStatsUnknownProtoPacketsReceived,
	eLinkStatsTransmittedBlocks,
	eLinkStatsReceivedBlocks,
	eLinkStatsCRCErrors,
	eLinkStatsHECErrors,
};

struct CWMP_LEAF tATMLinkStatsLeaf[] =
{
{ &tATMLinkStatsLeafInfo[eLinkStatsBytesSent]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsBytesReceived]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsPacketsSent]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsPacketsReceived]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsErrorsSent]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsErrorsReceived]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsUnicastPacketsSent]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsUnicastPacketsReceived]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsDiscardPacketsSent]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsDiscardPacketsReceived]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsMulticastPacketsSent]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsMulticastPacketsReceived]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsBroadcastPacketsSent]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsBroadcastPacketsReceived]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsUnknownProtoPacketsReceived]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsTransmittedBlocks]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsReceivedBlocks]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsCRCErrors]  },
{ &tATMLinkStatsLeafInfo[eLinkStatsHECErrors]  },
{ NULL	}
};

/****** Device.ATM.Link.{i} **************************************************/
struct CWMP_OP tATMLinkEntityLeafOP = { getATMLinkEntity, setATMLinkEntity};

struct CWMP_PRMT tATMLinkEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tATMLinkEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tATMLinkEntityLeafOP},
/*{"Alias",			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tATMLinkEntityLeafOP},*/
{"Name",			eCWMP_tSTRING,	CWMP_READ,		&tATMLinkEntityLeafOP},
{"LastChange",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tATMLinkEntityLeafOP},
{"LowerLayers",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tATMLinkEntityLeafOP},
{"LinkType",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tATMLinkEntityLeafOP},
{"AutoConfig",		eCWMP_tBOOLEAN,	CWMP_READ,		&tATMLinkEntityLeafOP},
{"DestinationAddress",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tATMLinkEntityLeafOP},
{"Encapsulation",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tATMLinkEntityLeafOP},
{"FCSPreserved",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tATMLinkEntityLeafOP},
{"VCSearchList",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tATMLinkEntityLeafOP},
{"AAL",				eCWMP_tSTRING,	CWMP_READ,		&tATMLinkEntityLeafOP},
};

enum eATMLinkEntityLeaf
{
	eLinkEntityEnable,
	eLinkEntityStatus,
//	eLinkEntityAlias,
	eLinkEntityName,
	eLinkEntityLastChange,
	eLinkEntityLowerLayers,
	eLinkEntityLinkType,
	eLinkEntityAutoConfig,
	eLinkEntityDestinationAddress,
	eLinkEntityEncapsulation,
	eLinkEntityFCSPreserved,
	eLinkEntityVCSearchList,
	eLinkEntityAAL,
};

struct CWMP_LEAF tATMLinkEntityLeaf[] =
{
{ &tATMLinkEntityLeafInfo[eLinkEntityEnable]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityStatus]  },
//{ &tATMLinkEntityLeafInfo[eLinkEntityAlias]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityName]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityLastChange]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityLowerLayers]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityLinkType]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityAutoConfig]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityDestinationAddress]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityEncapsulation]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityFCSPreserved]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityVCSearchList]  },
{ &tATMLinkEntityLeafInfo[eLinkEntityAAL]  },
{ NULL	}
};

struct CWMP_PRMT tATMLinkEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eATMLinkEntityObject
{
	eATMLinkEntityStats,
};

struct CWMP_NODE tATMLinkEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tATMLinkEntityObjectInfo[eATMLinkEntityStats],		tATMLinkStatsLeaf,	NULL},
{NULL,					NULL,			NULL}
};

/****** Device.ATM.Link ******************************************************/
struct CWMP_PRMT tATMLinkObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum eATMLinkObject
{
	eATMLink0,
};

struct CWMP_LINKNODE tATMLinkObject[] =
{
/*info, 						leaf,				next,				sibling,	instnum)*/
{&tATMLinkObjectInfo[eATMLink0], tATMLinkEntityLeaf, tATMLinkEntityObject, NULL, 0},
};

/****** Device.ATM ***********************************************************/
struct CWMP_OP tATMLeafOP = { getATM, NULL};
struct CWMP_OP tATMLinkObjectOP = { NULL, objATMLink};

struct CWMP_PRMT tATMLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"LinkNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tATMLeafOP},
};
enum eATMLeaf
{
	eLinkNumberOfEntries,
};

struct CWMP_LEAF tATMLeaf[] =
{
{ &tATMLeafInfo[eLinkNumberOfEntries]  },
{ NULL	}
};


struct CWMP_PRMT tATMObjectInfo[] =
{
/*(name,	type,			flag,					op)*/
{"Link",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tATMLinkObjectOP},
{"Diagnostics",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eATMObject
{
	eATMLink,
	eATMDiagnostics,
};

struct CWMP_NODE tATMObject[] =
{
/*info,  				leaf,			next)*/
{&tATMObjectInfo[eATMLink],			NULL,	NULL},
{&tATMObjectInfo[eATMDiagnostics],	NULL,	tATMDiagObject},
{NULL,					NULL,			NULL}
};

/***** Utility Functions *****************************************************/
int getATMLinkCnt()
{
	int numofentries = mib_chain_total(MIB_ATM_VC_TBL);
	unsigned int totalwancon = 0;
	MIB_CE_ATM_VC_T vc_entity;
	int i;

	for( i = 0; i < numofentries; i++ )
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vc_entity))
		{
			CWMPDBP(1, "Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)", i);
			return ERR_9002;
		}

		if(MEDIA_INDEX(vc_entity.ifIndex) == MEDIA_ATM)
			totalwancon++;
	}
	return totalwancon;
}

/***** Operations ************************************************************/
extern struct WANATMF5Loopback gWANATMF5LB;
int getATMF5LB(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	unsigned int	chainid;
	MIB_CE_ATM_VC_T	*pEntry, vc_entity;
	char ifname[IFNAMSIZ] = {0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Interface" )==0 )
	{
		int total = mib_chain_total(MIB_ATM_VC_TBL);
		int i;
	
		for( i = 0; i < total; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vc_entity))
			{
				CWMPDBP(1, "Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)", i);
				return ERR_9002;
			}

			// how about 2 entries have the same vpi/vci?
			if(MEDIA_INDEX(vc_entity.ifIndex) == MEDIA_ATM &&
				vc_entity.vpi == gWANATMF5LB.vpi && vc_entity.vci == gWANATMF5LB.vci)
				break;
		}

		if(i < total)
		{
			char path[256] = {0};
			
			snprintf(path, 256, "Device.ATM.Link.%d", vc_entity.ConDevInstNum + 1);
			*data = strdup(path);
		}
		else
			*data = strdup("");
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setATMF5LB(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char		*lastname = entity->info->name;
	unsigned int	*pUint=data;
	unsigned int	wandevnum,condevnum;	
	unsigned int	chainid;
	MIB_CE_ATM_VC_T	vc_entity;
	char ifname[IFNAMSIZ] = {0};

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Interface" )==0 )
	{
		char *buf=data;
		int linknum;

		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		
		linknum = getInstNum(buf, "ATM.Link");
		if(linknum < 1) return ERR_9007;

		if(get_wan_info_tr181(linknum, &vc_entity, &chainid, ifname) != 1)
			return ERR_9007;
		
		if(MEDIA_INDEX(vc_entity.ifIndex) != MEDIA_ATM)
			return ERR_9007;

		gWANATMF5LB.vpi=vc_entity.vpi;
		gWANATMF5LB.vci=vc_entity.vci;
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}


int getATMLinkStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int linknum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	ifname[16];
	struct net_device_stats nds;
	int		iserror=0;
	ch_stat stats = {0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	linknum = getInstNum(name, "ATM.Link");
	if(linknum == 0) return ERR_9005;

	pEntry = &vc_entity;
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(MEDIA_INDEX(pEntry->ifIndex) != MEDIA_ATM)
			continue;

		if( pEntry->ConDevInstNum + 1 == linknum )
			break;
	}
	if(i==num) return ERR_9005;

	if( ifGetName(pEntry->ifIndex, ifname, sizeof(ifname)) )
		getATMStats(pEntry->vpi, pEntry->vci, &stats);

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "BytesSent" )==0 )
	{
		*data = (void*)ulongdup(stats.tx_byte_cnt);
	}
	else if( strcmp( lastname, "BytesReceived" )==0 )
	{
		*data = (void*)ulongdup(stats.rx_byte_cnt);
	}
	else if( strcmp( lastname, "PacketsSent" )==0 )
	{
		*data = (void *)ulongdup(stats.tx_pkt_ok_cnt);
	}
	else if( strcmp( lastname, "PacketsReceived" )==0 )
	{
		*data = (void *)ulongdup(stats.rx_pkt_cnt);
	}
	else if( strcmp( lastname, "ErrorsSent" )==0 )
	{
		*data = uintdup(stats.tx_pkt_fail_cnt);
	}
	else if( strcmp( lastname, "ErrorsReceived" )==0 )
	{
		*data = uintdup(stats.rx_pkt_fail);
	}
	else if( strcmp( lastname, "UnicastPacketsSent" )==0 )
	{
		//we do not support
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "UnicastPacketsReceived" )==0 )
	{
		//we do not support
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "DiscardPacketsSent" )==0 )
	{
		//we do not support
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "DiscardPacketsReceived" )==0 )
	{
		//we do not support
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "MulticastPacketsSent" )==0 )
	{
		//we do not support
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "MulticastPacketsReceived" )==0 )
	{
		//we do not support
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "BroadcastPacketsSent" )==0 )
	{
		//we do not support
		*data = (void *)ulongdup(0);
	}
	else if( strcmp( lastname, "BroadcastPacketsReceived" )==0 )
	{
		//we do not support
		*data = (void *)ulongdup(0);
	}
	else if( strcmp( lastname, "UnknownProtoPacketsReceived" )==0 )
	{
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "TransmittedBlocks" )==0 )
	{
		*data = uintdup( stats.tx_byte_cnt/48 );
	}
	else if( strcmp( lastname, "ReceivedBlocks" )==0 )
	{
		*data = uintdup( stats.rx_byte_cnt/48 );
	}
	else if( strcmp( lastname, "CRCErrors" )==0 )
	{
		*data = uintdup(stats.rx_crc_error);
	}
	else if( strcmp( lastname, "HECErrors" )==0 )
	{
		//we do not support
		*data = uintdup(0);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int getATMLinkEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int linknum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	linknum = getInstNum(name, "ATM.Link");
	if(linknum == 0) return ERR_9005;

	pEntry = &vc_entity;
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(MEDIA_INDEX(pEntry->ifIndex) != MEDIA_ATM)
			continue;

		if( pEntry->ConDevInstNum + 1 == linknum )
			break;
	}
	if(i==num) return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		if(pEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int 	flags;
		char	ifname[16];

		if(pEntry->cmode == CHANNEL_MODE_PPPOA)
		{
			if(ifGetName(pEntry->ifIndex, ifname, sizeof(ifname)) == 0)
				return ERR_9002;
		}
		else
		{
			if(ifGetName(PHY_INTF(pEntry->ifIndex), ifname, sizeof(ifname)) == 0)
				return ERR_9002;
		}

		if(pEntry->enable==0)
			*data = strdup( "Down" );
		else if(getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_RUNNING)
				*data = strdup( "Up" );
			else
				*data = strdup( "Down" );
		}else
			*data = strdup( "Unavailable" );
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		*data = strdup("Up");
	}*/
	else if( strcmp( lastname, "Name" )==0 )
	{
		char	ifname[16];

		if(pEntry->cmode == CHANNEL_MODE_PPPOA)
		{
			if(ifGetName(pEntry->ifIndex, ifname, sizeof(ifname)) == 0)
				return ERR_9002;
		}
		else
		{
			if(ifGetName(PHY_INTF(pEntry->ifIndex), ifname, sizeof(ifname)) == 0)
				return ERR_9002;
		}

		*data = strdup(ifname);
	}
	else if( strcmp( lastname, "LastChange" )==0 )
	{
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		*data = strdup("Device.DSL.Channel.1");
	}
	else if( strcmp( lastname, "LinkType" )==0 )
	{
		if( pEntry->cmode == CHANNEL_MODE_PPPOE )
			*data = strdup("EoA");//PPPoE => EoA
		else if (pEntry->cmode == CHANNEL_MODE_PPPOA)
			*data = strdup("PPPoA");
		else if (pEntry->cmode == CHANNEL_MODE_BRIDGE)
			*data = strdup("EoA");
		else if (pEntry->cmode == CHANNEL_MODE_IPOE)
			*data = strdup("EoA");
		else if (pEntry->cmode == CHANNEL_MODE_RT1483)
			*data = strdup("IPoA");
		else
			*data = strdup("Unconfigured");
	}
	else if( strcmp( lastname, "AutoConfig" )==0 )
	{
		*data = booldup(0);
	}
	else if( strcmp( lastname, "DestinationAddress" )==0 )
	{
		char buf[256] = {0};

		sprintf( buf, "%u/%u", pEntry->vpi, pEntry->vci );
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "Encapsulation" )==0 )
	{
		if( pEntry->encap==ENCAP_VCMUX )
			*data = strdup( "VCMUX" );
		else if( pEntry->encap==ENCAP_LLC )
			*data = strdup( "LLC" );
		else
			return ERR_9002;
	}
	else if( strcmp( lastname, "FCSPreserved" )==0 )
	{
		//Linux 2.6.30 does not support this value
		*data = booldup(0);
	}
	else if( strcmp( lastname, "VCSearchList" )==0 )
	{
		//We don't support this function
		*data = strdup("");
	}
	else if( strcmp( lastname, "AAL" )==0 )
	{
		*data = strdup("AAL5");
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setATMLinkEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf=data;
	char tmp[256]="";
	unsigned int linknum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	linknum = getInstNum(name, "ATM.Link");
	if(linknum == 0) return ERR_9005;

	pEntry = &vc_entity;
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(MEDIA_INDEX(pEntry->ifIndex) != MEDIA_ATM)
			continue;

		if( pEntry->ConDevInstNum + 1 == linknum )
			break;
	}
	if(i==num) return ERR_9005;
	gWanBitMap |= (1U << i);

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *integer = data;
		unsigned char enable = *integer;

		if(enable == pEntry->enable)
			return CWMP_APPLIED;

		if(enable)
			pEntry->enable = 1;
		else
			pEntry->enable = 0;

		mib_chain_update(MIB_ATM_VC_TBL, pEntry, i);

		return CWMP_NEED_RESTART_WAN;
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		return ERR_9001;
	}*/
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		return ERR_9001;
	}
	else if( strcmp( lastname, "LinkType" )==0 )
	{
		char *tmp=data;
		unsigned int old_cmode, new_cmode;
		unsigned int old_ifindex, new_ifindex;

		old_cmode = pEntry->cmode;
		old_ifindex = pEntry->ifIndex;

		if( tmp==NULL ) return ERR_9007;
		if( strlen(tmp)==0 ) return ERR_9007;

		if( strcmp( "PPPoA", tmp )==0 )
		{
			if(pEntry->cmode != CHANNEL_MODE_PPPOA)
			{
				new_ifindex = getNewIfIndex( CHANNEL_MODE_PPPOA, pEntry->ConDevInstNum, MEDIA_INDEX(pEntry->ifIndex), i);
				if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9001;
				pEntry->cmode = CHANNEL_MODE_PPPOA;
				pEntry->ifIndex = new_ifindex;
				pEntry->mtu = 1500;
			}
		}
		else if( strcmp( "EoA", tmp )==0 )
		{
			if( (pEntry->cmode!=CHANNEL_MODE_BRIDGE) &&
			    (pEntry->cmode!=CHANNEL_MODE_IPOE) &&
			    (pEntry->cmode!=CHANNEL_MODE_PPPOE) )
			{
				new_ifindex = getNewIfIndex( CHANNEL_MODE_BRIDGE, pEntry->ConDevInstNum, MEDIA_INDEX(pEntry->ifIndex), i);
				if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9001;
				pEntry->cmode = CHANNEL_MODE_BRIDGE;
				pEntry->ifIndex = new_ifindex;
				pEntry->mtu = 1500;
		#ifdef CONFIG_E8B
			#ifdef _PRMT_X_CT_COM_WANEXT_
				pEntry->ServiceList=X_CT_SRV_INTERNET;
			#endif
			#ifdef CTC_WAN_NAME
				pEntry->applicationtype = X_CT_SRV_INTERNET;
			#endif
		#endif
			}
		}
		else if( strcmp( "IPoA", tmp )==0 )
		{
			if(pEntry->cmode != CHANNEL_MODE_RT1483)
			{
				new_ifindex = getNewIfIndex( CHANNEL_MODE_RT1483, pEntry->ConDevInstNum, MEDIA_INDEX(pEntry->ifIndex), i);
				if( (new_ifindex==NA_VC)||(new_ifindex==NA_PPP) ) return ERR_9001;
				pEntry->cmode = CHANNEL_MODE_RT1483;
				pEntry->ifIndex = new_ifindex;
				pEntry->mtu = 1500;
			}
		}
		else
			return ERR_9007;

		if(pEntry->cmode!= old_cmode)
		{
			//if ip -> ppp	or ppp->ip, destroy ppp/ip objects and reset values??
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
#ifdef PORT_FORWARD_GENERAL
			updatePortForwarding( old_ifindex, pEntry->ifIndex );
#endif
#ifdef ROUTING
			updateRoutingTable( old_ifindex, pEntry->ifIndex );
#endif
			return CWMP_NEED_RESTART_WAN;
		}
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "DestinationAddress" )==0 )
	{
		char *buf = data;
		unsigned int vpi, vci;

		if(sscanf(buf, "%u/%u", &vpi, &vci) != 2)
			return ERR_9007;

		pEntry->vpi = vpi;
		pEntry->vci = vci;

		//if (vpi,vci) conflicts with other channels??
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Encapsulation" )==0 )
	{
		char *tmp = data;

		if( strcmp(tmp, "VCMUX")==0 )
			pEntry->encap=ENCAP_VCMUX;
		else if( strcmp(tmp, "LLC")==0 )
			pEntry->encap=ENCAP_LLC;
		else
			return ERR_9007;

		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)pEntry, i );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "FCSPreserved" )==0 )
	{
		//Linux 2.6.30 does not support this value
		return ERR_9001;
	}
	else if( strcmp( lastname, "VCSearchList" )==0 )
	{
		//We don't support this function
		return ERR_9001;
	}
	else
	{
		return ERR_9005;
	}

	return CWMP_APPLIED;
}

int objATMLink(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);
	
	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
			unsigned int num=0,MaxInstNum=0,i;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			MaxInstNum = getATMLinkCnt();
				
			if( create_Object( c, tATMLinkObject, sizeof(tATMLinkObject), MaxInstNum, 1 ) < 0 )
				return -1;

			add_objectNum(name, MaxInstNum);
			return 0;
		}
	case eCWMP_tADDOBJ:
		{
			int ret;
			unsigned int ifindex=0;
			unsigned int *target = NULL;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			ifindex = getNewIfIndex(CHANNEL_MODE_BRIDGE, 0, MEDIA_ATM, -1);  //default: bridge mode
			//wt-121v8 2.31, fail due to reaching the limit, return 9004
			if( ifindex==NA_VC ) return ERR_9004; //Maximum number of VC exceeds

			CWMPDBG( 1, ( stderr, "<%s:%d>addobj, ifindex:0x%08x\n", __FUNCTION__, __LINE__, ifindex ) );

			target = data;
			*target = 1 + findMaxConDevInstNum(MEDIA_ATM) + 1;
			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tATMLinkObject, sizeof(tATMLinkObject), data );

			if( ret >= 0 )
			{
				MIB_CE_ATM_VC_T entry;
				memset( &entry, 0, sizeof( MIB_CE_ATM_VC_T ) );
				{ //default values for this new entry
					entry.enable = 0;
					entry.qos=0;
					entry.pcr=ATM_MAX_US_PCR;
					entry.encap=1;//LLC
					entry.mtu=1500;
					entry.ifIndex=ifindex;
					entry.ConDevInstNum=*target - 1;
#ifdef CONFIG_E8B
					entry.applicationtype=X_CT_SRV_INTERNET;
#ifdef _PRMT_X_CT_COM_WANEXT_
					entry.ServiceList=X_CT_SRV_INTERNET;
#endif
#endif
#ifdef CONFIG_USER_WT_146
					wt146_set_default_config( &entry );
#endif //CONFIG_USER_WT_146
#ifdef CONFIG_IPV6
					entry.IpProtocol=IPVER_IPV4;//ipv4 as default
#endif /*CONFIG_IPV6*/
				}
				mib_chain_add( MIB_ATM_VC_TBL, (unsigned char*)&entry );
				notify_set_wan_changed();
				update_Parameter();	//Add other objects like PPP, IP, ...
			}
			return ret;
		}
	case eCWMP_tDELOBJ:
		{
	     	int ret;
			unsigned int instNum = 0;
			MIB_CE_ATM_VC_T pvcentry;
			char ifname[IFNAMSIZ];
			int id;
			
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			instNum = *(unsigned int*)data;
			
			if(get_wan_info_tr181(instNum, &pvcentry, &id, ifname) == 0
				|| MEDIA_INDEX(pvcentry.ifIndex) != MEDIA_ATM)
				return ERR_9003;

			mib_chain_delete( MIB_ATM_VC_TBL, id);
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, instNum );

			//Delete other objects like PPP, IP, ...
			update_Parameter();

			if(ret == 0)
				ret =1;	//setting not applied

			return ret;
	     }
	case eCWMP_tUPDATEOBJ:
		{
			int num,i;
			struct CWMP_LINKNODE *old_table;
			int has_new=0;

			num = mib_chain_total( MIB_ATM_VC_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=0; i < num ; i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				MIB_CE_ATM_VC_T *p,vc_entity;

				p = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
					continue;
				
				if(MEDIA_INDEX(p->ifIndex) != MEDIA_ATM)
					continue;

				remove_entity = remove_SiblingEntity( &old_table, p->ConDevInstNum + 1 );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, p->ConDevInstNum + 1 )==NULL )
					{
						unsigned int MaxInstNum = p->ConDevInstNum + 1;

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tATMLinkObject, sizeof(tATMLinkObject), &MaxInstNum );
						has_new=1;
					}//else already in next_table
				}
			}

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

int getATM(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "LinkNumberOfEntries" )==0 )
	{
		*data = uintdup(getATMLinkCnt());
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}



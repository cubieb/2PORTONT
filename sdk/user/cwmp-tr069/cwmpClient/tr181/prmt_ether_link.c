#include <stdio.h>
#include <stdlib.h>

#include <linux/ethtool.h>
#include <rtk/mib.h>
#include <rtk/utility.h>

#include <parameter_api.h>

#include "prmt_ether_if.h"
#include "prmt_ether_link.h"

/****** Device.Ethernet.Link.{i}.Stats ***************************************/
struct CWMP_OP tEtherLinkStatsLeafOP = { getEtherLinkStats, NULL};

struct CWMP_PRMT tEtherLinkStatsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"BytesSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"BytesReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"PacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"PacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"ErrorsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"ErrorsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"UnicastPacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"UnicastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"DiscardPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"DiscardPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"MulticastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"MulticastPacketsReceived",eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"BroadcastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherLinkStatsLeafOP},
{"BroadcastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tEtherLinkStatsLeafOP},
{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEtherLinkStatsLeafOP},
};

enum eEtherLinkStatsLeaf
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
};

struct CWMP_LEAF tEtherLinkStatsLeaf[] =
{
{ &tEtherLinkStatsLeafInfo[eLinkStatsBytesSent]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsBytesReceived]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsPacketsSent]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsPacketsReceived]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsErrorsSent]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsErrorsReceived]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsUnicastPacketsSent]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsUnicastPacketsReceived]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsDiscardPacketsSent]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsDiscardPacketsReceived]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsMulticastPacketsSent]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsMulticastPacketsReceived]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsBroadcastPacketsSent]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsBroadcastPacketsReceived]  },
{ &tEtherLinkStatsLeafInfo[eLinkStatsUnknownProtoPacketsReceived]  },
{ NULL	}
};

/****** Device.Ethernet.Link.{i} *********************************************/
struct CWMP_OP tEtherLinkEntityLeafOP = { getEtherLinkEntity, setEtherLinkEntity};

//Note: Only for WAN ETH Links
#if 0
struct node *gDummyEthLinkObjList = NULL;
struct DummyEthLinkObj
{
	char *LowerLayers;
};
#endif

struct CWMP_PRMT tEtherLinkEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tEtherLinkEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tEtherLinkEntityLeafOP},
/*{"Alias",			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tEtherLinkEntityLeafOP},*/
{"Name",			eCWMP_tSTRING,	CWMP_READ,		&tEtherLinkEntityLeafOP},
{"LastChange",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEtherLinkEntityLeafOP},
{"LowerLayers",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tEtherLinkEntityLeafOP},
{"MACAddress",		eCWMP_tSTRING,	CWMP_READ,		&tEtherLinkEntityLeafOP},
};

enum eEtherLinkEntityLeaf
{
	eLinkEntityEnable,
	eLinkEntityStatus,
//	eLinkEntityAlias,
	eLinkEntityName,
	eLinkEntityLastChange,
	eLinkEntityLowerLayers,
	eLinkEntityMACAddress,
};

struct CWMP_LEAF tEtherLinkEntityLeaf[] =
{
{ &tEtherLinkEntityLeafInfo[eLinkEntityEnable]  },
{ &tEtherLinkEntityLeafInfo[eLinkEntityStatus]  },
//{ &tEtherLinkEntityLeafInfo[eLinkEntityAlias]  },
{ &tEtherLinkEntityLeafInfo[eLinkEntityName]  },
{ &tEtherLinkEntityLeafInfo[eLinkEntityLastChange]  },
{ &tEtherLinkEntityLeafInfo[eLinkEntityLowerLayers]  },
{ &tEtherLinkEntityLeafInfo[eLinkEntityMACAddress]  },
{ NULL	}
};

struct CWMP_PRMT tEtherLinkEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eEtherLinkEntityObject
{
	eEtherLinkEntityStats,
};

struct CWMP_NODE tEtherLinkEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tEtherLinkEntityObjectInfo[eEtherLinkEntityStats],		tEtherLinkStatsLeaf,	NULL},
{NULL,					NULL,			NULL}
};

/****** Device.Ethernet.Link *************************************************/
struct CWMP_PRMT tEtherLinkObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum eEtherLinkObject
{
	eEtherLink0,
};

struct CWMP_LINKNODE tEtherLinkObject[] =
{
/*info, 						leaf,				next,				sibling,	instnum)*/
{&tEtherLinkObjectInfo[eEtherLink0], tEtherLinkEntityLeaf, tEtherLinkEntityObject, NULL, 0},
};

/***** Utility Functions *****************************************************/
int inline get_eth_link_cnt()
{
	return mib_chain_total(MIB_ATM_VC_TBL) + 1;	// first one is br0(LAN)
}

static int get_eth_link_info(int num, MIB_CE_ATM_VC_T *pEntry, int *idx, char *ifname)
{
	int numofentries = mib_chain_total(MIB_ATM_VC_TBL);

	if(pEntry == NULL)
		return -1;

	if(num == 1)
	{
		strcpy(ifname, "br0");
		memset(pEntry, 0, sizeof(MIB_CE_ATM_VC_T));
		return 0;
	}
	else
	{
		int i;

		for( i = 0 ; i < numofentries ; i++ )
		{
			if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
				continue;

			if( num == pEntry->ConDevInstNum + 1)	//first 1 is for LAN
				break;
		}
		if(i == numofentries)
			goto error;

		if(ifGetName(pEntry->ifIndex, ifname, IFNAMSIZ) != 0) {
			*idx = i;
			return 0;	//OK
		}
	}

error:
	memset(pEntry, 0, sizeof(MIB_CE_ATM_VC_T));
	return -1;
}

/***** Operations ************************************************************/
int getEtherLinkStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int	chainidx;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	ifname[16];
	struct net_device_stats nds = {0};
	int		iserror=0;
	struct ethtool_stats *stats = NULL;
	int linknum;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	linknum = getInstNum(name, "Ethernet.Link");
	if(linknum == 0) return ERR_9005;

	pEntry = &vc_entity;
	if(get_eth_link_info(linknum, pEntry, &chainidx, ifname) < 0)
		return ERR_9005;

	if(get_net_device_stats(ifname, &nds) < 0)
		iserror=1;

	stats = ethtool_gstats(ifname);

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "BytesSent" )==0 )
	{
		if(stats)
			*data = (void*)ulongdup(stats->data[TX_OCTETS]);
		else
			*data = (void*)ulongdup(nds.tx_bytes);
	}
	else if( strcmp( lastname, "BytesReceived" )==0 )
	{
		if(stats)
			*data = (void*)ulongdup(stats->data[RX_OCTETS]);
		else
			*data = (void*)ulongdup(nds.rx_bytes);
	}
	else if( strcmp( lastname, "PacketsSent" )==0 )
	{
		if(stats)
			*data = (void*)ulongdup(stats->data[TX_UCAST_PACKETS]
				+ stats->data[TX_MCAST_PACKETS]
				+ stats->data[TX_BCAST_PACKETS]);
		else
			*data = (void*)ulongdup(nds.tx_packets);
	}
	else if( strcmp( lastname, "PacketsReceived" )==0 )
	{
		if(stats)
			*data = (void*)ulongdup(stats->data[RX_UCAST_PACKETS]
				+ stats->data[RX_MCAST_PACKETS]
				+ stats->data[RX_BCAST_PACKETS]);
		else
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
		*data = (void*)ulongdup(stats ? stats->data[TX_UCAST_PACKETS] : 0);
	}
	else if( strcmp( lastname, "UnicastPacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(stats ? stats->data[RX_UCAST_PACKETS] : 0);
	}
	else if( strcmp( lastname, "DiscardPacketsSent" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_DISCARDS] : nds.tx_dropped);
	}
	else if( strcmp( lastname, "DiscardPacketsReceived" )==0 )
	{
		*data = uintdup(stats ? stats->data[TX_DISCARDS] : nds.rx_dropped);
	}
	else if( strcmp( lastname, "MulticastPacketsSent" )==0 )
	{
		*data = (void*)ulongdup(stats ? stats->data[TX_MCAST_PACKETS] : 0);
	}
	else if( strcmp( lastname, "MulticastPacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(stats ? stats->data[RX_MCAST_PACKETS] : 0);
	}
	else if( strcmp( lastname, "BroadcastPacketsSent" )==0 )
	{
		*data = (void *)ulongdup(stats ? stats->data[TX_BCAST_PACKETS] : 0);
	}
	else if( strcmp( lastname, "BroadcastPacketsReceived" )==0 )
	{
		*data = (void *)ulongdup(stats ? stats->data[RX_BCAST_PACKETS] : 0);
	}
	else if( strcmp( lastname, "UnknownProtoPacketsReceived" )==0 )
	{
		*data = uintdup(0);
	}
	else
	{
		if(stats)
			free(stats);
		return ERR_9005;
	}

	if(stats)
		free(stats);
	return 0;
}

int getEtherLinkEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int linknum,total;
	int	chainidx;
	MIB_CE_ATM_VC_T *pEntry = NULL;
	MIB_CE_ATM_VC_T vc_entity = {0};
	int is_br0 = 0;
	char ifname[IFNAMSIZ] = {0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	linknum = getInstNum(name, "Ethernet.Link");
	if(linknum == 0) return ERR_9005;

	pEntry = &vc_entity;
	if(get_eth_link_info(linknum, pEntry, &chainidx, ifname) < 0)
		return ERR_9005;

	if(strcmp(ifname, "br0") == 0)
		is_br0 = 1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		if(is_br0 || pEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int 	flags;

		if(!is_br0 && pEntry->enable==0)
			*data = strdup( "Down" );
		else if(getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_RUNNING)
				*data = strdup( "Up" );
			else
				*data = strdup( "Down" );
		}else
			*data = strdup( "Unknown" );
	}
	//TODO
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		*data = strdup("Up");
	}*/
	else if( strcmp( lastname, "Name" )==0 )
	{
		*data = strdup(ifname);
	}
	//TODO
	else if( strcmp( lastname, "LastChange" )==0 )
	{
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		char lowerLayers[1024] = {0};
		if(is_br0)
			*data = strdup("Device.Bridging.Bridge.1.Port.1");
		else
		{
			switch(MEDIA_INDEX(pEntry->ifIndex))
			{
			case MEDIA_ATM:
				sprintf(lowerLayers, "Device.ATM.Link.%d", pEntry->ConDevInstNum + 1);
				break;
			case MEDIA_ETH:
				sprintf(lowerLayers, "Device.Ethernet.Interface.%d", pEntry->ConDevInstNum + CWMP_ETH_LAN_IF_NUM);
				break;
			case MEDIA_PTM:
				//TODO: Check is correct or not
				sprintf(lowerLayers, "Device.PTM.Link.%d", pEntry->ConDevInstNum + 1);
				break;
			default:
				break;
			}
			*data = strdup(lowerLayers);
		}
	}
	else if( strcmp( lastname, "MACAddress" )==0 )
	{
		char macadd[18] = {0};
		struct sockaddr sa;
		
		if(!getInAddr(ifname, HW_ADDR, (void *)&sa))
		{
			*data = strdup("");
		}
		else
		{
			sprintf(macadd, "%02x:%02x:%02x:%02x:%02x:%02x", sa.sa_data[0]&0xff, sa.sa_data[1]&0xff, sa.sa_data[2]&0xff, sa.sa_data[3]&0xff, sa.sa_data[4]&0xff, sa.sa_data[5]&0xff);
			*data = strdup(macadd);
		}
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setEtherLinkEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int linknum,total,i;
	MIB_CE_ATM_VC_T *pEntry = NULL;
	MIB_CE_ATM_VC_T vc_entity = {0};
	int is_br0 = 0;
	char ifname[IFNAMSIZ] = {0};

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	linknum = getInstNum(name, "Ethernet.Link");
	if(linknum == 0) return ERR_9005;

	pEntry = &vc_entity;
	if(get_eth_link_info(linknum, pEntry, &i, ifname) < 0)
		return ERR_9005;

	if(strcmp(ifname, "br0") == 0)
		is_br0 = 1;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *v = data;

		if(is_br0)
		{
			if(*v != 1)
				return ERR_9001;
		}
		else
		{
			if(*v)
				pEntry->enable = 1;
			else
				pEntry->enable = 0;

			mib_chain_update(MIB_ATM_VC_TBL, pEntry, i);
		}
		gWanBitMap |= (1U << i);

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
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objEtherLink(char *name, struct CWMP_LEAF *e, int type, void *data)
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

			MaxInstNum = get_eth_link_cnt();
				
			if( create_Object( c, tEtherLinkObject, sizeof(tEtherLinkObject), MaxInstNum, 1 ) < 0 )
				return -1;

			add_objectNum(name, MaxInstNum);

			return 0;
		}
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
		{
			int total,i;
			struct CWMP_LINKNODE *old_table;
			int has_new=0;
			struct CWMP_LINKNODE *remove_entity=NULL;
			MIB_CE_ATM_VC_T *p,vc_entity;

			total = mib_chain_total( MIB_ATM_VC_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			p = &vc_entity;

			//Move LAN entity to new list
			remove_entity = remove_SiblingEntity( &old_table, 1 );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}
			else
			{
				unsigned int MaxInstNum = 1;
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tEtherLinkObject, sizeof(tEtherLinkObject), &MaxInstNum );
			}
			
			for( i=0 ; i < total ; i++ )
			{
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
					continue;
				
				remove_entity = remove_SiblingEntity( &old_table, p->ConDevInstNum + 1);
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, p->ConDevInstNum + 1 )==NULL )
					{
						unsigned int MaxInstNum = p->ConDevInstNum + 1;

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tEtherLinkObject, sizeof(tEtherLinkObject), &MaxInstNum );
						has_new = 1;
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


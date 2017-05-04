#include <stdio.h>
#include <stdlib.h>

#include <linux/ethtool.h>
#include <linux/atm.h>
#include <rtk/mib.h>
#include <rtk/utility.h>

#include <parameter_api.h>
#include <cwmp_utility.h>

#include "prmt_ptm.h"
/****** Device.PTM.Link.{i}.Stats ********************************************/
struct CWMP_OP tPTMLinkStatsLeafOP = { getPTMLinkStats, NULL};

struct CWMP_PRMT tPTMLinkStatsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"BytesSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"BytesReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"PacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"PacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"ErrorsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"ErrorsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"UnicastPacketsSent",			eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"UnicastPacketsReceived",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"DiscardPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"DiscardPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"MulticastPacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"MulticastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"BroadcastPacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"BroadcastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkStatsLeafOP},
};

enum ePTMLinkStatsLeaf
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

struct CWMP_LEAF tPTMLinkStatsLeaf[] =
{
{ &tPTMLinkStatsLeafInfo[eLinkStatsBytesSent]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsBytesReceived]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsPacketsSent]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsPacketsReceived]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsErrorsSent]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsErrorsReceived]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsUnicastPacketsSent]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsUnicastPacketsReceived]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsDiscardPacketsSent]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsDiscardPacketsReceived]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsMulticastPacketsSent]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsMulticastPacketsReceived]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsBroadcastPacketsSent]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsBroadcastPacketsReceived]  },
{ &tPTMLinkStatsLeafInfo[eLinkStatsUnknownProtoPacketsReceived]  },
{ NULL	}
};

/****** Device.PTM.Link.{i} **************************************************/
struct CWMP_OP tPTMLinkEntityLeafOP = { getPTMLinkEntity, setPTMLinkEntity};

struct CWMP_PRMT tPTMLinkEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tPTMLinkEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tPTMLinkEntityLeafOP},
/*{"Alias",			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tPTMLinkEntityLeafOP},*/
{"Name",			eCWMP_tSTRING,	CWMP_READ,		&tPTMLinkEntityLeafOP},
{"LastChange",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tPTMLinkEntityLeafOP},
{"LowerLayers",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tPTMLinkEntityLeafOP},
{"MACAddress",		eCWMP_tSTRING,	CWMP_READ,		&tPTMLinkEntityLeafOP},
};

enum ePTMLinkEntityLeaf
{
	eLinkEntityEnable,
	eLinkEntityStatus,
//	eLinkEntityAlias,
	eLinkEntityName,
	eLinkEntityLastChange,
	eLinkEntityLowerLayers,
	eLinkEntityMACAddress,
};

struct CWMP_LEAF tPTMLinkEntityLeaf[] =
{
{ &tPTMLinkEntityLeafInfo[eLinkEntityEnable]  },
{ &tPTMLinkEntityLeafInfo[eLinkEntityStatus]  },
//{ &tPTMLinkEntityLeafInfo[eLinkEntityAlias]  },
{ &tPTMLinkEntityLeafInfo[eLinkEntityName]  },
{ &tPTMLinkEntityLeafInfo[eLinkEntityLastChange]  },
{ &tPTMLinkEntityLeafInfo[eLinkEntityLowerLayers]  },
{ &tPTMLinkEntityLeafInfo[eLinkEntityMACAddress]  },
{ NULL	}
};

struct CWMP_PRMT tPTMLinkEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum ePTMLinkEntityObject
{
	ePTMLinkEntityStats,
};

struct CWMP_NODE tPTMLinkEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tPTMLinkEntityObjectInfo[ePTMLinkEntityStats],		tPTMLinkStatsLeaf,	NULL},
{NULL,					NULL,			NULL}
};

/****** Device.PTM.Link ******************************************************/
struct CWMP_PRMT tPTMLinkObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum ePTMLinkObject
{
	ePTMLink0,
};

struct CWMP_LINKNODE tPTMLinkObject[] =
{
/*info, 						leaf,				next,				sibling,	instnum)*/
{&tPTMLinkObjectInfo[ePTMLink0], tPTMLinkEntityLeaf, tPTMLinkEntityObject, NULL, 0},
};

/****** Device.PTM ***********************************************************/
struct CWMP_OP tPTMLeafOP = { getPTM, NULL};
struct CWMP_OP tPTMLinkObjectOP = { NULL, objPTMLink};

struct CWMP_PRMT tPTMLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"LinkNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tPTMLeafOP},
};
enum ePTMLeaf
{
	eLinkNumberOfEntries,
};

struct CWMP_LEAF tPTMLeaf[] =
{
{ &tPTMLeafInfo[eLinkNumberOfEntries]  },
{ NULL	}
};


struct CWMP_PRMT tPTMObjectInfo[] =
{
/*(name,	type,			flag,					op)*/
{"Link",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tPTMLinkObjectOP},
};
enum ePTMObject
{
	ePTMLink,
};

struct CWMP_NODE tPTMObject[] =
{
/*info,  				leaf,			next)*/
{&tPTMObjectInfo[ePTMLink],		NULL,	NULL},
{NULL,					NULL,			NULL}
};

/***** Utility Functions *****************************************************/
int getPTMLinkCnt()
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

		if(MEDIA_INDEX(vc_entity.ifIndex) == MEDIA_PTM)
			totalwancon++;
	}
	return totalwancon;
}

/***** Operations ************************************************************/
int getPTMLinkStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int	chainidx;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	char	ifname[16];
	struct net_device_stats nds = {0};
	struct ethtool_stats *stats = NULL;
	int linknum;
	int i;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	linknum = getInstNum(name, "PTM.Link");
	if(linknum == 0) return ERR_9005;

	pEntry = &vc_entity;
	linknum = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<linknum;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(MEDIA_INDEX(pEntry->ifIndex) != MEDIA_PTM)
			continue;

		if( pEntry->ConDevInstNum + 1 == linknum )
			break;
	}
	if(i==linknum) return ERR_9005;

	if( ifGetName(pEntry->ifIndex, ifname, sizeof(ifname)) )
		get_net_device_stats(ifname, &nds);

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

int getPTMLinkEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int linknum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	linknum = getInstNum(name, "PTM.Link");
	if(linknum == 0) return ERR_9005;

	pEntry = &vc_entity;
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(MEDIA_INDEX(pEntry->ifIndex) != MEDIA_PTM)
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

		if(ifGetName(PHY_INTF(pEntry->ifIndex), ifname, sizeof(ifname)) == 0)
			return ERR_9002;

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

		if(ifGetName(PHY_INTF(pEntry->ifIndex), ifname, sizeof(ifname)) == 0)
				return ERR_9002;

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
	else if( strcmp( lastname, "MACAddress" )==0 )
	{
		char macadd[18] = {0};
		struct sockaddr sa;
		char	ifname[16];

		if(ifGetName(PHY_INTF(pEntry->ifIndex), ifname, sizeof(ifname)) == 0)
			return ERR_9002;
		
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

int setPTMLinkEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf=data;
	char tmp[256]="";
	unsigned int linknum,num,i;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	linknum = getInstNum(name, "PTM.Link");
	if(linknum == 0) return ERR_9005;

	pEntry = &vc_entity;
	num = mib_chain_total( MIB_ATM_VC_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
			continue;

		if(MEDIA_INDEX(pEntry->ifIndex) != MEDIA_PTM)
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
			return 0;

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
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objPTMLink(char *name, struct CWMP_LEAF *e, int type, void *data)
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

			MaxInstNum = getPTMLinkCnt();
				
			if( create_Object( c, tPTMLinkObject, sizeof(tPTMLinkObject), MaxInstNum, 1 ) < 0 )
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

			ifindex = getNewIfIndex(CHANNEL_MODE_BRIDGE, 0, MEDIA_PTM, -1);  //default: bridge mode
			//wt-121v8 2.31, fail due to reaching the limit, return 9004
			if( ifindex==NA_VC ) return ERR_9004; //Maximum number of VC exceeds

			CWMPDBG( 1, ( stderr, "<%s:%d>addobj, ifindex:0x%08x\n", __FUNCTION__, __LINE__, ifindex ) );

			target = data;
			//(max instance number + 1) is the real max instance number for TR-181
			*target = 1 + findMaxConDevInstNum(MEDIA_PTM) + 1;
			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tPTMLinkObject, sizeof(tPTMLinkObject), data );

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
				//Automatically create other objects like PPP, IP, ...
				update_Parameter();
				notify_set_wan_changed();
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
				|| MEDIA_INDEX(pvcentry.ifIndex) != MEDIA_PTM)
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
				
				if(MEDIA_INDEX(p->ifIndex) != MEDIA_PTM)
					continue;

				remove_entity = remove_SiblingEntity( &old_table, p->ConDevInstNum + 1 );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, p->ConDevInstNum + 1)==NULL )
					{
						unsigned int MaxInstNum = p->ConDevInstNum + 1;

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tPTMLinkObject, sizeof(tPTMLinkObject), &MaxInstNum );
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

int getPTM(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "LinkNumberOfEntries" )==0 )
	{
		*data = uintdup(getPTMLinkCnt());
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}




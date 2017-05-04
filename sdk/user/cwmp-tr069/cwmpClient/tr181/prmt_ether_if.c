/**
* Note:
* 1. LAN interface: {i} is one of 1 to CWMP_ETH_LAN_IF_NUM.
* 2. WAN interface: {i} is its conDevInstNum + CWMP_ETH_LAN_IF_NUM. The last one
*    is the dummy object. Enable it to create new Ethernet WAN connection.
*/

#include <stdio.h>
#include <stdlib.h>

#include <linux/ethtool.h>
#include <rtk/mib.h>
#include <rtk/utility.h>

#include <parameter_api.h>

#include "prmt_ether_if.h"
/****** Device.Ethernet.Interface.{i}.Stats **********************************/
struct CWMP_OP tEtherIfStatsLeafOP = { getEtherIfStats, NULL};

struct CWMP_PRMT tEtherIfStatsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"BytesSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"BytesReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"PacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"PacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"ErrorsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"ErrorsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"UnicastPacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"UnicastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"DiscardPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"DiscardPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"MulticastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"MulticastPacketsReceived",eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"BroadcastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tEtherIfStatsLeafOP},
{"BroadcastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tEtherIfStatsLeafOP},
{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tEtherIfStatsLeafOP},
};

enum eEtherIfStatsLeaf
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

struct CWMP_LEAF tEtherIfStatsLeaf[] =
{
{ &tEtherIfStatsLeafInfo[eStatsBytesSent]  },
{ &tEtherIfStatsLeafInfo[eStatsBytesReceived]  },
{ &tEtherIfStatsLeafInfo[eStatsPacketsSent]  },
{ &tEtherIfStatsLeafInfo[eStatsPacketsReceived]  },
{ &tEtherIfStatsLeafInfo[eStatsErrorsSent]  },
{ &tEtherIfStatsLeafInfo[eStatsErrorsReceived]  },
{ &tEtherIfStatsLeafInfo[eStatsUnicastPacketsSent]  },
{ &tEtherIfStatsLeafInfo[eStatsUnicastPacketsReceived]  },
{ &tEtherIfStatsLeafInfo[eStatsDiscardPacketsSent]  },
{ &tEtherIfStatsLeafInfo[eStatsDiscardPacketsReceived]  },
{ &tEtherIfStatsLeafInfo[eStatsMulticastPacketsSent]  },
{ &tEtherIfStatsLeafInfo[eStatsMulticastPacketsReceived]  },
{ &tEtherIfStatsLeafInfo[eStatsBroadcastPacketsSent]  },
{ &tEtherIfStatsLeafInfo[eStatsBroadcastPacketsReceived]  },
{ &tEtherIfStatsLeafInfo[eStatsUnknownProtoPacketsReceived]  },
{ NULL	}
};

/****** Device.Ethernet.Interface.{i} ****************************************/
struct CWMP_OP tEtherIfEntityLeafOP = { getEtherIfEntity, setEtherIfEntity };

struct CWMP_PRMT tEtherIfEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tEtherIfEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,		&tEtherIfEntityLeafOP},
/*{"Alias",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tEtherIfEntityLeafOP},*/
{"Name",		eCWMP_tSTRING,	CWMP_READ,		&tEtherIfEntityLeafOP},
{"LastChange",	eCWMP_tUINT,	CWMP_READ,		&tEtherIfEntityLeafOP},
{"LowerLayers",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tEtherIfEntityLeafOP},
{"Upstream",	eCWMP_tBOOLEAN,	CWMP_READ,		&tEtherIfEntityLeafOP},
{"MACAddress",	eCWMP_tSTRING,	CWMP_READ,		&tEtherIfEntityLeafOP},
{"MaxBitRate",	eCWMP_tINT,		CWMP_READ|CWMP_WRITE,	&tEtherIfEntityLeafOP},
{"DuplexMode",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tEtherIfEntityLeafOP},
};

enum eEtherIfEntityLeaf
{
	eIfEntityEnable,
	eIfEntityStatus,
//	eIfEntityAlias,
	eIfEntityName,
	eIfEntityLastChange,
	eIfEntityLowerLayers,
	eIfEntityUpstream,
	eIfEntityMACAddress,
	eIfEntityMaxBitRate,
	eIfEntityDuplexMode,
};

struct CWMP_LEAF tEtherIfEntityLeaf[] =
{
{ &tEtherIfEntityLeafInfo[eIfEntityEnable]  },
{ &tEtherIfEntityLeafInfo[eIfEntityStatus]  },
//{ &tEtherIfEntityLeafInfo[eIfEntityAlias]  },
{ &tEtherIfEntityLeafInfo[eIfEntityName]  },
{ &tEtherIfEntityLeafInfo[eIfEntityLastChange]  },
{ &tEtherIfEntityLeafInfo[eIfEntityLowerLayers]  },
{ &tEtherIfEntityLeafInfo[eIfEntityUpstream]  },
{ &tEtherIfEntityLeafInfo[eIfEntityMACAddress]  },
{ &tEtherIfEntityLeafInfo[eIfEntityMaxBitRate]  },
{ &tEtherIfEntityLeafInfo[eIfEntityDuplexMode]  },
{ NULL	}
};

struct CWMP_PRMT tEtherIfEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eEtherIfEntityObject
{
	eEtherIfEntityStats,
};

struct CWMP_NODE tEtherIfEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tEtherIfEntityObjectInfo[eEtherIfEntityStats],		tEtherIfStatsLeaf,	NULL},
{NULL,					NULL,			NULL}
};

/****** Device.Ethernet.Interface *********************************************/
struct CWMP_PRMT tEtherIfObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum eEtherIfObject
{
	eEtherIf0,
};

struct CWMP_LINKNODE tEtherIfObject[] =
{
/*info, 						leaf,				next,				sibling,	instnum)*/
{&tEtherIfObjectInfo[eEtherIf0], tEtherIfEntityLeaf, tEtherIfEntityObject, NULL, 0},
};

/***** Utility Functions *****************************************************/
int get_eth_if_cnt()
{
#ifdef CONFIG_ETHWAN
	int numofentries = mib_chain_total(MIB_ATM_VC_TBL);
	unsigned int cnt = CWMP_ETH_LAN_IF_NUM;
	MIB_CE_ATM_VC_T vc_entity = {0};
	int i;

	for( i = 0; i < numofentries; i++ )
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vc_entity))
		{
			CWMPDBP(1, "Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)", i);
			return ERR_9002;
		}

		if(MEDIA_INDEX(vc_entity.ifIndex) == MEDIA_ETH)
			cnt++;
	}
	return cnt + 1;	//The last 1 is dummy ETH WAN object
#else
	return CWMP_ETH_LAN_IF_NUM;
#endif
}

static inline int is_eth_wan(int instNum)
{
#ifdef CONFIG_ETHWAN
	if(instNum > CWMP_ETH_LAN_IF_NUM)
		return 1;
	else
#endif
		return 0;
}

static inline int is_eth_lan(int instNum)
{
	if(instNum <= CWMP_ETH_LAN_IF_NUM)
		return 1;
	else
		return 0;
}

int is_ethif_enabled(int instNum)
{
#ifdef CONFIG_ETHWAN
	if(is_eth_wan(instNum))
	{
		int numofentries = mib_chain_total(MIB_ATM_VC_TBL);
		MIB_CE_ATM_VC_T vc_entity = {0};
		int i;

		for( i = 0; i < numofentries; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vc_entity))
			{
				CWMPDBP(1, "Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)", i);
				return -1;
			}

			if(vc_entity.enable
				&& MEDIA_INDEX(vc_entity.ifIndex) == MEDIA_ETH
				&& vc_entity.ConDevInstNum== instNum - CWMP_ETH_LAN_IF_NUM)
					break;
		}
		if(i < numofentries)
			return 1;
		else
			return 0;
	}
#endif
	//ELAN interfaces
	return 1;
}

// Use Device.Ethernet.Interface.<instNum> to get interface name
static int get_eth_name(int instNum, char *buf, int len)
{
	if(buf == NULL)
		return -1;

	if(instNum < 1)
		return ERR_9005;

#ifdef CONFIG_ETHWAN
	if(is_eth_wan(instNum))
	{
		int numofentries = mib_chain_total(MIB_ATM_VC_TBL);
		MIB_CE_ATM_VC_T vc_entity = {0};
		int i;

		for( i = 0; i < numofentries; i++ )
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vc_entity))
			{
				CWMPDBP(1, "Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)", i);
				continue;
			}

			if(MEDIA_INDEX(vc_entity.ifIndex) == MEDIA_ETH
				&& instNum == vc_entity.ConDevInstNum + CWMP_ETH_LAN_IF_NUM)
				break;
		}
		if(i == numofentries)
		{
			if(instNum == findMaxConDevInstNum(MEDIA_ETH) + 1 + CWMP_ETH_LAN_IF_NUM)
				vc_entity.ifIndex = getNewIfIndex(CHANNEL_MODE_BRIDGE, 0, MEDIA_ETH, -1);	//dummy Ethernet Interface
			else
				return ERR_9005;
		}
		ifGetName(PHY_INTF(vc_entity.ifIndex), buf, len); //nas0_x
	}
	else
#endif
	{
#ifdef CONFIG_RTL_MULTI_LAN_DEV
		snprintf(buf, len, "%s%d", ALIASNAME_ELAN_PREFIX, instNum + 1); //eth0.x, start from eth0.2
#else
		snprintf(buf, len, "%s0", ALIASNAME_ETH);	//eth0
#endif
	}

	return 0;
}

struct CWMP_LINKNODE *get_parent_obj(char *name)
{
	char parent_name[MAX_PRMT_NAME_LEN] = {0};
	int dot = 0;
	struct CWMP_LINKNODE *parent = NULL;

	strcpy(parent_name, name);
	dot = strlen(parent_name);
	while(dot >=0 && parent_name[dot] != '.')
		dot--;

	if(dot == 0)
		return NULL;

	parent_name[dot+1] = '\0';
	if(get_ParameterEntity(parent_name, (struct CWMP_LEAF **)&parent) != 0)
		return NULL;
	else
		return parent;
}

static int new_eth_wan()
{
	MIB_CE_ATM_VC_T entry = {0};
	unsigned int ifindex;

	ifindex = getNewIfIndex(CHANNEL_MODE_BRIDGE, 0, MEDIA_ETH , -1);  //default: bridge mode
	if( ifindex==NA_VC )
		return -1; //Maximum number of VC exceeds

	//default values for this new entry
	entry.enable=1;
	entry.qos=0;
	entry.pcr=ATM_MAX_US_PCR;
	entry.encap=1;//LLC
	entry.mtu=1500;
	entry.ifIndex=ifindex;
	entry.connDisable=0; //0:enable, 1:disable
	entry.ConDevInstNum=1 + findMaxConDevInstNum(MEDIA_ETH);
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

	mib_chain_add( MIB_ATM_VC_TBL, (unsigned char*)&entry );

	update_Parameter();	//Add other objects like PPP, IP, ...
	return 0;
}

static int remove_eth_wan(int ifnum)
{
	int numofentries = mib_chain_total(MIB_ATM_VC_TBL);
	MIB_CE_ATM_VC_T vc_entity = {0};
	int i;

	for( i = 0; i < numofentries; i++ )
	{
		if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vc_entity))
		{
			CWMPDBP(1, "Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)\n", i);
			return -1;
		}

		if(vc_entity.enable
			&& MEDIA_INDEX(vc_entity.ifIndex) == MEDIA_ETH
			&& ifnum == vc_entity.ConDevInstNum + CWMP_ETH_LAN_IF_NUM)
		{
			mib_chain_delete(MIB_ATM_VC_TBL, i);
			update_Parameter();	//Delete other objects like PPP, IP, ...
			return 0;
		}
	}
	CWMPDBP(1, "Cannot find target ETH WAN\n", i);
	return -1;
}

/***** Operations ************************************************************/
int getEtherIfStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int	chainidx;
	char	ifname[16];
	struct net_device_stats nds;
	int		iserror=0;
	struct ethtool_stats *stats = NULL;
	int ifnum = -1, ret;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	ifnum = getInstNum(name, "Interface");

	ret = get_eth_name(ifnum, ifname, sizeof(ifname));
	if(ret == 0)
	{
		if(get_net_device_stats(ifname, &nds) < 0)
			iserror=1;
		stats = ethtool_gstats(ifname);
	}
	else if(ret == ERR_9005)
		return ret;
	else
		iserror=1;

	if(iserror)
		memset( &nds, 0, sizeof(nds) );

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

int getEtherIfEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int ifnum;
	char ifname[IFNAMSIZ] = {0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	ifnum = getInstNum(name, "Interface");

	if(get_eth_name(ifnum, ifname, sizeof(ifname)) < 0)
	{
		return ERR_9005;
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(is_ethif_enabled(ifnum));
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int 	flags;

		if(getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_RUNNING)
				*data = strdup( "Up" );
			else
				*data = strdup( "Down" );
		}
		else
			*data = strdup( "Down" );
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
		// OSI L1 has no lower layers
		*data = strdup("");
	}
	else if( strcmp( lastname, "Upstream" )==0 )
	{
		if(is_eth_wan(ifnum))
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "MACAddress" )==0 )
	{
		char macadd[64] = {0};
		struct sockaddr sa = {0};

		if(is_ethif_enabled(ifnum) && getInAddr(ifname, HW_ADDR, (void *)&sa) == 1)
		{
			sprintf(macadd, "%02x:%02x:%02x:%02x:%02x:%02x", sa.sa_data[0]&0xff, sa.sa_data[1]&0xff, sa.sa_data[2]&0xff, sa.sa_data[3]&0xff, sa.sa_data[4]&0xff, sa.sa_data[5]&0xff);
			*data = strdup(macadd);
		}
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		struct net_link_info netlink_info = {0};

		if(get_net_link_info(ifname, &netlink_info) == 0)
		{
			if(netlink_info.speed <= 0)
				intdup(-1);	//AUTO
			else
				*data = intdup(netlink_info.speed);
		}
		else
			intdup(-1);	//AUTO
	}
	else if( strcmp( lastname, "DuplexMode" )==0 )
	{
		struct net_link_info netlink_info = {0};

		if(get_net_link_info(ifname, &netlink_info) == 0)
		{
			if(netlink_info.duplex == DUPLEX_HALF)
				*data = strdup("Half");
			else if(netlink_info.duplex == DUPLEX_FULL)
				*data = strdup("Full");
			else
				return ERR_9002;
		}
		else
			*data = strdup("Auto");
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setEtherIfEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	int ifnum = 0;
	char ifname[IFNAMSIZ] = {0};
	MIB_CE_ATM_VC_T entry = {0};
	MIB_CE_ATM_VC_T *p_entry = NULL;
	struct CWMP_LINKNODE *parent = NULL;


	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if(entity->info->type != type) return ERR_9006;

	ifnum = getInstNum(name, "Interface");

	parent = get_parent_obj(name);

	if(parent == NULL)
		return ERR_9005;

	gWanBitMap = UINT_MAX;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned int ifindex;

		if( i == NULL ) return ERR_9007;

		if( *i == 1 )
		{
			if(parent->is_dummy)
			{
				if(new_eth_wan() == -1)
				{
					CWMPDBG(1, (stderr, "<%s:%d> Create new ETH WAN failed.\n", __FUNCTION__, __LINE__));
					return ERR_9004;
				}
				parent->is_dummy = 0;
				return CWMP_NEED_RESTART_WAN;
			}
			else
				return 0;	//already enabled
		}
		else if( *i == 0 )
		{
			if(is_eth_lan(ifnum))
				return ERR_9001;

			if(parent->is_dummy)
				return 0;

			remove_eth_wan(ifnum);
			return CWMP_NEED_RESTART_WAN;
		}
		else
			return ERR_9003;
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		return ERR_9001;
	}*/
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		return ERR_9001;
	}
	else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		return ERR_9001;
	}
	else if( strcmp( lastname, "DuplexMode" )==0 )
	{
		return ERR_9001;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objEtherIf(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
			unsigned int num=0,MaxInstNum=0,i;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
			struct CWMP_LINKNODE *dummy = NULL;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			MaxInstNum = get_eth_if_cnt();

			if( create_Object( c, tEtherIfObject, sizeof(tEtherIfObject), MaxInstNum, 1 ) < 0 )
				return -1;

			add_objectNum(name, MaxInstNum);

			return 0;
		}
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
#ifdef CONFIG_ETHWAN
		{
			int total_wan, i;
			struct CWMP_LINKNODE *old_table;
			struct CWMP_LINKNODE *remove_entity = NULL;
			int has_new=0, add_dummy = 0;
			unsigned int MaxInstNum;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			//Move ETH LAN objects
			for( i = 1 ; i <= CWMP_ETH_LAN_IF_NUM ; i++)
			{
				remove_entity = remove_SiblingEntity( &old_table, i );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
			}

			//Update ETH WAN objects
			total_wan = mib_chain_total( MIB_ATM_VC_TBL );
			for( i=0; i<total_wan;i++ )
			{
				MIB_CE_ATM_VC_T *p,vc_entity;
				int instNum;

				p = &vc_entity;
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
					continue;

				if(MEDIA_INDEX(p->ifIndex) != MEDIA_ETH)
					continue;

				instNum = p->ConDevInstNum + CWMP_ETH_LAN_IF_NUM;

				remove_entity = remove_SiblingEntity( &old_table, instNum);
				if( remove_entity != NULL )
				{
					remove_entity->is_dummy = 0;
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, instNum )==NULL )
					{
						MaxInstNum = instNum;

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tEtherIfObject, sizeof(tEtherIfObject), &MaxInstNum );
						has_new=1;
					}//else already in next_table
				}
			}

			if(total_wan < 8)
			{
				struct CWMP_LINKNODE *dummy = NULL;

				// add a dummy eth interface
				MaxInstNum = 1 + findMaxConDevInstNum(MEDIA_ETH) + CWMP_ETH_LAN_IF_NUM;
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tEtherIfObject, sizeof(tEtherIfObject), &MaxInstNum );
				dummy = find_SiblingEntity((struct CWMP_LINKNODE **) &entity->next, MaxInstNum);
				dummy->is_dummy = 1;
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );

			if(has_new)
				notify_set_wan_changed();

			return 0;
		}
#else
		return 0;	//Only ELAN, no need to update
#endif
	}

	return -1;
}


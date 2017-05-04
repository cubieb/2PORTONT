#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/ethtool.h>

#include <parameter_api.h>

#include "../prmt_landevice_eth.h"
#include "../cwmpc_utility.h"
#include "prmt_bridge_port.h"
#include "prmt_ether_if.h"

/****** Device.Bridging.Bridge.{i}.Port.{i}.Stats ****************************/
struct CWMP_OP tBRPortStatsLeafOP = { getBRPortStats, NULL};

struct CWMP_PRMT tBRPortStatsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"BytesSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"BytesReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"PacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"PacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"ErrorsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"ErrorsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"UnicastPacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"UnicastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"DiscardPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"DiscardPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"MulticastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"MulticastPacketsReceived",eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"BroadcastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"BroadcastPacketsReceived",	eCWMP_tULONG, CWMP_READ|CWMP_DENY_ACT,		&tBRPortStatsLeafOP},
{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tBRPortStatsLeafOP},
};

enum eBRPortStatsLeaf
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

struct CWMP_LEAF tBRPortStatsLeaf[] =
{
{ &tBRPortStatsLeafInfo[eStatsBytesSent]  },
{ &tBRPortStatsLeafInfo[eStatsBytesReceived]  },
{ &tBRPortStatsLeafInfo[eStatsPacketsSent]  },
{ &tBRPortStatsLeafInfo[eStatsPacketsReceived]  },
{ &tBRPortStatsLeafInfo[eStatsErrorsSent]  },
{ &tBRPortStatsLeafInfo[eStatsErrorsReceived]  },
{ &tBRPortStatsLeafInfo[eStatsUnicastPacketsSent]  },
{ &tBRPortStatsLeafInfo[eStatsUnicastPacketsReceived]  },
{ &tBRPortStatsLeafInfo[eStatsDiscardPacketsSent]  },
{ &tBRPortStatsLeafInfo[eStatsDiscardPacketsReceived]  },
{ &tBRPortStatsLeafInfo[eStatsMulticastPacketsSent]  },
{ &tBRPortStatsLeafInfo[eStatsMulticastPacketsReceived]  },
{ &tBRPortStatsLeafInfo[eStatsBroadcastPacketsSent]  },
{ &tBRPortStatsLeafInfo[eStatsBroadcastPacketsReceived]  },
{ &tBRPortStatsLeafInfo[eStatsUnknownProtoPacketsReceived]  },
{ NULL	}
};

/****** Device.Bridging.Bridge.{i}.Port.{i} **********************************/
struct CWMP_OP tBRPortEntityLeafOP = { getBRPortEntity, setBRPortEntity};

struct CWMP_PRMT tBRPortEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBRPortEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,	&tBRPortEntityLeafOP},
//{"Alias",			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBRPortEntityLeafOP},
{"Name",			eCWMP_tSTRING,	CWMP_READ,	&tBRPortEntityLeafOP},
{"LastChange",		eCWMP_tUINT,	CWMP_READ|CWMP_WRITE|CWMP_DENY_ACT,	&tBRPortEntityLeafOP},
{"LowerLayers",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tBRPortEntityLeafOP},
{"ManagementPort",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBRPortEntityLeafOP},
{"PortState",		eCWMP_tSTRING,	CWMP_READ,	&tBRPortEntityLeafOP},
};

enum eBRPortEntityLeaf
{
	eBRPortEnable,
	eBRPortStatus,
	eBRPortAlias,
	eBRPortName,
	eBRPortLastChange,
	eBRPortLowerLayers,
	eBRPortManagementPort,
	eBRPortPortState,
};

struct CWMP_LEAF tBRPortEntityLeaf[] =
{
{ &tBRPortEntityLeafInfo[eBRPortEnable]  },
{ &tBRPortEntityLeafInfo[eBRPortStatus]  },
{ &tBRPortEntityLeafInfo[eBRPortAlias]  },
{ &tBRPortEntityLeafInfo[eBRPortName]  },
{ &tBRPortEntityLeafInfo[eBRPortLastChange]  },
{ &tBRPortEntityLeafInfo[eBRPortLowerLayers]  },
{ &tBRPortEntityLeafInfo[eBRPortManagementPort]  },
{ &tBRPortEntityLeafInfo[eBRPortPortState]  },
{ NULL	}
};

struct CWMP_PRMT tBRPortEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Stats",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
};

enum eBRPortEntityObject
{
	eBRPortStats,
};

struct CWMP_NODE tBRPortEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tBRPortEntityObjectInfo[eBRPortStats],			tBRPortStatsLeaf, NULL},
{NULL,					NULL,			NULL}
};

/****** Device.Bridging.Bridge.{i}.Port **************************************/
struct CWMP_PRMT tBRPortObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum eBridgingBridgeObject
{
	eBRPort0,
};

struct CWMP_LINKNODE BRPortObject[] =
{
/*info,  			leaf,			next,		sibling,		instnum)*/
{&tBRPortObjectInfo[eBRPort0],	tBRPortEntityLeaf,	tBRPortEntityObject,		NULL,			0},
};

/***** Utility Functions *****************************************************/
int inline get_br_port_cnt()
{
	//1 management port + ELAN ports + wlan ports + number of wan ports
	return 1 + CWMP_LANETHIFNO + WLAN_IF_NUM + mib_chain_total(MIB_ATM_VC_TBL);
}

enum BRPORT_TYPE
{
	PORT_MANAGE,
	PORT_ELAN,
	PORT_WLAN,
	PORT_WAN,
};
#define PORT_MANAGE_BASE 1
#define PORT_ELAN_BASE (PORT_MANAGE_BASE + 1)
#define PORT_WLAN_BASE (PORT_ELAN_BASE + CWMP_LANETHIFNO)
#define PORT_WAN_BASE (PORT_WLAN_BASE + WLAN_IF_NUM)

/**
 * Get Bridge.1.Port.<instNum>'s port type and the index(starts with 0) of this type
 */
static int get_brport_type(int port_num, unsigned char *type, int *id)
{
	if(type == NULL || id ==NULL || port_num < 1)
		return -1;

	if(port_num == 1)
	{
		*type = PORT_MANAGE;
		*id = 0;
	}
	else if(port_num >= PORT_ELAN_BASE && port_num < PORT_WLAN_BASE)
	{
		//ELAN
		*type = PORT_ELAN;
		*id = port_num - PORT_ELAN_BASE;
	}
	else if(port_num >= PORT_WLAN_BASE && port_num < PORT_WAN_BASE)
	{
		//WLAN
		*type = PORT_WLAN;
		*id = port_num - PORT_WLAN_BASE; 
	}
	else
	{
		//WAN, id is entry number in mib chain
		int wan_total = mib_chain_total(MIB_ATM_VC_TBL);
		MIB_CE_ATM_VC_T vc_entity;
		int i, cnt = 0;

		*type = PORT_WAN;

		for(i = 0 ; i < wan_total ; i++)
		{
			if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vc_entity))
				continue;

			if(port_num == vc_entity.ConDevInstNum + PORT_WAN_BASE - 1)
			{
				*id = i;
				return 0;
			}
		}
		return -1;
	}

	return 0;
}

static int get_port_ifname(int port_num, char *ifname, int size)
{
	int total = get_br_port_cnt();
	unsigned char type = 0;
	int idx = 0;

	if(port_num < 1)
		return -1;

	if(get_brport_type(port_num, &type, &idx) < 0)
		return -1;

	switch(type)
	{
	case PORT_MANAGE:
		strncpy(ifname, "br0", size);
		break;
	case PORT_ELAN:
		if(CWMP_LANETHIFNO == 1)
			strncpy(ifname, "eth0", size);
		else
			snprintf(ifname, size, "eth0.%d", idx + 2);
		break;
	case PORT_WLAN:
		//WLAN
		if(idx == 0)
			strncpy(ifname, "wlan0", size);
		else
			snprintf(ifname, size, "wlan0-vap%d", idx - 1);
		break;
	case PORT_WAN:
		{
		int wan_total = mib_chain_total(MIB_ATM_VC_TBL);
		MIB_CE_ATM_VC_T vc_entity;
		
		if(idx >= wan_total)
			return -1;

		if(!mib_chain_get(MIB_ATM_VC_TBL, idx, (void*)&vc_entity))
			return -1;

		if(vc_entity.cmode == CHANNEL_MODE_PPPOA)
			ifGetName(vc_entity.ifIndex, ifname, size);
		else
			ifGetName(PHY_INTF(vc_entity.ifIndex), ifname, size);
		}
		break;
	}

	return 0;
}

static int get_lowerlayers(int port_num, char *buf, int size)
{
	int total = get_br_port_cnt();
	unsigned char type = 0;
	int idx = 0;

	if(port_num < 1)
		return -1;

	if(get_brport_type(port_num, &type, &idx) < 0)
		return -1;

	memset(buf, 0, size);
	switch(type)
	{
	case PORT_MANAGE:
		{
		int i = 0;
		MIB_CE_ATM_VC_T vc_entity;
		struct CWMP_LINKNODE *node=NULL;
		get_ParameterEntity("Device.Bridging.Bridge.1.Port.1", (struct CWMP_LEAF **)&node);

		if(node == NULL)
		{
			CWMPDBP(1, "<%s:%d> Get Device.Bridging.Bridge.1.Port.1 failed!", __FUNCTION__, __LINE__);
			return -1;
		}
			
		node = node->sibling;	//skip itself
		while(node)
		{
			snprintf(buf, size, "%sDevice.Bridging.Bridge.1.Port.%d", buf, node->instnum);
			node = node->sibling;
			if(node)
				strncat(buf, ",", size);
		}
		}
		break;
	case PORT_ELAN:
		snprintf(buf, size, "Device.Ethernet.Interface.%d", idx + 1);
		break;
	case PORT_WLAN:
		snprintf(buf, size, "Device.WiFi.SSID.%d", idx + 1);
		break;
	case PORT_WAN:
		{
		int wan_total = mib_chain_total(MIB_ATM_VC_TBL);
		MIB_CE_ATM_VC_T vc_entity;

		if(idx >= wan_total)
			return -1;

		if(!mib_chain_get(MIB_ATM_VC_TBL, idx, (void*)&vc_entity))
			return -1;

		switch(MEDIA_INDEX(vc_entity.ifIndex))
		{
		case MEDIA_ATM:
			snprintf(buf, size, "Device.ATM.Link.%d", vc_entity.ConDevInstNum + 1);
			break;
		case MEDIA_ETH:
			snprintf(buf, size, "Device.Ethernet.Interface.%d", vc_entity.ConDevInstNum + CWMP_ETH_LAN_IF_NUM);
			break;
		case MEDIA_PTM:
			snprintf(buf, size, "Device.PTM.Link.%d", vc_entity.ConDevInstNum + 1);
			break;
		}
		}
	}
	return 0;
}

/***** Operations ************************************************************/
int getBRPortEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int br_num,port_num;
	int total_port = get_br_port_cnt();
	char ifname[IFNAMSIZ] = {0};
	unsigned char port_type;
	int port_id;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	br_num = getInstNum(name, "Bridge");
	if(br_num != 1)
		return ERR_9005;
	
	port_num = getInstNum(name, "Port");
	if(port_num < 1)
		return ERR_9005;

	if(get_brport_type(port_num, &port_type, &port_id) < 0)
		return ERR_9002;

	if(get_port_ifname(port_num, ifname, IFNAMSIZ) < 0)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		if(port_type == PORT_WAN)
		{
			MIB_CE_ATM_VC_T vc_entity;

			if(!mib_chain_get(MIB_ATM_VC_TBL, port_id, (void*)&vc_entity))
			{
				CWMPDBP(1, "<%s:%d> Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)", __FUNCTION__, __LINE__, port_id);
				return ERR_9002;
			}
			if(vc_entity.cmode == CHANNEL_MODE_BRIDGE || vc_entity.brmode != BRIDGE_DISABLE)
				*data = booldup(1);
			else
				*data = booldup(0);
		}
		else
			*data = booldup(1);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int 	flags;

		if(port_type == PORT_WAN)
		{
			MIB_CE_ATM_VC_T vc_entity;

			if(!mib_chain_get(MIB_ATM_VC_TBL, port_id, (void*)&vc_entity))
			{
				CWMPDBP(1, "<%s:%d> Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)", __FUNCTION__, __LINE__, port_id);
				return ERR_9002;
			}
			if(vc_entity.cmode != CHANNEL_MODE_BRIDGE && vc_entity.brmode == BRIDGE_DISABLE)
			{
				*data = strdup( "Down" );
				return 0;
			}
		}

		if(getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_RUNNING)
				*data = strdup( "Up" );
			else
				*data = strdup( "Down" );
		}else
			*data = strdup( "Unavailable" );
	}
	//TODO
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		*data = uintdup(1);
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
		char buf[1024] = {0};

		if(get_lowerlayers(port_num, buf, 1024) < 0)
		{
			CWMPDBG(1, (stderr, "Get %s failed\n", name));
			return ERR_9002;
		}

		*data = strdup(buf);
	}
	else if( strcmp( lastname, "ManagementPort" )==0 )
	{
		if(port_num == 1)
			*data = booldup(1);
		else 
			*data = booldup(0);
	}
	else if( strcmp( lastname, "PortState" )==0 )
	{
		static char *state_names[5] = {"Disabled", "Listening", "Learning", "Forwarding", "Blocking"};
		int state;
		FILE *file = NULL;
		char fname[256] = {0}, buf[8] = {0};

		if(port_num == 1)
			strncpy(fname, "/sys/class/net/br0/bridge/stp_state", 256);
		else	// MANAGE_PORT
			snprintf(fname, 256, "/sys/class/net/%s/brport/state", ifname);

		file = fopen(fname, "r");

		if(file != NULL)
		{
			fgets(buf, 8, file);

			fclose(file);
			state = atoi(buf);

			*data = strdup(state_names[state]);
		}
		else
			*data = strdup("Disabled");
	}
	else
		return ERR_9005;

	return 0;
}

int setBRPortEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	int br_num, port_num;
	unsigned char port_type;
	int port_id;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	br_num = getInstNum(name, "Bridge");
	if(br_num != 1)
		return ERR_9005;
	
	port_num = getInstNum(name, "Port");
	if(port_num < 1 || port_num > get_br_port_cnt())
		return ERR_9005;

	if(get_brport_type(port_num, &port_type, &port_id) < 0)
		return ERR_9002;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned char enable = *i;

		if(port_type >= PORT_WAN)
		{
			MIB_CE_ATM_VC_T vc_entity;

			if(!mib_chain_get(MIB_ATM_VC_TBL, port_id, (void*)&vc_entity))
			{
				CWMPDBP(1, "<%s:%d> Get mib chain table MIB_ATM_VC_TBL failed! (At index i=%d)", __FUNCTION__, __LINE__, port_id);
				return ERR_9002;
			}

			if(enable)
			{
				if( (vc_entity.cmode==CHANNEL_MODE_BRIDGE) ||
				    (vc_entity.cmode==CHANNEL_MODE_IPOE) )  /*EoA*/
					vc_entity.cmode=CHANNEL_MODE_BRIDGE;
				else if( vc_entity.cmode==CHANNEL_MODE_RT1483 ) /*IPoA*/
					return ERR_9007;

				vc_entity.brmode = BRIDGE_ETHERNET;
				vc_entity.napt=0;
			}
			else
			{
				if( (vc_entity.cmode==CHANNEL_MODE_BRIDGE) ||
				    (vc_entity.cmode==CHANNEL_MODE_IPOE) )  /*EoA*/
					vc_entity.cmode=CHANNEL_MODE_IPOE;
				else if( vc_entity.cmode==CHANNEL_MODE_RT1483 ) /*IPoA*/
					vc_entity.cmode=CHANNEL_MODE_RT1483;

				vc_entity.brmode = BRIDGE_DISABLE;
				vc_entity.napt=1;
			}

			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, port_id );
			gWanBitMap |= (1U << port_id);

			return CWMP_NEED_RESTART_WAN;
		}
		else if(enable != 1)
			return ERR_9001;
		else
			return CWMP_APPLIED;
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		return ERR_9001;
	}*/
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		return ERR_9001;
	}
	else if( strcmp( lastname, "ManagementPort" )==0 )
	{
		return ERR_9001;
	}
	else
	{
		return ERR_9005;
	}

	return CWMP_APPLIED;
}

int objBRPort(char *name, struct CWMP_LEAF *e, int type, void *data)
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

			MaxInstNum = get_br_port_cnt();
				
			if( create_Object( c, BRPortObject, sizeof(BRPortObject), MaxInstNum, 1 ) < 0 )
				return -1;

			add_objectNum(name, MaxInstNum);
			return 0;
		}
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
		{
			int num,i;
			struct CWMP_LINKNODE *old_table;
			int has_new=0;
			int wan_total = mib_chain_total(MIB_ATM_VC_TBL);
			MIB_CE_ATM_VC_T vc_entity;

			num = get_br_port_cnt();
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			//LAN ports
			for(i = 1 ; i < PORT_WAN_BASE; i++)
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				remove_entity = remove_SiblingEntity( &old_table, i );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, i )==NULL )
					{
						unsigned int MaxInstNum = i;

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, BRPortObject, sizeof(BRPortObject), &MaxInstNum );
						has_new=1;
					}//else already in next_table
				}
			}

			// WAN ports
			for(i = 0 ; i < wan_total ; i++)
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				int instNum;

				if(!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)&vc_entity))
					continue;

				instNum = vc_entity.ConDevInstNum + PORT_WAN_BASE - 1;
				remove_entity = remove_SiblingEntity( &old_table, instNum );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, instNum )==NULL )
					{
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, BRPortObject, sizeof(BRPortObject), &instNum );
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

int getBRPortStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	ifname[IFNAMSIZ];
	struct net_device_stats nds = {0};
	struct ethtool_stats *stats = NULL;
	int port_num, br_num;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	br_num = getInstNum(name, "Bridge");
	if(br_num != 1)
		return ERR_9005;

	port_num = getInstNum(name, "Port");
	if(port_num > get_br_port_cnt())
		return ERR_9005;

	if(get_port_ifname(port_num, ifname, IFNAMSIZ) == 0)
	{
		get_net_device_stats(ifname, &nds);
		stats = ethtool_gstats(ifname);
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


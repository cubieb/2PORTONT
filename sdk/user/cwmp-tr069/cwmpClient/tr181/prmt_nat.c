#include <sys/types.h>
#include <netinet/in.h>
#include <linux/rtnetlink.h>
#include <rtk/mib.h>

#include <cwmp_utility.h>
#include "../prmt_apply.h"
#include "prmt_ip_if.h"
#include "prmt_nat.h"

/***** Device.NAT.PortMapping.{i}. *******************************************/
struct CWMP_OP tNATPortMappingEntityLeafOP = { getNATPortMappingEntity, setNATPortMappingEntity };
struct CWMP_PRMT tNATPortMappingEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,	&tNATPortMappingEntityLeafOP},
{"Interface",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingEntityLeafOP},
{"AllInterfaces",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingEntityLeafOP},
{"LeaseDuration",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE|CWMP_DENY_ACT,	&tNATPortMappingEntityLeafOP},
{"RemoteHost",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingEntityLeafOP},
{"ExternalPort",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingEntityLeafOP},
{"ExternalPortEndRange",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingEntityLeafOP},
{"InternalPort",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingEntityLeafOP},
{"Protocol",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingEntityLeafOP},
{"InternalClient",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingEntityLeafOP},
{"Description",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingEntityLeafOP},
};

enum eNATPortMappingEntityLeaf
{
	ePortMappingEnable,
	ePortMappingStatus,
	ePortMappingInterface,
	ePortMappingAllInterfaces,
	ePortMappingLeaseDuration,
	ePortMappingRemoteHost,
	ePortMappingExternalPort,
	ePortMappingExternalPortEndRange,
	ePortMappingInternalPort,
	ePortMappingProtocol,
	ePortMappingInternalClient,
	ePortMappingDescription,
};

struct CWMP_LEAF tNATPortMappingEntityLeaf[] =
{
{ &tNATPortMappingEntityLeafInfo[ePortMappingEnable]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingStatus]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingInterface]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingAllInterfaces]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingLeaseDuration]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingRemoteHost]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingExternalPort]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingExternalPortEndRange]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingInternalPort]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingProtocol]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingInternalClient]  },
{ &tNATPortMappingEntityLeafInfo[ePortMappingDescription]  },
{ NULL	}
};

/***** Device.NAT.PortMapping. ***********************************************/
struct CWMP_OP tNATPortMappingObjectOP = { NULL, objNATPortMapping };
struct CWMP_PRMT tNATPortMappingObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",	eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST|CWMP_WRITE,	NULL},
};

enum eNATPortMappingObject
{
	ePortMappingObj0,
};

struct CWMP_LINKNODE tNATPortMappingObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tNATPortMappingObjectInfo[ePortMappingObj0],	tNATPortMappingEntityLeaf,	NULL,	NULL,	0},
};

/***** Device.NAT.InterfaceSetting.{i}. **************************************/
struct CWMP_OP tNATIfSettingEntityLeafOP = { getNATIfSettingEntity, setNATIfSettingEntity };
struct CWMP_PRMT tNATIfSettingEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tNATIfSettingEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,	&tNATIfSettingEntityLeafOP},
{"Interface",	eCWMP_tSTRING,	CWMP_READ/*|CWMP_WRITE*/,	&tNATIfSettingEntityLeafOP},
};

enum eNATIfSettingEntityLeaf
{
	eIfSettingEnable,
	eIfSettingStatus,
	eIfSettingInterface,
};

struct CWMP_LEAF tNATIfSettingEntityLeaf[] =
{
{ &tNATIfSettingEntityLeafInfo[eIfSettingEnable]  },
{ &tNATIfSettingEntityLeafInfo[eIfSettingStatus]  },
{ &tNATIfSettingEntityLeafInfo[eIfSettingInterface]  },
{ NULL	}
};

/***** Device.NAT.InterfaceSetting. ******************************************/
struct CWMP_OP tNATIfSettingObjectOP = { NULL, objNATIfSetting };
struct CWMP_PRMT tNATIFSettingObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",	eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST/*|CWMP_WRITE*/,	NULL},
};

enum eNATIfSettingObject
{
	eIfSettingObj0,
};

struct CWMP_LINKNODE tNATIfSettingObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tNATIFSettingObjectInfo[eIfSettingObj0],	tNATIfSettingEntityLeaf,	NULL,	NULL,	0},
};

/***** Device.NAT. ***********************************************************/
struct CWMP_OP tNATLeafOP = { getNAT, NULL };
struct CWMP_PRMT tNATLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"InterfaceSettingNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tNATLeafOP},
{"PortMappingNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,	&tNATLeafOP},
};

enum eNATLeaf
{
	eInterfaceSettingNumberOfEntries,
	ePortMappingNumberOfEntries
};

struct CWMP_LEAF tNATLeaf[] =
{
{ &tNATLeafInfo[eInterfaceSettingNumberOfEntries]  },
{ &tNATLeafInfo[ePortMappingNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tNATObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"InterfaceSetting",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tNATIfSettingObjectOP},
{"PortMapping",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tNATPortMappingObjectOP},
};

enum eNATObject
{
	eInterfaceSetting,
	ePortMapping,
};

struct CWMP_NODE tNATObject[] =
{
/*info,  					leaf,			node)*/
{&tNATObjectInfo[eInterfaceSetting],	NULL,	NULL},
{&tNATObjectInfo[ePortMapping],			NULL,	NULL},
{NULL,						NULL,			NULL}
};

/***** Utilities *************************************************************/
int get_port_mapping_entry( unsigned int instnum, MIB_CE_PORT_FW_T *c, unsigned int *chainID )
{
	unsigned int total,i;

	if( (instnum == 0) || (c==NULL) || (chainID==NULL) ) return -1;

	total = mib_chain_total( MIB_PORT_FW_TBL );
	for( i=0;i<total;i++ )
	{
		if( !mib_chain_get( MIB_PORT_FW_TBL, i, (void*)c ) )
			continue;

		if( c->InstanceNum==instnum )
		{
			*chainID = i;
			return 0;
		}
	}

	return -1;
}

/***** Operations ************************************************************/
int getNAT(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "InterfaceSettingNumberOfEntries" )==0 )
	{
		*data = uintdup(mib_chain_total( MIB_ATM_VC_TBL ));
	}
	else if( strcmp( lastname, "PortMappingNumberOfEntries" )==0 )
	{
		*data = uintdup(mib_chain_total( MIB_PORT_FW_TBL ));
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int objNATIfSetting(char *name, struct CWMP_LEAF *e, int type, void *data)
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
			int total_wan,i;
			struct CWMP_LINKNODE *old_table;
			int has_new=0;
			struct CWMP_LINKNODE *tmp_entity=NULL;
			MIB_CE_ATM_VC_T *p,vc_entity;

			total_wan = mib_chain_total( MIB_ATM_VC_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			p = &vc_entity;

			for( i = 0 ; i < total_wan ; i++ )
			{
				if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)p ))
					continue;

				tmp_entity = remove_SiblingEntity( &old_table, p->ConDevInstNum + 1);
				if( tmp_entity!=NULL )
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, tmp_entity );
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, p->ConDevInstNum + 1 )==NULL )
					{
						unsigned int MaxInstNum = p->ConDevInstNum + 1;

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tNATIfSettingObject, sizeof(tNATIfSettingObject), &MaxInstNum );
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

int getNATIfSettingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int if_num;
	MIB_CE_ATM_VC_T vc_entity;
	char ifname[IFNAMSIZ];
	int id;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	if_num = getInstNum(name, "NAT.InterfaceSetting");
	if(if_num < 1) return ERR_9005;
	
	if(get_wan_info_tr181(if_num, &vc_entity, &id, ifname) == 0)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup((vc_entity.napt == 1) ? 1 : 0);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		*data = strdup((vc_entity.napt == 1) ? "Enabled" : "Disabled");
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		char interface[64];
		sprintf(interface, "Device.IP.Interface.%d", if_num);
		*data = strdup(interface);
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setNATIfSettingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char *buf = data;
	unsigned int if_num;
	MIB_CE_ATM_VC_T vc_entity = {0};
	char ifname[IFNAMSIZ];
	int id;

	if( (name==NULL) || (type==0) || (data==NULL) || (entity==NULL))
		return -1;

	
	if_num = getInstNum(name, "NAT.InterfaceSetting");
	if(if_num < 1) return ERR_9005;
	
	if(get_wan_info_tr181(if_num, &vc_entity, &id, ifname) == 0)
		return ERR_9005;
	
	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned char vChar = *i;

		vc_entity.napt= (vChar == 1) ? 1 : 0;

		mib_chain_update( MIB_ATM_VC_TBL, (void *)&vc_entity, id );
		gWanBitMap |= (1U << id);

		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		return ERR_9001;
	}
	else
	{
		return ERR_9005;
	}
	return CWMP_APPLIED;
}


int objNATPortMapping(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	if( (name==NULL) || (entity==NULL) ) return -1;

	switch( type )
	{
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
	{
		int ret;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tNATPortMappingObject, sizeof(tNATPortMappingObject), data );
		if( ret >= 0 )
		{
			MIB_CE_PORT_FW_T entry;
			char vCh=0;

			memset( &entry, 0, sizeof( MIB_CE_PORT_FW_T ) );
			{ //default values for this new entry
				entry.ifIndex = DUMMY_IFINDEX;	//set to any interface by default
				entry.InstanceNum= *(int*)data;
				entry.protoType = PROTO_TCP;
			}
			mib_chain_add( MIB_PORT_FW_TBL, (unsigned char*)&entry );

			mib_get(MIB_PORT_FW_ENABLE, (void *)&vCh);
			if( vCh!=1 )
			{
				vCh=1;
				mib_set( MIB_PORT_FW_ENABLE, (void *)&vCh);
			}
		}
		return ret;
	}
	case eCWMP_tDELOBJ:
		{
			int ret;
			MIB_CE_PORT_FW_T *p, port_entity;
			unsigned int port_chainID=0;
			int inst_num;
			int total;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			p = &port_entity;

			total = mib_chain_total( MIB_PORT_FW_TBL );
			if(total >= 8)
				return ERR_9004;

			inst_num = getInstNum(name, "NAT.PortMapping");

			if(get_port_mapping_entry(inst_num, p, &port_chainID) < 0)
				return ERR_9005;

			mib_chain_delete( MIB_PORT_FW_TBL, port_chainID );
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );

			apply_PortForwarding( CWMP_STOP, port_chainID, p );

			return ret;
			break;
		}
	case eCWMP_tUPDATEOBJ:
		{
			int num=0,i;
			struct CWMP_LINKNODE *old_table;

			num = mib_chain_total( MIB_PORT_FW_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i = 0; i < num ; i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				MIB_CE_PORT_FW_T *p, port_entity;

				p = &port_entity;

				mib_chain_get(MIB_PORT_FW_TBL, i, p);

				remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					unsigned int MaxInstNum=p->InstanceNum;

					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tNATPortMappingObject, sizeof(tNATPortMappingObject), &MaxInstNum );
					if(MaxInstNum != p->InstanceNum)
					{
						p->InstanceNum = MaxInstNum;
						mib_chain_update( MIB_PORT_FW_TBL, (unsigned char*)p, i );
					}
				}
			}

			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );
			return 0;
		}
	}

	return -1;
}

int getNATPortMappingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int inst_num;
	MIB_CE_PORT_FW_T port_entity;
	int id;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	inst_num = getInstNum(name, "NAT.PortMapping");
	if(inst_num < 1) return ERR_9005;

	if(get_port_mapping_entry(inst_num, &port_entity, &id) < 0)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup((port_entity.enable == 1) ? 1 : 0);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		*data = strdup((port_entity.enable == 1) ? "Enabled" : "Disabled");
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		char path[1024] = {0};
		if(port_entity.ifIndex != DUMMY_IFINDEX)
		{
			ifIdex_to_ip_if_path(port_entity.ifIndex, path);
			*data = strdup(path);
		}
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "AllInterfaces" )==0 )
	{
		*data = booldup( (port_entity.ifIndex == DUMMY_IFINDEX) ? 1 : 0);
	}
	else if( strcmp( lastname, "LeaseDuration" )==0 )
	{
		// always 0 (static)
		*data = uintdup( 0 );
		//*data = uintdup( port_entity.leaseduration );
	}
	else if( strcmp( lastname, "RemoteHost" )==0 )
	{
		char *tmp;
		char no_ip[IP_ADDR_LEN] = {0};
		if(memcpy(no_ip, port_entity.remotehost, IP_ADDR_LEN) != 0)
		{
			tmp = inet_ntoa(*((struct in_addr *)port_entity.remotehost));
			if(tmp)
				*data = strdup( tmp );
			else
				*data = strdup( "" );
		}
		else
			*data = strdup( "" );
	}
	else if( strcmp( lastname, "ExternalPort" )==0 )
	{
		*data=uintdup( port_entity.externalfromport );
	}
	else if( strcmp( lastname, "ExternalPortEndRange" )==0 )
	{
		if(port_entity.externalfromport == port_entity.externaltoport)
			*data=uintdup(0);	//only one port
		else
			*data=uintdup( port_entity.externaltoport);	//multiple ports
	}
	else if( strcmp( lastname, "InternalPort" )==0 )
	{
		*data=uintdup( port_entity.fromPort);
	}
	else if( strcmp( lastname, "Protocol" )==0 )
	{
		if( port_entity.protoType==PROTO_TCP )
			*data = strdup( "TCP" );
		else if( port_entity.protoType==PROTO_UDP )
			*data = strdup( "UDP" );
		else if (port_entity.protoType==PROTO_UDPTCP )
			*data = strdup("TCPandUDP");
		else /*PROTO_NONE or PROTO_ICMP*/
			*data = strdup( "" );
	}
	else if( strcmp( lastname, "InternalClient" )==0 )
	{
		char *tmp;
		tmp = inet_ntoa(*((struct in_addr *)port_entity.ipAddr));
		if(tmp)
			*data = strdup( tmp );
		else
			*data = strdup( "" );
	}
	else if( strcmp( lastname, "Description" )==0 )
	{
		*data = strdup( (char*)port_entity.comment );
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setNATPortMappingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char *buf = data;
	unsigned int inst_num;
	MIB_CE_PORT_FW_T port_entity, old_entity;
	int id;

	if( (name==NULL) || (type==0) || (data==NULL) || (entity==NULL))
		return -1;

	inst_num = getInstNum(name, "NAT.PortMapping");
	if(inst_num < 1) return ERR_9005;

	if(get_port_mapping_entry(inst_num, &port_entity, &id) < 0)
		return ERR_9005;	

	memcpy( &old_entity, &port_entity, sizeof(MIB_CE_PORT_FW_T) );

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		port_entity.enable = (*i==0)?0:1;

		mib_chain_update( MIB_PORT_FW_TBL, &port_entity, id );
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, id, &old_entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		char *buf = data;
		unsigned int wanifindex = DUMMY_IFINDEX;

		if(strlen(buf) != 0)
		{
			wanifindex = ip_if_path_to_ifIndex(buf);

			if(wanifindex == DUMMY_IFINDEX)
				return ERR_9007;
		}

		port_entity.ifIndex = wanifindex;

		mib_chain_update( MIB_PORT_FW_TBL, &port_entity, id );
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, id, &old_entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "AllInterfaces" )==0 )
	{
		int *i = data;
		unsigned char vChar = *i;

		if(vChar == 1)
		{
			port_entity.ifIndex = DUMMY_IFINDEX;
			mib_chain_update( MIB_PORT_FW_TBL, &port_entity, id );
			apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, id, &old_entity, sizeof(MIB_CE_PORT_FW_T) );
		}
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "LeaseDuration" )==0 )
	{
		unsigned int *i = data;

		//only support value 0
		if( *i!=0 ) return ERR_9001;
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "RemoteHost" )==0 )
	{
		char *zeroip="0.0.0.0";

		if( strlen(buf)==0 )
			buf = zeroip;
		if ( !inet_aton(buf, (struct in_addr *)&port_entity.remotehost) )
			return ERR_9007;

		mib_chain_update( MIB_PORT_FW_TBL, &port_entity, id );
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, id, &old_entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "ExternalPort" )==0 )
	{
		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i> 65535) return ERR_9007;
		if(*i == 0) return ERR_9001;	//cannot set to 0 currently

		port_entity.externalfromport= *i;
		if(port_entity.externalfromport > port_entity.externaltoport)
			port_entity.externaltoport = *i;

		mib_chain_update( MIB_PORT_FW_TBL, &port_entity, id );
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, id, &old_entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "ExternalPortEndRange" )==0 )
	{
		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i> 65535) return ERR_9007;
		if(*i == 0) return ERR_9001;	//cannot set to 0 currently

		if(port_entity.externalfromport > port_entity.externaltoport)
			return ERR_9007;
		
		port_entity.externaltoport = *i;

		mib_chain_update( MIB_PORT_FW_TBL, &port_entity, id );
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, id, &old_entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "InternalPort" )==0 )
	{
		unsigned int *i = data;

		if(i==NULL) return ERR_9007;
		if( *i > 65535) return ERR_9007;

		port_entity.fromPort = *i;
		port_entity.toPort= *i + (port_entity.externaltoport - port_entity.externalfromport);

		mib_chain_update( MIB_PORT_FW_TBL, &port_entity, id );
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, id, &old_entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
		
	}
	else if( strcmp( lastname, "Protocol" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007;
		if( strcmp( buf, "TCP" )==0 )
			port_entity.protoType = PROTO_TCP;
		else if( strcmp( buf, "UDP" )==0 )
			port_entity.protoType = PROTO_UDP;
		else if ( strcmp( buf, "TCPandUDP" )==0 )
			port_entity.protoType = PROTO_UDPTCP;
		else
			return ERR_9007;

		mib_chain_update( MIB_PORT_FW_TBL, &port_entity, id );
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, id, &old_entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "InternalClient" )==0 )
	{
		if( strlen(buf)==0 ) return ERR_9007; //can't be empty

		if ( !inet_aton(buf, (struct in_addr *)&port_entity.ipAddr) )
			return ERR_9007;

		mib_chain_update( MIB_PORT_FW_TBL, &port_entity, id );
		apply_add( CWMP_PRI_N, apply_PortForwarding, CWMP_RESTART, id, &old_entity, sizeof(MIB_CE_PORT_FW_T) );
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "Description" )==0 )
	{
		strncpy( (char*)port_entity.comment, buf, COMMENT_LEN-1 );
		port_entity.comment[COMMENT_LEN-1]=0;

		mib_chain_update( MIB_PORT_FW_TBL, &port_entity, id );
		return CWMP_APPLIED;
	}
	else
	{
		return ERR_9005;
	}
	return CWMP_APPLIED;
}


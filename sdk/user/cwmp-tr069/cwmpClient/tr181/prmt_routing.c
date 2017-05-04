#include <stdio.h>
#include <netinet/in.h>
#include <rtk/utility.h>
#include <rtk/mib.h>

#include <cwmp_utility.h>

#include "../prmt_apply.h"
#include "../cwmpc_utility.h"
#include "prmt_ip_if.h"
#include "prmt_routing.h"

/***** Device.Routing.RIP.InterfaceSetting. **********************************/
struct CWMP_OP tRIPIfSettingEntityLeafOP = { getRIPIfSettingEntity, setRIPIfSettingEntity };
struct CWMP_PRMT tRIPIfSettingEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tRIPIfSettingEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,				&tRIPIfSettingEntityLeafOP},
{"Interface",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tRIPIfSettingEntityLeafOP},
//{"Version",		eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tRIPIfSettingEntityLeafOP},
{"AcceptRA",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tRIPIfSettingEntityLeafOP},
{"SendRA",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tRIPIfSettingEntityLeafOP},
};

enum eRIPIfSettingEntityLeaf
{
	eRIPIfSettingEnable,
	eRIPIfSettingStatus,
	eRIPIfSettingInterface,
//	eRIPIfSettingVersion,
	eRIPIfSettingAcceptRA,
	eRIPIfSettingSendRA,
};

struct CWMP_LEAF tRIPIfSettingEntityLeaf[] =
{
{ &tRIPIfSettingEntityLeafInfo[eRIPIfSettingEnable]  },
{ &tRIPIfSettingEntityLeafInfo[eRIPIfSettingStatus]  },
{ &tRIPIfSettingEntityLeafInfo[eRIPIfSettingInterface]  },
//{ &tRIPIfSettingEntityLeafInfo[eRIPIfSettingVersion]  },
{ &tRIPIfSettingEntityLeafInfo[eRIPIfSettingAcceptRA]  },
{ &tRIPIfSettingEntityLeafInfo[eRIPIfSettingSendRA]	},
{ NULL	}
};

/***** Device.Routing.RIP.InterfaceSetting. **********************************/
struct CWMP_OP tRIPIfSettingObjectOP = { NULL, objRIPIfSetting };
struct CWMP_PRMT tRIPIfSettingObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",	eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST/*|CWMP_WRITE*/,	NULL},
};

enum eRIPIfSettingObject
{
	eRIPIfSettingObj0,
};

struct CWMP_LINKNODE tRIPIfSettingObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tRIPIfSettingObjectInfo[eRIPIfSettingObj0],	tRIPIfSettingEntityLeaf,	NULL,	NULL,	0},
};

/***** Device.Routing.RIP. ***************************************************/
struct CWMP_OP tRoutingRIPLeafOP = { getRoutingRIP, setRoutingRIP };
struct CWMP_PRMT tRoutingRIPLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tRoutingRIPLeafOP},
{"SupportedModes",	eCWMP_tSTRING,	CWMP_READ,				&tRoutingRIPLeafOP},
{"InterfaceSettingNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tRoutingRIPLeafOP},
};

enum eRoutingRIPLeaf
{
	eRIPEnable,
	eRIPSupportedModes,
	eRIPInterfaceSettingNumberOfEntries,
};

struct CWMP_LEAF tRoutingRIPLeaf[] =
{
{ &tRoutingRIPLeafInfo[eRIPEnable]  },
{ &tRoutingRIPLeafInfo[eRIPSupportedModes]  },
{ &tRoutingRIPLeafInfo[eRIPInterfaceSettingNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tRoutingRIPObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"InterfaceSetting",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tRIPIfSettingObjectOP},
};

enum eRoutingRIPObject
{
	eRIPIfSetting,
};

struct CWMP_NODE tRoutingRIPObject[] =
{
/*info,  					leaf,			node)*/
{&tRoutingRIPObjectInfo[eRIPIfSetting],		NULL,	NULL},
{NULL,						NULL,			NULL}
};

#ifdef ROUTING
/***** Device.Routing.Router.{i}.IPv4Forwarding.{i} **************************/
struct CWMP_OP tIPv4FwEntityLeafOP = { getIPv4FwEntity, setIPv4FwEntity };
struct CWMP_PRMT tIPv4FwEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",				eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPv4FwEntityLeafOP},
{"Status",				eCWMP_tSTRING,	CWMP_READ,	&tIPv4FwEntityLeafOP},
{"StaticRoute", 		eCWMP_tBOOLEAN, CWMP_READ,	&tIPv4FwEntityLeafOP},
{"DestIPAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPv4FwEntityLeafOP},
{"DestSubnetMask",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPv4FwEntityLeafOP},
{"GatewayIPAddress",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPv4FwEntityLeafOP},
{"Interface",			eCWMP_tSTRING,	CWMP_READ/*|CWMP_WRITE*/,	&tIPv4FwEntityLeafOP},
{"ForwardingMetric",	eCWMP_tINT,		CWMP_READ|CWMP_WRITE,	&tIPv4FwEntityLeafOP},
};

enum eIPv4FwEntityLeaf
{
	eIPv4FwEnable,
	eIPv4FwStatus,
	eIPv4FwStaticRoute,
	eIPv4FwDestIPAddress,
	eIPv4FwDestSubnetMask,
	eIPv4FwGatewayIPAddress,
	eIPv4FwInterface,
	eIPv4FwForwardingMetric,
};

struct CWMP_LEAF tIPv4FwEntityLeaf[] =
{
{ &tIPv4FwEntityLeafInfo[eIPv4FwEnable]  },
{ &tIPv4FwEntityLeafInfo[eIPv4FwStatus]  },
{ &tIPv4FwEntityLeafInfo[eIPv4FwStaticRoute]  },
{ &tIPv4FwEntityLeafInfo[eIPv4FwDestIPAddress]  },
{ &tIPv4FwEntityLeafInfo[eIPv4FwDestSubnetMask]  },
{ &tIPv4FwEntityLeafInfo[eIPv4FwGatewayIPAddress]	},
{ &tIPv4FwEntityLeafInfo[eIPv4FwInterface]  },
{ &tIPv4FwEntityLeafInfo[eIPv4FwForwardingMetric]  },
{ NULL	}
};

/***** Device.Routing.Router.{i}.IPv4Forwarding ******************************/
struct CWMP_OP tIPv4FwObjectOP = { NULL, objIPv4Fw };
struct CWMP_PRMT tIPv4FwObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",	eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST|CWMP_WRITE,	NULL},
};

enum eIPv4FwObject
{
	eIPv4FwObj0,
};

struct CWMP_LINKNODE tIPv4FwObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tIPv4FwObjectInfo[eIPv4FwObj0],	tIPv4FwEntityLeaf,	NULL,	NULL,	0},
};

/***** Device.Routing.Router.{i}. ********************************************/
struct CWMP_OP tRouterEntityLeafOP = { getRouterEntity, NULL };
struct CWMP_PRMT tRouterEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_READ/*|CWMP_WRITE*/,	&tRouterEntityLeafOP},
{"Status",	eCWMP_tSTRING,	CWMP_READ,	&tRouterEntityLeafOP},
{"IPv4ForwardingNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tRouterEntityLeafOP},
};

enum eRouterEntityLeaf
{
	eRouterEnable,
	eRouterStatus,
	eRouterIPv4ForwardingNumberOfEntries,
};

struct CWMP_LEAF tRouterEntityLeaf[] =
{
{ &tRouterEntityLeafInfo[eRouterEnable]  },
{ &tRouterEntityLeafInfo[eRouterStatus]  },
{ &tRouterEntityLeafInfo[eRouterIPv4ForwardingNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tRouterEntityObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"IPv4Forwarding",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tIPv4FwObjectOP},
};

enum eRouterEntityObject
{
	eIPv4Forwarding,
};

struct CWMP_NODE tRouterEntityObject[] =
{
/*info,  					leaf,			node)*/
{&tRouterEntityObjectInfo[eIPv4Forwarding],		NULL,	NULL},
{NULL,						NULL,			NULL}
};

/***** Device.Routing.Router. ************************************************/
struct CWMP_PRMT tRouterObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"1",	eCWMP_tOBJECT,	CWMP_READ/*|CWMP_WRITE*/,	NULL},
};

enum eRouterObject
{
	eRouterObj1,
};

struct CWMP_NODE tRouterObject[] =
{
/*info,  				leaf,			next)*/
{&tRouterObjectInfo[eRouterObj1],	tRouterEntityLeaf,	tRouterEntityObject},
{NULL,						NULL,			NULL}
};

/***** Device.Routing. *******************************************************/
struct CWMP_OP tRoutingLeafOP = { getRouting, NULL };
struct CWMP_PRMT tRoutingLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"RouterNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tRoutingLeafOP},
};

enum eRoutingLeaf
{
	eIRouterNumberOfEntries,
};

struct CWMP_LEAF tRoutingLeaf[] =
{
{ &tRoutingLeafInfo[eIRouterNumberOfEntries]  },
{ NULL	}
};
#endif

struct CWMP_PRMT tRoutingObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"RIP",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef ROUTING
{"Router",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
#endif
};

enum eRoutingObject
{
	eRIP,
#ifdef ROUTING
	eRouter,
#endif
};

struct CWMP_NODE tRoutingObject[] =
{
/*info,  					leaf,			node)*/
{&tRoutingObjectInfo[eRIP],		tRoutingRIPLeaf,	tRoutingRIPObject},
#ifdef ROUTING
{&tRoutingObjectInfo[eRouter],	NULL,	tRouterObject},
#endif
{NULL,						NULL,			NULL}
};

/***** Utilities *************************************************************/
int get_rip_info(unsigned int ifIndex, MIB_CE_RIP_T *info, int *id)
{
	if( info == NULL && id == NULL ) return -1;

	unsigned int entryNum=0, i=0;
	entryNum = mib_chain_total(MIB_RIP_TBL);
	for( i=0; i<entryNum; i++ )
	{
		if (!mib_chain_get(MIB_RIP_TBL, i, (void *)info))
			continue;
		if(info->ifIndex==ifIndex)
		{
			//read receiveMode.
			*id = i;
			return 0;
		}
	}

	*id = -1;
	return -1;
}


#ifdef ROUTING
/***** Operations ************************************************************/
int getRouting(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "RouterNumberOfEntries" )==0 )
	{
		*data = uintdup(1);
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int getRouterEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		*data = strdup("Enabled");
	}
	else if( strcmp( lastname, "IPv4ForwardingNumberOfEntries" )==0 )
	{
		unsigned int total=0;
		total = mib_chain_total( MIB_IP_ROUTE_TBL );
		total += getDynamicForwardingTotalNum();
		*data = uintdup( total );
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int objIPv4Fw(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
	     {
			int ret, num;
			int *inst_num = data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_IP_ROUTE_TBL );
			if(num >= 8)
				return ERR_9004;
			else if(num == 0)
				*inst_num = MAX_DYNAMIC_ROUTE_INSTNUM + 1;

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tIPv4FwObject, sizeof(tIPv4FwObject), data );
			if( ret >= 0 )
			{
				MIB_CE_IP_ROUTE_T fentry;
				memset( &fentry, 0, sizeof( MIB_CE_IP_ROUTE_T ) );
				fentry.InstanceNum = *(unsigned int*)data;
				fentry.FWMetric = -1;
				fentry.ifIndex = DUMMY_IFINDEX;
				mib_chain_add( MIB_IP_ROUTE_TBL, (unsigned char*)&fentry );
			}
			return ret;
		}
	case eCWMP_tDELOBJ:
		{
			int ret;
			unsigned int id;
			MIB_CE_IP_ROUTE_T route_old;
	     	
			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			if( *(unsigned int*)data<=MAX_DYNAMIC_ROUTE_INSTNUM ) //dynamic route
				return ERR_9001;

			if( getForwardingEntryByInstNum( *(unsigned int*)data, &route_old, &id )<0 )
				return ERR_9005;

			mib_chain_delete( MIB_IP_ROUTE_TBL, id );	
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );

			apply_Layer3Forwarding( CWMP_RESTART, id, &route_old );	
			return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
			int num=0,i;
			struct CWMP_LINKNODE *old_table;
	     	
			old_table = (struct CWMP_LINKNODE*)entity->next;
			entity->next = NULL;

			//Dynamic Routing
			num = getDynamicForwardingTotalNum();
			for( i = 1; i <= num ; i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				remove_entity = remove_SiblingEntity( &old_table, i );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{ 
					unsigned int MaxInstNum=i;					
					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tIPv4FwObject, sizeof(tIPv4FwObject), &MaxInstNum );
				}
			}

			//Static Routing
			num = mib_chain_total( MIB_IP_ROUTE_TBL );
			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				MIB_CE_IP_ROUTE_T *p,route_entity;

				p = &route_entity;
				if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
					continue;

				if(p->InstanceNum <= MAX_DYNAMIC_ROUTE_INSTNUM) p->InstanceNum = 0; //to get an new instnum > MAX_DYNAMIC_ROUTE_INSTNUM

				remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					unsigned int MaxInstNum = p->InstanceNum;

					// First new static routing rule
					if(i==0 && MaxInstNum <= MAX_DYNAMIC_ROUTE_INSTNUM)
						MaxInstNum = MAX_DYNAMIC_ROUTE_INSTNUM + 1;
					
					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tIPv4FwObject, sizeof(tIPv4FwObject), &MaxInstNum );
					if(MaxInstNum != p->InstanceNum)
					{
						p->InstanceNum = MaxInstNum;
						mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
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

int getIPv4FwEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char 	buf[64] = {0};
	unsigned int object_num=0;
	struct CWMP_NODE *fw_entity;
	int	chain_id=0;
	MIB_CE_IP_ROUTE_T *fw=NULL, route_entity;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	object_num = getInstNum( name, "IPv4Forwarding" );

	if(object_num < 1) return ERR_9005;
	fw = &route_entity;

	if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM )
	{
		//dynamic route
		getDynamicForwardingEntryByInstNum( object_num, fw );
	}
	else
	{
		//static route
		if(getForwardingEntryByInstNum( object_num, fw, &chain_id )<0)
			return ERR_9005;
	}
	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		//fprintf( stderr, "<%s:%d>enable:%d\n",__FUNCTION__,__LINE__,fw->Enable  );fflush(NULL);
		*data = booldup( fw->Enable!=0 );
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int RouteRet;

		if( object_num <= MAX_DYNAMIC_ROUTE_INSTNUM )
			RouteRet = (fw->Enable)?1:0;
		else
			RouteRet = queryRouteStatus(fw);

		switch(RouteRet)
		{
		case 1: //found
			*data = strdup( "Enabled" );
			break;
		case 0: //not found
			if( fw->Enable )
				*data = strdup( "Error" );
			else
				*data = strdup( "Disabled" );
			break;
		case -1://error
		default:
			*data = strdup( "Error" );
			break;
		}
	}
	else if( strcmp( lastname, "StaticRoute" )==0 )
	{
		if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM )
			*data = booldup(0);
		else
			*data = booldup(1);
	}
	else if( strcmp( lastname, "DestIPAddress" )==0 )
	{
		unsigned char zero_ip[IP_ADDR_LEN] = {0};

		if(memcmp(fw->destID, zero_ip, IP_ADDR_LEN) != 0)
			sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->destID)));

		*data = strdup( buf );
	}
	else if( strcmp( lastname, "DestSubnetMask" )==0 )
	{
		unsigned char zero_ip[IP_ADDR_LEN] = {0};

		if(memcmp(fw->destID, zero_ip, IP_ADDR_LEN) != 0)
			sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->netMask)));

		*data = strdup( buf );
	}
	else if( strcmp( lastname, "GatewayIPAddress" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->nextHop)));
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		char tmp[256]= {0};

		if( ifIdex_to_ip_if_path( fw->ifIndex, tmp )<0 ) 
			*data = strdup( "" );
		else
			*data = strdup( tmp );
	}
	else if( strcmp( lastname, "ForwardingMetric" )==0 )
	{
		*data = intdup( fw->FWMetric );
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}

int setIPv4FwEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	int	object_num=0;
	struct CWMP_NODE *fw_entity;
	int	chain_id=0;
	MIB_CE_IP_ROUTE_T *fw=NULL,route_entity;
	struct in_addr in;
	char	*pzeroip="0.0.0.0";
	MIB_CE_IP_ROUTE_T route_old;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;

	if( entity->info->type!=type ) return ERR_9006;

	object_num = getInstNum( name, "IPv4Forwarding" );
	if(object_num==0) return ERR_9005;

	if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM ) return ERR_9001; //reject to modify a dynamic route

	fw = &route_entity;
	if(getForwardingEntryByInstNum( object_num, fw, &chain_id )<0)
		return ERR_9005;

	memcpy( &route_old, fw, sizeof(MIB_CE_IP_ROUTE_T) );

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		fw->Enable = (*i==0)? 0:1;
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}
	else if( strcmp( lastname, "DestIPAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //invalid ip address
			return ERR_9007;

		memcpy( fw->destID, &in, sizeof(struct in_addr) );

		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}
	else if( strcmp( lastname, "DestSubnetMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //invalid subnet mask
			return ERR_9007;
		memcpy( fw->netMask, &in, sizeof(struct in_addr) );

		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}
	else if( strcmp( lastname, "GatewayIPAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;
		if( inet_aton( buf, &in )==0 ) //invalid ip address
			return ERR_9007;
		memcpy( fw->nextHop, &in, sizeof(struct in_addr) );

		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );
		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		unsigned int newifindex;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) 
			fw->ifIndex = DUMMY_IFINDEX;
		else
		{
			newifindex = ip_if_path_to_ifIndex( buf );
			if( newifindex == DUMMY_IFINDEX ) return ERR_9007;
			fw->ifIndex = newifindex;
		}
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}
	else if( strcmp( lastname, "ForwardingMetric" )==0 )
	{
		int *i = data;

		if( *i < -1 ) return ERR_9007;
		fw->FWMetric = *i;
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}
#endif

int getRoutingRIP(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		unsigned char enable;

		mib_get( MIB_RIP_ENABLE, (void *)&enable);

		*data = booldup((enable == 1) ? 1 : 0);
	}
	else if( strcmp( lastname, "SupportedModes" )==0 )
	{
		*data = strdup("Both");
	}
	else if( strcmp( lastname, "InterfaceSettingNumberOfEntries" )==0 )
	{
		unsigned int total=0;
		total = mib_chain_total( MIB_ATM_VC_TBL ) + 1;	//WAN + LAN
		*data = uintdup( total );
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setRoutingRIP(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==0) || (data==NULL) || (entity==NULL))
		return -1;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned char enable = (*i == 0) ? 0 : 1;

		mib_set( MIB_RIP_ENABLE, (void *)&enable);
		apply_add( CWMP_PRI_N, apply_RIP, CWMP_RESTART, 0, NULL, 0 );
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

int objRIPIfSetting(char *name, struct CWMP_LEAF *e, int type, void *data)
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

			//Move LAN object to new list
			tmp_entity = remove_SiblingEntity( &old_table, 1);
			if( tmp_entity!=NULL )
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, tmp_entity );
			else
			{
				unsigned int MaxInstNum = 1;
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tRIPIfSettingObject, sizeof(tRIPIfSettingObject), &MaxInstNum );
			}

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

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tRIPIfSettingObject, sizeof(tRIPIfSettingObject), &MaxInstNum );
					}//else already in next_table
				}
			}
			
			if( old_table )
			{
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );
			}
			return 0;
		}
	}
	
	return -1;
}

int getRIPIfSettingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int ifnum, vc_id, rip_id = -1;
	MIB_CE_ATM_VC_T vc_entity;
	char ifname[IFNAMSIZ] = {0};
	MIB_CE_RIP_T rip_info;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	ifnum = getInstNum(name, "RIP.InterfaceSetting");
	if(ifnum < 1) return ERR_9005;

	//LAN
	if(ifnum == 1)
	{
		get_rip_info(DUMMY_IFINDEX, &rip_info, &rip_id);
	}
	else
	{
		if(get_wan_info_tr181(ifnum, &vc_entity, &vc_id, ifname) == 0)
		return ERR_9005;

		get_rip_info(vc_entity.ifIndex, &rip_info, &rip_id);
	}

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		if(rip_id < 0)
			*data = booldup(0);
		else
			*data = booldup(1);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		if(rip_id < 0)
			*data = strdup("Disabled");
		else
			*data = strdup("Enabled");
		
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		char path[256] = {0};

		snprintf(path, 256, "Device.IP.Interface.%d", ifnum);
		*data = strdup(path);
	}
	else if( strcmp( lastname, "AcceptRA" )==0 )
	{
		if(rip_id < 0 || rip_info.receiveMode == RIP_NONE)
			*data = booldup(0);
		else
			*data = booldup(1);
	}
	else if( strcmp( lastname, "SendRA" )==0 )
	{
		if(rip_id < 0 || rip_info.sendMode == RIP_NONE)
			*data = booldup(0);
		else
			*data = booldup(1);
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setRIPIfSettingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char *buf = data;
	int ifnum, vc_id, rip_id = -1;
	MIB_CE_ATM_VC_T vc_entity;
	char ifname[IFNAMSIZ] = {0};
	MIB_CE_RIP_T rip_info;

	if( (name==NULL) || (type==0) || (data==NULL) || (entity==NULL))
		return -1;

	ifnum = getInstNum(name, "RIP.InterfaceSetting");
	if(ifnum < 1) return ERR_9005;

	//LAN
	if(ifnum == 1)
	{
		get_rip_info(DUMMY_IFINDEX, &rip_info, &rip_id);
	}
	else
	{
		if(get_wan_info_tr181(ifnum, &vc_entity, &vc_id, ifname) == 0)
		return ERR_9005;

		get_rip_info(vc_entity.ifIndex, &rip_info, &rip_id);
	}

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned char vChar = *i;

		if(vChar == 1)
		{
			if(rip_id < 0)
			{
				if(ifnum == 1)
					rip_info.ifIndex = DUMMY_IFINDEX;	//LAN
				else
					rip_info.ifIndex = vc_entity.ifIndex;
				rip_info.receiveMode = RIP_NONE;
				rip_info.sendMode = RIP_NONE;

				mib_chain_add(MIB_RIP_TBL, &rip_info);
				apply_add( CWMP_PRI_N, apply_RIP, CWMP_RESTART, 0, NULL, 0 );
			}

			return 0;
		}
		else if(vChar == 0)
		{
			if(rip_id >= 0)
			{
				mib_chain_delete(MIB_RIP_TBL, rip_id);
				apply_add( CWMP_PRI_N, apply_RIP, CWMP_RESTART, 0, NULL, 0 );
			}
			return 0;
		}
		else
			return ERR_9007;
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		return ERR_9001;
	}
	else if( strcmp( lastname, "AcceptRA" )==0 )
	{
		int *i = data;
		unsigned char vChar = *i;

		rip_info.receiveMode = (vChar == 0) ? RIP_NONE : RIP_V2;

		if(rip_id >= 0)
		{
			mib_chain_update(MIB_RIP_TBL, &rip_info, rip_id);
		}
#if 1
		else
			return ERR_9001;
#else
		else
		{
			//create new
			if(ifnum == 1)
				rip_info.ifIndex = DUMMY_IFINDEX;
			else
				rip_info.ifIndex = vc_entity.ifIndex;
			rip_info.sendMode = RIP_NONE;

			mib_chain_add(MIB_RIP_TBL, &rip_info);
		}
#endif		
		apply_add( CWMP_PRI_N, apply_RIP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "SendRA" )==0 )
	{
		int *i = data;
		unsigned char vChar = *i;

		rip_info.sendMode = (vChar == 0) ? RIP_NONE : RIP_V2;

		if(rip_id >= 0)
		{
			mib_chain_update(MIB_RIP_TBL, &rip_info, rip_id);
		}
#if 1
		else
			return ERR_9001;
#else
		else
		{
			//create new
			if(ifnum == 1)
				rip_info.ifIndex = DUMMY_IFINDEX;
			else
				rip_info.ifIndex = vc_entity.ifIndex;
			rip_info.sendMode = RIP_NONE;

			mib_chain_add(MIB_RIP_TBL, &rip_info);
		}
#endif		
		apply_add( CWMP_PRI_N, apply_RIP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}


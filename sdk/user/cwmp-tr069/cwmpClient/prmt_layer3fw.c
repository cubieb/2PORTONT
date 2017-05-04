#include "cwmpc_utility.h"

#include "prmt_layer3fw.h"
#include "prmt_wancondevice.h"

#ifdef ROUTING
#ifdef _PRMT_WT107_
int getDynamicForwardingTotalNum(void);
int getDynamicForwardingEntryByInstNum( unsigned int instnum, MIB_CE_IP_ROUTE_T *pRoute );
#endif //_PRMT_WT107_
extern unsigned char transfer2IfIndxfromIfName( char *ifname );
extern unsigned int getInstNum( char *name, char *objname );
int queryRouteStatus( MIB_CE_IP_ROUTE_T *p );
unsigned int getForwardingInstNum( char *name );
unsigned int findMaxForwardingInstNum(void);
int getForwardingEntryByInstNum( unsigned int instnum, MIB_CE_IP_ROUTE_T *p, unsigned int *id );

/*******ForwardingEntity****************************************************************************************/
struct CWMP_OP tForwardingEntityLeafOP = { getFwEntity, setFwEntity };
struct CWMP_PRMT tForwardingEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tForwardingEntityLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"StaticRoute",			eCWMP_tBOOLEAN,	CWMP_READ,		&tForwardingEntityLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"Type",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"DestIPAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"DestSubnetMask",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"SourceIPAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"SourceSubnetMask",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"GatewayIPAddress",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"Interface",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
{"ForwardingMetric",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tForwardingEntityLeafOP},
/*MTU*/
};
enum eForwardingEntityLeaf
{
	eFWEnable,
	eFWStatus,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eFWStaticRoute,
#endif
/*ping_zhang:20081217 END*/
	eFWType,
	eFWDestIPAddress,
	eFWDestSubnetMask,
	eFWSourceIPAddress,
	eFWSourceSubnetMask,
	eFWGatewayIPAddress,
	eFWInterface,
	eFWForwardingMetric
};
struct CWMP_LEAF tForwardingEntityLeaf[] =
{
{ &tForwardingEntityLeafInfo[eFWEnable] },
{ &tForwardingEntityLeafInfo[eFWStatus] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tForwardingEntityLeafInfo[eFWStaticRoute] },
#endif
/*ping_zhang:20081217 END*/
{ &tForwardingEntityLeafInfo[eFWType] },
{ &tForwardingEntityLeafInfo[eFWDestIPAddress] },
{ &tForwardingEntityLeafInfo[eFWDestSubnetMask] },
{ &tForwardingEntityLeafInfo[eFWSourceIPAddress] },
{ &tForwardingEntityLeafInfo[eFWSourceSubnetMask] },
{ &tForwardingEntityLeafInfo[eFWGatewayIPAddress] },
{ &tForwardingEntityLeafInfo[eFWInterface] },
{ &tForwardingEntityLeafInfo[eFWForwardingMetric] },
{ NULL }
};

/*******Forwarding****************************************************************************************/
struct CWMP_PRMT tForwardingOjbectInfo[] =
{
/*(name,	type,		flag,					op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eForwardingOjbect
{
	eFW0
};
struct CWMP_LINKNODE tForwardingObject[] =
{
/*info,  			leaf,			next,		sibling,		instnum)*/
{&tForwardingOjbectInfo[eFW0],	tForwardingEntityLeaf,	NULL,		NULL,			0},
};

/*******Layer3Forwarding****************************************************************************************/
struct CWMP_OP tLayer3ForwardingLeafOP = { getLayer3Fw,	setLayer3Fw };
struct CWMP_PRMT tLayer3ForwardingLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"DefaultConnectionService",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLayer3ForwardingLeafOP},
{"ForwardNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tLayer3ForwardingLeafOP},
};
enum eLayer3ForwardingLeaf
{
	eFWDefaultConnectionService,
	eFWForwardNumberOfEntries
};
struct CWMP_LEAF tLayer3ForwardingLeaf[] =
{
{ &tLayer3ForwardingLeafInfo[eFWDefaultConnectionService] },
{ &tLayer3ForwardingLeafInfo[eFWForwardNumberOfEntries] },
{ NULL }
};
struct CWMP_OP tFW_Forwarding_OP = { NULL, objForwading };
struct CWMP_PRMT tLayer3ForwardingObjectInfo[] =
{
/*(name,			type,		flag,			)*/
{"Forwarding",			eCWMP_tOBJECT,	CWMP_WRITE|CWMP_READ,	&tFW_Forwarding_OP},
};
enum eLayer3ForwardingObject
{
	eFWForwarding
};
struct CWMP_NODE tLayer3ForwardingObject[] =
{
/*info,  					leaf,			node)*/
{&tLayer3ForwardingObjectInfo[eFWForwarding],	NULL,			NULL},
{NULL,						NULL,			NULL}
};
/***********************************************************************************************/
int getFwEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	fw_buf[256], buf[64], *tok;
	unsigned int object_num=0;
	struct CWMP_NODE *fw_entity;
	int	chain_id=0;
	MIB_CE_IP_ROUTE_T *fw=NULL, route_entity;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	object_num=getForwardingInstNum( name );
	if(object_num==0) return ERR_9005;
	fw = &route_entity;
#ifdef _PRMT_WT107_
	if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM )
	{
		//dynamic route
		getDynamicForwardingEntryByInstNum( object_num, fw );
	}else
#endif //_PRMT_WT107_
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
	}else if( strcmp( lastname, "Status" )==0 )
	{
		int RouteRet;
#ifdef _PRMT_WT107_
		if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM )
			RouteRet = (fw->Enable)?1:0;
		else
#endif
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
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "StaticRoute" )==0 )
	{
		if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM )
			*data = booldup(0);
		else
			*data = booldup(1);
#endif //_PRMT_WT107_
	}else if( strcmp( lastname, "Type" )==0 )
	{
		if( fw->Type==0 )
			*data = strdup( "Network" );
		else if( fw->Type==1 )
			*data = strdup( "Host" );
		else if( fw->Type==2 )
			*data = strdup( "Default" );
		else 
			return ERR_9002;
	}else if( strcmp( lastname, "DestIPAddress" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->destID)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "DestSubnetMask" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->netMask)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "SourceIPAddress" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->SourceIP)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "SourceSubnetMask" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->SourceMask)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "GatewayIPAddress" )==0 )
	{
		sprintf(buf, "%s", inet_ntoa(*((struct in_addr *)&fw->nextHop)));
		*data = strdup( buf );
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		char tmp[256];
		//change from eth0 to internetgatewaydevice.xxx.xxxx.xxxx.xxxx
		//strcpy( tmp, fw->Interface );
		if( transfer2PathName( fw->ifIndex, tmp )<0 ) 
			*data = strdup( "" );//return ERR_9007;
		else
			*data = strdup( tmp );
	}else if( strcmp( lastname, "ForwardingMetric" )==0 )
	{
		*data = intdup( fw->FWMetric );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setFwEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	fw_buf[256], *tok;
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

	object_num=getForwardingInstNum( name );
	if(object_num==0) return ERR_9005;
#ifdef _PRMT_WT107_
	if( object_num<=MAX_DYNAMIC_ROUTE_INSTNUM ) return ERR_9001; //reject to modify a dynamic route
#endif //_PRMT_WT107_
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
	}else if( strcmp( lastname, "Type" )==0 )
	{
		if(  strcmp( buf, "Network" )==0 )
			fw->Type = 0;
		else if(  strcmp( buf, "Host" )==0 )
			fw->Type = 1;		
		else if(  strcmp( buf, "Default" )==0 )
			fw->Type = 2;
		else
			return ERR_9007;
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}else if( strcmp( lastname, "DestIPAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;//return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( fw->destID, &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}else if( strcmp( lastname, "DestSubnetMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;//return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( fw->netMask, &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}else if( strcmp( lastname, "SourceIPAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
#if 1
		if( (strlen(buf)==0) || (strcmp(buf, pzeroip)==0) )
			return 0;
		else
			return ERR_9001;//deny
#else
		if( strlen(buf)==0 ) buf=pzeroip;//return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( fw->SourceIP, &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#endif
	}else if( strcmp( lastname, "SourceSubnetMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
#if 1
		if( (strlen(buf)==0) || (strcmp(buf, pzeroip)==0) )
			return 0;
		else
			return ERR_9001;//deny
#else
		if( strlen(buf)==0 ) buf=pzeroip;//return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( fw->SourceMask, &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
#endif
	}else if( strcmp( lastname, "GatewayIPAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) buf=pzeroip;//return ERR_9007;
		if( inet_aton( buf, &in )==0 ) //the ip address is error.
			return ERR_9007;
		memcpy( fw->nextHop, &in, sizeof(struct in_addr) );
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}else if( strcmp( lastname, "Interface" )==0 )
	{
		//change from internetgatewaydevice.xxx.xx.xxx to eth0
		//strcpy( fw->Interface, "eth0" );
		unsigned int newifindex;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) 
			fw->ifIndex=DUMMY_IFINDEX;//return ERR_9007;
		else{
			newifindex = transfer2IfIndex( buf );
			if( newifindex==DUMMY_IFINDEX ) return ERR_9007;
			fw->ifIndex = newifindex;
		}
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}else if( strcmp( lastname, "ForwardingMetric" )==0 )
	{
		int *i = data;

		if( *i < -1 ) return ERR_9007;
		fw->FWMetric = *i;
		mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)fw, chain_id );

		apply_add( CWMP_PRI_N, apply_Layer3Forwarding, CWMP_RESTART, chain_id, &route_old, sizeof(MIB_CE_IP_ROUTE_T) );
		return 0;
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int getLayer3Fw(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "DefaultConnectionService" )==0 )
	{
		char buf[256];
		if( getDefaultRoute(buf)<0 )
		*data = strdup( "" );
		else
			*data = strdup( buf );
	}else if( strcmp( lastname, "ForwardNumberOfEntries" )==0 )
	{
		unsigned int total=0;
		total = mib_chain_total( MIB_IP_ROUTE_TBL );
#ifdef _PRMT_WT107_
		total += getDynamicForwardingTotalNum();
#endif //_PRMT_WT107_
		*data = uintdup( total );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setLayer3Fw(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "DefaultConnectionService" )==0 )
	{
		char *buf=data;
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;
		if( setDefaultRoute( buf ) < 0 ) return ERR_9007;

		apply_add( CWMP_PRI_N, apply_DefaultRoute, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int objForwading(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		unsigned int num=0,MaxInstNum=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		MIB_CE_IP_ROUTE_T *p,route_entity;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

#ifdef _PRMT_WT107_
		{
			//dynamic route
			unsigned int dyn_route_num=0;
			dyn_route_num=getDynamicForwardingTotalNum();
			if(dyn_route_num>0)
			{
				if( create_Object( c, tForwardingObject, sizeof(tForwardingObject), dyn_route_num, 1 ) < 0 )
					return -1;		
			}
			add_objectNum( name, MAX_DYNAMIC_ROUTE_INSTNUM );
		}
#endif //_PRMT_WT107_

		MaxInstNum=findMaxForwardingInstNum();
#ifdef _PRMT_WT107_
		if(MaxInstNum<=MAX_DYNAMIC_ROUTE_INSTNUM) MaxInstNum=MAX_DYNAMIC_ROUTE_INSTNUM;
#endif //_PRMT_WT107_
		num = mib_chain_total( MIB_IP_ROUTE_TBL );
		for( i=0; i<num;i++ )
		{
			p = &route_entity;
			if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
				continue;

			if( (p->InstanceNum==0) //maybe createn by web or cli
#ifdef _PRMT_WT107_
			    || (p->InstanceNum<=MAX_DYNAMIC_ROUTE_INSTNUM)
#endif //_PRMT_WT107_
			  )
			{
				MaxInstNum++;
				p->InstanceNum = MaxInstNum;
				mib_chain_update( MIB_IP_ROUTE_TBL, (unsigned char*)p, i );
			}
			
			if( create_Object( c, tForwardingObject, sizeof(tForwardingObject), 1, p->InstanceNum ) < 0 )
				return -1;
		}
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;
		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tForwardingObject, sizeof(tForwardingObject), data );
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

#ifdef _PRMT_WT107_
		if( *(unsigned int*)data<=MAX_DYNAMIC_ROUTE_INSTNUM ) //dynamic route
			return ERR_9001;
#endif //_PRMT_WT107_

		if( getForwardingEntryByInstNum( *(unsigned int*)data, &route_old, &id )<0 )
			return ERR_9005;
		apply_Layer3Forwarding( CWMP_STOP, id, &route_old );
     	mib_chain_delete( MIB_IP_ROUTE_TBL, id );	
		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
		return ret;
	     }
	case eCWMP_tUPDATEOBJ:	
	     {
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;
	     	
	     	old_table = (struct CWMP_LINKNODE*)entity->next;
	     	entity->next = NULL;

#ifdef _PRMT_WT107_
		num=getDynamicForwardingTotalNum();
	     	for( i=1; i<=num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;

			remove_entity = remove_SiblingEntity( &old_table, i );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{ 
				unsigned int MaxInstNum=i;					
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tForwardingObject, sizeof(tForwardingObject), &MaxInstNum );
			}
	     	}
#endif //_PRMT_WT107_

	     	num = mib_chain_total( MIB_IP_ROUTE_TBL );
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			MIB_CE_IP_ROUTE_T *p,route_entity;

			p = &route_entity;
			if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
				continue;

#ifdef _PRMT_WT107_
			if(p->InstanceNum<=MAX_DYNAMIC_ROUTE_INSTNUM) p->InstanceNum=0; //to get an new instnum > MAX_DYNAMIC_ROUTE_INSTNUM
#endif //_PRMT_WT107_			
			remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
			}else{ 
				unsigned int MaxInstNum=p->InstanceNum;
					
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tForwardingObject, sizeof(tForwardingObject), &MaxInstNum );
				if(MaxInstNum!=p->InstanceNum)
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

/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
int getChainID( struct CWMP_LINKNODE *ctable, int num )
{
	int id=-1;
	char buf[32];

//	sprintf( buf, "%d", num );
	while( ctable )
	{
		id++;
		//if( strcmp(ctable->name, buf)==0 )
		if( ctable->instnum==(unsigned int)num )
			break;
		ctable = ctable->sibling;
	}	
	return id;
}


//return -1:error,  0:not found, 1:found in /proc/net/route
//refer to user/busybox/route.c:displayroutes()
int queryRouteStatus( MIB_CE_IP_ROUTE_T *p )
{
	int ret=-1;

	//fprintf( stderr, "<%s:%d>Start\n",__FUNCTION__,__LINE__ );
	
	if(p)
	{
		char *routename="/proc/net/route";
		FILE *fp;
		
		fp=fopen( routename, "r" );
		if(fp)
		{
			char buff[256];
			int  i=0;
			while( fgets(buff, sizeof(buff), fp) != NULL )
			{
				int flgs, ref, use, metric;
				char ifname[16], ifname2[16]; 
				unsigned long int d,g,m;

				//fprintf( stderr, "<%s:%d>%s\n",__FUNCTION__,__LINE__,buff );
				i++;
				if(i==1) continue; //skip the first line
				 
				//Iface Destination Gateway Flags RefCnt Use Metric Mask MTU Window IRTT
				if(sscanf(buff, "%s%lx%lx%X%d%d%d%lx",
					ifname, &d, &g, &flgs, &ref, &use, &metric, &m)!=8)
					break;//Unsuported kernel route format
				
				if( ifGetName( p->ifIndex, ifname2, sizeof(ifname2) )==0 ) //any interface
					ifname2[0]=0;

				//fprintf( stderr, "<%s:%d>%s==%s(%d)\n",__FUNCTION__,__LINE__,ifname,ifname2, ( (strlen(ifname2)==0) || (strcmp(ifname, ifname2)==0) ) );
				//fprintf( stderr, "<%s:%d>%06x==%06x(%d)\n",__FUNCTION__,__LINE__,*((unsigned long int*)(p->destID)),d,(*((unsigned long int*)(p->destID))==d) );
				//fprintf( stderr, "<%s:%d>%06x==%06x(%d)\n",__FUNCTION__,__LINE__,*((unsigned long int*)(p->netMask)),m,(*((unsigned long int*)(p->netMask))==m) );
				//fprintf( stderr, "<%s:%d>%06x==%06x(%d)\n",__FUNCTION__,__LINE__,*((unsigned long int*)(p->nextHop)),g,(*((unsigned long int*)(p->nextHop))==g) );
				//fprintf( stderr, "<%s:%d>%d==%d(%d)\n",__FUNCTION__,__LINE__, p->FWMetric, metric,( (p->FWMetric==-1 && metric==0) || (p->FWMetric==metric )	) );

				if(  (p->Enable) &&
				     ( (p->ifIndex==DUMMY_IFINDEX) || (strcmp(ifname, ifname2)==0) ) && //interface: any or specific
				     (*((unsigned long int*)(p->destID))==d) &&  //destIPaddress
				     (*((unsigned long int*)(p->netMask))==m) && //netmask
				     (*((unsigned long int*)(p->nextHop))==g) && //GatewayIPaddress
				     ( (p->FWMetric==-1 && metric==0) || (p->FWMetric==metric )	) //ForwardingMetric
				  )
				{
					//fprintf( stderr, "<%s:%d>Found\n",__FUNCTION__,__LINE__ );
					ret=1;//found
					break;
				}
			}
			
			if( feof(fp) && (ret!=1) )
			{
					//fprintf( stderr, "<%s:%d>Not Found\n",__FUNCTION__,__LINE__ );
					ret=0; //not found				
			}
			fclose(fp);
		}
	}

	//fprintf( stderr, "<%s:%d>End(ret:%d)\n",__FUNCTION__,__LINE__,ret );
	
	return ret;
}

unsigned int getForwardingInstNum( char *name )
{
	return getInstNum( name, "Forwarding" );
}

unsigned int findMaxForwardingInstNum(void)
{
	unsigned int ret=0, i,num;
	MIB_CE_IP_ROUTE_T *p,entity;
	
	num = mib_chain_total( MIB_IP_ROUTE_TBL );
	for( i=0; i<num;i++ )
	{
		p = &entity;
		if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ))
			continue;
		if( p->InstanceNum > ret )
			ret = p->InstanceNum;
	}	
	return ret;
}

int getForwardingEntryByInstNum( unsigned int instnum, MIB_CE_IP_ROUTE_T *p, unsigned int *id )
{
	int ret=-1;
	unsigned int i,num;
	
	if( (instnum==0) || (p==NULL) || (id==NULL) )
		return ret;
		
	num = mib_chain_total( MIB_IP_ROUTE_TBL );
	for( i=0; i<num;i++ )
	{
		if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ) )
			continue;

		if( p->InstanceNum==instnum )
		{
			*id = i;
			ret = 0;
			break;
		}
	}
	return ret;
}

#ifdef _PRMT_WT107_
int getDynamicForwardingTotalNum(void)
{
	int ret=0;
	char *routename="/proc/net/route";
	FILE *fp;
	
	fp=fopen( routename, "r" );
	if(fp)
	{
		char buff[256];
		
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL )
		{
			int flgs, ref, use, metric;
			char ifname[16], ifname2[16]; 
			unsigned long int d,g,m;

			if(sscanf(buff, "%s%lx%lx%X%d%d%d%lx",
				ifname, &d, &g, &flgs, &ref, &use, &metric, &m)!=8)
				break;//Unsuported kernel route format
			
			{
				unsigned int i,num, is_match_static=0;
				MIB_CE_IP_ROUTE_T *p,entity;
				
				num = mib_chain_total( MIB_IP_ROUTE_TBL );
				for( i=0; i<num;i++ )
				{
					p = &entity;
					if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ))
						continue;
						
					if( ifGetName( p->ifIndex, ifname2, sizeof(ifname2) )==0 ) //any interface
						ifname2[0]=0;
		
					if(  (p->Enable) &&
					     ( (p->ifIndex==DUMMY_IFINDEX) || (strcmp(ifname, ifname2)==0) ) && //interface: any or specific
					     (*((unsigned long int*)(p->destID))==d) &&  //destIPaddress
					     (*((unsigned long int*)(p->netMask))==m) && //netmask
					     (*((unsigned long int*)(p->nextHop))==g) && //GatewayIPaddress
					     ( (p->FWMetric==-1 && metric==0) || (p->FWMetric==metric )	) //ForwardingMetric
					  )
					{
						is_match_static=1; //static route
						break;
					}
					
				}				
				if(is_match_static==0) ret++; //dynamic route
			}
		}
		fclose(fp);
	}
	return ret;
}

int getDynamicForwardingEntryByInstNum( unsigned int instnum, MIB_CE_IP_ROUTE_T *pRoute )
{
	int ret=-1;
	char *routename="/proc/net/route";
	FILE *fp;
	
	if( (instnum==0) || (pRoute==NULL) ) return ret;
	memset( pRoute, 0, sizeof(MIB_CE_IP_ROUTE_T) );
	pRoute->InstanceNum = instnum;
	pRoute->FWMetric = -1;
	pRoute->ifIndex = DUMMY_IFINDEX;

	fp=fopen( routename, "r" );
	if(fp)
	{
		char buff[256];
		int count=0;
		
		fgets(buff, sizeof(buff), fp);
		while( fgets(buff, sizeof(buff), fp) != NULL )
		{
			int flgs, ref, use, metric;
			char ifname[16], ifname2[16]; 
			unsigned long int d,g,m;

			if(sscanf(buff, "%s%lx%lx%X%d%d%d%lx",
				ifname, &d, &g, &flgs, &ref, &use, &metric, &m)!=8)
				break;//Unsuported kernel route format
			
			{
				unsigned int i,num, is_match_static=0;
				MIB_CE_IP_ROUTE_T *p,entity;
				
				num = mib_chain_total( MIB_IP_ROUTE_TBL );
				for( i=0; i<num;i++ )
				{
					p = &entity;
					if( !mib_chain_get( MIB_IP_ROUTE_TBL, i, (void*)p ))
						continue;
						
					if( ifGetName( p->ifIndex, ifname2, sizeof(ifname2) )==0 ) //any interface
						ifname2[0]=0;

					if(  (p->Enable) &&
					     ( (p->ifIndex==DUMMY_IFINDEX) || (strcmp(ifname, ifname2)==0) ) && //interface: any or specific
					     (*((unsigned long int*)(p->destID))==d) &&  //destIPaddress
					     (*((unsigned long int*)(p->netMask))==m) && //netmask
					     (*((unsigned long int*)(p->nextHop))==g) && //GatewayIPaddress
					     ( (p->FWMetric==-1 && metric==0) || (p->FWMetric==metric )	) //ForwardingMetric
					  )
					{
						is_match_static=1; //static route
						break;
					}
				}
				if(is_match_static==0) count++; //dynamic route
			}
			
			if(count==instnum)
			{
			#if 1
				//CONFIG_PTMWAN, CONFIG_ETHWAN
				//if failed, transfer2IfIndxfromIfName returns DUMMY_IFINDEX
				pRoute->ifIndex = transfer2IfIndxfromIfName(ifname);
			#else
				if( (strncmp(ifname,"ppp",3)==0) || (strncmp(ifname,"vc",2)==0) )
					pRoute->ifIndex = transfer2IfIndxfromIfName(ifname);
				else
					pRoute->ifIndex = DUMMY_IFINDEX;
			#endif
				*((unsigned long int*)(pRoute->destID))=d;
				*((unsigned long int*)(pRoute->netMask))=m;
				*((unsigned long int*)(pRoute->nextHop))=g;
				pRoute->FWMetric=metric;
				pRoute->Enable = (flgs&RTF_UP)?1:0;
				if(flgs&RTF_HOST)
					pRoute->Type = 1; //host
				else if( (d==0) && (m==0) )
					pRoute->Type = 2; //default
				else
					pRoute->Type = 0; //network

				break;
			}
		}
		fclose(fp);
	}	
	
	return ret;
}
#endif //_PRMT_WT107_
#endif

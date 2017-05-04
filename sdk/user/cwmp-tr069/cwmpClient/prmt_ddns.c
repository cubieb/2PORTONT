#include "prmt_ddns.h"
#if defined(_PRMT_X_CT_COM_DDNS_) && defined(CONFIG_USER_DDNS)

unsigned int getDDNSInstNum( char *name );
int getDDNSCount( unsigned int ifindex );
int getDDNS( unsigned int ifindex, int id, MIB_CE_DDNS_T *c );
int updateDDNS( unsigned int ifindex, int id, MIB_CE_DDNS_T *p );
int delDDNS( unsigned int ifindex, int id );
int delSpecficDDNS( unsigned int ifindex );
void modifyDDNSIfIndex( unsigned int old_id, unsigned int new_id );
struct CWMP_NODE *getDDNS_PrmtEntity( char *name );

struct CWMP_OP tDDNSEntityLeafOP = { getDDNSEntity, setDDNSEntity };
struct CWMP_PRMT tDDNSEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"DDNSCfgEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"DDNSProvider",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"DDNSUsername",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"DDNSPassword",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"ServicePort",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"DDNSDomainName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP},
{"DDNSHostName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDDNSEntityLeafOP}
};
enum eDDNSEntityLeaf
{
	eDDNSCfgEnabled,
	eDDNSProvider,
	eDDNSUsername,
	eDDNSPassword,
	eServicePort,
	eDDNSDomainName,
	eDDNSHostName
};
struct CWMP_LEAF tDDNSEntityLeaf[] =
{
{ &tDDNSEntityLeafInfo[eDDNSCfgEnabled] },
{ &tDDNSEntityLeafInfo[eDDNSProvider] },
{ &tDDNSEntityLeafInfo[eDDNSUsername] },
{ &tDDNSEntityLeafInfo[eDDNSPassword] },
{ &tDDNSEntityLeafInfo[eServicePort] },
{ &tDDNSEntityLeafInfo[eDDNSDomainName] },
{ &tDDNSEntityLeafInfo[eDDNSHostName] },
{ NULL }
};


struct CWMP_PRMT tDDNSObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eDDNSObject
{
	eDDNS0
};
struct CWMP_LINKNODE tDDNSObject[] =
{
/*info,  			leaf,			next,		sibling,		instnum)*/
{&tDDNSObjectInfo[eDDNS0],	tDDNSEntityLeaf,	NULL,		NULL,			0},
};


int getDDNSEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int 	devnum, mapnum, ipnum, pppnum;
	unsigned int	chainid;
	int		ddns_chainid=0;
	MIB_CE_ATM_VC_T	*pEntry, vc_entity;
	MIB_CE_DDNS_T *pDDNS, ddns_entity;
	struct CWMP_NODE *pPrmtDDNS=NULL;


	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );
	mapnum = getDDNSInstNum( name );
	if( (devnum==0) || (mapnum==0) || ((ipnum==0)&&(pppnum==0))  ) return ERR_9005;

	pEntry = &vc_entity;
	if( getATMVCByInstNum( devnum, ipnum, pppnum, pEntry, &chainid )==NULL)
		return ERR_9005;

	pPrmtDDNS = getDDNS_PrmtEntity( name );
	if(pPrmtDDNS==NULL) return ERR_9005;
	ddns_chainid = getChainID( pPrmtDDNS->next, mapnum );
	if(ddns_chainid==-1) return ERR_9005;
	pDDNS = &ddns_entity;
	if( getDDNS( pEntry->ifIndex, ddns_chainid, pDDNS ) )
		return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "DDNSCfgEnabled" )==0 )
	{
		if( pDDNS->Enabled==0 )
			*data = booldup(0);
		else
			*data = booldup(1);
	}else if( strcmp( lastname, "DDNSProvider" )==0 )
	{
		*data = strdup( pDDNS->provider );
	}else if( strcmp( lastname, "DDNSUsername" )==0 )
	{
		*data = strdup( pDDNS->username );
	}else if( strcmp( lastname, "DDNSPassword" )==0 )
	{
#ifdef CONFIG_MIDDLEWARE
		unsigned char vChar;
		mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
		if(vChar == 0){
			*data = strdup( pDDNS->password );
		}else
#endif	//end of CONFIG_MIDDLEWARE
		*data = strdup( "" ); //strdup( pDDNS->password );
	}else if( strcmp( lastname, "ServicePort" )==0 )
	{
		char	strPort[16];
		sprintf( strPort, "%u", pDDNS->ServicePort );
		*data=strdup( strPort );
	}else if( strcmp( lastname, "DDNSDomainName" )==0 )
	{
		char *pch;

		pch=strstr( pDDNS->hostname, "." );
		if( pch )
		{
			pch++;
			*data=strdup( pch );
		}else
			*data=strdup( "" );
	}else if( strcmp( lastname, "DDNSHostName" )==0 )
	{
		char *pch;

		pch=strstr( pDDNS->hostname, "." );
		if( pch )
		{
			*pch=0;;
			*data=strdup( pDDNS->hostname );
		}else
			*data=strdup( "" );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setDDNSEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int 	devnum, mapnum, ipnum, pppnum;
	unsigned int	chainid;
	int		ddns_chainid=0;
	char		*buf=data;
	MIB_CE_ATM_VC_T	*pEntry, vc_entity;
	MIB_CE_DDNS_T *pDDNS, ddns_entity;
	struct CWMP_NODE *pPrmtDDNS=NULL;
	char		tmp[256]="";

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );
	mapnum = getDDNSInstNum( name );
	if( (devnum==0) || (mapnum==0) || ((ipnum==0)&&(pppnum==0))  ) return ERR_9005;

	pEntry = &vc_entity;
	if( getATMVCByInstNum( devnum, ipnum, pppnum, pEntry, &chainid )==NULL)
		return ERR_9005;

	pPrmtDDNS = getDDNS_PrmtEntity( name );
	if(pPrmtDDNS==NULL) return ERR_9005;
	ddns_chainid = getChainID( pPrmtDDNS->next, mapnum );
	if(ddns_chainid==-1) return ERR_9005;
	pDDNS = &ddns_entity;
	if( getDDNS( pEntry->ifIndex, ddns_chainid, pDDNS ) )
		return ERR_9002;

	if( strcmp( lastname, "DDNSCfgEnabled" )==0 )
	{
		int *i = data;

		pDDNS->Enabled = (*i==0)?0:1;
		updateDDNS( pEntry->ifIndex, ddns_chainid, pDDNS );

		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "DDNSProvider" )==0 )
	{
		if( buf==NULL || (strlen(buf)==0) ||  (strlen(buf)>=sizeof(pDDNS->provider)) )
			return ERR_9007;

		strcpy( pDDNS->provider, buf );
		updateDDNS( pEntry->ifIndex, ddns_chainid, pDDNS );

		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "DDNSUsername" )==0 )
	{
		if( buf==NULL || (strlen(buf)==0) ||  (strlen(buf)>=sizeof(pDDNS->username)) )
			return ERR_9007;

		strcpy( pDDNS->username, buf );
		updateDDNS( pEntry->ifIndex, ddns_chainid, pDDNS );

		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "DDNSPassword" )==0 )
	{
		if( buf==NULL || (strlen(buf)==0) || (strlen(buf)>=sizeof(pDDNS->password)) )
			return ERR_9007;

		strcpy( pDDNS->password, buf );
		updateDDNS( pEntry->ifIndex, ddns_chainid, pDDNS );

		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "ServicePort" )==0 )
	{
		int iPort=0;

		if( buf==NULL || (strlen(buf)==0) ) return ERR_9007;
		iPort = atoi(buf);
		if(iPort<=0 || iPort>=65535) return ERR_9007;
		pDDNS->ServicePort=(unsigned short)iPort;
		updateDDNS( pEntry->ifIndex, ddns_chainid, pDDNS );

		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "DDNSDomainName" )==0 )
	{
		char *pch;
		if( (buf==NULL) || (strlen(buf)==0) ) return ERR_9007;
		pch=strstr( pDDNS->hostname, "." );
		if(pch) *pch=0;
		strcpy( tmp, pDDNS->hostname );
		if( buf[0]!='.' ) strcat( tmp, "." );
		strcat( tmp, buf );
		if( strlen(tmp) >= sizeof(pDDNS->hostname) ) return ERR_9007;
		strcpy( pDDNS->hostname, tmp );
		updateDDNS( pEntry->ifIndex, ddns_chainid, pDDNS );

		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "DDNSHostName" )==0 )
	{
		char *pch;
		if( (buf==NULL) || (strlen(buf)==0) ) return ERR_9007;
		strcat( tmp, buf );
		if( pDDNS->hostname[0]=='.' )
			strcat( tmp, pDDNS->hostname );
		else{
			pch=strstr( pDDNS->hostname, "." );
			if(pch) strcat( tmp, pch );
		}
		if( strlen(tmp) >= sizeof(pDDNS->hostname) ) return ERR_9007;
		strcpy( pDDNS->hostname, tmp );
		updateDDNS( pEntry->ifIndex, ddns_chainid, pDDNS );

		apply_add( CWMP_PRI_L, apply_DDNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else{
		return ERR_9005;
	}

	return 0;
}

int objDDNS(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	unsigned int devnum,ipnum,pppnum;
	unsigned int chainid;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	if( (name==NULL) || (entity==NULL) ) return -1;

	devnum = getWANConDevInstNum( name );
	ipnum  = getWANIPConInstNum( name );
	pppnum = getWANPPPConInstNum( name );
	if( (devnum==0) || ((ipnum==0)&&(pppnum==0))  ) return ERR_9005;

	pEntry = &vc_entity;
	if( getATMVCByInstNum( devnum, ipnum, pppnum, pEntry, &chainid )==NULL)
		return ERR_9005;

	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			int num=0, MaxInstNum=0, i;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
			MIB_CE_DDNS_T *p, ddns_entity;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = getDDNSCount( pEntry->ifIndex );
			for( i=0; i<num;i++ )
			{
				p = &ddns_entity;
				if( getDDNS( pEntry->ifIndex, i, p ) )
					continue;

				if( p->InstanceNum==0 ) //maybe createn by web or cli
				{
					MaxInstNum++;
					p->InstanceNum = MaxInstNum;
					updateDDNS( pEntry->ifIndex, i, p );
				}else
					MaxInstNum = p->InstanceNum;
				if( create_Object( c, tDDNSObject, sizeof(tDDNSObject), 1, MaxInstNum ) < 0 )
					return -1;
				//c = & (*c)->sibling;
			}
			add_objectNum( name, MaxInstNum );
			return 0;
		}
		case eCWMP_tADDOBJ:
		{
			int ret;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDDNSObject, sizeof(tDDNSObject), data );
			if( ret >= 0 )
			{
				MIB_CE_DDNS_T entry;
			     	char vCh=0;

				memset( &entry, 0, sizeof( MIB_CE_DDNS_T ) );
				{ //default values for this new entry
					char ifname[16]="";
					ifGetName( pEntry->ifIndex, ifname, 16 );
					strcpy( entry.interface, ifname );
	#ifdef CONFIG_BOA_WEB_E8B_CH
					char wanname[30];
					getWanName(pEntry,wanname);
					strcpy(entry.ifname,wanname);
	#endif
					//entry.ifIndex=pEntry->ifIndex;
					entry.InstanceNum= *(int*)data;
				}
				mib_chain_add( MIB_DDNS_TBL, (unsigned char*)&entry );
			}
			return ret;
		}
		case eCWMP_tDELOBJ:
		{
			int ret, id;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			id = getChainID( entity->next, *(int*)data );
			if(id==-1) return ERR_9005;

			delDDNS( pEntry->ifIndex, id );
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			return ret;
		}
		case eCWMP_tUPDATEOBJ:
		{
			int num=0, i;
			struct CWMP_LINKNODE *old_table;

			//CWMPDBG( 0, ( stderr, "<%s:%d>action=eCWMP_tUPDATEOBJ(name=%s)\n", __FUNCTION__, __LINE__, name ) );
			num = getDDNSCount( pEntry->ifIndex );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;
				MIB_CE_DDNS_T *p, ddns_entity;

				p = &ddns_entity;
				if( getDDNS( pEntry->ifIndex, i, p )<0 )
					continue;

				remove_entity = remove_SiblingEntity( &old_table, p->InstanceNum );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}else{
					unsigned int MaxInstNum=p->InstanceNum;

					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDDNSObject, sizeof(tDDNSObject), &MaxInstNum );
					if(MaxInstNum!=p->InstanceNum)
					{
						p->InstanceNum = MaxInstNum;
						updateDDNS( pEntry->ifIndex, i, p );
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
unsigned int getDDNSInstNum( char *name )
{
	return getInstNum( name, "X_CT-COM_DDNSConfiguration" );
}

/*portmapping utilities*/
int getDDNSCount( unsigned int ifindex )
{
	int count = 0;
	MIB_CE_DDNS_T *p, ddns_entity;
	unsigned int total,i;
	char ifname[16];

	if( ifGetName( ifindex, ifname, 16 ) )
	{
		total = mib_chain_total( MIB_DDNS_TBL );
		for( i=0;i<total;i++ )
		{
			p = &ddns_entity;
			if( mib_chain_get( MIB_DDNS_TBL, i, (void*)p ) )
				if( strcmp(p->interface, ifname)==0 ) count++;
				//if(p->ifIndex==ifindex) count++;
		}
	}

	return count;
}

/*id starts from 0*/
int getDDNS( unsigned int ifindex, int id, MIB_CE_DDNS_T *c )
{
	unsigned int total,i;
	char ifname[16];

	if( (id < 0) || (c==NULL) ) return -1;

	if( ifGetName( ifindex, ifname, 16 ) )
	{
		total = mib_chain_total( MIB_DDNS_TBL );
		for( i=0;i<total;i++ )
		{
			if( !mib_chain_get( MIB_DDNS_TBL, i, (void*)c ) )
				continue;

			//if(c->ifIndex==ifindex)
			if( strcmp(c->interface, ifname)==0 )
			{
				id--;
				if(id==-1)
				{
		     			return 0;
		     		}
			}
		}
	}

	return -1;
}

int updateDDNS( unsigned int ifindex, int id, MIB_CE_DDNS_T *p )
{
	int ret = -1;
	unsigned int total,i;
	char ifname[16];

	if( id < 0) return ret;

	if( ifGetName( ifindex, ifname, 16 ) )
	{
		total = mib_chain_total( MIB_DDNS_TBL );
		for( i=0;i<total;i++ )
		{
			MIB_CE_DDNS_T *c, ddns_entity;
			c = &ddns_entity;
			if( !mib_chain_get( MIB_DDNS_TBL, i, (void*)c ) )
				continue;

			//if(c->ifIndex==ifindex)
			if( strcmp(c->interface, ifname)==0 )
			{
				id--;
				if(id==-1)
				{
					mib_chain_update( MIB_DDNS_TBL, (unsigned char*)p, i );
					ret=0;
					break;
				}
			}
		}
	}
	return ret;
}

int delDDNS( unsigned int ifindex, int id )
{
	int ret = -1;
	unsigned int total,i;
	char ifname[16];

	if( id < 0) return ret;

	if( ifGetName( ifindex, ifname, 16 ) )
	{
		total = mib_chain_total( MIB_DDNS_TBL );
		for( i=0;i<total;i++ )
		{
			MIB_CE_DDNS_T *c, ddns_entity;
			c = &ddns_entity;
			if( !mib_chain_get( MIB_DDNS_TBL, i, (void*)c ) )
				continue;

			//if(c->ifIndex==ifindex)
			if( strcmp(c->interface, ifname)==0 )
			{
				id--;
				if(id==-1)
				{
					mib_chain_delete( MIB_DDNS_TBL, i );
					ret=0;
					break;
				}
			}
		}
	}

	return ret;
}

int delSpecficDDNS( unsigned int ifindex )
{
	int total,i;
	char ifname[16];

	if( ifGetName( ifindex, ifname, 16 ) )
	{
		total = mib_chain_total( MIB_DDNS_TBL );
		//for( i=0;i<total;i++ )
		for( i=total-1;i>=0;i-- )
		{
			MIB_CE_DDNS_T *c, ddns_entity;
			c = &ddns_entity;
			if( !mib_chain_get( MIB_DDNS_TBL, i, (void*)c ) )
				continue;

			//if(c->ifIndex==ifindex)
			if( strcmp(c->interface, ifname)==0 )
				mib_chain_delete( MIB_DDNS_TBL, i );
		}
	}

	return 0;
}

void modifyDDNSIfIndex( unsigned int old_id, unsigned int new_id )
{
	unsigned int total,i;
	char new_ifname[16];
	char old_ifname[16];

	if( ifGetName( old_id, old_ifname, 16 ) && ifGetName( new_id, new_ifname, 16 ) )
	{

		total = mib_chain_total( MIB_DDNS_TBL );
		for( i=0;i<total;i++ )
		{
			MIB_CE_DDNS_T *c, ddns_entity;
			c = &ddns_entity;
			if( !mib_chain_get( MIB_DDNS_TBL, i, (void*)c ) )
				continue;

			//if(c->ifIndex==old_id)
			if( strcmp(c->interface, old_ifname)==0 )
			{
				//c->ifIndex = new_id;
				strcpy( c->interface, new_ifname );
				mib_chain_update( MIB_DDNS_TBL, (unsigned char*)c, i );
			}
		}
	}

	return;
}

struct CWMP_NODE *getDDNS_PrmtEntity( char *name )
{
	struct CWMP_NODE *p=NULL;
	char *dupname;
	char strfmt[]=".X_CT-COM_DDNSConfiguration.";
	char *tok;

	dupname = strdup( name );
	if( dupname )
	{
		tok = strstr( dupname, strfmt );
		if(tok)
		{
			dupname[ tok+strlen(strfmt)-dupname ]=0;
			get_ParameterEntity(dupname, (struct CWMP_LEAF **)&p);
		}
		free( dupname );
	}

	return p;
}

#endif /*_PRMT_X_CT_COM_DDNS_*/

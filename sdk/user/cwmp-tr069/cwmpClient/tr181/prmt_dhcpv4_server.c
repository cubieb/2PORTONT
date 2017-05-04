#include <rtk/mib.h>
#include <rtk/utility.h>

#include <cwmp_utility.h>
#include "../prmt_landevice.h"
#include "prmt_dhcpv4_server.h"

/***** Device.DHCPv4.Server.Pool.{i} *****************************************/
struct CWMP_OP tDHCPv4PoolOptEntityLeafOP = { getDHCPv4PoolOptEntity, setDHCPv4PoolOptEntity };
struct CWMP_PRMT tDHCPv4PoolOptEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4PoolOptEntityLeafOP},
{"Tag",		eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4PoolOptEntityLeafOP},
{"Value",	eCWMP_tHEXBIN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4PoolOptEntityLeafOP},
};

enum eDHCPv4PoolOptEntityLeaf
{
	eDHCPv4PoolOptEnable,
	eDHCPv4PoolOptTag,
	eDHCPv4PoolOptValue,
};

struct CWMP_LEAF tDHCPv4PoolOptEntityLeaf[] =
{
{ &tDHCPv4PoolOptEntityLeafInfo[eDHCPv4PoolOptEnable]  },
{ &tDHCPv4PoolOptEntityLeafInfo[eDHCPv4PoolOptTag]  },
{ &tDHCPv4PoolOptEntityLeafInfo[eDHCPv4PoolOptValue]  },
{ NULL	}
};

/***** Device.DHCPv4.Server.Pool.{i}.Option **********************************/
struct CWMP_OP tDHCPv4PoolOptOP = { NULL, objDHCPv4PoolOpt };
struct CWMP_PRMT tDHCPv4PoolOjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};

enum eDHCPv4PoolOptOjbect
{
	eDHCPv4Pool0
};

struct CWMP_LINKNODE tDHCPv4PoolOptObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tDHCPv4PoolOjbectInfo[eDHCPv4Pool0],	tDHCPv4PoolOptEntityLeaf,	NULL,	NULL,	0},
};

/***** Device.DHCPv4.Server.Pool.{i} *****************************************/
struct CWMP_OP tDHCPv4ServerPoolEntityLeafOP = { getDHCPv4ServerPoolEntity, setDHCPv4ServerPoolEntity };
struct CWMP_PRMT tDHCPv4ServerPoolEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,	&tDHCPv4ServerPoolEntityLeafOP},
{"Order",			eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"Interface",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"VendorClassID",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"VendorClassIDExclude",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"VendorClassIDMode",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"ClientID",				eCWMP_tHEXBIN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"ClientIDExclude",			eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"UserClassID", 			eCWMP_tHEXBIN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"UserClassIDExclude",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"Chaddr",					eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"ChaddrMask",				eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"ChaddrMaskExclude",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"MinAddress",				eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"MaxAddress",				eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"ReservedAddresses",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"SubnetMask",				eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"DNSServers",				eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"DomainName",				eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"IPRouters",				eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"LeaseTime",				eCWMP_tINT,		CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
{"OptionNumberOfEntries",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolEntityLeafOP},
};

enum eDHCPv4ServerPoolEntityLeaf
{
	eDHCPv4ServerPoolEnable,
	eDHCPv4ServerPoolStatus,
	eDHCPv4ServerPoolOrder,
	eDHCPv4ServerPoolInterface,
	eDHCPv4ServerPoolVendorClassID,
	eDHCPv4ServerPoolVendorClassIDExclude,
	eDHCPv4ServerPoolVendorClassIDMode,
	eDHCPv4ServerPoolClientID,
	eDHCPv4ServerPoolClientIDExclude,
	eDHCPv4ServerPoolUserClassID,
	eDHCPv4ServerPoolUserClassIDExclude,
	eDHCPv4ServerPoolChaddr,
	eDHCPv4ServerPoolChaddrMask,
	eDHCPv4ServerPoolChaddrMaskExclude,
	eDHCPv4ServerPoolMinAddress,
	eDHCPv4ServerPoolMaxAddress,
	eDHCPv4ServerPoolReservedAddresses,
	eDHCPv4ServerPoolSubnetMask,
	eDHCPv4ServerPoolDNSServers,
	eDHCPv4ServerPoolDomainName,
	eDHCPv4ServerPoolIPRouters,
	eDHCPv4ServerPoolLeaseTime,
	eDHCPv4ServerPoolOptionNumberOfEntries,
};

struct CWMP_LEAF tDHCPv4ServerPoolEntityLeaf[] =
{
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolEnable]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolStatus]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolOrder]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolInterface]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolVendorClassID]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolVendorClassIDExclude]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolVendorClassIDMode]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolClientID]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolClientIDExclude]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolUserClassID]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolUserClassIDExclude]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolChaddr]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolChaddrMask]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolChaddrMaskExclude]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolMinAddress]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolMaxAddress]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolReservedAddresses]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolSubnetMask]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolDNSServers]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolDomainName]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolIPRouters]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolLeaseTime]  },
{ &tDHCPv4ServerPoolEntityLeafInfo[eDHCPv4ServerPoolOptionNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tDHCPv4ServerPoolEntityObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Option",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4PoolOptOP},
};

enum eDHCPv4ServerPoolEntityObject
{
	eDHCPv4ServerPoolOption,
};

struct CWMP_NODE tDHCPv4ServerPoolEntityObject[] =
{
/*info,  					leaf,			node)*/
{&tDHCPv4ServerPoolEntityObjectInfo[eDHCPv4ServerPoolOption],	NULL,	NULL},
{NULL,						NULL,			NULL}
};

/***** Device.DHCPv4.Server.Pool.{i}. ****************************************/
struct CWMP_OP tDHCPv4ServerPoolOP = { NULL, objDHCPv4ServerPool };
struct CWMP_PRMT tDHCPv4ServerPoolOjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};

enum eDHCPv4ServerPoolOjbect
{
	eDHCPv4ServerPool0
};

struct CWMP_LINKNODE tDHCPv4ServerPoolObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tDHCPv4ServerPoolOjbectInfo[eDHCPv4ServerPool0],	tDHCPv4ServerPoolEntityLeaf,	tDHCPv4ServerPoolEntityObject,	NULL,	0},
};

/***** Device.DHCPv4.Server **************************************************/
struct CWMP_OP tDHCPv4ServerLeafOP = { getDHCPv4Server, setDHCPv4Server };
struct CWMP_PRMT tDHCPv4ServerLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerLeafOP},
{"PoolNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tDHCPv4ServerLeafOP},
};

enum eDHCPv4ServerLeaf
{
	eDHCPv4ServerEnabled,
	eDHCPv4ServerPoolNumberOfEntries,
};

struct CWMP_LEAF tDHCPv4ServerLeaf[] =
{
{ &tDHCPv4ServerLeafInfo[eDHCPv4ServerEnabled]  },
{ &tDHCPv4ServerLeafInfo[eDHCPv4ServerPoolNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tDHCPv4ServerClientObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Pool",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ServerPoolOP},
};

enum eDHCPv4ServerObject
{
	eDHCPv4ServerPool,
};

struct CWMP_NODE tDHCPv4ServerObject[] =
{
/*info,  					leaf,			node)*/
{&tDHCPv4ServerClientObjectInfo[eDHCPv4ServerPool],	NULL,	NULL},
{NULL,						NULL,			NULL}
};

/***** Operations ************************************************************/
int getDHCPv4Server(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		unsigned char enable;

		mib_get( MIB_DHCP_MODE, (void *)&enable);
		*data = booldup( (enable == DHCP_LAN_SERVER)? 1 : 0 );
	}
	else if( strcmp( lastname, "PoolNumberOfEntries" )==0 )
	{
		// first one is default pool
		*data = uintdup(1 + mib_chain_total(MIB_DHCPS_SERVING_POOL_TBL));		
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setDHCPv4Server(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	struct in_addr addr;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned char enable;

		if( i==NULL ) return ERR_9007;
		enable = (*i==0) ? 0 : DHCP_LAN_SERVER;
		mib_set(MIB_DHCP_MODE, (void *)&enable);

		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objDHCPv4ServerPool(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	DHCPS_SERVING_POOL_T *pDHCPSPEntry, DHCPSPEntry;
	unsigned int chainid;
	unsigned int num,i;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch(type)
	{
		case eCWMP_tINITOBJ:
			return 0;
		case eCWMP_tADDOBJ:
			{
				int ret;
				char tmpbuf[128];

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

				ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPv4ServerPoolObject, sizeof(tDHCPv4ServerPoolObject), data );
				if( ret >= 0 )
				{
					DHCPS_SERVING_POOL_T entry;
					memset( &entry, 0, sizeof( DHCPS_SERVING_POOL_T ) );
					{ //default values for this new entry
						entry.enable = 0;
						entry.poolorder = findMaxDHCPConSPOrder() + 1;
						sprintf(tmpbuf,"servingpool%d",*(int*)data - 1); //compatable with TR-098
						strcpy(entry.poolname,tmpbuf);
						entry.leasetime=86400;
						entry.InstanceNum = *(int *)data - 1;	//compatable with TR-098
						entry.localserved = 1;//default: locallyserved=true;
						memset(entry.chaddrmask,0xff,MAC_ADDR_LEN);//default to all 0xff
						strncpy(entry.vendorclassmode,"Substring",MODE_LEN-1);
						entry.vendorclassmode[MODE_LEN-1]=0;
					}
					mib_chain_add( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)&entry);
				}
				return ret;
			}
		case eCWMP_tDELOBJ:
			{
				int ret, num, i;
				int found = 0;
				unsigned int *pUint=data;

				if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

				//Default server pool cannot be deleleted
				if(*pUint == 1)
					return ERR_9001;

				pDHCPSPEntry = &DHCPSPEntry;

				num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
				for( i=0; i<num;i++ )
				{
					if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)pDHCPSPEntry ) )
						continue;
					if(pDHCPSPEntry->InstanceNum + 1 == *pUint)
					{
						found =1;
						clearOptTbl(pDHCPSPEntry->InstanceNum);
#if defined(CONFIG_USER_DHCP_SERVER) && defined(SUPPORT_DHCP_RESERVED_IPADDR)
						clearDHCPReservedIPAddrByInstNum( pDHCPSPEntry->InstanceNum );
#endif
						mib_chain_delete( MIB_DHCPS_SERVING_POOL_TBL, i );
						compact_poolorder();
						break;
					}
				}

				if(found==0) return ERR_9005;
				ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
				apply_DHCP(CWMP_RESTART,0,NULL);

				return ret;
			}
		case eCWMP_tUPDATEOBJ:
			{
				int num,i;
				struct CWMP_LINKNODE *old_table;
				struct CWMP_LINKNODE *remove_entity=NULL;

				pDHCPSPEntry = &DHCPSPEntry;
				
				old_table = (struct CWMP_LINKNODE *)entity->next;
				entity->next = NULL;

				// Default DHCP server pool
				remove_entity = remove_SiblingEntity( &old_table, 1 );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, 1 )==NULL )
					{
						unsigned int MaxInstNum = 1;
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPv4ServerPoolObject, sizeof(tDHCPv4ServerPoolObject), &MaxInstNum );
					}
				}

				// conditional serving pool
				num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );
				for( i=0; i<num;i++ )
				{
					if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)pDHCPSPEntry ))
						continue;

					remove_entity = remove_SiblingEntity( &old_table, pDHCPSPEntry->InstanceNum + 1 );
					if( remove_entity!=NULL )
					{
							add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}
					else
					{
							if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, pDHCPSPEntry->InstanceNum + 1)==NULL )
							{
								unsigned int MaxInstNum = pDHCPSPEntry->InstanceNum + 1;
								add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPv4ServerPoolObject, sizeof(tDHCPv4ServerPoolObject), &MaxInstNum );
								if(MaxInstNum != pDHCPSPEntry->InstanceNum + 1)
								{
									pDHCPSPEntry->InstanceNum = MaxInstNum - 1;
									mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, i );
								}
							}//else already in next_table
					}
				}

				if( old_table )
					destroy_ParameterTable( (struct CWMP_NODE *)old_table );

				return 0;
			}
	}
}

int getDHCPv4ServerPoolEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;;
	unsigned int chainid;
	unsigned int inst_num;
	unsigned char default_pool = 0;
	int num,i;
	char buf[256] = {0};
	int ret=0;
	DHCPS_SERVING_POOL_T *pDHCPSPEntry, DhcpSPEntry;

	*type = entity->info->type;
	*data = NULL;

	pDHCPSPEntry = &DhcpSPEntry;
	inst_num = getInstNum( name, "DHCPv4.Server.Pool" );

	if(inst_num == 1)
		default_pool = 1;
	else
	{
		ret = getDHCPConSPByInstNum(inst_num - 1, pDHCPSPEntry, &chainid);
		if(ret<0) return ERR_9002;
	}

	if( strcmp( lastname, "Enable" )==0 )
	{
		if(default_pool)
			*data = booldup(1);	// default pool
		else
			*data = booldup(pDHCPSPEntry->enable);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		if(default_pool)
			*data = strdup("Enabled");	// default pool
		else
			*data = strdup((pDHCPSPEntry->enable) ? "Enabled" : "Disabled");
	}
	else if( strcmp( lastname, "Order" )==0 )
	{
		if(default_pool)
			*data = uintdup(findMaxDHCPConSPOrder() + 1);	// default is last
		else
			*data = uintdup(pDHCPSPEntry->poolorder);
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		*data = strdup("Device.IP.Interface.1");
	}
	else if( strcmp( lastname, "VendorClassID" )==0 )
	{
		if(default_pool)
			*data = strdup("");
		else
			*data = strdup(pDHCPSPEntry->vendorclass);
	}
	else if( strcmp( lastname, "VendorClassIDExclude" )==0 )
	{
		if(default_pool)
			*data = booldup(0);
		else
			*data = booldup(pDHCPSPEntry->vendorclassflag);
	}
	else if( strcmp( lastname, "VendorClassIDMode" )==0 )
	{
		if(default_pool)
			*data = strdup("Exact");
		else
			*data = strdup(pDHCPSPEntry->vendorclassmode);
	}
	else if( strcmp( lastname, "ClientID" )==0 )
	{
		struct xsd__hexBinary id = {0};
		if(!default_pool)
		{
			int len = strlen(pDHCPSPEntry->clientid);
			if(len > 0)
			{
				id.__size = len/2;
				id.__ptr = malloc(id.__size);
				string_to_hex(pDHCPSPEntry->clientid, id.__ptr, len);
			}
		}

		*data = (void *)hexBinarydup(id);
		if(id.__ptr)
			free(id.__ptr);
	}
	else if( strcmp( lastname, "ClientIDExclude" )==0 )
	{
		if(default_pool)
			*data = booldup(0);
		else
			*data = booldup(pDHCPSPEntry->clientidflag);
	}
	else if( strcmp( lastname, "UserClassID" )==0 )
	{
		struct xsd__hexBinary id = {0};
		if(!default_pool)
		{
			int len = strlen(pDHCPSPEntry->userclass);
			if(len > 0)
			{
				id.__size = len/2;
				id.__ptr = malloc(id.__size);
				string_to_hex(pDHCPSPEntry->userclass, id.__ptr, len);
			}
		}

		*data = (void *)hexBinarydup(id);
		if(id.__ptr)
			free(id.__ptr);
	}
	else if( strcmp( lastname, "UserClassIDExclude" )==0 )
	{
		if(default_pool)
			*data = booldup(0);
		else
			*data = booldup(pDHCPSPEntry->userclassflag);
	}
	else if( strcmp( lastname, "Chaddr" )==0 )
	{
		if(!default_pool)
		{
			//00:00:00:00:00:00 returns an empty string, not used for conditional serving
			if( (pDHCPSPEntry->chaddr[0]==0) &&
				(pDHCPSPEntry->chaddr[1]==0) &&
				(pDHCPSPEntry->chaddr[2]==0) &&
				(pDHCPSPEntry->chaddr[3]==0) &&
				(pDHCPSPEntry->chaddr[4]==0) &&
				(pDHCPSPEntry->chaddr[5]==0) )
			  	buf[0]=0;
			else
				sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",pDHCPSPEntry->chaddr[0],pDHCPSPEntry->chaddr[1],pDHCPSPEntry->chaddr[2],
					pDHCPSPEntry->chaddr[3],pDHCPSPEntry->chaddr[4],pDHCPSPEntry->chaddr[5]);
		}
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "ChaddrMask" )==0 )
	{
		if(!default_pool)
		{
			sprintf(buf,"%02x:%02x:%02x:%02x:%02x:%02x",pDHCPSPEntry->chaddrmask[0],pDHCPSPEntry->chaddrmask[1],pDHCPSPEntry->chaddrmask[2],
				pDHCPSPEntry->chaddrmask[3],pDHCPSPEntry->chaddrmask[4],pDHCPSPEntry->chaddrmask[5]);
		}
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "ChaddrExclude" )==0 )
	{
		if(default_pool)
			*data = booldup(0);
		else
			*data = booldup(pDHCPSPEntry->chaddrflag);
	}
	else if( strcmp( lastname, "MinAddress" )==0 )
	{
		if(default_pool)
			getMIB2Str(MIB_DHCP_POOL_START, buf);
		else
			strcpy(buf,inet_ntoa(*((struct in_addr *)pDHCPSPEntry->startaddr)));

		*data=strdup( buf );
	}
	else if( strcmp( lastname, "MaxAddress" )==0 )
	{
		if(default_pool)
			getMIB2Str(MIB_DHCP_POOL_END, buf);
		else
			strcpy(buf,inet_ntoa(*((struct in_addr *)pDHCPSPEntry->endaddr)));

		*data=strdup( buf );
	}
	else if( strcmp( lastname, "ReservedAddresses" )==0 )
	{
		int num;

		num = (default_pool) ? 0 : pDHCPSPEntry->InstanceNum;

		if( getDHCPReservedIPAddr(num , FILE4DHCPReservedIPAddr )==0 )
		{
			*type = eCWMP_tFILE; /*special case*/
			*data=strdup( FILE4DHCPReservedIPAddr );
		}
		else
			*data=strdup( "" );
	}
	else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		if(default_pool)
			getMIB2Str(MIB_DHCP_SUBNET_MASK, buf);
		else
			strcpy(buf,inet_ntoa(*((struct in_addr *)pDHCPSPEntry->subnetmask)));

		*data=strdup( buf );
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{
		unsigned char mode;

		if(default_pool)
			mib_get( MIB_DHCP_DNS_OPTION, (void *)&mode);
		else
			mode = pDHCPSPEntry->dnsservermode;

		if( mode == 0 ) // DNS Relay
			*data=strdup("");
		else
		{
			if(default_pool)
				getDNSList(buf, 1);
			else
				getSPDNSList(pDHCPSPEntry,buf);

			*data = strdup( buf );
		}
	}
	else if( strcmp( lastname, "DomainName" )==0 )
	{
		if(default_pool)
		{
			getMIB2Str(MIB_ADSL_LAN_DHCP_DOMAIN, buf);
			*data=strdup( buf );
		}
		else
			*data=strdup( pDHCPSPEntry->domainname );
	}
	else if( strcmp( lastname, "IPRouters" )==0 )
	{
		if(default_pool)
			getMIB2Str(MIB_ADSL_LAN_DHCP_GATEWAY, buf);
		else
			strcpy(buf,inet_ntoa(*((struct in_addr *)pDHCPSPEntry->iprouter)));

		*data=strdup( buf );
	}
	else if( strcmp( lastname, "LeaseTime" )==0 )
	{
		int time;

		if(default_pool)
			mib_get(MIB_ADSL_LAN_DHCP_LEASE, (void *)&time);
		else
			time = pDHCPSPEntry->leasetime;

		*data=intdup( time );
	}
	else if( strcmp( lastname, "OptionNumberOfEntries" )==0 )
	{
		unsigned int usedFor;

		if(default_pool)
		{
			usedFor=eUsedFor_DHCPServer;
			*data = uintdup(getDHCPOptEntryNum(usedFor));
		}
		else
		{
			usedFor=eUsedFor_DHCPServer_ServingPool;
			*data = uintdup(getSPDHCPOptEntryNum(usedFor, inst_num));
		}
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

int setDHCPv4ServerPoolEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int chainid;
	unsigned int pool_num;
	int num,i;
	int ret=0;
	char *buf=data;
	DHCPS_SERVING_POOL_T *pDHCPSPEntry, DhcpSPEntry;
	char tmpbuf[30]={0};
	char *tok, del[] = ", ", *pstr;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	pool_num = getInstNum( name, "DHCPv4.Server.Pool" );
	if(pool_num != 1)
	{
		pDHCPSPEntry = &DhcpSPEntry;
		ret = getDHCPConSPByInstNum(pool_num - 1, pDHCPSPEntry, &chainid);
		if(ret<0) return ERR_9002;
	}

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		
		//cannot disable default pool
		if(pool_num == 1 && *i == 0) return ERR_9001;

		if(pool_num == 1 && *i == 1) return 0;

		pDHCPSPEntry->enable = (*i==0) ? 0 : 1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "Order" )==0 )
	{
		unsigned int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<1) return ERR_9007;
		if(pool_num == 1) return ERR_9001;	//default pool is always the last one
		
		if(checkandmodify_poolorder(*i,chainid)<0) return ERR_9007;
		pDHCPSPEntry->poolorder = *i;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
		compact_poolorder();

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(strcmp(buf, "Device.IP.Interface.1") != 0)
			return ERR_9001;

		return 0;

	}
	else if( strcmp( lastname, "VendorClassID" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if(pool_num == 1) return ERR_9001;

		if( strlen(buf)==0)
			memset(pDHCPSPEntry->vendorclass,0,OPTION_60_LEN+1);
		else
		{
			strncpy(pDHCPSPEntry->vendorclass,buf,OPTION_60_LEN);
			pDHCPSPEntry->vendorclass[OPTION_60_LEN]=0;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "VendorClassIDExclude" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(pool_num == 1) return ERR_9001;

		pDHCPSPEntry->vendorclassflag = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "VendorClassIDMode" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if(pool_num == 1) return ERR_9001;

		if(strcmp(buf,"Exact")&&strcmp(buf,"Prefix")&&strcmp(buf,"Suffix")&&strcmp(buf,"Substring"))
			return ERR_9007;
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->vendorclassmode,0,MODE_LEN);
		else{
			strncpy(pDHCPSPEntry->vendorclassmode,buf,MODE_LEN-1);
			pDHCPSPEntry->vendorclassmode[MODE_LEN-1]=0;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "ClientID" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if(pool_num == 1) return ERR_9001;
		
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->clientid,0,OPTION_LEN);
		else{
			strncpy(pDHCPSPEntry->clientid,buf,OPTION_LEN-1);
			pDHCPSPEntry->clientid[OPTION_LEN-1]=0;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "ClientIDExclude" )==0 ){
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(pool_num == 1) return ERR_9001;

		pDHCPSPEntry->clientidflag = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "UserClassID" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if(pool_num == 1) return ERR_9001;

		if( strlen(buf)==0)
			memset(pDHCPSPEntry->userclass,0,OPTION_LEN);
		else{
			strncpy(pDHCPSPEntry->userclass,buf,OPTION_LEN-1);
			pDHCPSPEntry->userclass[OPTION_LEN-1]=0;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "UserClassIDExclude" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(pool_num == 1) return ERR_9001;

		pDHCPSPEntry->userclassflag = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "Chaddr" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if(pool_num == 1) return ERR_9001;

		printf("\nbuf=%s\n",buf);
		//00:00:00:00:00:00 or an empty string means "not used for conditional serving"
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->chaddr,0,MAC_ADDR_LEN);
		else{
			if( strlen(buf)!=17) return ERR_9007;
			if(compactmacaddr(tmpbuf,buf)==0) return ERR_9007;
			if(string_to_hex(tmpbuf,pDHCPSPEntry->chaddr,12)==0) return ERR_9007;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "ChaddrMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if(pool_num == 1) return ERR_9001;

		//FF:FF:FF:FF:FF:FF or an empty string indicates all bits of the Chaddr are to be used for conditional serving classification
		if( strlen(buf)==0)
			memset(pDHCPSPEntry->chaddrmask,0xff,MAC_ADDR_LEN);
		else{
			if(strlen(buf)!=17) return ERR_9007;
			if(compactmacaddr(tmpbuf,buf)==0) return ERR_9007;
			if(string_to_hex(tmpbuf,pDHCPSPEntry->chaddrmask,12)==0) return ERR_9007;
		}
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "ChaddrExclude" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(pool_num == 1) return ERR_9001;

		pDHCPSPEntry->chaddrflag = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "MinAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(pool_num == 1)
		{
			struct in_addr addr = {0};

			if( strlen(buf)==0 ) return ERR_9007;

			if( inet_aton( buf, &addr )==0 ) //the ip address is error.
				return ERR_9007;
			
			mib_set( MIB_DHCP_POOL_START, (void *)&addr);
		}
		else
		{
			if( strlen(buf)==0)
				memset(pDHCPSPEntry->startaddr,0,IP_ADDR_LEN);
			else
				if(!inet_aton(buf, (struct in_addr *)&pDHCPSPEntry->startaddr)) return ERR_9007;

			mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
		}

		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "MaxAddress" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(pool_num == 1)
		{
			struct in_addr addr = {0};

			if( strlen(buf)==0 ) return ERR_9007;

			if( inet_aton( buf, &addr )==0 ) //the ip address is error.
				return ERR_9007;
			
			mib_set( MIB_DHCP_POOL_END, (void *)&addr);
		}
		else
		{
			if( strlen(buf)==0)
				memset(pDHCPSPEntry->endaddr,0,IP_ADDR_LEN);
			else
				if(!inet_aton(buf, (struct in_addr *)&pDHCPSPEntry->endaddr)) return ERR_9007;

			mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
		}

		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "ReservedAddresses" )==0 )
	{
		if( setDHCPReservedIPAddr(pool_num - 1,buf)<0 ) return ERR_9007;

		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(pool_num == 1)
		{
			struct in_addr addr = {0};

			if( strlen(buf)==0 ) return ERR_9007;
			if( inet_aton( buf, &addr )==0 ) //the ip address is error.
				return ERR_9007;

			mib_set(MIB_DHCP_SUBNET_MASK, (void *)&addr);
		}
		else
		{
			if( strlen(buf)==0)
				memset(pDHCPSPEntry->subnetmask,0,IP_ADDR_LEN);
			else
				if(!inet_aton(buf, (struct in_addr *)&pDHCPSPEntry->subnetmask)) return ERR_9007;

			mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
		}
		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{
		unsigned char vChar;

		if(pool_num == 1)
		{
			if( (buf==NULL) || (strlen(buf)==0) )
			{ 	//DNS Relay
				vChar = 0;
				mib_set( MIB_DHCP_DNS_OPTION, (void *)&vChar);
			}
			else if( setDNSList( buf, 1 ) == 0, 1 )
			{
				vChar = 1;
				mib_set( MIB_DHCP_DNS_OPTION, (void *)&vChar);
			}
			else
				 return ERR_9007;
		}
		else
		{
			if( (buf==NULL) || (strlen(buf)==0) )
			{ 	//automatically attain DNS
				pDHCPSPEntry->dnsservermode=0;
			}
			else if( setSPDNSList( pDHCPSPEntry, buf ) == 0 )
			{
				pDHCPSPEntry->dnsservermode=1;
			}
			else
				 return ERR_9007;

			mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
		}

		apply_add( CWMP_PRI_SL, apply_DNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "DomainName" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(pool_num == 1)
		{
			if( strlen(buf)==0 ) return ERR_9007;

			//mib defines the length of domainname=30
			if( strlen(buf)>=30 ) return ERR_9001;
			mib_set(MIB_ADSL_LAN_DHCP_DOMAIN, (void *)buf);
		}
		else
		{
			if( strlen(buf)==0)
				memset(pDHCPSPEntry->domainname,0,GENERAL_LEN);
			else
			{
				strncpy(pDHCPSPEntry->domainname,buf,GENERAL_LEN-1);
				pDHCPSPEntry->domainname[GENERAL_LEN-1]=0;
			}
			mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
		}

		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "IPRouters" )==0 )
	{
		struct in_addr lan_ip = {0}, addr = {0};

		if( buf==NULL ) return ERR_9007;

		if(pool_num == 1)
		{
			if( strlen(buf)==0 ) return ERR_9007;
			if( inet_aton(buf, &addr)==0 ) return ERR_9007;
			mib_set( MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&addr);
		}
		else
		{
			if( strlen(buf)==0)
				memset(pDHCPSPEntry->iprouter,0,IP_ADDR_LEN);
			else
				if(!inet_aton(buf, (struct in_addr *)&pDHCPSPEntry->iprouter)) return ERR_9007;
			mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
		}

		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "LeaseTime" )==0 )
	{
		int *pInt = data;

		if(pInt==NULL) return ERR_9007;
		if(*pInt<-1) return ERR_9007;

		if(pool_num == 1)
			mib_set(MIB_ADSL_LAN_DHCP_LEASE, (void *)pInt);
		else
		{
			pDHCPSPEntry->leasetime = *pInt;
			mib_chain_update( MIB_DHCPS_SERVING_POOL_TBL, (unsigned char*)pDHCPSPEntry, chainid );
		}

		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}


int objDHCPv4PoolOpt(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int pool_num, usedFor,num,i;
	unsigned int chainid;

	pool_num = getInstNum( name, "DHCPv4.Server.Pool" );
	if(pool_num == 1)
		usedFor = eUsedFor_DHCPServer;
	else
		usedFor = eUsedFor_DHCPServer_ServingPool;

	switch( type )
	{
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		{
			int ret, found=0;
			DHCPS_SERVING_POOL_T *pDHCPSPEntry, DhcpSPEntry;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			// not default pool
			if(pool_num != 1)
			{
				pDHCPSPEntry = &DhcpSPEntry;
				num = mib_chain_total( MIB_DHCPS_SERVING_POOL_TBL );				
				for(i=0; i<num; i++)
				{
					if( !mib_chain_get( MIB_DHCPS_SERVING_POOL_TBL, i, (void*)pDHCPSPEntry ) )
						continue;
					if(pDHCPSPEntry->InstanceNum + 1 == pool_num )
					{
						found = 1;
						break;
					}
				}
				if(found ==0) return ERR_9005;
			}

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPv4PoolOptObject, sizeof(tDHCPv4PoolOptObject), data );
			if( ret >= 0 )
			{
				MIB_CE_DHCP_OPTION_T entry;
				memset( &entry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
				{ //default values for this new entry
					entry.enable = 0;
					entry.usedFor = usedFor;
					entry.dhcpOptInstNum =*(int*)data;
					entry.dhcpConSPInstNum = (pool_num == 1) ? 0 : pDHCPSPEntry->InstanceNum;
				}
				mib_chain_add( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)&entry);
			}
			return ret;
		}
	case eCWMP_tDELOBJ:
		{
			int ret, num, i;
			unsigned int *pUint=data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			pDHCPOptEntry = &DhcpOptEntry;
			num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
			for( i=0;i<num;i++ )
			{
				if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)pDHCPOptEntry ) )
					continue;
				if(pDHCPOptEntry->usedFor == usedFor
					&& pDHCPOptEntry->dhcpConSPInstNum == pool_num - 1
					&& pDHCPOptEntry->dhcpOptInstNum==*pUint)
				{
					mib_chain_delete( MIB_DHCP_SERVER_OPTION_TBL, i );
					break;
				}
			}
			if(i == num) return ERR_9005;
			
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			apply_DHCP(CWMP_RESTART,0,NULL);

			return ret;
		}
	case eCWMP_tUPDATEOBJ:
		{
			int num,i;
			struct CWMP_LINKNODE *old_table;

			num = mib_chain_total( MIB_DHCP_SERVER_OPTION_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_SERVER_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;

				if( (pDHCPOptEntry->usedFor == usedFor) &&
					(pDHCPOptEntry->dhcpConSPInstNum == pool_num - 1) )
				{
					remove_entity = remove_SiblingEntity( &old_table, pDHCPOptEntry->dhcpOptInstNum );
					if( remove_entity!=NULL )
					{
						add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
					}
					else
					{
						if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, pDHCPOptEntry->dhcpOptInstNum )==NULL )
						{
							unsigned int MaxInstNum = pDHCPOptEntry->dhcpOptInstNum;
							add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPv4PoolOptObject, sizeof(tDHCPv4PoolOptObject), &MaxInstNum );
							if(MaxInstNum!=pDHCPOptEntry->dhcpOptInstNum)
							{
								pDHCPOptEntry->dhcpOptInstNum = MaxInstNum;
								mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, i );
							}
						}//else already in next_table
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

int getDHCPv4PoolOptEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int opt_num,pool_num;
	unsigned char usedFor;
	unsigned int chainid;

	opt_num = getInstNum(name, "Option");
	pool_num = getInstNum( name, "DHCPv4.Server.Pool" );

	pDHCPOptEntry = &DhcpOptEntry;

	if(pool_num == 1 && opt_num > 0)
		usedFor = eUsedFor_DHCPServer;
	else if(pool_num != 1 && opt_num > 0)
		usedFor = eUsedFor_DHCPServer_ServingPool;
	else
		return ERR_9005;

	if( getDHCPOptionByOptInstNum(opt_num, pool_num - 1, usedFor, pDHCPOptEntry, &chainid) < 0)
		return ERR_9002;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		if(pDHCPOptEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "Tag" )==0 )
	{
		*data = uintdup(pDHCPOptEntry->tag);
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		struct xsd__hexBinary tmp = {0};

		tmp.__ptr = pDHCPOptEntry->value;
		tmp.__size = pDHCPOptEntry->len;
		*data = (void *)hexBinarydup(tmp);
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

int setDHCPv4PoolOptEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char *buf = data;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int opt_num,pool_num;
	unsigned char usedFor;
	unsigned int chainid;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	opt_num = getInstNum(name, "Option");
	pool_num = getInstNum( name, "DHCPv4.Server.Pool" );
	pDHCPOptEntry = &DhcpOptEntry;

	if(pool_num == 1 && opt_num > 0)
		usedFor = eUsedFor_DHCPServer;
	else if(pool_num != 1 && opt_num > 0)
		usedFor = eUsedFor_DHCPServer_ServingPool;
	else
		return ERR_9005;

	if( getDHCPOptionByOptInstNum(opt_num, pool_num - 1, usedFor, pDHCPOptEntry, &chainid) < 0)
		return ERR_9002;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPOptEntry->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "Tag" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<1 || *i>254) return ERR_9007;
		if(checkDHCPOptionTag(*i)<0)  return ERR_9001;
		pDHCPOptEntry->tag = *i;
		mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(data)
		{
			int i;
			struct xsd__hexBinary *b=data;

			if(b->__size>DHCP_OPT_VAL_LEN) return ERR_9001;
				pDHCPOptEntry->len=b->__size;
			memcpy(pDHCPOptEntry->value, b->__ptr, b->__size);
		}
		mib_chain_update( MIB_DHCP_SERVER_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}


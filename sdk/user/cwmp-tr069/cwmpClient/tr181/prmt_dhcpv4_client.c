#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <netinet/in.h>
#include <linux/rtnetlink.h>

#include <rtk/mib.h>
#include <rtk/utility.h>
#include <cwmp_utility.h>

#include "prmt_dhcpv4_client.h"


// SentOption and ReqOption use the same CWMP_OP.
struct CWMP_OP tDHCPv4ClientOptEntityLeafOP = { getDHCPv4ClientOptEntity, setDHCPv4ClientOptENTITY };
/***** Device.DHCPv4.Client.{i}.SentOption.{i}. ******************************/
struct CWMP_PRMT tDHCPv4ClientSentOptEntityLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"Enable",  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ClientOptEntityLeafOP},
{"Tag",		eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ClientOptEntityLeafOP},
{"Value",   eCWMP_tHEXBIN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ClientOptEntityLeafOP},
};

enum eDHCPv4ClientSentOptEntityLeaf
{
	eDHCPv4ClientSentOptEnable,
	eDHCPv4ClientSentOptTag,
	eDHCPv4ClientSentOptValue
};

struct CWMP_LEAF tDHCPv4ClientSentOptEntityLeaf[] =
{
{ &tDHCPv4ClientSentOptEntityLeafInfo[eDHCPv4ClientSentOptEnable] },
{ &tDHCPv4ClientSentOptEntityLeafInfo[eDHCPv4ClientSentOptTag] },
{ &tDHCPv4ClientSentOptEntityLeafInfo[eDHCPv4ClientSentOptValue] },
{ NULL }
};

/***** Device.DHCPv4.Client.{i}.SentOption. **********************************/
struct CWMP_PRMT tDHCPv4ClientSentOjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};

enum eDHCPv4ClientSentOptOjbect
{
	eDHCPv4ClientSentOpt0
};

struct CWMP_LINKNODE tDHCPv4ClientSentObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tDHCPv4ClientSentOjbectInfo[eDHCPv4ClientSentOpt0],	tDHCPv4ClientSentOptEntityLeaf,	NULL,	NULL,	0},
};

/***** Device.DHCPv4.Client.{i}.ReqOption.{i} ********************************/
struct CWMP_PRMT tDHCPv4ClientReqOptEntityLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"Enable",  eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ClientOptEntityLeafOP},
{"Order",   eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ClientOptEntityLeafOP},
{"Tag",		eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ClientOptEntityLeafOP},
{"Value",   eCWMP_tHEXBIN,	CWMP_READ,	&tDHCPv4ClientOptEntityLeafOP},
};
enum eDHCPv4ClientReqOptEntityLeaf
{
	eDHCPv4ClientReqOptEnable,
	eDHCPv4ClientReqOptOrder,
	eDHCPv4ClientReqOptTag,
	eDHCPv4ClientReqOptValue
};
struct CWMP_LEAF tDHCPv4ClientReqOptEntityLeaf[] =
{
{ &tDHCPv4ClientReqOptEntityLeafInfo[eDHCPv4ClientReqOptEnable] },
{ &tDHCPv4ClientReqOptEntityLeafInfo[eDHCPv4ClientReqOptOrder] },
{ &tDHCPv4ClientReqOptEntityLeafInfo[eDHCPv4ClientReqOptTag] },
{ &tDHCPv4ClientReqOptEntityLeafInfo[eDHCPv4ClientReqOptValue] },
{ NULL }
};

/***** Device.DHCPv4.Client.{i}.ReqOption. ***********************************/
struct CWMP_PRMT tDHCPv4ClientReqOptOjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};

enum eDHCPv4ClientReqOptOjbect
{
	eDHCPv4ClientReqOpt0
};

struct CWMP_LINKNODE tDHCPv4ClientReqOptObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tDHCPv4ClientReqOptOjbectInfo[eDHCPv4ClientReqOpt0],	tDHCPv4ClientReqOptEntityLeaf,	NULL,	NULL,	0},
};

/***** Device.DHCPv4.Client.{i}. *********************************************/
struct CWMP_OP tDHCPv4ClientEntityLeafOP = { getDHCPv4ClientEntity, setDHCPv4ClientEntity };
struct CWMP_PRMT tDHCPv4ClientEntityLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"Enable",                      eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ClientEntityLeafOP},
{"Interface",                   eCWMP_tSTRING,	CWMP_READ/*|CWMP_WRITE*/,	&tDHCPv4ClientEntityLeafOP},
{"Status",                      eCWMP_tSTRING,	CWMP_READ,	&tDHCPv4ClientEntityLeafOP},
{"Renew",                       eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ClientEntityLeafOP},
{"IPAddress",                   eCWMP_tSTRING,	CWMP_READ,	&tDHCPv4ClientEntityLeafOP},
{"SubnetMask",                  eCWMP_tSTRING,	CWMP_READ,	&tDHCPv4ClientEntityLeafOP},
{"IPRouters",                   eCWMP_tSTRING,	CWMP_READ,	&tDHCPv4ClientEntityLeafOP},
{"DNSServers",                  eCWMP_tSTRING,	CWMP_READ,	&tDHCPv4ClientEntityLeafOP},
{"LeaseTimeRemaining",          eCWMP_tINT,		CWMP_READ|CWMP_DENY_ACT,	&tDHCPv4ClientEntityLeafOP},
{"DHCPServer",                  eCWMP_tSTRING,	CWMP_READ,	&tDHCPv4ClientEntityLeafOP},
{"SentOptionNumberOfEntries",   eCWMP_tUINT,	CWMP_READ,	&tDHCPv4ClientEntityLeafOP},
{"ReqOptionNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tDHCPv4ClientEntityLeafOP},
};

enum eDHCPv4ClientEntityLeaf
{
	eDHCPv4ClientEnable,
	eDHCPv4ClientInterface,
	eDHCPv4ClientStatus,
	eDHCPv4ClientRenew,
	eDHCPv4ClientIPAddress,
	eDHCPv4ClientSubnetMask,
	eDHCPv4ClientIPRouters,
	eDHCPv4ClientDNSServers,
	eDHCPv4ClientLeaseTimeRemaining,
	eDHCPv4ClientDHCPServer,
	eDHCPv4ClientSentOptionNumberOfEntries,
	eDHCPv4ClientReqOptionNumberOfEntries
};

struct CWMP_LEAF tDHCPv4ClientEntityLeaf[] =
{
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientEnable] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientInterface] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientStatus] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientRenew] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientIPAddress] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientSubnetMask] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientIPRouters] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientDNSServers] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientLeaseTimeRemaining] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientDHCPServer] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientSentOptionNumberOfEntries] },
{ &tDHCPv4ClientEntityLeafInfo[eDHCPv4ClientReqOptionNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tDHCPv4ClientSentOpt_OP = { NULL, objDHCPv4ClientSentOpt };
struct CWMP_OP tDHCPv4ClientReqOpt_OP = { NULL, objDHCPv4ClientReqOpt };
struct CWMP_PRMT tDHCPv4ClientEntityObjectInfo[] =
{
/*(name,			type,		flag,			)*/
{"SentOption",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ClientSentOpt_OP},
{"ReqOption",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4ClientReqOpt_OP},
};

enum eDHCPv4ClientSentObject
{
	eDHCPv4ClientSentOpt,
	eDHCPv4ClientReqOpt,
};

struct CWMP_NODE tDHCPv4ClientEntityObject[] =
{
/*info,  					leaf,			node)*/
{&tDHCPv4ClientEntityObjectInfo[eDHCPv4ClientSentOpt],	NULL,	NULL},
{&tDHCPv4ClientEntityObjectInfo[eDHCPv4ClientReqOpt],	NULL,	NULL},
{NULL,						NULL,			NULL}
};

/***** Device.DHCPv4.Client. *************************************************/
struct CWMP_OP tDHCPv4Client_OP = {NULL, objDHCPv4Client};
struct CWMP_PRMT tDHCPv4ClientOjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ/*|CWMP_WRITE*/|CWMP_LNKLIST,	NULL}
};

enum eDHCPv4ClientOjbect
{
	eDHCPv4Client0
};

struct CWMP_LINKNODE tDHCPv4ClientObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tDHCPv4ClientOjbectInfo[eDHCPv4Client0],	tDHCPv4ClientEntityLeaf,	tDHCPv4ClientEntityObject,	NULL,	0},
};

/***** Utility Functions *****************************************************/
struct dhcpv4_info
{
	char ip[16];
	char serverip[16];
	char router[256];
	char dns[256];
	int lease_remain;
};

static int get_dhcpv4_info(const char *ifname, struct dhcpv4_info *info)
{
	char filename[256] = {0};
	FILE *f = NULL;
	char line[512];

	sprintf(filename, "/tmp/udhcpc_info.%s", ifname);
	f = fopen(filename, "r");
	if(f == NULL)
		return -1;

	while( fgets(line, 512, f) != NULL)
	{
		if(strncmp(line, "ip=", 3) == 0)
			sscanf(line, "ip=%s\n", info->ip);
		else if(strncmp(line, "siaddr", 3) == 0)
			sscanf(line, "siaddr=%s\n", info->serverip);
		else if(strncmp(line, "router=", 7) == 0)
			sscanf(line, "router=%s\n", info->router);
		else if(strncmp(line, "dns=", 4) == 0)
		{
			sscanf(line, "dns=%s\n", info->dns);
			info->dns[strlen(info->dns) - 1] = '\0';	//remove last ','
		}
		else if(strncmp(line, "expire=", 6) == 0)
		{
			time_t expire;
			sscanf(line, "expire=%ld\n", &expire);
			info->lease_remain = (int)(expire - time(0));
		}
	}
	fclose(f);
	return 0;
}

/***** Operations ************************************************************/
int objDHCPv4Client(char *name, struct CWMP_LEAF *e, int type, void *data)
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

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tDHCPv4ClientObject, sizeof(tDHCPv4ClientObject), &MaxInstNum );
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

int getDHCPv4ClientEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int client_num;
	MIB_CE_ATM_VC_T vc_entity;
	char ifname[IFNAMSIZ];
	struct dhcpv4_info info = {0};
	int id;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	client_num = getInstNum(name, "DHCPv4.Client");
	if(client_num < 1) return ERR_9005;
	
	if(get_wan_info_tr181(client_num, &vc_entity, &id, ifname) == 0)
		return ERR_9005;

	if (vc_entity.ipDhcp == DHCP_CLIENT)
		get_dhcpv4_info(ifname, &info);

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		if (vc_entity.ipDhcp == DHCP_CLIENT)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		char interface[64];
		sprintf(interface, "Device.IP.Interface.%d", client_num);
		*data = strdup(interface);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		if (vc_entity.ipDhcp == DHCP_CLIENT)
			*data = strdup("Enabled");
		else
			*data = strdup("Disabled");
	}
	else if( strcmp( lastname, "Renew" )==0 )
	{
		*data = booldup(0); 
	}
	else if( strcmp( lastname, "IPAddress" )==0 )
	{
		*data = strdup(info.ip);
	}
	else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		struct in_addr netmask={0};
		
		if (vc_entity.ipDhcp == DHCP_CLIENT)
		{
			if(!getInAddr(ifname, SUBNET_MASK, (void *)&netmask))
				*data = strdup("");
			else
				*data = strdup(inet_ntoa(netmask));
		}
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "IPRouters" )==0 )
	{
		*data = strdup(info.router);
	}
	else if( strcmp( lastname, "DNSServers" )==0 )
	{
		*data = strdup(info.dns);
	}
	else if( strcmp( lastname, "LeaseTimeRemaining" )==0 )
	{
		*data = intdup(info.lease_remain);
	}
	else if( strcmp( lastname, "DHCPServer" )==0 )
	{
		*data = strdup(info.serverip);
	}
	else if( strcmp( lastname, "SentOptionNumberOfEntries" )==0 )
	{
		*data = uintdup(findDHCPOptionNum(eUsedFor_DHCPClient_Sent, vc_entity.ifIndex));
	}
	else if( strcmp( lastname, "ReqOptionNumberOfEntries" )==0 )
	{
		*data = uintdup(findDHCPOptionNum(eUsedFor_DHCPClient_Req, vc_entity.ifIndex));
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setDHCPv4ClientEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char *buf = data;
	unsigned int client_num;
	MIB_CE_ATM_VC_T vc_entity = {0};
	char ifname[IFNAMSIZ];
	int id;

	if( (name==NULL) || (type==0) || (data==NULL) || (entity==NULL))
		return -1;

	client_num = getInstNum(name, "DHCPv4.Client");
	if(client_num < 1) return ERR_9005;

	if(get_wan_info_tr181(client_num, &vc_entity, &id, ifname) == 0)
		return ERR_9005;

	gWanBitMap |= (1U << id);

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i=data;
		unsigned char vChar = *i;

		vc_entity.ipDhcp = (vChar == 1) ? 1 : 0;

		mib_chain_update( MIB_ATM_VC_TBL, (void *)&vc_entity, id );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Renew" )==0 )
	{
		int pid;

		pid = read_pid((char*)DHCPC_PID);
		kill(pid, SIGUSR1);
		return 0;
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}


int objDHCPv4ClientSentOpt(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned char usedFor = eUsedFor_DHCPClient_Sent;
	MIB_CE_ATM_VC_T pvcentry;
	int client_num;
	char ifname[IFNAMSIZ];
	int id;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);
	
	client_num = getInstNum(name, "DHCPv4.Client");
	if(client_num < 1) return ERR_9005;
	
	if(get_wan_info_tr181(client_num, &pvcentry, &id, ifname) == 0)
		return ERR_9005;

	switch( type )
	{
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		{
			int ret;//found=0;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPv4ClientSentObject, sizeof(tDHCPv4ClientSentObject), data );
			if( ret >= 0 )
			{
				MIB_CE_DHCP_OPTION_T entry;
				memset( &entry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
				{ //default values for this new entry
					entry.enable = 0;
					entry.usedFor = usedFor;
					entry.dhcpOptInstNum = *(int *)data;
					entry.dhcpConSPInstNum = 0;
					entry.ifIndex = pvcentry.ifIndex;
					entry.order = findMaxDHCPReqOptionOrder()+1;
				}
				mib_chain_add( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)&entry);
			}
			return ret;
		}
	case eCWMP_tDELOBJ:
		{
			int ret, num, i;
			int found = 0;
			unsigned int *pUint=data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			for( i=num-1; i>=0;i-- )
			{
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ) )
					continue;
				if(pDHCPOptEntry->usedFor == usedFor && pDHCPOptEntry->ifIndex == pvcentry.ifIndex
					&& pDHCPOptEntry->dhcpOptInstNum==*pUint)
				{
					found =1;
					mib_chain_delete( MIB_DHCP_CLIENT_OPTION_TBL, i );
					compact_reqoption_order();
					break;
				}
			}

			if(found==0) return ERR_9005;
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )	ret=1;
			return ret;
		}
	case eCWMP_tUPDATEOBJ:
		{
			int num,i;
			struct CWMP_LINKNODE *old_table;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;

				if( (pDHCPOptEntry->usedFor == usedFor) && (pDHCPOptEntry->ifIndex == pvcentry.ifIndex) && (pDHCPOptEntry->dhcpOptInstNum!=0))
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
							add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPv4ClientSentObject, sizeof(tDHCPv4ClientSentObject), &MaxInstNum );
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

int objDHCPv4ClientReqOpt(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int num,i;
	unsigned char usedFor = eUsedFor_DHCPClient_Req;
	MIB_CE_ATM_VC_T pvcentry;
	int client_num;
	char ifname[IFNAMSIZ];
	int id;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);
	
	client_num = getInstNum(name, "DHCPv4.Client");
	if(client_num < 1) return ERR_9005;
	
	if(get_wan_info_tr181(client_num, &pvcentry, &id, ifname) == 0)
		return ERR_9005;

	switch( type )
	{
	case eCWMP_tINITOBJ:
			return 0;
	case eCWMP_tADDOBJ:
		{
			int ret;//found=0;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPv4ClientReqOptObject, sizeof(tDHCPv4ClientReqOptObject), data );
			if( ret >= 0 )
			{
				MIB_CE_DHCP_OPTION_T entry;
				memset( &entry, 0, sizeof( MIB_CE_DHCP_OPTION_T ) );
				{ //default values for this new entry
					entry.enable = 0;
					entry.usedFor = usedFor;
					entry.dhcpOptInstNum = *(int *)data;
					entry.dhcpConSPInstNum = 0;
					entry.ifIndex = pvcentry.ifIndex;
					entry.order = findMaxDHCPReqOptionOrder()+1;
				}
				mib_chain_add( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)&entry);
			}
			return ret;
		}
	case eCWMP_tDELOBJ:
		{
			int ret, num, i;
			int found = 0;
			unsigned int *pUint=data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			for( i=num-1; i>=0;i-- )
			{
				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ) )
					continue;
				if(pDHCPOptEntry->usedFor == usedFor && pDHCPOptEntry->ifIndex == pvcentry.ifIndex
					&& pDHCPOptEntry->dhcpOptInstNum==*pUint)
				{
					found =1;
					mib_chain_delete( MIB_DHCP_CLIENT_OPTION_TBL, i );
					compact_reqoption_order();
					break;
				}
			}

			if(found==0) return ERR_9005;
			ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
			if( ret==0 )	ret=1;
			return ret;
		}
	case eCWMP_tUPDATEOBJ:
		{
			int num,i;
			struct CWMP_LINKNODE *old_table;

			num = mib_chain_total( MIB_DHCP_CLIENT_OPTION_TBL );
			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;
			for( i=0; i<num;i++ )
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				pDHCPOptEntry = &DhcpOptEntry;
				if( !mib_chain_get( MIB_DHCP_CLIENT_OPTION_TBL, i, (void*)pDHCPOptEntry ))
					continue;

				if( (pDHCPOptEntry->usedFor == usedFor) && (pDHCPOptEntry->ifIndex == pvcentry.ifIndex) && (pDHCPOptEntry->dhcpOptInstNum!=0))
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
							add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tDHCPv4ClientReqOptObject, sizeof(tDHCPv4ClientReqOptObject), &MaxInstNum );
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


int getDHCPv4ClientOptEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry;
	unsigned int sentDhcpOptNum,reqDhcpOptNum;
	unsigned char usedFor;
	unsigned int client_num;
	unsigned int chainid;
	MIB_CE_ATM_VC_T vc_entity;
	char ifname[IFNAMSIZ];
	int id;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	client_num = getInstNum(name, "DHCPv4.Client");
	if(client_num < 1) return ERR_9005;
	
	if(get_wan_info_tr181(client_num, &vc_entity, &id, ifname) == 0)
		return ERR_9005;

	sentDhcpOptNum = getInstNum(name, "SentOption");
	reqDhcpOptNum = getInstNum(name, "ReqOption");

	if( sentDhcpOptNum==0 && reqDhcpOptNum ==0 ) return ERR_9005;

	pDHCPOptEntry = &DhcpOptEntry;

	if(sentDhcpOptNum != 0)
	{
		usedFor = eUsedFor_DHCPClient_Sent;
		if( getDHCPClientOptionByOptInstNum(sentDhcpOptNum, vc_entity.ifIndex,  usedFor, pDHCPOptEntry, &chainid) < 0)
			return ERR_9002;
	}
	else if(reqDhcpOptNum !=0)
	{
		usedFor = eUsedFor_DHCPClient_Req;
		if( getDHCPClientOptionByOptInstNum(reqDhcpOptNum, vc_entity.ifIndex, usedFor,pDHCPOptEntry, &chainid) < 0 )
			return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		if(pDHCPOptEntry->enable)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "Order")==0)
	{
		if(usedFor != eUsedFor_DHCPClient_Req)
			return ERR_9005;
		*data = uintdup(pDHCPOptEntry->order);
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
		*data = (void *)hexBinarydup( tmp );
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

int setDHCPv4ClientOptENTITY(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char *buf = data;
	MIB_CE_DHCP_OPTION_T *pDHCPOptEntry, DhcpOptEntry = {0};
	unsigned int sentDhcpOptNum,reqDhcpOptNum;
	unsigned char usedFor;
	unsigned int client_num;
	unsigned int chainid;
	MIB_CE_ATM_VC_T vc_entity = {0};
	char ifname[IFNAMSIZ];
	int id;

	if( (name==NULL) || (type==0) || (data==NULL) || (entity==NULL))
		return -1;

	client_num = getInstNum(name, "DHCPv4.Client");
	if(client_num < 1) return ERR_9005;

	if(get_wan_info_tr181(client_num, &vc_entity, &id, ifname) == 0)
		return ERR_9005;

	sentDhcpOptNum = getInstNum(name, "SentOption");
	reqDhcpOptNum = getInstNum(name, "ReqOption");

	if (sentDhcpOptNum==0 && reqDhcpOptNum ==0 ) return ERR_9005;

	if(sentDhcpOptNum != 0)
	{
		usedFor = eUsedFor_DHCPClient_Sent;
		if( getDHCPClientOptionByOptInstNum(sentDhcpOptNum, vc_entity.ifIndex, usedFor, pDHCPOptEntry, &chainid) < 0)
		return ERR_9002;
	}
	else if(reqDhcpOptNum !=0)
	{
		usedFor = eUsedFor_DHCPClient_Req;
		if( getDHCPClientOptionByOptInstNum(reqDhcpOptNum, vc_entity.ifIndex, usedFor,pDHCPOptEntry, &chainid) < 0 )
		return ERR_9002;
	}

	if(usedFor==eUsedFor_DHCPClient_Sent){
		if(checkDHCPClientOptionTag(pDHCPOptEntry->tag)<0)
			return ERR_9001;
	}

	pDHCPOptEntry = &DhcpOptEntry;

	gWanBitMap |= (1U << id);

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		pDHCPOptEntry->enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Order" )==0 )
	{
		unsigned int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<1 ) return ERR_9007;
		if(pDHCPOptEntry->usedFor != eUsedFor_DHCPClient_Req) return ERR_9005;
		if(checkandmodify_reqoption_order(*i,chainid)<0) return ERR_9007;
		pDHCPOptEntry->order = *i;
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		compact_reqoption_order();
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Tag" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i<1 || *i>254) return ERR_9007;
		if(usedFor==eUsedFor_DHCPClient_Sent){
			if(checkDHCPClientOptionTag(*i)<0)
				return ERR_9001;
		}
		pDHCPOptEntry->tag = *i;
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Value" )==0 )
	{
		if( buf==NULL ) return ERR_9007;

		if(pDHCPOptEntry->usedFor == eUsedFor_DHCPClient_Req)
			return ERR_9001;

		if(data)
		{
			int i;
			struct xsd__hexBinary *b=data;
			if(b->__size > DHCP_OPT_VAL_LEN) return ERR_9007;
			pDHCPOptEntry->len=b->__size;
			memcpy(pDHCPOptEntry->value, b->__ptr, b->__size);
		}
		mib_chain_update( MIB_DHCP_CLIENT_OPTION_TBL, (unsigned char*)pDHCPOptEntry, chainid );
		return CWMP_NEED_RESTART_WAN;
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}


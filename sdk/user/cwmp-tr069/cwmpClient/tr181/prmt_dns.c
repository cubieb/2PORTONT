#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>

#include <rtk/mib.h>
#include <rtk/utility.h>
#include <libcwmp.h>
#include <parameter_api.h>

#include "../prmt_apply.h"
#include "prmt_dns.h"

/****** Device.DNS.Client.Server.{i}. ****************************************/
struct CWMP_OP tDNSCServerEntityLeafOP = { getDNSCServerEntity, setDNSCServerEntity };
struct CWMP_PRMT tDNSCServerEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tDNSCServerEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,		&tDNSCServerEntityLeafOP},
{"DNSServer",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDNSCServerEntityLeafOP},
{"Interface",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDNSCServerEntityLeafOP},
{"Type",		eCWMP_tSTRING,	CWMP_READ,	&tDNSCServerEntityLeafOP},
};
enum eDNSCServerEntityLeaf
{
	eServerEnable,
	eServerStatus,
	eServerDNSServer,
	eServerInterface,
	eServerType,
};
struct CWMP_LEAF tDNSCServerEntityLeaf[] =
{
{ &tDNSCServerEntityLeafInfo[eServerEnable] },
{ &tDNSCServerEntityLeafInfo[eServerStatus] },
{ &tDNSCServerEntityLeafInfo[eServerDNSServer] },
{ &tDNSCServerEntityLeafInfo[eServerInterface] },
{ &tDNSCServerEntityLeafInfo[eServerType] },
{ NULL	}
};

/****** Device.DNS.Client.Server *********************************************/
struct CWMP_PRMT tDNSCServerOjbectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",	eCWMP_tOBJECT, CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,		NULL},
};
enum eDNSCServerObject
{
	eDNSServer0,
};

struct CWMP_LINKNODE tDNSCServerObject[] =
{
/*info,  				leaf,			next)*/
{&tDNSCServerOjbectInfo[eDNSServer0],		tDNSCServerEntityLeaf,	NULL},
{NULL,					NULL,			NULL}
};

/****** Device.DNS.Client ****************************************************/
struct CWMP_OP tDNSCServerObjectOP = { NULL, objDNSCServer};
struct CWMP_OP tDNSClientLeafOP = { getDNSClient, NULL};

struct CWMP_PRMT tDNSClientLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tDNSClientLeafOP},
{"Status",	eCWMP_tSTRING, CWMP_READ,		&tDNSClientLeafOP},
{"ServerNumberOfEntries",	eCWMP_tUINT, CWMP_READ,		&tDNSClientLeafOP},
};
enum eDNSClientLeaf
{
	eEnable,
	eStatus,
	eServerNumberOfEntries
};

struct CWMP_LEAF tDNSClientLeaf[] =
{
{ &tDNSClientLeafInfo[eEnable]  },
{ &tDNSClientLeafInfo[eStatus]	},
{ &tDNSClientLeafInfo[eServerNumberOfEntries]	},
{ NULL	}
};

/* DNSCServer is DNS server information for DNS Client */
struct CWMP_PRMT tDNSClientObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Server",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tDNSCServerObjectOP},
};
enum eDNSClientObject
{
	eServer,
};

struct CWMP_NODE tDNSClientObject[] =
{
/*info,  				leaf,			next)*/
{&tDNSClientObjectInfo[eServer],		NULL,	NULL},
{NULL,					NULL,			NULL}
};


/****** Device.DNS ***********************************************************/
struct CWMP_PRMT tDNSObjectInfo[] =
{
/*(name,		type,		flag,		op)*/
{"Client",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eDNSObject
{
	eClient,
};

struct CWMP_NODE tDNSObject[] =
{
/*info,  				leaf,			next)*/
{&tDNSObjectInfo[eClient],		tDNSClientLeaf,	tDNSClientObject},
{NULL,					NULL,			NULL}
};


/****** Utility Functions ****************************************************/
DNS_TYPE_T getDNSMode()
{
	unsigned char dnsMode = 0;

	mib_get( MIB_ADSL_WAN_DNS_MODE, &dnsMode);
	return dnsMode;
}


int getDNSServerCnt()
{
	int ret= 0;
	static const char fresolv[] = "/var/resolv.conf";
	FILE *fp;
	char line[128], addr[64];
	
#ifdef CONFIG_IPV6
	if(getDNSMode() == DNS_MANUAL)
		return 6;	//3 IPv4 + 3 IPv6
#else
	if(getDNSMode() == DNS_MANUAL)
		return 3;
#endif

	//DNS_AUTO
	if ((fp = fopen(fresolv, "r")) == NULL)
	{
		fprintf(stderr, "Open %s failed\n", fresolv);
		return 0;
	}

	while (fgets(line, sizeof(line), fp) != NULL) {
		if (sscanf(line, "nameserver %s", addr) != 1)
			continue;
		ret++;
	}

	fclose(fp);

	return ret;
}

// id must be a valid instance
int isDNSServerEnabled(int id)
{
	int base_mib_id = MIB_ADSL_WAN_DNS1;
	char addr[IP6_ADDR_LEN] = {0};	//IPv4 also use this variable
	char zero[IP6_ADDR_LEN] = {0};

	if(id < 1 || id > getDNSServerCnt())
		return 0;

	if(getDNSMode() == DNS_AUTO)
		return 1;

#ifdef CONFIG_IPV6
	//DNS_MANUAL
	if(id >= 4)
	{
		base_mib_id = MIB_ADSL_WAN_DNSV61;
		id -= 4;
	}
	else
#endif
		id--;

	mib_get(base_mib_id + id, addr);
	if(memcmp(addr, zero, sizeof(addr)) == 0)
		return 0;

	return 1;
}

int getDNSServer(int id, char *buf, int *isIPv6)
{
	static const char fresolv[] = "/var/resolv.conf";
	int cnt = 0;
	char addr[64];

	if(id < 1)
		return -1;

	if(getDNSMode() == DNS_AUTO)
	{
		FILE *fp;
		char line[128];
		struct addrinfo hint = {0}, *server;

		if ((fp = fopen(fresolv, "r")) == NULL)
		{
			fprintf(stderr, "Open %s failed\n", fresolv);
			return -1;
		}

		while (fgets(line, sizeof(line), fp) != NULL) {
			if (sscanf(line, "nameserver %s", addr) != 1)
				continue;
			cnt++;

			if(cnt == id)
			{
				strcpy(buf, addr);
				break;
			}
		}
		fclose(fp);
		
#ifdef CONFIG_IPV6
		hint.ai_family = PF_UNSPEC;
	    hint.ai_flags = AI_NUMERICHOST;

		if (getaddrinfo(addr, NULL, &hint, &server))
	    {
			fprintf(stderr, "<%s:%d> Get Address type error!\n", __FILE__, __LINE__);
			return -1;
	    }
		if(server->ai_family == AF_INET6)
			*isIPv6 = 1;
#endif
	}
	else	//DNS_MANUAL
	{
		int base_mib_id = MIB_ADSL_WAN_DNS1;

#ifdef CONFIG_IPV6
		if(id >= 4)
		{
			base_mib_id = MIB_ADSL_WAN_DNSV61;
			id -= 4;
			*isIPv6 = 1;
		}
		else
#endif
			id--;

		mib_get(base_mib_id + id, addr);

#ifdef CONFIG_IPV6
		if(*isIPv6)
			inet_ntop(AF_INET6, (struct in6_addr *)addr, buf, INET6_ADDRSTRLEN);
		else
#endif
			inet_ntop(AF_INET, (struct in_addr *)addr, buf, INET_ADDRSTRLEN);
	}
	
	return 0;
}

int setDNSServer(int id, char *server)
{
	DNS_TYPE_T dnsMode = getDNSMode();
	struct addrinfo hint = {0};
	struct addrinfo *tmpaddr = NULL;
	
	hint.ai_family = PF_UNSPEC;
    hint.ai_flags = AI_NUMERICHOST;

	CWMPDBG(1, (stderr, "<%s/%d> Setting DNS Server: %s\n", __FUNCTION__, __LINE__, server));

	if(dnsMode == DNS_AUTO)
		return ERR_9001;

	// DNS_MANUAL
	if(id < 1 || id > getDNSServerCnt() || server == NULL)
		return ERR_9005;

	if (getaddrinfo(server, NULL, &hint, &tmpaddr))
		return ERR_9007;

	if(tmpaddr->ai_family == AF_INET && id <= 3)
	{
		int mib_id = MIB_ADSL_WAN_DNS1 + id - 1;
		struct sockaddr_in *new_addr = NULL;

		new_addr = (struct sockaddr_in *)tmpaddr->ai_addr;
		mib_set(mib_id, &new_addr->sin_addr);
	}
	else if ( tmpaddr->ai_family == AF_INET6 && id >= 4 )
	{
		int mib_id = MIB_ADSL_WAN_DNSV61 + id - 4;
		struct sockaddr_in6 *new_addr = NULL;

		new_addr = (struct sockaddr_in6 *)tmpaddr->ai_addr;
		mib_set(mib_id, &new_addr->sin6_addr);
	}
	else
	{
		freeaddrinfo(tmpaddr);
		return ERR_9001;
	}

	freeaddrinfo(tmpaddr);

	apply_add( CWMP_PRI_L, apply_DNS, CWMP_RESTART, 0, NULL, 0 );
	return 0;
}

/****** Operations ***********************************************************/
int getDNSClient(char *name, struct CWMP_LEAF *entity, int *type, void **data)
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
	else if( strcmp( lastname, "ServerNumberOfEntries" )==0 )
	{
		*data = uintdup(getDNSServerCnt());
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objDNSCServer(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	DNS_TYPE_T dnsMode = getDNSMode();

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);
	
	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
		unsigned int num=0,MaxInstNum=0,i;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		MaxInstNum = getDNSServerCnt();
			
		if( create_Object( c, tDNSCServerObject, sizeof(tDNSCServerObject), MaxInstNum, 1 ) < 0 )
			return -1;

		add_objectNum(name, MaxInstNum);
		return 0;
		}
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:	
	     {
	     	int num=0,i;
	     	struct CWMP_LINKNODE *old_table;

			num = getDNSServerCnt();
	     	
	     	old_table = (struct CWMP_LINKNODE*)entity->next;
	     	entity->next = NULL;

			for( i=1 ; i <= num ; i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;

				remove_entity = remove_SiblingEntity( &old_table, i );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{		
					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  (struct CWMP_LINKNODE *)tDNSCServerObject, sizeof(tDNSCServerObject), &i );
				}	
	     	}
	     	
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );	     	

	     	return 0;
	     }
	}
	
	return -1;
}

int getDNSCServerEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int num; 

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	num = getInstNum(name, "Server");
	if(num < 1 || num > getDNSServerCnt())
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(isDNSServerEnabled(num));
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		if(isDNSServerEnabled(num))
			*data = strdup("Enabled");
		else
			*data = strdup("Disabled");
	}
	else if( strcmp( lastname, "DNSServer" )==0 )
	{
		char addr[128] = {0};
		int isIPv6 = 0;

		if(getDNSServer(num, addr, &isIPv6) == 0)
			*data = strdup(addr);
		else
			*data = strdup("0.0.0.0");
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		//Only support routing policy
		*data = strdup("");
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		char addr[128] = {0};
		int isIPv6 = 0;

		if(getDNSMode() == DNS_AUTO)
		{
#ifdef CONFIG_IPV6
			if(getDNSServer(num, addr, &isIPv6) == 0)
			{
				if(isIPv6)
					*data = strdup("DHCPv6");
				else
					*data = strdup("DHCPv4");
			}
			else	//error
				*data = strdup("Static");
#else
			*data = strdup("DHCPv4");
#endif
		}
		else //DNS_MANUAL
			*data = strdup("Static");
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setDNSCServerEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	int num;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	num = getInstNum(name, "Server");
	if(num < 1 || num > getDNSServerCnt())
		return ERR_9005;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		
		if( *i != isDNSServerEnabled(num)) return ERR_9001;
	}
	else if( strcmp( lastname, "DNSServer" )==0 )
	{
		return setDNSServer(num, (char *)data);
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		return ERR_9001;
	}
	else
	{
		return 0;
	}

	return 0;
}


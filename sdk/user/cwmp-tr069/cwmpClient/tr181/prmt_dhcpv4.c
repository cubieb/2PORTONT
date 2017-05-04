#include <rtk/mib.h>

#include "prmt_dhcpv4.h"
#include "prmt_dhcpv4_client.h"
#include "prmt_dhcpv4_server.h"
#include "prmt_dhcpv4_relay.h"

/***** Device.DHCPv4. ********************************************************/

struct CWMP_OP tDHCPv4LeafOP = { getDHCPv4, NULL };
struct CWMP_PRMT tDHCPv4LeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ClientNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tDHCPv4LeafOP},
};

enum eDHCPv4Leaf
{
	eDHCPv4ClientNumberOfEntries
};

struct CWMP_LEAF tDHCPv4Leaf[] =
{
{ &tDHCPv4LeafInfo[eDHCPv4ClientNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tDHCPv4ObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Client",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tDHCPv4Client_OP},
{"Server",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Relay",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eDHCPv4Object
{
	eDHCPv4Client,
	eDHCPv4Server,
	eDHCPv4Relay,
};

struct CWMP_NODE tDHCPv4Object[] =
{
/*info,  					leaf,			node)*/
{&tDHCPv4ObjectInfo[eDHCPv4Client],	NULL,	NULL},
{&tDHCPv4ObjectInfo[eDHCPv4Server],	tDHCPv4ServerLeaf,	tDHCPv4ServerObject},
{&tDHCPv4ObjectInfo[eDHCPv4Relay],	tDHCPv4RelayLeaf,	tDHCPv4RelayObject},
{NULL,						NULL,			NULL}
};

/***** Operations ************************************************************/
int getDHCPv4(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "ClientNumberOfEntries" )==0 )
	{
		*data = uintdup(mib_chain_total( MIB_ATM_VC_TBL ));
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}


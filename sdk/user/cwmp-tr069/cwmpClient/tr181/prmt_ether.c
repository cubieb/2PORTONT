#include <stdio.h>
#include <stdlib.h>
#include <rtk/mib.h>
#include <parameter_api.h>

#include "../prmt_landevice_eth.h"
#include "prmt_ether.h"
#include "prmt_ether_link.h"
#include "prmt_ether_if.h"

/****** Device.Ethernet ******************************************************/
struct CWMP_OP tEtherLeafOP = { getEther, NULL};
struct CWMP_OP tEtherIfObjectOP = { NULL, objEtherIf};
struct CWMP_OP tEtherLinkObjectOP = { NULL, objEtherLink};

struct CWMP_PRMT tEtherLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"InterfaceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tEtherLeafOP},
{"LinkNumberOfEntries",			eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tEtherLeafOP},
};
enum eEtherLeaf
{
	eInterfaceNumberOfEntries,
	eLinkNumberOfEntries,
};

struct CWMP_LEAF tEtherLeaf[] =
{
{ &tEtherLeafInfo[eInterfaceNumberOfEntries]  },
{ &tEtherLeafInfo[eLinkNumberOfEntries]  },
{ NULL	}
};


struct CWMP_PRMT tEtherObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Interface",	eCWMP_tOBJECT,	CWMP_READ,	&tEtherIfObjectOP},
{"Link",		eCWMP_tOBJECT,	CWMP_READ,	&tEtherLinkObjectOP},
};

enum eEtherObject
{
	eEtherInterface,
	eEtherLink,
};

struct CWMP_NODE tEtherObject[] =
{
/*info,  				leaf,			next)*/
{&tEtherObjectInfo[eEtherInterface],	NULL, NULL},
{&tEtherObjectInfo[eEtherLink],			NULL, NULL},
{NULL,					NULL,			NULL}
};

/***** Operations ************************************************************/
int getEther(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "InterfaceNumberOfEntries" )==0 )
	{
		*data = uintdup(get_eth_if_cnt());
	}
	else if( strcmp( lastname, "LinkNumberOfEntries" )==0 )
	{
		*data = uintdup(get_eth_link_cnt());
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}



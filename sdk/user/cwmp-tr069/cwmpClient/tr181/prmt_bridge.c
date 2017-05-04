#include <stdio.h>
#include <stdlib.h>

#include <rtk/mib.h>
#include <parameter_api.h>

#include "prmt_bridge.h"
#include "prmt_bridge_port.h"

/****** Device.Bridging.Bridge.{i}. ***********************************************/
struct CWMP_OP tBridgingBridgeEntityLeafOP = { getBridgingBridgeEntity, setBridgingBridgeEntity};
struct CWMP_OP tBRPortObjectOP = { NULL, objBRPort};

struct CWMP_PRMT tBridgingBridgeEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tBridgingBridgeEntityLeafOP},
{"Status",	eCWMP_tSTRING,	CWMP_READ,	&tBridgingBridgeEntityLeafOP},
{"PortNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tBridgingBridgeEntityLeafOP},
};

enum eBridgingBridgeEntityLeaf
{
	eBREnable,
	eBRStatus,
	eBRPortNumberOfEntries,
};

struct CWMP_LEAF tBridgingBridgeEntityLeaf[] =
{
{ &tBridgingBridgeEntityLeafInfo[eBREnable]  },
{ &tBridgingBridgeEntityLeafInfo[eBRStatus]  },
{ &tBridgingBridgeEntityLeafInfo[eBRPortNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tBridgingBridgeEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Port",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tBRPortObjectOP},
};

enum eBridgingBridgeEntityObject
{
	ePort,
};

struct CWMP_NODE tBridgingBridgeEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tBridgingBridgeEntityObjectInfo[ePort],			NULL, NULL},
{NULL,					NULL,			NULL}
};

/****** Device.Bridging.Bridge ***********************************************/
struct CWMP_PRMT tBridgingBridgeObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
};

enum eBridgingBridgeObject
{
	eBridge1,
};

struct CWMP_NODE tBridgingBridgeObject[] =
{
/*info,  				leaf,			next)*/
{&tBridgingBridgeObjectInfo[eBridge1],			tBridgingBridgeEntityLeaf, tBridgingBridgeEntityObject},
{NULL,					NULL,			NULL}
};

/****** Device.Bridging ******************************************************/
struct CWMP_OP tBridgingLeafOP = { getBridging, NULL};

struct CWMP_PRMT tBridgingLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"MaxBridgeEntries",		eCWMP_tUINT,	CWMP_READ,	&tBridgingLeafOP},
{"MaxDBridgeEntries",		eCWMP_tUINT,	CWMP_READ,	&tBridgingLeafOP},
{"BridgeNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tBridgingLeafOP},
};

enum eBridgingLeaf
{
	eMaxBridgeEntries,
	eMaxDBridgeEntries,
	eBridgeNumberOfEntries,
};

struct CWMP_LEAF tBridgingLeaf[] =
{
{ &tBridgingLeafInfo[eMaxBridgeEntries]  },
{ &tBridgingLeafInfo[eMaxDBridgeEntries]  },
{ &tBridgingLeafInfo[eBridgeNumberOfEntries]  },
{ NULL	}
};


struct CWMP_PRMT tBridgingObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Bridge",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
};

enum eBridgingObject
{
	eBridge,
};

struct CWMP_NODE tBridgingObject[] =
{
/*info,  				leaf,			next)*/
{&tBridgingObjectInfo[eBridge],			NULL, tBridgingBridgeObject},
{NULL,					NULL,			NULL}
};

/***** Operations ************************************************************/
int getBridging(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "MaxBridgeEntries" )==0 )
	{
		*data = uintdup(1);
	}
	else if( strcmp( lastname, "MaxDBridgeEntries" )==0 )
	{
		*data = uintdup(1);
	}
	else if( strcmp( lastname, "BridgeNumberOfEntries" )==0 )
	{
		*data = uintdup(1);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int getBridgingBridgeEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
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
	else if( strcmp( lastname, "PortNumberOfEntries" )==0 )
	{
		*data = uintdup(get_br_port_cnt());
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setBridgingBridgeEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		return ERR_9001;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


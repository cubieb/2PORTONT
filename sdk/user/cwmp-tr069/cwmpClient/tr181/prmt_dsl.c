#include <stdio.h>
#include <stdlib.h>
#include <rtk/mib.h>
#include <parameter_api.h>

#include "prmt_dsl.h"
#include "prmt_dsl_line.h"
#include "prmt_dsl_channel.h"
#include "prmt_dsl_diag.h"

/****** Device.DSL ***********************************************************/
struct CWMP_OP tDSLLeafOP = { getDSL, NULL};

struct CWMP_PRMT tDSLLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"LineNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tDSLLeafOP},
{"ChannelNumberOfEntries",	eCWMP_tUINT, CWMP_READ,		&tDSLLeafOP},
};
enum eDSLLeaf
{
	eLineNumberOfEntries,
	eChannelNumberOfEntries,
};

struct CWMP_LEAF tDSLLeaf[] =
{
{ &tDSLLeafInfo[eLineNumberOfEntries]  },
{ &tDSLLeafInfo[eChannelNumberOfEntries]	},
{ NULL	}
};


struct CWMP_PRMT tDSLObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Line",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Channel",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Diagnostics",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};
enum eDSLObject
{
	eDSLLINE,
	eDSLCHANNEL,
	eDSLDiagnostics,
};

struct CWMP_NODE tDSLObject[] =
{
/*info,  				leaf,			next)*/
{&tDSLObjectInfo[eDSLLINE],			NULL,	tDSLLineObject},
{&tDSLObjectInfo[eDSLCHANNEL], 		NULL,	tDSLChannelObject},
{&tDSLObjectInfo[eDSLDiagnostics], 	NULL,	tDSLDiagObject},
{NULL,					NULL,			NULL}
};

/***** Operations ************************************************************/
int getDSL(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "LineNumberOfEntries" )==0 )
	{
		*data = uintdup(1);
	}
	else if( strcmp( lastname, "ChannelNumberOfEntries" )==0 )
	{
		*data = uintdup(1);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


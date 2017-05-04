// Implementation of IPInterface:2 and IPv6Interface:1

#include <stdio.h>
#include <stdlib.h>

#include <parameter_api.h>

#include "../cwmpc_utility.h"
#include "prmt_ip.h"
#include "prmt_ip_if.h"
#include "prmt_ip_diag.h"

/****** Device.IP ***********************************************************/
struct CWMP_OP tIPLeafOP = { getIP, NULL};
struct CWMP_OP tIPIfObjectOP = { NULL, objIPIf};

struct CWMP_PRMT tIPLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"IPv4Capable",	eCWMP_tBOOLEAN,	CWMP_READ,	&tIPLeafOP},
{"IPv4Enable",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPLeafOP},
{"IPv4Status",	eCWMP_tSTRING,	CWMP_READ,	&tIPLeafOP},
#ifdef CONFIG_IPV6	
{"IPv6Capable",	eCWMP_tBOOLEAN,	CWMP_READ,	&tIPLeafOP},
{"IPv6Enable",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPLeafOP},
{"IPv6Status",	eCWMP_tSTRING,	CWMP_READ,	&tIPLeafOP},
{"ULAPrefix",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPLeafOP},
#endif
{"InterfaceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tIPLeafOP},
};

enum eIPLeaf
{
	eIPv4Capable,
	eIPv4Enable,
	eIPv4Status,
#ifdef CONFIG_IPV6
	eIPv6Capable,
	eIPv6Enable,
	eIPv6Status,
	eULAPrefix,
#endif
	eInterfaceNumberOfEntries,
};

struct CWMP_LEAF tIPLeaf[] =
{
{ &tIPLeafInfo[eIPv4Capable]  },
{ &tIPLeafInfo[eIPv4Enable]  },
{ &tIPLeafInfo[eIPv4Status]  },
#ifdef CONFIG_IPV6
{ &tIPLeafInfo[eIPv6Capable]  },
{ &tIPLeafInfo[eIPv6Enable]  },
{ &tIPLeafInfo[eIPv6Status]  },
{ &tIPLeafInfo[eULAPrefix]  },
#endif
{ &tIPLeafInfo[eInterfaceNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tIPObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Interface",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tIPIfObjectOP},
{"Diagnostics",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eIPObject
{
	eInterface,
	eDiagnostics,
};

struct CWMP_NODE tIPObject[] =
{
/*info,  				leaf,			next)*/
{&tIPObjectInfo[eInterface],			NULL, NULL},
{&tIPObjectInfo[eDiagnostics],			NULL, tIPDiagObject},
{NULL,					NULL,			NULL}
};

/******* Operations **********************************************************/
int getIP(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "IPv4Capable" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "IPv4Enable" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "IPv4Status" )==0 )
	{
		*data = strdup("Enabled");
	}
#ifdef CONFIG_IPV6
	else if( strcmp( lastname, "IPv6Capable" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "IPv6Enable" )==0 )
	{
		unsigned char enabled = 0;

		if(mib_get(MIB_V6_IPV6_ENABLE, (void *)&enabled) == 0)
			CWMPDBG(1, (stderr, "Get IPv6 Enable mib error!\n"));

		*data = booldup(enabled);
	}
	else if( strcmp( lastname, "IPv6Status" )==0 )
	{
		unsigned char enabled = 0;

		if(mib_get(MIB_V6_IPV6_ENABLE, (void *)&enabled) == 0)
			CWMPDBG(1, (stderr, "Get IPv6 Enable mib error!\n"));

		if(enabled)
			*data = strdup("Enabled");
		else
			*data = strdup("Disabled");
	}
	else if( strcmp( lastname, "ULAPrefix" )==0 )
	{
		unsigned char enabled = 0;

		if(mib_get(MIB_V6_ULAPREFIX_ENABLE, (void *)&enabled) == 0)
			CWMPDBG(1, (stderr, "<%s:%d> Get ULA Prefix Enable mib error!\n", __FUNCTION__, __LINE__));

		if(enabled)
		{
			char buf[64] = {0};
			char len[8] = {0};

			if(mib_get(MIB_V6_ULAPREFIX, (void *)buf) == 0)
				CWMPDBG(1, (stderr, "<%s:%d> Get ULA Prefix Enable mib error!\n", __FUNCTION__, __LINE__));

			if(mib_get(MIB_V6_ULAPREFIX_LEN, (void *)len) == 0)
				CWMPDBG(1, (stderr, "<%s:%d> Get ULA Prefix Length Enable mib error!\n", __FUNCTION__, __LINE__));

			strncat(buf, len, 64);
			*data = strdup(buf);
		}
		else
			*data = strdup("");	
	}
#endif
	else if( strcmp( lastname, "InterfaceNumberOfEntries" )==0 )
	{
		*data = uintdup(get_ip_if_cnt());
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


#include <rtk/mib.h>
#include <rtk/utility.h>

#include <cwmp_utility.h>

#include "../prmt_apply.h"
#include "prmt_dhcpv4_relay.h"

/***** Device.DHCPv4.Relay.1. ************************************************/
struct CWMP_OP tDHCPv4RelayFWEntityLeafOP = { getDHCPv4RelayFWEntity, setDHCPv4RelayFWEntity };
struct CWMP_PRMT tDHCPv4RelayFWEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tDHCPv4RelayFWEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,	&tDHCPv4RelayFWEntityLeafOP},
{"Order",			eCWMP_tUINT,	CWMP_READ/*|CWMP_WRITE*/,	&tDHCPv4RelayFWEntityLeafOP},
{"Interface",		eCWMP_tSTRING,	CWMP_READ/*|CWMP_WRITE*/,	&tDHCPv4RelayFWEntityLeafOP},
{"LocallyServed",	eCWMP_tBOOLEAN,	CWMP_READ/*|CWMP_WRITE*/,	&tDHCPv4RelayFWEntityLeafOP},
{"DHCPServerIPAddress",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tDHCPv4RelayFWEntityLeafOP},
{"VendorClassID",	eCWMP_tSTRING,	CWMP_READ/*|CWMP_WRITE*/,	&tDHCPv4RelayFWEntityLeafOP},
/* VendorClassIDExclude */
/* VendorClassIDMode */
{"ClientID",		eCWMP_tHEXBIN,	CWMP_READ/*|CWMP_WRITE*/,	&tDHCPv4RelayFWEntityLeafOP},
/* ClientIDExclude */
{"UserClassID",		eCWMP_tHEXBIN,	CWMP_READ/*|CWMP_WRITE*/,	&tDHCPv4RelayFWEntityLeafOP},
/* UserClassIDExclude */
{"Chaddr",			eCWMP_tSTRING,	CWMP_READ/*|CWMP_WRITE*/,	&tDHCPv4RelayFWEntityLeafOP},
{"ChaddrMask",		eCWMP_tSTRING,	CWMP_READ/*|CWMP_WRITE*/,	&tDHCPv4RelayFWEntityLeafOP},
/* ChaddrExclude */
};

enum eDHCPv4RelayFWEntityLeaf
{
	eDHCPv4RelayFWEnable,
	eDHCPv4RelayFWStatus,
	eDHCPv4RelayFWOrder,
	eDHCPv4RelayFWInterface,
	eDHCPv4RelayFWLocallyServed,
	eDHCPv4RelayFWDHCPServerIPAddress,
	eDHCPv4RelayFWVendorClassID,
	eDHCPv4RelayFWClientID,
	eDHCPv4RelayFWUserClassID,
	eDHCPv4RelayFWChaddr,
	eDHCPv4RelayFWChaddrMask,
};

struct CWMP_LEAF tDHCPv4RelayFWEntityLeaf[] =
{
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWEnable]  },
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWStatus]  },
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWOrder]  },
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWInterface]  },
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWLocallyServed]  },
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWDHCPServerIPAddress]  },
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWVendorClassID]  },
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWClientID]  },
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWUserClassID]  },
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWChaddr]  },
{ &tDHCPv4RelayFWEntityLeafInfo[eDHCPv4RelayFWChaddrMask]  },
{ NULL	}
};

/***** Device.DHCPv4.Relay.Forwarding. ***************************************/
struct CWMP_PRMT tDHCPv4RelayFWObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"1",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eDHCPv4RelayFWObject
{
	eDHCPv4RelayFW1,
};

struct CWMP_NODE tDHCPv4RelayFWObject[] =
{
/*info,  					leaf,			node)*/
{&tDHCPv4RelayFWObjectInfo[eDHCPv4RelayFW1],	tDHCPv4RelayFWEntityLeaf,	NULL},
{NULL,						NULL,			NULL}
};

/***** Device.DHCPv4.Relay ***************************************************/
struct CWMP_OP tDHCPv4RelayLeafOP = { getDHCPv4Relay, setDHCPv4Relay };
struct CWMP_PRMT tDHCPv4RelayLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_READ,	&tDHCPv4RelayLeafOP},
{"ForwardingNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tDHCPv4RelayLeafOP},
};

enum eDHCPv4RelayLeaf
{
	eDHCPv4RelayEnable,
	eDHCPv4RelayForwardingNumberOfEntries,
};

struct CWMP_LEAF tDHCPv4RelayLeaf[] =
{
{ &tDHCPv4RelayLeafInfo[eDHCPv4RelayEnable]  },
{ &tDHCPv4RelayLeafInfo[eDHCPv4RelayForwardingNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tDHCPv4RelayObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Forwarding",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eDHCPv4RelayObject
{
	eDHCPv4RelayFW,
};

struct CWMP_NODE tDHCPv4RelayObject[] =
{
/*info,  					leaf,			node)*/
{&tDHCPv4RelayObjectInfo[eDHCPv4RelayFW],	NULL,	tDHCPv4RelayFWObject},
{NULL,						NULL,			NULL}
};

/***** Operations ************************************************************/
int getDHCPv4Relay(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		unsigned char vChar;
		mib_get( MIB_DHCP_MODE, (void *)&vChar);
		*data = booldup( (vChar==DHCP_LAN_RELAY) );
	}
	else if( strcmp( lastname, "ForwardingNumberOfEntries" )==0 )
	{
		*data = uintdup(1);
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setDHCPv4Relay(char *name, struct CWMP_LEAF *entity, int type, void *data)
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
		enable = (*i==0) ? 0 : DHCP_LAN_RELAY;
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


int getDHCPv4RelayFWEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;;
	unsigned int inst_num;
	*type = entity->info->type;
	*data = NULL;

	inst_num = getInstNum( name, "DHCPv4.Relay.Forwarding" );

	if(inst_num != 1)
		return ERR_9005;

	if( strcmp( lastname, "Enable" )==0 )
	{
		unsigned char vChar;
		mib_get( MIB_DHCP_MODE, (void *)&vChar);
		*data = booldup( (vChar==DHCP_LAN_RELAY) );
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		unsigned char vChar;
		mib_get( MIB_DHCP_MODE, (void *)&vChar);
		*data = strdup( (vChar==DHCP_LAN_RELAY) ? "Enabled" : "Disabled" );
	}
	else if( strcmp( lastname, "Order" )==0 )
	{
		// only one row
		*data = uintdup(1);
	}
	else if( strcmp( lastname, "Interface" )==0 )
	{
		*data = strdup("Device.IP.Interface.1");
	}
	else if( strcmp( lastname, "VendorClassID" )==0 )
	{
		*data = strdup("");
	}
	else if( strcmp( lastname, "ClientID" )==0 )
	{
		struct xsd__hexBinary id = {0};
		*data = (void *)hexBinarydup(id);
	}
	else if( strcmp( lastname, "UserClassID" )==0 )
	{
		struct xsd__hexBinary id = {0};
		*data = (void *)hexBinarydup(id);
	}
	else if( strcmp( lastname, "Chaddr" )==0 )
	{
		*data = strdup("");
	}
	else if( strcmp( lastname, "ChaddrMask" )==0 )
	{
		*data = strdup("");
	}
	else if( strcmp( lastname, "LocallyServed" )==0 )
	{
		*data = booldup(0);
	}
	else if( strcmp( lastname, "DHCPServerIPAddress" )==0 )
	{
		char buf[32] = {0};
		struct in_addr addr = {0};

		mib_get(MIB_ADSL_WAN_DHCPS, &addr);

		strcpy(buf, inet_ntoa(addr));
		*data=strdup( buf );	
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}

int setDHCPv4RelayFWEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
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
		enable = (*i==0) ? 0 : DHCP_LAN_RELAY;
		mib_set(MIB_DHCP_MODE, (void *)&enable);

		apply_add( CWMP_PRI_L, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "DHCPServerIPAddress" )==0 )
	{
		struct in_addr addr = {0};

		if( buf==NULL ) return ERR_9007;

		if(!inet_aton(buf, (struct in_addr *)&addr)) return ERR_9007;

		mib_set(MIB_ADSL_WAN_DHCPS, (void *)&addr);

		apply_add( CWMP_PRI_N, apply_DHCP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


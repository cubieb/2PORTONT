// Implementation of IPInterface:2 and IPv6Interface:1

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <linux/rtnetlink.h>

#include <net/if.h>
#include <linux/sockios.h>
#include <linux/if_addr.h>

#include <rtk/utility.h>
#include <parameter_api.h>
#include <libcwmp.h>

#include "../cwmpc_utility.h"
#include "../prmt_apply.h"
#include "prmt_ip_if.h"

#ifdef CONFIG_IPV6
/****** Device.IP.Interface.{i}.IPv6Address.{i} ******************************/
struct CWMP_OP tIPv6AddrEntityLeafOP = { getIPv6AddrEntity, NULL/*setIPv6AddrEntity*/ };

struct CWMP_PRMT tIPv6AddrEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPv6AddrEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,		&tIPv6AddrEntityLeafOP},
{"IPAddressStatus",		eCWMP_tSTRING,	CWMP_READ,		&tIPv6AddrEntityLeafOP},
/*{"Alias",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE|CWMP_DENY_ACT,	&tIPv6AddrEntityLeafOP},*/
{"IPAddress",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPv6AddrEntityLeafOP},
{"Origin",		eCWMP_tSTRING,	CWMP_READ,	&tIPv6AddrEntityLeafOP},
{"Prefix",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPv6AddrEntityLeafOP},
{"PreferredLifetime",	eCWMP_tDATETIME,	CWMP_READ|CWMP_WRITE,	&tIPv6AddrEntityLeafOP},
{"ValidLifetime",	eCWMP_tDATETIME,	CWMP_READ|CWMP_WRITE,	&tIPv6AddrEntityLeafOP},
{"Anycast",			eCWMP_tBOOLEAN, 	CWMP_READ|CWMP_WRITE,	&tIPv6AddrEntityLeafOP},
};

enum eIPv6AddrEntityLeaf
{
	eIPv6EntityEnable,
	eIPv6EntityStatus,
	eIPv6EntityIPAddressStatus,
//	eIPv6EntityAlias,
	eIPv6EntityIPAddress,
	eIPv6EntityOrigin,
	eIPv6EntityPrefix,
	eIPv6EntityPreferredLifetime,
	eIPv6EntityValidLifetime,
	eIPv6EntityAnycast,
};

struct CWMP_LEAF tIPv6AddrEntityLeaf[] =
{
{ &tIPv6AddrEntityLeafInfo[eIPv6EntityEnable]  },
{ &tIPv6AddrEntityLeafInfo[eIPv6EntityStatus]  },
{ &tIPv6AddrEntityLeafInfo[eIPv6EntityIPAddressStatus]  },
//{ &tIPv6AddrEntityLeafInfo[eIPv6EntityAlias]  },
{ &tIPv6AddrEntityLeafInfo[eIPv6EntityIPAddress]  },
{ &tIPv6AddrEntityLeafInfo[eIPv6EntityOrigin]  },
{ &tIPv6AddrEntityLeafInfo[eIPv6EntityPrefix]  },
{ &tIPv6AddrEntityLeafInfo[eIPv6EntityPreferredLifetime]  },
{ &tIPv6AddrEntityLeafInfo[eIPv6EntityValidLifetime]  },
{ &tIPv6AddrEntityLeafInfo[eIPv6EntityAnycast]  },
{ NULL	}
};

/****** Device.IP.Interface.{i}.IPv6Address **********************************/
struct CWMP_PRMT tIPv6AddrObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum eIPv6AddrObject
{
	eIPv6Addr0,
};

struct CWMP_LINKNODE tIPv6AddrObject[] =
{
/*info, 						leaf,				next,				sibling,	instnum)*/
{&tIPv6AddrObjectInfo[eIPv6Addr0], tIPv6AddrEntityLeaf, NULL, NULL, 0},
};

/****** Device.IP.Interface.{i}.IPv6Prefix.{i} *******************************/
struct CWMP_OP tIPv6PrefixEntityLeafOP = { getIPv6PrefixEntity, NULL/*setIPv6PrefixEntity*/ };

struct CWMP_PRMT tIPv6PrefixEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPv6PrefixEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,		&tIPv6PrefixEntityLeafOP},
{"PrefixStatus",	eCWMP_tSTRING,	CWMP_READ,		&tIPv6PrefixEntityLeafOP},
{"Prefix",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPv6PrefixEntityLeafOP},
{"Origin",		eCWMP_tSTRING,	CWMP_READ,		&tIPv6PrefixEntityLeafOP},
//{"StaticType",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPv6PrefixEntityLeafOP},
//{"ParentPrefix",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPv6PrefixEntityLeafOP},
//{"ChildPrefixBits",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPv6PrefixEntityLeafOP},
{"OnLink",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPv6PrefixEntityLeafOP},
{"Autonomous",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPv6PrefixEntityLeafOP},
{"PreferredLifetime",	eCWMP_tDATETIME,	CWMP_READ|CWMP_WRITE,	&tIPv6PrefixEntityLeafOP},
{"ValidLifetime",		eCWMP_tDATETIME,	CWMP_READ|CWMP_WRITE,	&tIPv6PrefixEntityLeafOP},
};

enum eIPv6PrefixEntityLeaf
{
	eIPv6PrefixEntityEnable,
	eIPv6PrefixEntityStatus,
	eIPv6PrefixEntityPrefixStatus,
	eIPv6PrefixEntityPrefix,
	eIPv6PrefixEntityOrigin,
//	eIPv6PrefixEntityStaticType,
//	eIPv6PrefixEntityParentPrefix,
//	eIPv6PrefixEntityChildPrefixBits,
	eIPv6PrefixEntityOnLink,
	eIPv6PrefixEntityAutonomous,
	eIPv6PrefixEntityPreferredLifetime,
	eIPv6PrefixEntityValidLifetime,
};

struct CWMP_LEAF tIPv6PrefixEntityLeaf[] =
{
{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityEnable]  },
{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityStatus]  },
{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityPrefixStatus]  },
{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityPrefix]  },
{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityOrigin]  },
//{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityStaticType]  },
//{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityParentPrefix]  },
//{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityChildPrefixBits]  },
{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityOnLink]  },
{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityAutonomous]  },
{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityPreferredLifetime]  },
{ &tIPv6PrefixEntityLeafInfo[eIPv6PrefixEntityValidLifetime]  },
{ NULL	}
};

/****** Device.IP.Interface.{i}.IPv6Prefix ***********************************/
struct CWMP_PRMT tIPv6PrefixObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum eIPv6PrefixObject
{
	eIPv6Prefix0,
};

struct CWMP_LINKNODE tIPv6PrefixObject[] =
{
/*info, 						leaf,				next,				sibling,	instnum)*/
{&tIPv6PrefixObjectInfo[eIPv6Prefix0], tIPv6PrefixEntityLeaf, NULL, NULL, 0},
};
#endif //CONFIG_IPV6

/****** Device.IP.Interface.{i}.IPv4Address.{i} ******************************/
struct CWMP_OP tIPv4AddrEntityLeafOP = { getIPv4AddrEntity, setIPv4AddrEntity };

struct CWMP_PRMT tIPv4AddrEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPv4AddrEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,		&tIPv4AddrEntityLeafOP},
/*{"Alias",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE|CWMP_DENY_ACT,	&tIPv4AddrEntityLeafOP},*/
{"IPAddress",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPv4AddrEntityLeafOP},
{"SubnetMask",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tIPv4AddrEntityLeafOP},
{"AddressingType",	eCWMP_tSTRING,	CWMP_READ,	&tIPv4AddrEntityLeafOP},
};

enum eIPv4AddrEntityLeaf
{
	eIPv4EntityEnable,
	eIPv4EntityStatus,
//	eIPv4EntityAlias,
	eIPv4EntityIPAddress,
	eIPv4EntitySubnetMask,
	eIPv4EntityAddressingType,
};

struct CWMP_LEAF tIPv4AddrEntityLeaf[] =
{
{ &tIPv4AddrEntityLeafInfo[eIPv4EntityEnable]  },
{ &tIPv4AddrEntityLeafInfo[eIPv4EntityStatus]  },
//{ &tIPv4AddrEntityLeafInfo[eIPv4EntityAlias]  },
{ &tIPv4AddrEntityLeafInfo[eIPv4EntityIPAddress]  },
{ &tIPv4AddrEntityLeafInfo[eIPv4EntitySubnetMask]  },
{ &tIPv4AddrEntityLeafInfo[eIPv4EntityAddressingType]  },
{ NULL	}
};

/****** Device.IP.Interface.{i}.IPv4Address **********************************/
struct CWMP_PRMT tIPv4AddrObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eIPv4AddrObject
{
	eIPv4Addr1,
	eIPv4Addr2,
};

struct CWMP_NODE tWANIPv4AddrObject[] =
{
/*info,  				leaf,			next)*/
{&tIPv4AddrObjectInfo[eIPv4Addr1],	tIPv4AddrEntityLeaf,	NULL},
{NULL,					NULL,			NULL}
};

struct CWMP_NODE tLANIPv4AddrObject[] =
{
/*info,  				leaf,			next)*/
{&tIPv4AddrObjectInfo[eIPv4Addr1],	tIPv4AddrEntityLeaf,	NULL},
#ifdef CONFIG_SECONDARY_IP
{&tIPv4AddrObjectInfo[eIPv4Addr2],	tIPv4AddrEntityLeaf,	NULL},
#endif
{NULL,					NULL,			NULL}
};

/****** Device.IP.Interface.{i}.Stats ****************************************/
struct CWMP_OP tIPIfStatsLeafOP = { getIPIfStats, NULL};

struct CWMP_PRMT tIPIfStatsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"BytesSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"BytesReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"PacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"PacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"ErrorsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"ErrorsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"UnicastPacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"UnicastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"DiscardPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"DiscardPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"MulticastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"MulticastPacketsReceived",eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"BroadcastPacketsSent",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tIPIfStatsLeafOP},
{"BroadcastPacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,	&tIPIfStatsLeafOP},
{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tIPIfStatsLeafOP},
};

enum eIPIfStatsLeaf
{
	eStatsBytesSent,
	eStatsBytesReceived,
	eStatsPacketsSent,
	eStatsPacketsReceived,
	eStatsErrorsSent,
	eStatsErrorsReceived,
	eStatsUnicastPacketsSent,
	eStatsUnicastPacketsReceived,
	eStatsDiscardPacketsSent,
	eStatsDiscardPacketsReceived,
	eStatsMulticastPacketsSent,
	eStatsMulticastPacketsReceived,
	eStatsBroadcastPacketsSent,
	eStatsBroadcastPacketsReceived,
	eStatsUnknownProtoPacketsReceived,
};

struct CWMP_LEAF tIPIfStatsLeaf[] =
{
{ &tIPIfStatsLeafInfo[eStatsBytesSent]  },
{ &tIPIfStatsLeafInfo[eStatsBytesReceived]  },
{ &tIPIfStatsLeafInfo[eStatsPacketsSent]  },
{ &tIPIfStatsLeafInfo[eStatsPacketsReceived]  },
{ &tIPIfStatsLeafInfo[eStatsErrorsSent]  },
{ &tIPIfStatsLeafInfo[eStatsErrorsReceived]  },
{ &tIPIfStatsLeafInfo[eStatsUnicastPacketsSent]  },
{ &tIPIfStatsLeafInfo[eStatsUnicastPacketsReceived]  },
{ &tIPIfStatsLeafInfo[eStatsDiscardPacketsSent]  },
{ &tIPIfStatsLeafInfo[eStatsDiscardPacketsReceived]  },
{ &tIPIfStatsLeafInfo[eStatsMulticastPacketsSent]  },
{ &tIPIfStatsLeafInfo[eStatsMulticastPacketsReceived]  },
{ &tIPIfStatsLeafInfo[eStatsBroadcastPacketsSent]  },
{ &tIPIfStatsLeafInfo[eStatsBroadcastPacketsReceived]  },
{ &tIPIfStatsLeafInfo[eStatsUnknownProtoPacketsReceived]  },
{ NULL	}
};

/****** Device.IP.Interface.{i} *********************************************/
struct CWMP_LINKNODE *gIPIfEntityObjList = NULL;

struct CWMP_OP tIPIfEntityLeafOP = { getIPIfEntity, setIPIfEntity };
#ifdef CONFIG_IPV6
struct CWMP_OP tIPv6AddrObjectOP = { NULL, objIPv6Addr};
struct CWMP_OP tIPv6PrefixObjectOP = { NULL, objIPv6Prefix};
#endif

struct CWMP_PRMT tIPIfEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPIfEntityLeafOP},
{"IPv4Enable",	eCWMP_tBOOLEAN, CWMP_READ|CWMP_WRITE,	&tIPIfEntityLeafOP},
#ifdef CONFIG_IPV6
{"IPv6Enable",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPIfEntityLeafOP},
{"ULAEnable",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPIfEntityLeafOP},
#endif
{"Status",		eCWMP_tSTRING,	CWMP_READ,		&tIPIfEntityLeafOP},
/*{"Alias",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE|CWMP_DENY_ACT,	&tIPIfEntityLeafOP},*/
{"Name",		eCWMP_tSTRING,	CWMP_READ,		&tIPIfEntityLeafOP},
{"LastChange",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tIPIfEntityLeafOP},
{"LowerLayers",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tIPIfEntityLeafOP},
{"MaxMTUSize",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tIPIfEntityLeafOP},
{"Type",		eCWMP_tSTRING,	CWMP_READ,		&tIPIfEntityLeafOP},
{"Reset",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tIPIfEntityLeafOP},
{"IPv4AddressNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tIPIfEntityLeafOP},
#ifdef CONFIG_IPV6
{"IPv6AddressNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tIPIfEntityLeafOP},
{"IPv6PrefixNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tIPIfEntityLeafOP},
#endif
};

enum eIPIfEntityLeaf
{
	eIfEntityEnable,
	eIfEntityIPv4Enable,
#ifdef CONFIG_IPV6
	eIfEntityIPv6Enable,
	eIfEntityULAEnable,
#endif
	eIfEntityStatus,
//	eIfEntityAlias,
	eIfEntityName,
	eIfEntityLastChange,
	eIfEntityLowerLayers,
	eIfEntityMaxMTUSize,
	eIfEntityType,
	eIfEntityReset,
	eIfEntityIPv4AddressNumberOfEntries,
#ifdef CONFIG_IPV6
	eIfEntityIPv6AddressNumberOfEntries,
	eIfEntityIPv6PrefixNumberOfEntries,
#endif
};

struct CWMP_LEAF tIPIfEntityLeaf[] =
{
{ &tIPIfEntityLeafInfo[eIfEntityEnable]  },
{ &tIPIfEntityLeafInfo[eIfEntityIPv4Enable]  },
#ifdef CONFIG_IPV6
{ &tIPIfEntityLeafInfo[eIfEntityIPv6Enable]  },
{ &tIPIfEntityLeafInfo[eIfEntityULAEnable]  },
#endif
{ &tIPIfEntityLeafInfo[eIfEntityStatus]  },
//{ &tIPIfEntityLeafInfo[eIfEntityAlias]  },
{ &tIPIfEntityLeafInfo[eIfEntityName]  },
{ &tIPIfEntityLeafInfo[eIfEntityLastChange]  },
{ &tIPIfEntityLeafInfo[eIfEntityLowerLayers]  },
{ &tIPIfEntityLeafInfo[eIfEntityMaxMTUSize]  },
{ &tIPIfEntityLeafInfo[eIfEntityType]  },
{ &tIPIfEntityLeafInfo[eIfEntityReset]  },
{ &tIPIfEntityLeafInfo[eIfEntityIPv4AddressNumberOfEntries]	},
#ifdef CONFIG_IPV6
{ &tIPIfEntityLeafInfo[eIfEntityIPv6AddressNumberOfEntries] },
{ &tIPIfEntityLeafInfo[eIfEntityIPv6PrefixNumberOfEntries]	},
#endif
{ NULL	}
};

struct CWMP_PRMT tIPIfEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"IPv4Address",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
#ifdef CONFIG_IPV6
{"IPv6Address",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tIPv6AddrObjectOP},
{"IPv6Prefix", 	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tIPv6PrefixObjectOP},
#endif
{"Stats",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eIPIfEntityObject
{
	eIPIfEntityIPv4Address,
#ifdef CONFIG_IPV6
	eIPIfEntityIPv6Address,
	eIPIfEntityIPv6Prefix,
#endif
	eIPIfEntityStats,
};

struct CWMP_NODE tIPIfEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tIPIfEntityObjectInfo[eIPIfEntityIPv4Address],	NULL,	NULL},
#ifdef CONFIG_IPV6
{&tIPIfEntityObjectInfo[eIPIfEntityIPv6Address],	NULL,	NULL},
{&tIPIfEntityObjectInfo[eIPIfEntityIPv6Prefix],		NULL,	NULL},
#endif	
{&tIPIfEntityObjectInfo[eIPIfEntityStats],	tIPIfStatsLeaf,	NULL},
{NULL,					NULL,			NULL}
};

/****** Device.IP.Interface *********************************************/
struct CWMP_PRMT tIPIfObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum eIPIfObject
{
	eIPIf0,
};

struct CWMP_LINKNODE tIPIfObject[] =
{
/*info, 						leaf,				next,				sibling,	instnum)*/
{&tIPIfObjectInfo[eIPIf0], tIPIfEntityLeaf, tIPIfEntityObject, NULL, 0},
};

/***** Public Utilities *****************************************************/
int inline get_ip_if_cnt()
{
	return mib_chain_total(MIB_ATM_VC_TBL) + 1;	// first one is br0(LAN)
}

/* Use <num> to get VC table, chain index, and ifname */
int get_ip_if_info(int num, MIB_CE_ATM_VC_T *pEntry, int *idx, char *ifname)
{
	int numofentries = mib_chain_total(MIB_ATM_VC_TBL);

	if(pEntry == NULL)
		return -1;

	if(num == 1)
	{
		strcpy(ifname, "br0");
		memset(pEntry, 0, sizeof(MIB_CE_ATM_VC_T));
		return 0;
	}
	else
	{
		int i;

		for( i = 0 ; i < numofentries ; i++ )
		{
			if( !mib_chain_get( MIB_ATM_VC_TBL, i, (void*)pEntry ))
				continue;

			if( num == pEntry->ConDevInstNum + 1)	//first 1 is for LAN
				break;
		}
		if(i == numofentries)
			goto error;

		if(ifGetName(pEntry->ifIndex, ifname, IFNAMSIZ) != 0)
		{
			*idx = i;
			return 0;	//OK
		}
	}

error:
	memset(pEntry, 0, sizeof(MIB_CE_ATM_VC_T));
	return -1;
}

// Use Device.IP.Interface.<num> to get ifname
int get_ip_if_ifname(char *name, char *ifname)
{
	int num = getInstNum(name, "IP.Interface");
	MIB_CE_ATM_VC_T vc_entity;
	int numofentries = mib_chain_total(MIB_ATM_VC_TBL);
	
	if(ifname == NULL)
		return -1;

	ifname[0] = '\0';

	if(name == NULL || strlen(name) == 0)
		return 0;	//empty name is valid

	if(num < 1)
		return -1;

	if(num == 1)
	{
		strcpy(ifname, "br0");
		return 0;
	}
	else
	{
		int i;

		for( i = 0 ; i < numofentries ; i++ )
		{
			if( !mib_chain_get( MIB_ATM_VC_TBL, i, &vc_entity ))
				continue;

			if( vc_entity.ConDevInstNum + 1 == num )
				break;
		}
		if(i == numofentries)
			return -1;

		if(ifGetName(vc_entity.ifIndex, ifname, IFNAMSIZ) != 0)
			return 0;	//OK
	}

	return -1;
}

int ifname_to_ip_if_path( char *ifname, char *path )
{
	//struct CWMP_LEAF *e=NULL;
	MIB_CE_ATM_VC_T *pEntry,vc_entity;
	int total,i;
	unsigned int ifindex=DUMMY_IFINDEX;

	if( (ifname==NULL) || ( path==NULL ) )
		return -1;
	path[0]=0;

	if( i < total || strcmp( ifname, "br0" ) == 0 )
	{
		sprintf(path, "Device.IP.Interface.%d", 1);
		return 0;
	}

	//WAN Interface
	total = mib_chain_total(MIB_ATM_VC_TBL);
	for( i=0; i<total; i++ )
	{
		char tmp_ifname[32];
		pEntry = &vc_entity;
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, (void*)pEntry ) )
			continue;

		ifGetName(pEntry->ifIndex, tmp_ifname, sizeof(tmp_ifname));

		if( strcmp(ifname, tmp_ifname)==0 )
			break;
	}

	return -1;
}


// Use ifindex to get full path name like Device.IP.Interface.<num>
// Only for WAN
int ifIdex_to_ip_if_path( int ifindex, char *path )
{
	MIB_CE_ATM_VC_T vc_entity;
	int numofentries = mib_chain_total(MIB_ATM_VC_TBL);
	int i;
	
	if(path == NULL)
		return -1;

	for(i = 0 ; i < numofentries ; i++)
	{
		if( !mib_chain_get(MIB_ATM_VC_TBL, i, &vc_entity ) )
			continue;
		if(vc_entity.ifIndex == ifindex)
		{
			sprintf(path, "Device.IP.Interface.%d", vc_entity.ConDevInstNum + 1);	//first 1 is for LAN
			return 0;
		}
	}

	return -1;
}

// only for WAN
int ip_if_path_to_ifIndex(char *path)
{
	struct CWMP_LEAF *e=NULL;
	unsigned int ret=DUMMY_IFINDEX;
	unsigned int num;
	MIB_CE_ATM_VC_T vc_entity;
	int numofentries = mib_chain_total(MIB_ATM_VC_TBL);
	int i;

	if( path == NULL )
		return DUMMY_IFINDEX;

	if( get_ParameterEntity( path, &e ) < 0 )
		return DUMMY_IFINDEX;

	num = getInstNum(path, "IP.Interface");
	if( num == 0 || num == 1/*LAN*/)
		return DUMMY_IFINDEX;

	for(i = 0 ; i < numofentries ; i++)
	{
		if( !mib_chain_get( MIB_ATM_VC_TBL, i, &vc_entity ) )
			continue;

		if(num == vc_entity.ConDevInstNum + 1)
			return vc_entity.ifIndex;
	}

	return DUMMY_IFINDEX;
}

struct IPIfEntityData* get_ip_if_entity_data(int num)
{
	struct CWMP_LINKNODE *node = find_SiblingEntity(&gIPIfEntityObjList, num);
	struct IPIfEntityData *data = NULL;

	if(node)
		data = (struct IPIfEntityData *)node->obj_data;

	return data;
}

void free_ip_if_obj_data(void *obj_data)
{
	struct IPIfEntityData *data = (struct IPIfEntityData *)obj_data;

	if(data == NULL)
		return;

	if(data->LowerLayers)
		free(data->LowerLayers);

	free(data);
}

/***** Private Utilities *****************************************************/
static int get_ip_if_LowerLayers(int if_num, MIB_CE_ATM_VC_T *pEntry, char *buf)
{
	struct IPIfEntityData *data = get_ip_if_entity_data(if_num);
	if(pEntry == NULL || buf == NULL)
		return -1;

	if(data == NULL)
		return ERR_9005;

	if(data && data->LowerLayers)
		strcpy(buf, data->LowerLayers);
	else switch(pEntry->cmode)
	{
	case CHANNEL_MODE_PPPOA:
	case CHANNEL_MODE_PPPOE:
		sprintf(buf, "Device.PPP.Interface.%d", pEntry->ConDevInstNum + 1); //first one is LAN
		break;
	case CHANNEL_MODE_RT1483:
		//TODO: Consider PPPoE multi-session
		sprintf(buf, "ATM.Link.Interface.%d", pEntry->ConDevInstNum + 1);
		break;
	case CHANNEL_MODE_IPOE:
		sprintf(buf, "Device.Ethernet.Link.%d", pEntry->ConDevInstNum + 1);	//first one is LAN	
		break;
	default:	//Bridge or tunnel
		sprintf(buf, "");
		break;
	}

	return 0;
}

static int set_ip_if_LowerLayers(int num, char *lower)
{
	int target = 0;
	struct IPIfEntityData *data = NULL;

	// No need to check num is valid and lower is not NULL.

	// Brige or tunnel
	if(strcmp(lower, "") == 0 )
		goto ok;

	// PPPoE or PPPoA
	target = getInstNum(lower, "PPP.Interface");
	if(target == num)
		goto ok;

	// IPoE
	target = getInstNum(lower, "Ethernet.Link");
	if(target == num)
		goto ok;

	// Route 1483
	target = getInstNum(lower, "ATM.Link");
	if(target == num)
		goto ok;

	return ERR_9007;

ok:
	data = get_ip_if_entity_data(num);
	if(data->LowerLayers)
		free(data->LowerLayers);
	data->LowerLayers = strdup(lower);
	return 0;
}

#ifdef CONFIG_IPV6
static int get_ipv6_addr_cnt(char *ifname)
{
	static const char *proc_if_inet6 = "/proc/net/if_inet6";
	FILE *f = NULL;
	char dev[IFNAMSIZ] = {0};
	int cnt = 0;

	if(ifname == NULL)
		return -1;

	f = fopen(proc_if_inet6, "r");
	if(f == NULL)
		return -1;

	while(fscanf(f, "%*s %*08x %*02x %*02x %*02x %s\n", dev) != EOF)
	{
		if(strcmp(dev, ifname) == 0)
			cnt++;
	}
	fclose(f);
	return cnt;
}

struct netlink_req
{
	struct nlmsghdr		nlmsg_info;
	struct ifaddrmsg	ifaddrmsg_info;
};

struct ip6_info
{
	char addr[IP6_ADDR_LEN];
	unsigned char prefix_len;
	unsigned char scope;
	unsigned char anycast;
	unsigned char flags; //ifa_flags
	time_t prefered;
	time_t valid;
};

static int get_ip6_info(char *ifname, struct ip6_info *info, int size)
{
	int fd;
	int rtn;
	struct netlink_req req = {0};
	char buf[4096] = {0};
	struct nlmsghdr *nlmsg_ptr;
	int nlmsg_len;
	int i = 0;
	int target = 0;

	//open netlink socket
	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
	if(fd < 0)
	{
		CWMPDBG(1, (stderr, "<%s:%d> Open netlink socket error, ifname = %s\n", __FUNCTION__, __LINE__,ifname));
		return -1;
	}

	//prepare request
	req.nlmsg_info.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg));
	req.nlmsg_info.nlmsg_flags = NLM_F_REQUEST | NLM_F_MATCH;
	req.nlmsg_info.nlmsg_type = RTM_GETADDR;
	req.nlmsg_info.nlmsg_pid = getpid();
	req.ifaddrmsg_info.ifa_family = AF_INET6;
	target = if_nametoindex(ifname);
	/*This statement cannot work*/
//	req.ifaddrmsg_info.ifa_index = if_nametoindex(ifname);

	//send request
	rtn = send (fd, &req, req.nlmsg_info.nlmsg_len, 0);
	if(rtn < 0)
	{
		close(fd);
		CWMPDBG(1, (stderr, "<%s:%d> Send netlink message error, ifname = %s\n", __FUNCTION__, __LINE__,ifname));
		return -1;
	}

	//receive & handle response
	while(1)
	{
		bzero(buf, 4096);

		nlmsg_ptr = (struct nlmsghdr *) buf;
		nlmsg_len = recv(fd, buf, 4096, 0);
		if(nlmsg_len < 0)
		{
		CWMPDBG(1, (stderr, "<%s:%d> Receive netlink message error, ifname = %s\n", __FUNCTION__, __LINE__,ifname));
			return -1;
		}

		if (nlmsg_len < sizeof (struct nlmsghdr))
		{
			close(fd);
			CWMPDBG(1, (stderr, "<%s:%d> Received an uncomplete netlink packet\n", __FUNCTION__, __LINE__,ifname));
			return -1;
		}

		if (nlmsg_ptr->nlmsg_type == NLMSG_DONE)
			break;

		for(; NLMSG_OK(nlmsg_ptr, nlmsg_len); nlmsg_ptr = NLMSG_NEXT(nlmsg_ptr, nlmsg_len))
		{
			struct ifaddrmsg *ifaddrmsg_ptr;
			struct rtattr *rtattr_ptr;
			int ifaddrmsg_len;

			ifaddrmsg_ptr = (struct ifaddrmsg *) NLMSG_DATA(nlmsg_ptr);

			rtattr_ptr = (struct rtattr *) IFA_RTA(ifaddrmsg_ptr);
			ifaddrmsg_len = IFA_PAYLOAD(nlmsg_ptr);

			if(ifaddrmsg_ptr->ifa_index != target)
				continue;
			
			info[i].scope = ifaddrmsg_ptr->ifa_scope;
			info[i].prefix_len = ifaddrmsg_ptr->ifa_prefixlen;
			info[i].flags = ifaddrmsg_ptr->ifa_flags;

			for(;RTA_OK(rtattr_ptr, ifaddrmsg_len); rtattr_ptr = RTA_NEXT(rtattr_ptr, ifaddrmsg_len)) {

				switch(rtattr_ptr->rta_type) {
				case IFA_ADDRESS:
					memcpy(info[i].addr, RTA_DATA(rtattr_ptr), IP6_ADDR_LEN);
					break;
				case IFA_ANYCAST:
					info[i].anycast = 1;
					break;
				case IFA_LOCAL:
				case IFA_BROADCAST:
				case IFA_MULTICAST:
				case IFA_LABEL:
				case IFA_CACHEINFO:
					{
						struct ifa_cacheinfo *ci =  (struct ifa_cacheinfo *) RTA_DATA(rtattr_ptr);

						/* struct ifa_cacheinfo has four members
							__u32 ifa_prefered    how long this address can be in preferred  state, in seconds. When preferred time is finished, this IPv6 address  will stop 
	communicating. (will not answer ping6, etc)
							__u32 ifa_valid       how long this prefix is valid, in seconds. When the valid time is over, the IPV6 address is removed.
							__u32 cstamp          created timestamp (hundreths of seconds)
							__u32 tstamp          updated timestamp (hundreths of seconds) This should change everytime this line changes
						*/
						if (ci->ifa_valid == 0xFFFFFFFFUL)
							info[i].valid = LONG_MAX;	//forever
						else
							info[i].valid = time(NULL) + ci->ifa_valid;

						if (ci->ifa_prefered == 0xFFFFFFFFUL)
							info[i].prefered = LONG_MAX;	//forever
						else
							info[i].prefered = time(NULL) + ci->ifa_prefered;
					}
					break;

				default:
					CWMPDBG (1, (stderr, "<%s:%d> unknown rta_type: %d\n", __FUNCTION__, __LINE__, (int)rtattr_ptr->rta_type));
					break; 

				}
			}

			i++;
		}
	}

	close(fd);
	return i;
}

int get_ip6prefix_cnt(int if_num)
{
	MIB_CE_ATM_VC_T vc_entity = {0};
	char ifname[IFNAMSIZ] = {0};
	int total_addr = 0;
	int cnt = 0;
	struct ip6_info info[6] = {0};
	int chain_idx;

	get_ip_if_info(if_num, &vc_entity, &chain_idx, ifname);
	total_addr = get_ip6_info(ifname, info, 6);

	return cnt;
}
#endif

/***** Operations ************************************************************/
int getIPIfEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char ifname[IFNAMSIZ] = {0};
	int if_num = 0;
	MIB_CE_ATM_VC_T vc_entity = {0};
	unsigned char is_br0 = 0;
	int chain_idx = -1;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if_num = getInstNum(name, "IP.Interface");
	if(if_num < 1)
		return ERR_9005;

	if(get_ip_if_info(if_num, &vc_entity, &chain_idx, ifname) < 0)
		return ERR_9005;

	if(strcmp(ifname, "br0") == 0)
		is_br0 = 1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		if(!is_br0)
		{
			if(vc_entity.enable)
				*data = booldup(1);
			else
				*data = booldup(0);
		}
		else
			*data = booldup(1);
	}
	else if( strcmp( lastname, "IPv4Enable" )==0 )
	{
#ifdef CONFIG_IPV6
		if(vc_entity.IpProtocol & IPVER_IPV4)
			*data = booldup(1);
		else
			*data = booldup(0);
#else
		*data = booldup(1);
#endif
	}
#ifdef CONFIG_IPV6
	else if( strcmp( lastname, "IPv6Enable" )==0 )
	{
		if(!is_br0 && (vc_entity.IpProtocol & IPVER_IPV6))
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "ULAEnable" )==0 )
	{
		unsigned char enabled = 0;

		if(mib_get(MIB_V6_ULAPREFIX_ENABLE, (void *)&enabled) == 0)
			CWMPDBG(1, (stderr, "<%s:%d> Get ULA Prefix Enable mib error!\n", __FUNCTION__, __LINE__));

		if(enabled)
			booldup(1);
		else
			booldup(0);
	}
#endif
	else if( strcmp( lastname, "Status" )==0 )
	{
		int flags;

		if(!is_br0 && vc_entity.enable==0)
			*data = strdup( "Down" );
		else if(getInFlags( ifname, &flags) == 1)
		{
			if (flags & IFF_RUNNING)
				*data = strdup( "Up" );
			else
				*data = strdup( "Down" );
		}
		else
			*data = strdup( "Down" );
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		*data = strdup("");
	}*/
	else if( strcmp( lastname, "Name" )==0 )
	{
		*data = strdup(ifname);
	}
	//TODO
	else if( strcmp( lastname, "LastChange" )==0 )
	{
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		char buf[1024] = {0};

		if(is_br0)
		{
			sprintf(buf, "Device.Ethernet.Link.1");
			*data = strdup(buf);
		}
		else if(get_ip_if_LowerLayers(if_num, &vc_entity, buf) == 0)
			*data = strdup(buf);
		else
			*data = strdup("");
	}
	else if( strcmp( lastname, "MaxMTUSize" )==0 )
	{
		int s, af = AF_INET;
		struct ifreq ifr;

		if ((s = socket(af, SOCK_DGRAM, 0)) < 0)
			err(1, "socket");

		ifr.ifr_addr.sa_family = AF_INET;
		strcpy(ifr.ifr_name, ifname);
		if (ioctl(s, SIOCGIFMTU, (caddr_t)&ifr) < 0)
			return ERR_9002;
		close(s);

		*data = uintdup(ifr.ifr_mtu);
	}
	else if( strcmp( lastname, "Type" )==0 )
	{
		if(is_br0)
			*data = strdup("Normal");
		else if(vc_entity.cmode == CHANNEL_MODE_6RD || vc_entity.cmode == CHANNEL_MODE_DSLITE)
			*data = strdup("Tunnel");
		else
			*data = strdup("Normal");
	}
	else if( strcmp( lastname, "Reset" )==0 )
	{
		// alway return false
		*data = booldup(0);
	}
	else if( strcmp( lastname, "IPv4AddressNumberOfEntries" )==0 )
	{
		// alway return false
		if(is_br0)
#ifdef CONFIG_SECONDARY_IP
			*data = uintdup(2);
#else
			*data = uintdup(1);
#endif
#ifdef CONFIG_IPV6
		else if(vc_entity.IpProtocol & IPVER_IPV4)
			*data = uintdup(1);
#endif
		else
			*data = uintdup(0);
	}
#ifdef CONFIG_IPV6
	else if( strcmp( lastname, "IPv6AddressNumberOfEntries" )==0 )
	{
		*data = uintdup(get_ipv6_addr_cnt(ifname));
	}
	else if( strcmp( lastname, "IPv6PrefixNumberOfEntries" )==0 )
	{
		*data = uintdup(get_ipv6_addr_cnt(ifname));
	}
#endif
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setIPIfEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf=data;
	int if_num;
	char ifname[IFNAMSIZ] = {0};
	MIB_CE_ATM_VC_T vc_entity = {0};
	int chainidx = -1;
	unsigned char is_br0 = 0;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	if_num = getInstNum(name, "Interface");
	if(if_num < 1)
		return ERR_9005;

	if(get_ip_if_info(if_num, &vc_entity, &chainidx, ifname) < 0)
		return ERR_9005;

	gWanBitMap |= (1U << chainidx);

	if(strcmp(ifname, "br0") == 0)
		is_br0 = 1;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		vc_entity.enable = (*i==0) ? 0:1;
		mib_chain_update( MIB_ATM_VC_TBL, &vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "IPv4Enable" )==0 )
	{
		int *i = data;

		if(*i!=1)
			return ERR_9001;
		return 0;
	}
	else if( strcmp( lastname, "IPv6Enable" )==0 )
	{
		int *i = data;

		if(is_br0 && *i==0)
			return ERR_9001;
		else if(!is_br0 && *i==1)
			vc_entity.IpProtocol |= IPVER_IPV6; 
		else
			vc_entity.IpProtocol &= ~IPVER_IPV6; 

		mib_chain_update( MIB_ATM_VC_TBL, &vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "ULAEnable" )==0 )
	{
		int *i = data;
		unsigned char enable = 0;

		enable = (*i==0) ? 0:1;
		mib_set(MIB_V6_ULAPREFIX_ENABLE, (void *)&enable);
		return CWMP_NEED_RESTART_WAN;
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		return ERR_9001;
	}*/
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		char *lower = data;
		int ret;

		if(is_br0)
			return ERR_9001;

		//only WAN can set lower layers
		ret = set_ip_if_LowerLayers(if_num, lower);
		if(ret != 0)
			return ret;

		apply_add( CWMP_PRI_SL, lower_layers_changed, CWMP_RESTART, 0, NULL, 0 );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "MaxMTUSize" )==0 )	// Jenny
	{
		unsigned int *newmtu = data;

		if( newmtu==NULL ) return ERR_9007;
		if(*newmtu<1 || *newmtu>1540) return ERR_9007;
		vc_entity.mtu = *newmtu;
		mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, chainidx );
		return CWMP_NEED_RESTART_WAN;
	}
	else if( strcmp( lastname, "Reset" )==0 )
	{
		int *i=data;
		if(i==NULL) return ERR_9007;
		if(*i==1)
		{
			resetChainID = chainidx;
			gStartReset = 1;
		}
		return CWMP_APPLIED;
	}
	else
	{
		return ERR_9005;
	}

	return CWMP_APPLIED;
}

int objIPIf(char *name, struct CWMP_LEAF *e, int type, void *data)
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

			//Move LAN object to new list
			tmp_entity = remove_SiblingEntity( &old_table, 1);
			if( tmp_entity!=NULL )
				add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, tmp_entity );
			else
			{
				unsigned int MaxInstNum = 1;
				add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tIPIfObject, sizeof(tIPIfObject), &MaxInstNum );
			}
			tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, 1 );
			if(tmp_entity)
			{
				tmp_entity->next[eIPIfEntityIPv4Address].next = tLANIPv4AddrObject;
			}
			gIPIfEntityObjList = (struct CWMP_LINKNODE *)entity->next;

			// Update WAN objects
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

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tIPIfObject, sizeof(tIPIfObject), &MaxInstNum );
						has_new = 1;

						// hook required data
						tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, p->ConDevInstNum + 1 );
						if(tmp_entity)
						{
							tmp_entity->obj_data= malloc(sizeof(struct IPIfEntityData));
							if(tmp_entity->obj_data == NULL)
							{
								fprintf(stderr, "<%s:%d> malloc failed!\n", __FUNCTION__, __LINE__);
								return -1;
							}
							memset(tmp_entity->obj_data, 0, sizeof(struct IPIfEntityData));
							tmp_entity->free_obj_data = free_ip_if_obj_data;

							tmp_entity->next[eIPIfEntityIPv4Address].next = tWANIPv4AddrObject;
						}
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

int getIPv4AddrEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char ifname[IFNAMSIZ] = {0};
	int if_num = 0, addr_num = 0;
	MIB_CE_ATM_VC_T vc_entity = {0};
	unsigned char is_br0 = 0;
	int chainidx = -1;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if_num = getInstNum(name, "Interface");
	if(if_num < 1)
		return ERR_9005;

	if(get_ip_if_info(if_num, &vc_entity, &chainidx, ifname) < 0)
		return ERR_9005;

	if(strcmp(ifname, "br0") == 0)
		is_br0 = 1;

	addr_num = getInstNum(name, "IPv4Address");	
#ifdef CONFIG_SECONDARY_IP
	if(!is_br0 && addr_num != 1)
		return ERR_9005;
	else if (is_br0 && addr_num < 1 && addr_num > 2)
		return ERR_9005;
#else
	if(addr_num != 1)
		return ERR_9005;
#endif

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef CONFIG_SECONDARY_IP
		unsigned char enabled = 0;

		if(is_br0 && addr_num == 2)
		{
			mib_get(MIB_ADSL_LAN_ENABLE_IP2, (void *)&enabled);
			*data = booldup(enabled);
		}
		else
#endif
			*data = booldup(1);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		*data = strdup( "Enabled" );
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		*data = strdup("");
	}*/
	else if( strcmp( lastname, "IPAddress" )==0 )
	{
		char buf[INET_ADDRSTRLEN] = {0};
		struct in_addr inAddr = {0};

		if(is_br0)
		{
			//LAN
#ifdef CONFIG_SECONDARY_IP
		    if(addr_num == 2)
				getMIB2Str(MIB_ADSL_LAN_IP2, buf);
		    else
#endif
				getMIB2Str(MIB_ADSL_LAN_IP, buf);
		}
		else if (getInAddr( ifname, IP_ADDR, (void *)&inAddr) == 1)
			inet_ntop(AF_INET, (void *)&inAddr, buf, INET_ADDRSTRLEN);

		*data=strdup(buf);
	}
	else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		char buf[INET_ADDRSTRLEN] = {0};
		struct in_addr inAddr = {0};

		if(is_br0)
		{
#ifdef CONFIG_SECONDARY_IP
		if(addr_num == 2)
			getMIB2Str(MIB_ADSL_LAN_SUBNET2, buf);
	    else
#endif
			getMIB2Str(MIB_ADSL_LAN_SUBNET, buf);
			
		}
		else if (getInAddr( ifname, SUBNET_MASK, (void *)&inAddr) == 1)
			inet_ntop(AF_INET, (void *)&inAddr, buf, INET_ADDRSTRLEN);

		*data=strdup( buf );
	}
	else if( strcmp( lastname, "AddressingType" )==0 )
	{
		if(is_br0 || vc_entity.ipDhcp == (unsigned char)DHCP_DISABLED)
			*data = strdup( "Static" );
		else
			*data = strdup( "DHCP" );
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


int setIPv4AddrEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf=data;
	int if_num, addr_num;
	char ifname[IFNAMSIZ] = {0};
	MIB_CE_ATM_VC_T vc_entity = {0};
	int chainidx = -1;
	unsigned char is_br0 = 0;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	if_num = getInstNum(name, "Interface");
	if(if_num < 1)
		return ERR_9005;
	
	if(get_ip_if_info(if_num, &vc_entity, &chainidx, ifname) < 0)
		return ERR_9005;

	if(strcmp(ifname, "br0") == 0)
		is_br0 = 1;

	addr_num = getInstNum(name, "IPv4Address"); 
#ifdef CONFIG_SECONDARY_IP
	if(!is_br0 && addr_num != 1)
		return ERR_9005;
	else if (is_br0 && addr_num < 1 && addr_num > 2)
		return ERR_9005;
#else
	if(addr_num != 1)
		return ERR_9005;
#endif

	gWanBitMap |= (1U << chainidx);

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

#ifdef CONFIG_SECONDARY_IP
		if(is_br0 && addr_num == 2)
		{
			unsigned char enable = (*i==0) ? 0 : 1;
			mib_set(MIB_ADSL_LAN_ENABLE_IP2, (void *)&enable);

			apply_add( CWMP_PRI_N, apply_LANIP, CWMP_RESTART, 0, NULL, 0 );
			return CWMP_APPLIED;
		}
		else
#endif
		if(*i!=1)
			return ERR_9001;
		return CWMP_APPLIED;
	}
	else if( strcmp( lastname, "IPAddress" )==0 )
	{
		char	*buf=data;

		if( strlen(buf)==0 ) return ERR_9007;

		if(is_br0)
		{
			struct in_addr in = {0};

			if(  inet_aton( buf, &in )==0  ) return ERR_9007;

			if(addr_num==1)
			{
			    /*lan ip & dhcp gateway setting should be set independently*/
				struct in_addr dhcp_gw, origIp;
				mib_get(MIB_ADSL_LAN_IP, (void *)&origIp);
				mib_get(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&dhcp_gw);
				if(dhcp_gw.s_addr==origIp.s_addr)
					mib_set(MIB_ADSL_LAN_DHCP_GATEWAY, (void *)&in);
				mib_set(MIB_ADSL_LAN_IP, (void *)&in);
			}
#ifdef CONFIG_SECONDARY_IP
			else if(addr_num==2)
				mib_set(MIB_ADSL_LAN_IP2, (void *)&in);
#endif

			apply_add( CWMP_PRI_N, apply_LANIP, CWMP_RESTART, 0, NULL, 0 );
			return CWMP_APPLIED;
		}
		else if( vc_entity.ipDhcp == (char) DHCP_DISABLED )
		{
			if ( !inet_aton(buf, (struct in_addr *)vc_entity.ipAddr) )
				return ERR_9007;

			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, chainidx );
			return CWMP_NEED_RESTART_WAN;
		}
		else
			return ERR_9001;
	}
	else if( strcmp( lastname, "SubnetMask" )==0 )
	{
		if( buf==NULL ) return ERR_9007;
		if( strlen(buf)==0 ) return ERR_9007;

		if(is_br0)
		{
			struct in_addr in = {0};

			if( inet_aton( buf, &in )==0 ) //the ip address is error.
				return ERR_9007;
#ifdef CONFIG_SECONDARY_IP
		    if(addr_num==2)
				mib_set(MIB_ADSL_LAN_SUBNET2, (void *)&in);
		    else
#endif
				mib_set(MIB_ADSL_LAN_SUBNET, (void *)&in);

			apply_add( CWMP_PRI_N, apply_LANIP, CWMP_RESTART, 0, NULL, 0 );
			return CWMP_APPLIED;
		}
		else
		{
			if( vc_entity.ipDhcp == (char) DHCP_DISABLED )
			{
				if ( !inet_aton(buf, (struct in_addr *)vc_entity.netMask) )
					return ERR_9007;
			}
			else
				return ERR_9001;
			
			mib_chain_update( MIB_ATM_VC_TBL, (unsigned char*)&vc_entity, chainidx );
			return CWMP_NEED_RESTART_WAN;
		}
	}
	else
	{
		return ERR_9005;
	}

	return CWMP_APPLIED;
}

#ifdef CONFIG_IPV6
int getIPv6AddrEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	char ifname[IFNAMSIZ] = {0};
	int if_num = 0, addr_num = 0;
	MIB_CE_ATM_VC_T vc_entity = {0};
	unsigned char is_br0 = 0;
	int total_addr = 0;
	struct ip6_info info[6] = {0};
	int chainidx= -1;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if_num = getInstNum(name, "Interface");
	if(if_num < 1)
		return ERR_9005;

	if(get_ip_if_info(if_num, &vc_entity, &chainidx, ifname) < 0)
		return ERR_9005;

	if(strcmp(ifname, "br0") == 0)
		is_br0 = 1;

	total_addr = get_ip6_info(ifname, info, 6);
	addr_num = getInstNum(name, "IPv6Address");
	if(addr_num < 1 || addr_num > total_addr)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		*data = strdup( "Enabled" );
	}
	else if( strcmp( lastname, "IPAddressStatus" )==0 )
	{
		char buf[256] = {0};
		int id = addr_num - 1;

		//Concepts from net-snmp
		if(info[id].flags & IFA_F_PERMANENT /*RA or Static*/
		|| (!info[id].flags) /*DHCPv6*/)
			*data = strdup("Preferred");
		else if(info[id].flags & IFA_F_TENTATIVE)
			*data = strdup("Tentative");
		else if(info[id].flags & IFA_F_DEPRECATED)
			*data = strdup("Deprecated");
		else
			*data = strdup("Unknown");
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		*data = strdup("");
	}*/
	else if( strcmp( lastname, "IPAddress" )==0 )
	{
		char buf[INET6_ADDRSTRLEN] = {0};
		
		inet_ntop(PF_INET6, info[addr_num - 1].addr, buf, INET6_ADDRSTRLEN);
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "Origin" )==0 )
	{
		int id = addr_num-1;

		if(info[id].scope & IPV6_ADDR_LINKLOCAL || info[id].scope & IPV6_ADDR_MULTICAST
			|| info[id].scope & IPV6_ADDR_LOOPBACK || info[id].scope & IPV6_ADDR_COMPATv4)
			*data = strdup( "WellKnown" );
		else if(is_br0)
			*data = strdup( "AutoConfigured" );
		else
		{	
			if((vc_entity.AddrMode & 2)
			&& memcmp(vc_entity.Ipv6Addr, info[id].addr, IP6_ADDR_LEN) == 0)
				*data = strdup( "Static" );
			else if(vc_entity.Ipv6Dhcp & 1 && info[id].flags & IFA_F_PERMANENT)
				*data = strdup( "DHCPv6" );
			else if(vc_entity.Ipv6Dhcp & 1 && !(info[id].flags & IFA_F_PERMANENT))
				*data = strdup( "AutoConfigured" );	//ULA or SLAAC
			else
			{
				CWMPDBG(1, (stderr, "<%s:%d> Address origin cannot be identified\n", __FUNCTION__, __LINE__));
				*data = strdup( "WellKnown" );	//should not go to here
			}
		}
	}
	else if( strcmp( lastname, "Prefix" )==0 )
	{
		int id = addr_num-1;
		char buf[1024] = {0};

		if(info[id].scope & IPV6_ADDR_LINKLOCAL || info[id].scope & IPV6_ADDR_MULTICAST
			|| info[id].scope & IPV6_ADDR_LOOPBACK || info[id].scope & IPV6_ADDR_COMPATv4)
			*data = strdup( "" );

		else
		{
			snprintf(buf, 1024, "Device.IP.Interface.%d.IPv6Prefix.%d", if_num, addr_num);
			*data = strdup(buf);
		}
	}
	else if( strcmp( lastname, "PreferredLifetime" )==0 )
	{
		*data = timedup(info[addr_num - 1].prefered);
	}
	else if( strcmp( lastname, "ValidLifetime" )==0 )
	{
		*data = timedup(info[addr_num - 1].valid);
	}
	else if( strcmp( lastname, "Anycast" )==0 )
	{
		*data = booldup(info[addr_num - 1].anycast);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objIPv6Addr(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_ATM_VC_T vc_entity = {0};
	int if_num = getInstNum(name, "Interface");
	char ifname[IFNAMSIZ] = {0};
	int chainidx = -1;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	if(if_num < 1)
		return ERR_9005;
	
	get_ip_if_info(if_num, &vc_entity, &chainidx, ifname);
	
	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
			unsigned int num=0,MaxInstNum=0,i;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			MaxInstNum = get_ipv6_addr_cnt(ifname);

			if(MaxInstNum > 0)
			{
				if( create_Object( c, tIPv6AddrObject, sizeof(tIPv6AddrObject), MaxInstNum, 1 ) < 0 )
					return -1;

				add_objectNum(name, MaxInstNum);
			}
			return 0;
		}
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
		{
			int total,i;
			struct CWMP_LINKNODE *old_table;
			int has_new=0;
			struct CWMP_LINKNODE *tmp_entity=NULL;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			total = get_ipv6_addr_cnt(ifname);
		
			for( i = 1 ; i <= total ; i++ )
			{
				tmp_entity = remove_SiblingEntity( &old_table, i );
				if( tmp_entity!=NULL )
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, tmp_entity );
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, i )==NULL )
					{
						unsigned int MaxInstNum = i;

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tIPv6AddrObject, sizeof(tIPv6AddrObject), &MaxInstNum );
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

int getIPv6PrefixEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	char ifname[IFNAMSIZ] = {0};
	int if_num = 0, prefix_num = 0;
	MIB_CE_ATM_VC_T vc_entity = {0};
	unsigned char is_br0 = 0;
	int total_addr = 0;
	struct ip6_info info[6] = {0};
	int chainidx = -1;
	int id = 0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if_num = getInstNum(name, "Interface");
	if(if_num < 1)
		return ERR_9005;

	if(get_ip_if_info(if_num, &vc_entity, &chainidx, ifname) < 0)
		return ERR_9005;

	if(strcmp(ifname, "br0") == 0)
		is_br0 = 1;

	total_addr = get_ip6_info(ifname, info, 6);
	prefix_num = getInstNum(name, "IPv6Prefix");
	id = prefix_num - 1;

	if(prefix_num < 1 || prefix_num > total_addr)
		return ERR_9005;

	if(info[id].scope & IPV6_ADDR_LINKLOCAL || info[id].scope & IPV6_ADDR_MULTICAST
		|| info[id].scope & IPV6_ADDR_LOOPBACK || info[id].scope & IPV6_ADDR_COMPATv4)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		*data = strdup( "Enabled" );
	}
	else if( strcmp( lastname, "PrefixStatus" )==0 )
	{
		*data = strdup( "Preferred " );
	}
	else if( strcmp( lastname, "Prefix" )==0 )
	{
		char buf[INET6_ADDRSTRLEN] = {0};
		char prefix[IP6_ADDR_LEN]= {0};

		if(ip6toPrefix(info[id].addr, info[id].prefix_len, prefix) < 0)
			return ERR_9002;
		
		inet_ntop(PF_INET6, prefix, buf, 256);
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "Origin" )==0 )
	{
		if(info[id].scope & IPV6_ADDR_LINKLOCAL || info[id].scope & IPV6_ADDR_MULTICAST
			|| info[id].scope & IPV6_ADDR_LOOPBACK || info[id].scope & IPV6_ADDR_COMPATv4)
			*data = strdup( "WellKnown" );
		else if(is_br0)
			*data = strdup( "AutoConfigured" );
		else
		{	
			if((vc_entity.AddrMode & 2)
			&& memcmp(vc_entity.Ipv6Addr, info[id].addr, IP6_ADDR_LEN) == 0)
				*data = strdup( "Static" );
			else if(vc_entity.Ipv6Dhcp & 1 && info[id].flags & IFA_F_PERMANENT)
				*data = strdup( "DHCPv6" );
			else if(vc_entity.Ipv6Dhcp & 1 && !(info[id].flags & IFA_F_PERMANENT))
				*data = strdup( "RouterAdvertisement" );	//ULA or SLAAC
			else
			{
				CWMPDBG(1, (stderr, "<%s:%d> Address origin cannot be identified\n", __FUNCTION__, __LINE__));
				*data = strdup( "WellKnown" );	//should not go to here
			}
		}
	}
	else if( strcmp( lastname, "OnLink" )==0 )
	{
		//TODO
		*data = booldup(1);
	}
	else if( strcmp( lastname, "Autonomous" )==0 )
	{
		//TODO
		*data = booldup(1);
	}
	else if( strcmp( lastname, "ValidLifetime" )==0 )
	{
		*data = timedup(info[id].valid);
	}
	else if( strcmp( lastname, "Anycast" )==0 )
	{
		*data = booldup(info[id].anycast);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objIPv6Prefix(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	MIB_CE_ATM_VC_T vc_entity = {0};
	int chainidx = -1;
	int if_num = getInstNum(name, "Interface");
	char ifname[IFNAMSIZ] = {0};
	struct ip6_info info[6] = {0};
	int total_addr;
	int id;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	if(if_num < 1)
		return ERR_9005;
	
	get_ip_if_info(if_num, &vc_entity, &chainidx, ifname);
	total_addr = get_ipv6_addr_cnt(ifname);
	
	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
			unsigned int num=0,MaxInstNum=0,i;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

			MaxInstNum = total_addr;
/*			for(id = 0 ; id < total_addr; id++)
			{
				if(info[id].scope & IPV6_ADDR_LINKLOCAL || info[id].scope & IPV6_ADDR_MULTICAST
					|| info[id].scope & IPV6_ADDR_LOOPBACK || info[id].scope & IPV6_ADDR_COMPATv4)
				{
					MaxInstNum--;
				}
			}*/

			if(MaxInstNum > 0)
			{
				if( create_Object( c, tIPv6PrefixObject, sizeof(tIPv6PrefixObject), MaxInstNum, 1 ) < 0 )
					return -1;

				add_objectNum(name, MaxInstNum);
			}
			return 0;
		}
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
		{
			int total,i;
			struct CWMP_LINKNODE *old_table;
			int has_new=0;
			struct CWMP_LINKNODE *tmp_entity=NULL;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for(id = 0 ; id < total_addr; id++)
			{
				/*if(info[id].scope & IPV6_ADDR_LINKLOCAL || info[id].scope & IPV6_ADDR_MULTICAST
					|| info[id].scope & IPV6_ADDR_LOOPBACK || info[id].scope & IPV6_ADDR_COMPATv4)
					continue;*/

				tmp_entity = remove_SiblingEntity( &old_table, id+1 );
				if( tmp_entity!=NULL )
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, tmp_entity );
				else
				{
					if( find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, id+1 )==NULL )
					{
						unsigned int MaxInstNum = id+1;

						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, (struct CWMP_LINKNODE *)tIPv6PrefixObject, sizeof(tIPv6PrefixObject), &MaxInstNum );
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

#endif //CONFIG_IPV6

int getIPIfStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	ifname[16];
	struct net_device_stats nds = {0};
	int if_num;
	MIB_CE_ATM_VC_T vc_entity = {0};
	int chainidx = -1;
	unsigned char is_br0 = 0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if_num = getInstNum(name, "Interface");
	if(if_num < 1)
		return ERR_9005;

	if(get_ip_if_info(if_num, &vc_entity, &chainidx, ifname) < 0)
		return ERR_9005;

	if(strcmp(ifname, "br0") == 0)
		is_br0 = 1;

	get_net_device_stats(ifname, &nds);

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "BytesSent" )==0 )
	{
		*data = (void*)ulongdup(nds.tx_bytes);
	}
	else if( strcmp( lastname, "BytesReceived" )==0 )
	{
		*data = (void*)ulongdup(nds.rx_bytes);
	}
	else if( strcmp( lastname, "PacketsSent" )==0 )
	{
		*data = (void*)ulongdup(nds.tx_packets);
	}
	else if( strcmp( lastname, "PacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(nds.rx_packets);
	}
	else if( strcmp( lastname, "ErrorsSent" )==0 )
	{
		*data = uintdup(nds.tx_errors);
	}
	else if( strcmp( lastname, "ErrorsReceived" )==0 )
	{
		*data = uintdup(nds.rx_errors);
	}
	else if( strcmp( lastname, "UnicastPacketsSent" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "UnicastPacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "DiscardPacketsSent" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "DiscardPacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "MulticastPacketsSent" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "MulticastPacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "BroadcastPacketsSent" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "BroadcastPacketsReceived" )==0 )
	{
		*data = (void*)ulongdup(0);
	}
	else if( strcmp( lastname, "UnknownProtoPacketsReceived" )==0 )
	{
		*data = uintdup(0);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


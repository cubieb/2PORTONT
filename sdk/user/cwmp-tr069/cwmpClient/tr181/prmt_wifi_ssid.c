#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/ethtool.h>

#include <rtk/mib.h>
#include <rtk/utility.h>
#include <parameter_api.h>
#include <libcwmp.h>

#include "../prmt_apply.h"
#include "../cwmpc_utility.h"
#include "prmt_wifi_ssid.h"

/****** Device.WiFi.SSID.{i}.Stats *******************************************/
struct CWMP_OP tWiFiSSIDStatsLeafOP = { getWiFiSSIDStats, NULL};

struct CWMP_PRMT tWiFiSSIDStatsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"BytesSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiSSIDStatsLeafOP},
{"BytesReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiSSIDStatsLeafOP},
{"PacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiSSIDStatsLeafOP},
{"PacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiSSIDStatsLeafOP},
{"ErrorsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiSSIDStatsLeafOP},
{"ErrorsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiSSIDStatsLeafOP},
};

enum eWiFiSSIDStatsLeaf
{
	eStatsBytesSent,
	eStatsBytesReceived,
	eStatsPacketsSent,
	eStatsPacketsReceived,
	eStatsErrorsSent,
	eStatsErrorsReceived,
};

struct CWMP_LEAF tWiFiSSIDStatsLeaf[] =
{
{ &tWiFiSSIDStatsLeafInfo[eStatsBytesSent]  },
{ &tWiFiSSIDStatsLeafInfo[eStatsBytesReceived]  },
{ &tWiFiSSIDStatsLeafInfo[eStatsPacketsSent]  },
{ &tWiFiSSIDStatsLeafInfo[eStatsPacketsReceived]  },
{ &tWiFiSSIDStatsLeafInfo[eStatsErrorsSent]  },
{ &tWiFiSSIDStatsLeafInfo[eStatsErrorsReceived]  },
{ NULL	}
};

/****** Device.WiFi.SSID.{i} *************************************************/
struct CWMP_OP tWiFiSSIDEntityLeafOP = { getWiFiSSIDEntity, setWiFiSSIDEntity};

struct CWMP_PRMT tWiFiSSIDEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWiFiSSIDEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,	&tWiFiSSIDEntityLeafOP},
//{"Alias",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiSSIDEntityLeafOP},
{"Name", 		eCWMP_tSTRING,	CWMP_READ,	&tWiFiSSIDEntityLeafOP},
{"LowerLayers",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiSSIDEntityLeafOP},
{"BSSID",		eCWMP_tSTRING,	CWMP_READ,	&tWiFiSSIDEntityLeafOP},
{"MACAddress",	eCWMP_tSTRING,	CWMP_READ,	&tWiFiSSIDEntityLeafOP},
{"SSID",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiSSIDEntityLeafOP},
};

enum eWiFiSSIDEntityLeaf
{
	eSSIDEnable,
	eSSIDStatus,
//	eSSIDAlias,
	eSSIDName,
	eSSIDLowerLayers,
	eSSIDBSSID,
	eSSIDMACAddress,
	eSSIDSSID,
};

struct CWMP_LEAF tWiFiSSIDEntityLeaf[] =
{
{ &tWiFiSSIDEntityLeafInfo[eSSIDEnable]  },
{ &tWiFiSSIDEntityLeafInfo[eSSIDStatus]  },
//{ &tWiFiSSIDEntityLeafInfo[eSSIDAlias]  },
{ &tWiFiSSIDEntityLeafInfo[eSSIDName]  },
{ &tWiFiSSIDEntityLeafInfo[eSSIDLowerLayers]  },
{ &tWiFiSSIDEntityLeafInfo[eSSIDBSSID]  },
{ &tWiFiSSIDEntityLeafInfo[eSSIDMACAddress]	},
{ &tWiFiSSIDEntityLeafInfo[eSSIDSSID]  },
{ NULL	}
};

struct CWMP_PRMT tWiFiSSIDEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Stats",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eWiFiSSIDEntityObject
{
	eSSIDStats,
};

struct CWMP_NODE tWiFiSSIDEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tWiFiSSIDEntityObjectInfo[eSSIDStats],	tWiFiSSIDStatsLeaf, NULL},
{NULL,					NULL,			NULL}
};

/****** Device.WiFi.SSID ****************************************************/
struct CWMP_PRMT tWiFiSSIDObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"1",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
#ifdef WLAN_MBSSID
{"2",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"3",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"4",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"5",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
#endif
};

enum eWiFiSSIDObject
{
	eWiFiSSID1,
#ifdef WLAN_MBSSID
	eWiFiSSID2,
	eWiFiSSID3,
	eWiFiSSID4,
	eWiFiSSID5,
#endif
};

struct CWMP_NODE tWiFiSSIDObject[] =
{
/*info, 						leaf,				next)*/
{&tWiFiSSIDObjectInfo[eWiFiSSID1], tWiFiSSIDEntityLeaf, tWiFiSSIDEntityObject},
#ifdef WLAN_MBSSID
{&tWiFiSSIDObjectInfo[eWiFiSSID2], tWiFiSSIDEntityLeaf, tWiFiSSIDEntityObject},
{&tWiFiSSIDObjectInfo[eWiFiSSID3], tWiFiSSIDEntityLeaf, tWiFiSSIDEntityObject},
{&tWiFiSSIDObjectInfo[eWiFiSSID4], tWiFiSSIDEntityLeaf, tWiFiSSIDEntityObject},
{&tWiFiSSIDObjectInfo[eWiFiSSID5], tWiFiSSIDEntityLeaf, tWiFiSSIDEntityObject},
#endif
{NULL, NULL, NULL}
};

/***** Operations ************************************************************/
int getWiFiSSIDEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char *ifname = NULL;
	unsigned char vChar;
	unsigned int ssidnum = 0;
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_T Entry;
#endif

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	ssidnum = getInstNum(name, "SSID");
	
#ifdef WLAN_MBSSID
	if( ssidnum < 1 || ssidnum > WLAN_IF_NUM)	return ERR_9005;
	if (!mib_chain_get(MIB_MBSSIB_TBL, ssidnum-1, (void *)&Entry)) return ERR_9002;
#else
	if( ssidnum != 1 ) return ERR_9005;
#endif

	ifname = wlan_name[ssidnum-1];

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
#ifdef WLAN_MBSSID
		if(ssidnum != 1)
			vChar = Entry.wlanDisabled;
		else
#endif
			mib_get( MIB_WLAN_DISABLED, (void *)&vChar);

		*data = booldup( (vChar==0) );
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int flags=0;
		if( getInFlags(ifname, &flags)==1 )
		{
			if (flags & IFF_UP)
				*data = strdup( "Up" );
			else
				*data = strdup( "Down" );
		}else
			*data = strdup( "Error" );
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		*data = strdup("");
	}*/
	else if( strcmp( lastname, "Name" )==0 )
	{
		*data = strdup(ifname);
	}
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		*data = strdup("Device.WiFi.Radio.1");
	}
	else if( strcmp( lastname, "BSSID" )==0 || strcmp( lastname, "MACAddress" )==0 )
	{
		char macadd[18] = {0};
		struct sockaddr sa;
		
		if(!getInAddr(ifname, HW_ADDR, (void *)&sa))
		{
			*data = strdup("");
		}
		else
		{
			sprintf(macadd, "%02x:%02x:%02x:%02x:%02x:%02x", sa.sa_data[0]&0xff, sa.sa_data[1]&0xff, sa.sa_data[2]&0xff, sa.sa_data[3]&0xff, sa.sa_data[4]&0xff, sa.sa_data[5]&0xff);
			*data = strdup(macadd);
		}
	}
	else if( strcmp( lastname, "SSID" )==0 )
	{
		char buf[32] = {0};

	     /*for root ap, ssid and authType is not used => use MIB_WLAN_SSID and MIB_WLAN_AUTH_TYPE*/
#ifdef WLAN_MBSSID
	    if(ssidnum != 1)
			strcpy( buf, Entry.ssid );
	    else
#endif
			getMIB2Str(MIB_WLAN_SSID, buf);

		*data = strdup( buf );
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setWiFiSSIDEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int ssidnum;
#ifdef WLAN_MBSSID
	MIB_CE_MBSSIB_T Entry;
#endif

	if( (name==NULL) || (entity==NULL)) return -1;

	if( entity->info->type!=type ) return ERR_9006;

	ssidnum = getInstNum(name, "SSID");

#ifdef WLAN_MBSSID
	if( ssidnum < 1 || ssidnum > WLAN_IF_NUM)	return ERR_9005;
	if (!mib_chain_get(MIB_MBSSIB_TBL, ssidnum-1, (void *)&Entry)) return ERR_9002;
#else
	if( ssidnum != 1 ) return ERR_9005;
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned char vChar;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;

#ifdef WLAN_MBSSID
		if( ssidnum != 1 )
		{
			Entry.wlanDisabled = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, ssidnum-1);
		}
		else
#endif
			mib_set(MIB_WLAN_DISABLED, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
/*	else if( strcmp( lastname, "Alias" )==0 )
	{
		return 0;
	}*/
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		return ERR_9001;
	}
	else if( strcmp( lastname, "SSID" )==0 )
	{
		char *ssid = data;

		//MIB_WLAN_SSID
		if( ssid==NULL ) return ERR_9007;
		if( (strlen(ssid)==0) || (strlen(ssid)>=MAX_SSID_LEN) ) return ERR_9007;
#ifdef WLAN_MBSSID
		if( ssidnum != 1 )
		{
			strcpy( Entry.ssid, ssid );
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, ssidnum-1);
		}
		else
#endif
			mib_set( MIB_WLAN_SSID, ssid );

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int getWiFiSSIDStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char	*ifname = NULL;
	struct net_device_stats nds = {0};
	int		iserror=0;
	unsigned int ssidnum;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	ssidnum = getInstNum(name, "SSID");

#ifdef WLAN_MBSSID
	if( ssidnum < 1 || ssidnum > WLAN_IF_NUM)	return ERR_9005;
#else
	if( ssidnum != 1 ) return ERR_9005;
#endif

	ifname = wlan_name[ssidnum - 1];

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
	else
	{
		return ERR_9005;
	}

	return 0;
}


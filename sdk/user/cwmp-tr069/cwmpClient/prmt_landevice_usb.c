#include "prmt_landevice_usb.h"
#ifdef _PRMT_X_CT_COM_VLAN_BOUND_
#include "prmt_ctcom_lanext.h"
#endif

#ifdef _PRMT_USB_ETH_
/***_PRMT_USB_ETH_*****************************************************************************************************************/
char *usb_name[]={"usb0"};
/************tLANUSBStatsLeaf*****************/
struct CWMP_OP tLANUSBStatsLeafOP = { getLANUSBStats, NULL };
struct CWMP_PRMT tLANUSBStatsLeafInfo[] =
{
/*(name,			type,		flag,				op)*/
{"BytesSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"BytesReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"CellsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"CellsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
#ifdef _PRMT_WT107_
{"ErrorsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"ErrorsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"UnicastPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"UnicastPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"DiscardPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"DiscardPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"MulticastPacketsSent",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"MulticastPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"BroadcastPacketsSent",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"BroadcastPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
#endif //_PRMT_WT107_
};
enum eLANUSBStatsLeaf
{
	eLANUSB_BytesSent,
	eLANUSB_BytesReceived,
	eLANUSB_CellsSent,
	eLANUSB_CellsReceived,
#ifdef _PRMT_WT107_
	eLANUSB_ErrorsSent,
	eLANUSB_ErrorsReceived,
	eLANUSB_UnicastPacketsSent,
	eLANUSB_UnicastPacketsReceived,
	eLANUSB_DiscardPacketsSent,
	eLANUSB_DiscardPacketsReceived,
	eLANUSB_MulticastPacketsSent,
	eLANUSB_MulticastPacketsReceived,
	eLANUSB_BroadcastPacketsSent,
	eLANUSB_BroadcastPacketsReceived,
	eLANUSB_UnknownProtoPacketsReceived,
#endif //_PRMT_WT107_
};
struct CWMP_LEAF tLANUSBStatsLeaf[] =
{
{ &tLANUSBStatsLeafInfo[eLANUSB_BytesSent] },
{ &tLANUSBStatsLeafInfo[eLANUSB_BytesReceived] },
{ &tLANUSBStatsLeafInfo[eLANUSB_CellsSent] },
{ &tLANUSBStatsLeafInfo[eLANUSB_CellsReceived] },
#ifdef _PRMT_WT107_
{ &tLANUSBStatsLeafInfo[eLANUSB_ErrorsSent] },
{ &tLANUSBStatsLeafInfo[eLANUSB_ErrorsReceived] },
{ &tLANUSBStatsLeafInfo[eLANUSB_UnicastPacketsSent] },
{ &tLANUSBStatsLeafInfo[eLANUSB_UnicastPacketsReceived] },
{ &tLANUSBStatsLeafInfo[eLANUSB_DiscardPacketsSent] },
{ &tLANUSBStatsLeafInfo[eLANUSB_DiscardPacketsReceived] },
{ &tLANUSBStatsLeafInfo[eLANUSB_MulticastPacketsSent] },
{ &tLANUSBStatsLeafInfo[eLANUSB_MulticastPacketsReceived] },
{ &tLANUSBStatsLeafInfo[eLANUSB_BroadcastPacketsSent] },
{ &tLANUSBStatsLeafInfo[eLANUSB_BroadcastPacketsReceived] },
{ &tLANUSBStatsLeafInfo[eLANUSB_UnknownProtoPacketsReceived] },
#endif //_PRMT_WT107_
{ NULL }
};
/************tLANUSBConfEntityLeaf*****************/
struct CWMP_OP tLANUSBConfEntityLeafOP = { getLANUSBConf, setLANUSBConf };
struct CWMP_PRMT tLANUSBConfEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tLANUSBConfEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tLANUSBConfEntityLeafOP},
#ifdef _PRMT_WT107_
{"Name",			eCWMP_tSTRING,	CWMP_READ,		&tLANUSBConfEntityLeafOP},
#endif //_PRMT_WT107_
{"MACAddress",			eCWMP_tSTRING,	CWMP_READ,		&tLANUSBConfEntityLeafOP},
#ifdef MAC_FILTER
{"MACAddressControlEnabled",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tLANUSBConfEntityLeafOP},
#endif /*MAC_FILTER*/
{"Standard",			eCWMP_tSTRING,	CWMP_READ,		&tLANUSBConfEntityLeafOP},
{"Type",			eCWMP_tSTRING,	CWMP_READ,		&tLANUSBConfEntityLeafOP},
{"Rate",			eCWMP_tSTRING,	CWMP_READ,		&tLANUSBConfEntityLeafOP},
{"Power",			eCWMP_tSTRING,	CWMP_READ,		&tLANUSBConfEntityLeafOP},
};
enum eLANUSBConfEntityLeaf
{
	eLANUSB_Enable,
	eLANUSB_Status,
#ifdef _PRMT_WT107_
	eLANUSB_Name,
#endif //_PRMT_WT107_
	eLANUSB_MACAddress,
#ifdef MAC_FILTER
	eLANUSB_MACAddressControlEnabled,
#endif /*MAC_FILTER*/
	eLANUSB_Standard,
	eLANUSB_Type,
	eLANUSB_Rate,
	eLANUSB_Power
};
struct CWMP_LEAF tLANUSBConfEntityLeaf[] =
{
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Enable] },
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Status] },
#ifdef _PRMT_WT107_
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Name] },
#endif //_PRMT_WT107_
{ &tLANUSBConfEntityLeafInfo[eLANUSB_MACAddress] },
#ifdef MAC_FILTER
{ &tLANUSBConfEntityLeafInfo[eLANUSB_MACAddressControlEnabled] },
#endif /*MAC_FILTER*/
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Standard] },
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Type] },
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Rate] },
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Power] },
#ifdef _PRMT_X_CT_COM_VLAN_BOUND_
{ &tCTVlanBoundingLeafInfo[eCTBOUNDING_Mode] },
{ &tCTVlanBoundingLeafInfo[eCTBOUNDING_VLAN] },
#endif
{ NULL }
};
/************tLANUSBConfEntityObject*****************/
struct CWMP_PRMT tLANUSBConfEntityObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,		NULL}
};
enum eLANUSBConfEntityObject
{
	eLANUSB_Stats
};
struct CWMP_NODE tLANUSBConfEntityObject[] =
{
/*info,  					leaf,				node)*/
{ &tLANUSBConfEntityObjectInfo[eLANUSB_Stats],	tLANUSBStatsLeaf,		NULL},
{ NULL,						NULL,				NULL}
};
/************tLANUSBConfObject*****************/
struct CWMP_PRMT tLANUSBConfObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"1",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
};
enum eLANUSBConfObject
{
	eLANUsb1,
};
struct CWMP_NODE tLANUSBConfObject[] =
{
/*info,  					leaf,				node)*/
{ &tLANUSBConfObjectInfo[eLANUsb1],		tLANUSBConfEntityLeaf,		tLANUSBConfEntityObject},
{ NULL,						NULL,				NULL}
};
/********************************************/
int getLANUSBStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	unsigned long bs=0,br=0,ps=0,pr=0;
#ifdef _PRMT_WT107_
	unsigned long es=0,er=0, ups=0,upr=0, dps=0,dpr=0, mps=0,mpr=0, bps=0, bpr=0, uppr=0;
#endif //_PRMT_WT107_
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "BytesSent" )==0 )
	{
		if( getInterfaceStat( usb_name[0], &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( bs );
	}else if( strcmp( lastname, "BytesReceived" )==0 )
	{
		if( getInterfaceStat( usb_name[0], &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( br );
	}else if( strcmp( lastname, "CellsSent" )==0 )
	{
		if( getInterfaceStat( usb_name[0], &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( ps );
	}else if( strcmp( lastname, "CellsReceived" )==0 )
	{
		if( getInterfaceStat( usb_name[0], &bs, &br, &ps, &pr ) < 0 )
			return -1;
		*data = uintdup( pr );
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "ErrorsSent" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( es );
	}else if( strcmp( lastname, "ErrorsReceived" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( er );
	}else if( strcmp( lastname, "UnicastPacketsSent" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( ups );
	}else if( strcmp( lastname, "UnicastPacketsReceived" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( upr );
	}else if( strcmp( lastname, "DiscardPacketsSent" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( dps );
	}else if( strcmp( lastname, "DiscardPacketsReceived" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( dpr );
	}else if( strcmp( lastname, "MulticastPacketsSent" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( mps );
	}else if( strcmp( lastname, "MulticastPacketsReceived" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( mpr );
	}else if( strcmp( lastname, "BroadcastPacketsSent" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( bps );
	}else if( strcmp( lastname, "BroadcastPacketsReceived" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( bpr );
	}else if( strcmp( lastname, "UnknownProtoPacketsReceived" )==0 )
	{
		if( getInterfaceStat1( usb_name[0], &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0 )
			return -1;
		*data = uintdup( uppr );
#endif //_PRMT_WT107_
	}else{
		return ERR_9005;
	}

	return 0;
}
int getLANUSBConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		//mib_get(CWMP_LAN_ETHIFENABLE, (void *)&vChar);
		*data = booldup( 1 );
	}else if( strcmp( lastname, "Status" )==0 )
	{
		int flags=0;
		//how to detect "NoLink" condition
		if( getInFlags(usb_name[0], &flags)==1 )
		{
			if (flags & IFF_UP)
			{
				if( getUSBLANStatus()==1 )
					*data = strdup( "NoLink" );
				else
				*data = strdup( "Up" );
			}else
				*data = strdup( "Disabled" );
		}else
			*data = strdup( "Error" );
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Name" )==0 )
	{
		*data = strdup( usb_name[0] );
#endif //_PRMT_WT107_
	}else if( strcmp( lastname, "MACAddress" )==0 )
	{
		unsigned char buffer[64];
		unsigned char macadd[MAC_ADDR_LEN];
		//getMIB2Str(MIB_ELAN_MAC_ADDR, buffer);
		//mib_get(MIB_ELAN_MAC_ADDR, (void *)macadd);
		getUSBLANMacAddr( macadd );
		sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0], macadd[1],
			macadd[2], macadd[3], macadd[4], macadd[5]);
		*data=strdup(buffer);
#ifdef MAC_FILTER
	}else if( strcmp( lastname, "MACAddressControlEnabled" )==0 )
	{
		//mib_get(MIB_ETH_MAC_CTRL, (void *)&vChar);
		*data = booldup( vChar!=0 );
#endif /*MAC_FILTER*/
	}else if( strcmp( lastname, "Standard" )==0 )
	{
		*data = strdup("USB2.0");
	}else if( strcmp( lastname, "Type" )==0 )
	{
		*data = strdup("Device");
	}else if( strcmp( lastname, "Rate" )==0 )
	{
		switch( getUSBLANRate() )
		{
		default:
		case 0:
			*data = strdup("High");
			break;
		case 1:
			*data = strdup("Full");
			break;
		case 2:
			*data = strdup("Low");
			break;
		}
	}else if( strcmp( lastname, "Power" )==0 )
	{
		*data = strdup("Self");
	}else{
		return ERR_9005;
	}

	return 0;
}
int setLANUSBConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		fprintf(stderr, "set %s=%d\n", name, vChar );
		//mib_set(CWMP_LAN_ETHIFENABLE, (void *)&vChar);
		//apply_add( CWMP_PRI_N, apply_ETHER, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#ifdef MAC_FILTER
	}else if( strcmp( lastname, "MACAddressControlEnabled" )==0 )
	{
		int *i = data;
		if( i==NULL ) return ERR_9007;
		vChar = (*i!=0)?1:0;
		fprintf(stderr, "set %s=%d\n", name, vChar );
		//mib_set(MIB_ETH_MAC_CTRL, (void *)&vChar);
		//{
		//	unsigned char wlan_mac_ctrl=0,mac_out_dft=1;
		//	mib_get(MIB_WLAN_MAC_CTRL, (void *)&wlan_mac_ctrl);
		//	if( vChar==1 || wlan_mac_ctrl==1 )
		//		mac_out_dft=0;//0:deny, 1:allow
		//	mib_set(MIB_MACF_OUT_ACTION, (void *)&mac_out_dft);
		//}

		//apply_add( CWMP_PRI_N, apply_MACFILTER, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#endif /*MAC_FILTER*/
	}else{
		return ERR_9005;
	}

	return 0;
}
/***end _PRMT_USB_ETH_*****************************************************************************************************************/
#elif defined(_PRMT_USB_)
/***_PRMT_USB_*****************************************************************************************************************/
unsigned int getUSBIFInstNum( char *name );
struct CWMP_OP tLANUSBStatsLeafOP = { getLANUSBStats, NULL };
struct CWMP_PRMT tLANUSBStatsLeafInfo[] =
{
/*(name,			type,		flag,				op)*/
{"BytesSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"BytesReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"CellsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
{"CellsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANUSBStatsLeafOP},
};
enum eLANUSBStatsLeaf
{
	eLANUSB_BytesSent,
	eLANUSB_BytesReceived,
	eLANUSB_CellsSent,
	eLANUSB_CellsReceived
};
struct CWMP_LEAF tLANUSBStatsLeaf[] =
{
{ &tLANUSBStatsLeafInfo[eLANUSB_BytesSent] },
{ &tLANUSBStatsLeafInfo[eLANUSB_BytesReceived] },
{ &tLANUSBStatsLeafInfo[eLANUSB_CellsSent] },
{ &tLANUSBStatsLeafInfo[eLANUSB_CellsReceived] },
{ NULL }
};
struct CWMP_OP tLANUSBConfEntityLeafOP = { getLANUSBConf, setLANUSBConf };
struct CWMP_PRMT tLANUSBConfEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tLANUSBConfEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tLANUSBConfEntityLeafOP},
{"MACAddress",			eCWMP_tSTRING,	CWMP_READ,		&tLANUSBConfEntityLeafOP},
{"Type",			eCWMP_tSTRING,	CWMP_READ,	&tLANUSBConfEntityLeafOP},
{"Rate",			eCWMP_tSTRING,	CWMP_READ,	&tLANUSBConfEntityLeafOP},
{"Power",			eCWMP_tSTRING,	CWMP_READ,	&tLANUSBConfEntityLeafOP},
};
enum eLANUSBConfEntityLeaf
{
	eLANUSB_Enable,
	eLANUSB_Status,
	eLANUSB_MACAddress,
	eLANUSB_Type,
	eLANUSB_Rate,
	eLANUSB_Power
};
struct CWMP_LEAF tLANUSBConfEntityLeaf[] =
{
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Enable] },
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Status] },
{ &tLANUSBConfEntityLeafInfo[eLANUSB_MACAddress] },
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Type] },
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Rate] },
{ &tLANUSBConfEntityLeafInfo[eLANUSB_Power] },
{ NULL }
};
struct CWMP_PRMT tLANUSBConfEntityObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,		NULL}
};
enum eLANUSBConfEntityObject
{
	eLANUSB_Stats
};
struct CWMP_NODE tLANUSBConfEntityObject[] =
{
/*info,  					leaf,				node)*/
{ &tLANUSBConfEntityObjectInfo[eLANUSB_Stats],	tLANUSBStatsLeaf,		NULL},
{ NULL,						NULL,				NULL}
};


struct CWMP_PRMT tLANUSBConfObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"1",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
//{"2",				eCWMP_tOBJECT,	CWMP_READ,		NULL}
};
enum eLANUSBConfObject
{
	eLANUsb1,
//	eLANUsb2
};
struct CWMP_NODE tLANUSBConfObject[] =
{
/*info,  					leaf,				node)*/
{ &tLANUSBConfObjectInfo[eLANUsb1],		tLANUSBConfEntityLeaf,		tLANUSBConfEntityObject},
//{ &tLANUSBConfObjectInfo[eLANUsb2],		tLANUSBConfEntityLeaf,		tLANUSBConfEntityObject},
{ NULL,						NULL,				NULL}
};

int getLANUSBStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	unsigned long bs=0,br=0,ps=0,pr=0;
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "BytesSent" )==0 )
	{
		//if( getInterfaceStat( "eth0", &bs, &br, &ps, &pr ) < 0 )
		//	return -1;
		*data = uintdup( bs );
	}else if( strcmp( lastname, "BytesReceived" )==0 )
	{
		//if( getInterfaceStat( "eth0", &bs, &br, &ps, &pr ) < 0 )
		//	return -1;
		*data = uintdup( br );
	}else if( strcmp( lastname, "CellsSent" )==0 )
	{
		//if( getInterfaceStat( "eth0", &bs, &br, &ps, &pr ) < 0 )
		//	return -1;
		*data = uintdup( ps );
	}else if( strcmp( lastname, "CellsReceived" )==0 )
	{
		//if( getInterfaceStat( "eth0", &bs, &br, &ps, &pr ) < 0 )
		//	return -1;
		*data = uintdup( pr );
	}else{
		return ERR_9005;
	}

	return 0;
}

int getLANUSBConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{

		//mib_get(CWMP_LAN_ETHIFENABLE, (void *)&vChar);
		//*data = booldup( vChar!=0 );
		*data = booldup( 1 );
	}else if( strcmp( lastname, "Status" )==0 )
	{
		if (isUSBMounted > 0)
			*data = strdup("Up");
		else
			*data = strdup("NoLink");
	}else if( strcmp( lastname, "MACAddress" )==0 )
	{
	/*
		unsigned char buffer[64];
		unsigned char macadd[MAC_ADDR_LEN];
		//getMIB2Str(MIB_ELAN_MAC_ADDR, buffer);
		mib_get(MIB_ELAN_MAC_ADDR, (void *)macadd);
		sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0], macadd[1],
			macadd[2], macadd[3], macadd[4], macadd[5]);
		*data=strdup(buffer);
		*/
	}else if( strcmp( lastname, "Type" )==0 )
	{
		*data = strdup("Host");
	}else if( strcmp( lastname, "Rate" )==0 )
	{
		*data = strdup("High");
	}else if( strcmp( lastname, "Power" )==0 )
	{
		*data = strdup("Self");
	}else{
		return ERR_9005;
	}

	return 0;
}

int setLANUSBConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
//		mib_set(CWMP_LAN_ETHIFENABLE, (void *)&vChar);

	}else{
		return ERR_9005;
	}

	return 0;
}

unsigned int getUSBIFInstNum( char *name )
{
	return getInstNum( name, "LANUSBInterfaceConfig" );
}
/***end _PRMT_USB_*****************************************************************************************************************/
#endif

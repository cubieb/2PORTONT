#include <rtk/options.h>
#ifdef _PRMT_X_CT_COM_VLAN_BOUND_
#include "prmt_ctcom_lanext.h"
#endif

#include "prmt_landevice_eth.h"

extern unsigned int getInstNum( char *name, char *objname );
unsigned int getEthIFInstNum( char *name );

struct CWMP_OP tLANEthStatsLeafOP = { getLANEthStats, NULL };
struct CWMP_PRMT tLANEthStatsLeafInfo[] =
{
/*(name,			type,		flag,				op)*/
{"BytesSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"BytesReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"PacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"PacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"ErrorsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"ErrorsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"UnicastPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"UnicastPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"DiscardPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"DiscardPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"MulticastPacketsSent",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"MulticastPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"BroadcastPacketsSent",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"BroadcastPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP},
{"UnknownProtoPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tLANEthStatsLeafOP}
#endif
/*ping_zhang:20081217 END*/
};
enum eLANEthStatsLeaf
{
	eLANEth_BytesSent,
	eLANEth_BytesReceived,
	eLANEth_PacketsSent,
	eLANEth_PacketsReceived,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eLANEth_ErrorsSent,
	eLANEth_ErrorsReceived,
	eLANEth_UnicastPacketsSent,
	eLANEth_UnicastPacketsReceived,
	eLANEth_DiscardPacketsSent,
	eLANEth_DiscardPacketsReceived,
	eLANEth_MulticastPacketsSent,
	eLANEth_MulticastPacketsReceived,
	eLANEth_BroadcastPacketsSent,
	eLANEth_BroadcastPacketsReceived,
	eLANEth_UnknownProtoPacketsReceived
#endif
/*ping_zhang:20081217 END*/
};
struct CWMP_LEAF tLANEthStatsLeaf[] =
{
{ &tLANEthStatsLeafInfo[eLANEth_BytesSent] },
{ &tLANEthStatsLeafInfo[eLANEth_BytesReceived] },
{ &tLANEthStatsLeafInfo[eLANEth_PacketsSent] },
{ &tLANEthStatsLeafInfo[eLANEth_PacketsReceived] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tLANEthStatsLeafInfo[eLANEth_ErrorsSent] },
{ &tLANEthStatsLeafInfo[eLANEth_ErrorsReceived] },
{ &tLANEthStatsLeafInfo[eLANEth_UnicastPacketsSent] },
{ &tLANEthStatsLeafInfo[eLANEth_UnicastPacketsReceived] },
{ &tLANEthStatsLeafInfo[eLANEth_DiscardPacketsSent] },
{ &tLANEthStatsLeafInfo[eLANEth_DiscardPacketsReceived] },
{ &tLANEthStatsLeafInfo[eLANEth_MulticastPacketsSent] },
{ &tLANEthStatsLeafInfo[eLANEth_MulticastPacketsReceived] },
{ &tLANEthStatsLeafInfo[eLANEth_BroadcastPacketsSent] },
{ &tLANEthStatsLeafInfo[eLANEth_BroadcastPacketsReceived] },
{ &tLANEthStatsLeafInfo[eLANEth_UnknownProtoPacketsReceived] },
#endif
/*ping_zhang:20081217 END*/
{ NULL }
};



struct CWMP_OP tLANEthConfEntityLeafOP = { getLANEthConf, setLANEthConf };
struct CWMP_PRMT tLANEthConfEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tLANEthConfEntityLeafOP},
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tLANEthConfEntityLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Name",			eCWMP_tSTRING,	CWMP_READ,		&tLANEthConfEntityLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"MACAddress",			eCWMP_tSTRING,	CWMP_READ,		&tLANEthConfEntityLeafOP},
#ifdef MAC_FILTER
{"MACAddressControlEnabled",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tLANEthConfEntityLeafOP},
#endif /*MAC_FILTER*/
{"MaxBitRate",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANEthConfEntityLeafOP},
{"DuplexMode",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANEthConfEntityLeafOP}
};
enum tLANEthConfEntityLeaf
{
	eLANEth_Enable,
	eLANEth_Status,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eLANEth_Name,
#endif
/*ping_zhang:20081217 END*/
	eLANEth_MACAddress,
#ifdef MAC_FILTER
	eLANEth_MACAddressControlEnabled,
#endif /*MAC_FILTER*/
	eLANEth_MaxBitRate,
	eLANEth_DuplexMode
};
struct CWMP_LEAF tLANEthConfEntityLeaf[] =
{
{ &tLANEthConfEntityLeafInfo[eLANEth_Enable] },
{ &tLANEthConfEntityLeafInfo[eLANEth_Status] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tLANEthConfEntityLeafInfo[eLANEth_Name] },
#endif
/*ping_zhang:20081217 END*/
{ &tLANEthConfEntityLeafInfo[eLANEth_MACAddress] },
#ifdef MAC_FILTER
{ &tLANEthConfEntityLeafInfo[eLANEth_MACAddressControlEnabled] },
#endif /*MAC_FILTER*/
{ &tLANEthConfEntityLeafInfo[eLANEth_MaxBitRate] },
{ &tLANEthConfEntityLeafInfo[eLANEth_DuplexMode] },
#ifdef _PRMT_X_CT_COM_VLAN_BOUND_
{ &tCTVlanBoundingLeafInfo[eCTBOUNDING_Mode] },
{ &tCTVlanBoundingLeafInfo[eCTBOUNDING_VLAN] },
#endif
#ifdef _PRMT_X_CT_COM_LBD_
{ &tCT_EthLBDStatusLeafInfo[eCT_EthLBDStatus] },
#endif
{ NULL }
};
struct CWMP_PRMT tLANEthConfEntityObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Stats",			eCWMP_tOBJECT,	CWMP_READ,		NULL}
};
enum eLANEthConfEntityObject
{
	eLANEth_Stats
};
struct CWMP_NODE tLANEthConfEntityObject[] =
{
/*info,  					leaf,				node)*/
{ &tLANEthConfEntityObjectInfo[eLANEth_Stats],	tLANEthStatsLeaf,		NULL},
{ NULL,						NULL,				NULL}
};


struct CWMP_PRMT tLANEthConfObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL},
};
enum eLANEthConfObject
{
	eLANEth0,
};
struct CWMP_LINKNODE tLANEthConfObject[] =
{
/*info, 				leaf,			next,				sibling,		instnum)*/
{ &tLANEthConfObjectInfo[eLANEth0],		tLANEthConfEntityLeaf,		tLANEthConfEntityObject,		NULL,			0}
};

int getLANEthStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	unsigned long bs=0,br=0,ps=0,pr=0;
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	unsigned long es=0,er=0, ups=0,upr=0, dps=0,dpr=0, mps=0,mpr=0, bps=0, bpr=0, uppr=0;
#endif
/*ping_zhang:20081217 END*/
	char *lastname = entity->info->name;
	unsigned int instnum;
	char ifname[IFNAMSIZ];

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	instnum = getEthIFInstNum(name);
	if (instnum == 0)
		return ERR_9007;
#if defined(CONFIG_ETHWAN) || defined CONFIG_RTL_MULTI_LAN_DEV
	snprintf(ifname, sizeof(ifname), "%s%u", ALIASNAME_ELAN_PREFIX, ORIGINATE_NUM+instnum-1);
#else
	snprintf(ifname, sizeof(ifname), "eth%u", instnum - 1);
#endif

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "BytesSent") == 0) {
		if (getInterfaceStat(ifname, &bs, &br, &ps, &pr) < 0)
			return -1;
		*data = uintdup(bs);
	} else if (strcmp(lastname, "BytesReceived") == 0) {
		if (getInterfaceStat(ifname, &bs, &br, &ps, &pr) < 0)
			return -1;
		*data = uintdup(br);
	} else if (strcmp(lastname, "PacketsSent") == 0) {
		if (getInterfaceStat(ifname, &bs, &br, &ps, &pr) < 0)
			return -1;
		*data = uintdup(ps);
	} else if (strcmp(lastname, "PacketsReceived") == 0) {
		if (getInterfaceStat(ifname, &bs, &br, &ps, &pr) < 0)
			return -1;
		*data = uintdup(pr);
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	} else if (strcmp(lastname, "ErrorsSent") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(es);
	} else if (strcmp(lastname, "ErrorsReceived") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(er);
	} else if (strcmp(lastname, "UnicastPacketsSent") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(ups);
	} else if (strcmp(lastname, "UnicastPacketsReceived") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(upr);
	} else if (strcmp(lastname, "DiscardPacketsSent") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(dps);
	} else if (strcmp(lastname, "DiscardPacketsReceived") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(dpr);
	} else if (strcmp(lastname, "MulticastPacketsSent") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(mps);
	} else if (strcmp(lastname, "MulticastPacketsReceived") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(mpr);
	} else if (strcmp(lastname, "BroadcastPacketsSent") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(bps);
	} else if (strcmp(lastname, "BroadcastPacketsReceived") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(bpr);
	} else if (strcmp(lastname, "UnknownProtoPacketsReceived") == 0) {
		if (getInterfaceStat1(ifname, &es, &er, &ups, &upr, &dps, &dpr ,&mps, &mpr ,&bps, &bpr, &uppr) < 0)
			return -1;
		*data = uintdup(uppr);
#endif
/*ping_zhang:20081217 END*/
	} else {
		return ERR_9005;
	}

	return 0;
}

int objLANEthConf(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	unsigned int num=0,i,maxnum=0;

	CWMPDBG( 1, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
	fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	if( (name==NULL) || (entity==NULL) ) return -1;

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

#if defined(CONFIG_ETHWAN) || defined CONFIG_RTL_MULTI_LAN_DEV
		if( create_Object( ptable, tLANEthConfObject, sizeof(tLANEthConfObject), SW_LAN_PORT_NUM, 1) < 0 )
			return -1;
#else
		if( create_Object( ptable, tLANEthConfObject, sizeof(tLANEthConfObject), 1, 1) < 0 )
			return -1;
#endif
		return 0;
	     	break;
	     }
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
		return 0;
	}
	return -1;


}


int getLANEthConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned char vChar;
	unsigned int instnum;
	char ifname[IFNAMSIZ];

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	instnum = getEthIFInstNum(name);
	if (instnum == 0)
		return ERR_9007;
#if defined(CONFIG_ETHWAN) || defined CONFIG_RTL_MULTI_LAN_DEV
	snprintf(ifname, sizeof(ifname), "%s%u", ALIASNAME_ELAN_PREFIX, ORIGINATE_NUM+instnum-1);
#else
	snprintf(ifname, sizeof(ifname), "eth%u", instnum - 1);
#endif

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enable") == 0) {
		mib_get(CWMP_LAN_ETHIFENABLE, &vChar);
		*data = booldup(vChar != 0);
	} else if (strcmp(lastname, "Status") == 0) {
		int flags = 0;
		//how to detect "NoLink" condition
		if (getInFlags(ifname, &flags) == 1) {
			if (flags & IFF_UP)
				*data = strdup("Up");
			else
				*data = strdup("Disabled");
		} else
			*data = strdup( "Error" );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	} else if (strcmp(lastname, "Name") == 0) {
		*data = strdup(ifname);
#endif
/*ping_zhang:20081217 END*/
	} else if (strcmp(lastname, "MACAddress") == 0) {
		unsigned char buffer[64];
		unsigned char macadd[MAC_ADDR_LEN];

		//getMIB2Str(MIB_ELAN_MAC_ADDR, buffer);
		mib_get(MIB_ELAN_MAC_ADDR, macadd);
		sprintf(buffer, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0], macadd[1],
				macadd[2], macadd[3], macadd[4], macadd[5]);
		*data = strdup(buffer);
#ifdef MAC_FILTER
	} else if (strcmp(lastname, "MACAddressControlEnabled") == 0) {
		mib_get(MIB_ETH_MAC_CTRL, &vChar);
		*data = booldup(vChar != 0);
#endif /*MAC_FILTER*/
	} else if( strcmp( lastname, "MaxBitRate" )==0 ) {
#ifdef ELAN_LINK_MODE
		MIB_CE_SW_PORT_T Entry;

		if (!mib_chain_get(MIB_SW_PORT_TBL, instnum - 1, &Entry))
			return ERR_9002;
		switch (Entry.linkMode) {
		case LINK_10HALF:
		case LINK_10FULL:
			*data = strdup("10");
			break;
		case LINK_100HALF:
		case LINK_100FULL:
			*data = strdup("100");
			break;
		default:
			*data = strdup("Auto");
		}
#else
		*data = strdup("Auto"); //doesn't support this now!
#endif
	} else if (strcmp(lastname, "DuplexMode") == 0) {
#ifdef ELAN_LINK_MODE
		MIB_CE_SW_PORT_T Entry;

		if (!mib_chain_get(MIB_SW_PORT_TBL, instnum - 1, &Entry))
			return ERR_9002;
		switch (Entry.linkMode) {
		case LINK_10HALF:
		case LINK_100HALF:
			*data = strdup("Half");
			break;
		case LINK_10FULL:
		case LINK_100FULL:
			*data = strdup("Full");
			break;
		default:
			*data = strdup("Auto");
		}
#else
		*data = strdup("Auto"); //doesn't support this now!
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

int setLANEthConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned char vChar;
	unsigned int instnum;
	char ifname[IFNAMSIZ];
	char *buf = data;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	instnum = getEthIFInstNum(name);
	if (instnum == 0)
		return ERR_9007;
#if defined(CONFIG_ETHWAN) || defined CONFIG_RTL_MULTI_LAN_DEV
	snprintf(ifname, sizeof(ifname), "%s%u", ALIASNAME_ELAN_PREFIX, ORIGINATE_NUM+instnum-1);
#else
	snprintf(ifname, sizeof(ifname), "eth%u", instnum - 1);
#endif

	if (strcmp(lastname, "Enable") == 0) {
		int *i = data;

		if (i == NULL) return ERR_9007;
		vChar = (*i == 0) ? 0 : 1;
		mib_set(CWMP_LAN_ETHIFENABLE, &vChar);

		apply_add(CWMP_PRI_N, apply_ETHER, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#ifdef MAC_FILTER
	} else if (strcmp(lastname, "MACAddressControlEnabled") == 0) {
		int *i = data;

		if (i == NULL) return ERR_9007;
		vChar = (*i == 0) ? 0 : 1;
		mib_set(MIB_ETH_MAC_CTRL, &vChar);
		{
			unsigned char wlan_mac_ctrl = 0, mac_out_dft = 1;

			mib_get(MIB_WLAN_MAC_CTRL, &wlan_mac_ctrl);
			if (vChar == 1 || wlan_mac_ctrl == 1)
				mac_out_dft = 0; //0:deny, 1:allow
			mib_set(MIB_MACF_OUT_ACTION, &mac_out_dft);
		}

		apply_add(CWMP_PRI_N, apply_MACFILTER, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#endif /*MAC_FILTER*/
	} else if( strcmp( lastname, "MaxBitRate" )==0 ) {
#ifdef ELAN_LINK_MODE
		MIB_CE_SW_PORT_T Entry;

		if (buf == NULL)
			return ERR_9007;

		if (!mib_chain_get(MIB_SW_PORT_TBL, instnum - 1, &Entry))
			return ERR_9002;

		if (strcmp(buf, "Auto") == 0) {
			Entry.linkMode = LINK_AUTO;
		} else if (strcmp(buf, "10") == 0) {
			switch (Entry.linkMode) {
			case LINK_10HALF:
			case LINK_10FULL:
				break;
			case LINK_100HALF:
				Entry.linkMode = LINK_10HALF;
				break;
			case LINK_100FULL:
			case LINK_AUTO://or LINK_10Half
				Entry.linkMode = LINK_10FULL;
				break;
			}
		} else if (strcmp(buf, "100") == 0) {
			switch (Entry.linkMode) {
			case LINK_10HALF:
				Entry.linkMode = LINK_100HALF;
				break;
			case LINK_10FULL:
			case LINK_AUTO://or LINK_100Half
				Entry.linkMode = LINK_100FULL;
				break;
			case LINK_100HALF:
			case LINK_100FULL:
				break;
			}
		} else
			return ERR_9007;
		mib_chain_update(MIB_SW_PORT_TBL, &Entry, instnum - 1);

		apply_add(CWMP_PRI_N, apply_ETHER, CWMP_RESTART, instnum, NULL, 0);
		return 0;
#else
		if ((buf != NULL) && (strcmp(buf, "Auto") != 0))
			return ERR_9001;
#endif
	}else if( strcmp( lastname, "DuplexMode" )==0 )
	{
#ifdef ELAN_LINK_MODE
		MIB_CE_SW_PORT_T Entry;

		if (buf == NULL)
			return ERR_9007;

		if (!mib_chain_get(MIB_SW_PORT_TBL, instnum - 1, &Entry))
			return ERR_9002;

		if (strcmp(buf, "Auto") == 0) {
			Entry.linkMode = LINK_AUTO;
		} else if (strcmp(buf, "Half") == 0) {
			switch(Entry.linkMode) {
			case LINK_10HALF:
			case LINK_100HALF:
				break;
			case LINK_10FULL:
				Entry.linkMode = LINK_10HALF;
				break;
			case LINK_100FULL:
			case LINK_AUTO://or LINK_10Half
				Entry.linkMode = LINK_100HALF;
				break;
			}
		} else if (strcmp(buf, "Full") == 0) {
			switch(Entry.linkMode) {
			case LINK_10HALF:
				Entry.linkMode = LINK_10FULL;
				break;
			case LINK_100HALF:
			case LINK_AUTO://or LINK_10Full
				Entry.linkMode = LINK_100FULL;
				break;
			case LINK_10FULL:
			case LINK_100FULL:
				break;
			}
		} else
			return ERR_9007;
		mib_chain_update(MIB_SW_PORT_TBL, &Entry, instnum - 1);

		apply_add(CWMP_PRI_N, apply_ETHER, CWMP_RESTART, instnum, NULL, 0);
		return 0;
#else
		if ((buf != NULL) && (strcmp(buf, "Auto") != 0))
			return ERR_9001;
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}

/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
unsigned int getEthIFInstNum( char *name )
{
	return getInstNum( name, "LANEthernetInterfaceConfig" );
}

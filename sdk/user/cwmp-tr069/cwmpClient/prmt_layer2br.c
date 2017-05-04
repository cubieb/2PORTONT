#include "prmt_layer2br.h"
#include "prmt_wancondevice.h"

#define IFGROUP_NUM 5
#define CHECK_PARAM_NUM(input, min, max) if ( (input < min) || (input > max) ) return ERR_9007;

unsigned int AvailableInterfaceNum = 0;
struct availableItfInfo itfList[MAX_NUM_OF_ITFS];

////////////////////////////////////////////////////////////////////////////////////////
/* InternetGatewayDevice.Layer2Bridging.Bridge.{i}. */
struct CWMP_OP tBridgeOP = {getLayer2BrBridge, setLayer2BrBridge};
struct CWMP_PRMT tBridgeLeafInfo[] =
{
/* (name,               type,           flag,                 op) */
{"BridgeKey",           eCWMP_tUINT,    CWMP_READ,            &tBridgeOP},
{"BridgeStandard",      eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tBridgeOP},
{"BridgeEnable",        eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tBridgeOP},
{"BridgeStatus",        eCWMP_tSTRING,  CWMP_READ,            &tBridgeOP},
{"BridgeName",          eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tBridgeOP},
{"VLANID",              eCWMP_tUINT,    CWMP_WRITE|CWMP_READ, &tBridgeOP},
{"PortNumberOfEntries", eCWMP_tUINT,    CWMP_READ,            &tBridgeOP},
{"VLANNumberOfEntries", eCWMP_tUINT,    CWMP_READ,            &tBridgeOP}
};
enum eBridgeLeaf
{
	eBridgeKey,
	eBridgeStandard,
	eBridgeEnable,
	eBridgeStatus,
	eBridgeName,
	eVLANID,
	ePortNumberOfEntries,
	eVLANNumberOfEntries
};
struct CWMP_LEAF tBridgeLeaf[] =
{
{&tBridgeLeafInfo[eBridgeKey]},
//{&tBridgeLeafInfo[eBridgeStandard]},
{&tBridgeLeafInfo[eBridgeEnable]},
{&tBridgeLeafInfo[eBridgeStatus]},
{&tBridgeLeafInfo[eBridgeName]},
{&tBridgeLeafInfo[eVLANID]},
//{&tBridgeLeafInfo[ePortNumberOfEntries]},
//{&tBridgeLeafInfo[eVLANNumberOfEntries]},
{NULL}
};

struct CWMP_PRMT tBridgeObjectInfo[] =
{
/* (name, type, flag, op) */
{"0", eCWMP_tOBJECT, CWMP_READ|CWMP_WRITE|CWMP_LNKLIST, NULL}
};
enum eBridgeObject
{
	eBridgeObj
};
struct CWMP_LINKNODE tBridgeObject[] =
{
/* (info, leaf, next, sibling, instnum) */
{&tBridgeObjectInfo[eBridgeObj], tBridgeLeaf, NULL, NULL, 0}
};

////////////////////////////////////////////////////////////////////////////////////////
/* InternetGatewayDevice.Layer2Bridging.Filter.{i}. */
struct CWMP_OP tFilterOP = {getLayer2BrFilter, setLayer2BrFilter};
struct CWMP_PRMT tFilterLeafInfo[] = 
{
/* (name,                                   type,           flag,                 op) */
{"FilterKey",                               eCWMP_tUINT,    CWMP_READ,            &tFilterOP},
{"FilterEnable",                            eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP},
{"FilterStatus",                            eCWMP_tSTRING,  CWMP_READ,            &tFilterOP},
{"FilterBridgeReference",                   eCWMP_tINT,     CWMP_WRITE|CWMP_READ, &tFilterOP},
{"ExclusivityOrder",                        eCWMP_tUINT,    CWMP_WRITE|CWMP_READ, &tFilterOP},
{"FilterInterface",                         eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"VLANIDFilter",                            eCWMP_tINT,     CWMP_WRITE|CWMP_READ, &tFilterOP},
{"AdmitOnlyVLANTagged",                     eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP},
{"EthertypeFilterList",                     eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"EthertypeFilterExclude",                  eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP},
{"SourceMACAddressFilterList",              eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"SourceMACAddressFilterExclude",           eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP},
{"DestMACAddressFilterList",                eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"DestMACAddressFilterExclude",             eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP},
{"SourceMACFromVendorClassIDFilter",        eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"SourceMACFromVendorClassIDFilterExclude", eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP},
{"SourceMACFromVendorClassIDMode",          eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"DestMACFromVendorClassIDFilter",          eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"DestMACFromVendorClassIDFilterExclude",   eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP},
{"DestMACFromVendorClassIDMode",            eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"SourceMACFromClientIDFilter",             eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"SourceMACFromClientIDFilterExclude",      eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP},
{"DestMACFromClientIDFilter",               eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"DestMACFromClientIDFilterExclude",        eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP},
{"SourceMACFromUserClassIDFilter",          eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"SourceMACFromUserClassIDFilterExclude",   eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP},
{"DestMACFromUserClassIDFilter",            eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tFilterOP},
{"DestMACFromUserClassIDFilterExclude",     eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tFilterOP}
};
enum eFilterLeaf
{
	eFilterKey,
	eFilterEnable,
	eFilterStatus,
	eFilterBridgeReference,
	eExclusivityOrder,
	eFilterInterface,
	eVLANIDFilter,
	eAdmitOnlyVLANTagged,
	eEthertypeFilterList,
	eEthertypeFilterExclude,
	eSourceMACAddressFilterList,
	eSourceMACAddressFilterExclude,
	eDestMACAddressFilterList,
	eDestMACAddressFilterExclude,
	eSourceMACFromVendorClassIDFilter,
	eSourceMACFromVendorClassIDFilterExclude,
	eSourceMACFromVendorClassIDMode,
	eDestMACFromVendorClassIDFilter,
	eDestMACFromVendorClassIDFilterExclude,
	eDestMACFromVendorClassIDMode,
	eSourceMACFromClientIDFilter,
	eSourceMACFromClientIDFilterExclude,
	eDestMACFromClientIDFilter,
	eDestMACFromClientIDFilterExclude,
	eSourceMACFromUserClassIDFilter,
	eSourceMACFromUserClassIDFilterExclude,
	eDestMACFromUserClassIDFilter,
	eDestMACFromUserClassIDFilterExclude
};
struct CWMP_LEAF tFilterLeaf[] = 
{
{&tFilterLeafInfo[eFilterKey]},
{&tFilterLeafInfo[eFilterEnable]},
{&tFilterLeafInfo[eFilterStatus]},
{&tFilterLeafInfo[eFilterBridgeReference]},
//{&tFilterLeafInfo[eExclusivityOrder]},
{&tFilterLeafInfo[eFilterInterface]},
//{&tFilterLeafInfo[eVLANIDFilter]},
//{&tFilterLeafInfo[eAdmitOnlyVLANTagged]},
//{&tFilterLeafInfo[eEthertypeFilterList]},
//{&tFilterLeafInfo[eEthertypeFilterExclude]},
//{&tFilterLeafInfo[eSourceMACAddressFilterList]},
//{&tFilterLeafInfo[eSourceMACAddressFilterExclude]},
//{&tFilterLeafInfo[eDestMACAddressFilterList]},
//{&tFilterLeafInfo[eDestMACAddressFilterExclude]},
//{&tFilterLeafInfo[eSourceMACFromVendorClassIDFilter]},
//{&tFilterLeafInfo[eSourceMACFromVendorClassIDFilterExclude]},
//{&tFilterLeafInfo[eSourceMACFromVendorClassIDMode]},
//{&tFilterLeafInfo[eDestMACFromVendorClassIDFilter]},
//{&tFilterLeafInfo[eDestMACFromVendorClassIDFilterExclude]},
//{&tFilterLeafInfo[eDestMACFromVendorClassIDMode]},
//{&tFilterLeafInfo[eSourceMACFromClientIDFilter]},
//{&tFilterLeafInfo[eSourceMACFromClientIDFilterExclude]},
//{&tFilterLeafInfo[eDestMACFromClientIDFilter]},
//{&tFilterLeafInfo[eDestMACFromClientIDFilterExclude]},
//{&tFilterLeafInfo[eSourceMACFromUserClassIDFilter]},
//{&tFilterLeafInfo[eSourceMACFromUserClassIDFilterExclude]},
//{&tFilterLeafInfo[eDestMACFromUserClassIDFilter]},
//{&tFilterLeafInfo[eDestMACFromUserClassIDFilterExclude]},
{NULL}
};

struct CWMP_PRMT tFilterObjectInfo[] =
{
/* (name, type, flag, op) */
{"0", eCWMP_tOBJECT, CWMP_READ|CWMP_WRITE|CWMP_LNKLIST, NULL}
};
enum eFilterObject
{
	eFilterObj
};
struct CWMP_LINKNODE tFilterObject[] =
{
/* (info, leaf, next, sibling, instnum) */
{&tFilterObjectInfo[eFilterObj], tFilterLeaf, NULL, NULL, 0}
};

////////////////////////////////////////////////////////////////////////////////////////
/* InternetGatewayDevice.Layer2Bridging.Marking.{i}. */
struct CWMP_OP tMarkingLeafOP = {getLayer2BrMarking, setLayer2BrMarking};
struct CWMP_PRMT tMarkingLeafInfo[] = 
{
/* (name,                    type,           flag,                 op) */
{"MarkingKey",               eCWMP_tUINT,    CWMP_READ,            &tMarkingLeafOP},
{"MarkingEnable",            eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tMarkingLeafOP},
{"MarkingStatus",            eCWMP_tSTRING,  CWMP_READ,            &tMarkingLeafOP},
{"MarkingBridgeReference",   eCWMP_tINT,     CWMP_WRITE|CWMP_READ, &tMarkingLeafOP},
{"MarkingInterface",         eCWMP_tSTRING,  CWMP_WRITE|CWMP_READ, &tMarkingLeafOP},
{"VLANIDUntag",              eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tMarkingLeafOP},
{"VLANIDMark",               eCWMP_tINT,     CWMP_WRITE|CWMP_READ, &tMarkingLeafOP},
{"VLANIDMarkOverride",       eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tMarkingLeafOP},
{"EthernetPriorityMark",     eCWMP_tINT,     CWMP_WRITE|CWMP_READ, &tMarkingLeafOP},
{"EthernetPriorityOverride", eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ, &tMarkingLeafOP}
};
enum eMarkingLeaf
{
	eMarkingKey,
	eMarkingEnable,
	eMarkingStatus,
	eMarkingBridgeReference,
	eMarkingInterface,
	eVLANIDUntag,
	eVLANIDMark,
	eVLANIDMarkOverride,
	eEthernetPriorityMark,
	eEthernetPriorityOverride
};
struct CWMP_LEAF tMarkingLeaf[] = 
{
{&tMarkingLeafInfo[eMarkingKey]},
{&tMarkingLeafInfo[eMarkingEnable]},
{&tMarkingLeafInfo[eMarkingStatus]},
{&tMarkingLeafInfo[eMarkingBridgeReference]},
{&tMarkingLeafInfo[eMarkingInterface]},
//{&tMarkingLeafInfo[eVLANIDUntag]},
{&tMarkingLeafInfo[eVLANIDMark]},
//{&tMarkingLeafInfo[eVLANIDMarkOverride]},
{&tMarkingLeafInfo[eEthernetPriorityMark]},
//{&tMarkingLeafInfo[eEthernetPriorityOverride]},
{NULL}
};

struct CWMP_PRMT tMarkingObjectInfo[] =
{
/* (name, type, flag, op) */
{"0", eCWMP_tOBJECT, CWMP_READ|CWMP_WRITE|CWMP_LNKLIST, NULL}
};
enum eMarkingObject
{
	eMarkingObj
};
struct CWMP_LINKNODE tMarkingObject[] =
{
/* (info, leaf, next, sibling, instnum) */
{&tMarkingObjectInfo[eMarkingObj], tMarkingLeaf, NULL, NULL, 0}
};

////////////////////////////////////////////////////////////////////////////////////////
/* InternetGatewayDevice.Layer2Bridging.AvailableInterface.{i}. */
struct CWMP_OP tAvailableInterfaceLeafOP = {getLayer2BrAvailableInterface, NULL};
struct CWMP_PRMT tAvailableInterfaceLeafInfo[] = 
{
/* (name,                 type,          flag,      op) */
{"AvailableInterfaceKey", eCWMP_tUINT,   CWMP_READ, &tAvailableInterfaceLeafOP},
{"InterfaceType",         eCWMP_tSTRING, CWMP_READ, &tAvailableInterfaceLeafOP},
{"InterfaceReference",    eCWMP_tSTRING, CWMP_READ, &tAvailableInterfaceLeafOP}
};
enum eAvailableInterfaceLeaf
{
	eAvailableInterfaceKey,
	eInterfaceType,
	eInterfaceReference	
};
struct CWMP_LEAF tAvailableInterfaceLeaf[] = 
{
{&tAvailableInterfaceLeafInfo[eAvailableInterfaceKey]},
{&tAvailableInterfaceLeafInfo[eInterfaceType]},
{&tAvailableInterfaceLeafInfo[eInterfaceReference]},
{NULL}
};

struct CWMP_PRMT tAvailableInterfaceObjectInfo[] = 
{
/* (name, type, flag, op) */
{"0", eCWMP_tOBJECT, CWMP_READ|CWMP_LNKLIST, NULL}
};
enum eAvailableInterfaceObject
{
	eAvailableInterfaceObj
};
struct CWMP_LINKNODE tAvailableInterfaceObject[] = 
{
/* (info, leaf, next, sibling, instnum) */
{&tAvailableInterfaceObjectInfo[eAvailableInterfaceObj], tAvailableInterfaceLeaf, NULL, NULL, 0}
};

////////////////////////////////////////////////////////////////////////////////////////////
/* InternetGatewayDevice.Layer2Bridging. */
/* Leaves */
struct CWMP_OP tLayer2BridgingLeafOP = {getLayer2Br, NULL};
struct CWMP_PRMT tLayer2BridgingLeafInfo[] =
{
/* (name,                             type,        flag,      op) */
{"MaxBridgeEntries",                  eCWMP_tUINT, CWMP_READ, &tLayer2BridgingLeafOP},
{"MaxDBridgeEntries",                 eCWMP_tUINT, CWMP_READ, &tLayer2BridgingLeafOP},
{"MaxQBridgeEntries",                 eCWMP_tUINT, CWMP_READ, &tLayer2BridgingLeafOP},
{"MaxVLANEntries",                    eCWMP_tUINT, CWMP_READ, &tLayer2BridgingLeafOP},
{"MaxFilterEntries",                  eCWMP_tUINT, CWMP_READ, &tLayer2BridgingLeafOP},
{"MaxMarkingEntries",                 eCWMP_tUINT, CWMP_READ, &tLayer2BridgingLeafOP},
{"BridgeNumberOfEntries",             eCWMP_tUINT, CWMP_READ, &tLayer2BridgingLeafOP},
{"FilterNumberOfEntries",             eCWMP_tUINT, CWMP_READ, &tLayer2BridgingLeafOP},
{"MarkingNumberOfEntries",            eCWMP_tUINT, CWMP_READ, &tLayer2BridgingLeafOP},
{"AvailableInterfaceNumberOfEntries", eCWMP_tUINT, CWMP_READ, &tLayer2BridgingLeafOP}
};
enum eLayer2BridgingLeaf
{
	eBRMaxBridgeEntries,
	eBRMaxDBridgeEntries,
	eBRMaxQBridgeEntries,
	eBRMaxVLANEntries,
	eBRMaxFilterEntries,
	eBRMaxMarkingEntries,
	eBRBridgeNumberOfEntries,
	eBRFilterNumberOfEntries,
	eBRMarkingNumberOfEntries,
	eBRAvailableInterfaceNumberOfEntries
};
struct CWMP_LEAF tLayer2BridgingLeaf[] = 
{
{&tLayer2BridgingLeafInfo[eBRMaxBridgeEntries]},
//{&tLayer2BridgingLeafInfo[eBRMaxDBridgeEntries]},
//{&tLayer2BridgingLeafInfo[eBRMaxQBridgeEntries]},
//{&tLayer2BridgingLeafInfo[eBRMaxVLANEntries]},
{&tLayer2BridgingLeafInfo[eBRMaxFilterEntries]},
{&tLayer2BridgingLeafInfo[eBRMaxMarkingEntries]},
{&tLayer2BridgingLeafInfo[eBRBridgeNumberOfEntries]},
{&tLayer2BridgingLeafInfo[eBRFilterNumberOfEntries]},
{&tLayer2BridgingLeafInfo[eBRMarkingNumberOfEntries]},
{&tLayer2BridgingLeafInfo[eBRAvailableInterfaceNumberOfEntries]},
{NULL}
};

/* Nodes */
struct CWMP_OP tBridgeObjOP = {NULL, objLayer2BrBridgeOP};
struct CWMP_OP tFilterObjOP = {NULL, objLayer2BrFilterOP};
struct CWMP_OP tMarkingObjOP = {NULL, objLayer2BrMarkingOP};
struct CWMP_OP tAvailableInterfaceObjOP = {NULL, objLayer2BrAvailableInterfaceOP};
struct CWMP_PRMT tLayer2BridgingObjectInfo[] =
{
/* (name,              type,          flag,                 op) */
{"Bridge",             eCWMP_tOBJECT, CWMP_READ|CWMP_WRITE, &tBridgeObjOP},
{"Filter",             eCWMP_tOBJECT, CWMP_READ|CWMP_WRITE, &tFilterObjOP},
{"Marking",            eCWMP_tOBJECT, CWMP_READ|CWMP_WRITE, &tMarkingObjOP},
{"AvailableInterface", eCWMP_tOBJECT, CWMP_READ,            &tAvailableInterfaceObjOP}
};
enum eLayer2BridgingObject
{
	eBRBridge,
	eBRFilter,
	eBRMarking,
	eBRAvailableInterface
};
struct CWMP_NODE tLayer2BridgingObject[] =
{
/* (info,                                           leaf, node) */
{&tLayer2BridgingObjectInfo[eBRBridge],             NULL, NULL},
{&tLayer2BridgingObjectInfo[eBRFilter],             NULL, NULL},
{&tLayer2BridgingObjectInfo[eBRMarking],            NULL, NULL},
{&tLayer2BridgingObjectInfo[eBRAvailableInterface], NULL, NULL},
{NULL, NULL, NULL}
};
/***************************************************************************************/

int getATMVCEntryandIDByIfindex(unsigned int ifindex, MIB_CE_ATM_VC_T *p, unsigned int *id)
{
	int ret = -1;
	unsigned int i = 0, num = 0;

	if ((ifindex == 0) || (p == NULL) || (id == NULL))
		return ret;

	num = mib_chain_total(MIB_ATM_VC_TBL);
	for (i = 0; i < num; i++)
	{
		if (!mib_chain_get(MIB_ATM_VC_TBL, i, (void*)p))
			continue;

		if (p->ifIndex == ifindex)
		{
			*id = i;
			ret = 0;
			break;
		}
	}
	return ret;
}

#if 0 // not support vlan now
int setVLANByGroupnum(int groupnum, int enable, int vlan)
{
	struct itfInfo itfs[MAX_NUM_OF_ITFS];
	unsigned int i = 0, j = 0, total = 0, ifnum = 0, LAN_change = 0, WAN_change = 0, ret = 0;

	ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, groupnum);

	if (ifnum > 0)
	{
		for (i = 0; i < ifnum; i++)
		{
			if (itfs[i].ifdomain == DOMAIN_ELAN)
			{
				MIB_CE_SW_PORT_T Entry;
#ifdef CONFIG_RTK_RG_INIT // Rostelecom, Port Binding function
				if (!mib_chain_get(MIB_SW_PORT_TBL, itfs[i].ifid, &Entry))
					continue;

				Entry.vlan_on_lan_enabled = enable;
				Entry.vid = vlan;

				mib_chain_update(MIB_SW_PORT_TBL, &Entry, itfs[i].ifid);
				LAN_change++;
#else
				total = mib_chain_total(MIB_SW_PORT_TBL);
				if (itfs[i].ifid < total)
				{
					continue; // no VLAN
				}
				else if (itfs[i].ifid >= total && itfs[i].ifid < (total * 2))
				{
					if (!mib_chain_get(MIB_SW_PORT_TBL, itfs[i].ifid - total, &Entry))
						continue;

					if (LAN_change == 0)
					{
						setup_bridge_grouping(DEL_RULE);
						setup_VLANonLAN(DEL_RULE);
					}

					Entry.vlan_on_lan_enabled = enable;
					Entry.vid = vlan;

					mib_chain_update(MIB_SW_PORT_TBL, &Entry, itfs[i].ifid - total);
					LAN_change++;
				}
#endif
			}
			else if (itfs[i].ifdomain == DOMAIN_WAN)
			{
				MIB_CE_ATM_VC_T Entry;

				total = mib_chain_total(MIB_ATM_VC_TBL);
				for (j = 0; j < total; j++)
				{
					if (!mib_chain_get(MIB_ATM_VC_TBL, j, (void *)&Entry))
						continue;

					if (Entry.enable && isValidMedia(Entry.ifIndex) && Entry.ifIndex == itfs[i].ifid && (Entry.vlan != enable || Entry.vid != vlan))
					{
						Entry.vlan = enable;
						Entry.vid = vlan;
						mib_chain_update(MIB_ATM_VC_TBL, (void *)&Entry, j);
						WAN_change++;
					}
				}
			}
			else if (itfs[i].ifdomain == DOMAIN_WLAN)
			{
				continue; // not support VLAN now
			}
			else if (itfs[i].ifdomain == DOMAIN_ULAN)
			{
				continue; // not support VLAN now
			}
		}
	}

	if (LAN_change)
	{
		setup_VLANonLAN(ADD_RULE);
		setup_bridge_grouping(ADD_RULE);
	}

	if (WAN_change)
	{
		//restartWAN(); do it after session termination
		ret = CWMP_NEED_RESTART_WAN;
	}

	return ret;
}
#endif

int setGroupEnableByGroupnum(int groupnum, int enable)
{
	int ret = -1;
	unsigned int i = 0, j = 0, total = 0, ifnum = 0, isRestartWAN = 0;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];
	MIB_L2BRIDGE_GROUP_T *p, entry;
	p = &entry;

	total = mib_chain_total(MIB_L2BRIDGING_BRIDGE_GROUP_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, i, (void *)p))
			continue;

		if (p->groupnum == groupnum)
		{
			if (enable == 0)
			{
#if 0 // not support vlan now
				// remove vlan of this group
				if (p->vlanid)
				{
					isRestartWAN = setVLANByGroupnum(p->groupnum, 0, 0);
				}
#endif
				memset(itfs, 0, sizeof(itfs));
				ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, p->groupnum);
				if (ifnum > 0)
				{
					char list[512] = {0};
					char *ptr = list;

					// p->groupnum group
					for (j = 0; j < ifnum; j++)
					{
						ptr += snprintf(ptr, 64, "%u,", IF_ID(itfs[j].ifdomain, itfs[j].ifid));
					}

					memset(itfs, 0, sizeof(itfs));
					ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, 0);
					if (ifnum > 0)
					{
						// default group
						for (j = 0; j < ifnum; j++)
						{
							ptr += snprintf(ptr, 64, "%u,", IF_ID(itfs[j].ifdomain, itfs[j].ifid));
						}
					}

					setup_bridge_grouping(DEL_RULE);
					setgroup(list, 0);
					setup_bridge_grouping(ADD_RULE);
#ifdef CONFIG_RTK_RG_INIT // Rostelecom, Port Binding function
					unsigned int set_wanlist = 0;
				
					if (set_port_binding_mask(&set_wanlist) > 0)
					{
						rg_set_port_binding_mask(set_wanlist);
					}
#endif

#ifdef COMMIT_IMMEDIATELY
					Commit();
#endif

					ret = 0;
					break;
				}
			}
		}
	}

	if (ret == 0 && isRestartWAN == CWMP_NEED_RESTART_WAN)
	{
		ret = CWMP_NEED_RESTART_WAN;
	}
	return ret;
}

int getGroupByGroupnum(MIB_L2BRIDGE_GROUP_T *pentry, int groupnum, unsigned int *chainid)
{	
	int ret = -1;
	unsigned int i = 0, total = 0;

	if (!pentry || !chainid) return ret;

	total = mib_chain_total(MIB_L2BRIDGING_BRIDGE_GROUP_TBL);
	for (i = 0; i < total; i++)
	{
		if (!mib_chain_get(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, i, (void *)pentry))
			continue;

		if (pentry->groupnum == groupnum) 
		{
			*chainid = i;
			ret = 0;
			break;
		}
	}

	return ret;
}

int getUnusedGroupnum()
{
	unsigned int ret = 0, i = 0, num = 0;
	MIB_L2BRIDGE_GROUP_T *p, entry;
	int chainid = 0;

	for (i = 0; i < IFGROUP_NUM; i++)
	{
		p = &entry;
		if (getGroupByGroupnum(p, i, &chainid) == 0)
			continue;

		ret = i;
		break;
	}

	return ret;
}

unsigned int getLayer2BrBridgeInstNum(char *name)
{
	return getInstNum(name, "Bridge");
}

unsigned int getLayer2BrFilterInstNum(char *name)
{
	return getInstNum(name, "Filter");
}

unsigned int getLayer2BrMarkingInstNum(char *name)
{
	return getInstNum(name, "Marking");
}

unsigned int getLayer2BrAvailableInterfaceInstNum(char *name)
{
	return getInstNum(name, "AvailableInterface");
}

static int update_itfList()
{
	int i = 0, ifnum = 0;
	struct itfInfo itfs[MAX_NUM_OF_ITFS];
	unsigned int chainid = 0;
	MIB_L2BRIDGE_GROUP_T *p, entry;
	p = &entry;

	AvailableInterfaceNum = 0;
	memset(itfList, 0, sizeof(itfList));

	for (i = 0; i < IFGROUP_NUM; i++)
	{
		memset(itfs, 0, sizeof(itfs));
		ifnum = get_group_ifinfo(itfs, MAX_NUM_OF_ITFS, i);
		if (ifnum > 0) 
		{
			if (getGroupByGroupnum(p, i, &chainid) == -1)
			{
				memset(p, 0, sizeof(MIB_L2BRIDGE_GROUP_T));
				p->enable = 1;
				p->groupnum = i;

				if (i == 0)
					strcpy(p->name, "Default");
				else
					sprintf(p->name, "Group%d", i);

				p->vlanid = 0;
				mib_chain_add(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, (unsigned char *)p);
#ifdef COMMIT_IMMEDIATELY
				Commit();
#endif
			}
		}
	}

	AvailableInterfaceNum = get_AvailableInterface(itfList, MAX_NUM_OF_ITFS);

#if 0 // debug message
	fprintf(stderr, "\033[1;31m AvailableInterfaceNum=%d\n\033[0m", AvailableInterfaceNum);
	for (i = 0; i < AvailableInterfaceNum; i++)
	{
		fprintf(stderr, "===== [%d] =====\n", i);
		fprintf(stderr, "  itfList[%d].ifdomain=%d\n", i, itfList[i].ifdomain);
		fprintf(stderr, "  itfList[%d].ifid=%d\n", i, itfList[i].ifid);
		fprintf(stderr, "  itfList[%d].name=%s\n", i, itfList[i].name);
		fprintf(stderr, "  itfList[%d].itfGroup=%d\n\n", i, itfList[i].itfGroup);
	}
#endif

	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
/* InternetGatewayDevice.Layer2Bridging. */
int getLayer2Br(char* name, struct CWMP_LEAF* entity, int* type, void** data)
{
	if ((name == NULL) || (type == NULL) || (data == NULL) || (entity == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	char *lastname = entity->info->name;
	
	if (strcmp(lastname, "MaxBridgeEntries") == 0)
	{
		*data = uintdup(IFGROUP_NUM);
	}
	else if (strcmp(lastname, "MaxDBridgeEntries") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "MaxQBridgeEntries") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "MaxVLANEntries") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "MaxFilterEntries") == 0)
	{
		*data = uintdup(MAX_NUM_OF_ITFS);
	}
	else if (strcmp(lastname, "MaxMarkingEntries") == 0)
	{
		*data = uintdup(MAX_NUM_OF_ITFS);
	}
	else if (strcmp(lastname, "BridgeNumberOfEntries") == 0)
	{
		*data = uintdup(mib_chain_total(MIB_L2BRIDGING_BRIDGE_GROUP_TBL));
	}
	else if (strcmp(lastname, "FilterNumberOfEntries") == 0)
	{
		*data = uintdup(AvailableInterfaceNum);
	}
	else if (strcmp(lastname, "MarkingNumberOfEntries") == 0)
	{
		*data = uintdup(AvailableInterfaceNum);
	}
	else if (strcmp(lastname, "AvailableInterfaceNumberOfEntries") == 0)
	{
		*data = uintdup(AvailableInterfaceNum);
	}
	else
	{
		return ERR_9005;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////////////
/* InternetGatewayDevice.Layer2Bridging.Bridge.{i}. */
int getLayer2BrBridge(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	if ((name == NULL) || (type == NULL) || (data == NULL) || (entity == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	char *lastname = entity->info->name;

	unsigned int object_num = 0, chain_id = 0;
	MIB_L2BRIDGE_GROUP_T *p, entry;
	p = &entry;

	object_num = getLayer2BrBridgeInstNum(name);

	if (!mib_chain_get(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, (object_num - 1), (void *)p))
		return ERR_9002;

	if (strcmp(lastname, "BridgeKey") == 0)
	{
		*data = uintdup(p->groupnum);
	}
	else if (strcmp(lastname, "BridgeStandard") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "BridgeEnable") == 0)
	{
		if (p->enable == 0)
			*data = booldup(0);
		else
			*data = booldup(1);
	}
	else if (strcmp(lastname, "BridgeStatus") == 0)
	{
		if (p->enable == 0)
			*data = strdup("Disabled");
		else
			*data = strdup("Enabled");
	}
	else if (strcmp(lastname, "BridgeName") == 0)
	{
		*data = (p->name != NULL && strlen(p->name) > 0) ? strdup(p->name) : strdup("");
	}
	else if (strcmp(lastname, "VLANID") == 0)
	{
		*data = uintdup(p->vlanid);
	}
	else if (strcmp(lastname, "PortNumberOfEntries") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "VLANNumberOfEntries") == 0)
	{
		return ERR_9005;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setLayer2BrBridge(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (data == NULL) || (entity == NULL))
		return -1;

	if (entity->info->type != type)
		return ERR_9006;	

	unsigned int object_num = 0, chain_id = 0;
	MIB_L2BRIDGE_GROUP_T *p, entry;
	p = &entry;

	object_num = getLayer2BrBridgeInstNum(name);

	if (!mib_chain_get(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, (object_num - 1), (void *)p))
		return ERR_9002;

	if (strcmp(lastname, "BridgeEnable") == 0)
	{
		int *i = data;
		int isRestartWAN = 0;

		if (i == NULL) return ERR_9007;
		CHECK_PARAM_NUM(*i, 0, 1);

		if (p->groupnum == 0) // default group is not permit to disable.
			return ERR_9007;

		if (p->enable != *i)
		{
			p->enable = (*i == 0) ? 0 : 1;

			mib_chain_update(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, (unsigned char *)p, (object_num - 1));

			if (p->enable == 0) // enable -> disable
			{
				isRestartWAN = setGroupEnableByGroupnum(p->groupnum, 0);
				update_itfList();
			}
		}

		if (isRestartWAN == CWMP_NEED_RESTART_WAN)
			return CWMP_NEED_RESTART_WAN;
	}
	else if (strcmp(lastname, "BridgeStandard") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "BridgeName") == 0)
	{
		char *buf = data;

		if(buf == NULL) return ERR_9007;
		if(strlen(buf) == 0) return ERR_9007;

		strncpy(p->name, buf, 64);
		mib_chain_update(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, (unsigned char *)p, (object_num - 1));
	}
	else if (strcmp(lastname, "VLANID") == 0)
	{
		unsigned int *i = data;
		int isRestartWAN = 0;

		if (i == NULL) return ERR_9007;
		CHECK_PARAM_NUM(*i, 0, 4094);

		if (p->groupnum == 0) // default group is not permit to set VLANID.
			return ERR_9007;

		if (p->vlanid != *i)
		{
			p->vlanid = *i;
#if 0 // not support vlan now
			isRestartWAN = setVLANByGroupnum(p->groupnum, 1, p->vlanid);
#endif
			mib_chain_update(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, (unsigned char *)p, (object_num - 1));
		}

		if (isRestartWAN == CWMP_NEED_RESTART_WAN)
			return CWMP_NEED_RESTART_WAN;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objLayer2BrBridgeOP(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	unsigned int i = 0, total = 0, chainid = 0;

	MIB_L2BRIDGE_GROUP_T *p, entry;
	p = &entry;

	update_itfList();

	switch(type)
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if ((name == NULL) || (entity == NULL) || (data == NULL))
				return -1;

			total = mib_chain_total(MIB_L2BRIDGING_BRIDGE_GROUP_TBL);
			for (i = 0; i < total; i++)
			{
				if (!mib_chain_get(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, i, (void *)p))
					continue;

				if (create_Object(c, tBridgeObject, sizeof(tBridgeObject), 1, (i + 1)) < 0)
					return -1;
			}

			add_objectNum(name, total);

			return 0;
		}
		case eCWMP_tADDOBJ:
		{
			int ret = 0;

			if ((name == NULL) || (entity == NULL) || (data == NULL))
				return -1;

			total = mib_chain_total(MIB_L2BRIDGING_BRIDGE_GROUP_TBL);
			if (total >= IFGROUP_NUM) 
				return ERR_9004;

			ret = add_Object(name, (struct CWMP_LINKNODE **)&entity->next,  tBridgeObject, sizeof(tBridgeObject), data);

			if (ret >= 0)
			{
				memset(p, 0, sizeof(MIB_L2BRIDGE_GROUP_T));
				p->enable = 0;
				p->groupnum = getUnusedGroupnum();
				sprintf(p->name, "Group%d", p->groupnum);
				p->vlanid = 0;
				mib_chain_add(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, (unsigned char *)p);
			}

			return ret;
		}
		case eCWMP_tDELOBJ:
		{
			unsigned int *pUint = data;
			int ret = 0, found = 0, ifnum = 0, isRestartWAN = 0;
			struct itfInfo itfs[MAX_NUM_OF_ITFS];

			total = mib_chain_total(MIB_L2BRIDGING_BRIDGE_GROUP_TBL);
			if(*pUint  > total)
				return ERR_9004;

			if (mib_chain_get(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, (*pUint) - 1, (void *)p))
			{
				if (p->groupnum == 0)
					return ERR_9005;

				found = 1;

				isRestartWAN = setGroupEnableByGroupnum(p->groupnum, 0);

				mib_chain_delete(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, (*pUint) - 1);
			}

			if (found == 0)
				return ERR_9005;

			ret = del_Object(name, (struct CWMP_LINKNODE **)&entity->next, *(int *)data);

			update_itfList();

			if (ret == 0 && isRestartWAN == CWMP_NEED_RESTART_WAN)
			{
				ret = CWMP_NEED_RESTART_WAN;
			}

			return ret;
		}
		case eCWMP_tUPDATEOBJ:
		{
			int i = 0, total = 0;
			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE*)entity->next;
			entity->next = NULL;

			total = mib_chain_total(MIB_L2BRIDGING_BRIDGE_GROUP_TBL);
			for (i = 0; i < total; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL;

				if (!mib_chain_get(MIB_L2BRIDGING_BRIDGE_GROUP_TBL, i, (void *)p))
					continue;

				remove_entity = remove_SiblingEntity( &old_table, (i + 1));

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					unsigned int MaxInstNum = (i + 1);
					add_Object(name, (struct CWMP_LINKNODE**)&entity->next, tBridgeObject, sizeof(tBridgeObject), &MaxInstNum);
				}
			}

			if (old_table)
			{
				destroy_ParameterTable((struct CWMP_NODE *)old_table);
			}
			return 0;
		}
	}
	return -1;
}


////////////////////////////////////////////////////////////////////////////////////////
/* InternetGatewayDevice.Layer2Bridging.Filter.{i}. */
int getLayer2BrFilter(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	if ((name == NULL) || (type == NULL) || (data == NULL) || (entity == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	char *lastname = entity->info->name;

	unsigned int object_num = 0;
	object_num = getLayer2BrFilterInstNum(name);

	if (object_num > AvailableInterfaceNum || object_num <= 0) return ERR_9005;

	if (strcmp(lastname, "FilterKey") == 0)
	{
		*data = uintdup(object_num);
	}
	else if (strcmp(lastname, "FilterEnable") == 0)
	{
		*data = booldup(1);
	}
	else if (strcmp(lastname, "FilterStatus") == 0)
	{
		*data = strdup("Enabled");
	}
	else if (strcmp(lastname, "FilterBridgeReference") == 0)
	{
		*data = intdup(itfList[object_num - 1].itfGroup);
	}
	else if (strcmp(lastname, "ExclusivityOrder") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "FilterInterface") == 0)
	{
		char intf[4] = {0};

		sprintf(intf, "%d", object_num);
		*data = strdup(intf);
	}
	else if (strcmp(lastname, "VLANIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "AdmitOnlyVLANTagged") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "EthertypeFilterList") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "EthertypeFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACAddressFilterList") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACAddressFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACAddressFilterList") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACAddressFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromVendorClassIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromVendorClassIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromVendorClassIDMode") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromVendorClassIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromVendorClassIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromVendorClassIDMode") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromClientIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromClientIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromClientIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromClientIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromUserClassIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromUserClassIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromUserClassIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromUserClassIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setLayer2BrFilter(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (data == NULL) || (entity == NULL))
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	unsigned int object_num = 0;
	object_num = getLayer2BrFilterInstNum(name);

	if (object_num > AvailableInterfaceNum || object_num <= 0) return ERR_9005;

	if (strcmp(lastname, "FilterEnable") == 0)
	{
		int *i = data;
		if (i == NULL) return ERR_9007;

		if (*i != 1) return ERR_9007;
	}
	else if (strcmp(lastname, "FilterBridgeReference") == 0)
	{
		int *i = data;
		if (i == NULL) return ERR_9007;

		int chainid = 0, total = 0, portindex = 0;
		char list[512] = {0};
		MIB_L2BRIDGE_GROUP_T *p, entry;
		p = &entry;

		if (getGroupByGroupnum(p, *i, &chainid) == -1) return ERR_9007;

		if (itfList[object_num - 1].itfGroup != *i)
		{
			setup_bridge_grouping(DEL_RULE);
#if 0 // not support vlan now
			setup_VLANonLAN(DEL_RULE);

			if (itfList[object_num - 1].ifdomain == DOMAIN_ELAN)
			{
				MIB_CE_SW_PORT_T swEntry;
				MIB_L2BRIDGE_GROUP_T *oldp, oldentry;
				oldp = &oldentry;

				getGroupByGroupnum(oldp, itfList[object_num - 1].itfGroup, &chainid);

				total = mib_chain_total(MIB_SW_PORT_TBL);
				if (itfList[object_num - 1].ifid < total)
					portindex = itfList[object_num - 1].ifid;
				else if (itfList[object_num - 1].ifid >= total && itfList[object_num - 1].ifid < (total * 2))
					portindex = itfList[object_num - 1].ifid - total;

				mib_chain_get(MIB_SW_PORT_TBL, portindex, &swEntry);

				if ((oldp->vlanid == 0 && p->vlanid == 0) || (oldp->vlanid != 0 && p->vlanid != 0))
				{
					snprintf(list, 64, "%u,", IF_ID(itfList[object_num - 1].ifdomain, itfList[object_num - 1].ifid));
					setgroup(list, *i);
				}
				else if (oldp->vlanid == 0 && p->vlanid != 0)
				{
					swEntry.itfGroup = 0;
					swEntry.vlan_on_lan_enabled = 1;
					swEntry.vlan_on_lan_itfGroup = *i;
					swEntry.vid = p->vlanid;
					mib_chain_update(MIB_SW_PORT_TBL, &swEntry, portindex);
				}
				else if (oldp->vlanid != 0 && p->vlanid == 0)
				{
					swEntry.itfGroup = *i;
					swEntry.vlan_on_lan_enabled = 0;
					swEntry.vlan_on_lan_itfGroup = 0;
					swEntry.vid = 0;
					mib_chain_update(MIB_SW_PORT_TBL, &swEntry, portindex);
				}
			}
			else
#endif
			{
				snprintf(list, 64, "%u,", IF_ID(itfList[object_num - 1].ifdomain, itfList[object_num - 1].ifid));
				setgroup(list, *i);
			}

#if 0 // not support vlan now
			setup_VLANonLAN(ADD_RULE);
#endif
			setup_bridge_grouping(ADD_RULE);
#ifdef CONFIG_RTK_RG_INIT // Rostelecom, Port Binding function
			unsigned int set_wanlist = 0;

			if (set_port_binding_mask(&set_wanlist) > 0)
			{
				rg_set_port_binding_mask(set_wanlist);
			}
#endif

#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif

			update_itfList();
		}
	}
	else if (strcmp(lastname, "ExclusivityOrder") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "FilterInterface") == 0)
	{
		int *i = data;
		if (i == NULL) return ERR_9007;

		if (*i != object_num) return ERR_9007;
	}
	else if (strcmp(lastname, "VLANIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "AdmitOnlyVLANTagged") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "EthertypeFilterList") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "EthertypeFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACAddressFilterList") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACAddressFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACAddressFilterList") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACAddressFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromVendorClassIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromVendorClassIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromVendorClassIDMode") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromVendorClassIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromVendorClassIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromVendorClassIDMode") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromClientIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromClientIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromClientIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromClientIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromUserClassIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "SourceMACFromUserClassIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromUserClassIDFilter") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "DestMACFromUserClassIDFilterExclude") == 0)
	{
		return ERR_9005;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objLayer2BrFilterOP(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;

	switch(type)
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if ((name == NULL) || (entity == NULL) || (data == NULL))
				return -1;
			
			update_itfList();
			if (create_Object(c, tFilterObject, sizeof(tFilterObject), AvailableInterfaceNum, 1) < 0)
				return -1;

			add_objectNum( name, AvailableInterfaceNum);

			return 0;
		}
		case eCWMP_tUPDATEOBJ:
		{
			int i = 0;
			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE*)entity->next;
			entity->next = NULL;

			update_itfList();
			for (i = 0; i < AvailableInterfaceNum; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL;

				remove_entity = remove_SiblingEntity(&old_table, (i + 1));

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{ 
					unsigned int InstNum = (i + 1);
					add_Object(name, (struct CWMP_LINKNODE**)&entity->next,  tFilterObject, sizeof(tFilterObject), &InstNum);
				}
			}

			if (old_table)
			{
				destroy_ParameterTable((struct CWMP_NODE *)old_table);	
			}
			return 0;
		}
	}
	return -1;
}


////////////////////////////////////////////////////////////////////////////////////////
/* InternetGatewayDevice.Layer2Bridging.Marking.{i}. */
int getLayer2BrMarking(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	if ((name == NULL) || (type == NULL) || (data == NULL) || (entity == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	char *lastname = entity->info->name;

	unsigned int object_num = 0;
	object_num = getLayer2BrMarkingInstNum(name);

	if (object_num > AvailableInterfaceNum || object_num <= 0) return ERR_9005;

	if (strcmp(lastname, "MarkingKey") == 0)
	{
		*data = uintdup(object_num);
	}
	else if (strcmp(lastname, "MarkingEnable") == 0)
	{
		int enabled = 0;

		if (itfList[object_num - 1].ifdomain == DOMAIN_WAN)
		{
			MIB_CE_ATM_VC_T wanEntry;
			int vcchainid = 0;

			getATMVCEntryandIDByIfindex(itfList[object_num - 1].ifid, &wanEntry, &vcchainid);

			if (wanEntry.vlan)
				enabled = 1;
		}

		*data = booldup(enabled);
	}
	else if (strcmp(lastname, "MarkingStatus") == 0)
	{
		*data = strdup("Enabled");
	}
	else if (strcmp(lastname, "MarkingBridgeReference") == 0)
	{
		*data = intdup(itfList[object_num - 1].itfGroup);
	}
	else if (strcmp(lastname, "MarkingInterface") == 0)
	{
		char intf[4] = {0};
		
		sprintf(intf, "%d", object_num);
		*data = strdup(intf);
	}
	else if (strcmp(lastname, "VLANIDUntag") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "VLANIDMark") == 0)
	{
		int vlanid = -1;

		if (itfList[object_num - 1].ifdomain == DOMAIN_WAN)
		{
			MIB_CE_ATM_VC_T wanEntry;
			int vcchainid = 0;

			getATMVCEntryandIDByIfindex(itfList[object_num - 1].ifid, &wanEntry, &vcchainid);

			if (wanEntry.vlan == 0)
				vlanid = -1;
			else
			{
				if (wanEntry.vid == 0)
					vlanid = -1;
				else
					vlanid = wanEntry.vid;
			}
		}

		*data = intdup(vlanid);
	}
	else if (strcmp(lastname, "VLANIDMarkOverride") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "EthernetPriorityMark") == 0)
	{
		int vlanpriority = -1;

		if (itfList[object_num - 1].ifdomain == DOMAIN_WAN)
		{
			MIB_CE_ATM_VC_T wanEntry;
			int vcchainid = 0;

			getATMVCEntryandIDByIfindex(itfList[object_num - 1].ifid, &wanEntry, &vcchainid);

			if (wanEntry.vlan == 0)
				vlanpriority = -1;
			else
			{
				if (wanEntry.vprio == 0)
					vlanpriority = -1;
				else
					vlanpriority = (wanEntry.vprio - 1);
			}
		}

		*data = intdup(vlanpriority);
	}
	else if (strcmp(lastname, "EthernetPriorityOverride") == 0)
	{
		return ERR_9005;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setLayer2BrMarking(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (data == NULL) || (entity == NULL))
		return -1;

	if (entity->info->type != type)
		return ERR_9006;	

	unsigned int object_num = 0;
	object_num = getLayer2BrMarkingInstNum(name);

	if (object_num > AvailableInterfaceNum || object_num <= 0) return ERR_9005;

	if (strcmp(lastname, "MarkingEnable") == 0)
	{
		int *i = data;
		if (i == NULL) return ERR_9007;
		CHECK_PARAM_NUM(*i, 0, 1);

		if (itfList[object_num - 1].ifdomain == DOMAIN_WAN)
		{
			MIB_CE_ATM_VC_T wanEntry;
			int vcchainid = 0;

			getATMVCEntryandIDByIfindex(itfList[object_num - 1].ifid, &wanEntry, &vcchainid);

			if (wanEntry.vlan != *i)
			{
				wanEntry.vlan = *i;
				if (*i == 0)
				{
					wanEntry.vid = 0;
					wanEntry.vprio = 0;
				}

				mib_chain_update(MIB_ATM_VC_TBL, (void *)&wanEntry, vcchainid);
				return CWMP_NEED_RESTART_WAN;
			}
		}
		else
		{
			if (*i == 1) return ERR_9007;
		}
	}
	else if (strcmp(lastname, "MarkingBridgeReference") == 0)
	{
		int *i = data;
		if (i == NULL) return ERR_9007;

		int chainid = 0, total = 0, portindex = 0;
		char list[512] = {0};
		MIB_L2BRIDGE_GROUP_T *p, entry;
		p = &entry;

		if (getGroupByGroupnum(p, *i, &chainid) == -1) return ERR_9007;

		if (itfList[object_num - 1].itfGroup != *i)
		{
			setup_bridge_grouping(DEL_RULE);
#if 0 // not support vlan now
			//setup_VLANonLAN(DEL_RULE);

			if (itfList[object_num - 1].ifdomain == DOMAIN_ELAN)
			{
				MIB_CE_SW_PORT_T swEntry;
				MIB_L2BRIDGE_GROUP_T *oldp, oldentry;
				oldp = &oldentry;

				getGroupByGroupnum(oldp, itfList[object_num - 1].itfGroup, &chainid);

				total = mib_chain_total(MIB_SW_PORT_TBL);
				if (itfList[object_num - 1].ifid < total)
					portindex = itfList[object_num - 1].ifid;
				else if (itfList[object_num - 1].ifid >= total && itfList[object_num - 1].ifid < (total * 2))
					portindex = itfList[object_num - 1].ifid - total;

				mib_chain_get(MIB_SW_PORT_TBL, portindex, &swEntry);

				if ((oldp->vlanid == 0 && p->vlanid == 0) || (oldp->vlanid != 0 && p->vlanid != 0))
				{
					snprintf(list, 64, "%u,", IF_ID(itfList[object_num - 1].ifdomain, itfList[object_num - 1].ifid));
					setgroup(list, *i);
				}
				else if (oldp->vlanid == 0 && p->vlanid != 0)
				{
					swEntry.itfGroup = 0;
					swEntry.vlan_on_lan_enabled = 1;
					swEntry.vlan_on_lan_itfGroup = *i;
					swEntry.vid = p->vlanid;
					mib_chain_update(MIB_SW_PORT_TBL, &swEntry, portindex);
				}
				else if (oldp->vlanid != 0 && p->vlanid == 0)
				{
					swEntry.itfGroup = *i;
					swEntry.vlan_on_lan_enabled = 0;
					swEntry.vlan_on_lan_itfGroup = 0;
					swEntry.vid = 0;
					mib_chain_update(MIB_SW_PORT_TBL, &swEntry, portindex);
				}
			}
			else
#endif
			{
				snprintf(list, 64, "%u,", IF_ID(itfList[object_num - 1].ifdomain, itfList[object_num - 1].ifid));
				setgroup(list, *i);
			}

#if 0 // not support vlan now
			setup_VLANonLAN(ADD_RULE);
#endif
			setup_bridge_grouping(ADD_RULE);
#ifdef CONFIG_RTK_RG_INIT // Rostelecom, Port Binding function
			unsigned int set_wanlist = 0;

			if (set_port_binding_mask(&set_wanlist) > 0)
			{
				rg_set_port_binding_mask(set_wanlist);
			}
#endif

#ifdef COMMIT_IMMEDIATELY
			Commit();
#endif

			update_itfList();
		}
	}
	else if (strcmp(lastname, "MarkingInterface") == 0)
	{
		int *i = data;
		if (i == NULL) return ERR_9007;

		if (*i != object_num) return ERR_9007;
	}
	else if (strcmp(lastname, "VLANIDUntag") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "VLANIDMark") == 0)
	{
		int *i = data;
		if (i == NULL) return ERR_9007;
		if (*i == 0 || *i > 4094 || *i < -1) return ERR_9007;

		if (itfList[object_num - 1].ifdomain == DOMAIN_WAN)
		{
			MIB_CE_ATM_VC_T wanEntry;
			int vcchainid = 0;

			getATMVCEntryandIDByIfindex(itfList[object_num - 1].ifid, &wanEntry, &vcchainid);

			if (wanEntry.vid != *i)
			{
				if (*i == -1)
				{
					wanEntry.vlan = 0;
					wanEntry.vid = 0;
				}
				else
				{
					wanEntry.vid = *i;
				}

				mib_chain_update(MIB_ATM_VC_TBL, (void *)&wanEntry, vcchainid);
				return CWMP_NEED_RESTART_WAN;
			}
		}
		else
		{
			if (*i != -1)
				return ERR_9007;
		}
	}
	else if (strcmp(lastname, "VLANIDMarkOverride") == 0)
	{
		return ERR_9005;
	}
	else if (strcmp(lastname, "EthernetPriorityMark") == 0)
	{
		int *i = data;
		if (i == NULL) return ERR_9007;
		if (*i > 7 || *i < -1) return ERR_9007;

		if (itfList[object_num - 1].ifdomain == DOMAIN_WAN)
		{
			MIB_CE_ATM_VC_T wanEntry;
			int vcchainid = 0;

			getATMVCEntryandIDByIfindex(itfList[object_num - 1].ifid, &wanEntry, &vcchainid);

			if (wanEntry.vid != *i)
			{
				if (*i == -1)
				{
					wanEntry.vprio = 0;
				}
				else
				{
					wanEntry.vprio = *i + 1;
				}

				mib_chain_update(MIB_ATM_VC_TBL, (void *)&wanEntry, vcchainid);
				return CWMP_NEED_RESTART_WAN;
			}
		}
		else
		{
			if (*i != -1)
				return ERR_9007;
		}
	}
	else if (strcmp(lastname, "EthernetPriorityOverride") == 0)
	{
		return ERR_9005;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int objLayer2BrMarkingOP(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;

	switch(type)
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if ((name == NULL) || (entity == NULL) || (data == NULL))
				return -1;
			
			update_itfList();
			if (create_Object(c, tMarkingObject, sizeof(tMarkingObject), AvailableInterfaceNum, 1) < 0)
				return -1;

			add_objectNum( name, AvailableInterfaceNum);

			return 0;
		}
		case eCWMP_tUPDATEOBJ:
		{
			int i = 0;
			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE*)entity->next;
			entity->next = NULL;

			update_itfList();
			for (i = 0; i < AvailableInterfaceNum; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL;

				remove_entity = remove_SiblingEntity(&old_table, (i + 1));

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{ 
					unsigned int InstNum = (i + 1);
					add_Object(name, (struct CWMP_LINKNODE**)&entity->next,  tMarkingObject, sizeof(tMarkingObject), &InstNum);
				}
			}

			if (old_table)
			{
				destroy_ParameterTable((struct CWMP_NODE *)old_table);	
			}
			return 0;
		}
	}
	return -1;
}


////////////////////////////////////////////////////////////////////////////////////////
/* InternetGatewayDevice.Layer2Bridging.AvailableInterface.{i}. */
int getLayer2BrAvailableInterface(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	if ((name == NULL) || (type == NULL) || (data == NULL) || (entity == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	char *lastname = entity->info->name;

	unsigned int object_num = 0;
	object_num = getLayer2BrAvailableInterfaceInstNum(name);

	if (strcmp(lastname, "AvailableInterfaceKey") == 0)
	{
		*data = uintdup(object_num);
	}
	else if (strcmp(lastname, "InterfaceType") == 0)
	{
		if (itfList[object_num - 1].ifdomain == DOMAIN_ELAN
			|| itfList[object_num - 1].ifdomain == DOMAIN_WLAN
			|| itfList[object_num - 1].ifdomain == DOMAIN_ULAN)
		{
			*data = strdup("LANInterface");
		}
		else
		{
			*data = strdup("WANInterface");
		}
	}
	else if (strcmp(lastname, "InterfaceReference") == 0)
	{
		char itfName[256] = {0};
		if (itfList[object_num - 1].ifdomain == DOMAIN_ELAN)
		{
			int swNum = mib_chain_total(MIB_SW_PORT_TBL);
			snprintf(itfName, sizeof(itfName), "InternetGatewayDevice.LANDevice.1.LANEthernetInterfaceConfig.%d",
				(itfList[object_num - 1].ifid >= swNum && itfList[object_num - 1].ifid < (swNum * 2))?((itfList[object_num - 1].ifid - swNum) + 1):(itfList[object_num - 1].ifid + 1));
		}
		else if (itfList[object_num - 1].ifdomain == DOMAIN_WLAN)
		{
			if (itfList[object_num - 1].ifid < 10)
				snprintf(itfName, sizeof(itfName), "InternetGatewayDevice.LANDevice.1.WLANConfiguration.%d", itfList[object_num - 1].ifid + 1);
			else
				snprintf(itfName, sizeof(itfName), "InternetGatewayDevice.LANDevice.1.WLANConfiguration.%d", itfList[object_num - 1].ifid - 4);
		}
		else if (itfList[object_num - 1].ifdomain == DOMAIN_ULAN)
		{
			snprintf(itfName, sizeof(itfName), "InternetGatewayDevice.LANDevice.1.LANUSBInterfaceConfig.%d", itfList[object_num - 1].ifid + 1);
		}
		else
		{
			update_WANDEVNUM();
			unsigned int wanInst = 0;
			int vcchainid = 0;
			MIB_CE_ATM_VC_T wanEntry;

			if (MEDIA_INDEX(itfList[object_num - 1].ifid) == MEDIA_ETH)
				wanInst = WANDEVNUM_ETH;
			else if (MEDIA_INDEX(itfList[object_num - 1].ifid) == MEDIA_PTM)
				wanInst = WANDEVNUM_PTM;
			else //default
				wanInst = WANDEVNUM_ATM;

			getATMVCEntryandIDByIfindex(itfList[object_num - 1].ifid, &wanEntry, &vcchainid);

			snprintf(itfName, sizeof(itfName), "InternetGatewayDevice.WANDevice.%d.WANConnectionDevice.%d", wanInst, wanEntry.ConDevInstNum);
		}

		*data = strdup(itfName);
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int objLayer2BrAvailableInterfaceOP(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;

	switch(type)
	{
		case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if ((name == NULL) || (entity == NULL) || (data == NULL))
				return -1;

			update_itfList();
			if(create_Object(c, tAvailableInterfaceObject, sizeof(tAvailableInterfaceObject), AvailableInterfaceNum, 1) < 0)
				return -1;

			add_objectNum(name, AvailableInterfaceNum);

			return 0;
		}
			
		case eCWMP_tUPDATEOBJ:
		{
			int i = 0;
			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			update_itfList();
			for (i = 0; i < AvailableInterfaceNum; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL;

				remove_entity = remove_SiblingEntity(&old_table, (i + 1));

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{ 
					unsigned int InstNum = (i + 1);
					add_Object(name, (struct CWMP_LINKNODE**)&entity->next, tAvailableInterfaceObject, sizeof(tAvailableInterfaceObject), &InstNum);
				}
			}

			if (old_table)
			{
				destroy_ParameterTable((struct CWMP_NODE *)old_table);
			}
			return 0;
		}
	}
	return -1;
}


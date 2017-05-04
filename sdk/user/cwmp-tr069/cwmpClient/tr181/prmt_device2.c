//TR-181 issue 2 parameters
#include "prmt_device2.h"
#include "prmt_deviceinfo.h"
#include "prmt_ifstack.h"
#include "prmt_dns.h"
#include "prmt_nat.h"
#include "prmt_dhcpv4.h"
#ifdef CONFIG_DEV_xDSL
#include "prmt_dsl.h"
#include "prmt_atm.h"
#endif
#ifdef CONFIG_PTMWAN
#include "prmt_ptm.h"
#endif
#ifdef CONFIG_WIRELESS
#include "prmt_wifi.h"
#endif
#include "prmt_bridge.h"
#include "prmt_ppp.h"
#include "prmt_ip.h"
#include "prmt_ether.h"
#include "prmt_routing.h"

//Old parameters
#include "../prmt_mngmtserver.h"
#include "../prmt_time.h"
#include "../prmt_lanconfsec.h"
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
#include "../prmt_captiveportal.h"
#endif

/****** Device. **************************************************************/
struct CWMP_OP tDeviceLeafOP = { getDevice, NULL};
struct CWMP_PRMT tDeviceLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"InterfaceStackNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tDeviceLeafOP},	//number of InterfaceStack Table entry
};
enum eDeviceLeaf
{
	eInterfaceStackNumberOfEntries,
};
struct CWMP_LEAF tDeviceLeaf[] =
{
{ &tDeviceLeafInfo[eInterfaceStackNumberOfEntries]  },
{ NULL	}
};

struct CWMP_PRMT tDeviceObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"DeviceInfo",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"ManagementServer",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"Time",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"InterfaceStack",	eCWMP_tOBJECT,	CWMP_READ,	&tIfStackEntityOP},
#ifdef CONFIG_DEV_xDSL
{"DSL",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"ATM", 	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef CONFIG_PTMWAN
{"PTM", 	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
{"Ethernet",eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef WLAN_SUPPORT
{"WiFi",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
{"Bridging",eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"PPP",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"IP", 		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
{"CaptivePortal",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
{"Routing",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"LANConfigSecurity",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"DNS",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"NAT",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"DHCPv4", 	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eDeviceObject
{
	eDeviceInfo,
	eManagementServer,
	eTime,
	eInterfaceStack,
#ifdef CONFIG_DEV_xDSL
	eDSL,
	eATM,
#endif
#ifdef CONFIG_PTMWAN
	ePTM,
#endif
	eEthernet,
#ifdef WLAN_SUPPORT
	eWiFi,
#endif
	eBridging,
	ePPP,
	eIP,
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
	eCaptivePortal,
#endif
	eRouting,
	eLANConfigSecurity,
	eDNS,
	eNAT,
	eDHCPv4,
};

struct CWMP_NODE tDeviceObject[] =
{
/*info,  				leaf,			next)*/
{&tDeviceObjectInfo[eDeviceInfo],		tTR181DeviceInfoLeaf,	tTR181DeviceInfoObject},
{&tDeviceObjectInfo[eManagementServer],	tManagementServerLeaf,	tManagementServerObject},
{&tDeviceObjectInfo[eTime], tTimeLeaf,	NULL},
{&tDeviceObjectInfo[eInterfaceStack], NULL,	NULL},
#ifdef CONFIG_DEV_xDSL
{&tDeviceObjectInfo[eDSL], tDSLLeaf, tDSLObject},
{&tDeviceObjectInfo[eATM], tATMLeaf, tATMObject},
#endif
#ifdef CONFIG_PTMWAN
{&tDeviceObjectInfo[ePTM], tPTMLeaf, tPTMObject},
#endif
{&tDeviceObjectInfo[eEthernet], tEtherLeaf, tEtherObject},
#ifdef WLAN_SUPPORT
{&tDeviceObjectInfo[eWiFi], tWiFiLeaf, tWiFiObject},
#endif
{&tDeviceObjectInfo[eBridging], tBridgingLeaf, tBridgingObject},
{&tDeviceObjectInfo[ePPP], tPPPLeaf, tPPPObject},
{&tDeviceObjectInfo[eIP], tIPLeaf, tIPObject},
#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
{&tDeviceObjectInfo[eCaptivePortal], tCaptivePortalLeaf, NULL},
#endif
{&tDeviceObjectInfo[eRouting], NULL, tRoutingObject},
{&tDeviceObjectInfo[eLANConfigSecurity], tLANConfigSecurityLeaf, NULL},
{&tDeviceObjectInfo[eDNS], NULL, tDNSObject},
{&tDeviceObjectInfo[eNAT], tNATLeaf, tNATObject},
{&tDeviceObjectInfo[eDHCPv4], tDHCPv4Leaf, tDHCPv4Object},
{NULL,					NULL,			NULL}
};

/****** Root *****************************************************************/
struct CWMP_PRMT device_tROOTObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Device",	eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eROOTObject
{
	eDevice
};
struct CWMP_NODE device_tROOT[] =
{
/*info, 	 				leaf,			next*/
{&device_tROOTObjectInfo[eDevice],	tDeviceLeaf,		tDeviceObject	},
{NULL,						NULL,			NULL		}
};

/*****************************************************************************/
int getDevice(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "InterfaceStackNumberOfEntries" )==0 )
	{
		*data = uintdup(ParameterEntityCount((struct CWMP_LEAF *)gIfStackObjList));
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

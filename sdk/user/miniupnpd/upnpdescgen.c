/* $Id: upnpdescgen.c,v 1.15 2012/08/01 04:27:13 adsmt Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2008 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "upnpdescgen.h"
#include "miniupnpdpath.h"
#include "upnpglobalvars.h"
#include "upnpdescstrings.h"

#ifdef CONFIG_TR_064
bool IS_TR064_ENABLED;
#endif

static const char *const upnptypes_tr064[] =
{
	"string",
	"boolean",
// For current version of Intel Device Spy (1.3.2777.23551), you must set all types to string for compatibility.
// But for Windows XP Internet Gateway Device, they should be kept in original types.
#ifdef CONFIG_TR_064
#ifdef DEVICE_SPY_COMPATIBILITY
		"string",
		"string",
		"string",
		"string",    // Index 5
		"string",
		"string"
#else
		"ui2",
		"ui4",
		"uuid",
		"DateTime",    // Index 5
		"i4",
		"ui1"
#endif
#endif
};

static const char *const upnptypes_igd[] =
{
	"string",
	"boolean",
	"ui2",
	"ui4"
};

static const char * const upnpdefaultvalues[] =
{
	0,
	"Unconfigured"
};

static const char * const upnpallowedvalues[] =
{
	0,		/* 0 */
	"DSL",	/* 1 */
	"POTS",
	"Cable",
	"Ethernet",
	0,
	"Up",	/* 6 */
	"Down",
	"Initializing",
	"Unavailable",
	0,
	"TCP",	/* 11 */
	"UDP",
	0,
	"Unconfigured",	/* 14 */
	"IP_Routed",
	"IP_Bridged",
	0,
	"Unconfigured",	/* 18 */
	"Connecting",
	"Connected",
	"PendingDisconnect",
	"Disconnecting",
	"Disconnected",
	0,
	"ERROR_NONE",	/* 25 */
	0,

	"RebootRequired",		/* 27 */
	"ChangesApplied",
	0,
	"Disabled",     // 30
	"Enabled",
	"Error",
	0,
	"Default",     // 34
	"Network",
	"Host",
	0,
	"Normal",     // 38
	"UseAllocatedS",
	"ubnet",
	"Passthrough",
	0,
	"DHCP",     // 43
	"Static",
	"AutoIP",
	0,
	"Up",    // 47
	"NoLink",
	"Error",
	"Disabled",
	0,
	"10",    // 52
	"100",
	"1000",
	"Auto",
	0,
	"Half",   //57
	"Full",
	"Auto",
	0,
	"Up",     // 61
	"Error",
	"Disabled",
	0,
	"a",       // 65
	"b",
	"g",
	0,
	"None",    // 69
	"WEPEncryption",
	0,
	"None",       // 72
	"EAPAuthentication",    // Optional
	0,
	"WEPEncryption",   // 75
	"TKIPEncryption",
	"WEPandTKIPEncryption",
	"AESEncryption",                      // Optional
	"WEPandAESEncryption",       // Optional
	"TKIPandAESEncryption",       // Optional
	"WEPandTKIPandAESEncryption",  // Optional
	0,
	"PSKAuthentication",    //83
	"EAPAuthentication",    // Optional
	0,
	"WEPEncryption",    // 86
	"TKIPEncryption",
	"WEPandTKIPEncryption",
	"AESEncryption",                             // Optional
	"WEPandAESEncrytion",                 // Optional
	"TKIPandAESEncryption",               // Optional
	"WEPandTKIPandAESEncryption",   // Optional
	0,
	"PSKAuthentication",    // 94
	"EAPAuthentication",
	"EAPandPSKAuthentication",
	0,
	"Ethernet",       // 98
	"USB",
	"802.11",
	"HomePNA",
	"HomePlug",
	"Other",
	0,
	"PVC: VPI/VCI",   // 105
	"SVC: ATM connection name",
	"SVC: ATM address",
	0,
	"Up",              // 109
	"Initializing",
	"EstablishingLink",
	"NoSignal",
	"Error",
	"Disabled",
	0,
	"ADSL_G.dmt",     // 116
	"ADSL_G.lite",
	"ADSL_G.dmt.bis",
	"ADSL_re-adsl",
	"ADSL_2plus",
	"ADSL_four",
	"ADSL_ANSI_T1.413",
	"G.shdsl",
	"IDSL",
	"HDSL",
	"SDSL",
	"VDSL",
	0,
	"DMT",        // 129
	"CAP",
	"2B1Q",
	"43BT",
	"PAM",
	"QAM",
	0,
	"Interleaved",      // 136
	"Fast",
	0,
	"EoA",     // 139
	"IPoA",
	"PPPoA",
	"CIP",
	"PPPoE",
	"Unconfigured",
	0,
	"LLC",          // 146
	"VCMUX",
	0,
	"AAL1",     // 149
	"AAL2",
	"AAL3",
	"AAL4",
	"AAL5",
	0,
	"UBR",      // 155
	"CBR",
	"GFR",
	"VBR-nrt",
	"VBR-rt",
	"UBR+",
	"ABR",
	0,
	"Unconfigured",       // 163
	"IP_Routed",
	"DHCP_Spoofed",
	"PPPoE_Bridged",
	"PPTP_Relay",
	"L2TP_Relay",
	"PPPoE_Relay",
	0,
	"Unconfigured",     // 171
	"Connecting",
	"Authenticating",
	"Connected",
	"PendingDisconnect",
	"Disconnecting",
	"Disconnected",
	0,
	"ERROR_NONE",              // 179
	"ERROR_ISP_TIME_OUT",                   // Optional
	"ERROR COMMAND ABORTED",           // Optional
	"ERROR_NOT_ENABLED_FOR_INTERNET",     // Optional
	"ERROR_BAD_PHONE_NUMBER",                   // Optional
	"ERROR_USER_DISCONNECT",                      // Optional
	"ERROR_ISP_DISCONNECT",                         // Optional
	"ERROR_IDLE_DISCONNECT",                       // Optional
	"ERROR_FORCED_DISCONNECT",                 // Optional
	"ERROR_SERVER_OUT_OF_RESOURCES",    // Optional
	"ERROR_RESTRICTED_LOGON_HOURS",       // Optional
	"ERROR_ACCOUNT_DISABLED",               // Optional
	"ERROR_ACCOUNT_EXPIRED",                // Optional
	"ERROR_PASSWORD_EXPIRED",             // Optional
	"ERROR_AUTHENTICATION_FAILURE",
	"ERROR_NO_DIALTONE",                       // Optional
	"ERROR_NO_CARRIER",                          // Optional
	"ERROR_NO_ANSWER",                          // Optional
	"ERROR_LINE_BUSY",                             // Optional
	"ERROR_UNSUPPORTED_BITSPERSECOND",      // Optional
	"ERROR_TOO_MANY_LINE_ERRORS",                // Optional
	"ERROR_IP_CONFIGURATION",
	"ERROR_UNKNOWN",
	0,
	"OnDemand",    // 203
	"AlwaysOn",
	"Manual",
	0,
	"STAC LZS",      // 207
	"Van Jacobsen",
	0,
	"PAP",            // 210
	"CHAP",
	"MS-CHAP",
	0,
	"PPPoA",     // 214
	"PPPoE",
	"L2TP",
	"PPTP",
	0,
	"Off",        // 219
	"RIPv1",   //Optional
	"RIPv2",   //Optional
	"OSPF",   //Optional
	0,
	"",      // 224
	0
};

static const char xmlver[] = "<?xml version=\"1.0\"?>\n";
#if 0 //def TR_064  // For WIndows XP Internet Gateway Device compatibility, we cannot use "dslforum-org" string as root service string.
static const char root_service[] = "scpd xmlns=\"urn:dslforum-org:service-1-0\"";
static const char root_device[] = "root xmlns=\"urn:dslforum-org:device-1-0\"";
#else
static const char root_service[] = "scpd xmlns=\"urn:schemas-upnp-org:service-1-0\"";
static const char root_device[] = "root xmlns=\"urn:schemas-upnp-org:device-1-0\"";
#endif
char URL_BASE[64] = "http://192.168.1.1:49152";

/* root Description of the UPnP Device
 * fixed to match UPnP_IGD_InternetGatewayDevice 1.0.pdf */
static const struct XMLElt rootDesc[] =
{
#if 0
/* 0 */
	{root_device, INITHELPER(1, 3)},
	{"specVersion", INITHELPER(4, 2)},
	{"/URLBase", URL_BASE},
	{"device", INITHELPER(6,14)},
	{"/major", "1"},
	{"/minor", "0"},
/* 6 */
	{"/deviceType", "urn:schemas-upnp-org:device:InternetGatewayDevice:1"},
	{"/friendlyName", ROOTDEV_FRIENDLYNAME},	/* required */
	{"/manufacturer", ROOTDEV_MANUFACTURER},		/* required */
/* 9 */
	{"/manufacturerURL", ROOTDEV_MANUFACTURERURL},	/* optional */
	{"/modelDescription", ROOTDEV_MODELDESCRIPTION}, /* recommended */
	{"/modelName", ROOTDEV_MODELNAME},	/* required */
	{"/modelNumber", "ADSL Modem" /*modelnumber*/},
	{"/modelURL", ROOTDEV_MODELURL},
	{"/serialNumber", serialnumber},
	{"/UDN", uuidvalue},	/* required */
	{"/UPC", "000000000001"},
	{"serviceList", INITHELPER(65,2)},
	{"deviceList", INITHELPER(20,1)},
	{"/presentationURL", presentationurl},	/* recommended */
/* 20 */
	{"device", INITHELPER(21,13)},
/* 21 */
	{"/deviceType", "urn:schemas-upnp-org:device:WANDevice:1"}, /* required */
	{"/friendlyName", WANDEV_FRIENDLYNAME},
	{"/manufacturer", WANDEV_MANUFACTURER},
	{"/manufacturerURL", WANDEV_MANUFACTURERURL},
	{"/modelDescription" , WANDEV_MODELDESCRIPTION},
	{"/modelName", WANDEV_MODELNAME},
	{"/modelNumber", WANDEV_MODELNUMBER},
	{"/modelURL", WANDEV_MODELURL},
	{"/serialNumber", serialnumber},
	{"/UDN", WANDEV_UDN},
	{"/UPC", WANDEV_UPC},
/* 32 */
	{"serviceList", INITHELPER(34,1)},
	{"deviceList", INITHELPER(40,1)},
/* 34 */
	{"service", INITHELPER(35,5)},
/* 35 */
	{"/serviceType",	"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1"},
	/*{"/serviceId", "urn:upnp-org:serviceId:WANCommonInterfaceConfig"}, */
	{"/serviceId", "urn:upnp-org:serviceId:WANCommonIFC1"}, /* required */
	{"/SCPDURL", WANCFG_PATH},
	{"/controlURL", WANCFG_CONTROLURL},
	{"/eventSubURL", WANCFG_EVENTURL},
/* 40 */
	{"device", INITHELPER(41,12)},
/* 41 */
	{"/deviceType", "urn:schemas-upnp-org:device:WANConnectionDevice:1"},
	{"/friendlyName", WANCDEV_FRIENDLYNAME},
	{"/manufacturer", WANCDEV_MANUFACTURER},
	{"/manufacturerURL", WANCDEV_MANUFACTURERURL},
	{"/modelDescription", WANCDEV_MODELDESCRIPTION},
	{"/modelName", WANCDEV_MODELNAME},
	{"/modelNumber", WANCDEV_MODELNUMBER},
	{"/modelURL", WANCDEV_MODELURL},
	{"/serialNumber", serialnumber},
	{"/UDN", WANCDEV_UDN},
	{"/UPC", WANCDEV_UPC},
	{"serviceList", INITHELPER(53,2)},
/* 53 */
	{"service", INITHELPER(55,5)},
	{"service", INITHELPER(60,5)},
/* 55 */
	{"/serviceType", "urn:schemas-upnp-org:service:WANDSLLinkConfig:1"},
	{"/serviceId", "urn:upnp-org:serviceId:WANDSLLinkC1"},
	{"/SCPDURL", WANDSLLINK_PATH},
	{"/controlURL", WANDSLLINK_CONTROLURL},
	{"/eventSubURL", WANDSLLINK_EVENTURL},
/* 60 */
	{"/serviceType", "urn:schemas-upnp-org:service:WANIPConnection:1"},
	/* {"/serviceId", "urn:upnp-org:serviceId:WANIPConnection"}, */
	{"/serviceId", "urn:upnp-org:serviceId:WANIPConn1"},
	{"/SCPDURL", WANIPC_PATH},
	{"/controlURL", WANIPC_CONTROLURL},
	{"/eventSubURL", WANIPC_EVENTURL},
/* 65 */
	{"service", INITHELPER(67,5)},
	{"service", INITHELPER(72,5)},
/* 67 */
	{"/serviceType", "urn:schemas-upnp-org:service:Layer3Forwarding:1"},
	{"/serviceId", "urn:upnp-org:serviceId:L3Forwarding1"},
	{"/SCPDURL", L3F_PATH},
	{"/controlURL", L3F_CONTROLURL}, /* The Layer3Forwarding service is only */
	{"/eventSubURL", L3F_EVENTURL}, /* recommended, not mandatory */
/* 72 */
	{"/serviceType", "urn:schemas-upnp-org:service:DeviceInfo:1"},
	{"/serviceId", "urn:upnp-org:serviceId:DeviceInfo1"},
	{"/SCPDURL", DEVICEINFO_PATH},
	{"/controlURL", "/ctl/DevInfo"},
	{"/eventSubURL", "/evt/DevInfo"},
#endif
/* 0 */
	{root_device, INITHELPER(1, 3)},
	{"specVersion", INITHELPER(4, 2)},
	{"/URLBase", URL_BASE},
#if defined(ENABLE_L3F_SERVICE) || defined(HAS_DUMMY_SERVICE)
	{"device", INITHELPER(6,14)},
#else
	{"device", INITHELPER(6,13)},
#endif
	{"/major", "1"},
	{"/minor", "0"},
/* 6 */
	{"/deviceType", "urn:schemas-upnp-org:device:InternetGatewayDevice:1"},
	{"/friendlyName", ROOTDEV_FRIENDLYNAME},	/* required */
	{"/manufacturer", ROOTDEV_MANUFACTURER},		/* required */
/* 9 */
	{"/manufacturerURL", ROOTDEV_MANUFACTURERURL},	/* optional */
	{"/modelDescription", ROOTDEV_MODELDESCRIPTION}, /* recommended */
	{"/modelName", ROOTDEV_MODELNAME},	/* required */
	{"/modelNumber", ROOTDEV_MODELNUMBER /*modelnumber*/},
	{"/modelURL", ROOTDEV_MODELURL},
	{"/serialNumber", serialnumber},
	{"/UDN", uuidvalue},	/* required */
	{"/UPC", ROOTDEV_UPC},
#if defined(ENABLE_L3F_SERVICE) || defined(HAS_DUMMY_SERVICE)
	{"serviceList", INITHELPER(65,1)},
	{"deviceList", INITHELPER(20,1)},
	{"/presentationURL", presentationurl},	/* recommended */
#else
	{"deviceList", INITHELPER(20,1)},
	{"/presentationURL", presentationurl},	/* recommended */
	{0,0},
#endif
/* 20 */
	{"device", INITHELPER(21,13)},
/* 21 */
	{"/deviceType", "urn:schemas-upnp-org:device:WANDevice:1"}, /* required */
	{"/friendlyName", WANDEV_FRIENDLYNAME},
	{"/manufacturer", WANDEV_MANUFACTURER},
	{"/manufacturerURL", WANDEV_MANUFACTURERURL},
	{"/modelDescription" , WANDEV_MODELDESCRIPTION},
	{"/modelName", WANDEV_MODELNAME},
	{"/modelNumber", WANDEV_MODELNUMBER},
	{"/modelURL", WANDEV_MODELURL},
	{"/serialNumber", serialnumber},
	{"/UDN", WANDEV_UDN},
	{"/UPC", WANDEV_UPC},
/* 32 */
	{"serviceList", INITHELPER(34,1)},
	{"deviceList", INITHELPER(40,1)},
/* 34 */
	{"service", INITHELPER(35,5)},
/* 35 */
	{"/serviceType",	"urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1"},
	/*{"/serviceId", "urn:upnp-org:serviceId:WANCommonInterfaceConfig"}, */
	{"/serviceId", "urn:upnp-org:serviceId:WANCommonIFC1"}, /* required */
	{"/SCPDURL", WANCFG_PATH},
	{"/controlURL", WANCFG_CONTROLURL},
	{"/eventSubURL", WANCFG_EVENTURL},
/* 40 */
	{"device", INITHELPER(41,12)},
/* 41 */
	{"/deviceType", "urn:schemas-upnp-org:device:WANConnectionDevice:1"},
	{"/friendlyName", WANCDEV_FRIENDLYNAME},
	{"/manufacturer", WANCDEV_MANUFACTURER},
	{"/manufacturerURL", WANCDEV_MANUFACTURERURL},
	{"/modelDescription", WANCDEV_MODELDESCRIPTION},
	{"/modelName", WANCDEV_MODELNAME},
	{"/modelNumber", WANCDEV_MODELNUMBER},
	{"/modelURL", WANCDEV_MODELURL},
	{"/serialNumber", serialnumber},
	{"/UDN", WANCDEV_UDN},
	{"/UPC", WANCDEV_UPC},
	{"serviceList", INITHELPER(53,2)},
/* 53 */
	{"service", INITHELPER(55,5)},
	{"service", INITHELPER(60,5)},
/* 55 */
	{"/serviceType", "urn:schemas-upnp-org:service:WANDSLLinkConfig:1"},
	{"/serviceId", "urn:upnp-org:serviceId:WANDSLLinkC1"},
	{"/SCPDURL", WANDSLLINKCONFIG_PATH},
	{"/controlURL", WANDSLLINKCONFIG_CONTROLURL},
	{"/eventSubURL", WANDSLLINKCONFIG_EVENTURL},
/* 60 */
	{"/serviceType", "urn:schemas-upnp-org:service:WANIPConnection:1"},
	/* {"/serviceId", "urn:upnp-org:serviceId:WANIPConnection"}, */
	{"/serviceId", "urn:upnp-org:serviceId:WANIPConn1"},
	{"/SCPDURL", WANIPC_PATH},
	{"/controlURL", WANIPC_CONTROLURL},
	{"/eventSubURL", WANIPC_EVENTURL},
/* 65 */
#ifdef HAS_DUMMY_SERVICE
	{"service", INITHELPER(66,5)},
/* 66 */
	{"/serviceType", "urn:schemas-dummy-com:service:Dummy:1"},
	{"/serviceId", "urn:dummy-com:serviceId:dummy1"},
	{"/controlURL", "/dummy"},
	{"/eventSubURL", "/dummy"},
	{"/SCPDURL", DUMMY_PATH},
#endif
#ifdef ENABLE_L3F_SERVICE
	{"service", INITHELPER(66,5)},
/* 66 */
	{"/serviceType", "urn:schemas-upnp-org:service:Layer3Forwarding:1"},
	{"/serviceId", "urn:upnp-org:serviceId:L3Forwarding1"},
	{"/SCPDURL", L3F_PATH},
	{"/controlURL", L3F_CONTROLURL}, /* The Layer3Forwarding service is only */
	{"/eventSubURL", L3F_EVENTURL}, /* recommended, not mandatory */
#endif
	{0, 0}
};


static const struct argument WANDLCASetEnable[] =
{
	{"NewEnable", 1, 0},
	{0, 0, 0}
};

static const struct argument WANDLCAGetInfo[] =
{
	{"NewEnable", 2, 0},
	{"NewLinkType", 2, 1},
	{"NewLinkStatus", 2, 2},
	{"NewAutoConfig", 2, 3},
	{"NewModulationType", 2, 4},
	{"NewDestinationAddress", 2, 5},
	{"NewATMEncapsulation", 2, 6},
	{"NewFCSPreserved", 2, 7},
	{"NewVCSearchList", 2, 8},
	{"NewATMAAL", 2, 9},
	{"NewATMQoS", 2, 12},
	{"NewATMPeakCellRate", 2, 13},
	{"NewATMMaximumBurstSize", 2, 14},
	{"NewATMSustainableCellRate", 2, 15},
	{0, 0, 0}
};

static const struct argument WANDLCASetDSLLinkType[] =
{
	{"NewLinkType", 1, 1},
	{0, 0, 0}
};

static const struct argument WANDLCAGetDSLLinkInfo[] =
{
	{"NewLinkType", 2, 1},
	{0, 0, 0}
};

static const struct argument WANDLCAGetAutoConfig[] =
{
	{"NewAutoConfig", 2, 3},
	{0, 0, 0}
};

static const struct argument WANDLCAGetDestinationAddress[] =
{
	{"NewDestinationAddress", 2, 5},
	{0, 0, 0}
};

static const struct argument WANDLCASetDestinationAddress[] =
{
	{"NewDestinationAddress", 1, 5},
	{0, 0, 0}
};

static const struct argument WANDLCAGetStatistics[] =
{
	{"NewATMTransmittedBlocks", 2, 10},
	{"NewATMReceivedBlocks", 2, 11},
	{"NewAAL5CRCErrors", 2, 16},
	{"NewATMCRCErrors", 2, 17},
	{"NewATMHECErrors", 2, 18},
	{0, 0, 0}
};

static const struct argument WANDLCASetATMEncapsulation[] =
{
	{"NewATMEncapsulation", 1, 6},
	{0, 0, 0}
};

static const struct argument WANDLCAGetATMEncapsulation[] =
{
	{"NewATMEncapsulation", 2, 6},
	{0, 0, 0}
};

static const struct action WANDSLLinkConfigActions[] =
{
	{"SetEnable", WANDLCASetEnable},
	{"GetInfo", WANDLCAGetInfo},
	{"SetDSLLinkType", WANDLCASetDSLLinkType},
	{"GetDSLLinkInfo", WANDLCAGetDSLLinkInfo},
	{"GetAutoConfig", WANDLCAGetAutoConfig},
	{"GetDestinationAddress", WANDLCAGetDestinationAddress},
	{"SetDestinationAddress", WANDLCASetDestinationAddress},
	{"GetStatistics", WANDLCAGetStatistics},
	{"SetATMEncapsulation", WANDLCASetATMEncapsulation},
	{"GetATMEncapsulation", WANDLCAGetATMEncapsulation},
	{0, 0}
};

static const struct stateVar WANDSLLinkConfigVars[] =
{
	{"Enable", 1, 0},
	{"LinkType", 0, 0, 139},
	{"LinkStatus", 0, 0, 6},
	{"AutoConfig", 1, 0},
	{"ModulationType", 0, 0, 116},         // Optional
/*5*/   {"DestinationAddress", 0, 0/*, 105*/},
	{"ATMEncapsulation", 0, 0, 146},     // Optional
	{"FCSPreserved", 1, 0},                        // Optional
	{"VCSearchList", 0, 0},                         // Optional
	{"ATMAAL", 0, 0, 149},               // Optional
/*10*/  {"ATMTransmittedBlocks", 3, 0},
	{"ATMReceivedBlocks", 3, 0},
	{"ATMQoS", 0, 0, 155},        // Optional
	{"ATMPeakCellRate", 3, 0},            // Optional
	{"ATMMaximumBurstSize", 3, 0},    // Optional
/*15*/  {"ATMSustainableCellRate", 3, 0}, // Optional
	{"AAL5CRCErrors", 3, 0},
	{"ATMCRCErrors", 3, 0},
	{"ATMHECErrors", 3, 0},               // Optional
	{0, 0, 0}
};

static const struct serviceDesc scpdWANDSLLink = { WANDSLLinkConfigActions, WANDSLLinkConfigVars };
/* WANIPCn.xml */
/* see UPnP_IGD_WANIPConnection 1.0.pdf
static struct XMLElt scpdWANIPCn[] =
{
	{root_service, {INITHELPER(1,2)}},
	{0, {0}}
};
*/
static const struct argument AddPortMappingArgs[] =
{
	{"NewRemoteHost", 1, 11},
	{"NewExternalPort", 1, 12},
	{"NewProtocol", 1, 14},
	{"NewInternalPort", 1, 13},
	{"NewInternalClient", 1, 15},
	{"NewEnabled", 1, 9},
	{"NewPortMappingDescription", 1, 16},
	{"NewLeaseDuration", 1, 10},
	{0, 0, 0}
};

static const struct argument GetExternalIPAddressArgs[] =
{
	{"NewExternalIPAddress", 2, 7},
	{0, 0, 0}
};

static const struct argument DeletePortMappingArgs[] =
{
	{"NewRemoteHost", 1, 11},
	{"NewExternalPort", 1, 13},
	{"NewProtocol", 1, 14},
	{0, 0, 0}
};

static const struct argument SetConnectionTypeArgs[] =
{
	{"NewConnectionType", 1, 0},
	{0, 0, 0}
};

static const struct argument GetConnectionTypeInfoArgs[] =
{
	{"NewConnectionType", 2, 0},
	{"NewPossibleConnectionTypes", 2, 1},
	{0, 0, 0}
};

static const struct argument GetStatusInfoArgs[] =
{
	{"NewConnectionStatus", 2, 2},
	{"NewLastConnectionError", 2, 4},
	{"NewUptime", 2, 3},
	{0, 0, 0}
};

static const struct argument GetNATRSIPStatusArgs[] =
{
	{"NewRSIPAvailable", 2, 5},
	{"NewNATEnabled", 2, 6},
	{0, 0, 0}
};

static const struct argument GetGenericPortMappingEntryArgs[] =
{
	{"NewPortMappingIndex", 1, 8},
	{"NewRemoteHost", 2, 11},
	{"NewExternalPort", 2, 12},
	{"NewProtocol", 2, 14},
	{"NewInternalPort", 2, 13},
	{"NewInternalClient", 2, 15},
	{"NewEnabled", 2, 9},
	{"NewPortMappingDescription", 2, 16},
	{"NewLeaseDuration", 2, 10},
	{0, 0, 0}
};

static const struct argument GetSpecificPortMappingEntryArgs[] =
{
	{"NewRemoteHost", 1, 11},
	{"NewExternalPort", 1, 12},
	{"NewProtocol", 1, 14},
	{"NewInternalPort", 2, 13},
	{"NewInternalClient", 2, 15},
	{"NewEnabled", 2, 9},
	{"NewPortMappingDescription", 2, 16},
	{"NewLeaseDuration", 2, 10},
	{0, 0, 0}
};

#ifdef CONFIG_TR_064
static const struct argument WANIPCnAGetInfo[] =
{
	{"NewEnable", 2, 32},
	{"NewConnectionType", 2, 0},
	{"NewPossibleConnectionTypes", 2, 1},
	{"NewConnectionStatus", 2, 2},
	{"NewName", 2, 17},
	{"NewUptime", 2, 3},
	{"NewLastConnectionError", 2, 4},
	{"NewAutoDisconnectTime", 2, 18},
	{"NewIdleDisconnectTime", 2, 19},
	{"NewWarnDisconnectDelay", 2, 20},
	{"NewRSIPAvailable", 2, 5},
	{"NewNATEnabled", 2, 6},
	{"NewExternalIPAddress", 2, 7},
	{"NewSubnetMask", 2, 21},
	{"NewAddressingType", 2, 22},
	{"NewDefaultGateway", 2, 23},
	{"NewMACAddress", 2, 24},
	{"NewMACAddressOverride", 2, 25},
	{"NewMaxMTUSize", 2, 26},
	{"NewDNSEnabled", 2, 27},
 	{"NewDNSOverrideAllowed", 2, 28},
	{"NewDNSServers", 2, 29},
	{"NewConnectionTrigger", 2, 30},
	{"NewRouteProtocolRx", 2, 31},
	{0, 0, 0}
};

static const struct argument WANIPCnAGetPortMappingNumberOfEntries[] =
{
	{"NewPortMappingNumberOfEntries", 2, 8},
	{0, 0, 0}
};

static const struct argument WANIPCnASetIPInterfaceInfo[] =
{
	{"NewAddressingType", 1, 22},
	{"NewExternalIPAddress", 1, 7},
	{"NewSubnetMask", 1, 21},
	{"NewDefaultGateway", 1, 23},
	{0, 0, 0}
};

static const struct argument WANIPCnASetConnectionTrigger[] =
{
	{"NewConnectionTrigger", 1, 30},
	{0, 0, 0}
};

static const struct argument WANIPCnASetEnable[] =
{
	{"NewEnable", 1, 32},
	{0, 0, 0}
};
#endif

static const struct action WANIPCnActions[] =
{
	{"AddPortMapping", AddPortMappingArgs}, /* R */
	{"GetExternalIPAddress", GetExternalIPAddressArgs}, /* R */
	{"DeletePortMapping", DeletePortMappingArgs}, /* R */
	{"SetConnectionType", SetConnectionTypeArgs}, /* R */
	{"GetConnectionTypeInfo", GetConnectionTypeInfoArgs}, /* R */
	{"RequestConnection", 0}, /* R */
	{"ForceTermination", 0}, /* R */
	{"GetStatusInfo", GetStatusInfoArgs}, /* R */
	{"GetNATRSIPStatus", GetNATRSIPStatusArgs}, /* R */
	{"GetGenericPortMappingEntry", GetGenericPortMappingEntryArgs}, /* R */
	{"GetSpecificPortMappingEntry", GetSpecificPortMappingEntryArgs}, /* R */
#ifdef CONFIG_TR_064
	{"GetInfo", WANIPCnAGetInfo}, /* R */
	{"RequestTermination", 0},
	{"GetPortMappingNumberOfEntries", WANIPCnAGetPortMappingNumberOfEntries},
	{"SetIPInterfaceInfo", WANIPCnASetIPInterfaceInfo},
	{"SetConnectionTrigger", WANIPCnASetConnectionTrigger},
	{"SetEnable", WANIPCnASetEnable},
#endif
	{0, 0}
};
/* R=Required, O=Optional */

static const struct stateVar WANIPCnVars[] =
{
#ifdef CONFIG_TR_064
	{"ConnectionType", 0, 0, 14}, /* required */
#else
	{"ConnectionType", 0, 0}, /* required */
#endif
	{"PossibleConnectionTypes", 0|0x80, 0, 14},
	 /* Required
	  * Allowed values : Unconfigured / IP_Routed / IP_Bridged */
	{"ConnectionStatus", 0|0x80, 0/*1*/, 18}, /* required */
	 /* Allowed Values : Unconfigured / Connecting(opt) / Connected
	  *                  PendingDisconnect(opt) / Disconnecting (opt)
	  *                  Disconnected */
	{"Uptime", 3, 0},	/* Required */
	{"LastConnectionError", 0, 0, 25},	/* required : */
	 /* Allowed Values : ERROR_NONE(req) / ERROR_COMMAND_ABORTED(opt)
	  *                  ERROR_NOT_ENABLED_FOR_INTERNET(opt)
	  *                  ERROR_USER_DISCONNECT(opt)
	  *                  ERROR_ISP_DISCONNECT(opt)
	  *                  ERROR_IDLE_DISCONNECT(opt)
	  *                  ERROR_FORCED_DISCONNECT(opt)
	  *                  ERROR_NO_CARRIER(opt)
	  *                  ERROR_IP_CONFIGURATION(opt)
	  *                  ERROR_UNKNOWN(opt) */
	{"RSIPAvailable", 1, 0}, /* required */
	{"NATEnabled", 1, 0},    /* required */
	{"ExternalIPAddress", 0|0x80, 0}, /* required. Default : empty string */
	{"PortMappingNumberOfEntries", 2|0x80, 0}, /* required >= 0 */
	{"PortMappingEnabled", 1, 0}, /* Required */
	{"PortMappingLeaseDuration", 3, 0}, /* required */
	{"RemoteHost", 0, 0},   /* required. Default : empty string */
	{"ExternalPort", 2, 0}, /* required */
	{"InternalPort", 2, 0}, /* required */
	{"PortMappingProtocol", 0, 0, 11}, /* required allowedValues: TCP/UDP */
	{"InternalClient", 0, 0}, /* required */
	{"PortMappingDescription", 0, 0}, /* required default: empty string */
#ifdef CONFIG_TR_064
	{"Name", 0, 0}, /* required */
	{"AutoDisconnectTime", 3, 0}, /* required */
	{"IdleDisconnectTime", 3, 0}, /* required */
/*20*/	{"WarnDisconnectDelay", 3, 0}, /* required */
	{"SubnetMask", 0, 0},
	{"AddressingType", 0, 0, 43},
	{"DefaultGateway", 0, 0},
	{"MACAddress", 0, 0},
/*25*/	{"MACAddressOverride", 1, 0},
	{"MaxMTUSize", 2, 0},
	{"DNSEnabled", 1, 0},
	{"DNSOverrideAllowed", 1, 0},
	{"DNSServers", 0, 0},
/*30*/	{"ConnectionTrigger", 0, 0 /* to do */},
	{"RouteProtocolRx", 0, 0 /* to do */},
	{"Enable", 1, 0},
#endif
	{0, 0, 0}
};

static const struct serviceDesc scpdWANIPCn =
{ WANIPCnActions, WANIPCnVars };

/* WANCfg.xml */
/* See UPnP_IGD_WANCommonInterfaceConfig 1.0.pdf */

static const struct argument GetCommonLinkPropertiesArgs[] =
{
	{"NewWANAccessType", 2, 0},
	{"NewLayer1UpstreamMaxBitRate", 2, 1},
	{"NewLayer1DownstreamMaxBitRate", 2, 2},
	{"NewPhysicalLinkStatus", 2, 3},
	{0, 0, 0}
};

static const struct argument GetTotalBytesSentArgs[] =
{
#ifdef CONFIG_TR_064
	{"NewTotalBytesSent", 2, 9},
#else
	{"NewTotalBytesSent", 2, 4},
#endif
	{0, 0, 0}
};

static const struct argument GetTotalBytesReceivedArgs[] =
{
#ifdef CONFIG_TR_064
	{"NewTotalBytesReceived", 2, 10},
#else
	{"NewTotalBytesReceived", 2, 5},
#endif
	{0, 0, 0}
};

static const struct argument GetTotalPacketsSentArgs[] =
{
#ifdef CONFIG_TR_064
	{"NewTotalPacketsSent", 2, 11},
#else
	{"NewTotalPacketsSent", 2, 6},
#endif
	{0, 0, 0}
};

static const struct argument GetTotalPacketsReceivedArgs[] =
{
#ifdef CONFIG_TR_064
	{"NewTotalPacketsReceived", 2, 12},
#else
	{"NewTotalPacketsReceived", 2, 7},
#endif
	{0, 0, 0}
};

#ifdef CONFIG_TR_064
static const struct argument SetEnabledForInternet[] =
{
	{"NewEnabledForInternet", 1, 13},
	{0, 0, 0}
};

static const struct argument GetEnabledForInternet[] =
{
	{"NewEnabledForInternet", 2, 13},
	{0, 0, 0}
};
#endif

static const struct action WANCfgActions[] =
{
#ifdef CONFIG_TR_064
	{"SetEnabledForInternet", SetEnabledForInternet},         /* Optional */
	{"GetEnabledForInternet", GetEnabledForInternet},         /* Optional */
#endif
	{"GetCommonLinkProperties", GetCommonLinkPropertiesArgs}, /* Required */
	{"GetTotalBytesSent", GetTotalBytesSentArgs},             /* optional */
	{"GetTotalBytesReceived", GetTotalBytesReceivedArgs},     /* optional */
	{"GetTotalPacketsSent", GetTotalPacketsSentArgs},         /* optional */
	{"GetTotalPacketsReceived", GetTotalPacketsReceivedArgs}, /* optional */
	{0, 0}
};

/* See UPnP_IGD_WANCommonInterfaceConfig 1.0.pdf */
static const struct stateVar WANCfgVars[] =
{
	{"WANAccessType", 0, 0, 1},
	/* Allowed Values : DSL / POTS / Cable / Ethernet
	 * Default value : empty string */
	{"Layer1UpstreamMaxBitRate", 3, 0},
	{"Layer1DownstreamMaxBitRate", 3, 0},
	{"PhysicalLinkStatus", 0|0x80, 0, 6},
	/*  allowed values :
	 *      Up / Down / Initializing (optional) / Unavailable (optional)
	 *  no Default value
	 *  Evented */
#ifdef CONFIG_TR_064
	{"WANAccessProvider", 0, 0},	           /* Optional */
	{"MaximumActiveConnections", 2, 0},	   /* Optional */
	{"NumberOfActiveConnections", 2, 0},	   /* Optional */
	{"ActiveConnectionDeviceContainer", 0, 0},	   /* Optional */
	{"ActiveConnectionServiceID", 0, 0},	   /* Optional */
#endif
	{"TotalBytesSent", 3, 0},	   /* Optional */
	{"TotalBytesReceived", 3, 0},  /* Optional */
	{"TotalPacketsSent", 3, 0},    /* Optional */
	{"TotalPacketsReceived", 3, 0},/* Optional */
	/*{"MaximumActiveConnections", 2, 0},	// allowed Range value // OPTIONAL */
#ifdef CONFIG_TR_064
	{"EnabledForInternet", 1, 0},/* Optional */
#endif
	{0, 0, 0}
};

static const struct serviceDesc scpdWANCfg = { WANCfgActions, WANCfgVars };

#ifdef CONFIG_TR_064
static const struct stateVar DeviceInfoVars[] =
{
	{"ManufacturerName", 0, 0},
	{"ManufacturerOUI", 0, 0},
	{"ModelName", 0, 0},
	{"Description", 0, 0},
	{"ProductClass", 0, 0},   //Optional
	{"SerialNumber", 0, 0},  //index 5
	{"SoftwareVersion", 0, 0},
	{"AdditionalSoftwareVersions", 0, 0}, //Optional
	{"ModemFirmwareVersion", 0, 0},  //Optional
	{"EnabledOptions", 0, 0},  //Optional
	{"HardwareVersion", 0, 0},   //index 10
	{"AdditionalHardwareVersions", 0, 0},  //Optional
	{"SpecVersion", 0, 0},
	{"ProvisioningCode", 0, 0},  //Optional
	{"UpTime", 3, 0},      //Optional
	{"FirstUseDate", 0, 0},  //Optional.     //index 15
	{"DeviceLog", 0, 0},
	{0, 0, 0}
};

static const struct argument DeviceInfoGetInfoArgs[] =
{
	{"NewManufacturerName", 2, 0},
	{"NewManufacturerOUI", 2, 1},
	{"NewModelName", 2, 2},
	{"NewDescription", 2, 3},
	{"NewProductClass", 2, 4},
	{"NewSerialNumber", 2, 5},
	{"NewSoftwareVersion", 2, 6},
	{"NewModemFirmwareVersion", 2, 8},
	{"NewHardwareVersion", 2, 10},
	{"NewSpecVersion", 2, 12},
	{"NewProvisioningCode", 2, 13},
	{"NewUpTime", 2, 14},
	{0, 0, 0}
};

static const struct argument DeviceInfoSetProvisioningCodeArgs[] =
{
	{"NewProvisioningCode", 1, 13},
	{0, 0, 0}
};

static const struct argument DeviceInfoGetDeviceLogArgs[] =
{
	{"NewDeviceLog", 2, 16},
	{0, 0, 0}
};

static const struct action DeviceInfoActions[] =
{
	{"GetInfo", DeviceInfoGetInfoArgs},
	{"SetProvisioningCode", DeviceInfoSetProvisioningCodeArgs},
	{"GetDeviceLog", DeviceInfoGetDeviceLogArgs},
	{0, 0}
};

static const struct stateVar DeviceConfigVars[] =
{
	{"PersistentData", 0, 0},
	{"ConfigFile", 0, 0},         //Optional
	{"A_ARG_TYPE_Status", 0, 0, 27},  //Allowed Value: RebootRequired, ChangesApplied
	{"A_ARG_TYPE_UUID", 4, 0},
	{0, 0, 0}
};

static const struct argument DeviceConfigGetConfigurationArgs[] =
{
	{"NewConfigFile", 2, 1},
	{0, 0, 0}
};

static const struct argument DeviceConfigSetConfigurationArgs[] =
{
	{"NewConfigFile", 1, 1},
	{0, 0, 0}
};

static const struct argument DeviceConfigConfigurationStartedArgs[] =
{
	{"NewSessionID", 1, 3},
	{0, 0, 0}
};

static const struct argument DeviceConfigConfigurationFinishedArgs[] =
{
	{"NewStatus", 2, 2},
	{0, 0, 0}
};

static const struct action DeviceConfigActions[] =
{
	{"GetConfiguration", DeviceConfigGetConfigurationArgs},        // Optional
	{"SetConfiguration", DeviceConfigSetConfigurationArgs},        // Optional
	{"ConfigurationStarted", DeviceConfigConfigurationStartedArgs},
	{"ConfigurationFinished", DeviceConfigConfigurationFinishedArgs},
	{"FactoryReset"},     // This action does not have any arguments.
	{"Reboot"},     // This action does not have any arguments.
	{0, 0}
};

static const struct stateVar Layer3ForwardingVars[] =
{
	{"DefaultConnectionService", 0, 0},
	{"ForwardNumberOfEntries", 2, 0},    // >=0
	{"Enable", 1, 0},  // Allowed value: 1, 0
	{"Status", 0, 0, 30},
	{"Type", 0, 0, 34},
	{"DestIPAddress", 0, 0},             // index 5
	{"DestSubnetMask", 0, 0},
	{"SourceIPAddress", 0, 0},
	{"SourceSubnetMask", 0, 0},
	{"GatewayIPAddress", 0, 0},
	{"Interface", 0, 0},                      // index 10
	{"ForwardingMetric", 6, 0},   // >=-1
	{"MTU", 2, 0},    // Optional
	{0, 0, 0}
};

static const struct argument  Layer3ForwardingSetDefaultConnectionServiceArgs[] =
{
	{"NewDefaultConnectionService", 1, 0},
	{0, 0, 0}
};

static const struct argument  Layer3ForwardingGetDefaultConnectionServiceArgs[] =
{
	{"NewDefaultConnectionService", 2, 0},
	{0, 0, 0}
};

static const struct argument  Layer3ForwardingGetForwardNumberOfEntriesArgs[] =
{
	{"NewForwardNumberOfEntries", 2, 1},
	{0, 0, 0}
};

static const struct argument  Layer3ForwardingAddForwardingEntryArgs[] =
{
	{"NewType", 1, 4},
	{"NewDestIPAddress", 1, 5},
	{"NewDestSubnetMask", 1, 6},
	{"NewSourceIPAddress", 1, 7},
	{"NewSourceSubnetMask", 1, 8},
	{"NewGatewayIPAddress", 1, 9},
	{"NewInterface", 1, 10},
	{"NewForwardingMetric", 1, 11},
	//{"NewMTU", 1, 12},
	{0, 0, 0}
};

static const struct argument  Layer3ForwardingDeleteForwardingEntryArgs[] =
{
	{"NewDestIPAddress", 1, 5},
	{"NewDestSubnetMask", 1, 6},
	{"NewSourceIPAddress", 1, 7},
	{"NewSourceSubnetMask", 1, 8},
	{0, 0, 0}
};

static const struct argument  Layer3ForwardingGetSpecificForwardingEntryArgs[] =
{
	{"NewDestIPAddress", 1, 5},
	{"NewDestSubnetMask", 1, 6},
	{"NewSourceIPAddress", 1, 7},
	{"NewSourceSubnetMask", 1, 8},
	{"NewGatewayIPAddress", 2, 9},
	{"NewEnable", 2, 2},
	{"NewStatus", 2, 3},
	{"NewType", 2, 4},
	{"NewInterface", 2, 10},
	{"NewForwardingMetric", 2, 11},
	//{"NewMTU", 2, 12},
	{0, 0, 0}
};

static const struct argument  Layer3ForwardingGetGenericForwardingEntryArgs[] =
{
	{"NewForwardingIndex", 1, 1},
	{"NewEnable", 2, 2},
	{"NewStatus", 2, 3},
	{"NewType", 2, 4},
	{"NewDestIPAddress", 2, 5},
	{"NewDestSubnetMask", 2, 6},
	{"NewSourceIPAddress", 2, 7},
	{"NewSourceSubnetMask", 2, 8},
	{"NewGatewayIPAddress", 2, 9},
	{"NewInterface", 2, 10},
	{"NewForwardingMetric", 2, 11},
	//{"NewMTU", 2, 12},
	{0, 0, 0}
};

static const struct argument  Layer3ForwardingSetForwardingEntryEnableArgs[] =
{
	{"NewDestIPAddress", 1, 5},
	{"NewDestSubnetMask", 1, 6},
	{"NewSourceIPAddress", 1, 7},
	{"NewSourceSubnetMask", 1, 8},
	{"NewEnable", 1, 2},
	{0, 0, 0}
};

static const struct action Layer3ForwardingActions[] =
{
	{"SetDefaultConnectionService", Layer3ForwardingSetDefaultConnectionServiceArgs},
	{"GetDefaultConnectionService", Layer3ForwardingGetDefaultConnectionServiceArgs},
	{"GetForwardNumberOfEntries", Layer3ForwardingGetForwardNumberOfEntriesArgs},
	{"AddForwardingEntry", Layer3ForwardingAddForwardingEntryArgs},
	{"DeleteForwardingEntry", Layer3ForwardingDeleteForwardingEntryArgs},
	{"GetSpecificForwardingEntry", Layer3ForwardingGetSpecificForwardingEntryArgs},
	{"GetGenericForwardingEntry", Layer3ForwardingGetGenericForwardingEntryArgs},
	{"SetForwardingEntryEnable", Layer3ForwardingSetForwardingEntryEnableArgs},
	{0, 0}
};

static const struct stateVar LANConfigSecurityVars[] =
{
	{"ConfigPassword", 0, 0},
	{0, 0, 0}
};

static const struct argument  LANConfigSecuritySetConfigPasswordArgs[] =
{
	{"NewPassword", 1, 0},
	{0, 0, 0}
};

static const struct action LANConfigSecurityActions[] =
{
	{"SetConfigPassword",  LANConfigSecuritySetConfigPasswordArgs},
	{0, 0}
};

static const struct stateVar ManagementServerVars[] =
{
	{"URL", 0, 0},
	{"Password", 0, 0},
	{"PeriodicInformEnable", 1, 0},
	{"PeriodicInformInterval", 3, 0},
	{"PeriodicInformTime", 5, 0},
	{"ParameterKey", 0, 0},                   // Index 5
	{"ParameterHash", 0, 0},
	{"ConnectionRequestURL", 0, 0},
	{"ConnectionRequestUsername", 0, 0},
	{"ConnectionRequestPassword", 0, 0},
	{"UpgradesManaged", 1, 0},          // Index 10
	{"KickURL", 0, 0},        //Optional
	{"DownloadProgressURL", 0, 0},   //Optional
	{0, 0, 0}
};

static const struct argument ManagementServerGetInfoArgs[] =
{
		{"NewURL", 2, 0},
		{"NewPeriodicInformEnable", 2, 2},
		{"NewPeriodicInformInterval", 2, 3},
		{"NewPeriodicInformTime", 2, 4},
		{"NewParameterKey", 2, 5},
		{"NewParameterHash", 2, 6},
		{"NewConnectionRequestURL", 2, 7},
		{"NewConnectionRequestUsername", 2, 8},
		{"NewUpgradesManaged", 2, 10},
		{0, 0, 0}
};

static const struct argument ManagementServerSetManagementServerURLArgs[] =
{
	{"NewURL", 1, 0},
	{0, 0, 0}
};

static const struct argument ManagementServerSetManagementServerPasswordArgs[] =
{
	{"NewPassword", 1, 1},
	{0, 0, 0}
};

static const struct argument ManagementServerSetPeriodicInformArgs[] =
{
	{"NewPeriodicInformEnable", 1, 2},
	{"NewPeriodicInformInterval", 1, 3},
	{"NewPeriodicInformTime", 1, 4},
	{0, 0, 0}
};

static const struct argument ManagementServerSetUpgradeManagementArgs[] =
{
	{"NewUpgradesManaged", 1, 10},
	{0, 0, 0}
};

static const struct argument ManagementServerSetConnectionRequestAuthentication[] =
{
	{"NewConnectionRequestUsername", 1, 8},
	{"NewConnectionRequestPassword", 1, 9},
	{0, 0, 0}
};

static const struct action ManagementServerActions[] =
{
	{"GetInfo", ManagementServerGetInfoArgs},
	{"SetManagementServerURL", ManagementServerSetManagementServerURLArgs},
	{"SetManagementServerPassword", ManagementServerSetManagementServerPasswordArgs},
	{"SetPeriodicInform", ManagementServerSetPeriodicInformArgs},
	{"SetUpgradeManagement", ManagementServerSetUpgradeManagementArgs},
	{"SetConnectionRequestAuthentication", ManagementServerSetConnectionRequestAuthentication},
	{0, 0}
};

static const struct stateVar TimeVars[] =
{
	{"NTPServer1", 0, 0},
	{"NTPServer2", 0, 0},
	{"NTPServer3", 0, 0},
	{"NTPServer4", 0, 0},
	{"NTPServer5", 0, 0},
	{"CurrentLocalTime", 5, 0},    // Index 5
	{"LocalTimeZone", 0, 0},
	{"LocalTimeZoneName", 0, 0},
	{"DaylightSavingsUsed", 1, 0},
	{"DaylightSavingsStart", 5, 0},
	{"DaylightSavingsEnd", 5, 0},    // Index 10
	{0, 0, 0}
};

static const struct argument TimeGetInfoArgs[] =
{
	{"NewNTPServer1", 2, 0},
	{"NewNTPServer2", 2, 1},
	{"NewCurrentLocalTime", 2, 5},
	{"NewLocalTimeZone", 2, 6},
	{"NewLocalTimeZoneName", 2, 7},
	{"NewDaylightSavingsUsed", 2, 8},
	{"NewDaylightSavingsStart", 2, 9},
	{"NewDaylightSavingsEnd", 2, 10},
	{0 ,0 ,0}
};

static const struct argument TimeSetNTPServersArgs[] =
{
	{"NewNTPServer1", 1, 0},
	{"NewNTPServer2", 1, 1},
	{0, 0, 0}
};

static const struct argument TimeSetLocalTimeZoneArgs[] =
{
	{"NewLocalTimeZone", 1, 6},
	{"NewLocalTimeZoneName", 1, 7},
	{"NewDaylightSavingsUsed", 1, 8},
	{"NewDaylightSavingsStart", 1, 9},
	{"NewDaylightSavingsEnd", 1, 10},
	{0, 0, 0}
};

static const struct action TimeActions[] =
{
	{"GetInfo", TimeGetInfoArgs},
	{"SetNTPServers", TimeSetNTPServersArgs},
	{"SetLocalTimeZone", TimeSetLocalTimeZoneArgs},
	{0, 0}
};

static const struct serviceDesc scpdDeviceInfo = { DeviceInfoActions, DeviceInfoVars };
static const struct serviceDesc scpdDeviceConfig = { DeviceConfigActions, DeviceConfigVars };
static const struct serviceDesc scpdLayer3Forwarding = { Layer3ForwardingActions, Layer3ForwardingVars };
static const struct serviceDesc scpdLANConfigSecurity = { LANConfigSecurityActions, LANConfigSecurityVars };
static const struct serviceDesc scpdManagementServer = { ManagementServerActions, ManagementServerVars };
static const struct serviceDesc scpdTime = { TimeActions, TimeVars };

static const struct stateVar LANHostConfigManagementVars[] =
{
	{"DHCPServerConfigurable", 1, 0},
	{"DHCPRelay", 1, 0},
	{"SubnetMask", 0, 0},
	{"DNSServers", 0, 0},
	{"DomainName", 0, 0},
	{"MinAddress", 0, 0},      // Index 5
	{"MaxAddress", 0, 0},
	{"IPRouters", 0, 0},
	{"ReservedAddresses", 0, 0},
	{"DHCPLeaseTime", 6, 0},    //Optional       >= -1, -1 for infinite lease.
	{"DHCPServerEnable", 1, 0},       // Index 10
	{"UseAllocatedWAN", 0, 0, 38},  //Optional. Allowed value: Normal, UseAllocatedS, ubnet, Passthrough.
	{"AssociatedConnection", 0, 0},   //Optional
	{"PassthroughLease", 3, 0},    //Optional        0-65535
	{"PassthroughMACAddress", 0, 0},    //Optional
	{"AllowedMACAddresses", 0, 0},       // Index 15
	{"Enable", 1, 0},
	{"IPInterfaceIPAddress", 0, 0},
	{"IPInterfaceSubnetMask", 0, 0},
	{"IPInterfaceAddressingType", 0, 0, 43},
	{"IPInterfaceNumberOfEntries", 2, 0},   // >=0        // Index 20
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementGetInfo[] =
{
	{"NewDHCPServerConfigurable", 2, 0},
	{"NewDHCPRelay", 2, 1},
	{"NewSubnetMask", 2, 2},
	{"NewDNSServers", 2, 3},
	{"NewDomainName", 2, 4},
	{"NewMinAddress", 2, 5},
	{"NewMaxAddress", 2, 6},
	{"NewIPRouters", 2, 7},
	{"NewReservedAddresses", 2, 8},
	{"NewDHCPLeaseTime", 2, 9},
	{"NewDHCPServerEnable", 2, 10},
	{"NewUseAllocatedWAN", 2, 11},
	{"NewAssociatedConnection", 2, 12},
	{"NewPassthroughLease", 2, 13},
	{"NewPassthroughMACAddress", 2, 14},
	{"NewAllowedMACAddresses", 2, 15},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementSetDHCPServerConfigurable[] =
{
	{"NewDHCPServerConfigurable", 1, 0},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementGetDHCPServerConfigurable[] =
{
	{"NewDHCPServerConfigurable", 2, 0},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementGetDHCPRelay[] =
{
	{"NewDHCPRelay", 2, 1},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementSetSubnetMask[] =
{
	{"NewSubnetMask", 1, 2},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementGetSubnetMask[] =
{
	{"NewSubnetMask", 2, 2},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementSetIPRouter[] =
{
	{"NewIPRouters", 1, 7},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementGetIPRoutersList[] =
{
	{"NewIPRouters", 2, 7},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementSetDomainName[] =
{
	{"NewDomainName", 1, 4},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementGetDomainName[] =
{
	{"NewDomainName", 2, 4},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementSetAddressRange[] =
{
	{"NewMinAddress", 1, 5},
	{"NewMaxAddress", 1, 6},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementGetAddressRange[] =
{
	{"NewMinAddress", 2, 5},
	{"NewMaxAddress", 2, 6},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementSetDNSServer[] =
{
	{"NewDNSServers", 1, 3},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementDeleteDNSServer[] =
{
	{"NewDNSServers", 1, 3},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementGetDNSServers[] =
{
	{"NewDNSServers", 2, 3},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementSetDHCPLeaseTime[] =
{
	{"NewDHCPLeaseTime", 1, 9},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementSetDHCPServerEnable[] =
{
	{"NewDHCPServerEnable", 1, 10},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementGetIPInterfacetNumberOfEntries[] =
{
	{"NewIPInterfaceNumberOfEntries", 2, 20},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementSetIPInterface[] =
{
	{"NewEnable", 1, 16},
	{"NewIPAddress", 1, 17},
	{"NewSubnetMask", 1, 18},
	{"NewIPAddressingType", 1, 19},
	{0, 0, 0}
};

static const struct argument LANHostConfigManagementGetIPInterfaceGenericEntry[] =
{
	{"NewIPInterfaceIndex", 1, 20},
	{"NewEnable", 2, 16},
	{"NewIPAddress", 2, 17},
	{"NewSubnetMask", 2, 18},
	{"NewIPAddressingType", 2, 19},
	{"NewIPInterfaceNumberOfEntries", 2, },
	{0, 0, 0}
};


static const struct action LANHostConfigManagementActions[] =
{
	{"GetInfo", LANHostConfigManagementGetInfo},
	{"SetDHCPServerConfigurable", LANHostConfigManagementSetDHCPServerConfigurable},
	{"GetDHCPServerConfigurable", LANHostConfigManagementGetDHCPServerConfigurable},
	{"GetDHCPRelay", LANHostConfigManagementGetDHCPRelay},
	{"SetSubnetMask", LANHostConfigManagementSetSubnetMask},
	{"GetSubnetMask", LANHostConfigManagementGetSubnetMask},
	{"SetIPRouter", LANHostConfigManagementSetIPRouter},
	{"GetIPRoutersList", LANHostConfigManagementGetIPRoutersList},
	{"SetDomainName", LANHostConfigManagementSetDomainName},
	{"GetDomainName", LANHostConfigManagementGetDomainName},
	{"SetAddressRange", LANHostConfigManagementSetAddressRange},
	{"GetAddressRange", LANHostConfigManagementGetAddressRange},
	{"SetDNSServer", LANHostConfigManagementSetDNSServer},
	{"DeleteDNSServer", LANHostConfigManagementDeleteDNSServer},
	{"GetDNSServers", LANHostConfigManagementGetDNSServers},
	{"SetDHCPLeaseTime", LANHostConfigManagementSetDHCPLeaseTime},         // Optional
	{"SetDHCPServerEnable", LANHostConfigManagementSetDHCPServerEnable},
	{"GetIPInterfacetNumberOfEntries", LANHostConfigManagementGetIPInterfacetNumberOfEntries},
	{"SetIPInterface", LANHostConfigManagementSetIPInterface},
	{"GetIPInterfaceGenericEntry", LANHostConfigManagementGetIPInterfaceGenericEntry},
	{0, 0}
};

static const struct stateVar LANEthernetInterfaceConfigVars[] =
{
	{"Enable", 1, 0},
	{"Status", 0, 0, 47},
	{"MACAddress", 0, 0},
	{"MACAddressControlEnabled", 1, 0},   // Optional
	{"MaxBitRate", 0, 0, 52},
	{"DuplexMode", 0, 0, 57},             // Index 5
	{"Stats.BytesSent", 3, 0},      // >=0
	{"Stats.BytesReceived", 3, 0},   // >=0
	{"Stats.PacketsSent", 3, 0},      // >=0
	{"Stats.PacketsReceived", 3, 0},      // >=0
	{0, 0, 0}
};

static const struct argument LANEthernetInterfaceSetEnable[] =
{
	{"NewEnable", 1, 0},
	{0, 0, 0}
};

static const struct argument LANEthernetInterfaceSetMaxBitRate[] =
{
	{"NewMaxBitRate", 1, 4},
	{0, 0, 0}
};

static const struct argument LANEthernetInterfaceNewDuplexMode[] =
{
	{"NewDuplexMode", 1, 5},
	{0, 0, 0}
};

static const struct argument LANEthernetInterfaceGetInfo[] =
{
	{"NewEnable", 2, 0},
	{"NewStatus", 2, 1},
	{"NewMACAddress", 2, 2},
	{"NewMACAddressControlEnabled", 2, 3},
	{"NewMaxBitRate", 2, 4},
	{"NewDuplexMode", 2, 5},
	{0, 0, 0}
};

static const struct argument LANEthernetInterfaceGetStatistics[] =
{
	{"NewBytesSent", 2, 6},
	{"NewBytesReceived", 2, 7},
	{"NewPacketsSent", 2, 8},
	{"NewPacketsReceived", 2, 9},
	{0, 0, 0}
};

static const struct argument LANEthernetInterfaceSetMACAddressControlEnabled[] =
{
	{"NewMACAddressControlEnabled", 1, 3},
	{0, 0, 0}
};

static const struct action LANEthernetInterfaceConfigActions[] =
{
	{"SetEnable", LANEthernetInterfaceSetEnable},
	{"SetMaxBitRate", LANEthernetInterfaceSetMaxBitRate},      // Optional
	{"SetDuplexMode", LANEthernetInterfaceNewDuplexMode},     // Optional
	{"GetInfo", LANEthernetInterfaceGetInfo},
	{"GetStatistics", LANEthernetInterfaceGetStatistics},
	{"SetMACAddressControlEnabled", LANEthernetInterfaceSetMACAddressControlEnabled},    // Optional
	{0, 0}
};

static const struct stateVar scpdWLANConfigurationVars[] =
{
	{"Enable", 1, 0},          // Index 0
	{"Status", 0, 0, 61},
	{"MaxBitRate", 0, 0},
	{"Channel", 7, 0},
	{"SSID", 0, 0},
	{"BeaconType", 0, 0},            // Index 5
	{"WEPKeyIndex", 7, 0},      //0-3
	{"WEPKey", 0, 0},
	{"WEPEncryptionLevel", 0, 0},
	{"PreSharedKeyIndex", 7, 0},    //0-9       // Optional
	{"AssociatedDeviceMACAddress", 0, 0},     // Index 10
	{"PreSharedKey", 0, 0},
	{"KeyPassphrase", 0, 0},
	{"MACAddressControlEnabled", 1, 0},
	{"Standard", 0, 0, 65},
	{"BSSID", 0, 0},        //MAC address         // Index 15
	{"TotalBytesSent", 3, 0},       // >=0        //Optional
	{"TotalBytesReceived", 3, 0},   // >=0
	{"TotalPacketsSent", 3, 0},
	{"TotalPacketsReceived", 3, 0},
	{"BasicEncryptionModes", 0, 0, 69},       // Index 20
	{"BasicAuthenticationMode", 0, 0, 72},
	{"WPAEncryptionModes", 0, 0, 75},
	{"WPAAuthenticationMode", 0, 0, 83},

	//The folowing variables are needed only for advanced use.
	{"PossibleChannels", 0, 0},
	{"BasicDataTransmitRates", 0, 0},      // Index 25
	{"OperationalDataTransmitRates", 0, 0},
	{"PossibleDataTransmitRates", 0, 0},
	{"IEEE11iEncryptionModes", 0, 0, 86},         // Optional
	{"IEEE11iAuthenticationMode", 0, 0, 94},    // Optional
	{"TotalAssociations", 2, 0},     // >=0    // Index 30
	{"AssociatedDeviceMACAddress", 0, 0},     // MAC address
	{"AssociatedDeviceIPAddress", 0, 0},        // IP address
	{"AssociatedDeviceAuthenticationState", 1, 0},
	{"LastRequestedUnicastCipher", 0, 0},      // Optional
	{"LastRequestedMulticastCipher", 0, 0},    // Optional    // Index 35
	{"LastPMKId", 0, 0},      // Optional
	{"RadioEnabled", 1, 0},
	{"AutoRateFallBackEnabled", 0, 0},
	{0, 0, 0}
};

static const struct argument WLANConfigurationSetEnable[] =
{
	{"NewEnable", 1, 0},
	{0, 0, 0}
};

static const struct argument WLANConfigurationGetInfo[] =
{
	{"NewEnable", 2, 0},
	{"NewStatus", 2, 1},
	{"NewMaxBitRate", 2, 2},
	{"NewChannel", 2, 3},
	{"NewSSID", 2, 4},
	{"NewBeaconType", 2, 5},
	{"NewMACAddressControlEnabled", 2, 13},
	{"NewStandard", 2, 14},
	{"NewBSSID", 2, 15},
	{"NewBasicEncryptionModes", 2, 20},
	{"NewBasicAuthenticationMode", 2, 21},
	{0, 0, 0}
};

static const struct argument WLANConfigurationSetConfig[] =
{
	{"NewMaxBitRate", 1, 2},
	{"NewChannel", 1, 3},
	{"NewSSID", 1, 4},
	{"NewBeaconType", 1, 5},
	{"NewMACAddressControlEnabled", 1, 13},
	{"NewBasicEncryptionModes", 1, 20},
	{"NewBasicAuthenticationMode", 1, 21},
	{0, 0, 0}
};

static const struct argument WLANConfigurationSetSecurityKeys[] =
{
	{"NewWEPKey0", 1, 7},
	{"NewWEPKey1", 1, 7},
	{"NewWEPKey2", 1, 7},
	{"NewWEPKey3", 1, 7},
	{"NewPreSharedKey", 1, 11},
	{"NewKeyPassphrase", 1, 12},
	{0, 0, 0}
};

static const struct argument WLANConfigurationGetSecurityKeys[] =
{
	{"NewWEPKey0", 2, 7},
	{"NewWEPKey1", 2, 7},
	{"NewWEPKey2", 2, 7},
	{"NewWEPKey3", 2, 7},
	{"NewPreSharedKey", 2, 11},
	{"NewKeyPassphrase", 2, 12},
	{0, 0, 0}
};

static const struct argument WLANConfigurationSetDefaultWEPKeyIndex[] =
{
	{"NewDefaultWEPKeyIndex", 1, 6},
	{0, 0, 0}
};

static const struct argument WLANConfigurationGetDefaultWEPKeyIndex[] =
{
	{"NewDefaultWEPKeyIndex", 1, 6},
	{0, 0, 0}
};

static const struct argument WLANConfigurationSetBasBeaconSecurityProperties[] =
{
	{"NewBasicEncryptionModes", 1, 20},
	{"NewBasicAuthenticationMode", 1, 21},
	{0, 0, 0}
};

static const struct argument WLANConfigurationGetBasBeaconSecurityProperties[] =
{
	{"NewBasicEncryptionModes", 2, 20},
	{"NewBasicAuthenticationMode", 2, 21},
	{0, 0, 0}
};

static const struct argument WLANConfigurationSetWPABeaconSecurityProperties[] =
{
	{"NewWPAEncryptionModes", 1, 22},
	{"NewWPAAuthenticationMode", 1, 23},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetWPABeaconSecurityProperties[] =
{
	{"NewWPAEncryptionModes", 2, 22},
	{"NewWPAAuthenticationMode", 2, 23},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetStatistics[] =
{
	{"NewTotalBytesSent", 2, 16},
	{"NewTotalBytesReceived", 2, 17},
	{"NewTotalPacketsSent", 2, 18},
	{"NewTotalPacketsReceived", 2, 19},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationSetSSID[] =
{
	{"NewSSID", 1, 4},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetSSID[] =
{
	{"NewSSID", 2, 4},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetBSSID[] =
{
	{"NewBSSID", 2, 15},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationSetBeaconType[] =
{
	{"NewBeaconType", 1, 5},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetBeaconType[] =
{
	{"NewBeaconType", 2, 5},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationSetChannel[] =
{
	{"NewChannel", 1, 3},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetChannelInfo[] =
{
	{"NewChannel", 2, 3},
	{"NewPossibleChannels", 2, 24},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationSetDataTransmitRates[] =
{
	{"NewBasicDataTransmitRates", 1, 25},
	{"NewOperationalDataTransmitRates", 1, 26},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetDataTransmitRateInfo[] =
{
	{"NewBasicDataTransmitRates", 2, 25},
	{"NewOperationalDataTransmitRates", 2, 26},
	{"NewPossibleDataTransmitRates", 2, 27},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetPacketStatistics[] =
{
	{"NewTotalPacketsSent", 2, 18},
	{"NewTotalPacketsReceived", 2, 19},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetTotalAssociations[] =
{
	{"NewTotalAssociations", 2, 30},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetGenericAssociatedDeviceInfo[] =
{
	{"NewAssociatedDeviceIndex", 1, 30},
	{"NewAssociatedDeviceMACAddress", 2, 31},
	{"NewAssociatedDeviceIPAddress", 2, 32},
	{"NewAssociatedDeviceAuthState", 2, 33},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetSpecificAssociatedDeviceInfo[] =
{
	{"NewAssociatedDeviceMACAddress", 2, 31},
	{"NewAssociatedDeviceIPAddress", 2, 32},
	{"NewAssociatedDeviceAuthState", 2, 33},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationSetRadioMode[] =
{
	{"NewRadioEnable", 1, 37},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetRadioMode[] =
{
	{"NewRadioEnable", 2, 37},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationSetAutoRateFallBackMode[] =
{
	{"NewAutoRateFallBackEnabled", 1, 38},
	{0 ,0 , 0}
};

static const struct argument WLANConfigurationGetAutoRateFallBackMode[] =
{
	{"NewAutoRateFallBackEnabled", 2, 38},
	{0 ,0 , 0}
};

static const struct action scpdWLANConfigurationActions[] =
{
	{"SetEnable", WLANConfigurationSetEnable},
	{"GetInfo", WLANConfigurationGetInfo},
	{"SetConfig", WLANConfigurationSetConfig},
	{"SetSecurityKeys", WLANConfigurationSetSecurityKeys},
	{"GetSecurityKeys", WLANConfigurationGetSecurityKeys},
	{"SetDefaultWEPKeyIndex", WLANConfigurationSetDefaultWEPKeyIndex},
	{"GetDefaultWEPKeyIndex", WLANConfigurationGetDefaultWEPKeyIndex},
	{"SetBasBeaconSecurityProperties", WLANConfigurationSetBasBeaconSecurityProperties},
	{"GetBasBeaconSecurityProperties", WLANConfigurationGetBasBeaconSecurityProperties},
	{"SetWPABeaconSecurityProperties", WLANConfigurationSetWPABeaconSecurityProperties},
	{"GetWPABeaconSecurityProperties", WLANConfigurationGetWPABeaconSecurityProperties},
	{"GetStatistics", WLANConfigurationGetStatistics},

	//The following actions are needed only for advanced use.
	{"SetSSID", WLANConfigurationSetSSID},
	{"GetSSID", WLANConfigurationGetSSID},
	{"GetBSSID", WLANConfigurationGetBSSID},
	{"SetBeaconType", WLANConfigurationSetBeaconType},
	{"GetBeaconType", WLANConfigurationGetBeaconType},
	{"SetChannel", WLANConfigurationSetChannel},
  {"GetChannelInfo", WLANConfigurationGetChannelInfo},
	{"SetDataTransmitRates", WLANConfigurationSetDataTransmitRates},
	{"GetDataTransmitRateInfo", WLANConfigurationGetDataTransmitRateInfo},
	{"GetPacketStatistics", WLANConfigurationGetPacketStatistics},
	{"GetTotalAssociations", WLANConfigurationGetTotalAssociations},
	{"GetGenericAssociatedDeviceInfo", WLANConfigurationGetGenericAssociatedDeviceInfo},
	{"GetSpecificAssociatedDeviceInfo", WLANConfigurationGetSpecificAssociatedDeviceInfo},
	{"SetRadioMode", WLANConfigurationSetRadioMode},
	{"GetRadioMode", WLANConfigurationGetRadioMode},
	{"SetAutoRateFallBackMode", WLANConfigurationSetAutoRateFallBackMode},
	{"GetAutoRateFallBackMode", WLANConfigurationGetAutoRateFallBackMode},
	{0, 0}
};

static const struct stateVar scpdHostsVars[] =
{
	{"HostNumberOfEntries", 2, 0},   // >= 0    // Index 0
	{"IPAddress", 0, 0},     // IP address
	{"AddressSource", 2, 0, 43},
	{"LeaseTimeRemaining", 6, 0},  // >= -1, -1 = infinite lease
	{"MACAddress", 2, 0},      // MAC address
	{"HostName", 2, 0},            // Index 5
	{"InterfaceType", 2, 0, 98},
	{"Active", 1, 0},
	{0, 0, 0}
};

static const struct argument HostsGetHostNumberOfEntries[] =
{
	{"NewHostNumberOfEntries", 2, 0},
	{0 ,0 , 0}
};

static const struct argument HostsGetSpecificHostEntry[] =
{
	{"NewMACAddress", 1, 4},
	{"NewIPAddress", 2, 1},
	{"NewAddressSource", 2, 2},
	{"NewLeaseTimeRemaining", 2, 3},
	{"NewInterfaceType", 2, 6},
	{"NewActive", 2, 7},
	{"NewHostName", 2, 5},
	{0 ,0 , 0}
};

static const struct argument HostsGetGenericHostEntry[] =
{
	{"NewIndex", 1, 4},
	{"NewIPAddress", 2, 1},
	{"NewAddressSource", 2, 2},
	{"NewLeaseTimeRemaining", 2, 3},
	{"NewInterfaceType", 2, 6},
	{"NewActive", 2, 7},
	{"NewHostName", 2, 5},
	{0 ,0 , 0}
};

static const struct action scpdHostsActions[] =
{
	{"GetHostNumberOfEntries", HostsGetHostNumberOfEntries},
	{"GetSpecificHostEntry", HostsGetSpecificHostEntry},
	{"GetGenericHostEntry", HostsGetGenericHostEntry},
	{0, 0}
};

static const struct serviceDesc scpdLANHostConfigManagement = { LANHostConfigManagementActions, LANHostConfigManagementVars };
static const struct serviceDesc scpdLANEthernetInterfaceConfig = { LANEthernetInterfaceConfigActions, LANEthernetInterfaceConfigVars };
static const struct serviceDesc scpdWLANConfiguration = { scpdWLANConfigurationActions, scpdWLANConfigurationVars };
static const struct serviceDesc scpdHosts = { scpdHostsActions, scpdHostsVars };

static const struct stateVar WANDSLConnectionManagementVars[] =
{
	{"WANConnectionDevice", 0, 0},
	{"WANConnectionService", 0, 0},
	{"DestinationAddress", 0, 0},
	{"LinkType", 0, 0, 139},
	{"ConnectionType", 0, 0, 14},
	{"Name", 0, 0},                                   // Index 5
	{"WANConnectionServiceNumberOfEntries", 0, 0},
	{0, 0, 0}
};

static const struct argument WANDCMAGetWANConnectionServiceNumberOfEntries[] =
{
	{"NewWANConnectionServiceNumberOfEntries", 2, 6},
	{0, 0, 0}
};

static const struct argument WANDCMAGetGenericConnectionServiceEntry[] =
{
	{"NewConnectionServiceIndex", 1, 6},
	{"NewWANConnectionDevice", 2, 0},
	{"NewWANConnectionService", 2, 0},
	{"NewName", 2, 5},
	{0, 0, 0}
};

static const struct argument WANDCMAGetSpecificConnectionServiceEntry[] =
{
	{"NewWANConnectionService", 1, 0},
	{"NewWANConnectionDevice", 2, 0},
	{"NewName", 2, 5},
	{0, 0, 0}
};

static const struct argument WANDCMAAddConnectionDeviceAndService[] =
{
	{"NewDestinationAddress", 1, 2},
	{"NewLinkType", 1, 3},
	{"NewConnectionType", 1, 4},
	{"NewName", 1, 5},
	{"NewWANConnectionDevice", 2, 0},
	{"NewWANConnectionService", 2, 1},
	{0, 0, 0}
};

static const struct argument WANDCMAAddConnectionService[] =
{
	{"NewWANConnectionDevice", 1, 0},
	{"NewConnectionType", 1, 4},
	{"NewName", 1, 5},
	{"NewWANConnectionService", 2, 1},
	{0, 0, 0}
};

static const struct argument WANDCMADeleteConnectionService[] =
{
	{"NewWANConnectionService", 1, 1},
	{0, 0, 0}
};

static const struct action WANDSLConnectionManagementActions[] =
{
	{"GetWANConnectionServiceNumberOfEntries", WANDCMAGetWANConnectionServiceNumberOfEntries},
	{"GetGenericConnectionServiceEntry", WANDCMAGetGenericConnectionServiceEntry},
	{"GetSpecificConnectionServiceEntry", WANDCMAGetSpecificConnectionServiceEntry},
	{"AddConnectionDeviceAndService", WANDCMAAddConnectionDeviceAndService},
	{"AddConnectionService", WANDCMAAddConnectionService},
	{"DeleteConnectionService", WANDCMADeleteConnectionService},
	{0, 0}
};

static const struct stateVar WANDSLInterfaceConfigVars[] =
{
	{"Enable", 1, 0},              // Index 0
	{"Status", 0, 0, 109},
	{"ModulationType", 0, 0, 116},       // Optional
	{"LineEncoding", 0, 0, 129},    // Optional
	{"LineNumber", 7, 0},               // Optional
	{"UpstreamCurrRate", 3, 0},                          // Index 5
	{"DownstreamCurrRate", 3, 0},
	{"UpstreamMaxRate", 3, 0},
	{"DownstreamMaxRate", 3, 0},
	{"UpstreamNoiseMargin", 6, 0},
	{"DownstreamNoiseMargin", 6, 0},                     // Index 10
	{"UpstreamAttenuation", 6, 0},
	{"DownstreamAttenuation", 6, 0},
	{"UpstreamPower", 6, 0},
	{"DownstreamPower", 6, 0},
	{"DataPath", 0, 0, 136},          // Optional        // Index 15
	{"InterleavedDepth", 2, 0},    // Optional
	{"ATURVendor", 0, 0},
	{"ATURCountry", 3, 0},
	{"ATURANSIStd", 3, 0},
	{"ATURANSIRev", 3, 0},                              // Index 20
	{"ATUCVendor", 0, 0},
	{"ATUCCountry", 3, 0},
	{"ATUCANSIStd", 3, 0},
	{"ATUCANSIRev", 3, 0},
	{"TotalStart", 3, 0},                              // Index 25
	{"ShowtimeStart", 3, 0},
	{"LastShowtimeStart", 3, 0},
	{"CurrentDayStart", 3, 0},
	{"QuarterHourStart", 3, 0},
	{"Stats.Total.ReceiveBlocks", 3, 0},               // Index 30
	{"Stats.Total.TransmitBlocks", 3, 0},
	{"Stats.Total.CellDelin", 3, 0},
	{"Stats.Total.LinkRetrain", 3, 0},
	{"Stats.Total.InitErrors", 3, 0},
	{"Stats.Total.InitTimeouts", 3, 0},                // Index 35
	{"Stats.Total.LossOfFraming", 3, 0},
	{"Stats.Total.ErroredSecs", 3, 0},
	{"Stats.Total.SeverelyErroredSecs", 3, 0},
	{"Stats.Total.FECErrors", 3, 0},
	{"Stats.Total.ATUCFECErrors", 3, 0},              // Index 40
	{"Stats.Total.HECErrors", 3, 0},
	{"Stats.Total.ATUCHECErrors", 3, 0},
	{"Stats.Total.CRCErrors", 3, 0},
	{"Stats.Total.ATUCCRCErrors", 3, 0},
	{0, 0, 0}
};

static const struct argument WANDICASetEnable[] =
{
	{"NewEnable", 1, 0},
	{0, 0, 0}
};

static const struct argument WANDICAGetInfo[] =
{
	{"NewEnable", 2, 0},
	{"NewStatus", 2, 1},
	{"NewModulationType", 2, 2},
	{"NewLineEncoding", 2, 3},
	{"NewLineNumber", 2, 4},
	{"NewUpstreamCurrRate", 2, 5},
	{"NewDownstreamCurrRate", 2, 6},
	{"NewUpstreamMaxRate", 2, 7},
	{"NewDownstreamMaxRate", 2, 8},
	{"NewUpstreamNoiseMargin", 2, 9},
	{"NewDownstreamNoiseMargin", 2, 10},
	{"NewUpstreamAttenuation", 2, 11},
	{"NewDownstreamAttenuation", 2, 12},
	{"NewUpstreamPower", 2, 13},
	{"NewDownstreamPower", 2, 14},
	{"NewDataPath", 2, 15},
	{"NewInterleavedDepth", 2, 16},
	{"NewATURVendor", 2, 17},
	{"NewATURCountry", 2, 18},
	{"NewATURANSIStd", 2, 19},
	{"NewATURANSIRev", 2, 20},
	{"NewATUCVendor", 2, 21},
	{"NewATUCCountry", 2, 22},
	{"NewATUCANSIStd", 2, 23},
	{"NewATUCANSIRev", 2, 24},
	{"NewTotalStart", 2, 25},
	{"NewShowtimeStart", 2, 26},
	{"NewLastShowtimeStart", 2, 27},
	{"NewCurrentDayStart", 2, 28},
	{"NewQuarterHourStart", 2, 29},
	{0, 0, 0}
};

static const struct argument WANDICAGetStatisticsTotal[] =
{
	{"NewReceiveBlocks", 2, 30},
	{"NewTransmitBlocks", 2, 31},
	{"NewCellDelin", 2, 32},
	{"NewLinkRetrain", 2, 33},
	{"NewInitErrors", 2, 34},
	{"NewInitTimeouts", 2, 35},
	{"NewLossOfFraming", 2, 36},
	{"NewErroredSecs", 2, 37},
	{"NewSeverelyErroredSecs", 2, 38},
	{"NewFECErrors", 2, 39},
	{"NewATUCFECErrors", 2, 40},
	{"NewHECErrors", 2, 41},
	{"NewATUCHECErrors", 2, 42},
	{"NewCRCErrors", 2, 43},
	{"NewATUCCRCErrors", 2, 44},
	{0, 0, 0}
};

static const struct action WANDSLInterfaceConfigActions[] =
{
	{"SetEnable", WANDICASetEnable},
	{"GetInfo", WANDICAGetInfo},
	{"GetStatisticsTotal", WANDICAGetStatisticsTotal},
	//{"GetStatisticsShowtime", WANDICAGetStatisticsShowtime},
	{0, 0}
};

#if 0   // WANDSLLinkConfig move to as a standard miniupnp igd service
static const struct argument WANDLCASetEnable[] =
{
	{"NewEnable", 1, 0},
	{0, 0, 0}
};

static const struct argument WANDLCAGetInfo[] =
{
	{"NewEnable", 2, 0},
	{"NewLinkType", 2, 1},
	{"NewLinkStatus", 2, 2},
	{"NewAutoConfig", 2, 3},
	{"NewModulationType", 2, 4},
	{"NewDestinationAddress", 2, 5},
	{"NewATMEncapsulation", 2, 6},
	{"NewFCSPreserved", 2, 7},
	{"NewVCSearchList", 2, 8},
	{"NewATMAAL", 2, 9},
	{"NewATMQoS", 2, 12},
	{"NewATMPeakCellRate", 2, 13},
	{"NewATMMaximumBurstSize", 2, 14},
	{"NewATMSustainableCellRate", 2, 15},
	{0, 0, 0}
};

static const struct argument WANDLCASetDSLLinkType[] =
{
	{"NewLinkType", 1, 1},
	{0, 0, 0}
};

static const struct argument WANDLCAGetDSLLinkInfo[] =
{
	{"NewLinkType", 2, 1},
	{0, 0, 0}
};

static const struct argument WANDLCAGetAutoConfig[] =
{
	{"NewAutoConfig", 2, 3},
	{0, 0, 0}
};

static const struct argument WANDLCAGetDestinationAddress[] =
{
	{"NewDestinationAddress", 2, 5},
	{0, 0, 0}
};

static const struct argument WANDLCASetDestinationAddress[] =
{
	{"NewDestinationAddress", 1, 5},
	{0, 0, 0}
};

static const struct argument WANDLCAGetStatistics[] =
{
	{"NewATMTransmittedBlocks", 2, 10},
	{"NewATMReceivedBlocks", 2, 11},
	{"NewAAL5CRCErrors", 2, 16},
	{"NewATMCRCErrors", 2, 17},
	{"NewATMHECErrors", 2, 18},
	{0, 0, 0}
};

static const struct argument WANDLCASetATMEncapsulation[] =
{
	{"NewATMEncapsulation", 1, 6},
	{0, 0, 0}
};

static const struct argument WANDLCAGetATMEncapsulation[] =
{
	{"NewATMEncapsulation", 2, 6},
	{0, 0, 0}
};

static const struct action WANDSLLinkConfigActions[] =
{
	{"SetEnable", WANDLCASetEnable},
	{"GetInfo", WANDLCAGetInfo},
	{"SetDSLLinkType", WANDLCASetDSLLinkType},
	{"GetDSLLinkInfo", WANDLCAGetDSLLinkInfo},
	{"GetAutoConfig", WANDLCAGetAutoConfig},
	{"GetDestinationAddress", WANDLCAGetDestinationAddress},
	{"SetDestinationAddress", WANDLCASetDestinationAddress},
	{"GetStatistics", WANDLCAGetStatistics},
	{"SetATMEncapsulation", WANDLCASetATMEncapsulation},
	{"GetATMEncapsulation", WANDLCAGetATMEncapsulation},
	{0, 0}
};

static const struct stateVar WANDSLLinkConfigVars[] =
{
	{"Enable", 1, 0},
	{"LinkType", 0, 0, 139},
	{"LinkStatus", 0, 0, 6},
	{"AutoConfig", 1, 0},
	{"ModulationType", 0, 0, 116},         // Optional
/*5*/   {"DestinationAddress", 0, 0/*, 105*/},
	{"ATMEncapsulation", 0, 0, 146},     // Optional
	{"FCSPreserved", 1, 0},                        // Optional
	{"VCSearchList", 0, 0},                         // Optional
	{"ATMAAL", 0, 0, 149},               // Optional
/*10*/  {"ATMTransmittedBlocks", 3, 0},
	{"ATMReceivedBlocks", 3, 0},
	{"ATMQoS", 0, 0, 155},        // Optional
	{"ATMPeakCellRate", 3, 0},            // Optional
	{"ATMMaximumBurstSize", 3, 0},    // Optional
/*15*/  {"ATMSustainableCellRate", 3, 0}, // Optional
	{"AAL5CRCErrors", 3, 0},
	{"ATMCRCErrors", 3, 0},
	{"ATMHECErrors", 3, 0},               // Optional
	{0, 0, 0}
};
#endif

static const struct stateVar WANEthernetInterfaceConfigVars[] =
{
	{"Enable", 1, 0},
	{"Status", 0, 0, 47},
	{"MACAddress", 0, 0},
	{"MACAddressControlEnabled", 1, 0},   // Optional
	{"MaxBitRate", 0, 0, 52},
	{"DuplexMode", 0, 0, 57},             // Index 5
	{"Stats.BytesSent", 3, 0},      // >=0
	{"Stats.BytesReceived", 3, 0},   // >=0
	{"Stats.PacketsSent", 3, 0},      // >=0
	{"Stats.PacketsReceived", 3, 0},      // >=0
	{0, 0, 0}
};

static const struct argument WANEthernetInterfaceSetEnable[] =
{
	{"NewEnable", 1, 0},
	{0, 0, 0}
};

static const struct argument WANEthernetInterfaceSetMaxBitRate[] =
{
	{"NewMaxBitRate", 1, 4},
	{0, 0, 0}
};

static const struct argument WANEthernetInterfaceNewDuplexMode[] =
{
	{"NewDuplexMode", 1, 5},
	{0, 0, 0}
};

static const struct argument WANEthernetInterfaceGetInfo[] =
{
	{"NewEnable", 2, 0},
	{"NewStatus", 2, 1},
	{"NewMACAddress", 2, 2},
	{"NewMACAddressControlEnabled", 2, 3},
	{"NewMaxBitRate", 2, 4},
	{"NewDuplexMode", 2, 5},
	{0, 0, 0}
};

static const struct argument WANEthernetInterfaceGetStatistics[] =
{
	{"NewBytesSent", 2, 6},
	{"NewBytesReceived", 2, 7},
	{"NewPacketsSent", 2, 8},
	{"NewPacketsReceived", 2, 9},
	{0, 0, 0}
};

static const struct argument WANEthernetInterfaceSetMACAddressControlEnabled[] =
{
	{"NewMACAddressControlEnabled", 1, 3},
	{0, 0, 0}
};

static const struct action WANEthernetInterfaceConfigActions[] =
{
	{"SetEnable", WANEthernetInterfaceSetEnable},
	{"SetMaxBitRate", WANEthernetInterfaceSetMaxBitRate},      // Optional
	{"SetDuplexMode", WANEthernetInterfaceNewDuplexMode},     // Optional
	{"GetInfo", WANEthernetInterfaceGetInfo},
	{"GetStatistics", WANEthernetInterfaceGetStatistics},
	{"SetMACAddressControlEnabled", WANEthernetInterfaceSetMACAddressControlEnabled},    // Optional
	{0, 0}
};

static const struct argument WANPPPCnASetEnable[] =
{
	{"NewEnable", 1, 0},
	{0, 0, 0}
};

static const struct argument WANPPPCnAGetInfo[] =
{
	{"NewEnable", 2, 0},
	{"NewConnectionType", 2, 1},
	{"NewPossibleConnectionTypes", 2, 2},
	{"NewConnectionStatus", 2, 3},
	{"NewName", 2, 4},
	{"NewUptime", 2, 5},
	{"NewUpstreamMaxBitRate", 2, 6},
	{"NewDownstreamMaxBitRate", 2, 7},
	{"NewLastConnectionError", 2, 8},
	{"NewAutoDisconnectTime", 2, 9},
	{"NewIdleDisconnectTime", 2, 10},
	{"NewWarnDisconnectDelay", 2, 11},
	{"NewConnectionTrigger", 2, 12},
	{"NewRSIPAvailable", 2, 13},
	{"NewNATEnabled", 2, 14},
	{"NewUserName", 2, 15},
	{"NewPPPEncryptionProtocol", 2, 17},
	{"NewPPPCompressionProtocol", 2, 18},
	{"NewPPPAuthenticationProtocol", 2, 19},
	{"NewExternalIPAddress",2, 20},
	{"NewRemoteIPAddress", 2, 21},
	{"NewMACAddress", 2, 33},
	{"NewMACAddressOverride", 2, 34},
	{"NewMaxMRUSize", 2, 22},
	{"NewCurrentMRUSize", 2, 23},
	{"NewDNSEnabled", 2, 35},
	{"NewDNSOverrideAllowed", 2, 36},
	{"NewDNSServers", 2, 37},
	{"NewTransportType", 2, 38},
	{"NewPPPoEACName", 2, 39},
	{"NewPPPoEServiceName", 2, 40},
	{"NewRouteProtocolRx", 2, 41},
	{"NewPPPLCPEcho", 2, 42},
	{"NewPPPLCPEchoRetry", 2, 43},
	{0, 0, 0}
};

static const struct argument WANPPPCnASetConnectionType[] =
{
	{"NewConnectionType", 1, 1},
	{0, 0, 0}
};

static const struct argument WANPPPCnAGetConnectionTypeInfo[] =
{
	{"NewConnectionType", 2, 1},
	{"NewPossibleConnectionTypes", 2, 2},
	{0, 0, 0}
};

static const struct argument WANPPPCnASetIdleDisconnectTime[] =
{
	{"NewIdleDisconnectTime", 1, 10},
	{0, 0, 0}
};

static const struct argument WANPPPCnAGetStatusInfo[] =
{
	{"NewConnectionStatus", 2, 3},
	{"NewLastConnectionError", 2, 8},
	{"NewUptime", 2, 5},
	{0, 0, 0}
};

static const struct argument WANPPPCnAGetUserName[] =
{
	{"NewUserName", 2, 15},
	{0, 0, 0}
};

static const struct argument WANPPPCnASetUserName[] =
{
	{"NewUserName", 1, 15},
	{0, 0, 0}
};

static const struct argument WANPPPCnASetPassword[] =
{
	{"NewPassword", 1, 16},
	{0, 0, 0}
};

static const struct argument WANPPPCnAGetIdleDisconnectTime[] =
{
	{"NewIdleDisconnectTime", 2, 10},
	{0, 0, 0}
};

static const struct argument WANPPPCnASetPPPoEService[] =
{
	{"NewPPPoEACName", 1, 39},
	{"NewPPPoEServiceName", 1, 40},
	{0, 0, 0}
};

static const struct argument WANPPPCnASetConnectionTrigger[] =
{
	{"NewConnectionTrigger", 1, 12},
	{0, 0, 0}
};

static const struct argument WANPPPCnAGetPortMappingNumberOfEntries[] =
{
	{"NewPortMappingNumberOfEntries", 2, 24},
	{0, 0, 0}
};

static const struct argument WANPPPCnAGetGenericPortMappingEntry[] =
{
	{"NewPortMappingIndex", 1, 24},
	{"NewRemoteHost", 2, 27},
	{"NewExternalPort", 2, 28},
	{"NewPortMappingProtocol", 2,30},
	{"NewInternalPort", 2, 29},
	{"NewInternalClient", 2, 31},
	{"NewPortMappingEnabled", 2, 25},
	{"NewPortMappingDescription", 2, 32},
	{"NewPortMappingLeaseDuration", 2, 26},
	{0, 0, 0}
};

static const struct argument WANPPPCnAGetSpecificPortMappingEntry[] =
{
	{"NewRemoteHost", 1, 27},
	{"NewExternalPort", 1, 28},
	{"NewPortMappingProtocol", 1,30},
	{"NewInternalPort", 2, 29},
	{"NewInternalClient", 2, 31},
	{"NewPortMappingEnabled", 2, 25},
	{"NewPortMappingDescription", 2, 32},
	{"NewPortMappingLeaseDuration", 2, 26},
	{0, 0, 0}
};

static const struct argument WANPPPCnAAddPortMapping[] =
{
	{"NewRemoteHost", 1, 27},
	{"NewExternalPort", 1, 28},
	{"NewPortMappingProtocol", 1,30},
	{"NewInternalPort", 1, 29},
	{"NewInternalClient", 1, 31},
	{"NewPortMappingEnabled", 1, 25},
	{"NewPortMappingDescription", 1, 32},
	{"NewPortMappingLeaseDuration", 1, 26},
	{0, 0, 0}
};

static const struct argument WANPPPCnADeletePortMapp[] =
{
	{"NewRemoteHost", 1, 27},
	{"NewExternalPort", 1, 28},
	{"NewPortocol", 1,30},
	{0, 0, 0}
};

static const struct action WANPPPCnActions[] =
{
	{"SetEnable", WANPPPCnASetEnable},
	{"GetInfo", WANPPPCnAGetInfo},
	{"SetConnectionType", WANPPPCnASetConnectionType},
	{"GetConnectionTypeInfo", WANPPPCnAGetConnectionTypeInfo},
	{"RequestConnection", 0},
	{"RequestTermination", 0},
	{"ForceTermination", 0},
	{"SetIdleDisconnectTime", WANPPPCnASetIdleDisconnectTime},
	{"GetStatusInfo", WANPPPCnAGetStatusInfo},
	{"GetUserName", WANPPPCnAGetUserName},
	{"SetUserName", WANPPPCnASetUserName},
	{"SetPassword", WANPPPCnASetPassword},
	{"GetIdleDisconnectTime", WANPPPCnAGetIdleDisconnectTime},
	{"SetPPPoEService", WANPPPCnASetPPPoEService},
	{"SetConnectionTrigger", WANPPPCnASetConnectionTrigger},
	{"GetPortMappingNumberOfEntries", WANPPPCnAGetPortMappingNumberOfEntries},
	{"GetGenericPortMappingEntry", WANPPPCnAGetGenericPortMappingEntry},
	{"GetSpecificPortMappingEntry", WANPPPCnAGetSpecificPortMappingEntry},
	{"AddPortMapping", WANPPPCnAAddPortMapping},
	{"DeletePortMapp", WANPPPCnADeletePortMapp},
	{0, 0}
};

static const struct stateVar WANPPPCnVars[] =
{
	{"Enable", 1, 0},
	{"ConnectionType", 0, 0},
	{"PossibleConnectionTypes", 0, 0, 163},
	{"ConnectionStatus", 0, 0, 171},
	{"Name", 0, 0},
/*5*/   {"Uptime", 3, 0},
	{"UpstreamMaxBitRate", 3, 0},
	{"DownstreamMaxBitRate", 3, 0},
	{"LastConnectionError", 0, 0, 179},
	{"AutoDisconnectTime", 3, 0},                    // Optional
/*10*/  {"IdleDisconnectTime", 3, 0},         // Optional
	{"WarnDisconnectDelay", 3, 0},                 // Optional
	{"ConnectionTrigger", 0, 0, 203},
	{"RSIPAvailable", 1, 0},
	{"NATEnabled", 1, 0},
/*15*/  {"UserName", 0, 0},    // Alphanumeric text
	{"Password", 0, 0},                 // Alphanumeric text
	{"PPPEncryptionProtocol", 0, 0},           // Optional
	{"PPPCompressionProtocol", 0, 0, 207},       // Optional
	{"PPPAuthenticationProtocol", 0, 0, 210},     // Optional
/*20*/  {"ExternalIPAddress", 0, 0},
	{"RemoteIPAddress", 0, 0},                        // Optional
	{"MaxMRUSize", 2, 0},                              // Optional
	{"CurrentMRUSize", 2, 0},                          // Optional
	{"PortMappingNumberOfEntries", 2, 0},
/*25*/  {"PortMappingEnabled", 1, 0},
	{"PortMappingLeaseDuration", 3, 0},
	{"RemoteHost", 0, 0},
	{"ExternalPort", 2, 0},
	{"InternalPort", 2, 0},
/*30*/  {"PortMappingProtocol", 0, 0, 11},
	{"InternalClient", 0, 0},
	{"PortMappingDescription", 0, 0},
	{"MACAddress", 0, 0},
	{"MACAddressOverride", 1, 0},                 // Optional
/*35*/  {"DNSEnabled", 1, 0},
	{"DNSOverrideAllowed", 1, 0},
	{"DNSServers", 0, 0},
	{"TransportType", 0, 0, 214},
	{"PPPoEACName", 0, 0},
/*40*/  {"PPPoEServiceName", 0, 0},
	{"RouteProtocolRx", 0, 0, 219},
	{"PPPLCPEcho", 2, 0},                            // Optional
	{"PPPLCPEchoRetry", 2, 0},                    // Optional
	{0, 0, 0}
};

static const struct serviceDesc scpdWANDSLConnectionManagement = { WANDSLConnectionManagementActions, WANDSLConnectionManagementVars };
static const struct serviceDesc scpdWANDSLInterfaceConfig = { WANDSLInterfaceConfigActions, WANDSLInterfaceConfigVars };
static const struct serviceDesc scpdWANEthernetInterfaceConfig = { WANEthernetInterfaceConfigActions, WANEthernetInterfaceConfigVars };
static const struct serviceDesc scpdWANPPPCn = { WANPPPCnActions, WANPPPCnVars };
#endif

#ifdef ENABLE_L3F_SERVICE
/* Read UPnP_IGD_Layer3Forwarding_1.0.pdf */
static const struct argument SetDefaultConnectionServiceArgs[] =
{
	{"NewDefaultConnectionService", 1, 0}, /* in */
	{0, 0, 0}
};

static const struct argument GetDefaultConnectionServiceArgs[] =
{
	{"NewDefaultConnectionService", 0, 0}, /* out */
	{0, 0, 0}
};

static const struct action L3FActions[] =
{
	{"SetDefaultConnectionService", SetDefaultConnectionServiceArgs}, /* Req */
	{"GetDefaultConnectionService", GetDefaultConnectionServiceArgs}, /* Req */
	{0, 0}
};

static const struct stateVar L3FVars[] =
{
	{"DefaultConnectionService", 0|0x80, 0}, /* Required */
	{0, 0, 0}
};

static const struct serviceDesc scpdL3F = { L3FActions, L3FVars };
#endif

/* strcat_str()
 * concatenate the string and use realloc to increase the
 * memory buffer if needed. */
static char *
strcat_str(char * str, int * len, int * tmplen, const char * s2)
{
	int s2len;
	s2len = (int)strlen(s2);
	if(*tmplen <= (*len + s2len))
	{
		if(s2len < 256)
			*tmplen += 256;
		else
			*tmplen += s2len;
		str = (char *)realloc(str, *tmplen);
	}
	/*strcpy(str + *len, s2); */
	memcpy(str + *len, s2, s2len + 1);
	*len += s2len;
	return str;
}

/* strcat_char() :
 * concatenate a character and use realloc to increase the
 * size of the memory buffer if needed */
static char *
strcat_char(char * str, int * len, int * tmplen, char c)
{
	if(*tmplen <= (*len + 1))
	{
		*tmplen += 256;
		str = (char *)realloc(str, *tmplen);
	}
	str[*len] = c;
	(*len)++;
	return str;
}

/* iterative subroutine using a small stack
 * This way, the progam stack usage is kept low */
static char *genXML(char * str, int * len, int * tmplen, const struct XMLElt * p)
{
	unsigned short i, j, k;
	int top;
	const char * eltname, *s;
	char c;
	struct {
		unsigned short i;
		unsigned short j;
		const char * eltname;
	} pile[16]; /* stack */
	top = -1;
	i = 0;	/* current node */
	j = 1;	/* i + number of nodes*/
	for(;;)
	{
		eltname = p[i].eltname;
		if(!eltname)
			return str;
		if(eltname[0] == '/')
		{
			/*printf("<%s>%s<%s>\n", eltname+1, p[i].data, eltname); */
			str = strcat_char(str, len, tmplen, '<');
			str = strcat_str(str, len, tmplen, eltname+1);
			str = strcat_char(str, len, tmplen, '>');
			str = strcat_str(str, len, tmplen, p[i].data);
			str = strcat_char(str, len, tmplen, '<');
			str = strcat_str(str, len, tmplen, eltname);
			str = strcat_char(str, len, tmplen, '>');
			str = strcat_char(str, len, tmplen, '\n');
			for(;;)
			{
				if(top < 0)
					return str;
				i = ++(pile[top].i);
				j = pile[top].j;
				/*printf("  pile[%d]\t%d %d\n", top, i, j); */
				if(i==j)
				{
					/*printf("</%s>\n", pile[top].eltname); */
					str = strcat_char(str, len, tmplen, '<');
					str = strcat_char(str, len, tmplen, '/');
					s = pile[top].eltname;
					for(c = *s; c > ' '; c = *(++s))
						str = strcat_char(str, len, tmplen, c);

					str = strcat_char(str, len, tmplen, '>');
					str = strcat_char(str, len, tmplen, '\n');
					top--;
				}
				else
					break;
			}
		}
		else
		{
			/*printf("<%s>\n", eltname); */
			str = strcat_char(str, len, tmplen, '<');
			str = strcat_str(str, len, tmplen, eltname);
			str = strcat_char(str, len, tmplen, '>');
			str = strcat_char(str, len, tmplen, '\n');
			k = i;
			/*i = p[k].index; */
			/*j = i + p[k].nchild; */
			i = (unsigned)p[k].data & 0xffff;
			j = i + ((unsigned)p[k].data >> 16);
			top++;
			/*printf(" +pile[%d]\t%d %d\n", top, i, j); */
			pile[top].i = i;
			pile[top].j = j;
			pile[top].eltname = eltname;
		}
	}
}

/* genRootDesc() :
 * - Generate the root description of the UPnP device.
 * - the len argument is used to return the length of
 *   the returned string.
 * - tmp_uuid argument is used to build the uuid string */
char *genRootDesc(int * len)
{
	char * str;
	int tmplen;
	tmplen = 2048;
	str = (char *)malloc(tmplen);
	if(str == NULL)
		return NULL;
	* len = strlen(xmlver);
	/*strcpy(str, xmlver); */
	memcpy(str, xmlver, *len + 1);
	str = genXML(str, len, &tmplen, rootDesc);
	str[*len] = '\0';
	return str;
}

/* genServiceDesc() :
 * Generate service description with allowed methods and
 * related variables. */
static char *genServiceDesc(int * len, const struct serviceDesc * s)
{
	int i, j;
	const struct action * acts;
	const struct stateVar * vars;
	const struct argument * args;
	char * str;
	int tmplen;
	tmplen = 2048;
	str = (char *)malloc(tmplen);
	if(str == NULL)
		return NULL;
	/*strcpy(str, xmlver); */
	*len = strlen(xmlver);
	memcpy(str, xmlver, *len + 1);

	acts = s->actionList;
	vars = s->serviceStateTable;

	str = strcat_char(str, len, &tmplen, '<');
	str = strcat_str(str, len, &tmplen, root_service);
	str = strcat_char(str, len, &tmplen, '>');

	str = strcat_str(str, len, &tmplen, "\n<specVersion>\n<major>1</major>\n<minor>0</minor>\n</specVersion>\n");

	i = 0;
	str = strcat_str(str, len, &tmplen, "<actionList>\n");
	while(acts[i].name)
	{
		str = strcat_str(str, len, &tmplen, "<action>\n<name>");
		str = strcat_str(str, len, &tmplen, acts[i].name);
		str = strcat_str(str, len, &tmplen, "</name>\n");
		/* argument List */
		args = acts[i].args;
		if(args)
		{
			str = strcat_str(str, len, &tmplen, "<argumentList>\n");
			j = 0;
			while(args[j].name)
			{
				str = strcat_str(str, len, &tmplen, "<argument>\n<name>");
				str = strcat_str(str, len, &tmplen, args[j].name);
				str = strcat_str(str, len, &tmplen, "</name>\n<direction>");
				str = strcat_str(str, len, &tmplen, args[j].dir==1?"in":"out");
				str = strcat_str(str, len, &tmplen,
						"</direction>\n<relatedStateVariable>");
				str = strcat_str(str, len, &tmplen,
						vars[args[j].relatedVar].name);
				str = strcat_str(str, len, &tmplen,
						"</relatedStateVariable>\n</argument>\n");
				j++;
			}
			str = strcat_str(str, len, &tmplen,"</argumentList>\n");
		}
		str = strcat_str(str, len, &tmplen, "</action>\n");
		/*str = strcat_char(str, len, &tmplen, '\n'); // TEMP ! */
		i++;
	}
	str = strcat_str(str, len, &tmplen, "</actionList>\n<serviceStateTable>\n");
	i = 0;
	while(vars[i].name)
	{
		str = strcat_str(str, len, &tmplen, "<stateVariable sendEvents=\"");
#ifdef ENABLE_EVENTS
		str = strcat_str(str, len, &tmplen, (vars[i].itype & 0x80)?"yes":"no");
#else
		/* for the moment allways send no. Wait for SUBSCRIBE implementation
		 * before setting it to yes */
		str = strcat_str(str, len, &tmplen, "no");
#endif

		str = strcat_str(str, len, &tmplen, "\">\n<name>");
		str = strcat_str(str, len, &tmplen, vars[i].name);
		str = strcat_str(str, len, &tmplen, "</name>\n<dataType>");

#ifdef CONFIG_TR_064
		if(IS_TR064_ENABLED)
			str = strcat_str(str, len, &tmplen, upnptypes_tr064[vars[i].itype & 0x0f]);
		else
#endif
			str = strcat_str(str, len, &tmplen, upnptypes_igd[vars[i].itype & 0x0f]);

		str = strcat_str(str, len, &tmplen, "</dataType>\n");
		if(vars[i].iallowedlist)
		{
		  str = strcat_str(str, len, &tmplen, "<allowedValueList>\n");
		  for(j=vars[i].iallowedlist; upnpallowedvalues[j]; j++)
		  {
		    str = strcat_str(str, len, &tmplen, "<allowedValue>");
		    str = strcat_str(str, len, &tmplen, upnpallowedvalues[j]);
		    str = strcat_str(str, len, &tmplen, "</allowedValue>\n");
		  }
		  str = strcat_str(str, len, &tmplen, "</allowedValueList>\n");
		}
		/*if(vars[i].defaultValue) */
		if(vars[i].idefault)
		{
		  str = strcat_str(str, len, &tmplen, "<defaultValue>");
		  /*str = strcat_str(str, len, &tmplen, vars[i].defaultValue); */
		  str = strcat_str(str, len, &tmplen, upnpdefaultvalues[vars[i].idefault]);
			str = strcat_str(str, len, &tmplen, "</defaultValue>\n");
		}
		str = strcat_str(str, len, &tmplen, "</stateVariable>\n");
		/*str = strcat_char(str, len, &tmplen, '\n'); // TEMP ! */
		i++;
	}
	str = strcat_str(str, len, &tmplen, "</serviceStateTable>\n");
	str = strcat_str(str, len, &tmplen, "</scpd>\n");
	str[*len] = '\0';
	return str;
}

char *genWANDSLLink(int * len)
{
	return genServiceDesc(len, &scpdWANDSLLink);
}

/* genWANIPCn() :
 * Generate the WANIPConnection xml description */
char *genWANIPCn(int * len)
{
	return genServiceDesc(len, &scpdWANIPCn);
}

/* genWANCfg() :
 * Generate the WANInterfaceConfig xml description. */
char *genWANCfg(int * len)
{
	return genServiceDesc(len, &scpdWANCfg);
}

#ifdef ENABLE_L3F_SERVICE
char *genL3F(int * len)
{
	return genServiceDesc(len, &scpdL3F);
}
#endif

#ifdef CONFIG_TR_064
char *genDeviceInfo(int *len)
{
	return genServiceDesc(len, &scpdDeviceInfo);
}

char *genDeviceConfig(int *len)
{
	return genServiceDesc(len, &scpdDeviceConfig);
}

char *genLayer3Forwarding(int *len)
{
	return genServiceDesc(len, &scpdLayer3Forwarding);
}

char *genLANConfigSecurity(int *len)
{
	return genServiceDesc(len, &scpdLANConfigSecurity);
}

char *genManagementServer(int *len)
{
	return genServiceDesc(len, &scpdManagementServer);
}

char *genTime(int *len)
{
	return genServiceDesc(len, &scpdTime);
}

char *genLANHostConfigManagement(int *len)
{
	return genServiceDesc(len, &scpdLANHostConfigManagement);
}

char *genLANEthernetInterfaceConfig(int *len)
{
	return genServiceDesc(len, &scpdLANEthernetInterfaceConfig);
}

char* genWLANConfiguration(int *len)
{
	return genServiceDesc(len, &scpdWLANConfiguration);
}

char *genHosts(int *len)
{
	return genServiceDesc(len, &scpdHosts);
}

char *genWANDSLConnectionManagement(int *len)
{
	return genServiceDesc(len, &scpdWANDSLConnectionManagement);
}

char *genWANDSLInterfaceConfig(int *len)
{
	return genServiceDesc(len, &scpdWANDSLInterfaceConfig);
}

char *genWANEthernetInterfaceConfig(int *len)
{
	return genServiceDesc(len, &scpdWANEthernetInterfaceConfig);
}

char *genWANPPPCn(int *len)
{
	return genServiceDesc(len, &scpdWANPPPCn);
}
#endif


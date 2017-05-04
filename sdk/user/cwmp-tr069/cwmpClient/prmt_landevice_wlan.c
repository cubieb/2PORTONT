#include "cwmpc_utility.h"
#include "prmt_landevice_wlan.h"
#include "prmt_landevice.h"
#include <rtk/utility.h>
#ifdef _PRMT_X_CT_EXT_ENABLE_
#include "prmt_ctcom_lanext.h"
#endif


#ifdef WLAN_SUPPORT

#ifdef TELEFONICA_DEFAULT_CFG
#define _SHOW_WLAN_KEY_WHEN_GET_
#endif //TELEFONICA_DEFAULT_CFG

#define GETPSKINDEX(a,b) ( (unsigned char)(((a&0xf)<<4)|(b&0xf)) )

char		gLocationDescription[4096]={0};

int updateWLANAssociations(void);
int loadWLANAssInfoByInstNum( unsigned int instnum );
int getWLANSTAINFO(int id, WLAN_STA_INFO_T *info);
int getRateStr( unsigned short rate, char *buf );
int setRateStr( char *buf, unsigned short *rate );
int getIPbyMAC( char *mac, char *ip );

unsigned int getWLANConfInstNum( char *name );
unsigned int getWEPInstNum( char *name );
unsigned int getAssDevInstNum( char *name );
unsigned int getPreSharedKeyInstNum( char *name );

#ifdef E8B_NEW_DIAGNOSE
void writeSSIDFile(char *msg, int index)
{
	FILE *fp;
	char buf[32];

	fp = fopen(NEW_SETTING, "r+");
	if (fp) {
		fgets(buf, sizeof(buf), fp);
		if (strcmp(buf, SSID_FILE)) {
			clearssidfile();
		} else {
			rewind(fp);
			ftruncate(fileno(fp), 0);
			fprintf(fp, "%s", SSID_FILE);
		}
		fclose(fp);
	}

	sprintf(buf, "%s%d", SSID_FILE, index);
	fp = fopen(buf, "w");
	if (fp) {
		fprintf(fp, msg);
		fclose(fp);

		fp = fopen(REMOTE_SETSAVE_FILE, "r");
		if (fp) {
			fclose(fp);
			unlink(REMOTE_SETSAVE_FILE);
#ifdef CONFIG_USER_FLATFSD_XXX
			va_cmd("/bin/flatfsd", 1, 1, "-s");
#endif
		}
	}
}
#endif

struct CWMP_OP tPreSharedKeyEntityLeafOP = { getPreSharedKeyEntity, setPreSharedKeyEntity };
struct CWMP_PRMT tPreSharedKeyEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PreSharedKey",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPreSharedKeyEntityLeafOP},
{"KeyPassphrase",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPreSharedKeyEntityLeafOP},
{"AssociatedDeviceMACAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tPreSharedKeyEntityLeafOP}
};
enum ePreSharedKeyEntityLeaf
{
	ePreSharedKey,
	eKeyPassphrase,
	ePreAssociatedDeviceMACAddress
};
struct CWMP_LEAF tPreSharedKeyEntityLeaf[] =
{
{ &tPreSharedKeyEntityLeafInfo[ePreSharedKey] },
{ &tPreSharedKeyEntityLeafInfo[eKeyPassphrase] },
{ &tPreSharedKeyEntityLeafInfo[ePreAssociatedDeviceMACAddress] },
{ NULL }
};


struct CWMP_PRMT tPreSharedKeyObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"5",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"6",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"7",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"8",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"9",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"10",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum ePreSharedKeyObject
{
	ePreSharedKey1,
	ePreSharedKey2,
	ePreSharedKey3,
	ePreSharedKey4,
	ePreSharedKey5,
	ePreSharedKey6,
	ePreSharedKey7,
	ePreSharedKey8,
	ePreSharedKey9,
	ePreSharedKey10
};
struct CWMP_NODE tPreSharedKeyObject[] =
{
/*info,  					leaf,				next)*/
{&tPreSharedKeyObjectInfo[ePreSharedKey1],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey2],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey3],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey4],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey5],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey6],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey7],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey8],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey9],	tPreSharedKeyEntityLeaf,	NULL},
{&tPreSharedKeyObjectInfo[ePreSharedKey10],	tPreSharedKeyEntityLeaf,	NULL},
{NULL,						NULL,				NULL}
};


struct CWMP_OP tWEPKeyEntityLeafOP = { getWEPKeyEntity, setWEPKeyEntity };
struct CWMP_PRMT tWEPKeyEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WEPKey",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWEPKeyEntityLeafOP}
};
enum eWEPKeyEntityLeaf
{
	eWEPKey
};
struct CWMP_LEAF tWEPKeyEntityLeaf[] =
{
{ &tWEPKeyEntityLeafInfo[eWEPKey] },
{ NULL }
};


struct CWMP_PRMT tWEPKeyObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eWEPKeyObject
{
	eWEP1,
	eWEP2,
	eWEP3,
	eWEP4
};
struct CWMP_NODE tWEPKeyObject[] =
{
/*info,  			leaf,			next)*/
{&tWEPKeyObjectInfo[eWEP1],	tWEPKeyEntityLeaf,	NULL},
{&tWEPKeyObjectInfo[eWEP2],	tWEPKeyEntityLeaf,	NULL},
{&tWEPKeyObjectInfo[eWEP3],	tWEPKeyEntityLeaf,	NULL},
{&tWEPKeyObjectInfo[eWEP4],	tWEPKeyEntityLeaf,	NULL},
{NULL,				NULL,			NULL}
};


struct CWMP_OP tAscDeviceEntityLeafOP = { getAscDeviceEntity,NULL };
struct CWMP_PRMT tAscDeviceEntityLeafInfo[] =
{
/*(name,				type,		flag,				op)*/
{"AssociatedDeviceMACAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"AssociatedDeviceIPAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"AssociatedDeviceAuthenticationState",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"LastRequestedUnicastCipher",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"LastRequestedMulticastCipher",	eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP},
{"LastPMKId",				eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tAscDeviceEntityLeafOP}
};
enum eAscDeviceEntityLeaf
{
	eAssociatedDeviceMACAddress,
	eAssociatedDeviceIPAddress,
	eAssociatedDeviceAuthenticationState,
	eLastRequestedUnicastCipher,
	eLastRequestedMulticastCipher,
	eLastPMKId
};
struct CWMP_LEAF tAscDeviceEntityLeaf[] =
{
{ &tAscDeviceEntityLeafInfo[eAssociatedDeviceMACAddress] },
{ &tAscDeviceEntityLeafInfo[eAssociatedDeviceIPAddress] },
{ &tAscDeviceEntityLeafInfo[eAssociatedDeviceAuthenticationState] },
{ &tAscDeviceEntityLeafInfo[eLastRequestedUnicastCipher] },
{ &tAscDeviceEntityLeafInfo[eLastRequestedMulticastCipher] },
{ &tAscDeviceEntityLeafInfo[eLastPMKId] },
{ NULL }
};


struct CWMP_PRMT tAscDeviceObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eAscDeviceObject
{
	eAscDevice0
};
struct CWMP_LINKNODE tAscDeviceObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tAscDeviceObjectInfo[eAscDevice0],	tAscDeviceEntityLeaf,	NULL,		NULL,			0}
};



struct CWMP_OP tWLANConfEntityLeafOP = { getWLANConf, setWLANConf };
struct CWMP_PRMT tWLANConfEntityLeafInfo[] =
{
/*(name,				type,		flag,			op)*/
{"Enable",				eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"Status",				eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"BSSID",				eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"MaxBitRate",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"Channel",				eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"SSID",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"BeaconType",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
#ifdef MAC_FILTER
{"MACAddressControlEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
#endif /*MAC_FILTER*/
{"Standard",				eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"WEPKeyIndex",				eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"KeyPassphrase",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"WEPEncryptionLevel",			eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"BasicEncryptionModes",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"BasicAuthenticationMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"WPAEncryptionModes",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"WPAAuthenticationMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"IEEE11iEncryptionModes",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"IEEE11iAuthenticationMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"PossibleChannels",			eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
{"BasicDataTransmitRates",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"OperationalDataTransmitRates",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"PossibleDataTransmitRates",		eCWMP_tSTRING,	CWMP_READ,		&tWLANConfEntityLeafOP},
/*InsecureOOBAccessEnabled*/
{"BeaconAdvertisementEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"SSIDAdvertisementEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP}, /*version 1.4*/
{"RadioEnabled",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"AutoRateFallBackEnabled",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"LocationDescription",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
/*RequlatoryDomain*/
/*TotalPSKFailures*/
/*TotalIntegrityFailures*/
{"ChannelsInUse",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"DeviceOperationMode",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
/*DistanceFromRoot*/
/*PeerBSSID*/
{"AuthenticationServiceMode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANConfEntityLeafOP},
{"TotalBytesSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"TotalBytesReceived",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"TotalPacketsSent",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"TotalPacketsReceived",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
{"TotalAssociations",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tWLANConfEntityLeafOP},
};
enum eWLANConfEntityLeafInfo
{
	eWL_Enable,
	eWL_Status,
	eWL_BSSID,
	eWL_MaxBitRate,
	eWL_Channel,
	eWL_SSID,
	eWL_BeaconType,
#ifdef MAC_FILTER
	eWL_MACAddressControlEnabled,
#endif
	eWL_Standard,
	eWL_WEPKeyIndex,
	eWL_KeyPassphrase,
	eWL_WEPEncryptionLevel,
	eWL_BasicEncryptionModes,
	eWL_BasicAuthenticationMode,
	eWL_WPAEncryptionModes,
	eWL_WPAAuthenticationMode,
	eWL_IEEE11iEncryptionModes,
	eWL_IEEE11iAuthenticationMode,
	eWL_PossibleChannels,
	eWL_BasicDataTransmitRates,
	eWL_OperationalDataTransmitRates,
	eWL_PossibleDataTransmitRates,
	/*InsecureOOBAccessEnabled*/
	eWL_BeaconAdvertisementEnabled,
	eWL_SSIDAdvertisementEnabled,
	eWL_RadioEnabled,
	eWL_AutoRateFallBackEnabled,
	eWL_LocationDescription,
	/*RequlatoryDomain*/
	/*TotalPSKFailures*/
	/*TotalIntegrityFailures*/
	eWL_ChannelsInUse,
	eWL_DeviceOperationMode,
	/*DistanceFromRoot*/
	/*PeerBSSID*/
	eWL_AuthenticationServiceMode,
	eWL_TotalBytesSent,
	eWL_TotalBytesReceived,
	eWL_TotalPacketsSent,
	eWL_TotalPacketsReceived,
	eWL_TotalAssociations,
};
struct CWMP_LEAF tWLANConfEntityLeaf[] =
{
{ &tWLANConfEntityLeafInfo[eWL_Enable] },
{ &tWLANConfEntityLeafInfo[eWL_Status] },
{ &tWLANConfEntityLeafInfo[eWL_BSSID] },
{ &tWLANConfEntityLeafInfo[eWL_MaxBitRate] },
{ &tWLANConfEntityLeafInfo[eWL_Channel] },
{ &tWLANConfEntityLeafInfo[eWL_SSID] },
{ &tWLANConfEntityLeafInfo[eWL_BeaconType] },
#ifdef MAC_FILTER
{ &tWLANConfEntityLeafInfo[eWL_MACAddressControlEnabled] },
#endif
#ifdef _PRMT_X_CT_COM_WLAN_
{ &tCTWLANConfEntityLeafInfo[eWL_CTStandard] },
#else
{ &tWLANConfEntityLeafInfo[eWL_Standard] },
#endif
{ &tWLANConfEntityLeafInfo[eWL_WEPKeyIndex] },
{ &tWLANConfEntityLeafInfo[eWL_KeyPassphrase] },
#ifdef _PRMT_X_CT_COM_WLAN_
{ &tCTWLANConfEntityLeafInfo[eWL_CTWEPEncryptionLevel] },
#else
{ &tWLANConfEntityLeafInfo[eWL_WEPEncryptionLevel] },
#endif
{ &tWLANConfEntityLeafInfo[eWL_BasicEncryptionModes] },
{ &tWLANConfEntityLeafInfo[eWL_BasicAuthenticationMode] },
{ &tWLANConfEntityLeafInfo[eWL_WPAEncryptionModes] },
{ &tWLANConfEntityLeafInfo[eWL_WPAAuthenticationMode] },
{ &tWLANConfEntityLeafInfo[eWL_IEEE11iEncryptionModes] },
{ &tWLANConfEntityLeafInfo[eWL_IEEE11iAuthenticationMode] },
{ &tWLANConfEntityLeafInfo[eWL_PossibleChannels] },
{ &tWLANConfEntityLeafInfo[eWL_BasicDataTransmitRates] },
{ &tWLANConfEntityLeafInfo[eWL_OperationalDataTransmitRates] },
{ &tWLANConfEntityLeafInfo[eWL_PossibleDataTransmitRates] },
/*InsecureOOBAccessEnabled*/
{ &tWLANConfEntityLeafInfo[eWL_BeaconAdvertisementEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_SSIDAdvertisementEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_RadioEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_AutoRateFallBackEnabled] },
{ &tWLANConfEntityLeafInfo[eWL_LocationDescription] },
/*RequlatoryDomain*/
/*TotalPSKFailures*/
/*TotalIntegrityFailures*/
{ &tWLANConfEntityLeafInfo[eWL_ChannelsInUse] },
{ &tWLANConfEntityLeafInfo[eWL_DeviceOperationMode] },
/*DistanceFromRoot*/
/*PeerBSSID*/
{ &tWLANConfEntityLeafInfo[eWL_AuthenticationServiceMode] },
{ &tWLANConfEntityLeafInfo[eWL_TotalBytesSent] },
{ &tWLANConfEntityLeafInfo[eWL_TotalBytesReceived] },
{ &tWLANConfEntityLeafInfo[eWL_TotalPacketsSent] },
{ &tWLANConfEntityLeafInfo[eWL_TotalPacketsReceived] },
#ifdef _PRMT_X_CT_COM_WLAN_
{ &tCTWLANConfEntityLeafInfo[eWL_X_CTCOM_SSIDHide] },
{ &tCTWLANConfEntityLeafInfo[eWL_X_CTCOM_RFBand] },
{ &tCTWLANConfEntityLeafInfo[eWL_X_CTCOM_ChannelWidth] },
{ &tCTWLANConfEntityLeafInfo[eWL_X_CTCOM_GuardInterval] },
{ &tCTWLANConfEntityLeafInfo[eWL_X_CTCOM_RetryTimeout] },
{ &tCTWLANConfEntityLeafInfo[eWL_X_CTCOM_Powerlevel] },
{ &tCTWLANConfEntityLeafInfo[eWL_X_CTCOM_PowerValue] },
{ &tCTWLANConfEntityLeafInfo[eWL_X_CTCOM_APModuleEnable] },
{ &tCTWLANConfEntityLeafInfo[eWL_X_CTCOM_WPSKeyWord] },
#endif //_PRMT_X_CT_COM_WLAN_
#ifdef _PRMT_X_CT_COM_VLAN_BOUND_
{ &tCTVlanBoundingLeafInfo[eCTBOUNDING_Mode] },
{ &tCTVlanBoundingLeafInfo[eCTBOUNDING_VLAN] },
#endif
{ &tWLANConfEntityLeafInfo[eWL_TotalAssociations] },
{ NULL }
};

#ifdef CONFIG_WIFI_SIMPLE_CONFIG
struct CWMP_OP tWPSRegistrarEntityLeafOP = { getWPSRegistrarEntity, setWPSRegistrarEntity };
struct CWMP_PRMT tWPSRegistrarEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWPSRegistrarEntityLeafOP},
{"UUID",			eCWMP_tSTRING,	CWMP_READ,		&tWPSRegistrarEntityLeafOP},
{"DeviceName",			eCWMP_tSTRING,	CWMP_READ,		&tWPSRegistrarEntityLeafOP},
};
enum eWPSRegistrarEntityLeaf
{
	eWPSRG_Enable,
	eWPSRG_UUID,
	eWPSRG_DeviceName,
};
struct CWMP_LEAF tWPSRegistrarEntityLeaf[] =
{
{ &tWPSRegistrarEntityLeafInfo[eWPSRG_Enable] },
{ &tWPSRegistrarEntityLeafInfo[eWPSRG_UUID] },
{ &tWPSRegistrarEntityLeafInfo[eWPSRG_DeviceName] },
{ NULL }
};
struct CWMP_PRMT tWPSRegistrarObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"1",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
};
enum eWPSRegistrarObject
{
	eWPS1,
};
struct CWMP_NODE tWPSRegistrarObject[] =
{
/*info,  				leaf,				node)*/
{ &tWPSRegistrarObjectInfo[eWPS1],	tWPSRegistrarEntityLeaf,	NULL},	
{ NULL,					NULL,				NULL}
};

struct CWMP_OP tWLANWPSLeafOP = { getWLANWPS, setWLANWPS };
struct CWMP_PRMT tWLANWPSLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANWPSLeafOP},
{"DeviceName",			eCWMP_tSTRING,	CWMP_READ,		&tWLANWPSLeafOP},
{"DevicePassword",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tWLANWPSLeafOP},
{"UUID",			eCWMP_tSTRING,	CWMP_READ,		&tWLANWPSLeafOP},
{"Version",			eCWMP_tUINT,	CWMP_READ,		&tWLANWPSLeafOP},
{"ConfigMethodsSupported",	eCWMP_tSTRING,	CWMP_READ,		&tWLANWPSLeafOP},
{"ConfigMethodsEnabled",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tWLANWPSLeafOP},
{"SetupLockedState",		eCWMP_tSTRING,	CWMP_READ,		&tWLANWPSLeafOP},
{"SetupLock",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tWLANWPSLeafOP},
{"ConfigurationState",		eCWMP_tSTRING,	CWMP_READ,		&tWLANWPSLeafOP},
{"LastConfigurationError",	eCWMP_tSTRING,	CWMP_READ,		&tWLANWPSLeafOP},
{"RegistrarNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tWLANWPSLeafOP},
{"RegistrarEstablished",	eCWMP_tBOOLEAN,	CWMP_READ,		&tWLANWPSLeafOP},
};
enum eAWLANWPSLeaf
{
	eWPS_Enable,
	eWPS_DeviceName,
	eWPS_DevicePassword,
	eWPS_UUID,
	eWPS_Version,
	eWPS_ConfigMethodsSupported,
	eWPS_ConfigMethodsEnabled,
	eWPS_SetupLockedState,
	eWPS_SetupLock,
	eWPS_ConfigurationState,
	eWPS_LastConfigurationError,
	eWPS_RegistrarNumberOfEntries,
	eWPS_RegistrarEstablished,
};
struct CWMP_LEAF tWLANWPSLeaf[] =
{
{ &tWLANWPSLeafInfo[eWPS_Enable] },
{ &tWLANWPSLeafInfo[eWPS_DeviceName] },
{ &tWLANWPSLeafInfo[eWPS_DevicePassword] },
{ &tWLANWPSLeafInfo[eWPS_UUID] },
{ &tWLANWPSLeafInfo[eWPS_Version] },
{ &tWLANWPSLeafInfo[eWPS_ConfigMethodsSupported] },
{ &tWLANWPSLeafInfo[eWPS_ConfigMethodsEnabled] },
{ &tWLANWPSLeafInfo[eWPS_SetupLockedState] },
{ &tWLANWPSLeafInfo[eWPS_SetupLock] },
{ &tWLANWPSLeafInfo[eWPS_ConfigurationState] },
{ &tWLANWPSLeafInfo[eWPS_LastConfigurationError] },
{ &tWLANWPSLeafInfo[eWPS_RegistrarNumberOfEntries] },
{ &tWLANWPSLeafInfo[eWPS_RegistrarEstablished] },
{ NULL }
};
struct CWMP_PRMT tWLANWPSObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Registrar",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
};
enum eWLANWPSObject
{
	eWPS_Registrar,
};
struct CWMP_NODE tWLANWPSObject[] =
{
/*info,  				leaf,		node)*/
{ &tWLANWPSObjectInfo[eWPS_Registrar],	NULL,		tWPSRegistrarObject},	
{ NULL,					NULL,		NULL}
};

#endif	//CONFIG_WIFI_SIMPLE_CONFIG

struct CWMP_OP tWLAN_AssociatedDevice_OP = { NULL, objAscDevice };
struct CWMP_PRMT tWLANConfEntityObjectInfo[] =
{
/*(name,				type,		flag,			op)*/
{"AssociatedDevice",	eCWMP_tOBJECT,	CWMP_READ,		&tWLAN_AssociatedDevice_OP},
{"WEPKey",			eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"PreSharedKey",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
{"WPS", 				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif

};
enum eWLANConfEntityObject
{
	eWLAN_AssociatedDevice,
	eWLAN_WEPKey,
	eWLAN_PreSharedKey,
#ifdef CONFIG_WIFI_SIMPLE_CONFIG	
	eWLAN_WPS,
#endif
};
struct CWMP_NODE tWLANConfEntityObject[] =
{
/*info,  						leaf,	next)*/
{&tWLANConfEntityObjectInfo[eWLAN_AssociatedDevice],	NULL,	NULL},
{&tWLANConfEntityObjectInfo[eWLAN_WEPKey],		NULL,	tWEPKeyObject},
{&tWLANConfEntityObjectInfo[eWLAN_PreSharedKey],	NULL,	tPreSharedKeyObject},
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
{&tWLANConfEntityObjectInfo[eWLAN_WPS], tWLANWPSLeaf,	tWLANWPSObject},
#endif
{NULL,							NULL,	NULL}
};



struct CWMP_PRMT tWLANConfigObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"1",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef WLAN_MBSSID
{"2",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"3",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"4",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"5",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef WLAN_DUALBAND_CONCURRENT
{"6",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef WLAN_MBSSID
{"7",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"8",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"9",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"10",				eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#endif
};
enum eWLANConfigObject
{
	eWLAN1,
#ifdef WLAN_MBSSID
	eWLAN2,
	eWLAN3,
	eWLAN4,
	eWLAN5
#endif
#ifdef WLAN_DUALBAND_CONCURRENT
	,
	eWLAN6,
#ifdef WLAN_MBSSID
	eWLAN7,
	eWLAN8,
	eWLAN9,
	eWLAN10
#endif
#endif
};
struct CWMP_NODE tWLANConfigObject[] =
{
/*info,  			leaf,			node)*/
{ &tWLANConfigObjectInfo[eWLAN1],tWLANConfEntityLeaf,	tWLANConfEntityObject},
#ifdef WLAN_MBSSID
{ &tWLANConfigObjectInfo[eWLAN2],tWLANConfEntityLeaf,	tWLANConfEntityObject},
{ &tWLANConfigObjectInfo[eWLAN3],tWLANConfEntityLeaf,	tWLANConfEntityObject},
{ &tWLANConfigObjectInfo[eWLAN4],tWLANConfEntityLeaf,	tWLANConfEntityObject},
{ &tWLANConfigObjectInfo[eWLAN5],tWLANConfEntityLeaf,	tWLANConfEntityObject},
#endif
#ifdef WLAN_DUALBAND_CONCURRENT
{ &tWLANConfigObjectInfo[eWLAN6],tWLANConfEntityLeaf,	tWLANConfEntityObject},
#ifdef WLAN_MBSSID
{ &tWLANConfigObjectInfo[eWLAN7],tWLANConfEntityLeaf,	tWLANConfEntityObject},
{ &tWLANConfigObjectInfo[eWLAN8],tWLANConfEntityLeaf,	tWLANConfEntityObject},
{ &tWLANConfigObjectInfo[eWLAN9],tWLANConfEntityLeaf,	tWLANConfEntityObject},
{ &tWLANConfigObjectInfo[eWLAN10],tWLANConfEntityLeaf,	tWLANConfEntityObject},
#endif
#endif
{ NULL,				NULL,			NULL}
};

#ifdef CTCOM_WLAN_REQ
struct CWMP_PRMT tWLANObjectInfo[] =
{
/*(name,			type,		flag,					op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eWLANObject
{
	eWLAN0
};
struct CWMP_LINKNODE tWLANObject[] =
{
/*info,  				leaf,			next,				sibling,		instnum)*/
{&tWLANObjectInfo[eWLAN0],	tWLANConfEntityLeaf,	tWLANConfEntityObject,		NULL,			0}
};
#endif

int getPreSharedKeyEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	int		id=0,chain_id=0, wlaninst=0;
	CWMP_PSK_T	*pEntry=NULL, psk_entity;
	unsigned char	buf[128]="";
	//char		*tok;
#ifdef WLAN_DUALBAND_CONCURRENT
	int ori_wlan_idx = wlan_idx;
#endif

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wlaninst = getWLANConfInstNum( name );

#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>WLAN_IF_NUM )	return ERR_9007;
#else
#ifdef WLAN_DUALBAND_CONCURRENT
	if( wlaninst!=1 || wlaninst!=6) return ERR_9007;
#else
	if( wlaninst != 1 ) return ERR_9007;
#endif
#endif	//WLAN_MBSSID

	id = getPreSharedKeyInstNum( name );
	if( (id<1) || (id>10) ) return ERR_9007;

#ifdef WLAN_DUALBAND_CONCURRENT
	if(wlaninst <= 5)
		wlan_idx = 0;
	else
	{
		wlan_idx = 1;
		wlaninst -= 5;
	}
#endif

	chain_id = getPSKChainId( GETPSKINDEX(wlaninst-1,id) );
	if( chain_id >= 0 )
	{
		pEntry = &psk_entity;
		if( !mib_chain_get(CWMP_PSK_TBL, chain_id, (void*)pEntry ) )
		{
#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
#endif
			return ERR_9002;
		}
	}else
		pEntry = NULL;


	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "PreSharedKey" )==0 )
	{
#ifndef _SHOW_WLAN_KEY_WHEN_GET_
		*data = strdup( "" ); /*return an empty string*/
#else
		if( id==1 )
		{
	#ifdef WLAN_MBSSID
			if(wlaninst!= 1){
				MIB_CE_MBSSIB_T Entry;

				if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) 
				{
				#ifdef WLAN_DUALBAND_CONCURRENT
					wlan_idx = ori_wlan_idx;
				#endif
					return ERR_9002;
				}

				if(Entry.wpaPSKFormat==1) //0:Passphrase,   1:hex
				{
					*data = strdup( Entry.wpaPSK );
					#ifdef WLAN_DUALBAND_CONCURRENT
					wlan_idx = ori_wlan_idx;
					#endif
					return 0;
				}
			}else
	#endif //WLAN_MBSSID
			{
				unsigned char pskfmt;//0:Passphrase,   1:hex
				mib_get(MIB_WLAN_WPA_PSK_FORMAT, (void *)&pskfmt);
				if(pskfmt==1)
				{
					mib_get(MIB_WLAN_WPA_PSK, (void *)buf);
					*data = strdup( buf );
					#ifdef WLAN_DUALBAND_CONCURRENT
					wlan_idx = ori_wlan_idx;
					#endif
					return 0;
				}
			}
		}

		if(pEntry)
			*data = strdup( pEntry->presharedkey );
		else
			*data = strdup( "" );
#endif //_SHOW_WLAN_KEY_WHEN_GET_
	}else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
#ifndef _SHOW_WLAN_KEY_WHEN_GET_
		*data = strdup( "" ); /*return an empty string*/
#else
		if( id==1 )
		{
			MIB_CE_MBSSIB_T Entry;

			if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) 
			{
			#ifdef WLAN_DUALBAND_CONCURRENT
				wlan_idx = ori_wlan_idx;
			#endif
				return ERR_9002;
			}
			if(Entry.wpaPSKFormat==0) //0:Passphrase,   1:hex
			{
				*data = strdup( Entry.wpaPSK );
				#ifdef WLAN_DUALBAND_CONCURRENT
				wlan_idx = ori_wlan_idx;
				#endif
				return 0;
			}
		}

		if(pEntry)
			*data = strdup( pEntry->keypassphrase );
		else
			*data = strdup( "" );
#endif //_SHOW_WLAN_KEY_WHEN_GET_
	}else if( strcmp( lastname, "AssociatedDeviceMACAddress" )==0 )
	{
		*data = strdup( "" );
	}else{
	#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx = ori_wlan_idx;
		return ERR_9005;
	}
	wlan_idx = ori_wlan_idx;
	#else
		return ERR_9005;
	}
	#endif
	return 0;
}

int setPreSharedKeyEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char		*lastname = entity->info->name;
	int	id=0, chain_id=0, wlaninst=0;
	CWMP_PSK_T	*pEntry=NULL, psk_entity;
	char		*tok;
	char		*buf=data;
	#ifdef WLAN_DUALBAND_CONCURRENT
	int ori_wlan_idx = wlan_idx;
	#endif


	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>WLAN_IF_NUM )	return ERR_9007;
#else
#ifdef WLAN_DUALBAND_CONCURRENT
	if( wlaninst!=1 || wlaninst!=6) return ERR_9007;
#else
	if( wlaninst != 1 ) return ERR_9007;
#endif
#endif	//WLAN_MBSSI

	id = getPreSharedKeyInstNum( name );
	if( (id<1) || (id>10) ) return ERR_9007;
	#ifdef WLAN_DUALBAND_CONCURRENT
	if(wlaninst<=5)
		wlan_idx = 0;
	else
	{
		wlan_idx = 1;
		wlaninst -= 5;
	}
	#endif
	chain_id = getPSKChainId( GETPSKINDEX(wlaninst-1,id) );
	if( chain_id >= 0 )
	{
		pEntry = &psk_entity;
		if( !mib_chain_get(CWMP_PSK_TBL, chain_id, (void*)pEntry ) )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
		#endif
			return ERR_9002;
		}
	}else
		pEntry = NULL;

	if( strcmp( lastname, "PreSharedKey" )==0 )
	{
		int i,len;

		if( buf==NULL ) 
		{
#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
#endif
			return ERR_9007;
		}
		len = strlen(buf);
		if( len==0 || len>64 ) 
		{
#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
#endif
			return ERR_9007;
		}
#ifndef _PRMT_X_CT_COM_WLAN_	//cathy
		for( i=0; i<len; i++ )
			if( is_hex(buf[i])==0 ) 
			{
#ifdef WLAN_DUALBAND_CONCURRENT
				wlan_idx = ori_wlan_idx;
#endif
				return ERR_9007;
			}
#endif	//_PRMT_X_CT_COM_WLAN_

		//only the first needs to add/update the chain to avoid wasting flash
		//and wlan has no action for other keys
		if( id==1 )
		{
			if( pEntry )
			{
				strcpy( pEntry->presharedkey, buf );
				mib_chain_update(CWMP_PSK_TBL, (char *)pEntry, chain_id );
			}else{
				CWMP_PSK_T new_psk;
				new_psk.index=GETPSKINDEX(wlaninst-1,id);
				strcpy( new_psk.presharedkey, buf );
				strcpy( new_psk.keypassphrase, "" );
				mib_chain_add( CWMP_PSK_TBL, (unsigned char*)&new_psk );
			}
		}

		if( id==1 ) //also update MIB_WLAN_WPA_PSK
		{
			unsigned char pskfmt;
			pskfmt = 1; // 0: Passphrase, 1: Hex

			MIB_CE_MBSSIB_T Entry;

			if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) 
			{
			#ifdef WLAN_DUALBAND_CONCURRENT
				wlan_idx = ori_wlan_idx;
			#endif
				return ERR_9002;
			}

			strcpy( Entry.wpaPSK, buf );
			Entry.wpaPSKFormat = pskfmt;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);


			apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
//			return 0;
		}

//		return 0;
	}else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( (strlen(buf)<8) || (strlen(buf)>63) )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
		#endif
			return ERR_9007;
		}

		//only the first needs to add/update the chain to avoid wasting flash
		//and wlan has no action for other keys
		if( id==1 )
		{
			if( pEntry )
			{
				strcpy( pEntry->keypassphrase, buf );
				mib_chain_update(CWMP_PSK_TBL, (char *)pEntry, chain_id);
			}else{
				CWMP_PSK_T new_psk;
				new_psk.index=GETPSKINDEX(wlaninst-1,id);
				strcpy( new_psk.presharedkey, "" );
				strcpy( new_psk.keypassphrase, buf );
				mib_chain_add( CWMP_PSK_TBL, (unsigned char*)&new_psk );
			}
		}

		if( id==1 ) //also update MIB_WLAN_WPA_PSK
		{
			unsigned char pskfmt;
			pskfmt = 0; //0:Passphrase,   1:hex

			MIB_CE_MBSSIB_T Entry;

			if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) 
			{
			#ifdef WLAN_DUALBAND_CONCURRENT
				wlan_idx = ori_wlan_idx;
			#endif
				return ERR_9002;
			}
			strcpy( Entry.wpaPSK, buf );
			Entry.wpaPSKFormat = pskfmt;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

			apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
//			return 0;
		}

//		return 0;
	}else if( strcmp( lastname, "AssociatedDeviceMACAddress" )==0 )
	{
		if( buf==NULL ) 
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strlen(buf)!=0 ) 
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
		#endif
			return ERR_9001;
		}
	}
	else
	{
	#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx = ori_wlan_idx;
	#endif
		return ERR_9005;
	}

#ifdef WLAN_DUALBAND_CONCURRENT
	wlan_idx = ori_wlan_idx;
#endif
	return 0;
}

int getWEPKeyEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	int	keyid=0;
	char	*tok;
	unsigned int wlaninst=0,wepinst=0;;
	unsigned char	hex_key[32], ascii_key[32];
	unsigned char keyfmt = WEP64; 	//0:disable, 1:64, 2:128
	MIB_CE_MBSSIB_T EntryMbssid;
	#ifdef WLAN_DUALBAND_CONCURRENT
	int ori_wlan_idx = wlan_idx;
	#endif


	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>WLAN_IF_NUM )	return ERR_9007;
#else
#ifdef WLAN_DUALBAND_CONCURRENT
	if( wlaninst!=1 || wlaninst!=6) return ERR_9007;
#else
	if( wlaninst != 1 ) return ERR_9007;
#endif
#endif	//WLAN_MBSSI

	wepinst = getWEPInstNum( name );
	if( wepinst<1 || wepinst>4 )	return ERR_9007;
	#ifdef WLAN_DUALBAND_CONCURRENT
	if(wlaninst<=5)
		wlan_idx = 0;
	else
	{
		wlan_idx = 1;
		wlaninst -= 5;
	}
	#endif

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "WEPKey" )==0 )
	{
#ifndef _SHOW_WLAN_KEY_WHEN_GET_
		*data = strdup( "" ); /*return an empty string*/
#else

		if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&EntryMbssid)) 
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
		#endif
			return ERR_9002;
		}
		keyfmt = EntryMbssid.wep;

		keyid = wepinst;

		switch( keyid )
		{
			case 1:
				if(keyfmt == WEP64)
				{
						memcpy(ascii_key, EntryMbssid.wep64Key1, sizeof(EntryMbssid.wep64Key1));
				}
				else
				{
						memcpy(ascii_key, EntryMbssid.wep128Key1, sizeof(EntryMbssid.wep128Key1));
				}
				break;
			case 2:
				if(keyfmt == WEP64)
				{
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						memcpy(ascii_key, EntryMbssid.wep64Key2, sizeof(EntryMbssid.wep64Key2));
						//bzero(ascii_key, sizeof(ascii_key));
				}
				else
				{
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						memcpy(ascii_key, EntryMbssid.wep128Key2, sizeof(EntryMbssid.wep128Key2));
						//bzero(ascii_key, sizeof(ascii_key));
				}
				break;
			case 3:
				if(keyfmt == WEP64)
				{
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						memcpy(ascii_key, EntryMbssid.wep64Key3, sizeof(EntryMbssid.wep64Key3));
						//bzero(ascii_key, sizeof(ascii_key));
				}
				else
				{
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						memcpy(ascii_key, EntryMbssid.wep128Key3, sizeof(EntryMbssid.wep128Key3));
						//bzero(ascii_key, sizeof(ascii_key));
				}
				break;
			case 4:
				if(keyfmt == WEP64)
				{
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						memcpy(ascii_key, EntryMbssid.wep64Key4, sizeof(EntryMbssid.wep64Key4));
						//bzero(ascii_key, sizeof(ascii_key));
				}
				else
				{
						// Magician: No support multi-wep-keys for MBSSID, return 0.
						memcpy(ascii_key, EntryMbssid.wep128Key4, sizeof(EntryMbssid.wep128Key4));
						//bzero(ascii_key, sizeof(ascii_key));
				}
				break;
			default:
				#ifdef WLAN_DUALBAND_CONCURRENT
				wlan_idx = ori_wlan_idx;
				#endif
				return ERR_9005;
		}
		
		if( keyfmt!=WEP128 )
			sprintf( hex_key, "%02x%02x%02x%02x%02x",
				ascii_key[0], ascii_key[1], ascii_key[2], ascii_key[3], ascii_key[4] );
		else
			sprintf( hex_key, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				ascii_key[0], ascii_key[1], ascii_key[2], ascii_key[3], ascii_key[4],
				ascii_key[5], ascii_key[6], ascii_key[7], ascii_key[8], ascii_key[9],
				ascii_key[10], ascii_key[11], ascii_key[12] );

		*data = strdup( hex_key );
#endif //_SHOW_WLAN_KEY_WHEN_GET_
	}else{
	#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx = ori_wlan_idx;
		return ERR_9005;
	}
	wlan_idx = ori_wlan_idx;
	#else
		return ERR_9005;
	}
	#endif
	return 0;
}

int setWEPKeyEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int wlaninst=0,wepinst=0;;
	MIB_CE_MBSSIB_T EntryMbssid;
	#ifdef WLAN_DUALBAND_CONCURRENT
	int ori_wlan_idx = wlan_idx;
	#endif


	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>WLAN_IF_NUM )	return ERR_9007;
#else
#ifdef WLAN_DUALBAND_CONCURRENT
	if( wlaninst!=1 || wlaninst!=6) return ERR_9007;
#else
	if( wlaninst != 1 ) return ERR_9007;
#endif
#endif	//WLAN_MBSSI

	wepinst = getWEPInstNum( name );
	if( wepinst<1 || wepinst>4 )	return ERR_9007;
	#ifdef WLAN_DUALBAND_CONCURRENT
	if(wlaninst<=5)
		wlan_idx = 0;
	else
	{
		wlan_idx = 1;
		wlaninst -= 5;
	}
	#endif
	if( strcmp( lastname, "WEPKey" )==0 )
	{
		unsigned char ascii_key[32];
		int len=0, keyid;
		unsigned char keyfmt,key_type;

		if( buf==NULL ) 
		{
#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
#endif
			return ERR_9007;
		}
		len = strlen(buf);
		if( (len!=10) && (len!=26) ) 
		{
#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
#endif
			return ERR_9007;
		}
		memset( ascii_key, 0, sizeof(ascii_key) );
		if(!string_to_hex(buf, ascii_key, len)) 
		{
#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
#endif
			return ERR_9007;
		}

		keyfmt = (len==10)?WEP64:WEP128; //key format==>0:disable, 1:64, 2:128
		key_type = KEY_HEX; //key type==>KEY_ASCII:ascii, KEY_HEX:hex, tr-069 always uses the hex format.

		if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&EntryMbssid))
		{
#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx = ori_wlan_idx;
#endif
			return ERR_9002;
		}

		EntryMbssid.wep = keyfmt;
		EntryMbssid.wepKeyType = key_type;


		keyid = wepinst;
		//ascii may not be printable
		//printf("\n~~ascii_key=%s,keyfmt=%d,keyid=%d\n\n", ascii_key, keyfmt, keyid);

		switch( keyid )
		{
			case 1:
				if(keyfmt==WEP64)
				{
					memcpy(EntryMbssid.wep64Key1, ascii_key, sizeof(EntryMbssid.wep64Key1));
				}
				else
				{
					memcpy(EntryMbssid.wep128Key1, ascii_key, sizeof(EntryMbssid.wep128Key1));
				}
				break;
			case 2:
				if(keyfmt==WEP64)
				{
						// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
						memcpy( EntryMbssid.wep64Key2, ascii_key, sizeof(EntryMbssid.wep64Key2) );
					/*#ifdef WLAN_DUALBAND_CONCURRENT
						wlan_idx = ori_wlan_idx;
					#endif
						return ERR_9000; */
				}
				else
				{
						// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
						memcpy( EntryMbssid.wep128Key2, ascii_key, sizeof(EntryMbssid.wep128Key2) );
					/*#ifdef WLAN_DUALBAND_CONCURRENT
						wlan_idx = ori_wlan_idx;
					#endif
						return ERR_9000;*/
				}
				break;
			case 3:
				if(keyfmt==WEP64)
				{
						// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
						memcpy( EntryMbssid.wep64Key3, ascii_key, sizeof(EntryMbssid.wep64Key3) );
					/*#ifdef WLAN_DUALBAND_CONCURRENT
						wlan_idx = ori_wlan_idx;
					#endif
						return ERR_9000;*/
				}
				else
				{
						// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
						memcpy( EntryMbssid.wep128Key3, ascii_key, sizeof(EntryMbssid.wep128Key3) );
					/*#ifdef WLAN_DUALBAND_CONCURRENT
						wlan_idx = ori_wlan_idx;
					#endif
						return ERR_9000;*/
				}
				break;
			case 4:
				if(keyfmt==WEP64)
				{
						// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
						memcpy( EntryMbssid.wep64Key4, ascii_key, sizeof(EntryMbssid.wep64Key4) );
					/*#ifdef WLAN_DUALBAND_CONCURRENT
						wlan_idx = ori_wlan_idx;
					#endif
						return ERR_9000;*/
				}
				else
				{
						// Magician: No support multi-wep-keys for MBSSID, return ERR_9000.
						memcpy( EntryMbssid.wep128Key4, ascii_key, sizeof(EntryMbssid.wep128Key4) );
					/*#ifdef WLAN_DUALBAND_CONCURRENT
						wlan_idx = ori_wlan_idx;
					#endif
						return ERR_9000; */
				}
				break;
			default:
				#ifdef WLAN_DUALBAND_CONCURRENT
				wlan_idx = ori_wlan_idx;
				#endif
				return ERR_9005;
		}

		mib_chain_update(MIB_MBSSIB_TBL, (void *)&EntryMbssid, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx = ori_wlan_idx;
		return 0;
	}else{
		wlan_idx = ori_wlan_idx;
		return ERR_9005;
	}

	wlan_idx = ori_wlan_idx;
#else
		return 0;
	}else{
		return ERR_9005;
	}
#endif
	return 0;
}

int getAscDeviceEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	unsigned int	device_id=0;
	WLAN_STA_INFO_T info;
	char		*tok=NULL;
	unsigned int wlaninst=0;
	#ifdef WLAN_DUALBAND_CONCURRENT
	int ori_wlan_idx = wlan_idx;
	#endif

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	if( wlaninst<1 || wlaninst>WLAN_IF_NUM )	return ERR_9007;
#else
#ifdef WLAN_DUALBAND_CONCURRENT
	if( wlaninst!=1 || wlaninst!=6) return ERR_9007;
#else
	if( wlaninst != 1 ) return ERR_9007;
#endif
#endif	//WLAN_MBSSI

	#ifdef WLAN_DUALBAND_CONCURRENT
	if(wlaninst<=5)
		wlan_idx = 0;
	else
	{
		wlan_idx = 1;
		wlaninst -= 5;
	}

	if( loadWLANAssInfoByInstNum(wlaninst)<0 )
	{
		wlan_idx = ori_wlan_idx;
		return ERR_9002;
	}
	device_id = getAssDevInstNum(name);
	if( device_id<1 || device_id>gWLANTotalClients )
	{
		wlan_idx = ori_wlan_idx;
		return ERR_9005;
	}
	if( getWLANSTAINFO( device_id-1, &info )<0 )
	{
		wlan_idx = ori_wlan_idx;
		return ERR_9002;
	}
	#else
	if( loadWLANAssInfoByInstNum(wlaninst)<0 ) return ERR_9002;
	device_id = getAssDevInstNum(name);
	if( device_id<1 || device_id>gWLANTotalClients ) return ERR_9005;
	if( getWLANSTAINFO( device_id-1, &info )<0 ) return ERR_9002;
	#endif


	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "AssociatedDeviceMACAddress" )==0 )
	{
		char buf[32];
		sprintf( buf, "%02x:%02x:%02x:%02x:%02x:%02x",
				info.addr[0],info.addr[1],info.addr[2],
				info.addr[3],info.addr[4],info.addr[5] );
		*data = strdup( buf );
	}else if( strcmp( lastname, "AssociatedDeviceIPAddress" )==0 )
	{
		char aip[32]="",amac[32]="";
		sprintf( amac, "%02x:%02x:%02x:%02x:%02x:%02x",
				info.addr[0],info.addr[1],info.addr[2],
				info.addr[3],info.addr[4],info.addr[5] );
		if( getIPbyMAC( amac, aip ) < 0 )
			*data = strdup( "" );
		else
			*data = strdup( aip );
	}else if( strcmp( lastname, "AssociatedDeviceAuthenticationState" )==0 )
	{
		int i = ((info.flag & STA_INFO_FLAG_ASOC)==STA_INFO_FLAG_ASOC);
		*data = intdup( i );
	}else if( strcmp( lastname, "LastRequestedUnicastCipher" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "LastRequestedMulticastCipher" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "LastPMKId" )==0 )
	{
		*data = strdup( "" );
	}else{
	#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx = ori_wlan_idx;
		return ERR_9005;
	}
	wlan_idx = ori_wlan_idx;
	#else
		return ERR_9005;
	}
	#endif
	return 0;
}

int objAscDevice(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	unsigned int wlaninst=0;

	wlaninst = getWLANConfInstNum( name );
	if( wlaninst<1 || wlaninst>WLAN_IF_NUM ) return ERR_9007;

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		*c=NULL;
		loadWLANAssInfoByInstNum(wlaninst);
		if(gWLANTotalClients>0)
			return create_Object( c, tAscDeviceObject, sizeof(tAscDeviceObject), gWLANTotalClients, 1 );

		return 0;
		break;
	     }
	case eCWMP_tUPDATEOBJ:
	     {
	     	unsigned int num,i;
	     	struct CWMP_LINKNODE *old_table;

		loadWLANAssInfoByInstNum(wlaninst);
		num = gWLANTotalClients;
	     	old_table = (struct CWMP_LINKNODE*)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<num;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;

			remove_entity = remove_SiblingEntity( &old_table, i+1 );
			if( remove_entity!=NULL )
			{
				add_SiblingEntity( (struct CWMP_LINKNODE**)&entity->next, remove_entity );
			}else{
				unsigned int InstNum=i+1;
				add_Object( name, (struct CWMP_LINKNODE**)&entity->next,  tAscDeviceObject, sizeof(tAscDeviceObject), &InstNum );
			}
	     	}
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE*)old_table );
		return 0;
	     	break;
	     }
	}

	return -1;
}

#ifdef CTCOM_WLAN_REQ
#define WLANENTRYNUM   5
int objWLANConfiguration(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	unsigned int num=0,i,maxnum=0;

	unsigned int wlaninst=0;
	MIB_CE_MBSSIB_T Entry;
	unsigned int total;
	unsigned char mssidmap=1;

	total = mib_chain_total(MIB_MBSSIB_TBL);
//	printf("\ntotal=%d\n",total);
	for(i=1;i<total;i++){
		if (!mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry))
			continue;
//		printf("\nEntry.instnum=%d\n",Entry.instnum);
		if(Entry.instnum==(i+1))
			mssidmap|=(1<<i);
	}
//	printf("\nmssidmap=%x\n",mssidmap);
//	printf("\nname=%s\n",name);

	//CWMPDBG( 1, ( stderr, "<%s:%d>name:%s(action:%d)\n", __FUNCTION__, __LINE__, name,type ) );
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	if( (name==NULL) || (entity==NULL) ) return -1;

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		struct CWMP_LINKNODE **ptable = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		for(i=0;i<WLANENTRYNUM;i++){
			if(mssidmap&(1<<i)!=0)
				if( create_Object( ptable, tWLANObject, sizeof(tWLANObject), 1, (i+1)) < 0 )
						return -1;
		}
		return 0;
	     	break;
	     }
	case eCWMP_tADDOBJ:
	     {
	     	int ret;

	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		for(i=1;i<WLANENTRYNUM;i++){
			if((mssidmap&(1<<i))==0)
				break;
		}
//		printf("\nfree num=%d\n",i);
		if(i==WLANENTRYNUM)	return ERR_9004;

		*(unsigned int*)data=i+1;

		ret = add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWLANObject, sizeof(tWLANObject), data );
		if( ret >= 0 && i > 0)
		{
			mib_chain_get(MIB_MBSSIB_TBL, i, (void *)&Entry);
			Entry.instnum = (i+1);
			mib_chain_update( MIB_MBSSIB_TBL, (void*)&Entry, i );
#ifdef E8B_NEW_DIAGNOSE
			writeSSIDFile("Ìí¼Ó", Entry.instnum);
#endif
		}

		return ret;
	     	break;
	     }
	case eCWMP_tDELOBJ:
	     {
	     	int ret;
	     	if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

//		printf("\ndata=%d\n",*(int*)data);
		num = mib_chain_total( MIB_MBSSIB_TBL );
		for( i=1; i<num;i++ )
		{
			if( !mib_chain_get( MIB_MBSSIB_TBL, i, (void*)&Entry ))
				continue;
			if(Entry.instnum == *(int*)data)
				break;
		}//for
		if(i==num) return ERR_9005;

		ret = del_Object( name, (struct CWMP_LINKNODE **)&entity->next, *(int*)data );
		if( ret==0 )
		{
			Entry.instnum=0;
			Entry.wlanDisabled=1;
			mib_chain_update( MIB_MBSSIB_TBL, (void*)&Entry, i );
#ifdef E8B_NEW_DIAGNOSE
			writeSSIDFile("É¾³ý", *(int*)data);
#endif

			apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		}
		return ret;
	     	break;
	     }
	case eCWMP_tUPDATEOBJ:
	     {
		struct CWMP_LINKNODE *old_table;

		//CWMPDBG( 1, ( stderr, "<%s:%d>\n", __FUNCTION__, __LINE__ ) );
	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;
	     	for( i=0; i<WLANENTRYNUM;i++ )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			MIB_CE_MBSSIB_T *p,ssid_entity;

			if(i!=0){
				p = &ssid_entity;
				if( !mib_chain_get( MIB_MBSSIB_TBL, i, (void*)p ))
					continue;

				if(p->instnum!=(i+1))  continue;
	     		}

			if( (i==0)||(p->instnum==(i+1)))
			{
				unsigned int num=1;
				if(i!=0)
					num=p->instnum;
				remove_entity = remove_SiblingEntity( &old_table, num );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}else{
					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tWLANObject, sizeof(tWLANObject), &num );
				}
		     	}
		}
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );
	     	return 0;
	     }
	}
	return -1;


}
#endif

int getWLANConf(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned long bs=0,br=0,ps=0,pr=0;
	unsigned char buf[256]="";
	unsigned char vChar=0;
	unsigned int vUint=0;
	unsigned int wlaninst=0;
	#ifdef WLAN_DUALBAND_CONCURRENT
	int orig_wlan_idx=wlan_idx;
	#endif
	MIB_CE_MBSSIB_T Entry;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	wlaninst = getWLANConfInstNum( name );

	if( wlaninst<1 || wlaninst>WLAN_IF_NUM)	return ERR_9007;

#ifdef WLAN_DUALBAND_CONCURRENT
	if(wlaninst>5)
	{
		wlan_idx=1;
		wlaninst-=5;
	}
	else
		wlan_idx=0;
#endif

	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) 
	{
	#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
	#endif
		return ERR_9002;
	}
		
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		vChar = Entry.wlanDisabled;
		*data = booldup( (vChar==0) );
	}else if( strcmp( lastname, "Status" )==0 )//to test
	{
		int flags=0;
		#ifdef WLAN_DUALBAND_CONCURRENT
		if(wlan_idx==0)
		{
		#endif
			if( getInFlags(wlan_name[wlaninst-1], &flags)==1 )
			{
				if (flags & IFF_UP)
					*data = strdup( "Up" );
				else
					*data = strdup( "Disabled" );
			}else
				*data = strdup( "Error" );
		#ifdef WLAN_DUALBAND_CONCURRENT
		}
		else
		{
			if( getInFlags(wlan_name[wlaninst+4], &flags)==1 )
			{
				if (flags & IFF_UP)
					*data = strdup( "Up" );
				else
					*data = strdup( "Disabled" );
			}else
				*data = strdup( "Error" );

		}
		#endif
	}else if( strcmp( lastname, "BSSID" )==0 )
	{
		unsigned char macadd[MAC_ADDR_LEN];
		mib_get(MIB_ELAN_MAC_ADDR, (void *)macadd);
		sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", macadd[0], macadd[1],
			macadd[2], macadd[3], macadd[4], macadd[5]+wlaninst-1);
		*data=strdup(buf);
	}else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		vChar = Entry.rateAdaptiveEnabled;
		if( vChar==1 )
			*data=strdup( "Auto" );
		else
		{
			vChar = Entry.wlanBand;
			if( vChar==1 ) //2.4 GHz (B)
				*data = strdup( "11" );
			else if( vChar==2 )//2.4 GHz (G)
				*data = strdup( "54" );
			else if( vChar==3 )//2.4 GHz (B+G)
				*data = strdup( "54" );
			else if(vChar==BAND_11A)
				*data = strdup( "54" );
			else if(vChar==BAND_11N||vChar==BAND_5G_11AN)
				*data = strdup( "600" );
			else if (vChar>=BAND_5G_11AC)
				*data = strdup( "6.933G" );
			else /*0, wifi_g==4, or wifi_bg==5?????*/
				*data = strdup( "" );//return ERR_9002;
		}
	}else if( strcmp( lastname, "Channel" )==0 )
	{
#ifdef _PRMT_X_CT_COM_WLAN_
		//e8
		unsigned char auto_chan;

		mib_get( MIB_WLAN_AUTO_CHAN_ENABLED, (void *)&auto_chan);
		mib_get( MIB_WLAN_CHAN_NUM, (void *)&vChar);
		if(auto_chan)
			*data = uintdup(0);
		else
			*data = uintdup( (unsigned int)vChar );
			
#else
		mib_get( MIB_WLAN_CHAN_NUM, (void *)&vChar);
		*data = uintdup( (unsigned int)vChar );
#endif
	}else if( strcmp( lastname, "SSID" )==0 )
	{
	     /*for root ap, ssid and authType is not used => use MIB_WLAN_SSID and MIB_WLAN_AUTH_TYPE*/

		strcpy( buf, Entry.ssid );

		*data = strdup( buf );
	}else if( strcmp( lastname, "BeaconType" )==0 )
	{
#ifdef _PRMT_X_CT_COM_WLAN_	//cathy, for e8

		vChar = Entry.encrypt;

		if(vChar==WIFI_SEC_NONE)
			*data = strdup( "None" );
		else if(vChar==WIFI_SEC_WEP)
			*data = strdup( "Basic" );
#else	//_PRMT_X_CT_COM_WLAN_

		vChar = Entry.wlanDisabled;
		if(vChar==1) //disabled, so no beacon type
		{
			*data = strdup( "None" );
			#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
			#endif
			return 0;
		}


		vChar = Entry.encrypt;
		if( (vChar==WIFI_SEC_WEP) || (vChar==WIFI_SEC_NONE) )
			*data = strdup( "Basic" );
#endif//_PRMT_X_CT_COM_WLAN_
		else if( vChar==WIFI_SEC_WPA)
			*data = strdup( "WPA" );
		else if( vChar==WIFI_SEC_WPA2) /*IEEE 802.11i*/
			*data = strdup( "11i" );
		else if( vChar==WIFI_SEC_WPA2_MIXED ) /*WPA & WPA2*/
			*data = strdup( "WPAand11i" );
		else
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9002;	
		}

#ifdef MAC_FILTER
	}else if( strcmp( lastname, "MACAddressControlEnabled" )==0 )
	{
		mib_get(MIB_WLAN_MAC_CTRL, (void *)&vChar);
		*data = booldup( vChar!=0 );
#endif /*MAC_FILTER*/
	}else if( strcmp( lastname, "Standard" )==0 )
	{
		vChar = Entry.wlanBand;

		if( vChar==BAND_11B) //2.4 GHz (B)
			*data = strdup( "b" );
		else if( vChar==BAND_11G )//2.4 GHz (G)
			*data = strdup( "g-only" );
		else if( vChar==BAND_11BG)//2.4 GHz (B+G)
			*data = strdup( "g" );
		else if(vChar==BAND_11A)
			*data = strdup( "a" );
		else if(vChar==BAND_11N||vChar==BAND_5G_11AN)
			*data = strdup( "n" );
		else if(vChar>=BAND_5G_11AC)
			*data = strdup( "ac" );
		else /*0, wifi_g==4, or wifi_bg==5?????*/
			*data = strdup( "" ); //return ERR_9002;
	}else if( strcmp( lastname, "WEPKeyIndex" )==0 )
	{
#if 0 //#ifdef WLAN_MBSSID
		MIB_CE_MBSSIB_WEP_T WEPEntry;
		if(wlaninst != 1){
			if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, wlaninst-1, (void *)&WEPEntry)) return ERR_9002;
			vChar = WEPEntry.wepDefaultKey;
		}else
#endif
		mib_get( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar );

		vChar = vChar + 1;//mib's wepid is from 0 to 3
		*data = uintdup( (unsigned int)vChar );
	}else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
#ifndef _SHOW_WLAN_KEY_WHEN_GET_
		*data = strdup( "" ); /*return an empty string*/
#else
		memcpy( buf, Entry.wpaPSK, sizeof(Entry.wpaPSK) );
		*data = strdup( buf );
#endif //_SHOW_WLAN_KEY_WHEN_GET_
	}else if( strcmp( lastname, "WEPEncryptionLevel" )==0 )
	{
		*data = strdup( "Disabled,40-bit,104-bit" );
	}else if( strcmp( lastname, "BasicEncryptionModes" )==0 )
	{
		vChar = Entry.encrypt;

		if( vChar==WIFI_SEC_WEP )
			*data = strdup( "WEPEncryption" );
		else if( vChar==WIFI_SEC_NONE )
			*data = strdup( "None" );
		else
		{
			vChar = Entry.cwmp_WLAN_BasicEncry;

			if( vChar==1 )
				*data = strdup( "WEPEncryption" );
			else
				*data = strdup( "None" );
		}
	}else if( strcmp( lastname, "BasicAuthenticationMode" )==0 )
	{
#ifdef _PRMT_X_CT_COM_WLAN_	//cathy, for e8
		vChar = Entry.authType;
		if(vChar==0)
			*data = strdup( "OpenSystem" );
		else if(vChar==1)
			*data = strdup( "SharedKey" );
		else
			*data = strdup( "Both" );
#else
		vChar = Entry.enable1X;
		if(vChar)
			*data = strdup( "EAPAuthentication" );
		else
			*data = strdup( "None" );
#endif
	}else if( strcmp( lastname, "WPAEncryptionModes" )==0 )
	{
		vChar = Entry.unicastCipher;
		if( vChar==WPA_CIPHER_TKIP )
			*data = strdup( "TKIPEncryption" );
#ifdef ENABLE_WPAAES_WPA2TKIP
		else if( vChar==WPA_CIPHER_AES )
			*data = strdup( "AESEncryption" );
		else if( vChar==WPA_CIPHER_MIXED )
			*data = strdup( "TKIPandAESEncryption" );
#endif
		else
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9002;
		}
	}else if( strcmp( lastname, "WPAAuthenticationMode" )==0 )
	{
		vChar = Entry.wpaAuth;
		if(vChar==WPA_AUTH_PSK)
			*data = strdup( "PSKAuthentication" );
		else
			*data = strdup( "EAPAuthentication" );
	}else if( strcmp( lastname, "IEEE11iEncryptionModes" )==0 )
	{
		vChar = Entry.wpa2UnicastCipher;
		if( vChar==WPA_CIPHER_AES )
			*data = strdup( "AESEncryption" );
#ifdef ENABLE_WPAAES_WPA2TKIP
		else if( vChar==WPA_CIPHER_TKIP )
			*data = strdup( "TKIPEncryption" );
		else if( vChar==WPA_CIPHER_MIXED )
			*data = strdup( "TKIPandAESEncryption" );
#endif
		else
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9002;
		}
	}else if( strcmp( lastname, "IEEE11iAuthenticationMode" )==0 )
	{
		vChar = Entry.wpaAuth;
		if(vChar==WPA_AUTH_PSK)
			*data = strdup( "PSKAuthentication" );
		else
			*data = strdup( "EAPAuthentication" );
	}else if( strcmp( lastname, "PossibleChannels" )==0 )
	{
		unsigned char dm, phyband;	
		mib_get( MIB_HW_REG_DOMAIN, (void *)&dm);
		mib_get( MIB_WLAN_PHY_BAND_SELECT, (void *)&phyband);
		if(phyband == PHYBAND_2G){
		switch(dm)
		{
		case 1:	//FCC
		case 2:	//IC
		case 11:	//NCC
		case 14:	//GLOBAL
		case 15:	//WORLD-WIDE
			*data = strdup("1-11");
			break;
		case 3:	//ETSI
		case 4:	//SPAIN
		case 12: //RUSSIAN
		case 13:	//CN
			*data = strdup("1-13");
			break;
		case 5:	//FRANCE
			*data = strdup("10-13");
			break;
		case 6:	//MKK
		case 8:	//MKK1
		case 9:	//MKK2
		case 10:	//MKK3
		case 16:	//TEST
			*data = strdup("1-14");
			break;
		case 7:	//ISRAEL
			*data = strdup("3-13");
			break;
		default:
#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
#endif
			return ERR_9002;
		}
		}
		else{
			mib_get(MIB_WLAN_CHANNEL_WIDTH, (void *)&vChar);
			switch(dm)
			{
#ifdef CONFIG_RTL_DFS_SUPPORT
			case 1:
				if(vChar==0)
					*data = strdup("36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165");
				else
					*data = strdup("36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161");
				break;
			case 2:
				*data = strdup("36,40,44,48,52,56,60,64,149,153,157,161");
				break;
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				*data = strdup("36,40,44,48,52,56,60,64,100,104,108,112,116,136,140");
				break;
			case 8:
				*data = strdup("34,38,42,46");
				break;
			case 9:
				*data = strdup("36,40,44,48");
				break;
			case 10:
				*data = strdup("36,40,44,48,52,56,60,64");
				break;
			case 11:
				if(vChar==0)
					*data = strdup("56,60,64,100,104,108,112,116,136,140,149,153,157,161,165");
				else
					*data = strdup("56,60,64,100,104,108,112,116,136,140,149,153,157,161");
				break;
			case 12:
				*data = strdup("36,40,44,48");
				break;
			case 13:
				if(vChar==0)
					*data = strdup("149,153,157,161,165");
				else
					*data = strdup("149,153,157,161");
				break;
			case 14:
			case 15:
				if(vChar==0)
					*data = strdup("36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165");
				else
					*data = strdup("36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161");
				break;			
#else
			case 1:
				if(vChar==0)
					*data = strdup("36,40,44,48,149,153,157,161,165");
				else
					*data = strdup("36,40,44,48,149,153,157,161");
				break;
				
			case 2:
				*data = strdup("36,40,44,48,149,153,157,161");
				break;
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				*data = strdup("36,40,44,48");
				break;
			case 8:
				*data = strdup("34,38,42,46");
				break;
			case 9:
				*data = strdup("36,40,44,48");
				break;
			case 10:
				*data = strdup("36,40,44,48,52,56,60,64");
				break;
			case 11:
				if(vChar==0)
					*data = strdup("56,60,64,149,153,157,161,165");
				else
					*data = strdup("56,60,64,149,153,157,161");
				break;
			case 12:
				*data = strdup("36,40,44,48");
				break;
			case 13:
				if(vChar==0)
					*data = strdup("149,153,157,161,165");
				else
					*data = strdup("149,153,157,161");
				break;
			case 14:
			case 15:
				if(vChar==0)
					*data = strdup("36,40,44,48,149,153,157,161,165");
				else
					*data = strdup("36,40,44,48,149,153,157,161");
				break;
#endif
			default:
#ifdef WLAN_DUALBAND_CONCURRENT
				wlan_idx=orig_wlan_idx;
#endif
				return ERR_9002;
			}
		}
	}else if( strcmp( lastname, "BasicDataTransmitRates" )==0 )
	{
		unsigned short uShort;
		mib_get(MIB_WLAN_BASIC_RATE, (void *)&uShort);
		getRateStr( uShort, buf );
		*data = strdup( buf );
	}else if( strcmp( lastname, "OperationalDataTransmitRates" )==0 )
	{
		unsigned short uShort;
		mib_get(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort);
		getRateStr( uShort, buf );
		*data = strdup( buf );
	}else if( strcmp( lastname, "PossibleDataTransmitRates" )==0 )
	{
		unsigned short uShort;
		mib_get(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort);
		getRateStr( uShort, buf );
		*data = strdup( buf );
	}else if( strcmp( lastname, "BeaconAdvertisementEnabled" )==0 )
	{
		vChar = Entry.bcnAdvtisement;
		*data = booldup( (vChar!=0) );
	}else if( strcmp( lastname, "SSIDAdvertisementEnabled" )==0 )
	{
		vChar = Entry.hidessid;
		*data = booldup( (vChar==0) );
	}else if( strcmp( lastname, "RadioEnabled" )==0 )
	{
		vChar = Entry.wlanDisabled;
		*data = booldup( (vChar==0) );
	}else if( strcmp( lastname, "AutoRateFallBackEnabled" )==0 )
	{
		vChar = Entry.rateAdaptiveEnabled;
		*data = booldup( (vChar==1) );
	}else if( strcmp( lastname, "LocationDescription" )==0 )
	{
		*data = strdup( gLocationDescription );
	}else if( strcmp( lastname, "ChannelsInUse" )==0 )
	{
		bss_info bss;
		
		if (getWlBssInfo(getWlanIfName(), &bss) < 0)
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9002;
		}
		else
			sprintf(buf, "%d", bss.channel);
		*data = strdup( buf );
	}else if( strcmp( lastname, "DeviceOperationMode" )==0 )
	{
		vChar = Entry.wlanMode;
		if(vChar==AP_MODE)
			*data = strdup( "InfrastructureAccessPoint" );
		else if(vChar==CLIENT_MODE)
			*data = strdup( "WirelessStation" );
		else if(vChar==WDS_MODE)
			*data = strdup( "WirelessBridge" );
		else if(vChar==AP_WDS_MODE)
			*data = strdup( "WirelessBridge" );
		else /*WirelessRepeater or others*/
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9002;
		}
	}else if( strcmp( lastname, "AuthenticationServiceMode" )==0 )
	{
		*data = strdup( "None" );
	}else if( strcmp( lastname, "TotalBytesSent" )==0 )
	{
		#ifdef WLAN_DUALBAND_CONCURRENT
		if(wlan_idx==0)
		{
		#endif
			if( getInterfaceStat( wlan_name[wlaninst-1], &bs, &br, &ps, &pr ) < 0 )
		#ifndef WLAN_DUALBAND_CONCURRENT
				return -1;
		#else
			{
				wlan_idx=orig_wlan_idx;
				return -1;
			}
		}
		else
		{
			if( getInterfaceStat( wlan_name[wlaninst+4], &bs, &br, &ps, &pr ) < 0 )
			{
				wlan_idx=orig_wlan_idx;
				return -1;
			}
		}
		#endif
		*data = uintdup( bs );
	}else if( strcmp( lastname, "TotalBytesReceived" )==0 )
	{
		//todo
		#ifdef WLAN_DUALBAND_CONCURRENT
		if(wlan_idx==0)
		{
		#endif
			if( getInterfaceStat( wlan_name[wlaninst-1], &bs, &br, &ps, &pr ) < 0 )
		#ifndef WLAN_DUALBAND_CONCURRENT
				return -1;
		#else
			{
				wlan_idx=orig_wlan_idx;
				return -1;
			}
		}
		else
		{
			if( getInterfaceStat( wlan_name[wlaninst+4], &bs, &br, &ps, &pr ) < 0 )
			{
				wlan_idx=orig_wlan_idx;
				return -1;
			}
		}
		#endif
		*data = uintdup( br );
	}else if( strcmp( lastname, "TotalPacketsSent" )==0 )
	{
		#ifdef WLAN_DUALBAND_CONCURRENT
		if(wlan_idx==0)
		{
		#endif
			if( getInterfaceStat( wlan_name[wlaninst-1], &bs, &br, &ps, &pr ) < 0 )
		#ifndef WLAN_DUALBAND_CONCURRENT
				return -1;
		#else
			{
				wlan_idx=orig_wlan_idx;
				return -1;
			}
		}
		else
		{
			if( getInterfaceStat( wlan_name[wlaninst+4], &bs, &br, &ps, &pr ) < 0 )
			{
				wlan_idx=orig_wlan_idx;
				return -1;
			}
		}
		#endif
		*data = uintdup( ps );
	}else if( strcmp( lastname, "TotalPacketsReceived" )==0 )
	{
		#ifdef WLAN_DUALBAND_CONCURRENT
		if(wlan_idx==0)
		{
		#endif
			if( getInterfaceStat( wlan_name[wlaninst-1], &bs, &br, &ps, &pr ) < 0 )
		#ifndef WLAN_DUALBAND_CONCURRENT
				return -1;
		#else
			{
				wlan_idx=orig_wlan_idx;
				return -1;
			}
		}
		else
		{
			if( getInterfaceStat( wlan_name[wlaninst+4], &bs, &br, &ps, &pr ) < 0 )
			{
				wlan_idx=orig_wlan_idx;
				return -1;
			}
		}
		#endif
		*data = uintdup( pr );
	}else if( strcmp( lastname, "TotalAssociations" )==0 )
	{
		#ifdef WLAN_DUALBAND_CONCURRENT
		if(wlan_idx==0)
		{
		#endif
			if( loadWLANAssInfoByInstNum(wlaninst)< 0 )
				*data = uintdup( 0 );
			else
				*data = uintdup( gWLANTotalClients );
		#ifdef WLAN_DUALBAND_CONCURRENT
		}
		else{
			if( loadWLANAssInfoByInstNum(wlaninst+5)< 0 )
				*data = uintdup( 0 );
			else
				*data = uintdup( gWLANTotalClients );
		}
		#endif
	#ifdef WLAN_DUALBAND_CONCURRENT	
	}else{		
		wlan_idx=orig_wlan_idx;
		return ERR_9005;
	}
	wlan_idx=orig_wlan_idx;
	#else
	}else{
		return ERR_9005;
	}
	#endif
	return 0;
}

int setWLANConf(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char	vChar=0;
	unsigned int wlaninst=0;
	#ifdef WLAN_DUALBAND_CONCURRENT
	int orig_wlan_idx=wlan_idx;
	#endif
	MIB_CE_MBSSIB_T Entry;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;


	wlaninst = getWLANConfInstNum( name );
#ifdef WLAN_MBSSID
	#ifndef WLAN_DUALBAND_CONCURRENT
	if( wlaninst<1 || wlaninst>5 )	return ERR_9007;
	#else
	if( wlaninst<1 || wlaninst>WLAN_IF_NUM )	return ERR_9007;
	if(wlaninst>5){
		wlan_idx=1;
		wlaninst-=5;
	}	
	else
		wlan_idx=0;	
	#endif
#else
#ifdef WLAN_DUALBAND_CONCURRENT
	if( wlaninst!=1 && wlaninst!=6) return ERR_9007;
#else
	if( wlaninst!=1 ) return ERR_9007;
#endif
#endif
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;


#ifdef E8B_NEW_DIAGNOSE
	writeSSIDFile("ÐÞ¸Ä", wlaninst);
#endif

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		if( i==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		vChar = (*i==0)?1:0;
		
		Entry.wlanDisabled = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		if( strlen(buf)==0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strcmp(buf,"Auto")!=0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9001;
		}
		Entry.rateAdaptiveEnabled = 1;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "Channel" )==0 )
	{
		unsigned int *i = data;

		if( i==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		//check if an valid channel depending on MIB_HW_REG_DOMAIN??
		if (*i != 0)			//0:auto
		{
			unsigned char dm, phyband;
			char buf[8];
			int valid = 0;

			mib_get(MIB_HW_REG_DOMAIN, (void *)&dm);
			mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&phyband);
			if (phyband == PHYBAND_2G) {
				switch(dm)
				{
				case 1:	//FCC
				case 2:	//IC
				case 11:	//NCC
				case 14:	//GLOBAL
				case 15:	//WORLD-WIDE
					if ((*i >= 1) && (*i <= 11))
						valid = 1;
					break;
				case 3:	//ETSI
				case 4:	//SPAIN
				case 12: //RUSSIAN
				case 13:	//CN
					if ((*i >= 1) && (*i <= 13))
						valid = 1;
					break;
				case 5:	//FRANCE
					if ((*i >= 10) && (*i <= 13))
						valid = 1;
					break;
				case 6:	//MKK
				case 8:	//MKK1
				case 9:	//MKK2
				case 10:	//MKK3
				case 16:	//TEST
					if ((*i >= 1) && (*i <= 14))
						valid = 1;
					break;
				case 7:	//ISRAEL
					if ((*i >= 3) && (*i <= 13))
						valid = 1;
					break;
				default:
#ifdef WLAN_DUALBAND_CONCURRENT
					wlan_idx=orig_wlan_idx;
#endif
					return ERR_9002;
				}
			} else {
				/* the search pattern: ",ChannelNumber," */
				snprintf(buf, sizeof(buf), ",%u,", *i);

				mib_get(MIB_WLAN_CHANNEL_WIDTH, (void *)&vChar);
				switch (dm) {
#ifdef CONFIG_RTL_DFS_SUPPORT
				case 1:
					if (vChar == 0) {
						if (strstr(",36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165,", buf))
							valid = 1;
					} else {
						if (strstr(",36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,", buf))
							valid = 1;
					}
					break;
				case 2:
					if (strstr(",36,40,44,48,52,56,60,64,149,153,157,161,", buf))
						valid = 1;
					break;
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					if (strstr(",36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,", buf))
						valid = 1;
					break;
				case 8:
					if (strstr(",34,38,42,46,", buf))
						valid = 1;
					break;
				case 9:
					if (strstr(",36,40,44,48,", buf))
						valid = 1;
					break;
				case 10:
					if (strstr(",36,40,44,48,52,56,60,64,", buf))
						valid = 1;
					break;
				case 11:
					if (vChar == 0) {
						if (strstr(",56,60,64,100,104,108,112,116,136,140,149,153,157,161,165,", buf))
							valid = 1;
					} else {
						if (strstr(",56,60,64,100,104,108,112,116,136,140,149,153,157,161,", buf))
							valid = 1;
					}
					break;
				case 12:
					if (strstr(",36,40,44,48,", buf))
						valid = 1;
					break;
				case 13:
					if (vChar == 0) {
						if (strstr(",149,153,157,161,165,", buf))
							valid = 1;
					} else {
						if (strstr(",149,153,157,161,", buf))
							valid = 1;
					}
					break;
				case 14:
				case 15:
					if (vChar == 0) {
						if (strstr(",36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,165,", buf))
							valid = 1;
					} else {
						if (strstr(",36,40,44,48,52,56,60,64,100,104,108,112,116,136,140,149,153,157,161,", buf))
							valid = 1;
					}
					break;
#else
				case 1:
					if (vChar == 0) {
						if (strstr(",36,40,44,48,149,153,157,161,165,", buf))
							valid = 1;
					} else {
						if (strstr(",36,40,44,48,149,153,157,161,", buf))
							valid = 1;
					}
					break;
				case 2:
					if (strstr(",36,40,44,48,149,153,157,161,", buf))
						valid = 1;
					break;
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					if (strstr(",36,40,44,48,", buf))
						valid = 1;
					break;
				case 8:
					if (strstr(",34,38,42,46,", buf))
						valid = 1;
					break;
				case 9:
					if (strstr(",36,40,44,48,", buf))
						valid = 1;
					break;
				case 10:
					if (strstr(",36,40,44,48,52,56,60,64,", buf))
						valid = 1;
					break;
				case 11:
					if (vChar == 0) {
						if (strstr(",56,60,64,149,153,157,161,165,", buf))
							valid = 1;
					} else {
						if (strstr(",56,60,64,149,153,157,161,", buf))
							valid = 1;
					}
					break;
				case 12:
					if (strstr(",36,40,44,48,", buf))
						valid = 1;
					break;
				case 13:
					if (vChar == 0) {
						if (strstr(",149,153,157,161,165,", buf))
							valid = 1;
					} else {
						if (strstr(",149,153,157,161,", buf))
							valid = 1;
					}
					break;
				case 14:
				case 15:
					if (vChar == 0) {
						if (strstr(",36,40,44,48,149,153,157,161,165,", buf))
							valid = 1;
					} else {
						if (strstr(",36,40,44,48,149,153,157,161,", buf))
							valid = 1;
					}
					break;
#endif
				default:
					break;
				}
			}

			if (valid == 0) {
#ifdef WLAN_DUALBAND_CONCURRENT
				wlan_idx = orig_wlan_idx;
#endif
				return ERR_9007;
			}
		}
		vChar = *i;

		if(vChar == 0)
		{
			vChar = 1;
			mib_set( MIB_WLAN_AUTO_CHAN_ENABLED, (void *)&vChar);
		}
		else
			mib_set( MIB_WLAN_CHAN_NUM, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "SSID" )==0 )
	{
		//MIB_WLAN_SSID
		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( (strlen(buf)==0) || (strlen(buf)>=MAX_SSID_LEN) )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		strcpy( Entry.ssid, buf );
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "BeaconType" )==0 )
	{
		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strlen(buf)==0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		if( strcmp( buf, "None" )==0 )
		{
#ifdef _PRMT_X_CT_COM_WLAN_	//cathy
			vChar = WIFI_SEC_NONE;
			Entry.encrypt = vChar;
			Entry.authType = vChar;
			Entry.cwmp_WLAN_BasicEncry = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
#else
			vChar = 1;
			Entry.wlanDisabled = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

#endif
		}else if( strcmp( buf, "Basic" )==0 )
		{
#ifdef _PRMT_X_CT_COM_WLAN_	//cathy
			vChar = WIFI_SEC_WEP;
			Entry.encrypt = vChar;
			Entry.cwmp_WLAN_BasicEncry = 1;	//basic:wep encryption
			//mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

			//0:disable, 1:64, 2:128
			vChar = Entry.wep;

			if(vChar==0)
			{
				vChar=WEP64;
				Entry.wep = vChar;
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);


			}
			vChar = 2;	//both open and shared mode
			Entry.authType = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

#else
			vChar = Entry.encrypt;
			if (vChar == WIFI_SEC_WPA
				|| vChar == WIFI_SEC_WPA2
				|| vChar == WIFI_SEC_WPA2_MIXED) {
				unsigned char vch=0;

				vch = Entry.cwmp_WLAN_BasicEncry;


				if( vch==1 )//basic:wep encryption
				{
					//0:disable, 1:64, 2:128
					vChar = Entry.wep;


					if(vChar==0)
					{
						vChar=WEP64;
						Entry.wep = vChar;
						//mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
					}

					vChar = WIFI_SEC_WEP;
				}else //basic:none encryption
					vChar = WIFI_SEC_NONE;
					Entry.encrypt = vChar;
					mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			}
#endif
		}else if( strcmp( buf, "WPA" )==0 )
		{
#ifdef CONFIG_E8B
			unsigned char cipher_aes = WPA_CIPHER_AES;
			vChar = WIFI_SEC_WPA;
#else
			vChar = WIFI_SEC_WPA;
#endif

	#ifdef CONFIG_E8B
				Entry.unicastCipher = cipher_aes;
	#endif
				Entry.encrypt = vChar;
				Entry.authType = 0;	//open mode
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			
		}else if( strcmp( buf, "11i" )==0 )
		{
				vChar = WIFI_SEC_WPA2;
				Entry.encrypt = vChar;
				Entry.authType = 0;	//open mode
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		}else if( strcmp( buf, "WPAand11i" )==0 )
		{
				vChar = WIFI_SEC_WPA2_MIXED;
				Entry.encrypt = vChar;
				Entry.authType = 0;	//open mode
				mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		}else
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
#ifdef MAC_FILTER
	}else if( strcmp( lastname, "MACAddressControlEnabled" )==0 )
	{
		int *i = data;
		if( i==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		vChar = (*i==1)?1:0;
		mib_set( MIB_WLAN_MAC_CTRL, (void *)&vChar);
		{
			unsigned char eth_mac_ctrl=0,mac_out_dft=1;
			mib_get(MIB_ETH_MAC_CTRL, (void *)&eth_mac_ctrl);
			if( vChar==1 || eth_mac_ctrl==1 )
				mac_out_dft=0;//0:deny, 1:allow
			mib_set(MIB_MACF_OUT_ACTION, (void *)&mac_out_dft);
		}

		apply_add( CWMP_PRI_N, apply_MACFILTER, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
#endif /*MAC_FILTER*/
	}else if( strcmp( lastname, "WEPKeyIndex" )==0 )
	{
		unsigned int *i = data;

		if( i==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( (*i<1) || (*i>4) )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		vChar = (unsigned char)*i;
		vChar = vChar - 1; //mib's wepid is from 0 to 3
#if 0 //#ifdef WLAN_MBSSID
		if(wlaninst != 1){
			MIB_CE_MBSSIB_WEP_T WEPEntry;
			if (!mib_chain_get(MIB_MBSSIB_WEP_TBL, wlaninst-1, (void *)&WEPEntry)) return ERR_9002;
			WEPEntry.wepDefaultKey = vChar;
			mib_chain_update(MIB_MBSSIB_WEP_TBL, (void *)&WEPEntry, wlaninst-1);
		}else
#endif
		mib_set( MIB_WLAN_WEP_DEFAULT_KEY, (void *)&vChar );

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		unsigned char pskfmt;

		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( (strlen(buf)<8) || (strlen(buf)>63) )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		pskfmt = 0; //0:Passphrase,   1:hex
		strcpy( Entry.wpaPSK, buf );
		Entry.wpaPSKFormat = pskfmt;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		{
			int chain_id;
			CWMP_PSK_T	*pEntry=NULL, psk_entity;

			chain_id = getPSKChainId( GETPSKINDEX(wlaninst-1,1) ); //default presharedkey
			if( chain_id >= 0 )
			{
				pEntry = &psk_entity;
				if( mib_chain_get(CWMP_PSK_TBL, chain_id, (void*)pEntry ) )
				{
					strcpy( pEntry->keypassphrase, buf );
					mib_chain_update(CWMP_PSK_TBL, (char *)pEntry, chain_id);
				}
			}
		}

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}
	else if( strcmp( lastname, "BasicEncryptionModes" )==0 )
	{
		unsigned char c_mode;

		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strlen(buf)==0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		c_mode = Entry.encrypt;

		if( strcmp( buf, "WEPEncryption" )==0 )
		{
			if( c_mode==WIFI_SEC_NONE )
			{
				vChar = WIFI_SEC_WEP;
				Entry.encrypt = vChar;
				if(Entry.wep==WEP_DISABLED)
					Entry.wep=WEP64;
				//mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			}

			vChar=1;
			Entry.cwmp_WLAN_BasicEncry = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		}else if( strcmp( buf, "None" )==0 )
		{
			if( c_mode==WIFI_SEC_WEP )
			{
				vChar = WIFI_SEC_NONE;
				Entry.encrypt = vChar;
				//mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
			}

			vChar=0;
			Entry.cwmp_WLAN_BasicEncry = vChar;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);


		}else
		#ifdef WLAN_DUALBAND_CONCURRENT
		{
			wlan_idx=orig_wlan_idx;
			return ERR_9007;
		}
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		wlan_idx=orig_wlan_idx;
		#else
			return ERR_9007;

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#endif
		return 0;
	}else if( strcmp( lastname, "BasicAuthenticationMode" )==0 )
	{
		if( buf==NULL ) 
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strlen(buf)==0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

#ifdef _PRMT_X_CT_COM_WLAN_	//cathy
		if( strcmp( buf, "OpenSystem")==0 )
			vChar=0;
		else if( strcmp( buf, "SharedKey")==0 )
			vChar=1;
		else if( strcmp( buf, "Both")==0 )
			vChar=2;
		else
#else
		if( strcmp( buf, "None")==0 )
			vChar=0;
		else if( strcmp( buf, "EAPAuthentication")==0 )
			vChar=1;
		else
#endif
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

#ifndef _PRMT_X_CT_COM_WLAN_	//cathy
		Entry.enable1X = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		vChar=AUTH_BOTH;
#endif
		Entry.authType = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "WPAEncryptionModes" )==0 )
	{
		unsigned char c_mode;

		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strlen(buf)==0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		if( strcmp( buf, "TKIPEncryption" )==0 )
			vChar=WPA_CIPHER_TKIP;
#ifdef ENABLE_WPAAES_WPA2TKIP
		else if( strcmp( buf, "AESEncryption" )==0 )
			vChar=WPA_CIPHER_AES;
		else if( strcmp( buf, "TKIPandAESEncryption" )==0 )
			vChar=WPA_CIPHER_MIXED;
#endif
		else
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9001;
		}

		Entry.unicastCipher = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "WPAAuthenticationMode" )==0 )
	{
		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strlen(buf)==0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		if( strcmp( buf, "PSKAuthentication")==0 )
			vChar=WPA_AUTH_PSK;
		else if( strcmp( buf, "EAPAuthentication")==0 )
			vChar=WPA_AUTH_AUTO;
		else
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9001;
		}

		Entry.wpaAuth = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "IEEE11iEncryptionModes" )==0 )
	{
		unsigned char c_mode;

		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strlen(buf)==0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		if( strcmp( buf, "AESEncryption" )==0 )
			vChar=WPA_CIPHER_AES;
#ifdef ENABLE_WPAAES_WPA2TKIP
		else if( strcmp( buf, "TKIPEncryption" )==0 )
			vChar=WPA_CIPHER_TKIP;
		else if( strcmp( buf, "TKIPandAESEncryption" )==0 )
			vChar=WPA_CIPHER_MIXED;
#endif
		else
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9001;	
		}

		Entry.wpa2UnicastCipher = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "IEEE11iAuthenticationMode" )==0 )
	{
		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strlen(buf)==0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		if( strcmp( buf, "PSKAuthentication")==0 )
			vChar=WPA_AUTH_PSK;
		else if( strcmp( buf, "EAPAuthentication")==0 )
			vChar=WPA_AUTH_AUTO;
		else
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9001;	
		}

		Entry.wpaAuth = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "BasicDataTransmitRates" )==0 )
	{
		unsigned short uShort;

		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strlen(buf)==0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( setRateStr( buf, &uShort )<0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		
		mib_set(MIB_WLAN_BASIC_RATE, (void *)&uShort);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "OperationalDataTransmitRates" )==0 )
	{
		unsigned short uShort;
		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strlen(buf)==0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		if( setRateStr( buf, &uShort )<0 ) return ERR_9007;
		mib_set(MIB_WLAN_SUPPORTED_RATE, (void *)&uShort);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "BeaconAdvertisementEnabled" )==0 )
	{
		int *i = data;

		if( i==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		vChar = (*i==0)?0:1;
		Entry.bcnAdvtisement = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "SSIDAdvertisementEnabled" )==0 )
	{
		int *i = data;

		if( i==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		vChar = (*i==0)?1:0;
		Entry.hidessid = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "RadioEnabled" )==0 )
	{
		int *i = data;

		if( i==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		vChar = (*i==0)?1:0;
		Entry.wlanDisabled = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "AutoRateFallBackEnabled" )==0 )
	{
		int *i = data;

		if( i==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		vChar = (*i==1)?1:0;
		Entry.rateAdaptiveEnabled = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "LocationDescription" )==0 )
	{
		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		if( strlen(buf)==0 )
			strcpy( gLocationDescription, "" );
		else if( strlen(buf)<4096 )
			strcpy( gLocationDescription, buf );
		else
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
	}else if( strcmp( lastname, "DeviceOperationMode" )==0 )
	{
		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		if( strcmp(buf, "InfrastructureAccessPoint" )==0 )
			vChar=AP_MODE;
		else if( strcmp(buf, "WirelessStation" )==0 )
		{
#ifdef WLAN_CLIENT
			vChar=CLIENT_MODE;
#else
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9001;
#endif
		}
		else if( strcmp(buf, "WirelessBridge" )==0 )
			vChar=WDS_MODE;
		else
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}

		Entry.wlanMode = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
		#endif
		return 0;
	}else if( strcmp( lastname, "AuthenticationServiceMode" )==0 )
	{
		if( buf==NULL )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9007;
		}
		if( strcmp(buf, "None" )!=0 )
		{
		#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
		#endif
			return ERR_9001;
		}
	}else
	{
	#ifdef WLAN_DUALBAND_CONCURRENT
		wlan_idx=orig_wlan_idx;
	#endif
		return ERR_9005;
	}

	return 0;
}

#ifdef CONFIG_WIFI_SIMPLE_CONFIG
int getWLANWPS(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int wlaninst;
	unsigned char buf[256]={0};
#ifdef WLAN_DUALBAND_CONCURRENT
	int orig_wlan_idx=wlan_idx;
#endif
	MIB_CE_MBSSIB_T Entry;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	wlaninst = getWLANConfInstNum( name );
	//only root ap supports wps
#ifdef WLAN_MBSSID
	#ifndef WLAN_DUALBAND_CONCURRENT
		if( wlaninst!=1 )	return ERR_9007;
	#else
		if( (wlaninst!=1) && (wlaninst!=6)) return ERR_9007;
		if(wlaninst==6)
		{
			wlan_idx=1;
			wlaninst=1;
		}
		else
			wlan_idx=0;
	#endif
#else
		if( (wlaninst!=1) && (wlaninst!=6)) return ERR_9007;
#endif

	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) 
	{
#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
#endif
			return ERR_9002;
	}

	
	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		if(Entry.wsc_disabled==1)
			*data = booldup( 0 );
		else
			*data = booldup( 1 );
	}else if( strcmp( lastname, "DeviceName" )==0 )
	{
		mib_get( MIB_SNMP_SYS_NAME, (void *)buf);
		*data = strdup( buf );
		
	}else if( strcmp( lastname, "DevicePassword" )==0 )
	{
		*data = uintdup(0);
	}else if( strcmp( lastname, "UUID" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "Version" )==0 )
	{
		unsigned char wscUseVersion;
		mib_get(MIB_WSC_VERSION, (void *) &wscUseVersion);
		*data = uintdup(wscUseVersion);
	}else if( strcmp( lastname, "ConfigMethodsSupported" )==0 )
	{
		*data = strdup("PushButton");
	}else if( strcmp( lastname, "ConfigMethodsEnabled" )==0 )
	{
		*data = strdup("PushButton");
	}else if( strcmp( lastname, "SetupLockedState" )==0 )
	{
		*data = strdup( "Unlocked" );
	}else if( strcmp( lastname, "SetupLock" )==0 )
	{
		*data = booldup( 0 );
	}else if( strcmp( lastname, "ConfigurationState" )==0 )
	{
		if(Entry.wsc_configured)
			*data = strdup("Configured");
		else
			*data = strdup("Not configured");	
	}else if( strcmp( lastname, "LastConfigurationError" )==0 )
	{
		*data = strdup( "NoError" );
	}else if( strcmp( lastname, "RegistrarNumberOfEntries" )==0 )
	{
	  	*data = uintdup(1);
	}else if( strcmp( lastname, "RegistrarEstablished" )==0 )
	{
		*data = booldup(1);
	}else{
		return ERR_9005;
	}
	return 0;
}

int setWLANWPS(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*pbuf=data;
	unsigned int *pUINT=data;
	unsigned int wlaninst;
#ifdef WLAN_DUALBAND_CONCURRENT
	int orig_wlan_idx=wlan_idx;
#endif
	MIB_CE_MBSSIB_T Entry;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wlaninst = getWLANConfInstNum( name );
	//only root ap supports wps
#ifdef WLAN_MBSSID
	#ifndef WLAN_DUALBAND_CONCURRENT
		if( wlaninst!=1 )	return ERR_9007;
	#else
		if( (wlaninst!=1) && (wlaninst!=6)) return ERR_9007;
		if(wlaninst==6)
		{
			wlan_idx=1;
			wlaninst=1;
		}
		else
			wlan_idx=0;
	#endif
#else
		if( (wlaninst!=1) && (wlaninst!=6)) return ERR_9007;
#endif
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) 
	{
#ifdef WLAN_DUALBAND_CONCURRENT
			wlan_idx=orig_wlan_idx;
#endif
			return ERR_9002;
	}

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned char vChar;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==1)?0:1;

		Entry.wsc_disabled = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
		
	}else if( strcmp( lastname, "DevicePassword" )==0 )
	{
		char tmp[32]="";
		sprintf( tmp, "%u", *pUINT );
		if( strlen(tmp)!=4 && strlen(tmp)!=8  ) return ERR_9007;
		mib_set(MIB_WSC_PIN, (void *)tmp);
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "ConfigMethodsEnabled" )==0 )
	{
		return ERR_9001;
	}else if( strcmp( lastname, "SetupLock" )==0 )
	{
		return ERR_9001;
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int getWPSRegistrarEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int wlaninst;
	unsigned char buf[256]="";
	char	vChar=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	wlaninst = getWLANConfInstNum( name );
	//only root ap supports wps
#ifdef WLAN_DUALBAND_CONCURRENT	
	if( (wlaninst!=1) && (wlaninst!=6)) return ERR_9007;
#else
	if( wlaninst!=1) return ERR_9007;
#endif


	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(0);
	}else if( strcmp( lastname, "UUID" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "DeviceName" )==0 )
	{
		*data = strdup( "" );
	}else{
		return ERR_9005;
	}
	return 0;
}
int setWPSRegistrarEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int wlaninst;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data==NULL ) return ERR_9007;

	wlaninst = getWLANConfInstNum( name );
	//only root ap supports wps
#ifdef WLAN_DUALBAND_CONCURRENT	
	if( (wlaninst!=1) && (wlaninst!=6)) return ERR_9007;
#else
	if( wlaninst!=1) return ERR_9007;
#endif


	if( strcmp( lastname, "Enable" )==0 )
	{
		return ERR_9001;
	}else{
		return ERR_9005;
	}

	return 0;
}
#endif


/**************************************************************************************/
/* utility functions*/
/**************************************************************************************/
extern unsigned int getInstNum( char *name, char *objname );
unsigned int getWLANConfInstNum( char *name )
{
	return getInstNum( name, "WLANConfiguration" );
}

unsigned int getWEPInstNum( char *name )
{
	return getInstNum( name, "WEPKey" );
}

unsigned int getAssDevInstNum( char *name )
{
	return getInstNum( name, "AssociatedDevice" );
}

unsigned int getPreSharedKeyInstNum( char *name )
{
	return getInstNum( name, "PreSharedKey" );
}

int getWLANSTAINFO(int id, WLAN_STA_INFO_T *info)
{
	WLAN_STA_INFO_T* pInfo;
	int found=-1, i;
	//id starts from 0,1,2...
	if( (id<0) || (id>=gWLANTotalClients) || (info==NULL) ) return -1;

	for (i=1; i<=MAX_STA_NUM; i++)
	{
		pInfo = (WLAN_STA_INFO_T*)&gWLANAssociations[i*sizeof(WLAN_STA_INFO_T)];
		if (pInfo->aid && (pInfo->flag & STA_INFO_FLAG_ASOC))
		{
			found++;
			if(found==id) break;
		}
	}
	if( i>MAX_STA_NUM ) return -1;

	memcpy( info, pInfo, sizeof(WLAN_STA_INFO_T) );
	return 0;
}

int getPSKChainId( int index )
{
	CWMP_PSK_T	*pEntry=NULL,psk_entity;
	int total=0, i=0;

	total = mib_chain_total( CWMP_PSK_TBL );
	if( total<=0 ) return -1;

	for( i=0; i<total; i++)
	{
		pEntry = &psk_entity;
		if( mib_chain_get(CWMP_PSK_TBL, i, (void*)pEntry ) )
			if( pEntry->index==index ) break;
	}
	if( i==total ) return -1;

	return i;
}

int getRateStr( unsigned short rate, char *buf )
{
	int len;
	if( buf==NULL ) return -1;

	buf[0]=0;
	if( rate & TX_RATE_1M )
		strcat( buf, "1," );
	if( rate & TX_RATE_2M )
		strcat( buf, "2," );
	if( rate & TX_RATE_5M )
		strcat( buf, "5.5," );
	if( rate & TX_RATE_11M )
		strcat( buf, "11," );
	if( rate & TX_RATE_6M )
		strcat( buf, "6," );
	if( rate & TX_RATE_9M )
		strcat( buf, "9," );
	if( rate & TX_RATE_12M )
		strcat( buf, "12," );
	if( rate & TX_RATE_18M )
		strcat( buf, "18," );
	if( rate & TX_RATE_24M )
		strcat( buf, "24," );
	if( rate & TX_RATE_36M )
		strcat( buf, "36," );
	if( rate & TX_RATE_48M )
		strcat( buf, "48," );
	if( rate & TX_RATE_54M )
		strcat( buf, "54," );

	len = strlen(buf);
	if( len>1 )
		buf[len-1]=0;
	return 0;
}

int setRateStr( char *buf, unsigned short *rate )
{
	if( (rate!=NULL) && (buf!=NULL) )
	{
		char *tok;

		*rate=0;
		tok = strtok(buf,", \n\r");
		while(tok)
		{
			if( strcmp( tok, "1" )==0 )
				*rate = *rate | TX_RATE_1M;
			else if( strcmp( tok, "2" )==0 )
				*rate = *rate | TX_RATE_2M;
			else if( strcmp( tok, "5.5" )==0 )
				*rate = *rate | TX_RATE_5M;
			else if( strcmp( tok, "11" )==0 )
				*rate = *rate | TX_RATE_11M;
			else if( strcmp( tok, "6" )==0 )
				*rate = *rate | TX_RATE_6M;
			else if( strcmp( tok, "9" )==0 )
				*rate = *rate | TX_RATE_9M;
			else if( strcmp( tok, "12" )==0 )
				*rate = *rate | TX_RATE_12M;
			else if( strcmp( tok, "18" )==0 )
				*rate = *rate | TX_RATE_18M;
			else if( strcmp( tok, "24" )==0 )
				*rate = *rate | TX_RATE_24M;
			else if( strcmp( tok, "36" )==0 )
				*rate = *rate | TX_RATE_36M;
			else if( strcmp( tok, "48" )==0 )
				*rate = *rate | TX_RATE_48M;
			else if( strcmp( tok, "54" )==0 )
				*rate = *rate | TX_RATE_54M;
			else{
				*rate=0;
				return -1;
			}

			tok = strtok(NULL,", \n\r");
		}
		return 0;
	}

	return -1;
}


int getIPbyMAC( char *mac, char *ip )
{
	int	ret=-1;
	FILE 	*fh;
	char 	buf[128];

	if( (mac==NULL) || (ip==NULL) )	return ret;
	ip[0]=0;

	fh = fopen("/proc/net/arp", "r");
	if (!fh) return ret;

	fgets(buf, sizeof buf, fh);	/* eat line */
	//fprintf( stderr, "%s\n", buf );
	while (fgets(buf, sizeof buf, fh))
	{
		char cip[32],cmac[32];

		//fprintf( stderr, "%s\n", buf );
		//format: IP address       HW type     Flags       HW address            Mask     Device
		if( sscanf(buf,"%s %*s %*s %s %*s %*s", cip,cmac)!=2 )
			continue;

		//fprintf( stderr, "mac:%s, cmac:%s, cip:%s\n", mac, cmac, cip );
		if( strcasecmp( mac, cmac )==0 )
		{
			strcpy( ip, cip );
			ret=0;
			break;
		}
	}
	fclose(fh);
	return ret;
}
#endif /*#ifdef WLAN_SUPPORT*/

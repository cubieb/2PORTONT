#include <stdio.h>
#include <stdlib.h>

#include <parameter_api.h>

#include "../cwmpc_utility.h"
#include "prmt_wifi.h"
#include "prmt_wifi_radio.h"
#include "prmt_wifi_ssid.h"
#include "prmt_wifi_ap.h"

/****** Device.WiFi **********************************************************/
struct CWMP_OP tWiFiLeafOP = { getWiFi, NULL};

struct CWMP_PRMT tWiFiLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"RadioNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,	&tWiFiLeafOP},
{"SSIDNumberOfEntries",			eCWMP_tUINT,	CWMP_READ,	&tWiFiLeafOP},
{"AccessPointNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tWiFiLeafOP},
{"EndPointNumberOfEntries", 	eCWMP_tUINT,	CWMP_READ,	&tWiFiLeafOP},
};
enum eWiFiLeaf
{
	eRadioNumberOfEntries,
	eSSIDNumberOfEntries,
	eAccessPointNumberOfEntries,
	eEndPointNumberOfEntries,
};

struct CWMP_LEAF tWiFiLeaf[] =
{
{ &tWiFiLeafInfo[eRadioNumberOfEntries]  },
{ &tWiFiLeafInfo[eSSIDNumberOfEntries]  },
{ &tWiFiLeafInfo[eAccessPointNumberOfEntries]  },
{ &tWiFiLeafInfo[eEndPointNumberOfEntries]  },
{ NULL	}
};


struct CWMP_PRMT tWiFiObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Radio",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"SSID",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"AccessPoint",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"EndPoint",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
};

enum eWiFiObject
{
	eRadio,
	eSSID,
	eAccessPoint,
	eEndPoint,
};

struct CWMP_NODE tWiFiObject[] =
{
/*info,  				leaf,			next)*/
{&tWiFiObjectInfo[eRadio],			NULL, tWiFiRadioObject},
{&tWiFiObjectInfo[eSSID],			NULL, tWiFiSSIDObject},
{&tWiFiObjectInfo[eAccessPoint],	NULL, tWiFiAPObject},
//{&tWiFiObjectInfo[eEndPoint], 		NULL, NULL},
{NULL,					NULL,			NULL}
};

/***** Operations ************************************************************/
int getWiFi(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "RadioNumberOfEntries" )==0 )
	{
		*data = uintdup(1);
	}
	else if( strcmp( lastname, "SSIDNumberOfEntries" )==0 )
	{
		*data = uintdup(WLAN_IF_NUM);
	}
	else if( strcmp( lastname, "AccessPointNumberOfEntries" )==0 )
	{
		*data = uintdup(WLAN_IF_NUM);
	}
	else if( strcmp( lastname, "EndPointNumberOfEntries" )==0 )
	{
		*data = uintdup(0);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}




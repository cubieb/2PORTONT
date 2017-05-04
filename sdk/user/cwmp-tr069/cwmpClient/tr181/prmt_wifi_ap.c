#include <rtk/mib.h>
#include <parameter_api.h>
#include <cwmp_utility.h>

#include "../prmt_apply.h"
#include "../cwmpc_utility.h"
#include "prmt_wifi_ap.h"

/***** Device.WiFi.AccessPoint.{i}.Security **********************************/
struct CWMP_OP tWiFiAPSecurityLeafOP = { getWiFiAPSecurity, setWiFiAPSecurity};
struct CWMP_PRMT tWiFiAPSecurityLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"ModesSupported",		eCWMP_tSTRING,	CWMP_READ,	&tWiFiAPSecurityLeafOP},
{"ModeEnabled",			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiAPSecurityLeafOP},
{"WEPKey",				eCWMP_tHEXBIN,	CWMP_READ|CWMP_WRITE,	&tWiFiAPSecurityLeafOP},
{"PreSharedKey",		eCWMP_tHEXBIN,	CWMP_READ|CWMP_WRITE,	&tWiFiAPSecurityLeafOP},
{"KeyPassphrase",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiAPSecurityLeafOP},
{"RekeyingInterval",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tWiFiAPSecurityLeafOP},
{"RadiusServerIPAddr",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiAPSecurityLeafOP},
{"RadiusServerPort",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tWiFiAPSecurityLeafOP},
{"RadiusSecret",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiAPSecurityLeafOP},
};

enum eWiFiAPSecurityLeaf
{
	eWiFiAPSecurityModesSupported,
	eWiFiAPSecurityModeEnabled,
	eWiFiAPSecurityWEPKey,
	eWiFiAPSecurityPreSharedKey,
	eWiFiAPSecurityKeyPassphrase,
	eWiFiAPSecurityRekeyingInterval,
	eWiFiAPSecurityRadiusServerIPAddr,
	eWiFiAPSecurityRadiusServerPort,
	eWiFiAPSecurityRadiusSecret
};

struct CWMP_LEAF tWiFiAPSecurityLeaf[] =
{
{ &tWiFiAPSecurityLeafInfo[eWiFiAPSecurityModesSupported] },
{ &tWiFiAPSecurityLeafInfo[eWiFiAPSecurityModeEnabled] },
{ &tWiFiAPSecurityLeafInfo[eWiFiAPSecurityWEPKey] },
{ &tWiFiAPSecurityLeafInfo[eWiFiAPSecurityPreSharedKey] },
{ &tWiFiAPSecurityLeafInfo[eWiFiAPSecurityKeyPassphrase] },
{ &tWiFiAPSecurityLeafInfo[eWiFiAPSecurityRekeyingInterval] },
{ &tWiFiAPSecurityLeafInfo[eWiFiAPSecurityRadiusServerIPAddr] },
{ &tWiFiAPSecurityLeafInfo[eWiFiAPSecurityRadiusServerPort] },
{ &tWiFiAPSecurityLeafInfo[eWiFiAPSecurityRadiusSecret] },
{ NULL }
};

/***** Device.WiFi.AccessPoint.{i}.WPS ***************************************/
struct CWMP_OP tWiFiAPWPSLeafOP = { getWiFiAPWPS, setWiFiAPWPS};
struct CWMP_PRMT tWiFiAPWPSLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"Enable",					eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWiFiAPWPSLeafOP},
{"ConfigMethodsSupported",	eCWMP_tSTRING,	CWMP_READ,	&tWiFiAPWPSLeafOP},
{"ConfigMethodsEnabled",	eCWMP_tSTRING,	CWMP_READ/*|CWMP_WRITE*/,	&tWiFiAPWPSLeafOP},
};

enum eWiFiAPWPSLeaf
{
	eWiFiAPWPSEnable,
	eWiFiAPWPSConfigMethodsSupported,
	eWiFiAPWPSConfigMethodsEnabled
};

struct CWMP_LEAF tWiFiAPEntityWPSLeaf[] =
{
{ &tWiFiAPWPSLeafInfo[eWiFiAPWPSEnable] },
{ &tWiFiAPWPSLeafInfo[eWiFiAPWPSConfigMethodsSupported] },
{ &tWiFiAPWPSLeafInfo[eWiFiAPWPSConfigMethodsEnabled] },
{ NULL }
};

/***** Device.WiFi.AccessPoint.{i}.AssociatedDevice **************************/
struct CWMP_OP tWiFiAPAssocEntityLeafOP = { getWiFiAPAssocats, NULL };
struct CWMP_PRMT tWiFiAPAssocEntityLeafInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"MACAddress",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWiFiAPAssocEntityLeafOP},
	{"AuthenticationState", eCWMP_tBOOLEAN,	CWMP_READ|CWMP_DENY_ACT,	&tWiFiAPAssocEntityLeafOP},
	{"LastDataDownlinkRate",eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWiFiAPAssocEntityLeafOP},
	{"LastDataUplinkRate",  eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWiFiAPAssocEntityLeafOP},
	{"SignalStrength",      eCWMP_tINT,		CWMP_READ|CWMP_DENY_ACT,	&tWiFiAPAssocEntityLeafOP},
	{"Retransmissions",     eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,	&tWiFiAPAssocEntityLeafOP},
	{"Active",              eCWMP_tBOOLEAN,	CWMP_READ|CWMP_DENY_ACT,	&tWiFiAPAssocEntityLeafOP},
};
enum eWiFiAPAssocEntityLeaf
{
	eWiFiAPAssocMACAddress,
	eWiFiAPAssocAuthenticationState,
	eWiFiAPAssocLastDataDownlinkRate,
	eWiFiAPAssocLastDataUplinkRate,
	eWiFiAPAssocSignalStrength,
	eWiFiAPAssocRetransmissions,
	eWiFiAPAssocActive
};
struct CWMP_LEAF tWiFiAPAssocEntityLeaf[] =
{
	{ &tWiFiAPAssocEntityLeafInfo[eWiFiAPAssocMACAddress] },
	{ &tWiFiAPAssocEntityLeafInfo[eWiFiAPAssocAuthenticationState] },
	{ &tWiFiAPAssocEntityLeafInfo[eWiFiAPAssocLastDataDownlinkRate] },
	{ &tWiFiAPAssocEntityLeafInfo[eWiFiAPAssocLastDataUplinkRate] },
	{ &tWiFiAPAssocEntityLeafInfo[eWiFiAPAssocSignalStrength] },
	{ &tWiFiAPAssocEntityLeafInfo[eWiFiAPAssocRetransmissions] },
	{ &tWiFiAPAssocEntityLeafInfo[eWiFiAPAssocActive] },
	{ NULL }
};

struct CWMP_PRMT tWiFiAPAssocOjbectInfo[] =
{
	/*(name,	type,		flag,			op)*/
	{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};

enum eWiFiAPAssocOjbect
{
	eWiFiAPAssoc0
};

struct CWMP_LINKNODE tWiFiAPAssocObject[] =
{
	/*info,  				leaf,			next,		sibling,		instnum)*/
	{&tWiFiAPAssocOjbectInfo[eWiFiAPAssoc0],	tWiFiAPAssocEntityLeaf,	NULL,	NULL,	0},
};

/***** Device.WiFi.AccessPoint.{i}. ******************************************/
struct CWMP_OP tWiFiAPEntityLeafOP = { getWiFiAPEntity, setWiFiAPEntity };
struct CWMP_PRMT tWiFiAPEntityLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"Enable",							eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWiFiAPEntityLeafOP}, // default false
{"Status",							eCWMP_tSTRING,	CWMP_READ,	&tWiFiAPEntityLeafOP},			   // default Disabled
//{"Alias",							eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiAPEntityLeafOP},
{"SSIDReference",					eCWMP_tSTRING,	CWMP_READ/*|CWMP_WRITE*/,	&tWiFiAPEntityLeafOP},
{"SSIDAdvertisementEnabled",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWiFiAPEntityLeafOP},
{"RetryLimit",						eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tWiFiAPEntityLeafOP},
{"WMMCapability",					eCWMP_tBOOLEAN,	CWMP_READ,	&tWiFiAPEntityLeafOP},
{"UAPSDCapability",					eCWMP_tBOOLEAN,	CWMP_READ,	&tWiFiAPEntityLeafOP},
{"WMMEnable",						eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWiFiAPEntityLeafOP},
{"UAPSDEnable",						eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWiFiAPEntityLeafOP},
{"AssociatedDeviceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tWiFiAPEntityLeafOP},
};

enum eWiFiAPEntityLeaf
{
	eWiFiAPEnable,
	eWiFiAPStatus,
//	eWiFiAPAlias,
	eWiFiAPSSIDReference,
	eWiFiAPSSIDAdvertisementEnabled,
	eWiFiAPRetryLimit,
	eWiFiAPWMMCapability,
	eWiFiAPUAPSDCapability,
	eWiFiAPWMMEnable,
	eWiFiAPUAPSDEnable,
	eWiFiAPAssociatedDeviceNumberOfEntries
};

struct CWMP_LEAF tWiFiAPEntityLeaf[] =
{
{ &tWiFiAPEntityLeafInfo[eWiFiAPEnable] },
{ &tWiFiAPEntityLeafInfo[eWiFiAPStatus] },
//{ &tWiFiAPEntityLeafInfo[eWiFiAPAlias] },
{ &tWiFiAPEntityLeafInfo[eWiFiAPSSIDReference] },
{ &tWiFiAPEntityLeafInfo[eWiFiAPSSIDAdvertisementEnabled] },
{ &tWiFiAPEntityLeafInfo[eWiFiAPRetryLimit] },
{ &tWiFiAPEntityLeafInfo[eWiFiAPWMMCapability] },
{ &tWiFiAPEntityLeafInfo[eWiFiAPUAPSDCapability] },
{ &tWiFiAPEntityLeafInfo[eWiFiAPWMMEnable] },
{ &tWiFiAPEntityLeafInfo[eWiFiAPUAPSDEnable] },
{ &tWiFiAPEntityLeafInfo[eWiFiAPAssociatedDeviceNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tWiFiAPAssocDev_OP = { NULL, objWiFiAPAssocDev };
struct CWMP_PRMT tWiFiAPEntityObjectInfo[] =
{
/*(name,			type,		flag,			)*/
{"Security",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"WPS",				eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"AssociatedDevice",eCWMP_tOBJECT,	CWMP_READ,	&tWiFiAPAssocDev_OP},
};

enum eWiFiAPEntityObject
{
	eWiFiAPSecurity,
	eWiFiAPWPS,
	eWiFiAPAssociatedDevice
};

struct CWMP_NODE tWiFiAPEntityObjectRootAP[] =
{
/*info,  					leaf,			node)*/
{&tWiFiAPEntityObjectInfo[eWiFiAPSecurity],	tWiFiAPSecurityLeaf,	NULL},
{&tWiFiAPEntityObjectInfo[eWiFiAPWPS],		tWiFiAPEntityWPSLeaf,	NULL},
{&tWiFiAPEntityObjectInfo[eWiFiAPAssociatedDevice],	NULL,	NULL},
{NULL,						NULL,			NULL}
};

struct CWMP_NODE tWiFiAPEntityObjectVAP[] =
{
/*info,  					leaf,			node)*/
{&tWiFiAPEntityObjectInfo[eWiFiAPSecurity],	tWiFiAPSecurityLeaf,	NULL},
{&tWiFiAPEntityObjectInfo[eWiFiAPAssociatedDevice],	NULL,	NULL},
{NULL,						NULL,			NULL}
};


/***** Device.WiFi.AccessPoint. **********************************************/
struct CWMP_PRMT tWiFiAPOjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#ifdef WLAN_MBSSID
{"2",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"3",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"4",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"5",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
};
enum eWiFiAPOjbect
{
	eWiFiAP1,
#ifdef WLAN_MBSSID
	eWiFiAP2,
	eWiFiAP3,
	eWiFiAP4,
	eWiFiAP5,
#endif
};
struct CWMP_NODE tWiFiAPObject[] =
{
/*info,  				leaf,			next)*/
{&tWiFiAPOjbectInfo[eWiFiAP1],	tWiFiAPEntityLeaf,	tWiFiAPEntityObjectRootAP},
#ifdef WLAN_MBSSID
{&tWiFiAPOjbectInfo[eWiFiAP2],	tWiFiAPEntityLeaf,	tWiFiAPEntityObjectVAP},
{&tWiFiAPOjbectInfo[eWiFiAP3],	tWiFiAPEntityLeaf,	tWiFiAPEntityObjectVAP},
{&tWiFiAPOjbectInfo[eWiFiAP4],	tWiFiAPEntityLeaf,	tWiFiAPEntityObjectVAP},
{&tWiFiAPOjbectInfo[eWiFiAP5],	tWiFiAPEntityLeaf,	tWiFiAPEntityObjectVAP},
#endif
{NULL,						NULL,			NULL}
};

/***** Utility Functions *****************************************************/
//changes in following table should be synced to VHT_MCS_DATA_RATE[] in 8812_vht_gen.c
const unsigned short VHT_MCS_DATA_RATE[3][2][20] = 
	{	{	{13, 26, 39, 52, 78, 104, 117, 130, 156, 156,
			 26, 52, 78, 104, 156, 208, 234, 260, 312, 312},			// Long GI, 20MHz
			{14, 29, 43, 58, 87, 116, 130, 144, 173, 173,
			29, 58, 87, 116, 173, 231, 260, 289, 347, 347}	},		// Short GI, 20MHz
		{	{27, 54, 81, 108, 162, 216, 243, 270, 324, 360, 
			54, 108, 162, 216, 324, 432, 486, 540, 648, 720}, 		// Long GI, 40MHz
			{30, 60, 90, 120, 180, 240, 270, 300,360, 400, 
			60, 120, 180, 240, 360, 480, 540, 600, 720, 800}},		// Short GI, 40MHz
		{	{59, 117,  176, 234, 351, 468, 527, 585, 702, 780,
			117, 234, 351, 468, 702, 936, 1053, 1170, 1404, 1560}, 	// Long GI, 80MHz
			{65, 130, 195, 260, 390, 520, 585, 650, 780, 867, 
			130, 260, 390, 520, 780, 1040, 1170, 1300, 1560,1733}	}	// Short GI, 80MHz
	};

//changes in following table should be synced to MCS_DATA_RATEStr[] in 8190n_proc.c
WLAN_RATE_T rate_11n_table_20M_LONG[]={
	{MCS0, 	"6.5"},
	{MCS1, 	"13"},
	{MCS2, 	"19.5"},
	{MCS3, 	"26"},
	{MCS4, 	"39"},
	{MCS5, 	"52"},
	{MCS6, 	"58.5"},
	{MCS7, 	"65"},
	{MCS8, 	"13"},
	{MCS9, 	"26"},
	{MCS10, 	"39"},
	{MCS11, 	"52"},
	{MCS12, 	"78"},
	{MCS13, 	"104"},
	{MCS14, 	"117"},
	{MCS15, 	"130"},
	{0}
};
WLAN_RATE_T rate_11n_table_20M_SHORT[]={
	{MCS0, 	"7.2"},
	{MCS1, 	"14.4"},
	{MCS2, 	"21.7"},
	{MCS3, 	"28.9"},
	{MCS4, 	"43.3"},
	{MCS5, 	"57.8"},
	{MCS6, 	"65"},
	{MCS7, 	"72.2"},
	{MCS8, 	"14.4"},
	{MCS9, 	"28.9"},
	{MCS10, 	"43.3"},
	{MCS11, 	"57.8"},
	{MCS12, 	"86.7"},
	{MCS13, 	"115.6"},
	{MCS14, 	"130"},
	{MCS15, 	"144.5"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_LONG[]={
	{MCS0, 	"13.5"},
	{MCS1, 	"27"},
	{MCS2, 	"40.5"},
	{MCS3, 	"54"},
	{MCS4, 	"81"},
	{MCS5, 	"108"},
	{MCS6, 	"121.5"},
	{MCS7, 	"135"},
	{MCS8, 	"27"},
	{MCS9, 	"54"},
	{MCS10, 	"81"},
	{MCS11, 	"108"},
	{MCS12, 	"162"},
	{MCS13, 	"216"},
	{MCS14, 	"243"},
	{MCS15, 	"270"},
	{0}
};
WLAN_RATE_T rate_11n_table_40M_SHORT[]={
	{MCS0, 	"15"},
	{MCS1, 	"30"},
	{MCS2, 	"45"},
	{MCS3, 	"60"},
	{MCS4, 	"90"},
	{MCS5, 	"120"},
	{MCS6, 	"135"},
	{MCS7, 	"150"},
	{MCS8, 	"30"},
	{MCS9, 	"60"},
	{MCS10, 	"90"},
	{MCS11, 	"120"},
	{MCS12, 	"180"},
	{MCS13, 	"240"},
	{MCS14, 	"270"},
	{MCS15, 	"300"},
	{0}
};

void set_11ac_txrate(WLAN_STA_INFO_Tp pInfo,char* txrate)
{
	char channelWidth=0;//20M 0,40M 1,80M 2
	char shortGi=0;
	char rate_idx=pInfo->txOperaRates-0x90;
	if(!txrate)return;
/*
	TX_USE_40M_MODE		= BIT(0),
	TX_USE_SHORT_GI		= BIT(1),
	TX_USE_80M_MODE		= BIT(2)
*/
	if(pInfo->ht_info & 0x4)
		channelWidth=2;
	else if(pInfo->ht_info & 0x1)
		channelWidth=1;
	else
		channelWidth=0;
	if(pInfo->ht_info & 0x2)
		shortGi=1;

	sprintf(txrate, "%d", VHT_MCS_DATA_RATE[channelWidth][shortGi][rate_idx]>>1);
}

static int tranRate(WLAN_STA_INFO_T *pInfo, unsigned char rate)
{
	char txrate[20];
	int rateid=0;
	
	if(rate >= 0x90) {
		//sprintf(txrate, "%d", pInfo->acTxOperaRate); 
		set_11ac_txrate(pInfo, txrate);
	} else if((rate & 0x80) != 0x80){	
		if(rate%2){
			sprintf(txrate, "%d%s",rate/2, ".5"); 
		}else{
			sprintf(txrate, "%d",rate/2); 
		}
	}else{
		if((pInfo->ht_info & 0x1)==0){ //20M
			if((pInfo->ht_info & 0x2)==0){//long
				for(rateid=0; rateid<16;rateid++){
					if(rate_11n_table_20M_LONG[rateid].id == rate){
						sprintf(txrate, "%s", rate_11n_table_20M_LONG[rateid].rate);
						break;
					}
				}
			}else if((pInfo->ht_info & 0x2)==0x2){//short
				for(rateid=0; rateid<16;rateid++){
					if(rate_11n_table_20M_SHORT[rateid].id == rate){
						sprintf(txrate, "%s", rate_11n_table_20M_SHORT[rateid].rate);
						break;
					}
				}
			}
		}else if((pInfo->ht_info & 0x1)==0x1){//40M
			if((pInfo->ht_info & 0x2)==0){//long
				
				for(rateid=0; rateid<16;rateid++){
					if(rate_11n_table_40M_LONG[rateid].id == rate){
						sprintf(txrate, "%s", rate_11n_table_40M_LONG[rateid].rate);
						break;
					}
				}
			}else if((pInfo->ht_info & 0x2)==0x2){//short
				for(rateid=0; rateid<16;rateid++){
					if(rate_11n_table_40M_SHORT[rateid].id == rate){
						sprintf(txrate, "%s", rate_11n_table_40M_SHORT[rateid].rate);
						break;
					}
				}
			}
		}
		
	}

	return atof(txrate) * (double)1000;	//convert to kbps
}

static inline int set_wpa(unsigned int wlaninst, MIB_CE_MBSSIB_T *pEntry, unsigned char encrypt, unsigned char auth)
{
	pEntry->encrypt = encrypt;
	pEntry->wpaAuth= auth;
	mib_chain_update(MIB_MBSSIB_TBL, (void *)pEntry, wlaninst-1);
}

/***** Operations ************************************************************/
int getWiFiAPEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]="";
	unsigned char vChar=0;
	unsigned int wlaninst=0;
	MIB_CE_MBSSIB_T Entry;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
					
	wlaninst = getInstNum(name, "AccessPoint");

	if( wlaninst < 1 || wlaninst > WLAN_IF_NUM)	return ERR_9005;
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst - 1, (void *)&Entry)) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		vChar = Entry.wlanDisabled;
		*data = booldup( (vChar==0) );
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		vChar = Entry.wlanDisabled;

		if(vChar == 1)
			*data = strdup("Disabled");
		else
			*data = strdup("Enabled");
	}
	else if( strcmp( lastname, "SSIDReference" )==0 )
	{
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "Device.WiFi.SSID.%d", wlaninst);
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "SSIDAdvertisementEnabled" )==0 )
	{
		vChar = Entry.hidessid;

		*data = booldup( (vChar == 0) );
	}
	else if( strcmp( lastname, "RetryLimit" )==0 )
	{
		// This value is hard coded in boa setupWLan()
		*data = uintdup(6);
	}
	else if( strcmp( lastname, "WMMCapability" )==0 )
	{
#ifdef WLAN_QoS
		*data = booldup(1);
#else
		*data = booldup(0);
#endif
	}
	else if( strcmp( lastname, "UAPSDCapability" )==0 )
	{
		*data = booldup(0);
	}
	else if( strcmp( lastname, "WMMEnable" )==0 )
	{
#ifdef WLAN_QoS

		vChar = Entry.wmmEnabled;

		*data = booldup( (vChar==1) ? 1 : 0);
#else
		*data = booldup(0);
#endif
	}
	else if( strcmp( lastname, "UAPSDEnable" )==0 )
	{
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "AssociatedDeviceNumberOfEntries" )==0 )
	{
		loadWLANAssInfoByInstNum(wlaninst);
		*data = uintdup(gWLANTotalClients);
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setWiFiAPEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int wlaninst=0;
	MIB_CE_MBSSIB_T Entry;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data == NULL ) return ERR_9007;
	
	wlaninst = getInstNum(name, "AccessPoint");

	if( wlaninst < 1 || wlaninst > WLAN_IF_NUM)	return ERR_9005;
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned char vChar;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?1:0;

		Entry.wlanDisabled = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "SSIDAdvertisementEnabled" )==0 )
	{
		int *i = data;
		unsigned char vChar;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==1)?0:1;
		
		Entry.hidessid= vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "RetryLimit" )==0 )
	{
		unsigned int *i = data;
		
		if( i == NULL ) return ERR_9007;
		if( *i != 6) return ERR_9001;

		return 0;
	}
	else if( strcmp( lastname, "WMMEnable" )==0 )
	{
		int *i = data;
		unsigned char vChar;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==1)?1:0;

		Entry.wmmEnabled = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "UAPSDEnable" )==0 )
	{
		int *i = data;
		
		if( i == NULL ) return ERR_9007;
		if( *i != 0) return ERR_9001;

		return 0;
	}
	else
	{
		return ERR_9005;
	}
}
#ifdef CONFIG_WIFI_SIMPLE_CONFIG
int getWiFiAPWPS(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]="";
	unsigned int wlaninst=0;
	MIB_CE_MBSSIB_T Entry;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
					
	wlaninst = getInstNum(name, "AccessPoint");

#ifdef WLAN_MBSSID
	if( wlaninst < 1 || wlaninst > WLAN_IF_NUM) return ERR_9005;
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst - 1, (void *)&Entry)) return ERR_9002;
#else
	if( wlaninst != 1 ) return ERR_9005;
#endif

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		unsigned enable;

		mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);
		enable = Entry.wsc_disabled;

		if(enable==1)
			*data = booldup( 0 );
		else
			*data = booldup( 1 );
	}
	else if ( strcmp( lastname, "ConfigMethodsSupported" )==0 )
	{
		*data = strdup("PushButton,PIN");
	}
	else if ( strcmp( lastname, "ConfigMethodsEnabled" )==0 )
	{
		*data = strdup("PushButton,PIN");
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setWiFiAPWPS(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned int wlaninst;
	MIB_CE_MBSSIB_T Entry;

	if( (name==NULL) || (entity==NULL)) return -1;

	if( entity->info->type!=type ) return ERR_9006;

	wlaninst = getInstNum(name, "AccessPoint");

	// Only root AP have WPS function
	if( wlaninst != 1 ) return ERR_9005;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned char vChar;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==1)?1:0;

		mib_chain_get(MIB_MBSSIB_TBL, 0, &Entry);
		Entry.wsc_disabled = vChar;
		mib_chain_update(MIB_MBSSIB_TBL, &Entry, 0);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}
#endif


int getWiFiAPSecurity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]="";
	unsigned int wlaninst=0;
	MIB_CE_MBSSIB_T Entry;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;
					
	wlaninst = getInstNum(name, "AccessPoint");

	if( wlaninst < 1 || wlaninst > WLAN_IF_NUM) return ERR_9005;
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst - 1, (void *)&Entry)) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ModesSupported" )==0 )
	{
		*data = strdup("None,WEP-64,WEP-128,WPA-Personal,WPA-Enterprise,"
                       "WPA2-Personal,WPA2-Enterprise,"
                       "WPA-WPA2-Personal,WPA-WPA2-Enterprise");
	}
	else if ( strcmp( lastname, "ModeEnabled" )==0 )
	{
		unsigned char encrypt = 0;
		unsigned char wep = 0;
		unsigned char auth = 0;

		encrypt = Entry.encrypt;
		wep = Entry.wep;
		auth = Entry.wpaAuth;

		fprintf(stderr, "Getting ModeEnabled: no=%d, encrypt=%d, wep=%d, auth=%d\n", wlaninst, encrypt, wep, auth);

		if (encrypt == WIFI_SEC_NONE)
		{
			*data = strdup("None");
		}
		else if (encrypt == WIFI_SEC_WEP)
		{
			if (wep == WEP64)
				*data = strdup("WEP-64");
			else if (wep == WEP128)
				*data = strdup("WEP-128");
			else
				*data = strdup("None");
		}
		else if (encrypt == WIFI_SEC_WPA)
		{
			if (auth == WPA_AUTH_AUTO)
				*data = strdup("WPA-Enterprise");
			else if (auth == WPA_AUTH_PSK)
				*data = strdup("WPA-Personal");
		}
		else if (encrypt == WIFI_SEC_WPA2)
		{
			if (auth == WPA_AUTH_AUTO)
				*data = strdup("WPA2-Enterprise");
			else if (auth == WPA_AUTH_PSK)
				*data = strdup("WPA2-Personal");
		}
		else if (encrypt == WIFI_SEC_WPA2_MIXED)
		{
			if (auth == WPA_AUTH_AUTO)
				*data = strdup("WPA-WPA2-Enterprise");
			else if (auth == WPA_AUTH_PSK)
				*data = strdup("WPA-WPA2-Personal");
		}
		else
			*data = strdup("None");
	}
	else if ( strcmp( lastname, "WEPKey" )==0 )
	{
		struct xsd__hexBinary empty = {0};
		*data = (void *)hexBinarydup(empty);
	}
	else if ( strcmp( lastname, "PreSharedKey" )==0 )
	{
		struct xsd__hexBinary empty = {0};
		*data = (void *)hexBinarydup(empty);
	}
	else if ( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		*data = strdup("");
	}
	else if ( strcmp( lastname, "RekeyingInterval" )==0 )
	{
		unsigned int rekeying_intv = 0;

		rekeying_intv = Entry.wpaGroupRekeyTime;
		
		*data = uintdup(rekeying_intv);
	}
	else if ( strcmp( lastname, "RadiusServerIPAddr" )==0 )
	{
		char ip[INET_ADDRSTRLEN] = {0};
		struct in_addr inAddr = {0};
		
		memcpy(&inAddr, Entry.rsIpAddr, IP_ADDR_LEN);

		inet_ntop(AF_INET, (void *)&inAddr, ip, INET_ADDRSTRLEN);
		*data=strdup(ip);
	}
	else if ( strcmp( lastname, "RadiusServerPort" )==0 )
	{
		unsigned short port;
		
		port = Entry.rsPort;
		
		*data=uintdup(port);
	}
	else if ( strcmp( lastname, "RadiusSecret" )==0 )
	{
		*data=strdup("");
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int setWiFiAPSecurity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned int wlaninst=0;
	MIB_CE_MBSSIB_T Entry;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if( data == NULL ) return ERR_9007;
	
	wlaninst = getInstNum(name, "AccessPoint");

	if( wlaninst < 1 || wlaninst > WLAN_IF_NUM)	return ERR_9005;
	if (!mib_chain_get(MIB_MBSSIB_TBL, wlaninst-1, (void *)&Entry)) return ERR_9002;

	if ( strcmp( lastname, "ModeEnabled" )==0 )
	{
		unsigned char encrypt = 0;
		unsigned char wep = 0;
		unsigned char auth = 0;
		
		if (buf==NULL) return ERR_9007;

		if (strcmp(buf, "None") == 0)
		{
			encrypt = WIFI_SEC_NONE;
			Entry.encrypt = encrypt;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		}
		else if (strcmp(buf, "WEP-64") == 0) {
			encrypt = WIFI_SEC_WEP;
			wep = WEP64;
			Entry.encrypt = encrypt;
			Entry.wep = wep;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}
		else if (strcmp(buf, "WEP-128") == 0)
		{
			encrypt = WIFI_SEC_WEP;
			wep = WEP128;
			Entry.encrypt = encrypt;
			Entry.wep = wep;
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}
		else if (strcmp(buf, "WPA-Personal") == 0) {
			encrypt = WIFI_SEC_WPA;
			auth = WPA_AUTH_PSK;
			set_wpa(wlaninst, &Entry, encrypt, auth);
		}
		else if (strcmp(buf, "WPA-Enterprise") == 0) {
			encrypt = WIFI_SEC_WPA;
			auth = WPA_AUTH_AUTO;
			set_wpa(wlaninst, &Entry, encrypt, auth);
		}
		else if (strcmp(buf, "WPA2-Personal") == 0) {
			encrypt = WIFI_SEC_WPA2;
			auth = WPA_AUTH_PSK;
			set_wpa(wlaninst, &Entry, encrypt, auth);
		}
		else if (strcmp(buf, "WPA2-Enterprise") == 0) {
			encrypt = WIFI_SEC_WPA2;
			auth = WPA_AUTH_AUTO;
			set_wpa(wlaninst, &Entry, encrypt, auth);
		}
		else if (strcmp(buf, "WPA-WPA2-Personal") == 0) {
			encrypt = WIFI_SEC_WPA2_MIXED;
			auth = WPA_AUTH_PSK;
			set_wpa(wlaninst, &Entry, encrypt, auth);
		}
		else if (strcmp(buf, "WPA-WPA2-Enterprise") == 0) {
			encrypt = WIFI_SEC_WPA2_MIXED;
			auth = WPA_AUTH_AUTO;
			set_wpa(wlaninst, &Entry, encrypt, auth);
		}
		else
		{
			return ERR_9007;
		}

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if ( strcmp( lastname, "WEPKey" )==0 )
	{
		struct xsd__hexBinary *key = data;
		unsigned char wep;
		unsigned char key_type;

		if ( key == NULL ) return ERR_9007;

		key_type = KEY_HEX; //key type==>KEY_ASCII:ascii, KEY_HEX:hex, tr-069 always uses the hex format.
		wep = Entry.wep;

		if(wep == WEP64 && key->__size == 5)
		{
			Entry.wepKeyType = key_type;
			memset(Entry.wep64Key1, 0, WEP64_KEY_LEN + 1);
			memcpy(Entry.wep64Key1, key->__ptr, WEP64_KEY_LEN);
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}
		else if(wep == WEP128 && key->__size == 13)
		{
			Entry.wepKeyType = key_type;
			memset(Entry.wep128Key1, 0, WEP128_KEY_LEN + 1);
			memcpy(Entry.wep128Key1, key->__ptr, 13);
			mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		}
		else
			return ERR_9007;
		
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if ( strcmp( lastname, "PreSharedKey" )==0 )
	{
		struct xsd__hexBinary *key = data;
		unsigned char pskfmt = 1; // 0: Passphrase, 1: Hex
		
		if (key == NULL || key->__size != 32) return ERR_9007;

		//We still save as string
		unsigned char wpa_key[MAX_PSK_LEN + 1] = {0};
		unsigned char tmp[3] = {0};
		int i;

		for (i = 0; i < key->__size; i++)
		{
			sprintf(tmp, "%02x", key->__ptr[i]);
			strcat(wpa_key, tmp);
		}

		Entry.wpaPSKFormat = pskfmt;
		strcpy(Entry.wpaPSK, wpa_key);
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if ( strcmp( lastname, "KeyPassphrase" )==0 )
	{
		unsigned char pskfmt;
		
		if( buf==NULL ) return ERR_9007;
		if( (strlen(buf)<8) || (strlen(buf)>63) ) return ERR_9007;

		pskfmt = 0; //0:Passphrase,   1:hex

		strncpy(Entry.wpaPSK, buf, MIB_WLAN_WPA_PSK + 1);
		Entry.wpaPSKFormat = pskfmt;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if ( strcmp( lastname, "RekeyingInterval" )==0 )
	{
		unsigned int *rekeying_intv = data;

		if( rekeying_intv == NULL ) return ERR_9007;

		Entry.wpaGroupRekeyTime = *rekeying_intv;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if ( strcmp( lastname, "RadiusServerIPAddr" )==0 )
	{
		struct in_addr inIp;
		
		if (buf == NULL) return ERR_9007;

		inet_aton(buf, &inIp);
		
		memcpy(Entry.rsIpAddr, &inIp, IP_ADDR_LEN);
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if ( strcmp( lastname, "RadiusServerPort" )==0 )
	{
		unsigned int *val = data;
		unsigned short radiusPort;

		if( val == NULL ) return ERR_9007;

		radiusPort = *val;

		Entry.rsPort = radiusPort;
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if ( strcmp( lastname, "RadiusSecret" )==0 )
	{
		if( buf == NULL ) return ERR_9007;

		strncpy(Entry.rsPassword, buf, MAX_PSK_LEN + 1);
		mib_chain_update(MIB_MBSSIB_TBL, (void *)&Entry, wlaninst-1);
		
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}


int objWiFiAPAssocDev(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);fflush(NULL);
	unsigned int wlaninst=0;
		
	wlaninst = getInstNum(name, "AccessPoint");

	if( wlaninst<1 || wlaninst > WLAN_IF_NUM) return ERR_9007;

	switch( type )
	{
		case eCWMP_tINITOBJ:
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
				}
				else
				{ 
					unsigned int InstNum=i+1;
					add_Object( name, (struct CWMP_LINKNODE**)&entity->next,  tWiFiAPAssocObject, sizeof(tWiFiAPAssocObject), &InstNum );
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

int getWiFiAPAssocats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	unsigned int	device_id=0;
	WLAN_STA_INFO_T info;
	unsigned int wlaninst=0;

	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	wlaninst = getInstNum(name, "AccessPoint");

	if( wlaninst<1 || wlaninst>10 )	return ERR_9007;

	if( loadWLANAssInfoByInstNum(wlaninst)<0 ) return ERR_9002;
	
	device_id = getInstNum( name, "AssociatedDevice" );

	if( device_id<1 || device_id>gWLANTotalClients ) return ERR_9005;
	
	if( getWLANSTAINFO( device_id-1, &info )<0 ) return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "MACAddress" )==0 )
	{
		char buf[32];
		sprintf( buf, "%02x:%02x:%02x:%02x:%02x:%02x",
				info.addr[0],info.addr[1],info.addr[2],
				info.addr[3],info.addr[4],info.addr[5] );
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "AuthenticationState" )==0 )
	{
		int i = ((info.flag & STA_INFO_FLAG_ASOC)==STA_INFO_FLAG_ASOC);
		*data = intdup( i );
	}
	else if( strcmp( lastname, "LastDataDownlinkRate" )==0 )
	{
		*data = uintdup(tranRate(&info, info.txOperaRates));
	}
	else if( strcmp( lastname, "LastDataUplinkRate" )==0 )
	{
		*data = uintdup(tranRate(&info, info.txOperaRates));
	}
	else if( strcmp( lastname, "SignalStrength" )==0 )
	{
		*data = intdup(info.rssi - 100);
	}
	else if( strcmp( lastname, "Retransmissions" )==0 )
	{
		//We don't support this parameter
		*data = uintdup(0);
	}
	else if( strcmp( lastname, "Active" )==0 )
	{
		// We only list active nodes
		*data = booldup(1);
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}


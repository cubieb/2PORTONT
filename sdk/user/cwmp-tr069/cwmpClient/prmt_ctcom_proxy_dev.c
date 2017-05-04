#include <parameter_api.h>

#include "ctcom_upnpdm_proxy.h"
#include "prmt_ctcom_proxy_dev.h"
#include "prmt_ctcom_proxy_dev_list.h"

#ifndef WLAN_SUPPORT
//Define some wifi enumuration if Wi-Fi is not supported.
typedef enum {
	WIFI_SEC_NONE = 0,
	WIFI_SEC_WEP = 1,
	WIFI_SEC_WPA = 2,
	WIFI_SEC_WPA2 = 4,
	WIFI_SEC_WPA2_MIXED = 6,
	WIFI_SEC_WAPI = 8
} WIFI_SECURITY_T;

typedef enum { WPA_CIPHER_TKIP=1, WPA_CIPHER_AES=2, WPA_CIPHER_MIXED=3 } WPA_CIPHER_T;   // Mason Yu. 201009_new_security
typedef enum { BAND_11B=1, BAND_11G=2, BAND_11BG=3, BAND_11A=4, BAND_11N=8, BAND_5G_11AN=12,
	BAND_5G_11AC=64,BAND_5G_11AAC=68,BAND_5G_11NAC=72,BAND_5G_11ANAC=76} BAND_TYPE_T;
#endif


/***** IGD.X_CT-COM_ProxyDevice.ServiceProfile.ConfigTemplate.X_CT-COM_AP_XXXX *****/
struct CWMP_OP tCT_Template_APEntityLeafOP = { getCT_Template_APEntity, setCT_Template_APEntity };
struct CWMP_PRMT tCT_Template_APEntityLeafInfo[] =
{
/*(name,				type,		flag,			op)*/
{"X_CT-COM_SSIDHide", 		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ, 	&tCT_Template_APEntityLeafOP},
{"X_CT-COM_RFBand", 		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"VLAN", 			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"X_CT-COM_ChannelWidth",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"X_CT-COM_GuardInterval",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"X_CT-COM_RetryTimeout",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"X_CT-COM_Powerlevel", 	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"X_CT-COM_APModuleEnable", eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"X_CT-COM_WPSKeyWord", 	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"Enable",				eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"Channel", 			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"BeaconType",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"Standard",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tCT_Template_APEntityLeafOP},
{"WEPEncryptionLevel",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tCT_Template_APEntityLeafOP},
{"BasicAuthenticationMode", 	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"BasicEncryptionModes",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"WPAEncryptionModes",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"SSID",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"WEPKeyIndex", 			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"MaxBitRate",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
{"MACAddressControlEnabled",		eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tCT_Template_APEntityLeafOP},
};

enum eCT_Template_APEntityLeafInfo
{
	eCT_Template_AP_X_CTCOM_SSIDHide,
	eCT_Template_AP_X_CTCOM_RFBand,
	eCT_Template_AP_VLAN,
	eCT_Template_AP_X_CTCOM_ChannelWidth,
	eCT_Template_AP_X_CTCOM_GuardInterval,
	eCT_Template_AP_X_CTCOM_RetryTimeout,
	eCT_Template_AP_X_CTCOM_Powerlevel,
	eCT_Template_AP_X_CTCOM_APModuleEnable,
	eCT_Template_AP_X_CTCOM_WPSKeyWord,
	eCT_Template_AP_Enable,
	eCT_Template_AP_Channel,
	eCT_Template_AP_BeaconType,
	eCT_Template_AP_Standard,
	eCT_Template_AP_WEPEncryptionLevel,
	eCT_Template_AP_BasicAuthenticationMode,
	eCT_Template_AP_BasicEncryptionModes,
	eCT_Template_AP_WPAEncryptionModes,
	eCT_Template_AP_SSID,
	eCT_Template_AP_WEPKeyIndex,
	eCT_Template_AP_MaxBitRate,
	eCT_Template_AP_MACAddressControlEnabled,
};
struct CWMP_LEAF tCT_Template_APEntityLeaf[] =
{
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_X_CTCOM_SSIDHide] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_X_CTCOM_RFBand] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_VLAN] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_X_CTCOM_ChannelWidth] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_X_CTCOM_GuardInterval] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_X_CTCOM_RetryTimeout] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_X_CTCOM_Powerlevel] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_X_CTCOM_APModuleEnable] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_X_CTCOM_WPSKeyWord] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_Enable] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_Channel] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_BeaconType] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_Standard] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_WEPEncryptionLevel] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_BasicAuthenticationMode] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_BasicEncryptionModes] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_WPAEncryptionModes] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_SSID] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_WEPKeyIndex] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_MaxBitRate] },
{ &tCT_Template_APEntityLeafInfo[eCT_Template_AP_MACAddressControlEnabled] },
{ NULL }
};


struct CWMP_PRMT tCT_Template_APObjectInfo[] =
{
/*(name,		type,		flag,					op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
struct CWMP_LINKNODE tCT_Template_APObject[] =
{
/*info,  					leaf,			next,		sibling,		instnum)*/
{&tCT_Template_APObjectInfo[0], 	tCT_Template_APEntityLeaf,	NULL,		NULL,			0}
};

int get_cfg_template_ap(int mib_id, int inst_num, MIB_CE_UPNPDM_CFG_TEMP_AP_T *pEntry, int *chainidx)
{
	int total = 0;
	int i;

	if(pEntry == NULL || chainidx == NULL)
		return -1;

	if(mib_id < UPNPDM_CFG_TEMP_AP_INTERNET_TBL || mib_id > UPNPDM_CFG_TEMP_AP_VOIP_TBL)
		return -1;

	total = mib_chain_total(mib_id);
	*chainidx = -1;

	for(i = 0 ; i < total ; i++)
	{
		mib_chain_get(mib_id, i, pEntry);
		
		if(pEntry->inst_num == inst_num)
		{
			*chainidx = i;
			return 0;
		}
	}

	return -1;
}


int objCT_Template_AP(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, num, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	int mib_id = 0;

	if (name == NULL || entity == NULL)
		return -1;

	if(strstr(name, "X_CT-COM_AP_Internet"))
		mib_id = UPNPDM_CFG_TEMP_AP_INTERNET_TBL;
	else if(strstr(name, "X_CT-COM_AP_IPTV"))
		mib_id = UPNPDM_CFG_TEMP_AP_IPTV_TBL;
	else if(strstr(name, "X_CT-COM_AP_VoIP"))
		mib_id = UPNPDM_CFG_TEMP_AP_VOIP_TBL;
	else 
		return ERR_9005;
	
	num = mib_chain_total(mib_id);

	switch (type) {
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		{
			if (data == NULL)
				return -1;

			ret = add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_Template_APObject,
				       sizeof(tCT_Template_APObject), data);
			if (ret >= 0)
			{
				struct CWMP_LINKNODE *tmp_entity = NULL;
				MIB_CE_UPNPDM_CFG_TEMP_AP_T entry = {0};

				entry.WEPKeyIndex = 1;
				entry.WPSKeyWord = 128;
				entry.Powerlevel = 1;
				entry.VLAN = -1;
				entry.Standard = BAND_11BG;
				strcpy(entry.MaxBitRate, "Auto");
				entry.inst_num = *(int *)data;
				mib_chain_add(mib_id, &entry);
			}
			break;
		}
	case eCWMP_tDELOBJ:
		{
			unsigned int *pUint = data;
			MIB_CE_UPNPDM_CFG_TEMP_AP_T entry = {0};

			ret = ERR_9005;

			if (data == NULL)
				return -1;

			for (i = 0; i < num; i++)
			{
				if (!mib_chain_get(mib_id, i, &entry))
					continue;

				if (entry.inst_num == *pUint)
				{
					mib_chain_delete(mib_id, i);
					ret = del_Object(name, (struct CWMP_LINKNODE **) &entity->next, *(int *)data);
					break;;
				}
			}
			break;
		}
	case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;
			MIB_CE_UPNPDM_CFG_TEMP_AP_T entry = {0};

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < num; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL;

				if (!mib_chain_get(mib_id, i, &entry))
					continue;

				remove_entity = remove_SiblingEntity(&old_table, entry.inst_num);

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					add_Object(name,
						   (struct CWMP_LINKNODE **) &entity->next,
						   tCT_Template_APObject,
						   sizeof(tCT_Template_APObject),
						   &entry.inst_num);
				}
			}

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)old_table);

			break;
		}
	}

	return ret;
}

int getCT_Template_APEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int vUint=0;
	int mib_id = 0;
	int inst_num = 0, chainidx = -1;
	MIB_CE_UPNPDM_CFG_TEMP_AP_T entry = {0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if(strstr(name, "X_CT-COM_AP_Internet"))
	{
		mib_id = UPNPDM_CFG_TEMP_AP_INTERNET_TBL;
		inst_num = getInstNum(name, "X_CT-COM_AP_Internet");
	}
	else if(strstr(name, "X_CT-COM_AP_IPTV"))
	{
		mib_id = UPNPDM_CFG_TEMP_AP_IPTV_TBL;
		inst_num = getInstNum(name, "X_CT-COM_AP_IPTV");
	}
	else if(strstr(name, "X_CT-COM_AP_VoIP"))
	{
		mib_id = UPNPDM_CFG_TEMP_AP_VOIP_TBL;
		inst_num = getInstNum(name, "X_CT-COM_AP_VoIP");
	}

	if(mib_id == 0 || inst_num == 0)
		return ERR_9005;

	if(get_cfg_template_ap(mib_id, inst_num, &entry, &chainidx) < 0)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Standard" )==0 )
	{
		if( entry.Standard==BAND_11B) //2.4 GHz (B)
			*data = strdup( "b" );
		else if( entry.Standard==BAND_11G )//2.4 GHz (G)
			*data = strdup( "g" );
		else if( entry.Standard==BAND_11BG)//2.4 GHz (B+G)
			*data = strdup( "b,g" );
		else
			*data = strdup( "" );
	}
	else if( strcmp( lastname, "WEPEncryptionLevel" )==0 )
	{
		//0:disable, 1:64, 2:128
		if(entry.WEPEncryptionLevel == 0)
			*data = strdup( "Disabled" );
		else if (entry.WEPEncryptionLevel == 1)
			*data = strdup( "40-bit" );
		else
			*data = strdup( "104-bit" );
	}
	else if( strcmp( lastname, "X_CT-COM_SSIDHide" )==0 )
	{
		*data = booldup(entry.SSIDHide);
	}
	else if( strcmp( lastname, "X_CT-COM_RFBand" )==0 )
	{
		*data = uintdup(entry.RFBand);
	}
	else if( strcmp( lastname, "VLAN" )==0 )
	{
		*data = intdup(entry.VLAN);
	}
	else if( strcmp( lastname, "X_CT-COM_ChannelWidth" )==0 )
	{
		// 0: 20 MHz, 1: 40 MHz, 2: 20/40 MHz
		*data = uintdup(entry.ChannelWidth);
	}
	else if( strcmp( lastname, "X_CT-COM_GuardInterval" )==0 )
	{
		*data = uintdup(entry.GuardInterval);
	}
	else if( strcmp( lastname, "X_CT-COM_RetryTimeout" )==0 )
	{
		*data = uintdup(entry.RetryTimeout);
	}
	else if( strcmp( lastname, "X_CT-COM_Powerlevel" )==0 )
	{
		*data = uintdup( entry.Powerlevel);
	}
	else if(strcmp(lastname,"X_CT-COM_APModuleEnable")==0 )
	{
		*data = booldup(entry.APModuleEnable);
	}
	else if( strcmp( lastname, "X_CT-COM_WPSKeyWord" )==0 )
	{
		*data = uintdup(entry.WPSKeyWord);
	}
	else if( strcmp( lastname, "Enable" )==0 )
	{
		*data = booldup(entry.Enable);
	}
	else if( strcmp( lastname, "Channel" )==0 )
	{
		*data = uintdup(entry.Channel);
	}
	else if( strcmp( lastname, "SSID" )==0 )
	{
		*data = strdup(entry.SSID);
	}
	else if( strcmp( lastname, "BeaconType" )==0 )
	{
		switch(entry.BeaconType)
		{
		case WIFI_SEC_NONE:
			*data = strdup("None");
			break;
		case WIFI_SEC_WEP:
			*data = strdup("Basic");
			break;
		case WIFI_SEC_WPA:
			*data = strdup("WPA");
			break;
		case WIFI_SEC_WPA2:
			*data = strdup("11i");
			break;
		default:
			*data = strdup("");
			break;
		}
	}
	else if( strcmp( lastname, "MACAddressControlEnabled" )==0 )
	{
		*data = booldup(entry.MACAddressControlEnabled);
	}
	else if( strcmp( lastname, "WEPKeyIndex" )==0 )
	{
		*data = uintdup(entry.WEPKeyIndex);
	}
	else if( strcmp( lastname, "BasicAuthenticationMode" )==0 )
	{
		switch(entry.BasicAuthenticationMode)
		{
		case 0:
			*data = strdup( "OpenSystem" );
			break;
		case 1:
			*data = strdup( "SharedKey" );
			break;
		case 2:
			*data = strdup( "Both" );
			break;
		default:
			break;
		}
	}
	else if( strcmp( lastname, "WPAEncryptionModes" )==0 )
	{
		vChar = entry.WPAEncryptionModes;
		if( vChar==WPA_CIPHER_TKIP )
			*data = strdup( "TKIPEncryption" );
		else if( vChar==WPA_CIPHER_AES )
			*data = strdup( "AESEncryption" );
		else if( vChar==WPA_CIPHER_MIXED )
			*data = strdup( "TKIPandAESEncryption" );
	}
	else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		*data = strdup(entry.MaxBitRate);
	}
	else{
		return ERR_9005;
	}

	return 0;
}


int setCT_Template_APEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	int mib_id = 0;
	int inst_num = 0, chainidx = -1;
	MIB_CE_UPNPDM_CFG_TEMP_AP_T entry = {0};

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	if(strstr(name, "X_CT-COM_AP_Internet"))
	{
		mib_id = UPNPDM_CFG_TEMP_AP_INTERNET_TBL;
		inst_num = getInstNum(name, "X_CT-COM_AP_Internet");
	}
	else if(strstr(name, "X_CT-COM_AP_IPTV"))
	{
		mib_id = UPNPDM_CFG_TEMP_AP_IPTV_TBL;
		inst_num = getInstNum(name, "X_CT-COM_AP_IPTV");
	}
	else if(strstr(name, "X_CT-COM_AP_VoIP"))
	{
		mib_id = UPNPDM_CFG_TEMP_AP_VOIP_TBL;
		inst_num = getInstNum(name, "X_CT-COM_AP_VoIP");
	}

	if(mib_id == 0 || inst_num == 0)
		return ERR_9005;

	if(get_cfg_template_ap(mib_id, inst_num, &entry, &chainidx) < 0)
		return ERR_9005;

	if( strcmp( lastname, "Standard" )==0 )
	{
		char *buf = data;

		if(strcmp(buf, "b") == 0)
			entry.Standard = BAND_11B;
		else if(strcmp(buf, "g") == 0)
			entry.Standard = BAND_11G;
		else if(strcmp(buf, "b,g") == 0)
			entry.Standard = BAND_11BG;
		else
			return ERR_9007;
	}
	else if( strcmp( lastname, "WEPEncryptionLevel" )==0 )
	{
		char *buf = data;

		//0:disable, 1:64, 2:128
		if(strcmp(buf, "Disabled") == 0)
			entry.WEPEncryptionLevel = 0;
		else if(strcmp(buf, "40-bit") == 0)
			entry.WEPEncryptionLevel = 1;
		else if(strcmp(buf, "104-bit") == 0)
			entry.WEPEncryptionLevel = 2;
		else
			return ERR_9007;
	}
	else if( strcmp( lastname, "X_CT-COM_SSIDHide" )==0 )
	{
		int *hide = data;
		entry.SSIDHide = *hide;
	}
	else if( strcmp( lastname, "X_CT-COM_RFBand" )==0 )
	{
		unsigned int *band = data;
		entry.RFBand = *band;
	}
	else if( strcmp( lastname, "VLAN" )==0 )
	{
		int *vlan = data;
		entry.VLAN = *vlan;
	}
	else if( strcmp( lastname, "X_CT-COM_ChannelWidth" )==0 )
	{
		unsigned int *value = data;
		// 0: 20 MHz, 1: 40 MHz, 2: 20/40 MHz
		entry.ChannelWidth = *value;
	}
	else if( strcmp( lastname, "X_CT-COM_GuardInterval" )==0 )
	{
		unsigned int *value = data;
		entry.GuardInterval = *value;
	}
	else if( strcmp( lastname, "X_CT-COM_RetryTimeout" )==0 )
	{
		unsigned int *value = data;
		entry.RetryTimeout = *value;
	}
	else if( strcmp( lastname, "X_CT-COM_Powerlevel" )==0 )
	{
		unsigned int *value = data;
		entry.Powerlevel = *value;
	}
	else if(strcmp(lastname,"X_CT-COM_APModuleEnable")==0 )
	{
		int *enable = data;
		entry.APModuleEnable = *enable;
	}
	else if( strcmp( lastname, "X_CT-COM_WPSKeyWord" )==0 )
	{
		unsigned int *value = data;
		entry.WPSKeyWord = *value;
	}
	else if( strcmp( lastname, "Enable" )==0 )
	{
		int *enable = data;
		entry.Enable = *enable;
	}
	else if( strcmp( lastname, "Channel" )==0 )
	{
		unsigned int *value = data;
		entry.Channel = *value;
	}
	else if( strcmp( lastname, "SSID" )==0 )
	{
		char *buf = data;

		strncpy(entry.SSID, buf, MAX_SSID_LEN);
	}
	else if( strcmp( lastname, "BeaconType" )==0 )
	{
		char *buf = data;
		if(strcmp(buf, "None") == 0)
			entry.BeaconType = WIFI_SEC_NONE;
		else if(strcmp(buf, "Basic") == 0)
			entry.BeaconType = WIFI_SEC_WEP;
		else if(strcmp(buf, "WPA") == 0)
			entry.BeaconType = WIFI_SEC_WPA;
		else if(strcmp(buf, "11i") == 0)
			entry.BeaconType = WIFI_SEC_WPA2;
		else
			return ERR_9007;
	}
	else if( strcmp( lastname, "MACAddressControlEnabled" )==0 )
	{
		int *enable = data;
		entry.MACAddressControlEnabled = *enable;
	}
	else if( strcmp( lastname, "WEPKeyIndex" )==0 )
	{
		unsigned int *value = data;
		entry.WEPKeyIndex = *value;
	}
	else if( strcmp( lastname, "BasicAuthenticationMode" )==0 )
	{
		char *buf = data;
		if(strcmp(buf, "OpenSystem") == 0)
			entry.BasicAuthenticationMode = 0;
		else if(strcmp(buf, "SharedKey") == 0)
			entry.BasicAuthenticationMode = 1;
		else if(strcmp(buf, "Both") == 0)
			entry.BasicAuthenticationMode = 2;
		else
			return ERR_9007;
	}
	else if( strcmp( lastname, "WPAEncryptionModes" )==0 )
	{
		char *buf = data;
		if(strcmp(buf, "TKIPEncryption") == 0)
			entry.WPAEncryptionModes = WPA_CIPHER_TKIP;
		else if(strcmp(buf, "AESEncryption") == 0)
			entry.WPAEncryptionModes = WPA_CIPHER_AES;
		else if(strcmp(buf, "TKIPandAESEncryption") == 0)
			entry.WPAEncryptionModes = WPA_CIPHER_MIXED;
		else
			return ERR_9007;
	}
	else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		char *buf = data;

		strncpy(entry.MaxBitRate, buf, 5);
	}
	else{
		return ERR_9005;
	}

	mib_chain_update(mib_id, &entry, chainidx);

	return 0;
}




/***** IGD.X_CT-COM_ProxyDevice.ServiceProfile.ConfigTemplate ****************/
struct CWMP_OP tCT_ConfigTemplateOP = {getCT_ConfigTemplate, NULL};
struct CWMP_OP tCT_Template_APOP = {NULL, objCT_Template_AP};

struct CWMP_PRMT tCT_ConfigTemplateLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"X_CT-COM_AP_InternetNumberOfEntries",			eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_AP_IPTVNumberOfEntries", 			eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_AP_VoIPNumberOfEntries",				eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_STB_InternetNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_STB_IPTVNumberOfEntries",			eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_STB_VOIPNumberOfEntries",			eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_Screen4_IPTVNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_Screen4_InternetNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_Screen4_VoIPNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_HomeCenter_InternetNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_HomeCenter_NASNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_HomeCenter_IPTVNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
{"X_CT-COM_HomeCenter_VoIPNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,	&tCT_ConfigTemplateOP},
};

enum eCT_ConfigTemplateLeaf
{
	eCT_ConfigTemplate_AP_INTERNET,
	eCT_ConfigTemplate_AP_IPTV,
	eCT_ConfigTemplate_AP_VOIP,
	eCT_ConfigTemplate_STB_INTERNET,
	eCT_ConfigTemplate_STB_IPTV,
	eCT_ConfigTemplate_STB_VOIP,
	eCT_ConfigTemplate_SCREEN4_IPTV,
	eCT_ConfigTemplate_SCREEN4_INTERNET,
	eCT_ConfigTemplate_SCREEN4_VOIP,
	eCT_ConfigTemplate_HOMECENTER_INTERNET,
	eCT_ConfigTemplate_HOMECENTER_NAS,
	eCT_ConfigTemplate_HOMECENTER_IPTV,
	eCT_ConfigTemplate_HOMECENTER_VOIP,
};

struct CWMP_LEAF tCT_ConfigTemplateLeaf[] =
{
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_AP_INTERNET] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_AP_IPTV] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_AP_VOIP] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_STB_INTERNET] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_STB_IPTV] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_STB_VOIP] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_SCREEN4_IPTV] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_SCREEN4_INTERNET] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_SCREEN4_VOIP] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_HOMECENTER_INTERNET] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_HOMECENTER_NAS] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_HOMECENTER_IPTV] },
{ &tCT_ConfigTemplateLeafInfo[eCT_ConfigTemplate_HOMECENTER_VOIP] },
{ NULL }
};

struct CWMP_PRMT tCT_ConfigTemplateObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"X_CT-COM_AP_Internet",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, &tCT_Template_APOP},
{"X_CT-COM_AP_IPTV",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tCT_Template_APOP},
{"X_CT-COM_AP_VoIP",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tCT_Template_APOP},
{"X_CT-COM_STB_Internet",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, NULL},
{"X_CT-COM_STB_IPTV",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"X_CT-COM_STB_VoIP",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"X_CT-COM_Screen4_Internet",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, NULL},
{"X_CT-COM_Screen4_VoIP",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"X_CT-COM_Screen4_IPTV",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"X_CT-COM_HomeCenter_Internet",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, NULL},
{"X_CT-COM_HomeCenter_NAS",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, NULL},
{"X_CT-COM_HomeCenter_IPTV",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
{"X_CT-COM_HomeCenter_VoIP",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
};

struct CWMP_NODE tCT_ConfigTemplateObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_AP_INTERNET], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_AP_IPTV], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_AP_VOIP], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_STB_INTERNET], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_STB_IPTV], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_STB_VOIP], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_SCREEN4_IPTV], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_SCREEN4_INTERNET], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_SCREEN4_VOIP], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_HOMECENTER_INTERNET], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_HOMECENTER_NAS], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_HOMECENTER_IPTV], 	NULL,		NULL},
{&tCT_ConfigTemplateObjectInfo[eCT_ConfigTemplate_HOMECENTER_VOIP], 	NULL,		NULL},
{NULL,					NULL,			NULL}
};

int getCT_ConfigTemplate(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "X_CT-COM_AP_InternetNumberOfEntries") == 0)
	{
		*data = uintdup(mib_chain_total(UPNPDM_CFG_TEMP_AP_INTERNET_TBL));
	}
	else if (strcmp(lastname, "X_CT-COM_AP_IPTVNumberOfEntries") == 0)
	{
		*data = uintdup(mib_chain_total(UPNPDM_CFG_TEMP_AP_IPTV_TBL));
	}
	else if (strcmp(lastname, "X_CT-COM_AP_VoIPNumberOfEntries") == 0)
	{
		*data = uintdup(mib_chain_total(UPNPDM_CFG_TEMP_AP_VOIP_TBL));
	}
	else if (strcmp(lastname, "X_CT-COM_STB_InternetNumberOfEntries") == 0)
	{
		// not support yet
		*data = uintdup(0);
	}
	else if (strcmp(lastname, "X_CT-COM_STB_IPTVNumberOfEntries") == 0)
	{
		// not support yet
		*data = uintdup(0);
	}
	else if (strcmp(lastname, "X_CT-COM_STB_VoIPNumberOfEntries") == 0)
	{
		// not support yet
		*data = uintdup(0);
	}
	else if (strcmp(lastname, "X_CT-COM_Screen4_IPTVNumberOfEntries") == 0)
	{
		// not support yet
		*data = uintdup(0);
	}
	else if (strcmp(lastname, "X_CT-COM_Screen4_InternetNumberOfEntries") == 0)
	{
		// not support yet
		*data = uintdup(0);
	}
	else if (strcmp(lastname, "X_CT-COM_Screen4_InternetNumberOfEntries") == 0)
	{
		// not support yet
		*data = uintdup(0);
	}
	else if (strcmp(lastname, "X_CT-COM_HomeCenter_InternetNumberOfEntries") == 0)
	{
		// not support yet
		*data = uintdup(0);
	}
	else if (strcmp(lastname, "X_CT-COM_HomeCenter_NASNumberOfEntries") == 0)
	{
		// not support yet
		*data = uintdup(0);
	}
	else if (strcmp(lastname, "X_CT-COM_HomeCenter_IPTVNumberOfEntries") == 0)
	{
		// not support yet
		*data = uintdup(0);
	}
	else if (strcmp(lastname, "X_CT-COM_HomeCenter_VoIPNumberOfEntries") == 0)
	{
		// not support yet
		*data = uintdup(0);
	}
	else {
		return ERR_9005;
	}

	return 0;
}




/** InternetGatewayDevice.X_CT-COM_ProxyDevice.ServiceProfile.ConfigProfile **/
typedef enum { 
	UPNPDM_MATCH_OUI = 0,
	UPNPDM_MATCH_PRODUCT_CLS,
	UPNPDM_MATCH_SERIAL,
	UPNPDM_MATCH_DEV_TYPE,
	UPNPDM_MATCH_SW_VER,
	UPNPDM_MATCH_HW_VER,
	UPNPDM_MATCH_ATTACHED_PORT,
	UPNPDM_MATCH_MAX,
} UNPDM_CFG_PROFILE_RULE;

static const char *match_type_str[] =
{
	"ManufacturerOUI",
	"ProductClass",
	"SerialNumber",
	"DeviceType",
	"SoftwareVersion",
	"HardwareVersion",
	"AttachedPort",
};

struct CT_cfg_profile_rule
{
	unsigned char match_type[UPNPDM_MATCH_MAX];
	unsigned char is_neq[UPNPDM_MATCH_MAX];	// is "!=" or not
	unsigned char *value[UPNPDM_MATCH_MAX];
};

struct CWMP_LINKNODE *gCT_cfg_profile_list = NULL;

static UNPDM_CFG_PROFILE_RULE get_cfg_rule_type(char *str)
{
	int i;

	if(str == NULL)
		return UPNPDM_MATCH_MAX;

	for(i = UPNPDM_MATCH_OUI ; i < UPNPDM_MATCH_MAX ; i++)
	{
		if(strcmp(match_type_str[i], str) == 0)
			break;
	}
	return i;
}

void free_CT_cfg_profile_rule(void *obj_data)
{
	struct CT_cfg_profile_rule *rule = (struct CT_cfg_profile_rule *)obj_data;
	int i;

	if(rule == NULL)
		return;

	for(i = 0 ; i < UPNPDM_MATCH_MAX ; i++)
	{
		if(rule->value[i])
			free(rule->value[i]);
	}

	free(rule);
}

/* Compile str and save to rule */
int compile_cfg_profile_rule(char *str, int len, struct CT_cfg_profile_rule *rule)
{
	int i = 0;
	unsigned char type = UPNPDM_MATCH_MAX;
	char type_str[64] = "";
	char value[64] = "";
	int tok_start = 0;
	enum {RULE_TYPE, RULE_VALUE} tok_type = RULE_TYPE;

	if(str == NULL || rule == NULL)
		return -1;

	if(len == 0)
		return 0;

	while(i <= len)
	{
		switch(str[i])
		{
		case ',':
		case '\0':
			if(type >= UPNPDM_MATCH_MAX)
			{
				CWMPDBP(1, "UPNPDM unknown type [%s]\n", type_str);
				return -1;
			}

			rule->value[type] = strdup(value);

			CWMPDBP(2,"UPNPDM rule added: %s %s %s\n",
				type_str, (rule->is_neq[type]) ? "!=" : "=", value);

			//reset to find next rule
			tok_start = i + 1;
			tok_type = RULE_TYPE;
			type = UPNPDM_MATCH_MAX;
			memset(type_str, 0, sizeof(type_str));
			memset(value, 0, sizeof(value));
			break;
		case '=':
			type = get_cfg_rule_type(type_str);

			if(type >= UPNPDM_MATCH_MAX)
			{
				CWMPDBP(1, "UPNPDM unknown type %s\n", type_str);
				return -1;
			}

			rule->match_type[type] = 1;

			// Get value next
			tok_start = i + 1;
			tok_type = RULE_VALUE;
		case '!':
			if(i+1 < len && str[i+1] == '=')
			{
				rule->is_neq[type] = 1;
				break;
			}
			//treat '!' as simple char if next char is not '='
		default:
			if(tok_type == RULE_TYPE)
				type_str[i-tok_start] = str[i];
			else
				value[i-tok_start] = str[i];
			break;
		}
		i++;
	}

	return 0;
}

int getCT_ConfigProfile_entry(unsigned int num, MIB_CE_UPNPDM_CFG_PROFILE_T *pEntry, int *chainid)
{
	int i;
	int total = mib_chain_total(UPNPDM_CFG_PROFILE_TBL);

	*chainid = -1;
	for(i = 0 ; i < total; i++)
	{
		if(mib_chain_get(UPNPDM_CFG_PROFILE_TBL, i, pEntry) == 0)
			continue;

		if(pEntry->inst_num == num)
		{
			*chainid = i;
			return 0;
		}
	}

	return -1;
}


struct CWMP_OP tCT_ConfigProfileOP = {NULL, objCT_ConfigProfile};
struct CWMP_OP tCT_ConfigProfileEntityOP = {getCT_ConfigProfileEntity, setCT_ConfigProfileEntity};

struct CWMP_PRMT tCT_ConfigProfileEntityLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"Rule",				eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_ConfigProfileEntityOP},
{"ConfigurationPath",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_ConfigProfileEntityOP},
{"ProvisioningCode",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_ConfigProfileEntityOP},
};

enum eCT_ConfigProfileEntityLeaf
{
	eCT_ConfigProfile_Rule,
	eCT_ConfigProfile_ConfigurationPath,
	eCT_ConfigProfile_ProvisioningCode,
};
struct CWMP_LEAF tCT_ConfigProfileEntityLeaf[] =
{
{ &tCT_ConfigProfileEntityLeafInfo[eCT_ConfigProfile_Rule] },
{ &tCT_ConfigProfileEntityLeafInfo[eCT_ConfigProfile_ConfigurationPath] },
{ &tCT_ConfigProfileEntityLeafInfo[eCT_ConfigProfile_ProvisioningCode] },
{ NULL }
};

struct CWMP_PRMT tCT_ConfigProfileObjectInfo[] =
{
/*(name,		type,		flag,					op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_ConfigProfileObject
{
	eConfigProfile0
};
struct CWMP_LINKNODE tCT_ConfigProfileObject[] =
{
/*info,  					leaf,			next,		sibling,		instnum)*/
{&tCT_ConfigProfileObjectInfo[eConfigProfile0], 	tCT_ConfigProfileEntityLeaf,	NULL,		NULL,			0}
};


int objCT_ConfigProfile(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, num, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;

	if (name == NULL || entity == NULL)
		return -1;

	num = mib_chain_total(UPNPDM_CFG_PROFILE_TBL);

	switch (type) {
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		{
			if (data == NULL)
				return -1;

			ret = add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_ConfigProfileObject,
				       sizeof(tCT_ConfigProfileObject), data);
			if (ret >= 0)
			{
				struct CWMP_LINKNODE *tmp_entity = NULL;
				MIB_CE_UPNPDM_CFG_PROFILE_T entry = {0};

				entry.inst_num = *(int *)data;
				mib_chain_add(UPNPDM_CFG_PROFILE_TBL, &entry);

				//Attach object data
				tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, entry.inst_num);
				if(tmp_entity)
				{
					struct CT_cfg_profile_rule *rule = NULL;
					rule = malloc(sizeof(struct CT_cfg_profile_rule));
					if(rule == NULL)
					{
						fprintf(stderr, "<%s:%d> malloc failed!\n", __FUNCTION__, __LINE__);
						return ERR_9004;
					}
					memset(rule, 0, sizeof(struct CT_cfg_profile_rule));
					
					tmp_entity->obj_data = (void *)rule;
					tmp_entity->free_obj_data = free_CT_cfg_profile_rule;
				}
			}

			gCT_cfg_profile_list = (struct CWMP_LINKNODE *)entity->next;
			break;
		}
	case eCWMP_tDELOBJ:
		{
			unsigned int *pUint = data;
			MIB_CE_UPNPDM_CFG_PROFILE_T entry = {0};

			ret = ERR_9005;

			if (data == NULL)
				return -1;

			for (i = 0; i < num; i++)
			{
				if (!mib_chain_get(UPNPDM_CFG_PROFILE_TBL, i, &entry))
					continue;

				if (entry.inst_num == *pUint)
				{
					mib_chain_delete(UPNPDM_CFG_PROFILE_TBL, i);
					ret = del_Object(name, (struct CWMP_LINKNODE **) &entity->next, *(int *)data);
					break;;
				}
			}
			gCT_cfg_profile_list = (struct CWMP_LINKNODE *)entity->next;
			break;
		}
	case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;
			MIB_CE_UPNPDM_CFG_PROFILE_T entry = {0};

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < num; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL;

				if (!mib_chain_get(UPNPDM_CFG_PROFILE_TBL, i, &entry))
					continue;

				remove_entity = remove_SiblingEntity(&old_table, entry.inst_num);

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					struct CWMP_LINKNODE *tmp_entity = NULL;

					add_Object(name,
						   (struct CWMP_LINKNODE **) &entity->next,
						   tCT_ConfigProfileObject,
						   sizeof(tCT_ConfigProfileObject),
						   &entry.inst_num);

					tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, entry.inst_num);
					if(tmp_entity)
					{
						struct CT_cfg_profile_rule *rule = NULL;

						rule = malloc(sizeof(struct CT_cfg_profile_rule));
						if(rule == NULL)
						{
							fprintf(stderr, "<%s:%d> malloc failed!\n", __FUNCTION__, __LINE__);
							return ERR_9002;
						}
						memset(rule, 0, sizeof(struct CT_cfg_profile_rule));
						compile_cfg_profile_rule(entry.rule, strlen(entry.rule), rule);

						tmp_entity->obj_data = (void *)rule;
						tmp_entity->free_obj_data = free_CT_cfg_profile_rule;
					}
				}
			}
			gCT_cfg_profile_list = (struct CWMP_LINKNODE *)entity->next;

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)old_table);

			break;
		}
	}

	return ret;
}

int getCT_ConfigProfileEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	MIB_CE_UPNPDM_CFG_PROFILE_T entry = {0};
	int chainid = -1;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	num = getInstNum(name, "ConfigProfile");

	if(getCT_ConfigProfile_entry(num, &entry, &chainid) == -1)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "Rule") == 0)
	{
		*data = strdup(entry.rule);
	}
	else if (strcmp(lastname, "ConfigurationPath") == 0)
	{
		*data  = strdup(entry.cfg_path);
	}
	else if (strcmp(lastname, "ProvisioningCode") == 0)
	{
		*data  = strdup(entry.provisioning_code);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_ConfigProfileEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	MIB_CE_UPNPDM_CFG_PROFILE_T entry = {0};
	int chainid;
	char *buf = (char *)data;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	num = getInstNum(name, "ConfigProfile");

	if(getCT_ConfigProfile_entry(num, &entry, &chainid) == -1)
		return ERR_9005;

	if (strcmp(lastname, "Rule") == 0)
	{
		struct CT_cfg_profile_rule *rule = NULL;
		struct CWMP_LINKNODE *profile_entity = NULL;

		profile_entity = find_SiblingEntity( &gCT_cfg_profile_list, num);

		rule = malloc(sizeof(struct CT_cfg_profile_rule));
		if(rule == NULL)
			return ERR_9002;

		memset(rule, 0, sizeof(struct CT_cfg_profile_rule));
		if(compile_cfg_profile_rule(buf, strlen(buf), rule) < 0)
		{
			free_CT_cfg_profile_rule((void *) rule);
			return ERR_9007;
		}
		free_CT_cfg_profile_rule(profile_entity->obj_data);
		profile_entity->obj_data = (void *)rule;

		strcpy(entry.rule, buf);
	}
	else if (strcmp(lastname, "ConfigurationPath") == 0)
	{
		if(strlen(buf) > 256)
			return ERR_9007;

		strncpy(entry.cfg_path, buf, sizeof(entry.cfg_path));
		//TODO: apply
	}
	else if (strcmp(lastname, "ProvisioningCode") == 0)
	{
		if(strlen(buf) > 64)
			return ERR_9007;

		strncpy(entry.provisioning_code, buf, sizeof(entry.provisioning_code));
	}
	else {
		return ERR_9005;
	}

	mib_chain_update(UPNPDM_CFG_PROFILE_TBL, &entry, chainid);

	return 0;
}





/***** InternetGatewayDevice.X_CT-COM_ProxyDevice.ServiceProfile. ************/
struct CWMP_OP tCT_ServiceProfileLeafOP = {getCT_ServiceProfile, setCT_ServiceProfile};
struct CWMP_PRMT tCT_ServiceProfileLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"ConfigProfileNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,			&tCT_ServiceProfileLeafOP},
{"X_CT-COM_RMSConfigOver",			eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tCT_ServiceProfileLeafOP},
{"X_CT-COM_ConfigurationMode",		eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tCT_ServiceProfileLeafOP},
};
enum eCT_ServiceProfileLeaf
{
	eCT_ConfigProfileNumberOfEntries,
	eCT_RMSConfigOver,
	eCT_ConfigurationMode,
};
struct CWMP_LEAF tCT_ServiceProfileLeaf[] =
{
{ &tCT_ServiceProfileLeafInfo[eCT_ConfigProfileNumberOfEntries] },
{ &tCT_ServiceProfileLeafInfo[eCT_RMSConfigOver] },
{ &tCT_ServiceProfileLeafInfo[eCT_ConfigurationMode] },
{ NULL }
};

struct CWMP_PRMT tCT_ServiceProfileObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"ConfigProfile",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, 	&tCT_ConfigProfileOP},
{"ConfigTemplate",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eCT_ServiceProfileObject
{
	eCT_ConfigProfile,
	eCT_ConfigTemplate,
};

struct CWMP_NODE tCT_ServiceProfileObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_ServiceProfileObjectInfo[eCT_ConfigProfile], 	NULL,		NULL},
{&tCT_ServiceProfileObjectInfo[eCT_ConfigTemplate],	tCT_ConfigTemplateLeaf, 		tCT_ConfigTemplateObject},
{NULL,					NULL,			NULL}
};


int getCT_ServiceProfile(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "ConfigProfileNumberOfEntries") == 0)
	{
		*data = uintdup(mib_chain_total(UPNPDM_CFG_PROFILE_TBL));
	}
	else if (strcmp(lastname, "X_CT-COM_RMSConfigOver") == 0)
	{
		unsigned char config_over;

		mib_get(UPNPDM_RMS_CONFIG_OVER, &config_over);
		*data  = uintdup(config_over);
	}
	else if (strcmp(lastname, "X_CT-COM_ConfigurationMode") == 0)
	{
		unsigned char mode;

		mib_get(UPNPDM_CONFIG_MODE, &mode);
		*data  = uintdup(mode);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_ServiceProfile(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "X_CT-COM_RMSConfigOver") == 0)
	{
		unsigned int *i = data;
		unsigned char config_over=0;

		config_over = *i;
		mib_set(UPNPDM_RMS_CONFIG_OVER, &config_over);
	}
	else if (strcmp(lastname, "X_CT-COM_ConfigurationMode") == 0)
	{
		unsigned int *i = data;
		unsigned char mode=0;

		mode = *i;
		mib_set(UPNPDM_CONFIG_MODE, &mode);
	}
	else {
		return ERR_9005;
	}

	return 0;
}


/*** IGD.X_CT-COM_PorxyDevice.SoftwareProfile.FileProfile.{i}.TimeWindowList */
struct CWMP_OP tCT_FP_TimeWinOP = {NULL, objCT_TimeWin};
struct CWMP_OP tCT_FP_TimeWinEntityLeafOP = {getCT_FP_TimeWinEntity, setCT_FP_TimeWinEntity};

struct CWMP_PRMT tCT_FP_TimeWinEntityLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"WindowStart",	eCWMP_tUINT,		CWMP_READ|CWMP_WRITE,		&tCT_FP_TimeWinEntityLeafOP},
{"WindowEnd",		eCWMP_tUINT,		CWMP_READ|CWMP_WRITE,		&tCT_FP_TimeWinEntityLeafOP},
{"WindowMode",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_FP_TimeWinEntityLeafOP},
{"UserMessage", 	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_FP_TimeWinEntityLeafOP},
{"MaxRetries", 	eCWMP_tINT,		CWMP_READ|CWMP_WRITE,		&tCT_FP_TimeWinEntityLeafOP},
};

enum eCT_FP_TimeWinEntityLeaf
{
	eCT_FP_TimeWin_WindowStart,
	eCT_FP_TimeWin_WindowEnd,
	eCT_FP_TimeWin_WindowMode,
	eCT_FP_TimeWin_UserMessage,
	eCT_FP_TimeWin_MaxRetries,
};

struct CWMP_LEAF tCT_FP_TimeWinEntityLeaf[] =
{
{ &tCT_FP_TimeWinEntityLeafInfo[eCT_FP_TimeWin_WindowStart] },
{ &tCT_FP_TimeWinEntityLeafInfo[eCT_FP_TimeWin_WindowEnd] },
{ &tCT_FP_TimeWinEntityLeafInfo[eCT_FP_TimeWin_WindowMode] },
{ &tCT_FP_TimeWinEntityLeafInfo[eCT_FP_TimeWin_UserMessage] },
{ &tCT_FP_TimeWinEntityLeafInfo[eCT_FP_TimeWin_MaxRetries] },
{ NULL }
};

struct CWMP_PRMT tCT_FP_TimeWinObjectInfo[] =
{
/*(name,		type,		flag,					op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};

struct CWMP_LINKNODE tCT_FP_TimeWinObject[] =
{
/*info,  					leaf,			next,		sibling,		instnum)*/
{&tCT_FP_TimeWinObjectInfo[0], 	tCT_FP_TimeWinEntityLeaf,	NULL,		NULL,			0}
};

int objCT_TimeWin(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, maxInstNum, fp_num;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	MIB_CE_UPNPDM_FILE_PROFILE_T entry = {0};
	int fp_chainid = -1;

	if (name == NULL || entity == NULL)
		return -1;

	switch (type) {
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		{
			if (data == NULL)
				return -1;

			fp_num = getInstNum(name, "FileProfile");

			if(getCT_FileProfile_entry(fp_num, &entry, &fp_chainid) == -1)
				return ERR_9005;

			for(i = 0 ; i < UPNPDM_TIME_WIN_MAX ; i++)
			{
				if(entry.tw_inst_num[i] == 0)
					break;
			}
			if(i == UPNPDM_TIME_WIN_MAX)
				return ERR_9004;

			ret = add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_FP_TimeWinObject,
				       sizeof(tCT_FP_TimeWinObject), data);

			if (ret < 0)
				return ERR_9002;

			entry.tw_inst_num[i] = *(int *)data;
			entry.tw_max_retries[i] = 0;
			entry.tw_start[i] = 0;
			entry.tw_end[i] = 0;
			entry.tw_mode[i] = CWMP_TIME_WIN_MODE_AT_ANY_TIME;
			mib_chain_update(UPNPDM_FILE_PROFILE_TBL, &entry, fp_chainid);
			
			break;
		}
	case eCWMP_tDELOBJ:
		{
			unsigned int *pUint = data;

			if (data == NULL)
				return -1;

			fp_num = getInstNum(name, "FileProfile");

			if(getCT_FileProfile_entry(fp_num, &entry, &fp_chainid) == -1)
				return ERR_9005;

			for(i = 0 ; i < UPNPDM_TIME_WIN_MAX ; i++)
			{
				if(entry.tw_inst_num[i] == *(int *)data)
				{
					entry.tw_inst_num[i] = 0;
					mib_chain_update(UPNPDM_FILE_PROFILE_TBL, &entry, fp_chainid);
					ret = del_Object(name, (struct CWMP_LINKNODE **) &entity->next, *(int *)data);
					break;
				}
			}
			if(i == UPNPDM_TIME_WIN_MAX)
				return ERR_9005;
			break;
		}
	case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;

			fp_num = getInstNum(name, "FileProfile");

			if(getCT_FileProfile_entry(fp_num, &entry, &fp_chainid) == -1)
				return ERR_9005;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0 ; i < UPNPDM_TIME_WIN_MAX; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL;

				if(entry.tw_inst_num[i] == 0)
					continue;

				remove_entity = remove_SiblingEntity(&old_table, entry.tw_inst_num[i]);

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					struct CWMP_LINKNODE *tmp_entity = NULL;

					add_Object(name,
						   (struct CWMP_LINKNODE **) &entity->next,
						   tCT_FP_TimeWinObject,
						   sizeof(tCT_FP_TimeWinObject),
						   &entry.tw_inst_num[i]);
				}
			}

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)old_table);

			break;
		}
	}

	return ret;
}

int getCT_FP_TimeWinEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int num = 0, fp_num;
	MIB_CE_UPNPDM_FILE_PROFILE_T entry = {0};
	int chainid = -1;
	int i;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	fp_num = getInstNum(name, "FileProfile");

	if(getCT_FileProfile_entry(fp_num, &entry, &chainid) == -1)
		return ERR_9005;

	num = getInstNum(name, "TimeWindowList");

	for(i = 0 ; i < UPNPDM_TIME_WIN_MAX ; i++)
	{
		if(entry.tw_inst_num[i] == num)
			break;
	}
	if(i == UPNPDM_TIME_WIN_MAX)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "WindowStart") == 0)
	{
		*data = uintdup(entry.tw_start[i]);
	}
	else if (strcmp(lastname, "WindowEnd") == 0)
	{
		*data = uintdup(entry.tw_end[i]);
	}
	else if (strcmp(lastname, "WindowMode") == 0)
	{
		switch(entry.tw_mode[i])
		{
		case CWMP_TIME_WIN_MODE_AT_ANY_TIME:
			*data  = strdup("1 At Any Time");
			break;
		case CWMP_TIME_WIN_MODE_IMMEDIATELY:
			*data  = strdup("2 Immediately");
			break;
		case CWMP_TIME_WIN_MODE_WHEN_IDLE:
			*data  = strdup("3 When Idle");
			break;
		case CWMP_TIME_WIN_MODE_CONFIRM_NEEDED:
			*data  = strdup("4 Confirmation Needed");
			break;
		default:
			*data  = strdup("");
			break;
		}
	}
	else if (strcmp(lastname, "UserMessage") == 0)
	{
		*data  = strdup(entry.tw_user_msg[i]);
	}
	else if (strcmp(lastname, "MaxRetries") == 0)
	{
		*data  = intdup(entry.tw_max_retries[i]);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_FP_TimeWinEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int num = 0, fp_num;
	MIB_CE_UPNPDM_FILE_PROFILE_T entry = {0};
	int chainid;
	char *buf = (char *)data;
	int i;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	fp_num = getInstNum(name, "FileProfile");

	if(getCT_FileProfile_entry(fp_num, &entry, &chainid) == -1)
		return ERR_9005;

	num = getInstNum(name, "TimeWindowList");

	for(i = 0 ; i < UPNPDM_TIME_WIN_MAX ; i++)
	{
		if(entry.tw_inst_num[i] == num)
			break;
	}
	if(i == UPNPDM_TIME_WIN_MAX)
		return ERR_9005;

	if (strcmp(lastname, "WindowStart") == 0)
	{
		unsigned int *start = data;

		entry.tw_start[i] = *start;
	}
	else if (strcmp(lastname, "WindowEnd") == 0)
	{
		unsigned int *end = data;

		entry.tw_end[i] = *end;
	}
	else if (strcmp(lastname, "WindowMode") == 0)
	{
		char *buf = data;

		if(strcmp(buf, "1 At Any Time") == 0)
			entry.tw_mode[i] = CWMP_TIME_WIN_MODE_AT_ANY_TIME;
		else if(strcmp(buf, "2 Immediately") == 0)
			entry.tw_mode[i] = CWMP_TIME_WIN_MODE_IMMEDIATELY;
		else if(strcmp(buf, "3 When Idle") == 0)
			entry.tw_mode[i] = CWMP_TIME_WIN_MODE_WHEN_IDLE;
		else if(strcmp(buf, "4 Confirmation Needed") == 0)
			entry.tw_mode[i] = CWMP_TIME_WIN_MODE_CONFIRM_NEEDED;
		else
			return ERR_9007;
		
	}
	else if (strcmp(lastname, "UserMessage") == 0)
	{
		char *buf = data;
		strncpy(entry.tw_user_msg[i], buf, sizeof(entry.tw_user_msg[i]));
	}
	else if (strcmp(lastname, "MaxRetries") == 0)
	{
		int *max = data;
		entry.tw_max_retries[i] = *max;
	}
	else {
		return ERR_9005;
	}

	mib_chain_update(UPNPDM_FILE_PROFILE_TBL, &entry, chainid);

	return 0;
}



/***** IGD.X_CT-COM_PorxyDevice.SoftwareProfile.FileProfile ******************/
struct CWMP_LINKNODE *gCT_file_profile_list = NULL;

int getCT_FileProfile_entry(unsigned int num, MIB_CE_UPNPDM_FILE_PROFILE_T *pEntry, int *chainid)
{
	int i;
	int total = mib_chain_total(UPNPDM_FILE_PROFILE_TBL);

	*chainid = -1;
	for(i = 0 ; i < total; i++)
	{
		if(mib_chain_get(UPNPDM_FILE_PROFILE_TBL, i, pEntry) == 0)
			continue;

		if(pEntry->inst_num == num)
		{
			*chainid = i;
			return 0;
		}
	}

	return -1;
}


struct CWMP_OP tCT_FileProfileOP = {NULL, objCT_FileProfile};
struct CWMP_OP tCT_FileProfileEntityLeafOP = {getCT_FileProfileEntity, setCT_FileProfileEntity};

struct CWMP_PRMT tCT_FileProfileEntityLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"Rule",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_FileProfileEntityLeafOP},
{"FileType",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_FileProfileEntityLeafOP},
{"URL",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_FileProfileEntityLeafOP},
{"SoftwareVersion", 	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_FileProfileEntityLeafOP},
{"HardwareVersion", 	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_FileProfileEntityLeafOP},
{"Channel", 	eCWMP_tINT,	CWMP_READ|CWMP_WRITE,		&tCT_FileProfileEntityLeafOP},
{"Username", 	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_FileProfileEntityLeafOP},
{"Password", 	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_FileProfileEntityLeafOP},
{"TimeWindowListNumberOfEntries", 	eCWMP_tUINT, CWMP_READ|CWMP_WRITE,		&tCT_FileProfileEntityLeafOP},
};

enum eCT_FileProfileEntityLeaf
{
	eCT_FileProfile_Rule,
	eCT_FileProfile_FileProfile,
	eCT_FileProfile_URL,
	eCT_FileProfile_SoftwareVersion,
	eCT_FileProfile_HardwareVersion,
	eCT_FileProfile_Channel,
	eCT_FileProfile_Username,
	eCT_FileProfile_Password,
	eCT_FileProfile_TimeWindowListNumberOfEntries,
};

struct CWMP_LEAF tCT_FileProfileEntityLeaf[] =
{
{ &tCT_FileProfileEntityLeafInfo[eCT_FileProfile_Rule] },
{ &tCT_FileProfileEntityLeafInfo[eCT_FileProfile_FileProfile] },
{ &tCT_FileProfileEntityLeafInfo[eCT_FileProfile_URL] },
{ &tCT_FileProfileEntityLeafInfo[eCT_FileProfile_SoftwareVersion] },
{ &tCT_FileProfileEntityLeafInfo[eCT_FileProfile_HardwareVersion] },
{ &tCT_FileProfileEntityLeafInfo[eCT_FileProfile_Channel] },
{ &tCT_FileProfileEntityLeafInfo[eCT_FileProfile_Username] },
{ &tCT_FileProfileEntityLeafInfo[eCT_FileProfile_Password] },
{ &tCT_FileProfileEntityLeafInfo[eCT_FileProfile_TimeWindowListNumberOfEntries] },
{ NULL }
};

struct CWMP_PRMT tCT_FileProfileEntityObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"TimeWindowList",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, 	&tCT_FP_TimeWinOP},
};

enum eCT_FileProfileEntityObject
{
	eCT_TimeWinList,
};

struct CWMP_NODE tCT_FileProfileEntityObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_FileProfileEntityObjectInfo[eCT_TimeWinList], 	NULL,		NULL},
{NULL,					NULL,			NULL}
};


struct CWMP_PRMT tCT_FileProfileObjectInfo[] =
{
/*(name,		type,		flag,					op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
struct CWMP_LINKNODE tCT_FileProfileObject[] =
{
/*info,  					leaf,			next,		sibling,		instnum)*/
{&tCT_FileProfileObjectInfo[0], 	tCT_FileProfileEntityLeaf,	tCT_FileProfileEntityObject,		NULL,			0}
};

int objCT_FileProfile(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, num, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;

	if (name == NULL || entity == NULL)
		return -1;

	num = mib_chain_total(UPNPDM_FILE_PROFILE_TBL);

	switch (type) {
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		{
			if (data == NULL)
				return -1;

			ret = add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_FileProfileObject,
				       sizeof(tCT_FileProfileObject), data);
			if (ret >= 0)
			{
				struct CWMP_LINKNODE *tmp_entity = NULL;
				MIB_CE_UPNPDM_FILE_PROFILE_T entry = {0};

				entry.inst_num = *(int *)data;
				entry.channel = -1;
				entry.file_type = DLTYPE_IMAGE;
				mib_chain_add(UPNPDM_FILE_PROFILE_TBL, &entry);

				//Attach object data
				tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, entry.inst_num);
				if(tmp_entity)
				{
					struct CT_cfg_profile_rule *rule = NULL;
					rule = malloc(sizeof(struct CT_cfg_profile_rule));
					if(rule == NULL)
					{
						fprintf(stderr, "<%s:%d> malloc failed!\n", __FUNCTION__, __LINE__);
						return ERR_9004;
					}
					memset(rule, 0, sizeof(struct CT_cfg_profile_rule));
					
					tmp_entity->obj_data = (void *)rule;
					tmp_entity->free_obj_data = free_CT_cfg_profile_rule;
				}
			}

			gCT_file_profile_list = (struct CWMP_LINKNODE *)entity->next;
			break;
		}
	case eCWMP_tDELOBJ:
		{
			unsigned int *pUint = data;
			MIB_CE_UPNPDM_FILE_PROFILE_T entry = {0};

			ret = ERR_9005;

			if (data == NULL)
				return -1;

			for (i = 0; i < num; i++)
			{
				if (!mib_chain_get(UPNPDM_FILE_PROFILE_TBL, i, &entry))
					continue;

				if (entry.inst_num == *pUint)
				{
					mib_chain_delete(UPNPDM_FILE_PROFILE_TBL, i);
					ret = del_Object(name, (struct CWMP_LINKNODE **) &entity->next, *(int *)data);
					break;
				}
			}
			gCT_file_profile_list = (struct CWMP_LINKNODE *)entity->next;
			break;
		}
	case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;
			MIB_CE_UPNPDM_FILE_PROFILE_T entry = {0};

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < num; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL;

				if (!mib_chain_get(UPNPDM_FILE_PROFILE_TBL, i, &entry))
					continue;

				remove_entity = remove_SiblingEntity(&old_table, entry.inst_num);

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					struct CWMP_LINKNODE *tmp_entity = NULL;

					add_Object(name,
						   (struct CWMP_LINKNODE **) &entity->next,
						   tCT_FileProfileObject,
						   sizeof(tCT_FileProfileObject),
						   &entry.inst_num);

					tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, entry.inst_num);
					if(tmp_entity)
					{
						struct CT_cfg_profile_rule *rule = NULL;

						rule = malloc(sizeof(struct CT_cfg_profile_rule));
						if(rule == NULL)
						{
							fprintf(stderr, "<%s:%d> malloc failed!\n", __FUNCTION__, __LINE__);
							return ERR_9002;
						}
						memset(rule, 0, sizeof(struct CT_cfg_profile_rule));
						compile_cfg_profile_rule(entry.rule, strlen(entry.rule), rule);

						tmp_entity->obj_data = (void *)rule;
						tmp_entity->free_obj_data = free_CT_cfg_profile_rule;
					}
				}
			}
			gCT_file_profile_list = (struct CWMP_LINKNODE *)entity->next;

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)old_table);

			break;
		}
	}

	return ret;
}

int getCT_FileProfileEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	MIB_CE_UPNPDM_FILE_PROFILE_T entry = {0};
	int chainid = -1;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	num = getInstNum(name, "FileProfile");

	if(getCT_FileProfile_entry(num, &entry, &chainid) == -1)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "Rule") == 0)
	{
		*data = strdup(entry.rule);
	}
	else if (strcmp(lastname, "FileType") == 0)
	{
		switch(entry.file_type)
		{
		case DLTYPE_IMAGE:
			*data  = strdup("1 Firmware Upgrade Image");
			break;
		case DLTYPE_WEB:
			*data  = strdup("2 Web Content");
			break;
		case DLTYPE_CONFIG:
			*data  = strdup("3 Vendor Configuration File");
			break;
		default:
			*data  = strdup("");
			break;
		}
	}
	else if (strcmp(lastname, "URL") == 0)
	{
		*data  = strdup(entry.url);
	}
	else if (strcmp(lastname, "SoftwareVersion") == 0)
	{
		*data  = strdup(entry.sw_ver);
	}
	else if (strcmp(lastname, "HardwareVersion") == 0)
	{
		*data  = strdup(entry.hw_ver);
	}
	else if (strcmp(lastname, "Channel") == 0)
	{
		*data  = intdup(entry.channel);
	}
	else if (strcmp(lastname, "Username") == 0)
	{
		*data  = strdup(entry.username);
	}
	else if (strcmp(lastname, "Password") == 0)
	{
		*data  = strdup(entry.password);
	}
	else if (strcmp(lastname, "TimeWindowListNumberOfEntries") == 0)
	{
		unsigned int cnt = 0;
		int i;

		for(i = 0 ; i < UPNPDM_TIME_WIN_MAX ; i++)
		{
			if(entry.tw_inst_num[i] != 0)
				cnt++;
		}
		*data  = uintdup(cnt);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_FileProfileEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	MIB_CE_UPNPDM_FILE_PROFILE_T entry = {0};
	int chainid;
	char *buf = (char *)data;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	num = getInstNum(name, "FileProfile");

	if(getCT_FileProfile_entry(num, &entry, &chainid) == -1)
		return ERR_9005;

	if (strcmp(lastname, "Rule") == 0)
	{
		struct CT_cfg_profile_rule *rule = NULL;
		struct CWMP_LINKNODE *profile_entity = NULL;

		profile_entity = find_SiblingEntity( &gCT_file_profile_list, num);

		rule = malloc(sizeof(struct CT_cfg_profile_rule));
		if(rule == NULL)
			return ERR_9002;

		memset(rule, 0, sizeof(struct CT_cfg_profile_rule));
		if(compile_cfg_profile_rule(buf, strlen(buf), rule) < 0
			|| rule->match_type[UPNPDM_MATCH_ATTACHED_PORT])
		{
			free_CT_cfg_profile_rule((void *) rule);
			return ERR_9007;
		}

		free_CT_cfg_profile_rule(profile_entity->obj_data);
		profile_entity->obj_data = (void *)rule;

		strcpy(entry.rule, buf);
	}
	else if (strcmp(lastname, "FileType") == 0)
	{
		char *buf = data;

		if(strcmp(buf, "1 Firmware Upgrade Image") == 0)
			entry.file_type = DLTYPE_IMAGE;
		else if(strcmp(buf, "2 Web Content") == 0)
			entry.file_type = DLTYPE_WEB;
		else if(strcmp(buf, "3 Vendor Configuration File") == 0)
			entry.file_type = DLTYPE_CONFIG;
		else
			return ERR_9007;
	}
	else if (strcmp(lastname, "URL") == 0)
	{
		char *buf = data;

		strncpy(entry.url, buf, sizeof(entry.url));
	}
	else if (strcmp(lastname, "SoftwareVersion") == 0)
	{
		char *buf = data;

		strncpy(entry.sw_ver, buf, sizeof(entry.sw_ver));
	}
	else if (strcmp(lastname, "HardwareVersion") == 0)
	{
		char *buf = data;

		strncpy(entry.hw_ver, buf, sizeof(entry.hw_ver));
	}
	else if (strcmp(lastname, "Channel") == 0)
	{
		int *i = data;

		if(*i < -1 || *i > 4095)
			return ERR_9007;

		entry.channel = *i;
	}
	else if (strcmp(lastname, "Username") == 0)
	{
		char *buf = data;

		strncpy(entry.username, buf, sizeof(entry.username));
	}
	else if (strcmp(lastname, "Password") == 0)
	{
		char *buf = data;

		strncpy(entry.password, buf, sizeof(entry.password));
	}
	else {
		return ERR_9005;
	}

	mib_chain_update(UPNPDM_FILE_PROFILE_TBL, &entry, chainid);

	return 0;
}



/***** IGD.X_CT-COM_PorxyDevice.SoftwareProfile. *****************************/
struct CWMP_OP tCT_SoftwareProfileLeafOP = {getCT_SoftwareProfile, setCT_SoftwareProfile};
struct CWMP_PRMT tCT_SoftwareProfileLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"X_CT-COM_RMSSoftwareProfileConfigOver",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tCT_SoftwareProfileLeafOP},
{"FileProfileNumberOfEntries",				eCWMP_tUINT,	CWMP_READ,			&tCT_SoftwareProfileLeafOP},
};
enum eCT_SoftwareProfileLeaf
{
	eCT_RMSSoftwareProfileConfigOver,
	eCT_FileProfileNumberOfEntries,
};
struct CWMP_LEAF tCT_SoftwareProfileLeaf[] =
{
{ &tCT_SoftwareProfileLeafInfo[eCT_RMSSoftwareProfileConfigOver] },
{ &tCT_SoftwareProfileLeafInfo[eCT_FileProfileNumberOfEntries] },
{ NULL }
};

struct CWMP_PRMT tCT_SoftwareProfileObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"FileProfile",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, 	&tCT_FileProfileOP},
};

enum eCT_SoftwareProfileObject
{
	eCT_FileProfile,
};

struct CWMP_NODE tCT_SoftwareProfileObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_SoftwareProfileObjectInfo[eCT_FileProfile], 	NULL,		NULL},
{NULL,					NULL,			NULL}
};


int getCT_SoftwareProfile(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "FileProfileNumberOfEntries") == 0)
	{
		*data = uintdup(mib_chain_total(UPNPDM_FILE_PROFILE_TBL));
	}
	else if (strcmp(lastname, "X_CT-COM_RMSSoftwareProfileConfigOver") == 0)
	{
		unsigned char config_over;

		mib_get(UPNPDM_RMS_SW_CONFIG_OVER, &config_over);
		*data  = uintdup(config_over);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_SoftwareProfile(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "X_CT-COM_RMSSoftwareProfileConfigOver") == 0)
	{
		unsigned int *i = data;
		unsigned char config_over=0;

		config_over = *i;
		mib_set(UPNPDM_RMS_SW_CONFIG_OVER, &config_over);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

/***** InternetGatewayDevice.X_CT-COM_ProxyDevice. ***************************/
struct CWMP_OP tCT_ProxyDeviceLeafOP = {getCT_ProxyDevice, NULL};
struct CWMP_PRMT tCT_ProxyDeviceLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"DeviceListNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tCT_ProxyDeviceLeafOP},
};
enum eCT_ProxyDevice
{
	eCT_DeviceListNumberOfEntries,
};
struct CWMP_LEAF tCT_ProxyDeviceLeaf[] =
{
{ &tCT_ProxyDeviceLeafInfo[eCT_DeviceListNumberOfEntries] },
{ NULL }
};

struct CWMP_PRMT tCT_ProxyDeviceObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"ServiceProfile",	eCWMP_tOBJECT,	CWMP_READ, 	NULL},
{"SoftwareProfile",	eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"DeviceList",		eCWMP_tOBJECT,	CWMP_READ,	&tCT_DeviceListOP},
};

enum eCT_ProxyDeviceObject
{
	eCT_ServiceProfile,
	eCT_SoftwareProfile,
	eCT_DeviceList,
};

struct CWMP_NODE tCT_ProxyDeviceObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_ProxyDeviceObjectInfo[eCT_ServiceProfile], 	tCT_ServiceProfileLeaf,			tCT_ServiceProfileObject},
{&tCT_ProxyDeviceObjectInfo[eCT_SoftwareProfile],	tCT_SoftwareProfileLeaf,			tCT_SoftwareProfileObject},
{&tCT_ProxyDeviceObjectInfo[eCT_DeviceList], 		NULL,			NULL},
{NULL,					NULL,			NULL}
};

int getCT_ProxyDevice(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "DeviceListNumberOfEntries") == 0)
	{
		*data = uintdup(get_device_cnt());
	}
	else {
		return ERR_9005;
	}

	return 0;
}


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <linux/ethtool.h>

#include <rtk/mib.h>
#include <rtk/utility.h>
#include <parameter_api.h>
#include <libcwmp.h>

#include "../prmt_apply.h"
#include "../cwmpc_utility.h"
#include "prmt_wifi_radio.h"
/****** Device.WiFi.Radio.{i}.Stats ******************************************/
struct CWMP_OP tWiFiRadioStatsLeafOP = { getWiFiRadioStats, NULL};

struct CWMP_PRMT tWiFiRadioStatsLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"BytesSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiRadioStatsLeafOP},
{"BytesReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiRadioStatsLeafOP},
{"PacketsSent",		eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiRadioStatsLeafOP},
{"PacketsReceived",	eCWMP_tULONG,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiRadioStatsLeafOP},
{"ErrorsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiRadioStatsLeafOP},
{"ErrorsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiRadioStatsLeafOP},
{"DiscardPacketsSent",		eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiRadioStatsLeafOP},
{"DiscardPacketsReceived",	eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,		&tWiFiRadioStatsLeafOP},
};

enum eWiFiRadioStatsLeaf
{
	eStatsBytesSent,
	eStatsBytesReceived,
	eStatsPacketsSent,
	eStatsPacketsReceived,
	eStatsErrorsSent,
	eStatsErrorsReceived,
	eStatsDiscardPacketsSent,
	eStatsDiscardPacketsReceived,
};

struct CWMP_LEAF tWiFiRadioStatsLeaf[] =
{
{ &tWiFiRadioStatsLeafInfo[eStatsBytesSent]  },
{ &tWiFiRadioStatsLeafInfo[eStatsBytesReceived]  },
{ &tWiFiRadioStatsLeafInfo[eStatsPacketsSent]  },
{ &tWiFiRadioStatsLeafInfo[eStatsPacketsReceived]  },
{ &tWiFiRadioStatsLeafInfo[eStatsErrorsSent]  },
{ &tWiFiRadioStatsLeafInfo[eStatsErrorsReceived]  },
{ &tWiFiRadioStatsLeafInfo[eStatsDiscardPacketsSent]  },
{ &tWiFiRadioStatsLeafInfo[eStatsDiscardPacketsReceived]  },
{ NULL	}
};

/****** Device.WiFi.Radio.{i} ************************************************/
struct CWMP_OP tWiFiRadioEntityLeafOP = { getWiFiRadioEntity, setWiFiRadioEntity};

struct CWMP_PRMT tWiFiRadioEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"Status",		eCWMP_tSTRING,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
//{"Alias",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"Name", 		eCWMP_tSTRING,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
//{"LastChange",	eCWMP_tUINT,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
{"LowerLayers",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"Upstream",	eCWMP_tBOOLEAN,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
{"MaxBitRate",	eCWMP_tUINT,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
{"SupportedFrequencyBands",	eCWMP_tSTRING,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
{"OperatingFrequencyBand",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"SupportedStandards",	eCWMP_tSTRING,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
{"OperatingStandards",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"RegulatoryDomain",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"PossibleChannels", 	eCWMP_tSTRING,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
{"ChannelsInUse", 	eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tWiFiRadioEntityLeafOP},
{"Channel",				eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"AutoChannelSupported",	eCWMP_tBOOLEAN,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
{"AutoChannelEnable",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
//AutoChannelRefreshPeriod
{"TransmitPowerSupported",	eCWMP_tSTRING,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
{"TransmitPower",			eCWMP_tINT,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"OperatingChannelBandwidth",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"ExtensionChannel",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"GuardInterval", 		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"MCS",					eCWMP_tINT,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
{"IEEE80211hSupported",	eCWMP_tBOOLEAN,	CWMP_READ,	&tWiFiRadioEntityLeafOP},
{"IEEE80211hEnabled",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tWiFiRadioEntityLeafOP},
};

enum eWiFiRadioEntityLeaf
{
	eRadioEnable,
	eRadioStatus,
//	eRadioAlias,
	eRadioName,
//	eRadioLastChange,
	eRadioLowerLayers,
	eRadioUpstream,
	eRadioMaxBitRate,
	eRadioSupportedFrequencyBands,
	eRadioOperatingFrequencyBand,
	eRadioSupportedStandards,
	eRadioOperatingStandards,
	eRadioRegulatoryDomain,
	eRadioPossibleChannels,
	eRadioChannelsInUse,
	eRadioChannel,
	eRadioAutoChannelSupported,
	eRadioAutoChannelEnable,
	eRadioTransmitPowerSupported,
	eRadioTransmitPower,
	eRadioOperatingChannelBandwidth,
	eRadioExtensionChannel,
	eRadioGuardInterval,
	eRadioMCS,
	eRadioIEEE80211hSupported,
	eRadioIEEE80211hEnabled,
};

struct CWMP_LEAF tWiFiRadioEntityLeaf[] =
{
{ &tWiFiRadioEntityLeafInfo[eRadioEnable]  },
{ &tWiFiRadioEntityLeafInfo[eRadioStatus]  },
//{ &tWiFiRadioEntityLeafInfo[eRadioAlias]  },
{ &tWiFiRadioEntityLeafInfo[eRadioName]  },
//{ &tWiFiRadioEntityLeafInfo[eRadioLastChange]  },
{ &tWiFiRadioEntityLeafInfo[eRadioLowerLayers]  },
{ &tWiFiRadioEntityLeafInfo[eRadioUpstream]	},
{ &tWiFiRadioEntityLeafInfo[eRadioMaxBitRate]  },
{ &tWiFiRadioEntityLeafInfo[eRadioSupportedFrequencyBands]  },
{ &tWiFiRadioEntityLeafInfo[eRadioOperatingFrequencyBand]  },
{ &tWiFiRadioEntityLeafInfo[eRadioSupportedStandards]  },
{ &tWiFiRadioEntityLeafInfo[eRadioOperatingStandards]  },
{ &tWiFiRadioEntityLeafInfo[eRadioRegulatoryDomain]  },
{ &tWiFiRadioEntityLeafInfo[eRadioPossibleChannels]  },
{ &tWiFiRadioEntityLeafInfo[eRadioChannelsInUse]  },
{ &tWiFiRadioEntityLeafInfo[eRadioChannel]  },
{ &tWiFiRadioEntityLeafInfo[eRadioAutoChannelSupported]  },
{ &tWiFiRadioEntityLeafInfo[eRadioAutoChannelEnable]  },
{ &tWiFiRadioEntityLeafInfo[eRadioTransmitPowerSupported]  },
{ &tWiFiRadioEntityLeafInfo[eRadioTransmitPower]  },
{ &tWiFiRadioEntityLeafInfo[eRadioOperatingChannelBandwidth]  },
{ &tWiFiRadioEntityLeafInfo[eRadioExtensionChannel]  },
{ &tWiFiRadioEntityLeafInfo[eRadioGuardInterval]  },
{ &tWiFiRadioEntityLeafInfo[eRadioMCS]  },
{ &tWiFiRadioEntityLeafInfo[eRadioIEEE80211hSupported]  },
{ &tWiFiRadioEntityLeafInfo[eRadioIEEE80211hEnabled]  },
{ NULL	}
};

struct CWMP_PRMT tWiFiRadioEntityObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Stats",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eWiFiRadioEntityObject
{
	eRadioStats,
};

struct CWMP_NODE tWiFiRadioEntityObject[] =
{
/*info,  				leaf,			next)*/
{&tWiFiRadioEntityObjectInfo[eRadioStats],	tWiFiRadioStatsLeaf, NULL},
{NULL,					NULL,			NULL}
};

/****** Device.WiFi.Radio ****************************************************/
struct CWMP_PRMT tWiFiRadioObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"1",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	NULL},
};

enum eWiFiRadioObject
{
	eWiFiRadio1,
};

struct CWMP_NODE tWiFiRadioObject[] =
{
/*info, 						leaf,				next)*/
{&tWiFiRadioObjectInfo[eWiFiRadio1], tWiFiRadioEntityLeaf, tWiFiRadioEntityObject},
{NULL, NULL, NULL}
};

/***** Utility Functions *****************************************************/

int get_radio_stats(unsigned int radionum, struct net_device_stats *nds)
{
	struct net_device_stats tmp;
	int i;

	if(radionum != 1 || nds == NULL)
		return -1;

	for(i = 0 ; i < WLAN_IF_NUM ; i++)
	{
		if(get_net_device_stats(wlan_name[i], &tmp) < 0)
			continue;
		nds->tx_bytes += tmp.tx_bytes;
		nds->rx_bytes += tmp.rx_bytes;
		nds->tx_packets += tmp.tx_packets;
		nds->rx_packets += tmp.rx_packets;
		nds->tx_errors+= tmp.tx_errors;
		nds->rx_errors += tmp.rx_errors;
		nds->tx_dropped += tmp.tx_dropped;
		nds->rx_dropped += tmp.rx_dropped;
	}
	return 0;
}

/***** Operations ************************************************************/
int getWiFiRadioEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char dot11n = 0;
	unsigned char vChar;
	unsigned int radionum = 0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	radionum = getInstNum(name, "Radio");
	if(radionum != 1)
		return ERR_9005;
	
	mib_get( MIB_WLAN_BAND, (void *)&vChar);
	if(vChar & BAND_11N)
		dot11n = 1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( MIB_WLAN_DISABLED, (void *)&vChar);

		*data = booldup((vChar == 0));
	}
	else if( strcmp( lastname, "Status" )==0 )
	{
		int flags=0;
		if( getInFlags("wlan0", &flags)==1 )
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
		*data = strdup("wlan0");
	}
/*	else if( strcmp( lastname, "LastChange" )==0 )
	{
		*data = uintdup(1);
	}*/
	else if( strcmp( lastname, "LowerLayers" )==0 )
	{
		*data = strdup("");
	}
	else if( strcmp( lastname, "Upstream" )==0 )
	{
		*data = booldup(0);
	}
	else if( strcmp( lastname, "MaxBitRate" )==0 )
	{
		mib_get( MIB_WLAN_BAND, (void *)&vChar);

		if( vChar==BAND_11B ) //2.4 GHz (B)
			*data = uintdup( 11 ); 
		else if( vChar==BAND_11G )//2.4 GHz (G)
			*data = uintdup( 54 );
		else if( vChar==BAND_11BG )//2.4 GHz (B+G)
			*data = uintdup( 54 );
		else if( vChar==BAND_11A )
			*data = uintdup( 54 );
		else if( vChar==BAND_11N )
			*data = uintdup( 144 );
		else if( vChar==BAND_5G_11AN )
			*data = uintdup( 144 );
		else if( vChar==(BAND_11B|BAND_11G|BAND_11N) )
			*data = uintdup( 144 );
		else /* 0 */
			*data = uintdup( 0 );//return ERR_9002;
	}
	else if( strcmp( lastname, "SupportedFrequencyBands" )==0 )
	{
#if defined(CONFIG_RTL_92D_SUPPORT)
			*data = strdup("2.4GHz,5GHz");
#else
			*data = strdup("2.4GHz");
#endif
	}
	else if( strcmp( lastname, "OperatingFrequencyBand" )==0 )
	{
#if defined(CONFIG_RTL_92D_SUPPORT)
	if ( mib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&vChar) == 0)
		return ERR_9002;

	if(vChar == PHYBAND_2G)
		*data = strdup("2.4GHz");
	else if(vChar == PHYBAND_5G)
		*data = strdup("5GHz");
#else
	*data = strdup("2.4GHz");
#endif
	}
	else if( strcmp( lastname, "SupportedStandards" )==0 )
	{
#if defined(CONFIG_RTL_92D_SUPPORT)
		*data = strdup("a,b,g,n");
#else
		*data = strdup("b,g,n");
#endif
	}
	else if( strcmp( lastname, "OperatingStandards" )==0 )
	{
		unsigned char vChar;
		char buf[64] = {0};

		mib_get( MIB_WLAN_BAND, (void *)&vChar);

		if(vChar & BAND_11B)
			strcat(buf, "b,");
		if(vChar & BAND_11G)
			strcat(buf, "g,");
		if(vChar & BAND_11A)
			strcat(buf, "a,");
		if(vChar & BAND_11N)
			strcat(buf, "n,");

		//remove last ','
		if(strlen(buf) > 0)
			buf[strlen(buf) - 1] = '\0';

		*data = strdup(buf);
	}
	//TODO
	else if( strcmp( lastname, "RegulatoryDomain" )==0 )
	{
		*data = strdup("00 ");	//Unknown Country
	}
	else if( strcmp( lastname, "PossibleChannels" )==0 )
	{
		unsigned char dm;
		mib_get( MIB_HW_REG_DOMAIN, (void *)&dm);
		switch(dm)
		{
		case 1: //1-11
		case 2:
			*data = strdup("1-11");
			break;
		case 3: //1-13
			*data = strdup("1-13");
			break;
		case 4: //10-11
			*data = strdup("10-11");
			break;
		case 5: //10-13
			*data = strdup("10-13");
			break;
		case 6: //1-14
			*data = strdup("1-14");
			break;
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
			*data = strdup("");
			break;
		default:
			return ERR_9002;
		}
	}
	else if( strcmp( lastname, "ChannelsInUse" )==0 )
	{
		bss_info info = {0};
		char buf[8] = {0};

		if(getWlBssInfo("wlan0", &info) < 0)
			return ERR_9002;

		sprintf( buf, "%u", (unsigned int)info.channel );
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "Channel" )==0 )
	{
		bss_info info = {0};
		char buf[8] = {0};

		if(getWlBssInfo("wlan0", &info) < 0)
			return ERR_9002;

		*data = uintdup(info.channel);
	}
	else if( strcmp( lastname, "AutoChannelSupported" )==0 )
	{
		*data = booldup(1);
	}
	else if( strcmp( lastname, "AutoChannelEnable" )==0 )
	{
		mib_get( MIB_WLAN_AUTO_CHAN_ENABLED, (void *)&vChar);
		*data = booldup(vChar);
	}
	else if( strcmp( lastname, "TransmitPowerSupported" )==0 )
	{
		//15mW, 30mW, 60mW
		*data = strdup("25,50,100");
	}
	else if( strcmp( lastname, "TransmitPower" )==0 )
	{
		mib_get( MIB_TX_POWER, (void *)&vChar);
		switch(vChar)
		{
		case 0:
			*data = intdup(25);
			break;
		case 1:
			*data = intdup(50);
			break;
		case 2:
			*data = intdup(100);
			break;
		}
	}
	else if( strcmp( lastname, "OperatingChannelBandwidth" )==0 )
	{
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)	//11n only
		if(dot11n)
		{
			mib_get(MIB_WLAN_CHANNEL_WIDTH, &vChar);
			if(vChar)
				*data = strdup("40MHz");
			else
				*data = strdup("20MHz");
		}
		else
#endif
			*data = strdup("20MHz");
	}
	else if( strcmp( lastname, "ExtensionChannel" )==0 )
	{
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)	//11n only
		mib_get(MIB_WLAN_AUTO_CHAN_ENABLED, &vChar);
		if(vChar == 0)
		{
			mib_get(MIB_WLAN_CONTROL_BAND, &vChar);
			if(vChar == 0)
				*data = strdup("AboveControlChannel");
			else
				*data = strdup("BelowControlChannel");
		}
		else
#endif
			*data = strdup("Auto");
	}
	else if( strcmp( lastname, "GuardInterval" )==0 )
	{
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)	//11n only
		if(dot11n)
		{
			mib_get(MIB_WLAN_SHORTGI_ENABLED, &vChar);
			if(vChar)
				*data = strdup("400nsec");
			else
				*data = strdup("800nsec");
		}
		else
#endif
			*data = strdup("800nsec");
	}
	else if( strcmp( lastname, "MCS" )==0 )
	{
		unsigned char auto_rate;
		
		mib_get( MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&auto_rate);
		if(auto_rate && dot11n)
		{
			int mcs;
			unsigned int fix_rate;

			mib_get( MIB_WLAN_FIX_RATE, (void *)&fix_rate);

			mcs = log2(vChar)-12;
			if (mcs >=0 && mcs <= 15)
				*data = intdup( mcs );
			else
				*data = intdup( -1 );
		}
		else	//auto rate or 802.11n is not enabled
			*data =  intdup(-1);
	}
	else if( strcmp( lastname, "IEEE80211hSupported" )==0 )
	{
		*data = booldup(0);
	}
	else if( strcmp( lastname, "IEEE80211hEnabled" )==0 )
	{
		*data = booldup(0);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setWiFiRadioEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (entity==NULL)) return -1;

	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;
		unsigned vChar;
		
		if( i==NULL ) return ERR_9007;

		vChar = (*i==0)?1:0;

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
		return ERR_9007;
	}
	else if( strcmp( lastname, "OperatingFrequencyBand" )==0 )
	{
		char *feq_band = data;
		unsigned char vChar;
		if(feq_band == NULL)
			return ERR_9007;

#if defined(CONFIG_RTL_92D_SUPPORT)
		if(strcmp(feq_band, "2.4GHz") == 0)
			vChar = PHYBAND_2G;
		else if(strcmp(feq_band, "5GHz") == 0)
			vChar = PHYBAND_5G;
		else
			return ERR_9007;

		mib_set(MIB_WLAN_DISABLED, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else	//CONFIG_RTL_92D_SUPPORT
	if(strcmp(feq_band, "2.4GHz") != 0)
		return ERR_9007;
	else
		return 0;
#endif
	}
	else if( strcmp( lastname, "OperatingStandards" )==0 )
	{
		char *str = data;
		char *token = NULL;
		unsigned char vChar = 0;

		if(str == NULL)
			return ERR_9007;

		// use strtok because there may be "ac" in future.
		token = strtok(str, ",");
		while(token)
		{
			switch(token[0])
			{
			case 'a':
				vChar |= BAND_11A;
				break;
			case 'b':
				vChar |= BAND_11B;
				break;
			case 'g':
				vChar |= BAND_11G;
				break;
			case 'n':
				vChar |= BAND_11N;
				break;
			default:
				return ERR_9007;
			}
			token = strtok(NULL, ",");
		}
		if(vChar == 0)
			return ERR_9007;

		mib_set( MIB_WLAN_BAND, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "Channel" )==0 )
	{
		int *i = data;
		unsigned char vChar = 0;
		unsigned char dm;
		unsigned char ch;
		unsigned char auto_ch;
		int valid = 0;

		if(i == NULL)
			return ERR_9007;

		mib_get( MIB_WLAN_AUTO_CHAN_ENABLED, (void *)&auto_ch);

		if(auto_ch)	//AutoChannelEnable is true
			return ERR_9001;
		
		mib_get( MIB_HW_REG_DOMAIN, (void *)&dm);
		switch(dm)
		{
		case 1: //1-11
		case 2:
			if( (*i>=1) && (*i<=11) ) valid = 1;
			break;
		case 3: //1-13
			if( (*i>=1) && (*i<=13) ) valid = 1;
			break;
		case 4: //10-11
			if( (*i>=10) && (*i<=11) ) valid = 1;
			break;
		case 5: //10-13
			if( (*i>=10) && (*i<=11) ) valid = 1;
			break;
		case 6: //1-14
			if( (*i>=1) && (*i<=14) ) valid = 1;
			break;
		}
		if( valid==0 )
			return ERR_9007;

		vChar = *i;
		mib_set( MIB_WLAN_CHAN_NUM, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "AutoChannelEnable" )==0 )
	{
		int *i = data;
		unsigned char vChar;
		
		if( i==NULL ) return ERR_9007;

		if(*i == 1)
			vChar = 1;
		else if(*i == 0)
			vChar = 0;
		else
			return ERR_9007;

		mib_set(MIB_WLAN_AUTO_CHAN_ENABLED, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "OperatingChannelBandwidth" )==0 )
	{
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)
		char *str = data;
		unsigned char vChar;
		
		if( str == NULL ) return ERR_9007;

		if(strcmp(str, "40MHz") == 0)
			vChar = 1;
		else if(strcmp(str, "20MHz") == 0)
			vChar = 0;
		else
			return ERR_9007;

		mib_set(MIB_WLAN_CHANNEL_WIDTH, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else	//defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)
	return ERR_9001;
#endif
	}
	else if( strcmp( lastname, "ExtensionChannel" )==0 )
	{
		char *str = data;
		unsigned vChar;
		
		if( str == NULL ) return ERR_9007;

		mib_get(MIB_WLAN_AUTO_CHAN_ENABLED, &vChar);

		if(vChar)
			return ERR_9001;

		if(strcmp(str, "AboveControlChannel") == 0)
			vChar = 0;
		else if(strcmp(str, "BelowControlChannel") == 0)
			vChar = 1;
		else if(strcmp(str, "Auto") == 0)
			return ERR_9001;
		else
			return ERR_9007;

		mib_set(MIB_WLAN_CONTROL_BAND, &vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "GuardInterval" )==0 )
	{
		char *str = data;
		unsigned vChar;
		
		if( str == NULL ) return ERR_9007;

		if(strcmp(str, "400nsec") == 0)
			vChar = 1;
		else if(strcmp(str, "800nsec") == 0)
			vChar = 0;
		else if(strcmp(str, "Auto") == 0)
			vChar = ERR_9001;
		else
			return ERR_9007;

		mib_set(MIB_WLAN_SHORTGI_ENABLED, &vChar);

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "MCS" )==0 )
	{
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) || defined(CONFIG_RTL8192CD)
		int mcs;
		int *i = data;
		unsigned vChar;
		
		if( i == NULL ) return ERR_9007;

		if (*i == -1) // Auto 
		{
			vChar = 1;
			mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);

			mcs = 0;
			mib_set(MIB_WLAN_FIX_RATE, (void *)&mcs);
		}
		else if (*i >= 0 && *i <= 15)
		{
			vChar = 0;
			mib_set(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&vChar);

			mcs = 1 << (*i+12);
			mib_set(MIB_WLAN_FIX_RATE, (void *)&mcs);
		}
		else
			return ERR_9007;

		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
	return ERR_9001;
#endif
	}
	else if( strcmp( lastname, "TransmitPower" )==0 )
	{
		unsigned char vChar;
		int *i = data;

		if(i == NULL)
			return ERR_9007;

		switch(*i)
		{
		case 25:
			vChar = 0;
			break;
		case 50:
			vChar = 1;
			break;
		case -1:
		case 100:
			vChar = 2;
			break;
		default:
			return ERR_9007;
		}

		mib_set( MIB_TX_POWER, (void *)&vChar);
		apply_add( CWMP_PRI_N, apply_WLAN, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "IEEE80211hEnabled" )==0 )
	{
		return ERR_9001;
	}
	else if( strcmp( lastname, "RegulatoryDomain" )==0 )
	{
		return ERR_9001;
	}
	else
	{
		return ERR_9005;
	}
	
	return 0;
}

int getWiFiRadioStats(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	struct net_device_stats nds = {0};
	unsigned int radionum;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	radionum = getInstNum(name, "Radio");
	if(radionum != 1) return ERR_9005;

	get_radio_stats(radionum, &nds);

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
	else if( strcmp( lastname, "DiscardPacketsSent" )==0 )
	{
		*data = uintdup(nds.tx_dropped);
	}
	else if( strcmp( lastname, "DiscardPacketsReceived" )==0 )
	{
		*data = uintdup(nds.rx_dropped);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}
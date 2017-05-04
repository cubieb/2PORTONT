#include "prmt_deviceinfo.h"
#include <sys/sysinfo.h>
#ifdef _PRMT_X_CT_EXT_ENABLE_
#include "prmt_ctcom.h"
#include "prmt_ctcom_ping.h"
#endif	//_PRMT_X_CT_EXT_ENABLE_
#ifdef _PRMT_X_CT_COM_ALG_
#ifdef EMBED
#include <linux/autoconf.h>
#else
#include "../../../uClibc/include/linux/autoconf.h"
#endif
#endif // of _PRMT_X_CT_COM_ALG_

#include <rtk/options.h>
#include <rtk/mib.h>
#include <config/autoconf.h>

#ifdef CONFIG_MIDDLEWARE
#define MANUFACTURER_STR	"REALTEK"
#else
#define MANUFACTURER_STR	DEF_MANUFACTURER_STR //"REALTEK SEMICONDUCTOR CORP."
#endif
#define MANUFACTUREROUI_STR	DEF_MANUFACTUREROUI_STR //"00E04C"
#define SPECVERSION_STR		"1.0"
#ifdef CONFIG_RTL8686
#define HWVERSION_STR           "RTL960x"
#else 
#define HWVERSION_STR		"8671x"
#endif


struct CWMP_OP tVendorCfgEntityLeafOP = { getVendorCfgEntity, NULL };
struct CWMP_PRMT tVendorCfgEntityLeafInfo[] =
{
/*(name,	type,		flag,		op)*/
{"Name",	eCWMP_tSTRING,	CWMP_READ,	&tVendorCfgEntityLeafOP},
{"Version",	eCWMP_tSTRING,	CWMP_READ,	&tVendorCfgEntityLeafOP},
{"Date",	eCWMP_tDATETIME,CWMP_READ,	&tVendorCfgEntityLeafOP},
{"Description",	eCWMP_tSTRING,	CWMP_READ,	&tVendorCfgEntityLeafOP},
};
enum eVendorCfgEntityLeaf
{
	eVCName,
	eVCVersion,
	eVCDate,
	eDescription
};
struct CWMP_LEAF tVendorCfgEntityLeaf[] =
{
{ &tVendorCfgEntityLeafInfo[eVCName] },
{ &tVendorCfgEntityLeafInfo[eVCVersion] },
{ &tVendorCfgEntityLeafInfo[eVCDate] },
{ &tVendorCfgEntityLeafInfo[eDescription] },
{ NULL }
};


struct CWMP_PRMT tVendorConfigObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eVendorConfigObject
{
	eVC1
};
struct CWMP_NODE tVendorConfigObject[] =
{
/*info,  				leaf,			next)*/
{&tVendorConfigObjectInfo[eVC1],	tVendorCfgEntityLeaf, 	NULL},
{NULL, 					NULL, 			NULL}
};


struct CWMP_OP tDeviceInfoLeafOP = { getDeviceInfo, setDeviceInfo };
struct CWMP_PRMT tDeviceInfoLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Manufacturer",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ManufacturerOUI",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ModelName",			eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"Description",			eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ProductClass",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"SerialNumber",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"HardwareVersion",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"SoftwareVersion",		eCWMP_tSTRING,	CWMP_READ|CWMP_FORCE_ACT,		&tDeviceInfoLeafOP},
#ifdef CONFIG_DEV_xDSL
{"ModemFirmwareVersion",	eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
#endif
{"EnabledOptions",		eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
/*AdditionalHardwareVersion*/
/*AdditionalSoftwareVersion*/
{"SpecVersion",			eCWMP_tSTRING,	CWMP_READ,		&tDeviceInfoLeafOP},
{"ProvisioningCode",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ|CWMP_FORCE_ACT,	&tDeviceInfoLeafOP},
{"UpTime",			eCWMP_tUINT,	CWMP_READ|CWMP_DENY_ACT,&tDeviceInfoLeafOP},
{"FirstUseDate",		eCWMP_tDATETIME,CWMP_READ,		&tDeviceInfoLeafOP},
{"DeviceLog",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,&tDeviceInfoLeafOP},
{"VendorConfigFileNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tDeviceInfoLeafOP},
};

struct CWMP_LEAF tDeviceInfoLeaf[] =
{
{ &tDeviceInfoLeafInfo[eDIManufacturer] },
{ &tDeviceInfoLeafInfo[eDIManufacturerOUI] },
{ &tDeviceInfoLeafInfo[eDIModelName] },
{ &tDeviceInfoLeafInfo[eDIDescription] },
{ &tDeviceInfoLeafInfo[eDIProductClass] },
{ &tDeviceInfoLeafInfo[eDISerialNumber] },
{ &tDeviceInfoLeafInfo[eDIHardwareVersion] },
{ &tDeviceInfoLeafInfo[eDISoftwareVersion] },
#ifdef CONFIG_DEV_xDSL
{ &tDeviceInfoLeafInfo[eDIModemFirmwareVersion] },
#endif
{ &tDeviceInfoLeafInfo[eDIEnabledOptions] },
{ &tDeviceInfoLeafInfo[eDISpecVersion] },
{ &tDeviceInfoLeafInfo[eDIProvisioningCode] },
{ &tDeviceInfoLeafInfo[eDIUpTime] },
{ &tDeviceInfoLeafInfo[eDIFirstUseDate] },
{ &tDeviceInfoLeafInfo[eDIDeviceLog] },
{ &tDeviceInfoLeafInfo[eDIVendorConfigFileNumberOfEntries] },
#ifdef _PRMT_X_CT_COM_DEVINFO_
{ &tCTDeviceInfoLeafInfo[eDIX_CTCOM_InterfaceVersion] },
{ &tCTDeviceInfoLeafInfo[eDIX_CTCOM_CardInterVersion] },
{ &tCTDeviceInfoLeafInfo[eDIX_CTCOM_DeviceType] },
{ &tCTDeviceInfoLeafInfo[eDIX_CTCOM_Card] },
{ &tCTDeviceInfoLeafInfo[eDIX_CTCOM_UPLink] },
{ &tCTDeviceInfoLeafInfo[eDIX_CTCOM_Capability] },
{ &tCTDeviceInfoLeafInfo[eDIX_CTCOM_IPForwardModeEnabled] },
#endif
{ NULL	}
};
struct CWMP_PRMT tDeviceInfoObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"VendorConfigFile",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#ifdef _PRMT_X_CT_COM_ACCOUNT_
{"X_CT-COM_TeleComAccount",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_ALG_
{"X_CT-COM_ALGAbility",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_RECON_
{"X_CT-COM_ReConnect",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_PORTALMNT_
{"X_CT-COM_PortalManagement",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_SRVMNG_
{"X_CT-COM_ServiceManage",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_SYSLOG_
{"X_CT-COM_Syslog",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef CONFIG_MIDDLEWARE
{"X_CT-COM_MiddlewareMgt",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_UPNP_
{"X_CT-COM_UPNP",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
/*ping_zhang:20100128 START:add for e8b tr069 Alarm*/
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
{"X_CT-COM_Alarm",  		eCWMP_tOBJECT,	CWMP_READ,		NULL},
{"X_CT-COM_Monitor",		eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
/*ping_zhang:20100128 END*/
#ifdef _PRMT_X_CT_COM_IPv6_
{"X_CT-COM_IPProtocolVersion",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_DLNA_
{"X_CT-COM_DLNA",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_PING_
{"X_CT-COM_Ping",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
#ifdef _PRMT_USBRESTORE
{"X_CT-COM_Restore",	eCWMP_tOBJECT,	CWMP_READ,		NULL},
#endif
};
enum eDeviceInfoObject
{
	eDIVendorConfigFile,
#ifdef _PRMT_X_CT_COM_ACCOUNT_
	eDIX_CTCOM_TeleComAccount,
#endif
#ifdef _PRMT_X_CT_COM_ALG_
	eDIX_CTCOM_ALGAbility,
#endif
#ifdef _PRMT_X_CT_COM_RECON_
	eDIX_CTCOM_ReConnect,
#endif
#ifdef _PRMT_X_CT_COM_PORTALMNT_
	eDIX_CTCOM_PortalManagement,
#endif
#ifdef _PRMT_X_CT_COM_SRVMNG_
	eDIX_CTCOM_ServiceManage,
#endif
#ifdef _PRMT_X_CT_COM_SYSLOG_
	eDIX_CTCOM_Syslog,
#endif
#ifdef CONFIG_MIDDLEWARE
	eDIX_CTCOM_MiddlewareMgt,
#endif
#ifdef _PRMT_X_CT_COM_UPNP_
	eDIX_CTCOM_UPNP,
#endif
/*ping_zhang:20100128 START:add for e8b tr069 Alarm*/
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
	eDIX_CTCOM_Alarm,
	eDIX_CTCOM_Monitor,
#endif
/*ping_zhang:20100128 END*/
#ifdef _PRMT_X_CT_COM_IPv6_
	eDIX_CTCOM_IPProtocolVersion,
#endif
#ifdef _PRMT_X_CT_COM_DLNA_
	eDIX_CTCOM_DLNA,
#endif
#ifdef _PRMT_X_CT_COM_PING_
	eDIX_CTCOM_Ping,
#endif
#ifdef _PRMT_USBRESTORE
	eDIX_CTCOM_Restore,
#endif
};
struct CWMP_NODE tDeviceInfoObject[] =
{
/*info,  						leaf,		next)*/
{ &tDeviceInfoObjectInfo[eDIVendorConfigFile],		NULL,		tVendorConfigObject },
#ifdef _PRMT_X_CT_COM_ACCOUNT_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_TeleComAccount],	tCTAccountLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_ALG_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_ALGAbility],	tXCTCOMALGLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_RECON_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_ReConnect],		tCT_ReConLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_PORTALMNT_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_PortalManagement],	tCT_PortalMNTLeaf,NULL },
#endif
#ifdef _PRMT_X_CT_COM_SRVMNG_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_ServiceManage],	tCTServiceLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_SYSLOG_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Syslog],		tCT_SyslogLeaf,	NULL },
#endif
#ifdef CONFIG_MIDDLEWARE
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_MiddlewareMgt],	tCT_MiddlewareMgtLeaf,	NULL },
#endif
#if defined(CONFIG_USER_UPNPD) || defined(CONFIG_USER_MINIUPNPD)
#ifdef _PRMT_X_CT_COM_UPNP_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_UPNP],		tCT_UPnPLeaf,	NULL },
#endif
#endif
/*ping_zhang:20100128 START:add for e8b tr069 Alarm*/
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Alarm],		tCT_AlarmLeaf,	tCT_AlarmObject },
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Monitor],	tCT_MonitorLeaf,tCT_MonitorObject },
#endif
/*ping_zhang:20100128 END*/
#ifdef _PRMT_X_CT_COM_IPv6_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_IPProtocolVersion],		tCT_IPProtocolVersionLeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_DLNA_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_DLNA],		tCT_DLNALeaf,	NULL },
#endif
#ifdef _PRMT_X_CT_COM_PING_
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Ping],		tCT_PingLeaf,	tCT_PingObject },
#endif
#ifdef _PRMT_USBRESTORE
{ &tDeviceInfoObjectInfo[eDIX_CTCOM_Restore],		tCT_RestoreLeaf,	NULL},
#endif
{ NULL,							NULL,		NULL }
};


#ifdef _PRMT_DEVICECONFIG_
struct CWMP_OP tDeviceConfigLeafOP = { getDeviceConfig, setDeviceConfig };
struct CWMP_PRMT tDeviceConfigLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PersistentData",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDeviceConfigLeafOP},
{"ConfigFile",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tDeviceConfigLeafOP}
};
enum eDeviceConfigLeaf
{
	eDCPersistentData,
	eDCConfigFile
};
struct CWMP_LEAF tDeviceConfigLeaf[] =
{
{ &tDeviceConfigLeafInfo[eDCPersistentData] },
{ &tDeviceConfigLeafInfo[eDCConfigFile] },
{ NULL	}
};
#endif //_PRMT_DEVICECONFIG_

int getVendorCfgEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Name" )==0 )
	{
		*data = strdup( "config.xml" );
	}else if( strcmp( lastname, "Version" )==0 )
	{
		getSYS2Str( SYS_FWVERSION, buf );
		*data = strdup( buf ); /*use the software version as config version*/
	}else if( strcmp( lastname, "Date" )==0 )
	{
		*data = timedup( 0 );//unknown time
	}else if( strcmp( lastname, "Description" )==0 )
	{
		*data = strdup( "" );
	}else{
		return ERR_9005;
	}

	return 0;
}


int getDeviceInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Manufacturer" )==0 )
	{
		mib_get(MIB_HW_CWMP_MANUFACTURER, (void *) buf);
			*data = strdup(buf);
	}else if( strcmp( lastname, "ManufacturerOUI" )==0 )
	{
#ifdef E8B_GET_OUI
#ifdef CONFIG_E8B
                if(mib_get(RTK_DEVID_OUI, (void *)buf) != 0 && buf[0])
		{
                        *data = strdup(buf);
			printf("Get OUI(%s) from flash \n", buf);
		}
                else
		{
#endif
		getOUIfromMAC(buf);
		*data = strdup(buf);
#ifdef CONFIG_E8B
		}
#endif
#else
		getOUIfromMAC(buf);
		*data = strdup(buf);
#endif
	}else if( strcmp( lastname, "ModelName" )==0 )
	{
		mib_get( MIB_SNMP_SYS_NAME, (void *)buf);
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "Description" )==0 )
	{
		mib_get( MIB_SNMP_SYS_DESCR, (void *)buf);
		*data = strdup( buf );
	}
	else if( strcmp( lastname, "ProductClass" )==0 )
	{
		mib_get( MIB_HW_CWMP_PRODUCTCLASS, (void *)buf);
                        *data = strdup(buf);
	}else if( strcmp( lastname, "SerialNumber" )==0 )
	{
		mib_get( MIB_HW_SERIAL_NUMBER, (void *)buf);
		*data = strdup( buf );
	}else if( strcmp( lastname, "HardwareVersion" )==0 )
	{
		mib_get(MIB_HW_HWVER, (void *) buf);
            *data = strdup(buf);
		
	}else if( strcmp( lastname, "SoftwareVersion" )==0 )
	{
#ifdef CONFIG_E8B
        if(mib_get(RTK_DEVINFO_SWVER, (void *)buf) != 0 && buf[0])
		{
            *data = strdup(buf);
			printf("Get Software Version(%s) from flash \n" , buf);
		}
        else
		{
			*data = strdup(SOFTWARE_VERSION_STR);
		}
#else
		getSYS2Str( SYS_FWVERSION, buf );
		*data = strdup( buf );
#endif
#ifdef CONFIG_DEV_xDSL
	}else if( strcmp( lastname, "ModemFirmwareVersion" )==0 )
	{
		getAdslInfo( ADSL_GET_VERSION, buf, 256 );
		*data = strdup( buf );
#endif
	}else if( strcmp( lastname, "EnabledOptions" )==0 )
	{
		*data = strdup( "" );
	}else if( strcmp( lastname, "SpecVersion" )==0 )
	{
#ifdef CONFIG_E8B
                if(mib_get(RTK_DEVINFO_SPECVER, (void *)buf) != 0 && buf[0])
		{
                        *data = strdup(buf);
			printf("Get Spec. Version (%s) from flash \n" , buf);
		}
                else
#endif
		*data = strdup( SPECVERSION_STR );
	}else if( strcmp( lastname, "ProvisioningCode" )==0 )
	{
		mib_get( CWMP_PROVISIONINGCODE, (void *)buf);
		*data = strdup( buf );
	}else if( strcmp( lastname, "UpTime" )==0 )
	{
		struct sysinfo info;
		sysinfo(&info);
		*data = uintdup( info.uptime );
	}else if( strcmp( lastname, "FirstUseDate" )==0 )
	{
		*data = timedup( 0 );
	}else if( strcmp( lastname, "DeviceLog" )==0 )
	{
#if defined(CONFIG_USER_BUSYBOX_SYSLOGD) || defined(CONFIG_USER_RTK_SYSLOG)
		*type = eCWMP_tFILE; /*special case*/
#ifdef CONFIG_E8B
		*data = strdup( "/var/config/syslogd.txt" );
#else
		*data = strdup( "/var/log/messages" );
#endif
#else
		*data = strdup( "" );
#endif //#ifdef CONFIG_USER_BUSYBOX_SYSLOGD
	}else if( strcmp( lastname, "VendorConfigFileNumberOfEntries" )==0 )
	{
		*data = uintdup( 1 );
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setDeviceInfo(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char 	*buf=data;
	int  	len=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "ProvisioningCode" )==0 )
	{
		if( buf ) len = strlen( buf );
		if( len ==0 )
			mib_set( CWMP_PROVISIONINGCODE, (void *)"");
		else if( len < 64 )
			mib_set( CWMP_PROVISIONINGCODE, (void *)buf);
		else
			return ERR_9007;

		return 0;
	}else
		return ERR_9005;
	return 0;
}

#ifdef _PRMT_DEVICECONFIG_
int getDeviceConfig(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	char persis_data[256];

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "PersistentData" )==0 )
	{
		if(!mib_get(CWMP_PERSISTENT_DATA,  (void *)persis_data))
			return ERR_9005;

		*data = strdup(persis_data);
	}else if( strcmp( lastname, "ConfigFile" )==0 )
	{
#ifdef CONFIG_USE_XML
		if( va_cmd("/bin/saveconfig",0,1) )
		{
			fprintf( stderr, "<%s:%d>exec /bin/saveconfig error!\n", __FUNCTION__, __LINE__  );
			return ERR_9002;
		}
		// rename
		rename("/tmp/config.xml", CONFIG_FILE_NAME);
		*type = eCWMP_tFILE; /*special case*/
		*data = strdup(CONFIG_FILE_NAME);
#elif defined(CONFIG_USER_XMLCONFIG)
		*type = eCWMP_tFILE; /*special case*/
		*data = strdup("/var/config/lastgood.xml");
#else
		*data = strdup("*** RAW Data ***");
#endif
	}else{
		return ERR_9005;
	}

	return 0;
}

int setDeviceConfig(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "PersistentData" )==0 )
	{
		if(strlen(data) >= 256)
			return ERR_9007;

		if(!mib_set(CWMP_PERSISTENT_DATA,  (void *)data))
		return ERR_9001;
	}
	else if( strcmp( lastname, "ConfigFile" )==0 )
	{
		char *buf=data;
		int  buflen=0;
		FILE *fp=NULL;

		if( buf==NULL ) return ERR_9007;
		buflen = strlen(buf);
		if( buflen==0 ) return ERR_9007;

#ifdef CONFIG_USE_XML
/*star:20100312 START add to check the value*/
		if(strncmp(buf, "<Config_Information_File", 24))
		{
			return ERR_9007;
		}
/*star:20100312 END*/
#elif defined(CONFIG_USER_XMLCONFIG)
		if(strncmp(buf, "<Config Name=", 13))
		{
			return ERR_9007;
		}
#endif

		fp=fopen( CONFIG_XMLFILE, "w" );
		if(fp)
		{
			int retlen=0, id;

			fprintf( stderr, "New config length:%d\n", buflen );
#if 0
			retlen = fwrite( buf, 1, buflen, fp );
#else
			/*somehow, the '\n'is gone between lines,
			  but loadconfig needs it to parse config.
			  the better way is to rewirte the parsing code of loadconfig*/
			for( id=0;id<buflen;id++ )
			{
				if( (id>0) && (buf[id-1]=='>') && (buf[id]=='<') )
					if( fputc( '\n',fp )==EOF )
						break;

				if(fputc( buf[id],fp )==EOF)
					break;

				retlen=id+1;
			}
			fputc( '\n',fp );
#endif
			fclose(fp);

			if( retlen!=buflen )
				return 9002;

#ifdef CONFIG_USE_XML
			if( va_cmd("/bin/loadconfig",0,1) )
			{
				fprintf( stderr, "<%s:%d>exec /bin/loadconfig error!\n", __FUNCTION__, __LINE__ );
				return ERR_9002;
			}
#elif defined(CONFIG_USER_XMLCONFIG)
			if (va_cmd("/bin/sh",3,1, "/etc/scripts/config_xmlconfig.sh", "-l", CONFIG_XMLFILE) != 0)
			{
				fprintf(stderr, "Load config error\n");
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
				/* This event will be cleared if loadconfig success. */
				set_ctcom_alarm(CTCOM_ALARM_CONF_INVALID);
				syslog(LOG_ERR, "Invalid configuration file.");
#endif
				return ERR_9007;
			}
#else	/* raw config data does not support write function */
			return ERR_9001;
#endif
		}else
			return ERR_9002;
		return 1;
	}else{
		return ERR_9005;
	}

	return 0;

}
#endif //_PRMT_DEVICECONFIG_



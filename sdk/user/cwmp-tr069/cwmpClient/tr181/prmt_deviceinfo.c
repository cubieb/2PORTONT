#include "prmt_deviceinfo.h"
#include "../prmt_deviceinfo.h"
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <rtk/options.h>
#include <config/autoconf.h>

extern struct CWMP_OP tDeviceInfoLeafOP;
extern struct CWMP_PRMT tDeviceInfoLeafInfo[];

/****** Device.DeviceInfo.SupportedDataModel.{i} ****************************************/
struct CWMP_OP tSupportedDMEntityOP = { NULL, objSupportedDM};
struct CWMP_OP tSupportedDMEntityLeafOP = { getSupportedDMEntity, NULL };

struct CWMP_PRMT tSupportedDMEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"URL",			eCWMP_tSTRING,	CWMP_READ,	&tSupportedDMEntityLeafOP},
{"URN",			eCWMP_tSTRING,	CWMP_READ,	&tSupportedDMEntityLeafOP},
{"Features",	eCWMP_tSTRING,	CWMP_READ,	&tSupportedDMEntityLeafOP},
};

enum eSupportedDMEntityLeaf
{
	eURL,
	eURN,
	eFeatures,
};

struct CWMP_LEAF tSupportedDMEntityLeaf[] =
{
{ &tSupportedDMEntityLeafInfo[eURL]  },
{ &tSupportedDMEntityLeafInfo[eURN]  },
{ &tSupportedDMEntityLeafInfo[eFeatures]	},
{ NULL	}
};

struct DTInstanceInfo
{
	char *filename;
};

struct DTInstanceInfo supportedDMList[] =
{
{"rtk-xdsl-device-base.xml"},
#ifdef CONFIG_DEV_xDSL
#ifdef CONFIG_VDSL
{"rtk-xdsl-device-xdsl.xml"},
#else  //CONFIG_VDSL
{"rtk-xdsl-device-adsl.xml"},
#endif //CONFIG_VDSL
#endif //CONFIG_DEV_xDSL
#ifdef CONFIG_PTMWAN
{"rtk-xdsl-device-ptm.xml"},
#endif
#ifdef WLAN_SUPPORT
{"rtk-xdsl-device-wlan.xml"},
#endif
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
{"rtk-xdsl-device-traceroute.xml"},
#endif
};

/****** Device.DeviceInfo.SupportedDataModel. *********************************************/
struct CWMP_PRMT tSupportedDMObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL},
};

enum eSupportedDMObject
{
	eSupportedDM0,
};

struct CWMP_LINKNODE tSupportedDMObject[] =
{
/*info, 						leaf,				next,				sibling,	instnum)*/
{&tSupportedDMObjectInfo[eSupportedDM0], tSupportedDMEntityLeaf, NULL, NULL, 0},
};

/***** DeviceInfo.VendorLogFiles.{i} *****************************************/
struct CWMP_OP tVendorLogEntityLeafOP = { getVendorLogEntity, NULL };
struct CWMP_PRMT tVendorLogEntityLeafInfo[] =
{
/*(name,	type,		flag,		op)*/
{"Name",	eCWMP_tSTRING,	CWMP_READ,	&tVendorLogEntityLeafOP},
{"MaximumSize",	eCWMP_tUINT,	CWMP_READ,	&tVendorLogEntityLeafOP},
{"Persistent",	eCWMP_tBOOLEAN,	CWMP_READ,	&tVendorLogEntityLeafOP},
};
enum eVendorCfgEntityLeaf
{
	eVLName,
	eVLMaximumSize,
	eVLPersistent,
};
struct CWMP_LEAF tVendorLogEntityLeaf[] =
{
{ &tVendorLogEntityLeafInfo[eVLName] },
{ &tVendorLogEntityLeafInfo[eVLMaximumSize] },
{ &tVendorLogEntityLeafInfo[eVLPersistent] },
{ NULL }
};

struct CWMP_PRMT tVendorLogObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"1",		eCWMP_tOBJECT,	CWMP_READ,	NULL}
};
enum eVendorLogObject
{
	eVL1
};
struct CWMP_NODE tVendorLogObject[] =
{
/*info,  				leaf,			next)*/
{&tVendorLogObjectInfo[eVL1],	tVendorLogEntityLeaf, 	NULL},
{NULL, 					NULL, 			NULL}
};


/***** DeviceInfo ************************************************************/
struct CWMP_OP tTR181DeviceInfoLeafOP = { getTR181DeviceInfo, NULL };
struct CWMP_PRMT tTR181DeviceInfoLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"SupportedDataModelNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tTR181DeviceInfoLeafOP},
{"VendorLogFileNumberOfEntries",		eCWMP_tUINT,	CWMP_READ,	&tTR181DeviceInfoLeafOP}
};

enum eTR181DeviceInfoLeaf
{
	eSupportedDataModelNumberOfEntries,
	eDIVendorLogFileNumberOfEntries,
};

struct CWMP_LEAF tTR181DeviceInfoLeaf[] =
{
{ &tDeviceInfoLeafInfo[eDIManufacturer] },
{ &tDeviceInfoLeafInfo[eDIManufacturerOUI] },
{ &tDeviceInfoLeafInfo[eDIModelName] },
{ &tDeviceInfoLeafInfo[eDIDescription] },
{ &tDeviceInfoLeafInfo[eDIProductClass] },
{ &tDeviceInfoLeafInfo[eDISerialNumber] },
{ &tDeviceInfoLeafInfo[eDIHardwareVersion] },
{ &tDeviceInfoLeafInfo[eDISoftwareVersion] },
{ &tDeviceInfoLeafInfo[eDIProvisioningCode] },
{ &tDeviceInfoLeafInfo[eDIUpTime] },
{ &tTR181DeviceInfoLeafInfo[eSupportedDataModelNumberOfEntries] },
{ &tTR181DeviceInfoLeafInfo[eDIVendorLogFileNumberOfEntries] },
{ NULL	}
};

struct CWMP_PRMT tTR181DeviceInfoObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"SupportedDataModel",	eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, &tSupportedDMEntityOP	},
{"VendorLogFile",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eDeviceInfoObject
{
	eDISupportedDataModel,
	eDIVendorLogFile,
};

struct CWMP_NODE tTR181DeviceInfoObject[] =
{
/*info,  						leaf,		next)*/
{ &tTR181DeviceInfoObjectInfo[eDISupportedDataModel],	NULL,		NULL},
{ &tTR181DeviceInfoObjectInfo[eDIVendorLogFile],		NULL,		tVendorLogObject },
{NULL,		NULL,	NULL}
};

/****** Utilities ************************************************************/
static inline int getSupportedDMInstNum()
{
	int cnt = 1;	//base profiles	
#ifdef CONFIG_DEV_xDSL
	cnt++;
#endif
#ifdef CONFIG_PTMWAN
	cnt++;
#endif
#ifdef _SUPPORT_TRACEROUTE_PROFILE_
	cnt++;
#endif
#ifdef WLAN_SUPPORT
	cnt++;
#endif

	return cnt;
}

/****** Operations ***********************************************************/
int getVendorLogEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Name" )==0 )
	{
#if defined(CONFIG_USER_BUSYBOX_SYSLOGD) || defined(CONFIG_USER_RTK_SYSLOG)
#ifdef CONFIG_E8B
		*data = strdup( "/var/config/syslogd.txt" );
#else
		*data = strdup( "/var/log/messages" );
#endif
#else
		*data = strdup( "" );
#endif //#ifdef CONFIG_USER_BUSYBOX_SYSLOGD
	}else if( strcmp( lastname, "MaximumSize" )==0 )
	{
		*data = uintdup(0); //don't know the size
	}else if( strcmp( lastname, "Persistent" )==0 )
	{
		*data = booldup(0);
	}else{
		return ERR_9005;
	}

	return 0;
}

int getTR181DeviceInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char buf[256]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "SupportedDataModelNumberOfEntries" )==0 )
	{
		*data = uintdup(getSupportedDMInstNum());
	}
	else if( strcmp( lastname, "VendorLogFileNumberOfEntries" )==0 )
	{
		*data = uintdup( 1 );
	}else{
		return ERR_9005;
	}

	return 0;
}

int getSupportedDMEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int inst_num = 0;
	struct DTInstanceInfo *dt_info = NULL;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	inst_num = getInstNum(name, "SupportedDataModel");
	if(inst_num < 1 || inst_num > getSupportedDMInstNum())
		return ERR_9005;

	dt_info = &supportedDMList[inst_num - 1];

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "URL" )==0 )
	{
		char url_fmt[] = "http://localhost:%d%s%s";
		char url[1024] = {0};
		unsigned port = 0;

		mib_get(CWMP_CONREQ_PORT, &port);
		if(port == 0)
			port = 7547;

		snprintf(url, 1024, url_fmt, port, DT_DOC_PATH, dt_info->filename);
		*data = strdup(url);
	}
	else if( strcmp( lastname, "URN" )==0 )
	{
		*data = strdup("urn:rtk-xdsl:device-1-0-0");
	}
	else if( strcmp( lastname, "Features" )==0 )
	{
		*data = strdup("");
	}
	else{
		return ERR_9005;
	}

	return 0;
}


int objSupportedDM(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);
	
	switch( type )
	{
	case eCWMP_tINITOBJ:
		{
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;
		int inst_num;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		inst_num = getSupportedDMInstNum();

		if( create_Object(c, tSupportedDMObject, sizeof(tSupportedDMObject), inst_num, 1 ) < 0 )
			return -1;

		add_objectNum(name, inst_num);

		return 0;
		}
	case eCWMP_tADDOBJ:
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
		return 0;	//Never changed since initialized.
	}
	
	return -1;
}


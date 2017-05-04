#include "prmt_mngmtserver.h"

/*star:20091228 START add for store parameterkey*/
//char gParameterKey[32+1];
/*star:20091228 END*/
char gConnectionRequestURL[256+1];

#ifdef _TR_111_PRMT_
int gDeviceNumber=0;

/****ManageDevEntity*******************************************************************************************/
struct CWMP_OP tManageDevEntityLeafOP = { getManageDevEntity, NULL };
struct CWMP_PRMT tManageDevEntityLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"ManufacturerOUI",	eCWMP_tSTRING,	CWMP_READ,	&tManageDevEntityLeafOP},
{"SerialNumber",	eCWMP_tSTRING,	CWMP_READ,	&tManageDevEntityLeafOP},
{"ProductClass",	eCWMP_tSTRING,	CWMP_READ,	&tManageDevEntityLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Host",	eCWMP_tSTRING,	CWMP_READ,	&tManageDevEntityLeafOP},
#endif
/*ping_zhang:20081217 END*/
};
enum eManageDevEntityLeaf
{
	eMDManufacturerOUI,
	eMDSerialNumber,
	eMDProductClass,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eMDHost
#endif
/*ping_zhang:20081217 END*/
};
struct CWMP_LEAF tManageDevEntityLeaf[] =
{
{ &tManageDevEntityLeafInfo[eMDManufacturerOUI] },
{ &tManageDevEntityLeafInfo[eMDSerialNumber] },
{ &tManageDevEntityLeafInfo[eMDProductClass] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tManageDevEntityLeafInfo[eMDHost] },
#endif
/*ping_zhang:20081217 END*/
{ NULL }
};

/****ManageableDevice*******************************************************************************************/
struct CWMP_PRMT tManageableDeviceOjbectInfo[] =
{
/*(name,	type,		flag,			op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
enum eManageableDeviceOjbect
{
	eMD0
};
struct CWMP_LINKNODE tManageableDeviceObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tManageableDeviceOjbectInfo[eMD0],tManageDevEntityLeaf,	NULL,		NULL,			0},
};
/***********************************************************************************************/
#endif

/*******ManagementServer****************************************************************************************/
struct CWMP_OP tManagementServerLeafOP = { getMngmntServer,setMngmntServer };
struct CWMP_PRMT tManagementServerLeafInfo[] =
{
/*(name,				type,		flag,			op)*/
{"EnableCWMP",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"URL",					eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"Username",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"Password",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"PeriodicInformEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"PeriodicInformInterval",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"PeriodicInformTime",			eCWMP_tDATETIME,CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"ParameterKey",			eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,&tManagementServerLeafOP},
{"ConnectionRequestURL",		eCWMP_tSTRING,	CWMP_READ,		&tManagementServerLeafOP},
{"ConnectionRequestUsername",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"ConnectionRequestPassword",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"UpgradesManaged",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tManagementServerLeafOP},
{"KickURL",				eCWMP_tSTRING,	CWMP_READ,		&tManagementServerLeafOP},
{"DownloadProgressURL",			eCWMP_tSTRING,	CWMP_READ,		&tManagementServerLeafOP},
#ifdef _TR_111_PRMT_
{"ManageableDeviceNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tManagementServerLeafOP},
/*ManageableDeviceNotificationLimit*/
#endif
};
enum eManagementServerLeaf
{
	eMSEnableCWMP,
	eMSURL,
	eMSUsername,
	eMSPassword,
	eMSPeriodicInformEnable,
	eMSPeriodicInformInterval,
	eMSPeriodicInformTime,
	eMSParameterKey,
	eMSConnectionRequestURL,
	eMSConnectionRequestUsername,
	eMSConnectionRequestPassword,
	eMSUpgradesManaged,
	eMSKickURL,
	eMSDownloadProgressURL,
#ifdef _TR_111_PRMT_
	eMSManageableDeviceNumberOfEntries,
#endif
};
struct CWMP_LEAF tManagementServerLeaf[] =
{
{ &tManagementServerLeafInfo[eMSEnableCWMP] },
{ &tManagementServerLeafInfo[eMSURL] },
{ &tManagementServerLeafInfo[eMSUsername] },
{ &tManagementServerLeafInfo[eMSPassword] },
{ &tManagementServerLeafInfo[eMSPeriodicInformEnable] },
{ &tManagementServerLeafInfo[eMSPeriodicInformInterval] },
{ &tManagementServerLeafInfo[eMSPeriodicInformTime] },
{ &tManagementServerLeafInfo[eMSParameterKey] },
{ &tManagementServerLeafInfo[eMSConnectionRequestURL] },
{ &tManagementServerLeafInfo[eMSConnectionRequestUsername] },
{ &tManagementServerLeafInfo[eMSConnectionRequestPassword] },
{ &tManagementServerLeafInfo[eMSUpgradesManaged] },
{ &tManagementServerLeafInfo[eMSKickURL] },
{ &tManagementServerLeafInfo[eMSDownloadProgressURL] },
#ifdef _TR_111_PRMT_
{ &tManagementServerLeafInfo[eMSManageableDeviceNumberOfEntries] },
#endif
{ NULL	}
};
#ifdef _TR_111_PRMT_
struct CWMP_OP tMS_ManageableDevice_OP = { NULL, objManageDevice };
struct CWMP_PRMT tManagementServerObjectInfo[] =
{
/*(name,				type,		flag,			op)*/
{"ManageableDevice",			eCWMP_tOBJECT,	CWMP_READ,		&tMS_ManageableDevice_OP}
};
enum eManagementServerObject
{
	eMSManageableDevice
};
struct CWMP_NODE tManagementServerObject[] =
{
/*info,  						leaf,		next)*/
{&tManagementServerObjectInfo[eMSManageableDevice],	NULL,		NULL},
{NULL,							NULL,		NULL}
};
#endif

#define CONFIG_SET(key, val) if ( mib_set(key, val)==0)  return ERR_9002
#define CONFIG_GET(key, ret) if ( mib_get(key, ret)==0)  return ERR_9002
#define CHECK_PARAM_STR(str, min, max)  do { \
	int tmp; \
	if (!str) return ERR_9007; \
	tmp=strlen(str); \
	if ((tmp < min) || (tmp > max)) return ERR_9007; \
}	while (0)

#define CHECK_PARAM_NUM(input, min, max) if ( (input < min) || (input > max) ) return ERR_9007;

enum {
	EN_ENABLE_CWMP = 0,
	EN_URL,
	EN_USERNAME,
	EN_PASSWORD,
	EN_PERIODIC_INFORM_ENABLE,
	EN_PERIODIC_INTERVAL,
	EN_PERIODIC_TIME,
	EN_PARAMETER_KEY,
	EN_CONNREQ_URL,
	EN_CONNREQ_USERNAME,
	EN_CONNREQ_PASSWORD,
	EN_UPGRADE_MANAGED,
	EN_KICKURL,
	EN_DOWNLOADURL
#ifdef _TR_111_PRMT_
	,EN_MANAGEABLEDEVICENUMBER
#endif
};

void MgmtSrvSetParamKey(const char *key) {
/*star:20091228 START add for store parameterkey*/
	unsigned char gParameterKey[32+1];
	gParameterKey[0]='\0';
	if (key)
	{
		strncpy(gParameterKey, key, sizeof(gParameterKey) -1);
		gParameterKey[sizeof(gParameterKey) -1]=0;
	}
	mib_set(CWMP_PARAMETERKEY,gParameterKey);
/*star:20091228 END*/
}

int getMngmntServer(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	unsigned char buf[256+1]={0};
	unsigned char ch=0;
	unsigned int  in=0;
#ifdef CONFIG_MIDDLEWARE
	unsigned char vChar;
#endif

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	switch(getIndexOf(tManagementServerLeaf, entity->info->name))
	{
	case EN_ENABLE_CWMP:
		CONFIG_GET(CWMP_FLAG2, &ch);
		*data = (ch & CWMP_FLAG2_CWMP_DISABLE) ? booldup(0) : booldup(1);
		break;
	case EN_URL: //URL
		CONFIG_GET(CWMP_ACS_URL, buf);
		*data = strdup( buf );
		break;
	case EN_USERNAME: //Username
//	#ifdef DEBUG
		CONFIG_GET(CWMP_ACS_USERNAME, buf);
		*data = strdup(buf);
		break;
	case EN_PASSWORD: // Password
	#ifdef DEBUG
		CONFIG_GET(CWMP_ACS_PASSWORD, buf);
		*data = strdup(buf);
	#else
	#ifdef CONFIG_MIDDLEWARE
		mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
		if(vChar == 0){
			CONFIG_GET(CWMP_ACS_PASSWORD, buf);
			*data = strdup(buf);
		}else
	#endif	//end of CONFIG_MIDDLEWARE
		*data = strdup("");
	#endif
		break;
	case EN_PERIODIC_INFORM_ENABLE: // #PeriodicInformEnable
		CONFIG_GET(CWMP_INFORM_ENABLE, &ch);
		*data = booldup(ch);
		break;
	case EN_PERIODIC_INTERVAL: // PeriodicInformInterval
		CONFIG_GET(CWMP_INFORM_INTERVAL, &in);
		*data = uintdup(in);
		break;
	case EN_PERIODIC_TIME: // PeriodicInformTime
		CONFIG_GET(CWMP_INFORM_TIME, &in);
		*data = timedup(in);
		break;
	case EN_PARAMETER_KEY: // ParameterKey
		{
/*star:20091228 START add for store parameterkey*/
			unsigned char gParameterKey[32+1];
			CONFIG_GET(CWMP_PARAMETERKEY,gParameterKey);
/*star:20091228 END*/
			*data = strdup(gParameterKey);
			break;
		}
	case EN_CONNREQ_URL: // ConnectionRequestURL
		if (MgmtSrvGetConReqURL(buf, 256))
			*data = strdup(buf);
		else
			*data = strdup("");
		break;
	case EN_CONNREQ_USERNAME: // ConnectionRequestUsername
		CONFIG_GET(CWMP_CONREQ_USERNAME, buf);
		*data = strdup(buf);
		break;
	case EN_CONNREQ_PASSWORD: // ConnectionRequestPassword
	#ifdef DEBUG
		CONFIG_GET(CWMP_CONREQ_PASSWORD, buf);
		*data = strdup(buf);
	#else
	#ifdef CONFIG_MIDDLEWARE
		mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
		if(vChar == 0){
			CONFIG_GET(CWMP_CONREQ_PASSWORD, buf);
			*data = strdup(buf);
		}else
	#endif
		*data = strdup("");
	#endif
		break;
	case EN_UPGRADE_MANAGED: // UpgradesManaged
		CONFIG_GET(CWMP_ACS_UPGRADESMANAGED, &ch);
		*data = booldup(ch);
		break;
	case EN_KICKURL:
		CONFIG_GET(CWMP_ACS_KICKURL, buf);
		*data = strdup(buf);
		break;
	case EN_DOWNLOADURL:
		CONFIG_GET(CWMP_ACS_DOWNLOADURL, buf);
		*data = strdup(buf);
		break;
#ifdef _TR_111_PRMT_
	case EN_MANAGEABLEDEVICENUMBER:
	{
/*star:20100127 START add to update gDeviceNumber, used for notification list check*/
		FILE *fp;
		int count=0;
		fp=fopen( TR111_DEVICEFILE, "r" );

		while( fp && fgets( buf,160,fp ) )
		{
			char *p;

			p = strtok( buf, " \n\r" );
			if( p && atoi(p)>0 )
			{
				count++;
			}
		}
		if(fp) fclose(fp);
		gDeviceNumber = count;
/*star:20100127 END*/
		*data = uintdup(gDeviceNumber);
	}
		break;
#endif
	default:
		return ERR_9005;

	}

	return 0;

}

int setMngmntServer(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char 	*buf=data;
	int  	len=0;
	unsigned int *pNum;
	unsigned char byte;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	switch(getIndexOf(tManagementServerLeaf, entity->info->name))
	{
	case EN_ENABLE_CWMP:
		pNum = (unsigned int *)data;

		CHECK_PARAM_NUM(*pNum, 0, 1);
		CONFIG_GET(CWMP_FLAG2, &byte);
		if(*pNum == 0)
			byte |= CWMP_FLAG2_CWMP_DISABLE;
		else
			byte &= ~CWMP_FLAG2_CWMP_DISABLE;

		CONFIG_SET(CWMP_FLAG2, &byte);
		break;
	case EN_URL: //URL
		CHECK_PARAM_STR(buf, 0, 256+1);
/*star:20100305 START add qos rule to set tr069 packets to the first priority queue*/
		storeOldACS();
/*star:20100305 END*/
		CONFIG_SET(CWMP_ACS_URL, buf);
		break;
	case EN_USERNAME: //Username
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(CWMP_ACS_USERNAME, buf);
		break;
	case EN_PASSWORD: // Password
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(CWMP_ACS_PASSWORD, buf);
		break;
	case EN_PERIODIC_INFORM_ENABLE: // #PeriodicInformEnable
		pNum = (unsigned int *)data;

		CHECK_PARAM_NUM(*pNum, 0, 1);
		byte = (*pNum == 0) ? 0 : 1;
		CONFIG_SET(CWMP_INFORM_ENABLE, &byte);
/*star:20100112 START move to port_update_userdata()*/
		//cwmpMgmtSrvInformInterval();
/*star:20100112 END*/
		break;
	case EN_PERIODIC_INTERVAL: // PeriodicInformInterval
		pNum = (unsigned int *)data;

		if (*pNum < 1) return ERR_9007;
		CONFIG_SET(CWMP_INFORM_INTERVAL, pNum);
/*star:20100112 START move to port_update_userdata()*/
		//cwmpMgmtSrvInformInterval();
/*star:20100112 END*/
		break;
	case EN_PERIODIC_TIME: // PeriodicInformTime
		pNum = (unsigned int *)buf;
		CONFIG_SET(CWMP_INFORM_TIME, buf);
/*star:20100112 START move to port_update_userdata()*/
		//cwmpMgmtSrvInformInterval();
/*star:20100112 END*/
		break;
	case EN_CONNREQ_USERNAME: // ConnectionRequestUsername
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(CWMP_CONREQ_USERNAME, buf);
		break;
	case EN_CONNREQ_PASSWORD: // ConnectionRequestPassword
		CHECK_PARAM_STR(buf, 0, 256+1);
		CONFIG_SET(CWMP_CONREQ_PASSWORD, buf);
		break;
	case EN_UPGRADE_MANAGED: // UpgradesManaged
		pNum = (unsigned int *)data;

		CHECK_PARAM_NUM(*pNum, 0, 1);
		byte = (*pNum == 0) ? 0 : 1;
		CONFIG_SET(CWMP_ACS_UPGRADESMANAGED, &byte);
		break;
	default:
		return ERR_9005;

	}

	return 0;
}

#ifdef _TR_111_PRMT_
extern unsigned int getInstNum( char *name, char *objname );
int getManageDeviceInfo( unsigned int num, char *sOUI, char *sSN, char *sClass );
int getManageDevEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char		*lastname = entity->info->name;
	unsigned int	num=0;
	char		sOUI[7]="", sSN[65]="", sClass[65]="";

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	num = getInstNum( name, "ManageableDevice" );
	if(num==0) return ERR_9005;
	getManageDeviceInfo( num, sOUI, sSN, sClass );

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ManufacturerOUI" )==0 )
	{
		*data = strdup( sOUI );
	}else if( strcmp( lastname, "SerialNumber" )==0 )
	{
		*data = strdup( sSN );
	}else if( strcmp( lastname, "ProductClass" )==0 )
	{
		*data = strdup( sClass );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Host" )==0 )
	{
		extern unsigned int gTotalHost;
		extern int updateHosts();
		char hostStrHead[] = "InternetGatewayDevice.LANDevice.1.Hosts.Host.";
		char hostStr[1000];
		int i,len=0;

		updateHosts();

		if(gTotalHost>0)
		{
			for(i=0; i<gTotalHost; i++)
			{
				if( len+strlen(hostStrHead)+16 > sizeof(hostStr) )
					break;

				if(i!=0)
					len += sprintf(hostStr+len,",");
				len += sprintf(hostStr+len,"%s%d",hostStrHead,i+1);
			}
			*data = strdup(hostStr);
		}
		else
			*data = strdup( "" );
#endif
/*ping_zhang:20081217 END*/
	}else{
		return ERR_9005;
	}

	return 0;
}

int objManageDevice(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;
	FILE *fp;
	char buf[160];

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);

	switch( type )
	{
	case eCWMP_tINITOBJ:
	     {
		int MaxInstNum=0,count=0;
		struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

		if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		fp=fopen( TR111_DEVICEFILE, "r" );
		if(fp==NULL) return 0;

		while( fgets( buf,160,fp ) )
		{
			char *p;

			p = strtok( buf, " \n\r" );
			if( p && atoi(p)>0 )
			{
				if( MaxInstNum < atoi(p) )
					MaxInstNum = atoi(p);

				if( create_Object( c, tManageableDeviceObject, sizeof(tManageableDeviceObject), 1, atoi(p) ) < 0 )
					break;
				count++;
				//c = & (*c)->sibling;
			}
		}
		fclose(fp);
		gDeviceNumber = count;
		add_objectNum( name, MaxInstNum );
		return 0;
	     }
	case eCWMP_tUPDATEOBJ:
	     {
	     	int count=0;
	     	struct CWMP_LINKNODE *old_table;

	     	old_table = (struct CWMP_LINKNODE *)entity->next;
	     	entity->next = NULL;

		fp=fopen( TR111_DEVICEFILE, "r" );
	     	while( fp && fgets( buf,160,fp ) )
	     	{
	     		struct CWMP_LINKNODE *remove_entity=NULL;
			char *p;

			p = strtok( buf, " \n\r" );
			if( p && atoi(p)>0 )
			{
				remove_entity = remove_SiblingEntity( &old_table, atoi(p) );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}else{
					unsigned int MaxInstNum;
					MaxInstNum = atoi(p);
					add_Object( name, (struct CWMP_LINKNODE **)&entity->next,  tManageableDeviceObject, sizeof(tManageableDeviceObject), &MaxInstNum );
				}
				count++;
			}
	     	}

	     	if(fp) fclose(fp);

	     	gDeviceNumber = count;
	     	if( old_table )
	     		destroy_ParameterTable( (struct CWMP_NODE *)old_table );
	     	return 0;
	     }
	}

	return -1;
}



int getManageDeviceInfo( unsigned int num, char *sOUI, char *sSN, char *sClass )
{
	FILE *fp;
	int ret=-1;

	if( num<=0 || sOUI==NULL || sSN==NULL || sClass==NULL ) return ret;

	sOUI[0]=sSN[0]=sClass[0]=0;

	fp=fopen( TR111_DEVICEFILE, "r" );
	if(fp)
	{
		char buf[160], *p, *n;

		while( fgets( buf,160,fp ) )
		{
			p = strtok( buf, " \n\r" );
			n = strtok( NULL, "\n\r" );
			if( p && (atoi(p)==num) && n )
			{
				char *s1=NULL, *s2=NULL, *s3=NULL;

				s1 = strtok( n, "?\n\r" );
				s2 = strtok( NULL, "?\n\r" );
				s3 = strtok( NULL, "?\n\r" );
				if( s1 && s2 && s3 )
				{
					strncpy( sOUI, s1, 6 );
					sOUI[6]=0;
					strncpy( sClass, s2, 64 );
					sClass[64]=0;
					strncpy( sSN, s3, 64 );
					sSN[64]=0;
					ret = 0;
				}else if( s1 && s2 )
				{
					strncpy( sOUI, s1, 6 );
					sOUI[6]=0;
					sClass[0]=0;
					strncpy( sSN, s2, 64 );
					sSN[64]=0;
					ret = 0;
				}//else error
				break;
			}
		}

		fclose(fp);
	}
	return ret;
}
#endif

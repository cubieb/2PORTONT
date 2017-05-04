#include "prmt_services.h"

#ifdef CONFIG_USER_TR104
#include "prmt_voiceservice.h"
#endif
#ifdef _PRMT_SERVICES_


#ifdef _PRMT_X_CT_COM_IPTV_
struct CWMP_OP tCT_IPTVLeafOP = { getCT_IPTV, setCT_IPTV };
struct CWMP_PRMT tCT_IPTVLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"IGMPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_IPTVLeafOP},
{"ProxyEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_IPTVLeafOP},
{"SnoopingEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_IPTVLeafOP},
{"STBNumber",			eCWMP_tUINT,	CWMP_READ,	&tCT_IPTVLeafOP},
};
enum eCT_IPTVLeaf
{
	eCTIGMPEnable,
	eCTProxyEnable,
	eCTSnoopingEnable,
	eCTSTBNumber
};
struct CWMP_LEAF tCT_IPTVLeaf[]=
{
{ &tCT_IPTVLeafInfo[eCTIGMPEnable] },
{ &tCT_IPTVLeafInfo[eCTProxyEnable] },
{ &tCT_IPTVLeafInfo[eCTSnoopingEnable] },
{ &tCT_IPTVLeafInfo[eCTSTBNumber] },
{ NULL }
};
#endif

#ifdef _PRMT_X_CT_COM_MWBAND_
struct CWMP_OP tCT_MWBANDLeafOP = { getCT_MWBAND, setCT_MWBAND };
struct CWMP_PRMT tCT_MWBANDLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Mode",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"TotalTerminalNumber",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"STBRestrictEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"STBNumber",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"CameraRestrictEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"CameraNumber",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"ComputerRestrictEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"ComputerNumber",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"PhoneRestrictEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP},
{"PhoneNumber",			eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCT_MWBANDLeafOP}
};
enum eCT_MWBANDLeaf
{
	eCTMMode,
	eCTMTotalTerminalNumber,
	eCTMSTBRestrictEnable,
	eCTMSTBNumber,
	eCTMCameraRestrictEnable,
	eCTMCameraNumber,
	eCTMComputerRestrictEnable,
	eCTMComputerNumber,
	eCTMPhoneRestrictEnable,
	eCTMPhoneNumber
};
struct CWMP_LEAF tCT_MWBANDLeaf[] =
{
{ &tCT_MWBANDLeafInfo[eCTMMode] },
{ &tCT_MWBANDLeafInfo[eCTMTotalTerminalNumber] },
{ &tCT_MWBANDLeafInfo[eCTMSTBRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMSTBNumber] },
{ &tCT_MWBANDLeafInfo[eCTMCameraRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMCameraNumber] },
{ &tCT_MWBANDLeafInfo[eCTMComputerRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMComputerNumber] },
{ &tCT_MWBANDLeafInfo[eCTMPhoneRestrictEnable] },
{ &tCT_MWBANDLeafInfo[eCTMPhoneNumber] },
{ NULL }
};
#endif

#ifdef CONFIG_USER_TR104
struct CWMP_OP tSRV_VoiceService_OP = { NULL, objVoiceService };
#endif

struct CWMP_PRMT tServicesObjectInfo[] =
{
/*(name,			type,		flag,		op)*/
#ifdef _PRMT_X_CT_COM_IPTV_
{"X_CT-COM_IPTV",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef _PRMT_X_CT_COM_MWBAND_
{"X_CT-COM_MWBAND",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
#endif
#ifdef CONFIG_USER_TR104
{"VoiceService",	eCWMP_tOBJECT,	CWMP_READ,	&tSRV_VoiceService_OP},
#endif
};
enum eServicesObject
{
#ifdef _PRMT_X_CT_COM_IPTV_
	eSX_CTCOM_IPTV,
#endif
#ifdef _PRMT_X_CT_COM_MWBAND_
	eSX_CTCOM_MWBAND,
#endif
#ifdef CONFIG_USER_TR104
    eSX_VoiceService,
#endif
	eSX_CTCOM_DUMMY,
};
struct CWMP_NODE tServicesObject[] =
{
/*info,  				leaf,			next)*/
#ifdef _PRMT_X_CT_COM_IPTV_
{&tServicesObjectInfo[eSX_CTCOM_IPTV],	tCT_IPTVLeaf,		NULL},
#endif
#ifdef _PRMT_X_CT_COM_MWBAND_
{&tServicesObjectInfo[eSX_CTCOM_MWBAND],tCT_MWBANDLeaf,		NULL},
#endif
#ifdef CONFIG_USER_TR104
{&tServicesObjectInfo[eSX_VoiceService],	NULL,		NULL},
#endif
{NULL,					NULL,			NULL}	
};
#ifdef _PRMT_X_CT_COM_USERINFO_
struct CWMP_OP tCT_UserInfoLeafOP = { getCT_UserInfo, setCT_UserInfo };
struct CWMP_PRMT tCT_UserInfoLeafInfo[] =
{
/*(name,			type,		flag,				op)*/
{"UserName",			eCWMP_tSTRING,	CWMP_WRITE | CWMP_READ,	&tCT_UserInfoLeafOP},
{"UserId",			eCWMP_tSTRING,	CWMP_WRITE | CWMP_READ,	&tCT_UserInfoLeafOP},
{"Status",			eCWMP_tUINT,		CWMP_WRITE | CWMP_READ,	&tCT_UserInfoLeafOP},
{"Limit",			eCWMP_tUINT,		CWMP_WRITE | CWMP_READ,	&tCT_UserInfoLeafOP},
{"Times",			eCWMP_tUINT,		CWMP_WRITE | CWMP_READ,	&tCT_UserInfoLeafOP},
#ifdef E8B_NEW_DIAGNOSE
{"Result",			eCWMP_tUINT,		CWMP_WRITE | CWMP_READ,	&tCT_UserInfoLeafOP},
{"ServiceNum",		eCWMP_tINT,		CWMP_WRITE | CWMP_READ, 	&tCT_UserInfoLeafOP},
{"ServiceName",		eCWMP_tSTRING,	CWMP_WRITE | CWMP_READ, 	&tCT_UserInfoLeafOP},
#endif
};
enum eCT_UserInfoLeaf
{
	eCT_Q_UserName,
	eCT_Q_UserId,
	eCT_Q_Status,
	eCT_Q_Limit,
	eCT_Q_Times,
#ifdef E8B_NEW_DIAGNOSE
	eCT_Q_Result,
	eCT_Q_ServiceNum,
	eCT_Q_ServiceName,
#endif
};
struct CWMP_LEAF tCT_UserInfoLeaf[] =
{
{ &tCT_UserInfoLeafInfo[eCT_Q_UserName] },
{ &tCT_UserInfoLeafInfo[eCT_Q_UserId] },
{ &tCT_UserInfoLeafInfo[eCT_Q_Status] },
{ &tCT_UserInfoLeafInfo[eCT_Q_Limit] },
{ &tCT_UserInfoLeafInfo[eCT_Q_Times] },
#ifdef E8B_NEW_DIAGNOSE
{ &tCT_UserInfoLeafInfo[eCT_Q_Result] },
{ &tCT_UserInfoLeafInfo[eCT_Q_ServiceNum] },
{ &tCT_UserInfoLeafInfo[eCT_Q_ServiceName] },
#endif
{ NULL },
};

int reset_reg_times_timer = -1;

int getCT_UserInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned char vChar;
	unsigned int vUint;
	unsigned int vInt;
	char buf[128];
	
	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "UserName") == 0) {
		mib_get(MIB_LOID, buf);
		*data = strdup(buf);
	} else if (strcmp(lastname, "UserId") == 0) {
		mib_get(MIB_LOID_PASSWD, buf);
		*data = strdup(buf);
	} else if (strcmp(lastname, "Status") == 0) {
		mib_get(CWMP_USERINFO_STATUS, &vUint);
		*data = uintdup(vUint);
	} else if (strcmp(lastname, "Limit") == 0) {
		mib_get(CWMP_USERINFO_LIMIT, &vUint);
		*data = uintdup(vUint);
	} else if (strcmp(lastname, "Times") == 0) {
		mib_get(CWMP_USERINFO_TIMES, &vUint);
		*data = uintdup(vUint);
	}
#ifdef E8B_NEW_DIAGNOSE
	else if (strcmp(lastname, "Result") == 0) {
		mib_get(CWMP_USERINFO_RESULT, &vUint);
		*data = uintdup(vUint);
	}
	else if (strcmp(lastname, "ServiceNum") == 0) {
		mib_get(CWMP_USERINFO_SERV_NUM, &vInt);
		*data = intdup(vInt);
	}
	else if (strcmp(lastname, "ServiceName") == 0) {
		mib_get(CWMP_USERINFO_SERV_NAME, buf);
		*data = strdup(buf);
	}
#endif
	else {
		return ERR_9005;
	}
	
	return 0;
}

int setCT_UserInfo(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	char *buf = data;
	unsigned char vChar = 0;
	
	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	
	if (strcmp(lastname, "UserName") == 0) {
		if (buf == NULL)
			return ERR_9007;
		if (strlen(buf) >= MAX_NAME_LEN)
			return ERR_9007;
		mib_set(MIB_LOID, buf);
		mib_set(MIB_LOID_OLD, buf);

		return 0;
	} else if (strcmp(lastname, "UserId") == 0) {
		if (buf == NULL)
			return ERR_9007;
		if (strlen(buf) >= MAX_NAME_LEN)
			return ERR_9007;
		mib_set(MIB_LOID_PASSWD, buf);
		mib_set(MIB_LOID_PASSWD_OLD, buf);

		return 0;
	} else if (strcmp(lastname, "Status") == 0) {
		int *i = data;

		unsigned int regStatus;
		unsigned int regLimit;
		unsigned int regTimes;
		unsigned char regInformStatus;

		if (i == NULL)
			return ERR_9007;

		/*xl_yue:20081225 get the inform status to avoid acs responses twice for only once informing*/
		mib_get(CWMP_REG_INFORM_STATUS, &regInformStatus);
		/*xl_yue:20081225 END*/
		if (regInformStatus != CWMP_REG_REQUESTED) {
			return 0;
		}	
		regInformStatus = CWMP_REG_RESPONSED;
		mib_set(CWMP_REG_INFORM_STATUS, &regInformStatus);

		mib_get(CWMP_USERINFO_TIMES, &regTimes);
		mib_get(CWMP_USERINFO_LIMIT, &regLimit);
		regStatus = *i;
		if (regStatus == 0) {
			regTimes = 0;
			/* 
			 * regStatus == 4 indicates timeout,
			 * so regTimes should not be increased
			 * according to the specification
			 */
		} else if (regStatus != 4) {
			regTimes++;
			if(regTimes >= regLimit)
				reset_reg_times_timer = 180;	//reset after 3 minutes
		}
		mib_set(CWMP_USERINFO_STATUS, &regStatus);
		mib_set(CWMP_USERINFO_TIMES, &regTimes);

		unlink(REBOOT_DELAY_FILE);

		return 0;
	} else if (strcmp(lastname, "Limit") == 0) {
		unsigned int *i = data;
		unsigned int regTimes;

		if (i == NULL)
			return ERR_9007;
		mib_set(CWMP_USERINFO_LIMIT, i);
		mib_get(CWMP_USERINFO_TIMES, &regTimes);

		if(regTimes >= *i && reset_reg_times_timer == -1)
			reset_reg_times_timer = 180;	//reset after 3 minutes

		return 0;
	} else if (strcmp(lastname, "Times") == 0) {
		unsigned int *i = data;
		unsigned int regLimit;

		if (i == NULL)
			return ERR_9007;
		mib_set(CWMP_USERINFO_TIMES, i);
		mib_get(CWMP_USERINFO_LIMIT, &regLimit);

		if(*i >= regLimit && reset_reg_times_timer == -1)
			reset_reg_times_timer = 180;	//reset after 3 minutes

		return 0;
	}
#ifdef E8B_NEW_DIAGNOSE
	else if (strcmp(lastname, "Result") == 0) {
		unsigned int *i = data;

		unsigned int vUint;
		FILE *fp;

		if (i == NULL) return ERR_9007;
		vUint = *i;

		if (vUint != NOW_SETTING && vUint != SET_SUCCESS && vUint != SET_FAULT)
			return ERR_9002;
		mib_set(CWMP_USERINFO_RESULT, &vUint);
		
		switch (vUint) {
		case NOW_SETTING:
			if (access(REMOTE_SETSAVE_FILE, R_OK) == 0) {
				unlink(REMOTE_SETSAVE_FILE);
#ifdef CONFIG_USER_FLATFSD_XXX
				va_cmd("/bin/flatfsd", 1, 1, "-s");
#endif
			}
			break;
		case SET_SUCCESS:
			fp = fopen(REBOOT_DELAY_FILE, "w");
			if (fp) {
				fprintf(fp, "%s", "1");
				fclose(fp);
			}
			break;
		case SET_FAULT:
			clearpvcfile();
			break;
		default:
			break;
		}

		return 0;
	}
	else if (strcmp(lastname, "ServiceNum") == 0) {
		int *i = data;

		mib_set(CWMP_USERINFO_SERV_NUM, i);
	}
	else if (strcmp(lastname, "ServiceName") == 0) {
		int inc = 0, num, num_done;
		unsigned char name_done[32];

		mib_set(CWMP_USERINFO_SERV_NAME, buf);

		/* increase the Service Num Done the amount that will be done */
		if (strstr(buf, "IPTV"))
			inc++;
		if (strstr(buf, "INTERNET"))
			inc++;
		if (strstr(buf, "VOIP"))
			inc++;
		mib_get(CWMP_USERINFO_SERV_NUM, &num);
		mib_get(CWMP_USERINFO_SERV_NUM_DONE, &num_done);
		num_done += inc;
		if (num_done > num)
			num_done = num;
		mib_set(CWMP_USERINFO_SERV_NUM_DONE, &num_done);

		/* concatenate the Service Name(s) that will be done into Service Name Done */
		mib_get(CWMP_USERINFO_SERV_NAME_DONE, name_done);
		strncat(name_done, buf, sizeof(name_done));
		strncat(name_done, " ", sizeof(name_done));
		mib_set(CWMP_USERINFO_SERV_NAME_DONE, name_done);
	}
#endif
	else {
		return ERR_9005;
	}

	return 0;
}

#endif

#ifdef _PRMT_X_CT_COM_IPTV_
int getCT_IPTV(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int  vUint=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "IGMPEnable" )==0 )
	{
		mib_get( CWMP_CT_IPTV_IGMPENABLE, (void *)&vChar);
		if(vChar==1)
			*data = booldup(1);
		else
			*data = booldup(0);
	}
	else if( strcmp( lastname, "ProxyEnable" )==0 )
	{
		mib_get(MIB_IGMP_PROXY, (void *)&vChar);

		*data = booldup((vChar == 1) ? 1 : 0);
	}
	else if( strcmp( lastname, "SnoopingEnable" )==0 )
	{
		mib_get(MIB_MPMODE, (void *)&vChar);

		*data = booldup((vChar & MP_IGMP_MASK) ? 1 : 0);
	}
	else if( strcmp( lastname, "STBNumber" )==0 )
	{
		mib_get( CWMP_CT_IPTV_STBNUMBER, (void *)&vUint);
		*data = uintdup( vUint );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_IPTV(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if(data == NULL) return ERR_9007;

	if( strcmp( lastname, "IGMPEnable" )==0 )
	{
		char mode;
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_IPTV_IGMPENABLE, (void *)&vChar);
	}
	else if( strcmp( lastname, "ProxyEnable" )==0 )
	{
		int *i = data;

		vChar = (*i == 1) ? 1 : 0;

		mib_set(MIB_IGMP_PROXY, &vChar);
		apply_add(CWMP_PRI_N, apply_IGMPProxy, CWMP_RESTART, 0, NULL, 0 );
	}
	else if( strcmp( lastname, "SnoopingEnable" )==0 )
	{
		int *i = data;
		unsigned char mode;
		unsigned char mask = MP_IGMP_MASK;

		mib_get(MIB_MPMODE, &mode);
		if(*i == 1)
			mode |= MP_IGMP_MASK;
		else
			mode &= ~MP_IGMP_MASK;

		mib_set(MIB_MPMODE, &mode);
		apply_add(CWMP_PRI_N, apply_IGMPSnooping, CWMP_RESTART, 0, NULL, 0 );
	}
	else if( strcmp( lastname, "STBNumber" )==0 )
	{
		unsigned int *i = data;
		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_IPTV_STBNUMBER, (void *)i);
	}
	else{
		return ERR_9005;
	}
	
	return 0;
}
#endif //_PRMT_X_CT_COM_IPTV_

#ifdef _PRMT_X_CT_COM_MWBAND_
int getCT_MWBAND(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	int  vUint=0;
	int	vInt=0;
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Mode" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_MODE, (void *)&vInt);
		*data = intdup( vInt );
	}else if( strcmp( lastname, "TotalTerminalNumber" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_NUMBER, (void *)&vUint);
		*data = intdup( vUint );
	}else if( strcmp( lastname, "STBRestrictEnable" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_STB_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "STBNumber" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_STB_NUM, (void *)&vUint);
		*data = intdup( vUint );
	}else if( strcmp( lastname, "CameraRestrictEnable" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_CMR_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "CameraNumber" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_CMR_NUM, (void *)&vUint);
		*data = intdup( vUint );
	}else if( strcmp( lastname, "ComputerRestrictEnable" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_PC_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "ComputerNumber" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_PC_NUM, (void *)&vUint);
		*data = intdup( vUint );
	}else if( strcmp( lastname, "PhoneRestrictEnable" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_PHN_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "PhoneNumber" )==0 )
	{
		mib_get( CWMP_CT_MWBAND_PHN_NUM, (void *)&vUint);
		*data = intdup( vUint );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setCT_MWBAND(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Mode" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		if( *i<0 || *i>2 ) return ERR_9007;
		mib_set(CWMP_CT_MWBAND_MODE, (void *)i);
	}else if( strcmp( lastname, "TotalTerminalNumber" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_MWBAND_NUMBER, (void *)i);
#ifdef E8B_NEW_DIAGNOSE
		FILE *fp;
		char buf[32];

		fp = fopen(NEW_SETTING, "r+");
		if (fp) {
			fgets(buf, sizeof(buf), fp);
			if (strcmp(buf, USERLIMIT_FILE)) {
				unlink(USERLIMIT_FILE);
			} else {
				rewind(fp);
				ftruncate(fileno(fp), 0);
				fprintf(fp, "%s", USERLIMIT_FILE);
			}
			fclose(fp);
		}

		fp = fopen(USERLIMIT_FILE, "w");
		if (fp) {
			fprintf(fp, "%d", *i);
			fclose(fp);

			if (access(REMOTE_SETSAVE_FILE, R_OK) == 0) {
				unlink(REMOTE_SETSAVE_FILE);
#ifdef CONFIG_USER_FLATFSD_XXX
				va_cmd("/bin/flatfsd", 1, 1, "-s");
#endif
			}
		}
#endif
	}else if( strcmp( lastname, "STBRestrictEnable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_MWBAND_STB_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "STBNumber" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_MWBAND_STB_NUM, (void *)i);
	}else if( strcmp( lastname, "CameraRestrictEnable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_MWBAND_CMR_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "CameraNumber" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_MWBAND_CMR_NUM, (void *)i);
	}else if( strcmp( lastname, "ComputerRestrictEnable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_MWBAND_PC_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "ComputerNumber" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_MWBAND_PC_NUM, (void *)i);		
	}else if( strcmp( lastname, "PhoneRestrictEnable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_MWBAND_PHN_ENABLE, (void *)&vChar);
	}else if( strcmp( lastname, "PhoneNumber" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		mib_set( CWMP_CT_MWBAND_PHN_NUM, (void *)i);
	}else{
		return ERR_9005;
	}
	
#ifdef CONFIG_CTC_E8_CLIENT_LIMIT
	proc_write_for_mwband();
	return 0;
#else
	return 1;
#endif
}
#endif //_PRMT_X_CT_COM_MWBAND_



#endif /*_PRMT_SERVICES_*/
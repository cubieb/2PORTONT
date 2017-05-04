#include <cwmp_utility.h>

#include "prmt_apply.h"
#include "prmt_ctcom.h"



/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_ACCOUNT_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_ACCOUNT_
struct CWMP_OP tCTAccountLeafOP = { getCTAccount, setCTAccount };
struct CWMP_PRMT tCTAccountLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTAccountLeafOP},
{"Password",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTAccountLeafOP}
};
enum eCTAccountLeaf
{
	eCTTA_Enable,
	eCTTA_Password
};
struct CWMP_LEAF tCTAccountLeaf[] =
{
{ &tCTAccountLeafInfo[eCTTA_Enable] },
{ &tCTAccountLeafInfo[eCTTA_Password] },
{ NULL }
};
/****************************************/
int getCTAccount(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	struct in_addr ipAddr;
	char buff[256]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( MIB_CTC_ACCOUNT_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "Password" )==0 )
	{
#ifdef CONFIG_MIDDLEWARE
		mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
		if(vChar == 0){
			mib_get(MIB_SUSER_PASSWORD, (void *)buff);
			*data = strdup(buff);
		}else
#endif	//end of CONFIG_MIDDLEWARE
		*data = strdup( "" );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setCTAccount(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_CTC_ACCOUNT_ENABLE, (void *)&vChar);
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "Password" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
		mib_set(MIB_SUSER_PASSWORD, (void *)buf);
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else{
		return ERR_9005;
	}

	return 0;
}
#endif /*_PRMT_X_CT_COM_ACCOUNT_*/
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_ACCOUNT_****************************************************************************/
/*************************************************************************************************************************/









/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_ALG_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_ALG_
struct CWMP_OP tXCTCOMALGLeafOP = { getXCTCOMALG, setXCTCOMALG };
struct CWMP_PRMT tXCTCOMALGLeafInfo[] =
{
/*(name,		type,		flag,		op)*/
{"H323Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"SIPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"RTSPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"L2TPEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"IPSECEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"FTPEnable",		eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
{"PPTPEnable", 	eCWMP_tBOOLEAN, CWMP_WRITE|CWMP_READ,	&tXCTCOMALGLeafOP},
};
enum eXCTCOMALGLeaf
{
	eCT_H323Enable,
	eCT_SIPEnable,
	eCT_RTSPEnable,
	eCT_L2TPEnable,
	eCT_IPSECEnable,
	eCT_FTPEnable,
	eCT_PPTPEnable,
};
struct CWMP_LEAF tXCTCOMALGLeaf[] =
{
{ &tXCTCOMALGLeafInfo[eCT_H323Enable] },
{ &tXCTCOMALGLeafInfo[eCT_SIPEnable] },
{ &tXCTCOMALGLeafInfo[eCT_RTSPEnable] },
{ &tXCTCOMALGLeafInfo[eCT_L2TPEnable] },
{ &tXCTCOMALGLeafInfo[eCT_IPSECEnable] },
{ &tXCTCOMALGLeafInfo[eCT_FTPEnable] },
{ &tXCTCOMALGLeafInfo[eCT_PPTPEnable] },
{ NULL }
};
/*******************************************/
int getXCTCOMALG(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "H323Enable" )==0 )
	{
#ifdef CONFIG_NF_CONNTRACK_H323
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_H323, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else if( strcmp( lastname, "SIPEnable" )==0 )
	{
#ifdef CONFIG_NF_CONNTRACK_SIP
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_SIP, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else if( strcmp( lastname, "RTSPEnable" )==0 )
	{
#ifdef CONFIG_NF_CONNTRACK_RTSP
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_RTSP, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else if( strcmp( lastname, "L2TPEnable" )==0 )
	{
#ifdef CONFIG_NF_CONNTRACK_L2TP
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_L2TP, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else if( strcmp( lastname, "IPSECEnable" )==0 )
	{
#ifdef CONFIG_NF_CONNTRACK_IPSEC
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_IPSEC, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else if( strcmp( lastname, "FTPEnable" )==0 )
	{
#ifdef CONFIG_NF_CONNTRACK_FTP
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_FTP, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else if( strcmp( lastname, "PPTPEnable" )==0 )
	{
#ifdef CONFIG_NF_CONNTRACK_PPTP
#ifdef CONFIG_IP_NF_ALG_ONOFF
		mib_get( MIB_IP_ALG_PPTP, (void *)&vChar);
#else
		vChar=1;
#endif
#endif
		*data = booldup( vChar );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setXCTCOMALG(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

#ifndef CONFIG_IP_NF_ALG_ONOFF
	 	return ERR_9008;
#else
	if( strcmp( lastname, "H323Enable" )==0 )
#ifdef CONFIG_NF_CONNTRACK_H323
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_H323, (void *)&vChar);

		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
#else
		return ERR_9005;
#endif


	if( strcmp( lastname, "SIPEnable" )==0 )
#ifdef CONFIG_NF_CONNTRACK_SIP
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_SIP, (void *)&vChar);

		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
#else
		return ERR_9005;
#endif

	if( strcmp( lastname, "RTSPEnable" )==0 )
#ifdef CONFIG_NF_CONNTRACK_RTSP
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_RTSP, (void *)&vChar);

		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
#else
		return ERR_9005;
#endif

	if( strcmp( lastname, "L2TPEnable" )==0 )
#ifdef CONFIG_NF_CONNTRACK_L2TP
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_L2TP, (void *)&vChar);

		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
#else
		return ERR_9005;
#endif

	if( strcmp( lastname, "IPSECEnable" )==0 )
#ifdef CONFIG_NF_CONNTRACK_IPSEC
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_IPSEC, (void *)&vChar);

		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
#else
		return ERR_9005;
#endif

	if( strcmp( lastname, "FTPEnable" )==0 )
#ifdef CONFIG_NF_CONNTRACK_FTP
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_FTP, (void *)&vChar);

		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
#else
		return ERR_9005;
#endif

	if( strcmp( lastname, "PPTPEnable" )==0 )
#ifdef CONFIG_NF_CONNTRACK_PPTP
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_IP_ALG_PPTP, (void *)&vChar);

		apply_add( CWMP_PRI_L, apply_ALGONOFF, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
#else
		return ERR_9005;
#endif

	return 0;
#endif
}
#endif //_PRMT_X_CT_COM_ALG_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_ALG_****************************************************************************/
/*************************************************************************************************************************/










/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_RECON_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_RECON_
struct CWMP_OP tCT_ReConLeafOP = { getCT_ReCon,	setCT_ReCon };
struct CWMP_PRMT tCT_ReConLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_ReConLeafOP},
};
enum eCT_ReConLeafInfo
{
	eCT_RCEnable
};
struct CWMP_LEAF tCT_ReConLeaf[] =
{
{ &tCT_ReConLeafInfo[eCT_RCEnable] },
{ NULL }
};
/***********************************/
int getCT_ReCon(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( CWMP_CT_RECON_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setCT_ReCon(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(CWMP_CT_RECON_ENABLE, (void *)&vChar);
#ifdef CONFIG_NO_REDIAL

		apply_add( CWMP_PRI_N, apply_Reconnect, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		return 0;
#endif
	}else{
		return ERR_9005;
	}

	return 0;
}
#endif //_PRMT_X_CT_COM_RECON_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_RECON_****************************************************************************/
/*************************************************************************************************************************/









/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_PORTALMNT_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_PORTALMNT_
struct CWMP_OP tCT_PortalMNTLeafOP = { getCT_PortalMNT, setCT_PortalMNT };
struct CWMP_PRMT tCT_PortalMNTLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
{"PortalUrl-Computer",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
{"PortalUrl-STB",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
{"PortalUrl-Phone",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PortalMNTLeafOP},
};
enum eCT_PortalMNTLeaf
{
	eCT_PMEnable,
	eCT_PMComputer,
	eCT_PMSTB,
	eCT_PMPhone
};
struct CWMP_LEAF tCT_PortalMNTLeaf[] =
{
{ &tCT_PortalMNTLeafInfo[eCT_PMEnable] },
{ &tCT_PortalMNTLeafInfo[eCT_PMComputer] },
{ &tCT_PortalMNTLeafInfo[eCT_PMSTB] },
{ &tCT_PortalMNTLeafInfo[eCT_PMPhone] },
{ NULL }
};
/********************************************/
int getCT_PortalMNT(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned char buf[256]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( CWMP_CT_PM_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "PortalUrl-Computer" )==0 )
	{
		mib_get( CWMP_CT_PM_URL4PC, (void *)buf);
		*data = strdup( buf );
	}else if( strcmp( lastname, "PortalUrl-STB" )==0 )
	{
		mib_get( CWMP_CT_PM_URL4STB, (void *)buf);
		*data = strdup( buf );
	}else if( strcmp( lastname, "PortalUrl-Phone" )==0 )
	{
		mib_get( CWMP_CT_PM_URL4MOBILE, (void *)buf);
		*data = strdup( buf );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setCT_PortalMNT(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;

		mib_set(CWMP_CT_PM_ENABLE, (void *)&vChar);
		apply_add( CWMP_PRI_N, apply_PortalMNT, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "PortalUrl-Computer" )==0 )
	{
		if(buf==NULL) return ERR_9007;

		mib_set( CWMP_CT_PM_URL4PC, (void *)buf);
		apply_add( CWMP_PRI_N, apply_PortalMNT, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "PortalUrl-STB" )==0 )
	{
		if(buf==NULL) return ERR_9007;

		mib_set( CWMP_CT_PM_URL4STB, (void *)buf);
		apply_add( CWMP_PRI_N, apply_PortalMNT, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "PortalUrl-Phone" )==0 )
	{
		if(buf==NULL) return ERR_9007;

		mib_set( CWMP_CT_PM_URL4MOBILE, (void *)buf);
		apply_add( CWMP_PRI_N, apply_PortalMNT, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else{
		return ERR_9005;
	}

	return 0;
}
#endif //_PRMT_X_CT_COM_PORTALMNT_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_PORTALMNT_****************************************************************************/
/*************************************************************************************************************************/










/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_SRVMNG_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_SRVMNG_
struct CWMP_OP tCTServiceLeafOP = { getCTService, setCTService };
struct CWMP_PRMT tCTServiceLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"FtpEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"FtpUserName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"FtpPassword",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"FtpPort",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetEnable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetUserName",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetPassword",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP},
{"TelnetPort",		eCWMP_tINT,	CWMP_WRITE|CWMP_READ,	&tCTServiceLeafOP}
};
enum eCTServiceLeaf
{
	eCT_FtpEnable,
	eCT_FtpUserName,
	eCT_FtpPassword,
	eCT_FtpPort,
	eCT_TelnetEnable,
	eCT_TelnetUserName,
	eCT_TelnetPassword,
	eCT_TelnetPort
};
struct CWMP_LEAF tCTServiceLeaf[] =
{
{ &tCTServiceLeafInfo[eCT_FtpEnable] },
{ &tCTServiceLeafInfo[eCT_FtpUserName] },
{ &tCTServiceLeafInfo[eCT_FtpPassword] },
{ &tCTServiceLeafInfo[eCT_FtpPort] },
{ &tCTServiceLeafInfo[eCT_TelnetEnable] },
{ &tCTServiceLeafInfo[eCT_TelnetUserName] },
{ &tCTServiceLeafInfo[eCT_TelnetPassword] },
{ &tCTServiceLeafInfo[eCT_TelnetPort] },
{ NULL }
};
/*************************************************/
int getCTService(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	struct in_addr ipAddr;
	char buff[256]={0};
	MIB_CE_ACC_T Entry;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry))
		return ERR_9002;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "FtpEnable" )==0 )
	{
		// LAN side are enabled => return 1
		*data = booldup( Entry.ftp==0x3 );
	}else if( strcmp( lastname, "FtpUserName" )==0 )
	{
#ifdef FTP_ACCOUNT_INDEPENDENT
		mib_get(MIB_FTP_USER, (void *)buff);
#else
		mib_get(MIB_SUSER_NAME, (void *)buff);
#endif
		*data=strdup( buff );
	}else if( strcmp( lastname, "FtpPassword" )==0 )
	{
#ifdef FTP_ACCOUNT_INDEPENDENT
		mib_get(MIB_FTP_PASSWD, (void *)buff);
#else
		mib_get(MIB_SUSER_PASSWORD, (void *)buff);
#endif
		*data=strdup( buff );
	}else if( strcmp( lastname, "FtpPort" )==0 )
	{
		*data = intdup( Entry.ftp_port );
	}else if( strcmp( lastname, "TelnetEnable" )==0 )
	{
		// LAN side are enabled => return 1
		*data = booldup( Entry.telnet==0x3 );
	}else if( strcmp( lastname, "TelnetUserName" )==0 )
	{
#ifdef TELNET_ACCOUNT_INDEPENDENT
		mib_get(MIB_TELNET_USER, (void *)buff);
#else
		mib_get(MIB_SUSER_NAME, (void *)buff);
#endif
		 *data=strdup( buff );
	}else if( strcmp( lastname, "TelnetPassword" )==0 )
	{
#ifdef TELNET_ACCOUNT_INDEPENDENT
		mib_get(MIB_TELNET_PASSWD, (void *)buff);
#else
		mib_get(MIB_SUSER_PASSWORD, (void *)buff);
#endif
		*data=strdup( buff );
	}else if( strcmp( lastname, "TelnetPort" )==0 )
	{
		*data = intdup( Entry.telnet_port );	}else{
		return ERR_9005;
	}

	return 0;
}

int setCTService(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	MIB_CE_ACC_T Entry;
	MIB_CE_ACC_T OldEntry;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if (!mib_chain_get(MIB_ACC_TBL, 0, (void *)&Entry))
		return ERR_9002;

	memcpy( &OldEntry, &Entry, sizeof(MIB_CE_ACC_T) );

	if( strcmp( lastname, "FtpEnable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		Entry.ftp=(*i==0)?0:0x3;
		mib_chain_update(MIB_ACC_TBL, (char *)&Entry, 0 );

		apply_add( CWMP_PRI_N, apply_RemoteAccess, CWMP_RESTART, 0, &OldEntry, sizeof(MIB_CE_ACC_T) );
		return 0;
	}else if( strcmp( lastname, "FtpUserName" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
#ifdef FTP_ACCOUNT_INDEPENDENT
		mib_set(MIB_FTP_USER, (void *)buf);
#else
		mib_set(MIB_SUSER_NAME, (void *)buf);
#endif
#ifndef FTP_ACCOUNT_INDEPENDENT
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
#endif
		return 0;
	}else if( strcmp( lastname, "FtpPassword" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
#ifdef FTP_ACCOUNT_INDEPENDENT
		mib_set(MIB_FTP_PASSWD, (void *)buf);
#else
		mib_set(MIB_SUSER_PASSWORD, (void *)buf);
#endif
#ifndef FTP_ACCOUNT_INDEPENDENT
		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
#endif
		return 0;
	}else if( strcmp( lastname, "FtpPort" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		if(*i<0 || *i>65535 ) return ERR_9007;
		Entry.ftp_port = *i;
		mib_chain_update(MIB_ACC_TBL, (char *)&Entry, 0 );

		apply_add( CWMP_PRI_N, apply_RemoteAccess, CWMP_RESTART, 0, &OldEntry, sizeof(MIB_CE_ACC_T) );
		return 0;
	}else if( strcmp( lastname, "TelnetEnable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		Entry.telnet=(*i==0)?0:0x3;
		mib_chain_update(MIB_ACC_TBL, (char *)&Entry, 0 );

		apply_add( CWMP_PRI_N, apply_RemoteAccess, CWMP_RESTART, 0, &OldEntry, sizeof(MIB_CE_ACC_T) );
		return 0;
	}else if( strcmp( lastname, "TelnetUserName" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
#ifdef TELNET_ACCOUNT_INDEPENDENT
		mib_set(MIB_TELNET_USER, (void *)buf);
		return 0;
#else
		mib_set(MIB_SUSER_NAME, (void *)buf);
#endif

		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "TelnetPassword" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
#ifdef TELNET_ACCOUNT_INDEPENDENT
		mib_set(MIB_TELNET_PASSWD, (void *)buf);
		return 0;
#else
		mib_set(MIB_SUSER_PASSWORD, (void *)buf);
#endif

		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "TelnetPort" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		if(*i<0 || *i>65535 ) return ERR_9007;
		Entry.telnet_port = *i;
		mib_chain_update(MIB_ACC_TBL, (char *)&Entry, 0 );

		apply_add( CWMP_PRI_N, apply_RemoteAccess, CWMP_RESTART, 0, &OldEntry, sizeof(MIB_CE_ACC_T) );
		return 0;
	}else{
		return ERR_9005;
	}

	return 0;
}
#endif //_PRMT_X_CT_COM_SRVMNG_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_SRVMNG_****************************************************************************/
/*************************************************************************************************************************/









/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_SYSLOG_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_SYSLOG_
struct CWMP_OP tCT_SyslogLeafOP = { getCT_Syslog, setCT_Syslog };
struct CWMP_PRMT tCT_SyslogLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_SyslogLeafOP},
{"Level",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_SyslogLeafOP},
};
enum eCT_SyslogLeaf
{
	eCT_SyslogEnable,
	eCT_SyslogLevel,
};
struct CWMP_LEAF tCT_SyslogLeaf[] =
{
{ &tCT_SyslogLeafInfo[eCT_SyslogEnable] },
{ &tCT_SyslogLeafInfo[eCT_SyslogLevel] },
{ NULL }
};

int getCT_Syslog(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned char buf[256]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get(MIB_SYSLOG, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}
	else if( strcmp( lastname, "Level" )==0 )
	{
		mib_get(MIB_SYSLOG_LOG_LEVEL, (void *)&vChar);

		if( vChar < 0 || vChar > 7 ) return ERR_9002;

		*data = uintdup(vChar);
	}
	else
		return ERR_9005;

	return 0;
}

int setCT_Syslog(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;

		vChar = (*i==0)?0:1;
		mib_set(MIB_SYSLOG, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_Syslog, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else if( strcmp( lastname, "Level" )==0 )
	{
		int *level = data;

		if( *level < 0 || *level > 7 ) return ERR_9007;

		vChar = *level;
		mib_set(MIB_SYSLOG_LOG_LEVEL, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_Syslog, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else
		return ERR_9005;

	return 0;
}
#endif //_PRMT_X_CT_COM_SYSLOG_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_SYSLOG_****************************************************************************/
/*************************************************************************************************************************/


/*************************************************************************************************************************/
/***********CONFIG_MIDDLEWARE****************************************************************************/
/*************************************************************************************************************************/
#ifdef CONFIG_MIDDLEWARE
struct CWMP_OP tCT_MiddlewareMgtLeafOP = { getCT_MiddlewareMgt, setCT_MiddlewareMgt };
struct CWMP_PRMT tCT_MiddlewareMgtLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Tr069Enable", 		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_MiddlewareMgtLeafOP},
{"MiddlewareURL",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_MiddlewareMgtLeafOP},
};

enum eCT_MiddlewareMgtLeaf
{
	eCT_MiddlewareURL,
	eCT_Tr069Enable,
};
struct CWMP_LEAF tCT_MiddlewareMgtLeaf[] =
{
{ &tCT_MiddlewareMgtLeafInfo[eCT_MiddlewareURL] },
{ &tCT_MiddlewareMgtLeafInfo[eCT_Tr069Enable] },
{ NULL }
};
/***************************************************/
int getCT_MiddlewareMgt(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned char buf[32] = {0};//martin zhu add
	unsigned char addrBuf[256+1]={0};
	int port = 0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Tr069Enable" )==0 )
	{
		mib_get(CWMP_TR069_ENABLE, (void *)&vChar);
		sprintf(buf, "%d", vChar);//martin zhu modify
		*data = strdup(buf);
	}else if( strcmp( lastname, "MiddlewareURL" )==0 )
	{
		mib_get(CWMP_MIDWARE_SERVER_ADDR,(void *)addrBuf);
		if(strlen(addrBuf) == 0)
			strcpy(addrBuf,"0.0.0.0");
		mib_get(CWMP_MIDWARE_SERVER_PORT,(void *)&port);
		sprintf(addrBuf,"%s:%d",addrBuf,port);
		*data = strdup(addrBuf);
	}else{
		return ERR_9005;
	}

	return 0;
}

#define MAX_MIDWARE_URL_LEN	256
int setCT_MiddlewareMgt(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned char buf[MAX_MIDWARE_URL_LEN+1];
	int port;
	char * delim = ":";
	char * pAddr=NULL;
	char * pPort=NULL;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Tr069Enable" )==0 )
	{
		unsigned char origVal;
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = *(char *)i - '0';
		mib_get(CWMP_TR069_ENABLE, (void *)&origVal);
		if (origVal == vChar)
			return 0;
		if(vChar < 0 || vChar > 2)
			return ERR_9007;
		if((origVal == 0 && vChar == 2) || (origVal == 2 && vChar == 0))
			return ERR_9001;
		mib_set(CWMP_TR069_ENABLE, (void *)&vChar);

		if((vChar == 2) || (origVal== 2 && vChar == 1))
		{
			apply_add( CWMP_PRI_H, apply_Midware, CWMP_RESTART, 0, &origVal, 1 );
			return 0;
		}
		return 1;
	}else if( strcmp( lastname, "MiddlewareURL" )==0 )
	{
		if(strlen(data) > MAX_MIDWARE_URL_LEN || strlen(data) == 0)
			return -1;
		strcpy(buf,(char *)data);
		pAddr = strtok(buf,delim);
		pPort = strtok(NULL,delim);
		if(!pAddr || !pPort || strlen(pAddr)==0 || strlen(pPort)==0)
			return -1;
		port = atoi(pPort);
		if(!mib_set(CWMP_MIDWARE_SERVER_ADDR,(void *)pAddr))
			return -1;
		if(!mib_set(CWMP_MIDWARE_SERVER_PORT,(void *)&port))
			return -1;

		return 0;
	}else{
		return ERR_9005;
	}

	return 0;
}
#endif //CONFIG_MIDDLEWARE
/*************************************************************************************************************************/
/***********END CONFIG_MIDDLEWARE****************************************************************************/
/*************************************************************************************************************************/







/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_UPNP_****************************************************************************/
/*************************************************************************************************************************/
#if defined(CONFIG_USER_UPNPD) || defined(CONFIG_USER_MINIUPNPD)
#ifdef _PRMT_X_CT_COM_UPNP_
struct CWMP_OP tCT_UPnPLeafOP = { getCT_UPnP, setCT_UPnP };
struct CWMP_PRMT tCT_UPnPLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_UPnPLeafOP},
};
enum eCT_UPnPLeaf
{
	eCT_UPnPEnable
};
struct CWMP_LEAF tCT_UPnPLeaf[] =
{
{ &tCT_UPnPLeafInfo[eCT_UPnPEnable] },
{ NULL }
};
/***********************************/
int getCT_UPnP(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "Enable" )==0 )
	{
		mib_get( MIB_UPNP_DAEMON, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else{
		return ERR_9005;
	}

	return 0;
}

int setCT_UPnP(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "Enable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_UPNP_DAEMON, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_UPnP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}else{
		return ERR_9005;
	}

	return 0;
}
#endif //_PRMT_X_CT_COM_UPNP_
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_UPNP_****************************************************************************/
/*************************************************************************************************************************/
#endif






/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_DLNA_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_DLNA_
struct CWMP_OP tCT_DLNALeafOP = { getCT_DLNA, setCT_DLNA };

struct CWMP_PRMT tCT_DLNALeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"DMSEnable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_DLNALeafOP},
};

enum eCT_DLNALeaf
{
	eCT_DMSEnable,
};

struct CWMP_LEAF tCT_DLNALeaf[] = {
{ &tCT_DLNALeafInfo[eCT_DMSEnable] },
{ NULL },
};

int getCT_DLNA(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "DMSEnable") == 0)
	{
		unsigned int enable = 0;

#ifdef CONFIG_USER_MINIDLNA
		if (!mib_get(MIB_DMS_ENABLE, &enable))
		{

			CWMPDBG(1, (stderr, "Get MIB_DMS_ENABLE failed.\n"));
			return ERR_9002;
		}
#endif
		*data = booldup(enable);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setCT_DLNA(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "DMSEnable") == 0)
	{
		int *i = data;;
		unsigned int enable = 0;

#ifdef CONFIG_USER_MINIDLNA
		enable = (*i == 1) ? 1 : 0;

		mib_set(MIB_DMS_ENABLE, &enable);

		apply_add(CWMP_PRI_L, apply_DLNA, CWMP_RESTART, 0, NULL, 0);
		return 0;
#else
		return ERR_9001;
#endif
	} else {
		return ERR_9005;
	}

	return 0;
}
#endif

/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_DLNA_****************************************************************************/
/*************************************************************************************************************************/





/*************************************************************************************************************************/
/***********_PRMT_USBRESTORE ****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_USBRESTORE
struct CWMP_OP tCT_RestoreLeafOP = { getCT_Restore, setCT_Restore };

struct CWMP_PRMT tCT_RestoreLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_RestoreLeafOP},
};

enum eCT_RestoreLeaf
{
	eCT_Restore_Enable,
};

struct CWMP_LEAF tCT_RestoreLeaf[] =
{
{ &tCT_RestoreLeafInfo[eCT_Restore_Enable] },
{ NULL },
};

int getCT_Restore(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enable") == 0)
	{
		unsigned char enable = 0;

		mib_get(MIB_USBRESTORE, &enable);

		*data = booldup(enable);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setCT_Restore(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Enable") == 0)
	{
		int *i = data;
		unsigned char enable = (*i == 0) ? 0 : 1;

		mib_set(MIB_USBRESTORE, &enable);
		return 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}
#endif

/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_RESTORE_****************************************************************************/
/*************************************************************************************************************************/





/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_DEVINFO_****************************************************************************/
/*************************************************************************************************************************/

/***** InternetGatewayDevice.DeviceInfo. */
#ifdef _PRMT_X_CT_COM_DEVINFO_
struct CWMP_OP tCTDeviceInfoLeafOP = { getCTDeviceInfo, setCTDeviceInfo };
struct CWMP_PRMT tCTDeviceInfoLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"X_CT-COM_InterfaceVersion",	eCWMP_tSTRING,	CWMP_READ,	&tCTDeviceInfoLeafOP},
{"X_CT-COM_CardInterVersion",	eCWMP_tSTRING,	CWMP_READ,	&tCTDeviceInfoLeafOP},
{"X_CT-COM_DeviceType",			eCWMP_tUINT,	CWMP_READ,	&tCTDeviceInfoLeafOP},
{"X_CT-COM_Card",				eCWMP_tBOOLEAN,	CWMP_READ,	&tCTDeviceInfoLeafOP},
{"X_CT-COM_UPLink",				eCWMP_tUINT,	CWMP_READ,	&tCTDeviceInfoLeafOP},
{"X_CT-COM_Capability",			eCWMP_tSTRING,	CWMP_READ,	&tCTDeviceInfoLeafOP},
{"X_CT-COM_IPForwardModeEnabled",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,	&tCTDeviceInfoLeafOP},
};

int getCTDeviceInfo(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "X_CT-COM_InterfaceVersion" )==0 )
	{
		// ITMS+ Interface version
		*data = strdup("2271-2013.A.1");
	}
	else if( strcmp( lastname, "X_CT-COM_CardInterVersion" )==0 )
	{
		*data = strdup("");	//no card
	}
	else if( strcmp( lastname, "X_CT-COM_DeviceType" )==0 )
	{
#ifdef CONFIG_LUNA
		if(SW_LAN_PORT_NUM == 4)
			*data = uintdup(3);	//standard e8-C
		else
			*data = uintdup(4);	//9602B e8-C
#else
		*data = uintdup(1);	//standard e8-B
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_Card" )==0 )
	{
		*data = booldup(0);	//no card
	}
	else if( strcmp( lastname, "X_CT-COM_UPLink" )==0 )
	{
#ifdef CONFIG_DEV_xDSL
#ifdef CONFIG_VDSL
		int mval = 0;
		XDSL_OP *op = xdsl_get_op(0);

		op->xdsl_msg_get(GetPmdMode,&mval);

		if(mval & MODE_VDSL2)
			*data = uintdup(3); 	//VDSL2
		else
#endif	//CONFIG_VDSL
			*data = uintdup(1);		//ADSL2+
#elif defined(CONFIG_LUNA)
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
		int mode;

		mib_get(MIB_PON_MODE, &mode);
		if(mode == EPON_MODE)
			*data = uintdup(4);
		else if(mode == GPON_MODE)
			*data = uintdup(5);
		else
#endif
			*data = uintdup(2);

#else
#error X_CT-COM_UPLink is not defined
#endif
		/*	1 ADSL2+
		2 LAN
		3 VDSL2
		4 EPON
		5 GPON
		6 10GEOPN
		7 XGPON*/
	}
	else if( strcmp( lastname, "X_CT-COM_Capability" )==0 )
	{
		/*	4th (2.4G WiFi) and 5th (5G WiFi) segment:
				0: No WiFi
				1: 802.11 b/g
				2: 1x1 11n
				3: 2x2 11n
				4: 3x3 11n
				5: AC*/

		//Need to check if platform is changed
#ifdef CONFIG_LUNA
		/*	2 POTS
			3 FE + 1 GE Ethernet LAN ports
			2 USB Port
			2x2 11n 2.4G WiFi
			No 5G WiFi*/
		//This value is for 9607
		if(SW_LAN_PORT_NUM == 4)
			*data = strdup("2;3FE+1GE;2;3;0");	//9607
		else
			*data = strdup("1;1FE+1GE;0;0;0");	//9602B
#else
		/*	2 POTS
			5 FE Ethernet LAN ports
			1 USB Port
			2x2 11n 2.4G WiFi
			No 5G WiFi*/
		//This value is for 8685
		*data = strdup("2;5FE;1;3;0");
#endif
	}
	else if( strcmp( lastname, "X_CT-COM_IPForwardModeEnabled" )==0 )
	{
		// We don't support IP Forward Mode
		*data = booldup(0);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setCTDeviceInfo(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "X_CT-COM_IPForwardModeEnabled") == 0)
	{
		// We don't support IP Forward Mode
		return ERR_9001;
	}
	else {
		return ERR_9005;
	}

	return 0;
}

#endif

/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_DEVINFO_****************************************************************************/
/*************************************************************************************************************************/







/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_ALARM_MONITOR_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_ALARM_MONITOR_
struct CWMP_OP tCT_AlarmLeafOP = { getCT_Alarm, setCT_Alarm };
struct CWMP_PRMT tCT_AlarmLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_AlarmLeafOP},
{"AlarmNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tCT_AlarmLeafOP},
{"AlarmNumber",	eCWMP_tSTRING,	CWMP_READ,				&tCT_AlarmLeafOP},
};
enum eCT_AlarmLeaf
{
	eCT_AlarmEnable,
	eCT_AlarmNumberOfEntries,
	eCT_AlarmNumber,
};
struct CWMP_LEAF tCT_AlarmLeaf[] =
{
{ &tCT_AlarmLeafInfo[eCT_AlarmEnable] },
{ &tCT_AlarmLeafInfo[eCT_AlarmNumberOfEntries] },
{ &tCT_AlarmLeafInfo[eCT_AlarmNumber] },
{ NULL }
};

struct CWMP_OP tCT_AlarmEntityLeafOP = { getCT_AlarmEntity, setCT_AlarmEntity };
struct CWMP_PRMT tCT_AlarmEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"ParaList",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_AlarmEntityLeafOP},
{"Limit-Max",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_AlarmEntityLeafOP},
{"Limit-Min",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_AlarmEntityLeafOP},
{"TimeList",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_AlarmEntityLeafOP},
{"Mode",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_AlarmEntityLeafOP},
};
enum eCT_AlarmEntityLeaf
{
	eCT_ParaList,
	eCT_LimitMax,
	eCT_LimitMin,
	eCT_TimeList,
	eCT_Mode,
};
struct CWMP_LEAF tCT_AlarmEntityLeaf[] =
{
{ &tCT_AlarmEntityLeafInfo[eCT_ParaList] },
{ &tCT_AlarmEntityLeafInfo[eCT_LimitMax] },
{ &tCT_AlarmEntityLeafInfo[eCT_LimitMin] },
{ &tCT_AlarmEntityLeafInfo[eCT_TimeList] },
{ &tCT_AlarmEntityLeafInfo[eCT_Mode] },
{ NULL }
};

struct CWMP_PRMT tCT_AlarmEntityObjectInfo[] =
{
/*(name,		type,		flag,					op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_AlarmEntityObject
{
	eAlarmEntity0
};
struct CWMP_LINKNODE tCT_AlarmEntityObject[] =
{
/*info,  					leaf,			next,		sibling,		instnum)*/
{&tCT_AlarmEntityObjectInfo[eAlarmEntity0], 	tCT_AlarmEntityLeaf,	NULL,		NULL,			0}
};

struct CWMP_OP tCT_Alarm_OP = { NULL, objCT_AlarmEntity};
struct CWMP_PRMT tCT_AlarmObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"AlarmConfig",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tCT_Alarm_OP}
};
enum eCT_AlarmObject
{
	eAlarm
};
struct CWMP_NODE tCT_AlarmObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_AlarmObjectInfo[eAlarm], 		NULL,			NULL},
{NULL,					NULL,			NULL}
};

/*** Monitor ***/
struct CWMP_OP tCT_MonitorLeafOP = { getCT_Monitor, setCT_Monitor };
struct CWMP_PRMT tCT_MonitorLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_MonitorLeafOP},
{"MonitormNumberOfEntries",eCWMP_tUINT,	CWMP_READ,		&tCT_MonitorLeafOP},
};
enum eCT_MonitorLeaf
{
	eCT_MonitorEnable,
	eCT_MonitorNumberOfEntries,
};
struct CWMP_LEAF tCT_MonitorLeaf[] =
{
{ &tCT_MonitorLeafInfo[eCT_MonitorEnable] },
{ &tCT_MonitorLeafInfo[eCT_MonitorNumberOfEntries] },
{ NULL }
};

struct CWMP_OP tCT_MonitorEntityLeafOP = { getCT_MonitorEntity, setCT_MonitorEntity };
struct CWMP_PRMT tCT_MonitorEntityLeafInfo[] =
{
/*(name,		type,		flag,			op)*/
{"ParaList",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_MonitorEntityLeafOP},
{"TimeList",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_MonitorEntityLeafOP},
};
enum eCT_MonitorEntityLeaf
{
	eCT_MonitorParaList,
	eCT_MonitorTimeList,
};
struct CWMP_LEAF tCT_MonitorEntityLeaf[] =
{
{ &tCT_MonitorEntityLeafInfo[eCT_MonitorParaList] },
{ &tCT_MonitorEntityLeafInfo[eCT_MonitorTimeList] },
{ NULL }
};

struct CWMP_PRMT tCT_MonitorEntityObjectInfo[] =
{
/*(name,		type,		flag,					op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_MonitorEntityObject
{
	eMonitorEntity0
};
struct CWMP_LINKNODE tCT_MonitorEntityObject[] =
{
/*info,  					leaf,			next,		sibling,		instnum)*/
{&tCT_MonitorEntityObjectInfo[eMonitorEntity0], 	tCT_MonitorEntityLeaf,	NULL,		NULL,			0}
};

struct CWMP_OP tCT_Monitor_OP = { NULL, objCT_MonitorEntity};
struct CWMP_PRMT tCT_MonitorObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"MonitorConfig",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tCT_Monitor_OP}
};
enum eCT_MonitorObject
{
	eMonitor
};
struct CWMP_NODE tCT_MonitorObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_MonitorObjectInfo[eMonitor], 		NULL,			NULL},
{NULL,					NULL,			NULL}
};


extern ALARM_TIMER *alarm_timer;
enum timer_type {FOR_ALARM, FOR_MONITOR};

int clearAlarmTimer(unsigned char type)
{
	ALARM_TIMER *timer, *timerNext;

	for (timer = alarm_timer; timer; timer = timerNext)
	{
		timerNext = timer->next;

		if((type == FOR_MONITOR && timer->state == STATE_MONITOR)
			|| (type == FOR_ALARM && timer->state != STATE_MONITOR))
			free(timer);
	}

	alarm_timer = NULL;

	return 0;
}

int insertAlarmTimer(unsigned char type, unsigned int instnum, unsigned int period)
{
	ALARM_TIMER *timer;

	if (instnum == 0)
		return -1;

	for (timer = alarm_timer; timer; timer = timer->next)
	{
		if (type == FOR_ALARM && timer->state != STATE_MONITOR && timer->instnum == instnum)
			return 0;
		if(type == FOR_MONITOR && timer->state == STATE_MONITOR && timer->instnum == instnum)
			return 0;
	}

	timer = malloc(sizeof(*timer));
	if (!timer) {
		warn("%s():%d ", __FUNCTION__, __LINE__);
		return -1;
	}
	memset(timer, 0, sizeof(*timer));
	timer->period = period;
	timer->instnum = instnum;
	timer->next = alarm_timer;
	alarm_timer = timer;
	timer->divisor = 0;
	timer->value = 0;
	if(type == FOR_MONITOR)
		timer->state = STATE_MONITOR;
	else
		timer->state = STATE_OFF;

	return 0;
}

int deleteAlarmTimerByInstnum(unsigned char type, unsigned int instnum)
{
	ALARM_TIMER *tmp, **p;

	if (instnum == 0)
		return -1;

	for (p = &alarm_timer; *p; p = &(*p)->next)
	{
		if(type == FOR_MONITOR && (*p)->state != STATE_MONITOR)
			continue;
		if(type == FOR_ALARM && (*p)->state == STATE_MONITOR)
			continue;

		if ((*p)->instnum == instnum) {
			tmp = *p;
			*p = (*p)->next;
			free(tmp);
			break;
		}
	}

	return 0;
}

int getAlarmInstNum(char *name, unsigned int *pInstNum)
{
	if ((name == NULL) || (pInstNum == NULL))
		return -1;

	*pInstNum = getInstNum(name, "AlarmConfig");

	return 0;
}

int getAlarmByInstnum(unsigned int instnum, CWMP_CT_ALARM_T *p, unsigned int *id)
{
	int ret = -1, i, num;

	if ((instnum == 0) || (p == NULL) || (id == NULL))
		return ret;

	num = mib_chain_total(CWMP_CT_ALARM_TBL);
	for (i = 0; i < num; i++) {
		if (!mib_chain_get(CWMP_CT_ALARM_TBL, i, p)) {
			continue;
		}

		if (p->InstanceNum == instnum) {
			*id = i;
			ret = 0;
			break;
		}
	}

	return ret;
}

int getAlarmPeriodByInstnum(unsigned int instnum, unsigned int *pPeriod)
{
	int ret = -1, i, num;
	unsigned char vChar = 0;
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

	if ((instnum == 0) || (pPeriod == NULL))
		return ret;

	mib_get(CWMP_CT_ALARM_ENABLE, (void *)&vChar);
	if (vChar) {
		num = mib_chain_total(CWMP_CT_ALARM_TBL);
		for (i = 0; i < num; i++) {
			if (!mib_chain_get(CWMP_CT_ALARM_TBL, i, pAlarm)) {
				continue;
			}

			if (pAlarm->InstanceNum == instnum) {
				*pPeriod = pAlarm->period * 60;
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

int findMaxAlarmInstNum(unsigned int *pInstNum)
{
	int ret = -1, i, num;
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

	if (pInstNum == NULL)
		return ret;

	*pInstNum = 0;

	num = mib_chain_total(CWMP_CT_ALARM_TBL);
	for (i = 0; i < num; i++) {
		if (!mib_chain_get(CWMP_CT_ALARM_TBL, i, pAlarm)) {
			continue;
		}

		if (pAlarm->InstanceNum > *pInstNum) {
			*pInstNum = pAlarm->InstanceNum;
			ret = 0;
		}
	}

	return ret;
}

static const char *alarmNumPrefix[] = {
	"100", "101", "102", "103", "104",	 	//device alarm
	"201", "202",					//service quality alarm
	"301", "302", "303",				//process error alarm
	"401",						//comnunication alarm
	"500" };					//environment alarm

int isValidAlarmNumber(unsigned int alarmNumber)
{
	int ret = 0;
	unsigned int i;
	char alarmNumStr[8];

	if (alarmNumber > 999999) {
		return ret;
	}

	sprintf(alarmNumStr, "%u", alarmNumber);
	for (i = 0; i < ARRAY_SIZE(alarmNumPrefix); i++) {
		if (!strcmp(alarmNumPrefix[i], alarmNumStr)) {
			ret = 1;
			break;
		}
	}

	return ret;
}

int isOverThresholdAlarm(unsigned int alarmNumber)
{
	char alarmNumStr[8];

	sprintf(alarmNumStr, "%u", alarmNumber);
	return (!strncmp(alarmNumPrefix[5], alarmNumStr, 3));
}

int alarm_check_threshold(ALARM_TIMER *timer)
{
	int type, ret = 0;
	unsigned int i, num;
	void *value;
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

	num = mib_chain_total(CWMP_CT_ALARM_TBL);
	for (i = 0; i < num; i++) {
		if (!mib_chain_get(CWMP_CT_ALARM_TBL, i, pAlarm)) {
			continue;
		}

		if (!isOverThresholdAlarm(pAlarm->alarmNumber)) {
			// Not a instance of AlarmConfig
			continue;
		}

		if (pAlarm->InstanceNum == timer->instnum &&
			get_ParameterValue(pAlarm->para, &type, &value) == 0)
		{
			unsigned check = 0;

			switch(pAlarm->mode)
			{
			case ALARM_MODE_INST:
				timer->value = *(int *)value;
				check = 1;
				break;
			case ALARM_MODE_ADD:
				timer->value += *(int *)value;
				CWMPDBG(2, (stderr, "ALARM_MODE_ADD: %s, value=%d\n", pAlarm->para, timer->value));
				check = 1;
				break;
			case ALARM_MODE_AVER:
				timer->value = (timer->value * timer->divisor + (*(int *)value))
								/ ++timer->divisor;
				CWMPDBG(2, (stderr, "ALARM_MODE_AVER: %s, value=%d, divisor = %d\n", pAlarm->para, timer->value, timer->divisor));
				check = 1;
				break;
			}

			get_ParameterValueFree(type, value);

			if(check)
			{
				if( (timer->value < pAlarm->limitMin)
					|| (timer->value > pAlarm->limitMax))
				{
					CWMPDBG(1, (stderr, "Send alarm: %s, mode=%u, value=%d\n", pAlarm->para, pAlarm->mode, timer->value));
					ret = 1;
				}
			}

			break;
		}
	}

	return ret;
}

int getMonitorInstNum(char *name, unsigned int *pInstNum)
{
	if ((name == NULL) || (pInstNum == NULL))
		return -1;

	*pInstNum = getInstNum(name, "MonitorConfig");

	return 0;
}

int getMonitorByInstnum(unsigned int instnum, CWMP_CT_MONITOR_T *p, unsigned int *id)
{
	int ret = -1, i, num;

	if ((instnum == 0) || (p == NULL) || (id == NULL))
		return ret;

	num = mib_chain_total(CWMP_CT_MONITOR_TBL);
	for (i = 0; i < num; i++) {
		if (!mib_chain_get(CWMP_CT_MONITOR_TBL, i, p)) {
			continue;
		}

		if (p->InstanceNum == instnum) {
			*id = i;
			ret = 0;
			break;
		}
	}

	return ret;
}

int getMonitorPeriodByInstnum(unsigned int instnum, unsigned int *pPeriod)
{
	int ret = -1, i, num;
	unsigned char vChar = 0;
	CWMP_CT_MONITOR_T monitor, *pMonitor = &monitor;

	if ((instnum == 0) || (pPeriod == NULL))
		return ret;

	mib_get(CWMP_CT_MONITOR_ENABLE, (void *)&vChar);
	if (vChar) {
		num = mib_chain_total(CWMP_CT_MONITOR_TBL);
		for (i = 0; i < num; i++) {
			if (!mib_chain_get(CWMP_CT_MONITOR_TBL, i, pMonitor)) {
				continue;
			}

			if (pMonitor->InstanceNum == instnum) {
				*pPeriod = pMonitor->period * 60;
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

int findMaxMonitorInstNum(unsigned int *pInstNum)
{
	int ret = -1, i, num;
	CWMP_CT_MONITOR_T monitor, *pAlarm = &monitor;

	if (pInstNum == NULL)
		return ret;

	*pInstNum = 0;

	num = mib_chain_total(CWMP_CT_MONITOR_TBL);
	for (i = 0; i < num; i++) {
		if (!mib_chain_get(CWMP_CT_MONITOR_TBL, i, pAlarm)) {
			continue;
		}

		if (pAlarm->InstanceNum > *pInstNum) {
			*pInstNum = pAlarm->InstanceNum;
			ret = 0;
		}
	}

	return ret;
}


int init_alarm_timer()
{
	int period, i, num;
	unsigned char enable = 0;
	ALARM_TIMER *timer;

	//Alarm timers
	mib_get(CWMP_CT_ALARM_ENABLE, &enable);
	if (enable)
	{
		CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

		num = mib_chain_total(CWMP_CT_ALARM_TBL);
		for (i = 0; i < num; i++)
		{
			if (!mib_chain_get(CWMP_CT_ALARM_TBL, i, pAlarm))
				continue;

			/* Tsai: why is these checkings necessary */
			if (pAlarm->para[0]
					&& pAlarm->mode
					&& pAlarm->limitMax
					&& pAlarm->limitMin
					&& pAlarm->period)
			{
				period = pAlarm->period * 60;
			}
			else
			{
				period = 0;
			}

			insertAlarmTimer(FOR_ALARM, pAlarm->InstanceNum, period);
		}
	}
	else
		clearAlarmTimer(FOR_ALARM);

	// Monitor timers
	mib_get(CWMP_CT_MONITOR_ENABLE, &enable);
	if (enable)
	{
		CWMP_CT_MONITOR_T monitor, *pMonitor = &monitor;

		num = mib_chain_total(CWMP_CT_MONITOR_TBL);
		for (i = 0; i < num; i++) {
			if (!mib_chain_get(CWMP_CT_MONITOR_TBL, i, pMonitor)) {
				continue;
			}

			if (pMonitor->para[0] && pMonitor->period)
			{
				period = pMonitor->period * 60;
				insertAlarmTimer(FOR_MONITOR, pMonitor->InstanceNum, period);
			}
		}
	}
	else
		clearAlarmTimer(FOR_MONITOR);

	return 0;
}

int updateAlarmTimer(CWMP_CT_ALARM_T *pAlarm)
{
	int ret = 0;
	ALARM_TIMER *timer;

	if (!pAlarm || pAlarm->para[0] == 0
			|| pAlarm->mode == ALARM_MODE_NONE
			|| pAlarm->limitMax == 0
			|| pAlarm->limitMin == 0
			|| pAlarm->period == 0) {
		return ret;
	}

	for (timer = alarm_timer; timer; timer = timer->next)
	{
		if(timer->state != STATE_MONITOR
			&& timer->instnum == pAlarm->InstanceNum)
		{
			timer->period = pAlarm->period * 60;
			timer->divisor = 0;
			timer->value = 0;
			timer->state = STATE_OFF;
			ret = 1;
			break;
		}
	}

	return ret;
}

int updateMonitorTimer(CWMP_CT_MONITOR_T *pMonitor)
{
	int ret = 0;
	ALARM_TIMER *timer;

	if (!pMonitor || pMonitor->para[0] == 0
			|| pMonitor->period == 0) {
		return ret;
	}

	for (timer = alarm_timer; timer; timer = timer->next)
	{
		if(timer->state == STATE_MONITOR
			&& timer->instnum == pMonitor->InstanceNum)
		{
			timer->period = pMonitor->period * 60;
			ret = 1;
			break;
		}
	}

	return ret;
}


int getCT_Alarm(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enable") == 0) {
		unsigned char vChar = 0;
		mib_get(CWMP_CT_ALARM_ENABLE, &vChar);
		*data = booldup(vChar != 0);
	} else if (strcmp(lastname, "AlarmNumberOfEntries") == 0) {
		int total = mib_chain_total(CWMP_CT_ALARM_TBL);
		int i, cnt = 0;
		CWMP_CT_ALARM_T alarm;

		for(i = 0 ; i < total ; i++)
		{
			mib_chain_get(CWMP_CT_ALARM_TBL, i, &alarm);
			if(alarm.mode != ALARM_MODE_NONE)
				cnt++;
		}
		*data = uintdup(cnt);
	} else if (strcmp(lastname, "AlarmNumber") == 0) {
		int total = mib_chain_total(CWMP_CT_ALARM_TBL);
		int i, first = 1;
		CWMP_CT_ALARM_T alarm;
		char buf[1024] = {0};

		for(i = 0 ; i < total ; i++)
		{
			mib_chain_get(CWMP_CT_ALARM_TBL, i, &alarm);
			if(alarm.mode == ALARM_MODE_NONE)
			{
				if(first)
				{
					sprintf(buf, "%d", alarm.alarmNumber);
					first = 0;
				}
				else
					sprintf(buf, "%s,%d", buf, alarm.alarmNumber);
			}
		}
		*data = strdup(buf);
	} else {
		return ERR_9005;
	}

	return 0;
}

int setCT_Alarm(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Enable") == 0) {
		unsigned char vChar=0;

		int *i = data;

		if (i == NULL) {
			return ERR_9007;
		}
		vChar = (*i != 0);
		mib_set(CWMP_CT_ALARM_ENABLE, &vChar);
		init_alarm_timer();
	} else {
		return ERR_9005;
	}

	return 0;
}

int getCT_AlarmEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int instNum, chainid;
	char buf[16] = {0};
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	getAlarmInstNum(name, &instNum);
	if (instNum == 0)
		return ERR_9005;

	if (getAlarmByInstnum(instNum, pAlarm, &chainid) <0)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "ParaList") == 0)
	{
		*data = strdup(pAlarm->para);
	}
	else if (strcmp(lastname, "Limit-Max") == 0)
	{
		sprintf(buf, "%d", pAlarm->limitMax);
		*data = strdup(buf);
	}
	else if (strcmp(lastname, "Limit-Min") == 0)
	{
		sprintf(buf, "%d", pAlarm->limitMin);
		*data = strdup(buf);
	}
	else if (strcmp(lastname, "TimeList") == 0)
	{
		sprintf(buf, "%u", pAlarm->period);
		*data = strdup(buf);
	}
	else if (strcmp(lastname, "Mode") == 0)
	{
		*data = uintdup(pAlarm->mode);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_AlarmEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int instNum, chainid;
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	getAlarmInstNum(name, &instNum);
	if (instNum == 0)
		return ERR_9005;

	if (getAlarmByInstnum(instNum, pAlarm, &chainid) < 0)
		return ERR_9005;

	if (strcmp(lastname, "ParaList") == 0)
	{
		struct CWMP_LEAF *e;
		char *buf = data;

		if (strlen(buf) == 0 || get_ParameterEntity(buf, &e) < 0)
			return ERR_9007;

		strncpy(pAlarm->para, buf, PARA_NAME_LEN - 1);
		pAlarm->para[PARA_NAME_LEN] = 0;
		updateAlarmTimer(pAlarm);
		mib_chain_update(CWMP_CT_ALARM_TBL, pAlarm, chainid);

		return 0;
	} else if (strcmp(lastname, "Limit-Max") == 0) {
		char *buf = data;
		int max = atoi(buf);

		pAlarm->limitMax = max;
		updateAlarmTimer(pAlarm);
		mib_chain_update(CWMP_CT_ALARM_TBL, pAlarm, chainid);

		return 0;
	} else if (strcmp(lastname, "Limit-Min") == 0) {
		char *buf = data;
		int min = atoi(buf);

		pAlarm->limitMin = min;
		updateAlarmTimer(pAlarm);
		mib_chain_update(CWMP_CT_ALARM_TBL, pAlarm, chainid);

		return 0;
	} else if (strcmp(lastname, "TimeList") == 0) {
		char *buf = data;
		int period = atoi(buf);

		if (period <= 0)
			return ERR_9007;

		pAlarm->period = period;
		updateAlarmTimer(pAlarm);
		mib_chain_update(CWMP_CT_ALARM_TBL, pAlarm, chainid);

		return 0;
	} else if (strcmp(lastname, "Mode") == 0) {
		unsigned int *mode = data;

		if (mode == NULL || *mode < ALARM_MODE_ADD || *mode > ALARM_MODE_INST)
			return ERR_9007;

		pAlarm->mode = *mode;
		updateAlarmTimer(pAlarm);
		mib_chain_update(CWMP_CT_ALARM_TBL, pAlarm, chainid);

		return 0;
	} else {
		return ERR_9005;
	}

	return 0;
}

int objCT_AlarmEntity(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, num, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	CWMP_CT_ALARM_T alarm, *pAlarm = &alarm;

	if (name == NULL || entity == NULL)
		return -1;

	num = mib_chain_total(CWMP_CT_ALARM_TBL);

	switch (type) {
	case eCWMP_tINITOBJ:{
			struct CWMP_LINKNODE **c =
			    (struct CWMP_LINKNODE **)data;

			if (data == NULL)
				return -1;

			findMaxAlarmInstNum(&maxInstNum);

			for (i = 0; i < num; i++) {
				if (!mib_chain_get
				    (CWMP_CT_ALARM_TBL, i, pAlarm))
					continue;

				if(pAlarm->mode == ALARM_MODE_NONE)
					continue;

				if (pAlarm->InstanceNum == 0) {
					maxInstNum++;
					pAlarm->InstanceNum = maxInstNum;
					mib_chain_update(CWMP_CT_ALARM_TBL,
							 pAlarm, i);
				}

				if (create_Object(c, tCT_AlarmEntityObject,
						  sizeof(tCT_AlarmEntityObject),
						  1, pAlarm->InstanceNum) < 0)
					return -1;
			}

			add_objectNum(name, maxInstNum);

			break;
		}
	case eCWMP_tADDOBJ:{
			if (data == NULL)
				return -1;

			ret =
			    add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_AlarmEntityObject,
				       sizeof(tCT_AlarmEntityObject), data);
			if (ret >= 0) {
				CWMP_CT_ALARM_T entry;

				memset(&entry, 0, sizeof(entry));
				entry.alarmNumber = 201000 + *(int *)data;
				entry.InstanceNum = *(int *)data;
				entry.mode = ALARM_MODE_INST;
				insertAlarmTimer(FOR_ALARM, entry.InstanceNum, 0);
				mib_chain_add(CWMP_CT_ALARM_TBL, &entry);
			}

			break;
		}
	case eCWMP_tDELOBJ:{
			int found = 0;
			unsigned int *pUint = data;

			if (data == NULL)
				return -1;

			for (i = 0; i < num; i++) {
				if (!mib_chain_get
				    (CWMP_CT_ALARM_TBL, i, pAlarm))
					continue;

				if(pAlarm->mode == ALARM_MODE_NONE)
					continue;

				if (pAlarm->InstanceNum == *pUint) {
					found = 1;
					deleteAlarmTimerByInstnum(FOR_ALARM, pAlarm->InstanceNum);
					mib_chain_delete(CWMP_CT_ALARM_TBL, i);

					break;
				}
			}

			if (found) {
				ret =
				    del_Object(name,
					       (struct CWMP_LINKNODE **)
					       &entity->next, *(int *)data);
			} else {
				ret = ERR_9005;
			}

			break;
		}
	case eCWMP_tUPDATEOBJ:{
			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < num; i++) {
				struct CWMP_LINKNODE *remove_entity = NULL;

				if (!mib_chain_get
				    (CWMP_CT_ALARM_TBL, i, pAlarm))
					continue;

				if(pAlarm->mode == ALARM_MODE_NONE)
					continue;

				remove_entity =
				    remove_SiblingEntity(&old_table,
							 pAlarm->InstanceNum);

				if (remove_entity != NULL) {
					/* Tsai: it exists both in the MIB and the parameter tree */
					add_SiblingEntity((struct CWMP_LINKNODE
							   **)&entity->next,
							  remove_entity);
				} else {
					/* Tsai: it exists only in the MIB,
					 * so we add it into the parameter tree */
					add_Object(name,
						   (struct CWMP_LINKNODE **)
						   &entity->next,
						   tCT_AlarmEntityObject,
						   sizeof
						   (tCT_AlarmEntityObject),
						   &pAlarm->InstanceNum);
					mib_chain_update(CWMP_CT_ALARM_TBL,
							 pAlarm, i);
				}
			}

			if (old_table) {
				/* Tsai: it exists only in the parameter tree,
				 * so we remove it from the parameter tree */
				destroy_ParameterTable((struct CWMP_NODE *)
						       old_table);
			}

			break;
		}
	}

	return ret;
}

/***** Monitor Operations ****/
int getCT_Monitor(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enable") == 0)
	{
		unsigned char vChar = 0;
		mib_get(CWMP_CT_MONITOR_ENABLE, &vChar);
		*data = booldup(vChar != 0);
	}
	else if (strcmp(lastname, "MonitorNumberOfEntries") == 0)
	{
		*data = uintdup(mib_chain_total(CWMP_CT_MONITOR_TBL));
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_Monitor(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, "Enable") == 0)
	{
		unsigned char vChar=0;
		int *i = data;

		vChar = (*i != 0);
		mib_set(CWMP_CT_MONITOR_ENABLE, &vChar);
		init_alarm_timer();
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int getCT_MonitorEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int instNum, chainid;
	char buf[16] = {0};
	CWMP_CT_MONITOR_T monitor, *pMonitor = &monitor;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	getMonitorInstNum(name, &instNum);
	if (instNum == 0)
		return ERR_9005;

	if (getMonitorByInstnum(instNum, pMonitor, &chainid) <0)
		return ERR_9005;

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "ParaList") == 0)
	{
		*data = strdup(pMonitor->para);
	}
	else if (strcmp(lastname, "TimeList") == 0)
	{
		sprintf(buf, "%u", pMonitor->period);
		*data = strdup(buf);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_MonitorEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int instNum, chainid;
	CWMP_CT_MONITOR_T monitor, *pMonitor = &monitor;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	getMonitorInstNum(name, &instNum);
	if (instNum == 0)
		return ERR_9005;

	if (getMonitorByInstnum(instNum, pMonitor, &chainid) < 0)
		return ERR_9005;

	if (strcmp(lastname, "ParaList") == 0)
	{
		struct CWMP_LEAF *e;
		char *buf = data;

		if (strlen(buf) == 0 || get_ParameterEntity(buf, &e) < 0)
			return ERR_9007;

		strncpy(pMonitor->para, buf, PARA_NAME_LEN - 1);
		pMonitor->para[PARA_NAME_LEN] = 0;
		updateMonitorTimer(pMonitor);
		mib_chain_update(CWMP_CT_MONITOR_TBL, pMonitor, chainid);

		return 0;
	}
	else if (strcmp(lastname, "TimeList") == 0)
	{
		char *buf = data;
		int period = atoi(buf);

		if (period <= 0)
			return ERR_9007;

		pMonitor->period = period;
		updateMonitorTimer(pMonitor);
		mib_chain_update(CWMP_CT_MONITOR_TBL, pMonitor, chainid);

		return 0;
	} else {
		return ERR_9005;
	}

	return 0;
}

int objCT_MonitorEntity(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, num, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	CWMP_CT_MONITOR_T monitor, *pMonitor = &monitor;

	if (name == NULL || entity == NULL)
		return -1;

	num = mib_chain_total(CWMP_CT_MONITOR_TBL);

	switch (type) {
	case eCWMP_tINITOBJ:
		{
			struct CWMP_LINKNODE **c =
			    (struct CWMP_LINKNODE **)data;

			if (data == NULL)
				return -1;

			findMaxMonitorInstNum(&maxInstNum);

			for (i = 0; i < num; i++) {
				if (!mib_chain_get
				    (CWMP_CT_MONITOR_TBL, i, pMonitor))
					continue;

				if (pMonitor->InstanceNum == 0) {
					maxInstNum++;
					pMonitor->InstanceNum = maxInstNum;
					mib_chain_update(CWMP_CT_MONITOR_TBL,
							 pMonitor, i);
				}

				if (create_Object(c, tCT_MonitorEntityObject,
						  sizeof(tCT_MonitorEntityObject),
						  1, pMonitor->InstanceNum) < 0)
					return -1;
			}

			add_objectNum(name, maxInstNum);

			break;
		}
	case eCWMP_tADDOBJ:{
			if (data == NULL)
				return -1;

			ret =
			    add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_MonitorEntityObject,
				       sizeof(tCT_MonitorEntityObject), data);
			if (ret >= 0) {
				CWMP_CT_MONITOR_T entry;

				memset(&entry, 0, sizeof(entry));
				entry.InstanceNum = *(int *)data;
				insertAlarmTimer(FOR_MONITOR, entry.InstanceNum, 0);
				mib_chain_add(CWMP_CT_MONITOR_TBL, &entry);
			}

			break;
		}
	case eCWMP_tDELOBJ:{
			int found = 0;
			unsigned int *pUint = data;

			if (data == NULL)
				return -1;

			for (i = 0; i < num; i++) {
				if (!mib_chain_get
				    (CWMP_CT_MONITOR_TBL, i, pMonitor))
					continue;

				if (pMonitor->InstanceNum == *pUint) {
					found = 1;
					deleteAlarmTimerByInstnum(FOR_MONITOR, pMonitor->InstanceNum);
					mib_chain_delete(CWMP_CT_MONITOR_TBL, i);

					break;
				}
			}

			if (found) {
				ret =
				    del_Object(name,
					       (struct CWMP_LINKNODE **)
					       &entity->next, *(int *)data);
			} else {
				ret = ERR_9005;
			}

			break;
		}
	case eCWMP_tUPDATEOBJ:{
			struct CWMP_LINKNODE *old_table;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < num; i++) {
				struct CWMP_LINKNODE *remove_entity = NULL;

				if (!mib_chain_get
				    (CWMP_CT_MONITOR_TBL, i, pMonitor))
					continue;

				remove_entity =
				    remove_SiblingEntity(&old_table,
							 pMonitor->InstanceNum);

				if (remove_entity != NULL) {
					/* Tsai: it exists both in the MIB and the parameter tree */
					add_SiblingEntity((struct CWMP_LINKNODE
							   **)&entity->next,
							  remove_entity);
				} else {
					/* Tsai: it exists only in the MIB,
					 * so we add it into the parameter tree */
					add_Object(name,
						   (struct CWMP_LINKNODE **)
						   &entity->next,
						   tCT_MonitorEntityObject,
						   sizeof
						   (tCT_MonitorEntityObject),
						   &pMonitor->InstanceNum);
					mib_chain_update(CWMP_CT_MONITOR_TBL,
							 pMonitor, i);
				}
			}

			if (old_table) {
				/* Tsai: it exists only in the parameter tree,
				 * so we remove it from the parameter tree */
				destroy_ParameterTable((struct CWMP_NODE *)
						       old_table);
			}

			break;
		}
	}

	return ret;
}
#endif
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_ALARM_MONITOR_****************************************************************************/
/*************************************************************************************************************************/





/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_IPV6_****************************************************************************/
/*************************************************************************************************************************/
#ifdef _PRMT_X_CT_COM_IPv6_
struct CWMP_OP tCT_IPProtocolVersionLeafOP = { getCT_IPProtocolVersion, setCT_IPProtocolVersion };

struct CWMP_PRMT tCT_IPProtocolVersionLeafInfo[] = {
/*(name,		type,		flag,			op)*/
{"Mode",		eCWMP_tUINT,	CWMP_WRITE | CWMP_READ,	&tCT_IPProtocolVersionLeafOP},
};

enum eCT_IPProtocolVersionLeaf
{
	eCT_IPPVMode,
};

struct CWMP_LEAF tCT_IPProtocolVersionLeaf[] = {
{ &tCT_IPProtocolVersionLeafInfo[eCT_IPPVMode] },
{ NULL },
};

int getCT_IPProtocolVersion(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || type == NULL || data == NULL)
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, tCT_IPProtocolVersionLeafInfo[eCT_IPPVMode].name) == 0)
	{
		unsigned char mode;
		if (!mib_get(MIB_V6_IPV6_ENABLE, &mode))
			return ERR_9001;

		if(mode == 1)
			*data = uintdup(3);	//v4+v6
		else
			*data = uintdup(1);	//v4 only
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_IPProtocolVersion(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;

	if (name == NULL || entity == NULL || data == NULL)
		return -1;

	if (entity->info->type != type)
		return ERR_9006;

	if (strcmp(lastname, tCT_IPProtocolVersionLeafInfo[eCT_IPPVMode].name) == 0)
	{
		unsigned int *i;
		unsigned char mode;
		FILE *proc;

		i = (unsigned int *)data;
		mode = *i;

		if (mode < 1 || mode > 3)
			return ERR_9007;

		/* IPv4 should be always enabled,
		 * so only testing whether IPv6 should be disabled */
		if(mode == 2)
			fprintf(stderr, "<%s:%d> Cannot disable IPv4\n", __FUNCTION__, __LINE__);

		mode = (mode & 2) ? 1 : 0;

		mib_set(MIB_V6_IPV6_ENABLE, &mode);

		mode += '0';
		proc = fopen("/proc/sys/net/ipv6/conf/all/disable_ipv6", "w");
		if (proc)
		{
			fputc(mode, proc);
			fclose(proc);
		}
	}
	else {
		return ERR_9005;
	}

	return 0;
}
#endif
/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_IPV6_****************************************************************************/
/*************************************************************************************************************************/






/*************************************************************************************************************************/
/***********_PRMT_X_CT_COM_TIME_****************************************************************************/
/*************************************************************************************************************************/

/***** InternetGatewayDevice.Time. */
#ifdef _PRMT_X_CT_COM_TIME_
struct CWMP_OP tCTTimeLeafOP = { getCTTime, setCTTime };
struct CWMP_PRMT tCTTimeLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"X_CT-COM_NTPInterval",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tCTTimeLeafOP},
{"X_CT-COM_NTPServerType",	eCWMP_tUINT,	CWMP_READ|CWMP_WRITE,	&tCTTimeLeafOP},
};

int getCTTime(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "X_CT-COM_NTPInterval" )==0 )
	{
		unsigned int interval;

		mib_get(MIB_NTP_INTERVAL, &interval);

		*data = uintdup(interval);
	}
	else if( strcmp( lastname, "X_CT-COM_NTPServerType" )==0 )
	{
		unsigned char type;

		mib_get(MIB_NTP_IF_TYPE, &type);
		*data = uintdup(type);
	}
	else{
		return ERR_9005;
	}

	return 0;
}


int setCTTime(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	int vInt;

	if (name == NULL || entity == NULL) return -1;
	if (data == NULL) return ERR_9007;
	if (entity->info->type != type) return ERR_9006;

	if (strcmp(lastname, "X_CT-COM_NTPInterval") == 0)
	{
		unsigned int interval;

		interval = *(unsigned int *)data;

		if(interval <= 0)
			return ERR_9007;

		mib_set(MIB_NTP_INTERVAL, &interval);
		apply_add( CWMP_PRI_N, apply_NTP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if(strcmp(lastname, "X_CT-COM_NTPServerType") == 0)
	{
		unsigned int *i = data;
		unsigned char type;

		type = *i;

		if(type < 0 || type > 3)
			return ERR_9007;

		mib_set(MIB_NTP_IF_TYPE, &type);
		apply_add( CWMP_PRI_N, apply_NTP, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}

#endif

/*************************************************************************************************************************/
/***********END _PRMT_X_CT_COM_TIME_****************************************************************************/
/*************************************************************************************************************************/




/*************************************************************************************************************************/
/*********** CTC_DNS_SPEED_LIMIT ****************************************************************************/
/*************************************************************************************************************************/
#ifdef CTC_DNS_SPEED_LIMIT
/***** InternetGatewayDevice.X_CT-COM_DNSSpeedLimit. */

struct CWMP_OP tCTDnsSpeedLimitOP = { getCTDnsSpeedLimit, setCTDnsSpeedLimit };
struct CWMP_PRMT tCT_DnsSpeedLimitLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Domain",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTDnsSpeedLimitOP},
{"LimitAction",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCTDnsSpeedLimitOP},
{"HgwInfo", 		eCWMP_tSTRING,	CWMP_READ,	&tCTDnsSpeedLimitOP},
{"DeviceInfo",	eCWMP_tSTRING,	CWMP_READ,	&tCTDnsSpeedLimitOP},
};

enum eCT_DnsSpeedLimitLeaf
{
	eCT_Domain,
	eCT_LimitAction,
	eCT_HgwInfo,
	eCT_DeviceInfo,
};
struct CWMP_LEAF tCT_DnsSpeedLimitLeaf[] =
{
{ &tCT_DnsSpeedLimitLeafInfo[eCT_Domain] },
{ &tCT_DnsSpeedLimitLeafInfo[eCT_LimitAction] },
{ &tCT_DnsSpeedLimitLeafInfo[eCT_HgwInfo] },
{ &tCT_DnsSpeedLimitLeafInfo[eCT_DeviceInfo] },
{ NULL },
};


int getCTDnsSpeedLimit(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if( strcmp( lastname, "Domain" )==0 )
	{
		unsigned char all;
		int total = mib_chain_total(DNS_LIMIT_DOMAIN_TBL);

		if(total > 0)
		{
			int i;
			MIB_CE_DNS_LIMIT_DOMAIN_T entry = {0};
			char buf[1024] = {0};

			for(i = 0 ; i < total ; i++)
			{
				mib_chain_get(DNS_LIMIT_DOMAIN_TBL, i, &entry);
				if (i == 0)
					sprintf(buf, "%s/%u", entry.domain, entry.limit);
				else
					sprintf(buf, "%s,%s/%u", buf, entry.domain, entry.limit);
			}

			*data = strdup(buf);
		}
		else
			*data = strdup("NULL");
	}
	else if( strcmp( lastname, "LimitAction" )==0 )
	{
		unsigned char action;

		mib_get(MIB_DNS_LIMIT_ACTION, &action);

		if(action == DNS_LIMIT_ACTION_ALERT)
			*data = strdup("Alert");
		else
			*data = strdup("Drop");
	}
	else if( strcmp( lastname, "HgwInfo" )==0 )
	{
		int total = mib_chain_total(MIB_ATM_VC_TBL);
		int i;
		MIB_CE_ATM_VC_T entry;
		struct in_addr inAddr;
		struct sockaddr sa;
		int flags = 0;
		char ifname[IFNAMSIZ] = {0};
		char phy_ifname[IFNAMSIZ] = {0};
		unsigned char mac[MAC_ADDR_LEN] = {0};
		char buf[256] = {0};

		for(i = 0 ; i < total ; i++)
		{
			if(mib_chain_get(MIB_ATM_VC_TBL, i, &entry)!=1)
				continue;

			if(entry.applicationtype & X_CT_SRV_INTERNET && entry.cmode != CHANNEL_MODE_BRIDGE)
			{
			
				if(ifGetName( entry.ifIndex, ifname, IFNAMSIZ) == 0
					|| ifGetName( PHY_INTF(entry.ifIndex), phy_ifname, IFNAMSIZ) == 0)
					continue;

				if (getInFlags( ifname, &flags) == 1 && flags & IFF_UP)
				{
					if (getInAddr(ifname, IP_ADDR, (void *)&inAddr) && getInAddr(phy_ifname, HW_ADDR, (void *)&sa))
					{
						memcpy( mac, sa.sa_data, MAC_ADDR_LEN );
						sprintf(buf, "%s|%02x:%02x:%02x:%02x:%02x:%02x", inet_ntoa(inAddr), mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
						break;
					}
				}
			}
		}
		
		*data = strdup(buf);
	}
	else if( strcmp( lastname, "DeviceInfo" )==0 )
	{
		int total = mib_chain_total(DNS_LIMIT_DEV_INFO_TBL);
		char buf[2048] = {0}, *p=buf;
		const char *ret = NULL;
		int i;
		MIB_CE_DNS_LIMIT_DEV_INFO_T entry = {0};
		char addr[MAX_V6_IP_LEN] = {0}, mac[20] = {0};

		for(i = 0 ; i < total ; i++)
		{
			if(mib_chain_get(DNS_LIMIT_DEV_INFO_TBL, i, &entry) == 0)
				continue;

			if(entry.ip_ver == IPVER_IPV4)
				ret = inet_ntop(AF_INET, (struct in_addr *)entry.ip_addr, addr, sizeof(addr));
			else
				ret = inet_ntop(AF_INET6, (struct in6_addr *)entry.ip6_addr, addr, sizeof(addr));

			if(ret == NULL)
			{
				DBPRINT1("Convert IP address failed, i=%d\n", i);
				continue;
			}

			sprintf(mac, "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", entry.mac[0]
				, entry.mac[1], entry.mac[2], entry.mac[3], entry.mac[4], entry.mac[5]);

			p += sprintf(p, "%s%s/%s/%s", (i == 0) ? "" : ",", entry.domain, addr, mac);
		}

	//	fprintf(stderr, "<%s:%d> buf= %s\n", __FUNCTION__, __LINE__, buf);
		*data = strdup(buf);
	}
	else
	{
		return ERR_9005;
	}

	return 0;
}

int setCTDnsSpeedLimit(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	int total, i;

	fprintf(stderr, "enter %s\n", __FUNCTION__);

	if (name == NULL || entity == NULL)
		return -1;

	if (data == NULL)
	{
		fprintf(stderr, "data is NULL\n");
		return ERR_9007;
	}

	if (entity->info->type != type)
		return ERR_9006;

	if( strcmp( lastname, "Domain" )==0 )
	{
		char *buf = data;

		mib_chain_clear(DNS_LIMIT_DOMAIN_TBL);

		if(strstr(buf, "NULL") == NULL)
		{
			char *record = NULL;
			char *tmp = NULL;
			MIB_CE_DNS_LIMIT_DOMAIN_T entry = {0};

			while((record = strtok_r(buf, ",", &tmp)) != NULL)
			{
				CWMPDBP2("Parsing record = %s\n", record);

				if( sscanf(record, "%[^/]/%u", entry.domain, &entry.limit) != 2)
					return ERR_9007;

				mib_chain_add(DNS_LIMIT_DOMAIN_TBL, &entry);
				buf = NULL;
			}
		}

		//reset DeviceInfo
		mib_chain_clear(DNS_LIMIT_DEV_INFO_TBL);

		// restart dnsmasq to reset query statistics
		apply_add( CWMP_PRI_N, apply_DNS, CWMP_RESTART, 0, NULL, 0 );
		return 0;
	}
	else if( strcmp( lastname, "LimitAction" )==0 )
	{
		char *buf = data;
		unsigned char action;

		if(strcmp(buf, "Alert") == 0)
			action = DNS_LIMIT_ACTION_ALERT;
		else if(strcmp(buf, "Drop") == 0)
			action = DNS_LIMIT_ACTION_DROP;
		else
			return ERR_9007;

		mib_set(MIB_DNS_LIMIT_ACTION, &action);
		return 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}
#endif

/*************************************************************************************************************************/
/***********END CTC_DNS_SPEED_LIMIT****************************************************************************/
/*************************************************************************************************************************/



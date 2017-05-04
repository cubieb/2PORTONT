#include <rtk/utility.h>
#include <parameter_api.h>

#include "prmt_lanconfsec.h"


/******LANConfigSecurity***************************************************************************/
struct CWMP_OP tLANConfigSecurityLeafOP = { getLANConfSec, setLANConfSec};
struct CWMP_PRMT tLANConfigSecurityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"ConfigPassword",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tLANConfigSecurityLeafOP },
};
enum eLANConfigSecurityLeaf
{
	eConfigPassword,
};
struct CWMP_LEAF tLANConfigSecurityLeaf[] =
{
{ &tLANConfigSecurityLeafInfo[eConfigPassword] },
{ NULL	}
};

int getLANConfSec(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "ConfigPassword" )==0 )
	{
#if 1
#ifdef CONFIG_MIDDLEWARE
		unsigned char vChar;
		char buf[250];
		mib_get(CWMP_TR069_ENABLE,(void *)&vChar);
		if(vChar == 0){
			mib_get( CWMP_LAN_CONFIGPASSWD, (void *)buf);
			*data = strdup(buf);
		}else
#endif	//end of CONFIG_MIDDLEWARE
		*data = strdup( "" ); /*return an empty string*/
#else
		char buf[65];
	#ifdef TELEFONICA_DEFAULT_CFG
		mib_get(MIB_SUSER_PASSWORD, (void *)buf);
	#else
		mib_get(CWMP_LAN_CONFIGPASSWD, (void *)buf);
	#endif //TELEFONICA_DEFAULT_CFG
		*data = strdup( buf );
#endif
	}else{
		return ERR_9005;
	}

	return 0;
}

int setLANConfSec(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "ConfigPassword" )==0 )
	{
#ifdef TELEFONICA_DEFAULT_CFG
		char *buf=data;
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9001;
		if(strlen(buf)>=MAX_NAME_LEN) return ERR_9001;
		mib_set(MIB_SUSER_PASSWORD, (void *)buf);

		apply_add( CWMP_PRI_N, apply_UserAccount, CWMP_RESTART, 0, NULL, 0 );
		return 0;
#else
		char *buf=data;
		int  len=0;
		if( buf ) len = strlen( buf );
		if( len == 0 )
			mib_set( CWMP_LAN_CONFIGPASSWD, (void *)"");
		else if( len < 64 )
			mib_set( CWMP_LAN_CONFIGPASSWD, (void *)buf);
		else
			return ERR_9007;

		return 0;
#endif //TELEFONICA_DEFAULT_CFG
	}else{
		return ERR_9005;
	}

	return 0;

}



#include "prmt_userif.h"

#ifdef _PRMT_USERINTERFACE_


struct CWMP_OP tUserIFLeafOP = { getUserIF, setUserIF };
struct CWMP_PRMT tUserIFLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PasswordRequired",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tUserIFLeafOP},
{"PasswordUserSelectable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tUserIFLeafOP},
{"UpgradeAvailable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tUserIFLeafOP},
{"WarrantyDate",		eCWMP_tDATETIME,CWMP_WRITE|CWMP_READ,	&tUserIFLeafOP},
{"AutoUpdateServer",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tUserIFLeafOP},
{"UserUpdateServer",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tUserIFLeafOP}
};

enum eUserIFLeaf
{
	ePasswordRequired,
	ePasswordUserSelectable,
	eUpgradeAvailable,
	eWarrantyDate,
	eAutoUpdateServer,
	eUserUpdateServer
};

struct CWMP_LEAF tUserIFLeaf[] =
{
{ &tUserIFLeafInfo[ePasswordRequired] },
{ &tUserIFLeafInfo[ePasswordUserSelectable] },
{ &tUserIFLeafInfo[eUpgradeAvailable] },
{ &tUserIFLeafInfo[eWarrantyDate] },
{ &tUserIFLeafInfo[eAutoUpdateServer] },
{ &tUserIFLeafInfo[eUserUpdateServer] },
{ NULL }
};


int getUserIF(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	unsigned int vUint=0;
	char buff[256]={0};
	
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "PasswordRequired" )==0 )
	{
		mib_get( UIF_PW_REQUIRED, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "PasswordUserSelectable" )==0 )
	{
		mib_get( UIF_PW_USER_SEL, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "UpgradeAvailable" )==0 )
	{
		mib_get( UIF_UPGRADE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, "WarrantyDate" )==0 )
	{
		mib_get( UIF_WARRANTYDATE, (void *)&vUint);
		*data = timedup( vUint );
	}else if( strcmp( lastname, "AutoUpdateServer" )==0 )
	{
		mib_get( UIF_AUTOUPDATESERVER, (void *)buff);
		*data = strdup( buff );
	}else if( strcmp( lastname, "UserUpdateServer" )==0 )
	{
		mib_get( UIF_USERUPDATESERVER, (void *)buff);
		*data = strdup( buff );
	}else{
		return ERR_9005;
	}
	
	return 0;
}

int setUserIF(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;
	
	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "PasswordRequired" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(UIF_PW_REQUIRED, (void *)&vChar);
		return 0;
	}else if( strcmp( lastname, "PasswordUserSelectable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(UIF_PW_USER_SEL, (void *)&vChar);
		return 0;
	}else if( strcmp( lastname, "UpgradeAvailable" )==0 )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(UIF_UPGRADE, (void *)&vChar);
		return 0;
	}else if( strcmp( lastname, "WarrantyDate" )==0 )
	{
		unsigned int *i = data;
		if( i==NULL ) return ERR_9007;
		mib_set(UIF_WARRANTYDATE, (void *)i);
		return 0;
	}else if( strcmp( lastname, "AutoUpdateServer" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strlen(buf)>=256 ) return ERR_9007;
		mib_set(UIF_AUTOUPDATESERVER, (void *)buf);		
		return 0;
	}else if( strcmp( lastname, "UserUpdateServer" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if( strlen(buf)>=256 ) return ERR_9007;
		mib_set(UIF_USERUPDATESERVER, (void *)buf);		
		return 0;
	}else{
		return ERR_9005;
	}
	
	return 0;
}

#endif /*_PRMT_USERINTERFACE_*/


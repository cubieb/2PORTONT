#include "prmt_captiveportal.h"

#ifdef _SUPPORT_CAPTIVEPORTAL_PROFILE_
struct CWMP_OP tCaptivePortalLeafOP = { getCaptivePortal,	setCaptivePortal };
struct CWMP_PRMT tCaptivePortalLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCaptivePortalLeafOP },
{"Status",			eCWMP_tSTRING,	CWMP_READ,		&tCaptivePortalLeafOP },
{"AllowedList",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCaptivePortalLeafOP },
{"CaptivePortalURL",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCaptivePortalLeafOP },
};
enum eCaptivePortalLeaf
{
	eCP_Enable,
	eCP_Status,
	eCP_AllowedList,
	eCP_CaptivePortalURL,
};
struct CWMP_LEAF tCaptivePortalLeaf[] =
{
{ &tCaptivePortalLeafInfo[eCP_Enable] },
{ &tCaptivePortalLeafInfo[eCP_Status] },
{ &tCaptivePortalLeafInfo[eCP_AllowedList] },
{ &tCaptivePortalLeafInfo[eCP_CaptivePortalURL] },
{ NULL	}
};
/*************************************************************************************/
int getCaptivePortal(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar;
	unsigned char buf[256+1]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, tCaptivePortalLeafInfo[eCP_Enable].name )==0 )
	{
		mib_get( MIB_CAPTIVEPORTAL_ENABLE, (void *)&vChar);
		*data = booldup( vChar!=0 );
	}else if( strcmp( lastname, tCaptivePortalLeafInfo[eCP_Status].name )==0 )
	{
		mib_get( MIB_CAPTIVEPORTAL_ENABLE, (void *)&vChar);
		if(vChar!=0)
		{
			mib_get( MIB_CAPTIVEPORTAL_URL, (void *)buf);
			if( strlen(buf)==0 )
				*data = strdup( "Error_URLEmpty" );
			else
				*data = strdup( "Enabled" );
		}else
			*data = strdup( "Disabled" );
	}else if( strcmp( lastname, tCaptivePortalLeafInfo[eCP_AllowedList].name )==0 )
	{
		getCaptivePortalAllowedList( FILE4CaptivePortal );
		*type = eCWMP_tFILE;
		*data = strdup( FILE4CaptivePortal );
	}else if( strcmp( lastname, tCaptivePortalLeafInfo[eCP_CaptivePortalURL].name )==0 )
	{
		mib_get( MIB_CAPTIVEPORTAL_URL, (void *)buf);
		*data = strdup( buf );
	}else{
		return ERR_9005;
	}

	return 0;
}
int setCaptivePortal(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	int *pINT=data;
	char *pbuf = data;
	unsigned char vChar;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, tCaptivePortalLeafInfo[eCP_Enable].name )==0 )
	{
		unsigned char last_enable;
		if(pINT==NULL) return ERR_9007;
		//fprintf(stderr, "set %s=%d\n", name, (*pINT==0)?0:1 );
		vChar=(*pINT==0)?0:1;
		mib_get(MIB_CAPTIVEPORTAL_ENABLE, (void *)&last_enable);
		mib_set(MIB_CAPTIVEPORTAL_ENABLE, (void *)&vChar);

		if(!last_enable && vChar)
			apply_add(CWMP_PRI_L, Apply_CaptivePortal, CWMP_START, 0, NULL, 0);
		else if(last_enable && !vChar)
			apply_add(CWMP_PRI_L, Apply_CaptivePortal, CWMP_STOP, 0, NULL, 0);

		return 0;
	}else if( strcmp( lastname, tCaptivePortalLeafInfo[eCP_AllowedList].name )==0 )
	{
		//fprintf(stderr, "set %s=%s\n", name, pbuf?pbuf:""  );
		if( setCaptivePortalAllowedList( pbuf )<0 ) return ERR_9007;

		mib_get(MIB_CAPTIVEPORTAL_ENABLE, (void *)&vChar);
		if(vChar)
			apply_add(CWMP_PRI_L, Apply_CaptivePortal, CWMP_RESTART, 0, NULL, 0);

		return 0;
	}else if( strcmp( lastname, tCaptivePortalLeafInfo[eCP_CaptivePortalURL].name )==0 )
	{
		//fprintf(stderr, "set %s=%s\n", name, pbuf?pbuf:"" );
		if(pbuf==NULL || strlen(pbuf)==0 )
			mib_set( MIB_CAPTIVEPORTAL_URL, (void *)"");
		else{
			if( (strlen(pbuf)>=MAX_URL_LEN) || strncmp( pbuf, "http://", 7 ) ) return ERR_9007;
			mib_set( MIB_CAPTIVEPORTAL_URL, (void *)pbuf);
		}
		return 0;
	}else{
		return ERR_9005;
	}

	return 0;

}

/***********************************************************************/
/*API*/
/***********************************************************************/
int getCaptivePortalAllowedList( char *pfilename )
{
	FILE *fp;
	int num,i;

	if(!pfilename) return -1;
	fp=fopen( pfilename, "w" );
	if(!fp) return -1;
	num = mib_chain_total( CWMP_CAPTIVEPORTAL_ALLOWED_LIST );
	for( i=0;i<num;i++ )
	{
		CWMP_CAPTIVEPORTAL_ALLOWED_LIST_T cpal_entry;
		if(!mib_chain_get( CWMP_CAPTIVEPORTAL_ALLOWED_LIST, i, (void*)&cpal_entry ))
			continue;
		//fprintf( stderr, "(get:%d:%d:%d:%d/%d)\n", cpal_entry.ip_addr[0], cpal_entry.ip_addr[1], cpal_entry.ip_addr[2], cpal_entry.ip_addr[3], cpal_entry.mask );
		//fprintf( stderr, "(get:%s/%u)\n", inet_ntoa(*((struct in_addr *)&(cpal_entry.ip_addr))),  cpal_entry.mask );

		if(i>0) fprintf( fp, "," );
		fprintf( fp, "%s",  inet_ntoa(*((struct in_addr *)&(cpal_entry.ip_addr))) );
		if(cpal_entry.mask!=CP_MASK_DONOT_CARE) fprintf( fp, "/%u", cpal_entry.mask);
	}
	fclose(fp);
	return 0;
}

int setCaptivePortalAllowedList( char *list )
{
	FILE *fp;
	int num,i, count=0;
	char buf[32];
	struct in_addr	inaddr;

	fp=fopen( FILE4CaptivePortal, "w" );
	if(!fp) return -1;
	if( list && strlen(list) )
	{
		char *tok;
		tok=strtok( list, ", " );
		while(tok)
		{
			char *p, *pmask=NULL;
			int paser_error=0;
			p=strchr( tok, '/' );
			if(p)
			{
				*p=0;
				p++;
				pmask=p;
				if(strlen(p)>0)
				{
					while(*p!=0)
					{
						if(isdigit(*p)==0)
						{
							paser_error=1;
							break;
						}
						p++;
					}
					if( paser_error==0 && (atoi(pmask)<0 || atoi(pmask)>32) )
						paser_error=1;
				}else
					paser_error=1;
			}

			if(paser_error==0 && (inet_aton(tok, &inaddr)==0) )
				paser_error=1;

			if(paser_error)
			{
				fclose(fp);
				unlink(FILE4CaptivePortal);
				return -1;
			}

			if(pmask)
				fprintf( fp, "%s %s\n", tok, pmask );
			else
				fprintf( fp, "%s %d\n", tok, CP_MASK_DONOT_CARE );

			//next
			count++;
			tok=strtok( NULL, ", " );
		}
	}
	fclose(fp);
	if(count>MAX_ALLOWED_LIST)
	{
		unlink(FILE4CaptivePortal);
		return -1;
	}

	//destroy allowlist
	num = mib_chain_total( CWMP_CAPTIVEPORTAL_ALLOWED_LIST );
	for( i=num-1;i>=0;i-- ) mib_chain_delete( CWMP_CAPTIVEPORTAL_ALLOWED_LIST, i );

	//save the new allowlist
	fp=fopen( FILE4CaptivePortal, "r" );
	if(!fp) return -1;
	while(fgets(buf, 32, fp))
	{
		char ipbuf[32];
		int ipmask;
		CWMP_CAPTIVEPORTAL_ALLOWED_LIST_T cpal_entry;

		//fprintf( stderr, "set:buf=%s\n", buf );
		sscanf( buf, "%s %d", ipbuf, &ipmask );
		inet_aton(ipbuf, (struct in_addr *)cpal_entry.ip_addr);
		cpal_entry.mask=(unsigned char)ipmask;
		//fprintf( stderr, "set:ipbuf=%s(%d,%d,%d,%d), ipmask=%d\n", ipbuf, ipmask, cpal_entry.ip_addr[0], cpal_entry.ip_addr[1], cpal_entry.ip_addr[2], cpal_entry.ip_addr[3] );
		mib_chain_add( CWMP_CAPTIVEPORTAL_ALLOWED_LIST, (unsigned char*)&cpal_entry);
	}
	fclose(fp);

	unlink(FILE4CaptivePortal);
	return 0;
}

#endif //_SUPPORT_CAPTIVEPORTAL_PROFILE_


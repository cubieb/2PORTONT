#include "prmt_time.h"
#ifdef _PRMT_X_CT_COM_TIME_
#include "prmt_ctcom.h"
#endif

#ifdef TIME_ZONE

struct CWMP_OP tTimeLeafOP = { getTime,	setTime };
struct CWMP_PRMT tTimeLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"Enable",			eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tTimeLeafOP}, /*_PRMT_WT107_*/
{"X_REALTEK_Enable",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tTimeLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"Status",			eCWMP_tSTRING,	CWMP_READ,	&tTimeLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"NTPServer1",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tTimeLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{"NTPServer2",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tTimeLeafOP},
#endif
/*ping_zhang:20081217 END*/
{"CurrentLocalTime",		eCWMP_tDATETIME,CWMP_READ|CWMP_DENY_ACT,&tTimeLeafOP},
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifndef _PRMT_WT107_
{"LocalTimeZone",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tTimeLeafOP},
#else
{"LocalTimeZoneName",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tTimeLeafOP},
#endif
/*ping_zhang:20081217 END*/
};
enum eTimeLeaf
{
	eEnable, /*_PRMT_WT107_*/
	eTX_REALTEK_Enable,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eTStatus,
#endif
/*ping_zhang:20081217 END*/
	eTNTPServer1,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	eTNTPServer2,
#endif
/*ping_zhang:20081217 END*/
	eTCurrentLocalTime,
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifndef _PRMT_WT107_
	eTLocalTimeZone,
#else
	eTLocalTimeZoneName,
#endif
/*ping_zhang:20081217 END*/
};
struct CWMP_LEAF tTimeLeaf[] =
{
{ &tTimeLeafInfo[eEnable] }, /*_PRMT_WT107_*/
{ &tTimeLeafInfo[eTX_REALTEK_Enable] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tTimeLeafInfo[eTStatus] },
#endif
/*ping_zhang:20081217 END*/
{ &tTimeLeafInfo[eTNTPServer1] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
{ &tTimeLeafInfo[eTNTPServer2] },
#endif
/*ping_zhang:20081217 END*/
{ &tTimeLeafInfo[eTCurrentLocalTime] },
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifndef _PRMT_WT107_
{ &tTimeLeafInfo[eTLocalTimeZone] },
#else
{ &tTimeLeafInfo[eTLocalTimeZoneName] },
#endif
/*ping_zhang:20081217 END*/

#ifdef _PRMT_X_CT_COM_TIME_
{ &tCTTimeLeafInfo[eCT_NTPInterval] },
{ &tCTTimeLeafInfo[eCT_NTPServerType] },
#endif
{ NULL }
};

int getTime(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned char vChar=0;
	struct in_addr ipAddr;
	char buff[256]={0};

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, "X_REALTEK_Enable" )==0
	    || strcmp( lastname, "Enable" )==0 /*_PRMT_WT107_*/
	  )
	{
		mib_get( MIB_NTP_ENABLED, (void *)&vChar);
		*data = booldup( vChar!=0 );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "Status" )==0 )
	{
		unsigned int timeStatus = eTStatusError;
		mib_get( MIB_NTP_ENABLED, (void *)&vChar);
		if(vChar==0)
		 	*data=strdup( "Disabled" );
		else
		{
			FILE *fp;
			fp=fopen("/tmp/timeStatus","r");
			if(fp){
				fscanf(fp,"%d",&timeStatus);
				fclose(fp);
				//unlink("/tmp/timeStatus");
			}
			switch(timeStatus)
			{
			case eTStatusDisabled:
				*data=strdup( "Disabled" );
				break;
			case eTStatusUnsynchronized:
				*data=strdup( "Unsychronized" );
				break;
			case eTStatusSynchronized:
				*data=strdup( "Synchronized" );
				break;
			case eTStatusErrorFailed:
				*data=strdup( "Error_FailedToSynchronize" );
				break;
			case eTStatusError:
			default:
				*data=strdup( "Error" );
				break;
			}
		}
#endif
/*ping_zhang:20081217 END*/
	}else if( strcmp( lastname, "NTPServer1" )==0 )
	{
#ifdef CONFIG_E8B
		mib_get(MIB_NTP_SERVER_HOST1, (void *)buff);
#else
		mib_get( MIB_NTP_SERVER_ID, (void *)&vChar);
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
		if( vChar==0 ) /*select from a list*/{
			mib_get(MIB_NTP_SERVER_HOST1, (void *)buff);
		}
		else /*manual setting*/
			mib_get(MIB_NTP_SERVER_HOST2, (void *)buff);
/*ping_zhang:20081217 END*/
#endif

                *data=strdup( buff );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "NTPServer2" )==0 )
	{
#ifdef CONFIG_E8B
		mib_get(MIB_NTP_SERVER_HOST2, (void *)buff);
#else
		mib_get( MIB_NTP_SERVER_ID, (void *)&vChar);
		if( vChar==0 ) /*select from a list*/
			buff[0]=0;
		else /*manual setting*/
			mib_get(MIB_NTP_SERVER_HOST1, (void *)buff);
#endif

		 *data=strdup( buff );
#endif
/*ping_zhang:20081217 END*/
	}else if( strcmp( lastname, "CurrentLocalTime" )==0 )
	{
		*data = timedup( time(NULL) );
/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifndef _PRMT_WT107_
	}else if( strcmp( lastname, "LocalTimeZone" )==0 )
	{
		unsigned int index = 0;

		if (mib_get(MIB_NTP_TIMEZONE_DB_INDEX, &index))
			*data = strdup(get_tz_utc_offset(index));
		else
			*data = strdup("");
#else
	}else if( strcmp( lastname, "LocalTimeZoneName" )==0 )
	{
		/* example:EST+5EDT,M4.1.0/2,M10.5.0/2 */
		unsigned int index = 0;
		unsigned char dst_enabled = 1;

		if (mib_get(MIB_NTP_TIMEZONE_DB_INDEX, &index)) {
			mib_get(MIB_DST_ENABLED, &dst_enabled);
			*data = strdup(get_tz_string(index, dst_enabled));
		} else {
			*data = strdup("");
		}
#endif
/*ping_zhang:20081217 END*/
	}else{
		return ERR_9005;
	}

	return 0;
}

int setTime(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	char	*buf=data;
	unsigned char vChar=0;

	if( (name==NULL) || (data==NULL) || (entity==NULL)) return -1;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, "X_REALTEK_Enable" )==0
	    || strcmp( lastname, "Enable" )==0 /*_PRMT_WT107_*/
	  )
	{
		int *i = data;

		if( i==NULL ) return ERR_9007;
		vChar = (*i==0)?0:1;
		mib_set(MIB_NTP_ENABLED, (void *)&vChar);

		apply_add( CWMP_PRI_N, apply_NTP, CWMP_RESTART, 0, NULL, 0 );
	}else if( strcmp( lastname, "NTPServer1" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9007;

#ifdef CONFIG_E8B
		mib_set(MIB_NTP_SERVER_HOST1, (void *)buf);
#else
/*ping_zhang:20081017 START:telefonica tr069 new request: new parameter of WT-107*/
		//always set to "Manual IP Setting"
		vChar=1;
		mib_set( MIB_NTP_SERVER_ID, (void *)&vChar);
		mib_set(MIB_NTP_SERVER_HOST2, (void *)buf);
/*ping_zhang:20081017 END*/
#endif
		apply_add( CWMP_PRI_N, apply_NTP, CWMP_RESTART, 0, NULL, 0 );

/*ping_zhang:20081217 START:patch from telefonica branch to support WT-107*/
#ifdef _PRMT_WT107_
	}else if( strcmp( lastname, "NTPServer2" )==0 )
	{
		if(buf==NULL) return ERR_9007;
		if(strlen(buf)==0) return ERR_9007;

#ifdef CONFIG_E8B
		mib_set(MIB_NTP_SERVER_HOST2, (void *)buf);
#else
		//always set to "Manual IP Setting"
		vChar=1;
		mib_set( MIB_NTP_SERVER_ID, (void *)&vChar);
		mib_set(MIB_NTP_SERVER_HOST1, (void *)buf);
#endif
		apply_add( CWMP_PRI_N, apply_NTP, CWMP_RESTART, 0, NULL, 0 );
#endif
/*ping_zhang:20081217 END*/
/*ping_zhang:20081217 START:telefonica tr069 new request: new parameter of WT-107*/
#ifndef _PRMT_WT107_
	}else if( strcmp( lastname, "LocalTimeZone" )==0 )
 	{
		int hh = 0, mm = 0, i;
 		char tmp[16];

		if (buf == NULL)
			return ERR_9007;
		if ((strlen(buf) == 0) || (strlen(buf) > 6))
			return ERR_9007;
				//format: +/-hh:mm (length==6)
		if (sscanf(buf, "%d:%d", &hh, &mm) == 2) {
			if (hh < -12 || hh > 12 || mm < 0 || mm > 59)
				return ERR_9007;
			sprintf(tmp, "%+03d:%02d", hh, mm);

			for (i = 0; i < nr_tz; i++) {
				if (strncmp(tmp,
					get_tz_utc_offset(i), 6) == 0)
					break;
			}

			if (i == nr_tz)
				return ERR_9007;

			mib_set(MIB_NTP_TIMEZONE_DB_INDEX, &i);
		} else
 			return ERR_9007;
		apply_add(CWMP_PRI_N, apply_NTP, CWMP_RESTART, 0, NULL, 0);
#else
 	}else if( strcmp( lastname, "LocalTimeZoneName" )==0 )
 	{
		int i;
		unsigned char dst_enabled = 1;

		if (buf == NULL)
			return ERR_9007;
		if (strlen(buf) == 0)
			return ERR_9007;

		mib_get(MIB_DST_ENABLED, &dst_enabled);
		for (i = 0; i < nr_tz; i++) {
			if (strcmp(buf, get_tz_string(i, dst_enabled)) == 0)
				break;
		}

		if (i == nr_tz)
			return ERR_9007;

		mib_set(MIB_NTP_TIMEZONE_DB_INDEX, &i);
		apply_add(CWMP_PRI_N, apply_NTP, CWMP_RESTART, 0, NULL, 0);
#endif
/*ping_zhang:20081217 END*/
	}else{
		return ERR_9005;
	}

	return 0;
}

#endif /*TIME_ZONE*/

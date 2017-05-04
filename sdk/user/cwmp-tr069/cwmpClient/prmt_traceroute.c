#include "prmt_traceroute.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#ifdef _SUPPORT_TRACEROUTE_PROFILE_

#define	ICMP_UNREACH_NET	        0	/* bad net */
#define	ICMP_UNREACH_HOST	        1	/* bad host */
#define	ICMP_UNREACH_PROTOCOL	        2	/* bad protocol */
#define	ICMP_UNREACH_PORT	        3	/* bad port */
#define	ICMP_UNREACH_NEEDFRAG	        4	/* IP_DF caused drop */
#define	ICMP_UNREACH_SRCFAIL	        5	/* src route failed */

enum eTR098_TraceRouteDiagState
{
	eTRSTATE_None,
	eTRSTATE_Requested,
	eTRSTATE_Complete,
	eTRSTATE_Error_CannotResolveHostName,
	eTRSTATE_Error_MaxHopCountExceeded,
	eTRSTATE_Error_Other,
};
char *strTraceRouteDiagState[] =
{
	"None",
	"Requested",
	"Complete",
	"Error_CannotResolveHostName",
	"Error_MaxHopCountExceeded",
	"Error_Other",
};
struct TR098_TraceRouteDiag
{
	int			DiagnosticsState;
	char			*pInterface;
	char			IfName[32];
	char			*pHost;
	unsigned int		NumberOfTries;
	unsigned int		Timeout; /*milliseconds*/
	unsigned int		DataBlockSize;
	unsigned int		DSCP;
	unsigned int		MaxHopCount;
	unsigned int		ResponseTime;
	unsigned int		RouteHopsNumberOfEntries;
	unsigned long int	traceroute_pid;
};
struct TR098_TraceRouteDiag gTraceRouteDiag =
{
	eTRSTATE_None,		/*DiagnosticsState*/
	NULL,			/*pInterface*/
	"", 			/*IfName*/
	NULL,			/*pHost*/
	3,			/*NumberOfTries*/
	5000,/*milliseconds*/	/*Timeout*/
	38,			/*DataBlockSize*/
	0,			/*DSCP*/
	30,			/*MaxHopCount*/
	0,			/*ResponseTime*/
	0,			/*RouteHopsNumberOfEntries*/
	0,			/*traceroute_pid*/
};

/******API********************************************************************/
int gStartTraceRouteDiag = 0;
void checkPidforTraceRouteDiag( pid_t  pid )
{
	int st;

	if(gTraceRouteDiag.traceroute_pid > 0)
		pid = waitpid(gTraceRouteDiag.traceroute_pid, &st, WNOHANG);

	if(pid > 0)
	// if(pid!=-1)
	{
		if(pid==gTraceRouteDiag.traceroute_pid)
		{
			FILE *fp;
			char line[256];
			int i;

			gTraceRouteDiag.traceroute_pid=0;
			//update the result
			gTraceRouteDiag.DiagnosticsState=eTRSTATE_Complete;

			if(fp = fopen("/var/restime", "r"))
			{
				fscanf(fp, "%d", &gTraceRouteDiag.ResponseTime);
				fclose(fp);
				unlink("/var/restime");
			}
			else
				fprintf(stderr, "Read response time file failed!\n");

			if(fp = fopen("/var/traceroute.log", "r"))
			{
				for( i = -1; fgets(line, 256, fp); i++ )
				{
					if(strstr(line, "traceroute:"))
					{
						gTraceRouteDiag.DiagnosticsState=eTRSTATE_Error_Other;
						i = 0;
						break;
					}
				}

				gTraceRouteDiag.RouteHopsNumberOfEntries = i;
				fclose(fp);
			}
			else
				fprintf(stderr, "Open file /var/traceroute.log failed!\n");

			cwmpDiagnosticDone();
		}
	}
}
void ResetTraceRouteDiag(struct TR098_TraceRouteDiag *p)
{
	if(p)
	{
		p->ResponseTime=0;
		p->RouteHopsNumberOfEntries=0;
		p->traceroute_pid=0;
		if( p->pInterface && p->pInterface[0] && (transfer2IfName( p->pInterface, p->IfName )==0) )
		{
			LANDEVNAME2BR0(p->IfName);
		}else
			p->IfName[0]=0;
		//delete the last result file???
	}
}
void StartTraceRouteDiag(void)
{
	pid_t pid;
	struct TR098_TraceRouteDiag *p=	&gTraceRouteDiag;
	struct timeval time_start, time_end, time_intvl;

	ResetTraceRouteDiag( p );
	pid = vfork();
	if(pid==0)
	{
		/* the child */
		//char *env[2];
		char cmdstr[256] = {0};
		char *filename="/bin/traceroute";
		FILE *fp;
		char lan_ip[20];

		signal(SIGINT, SIG_IGN);

		strcpy(cmdstr, filename);
		getMIB2Str(MIB_ADSL_LAN_IP, lan_ip);
		sprintf(cmdstr, "%s -s %s", cmdstr, lan_ip);

		if(p->IfName[0])
			sprintf(cmdstr, "%s -i %s", cmdstr, p->IfName);

		sprintf(cmdstr, "%s -q %d", cmdstr, p->NumberOfTries);
		sprintf(cmdstr, "%s -w %d", cmdstr, (p->Timeout/1000 < 2) ? 2 : p->Timeout/1000);
		sprintf(cmdstr, "%s -m %d", cmdstr, p->MaxHopCount);
		sprintf(cmdstr, "%s -t %d", cmdstr, p->DSCP << 2 );

		sprintf(cmdstr, "%s %s %d", cmdstr, p->pHost, p->DataBlockSize);
		sprintf(cmdstr, "%s > /var/traceroute.log 2>&1", cmdstr);

		printf("TRACEROUTE: %s\n", cmdstr);

		//env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		//env[1] = NULL;

		gettimeofday(&time_start, NULL);
		system(cmdstr);
		gettimeofday(&time_end, NULL);
		timersub(&time_end, &time_start, &time_intvl);
		p->ResponseTime = time_intvl.tv_sec * 1000 + time_intvl.tv_usec / 1000;

		fp = fopen("/var/restime", "w");
		fprintf(fp, "%d", p->ResponseTime);
		fclose(fp);

		//execve(filename, argv, env);
		//perror( "execve" );
		//printf("exec %s failed\n", filename);
		_exit(2);
	}else if(pid < 0)
	{
		perror( "vfork" );
		p->DiagnosticsState=eTRSTATE_Error_Other;//???
		cwmpDiagnosticDone();
		return;
	}

	p->traceroute_pid=pid;
}

extern unsigned int getInstNum( char *name, char *objname );
unsigned int getRouteHopsInstNum( char *name )
{
	return getInstNum( name, "RouteHops" );
}

unsigned int getRouteHopsEntityNum(void)
{
	return gTraceRouteDiag.RouteHopsNumberOfEntries;
}

int getRouteHopsEntityByIdx(unsigned int idx, unsigned int *hoperrorcode, char *hophost, char *hophostaddr, char *hoprttimes)
{
	int ret = -1, i;
	FILE *fp;
	char line[256], *tok, del[] = " ()";
	struct in_addr inp;
	char *tmp;

	//idx starts from 1,2,3,....
	if( idx<1 || hoperrorcode==NULL || hophost==NULL || hophostaddr==NULL || hoprttimes==NULL) return -1;
	*hoperrorcode=255;
	hophost[0]=0;
	hophostaddr[0]=0;
	hoprttimes[0]=0;

	if(!(fp = fopen("/var/traceroute.log", "r")))
	{
		fprintf(stderr, "Open file /var/traceroute.log failed!\n");
		return ret;
	}

	fgets(line, 256, fp);

	for( i = idx; i; i-- )
		fgets(line, 256, fp);

	// Start retrieve info from traceroute log file.
	if(!(tok = strtok(line, del)))  // Expected for index of hops
	{
		fclose(fp);
		return ret;
	}

	while(true)
	{
		if(!(tok = strtok(NULL, del)))  // Expected for domain name or IP address (when DN cannot be resolved)
			break;

		if( tok[0] == '!' )  // Got ICMP error code.
		{
			switch(tok[1])
			{
				case 'N':
					*hoperrorcode = ICMP_UNREACH_NET;
					break;
				case 'H':
					*hoperrorcode = ICMP_UNREACH_HOST;
					break;
				case 'P':
					*hoperrorcode = ICMP_UNREACH_PROTOCOL;
					break;
				case '\0':
					*hoperrorcode = ICMP_UNREACH_PORT;
					break;
				case 'F':
					*hoperrorcode = ICMP_UNREACH_NEEDFRAG;
					break;
				case 'S':
					*hoperrorcode = ICMP_UNREACH_SRCFAIL;
					break;
			}

			if(!(tok = strtok(NULL, del)))
				break;
		}

		if( tok[0] == '*' )  // No reply
			continue;

		if(!inet_aton(tok, &inp))
		{
			if(tmp = strchr(tok, '.'))
				*tmp = '\0';

			snprintf(hoprttimes, 16, "%s,%s", hoprttimes, tok);

			if(!(tok = strtok(NULL, del)))  // Expected for "ms"
				break;

			continue;
		}

		strncpy(hophost, tok, 256);

		if(!(tok = strtok(NULL, del)))  // Expected for IP address.
			break;

		strncpy(hophostaddr, tok, 16);
		if(!(tok = strtok(NULL, del)))  // Expected for response time
			break;

		if(tmp = strchr(tok, '.'))
			*tmp = '\0';

		if(!hoprttimes[0])
			strncpy(hoprttimes, tok, 16);
		else
			snprintf(hoprttimes, 16, "%s,%s", hoprttimes, tok);

		if(!(tok = strtok(NULL, del)))  // Expected for "ms"
			break;
	}

	fclose(fp);

	ret = 0;
	return ret;
}
/***********RouteHopsEntity***********/
struct CWMP_OP tRouteHopsEntityLeafOP = { getRouteHopsEntity,	NULL };
struct CWMP_PRMT tRouteHopsEntityLeafInfo[] =
{
/*(name,			type,		flag,		op)*/
{"HopHost",			eCWMP_tSTRING,	CWMP_READ,	&tRouteHopsEntityLeafOP },
{"HopHostAddress",		eCWMP_tSTRING,	CWMP_READ,	&tRouteHopsEntityLeafOP },
{"HopErrorCode",		eCWMP_tUINT,	CWMP_READ,	&tRouteHopsEntityLeafOP },
{"HopRTTimes",			eCWMP_tSTRING,	CWMP_READ,	&tRouteHopsEntityLeafOP },
};
enum eRouteHopsEntityLeaf
{
	eRH_HopHost,
	eRH_HopHostAddress,
	eRH_HopErrorCode,
	eRH_HopRTTimes,
};
struct CWMP_LEAF tRouteHopsEntityLeaf[] =
{
{ &tRouteHopsEntityLeafInfo[eRH_HopHost] },
{ &tRouteHopsEntityLeafInfo[eRH_HopHostAddress] },
{ &tRouteHopsEntityLeafInfo[eRH_HopErrorCode] },
{ &tRouteHopsEntityLeafInfo[eRH_HopRTTimes] },
{ NULL	}
};

#ifdef CONFIG_CWMP_TR181_SUPPORT
struct CWMP_PRMT tTR181RouteHopsEntityLeafInfo[] =
{
/*(name,			type,		flag,		op)*/
{"Host",			eCWMP_tSTRING,	CWMP_READ,	&tRouteHopsEntityLeafOP },
{"HostAddress",		eCWMP_tSTRING,	CWMP_READ,	&tRouteHopsEntityLeafOP },
{"ErrorCode",		eCWMP_tUINT,	CWMP_READ,	&tRouteHopsEntityLeafOP },
{"RTTimes",			eCWMP_tSTRING,	CWMP_READ,	&tRouteHopsEntityLeafOP },
};

struct CWMP_LEAF tTR181RouteHopsEntityLeaf[] =
{
{ &tTR181RouteHopsEntityLeafInfo[eRH_HopHost] },
{ &tTR181RouteHopsEntityLeafInfo[eRH_HopHostAddress] },
{ &tTR181RouteHopsEntityLeafInfo[eRH_HopErrorCode] },
{ &tTR181RouteHopsEntityLeafInfo[eRH_HopRTTimes] },
{ NULL	}
};
#endif
/**************************/
int getRouteHopsEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int instnum, hoperrorcode;
	char	hophost[257], hophostaddr[17], hoprttimes[17];

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	instnum = getRouteHopsInstNum( name );
	if(instnum<1 || instnum>gTraceRouteDiag.RouteHopsNumberOfEntries) return ERR_9007;
	hoperrorcode=0; hophost[0]=0; hophostaddr[0]=0; hoprttimes[0]=0;
	getRouteHopsEntityByIdx( instnum, &hoperrorcode, hophost, hophostaddr, hoprttimes );

	*type = entity->info->type;
	*data = NULL;

#ifdef CONFIG_CWMP_TR181_SUPPORT
	if(gUseTR181)
	{
		if( strcmp( lastname, tTR181RouteHopsEntityLeafInfo[eRH_HopHost].name )==0 )
		{
			*data = strdup( hophost );
		}else if( strcmp( lastname, tTR181RouteHopsEntityLeafInfo[eRH_HopHostAddress].name )==0 )
		{
			*data = strdup( hophostaddr );
		}else if( strcmp( lastname, tTR181RouteHopsEntityLeafInfo[eRH_HopErrorCode].name )==0 )
		{
			*data = uintdup( hoperrorcode );
		}else if( strcmp( lastname, tTR181RouteHopsEntityLeafInfo[eRH_HopRTTimes].name )==0 )
		{
			*data = strdup( hoprttimes );
		}else{
			return ERR_9005;
		}
	}
	else
	{
		if( strcmp( lastname, tRouteHopsEntityLeafInfo[eRH_HopHost].name )==0 )
		{
			*data = strdup( hophost );
		}else if( strcmp( lastname, tRouteHopsEntityLeafInfo[eRH_HopHostAddress].name )==0 )
		{
			*data = strdup( hophostaddr );
		}else if( strcmp( lastname, tRouteHopsEntityLeafInfo[eRH_HopErrorCode].name )==0 )
		{
			*data = uintdup( hoperrorcode );
		}else if( strcmp( lastname, tRouteHopsEntityLeafInfo[eRH_HopRTTimes].name )==0 )
		{
			*data = strdup( hoprttimes );
		}else{
			return ERR_9005;
		}
	}
#else	//CONFIG_CWMP_TR181_SUPPORT
	if( strcmp( lastname, tRouteHopsEntityLeafInfo[eRH_HopHost].name )==0 )
	{
		*data = strdup( hophost );
	}else if( strcmp( lastname, tRouteHopsEntityLeafInfo[eRH_HopHostAddress].name )==0 )
	{
		*data = strdup( hophostaddr );
	}else if( strcmp( lastname, tRouteHopsEntityLeafInfo[eRH_HopErrorCode].name )==0 )
	{
		*data = uintdup( hoperrorcode );
	}else if( strcmp( lastname, tRouteHopsEntityLeafInfo[eRH_HopRTTimes].name )==0 )
	{
		*data = strdup( hoprttimes );
	}else{
		return ERR_9005;
	}
#endif	//CONFIG_CWMP_TR181_SUPPORT
	return 0;
}
/*************RouteHops****************/
struct CWMP_PRMT tRouteHopsObjectInfo[] =
{
/*(name,		type,		flag,				op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,		NULL},
};
enum eRouteHopsObject
{
	eRouteHops0,
};
struct CWMP_LINKNODE tRouteHopsObject[] =
{
/*info,  				leaf,				next,		sibling,		instnum)*/
{&tRouteHopsObjectInfo[eRouteHops0],	tRouteHopsEntityLeaf,		NULL,		NULL,			0},
};

#ifdef CONFIG_CWMP_TR181_SUPPORT
struct CWMP_LINKNODE tTR181RouteHopsObject[] =
{
/*info,  				leaf,				next,		sibling,		instnum)*/
{&tRouteHopsObjectInfo[eRouteHops0],	tTR181RouteHopsEntityLeaf,		NULL,		NULL,			0},
};
#endif
/*************************************************************************************/
int objRouteHops(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity=(struct CWMP_NODE *)e;

	//fprintf( stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);
	switch( type )
	{
		case eCWMP_tINITOBJ:
		{
			int MaxInstNum=0,count=0;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if( (name==NULL) || (entity==NULL) || (data==NULL) ) return -1;

		#if 0
			count=getRouteHopsEntityNum();
			if(count)
			{
				MaxInstNum=count;
				if( create_Object( c, tRouteHopsObject, sizeof(tRouteHopsObject), count, 1 ) < 0 )
					break;
			}
			gTraceRouteDiag.RouteHopsNumberOfEntries=count;
			add_objectNum( name, MaxInstNum );
		#endif

			return 0;
		}
		case eCWMP_tUPDATEOBJ:
		{
			int count=0;
			struct CWMP_LINKNODE *old_table;
			unsigned int i;

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			count = getRouteHopsEntityNum();

			for(i=1;i<=count;i++)
			{
				struct CWMP_LINKNODE *remove_entity=NULL;

				remove_entity = remove_SiblingEntity( &old_table, i );
				if( remove_entity!=NULL )
				{
					add_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, remove_entity );
				}
				else
				{
					unsigned int MaxInstNum;
					MaxInstNum = i;
#ifdef CONFIG_CWMP_TR181_SUPPORT
					if(gUseTR181)
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tTR181RouteHopsObject, sizeof(tTR181RouteHopsObject), &MaxInstNum );
					else
#endif
						add_Object( name, (struct CWMP_LINKNODE **)&entity->next, tRouteHopsObject, sizeof(tRouteHopsObject), &MaxInstNum );
				}
			}

     			if( old_table )
				destroy_ParameterTable( (struct CWMP_NODE *)old_table );

			return 0;
		}
	}

	return -1;
}
/*************traceroutediagnostics****************/
struct CWMP_OP tTraceRouteDiagLeafOP = { getTraceRouteDiag,	setTraceRouteDiag };
struct CWMP_PRMT tTraceRouteDiagLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"DiagnosticsState",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tTraceRouteDiagLeafOP },
{"Interface",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tTraceRouteDiagLeafOP },
{"Host",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tTraceRouteDiagLeafOP },
{"NumberOfTries",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tTraceRouteDiagLeafOP },
{"Timeout",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tTraceRouteDiagLeafOP },
{"DataBlockSize",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tTraceRouteDiagLeafOP },
{"DSCP",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tTraceRouteDiagLeafOP },
{"MaxHopCount",			eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tTraceRouteDiagLeafOP },
{"ResponseTime",		eCWMP_tUINT,	CWMP_READ,		&tTraceRouteDiagLeafOP },
{"RouteHopsNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,		&tTraceRouteDiagLeafOP },
};
enum eTraceRouteDiagLeaf
{
	eTR_DiagnosticsState,
	eTR_Interface,
	eTR_Host,
	eTR_NumberOfTries,
	eTR_Timeout,
	eTR_DataBlockSize,
	eTR_DSCP,
	eTR_MaxHopCount,
	eTR_ResponseTime,
	eTR_RouteHopsNumberOfEntries,
};
struct CWMP_LEAF tTraceRouteDiagLeaf[] =
{
{ &tTraceRouteDiagLeafInfo[eTR_DiagnosticsState] },
{ &tTraceRouteDiagLeafInfo[eTR_Interface] },
{ &tTraceRouteDiagLeafInfo[eTR_Host] },
{ &tTraceRouteDiagLeafInfo[eTR_NumberOfTries] },
{ &tTraceRouteDiagLeafInfo[eTR_Timeout] },
{ &tTraceRouteDiagLeafInfo[eTR_DataBlockSize] },
{ &tTraceRouteDiagLeafInfo[eTR_DSCP] },
{ &tTraceRouteDiagLeafInfo[eTR_MaxHopCount] },
{ &tTraceRouteDiagLeafInfo[eTR_ResponseTime] },
{ &tTraceRouteDiagLeafInfo[eTR_RouteHopsNumberOfEntries] },
{ NULL	}
};
struct CWMP_OP tRouteHops_OP = { NULL, objRouteHops };
struct CWMP_PRMT tTraceRouteDiagObjectInfo[] =
{
/*(name,		type,		flag,		op)*/
{"RouteHops",		eCWMP_tOBJECT,	CWMP_READ,	&tRouteHops_OP},
};
enum eTraceRouteDiagObject
{
	eRouteHops,
};
struct CWMP_NODE tTraceRouteDiagObject[] =
{
/*info,  					leaf,		node)*/
{&tTraceRouteDiagObjectInfo[eRouteHops],	NULL,		NULL},
{NULL,						NULL,		NULL}
};
/*************************************************************************************/
int getTraceRouteDiag(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_DiagnosticsState].name )==0 )
	{
		*data = strdup( strTraceRouteDiagState[gTraceRouteDiag.DiagnosticsState] );
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_Interface].name )==0 )
	{
		if(gTraceRouteDiag.pInterface)
			*data = strdup( gTraceRouteDiag.pInterface );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_Host].name )==0 )
	{
		if(gTraceRouteDiag.pHost)
			*data = strdup( gTraceRouteDiag.pHost );
		else
			*data = strdup( "" );
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_NumberOfTries].name )==0 )
	{
		*data = uintdup( gTraceRouteDiag.NumberOfTries );
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_Timeout].name )==0 )
	{
		*data = uintdup( gTraceRouteDiag.Timeout );
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_DataBlockSize].name )==0 )
	{
		*data = uintdup( gTraceRouteDiag.DataBlockSize );
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_DSCP].name )==0 )
	{
		*data = uintdup( gTraceRouteDiag.DSCP );
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_MaxHopCount].name )==0 )
	{
		*data = uintdup( gTraceRouteDiag.MaxHopCount );
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_ResponseTime].name )==0 )
	{
		*data = uintdup( gTraceRouteDiag.ResponseTime );
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_RouteHopsNumberOfEntries].name )==0 )
	{
		*data = uintdup( gTraceRouteDiag.RouteHopsNumberOfEntries );
	}else{
		return ERR_9005;
	}

	return 0;
}
int setTraceRouteDiag(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	int *pUINT=data;
	char *pbuf = data;

	if( (name==NULL) || (entity==NULL)) return -1;
	if( data==NULL ) return ERR_9007;
	if( entity->info->type!=type ) return ERR_9006;

	if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_DiagnosticsState].name )==0 )
	{
		if( pbuf && strcmp( pbuf, strTraceRouteDiagState[eTRSTATE_Requested] )==0 )
		{
			if(gTraceRouteDiag.DiagnosticsState==eTRSTATE_Requested) return ERR_9001;
			gTraceRouteDiag.DiagnosticsState=eTRSTATE_Requested;
			gStartTraceRouteDiag=1;
			fprintf(stderr, "set %s=%s\n", name, pbuf?pbuf:"" );
			return 0;
		}
		return ERR_9007;
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_Interface].name )==0 )
	{
		char ifname[32];
		if( (pbuf==NULL) || strlen(pbuf)==0 )
		{
			if( gTraceRouteDiag.pInterface )
			{
				free( gTraceRouteDiag.pInterface );
				gTraceRouteDiag.pInterface = NULL;
			}
			strcpy( gTraceRouteDiag.IfName, "" );
			fprintf(stderr, "set %s=%s(%s)\n", name, pbuf?pbuf:"", "" );
			return 0;
		}
		else
		{
#ifdef CONFIG_CWMP_TR181_SUPPORT
			if(gUseTR181)
			{
				if(get_ip_if_ifname(pbuf, ifname) == 0)
				{
					if( gTraceRouteDiag.pInterface )
					{
						free( gTraceRouteDiag.pInterface );
						gTraceRouteDiag.pInterface = NULL;
					}
					gTraceRouteDiag.pInterface = strdup( pbuf );
					strcpy( gTraceRouteDiag.IfName, ifname );
					fprintf(stderr, "set %s=%s(%s)\n", name, pbuf?pbuf:"", ifname );
					return 0;
				}
			}
			else
#endif
			if( transfer2IfName( pbuf, ifname )==0 )
			{
				LANDEVNAME2BR0(ifname);
				if( gTraceRouteDiag.pInterface )
				{
					free( gTraceRouteDiag.pInterface );
					gTraceRouteDiag.pInterface = NULL;
				}
				gTraceRouteDiag.pInterface = strdup( pbuf );
				strcpy( gTraceRouteDiag.IfName, ifname );
				fprintf(stderr, "set %s=%s(%s)\n", name, pbuf?pbuf:"", ifname );
				return 0;
			}
		}
		return ERR_9007;
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_Host].name )==0 )
	{
		if(gTraceRouteDiag.pHost)
		{
			free( gTraceRouteDiag.pHost );
			gTraceRouteDiag.pHost=NULL;
		}
		if(pbuf) gTraceRouteDiag.pHost=strdup( pbuf );
		fprintf(stderr, "set %s=%s\n", name, pbuf?pbuf:"" );
		return 0;
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_NumberOfTries].name )==0 )
	{
		if( (*pUINT<1) || (*pUINT>3) ) return ERR_9007;
		gTraceRouteDiag.NumberOfTries=*pUINT;
		fprintf(stderr, "set %s=%u\n", name, *pUINT );
		return 0;
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_Timeout].name )==0 )
	{
		if( *pUINT<1 ) return ERR_9007;
		gTraceRouteDiag.Timeout=*pUINT;
		fprintf(stderr, "set %s=%u\n", name, *pUINT );
		return 0;
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_DataBlockSize].name )==0 )
	{
		if( (*pUINT<1) || (*pUINT>65535) ) return ERR_9007;
		gTraceRouteDiag.DataBlockSize=*pUINT;
		fprintf(stderr, "set %s=%u\n", name, *pUINT );
		return 0;
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_DSCP].name )==0 )
	{
		if( *pUINT>63 ) return ERR_9007;
		gTraceRouteDiag.DSCP=*pUINT;
		fprintf(stderr, "set %s=%u\n", name, *pUINT );
		return 0;
	}else if( strcmp( lastname, tTraceRouteDiagLeafInfo[eTR_MaxHopCount].name )==0 )
	{
		if( (*pUINT<1) || (*pUINT>64) ) return ERR_9007;
		gTraceRouteDiag.MaxHopCount=*pUINT;
		fprintf(stderr, "set %s=%u\n", name, *pUINT );
		return 0;
	}else{
		return ERR_9005;
	}

	return 0;

}
#endif //_SUPPORT_TRACEROUTE_PROFILE_


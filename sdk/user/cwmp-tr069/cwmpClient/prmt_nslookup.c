#include "prmt_nslookup.h"
#include <signal.h>
#include <sys/wait.h>

#ifdef _PRMT_NSLOOKUP_
#define NSLOOKUP_LOG "/var/nslookup.log"

struct NSLookupResult
{
	char Status[30];
	char AnswerType[20];
	char HostNameReturned[256];
	char IPAddresses[256];
	char DNSServerIP[45];
	unsigned int ResponseTime;
};

enum eNSLookupDiagState
{
	eNSLSTATE_None,
	eNSLSTATE_Requested,
	eNSLSTATE_Complete,
	eNSLSTATE_Error_DNSServerNotResolved,
	eNSLSTATE_Error_Internal,
	eNSLSTATE_Error_Other,
};

char *strNSLookupDiagState[] =
{
	"None",
	"Requested",
	"Complete",
	"Error_DNSServerNotResolved",
	"Error_Internal",
	"Error_Other",
};

struct NSLookupDiagnostics
{
	int DiagnosticsState;
	char *pInterface;
	char IfName[32];
	char *pHostName;
	char *pDNSServer;
	unsigned int Timeout; /*milliseconds*/
	unsigned int NumberOfRepetitions;
	unsigned int SuccessCount;
	unsigned int ResultNumberOfEntries;
	unsigned long int nslookup_pid;
};

struct NSLookupDiagnostics gNSLookupDiag =
{
	eNSLSTATE_None,		/*DiagnosticsState*/
	NULL,			/*pInterface*/
	"", 			/*IfName*/
	NULL,			/*pHostName*/
	NULL,  /* pDNSServer */
	5000,/* milliseconds */	/* Timeout */
	1,			/* NumberOfRepetitions */
	0,			/* SuccessCount */
	0,			/* ResultNumberOfEntries */
	0,			/* nslookup_pid */
};

int gStartNSLookupDiag = 0;
void checkPidforNSLookupDiag(pid_t pid)
{
	int st;

	if(gNSLookupDiag.nslookup_pid > 0)
		pid = waitpid(gNSLookupDiag.nslookup_pid, &st, WNOHANG);

	if(pid > 0)
	{
		if(pid == gNSLookupDiag.nslookup_pid)
		{
			FILE *fp;
			char line[256];
			int i;
			char success = 0;

			gNSLookupDiag.nslookup_pid = 0;
			//update the result
			gNSLookupDiag.DiagnosticsState = eNSLSTATE_Error_DNSServerNotResolved;

			if(fp = fopen(NSLOOKUP_LOG, "r"))
			{
				while(fgets(line, 256, fp))
				{
					if(strstr(line, "Name:") == line)
					{
						gNSLookupDiag.DiagnosticsState = eNSLSTATE_Complete;
						gNSLookupDiag.SuccessCount++;
					}

					if(strstr(line, "===="))
						gNSLookupDiag.ResultNumberOfEntries++;
				}

				fclose(fp);
			}
			else
			{
				gNSLookupDiag.DiagnosticsState = eNSLSTATE_Error_Internal;
				CWMPDBP(1, "Open file %s failed!\n", NSLOOKUP_LOG);
			}

			cwmpDiagnosticDone();
		}
	}
}

void ResetNSLookupDiag(struct NSLookupDiagnostics *p)
{
	if(p)
	{
		p->SuccessCount=0;
		p->ResultNumberOfEntries=0;
		p->nslookup_pid=0;
		if(p->pInterface && p->pInterface[0] && (transfer2IfName(p->pInterface, p->IfName)==0))
		{
			LANDEVNAME2BR0(p->IfName);
		}else
			p->IfName[0]=0;
	}
}

void StartNSLookupDiag()
{
	pid_t pid;
	struct NSLookupDiagnostics *p = &gNSLookupDiag;
	struct timeval time_start, time_end, time_intvl;
	int i;

	ResetNSLookupDiag(p);
	pid = vfork();
	if(pid==0)
	{
		/* the child */
		//char *env[2];
		char cmdstr[256] = {0};
		char *filename = "/bin/nslookup";
		FILE *fp;

		signal(SIGINT, SIG_IGN);

		strcpy(cmdstr, filename);

		if(p->pDNSServer)
			sprintf(cmdstr, "%s %s %s", cmdstr, p->pHostName, p->pDNSServer);
		else
			sprintf(cmdstr, "%s %s", cmdstr, p->pHostName);

		unlink(NSLOOKUP_LOG);
		sprintf(cmdstr, "%s >> %s 2>&1", cmdstr, NSLOOKUP_LOG, cmdstr);

		CWMPDBP(1, "NSLOOKUP: %s\n", cmdstr);

		//env[0] = "PATH=/bin:/usr/bin:/etc:/sbin:/usr/sbin";
		//env[1] = NULL;

		for( i = 0; i < p->NumberOfRepetitions; i++)
		{
			gettimeofday(&time_start, NULL);
			system(cmdstr);
			gettimeofday(&time_end, NULL);
			timersub(&time_end, &time_start, &time_intvl);

			if(!(fp = fopen(NSLOOKUP_LOG, "a")))
			{
				unlink(NSLOOKUP_LOG);
				_exit(2);
			}

			fprintf(fp, "Response Time: %d\n====\n", time_intvl.tv_sec * 1000 + time_intvl.tv_usec / 1000);
			fclose(fp);
		}

		_exit(2);
	}else if(pid < 0)
	{
		perror("vfork");
		p->DiagnosticsState = eNSLSTATE_Error_Other;
		cwmpDiagnosticDone();
		return;
	}

	p->nslookup_pid = pid;
}

extern unsigned int getInstNum(char *name, char *objname);
unsigned int getResultInstNum(char *name)
{
	return getInstNum(name, "Result");
}

unsigned int getResultEntityNum()
{
	return gNSLookupDiag.ResultNumberOfEntries;
}

int getResultEntityByIdx(struct NSLookupResult *result, int instnum)
{
	int ret = -1, i;
	FILE *fp;
	char line[256], *tok, is_name_got = 0;
	const char del[] = " \n";

	memset(result, 0, sizeof(struct NSLookupResult));

	if(!(fp = fopen(NSLOOKUP_LOG, "r")))
	{
		CWMPDBP(1, "Open file %s failed!\n", NSLOOKUP_LOG);
		return ret;
	}

	for(i = instnum-1; i; i--)
	{
		while(true)
		{
			if(!fgets(line, 256, fp))
				return ret;

			if(strstr(line, "===="))
				break;
		}
	}

	strcpy(result->AnswerType, "Authoritative");
	strcpy(result->Status, "Success");

	while(fgets(line, 256, fp))
	{
		if(strstr(line, "===="))
			break;

		if(strstr(line, "Server:"))
		{
			strtok(line, del);
			tok = strtok(NULL, del);

			if(!strcmp(tok, "0.0.0.0"))
				strcpy(result->Status, "Error_DNSServerNotAvailable");
			else
				strcpy(result->DNSServerIP, tok);
		}
		else if(strstr(line, "Name:"))
		{
			strtok(line, del);
			tok = strtok(NULL, del);
			is_name_got = 1;

			strcpy(result->HostNameReturned, tok);
		}
		else if(strstr(line, "Address") == line)
		{
			struct in_addr inp;

			if(!is_name_got)
				continue;

			strtok(line, del);
			strtok(NULL, del);
			tok = strtok(NULL, del);

			if(!inet_aton(tok, &inp))
				continue;

			if(result->IPAddresses[0] == 0)
				strcpy(result->IPAddresses, tok);
			else
			{
				strcat(result->IPAddresses, ",");
				strcat(result->IPAddresses, tok);
			}
		}
		else if(strstr(line, "Response Time:"))
		{
			strtok(line, del);
			strtok(NULL, del);
			tok = strtok(NULL, del);

			result->ResponseTime = atoi(tok);
		}
		else if(strstr(line, "can't resolve"))
		{
			strcpy(result->AnswerType, "None");
			strcpy(result->Status, "Error_HostNameNotResolved");
		}
	}

	fclose(fp);

	return ret = 0;
}

/*********** ResultEntity ***********/
struct CWMP_OP tResultEntityLeafOP = {getResultEntity, NULL};
struct CWMP_PRMT tResultEntityLeafInfo[] =
{
	/*(name,			type,		flag,		op)*/
	{"Status", eCWMP_tSTRING, CWMP_READ, &tResultEntityLeafOP},
	{"AnswerType", eCWMP_tSTRING, CWMP_READ, &tResultEntityLeafOP},
	{"HostNameReturned", eCWMP_tSTRING, CWMP_READ, &tResultEntityLeafOP},
	{"IPAddresses", eCWMP_tSTRING, CWMP_READ, &tResultEntityLeafOP},
	{"DNSServerIP", eCWMP_tSTRING, CWMP_READ, &tResultEntityLeafOP},
	{"ResponseTime", eCWMP_tUINT, CWMP_READ, &tResultEntityLeafOP},
};

enum eRouteHopsEntityLeaf
{
	eREL_Status,
	eREL_AnswerType,
	eREL_HostNameReturned,
	eREL_IPAddresses,
	eREL_DNSServerIP,
	eREL_ResponseTime
};

struct CWMP_LEAF tResultEntityLeaf[] =
{
	{&tResultEntityLeafInfo[eREL_Status]},
	{&tResultEntityLeafInfo[eREL_AnswerType]},
	{&tResultEntityLeafInfo[eREL_HostNameReturned]},
	{&tResultEntityLeafInfo[eREL_IPAddresses]},
	{&tResultEntityLeafInfo[eREL_DNSServerIP]},
	{&tResultEntityLeafInfo[eREL_ResponseTime]},
	{NULL}
};

int getResultEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;
	unsigned int instnum;
	struct NSLookupResult result;

	if((name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	instnum = getResultInstNum(name);
	if(instnum < 1 || instnum > gNSLookupDiag.ResultNumberOfEntries) return ERR_9007;
	getResultEntityByIdx(&result, instnum);

	*type = entity->info->type;
	*data = NULL;
	if(strcmp(lastname, "Status") == 0)
	{
		*data = strdup(result.Status);
	}
	else if(strcmp(lastname, "AnswerType") == 0)
	{
		*data = strdup(result.AnswerType);
	}
	else if(strcmp(lastname, "HostNameReturned") == 0)
	{
		*data = strdup(result.HostNameReturned);
	}
	else if(strcmp(lastname, "IPAddresses") == 0)
	{
		*data = strdup(result.IPAddresses);
	}
	else if(strcmp(lastname, "DNSServerIP") == 0)
	{
		*data = strdup(result.DNSServerIP);
	}
	else if(strcmp(lastname, "ResponseTime") == 0)
	{
		*data = uintdup(result.ResponseTime);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

/************* Result ****************/
struct CWMP_PRMT tResultObjectInfo[] =
{
	/*(name,		type,		flag,				op)*/
	{"0", eCWMP_tOBJECT, CWMP_READ|CWMP_LNKLIST, NULL},
};

enum eResultObject
{
	eResult0,
};

struct CWMP_LINKNODE tResultObject[] =
{
	/*info,  				leaf,				next,		sibling,		instnum)*/
	{&tResultObjectInfo[eResult0], tResultEntityLeaf, NULL, NULL, 0},
};

int objResult(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;

	//fprintf(stderr, "%s:action:%d: %s\n", __FUNCTION__, type, name);
	switch(type)
	{
		case eCWMP_tINITOBJ:
		{
			int MaxInstNum=0, count=0;
			struct CWMP_LINKNODE **c = (struct CWMP_LINKNODE **)data;

			if((name==NULL) || (entity==NULL) || (data==NULL)) return -1;

		#if 0
			count=getRouteHopsEntityNum();
			if(count)
			{
				MaxInstNum=count;
				if(create_Object(c, tResultObject, sizeof(tResultObject), count, 1) < 0)
					break;
			}
			gNSLookupDiag.RouteHopsNumberOfEntries=count;
			add_objectNum(name, MaxInstNum);
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

			count = getResultEntityNum();

			for(i = 1; i <= count; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL;

				remove_entity = remove_SiblingEntity(&old_table, i);
				if(remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					unsigned int MaxInstNum;
					MaxInstNum = i;
					add_Object(name, (struct CWMP_LINKNODE **)&entity->next, tResultObject, sizeof(tResultObject), &MaxInstNum);
				}
			}

     	if(old_table)
				destroy_ParameterTable((struct CWMP_NODE *)old_table);

			return 0;
		}
	}

	return -1;
}

/*************traceroutediagnostics****************/
struct CWMP_OP tNSLookupDiagLeafOP = {getNSLookupDiag,	setNSLookupDiag};
struct CWMP_PRMT tNSLookupDiagLeafInfo[] =
{
	/*(name,			type,		flag,			op)*/
	{"DiagnosticsState", eCWMP_tSTRING, CWMP_WRITE|CWMP_READ, &tNSLookupDiagLeafOP},
	{"Interface", eCWMP_tSTRING, CWMP_WRITE|CWMP_READ, &tNSLookupDiagLeafOP},
	{"HostName", eCWMP_tSTRING, CWMP_WRITE|CWMP_READ, &tNSLookupDiagLeafOP},
	{"DNSServer", eCWMP_tSTRING, CWMP_WRITE|CWMP_READ, &tNSLookupDiagLeafOP},
	{"Timeout", eCWMP_tUINT, CWMP_WRITE|CWMP_READ, &tNSLookupDiagLeafOP},
	{"NumberOfRepetitions", eCWMP_tUINT, CWMP_WRITE|CWMP_READ, &tNSLookupDiagLeafOP},
	{"SuccessCount", eCWMP_tUINT, CWMP_WRITE|CWMP_READ, &tNSLookupDiagLeafOP},
	{"ResultNumberOfEntries", eCWMP_tUINT, CWMP_READ, &tNSLookupDiagLeafOP},
};

enum eNSLookupDiagLeaf
{
	eNSL_DiagnosticsState,
	eNSL_Interface,
	eNSL_HostName,
	eNSL_DNSServer,
	eNSL_Timeout,
	eNSL_NumberOfRepetitions,
	eNSL_SuccessCount,
	eNSL_ResultNumberOfEntries,
};

struct CWMP_LEAF tNSLookupDiagLeaf[] =
{
	{&tNSLookupDiagLeafInfo[eNSL_DiagnosticsState]},
	{&tNSLookupDiagLeafInfo[eNSL_Interface]},
	{&tNSLookupDiagLeafInfo[eNSL_HostName]},
	{&tNSLookupDiagLeafInfo[eNSL_DNSServer]},
	{&tNSLookupDiagLeafInfo[eNSL_Timeout]},
	{&tNSLookupDiagLeafInfo[eNSL_NumberOfRepetitions]},
	{&tNSLookupDiagLeafInfo[eNSL_SuccessCount]},
	{&tNSLookupDiagLeafInfo[eNSL_ResultNumberOfEntries]},
	{NULL}
};

struct CWMP_OP tResult_OP = {NULL, objResult};
struct CWMP_PRMT tNSLookupDiagObjectInfo[] =
{
	/*(name,		type,		flag,		op)*/
	{"Result", eCWMP_tOBJECT, CWMP_READ, &tResult_OP},
};

enum eNSLookupDiagObject
{
	eResult,
};

struct CWMP_NODE tNSLookupDiagObject[] =
{
	/*info, leaf, node)*/
	{&tNSLookupDiagObjectInfo[eResult], NULL, NULL},
	{NULL, NULL, NULL}
};

int getNSLookupDiag(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char	*lastname = entity->info->name;

	if((name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;

	if(strcmp(lastname, "DiagnosticsState") == 0)
	{
		*data = strdup(strNSLookupDiagState[gNSLookupDiag.DiagnosticsState]);
	}
	else if(strcmp(lastname, "Interface") == 0)
	{
		if(gNSLookupDiag.pInterface)
			*data = strdup(gNSLookupDiag.pInterface);
		else
			*data = strdup("");
	}
	else if(strcmp(lastname, "HostName") == 0)
	{
		if(gNSLookupDiag.pHostName)
			*data = strdup(gNSLookupDiag.pHostName);
		else
			*data = strdup("");
	}
	else if(strcmp(lastname, "DNSServer") == 0)
	{
		if(gNSLookupDiag.pDNSServer)
			*data = strdup(gNSLookupDiag.pDNSServer);
		else
			*data = strdup("");
	}
	else if(strcmp(lastname, "Timeout") == 0)
	{
		*data = uintdup(gNSLookupDiag.Timeout);
	}
	else if(strcmp(lastname, "NumberOfRepetitions") == 0)
	{
		*data = uintdup(gNSLookupDiag.NumberOfRepetitions);
	}
	else if(strcmp(lastname, "SuccessCount") == 0)
	{
		*data = uintdup(gNSLookupDiag.SuccessCount);
	}
	else if(strcmp(lastname, "ResultNumberOfEntries") == 0)
	{
		*data = uintdup(gNSLookupDiag.ResultNumberOfEntries);
	}
	else{
		return ERR_9005;
	}

	return 0;
}

int setNSLookupDiag(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char	*lastname = entity->info->name;
	int *pUINT = data;
	char *pbuf = data;

	if((name == NULL) || (entity == NULL)) return -1;
	if(data == NULL) return ERR_9007;
	if(entity->info->type != type) return ERR_9006;

	if(strcmp(lastname, "DiagnosticsState") == 0)
	{
		if(pbuf && strcmp(pbuf, strNSLookupDiagState[eNSLSTATE_Requested]) == 0)
		{
			if(gNSLookupDiag.DiagnosticsState == eNSLSTATE_Requested) return ERR_9001;

			gNSLookupDiag.DiagnosticsState = eNSLSTATE_Requested;
			gStartNSLookupDiag = 1;
			CWMPDBP(0, "set %s=%s\n", name, pbuf? pbuf: "");
			return 0;
		}
		return ERR_9007;
	}
	else if(strcmp(lastname, "Interface") == 0)
	{
		char ifname[32];
		if((pbuf == NULL) || strlen(pbuf) == 0)
		{
			if(gNSLookupDiag.pInterface)
			{
				free(gNSLookupDiag.pInterface);
				gNSLookupDiag.pInterface = NULL;
			}
			strcpy(gNSLookupDiag.IfName, "");
			CWMPDBP(0, "set %s=%s(%s)\n", name, pbuf? pbuf: "", "");
			return 0;
		}
		else
		{
			if(transfer2IfName(pbuf, ifname) == 0)
			{
				LANDEVNAME2BR0(ifname);
				if(gNSLookupDiag.pInterface)
				{
					free(gNSLookupDiag.pInterface);
					gNSLookupDiag.pInterface = NULL;
				}
				gNSLookupDiag.pInterface = strdup(pbuf);
				strcpy(gNSLookupDiag.IfName, ifname);
				CWMPDBP(0, "set %s=%s(%s)\n", name, pbuf?pbuf:"", ifname);
				return 0;
			}
		}
		return ERR_9007;
	}
	else if(strcmp(lastname, "HostName") == 0)
	{
		if(gNSLookupDiag.pHostName)
		{
			free(gNSLookupDiag.pHostName);
			gNSLookupDiag.pHostName = NULL;
		}

		if(pbuf)
			gNSLookupDiag.pHostName = strdup(pbuf);

		CWMPDBP(0, "set %s=%s\n", name, pbuf? pbuf: "");
		return 0;
	}
	else if(strcmp(lastname, "DNSServer") == 0)
	{
		if(gNSLookupDiag.pDNSServer)
		{
			free(gNSLookupDiag.pDNSServer);
			gNSLookupDiag.pDNSServer = NULL;
		}

		if(pbuf)
			gNSLookupDiag.pDNSServer = strdup(pbuf);
	}
	else if(strcmp(lastname, "Timeout") == 0)
	{
		if(*pUINT<1) return ERR_9007;
		gNSLookupDiag.Timeout = *pUINT;
		CWMPDBP(0, "set %s=%u\n", name, *pUINT);
		return 0;
	}
	else if(strcmp(lastname, "NumberOfRepetitions") == 0)
	{
		if(*pUINT < 0) return ERR_9007;

		gNSLookupDiag.NumberOfRepetitions = *pUINT;
		CWMPDBP(0, "set %s=%u\n", name, *pUINT);
		return 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}
#endif


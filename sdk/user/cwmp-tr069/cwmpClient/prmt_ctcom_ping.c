#include <wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <config/autoconf.h>
#include <libcwmp.h>
#include <parameter_api.h>
#include <rtk/options.h>

#include "prmt_ctcom_ping.h"

#ifdef _PRMT_X_CT_COM_PING_

/***** InternetGatewayDevice.DeviceInfo.X_CT-COM_Ping.PingConfig.{i}. ********/
struct CWMP_OP tCT_PingEntityLeafOP = { getCT_PingEntity, setCT_PingEntity};
struct CWMP_PRMT tCT_PingEntityLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"DiagnosticsState",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"Interface",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tCT_PingEntityLeafOP},
{"Host",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tCT_PingEntityLeafOP},
{"NumberOfRepetitions",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"Timeout",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tCT_PingEntityLeafOP},
{"DataBlockSize",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"DSCP",		eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,		&tCT_PingEntityLeafOP},
{"Interval",	eCWMP_tUINT,	CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
{"Stop",		eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_PingEntityLeafOP},
};
enum eCT_PingEntityLeaf
{
	eCT_PingEntityDiagnosticsState,
	eCT_PingEntityInterface,
	eCT_PingEntityHost,
	eCT_PingEntityNumberOfRepetitions,
	eCT_PingEntityTimeout,
	eCT_PingEntityDataBlockSize,
	eCT_PingEntityDSCP,
	eCT_PingEntityInterval,
	eCT_PingEntityStop,
};
struct CWMP_LEAF tCT_PingEntityLeaf[] =
{
{ &tCT_PingEntityLeafInfo[eCT_PingEntityDiagnosticsState] },
{ &tCT_PingEntityLeafInfo[eCT_PingEntityInterface] },
{ &tCT_PingEntityLeafInfo[eCT_PingEntityHost] },
{ &tCT_PingEntityLeafInfo[eCT_PingEntityNumberOfRepetitions] },
{ &tCT_PingEntityLeafInfo[eCT_PingEntityTimeout] },
{ &tCT_PingEntityLeafInfo[eCT_PingEntityDataBlockSize] },
{ &tCT_PingEntityLeafInfo[eCT_PingEntityDSCP] },
{ &tCT_PingEntityLeafInfo[eCT_PingEntityInterval] },
{ &tCT_PingEntityLeafInfo[eCT_PingEntityStop] },
{ NULL }
};

/***** InternetGatewayDevice.DeviceInfo.X_CT-COM_Ping.PingConfig. ************/
struct CWMP_PRMT tCT_PingEntityObjectInfo[] =
{
/*(name,		type,		flag,					op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};
enum eCT_PingEntityObject
{
	ePingEntity0
};
struct CWMP_LINKNODE tCT_PingEntityObject[] =
{
/*info,  					leaf,			next,		sibling,		instnum)*/
{&tCT_PingEntityObjectInfo[ePingEntity0], 	tCT_PingEntityLeaf,	NULL,		NULL,			0}
};


/***** InternetGatewayDevice.DeviceInfo.X_CT-COM_Ping. ***********************/
struct CWMP_OP tCT_PingLeafOP = {getCT_Ping, setCT_Ping};
struct CWMP_PRMT tCT_PingLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"Enable",	eCWMP_tBOOLEAN,	CWMP_WRITE|CWMP_READ,	&tCT_PingLeafOP},
{"PingNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tCT_PingLeafOP},
};
enum eCT_PingLeaf
{
	eCT_PingEnable,
	eCT_PingPingNumberOfEntries,
};
struct CWMP_LEAF tCT_PingLeaf[] =
{
{ &tCT_PingLeafInfo[eCT_PingEnable] },
{ &tCT_PingLeafInfo[eCT_PingPingNumberOfEntries] },
{ NULL }
};


struct CWMP_OP tCT_Ping_OP = { NULL, objCT_PingConfig};
struct CWMP_PRMT tCT_PingObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"PingConfig",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tCT_Ping_OP}
};
enum eCT_MonitorObject
{
	ePingConfig
};
struct CWMP_NODE tCT_PingObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_PingObjectInfo[ePingConfig], 		NULL,			NULL},
{NULL,					NULL,			NULL}
};



/***** Utilities *************************************************************/
enum {
	STATE_NONE,
	STATE_REQUESTED,
	STATE_COMPLETE,
	STATE_ERROR_HOSTNAME,
	STATE_ERROR_INTERNAL,
	STATE_ERROR_OTHER,
};

static const char * strDiagnosticState[] = { "None", "Requested", "Complete", "Error_ConnotResolveHostName", "Error_Internal", "Error_Other",0 };
// Diagnostic Variables
static unsigned char startup_checked = 0;
struct CWMP_LINKNODE *gCT_ping_list = NULL;
struct CT_PingConfig
{
	CWMP_CT_PING_T *entry;
	int chain_id;
	unsigned char stop;
	pthread_t thread;
	pthread_mutex_t mutex;
	pid_t ping_pid;
};

/***** Utilities *************************************************************/
void free_PingConfig(void *obj_data)
{
	struct CT_PingConfig *config = (struct CT_PingConfig *)obj_data;

	if(config == NULL)
		return;

	// stop ping process if there is
	if(config->ping_pid)
	{
		//Kill ping process
		kill(config->ping_pid, SIGTERM);

		//wait for thread terminated automatically
		sleep(1);

		// free thread resources
		pthread_mutex_destroy(&config->mutex);
	}

	if(config->entry)
		free(config->entry);

	free(config);
}

static void *ct_ping_thread(void *arg)
{
	struct CT_PingConfig *config = (struct CT_PingConfig *)arg;
	int status = -1;
	char ifname[16], repetitions[16], timeout[16], size[16];
	char dscp[16], interval[16];
	char *pifname = NULL;
	pid_t wpid;

	char * argv[14];
	int idx = 0;

	argv[idx++] = "/bin/ping";

	if(strlen(config->entry->host) == 0)
	{
		status = 1;
		goto END;
	}
	
	if (transfer2IfName(config->entry->interface, ifname))
	{
		printf("convert %s failed, use default routing\n", config->entry->interface);
		pifname = NULL;
	}
	else
	{
		LANDEVNAME2BR0( ifname );
		pifname = ifname;
		CWMPDBG(1, (stderr, "CT ping thread converted %s->%s\n", config->entry->interface, ifname));
	}

	// interface
	if(pifname && strlen(pifname) > 0)
	{
		argv[idx++] = "-I";
		argv[idx++] = ifname;
	}

	//repetitions
	if(config->entry->repetitions > 0)
	{
		argv[idx++] = "-c";
		sprintf(repetitions, "%d", config->entry->repetitions);
		argv[idx++] = repetitions;
	}

	//timeout
	argv[idx++] = "-W";
	sprintf(timeout, "%d", config->entry->timeout);
	argv[idx++] = timeout;

	//data block size
	argv[idx++] = "-s";
	sprintf(size, "%d", config->entry->size);
	argv[idx++] = size;

	//dscp
	if(config->entry->dscp)
	{
		//DSCP is bit 2~7 of TOS field
		argv[idx++] = "-Q";
		sprintf(dscp, "%d", config->entry->dscp << 2);
		argv[idx++] = dscp;
	}

	//interval
	if(config->entry->interval > 1)
	{
		argv[idx++] = "-i";
		sprintf(interval, "%d", config->entry->interval);
		argv[idx++] = interval;
	}

	//host
	argv[idx++] = config->entry->host;
	argv[idx] = NULL;

	config->ping_pid = fork();
	if(config->ping_pid == 0)
	{
		exit(execv("/bin/ping", argv));
	}
	else if(config->ping_pid > 0)
	{
		/* parent, wait till ping process end */
		while ((wpid = wait(&status)) != config->ping_pid)
		{
			if (wpid == -1 && errno == ECHILD)	/* see wait(2) manpage */
				break;
		}

		if(WIFSIGNALED(status))
			status = 0;
		else
			status = WEXITSTATUS(status);
	}

END:
	pthread_mutex_lock(&config->mutex);
	if (status == -1) /*execv failed*/
		config->entry->diag_state = STATE_ERROR_INTERNAL;
	else if(status != 0)
		config->entry->diag_state = STATE_ERROR_HOSTNAME;
	else
		config->entry->diag_state = STATE_COMPLETE;



	config->stop = 0;
	config->thread = 0;
	config->ping_pid = 0;
	pthread_mutex_unlock(&config->mutex);
	pthread_detach(pthread_self());

	return NULL;
}

void ct_check_ping()
{
	struct CWMP_LINKNODE *node = gCT_ping_list;
	struct CT_PingConfig *config = NULL;
	void *res;
	unsigned char enable;

	mib_get(CWMP_CT_PING_ENABLE, &enable);

	if(enable == 0)
		return;

	while(node)
	{
		config = (struct CT_PingConfig *) node->obj_data;

		if(config->entry->diag_state== STATE_REQUESTED && config->thread == 0)
		{
			// ct_ping_thread() will return very soon if ping failed
			// lock to avoid race condition
			pthread_mutex_lock(&config->mutex);

			//start new ping process
			if(pthread_create( &config->thread, NULL, ct_ping_thread, config) != 0)
			{
				config->entry->diag_state = STATE_ERROR_INTERNAL;
				mib_chain_update(CWMP_CT_PING_TBL, config->entry, config->chain_id);
				config->thread = 0;
			}
			pthread_mutex_unlock(&config->mutex);
			CWMPDBG(1, (stderr, "CT ping thread id %d created\n", config->thread));
		}
		else if(config->stop == 1)
		{
			pthread_mutex_lock(&config->mutex);
			CWMPDBG(1, (stderr, "Stopping  CT ping pid %d....\n", config->ping_pid));
			if(config->ping_pid == 0)
			{
				config->stop = 0;
				pthread_mutex_unlock(&config->mutex);
				node = node->sibling;
				continue;
			}
 
			//Kill ping process
			kill(config->ping_pid, SIGTERM);
			CWMPDBG(1, (stderr, "CT ping pid %d killed\n", config->ping_pid));
			pthread_mutex_unlock(&config->mutex);
		}

		node = node->sibling;
	}
}

/***** Operations ************************************************************/

int getCT_Ping(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	*type = entity->info->type;
	*data = NULL;
	if (strcmp(lastname, "Enable") == 0)
	{
		unsigned char vChar = 0;
		mib_get(CWMP_CT_PING_ENABLE, &vChar);
		*data = booldup(vChar != 0);
	}
	else if (strcmp(lastname, "PingNumberOfEntries") == 0)
	{
		*data = uintdup(mib_chain_total(CWMP_CT_PING_TBL));
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_Ping(char *name, struct CWMP_LEAF *entity, int type, void *data)
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
		mib_set(CWMP_CT_PING_ENABLE, &vChar);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int objCT_PingConfig(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, num, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;

	if (name == NULL || entity == NULL)
		return -1;

	num = mib_chain_total(CWMP_CT_PING_TBL);

	switch (type) {
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		{
			if (data == NULL)
				return -1;

			ret =
			    add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_PingEntityObject,
				       sizeof(tCT_PingEntityObject), data);
			if (ret >= 0)
			{
				struct CWMP_LINKNODE *tmp_entity = NULL;
				CWMP_CT_PING_T *entry = malloc(sizeof(CWMP_CT_PING_T));

				if(entry == NULL)
					return ERR_9004;

				memset(entry, 0, sizeof(CWMP_CT_PING_T));
				entry->InstanceNum = *(int *)data;
				entry->size = 56;
				entry->timeout = 1000;
				entry->interval = 1;
				mib_chain_add(CWMP_CT_PING_TBL, entry);

				//Attach object data
				tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, entry->InstanceNum);
				if(tmp_entity)
				{
					struct CT_PingConfig *config = NULL;
					config = malloc(sizeof(struct CT_PingConfig));
					if(config == NULL)
					{
						fprintf(stderr, "<%s:%d> malloc failed!\n", __FUNCTION__, __LINE__);
						return ERR_9004;
					}
					memset(config, 0, sizeof(struct CT_PingConfig));
					config->entry = entry;
					config->chain_id = num;
					pthread_mutex_init(&config->mutex, NULL);

					tmp_entity->obj_data = (void *)config;
					tmp_entity->free_obj_data = free_PingConfig;
				}
			}

			gCT_ping_list = (struct CWMP_LINKNODE *)entity->next;

			break;
		}
	case eCWMP_tDELOBJ:
		{
			unsigned int *pUint = data;
			CWMP_CT_PING_T entry = {0};
			unsigned char found = 0;

			ret = ERR_9005;

			if (data == NULL)
				return -1;

			for (i = 0; i < num; i++)
			{
				if (!mib_chain_get(CWMP_CT_PING_TBL, i, &entry))
					continue;

				if (!found && entry.InstanceNum == *pUint)
				{
					mib_chain_delete(CWMP_CT_PING_TBL, i);
					ret = del_Object(name, (struct CWMP_LINKNODE **) &entity->next, *(int *)data);
					found = 1;
					--i;
					--num;
					continue;
				}
				else if(found)
				{
					struct CWMP_LINKNODE *tmp_entity = NULL;
					// update chain_id
					tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, entry.InstanceNum);
					if(tmp_entity)
					{
						struct CT_PingConfig *config = (struct CT_PingConfig *)tmp_entity->obj_data;
						config->chain_id = i;
					}
				}
			}

			gCT_ping_list = (struct CWMP_LINKNODE *)entity->next;

			break;
		}
	case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;
			CWMP_CT_PING_T entry = {0};

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < num; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL;

				if (!mib_chain_get(CWMP_CT_PING_TBL, i, &entry))
					continue;

				remove_entity = remove_SiblingEntity(&old_table, entry.InstanceNum);

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					struct CWMP_LINKNODE *tmp_entity = NULL;
					add_Object(name,
						   (struct CWMP_LINKNODE **) &entity->next,
						   tCT_PingEntityObject,
						   sizeof(tCT_PingEntityObject),
						   &entry.InstanceNum);

					tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&entity->next, entry.InstanceNum);
					if(tmp_entity)
					{
						struct CT_PingConfig *config = NULL;
						config = malloc(sizeof(struct CT_PingConfig));
						if(config == NULL)
						{
							fprintf(stderr, "<%s:%d> malloc failed!\n", __FUNCTION__, __LINE__);
							return ERR_9002;
						}
						memset(config, 0, sizeof(struct CT_PingConfig));
						config->entry = malloc(sizeof(CWMP_CT_PING_T));
						if(config->entry == NULL)
						{
							fprintf(stderr, "<%s:%d> malloc failed!\n", __FUNCTION__, __LINE__);
							free(config);
							return ERR_9002;
						}
						memcpy(config->entry, &entry, sizeof(CWMP_CT_PING_T));
						config->chain_id = i;
						pthread_mutex_init(&config->mutex, NULL);
					
						tmp_entity->obj_data = (void *)config;
						tmp_entity->free_obj_data = free_PingConfig;
					}
				}
			}

			gCT_ping_list = (struct CWMP_LINKNODE *)entity->next;

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)old_table);

			if(!startup_checked)
			{
				ct_check_ping();
				startup_checked = 1;
			}	

			break;
		}
	}

	return ret;
}

int getCT_PingEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	unsigned int num = 0;
	struct CT_PingConfig *config = NULL;
	struct CWMP_LINKNODE *config_entity = NULL;

	// ASSERT
	if( (name==NULL) || (type==NULL) || (data==NULL) || (entity==NULL)) 
		return -1;

	*type = entity->info->type;
	*data = NULL;

	num = getInstNum(name, "X_CT-COM_Ping.PingConfig");
	config_entity = find_SiblingEntity( &gCT_ping_list, num);

	if(config_entity == NULL)
		return ERR_9005;

	config = (struct CT_PingConfig *)config_entity->obj_data;

	switch(getIndexOf(tCT_PingEntityLeaf, entity->info->name))
	{
	case eCT_PingEntityDiagnosticsState:	//State
		*data = strdup( strDiagnosticState[config->entry->diag_state] );
		break;
	case eCT_PingEntityInterface:	//Interface
		*data = strdup(config->entry->interface);
		break;
	case eCT_PingEntityHost: // Host
		*data = strdup(config->entry->host);		
		break;
	case eCT_PingEntityNumberOfRepetitions: // #ofRepetitions
		*data = uintdup(config->entry->repetitions);
		break;	
	case eCT_PingEntityTimeout: // timeout
		*data = uintdup(config->entry->timeout);
		break;	
	case eCT_PingEntityDataBlockSize: // datablock size
		*data = uintdup(config->entry->size);
		break;	
	case eCT_PingEntityDSCP: // DSCP
		*data = uintdup(config->entry->dscp);
		break;	
	case eCT_PingEntityInterval: // Interval
		*data = uintdup(config->entry->interval);
		break;	
	case eCT_PingEntityStop: // Stop
		*data = booldup(config->stop);
		break;	
	default:
		return ERR_9005;
				
	}

	return 0;
}


int setCT_PingEntity(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	int num = 0;
	struct CT_PingConfig *config = NULL;
	struct CWMP_LINKNODE *config_entity = NULL;
	unsigned char update = 0;
	unsigned int *pNum = (unsigned int *)data;

	num = getInstNum(name, "X_CT-COM_Ping.PingConfig");
	config_entity = find_SiblingEntity( &gCT_ping_list, num);

	if(config_entity == NULL)
		return ERR_9005;

	config = (struct CT_PingConfig *)config_entity->obj_data;

	// sanity check
	if( (name==NULL) || (entity==NULL)) 
		return -1;
	if( entity->info->type!=type ) return ERR_9006;
	if(data == NULL) return ERR_9007;

	switch(getIndexOf(tCT_PingEntityLeaf, entity->info->name))
	{
	case eCT_PingEntityDiagnosticsState: //DiagnosticState
		{
			unsigned char enable;
	
			if (getStrIndexOf(strDiagnosticState, (char *)data) != STATE_REQUESTED)
				return ERR_9007;

			mib_get(CWMP_CT_PING_ENABLE, &enable);
			if(enable == 0)
				return ERR_9001;

			// already working
			if (STATE_REQUESTED == config->entry->diag_state)
				return ERR_9004;

			config->entry->diag_state = STATE_REQUESTED;
			break;
		}
	case eCT_PingEntityInterface: //Interface
		if (strlen((char *)data) > 256)
			return ERR_9007;
		strcpy(config->entry->interface, (char *)data);
		update = 1;
		break;
	case eCT_PingEntityHost: // Host
		if (strlen((char *)data) > 256)
			return ERR_9007;
		strcpy(config->entry->host, (char *)data);
		update = 1;
		break;
	case eCT_PingEntityNumberOfRepetitions: // #ofRepetitions
		if ((*pNum) < 0)
			return ERR_9007;
		
		config->entry->repetitions= *pNum;
		update = 1;
		break;	
	case eCT_PingEntityTimeout: // timeout
		if ((*pNum) < 1)
			return ERR_9007;
		
		config->entry->timeout= *pNum;
		update = 1;
		break;	
	case eCT_PingEntityDataBlockSize: // datablock size
		if ( ((*pNum) < 1) || ((*pNum) > 65535))
			return ERR_9007;
		
		config->entry->size = *pNum;
		update = 1;
		break;	
	case eCT_PingEntityDSCP: // DSCP
		if ((*pNum) > 64)
			return ERR_9007;
		
		config->entry->dscp = *pNum;
		update = 1;
		break;
	case eCT_PingEntityInterval:	//start
		if ((*pNum) < 0)
			return ERR_9007;

		config->entry->interval = *pNum;
		update = 1;
		break;
	case eCT_PingEntityStop:	//stop
		{
			int *i = data;
			if(*i == 1 && config->ping_pid != 0)
			{
				config->stop = 1;
			}
			else
				config->stop = 0;
			break;
		}
	default:
		return ERR_9005;
				
	}

	if(update)
		mib_chain_update(CWMP_CT_PING_TBL, config->entry, config->chain_id);
	
	return 0;

}


#endif	//_PRMT_X_CT_COM_PING_


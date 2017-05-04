#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <upnpdm.h>

#include "ctcom_upnpdm_proxy.h"

char *DEV_LIST_PATH = "InternetGatewayDevice.X_CT-COM_ProxyDevice.DeviceList.";
char *UPNPDM_PREFIX = "/UPnP/DM/";

/**
 * to: translated result.
 * len: length of to.
 * from: UPnP path.
 * dev_num: the number {i} in DeviceList.{i}
 *
 * return: 0 if success, -1 if error.
 */
int convert2cwmp_path(char *to, int len, char *from, unsigned int dev_num)
{
	char *postfix = NULL;
	char *upnp_path = NULL;
	int cwmp_idx = 0, upnp_idx = 0;

	if(to == NULL || from == NULL || dev_num < 1)
		return -1;

	upnp_idx = strlen(UPNPDM_PREFIX);
	if(strncmp(from, UPNPDM_PREFIX, upnp_idx) != 0)
	{
		fprintf(stderr, "<%s:%d> from is not start with %s\n", __FUNCTION__, __LINE__, UPNPDM_PREFIX);
		return -1;
	}

	memset(to, 0, len);
	snprintf(to, len, "%s%u.", DEV_LIST_PATH, dev_num);
	cwmp_idx = strlen(to);

	while(1)
	{
		if(from[upnp_idx] == '\0')
			break;

		if(cwmp_idx >= len)
		{
			fprintf(stderr, "<%s:%d> lenght of to is too small\n", __FUNCTION__, __LINE__);
			return -1;
		}

		switch(from[upnp_idx])
		{
		case '/':
			to[cwmp_idx] = '.';
			break;
		default:
			to[cwmp_idx] = from[upnp_idx];
			break;
		}
		upnp_idx++;
		cwmp_idx++;
	}
	return 0;
}

/**
 * to: translated result.
 * len: length of to.
 * from: CWMP path.
 *
 * return: 0 if success, -1 if error.
 */
int convert2upnpdm_path(char *to, int len, char *from)
{
	char *postfix = NULL;
	char *upnp_path = NULL;
	int cwmp_idx = 0, upnp_idx = 0, tmp;

	if(to == NULL || from == NULL)
		return -1;

	cwmp_idx = strlen(DEV_LIST_PATH);
	if(strncmp(from, DEV_LIST_PATH, cwmp_idx) != 0)
	{
		fprintf(stderr, "<%s:%d> from is not start with %s\n", __FUNCTION__, __LINE__, DEV_LIST_PATH);
		return -1;
	}

	//skip instance number
	tmp = cwmp_idx;
	while(isdigit(from[cwmp_idx]))
		cwmp_idx++;
	if(tmp >= cwmp_idx)
	{
		fprintf(stderr, "<%s:%d> Cannot find instance number in %s\n", __FUNCTION__, __LINE__, from);
		return -1;
	}
	if(from[cwmp_idx] != '.')
	{
		fprintf(stderr, "<%s:%d> Expect '.' but got '%c'\n", __FUNCTION__, __LINE__, from[cwmp_idx]);
		return -1;
	}
	cwmp_idx++;

	memset(to, 0, len);
	strncpy(to, UPNPDM_PREFIX, len);
	upnp_idx = strlen(to);

	while(1)
	{
		if(from[cwmp_idx] == '\0')
			break;

		if(upnp_idx >= len)
		{
			fprintf(stderr, "<%s:%d> lenght of to is too small\n", __FUNCTION__, __LINE__);
			return -1;
		}

		switch(from[cwmp_idx])
		{
		case '.':
			to[upnp_idx] = '/';
			break;
		default:
			to[upnp_idx] = from[cwmp_idx];
			break;
		}
		upnp_idx++;
		cwmp_idx++;
	}

	return 0;
}

void free_upnpdm_Reboot_result(struct upnpdm_Reboot_result *result)
{
	if(result == NULL)
		return;

	if(result->RebootStatus) free(result->RebootStatus);
	free(result);
}

void free_upnpdm_GetDeviceStatus_result(struct upnpdm_GetDeviceStatus_result *result)
{
	if(result == NULL)
		return;

	if(result->DeviceStatus) free(result->DeviceStatus);
	free(result);
}

void free_upnpdm_BaselineReset_result(struct upnpdm_BaselineReset_result *result)
{
	if(result) free(result);
}

void free_upnpdm_Download_result(struct upnpdm_Download_result *result)
{
	if(result) free(result);
}

void free_upnpdm_GetInstances_result(struct upnpdm_GetInstances_result *result)
{
	int i;
	if(result == NULL)
		return;

	if(result->InstancePathList)
	{
		for(i = 0 ; i < result->_size ; i++)
			free(result->InstancePathList[i]);

		free(result->InstancePathList);
	}
	free(result);
}

void free_upnpdm_GetValues_result(struct upnpdm_GetValues_result *result)
{
	int i;

	if(result == NULL || result->ParameterValueList == NULL)
		return ;

	for(i = 0 ; i < result->_size ; i++)
	{
		if(result->ParameterValueList[i].path) free(result->ParameterValueList[i].path);
		if(result->ParameterValueList[i].value) free(result->ParameterValueList[i].value);
	}
	free(result->ParameterValueList);
	free(result);
}

void free_upnpdm_SetValues_result(struct upnpdm_SetValues_result *result)
{
	if(result) free(result);
}

void free_upnpdm_CreateInstance_result(struct upnpdm_CreateInstance_result *result)
{
	if(result == NULL)
		return;

	if(result->InstanceIdentifier) free(result->InstanceIdentifier);
	free(result);
}

void free_upnpdm_DeleteInstance_result(struct upnpdm_DeleteInstance_result *result)
{
	if(result) free(result);
}

static void *deserialize_upnpdm_result_only_string(int sock)
{
	FILE *fp = NULL;
	struct upnpdm_Reboot_result *result = NULL;
	size_t len;
	int i;

	fp = fdopen(sock, "r");
	if(fp == NULL)
	{
		perror("<upnpdm_Reboot_result> fdopen");
		return NULL;
	}

	result = malloc(sizeof(struct upnpdm_Reboot_result));
	if(result == NULL)
	{
		perror("<upnpdm_Reboot_result> malloc");
		return NULL;
	}
	memset(result, 0, sizeof(struct upnpdm_Reboot_result));

	fread(&result->err_num, sizeof(result->err_num), 1, fp);
	fread(&result->_size, sizeof(result->_size), 1, fp);

	if(result->err_num == 0 && result->_size > 0)
		getdelim(&result->RebootStatus, &len, '\0',fp);
	else
		fprintf(stderr, "<%s:%d> got err_num = %d\n", __FUNCTION__, __LINE__, result->err_num);

	return (void *)result;
}

static struct upnpdm_Reboot_result *deserialize_upnpdm_Reboot_result(int sock)
{
	return (struct upnpdm_Reboot_result *)deserialize_upnpdm_result_only_string(sock);
}

static struct upnpdm_GetDeviceStatus_result *deserialize_upnpdm_GetDeviceStatus_result(int sock)
{
	return (struct upnpdm_GetDeviceStatus_result *)deserialize_upnpdm_result_only_string(sock);
}

static struct  upnpdm_BaselineReset_result *deserialize_upnpdm_BaselineReset_result(int sock)
{
	FILE *fp = NULL;
	struct upnpdm_BaselineReset_result *result = NULL;
	size_t len;
	int i;

	fp = fdopen(sock, "r");
	if(fp == NULL)
	{
		perror("<upnpdm_Reboot_result> fdopen");
		return NULL;
	}

	result = malloc(sizeof(struct upnpdm_BaselineReset_result));
	if(result == NULL)
	{
		perror("<upnpdm_Reboot_result> malloc");
		return NULL;
	}
	memset(result, 0, sizeof(struct upnpdm_BaselineReset_result));

	fread(&result->err_num, sizeof(result->err_num), 1, fp);
	fread(&result->_size, sizeof(result->_size), 1, fp);

	if(result->err_num != 0)
		fprintf(stderr, "<%s:%d> got err_num = %d\n", __FUNCTION__, __LINE__, result->err_num);

	return result;
}

static struct upnpdm_Download_result *deserialize_upnpdm_Download_result(int sock)
{
	FILE *fp = NULL;
	struct upnpdm_Download_result *result = NULL;
	int i;

	fp = fdopen(sock, "r");
	if(fp == NULL)
	{
		perror("<upnpdm_Download_result> fdopen");
		return NULL;
	}

	result = malloc(sizeof(struct upnpdm_Download_result));
	if(result == NULL)
	{
		perror("<upnpdm_Download_result> malloc");
		return NULL;
	}
	memset(result, 0, sizeof(struct upnpdm_Download_result));

	fread(&result->err_num, sizeof(result->err_num), 1, fp);
	fread(&result->_size, sizeof(result->_size), 1, fp);

	if(result->err_num == 0 && result->_size == 2)
	{
		char *buf = NULL;
		size_t len;

		getdelim(&buf, &len, '\0',fp);
		result->DownloadStatus = strtoul(buf, NULL, 10);
		if(buf) free(buf);
		buf = NULL;

		getdelim(&buf, &len, '\0',fp);
		result->OperaionID = strtoul(buf, NULL, 10);
		if(buf) free(buf);
	}
	else
	{
		fprintf(stderr, "<%s:%d> got err_num = %d\n", __FUNCTION__, __LINE__, result->err_num);
	}

	return (void *)result;
}

static struct upnpdm_GetInstances_result *deserialize_upnpdm_GetInstances_result(int sock)
{
	FILE *fp = NULL;
	struct upnpdm_GetInstances_result *result = NULL;
	size_t len;
	int i;

	fp = fdopen(sock, "r");
	if(fp == NULL)
	{
		perror("<upnpdm_GetInstances_result> fdopen");
		return NULL;
	}

	result = malloc(sizeof(struct upnpdm_GetInstances_result));
	if(result == NULL)
	{
		perror("<upnpdm_GetInstances_result> malloc");
		return NULL;
	}
	memset(result, 0, sizeof(struct upnpdm_GetInstances_result));

	fread(&result->err_num, sizeof(result->err_num), 1, fp);
	fread(&result->_size, sizeof(result->_size), 1, fp);

	if(result->err_num == 0 && result->_size > 0)
	{
		//fprintf(stderr, "<%s:%d> err_num= %d, Response %d path.\n", __FUNCTION__, __LINE__, result->err_num, result->_size);
		result->InstancePathList = malloc(sizeof(char *) * result->_size);
		memset(result->InstancePathList, 0, sizeof(char *) * result->_size);
		for(i = 0 ; i < result->_size ; i++)
		{
			getdelim(&result->InstancePathList[i], &len, '\0',fp);
			//fprintf(stderr, "<%s:%d> path[%d]=%s\n", __FUNCTION__, __LINE__, i, result->InstancePathList[i]);
		}
	}
	else
	{
		fprintf(stderr, "<%s:%d> got err_num = %d\n", __FUNCTION__, __LINE__, result->err_num);
	}

	return result;
}


static struct upnpdm_GetValues_result *deserialize_upnpdm_GetValues_result(int sock)
{
	FILE *fp = NULL;
	struct upnpdm_GetValues_result *result = NULL;
	size_t len;
	int i;

	fp = fdopen(sock, "r");
	if(fp == NULL)
	{
		perror("<upnpdm_GetValues_result> fdopen");
		return NULL;
	}

	result = malloc(sizeof(struct upnpdm_GetValues_result));
	if(result == NULL)
	{
		perror("<upnpdm_GetValues_result> malloc");
		return NULL;
	}
	memset(result, 0, sizeof(struct upnpdm_GetValues_result));

	fread(&result->err_num, sizeof(result->err_num), 1, fp);
	fread(&result->_size, sizeof(result->_size), 1, fp);

	if(result->err_num == 0 && result->_size > 0)
	{
		//fprintf(stderr, "<%s:%d> err_num= %d, Response %d parameters.\n", __FUNCTION__, __LINE__, result->err_num, result->_size);
		result->ParameterValueList = malloc(sizeof(struct upnpdm_parameter) * result->_size);
		memset(result->ParameterValueList, 0, sizeof(struct upnpdm_parameter) * result->_size);
		for(i = 0 ; i < result->_size ; i++)
		{
			getdelim(&result->ParameterValueList[i].path, &len, '\0',fp);
			getdelim(&result->ParameterValueList[i].value, &len, '\0',fp);
			//printf("%s=%s\n", result->ParameterValueList[i].path, result->ParameterValueList[i].value);
		}
	}
	else
	{
		fprintf(stderr, "<%s:%d> got err_num = %d\n", __FUNCTION__, __LINE__, result->err_num);
	}

	return result;
}

static void *deserialize_upnpdm_result_only_status(int sock)
{
	FILE *fp = NULL;
	struct upnpdm_SetValues_result *result = NULL;
	int i;

	fp = fdopen(sock, "r");
	if(fp == NULL)
	{
		perror("<upnpdm_SetValues_result> fdopen");
		return NULL;
	}

	result = malloc(sizeof(struct upnpdm_SetValues_result));
	if(result == NULL)
	{
		perror("<upnpdm_SetValues_result> malloc");
		return NULL;
	}
	memset(result, 0, sizeof(struct upnpdm_SetValues_result));

	fread(&result->err_num, sizeof(result->err_num), 1, fp);
	fread(&result->_size, sizeof(result->_size), 1, fp);

	if(result->err_num == 0 && result->_size > 0)
	{
		char *buf = NULL;
		size_t len;

		getdelim(&buf, &len, '\0',fp);
		if(strcmp(buf, "ChangesCommitted") == 0)
			result->Status = 1;
		if(buf) free(buf);
	}
	else
	{
		fprintf(stderr, "<%s:%d> got err_num = %d\n", __FUNCTION__, __LINE__, result->err_num);
	}

	return (void *)result;
}

static struct upnpdm_SetValues_result *deserialize_upnpdm_SetValues_result(int sock)
{
	return (struct upnpdm_SetValues_result *)deserialize_upnpdm_result_only_status(sock);
}

static struct upnpdm_DeleteInstance_result *deserialize_upnpdm_DeleteInstance_result(int sock)
{
	return (struct upnpdm_DeleteInstance_result *)deserialize_upnpdm_result_only_status(sock);
}

static struct upnpdm_CreateInstance_result *deserialize_upnpdm_CreateInstance_result(int sock)
{
	FILE *fp = NULL;
	struct upnpdm_CreateInstance_result *result = NULL;
	size_t len;
	int i;

	fp = fdopen(sock, "r");
	if(fp == NULL)
	{
		perror("<upnpdm_CreateInstance_result> fdopen");
		return NULL;
	}

	result = malloc(sizeof(struct upnpdm_CreateInstance_result));
	if(result == NULL)
	{
		perror("<upnpdm_CreateInstance_result> malloc");
		return NULL;
	}
	memset(result, 0, sizeof(struct upnpdm_CreateInstance_result));

	fread(&result->err_num, sizeof(result->err_num), 1, fp);
	fread(&result->_size, sizeof(result->_size), 1, fp);

	if(result->err_num == 0 && result->_size > 0)
	{
		char *buf = NULL;

		getdelim(&result->InstanceIdentifier, &len, '\0',fp);

		getdelim(&buf, &len, '\0',fp);
		if(strcmp(buf, "ChangesCommitted") == 0)
			result->Status = 1;
		if(buf) free(buf);
	}
	else
	{
		fprintf(stderr, "<%s:%d> got err_num = %d\n", __FUNCTION__, __LINE__, result->err_num);
	}

	return result;
}


// return result
void *upnpdm_action_process(struct upnpdm_action *action, int size)
{
	int sock = -1;
	struct sockaddr_un saun = {0};
	void *result = NULL;
	struct timeval timeout = {0};

	if(action == NULL)
		return NULL;

	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("<upnpdm_proxy> socket");
		return NULL;
	}

	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("setsockopt failed\n");

	if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		perror("setsockopt failed\n");

	saun.sun_family = AF_UNIX;
	strcpy(saun.sun_path, DEV_ACTION_FILE);

	if (connect(sock, (struct sockaddr *)&saun, sizeof(struct sockaddr_un)) < 0)
	{
		perror("<upnpdm_proxy> connect");
		return NULL;
	}

	// send action
	if(send(sock, action, size, 0) < 0)
		return NULL;

	switch(action->action)
	{
	case UPNPDM_BMS_Reboot:
		result = (void *)deserialize_upnpdm_Reboot_result(sock);
		break;
	case UPNPDM_BMS_BaselineReset:
		result = (void *)deserialize_upnpdm_BaselineReset_result(sock);
		break;
	case UPNPDM_BMS_GetDeviceStatus:
		result = (void *)deserialize_upnpdm_GetDeviceStatus_result(sock);
		break;
	case UPNPDM_BMS_Download:
		result = (void *)deserialize_upnpdm_Download_result(sock);
		break;
	case UPNPDM_CMS_GetInstances:
		result = (void *)deserialize_upnpdm_GetInstances_result(sock);
		break;
	case UPNPDM_CMS_GetValues:
		result = (void *)deserialize_upnpdm_GetValues_result(sock);
		break;
	case UPNPDM_CMS_CreateInstance:
		result = (void *)deserialize_upnpdm_CreateInstance_result(sock);
		break;
	case UPNPDM_CMS_SetValues:
	case UPNPDM_CMS_DeleteInstance:
		result = (void *)deserialize_upnpdm_result_only_status(sock);
		break;
	default:
		fprintf(stderr, "Unknown UPnP DM command: %d\n", action->action);
		break;
	}

	close(sock);
	return result;
}


struct upnpdm_Reboot_result *upnpdm_action_Reboot(char *uuid)
{
	struct upnpdm_action *action = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};

	if(uuid == NULL)
		return NULL;

	action = (struct upnpdm_action *)buf;
	strncpy(action->uuid, uuid, sizeof(action->uuid));
	action->action = UPNPDM_BMS_Reboot;

	snprintf(action->cmd, MAX_UPNPDM_IPC_BUF - sizeof(struct upnpdm_action)
		, "Reboot");

	return (struct upnpdm_Reboot_result *)upnpdm_action_process(action, sizeof(struct upnpdm_action) + strlen(action->cmd) + 1);
}

struct upnpdm_BaselineReset_result *upnpdm_action_BaselineReset(char *uuid)
{
	struct upnpdm_action *action = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};

	if(uuid == NULL)
		return;

	action = (struct upnpdm_action *)buf;
	strncpy(action->uuid, uuid, sizeof(action->uuid));
	action->action = UPNPDM_BMS_BaselineReset;

	snprintf(action->cmd, MAX_UPNPDM_IPC_BUF - sizeof(struct upnpdm_action)
		, "BaselineReset");

	return (struct upnpdm_BaselineReset_result *)upnpdm_action_process(action, sizeof(struct upnpdm_action) + strlen(action->cmd) + 1);
}

struct upnpdm_GetDeviceStatus_result *upnpdm_action_GetDeviceStatus(char *uuid)
{
	struct upnpdm_action *action = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};

	if(uuid == NULL)
		return NULL;

	action = (struct upnpdm_action *)buf;
	strncpy(action->uuid, uuid, sizeof(action->uuid));
	action->action = UPNPDM_BMS_GetDeviceStatus;

	snprintf(action->cmd, MAX_UPNPDM_IPC_BUF - sizeof(struct upnpdm_action)
		, "GetDeviceStatus");

	return (struct upnpdm_GetDeviceStatus_result *)upnpdm_action_process(action, sizeof(struct upnpdm_action) + strlen(action->cmd) + 1);
}

struct upnpdm_Download_result *upnpdm_action_Download(char *uuid, struct bms_download *download)
{
	struct upnpdm_action *action = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};

	if(uuid == NULL || download == NULL || download->URL == NULL || strlen(download->URL) == 0)
		return NULL;

	action = (struct upnpdm_action *)buf;
	strncpy(action->uuid, uuid, sizeof(action->uuid));
	action->action = UPNPDM_BMS_Download;

	if(download->Username && strlen(download->Username) && download->Password && strlen(download->Password))
		snprintf(action->cmd, MAX_UPNPDM_IPC_BUF - sizeof(struct upnpdm_action)
			, "X_CT-COM_Download %s %s %s", download->URL, download->Username, download->Password);
	else
		snprintf(action->cmd, MAX_UPNPDM_IPC_BUF - sizeof(struct upnpdm_action)
			, "X_CT-COM_Download %s", download->URL);

	return (struct upnpdm_Download_result *)upnpdm_action_process(action, sizeof(struct upnpdm_action) + strlen(action->cmd) + 1);
}

struct upnpdm_GetInstances_result *upnpdm_action_GetInstances(char *uuid, char *starting_node, int search_depth)
{
	struct upnpdm_action *action = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};

	if(uuid == NULL || starting_node == NULL)
		return NULL;

	action = (struct upnpdm_action *)buf;
	strncpy(action->uuid, uuid, sizeof(action->uuid));
	action->action = UPNPDM_CMS_GetInstances;

	snprintf(action->cmd, MAX_UPNPDM_IPC_BUF - sizeof(struct upnpdm_action)
		, "GetInstances %s %d", starting_node, search_depth);

	return (struct upnpdm_GetInstances_result *)upnpdm_action_process(action, sizeof(struct upnpdm_action) + strlen(action->cmd) + 1);
}


struct upnpdm_GetValues_result *upnpdm_action_GetValues(char *uuid, char *path)
{
	struct upnpdm_action *action = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};

	if(uuid == NULL || path == NULL)
		return NULL;

	action = (struct upnpdm_action *)buf;
	strncpy(action->uuid, uuid, sizeof(action->uuid));
	action->action = UPNPDM_CMS_GetValues;

	snprintf(action->cmd, MAX_UPNPDM_IPC_BUF - sizeof(struct upnpdm_action)
		, "GetValues %s", path);

	return (struct upnpdm_GetValues_result *)upnpdm_action_process(action, sizeof(struct upnpdm_action) + strlen(action->cmd) + 1);
}


struct upnpdm_SetValues_result *upnpdm_action_SetValues(char *uuid, char *path, char *value)
{
	struct upnpdm_action *action = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};

	if(uuid == NULL || path == NULL || value == NULL)
		return NULL;

	action = (struct upnpdm_action *)buf;
	strncpy(action->uuid, uuid, sizeof(action->uuid));
	action->action = UPNPDM_CMS_SetValues;

	snprintf(action->cmd, MAX_UPNPDM_IPC_BUF - sizeof(struct upnpdm_action)
		, "SetValues %s %s", path, value);

	return (struct upnpdm_SetValues_result *)upnpdm_action_process(action, sizeof(struct upnpdm_action) + strlen(action->cmd) + 1);
}



struct upnpdm_CreateInstance_result *upnpdm_action_CreateInstance(char *uuid, char *path)
{
	struct upnpdm_action *action = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};

	if(uuid == NULL || path == NULL)
		return NULL;

	action = (struct upnpdm_action *)buf;
	strncpy(action->uuid, uuid, sizeof(action->uuid));
	action->action = UPNPDM_CMS_CreateInstance;

	snprintf(action->cmd, MAX_UPNPDM_IPC_BUF - sizeof(struct upnpdm_action)
		, "CreateInstance %s", path);

	return (struct upnpdm_CreateInstance_result *)upnpdm_action_process(action, sizeof(struct upnpdm_action) + strlen(action->cmd) + 1);
}


struct upnpdm_DeleteInstance_result *upnpdm_action_DeleteInstance(char *uuid, char *path)
{
	struct upnpdm_action *action = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};

	if(uuid == NULL || path == NULL)
		return NULL;

	action = (struct upnpdm_action *)buf;
	strncpy(action->uuid, uuid, sizeof(action->uuid));
	action->action = UPNPDM_CMS_DeleteInstance;

	snprintf(action->cmd, MAX_UPNPDM_IPC_BUF - sizeof(struct upnpdm_action)
		, "DeleteInstance %s", path);

	return (struct upnpdm_DeleteInstance_result *)upnpdm_action_process(action, sizeof(struct upnpdm_action) + strlen(action->cmd) + 1);
}



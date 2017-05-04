#ifndef _CWMP_CTCOM_UPNPDM_PROXY_H_
#define _CWMP_CTCOM_UPNPDM_PROXY_H_


extern char *DEV_LIST_PATH;

/* Data Structures of Action results */
struct upnpdm_Reboot_result
{
	int err_num;
	int _size;
	char *RebootStatus;
};

struct upnpdm_BaselineReset_result
{
	int err_num;
	int _size;
};

struct upnpdm_GetDeviceStatus_result
{
	int err_num;
	int _size;
	char *DeviceStatus;
};

struct bms_download
{
	unsigned char invoked;
	char *FileType;
	char *URL;
	char *SoftwareVersion;
	char *HardwareVersion;
	int Channel;
	char *TimeWindow;
	char *Username;
	char *Password;
	unsigned char DownloadStatus;
	unsigned int OperationID;
};

struct upnpdm_Download_result
{
	int err_num;
	int _size;
	unsigned int DownloadStatus;
	unsigned int OperaionID;
};

struct upnpdm_GetInstances_result
{
	int err_num;
	int _size;	//number of instance path
	char **InstancePathList;
};

struct upnpdm_parameter
{
	char *path;
	char *value;
};

struct upnpdm_GetValues_result
{
	int err_num;
	int _size;	//number of parameters
	struct upnpdm_parameter *ParameterValueList;
};

struct upnpdm_SetValues_result
{
	int err_num;
	int _size;
	int Status;
};

struct upnpdm_CreateInstance_result
{
	int err_num;
	int _size;
	char *InstanceIdentifier;
	int Status;
};

struct upnpdm_DeleteInstance_result
{
	int err_num;
	int _size;
	int Status;
};


int convert2cwmp_path(char *to, int len, char *from, unsigned int dev_num);
int convert2upnpdm_path(char *to, int len, char *from);

void free_upnpdm_Reboot_result(struct upnpdm_Reboot_result *result);
void free_upnpdm_BaselineReset_result(struct upnpdm_BaselineReset_result *result);
void free_upnpdm_GetDeviceStatus_result(struct upnpdm_GetDeviceStatus_result *result);
void free_upnpdm_Download_result(struct upnpdm_Download_result *result);
void free_upnpdm_GetInstances_result(struct upnpdm_GetInstances_result *result);
void free_upnpdm_GetValues_result(struct upnpdm_GetValues_result *result);
void free_upnpdm_SetValues_result(struct upnpdm_SetValues_result *result);
void free_upnpdm_CreateInstance_result(struct upnpdm_CreateInstance_result *result);
void free_upnpdm_DeleteInstance_result(struct upnpdm_DeleteInstance_result *result);

struct upnpdm_Reboot_result *upnpdm_action_Reboot(char *uuid);
struct upnpdm_BaselineReset_result *upnpdm_action_BaselineReset(char *uuid);
struct upnpdm_GetDeviceStatus_result *upnpdm_action_GetDeviceStatus(char *uuid);
struct upnpdm_Download_result *upnpdm_action_Download(char *uuid, struct bms_download *download);
struct upnpdm_GetInstances_result *upnpdm_action_GetInstances(char *uuid, char *starting_node, int search_depth);
struct upnpdm_GetValues_result *upnpdm_action_GetValues(char *uuid, char *path);
struct upnpdm_SetValues_result *upnpdm_action_SetValues(char *uuid, char *path, char *value);
struct upnpdm_CreateInstance_result *upnpdm_action_CreateInstance(char *uuid, char *path);
struct upnpdm_DeleteInstance_result *upnpdm_action_DeleteInstance(char *uuid, char *path);

#endif /*_CWMP_CTCOM_UPNPDM_PROXY_H_*/

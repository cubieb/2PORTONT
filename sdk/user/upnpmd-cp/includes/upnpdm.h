#ifndef UPNP_DM_H
#define UPNP_DM_H
/* For UPNPDM IPC */

#define DEV_LISTENER_FILE "/tmp/upnpdm_dev_listener.sock"
#define DEV_ACTION_FILE "/tmp/upnpdm_action.sock"
#define MAX_UPNPDM_IPC_BUF      2048

enum upnpdm_cmd{
    UPNPDM_CMD_DEV_ONLINE = 1,
    UPNPDM_CMD_DEV_OFFLINE,
    UPNPDM_CMD_MAX,
};
struct upnpdm_ipc_msg
{
    unsigned int cmd;
    unsigned int data_len;
    char data[0];
};
struct upnpdm_dev_info
{
    unsigned char AttachedPort;
    char uuid[64];
    // got from SSDP
    char ManufacturerOUI[6+1];
    char SerialNumber[64+1];
    char ProductClass[64+1];
    char ProvisioningCode[64+1];
    char DeviceType[64+1];
    char SoftwareVersion[64+1];
    char HardwareVersion[64+1];
};

enum upnpdm_cms_action{
	UPNPDM_BMS_Reboot = 1,
	UPNPDM_BMS_BaselineReset,
	UPNPDM_BMS_GetDeviceStatus,
	UPNPDM_BMS_Download,
	UPNPDM_BMS_GetDownloadStatus,
	UPNPDM_CMS_GetSupportedDataModels = 101,
	UPNPDM_CMS_GetSupportedParameters,
	UPNPDM_CMS_GetInstances,
	UPNPDM_CMS_GetValues,
	UPNPDM_CMS_GetAttributes,
	UPNPDM_CMS_SetAttributes,
	UPNPDM_CMS_SetValues,
	UPNPDM_CMS_CreateInstance,
	UPNPDM_CMS_DeleteInstance,
	UPNPDM_CMS_GetNames,
	UPNPDM_ACTION_MAX,
};

struct upnpdm_action{    
	char uuid[64];    
	int action;    
	char cmd[0];
};

struct upnp_dm_action_commands{
	const char *str;
	int cmdnum;
	int numargs;
};

static struct upnp_dm_action_commands upnp_dm_action_command_list[]=
{
	{"Reboot", UPNPDM_BMS_Reboot, 1},
	{"BaselineReset", UPNPDM_BMS_BaselineReset, 1},
	{"GetDeviceStatus", UPNPDM_BMS_GetDeviceStatus, 1},
	{"X_CT-COM_Download", UPNPDM_BMS_Download, 4},
	{"X_CT-COM_GetDownloadStatus", UPNPDM_BMS_GetDownloadStatus, 1},
	{"GetSupportedDataModels", UPNPDM_CMS_GetSupportedDataModels, 1},
	{"GetSupportedParameters", UPNPDM_CMS_GetSupportedParameters, 3},
	{"GetInstances", UPNPDM_CMS_GetInstances, 3},
	{"GetValues", UPNPDM_CMS_GetValues, 2},
	{"GetAttributes", UPNPDM_CMS_GetAttributes, 2},
	{"SetValues", UPNPDM_CMS_SetValues, 3},
	{"CreateInstance", UPNPDM_CMS_CreateInstance, 2},
	{"DeleteInstance", UPNPDM_CMS_DeleteInstance, 2},
	{"X_CT-COM_GetNames", UPNPDM_CMS_GetNames, 3},
};

enum upnpdm_lan_port
{
    UPNPDM_LAN_PORT_NONE = 0,
    UPNPDM_LAN_PORT_1,
    UPNPDM_LAN_PORT_2,
    UPNPDM_LAN_PORT_3,
    UPNPDM_LAN_PORT_4,
    UPNPDM_WLAN_2G,
    UPNPDM_WLAN_5G,
};

#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <parameter_api.h>
#include <upnpdm.h>

#include "ctcom_upnpdm_proxy.h"
#include "prmt_ctcom_proxy_dev_list.h"

struct dev_list_obj_data
{
	unsigned char IsOnLine;
	unsigned char AttachedPort;
	char *UUID;
	char *ManufacturerOUI;
	char *SerialNumber;
	char *ProductClass;
	char *ProvisioningCode;
	char *DeviceType;
	char *SoftwareVersion;
	char *HardwareVersion;

	// BMS parameters
	unsigned char RebootInvoked;
	char *RebootStatus;

	unsigned char BaselineResetInvoked;

	unsigned char GetDevStatusInvoked;
	char *DeviceStatus;

	struct bms_download download;

	unsigned char GetDownloadStatusInvoked;
	char *OperationStatus;
};


/***** Utilities ******/
struct CWMP_NODE *gCT_device_list = NULL;
int device_cnt = 0;


int get_device_cnt(void)
{
	return device_cnt;
}

void upnpdm_check_download(void)
{
	struct CWMP_LINKNODE **ctable;
	struct dev_list_obj_data *data = NULL;
	struct bms_download *download = NULL;

	if (gCT_device_list == NULL || gCT_device_list->next == NULL)
		return;

	ctable = (struct CWMP_LINKNODE **)&gCT_device_list->next;
	while (*ctable)
	{
		data = (struct dev_list_obj_data *)(*ctable)->obj_data;

		if(data)
			download = &data->download;

		if(download->invoked && download->URL && strlen(download->URL) > 0)
		{
			struct upnpdm_Download_result *result = NULL;
			result = upnpdm_action_Download(data->UUID, download);

			if(result)
			{
				if(result->err_num == 0)
				{
					download->DownloadStatus = result->DownloadStatus;
					download->OperationID = result->OperaionID;

					CWMPDBP1("Download status = %u, id = %u\n", download->DownloadStatus, download->OperationID);
				}
				else
					CWMPDBP1("Downlaod error: %d\n", result->err_num);

				free_upnpdm_Download_result(result);
			}
			download->invoked = 0;
		}

		ctable = &(*ctable)->sibling;
	}
}



void free_CT_dev_list_obj(void *obj_data)
{
	struct dev_list_obj_data *data = (struct dev_list_obj_data *)obj_data;

	if(data == NULL)
		return;

	if(data->UUID) free(data->UUID);
	if(data->ManufacturerOUI) free(data->ManufacturerOUI);
	if(data->SerialNumber) free(data->SerialNumber);
	if(data->ProductClass) free(data->ProductClass);
	if(data->ProvisioningCode) free(data->ProvisioningCode);
	if(data->DeviceType) free(data->DeviceType);
	if(data->SoftwareVersion) free(data->SoftwareVersion);
	if(data->HardwareVersion) free(data->HardwareVersion);


	// BMS parameters
	if(data->RebootStatus) free(data->RebootStatus);

	if(data->DeviceStatus) free(data->DeviceStatus);

	if(data->download.FileType) free(data->download.FileType);
	if(data->download.URL) free(data->download.URL);
	if(data->download.SoftwareVersion) free(data->download.SoftwareVersion);
	if(data->download.HardwareVersion) free(data->download.HardwareVersion);
	if(data->download.TimeWindow) free(data->download.TimeWindow);
	if(data->download.Username) free(data->download.Username);
	if(data->download.Password) free(data->download.Password);

	if(data->OperationStatus) free(data->OperationStatus);

	free(data);
}

struct dev_list_obj_data *get_dev_obj_data(struct upnpdm_dev_info *dev)
{
	struct CWMP_LINKNODE **ctable;
	struct dev_list_obj_data *data = NULL;

	if (gCT_device_list == NULL || gCT_device_list->next == NULL)
		return NULL;

	ctable = (struct CWMP_LINKNODE **)&gCT_device_list->next;
	while (*ctable)
	{
		data = (struct dev_list_obj_data *)(*ctable)->obj_data;
	
		if(data && strcmp(data->ManufacturerOUI, dev->ManufacturerOUI) == 0
			&& strcmp(data->SerialNumber, dev->SerialNumber) == 0)
//			&& strcmp(data->SoftwareVersion, dev->SoftwareVersion) == 0)
			return data;

		ctable = &(*ctable)->sibling;
	}

	return NULL;
}

struct dev_list_obj_data *get_dev_obj_data_from_uuid(char *uuid)
{
	struct CWMP_LINKNODE **ctable;
	struct dev_list_obj_data *data = NULL;

	if (gCT_device_list == NULL || gCT_device_list->next == NULL)
		return NULL;

	ctable = (struct CWMP_LINKNODE **)&gCT_device_list->next;
	while (*ctable)
	{
		data = (struct dev_list_obj_data *)(*ctable)->obj_data;
	
		if(data && strcmp(data->UUID, uuid) == 0)
			return data;

		ctable = &(*ctable)->sibling;
	}

	return NULL;
}

/***** UPNPDM CMS paramter operations ****************************************/

/* Convert UPnP error number to CWMP error number */
int convert2cwmp_err(int err)
{
	if(err < 0)
		err = -err;

	switch(err)
	{
	case 604:
	case 606:
	case 607:
	case 608:
	case 609:
	case 610:
	case 611:
	case 612:
		return ERR_9001;
	case 501:
	case 603:
	case 803:
	case 804:
		return ERR_9002;
	case 401:
	case 402:
	case 602:
	case 707:
		return ERR_9003;
	case 605:
	case 708:
	case 709:
		return ERR_9004;
	case 701:
	case 702:
	case 703:
		return ERR_9005;
	case 704:
		return ERR_9006;
	case 600:
	case 601:
	case 801:
	case 705:
		return ERR_9007;
	case 706:
		return ERR_9008;
	default:
		CWMPDBP1("Unknown UPnP error number %d\n", err);
		return ERR_9000;
	break;
	}
}

int getCT_UPNPDM(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;
	char buf[512] = {0};
	struct upnpdm_GetValues_result *result = NULL;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	CWMPDBP2("GetValues via UPnP: %s\n", name);
	num = getInstNum(name, "DeviceList");

	//Attach object data
	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;
	if(convert2upnpdm_path(buf, sizeof(buf), name) < 0 )
		return ERR_9005;

	if(dev_data->IsOnLine)
		result = upnpdm_action_GetValues(dev_data->UUID, buf);

	if(result)
	{
		if(result->err_num != 0)
		{
			int err;
			CWMPDBP1("UPnP err_num = %d\n", result->err_num);
			err = convert2cwmp_err(result->err_num);
			free_upnpdm_GetValues_result(result);
			return err;
		}

		if(result->_size == 0)
		{
			CWMPDBP1("WARNING: No data & no errors\n");
			dev_data->IsOnLine = 0;
		}

		if(result->_size > 1)
			CWMPDBP1("WARNING: There's more than one result\n");
	}
	else
		dev_data->IsOnLine = 0;

	switch(*type)
	{
	case eCWMP_tSTRING:
		if(dev_data->IsOnLine && result->ParameterValueList[0].value)
		{
			*data = strdup(result->ParameterValueList[0].value);
			CWMPDBP2("Got string: %s\n", result->ParameterValueList[0].value);
		}
		else
			*data = strdup("");
		break;
	case eCWMP_tINT:
		if(dev_data->IsOnLine && result->ParameterValueList[0].value)
		{
			*data = intdup(atoi(result->ParameterValueList[0].value));
			CWMPDBP2("Convert integer: %s\n", result->ParameterValueList[0].value);
		}
		else
			*data = intdup(0);
		break;
	case eCWMP_tUINT:
		if(dev_data->IsOnLine && result->ParameterValueList[0].value)
		{
			*data = uintdup(strtoul(result->ParameterValueList[0].value, NULL, 10));
			CWMPDBP2("Convert unsigned integer: %s\n", result->ParameterValueList[0].value);
		}
		else
			*data = uintdup(0);
		break;
	case eCWMP_tBOOLEAN:
		if(dev_data->IsOnLine && result->ParameterValueList[0].value)
		{
			*data = intdup(atoi(result->ParameterValueList[0].value));
			CWMPDBP2("Convert boolean: %s\n", result->ParameterValueList[0].value);
		}
		else
			*data = booldup(0);
		
		break;
	case eCWMP_tDATETIME:
		{
			if(dev_data->IsOnLine && result->ParameterValueList[0].value)
			{
				struct tm ctime = {0};
			
				strptime(result->ParameterValueList[0].value, "%FT%T%z", &ctime);
				*data = timedup(mktime(&ctime));
				CWMPDBP2("Convert dateTime: %s\n", result->ParameterValueList[0].value);
			}
			else
				*data = timedup(0);
		}
		break;
	case eCWMP_tULONG:
		if(dev_data->IsOnLine && result->ParameterValueList[0].value)
		{
			*data = ulongdup(strtoul(result->ParameterValueList[0].value, NULL, 10));
			CWMPDBP2("Convert unsigned long: %s\n", result->ParameterValueList[0].value);
		}
		else
			*data = ulongdup(0);
		break;
	case eCWMP_tHEXBIN:
	case eCWMP_tBASE64:
	default:
		CWMPDBP1("Unhandled data type(%d): %s\n", *type, name);
		return ERR_9002;
		break;
	}
	free_upnpdm_GetValues_result(result);
	return 0;
}

int setCT_UPNPDM(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;
	struct upnpdm_SetValues_result *result = NULL;
	char path[512] = {0};
	char value[1024] = {0};
	int status = 0;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	CWMPDBP2("SetValues via UPnP: %s\n", name);

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	if(!dev_data->IsOnLine)
	{
		CWMPDBP1("<%s> is offlined\n", dev_data->UUID);
		return ERR_9005;
	}

	switch(type)
	{
	case eCWMP_tSTRING:
		{
			char *str = data;
			strncpy(value, data, sizeof(value));
		}
		break;
	case eCWMP_tINT:
	case eCWMP_tBOOLEAN:
		{
			int *i = data;
			snprintf(value, sizeof(value), "%d", *i);
		}
		break;
	case eCWMP_tUINT:
		{
			unsigned int *uint = data;
			snprintf(value, sizeof(value), "%u", *uint);
		}
		break;
	case eCWMP_tDATETIME:
		{
			time_t *t = data;
			struct tm ctime = {0};

			localtime_r(t, &ctime);
			strftime(value, sizeof(value), "%FT%T%z", &ctime);
		}
		break;
	case eCWMP_tULONG:
		{
			unsigned long *ulong = data;
			snprintf(value, sizeof(value), "%lu", *ulong);
		}
		break;
	case eCWMP_tHEXBIN:
	case eCWMP_tBASE64:
	default:
		CWMPDBP1("Unhandled data type(%d): %s\n", type, name);
		return ERR_9002;
		break;
	}

	if(convert2upnpdm_path(path, sizeof(path), name) < 0 )
		return ERR_9005;
	result = upnpdm_action_SetValues(dev_data->UUID, path, value);
	
	if(result)
	{
		if(result->err_num != 0)
		{
			CWMPDBP1("UPnP err_num = %d\n", result->err_num);
			return convert2cwmp_err(result->err_num);
		}

		if(result->_size == 0)
		{
			CWMPDBP1("WARNING: No data & no errors\n");
			dev_data->IsOnLine = 0;
		}

		if(result->_size > 1)
			CWMPDBP1("WARNING: There's more than one result\n");

		status = result->Status;

		free_upnpdm_SetValues_result(result);
	}
	else
		dev_data->IsOnLine = 0;

	return status;
}

struct CWMP_OP tCT_UPNPDMOP = {getCT_UPNPDM, setCT_UPNPDM};

/***** IGD.X_CT-COM_PorxyDevice.DeviceList.PhysicalDevice.DeviceID ***********/
struct CWMP_PRMT tCT_DeviceIDLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"ManufacturerOUI",	eCWMP_tSTRING,	CWMP_READ,			&tCT_UPNPDMOP},
{"ProductClass",		eCWMP_tSTRING,	CWMP_READ,			&tCT_UPNPDMOP},
{"SerialNumber",		eCWMP_tSTRING,	CWMP_READ,			&tCT_UPNPDMOP},
};

enum eCT_DeviceIDLeaf
{
	eCT_DeviceID_ManufacturerOUI,
	eCT_DeviceID_ProductClass,
	eCT_DeviceID_SerialNumber,
};

struct CWMP_LEAF tCT_DeviceIDLeaf[] =
{
{ &tCT_DeviceIDLeafInfo[eCT_DeviceID_ManufacturerOUI] },
{ &tCT_DeviceIDLeafInfo[eCT_DeviceID_ProductClass] },
{ &tCT_DeviceIDLeafInfo[eCT_DeviceID_SerialNumber] },
{ NULL }
};


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.PhysicalDevice. *******************/
struct CWMP_PRMT tCT_PhyDevLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"HardwareVersion",			eCWMP_tSTRING,	CWMP_READ,			&tCT_UPNPDMOP},
};

enum eCT_PhyDevLeaf
{
	eCT_PhyDev_HardwareVersion,
};

struct CWMP_LEAF tCT_PhyDevLeaf[] =
{
{ &tCT_PhyDevLeafInfo[eCT_PhyDev_HardwareVersion] },
{ NULL }
};

struct CWMP_PRMT tCT_PhyDevObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"DeviceID",	eCWMP_tOBJECT,	CWMP_READ, 	NULL},
};

enum eCT_PhyDevObject
{
	eCT_PhyDev_DeviceID,
};

struct CWMP_NODE tCT_PhyDevObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_PhyDevObjectInfo[eCT_PhyDev_DeviceID], 	tCT_DeviceIDLeaf,			NULL},
{NULL,					NULL,			NULL}
};


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.DeviceInfo. ***********************/
struct CWMP_PRMT tCT_DevList_DevInfoLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"FriendlyName",			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCT_UPNPDMOP},
{"DeviceType",			eCWMP_tSTRING,	CWMP_READ,			&tCT_UPNPDMOP},
{"ProvisioningCode",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCT_UPNPDMOP},
{"SoftwareVersion",		eCWMP_tSTRING,	CWMP_READ,			&tCT_UPNPDMOP},
{"SoftwareDescription",	eCWMP_tSTRING,	CWMP_READ,			&tCT_UPNPDMOP},
{"UpTime",				eCWMP_tUINT,		CWMP_READ,			&tCT_UPNPDMOP},
{"Mode",					eCWMP_tUINT,		CWMP_READ,			&tCT_UPNPDMOP},
};

enum eCT_DevList_DevInfoLeaf
{
	eCT_DevList_FriendlyName,
	eCT_DevList_DeviceType,
	eCT_DevList_ProvisioningCode,
	eCT_DevList_SoftwareVersion,
	eCT_DevList_SoftwareDescription,
	eCT_DevList_UpTime,
	eCT_DevList_Mode,
};

struct CWMP_LEAF tCT_DevList_DevInfoLeaf[] =
{
{ &tCT_DevList_DevInfoLeafInfo[eCT_DevList_FriendlyName] },
{ &tCT_DevList_DevInfoLeafInfo[eCT_DevList_DeviceType] },
{ &tCT_DevList_DevInfoLeafInfo[eCT_DevList_ProvisioningCode] },
{ &tCT_DevList_DevInfoLeafInfo[eCT_DevList_SoftwareVersion] },
{ &tCT_DevList_DevInfoLeafInfo[eCT_DevList_SoftwareDescription] },
{ &tCT_DevList_DevInfoLeafInfo[eCT_DevList_UpTime] },
{ &tCT_DevList_DevInfoLeafInfo[eCT_DevList_Mode] },
{ NULL }
};

struct CWMP_PRMT tCT_DevList_DevInfoObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"PhysicalDevice",	eCWMP_tOBJECT,	CWMP_READ, 	NULL},
};

enum eCT_DevList_DevInfoObject
{
	eCT_DevList_PhysicalDevice,
};

struct CWMP_NODE tCT_DevList_DevInfoObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_DevList_DevInfoObjectInfo[eCT_DevList_PhysicalDevice], 	tCT_PhyDevLeaf,	tCT_PhyDevObject},
{NULL,					NULL,			NULL}
};



/***** IGD.X_CT-COM_PorxyDevice.DeviceList.DeviceInfo.Monitoring.OperatingSystem. *****/
struct CWMP_PRMT tCT_Monitor_OSLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"CurrentTime",			eCWMP_tSTRING,	CWMP_READ,	&tCT_UPNPDMOP},
{"CPUUsage",			eCWMP_tSTRING,	CWMP_READ,	&tCT_UPNPDMOP},
{"MemoryUsage",		eCWMP_tSTRING,	CWMP_READ,	&tCT_UPNPDMOP},
};

enum eCT_Monitor_OSLeaf
{
	eCT_Monitor_OS_CurrentTime,
	eCT_Monitor_OS_CPUUsage,
	eCT_Monitor_OS_MemoryUsage,
};

struct CWMP_LEAF tCT_Monitor_OSLeaf[] =
{
{ &tCT_Monitor_OSLeafInfo[eCT_Monitor_OS_CurrentTime] },
{ &tCT_Monitor_OSLeafInfo[eCT_Monitor_OS_CPUUsage] },
{ &tCT_Monitor_OSLeafInfo[eCT_Monitor_OS_MemoryUsage] },
{ NULL }
};

/***** IGD.X_CT-COM_PorxyDevice.DeviceList.Monitoring. ***********************/
struct CWMP_PRMT tCT_MonitoringObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"OperatingSystem",	eCWMP_tOBJECT,	CWMP_READ, 	NULL},
};

enum eCT_MonitoringObject
{
	eCT_Monitoring_OperatingSystem,
};

struct CWMP_NODE tCT_MonitoringObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_MonitoringObjectInfo[eCT_Monitoring_OperatingSystem], 	tCT_Monitor_OSLeaf,	NULL},
{NULL,					NULL,			NULL}
};


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ApplyMode. *************/
struct CWMP_PRMT tCT_ApplyModeLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"Mode",			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCT_UPNPDMOP},
{"UserMessage",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,	&tCT_UPNPDMOP},
};

enum eCT_ApplyModeLeaf
{
	eCT_ApplyMode_Mode,
	eCT_ApplyMode_UserMessage,
};

struct CWMP_LEAF tCT_ApplyModeLeaf[] =
{
{ &tCT_ApplyModeLeafInfo[eCT_ApplyMode_Mode] },
{ &tCT_ApplyModeLeafInfo[eCT_ApplyMode_UserMessage] },
{ NULL }
};

/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ServiceObject.WLANConfiguration.{i}.AssociatedDevice. *****/
struct CWMP_PRMT tCT_AP_AscDeviceEntityLeafInfo[] =
{
/*(name,				type,		flag,				op)*/
{"AssociatedDeviceMACAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP},
{"AssociatedDeviceIPAddress",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP},
{"AssociatedDeviceAuthenticationState",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP},
{"LastRequestedUnicastCipher",		eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP},
{"LastRequestedMulticastCipher",	eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP},
{"LastPMKId",				eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP}
};
enum eCT_AP_AscDeviceEntityLeaf
{
	eAssociatedDeviceMACAddress,
	eAssociatedDeviceIPAddress,
	eAssociatedDeviceAuthenticationState,
	eLastRequestedUnicastCipher,
	eLastRequestedMulticastCipher,
	eLastPMKId
};
struct CWMP_LEAF tCT_AP_AscDeviceEntityLeaf[] =
{
{ &tCT_AP_AscDeviceEntityLeafInfo[eAssociatedDeviceMACAddress] },
{ &tCT_AP_AscDeviceEntityLeafInfo[eAssociatedDeviceIPAddress] },
{ &tCT_AP_AscDeviceEntityLeafInfo[eAssociatedDeviceAuthenticationState] },
{ &tCT_AP_AscDeviceEntityLeafInfo[eLastRequestedUnicastCipher] },
{ &tCT_AP_AscDeviceEntityLeafInfo[eLastRequestedMulticastCipher] },
{ &tCT_AP_AscDeviceEntityLeafInfo[eLastPMKId] },
{ NULL }
};


struct CWMP_PRMT tCT_AP_AscDeviceObjectInfo[] =
{
/*(name,			type,		flag,			op)*/
{"0",				eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
struct CWMP_LINKNODE tCT_AP_AscDeviceObject[] =
{
/*info,  				leaf,			next,		sibling,		instnum)*/
{&tCT_AP_AscDeviceObjectInfo[0],	tCT_AP_AscDeviceEntityLeaf,	NULL,		NULL,			0}
};

int objCT_AP_AscDevice(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, dev_num = 0, num = 0, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	struct dev_list_obj_data *dev_data = NULL;
	struct CWMP_LINKNODE *dev_node = NULL;

	if (name == NULL || entity == NULL)
		return -1;

	dev_num = getInstNum(name, "DeviceList");
	//Attach object data
	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, dev_num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	switch (type) {
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		return ERR_9001;
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;
			char buf[512] = {0};
			struct upnpdm_GetInstances_result *result = NULL;

			if(dev_data->IsOnLine == 0)
				return 0;

			convert2upnpdm_path(buf, 512, name);
			result = upnpdm_action_GetInstances(dev_data->UUID, buf, 1);
			if(result == NULL)
			{
				dev_data->IsOnLine= 0;
				return 0;
			}

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < result->_size ; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL, *tmp = NULL;

				convert2cwmp_path(buf, sizeof(buf), result->InstancePathList[i], dev_num);
				num = getInstNum(buf, "AssociatedDevice");

				if(num == 0)
					continue;

				// we have added this object, this is a bug of test device (WAP120NF).
				tmp = find_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, num);
				if(tmp != NULL)
					continue;

				remove_entity = remove_SiblingEntity((struct CWMP_LINKNODE **)&old_table, num);

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					add_Object(name,
						   (struct CWMP_LINKNODE **) &entity->next,
						   tCT_AP_AscDeviceObject,
						   sizeof(tCT_AP_AscDeviceObject),
						   &num);
				}
			}
			free_upnpdm_GetInstances_result(result);

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)old_table);

			break;
		}
		
		return 0;
	}

	return ret;
}


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ServiceObject.WLANConfiguration.{i}.WEPKey *****/
struct CWMP_PRMT tCT_AP_WEPKeyEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"WEPKey",			eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UPNPDMOP}
};
enum eCT_AP_WEPKeyEntityLeaf
{
	eWEPKey
};
struct CWMP_LEAF tCT_AP_WEPKeyEntityLeaf[] =
{
{ &tCT_AP_WEPKeyEntityLeafInfo[eWEPKey] },
{ NULL }
};


struct CWMP_PRMT tCT_AP_WEPKeyObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};

struct CWMP_LINKNODE tCT_AP_WEPKeyObject[] =
{
/*info, 				leaf,			next,		sibling,		instnum)*/
{&tCT_AP_WEPKeyObjectInfo[0],	tCT_AP_WEPKeyEntityLeaf,	NULL,		NULL,		0}
};

int objCT_AP_WEPKey(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, dev_num = 0, num = 0, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	struct dev_list_obj_data *dev_data = NULL;
	struct CWMP_LINKNODE *dev_node = NULL;
	char upnp_path[256] = {0};

	if (name == NULL || entity == NULL)
		return -1;

	dev_num = getInstNum(name, "DeviceList");
	//Attach object data
	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, dev_num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	if(convert2upnpdm_path(upnp_path, sizeof(upnp_path), name) < 0)
		return ERR_9005;

	switch (type) {
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		{
			struct upnpdm_CreateInstance_result *result = NULL;
			char cwmp_path[512] = {0};
			int inst_num = 0;

			if (data == NULL)
				return -1;

			if(!dev_data->IsOnLine)
				return ERR_9005;

			result = upnpdm_action_CreateInstance(dev_data->UUID, upnp_path);
			if(result == NULL)
			{
				dev_data->IsOnLine= 0;
				return ERR_9005;
			}

			if(result->err_num != 0 )
			{
				int err;
				err = convert2cwmp_err(result->err_num);
				free_upnpdm_CreateInstance_result(result);
				return err;
			}

			if(convert2cwmp_path(cwmp_path, sizeof(cwmp_path), result->InstanceIdentifier, dev_num) < 0)
			{
				free_upnpdm_CreateInstance_result(result);
				CWMPDBP2("ERROR: Invalid UPnP path: %s\n", result->InstanceIdentifier);
				return ERR_9005;
			}

			inst_num = getInstNum(cwmp_path, "WEPKey");
			if(inst_num == 0)
			{
				free_upnpdm_CreateInstance_result(result);
				CWMPDBP2("ERROR: Invalid path: %s\n", cwmp_path);
				return ERR_9005;
			}

			*(int *)data = inst_num;
			ret = add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_AP_WEPKeyObject,
				       sizeof(tCT_AP_WEPKeyObject), data);

			ret = result->Status;
			free_upnpdm_CreateInstance_result(result);
			break;
		}
	case eCWMP_tDELOBJ:
		{
			struct upnpdm_DeleteInstance_result *result = NULL;
			unsigned int *pUint = data;
			char target[256] = {0};

			if (data == NULL)
				return -1;

			if(!dev_data->IsOnLine)
				return ERR_9005;

			snprintf(target, sizeof(target), "%s/%u/", upnp_path, *pUint);
			result = upnpdm_action_DeleteInstance(dev_data->UUID, target);
			if(result == NULL)
			{
				dev_data->IsOnLine= 0;
				return ERR_9005;
			}

			if(result->err_num != 0 )
			{
				int err;
				err = convert2cwmp_err(result->err_num);
				free_upnpdm_DeleteInstance_result(result);
				return err;
			}

			del_Object(name, (struct CWMP_LINKNODE **) &entity->next, *(int *)data);

			ret = result->Status;
			free_upnpdm_DeleteInstance_result(result);
			break;
		}
	case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;
			struct upnpdm_GetInstances_result *result = NULL;

			if(dev_data->IsOnLine == 0)
				return 0;

			result = upnpdm_action_GetInstances(dev_data->UUID, upnp_path, 1);
			if(result == NULL)
			{
				dev_data->IsOnLine= 0;
				return 0;
			}

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < result->_size ; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL, *tmp = NULL;
				char buf[256] = {0};

				convert2cwmp_path(buf, sizeof(buf), result->InstancePathList[i], dev_num);
				num = getInstNum(buf, "WEPKey");		

				if(num == 0)
					continue;

				// we have added this object, this is a bug of test device (WAP120NF).
				tmp = find_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, num);
				if(tmp != NULL)
					continue;

				remove_entity = remove_SiblingEntity(&old_table, num);

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					add_Object(name,
						   (struct CWMP_LINKNODE **) &entity->next,
						   tCT_AP_WEPKeyObject,
						   sizeof(tCT_AP_WEPKeyObject),
						   &num);
				}
			}
			free_upnpdm_GetInstances_result(result);

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)old_table);

			break;
		}
		
		return 0;
	}

	return ret;
}



/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ServiceObject.WLANConfiguration.{i}.PresharedKey. *****/
struct CWMP_PRMT tCT_AP_PreSharedKeyEntityLeafInfo[] =
{
/*(name,			type,		flag,			op)*/
{"PreSharedKey",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UPNPDMOP},
{"KeyPassphrase",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UPNPDMOP},
{"AssociatedDeviceMACAddress",	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UPNPDMOP}
};
enum eCT_AP_PreSharedKeyEntityLeaf
{
	ePreSharedKey,
	eKeyPassphrase,
	ePreAssociatedDeviceMACAddress
};
struct CWMP_LEAF tCT_AP_PreSharedKeyEntityLeaf[] =
{
{ &tCT_AP_PreSharedKeyEntityLeafInfo[ePreSharedKey] },
{ &tCT_AP_PreSharedKeyEntityLeafInfo[eKeyPassphrase] },
{ &tCT_AP_PreSharedKeyEntityLeafInfo[ePreAssociatedDeviceMACAddress] },
{ NULL }
};

struct CWMP_PRMT tCT_AP_PreSharedKeyObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};

struct CWMP_LINKNODE tCT_AP_PreSharedKeyObject[] =
{
/*info, 				leaf,			next,		sibling,		instnum)*/
{&tCT_AP_PreSharedKeyObjectInfo[0],	tCT_AP_PreSharedKeyEntityLeaf,	NULL, 	NULL,	0}
};


int objCT_AP_PreSharedKey(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, dev_num = 0, num = 0, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	struct dev_list_obj_data *dev_data = NULL;
	struct CWMP_LINKNODE *dev_node = NULL;
	char upnp_path[256] = {0};

	if (name == NULL || entity == NULL)
		return -1;

	dev_num = getInstNum(name, "DeviceList");
	//Attach object data
	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, dev_num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	if(convert2upnpdm_path(upnp_path, sizeof(upnp_path), name) < 0)
		return ERR_9005;

	switch (type) {
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		{
			struct upnpdm_CreateInstance_result *result = NULL;
			char cwmp_path[512] = {0};
			int inst_num = 0;

			if (data == NULL)
				return -1;

			if(!dev_data->IsOnLine)
				return ERR_9005;

			result = upnpdm_action_CreateInstance(dev_data->UUID, upnp_path);
			if(result == NULL)
			{
				dev_data->IsOnLine= 0;
				return ERR_9005;
			}

			if(result->err_num != 0 )
			{
				int err;
				err = convert2cwmp_err(result->err_num);
				free_upnpdm_CreateInstance_result(result);
				return err;
			}

			if(convert2cwmp_path(cwmp_path, sizeof(cwmp_path), result->InstanceIdentifier, dev_num) < 0)
			{
				free_upnpdm_CreateInstance_result(result);
				CWMPDBP2("ERROR: Invalid UPnP path: %s\n", result->InstanceIdentifier);
				return ERR_9005;
			}

			inst_num = getInstNum(cwmp_path, "PreSharedKey");
			if(inst_num == 0)
			{
				free_upnpdm_CreateInstance_result(result);
				CWMPDBP2("ERROR: Invalid path: %s\n", cwmp_path);
				return ERR_9005;
			}

			*(int *)data = inst_num;
			ret = add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_AP_PreSharedKeyObject,
				       sizeof(tCT_AP_PreSharedKeyObject), data);

			ret = result->Status;
			free_upnpdm_CreateInstance_result(result);
			break;
		}
	case eCWMP_tDELOBJ:
		{
			struct upnpdm_DeleteInstance_result *result = NULL;
			unsigned int *pUint = data;
			char target[256] = {0};

			if (data == NULL)
				return -1;

			if(!dev_data->IsOnLine)
				return ERR_9005;

			snprintf(target, sizeof(target), "%s/%u/", upnp_path, *pUint);
			result = upnpdm_action_DeleteInstance(dev_data->UUID, target);
			if(result == NULL)
			{
				dev_data->IsOnLine= 0;
				return ERR_9005;
			}

			if(result->err_num != 0 )
			{
				int err;
				err = convert2cwmp_err(result->err_num);
				free_upnpdm_DeleteInstance_result(result);
				return err;
			}

			del_Object(name, (struct CWMP_LINKNODE **) &entity->next, *(int *)data);

			ret = result->Status;
			free_upnpdm_DeleteInstance_result(result);
			break;
		}
	case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;
			struct upnpdm_GetInstances_result *result = NULL;

			if(dev_data->IsOnLine == 0)
				return 0;

			result = upnpdm_action_GetInstances(dev_data->UUID, upnp_path, 1);
			if(result == NULL)
			{
				dev_data->IsOnLine= 0;
				return 0;
			}

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < result->_size ; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL, *tmp = NULL;
				char buf[512] = {0};

				convert2cwmp_path(buf, sizeof(buf), result->InstancePathList[i], dev_num);
				num = getInstNum(buf, "PreSharedKey");

				if(num == 0)
					continue;

				// we have added this object, this is a bug of test device (WAP120NF).
				tmp = find_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, num);
				if(tmp != NULL)
					continue;

				remove_entity = remove_SiblingEntity(&old_table, num);

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					add_Object(name,
						   (struct CWMP_LINKNODE **) &entity->next,
						   tCT_AP_PreSharedKeyObject,
						   sizeof(tCT_AP_PreSharedKeyObject),
						   &num);
				}
			}
			free_upnpdm_GetInstances_result(result);

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)old_table);

			break;
		}
		
		return 0;
	}

	return ret;
}


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ServiceObject.WLANConfiguration *****/
struct CWMP_OP tCT_AP_WLANConfOP = { NULL, objCT_AP_WLANConf };
struct CWMP_OP tCT_AP_AscDeviceOP = { NULL, objCT_AP_AscDevice };
struct CWMP_OP tCT_AP_WEPKeyOP = { NULL, objCT_AP_WEPKey };
struct CWMP_OP tCT_AP_PreSharedKeyOP = { NULL, objCT_AP_PreSharedKey };


struct CWMP_PRMT tCT_AP_WLANConfigurationEntityLeafInfo[] = 
{
/*(name,				type,		flag,			op)*/
{"TotalBytesSent",			eCWMP_tUINT,		CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP},
{"TotalBytesReceived",		eCWMP_tUINT,		CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP},
{"TotalPacketsSent",			eCWMP_tUINT,		CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP},
{"TotalPacketsReceived",		eCWMP_tUINT,		CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP},
{"WPAAuthenticationMode",		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_UPNPDMOP},
{"SSID",						eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,		&tCT_UPNPDMOP},
{"X_CT-COM_SSIDHide",			eCWMP_tBOOLEAN, 	CWMP_WRITE|CWMP_READ,		&tCT_UPNPDMOP},
{"X_CT-COM_RFBand", 			eCWMP_tUINT,		CWMP_WRITE|CWMP_READ, 	&tCT_UPNPDMOP},
{"VLAN",						eCWMP_tINT, 		CWMP_WRITE|CWMP_READ, 	&tCT_UPNPDMOP},
{"X_CT-COM_ChannelWidth",		eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,		&tCT_UPNPDMOP},
{"X_CT-COM_GuardInterval",		eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,		&tCT_UPNPDMOP},
{"X_CT-COM_RetryTimeout",		eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,		&tCT_UPNPDMOP},
{"X_CT-COM_Powerlevel",	 	eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,		&tCT_UPNPDMOP},
{"X_CT-COM_PowerValue", 		eCWMP_tUINT,		CWMP_READ,				&tCT_UPNPDMOP},
{"X_CT-COM_APModuleEnable",	eCWMP_tBOOLEAN, 	CWMP_WRITE|CWMP_READ,		&tCT_UPNPDMOP},
{"X_CT-COM_WPSKeyWord", 		eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,		&tCT_UPNPDMOP},
{"Enable",					eCWMP_tBOOLEAN, 	CWMP_WRITE|CWMP_READ,		&tCT_UPNPDMOP},
{"Channel", 					eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,		&tCT_UPNPDMOP},
{"ChannelsInUse",				eCWMP_tSTRING,	CWMP_READ|CWMP_DENY_ACT,	&tCT_UPNPDMOP},
{"BeaconType",				eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UPNPDMOP},
{"Standard",					eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UPNPDMOP},
{"WEPEncryptionLevel",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UPNPDMOP},
{"BasicAuthenticationMode", 	eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UPNPDMOP},
{"WPAEncryptionModes",		eCWMP_tSTRING,	CWMP_WRITE|CWMP_READ,	&tCT_UPNPDMOP},
{"WEPKeyIndex", 				eCWMP_tUINT,		CWMP_WRITE|CWMP_READ,	&tCT_UPNPDMOP},
{"AssociatedDeviceNumberofEntries", eCWMP_tUINT,		CWMP_READ,	&tCT_UPNPDMOP},
{"WEPKeyNumberofEntries", 		eCWMP_tUINT,		CWMP_READ,	NULL},
{"PreSharedKeyNumberofEntries",	eCWMP_tUINT,		CWMP_READ,	NULL},
};

enum eCT_AP_WLANConfigurationEntityLeaf
{
	eCT_AP_TotalBytesSent,
	eCT_AP_TotalBytesReceived,
	eCT_AP_TotalPacketsSent,
	eCT_AP_TotalPacketsReceived,
	eCT_AP_WPAAuthenticationMode,
	eCT_AP_SSID,
	eCT_AP_SSIDHide,
	eCT_AP_RFBand,
	eCT_AP_VLAN,
	eCT_AP_ChannelWidth,
	eCT_AP_GuardInterval,
	eCT_AP_RetryTimeout,
	eCT_AP_Powerlevel,
	eCT_AP_PowerValue,
	eCT_AP_APModuleEnable,
	eCT_AP_WPSKeyWord,
	eCT_AP_Enable,
	eCT_AP_Channel,
	eCT_AP_ChannelsInUse,
	eCT_AP_BeaconType,
	eCT_AP_Standard,
	eCT_AP_WEPEncryptionLevel,
	eCT_AP_BasicAuthenticationMode,
	eCT_AP_WPAEncryptionModes,
	eCT_AP_WEPKeyIndex,
	eCT_AP_AssociatedDeviceNumberofEntries,
	eCT_AP_WEPKeyNumberofEntries,
	eCT_AP_PreSharedKeyNumberofEntries,
};

struct CWMP_LEAF tCT_AP_WLANConfigurationEntityLeaf[] =
{
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_TotalBytesSent] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_TotalBytesReceived] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_TotalPacketsSent] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_TotalPacketsReceived] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_WPAAuthenticationMode] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_SSID] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_SSIDHide] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_RFBand] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_VLAN] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_ChannelWidth] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_GuardInterval] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_RetryTimeout] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_Powerlevel] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_PowerValue] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_APModuleEnable] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_WPSKeyWord] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_Enable] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_Channel] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_ChannelsInUse] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_BeaconType] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_Standard] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_WEPEncryptionLevel] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_BasicAuthenticationMode] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_WPAEncryptionModes] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_WEPKeyIndex] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_AssociatedDeviceNumberofEntries] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_WEPKeyNumberofEntries] },
{ &tCT_AP_WLANConfigurationEntityLeafInfo[eCT_AP_PreSharedKeyNumberofEntries] },
{ NULL }
};

struct CWMP_PRMT tCT_AP_WLANConfigurationEntityObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
/* Do not update AssociatedDevice becuase test device will be brocken. */
{"AssociatedDevice",	eCWMP_tOBJECT,	CWMP_READ, 			NULL/*&tCT_AP_AscDeviceOP*/},
{"WEPKey",			eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE,	&tCT_AP_WEPKeyOP},
{"PreSharedKey",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, &tCT_AP_PreSharedKeyOP},
};

enum eCT_AP_WLANConfigurationEntityObject
{
	eCT_AP_AssociatedDevice,
	eCT_AP_WEPKey,
	eCT_AP_PreSharedKey,
};

struct CWMP_NODE tCT_AP_WLANConfigurationEntityObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_AP_WLANConfigurationEntityObjectInfo[eCT_AP_AssociatedDevice], 	NULL,	NULL},
{&tCT_AP_WLANConfigurationEntityObjectInfo[eCT_AP_WEPKey],				NULL,	NULL},
{&tCT_AP_WLANConfigurationEntityObjectInfo[eCT_AP_PreSharedKey], 		NULL,	NULL},
{NULL,					NULL,			NULL}
};


struct CWMP_PRMT tCT_AP_WLANConfigurationObjectInfo[] =
{
/*(name,	type,		flag,		op)*/
{"0",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE|CWMP_LNKLIST,	NULL}
};

struct CWMP_LINKNODE tCT_AP_WLANConfigurationObject[] =
{
/*info, 				leaf,			next,		sibling,		instnum)*/
{&tCT_AP_WLANConfigurationObjectInfo[0],	tCT_AP_WLANConfigurationEntityLeaf,		tCT_AP_WLANConfigurationEntityObject, 	NULL,	0}
};

int objCT_AP_WLANConf(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, dev_num = 0, num = 0, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;
	struct dev_list_obj_data *dev_data = NULL;
	struct CWMP_LINKNODE *dev_node = NULL;
	char upnp_path[512] = {0};

	if (name == NULL || entity == NULL)
		return -1;

	dev_num = getInstNum(name, "DeviceList");
	//Attach object data
	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, dev_num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	if(convert2upnpdm_path(upnp_path, sizeof(upnp_path), name) < 0)
		return ERR_9005;

	switch (type) {
	case eCWMP_tINITOBJ:
		return 0;
	case eCWMP_tADDOBJ:
		{
			struct upnpdm_CreateInstance_result *result = NULL;
			char cwmp_path[512] = {0};
			int inst_num = 0;

			if (data == NULL)
				return -1;

			if(!dev_data->IsOnLine)
				return ERR_9005;

			result = upnpdm_action_CreateInstance(dev_data->UUID, upnp_path);
			if(result == NULL)
			{
				dev_data->IsOnLine= 0;
				return ERR_9005;
			}

			if(result->err_num != 0 )
			{
				int err;
				err = convert2cwmp_err(result->err_num);
				free_upnpdm_CreateInstance_result(result);
				return err;
			}

			if(convert2cwmp_path(cwmp_path, sizeof(cwmp_path), result->InstanceIdentifier, dev_num) < 0)
			{
				free_upnpdm_CreateInstance_result(result);
				CWMPDBP2("ERROR: Invalid UPnP path: %s\n", result->InstanceIdentifier);
				return ERR_9005;
			}

			inst_num = getInstNum(cwmp_path, "WLANConfiguration");
			if(inst_num == 0)
			{
				free_upnpdm_CreateInstance_result(result);
				CWMPDBP2("ERROR: Invalid path: %s\n", cwmp_path);
				return ERR_9005;
			}

			*(int *)data = inst_num;
			ret = add_Object(name,
				       (struct CWMP_LINKNODE **)&entity->next,
				       tCT_AP_WLANConfigurationObject,
				       sizeof(tCT_AP_WLANConfigurationObject), data);

			ret = result->Status;
			free_upnpdm_CreateInstance_result(result);
			break;
		}
	case eCWMP_tDELOBJ:
		{
			struct upnpdm_DeleteInstance_result *result = NULL;
			unsigned int *pUint = data;
			char target[256] = {0};

			if (data == NULL)
				return -1;

			if(!dev_data->IsOnLine)
				return ERR_9005;

			snprintf(target, sizeof(target), "%s/%u/", upnp_path, *pUint);
			result = upnpdm_action_DeleteInstance(dev_data->UUID, target);
			if(result == NULL)
			{
				dev_data->IsOnLine= 0;
				return ERR_9005;
			}

			if(result->err_num != 0 )
			{
				int err;
				err = convert2cwmp_err(result->err_num);
				free_upnpdm_DeleteInstance_result(result);
				return err;
			}

			del_Object(name, (struct CWMP_LINKNODE **) &entity->next, *(int *)data);

			ret = result->Status;
			free_upnpdm_DeleteInstance_result(result);
			break;
		}
	case eCWMP_tUPDATEOBJ:
		{
			struct CWMP_LINKNODE *old_table;
			struct upnpdm_GetInstances_result *result = NULL;

			if(dev_data->IsOnLine == 0)
				return 0;

			result = upnpdm_action_GetInstances(dev_data->UUID, upnp_path, 1);
			if(result == NULL)
			{
				dev_data->IsOnLine= 0;
				return 0;
			}

			old_table = (struct CWMP_LINKNODE *)entity->next;
			entity->next = NULL;

			for (i = 0; i < result->_size ; i++)
			{
				struct CWMP_LINKNODE *remove_entity = NULL, *tmp = NULL;
				char buf[512] = {0};

				convert2cwmp_path(buf, sizeof(buf), result->InstancePathList[i], dev_num);
				num = getInstNum(buf, "WLANConfiguration");

				if(num == 0)
					continue;

				// we have added this object, this is a bug of test device (WAP120NF).
				tmp = find_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, num);
				if(tmp != NULL)
					continue;

				remove_entity = remove_SiblingEntity(&old_table, num);

				if (remove_entity != NULL)
				{
					add_SiblingEntity((struct CWMP_LINKNODE **)&entity->next, remove_entity);
				}
				else
				{
					add_Object(name,
						   (struct CWMP_LINKNODE **) &entity->next,
						   tCT_AP_WLANConfigurationObject,
						   sizeof(tCT_AP_WLANConfigurationObject),
						   &num);
				}
			}
			free_upnpdm_GetInstances_result(result);

			if (old_table)
				destroy_ParameterTable((struct CWMP_NODE *)old_table);

			break;
		}
	}

	return ret;
}


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ServiceObject. ********************/
struct CWMP_PRMT tCT_ServiceObjectLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"WLANConfigurationNumberOfEntries",	eCWMP_tUINT,	CWMP_READ,	&tCT_UPNPDMOP},
};

enum eCT_ServiceObjectLeaf
{
	eCT_SrvObj_WLANConfigurationNumberOfEntries,
};

struct CWMP_LEAF tCT_AP_ServiceObjectLeaf[] =
{
{ &tCT_ServiceObjectLeafInfo[eCT_SrvObj_WLANConfigurationNumberOfEntries] },
{ NULL }
};

struct CWMP_PRMT tCT_ServiceOjecetObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"WLANConfiguration",		eCWMP_tOBJECT,	CWMP_READ|CWMP_WRITE, 	&tCT_AP_WLANConfOP},
};

enum eCT_ServiceObject
{
	eCT_ServiceObject_WLANConfiguration,
};

struct CWMP_NODE tCT_AP_ServiceObjectObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_ServiceOjecetObjectInfo[eCT_ServiceObject_WLANConfiguration], 	NULL,	NULL},
{NULL,					NULL,			NULL}
};


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS.Reboot. ************/
struct CWMP_OP tCT_BMS_RebootOP = {getCT_BMS_Reboot, setCT_BMS_Reboot};

struct CWMP_PRMT tCT_BMS_RebootLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"InvokeAction",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_RebootOP},
{"RebootStatus",	eCWMP_tSTRING,	CWMP_READ,				&tCT_BMS_RebootOP},
};

enum eCT_BMS_RebootLeaf
{
	eCT_BMS_Reboot_InvokeAction,
	eCT_BMS_Reboot_RebootStatus,
};

struct CWMP_LEAF tCT_BMS_RebootLeaf[] =
{
{ &tCT_BMS_RebootLeafInfo[eCT_BMS_Reboot_InvokeAction] },
{ &tCT_BMS_RebootLeafInfo[eCT_BMS_Reboot_RebootStatus] },
{ NULL }
};

int getCT_BMS_Reboot(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "InvokeAction") == 0)
	{
		*data = booldup(dev_data->RebootInvoked);
	}
	else if (strcmp(lastname, "RebootStatus") == 0)
	{
		if(dev_data->RebootStatus)
			*data = strdup(dev_data->RebootStatus);
		else
			*data = strdup("");
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_BMS_Reboot(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	if (strcmp(lastname, "InvokeAction") == 0)
	{
		int *i = data;
		struct upnpdm_Reboot_result *result = NULL;

		if(*i == 0)
			return 0;

		if(dev_data->IsOnLine == 0)
			return ERR_9005;

		dev_data->RebootInvoked = 1;

		result = upnpdm_action_Reboot(dev_data->UUID);
		if(result == NULL)
		{
			dev_data->IsOnLine = 0;
			dev_data->RebootInvoked = 0;
			return ERR_9005;
		}

		if(dev_data->RebootStatus) free(dev_data->RebootStatus);
		dev_data->RebootStatus = strdup(result->RebootStatus);
		free_upnpdm_Reboot_result(result);
		dev_data->RebootInvoked = 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS.BaselineReset. *****/
struct CWMP_OP tCT_BMS_BaselineResetOP = {getCT_BMS_BaselineReset, setCT_BMS_BaselineReset};

struct CWMP_PRMT tCT_BMS_BaselineResetLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"InvokeAction",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_BaselineResetOP},
};

enum eCT_BMS_BaselineResetLeaf
{
	eCT_BMS_BaselineReset_InvokeAction,
};

struct CWMP_LEAF tCT_BMS_BaselineResetLeaf[] =
{
{ &tCT_BMS_BaselineResetLeafInfo[eCT_BMS_BaselineReset_InvokeAction] },
{ NULL }
};

int getCT_BMS_BaselineReset(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "InvokeAction") == 0)
	{
		*data = booldup(dev_data->BaselineResetInvoked);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_BMS_BaselineReset(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	if (strcmp(lastname, "InvokeAction") == 0)
	{
		int *i = data;
		struct  upnpdm_BaselineReset_result *result = NULL;

		if(*i == 0)
			return 0;

		if(dev_data->IsOnLine == 0)
			return ERR_9005;

		dev_data->BaselineResetInvoked = 1;

		result = upnpdm_action_BaselineReset(dev_data->UUID);
		free_upnpdm_BaselineReset_result(result);

		dev_data->BaselineResetInvoked = 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS.GetDeviceStatus. ************/
struct CWMP_OP tCT_BMS_GetDevStatusOP = {getCT_BMS_GetDevStatus, setCT_BMS_GetDevStatus};

struct CWMP_PRMT tCT_BMS_GetDevStatusLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"InvokeAction",	eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_GetDevStatusOP},
{"DeviceStatus",	eCWMP_tSTRING,	CWMP_READ,				&tCT_BMS_GetDevStatusOP},
};

enum eCT_BMS_GetDevStatusLeaf
{
	eCT_BMS_GetDevStatus_InvokeAction,
	eCT_BMS_GetDevStatus_DeviceStatus,
};

struct CWMP_LEAF tCT_BMS_GetDevStatusLeaf[] =
{
{ &tCT_BMS_GetDevStatusLeafInfo[eCT_BMS_GetDevStatus_InvokeAction] },
{ &tCT_BMS_GetDevStatusLeafInfo[eCT_BMS_GetDevStatus_DeviceStatus] },
{ NULL }
};

int getCT_BMS_GetDevStatus(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "InvokeAction") == 0)
	{
		*data = booldup(dev_data->GetDevStatusInvoked);
	}
	else if (strcmp(lastname, "DeviceStatus") == 0)
	{
		if(dev_data->DeviceStatus)
			*data = strdup(dev_data->DeviceStatus);
		else
			*data = strdup("");
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_BMS_GetDevStatus(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	if (strcmp(lastname, "InvokeAction") == 0)
	{
		int *i = data;
		struct upnpdm_GetDeviceStatus_result *result = NULL;

		if(*i == 0)
			return 0;

		if(dev_data->IsOnLine == 0)
			return ERR_9005;

		dev_data->GetDevStatusInvoked= 1;

		result = upnpdm_action_GetDeviceStatus(dev_data->UUID);
		if(result == NULL)
		{
			dev_data->IsOnLine = 0;
			dev_data->GetDevStatusInvoked = 0;
			return ERR_9005;
		}

		if(dev_data->DeviceStatus) free(dev_data->DeviceStatus);
		dev_data->DeviceStatus = strdup(result->DeviceStatus);
		free_upnpdm_GetDeviceStatus_result(result);
		dev_data->GetDevStatusInvoked = 0;
	}
	else {
		return ERR_9005;
	}

	return 0;
}




/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS.X_CT-COM_Download. *****/
struct CWMP_OP tCT_BMS_DownloadOP = {getCT_BMS_Download, setCT_BMS_Download};

struct CWMP_PRMT tCT_BMS_DownloadLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"InvokeAction",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_DownloadOP},
{"FileType",			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_DownloadOP},
{"URL",				eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_DownloadOP},
{"SoftwareVersion",	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_DownloadOP},
{"HardwareVersion", 	eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_DownloadOP},
{"Channel", 			eCWMP_tINT,		CWMP_READ|CWMP_WRITE,		&tCT_BMS_DownloadOP},
{"TimeWindow", 		eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_DownloadOP},
{"Username", 			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_DownloadOP},
{"Password", 			eCWMP_tSTRING,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_DownloadOP},
{"DownloadStatus", 	eCWMP_tUINT, 		CWMP_READ,				&tCT_BMS_DownloadOP},
{"OperationID", 		eCWMP_tUINT, 		CWMP_READ,				&tCT_BMS_DownloadOP},
};

enum eCT_BMS_DownloadLeaf
{
	eCT_BMS_Download_InvokeAction,
	eCT_BMS_Download_FileType,
	eCT_BMS_Download_URL,
	eCT_BMS_Download_SoftwareVersion,
	eCT_BMS_Download_HardwareVersion,
	eCT_BMS_Download_Channel,
	eCT_BMS_Download_TimeWindow,
	eCT_BMS_Download_Username,
	eCT_BMS_Download_Password,
	eCT_BMS_Download_DownloadStatus,
	eCT_BMS_Download_OperationID,
};

struct CWMP_LEAF tCT_BMS_DownloadLeaf[] =
{
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_InvokeAction] },
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_FileType] },
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_URL] },
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_SoftwareVersion] },
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_HardwareVersion] },
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_Channel] },
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_TimeWindow] },
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_Username] },
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_Password] },
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_DownloadStatus] },
{ &tCT_BMS_DownloadLeafInfo[eCT_BMS_Download_OperationID] },
{ NULL }
};

int getCT_BMS_Download(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "InvokeAction") == 0)
	{
		*data = booldup(dev_data->download.invoked);
	}
	else if (strcmp(lastname, "FileType") == 0)
	{
		if(dev_data->download.FileType)
			*data = strdup(dev_data->download.FileType);
		else
			*data = strdup("");
	}
	else if (strcmp(lastname, "URL") == 0)
	{
		if(dev_data->download.URL)
			*data = strdup(dev_data->download.URL);
		else
			*data = strdup("");
	}
	else if (strcmp(lastname, "SoftwareVersion") == 0)
	{
		if(dev_data->download.SoftwareVersion)
			*data = strdup(dev_data->download.SoftwareVersion);
		else
			*data = strdup("");
	}
	else if (strcmp(lastname, "HardwareVersion") == 0)
	{
		if(dev_data->download.HardwareVersion)
			*data = strdup(dev_data->download.HardwareVersion);
		else
			*data = strdup("");
	}
	else if (strcmp(lastname, "Channel") == 0)
	{
		*data = intdup(dev_data->download.Channel);
	}
	else if (strcmp(lastname, "TimeWindow") == 0)
	{
		if(dev_data->download.TimeWindow)
			*data = strdup(dev_data->download.TimeWindow);
		else
			*data = strdup("");
	}
	else if (strcmp(lastname, "Username") == 0)
	{
		if(dev_data->download.Username)
			*data = strdup(dev_data->download.Username);
		else
			*data = strdup("");
	}
	else if (strcmp(lastname, "Password") == 0)
	{
		if(dev_data->download.Password)
			*data = strdup(dev_data->download.Password);
		else
			*data = strdup("");
	}
	else if (strcmp(lastname, "DownloadStatus") == 0)
	{
		*data = uintdup(dev_data->download.DownloadStatus);
	}
	else if (strcmp(lastname, "OperationID") == 0)
	{
		*data = uintdup(dev_data->download.OperationID);
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_BMS_Download(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	if (strcmp(lastname, "InvokeAction") == 0)
	{
		int *i = data;
		unsigned char invoke=0;

		dev_data->download.invoked = *i;
		//TODO: Invoke Download
	}
	else if (strcmp(lastname, "FileType") == 0)
	{
		char *buf = data;

		dev_data->download.FileType = strdup(buf);
	}
	else if (strcmp(lastname, "URL") == 0)
	{
		char *buf = data;

		dev_data->download.URL = strdup(buf);
	}
	else if (strcmp(lastname, "SoftwareVersion") == 0)
	{
		char *buf = data;

		dev_data->download.SoftwareVersion = strdup(buf);
	}
	else if (strcmp(lastname, "HardwareVersion") == 0)
	{
		char *buf = data;

		dev_data->download.HardwareVersion = strdup(buf);
	}
	else if (strcmp(lastname, "Channel") == 0)
	{
		int *channel = data;

		dev_data->download.Channel = *channel;
	}
	else if (strcmp(lastname, "TimeWindow") == 0)
	{
		char *buf = data;

		dev_data->download.TimeWindow = strdup(buf);
	}
	else if (strcmp(lastname, "Username") == 0)
	{
		char *buf = data;

		dev_data->download.Username = strdup(buf);
	}
	else if (strcmp(lastname, "Password") == 0)
	{
		char *buf = data;

		dev_data->download.Password = strdup(buf);
	}
	else {
		return ERR_9005;
	}

	return 0;
}



/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS.GetDownloadStatus. ************/
struct CWMP_OP tCT_BMS_GetDownloadStatusOP = {getCT_BMS_GetDownloadStatus, setCT_BMS_GetDownloadStatus};

struct CWMP_PRMT tCT_BMS_GetDownloadStatusLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"InvokeAction",		eCWMP_tBOOLEAN,	CWMP_READ|CWMP_WRITE,		&tCT_BMS_GetDownloadStatusOP},
{"OperationStatus",	eCWMP_tSTRING,	CWMP_READ,				&tCT_BMS_GetDownloadStatusOP},
};

enum eCT_BMS_GetDownloadStatusLeaf
{
	eCT_BMS_GetDownloadStatus_InvokeAction,
	eCT_BMS_GetDownloadStatus_OperationStatus,
};

struct CWMP_LEAF tCT_BMS_GetDownloadStatusLeaf[] =
{
{ &tCT_BMS_GetDownloadStatusLeafInfo[eCT_BMS_GetDownloadStatus_InvokeAction] },
{ &tCT_BMS_GetDownloadStatusLeafInfo[eCT_BMS_GetDownloadStatus_OperationStatus] },
{ NULL }
};

int getCT_BMS_GetDownloadStatus(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "InvokeAction") == 0)
	{
		*data = booldup(dev_data->GetDownloadStatusInvoked);
	}
	else if (strcmp(lastname, "GetDownloadStatusStatus") == 0)
	{
		if(dev_data->OperationStatus)
			*data = strdup(dev_data->OperationStatus);
		else
			*data = strdup("");
	}
	else {
		return ERR_9005;
	}

	return 0;
}

int setCT_BMS_GetDownloadStatus(char *name, struct CWMP_LEAF *entity, int type, void *data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (data == NULL))
		return -1;
	if (entity->info->type != type)
		return ERR_9006;

	num = getInstNum(name, "DeviceList");

	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	if (strcmp(lastname, "InvokeAction") == 0)
	{
		int *i = data;
		unsigned char invoke=0;

		dev_data->GetDownloadStatusInvoked = *i;
		//TODO: Invoke GetDownloadStatus
	}
	else {
		return ERR_9005;
	}

	return 0;
}



/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList.BMS. *******************/
struct CWMP_PRMT tCT_BMSObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"Reboot",				eCWMP_tOBJECT,	CWMP_READ, 	NULL},
{"BaselineReset",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"GetDeviceStatus",		eCWMP_tOBJECT,	CWMP_READ, 	NULL},
{"X_CT-COM_Download",		eCWMP_tOBJECT,	CWMP_READ, 	NULL},
{"X_CT-COM_GetDownloadStatus",		eCWMP_tOBJECT,	CWMP_READ, 	NULL},
};

enum eCT_BMSObject
{
	eCT_BMS_Reboot,
	eCT_BMS_BaselineReset,
	eCT_BMS_GetDeviceStatus,
	eCT_BMS_Download,
	eCT_BMS_GetDownloadStatus,
};

struct CWMP_NODE tCT_BMSObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_BMSObjectInfo[eCT_BMS_Reboot], 			tCT_BMS_RebootLeaf,	NULL},
{&tCT_BMSObjectInfo[eCT_BMS_BaselineReset],		tCT_BMS_BaselineResetLeaf,	NULL},
{&tCT_BMSObjectInfo[eCT_BMS_GetDeviceStatus], 	tCT_BMS_GetDevStatusLeaf,	NULL},
{&tCT_BMSObjectInfo[eCT_BMS_Download], 			tCT_BMS_DownloadLeaf,	NULL},
{&tCT_BMSObjectInfo[eCT_BMS_GetDownloadStatus], 	tCT_BMS_GetDownloadStatusLeaf,	NULL},
{NULL,					NULL,			NULL}
};


/***** IGD.X_CT-COM_PorxyDevice.DeviceList.ActionList ************************/
struct CWMP_PRMT tCT_ActionListObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"BMS",		eCWMP_tOBJECT,	CWMP_READ, 	NULL},
};

enum eCT_ActionListObject
{
	eCT_ActionList_BMS,
};

struct CWMP_NODE tCT_ActionListObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_ActionListObjectInfo[eCT_ActionList_BMS], 	NULL,	tCT_BMSObject},
{NULL,					NULL,			NULL}
};



/***** IGD.X_CT-COM_PorxyDevice.DeviceList. **********************************/
struct CWMP_OP tCT_DeviceListOP = {NULL, objCT_DeviceList};
struct CWMP_OP tCT_DeviceListEntityOP = {getCT_DeviceListEntity, NULL};

struct CWMP_PRMT tCT_DeviceListEntityLeafInfo[] = 
{
/*(name,		type,		flag,				op)*/
{"IsOnLine",		eCWMP_tBOOLEAN,	CWMP_READ,		&tCT_DeviceListEntityOP},
{"AttachedPort",	eCWMP_tSTRING,	CWMP_READ,		&tCT_DeviceListEntityOP},
};

enum eCT_DeviceListEntityLeaf
{
	eCT_DeviceList_IsOnline,
	eCT_DeviceList_AttachedPort,
};

struct CWMP_LEAF tCT_DeviceListEntityLeaf[] =
{
{ &tCT_DeviceListEntityLeafInfo[eCT_DeviceList_IsOnline] },
{ &tCT_DeviceListEntityLeafInfo[eCT_DeviceList_AttachedPort] },
{ NULL }
};

struct CWMP_PRMT tCT_DeviceListEntityObjectInfo[] =
{
/*(name,		type,		flag,			op)*/
{"DeviceInfo",		eCWMP_tOBJECT,	CWMP_READ, 	NULL},
{"Monitoring",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"ApplyMode",			eCWMP_tOBJECT,	CWMP_READ,	NULL},
{"ServiceObject",		eCWMP_tOBJECT,	CWMP_READ, 	NULL},
{"ActionList",		eCWMP_tOBJECT,	CWMP_READ,	NULL},
};

enum eCT_DeviceListEntityObject
{
	eCT_DevList_DeviceInfo,
	eCT_DevList_Monitoring,
	eCT_DevList_ApplyMode,
	eCT_DevList_ServiceObject,
	eCT_DevList_ActionList,
};

struct CWMP_NODE tCT_DeviceListEntityObject[] =
{
/*info,  				leaf,			node)*/
{&tCT_DeviceListEntityObjectInfo[eCT_DevList_DeviceInfo], 	tCT_DevList_DevInfoLeaf,	tCT_DevList_DevInfoObject},
{&tCT_DeviceListEntityObjectInfo[eCT_DevList_Monitoring],		NULL,			tCT_MonitoringObject},
{&tCT_DeviceListEntityObjectInfo[eCT_DevList_ApplyMode], 		tCT_ApplyModeLeaf,	NULL},
{&tCT_DeviceListEntityObjectInfo[eCT_DevList_ServiceObject],	NULL,			NULL},
{&tCT_DeviceListEntityObjectInfo[eCT_DevList_ActionList],		NULL,			tCT_ActionListObject},
{NULL,					NULL,			NULL}
};

struct CWMP_PRMT tCT_DeviceListObjectInfo[] =
{
/*(name,		type,		flag,					op)*/
{"0",			eCWMP_tOBJECT,	CWMP_READ|CWMP_LNKLIST,	NULL}
};
struct CWMP_LINKNODE tCT_DeviceListObject[] =
{
/*info,  					leaf,			next,		sibling,		instnum)*/
{&tCT_DeviceListObjectInfo[0], 	tCT_DeviceListEntityLeaf,	tCT_DeviceListEntityObject,		NULL,			0}
};

int objCT_DeviceList(char *name, struct CWMP_LEAF *e, int type, void *data)
{
	int ret = 0;
	unsigned int i, num, maxInstNum;
	struct CWMP_NODE *entity = (struct CWMP_NODE *)e;

	if (name == NULL || entity == NULL)
		return -1;

	switch (type) {
	case eCWMP_tINITOBJ:
		gCT_device_list = entity;
		return 0;
	case eCWMP_tADDOBJ:
		return ERR_9001;
	case eCWMP_tDELOBJ:
		return ERR_9001;
	case eCWMP_tUPDATEOBJ:
		return 0;
	}

	return ret;
}

int getCT_DeviceListEntity(char *name, struct CWMP_LEAF *entity, int *type, void **data)
{
	char *lastname = entity->info->name;
	unsigned int num;
	struct CWMP_LINKNODE *dev_node = NULL;
	struct dev_list_obj_data *dev_data = NULL;

	if ((name == NULL) || (entity == NULL) || (type == NULL) || (data == NULL))
		return -1;

	num = getInstNum(name, "DeviceList");

	//Attach object data
	dev_node = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
	if(dev_node == NULL)
		return ERR_9005;

	dev_data = (struct dev_list_obj_data *)dev_node->obj_data;
	if(dev_data == NULL)
	{
		CWMPDBP1("obj_data is NULL\n");
		return ERR_9002;
	}

	*type = entity->info->type;
	*data = NULL;

	if (strcmp(lastname, "IsOnLine") == 0)
	{
		*data = booldup(dev_data->IsOnLine);
	}
	else if (strcmp(lastname, "AttachedPort") == 0)
	{
		static const char *ports[] = {
			"", "LAN_Port1", "LAN_Port2", "LAN_Port3", "LAN_Port4",
			"Wireless_2.4G", "Wireless_5.8G"
		};
		*data = strdup(ports[dev_data->AttachedPort]);
	}
	else {
		return ERR_9005;
	}

	return 0;
}


/***** event handler *****/

static void upnpdm_dev_online(struct upnpdm_dev_info *dev)
{
	struct dev_list_obj_data *data = NULL;
	int ret;
	unsigned int num = 0;

	data = get_dev_obj_data(dev);
	if(data == NULL)
	{
		ret = add_Object(DEV_LIST_PATH,
			       (struct CWMP_LINKNODE **)&gCT_device_list->next,
			       tCT_DeviceListObject,
			       sizeof(tCT_DeviceListObject), &num);

		device_cnt++;

		CWMPDBP1("New device <%s> is added as num %u\n", dev->uuid, num);

		if (ret >= 0)
		{
			struct CWMP_LINKNODE *tmp_entity = NULL;

			//Attach object data
			tmp_entity = find_SiblingEntity( (struct CWMP_LINKNODE **)&gCT_device_list->next, num);
			if(tmp_entity)
			{
				data = malloc(sizeof(struct dev_list_obj_data));

				memset(data, 0, sizeof(struct dev_list_obj_data));

				data->UUID = strdup(dev->uuid);
				data->ManufacturerOUI = strdup(dev->ManufacturerOUI);
				data->SerialNumber = strdup(dev->SerialNumber);
				data->ProductClass = strdup(dev->ProductClass);
				data->ProvisioningCode = strdup(dev->ProvisioningCode);
				data->DeviceType = strdup(dev->DeviceType);
				data->SoftwareVersion = strdup(dev->SoftwareVersion);
				data->HardwareVersion = strdup(dev->HardwareVersion);
				
				tmp_entity->obj_data = (void *)data;
				tmp_entity->free_obj_data = free_CT_dev_list_obj;

				if(strcmp(data->DeviceType, "CT-COM_AP") == 0)
				{
					struct CWMP_NODE *srv_obj = &tmp_entity->next[eCT_DevList_ServiceObject];
					srv_obj->next = tCT_AP_ServiceObjectObject;
					srv_obj->leaf = tCT_AP_ServiceObjectLeaf;

					init_ParameterTable(&tmp_entity->next[eCT_DevList_ServiceObject].next, tCT_AP_ServiceObjectObject, "InternetGatewayDevice.X_CT-COM_ProxyDevice.DeviceList.1.ServiceObject.");
				}
				else
				{
					CWMPDBP1("Do not support device type: %s\n", data->DeviceType);
				}
			}
		}
	}
	if(data)
	{
		data->IsOnLine = 1;
		data->AttachedPort = dev->AttachedPort;
	}
}

static int upnpdm_dev_offline(char *uuid)
{
	struct dev_list_obj_data *data = NULL;
	unsigned int num;

	if(uuid == NULL)
		return -1;

	data = get_dev_obj_data_from_uuid(uuid);
	if(data == NULL)
		return -1;

	data->AttachedPort = UPNPDM_LAN_PORT_NONE;
	data->IsOnLine = 0;
	
	CWMPDBP1("Device <%s> is offline\n", uuid);
}

static int dev_listener_sock = -1;

int init_upnpdm_socket(int epollfd)
{
	struct epoll_event ev;
	struct sockaddr_un bind_addr = {0};

	ev.events = EPOLLIN;

	if ((dev_listener_sock = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
		perror("<upnpdm> socket");
		return -1;
	}

	bind_addr.sun_family = AF_UNIX;
	strcpy(bind_addr.sun_path, DEV_LISTENER_FILE);

	/* unlink before bind to avoid failure */
	unlink(DEV_LISTENER_FILE);
	if (bind(dev_listener_sock, (struct sockaddr *)&bind_addr, sizeof(bind_addr)) < 0)
	{
		perror("<upnpdm> bind");
		goto fail;
	}

	ev.data.fd = dev_listener_sock;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, dev_listener_sock, &ev) == -1) 
	{
		perror("<upnpdm> epoll_ctl");
		goto fail;
	}

	CWMPDBP1("Start to listening UPnPDM devices on socket %d...\n", dev_listener_sock);
	return 0;

fail:
	close(dev_listener_sock);
	return -1;
}

int handle_upnpdm_event(int fd)
{
	int cmd, ret;
	struct sockaddr_un from_addr = {0};
	int fromlen = sizeof(from_addr);
	struct upnpdm_ipc_msg *msg = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};
	int client_sock = -1;

	if(dev_listener_sock != fd)
		return -1;

	msg = (struct upnpdm_ipc_msg *)buf;

	ret = recv(dev_listener_sock, msg, MAX_UPNPDM_IPC_BUF, 0);

	switch(msg->cmd)
	{
	case UPNPDM_CMD_DEV_ONLINE:
		{
			struct upnpdm_dev_info *dev = NULL;

			if(msg->data_len != sizeof(struct upnpdm_dev_info))
			{
				CWMPDBP1("wrong data length %d\n", msg->data_len);
				return -1;
			}

			dev = (struct upnpdm_dev_info *)msg->data;

			upnpdm_dev_online(dev);
			break;
		}
	case UPNPDM_CMD_DEV_OFFLINE:
		{
			upnpdm_dev_offline(msg->data);
			break;
		}
	default:
		CWMPDBP1("Unknown command %d\n", msg->cmd);
		break;
	}

	return 0;
}


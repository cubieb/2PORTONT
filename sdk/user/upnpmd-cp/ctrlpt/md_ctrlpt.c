/*******************************************************************************
 *
 * Copyright (c) 2000-2003 Intel Corporation 
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 *
 * - Redistributions of source code must retain the above copyright notice, 
 * this list of conditions and the following disclaimer. 
 * - Redistributions in binary form must reproduce the above copyright notice, 
 * this list of conditions and the following disclaimer in the documentation 
 * and/or other materials provided with the distribution. 
 * - Neither name of Intel Corporation nor the names of its contributors 
 * may be used to endorse or promote products derived from this software 
 * without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

/*!
 * \addtogroup UpnpSamples
 *
 * @{
 *
 * \name Control Point Sample Module
 *
 * @{
 *
 * \file
 */

#include "md_ctrlpt.h"

#include "upnp.h"

#include <linux/if_bridge.h>
#include <sys/sysinfo.h>
#include <sys/ioctl.h>


/*!
 * Mutex for protecting the global device list in a multi-threaded,
 * asynchronous environment. All functions should lock this mutex before
 * reading or writing the device list. 
 */
ithread_mutex_t DeviceListMutex;

UpnpClient_Handle ctrlpt_handle = -1;

/*! Device type for tv device. */
const char DeviceType[] = "urn:schemas-upnp-org:device:ManageableDevice:1";


/*! Service names.*/
const char *ServiceName[] = { "BasicManagement", "ConfigurationManagement" };


/*!
   Global arrays for storing variable names and counts for 
   TvControl and TvPicture services 
 */
const char *MdVarName[MD_SERVICE_SERVCOUNT][MD_MAXVARS] = {
    {"Power", "Channel", "Volume", ""},
    {"Color", "Tint", "Contrast", "Brightness"}
};
char MdVarCount[MD_SERVICE_SERVCOUNT] =
    { MD_CONTROL_VARCOUNT, MD_PICTURE_VARCOUNT };

/*!
   Timeout to request during subscriptions 
 */
int default_timeout = 1801;

/*!
   The first node in the global device list, or NULL if empty 
 */
struct DeviceNode *GlobalDeviceList = NULL;

static char *payload_getvalues =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<cms:ContentPathList xmlns:cms=\"urn:schemas-upnp-org:dm:cms\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:schemas-upnp-org:dm:cms http://www.upnp.org/schemas/dm/cms.xsd\">"
	"<ContentPath>%s</ContentPath>"
	"</cms:ContentPathList>";

static char *payload_setvalues =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<cms:ParameterValueList xmlns:cms=\"urn:schemas-upnp-org:dm:cms\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:schemas-upnp-org:dm:cms http://www.upnp.org/schemas/dm/cms.xsd\">"
	"<Parameter>"
	"<ParameterPath>%s</ParameterPath>"
	"<Value>%s</Value>"
	"</Parameter>"
	"</cms:ParameterValueList>";


static char *support_datamodels =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<cms:SupportedDataModels xmlns:cms=\"urn:schemas-upnp-org:dm:cms\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:schemas-upnp-org:dm:cms http://www.upnp.org/schemas/dm/cms.xsd\">"
	"</cms:SupportedDataModels>";

const char *path_depth[] = {"StartingNode", "SearchDepth"};

static char *payload_getattr =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<cms:NodeAttributePathList xmlns:cms=\"urn:schemas-upnp-org:dm:cms\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:schemas-upnp-org:dm:cms http://www.upnp.org/schemas/dm/cms.xsd\">"
	"<NodeAttributePath>%s</NodeAttributePath>"
	"</cms:NodeAttributePathList>";

const char *get_instances[] ={"MultiInstanceName"};

static char *payload_getdevstatus =
	"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
	"<cms:NodeAttributePathList xmlns:cms=\"urn:schemas-upnp-org:dm:cms\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"urn:schemas-upnp-org:dm:cms http://www.upnp.org/schemas/dm/cms.xsd\">"
	"<NodeAttributePath>%s</NodeAttributePath>"
	"</cms:NodeAttributePathList>";

	
const char *download_parm[] = {"FileType", "URL", "SoftwareVersion", "HardwareVersion", "Channel", "TimeWindow", "Username", "Password"};	

const char *download_val[] = {"1 Firmware Upgrade Image", "ftp://192.168.9.67/110VDA2b1.bin", "", "", "-1",
	"<TimeWindowList><Window><WindowStart>10</WindowStart><WindowEnd>30</WindowEnd><WindowMode>1 At Any Time</WindowMode><MaxRetries>3</MaxRetries><UserMessage>Update now?</UserMessage></Window></TimeWindowList>",
	"", ""};

/********************************************************************************
 * MDCtrlPointDeleteNode
 *
 * Description: 
 *       Delete a device node from the global device list.  Note that this
 *       function is NOT thread safe, and should be called from another
 *       function that has already locked the global device list.
 *
 * Parameters:
 *   node -- The device node
 *
 ********************************************************************************/
int
MDCtrlPointDeleteNode( struct DeviceNode *node )
{
	int rc, service, var;

	if (NULL == node) {
		SampleUtil_Print
		    ("ERROR: MDCtrlPointDeleteNode: Node is empty\n");
		return MD_ERROR;
	}

	for (service = 0; service < MD_SERVICE_SERVCOUNT; service++) {
		/*
		   If we have a valid control SID, then unsubscribe 
		 */
		if (strcmp(node->device.Service[service].SID, "") != 0) {
			rc = UpnpUnSubscribe(ctrlpt_handle,
					     node->device.Service[service].
					     SID);
			if (UPNP_E_SUCCESS == rc) {
				SampleUtil_Print
				    ("Unsubscribed from MD %s EventURL with SID=%s\n",
				     ServiceName[service],
				     node->device.Service[service].SID);
			} else {
				SampleUtil_Print
				    ("Error unsubscribing to MD %s EventURL -- %d\n",
				     ServiceName[service], rc);
			}
		}

		for (var = 0; var < MdVarCount[service]; var++) {
			if (node->device.Service[service].VariableStrVal[var]) {
				free(node->device.
				     Service[service].VariableStrVal[var]);
			}
		}
	}

	/*Notify New Device Added */
	SampleUtil_StateUpdate(NULL, NULL, node->device.UDN, DEVICE_REMOVED);
	free(node);
	node = NULL;

	return MD_SUCCESS;
}

/********************************************************************************
 * MDCtrlPointRemoveDevice
 *
 * Description: 
 *       Remove a device from the global device list.
 *
 * Parameters:
 *   UDN -- The Unique Device Name for the device to remove
 *
 ********************************************************************************/
int MDCtrlPointRemoveDevice(const char *UDN)
{
	struct DeviceNode *curdevnode;
	struct DeviceNode *prevdevnode;

	ithread_mutex_lock(&DeviceListMutex);

	curdevnode = GlobalDeviceList;
	if (!curdevnode) {
		SampleUtil_Print(
			"WARNING: MDCtrlPointRemoveDevice: Device list empty\n");
	} else {
		if (0 == strcmp(curdevnode->device.UDN, UDN)) {
			GlobalDeviceList = curdevnode->next;
			MDCtrlPointDeleteNode(curdevnode);
		} else {
			prevdevnode = curdevnode;
			curdevnode = curdevnode->next;
			while (curdevnode) {
				if (strcmp(curdevnode->device.UDN, UDN) == 0) {
					prevdevnode->next = curdevnode->next;
					MDCtrlPointDeleteNode(curdevnode);
					break;
				}
				prevdevnode = curdevnode;
				curdevnode = curdevnode->next;
			}
		}
	}

	ithread_mutex_unlock(&DeviceListMutex);

	return MD_SUCCESS;
}

/********************************************************************************
 * MDCtrlPointRemoveAll
 *
 * Description: 
 *       Remove all devices from the global device list.
 *
 * Parameters:
 *   None
 *
 ********************************************************************************/
int MDCtrlPointRemoveAll(void)
{
	struct DeviceNode *curdevnode, *next;

	ithread_mutex_lock(&DeviceListMutex);

	curdevnode = GlobalDeviceList;
	GlobalDeviceList = NULL;

	while (curdevnode) {
		next = curdevnode->next;
		MDCtrlPointDeleteNode(curdevnode);
		curdevnode = next;
	}

	ithread_mutex_unlock(&DeviceListMutex);

	return MD_SUCCESS;
}

/********************************************************************************
 * MDCtrlPointRefresh
 *
 * Description: 
 *       Clear the current global device list and issue new search
 *	 requests to build it up again from scratch.
 *
 * Parameters:
 *   None
 *
 ********************************************************************************/
int MDCtrlPointRefresh(void)
{
	int rc;

	MDCtrlPointRemoveAll();
	/* Search for all devices of type tvdevice version 1,
	 * waiting for up to 5 seconds for the response */
	rc = UpnpSearchAsync(ctrlpt_handle, 5, DeviceType, NULL);
	if (UPNP_E_SUCCESS != rc) {
		SampleUtil_Print("Error sending search request%d\n", rc);

		return MD_ERROR;
	}

	return MD_SUCCESS;
}

/********************************************************************************
 * MDCtrlPointGetVar
 *
 * Description: 
 *       Send a GetVar request to the specified service of a device.
 *
 * Parameters:
 *   service -- The service
 *   devnum -- The number of the device (order in the list,
 *             starting with 1)
 *   varname -- The name of the variable to request.
 *
 ********************************************************************************/
int MDCtrlPointGetVar(int service, int devnum, const char *varname)
{
	struct DeviceNode *devnode;
	int rc;

	ithread_mutex_lock(&DeviceListMutex);

	rc = MDCtrlPointGetDevice(devnum, &devnode);

	if (MD_SUCCESS == rc) {
		rc = UpnpGetServiceVarStatusAsync(
			ctrlpt_handle,
			devnode->device.Service[service].ControlURL,
			varname,
			MDCtrlPointCallbackEventHandler,
			NULL);
		if (rc != UPNP_E_SUCCESS) {
			SampleUtil_Print(
				"Error in UpnpGetServiceVarStatusAsync -- %d\n",
				rc);
			rc = MD_ERROR;
		}
	}

	ithread_mutex_unlock(&DeviceListMutex);

	return rc;
}
/********************************************************************************
 * MDCtrlPointSendAction
 *
 * Description: 
 *       Send an Action request to the specified service of a device.
 *
 * Parameters:
 *   service -- The service
 *   devnum -- The number of the device (order in the list,
 *             starting with 1)
 *   actionname -- The name of the action.
 *   param_name -- An array of parameter names
 *   param_val -- The corresponding parameter values
 *   param_count -- The number of parameters
 *
 ********************************************************************************/
int MDCtrlPointSendAction(
	int service,
	int devnum,
	const char *actionname,
	const char **param_name,
	char **param_val,
	int param_count, void *Cookie)
{
	struct DeviceNode *devnode;
	IXML_Document *actionNode = NULL;
	int rc = MD_SUCCESS;
	int param;

	ithread_mutex_lock(&DeviceListMutex);

	rc = MDCtrlPointGetDevice(devnum, &devnode);
	if (MD_SUCCESS == rc) {
		if (0 == param_count) {
			actionNode =
			    UpnpMakeAction(actionname, MDServiceType[service],
					   0, NULL);
		} else {
			for (param = 0; param < param_count; param++) {
				if (UpnpAddToAction
				    (&actionNode, actionname,
				     MDServiceType[service], param_name[param],
				     param_val[param]) != UPNP_E_SUCCESS) {
					SampleUtil_Print
					    ("ERROR: MDCtrlPointSendAction: Trying to add action param\n");
					/*return -1; // TBD - BAD! leaves mutex locked */
				}
			}
		}

		rc = UpnpSendActionAsync(ctrlpt_handle,
					 devnode->device.
					 Service[service].ControlURL,
					 MDServiceType[service], NULL,
					 actionNode,
					 MDCtrlPointCallbackEventHandler, Cookie);

		if (rc != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in UpnpSendActionAsync -- %d\n",
					 rc);
			rc = MD_ERROR;
		}
	}

	ithread_mutex_unlock(&DeviceListMutex);

	if (actionNode)
		ixmlDocument_free(actionNode);

	return rc;
}

/********************************************************************************
 * MDCtrlPointSendActionNumericArg
 *
 * Description:Send an action with one argument to a device in the global device list.
 *
 * Parameters:
 *   devnum -- The number of the device (order in the list, starting with 1)
 *   service -- TV_SERVICE_CONTROL or TV_SERVICE_PICTURE
 *   actionName -- The device action, i.e., "SetChannel"
 *   paramName -- The name of the parameter that is being passed
 *   paramValue -- Actual value of the parameter being passed
 *
 ********************************************************************************/
int MDCtrlPointSendActionNumericArg(int devnum, int service,
	const char *actionName, const char *paramName, int paramValue)
{
	char param_val_a[50];
	char *param_val = param_val_a;

	sprintf(param_val_a, "%d", paramValue);
	return MDCtrlPointSendAction(
		service, devnum, actionName, &paramName,
		&param_val, 1, NULL);
}

int CMS_GetSupportedDM(int devnum, char *actionName, char *paramName, void *Cookie)
{
	int ret;
	
	ret = MDCtrlPointSendAction(MD_CONFIGURATION_MANAGEMENT, devnum, actionName, &paramName, &support_datamodels, 1, Cookie);

	return ret;
}

int CMS_GetIns(int devnum, char *actionName, char **paramName, char *param_path,char *param_val, void *Cookie)
{
	char *val[2];
	int ret;
	val[0] = strdup(param_path);
	val[1] = strdup(param_val);

	ret = MDCtrlPointSendAction(MD_CONFIGURATION_MANAGEMENT, devnum, actionName, paramName, val, 2, Cookie);

	free(val[0]);
	free(val[1]);
	return ret;
}

int CMS_GetValues(int devnum, char *actionName, char *paramName, char *param_path, void *Cookie)
{
	
	char *ptr;
	int ret;
	char _buffer[1024];

	memset(_buffer, "\0", sizeof(_buffer));
	snprintf(_buffer, sizeof(_buffer), payload_getvalues, param_path);
	ptr = strdup(_buffer);
	
	ret = MDCtrlPointSendAction(MD_CONFIGURATION_MANAGEMENT, devnum, actionName, &paramName, &ptr, 1, Cookie);
	free(ptr);
	return ret;
}

int CMS_SetValues(int devnum, char *actionName, char *paramName, char *param_path,char *param_val, void *Cookie)
{
	char *ptr;
	int ret;
	char _buffer[1024];

	memset(_buffer, "\0", sizeof(_buffer));
	snprintf(_buffer, sizeof(_buffer), payload_setvalues, param_path, param_val);
	ptr = strdup(_buffer);

	ret = MDCtrlPointSendAction(MD_CONFIGURATION_MANAGEMENT, devnum, actionName, &paramName, &ptr, 1, Cookie);
	free(ptr);
	return ret;
}

int CMS_GetAttr(int devnum, char *actionName, char *paramName, char *param_path, void *Cookie)
{
	
	char *ptr;
	int ret;
	char _buffer[1024];

	memset(_buffer, "\0", sizeof(_buffer));
	snprintf(_buffer, sizeof(_buffer), payload_getattr, param_path);
	ptr = strdup(_buffer);
	
	ret = MDCtrlPointSendAction(MD_CONFIGURATION_MANAGEMENT, devnum, actionName, &paramName, &ptr, 1, Cookie);
	free(ptr);
	return ret;
}

int CMS_CreateIns(int devnum, char *actionName, char *paramName, char *param_path, void *Cookie)
{
	int ret;
	
	ret = MDCtrlPointSendAction(MD_CONFIGURATION_MANAGEMENT, devnum, actionName, &paramName, &param_path, 1, Cookie);

	return ret;
}

int BMS_GetDeviceStatus(int devnum, char *actionName, void *Cookie)
{
	return MDCtrlPointSendAction(
		MD_BASIC_MANAGEMENT, devnum, actionName, NULL, NULL, 0, Cookie);
}

int BMS_Download(int devnum, char *actionName, char **paramName, char **paramVal, void *Cookie)
{
	return MDCtrlPointSendAction(MD_BASIC_MANAGEMENT, devnum, actionName, paramName, paramVal, 8, Cookie);
}


/********************************************************************************
 * MDCtrlPointGetDevice
 *
 * Description: 
 *       Given a list number, returns the pointer to the device
 *       node at that position in the global device list.  Note
 *       that this function is not thread safe.  It must be called 
 *       from a function that has locked the global device list.
 *
 * Parameters:
 *   devnum -- The number of the device (order in the list,
 *             starting with 1)
 *   devnode -- The output device node pointer
 *
 ********************************************************************************/
int MDCtrlPointGetDevice(int devnum, struct DeviceNode **devnode)
{
	int count = devnum;
	struct DeviceNode *tmpdevnode = NULL;

	if (count)
		tmpdevnode = GlobalDeviceList;
	while (--count && tmpdevnode) {
		tmpdevnode = tmpdevnode->next;
	}
	if (!tmpdevnode) {
		SampleUtil_Print("Error finding Device number -- %d\n",
				 devnum);
		return MD_ERROR;
	}
	*devnode = tmpdevnode;

	return MD_SUCCESS;
}

/********************************************************************************
 * MDCtrlPointPrintList
 *
 * Description: 
 *       Print the universal device names for each device in the global device list
 *
 * Parameters:
 *   None
 *
 ********************************************************************************/
int MDCtrlPointPrintList()
{
	struct DeviceNode *tmpdevnode;
	int i = 0;

	ithread_mutex_lock(&DeviceListMutex);

	SampleUtil_Print("MDCtrlPointPrintList:\n");
	tmpdevnode = GlobalDeviceList;
	while (tmpdevnode) {
		SampleUtil_Print(" %3d -- %s\n", ++i, tmpdevnode->device.UDN);
		tmpdevnode = tmpdevnode->next;
	}
	SampleUtil_Print("\n");
	ithread_mutex_unlock(&DeviceListMutex);

	return MD_SUCCESS;
}

/********************************************************************************
 * MDCtrlPointPrintDevice
 *
 * Description: 
 *       Print the identifiers and state table for a device from
 *       the global device list.
 *
 * Parameters:
 *   devnum -- The number of the device (order in the list,
 *             starting with 1)
 *
 ********************************************************************************/
int MDCtrlPointPrintDevice(int devnum)
{
	struct DeviceNode *tmpdevnode;
	int i = 0, service, var;
	char spacer[15];

	if (devnum <= 0) {
		SampleUtil_Print(
			"Error in MDCtrlPointPrintDevice: "
			"invalid devnum = %d\n",
			devnum);
		return MD_ERROR;
	}

	ithread_mutex_lock(&DeviceListMutex);

	SampleUtil_Print("MDCtrlPointPrintDevice:\n");
	tmpdevnode = GlobalDeviceList;
	while (tmpdevnode) {
		i++;
		if (i == devnum)
			break;
		tmpdevnode = tmpdevnode->next;
	}
	if (!tmpdevnode) {
		SampleUtil_Print(
			"Error in MDCtrlPointPrintDevice: "
			"invalid devnum = %d  --  actual device count = %d\n",
			devnum, i);
	} else {
		SampleUtil_Print(
			"  Device -- %d\n"
			"    |                  \n"
			"    +- UDN        = %s\n"
			"    +- DescDocURL     = %s\n"
			"    +- FriendlyName   = %s\n"
			"    +- PresURL        = %s\n"
			"    +- Adver. TimeOut = %d\n",
			devnum,
			tmpdevnode->device.UDN,
			tmpdevnode->device.DescDocURL,
			tmpdevnode->device.FriendlyName,
			tmpdevnode->device.PresURL,
			tmpdevnode->device.AdvrTimeOut);
		for (service = 0; service < MD_SERVICE_SERVCOUNT; service++) {
			if (service < MD_SERVICE_SERVCOUNT - 1)
				sprintf(spacer, "    |    ");
			else
				sprintf(spacer, "         ");
			SampleUtil_Print(
				"    |                  \n"
				"    +- MD %s Service\n"
				"%s+- ServiceId       = %s\n"
				"%s+- ServiceType     = %s\n"
				"%s+- EventURL        = %s\n"
				"%s+- ControlURL      = %s\n"
				"%s+- SID             = %s\n"
				"%s+- ServiceStateTable\n",
				ServiceName[service],
				spacer,
				tmpdevnode->device.Service[service].ServiceId,
				spacer,
				tmpdevnode->device.Service[service].ServiceType,
				spacer,
				tmpdevnode->device.Service[service].EventURL,
				spacer,
				tmpdevnode->device.Service[service].ControlURL,
				spacer,
				tmpdevnode->device.Service[service].SID,
				spacer);
			for (var = 0; var < MdVarCount[service]; var++) {
				SampleUtil_Print(
					"%s     +- %-10s = %s\n",
					spacer,
					MdVarName[service][var],
					tmpdevnode->device.Service[service].VariableStrVal[var]);
			}
		}
	}
	SampleUtil_Print("\n");
	ithread_mutex_unlock(&DeviceListMutex);

	return MD_SUCCESS;
}

/********************************************************************************
 * MDCtrlPointAddDevice
 *
 * Description: 
 *       If the device is not already included in the global device list,
 *       add it.  Otherwise, update its advertisement expiration timeout.
 *
 * Parameters:
 *   DescDoc -- The description document for the device
 *   location -- The location of the description document URL
 *   expires -- The expiration time for this advertisement
 *
 ********************************************************************************/
void MDCtrlPointAddDevice(
	IXML_Document *DescDoc,
	const char *location,
	int expires)
{
	char *deviceType = NULL;
	char *friendlyName = NULL;
	char presURL[200];
	char *baseURL = NULL;
	char *relURL = NULL;
	char *UDN = NULL;
	char *serviceId[MD_SERVICE_SERVCOUNT] = { NULL, NULL };
	char *eventURL[MD_SERVICE_SERVCOUNT] = { NULL, NULL };
	char *controlURL[MD_SERVICE_SERVCOUNT] = { NULL, NULL };
	Upnp_SID eventSID[MD_SERVICE_SERVCOUNT];
	int TimeOut[MD_SERVICE_SERVCOUNT] = {
		default_timeout,
		default_timeout
	};
	struct DeviceNode *deviceNode;
	struct DeviceNode *tmpdevnode;
	int ret = 1;
	int found = 0;
	int service;
	int var;

	ithread_mutex_lock(&DeviceListMutex);

	/* Read key elements from description document */
	UDN = SampleUtil_GetFirstDocumentItem(DescDoc, "UDN");
	deviceType = SampleUtil_GetFirstDocumentItem(DescDoc, "deviceType");
	friendlyName = SampleUtil_GetFirstDocumentItem(DescDoc, "friendlyName");
	baseURL = SampleUtil_GetFirstDocumentItem(DescDoc, "URLBase");
	relURL = SampleUtil_GetFirstDocumentItem(DescDoc, "presentationURL");

	ret = UpnpResolveURL((baseURL ? baseURL : location), relURL, presURL);

	if (UPNP_E_SUCCESS != ret)
		SampleUtil_Print("Error generating presURL from %s + %s\n",
				 baseURL, relURL);

	if (strcmp(deviceType, DeviceType) == 0) {
		SampleUtil_Print("Found device\n");

		/* Check if this device is already in the list */
		tmpdevnode = GlobalDeviceList;
		while (tmpdevnode) {
			if (strcmp(tmpdevnode->device.UDN, UDN) == 0) {
				found = 1;
				break;
			}
			tmpdevnode = tmpdevnode->next;
		}

		if (found) {
			/* The device is already there, so just update  */
			/* the advertisement timeout field */
			tmpdevnode->device.AdvrTimeOut = expires;
		} else {
			for (service = 0; service < MD_SERVICE_SERVCOUNT;
			     service++) {
				if (SampleUtil_FindAndParseService
				    (DescDoc, location, MDServiceType[service],
				     &serviceId[service], &eventURL[service],
				     &controlURL[service])) {
					SampleUtil_Print
					    ("Subscribing to EventURL %s...\n",
					     eventURL[service]);
					ret =
					    UpnpSubscribe(ctrlpt_handle,
							  eventURL[service],
							  &TimeOut[service],
							  eventSID[service]);
					if (ret == UPNP_E_SUCCESS) {
						SampleUtil_Print
						    ("Subscribed to EventURL with SID=%s\n",
						     eventSID[service]);
					} else {
						SampleUtil_Print
						    ("Error Subscribing to EventURL -- %d\n",
						     ret);
						strcpy(eventSID[service], "");
					}
				} else {
					SampleUtil_Print
					    ("Error: Could not find Service: %s\n",
					     MDServiceType[service]);
				}
			}
			/* Create a new device node */
			deviceNode =
			    (struct DeviceNode *)
			    malloc(sizeof(struct DeviceNode));
			strcpy(deviceNode->device.UDN, UDN);
			strcpy(deviceNode->device.DescDocURL, location);
			strcpy(deviceNode->device.FriendlyName, friendlyName);
			strcpy(deviceNode->device.PresURL, presURL);
			deviceNode->device.AdvrTimeOut = expires;
			for (service = 0; service < MD_SERVICE_SERVCOUNT;
			     service++) {
				if (serviceId[service] == NULL) {
					/* not found */
					continue;
				}
				strcpy(deviceNode->device.Service[service].
				       ServiceId, serviceId[service]);
				strcpy(deviceNode->device.Service[service].
				       ServiceType, MDServiceType[service]);
				strcpy(deviceNode->device.Service[service].
				       ControlURL, controlURL[service]);
				strcpy(deviceNode->device.Service[service].
				       EventURL, eventURL[service]);
				strcpy(deviceNode->device.Service[service].
				       SID, eventSID[service]);
				for (var = 0; var < MdVarCount[service]; var++) {
					deviceNode->device.
					    Service[service].VariableStrVal
					    [var] =
					    (char *)malloc(MD_MAX_VAL_LEN);
					strcpy(deviceNode->device.
					       Service[service].VariableStrVal
					       [var], "");
				}
			}
			deviceNode->next = NULL;
			/* Insert the new device node in the list */
			if ((tmpdevnode = GlobalDeviceList)) {
				while (tmpdevnode) {
					if (tmpdevnode->next) {
						tmpdevnode = tmpdevnode->next;
					} else {
						tmpdevnode->next = deviceNode;
						break;
					}
				}
			} else {
				GlobalDeviceList = deviceNode;
			}
			/*Notify New Device Added */
			SampleUtil_StateUpdate(NULL, NULL,
					       deviceNode->device.UDN,
					       DEVICE_ADDED);
		}
	}

	ithread_mutex_unlock(&DeviceListMutex);

	if (deviceType)
		free(deviceType);
	if (friendlyName)
		free(friendlyName);
	if (UDN)
		free(UDN);
	if (baseURL)
		free(baseURL);
	if (relURL)
		free(relURL);
	for (service = 0; service < MD_SERVICE_SERVCOUNT; service++) {
		if (serviceId[service])
			free(serviceId[service]);
		if (controlURL[service])
			free(controlURL[service]);
		if (eventURL[service])
			free(eventURL[service]);
	}
}

void MDStateUpdate(char *UDN, int Service, IXML_Document *ChangedVariables,
		   char **State)
{
	IXML_NodeList *properties;
	IXML_NodeList *variables;
	IXML_Element *property;
	IXML_Element *variable;
	long unsigned int length;
	long unsigned int length1;
	long unsigned int i;
	int j;
	char *tmpstate = NULL;

	SampleUtil_Print("MD State Update (service %d):\n", Service);
	/* Find all of the e:property tags in the document */
	properties = ixmlDocument_getElementsByTagName(ChangedVariables,
		"e:property");
	if (properties) {
		length = ixmlNodeList_length(properties);
		for (i = 0; i < length; i++) {
			/* Loop through each property change found */
			property = (IXML_Element *)ixmlNodeList_item(
				properties, i);
			/* For each variable name in the state table,
			 * check if this is a corresponding property change */
			for (j = 0; j < MdVarCount[Service]; j++) {
				variables = ixmlElement_getElementsByTagName(
					property, MdVarName[Service][j]);
				/* If a match is found, extract 
				 * the value, and update the state table */
				if (variables) {
					length1 = ixmlNodeList_length(variables);
					if (length1) {
						variable = (IXML_Element *)
							ixmlNodeList_item(variables, 0);
						tmpstate =
						    SampleUtil_GetElementValue(variable);
						if (tmpstate) {
							strcpy(State[j], tmpstate);
							SampleUtil_Print(
								" Variable Name: %s New Value:'%s'\n",
								MdVarName[Service][j], State[j]);
						}
						if (tmpstate)
							free(tmpstate);
						tmpstate = NULL;
					}
					ixmlNodeList_free(variables);
					variables = NULL;
				}
			}
		}
		ixmlNodeList_free(properties);
	}
	return;
	UDN = UDN;
}

/********************************************************************************
 * MDCtrlPointHandleEvent
 *
 * Description: 
 *       Handle a UPnP event that was received.  Process the event and update
 *       the appropriate service state table.
 *
 * Parameters:
 *   sid -- The subscription id for the event
 *   eventkey -- The eventkey number for the event
 *   changes -- The DOM document representing the changes
 *
 ********************************************************************************/
void MDCtrlPointHandleEvent(
	const char *sid,
	int evntkey,
	IXML_Document *changes)
{
	struct DeviceNode *tmpdevnode;
	int service;

	ithread_mutex_lock(&DeviceListMutex);

	tmpdevnode = GlobalDeviceList;
	while (tmpdevnode) {
		for (service = 0; service < MD_SERVICE_SERVCOUNT; ++service) {
			if (strcmp(tmpdevnode->device.Service[service].SID, sid) ==  0) {
				SampleUtil_Print("Received MD %s Event: %d for SID %s\n",
					ServiceName[service],
					evntkey,
					sid);
				MDStateUpdate(
					tmpdevnode->device.UDN,
					service,
					changes,
					(char **)&tmpdevnode->device.Service[service].VariableStrVal);
				break;
			}
		}
		tmpdevnode = tmpdevnode->next;
	}

	ithread_mutex_unlock(&DeviceListMutex);
}

/********************************************************************************
 * MDCtrlPointHandleSubscribeUpdate
 *
 * Description: 
 *       Handle a UPnP subscription update that was received.  Find the 
 *       service the update belongs to, and update its subscription
 *       timeout.
 *
 * Parameters:
 *   eventURL -- The event URL for the subscription
 *   sid -- The subscription id for the subscription
 *   timeout  -- The new timeout for the subscription
 *
 ********************************************************************************/
void MDCtrlPointHandleSubscribeUpdate(
	const char *eventURL,
	const Upnp_SID sid,
	int timeout)
{
	struct DeviceNode *tmpdevnode;
	int service;

	ithread_mutex_lock(&DeviceListMutex);

	tmpdevnode = GlobalDeviceList;
	while (tmpdevnode) {
		for (service = 0; service < MD_SERVICE_SERVCOUNT; service++) {
			if (strcmp
			    (tmpdevnode->device.Service[service].EventURL,
			     eventURL) == 0) {
				SampleUtil_Print
				    ("Received MD %s Event Renewal for eventURL %s\n",
				     ServiceName[service], eventURL);
				strcpy(tmpdevnode->device.Service[service].
				       SID, sid);
				break;
			}
		}

		tmpdevnode = tmpdevnode->next;
	}

	ithread_mutex_unlock(&DeviceListMutex);

	return;
	timeout = timeout;
}

void MDCtrlPointHandleGetVar(
	const char *controlURL,
	const char *varName,
	const DOMString varValue)
{

	struct DeviceNode *tmpdevnode;
	int service;

	ithread_mutex_lock(&DeviceListMutex);

	tmpdevnode = GlobalDeviceList;
	while (tmpdevnode) {
		for (service = 0; service < MD_SERVICE_SERVCOUNT; service++) {
			if (strcmp
			    (tmpdevnode->device.Service[service].ControlURL,
			     controlURL) == 0) {
				SampleUtil_StateUpdate(varName, varValue,
						       tmpdevnode->device.UDN,
						       GET_VAR_COMPLETE);
				break;
			}
		}
		tmpdevnode = tmpdevnode->next;
	}

	ithread_mutex_unlock(&DeviceListMutex);
}

unsigned char parsePort(char *location);

int checkDeviceType(struct Upnp_Discovery *d_event)
{
	if(!strcmp(d_event->DeviceType, DeviceType))
		return 1;
	else
		return 0;
}

int notifyCwmpSSDP(struct Upnp_Discovery *d_event, int status)
{
	struct upnpdm_dev_info *upnpdm = NULL;
	struct upnpdm_ipc_msg *msg = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};
	int ret = 0;

	msg = (struct upnpdm_ipc_msg *)buf;
	msg->cmd = status;

	switch(status){

		case UPNPDM_CMD_DEV_ONLINE:
				
			upnpdm = (struct upnpdm_dev_info *)msg->data;
			msg->data_len = sizeof(struct upnpdm_dev_info);

			//memset(upnpdm, 0, sizeof(upnpdm));

			upnpdm->AttachedPort = parsePort(d_event->Location);
			
			strncpy(upnpdm->DeviceType, d_event->DeviceTypeName, sizeof(upnpdm->DeviceType));
			strncpy(upnpdm->HardwareVersion, d_event->HardwareVersion, sizeof(upnpdm->HardwareVersion));
			strncpy(upnpdm->ManufacturerOUI, d_event->ManufacturerOUI, sizeof(upnpdm->ManufacturerOUI));
			strncpy(upnpdm->ProductClass, d_event->ProductClass, sizeof(upnpdm->ProductClass));
			strncpy(upnpdm->ProvisioningCode, d_event->ProvisioningCode , sizeof(upnpdm->ProvisioningCode));	
			strncpy(upnpdm->SerialNumber, d_event->SerialNumber , sizeof(upnpdm->SerialNumber));	
			strncpy(upnpdm->SoftwareVersion, d_event->SoftwareVersion , sizeof(upnpdm->SoftwareVersion));
			strncpy(upnpdm->uuid, d_event->DeviceId , sizeof(upnpdm->uuid));

			break;
			
		case UPNPDM_CMD_DEV_OFFLINE:
			if(d_event)
			{
				msg->data_len = strlen(d_event->DeviceId) + 1;
				strncpy(msg->data, d_event->DeviceId, sizeof(upnpdm->uuid));
			}
			else
				msg->data_len = 0;
			break;
		
	
	}
	ret = send_response((char *)msg, sizeof(struct upnpdm_ipc_msg) + msg->data_len);
	return ret;
}

int notifyCwmpTimeout(struct Device *dev_cp)
{
	struct upnpdm_dev_info *upnpdm = NULL;
	struct upnpdm_ipc_msg *msg = NULL;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};
	int ret = 0;

	msg = (struct upnpdm_ipc_msg *)buf;
	msg->cmd = UPNPDM_CMD_DEV_OFFLINE;

	msg->data_len = strlen(dev_cp->UDN) + 1;
	strncpy(msg->data, dev_cp->UDN, sizeof(upnpdm->uuid));
	
	ret = send_response((char *)msg, sizeof(struct upnpdm_ipc_msg) + msg->data_len);
	return ret;
}


void translate_xml(char * buffer, char *tmpBuf)
{
	char *p1 = buffer, *p2 = tmpBuf;


    while (*p1) {
		if (strncmp(p1, "&lt;", 4)==0){
                memcpy(p2, "<", 1);
				p1+=4;
        }
        else if (strncmp(p1, "&gt;", 4)==0){
                memcpy(p2, ">", 1);
				p1+=4;
        }
		else if(strncmp(p1, "&quot;", 6)==0){
				memcpy(p2, "\"", 1);
				p1+=6;
				
		}
        else
                *p2 = *p1++;

        p2++;
        
    }
    *p2 = '\0';

    //strcpy(buffer, tmpBuf);
}

int BMS_RebootResp(int error, int size, char *data, void *Cookie)
{
	return SendGeneralActionResult(error, size, data, Cookie, 1);
}

int CMS_GetAttributesResp(int error, int size, char *data, void *Cookie)
{  
	return SendGeneralActionResult(error, size, data, Cookie, 3); 
}

int CMS_GetInstancesResp(int error, int size, char *data, void *Cookie)
{  
	return SendGeneralActionResult(error, size, data, Cookie, 1); 
}

int CMS_GetNamesResp(int error, int size, char *data, void *Cookie)
{  
	return SendGeneralActionResult(error, size, data, Cookie, 2); 
}


int CMS_GetValuesResp(int error, int size, char *data, void *Cookie)
{  
	return SendGeneralActionResult(error, size, data, Cookie, 2); 
}

int CMS_SetValuesResp(int error, int size, char *data, void *Cookie)
{
	return SendGeneralActionResult(error, size, data, Cookie, 1);
}

int CMS_CreateInstanceResp(int error, int size, char *data, void *Cookie)
{
	return SendGeneralActionResult(error, size, data, Cookie, 1);
}


int CMS_DeleteInstanceResp(int error, int size, char *data, void *Cookie)
{
	return SendGeneralActionResult(error, size, data, Cookie, 1);
}


int CMS_GetSupportedDataModelsResp(int error, int size, char *data, void *Cookie)
{
	return SendGeneralActionResult(error, size, data, Cookie, 4);
}

int BMS_DownloadResp(int error, int size, char *data, void *Cookie)
{
	return SendGeneralActionResult(error, size, data, Cookie, 1);
}

int BMS_GetDownloadStatusResp(int error, int size, char *data, void *Cookie)
{
	return SendGeneralActionResult(error, size, data, Cookie, 1);
}

int SendGeneralActionResult(int error, int size, char *data, void *Cookie, int numargs)
{    
	char *snd_buf;
	int len; 
	int i,j;
	int ns = *((int *)Cookie);
  
	if(send(ns, &error, sizeof(error), 0)==-1)
		printf("send failed\n");
	
	if(send(ns, &size, sizeof(size), 0)==-1)
		printf("send failed\n");

	snd_buf = data;
	for(i=0;i<size;i++){
		for(j=0;j<numargs;j++){
			len = strlen(snd_buf);
			printf("sending %s\n", snd_buf);
			//printf("check %d\n", snd_buf[len]=='\0');
			if(send(ns, snd_buf, len+1, 0)==-1)
				printf("send failed\n");
			snd_buf +=(len+1);
		}
	}
	  
	close(ns);  
	return 0; 
}

#define strlens(s) (s==NULL?0:strlen(s))

int xmldoc_datalength(IXML_Node *inode, int param_list_num, int *count, IXML_Document *doc)
{
	int data_len=0;
	IXML_Node *tnode, *snode;
	
	if(param_list_num==0){
		tnode = ixmlNode_getFirstChild(inode);
		data_len+=(strlens(ixmlNode_getNodeValue(ixmlNode_getFirstChild(tnode)))+1);
		while((tnode = ixmlNode_getNextSibling(tnode))!=NULL){
			data_len+=(strlens(ixmlNode_getNodeValue(ixmlNode_getFirstChild(tnode)))+1);
		}
	}
	else{
		tnode = ixmlNode_getFirstChild((IXML_Node *)doc);	
		tnode = ixmlNode_getFirstChild(tnode);
		while(tnode!=NULL){
			(*count)+=1;
			snode = ixmlNode_getFirstChild(tnode);
			if(param_list_num>1){
				while(snode!=NULL){
					data_len += (strlens(ixmlNode_getNodeValue(ixmlNode_getFirstChild(snode)))+1);
					snode = ixmlNode_getNextSibling(snode);
				}
			}
			else
				data_len += (strlens(ixmlNode_getNodeValue(snode))+1);
			tnode = ixmlNode_getNextSibling(tnode);
		}
	}

	return data_len;
}

void xmldoc_parse(IXML_Node *inode, int param_list_num, char *out_buf, IXML_Document *doc)
{
	int length;
	char *result, *s_ptr;
	IXML_Node *tnode, *snode;
	
	if(param_list_num==0){
		result = out_buf;
		tnode = ixmlNode_getFirstChild(inode);
		s_ptr = ixmlNode_getNodeValue(ixmlNode_getFirstChild(tnode));
		length = strlens(s_ptr);
		memcpy(result, s_ptr, length);
		result+=(length+1);
		while((tnode = ixmlNode_getNextSibling(tnode))!=NULL){
			s_ptr = ixmlNode_getNodeValue(ixmlNode_getFirstChild(tnode));
			length = strlens(s_ptr);
			memcpy(result, s_ptr, length);
			result+=(length+1);
		}
	}
	else{
		result = out_buf;
		tnode = ixmlNode_getFirstChild((IXML_Node *)doc);	
		tnode = ixmlNode_getFirstChild(tnode);
		while(tnode!=NULL){
			snode = ixmlNode_getFirstChild(tnode);
			if(param_list_num>1){
				while(snode!=NULL){
					s_ptr = ixmlNode_getNodeValue(ixmlNode_getFirstChild(snode));
					length = strlens(s_ptr);
					memcpy(result, s_ptr, length);
					result+=(length+1);
					snode = ixmlNode_getNextSibling(snode);
				}
			}
			else{
				s_ptr = ixmlNode_getNodeValue(snode);
				length = strlens(s_ptr);
				memcpy(result, s_ptr, length);
				result+=(length+1);
			}
			tnode = ixmlNode_getNextSibling(tnode);
		}
	}
}

void process_action_response(struct Upnp_Action_Complete *a_event, void *Cookie)
{
	char buf[6400] = {0};
	IXML_Node *inode, *tnode;
	char actionRequest[64];
	int i, child_count=0, data_len=0;
	int cmdnum = -1;
	IXML_Document *doc=NULL;
	int ret;
	char *result;
	
	if(a_event->ErrCode==UPNP_E_SUCCESS){
		
		inode = ixmlNode_getFirstChild((IXML_Node *)a_event->ActionRequest);
		for(i=0;i<(sizeof(upnp_dm_action_command_list)/sizeof(struct upnp_dm_action_commands));i++){
			snprintf(actionRequest, sizeof(actionRequest), "u:%s", upnp_dm_action_command_list[i].str);
			if(strcasecmp(actionRequest, ixmlNode_getNodeName(inode))==0){
				cmdnum = upnp_dm_action_command_list[i].cmdnum;
				printf("%sResponse\n", upnp_dm_action_command_list[i].str);
				break;
			}
		}
		if(cmdnum!=-1)
			inode = ixmlNode_getFirstChild((IXML_Node *)a_event->ActionResult);


		if( cmdnum==UPNPDM_CMS_GetValues ||
			cmdnum==UPNPDM_CMS_GetInstances ||
			cmdnum==UPNPDM_CMS_GetNames ||
			cmdnum==UPNPDM_CMS_GetAttributes ||
			cmdnum==UPNPDM_CMS_GetSupportedParameters ||
			cmdnum==UPNPDM_CMS_GetSupportedDataModels
			)
		{
			memset(buf, '\0', sizeof(buf));
			tnode = inode;
			while(1){
				tnode = ixmlNode_getFirstChild(tnode);
				//printf("%s\n", ixmlNode_getNodeName(inode));
				if(strcmp("#text",ixmlNode_getNodeName(tnode))==0)
					break;
			}
			translate_xml(ixmlNode_getNodeValue(tnode), buf);
			ret = ixmlParseBufferEx(buf, &doc);
			//printf("ret %d\n", ret);
			//printf("translate %s \n", buf);
		}
		
		switch(cmdnum){
			case UPNPDM_BMS_GetDeviceStatus:
			case UPNPDM_BMS_Reboot:
				data_len = xmldoc_datalength(inode, 0, NULL, NULL);
				result = (char *)malloc(data_len);
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 0, result, NULL);
				BMS_RebootResp(a_event->ErrCode, 1, result, Cookie);
				free(result);
				break;
			case UPNPDM_CMS_GetValues:
				data_len = xmldoc_datalength(inode, 2, &child_count, doc);
				result = (char *)malloc(data_len);
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 2, result, doc);
				CMS_GetValuesResp(a_event->ErrCode, child_count, result, Cookie);
				free(result);
				break;
			case UPNPDM_CMS_SetValues:
				data_len = xmldoc_datalength(inode, 0, NULL, NULL);
				result = (char *)malloc(data_len);
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 0, result, NULL);
				CMS_SetValuesResp(a_event->ErrCode, 1, result, Cookie);
				free(result);
				break;
			case UPNPDM_CMS_GetAttributes:
				data_len = xmldoc_datalength(inode, 3, &child_count, doc);
				result = (char *)malloc(data_len);
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 3, result, doc);
				CMS_GetAttributesResp(a_event->ErrCode, child_count, result, Cookie);
				free(result);
				break;
			case UPNPDM_CMS_GetNames:				
				data_len = xmldoc_datalength(inode, 2, &child_count, doc);
				result = (char *)malloc(data_len);
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 2, result, doc);
				CMS_GetNamesResp(a_event->ErrCode, child_count, result, Cookie);
				free(result);
				break;
			case UPNPDM_CMS_GetSupportedParameters:
			case UPNPDM_CMS_GetInstances:
				data_len = xmldoc_datalength(inode, 1, &child_count, doc);
				result = (char *)malloc(data_len);	
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 1, result, doc);
				CMS_GetInstancesResp(a_event->ErrCode, child_count, result, Cookie);
				free(result);
				break;
			case UPNPDM_CMS_CreateInstance:
				data_len = xmldoc_datalength(inode, 0, NULL, NULL);
				result = (char *)malloc(data_len);
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 0, result, NULL);
				CMS_CreateInstanceResp(a_event->ErrCode, 2, result, Cookie);
				free(result);
				break;
			case UPNPDM_CMS_DeleteInstance:
				data_len = xmldoc_datalength(inode, 0, NULL, NULL);
				result = (char *)malloc(data_len);
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 0, result, NULL);
				CMS_DeleteInstanceResp(a_event->ErrCode, 1, result, Cookie);
				free(result);
				break;
			case UPNPDM_CMS_GetSupportedDataModels:
				data_len = xmldoc_datalength(inode, 4, &child_count, doc);
				result = (char *)malloc(data_len);
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 4, result, doc);
				CMS_GetSupportedDataModelsResp(a_event->ErrCode, child_count, result, Cookie);
				free(result);
				break;
			case UPNPDM_BMS_Download:
				data_len = xmldoc_datalength(inode, 0, NULL, NULL);
				result = (char *)malloc(data_len);
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 0, result, NULL);
				BMS_DownloadResp(a_event->ErrCode, 2, result, Cookie);
				free(result);
				break;
			case UPNPDM_BMS_GetDownloadStatus:
				data_len = xmldoc_datalength(inode, 0, NULL, NULL);
				result = (char *)malloc(data_len);
				memset(result, '\0', data_len);
				xmldoc_parse(inode, 0, result, NULL);
				BMS_GetDownloadStatusResp(a_event->ErrCode, 1, result, Cookie);
				free(result);
				break;
			case UPNPDM_BMS_BaselineReset:
			default:
				SendGeneralActionResult(a_event->ErrCode, 0, NULL, Cookie, 0);
				break;
				
		}
	}
	else{
		SendGeneralActionResult(a_event->ErrCode, 0, NULL, Cookie, 0);
	}

	ixmlDocument_free(doc);
	
}

int send_response(char *snd_buf, int size)  
{  
	int connect_fd;  
	int ret;  
	//char snd_buf[1024];    
	static struct sockaddr_un srv_addr;  
	//creat unix socket  
	connect_fd=socket(PF_UNIX,SOCK_DGRAM,0);  
	if(connect_fd<0)  
	{  
	    printf("cannot create communication socket");  
	    return 1;  
	}     
	srv_addr.sun_family=AF_UNIX;  
	strcpy(srv_addr.sun_path,DEV_LISTENER_FILE);  
	//connect server  
	ret=connect(connect_fd,(struct sockaddr*)&srv_addr,sizeof(srv_addr));  
	if(ret==-1)  
	{  
	    printf("cannot connect to the server");  
	    close(connect_fd);  
	    return 1;  
	}  
	//memset(snd_buf,0,1024);  
	//while(i<argc)
	//    snprintf(snd_buf, sizeof(snd_buf), "%s %s", snd_buf, argv[i++]);
	//strcpy(snd_buf,"message from client");  
	if(send(connect_fd, snd_buf, size, 0)==-1)
		printf("send failed");
	//send info server  
	close(connect_fd);  
	return 0;  
}  

void getMac(char *ip, char *mac)
{
	FILE *fp;
	char strbuf[256];
	char *ptr;
	
	fp = fopen("/proc/net/arp", "r");
	if (fp == NULL){
	    printf("read arp file fail!\n");
	    return;
	}
	fgets(strbuf, sizeof(strbuf), fp);
	while (fgets(strbuf, sizeof(strbuf), fp)) {
		ptr=strstr(strbuf, ip);
	    if(ptr!=NULL){
		  	sscanf(strbuf, "%*s %*s %*s %s %*s", mac);
			printf("mac %s\n", mac);
			break;
	  	}
	}

	fclose(fp);
}

static unsigned char check_fdb_entry(struct __fdb_entry *f, char *mac)
{
	char find_mac[20];

	snprintf(find_mac, sizeof(find_mac), "%02x:%02x:%02x:%02x:%02x:%02x"
		, f->mac_addr[0], f->mac_addr[1], f->mac_addr[2]
		, f->mac_addr[3], f->mac_addr[4], f->mac_addr[5]);

	if(!strcmp(find_mac, mac))
		return f->port_no;
	else
		return 0;		
}

unsigned char findPort(char *mac)
{
	int br_socket_fd;
	//struct bridge *br;
	struct __fdb_entry fdb[256];
	int offset;
	unsigned long args[4];
	struct ifreq ifr;
	unsigned char port;

	offset = 0;
	args[0] = BRCTL_GET_FDB_ENTRIES;
	args[1] = (unsigned long)fdb;
	args[2] = 256;
	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
	        printf("socket not avaiable !!\n");
	        return 0;
	}
	//      memcpy(ifr.ifr_name, br->ifname, IFNAMSIZ);
	memcpy(ifr.ifr_name, "br0", IFNAMSIZ);
	((unsigned long *)(&ifr.ifr_data))[0] = (unsigned long)args;
	//while (1) {
	    int i;
	    int num;

	    args[3] = offset;
	    num = ioctl(br_socket_fd, SIOCDEVPRIVATE, &ifr);

	    if (num <= 0)
	    {
            if (num < 0)
				printf("br0 interface not exists !!\n");
            //break;
	    }

	    for (i=0;i<num;i++){
            port = check_fdb_entry(fdb+i, mac);
			if(port){
				printf("port %d\n", port);
				break;
			}
	    }

	    //offset += num;
	//}
	close(br_socket_fd);

	return port;

}

unsigned char parsePort(char *location)
{
	char buffer[1024], mac[20];
	sscanf(location, "http://%s", buffer);
	char *ip = strtok(buffer, ":");
	
	getMac(ip, mac);
	return findPort(mac);

}

/********************************************************************************
 * MDCtrlPointCallbackEventHandler
 *
 * Description: 
 *       The callback handler registered with the SDK while registering
 *       the control point.  Detects the type of callback, and passes the 
 *       request on to the appropriate function.
 *
 * Parameters:
 *   EventType -- The type of callback event
 *   Event -- Data structure containing event data
 *   Cookie -- Optional data specified during callback registration
 *
 ********************************************************************************/
int MDCtrlPointCallbackEventHandler(Upnp_EventType EventType, void *Event, void *Cookie)
{
	/*int errCode = 0;*/

	SampleUtil_PrintEvent(EventType, Event);
	switch ( EventType ) {
	/* SSDP Stuff */
	case UPNP_DISCOVERY_ADVERTISEMENT_ALIVE:
		if(checkDeviceType((struct Upnp_Discovery *)Event))
			notifyCwmpSSDP((struct Upnp_Discovery *)Event, UPNPDM_CMD_DEV_ONLINE);
	case UPNP_DISCOVERY_SEARCH_RESULT: {
		struct Upnp_Discovery *d_event = (struct Upnp_Discovery *)Event;
		IXML_Document *DescDoc = NULL;
		int ret;

		if (d_event->ErrCode != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in Discovery Callback -- %d\n",
				d_event->ErrCode);
		}
		ret = UpnpDownloadXmlDoc(d_event->Location, &DescDoc);
		if (ret != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error obtaining device description from %s -- error = %d\n",
				d_event->Location, ret);
		} else {
			MDCtrlPointAddDevice(
				DescDoc, d_event->Location, d_event->Expires);
		}
		if (DescDoc) {
			ixmlDocument_free(DescDoc);
		}
		MDCtrlPointPrintList();
		break;
	}
	case UPNP_DISCOVERY_SEARCH_TIMEOUT:
		/* Nothing to do here... */
		break;
	case UPNP_DISCOVERY_ADVERTISEMENT_BYEBYE: {
		struct Upnp_Discovery *d_event = (struct Upnp_Discovery *)Event;

		if (d_event->ErrCode != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in Discovery ByeBye Callback -- %d\n",
					d_event->ErrCode);
		}
		SampleUtil_Print("Received ByeBye for Device: %s\n", d_event->DeviceId);
		MDCtrlPointRemoveDevice(d_event->DeviceId);
		notifyCwmpSSDP(d_event, UPNPDM_CMD_DEV_OFFLINE);
		SampleUtil_Print("After byebye:\n");
		MDCtrlPointPrintList();
		break;
	}
	/* SOAP Stuff */
	case UPNP_CONTROL_ACTION_COMPLETE: {
		struct Upnp_Action_Complete *a_event = (struct Upnp_Action_Complete *)Event;

		if (a_event->ErrCode != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in  Action Complete Callback -- %d\n",
					a_event->ErrCode);
		}
		/* No need for any processing here, just print out results.
		 * Service state table updates are handled by events. */
		process_action_response(a_event, Cookie);
		break;
	}
	case UPNP_CONTROL_GET_VAR_COMPLETE: {
		struct Upnp_State_Var_Complete *sv_event = (struct Upnp_State_Var_Complete *)Event;

		if (sv_event->ErrCode != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in Get Var Complete Callback -- %d\n",
					sv_event->ErrCode);
		} else {
			MDCtrlPointHandleGetVar(
				sv_event->CtrlUrl,
				sv_event->StateVarName,
				sv_event->CurrentVal);
		}
		break;
	}
	/* GENA Stuff */
	case UPNP_EVENT_RECEIVED: {
		struct Upnp_Event *e_event = (struct Upnp_Event *)Event;

		MDCtrlPointHandleEvent(
			e_event->Sid,
			e_event->EventKey,
			e_event->ChangedVariables);
		break;
	}
	case UPNP_EVENT_SUBSCRIBE_COMPLETE:
	case UPNP_EVENT_UNSUBSCRIBE_COMPLETE:
	case UPNP_EVENT_RENEWAL_COMPLETE: {
		struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe *)Event;

		if (es_event->ErrCode != UPNP_E_SUCCESS) {
			SampleUtil_Print("Error in Event Subscribe Callback -- %d\n",
					es_event->ErrCode);
		} else {
			MDCtrlPointHandleSubscribeUpdate(
				es_event->PublisherUrl,
				es_event->Sid,
				es_event->TimeOut);
		}
		break;
	}
	case UPNP_EVENT_AUTORENEWAL_FAILED:
	case UPNP_EVENT_SUBSCRIPTION_EXPIRED: {
		struct Upnp_Event_Subscribe *es_event = (struct Upnp_Event_Subscribe *)Event;
		int TimeOut = default_timeout;
		Upnp_SID newSID;
		int ret;

		ret = UpnpSubscribe(
			ctrlpt_handle,
			es_event->PublisherUrl,
			&TimeOut,
			newSID);
		if (ret == UPNP_E_SUCCESS) {
			SampleUtil_Print("Subscribed to EventURL with SID=%s\n", newSID);
			MDCtrlPointHandleSubscribeUpdate(
				es_event->PublisherUrl,
				newSID,
				TimeOut);
		} else {
			SampleUtil_Print("Error Subscribing to EventURL -- %d\n", ret);
		}
		break;
	}
	/* ignore these cases, since this is not a device */
	case UPNP_EVENT_SUBSCRIPTION_REQUEST:
	case UPNP_CONTROL_GET_VAR_REQUEST:
	case UPNP_CONTROL_ACTION_REQUEST:
		break;
	}

	return 0;
	Cookie = Cookie;
}

void MDCtrlPointVerifyTimeouts(int incr)
{
	struct DeviceNode *prevdevnode;
	struct DeviceNode *curdevnode;
	int ret;

	ithread_mutex_lock(&DeviceListMutex);

	prevdevnode = NULL;
	curdevnode = GlobalDeviceList;
	while (curdevnode) {
		curdevnode->device.AdvrTimeOut -= incr;
		/*SampleUtil_Print("Advertisement Timeout: %d\n", curdevnode->device.AdvrTimeOut); */
		if (curdevnode->device.AdvrTimeOut <= 0) {
			
			notifyCwmpTimeout(&curdevnode->device);
			
			/* This advertisement has expired, so we should remove the device
			 * from the list */
			if (GlobalDeviceList == curdevnode)
				GlobalDeviceList = curdevnode->next;
			else
				prevdevnode->next = curdevnode->next;
			MDCtrlPointDeleteNode(curdevnode);
			if (prevdevnode)
				curdevnode = prevdevnode->next;
			else
				curdevnode = GlobalDeviceList;
		} else {
			if (curdevnode->device.AdvrTimeOut < 2 * incr) {
				/* This advertisement is about to expire, so
				 * send out a search request for this device
				 * UDN to try to renew */
				ret = UpnpSearchAsync(ctrlpt_handle, incr,
						      curdevnode->device.UDN,
						      NULL);
				if (ret != UPNP_E_SUCCESS)
					SampleUtil_Print
					    ("Error sending search request for Device UDN: %s -- err = %d\n",
					     curdevnode->device.UDN, ret);
			}
			prevdevnode = curdevnode;
			curdevnode = curdevnode->next;
		}
	}

	ithread_mutex_unlock(&DeviceListMutex);
}

/*!
 * \brief Function that runs in its own thread and monitors advertisement
 * and subscription timeouts for devices in the global device list.
 */
static int MDCtrlPointTimerLoopRun = 1;
void *MDCtrlPointTimerLoop(void *args)
{
	/* how often to verify the timeouts, in seconds */
	int incr = 30;

	while (MDCtrlPointTimerLoopRun) {
		isleep((unsigned int)incr);
		MDCtrlPointVerifyTimeouts(incr);
	}

	return NULL;
	args = args;
}

/*!
 * \brief Call this function to initialize the UPnP library and start the MD
 * Control Point.  This function creates a timer thread and provides a
 * callback handler to process any UPnP events that are received.
 *
 * \return MD_SUCCESS if everything went well, else MD_ERROR.
 */
int MDCtrlPointStart(print_string printFunctionPtr, state_update updateFunctionPtr, int combo)
{
	ithread_t timer_thread;
	int rc;
	unsigned short port = 0;
	char *ip_address = NULL;

	SampleUtil_Initialize(printFunctionPtr);
	SampleUtil_RegisterUpdateFunction(updateFunctionPtr);

	ithread_mutex_init(&DeviceListMutex, 0);

	SampleUtil_Print("Initializing UPnP Sdk with\n"
			 "\tipaddress = %s port = %u\n",
			 ip_address ? ip_address : "{NULL}", port);

	rc = UpnpInit(ip_address, port);
	if (rc != UPNP_E_SUCCESS) {
		SampleUtil_Print("WinCEStart: UpnpInit() Error: %d\n", rc);
		if (!combo) {
			UpnpFinish();

			return MD_ERROR;
		}
	}
	if (!ip_address) {
		ip_address = UpnpGetServerIpAddress();
	}
	if (!port) {
		port = UpnpGetServerPort();
	}

	SampleUtil_Print("UPnP Initialized\n"
			 "\tipaddress = %s port = %u\n",
			 ip_address ? ip_address : "{NULL}", port);
	SampleUtil_Print("Registering Control Point\n");
	rc = UpnpRegisterClient(MDCtrlPointCallbackEventHandler,
				&ctrlpt_handle, &ctrlpt_handle);
	if (rc != UPNP_E_SUCCESS) {
		SampleUtil_Print("Error registering CP: %d\n", rc);
		UpnpFinish();

		return MD_ERROR;
	}

	SampleUtil_Print("Control Point Registered\n");

	MDCtrlPointRefresh();

	/* start a timer thread */
	ithread_create(&timer_thread, NULL, MDCtrlPointTimerLoop, NULL);
	ithread_detach(timer_thread);

	return MD_SUCCESS;
}

int MDCtrlPointStop(void)
{
	MDCtrlPointTimerLoopRun = 0;
	MDCtrlPointRemoveAll();
	UpnpUnRegisterClient( ctrlpt_handle );
	UpnpFinish();
	SampleUtil_Finish();

	return MD_SUCCESS;
}
void *MDCtrlPointCommandLoop(void *args)
{
	char cmdline[100];
	int temp=0;

	while (1) {
		SampleUtil_Print("\n>> ");
		fgets(cmdline, 100, stdin);
		MDCtrlPointProcessCommand(cmdline, &temp);
	}

	return NULL;
	args = args;
}

void *MDCtrlPoint_Socket(void *args)
{

    int fromlen;
    int s, ns;
	size_t len;
    struct sockaddr_un saun, fsaun;
	char buf[MAX_UPNPDM_IPC_BUF] = {0};
	char cmd_line[MAX_UPNPDM_IPC_BUF];
	struct upnpdm_action *action = NULL;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("server: socket");
        exit(1);
    }

    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path, DEV_ACTION_FILE);

    unlink(DEV_ACTION_FILE);
    len = sizeof(saun.sun_family) + strlen(saun.sun_path);

    if (bind(s, (struct sockaddr *)&saun, len) < 0) {
        perror("server: bind");
        exit(1);
    }

    if (listen(s, 5) < 0) {
        perror("server: listen");
        exit(1);
    }

	action = (struct upnpdm_action *)buf;

	while(1)
	{
	    if ((ns = accept(s, (struct sockaddr *)&fsaun, &fromlen)) < 0) {
	        perror("server: accept");
	        exit(1);
	    }

		len = recv(ns, buf, sizeof(buf), 0);
		printf("<server> read %d bytes, action=%d, uuid=%s, cmd=%s\n",
			len, action->action, action->uuid, action->cmd);

		snprintf(cmd_line, sizeof(cmd_line), "%s", action->cmd);
		//printf("%s\n", cmd_line);
		MDCtrlPointProcessCommand(cmd_line, &ns);

		//close(ns);
	}

	unlink(DEV_ACTION_FILE);
    close(s);

    return NULL;
	args = args;
}

int MDCtrlPointProcessCommand(char *cmdline, void *Cookie)
{
	char cmd[100];
	char strarg[100], strarg2[100], strarg3[100];
	int cmdnum = -1;
	int numofcmds = (sizeof upnp_dm_action_command_list) / sizeof (struct upnp_dm_action_commands);
	int cmdfound = 0;
	int i;
	int invalidargs = 0;
	int validargs;
	char *val[8];
	char *pch, *cmdline_temp;
	char url[100], username[100], password[100];

	validargs = sscanf(cmdline, "%s %s %s", cmd, strarg, strarg2);
	for (i = 0; i < numofcmds; i++) {
		if (strcasecmp(cmd, upnp_dm_action_command_list[i].str ) == 0) {
			cmdnum = upnp_dm_action_command_list[i].cmdnum;
			cmdfound++;
			if(cmdnum == UPNPDM_BMS_Download) // do not check num args for bms download
				break;
			if (validargs != upnp_dm_action_command_list[i].numargs)
				invalidargs++;
			break;
		}
	}
	if (!cmdfound) {
		SampleUtil_Print("Command not found\n");
		return MD_SUCCESS;
	}
	if (invalidargs) {
		SampleUtil_Print("Invalid arguments\n");
		return MD_SUCCESS;
	}
	
	switch (cmdnum) {

	case UPNPDM_CMS_GetSupportedDataModels:
		/* re-parse commandline since second arg is string. */
		validargs = sscanf(cmdline, "%s", strarg);
		if (validargs == 1)
			CMS_GetSupportedDM(1, strarg, "SupportedDataModels", Cookie); //SupportedDataModels
		else
			invalidargs++;
		break;
	case UPNPDM_CMS_GetSupportedParameters:
	case UPNPDM_CMS_GetInstances:
	case UPNPDM_CMS_GetNames:	
		validargs = sscanf(cmdline, "%s %s %s", strarg, strarg2, strarg3);
		if (validargs == 3)
			CMS_GetIns(1, strarg, path_depth, strarg2, strarg3, Cookie);
		else
			invalidargs++;
		break;
	case UPNPDM_CMS_GetValues:
		validargs = sscanf(cmdline, "%s %s", strarg, strarg2);
		if (validargs == 2)
			CMS_GetValues(1, strarg, "Parameters", strarg2, Cookie);
		else
			invalidargs++;
		break;
	case UPNPDM_CMS_SetValues:
		validargs = sscanf(cmdline, "%s %s %[^\n]", strarg, strarg2, strarg3);
		if (validargs == 3)
			CMS_SetValues(1, strarg, "ParameterValueList", strarg2, strarg3, Cookie);
		else
			invalidargs++;
		break;
	case UPNPDM_CMS_GetAttributes:
		validargs = sscanf(cmdline, "%s %s", strarg, strarg2);
		if (validargs == 2)
			CMS_GetAttr(1, strarg, "Parameters", strarg2, Cookie);
		else
			invalidargs++;
		break;
	case UPNPDM_CMS_DeleteInstance:
		validargs = sscanf(cmdline, "%s %s", strarg, strarg2);
		if (validargs == 2)
			CMS_CreateIns(1, strarg, "InstanceIdentifier", strarg2, Cookie); 
		else
			invalidargs++;
		break;
	case UPNPDM_CMS_CreateInstance:
		validargs = sscanf(cmdline, "%s %s", strarg, strarg2);
		if (validargs == 2)
			CMS_CreateIns(1, strarg, "MultiInstanceName", strarg2, Cookie); 
		else
			invalidargs++;
		break;
	case UPNPDM_BMS_Reboot:
	case UPNPDM_BMS_BaselineReset:
	case UPNPDM_BMS_GetDeviceStatus:
	case UPNPDM_BMS_GetDownloadStatus:
		validargs = sscanf(cmdline, "%s", strarg);
		if (validargs == 1)
			BMS_GetDeviceStatus(1, strarg, Cookie); 
		else
			invalidargs++;
		break;
	case UPNPDM_BMS_Download:
		cmdline_temp = strdup(cmdline);
		pch = strtok (cmdline_temp," ");
		validargs=0;
		while(pch != NULL){
			validargs++;
			pch = strtok (NULL," ");
		}
		if(validargs==4){
			sscanf(cmdline, "%s %s %s %s", strarg, url, username, password);
			val[0] = download_val[0];
			val[1] = url;
			val[2] = download_val[2];
			val[3] = download_val[3];
			val[4] = download_val[4];
			val[5] = download_val[5];
			val[6] = username;
			val[7] = password;
		}
		else{
			sscanf(cmdline, "%s %s", strarg, url);
			val[0] = download_val[0];
			val[1] = url;
			val[2] = download_val[2];
			val[3] = download_val[3];
			val[4] = download_val[4];
			val[5] = download_val[5];
			val[6] = "";
			val[7] = "";
		}
		if (validargs == 2 || validargs == 4)
			BMS_Download(1, strarg, download_parm, val, Cookie);
		else
			invalidargs++;	
		free(cmdline_temp);
		break;
	default:
		SampleUtil_Print("Command not implemented\n");
		break;
	}
	if(invalidargs)
		SampleUtil_Print("Invalid args in command\n");

	return MD_SUCCESS;
}

static char upnpmdcp_pidfile[] = "/var/run/upnpmd_cp.pid";
void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = upnpmdcp_pidfile;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}

/*! @} Control Point Sample Module */

/*! @} UpnpSamples */

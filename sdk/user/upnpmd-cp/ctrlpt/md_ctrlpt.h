#ifndef UPNP_MD_CTRLPT_H
#define UPNP_MD_CTRLPT_H

/**************************************************************************
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
 **************************************************************************/

/*!
 * \addtogroup UpnpSamples
 *
 * @{
 *
 * \name Contro Point Sample API
 *
 * @{
 *
 * \file
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "sample_util.h"

#include "upnp.h"
#include "UpnpString.h"
#include "upnptools.h"

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>

#include <sys/types.h>  
#include <sys/socket.h>  
#include <sys/un.h>
#include "../includes/upnpdm.h"


//#define UPNPMD_SOCK_FILE "/tmp/upnpmd-cp"
//#define MAXLENGTH       (64000)


#define MD_SERVICE_SERVCOUNT	2
#define MD_BASIC_MANAGEMENT		0
#define MD_CONFIGURATION_MANAGEMENT	1

#define MD_CONTROL_VARCOUNT	3
#define MD_CONTROL_POWER	0
#define MD_CONTROL_CHANNEL	1
#define MD_CONTROL_VOLUME	2

#define MD_PICTURE_VARCOUNT	4
#define MD_PICTURE_COLOR	0
#define MD_PICTURE_TINT		1
#define MD_PICTURE_CONTRAST	2
#define MD_PICTURE_BRIGHTNESS	3

#define MD_MAX_VAL_LEN		5

#define MD_SUCCESS		0
#define MD_ERROR		(-1)
#define MD_WARNING		1

/* This should be the maximum VARCOUNT from above */
#define MD_MAXVARS		MD_PICTURE_VARCOUNT

extern const char *ServiceName[];
extern const char *MdVarName[MD_SERVICE_SERVCOUNT][MD_MAXVARS];
extern char MdVarCount[];

struct md_service {
    char ServiceId[NAME_SIZE];
    char ServiceType[NAME_SIZE];
    char *VariableStrVal[MD_MAXVARS];
    char EventURL[NAME_SIZE];
    char ControlURL[NAME_SIZE];
    char SID[NAME_SIZE];
};

extern struct DeviceNode *GlobalDeviceList;

struct Device {
    char UDN[250];
    char DescDocURL[250];
    char FriendlyName[250];
    char PresURL[250];
    int  AdvrTimeOut;
    struct md_service Service[MD_SERVICE_SERVCOUNT];
};

struct DeviceNode {
    struct Device device;
    struct DeviceNode *next;
};

extern ithread_mutex_t DeviceListMutex;

extern UpnpClient_Handle ctrlpt_handle;

void	MDCtrlPointPrintHelp(void);
int		MDCtrlPointDeleteNode(struct DeviceNode *);
int		MDCtrlPointRemoveDevice(const char *);
int		MDCtrlPointRemoveAll(void);
int		MDCtrlPointRefresh(void);

int		MDCtrlPointSendAction(int, int, const char *, const char **, char **, int, void *);
int		MDCtrlPointSendActionNumericArg(int devnum, int service, const char *actionName, const char *paramName, int paramValue);
int		MDCtrlPointGetDevice(int, struct DeviceNode **);
int		MDCtrlPointPrintList(void);
int		MDCtrlPointPrintDevice(int);
void	MDCtrlPointAddDevice(IXML_Document *, const char *, int); 
void    MDCtrlPointHandleGetVar(const char *, const char *, const DOMString);

/*!
 * \brief Update a MD state table. Called when an event is received.
 *
 * Note: this function is NOT thread save. It must be called from another
 * function that has locked the global device list.
 **/
void MDStateUpdate(
	/*! [in] The UDN of the parent device. */
	char *UDN,
	/*! [in] The service state table to update. */
	int Service,
	/*! [out] DOM document representing the XML received with the event. */
	IXML_Document *ChangedVariables,
	/*! [out] pointer to the state table for the MD  service to update. */
	char **State);

void	MDCtrlPointHandleEvent(const char *, int, IXML_Document *); 
void	MDCtrlPointHandleSubscribeUpdate(const char *, const Upnp_SID, int); 
int		MDCtrlPointCallbackEventHandler(Upnp_EventType, void *, void *);

/*!
 * \brief Checks the advertisement each device in the global device list.
 *
 * If an advertisement expires, the device is removed from the list.
 *
 * If an advertisement is about to expire, a search request is sent for that
 * device.
 */
void MDCtrlPointVerifyTimeouts(
	/*! [in] The increment to subtract from the timeouts each time the
	 * function is called. */
	int incr);

void	MDCtrlPointPrintCommands(void);
void*	MDCtrlPointCommandLoop(void *);
void*	MDCtrlPoint_Socket(void *);

int		MDCtrlPointStart(print_string printFunctionPtr, state_update updateFunctionPtr, int combo);
int		MDCtrlPointStop(void);

/*!
 * \brief Print help info for this application.
 */
void MDCtrlPointPrintShortHelp(void);

/*!
 * \brief Print long help info for this application.
 */
void MDCtrlPointPrintLongHelp(void);

/*!
 * \briefPrint the list of valid command line commands to the user
 */
void MDCtrlPointPrintCommands(void);

/*!
 * \brief Function that receives commands from the user at the command prompt
 * during the lifetime of the device, and calls the appropriate
 * functions for those commands.
 */
void *MDCtrlPointCommandLoop(void *args);

/*!
 * \brief
 */
int MDCtrlPointProcessCommand(char *cmdline, void *Cookie);

#ifdef __cplusplus
};
#endif


/*! @} Device Sample */

/*! @} UpnpSamples */

#endif /* UPNP_MD_CTRLPT_H */

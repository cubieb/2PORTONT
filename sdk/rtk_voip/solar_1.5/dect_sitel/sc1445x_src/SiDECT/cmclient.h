/****************************************************************************
*  Program/file: CMCLIENT.H
*
*  Copyright (C) by RTX TELECOM A/S, Denmark.
*  These computer program listings and specifications, are the property of
*  RTX TELECOM A/S, Denmark and shall not be reproduced or copied or used in
*  whole or in part without written permission from RTX TELECOM A/S, Denmark.
*
*  Programmer: LHJ
*
*  MODULE: WIN32SIM, Communication manager
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*
*
*  DESCRIPTION: Handles the communication between programs, client part.
*
*
*
*
*
****************************************************************************/

/****************************************************************************
*                                  PVCS info
*****************************************************************************

$Author: krammer $
$Date: 2012/08/17 08:26:37 $
$Revision: 1.1 $
$Modtime:   21 Feb 2003 15:11:46  $

*/

#ifndef __CMCLIENT__
#define __CMCLIENT__

#ifdef __cplusplus
extern "C" {
#endif

extern void SendPacketToTask(int iTaskId, int iLength, unsigned char *bDataPtr);

extern int cm_ConnectPipe(int iWantedProgramId);
extern void cm_ClosePipe(int iProgramId);
extern void cm_OpenSharedMem(void);
extern void cm_SendPacketToCm(int iProgramId, int iTaskId, int iLength, unsigned char *bDataPtr);
extern void cm_SetProgramName(char *szName);
/****************************************************************************
*  FUNCTION: cm_SendPacketToCm2
*
*
*  DESCRIPTION:
*  Same as cm_SendPacketToCm, except that this version has a timeout (5s),
*  and will return a status value:    
*  
*  On success:
*  WAIT_OBJECT_0  The state of the specified object is signaled - normal.
*
*  On fail:
*  WAIT_FAILED    The function failed.
*  WAIT_ABANDONED The specified object is a mutex object that was not released 
*                 by the thread that owned the mutex object before the owning 
*                 thread terminated. Ownership of the mutex object is granted 
*                 to the calling thread, and the mutex is set to nonsignaled.
*  WAIT_TIMEOUT   The time-out interval elapsed, and the object's state is nonsignaled.
****************************************************************************/
unsigned long int cm_SendPacketToCm2(int iProgramId, int iTaskId, int iLength, unsigned char *bDataPtr);

#endif

#ifdef __cplusplus
}
#endif

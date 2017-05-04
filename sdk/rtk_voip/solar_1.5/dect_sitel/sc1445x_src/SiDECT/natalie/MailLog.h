/****************************************************************************
*  Program/file: MailLog.h 
*  
*  Copyright (C) by RTX TELECOM A/S, Denmark.
*  These computer program listings and specifications, are the property of 
*  RTX TELECOM A/S, Denmark and shall not be reproduced or copied or used in 
*  whole or in part without written permission from RTX TELECOM A/S, Denmark.
*
*  Programmer: LKA   
*
*  MODULE:
*  CONTROLLING DOCUMENT:
*  SYSTEM DEPENDENCIES:
*   
*   
*  DESCRIPTION:
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
$Modtime:   15 May 2008 14:59:54  $
$Archive:   J:/sw/Projects/450SIP/Fp/DectDemoApp/vcs/MailLog.h_v  $

*/

#ifndef MAILLOG_H  
#define MAILLOG_H  

/****************************************************************************
*                               Include files                                 
****************************************************************************/
//Type definitions

//Framework/Kernel

//Interfaces

//Configurations

//Private 

/****************************************************************************
*                              Macro definitions                             
****************************************************************************/
/* Debug prints */
#if 1
#define TIME_PRINT(args...)  //printf(args)
#define DEBUG_PRINT(args...)  //printf(args)
#define D_PRINT(args...)   //printf(args)
#define DUMP_TX_BYTES(args...)  //printf(args)
#define DUMP_RX_BYTES(args...)  //printf(args)
#define DUMP_TX_PRIM(args...)  //printf(args)
#define DUMP_RX_PRIM(args...)  //printf(args)
#define PRINT_MESSAGE(args...)  //printf(args)
#define PRINT_INFO(args...)  printf(args)
#endif
/****************************************************************************
*                     Enumerations/Type definitions/Structs                  
****************************************************************************/


/****************************************************************************
*                           Global variables/const                           
****************************************************************************/


/****************************************************************************
*                             Function prototypes                            
****************************************************************************/
char *MailLog(uint8 *MailPtr, uint16 MailLength);
  
#endif


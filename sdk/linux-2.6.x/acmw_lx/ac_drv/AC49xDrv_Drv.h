/** @file
 *  @brief This file is the only file that users must include in their application. The user's application needs
 *  to include this file (AC49xDrv_Drv.h) only. It integrates everything the user's application needs to use the
 *  drivers.
 */


/************************************************************************************/
/* AC49xDrv_Drv.h - AC49x Device Driver / Main driver include file					*/
/* Copyright (C) 2000 AudioCodes Ltd.												*/
/* 1/7/01 - Coding started.															*/
/*																					*/
/* ****************																	*/
/* IMPORTANT NOTE :																	*/
/* ****************																	*/
/* This file serves as the main include file.										*/
/* The user application needs to include this file only.							*/
/*																					*/
/* *************																	*/
/* Version Info:																	*/
/* *************																	*/
/* Data: "1234567890abcdef"															*/
/*																					*/
/*																					*/
/*																					*/
/*																					*/
/*																					*/
/************************************************************************************/

#ifndef AC49XDRV_H
#define AC49XDRV_H

/**************************************************/
/**************************************************/
/**           AC49x Driver Include Files         **/
/**************************************************/
/**************************************************/

/* AC49xDrv_Drv.h calls all other include files. The user application needs to include only this file */

#include "AC49xDrv_Options.h"				/* Constant Configuration Options Definitions  */
#include "AC49xDrv_Config.h"		        /* User User Defined Configuration Selection Header File*/
#include "AC49xDrv_Definitions.h"			/* Strcutures, Enums and Constants Definitions */
#include "AC49xDrv_Api.h"
#include "AC49xDrv_UserDefinedFunctions.h"


/**************************************************/
/**************************************************/
/**     External Global Device Arrays			 **/
/**************************************************/
/**************************************************/

#ifndef NDOC
#ifndef __ac49xdrv
#define __ac49xdrv	__attribute__ ((section(".text.ac49xdrv")))
#endif
#ifndef __ac49xdrvdata
#define __ac49xdrvdata	__attribute__ ((section(".data.ac49xdrv"))) __attribute__ ((aligned (8)))
#endif
#else  //NDOC
#ifndef __ac49xdrv
#define __ac49xdrv
#endif
#ifndef __ac49xdrvdata
#define __ac49xdrvdata
#endif

#endif //NDOC

#if __cplusplus
extern "C" {
#endif

#if  (AC49X_DEVICE_TYPE != ACLX5280_DEVICE)
    extern Tac49xDeviceControlRegister	        ac49xDeviceControlRegister[                 AC49X_NUMBER_OF_DEVICES];
    extern Tac49xBufferDescriptorCurrentIndex   ac49xHostBufferDescriptorCurrentIndex[      AC49X_NUMBER_OF_DEVICES];
    extern Tac49xBufferDescriptorCurrentIndex   ac49xNetworkBufferDescriptorCurrentIndex[   AC49X_NUMBER_OF_DEVICES];
#endif

                /* Debug information */
    extern int			                        ac49xRxHostPacketSequenceNumber   [         AC49X_NUMBER_OF_DEVICES];
    extern int		 	                        ac49xTxHostPacketSequenceNumber   [         AC49X_NUMBER_OF_DEVICES];
    extern int			                        ac49xRxNetworkPacketSequenceNumber[         AC49X_NUMBER_OF_DEVICES];
    extern int		 	                        ac49xTxNetworkPacketSequenceNumber[         AC49X_NUMBER_OF_DEVICES];
        /* Variable is used by Read/Write macros (should normally be set to 0). */
        /* extern unsigned int	acBoardIDOffset; */

#if __cplusplus
}
#endif

/**************************************************/
/**************************************************/
/**  Miscellaneous Back-Compatibility Constants  **/
/**************************************************/
/**************************************************/

/* Definitions for backward compatibility with other s/w modules : */
/* ----------------------------------------------------------------*/
#define CHANNELS_IN_AC4804 AC49X_NUMBER_OF_CHANNELS

/* MAX_CID_NUM - is the number of channels on user's hardware, in other words
   MAX_CID_NUM=(number of AC49x devices)*(number of channels per device).
   If MAX_CID_NUM is already defined and AC49X_NUMBER_OF_DEVICES isn't, use the
   first constant to calculate the second */

#ifndef MAX_CID_NUM
    #define MAX_CID_NUM				(AC49X_NUMBER_OF_DEVICES*AC49X_NUMBER_OF_CHANNELS)
#else
    #ifndef AC49X_NUMBER_OF_DEVICES
        #define AC49X_NUMBER_OF_DEVICES	(MAX_CID_NUM/AC49X_NUMBER_OF_CHANNELS)
    #endif
#endif
        /* Define the required Endian mode according to the selected mode */
#ifndef LITTLE_ENDIAN_MODE
#define LITTLE_ENDIAN_MODE	(1-AC49X_ENDIAN_MODE)
#endif

/*#ifndef BIG_ENDIAN_MODE*/
/*#define BIG_ENDIAN_MODE		AC49X_ENDIAN_MODE*/
/*#endif*/

#endif /* ifndef AC49XDRV_H */

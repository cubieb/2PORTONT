#ifndef __HALLIB_H__
#define __HALLIB_H__
/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	HalLib.h
	
Abstract:
	API Interface that export Function Prototype & Variable & Strcture & Enum & Marco 
	for Driver
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-03-23 Filen            Create.	
--*/

#ifndef WLAN_HAL_INTERNAL_USED
//Code below only are used by "Not WlanHAL"
//1 HAL Library API

//4 Common
#include "PlatformDef.h"
#include "StatusCode.h"
#include "GeneralDef.h"

//4 HAL Common
#include "HalCfg.h"
#include "HalCommon.h"


//4 Chip Dependent
#if IS_RTL88XX_GENERATION
#include "Hal88XXDef.h"
#include "Hal88XXDesc.h"
#include "Hal88XXTxDesc.h"
#include "Hal88XXRxDesc.h"
#include "Hal88XXFirmware.h"
#include "Hal88XXDebug.h"
#include "Hal88XXPhyCfg.h"

#if IS_EXIST_RTL8192EE
#endif

#if IS_EXIST_RTL8881AEM
#endif

#endif  //IS_RTL88XX_GENERATION



//Final Result
#include "HalDef.h"




#endif  //WLAN_HAL_INTERNAL_USED












#endif  //#ifndef __HAL8881A_LIB_H__



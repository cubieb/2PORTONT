#ifndef __INC_PRECOMPINC_H
#define __INC_PRECOMPINC_H


//HAL Shared with Driver
#include "StatusCode.h"
#include "HalDbgCmd.h"


//Prototype
#include "HalDef.h"

//MAC Header provided by SD1 HWSD
#include "HalHWCfg.h"
#include "HalComTXDesc.h"
#include "HalComRXDesc.h"
#include "HalComBit.h"
#include "HalComReg.h"
#include "HalComPhyBit.h"
#include "HalComPhyReg.h"

//Instance
#include "HalCommon.h"

#if IS_RTL88XX_GENERATION

#include "Hal88XXPwrSeqCmd.h"
#include "Hal88XXReg.h"
#include "Hal88XXDesc.h"
#include "Hal88XXTxDesc.h"
#include "Hal88XXRxDesc.h"
#include "Hal88XXFirmware.h"
#include "Hal88XXIsr.h"
#include "Hal88XXDebug.h"
#include "Hal88XXPhyCfg.h"


#if IS_RTL8881A_SERIES
#include "Hal8881APwrSeqCmd.h"
#include "Hal8881ADef.h"
#include "Hal8881APhyCfg.h"
#endif

#if IS_RTL8192E_SERIES
#include "Hal8192EPwrSeqCmd.h"
#include "Hal8192EDef.h"
#include "Hal8192EPhyCfg.h"
#endif

#if IS_EXIST_RTL8192EE
#include "Hal8192EEDef.h"
#endif


#include "Hal88XXDef.h"




#endif  //IS_RTL88XX_GENERATION



#endif  //#ifndef __INC_PRECOMPINC_H

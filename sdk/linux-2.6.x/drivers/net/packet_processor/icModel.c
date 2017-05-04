/*
* Copyright c                  Realtek Semiconductor Corporation, 2002  
* All rights reserved.
* 
* Program : Source File for RTL8672 IC-specific Function
* Abstract : 
* Author : Michael Mao-Lin Huang (michaelhuang@realtek.com.tw)
* $Id: icModel.c,v 1.1.1.1 2010/05/05 09:00:49 jiunming Exp $
*/


#include "rtl_types.h"
#include "rtl8672_asicregs.h"
#include "assert.h"
#include "rtl_glue.h"
#include "virtualMac.h"
#include "icModel.h"
#include "hsModel.h"
#include "modelTrace.h"
#include "rtl_utils.h"
#include "icExport.h"
#include "icModel_pktParser.h"
#include "icModel_ringController.h"
#include "icModel_pktTranslator.h"
#include "icModel_ALE.h"
#include "icTest.h"
#include "hsModel.h"
#if 0
/*
 *  Initialization sub-routine
 *  This function must be called at the first of every test case.
 *
 */
void modelIcInit( void )
{

	model_ring_controller_init();
	model_ale_init();
	rtlglue_printf("icModel: modelIcInit();\n");
	return;
}


/*
 *  Exit sub-toutine
 *  This function must be called at the end of every test case.
 *
 */
void modelIcExit( void )
{
	rtlglue_printf("icModel: modelIcExit();\n");
	return;
}


/*
 *  In the same test case, this function is used to re-chain allocated resource to free resource.
 *
 */
int32 modelIcReinit( void )
{
	model_ring_controller_init();
	rtlglue_printf("icModel: modelIcReinit();\n");
	return SUCCESS;
}


/*
func int32		| modelIcSetDefaultValue	| Set default value of IC registers after booted up.
parm void | |
rvalue void |
comm 
Set the default value of all registers in model code. Please key-in these default according the 865xC specification.
This function must be called after pVirtualSWReg/pVirtualSysReg/pVirtualSWTable had been allocated and 
  before ROME driver sets its default value.
*/
void modelIcSetDefaultValue( void )
{
	rtlglue_printf("icModel: modelIcSetDefaultValue();\n");
	return;
}


/*
func void		| startModel	| Starting model ( HSB --> HSA )
parm void | |
rvalue void |
comm 
This function has no parameter and return value.
Because this function will input parameter from global HSB, and generate return value into global HSA.
The decision flow is described according to Alpha's note.
*/
void modelStart( void )
{
	rtlglue_printf("icModel: modelIcStart();\n");
	return;
}
#endif

uint32 READ_MEM32(uint32 reg)
{
	uint32 ret;
	ret= big_endian(REG32(reg));
	return ret;
}




void WRITE_MEM32(uint32 reg, uint32 val)
{
	REG32(reg)=big_endian(val);
}


/************************** [ virtualMac Function ] **************************/


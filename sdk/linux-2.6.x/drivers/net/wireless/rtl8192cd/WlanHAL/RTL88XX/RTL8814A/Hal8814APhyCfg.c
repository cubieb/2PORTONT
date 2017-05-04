/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8814APhyCfg.c
	
Abstract:
	Defined HAL 8814A PHY BB setting functions
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2013-05-28 Filen              Create.	
--*/
#include "HalPrecomp.h"

// TODO: this function should be modified
void 
TXPowerTracking_ThermalMeter_Tmp8814A(
    IN  HAL_PADAPTER    Adapter
)
{
     // Do Nothing now
}

u4Byte
PHY_QueryRFReg_8814(
	IN  HAL_PADAPTER                Adapter,
	IN  u4Byte                      eRFPath,
	IN  u4Byte                      RegAddr,
	IN  u4Byte                      BitMask
)
{
	// The phy Query function have bug in 8812 & 8881, must follow the rules as below
	// 1. No matter path A or B, we have to assign the addr on 0x8b0[7:0]
	// 2. Before Query RF reg, we must turn off CCA
	// 3. delay 10us after set addr
	// 4. If SI mode, read value from 0xd08[19:0] on PathA, 0xd48[19:0] on PathB
	// 5. If PI mode, read value from 0xd08[19:0] on PathA, 0xd48[19:0] on PathB

	HAL_PADAPTER    priv     = Adapter;
	unsigned long   flags;
	u4Byte          Original_Value, Readback_Value, BitShift;
    u4Byte          temp_CCA;

#if CFG_HAL_DISABLE_BB_RF
	return 0;
#endif //CFG_HAL_DISABLE_BB_RF

	HAL_SAVE_INT_AND_CLI(flags);

	u4Byte	DataAndAddr = 0;
	u4Byte	Direct_Addr;	
	
	RegAddr &= 0xff;
	switch(eRFPath){
		case RF88XX_PATH_A:
			Direct_Addr = 0x2800+RegAddr*4;
		break;
		case RF88XX_PATH_B:
			Direct_Addr = 0x2c00+RegAddr*4;
		break;
		case RF88XX_PATH_C:
			Direct_Addr = 0x3800+RegAddr*4;
		break;
		case RF88XX_PATH_D:
			Direct_Addr = 0x3c00+RegAddr*4;
		break;
	}
	

	BitMask &= bRFRegOffsetMask;
	
	Readback_Value = PHY_QueryBBReg(Adapter, Direct_Addr, BitMask);		

	HAL_RESTORE_INT(flags);

	return (Readback_Value);
}




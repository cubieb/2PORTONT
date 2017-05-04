#ifndef __HAL8881APHYCFG_H__
#define __HAL8881APHYCFG_H__

/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8881APhyCfg.h
	
Abstract:
	Defined HAL 88XX PHY BB setting functions
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-11-14 Eric              Create.	
--*/


void 
PHYSetCCKTxPower8881A(
        IN  HAL_PADAPTER    Adapter, 
        IN  u1Byte          channel
);

void 
PHYSetOFDMTxPower8881A(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          channel
);


void 
CalOFDMTxPower_5G(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          ch_idx
);

void 
Cal_OFDMTxPower_2G(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          ch_idx
);

s1Byte
convert_diff(
    IN s1Byte value
);

void 
Write_1S_A(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
);

void 
Write_2S_A(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
);

void 
Write_OFDM_A(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
);

void 
Write_OFDM_B(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
);

void 
Write_1S_B(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
);

void 
Write_2S_B(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
);

void 
use_DefaultOFDMTxPower88XX(
    IN  HAL_PADAPTER    Adapter
);



#endif // #ifndef __HAL8881APHYCFG_H__


/*++
Copyright (c) Realtek Semiconductor Corp. All rights reserved.

Module Name:
	Hal8881APhyCfg.c
	
Abstract:
	Defined HAL 8881A PHY BB setting functions
	    
Major Change History:
	When       Who               What
	---------- ---------------   -------------------------------
	2012-11-14 Eric              Create.	
--*/

#include "HalPrecomp.h"

void 
PHYSetCCKTxPower8881A(
        IN  HAL_PADAPTER    Adapter, 
        IN  u1Byte          channel
)
{
	unsigned int def_power = 0x20202020;//0x12121212;
	unsigned char ch_idx = 0;
	unsigned char tmp_TPI = 0;
	unsigned char phy_band = 0;
	unsigned int  writeVal = 0;

	if (channel > 0)
		ch_idx = (channel - 1);
	else {
		printk("Error Channel !!\n");
		return;
	}

	if (channel > 14)
		phy_band = PHY_BAND_5G;
	else
		phy_band = PHY_BAND_2G;

	if ((HAL_VAR_pwrlevelCCK_A(ch_idx) == 0)
			|| (HAL_VAR_pwrlevelCCK_B(ch_idx) == 0)) {
		printk("NO Calibration data, use default CCK power = 0x%x\n", def_power);
		HAL_RTL_W32(rTxAGC_A_CCK11_CCK1_JAguar, def_power);
		HAL_RTL_W32(rTxAGC_B_CCK11_CCK1_JAguar, def_power);
        return;
	}

	if (phy_band == PHY_BAND_2G) {
		printk("pwrlevelCCK_A[%d]= %d \n", ch_idx, HAL_VAR_pwrlevelCCK_A(ch_idx));
		printk("pwrlevelCCK_B[%d]= %d \n", ch_idx, HAL_VAR_pwrlevelCCK_B(ch_idx));

		//PATH A
		tmp_TPI = HAL_VAR_pwrlevelCCK_A(ch_idx);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		HAL_RTL_W32(rTxAGC_A_CCK11_CCK1_JAguar, writeVal);

		//PATH B
		tmp_TPI = HAL_VAR_pwrlevelCCK_B(ch_idx);
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		HAL_RTL_W32(rTxAGC_B_CCK11_CCK1_JAguar, writeVal);
	}
}


void 
PHYSetOFDMTxPower8881A(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          channel
)
{
	unsigned char ch_idx = 0;
	unsigned char phy_band = 0;

	if (channel > 0)
		ch_idx = (channel - 1);
	else {
        RT_TRACE_F(COMP_RF, DBG_WARNING, ("Error Channel !!\n"));
		return;
	}

	if (channel > 14)
		phy_band = PHY_BAND_5G;
	else
		phy_band = PHY_BAND_2G;


	if (phy_band == PHY_BAND_5G)
		CalOFDMTxPower_5G(Adapter, ch_idx);
	else if (phy_band == PHY_BAND_2G)
		Cal_OFDMTxPower_2G(Adapter, ch_idx);

}


void 
CalOFDMTxPower_5G(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          ch_idx
)
{
	unsigned char tmp_TPI = 0;
	unsigned char pwr_40_1s = 0;
	signed char diff_ofdm_1t = 0;
	signed char diff_bw40_2s = 0;
	signed char diff_bw20_1s = 0;
	signed char diff_bw20_2s = 0;
	signed char diff_bw80_1s = 0;
	signed char diff_bw80_2s = 0;

	unsigned int  writeVal = 0;

	if ((HAL_VAR_pwrlevelHT40_1S_A(ch_idx)== 0)
			|| (HAL_VAR_pwrlevelHT40_1S_B(ch_idx) == 0)) {
			//printk("%s %d ====> use_DefaultOFDMTxPower_8812 Adapter=%s\n",__FUNCTION__,__LINE__,Adapter->dev->name);
		use_DefaultOFDMTxPower88XX(Adapter);
		return;
	}

	// PATH A, OFDM
	pwr_40_1s =  HAL_VAR_pwrlevelHT40_1S_A(ch_idx);
	diff_ofdm_1t = (HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_A(ch_idx) & 0x0f);
	diff_ofdm_1t = convert_diff(diff_ofdm_1t);
	tmp_TPI = pwr_40_1s + diff_ofdm_1t;
	writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
	Write_OFDM_A(Adapter, writeVal);

	//printk("Write_OFDM_A %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_ofdm_1t);


	// PATH B, OFDM
	pwr_40_1s = HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx);
	diff_ofdm_1t = (HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_B(ch_idx) & 0x0f);
	diff_ofdm_1t = convert_diff(diff_ofdm_1t);
	tmp_TPI = pwr_40_1s + diff_ofdm_1t;
	writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
	Write_OFDM_B(Adapter, writeVal);

	//printk("Write_OFDM_A %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_ofdm_1t);


	if (HAL_VAR_dot11nUse40M == 0) {
		//PATH A, BW20-1S
		pwr_40_1s = HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx);
		diff_bw20_1s = ((HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_A(ch_idx) & 0xf0) >> 4);
		diff_bw20_1s = convert_diff(diff_bw20_1s);
		tmp_TPI = pwr_40_1s + diff_bw20_1s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_A(Adapter, writeVal);

		//printk("Write_1S_A %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw20_1s);


		//PATH A, BW20-2S
		diff_bw20_2s = (HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_A(ch_idx) & 0x0f);
		diff_bw20_2s = convert_diff(diff_bw20_2s);
		tmp_TPI = pwr_40_1s + diff_bw20_1s + diff_bw20_2s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_A(Adapter, writeVal);

		//printk("Write_2S_A %d = %d + %d + %d\n", tmp_TPI, pwr_40_1s , diff_bw20_1s, diff_bw20_2s);

		// ==== //

		//PATH B, BW20-1S
		pwr_40_1s = HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx) ;
		diff_bw20_1s = ((HAL_VAR_pwrdiff_5G_20BW1S_OFDM1T_B(ch_idx) & 0xf0) >> 4);
		diff_bw20_1s = convert_diff(diff_bw20_1s);
		tmp_TPI = pwr_40_1s + diff_bw20_1s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_B(Adapter, writeVal);

		//printk("Write_1S_B %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw20_1s);

		//PATH B, BW20-2S
		diff_bw20_2s = (HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_B(ch_idx) & 0x0f);
		diff_bw20_2s = convert_diff(diff_bw20_2s);
		tmp_TPI = pwr_40_1s + diff_bw20_1s + diff_bw20_2s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_B(Adapter, writeVal);

		//printk("Write_2S_B %d = %d + %d + %d\n", tmp_TPI, pwr_40_1s , diff_bw20_1s, diff_bw20_2s);

	} else if (HAL_VAR_dot11nUse40M == 1) {
		//PATH A, BW40-1S
		pwr_40_1s = HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx);
		tmp_TPI = pwr_40_1s ;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_A(Adapter, writeVal);

		//printk("Write_1S_A %d = %d \n", tmp_TPI, pwr_40_1s);


		//PATH A, BW40-2S
		diff_bw40_2s = ((HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_A(ch_idx) & 0xf0) >> 4);
		diff_bw40_2s = convert_diff(diff_bw40_2s);
		tmp_TPI = pwr_40_1s + diff_bw40_2s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_A(Adapter, writeVal);

		//printk("Write_2S_A %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw40_2s);

		// ==== //

		//PATH B, BW40-1S
	    pwr_40_1s = HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx);
		tmp_TPI = pwr_40_1s ;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_B(Adapter, writeVal);

		//printk("Write_1S_B %d = %d \n", tmp_TPI, pwr_40_1s);


		//PATH A, BW40-2S
		diff_bw40_2s = ((HAL_VAR_pwrdiff_5G_40BW2S_20BW2S_B(ch_idx) & 0xf0) >> 4);
		diff_bw40_2s = convert_diff(diff_bw40_2s);
		tmp_TPI = pwr_40_1s + diff_bw40_2s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_B(Adapter, writeVal);

		//printk("Write_2S_B %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw40_2s);

	} else if (HAL_VAR_dot11nUse40M == 2) {
		//PATH A, BW80-1S
		pwr_40_1s = (HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx-4) +
					 HAL_VAR_pwrlevel5GHT40_1S_A(ch_idx+4)) / 2  ;
		diff_bw80_1s = ((HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_A(ch_idx) & 0xf0) >> 4);
		diff_bw80_1s = convert_diff(diff_bw80_1s);
		tmp_TPI = pwr_40_1s + diff_bw80_1s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_A(Adapter, writeVal);

		//printk("Write_1S_A %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw80_1s);


		//PATH A, BW80-2S
		diff_bw80_2s = ((HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_A(ch_idx) & 0xf0) >> 4);
		diff_bw80_2s = convert_diff(diff_bw80_2s);
		tmp_TPI = pwr_40_1s + diff_bw80_1s + diff_bw80_1s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_A(Adapter, writeVal);

		//printk("Write_2S_A %d = %d + %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw80_1s, diff_bw80_1s);


		//PATH B, BW80-1S
		pwr_40_1s = (HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx-4)+
					 HAL_VAR_pwrlevel5GHT40_1S_B(ch_idx+4) / 2);
		diff_bw80_1s = ((HAL_VAR_pwrdiff_5G_80BW1S_160BW1S_B(ch_idx) & 0xf0) >> 4);
		diff_bw80_1s = convert_diff(diff_bw80_1s);
		tmp_TPI = pwr_40_1s + diff_bw80_1s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_B(Adapter, writeVal);

		//printk("Write_1S_B %d = %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw80_1s);


		//PATH B, BW80-2S
		diff_bw80_2s = ((HAL_VAR_pwrdiff_5G_80BW2S_160BW2S_B(ch_idx) & 0xf0) >> 4);
		diff_bw80_2s = convert_diff(diff_bw80_2s);
		tmp_TPI = pwr_40_1s + diff_bw80_1s + diff_bw80_1s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_B(Adapter, writeVal);

		//printk("Write_2S_B %d = %d + %d + %d \n", tmp_TPI, pwr_40_1s , diff_bw80_1s, diff_bw80_1s);

	}
}

void 
Cal_OFDMTxPower_2G(
    IN  HAL_PADAPTER    Adapter, 
    IN  u1Byte          ch_idx
)
{
	unsigned char tmp_TPI = 0;
	unsigned char pwr_40_1s = 0;
	signed char diff_ofdm_1t = 0;
	signed char diff_bw40_2s = 0;
	signed char diff_bw20_1s = 0;
	signed char diff_bw20_2s = 0;

	unsigned int  writeVal = 0;

	if ((HAL_VAR_pwrlevelHT40_1S_A(ch_idx) == 0)
			|| (HAL_VAR_pwrlevelHT40_1S_B(ch_idx) == 0)) {
		use_DefaultOFDMTxPower88XX(Adapter);
		return;
	}

	// PATH A, OFDM
	pwr_40_1s = HAL_VAR_pwrlevelHT40_1S_A(ch_idx);
	diff_ofdm_1t = (HAL_VAR_pwrdiff_20BW1S_OFDM1T_A(ch_idx) & 0x0f);
	diff_ofdm_1t = convert_diff(diff_ofdm_1t);
	tmp_TPI = pwr_40_1s + diff_ofdm_1t;
	writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
	Write_OFDM_A(Adapter, writeVal);


	// PATH B, OFDM
	pwr_40_1s = HAL_VAR_pwrlevelHT40_1S_B(ch_idx);
	diff_ofdm_1t = (HAL_VAR_pwrdiff_20BW1S_OFDM1T_B(ch_idx) & 0x0f);
	diff_ofdm_1t = convert_diff(diff_ofdm_1t);
	tmp_TPI = pwr_40_1s + diff_ofdm_1t;
	writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
	Write_OFDM_B(Adapter, writeVal);

	if (HAL_VAR_dot11nUse40M == 0) {
		//PATH A, BW20-1S
		pwr_40_1s = HAL_VAR_pwrlevelHT40_1S_A(ch_idx);
		diff_bw20_1s = ((HAL_VAR_pwrdiff_20BW1S_OFDM1T_A(ch_idx) & 0xf0) >> 4);
		diff_bw20_1s = convert_diff(diff_bw20_1s);
		tmp_TPI = pwr_40_1s + diff_bw20_1s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_A(Adapter, writeVal);


		//PATH A, BW20-2S
		diff_bw20_2s = (HAL_VAR_pwrdiff_40BW2S_20BW2S_A(ch_idx) & 0x0f);
		diff_bw20_2s = convert_diff(diff_bw20_2s);
		tmp_TPI = pwr_40_1s + diff_bw20_1s + diff_bw20_2s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_A(Adapter, writeVal);

		// ==== //

		//PATH B, BW20-1S
		pwr_40_1s = HAL_VAR_pwrlevelHT40_1S_B(ch_idx);
		diff_bw20_1s = ((HAL_VAR_pwrdiff_20BW1S_OFDM1T_B(ch_idx) & 0xf0) >> 4);
		diff_bw20_1s = convert_diff(diff_bw20_1s);
		tmp_TPI = pwr_40_1s + diff_bw20_1s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_B(Adapter, writeVal);

		//PATH B, BW20-2S
		diff_bw20_2s = (HAL_VAR_pwrdiff_40BW2S_20BW2S_B(ch_idx) & 0x0f);
		diff_bw20_2s = convert_diff(diff_bw20_2s);
		tmp_TPI = pwr_40_1s + diff_bw20_1s + diff_bw20_2s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_B(Adapter, writeVal);

	} else if (HAL_VAR_dot11nUse40M == 1) {
		//PATH A, BW40-1S
		pwr_40_1s = HAL_VAR_pwrlevelHT40_1S_A(ch_idx);
		tmp_TPI = pwr_40_1s ;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_A(Adapter, writeVal);


		//PATH A, BW40-2S
		diff_bw40_2s = ((HAL_VAR_pwrdiff_40BW2S_20BW2S_A(ch_idx) & 0xf0) >> 4);
		diff_bw40_2s = convert_diff(diff_bw40_2s);
		tmp_TPI = pwr_40_1s + diff_bw40_2s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_A(Adapter, writeVal);

		// ==== //

		//PATH B, BW40-1S
		pwr_40_1s = HAL_VAR_pwrlevelHT40_1S_B(ch_idx);
		tmp_TPI = pwr_40_1s ;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_1S_B(Adapter, writeVal);


		//PATH A, BW40-2S
		diff_bw40_2s = ((HAL_VAR_pwrdiff_40BW2S_20BW2S_B(ch_idx) & 0xf0) >> 4);
		diff_bw40_2s = convert_diff(diff_bw40_2s);
		tmp_TPI = pwr_40_1s + diff_bw40_2s;
		writeVal = (tmp_TPI << 24) | (tmp_TPI << 16) | (tmp_TPI << 8) | tmp_TPI;
		Write_2S_B(Adapter, writeVal);

	}
}



s1Byte
convert_diff(
    IN s1Byte value
)
{
	// range from -8 ~ 7
	if (value <= 7)
		return value;
	else
		return (value - 16);
}

void 
Write_OFDM_A(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
)
{
	HAL_RTL_W32(rTxAGC_A_Ofdm18_Ofdm6_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_A_Ofdm54_Ofdm24_JAguar, writeVal);
}

void 
Write_1S_A(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
)
{
	HAL_RTL_W32(rTxAGC_A_MCS3_MCS0_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_A_MCS7_MCS4_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void 
Write_2S_A(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
)
{
	HAL_RTL_W32(rTxAGC_A_MCS11_MCS8_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_A_MCS15_MCS12_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar, writeVal);
	writeVal = (writeVal & 0xffff0000) | (HAL_RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar) & 0xffff);
	HAL_RTL_W32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void 
Write_OFDM_B(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
)
{
	HAL_RTL_W32(rTxAGC_B_Ofdm18_Ofdm6_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_B_Ofdm54_Ofdm24_JAguar, writeVal);
}


void 
Write_1S_B(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
)
{
	HAL_RTL_W32(rTxAGC_B_MCS3_MCS0_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_B_MCS7_MCS4_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_B_Nss1Index3_Nss1Index0_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void 
Write_2S_B(
    IN  HAL_PADAPTER    Adapter, 
    IN  u4Byte          writeVal
)    
{
	HAL_RTL_W32(rTxAGC_B_MCS11_MCS8_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_B_MCS15_MCS12_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_B_Nss2Index5_Nss2Index2_JAguar, writeVal);
	HAL_RTL_W32(rTxAGC_B_Nss2Index9_Nss2Index6_JAguar, writeVal);
	writeVal = (writeVal & 0xffff0000) | (HAL_RTL_R32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar) & 0xffff);
	HAL_RTL_W32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar, writeVal);
}

void 
use_DefaultOFDMTxPower88XX(
    IN  HAL_PADAPTER    Adapter
)
{

	unsigned int def_power = 0x20202020;//0x12121212;
	//printk("%s NO Calibration data, use default OFDM power = 0x%x\n",__FUNCTION__, def_power);

	HAL_RTL_W32(rTxAGC_A_Ofdm18_Ofdm6_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_A_Ofdm54_Ofdm24_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_A_MCS3_MCS0_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_A_MCS7_MCS4_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_A_MCS11_MCS8_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_A_MCS15_MCS12_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_A_Nss1Index3_Nss1Index0_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_A_Nss1Index7_Nss1Index4_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_A_Nss2Index1_Nss1Index8_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_A_Nss2Index5_Nss2Index2_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_A_Nss2Index9_Nss2Index6_JAguar, def_power);

	HAL_RTL_W32(rTxAGC_B_Ofdm18_Ofdm6_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_B_Ofdm54_Ofdm24_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_B_MCS3_MCS0_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_B_MCS7_MCS4_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_B_MCS11_MCS8_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_B_MCS15_MCS12_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_B_Nss1Index3_Nss1Index0_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_B_Nss1Index7_Nss1Index4_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_B_Nss2Index1_Nss1Index8_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_B_Nss2Index5_Nss2Index2_JAguar, def_power);
	HAL_RTL_W32(rTxAGC_B_Nss2Index9_Nss2Index6_JAguar, def_power);

}



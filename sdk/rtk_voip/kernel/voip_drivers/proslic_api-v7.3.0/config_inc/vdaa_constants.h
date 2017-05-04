/*
** Copyright (c) 2012 Silicon Laboratories, Inc.
** 2013-01-09 11:24:13
**
** Si3217x ProSLIC API Configuration Tool Version 2.12.0
*/


#ifndef VDAA_CONSTANTS_H
#define VDAA_CONSTANTS_H

/** Vdaa_Country Presets */
#ifdef REALTEK_PATCH_FOR_SILAB
enum {
	VDAA_COUNTRY_USA,
	VDAA_COUNTRY_UK,
	VDAA_COUNTRY_Australia,
	VDAA_COUNTRY_HK,
	VDAA_COUNTRY_JP,
	VDAA_COUNTRY_Sweden,
	VDAA_COUNTRY_Germany,
	VDAA_COUNTRY_France,
	VDAA_COUNTRY_TW,
	VDAA_COUNTRY_Belgium,
	VDAA_COUNTRY_Finland,
	VDAA_COUNTRY_Italy,
	VDAA_COUNTRY_China
};
#else
enum {
	COU_USA,
	COU_GERMANY,
	COU_CHINA,
	COU_AUSTRALIA
};
#endif

/** Vdaa_Audio_Gain Presets */
enum {
	AUDIO_GAIN_0DB,
	AUDIO_ATTEN_4DB,
	AUDIO_ATTEN_6DB,
	AUDIO_ATTEN_11DB
};

/** Vdaa_Ring_Validation Presets */
enum {
	RING_DET_NOVAL_LOWV,
	RING_DET_VAL_HIGHV
};

/** Vdaa_PCM Presets */
enum {
	PCM_8ULAW,
	PCM_8ALAW,
	PCM_16LIN
};

/** Vdaa_Hybrid Presets */
enum {
	HYB_600_0_0_500FT_24AWG,
	HYB_270_750_150_500FT_24AWG,
	HYB_200_680_100_500FT_24AWG,
	HYB_220_820_120_500FT_24AWG
};

#endif


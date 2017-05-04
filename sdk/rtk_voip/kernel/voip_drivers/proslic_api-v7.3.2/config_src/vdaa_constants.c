/*
** Copyright (c) 2012 Silicon Laboratories, Inc.
** 2014-06-19 15:58:20
**
** Si3217x ProSLIC API Configuration Tool Version 3.3.1
*/



#include "vdaa.h"

vdaa_General_Cfg Vdaa_General_Configuration  = {
    INTE_DISABLED,
    INTE_ACTIVE_LOW,
    RES_CAL_ENABLED,
    FS_8KHZ,
    FOH_128,
    LVS_FORCE_ENABLED,
    CVS_CURRENT,
    CVP_ABOVE,
    GCE_DISABLED,
    IIR_DISABLED,
    FULL2_ENABLED,
    FULL_DISABLED,
    FILT_HPF_200HZ,
    RG1_DISABLED,
    PWM_DELTA_SIGMA,
    PWM_DISABLED,
    SPIM_TRI_CS
};

#ifdef REALTEK_PATCH_FOR_SILAB

vdaa_Country_Cfg Vdaa_Country_Presets[] ={
    {
    RZ_MAX,
    DC_50,
    AC_600,
    DCV3_5,
    MINI_10MA,
    ILIM_DISABLED,
    OHS_LESS_THAN_0_5MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_USA */
    {
    RZ_MAX,
    DC_50,
    AC_320__1050_230,
    DCV3_5,
    MINI_10MA,
    ILIM_ENABLED,
    OHS_3MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_UK */
    {
    RZ_MAX,
    DC_50,
    AC_220__820_120,
    DCV3_2,
    MINI_12MA,
    ILIM_DISABLED,
    OHS_26MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_Australia */
    {
    RZ_MAX,
    DC_50,
    AC_600,
    DCV3_5,
    MINI_10MA,
    ILIM_DISABLED,
    OHS_LESS_THAN_0_5MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_HK */
    {
    RZ_MAX,
    DC_50,
    AC_600,
    DCV3_2,
    MINI_12MA,
    ILIM_DISABLED,
    OHS_LESS_THAN_0_5MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_JP */
    {
    RZ_MAX,
    DC_50,
    AC_270__750_150,
    DCV3_5,
    MINI_10MA,
    ILIM_ENABLED,
    OHS_3MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_Sweden */
    {
    RZ_MAX,
    DC_50,
    AC_270__750_150,
    DCV3_5,
    MINI_10MA,
    ILIM_ENABLED,
    OHS_3MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_Germany */
    {
    RZ_MAX,
    DC_50,
    AC_270__750_150,
    DCV3_5,
    MINI_10MA,
    ILIM_ENABLED,
    OHS_3MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_France */
    {
    RZ_MAX,
    DC_50,
    AC_600,
    DCV3_5,
    MINI_10MA,
    ILIM_DISABLED,
    OHS_LESS_THAN_0_5MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_TW */
    {
    RZ_MAX,
    DC_50,
    AC_270__750_150,
    DCV3_5,
    MINI_10MA,
    ILIM_ENABLED,
    OHS_3MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_Belgium */
    {
    RZ_MAX,
    DC_50,
    AC_270__750_150,
    DCV3_5,
    MINI_10MA,
    ILIM_ENABLED,
    OHS_3MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_Finland */
    {
    RZ_MAX,
    DC_50,
    AC_270__750_150,
    DCV3_5,
    MINI_10MA,
    ILIM_ENABLED,
    OHS_3MS,
    HYBRID_ENABLED
    },  /* VDAA_COUNTRY_Italy */
    {
    RZ_MAX,
    DC_50,
    AC_200__680_100,
    DCV3_5,
    MINI_10MA,
    ILIM_DISABLED,
    OHS_LESS_THAN_0_5MS,
    HYBRID_ENABLED
    }   /* VDAA_COUNTRY_China */
};


#else

vdaa_Country_Cfg Vdaa_Country_Presets[] ={
    {
    RZ_MAX,
    DC_50,
    AC_600,
    DCV3_5,
    MINI_10MA,
    ILIM_DISABLED,
    OHS_LESS_THAN_0_5MS,
    HYBRID_ENABLED
    },  /* COU_USA */
    {
    RZ_MAX,
    DC_50,
    AC_270__750_150,
    DCV3_5,
    MINI_10MA,
    ILIM_ENABLED,
    OHS_3MS,
    HYBRID_ENABLED
    },  /* COU_GERMANY */
    {
    RZ_MAX,
    DC_50,
    AC_200__680_100,
    DCV3_5,
    MINI_10MA,
    ILIM_DISABLED,
    OHS_LESS_THAN_0_5MS,
    HYBRID_ENABLED
    },  /* COU_CHINA */
    {
    RZ_MAX,
    DC_50,
    AC_220__820_120,
    DCV3_2,
    MINI_12MA,
    ILIM_DISABLED,
    OHS_26MS,
    HYBRID_ENABLED
    }   /* COU_AUSTRALIA */
};
#endif

vdaa_audioGain_Cfg Vdaa_audioGain_Presets[] ={
    {
    0,    /* mute */
    XGA_GAIN,    /* xXGA2 */
    0,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_GAIN_0DB */
#ifdef REALTEK_PATCH_FOR_SILAB
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    1,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_1DB */
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    2,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_2DB */
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    3,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_3DB */
#endif	
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    4,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_4DB */
#ifdef REALTEK_PATCH_FOR_SILAB
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    5,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_5DB */
#endif
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    6,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_6DB */
#ifdef REALTEK_PATCH_FOR_SILAB
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    7,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_7DB */
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    8,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_8DB */
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    9,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_9DB */
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    10,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    },  /* AUDIO_ATTEN_10DB */
#endif
    {
    0,    /* mute */
    XGA_ATTEN,    /* xXGA2 */
    11,    /* xXG2 */
    XGA_GAIN,    /* xXGA3 */
    0,    /* xXG3 */
    64,    /* AxM */
    0     /* cpEn */
    }   /* AUDIO_ATTEN_11DB */
};

vdaa_Ring_Detect_Cfg Vdaa_Ring_Detect_Presets[] ={
    {
    RDLY_512MS,
    RT__13_5VRMS_16_5VRMS,
    12,    /* RMX */
    RTO_1408MS,
    RCC_640MS,
    RNGV_DISABLED,
    17,    /* RAS */
    RFWE_HALF_WAVE,
    RDI_BEG_END_BURST, 
    RGDT_ACTIVE_LOW 
    },  /* RING_DET_NOVAL_LOWV */
    {
    RDLY_512MS,
    RT__40_5VRMS_49_5VRMS,
    12,    /* RMX */
    RTO_1408MS,
    RCC_640MS,
    RNGV_ENABLED,
    17,    /* RAS */
    RFWE_RNGV_RING_ENV,
    RDI_BEG_END_BURST, 
    RGDT_ACTIVE_LOW 
    }   /* RING_DET_VAL_HIGHV */
};

vdaa_PCM_Cfg Vdaa_PCM_Presets[] ={
    {
    U_LAW,
    PCLK_1_PER_BIT,
    TRI_POS_EDGE
    },  /* PCM_8ULAW */
    {
    A_LAW,
    PCLK_1_PER_BIT,
    TRI_POS_EDGE
    },  /* PCM_8ALAW */
    {
    LINEAR_16_BIT,
    PCLK_1_PER_BIT,
    TRI_POS_EDGE
    }   /* PCM_16LIN */
};

vdaa_Hybrid_Cfg Vdaa_Hybrid_Presets[] ={
    {
       0,    /* HYB1 */
     254,    /* HYB2 */
       0,    /* HYB3 */
       1,    /* HYB4 */
     255,    /* HYB5 */
       1,    /* HYB6 */
       0,    /* HYB7 */
       0    /* HYB8 */
    },  /* HYB_600_0_0_500FT_24AWG */
    {
       4,    /* HYB1 */
     246,    /* HYB2 */
     242,    /* HYB3 */
       4,    /* HYB4 */
     254,    /* HYB5 */
     255,    /* HYB6 */
       1,    /* HYB7 */
     255    /* HYB8 */
    },  /* HYB_270_750_150_500FT_24AWG */
    {
       4,    /* HYB1 */
     245,    /* HYB2 */
     243,    /* HYB3 */
       7,    /* HYB4 */
     253,    /* HYB5 */
       0,    /* HYB6 */
       1,    /* HYB7 */
     255    /* HYB8 */
    },  /* HYB_200_680_100_500FT_24AWG */
    {
       4,    /* HYB1 */
     244,    /* HYB2 */
     241,    /* HYB3 */
       6,    /* HYB4 */
     253,    /* HYB5 */
     255,    /* HYB6 */
       2,    /* HYB7 */
     255    /* HYB8 */
    }   /* HYB_220_820_120_500FT_24AWG */
};

#ifdef REALTEK_PATCH_FOR_SILAB
vdaa_Hybrid_Cfg2 Vdaa_Hybrid_Presets2[] ={
	/* 600 ohm echo settings */
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 10, 0, 6, 1, 254, 2, 255, 0, 0},
	{ 3, 255, 255, 0, 1, 0, 0, 0, 0},
	{ 3, 1, 253, 253, 2, 255, 0, 0, 0},
	{ 9, 254, 251, 255, 2, 0, 1, 0, 0},
	{ 5, 3, 251, 250, 2, 254, 0, 0, 255},
	{ 8, 253, 2, 244, 255, 10, 244, 3, 253},
	{ 10, 249, 244, 8, 12, 245, 252, 0, 1},
	
	/* 900 ohm echo settings */
	{ 1, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 10, 252, 255, 1, 255, 0, 0, 0, 0},
	{ 7, 255, 251, 251, 2, 255, 255, 1, 255},
	{ 3, 1, 251, 250, 1, 254, 255, 0, 255},
	{ 5, 252, 250, 0, 0, 255, 1, 0, 0},
	{ 5, 3, 251, 250, 1, 253, 0, 0, 255},
	{ 8, 253, 2, 244, 255, 10, 244, 3, 253},
	{ 10, 249, 244, 8, 12, 245, 252, 0, 1},

	/* 270 ohm + (750 ohm || 150 nF) (CTR21) */
	{ 2, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 7, 0, 0, 255, 254, 0, 0, 0, 0},
	{ 9, 0, 253, 254, 2, 255, 0, 0, 0},
	{ 5, 1, 249, 254, 4, 253, 1, 0, 0},
	{ 5, 252, 250, 1, 1, 254, 0, 255, 0},
	{ 5, 3, 251, 250, 2, 253, 255, 255, 255},
	{ 8, 253, 2, 244, 255, 10, 244, 3, 253},
	{ 10, 249, 244, 8, 12, 245, 252, 0, 1},

	/* 220 ohm + (820 ohm || 120 nF) (Australia/NewZealand) and 220 ohm + (820 ohm
	 * || 115nF) (Slovakia/SAfrica/Germany/Austria/Bulgaria)
	 */
	{ 3, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 7, 0, 255, 254, 255, 0, 255, 0, 0},
	{ 9, 0, 253, 253, 1, 255, 0, 0, 0},
	{ 5, 1, 249, 254, 3, 253, 1, 0, 0},
	{ 5, 252, 250, 1, 1, 254, 0, 255, 0},
	{ 5, 3, 251, 251, 2, 253, 255, 255, 255},
	{ 8, 253, 2, 244, 255, 10, 244, 3, 253},
	{ 10, 249, 244, 8, 12, 245, 252, 0, 1},
	
	/* 370 ohm + (620ohm || 310nF) (New Zealand #2/India) CO Termination */
	{ 4, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 9, 255, 1, 4, 0, 0, 1, 255, 0},
	{ 9, 0, 253, 0, 3, 254, 0, 0, 255},
	{ 9, 2, 250, 253, 5, 253, 1, 0 ,255},
	{ 5, 252, 250, 1, 2, 255, 0 ,255, 0},
	{ 5, 3, 251, 250, 3, 254, 255, 255, 255},
	{ 8, 253, 2, 244, 255, 10, 244, 3, 253},
	{ 10, 249, 244, 8, 12, 245, 252, 0, 1},
	
	/* 320 ohm + (1050ohm || 230 nF) (England) CO Termination */
	{ 5, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 9, 0 ,255, 1, 255, 255, 0, 255, 0},
	{ 5, 255, 252, 0, 2, 254, 0, 255, 255},
	{ 9, 2, 250, 253, 4, 252, 0, 255, 255},
	{ 5, 252, 250, 1, 1, 254, 0 ,255, 255},
	{ 5, 3, 251, 250, 2, 253, 255, 255, 254},
	{ 3, 1, 1, 242, 2, 9, 245, 3, 253},
	{ 10, 249, 244, 8, 12, 245, 252, 0, 1},

	/* 370 ohm + (820 ohm || 110 nF) CO Termination */
	{ 6, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 6, 1, 254, 253, 0, 255, 0, 0, 0},
	{ 9, 0, 251, 252, 2, 255, 0, 0, 0},
	{ 5, 1, 248, 252, 4, 253, 1, 0, 0},
	{ 5, 252, 250, 0, 0, 254, 0 , 255, 0},
	{ 5, 3, 251, 250, 2, 253, 255, 255, 254},
	{ 3, 1, 1, 242, 2, 9, 245, 3, 253},
	{ 10, 249, 244, 8, 12, 245, 252, 0, 1},

	/* 275 ohm + (780 ohm || 115 nF) CO Termination */
	{ 7, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 7, 255, 255, 255, 255, 0, 0, 0, 0},
	{ 9, 0, 253, 254, 2, 255, 0, 0, 0},
	{ 5, 1, 249, 254, 4, 253, 1, 0, 0},
	{ 5, 252, 250, 1, 1, 254, 0, 255, 0},
	{ 5, 3, 251, 250, 2, 253, 255, 255, 255},
	{ 8, 253, 2, 244, 255, 10, 244, 3, 253},
	{ 10, 249, 244, 8, 12, 245, 252, 0, 1},
	
	/* Make sure we include the rest of the impedances */
	{ 8, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 9, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 10, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 11, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 12, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 13, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 14, 0, 0, 0, 0, 0, 0, 0, 0},
	{ 15, 0, 0, 0, 0, 0, 0, 0, 0},
};
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "voip_manager.h"

typedef struct {
        uint32 slope_vlim;
        uint32 slope_rfeed;
        uint32 slope_ilim;
        uint32 delta1;
        uint32 delta2;
        uint32 v_vlim;
        uint32 v_rfeed;
        uint32 v_ilim;
        uint32 const_rfeed;
        uint32 const_ilim;
        uint32 i_vlim;
        uint32 lcronhk;
        uint32 lcroffhk;
        uint32 lcrdbi;
        uint32 longhith;
        uint32 longloth;
        uint32 longdbi;
        uint32 lcrmask;
        uint32 lcrmask_polrev;
        uint32 lcrmask_state;
        uint32 lcrmask_linecap;
        uint32 vcm_oh;
        uint32 vov_bat;
        uint32 vov_gnd;
} ProSLIC_DCfeed_Cfg;


typedef ProSLIC_DCfeed_Cfg Si3226x_DCfeed_Cfg;

typedef struct {
	uint32 rtper;
	uint32 freq;
	uint32 amp;
	uint32 phas;
        uint32 offset;
        uint32 slope_ring;
        uint32 iring_lim;
        uint32 rtacth;
        uint32 rtdcth;
        uint32 rtacdb;
        uint32 rtdcdb;
        uint32 vov_ring_bat;
        uint32 vov_ring_gnd;
        uint32 vbatr_expect;
        uint8 talo;
        uint8 tahi;
	uint8 tilo;
        uint8 tihi;
        uint32 adap_ring_min_i;
        uint32 counter_iring_val;
        uint32 counter_vtr_val;
        uint32 ar_const28;
        uint32 ar_const32;
        uint32 ar_const38;
        uint32 ar_const46;
        uint32 rrd_delay;
        uint32 rrd_delay2;
        uint32 vbat_track_min_rng;
        uint8 ringcon;
        uint8 userstat;
        uint32 vcm_ring;
        uint32 vcm_ring_fixed;
        uint32 delta_vcm;
        uint32 dcdc_rngtype;
        uint32 vov_dcdc_slope;
        uint32 vov_dcdc_os;
        uint32 vov_ring_bat_max;
} Si3226x_Ring_Cfg;


typedef struct {
	uint32 txaceq_c0;
	uint32 txaceq_c1;
	uint32 txaceq_c2;
	uint32 txaceq_c3;

	uint32 rxaceq_c0;
	uint32 rxaceq_c1;
	uint32 rxaceq_c2;
	uint32 rxaceq_c3;
} Si3226x_audioEQ_Cfg;

typedef struct {
        uint32 ecfir_c2;
        uint32 ecfir_c3;
        uint32 ecfir_c4;
        uint32 ecfir_c5;
        uint32 ecfir_c6;
        uint32 ecfir_c7;
        uint32 ecfir_c8;
        uint32 ecfir_c9;
        uint32 ecfir_b0;
        uint32 ecfir_b1;
        uint32 ecfir_a1;
        uint32 ecfir_a2;
} Si3226x_hybrid_Cfg;

typedef struct {
        uint32 zsynth_b0;
        uint32 zsynth_b1;
        uint32 zsynth_b2;
        uint32 zsynth_a1;
        uint32 zsynth_a2;
	uint8 ra;
} Si3226x_Zsynth_Cfg;


typedef struct {
	Si3226x_audioEQ_Cfg audioEQ;
	Si3226x_hybrid_Cfg hybrid;
    	Si3226x_Zsynth_Cfg zsynth;
	uint32 txgain;
	uint32 rxgain;
	uint32 rxachpf_b0_1;
	uint32  rxachpf_b1_1;
	uint32  rxachpf_a1_1;
	int16 txgain_db; /*overall gain associated with this configuration*/
	int16 rxgain_db;
} Si3226x_Impedance_Cfg;

Si3226x_Ring_Cfg Si3226x_Ring_profile[] =
{
	{
	/*
		Loop = 500 ft @ 0.044 ohms/ft, REN = 5, Rcpe = 600 ohms
		Rprot = 30 ohms, Type = BALANCED, Waveform = SINE
	*/ 
	0x00050000L,	/* RTPER */
	0x07EFE000L,	/* RINGFR (20.000 Hz) */
	0x001B9F2EL,	/* RINGAMP (45.000 vrms)  */
	0x00000000L,	/* RINGPHAS */
	0x00000000L,	/* RINGOF (0.000 vdc) */
	0x15E5200EL,	/* SLOPE_RING (100.000 ohms) */
	0x00D16348L,	/* IRING_LIM (90.000 mA) */
	0x0068E9B4L,	/* RTACTH (57.936 mA) */
	0x0FFFFFFFL,	/* RTDCTH (450.000 mA) */
	0x00006000L,	/* RTACDB (75.000 ms) */
	0x00006000L,	/* RTDCDB (75.000 ms) */
	0x00C49BA0L,	/* VOV_RING_BAT (12.000 v) */
	0x00000000L,	/* VOV_RING_GND (0.000 v) */
	0x0558ABFCL,	/* VBATR_EXPECT (83.537 v) */
	0x80,			/* RINGTALO (2.000 s) */
	0x3E,			/* RINGTAHI */
	0x00,			/* RINGTILO (4.000 s) */
	0x7D,			/* RINGTIHI */
	0x00000000L,	/* ADAP_RING_MIN_I */
	0x00003000L,	/* COUNTER_IRING_VAL */
	0x00051EB8L,	/* COUNTER_VTR_VAL */
	0x00000000L,	/* CONST_028 */
	0x00000000L,	/* CONST_032 */
	0x00000000L,	/* CONST_038 */
	0x00000000L,	/* CONST_046 */
	0x00000000L,	/* RRD_DELAY */
	0x00000000L,	/* RRD_DELAY2 */
	0x01893740L,	/* VBAT_TRACK_MIN_RNG */
	0x18,			/* RINGCON */
	0x00,			/* USERSTAT */
	0x02AC55FEL,	/* VCM_RING (38.769 v) */
	0x02AC55FEL,	/* VCM_RING_FIXED */
	0x003126E8L,	/* DELTA_VCM */
	0x00200000L,	/* DCDC_RNGTYPE */
	0x00FFFFFFL,	/* VOV_DCDC_SLOPE */
	0x0083126AL,	/* VOV_DCDC_OS */
	0x009374B8L,	/* VOV_RING_BAT_MAX */
	},  /* 20Hz_45VRMS_0VDC_BAL_600ohmCPE */



	{
	/*
		Loop = 500 ft @ 0.044 ohms/ft, REN = 5, Rcpe = 600 ohms
		Rprot = 30 ohms, Type = BALANCED, Waveform = SINE
	*/ 
	0x00050000L,	/* RTPER */
	0x07EFE000L,	/* RINGFR (20.000 Hz) */
	0x001D7698L,	/* RINGAMP (48.000 vrms)  */
	0x00000000L,	/* RINGPHAS */
	0x00000000L,	/* RINGOF (0.000 vdc) */
	0x15E5200EL,	/* SLOPE_RING (100.000 ohms) */
	0x00D16348L,	/* IRING_LIM (90.000 mA) */
	0x006FE837L,	/* RTACTH (61.799 mA) */
	0x0FFFFFFFL,	/* RTDCTH (450.000 mA) */
	0x00006000L,	/* RTACDB (75.000 ms) */
	0x00006000L,	/* RTDCDB (75.000 ms) */
	0x00C49BA0L,	/* VOV_RING_BAT (12.000 v) */
	0x00000000L,	/* VOV_RING_GND (0.000 v) */
	0x05A6CF35L,	/* VBATR_EXPECT (88.306 v) */
	0x80,			/* RINGTALO (2.000 s) */
	0x3E,			/* RINGTAHI */
	0x00,			/* RINGTILO (4.000 s) */
	0x7D,			/* RINGTIHI */
	0x00000000L,	/* ADAP_RING_MIN_I */
	0x00003000L,	/* COUNTER_IRING_VAL */
	0x00051EB8L,	/* COUNTER_VTR_VAL */
	0x00000000L,	/* CONST_028 */
	0x00000000L,	/* CONST_032 */
	0x00000000L,	/* CONST_038 */
	0x00000000L,	/* CONST_046 */
	0x00000000L,	/* RRD_DELAY */
	0x00000000L,	/* RRD_DELAY2 */
	0x01893740L,	/* VBAT_TRACK_MIN_RNG */
	0x18,			/* RINGCON */
	0x00,			/* USERSTAT */
	0x02D3679AL,	/* VCM_RING (41.153 v) */
	0x02D3679AL,	/* VCM_RING_FIXED */
	0x003126E8L,	/* DELTA_VCM */
	0x00200000L,	/* DCDC_RNGTYPE */
	0x00FFFFFFL,	/* VOV_DCDC_SLOPE */
	0x0083126AL,	/* VOV_DCDC_OS */
	0x009374B8L,	/* VOV_RING_BAT_MAX */
	},  /* 20Hz_48VRMS_0VDC_BAL_600ohmCPE */



	{
	/*
		Loop = 500 ft @ 0.044 ohms/ft, REN = 5, Rcpe = 600 ohms
		Rprot = 30 ohms, Type = BALANCED, Waveform = SINE
	*/ 
	0x00040000L,	/* RTPER */
	0x07E6C000L,	/* RINGFR (25.000 Hz) */
	0x0024DF0DL,	/* RINGAMP (48.000 vrms)  */
	0x00000000L,	/* RINGPHAS */
	0x00000000L,	/* RINGOF (0.000 vdc) */
	0x15E5200EL,	/* SLOPE_RING (100.000 ohms) */
	0x00D16348L,	/* IRING_LIM (90.000 mA) */
	0x0059D242L,	/* RTACTH (62.003 mA) */
	0x0FFFFFFFL,	/* RTDCTH (450.000 mA) */
	0x00008000L,	/* RTACDB (75.000 ms) */
	0x00008000L,	/* RTDCDB (75.000 ms) */
	0x00C49BA0L,	/* VOV_RING_BAT (12.000 v) */
	0x00000000L,	/* VOV_RING_GND (0.000 v) */
	0x05A75077L,	/* VBATR_EXPECT (88.337 v) */
	0x80,			/* RINGTALO (2.000 s) */
	0x3E,			/* RINGTAHI */
	0x00,			/* RINGTILO (4.000 s) */
	0x7D,			/* RINGTIHI */
	0x00000000L,	/* ADAP_RING_MIN_I */
	0x00003000L,	/* COUNTER_IRING_VAL */
	0x00066666L,	/* COUNTER_VTR_VAL */
	0x00000000L,	/* CONST_028 */
	0x00000000L,	/* CONST_032 */
	0x00000000L,	/* CONST_038 */
	0x00000000L,	/* CONST_046 */
	0x00000000L,	/* RRD_DELAY */
	0x00000000L,	/* RRD_DELAY2 */
	0x01893740L,	/* VBAT_TRACK_MIN_RNG */
	0x18,			/* RINGCON */
	0x00,			/* USERSTAT */
	0x02D3A83BL,	/* VCM_RING (41.169 v) */
	0x02D3A83BL,	/* VCM_RING_FIXED */
	0x003126E8L,	/* DELTA_VCM */
	0x00200000L,	/* DCDC_RNGTYPE */
	0x00FFFFFFL,	/* VOV_DCDC_SLOPE */
	0x0083126AL,	/* VOV_DCDC_OS */
	0x009374B8L,	/* VOV_RING_BAT_MAX */
	}  /* 25Hz_48VRMS_0VDC_BAL_600ohmCPE */
};

Si3226x_DCfeed_Cfg Si3226x_DCfeed_profile[] = 
{
	{
	0x1C8A024CL,	/* SLOPE_VLIM */
	0x1F909679L,	/* SLOPE_RFEED */
	0x0040A0E0L,	/* SLOPE_ILIM */
	0x1D5B21A9L,	/* SLOPE_DELTA1 */
	0x1DD87A3EL,	/* SLOPE_DELTA2 */
	0x05A38633L,	/* V_VLIM (48.000 v) */
	0x050D2839L,	/* V_RFEED (43.000 v) */
	0x03FE7F0FL,	/* V_ILIM  (34.000 v) */
	0x00B4F3C3L,	/* CONST_RFEED (15.000 mA) */
	0x005D0FA6L,	/* CONST_ILIM (20.000 mA) */
	0x002D8D96L,	/* I_VLIM (0.000 mA) */
	#ifdef REALTEK_PATCH_FOR_SILAB		//THLin: for 20pps pulse dial det
	0x00500000L,	/* LCRONHK (10.000 mA) */
	0x00600000L,	/* LCROFFHK (12.000 mA) */
	#else
	0x005B0AFBL,	/* LCRONHK (10.000 mA) */
	0x006D4060L,	/* LCROFFHK (12.000 mA) */
	#endif
	0x00008000L,	/* LCRDBI (5.000 ms) */
	0x0048D595L,	/* LONGHITH (8.000 mA) */
	0x003FBAE2L,	/* LONGLOTH (7.000 mA) */
	0x00008000L,	/* LONGDBI (5.000 ms) */
	0x000F0000L,	/* LCRMASK (150.000 ms) */
	0x00080000L,	/* LCRMASK_POLREV (80.000 ms) */
	0x00140000L,	/* LCRMASK_STATE (200.000 ms) */
	0x00140000L,	/* LCRMASK_LINECAP (200.000 ms) */
	0x01BA5E35L,	/* VCM_OH (27.000 v) */
	0x0051EB85L,	/* VOV_BAT (5.000 v) */
	0x00418937L,	/* VOV_GND (4.000 v) */
	},  /* DCFEED_48V_20MA */


	{
	0x1C8A024CL,	/* SLOPE_VLIM */
	0x1EE08C11L,	/* SLOPE_RFEED */
	0x0040A0E0L,	/* SLOPE_ILIM */
	0x1C940D71L,	/* SLOPE_DELTA1 */
	0x1DD87A3EL,	/* SLOPE_DELTA2 */
	0x05A38633L,	/* V_VLIM (48.000 v) */
	0x050D2839L,	/* V_RFEED (43.000 v) */
	0x03FE7F0FL,	/* V_ILIM  (34.000 v) */
	0x01241BC9L,	/* CONST_RFEED (15.000 mA) */
	0x0074538FL,	/* CONST_ILIM (25.000 mA) */
	0x002D8D96L,	/* I_VLIM (0.000 mA) */
	#ifdef REALTEK_PATCH_FOR_SILAB		//THLin: for 20pps pulse dial det
	0x00500000L,	/* LCRONHK (10.000 mA) */
	0x00600000L,	/* LCROFFHK (12.000 mA) */
	#else
	0x005B0AFBL,	/* LCRONHK (10.000 mA) */
	0x006D4060L,	/* LCROFFHK (12.000 mA) */
	#endif
	0x00008000L,	/* LCRDBI (5.000 ms) */
	0x0048D595L,	/* LONGHITH (8.000 mA) */
	0x003FBAE2L,	/* LONGLOTH (7.000 mA) */
	0x00008000L,	/* LONGDBI (5.000 ms) */
	0x000F0000L,	/* LCRMASK (150.000 ms) */
	0x00080000L,	/* LCRMASK_POLREV (80.000 ms) */
	0x00140000L,	/* LCRMASK_STATE (200.000 ms) */
	0x00140000L,	/* LCRMASK_LINECAP (200.000 ms) */
	0x01BA5E35L,	/* VCM_OH (27.000 v) */
	0x0051EB85L,	/* VOV_BAT (5.000 v) */
	0x00418937L,	/* VOV_GND (4.000 v) */
	}  /* DCFEED_48V_25MA */
};

Si3226x_Impedance_Cfg Si3226x_Impedance_profile[] =
/* Source: Database file: cwdb.db */
/* Database information: version: 1.0.2 build date: 2011-01-26*/
/* parameters: zref=200_680_100 rprot=30 rfuse=0 emi_cap=10*/
{
	/* Source: Database file: cwdb.db */
	/* Database information: version: 1.0.2 build date: 2011-01-26*/
	/* parameters: zref=600_0_0 rprot=30 rfuse=0 emi_cap=10*/
	{
	{0x07F46C00L, 0x000E4600L, 0x00008580L, 0x1FFD6100L,    /* TXACEQ */
	 0x07EF5000L, 0x0013F580L, 0x1FFDE000L, 0x1FFCB280L},   /* RXACEQ */
	{0x0027CB00L, 0x1F8A8880L, 0x02801180L, 0x1F625C80L,    /* ECFIR/ECIIR */
	 0x0314FB00L, 0x1E6B8E80L, 0x00C5FF00L, 0x1FC96F00L,
	 0x1FFD1200L, 0x00023C00L, 0x0ED29D00L, 0x192A9400L},
	{0x00810E00L, 0x1EFEBE80L, 0x00803500L, 0x0FF66D00L,    /* ZSYNTH */
	 0x18099080L, 0x59}, 
	 0x088E0D80L,   /* TXACGAIN */
	 0x01456D80L,   /* RXACGAIN */
	 0x07ABE580L, 0x18541B00L, 0x0757CB00L,    /* RXACHPF */
	#ifdef ENABLE_HIRES_GAIN
	 0, 0  /* TXGAIN*10, RXGAIN*10 (hi_res) */
	#else
	 0, 0  /* TXGAIN, RXGAIN */
	#endif
	 },  /* SLIC_IMPEDANCE_600_0_0 */
	 
	{ 
	{0x07365D80L, 0x1FC64180L, 0x00022980L, 0x1FFCE300L,    /* TXACEQ */
	 0x09C28580L, 0x1D1FD880L, 0x0071A280L, 0x1FDF7500L},   /* RXACEQ */
	{0x1FF15A00L, 0x005C0600L, 0x00828200L, 0x01B11D00L,    /* ECFIR/ECIIR */
	 0x027BB800L, 0x1EE9F200L, 0x028BAB80L, 0x1E57BE80L,
	 0x01007580L, 0x1EF8B780L, 0x0556EE80L, 0x028DFB80L},
	{0x014F2C00L, 0x1C7A1180L, 0x02369A00L, 0x0A138100L,    /* ZSYNTH */
	 0x1DEA2280L, 0x8E}, 
	 0x08000000L,   /* TXACGAIN */
	 0x010C7E80L,   /* RXACGAIN */
	 0x07BB2500L, 0x1844DB80L, 0x07764980L,    /* RXACHPF */
	#ifdef ENABLE_HIRES_GAIN
	 0, 0  /* TXGAIN*10, RXGAIN*10 (hi_res) */
	#else
	 0, 0  /* TXGAIN, RXGAIN */
	#endif
	 },  /* SLIC_IMPEDANCE_200_680_100 */
	 
	 
	 /* Source: Database file: cwdb.db */
	/* Database information: version: 1.0.2 build date: 2011-01-26*/
	/* parameters: zref=350_1000_210 rprot=30 rfuse=0 emi_cap=10*/
	{
	{0x06F46C80L, 0x1FDBB900L, 0x00031280L, 0x1FFD2C80L,    /* TXACEQ */
	 0x0AA3BE80L, 0x1A96E300L, 0x1FD90800L, 0x1FE59F00L},   /* RXACEQ */
	{0x0019AA80L, 0x1FD99F80L, 0x015EBF80L, 0x01578100L,    /* ECFIR/ECIIR */
	 0x01C7AA00L, 0x01386780L, 0x00908C80L, 0x00AB1180L,
	 0x00656D80L, 0x1F9ACD80L, 0x0DD7CA80L, 0x1A1BC180L},
	{0x1FFD3180L, 0x1FCAE180L, 0x0037CF00L, 0x0ED0D100L,    /* ZSYNTH */
	 0x192E7C80L, 0x87}, 
	 0x08000000L,   /* TXACGAIN */
	 0x0104BF80L,   /* RXACGAIN */
	 0x07BC2380L, 0x1843DD00L, 0x07784700L,    /* RXACHPF */
	#ifdef ENABLE_HIRES_GAIN
	 0, 0  /* TXGAIN*10, RXGAIN*10 (hi_res) */
	#else
	 0, 0  /* TXGAIN, RXGAIN */
	#endif
	 }     /* SLIC_IMPEDANCE_350_1000_210 */
 };

int main(int argc, char *argv[])
{
	if (argc == 4)
	{
		if (atoi(argv[1]) == 1)
		{
			rtk_SetRingFxs(atoi(argv[2]), atoi(argv[3]));
		}
		else if (atoi(argv[1]) == 2)
		{
			rtk_SetFxoTune(atoi(argv[2]), atoi(argv[3]));
		}
		else if (atoi(argv[1]) == 3)
		{
			rtkSetProslicParam(atoi(argv[2]), PROSLIC_TYPE_SI3226X, PROSLIC_PARAM_TYPE_RING, &Si3226x_Ring_profile[atoi(argv[3])], 132);
		}
		else if (atoi(argv[1]) == 4)
		{
			rtkSetProslicParam(atoi(argv[2]), PROSLIC_TYPE_SI3226X, PROSLIC_PARAM_TYPE_DCFEED, &Si3226x_DCfeed_profile[atoi(argv[3])], 96);
		}
		else if (atoi(argv[1]) == 5)
		{
			rtkSetProslicParam(atoi(argv[2]), PROSLIC_TYPE_SI3226X, PROSLIC_PARAM_TYPE_IMPEDANCE, &Si3226x_Impedance_profile[atoi(argv[3])], 128);
		}
	}
	else
	{
		printf("%s: cmd error!\n", argv[0]);
	}

	return 0;
}


#ifndef __CMU_H__
#define __CMU_H__

#define DISABLE_CMU_FIX_MODE    (0)
#define ENABLE_CMU_FIX_MODE     (1)
//#define ENABLE_CMU_DYNAMIC_MODE (2)

typedef union {
	struct {
		unsigned int busy:1;       // 1
        unsigned int mbz_30_7:24;  // 0
		unsigned int freq_div:3;   // 7
		unsigned int mbz_3_2:2;    // 0
		unsigned int cmu_mode:2;   // 0
	} f;
	unsigned int v;
} OC0_CMUGCR_T;
#define OC0_CMUGCRrv (*((regval)0xb8000380))
#define OC0_CMUGCRdv (0x80000070) 	/* Set all divisor to 1 and CMU disabled by default. */
#define RMOD_OC0_CMUGCR(...) rset(OC0_CMUGCR, OC0_CMUGCRrv, __VA_ARGS__)
#define RIZS_OC0_CMUGCR(...) rset(OC0_CMUGCR, 0, __VA_ARGS__)
#define RFLD_OC0_CMUGCR(fld) (*((const volatile OC0_CMUGCR_T *)0xb8000380)).f.fld

typedef union {
	struct {
		unsigned int mbz_31_3:29; // 0
		unsigned int dly_base:3;  // 0
	} f;
	unsigned int v;
} OC0_CMUSDCR_T;
#define OC0_CMUSDCRrv (*((regval)0xb8000384))
#define OC0_CMUSDCRdv (0x00000000)
#define RMOD_OC0_CMUSDCR(...) rset(OC0_CMUSDCR, OC0_CMUSDCRrv, __VA_ARGS__)
#define RIZS_OC0_CMUSDCR(...) rset(OC0_CMUSDCR, 0, __VA_ARGS__)
#define RFLD_OC0_CMUSDCR(fld) (*((const volatile OC0_CMUSDCR_T *)0xb8000384)).f.fld

typedef union {
	struct {
		unsigned int auto_bz:1;     //0
		unsigned int mbz_30_20:11;  //0
		unsigned int se_spif_wk:1;  //0
		unsigned int se_spif_slp:1; //0
		unsigned int se_spif:1;     //0
		unsigned int se_spif_hs:1;  //0
		unsigned int mbz_15_12:4;   //0
		unsigned int se_dram_wk:1;  //0
		unsigned int se_dram_slp:1; //0
		unsigned int se_dram:1;     //0
		unsigned int se_dram_hs:1;  //0
		unsigned int mbz_7_4:4;     //0
		unsigned int dly_mul:4;     //0
	} f;
	unsigned int v;
} OC0_CMUCR_T;
#define OC0_CMUCRrv (*((regval)0xb8000388))
#define OC0_CMUCRdv (0x00000000)
#define RMOD_OC0_CMUCR(...) rset(OC0_CMUCR, OC0_CMUCRrv, __VA_ARGS__)
#define RIZS_OC0_CMUCR(...) rset(OC0_CMUCR, 0, __VA_ARGS__)
#define RFLD_OC0_CMUCR(fld) (*((const volatile OC0_CMUCR_T *)0xb8000388)).f.fld


typedef union {
	struct {
		unsigned int bus_slp_cnt:32;     //0
	} f;
	unsigned int v;
} OC0_CMUSCR_T;
#define OC0_CMUSCRrv (*((regval)0xb800038C))
#define OC0_CMUSCRdv (0x00000000)
#define RMOD_OC0_CMUSCR(...) rset(OC0_CMUSCR, OC0_CMUSCRrv, __VA_ARGS__)
#define RIZS_OC0_CMUSCR(...) rset(OC0_CMUSCR, 0, __VA_ARGS__)
#define RFLD_OC0_CMUSCR(fld) (*((const volatile OC0_CMUSCR_T *)0xb800038C)).f.fld


//=================================================

typedef union {
	struct {
		unsigned int busy:1;       // 1
        unsigned int mbz_30_7:24;  // 0
		unsigned int freq_div:3;   // 7
		unsigned int mbz_3_2:2;    // 0
		unsigned int cmu_mode:2;   // 0
	} f;
	unsigned int v;
} LX0_CMUGCR_T;
#define LX0_CMUGCRrv (*((regval)0xb80003A0))
#define LX0_CMUGCRdv (0x80000070) 	/* Set all divisor to 1 and CMU disabled by default. */
#define RMOD_LX0_CMUGCR(...) rset(LX0_CMUGCR, LX0_CMUGCRrv, __VA_ARGS__)
#define RIZS_LX0_CMUGCR(...) rset(LX0_CMUGCR, 0, __VA_ARGS__)
#define RFLD_LX0_CMUGCR(fld) (*((const volatile LX0_CMUGCR_T *)0xb80003A0)).f.fld

typedef union {
	struct {
		unsigned int mbz_31_3:29; // 0
		unsigned int dly_base:3;  // 0
	} f;
	unsigned int v;
} LX0_CMUSDCR_T;
#define LX0_CMUSDCRrv (*((regval)0xb80003A4))
#define LX0_CMUSDCRdv (0x00000000)
#define RMOD_LX0_CMUSDCR(...) rset(LX0_CMUSDCR, LX0_CMUSDCRrv, __VA_ARGS__)
#define RIZS_LX0_CMUSDCR(...) rset(LX0_CMUSDCR, 0, __VA_ARGS__)
#define RFLD_LX0_CMUSDCR(fld) (*((const volatile LX0_CMUSDCR_T *)0xb80003A4)).f.fld

typedef union {
	struct {
		unsigned int auto_bz:1;     //0
		unsigned int mbz_30_10:21;  //0
		unsigned int se_dram:1;     //0
		unsigned int mbz_8_4:5;     //0
		unsigned int dly_mul:4;     //0
	} f;
	unsigned int v;
} LX0_CMUCR_T;
#define LX0_CMUCRrv (*((regval)0xb80003A8))
#define LX0_CMUCRdv (0x00000000)
#define RMOD_LX0_CMUCR(...) rset(LX0_CMUCR, LX0_CMUCRrv, __VA_ARGS__)
#define RIZS_LX0_CMUCR(...) rset(LX0_CMUCR, 0, __VA_ARGS__)
#define RFLD_LX0_CMUCR(fld) (*((const volatile LX0_CMUCR_T *)0xb80003A8)).f.fld

typedef union {
	struct {
		unsigned int bus_slp_cnt:32;     //0
	} f;
	unsigned int v;
} LX0_CMUSCR_T;
#define LX0_CMUSCRrv (*((regval)0xb80003AC))
#define LX0_CMUSCRdv (0x00000000)
#define RMOD_LX0_CMUSCR(...) rset(LX0_CMUSCR, LX0_CMUSCRrv, __VA_ARGS__)
#define RIZS_LX0_CMUSCR(...) rset(LX0_CMUSCR, 0, __VA_ARGS__)
#define RFLD_LX0_CMUSCR(fld) (*((const volatile LX0_CMUSCR_T *)0xb80003AC)).f.fld


//=================================================
typedef union {
	struct {
		unsigned int busy:1;       // 1
        unsigned int mbz_30_7:24;  // 0
		unsigned int freq_div:3;   // 7
		unsigned int mbz_3_2:2;    // 0
		unsigned int cmu_mode:2;   // 0
	} f;
	unsigned int v;
} LX1_CMUGCR_T;
#define LX1_CMUGCRrv (*((regval)0xb80003b0))
#define LX1_CMUGCRdv (0x80000070) 	/* Set all divisor to 1 and CMU disabled by default. */
#define RMOD_LX1_CMUGCR(...) rset(LX1_CMUGCR, LX1_CMUGCRrv, __VA_ARGS__)
#define RIZS_LX1_CMUGCR(...) rset(LX1_CMUGCR, 0, __VA_ARGS__)
#define RFLD_LX1_CMUGCR(fld) (*((const volatile LX1_CMUGCR_T *)0xb80003b0)).f.fld

typedef union {
	struct {
		unsigned int mbz_31_3:29; // 0
		unsigned int dly_base:3;  // 0
	} f;
	unsigned int v;
} LX1_CMUSDCR_T;
#define LX1_CMUSDCRrv (*((regval)0xb80003b4))
#define LX1_CMUSDCRdv (0x00000000)
#define RMOD_LX1_CMUSDCR(...) rset(LX1_CMUSDCR, LX1_CMUSDCRrv, __VA_ARGS__)
#define RIZS_LX1_CMUSDCR(...) rset(LX1_CMUSDCR, 0, __VA_ARGS__)
#define RFLD_LX1_CMUSDCR(fld) (*((const volatile LX1_CMUSDCR_T *)0xb80003b4)).f.fld

typedef union {
	struct {
		unsigned int auto_bz:1;     //0
		unsigned int mbz_30_10:21;  //0
		unsigned int se_dram:1;     //0
		unsigned int mbz_8_4:5;     //0
		unsigned int dly_mul:4;     //0
	} f;
	unsigned int v;
} LX1_CMUCR_T;
#define LX1_CMUCRrv (*((regval)0xb80003b8))
#define LX1_CMUCRdv (0x00000000)
#define RMOD_LX1_CMUCR(...) rset(LX1_CMUCR, LX1_CMUCRrv, __VA_ARGS__)
#define RIZS_LX1_CMUCR(...) rset(LX1_CMUCR, 0, __VA_ARGS__)
#define RFLD_LX1_CMUCR(fld) (*((const volatile LX1_CMUCR_T *)0xb80003b8)).f.fld

typedef union {
	struct {
		unsigned int bus_slp_cnt:32;     //0
	} f;
	unsigned int v;
} LX1_CMUSCR_T;
#define LX1_CMUSCRrv (*((regval)0xb80003bC))
#define LX1_CMUSCRdv (0x00000000)
#define RMOD_LX1_CMUSCR(...) rset(LX1_CMUSCR, LX1_CMUSCRrv, __VA_ARGS__)
#define RIZS_LX1_CMUSCR(...) rset(LX1_CMUSCR, 0, __VA_ARGS__)
#define RFLD_LX1_CMUSCR(fld) (*((const volatile LX1_CMUSCR_T *)0xb80003bC)).f.fld


//=================================================
typedef union {
	struct {
		unsigned int busy:1;       // 1
        unsigned int mbz_30_7:24;  // 0
		unsigned int freq_div:3;   // 7
		unsigned int mbz_3_2:2;    // 0
		unsigned int cmu_mode:2;   // 0
	} f;
	unsigned int v;
} LX2_CMUGCR_T;
#define LX2_CMUGCRrv (*((regval)0xb80003c0))
#define LX2_CMUGCRdv (0x80000070) 	/* Set all divisor to 1 and CMU disabled by default. */
#define RMOD_LX2_CMUGCR(...) rset(LX2_CMUGCR, LX2_CMUGCRrv, __VA_ARGS__)
#define RIZS_LX2_CMUGCR(...) rset(LX2_CMUGCR, 0, __VA_ARGS__)
#define RFLD_LX2_CMUGCR(fld) (*((const volatile LX2_CMUGCR_T *)0xb80003c0)).f.fld

typedef union {
	struct {
		unsigned int mbz_31_3:29; // 0
		unsigned int dly_base:3;  // 0
	} f;
	unsigned int v;
} LX2_CMUSDCR_T;
#define LX2_CMUSDCRrv (*((regval)0xb80003c4))
#define LX2_CMUSDCRdv (0x00000000)
#define RMOD_LX2_CMUSDCR(...) rset(LX2_CMUSDCR, LX2_CMUSDCRrv, __VA_ARGS__)
#define RIZS_LX2_CMUSDCR(...) rset(LX2_CMUSDCR, 0, __VA_ARGS__)
#define RFLD_LX2_CMUSDCR(fld) (*((const volatile LX2_CMUSDCR_T *)0xb80003c4)).f.fld

typedef union {
	struct {
		unsigned int auto_bz:1;     //0
		unsigned int mbz_30_10:21;  //0
		unsigned int se_dram:1;     //0
		unsigned int mbz_8_4:5;     //0
		unsigned int dly_mul:4;     //0
	} f;
	unsigned int v;
} LX2_CMUCR_T;
#define LX2_CMUCRrv (*((regval)0xb80003c8))
#define LX2_CMUCRdv (0x00000000)
#define RMOD_LX2_CMUCR(...) rset(LX2_CMUCR, LX2_CMUCRrv, __VA_ARGS__)
#define RIZS_LX2_CMUCR(...) rset(LX2_CMUCR, 0, __VA_ARGS__)
#define RFLD_LX2_CMUCR(fld) (*((const volatile LX2_CMUCR_T *)0xb80003c8)).f.fld

typedef union {
	struct {
		unsigned int bus_slp_cnt:32;     //0
	} f;
	unsigned int v;
} LX2_CMUSCR_T;
#define LX2_CMUSCRrv (*((regval)0xb80003CC))
#define LX2_CMUSCRdv (0x00000000)
#define RMOD_LX2_CMUSCR(...) rset(LX2_CMUSCR, LX2_CMUSCRrv, __VA_ARGS__)
#define RIZS_LX2_CMUSCR(...) rset(LX2_CMUSCR, 0, __VA_ARGS__)
#define RFLD_LX2_CMUSCR(fld) (*((const volatile LX2_CMUSCR_T *)0xb80003CC)).f.fld


//=================================================
typedef union {
	struct {
		unsigned int busy:1;       // 1
        unsigned int mbz_30_7:24;  // 0
		unsigned int freq_div:3;   // 7
		unsigned int mbz_3_2:2;    // 0
		unsigned int cmu_mode:2;   // 0
	} f;
	unsigned int v;
} LXP_CMUGCR_T;
#define LXP_CMUGCRrv (*((regval)0xb80003D0))
#define LXP_CMUGCRdv (0x80000070) 	/* Set all divisor to 1 and CMU disabled by default. */
#define RMOD_LXP_CMUGCR(...) rset(LXP_CMUGCR, LXP_CMUGCRrv, __VA_ARGS__)
#define RIZS_LXP_CMUGCR(...) rset(LXP_CMUGCR, 0, __VA_ARGS__)
#define RFLD_LXP_CMUGCR(fld) (*((const volatile LXP_CMUGCR_T *)0xb80003D0)).f.fld

typedef union {
	struct {
		unsigned int mbz_31_3:29; // 0
		unsigned int dly_base:3;  // 0
	} f;
	unsigned int v;
} LXP_CMUSDCR_T;
#define LXP_CMUSDCRrv (*((regval)0xb80003D4))
#define LXP_CMUSDCRdv (0x00000000)
#define RMOD_LXP_CMUSDCR(...) rset(LXP_CMUSDCR, LXP_CMUSDCRrv, __VA_ARGS__)
#define RIZS_LXP_CMUSDCR(...) rset(LXP_CMUSDCR, 0, __VA_ARGS__)
#define RFLD_LXP_CMUSDCR(fld) (*((const volatile LXP_CMUSDCR_T *)0xb80003D4)).f.fld

typedef union {
	struct {
		unsigned int auto_bz:1;     //0
		unsigned int mbz_30_10:21;  //0
		unsigned int se_dram:1;     //0
		unsigned int mbz_8_4:5;     //0
		unsigned int dly_mul:4;     //0
	} f;
	unsigned int v;
} LXP_CMUCR_T;
#define LXP_CMUCRrv (*((regval)0xb80003D8))
#define LXP_CMUCRdv (0x00000000)
#define RMOD_LXP_CMUCR(...) rset(LXP_CMUCR, LXP_CMUCRrv, __VA_ARGS__)
#define RIZS_LXP_CMUCR(...) rset(LXP_CMUCR, 0, __VA_ARGS__)
#define RFLD_LXP_CMUCR(fld) (*((const volatile LXP_CMUCR_T *)0xb80003D8)).f.fld

typedef union {
	struct {
		unsigned int bus_slp_cnt:32;     //0
	} f;
	unsigned int v;
} LXP_CMUSCR_T;
#define LXP_CMUSCRrv (*((regval)0xb80003DC))
#define LXP_CMUSCRdv (0x00000000)
#define RMOD_LXP_CMUSCR(...) rset(LXP_CMUSCR, LXP_CMUSCRrv, __VA_ARGS__)
#define RIZS_LXP_CMUSCR(...) rset(LXP_CMUSCR, 0, __VA_ARGS__)
#define RFLD_LXP_CMUSCR(fld) (*((const volatile LXP_CMUSCR_T *)0xb80003DC)).f.fld

#endif //__CMU_H__

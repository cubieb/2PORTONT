#ifndef _REGISTER_MAP_PAD_CTRL_H_
#define _REGISTER_MAP_PAD_CTRL_H_

#include <reg_map_util.h>

/*-----------------------------------------------------
 Extraced from file_PAD_CTRL.xml
-----------------------------------------------------*/

typedef union {
	struct {
		unsigned int mbz_0:12; //0
		unsigned int dram_type:2; //3
		unsigned int boot_up_type:2; //0
		unsigned int no_use15:1; //0
		unsigned int mbz_1:3; //0
		unsigned int no_use11:4; //0
		unsigned int mbz_2:1; //0
		unsigned int spi_flash_4b_en:1; //0
		unsigned int mbz_3:1; //0
		unsigned int scan_mode:1; //0
		unsigned int speed_up:1; //0
		unsigned int mbz_4:3; //0
	} f;
	unsigned int v;
} PIN_STS_T;
#define PIN_STSrv (*((regval)0xb8000100))
#define PIN_STSdv (0x000c0000)
#define RMOD_PIN_STS(...) rset(PIN_STS, PIN_STSrv, __VA_ARGS__)
#define RIZS_PIN_STS(...) rset(PIN_STS, 0, __VA_ARGS__)
#define RFLD_PIN_STS(fld) (*((const volatile PIN_STS_T *)0xb8000100)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:22; //0
		unsigned int mbz_1:4; //0
		unsigned int no_use05:6; //0
	} f;
	unsigned int v;
} PAD_CTRL_NO_USE_0104_T;
#define PAD_CTRL_NO_USE_0104rv (*((regval)0xb8000104))
#define PAD_CTRL_NO_USE_0104dv (0x00000000)
#define RMOD_PAD_CTRL_NO_USE_0104(...) rset(PAD_CTRL_NO_USE_0104, PAD_CTRL_NO_USE_0104rv, __VA_ARGS__)
#define RIZS_PAD_CTRL_NO_USE_0104(...) rset(PAD_CTRL_NO_USE_0104, 0, __VA_ARGS__)
#define RFLD_PAD_CTRL_NO_USE_0104(fld) (*((const volatile PAD_CTRL_NO_USE_0104_T *)0xb8000104)).f.fld

typedef union {
	struct {
		unsigned int no_use31:31; //0
		unsigned int mbz_0:1; //0
	} f;
	unsigned int v;
} PAD_CTRL_NO_USE_0108_T;
#define PAD_CTRL_NO_USE_0108rv (*((regval)0xb8000108))
#define PAD_CTRL_NO_USE_0108dv (0x00000000)
#define RMOD_PAD_CTRL_NO_USE_0108(...) rset(PAD_CTRL_NO_USE_0108, PAD_CTRL_NO_USE_0108rv, __VA_ARGS__)
#define RIZS_PAD_CTRL_NO_USE_0108(...) rset(PAD_CTRL_NO_USE_0108, 0, __VA_ARGS__)
#define RFLD_PAD_CTRL_NO_USE_0108(fld) (*((const volatile PAD_CTRL_NO_USE_0108_T *)0xb8000108)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:2; //0
		unsigned int no_use29:30; //0
	} f;
	unsigned int v;
} PAD_CTRL_NO_USE_010C_T;
#define PAD_CTRL_NO_USE_010Crv (*((regval)0xb800010c))
#define PAD_CTRL_NO_USE_010Cdv (0x00000000)
#define RMOD_PAD_CTRL_NO_USE_010C(...) rset(PAD_CTRL_NO_USE_010C, PAD_CTRL_NO_USE_010Crv, __VA_ARGS__)
#define RIZS_PAD_CTRL_NO_USE_010C(...) rset(PAD_CTRL_NO_USE_010C, 0, __VA_ARGS__)
#define RFLD_PAD_CTRL_NO_USE_010C(fld) (*((const volatile PAD_CTRL_NO_USE_010C_T *)0xb800010c)).f.fld

typedef union {
	struct {
		unsigned int no_use31:5; //0
		unsigned int mbz_0:27; //0
	} f;
	unsigned int v;
} PAD_CTRL_NO_USE_0110_T;
#define PAD_CTRL_NO_USE_0110rv (*((regval)0xb8000110))
#define PAD_CTRL_NO_USE_0110dv (0x00000000)
#define RMOD_PAD_CTRL_NO_USE_0110(...) rset(PAD_CTRL_NO_USE_0110, PAD_CTRL_NO_USE_0110rv, __VA_ARGS__)
#define RIZS_PAD_CTRL_NO_USE_0110(...) rset(PAD_CTRL_NO_USE_0110, 0, __VA_ARGS__)
#define RFLD_PAD_CTRL_NO_USE_0110(fld) (*((const volatile PAD_CTRL_NO_USE_0110_T *)0xb8000110)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:1; //0
		unsigned int no_use30:15; //0
		unsigned int mbz_1:3; //0
		unsigned int no_use12:13; //0
	} f;
	unsigned int v;
} PAD_CTRL_NO_USE_0114_T;
#define PAD_CTRL_NO_USE_0114rv (*((regval)0xb8000114))
#define PAD_CTRL_NO_USE_0114dv (0x00000000)
#define RMOD_PAD_CTRL_NO_USE_0114(...) rset(PAD_CTRL_NO_USE_0114, PAD_CTRL_NO_USE_0114rv, __VA_ARGS__)
#define RIZS_PAD_CTRL_NO_USE_0114(...) rset(PAD_CTRL_NO_USE_0114, 0, __VA_ARGS__)
#define RFLD_PAD_CTRL_NO_USE_0114(fld) (*((const volatile PAD_CTRL_NO_USE_0114_T *)0xb8000114)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_MD_IOPDCR0_T;
#define DDR_MD_IOPDCR0rv (*((regval)0xb8000118))
#define DDR_MD_IOPDCR0dv (0x00000000)
#define RMOD_DDR_MD_IOPDCR0(...) rset(DDR_MD_IOPDCR0, DDR_MD_IOPDCR0rv, __VA_ARGS__)
#define RIZS_DDR_MD_IOPDCR0(...) rset(DDR_MD_IOPDCR0, 0, __VA_ARGS__)
#define RFLD_DDR_MD_IOPDCR0(fld) (*((const volatile DDR_MD_IOPDCR0_T *)0xb8000118)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_MD_IOPDCR1_T;
#define DDR_MD_IOPDCR1rv (*((regval)0xb800011c))
#define DDR_MD_IOPDCR1dv (0x00000000)
#define RMOD_DDR_MD_IOPDCR1(...) rset(DDR_MD_IOPDCR1, DDR_MD_IOPDCR1rv, __VA_ARGS__)
#define RIZS_DDR_MD_IOPDCR1(...) rset(DDR_MD_IOPDCR1, 0, __VA_ARGS__)
#define RFLD_DDR_MD_IOPDCR1(fld) (*((const volatile DDR_MD_IOPDCR1_T *)0xb800011c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_MD_IOPDCR2_T;
#define DDR_MD_IOPDCR2rv (*((regval)0xb8000120))
#define DDR_MD_IOPDCR2dv (0x00000000)
#define RMOD_DDR_MD_IOPDCR2(...) rset(DDR_MD_IOPDCR2, DDR_MD_IOPDCR2rv, __VA_ARGS__)
#define RIZS_DDR_MD_IOPDCR2(...) rset(DDR_MD_IOPDCR2, 0, __VA_ARGS__)
#define RFLD_DDR_MD_IOPDCR2(fld) (*((const volatile DDR_MD_IOPDCR2_T *)0xb8000120)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_MD_IOPDCR3_T;
#define DDR_MD_IOPDCR3rv (*((regval)0xb8000124))
#define DDR_MD_IOPDCR3dv (0x00000000)
#define RMOD_DDR_MD_IOPDCR3(...) rset(DDR_MD_IOPDCR3, DDR_MD_IOPDCR3rv, __VA_ARGS__)
#define RIZS_DDR_MD_IOPDCR3(...) rset(DDR_MD_IOPDCR3, 0, __VA_ARGS__)
#define RFLD_DDR_MD_IOPDCR3(fld) (*((const volatile DDR_MD_IOPDCR3_T *)0xb8000124)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int hdqm_drv:1; //0
		unsigned int hdqm_nt:3; //0
		unsigned int hdqm_pt:3; //0
		unsigned int hdqm_plsb:2; //0
		unsigned int hdqm_force_mode:1; //0
		unsigned int mbz_1:6; //0
		unsigned int ldqm_drv:1; //0
		unsigned int ldqm_nt:3; //0
		unsigned int ldqm_pt:3; //0
		unsigned int ldqm_plsb:2; //0
		unsigned int ldqm_force_mode:1; //0
	} f;
	unsigned int v;
} DDR_DM_IOPDCR_T;
#define DDR_DM_IOPDCRrv (*((regval)0xb8000128))
#define DDR_DM_IOPDCRdv (0x00000000)
#define RMOD_DDR_DM_IOPDCR(...) rset(DDR_DM_IOPDCR, DDR_DM_IOPDCRrv, __VA_ARGS__)
#define RIZS_DDR_DM_IOPDCR(...) rset(DDR_DM_IOPDCR, 0, __VA_ARGS__)
#define RFLD_DDR_DM_IOPDCR(fld) (*((const volatile DDR_DM_IOPDCR_T *)0xb8000128)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int ckn_drv:1; //0
		unsigned int ckn_nt:3; //0
		unsigned int ckn_pt:3; //0
		unsigned int ckn_plsb:2; //0
		unsigned int ckn_force_mode:1; //0
		unsigned int mbz_1:6; //0
		unsigned int ck_drv:1; //0
		unsigned int ck_nt:3; //0
		unsigned int ck_pt:3; //0
		unsigned int ck_plsb:2; //0
		unsigned int ck_force_mode:1; //0
	} f;
	unsigned int v;
} DDR_CK_IOPDCR_T;
#define DDR_CK_IOPDCRrv (*((regval)0xb800012c))
#define DDR_CK_IOPDCRdv (0x00000000)
#define RMOD_DDR_CK_IOPDCR(...) rset(DDR_CK_IOPDCR, DDR_CK_IOPDCRrv, __VA_ARGS__)
#define RIZS_DDR_CK_IOPDCR(...) rset(DDR_CK_IOPDCR, 0, __VA_ARGS__)
#define RFLD_DDR_CK_IOPDCR(fld) (*((const volatile DDR_CK_IOPDCR_T *)0xb800012c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_DQS_IOPDCR0_T;
#define DDR_DQS_IOPDCR0rv (*((regval)0xb8000130))
#define DDR_DQS_IOPDCR0dv (0x00000000)
#define RMOD_DDR_DQS_IOPDCR0(...) rset(DDR_DQS_IOPDCR0, DDR_DQS_IOPDCR0rv, __VA_ARGS__)
#define RIZS_DDR_DQS_IOPDCR0(...) rset(DDR_DQS_IOPDCR0, 0, __VA_ARGS__)
#define RFLD_DDR_DQS_IOPDCR0(fld) (*((const volatile DDR_DQS_IOPDCR0_T *)0xb8000130)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_DQS_IOPDCR1_T;
#define DDR_DQS_IOPDCR1rv (*((regval)0xb8000134))
#define DDR_DQS_IOPDCR1dv (0x00000000)
#define RMOD_DDR_DQS_IOPDCR1(...) rset(DDR_DQS_IOPDCR1, DDR_DQS_IOPDCR1rv, __VA_ARGS__)
#define RIZS_DDR_DQS_IOPDCR1(...) rset(DDR_DQS_IOPDCR1, 0, __VA_ARGS__)
#define RFLD_DDR_DQS_IOPDCR1(fld) (*((const volatile DDR_DQS_IOPDCR1_T *)0xb8000134)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_DQS_IOPDCR2_T;
#define DDR_DQS_IOPDCR2rv (*((regval)0xb8000138))
#define DDR_DQS_IOPDCR2dv (0x00000000)
#define RMOD_DDR_DQS_IOPDCR2(...) rset(DDR_DQS_IOPDCR2, DDR_DQS_IOPDCR2rv, __VA_ARGS__)
#define RIZS_DDR_DQS_IOPDCR2(...) rset(DDR_DQS_IOPDCR2, 0, __VA_ARGS__)
#define RFLD_DDR_DQS_IOPDCR2(fld) (*((const volatile DDR_DQS_IOPDCR2_T *)0xb8000138)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int dtn:2; //0
		unsigned int dtp:2; //0
		unsigned int force_mode:1; //0
		unsigned int drv:1; //0
		unsigned int nt:3; //0
		unsigned int pt:3; //0
		unsigned int plsb:2; //0
		unsigned int ttcn:3; //0
		unsigned int ttcp:3; //0
		unsigned int ttfn:3; //0
		unsigned int ttfp:3; //0
	} f;
	unsigned int v;
} DDR_DQS_IOPDCR3_T;
#define DDR_DQS_IOPDCR3rv (*((regval)0xb800013c))
#define DDR_DQS_IOPDCR3dv (0x00000000)
#define RMOD_DDR_DQS_IOPDCR3(...) rset(DDR_DQS_IOPDCR3, DDR_DQS_IOPDCR3rv, __VA_ARGS__)
#define RIZS_DDR_DQS_IOPDCR3(...) rset(DDR_DQS_IOPDCR3, 0, __VA_ARGS__)
#define RFLD_DDR_DQS_IOPDCR3(fld) (*((const volatile DDR_DQS_IOPDCR3_T *)0xb800013c)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:6; //0
		unsigned int misc_drv:1; //0
		unsigned int misc_nt:3; //0
		unsigned int misc_pt:3; //0
		unsigned int misc_plsb:2; //0
		unsigned int misc_force_mode:1; //0
		unsigned int mbz_1:6; //0
		unsigned int mba_drv:1; //0
		unsigned int mba_nt:3; //0
		unsigned int mba_pt:3; //0
		unsigned int mba_plsb:2; //0
		unsigned int mba_force_mode:1; //0
	} f;
	unsigned int v;
} DDR_MA_IOPDCR_T;
#define DDR_MA_IOPDCRrv (*((regval)0xb8000140))
#define DDR_MA_IOPDCRdv (0x00000000)
#define RMOD_DDR_MA_IOPDCR(...) rset(DDR_MA_IOPDCR, DDR_MA_IOPDCRrv, __VA_ARGS__)
#define RIZS_DDR_MA_IOPDCR(...) rset(DDR_MA_IOPDCR, 0, __VA_ARGS__)
#define RFLD_DDR_MA_IOPDCR(fld) (*((const volatile DDR_MA_IOPDCR_T *)0xb8000140)).f.fld

typedef union {
	struct {
		unsigned int ddr3_md:1; //0
		unsigned int ddr_cal_srst:1; //1
		unsigned int ddr_zq_pwrdn:1; //0
		unsigned int force_mode:1; //0
		unsigned int plsb:1; //0
		unsigned int vpcalen:1; //0
		unsigned int podt:3; //0
		unsigned int pocd:5; //0
		unsigned int nodt:3; //0
		unsigned int nocd:5; //0
		unsigned int cal:2; //0
		unsigned int mbz_0:3; //0
		unsigned int ddr2_dqs_se:1; //0
		unsigned int ddr_io_speed:1; //1
		unsigned int ddr_io_pwdpad3:1; //0
		unsigned int ddr3_mode:1; //0
		unsigned int ddr2_mode:1; //1
	} f;
	unsigned int v;
} DDR123_MCR_T;
#define DDR123_MCRrv (*((regval)0xb8000144))
#define DDR123_MCRdv (0x40000009)
#define RMOD_DDR123_MCR(...) rset(DDR123_MCR, DDR123_MCRrv, __VA_ARGS__)
#define RIZS_DDR123_MCR(...) rset(DDR123_MCR, 0, __VA_ARGS__)
#define RFLD_DDR123_MCR(fld) (*((const volatile DDR123_MCR_T *)0xb8000144)).f.fld

#endif

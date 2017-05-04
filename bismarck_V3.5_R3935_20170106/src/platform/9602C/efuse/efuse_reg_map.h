#ifndef __EFUSE_REG_MAP_H__
#define __EFUSE_REG_MAP_H__


typedef union {
	struct {
		unsigned int no_use31_16:16; //0
		unsigned int wd_dat:16;      //0
	} f;
	unsigned int v;
} GPHY_IND_WD_T;
#define GPHY_IND_WDrv (*((regval)0xbb000000))
#define RMOD_GPHY_IND_WD(...) rset(GPHY_IND_WD, GPHY_IND_WDrv, __VA_ARGS__)
#define RFLD_GPHY_IND_WD(fld) (*((const volatile GPHY_IND_WD_T *)0xbb000000)).f.fld



typedef union {
	struct {
		unsigned int no_use31_16:9; //0
		unsigned int wr_en:1;       //0
		unsigned int cmd_en:1;      //0
		unsigned int addr:21;       //0
	} f;
	unsigned int v;
} GPHY_IND_CMD_T;
#define GPHY_IND_CMDrv (*((regval)0xbb000004))
#define RMOD_GPHY_IND_CMD(...) rset(GPHY_IND_CMD, GPHY_IND_CMDrv, __VA_ARGS__)
#define RFLD_GPHY_IND_CMD(fld) (*((const volatile GPHY_IND_CMD_T *)0xbb000004)).f.fld


typedef union {
	struct {
		unsigned int no_use31_17:15; //0
		unsigned int busy:1;         //0
		unsigned int rd_dat:16;      //0
	} f;
	unsigned int v;
} GPHY_IND_RD_T;
#define GPHY_IND_RDrv (*((regval)0xbb000008))
#define RMOD_GPHY_IND_RD(...) rset(GPHY_IND_RD, GPHY_IND_RDrv, __VA_ARGS__)
#define RFLD_GPHY_IND_RD(fld) (*((const volatile GPHY_IND_RD_T *)0xbb000008)).f.fld




typedef union {
	struct {
		unsigned int no_use31_18:14; //0
		unsigned int wren:1; //0
		unsigned int cmd_en:1; //0
		unsigned int addr:16; //0
	} f;
	unsigned int v;
} EFUSE_IND_CMD_T;
#define EFUSE_IND_CMDrv (*((regval)0xbb00001C))
#define RMOD_EFUSE_IND_CMD(...) rset(EFUSE_IND_CMD, EFUSE_IND_CMDrv, __VA_ARGS__)
#define RFLD_EFUSE_IND_CMD(fld) (*((const volatile EFUSE_IND_CMD_T *)0xbb00001C)).f.fld


typedef union {
	struct {
		unsigned int no_use31_17:15; //0
		unsigned int busy:1; //0
		unsigned int rd_dat_15_12:4;
		unsigned int rd_dat_11_8:4;
		unsigned int rd_dat_7_4:4;
		unsigned int rd_dat_3_0:4;
	} f;
	unsigned int v;
} EFUSE_IND_RD_T;
#define EFUSE_IND_RDrv (*((regval)0xbb000020))
#define RMOD_EFUSE_IND_RD(...) rset(EFUSE_IND_RD, EFUSE_IND_RDrv, __VA_ARGS__)
#define RFLD_EFUSE_IND_RD(fld) (*((const volatile EFUSE_IND_RD_T *)0xbb000020)).f.fld

#if 0
typedef union {
	struct {
		unsigned int no_use31_8:24; //0
		unsigned int wr_dat:8; //0
	} f;
	unsigned int v;
} SC_IND_WD_T;
#define SC_IND_WDrv (*((regval)0xbb00003C))
#define RMOD_SC_IND_WD(...) rset(SC_IND_WD, SC_IND_WDrv, __VA_ARGS__)
#define RFLD_SC_IND_WD(fld) (*((const volatile SC_IND_WD_T *)0xbb00003C)).f.fld


typedef union {
	struct {
		unsigned int no_use31_18:14; //0
		unsigned int wr_en:1; //0
		unsigned int cmd_en:1; //0
		unsigned int addr:16; //0
	} f;
	unsigned int v;
} SC_IND_CMD_T;
#define SC_IND_CMDrv (*((regval)0xbb000040))
#define RMOD_SC_IND_CMD(...) rset(SC_IND_CMD, SC_IND_CMDrv, __VA_ARGS__)
#define RFLD_SC_IND_CMD(fld) (*((const volatile SC_IND_CMD_T *)0xbb000040)).f.fld
#endif

typedef union {
	struct {
		unsigned int no_use31_29:3; //0
		unsigned int anarg_rdy_swr:1; //0
		unsigned int anarg_valid_swr:1; //0
		unsigned int fpwm_swr:1; //0
		unsigned int ref_saw_swr:4; //0
		unsigned int ref_saw_valid_swr:1; //0
		unsigned int miida_swr:5; //0
		unsigned int wrbus_swr:16; //0
	} f;
	unsigned int v;
} SWR_CTRL_0_T;
#define SWR_CTRL_0rv (*((regval)0xbb000144))
#define RMOD_SWR_CTRL_0_CMD(...) rset(SWR_CTRL_0, SWR_CTRL_0rv, __VA_ARGS__)


typedef union {
	struct {
		unsigned int anawrcode_swr:16; //0
		unsigned int no_use15_1:15; //0
		unsigned int anawrps_swr:1; //0
	} f;
	unsigned int v;
} SWR_CTRL_1_T;
#define SWR_CTRL_1rv (*((regval)0xbb000148))


typedef union {
	struct {
		unsigned int no_use31_29:3; //0
		unsigned int anarg_rdy_ddrswr:1; //0
		unsigned int anarg_valid_ddrswr:1; //0
		unsigned int fpwm_ddrswr:1; //0
		unsigned int ref_saw_ddrswr:4; //0
		unsigned int ref_saw_valid_ddrswr:1; //0
		unsigned int miida_ddrswr:5; //0
		unsigned int wrbus_ddrswr:16; //0
	} f;
	unsigned int v;
} DDRSWR_CTRL_0_T;
#define DDRSWR_CTRL_0rv (*((regval)0xbb000154))


typedef union {
	struct {
		unsigned int anawrcode_ddrswr:16; //0
		unsigned int no_use15_1:15; //0
		unsigned int anawrps_ddrswr:1; //0
	} f;
	unsigned int v;
} DDRSWR_CTRL_1_T;
#define DDRSWR_CTRL_1rv (*((regval)0xbb000158))

#endif //__EFUSE_REG_MAP_H__


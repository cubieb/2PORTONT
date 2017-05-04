#ifndef ONFI_CTRL_H
#define ONFI_CTRL_H

#include <soc.h>
#include <reg_map_util.h>
#include <onfi/onfi_struct.h>


/***********************************************
  * Driver Section Definitions
  ***********************************************/
#ifndef SECTION_ONFI
    #define SECTION_ONFI
#endif


/*-----------------------------------------------------
 Extraced from file_PARL_NAND_CTRL.xml
-----------------------------------------------------*/
typedef union {
	struct {
		unsigned int nafc_rc:2; //0
		unsigned int nafc_ac:2; //0
		unsigned int nafc_nf:1; //0
		unsigned int debug_select:3; //0
		unsigned int wp_b:1; //0
		unsigned int mbz_0:23; //0
	} f;
	unsigned int v;
} NACFR_T;
#define NACFRrv (*((regval)0xb801a000))
#define NACFRdv (0x00000000)
#define RMOD_NACFR(...) rset(NACFR, NACFRrv, __VA_ARGS__)
#define RIZS_NACFR(...) rset(NACFR, 0, __VA_ARGS__)
#define RFLD_NACFR(fld) (*((const volatile NACFR_T *)0xb801a000)).f.fld

typedef union {
	struct {
		unsigned int ready:1; //1
		unsigned int ecc:1; //1
		unsigned int rbo:1; //0
		unsigned int wbo:1; //0
		unsigned int ie:1; //0
		unsigned int no_use_26_20:7; //0
		unsigned int ce_twp:4; //15
		unsigned int ce_twh:4; //15
		unsigned int ce_trr:4; //15
		unsigned int ce_th:4; //15
		unsigned int ce_ts:4; //15
	} f;
	unsigned int v;
} NACR_T;
#define NACRrv (*((regval)0xb801a004))
#define NACRdv (0xc00fffff)
#define RMOD_NACR(...) rset(NACR, NACRrv, __VA_ARGS__)
#define RIZS_NACR(...) rset(NACR, 0, __VA_ARGS__)
#define RFLD_NACR(fld) (*((const volatile NACR_T *)0xb801a004)).f.fld

typedef union {
	struct {
		unsigned int cecs1:1; //0
		unsigned int cecs0:1; //0
		unsigned int nouse_29:22; //0
		unsigned int ce_cmd:8; //0
	} f;
	unsigned int v;
} NACMR_T;
#define NACMRrv (*((regval)0xb801a008))
#define NACMRdv (0x00000000)
#define RMOD_NACMR(...) rset(NACMR, NACMRrv, __VA_ARGS__)
#define RIZS_NACMR(...) rset(NACMR, 0, __VA_ARGS__)
#define RFLD_NACMR(fld) (*((const volatile NACMR_T *)0xb801a008)).f.fld

typedef union {
	struct {
		unsigned int nouse_31:4; //0
		unsigned int en_next_addr_cyc:1; //0
		unsigned int ad2en:1; //0
		unsigned int ad1en:1; //0
		unsigned int ad0en:1; //0
		unsigned int ce_addr2:8; //0
		unsigned int ce_addr1:8; //0
		unsigned int ce_addr0:8; //0
	} f;
	unsigned int v;
} NAADR_T;
#define NAADRrv (*((regval)0xb801a00c))
#define NAADRdv (0x00000000)
#define RMOD_NAADR(...) rset(NAWDR, NAADRrv, __VA_ARGS__)
#define RIZS_NAADR(...) rset(NAWDR, 0, __VA_ARGS__)
#define RFLD_NAADR(fld) (*((const volatile NAADR_T *)0xb801a00c)).f.fld

typedef union {
	struct {
		unsigned int nouse_31:23; //0
		unsigned int tag_sel:2; //0
		unsigned int tag_dis:1; //1
		unsigned int decs1:1; //0
		unsigned int decs0:1; //0
		unsigned int dmare:1; //0
		unsigned int dmawe:1; //0
		unsigned int lbc_bsz:2; //3
	} f;
	unsigned int v;
} NADCRR_T;
#define NADCRRrv (*((regval)0xb801a010))
#define NADCRRdv (0x00000043)
#define RMOD_NADCRR(...) rset(NADCRR, NADCRRrv, __VA_ARGS__)
#define RIZS_NADCRR(...) rset(NADCRR, 0, __VA_ARGS__)
#define RFLD_NADCRR(fld) (*((const volatile NADCRR_T *)0xb801a010)).f.fld

typedef union {
	struct {
		unsigned int data3:8; //0
		unsigned int data2:8; //0
		unsigned int data1:8; //0
		unsigned int data0:8; //0
	} f;
	unsigned int v;
} NADR_T;
#define NADRrv (*((regval)0xb801a014))
#define NADRdv (0x00000000)
#define RMOD_NADR(...) rset(NADR, NADRrv, __VA_ARGS__)
#define RIZS_NADR(...) rset(NADR, 0, __VA_ARGS__)
#define RFLD_NADR(fld) (*((const volatile NADR_T *)0xb801a014)).f.fld

typedef union {
	struct {
		unsigned int addr:32; //0
	} f;
	unsigned int v;
} NADFSAR_T;
#define NADFSARrv (*((regval)0xb801a018))
#define NADFSARdv (0x00000000)
#define RMOD_NADFSAR(...) rset(NADFSAR, NADFSARrv, __VA_ARGS__)
#define RIZS_NADFSAR(...) rset(NADFSAR, 0, __VA_ARGS__)
#define RFLD_NADFSAR(fld) (*((const volatile NADFSAR_T *)0xb801a018)).f.fld

typedef union {
	struct {
		unsigned int must_be_zero:29; //0
		unsigned int addr:3; //0
	} f;
	unsigned int v;
} NADFSAR2_T;
#define NADFSAR2rv (*((regval)0xb801a01c))
#define NADFSAR2dv (0x00000000)
#define RMOD_NADFSAR2(...) rset(NADFSAR2, NADFSAR2rv, __VA_ARGS__)
#define RIZS_NADFSAR2(...) rset(NADFSAR2, 0, __VA_ARGS__)
#define RFLD_NADFSAR2(fld) (*((const volatile NADFSAR2_T *)0xb801a01c)).f.fld

typedef union {
	struct {
		unsigned int addr:32; //2680160256
	} f;
	unsigned int v;
} NADRSAR_T;
#define NADRSARrv (*((regval)0xb801a020))
#define NADRSARdv (0x9fc00000)
#define RMOD_NADRSAR(...) rset(NADRSAR, NADRSARrv, __VA_ARGS__)
#define RIZS_NADRSAR(...) rset(NADRSAR, 0, __VA_ARGS__)
#define RFLD_NADRSAR(fld) (*((const volatile NADRSAR_T *)0xb801a020)).f.fld

typedef union {
	struct {
		unsigned int addr:32; //0
	} f;
	unsigned int v;
} NADTSAR_T;
#define NADTSARrv (*((regval)0xb801a024))
#define NADTSARdv (0x00000000)
#define RMOD_NADTSAR(...) rset(NADTSAR, NADTSARrv, __VA_ARGS__)
#define RIZS_NADTSAR(...) rset(NADTSAR, 0, __VA_ARGS__)
#define RFLD_NADTSAR(fld) (*((const volatile NADTSAR_T *)0xb801a024)).f.fld

typedef union {
	struct {
		unsigned int mbz_0:23; //0
		unsigned int all_one:1; //0
		unsigned int necn:4; //0
		unsigned int nrer:1; //0
		unsigned int nwer:1; //0
		unsigned int ndrs:1; //0
		unsigned int ndws:1; //0
	} f;
	unsigned int v;
} NASR_T;
#define NASRrv (*((regval)0xb801a028))
#define NASRdv (0x00000000)
#define RMOD_NASR(...) rset(NASR, NASRrv, __VA_ARGS__)
#define RIZS_NASR(...) rset(NASR, 0, __VA_ARGS__)
#define RFLD_NASR(fld) (*((const volatile NASR_T *)0xb801a028)).f.fld

typedef union {
	struct {
		unsigned int napr1_1:32; //0
	} f;
	unsigned int v;
} NAPR1_1_T;
#define NAPR1_1rv (*((regval)0xb801a03c))
#define NAPR1_1dv (0x00000000)
#define RMOD_NAPR1_1(...) rset(NAPR1_1, NAPR1_1rv, __VA_ARGS__)
#define RIZS_NAPR1_1(...) rset(NAPR1_1, 0, __VA_ARGS__)
#define RFLD_NAPR1_1(fld) (*((const volatile NAPR1_1_T *)0xb801a03c)).f.fld

typedef union {
	struct {
		unsigned int napr1_2:32; //0
	} f;
	unsigned int v;
} NAPR1_2_T;
#define NAPR1_2rv (*((regval)0xb801a040))
#define NAPR1_2dv (0x00000000)
#define RMOD_NAPR1_2(...) rset(NAPR1_2, NAPR1_2rv, __VA_ARGS__)
#define RIZS_NAPR1_2(...) rset(NAPR1_2, 0, __VA_ARGS__)
#define RFLD_NAPR1_2(fld) (*((const volatile NAPR1_2_T *)0xb801a040)).f.fld

typedef union {
	struct {
		unsigned int napr1_3:32; //0
	} f;
	unsigned int v;
} NAPR1_3_T;
#define NAPR1_3rv (*((regval)0xb801a044))
#define NAPR1_3dv (0x00000000)
#define RMOD_NAPR1_3(...) rset(NAPR1_3, NAPR1_3rv, __VA_ARGS__)
#define RIZS_NAPR1_3(...) rset(NAPR1_3, 0, __VA_ARGS__)
#define RFLD_NAPR1_3(fld) (*((const volatile NAPR1_3_T *)0xb801a044)).f.fld

typedef union {
	struct {
		unsigned int napr2_1:32; //0
	} f;
	unsigned int v;
} NAPR2_1_T;
#define NAPR2_1rv (*((regval)0xb801a048))
#define NAPR2_1dv (0x00000000)
#define RMOD_NAPR2_1(...) rset(NAPR2_1, NAPR2_1rv, __VA_ARGS__)
#define RIZS_NAPR2_1(...) rset(NAPR2_1, 0, __VA_ARGS__)
#define RFLD_NAPR2_1(fld) (*((const volatile NAPR2_1_T *)0xb801a048)).f.fld

typedef union {
	struct {
		unsigned int napr2_2:32; //0
	} f;
	unsigned int v;
} NAPR2_2_T;
#define NAPR2_2rv (*((regval)0xb801a04c))
#define NAPR2_2dv (0x00000000)
#define RMOD_NAPR2_2(...) rset(NAPR2_2, NAPR2_2rv, __VA_ARGS__)
#define RIZS_NAPR2_2(...) rset(NAPR2_2, 0, __VA_ARGS__)
#define RFLD_NAPR2_2(fld) (*((const volatile NAPR2_2_T *)0xb801a04c)).f.fld

typedef union {
	struct {
		unsigned int napr2_3:32; //0
	} f;
	unsigned int v;
} NAPR2_3_T;
#define NAPR2_3rv (*((regval)0xb801a050))
#define NAPR2_3dv (0x00000000)
#define RMOD_NAPR2_3(...) rset(NAPR2_3, NAPR2_3rv, __VA_ARGS__)
#define RIZS_NAPR2_3(...) rset(NAPR2_3, 0, __VA_ARGS__)
#define RFLD_NAPR2_3(fld) (*((const volatile NAPR2_3_T *)0xb801a050)).f.fld

typedef union {
	struct {
		unsigned int napr3_1:32; //0
	} f;
	unsigned int v;
} NAPR3_1_T;
#define NAPR3_1rv (*((regval)0xb801a054))
#define NAPR3_1dv (0x00000000)
#define RMOD_NAPR3_1(...) rset(NAPR3_1, NAPR3_1rv, __VA_ARGS__)
#define RIZS_NAPR3_1(...) rset(NAPR3_1, 0, __VA_ARGS__)
#define RFLD_NAPR3_1(fld) (*((const volatile NAPR3_1_T *)0xb801a054)).f.fld

typedef union {
	struct {
		unsigned int napr3_2:32; //0
	} f;
	unsigned int v;
} NAPR3_2_T;
#define NAPR3_2rv (*((regval)0xb801a058))
#define NAPR3_2dv (0x00000000)
#define RMOD_NAPR3_2(...) rset(NAPR3_2, NAPR3_2rv, __VA_ARGS__)
#define RIZS_NAPR3_2(...) rset(NAPR3_2, 0, __VA_ARGS__)
#define RFLD_NAPR3_2(fld) (*((const volatile NAPR3_2_T *)0xb801a058)).f.fld

typedef union {
	struct {
		unsigned int napr3_3:32; //0
	} f;
	unsigned int v;
} NAPR3_3_T;
#define NAPR3_3rv (*((regval)0xb801a05c))
#define NAPR3_3dv (0x00000000)
#define RMOD_NAPR3_3(...) rset(NAPR3_3, NAPR3_3rv, __VA_ARGS__)
#define RIZS_NAPR3_3(...) rset(NAPR3_3, 0, __VA_ARGS__)
#define RFLD_NAPR3_3(fld) (*((const volatile NAPR3_3_T *)0xb801a05c)).f.fld

typedef union {
	struct {
		unsigned int napr4_1:32; //0
	} f;
	unsigned int v;
} NAPR4_1_T;
#define NAPR4_1rv (*((regval)0xb801a060))
#define NAPR4_1dv (0x00000000)
#define RMOD_NAPR4_1(...) rset(NAPR4_1, NAPR4_1rv, __VA_ARGS__)
#define RIZS_NAPR4_1(...) rset(NAPR4_1, 0, __VA_ARGS__)
#define RFLD_NAPR4_1(fld) (*((const volatile NAPR4_1_T *)0xb801a060)).f.fld

typedef union {
	struct {
		unsigned int napr4_2:32; //0
	} f;
	unsigned int v;
} NAPR4_2_T;
#define NAPR4_2rv (*((regval)0xb801a064))
#define NAPR4_2dv (0x00000000)
#define RMOD_NAPR4_2(...) rset(NAPR4_2, NAPR4_2rv, __VA_ARGS__)
#define RIZS_NAPR4_2(...) rset(NAPR4_2, 0, __VA_ARGS__)
#define RFLD_NAPR4_2(fld) (*((const volatile NAPR4_2_T *)0xb801a064)).f.fld

typedef union {
	struct {
		unsigned int napr4_3:32; //0
	} f;
	unsigned int v;
} NAPR4_3_T;
#define NAPR4_3rv (*((regval)0xb801a068))
#define NAPR4_3dv (0x00000000)
#define RMOD_NAPR4_3(...) rset(NAPR4_3, NAPR4_3rv, __VA_ARGS__)
#define RIZS_NAPR4_3(...) rset(NAPR4_3, 0, __VA_ARGS__)
#define RFLD_NAPR4_3(fld) (*((const volatile NAPR4_3_T *)0xb801a068)).f.fld





/************************************************************/
/****** Paralle NAND Controller Field Enable/Disable Description ******/
/************************************************************/
// Register(NASR)
#define NECN       (1<<4)
#define NRER       (1<<3)
#define NWER       (1<<2)
#define NDRS       (1<<1)
#define NDWS       (1<<0)

// Register(NACMR)
#define CECS1      (1<<31)
#define CECS0      (1<<30)

// Register(NAADR)
#define EN_NEXT_AD (1<<27)
#define AD2EN      (1<<26)
#define AD1EN      (1<<25)
#define AD0EN      (1<<24)
#define CE_ADDR2   16
#define CE_ADDR1   8
#define CE_ADDR0   0


#define WAIT_ONFI_CTRL_READY() while(!RFLD_NACR(ready))
#define DEACTIVATE_CS0_CS1()   NACMRrv=0

//Controller relative part
int onaf_ctrl_init(void);
s32_t onaf_pio_write_data(onfi_info_t *info, void *ram_addr, u32_t wr_bytes, u32_t blk_page_idx, u32_t col_addr);
void onaf_pio_read_data(onfi_info_t *info, void *ram_addr, u32_t wr_bytes, u32_t blk_page_idx, u32_t col_addr);
s32_t onaf_page_write(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx);
void onaf_page_read(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx);
s32_t onaf_page_read_with_ecc_decode(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx, void *p_eccbuf);
s32_t onaf_page_write_with_ecc_encode(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx, void *p_eccbuf);
s32_t onaf_page_read_with_ondie_ecc(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx, void *p_eccbuf);
s32_t onaf_page_write_with_ondie_ecc(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx, void *p_eccbuf);

#endif //ONFI_CTRL_H

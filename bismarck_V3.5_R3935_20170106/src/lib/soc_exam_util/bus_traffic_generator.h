#ifndef BUS_TRAFFIC_GENERATOR_H
#define BUS_TRAFFIC_GENERATOR_H

#define BTG_WRITE 0
#define BTG_READ  1

#define BTG_RCHK_NONE 0
#define BTG_RCHK_DGEN 1
#define BTG_RCHK_CSUM 2

#define BTG_REC_FIRST  0
#define BTG_REC_LATEST 1

#define BTG_IMPRECISE 0
#define BTG_PRECISE   1

#define BTG_FINITE   0
#define BTG_INFINITE 1

#define BTG_BLEN_16  0
#define BTG_BLEN_32  1
#define BTG_BLEN_64  2
#define BTG_BLEN_128 3

#define GDMAREG(req, oft) (*((volatile u32_t *)(gdma_get_base(req) + oft)))
#define BTGREG(req, oft)  (*((volatile u32_t *)(btg_get_base(req) + oft)))
#define BTGIVEC(req, n)   (*((volatile u32_t *)(btg_get_base(req) + 0x30 + (n*4))))


#define GDMACRo   0x00
#define GDMAIMRo  0x04
#define GDMAISRo  0x08

#define BTGCRo   0x00
#define BTGIRo   0x04
#define BTGRTRo  0x08
#define BTGPRo   0x0C
#define BTGBARo  0x10
#define BTGAMRo  0x14
#define BTGGRo   0x18
#define BTGCARo  0x1C
#define BTGRFARo 0x20
#define BTGPFARo 0x24
#define BTGVFARo 0x28
#define BTGMRTRo 0x2c

typedef struct {
	u32_t gbase;
	u32_t lxid;
	u32_t feat:3;/* BTG_WRITE & BTG_READ */
	u32_t rec_latest:1;
	u32_t precise:1;
	u32_t rchkmode:2; /* 0: NONE; 1: data gen.; 2: checksum; 3: undefined. */
	u32_t burstlen:3; /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
	u32_t pktlen:14;  /* from 32B ~ 8160B in a unit of 32B. */
	u32_t iter;       /* 0: infinite test */
	u32_t resp_time;  /* in cycles. */
	u32_t perid_time; /* in cycles. */
	u32_t addr_base;
	u32_t addr_mask;
	s32_t  addr_gap;
} btg_para_v3_t;


typedef void (fpv_btg_t)(btg_para_v3_t *btg_entry);


// Extern for CLI use
extern btg_para_v3_t btg_parameter[];
extern void _gdma_reset(btg_para_v3_t *req);
extern void _btg_start(btg_para_v3_t *req);
extern void _btg_stop(btg_para_v3_t *req);
extern void _btg_setup(btg_para_v3_t *req);
extern void _btg_para_decode(btg_para_v3_t *req);
extern void _btg_print_error(btg_para_v3_t *req);
extern void _btg_show_reg(btg_para_v3_t *req);
extern void _gdma_show_reg(btg_para_v3_t *req);
extern s32_t btg_concur_mt_test(u32_t mt_loops);
extern void btg_pbo_like_data_init(void);
extern void btg_check_exist(btg_para_v3_t *req);
void _btg_func_4database(fpv_btg_t *func_ptr, btg_para_v3_t *req);

#define GDMA_RESET_ALL(req) _btg_func_4database(_gdma_reset, req)
#define BTG_SETUP_ALL(req) _btg_func_4database(_btg_setup, req)
#define BTG_START_ALL(req) _btg_func_4database(_btg_start, req)
#define BTG_PARA_DECODE_ALL(req) _btg_func_4database(_btg_para_decode, req)
#define BTG_PRI_ERR_ALL(req) _btg_func_4database(_btg_print_error, req)
#define BTG_SHOW_REG_ALL(req) _btg_func_4database(_btg_show_reg, req)
#define BTG_STOP_ALL(req) _btg_func_4database(_btg_stop, req)

#endif

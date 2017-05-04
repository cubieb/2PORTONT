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


#define GDMAREG(req, oft) (*((volatile uint32_t *)(gdma_get_base(req) + oft)))
#define BTGREG(req, oft)  (*((volatile uint32_t *)(btg_get_base(req) + oft)))
#define BTGIVEC(req, n)   (*((volatile uint32_t *)(btg_get_base(req) + 0x30 + (n*4))))



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
	const uint32_t gbase;
	const uint32_t lxid;
	const uint32_t feat:3;/* BTG_WRITE & BTG_READ */
	uint32_t rec_latest:1;
	uint32_t precise:1;
	uint32_t rchkmode:2; /* 0: NONE; 1: data gen.; 2: checksum; 3: undefined. */
	uint32_t burstlen:3; /* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
	uint32_t pktlen:14;  /* from 32B ~ 8160B in a unit of 32B. */
	uint32_t iter;       /* 0: infinite test */
	uint32_t resp_time;  /* in cycles. */
	uint32_t perid_time; /* in cycles. */
	uint32_t addr_base;
	uint32_t addr_mask;
	int32_t  addr_gap;
} btg_para_v3_t;

void btg_verification(void);

extern void btg_status(btg_para_v3_t *btg_entry);
extern void btg_controller_init(btg_para_v3_t *btg_entry);
extern void _btg_start(btg_para_v3_t *req);
extern void _btg_stop(btg_para_v3_t *req);


#endif

#ifndef BUS_TRAFFIC_GENERATOR_H
#define BUS_TRAFFIC_GENERATOR_H

/* typedef unsigned int   uint32_t; */
/* typedef          int    int32_t; */
/* typedef unsigned short uint16_t; */
/* typedef          short  int16_t; */
/* typedef unsigned char   uint8_t; */
/* typedef          char    int8_t; */

/* typedef unsigned int   u32_t; */
/* typedef          int   s32_t; */
/* typedef unsigned short u16_t; */
/* typedef          short s16_t; */
/* typedef unsigned char  u8_t; */
/* typedef          char  s8_t; */

#define BTGBASE  (0xb800a000)
#define GDMACR   *((volatile uint32_t *)(BTGBASE))
#define GDMAIMR  *((volatile uint32_t *)(BTGBASE + 0x004))
#define GDMAISR  *((volatile uint32_t *)(BTGBASE + 0x008))

#define BTGWCR   *((volatile uint32_t *)(BTGBASE + 0x100))
#define BTGWIR   *((volatile uint32_t *)(BTGBASE + 0x104))
#define BTGWRTR  *((volatile uint32_t *)(BTGBASE + 0x108))
#define BTGWPR   *((volatile uint32_t *)(BTGBASE + 0x10C))
#define BTGWBAR  *((volatile uint32_t *)(BTGBASE + 0x110))
#define BTGWAMR  *((volatile uint32_t *)(BTGBASE + 0x114))
#define BTGWGR   *((volatile uint32_t *)(BTGBASE + 0x118))
#define BTGWCAR  *((volatile uint32_t *)(BTGBASE + 0x11C))
#define BTGWRFAR *((volatile uint32_t *)(BTGBASE + 0x120))
#define BTGWPFAR *((volatile uint32_t *)(BTGBASE + 0x124))
#define RESERVE0 *((volatile uint32_t *)(BTGBASE + 0x128))
#define BTGWMRTR *((volatile uint32_t *)(BTGBASE + 0x12c))
#define BTGWMIV  ((volatile uint32_t *)(BTGBASE + 0x130))

#define BTGRCR   *((volatile uint32_t *)(BTGBASE + 0x200))
#define BTGRIR   *((volatile uint32_t *)(BTGBASE + 0x204))
#define BTGRRTR  *((volatile uint32_t *)(BTGBASE + 0x208))
#define BTGRPR   *((volatile uint32_t *)(BTGBASE + 0x20C))
#define BTGRBAR  *((volatile uint32_t *)(BTGBASE + 0x210))
#define BTGRAMR  *((volatile uint32_t *)(BTGBASE + 0x214))
#define BTGRGR   *((volatile uint32_t *)(BTGBASE + 0x218))
#define BTGRCAR  *((volatile uint32_t *)(BTGBASE + 0x21C))
#define BTGRRFAR *((volatile uint32_t *)(BTGBASE + 0x220))
#define BTGRPFAR *((volatile uint32_t *)(BTGBASE + 0x224))
#define BTGRVFAR *((volatile uint32_t *)(BTGBASE + 0x228))
#define BTGRMRTR *((volatile uint32_t *)(BTGBASE + 0x22c))
#define BTGRMIV  ((volatile uint32_t *)(BTGBASE + 0x230))

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
	uint8_t btg_id;     /* 0: use BTG0; 1: use BTG1. */
	uint8_t cmd;        /* 100: WRITE; 200: READ */
	uint8_t rec_latest; /* 0: record the first error; 1: record the latest error. */
	uint8_t precise;    /* 0: imprecise access; 1: precise access. */

	uint8_t rchkmode;					/* 0: NONE; 1: data gen.; 2: checksum */
	uint8_t burstlen;					/* 0: 16B; 1: 32B, 2: 64B, 3: 128B */
	uint16_t pktlen;					/* from 32B ~ 8160B in a unit of 32B. */

	uint32_t iter;								/* 0: for infinite test. */
	uint32_t response_time;				/* in cycles. */
	uint32_t period_time;					/* in cycles. */
	uint32_t addr_base;
	uint32_t addr_mask;
	int32_t addr_gap;
} btg_para_t;

void btg_verification(void);
/* u32_t printf(const char *fmt, ...); */
/* void pblr_puts (const char *s); */

#endif

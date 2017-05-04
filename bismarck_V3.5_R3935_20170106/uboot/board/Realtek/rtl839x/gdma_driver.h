#ifndef GDMAREGS_H
#define GDMAREGS_H

#define GDMABASE (0xb800a000)

#define GDMACNR  ((volatile unsigned int *)(GDMABASE))
#define GDMAIMR  ((volatile unsigned int *)(GDMABASE + 0x04))
#define GDMAISR  ((volatile unsigned int *)(GDMABASE + 0x08))
#define GDMACVL  ((volatile unsigned int *)(GDMABASE + 0x0c))
#define GDMACVR  ((volatile unsigned int *)(GDMABASE + 0x10))
#define GDMASBP0 ((volatile unsigned int *)(GDMABASE + 0x20))
#define GDMASBL0 ((volatile unsigned int *)(GDMABASE + 0x24))
#define GDMASBP1 ((volatile unsigned int *)(GDMABASE + 0x28))
#define GDMASBL1 ((volatile unsigned int *)(GDMABASE + 0x2c))
#define GDMASBP2 ((volatile unsigned int *)(GDMABASE + 0x30))
#define GDMASBL2 ((volatile unsigned int *)(GDMABASE + 0x34))
#define GDMASBP3 ((volatile unsigned int *)(GDMABASE + 0x38))
#define GDMASBL3 ((volatile unsigned int *)(GDMABASE + 0x3c))
#define GDMASBP4 ((volatile unsigned int *)(GDMABASE + 0x40))
#define GDMASBL4 ((volatile unsigned int *)(GDMABASE + 0x44))
#define GDMASBP5 ((volatile unsigned int *)(GDMABASE + 0x48))
#define GDMASBL5 ((volatile unsigned int *)(GDMABASE + 0x4c))
#define GDMASBP6 ((volatile unsigned int *)(GDMABASE + 0x50))
#define GDMASBL6 ((volatile unsigned int *)(GDMABASE + 0x54))
#define GDMASBP7 ((volatile unsigned int *)(GDMABASE + 0x58))
#define GDMASBL7 ((volatile unsigned int *)(GDMABASE + 0x5c))
#define GDMADBP0 ((volatile unsigned int *)(GDMABASE + 0x20 + 0x40))
#define GDMADBL0 ((volatile unsigned int *)(GDMABASE + 0x24 + 0x40))
#define GDMADBP1 ((volatile unsigned int *)(GDMABASE + 0x28 + 0x40))
#define GDMADBL1 ((volatile unsigned int *)(GDMABASE + 0x2c + 0x40))
#define GDMADBP2 ((volatile unsigned int *)(GDMABASE + 0x30 + 0x40))
#define GDMADBL2 ((volatile unsigned int *)(GDMABASE + 0x34 + 0x40))
#define GDMADBP3 ((volatile unsigned int *)(GDMABASE + 0x38 + 0x40))
#define GDMADBL3 ((volatile unsigned int *)(GDMABASE + 0x3c + 0x40))
#define GDMADBP4 ((volatile unsigned int *)(GDMABASE + 0x40 + 0x40))
#define GDMADBL4 ((volatile unsigned int *)(GDMABASE + 0x44 + 0x40))
#define GDMADBP5 ((volatile unsigned int *)(GDMABASE + 0x48 + 0x40))
#define GDMADBL5 ((volatile unsigned int *)(GDMABASE + 0x4c + 0x40))
#define GDMADBP6 ((volatile unsigned int *)(GDMABASE + 0x50 + 0x40))
#define GDMADBL6 ((volatile unsigned int *)(GDMABASE + 0x54 + 0x40))
#define GDMADBP7 ((volatile unsigned int *)(GDMABASE + 0x58 + 0x40))
#define GDMADBL7 ((volatile unsigned int *)(GDMABASE + 0x5c + 0x40))

#define LDB (0x80000000)

#define GDMA_start() do { \
		*GDMACNR = 0x00000000;  \
		*GDMAIMR = 0x80000000;  \
		*GDMAISR = 0x80000000;  \
		*GDMACNR = KICKOFF | FUNC_memcpy; \
	} while(0)

#define FUNC_memcpy   (0x0 << 24)
#define FUNC_checksum (0x1 << 24)
#define FUNC_seq_tcam (0x2 << 24)
#define FUNC_memset   (0x3 << 24)
#define FUNC_memcmp   (0xa << 24)
#define FUNC_byteswap (0xb << 24)
#define FUNC_patmatch (0xc << 24)

#define BURSTSIZ_4W  (0x0)
#define BURSTSIZ_8W  (0x1 << 6)
#define BURSTSIZ_16W (0x2 << 6)
#define BURSTSIZ_32W (0x3 << 6)

#define KICKOFF (0x3 << 30)

#define GDMA_reset() do {   \
		*GDMACNR = 0x00000000;  \
		*GDMAIMR = 0x80000000;  \
		*GDMAISR = 0x80000000;  \
		*GDMASBL0 = 0x00000000; \
		*GDMASBL1 = 0x00000000; \
		*GDMASBL2 = 0x00000000; \
		*GDMASBL3 = 0x00000000; \
		*GDMASBL4 = 0x00000000; \
		*GDMASBL5 = 0x00000000; \
		*GDMASBL6 = 0x00000000; \
		*GDMASBL7 = 0x00000000; \
		*GDMADBL0 = 0x00000000; \
		*GDMADBL1 = 0x00000000; \
		*GDMADBL2 = 0x00000000; \
		*GDMADBL3 = 0x00000000; \
		*GDMADBL4 = 0x00000000; \
		*GDMADBL5 = 0x00000000; \
		*GDMADBL6 = 0x00000000; \
		*GDMADBL7 = 0x00000000; \
	} while(0)

void GDMA_polling(void);
void GDMA_memcmp(unsigned char *s1, unsigned char *s2, unsigned int sz);
void GDMA_meminit(unsigned char *s1, unsigned char *s2, unsigned int sz);
void GDMA_blk_set(unsigned int src, unsigned int dest, unsigned int size, unsigned int burst_idx);

#endif

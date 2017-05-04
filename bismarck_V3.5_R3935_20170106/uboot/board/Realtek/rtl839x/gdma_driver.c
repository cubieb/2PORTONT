#include "concur_test.h"
//#include "gdma_driver.h"

#define TO_PHY_ADDR(VirAddr)      ((VirAddr)&((u32_t)0x1FFFFFFF))

#define GDMA_BUSY (1)
#define GDMA_IDLE (0)
#define GDMA_COMPIP (0x80000000)

extern u32_t err_cnt;

inline static u32_t GDMA_nb_polling(void) {
	if ((*GDMAISR & GDMA_COMPIP) == 0)
		return GDMA_BUSY;
	else
		return GDMA_IDLE;
}

void GDMA_polling(void) {
	volatile u32_t wait = 0;
	while (GDMA_nb_polling() == GDMA_BUSY) {
		if (wait++ == 10000) {
			printf("EE: GDMA no response...\n");
			while(1);
		}
	}
	return;
}

void GDMA_memcmp(u8_t *s1, u8_t *s2, u32_t sz) {
	u32_t curr = 0;
	u32_t dst, src;

	while (curr < sz) {
		src = *((volatile u8_t *)(s1 + curr));
		dst = *((volatile u8_t *)(s2 + curr));

		if (src != dst) {
			printf("EE: GDMA: DST: %x(%p) != SRC: %x(%p)\n",
			       dst, ((u8_t *)(s2 + curr)),
			       src, ((u8_t *)(s1 + curr)));
			ERR_HALT(); \
		}
		curr++;
	}
	return;
}

void GDMA_meminit(u8_t *s1, u8_t *s2, u32_t sz) {
	static u32_t f = 0;
	while (sz--) {
		*((u8_t *)(s1 + sz)) = (sz+f) & 0x000000ff;
		*((u8_t *)(s2 + sz)) = 0xff;
	}
	s1[0] = f;
	s1[1] = f;
	s1[2] = f;
	s1[3] = f;
	f++;
	return;
}

void GDMA_blk_set(u32_t src, u32_t dest, u32_t size, u32_t burst_idx) {
	volatile u32_t *curr_src_reg  = GDMASBP0;
	volatile u32_t *curr_dest_reg = GDMADBP0;

	if (size > 65528) {
		printf("EE: Can't DMA more than 65528 bytes.\n");
		while(1);
	}

	while (1) {
		*curr_src_reg  = TO_PHY_ADDR(src);
		*curr_dest_reg = TO_PHY_ADDR(dest);

		if (size > 8191) {
			*((volatile u32_t *)(curr_src_reg+1))  = 8191;
			*((volatile u32_t *)(curr_dest_reg+1)) = 8191;

			size -= 8191;
			src  += 8191;
			dest += 8191;
			curr_src_reg  += 2;
			curr_dest_reg += 2;
		} else {
			*((volatile u32_t *)(curr_src_reg+1))  = size | LDB;
			*((volatile u32_t *)(curr_dest_reg+1)) = size | LDB;
			break;
		}
	}

	*GDMACNR = (burst_idx << 6);

	return;
}

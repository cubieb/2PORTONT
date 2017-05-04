#include "concur_test.h"

u32_t err_cnt;

#define PAT_WR(dst, pat, range, lsrc) do { \
		u32_t lnum; \
		u32_t iter = range / pat->num; \
		u32_t rest = range % pat->num; \
		while (iter--) { \
			lnum = pat->num; \
			lsrc = pat->data; \
			while (lnum--) { \
				*dst++ = *lsrc++; \
			} \
		} \
		lsrc = pat->data; \
		while (rest--) { \
			*dst++ = *lsrc++; \
		} \
	} while(0)

#define PAT_VERIFY(dst, pat, range) do { \
		u32_t lnum; \
		u32_t iter = range / pat->num; \
		u32_t rest = range % pat->num; \
		while (iter--) { \
			lnum = pat->num; \
			lsrc = pat->data; \
			while (lnum--) { \
				if (*(dst) != *(lsrc)) { \
					printf("EE: DST: %x(%p) != SRC: %x(%p)\n", *dst, dst, *lsrc, lsrc); \
					ERR_HALT(); \
				} \
				dst++; lsrc++; \
			} \
		} \
		lsrc = pat->data; \
		while (rest--) { \
			if (*(dst) != *(lsrc)) { \
				printf("EE: DST: %x(%p) != SRC: %x(%p)\n", *dst, dst, *lsrc, lsrc); \
				ERR_HALT(); \
			} \
			dst++; lsrc++; \
		} \
	} while(0)

#if (GDMA_INBOUND == 1)
const u8_t  gdma_burst_words[] = {4, 8, 16, 32};
//const u32_t gdma_src[] = {GDMA_DRAM_SRC, GDMA_DRAM_SRC, GDMA_SRAM_SRC, GDMA_SRAM_SRC};
//const u32_t gdma_dst[] = {GDMA_DRAM_DST, GDMA_SRAM_DST, GDMA_DRAM_DST, GDMA_SRAM_DST};
/* gdma src     -> dst
    1. diam src -> dram dst
    2. dram src -> sram dst
    3. sram src -> dram dst
    4. sram src -> sram dst */
u32_t gdma_src[4], gdma_dst[4];

#endif

static inline u32_t pattern_write_1B(u8_t *dst, pattern_t *pat, u32_t range) {
	u8_t *lsrc;
	PAT_WR(dst, pat, range, lsrc);
	return 0;
}

static inline u32_t pattern_write_2B(u16_t *dst, pattern_t *pat, u32_t range) {
	u16_t *lsrc;
	PAT_WR(dst, pat, range, lsrc);
	return 0;
}

static inline u32_t pattern_write_4B(u32_t *dst, pattern_t *pat, u32_t range) {
	u32_t *lsrc;
	PAT_WR(dst, pat, range, lsrc);
	return 0;
}

static inline u32_t pattern_verify_1B(u8_t *dst, pattern_t *pat, u32_t range) {
	u8_t *lsrc;
	PAT_VERIFY(dst, pat, range);
	return 0;
}

static inline u32_t pattern_verify_2B(u16_t *dst, pattern_t *pat, u32_t range) {
	u16_t *lsrc;
	PAT_VERIFY(dst, pat, range);
	return 0;
}

static inline u32_t pattern_verify_4B(u32_t *dst, pattern_t *pat, u32_t range) {
	u32_t *lsrc;
	PAT_VERIFY(dst, pat, range);
	return 0;
}

static inline u32_t pattern_write(void *dst, pattern_t *pat, u32_t range) {
	u32_t ret;

#if (GDMA_INBOUND == 1)
	GDMA_start();
#endif
	if (pat->unit == 1) {
		ret = pattern_write_1B(dst, pat, range);
	} else if (pat->unit == 2) {
		ret = pattern_write_2B(dst, pat, range);
	} else if (pat->unit == 4) {
		ret = pattern_write_4B(dst, pat, range);
	}
#if (GDMA_INBOUND == 1)
	GDMA_polling();
	GDMA_memcmp((u8_t *)gdma_src[gregion], (u8_t *)gdma_dst[gregion], GDMA_LEN_B);
#endif

	return ret;
}

static inline u32_t pattern_verify(void *dst, pattern_t *pat, u32_t range) {
	u32_t ret;

#if (GDMA_INBOUND == 1)
	GDMA_start();
#endif
	if (pat->unit == 1) {
		ret = pattern_verify_1B(dst, pat, range);
	} else if (pat->unit == 2) {
		ret = pattern_verify_2B(dst, pat, range);
	} else if (pat->unit == 4) {
		ret = pattern_verify_4B(dst, pat, range);
	}
#if (GDMA_INBOUND == 1)
	GDMA_polling();
	GDMA_memcmp((u8_t *)gdma_src[gregion], (u8_t *)gdma_dst[gregion], GDMA_LEN_B);
#endif

	return ret;
}

int concur_test (int flag, int argc, char *argv[])
{
	u32_t input_data[PAT_LEN_4B];
    u32_t start_dram;	
	pattern_t pattern = {(void *)input_data, sizeof(input_data[0]), PAT_LEN_4B};
	pattern_generator *pattern_gen[4];
	u32_t i, t, p, range, total_err_cnt=0, iter=0, testLoops=1;
	char *pat_name;
	u32_t pfch_case = 0;
	const char *pfch_str[] = {"NO", "D", "I", "ID"};

	/* Assign Memory Test & GDMA Test Address */
    if ((initdram(0)-(CONFIG_SYS_TEXT_BASE+0x300000))>(TEST_DRAMLEN_B+2*GDMA_LEN_B)) {
        start_dram = (CONFIG_SYS_TEXT_BASE+0x300000);
        gdma_src[0] = gdma_src[1] = ((start_dram+TEST_DRAMLEN_B)|0x20000000);
        gdma_dst[0] = gdma_dst[2] = ((start_dram+2*TEST_DRAMLEN_B)|0x20000000);
    } else if ((CONFIG_SYS_TEXT_BASE-0x300000)>(TEST_DRAMLEN_B+2*GDMA_LEN_B)) {
    	start_dram = (CONFIG_SYS_TEXT_BASE-0x300000)-TEST_DRAMLEN_B;
        gdma_src[0] = gdma_src[1] = ((start_dram-GDMA_LEN_B)|0x20000000);
        gdma_dst[0] = gdma_dst[2] = ((start_dram-2*GDMA_LEN_B)|0x20000000);
    } else {
		printf("Error: No enough space\n");
        return -1;
    }
	/* Prepare SRAM */
    gdma_src[2] = gdma_src[3] = GDMA_SRAM_SRC;
    gdma_dst[1] = gdma_dst[3] = GDMA_SRAM_DST;
    
	test_range_t targets[] = {{(void *)(start_dram|0x20000000),	(void *)(start_dram|0x20000000),	TEST_DRAMLEN_B},
	                        {(void *)(start_dram&0xDFFFFFFF),	(void *)(start_dram&0xDFFFFFFF),	TEST_DRAMLEN_B},
	                        {(void *)(start_dram|0x20000000),	(void *)(start_dram&0xDFFFFFFF),	TEST_DRAMLEN_B},
	                        {(void *)(START_UCSRAM),			(void *)(START_UCSRAM),				TEST_SRAMLEN_B},
	                        {(void *)(START_UCSRAM&0xDFFFFFFF),	(void *)(START_UCSRAM&0xDFFFFFFF),	TEST_SRAMLEN_B},
	                        {(void *)(START_UCSRAM),			(void *)(START_UCSRAM&0xDFFFFFFF),	TEST_SRAMLEN_B}};

	pattern_gen[0] = pattern_gen_seq;
	pattern_gen[1] = pattern_gen_random;
	pattern_gen[2] = pattern_gen_random;
	pattern_gen[3] = pattern_gen_random;
   	err_cnt = 0;

    if(argc > 1)
        testLoops = simple_strtoul(argv[1], NULL, 10);
    else
        testLoops = 1;

 restart:
    if(iter >= testLoops) return 0;
    
	printf("II: pattern length: %dB\n"
	       "II: cache line size: %dB\n",
	       PAT_LEN_4B*4, CACHELINE_SZ_B);

	for (pfch_case=0; pfch_case<4; pfch_case++) /* prefetch case loop */
    {   
		u32_t mcr_val = (*((volatile u32_t *)0xb8001000)) & (~(0x3 << 22));
		(*((volatile u32_t *)0xb8001000)) = mcr_val | (pfch_case << 22);

		printf("II: %s-Prefetch\n", pfch_str[pfch_case]);

#if (GDMA_INBOUND == 1)
		for (gburst=0; gburst<4; gburst++) /* Burst type loop */
        {
			for (gregion=0; gregion<4; gregion++) /* GDMA test region */
            {
            	printf("II: GDMA %p -> %p (%dB) %dW\n", (void *)gdma_src[gregion],
                    (void *)gdma_dst[gregion], GDMA_LEN_B, gdma_burst_words[gburst]);
                GDMA_blk_set(gdma_src[gregion], gdma_dst[gregion], GDMA_LEN_B, gdma_burst_words[gburst]);
#endif
				for (t=0; t<6; t++)
                {
					/* Simple write/read test with different patterns. */
					/* For the 4 patterns */
					for (p=0; p<4; p++) {
						/* For cache/uncache */
						/* For 1B/2B/4B data unit */
						for (i=0; i<3; i++) {
							pattern.unit = (1 << i);
							pattern.num = (PAT_LEN_4B * 4) / pattern.unit;
							range = targets[t].len / pattern.unit;

							flush_dcache_all();
#if (GDMA_INBOUND == 1)
							GDMA_meminit((u8_t *)gdma_src[gregion],
								(u8_t *)gdma_dst[gregion], GDMA_LEN_B);
#endif
							pat_name = pattern_gen[p](&pattern, (u32_t)targets[t].wstart);

							printf("II: [%s][%08x] %dKx%dB @ %p->%p[%dKB]... ",
						       pat_name, *((u32_t *)pattern.data),
						       pattern.num>>10, pattern.unit,
						       input_data, targets[t].wstart, targets[t].len>>10);
							pattern_write(targets[t].wstart, &pattern, range);

							printf("verifying %p... ", targets[t].rstart);
							pattern_verify(targets[t].rstart, &pattern, range);

							printf("OK\r");
						}
					}

                    /* Unaligned write/read with 1B, 2B, and 3B offset */
					unaligned_access(&pattern, &targets[t]);

					/* Cache-line stride W/R. */
					cacheline_stride_access(&pattern, &targets[t]);
				}

				total_err_cnt += err_cnt;
				printf("Round #%d in-progress report: %d/%d errors.\n",
			       iter, err_cnt, total_err_cnt);
				err_cnt = 0;

#if (GDMA_INBOUND == 1)
			}	/* GDMA test region */
		}	/* Burst type loop */
#endif
	}	/* prefetch case loop */

	iter++;

	goto restart;

	return 0;
}

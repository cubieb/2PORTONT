#include "concur_test.h"

#define GET_SEED 1
#define SET_SEED 0

extern u32_t err_cnt;

inline static
void __srandom32(u32_t *a1, u32_t *a2, u32_t *a3, int get_or_set) {
	static int s1, s2, s3;
	if (GET_SEED==get_or_set) {
		*a1=s1;
		*a2=s2;
		*a3=s3;
	} else {
		s1=*a1;
		s2=*a2;
		s3=*a3;
	}
	return;
}

inline static
unsigned int __random32(void) {
#define TAUSWORTHE(s,a,b,c,d) ((s&c)<<d) ^ (((s <<a) ^ s)>>b)
        u32_t s1, s2, s3;
        __srandom32(&s1, &s2, &s3, GET_SEED);

        s1 = TAUSWORTHE(s1, 13, 19, 4294967294UL, 12);
        s2 = TAUSWORTHE(s2, 2, 25, 4294967288UL, 4);
        s3 = TAUSWORTHE(s3, 3, 11, 4294967280UL, 17);

        __srandom32(&s1, &s2, &s3, SET_SEED);

        return (s1 ^ s2 ^ s3);
}

u32_t rot_patterns[] = {
	0x5a5aa5a5,
	0xa5a55a5a,
	0x00010203,
	0x04050607,
	0x08090a0b,
	0x0c0d0e0f
};

inline static u32_t rotate_1bit(u32_t v) {
	return (((v) << 1) | ((v & 0x80000000)>>31));
}

char* pattern_gen_random(pattern_t *pat, u32_t parameter __attribute__((unused))) {
	u32_t i, n;
	u32_t a=0x13243, b=0xaaa0bdd, c=0xfffbda0;
	u8_t *data = (u8_t *)pat->data;
	u8_t tmp[4];
	static char pat_name[] = "RD0";
	static u32_t pat_id = 0;

	__srandom32(&a, &b, &c, SET_SEED);
	for (i=0; i<pat->num; i++) {
		rot_patterns[pat_id] = rotate_1bit(rot_patterns[pat_id]);
		*((u32_t *)tmp) = __random32() ^ rot_patterns[pat_id];
		for (n=0; n<pat->unit; n++) {
			*data++ = tmp[3-n];
		}
	}

	/* "48" stands for '0' in ASCII. */
	pat_name[2] = pat_id + 48;
	pat_id = (pat_id+1) % (sizeof(rot_patterns)/sizeof(u32_t));

	return pat_name;
}

char* pattern_gen_seq(pattern_t *pat, u32_t parameter) {
	u32_t i, n;
	u8_t *data = (u8_t *)pat->data;
	u8_t tmp[4];
	static char pat_name[] = "SEQ";

	for (i=0; i<pat->num; i++) {
		*((u32_t *)tmp) = i + parameter;
		for (n=0; n<pat->unit; n++) {
			*data++ = tmp[3-n];
		}
	}

	return pat_name;
}


#if (GDMA_INBOUND == 1)
extern const u8_t  gdma_burst_words[];
//extern const u32_t gdma_src[];
//extern const u32_t gdma_dst[];
extern u32_t gdma_src[4], gdma_dst[4];
#endif

#define UAL_WR_OFT(x, ual_st, end, pattern_array) do { \
		while (((u32_t)ual_st) < end) { \
			UAL_DUMMY_WR_OFT(x, ual_st); \
			ual_st->i = pattern_array[((u32_t)ual_st)%PAT_LEN_4B]; \
			ual_st++; \
		} \
	} while(0)

#define UAL_DUMMY_WR_OFT(x, ual_st) UAL_DUMMY_WR_OFT##x(ual_st)
#define UAL_DUMMY_WR_OFT1(ual_st) do {ual_st->c0 = 0xcc;} while(0)
#define UAL_DUMMY_WR_OFT2(ual_st) do {ual_st->c0 = 0xcc; ual_st->c1 = 0xdd;} while(0)
#define UAL_DUMMY_WR_OFT3(ual_st) do {ual_st->c0 = 0xcc; ual_st->c1 = 0xdd; ual_st->c2 = 0xee;} while(0)

#define UAL_VR_OFT(x, ual_st, end, pattern_array) do { \
		u32_t tmp_src, tmp_dst; \
		while (((u32_t)ual_st) < end) { \
			tmp_src = pattern_array[((u32_t)ual_st)%PAT_LEN_4B]; \
			tmp_dst = ual_st->i; \
			if ((UAL_DUMMY_VR_OFT(x, ual_st) + \
			     (tmp_dst - tmp_src)) != 0) { \
				printf("EE: DST: %x(%p) != SRC: %x(%p)\n", \
				       tmp_dst, ual_st, tmp_src, &pattern_array[((u32_t)tmp)%PAT_LEN_4B]); \
				ERR_HALT(); \
			} \
			ual_st++; \
		} \
	} while(0)

#define UAL_DUMMY_VR_OFT(x, ual_st) UAL_DUMMY_VR_OFT##x(ual_st)
#define UAL_DUMMY_VR_OFT1(ual_st) (ual_st->c0 - 0xcc)
#define UAL_DUMMY_VR_OFT2(ual_st) ((ual_st->c0 - 0xcc) + (ual_st->c1 - 0xdd))
#define UAL_DUMMY_VR_OFT3(ual_st) ((ual_st->c0 - 0xcc) + (ual_st->c1 - 0xdd) + (ual_st->c2 - 0xee))

static inline u32_t ual_pattern_write(test_range_t *range, pattern_t *pat, u32_t oft) {
	const u32_t data_end = (u32_t)range->wstart + range->len - sizeof(struct ual_3B);

#if (GDMA_INBOUND == 1)
	GDMA_start();
#endif

	if (oft == 1) {
		struct ual_1B *tmp = (struct ual_1B *)range->wstart;
		UAL_WR_OFT(1, tmp, data_end, ((u32_t *)pat->data));
	} else if (oft == 2) {
		struct ual_2B *tmp = (struct ual_2B *)range->wstart;
		UAL_WR_OFT(2, tmp, data_end, ((u32_t *)pat->data));
	} else if (oft == 3) {
		struct ual_3B *tmp = (struct ual_3B *)range->wstart;
		UAL_WR_OFT(3, tmp, data_end, ((u32_t *)pat->data));
	} else {
		printf("EE: unsupported unalignment offset.\n");
	}

#if (GDMA_INBOUND == 1)
	GDMA_polling();
	GDMA_memcmp((u8_t *)gdma_src[gregion], (u8_t *)gdma_dst[gregion], GDMA_LEN_B);
#endif

	return 0;
}

static inline u32_t ual_pattern_verify(test_range_t *range, pattern_t *pat, u32_t oft) {
	const u32_t data_end = (u32_t)range->rstart + range->len - sizeof(struct ual_3B);

#if (GDMA_INBOUND == 1)
	GDMA_start();
#endif

	if (oft == 1) {
		struct ual_1B *tmp = (struct ual_1B *)range->wstart;
		UAL_VR_OFT(1, tmp, data_end, ((u32_t *)pat->data));
	} else if (oft == 2) {
		struct ual_2B *tmp = (struct ual_2B *)range->wstart;
		UAL_VR_OFT(2, tmp, data_end, ((u32_t *)pat->data));
	} else if (oft == 3) {
		struct ual_3B *tmp = (struct ual_3B *)range->wstart;
		UAL_VR_OFT(3, tmp, data_end, ((u32_t *)pat->data));
	} else {
		printf("EE: unsupported unalignment offset.\n");
	}

#if (GDMA_INBOUND == 1)
	GDMA_polling();
	GDMA_memcmp((u8_t *)gdma_src[gregion], (u8_t *)gdma_dst[gregion], GDMA_LEN_B);
#endif

	return 0;
}

u32_t unaligned_access(pattern_t *pattern, test_range_t *targets) {
	u32_t u;

	/* unaligned 1 ~ 3 bytes test. */
	for (u=1; u<4; u++) {
		flush_dcache_all();

#if (GDMA_INBOUND == 1)
		GDMA_meminit((u8_t *)gdma_src[gregion], (u8_t *)gdma_dst[gregion],
		             GDMA_LEN_B);
#endif

		printf("II: [UA%d][%08x] 2Kx%dB @ %p->%p[%dKB]... ",
		       u, *((u32_t *)pattern->data), 4+u,
		       pattern->data, targets->wstart, targets->len>>10);
		ual_pattern_write(targets, pattern, u);

		printf("verifying %p... ", targets->rstart);
		ual_pattern_verify(targets, pattern, u);

		printf("OK\r");
	}

	return 0;
}

#define PAT_WR(dst, data, stride, len, pnum) do {	  \
		u32_t step = (CACHELINE_SZ_B / sizeof(data[0])) << stride; \
		u32_t i = 0; \
		u32_t bound = (((u32_t)dst) + len); \
		typeof(dst) orig_dst = dst; \
		while (i < step) { \
			dst = orig_dst + (i++); \
			while (((u32_t)dst) < bound) { \
				*dst = data[((u32_t)dst) % pnum]; \
				dst += step; \
			} \
		} \
	} while(0)

#define PAT_VER(src, data, stride, len, pnum) do {	  \
		u32_t step = (CACHELINE_SZ_B / sizeof(data[0])) << stride; \
		u32_t i = 0; \
		u32_t bound = (((u32_t)src) + len); \
		typeof(src) orig_src = src; \
		while (i < step) { \
			src = orig_src + (i++); \
			while (((u32_t)src) < bound) { \
				if (*src != data[((u32_t)src) % pnum]) { \
					printf("EE: DST: %x(%p) != SRC: %x(%p)\n", \
					       *src, src, \
					       data[((u32_t)src)%pnum], \
					       &data[((u32_t)src)%pnum]); \
					ERR_HALT(); \
				}; \
				src += step; \
			} \
		} \
	} while(0)

static inline u32_t cs_pattern_write(test_range_t *dst, pattern_t *pat, u32_t stride) {
	u32_t ret = 0;

#if (GDMA_INBOUND == 1)
	GDMA_start();
#endif
	if (pat->unit == 1) {
		u8_t *ldst = (u8_t *)(dst->wstart);
		PAT_WR(ldst, ((u8_t *)pat->data), stride, dst->len, pat->num);
	} else if (pat->unit == 2) {
		u16_t *ldst = (u16_t *)(dst->wstart);
		PAT_WR(ldst, ((u16_t *)pat->data), stride, dst->len, pat->num);
	} else if (pat->unit == 4) {
		u32_t *ldst = (u32_t *)(dst->wstart);
		PAT_WR(ldst, ((u32_t *)pat->data), stride, dst->len, pat->num);
	}
#if (GDMA_INBOUND == 1)
	GDMA_polling();
	GDMA_memcmp((u8_t *)gdma_src[gregion], (u8_t *)gdma_dst[gregion], GDMA_LEN_B);
#endif
	return ret;
}

static inline u32_t cs_pattern_verify(test_range_t *dst, pattern_t *pat, u32_t stride) {
	u32_t ret = 0;

#if (GDMA_INBOUND == 1)
	GDMA_start();
#endif
	if (pat->unit == 1) {
		u8_t *ldst = (u8_t *)(dst->rstart);
		PAT_VER(ldst, ((u8_t *)pat->data), stride, dst->len, pat->num);
	} else if (pat->unit == 2) {
		u16_t *ldst = (u16_t *)(dst->rstart);
		PAT_VER(ldst, ((u16_t *)pat->data), stride, dst->len, pat->num);
	} else if (pat->unit == 4) {
		u32_t *ldst = (u32_t *)(dst->rstart);
		PAT_VER(ldst, ((u32_t *)pat->data), stride, dst->len, pat->num);
	}
#if (GDMA_INBOUND == 1)
	GDMA_polling();
	GDMA_memcmp((u8_t *)gdma_src[gregion], (u8_t *)gdma_dst[gregion], GDMA_LEN_B);
#endif
	return ret;
}

u32_t cacheline_stride_access(pattern_t *pattern, test_range_t *targets) {
	u32_t s, u;
	u32_t *test_data = pattern->data;

	for (u=0; u<3; u++) {
		for (s=0; s<2; s++) {
			flush_dcache_all();
#if (GDMA_INBOUND == 1)
			GDMA_meminit((u8_t *)gdma_src[gregion],
			             (u8_t *)gdma_dst[gregion],
			             GDMA_LEN_B);
#endif

			pattern->unit = 1 << u;
			pattern->num = (PAT_LEN_4B * 4) / pattern->unit;
			pattern_gen_random(pattern, 0); /* The 2nd parameter is not used for random. */

			printf("II: [CL%d][%08x] %dKx%dB @ %p->%p[%dKB]... ",
			       1<<s, test_data[0], pattern->num>>10, pattern->unit,
			       test_data, targets->wstart, targets->len>>10);
			cs_pattern_write(targets, pattern, s);

			printf("verifying %p... ", targets->rstart);
			cs_pattern_verify(targets, pattern, s);

			printf("OK\r");
		}
	}

	return 0;
}
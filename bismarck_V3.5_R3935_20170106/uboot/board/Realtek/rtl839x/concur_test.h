#ifndef MEM_TEST_H
#define MEM_TEST_H

//#include <100_mem_test_conf.h>
//#include <mengele.h>
#include <soc.h>
#include <pblr.h>
#include <common.h>
#include "gdma_driver.h"

/* 100_mem_test_conf.h */
/* NO need to change 0xBFxxxxxx to 0x9Fxxxxxx.
   It will switch during test process. */
//#define START_UCDRAM   (0xa5000000)
#define START_UCSRAM   (0xbf005000)
#define TEST_DRAMLEN_B (24*1024)
#define TEST_SRAMLEN_B (24*1024)
#define PAT_LEN_4B     (2048)
#define CACHELINE_SZ_B (32)

#define GDMA_INBOUND (1)
#if (GDMA_INBOUND == 1)
  #define GDMA_LEN_B    (25*1024+1016)
  #define GDMA_DRAM_SRC (0xa5800000)
  #define GDMA_DRAM_DST (0xa5c00000 + 1)
  #define GDMA_SRAM_SRC (0xbf000000 + (44*1024) + 2)
  #define GDMA_SRAM_DST (0xbf000000 + (60*1024) + 3)
#endif

//#define INSTANT_DEATH (1)

/* mengele.h */
//#define alloca(sz) __builtin_alloca(sz)

//void flush_dcache_all(void);
#define flush_dcache_all	pblr_dc_flushall

#if 0
#define SWBREAK() do {               \
                __asm__ __volatile__ ("sdbbp;"); \
        } while(0)
#endif

#ifdef CONFIG_MT_ERR_HANDLE
  #define ERR_HALT() do { err_cnt++; } while(0)
#else
  #define ERR_HALT() while(1)
#endif

#if 0
static inline void *memset(void *s, int c, int n) {
        while (n--) {
                *((unsigned char *)s) = c;
                s++;
        }
        return s;
}

static inline void *memset32(unsigned int *s, int c, int n) {
        while (n--) {
                *s = c;
                s++;
        }
        return s;
}
#endif

typedef struct {
	void *data;
	unsigned int unit;
	unsigned int num;
} pattern_t;

typedef struct {
	void *wstart;
	void *rstart;
	unsigned int len;
} test_range_t;

struct ual_1B {
	unsigned char c0;
	unsigned int i;
} __attribute__((packed));

struct ual_2B {
	unsigned char c0;
	unsigned char c1;
	unsigned int i;
} __attribute__((packed));

struct ual_3B {
	unsigned char c0;
	unsigned char c1;
	unsigned char c2;
	unsigned int i;
} __attribute__((packed));

typedef char* pattern_generator(pattern_t *, __attribute__((unused)) unsigned int);

char* pattern_gen_random(pattern_t *pat, unsigned int parameter);
char* pattern_gen_seq(pattern_t *pat, unsigned int parameter);
unsigned int unaligned_access(pattern_t *pat, test_range_t *targets);
unsigned int cacheline_stride_access(pattern_t *pattern, test_range_t *targets);

unsigned int gregion, gburst;

#endif

// LX Timer Init Function
// For 9602C only

#include <soc.h>
typedef union {
	struct {
		unsigned int mbz_0:3; //0
		unsigned int en:1; //0
		unsigned int mbz_1:3; //0
		unsigned int mode:1; //0
		unsigned int mbz_2:8; //0
		unsigned int divfactor:16; //0
	} f;
	unsigned int v;
} TCCTRL_T;

#define TIMER_DATA REG32(UDELAY_TIMER_BASE+0x0)
#define TIMER_CNT  REG32(UDELAY_TIMER_BASE+0x4)
#define TIMER_CTRL REG32(UDELAY_TIMER_BASE+0x8)

#define TI_RES_BAD_PRECISION (1)
#define TI_RES_BAD_DIVISOR   (2)

#if (TIMER_STEP_PER_US && (TIMER_STEP_PER_US & (TIMER_STEP_PER_US - 1)))
	#error TIMER_STEP_PER_US is not power of 2
#endif

u32_t otto_lx_timer_init(const u32_t lx_freq_mhz) {
	TCCTRL_T tc_buffer;
	u32_t res = 0;

	/* divisor to make desired steps */
	u32_t divisor = ((lx_freq_mhz + TIMER_STEP_PER_US - 1) / TIMER_STEP_PER_US);

	if (lx_freq_mhz % TIMER_STEP_PER_US) {
		/* printf("WW: TIMER_FREQ_MHZ (%d) is not divisible by TIMER_STEP_PER_US (%d)\n", */
		/*        TIMER_FREQ_MHZ, */
		/*        TIMER_STEP_PER_US); */
		res |= TI_RES_BAD_PRECISION;
	}

	if (divisor > (1 << 16)) {
		/* printf("WW: Timer divisor overflow\n"); */
		divisor = (1 << 16) - 1;
		res |= TI_RES_BAD_DIVISOR;
	}

	tc_buffer.f.divfactor = divisor;
	tc_buffer.f.mode      = 1; /* timer mode */
	tc_buffer.f.en        = 0; /* disable timer/counter */
	TIMER_CTRL = tc_buffer.v;

	tc_buffer.f.en        = 1; /* enable timer/counter */
	TIMER_CTRL = tc_buffer.v;

	TIMER_DATA = 0x0fffffff;

	return res;
}


    
SECTION_RECYCLE void 
lx_timer_func_reassign(void)
{
    _lx_timer_init = otto_lx_timer_init;
}

REG_INIT_FUNC(lx_timer_func_reassign, 3);



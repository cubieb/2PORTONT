#include <linux/clocksource.h>
#include <linux/init.h>
#include <asm/time.h>
#include <asm/processor.h>
#include "bspchip.h"

#define BSP_QUERY_PLL_REGS

#ifdef BSP_QUERY_PLL_REGS
#define SYSREG_PIN_STATUS_REG_A           (0xB8000100)
#define SYSREG_PIN_STATUS_CLSEL__FD_S       (5)
#define SYSREG_PIN_STATUS_CLSEL_MASK        (1 << SYSREG_PIN_STATUS_CLSEL__FD_S)

#define SYSREG_SYSCLK_CONTROL_REG_A       (0xB8000200)
#define SYSREG_SYSCLK_CONTROL_OCP0PLL_FD_S  (16)
#define SYSREG_SYSCLK_CONTROL_OCP0PLL_MASK  (0x1f << SYSREG_SYSCLK_CONTROL_OCP0PLL_FD_S)

static inline u32 board_query_cpu_hz(void){
    u32 tmp;
    tmp = (REG32(SYSREG_SYSCLK_CONTROL_REG_A) & SYSREG_SYSCLK_CONTROL_OCP0PLL_MASK) 
             >> SYSREG_SYSCLK_CONTROL_OCP0PLL_FD_S ;
    
    if(REG32(SYSREG_PIN_STATUS_REG_A) & SYSREG_PIN_STATUS_CLSEL_MASK){
		return ((tmp + 25) * 20 * 1000000);
     }
    else{
		return ((tmp + 20) * 25 * 1000000);
    }
}
#endif
static cycle_t c0_hpt_read(struct clocksource *cs)
{
	return read_c0_count();
}

static struct clocksource clocksource_rlx = {
	.name  = "processor",
	.read  = c0_hpt_read,
	.mask  = CLOCKSOURCE_MASK(32),
	.flags  = CLOCK_SOURCE_IS_CONTINUOUS,
	.shift  = 30
};
/*
    Revision: 7242
    Author: weihsiang.hung
    Date: 2016年5月20日 下午 03:56:05
    Message:
    Add high resolution timer support.
    sync form lunae8c18 branch r7240.
*/
static u32 board_CPU_freq(void) {
#ifdef BSP_QUERY_PLL_REGS
      /* Get cpu_hz from PLL register */
        return board_query_cpu_hz();
#else

	return 600000000;
#endif
}

int __init rlx_clocksource_init(void)
{
	u32 c;
	/* Cause Reg: bit27
	 * DC Disable Count register.
	 * This bit controls the counting of COUNT register, 1 for disable
	 */
	c = read_c0_cause();

	c &= ~(1<<27);
	write_c0_cause(c);

	clocksource_rlx.rating = board_CPU_freq() / 1000000;
	clocksource_rlx.mult = clocksource_hz2mult(board_CPU_freq(), clocksource_rlx.shift);

	clocksource_register(&clocksource_rlx);
	
        printk("rlx_clocksource_init: High Resolution Timer Clock = %u\n", board_CPU_freq());
	
	return 0;
}

module_init(rlx_clocksource_init);


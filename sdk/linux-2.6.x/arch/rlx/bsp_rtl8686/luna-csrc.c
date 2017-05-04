#include <linux/clocksource.h>
#include <linux/init.h>
#include <asm/time.h>
#include <asm/processor.h>
#include "bspchip.h"

#undef CSRC_DEBUG
#if defined(CSRC_DEBUG)
#define PRINTK_CSRC(fmt, ...)   printk("[CSRC_DEBUG:(%s,%d)]" fmt, __FUNCTION__ ,__LINE__,##__VA_ARGS__)
#else
#define PRINTK_CSRC(fmt, ...)
#endif

#define AFTERX(y) BSP_TC1##y  //If change Timer, please modify here!

#define BSP_CSRC_CNT  AFTERX(CNT)
#define BSP_CSRC_INT  AFTERX(INT)
#define BSP_CSRC_CTL  AFTERX(CTL)
#define BSP_CSRC_DATA AFTERX(DATA)


static cycle_t bsp_soc_hpt_read(struct clocksource *cs)
{ 
       return REG32(BSP_CSRC_CNT);
}

static struct clocksource clocksource_soc = {
	.name  = "Soc H/W Timer",
	/* CPU's count is rating 300 */
	.rating = 340,
	.read  =  bsp_soc_hpt_read ,
	.mask  = CLOCKSOURCE_MASK(28),
	.flags  = CLOCK_SOURCE_IS_CONTINUOUS,
};


void __init clocksource_set_clock(struct clocksource *cs, unsigned int clock)
{
	u64 temp;
	u32 shift;

	/* Find a shift value */
	for (shift = 32; shift > 0; shift--) {
		temp = (u64) NSEC_PER_SEC << shift;
		do_div(temp, clock);
		if ((temp >> 32) == 0)
			break;
	}
	cs->shift = shift;
	cs->mult = (u32) temp;
}
/*
 * We desire 1/4 us == 4Mhz  
 * BSP_HT_DIVISOR= BSP_SYSCLK / 4Mhz 
 */
extern unsigned BSP_SYSCLK;
#define BSP_SOC_TIMER_CLOCK   (4*1000*1000)
#define BSP_HT_DIVISOR         (BSP_SYSCLK/BSP_SOC_TIMER_CLOCK)

extern unsigned int SOC_ID;
void __init luna_clocksource_enable(void){
  
        PRINTK_CSRC("SOC_ID = 0x%08x\n", SOC_ID);

  	if(SOC_ID == 0x0371){
           printk("SOC(0x%08x) not support \"High Resolution Timer Clock\"\n", SOC_ID);
	   return;
	}
	else
	{
		/* Clear Timer IP status */
		if (REG32(BSP_CSRC_INT) & BSP_TCIP) {
			REG32(BSP_CSRC_INT) |= (BSP_TCIP);
		}
	
		/* disable timer */
		REG32(BSP_CSRC_CTL) = 0x0; /* disable timer before setting CDBR */
	
		/* initialize timer registers */
		REG32(BSP_CSRC_CTL) |= (unsigned int)(BSP_HT_DIVISOR) << 0;
		/*MAX DATA Value*/
 		REG32(BSP_CSRC_DATA) = 0x0fffffff;//(28bit)
		/* enable timer */
		REG32(BSP_CSRC_CTL) |= BSP_TCEN |  BSP_TCMODE_TIMER;

	}
  
  	  PRINTK_CSRC("%s,%d; BSP_CSRC_CTL(0x%08x)=0x%08x\n", __func__, __LINE__, BSP_CSRC_CTL,REG32(BSP_CSRC_CTL) );
	  PRINTK_CSRC("%s,%d; BSP_CSRC_CNT(0x%08x)=0x%08x\n", __func__, __LINE__, BSP_CSRC_CNT, REG32(BSP_CSRC_CNT) );
	  PRINTK_CSRC("%s,%d; BSP_CSRC_DATA(0x%08x)=0x%08x\n", __func__, __LINE__, BSP_CSRC_DATA, REG32(BSP_CSRC_DATA) );
	  
}

int __init luna_clocksource_init(void)
{
        luna_clocksource_enable();
        clocksource_set_clock( &clocksource_soc , BSP_SOC_TIMER_CLOCK);
   	clocksource_register(&clocksource_soc);
 	
	PRINTK_CSRC("cs->shift=%u, cs->mult=%u\n", clocksource_soc.shift, clocksource_soc.mult);
	
        printk("luna_clocksource_init: High Resolution Timer Clock = %u, BSP_HT_DIVISOR=%u\n", (unsigned int)BSP_SOC_TIMER_CLOCK, (unsigned int)BSP_HT_DIVISOR);
	
	return 0;
}

module_init(luna_clocksource_init);


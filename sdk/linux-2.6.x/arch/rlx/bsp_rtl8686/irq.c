/*
 * Realtek Semiconductor Corp.
 *
 * arch/rlx/rlxocp0/irq.c
 *   Interrupt and exception initialization for RLX OCP Platform
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 7, 2006
 */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/timex.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/irq.h>

#include <asm/bitops.h>
#include <asm/bootinfo.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/irq_cpu.h>
#include <asm/irq_vec.h>
#include <asm/system.h>

#include <asm/rlxregs.h>
#include <asm/rlxbsp.h>


#include "bspchip.h"
#include "prom.h"

static void bsp_ictl_irq_mask(unsigned int irq)
{
	if(irq < 32) {
		REG32(BSP_GIMR0_0) &= (~(1 << irq));
	}else{
		REG32(BSP_GIMR1_0) &= (~(1 << (irq-32)));
	}
}

static void bsp_ictl_irq_unmask(unsigned int irq)
{
	if(irq < 32) {
		REG32(BSP_GIMR0_0) |=  (1 << irq);
	}else{
		REG32(BSP_GIMR1_0) |= (1 << (irq-32));
	}
}

static struct irq_chip bsp_ictl_irq = {
    .typename = "ICTL-RTL8672",
    .ack = bsp_ictl_irq_mask,
    .mask = bsp_ictl_irq_mask,
    .mask_ack = bsp_ictl_irq_mask,
    .unmask = bsp_ictl_irq_unmask,
};

/*
 *   RTL8672 Interrupt Scheme (Subject to change)
 *
 *   Source     EXT_INT   IRQ      CPU INT
 *   --------   -------   ------   -------
 *   PTM        31        31       2
 *   LBCTMOs2   30        30       2
 *   LBCTMOs1   29        29       2
 *   PKT        28        28       2
 *   SPI        27        27       2
 *   NIC100     26        26       2
 *   SAR        25        25       2
 *   DMT        24        24       2
 *   TC3        23        23       2
 *   TC2        22        22       2
 *   GDMA       21        21       2
 *   SECURITY   20        20       2
 *   PCM        19        19       2
 *   GPIO_EFGH  17        17       2
 *   GPIO_ABCD  16        16       2
 *   SW         15        15       6
 *   PCI        14        14       5
 *   UART1      13        13       2
 *   UART0      12        12       3
 *   USB_D      11        11       2
 *   USB_H      10        10       4
 *   TC1        9         9        2
 *   TC0        8         8        7
 *   LBCTMOm2   7         7        2
 *   LBCTMOm1   6         6        2
 *   SPEED      5         5        2
 *   LBCTMOs0   4         4        2
 *   LBCTMOm0   3         3        2
 *   OCPTMO     2         2        2
 *   PCIB0TO    0         0        2
 */

void __init bsp_irq_init(void)
{
   int i;

    /* disable ict interrupt */
    REG32(BSP_GIMR0_0) = 0;
    REG32(BSP_GIMR1_0) = 0;
    for (i=0; i < 64; i++) 
        set_irq_chip_and_handler( i, &bsp_ictl_irq, handle_level_irq);	

     /* Enable all interrupt mask of CPU */
    write_c0_status(read_c0_status() | ST0_IM); 	

      /* Set GIMR, IRR */
     REG32(BSP_GIMR0_0) = BSP_TMO_IE | BSP_PERIPHERAL_IE;
     REG32(BSP_GIMR1_0) = BSP_TC0_IE ;

#if (CONFIG_SERIAL_8250_RUNTIME_UARTS > 1)
     REG32(BSP_GIMR1_0) |= BSP_UART0_IE;
     REG32(BSP_GIMR1_0) |= BSP_UART1_IE;
#elif defined(CONFIG_USE_UART1)
     REG32(BSP_GIMR1_0) |= BSP_UART1_IE;
#else
     REG32(BSP_GIMR1_0) |= BSP_UART0_IE;
#endif /* #if (CONFIG_SERIAL_8250_RUNTIME_UARTS > 1) */


     REG32(BSP_IRR0_0) = BSP_IRR0_0_SETTING;
     REG32(BSP_IRR1_0) = BSP_IRR1_0_SETTING;
     REG32(BSP_IRR2_0) = BSP_IRR2_0_SETTING;
     REG32(BSP_IRR3_0) = BSP_IRR3_0_SETTING;
     REG32(BSP_IRR4_0) = BSP_IRR4_0_SETTING;
     REG32(BSP_IRR5_0) = BSP_IRR5_0_SETTING;
     REG32(BSP_IRR6_0) = BSP_IRR6_0_SETTING;

}

#define IRQ_LO_BOUND 0
#define IRQ_HI_BOUND 31  
#define TMO_LO 21
#define TMO_HI 31
#define PER_LO 9
#define PER_HI 20

static const char dispatch_table[64] = {
	BSP_CUP1_T_CPU0_IRQ,//0
	BSP_CUP0_T_CPU1_IRQ,//
	BSP_UTMD_IRQ,// 2
	BSP_FLSH_IRQ,//3
	BSP_FFTACC_IRQ,//4
	BSP_WDOG_IRQ,//5
	BSP_NFBI_IRQ,//6
	BSP_SATA_IRQ,//7
	BSP_SWITCH_IRQ,//8
	-1,//9
	BSP_GPIO1_IRQ,//10
	-1,//11
	BSP_PERIPHERAL_IRQ,//12
	BSP_USB_H3_IRQ,//13
	BSP_USB_H2_IRQ,//14
	BSP_PCIE0_IRQ,//15
	BSP_PCIE1_IRQ,//16
	BSP_PCM0_IRQ,//17
	BSP_PCM1_IRQ ,//18
	BSP_SECURITY_IRQ ,//19
	BSP_GDMA0_IRQ,//20
	BSP_GDMA1_IRQ,//21
	BSP_DMT_IRQ  ,//22
	BSP_PKT_XTM_IRQ ,//23
	BSP_PKT_GMAC_IRQ,//24
	BSP_SAR_IRQ,//25
	BSP_GMAC_IRQ,//26
	BSP_GMAC1_IRQ,//27
	BSP_PTM_IRQ ,//28
	BSP_SPI_IRQ,//29
	BSP_VOIPACC_IRQ,//30	
	BSP_TMO_IRQ,//31
	-1,//32
	-1,//33
	-1,//34
	-1,//35	
	-1,//36
	-1,//37
	-1,//38
	-1,//39
	-1,//40
#ifdef LUNA_RTL9602C
	BSP_GPIO_EFGH_IRQ,//41
	BSP_GPIO_ABCD_IRQ,//42
#else
	BSP_GPIO_ABCD_IRQ,//41
	BSP_GPIO_EFGH_IRQ,//42
#endif
	BSP_TC0_IRQ,//43
	BSP_TC1_IRQ,//44
	BSP_TC2_IRQ,//45
	BSP_TC3_IRQ,//46
	BSP_TC4_IRQ,//47
	BSP_TC5_IRQ,//48
	BSP_UART0_IRQ,//49	
	BSP_UART1_IRQ,//50
	BSP_UART2_IRQ,//51	
	BSP_UART3_IRQ,//52
	BSP_OCP0_CPU1_ILA_IP_IRQ ,//53
	BSP_OCP1_CPU0_ILA_IP_IRQ ,//54
	BSP_OCPTO0_IRQ,//55
	BSP_OCPTO1_IRQ,//56
	BSP_LBCTMOs0_IRQ,//57
	BSP_LBCTMOs1_IRQ,//58
	BSP_LBCTMOs2_IRQ,//59
	BSP_LBCTMOs3_IRQ, //60
	BSP_LBCTMOm0_IRQ, //61
	BSP_LBCTMOm1_IRQ,//62
	BSP_LBCTMOm2_IRQ,//63	
};

#ifdef CONFIG_LUNA_CMU
extern unsigned int cmu_cpu0_sleep_cnt;
#endif /* #ifdef CONFIG_LUNA_CMU */

__IRAM_SYS_MIDDLE void bsp_irq_dispatch(void) {
        static unsigned int extint0_ip;
    static unsigned int extint1_ip;
        unsigned int tmp0, tmp1;
	int timer_irq = 0;

#ifdef CONFIG_LUNA_CMU
	cmu_cpu0_sleep_cnt += ((REG32(0xb8000310) >> 24) & 0xFF);
#endif /* #ifdef CONFIG_LUNA_CMU */

        extint0_ip |= REG32(BSP_GIMR0_0) & REG32(BSP_GISR0_0);
        extint1_ip |= REG32(BSP_GIMR1_0) & REG32(BSP_GISR1_0);

        for (tmp0 = IRQ_LO_BOUND; tmp0 <= IRQ_HI_BOUND; tmp0++) {
                if (0 == extint0_ip)
                        break;

                switch (tmp0) {
                case BSP_PERIPHERAL_IRQ:
                        extint0_ip &= ~(1<<BSP_PERIPHERAL_IRQ);
                        for(tmp1= PER_LO; tmp1 <= PER_HI; tmp1++)  {
                                if ((extint1_ip & (1<<tmp1)) && (dispatch_table[tmp1+IRQ_HI_BOUND+1]>0)) {
                                        extint1_ip &= ~(1<<tmp1);
					if(unlikely((tmp1 + IRQ_HI_BOUND + 1) == BSP_TC0_IRQ))
						timer_irq = 1;
					else
                                        	do_IRQ(dispatch_table[tmp1+IRQ_HI_BOUND+1]);                              
                                }
                        }
                        break;

                case BSP_TMO_IRQ:
                        for(tmp1= TMO_LO; tmp1 <= TMO_HI; tmp1++) {
                                if ((extint1_ip & (1<<tmp1)) && (dispatch_table[tmp1+IRQ_HI_BOUND+1]>0)) {
                                        extint1_ip &= ~(1<<tmp1);
                                        do_IRQ(dispatch_table[tmp1+IRQ_HI_BOUND+1]);                              
                                }
                        }
                        break;

                default:
                        if ((extint0_ip & (1<<tmp0)) && (dispatch_table[tmp0]>=0)) {
                                extint0_ip &= ~(1<<tmp0);
                                do_IRQ(dispatch_table[tmp0]);
                        }
                }
        }
	// do timer interrupt handler
	if(timer_irq == 1)
		do_IRQ(dispatch_table[BSP_TC0_IRQ]);
}

/*


__IRAM_SYS_MIDDLE void bsp_irq_dispatch(void)
{
	unsigned int extint0_ip = REG32(BSP_GIMR0_0) & REG32(BSP_GISR0_0);
	unsigned int extint1_ip = REG32(BSP_GIMR1_0) & REG32(BSP_GISR1_0);
	unsigned int extint2_ip = extint1_ip;
	int tmp, tmp1;


	extint0_ip >>= IRQ_LO_BOUND;
	extint1_ip >>= IRQ_LO_BOUND;
	extint2_ip >>= IRQ_LO_BOUND;
	
	for (tmp = IRQ_LO_BOUND; tmp <= IRQ_HI_BOUND; tmp++) {

		if (0 == extint0_ip)
			return;

		if((extint0_ip & 1) && (dispatch_table[tmp] >= 0) && tmp==12 ) {
			extint1_ip >>= PER_LO;
			for(tmp1= PER_LO; tmp1 <= PER_HI; tmp1++) {
				if (0 == extint1_ip)
				return;

				if ((extint1_ip & 1) && (dispatch_table[tmp1+IRQ_HI_BOUND+1] >= 0)) {

					do_IRQ(dispatch_table[tmp1+IRQ_HI_BOUND+1]);
				}
		
				extint1_ip >>= 1;
			}
		}
		else if((extint0_ip & 1) && (dispatch_table[tmp] >= 0) && tmp==31) {
			extint2_ip >>= TMO_LO;
			for(tmp1= TMO_LO; tmp1 <= TMO_HI; tmp1++) {
				if (0 == extint2_ip)
				return;

				if ((extint2_ip & 1) && (dispatch_table[tmp1+IRQ_HI_BOUND+1] >= 0)) {
					//prom_printf("tmp=%d, interrupt pin is %d\n",tmp,tmp1+IRQ_HI_BOUND+1);
					do_IRQ(dispatch_table[tmp1+IRQ_HI_BOUND+1]);
				}
		
				extint2_ip >>= 1;
			}
		}
		else if((extint0_ip & 1) && (dispatch_table[tmp] >= 0)) {
			//prom_printf("tmp=%d, interrupt pin is %d\n",tmp,tmp);
			do_IRQ(dispatch_table[tmp]);
		}

		extint0_ip >>= 1;
	}
}


/*linux-2.6.19*/
#define rtl8672_disable_irq bsp_ictl_irq_mask
#define rtl8672_enable_irq bsp_ictl_irq_unmask
#define RCR		0x44
#define ACCEPT_BROADCAST	0x08
#define ACCEPT_MYPHYS		0x02

//For ADSL driver refered
atomic_t activeFlag=ATOMIC_INIT(1);



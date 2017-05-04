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


static void bsp_ictl_irq_mask(unsigned int irq)
{	
    	REG32(BSP_GIMR) &= ~(1 << (irq));
}

static void bsp_ictl_irq_unmask(unsigned int irq)
{
    	REG32(BSP_GIMR) |= (1 << (irq));
}

static struct irq_chip bsp_ictl_irq = {
    .typename = "ICTL-RTL8681",
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
    REG32(BSP_GIMR) = 0;

    for (i=0; i < 32; i++) 
        set_irq_chip_and_handler( i, &bsp_ictl_irq, handle_level_irq);

   /* Enable all interrupt mask of CPU */
   write_c0_status(read_c0_status() | ST0_IM);

   /* Set GIMR, IRR */
#ifndef CONFIG_USE_UART1
   REG32(BSP_GIMR) = BSP_TC0_IE | BSP_UART0_IE;
#else
   REG32(BSP_GIMR) = BSP_TC0_IE | BSP_UART1_IE;
#endif   

   REG32(BSP_IRR0) = BSP_IRR0_SETTING;
   REG32(BSP_IRR1) = BSP_IRR1_SETTING;
   REG32(BSP_IRR2) = BSP_IRR2_SETTING;
   REG32(BSP_IRR3) = BSP_IRR3_SETTING;
#if defined(CONFIG_RTK_VOIP)
    REG32(BSP_GIMR) |= (BSP_PCM_IE );
  //#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
	REG32(BSP_GIMR) |= (BSP_GPIO_ABCD_IE | BSP_GPIO_EFGH_IE); 
  //#endif
#endif
}

#define IRQ_LO_BOUND 0
#define IRQ_HI_BOUND 31

static const char dispatch_table[32] = {
	-1,				// 0
	-1,			 	// 1
	BSP_OCPTMO_IRQ,		// 2
	BSP_LBCTMOm0_IRQ,	// 3
	BSP_LBCTMOm1_IRQ,	// 4
	BSP_LBCTMOs0_IRQ,		// 5
	BSP_LBCTMOs1_IRQ,	// 6
	-1,	// 7
	BSP_SAR_IRQ,	// 8
	BSP_PTM_IRQ,	// 9
	BSP_NFBI_IRQ,	// 10
	BSP_NIC100_IRQ,	// 11
	BSP_PKT_IRQ,	// 12
	BSP_PKT_SAR_IRQ,	// 13
	BSP_PKT_NIC100_IRQ,	// 14
	-1,				// 15
	BSP_TC0_IRQ,	// 16
	BSP_UART0_IRQ,	// 17
	BSP_UART1_IRQ,	// 18
	BSP_GPIO_ABCD_IRQ,	// 19
	BSP_SCHM_IRQ,	// 20
	BSP_CMU_IRQ,	// 21
	BSP_PowerL_IRQ,	// 22
	BSP_MDIO_IRQ,	// 23
	-1, //DMT_IRQ,	// 24
	-1,	// 25
	-1,	// 26
	-1,	// 27
	-1,	// 28
	-1,	// 29
	-1,	// 30
	-1,	// 31
};

__IRAM_SYS_MIDDLE void bsp_irq_dispatch(void)
{
	//unsigned int cpuint_ip = read_c0_cause() & read_c0_status() & ST0_IM;
	unsigned int extint_ip = REG32(BSP_GIMR) & REG32(BSP_GISR);
	int tmp;

	extint_ip >>= IRQ_LO_BOUND;
	for (tmp = IRQ_LO_BOUND; tmp <= IRQ_HI_BOUND; tmp++) {
		if (0 == extint_ip)
			return;

		if ((extint_ip & 1) && (dispatch_table[tmp] >= 0)) {
			do_IRQ(dispatch_table[tmp]);
		}
		
		extint_ip >>= 1;
	}
}



/*linux-2.6.19*/
#define rtl8672_disable_irq bsp_ictl_irq_mask
#define rtl8672_enable_irq bsp_ictl_irq_unmask

#ifdef CONFIG_RTL8672NIC
// Kaohj -- for Ethernet poll mode
extern int eth_poll;
#endif
static int orig_rxmode;

#define ETHBASE 0xb8012000

#define RTL_W32(reg, value)			(*(volatile u32*)(ETHBASE+reg)) = (u32)value
#define RTL_R32(reg)				(*(volatile u32*)(ETHBASE+reg))
#define RCR		0x44
#define ACCEPT_BROADCAST	0x08
#define ACCEPT_MYPHYS		0x02

//For ADSL driver refered
atomic_t activeFlag=ATOMIC_INIT(1);
#if defined (CONFIG_USB_RTL8187SU_SOFTAP) || defined (CONFIG_USB_RTL8192SU_SOFTAP)
	extern int wifi_poll;
	extern void kick_usb_poll_timer(void);
#endif
void SuspendDevices(void)
{
	//int rx_mode;

	if (!test_and_clear_bit(0, &activeFlag))
	    return;

	#ifdef CONFIG_RTL867X_PACKET_PROCESSOR	
	rtl8672_disable_irq(BSP_PKT_NIC100_IRQ);
	#ifndef CONFIG_RTL8681
	rtl8672_disable_irq(BSP_PKT_NIC100MII_IRQ);
	#endif
	rtl8672_disable_irq(BSP_PKT_SAR_IRQ);
	#else
	rtl8672_disable_irq(BSP_NIC100_IRQ);
	rtl8672_disable_irq(BSP_SAR_IRQ);
	#endif

	// save RCR
	orig_rxmode = RTL_R32(RCR);
	// Kaohj -- accept broadcast and myphy only
	//rx_mode = (ACCEPT_BROADCAST | ACCEPT_MYPHYS);
	//RTL_W32(RCR, rx_mode);
	// transfer to poll mode for rate control
	#ifdef CONFIG_RTL8672NIC
	eth_poll = 1;
	#endif
	
	#if defined (CONFIG_USB_RTL8187SU_SOFTAP) || defined (CONFIG_USB_RTL8192SU_SOFTAP)
		wifi_poll = 1;
		kick_usb_poll_timer();
		#ifdef CONFIG_USB_OTG_HOST_RTL8672
		rtl8672_disable_irq(BSP_USB_D_IRQ);	//otg host
		#else
		rtl8672_disable_irq(BSP_USB_H_IRQ);	//usb host
		#endif
	#endif

	#ifdef CONFIG_USB_ETH
	rtl8672_disable_irq(BSP_USB_D_IRQ);
	#endif

	#ifdef CONFIG_PCI//for RTL8190
	rtl8672_disable_irq(PCI_IRQ);
	#endif
}

//For ADSL driver refered
//enable devices IRQ and their tasklet, called by mm_task()
void ResumeDevices(void)
{
	if (test_and_set_bit(0, &activeFlag))
	    return;

	#ifdef CONFIG_RTL8672NIC
	// Kaohj -- return to interrupt mode
	eth_poll = 0;
	#endif
		
	#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
	rtl8672_enable_irq(BSP_PKT_NIC100_IRQ);
	#ifndef CONFIG_RTL8681
	rtl8672_enable_irq(BSP_PKT_NIC100MII_IRQ);
	#endif
	rtl8672_enable_irq(BSP_PKT_SAR_IRQ);	
	#else
	rtl8672_enable_irq(BSP_NIC100_IRQ);
	rtl8672_enable_irq(BSP_SAR_IRQ);
	#endif
	// restore RCR
	//RTL_W32(RCR, orig_rxmode);
	

	#if defined (CONFIG_USB_RTL8187SU_SOFTAP) || defined (CONFIG_USB_RTL8192SU_SOFTAP)
		wifi_poll = 0;
		#ifdef CONFIG_USB_OTG_HOST_RTL8672
		rtl8672_enable_irq(BSP_USB_D_IRQ);	//otg host
		#else
		rtl8672_enable_irq(BSP_USB_H_IRQ);	//usb host
		#endif
	#endif

	#ifdef CONFIG_USB_ETH
	rtl8672_enable_irq(BSP_USB_D_IRQ);
	#endif
	#ifdef CONFIG_PCI	//for RTL8190
	rtl8672_enable_irq(PCI_IRQ);
	#endif
}



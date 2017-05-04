#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>	/* jiffies is defined */
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/delay.h>


#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI) || defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI)
#define ZSI_ISI_PIN_SEL			( ( volatile unsigned int * )0xbb023018 )
#define ZSI_ISI_PIN_EN			( ( volatile unsigned int * )0xbb000174 )
#endif

#define ZSI_ISI_PIN_MODE		( ( volatile unsigned int * )0xB8000600 )
#define PLL_CLOCK_CONTROL       ( ( volatile unsigned int * )0xB8000204 )
#define FRACEN	(1<<3)

#ifdef CONFIG_RTL8685S
#define PINMUX	( ( volatile unsigned int * )0xB800010C )
#define FRACENREG	( ( volatile unsigned int * )0xB8000228 )
#define PCMEN	(1<<2)
#define BIT27	(1<<27)
#define BIT26	(1<<26)
#define BIT19	(1<<19)
#endif

#ifdef CONFIG_RTL8676
#define ZSI_ISI_PIN_MODE ( ( volatile unsigned int * )0xB800030C )
#define ZSI_ISI	((1<<18) | ( 1<<17))
#define FFT (1<<15)
#define ACC (1<<13)
#define PCM_EN (1<<8)
#define ZSI_FUNCTION	(0<<16)
#define	ISI_FUNCTION	(1<<16)
#endif
#define REG32(reg) (*(volatile unsigned int *)(reg))

#define PCMCNR ( ( volatile unsigned int * )0xB8008004 )

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI 
extern void setISIMSIF_LOCK(unsigned char cs);
#endif

void slicInternalCS(int cs){
	unsigned int regValue = 0;
	regValue = *PCMCNR;
	regValue |= (cs<<28);
#if (CONFIG_RTK_VOIP_CON_CH_NUM == 2)
	regValue |= (cs<<20);
#endif
	*PCMCNR = regValue;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	printk("cs%d\n", cs);
	setISIMSIF_LOCK(cs);
#endif
}

static void __init slicInterface(void){
	unsigned int regValue = 0;

#ifdef CONFIG_RTL8685	
	unsigned int reg_tmp = 0;
	REG32(0xbb804004) = 0x841f0bc0;
	mdelay(1);
	REG32(0xbb804004) = 0x04110000;
	mdelay(1);
	printk("page 0xBC0 reg17 = 0x%x \n\r",REG32(0xbb804008));
	reg_tmp = (REG32(0xbb804008)&0x0FFFF)|(1<<3)|0x84110000;

	mdelay(1);
	REG32(0xbb804004) = reg_tmp;
	mdelay(2);
	REG32(0xbb804004) = 0x04110000;
	mdelay(1);
	printk("page 0xBC0 reg17 = 0x%x \n\r",REG32(0xbb804008));     
#endif	
	
	
#ifdef CONFIG_RTL8685S
#if 0 // Move PINMUX setting to aipc
	regValue = *PINMUX;
	regValue |= (1<<16);
	*PINMUX	= regValue;
#endif


	{//for 8685S version B
		regValue = *FRACENREG;
		regValue &= ~BIT19;
		*FRACENREG = regValue;

		regValue = *FRACENREG;
		regValue |= BIT19;
		*FRACENREG = regValue;
	}

	regValue = *ZSI_ISI_PIN_MODE;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI 
	regValue |= BIT27;
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI 
	regValue |= BIT27;
	regValue |= BIT26;
#endif
	regValue |= PCMEN;
	*ZSI_ISI_PIN_MODE = regValue;
#endif

        regValue = *PLL_CLOCK_CONTROL;
        regValue |= FRACEN;
        *PLL_CLOCK_CONTROL = regValue;
	

//	printk("[%s] slic isi/zsi enable\n", __FUNCTION__);
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI 
#ifdef CONFIG_RTL8686
	regValue = *ZSI_ISI_PIN_SEL;
	regValue = regValue & 0xfff9ffff; 
	regValue |= (1<<16);
	*ZSI_ISI_PIN_SEL = regValue;
#endif
	
	regValue = *ZSI_ISI_PIN_EN;
	regValue |= (1<<2);
	*ZSI_ISI_PIN_EN = regValue;

	regValue = *ZSI_ISI_PIN_MODE;//writel(0xB8000600 , readl(0xB8000600) | (1<<26) | (1<<25) );	
#ifdef CONFIG_RTL8686
	regValue |= (1<<26);
#endif
#ifdef CONFIG_RTL8685
	regValue &= 0xf7ffffff;
	regValue |= (1<<27);
#endif
	*ZSI_ISI_PIN_MODE = regValue;
#if 0
	regValue = pcm_inl(0xb8008000);
	regValue |= ZSILBE;
	pcm_outl(0xb8008000, regValue);
#endif
#endif


#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI 
#ifdef CONFIG_RTL8686
	regValue = *ZSI_ISI_PIN_SEL;
	regValue = regValue & 0xfff9ffff; 
	regValue |= (1<<15);
	*ZSI_ISI_PIN_SEL = regValue;
#endif
	regValue = *ZSI_ISI_PIN_EN;
	regValue |= (1<<2);
	*ZSI_ISI_PIN_EN = regValue;
	
	regValue = *ZSI_ISI_PIN_MODE;//writel(0xB8000600 , readl(0xB8000600) | (1<<26) | (1<<25) );	
#ifdef CONFIG_RTL8686
	regValue |= (1<<26);
	regValue |= (1<<25);
#endif
#ifdef CONFIG_RTL8685
	regValue &= 0xf3ffffff;
	regValue |= (1<<26);
	regValue |= (1<<27);

#endif
	*ZSI_ISI_PIN_MODE = regValue;


#ifdef CONFIG_RTL8676
	
        regValue = *ZSI_ISI_PIN_MODE;

	regValue &= 0xffffdfff;
	regValue |= FFT;
	regValue |= ACC;
	regValue |= PCM_EN;

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI
	regValue |= ZSI_ISI;
	regValue &= 0xfffeffff;
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	regValue |= ZSI_ISI;
        regValue |= (1<<16);
#endif
        *ZSI_ISI_PIN_MODE = regValue;	
		
	printk("\n\nPINMUX = 0x[%x]\n\n", *ZSI_ISI_PIN_MODE);
#endif

#if 0
	regValue = pcm_inl(0xbb023018);
	regValue = regValue & 0xfff9ffff; 
	regValue |= (1<<15);
	pcm_outl(0xbb023018, regValue);
	
	regValue = pcm_inl(0xbb000174);
	regValue |= (1<<2);
	pcm_outl(0xbb000174 , regValue);

	pcm_outl(0xB8000600 , pcm_inl(0xB8000600) | (1<<26) | (1<<25) );	

	regValue = pcm_inl(0xb8008000);
	regValue |= ISILBE;
	pcm_outl(0xb8008000, regValue);
	printk("0xb8008000 = [%04x]\n", pcm_inl(0xb8008000));
#endif
#endif
}

module_init(slicInterface);

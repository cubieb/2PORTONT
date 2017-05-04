#ifdef _AIPC_CPU_
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/delay.h>

#if defined(CONFIG_DEFAULTS_KERNEL_3_18) && !defined(CONFIG_LUNA)
#include "bspchip.h"
#else
#include "gpio.h"
#endif
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
#include "rtk/gpio.h"
#endif
#include "bspchip.h"

#include "./include/aipc_reg.h"
#include "./include/soc_type.h"
#include "./include/aipc_mem.h"
#include "./include/aipc_debug.h"


extern unsigned int SOC_ID , SOC_ID_EXT , SOC_BOND_ID;
extern unsigned int GPIO_CTRL_0, GPIO_CTRL_1, GPIO_CTRL_2, GPIO_CTRL_4;
unsigned int soc_set_pcm_fs = 0;

#ifdef AIPC_MODULE_VOIP_SET_PCM_FS
// 6266
#define REG_IO_MODE  (0xBB023018)
#define BIT_I2C0  BIT(13)
#define PIN_FS    26
#define PIN_PCM   27

#define PLL_CLOCK_CONTROL      (0xB8000204)
#define FRACEN BIT(3)
//#define SYNCE_FEATURE   /* PON Clock or Free Run */

#define WSDS_ANA_24    (0xBB022090)
//#define REG_CMU_EN_TX

#define WSDS_ANA_18    (0xBB022060)
//#define REG_CMU_TX_OFF

#define WSDS_ANA_1A                (0xBB022068)
#define REG_DIVN_GPHY_REF_SHIFT    (5)
#ifdef SYNCE_FEATURE
#define REG_DIVN_GPHY_REF          (77 << REG_DIVN_GPHY_REF_SHIFT)
#else
#define REG_DIVN_GPHY_REF          (96 << REG_DIVN_GPHY_REF_SHIFT)
#endif
#define REG_DIVN_GPHY_REF_MASK     (~(0x1fe0))

#define WSDS_ANA_07                (0xBB02201C)
//#define REG_CMU_SEL_FREF  

#define WSDS_ANA_22                (0xBB022088)
#define REG_CMU_N_PLL_TX_SHIFT     (2)
#ifdef SYNCE_FEATURE
#define REG_CMU_N_PLL_TX           (76 << REG_CMU_N_PLL_TX_SHIFT)
#else
#define REG_CMU_N_PLL_TX           (112 << REG_CMU_N_PLL_TX_SHIFT)
#endif
#define REG_CMU_N_PLL_TX_MASK      (~(0x3fc))

#define WSDS_ANA_23                (0xBB02208C)
#define REG_DIVN_POST_PCM_SHIFT    (16)
#ifdef SYNCE_FEATURE
#define REG_DIVN_POST_PCM          (96 << REG_DIVN_POST_PCM_SHIFT)
#else
#define REG_DIVN_POST_PCM          (114 << REG_DIVN_POST_PCM_SHIFT)
#endif
#define REG_DIVN_POST_PCM_MASK     (~(0x00ff0000))

#define WSDS_ANA_15                (0xBB022054)
//#define REG_PREDIV_BYPASS_TX

// 6318
#define REG_8685_PIN_STATUS         (0xB8000100)
#define BIT_BONDTYE                 BIT(27)

#define REG_8685_PINMUX_0           (0xB8000108)
#define BIT_UART1_SEL               BIT(11)

#define REG_8685_PINMUX_1			(0xB800010C)
#define BIT_VOIP_SPI_DIR_256_PIN	BIT(16)
#define BIT_VOIP_SPI_DIR_176_PIN	BIT(15)


// 0639
#define REG_0639_IO_MODE_EN         (0xbb023004)
#define REG_0639_VOIP_EN_MASK       (~(0x03ffff00))
#define REG_0639_VOIP_EN_SHIFT      (8)
#define REG_0639_DIS_TX_EN          (0x2)
#define REG_0639_PCMSPI             ((0x3ffff)<<REG_0639_VOIP_EN_SHIFT)
//#define REG_0639_ZSI              ((0x2aaaa)<<REG_0639_VOIP_EN_SHIFT)
#define REG_0639_ZSI                ((0x2aaa)<<REG_0639_VOIP_EN_SHIFT)
//#define REG_0639_ISI              ((0x15555)<<REG_0639_VOIP_EN_SHIFT)
#define REG_0639_ISI                ((0x1555)<<REG_0639_VOIP_EN_SHIFT)


// 0705
#define REG_0705_PINMUX	            (0xB800010C)

// 0676 : 9602C
#define REG_SDS_CFG	0xBB0001D0
#define FIB_1G		BIT(2)

#define REG_FRC		0xBB022500
#define FRC_RX_EN_ON	BIT(4)
#define FRC_RX_EN_VAL	BIT(5)
#define FRC_CMU_EN_ON	BIT(10)
#define FRC_CMU_EN_VAL	BIT(11)

#define REG_REG42	0xBB0226A8
#define REG_PCM_CMU_EN	BIT(2)


void
aipc_module_voip_set_pcm_fs(void)
{
	unsigned int rtl8685_bond_type = 0;
	
		// 6318 , 8685
		if (SOC_ID == 0x0561) {
			rtl8685_bond_type = REG32(REG_8685_PIN_STATUS);
			
			REG32(REG_8685_PINMUX_0) &= ~BIT_UART1_SEL;
			SDEBUG( "REG_8685_PINMUX_0 0x%08x\n" , REG32(REG_8685_PINMUX_0));
			
			if ((rtl8685_bond_type & BIT_BONDTYE) == BIT_BONDTYE) {
				REG32(REG_8685_PINMUX_1) |= BIT_VOIP_SPI_DIR_256_PIN;
			}
			else {
				REG32(REG_8685_PINMUX_1) |= BIT_VOIP_SPI_DIR_176_PIN;
			}
			SDEBUG( "REG_8685_PIN_STATUS 0x%08x\n" , REG32(REG_8685_PIN_STATUS) );
			SDEBUG( "REG_8685_PINMUX_1 0x%08x\n" , REG32(REG_8685_PINMUX_1) );
		}
		// 8685S
		else if( (SOC_ID == 0x0705) || (SOC_ID == 0x6405) ){
			REG32(REG_0705_PINMUX) |= BIT(16);
			SDEBUG( "REG_0705_PINMUX 0x%08x\n" , REG32(REG_0705_PINMUX) );
		}
		// 0639 , 6422 , 9601b
		else if( (SOC_ID == 0x0639) || (SOC_ID == 0x6422) ){
			SDEBUG( "REG_0639_IO_MODE_EN 0x%08x\n" , REG32(REG_0639_IO_MODE_EN));

			REG32(REG_0639_IO_MODE_EN) = (REG32(REG_0639_IO_MODE_EN) & REG_0639_VOIP_EN_MASK) 
#if   defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZSI)
										| REG_0639_ZSI
#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI)
										| REG_0639_ISI
#else
										| REG_0639_PCMSPI
#endif

#if   defined(CONFIG_RTL8686_SLIC_RESET_GPIO_NUM) 
#if   (CONFIG_RTL8686_SLIC_RESET_GPIO_NUM >= 2) && (CONFIG_RTL8686_SLIC_RESET_GPIO_NUM <= 6)
										| REG_0639_DIS_TX_EN
#endif
#endif
										;

			SDEBUG( "REG_0639_IO_MODE_EN 0x%08x\n" , REG32(REG_0639_IO_MODE_EN));
		}
		// 6266 , 9607 , 8696
		else if( SOC_ID == 0x6266 ){

			if (SOC_ID_EXT  >= 0x10) {
#if !defined(CONFIG_GPON_FEATURE) && !defined(CONFIG_EPON_FEATURE)
				REG32(WSDS_ANA_1A)  = (REG32(WSDS_ANA_1A) & REG_DIVN_GPHY_REF_MASK) | REG_DIVN_GPHY_REF;
				SDEBUG( "WSDS_ANA_1A 0x%08x 0x%08x\n" , WSDS_ANA_1A , REG32(WSDS_ANA_1A) ); 

				REG32(WSDS_ANA_07) &= ~BIT(7);
				SDEBUG( "WSDS_ANA_07 0x%08x 0x%08x\n" , WSDS_ANA_07 , REG32(WSDS_ANA_07) ); 

				REG32(WSDS_ANA_22)  = (REG32(WSDS_ANA_22) & REG_CMU_N_PLL_TX_MASK) | REG_CMU_N_PLL_TX;
				SDEBUG( "WSDS_ANA_22 0x%08x 0x%08x\n" , WSDS_ANA_22 , REG32(WSDS_ANA_22) ); 

				REG32(WSDS_ANA_23)  = (REG32(WSDS_ANA_23) & REG_DIVN_POST_PCM) | REG_DIVN_POST_PCM;
				SDEBUG( "WSDS_ANA_23 0x%08x 0x%08x\n" , WSDS_ANA_23 , REG32(WSDS_ANA_23) ); 

				REG32(WSDS_ANA_15) |= BIT(11);
				SDEBUG( "WSDS_ANA_15 0x%08x 0x%08x\n" , WSDS_ANA_15 , REG32(WSDS_ANA_15) ); 
#endif
				REG32(WSDS_ANA_24) |=  BIT(3);
				REG32(WSDS_ANA_18) &= ~BIT(7);				
				SDEBUG( "WSDS_ANA_24 0x%08x 0x%08x\n" , WSDS_ANA_24 , REG32(WSDS_ANA_24));
				SDEBUG( "WSDS_ANA_18 0x%08x 0x%08x\n" , WSDS_ANA_18 , REG32(WSDS_ANA_18));
			}

			if (SOC_BOND_ID == CHIP_96) {
				// disable I2C0
				SDEBUG( "REG IO_MODE %x\n" , REG32(REG_IO_MODE) );
				REG32(REG_IO_MODE) &= ~(BIT_I2C0);	
				SDEBUG( "REG IO_MODE %x\n" , REG32(REG_IO_MODE) );
			}

			// disalbe GPIO 26 , 27 for FS and PCM respectively
			REG32(GPIO_CTRL_2) &= ~(1<<(PIN_FS  - 1));
			REG32(GPIO_CTRL_2) &= ~(1<<(PIN_PCM - 1));
		}else if( SOC_ID == 0x0676 ){
#if !defined(CONFIG_GPON_FEATURE) && !defined(CONFIG_EPON_FEATURE)
			REG32(REG_SDS_CFG) = FIB_1G;
			REG32(REG_FRC) &= ~(FRC_RX_EN_VAL|FRC_RX_EN_VAL);
			REG32(REG_FRC) |= (FRC_RX_EN_ON|FRC_CMU_EN_ON);
			REG32(REG_REG42) |= (REG_PCM_CMU_EN);
			REG32(REG_REG42) &= ~(REG_PCM_CMU_EN);
			REG32(REG_REG42) |= (REG_PCM_CMU_EN);
			SDEBUG( "REG_SDS_CFG 0x%08x 0x%08x\n", REG_SDS_CFG, REG32(REG_SDS_CFG));
			SDEBUG( "REG_FRC     0x%08x 0x%08x\n", REG_FRC, REG32(REG_FRC));
			SDEBUG( "REG_REG42   0x%08x 0x%08x\n", REG_REG42, REG32(REG_REG42));
#endif
		}
		else {
    		SDEBUG( "SOC_ID 0x%04x is not handled\n" , SOC_ID );
		}
}

EXPORT_SYMBOL(aipc_module_voip_set_pcm_fs);
#endif

#ifdef CONFIG_RTL8686_SLIC_RESET
#if (CONFIG_RTL8686_SLIC_RESET_GPIO_NUM < 0) || (CONFIG_RTL8686_SLIC_RESET_GPIO_NUM > 71)
	#error "Wrong GPIO Number Setting"
#endif

#define SWITCH_GPIO_CTRL_0	(0xBB000100)
	#define SWITCH_GPIO_DATA_00_31	(SWITCH_GPIO_CTRL_0+0x0)
	#define SWITCH_GPIO_DATA_32_63	(SWITCH_GPIO_CTRL_0+0x4)
	#define SWITCH_GPIO_DATA_64_71	(SWITCH_GPIO_CTRL_0+0x8)

#define SWITCH_GPIO_CTRL_1	(0xBB00010C)

#define SWITCH_GPIO_CTRL_2	(0xBB000118)
	#define SWITCH_GPIO_ENABLE_00_31	(SWITCH_GPIO_CTRL_2+0x0)
	#define SWITCH_GPIO_ENABLE_32_63	(SWITCH_GPIO_CTRL_2+0x4)
	#define SWITCH_GPIO_ENABLE_64_71	(SWITCH_GPIO_CTRL_2+0x8)

#define SWITCH_GPIO_CTRL_3	(0xBB000124)

#define SWITCH_GPIO_CTRL_4	(0xBB0001D4)
	#define SWITCH_GPIO_DIR_00_31		(SWITCH_GPIO_CTRL_4+0x0)
	#define SWITCH_GPIO_DIR_32_63		(SWITCH_GPIO_CTRL_4+0x4)
	#define SWITCH_GPIO_DIR_64_71		(SWITCH_GPIO_CTRL_4+0x8)

void
aipc_module_voip_slic_reset_0371(void)
{
	int count = 0;
	unsigned int gpio_data = 0, gpio_dir = 0 , gpio_enable = 0;

	SDEBUG( "\n" );

	if ((CONFIG_RTL8686_SLIC_RESET_GPIO_NUM >= 0) && (CONFIG_RTL8686_SLIC_RESET_GPIO_NUM <= 31)){
		count = CONFIG_RTL8686_SLIC_RESET_GPIO_NUM-1;

		gpio_data   = SWITCH_GPIO_DATA_00_31 ;
		gpio_dir    = SWITCH_GPIO_DIR_00_31 ;
		gpio_enable = SWITCH_GPIO_ENABLE_00_31 ;
	}
	else if ((CONFIG_RTL8686_SLIC_RESET_GPIO_NUM >= 32) && (CONFIG_RTL8686_SLIC_RESET_GPIO_NUM <= 63)){
		count = CONFIG_RTL8686_SLIC_RESET_GPIO_NUM % 32;

		gpio_data   = SWITCH_GPIO_DATA_32_63 ;
		gpio_dir    = SWITCH_GPIO_DIR_32_63 ;
		gpio_enable = SWITCH_GPIO_ENABLE_32_63 ;
	}
	else if ((CONFIG_RTL8686_SLIC_RESET_GPIO_NUM >= 64) && (CONFIG_RTL8686_SLIC_RESET_GPIO_NUM <= 71)){
		count = CONFIG_RTL8686_SLIC_RESET_GPIO_NUM % 32;
	
		gpio_data   = SWITCH_GPIO_DATA_64_71 ;
		gpio_dir    = SWITCH_GPIO_DIR_64_71 ;
		gpio_enable = SWITCH_GPIO_ENABLE_64_71 ;
	}
	else{
		SDEBUG("Wrong GPIO Number Setting\n");
	}

	SDEBUG( "Pull low GPIO %d in switch. count = %d\n" , CONFIG_RTL8686_SLIC_RESET_GPIO_NUM , count );
    SDEBUG( "data %x = 0x%x dir %x = 0x%x enable %x = 0x%x\n" , 
    		gpio_data    ,  REG32(gpio_data) , 
			gpio_dir     ,  REG32(gpio_dir) , 
    		gpio_enable  ,  REG32(gpio_enable));

    REG32(gpio_data)	&= ~(1<<(count));  //data
    REG32(gpio_dir)		|=  (1<<(count));  //dir
    REG32(gpio_enable)	|=  (1<<(count));  //enable

	mdelay(RESET_SLIC_DELAY_TIME);

    REG32(gpio_data) 	|=  (1<<(count));
    
	SDEBUG( "Pull high GPIO %d in switch\n" , CONFIG_RTL8686_SLIC_RESET_GPIO_NUM );
    SDEBUG( "data %x = 0x%x dir %x = 0x%x enable %x = 0x%x\n" , 
    		gpio_data    ,  REG32(gpio_data) , 
			gpio_dir     ,  REG32(gpio_dir) , 
    		gpio_enable  ,  REG32(gpio_enable));
}

#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
void
aipc_module_voip_slic_reset_rtk_api(void)
{
	uint32 slic_reset = CONFIG_RTL8686_SLIC_RESET_GPIO_NUM;
    
	SDEBUG( "Pull low GPIO %d\n" , CONFIG_RTL8686_SLIC_RESET_GPIO_NUM );
	rtk_gpio_mode_set( slic_reset , GPIO_OUTPUT);
	rtk_gpio_databit_set( slic_reset , 0 );
	rtk_gpio_state_set( slic_reset , ENABLED); 

	mdelay(RESET_SLIC_DELAY_TIME);
	
	rtk_gpio_databit_set( slic_reset , 1 );
	
	SDEBUG( "Pull high GPIO %d\n" , CONFIG_RTL8686_SLIC_RESET_GPIO_NUM );
}
#endif

void
aipc_module_voip_slic_reset_6266(void)
{
	int slic_reset = CONFIG_RTL8686_SLIC_RESET_GPIO_NUM;
	int value = 0;

	SDEBUG( "\n" );
    SDEBUG( "Pull low GPIO %d in switch\n" , CONFIG_RTL8686_SLIC_RESET_GPIO_NUM );
	
	gpioClear( slic_reset );
	gpioConfig( slic_reset, GPIO_FUNC_OUTPUT );
	value = gpioRead( slic_reset );
    SDEBUG( "GPIO value = %d\n" , value );
	
	mdelay(RESET_SLIC_DELAY_TIME);
    
    SDEBUG( "Pull high GPIO %d in switch\n" , CONFIG_RTL8686_SLIC_RESET_GPIO_NUM );
	gpioSet( slic_reset );
	
	value = gpioRead( slic_reset );
    SDEBUG( "GPIO value = %d\n" , value );
}

void
aipc_module_voip_slic_reset_others(void)
{
	int slic_reset = CONFIG_RTL8686_SLIC_RESET_GPIO_NUM;
	int value = 0;

	SDEBUG( "\n" );
    SDEBUG( "Pull low GPIO %d in switch\n" , CONFIG_RTL8686_SLIC_RESET_GPIO_NUM );
	
	gpioClear( slic_reset );
	gpioConfig( slic_reset, GPIO_FUNC_OUTPUT );
	value = gpioRead( slic_reset );
    SDEBUG( "GPIO value = %d\n" , value );
	
	mdelay(RESET_SLIC_DELAY_TIME);
    
    SDEBUG( "Pull high GPIO %d in switch\n" , CONFIG_RTL8686_SLIC_RESET_GPIO_NUM );
	gpioSet( slic_reset );
	
	value = gpioRead( slic_reset );
    SDEBUG( "GPIO value = %d\n" , value );
}

void
aipc_module_voip_slic_reset(void)
{
	switch( SOC_ID ){
	case 0x0371:
		aipc_module_voip_slic_reset_0371();
		break;
#if defined(CONFIG_GPON_FEATURE) || defined(CONFIG_EPON_FEATURE)
	case 0x0639:  //9601b test
	case 0x6422:  //9601b formal
		aipc_module_voip_slic_reset_rtk_api();
		break;
#endif
	case 0x6266:
	case 0x6318:  //8685 formal
	case 0x0561:  //8685 test
	case 0x0705:  //8685S test
		aipc_module_voip_slic_reset_6266();
		break;
	default:
		aipc_module_voip_slic_reset_others();
	}
}
EXPORT_SYMBOL(aipc_module_voip_slic_reset);
#endif


#ifdef  AIPC_MODULE_DISALBE_WDOG
#define WDOG_ENABLE 0xb8003224
void
aipc_module_disable_wdog(void)
{
	REG32(WDOG_ENABLE) &= ~WATCHDOG_ENABLE;
//	SDEBUG("WDOG Setting 0x%08x\n" , REG32(WDOG_ENABLE));
}
#endif

#ifdef AIPC_MODULE_VOIP_IP_ENABLE
void
aipc_module_voip_ip_enable(void)
{
	if ( SOC_ID == 0x6266 && SOC_ID_EXT == 0x0 ){
		REG32(PLL_CLOCK_CONTROL) &= ~(FRACEN);
		REG32(PLL_CLOCK_CONTROL) |= (FRACEN);
	}

	REG32(R_AIPC_IP_ENABLE_CTRL) |= BIT_IP_ENABLE_VOIPFFT |     \
									BIT_IP_ENABLE_VOIPACC |     \
									/* BIT_IP_ENABLE_GDMA1 | */ \
									/* BIT_IP_ENABLE_GDMA0 | */ \
									BIT_IP_ENABLE_PCM |         \
									/* BIT_IP_ENABLE_GMAC | */  \
									BIT_IP_ENABLE_PREI_VOIP;

	mdelay(100);
}
#endif

#if 0
#define PCM_CTR         0xb8008000
#define BIT_PRECISE     BIT(10)

static void
aipc_module_set_pcm_precise(int enable)
{
	if (enable)
		REG32(PCM_CTR) |=  BIT_PRECISE;
	else
		REG32(PCM_CTR) &= ~BIT_PRECISE;
}
#endif

#endif


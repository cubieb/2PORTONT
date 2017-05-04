#include "rtk_voip.h"
#include "voip_init.h"
#include "voip_addrspace.h"

#ifndef NO_SPECIAL_ADDRSPACE
#if   defined(CONFIG_LUNA_MEMORY_AUTO_DETECTION) && defined(CONFIG_RTL8686_DSP_MEM_SIZE)
#include <bsp_automem.h>
/* Determined by prom_init@BSP, BSP_DSP_MEM_BASE */
#define  physical_addr_offset bsp_mem_map.BSP_DSP_MEM_BASE;
#elif   defined(CONFIG_RTL8686_DSP_MEM_BASE)
unsigned long physical_addr_offset = CONFIG_RTL8686_DSP_MEM_BASE;
#elif defined(CONFIG_RTK_VOIP_2_PHYSICAL_OFFSET)
unsigned long physical_addr_offset = CONFIG_RTK_VOIP_2_PHYSICAL_OFFSET;
#endif
#endif


#if defined CONFIG_RTK_VOIP_DRIVERS_PCM8186
#if RTL8186PV_RELAY_USE_Z  // For Z-version board 8186PV
#define RELAY_SW_CTRL_GPIOC	//for gpioC use. pull relay high. for Z-version board 1*FXS 1*FXO.
#else
#if (!defined (CONFIG_RTK_VOIP_DRIVERS_PCM8186V_OC)) && (!defined (CONFIG_RTK_VOIP_DRIVERS_PCM8651_2S_OC))
#ifdef CONFIG_RTK_VOIP_DRIVERS_8186V_ROUTER
#define RELAY_SW_CTRL_GPIOD	//for gpioD used.pull relay high. for new 4-LAN EV board. 2006-07.
#else
#define RELAY_SW_CTRL_GPIOE	//for gpioE used.pull relay high.
#endif
#endif
#endif
#endif


static int __init voip_con_start_setup_init( void )
{
#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
	extern void rtl8972B_hw_init(int mode);
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
	extern void rtl8954C_hw_init(int mode);
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxD
	extern void rtl8972D_hw_init(int mode);
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672
	extern void rtl8672_hw_init(void);
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8676
	extern void rtl8676_hw_init(int mode);
#endif
	
	extern void start_os_timer( void );
	extern void init_rtl_hw_spi_IP( void );
	
#ifdef  RELAY_SW_CTRL_GPIOC//for gpioC used.pull relay high.	
	#define GPCD_DIR  *((volatile unsigned int *)0xbd010134)
	#define GPCD_DATA  *((volatile unsigned int *)0xbd010130)	
	BOOT_MSG("GPCD_DIR = %x\n",GPCD_DIR);
	GPCD_DIR = GPCD_DIR | 0x01; 
	BOOT_MSG("GPCD_DIR = %x\n",GPCD_DIR);
	GPCD_DATA = GPCD_DATA & 0xfffffffe;
#endif

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY
    #ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
	rtl8972B_hw_init(1); /* need init iis */
    #else
	rtl8972B_hw_init(0); /* just init pcm */
    #endif
#endif /* CONFIG_RTK_VOIP_DRIVERS_PCM8972B_FAMILY */

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxC
    #ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
	rtl8954C_hw_init(1); /* need init iis */
    #else
	rtl8954C_hw_init(0); /* just init pcm */
    #endif
#endif /* CONFIG_RTK_VOIP_DRIVERS_PCM89xxC */

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM89xxD
    #ifdef CONFIG_RTK_VOIP_DRIVERS_IIS
	rtl8972D_hw_init(1); /* need init iis */
    #else
	rtl8972D_hw_init(0); /* just init pcm */
    #endif
#endif /* CONFIG_RTK_VOIP_DRIVERS_PCM89xxD */

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8672
	rtl8672_hw_init();
#endif /* CONFIG_RTK_VOIP_DRIVERS_PCM8672 */

#ifdef CONFIG_RTK_VOIP_DRIVERS_PCM8676
#ifdef CONFIG_RTK_VOIP_8676_ISI_ZSI
	//rtl8676_hw_init(1); /* normal + ISI/ZSI */	// disable it, handle by 8676 bsp setting
#else
	//rtl8676_hw_init(0); /* normal */		// disable it, handle by 8676 bsp setting
#endif
#endif /* CONFIG_RTK_VOIP_DRIVERS_PCM8676 */
	
	start_os_timer();

//#ifdef CONFIG_RTK_VOIP_IPC_ARCH_IS_HOST
#ifdef CONFIG_RTK_VOIP_HARDWARE_SPI	
	init_rtl_hw_spi_IP();
#endif
//#endif
	return 0;
}

voip_initcall_entry( voip_con_start_setup_init );


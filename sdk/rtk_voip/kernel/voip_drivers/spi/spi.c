//#include <linux/config.h>
#include <linux/sched.h>	/* jiffies is defined */
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/version.h>

#include "rtk_voip.h"
#include "voip_init.h"
#include "voip_proc.h"

#include "gpio/gpio.h"
#include "spi.h"

static inline void printk_off( const char *fmt, ... ) {}
void init_rtl_hw_spi_IP( void );
void gpioResetSLIC(void);
#define SPI_PRINTK		printk_off
//#define SPI_PRINTK		printk

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
#define ISI_LOOP 100
static int isi_count = 0;
static unsigned char MSIF_LOCK = 0;
#endif

// ------------------------------------------------------------------------------
// Software SPI (GPIO)
// ------------------------------------------------------------------------------
#ifdef CONFIG_RTK_VOIP_SOFTWARE_SPI

#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)
#define __udelay_val cpu_data[smp_processor_id()].udelay_val
static __inline__ void __nsdelay(unsigned long nssecs, unsigned long lpj)
{
	/* This function is a copy of __udelay() in linux-2.4.18/include/asm-mips/delay.h */
    unsigned long lo;

    //usecs *= 0x00068db8;        /* 2**32 / (1000000 / HZ) */
    nssecs *= 0x000001AE;         /* 2**32 / (1000000 * 1000 / HZ) */
    __asm__("multu\t%2,%3"
        :"=h" (nssecs), "=l" (lo)
        :"r" (nssecs),"r" (lpj));
    __delay(nssecs);
}

static inline void spi_nsdelay( unsigned long delay )
{
	__nsdelay( delay, __udelay_val );	// __udelay_val defined in linux-2.4.18/include/asm-mips/delay.h
}
#undef spi_rw_delay 
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,30))
#if (__GNUC__ == 4) && (__GNUC_MINOR__ == 4)  //RSDK 1.5.4 4.4
#define spi_rw_delay() __ndelay(300)	//data clock period MIN 48ns. GPIO delay latency may large then 122 ns
#else
#define spi_rw_delay() __ndelay(300)	//data clock period MIN 48ns. GPIO delay latency may large then 122 ns
#endif
#define spi_cs_delay() __ndelay(2200)	//chip select off time MIN 2500ns
#else
#define spi_rw_delay() spi_nsdelay(200) 
#define spi_cs_delay() spi_nsdelay(2200) 
#endif

/******************* SPI raw read/write API *************************/
static int32 _rtl_sw_spi_rawRead( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint8* pch = pData;
	*pch = 0;

	if ( pData == NULL ) return FAILED;

	RTK_GPIO_SET( pDev->sw.gpioCS_, 0 ); /* fall down the CS_ */
	RTK_GPIO_SET( pDev->sw.gpioSCLK, 0 ); /* fall down the SCLK */
	spi_rw_delay(); /* delay for a while */
	//RTK_GPIO_SET( pDev->gpioSCLK, 0 ); /* fall down the SCLK */

	for( bits--; bits >= 0; bits-- )
	{
		uint32 buf;

		RTK_GPIO_SET( pDev->sw.gpioSCLK, 0 ); /* raising up the SCLK */
		spi_rw_delay(); /* delay for a while */

		pch[bits/8] &= ~((uint8)1<<(bits&0x7));
		RTK_GPIO_GET( pDev->sw.gpioSDI, &buf );
		pch[bits/8] |= buf?((uint8)1<<(bits&0x7)):0;
		
		RTK_GPIO_SET( pDev->sw.gpioSCLK, 1 ); /* fall down the SCLK */
		spi_rw_delay(); /* delay for a while */
	}	
	
	RTK_GPIO_SET( pDev->sw.gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->sw.gpioSCLK, 1 ); /* raise the SCLK */
	
	spi_cs_delay(); /* delay for a while >2500 ns */
	
	return SUCCESS;
}

static int32 _rtl_sw_spi_rawWrite( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint8* pch = pData;

	if ( pData == NULL ) return FAILED;

	RTK_GPIO_SET( pDev->sw.gpioSCLK, 0 ); /* fall down SCLK */
	RTK_GPIO_SET( pDev->sw.gpioCS_, 0 ); /* fall down the CS_ */

	for( bits-- ; bits >= 0; bits-- )
	{
		RTK_GPIO_SET( pDev->sw.gpioSDO, (pch[bits/8]&((uint32)1<<(bits&0x7)))?1:0 );
		spi_rw_delay(); /* delay for a while */
		RTK_GPIO_SET( pDev->sw.gpioSCLK, 1 ); /* raising up the SCLK */
		spi_rw_delay(); /* delay for a while */
		RTK_GPIO_SET( pDev->sw.gpioSCLK, 0 ); /* fall down the SCLK */
	}	
	
	spi_rw_delay(); /* delay for a while */
	RTK_GPIO_SET( pDev->sw.gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->sw.gpioSCLK, 1 ); /* raise the SCLK */
	
	spi_cs_delay(); /* delay for a while >2500 ns */

	return SUCCESS;

}

#else

#ifndef spi_rw_delay
#define spi_rw_delay()	// dummy 
#endif

static int32 _rtl_sw_spi_rawRead( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint8* pch = pData;
	
	*pch = 0;

	if ( pData == NULL ) return FAILED;

	RTK_GPIO_SET( pDev->sw.gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->sw.gpioSCLK, 1 ); /* raise the SCLK */
	spi_rw_delay(); /* delay for a while */

	RTK_GPIO_SET( pDev->sw.gpioCS_, 0 ); /* fall down the CS_ */
	spi_rw_delay(); /* delay for a while */

	for( bits--; bits >= 0; bits-- )
	{
		uint32 buf;

		RTK_GPIO_SET( pDev->sw.gpioSCLK, 0 ); /* fall down the SCLK */
		spi_rw_delay(); /* delay for a while */

		pch[bits/8] &= ~((uint8)1<<(bits&0x7));
		RTK_GPIO_GET( pDev->sw.gpioSDI, &buf );
		pch[bits/8] |= buf?((uint8)1<<(bits&0x7)):0;
		
		RTK_GPIO_SET( pDev->sw.gpioSCLK, 1 ); /* raising up the SCLK */
		spi_rw_delay(); /* delay for a while */
	}	
	
	RTK_GPIO_SET( pDev->sw.gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->sw.gpioSCLK, 1 ); /* raise the SCLK */
	
	return SUCCESS;
}

static int32 _rtl_sw_spi_rawWrite( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint8* pch = pData;

	if ( pData == NULL ) return FAILED;

	RTK_GPIO_SET( pDev->sw.gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->sw.gpioSCLK, 1 ); /* raise the SCLK */
	spi_rw_delay(); /* delay for a while */

	RTK_GPIO_SET( pDev->sw.gpioCS_, 0 ); /* fall down the CS_ */
	spi_rw_delay(); /* delay for a while */

	for( bits-- ; bits >= 0; bits-- )
	{
		RTK_GPIO_SET( pDev->sw.gpioSDO, (pch[bits/8]&((uint32)1<<(bits&0x7)))?1:0 );
		RTK_GPIO_SET( pDev->sw.gpioSCLK, 0 ); /* fall down the SCLK */
		spi_rw_delay(); /* delay for a while */
		RTK_GPIO_SET( pDev->sw.gpioSCLK, 1 ); /* raising up the SCLK */
		spi_rw_delay(); /* delay for a while */
	}	
	
	RTK_GPIO_SET( pDev->sw.gpioCS_, 1 ); /* raise the CS_ */
	RTK_GPIO_SET( pDev->sw.gpioSCLK, 1 ); /* raise the SCLK */
	
	return SUCCESS;

}
#endif //CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK

int32 _init_rtl_spi_dev_type_sw( rtl_spi_dev_t* pDev, 
			uint32 gpioCS_, uint32 gpioSCLK, uint32 gpioSDO, uint32 gpioSDI, int32 gpioINT )
{
	pDev ->type = SPI_TYPE_SW;
	
	pDev ->sw.gpioSCLK = gpioSCLK;
	pDev ->sw.gpioCS_ = gpioCS_;
	pDev ->sw.gpioSDI = gpioSDI;
	pDev ->sw.gpioSDO = gpioSDO;
#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
	pDev ->sw.gpioINT = gpioINT;
#endif
	//pDev->SClkDelayLoop = SysClock / maxSpeed;
	
	_rtl_generic_initGpioPin( gpioSCLK, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl_generic_initGpioPin( gpioCS_, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	_rtl_generic_initGpioPin( gpioSDI, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_DISABLE );
	_rtl_generic_initGpioPin( gpioSDO, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
	_rtl_generic_initGpioPin( gpioINT, GPIO_CONT_GPIO, GPIO_DIR_IN, GPIO_INT_BOTH_EDGE ); //Enable both falling or rising edge interrupt
#endif
	
	return SUCCESS;
}

#endif // CONFIG_RTK_VOIP_SOFTWARE_SPI

// ------------------------------------------------------------------------------
// Hardware SPI 
// ------------------------------------------------------------------------------
#ifdef CONFIG_RTK_VOIP_HARDWARE_SPI

#if defined( CONFIG_RTK_VOIP_PLATFORM_8686 )
#define SPI_IP_VERSION		0x05	// 0.5 
#define SPI_BASE_ADDR		0xB8009000
#else
#error "unknwon hardware SPI version"
#endif

// 0x00: SPI Control Register 
#define SPICNR_ADDR			( SPI_BASE_ADDR + 0x00 )
#define pSPICNR				( ( volatile uint32 * )SPICNR_ADDR )
	#define SPICNR_CSTIME			0x04000000	// bit 26 
	#define SPICNR_CSTIME_8BIT		0x00000000
	#define SPICNR_CSTIME_16BIT		SPICNR_CSTIME
	#define SPICNR_WRIE				0x02000000	// bit 25
	#define SPICNR_RDIE				0x01000000	// bit 24
	#define SPICNR_ADD_MASK			0x00FF0000	// bit 23:16
	#define SPICNR_ADD_SHIFT		16
	#define SPICNR_CTRL_MASK		0x0000FF00	// bit 15:8
	#define SPICNR_CTRL_SHIFT		8
	#define SPICNR_LSB				0x00000080	// bit 7
	#define SPICNR_MSB				0x00000000
	#define SPICNR_CMD				0x00000020	// bit 5
	#define SPICNR_CMD_READ			0x00000000
	#define SPICNR_CMD_WRITE		SPICNR_CMD
	#define SPICNR_START			0x00000010	// bit 4
	#define SPICNR_SCLK_TYPE		0x00000008	// bit 3
	#define SPICNR_SCLK_TYPE_I		0x00000000
	#define SPICNR_SCLK_TYPE_II		SPICNR_SCLK_TYPE
	#define SPICNR_CSP				0x00000002	// bit 1
	#define SPICNR_CSP_LOW_ACTIVE	0x00000000
	#define SPICNR_CSP_HIGH_ACTIVE	SPICNR_CSP
// 0x04: SPI Status Register 
#define SPISTR_ADDR			( SPI_BASE_ADDR + 0x04 )
#define pSPISTR				( ( volatile uint32 * )SPISTR_ADDR )
	#define SPISTR_RDIP				0x80000000	// bit 31
	#define SPISTR_WDIP				0x40000000	// bit 30
	#define SPISTR_MSIF_LOCK		0x3F000000	// bit 29:24
// 0x08: SPI Clock Divisor Register 
#define SPICKDIV_ADDR		( SPI_BASE_ADDR + 0x08 )
#define pSPICKDIV			( ( volatile uint32 * )SPICKDIV_ADDR )
	#define SPICKDIV_DIV			0xFF000000	// bit 31:24
	#define SPICKDIV_DIV_SHIFT		24
// 0x0C: SPI Read Data Register 
#define SPIRDR_ADDR			( SPI_BASE_ADDR + 0x0C )
#define pSPIRDR				( ( volatile uint32 * )SPIRDR_ADDR )
	#define SPIRDR_RD0R				0xFF000000	// bit 31:24
	#define SPIRDR_RD0R_SHIFT		24
	#define SPIRDR_RD1R				0x00FF0000	// bit 23:16
	#define SPIRDR_RD1R_SHIFT		16
// 0x0C: SPI Write Data Register 
#define SPITDR_ADDR			( SPI_BASE_ADDR + 0x0C )
#define pSPITDR				( ( volatile uint32 * )SPITDR_ADDR )
	#define SPITDR_TD0R				0xFF000000	// bit 31:24
	#define SPIRDR_TD0R_SHIFT		24
	#define SPITDR_TD1R				0x00FF0000	// bit 23:16
	#define SPIRDR_TD1R_SHIFT		16
// 0x10: SPI Transaction Configuration Register 
#define SPITCR_ADDR			( SPI_BASE_ADDR + 0x10 )
#define pSPITCR				( ( volatile uint32 * )SPITCR_ADDR )
	#define SPITCR_SEL_CS0			0x80000000	// bit 31
	#define SPITCR_SEL_CS1			0x40000000	// bit 30
	#define SPITCR_SEL_CS2			0x20000000	// bit 29
	#define SPITCR_SEL_CS3			0x10000000	// bit 28
	#define SPITCR_SEL_CS4			0x08000000	// bit 27
	#define SPITCR_SEL_CS5			0x04000000	// bit 26
	#define SPITCR_SEL_CS( n )		( ( 0x80000000UL >> ( n ) ) & SPITCR_SEL_CS_MASK )
	#define SPITCR_SEL_CS_MASK		0xFC000000	// bit 31:26
	#define SPITCR_CTL_EN			0x00800000	// bit 23
	#define SPITCR_ADD_EN			0x00400000	// bit 22
	#define SPITCR_D0_EN			0x00200000	// bit 21
	#define SPITCR_D1_EN			0x00100000	// bit 20
	#define SPITCR_CSFL				0x00010000	// bit 16
	#define SPITCR_DCS_CNT_UNIT		0x0000FF00	// bit 15:8
	#define SPITCR_DCS_CNT_UNIT_SHIFT	8
	#define SPITCR_DCS_DEF_CNT		0x000000FF	// bit 7:0
	#define SPITCR_DCS_DEF_CNT_SHIFT	0
// 0x14: SPI CS Deselect Time Counter Register 0
#define SPICDTCR0_ADDR		( SPI_BASE_ADDR + 0x14 )
#define pSPICDTCR0			( ( volatile uint32 * )SPICDTCR0_ADDR )
	#define SPICDTCR0_CS0_DES_CN	0xFF000000	// bit 31:24
	#define SPICDTCR0_CS1_DES_CN	0x00FF0000	// bit 23:16
	#define SPICDTCR0_CS2_DES_CN	0x0000FF00	// bit 15:8
	#define SPICDTCR0_CS3_DES_CN	0x000000FF	// bit 7:0
// 0x18: SPI CS Deselect Time Counter Register 1
#define SPICDTCR1_ADDR		( SPI_BASE_ADDR + 0x18 )
#define pSPICDTCR1			( ( volatile uint32 * )SPICDTCR1_ADDR )
	#define SPICDTCR1_CS4_DES_CN	0xFF000000	// bit 31:24
	#define SPICDTCR1_CS5_DES_CN	0x00FF0000	// bit 23:16
// 0x1c: SPI Timing Calibration Register 
#define SPITCALR_ADDR		( SPI_BASE_ADDR + 0x1c )
#define pSPITCALR			( ( volatile uint32 * )SPITCALR_ADDR )
	#define SPITCALR_CS_SETUP_CAL		0xFF000000	// bit 31:24
	#define SPITCALR_CS_SETUP_CAL_SHIEF	24
	#define SPITCALR_CS_HOLD_CAL		0x00FF0000	// bit 23:16
	#define SPITCALR_CS_HOLD_CAL_SHIEF	16
	#define SPITCALR_SD0_SETUP_CAL		0x0000FF00	// bit 15:8
	#define SPITCALR_SD0_SETUP_CAL_SHIEF	8

static inline int32 __rtl_hw_spi_chip_select( rtl_spi_dev_t* pDev, int select )
{
	switch( pDev ->type ) {
	case SPI_TYPE_HW:	// use SPI CS 
		// Transaction Configuration Register 
		if( select )
			*pSPITCR |= SPITCR_SEL_CS( pDev ->hw.spiCS_ );
		else
			*pSPITCR &= ~SPITCR_SEL_CS( pDev ->hw.spiCS_ );
		break;
		
	case SPI_TYPE_HW2:	// use GPIO CS 
		if( select )
			_rtl_generic_setGpioDataBit( pDev->hw2.gpioCS_, 0 ); /* fall down the CS_ */
		else
			_rtl_generic_setGpioDataBit( pDev->hw2.gpioCS_, 1 ); /* raise the CS_ */
		break;
	case SPI_TYPE_HW3:	// use GPIO for CSEN (Zarlink CSMODE special mode)
		_rtl_generic_setGpioDataBit( pDev->hw3.gpioCSEN_, pDev->hw3.CSMode_ ); 

		if( select )
			*pSPITCR |= SPITCR_SEL_CS( pDev ->hw3.spiCS_ );
		else
			*pSPITCR &= ~SPITCR_SEL_CS( pDev ->hw3.spiCS_ );
		break;		

	default:
		PRINT_R( "unknown spi type=%d\n", pDev ->type );
		return FAILED;
		break;
	}
	
	return SUCCESS;
}

static inline int32 _rtl_hw_spi_chip_select( rtl_spi_dev_t* pDev )
{
	return __rtl_hw_spi_chip_select( pDev, 1 );	// chip select 
}

static inline int32 _rtl_hw_spi_chip_deselect( rtl_spi_dev_t* pDev )
{
	return __rtl_hw_spi_chip_select( pDev, 0 );	// chip deselect 
}

static inline void __rtl_hw_spi_phase_select( rtl_spi_dev_t* pDev, int phase )
{
	// 1 or 3 phase 
	switch( phase ) {
	case 1: 	// Zarlink SLIC 
		// 1 phase use 'D0' only 
		*pSPITCR &= ~( SPITCR_CTL_EN | SPITCR_ADD_EN | SPITCR_D0_EN | SPITCR_D1_EN );
		*pSPITCR |= SPITCR_D0_EN;
		break;
		
	case 3:		// Silab SLIC 
		// 3 phases use 'CTL', 'ADD' and 'D0' 
		*pSPITCR &= ~( SPITCR_CTL_EN | SPITCR_ADD_EN | SPITCR_D0_EN | SPITCR_D1_EN );
		*pSPITCR |= SPITCR_CTL_EN | SPITCR_ADD_EN | SPITCR_D0_EN;
		break;
		
	default:
		PRINT_R( "unknown spi phase=%d\n", phase );
		break;
	}
}

static int32 _rtl_hw_spi_rawRead( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint8* pch = pData;
	
	*pch = 0;

	if ( pData == NULL ) return FAILED;
	
	// chip select 
	_rtl_hw_spi_chip_select( pDev );
	
	// phase select 
	__rtl_hw_spi_phase_select( pDev, 1 );
	
	// clear reading data (D0)
	//*pSPIRDR = 0xDE << SPIRDR_RD0R_SHIFT;	// use magic number instead of 0 
	*pSPIRDR = 0x06 << SPIRDR_RD0R_SHIFT;	// use magic number instead of 0 
	
	// set ctrl register & start transfer 
	*pSPICNR = SPICNR_CSTIME_8BIT | SPICNR_MSB | SPICNR_CMD_READ |
				SPICNR_START | SPICNR_CSP_LOW_ACTIVE;
	
	// wait unitl finish
	while( *pSPICNR & SPICNR_START );
	
	// read data (D0)
	*pch = ( uint8 )( ( *pSPIRDR & SPIRDR_RD0R ) >> SPIRDR_RD0R_SHIFT );
	
	// chip deselect 
	_rtl_hw_spi_chip_deselect( pDev );
	
	SPI_PRINTK( "Raw Read 8 [1]: *data=%X *pch=%X *pSPIRDR=%08X\n", 
							*( ( uint8 * )pData ), *pch, *pSPIRDR );
	
	return SUCCESS;
}

static int32 _rtl_hw_spi_transRead8( rtl_spi_dev_t* pDev, uint8 ctrl, uint8 addr, uint8 *data )
{
	SPI_PRINTK( "Transaction Read 8 [1]: dev=%p, ctrl=%X, addr=%X, data=%p\n",
										pDev, ctrl, addr, data );
	
	if ( data == NULL ) return FAILED;
	
	// chip select 
	_rtl_hw_spi_chip_select( pDev );
	
	// phase select 
	__rtl_hw_spi_phase_select( pDev, 3 );
	
	// clear reading data (D0)
	*pSPIRDR = 0xDE << SPIRDR_TD0R_SHIFT;	// use magic number instead of 0 

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	// clear MSIF_LOCK
	while( (*pSPISTR & (MSIF_LOCK <<24) ) == 0 && (isi_count < ISI_LOOP) ){
		printk("str = [%08x:%08x]\n", *( ( unsigned long volatile * )0xb8009004 ), (MSIF_LOCK <<24)); 
		isi_count ++;
	}
#endif
	
	// set ctrl register 
	//spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_RDIE(1) | address<<16 | control<<8 | SPI_LSB(0) | SPI_CMD(0) | SPI_START(1) | SPI_CSP(0);
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	*pSPICNR = SPICNR_CSTIME_8BIT | SPICNR_MSB | SPICNR_CMD_READ | SPICNR_SCLK_TYPE_II |
#else
	*pSPICNR = SPICNR_CSTIME_8BIT | SPICNR_MSB | SPICNR_CMD_READ | 
#endif
				SPICNR_CSP_LOW_ACTIVE |
				( ( ctrl << SPICNR_CTRL_SHIFT ) & SPICNR_CTRL_MASK ) |
				( ( addr << SPICNR_ADD_SHIFT ) & SPICNR_ADD_MASK );

	SPI_PRINTK( "Read: 0xb8009000=%08X 0xb8009004=%08X 0xb8009008=%08X 0xb8009010=%08X 0xb8009014=%08X  0xb8009018=%08X\n",
				*( ( unsigned long volatile * )0xb8009000 ), *( ( unsigned long volatile * )0xb8009004 ), 
				*( ( unsigned long volatile * )0xb8009008 ), *( ( unsigned long volatile * )0xb8009010 ), 
				*( ( unsigned long volatile * )0xb8009014 ), *( ( unsigned long volatile * )0xb8009018 ) );
	
	// start transfer
	*pSPICNR |= SPICNR_START;
	
	//wait unitl finish
	while( *pSPICNR & SPICNR_START );

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	// wait for MSIF_UNLOCK
	while( (*pSPISTR & (MSIF_LOCK <<24) ) == 0 && (isi_count < ISI_LOOP) ){
		printk("str = [%08x]\n", *( ( unsigned long volatile * )0xb8009004 )); 
		isi_count ++;
	}
#endif

	// read data (D0)
	*data = ( uint8 )( ( *pSPIRDR & SPIRDR_RD0R ) >> SPIRDR_RD0R_SHIFT );
	
	// chip deselect 
	_rtl_hw_spi_chip_deselect( pDev );
	
	SPI_PRINTK( "Transaction Read 8 [1]: *data=%X\n", *data );
	
	return SUCCESS;
}

static int32 _rtl_hw_spi_rawWrite( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	uint8* pch = pData;

	if ( pData == NULL ) return FAILED;
	
	// chip select 
	_rtl_hw_spi_chip_select( pDev );
	
	// phase select 
	__rtl_hw_spi_phase_select( pDev, 1 );
	
	// set writing data (D0)
	*pSPITDR = ( *pch ) << SPIRDR_TD0R_SHIFT;
	
	// set ctrl register 
	*pSPICNR = SPICNR_CSTIME_8BIT | SPICNR_MSB | SPICNR_CMD_WRITE |
				SPICNR_CSP_LOW_ACTIVE;
	
	// start transfer 
	*pSPICNR |= SPICNR_START;
	
	// wait unitl finish
	while( *pSPICNR & SPICNR_START );
	
	// chip deselect 
	_rtl_hw_spi_chip_deselect( pDev );
	
	return SUCCESS;
}

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
void setISIMSIF_LOCK(unsigned char cs){
	MSIF_LOCK = (1<<cs);
	printk("MSIF_LOCK 0x%x, cs 0x%x\n", MSIF_LOCK, cs);
}
#endif

static int32 _rtl_hw_spi_transWrite8( rtl_spi_dev_t* pDev, uint8 ctrl, uint8 addr, uint8 data )
{
	SPI_PRINTK( "Transaction Write 8 [1]: dev=%p, ctrl=%X, addr=%X, data=%X\n",
										pDev, ctrl, addr, data );
	
	// chip select 
	_rtl_hw_spi_chip_select( pDev );
	
	// phase select 
	__rtl_hw_spi_phase_select( pDev, 3 );
	
	// set writing data (D0)
	*pSPITDR = data << SPIRDR_TD0R_SHIFT;

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	// clear MSIF_LOCK
	while( (*pSPISTR & (MSIF_LOCK << 24) ) == 0 && (isi_count < ISI_LOOP) ){
		printk("str = [%08x]\n", *( ( unsigned long volatile * )0xb8009004 )); 
		isi_count ++;
	}
#endif
	
	// set ctrl register 
	//spicnr = SPI_CSTIME(0) | SPI_WRIE(1) | SPI_RDIE(1) | address<<16 | control<<8 | SPI_LSB(0) | SPI_CMD(1) | SPI_START(1) | SPI_CSP(0) ;
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	*pSPICNR = SPICNR_CSTIME_8BIT | SPICNR_MSB | SPICNR_CMD_WRITE | SPICNR_SCLK_TYPE_II |
#else
	*pSPICNR = SPICNR_CSTIME_8BIT | SPICNR_MSB | SPICNR_CMD_WRITE |
#endif
				SPICNR_CSP_LOW_ACTIVE |
				( ( ctrl << SPICNR_CTRL_SHIFT ) & SPICNR_CTRL_MASK ) |
				( ( addr << SPICNR_ADD_SHIFT ) & SPICNR_ADD_MASK );

	SPI_PRINTK( "Write %0x08: 0xb8009000=%08X 0xb8009004=%08X 0xb8009008=%08X 0xb8009010=%08X 0xb8009014=%08X  0xb8009018=%08X\n",
				*( ( unsigned long volatile * )0xb800900C ), 
				*( ( unsigned long volatile * )0xb8009000 ), *( ( unsigned long volatile * )0xb8009004 ), 
				*( ( unsigned long volatile * )0xb8009008 ), *( ( unsigned long volatile * )0xb8009010 ), 
				*( ( unsigned long volatile * )0xb8009014 ), *( ( unsigned long volatile * )0xb8009018 ) );
	
	// start transfer
	*pSPICNR |= SPICNR_START;
	
	//wait unitl finish
	while( *pSPICNR & SPICNR_START );

#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
	// wait for MSIF_UNLOCK
	while( (*pSPISTR & (MSIF_LOCK << 24)) == 0 && (isi_count < ISI_LOOP) ){
		printk("str = [%08x]\n", *( ( unsigned long volatile * )0xb8009004 )); 
		isi_count ++;
	}
#endif
	
	// chip deselect 
	_rtl_hw_spi_chip_deselect( pDev );
	
	return SUCCESS;	
}

int32 _init_rtl_spi_dev_type_hw( rtl_spi_dev_t* pDev, uint32 spiCS_ )
{

	init_rtl_hw_spi_IP();
	pDev ->type = SPI_TYPE_HW;
	
	pDev ->hw.spiCS_ = spiCS_;
	
	return SUCCESS;
}

#define GPIORESET 2
void gpioResetSLIC(void)
{

	printk("init gpio = 0x[%x]\n", GPIORESET);
	_rtl_generic_initGpioPin(GPIORESET, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );

	printk("pull high gpio = 0x[%x]\n", GPIORESET);
	_rtl_generic_setGpioDataBit(GPIORESET, 1);
	mdelay(50);

	printk("pull low gpio = 0x[%x]\n", GPIORESET);
	_rtl_generic_setGpioDataBit(GPIORESET, 0);

	mdelay(50);

	printk("pull high gpio = 0x[%x]\n", GPIORESET);
	_rtl_generic_setGpioDataBit(GPIORESET, 1);
}


int32 _init_rtl_spi_dev_type_hw2( rtl_spi_dev_t* pDev, uint32 gpioCS_ )
{
	pDev ->type = SPI_TYPE_HW2;
	
	pDev ->hw2.gpioCS_ = gpioCS_;
	
	_rtl_generic_initGpioPin( gpioCS_, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );
	
	return SUCCESS;
}

// HW SPI + GPIO for CSEN
int32 _init_rtl_spi_dev_type_hw3( rtl_spi_dev_t* pDev, uint32 spiCS_ , uint32 gpioCSEN_, uint32 cs_mode)
{
	if (( cs_mode < 0 ) || ( cs_mode > 1 ) )
	{
		PRINT_R("Error csmode=%d, csmode should be 0 or 1.\n", cs_mode);
		return FAILED;
	}

	pDev ->type = SPI_TYPE_HW3;
	
	pDev ->hw3.spiCS_ = spiCS_;

	pDev ->hw3.gpioCSEN_ = gpioCSEN_;
	
	pDev ->hw3.CSMode_ = cs_mode;

	_rtl_generic_initGpioPin( gpioCSEN_, GPIO_CONT_GPIO, GPIO_DIR_OUT, GPIO_INT_DISABLE );		

	return SUCCESS;
}


void init_rtl_hw_spi_IP( void )
{
#if 0
	// clock divisor register 
	*pSPICKDIV = ( 0x40 << SPICKDIV_DIV_SHIFT );
	
	// transaction configuration register 
	*pSPITCR = 	( SPITCR_D0_EN ) |
				( ( 0x50 << SPITCR_DCS_CNT_UNIT_SHIFT ) & SPITCR_DCS_CNT_UNIT ) |
				( ( 0x45 << SPITCR_DCS_DEF_CNT_SHIFT ) & SPITCR_DCS_DEF_CNT );

	SPI_PRINTK( "*pSPICKDIV(%p)=%08X pSPITCR(%p)=%08X\n",
			pSPICKDIV, *pSPICKDIV, 
			pSPITCR, *pSPITCR );

#else	// Apply 8676 HW SPI configuration which comply with Zarlink SPI timing
	// transaction configuration register 
	*pSPITCR = 	( SPITCR_D0_EN ) |
				( ( 0x18 << SPITCR_DCS_CNT_UNIT_SHIFT ) & SPITCR_DCS_CNT_UNIT ) |
				( ( 0x23 << SPITCR_DCS_DEF_CNT_SHIFT ) & SPITCR_DCS_DEF_CNT );
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_ISI
        *pSPICKDIV = 0x26000000;
        *pSPITCALR = 0x08272700;
#else
	// clock divisor register 
	*pSPICKDIV = ( 0x7 << SPICKDIV_DIV_SHIFT );	// 7.8M
	
	// timing calibration register
	*pSPITCALR = ( ( 0x04 << SPITCALR_CS_SETUP_CAL_SHIEF ) & SPITCALR_CS_SETUP_CAL) |
			( ( 0x04 << SPITCALR_CS_HOLD_CAL_SHIEF ) & SPITCALR_CS_HOLD_CAL) |
			( ( 0x00 << SPITCALR_SD0_SETUP_CAL_SHIEF ) & SPITCALR_SD0_SETUP_CAL);
			//( ( 0x00 << SPITCALR_SD0_SETUP_CAL_SHIEF ) & SPITCALR_SD0_SETUP_CAL);
#endif

	SPI_PRINTK( "*pSPICKDIV(%p)=%08X pSPITCR(%p)=%08X pSPITCALR(%p)=%08X\n",
			pSPICKDIV, *pSPICKDIV, 
			pSPITCR, *pSPITCR,
			pSPITCALR, *pSPITCALR);
#endif
}

// ------------------------------------------------------------------------------
// HW SPI register proc 
// ------------------------------------------------------------------------------
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int voip_spi_regs_read_proc(struct seq_file *f, void *v)
{
	if( f->index ) {	/* In our case, we write out all data at once. */
		return 0;
	}
	seq_printf( f, "SPICNR   (%p)=%08X\n", pSPICNR, *pSPICNR );
	seq_printf( f, "SPISTR   (%p)=%08X\n", pSPISTR, *pSPISTR );
	seq_printf( f, "SPICKDIV (%p)=%08X\n", pSPICKDIV, *pSPICKDIV );
	seq_printf( f, "SPIRDR   (%p)=%08X\n", pSPIRDR, *pSPIRDR );
	seq_printf( f, "SPITDR   (%p)=%08X\n", pSPITDR, *pSPITDR );
	seq_printf( f, "SPITCR   (%p)=%08X\n", pSPITCR, *pSPITCR );
	seq_printf( f, "SPICDTCR0(%p)=%08X\n", pSPICDTCR0, *pSPICDTCR0 );
	seq_printf( f, "SPICDTCR1(%p)=%08X\n", pSPICDTCR1, *pSPICDTCR1 );
	seq_printf( f, "SPITCALR (%p)=%08X\n", pSPITCALR, *pSPITCALR );

	return 0;
}
#else
static int voip_spi_regs_read_proc( char *buf, char **start, off_t off, int count, int *eof, void *data )
{
	//int ch;//, ss;
	int n = 0;

	if( off ) {	/* In our case, we write out all data at once. */
		*eof = 1;
		return 0;
	}
	
	n += sprintf( buf + n, "SPICNR   (%p)=%08X\n", pSPICNR, *pSPICNR );
	n += sprintf( buf + n, "SPISTR   (%p)=%08X\n", pSPISTR, *pSPISTR );
	n += sprintf( buf + n, "SPICKDIV (%p)=%08X\n", pSPICKDIV, *pSPICKDIV );
	n += sprintf( buf + n, "SPIRDR   (%p)=%08X\n", pSPIRDR, *pSPIRDR );
	n += sprintf( buf + n, "SPITDR   (%p)=%08X\n", pSPITDR, *pSPITDR );
	n += sprintf( buf + n, "SPITCR   (%p)=%08X\n", pSPITCR, *pSPITCR );
	n += sprintf( buf + n, "SPICDTCR0(%p)=%08X\n", pSPICDTCR0, *pSPICDTCR0 );
	n += sprintf( buf + n, "SPICDTCR1(%p)=%08X\n", pSPICDTCR1, *pSPICDTCR1 );
	n += sprintf( buf + n, "SPITCALR (%p)=%08X\n", pSPITCALR, *pSPITCALR );
	
	*eof = 1;
	return n;
}
#endif

#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
static int spi_reg_open(struct inode *inode, struct file *file)
{
	return single_open(file, voip_spi_regs_read_proc, NULL);
}

struct file_operations proc_spi_reg_fops = {
	.owner	= THIS_MODULE,
	.open	= spi_reg_open,
	.read	= seq_read,
	.llseek	= seq_lseek,
	.release = single_release,	
//read:   voip_spi_regs_read_proc
};
#endif


static int __init voip_spi_proc_init( void )
{
#if  defined(CONFIG_DEFAULTS_KERNEL_3_10)
	proc_create_data( PROC_VOIP_DIR "/spi_regs", 0, NULL, &proc_spi_reg_fops, NULL );
#else
	create_proc_read_entry( PROC_VOIP_DIR "/spi_regs", 0, NULL, voip_spi_regs_read_proc, NULL );
#endif

	return 0;
}

static void __exit voip_spi_proc_exit( void )
{
	remove_voip_proc_entry( PROC_VOIP_DIR "/spi_regs", NULL );
}

voip_initcall_proc( voip_spi_proc_init );
voip_exitcall( voip_spi_proc_exit );

#endif // CONFIG_RTK_VOIP_HARDWARE_SPI

// ------------------------------------------------------------------------------
// SPI abstraction interfaces 
// ------------------------------------------------------------------------------

int32 _rtl_spi_rawRead( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	SPI_PRINTK( "Raw Read 8 [0]: dev=%p, data=%p *data8=%02X, bits=%d\n",
								pDev, pData, *( ( uint8 * )pData ), bits );
	
	switch( pDev ->type ) {
#ifdef CONFIG_RTK_VOIP_SOFTWARE_SPI
	case SPI_TYPE_SW:	// software SPI (GPIO)
		return _rtl_sw_spi_rawRead( pDev, pData, bits );
		break;
#endif
#ifdef CONFIG_RTK_VOIP_HARDWARE_SPI
	case SPI_TYPE_HW:	// hardware SPI fully (SPI CS)
	case SPI_TYPE_HW2:	// hardware SPI + GPIO CS 
	case SPI_TYPE_HW3:	// hardware SPI + GPIO for CSEN	
		return _rtl_hw_spi_rawRead( pDev, pData, bits );
		break;
#endif
	default:
		break;
	}
	
	return FAILED;
}

int32 _rtl_spi_transRead8( rtl_spi_dev_t* pDev, uint8 ctrl, uint8 addr, uint8 *data )
{
	int32 ret;
	
	SPI_PRINTK( "Transaction Read 8 [0]: dev=%p, ctrl=%X, addr=%X, data=%p\n",
										pDev, ctrl, addr, data );
	
	switch( pDev ->type ) {
#ifdef CONFIG_RTK_VOIP_SOFTWARE_SPI
	case SPI_TYPE_SW:	// software SPI (GPIO)
		if( ( ret = _rtl_sw_spi_rawWrite( pDev, &ctrl, 8 ) ) != SUCCESS )
			return ret;
		if( ( ret = _rtl_sw_spi_rawWrite( pDev, &addr, 8 ) ) != SUCCESS )
			return ret;
		if( ( ret = _rtl_sw_spi_rawRead( pDev, data, 8 ) ) != SUCCESS )
			return ret;
		break;
#endif
#ifdef CONFIG_RTK_VOIP_HARDWARE_SPI
	case SPI_TYPE_HW:	// hardware SPI fully (SPI CS)
		return _rtl_hw_spi_transRead8( pDev, ctrl, addr, data );
		break;
		
	case SPI_TYPE_HW2:	// hardware SPI + GPIO CS 
	case SPI_TYPE_HW3:	// hardware SPI + GPIO for CSEN
		if( ( ret = _rtl_hw_spi_rawWrite( pDev, &ctrl, 8 ) ) != SUCCESS )
			return ret;
		if( ( ret = _rtl_hw_spi_rawWrite( pDev, &addr, 8 ) ) != SUCCESS )
			return ret;
		if( ( ret = _rtl_hw_spi_rawRead( pDev, data, 8 ) ) != SUCCESS )
			return ret;	
		break;
#endif
	default:
		return FAILED;
		break;
	}
	
	return SUCCESS;
}

int32 _rtl_spi_rawWrite( rtl_spi_dev_t* pDev, void* pData, int32 bits )
{
	SPI_PRINTK( "Raw Write 8 [0]: dev=%p, data=%p *data8=%02X, bits=%d\n",
								pDev, pData, *( ( uint8 * )pData ), bits );
	
	switch( pDev ->type ) {
#ifdef CONFIG_RTK_VOIP_SOFTWARE_SPI
	case SPI_TYPE_SW:	// software SPI (GPIO)
		return _rtl_sw_spi_rawWrite( pDev, pData, bits );
		break;
#endif
#ifdef CONFIG_RTK_VOIP_HARDWARE_SPI
	case SPI_TYPE_HW:	// hardware SPI fully (SPI CS)
	case SPI_TYPE_HW2:	// hardware SPI + GPIO CS 
	case SPI_TYPE_HW3:	// hardware SPI + GPIO for CSEN
		return _rtl_hw_spi_rawWrite( pDev, pData, bits );
		break;
#endif
	default:
		break;
	}
	
	return FAILED;
}

int32 _rtl_spi_transWrite8( rtl_spi_dev_t* pDev, uint8 ctrl, uint8 addr, uint8 data )
{
	int32 ret;
	
	SPI_PRINTK( "Transaction Write 8 [0]: dev=%p, ctrl=%X, addr=%X, data=%X\n",
										pDev, ctrl, addr, data );
	
	switch( pDev ->type ) {
#ifdef CONFIG_RTK_VOIP_SOFTWARE_SPI
	case SPI_TYPE_SW:	// software SPI (GPIO)
		if( ( ret = _rtl_sw_spi_rawWrite( pDev, &ctrl, 8 ) ) != SUCCESS )
			return ret;
		if( ( ret = _rtl_sw_spi_rawWrite( pDev, &addr, 8 ) ) != SUCCESS )
			return ret;
		if( ( ret = _rtl_sw_spi_rawWrite( pDev, &data, 8 ) ) != SUCCESS )
			return ret;
		break;
#endif
#ifdef CONFIG_RTK_VOIP_HARDWARE_SPI
	case SPI_TYPE_HW:	// hardware SPI fully (SPI CS)
		return _rtl_hw_spi_transWrite8( pDev, ctrl, addr, data );
		break;
		
	case SPI_TYPE_HW2:	// hardware SPI + GPIO CS 
	case SPI_TYPE_HW3:	// hardware SPI + GPIO for CSEN
		if( ( ret = _rtl_hw_spi_rawWrite( pDev, &ctrl, 8 ) ) != SUCCESS )
			return ret;
		if( ( ret = _rtl_hw_spi_rawWrite( pDev, &addr, 8 ) ) != SUCCESS )
			return ret;
		if( ( ret = _rtl_hw_spi_rawWrite( pDev, &data, 8 ) ) != SUCCESS )
			return ret;	
		break;
#endif
	default:
		return FAILED;
		break;
	}
	
	return SUCCESS;
}




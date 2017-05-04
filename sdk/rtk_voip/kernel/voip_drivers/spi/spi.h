/*
* Copyright c                  Realtek Semiconductor Corporation, 2006  
* All rights reserved.
*/

#ifndef _SPI_H_
#define _SPI_H_

#include "voip_types.h"
#include "gpio/gpio.h"

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676)
#include "spi_8676.h"
#endif

// ------------------------------------------------------------------------------
// SPI dev context definition 
// ------------------------------------------------------------------------------

typedef enum {
	SPI_TYPE_SW,	// software SPI (GPIO)
	SPI_TYPE_HW,	// hardware SPI fully (SPI CS)
	SPI_TYPE_HW2,	// hardware SPI + GPIO CS 
	SPI_TYPE_HW3,	// hardware SPI + GPIO for CSEN
} spi_type_t;

typedef struct rtl_spi_dev_s
{
	spi_type_t type;	// hardware or software 

#ifndef __UC_EDITOR__		// help editor to parse C code 
	union {
#endif
		struct {	// type == SPI_TYPE_SW
			uint32 gpioSCLK;
			uint32 gpioCS_;
			uint32 gpioSDI;
			uint32 gpioSDO;
#ifdef CONFIG_RTK_VOIP_DECT_SPI_SUPPORT
			uint32 gpioINT;
#endif
		} sw;
		
		struct {	// type == SPI_TYPE_HW
			uint32 spiCS_;	// corresponding to SPITCR bit 31:26 
		} hw;
		
		struct {	// type == SPI_TYPE_HW2
			uint32 gpioCS_;
		} hw2;

		struct {	// type == SPI_TYPE_HW3
			uint32 spiCS_;	// corresponding to SPITCR bit 31:26
			uint32 gpioCSEN_;
			uint32 CSMode_;
		} hw3;		
#ifndef __UC_EDITOR__		// help editor to parse C code 
	};
#endif

	// backward compatible variables (going to remove)
#if defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK) && defined(CONFIG_RTK_VOIP_8676_SHARED_SPI)
	uint32 SPI_SEL_CS;
#endif

	//uint32 SClkDelayLoop;
} rtl_spi_dev_t;

// ------------------------------------------------------------------------------
// SPI interfaces 
// ------------------------------------------------------------------------------

// SPI context initialization 
extern int32 _init_rtl_spi_dev_type_sw( rtl_spi_dev_t* pDev, 
			uint32 gpioCS_, uint32 gpioSCLK, uint32 gpioSDO, uint32 gpioSDI, int32 gpioINT );
#define init_spi_pins( dev, cs, clk, do, di )	_init_rtl_spi_dev_type_sw( dev, cs, clk, do, di, 0 )

extern int32 _init_rtl_spi_dev_type_hw( rtl_spi_dev_t* pDev, uint32 spiCS_ );
extern int32 _init_rtl_spi_dev_type_hw2( rtl_spi_dev_t* pDev, uint32 gpioCS_ );
extern int32 _init_rtl_spi_dev_type_hw3( rtl_spi_dev_t* pDev, uint32 spiCS_ , uint32 gpioCSEN_, uint32 cs_mode);

// SPI raw (1 phase) read/write 
extern int32 _rtl_spi_rawRead( rtl_spi_dev_t* pDev, void* pData, int32 bits );
extern int32 _rtl_spi_rawWrite( rtl_spi_dev_t* pDev, void* pData, int32 bits );

// SPI transaction (3 phases) read/write 
extern int32 _rtl_spi_transRead8( rtl_spi_dev_t* pDev, uint8 ctrl, uint8 addr, uint8 *data );
extern int32 _rtl_spi_transWrite8( rtl_spi_dev_t* pDev, uint8 ctrl, uint8 addr, uint8 data );

#if defined(CONFIG_RTK_VOIP_DRIVERS_PCM8676)
#ifdef CONFIG_RTK_VOIP_8676_SPI_GPIO
extern int32 __rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 gpioSCLK, uint32 gpioCS_, uint32 gpioSDO, uint32 gpioSDI);
#define init_spi_pins( dev, cs, clk, do, di )  _rtl867x_spi_init( dev, clk, cs, do, di )
#elif defined (CONFIG_RTK_VOIP_8676_SHARED_SPI)
#ifdef CONFIG_RTK_VOIP_DRIVERS_SLIC_SILAB
extern void __rtl867x_spi_init(int ch_spi);
#define init_spi_pins( dev, cs, clk, do, di )	__rtl867x_spi_init(0)
#elif defined(CONFIG_RTK_VOIP_DRIVERS_SLIC_ZARLINK)
extern int32 __rtl867x_spi_init( rtl_spi_dev_t* pDev, uint32 SPI_SEL_CS);
#define init_spi_pins( dev, cs, clk, do, di )	__rtl867x_spi_init( dev, cs)
void vp_read(rtl_spi_dev_t *pDev, unsigned char *data);
void vp_write(rtl_spi_dev_t *pDev, unsigned int data);
#endif 
#endif
#endif

/***************************************************************************/

#ifndef SUCCESS
#define SUCCESS 	0
#endif
#ifndef FAILED
#define FAILED 		-1
#endif

#endif //_SPI_H_

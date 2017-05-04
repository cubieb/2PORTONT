/*
 *	Realtek Header file
 */

#ifndef _SPI_FLASH_RTK_
#define _SPI_FLASH_RTK_

 /*
  * 	Macro Definition
  */
#define SPI_CS(i)           	((i) << 30)   /* 0: CS0 & CS1   1: CS0   2: CS1   3: NONE */
#define SPI_LENGTH(i)       	((i) << 28)   /* 0 ~ 3 */
#define SPI_READY(i)        	((i) << 27)   /* 0: Busy  1: Ready */
#define SPI_CLK_DIV(i)      	((i) << 29)   /* 0: DIV_2  1: DIV_4  2: DIV_6 ... 7: DIV_16 */
#define SPI_RD_ORDER(i)    ((i) << 28)   /* 0: Little-Endian  1: Big-Endian */
#define SPI_WR_ORDER(i)   	((i) << 27)   /* 0: Little-Endian  1: Big-Endian */
#define SPI_RD_MODE(i)      ((i) << 26)   /* 0: Fast-Mode  1: Normal Mode */
#define SPI_SFSIZE(i)       	((i) << 23)   /* 0 ~ 7. 128KB * (i+1) */
#define SPI_TCS(i)          	((i) << 19)   /* 0 ~ 15 */
#define SPI_RD_OPT(i)       	((i) << 18)   /* 0: No-Optimization  1: Optimized for Sequential Access */

  /* 
    * SPI marcos definition 
    */
#define LENGTH(i)   	SPI_LENGTH(i)
#define CS(i)           	SPI_CS(i)
#define RD_ORDER(i)     SPI_RD_ORDER(i)
#define WR_ORDER(i)    SPI_WR_ORDER(i)
#define READY(i)        	SPI_READY(i)
#define CLK_DIV(i)      	SPI_CLK_DIV(i)
#define RD_MODE(i)      SPI_RD_MODE(i)
#define SFSIZE(i)       	SPI_SFSIZE(i)
#define TCS(i)          	SPI_TCS(i)
#define RD_OPT(i)       	SPI_RD_OPT(i)
  
/* 
  * SPI Flash size definition
  */
#define SPI_BLOCK_SIZE 	65536  	/* 64KB */
#define SPI_SECTOR_SIZE 	4096  	/*  4KB */
  
/* 
  * SPI Flash Controller
  */
#define SFCR		0xB8001200
#define SFCSR	0xB8001208
#define SFDR		0xB800120c


 /*
  * 	Structure Declaration
  */
 struct spi_flash_type
 {
	unsigned char maker_id;
	unsigned char type_id;
	unsigned char capacity_id;
	unsigned char device_size;		  // 2 ^ N (bytes)
	unsigned int 	sector_cnt; //enlarge sector_cnt, 16MB spi flash = 256 sectors
 };

struct spi_flash_db
{
   	unsigned char maker_id;
  	 unsigned char type_id;
	 signed char size_shift;
};

#define FLASHBASE	CONFIG_SYS_FLASH_BASE

#endif 

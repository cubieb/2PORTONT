#ifndef __BUS_H__
#define __BUS_H__


/* Switches */
#define SPI_BUS // Select SPI_BUS or RS232_BUS

/****************************************************************************
*                     Definitions
****************************************************************************/
/* Serial communication configuration */
#ifdef RS232_BUS
	#define BAUDRATE B115200            
	#define MODEMDEVICE "/dev/ttyS0"
#endif //RS232_BUS
/* Spi communication configuration */
#ifdef SPI_BUS
//#ifdef CONFIG_RTK_VOIP_DECT_SITEL_SUPPORT
//	#define MODEMDEVICE "/dev/dect_cvm480_spi"
//#elif defined (CONFIG_RTK_VOIP_DECT_DSPG_SUPPORT)
	#define MODEMDEVICE "/dev/dect_cmbs_spi"
//#endif
#endif //SPI_BUS
//#define DEBUG_BUS


/*Typedefs*/
#define u16 unsigned short


/*Function Prototypes */
/* Exported by bus.c*/

int Bus_Init(void);
int Bus_Open(void);
int Bus_Close(void);
int Bus_Read_byte_blocking(unsigned char *Buf);
int Bus_Read_blocking(unsigned char *Buf, unsigned int len);
int Bus_Write_byte_blocking(unsigned char *Buf, unsigned short Len);

#endif //__BUS_H__

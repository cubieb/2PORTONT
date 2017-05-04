#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_
#include <kernel_soc.h>

//#define MTD_SPI_DEBUG
#if defined(MTD_SPI_DEBUG)
#define KDEBUG(args...) printk(args)
#else
#define KDEBUG(args...)
#endif

//#define SPI_DEBUG

#endif /* _SPI_FLASH_H_ */

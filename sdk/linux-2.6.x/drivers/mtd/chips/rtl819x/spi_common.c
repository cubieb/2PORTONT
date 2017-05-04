#include "spi_flash.h"
#ifdef SPI_KERNEL
#include <linux/errno.h>
extern unsigned int SFCSR;
extern unsigned int SFDR;
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//type of SPI flash we support
struct spi_flash_db flash_tables[NUM_KNOWN] = {

	{
		mfr_id: 	SPANSION,
		dev_id: 	SPI,
		size_shift:	1,
#ifdef SPI_KERNEL			
		name: "spansion",
		DeviceSize: SIZE_2MiB,
#endif		
		EraseSize: SIZE_64KiB,
	},
// Support for MX2fL series flash 
	{
		mfr_id: 0xC2,
		dev_id: 0x20,
		size_shift:	0,
#ifdef SPI_KERNEL			
		name: "mxic",
		DeviceSize: 0x200000,
#endif		
		EraseSize: 4096,
	},
        {
		mfr_id: 0xC2,
		dev_id: 0x5e,
		size_shift:	0,
#ifdef SPI_KERNEL			
		name: "mxic",
		DeviceSize: 0x400000,
#endif		
		EraseSize: 4096,//104MHz
	},
	{
		mfr_id: 0xC2,
		dev_id: 0x24,
		size_shift:	0,
#ifdef SPI_KERNEL					
		name: "mxic",
		DeviceSize: 0x200000,
#endif		
		EraseSize: 4096,//104MHz
	},
	{
		mfr_id: 0xbf,
		dev_id: 0x25,
		size_shift:	0,
#ifdef SPI_KERNEL					
		name: "sst",
		DeviceSize: 0x200000,
#endif		
		EraseSize: 4096,//104MHz
	},
	{
		mfr_id: 0x01,
		dev_id: 0x20,
		size_shift:	0,
#ifdef SPI_KERNEL					
		name: "spansion",
		DeviceSize: 0x1000000,
#endif		
		EraseSize: SIZE_64KiB,//104MHz
	},
	{
		mfr_id: 0xbf,
		dev_id: 0x26,
		size_shift:	0,
#ifdef SPI_KERNEL			
		name: "sst",
		DeviceSize: 0x200000,
#endif		
		EraseSize: 4096,//104MHz
	},
	{
		mfr_id: 0xef,
		dev_id: 0x40,
		size_shift:	0,
#ifdef SPI_KERNEL			
		name: "winbond",
		DeviceSize: 0x200000,
#endif		
		EraseSize: 4096,//104MHz
	},
};
//---------------------------------------------------------
//---------------------------------------------------------
/*
* SPI Flash APIs
*/
 void spi_ready(void)
{
   while (1)
   {
      if ( (*(volatile unsigned int *) SFCSR) & READY(1))
         break;
   }
}

 #if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
void spi_pio_init_8198(void)
{
  spi_ready();
  *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(0) | READY(1);

  spi_ready();
  *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);

  spi_ready();
  *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(0) | READY(1);

  spi_ready();
  *(volatile unsigned int *) SFCSR_8198 = LENGTH(3) | CS(3) | READY(1);
}
#endif
//---------------------------------------------------------
/*
 * This function shall be called when switching from MMIO to PIO mode
*/
void spi_pio_init(void)
{
#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
   spi_pio_init_8198();
#else
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);

   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
#endif
}
/*
 The Block Erase function
*/
int do_spi_block_erase(unsigned int addr)
{
	int chip=0;

#ifdef SPI_DEBUG
#ifdef SPI_KERNEL
	printk("\r\n do_spi_block_erase : [%x] ", addr);
#else
	printf("\r\n do_spi_block_erase : [%x] ", addr);
#endif
#endif
	int flags;
	//spin_lock_irqsave(spi_lock,flags);
	spi_pio_init();
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	//spin_unlock_irqrestore(spi_lock,flags);

	return 0;
}

unsigned int do_spi_write(unsigned int from, unsigned int to, unsigned int size)
{
   unsigned int temp;
	unsigned int  remain;
   unsigned int cur_addr;
   unsigned int cur_size ,flash_addr;
   unsigned int cnt;
   unsigned int next_page_addr;
 
	int flags;
	//spin_lock_irqsave(spi_lock,flags);
   cur_addr = from;
   flash_addr = to;
   cur_size = size;

#ifdef SPI_DEBUG
#ifdef SPI_KERNEL
	printk("\r\n do_spi_write : from :[%x] to:[%x], size:[%x]  ", from, to, size);
#else
	dprintf("\r\n do_spi_write : from :[%x] to:[%x], size:[%x]  ", from, to, size);
#endif
#endif

   	   spi_pio_init();
      next_page_addr = ((flash_addr >> 8) +1) << 8;

      while (cur_size > 0)
      {
	   /* De-select Chip */
	   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	  
         /* WREN Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = 0x06 << 24;
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
         /* PP Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = (0x02 << 24) | (flash_addr & 0xFFFFFF);

	   while (flash_addr != next_page_addr)
	   {
		remain = (cur_size > 4)?4:cur_size;		
		temp = *((int*)cur_addr);
		
            spi_ready();
			
            *(volatile unsigned int *) SFCSR = LENGTH(remain-1) | CS(1) | READY(1);                     
            *(volatile unsigned int *) SFDR = temp;
		
		cur_size -= remain;
		cur_addr += remain;
		flash_addr+=remain;
		
            if (cur_size == 0)
               break;;
	   }
		next_page_addr = flash_addr + 256;
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

         /* RDSR Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = 0x05 << 24;

         cnt = 0;
         while (1)
         {
            unsigned int status = *(volatile unsigned int *) SFDR;

            /* RDSR Command */
            if ((status & 0x01000000) == 0x00000000)
            {
                break;
            }

            if (cnt > 200000)
            {
	//spin_unlock_irqrestore(spi_lock,flags);

            		return -EINVAL;
            }
            cnt++;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
      }
	//spin_unlock_irqrestore(spi_lock,flags);

	return 0;
}
//----------------------------------------------------------------------------
//sst
void sst_spi_write(unsigned int cnt, unsigned int address, unsigned char data_in)
{
        /* RDSR Command */
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = 0x05 << 24;

    while (1)
    {
        /* RDSR Command */
        if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x0)
        {
            break;
        }
    }

        //1.release CS
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

        /* WREN Command */
    spi_ready();                                    //2.waiting release OK
        //3.CS low
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = 0x06 << 24;   //4.instr code
        //1.release CS
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

        /* BP Command */
    spi_ready();                                    //2.waiting release OK
        //3.CS low
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);

        //JSW: for SST Byte Program,be aware of LENGTH @2007/9/26
        //4.instr code
    *(volatile unsigned int *) SFDR = (0x02 << 24) | (address & 0xFFFFFF);
        //  *(volatile unsigned int *) SFDR = (0x02<<24 ) ;  //4.instr code

        //3.CS low
    *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
    *(volatile unsigned int *) SFDR = (data_in<<24);
        //5.CS HIGH
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

}






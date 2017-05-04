#include "spi_flash.h"
#if 0
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kmod.h>
//#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>

#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>

#include <asm/io.h>
#endif
#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>
#include <linux/spinlock.h>
//#include <linux/wait.h>
//tylo, for ic ver. detect
//#include <asm/mach-realtek/rtl8672/platform.h>
/*linux-2.6.19*/
#include <bspchip.h>
unsigned int ICver=0;
/* SPI Flash Controller */
unsigned int SFCR=0;
unsigned int SFCSR=0;
unsigned int SFDR=0;


#ifdef CONFIG_RTL8676
unsigned int SPI_EN4B_ADDR = 0xB8000330;
#define EN4B_SHIFT	19
#else
unsigned int SPI_EN4B_ADDR = 0xB8000100;
#define EN4B_SHIFT	6
#endif
#define SPI_EN4B(i) ((i) << EN4B_SHIFT)
static int spi_en_4b=0;


// patch for flashsize > 4MiB
#if defined(CONFIG_32M_FLASH) || defined(CONFIG_16M_FLASH) || defined(CONFIG_8M_FLASH)
#define FLASH_START_ADDR 0xbd000000
#else
#define FLASH_START_ADDR 0xbfc00000
#endif

#define LENGTH(i)       SPI_LENGTH(i)
#define CS(i)           SPI_CS(i)
#define RD_ORDER(i)     SPI_RD_ORDER(i)
#define WR_ORDER(i)     SPI_WR_ORDER(i)
#define READY(i)        SPI_READY(i)
#define CLK_DIV(i)      SPI_CLK_DIV(i)
#define RD_MODE(i)      SPI_RD_MODE(i)
#define SFSIZE(i)       SPI_SFSIZE(i)
#define TCS(i)          SPI_TCS(i)
#define RD_OPT(i)       SPI_RD_OPT(i)

/*
 * SPI Flash Info
 */
const struct spi_flash_db   spi_flash_known[] =
{
   {0x01, 0x02,   1}, /* Spansion */
   {0xC2, 0x20,   0}, /* MXIC */
   {0x1C, 0x31,   0}, /* EON */
   {0x1C, 0x30,   0}, /* EON */
   {0xC2, 0x5E,   0}, /* MXIC high performance */
   {0xef, 0x40,   0}, /* winbond W25Q128FVFIG */   	
   {0x01, 0x20,   0}, /* Spansion S25FL128SAGMFI00*/ 	   
   {0x20, 0xBA,   0}, /* Micron */ 	   
};

//#define SPI_DEBUG

/*
 * SPI Flash Info
 */
struct spi_flash_type   spi_flash_info[2];

#define MAX_READY_POLLING (1000000)
static inline void spi_ready(void)
{
   int cnt = 0;
   u32 tested = 0;
 
   //while (1)
   while (cnt < 100)
   {
      if ( (*(volatile unsigned int *) SFCSR) & READY(1)) {
         cnt++;   
      } else {
         cnt = 0;
      }
      tested++;
      if(tested > MAX_READY_POLLING) {
         printk("ERROR: %s failed, polling READY bit time out, tested %u times\n", __FUNCTION__, tested);
         while(1);
      }
   }
}

/*
 * SPI Flash APIs
 */

/*
 * This function shall be called when switching from MMIO to PIO mode
 */
// #define __TESSPIIRAM		__attribute__ ((__section__ (".iram")))
//__TESSPIIRAM 
#if 0
volatile inline static void ___delay(unsigned int l) {
	volatile unsigned int x;
	while (l--) {
		for(x=0;x<40;x++)
			;
	}
}
#endif

static void flush_prediction(unsigned int x) {
    if (x>1)
        flush_prediction(x-1);
}

/*Delaying at least one clkm clock is to gaurantee that the controller samples the CS bit change correctly*/
#define CS_LOW_DELAY (udelay(3))/* This gaurantees clkm higher than or equal to 1.6MHz */
//#define CS_LOW_DELAY (ndelay(125))/* This gaurantees clkm higher than or equal to 8MHz, failed on some board whose DRAM running at 100MHz */
void spi_pio_init(void)
{
#ifdef SPI_DEBUG
   printk("%s(%d)\n",__FUNCTION__,__LINE__);
#endif 
   flush_prediction(10);
      
   /* CS low to high triggers de-select count down to start */
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(0) | READY(1);
   CS_LOW_DELAY;
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
   spi_ready();
}
#if 0

void spi_pio_init_ready(void)
{
   spi_ready();
}

void spi_pio_toggle1(void)
{
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}
void spi_pio_toggle2(void)
{
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
}
void spi_read(unsigned int chip, unsigned int address, unsigned int *data_out)
{
   /* De-Select Chip */
   ___delay(2);
   
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      unsigned int status;

      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
      {
         break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* READ Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);

   *(volatile unsigned int *) SFDR = (0x03 << 24) | (address & 0xFFFFFF);

   /* Read Data Out */
   *data_out = *(volatile unsigned int *) SFDR;

   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}

__TESSPIIRAM void spi_write(unsigned int chip, unsigned int address, unsigned int data_in)
{
   /* De-select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      unsigned int status;

      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
      {
         break;
      }
   }

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* WREN Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x06 << 24;

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* PP Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = (0x02 << 24) | (address & 0xFFFFFF);
   *(volatile unsigned int *) SFDR = data_in;
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}

 void spi_erase_chip(unsigned int chip)
{
	   ___delay(2);
   /* De-select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

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

   /* WREN Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = 0x06 << 24;
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* BE Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
   *(volatile unsigned int *) SFDR = (0xC7 << 24);
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
}

 void spi_erase_sector(int sector){
	int chip=0;
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | (sector * 65536);
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
}
#endif
void spi_cp_probe(void)
{
   unsigned int cnt=0, i=0;
   unsigned int temp;

#ifdef SPI_DEBUG
   printk("%s(%d)\n",__FUNCTION__,__LINE__);
#endif
   //___delay(2);
   flush_prediction(10);
   for (cnt = 0; cnt < 2; cnt++)
   {
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
      CS_LOW_DELAY;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
      spi_ready();

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);
      *(volatile unsigned int *) SFDR = RDID << 24;
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt);
      spi_ready();
      temp = *(volatile unsigned int *) SFDR;

      spi_flash_info[cnt].maker_id = (temp >> 24) & 0xFF;
      spi_flash_info[cnt].type_id = (temp >> 16) & 0xFF;
      spi_flash_info[cnt].capacity_id = (temp >> 8) & 0xFF;

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3);
      spi_ready();

      /* Iterate Each Maker ID/Type ID Pair */
      for (i = 0; i < sizeof(spi_flash_known) / sizeof(struct spi_flash_db); i++)
      {
         if ( (spi_flash_info[cnt].maker_id == spi_flash_known[i].maker_id) &&
              (spi_flash_info[cnt].type_id == spi_flash_known[i].type_id) )
         {
            spi_flash_info[cnt].device_size = (unsigned char)((signed char)spi_flash_info[cnt].capacity_id + spi_flash_known[i].size_shift);
         }
      }

      spi_flash_info[cnt].sector_cnt = 1 << (spi_flash_info[cnt].device_size - 16);
   }
#ifdef SPI_DEBUG
   for(i=0;i<2;i++){
	printk("get SPI CS%d\n\r",i);
	printk("maker:%x  type:%x  sector_cnt:%d\n",spi_flash_info[i].maker_id,spi_flash_info[i].type_id,spi_flash_info[i].sector_cnt);
   }
#endif
}

#if 0
 void spi_burn_image(unsigned int chip, unsigned char *image_addr, unsigned int image_size)
{
   unsigned int temp;
   unsigned int i, j, k;
   unsigned char *cur_addr;
   unsigned int cur_size;
   unsigned int cnt;

   cur_addr = image_addr;
   cur_size = image_size;

   /* Iterate Each Sector */
   for (i = 0; i < spi_flash_info[chip].sector_cnt; i++)
   {
      //unsigned int spi_data;

      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | (i * 65536);
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

#if 1
      printk("Erase Sector: %d\n", i);
#endif

      /* Iterate Each Page */
      for (j = 0; j < 256; j++)
      {
         if (cur_size == 0)
            break;

         /* WREN Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = 0x06 << 24;
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

         /* PP Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = (0x02 << 24) | (i * 65536) | (j * 256);

         for (k = 0; k < 64; k++)
         {
            temp = (*(cur_addr)) << 24 | (*(cur_addr + 1)) << 16 | (*(cur_addr + 2)) << 8 | (*(cur_addr + 3));

            spi_ready();
            if (cur_size >= 4)
            {
               *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
               cur_size -= 4;
            }
            else
            {
               *(volatile unsigned int *) SFCSR = LENGTH(cur_size-1) | CS(1+chip) | READY(1);
               cur_size = 0;
            }

            *(volatile unsigned int *) SFDR = temp;

            cur_addr += 4;

            if (cur_size == 0)
               break;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

         /* RDSR Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
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

            if (cnt > 2000)
            {
               printk("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, i*65536+j*256);
busy:
               goto busy;
            }
            cnt++;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

         /* Verify Burned Image */
         /* READ Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
         *(volatile unsigned int *) SFDR = (0x03 << 24) | (i * 65536) | (j * 256);

         for (k = 0; k < 64; k++)
         {
            unsigned int data;

            temp = (*(cur_addr -256 + (k<<2) )) << 24 | (*(cur_addr -256 + (k<<2) + 1)) << 16 | (*(cur_addr - 256 + (k<<2) + 2)) << 8 | (*(cur_addr - 256 + (k<<2) + 3));

            data = *(volatile unsigned int *) SFDR;

            if ((data != temp))
            {
               printk("\nVerify Error at 0x%08X: Now 0x%08X, Expect 0x%08X",
                      i*65536+j*256+(k<<2), data, temp);
halt_here:
               goto halt_here;
            }
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
      }

      if (cur_size == 0)
         break;
   } /* Iterate Each Sector */
}
#endif
//new SPI driver


//#define MTD_SPI_SUZAKU_DEBUG

#if defined(MTD_SPI_SUZAKU_DEBUG)
#define KDEBUG(args...) printk(args)
#else
#define KDEBUG(args...)
#endif

#define write32(a, v)       __raw_writel(v, a)
#define read32(a)           __raw_readl(a)

/* Proto-type declarations */
static u8 spi_read_status(void);
static void spi_set_cs(u32);

#define SPI_ERASING 1

static int spi_state = 0;
static spinlock_t spi_mutex = SPIN_LOCK_UNLOCKED;
//static wait_queue_head_t spi_wq;

//#ifdef CONFIG_RTK_VOIP
spinlock_t spi_lock = SPIN_LOCK_UNLOCKED;
//#endif


/**
 * select which cs (chip select) line to activate
 */
inline static void spi_set_cs(u32 cs)
{
	;//write32(REG_SPISSR, !cs);
}


 static u32 spi_copy_to_dram(const u32 from, const u32 to, const u32 size)
{
//	memcpy(to,from|0xbfc00000,size);
	//memcpy(to,from|FLASH_START_ADDR ,size);
	memcpy(to,from+FLASH_START_ADDR ,size);

	return 0;
}


#if 0
//for test
void spi_read_pio(unsigned int address, unsigned int to, unsigned int size)
{
   unsigned char *buf;
   unsigned int *ptr = (volatile unsigned int *)to;

   /* De-Select Chip */
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

   /* RDSR Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
   *(volatile unsigned int *) SFDR = RDSR << 24;
   while (1)
   {
      unsigned int status;

      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
      {
         break;
      }
   }
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

   /* READ Command */
   spi_ready();
   if(spi_en_4b){
	   	//printk("spi_en_4b:%d\n",spi_en_4b);
		*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
		*(volatile unsigned int *) SFDR = (READ << 24);// | (address & 0xFFFFFF);
		*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
		*(volatile unsigned int *) SFDR = (address & 0xFFFFFFFF);
   }else{
		*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
		*(volatile unsigned int *) SFDR = (READ << 24) | (address & 0xFFFFFF);
   }
	/* Read Data Out */
	//*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
	while(size>=4){
		spi_ready();
		*ptr = *(volatile unsigned int *) SFDR;
		//printk("to:0x%x D:%08x size:%d\n",ptr,*ptr,size);
		ptr++;
		size-=4;		
	}
	//printk("to:0x%x size:%d\n",to,size);
	buf = (unsigned char*)ptr;
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
	while(size>0)
	{
		spi_ready();
		*(buf) = (*(volatile unsigned int *) SFDR) >> 24;
		//printk("to:0x%x D:%x size:%d\n",buf,*buf,size);
		buf++;
		size--;
	}	
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}
//end test
#endif
static u32 do_spi_read(u32 from, u32 to, u32 size)
{
	//DECLARE_WAITQUEUE(wait, current);
	//unsigned long timeo;
	u32 ret;
//#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(&spi_lock, flags);
//#endif
	ret = spi_copy_to_dram(from, to, size);
	spi_ready();
	//spi_pio_init();
//#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(&spi_lock, flags);
//#endif
	return ret;
}


static u32 do_spi_write(u32 from, u32 to, u32 size)
{
   unsigned int temp;
	unsigned int  remain;
   unsigned int cur_addr;
   unsigned int cur_size ,flash_addr;
   unsigned int cnt;
   unsigned int next_page_addr;
 
//#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(&spi_lock, flags);
//#endif
   cur_addr = from;
   flash_addr = to;
   cur_size = size;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_write : from :[%x] to:[%x], size:[%x]  ", from, to, size);
#endif

   	   spi_pio_init();
      next_page_addr = ((flash_addr >> 8) +1) << 8;
	  //printk("%s(%d)\n",__FUNCTION__,__LINE__);
		//___delay(2);
		flush_prediction(10);
      while (cur_size > 0)
      {
      	
	   /* De-select Chip */
	   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	  
         /* WREN Command */
         spi_ready();
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
         *(volatile unsigned int *) SFDR = WREN << 24;
         *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
         /* PP Command */
         spi_ready();
		if(spi_en_4b){
			*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
			*(volatile unsigned int *) SFDR = (PP << 24);
			*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
			*(volatile unsigned int *) SFDR = (flash_addr & 0xFFFFFFFF);
		}else{
         	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
         	*(volatile unsigned int *) SFDR = (PP << 24) | (flash_addr & 0xFFFFFF);
		}
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
         *(volatile unsigned int *) SFDR = RDSR << 24;

         cnt = 0;
         while (1)
         {
            /* RDSR Command */
            if ((*(volatile unsigned int *)SFDR & 0x01000000) == 0x00000000)
            {
                break;
            }

            if (cnt > 200000)
            {
//#ifdef CONFIG_RTK_VOIP
				spin_unlock_irqrestore(&spi_lock, flags);
//#endif
				return -EINVAL;
            }
            cnt++;
         }

         *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
      }
//#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(&spi_lock, flags);
//#endif

	return 0;
}




/*Notice !!!
 * To comply current design, the erase function will implement sector erase
*/
static int do_spi_erase(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_erase : [%x] ", addr);
#endif
//#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(&spi_lock, flags);
//#endif
	spi_pio_init();
	
      /* WREN Command */
      spi_ready();
	  //printk("%s(%d)\n",__FUNCTION__,__LINE__);
	  //___delay(2);
	  flush_prediction(10);
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = WREN << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
	if(spi_en_4b){
		*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
		*(volatile unsigned int *) SFDR = (SE << 24);// | (sector << 12);
		*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
		*(volatile unsigned int *) SFDR = addr;
	}
	else{	  
    	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
      	*(volatile unsigned int *) SFDR = (SE << 24) | addr;
	}
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = RDSR << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
//#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(&spi_lock, flags);
//#endif

	return 0;
}
/*
 The Block Erase function
*/
static int do_spi_block_erase(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_block_erase : [%x] ", addr);
#endif
//#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(&spi_lock, flags);
//#endif
	spi_pio_init();
	
      /* WREN Command */
      spi_ready();
	  //printk("%s(%d)\n",__FUNCTION__,__LINE__);
	  //___delay(2);
	  flush_prediction(10);
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);

      *(volatile unsigned int *) SFDR = WREN << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* BE Command */
      spi_ready();
	if(spi_en_4b){
		*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
	/*0xD8 is block erase command, 2^16 = 64KB*/
		*(volatile unsigned int *) SFDR = (BE << 24);// | (block << 16);	
		*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);	
		*(volatile unsigned int *) SFDR = addr;
	}else{
    	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+chip) | READY(1);
    	*(volatile unsigned int *) SFDR = (BE << 24) | addr;
	}
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+chip) | READY(1);
      *(volatile unsigned int *) SFDR = RDSR << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
         {
            break;
         }
      }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

//#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(&spi_lock, flags);
//#endif

	return 0;
}

static u32 do_spi_block_write(u32 from, u32 to, u32 size)
{
	unsigned char *ptr;

	//don't support write through 1st block
	if ((to < SIZE_64KiB) && ((to+size) > SIZE_64KiB))
		return -EINVAL;
	if (to < SIZE_64KiB)
	{
		ptr = kmalloc(SIZE_64KiB,GFP_KERNEL );
		if (!ptr)
			return -EINVAL;
//		memcpy(ptr,0xbfc00000, SIZE_64KiB);
		memcpy(ptr,FLASH_START_ADDR, SIZE_64KiB);
		do_spi_block_erase(0); // erase 1 sector
		memcpy(ptr+to,from , size);
		do_spi_write(ptr, 0 , SIZE_64KiB);
		kfree(ptr);
		return  0 ;
	}
	else 
		return do_spi_write(from , to, size);
}


//type of SPI flash we support
static const struct spi_flash_info flash_tables[] = {

	{
		mfr_id: SPANSION,
		dev_id: SPI,
		name: "spi_probe",
		DeviceSize: SIZE_2MiB,
		EraseSize: SIZE_64KiB,
	},
// Support for MX2fL series flash 
	{
		mfr_id: 0xC2,
		dev_id: 0x20,
		name: "mxic",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},
// Support EON Flash
	{
		mfr_id: 0x1C,
		dev_id: 0x30,
		name: "EON",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},
// Support EON Flash
	{
		mfr_id: 0x1C,
		dev_id: 0x31,
		name: "EON",
		DeviceSize: 0x200000,
		EraseSize: 4096,
	},	
// Support for MX high performace series flash 
	{
		mfr_id: 0xC2,
		dev_id: 0x5E,
		name: "mxic",
		DeviceSize: 0x400000,
		EraseSize: 4096,
	},
// Support for  winbond W25Q128FVFIG 
	{
		mfr_id: 0xef,
		dev_id: 0x40,
		name: "Winbond",
		DeviceSize: 0x1000000,
		EraseSize: 4096,
	},
// Support for Spansion S25FL128SAGMFI00
	{
		mfr_id: 0x01,
		dev_id: 0x20,
		name: "Spansion",
		DeviceSize: 0x1000000,
		EraseSize: 4096,
	},
#if 0
// Support for Micron N25Q128A
	{
		mfr_id: 0x20,
		dev_id: 0xBA,
		name: "MICRON_N25Q128A",
		DeviceSize: 0x1000000,
		EraseSize: 65536,
	},
#else
// Support for Micron N25Q256A
	{
		mfr_id: 0x20,
		dev_id: 0xBA,
		name: "MICRON_N25Q256A",
		DeviceSize: 0x2000000,
		EraseSize: 4096,
	},

#endif
		
};
static struct spi_chip_info *spi_suzaku_setup(struct map_info *map)
{
	struct spi_chip_info *chip_info;

	chip_info = kmalloc(sizeof(*chip_info), GFP_KERNEL);
	if (!chip_info) {
		printk(KERN_WARNING "Failed to allocate memory for MTD device\n");
		return NULL;
	}

	memset(chip_info, 0, sizeof(struct spi_chip_info));

	return chip_info;
}
static void spi_suzaku_destroy(struct spi_chip_info *chip_info)
{
	printk("spi destroy!\n");
}

//tylo, for 8671b, test IC version
#define CHIPVERMASK (0xFFF00000)
#define IC8672 	(0xFFF00000)   //jim we don't take account of 8672 Version A
#define IC8671B 	(0xFFE00000)
#define IC0315    (0xFFD00000)
#define IC6166    (0xFFC00000)
void checkICver(void){
	//spi controller register settings are moved to arch/mips/realtek/rtl8672/setup.c
	printk("SFCR:0x%x SFCSR:0x%x SFDR:0x%x\n", SFCR, SFCSR, SFDR);
}
/*ccwei: 120411*/
/*write EN4B*/
void do_spi_en4b(void)
{
	//spi_pio_init();
	printk("%s:%d\n\r",__FUNCTION__,__LINE__);
   	spi_ready();
	/* EN4B (enable 4B addr mode) Command */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
   	*(volatile unsigned int *) SFDR = (EN4B << 24);
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	spi_en_4b = 1;
	printk("%s:%d spi_en_4b:%d\n\r",__FUNCTION__,__LINE__, spi_en_4b);
}

void do_spi_ex4b(void)
{
	//spi_pio_init();
	printk("%s:%d\n\r",__FUNCTION__,__LINE__);
   	spi_ready();
	/* EN4B (enable 4B addr mode) Command */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
   	*(volatile unsigned int *) SFDR = (EX4B << 24);
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	spi_en_4b = 0;
	printk("%s:%d spi_en_4b:%d\n\r",__FUNCTION__,__LINE__, spi_en_4b);
}

struct spi_chip_info *spi_probe_flash_chip(struct map_info *map, struct chip_probe *cp)
{
	int i;
	struct spi_chip_info *chip_info = NULL;
	
//#ifdef CONFIG_RTK_VOIP
	int flags;
	spin_lock_irqsave(&spi_lock, flags);
//#endif
	printk("=================================\n");
	printk("luna SPI FLASH driver version 0.7\n");
	printk("=================================\n");
	checkICver();
	spi_pio_init();
	//*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(2);//
	/* 20130107, leave the settings to be filled by U-Boot */
	//*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR & 0x1fffffff;
	//*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(2);
	//*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR  &(~(1<<26));

	spi_cp_probe();
	for (i=0; i < (sizeof(flash_tables)/sizeof(struct spi_flash_info)); i++) {
		//printk("%x   %x",spi_flash_info[0].maker_id ,spi_flash_info[0].type_id );
		if ( (spi_flash_info[0].maker_id == spi_flash_known[i].maker_id) &&
              		(spi_flash_info[0].type_id == spi_flash_known[i].type_id) ) {
			chip_info = spi_suzaku_setup(map);
			if (chip_info) {
				chip_info->flash      = &flash_tables[i];
				if (spi_flash_info[0].maker_id == 0xC2){
					printk("\r\nMXIC matched!!");
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
					printk("\r\nMXIC matched!! i:%d DeviceSize:0x%x\n",i,chip_info->flash->DeviceSize);
					if(spi_flash_info[0].capacity_id == 0x19){
						*(volatile unsigned int *)SPI_EN4B_ADDR = *(volatile unsigned int *) SPI_EN4B_ADDR | SPI_EN4B(1);
						printk("%s: SPI_EN4B_ADDR=0x%x\n",__FUNCTION__, *(volatile unsigned int *)SPI_EN4B_ADDR);
						do_spi_en4b();
					}
				} else if(spi_flash_info[0].maker_id == 0x1C ){					
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
					printk("\r\nEON matched!!\n");
				} else if(spi_flash_info[0].maker_id == 0x20 ){					
					printk("\r\nMicron matched!!");
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
					printk("\r\nMicron matched!! i:%d DeviceSize:0x%x\n",i,chip_info->flash->DeviceSize);
					if(spi_flash_info[0].capacity_id == 0x19){
						*(volatile unsigned int *)SPI_EN4B_ADDR = *(volatile unsigned int *) SPI_EN4B_ADDR | SPI_EN4B(1);
						printk("%s: SPI_EN4B_ADDR=0x%x\n",__FUNCTION__, *(volatile unsigned int *)SPI_EN4B_ADDR);
						do_spi_en4b();
					}
				}				
				chip_info->destroy    = spi_suzaku_destroy;

				chip_info->read       = do_spi_read;
				
				if (flash_tables[i].EraseSize == 4096) //sector or block erase
				{
					chip_info->erase      = do_spi_erase;
					chip_info->write      = do_spi_write;
				}
				else
				{
					chip_info->erase      = do_spi_block_erase;
					chip_info->write      = do_spi_block_write;
				}
			}		
			printk("get SPI chip driver!\n");
//#ifdef CONFIG_RTK_VOIP
			spin_unlock_irqrestore(&spi_lock, flags);
//#endif
			
			return chip_info;
		}
		else{
			printk("can not get SPI chip driver!\n");
		}
	}
//#ifdef CONFIG_RTK_VOIP
	spin_unlock_irqrestore(&spi_lock, flags);
//#endif

	return NULL;
}
EXPORT_SYMBOL(spi_probe_flash_chip);
//module_exit(spi_suzaku_destroy);

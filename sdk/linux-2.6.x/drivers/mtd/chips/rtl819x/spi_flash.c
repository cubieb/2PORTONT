#include <linux/config.h>
#include "spi_flash.h"

#include <linux/mtd/map.h>
#include <linux/mtd/gen_probe.h>
#include <linux/spinlock.h>

unsigned char ICver=0;
/* SPI Flash Controller */
unsigned int SFCR=0;
unsigned int SFCSR=0;
unsigned int SFDR=0;

//#define SPI_DEBUG


struct spi_flash_type   spi_flash_info[2];
/*
 * SPI Flash Info
 */

extern struct spi_flash_db flash_tables[NUM_KNOWN] ;

/*
 * This function shall be called when switching from MMIO to PIO mode
 */
// #define __TESSPIIRAM		__attribute__ ((__section__ (".iram")))
//__TESSPIIRAM 



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

#if 0
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
#endif

 void spi_erase_chip(unsigned int chip)
{
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



void spi_cp_probe(void)
{
   unsigned int cnt, i;
   unsigned int temp;

   for (cnt = 0; cnt < 1; cnt++)
   {
#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
      CHECK_READY;
      SPI_REG(SFCSR_8198) = SPI_CS_INIT;      //deactive CS0, CS1
      CHECK_READY;
      SPI_REG(SFCSR_8198) = 0;                //active CS0,CS1
      CHECK_READY;
      SPI_REG(SFCSR_8198) = SPI_CS_INIT;      //deactive CS0, CS1
      CHECK_READY;
      //unsigned int read_data;
      // Read Flash ID, JSW@20090304: only for 8198 new design
      SPI_REG(SFCSR_8198) = (CS_8198(cnt) | SFCSR_LEN(3) | SFCSR_CMD_BYTE(0x9f));
      temp = (SPI_REG(SFDR2_8198) >> 8);
      *(volatile unsigned int *) SFCSR_8198 = LENGTH(0) | CS(3) | READY(1);
      //JSW:Pre-init 8198 SPI-controller for Set Quad Mode and QE bit
      //__spi_flash_preinit_8198(read_data, cnt);
		
      spi_flash_info[cnt].mfr_id = (temp >> 16) & 0xFF;
      spi_flash_info[cnt].dev_id = (temp >> 8) & 0xFF;
      spi_flash_info[cnt].capacity_id = (temp) & 0xFF;
      CHECK_READY;
#else
      /* Here set the default setting */
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

      /* One More Toggle (May not Necessary) */
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1+cnt) | READY(1);

      /* RDID Command */
      *(volatile unsigned int *) SFDR = 0x9F << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1+cnt) | READY(1);
      temp = *(volatile unsigned int *) SFDR;

      spi_flash_info[cnt].mfr_id = (temp >> 24) & 0xFF;
      spi_flash_info[cnt].dev_id = (temp >> 16) & 0xFF;
      spi_flash_info[cnt].capacity_id = (temp >> 8) & 0xFF;
      spi_ready();
#endif

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* Iterate Each Maker ID/Type ID Pair */
      for (i = 0; i < sizeof(flash_tables) / sizeof(struct spi_flash_db); i++)
      {
         if ( (spi_flash_info[cnt].mfr_id == flash_tables[i].mfr_id) &&
              (spi_flash_info[cnt].dev_id == flash_tables[i].dev_id) )
         {
            spi_flash_info[cnt].device_size = (unsigned char)((signed char)spi_flash_info[cnt].capacity_id + flash_tables[i].size_shift);
         }
      }

      spi_flash_info[cnt].sector_cnt = 1 << (spi_flash_info[cnt].device_size - 16);
   }
   for(i=0;i<2;i++){
	printk("get SPI CS%d\n\r",i);
	printk("maker:%x  type:%x  sector_cnt:%d\n",spi_flash_info[i].mfr_id,spi_flash_info[i].dev_id,spi_flash_info[i].sector_cnt);
   }
}

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

//spinlock_t spi_lock = SPIN_LOCK_UNLOCKED;


/**
 * select which cs (chip select) line to activate
 */
inline static void spi_set_cs(u32 cs)
{
	;//write32(REG_SPISSR, !cs);
}


 static u32 spi_copy_to_dram(const u32 from, const u32 to, const u32 size)
{
	memcpy(to,from|0xbd000000,size);
	
	return 0;
}




static u32 do_spi_read(u32 from, u32 to, u32 size)
{
	//DECLARE_WAITQUEUE(wait, current);
	//unsigned long timeo;
	u32 ret;
	int flags;
	//spin_lock_irqsave(spi_lock,flags);
	ret = spi_copy_to_dram(from, to, size);
#if !defined(CONFIG_RTL_8196C) && !defined(CONFIG_RTL_8198)
	spi_pio_init();
#endif
	//spin_unlock_irqrestore(spi_lock,flags);
	return ret;
}

static u32 do_spi_write_SST(u32 from, u32 to, u32 size)
{
	unsigned int temp;
	unsigned int  remain;
	unsigned char *cur_addr;
	unsigned int cur_size ,flash_addr;
	unsigned int cnt;
	unsigned int next_page_addr;
	unsigned int byte_cnt=0;
	short shift_cnt8=0;
	unsigned int SST_Single_Byte_Data=0,SST_Flash_Offset=0;

	int flags;
	//spin_lock_irqsave(spi_lock,flags);
	cur_addr =(unsigned char*) from;
	flash_addr = to;
	cur_size = size;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_write : from :[%x] to:[%x], size:[%x]  ", from, to, size);
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
         //*(volatile unsigned int *) SFDR = (0x02 << 24) | (flash_addr & 0xFFFFFF);

	   while (flash_addr != next_page_addr)
	   {
		remain = (cur_size > 4)?4:cur_size;
		temp =(unsigned int)(((*cur_addr) << 24 )| ((*(cur_addr + 1)) << 16) | ((*(cur_addr + 2)) << 8) | ((*(cur_addr + 3))));
		for(shift_cnt8=24;shift_cnt8>=0;shift_cnt8-=8)
		{
			 byte_cnt%=4;                        
			 SST_Single_Byte_Data=(temp>>shift_cnt8)&0xff;                        
			 SST_Flash_Offset=flash_addr+byte_cnt;
			 /* RDSR Command */                        
			 spi_ready();                        
			 *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);                       
			 *(volatile unsigned int *) SFDR = 0x05 << 24;                       
			 while (1)
			 {                            
			 	if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)                            
				{                                
					break;                            
				}                        
			  }
			  sst_spi_write(0,SST_Flash_Offset,SST_Single_Byte_Data);                        
			  byte_cnt+=1;
		}
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


/*Notice !!!
 * To comply current design, the erase function will implement sector erase
*/
static int do_spi_erase(u32 addr)
{
	int chip=0;

#ifdef SPI_DEBUG
	printk("\r\n do_spi_erase : [%x] ", addr);
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
      *(volatile unsigned int *) SFDR = (0x20 << 24) | addr;
//	  *(volatile unsigned int *) SFDR = (0xD8 << 24) | addr;
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
		memcpy(ptr,0xbd000000, SIZE_64KiB);
		do_spi_block_erase(0); // erase 1 sector
		memcpy(ptr+to,from , size);
		do_spi_write(ptr, 0 , SIZE_64KiB);
		kfree(ptr);
		return  0 ;
	}
	else 
		return do_spi_write(from , to, size);
}


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

//tylo, for 8196b, test IC version
void checkICver(void){

#ifdef CONFIG_RTL8672	
	printk("SFCR:0x%x SFCSR:0x%x SFDR:0x%x\n", SFCR, SFCSR, SFDR);
#else //CONFIG_RTL8672
		SFCR = 0xB8001200;
#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
		SFCSR= 0xB8001208;
		SFDR = 0xB800120C;
#else
		SFCSR= 0xB8001204;
		SFDR = 0xB8001208;
#endif	
#endif //CONFIG_RTL8672

}

struct spi_chip_info *spi_probe_flash_chip(struct map_info *map, struct chip_probe *cp)
{
	int i;
	struct spi_chip_info *chip_info = NULL;
	
	int flags;
	//spin_lock_irqsave(spi_lock,flags);
	checkICver();
	spi_pio_init();
	//*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(2);//
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR & 0x1fffffff;
#ifdef CONFIG_RTL8672
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(1);
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR  &(~(1<<26));
#else //CONFIG_RTL8672
#if !defined(CONFIG_RTL_8196C) && !defined(CONFIG_RTL_8198)
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR |SPI_CLK_DIV(1);
	*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR  &(~(1<<26));
#endif
#endif //CONFIG_RTL8672
	spi_cp_probe();
	for (i=0; i < (sizeof(flash_tables)/sizeof(struct spi_flash_db)); i++) {
		//printk("%x   %x",spi_flash_info[0].mfr_id ,spi_flash_info[0].dev_id );
		if ( (spi_flash_info[0].mfr_id == flash_tables[i].mfr_id) &&
              		(spi_flash_info[0].dev_id == flash_tables[i].dev_id) ) {
			chip_info = spi_suzaku_setup(map);
			if (chip_info) {
				chip_info->flash      = &flash_tables[i];
				if (spi_flash_info[0].mfr_id == 0xC2){
					printk("\r\nMXIC matched!!");
					chip_info->flash->DeviceSize = 1 << spi_flash_info[0].capacity_id;
				}
				
				chip_info->destroy    = spi_suzaku_destroy;

				chip_info->read       = do_spi_read;
				
				if (flash_tables[i].EraseSize == 4096) //sector or block erase
				{
					chip_info->erase      = do_spi_erase;
					if(spi_flash_info[0].mfr_id == 0xBF)
					{
						chip_info->write      = do_spi_write_SST;
					}
					else
					chip_info->write      = do_spi_write;
				}
				else
				{
					chip_info->erase      = do_spi_block_erase;
					chip_info->write      = do_spi_block_write;
				}
			}		
			printk("get SPI chip driver!\n");
			//spin_unlock_irqrestore(spi_lock,flags);
			
			return chip_info;
		}
		else{
			//printk("can not get SPI chip driver!\n");
		}
	}
	//spin_unlock_irqrestore(spi_lock,flags);

	return NULL;
}
EXPORT_SYMBOL(spi_probe_flash_chip);
//module_exit(spi_suzaku_destroy);

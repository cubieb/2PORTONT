/*
 *	Realtek u-boot SPI Flash driver
 *	
 *	Version 0.0
 */

#include <common.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/byteorder.h>
#include <environment.h>
#include <malloc.h>
#include "spi_flash_rtk.h"

/* FLASH chips info */
flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

/* SPI Flash Info */
struct spi_flash_type	spi_flash_info;

/* Known SPI Flash type */
const struct spi_flash_db	spi_flash_known[] =
{
   {0xC2, 0x20,   0}, /* MXIC */
   {0x01, 0x02,   1}, /* Spansion */
   {0x1C, 0x31,  0}, /* EON */
   {0x1C, 0x30,  0}, /* EON */	 
   {0x8C, 0x20,  0}, /* F25L016A */
   {0xEF, 0x30,  0}, /* W25X16 */
   {0x1F, 0x46,  0}, /* AT26DF161 */
   {0xBF, 0x25,  0}, /* 25VF016B-50-4c-s2AF */
   {0xC8, 0x40, 0}, /*GigaDevice GD25Q16*/
   {0xEF, 0x40, 0}, /*SPANSION S25FL016K*/
};


extern void lx5281_WriteBpctl(uint32);
extern void lx5281_ReadBpctl();

/*	SPI Flash APIs	*/
static inline void spi_ready(void)
{
	while (1)
	{
	  if ( (*(volatile unsigned int *) SFCSR) & READY(1))
		 break;
	} 
}

static void flush_prediction(unsigned int x) {
	 if (x>1)
		 flush_prediction(x-1);
}

 
/*This function shall be called when switching from MMIO to PIO mode */
volatile void spi_pio_init(void)
{
	spi_ready();

	flush_prediction(10); // RDC's suggestion for prediction on 1fc00000 conflict with SPI PIO, andrew
	//toggle_a6();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);

	spi_ready(); __udelay(1);
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);

	spi_ready(); __udelay(1);
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

	spi_ready();
}

void spi_write(unsigned int address, unsigned char data_in)
{
	spi_pio_init();
	
   /* WREN Command */
   spi_ready();
   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
   *(volatile unsigned int *) SFDR = 0x06 << 24;

   *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

	/* BP Command */
   spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
   *(volatile unsigned int *) SFDR = (0x02 << 24) | (address & 0xFFFFFF);
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
   *(volatile unsigned int *) SFDR = (data_in<<24) | 0xFFFFFF;
   
   *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
   
   /* RDSR Command */
   spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
   *(volatile unsigned int *) SFDR = 0x05 << 24;

   while (1)
   {
      unsigned int status;

      status = *(volatile unsigned int *) SFDR;

      /* RDSR Command */
      if ( (status & 0x01000000) == 0x00000000)
         break;
      }
    *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
}


void spi_erase_block(int sector)
{
	spi_pio_init();
	
      /* WREN Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);

      *(volatile unsigned int *) SFDR = 0x06 << 24;
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

      /* SE Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
      *(volatile unsigned int *) SFDR = (0xD8 << 24) | (sector << 16);
      *(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);

      /* RDSR Command */
      spi_ready();
      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
      *(volatile unsigned int *) SFDR = 0x05 << 24;

      while (1)
      {
         /* RDSR Command */
         if ( ((*(volatile unsigned int *) SFDR) & 0x01000000) == 0x00000000)
            break;
         }

      *(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
}

void spi_program_by_byte(unsigned long flash_address ,unsigned char *pData, unsigned int len)
{
	unsigned int idx;
	unsigned int cnt=0;

	for (idx=0; idx<len; idx++) {
		spi_write(flash_address++, pData[idx]);
		if (cnt++ >= 65536) {
			printf(".");
			cnt=0;
		}
	}
}

/* currently this function is dedicate for 64KB flash size and 256B page size*/
void spi_program_by_page(unsigned long flash_address ,unsigned char *image_addr, unsigned int image_size)
{
	unsigned int temp;
	unsigned int i, j, k;
	unsigned char *cur_addr;
	unsigned int cur_size, unalign_size;
	unsigned int cnt;
	unsigned int sect_cnt;

	cur_addr = image_addr;
	cur_size = image_size;

	//printf("flash_address = 0x%08X, image_addr = 0x%08X, image_size = %d\n",
	//	(unsigned int)flash_address, (unsigned int)image_addr, image_size);

	if (flash_address & ((1 << 16) -1))
	{
		unalign_size = SPI_BLOCK_SIZE - ( (unsigned int)flash_address % SPI_BLOCK_SIZE);

		if(unalign_size > cur_size){
			spi_program_by_byte(flash_address, cur_addr, cur_size);
			return;
		}else{
			spi_program_by_byte(flash_address, cur_addr, unalign_size);

			cur_size -= unalign_size;
			cur_addr += unalign_size;
		}
	}

	sect_cnt = flash_address >> 16; //flash_address must be 64KB aligned
	
	/* Iterate Each Sector */
	for (i = sect_cnt; i < spi_flash_info.sector_cnt; i++)
	{
		printf(".");

		/* Iterate Each Page,  64 KB = 256(page size)*256(page number) */
		for (j = 0; j < 256; j++)
		{
			if (cur_size == 0)
				break;

			/* WREN Command */
			spi_ready();
			*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
			*(volatile unsigned int *) SFDR = 0x06 << 24;
			*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);

			/* PP Command */
			spi_ready();
			*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
			*(volatile unsigned int *) SFDR = (0x02 << 24) | (i << 16) | (j << 8);

			for (k = 0; k < 64; k++)
			{
				temp = (*(cur_addr)) << 24 | (*(cur_addr + 1)) << 16 | (*(cur_addr + 2)) << 8 | (*(cur_addr + 3));

				spi_ready();
				if (cur_size >= 4)
				{
					*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
					cur_size -= 4;
				}
				else
				{
					*(volatile unsigned int *) SFCSR = LENGTH(cur_size-1) | CS(1) | READY(1);
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
			*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);
			*(volatile unsigned int *) SFDR = 0x05 << 24;

			cnt = 0;
			while (1)
			{
				unsigned int status = *(volatile unsigned int *) SFDR;

				/* RDSR Command */
				if ((status & 0x01000000) == 0x00000000)
					break;

				if (cnt > 200000)
				{
					printf("\nBusy Loop for RSDR: %d, Address at 0x%08X\n", status, (i<<12)+(j<<8));
				busy:
					goto busy;
				}
				cnt++;
			}
			*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(3) | READY(1);
		}

		if (cur_size == 0)
			break;
	} /* Iterate Each Sector */
}


/*-----------------------------------------------------------------------
 * Initial SPI Flash
 * Init flash device information, probe device, set flash size
 -----------------------------------------------------------------------*/
unsigned long flash_init (void)
{
	unsigned long size = 0;
	int i,j, temp, offset;
	unsigned int flashbase = FLASHBASE;
	unsigned int bpctVal;

	memset(&flash_info, 0, sizeof(flash_info));
	memset(&spi_flash_info, 0, sizeof(struct spi_flash_type));
	
	//*(volatile unsigned int *) SFCR =*(volatile unsigned int *) SFCR | SPI_CLK_DIV(1);
	//*(volatile unsigned int *) SFCR = 0xffc00000;

	lx5281_ReadBpctl(bpctVal);
	lx5281_WriteBpctl(bpctVal &(~0x38));

	spi_pio_init(); 

	lx5281_WriteBpctl(bpctVal);	

	/* Here set the default setting */
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(1) | READY(1);

	/* RDID Command */
	*(volatile unsigned int *) SFDR = 0x9F << 24;
	spi_ready();
	*(volatile unsigned int *) SFCSR = LENGTH(3) | CS(1) | READY(1);
	temp = *(volatile unsigned int *) SFDR;

	spi_flash_info.maker_id = (temp >> 24) & 0xFF;
	spi_flash_info.type_id = (temp >> 16) & 0xFF;
	spi_flash_info.capacity_id = (temp >> 8) & 0xFF;

	*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
	//printf("delay before spi_ready\n\r");
	spi_ready();

	/* Iterate Each Maker ID/Type ID Pair */
	for (i = 0; i < sizeof(spi_flash_known) / sizeof(struct spi_flash_db); i++)
	{
		if ( (spi_flash_info.maker_id == spi_flash_known[i].maker_id) &&
			(spi_flash_info.type_id == spi_flash_known[i].type_id) )
		{
			spi_flash_info.device_size = (unsigned char)((signed char)spi_flash_info.capacity_id + spi_flash_known[i].size_shift);
			break;
		}
	}
	//for atmel and sst flash, its device_size should be re-calculated.
	if (spi_flash_info.maker_id == 0x1F) {
		spi_flash_info.device_size = (spi_flash_info.type_id & 0x1F) + 15;
	}
	else if (spi_flash_info.maker_id == 0xBF) {
		if (spi_flash_info.device_size == 0x41) //identifies the device as SST25VF032B
			spi_flash_info.device_size = 21;
		else
			spi_flash_info.device_size = 0;
	}
	
	spi_flash_info.sector_cnt = ((1<<spi_flash_info.device_size)>>16);

	for(i=0;i<CONFIG_SYS_MAX_FLASH_BANKS;i++){
		flash_info[i].size = (1<<(spi_flash_info.device_size));
		flash_info[i].sector_count = spi_flash_info.sector_cnt;
		flash_info[i].flash_id = ((spi_flash_info.maker_id<<16) | (spi_flash_info.type_id<<8));
		size += flash_info[i].size;

		for(j=0, offset=0;offset<(flash_info[i].size);j++, offset+=SPI_BLOCK_SIZE){
			flash_info[i].start[j] = flashbase+offset;
		}
	}

	if( size == 0){
		printf("NO Flash , the system is blocking\n");
	}

	 if(flash_info[0].size > CONFIG_SYS_MONITOR_LEN){
    		flash_info[0].protect[ CONFIG_SYS_MONITOR_LEN/SPI_BLOCK_SIZE ] = 1;
    	}
	
	puts("SPI ");

	return size;
}

/*-----------------------------------------------------------------------
 * Print Flash Information
 * Print Block Number, Block Start Address, ReadOnly/Writable
 -----------------------------------------------------------------------*/
void flash_print_info  (flash_info_t *info)
{	
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {		
		printf ("missing or unknown FLASH type\n");		
		return;	
	}

	if (info->size >= (1 << 20)) {	
		i = 20;	
	} else {		
		i = 10;	
	}	
	printf ("  Size: %ld %cB in %d Sectors\n",
		info->size >> i,
		(i == 20) ? 'M' : 'k',
		info->sector_count);	

	printf ("  Sector Start Addresses:");	
	for (i=0; i<info->sector_count; ++i) {		
		if ((i % 5) == 0)			
			printf ("\n   ");		

		printf (" %08lX%s",			
			info->start[i],			
			info->protect[i] ? " (RO)" : "     "		);	
	}	printf ("\n");
	
	
	return;
}

/*-----------------------------------------------------------------------
 * Erase SPI Flash
 *       info: target chip information
 *    	  s_first: first block will be erased
 *       s_last: last block will be erased
 * Return value:
 *       0 - OK
 *       1 - Fail
 -----------------------------------------------------------------------*/
int flash_erase (flash_info_t *info, int s_first, int s_last)
{
	int idx;

	//printf("[%s, line %d] s_first = %d, s_last = %d\n",__func__,__LINE__,s_first, s_last);

	for (idx = s_first; idx<=s_last; idx++)
	{
		spi_erase_block(idx);

		 if(s_last!=s_first){
   			printf("\rErase: %3d%%  ",(idx-s_first)*100/(s_last-s_first));
   		}
	}
	
	/*To Do*/
	printf("\rErase: 100%%\n");
	return 0;
}

/*-----------------------------------------------------------------------
* Copy memory to flash, returns:
* 0 - OK
* 1 - write timeout
* 2 - Flash not erased
-------------------------------------------------------------------------*/
int write_buff (flash_info_t * pVol, unsigned char * src, unsigned long addr, unsigned long cnt)
{
	unsigned int dstAddr = (unsigned int) addr;
	unsigned int srcAddr = (unsigned int) src;

	if( 0==cnt ){
		printf("Flash Write cnt=0\n");
		return 1;
	}

	if( (dstAddr-pVol->start[0]+cnt)> pVol->size ){
		printf("Flash Write Out of range\n");
		return 2;
	}

	/* To fit realtek address mapping */
	addr = addr - FLASHBASE;

	switch((pVol->flash_id>>16)&0xff)
	{
		case 0xBF://sst25VF016B-50-4c-s2AF
		case 0x8C://esmt
			printf("Esmt.....\n");
			spi_program_by_byte(addr , src , cnt);
			break;
		default:
			//printf("Starting.....\n");
			spi_program_by_page(addr , src , cnt);
			break;
	}
	
	return 0;
}

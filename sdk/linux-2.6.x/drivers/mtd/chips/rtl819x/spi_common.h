
#ifndef SPI_KERNEL
#define SPI_KERNEL 1
#endif

/*
 * Structure Declaration
 */
struct spi_flash_type
{
   unsigned char mfr_id;
   unsigned char dev_id;
   unsigned char capacity_id;
   unsigned char device_size;        // 2 ^ N (bytes)
   unsigned char sector_cnt;
};


struct spi_flash_db {
	unsigned char mfr_id;
	unsigned char dev_id;
	signed char size_shift;
#ifdef SPI_KERNEL	
	unsigned short extra_id;
	char *name;
	int DeviceSize;
#endif
	int EraseSize;
};

#define SPANSION 0x01
#define SPI          0x02
#define SIZE_64KiB      0x10000



void spi_ready(void);
 #if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
void spi_pio_init_8198(void);
#endif
void spi_pio_init(void);
int do_spi_block_erase(unsigned int addr);
unsigned int do_spi_write(unsigned int from, unsigned int to, unsigned int size);
void sst_spi_write(unsigned int cnt, unsigned int address, unsigned char data_in);


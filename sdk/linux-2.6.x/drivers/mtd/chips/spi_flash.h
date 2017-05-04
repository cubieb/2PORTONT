#ifndef _MTD_SPI_PROBE_H_
#define _MTD_SPI_PROBE_H_
#define ID_MASK         0xffff

#define SPANSION 0x01
#define SPI          0x02

#define SIZE_64KiB      0x10000
#define SIZE_2MiB       0x200000

struct spi_flash_info {
	const unsigned char mfr_id;
	const unsigned char dev_id;
	const unsigned short extra_id;
	const char *name;
	//const int DeviceSize;
	int DeviceSize;
	const int EraseSize;
};

struct spi_chip_info {
	struct spi_flash_info *flash;
	void (*destroy)(struct spi_chip_info *chip_info);

	unsigned int (*read)(unsigned int  from, unsigned int  to, unsigned int  size);
	unsigned int  (*write)(unsigned int  from, unsigned int  to, unsigned int  size);
	int (*erase)(unsigned int  addr);
};

#endif /* _MTD_SPI_PROBE_H_ */
#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_


/*ccwei: spi command description*/
/*write command*/
#define WREN 	0x06 /*write enable, set the write enable latch bit*/
#define WRDI 	0x04 /*write disable, reset the write enable latch bit*/
#define RDSR 	0x05 /*read status register, to read out the values of the status register*/
#define RDID 	0x9f /*read identification, 1byte: Manufacturer id, 2byte: device id*/
#define PP 		0x02 /*page program, to program the selected page*/
#define WRSR 	0x01
/*read command*/
#define READ 	0x03 /*read data, n bytes read out until CS# goes high*/
#define FREAD 	0x0B /*fast read data*/
#define RES 	0xAB /*to read out 1-byte Device ID*/
#define REMS 	0x90 /*output the Manufacturer ID & Device ID*/
/*erase command*/
#define SE 		0x20 /*sector erase*/
#define BE 		0xD8 /*block erase 64KB*/
#define BE32K 	0x52 /*block erase 32KB*/
#define CEA 	0x60 /*erase whole chip*/
#define CEB 	0xC7 /*erase whole chip*/
/*enable 4B addr. mode to access 32MB spi flash*/
/*4B addr cycle*/
#define EN4B	0xB7
#define EX4B	0xE9

/*
 * Macro Definition
 */
#define SPI_CS(i)           ((i) << 30)   /* 0: CS0 & CS1   1: CS0   2: CS1   3: NONE */
#define SPI_LENGTH(i)       ((i) << 28)   /* 0 ~ 3 */
#define SPI_READY(i)        ((i) << 27)   /* 0: Busy  1: Ready */

#define SPI_CLK_DIV(i)      ((i) << 29)   /* 0: DIV_2  1: DIV_4  2: DIV_6 ... 7: DIV_16 */
#define SPI_RD_ORDER(i)     ((i) << 28)   /* 0: Little-Endian  1: Big-Endian */
#define SPI_WR_ORDER(i)     ((i) << 27)   /* 0: Little-Endian  1: Big-Endian */
#define SPI_RD_MODE(i)      ((i) << 26)   /* 0: Fast-Mode  1: Normal Mode */
#define SPI_SFSIZE(i)       ((i) << 23)   /* 0 ~ 7. 128KB * (i+1) */
#define SPI_TCS(i)          ((i) << 19)   /* 0 ~ 15 */
#define SPI_RD_OPT(i)       ((i) << 18)   /* 0: No-Optimization  1: Optimized for Sequential Access */


/*
 * Structure Declaration
 */
struct spi_flash_type
{
   unsigned char maker_id;
   unsigned char type_id;
   unsigned char capacity_id;
   unsigned char device_size;        // 2 ^ N (bytes)
// for flash size >4MiB   unsigned char sector_cnt;
   unsigned int sector_cnt;
};

struct spi_flash_db
{
   unsigned char maker_id;
   unsigned char type_id;
   signed char size_shift;
};


/*
 * Function Prototypes
 */
void spi_pio_init(void);

void spi_read(unsigned int chip, unsigned int address, unsigned int *data_out);
//void spi_write(unsigned int chip, unsigned int address, unsigned int data_in);

void spi_cp_probe(void);
void spi_burn_image(unsigned int chip, unsigned char *image_addr, unsigned int image_size);


#endif /* _SPI_FLASH_H_ */

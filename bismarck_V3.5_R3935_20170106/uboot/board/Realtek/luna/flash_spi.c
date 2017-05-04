/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 9021 $
 * $Date: 2010-04-13 15:45:15 +0800 (Tue, 13 Apr 2010) $
 *
 */
/* for U-Boot Loader */
#include <common.h>
#include <config.h>
#include "flash_spi.h"
#define FLASH_INFO_TYPE flash_info_t
#define OS_PRINTF printf
#define PRINT_DEVICE printf
//#define PRINT_DEVICE(...)

//FLASH_INFO_TYPE __dataflash flash_info[MAX_SPI_FLASH_CHIPS];
FLASH_INFO_TYPE flash_info[MAX_SPI_FLASH_CHIPS];


#ifndef __textflash
	#define __textflash
#endif 

#ifndef __dataflash
	#define __dataflash
#endif



static void __wait_Ready(FLASH_INFO_TYPE *info);
static int __textflash __spi_commands(FLASH_INFO_TYPE *info, spi_request_t *req);
static int __get_spi_SR(FLASH_INFO_TYPE *info);
static int __wait_WEL(FLASH_INFO_TYPE *info);

static uint32 spi_flash_total_size;
static uint32 spi_flash_num_of_chips = 0;

static int spi_en_4b=0;
/*
 *  Set spi_chips as constant can reduce 
 *  the data segment size (For flash only)
 */
//static const spi_chip_info_t spi_chips[] = {
static const spi_chip_info_t spi_chips[] = {
#if 0
	{
		.chip_id = MX25L25635E,
		.chip_size = 0x02000000U,  /* 32MB*/
		.chip_name = "MX25L25635E",
		.io_status  = (IO1|IO2|IO4|CMD_IO1|CIO4|W_ADDR_IO4| \
		    R_ADDR_IO4|R_DATA_IO4|W_DATA_IO4|QE_BIT|MODE_EN),
		.dio_read = 0xbb,
		.dio_mode = 0x00,
		.dio_read_dummy = 0x00,
		.qio_read = 0xeb,
		.qio_mode = 0x00,
		.qio_read_dummy = 0x02,
		.qio_pp = 0x38,
		.qio_eq = 0x00,
		.qio_eq_dummy = 0x00,
		.qio_es = 0xff,
		.qio_wqe_cmd = 0x01,
		.qio_qeb_loc = 6,
		.qio_status_len = 1
	},
#else
	{
		.chip_id = MX25L25735E,
		.chip_size = 0x02000000U,  /* 32MB*/
		.chip_name = "MX25L25735E",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
#endif
	{
		.chip_id = SPI_UNKNOWN,
		.chip_size = 0x00100000U,  /*  1MB*/
		.chip_name = "UNKOWN",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
	{
		.chip_id = MX25L3235D,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "MX25L3235D",
		.io_status  = (IO1|IO2|IO4|CMD_IO1|CIO4|W_ADDR_IO4|
		    R_ADDR_IO4|R_DATA_IO4|W_DATA_IO4|QE_BIT|MODE_EN),
/*		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1| \
		    R_ADDR_IO1|R_DATA_IO1|W_DATA_IO1),*/
		.dio_read = 0xbb,
		.dio_mode = 0x00,
		.dio_read_dummy = 0x00,
		.qio_read = 0xeb,
		.qio_mode = 0x00,
		.qio_read_dummy = 0x02,
		.qio_pp = 0x38,

		.qio_eq = 0x00,
		.qio_eq_dummy = 0x00,
		.qio_es = 0xff,
		.qio_wqe_cmd = 0x01,
		.qio_qeb_loc = 6,
		.qio_status_len = 1
	},
	{
		.chip_id = MX25L3205D,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "MX25L3205D",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},

#if 0
	{
		.chip_id = W25Q32,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "W25Q32",
		.io_status = (IO1|IO2|IO4|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO4\
			                    |R_DATA_IO4|W_DATA_IO4|HAVE_EQ_CMD|QE_BIT|MODE_EN),
//		.io_status = (IO1|IO2|IO4|CMD_IO1|CIO2|W_ADDR_IO1|R_ADDR_IO2|R_DATA_IO2|W_DATA_IO4|HAVE_EQ_CMD|QE_BIT|MODE_EN),
		.dio_read=0xbb,
		.dio_mode = 0x00,
		.dio_read_dummy = 0x00,
			
		.qio_read=0xeb,
		.qio_mode = 0x00,
		.qio_read_dummy=0x02,
		.qio_pp = 0x32,
		.qio_eq = 0xa3,
		.qio_eq_dummy = 0x03,
		.qio_es = 0xff,
		.qio_wqe_cmd = 0x01,
		.qio_qeb_loc = 2,
		.qio_status_len = 2
	},
#endif
	{
		.chip_id = SST26VF032,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "SST26VF032",
		.io_status  = (IO1|IO4|CMD_IO4|CIO1|W_ADDR_IO4| \
		    R_ADDR_IO4|R_DATA_IO4|W_DATA_IO4|HAVE_EQ_CMD),
		.qio_read = 0x0b,
		.qio_mode = 0x00,
		.qio_read_dummy = 0x01,
		.qio_pp = 0x02,
		.qio_eq = 0x38,
		.qio_eq_dummy = 0x00,
		.qio_es = 0xff
	},
	{
		.chip_id = SST25VF032B,
		.chip_size = 0x00400000U,  /*  4MB*/
		.chip_name = "SST25VF032B",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
	{
		.chip_id = MX25L6405D,
		.chip_size = 0x00800000U,  /*  8MB*/
		.chip_name = "MX25L6405D",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
	{
		.chip_id = S25FL064A,
		.chip_size = 0x00800000U,  /*  8MB*/
		.chip_name = "S25FL064A",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
	{
		.chip_id = MX25L8035EM,
		.chip_size = 0x0100000U,  /* 1MB*/
		.chip_name = "MX25L8035EM",
		//.io_status  = (IO1|IO2|IO4|CMD_IO1|CIO4|W_ADDR_IO4|
		.io_status  = (IO1|IO2|IO4|CMD_IO1|CIO1|W_ADDR_IO4| \
		    R_ADDR_IO4|R_DATA_IO4|W_DATA_IO4|QE_BIT|MODE_EN),
		.dio_read = 0xbb,
		.dio_mode = 0x00,
		.dio_read_dummy = 0x00,
		.qio_read = 0xeb,
		.qio_mode = 0x00,
		.qio_read_dummy = 0x02,
		.qio_pp = 0x38,
		.qio_eq = 0x00,
		.qio_eq_dummy = 0x00,
		.qio_es = 0xff,
		.qio_wqe_cmd = 0x01,
		.qio_qeb_loc = 6,
		.qio_status_len = 1
	},
	{
		.chip_id = MX25L12845E,
		.chip_size = 0x01000000U,  /* 16MB*/
		.chip_name = "MX25L12845E",
#ifdef CONFIG_FLASH_SPI_OP_QIO
		.io_status  = (IO1|IO2|IO4|CMD_IO1|CIO4|W_ADDR_IO4| \
		    R_ADDR_IO4|R_DATA_IO4|W_DATA_IO4|QE_BIT|MODE_EN), /* Max. 70MHz */
#else
	#ifdef CONFIG_FLASH_SPI_OP_DIO
		.io_status  = (IO1|IO2|CMD_IO1|CIO2|W_ADDR_IO1| \
		    R_ADDR_IO2|R_DATA_IO2|W_DATA_IO1|QE_BIT|MODE_EN), /* Max. 70MHz */
	#else
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1| \
		    R_ADDR_IO1|R_DATA_IO1|W_DATA_IO1), /* Max. 104MHz */
	#endif
#endif
		.dio_read = 0xbb,
		.dio_mode = 0x00,
		.dio_read_dummy = 0x00,
		.qio_read = 0xeb,
		.qio_mode = 0x00,
		.qio_read_dummy = 0x02,
		.qio_pp = 0x38,
		.qio_eq = 0x00,
		.qio_eq_dummy = 0x00,
		.qio_es = 0xff,
		.qio_wqe_cmd = 0x01,
		.qio_qeb_loc = 6,
		.qio_status_len = 1
	},
	{
		.chip_id = S25FL128P,
		.chip_size = 0x01000000U,  /* 16MB*/
		.chip_name = "S25FL128P",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
        {
                .chip_id = M25P128,
                .chip_size = 0x01000000U,  /* 16MB*/
                .chip_name = "M25P128",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
        },
        {
                .chip_id = S25FL032P,
                .chip_size = 0x00400000U,  /*  4MB*/
                .chip_name = "S25FL032P",
                .io_status  = (IO1|IO2|IO4|CMD_IO1|CIO4|W_ADDR_IO1|R_ADDR_IO4|R_DATA_IO4| \
                    W_DATA_IO4|QE_BIT),
                .dio_read=0xbb,
                .dio_mode = 0x00,
                .dio_read_dummy = 0x01,
                .qio_read=0xeb,
                .qio_mode = 0x00,
                .qio_read_dummy=0x03,
                .qio_pp = 0x32,
                .qio_es = 0xd8,
                .qio_wqe_cmd = 0x01,
                .qio_qeb_loc = 1,
                .qio_status_len = 2
        },
	{
		.chip_id = MX25L1635D,
		.chip_size = 0x00200000U,  /*  2MB*/
		.chip_name = "MX25L1635D",
		.io_status  = (IO1|IO2|IO4|CMD_IO1|CIO1|W_ADDR_IO4| \
		    R_ADDR_IO4|R_DATA_IO4|W_DATA_IO4|QE_BIT|MODE_EN),
/*		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1| \
		    R_ADDR_IO1|R_DATA_IO1|W_DATA_IO1),*/
		.dio_read = 0xbb,
		.dio_mode = 0x00,
		.dio_read_dummy = 0x00,
		.qio_read = 0xeb,
		.qio_mode = 0x00,
		.qio_read_dummy = 0x02,
		.qio_pp = 0x38,

		.qio_eq = 0x00,
		.qio_eq_dummy = 0x00,
		.qio_es = 0xff,
		.qio_wqe_cmd = 0x01,
		.qio_qeb_loc = 6,
		.qio_status_len = 1
		
	},
	{
		.chip_id = MX25L1605D,
		.chip_size = 0x00200000U,  /*  2MB*/
		.chip_name = "MX25L1605D",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
#if 0
        {
                .chip_id = W25Q32,
                .chip_size = 0x00400000U,  /*  4MB*/
                .chip_name = "W25Q32",
                .io_status  = (IO1|IO2|IO4|CMD_IO1|CIO4|W_ADDR_IO1| \
                    R_ADDR_IO4|R_DATA_IO4|W_DATA_IO4|HAVE_EQ_CMD|QE_BIT|MODE_EN),
                .dio_read=0xbb,
                .dio_mode = 0x00,
                .dio_read_dummy = 0x01,
                .qio_read=0xeb,
                .qio_mode = 0x00,
                .qio_read_dummy=0x02,
                .qio_pp = 0x32,
                .qio_eq = 0xa3,
                .qio_eq_dummy = 0x03,
                .qio_es = 0xff,
                .qio_wqe_cmd = 0x01,
                .qio_qeb_loc = 1,
                .qio_status_len = 2
        },
#else
	{
		.chip_id = W25Q32,
		.chip_size = 0x00400000U,  /* 16MB*/
		.chip_name = "W25Q32",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
#endif
	{
		.chip_id = W25Q16,
		.chip_size = 0x00200000U,  /*  2MB*/
		.chip_name = "W25Q16",
		.io_status = (IO1|IO2|IO4|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO4\
			                    |R_DATA_IO4|W_DATA_IO4|HAVE_EQ_CMD|QE_BIT|MODE_EN),
//		.io_status = (IO1|IO2|IO4|CMD_IO1|CIO2|W_ADDR_IO1|R_ADDR_IO2|R_DATA_IO2|W_DATA_IO4|HAVE_EQ_CMD|QE_BIT|MODE_EN),
		.dio_read=0xbb,
		.dio_mode = 0x00,
		.dio_read_dummy = 0x00,
			
		.qio_read=0xeb,
		.qio_mode = 0x00,
		.qio_read_dummy=0x02,
		.qio_pp = 0x32,
		.qio_eq = 0xa3,
		.qio_eq_dummy = 0x03,
		.qio_es = 0xff,
		.qio_wqe_cmd = 0x01,
		.qio_qeb_loc = 2,
		.qio_status_len = 2
	},
	{
		.chip_id = SST26VF016,
		.chip_size = 0x00200000U,  /*  2MB*/
		.chip_name = "SST26VF016",
		.io_status  = (IO1|IO4|CMD_IO4|CIO1|W_ADDR_IO4| \
		    R_ADDR_IO4|R_DATA_IO4|W_DATA_IO4|HAVE_EQ_CMD),
		.qio_read = 0x0b,
		.qio_mode = 0x00,
		.qio_read_dummy = 0x01,
		.qio_pp = 0x02,
		.qio_eq = 0x38,
		.qio_eq_dummy = 0x00,
		.qio_es = 0xff
	},
	{
		.chip_id = W25Q80,
		.chip_size = 0x00100000U,  /*  1MB*/
		.chip_name = "W25Q80",
		.io_status = (IO1|IO2|IO4|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO4\
			                    |R_DATA_IO4|W_DATA_IO4|HAVE_EQ_CMD|QE_BIT|MODE_EN),
//		.io_status = (IO1|IO2|IO4|CMD_IO1|CIO2|W_ADDR_IO1|R_ADDR_IO2|R_DATA_IO2|W_DATA_IO4|HAVE_EQ_CMD|QE_BIT|MODE_EN),
		.dio_read=0xbb,
		.dio_mode = 0x00,
		.dio_read_dummy = 0x00,
			
		.qio_read=0xeb,
		.qio_mode = 0x00,
		.qio_read_dummy=0x02,
		.qio_pp = 0x32,
		.qio_eq = 0xa3,
		.qio_eq_dummy = 0x03,
		.qio_es = 0xff,
		.qio_wqe_cmd = 0x01,
		.qio_qeb_loc = 2,
		.qio_status_len = 2
	},
	{
		.chip_id = S25FL016A,
		.chip_size = 0x00200000U,  /*  2MB*/
		.chip_name = "S25FL016A",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
	{
		.chip_id = S25FL004A,
		.chip_size = 0x00080000U , /*512KB*/
		.chip_name = "S25FL004A",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
	{
		.chip_id = MX25L4005,
		.chip_size = 0x00080000U , /*512KB*/
		.chip_name = "MX25L4005",
		.io_status  = (IO1|CMD_IO1|CIO1|W_ADDR_IO1|R_ADDR_IO1|R_DATA_IO1| \
		    W_DATA_IO1),
	},
};

/*-----------------------------------------------------------------------
 * Read Device ID
 * Return Value: device ID
 -----------------------------------------------------------------------*/
__textflash
static int __read_spi_id(FLASH_INFO_TYPE *info){
	spi_request_t req;
//	__wait_Ready(info);
	CHECK_READY;
	req.cmd_t = SPI_C_RDID;
	req.address = -1;
	return(__spi_commands(info, &req));
}

/*-----------------------------------------------------------------------
 * Enter Serial IO from Quad IO
 * Return Value: None
 -----------------------------------------------------------------------*/
__textflash
static int  __textflash __spi_enter_sio(FLASH_INFO_TYPE *info){
	spi_request_t req;
	CHECK_READY;
	req.cmd_t = SPI_C_RSTQIO;
	req.address = -1;
	return(__spi_commands(info, &req));
}

/*-----------------------------------------------------------------------
 * Enter Dual/Quad IOs from Serial IO
 * Return Value: None
 -----------------------------------------------------------------------*/
__textflash
static int __textflash __spi_enter_ios(FLASH_INFO_TYPE *info){
	spi_request_t req;
	if(info->io_status & IO4){
		info->io_status = (info->io_status&(~IOSTATUS_CIO_MASK)) | CIO4;
	}else if(info->io_status & IO2){
		info->io_status = (info->io_status&(~IOSTATUS_CIO_MASK)) | CIO2;
	}else{
		info->io_status = (info->io_status&(~IOSTATUS_CIO_MASK)) | CIO1;
	}
	req.cmd_t = SPI_C_EMIO;
	req.address = -1;
	return(__spi_commands(info, &req));
}

/*-----------------------------------------------------------------------
 * Read Status Register
 * Return Value: Status
 -----------------------------------------------------------------------*/
static int __textflash __get_spi_SR(FLASH_INFO_TYPE *info)
{
	spi_request_t req;
	uint32 status;
	req.cmd_t = SPI_C_RDSR;
	req.address = -1;
	status = (__spi_commands(info, &req))>>24;
	return(status);
}

/*-----------------------------------------------------------------------
 * Write Status Register
 * Return Value: 0
 -----------------------------------------------------------------------*/
static int __textflash __set_spi_SR(FLASH_INFO_TYPE *info, unsigned char sr_value)
{
	spi_request_t req;
	__wait_WEL(info);
	req.cmd_t = SPI_C_WRSR;
	req.address = sr_value<<16;
	__spi_commands(info, &req);
	return(0);
}

/*-----------------------------------------------------------------------
 * Wait Control Register Ready
 * Wait Writing Progress Over
 * Return Value: None
 -----------------------------------------------------------------------*/
static void __textflash __wait_Ready(FLASH_INFO_TYPE *info)
{
	CHECK_READY;
	if(SPI_VENDOR_SST_QIO==( (info->flash_id)&SPI_VENDOR_DEVICETYPE_MASK)){
		while(__get_spi_SR(info)&SPI_SST_QIO_WIP);
	}else{
		while(__get_spi_SR(info)&SPI_WIP);
	}
}

/*-----------------------------------------------------------------------
 * Write Disable (erase, page program, ..., is not available)
 * Return Value: None
 -----------------------------------------------------------------------*/
static void __textflash __spi_WRDI(FLASH_INFO_TYPE *info)
{
	spi_request_t req;
	req.cmd_t = SPI_C_WRDI;
	__spi_commands(info, &req);
}

/*-----------------------------------------------------------------------
 * Set Write Enable
 * Return Value: Device Status
 -----------------------------------------------------------------------*/
static int __textflash __wait_WEL(FLASH_INFO_TYPE *info)
{
	spi_request_t req_wel;
	uint32 ret;
	CHECK_READY;
	if(SPI_VENDOR_SST_QIO==( (info->flash_id)&SPI_VENDOR_DEVICETYPE_MASK)){
		while(__get_spi_SR(info)&SPI_SST_QIO_WIP);
	}else{
		while(__get_spi_SR(info)&SPI_WIP);
	}
	do{
		/*WREN*/
		req_wel.cmd_t = SPI_C_WREN;
		req_wel.address = -1;
		__spi_commands(info, &req_wel);
		CHECK_READY;
	}while( !((ret=__get_spi_SR(info))&SPI_WEL) );
	return(ret);
}

/*ccwei-4BEN*/
static int __textflash __spi_en_4B(FLASH_INFO_TYPE *info)
{
	spi_request_t req;
	spi_en_4b = 1;
	printf("%s-[%d] spi_en_4b:%d\n",__FUNCTION__,__LINE__, spi_en_4b);
	CHECK_READY;
	req.cmd_t = SPI_C_EN4B;
	req.address = -1;
	return(__spi_commands(info, &req));

}

/*-----------------------------------------------------------------------
 * All SPI commands.
 * Control the SFCSR and SFDR.
 * Support Commands:
 *     Write Enable, Write Disable, Chip Erase, Power Down, 
 *     Write Status Register, Read Status Register, Read Device ID,
 *     Read, Block Erase, Auto Address Increment,
 *     Page Program
 * info: chip information
 * req : request command, address, size, output/input buffer
 -----------------------------------------------------------------------*/
static int __textflash __spi_commands(FLASH_INFO_TYPE *info, spi_request_t *req){
	uint32 addr = 0;
	uint32 size = req->size;
	uint8 *buf = (uint8 *)req->buf;
	uint32 sfcsr_value = 0, sfdr_value = 0, sfcr2_value = SFCR2_HOLD_TILL_SFDR2;
	uint32 ret = 0, io_status = 0, tmp = 0, shift_bit=0;
	uint8 read_cmd, read_mode, read_dummy;
	CHECK_READY;
	SPI_REG(SFCSR) = SPI_CS_INIT; //deactive CS0, CS1
	CHECK_READY;
	SPI_REG(SFCSR) = 0; //active CS0,CS1
	CHECK_READY;
	SPI_REG(SFCSR) = SPI_CS_INIT; //deactive CS0, CS1
	CHECK_READY;
	
	sfcsr_value = ( CS0 & info->flags)?(SPI_eCS0&SPI_LEN_INIT):((SPI_eCS1&SPI_LEN_INIT)|SFCSR_CHIP_SEL);
	sfdr_value = (req->cmd_t)<<24;
	addr = req->address;

	io_status = info->io_status;
	//printf("%s-[%d] io_status:0x%x\n",__FUNCTION__,__LINE__, io_status);	
	if( (io_status)&CIO4 ){
		read_cmd = info->qio_read;
		read_mode = info->qio_mode;
		read_dummy = info->qio_read_dummy;
		shift_bit = 0;
	}else if( (io_status)&CIO2 ){
		read_cmd = info->dio_read;
		read_mode = info->dio_mode;
		read_dummy = info->dio_read_dummy;
		shift_bit = 1;
	}else{
		read_cmd = SPI_C_FREAD; //fast read
		read_mode = 0;
		read_dummy = 1;         //1 byte = 8 dummy cycles
		shift_bit = 2;
	}
	if( SPI_VENDOR_SST_QIO==(info->flash_id&SPI_VENDOR_DEVICETYPE_MASK) ){
		if( (SPI_C_QPP!=req->cmd_t) && \
		    (SPI_C_MREAD!=req->cmd_t) && \
		    (SPI_C_EMIO!=req->cmd_t)  ){
		    	sfcsr_value |= SFCSR_IO_WIDTH(2);
		}
	}
	
	switch(req->cmd_t){
	/*No Address/Dummy Bytes, data length = 1*/
	case SPI_C_WREN:
//printf("%s sfcsr_value:0x%x cmd:0x%x\n",__FUNCTION__,sfcsr_value,req->cmd_t);		
	case SPI_C_WRDI:
	case SPI_C_CE:
	case SPI_C_DP:
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value;
		CHECK_READY;
		SPI_REG(SFDR) = sfdr_value;
		break;
	
	case SPI_C_WRSR:
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN2;
		sfdr_value |= addr;
		CHECK_READY;
		SPI_REG(SFDR) = sfdr_value;
		break;
	case SPI_C_RDSR:
	case SPI_C_RDID:
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value;
		CHECK_READY;
		SPI_REG(SFDR) = sfdr_value;
		CHECK_READY;
		SPI_REG(SFCSR) |= SPI_LEN4;
		CHECK_READY;
		ret = SPI_REG(SFDR);
		break;
	case SPI_C_READ:
		if(spi_en_4b){
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value;
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4;
			CHECK_READY;
			SPI_REG(SFDR) = addr;

		}else{
			sfcsr_value |=SPI_LEN4;
			sfdr_value |= addr;
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value;
		}
		while(size>=4){
			CHECK_READY;
			*((uint32*) buf) = SPI_REG(SFDR);
			buf+=4;
			size-=4;
		}
		sfcsr_value &= SPI_LEN_INIT|SPI_LEN1;
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value;
		while(size>0)
		{
			CHECK_READY;
			*(buf) = SPI_REG(SFDR)>>24;
			buf++;
			size--;
		}
		break;
	case SPI_C_FREAD:
		CHECK_READY;
		if(spi_en_4b){
			/*1 byte cmd*/
			SPI_REG(SFCSR) = sfcsr_value|SPI_LEN1;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value;
			CHECK_READY;
			/*4 byte addr*/
			SPI_REG(SFCSR) = sfcsr_value|SPI_LEN4;
			CHECK_READY;
			SPI_REG(SFDR) = addr;
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value|SPI_LEN1; //dummy cycles
			CHECK_READY;
			SPI_REG(SFDR) = 0;
		}else{
			SPI_REG(SFCSR) = sfcsr_value|SPI_LEN4;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value|addr;
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value|SPI_LEN1; //dummy cycles
			CHECK_READY;
			SPI_REG(SFDR) = 0;
		}
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value|SPI_LEN4;
		
		while(size>=4){
			CHECK_READY;
			*((uint32*) buf) = SPI_REG(SFDR);
			buf+=4;
			size-=4;
		}
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value|SPI_LEN1;
		while(size>0){
			CHECK_READY;
			*(buf) = SPI_REG(SFDR)>>24;
			buf++;
			size--;
		}
		break;
	case SPI_C_BE:
	case SPI_C_SE:
		if(spi_en_4b){
			/*1 byte cmd*/
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value;
			CHECK_READY;
			/*4 byte addr*/
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4;			
			CHECK_READY;
			SPI_REG(SFDR) = addr;
		}else{
			sfcsr_value |=SPI_LEN4;
			sfdr_value |= addr;
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value;
		}
		break;
	case SPI_C_AAI:/*CP: Continously Program*/
//printf("%s SPI_C_AAI: sfdr_value:0x%x, size:%d\n",__FUNCTION__,sfdr_value, size);			
		if(0==size){
			break;
		}
		if(spi_en_4b){
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value;
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4;
			CHECK_READY;
			SPI_REG(SFDR) = addr;
		}else{
			sfdr_value |= addr;
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value;
		}
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN2;
		if(1==size){
			CHECK_READY;
			SPI_REG(SFDR) = (((uint32)(*((uint16*) buf)))<<16 | 0x00FF0000);
			buf+=1;
			size = 0;
			__wait_Ready(info);
			req->cmd_t = SPI_C_WRDI;
			__spi_commands(info, req);
			__wait_Ready(info);
			break;
		}
		CHECK_READY;
		SPI_REG(SFDR) = ((uint32)(*((uint16*) buf)))<<16;
		buf+=2;
		size-=2;
		while(size>=2){
			__wait_Ready(info);
			SPI_REG(SFCSR) = SPI_CS_INIT;
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN3;
			CHECK_READY;
			SPI_REG(SFDR) = (SPI_C_AAI<<24) | ((uint32)(*((uint16*) buf)))<<8;
			buf+=2;
			size-=2;
		}
		while(size>0)
		{
			__wait_Ready(info);
			SPI_REG(SFCSR) = SPI_CS_INIT;
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN3;
			/* or 0x0000FF00 to keep the original data in spi flash (AAI mode only)*/
			CHECK_READY;
			SPI_REG(SFDR) = (SPI_C_AAI<<24) | ((uint32)(*((uint16*) buf)))<<8 | 0x0000FF00;
			buf++;
			size--;
		}
		__wait_Ready(info);
		req->cmd_t = SPI_C_WRDI;
		__spi_commands(info, req);
		__wait_Ready(info);
		break;
	case SPI_C_PP:
		/*Page Program command*/
//printf("%s sfcsr_value:0x%x cmd:0x%x\n",__FUNCTION__,sfcsr_value,req->cmd_t);				
//printf("%s A sfdr_value:0x%x\n",__FUNCTION__,sfdr_value);				
		if(spi_en_4b){
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value;
			CHECK_READY;			
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4;
			CHECK_READY;
			SPI_REG(SFDR) = addr;
		}else{
			sfdr_value |= addr;
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value;
		}
		CHECK_READY;
		/* Check 4byte-aligned issue of buf */
		if(((uint32)buf)%4 != 0){
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1;
			while(size>0 && (((uint32)buf)%4 != 0))
			{
				CHECK_READY;
				SPI_REG(SFDR) = ((uint32)(*buf)) << 24;
				buf++;
				size--;
			}
		}
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4;
		/* Now buf is 4-byte aligned.*/
		while(size>=4){
			CHECK_READY;
			SPI_REG(SFDR) = *((uint32*) buf);
			buf+=4;
			size-=4;
		}
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1;
		while(size>0)
		{
			CHECK_READY;
			SPI_REG(SFDR) = ((uint32)(*buf)) << 24;
			buf++;
			size--;
		}
		__wait_Ready(info);
		break;
	case SPI_C_RES: /*3 bytes of dummy byte*/
		sfcsr_value |=SPI_LEN4;
		break;
	case SPI_C_WBPR: //Write Block Protect Register = 0
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4 | \
		                 SFCSR_IO_WIDTH( (GET_CMD_IO(io_status)-1) );
		CHECK_READY;
		SPI_REG(SFDR) = (sfdr_value);
		CHECK_READY;
		SPI_REG(SFDR) = 0;
		CHECK_READY;
		SPI_REG(SFDR) = 0;
		CHECK_READY;
		SPI_REG(SFDR) = 0;
		CHECK_READY;
		SPI_REG(SFDR) = 0;
		break;
	case SPI_C_EMIO: //enter multi-IO mode
		/*Send Command*/
		CHECK_READY;
		tmp = SPI_REG(SFCR2);
		sfcr2_value |= SFCR2_SIZE(SFCR2_GETSIZE( tmp ));
		sfcr2_value |= tmp & SFCR2_RDOPT;
		sfcr2_value |= SFCR2_CMDIO( GET_CMD_IO(io_status)-1 );
		sfcr2_value |= SFCR2_ADDRIO( GET_R_ADDR_IO(io_status)-1 );
		sfcr2_value |= SFCR2_DATAIO( GET_R_DATA_IO(io_status)-1 );
		sfcr2_value |= SFCR2_SFCMD(read_cmd);	
		if(MODE_EN&io_status){
				sfcr2_value |= SFCR2_DUMMYCYCLE( ((read_dummy)+1)<<shift_bit );
		}else{
				sfcr2_value |= SFCR2_DUMMYCYCLE( (read_dummy)<<shift_bit );
		}
		CHECK_READY;
		SPI_REG(SFCR2) = sfcr2_value;
		
		if( (HAVE_EQ_CMD&io_status) ){
			sfcsr_value |= SFCSR_IO_WIDTH(0);
			sfcsr_value |= SFCSR_LEN(info->qio_eq_dummy);
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value;
			CHECK_READY;
			SPI_REG(SFDR) = sfdr_value;
			CHECK_READY;
		}
		SPI_REG(SFCR2) &= ~(SFCR2_HOLD_TILL_SFDR2);
		break;
	case SPI_C_RSTQIO:
		CHECK_READY;
		tmp = SPI_REG(SFCR2);
		sfcr2_value |= SFCR2_SIZE(SFCR2_GETSIZE( tmp ));
		sfcr2_value |= tmp & SFCR2_RDOPT;
		sfcr2_value |= SFCR2_CMDIO( 0 );
		sfcr2_value |= SFCR2_ADDRIO( 0 );
		sfcr2_value |= SFCR2_DUMMYCYCLE( 4 ); //Serial Fast Read
		sfcr2_value |= SFCR2_DATAIO( 0 );
		sfcr2_value |= SFCR2_SFCMD(SPI_C_FREAD);	
		CHECK_READY;
		SPI_REG(SFCR2) = sfcr2_value;
                CHECK_READY;
                SPI_REG(SFCSR) = sfcsr_value;
                CHECK_READY;
                SPI_REG(SFDR) = sfdr_value;
		CHECK_READY;

		SPI_REG(SFCR2) &= ~(SFCR2_HOLD_TILL_SFDR2);
		io_status &= ~IOSTATUS_CIO_MASK;
		io_status |= CIO1;
		info->io_status = io_status;
		break;
	case SPI_C_QPP: // Only Support WBO=1
		/*Send Command*/
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1 | \
		                 SFCSR_IO_WIDTH( (GET_CMD_IO(io_status)-1) );
		CHECK_READY;
		SPI_REG(SFDR) = ((uint32)(info->qio_pp))<<24;

		/*Send Address*/
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN3 | \
		                 SFCSR_IO_WIDTH( (GET_W_ADDR_IO(io_status)-1) );
		CHECK_READY;
		SPI_REG(SFDR) = (addr<<8);

		                 
		/*Send Dummy*/
		
		/*Send Data*/
		CHECK_READY;
		/* Check 4byte-aligned issue of buf */
		if(((uint32)buf)%4 != 0){
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1 | SFCSR_IO_WIDTH( (GET_W_DATA_IO(io_status)-1) ) ;
			while(size>0 && (((uint32)buf)%4 != 0)){
				CHECK_READY;
				SPI_REG(SFDR) = ((uint32)(*buf)) << 24;
				buf++;
				size--;
			}
		}
		CHECK_READY;
		/* Now buf is 4-byte aligned.*/
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4 | \
		                 SFCSR_IO_WIDTH( (GET_W_DATA_IO(io_status)-1) );
		while(size>=4){
			CHECK_READY;
			SPI_REG(SFDR) = *((uint32*) buf);
			buf+=4;
			size-=4;
		}
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1 | SFCSR_IO_WIDTH( (GET_W_DATA_IO(io_status)-1) ) ;
		while(size>0)
		{
			CHECK_READY;
			SPI_REG(SFDR) = ((uint32)(*buf)) << 24;
			buf++;
			size--;
		}
		__wait_Ready(info);
		break;
	case SPI_C_MREAD: // Only Support RBO=1
		/*Send Command*/
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1 | \
		                 SFCSR_IO_WIDTH( (GET_CMD_IO(io_status)-1) );
		CHECK_READY;
		SPI_REG(SFDR) = ((uint32)(read_cmd))<<24;
		
		/*Send Address*/
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN3 | \
		                 SFCSR_IO_WIDTH( (GET_R_ADDR_IO(io_status)-1) );
		                 
		CHECK_READY;
		SPI_REG(SFDR) = (addr<<8);
		
		/*Send Mode*/
		if((io_status)&MODE_EN){//assume that mode is always 1 byte
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1 | \
		                 SFCSR_IO_WIDTH( (GET_R_ADDR_IO(io_status)-1) );
			CHECK_READY;
			SPI_REG(SFDR) = ((uint32)(read_mode))<<24;
		}

		/*Send Dummy, max dummy bytes = 4 (dummy_cycle=8)*/
		if(read_dummy!=0){
			CHECK_READY;
			SPI_REG(SFCSR) = sfcsr_value | SFCSR_LEN(read_dummy-1) | \
		             SFCSR_IO_WIDTH( (GET_R_ADDR_IO(io_status)-1) );
			CHECK_READY;
			SPI_REG(SFDR) = 0x00000000;
		}
		
		
		/*Read Data*/
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN4 | SFCSR_IO_WIDTH( (GET_R_DATA_IO(io_status)-1) );

		while(size>=4){
			CHECK_READY;
			*((uint32*) buf) = SPI_REG(SFDR);
			buf+=4;
			size-=4;
		}
		CHECK_READY;
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1 | \
		                 SFCSR_IO_WIDTH( (GET_R_DATA_IO(io_status)-1) ) ;
		while(size>0)
		{
			CHECK_READY;
			*(buf) = SPI_REG(SFDR)>>24;
			buf++;
			size--;
		}
		break;
/*ccwei-en4b*/
	case SPI_C_EN4B:
		CHECK_READY;
		//printf("%s:%d\n\r",__FUNCTION__,__LINE__);
		/* EN4B (enable 4B addr mode) Command */
		SPI_REG(SFCSR) = sfcsr_value | SPI_LEN1 ;
		CHECK_READY;
		SPI_REG(SFDR) = sfdr_value;		
   		//*(volatile unsigned int *) SFDR = (EN4B << 24);
		//*(volatile unsigned int *) SFCSR = LENGTH(0) | CS(3) | READY(1);
		//printf("%s:%d\n\r",__FUNCTION__,__LINE__);		
		break;
	default:
		break;
	};
	CHECK_READY;
	SPI_REG(SFCSR) = SPI_CS_INIT; //deactive CS0, CS1
	CHECK_READY;
	SPI_REG(SFCSR) = 0; //active CS0,CS1
	CHECK_READY;
	SPI_REG(SFCSR) = SPI_CS_INIT; //deactive CS0, CS1
	CHECK_READY;
	return ret;
}

/*-----------------------------------------------------------------------
 * Unlock Write-Protect of SST SPI Flash
 *    info: target chip information
 *    val : SST_WRITE_UNLOCK/SST_WRITE_LOCK
 *
 -----------------------------------------------------------------------*/
__textflash
void __spi_write_BlockProtect_Register(FLASH_INFO_TYPE *info, uint32 val){
	spi_request_t req;
	__wait_WEL(info);
	req.cmd_t = SPI_C_WBPR;
	req.address = (val==SST_WRITE_LOCK)?0xFFFFFFFF:0;
	__spi_commands(info, &req);
}
/*-----------------------------------------------------------------------
 * Check SPI Flash IO Status
 *    info: target chip information
 *
 * Assumption: Max Supported IO Width is set when booting(In Assembly Code)
 *
 FLASH_INFO_TYPE: IO related fields
 	ulong	io_status;
	uchar	dio_read;
	uchar	dio_read_dummy;
	uchar	dio_pp; //page program command
	uchar	dio_pp_dummy;
	uchar	dio_mode;
	uchar	qio_read;
	uchar	qio_read_dummy;
	uchar	qio_pp;
	uchar	qio_pp_dummy;
	uchar	qio_mode;     //enhance mode format
	uchar	qio_eq;       //enter quad mode command
	uchar	qio_eq_dummy; //enter quad mode dummy
 -----------------------------------------------------------------------*/
 __textflash
int __spi_init_io_status(FLASH_INFO_TYPE *info){

	static int i = 0;

	if( QE_BIT&(info->io_status) ){
		__wait_Ready(info);
		// WREN
		SPI_REG(SFCSR) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(0)|SFCSR_CMD_BYTE(0x06));
		SPI_REG(SFDR2) = 0;
		// SET QE bit
		if(i == 0){
			SPI_REG(SFCSR) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1| \
		       	    SFCSR_LEN(info->qio_status_len)| \
		            SFCSR_CMD_BYTE(info->qio_wqe_cmd));
		}
		else{
                        SPI_REG(SFCSR) = (SFCSR_CHIP_SEL|SFCSR_SPI_CSB0|SFCSR_SPI_CSB1| \
                            SFCSR_LEN(info->qio_status_len)| \
                            SFCSR_CMD_BYTE(info->qio_wqe_cmd));
		}
		/*
		 * (1<<(info->qio_qeb_loc)): quad enable bit location in the status register
		 * (8*(4-info->qio_status_len)): shift (4-sizeof(status register))*8 bits
		 */
		SPI_REG(SFDR2) = (1<<(info->qio_qeb_loc))<<(8*(4-info->qio_status_len))  ;
		__wait_Ready(info);
	}
	__spi_enter_ios(info);
	if(SPI_VENDOR_SST_QIO==( (info->flash_id)&SPI_VENDOR_DEVICETYPE_MASK)){
		__spi_write_BlockProtect_Register(info, SST_WRITE_UNLOCK);
	}

	i++;
	return 0;
}

/*-----------------------------------------------------------------------
 * Write SPI Flash
 *     info: target chip information
 *     buff: source address
 *     size: input data size
 * start_addr: flash destination address, (0 ~ Flash_Size-4) is valid value
 -----------------------------------------------------------------------*/
static int __textflash __write_spi_flash(FLASH_INFO_TYPE *info, const uint8 *buff, uint32 size, void *start_addr){
	spi_request_t req;
	uint32 offset = 0;
#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
	uint32 i = 0;
	uint32 org_size = size;
#endif /*SPI_SHOW_PROGRESS*/
	/*WREN*/
	if(SST25VF032B==info->flash_id){
		__set_spi_SR(info, 0);
	}

	/*Page Program*/
	if(SST25VF032B==(info->flash_id&0x00FFFFFF)){
		req.address = (uint32) start_addr;
		req.buf = (uint8 *)buff;
		req.size = size;
		req.cmd_t = SPI_C_AAI;
		__spi_commands(info, &req);
		return 0;
	}else{
		if( (info->io_status&(CIO4)) ){
			req.cmd_t = SPI_C_QPP;
		}else{
			req.cmd_t = SPI_C_PP;
		}
	}

#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
	OS_PRINTF("\n");
#endif /*SPI_SHOW_PROGRESS*/
	/*start_addr is not aligned on SPI_MAX_TRANSFER_SIZE*/
	if( 0!=( (uint32)start_addr%SPI_MAX_TRANSFER_SIZE)){
		__wait_WEL(info);
		req.address = (uint32) start_addr;
		req.buf = (uint8 *)buff;
		req.size = SPI_MAX_TRANSFER_SIZE-((uint32)start_addr%SPI_MAX_TRANSFER_SIZE);
		if(size<req.size){
			req.size = size;
		}
		size -= req.size;
		offset += req.size;
		__spi_commands(info, &req);
	}
	/*start_addr+offset is aligned on SPI_MAX_TRANSFER_SIZE*/
	while(size>=SPI_MAX_TRANSFER_SIZE)
	{
#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
		if(0==i%10){
			OS_PRINTF("\rWrite: %3d%%", offset*100/org_size);
		}
		i++;
#endif /*SPI_SHOW_PROGRESS*/
		__wait_WEL(info);
		req.address = (uint32) start_addr + offset;
		req.buf = (uint8 *)(buff+offset);
		req.size = SPI_MAX_TRANSFER_SIZE;
		size -= SPI_MAX_TRANSFER_SIZE;
		offset += SPI_MAX_TRANSFER_SIZE;
		__spi_commands(info, &req);
	}
	__wait_WEL(info);
	if(0!=size){
		req.address = ((uint32) start_addr) + offset;
		req.buf = (uint8 *)(buff+offset);
		req.size = size;
		__spi_commands(info, &req);
	}
#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
	OS_PRINTF("\rWrite: 100%%\n");
#endif /*SPI_SHOW_PROGRESS*/
	return 0;
}


/*-----------------------------------------------------------------------
 * Erase SPI Flash
 *    info: target chip information
 *       set: flash erase command, SPI_C_BE and SPI_C_CE are valid value
 *      addr: flash destination address, (0 ~ Flash_Size-1) is valid value
 -----------------------------------------------------------------------*/
static int __textflash __erase_spi_flash(FLASH_INFO_TYPE *info, spi_cmdType_t set, void *addr){
	spi_request_t req;
	__wait_WEL(info);
	req.cmd_t = set;
	req.address = (uint32) addr;
	__spi_commands(info, &req);
	return(0);
}

/*-----------------------------------------------------------------------
 * Invoke by lowlevel_init.S
 * Set Quad Mode and QE bit
 -----------------------------------------------------------------------*/
//__textflash
unsigned long __spi_flash_preinit (uint32 ra)
{
	uint32 read_data;
	
	// Read Flash ID
	SPI_REG(SFCSR) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(3)|SFCSR_CMD_BYTE(0x9f));
	read_data = (SPI_REG(SFDR2)>>8);
    switch( read_data&0x00FFFFFF ){
	case MX25L1635D:
	case MX25L3235D:
		// Read EQ bit
		SPI_REG(SFCSR) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(1)|SFCSR_CMD_BYTE(0x05));
		read_data = SPI_REG(SFDR2);
		if(read_data&0x40000000){ // MXIC Quad Enable Bit
			SPI_REG(SFCR2) = (SFCR2_SFCMD(0xeb)|SFCR2_SIZE(7)|SFCR2_RDOPT|SFCR2_ADDRIO(2)|SFCR2_DUMMYCYCLE(3)|SFCR2_DATAIO(2));
		}else{
			SPI_REG(SFCR2) = (SFCR2_SFCMD(0x0b)|SFCR2_SIZE(7)|SFCR2_RDOPT|SFCR2_ADDRIO(0)|SFCR2_DUMMYCYCLE(4)|SFCR2_DATAIO(0));
		}
        break;
    case SST26VF016:
	case SST26VF032:
		// Set MMIO Controller Register
		SPI_REG(SFCR2) = (SFCR2_SFCMD(0x0b)|SFCR2_SIZE(7)|SFCR2_RDOPT|SFCR2_CMDIO(2)|SFCR2_ADDRIO(2)|SFCR2_DUMMYCYCLE(1)|SFCR2_DATAIO(2)|SFCR2_HOLD_TILL_SFDR2);
		
		// Enter Quad Mode
		SPI_REG(SFCSR) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(0)|SFCSR_CMD_BYTE(0x38));
		SPI_REG(SFDR2) = 0x02000000;
        break;
    case W25Q80:
    case W25Q16:
 //   case W25Q32:
		// Read EQ bit
		SPI_REG(SFCSR) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(1)|SFCSR_CMD_BYTE(0x35));
		read_data = SPI_REG(SFDR2);
		if(read_data&0x02000000){ // MXIC Quad Enable Bit
			// Enter High Performane Mode
			SPI_REG(SFCSR) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(3)|SFCSR_CMD_BYTE(0xA3));
			SPI_REG(SFDR2) = 0x00000000;
			SPI_REG(SFCR2) = (SFCR2_SFCMD(0xeb)|SFCR2_SIZE(7)|SFCR2_RDOPT|SFCR2_ADDRIO(2)|SFCR2_DUMMYCYCLE(3)|SFCR2_DATAIO(2));
		}else{
			SPI_REG(SFCR2) = (SFCR2_SFCMD(0x0b)|SFCR2_SIZE(7)|SFCR2_RDOPT|SFCR2_ADDRIO(0)|SFCR2_DUMMYCYCLE(4)|SFCR2_DATAIO(0));
		}
    	break;

    case S25FL032P:
                //Read Configuration Register
                SPI_REG(SFCSR) = (SFCSR_SPI_CSB0|SFCSR_SPI_CSB1|SFCSR_LEN(1)|SFCSR_CMD_BYTE(0x35));
                read_data = SPI_REG(SFDR2);
                if(read_data&0x02000000){//If Quad enable bit set.
                        SPI_REG(SFCR2) = (SFCR2_SFCMD(0xeb)|SFCR2_SIZE(7)|SFCR2_RDOPT|SFCR2_ADDRIO(2)|SFCR2_DUMMYCYCLE(3)|SFCR2_DATAIO(2));

                }else{
                        SPI_REG(SFCR2) = (SFCR2_SFCMD(0x0b)|SFCR2_SIZE(7)|SFCR2_RDOPT|SFCR2_ADDRIO(0)|SFCR2_DUMMYCYCLE(4)|SFCR2_DATAIO(0));
                }

        break;

	default:
		break;
	}
	read_data = SPI_REG(SFCR2) & (~SFCR2_HOLD_TILL_SFDR2);
	SPI_REG(SFCR2) = read_data;
	return ra;
}

/*-----------------------------------------------------------------------
 * Initial SPI Flash
 * Init flash device information, probe device, set flash size
 -----------------------------------------------------------------------*/
__textflash
unsigned long flash_init (void)
{
	int i=0, j=0, offset=0;
	int flashbase = FLASHBASE;
	spi_flash_total_size= 0;
	unsigned int spi_clk_div;

	/* configure spi clock */
	spi_clk_div = ((board_DRAM_freq_mhz()/CONFIG_FLASH_SPI_MAXCLK)/2);
	SPI_REG(SFCR) = (SPI_REG(SFCR) & 0x1FFFFFFF) | (spi_clk_div << 29);

	SPI_REG(SFCR) |= (SFCR_EnableWBO);
	SPI_REG(SFCR) |= (SFCR_EnableRBO);
	
	for(i=0;i<MAX_SPI_FLASH_CHIPS;i++){
		flash_info[i].flags = (CS0<<i)|R_MODE;
		__spi_WRDI(&flash_info[i]);
		flash_info[i].qio_es = 0xff;
		__spi_enter_sio(&flash_info[i]);
	}

    for(i=0;i<MAX_SPI_FLASH_CHIPS;i++){
        flash_info[i].flash_id = __read_spi_id(&flash_info[i])>>8;
//PRINT_DEVICE("Probe: SPI CS%d Flash id 0x%x\n", i, flash_info[i].flash_id);		
        for(j=0;j<sizeof(spi_chips)/sizeof(spi_chip_info_t);j++){
        	if( (flash_info[i].flash_id&0x00FFFFFF) == (spi_chips[j].chip_id) ){
//        		PRINT_DEVICE("Probe: SPI CS%d Flash Type %s size:0x%x\n", i, spi_chips[j].chip_name, spi_chips[j].chip_size);

				if((flash_info[i].flash_id&0x00FFFFFF) == MX25L25735E){
					*(volatile unsigned int *)PSR = *(volatile unsigned int *) PSR | SPI_EN4B(1);
					PRINT_DEVICE("%s: PSR=0x%x\n",__FUNCTION__, *(volatile unsigned int *)PSR);
					__spi_en_4B(&flash_info[i]);
				}
        			flash_info[i].size 		= spi_chips[j].chip_size;
				flash_info[i].dio_read		= spi_chips[j].dio_read;
				flash_info[i].dio_mode		= spi_chips[j].dio_mode; //mode command, default:disable enhance mode
				flash_info[i].dio_read_dummy 	= spi_chips[j].dio_read_dummy;
				flash_info[i].io_status  	= spi_chips[j].io_status;

				flash_info[i].qio_read   	= spi_chips[j].qio_read;
				flash_info[i].qio_mode   	= spi_chips[j].qio_mode; //mode command, default:disable enhance mode
				flash_info[i].qio_read_dummy 	= spi_chips[j].qio_read_dummy;
				flash_info[i].qio_pp     	= spi_chips[j].qio_pp;
				flash_info[i].qio_eq     	= spi_chips[j].qio_eq;
				flash_info[i].qio_eq_dummy 	= spi_chips[j].qio_eq_dummy;
				flash_info[i].qio_es     	= spi_chips[j].qio_es;
				
				flash_info[i].qio_wqe_cmd     	= spi_chips[j].qio_wqe_cmd;
				flash_info[i].qio_qeb_loc     	= spi_chips[j].qio_qeb_loc;
				flash_info[i].qio_status_len    = spi_chips[j].qio_status_len;
				
        		break;
        	}
        }

        if( j==(sizeof(spi_chips)/sizeof(spi_chip_info_t)) ){
        	continue;
        }
		for(j=0, offset=0;offset<(flash_info[i].size);j++, offset+=SPI_BLOCK_SIZE){
			flash_info[i].start[j] = flashbase+offset;
		}
		if(0==i){
			SPI_REG(SFCR2) &= ~(SFCR2_SIZE(0x7));
			switch(flash_info[i].size){
				case 0x00020000U:
					SPI_REG(SFCR2) |= SFCR2_SIZE(0x0);
					break;
				case 0x00040000U:
					SPI_REG(SFCR2) |= SFCR2_SIZE(0x1);
					break;
				case 0x00080000U:
					SPI_REG(SFCR2) |= SFCR2_SIZE(0x2);
					break;
				case 0x00100000U:
					SPI_REG(SFCR2) |= SFCR2_SIZE(0x3);
					break;
				case 0x00200000U:
					SPI_REG(SFCR2) |= SFCR2_SIZE(0x4);
					break;
				case 0x00400000U:
					SPI_REG(SFCR2) |= SFCR2_SIZE(0x5);
					break;
				case 0x00800000U:
					SPI_REG(SFCR2) |= SFCR2_SIZE(0x6);
					break;
				case 0x01000000U:
					SPI_REG(SFCR2) |= SFCR2_SIZE(0x7);
					break;
				default:
					SPI_REG(SFCR2) |= SFCR2_SIZE(0x7);
					break;
			}
		}

		__spi_init_io_status(&flash_info[i]);
		flash_info[i].sector_count = flash_info[i].size/SPI_BLOCK_SIZE;
		flashbase += flash_info[i].size;
		spi_flash_total_size += flash_info[i].size;
		spi_flash_num_of_chips++;
    }
	if(0==spi_flash_num_of_chips){
		/* Configure to UNKNOWN SPI */
		OS_PRINTF("Warnning: No SPI Flash Detected!!!\n");
		OS_PRINTF("Confiure to UNKNOWN SPI, Serial Mode, Size 0x100000\n");
        		flash_info[0].size = spi_chips[0].chip_size;
				flash_info[0].dio_read   = spi_chips[0].dio_read;
				flash_info[0].dio_mode   = spi_chips[0].dio_mode; //mode command, default:disable enhance mode
				flash_info[0].dio_read_dummy = spi_chips[0].dio_read_dummy;
				flash_info[0].io_status  = spi_chips[0].io_status;

				flash_info[0].qio_read   = spi_chips[0].qio_read;
				flash_info[0].qio_mode   = spi_chips[0].qio_mode; //mode command, default:disable enhance mode
				flash_info[0].qio_read_dummy = spi_chips[0].qio_read_dummy;
				flash_info[0].qio_pp     = spi_chips[0].qio_pp;
				flash_info[0].qio_eq     = spi_chips[0].qio_eq;
				flash_info[0].qio_eq_dummy = spi_chips[0].qio_eq_dummy;
				flash_info[0].qio_es     = spi_chips[0].qio_es;
				
				flash_info[0].qio_wqe_cmd     = spi_chips[0].qio_wqe_cmd;
				flash_info[0].qio_qeb_loc     = spi_chips[0].qio_qeb_loc;
				flash_info[0].qio_status_len     = spi_chips[0].qio_status_len;

				spi_flash_total_size = spi_chips[0].chip_size;
	}
    if( 0!=(CONFIG_ENV_OFFSET%SPI_BLOCK_SIZE) ){
    	OS_PRINTF("CONFIG_ENV_OFFSET=0x%08x is defined error!\n", CONFIG_ENV_OFFSET);
    	
    }
    if( 0==spi_flash_total_size ){
    	OS_PRINTF("No Flash, the system is blocking!\n");
    	while(1);
    }
    if( spi_flash_total_size<CONFIG_ENV_OFFSET ){
    	OS_PRINTF("CONFIG_ENV_OFFSET must be aligned to 0x%08x, smaller than 0x%08x", SPI_BLOCK_SIZE, spi_flash_total_size);
    	while(1);
    }
    if(flash_info[0].size > CONFIG_ENV_OFFSET){
    	flash_info[0].protect[ CONFIG_ENV_OFFSET/SPI_BLOCK_SIZE ] = 1;
    }else{
    	flash_info[1].protect[(CONFIG_ENV_OFFSET-flash_info[0].size)/SPI_BLOCK_SIZE] = 1;
    }
	return spi_flash_total_size;
}

/*-----------------------------------------------------------------------
 * Print Flash Information
 * Print Block Number, Block Start Address, ReadOnly/Writable
 -----------------------------------------------------------------------*/
void flash_print_info (FLASH_INFO_TYPE * info){
	int i;
	OS_PRINTF("SPI FLASH:\n");
	OS_PRINTF("size:%d KB\n", info->size>>10);
	for(i=0;i< (info->size/SPI_BLOCK_SIZE);i++){
		if(info->protect[i]){
			OS_PRINTF("[%03d:%08x]   RO   ", i, (info->start[i]));
		}else{
			OS_PRINTF("[%03d:%08x]        ", i, (info->start[i]));
		}
		if(0==((i+1)%5) && i!=0){
			OS_PRINTF("\n");
		}
	}
//	if(0!=((i+1)%5)){
		OS_PRINTF("\n");
//	}
}

/*-----------------------------------------------------------------------
 * Erase SPI Flash
 *       info: target chip information
 *    s_first: first block will be erased
 *     s_last: last block will be erased
 * Return value:
 *    0 - OK
 *    1 - Fail
 -----------------------------------------------------------------------*/
int __textflash flash_erase (FLASH_INFO_TYPE * info, int s_first, int s_last){
	uint32 offset;
	uint32 addr = info->start[0] ;
	uint32 index;
	uint32 i;
	
	if( (s_first>=info->sector_count) || (s_last>=info->sector_count)){
		OS_PRINTF("[%s][%s][%d]Out of Rande, Addr=%08x\n", \
			__FILE__, __FUNCTION__, __LINE__, addr);
		while(1);
	}
	offset = s_first*SPI_BLOCK_SIZE;
	index = s_first;
    
	for(;offset<=s_last*SPI_BLOCK_SIZE;offset+=SPI_BLOCK_SIZE, index++){
		if(info->protect[index]!=0){
#if !defined(CONFIG_FLASH_SRAM_ONLY)
			OS_PRINTF("Skip Readonly Block on block[%03d]\n", index);
			continue;
#endif
		}
		if(SPI_VENDOR_SST_QIO==( (info->flash_id)&SPI_VENDOR_DEVICETYPE_MASK)){
			for(i=0;i<SPI_BLOCK_SIZE/SPI_SECTOR_SIZE;i++){
				__erase_spi_flash(info, SPI_C_SE, (void*) (offset+(SPI_SECTOR_SIZE*i)));
			}
		}else{
   			__erase_spi_flash(info, SPI_C_BE, (void*) (offset));
   		}
#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
   		if(s_last!=s_first){
   			OS_PRINTF("\rErase: %3d%%  ",(index-s_first)*100/(s_last-s_first));
   		}
#endif /*SPI_SHOW_PROGRESS*/
	}
	__wait_Ready(info);
#if defined(SPI_SHOW_PROGRESS)&&!defined(CONFIG_FLASH_SRAM_ONLY)
	OS_PRINTF("\rErase: 100%%\n");
#endif /*SPI_SHOW_PROGRESS*/
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to SPI Flash, 
 * addr: flash destination address, (FLASHBASE+(0 ~ Flash_Size-4)) is valid value
 * returns value:
 *     0 - OK
 *     1 - write timeout
 *     2 - Flash not erased
 -----------------------------------------------------------------------*/
int __textflash write_buff (FLASH_INFO_TYPE * info, uchar * src, ulong addr, ulong cnt){
	uint32 dstAddr = (uint32) addr;
	uint32 srcAddr = (uint32) src;


	if( 0==cnt ){
		OS_PRINTF("Flash Write cnt=0\n");
		return 1;
	}

	if( (dstAddr-info->start[0]+cnt)> info->size ){
		OS_PRINTF("Flash Write Out of range\n");
		return 2;
	}

    __write_spi_flash(info, 
		(uint8*) (srcAddr), 
		(uint32)cnt, 
		(void *) (dstAddr-(info->start[0])));
	
	return 0;
}

#if defined(CONFIG_FLASH_SRAM_ONLY)
/*-----------------------------------------------------------------------
 * Memory Copy for updating loader
 * As CONFIG_FLASH_SRAM_ONLY is defined, 
 * the original memcpy code segment is in flash.
 * This function is used when CONFIG_FLASH_SRAM_ONLY is defined
 * returns value: none
 -----------------------------------------------------------------------*/
static void __textflash flashonly_memcpy(void *dst, void* src, ulong size){
	int i;
	for(i=0;i<size;i++){
		*(((char*)dst)+i) = *(((char*)src)+i);
	}
}

/*-----------------------------------------------------------------------
 * Update loader when CONFIG_FLASH_SRAM_ONLY is defined
 * This function is never return. 
 * This function reboot by jumping to flash start address
 * returns value: 1
 -----------------------------------------------------------------------*/
int __textflash update_loader(ulong addr_first, ulong size){
	int i;
	unsigned char buf[128];
	void (*f)(void) = (void *) (0xbfc00000);
	unsigned int offset = 0;
	if(size>flash_info[0].size){
		OS_PRINTF("Outside available Flash\n");
		return (-1);
	}
	for(i=0;i<CFG_MAX_FLASH_SECT;i++){
		flash_info[0].protect[i] = 0;
	}
	for(i=0;i<flash_info[0].sector_count;i++){
		if( (flash_info[0].start[i]-FLASHBASE) > size){
			flash_erase( &flash_info[0], 0, i);
			break;
		}
	}
	
	for(i=0;i<=size;i+=sizeof(buf)){
		if( (size-i)>= sizeof(buf)){
			flashonly_memcpy(buf, addr_first+i, sizeof(buf));
			write_buff (&flash_info[0], buf, (FLASHBASE+i+offset), sizeof(buf));
		}else if(0!=(size-i)){
			flashonly_memcpy(buf, addr_first+i, size-i);
			write_buff (&flash_info[0], buf, (FLASHBASE+i+offset), size-i);
		}
	}
	
	f();
	return 1;
}
#endif /*CONFIG_FLASH_SRAM_ONLY*/


/******************************************************************************
 * $Id: nand_base_sd5.c,v 1.7 2012/08/06 07:14:18 ccwei0908 Exp $
 * drivers/mtd/nand/nand_base_rtk.c
 * Overview: Realtek MTD NAND Driver 
 * Copyright (c) 2008 Realtek Semiconductor Corp. All Rights Reserved.
 * Modification History:
 *    #000 2008-06-10 Ken-Yu   create file
 *    #001 2008-09-10 Ken-Yu   add BBT and BB management
 *    #002 2008-09-28 Ken-Yu   change r/w from single to multiple pages
 *    #003 2008-10-09 Ken-Yu   support single nand with multiple dies
 *    #004 2008-10-23 Ken-Yu   support multiple nands
 *
 *******************************************************************************/
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/bitops.h>

// Ken-Yu 
#include <mtd/mtd-abi.h>
//#include <linux/mtd/rtk_nand_reg.h>
//#include <linux/mtd/rtk_nand.h>
//#include <asm/r4kcache.h>
#include <asm/page.h>
#include <linux/jiffies.h>
#include "bspchip.h"
#include "./rtk_nand.h"
//#include <asm/mach-venus/platform.h>

//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
//int rtk_update_bbt (struct mtd_info *mtd, __u8 *data_buf, __u8 *oob_buf, BB_t *bbt);
int rtk_update_bbt (struct mtd_info *mtd, BB_t *bbt);

#endif

#define Nand_Block_Isbad_Slow_Version 0
 
#define NOTALIGNED(mtd, x) ((x & (mtd->writesize-1)) != 0)   //czyao, 2010/0917

#define check_end(mtd, addr, len)					\
do {									\
	if (mtd->size == 0) 						\
		return -ENODEV;						\
	else								\
	if ((addr + len) > mtd->size) {					\
		printk (				\
			"%s: attempt access past end of device\n",	\
			__FUNCTION__);					\
		return -EINVAL;						\
	}								\
} while(0)

//czyao, 2010/0917
#define check_page_align(mtd, addr)					\
do {									\
	if (addr & (mtd->writesize - 1)) {				\
		printk (				\
			"%s: attempt access non-page-aligned data\n",	\
			__FUNCTION__);					\
		printk (				\
			"%s: mtd->writesize = 0x%x\n",			\
			__FUNCTION__,mtd->writesize);			\
		return -EINVAL;						\
	}								\
} while (0)

#define check_block_align(mtd, addr)					\
do {									\
	if (addr & (mtd->erasesize - 1)) {				\
		printk (				\
			"%s: attempt access non-block-aligned data\n",	\
			__FUNCTION__);					\
		return -EINVAL;						\
	}								\
} while (0)

#define check_len_align(mtd,len)					\
do {									\
	if (len & (512 - 1)) {          	 			\
		printk (				\
               	      "%s: attempt access non-512bytes-aligned mem\n",	\
			__FUNCTION__);					\
		return -EINVAL;						\
	}								\
} while (0)


/* Realtek supports nand chip types */
/* Micorn */
#define MT29F2G08AAD	0x2CDA8095	//SLC, 128 MB, 1 dies
#define MT29F2G08ABAE	0x2CDA9095  //SLC, 256MB, 1 dies

/* STMicorn */
#define NAND01GW3B2B	0x20F1801D	//SLC, 128 MB, 1 dies
#define NAND01GW3B2C	0x20F1001D	//SLC, 128 MB, 1 dies, son of NAND01GW3B2B
#define NAND02GW3B2D	0x20DA1095	//SLC, 256 MB, 1 dies
#define NAND04GW3B2B	0x20DC8095	//SLC, 512 MB, 1 dies
#define NAND04GW3B2D	0x20DC1095	//SLC, 512 MB, 1 dies
#define NAND04GW3C2B	0x20DC14A5	//MLC, 512 MB, 1 dies
#define NAND08GW3C2B	0x20D314A5	//MLC, 1GB, 1 dies

/* Hynix Nand */
#define HY27UF081G2A	0xADF1801D	//SLC, 128 MB, 1 dies
#define HY27UF082G2A	0xADDA801D	//SLC, 256 MB, 1 dies
#define HY27UF082G2B	0xADDA1095	//SLC, 256 MB, 1 dies
#define HY27UF084G2B	0xADDC1095	//SLC, 512 MB, 1 dies
#define HY27UF084G2M	0xADDC8095	//SLC, 512 MB, 1 dies
	/* HY27UT084G2M speed is slower, we have to decrease T1, T2 and T3 */
#define HY27UT084G2M	0xADDC8425	//MLC, 512 MB, 1 dies, BB check at last page, SLOW nand
#define HY27UT084G2A	0xADDC14A5	//MLC, 512 MB, 1 dies
#define H27U4G8T2B		0xADDC14A5	//MLC, 512 MB, 1 dies
#define HY27UT088G2A	0xADD314A5	//MLC, 1GB, 1 dies, BB check at last page
#define HY27UG088G5M	0xADDC8095	//SLC, 1GB, 2 dies
#define HY27UG088G5B	0xADDC1095	//SLC, 1GB, 2 dies
#define H27U8G8T2B	0xADD314B6	//MLC, 1GB, 1 dies, 4K page
#define H27UAG8T2A		0xADD59425	//MLC, 2GB, 1 dies, 4K page
#define H27U2G8F2C		0xADDA9095	//SLC, 256 MB, 1 dies, 2K page

/* Samsung Nand */
#define K9F1G08U0B	0xECF10095	//SLC, 128 MB, 1 dies
#define K9F2G08U0B	0xECDA1095	//SLC, 256 MB, 1 dies
#define K9G4G08U0A	0xECDC1425	//MLC, 512 MB, 1 dies, BB check at last page
#define K9G4G08U0B	0xECDC14A5	//MLC, 512 MB, 1 dies, BB check at last page
#define K9F4G08U0B	0xECDC1095	//SLC, 512 MB, 1 dies
#define K9G8G08U0A	0xECD314A5	//MLC, 1GB, 1 dies, BB check at last page
#define K9G8G08U0M	0xECD31425	//MLC, 1GB, 1 dies, BB check at last page
#define K9K8G08U0A	0xECD35195	//SLC, 1GB, 1 dies
#define K9F8G08U0M	0xECD301A6	//SLC, 1GB, 1 dies, 4K page

/* Toshiba */
#define TC58NVG0S3C	0x98F19095	//128 MB, 1 dies
#define TC58NVG0S3E	0x98D19015	//128 MB, 1 dies
#define TC58NVG1S3C	0x98DA9095	//256 MB, 1 dies
#define TC58NVG1S3E	0x98DA9015	//256 MB, 1 dies
#define TC58NVG2S3E	0x98DC9015	//512 MB, 1 dies

/*ccwei*/
/* Samsung Nand */
#define K9F1G08U0D	0xECF10015	//SLC, 128 MB, 1 dies
/*MXIC*/
//#define MX30LF1G08AM	0xC2F1801D	//SLC, 128MB, 1 dies
#define MX30LF1G08AA	0xC2F1801D	//SLC, 128MB, 1 dies
/*ST*/
#define NAND256W3A		0x20752075	//SLC, 32MB,    1dies, 512 Bytes page

#define MT29F1G08A		0x2CF18095	//Micron, SLC, 128MB, 1dies, 2K Bytes per page

/*end-ccwei*/


/* RTK Nand Chip ID list */
static device_type_t nand_device[] = 
{
	{"MT29F2G08AAD", MT29F2G08AAD, 0x10000000, 0x10000000, 2048, 64*2048, 64, 1, 0, 0x50, 0x00, 0x00, 0x00},
	{"MT29F2G08ABAE", MT29F2G08ABAE, 0x10000000, 0x10000000, 2048, 64*2048, 64, 1, 0, 0x03, 0x00, 0x00, 0x00},
	{"NAND01GW3B2B", NAND01GW3B2B, 0x8000000, 0x8000000, 2048, 64*2048, 64, 1, 0, 0xff, 0x00, 0x00, 0x00} ,
	{"NAND01GW3B2C", NAND01GW3B2C, 0x8000000, 0x8000000, 2048, 64*2048, 64, 1, 0, 0xff, 0x00, 0x00, 0x00} ,
	{"NAND02GW3B2D", NAND02GW3B2D, 0x10000000, 0x10000000, 2048, 64*2048, 64, 1, 0, 0x44, 0x00, 0x00, 0x00} ,
	{"NAND04GW3B2B", NAND04GW3B2B, 0x20000000, 0x20000000, 2048, 64*2048, 64, 1, 0, 0x20, 0x00, 0x00, 0x00} ,
	{"NAND04GW3B2D", NAND04GW3B2D, 0x20000000, 0x20000000, 2048, 64*2048, 64, 1, 0, 0x54, 0x00, 0x00, 0x00} ,
	{"NAND04GW3C2B", NAND04GW3C2B, 0x20000000, 0x20000000, 2048, 128*2048, 64, 1, 1, 0x24, 0x00, 0x00, 0x00} ,
	{"NAND08GW3C2B", NAND08GW3C2B, 0x40000000, 0x40000000, 2048, 128*2048, 64, 1, 1, 0x34, 0x00, 0x00, 0x00} ,
	{"HY27UF081G2A", HY27UF081G2A, 0x8000000, 0x8000000, 2048, 64*2048, 64, 1, 0, 0xff, 0x00, 0x00, 0x00} ,
	{"HY27UF082G2A", HY27UF082G2A, 0x10000000, 0x10000000, 2048, 64*2048, 64, 1, 0, 0x00, 0x00, 0x00, 0x00} ,
	{"HY27UF082G2B", HY27UF082G2B, 0x10000000, 0x10000000, 2048, 64*2048, 64, 1, 0, 0x44, 0x00, 0x00, 0x00} ,
	{"HY27UF084G2B", HY27UF084G2B, 0x20000000, 0x20000000, 2048, 64*2048, 64, 1, 0, 0x54, 0x00, 0x00, 0x00} ,
	{"HY27UT084G2A", HY27UT084G2A, 0x20000000, 0x20000000, 2048, 128*2048, 64, 1, 1, 0x24, 0x00, 0x00, 0x00} ,
	{"H27U4G8T2B",   H27U4G8T2B,   0x20000000, 0x20000000, 2048, 128*2048, 64, 1, 1, 0x24, 0x00, 0x00, 0x00} ,
	{"HY27UT088G2A", HY27UT088G2A, 0x40000000, 0x40000000, 2048, 128*2048, 64, 1, 1, 0x34, 0x00, 0x00, 0x00} ,
	{"HY27UF084G2M", HY27UF084G2M, 0x20000000, 0x20000000, 2048, 64*2048, 64, 1, 0, 0xff, 0x00, 0x00, 0x00} ,
	{"HY27UT084G2M", HY27UT084G2M, 0x20000000, 0x20000000, 2048, 128*2048, 64, 1, 1, 0xff, 0x04, 0x04, 0x04} ,
	{"HY27UG088G5M", HY27UG088G5M, 0x40000000, 0x20000000, 2048, 64*2048, 64, 2, 0, 0xff, 0x00, 0x00, 0x00} ,
	{"HY27UG088G5B", HY27UG088G5B, 0x40000000, 0x20000000, 2048, 64*2048, 64, 2, 0, 0x54, 0x00, 0x00, 0x00} ,
	{"H27U8G8T2B", H27U8G8T2B, 0x40000000, 0x40000000, 4096, 128*4096, 128, 1, 1, 0x34, 0x00, 0x00, 0x00} ,
  {"H27UAG8T2A", H27UAG8T2A, 0x80000000, 0x80000000, 4096, 128*4096, 224, 1, 1, 0x44, 0x00, 0x00, 0x00},
  {"H27U2G8F2C", H27U2G8F2C, 0x10000000, 0x10000000, 2048, 64*2048, 64, 1, 1, 0x44, 0x00, 0x00, 0x00}, 	
	{"K9F1G08U0B", K9F1G08U0B, 0x8000000, 0x8000000, 2048, 64*2048, 64, 1, 0, 0x40, 0x00, 0x00, 0x00} ,
	{"K9F2G08U0B", K9F2G08U0B, 0x10000000, 0x10000000, 2048, 64*2048, 64, 1, 0, 0x44, 0x00, 0x00, 0x00} ,
	{"K9G4G08U0A", K9G4G08U0A, 0x20000000, 0x20000000, 2048, 128*2048, 64, 1, 1, 0x54, 0x00, 0x00, 0x00} ,
	{"K9G4G08U0B", K9G4G08U0B, 0x20000000, 0x20000000, 2048, 128*2048, 64, 1, 1, 0x54, 0x00, 0x00, 0x00} ,
	{"K9F4G08U0B", K9F4G08U0B, 0x20000000, 0x20000000, 2048, 64*2048, 64, 1, 0, 0x54, 0x00, 0x00, 0x00} ,
	{"K9G8G08U0A", K9G8G08U0A, 0x40000000, 0x40000000, 2048, 128*2048, 64, 1, 1, 0x64, 0x00, 0x00, 0x00} ,
	{"K9G8G08U0M", K9G8G08U0M, 0x40000000, 0x40000000, 2048, 128*2048, 64, 1, 1, 0x64, 0x00, 0x00, 0x00} ,
	{"K9K8G08U0A", K9K8G08U0A, 0x40000000, 0x40000000, 2048, 64*2048, 64, 1, 0, 0x58, 0x00, 0x00, 0x00} ,
	{"K9F8G08U0M", K9F8G08U0M, 0x40000000, 0x40000000, 4096, 64*4096, 128, 1, 0, 0x64, 0x00, 0x00, 0x00} ,
	{"TC58NVG0S3C", TC58NVG0S3C, 0x8000000, 0x8000000, 2048, 64*2048, 64, 1, 0, 0xff, 0x00, 0x00, 0x00} ,
	{"TC58NVG0S3E", TC58NVG0S3E, 0x8000000, 0x8000000, 2048, 64*2048, 64, 1, 0, 0x76, 0x00, 0x00, 0x00} ,
	{"TC58NVG1S3C", TC58NVG1S3C, 0x10000000, 0x10000000, 2048, 64*2048, 64, 1, 0, 0xff, 0x00, 0x00, 0x00} ,
	{"TC58NVG1S3E", TC58NVG1S3E, 0x10000000, 0x10000000, 2048, 64*2048, 64, 1, 0, 0x76, 0x00, 0x00, 0x00} ,	
	{"TC58NVG2S3E", TC58NVG2S3E, 0x20000000, 0x20000000, 2048, 64*2048, 64, 1, 0, 0x76, 0x00, 0x00, 0x00} ,
/*ccwei*/
	{"K9F1G08U0D", K9F1G08U0D, 0x8000000, 0x8000000, 2048, 64*2048, 64, 1, 0, 0x40, 0x00, 0x00, 0x00} ,
//	{"MX30LF1G08AM", MX30LF1G08AM, 0x8000000, 0x8000000, 2048, 64*2048, 64, 1, 0, 0xff, 0x00, 0x00, 0x00},
	{"MX30LF1G08AA", MX30LF1G08AA, 0x8000000, 0x8000000, 2048, 64*2048, 64, 1, 0, 0xff, 0x00, 0x00, 0x00},
	{"NAND256W3A", NAND256W3A, 0x2000000, 0x2000000,  512, 32*512, 16, 1, 0, 0xff, 0x00, 0x00, 0x00},
	{"MT29F1G08A", MT29F1G08A, 0x8000000, 0x8000000, 2048, 64*2048, 64, 1, 0, 0x02, 0x00, 0x00, 0x00} ,  		
/*end-ccwei*/  		
	{NULL, }
};

/* NAND low-level MTD interface functions */
static int nand_read (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf, 
			u_char *oob_buf, struct nand_oobinfo *oobsel);

static int nand_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
#if 0 //czyao
static int nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *oob_buf);
#else
static int nand_read_oob_ext (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
#endif
static int nand_write (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char * buf, 
			const u_char *oob_buf, struct nand_oobinfo *oobsel);
#if 0 //czyao
static int nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
#else
static int nand_write_oob (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);
#endif
static int nand_erase (struct mtd_info *mtd, struct erase_info *instr);
static void nand_sync (struct mtd_info *mtd);
//static int nand_suspend (struct mtd_info *mtd);
//static void nand_resume (struct mtd_info *mtd);

/* YAFFS2 */
static int nand_block_isbad (struct mtd_info *mtd, loff_t ofs);
static int nand_block_markbad (struct mtd_info *mtd, loff_t ofs);

/* Global Variables */
int RBA;
static int page_size, oob_size, ppb, isLastPage;
//CMYu:, 20090415
//extern platform_info_t platform_info;   //czyao
char *mp_erase_nand;
int mp_erase_flag = 0;
//CMYu:, 20090512
char *mp_time_para;
int mp_time_para_value = 0;
char *nf_clock;
int nf_clock_value = 0;
//CMYu:, 20090720
char *mcp;
//CMYu:, 20091030
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
int read_has_check_bbt = 0;
unsigned int read_block = 0XFFFFFFFF;
unsigned int read_remap_block = 0XFFFFFFFF;
int write_has_check_bbt = 0;
unsigned int write_block = 0XFFFFFFFF;
unsigned int write_remap_block = 0XFFFFFFFF;
#endif
//===========================================================================
static void NF_CKSEL(char *PartNum, unsigned int value)
{
	rtk_writel(rtk_readl(0xb800000c)& (~0x00800000), 0xb800000c);
	rtk_writel( value, 0xb8000034 );
	rtk_writel(rtk_readl(0xb800000c)| (0x00800000), 0xb800000c);
	printk("[%s] %s is set to nf clock: 0x%x\n", __FUNCTION__, PartNum, value);
}

#if 0
static void dump_BBT(struct mtd_info *mtd)  //czyao moves it

static void reverse_to_Yaffs2Tags(__u8 *r_oobbuf) //czyao moves it

static int rtk_block_isbad(struct mtd_info *mtd, u16 chipnr, loff_t ofs) //czyao moves it

static int nand_block_isbad (struct mtd_info *mtd, loff_t ofs) //czyao moves it

static int nand_block_markbad (struct mtd_info *mtd, loff_t ofs) //czyao moves it

static int nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, 
			u_char *oob_buf) //czyao moves it

static int nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len, size_t * retlen, 
			const u_char * oob_buf) //czyao moves it

static int nand_read (struct mtd_info *mtd, loff_t from, size_t len, size_t * retlen, u_char * buf) //czyao moves it
                       
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel) //czyao moves it


static int nand_write (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf) //czyao moves it

static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, 
			const u_char * buf, const u_char *oob_buf, struct nand_oobinfo *oobsel) //czyao moves it


static int nand_erase (struct mtd_info *mtd, struct erase_info *instr) //czyao moves it

int nand_erase_nand (struct mtd_info *mtd, struct erase_info *instr, int allowbbt) //czyao moves it

static void nand_sync (struct mtd_info *mtd) //czyao moves it

static int nand_suspend (struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	this->suspend(mtd);
	return 0;
}


static void nand_resume (struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	this->resume(mtd);
}


static void nand_select_chip(struct mtd_info *mtd, int chip) //czyao moves it

static int scan_last_die_BB(struct mtd_info *mtd) //czyao moves it

static int scan_other_die_BB(struct mtd_info *mtd) //czyao moves it 

static int rtk_create_bbt(struct mtd_info *mtd, int page) //czyao moves it

int rtk_update_bbt (struct mtd_info *mtd, __u8 *data_buf, __u8 *oob_buf, BB_t *bbt) //czyao moves it

static int rtk_nand_scan_bbt(struct mtd_info *mtd)  //czyao moves it

#endif
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
static int check_BBT(struct mtd_info *mtd, unsigned int blk)
{	
	int i;	
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	printk("[%s] blk %d\n", __FUNCTION__, blk);

	for ( i=0; i<RBA; i++)	
	{		
	    if ( this->bbt[i].bad_block == blk )
        {			
            printk("blk 0x%x already exist\n",blk);			
		    return -1;			
	    }	
	}	
	return 0;
}

static void dump_BBT(struct mtd_info *mtd)
{
	printk("[%s] Nand BBT Content\n", __FUNCTION__);
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	int i;
	int BBs=0;
	for ( i=0; i<RBA; i++){
		if ( i==0 && this->bbt[i].BB_die == BB_DIE_INIT && this->bbt[i].bad_block == BB_INIT ){
			printk("Congratulation!! No BBs in this Nand.\n");
			break;
		}
		if ( this->bbt[i].bad_block != BB_INIT ){
			printk("[%d] (%d, %u, %d, %u)\n", i, this->bbt[i].BB_die, this->bbt[i].bad_block, 
				this->bbt[i].RB_die, this->bbt[i].remap_block);
			BBs++;
		}
#if 0
		else {
			printk("[%d] (%d, %u, %d, %u)\n", i, this->bbt[i].BB_die, this->bbt[i].bad_block, 
				this->bbt[i].RB_die, this->bbt[i].remap_block);
		}
#endif
	}
	this->BBs = BBs;
}
#endif
//ccwei: 120203
#if 0
static void reverse_to_Yaffs2Tags(__u8 *r_oobbuf)
{
	int k;
	for ( k=0; k<16; k++ ){
		r_oobbuf[k]  = r_oobbuf[1+k];
	}
}
#endif
static int rtk_block_isbad(struct mtd_info *mtd, u16 chipnr, loff_t ofs)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int page, block, page_offset;
	unsigned char block_status_p1;
#if Nand_Block_Isbad_Slow_Version
	unsigned char block_status_p2;
#endif
//ccwei 111116
	//unsigned char buf[oob_size] __attribute__((__aligned__(4)));
	unsigned char buf[oob_size] __attribute__((__aligned__(64)));
	unsigned char data_buf[page_size]; //fixme!

	
	page = ((int) ofs) >> this->page_shift;
	page_offset = page & (ppb-1);
	block = page/ppb;

	if ( isLastPage ){
		page = block*ppb + (ppb-1);	
//ccwei 111116
//		if ( this->read_oob (mtd, chipnr, page, oob_size, buf) ){
		if ( this->read_ecc_page (mtd, chipnr, page, data_buf, buf) ){

			printk ("%s: read_oob page=%d failed\n", __FUNCTION__, page);
			return 1;
		}
		 if(!NAND_ADDR_CYCLE){
			block_status_p1 = buf[0];
		 }else{
			block_status_p1 = buf[5];
		 }
#if Nand_Block_Isbad_Slow_Version
		page = block*ppb + (ppb-2);	
		if ( this->read_oob (mtd, chipnr, page, oob_size, buf) ){
			printk ("%s: read_oob page=%d failed\n", __FUNCTION__, page);
			return 1;
		}
		if(!NAND_ADDR_CYCLE){
			block_status_p2 = buf[0];
		 }else if{
			block_status_p2 = buf[5];
		 }
		//printk("[1]block_status_p1=0x%x, block_status_p2=0x%x\n", block_status_p1, block_status_p2);
#endif		
	}else{	
//ccwei 111116
//		if ( this->read_oob (mtd, chipnr, page, oob_size, buf) ){
		if ( this->read_ecc_page (mtd, chipnr, page, data_buf, buf) ){

			printk ("%s: read_oob page=%d failed\n", __FUNCTION__, page);
			return 1;
		}
		 if(!NAND_ADDR_CYCLE){
			block_status_p1 = buf[0];
		 }else{
			block_status_p1 = buf[5];
		 }
#if Nand_Block_Isbad_Slow_Version
		if ( this->read_oob (mtd, chipnr, page+1, oob_size, buf) ){
			printk ("%s: read_oob page+1=%d failed\n", __FUNCTION__, page+1);
			return 1;
		}
		 if(!NAND_ADDR_CYCLE){
			block_status_p2 = buf[0];
		 }else{
			block_status_p2 = buf[5];
		 }
		//printk("[2]block_status_p1=0x%x, block_status_p2=0x%x\n", block_status_p1, block_status_p2);
#endif
	}
#if Nand_Block_Isbad_Slow_Version
	if ( (block_status_p1 != 0xff) && (block_status_p2 != 0xff) ){
#else
//ccwei 120116
#ifdef CONFIG_RTK_NAND_BBT
    if ( block_status_p1 == BBT_TAG){
        printk ("INFO: Stored BBT in Die %d: block=%d , block_status_p1=0x%x\n", chipnr, block, block_status_p1);
    }else 
#endif    
    if ( block_status_p1 != 0xff){
#endif		
		printk ("WARNING: Die %d: block=%d is bad, block_status_p1=0x%x\n", chipnr, block, block_status_p1);
		return -1;
	}
	
	return 0;
}

static int nand_block_isbad (struct mtd_info *mtd, loff_t ofs)
{
	int chipnr=0, val=0;

//ccwei 111116
	//val = rtk_block_isbad(mtd,chipnr,ofs);
//printk("[%s] ofs %lld\n",__FUNCTION__, ofs);
#ifndef CONFIG_RTK_NAND_BBT
	if(ofs == 0)
		;//skip first block!
	else{
	    val = rtk_block_isbad(mtd,chipnr,ofs);
		if(val){
	        printk("[%s] this block is bad %lld\n",__FUNCTION__, ofs);
	    }
	}
#endif	
	return val;
}


static int nand_block_markbad (struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int page, block, page_offset;
	int i;
	int rc = 0;
	unsigned char buf[oob_size] __attribute__((__aligned__(4)));
	int chipnr, chipnr_remap;
#ifdef CONFIG_RTK_NAND_BBT
	{
		i = (ofs >> this->phys_erase_shift);//virtual block index
		ofs = (this->bbt_v2r[i].block_r << this->phys_erase_shift);//real block index, addr.
		printk("%s: blockv:%d blockr:%d ofs:%lld\n\r",__FUNCTION__,i,this->bbt_v2r[i].block_r, ofs);
	}
#endif	
	page = ((int) ofs) >> this->page_shift;
	this->active_chip = chipnr = chipnr_remap = (int)(ofs >> this->chip_shift);
	page_offset = page & (ppb-1);
	block = page/ppb;

	this->select_chip(mtd, chipnr);
//ccwei: 120228
#ifdef CONFIG_RTK_NAND_BBT
	for ( i=0; i<RBA; i++){
		if ( this->bbt[i].bad_block != BB_INIT ){
			if ( chipnr == this->bbt[i].BB_die && block == this->bbt[i].bad_block ){
				block = this->bbt[i].remap_block;
				if ( this->bbt[i].BB_die != this->bbt[i].RB_die ){
					this->active_chip = chipnr_remap = this->bbt[i].RB_die;
					this->select_chip(mtd, chipnr_remap);
				}					
			}
		}else
			break;
	}
#endif	
	page = block*ppb + page_offset;
    //ccwei: 120228
    if(!NAND_ADDR_CYCLE){
        buf[0] = 0x00;
    }else{
        buf[5] = 0x00;
    }				
	//buf[0]=0x00;
	rc = this->write_oob (mtd, this->active_chip, page, oob_size, buf);
	if (rc) {
			DEBUG (MTD_DEBUG_LEVEL0, "%s: write_oob failed\n", __FUNCTION__);
			return -1;
	}
	return 0;
}
static void memDump (void *start, u32 size, char * strHeader)
{
	int row, column, index, index2, max;
//	uint32 buffer[5];
	u8 *buf, *line, ascii[17];
	char empty = ' ';

	if(!start ||(size==0))
		return;
	line = (u8*)start;

	/*
	16 bytes per line
	*/
	if (strHeader)
		printk("%s", strHeader);
	column = size % 16;
	row = (size / 16) + 1;
	for (index = 0; index < row; index++, line += 16) 
	{
		buf = line;

		memset (ascii, 0, 17);

		max = (index == row - 1) ? column : 16;
		if ( max==0 ) break; /* If we need not dump this line, break it. */

		printk("\n%08x ", (u32) line);
		
		//Hex
		for (index2 = 0; index2 < max; index2++)
		{
			if (index2 == 8)
			printk("  ");
			printk("%02x ", (u8) buf[index2]);
			ascii[index2] = ((u8) buf[index2] < 32) ? empty : buf[index2];
		}

		if (max != 16)
		{
			if (max < 8)
				printk("  ");
			for (index2 = 16 - max; index2 > 0; index2--)
				printk("   ");
		}

		//ASCII
		printk("  %s", ascii);
	}
	printk("\n");
	return;
}

static int nand_read_oob (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{		
	//printk("[%s] mtd->oobblock =%u\n", __FUNCTION__, mtd->oobblock);	
	//printk("%s(%d):%d,%d\n",__FUNCTION__,__LINE__,ops->len,ops->ooblen);
	if(ops->len==ops->ooblen)// read oob 		
		return nand_read_oob_ext(mtd, from,ops);	
	else		
		return nand_read_ecc(mtd, from, ops->len, &ops->retlen,ops->datbuf, ops->oobbuf, NULL);
}           



#if 1
static int nand_read_oob_ext  (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int page, realpage;
	int oob_len = 0, thislen;
	int rc=0;
	int i, old_page, page_offset, block;
	int chipnr, chipnr_remap;
	//__u8 oob_area[64];
	__u8 *oob_area, oob_area0[64+16];

	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);
#ifdef CONFIG_RTK_NAND_BBT
	{
        unsigned int offset=0, aa=0;
		i = (from >> this->phys_erase_shift);//virtual block index
        aa = from & ~(this->block_size - 1); 
        offset = from - aa; //data offset
		from = (this->bbt_v2r[i].block_r << this->phys_erase_shift) + offset;	
//		printk("%s: blockv:%d blockr:%d from:%lld\n\r",__FUNCTION__,i,this->bbt_v2r[i].block_r, from);
	}
#endif	
	if ((from + ops->len) > mtd->size) {
		printk ("nand_read_oob: Attempt read beyond end of device\n");
		ops->retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, from)) {
		printk (KERN_NOTICE "nand_read_oob: Attempt to read not page aligned data. %llx\n", from);
		dump_stack();
		return -EINVAL;
	}

	/*printk("NAND :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n"
			,ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );*/

	realpage = (int)(from >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(from >> this->chip_shift);
	old_page = page = realpage & this->pagemask;
	page_offset = page & (ppb-1);
	block = page/ppb;
		
	this->select_chip(mtd, chipnr);
	
	if ( &(ops->retlen)) 
		ops->retlen = 0;
	thislen = oob_size;

	while(oob_len < ops->len) {

		if (thislen > (ops->len - oob_len)) 
			thislen = (ops->len - oob_len);
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
		for ( i=0; i<RBA; i++){
			if ( this->bbt[i].bad_block != BB_INIT ){
				if ( this->active_chip == this->bbt[i].BB_die && block == this->bbt[i].bad_block ){
					block = this->bbt[i].remap_block;
					if ( this->bbt[i].BB_die != this->bbt[i].RB_die ){
						this->active_chip = chipnr_remap = this->bbt[i].RB_die;
						this->select_chip(mtd, chipnr_remap);
					}					
				}
			}else
				break;
		}
#endif
		page = block*ppb + page_offset;

		//printk("[%s, line %d] page = %d, (ops->oobbuf)+oob_len = 0x%p\n",__FUNCTION__,__LINE__, page, ((ops->oobbuf)+oob_len));
		//rc = this->read_oob (mtd, this->active_chip, page, thislen, (ops->oobbuf)+oob_len);
		rc = this->read_oob (mtd, this->active_chip, page, thislen, oob_area /*ops->oobbuf */);

		//printk("%s(%d):from=%llx,pg=%d,fn=%p\n",__FUNCTION__,__LINE__,from,page,this->read_oob);		
		if(rc<0){
			if (rc == -1){
				printk ("%s: read_oob: Un-correctable HW ECC\n", __FUNCTION__);
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
				if(check_BBT(mtd,page/ppb)==0)
				{

				    for( i=0; i<RBA; i++){
					    if ( this->bbt[i].bad_block == BB_INIT && this->bbt[i].remap_block != RB_INIT){
						    if ( chipnr != chipnr_remap)	//remap block is bad
							    this->bbt[i].BB_die = chipnr_remap;
						    else
							    this->bbt[i].BB_die = chipnr;
						    this->bbt[i].bad_block = page/ppb;
						    break;
					    }
				    }

				    dump_BBT(mtd);
				
				    //if ( rtk_update_bbt (mtd, this->g_databuf, this->g_oobbuf, this->bbt) ){
				    if ( rtk_update_bbt (mtd,this->bbt) ){
					    printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
					    return -1;
				    }
				}
#endif				
                //ccwei 120116
                //this->g_oobbuf[0] = 0x00;
                if(!NAND_ADDR_CYCLE){
                    this->g_oobbuf[0] = 0x00;
                }else{
                    this->g_oobbuf[5] = 0x00;
                }
                //ccwei
				if ( isLastPage ){
					this->erase_block (mtd, this->active_chip, block*ppb);					
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-1, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-2, oob_size, this->g_oobbuf);
				}else{
					this->erase_block (mtd, this->active_chip, block*ppb);				
					this->write_oob(mtd, this->active_chip, block*ppb, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+1, oob_size, this->g_oobbuf);
				}
				printk("rtk_read_oob: Un-correctable HW ECC Error at page=%d\n", page);
				rc = 0;
#ifndef CONFIG_RTK_NAND_BBT
			    return -1;
#endif			    
			}else{
				printk ("%s: rtk_read_oob: semphore failed\n", __FUNCTION__);
				return -1;
			}
		}

		
		
		#if 1
		if (ops->oobbuf) {
			int oob_shift;
			for(oob_shift=0; oob_shift<4; oob_shift++){
				memcpy(ops->oobbuf+(oob_shift*6), oob_area+(oob_shift*16), 6);
			}			
			/*if (*((u32 *)oob_area) !=0xFFFFFFFF) {
				printk("READOOB: 0x%llx, oobarea=%p page=%d Treg=%p\n", from, oob_area, this->active_chip, page, *((volatile u32*)NADTSAR));
				memDump(oob_area, 64, "OOB_orig");
				memDump(ops->oobbuf, ops->ooblen, "OOB_return");
			}*/
			
		}
		#endif
		
		oob_len += thislen;

		old_page++;
		page_offset = old_page & (ppb-1);
		if ( oob_len<(ops->len) && !(old_page & this->pagemask)) {
			old_page &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		block = old_page/ppb;

	}

	if ( &(ops->retlen) ){
		if ( oob_len == ops->len )
			ops->retlen = oob_len;
		else{
			printk("[%s] error: oob_len %d != len %d\n", __FUNCTION__, oob_len, ops->len);
			return -1;
		}	
	}

	return rc;
}

#else
static int nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, 
			u_char *oob_buf)
{
	struct nand_chip *this = mtd->priv;
	unsigned int page, realpage;
	int oob_len = 0, thislen;
	int rc=0;
	int i, old_page, page_offset, block;
	int chipnr, chipnr_remap;

	if ((from + len) > mtd->size) {
		printk ("nand_read_oob: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, from)) {
		printk (KERN_NOTICE "nand_read_oob: Attempt to read not page aligned data\n");
		return -EINVAL;
	}

	realpage = (int)(from >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(from >> this->chip_shift);
	old_page = page = realpage & this->pagemask;
	page_offset = page & (ppb-1);
	block = page/ppb;
		
	this->select_chip(mtd, chipnr);

	//printk("%s, line %d, page:%d, page_offset:%d, block:%d\n",__FILE__,__LINE__,page,page_offset,block);
	//printk("%s, line %d, oob_len=%d, len=%d\n",__FILE__,__LINE__,oob_len, len);
	
	if ( retlen ) 
		*retlen = 0;
	thislen = oob_size;

	while (oob_len < len) {
		//printk("%s, line %d, czyao haha\n",__FILE__,__LINE__);
		if (thislen > (len - oob_len)) 
			thislen = (len - oob_len);

		//printk("%s, line %d, czyao haha\n",__FILE__,__LINE__);
		for ( i=0; i<RBA; i++){
			if ( this->bbt[i].bad_block != BB_INIT ){
				if ( this->active_chip == this->bbt[i].BB_die && block == this->bbt[i].bad_block ){
					block = this->bbt[i].remap_block;
					if ( this->bbt[i].BB_die != this->bbt[i].RB_die ){
						this->active_chip = chipnr_remap = this->bbt[i].RB_die;
						this->select_chip(mtd, chipnr_remap);
					}					
				}
			}else
				break;
		}
		page = block*ppb + page_offset;
		//printk("[line %d] page:%d, block:%d, ppb:%d, page_offset:%d\n",
		//	__LINE__,page,block,ppb,page_offset);
		
		rc = this->read_oob (mtd, this->active_chip, page, thislen, &oob_buf[oob_len]);
		if (rc < 0) {
			if (rc == -1){
				printk ("%s: read_oob: Un-correctable HW ECC\n", __FUNCTION__);
				for( i=0; i<RBA; i++){
					if ( this->bbt[i].bad_block == BB_INIT && this->bbt[i].remap_block != RB_INIT){
						if ( chipnr != chipnr_remap)	//remap block is bad
							this->bbt[i].BB_die = chipnr_remap;
						else
							this->bbt[i].BB_die = chipnr;
						this->bbt[i].bad_block = page/ppb;
						break;
					}
				}

				dump_BBT(mtd);
				
				if ( rtk_update_bbt (mtd, this->g_databuf, this->g_oobbuf, this->bbt) ){
					printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
					return -1;
				}
				
				this->g_oobbuf[0] = 0x00;
				if ( isLastPage ){
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-1, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-2, oob_size, this->g_oobbuf);
				}else{
					this->write_oob(mtd, this->active_chip, block*ppb, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+1, oob_size, this->g_oobbuf);
				}
				printk("rtk_read_oob: Un-correctable HW ECC Error at page=%d\n", page);				
			}else{
				printk ("%s: rtk_read_oob: semphore failed\n", __FUNCTION__);
				return -1;
			}
		}

		oob_len += thislen;

		old_page++;
		page_offset = old_page & (ppb-1);
		if ( oob_len<len && !(old_page & this->pagemask)) {
			old_page &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		block = old_page/ppb;
	}

	if ( retlen ){
		if ( oob_len == len )
			*retlen = oob_len;
		else{
			printk("[%s] error: oob_len %d != len %d\n", __FUNCTION__, oob_len, len);
			return -1;
		}	
	}

	return rc;
}
#endif

#if 1
static int nand_write_oob (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	int rc=0;
/*
	if ((to + ops->len) > mtd->size) {
		printk ("%s: Attempt write beyond end of device\n", __FUNCTION__);
		ops->retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, to)) {
		printk (KERN_NOTICE "%s: Attempt to write not page aligned data\n",__FUNCTION__);
		return -EINVAL;
	}
*/	
#if 0
	printk ("%s: to %lld ops->len %d\n",__FUNCTION__, to, ops->len);
    if(ops->datbuf){
        int i=0;
		printk("data:\n");
		for(i=0;i<16;i++)
			printk("%x ", ops->datbuf[i]);
		printk("\n");
	}
    if(ops->oobbuf){
        int i=0;
		printk("oob:\n");		
		for(i=0;i<16;i++)
			printk("%x ", ops->oobbuf[i]);
		printk("\n");

	}
#endif	
	rc =  nand_write_ecc (mtd, to, ops->len, &ops->retlen,ops->datbuf, ops->oobbuf, NULL);
	return rc;

}
#if 0
static int nand_write_oob (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int page, realpage;
	int oob_len=0, thislen;
	int rc=0; 
	int i, old_page, page_offset, block;
	int chipnr, chipnr_remap, err_chipnr = 0, err_chipnr_remap = 1;
	int test=0;
	int oob_shift, oob_new=32;
	//__u8 oob_area[64];// = kmalloc( oob_new, GFP_KERNEL );	
	__u8 *oob_area, oob_area0[64+16];

	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);
#ifdef CONFIG_RTK_NAND_BBT
	{
        unsigned int offset=0, aa=0;
        aa = to & ~(this->block_size - 1); 
        offset = to - aa; //data offset	
//printk("%s: aa:%d to:%lld offset:%d\n\r",__FUNCTION__,aa,to,offset);
		i = (to >> this->phys_erase_shift);//virtual block index
		to = (this->bbt_v2r[i].block_r << this->phys_erase_shift) + offset;//real block index + addr offset.
//printk("%s: blockv:%d blockr:%d to:%lld\n\r",__FUNCTION__,i,this->bbt_v2r[i].block_r, to);		
	}
#endif

	
	if ((to + ops->len) > mtd->size) {
		printk ("nand_write_oob: Attempt write beyond end of device\n");
		ops->retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, to)) {
		printk (KERN_NOTICE "nand_write_oob: Attempt to write not page aligned data\n");
		return -EINVAL;
	}

	if(!oob_area){
		printk("Can't allocate memory\n");
		return -EINVAL;
	}

	/*printk("In NAND\n");
	printk("ops.mode = %d\n",ops->mode);
	printk("ops.ooblen = %d\n",ops->ooblen);
	printk("ops.len = %d\n",ops->len);
	printk("ops.ooboffs= %d\n",ops->ooboffs);
	printk("ops.datbuf = 0x%p\n",ops->datbuf);
	printk("ops.oobbuf = 0x%p\n",ops->oobbuf);*/

	realpage = (int)(to >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(to >> this->chip_shift);
	old_page = page = realpage & this->pagemask;
	page_offset = page & (ppb-1);
	block = page/ppb;
	
	this->select_chip(mtd, chipnr);
		
	if ( &(ops->retlen) )
		ops->retlen = 0;
	thislen = oob_size;
	//printk("%s(%d):\n",__FUNCTION__,__LINE__);
	while (oob_len < (ops->len)) {
		if (thislen> (ops->len - oob_len)) 
			thislen = (ops->len - oob_len);
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT		
		for ( i=0; i<RBA; i++){
			if ( this->bbt[i].bad_block != BB_INIT ){
				if ( this->active_chip == this->bbt[i].BB_die && block == this->bbt[i].bad_block ){
					block = this->bbt[i].remap_block;
					if ( this->bbt[i].BB_die != this->bbt[i].RB_die ){
						this->active_chip = chipnr_remap = this->bbt[i].RB_die;
						this->select_chip(mtd, chipnr_remap);
					}					
				}
			}else
				break;
		}
#endif		
		page = block*ppb + page_offset;
		//printk("%s(%d):%p\n",__FUNCTION__,__LINE__,ops->oobbuf);
		
		//------- shift the ops->oobbuf to oob_area for 4-byte alignment ------//
		#if 1
		//ops->ooblen += 4;
		if (ops->oobbuf) {
			//printk("write:to=%llx len=%d\n",to,ops->ooblen);
			//memDump(ops->oobbuf, ops->ooblen, "OOB");
			memset(oob_area, 0x0, sizeof(oob_area));
		
			for(oob_shift=0; oob_shift<4; oob_shift++){
				memcpy(oob_area+(oob_shift*16), ops->oobbuf+(oob_shift*6), 6);
			}
		}
		#endif
		/*printk("[%s, line %d] page = %d\n",__FUNCTION__,__LINE__,page);
		printk("----------%s----------\n",__FUNCTION__);
		for(test=1;test<=ops->ooblen;test++){
			printk("0x%02X ",*(oob_area+test-1));
			if( (test%8)==0 ) printk("\n");
		}
		printk("\n");*/
		//printk("%s(%d):\n",__FUNCTION__,__LINE__);
		//rc = this->write_oob (mtd, this->active_chip, page, thislen, (ops->oobbuf));
		rc = this->write_ecc_page(mtd, this->active_chip, page, ops->datbuf, oob_area,0 );
		//printk("%s(%d):\n",__FUNCTION__,__LINE__);
		if( rc < 0){
			if ( rc == -1){
				printk ("%s: write_ecc_page:  write failed\n", __FUNCTION__);
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
				int block_remap = 0x12345678;
				if(check_BBT(mtd,page/ppb)==0)
				{

				    for( i=0; i<RBA; i++){
					    if ( this->bbt[i].bad_block == BB_INIT && this->bbt[i].remap_block != RB_INIT){
						    if ( chipnr != chipnr_remap)
							    err_chipnr = chipnr_remap;
						    else
							    err_chipnr = chipnr;
						    this->bbt[i].BB_die = err_chipnr;
						    this->bbt[i].bad_block = page/ppb;
						    err_chipnr_remap = this->bbt[i].RB_die;
						    block_remap = this->bbt[i].remap_block;
						    break;
					    }
				    } 
			
				    if ( block_remap == 0x12345678 ){
					    printk("[%s] RBA do not have free remap block\n", __FUNCTION__);
					    return -1;
				    }
			
				    dump_BBT(mtd);
				
				    if ( rtk_update_bbt (mtd, this->g_databuf, this->g_oobbuf, this->bbt) ){
					    printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
					    return -1;
				    }
				
				    int backup_offset = page&(ppb-1);
				    this->select_chip(mtd, err_chipnr_remap);
				    this->erase_block (mtd, err_chipnr_remap, block_remap*ppb);
				    printk("[%s] Start to Backup old_page from %d to %d\n", __FUNCTION__, block*ppb, block*ppb+backup_offset-1);
				    for ( i=0; i<backup_offset; i++){
					    if ( err_chipnr != err_chipnr_remap ){
						    this->active_chip = err_chipnr;
						    this->select_chip(mtd, err_chipnr);
					    }
					    this->read_ecc_page(mtd, this->active_chip, block*ppb+i, this->g_databuf, this->g_oobbuf);
/*
					if ( this->g_oobbuf )
						reverse_to_Yaffs2Tags(this->g_oobbuf);
*/						
					    if ( err_chipnr != err_chipnr_remap ){
						    this->active_chip = err_chipnr_remap;
						    this->select_chip(mtd, err_chipnr_remap);
					    }
					    this->write_ecc_page(mtd, this->active_chip, block_remap*ppb+i, this->g_databuf, this->g_oobbuf, 0);
				    }
				    this->write_oob (mtd, this->active_chip, block_remap*ppb+backup_offset, oob_size, (ops->oobbuf)+oob_len);
				    printk("[%s] write failure page = %d to %d\n", __FUNCTION__, page, block_remap*ppb+backup_offset);
				
				    if ( err_chipnr != err_chipnr_remap ){
					    this->active_chip = err_chipnr;
					    this->select_chip(mtd, err_chipnr);
				    }
				}
#endif
                //ccwei 120116
                //this->g_oobbuf[0] = 0x00;
                if(!NAND_ADDR_CYCLE){
                    this->g_oobbuf[0] = 0x00;
                }else{
                    this->g_oobbuf[5] = 0x00;
                }
                //ccwei
   				block = page/ppb;
				if ( isLastPage ){
					this->erase_block(mtd, this->active_chip, block*ppb);
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-1, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-2, oob_size, this->g_oobbuf);
				}else{
					this->erase_block(mtd, this->active_chip, block*ppb);
					this->write_oob(mtd, this->active_chip, block*ppb, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+1, oob_size, this->g_oobbuf);
				}
				rc = 0;
#ifndef CONFIG_RTK_NAND_BBT				
			    return -1;
#endif
			}else{
				printk ("%s: write_ecc_page:  semphore failed\n", __FUNCTION__);
				return -1;
			}	
		}

		//oob_len += thislen;
		oob_len += page_size;

		old_page++;
		page_offset = old_page & (ppb-1);
		if ( oob_len<(ops->len) && !(old_page & this->pagemask)) {
			old_page &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		block = old_page/ppb;
	}
	//printk("%s(%d):\n",__FUNCTION__,__LINE__);
	if ( &(ops->retlen) ){
		if ( oob_len == (ops->len) )
			ops->retlen = oob_len;
		else{
			printk("[%s] error: oob_len %d != len %d\n", __FUNCTION__, oob_len, (ops->len));
			//kfree(oob_area);
			return -1;
		}	
	}
	
	//kfree(oob_area);
	return rc;
	
}
#endif
#else
static int nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len, size_t * retlen, 
			const u_char * oob_buf)
{
	struct nand_chip *this = mtd->priv;
	unsigned int page, realpage;
	int oob_len=0, thislen;
	int rc=0; 
	int i, old_page, page_offset, block;
	int chipnr, chipnr_remap, err_chipnr = 0, err_chipnr_remap = 1;
	
	if ((to + len) > mtd->size) {
		printk ("nand_write_oob: Attempt write beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, to)) {
		printk (KERN_NOTICE "nand_write_oob: Attempt to write not page aligned data\n");
		return -EINVAL;
	}

	realpage = (int)(to >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(to >> this->chip_shift);
	old_page = page = realpage & this->pagemask;
	page_offset = page & (ppb-1);
	block = page/ppb;
		
	this->select_chip(mtd, chipnr);
		
	if ( retlen )
		*retlen = 0;
	thislen = oob_size;
	
	while (oob_len < len) {
		if (thislen> (len - oob_len)) 
			thislen = (len - oob_len);
		for ( i=0; i<RBA; i++){
			if ( this->bbt[i].bad_block != BB_INIT ){
				if ( this->active_chip == this->bbt[i].BB_die && block == this->bbt[i].bad_block ){
					block = this->bbt[i].remap_block;
					if ( this->bbt[i].BB_die != this->bbt[i].RB_die ){
						this->active_chip = chipnr_remap = this->bbt[i].RB_die;
						this->select_chip(mtd, chipnr_remap);
					}					
				}
			}else
				break;
		}
		page = block*ppb + page_offset;
	
		rc = this->write_oob (mtd, this->active_chip, page, thislen, &oob_buf[oob_len]);
		if (rc < 0) {
			if ( rc == -1){
				printk ("%s: write_ecc_page:  write failed\n", __FUNCTION__);
				int block_remap = 0x12345678;
				for( i=0; i<RBA; i++){
					if ( this->bbt[i].bad_block == BB_INIT && this->bbt[i].remap_block != RB_INIT){
						if ( chipnr != chipnr_remap)
							err_chipnr = chipnr_remap;
						else
							err_chipnr = chipnr;
						this->bbt[i].BB_die = err_chipnr;
						this->bbt[i].bad_block = page/ppb;
						err_chipnr_remap = this->bbt[i].RB_die;
						block_remap = this->bbt[i].remap_block;
						break;
					}
				}
			
				if ( block_remap == 0x12345678 ){
					printk("[%s] RBA do not have free remap block\n", __FUNCTION__);
					return -1;
				}
			
				dump_BBT(mtd);
				
				if ( rtk_update_bbt (mtd, this->g_databuf, this->g_oobbuf, this->bbt) ){
					printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
					return -1;
				}
				
				int backup_offset = page&(ppb-1);
				this->select_chip(mtd, err_chipnr_remap);
				this->erase_block (mtd, err_chipnr_remap, block_remap*ppb);
				printk("[%s] Start to Backup old_page from %d to %d\n", __FUNCTION__, block*ppb, block*ppb+backup_offset-1);
				for ( i=0; i<backup_offset; i++){
					if ( err_chipnr != err_chipnr_remap ){
						this->active_chip = err_chipnr;
						this->select_chip(mtd, err_chipnr);
					}
					this->read_ecc_page(mtd, this->active_chip, block*ppb+i, this->g_databuf, this->g_oobbuf);
					if ( this->g_oobbuf )
						reverse_to_Yaffs2Tags(this->g_oobbuf);
					if ( err_chipnr != err_chipnr_remap ){
						this->active_chip = err_chipnr_remap;
						this->select_chip(mtd, err_chipnr_remap);
					}
					this->write_ecc_page(mtd, this->active_chip, block_remap*ppb+i, this->g_databuf, this->g_oobbuf, 0);
				}
				this->write_oob (mtd, this->active_chip, block_remap*ppb+backup_offset, oob_size, &oob_buf[oob_len]);
				printk("[%s] write failure page = %d to %d\n", __FUNCTION__, page, block_remap*ppb+backup_offset);
				
				if ( err_chipnr != err_chipnr_remap ){
					this->active_chip = err_chipnr;
					this->select_chip(mtd, err_chipnr);
				}

				this->g_oobbuf[0] = 0x00;
				if ( isLastPage ){
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-1, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-2, oob_size, this->g_oobbuf);
				}else{
					this->write_oob(mtd, this->active_chip, block*ppb, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+1, oob_size, this->g_oobbuf);
				}
			}else{
				printk ("%s: write_ecc_page:  semphore failed\n", __FUNCTION__);
				return -1;
			}	
		}
		
		oob_len += thislen;

		old_page++;
		page_offset = old_page & (ppb-1);
		if ( oob_len<len && !(old_page & this->pagemask)) {
			old_page &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		block = old_page/ppb;
	}

	if ( retlen ){
		if ( oob_len == len )
			*retlen = oob_len;
		else{
			printk("[%s] error: oob_len %d != len %d\n", __FUNCTION__, oob_len, len);
			return -1;
		}	
	}
	
	return rc;
}
#endif

static int nand_read_ecc_512 (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel);

static int nand_read (struct mtd_info *mtd, loff_t from, size_t len, size_t * retlen, u_char * buf)
{
	if (len==512 && !(NAND_ADDR_CYCLE))
		return nand_read_ecc_512 (mtd, from, len, retlen, buf, NULL, NULL);
	else
		return nand_read_ecc(mtd, from, len, retlen, buf, NULL, NULL);
}                          


static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel)
{
	struct nand_chip *this = mtd->priv;
	unsigned int page, realpage;
	int data_len, oob_len;
	int rc;
	int i, old_page, page_offset, block;
	int chipnr, chipnr_remap;
	//__u8 oob_area[64];
	__u8 *oob_area, oob_area0[64+16];

	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);
	//printk("buf address = 0x%p, len = %d  ,mtd->size=%d ,in line %d\n"
	//		, buf, len,mtd->size,__LINE__);
	//printk("%s(%d): len %d\n",__FUNCTION__,__LINE__, len);
#ifdef CONFIG_RTK_NAND_BBT
	{
        unsigned int offset=0, aa=0;
		i = (from >> this->phys_erase_shift);//virtual block index
        aa = from & ~(this->block_size - 1); 
        offset = from - aa; //data offset
		from = (this->bbt_v2r[i].block_r << this->phys_erase_shift) + offset;//real block index + addr offset.
		//printk("%s: blockv:%d blockr:%d from:%lld\n\r",__FUNCTION__,i,this->bbt_v2r[i].block_r, from);
	}
#endif
	if ((from + len) > mtd->size) {
		printk ("nand_read_ecc: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}
	
	if (NOTALIGNED (mtd, from) || NOTALIGNED(mtd, len)) {		
		printk("[%s, line %d] from=%llxh, mtd->writesize=%d, len=%d\n"	,__FUNCTION__,__LINE__,from, mtd->writesize,len);
		printk (KERN_NOTICE "nand_read_ecc: Attempt to read not page aligned data\n");
		dump_stack();
		return -EINVAL;
	}

	realpage = (int)(from >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(from >> this->chip_shift);
	old_page = page = realpage & this->pagemask;
	page_offset = page & (ppb-1);
	block = page/ppb;
	//printk("%s(%d): block %d page %d realpage %d\n",__FUNCTION__,__LINE__, block, page, realpage);
	
	//CMYu, 20091030
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT	
	if ( this->numchips == 1 && block != read_block ){
		read_block = block;
		read_remap_block = 0xFFFFFFFF;
		read_has_check_bbt = 0;
	}
#endif
	#if 0 // andrew
	if ( oobsel && oobsel->useecc==1 ){
		mtd->ecctype = MTD_ECC_RTK_HW;
	}else
		mtd->ecctype = MTD_ECC_NONE;
	#endif 
		
	this->select_chip(mtd, chipnr);
	
	if ( retlen )
		*retlen = 0;
	
	data_len = oob_len = 0;

	while (data_len < len) {
		//CMYu, 20091030
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT	
		if ( this->numchips == 1){
			if ( (page>=block*ppb) && (page<(block+1)*ppb) && read_has_check_bbt==1 )
				goto SKIP_BBT_CHECK;
		}
				
		for ( i=0; i<RBA; i++){
			if ( this->bbt[i].bad_block != BB_INIT ){
				if ( this->active_chip == this->bbt[i].BB_die && block == this->bbt[i].bad_block ){
					read_remap_block = block = this->bbt[i].remap_block;
					if ( this->bbt[i].BB_die != this->bbt[i].RB_die ){
						this->active_chip = chipnr_remap = this->bbt[i].RB_die;
						this->select_chip(mtd, chipnr_remap);
					}
				}
			}else
				break;
		}
		read_has_check_bbt = 1;
		
SKIP_BBT_CHECK:
		if ( this->numchips == 1 && read_has_check_bbt==1 ){
			if ( read_remap_block == 0xFFFFFFFF )
				page = block*ppb + page_offset;
			else	
				page = read_remap_block*ppb + page_offset;
		}else
			page = block*ppb + page_offset;  
#else			
        //ccwei: 120203
		page = block*ppb + page_offset;  
#endif
		//printk ("%s: block %d at page =%d\n", __FUNCTION__,block, page);

		//printk("%s(%d):from=%llx,pg=%d,fn=%p\n",__FUNCTION__,__LINE__,from,page,this->read_ecc_page);
		rc = this->read_ecc_page (mtd, this->active_chip, page, &buf[data_len], oob_area/*&oob_buf[oob_len]*/);
		
		//printk("data_len=%d, oob_len=%d, buf address = 0x%p, oob address = 0x%p  ,in line %d\n"
		//	, data_len, oob_len, buf, oob_buf,__LINE__);
		if (rc < 0) {
			if (rc == -1){
				printk ("%s: Un-correctable HW ECC data_len %d\n", __FUNCTION__, data_len);
				//update BBT
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT	
				if(check_BBT(mtd,page/ppb)==0)
				{
				    for( i=0; i<RBA; i++){
					    if ( this->bbt[i].bad_block == BB_INIT && this->bbt[i].remap_block != RB_INIT){
						    if ( chipnr != chipnr_remap)	//remap block is bad
							    this->bbt[i].BB_die = chipnr_remap;
						    else
							    this->bbt[i].BB_die = chipnr;
						    this->bbt[i].bad_block = page/ppb;
						    break;
					    }
				    }
				
				    dump_BBT(mtd);
				
				    //if ( rtk_update_bbt (mtd, this->g_databuf, this->g_oobbuf, this->bbt) ){
				    if ( rtk_update_bbt (mtd, this->bbt) ){
					    printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
					    return -1;
				    }
				}
#endif				
				//this->g_oobbuf[0] = 0x00;
                //ccwei 120116
                if(!NAND_ADDR_CYCLE){
                    this->g_oobbuf[0] = 0x00;
                }else{
                    this->g_oobbuf[5] = 0x00;
                }
				block = page/ppb;
                //ccwei	
				if ( isLastPage){
					this->erase_block(mtd, this->active_chip, block*ppb);
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-1, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-2, oob_size, this->g_oobbuf);
				}else{
					this->erase_block(mtd, this->active_chip, block*ppb);
					this->write_oob(mtd, this->active_chip, block*ppb, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+1, oob_size, this->g_oobbuf);
				}
				printk("rtk_read_ecc_page: Un-correctable HW ECC Error at page=%d\n", page);
                rc = 0;
#ifndef CONFIG_RTK_NAND_BBT	
			    return -1;
#endif
			}else{
				printk ("%s: read_ecc_page:  semphore failed\n", __FUNCTION__);
				return -1;
			}
		}

		if (oob_buf) {
			int oob_shift;

			//memDump(oob_area,64,"nand_read_ecc OOB");
			for(oob_shift=0; oob_shift<4; oob_shift++){
				memcpy(oob_buf+(oob_shift*6), oob_area+(oob_shift*16), 6);			
			}			
			/*if (*((u32 *)oob_area) !=0xFFFFFFFF) {
				printk("READECC: 0x%llx, oobarea=%p chip=%d,page=%d\n", from, oob_area, this->active_chip, page);
				memDump(oob_area,64,"ECC_orig");			
				memDump(oob_buf,32,"ECC_return");			
			}*/
							
		}

		if(buf)//add by alexchang 0524-2010
		data_len += page_size;

		if(oob_buf)//add by alexchang 0524-2010
		oob_len += oob_size;
		
		old_page++;
		page_offset = old_page & (ppb-1);
		if ( data_len<len && !(old_page & this->pagemask)) {
			old_page &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		block = old_page/ppb;
	}

	if ( retlen ){
		if ( data_len == len )
			*retlen = data_len;
		else{
				printk("[%s] error: data_len %d != len %d\n", __FUNCTION__, data_len, len);
				return -1;
		}	
	}
	return rc;
}

/* goes around 2k limitation , andrew */

static int nand_read_ecc_512 (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel)
{
	static u64 nand_offset;
	static u8 nand_buffer[2048];
	static DECLARE_MUTEX(nandlock);
	
	size_t my_retlen;
	#define ADDR_MASK (2048 - 1)
	#define NANDSEG(x) (x & ~ADDR_MASK)
	down_interruptible(&nandlock);
	if (NANDSEG(from) != NANDSEG(nand_offset)) {
		int ret;
		ret = nand_read_ecc(mtd, NANDSEG(from), 2048, &my_retlen, nand_buffer, NULL, NULL);
		if (ret)
			return ret;
		nand_offset = NANDSEG(from);
	}

	memcpy(buf, &nand_buffer[ from - nand_offset ], len);
	up(&nandlock);
	*retlen = len;
	return 0;
	
}


static int nand_write (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	
	return (nand_write_ecc (mtd, to, len, retlen, buf, NULL, NULL));
}


static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, 
			const u_char * buf, const u_char *oob_buf, struct nand_oobinfo *oobsel)
{
	struct nand_chip *this = mtd->priv;
	unsigned int page, realpage;
	int data_len, oob_len;
	int rc;
	int i, old_page, page_offset, block;
	int chipnr, chipnr_remap, err_chipnr = 0, err_chipnr_remap = 1;
	//__u8 oob_area[64];
	__u8 *oob_area, oob_area0[64+16];

	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);
#ifdef CONFIG_RTK_NAND_BBT
	{
        unsigned int offset=0, aa=0;
        aa = to & ~(this->block_size - 1); 
        offset = to - aa; //data offset	
//printk("%s: aa:%d to:%lld offset:%d\n\r",__FUNCTION__,aa,to,offset);
		i = (to >> this->phys_erase_shift);//virtual block index
		to = (this->bbt_v2r[i].block_r << this->phys_erase_shift) + offset;//real block index + addr offset.
//printk("%s: blockv:%d blockr:%d to:%lld\n\r",__FUNCTION__,i,this->bbt_v2r[i].block_r, to);
	}
#endif

	if ((to + len) > mtd->size) {
		printk ("nand_write_ecc: Attempt write beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, to) || NOTALIGNED(mtd, len)) {
		printk("[%s, line %d] to=%llxh, mtd->writesize=%d, len=%d\n"	,__FUNCTION__,__LINE__,to, mtd->writesize,len);
		printk (KERN_NOTICE "nand_write_ecc: Attempt to write not page aligned data\n");
		dump_stack();
		return -EINVAL;
	}

	realpage = (int)(to >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(to >> this->chip_shift);
	old_page = page = realpage & this->pagemask;
	page_offset = page & (ppb-1);
	block = page/ppb;

	//CMYu, 20091030
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
	if ( this->numchips == 1 && block != write_block ){
		write_block = block;
		write_remap_block = 0xFFFFFFFF;
		write_has_check_bbt = 0;
	}
#endif
	this->select_chip(mtd, chipnr);
	
	if ( retlen )
		*retlen = 0;
	
	data_len = oob_len = 0;

	while ( data_len < len) {
		//CMYu, 20091030
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
		if ( this->numchips == 1){
			if ( (page>=block*ppb) && (page<(block+1)*ppb) && write_has_check_bbt==1 )
				goto SKIP_BBT_CHECK;
		}
				
		for ( i=0; i<RBA; i++){
			if ( this->bbt[i].bad_block != BB_INIT ){
				if ( this->active_chip == this->bbt[i].BB_die && block == this->bbt[i].bad_block ){
					write_remap_block = block = this->bbt[i].remap_block;
					if ( this->bbt[i].BB_die != this->bbt[i].RB_die ){
						this->active_chip = chipnr_remap = this->bbt[i].RB_die;
						this->select_chip(mtd, chipnr_remap);
					}
				}
			}else
				break;
		}
		
		write_has_check_bbt = 1;
		
SKIP_BBT_CHECK:
		if ( this->numchips == 1 && write_has_check_bbt==1 ){
			if ( write_remap_block == 0xFFFFFFFF )
				page = block*ppb + page_offset;
			else	
				page = write_remap_block*ppb + page_offset;
		}else
			page = block*ppb + page_offset;
#else
            //ccwei: 120203
		page = block*ppb + page_offset;
#endif
//printk("nand_write_ecc: inin block %d page %d data_len %d\n",block,page, data_len); 
		//printk ("%s: block %d at page =%d\n", __FUNCTION__,block, page);

		if (oob_buf) {
			int oob_shift;
			for(oob_shift=0; oob_shift<4; oob_shift++){
				memcpy(oob_area+(oob_shift*16), oob_buf+(oob_shift*6), 6);
			}	
		}
		rc = this->write_ecc_page (mtd, this->active_chip, page, &buf[data_len], oob_buf ? oob_area : NULL/*&oob_buf[oob_len]*/, 0);
		if (rc < 0) {
			if ( rc == -1){
				printk ("%s: write_ecc_page:  write failed\n", __FUNCTION__);
#ifdef CONFIG_RTK_NAND_BBT
				int block_remap = 0x12345678;
				/* update BBT */
				if(check_BBT(mtd,page/ppb)==0)
				{				
				    for( i=0; i<RBA; i++){
					    if ( this->bbt[i].bad_block == BB_INIT && this->bbt[i].remap_block != RB_INIT){
						    if ( chipnr != chipnr_remap)	//remap block is bad
							    err_chipnr = chipnr_remap;
						    else
							    err_chipnr = chipnr;
						    this->bbt[i].BB_die = err_chipnr;
						    this->bbt[i].bad_block = page/ppb;
						    err_chipnr_remap = this->bbt[i].RB_die;
						    block_remap = this->bbt[i].remap_block;
						    break;
					    }
				    }
			
				    if ( block_remap == 0x12345678 ){
					    printk("[%s] RBA do not have free remap block\n", __FUNCTION__);
					    return -1;
				    }
			
				    dump_BBT(mtd);
				
				    //if ( rtk_update_bbt (mtd, this->g_databuf, this->g_oobbuf, this->bbt) ){
				    if ( rtk_update_bbt (mtd, this->bbt) ){
					    printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
					    return -1;
				    }
				}
				int backup_offset = page&(ppb-1);
				this->select_chip(mtd, err_chipnr_remap);
				this->erase_block (mtd, err_chipnr_remap, block_remap*ppb);
				printk("[%s] Start to Backup old_page from %d to %d\n", __FUNCTION__, block*ppb, block*ppb+backup_offset-1);
				for ( i=0; i<backup_offset; i++){
					if ( err_chipnr != err_chipnr_remap ){
						this->active_chip = err_chipnr;
						this->select_chip(mtd, err_chipnr);
					}
					this->read_ecc_page(mtd, this->active_chip, block*ppb+i, this->g_databuf, this->g_oobbuf);
/*
					if ( this->g_oobbuf )
						reverse_to_Yaffs2Tags(this->g_oobbuf); //czyao
*/						
					if ( err_chipnr != err_chipnr_remap ){
						this->active_chip = err_chipnr_remap;
						this->select_chip(mtd, err_chipnr_remap);
					}
					this->write_ecc_page(mtd, this->active_chip, block_remap*ppb+i, this->g_databuf, this->g_oobbuf, 0);
				}
				//Write the written failed page to new block
				this->write_ecc_page (mtd, this->active_chip, block_remap*ppb+backup_offset, &buf[data_len], &oob_buf[oob_len], 0);
				printk("[%s] write failure page = %d to %d\n", __FUNCTION__, page, block_remap*ppb+backup_offset);
			
				if ( err_chipnr != err_chipnr_remap ){
					this->active_chip = err_chipnr;
					this->select_chip(mtd, err_chipnr);
				}
#endif
                //ccwei 120116
                //this->g_oobbuf[0] = 0x00;
                if(!NAND_ADDR_CYCLE){
                    this->g_oobbuf[0] = 0x00;
                }else{
                    this->g_oobbuf[5] = 0x00;
                }
				block = page/ppb;				
                //ccwei
				if ( isLastPage ){
					this->erase_block (mtd, err_chipnr, block*ppb);					
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-1, oob_size, this->g_oobbuf);
					this->write_oob(mtd, this->active_chip, block*ppb+ppb-2, oob_size, this->g_oobbuf);
				}else{
					this->erase_block (mtd, err_chipnr, block*ppb);
					this->write_oob(mtd, this->active_chip, block*ppb, oob_size, this->g_oobbuf);	
					this->write_oob(mtd, this->active_chip, block*ppb+1, oob_size, this->g_oobbuf);
				}
                rc = 0;
#ifndef CONFIG_RTK_NAND_BBT
				return -1;
#endif
			}else{
				printk ("%s: write_ecc_page:  rc=%d\n", __FUNCTION__, rc);
				return -1;
			}	
		}
		
		if(buf)//add by alexchang 0524-2010
		data_len += page_size;
		if(oob_buf) //add by alexchang 0524-2010
		oob_len += oob_size;
		
		old_page++;
		page_offset = old_page & (ppb-1);
		if ( data_len<len && !(old_page & this->pagemask)) {
			old_page &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		block = old_page/ppb;
	}

	if ( retlen ){
		
		if ( data_len == len )
			*retlen = data_len;
		else{
			printk("[%s] error: data_len %d != len %d\n", __FUNCTION__, data_len, len);
			return -1;
		}	
	}
	return rc;
}

static void nand_sync (struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	this->state = FL_READY;
}


static int nand_erase (struct mtd_info *mtd, struct erase_info *instr)
{
	
	return nand_erase_nand (mtd, instr, 0);
}


int nand_erase_nand (struct mtd_info *mtd, struct erase_info *instr, int allowbbt)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	u_int32_t addr = instr->addr;
	u_int32_t len = instr->len;
	int page, chipnr;
	int i, old_page, block;
	int elen = 0;
	int rc = 0;
	int realpage, chipnr_remap;

#ifdef CONFIG_RTK_NAND_BBT
	{
		i = (addr >> this->phys_erase_shift);//virtual block index
		addr = (this->bbt_v2r[i].block_r << this->phys_erase_shift);//real block index, addr.
		printk("%s: blockv:%d blockr:%d addr:%d\n\r",__FUNCTION__,i,this->bbt_v2r[i].block_r, addr);
	}
#endif
	

	check_end (mtd, addr, len);
	check_block_align (mtd, addr);

	instr->fail_addr = 0xffffffff;

	realpage = ((int) addr) >> this->page_shift;
	this->active_chip = chipnr = chipnr_remap = ((int) addr) >> this->chip_shift;
	old_page = page = realpage & this->pagemask;
	block = page/ppb;

	//printk("[%s] block: %d, page: %d\n",__FUNCTION__,block,page);
	this->select_chip(mtd, chipnr);

	instr->state = MTD_ERASING;
	while (elen < len) {
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
		for ( i=0; i<RBA; i++){
			if ( this->bbt[i].bad_block != BB_INIT ){
				if ( this->active_chip == this->bbt[i].BB_die && block == this->bbt[i].bad_block ){
					block = this->bbt[i].remap_block;
					if ( this->bbt[i].BB_die != this->bbt[i].RB_die ){
						this->active_chip = chipnr_remap = this->bbt[i].RB_die;
						this->select_chip(mtd, chipnr_remap);
					}
				}
			}else
				break;
		}
#endif		
		page = block*ppb;
printk ("%s: erase block %d at page =%d\n", __FUNCTION__,block, page);		
		rc = this->erase_block (mtd, this->active_chip, page);
			
		if (rc) {
			printk ("%s: block erase failed at page address=0x%08x\n", __FUNCTION__, addr);
			instr->fail_addr = (page << this->page_shift);	
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT		
			int block_remap = 0x12345678;
			if(check_BBT(mtd,page/ppb)==0)
			{	

			    for( i=0; i<RBA; i++){
				    if ( this->bbt[i].bad_block == BB_INIT && this->bbt[i].remap_block != RB_INIT){
						if ( chipnr != chipnr_remap)
							this->bbt[i].BB_die = chipnr_remap;
						else
							this->bbt[i].BB_die = chipnr;
					    this->bbt[i].bad_block = page/ppb;
					    block_remap = this->bbt[i].remap_block;
					    break;
				    }
			    }

			    if ( block_remap == 0x12345678 ){
				    printk("[%s] RBA do not have free remap block\n", __FUNCTION__);
				    return -1;
			    }

			    dump_BBT(mtd);

			    //if ( rtk_update_bbt (mtd, this->g_databuf, this->g_oobbuf, this->bbt) ){
			    if ( rtk_update_bbt (mtd,this->bbt) ){
				    printk("[%s] rtk_update_bbt() fails\n", __FUNCTION__);
				    return -1;
			    }
			}
#endif			
            //ccwei 120116
            //this->g_oobbuf[0] = 0x00;
            if(!NAND_ADDR_CYCLE){
                this->g_oobbuf[0] = 0x00;
            }else{
                this->g_oobbuf[5] = 0x00;
            }
            //ccwei
			if ( isLastPage ){
				this->write_oob(mtd, chipnr, page+ppb-1, oob_size, this->g_oobbuf);
				this->write_oob(mtd, chipnr, page+ppb-2, oob_size, this->g_oobbuf);
			}else{
				this->write_oob(mtd, chipnr, page, oob_size, this->g_oobbuf);
				this->write_oob(mtd, chipnr, page+1, oob_size, this->g_oobbuf);
			}
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
			rc = this->erase_block(mtd, chipnr_remap, block_remap*ppb);
#else
		    instr->state = MTD_ERASE_FAILED;
#endif
		}
		
		if ( chipnr != chipnr_remap )
			this->select_chip(mtd, chipnr);
			
		elen += mtd->erasesize;

		old_page += ppb;
		
		if ( elen<len && !(old_page & this->pagemask)) {
			old_page &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}

		block = old_page/ppb;
	}
	//ccwei: 120228
	if(!rc)
	    instr->state = MTD_ERASE_DONE;

	return rc;
}


static void nand_select_chip(struct mtd_info *mtd, int chip)
{
	//nand_select_chip doesn't use in SD5
	switch(chip) {
		case -1:
			rtk_writel(0x0, NACMR);
			break;			
		case 0:
			rtk_writel(CECS0, NACMR);
			break;
		case 1:
			rtk_writel(CECS1, NACMR);
			break;
		default:
			rtk_writel(0x0, NACMR);  //SD5 only support chip1 & chip0
	}
}
//ccwei: 120203
#ifdef CONFIG_RTK_NAND_BBT
static int scan_last_die_BB(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	__u32 start_page;
	__u32 addr;
	int block_num = this->block_num;
	int block_size = 1 << this->phys_erase_shift;
	int table_index=0;
	int remap_block[RBA];
	int remap_count = 0;
	int i, j;
	int numchips = this->numchips;
	int chip_size = this->chipsize;
	int rc = 0;

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
/*
	if ( numchips>1 ){
		start_page = 0x00000000;
	}else{
        //ccwei 120116
        //start_page = 0x01000000;
        start_page = USER_SPACE_START;
	}		
*/
	//printk("[%s, line %d] block_num=%d, block_size=%d, numchips=%d, chip_size=%d\n"
	//	,__FUNCTION__,__LINE__,block_num,block_size,numchips,chip_size);
	
	this->active_chip = numchips-1;
	this->select_chip(mtd, numchips-1);

	
	__u8 *block_status = kmalloc( block_num, GFP_KERNEL );	
	if ( !block_status ){
		printk("%s: Error, no enough memory for block_status\n",__FUNCTION__);
		rc = -ENOMEM;
		goto EXIT;
	}
	memset ( (__u32 *)block_status, 0, block_num );

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	for( addr=0; addr<chip_size; addr+=block_size ){
		if ( rtk_block_isbad(mtd, numchips-1, addr) ){
			int bb = addr >> this->phys_erase_shift;
			block_status[bb] = 0xff;
		}
	}

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	for ( i=0; i<RBA; i++){
		if ( block_status[(block_num-1)-i] == 0x00){
			remap_block[remap_count] = (block_num-1)-i;
			remap_count++;
		}
	}

	//printk("[remap_count=%d]\n",remap_count);
	if (remap_count<RBA+1){
		for (j=remap_count+1; j<RBA+1; j++){
			//printk("[j=%d]\n",j);
			remap_block[j-1] = RB_INIT;
		}
	}
#if 0		
	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	for ( i=0; i<(block_num-RBA); i++){
		if (block_status[i] == 0xff){
			this->bbt[table_index].bad_block = i;
			this->bbt[table_index].BB_die = numchips-1;
			this->bbt[table_index].remap_block = remap_block[table_index];
			this->bbt[table_index].RB_die = numchips-1;
			table_index++;
		}
	}
#endif
	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	for( i=table_index; table_index<RBA; table_index++){
		this->bbt[table_index].bad_block = BB_INIT;
		this->bbt[table_index].BB_die = BB_DIE_INIT;
		this->bbt[table_index].remap_block = remap_block[table_index];
		this->bbt[table_index].RB_die = numchips-1;
	}

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	kfree(block_status);

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	dma_cache_wback((unsigned long) this->bbt,sizeof(BB_t)*RBA);   //czyao
	
EXIT:
	if (rc){
		if (block_status)
			kfree(block_status);	
	}
				
	return 0;
}

/*we don't support multi chips/die right now!!*/
static int scan_other_die_BB(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	__u32 start_page;
	__u32 addr;
	int block_size = 1 << this->phys_erase_shift;
	int j, k;
	int numchips = this->numchips;
	int chip_size = this->chipsize;

	//printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
	for( k=0; k<numchips-1; k++ ){
		this->active_chip = k;
		this->select_chip(mtd, k);
		if( k==0 ){
            start_page = 0x00000000;
		}else{
			start_page = 0x00000000;
		}
		
		for( addr=start_page; addr<chip_size; addr+=block_size ){
			if ( rtk_block_isbad(mtd, k, addr) ){
				for( j=0; j<RBA; j++){
					if ( this->bbt[j].bad_block == BB_INIT && this->bbt[j].remap_block != RB_INIT){
						this->bbt[j].bad_block = addr >> this->phys_erase_shift;
						this->bbt[j].BB_die = k;
						this->bbt[j].RB_die = numchips-1;
						break;
					}
				}
			}
		}
	}

	//printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
	//RTK_FLUSH_CACHE((unsigned long) this->bbt, sizeof(BB_t)*RBA);	
	dma_cache_wback((unsigned long) this->bbt,sizeof(BB_t)*RBA);   //czyao
	
	return 0;
}

static int rtk_create_bbt(struct mtd_info *mtd, int page)
{
	printk("[%s] nand driver creates B%d !!\n", __FUNCTION__, page ==0?0:1);
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	int rc = 0;
	u8 *temp_BBT = 0;
	u8 mem_page_num, page_counter=0;

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	if ( scan_last_die_BB(mtd) ){
		printk("[%s] scan_last_die_BB() error !!\n", __FUNCTION__);
		return -1;
	}

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	if ( this->numchips >1 ){	
		printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
		if ( scan_other_die_BB(mtd) ){
			printk("[%s] scan_last_die() error !!\n", __FUNCTION__);
			return -1;
		}
	}

	//printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
	mem_page_num = (sizeof(BB_t)*RBA + page_size-1 )/page_size;
	temp_BBT = kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !temp_BBT ){
		printk("%s: Error, no enough memory for temp_BBT\n",__FUNCTION__);
		return -ENOMEM;
	}
	//printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
	memset( temp_BBT, 0xff, mem_page_num*page_size);

	this->select_chip(mtd, 0);

  	//czyao	
  	//printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
  	if ( this->erase_block(mtd, 0, page) ){
		printk("[%s]erase block %d failure !!\n", __FUNCTION__, page/ppb);
		rc =  -1;
		goto EXIT;
	}

	 if(!NAND_ADDR_CYCLE){
		this->g_oobbuf[0] = BBT_TAG;;
	 }else{
		this->g_oobbuf[5] = BBT_TAG;;
	 }	

	//printk("[%s, line %d]\n",__FUNCTION__,__LINE__);
	memcpy( temp_BBT, this->bbt, sizeof(BB_t)*RBA );
	while( mem_page_num>0 ){
		if ( this->write_ecc_page(mtd, 0, page+page_counter, temp_BBT+page_counter*page_size, 
			this->g_oobbuf, 1) ){
				printk("[%s] write BBT B%d page %d failure!!\n", __FUNCTION__, 
					page ==0?0:1, page+page_counter);
				rc =  -1;
				goto EXIT;
		}
		page_counter++;
		mem_page_num--;		
	}
	
EXIT:
	if (temp_BBT)
		kfree(temp_BBT);
		
	return rc;		
}


//int rtk_update_bbt (struct mtd_info *mtd, __u8 *data_buf, __u8 *oob_buf, BB_t *bbt)
int rtk_update_bbt (struct mtd_info *mtd, BB_t *bbt)
{
	int rc = 0,i=0, error_count=0;
	struct nand_chip *this = mtd->priv;
	unsigned char active_chip = this->active_chip;
	unsigned int bbt_page;
	unsigned char mem_page_num, page_counter=0, mem_page_num_tmp=0;
	int numchips = this->numchips;

	u8 *temp_BBT = 0;

	bbt_page = ((BOOT_SIZE >> this->phys_erase_shift)-BACKUP_BBT)*ppb;
	mem_page_num = (sizeof(BB_t)*this->RBA + page_size-1 )/page_size;
	printk("[%s] mem_page_num %d bbt_page %d\n\r", __FUNCTION__, mem_page_num, bbt_page);

	temp_BBT = kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !(temp_BBT) ){
		printk("%s: Error, no enough memory for temp_BBT\n",__FUNCTION__);
		return -1;
	}

	memset(temp_BBT, 0xff, mem_page_num*page_size);
	memcpy(temp_BBT, bbt, sizeof(BB_t)*this->RBA );

	//ccwei 111116
	if(!NAND_ADDR_CYCLE)
	    this->g_oobbuf[0] = BBT_TAG;
    else
		this->g_oobbuf[5] = BBT_TAG;
	this->select_chip(mtd, numchips-1);	
#if 0		
	if ( sizeof(BB_t)*RBA <= page_size){
		memcpy( data_buf, bbt, sizeof(BB_t)*RBA );
	}else{
		temp_BBT = kmalloc( 2*page_size, GFP_KERNEL );
		if ( !(temp_BBT) ){
			printk("%s: Error, no enough memory for temp_BBT\n",__FUNCTION__);
			return -ENOMEM;
		}
		memset(temp_BBT, 0xff, 2*page_size);
		memcpy(temp_BBT, bbt, sizeof(BB_t)*RBA );
		memcpy(data_buf, temp_BBT, page_size);
	}
#endif
//new method
    for(i=0;i<BACKUP_BBT;i++){
		mem_page_num_tmp = mem_page_num;
		page_counter=0;
	    if(!NAND_ADDR_CYCLE)
	        this->g_oobbuf[0] = BBT_TAG;
        else
		    this->g_oobbuf[5] = BBT_TAG;
	    this->select_chip(mtd, numchips-1);			
		//if(rtk_erase_block(bbt_page+(ppb*i))){
		if ( this->erase_block(mtd, numchips-1, bbt_page+(ppb*i)) ){	
			printk("[%s]error: erase BBT%d page %d failure\n\r", __FUNCTION__,i, bbt_page+(ppb*i));
			/*erase fail: mean this block is bad, so do not write data!!!*/
			mem_page_num_tmp = 0; 
			error_count++;
		}
		while( mem_page_num_tmp>0 ){
			//if(rtk_write_ecc_page(bbt_page+(ppb*i)+page_counter,temp_BBT+page_counter*page_size, &NfSpareBuf, page_size))	{
		    if ( this->write_ecc_page(mtd, numchips-1, bbt_page+(ppb*i)+page_counter, temp_BBT+page_counter*page_size, 
			    this->g_oobbuf, 1) ){			
					printk("[%s] write BBT%d page %d failure!!\n\r", __FUNCTION__,i, bbt_page+(ppb*i)+page_counter);
					//rc =  -1;
					//goto EXIT;
					error_count++;
					break;
			}
		//printf("[%s, line %d] mem_page_num = %d page_counter %d\n\r",__FUNCTION__,__LINE__,mem_page_num, page_counter);
			page_counter++;
			mem_page_num_tmp--; 	
		}	 

	}
//end

#if 0 //test new method	
	//if ( this->erase_block(mtd, 0, 0) ){
	if ( this->erase_block(mtd, numchips-1, bbt_page) ){	
		printk("[%s]error: erase block %d page %d failure\n", __FUNCTION__,(bbt_page/ppb),bbt_page);
	}
#endif	
/*
	if ( this->write_ecc_page(mtd, 0, 0, data_buf, oob_buf, 1) ){
		printk("[%s]update BBT B0 page 0 failure\n", __FUNCTION__);
	}else{
		if ( sizeof(BB_t)*RBA > page_size){
			memset(data_buf, 0xff, page_size);
			memcpy( data_buf, temp_BBT+page_size, sizeof(BB_t)*RBA - page_size );
			if ( this->write_ecc_page(mtd, 0, 1, data_buf, oob_buf, 1) ){
				printk("[%s]update BBT B0 page 1 failure\n", __FUNCTION__);
			}
		}	
	}
*/
#if 0 //test new method	
	while( mem_page_num>0 ){
		if ( this->write_ecc_page(mtd, numchips-1, bbt_page+page_counter, temp_BBT+page_counter*page_size, 
			this->g_oobbuf, 1) ){
		//if(rtk_write_ecc_page(bbt_page+page_counter,temp_BBT+page_counter*page_size, &NfSpareBuf, page_size))	{
				printk("[%s] write BBT0 page %d failure!!\n\r", __FUNCTION__, bbt_page+page_counter);
				rc =  -1;
				//goto EXIT;
		}
	//printf("[%s, line %d] mem_page_num = %d page_counter %d\n\r",__FUNCTION__,__LINE__,mem_page_num, page_counter);
		page_counter++;
		mem_page_num--; 	
	}	

	//if ( this->erase_block(mtd, 1, bbt0_block_num) ){
    if ( this->erase_block(mtd, numchips-1, bbt_page+ppb) ){	
		printk("[%s]error: erase block:%d page:%d failure\n", __FUNCTION__, (bbt_page/ppb)+1, bbt_page+ppb);
        rc = -1;
		goto EXIT;
		//return -1;
	}
/*	
	if ( this->write_ecc_page(mtd, 0, bbt0_block_num, data_buf, oob_buf, 1) ){
		printk("[%s]update BBT B1 failure\n", __FUNCTION__);
		return -1;
	}else{
		if ( sizeof(BB_t)*RBA > page_size){
			memset(data_buf, 0xff, page_size);
			memcpy( data_buf, temp_BBT+page_size, sizeof(BB_t)*RBA - page_size );
			if ( this->write_ecc_page(mtd, 0, bbt0_block_num+1, data_buf, oob_buf, 1) ){
				printk("[%s]error: erase block 0 failure\n", __FUNCTION__);
				return -1;
			}
		}		
	}
*/
	while( mem_page_num>0 ){
		if ( this->write_ecc_page(mtd, numchips-1, bbt_page+ppb+page_counter, temp_BBT+page_counter*page_size, 
			this->g_oobbuf, 1) ){
		//if(rtk_write_ecc_page(bbt_page+page_counter,temp_BBT+page_counter*page_size, &NfSpareBuf, page_size))	{
				printk("[%s] write BBT1 page %d failure!!\n\r", __FUNCTION__, bbt_page+ppb+page_counter);
				rc =  -1;
				goto EXIT;
		}
	//printf("[%s, line %d] mem_page_num = %d page_counter %d\n\r",__FUNCTION__,__LINE__,mem_page_num, page_counter);
		page_counter++;
		mem_page_num--; 	
	}
#endif
	this->select_chip(mtd, active_chip);
EXIT:
	if (temp_BBT)
		kfree(temp_BBT);
    if(error_count >= BACKUP_BBT){
		rc = -1;
		printk("%d bbt table are all bad!(T______T)\n\r", BACKUP_BBT);
	}			
	return rc;
}


//ccwei 120316
//####################################################################
// Function : create_v2r_remapping
// Description : Create virtual block to real good block mapping table in specific block
// Input:
//		page: the page we want to put the V2R mapping table, it must be block alignment
//         block_v2r_num: from block 0 to the specified block number 
// Output:
//		BOOL: 0=>OK, -1=>FAIL 
//####################################################################
static int create_v2r_remapping(struct mtd_info *mtd, unsigned int page, unsigned int block_v2r_num)
{
	//unsigned int block_v2r_num=0;
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int offs=0, offs_real=0;
	unsigned char mem_page_num, page_counter=0;
	unsigned char *temp_BBT = 0;
    int rc=0;
	int numchips = this->numchips;
	unsigned int search_region=0, count=0;
	unsigned int block_size = (1 << this->phys_erase_shift);
	unsigned int page_size = (1 << this->page_shift);
    //offs = start_page;
	//offs_real = start_page;
	count = 0;
	search_region = (block_v2r_num << this->phys_erase_shift);
	//just create [bootloader+user+rootfs+rootfs2] region remapping
    while(offs < search_region){
		if ( rtk_block_isbad(mtd,numchips-1,offs_real) ){
			offs_real += this->erase_block;
		}else{
    		//this->bbt_v2r[count].block_v = (offs >> this->phys_erase_shift);
    		this->bbt_v2r[count].block_r = (offs_real >> this->phys_erase_shift);			
			offs+=block_size;
			offs_real += block_size;			
//printk("bbt_v2r[%d].block_v %d,  bbt_v2r[%d].block_r %d\n",count,bbt_v2r[count].block_v,count,bbt_v2r[count].block_r);
			count++;
		}
	}
	//printk("[%s, line %d] block_v2r_num %d\n\r",__FUNCTION__,__LINE__, block_v2r_num);

	mem_page_num = ((sizeof(BB_v2r)*block_v2r_num) + page_size-1 )/page_size;
	//printk("[%s, line %d] mem_page_num = %d\n\r",__FUNCTION__,__LINE__,mem_page_num);

	//temp_BBT = (unsigned char *) malloc( mem_page_num*page_size);
	temp_BBT = (unsigned char *)kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !temp_BBT ){
		printk("%s: Error, no enough memory for temp_BBT v2r\n\r",__FUNCTION__);
		rc = -1;
		goto EXIT_V2R;
	}
	memset( temp_BBT, 0xff, mem_page_num*page_size);

	//if ( rtk_erase_block(mtd,numchips-1,page)){
  	if ( this->erase_block(mtd, numchips-1, page) ){
		printk("[%s]erase block %d failure !!\n\r", __FUNCTION__, page/this->ppb);
		rc =  -1;
		goto EXIT_V2R;
	}
	if(!NAND_ADDR_CYCLE)
		this->g_oobbuf[0] = BBT_TAG;
	else
		this->g_oobbuf[5] = BBT_TAG;
	memcpy( temp_BBT, this->bbt_v2r, sizeof(BB_v2r)*block_v2r_num );
	//dump_mem((unsigned int)temp_BBT,512);
	while( mem_page_num>0 ){
		if ( this->write_ecc_page(mtd, 0, page+page_counter, temp_BBT+page_counter*page_size, 
			this->g_oobbuf, 1) ){
				printk("[%s] write BBT page %d failure!!\n\r", __FUNCTION__, page+page_counter);
				rc =  -1;
				goto EXIT_V2R;
		}
//printk("[%s, line %d] mem_page_num = %d page_counter %d\n\r",__FUNCTION__,__LINE__,mem_page_num, page_counter);
			page_counter++;
			mem_page_num--; 	
	}
	EXIT_V2R:
	if(temp_BBT)
		kfree(temp_BBT);
	return rc;

}


int rtk_scan_v2r_bbt(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int bbt_v2r_page;
    int rc=0, i=0, error_count=0;
	unsigned char isbbt=0;
	unsigned char mem_page_num=0, page_counter=0, mem_page_num_tmp=0;
	unsigned char *temp_BBT=NULL;
	unsigned int block_v2r_num=0;
	unsigned int block_size = (1 << this->phys_erase_shift);
	unsigned int page_size = (1 << this->page_shift);
	unsigned char load_bbt_error = 0, is_first_boot=1;
	int numchips = this->numchips;

    bbt_v2r_page = ((BOOT_SIZE/block_size)-(2*BACKUP_BBT))*this->ppb;

	block_v2r_num = ((BOOT_SIZE + USER_SPACE_SIZE + VIMG_SPACE_SIZE + VIMG_SPACE_SIZE) >> this->phys_erase_shift);
printk("[%s, line %d] block_v2r_num %d bbt_v2r_page %d\n\r",__FUNCTION__,__LINE__, block_v2r_num, bbt_v2r_page);

	//create virtual block to real good block remapping!!!
	dma_cache_wback((unsigned long)this->bbt_v2r,(sizeof(BB_v2r)*block_v2r_num));   //czyao

#if 0	
	this->bbt_v2r = (unsigned char *) malloc(sizeof(struct BBT_v2r)*(block_v2r_num));
	if(!this->bbt_v2r){
		printf("%s-%d: Error, no enough memory for bbt_v2r\n",__FUNCTION__,__LINE__);
		return FAIL;
	}
#endif

	mem_page_num = ((sizeof(BB_v2r)*block_v2r_num) + page_size-1 )/page_size;
	//printk("[%s, line %d] mem_page_num = %d\n\r",__FUNCTION__,__LINE__,mem_page_num);


	//temp_BBT =(unsigned char *)malloc( mem_page_num*page_size );
	temp_BBT = (unsigned char *)kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if(!temp_BBT){
		printk("%s: Error, no enough memory for temp_BBT_v2r\n",__FUNCTION__);
		return -1;
	}
//test NEW method!
	for(i=0;i<BACKUP_BBT;i++){
	    //rc = rtk_read_ecc_page(bbt_page+(i*ppb), &NfDataBuf, &NfSpareBuf,page_size);
		rc = this->read_ecc_page(mtd, numchips-1, bbt_v2r_page+(i*ppb), this->g_databuf, this->g_oobbuf);
	    if(!NAND_ADDR_CYCLE)
		    isbbt = this->g_oobbuf[0];
	    else
		    isbbt = this->g_oobbuf[5];
		if(!rc){
		    if(isbbt==BBT_TAG)//bbt has already created
				is_first_boot = 0;
		}
	}
	printk("%s: is_first_boot:%d\n\r",__FUNCTION__, is_first_boot);

	for(i=0;i<BACKUP_BBT;i++){
		mem_page_num_tmp = mem_page_num;
		page_counter=0;load_bbt_error=0;
		rc = rtk_block_isbad(mtd,numchips-1,(bbt_v2r_page+(i*ppb))*page_size);
		if(!rc){
			printk("load bbt v2r table:%d page:%d\n\r",i, (bbt_v2r_page+(i*ppb)));
		    //rc = rtk_read_ecc_page(bbt0_block_num+(i*ppb), &NfDataBuf, &NfSpareBuf,page_size);
			rc = this->read_ecc_page(mtd, numchips-1, bbt_v2r_page+(i*ppb), this->g_databuf, this->g_oobbuf);
		    if(!NAND_ADDR_CYCLE)
			    isbbt = this->g_oobbuf[0];
		    else
			    isbbt = this->g_oobbuf[5];
	        if(!rc){
			    if(isbbt == BBT_TAG){
			        printk("[%s] have created v2r bbt table:%d on block %d, just loads it !!\n\r", __FUNCTION__,i,(bbt_v2r_page/ppb)+i);
			        //memcpy( temp_BBT, &NfDataBuf, page_size );
					memcpy( temp_BBT, this->g_databuf, page_size );
			        page_counter++;
			        mem_page_num_tmp--;
			        while( mem_page_num_tmp>0 ){
				        //if( rtk_read_ecc_page((bbt0_block_num+(i*ppb)+page_counter), &NfDataBuf, &NfSpareBuf, page_size)){
				        if( this->read_ecc_page(mtd, numchips-1, bbt_v2r_page+(i*ppb)+page_counter, this->g_databuf, this->g_oobbuf) ){				        
					        printk("[%s] load v2r bbt table%d error!!\n\r", __FUNCTION__,i);
					        //free(temp_BBT);
					        //load_bbt1 = 1;
                            load_bbt_error=1;
					        //return -1;
					        //goto TRY_LOAD_BBT1;
			                error_count++;					        
					        break;
				        }
				        //memcpy( temp_BBT+page_counter*page_size, &NfDataBuf, page_size );
						memcpy( temp_BBT+page_counter*page_size, this->g_databuf, page_size );				        
				        page_counter++;
				        mem_page_num_tmp--;
			        }
					if(!load_bbt_error){
						memcpy( this->bbt_v2r, temp_BBT, sizeof(BB_v2r)*(block_v2r_num));
					    printk("check v2r bbt table:%d OK\n\r",i);
					    goto CHECK_BBT_OK;
					}
			    }else{
					if(is_first_boot){
						printk("Create v2r bbt table:%d is_first_boot:%d\n\r",i, is_first_boot);
						create_v2r_remapping(mtd, bbt_v2r_page+(i*ppb), block_v2r_num);
					}
			    }
		    }
		}else{
            printk("v2r bbt table:%d block:%d page:%d is bad\n\r",i,(bbt_v2r_page/ppb)+i,bbt_v2r_page+(i*ppb));
			error_count++;
		}
	}
CHECK_BBT_OK:			
#if 0	
	rc = this->read_ecc_page(mtd, 0, bbt_v2r_page, this->g_databuf, this->g_oobbuf);
	//rc = rtk_read_ecc_page(bbt_v2r_page, &NfDataBuf, &NfSpareBuf,page_size);
    if(!NAND_ADDR_CYCLE)
	    isbbt = this->g_oobbuf[0];
	else
		isbbt = this->g_oobbuf[5];

    //printk("[%s, line %d] isbbt_b0 = %d rc %d\n\r",__FUNCTION__,__LINE__,isbbt, rc);

	if(!rc){
		if(isbbt == BBT_TAG){
			printk("[%s] have created bbt_v2r B0 on block %d, just loads it !!\n\r", __FUNCTION__,bbt_v2r_page/this->ppb);
			memcpy( temp_BBT, this->g_databuf, page_size );
			page_counter++;
			mem_page_num--;
			while( mem_page_num>0 ){
//				if( rtk_read_ecc_page((bbt_v2r_page+page_counter), this->g_databuf, this->g_oobbuf, page_size)){
				if(this->read_ecc_page(mtd, 0, bbt_v2r_page+page_counter, this->g_databuf, this->g_oobbuf)){
					printk("[%s] load bbt_v2r B0 error!!\n\r", __FUNCTION__);
					kfree(temp_BBT);
					return -1;
				}
				memcpy( temp_BBT+page_counter*page_size, this->g_databuf, page_size );
				page_counter++;
				mem_page_num--;
			}
			memcpy( this->bbt_v2r, temp_BBT, sizeof(BB_v2r)*(block_v2r_num));
		}else{
			printk("[%s] read bbt_v2r B0 tags fails, try to load bbt_v2r B1\n\r", __FUNCTION__);
			//rc = rtk_read_ecc_page(bbt_v2r_page+ppb, this->g_databuf, this->g_oobbuf, page_size);
			rc = this->read_ecc_page(mtd, 0, bbt_v2r_page+this->ppb, this->g_databuf, this->g_oobbuf);
            if(!NAND_ADDR_CYCLE)
	            isbbt = this->g_oobbuf[0];
	        else
		        isbbt = this->g_oobbuf[5];	
			if ( !rc ){
				if ( isbbt == BBT_TAG ){
					printk("[%s] have created bbt_v2r B1 on block %d, just loads it !!\n", __FUNCTION__, (bbt_v2r_page/this->ppb)+1);
					memcpy( temp_BBT, this->g_databuf, page_size );
					page_counter++;
					mem_page_num--;

					while( mem_page_num>0 ){
						//if(rtk_read_ecc_page((bbt_v2r_page+ppb+page_counter), this->g_databuf, this->g_oobbuf, page_size)){
				        if(this->read_ecc_page(mtd, 0, bbt_v2r_page+this->ppb+page_counter, this->g_databuf, this->g_oobbuf)){						
							printk("[%s] load bbt_v2r B1 error!!\n\r", __FUNCTION__);
							kfree(temp_BBT);
							return -1;
						}
						memcpy( temp_BBT+page_counter*page_size, this->g_databuf, page_size );
						page_counter++;
						mem_page_num--;
					}
					memcpy( this->bbt_v2r, temp_BBT, sizeof(BB_v2r)*(block_v2r_num));
				}else{
					printk("[%s] read bbt_v2r B1 tags fails, nand driver will creat bbt_v2r B0 and B1\n\r", __FUNCTION__);
					//rtk_create_bbt(bbt_v2r_page);
					//rtk_create_bbt(bbt_v2r_page+ppb);
					create_v2r_remapping(mtd,bbt_v2r_page,block_v2r_num);//fix me! later
					create_v2r_remapping(mtd,bbt_v2r_page+this->ppb,block_v2r_num);//fix me! later
				}
		}else{
				printk("[%s] read bbt_v2r B1 with HW ECC fails, nand driver will creat BBT B0\n", __FUNCTION__);
				create_v2r_remapping(mtd,bbt_v2r_page, block_v2r_num);
			}
		}// if BBT_TAG
	}else{
		printk("[%s] read bbt_v2r B0 with HW ECC error, try to load BBT B1\n\r", __FUNCTION__);
		//rc = rtk_read_ecc_page(bbt_v2r_page+ppb, this->g_databuf, this->g_oobbuf, page_size);
        if(!NAND_ADDR_CYCLE)
	        isbbt = this->g_oobbuf[0];
	    else
		    isbbt = this->g_oobbuf[5];	
		if ( !rc ){
			if ( isbbt == BBT_TAG ){
				printk("[%s] have created bbt_v2r B1 on block %d, just loads it !!\n\r", __FUNCTION__,(bbt_v2r_page/this->ppb)+1);
				memcpy( temp_BBT, this->g_databuf, page_size );
				page_counter++;
				mem_page_num--;

				while( mem_page_num>0 ){
					//if(rtk_read_ecc_page((bbt_v2r_page+ppb+page_counter), this->g_databuf, this->g_oobbuf, page_size)){
				    if(this->read_ecc_page(mtd, 0, bbt_v2r_page+this->ppb+page_counter, this->g_databuf, this->g_oobbuf)){											
						printk("[%s] load bbt_v2r B1 error!!\n\r", __FUNCTION__);
						kfree(temp_BBT);
						return -1;
					}
					memcpy( temp_BBT+page_counter*page_size, this->g_databuf, page_size );
					page_counter++;
					mem_page_num--;
				}
				memcpy(this->bbt_v2r, temp_BBT, sizeof(BB_v2r)*(block_v2r_num));
			}else{
				printk("[%s] read bbt_v2r B1 tags fails, nand driver will creat BBT B1\n\r", __FUNCTION__);
				create_v2r_remapping(mtd,bbt_v2r_page+this->ppb,block_v2r_num);//fix me! later
			}
		}else{
			printk("[%s-%d:] read bbt_v2r B0 and B1 with HW ECC fails\n\r", __FUNCTION__,__LINE__);
			kfree(temp_BBT);
			return -1;
		}
	}
#endif	
	if (temp_BBT)
		kfree(temp_BBT);
    //printk("[%s, line %d] done rc %d\n",__FUNCTION__,__LINE__,rc);
    if(error_count >= BACKUP_BBT){
        rc = -1;
		printk("%d v2r table are all bad!(T______T)\n\r", BACKUP_BBT);
	}

    return rc;
}

//end


static int rtk_nand_scan_bbt(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	int rc = 0, i;
	__u8 isbbt;
	u8 *temp_BBT=0;
	u8 mem_page_num, page_counter=0, mem_page_num_tmp=0;
	u8 load_bbt_error=0,is_first_boot=1, error_count=0;
	int numchips = this->numchips;
	unsigned int bbt_page;
	
	//__u8 check0, check1, check2, check3;
	dma_cache_wback((unsigned long) this->bbt,sizeof(BB_t)*RBA);   //czyao

	bbt_page = ((BOOT_SIZE/this->block_size)-BACKUP_BBT)*ppb;

	mem_page_num = (sizeof(BB_t)*RBA + page_size-1 )/page_size;
	printk("[%s, line %d] mem_page_num=%d bbt_page %d\n",__FUNCTION__,__LINE__,mem_page_num, bbt_page);

	temp_BBT = kmalloc( mem_page_num*page_size, GFP_KERNEL );
	if ( !temp_BBT ){
		printk("%s: Error, no enough memory for temp_BBT\n",__FUNCTION__);
		return -ENOMEM;
	}

	
	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	memset( temp_BBT, 0xff, mem_page_num*page_size);
//NEW method!
	for(i=0;i<BACKUP_BBT;i++){
	    //rc = rtk_read_ecc_page(bbt_page+(i*ppb), &NfDataBuf, &NfSpareBuf,page_size);
		rc = this->read_ecc_page(mtd, numchips-1, bbt_page+(i*ppb), this->g_databuf, this->g_oobbuf);
	    if(!NAND_ADDR_CYCLE)
		    isbbt = this->g_oobbuf[0];
	    else
		    isbbt = this->g_oobbuf[5];
		if(!rc){
		    if(isbbt==BBT_TAG)//bbt has already created
				is_first_boot = 0;
		}
	}
	printk("%s: is_first_boot:%d\n\r",__FUNCTION__, is_first_boot);

	for(i=0;i<BACKUP_BBT;i++){
		mem_page_num_tmp = mem_page_num;
		page_counter=0;load_bbt_error=0;
		rc = rtk_block_isbad(mtd,numchips-1,(bbt_page+(i*ppb))*page_size);
		if(!rc){
			printk("load bbt table:%d page:%d\n\r",i, (bbt_page+(i*ppb)));
		    //rc = rtk_read_ecc_page(bbt_page+(i*ppb), &NfDataBuf, &NfSpareBuf,page_size);
			rc = this->read_ecc_page(mtd, numchips-1, bbt_page+(i*ppb), this->g_databuf, this->g_oobbuf);
		    if(!NAND_ADDR_CYCLE)
			    isbbt = this->g_oobbuf[0];
		    else
			    isbbt = this->g_oobbuf[5];
	        if(!rc){
			    if(isbbt == BBT_TAG){
			        printk("[%s] have created bbt table:%d on block %d, just loads it !!\n\r", __FUNCTION__,i,(bbt_page/ppb)+i);
			        //memcpy( temp_BBT, &NfDataBuf, page_size );
					memcpy( temp_BBT, this->g_databuf, page_size );
			        page_counter++;
			        mem_page_num_tmp--;
			        while( mem_page_num_tmp>0 ){
				        //if( rtk_read_ecc_page((bbt_page+(i*ppb)+page_counter), &NfDataBuf, &NfSpareBuf, page_size)){
				        if( this->read_ecc_page(mtd, 0, bbt_page+(i*ppb)+page_counter, this->g_databuf, this->g_oobbuf) ){				        
					        printk("[%s] load bbt table%d error!!\n\r", __FUNCTION__,i);
					        //free(temp_BBT);
					        //load_bbt1 = 1;
                            load_bbt_error=1;
					        //return -1;
					        //goto TRY_LOAD_BBT1;
			                error_count++;					        
					        break;
				        }
				        //memcpy( temp_BBT+page_counter*page_size, &NfDataBuf, page_size );
						memcpy( temp_BBT+page_counter*page_size, this->g_databuf, page_size );				        
				        page_counter++;
				        mem_page_num_tmp--;
			        }
					if(!load_bbt_error){
					    memcpy( this->bbt, temp_BBT, sizeof(BB_t)*RBA );
					    printk("check bbt table:%d OK\n\r",i);
					    goto CHECK_BBT_OK;
					}
			    }else{
					if(is_first_boot){
					    printk("Create bbt table:%d is_first_boot:%d\n\r",i, is_first_boot);						
				        rtk_create_bbt(mtd, bbt_page+(i*ppb));
					}
			    }
		    }
		}else{
            printk("bbt table:%d block:%d page:%d is bad\n\r",i,(bbt_page/ppb)+i,bbt_page+(i*ppb));
			error_count++;
		}
	}
CHECK_BBT_OK:			
#if 0	
    rc = this->read_ecc_page(mtd, 0, bbt_page, this->g_databuf, this->g_oobbuf);
//ccwei 111116

	if(!NAND_ADDR_CYCLE)
	    isbbt_b0 = this->g_oobbuf[0];
	else
        isbbt_b0 = this->g_oobbuf[5];
/* //ccwei 120116	
        check0 = this->g_oobbuf[0];
	check1 = this->g_oobbuf[1];
	check2 = this->g_oobbuf[2];
	check3 = this->g_oobbuf[3];
	//printk("[%s, line %d]  check0 = %d, check1 = %d, check2 = %d, check3 = %d\n",__FUNCTION__,__LINE__,check0,check1,check2,check3);
*/
	//printk("[%s, line %d] isbbt_b0 = %d bbt_page %d\n",__FUNCTION__,__LINE__,isbbt_b0, bbt_page);
	if ( !rc ){
		if ( isbbt_b0 == BBT_TAG ){
			printk("[%s] have created bbt B0, just loads it !!\n", __FUNCTION__);
			memcpy( temp_BBT, this->g_databuf, page_size );
			page_counter++;
			mem_page_num--;

			while( mem_page_num>0 ){
				if ( this->read_ecc_page(mtd, 0, bbt_page+page_counter, this->g_databuf, this->g_oobbuf) ){
					printk("[%s] load bbt B0 error!!\n", __FUNCTION__);
					kfree(temp_BBT);
					return -1;
				}
				memcpy( temp_BBT+page_counter*page_size, this->g_databuf, page_size );
				page_counter++;
				mem_page_num--;
			}
			memcpy( this->bbt, temp_BBT, sizeof(BB_t)*RBA );
		}else{
			printk("[%s] read BBT B0 tags fails, try to load BBT B1\n", __FUNCTION__);
			rc = this->read_ecc_page(mtd, 0, bbt1_block_num, this->g_databuf, this->g_oobbuf);

                        //ccwei 120116
                        //isbbt_b1 = this->g_oobbuf[0];
                        if(!NAND_ADDR_CYCLE)
                            isbbt_b1 = this->g_oobbuf[0];
                        else
                            isbbt_b1 = this->g_oobbuf[5];
			if ( !rc ){
				if ( isbbt_b1 == BBT_TAG ){
					printk("[%s] have created bbt B1, just loads it !!\n", __FUNCTION__);
					memcpy( temp_BBT, this->g_databuf, page_size );
					page_counter++;
					mem_page_num--;

					while( mem_page_num>0 ){
						if ( this->read_ecc_page(mtd, 0, bbt1_block_num+page_counter, this->g_databuf, this->g_oobbuf) ){
							printk("[%s] load bbt B1 error!!\n", __FUNCTION__);
							kfree(temp_BBT);
							return -1;
						}
						memcpy( temp_BBT+page_counter*page_size, this->g_databuf, page_size );
						page_counter++;
						mem_page_num--;
					}
					memcpy( this->bbt, temp_BBT, sizeof(BB_t)*RBA );
				}else{
					printk("[%s] read BBT B1 tags fails, nand driver will creat BBT B0 and B1\n", __FUNCTION__);
					rtk_create_bbt(mtd, bbt_page);
					rtk_create_bbt(mtd, bbt_page+this->ppb);
				}
			}else{
				printk("[%s] read BBT B1 with HW ECC fails, nand driver will creat BBT B0\n", __FUNCTION__);
				rtk_create_bbt(mtd, bbt_page);
			}
		}
	}else{
		printk("[%s] read BBT B0 with HW ECC error, try to load BBT B1\n", __FUNCTION__);
		rc = this->read_ecc_page(mtd, 0, bbt1_block_num, this->g_databuf, this->g_oobbuf);

		if(!NAND_ADDR_CYCLE){
			isbbt_b1 = this->g_oobbuf[0];
		}else{
			isbbt_b1 = this->g_oobbuf[5];
		}	
		if ( !rc ){
			if ( isbbt_b1 == BBT_TAG ){
				printk("[%s] have created bbt B1, just loads it !!\n", __FUNCTION__);
				memcpy( temp_BBT, this->g_databuf, page_size );
				page_counter++;
				mem_page_num--;

				while( mem_page_num>0 ){
					if ( this->read_ecc_page(mtd, 0, bbt1_block_num+page_counter, this->g_databuf, this->g_oobbuf) ){
						printk("[%s] load bbt B1 error!!\n", __FUNCTION__);
						kfree(temp_BBT);
						return -1;
					}
					memcpy( temp_BBT+page_counter*page_size, this->g_databuf, page_size );
					page_counter++;
					mem_page_num--;
				}
				memcpy( this->bbt, temp_BBT, sizeof(BB_t)*RBA );
			}else{
				printk("[%s] read BBT B1 tags fails, nand driver will creat BBT B1\n", __FUNCTION__);
				rtk_create_bbt(mtd, bbt1_block_num);
			}
		}else{
			printk("[%s] read BBT B0 and B1 with HW ECC fails\n", __FUNCTION__);
			kfree(temp_BBT);
			return -1;
		}
	}
#endif	
	dump_BBT(mtd);

	if (temp_BBT)
		kfree(temp_BBT);
    if(error_count >= BACKUP_BBT){
        rc = -1;
		printk("%d bbt table are all bad!(T______T)\n\r", BACKUP_BBT);
	}	
	//printk("[%s, line %d] rc = %d\n",__FUNCTION__,__LINE__,rc);	
	return rc;
}
#endif
static inline int generic_ffs(int x)
{
        int r = 1;

        if (!x)
                return 0;
        if (!(x & 0xffff)) {
                x >>= 16;
                r += 16;
        }
        if (!(x & 0xff)) {
                x >>= 8;
                r += 8;
        }
        if (!(x & 0xf)) {
                x >>= 4;
                r += 4;
        }
        if (!(x & 3)) {
                x >>= 2;
                r += 2;
        }
        if (!(x & 1)) {
                x >>= 1;
                r += 1;
        }
        return r;
}


int rtk_nand_scan(struct mtd_info *mtd, int maxchips)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned char id[6];
	unsigned int device_size=0;
	unsigned int i;
	unsigned int nand_type_id ;
	int rtk_lookup_table_flag=0;
	unsigned char maker_code;	
	unsigned char device_code; 
	unsigned char B5th;
	unsigned char B6th;
	unsigned int block_size;
	unsigned int num_chips = 1;
	unsigned int chip_size=0;
	unsigned int num_chips_probe = 1;
#ifdef CONFIG_RTK_NAND_BBT
	unsigned int block_v2r_num=0;
#endif	
	if ( !this->select_chip )
		this->select_chip = nand_select_chip;

#ifdef CONFIG_RTK_NAND_BBT
	if ( !this->scan_bbt )
		this->scan_bbt = rtk_nand_scan_bbt;

	if ( !this->scan_v2r_bbt )
		this->scan_v2r_bbt = rtk_scan_v2r_bbt;
#endif
	this->active_chip = 0;
	this->select_chip(mtd, 0);

	mtd->name = "rtk_nand";
	printk("%s(%d):\n",__FUNCTION__,__LINE__);

	while(1){
		
		this->read_id(mtd, id,0);

		this->maker_code = maker_code = id[0];
		this->device_code = device_code = id[1];
		nand_type_id = maker_code<<24 | device_code<<16 | id[2]<<8 | id[3];
		B5th = id[4];
		B6th = id[5];

		 //czyao, only uses one die
		for (i=1;  i<maxchips; i++) {
			this->select_chip(mtd, i);
			this->read_id(mtd, id,i);
			if ( maker_code !=  id[0] ||device_code != id[1] )
				break;
		}

		rtk_writel(0xC00FFFFF, NACR);     //Enable ECC	
		rtk_writel(0x0000000F, NASR);     //clear NAND flash status register

		if (i > 1){
			num_chips_probe = i;
			printk(KERN_INFO "NAND Flash Controller detects %d dies\n", num_chips_probe);
		}


		for (i = 0; nand_device[i].name; i++){
			if ( nand_device[i].id==nand_type_id && 
				((nand_device[i].CycleID5th==0xff)?1:(nand_device[i].CycleID5th==B5th)) ){
				//rtk_writel( nand_device[i].T1, REG_T1 );  //czyao
				//rtk_writel( nand_device[i].T2, REG_T2 );  //czyao
				//rtk_writel( nand_device[i].T3, REG_T3 );  //czyao
				/*   //czyao
				if ( nand_type_id != HY27UT084G2M ){
					rtk_writel( 0x20, REG_MULTICHNL_MODE);
				}
				*/
				if (nand_device[i].size == num_chips_probe * nand_device[i].chipsize){
					if ( num_chips_probe == nand_device[i].num_chips ){
						printk("One %s chip has %d die(s) on board\n", 
							nand_device[i].name, nand_device[i].num_chips);
						//mtd->PartNum = nand_device[i].name;
						device_size = nand_device[i].size;
						chip_size = nand_device[i].chipsize;	
						page_size = nand_device[i].PageSize;
						block_size = nand_device[i].BlockSize;
						oob_size = nand_device[i].OobSize;
						num_chips = nand_device[i].num_chips;
						isLastPage = nand_device[i].isLastPage;
						rtk_lookup_table_flag = 1;
						printk("nand part=%s, id=%x, device_size=%u, chip_size=%u, num_chips=%d, isLastPage=%d\n", 
							nand_device[i].name, nand_device[i].id, nand_device[i].size, nand_device[i].chipsize, 
							nand_device[i].num_chips, nand_device[i].isLastPage);
						break;
					}				
				}else{
					if ( !strcmp(nand_device[i].name, "HY27UF084G2M" ) )
						continue;
					else{	
						printk("We have %d the same %s chips on board\n", 
							num_chips_probe/nand_device[i].num_chips, nand_device[i].name);
						//mtd->PartNum = nand_device[i].name;
						device_size = nand_device[i].size;
						chip_size = nand_device[i].chipsize;	
						page_size = nand_device[i].PageSize;
						block_size = nand_device[i].BlockSize;
						oob_size = nand_device[i].OobSize;
						num_chips = nand_device[i].num_chips;
						isLastPage = nand_device[i].isLastPage;
						rtk_lookup_table_flag = 1;
						printk("nand part=%s, id=%x, device_size=%d, chip_size=%d,\nnum_chips=%d, isLastPage=%d\n", 
							nand_device[i].name, nand_device[i].id, nand_device[i].size, nand_device[i].chipsize, 
							nand_device[i].num_chips, nand_device[i].isLastPage);
						break;
					}
				}
			}
		}
		
		if ( !rtk_lookup_table_flag ){
			printk("Warning: Lookup Table do not have this nand flash !!\n");
			printk ("%s: Manufacture ID=0x%02x, Chip ID=0x%02x, "
					"3thID=0x%02x, 4thID=0x%02x, 5thID=0x%02x, 6thID=0x%02x\n",
					mtd->name, id[0], id[1], id[2], id[3], id[4], id[5]);
			return -1;
		}
		
		this->page_shift = generic_ffs(page_size)-1; 
		this->phys_erase_shift = generic_ffs(block_size)-1;
		this->oob_shift = generic_ffs(oob_size)-1;
		ppb = this->ppb = block_size >> this->page_shift;
#ifdef CONFIG_RTK_NAND_BBT
		this->block_size = block_size;
#endif
		if (chip_size){
			this->block_num = chip_size >> this->phys_erase_shift;
			this->page_num = chip_size >> this->page_shift;
			this->chipsize = chip_size;
			this->device_size = device_size;
			this->chip_shift =  generic_ffs(this->chipsize )-1;
		}

		printk("[%s, line %d] page_shift=%d, phy_erase_shift=%d, oob_shift=%d, chip_shift=%d\n"
			,__FUNCTION__,__LINE__,this->page_shift, this->phys_erase_shift, this->oob_shift,this->chip_shift);

		printk("%s(%d): pagesize=%x,oob=%x mtd=%p(%p)\n",__FUNCTION__,__LINE__,page_size,oob_size,mtd,&mtd->writesize);

		this->pagemask = (this->chipsize >> this->page_shift) - 1;
	
		mtd->oobsize = this->oob_size = oob_size;

		mtd->writesize = page_size;
			
		mtd->erasesize = block_size;
//ccwei 111116
		mtd->writesize_shift = generic_ffs(page_size)-1;
		
		mtd->erasesize_shift = generic_ffs(block_size)-1;
//end 111116
		//mtd->eccsize = 512; 	
	
		//this->eccmode = MTD_ECC_RTK_HW;  //czyao

		break;

	} //end of while(1)

	this->select_chip(mtd, 0);
			
	if ( num_chips != num_chips_probe )
		this->numchips = num_chips_probe;
	else
		this->numchips = num_chips;

	//printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	mtd->size = this->numchips * this->chipsize;

	if (mtd->size > 0xFFFFFFFF) {
		panic("MTD Size too large\n");
	}

#ifdef CONFIG_RTK_NAND_BBT
	RBA = this->RBA = ((u32)mtd->size/block_size) * this->RBA_PERCENT/100;
	printk("[%s, line %d], RBA=%d, this->RBA_PERCENT = %d\n",__func__,__LINE__,RBA,this->RBA_PERCENT);


	this->bbt = kmalloc( sizeof(BB_t)*RBA, GFP_KERNEL );
	if ( !this->bbt ){
		printk("%s: Error, no enough memory for BBT\n",__FUNCTION__);
		return -1;
	}
	//printk("%s, %s, line %d, REG(NACR) = 0x%08x\n",__FILE__,__func__,__LINE__,rtk_readl(NACR));
	memset(this->bbt, 0,  sizeof(BB_t)*RBA);

	block_v2r_num = ((BOOT_SIZE + USER_SPACE_SIZE + VIMG_SPACE_SIZE + VIMG_SPACE_SIZE) >> this->phys_erase_shift);
	printk("[%s, line %d] block_v2r_num %d\n\r",__FUNCTION__,__LINE__, block_v2r_num);

	this->bbt_v2r = kmalloc(sizeof(BB_v2r)*block_v2r_num, GFP_KERNEL );
	if ( !this->bbt_v2r ){
		printk("%s: Error, no enough memory for bbt_v2r\n",__FUNCTION__);
		return -1;
	}
	//printk("%s, %s, line %d, REG(NACR) = 0x%08x\n",__FILE__,__func__,__LINE__,rtk_readl(NACR));
	memset(this->bbt_v2r, 0,  sizeof(BB_v2r)*block_v2r_num);
#endif	
	//printk("%s, %s, line %d, REG(NACR) = 0x%08x\n",__FILE__,__func__,__LINE__,rtk_readl(NACR));
	this->g_databuf = kmalloc( page_size, GFP_KERNEL );
	if ( !this->g_databuf ){
		printk("%s: Error, no enough memory for g_databuf\n",__FUNCTION__);
		return -ENOMEM;
	}
	memset(this->g_databuf, 0xff, page_size);

	//printk("%s, %s, line %d, REG(NACR) = 0x%08x\n",__FILE__,__func__,__LINE__,rtk_readl(NACR));
	this->g_oobbuf = kmalloc( oob_size, GFP_KERNEL );
	if ( !this->g_oobbuf ){
		printk("%s: Error, no enough memory for g_oobbuf\n",__FUNCTION__);
		return -ENOMEM;
	}
	memset(this->g_oobbuf, 0xff, oob_size);

	unsigned int flag_size =  (this->numchips * this->page_num) >> 3;
	//printk("%s, %s, line %d, flag_size=%d\n",__FILE__,__func__,__LINE__,flag_size);
	
	unsigned int mempage_order = get_order(flag_size);
	this->erase_page_flag = (void *)__get_free_pages(GFP_KERNEL, mempage_order);	
	if ( !this->erase_page_flag ){
		printk("%s: Error, no enough memory for erase_page_flag\n",__FUNCTION__);
		return -ENOMEM;
	}
	//printk("%s, %s, line %d, REG(NACR) = 0x%08x\n",__FILE__,__func__,__LINE__,rtk_readl(NACR));
	memset ( (__u32 *)this->erase_page_flag, 0, flag_size);

	mtd->type			= MTD_NANDFLASH;
	mtd->flags			= MTD_CAP_NANDFLASH;
	//mtd->ecctype			= MTD_ECC_NONE;
	mtd->erase			= nand_erase;
	mtd->point			= NULL;
	mtd->unpoint			= NULL;
	mtd->read			= nand_read;
	mtd->write			= nand_write;
	//mtd->read_ecc		= nand_read_ecc;
	//mtd->write_ecc		= nand_write_ecc;
	mtd->read_oob		= nand_read_oob;
	mtd->write_oob		= nand_write_oob;
	//mtd->readv			= NULL;
	mtd->writev			= NULL;
	//mtd->readv_ecc		= NULL;
	//mtd->writev_ecc		= NULL;
	mtd->sync			= nand_sync;
	mtd->lock			= NULL;
	mtd->unlock			= NULL;
	//mtd->suspend		= nand_suspend;
	//mtd->resume		= nand_resume;
	mtd->owner			= THIS_MODULE;

	mtd->block_isbad		= nand_block_isbad;
	mtd->block_markbad	= nand_block_markbad;
	
	/* Ken: 20090210 */
	//mtd->reload_bbt 		= rtk_nand_scan_bbt;

	mtd->owner = THIS_MODULE;

	
#ifdef NAND_MP
	/* =========================== for MP usage =======================*/
	mp_time_para = (char *) parse_token(platform_info.system_parameters, "mp_time_para");
	if ( mp_time_para && strlen(mp_time_para) )
		mp_time_para_value = simple_strtoul(mp_time_para, &mp_time_para, 10);

	if ( mp_time_para_value ){
		/* reset the optimal speed */
		rtk_writel( mp_time_para_value, REG_T1 );
		rtk_writel( mp_time_para_value, REG_T2 );
		rtk_writel( mp_time_para_value, REG_T3 );
	}

	/* get nf_clock from /sys/realtek_boards/system_parameters */
	nf_clock = (char *) parse_token(platform_info.system_parameters, "nf_clock");
	if ( nf_clock && strlen(nf_clock) )
		nf_clock_value = simple_strtoul(nf_clock, &nf_clock, 10);

	if ( nf_clock_value )
		NF_CKSEL(/*mtd->PartNum*/"NAND", nf_clock_value);

	/* get mp_erase_nand from /sys/realtek_boards/system_parameters */
	mp_erase_nand = (char *) parse_token(platform_info.system_parameters, "mp_erase_nand");
	//printk("mp_erase_nand=%s\n", mp_erase_nand);
	if ( mp_erase_nand && strlen(mp_erase_nand) )
		mp_erase_flag = simple_strtoul(mp_erase_nand, &mp_erase_nand, 10);
	//printk("mp_erase_flag=%d\n", mp_erase_flag);
	
	if ( mp_erase_flag ){
		int start_pos = 0;
		int start_page = (start_pos/mtd->erasesize)*ppb;
		int block_num = (mtd->size - start_pos)/mtd->erasesize;
		//printk("start_page=%d, block_num=%d\n", start_page, block_num);
		printk("Starting erasure all contents of nand for MP.\n");
		TEST_ERASE_ALL(mtd, start_page, block_num);
		this->select_chip(mtd, 0);
		return 0;
	}

	/* CMYu, 20090720, get mcp from /sys/realtek_boards/system_parameters */
	mcp = (char *) parse_token(platform_info.system_parameters, "mcp");
	//printk("mcp=%s\n", mcp);
	if (mcp){
		if ( strstr(mcp, "ecb") )
			this->mcp = MCP_AES_ECB;
		else if ( strstr(mcp, "cbc") )
			this->mcp = MCP_AES_CBC;
		else if ( strstr(mcp, "ctr") )
			this->mcp = MCP_AES_CTR;
		else
			this->mcp = MCP_NONE;			
	}
	
	//for MCP test
	//this->mcp = MCP_AES_ECB;
	//printk("[%s] this->mcp=%d\n", __FUNCTION__, this->mcp);
#endif //NAND_MP

	//printk("%s, %s, line %d, REG(NACR) = 0x%08x\n",__FILE__,__func__,__LINE__,rtk_readl(NACR));
	/* =========== WE Over spec: Underclocking lists: ========== */
	switch(nand_type_id){
		case HY27UT084G2M:	//WE: 40 ns
			//NF_CKSEL(mtd->PartNum, 0x04);	//43.2 MHz
			break;
		case HY27UF081G2A:	//WE: 15 ns
		case HY27UF082G2A:
		case K9G4G08U0A:
		case K9G8G08U0M:
		case TC58NVG0S3C:
		case TC58NVG1S3C:
		case HY27UT084G2A:
			//NF_CKSEL(mtd->PartNum, 0x03);	//54 MHz
			break;
		default:
			;
	}	
	//printk("%s, %s, line %d, REG(NACR) = 0x%08x\n",__FILE__,__func__,__LINE__,rtk_readl(NACR));		
    //ccwei:120203
    #ifdef CONFIG_RTK_NAND_BBT
	       this->scan_v2r_bbt(mtd);
	printk("%s, line %d\n",__func__,__LINE__);			
	return this->scan_bbt(mtd);
	#else
	return 1;
	#endif
}

/*
int TEST_ERASE_ALL(struct mtd_info *mtd, int page, int bc)
{
	struct nand_chip *this = (struct nand_chip *) mtd->priv;
	int i;
	int chip_block_num = this->block_num;
	int start_block = page/ppb;
	int block_in_die; 
	int rc = 0;
	int chipnr =0, block;
	
	if ( page & (ppb-1) ){
		page = (page/ppb)*ppb;
	}

	for ( i=0; i<bc; i++){
		block_in_die = start_block + i;
		chipnr = block_in_die/chip_block_num;
		block = block_in_die%chip_block_num;
		this->select_chip(mtd, block_in_die/chip_block_num);
		rc = this->erase_block(mtd, chipnr, block*ppb);
		if ( rc<0 ){
			this->g_oobbuf[0] = 0x00;
			if ( isLastPage ){
				this->write_oob(mtd, chipnr, block*ppb+ppb-1, oob_size, this->g_oobbuf);
				this->write_oob(mtd, chipnr, block*ppb+ppb-2, oob_size, this->g_oobbuf);
			}else{
				this->write_oob(mtd, chipnr, block*ppb, oob_size, this->g_oobbuf);
				this->write_oob(mtd, chipnr, block*ppb+1, oob_size, this->g_oobbuf);
			}
		}
	}

	return 0;
}
*/


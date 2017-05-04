/******************************************************************************
 * $Id: rtk_nand_sd5.c,v 1.4 2012/04/02 06:39:30 ccwei0908 Exp $
 * drivers/mtd/nand/rtk_nand.c
 * Overview: Realtek NAND Flash Controller Driver
 * Copyright (c) 2008 Realtek Semiconductor Corp. All Rights Reserved.
 * Modification History:
 *    #000 2008-05-30 CMYu   create file
 *
 *******************************************************************************/
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/pm.h>
#include <asm/io.h>
#include "bspchip.h" 
#include <soc/kernel_soc.h> //soc.h must include
#include "./rtk_nand.h" 
#include <linux/bitops.h>
#include <mtd/mtd-abi.h>
//#include <asm/r4kcache.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/root_dev.h>
#include "./rtk_nand.h"

#define BANNER  "Realtek Luna NAND Flash Driver"
#define VERSION  "$Id: luna_nand.c,2012/04/02 06:39:30 ccwei0908 Exp $"


#define MTDSIZE	(sizeof (struct mtd_info) + sizeof (struct nand_chip))
#define MAX_PARTITIONS	16

#ifndef CONFIG_USE_PRELOADER_PARAMETERS 
extern nand_flash_info_t nandflash_info;
#else
/*define parameters struct for nand flash information */
extern parameter_to_bootloader_t kernel_soc_parameters;
#define plr_param_soc (kernel_soc_parameters.soc)
#define nandflash_info  (kernel_soc_parameters.soc.flash_info)
#endif

/* nand driver low-level functions */
static void rtk_nand_read_id(struct mtd_info *mtd, unsigned char id[5], int chip_sel);
static int rtk_read_oob(struct mtd_info *mtd, u16 chipnr, int page, int len, u_char *buf);
static int rtk_read_ecc_page(struct mtd_info *mtd, u16 chipnr, unsigned int chunk_id, 
			u_char *data, u_char *oob_buf);
static int rtk_write_ecc_page(struct mtd_info *mtd, u16 chipnr, unsigned int page, 
			const u_char *data, const u_char *oob_buf, int isBBT);
	
static int _nand_erase_block(struct mtd_info *mtd, u16 chipnr, int chunk_id);


/* Global Variables */
struct mtd_info *rtk_mtd; 
static DECLARE_MUTEX (sem);
static int page_size,chunk_size, oob_size, ppb;

static u32_t num_chunk_per_block;

#define ADDR_NOTALIGNED(byte, addr) ((addr & (byte-1)) != 0)  

#define flash_info_num_chunk_per_block	  \
		((nandflash_info.num_page_per_block)/(nandflash_info.page_per_chunk))



/* for fixed partition */
const char *ptypes[] = {"cmdlinepart", NULL};
//const char *ptypes[] = {"mtdparts=rtk_nand:640k@0(boot),6M@0x180000(linux),-(rootfs)", NULL};


//eric, use cmdlinepart now
static struct mtd_partition rtl8686_parts[] = {
	/*patch from linux 2.4*/
	{ name: "boot", 	offset: 0, size:  0xa0000, mask_flags: 0 }, 
	{ name: "linux",	offset: 0x180000, size: 0x600000 , mask_flags:0}, 
	{ name: "rootfs1",	offset: 0xa00000, size: 0xa00000 , mask_flags:0}, 
	{ name: "rootfs2",	offset: 0x1400000, size: 0xa00000 , mask_flags:0}, 
	{ name: "rootfs3",	offset: 0x1e00000, size: 0xa00000 , mask_flags:0}, 	
//	{ name: "rtk_rootfs",	offset: MTDPART_OFS_NXTBLK, size: MTDPART_SIZ_FULL, mask_flags:0}, 	
};



/*************************************************************************	
**  check_ready()
**	descriptions: check nand flash busy/ready bit
**	parameters: 
**	return: 
**  note: call by internal rtk flash driver 
*************************************************************************/
static void check_ready()
{
	while(1) {
		if(  ( rtk_readl(NACR) & 0x80000000) == 0x80000000 ) 
			break;
	}
}

/*************************************************************************	
**  rtk_nand_read_id()
**	descriptions: read nand flash id
**	parameters: 
**	return: id[5] , flash id
**  note: chip_sel not use
*************************************************************************/
static void rtk_nand_read_id(struct mtd_info *mtd, u_char id[5], int chip_sel)
{
	int id_chain;
	int nand_strap_pin;

	if(chip_sel>1)  //SD5 only supports chip0, chip1
		return;


	check_ready();
	rtk_writel( (rtk_readl(NACR) |ECC_enable|RBO|WBO), NACR);

	//rtk_writel(0x0, NACMR);	
	rtk_writel( ((1<<(Chip_Seletc_Base+chip_sel))|CMD_READ_ID) , NACMR);          //read ID command
	check_ready();

	rtk_writel( (0x0 |AD2EN|AD1EN|AD0EN) , NAADR);  //dummy address cycle
	check_ready();
	
	id_chain = rtk_readl(NADR);
	//printk("rtk_nand_read_id id_chain is %lx\n",id_chain);
	id[0] = id_chain & 0xff;
	id[1] = (id_chain >> 8) & 0xff;
	id[2] = (id_chain >> 16) & 0xff;
	id[3] = (id_chain >> 24) & 0xff;

	id_chain = rtk_readl(NADR);
	id[4] = id_chain & 0xff;

	printk("rtk_nand_read_id id is %x %x %x %x %x",id[0],id[1],id[2],id[3],id[4]);

	rtk_writel( 0x0, NACMR);
	rtk_writel( 0x0, NAADR);


}


/*************************************************************************	
**  rtk_PIO_read_is_allone()
**	descriptions: pio read nand data, check data is all one
**	parameters:  page index, offset
**	return:
**  note: internal usage for rtk driver
*************************************************************************/

int rtk_PIO_read_is_allone(int page, int offset)
{
	int i;
	unsigned int flash_addr1=0;
	unsigned int data_out;
	int rlen, real_page;
	unsigned int cmd;
	real_page = page;
	
/*
	if(offset==0 || offset==256){
		rlen = page_size/2;
		
		if(offset==0)
			cmd = CMD_PG_READ_A;
		else
			cmd = CMD_PG_READ_B;
		
	}else{
		rlen = oob_size;
		cmd = CMD_PG_READ_C;
	}
*/
	rlen = page_size + oob_size;
	cmd = CMD_PG_READ_A;

	rtk_writel(0xc00fffff, NACR);

	/* Command cycle 1*/
	rtk_writel((CECS0|cmd), NACMR);

	check_ready();

	flash_addr1 |= ((real_page & 0xffffff) << 8) ;

	/* Give address */
	rtk_writel( (AD2EN|AD1EN|AD0EN|flash_addr1), NAADR);

	check_ready();

	for(i=0; i<(rlen/4); i++){
		data_out = rtk_readl(NADR);
		//printf("[%3d] 0x%08X \n",i, data_out);
		if(data_out!=0xffffffff){
			printk("[%3d] 0x%08X \n",i, data_out);
			printk("%s: page %d offset %d i %d rlen %d\n",__FUNCTION__, page, offset, i, rlen);			
			return -1;
		}
	}

	check_ready();
	rtk_writel(0, NACMR);
	
	return 0;
	
}


/*************************************************************************	
**  rtk_check_allone_512()
**	descriptions: pio read nand data, check data is all one for 512 page size.
**	parameters:  page index,
**	return:
**  note: internal usage for rtk driver (RS code only)
*************************************************************************/

int rtk_check_allone_512(int page)
{
	int rc=0;

	//printf("[%s] page = %d\n",__func__,page);
	rc = rtk_PIO_read_is_allone(page,0);
	if(rc < 0)
		goto read_finish;
/*
	rc = rtk_PIO_read_is_allone(page,256);
	if(rc < 0)
		goto read_finish;

	rc = rtk_PIO_read_is_allone(page,512);
*/
read_finish:
	return rc;
}

/*************************************************************************	
**  rtk_check_allone()
**	descriptions: pio read nand data, check data is all one for large page size.
**	parameters:  page index,
**	return:
**  note: internal usage for rtk driver (RS code only)
*************************************************************************/

int rtk_check_allone(int page, int offset)
{
	unsigned int flash_addr1, flash_addr2;
	unsigned int data_out;
	int real_page, i, rlen;;

	real_page = page;

	/* rlen is equal to (512 + 16) */
	rlen = 528; 
	
	rtk_writel(0xc00fffff, NACR);

	/* Command cycle 1*/
	rtk_writel((CECS0|CMD_PG_READ_C1), NACMR);

	check_ready();

	flash_addr1 =  ((real_page & 0xff) << 16) | offset;
	flash_addr2 = (real_page >> 8) & 0xffffff;

	/* Give address */
	rtk_writel( (enNextAD|AD2EN|AD1EN|AD0EN|flash_addr1), NAADR);
	rtk_writel( (AD1EN|AD0EN|flash_addr2), NAADR);

	/* Command cycle 2*/
	rtk_writel((CECS0|CMD_PG_READ_C2), NACMR);

	check_ready();

	for(i=0; i<(rlen/4); i++){
		data_out = rtk_readl(NADR);	
		if( data_out != 0xffffffff)
			return -1;
	}

	check_ready();
	rtk_writel(0, NACMR);

	return 0;

}


/*function : rtk_read_oob */
static int rtk_read_oob (struct mtd_info *mtd, u16 chipnr, int chunk_id, int len, u_char *oob_buf)
{
	struct nand_chip *this = (struct nand_chip *) mtd->priv;

	return parameters._nand_read_chunk(this->g_databuf,oob_buf,chunk_id);
}

/* rtk_read_ecc_page*/
static int rtk_read_ecc_page (struct mtd_info *mtd, u16 chipnr, unsigned int chunk_id, 
			u_char *data_buf, u_char *oob_buf)
{

	return parameters._nand_read_chunk(data_buf,oob_buf,chunk_id);

}

/* function: write callback function */
int rtk_write_ecc_page (struct mtd_info *mtd, u16 chipnr, unsigned int chunk_id, 
			const u_char *data_buf, const u_char *oob_buf, int isBBT)	
{
	return parameters._nand_write_chunk(data_buf,oob_buf,chunk_id);	
}



/*************************************************************************	
**  rtk_nand_profile()
**	descriptions: rtk luna nand driver init function
**	parameters: 
**	return: 
**  note:  
*************************************************************************/
static int rtk_nand_profile (void)
{
	int maxchips = 4;	
	char *ptype;
	int pnum = 0;
	struct mtd_partition *mtd_parts;
	struct nand_chip *this = (struct nand_chip *)rtk_mtd->priv;
	
	if (rtk_nand_scan (rtk_mtd, maxchips) < 0 || rtk_mtd->size == 0){
		printk("%s: Error, cannot do nand_scan(on-board)\n", __FUNCTION__);
		return -ENODEV;
	}

	/*check page size(write size) is 512/2048/4096.. must 512byte align */
	if ( !(rtk_mtd->writesize&(0x200-1)) )
		;//rtk_writel( rtk_mtd->oobblock >> 9, REG_PAGE_LEN);
	else{ 
		printk("Error: pagesize is not 512Byte Multiple");
		return -1;
	}

	

#ifdef CONFIG_MTD_CMDLINE_PARTS
	/*partitions from cmdline */
	ptype = (char *)ptypes[0];
	pnum = parse_mtd_partitions (rtk_mtd, ptypes, &mtd_parts, 0);



	if (pnum <= 0) {	
		printk(KERN_NOTICE "RTK: using the whole nand as a partitoin\n");
		if(add_mtd_device(rtk_mtd)) {
			printk(KERN_WARNING "RTK: Failed to register new nand device\n");
			return -EAGAIN;
		}
	}else{
		printk(KERN_NOTICE "RTK: using dynamic nand partition\n");
		if (add_mtd_partitions (rtk_mtd, mtd_parts, pnum)) {	
			printk("%s: Error, cannot add %s device\n", 
					__FUNCTION__, rtk_mtd->name);
			rtk_mtd->size = 0;
			return -EAGAIN;
		}	
	}

#else
/* fixed partition ,modify rtl8686_parts table*/
	if(add_mtd_device(rtk_mtd)) {
		printk(KERN_WARNING "RTK: Failed to register new nand device\n");
		return -EAGAIN;
	}

	if (add_mtd_partitions (rtk_mtd, rtl8686_parts, ARRAY_SIZE(rtl8686_parts))) {	
			printk("%s: Error, cannot add %s device\n", 
					__FUNCTION__, rtk_mtd->name);
			rtk_mtd->size = 0;
			return -EAGAIN;
		}

	ROOT_DEV = MKDEV(MTD_BLOCK_MAJOR, 0);

#endif
	return 0;
}

/*************************************************************************	
**  rtk_read_proc_nandinfo()
**	descriptions: proc read nand flash information
**	parameters: 
**	return: 
**  note:  
*************************************************************************/

int rtk_read_proc_nandinfo(char *buf, char **start, off_t offset, int len, int *eof, void *data)
{
	struct nand_chip *this = (struct nand_chip *) rtk_mtd->priv;
	int wlen = 0;

	//wlen += sprintf(buf+wlen,"nand_PartNum:%s\n", rtk_mtd->PartNum); //czyao
	wlen += sprintf(buf+wlen,"nand_size:%u\n", this->device_size);
	wlen += sprintf(buf+wlen,"chip_size:%u\n", this->chipsize);
	wlen += sprintf(buf+wlen,"block_size:%u\n", rtk_mtd->erasesize);
	wlen += sprintf(buf+wlen,"chunk_size:%u\n", rtk_mtd->writesize);
	wlen += sprintf(buf+wlen,"real page_size:%u\n", page_size);
	wlen += sprintf(buf+wlen,"oob_size:%u\n", rtk_mtd->oobsize);
	wlen += sprintf(buf+wlen,"ppb:%u\n", rtk_mtd->erasesize/rtk_mtd->writesize);

	return wlen;
}

/*************************************************************************	
**  display_version()
**	descriptions: display driver version
**	parameters: 
**	return: 
**  note:  
*************************************************************************/

static void display_version (void)
{
	const __u8 *revision;
	const __u8 *date;
	char *running = (__u8 *)VERSION;
	strsep(&running, " ");
	strsep(&running, " ");
	revision = strsep(&running, " ");
	date = strsep(&running, " ");
	printk(BANNER " Rev:%s (%s)\n", revision, date);
}

/*************************************************************************	
**  rtk_nand_init()
**	descriptions: rtk luna nand driver init function
**	parameters: 
**	return: 
**  note:  
*************************************************************************/
static int __init rtk_nand_init (void)
{
	struct nand_chip *this;
	int rc = 0;
	
	display_version();
	
	rtk_mtd = kmalloc (MTDSIZE, GFP_KERNEL); //mtd_info struct + nand_chip struct
	
	if ( !rtk_mtd ){
		printk("%s: Error, no enough memory for rtk_mtd\n",__FUNCTION__);
		rc = -ENOMEM;
		goto EXIT;
	}
	memset ( (char *)rtk_mtd, 0, MTDSIZE);
	rtk_mtd->priv = this = (struct nand_chip *)(rtk_mtd+1);

	rtk_writel(0xC00FFFFF, NACR);     //Enable ECC	
	rtk_writel(0x0000000F, NASR);     //clear NAND flash status register

	/*nand_chip struct assign call back function */	
	this->read_id			= rtk_nand_read_id;	/*read id function */
	this->read_ecc_page 	= rtk_read_ecc_page;
	this->read_oob 			= rtk_read_oob;
	this->write_ecc_page	= rtk_write_ecc_page;
	this->erase_block		= _nand_erase_block;
	this->sync			= NULL;

	/*init nand flash */
	if( rtk_nand_profile() < 0 ){
		rc = -1;
		goto EXIT;
	}
	
	chunk_size = rtk_mtd->writesize;
	page_size=nandflash_info.page_size;
	oob_size = rtk_mtd->oobsize;
	ppb = (rtk_mtd->erasesize)/(rtk_mtd->writesize);
	
	create_proc_read_entry("nandinfo", 0, NULL, rtk_read_proc_nandinfo, NULL);
	

EXIT:
	if (rc < 0) {
		if (rtk_mtd){
			del_mtd_partitions (rtk_mtd);
			if (this->g_databuf)
				kfree(this->g_databuf);
			if(this->g_oobbuf)
				kfree(this->g_oobbuf);

			kfree(rtk_mtd);
		}
		remove_proc_entry("nandinfo", NULL);
	}else
		printk(KERN_INFO "Realtek Nand Flash Driver is successfully installing.\n");
	
	return rc;
}

/**	
**  nand_check_eccStatus()
**	descriptions: check ecc counter,status.
**	parameters:  none
**	return: 0: ecc ok, 1~N ,ecc can fix it, return ecc counter,  -1: ecc can't fix it.
**  note: 
**/
int nand_check_eccStatus()
{
	int error_count,status;
	int rc=0;
	
	status = rtk_readl(NASR);
	
	if( (status & NDRS)== NDRS){	 //read status
		if( status & NRER) { //ecc result = 1 , ecc read fail.
	 		error_count = (status & 0xf0) >> 4;
	 		/*ecc correction counter must change */
	 		if(error_count <=4 && error_count > 0 ) { //some bit fail, but ecc can fix it.
				//printk("[%s] R: Correctable HW ECC Error at page=%u, status=0x%08X\n\r", __FUNCTION__, page,status);
				status &= 0x0f; //clear NECN
				rtk_writel(status, NASR);
				rc=error_count;
			}else{ 
				//counter = 0 and ecc fail,  it mean ecc can' fix it
				rc = -1;					
			}
		}
	}else if( (status & NDWS)== NDWS){ //write status
			if( status & NWER) {
//				printk("Un-Correctable HW ECC Error in Write \n");
				rc=-1;		
			}
	}else {
		printk("!!!! nand_check_eccStatus ERROR !!! \n");
	}
	status |= 0x0f; //clear status.
	rtk_writel(status, NASR);	
	return rc;		
	
}

/* Read Toshiba BENAND ECC status */
/**	return:  -1 is fail , 0 is success */

int nand_check_toshiba_eccStatus(){

	int ecc_status;
	
	rtk_writel((CECS0|CMD_TOSHIBA_BENAND_ECCSTATUS),NACMR);//read ecc status
	check_ready();

	ecc_status=rtk_readl(NADR);
	
	//printf("status %x\n",ecc_status); //read 4byte data

	/*check ecc status have bit can't correct */

	if(( ecc_status & 0xff == 0x3f) || 
	    (((ecc_status >> 8) & 0xff)==0x2f) || 
	    (((ecc_status >> 16) & 0xff)==0x1f) || 
	    (((ecc_status >> 24) & 0xff)==0x0f)){
		//printf(" ecc error happen\n");		
		return -1;
	}
	return 0;
}


/**	
**  _nand_read_noecc_chunk()
**	descriptions: DAM read disable HW ECC for toshiba be-nandflash
**	parameters: chunk_buf, data buffer point 
 				spare , oob buffer point
	 			chunk_id, chunk number (will Calculate to real page number)
**	return:  -1 is fail, 0~N : ecc counter
**  note:  one chunk = 2k data.
**/

int
_nand_read_noecc_chunk(u8_t *chunk_buf, spare_u *spare, u32_t chunk_id) {
	int real_page = chunk_id*nandflash_info.page_per_chunk; //get real_page number
	int dma_counter = 4; //always 512byte * 4
	int dram_sa, oob_sa;
	int page_shift;
	int page_num[3];
	unsigned long flash_addr_t=0;
	unsigned long flash_addr2_t=0;
	int buf_pos=0;
	int return_value=0;
	int ecc_count;
	int i;
	int block = real_page/nandflash_info.num_page_per_block;
	u8_t tempvalue;
	u8_t *oob_area;
	u8_t temp_buf;
	u8_t *data_area=NULL;
	u8_t data_area0[512+16+CACHELINE_SIZE+CACHELINE_SIZE-4]; //data,oob point must 32 cache aligment
	unsigned long flags;
	
		
	if((chunk_buf == NULL)||(spare == NULL)){
		printk("chunk_buf/spare point is null\n");
		return -1;
	}
	
	memset(data_area0, 0xff, 512+16+CACHELINE_SIZE+CACHELINE_SIZE-4);
	data_area = (u8_t*) ((u32_t)(data_area0 + CACHELINE_SIZE-4) & 0xFFFFFFF0);
	oob_area=(u8_t*) data_area+512;
		
	oob_sa = ((uint32_t)oob_area) & (~M_mask);
	dram_sa = ((u32_t)data_area) & (~M_mask);
	
	rtk_writel(0x800FFFFF, NACR);	  //Disable CPU ECC
	rtk_writel(0x0000000F, NASR);	  //clear NAND flash status register
	
	for(page_shift=0;page_shift<3; page_shift++) {
		page_num[page_shift] = ((real_page>>(8*page_shift)) & 0xff);
		if(nandflash_info.page_size==2048){
			flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	  }else if(nandflash_info.page_size==4096){
		  flash_addr_t |= (page_num[page_shift] << (13+8*page_shift));
	  }else if(nandflash_info.page_size==512){ //512byte
			flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
		}
	}
	
	  // If the nand flash size is larger than 2G bytes, fill the other address for DMA
	  flash_addr2_t= (real_page >> 20);
	

  /* DMA start read */
	
	while(dma_counter>0){
		
		//set DMA RAM DATA start address
		rtk_writel(dram_sa, NADRSAR);
		//set DMA RAM oob start address , always use oob_sa buffer
		rtk_writel(oob_sa, NADTSAR);
		
		//set DMA flash start address,
		rtk_writel( flash_addr_t, NADFSAR); //write flash address 
		rtk_writel( flash_addr2_t, NADFSAR2);

		dma_cache_wback_inv((u32_t *)data_area,528);
		local_irq_save(flags);			
		//DMA read command
		rtk_writel( ((~TAG_DIS)&(DESC0|DMARE|LBC_64)),NADCRR);
		check_ready();
		//check status register 
		local_irq_restore(flags);
		//copy data
		memcpy(chunk_buf+(buf_pos*512), data_area, 512);	  
		//copy oob
		memcpy(spare->u8_oob+(buf_pos*6), oob_area, 6);	
		flash_addr_t += 528;//move next flash addr add (512+6+10)bytes
		dma_counter--;
		buf_pos++; 
				
	}

	
	//check status register
	return_value=nand_check_toshiba_eccStatus();

	if(block>0){
		//change bbi,data
		temp_buf=chunk_buf[nandflash_info.bbi_dma_offset];		
		chunk_buf[nandflash_info.bbi_dma_offset] = spare->u8_oob[nandflash_info.bbi_swap_offset];
		spare->u8_oob[nandflash_info.bbi_swap_offset]=temp_buf;
	}
	  
	return return_value;


}


/**
**  _nand_PIO_write_noecc_chunk()
**	descriptions: PIO write ,flash layout the same dma write
**	parameters:  chunk id  (must alignment of real_page/block)
**	return:  0:succes,-1 is fail
**  note: this function not check bad block table.
**/


int
_nand_PIO_write_noecc_chunk(u8_t *chunk_buf, spare_u *spare, u32_t chunk_id) {
	int real_page = chunk_id*nandflash_info.page_per_chunk; //get real_page number
	int dma_counter = 4; //always 512byte * 4
	int dram_sa, oob_sa;
	int page_shift;
	int page_num[3];
	unsigned long flash_addr_t=0;
	unsigned long flash_addr2_t=0;
	int buf_pos=0;
	int return_value=0;
	int ecc_count=0;
	int i;
	unsigned long flags;
	int block = real_page/nandflash_info.num_page_per_block;

	u8_t *oob_area;
	u8_t *data_area, data_area0[2112];  //2048+64 , currently driver only support page + oob = 2112

	//printk("_nand_PIO_write_noecc_chunk chunk_id %x\n",chunk_id);
	memset(data_area0, 0xff, sizeof(data_area0));

	data_area=(u8_t*) (u32_t) data_area0;
	oob_area=(u8_t*) data_area+512;
	
	//printk("_nand_write_chunk : spare->u8_oob 0x%p\n",spare->u8_oob );

	if(block>0){
		//swap bbi and data
		spare->u8_oob[nandflash_info.bbi_swap_offset]=chunk_buf[nandflash_info.bbi_dma_offset];
		chunk_buf[nandflash_info.bbi_dma_offset]=0xff;
	}

	/* dma move  4*528byte */
	
	while(dma_counter>0){
		//copy oob to buffer
		
		memcpy(data_area+(buf_pos*528), chunk_buf+(buf_pos*512), 512);
		oob_area=(u8_t*) data_area+(buf_pos*528)+512;
		memcpy(oob_area, spare->u8_oob+(buf_pos*6), 6);
		dma_counter--;
		buf_pos++;
	}


	if(block>0){
		//swap bbi and data again. (recovery data)
		chunk_buf[nandflash_info.bbi_dma_offset]= spare->u8_oob[nandflash_info.bbi_swap_offset];
		//eric mark not need
	//	dma_cache_wback_inv((u32_t *)chunk_buf,2048);
	}
	

	rtk_PIO_write(chunk_id,0,2112,data_area);

	return return_value;
}








/**	
**  _nand_read_chunk()
**	descriptions: DAM read one ecc chunk  
**	parameters: chunk_buf, data buffer point 
 								spare , oob buffer point
	 							chunk_id, chunk number (will Calculate to real page number)
**	return:  -1 is fail, 0~N : ecc counter
**  note:  one chunk = 2k data.
**/
// parameters._nand_read_chunk

int 
_nand_read_chunk(u8_t *chunk_buf, spare_u *spare, u32_t chunk_id) {
  int real_page = chunk_id*nandflash_info.page_per_chunk; //get real_page number
	int dma_counter = 4; //always 512byte * 4
	int dram_sa, oob_sa;
	int page_shift;
	int page_num[3];
	unsigned long flash_addr_t=0;
	unsigned long flash_addr2_t=0;
	int buf_pos=0;
	int return_value=0;
	int ecc_count;
	int i;
	int block = real_page/nandflash_info.num_page_per_block;
	u8_t tempvalue;
	u8_t *oob_area;
	u8_t temp_buf;
	u8_t *data_area=NULL;
	u8_t data_area0[512+16+CACHELINE_SIZE+CACHELINE_SIZE-4]; //data,oob point must 32 cache aligment
	unsigned long flags;

	
	if((chunk_buf == NULL)||(spare == NULL)){
		printk("chunk_buf/spare point is null\n");
		return -1;
	}

	memset(data_area0, 0xff, 512+16+CACHELINE_SIZE+CACHELINE_SIZE-4);
	data_area = (u8_t*) ((u32_t)(data_area0 + CACHELINE_SIZE-4) & 0xFFFFFFF0);
	oob_area=(u8_t*) data_area+512;
		
	oob_sa = ((uint32_t)oob_area) & (~M_mask);
	dram_sa = ((u32_t)data_area) & (~M_mask);



	rtk_writel(0xC00FFFFF, NACR);     //Enable ECC	
	rtk_writel(0x0000000F, NASR);	  //clear NAND flash status register

	for(page_shift=0;page_shift<3; page_shift++) {
		page_num[page_shift] = ((real_page>>(8*page_shift)) & 0xff);
		if(nandflash_info.page_size==2048){
			flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
	  }else if(nandflash_info.page_size==4096){
		  flash_addr_t |= (page_num[page_shift] << (13+8*page_shift));
	  }else if(nandflash_info.page_size==512){ //512byte
			flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
		}
	}

  // If the nand flash size is larger than 2G bytes, fill the other address for DMA
  flash_addr2_t= (real_page >> 20);

	

  /* DMA start read */

	while(dma_counter>0){
  	
		//set DMA RAM DATA start address
  		rtk_writel(dram_sa, NADRSAR);
		//set DMA RAM oob start address , always use oob_sa buffer
		rtk_writel(oob_sa, NADTSAR);
		
		//set DMA flash start address,
 		rtk_writel( flash_addr_t, NADFSAR); //write flash address 
#if 1 //BCH
		rtk_writel( flash_addr2_t, NADFSAR2);
#endif

		dma_cache_wback_inv((u32_t *)data_area,528);
		local_irq_save(flags);			
		//DMA read command
	    rtk_writel( ((~TAG_DIS)&(DESC0|DMARE|LBC_64)),NADCRR);
		check_ready();
	  	//check status register 
	local_irq_restore(flags);
	  	ecc_count=nand_check_eccStatus();


		if((return_value!=-1) && (ecc_count != -1))
		{
			return_value= (ecc_count>return_value)?ecc_count:return_value;
		}else{
			return_value=-1;
		}


			//copy data
		memcpy(chunk_buf+(buf_pos*512), data_area, 512);	  
		//copy oob
		memcpy(spare->u8_oob+(buf_pos*6), oob_area, 6);
			

		flash_addr_t += 528;//move next flash addr add (512+6+10)bytes
		dma_counter--;
		buf_pos++; 
			
	}


	if(block>0){
		//change bbi,data
		temp_buf=chunk_buf[nandflash_info.bbi_dma_offset];		
  		chunk_buf[nandflash_info.bbi_dma_offset] = spare->u8_oob[nandflash_info.bbi_swap_offset];
		spare->u8_oob[nandflash_info.bbi_swap_offset]=temp_buf;
  	}
//eric mark, not need flush cache
//	dma_cache_wback_inv((u32_t *)chunk_buf,2048);	
//	dma_cache_wback_inv((u32_t *)spare->u8_oob,64);


#if 0	 //only for test chip
	  if(return_value==-1){ //ecc fail ,test chip must check data is all 0xff
			return_value=0; 		
			for(i=0; i<(24); i++){
				if( spare->u8_oob[i] != 0xff){
					printk("ecc fail in _nand_read_chunk\n");
		//		  	if(data_area0!=NULL)
			//			kfree(data_area0);
					return_value= -1;
				  }
			}
	  }
#endif  

//	if(data_area0!=NULL)
	//	kfree(data_area0);
  
  return return_value;


}


/**	
**  _nand_read_bbi()
**	descriptions: read flash bbi byte and return it
**	parameters:  byte: bbi offset byte (should be nandflash_info.bbi_offset) , chunk id: chunk number
**	return:  1 is badblock,  0 is ok block.
**  Note: only for page size 2k , 
**/
u8_t
_nand_read_bbi(u32_t bytes, u32_t chunk_id ) {
	int real_page;
	unsigned int flash_addr1, flash_addr2;
	unsigned int data_out;
	real_page = chunk_id*nandflash_info.page_per_chunk;  //get real_page number

	rtk_writel(0xc00fffff, NACR);
	


	if(nandflash_info.page_size==512){
		flash_addr1 = (real_page << 8) & 0xffffff;
		flash_addr2 = (real_page >> 16) & 0xffffff;

	}else{
		flash_addr1 =  ((real_page & 0xff) << 16) | bytes;
		flash_addr2 = (real_page >> 8) & 0xffffff;
	}


	

	/* Command cycle 1*/
	if(nandflash_info.page_size!=512){	  
	rtk_writel((CECS0|CMD_PG_READ_C1), NACMR);

	}else{
		/*bbi should in spare-area (oob) 512+16 */
		bytes-=512; 		
		flash_addr1|=bytes; 
		/* Command cycle 1*/
		rtk_writel((CECS0|CMD_PG_READ_512_R3), NACMR); // CMD 50h	
	}
	check_ready();
	

	

	//use pio read.
	//write address to flash control	
	switch(nandflash_info.addr_cycles){
		case 3:
			rtk_writel( (AD2EN|AD1EN|AD0EN|flash_addr1), NAADR);			
			break;
		case 4:
			/* set address 1 */
	rtk_writel( (enNextAD|AD2EN|AD1EN|AD0EN|flash_addr1), NAADR);
			check_ready();				
		rtk_writel( (AD0EN|flash_addr2), NAADR);
			break;
		case 5:
			rtk_writel( (enNextAD|AD2EN|AD1EN|AD0EN|flash_addr1), NAADR);
			check_ready();				
			rtk_writel( (AD1EN|AD0EN|flash_addr2), NAADR);
			break;
	}
	check_ready();

	/* Command cycle 2*/
	if(nandflash_info.page_size!=512){ //512 page size not need 'end' command 
		/* Command cycle 2*/
	rtk_writel((CECS0|CMD_PG_READ_C2), NACMR);
	check_ready();
	}

	data_out = rtk_readl(NADR); //read 4 byte from NADR.
	
	if(((data_out >> 24) & 0xff) !=0xff){
		return 1;		//bad
	}
	
	return 0; //good

}


/*************************************************************************	
**  _nand_erase_block()
**	descriptions: erase nand flash block
**	parameters: chunk id  (must alignment of real_page/block)
			 	mtd/chipnr un-used
**	return: 0:succes,-1 is fail
**  note: this function not check bad block table.
    
*************************************************************************/

static int
_nand_erase_block(struct mtd_info *mtd, u16 chipnr,int chunk_id) {

	int addr_cycle[5],page_shift;
	int real_page = chunk_id*nandflash_info.page_per_chunk; //get real_page number

 	if (down_interruptible (&sem)) {
		printk("%s : user breaking\n",__FUNCTION__);
		return -ERESTARTSYS;
	}
 
	if ( real_page & (nandflash_info.num_page_per_block-1) ){
		printk("%s: (page)chunk_id %d is not block alignment !!\n", __FUNCTION__, real_page);
		up (&sem);
		return -1;
	}

//	rtk_writel( (rtk_readl(NACR) |ECC_enable), NACR); //erase not need set this reg
	rtk_writel((NWER|NRER|NDRS|NDWS), NASR);
	rtk_writel(0x0, NACMR);

	rtk_writel((CECS0|CMD_BLK_ERASE_C1),NACMR); //Command register , write erase command (1 cycle)
	check_ready();

	if(nandflash_info.addr_cycles!=3){
	for(page_shift=0; page_shift<3; page_shift++){
		addr_cycle[page_shift] = (real_page>>(8*page_shift)) & 0xff;
	}

		rtk_writel( ((~enNextAD) & (AD2EN|AD1EN|AD0EN|(addr_cycle[0]<<CE_ADDR0) |(addr_cycle[1]<<CE_ADDR1)|(addr_cycle[2]<<CE_ADDR2))),NAADR); //NAND Flash Address Register1
		
	}else{
		addr_cycle[0] = 0;
		for(page_shift=0; page_shift<4; page_shift++){
			addr_cycle[page_shift+1] = (real_page>>(8*page_shift)) & 0xff;
		}		
	rtk_writel( ((~enNextAD) & AD2EN|AD1EN|AD0EN|
				(addr_cycle[1]<<CE_ADDR0) |(addr_cycle[2]<<CE_ADDR1)|(addr_cycle[3]<<CE_ADDR2)),NAADR);
	}

			
	check_ready();

	rtk_writel((CECS0|CMD_BLK_ERASE_C2),NACMR); //write erase command cycle 2
	check_ready();

	rtk_writel((CECS0|CMD_BLK_ERASE_C3),NACMR);//read status
	check_ready();

	
	if ( rtk_readl(NADR) & 0x01 ){
		printk("[%s] erase block is not completed at block %d\n", __FUNCTION__, real_page/nandflash_info.num_page_per_block);
		up (&sem);			
		return -1;
	}else{
		up (&sem);		
		return 0;
	}
 
}


/*************************************************************************	
**  _nand_write_chunk()
**	descriptions: write data to nand flash
**	parameters: chunk id  (must alignment of real_page/block)			 
**	return: 0:succes,-1 is fail
**  note: this function not check bad block table.
    
*************************************************************************/


int
_nand_write_chunk(u8_t *chunk_buf, spare_u *spare, u32_t chunk_id) {
	int real_page = chunk_id*nandflash_info.page_per_chunk; //get real_page number
	int dma_counter = 4; //always 512byte * 4
	int dram_sa, oob_sa;
	int page_shift;
	int page_num[3];
	unsigned long flash_addr_t=0;
	unsigned long flash_addr2_t=0;
	int buf_pos=0;
	int return_value=0;
	int ecc_count=0;
	int i;
	unsigned long flags;
	int block = real_page/nandflash_info.num_page_per_block;

	u8_t *oob_area;
	u8_t *data_area, data_area0[512+16+CACHELINE_SIZE+CACHELINE_SIZE-4]; //data,oob point must 32 cache aligment

	memset(data_area0, 0xff, 512+16+CACHELINE_SIZE+CACHELINE_SIZE-4);


	data_area = (u8_t*) ((u32_t)(data_area0 + 12) & 0xFFFFFFF0);
	oob_area=(u8_t*) data_area+512;
	
	//printk("_nand_write_chunk : spare->u8_oob 0x%p\n",spare->u8_oob );

	if(block>0){
		//swap bbi and data
		spare->u8_oob[nandflash_info.bbi_swap_offset]=chunk_buf[nandflash_info.bbi_dma_offset];
 	 	chunk_buf[nandflash_info.bbi_dma_offset]=0xff;
  	}
//	dma_cache_wback_inv((u32_t *)chunk_buf,2048);

	//set DMA flash start address
	for(page_shift=0;page_shift<3; page_shift++) {
	    page_num[page_shift] = ((real_page>>(8*page_shift)) & 0xff);
		if(nandflash_info.page_size==2048){		
			flash_addr_t |= (page_num[page_shift] << (12+8*page_shift));
		}else if(nandflash_info.page_size==4096){
			flash_addr_t |= (page_num[page_shift] << (13+8*page_shift));		
		}else if(nandflash_info.page_size==512){/* 512 */
			flash_addr_t |= (page_num[page_shift] << (9+8*page_shift));
		}
	}

#if 1 //BCH
		// If the nand flash size is larger than 2G bytes, fill the other address for DMA
		flash_addr2_t= (real_page >> 20);
#endif
	
	rtk_writel(0xC00FFFFF, NACR);     //Enable ECC	
	rtk_writel(0x0000000F, NASR);	  //clear NAND flash status register
	
 	oob_sa =  ( (u32_t)(oob_area ) & (~M_mask));
	dram_sa = ((u32_t)data_area) & (~M_mask);

	/* dma move  4*528byte */

  	while(dma_counter>0){

		//copy oob to buffer
		memcpy(oob_area, spare->u8_oob+(buf_pos*6), 6);
		memcpy(data_area, chunk_buf+(buf_pos*512), 512);
		dma_cache_wback_inv((u32_t *)data_area,528);//512+16
		rtk_writel( rtk_readl(NACR) & (~RBO) & (~WBO) , NACR);
		//write data/oob address
		rtk_writel(dram_sa, NADRSAR);
		rtk_writel( oob_sa, NADTSAR);
		rtk_writel( flash_addr_t, NADFSAR);
#if 1//BCH		
		rtk_writel( flash_addr2_t, NADFSAR2);		
#endif
		//dma write cmd
	local_irq_save(flags);			
		rtk_writel( (DESC0|DMAWE|LBC_64 & (~TAG_DIS)),NADCRR);

		check_ready();
			//check status register 
		local_irq_restore(flags);
#if 0 //eric not need check. for write always ok...
		ecc_count=nand_check_eccStatus();
	  //Record ecc counter ,return the max number
		if((return_value!=-1) && (ecc_count != -1))
		{
			return_value= (ecc_count>return_value)?ecc_count:return_value;
		}else{
			return_value=-1;
			//printk("\r\n _nand_write_chunk return -1\n");
		}
#endif		
		rtk_writel(0xF, NASR);	


		flash_addr_t += (528); //512+16 one unit

		dma_counter--;
		buf_pos++;
	}
	
	if(block>0){
		//swap bbi and data again. (recovery data)
		chunk_buf[nandflash_info.bbi_dma_offset]= spare->u8_oob[nandflash_info.bbi_swap_offset];
		//eric mark not need
	//	dma_cache_wback_inv((u32_t *)chunk_buf,2048);
	}
	

	return return_value;
}




/**
**  create_bbt()
**	descriptions: create bbt table 
**	parameters:  dma_buf (temp buffer)
**	return: 
**  note: pio write is by pass hardware ecc.
**/


inline static u8_t*
pbldr_spare_bbi(spare_u *spare) {
	return &(spare->u8_oob[nandflash_info.bbi_swap_offset]);
}

#ifndef CONFIG_USE_PRELOADER_PARAMETERS

void
create_bbt(nand_dma_buf_t *dma_buf,u32_t *bbt_table) {
    u32_t b, ecc;
    u8_t bbi;

	dbg_printf("create bbt , for loop %d num_chunk_per_block %d\n",nandflash_info.num_block,num_chunk_per_block);
	
	memset(bbt_table, 0x0, NUM_WORD(MAX_BLOCKS));
	
    for (b=1;b<nandflash_info.num_block;++b) {
		
		bbi=_nand_read_bbi(nandflash_info.bbi_raw_offset,b*num_chunk_per_block);
        if (bbi==0) continue;
		dbg_printf("1 bad block[%d], page %d , bbi %x\n",b,b*num_chunk_per_block,bbi);
		if ((ecc=parameters._nand_read_chunk(dma_buf->chunk_buf, &dma_buf->spare, b*num_chunk_per_block))<0) {
		
			
			dbg_printf("read fail, set block %d is bad \n",b);
            _set_flags(bbt_table, b);
            continue;
        }
        u8_t oob_bbi=*pbldr_spare_bbi(&(dma_buf->spare));
		dbg_printf("oob_bbi is %x\n",oob_bbi);
        if (oob_bbi==0xFF) continue;
        if ((oob_bbi==0x00) && (dma_buf->spare.oob.signature==OOB_SIG_RESERVED)) continue;
 
        // mark bad block
        dbg_printf("2 bad block %d b is bad \n",b);
        _set_flags(bbt_table, b);
    }
//    parameters.bbt_valid=1;
	dbg_printf("finish create_bbt\n");
	
}

int nand_default(){

	int nand_strap_pin;
	
	//read from strap pin
	nand_strap_pin= rtk_readl(NACFR);

//READ page size strap ping
 switch( (nand_strap_pin & 0xC0000000) >>30){
 		case 0:
 					nandflash_info.page_size = 512;
 					break;
 		case 1:
 					nandflash_info.page_size = 2048;
 					break;
 		case 2:
 			 	  nandflash_info.page_size = 4096;
 					break;		
  	case 3:
 			 	  nandflash_info.page_size = 8192;
 					break;			 	
	}
	//read address cycle pin
	nandflash_info.addr_cycles = ((nand_strap_pin & 0x30000000) >>28)+3;  //3,4,5 address cycle	


	/*this hard-copy ,page 2k , block number 1024, total 128MB */
	
	nandflash_info.num_block  = 1024;
	//page number of one block
	nandflash_info.num_page_per_block = 64;
	nandflash_info.page_per_chunk=1;
	nandflash_info.bbi_dma_offset=2000; 
	//for 2048+64 pagesize, if spare_area byte[0] is bbi, in dma data_buffer is byte 2000, because must ignore 16byte oob *3, 
	// oob buffer not inside in data buffer.
	nandflash_info.bbi_raw_offset=2048; //bbi in nand flash raw address, for pio read 
	
	nandflash_info.bbi_swap_offset=23; // 512+16 nand control read, use 4th 512's oob byte[5] for data swap
	                               // after dma move to oob buffer, data byte is in oob_buf[23]  (6+6+6+6) (no ecc byte)


}

#endif


int
_nand_init(){

	/* initialize hardware */
 
	rtk_writel(0x0000000F, NASR);     //clear NAND flash status register
	rtk_writel( (rtk_readl(NACR) |ECC_enable|RBO|WBO), NACR); //Enable ECC	

#if 1

//printk("parameters at 0x%p\n",parameters);
//printk("parameters.read at 0x%p\n",parameters._nand_read_chunk);
//printk("parameters.write at 0x%p\n",parameters._nand_write_chunk);
//printk("parameters.bbt at 0x%p\n",parameters.bbt);
//printk("uboot- read nand flash info from SRAM\n");

printk("flash_info list\n");
printk("flash_info.num_block : %d\n",nandflash_info.num_block);
printk("flash_info.num_page_per_block : %d\n",nandflash_info.num_page_per_block);
printk("flash_info.page_per_chunk : %d\n",nandflash_info.page_per_chunk);
printk("flash_info.bbi_dma_offset : %d\n",nandflash_info.bbi_dma_offset);
printk("flash_info.bbi_raw_offset : %d\n",nandflash_info.bbi_raw_offset);
printk("flash_info.bbi_swap_offset : %d\n",nandflash_info.bbi_swap_offset);
printk("flash_info.page_size : %d\n",nandflash_info.page_size);
printk("flash_info.addr_cycles : %d\n",nandflash_info.addr_cycles);
printk("pblr_start_block : %d\n",nandflash_info.pblr_start_block);
printk("num_pblr_block : %d\n",nandflash_info.num_pblr_block);
#ifdef CONFIG_USE_PRELOADER_PARAMETERS
//	printk("parameters.curr_ver is %x\n",preloader_parameters.curr_ver);
//	printk("parameters.plr_num_chunk is %d\n",preloader_parameters.plr_num_chunk);
//	printk("parameters.blr_num_chunk is %d\n",preloader_parameters.blr_num_chunk);
//	printk("parameters.end_pblr_block is %d\n",preloader_parameters.end_pblr_block);
#endif

#endif

	chunk_size=nandflash_info.page_size*nandflash_info.page_per_chunk;
	num_chunk_per_block=flash_info_num_chunk_per_block;
	printk("chunk size is %d\n",chunk_size);
	printk("flash_info_num_chunk_per_block is %d\n",flash_info_num_chunk_per_block);
	
	return 0;
}


void __exit rtk_nand_exit (void)
{
	if (rtk_mtd){
		del_mtd_partitions (rtk_mtd);
		struct nand_chip *this = (struct nand_chip *)rtk_mtd->priv;
		if (this->g_databuf)
			kfree(this->g_databuf);
		if(this->g_oobbuf)
			kfree(this->g_oobbuf);
		kfree(rtk_mtd);
	}
	remove_proc_entry("nandinfo", NULL);
}

module_init(rtk_nand_init);
module_exit(rtk_nand_exit);
MODULE_AUTHOR(" Eric<ericchung@realtek.com>");
MODULE_DESCRIPTION("Realtek Luna NAND Flash Controller Driver");

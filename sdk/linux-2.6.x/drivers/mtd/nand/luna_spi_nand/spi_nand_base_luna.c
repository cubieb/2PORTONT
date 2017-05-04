/******************************************************************************
 * $Id: nand_base_sd5.c,v 1.6 2012/04/02 06:47:32 ccwei0908 Exp $
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
#include <linux/mtd/mtd.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <mtd/mtd-abi.h>
#include <asm/page.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
//#include "bspchip.h"
#include <soc/kernel_soc.h> //must include soc.h
#include "rtk_spi_nand.h"

#include "spi_nand/spi_nand_struct.h"
#include "spin_kernel_util.h"
static struct spi_nand_flash_info_s spi_nand_flash_info;
struct spi_nand_flash_info_s *_spi_nand_info = &spi_nand_flash_info;

#define USE_BBT_SKIP 1

/* NOTE: Need to be changed if use a flash with more block */
#define MAX_BLOCKS (1024<<3)
static u32_t _bbt_table[NUM_WORD(MAX_BLOCKS)];

#define NAND_LOADER_CHUNK	6	//MAX NAND ECC SKIP PAGE

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

#if 0
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
#endif
#define check_block_align(mtd, addr)					\
do {									\
	if (addr & (mtd->erasesize - 1)) {				\
		printk (				\
			"%s: attempt access non-block-aligned data\n",	\
			__FUNCTION__);					\
		return -EINVAL;						\
	}								\
} while (0)
#if 0
#define check_len_align(mtd,len)					\
do {									\
	if (len & (512 - 1)) {          	 			\
		printk (				\
               	      "%s: attempt access non-512bytes-aligned mem\n",	\
			__FUNCTION__);					\
		return -EINVAL;						\
	}								\
} while (0)

#endif


static struct nand_ecclayout nand_bch_oob_64 = {
	.eccbytes = 41, //ecc 40byte, + 1 bbi
	.eccpos = {
			24,25, 26, 27, 28, 29, 30, 31, 32,
			33, 32, 34, 34, 36, 37, 38, 39,
			40, 41, 42, 43, 44, 45, 46, 47,
			48, 49, 50, 51, 52, 53, 54, 55,
			56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {
		{.offset = 0,
		 .length = 23}}
};


extern int rtk_PIO_write (int page_id, int offset, int length, unsigned char *buffer);
extern int rtk_pio_read (struct mtd_info *mtd, u16 chipnr, int page_id, int len, u_char *oob_buf);

/* NAND low-level MTD interface functions */
static int nand_read (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf);
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf, 
			u_char *oob_buf, struct mtd_oob_ops *ops);

static int nand_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);

static int nand_read_oob_ext (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
static int nand_read_oob_auto (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);

static int nand_write (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf);
static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char * buf, 
			const u_char *oob_buf, struct mtd_oob_ops *ops);

static int nand_write_oob (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);

static int nand_erase (struct mtd_info *mtd, struct erase_info *instr);
static void nand_sync (struct mtd_info *mtd);


/* YAFFS2 */
static int nand_block_isbad (struct mtd_info *mtd, loff_t ofs);
static int nand_block_markbad (struct mtd_info *mtd, loff_t ofs);

/* Global Variables */
static int nand_pio_write_data_oob  (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);
static int nand_pio_write_data  (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);


static int nand_read_ecc_subpage(struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel);

static int nand_write_oob_auto  (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);

static int page_size, oob_size, ppb, isLastPage;
static int chunk_size; /*luna nand driver use chunk size */
static u64 nand_offset=0xffffffff;

//static void check_ready (void);

int spin_detect (void);

//===========================================================================

/* for debug usage */
int dump_mem (unsigned int addr, int len)
{
	unsigned char	*raw,ch;
	int	row,col,rowsz; 

	raw = (unsigned char *)addr;
	if (len == 0) {
 		rowsz = 20;
	}
	else {
		rowsz = (len + 15)/16;
	}


	for (row=0;row<rowsz;row++)
	{ 	
	 	// Address
 		printk("0x%08X: ",(addr + row * 16));
 		
	 	// Show HEX
 		for (col=0;col<8;col++) {
 			printk("%02X ",raw[col]);
 		}
		//printf("- ");
 		for (col=8;col<16;col++) {
 			printk("%02X ",raw[col]);
 		}

 		// Show ASCII
	 	for (col=0;col<16;col++) {
 			if ((raw[col] < 0x20) || (raw[col] > 0x7e)) {
 				ch = '.';
 			}
 			else {
 				if ((raw[col] == 0x25) || (raw[col] == 0x5c))
 					ch = '.';
 				else
 					ch = raw[col];
 			}
 			printk("%c",ch);
 		}

 		raw += 16;

 		printk("\n\r");
	
	}	// end of for
	return 1;	
}



/**	
**  nand_block_isbad()
**	descriptions: check block is bad or good
**	parameters: 
**	return: 1: bad, 0:good
**  note:  
**/
static int nand_block_isbad (struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int chunk_id, phy_block, logical_block;
	int j;

	chunk_id = ((int) ofs) >> this->page_shift;
	phy_block = chunk_id/ppb;

	/*return bad block table */

#ifdef USE_BBT_SKIP
	//if use logical block ,always return good

	logical_block = phy_block + this->bbt[phy_block];
	if (this->bbt[phy_block] >= 0xff)
		return 1;
	else
		return 0;
#else
	
	/* read bad table array, return 1 is bad , return 0 is good */
	j = _get_flags (_bbt_table, phy_block);
	return j;
#endif

}

/**	
**  nand_block_markbad()
**	descriptions: mark block is bad
**	parameters: 
**	return: 1: Fail, 0:success
**  note:  
**/
static int nand_block_markbad (struct mtd_info *mtd, loff_t ofs)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int chunk_id,block;
	int j;
#ifndef USE_BBT_SKIP	
	u8 nand_buffer[2048+64];
#endif	
	chunk_id = ((int) ofs) >> this->page_shift;

	block = chunk_id/ppb;

#ifdef USE_BBT_SKIP
	//if use logical block bbt, always return fail
	return 1;
#else

	/* set bad table array*/
	printk("Application set block %d is bad \n");
	_set_flags (_bbt_table, block);		
	memset (nand_buffer, 0x0, sizeof(nand_buffer));
	//pio write to chunk
	rtk_PIO_write(chunk_id,0,chunk_size+oob_size,&nand_buffer[0]);

	return 0;
#endif

}

static int nand_read_oob (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{		
	int ret;
	struct nand_chip *this = mtd->priv;

	
	if(ops->mode == MTD_OOB_AUTO){
		//case 1,  ops->ooblen = 16,  ops->len = 16 , ops.datbuf is null  (yaffs2 only read oob 16 byte )
		//printk("%s %s(%d):%d,%d\n",__FILE__,__FUNCTION__,__LINE__,ops->len,ops->ooblen);		
		//printk("MTD_OOB_AUTO :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n",ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );	

		if((ops->len==ops->ooblen) &&(ops->datbuf==NULL) ){
				ret = nand_read_oob_ext(mtd, from,ops);	
				return ret;
		}

		//case 2 , check ooblen

		if((ops->ooblen!=0)&&(ops->len!=0) &&(ops->datbuf!=NULL)&&(ops->oobbuf!=NULL)){
			ret = nand_read_ecc(mtd, from, ops->len, &ops->retlen,ops->datbuf, ops->oobbuf, ops);
			return ret;
		}else{
			ret = nand_read_oob_auto(mtd, from,ops);
			 return ret;
		}
	}

	if(ops->mode == MTD_OOB_PLACE){		
		ret = nand_read_oob_auto(mtd, from,ops);
		return ret;
	}


	if((ops->len==ops->ooblen)){// read oob 		
		ret = nand_read_oob_ext(mtd, from,ops);	
		return ret;
	}else{		
		ret = nand_read_ecc(mtd, from, ops->len, &ops->retlen,ops->datbuf, ops->oobbuf, NULL);
		return ret;
	}
}           

/* read oob only  */
static int nand_read_oob_auto (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int oob_len = 0, thislen;
	int rc=0;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap;
	//__u8 oob_area[64];
	__u8 *oob_area, oob_area0[64+16];
	u_char *oob_buf=ops->oobbuf;
	//dbg_printf("%s:%s(%d)\n",__FILE__,__FUNCTION__,__LINE__);

	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);

	if ((from + ops->ooblen) > mtd->size) {
		printk ("nand_read_oob: Attempt read beyond end of device\n");
		ops->oobretlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, from)) {
		printk (KERN_NOTICE "nand_read_oob: Attempt to read not page aligned data. %llx\n", from);
		dump_stack();
		return -EINVAL;
	}
	//printk("nand_read_oob_auto : from=%llxh\n",from);
	//printf("nand_read_oob_auto ops.ooblen  %d\n",ops->ooblen);
	//printk("nand_read_oob_auto :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n"	,ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );

	realchunk = (int)(from >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(from >> this->chip_shift);
	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1);
	phy_block = chunk_id/ppb;
	logical_block=phy_block+this->bbt[phy_block];

	//printk("[%s][%d]phy_block %d logical_block %d \n",__FUNCTION__,__LINE__,phy_block,logical_block);
	if(this->bbt[phy_block]>=0xff){
		printk ("nand_read_oob_ext: logical_block beyond end of device\n");
		ops->retlen = 0;
		return -EINVAL;
	}

	this->select_chip(mtd, chipnr);
	
	if ( &(ops->oobretlen)) 
		ops->oobretlen = 0;
	
	//thislen = oob_size;
	thislen=mtd->oobavail; //2k chunk should 23 byte for oob free

	while(oob_len < ops->ooblen) {

		if (thislen > (ops->ooblen - oob_len)) {
				thislen = (ops->ooblen - oob_len);
		}

		chunk_id = logical_block*ppb + chunk_offset;
		memset(this->g_databuf, 0xff, chunk_size);
		memset(this->g_oobbuf, 0xff, oob_size);
		//rc=parameters._nand_read_chunk(this->g_databuf,this->g_oobbuf,chunk_id);
		
		//rc = this->read_oob (mtd, this->active_chip, chunk_id, thislen, (ops->oobbuf)+oob_len );
		rc = this->read_oob (mtd, this->active_chip, chunk_id, thislen+2, oob_area );
		memcpy((ops->oobbuf)+oob_len, oob_area+2, thislen);

		
		if(rc!=0){
			if (rc == -1){
				printk ("%s: read_oob: Un-correctable HW ECC\n", __FUNCTION__);
				printk("%s: Un-correctable HW ECC Error at page=%d\n",__FUNCTION__, chunk_id);		    
	
				//ericchung: fix when read page ecc fail, still need response data to uplayer application.
				//return -1;
				mtd->ecc_stats.failed++;
				rc=-EBADMSG;

			}else if(rc== -ERESTARTSYS){
				printk ("%s: rtk_read_oob: semphore failed\n", __FUNCTION__);
				return -1;
			}else{
				if(rc < 0){ // special ecc error like that it is reading a erased page
					; // do nothing
				}else{
					printk("ecc fail count %d at page=%d ,but HW ECC can fix it\n",rc,chunk_id);
				}
				/*note: if ecc count > 4 bit (total hw can fix 6bit),suggest backup this block */
				rc = -EUCLEAN;
			}
		}	

		
		oob_len += thislen;
		old_chunk++;
		chunk_offset = old_chunk & (ppb-1);
		if ( oob_len<(ops->ooblen) && !(old_chunk & this->pagemask)) {
			old_chunk &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		phy_block = old_chunk/ppb;		
		logical_block=phy_block+this->bbt[phy_block];


		if(this->bbt[phy_block]>=0xff){
			printk ("nand_read_oob_ext: logical_block beyond end of device\n");
			ops->oobretlen = oob_len;
			return -EINVAL;
		}

	}

	if ( &(ops->oobretlen) ){
		if ( oob_len == ops->ooblen )
			ops->oobretlen = oob_len;
		else{
			printk("[%s] error: oob_len %d != len %d\n", __FUNCTION__, oob_len, ops->ooblen);
			return -1;
		}	
	}
	return rc;
}

static int nand_read_oob_ext (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int oob_len = 0, thislen;
	int rc=0;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap;
	//__u8 oob_area[64];
	__u8 *oob_area, oob_area0[64+16];
	u_char *oob_buf=ops->oobbuf;
	//	dbg_printf("%s:%s(%d)\n",__FILE__,__FUNCTION__,__LINE__);

	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);

	if ((from + ops->len) > mtd->size) {
		printk ("%s: Attempt read beyond end of device\n", __FUNCTION__);
		ops->retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, from)) {
		printk (KERN_NOTICE "%s: Attempt to read not page aligned data. %llx\n",__FUNCTION__,  from);
		dump_stack();
		return -EINVAL;
	}
	//	dbg_printf("%s : from=%llxh\n",__FUNCTION__, from);
	//	dbg_printf("%s :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n",__FUNCTION__	,ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );

	realchunk = (int)(from >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(from >> this->chip_shift);
	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1);
	phy_block = chunk_id/ppb;
	logical_block=phy_block+this->bbt[phy_block];

//	dbg_printf("[%s][%d]phy_block %d logical_block %d \n",__FUNCTION__,__LINE__,phy_block,logical_block);
	if(this->bbt[phy_block]>=0xff){
		printk ("nand_read_oob_ext: logical_block beyond end of device\n");
		ops->retlen = 0;
		return -EINVAL;
	}
		
	this->select_chip(mtd, chipnr);
	
	if ( &(ops->retlen)) 
		ops->retlen = 0;
	
	thislen = oob_size;

	while(oob_len < ops->len) {

		if (thislen > (ops->len - oob_len)) 
			thislen = (ops->len - oob_len);

		chunk_id = logical_block*ppb + chunk_offset;
		
		//printk("[%s, line %d] chunk_id = %d, (ops->oobbuf)+oob_len = 0x%p oob_len is %d\n",__FUNCTION__,__LINE__, chunk_id, ((ops->oobbuf)+oob_len),oob_len);
		//rc = this->read_oob (mtd, this->active_chip, chunk_id, thislen, (ops->oobbuf)+oob_len );
		rc = this->read_oob (mtd, this->active_chip, chunk_id, thislen, oob_area );
			
		memcpy((ops->oobbuf)+oob_len, oob_area+2, thislen);


		
		
		if(rc!=0){
			if (rc == -1){
				printk ("%s: read_oob: Un-correctable HW ECC\n", __FUNCTION__);
				printk("rtk_read_oob: Un-correctable HW ECC Error at page=%d\n", chunk_id);		    
//				return -1;
				//ericchung: fix when read page ecc fail, still need response data to uplayer application.
				//return -1;
				mtd->ecc_stats.failed++;
				rc=-EBADMSG;

			}else if(rc== -ERESTARTSYS){
				printk ("%s: rtk_read_oob: semphore failed\n", __FUNCTION__);
				return -1;
			}else{
				if(rc < 0){ // special ecc error like that it is reading a erased page
					; // do nothing
				}else{
					printk("ecc fail count %d at page=%d ,but HW ECC can fix it\n",rc,chunk_id);
				}
				/*note: if ecc count > 4 bit (total hw can fix 6bit),suggest backup this block */
				rc = -EUCLEAN;
			}
		}		
		oob_len += thislen;
		old_chunk++;
		chunk_offset = old_chunk & (ppb-1);
		if ( oob_len<(ops->len) && !(old_chunk & this->pagemask)) {
			old_chunk &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		phy_block = old_chunk/ppb;		
		logical_block=phy_block+this->bbt[phy_block];
//		printk("[%s][%d]phy_block %d logical_block %d \n",__FILE__,__LINE__,phy_block,logical_block);
//		printk("[%s][%d]phy_block %d logical_block %d \n",__FUNCTION__,__LINE__,phy_block,logical_block);

		if(this->bbt[phy_block]>=0xff){
			printk ("nand_read_oob_ext: logical_block beyond end of device\n");
			ops->retlen = oob_len;
			return -EINVAL;
		}

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

static int nand_write_oob (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	int ret;
	 
	nand_offset=0xffffffff;


	if(ops->mode == MTD_OOB_RAW){
		//printk("%s %s(%d):%d,%d\n",__FILE__,__FUNCTION__,__LINE__,ops->len,ops->ooblen);		
		//printk("%s MTD_OOB_RAW :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n", __FUNCTION__  ,ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );	
		if(ops->oobbuf==NULL){
			return nand_pio_write_data(mtd,to,ops); // no oob data
		}else{
			return nand_pio_write_data_oob(mtd,to,ops);
		}
 		
	}else if (ops->mode == MTD_OOB_AUTO){
		//printk("%s %s(%d):%d,%d\n",__FILE__,__FUNCTION__,__LINE__,ops->len,ops->ooblen); 	
		//printk("%s MTD_OOB_AUTO :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n",__FUNCTION__ ,ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );			 
		//case 1 , for yaffs , mtd_oob_auto , read both data, oob
		if((ops->ooblen!=0)&&(ops->len!=0) &&(ops->datbuf!=NULL)&&(ops->oobbuf!=NULL)){	
			ret = nand_write_ecc (mtd, to, ops->len, &ops->retlen,ops->datbuf, ops->oobbuf, ops);
			
			return ret;
		}
		//case 2 , only write oob	
		if((ops->len==0)&&(ops->ooblen!=0)&&(ops->oobbuf!=NULL)){
			return nand_write_oob_auto(mtd,to,ops);
		}else{
			printk("NAND need check at nand_write_oob !!!! \n");
		}
			 
	
	}else if (ops->mode == MTD_OOB_PLACE){
		//printk("MTD_OOB_PLACE\n");
		if((ops->datbuf==NULL)&&(ops->ooblen!=0)&&(ops->oobbuf!=NULL)){
			 return nand_write_oob_auto(mtd,to,ops);
		}else{
			printk("NAND need check at nand_write_oob !!!! \n");
		}

	}
#if 0
	else{
		//not use for this case.
	return nand_write_ecc (mtd, to, ops->len, &ops->retlen,ops->datbuf, ops->oobbuf, NULL);
	}
#endif	

}

/**
**  nand_write_oob_auto()
**	descriptions: write oob only  , only one chunk oob
**	parameters:   ops must include oobdata
**	return: 
**  note: will read old data first and write old data + new oob.
**/
static int nand_write_oob_auto (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int data_len, oob_len;
	int rc;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap, err_chipnr = 0, err_chipnr_remap = 1;

	//dbg_printf("%s:%s(%d)\n",__FILE__,__FUNCTION__,__LINE__);

	if ((to + ops->ooblen) > mtd->size) {
		printk ("nand_write_oob_auot: Attempt write beyond end of device\n");
		ops->oobretlen = 0;
		return -EINVAL;
	}
	if (NOTALIGNED (mtd, to)) {
		printk (KERN_NOTICE "nand_write_oob_auot: Attempt to write not page aligned data. %llx\n", to);
		dump_stack();
		return -EINVAL;
}

	if(ops->ooblen>mtd->oobavail){
		printk (KERN_NOTICE "nand_write_oob_auot: ops->ooblen size %d > oob area %d\n", ops->ooblen,mtd->oobavail);
		dump_stack();
		return -EINVAL;

	}

	realchunk = (int)(to >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(to >> this->chip_shift);
	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1); //get page index on every one block
	phy_block = chunk_id/ppb;
	logical_block=phy_block+this->bbt[phy_block];

	//oob_len=mtd->oobavail;
	//printk("nand_write_oob_auot write oob size %d to chunk id %d\n",ops->ooblen,chunk_id);
	if(this->bbt[phy_block]>=0xff){
		printk ("nand_write_oob_auot: logical_block beyond end of device\n");
		ops->oobretlen = 0;
		return -EINVAL;
	}


	this->select_chip(mtd, chipnr);

	chunk_id = logical_block*ppb + chunk_offset; //get logial chunk id.

	//read chunk data buf
	memset(this->g_databuf, 0xff, chunk_size);
	memset(this->g_oobbuf, 0xff, oob_size);
	rc = this->read_ecc_page(mtd, 0, chunk_id, this->g_databuf, this->g_oobbuf);
	//rc=parameters._nand_read_chunk(this->g_databuf,this->g_oobbuf,chunk_id); 
	
	//append chunk data buf + oob data buf
	//memset(this->g_oobbuf, 0xff, oob_size);

	memcpy(this->g_oobbuf+2,ops->oobbuf,ops->ooblen);
	//write chunk with HW ecc
	rc = this->write_ecc_page(mtd, 0, chunk_id, this->g_databuf, this->g_oobbuf, 0);
	//rc=parameters._nand_write_chunk(this->g_databuf,this->g_oobbuf,chunk_id);

	if (rc !=0) {
		if (rc == -1){
			printk ("%s: Un-correctable HW ECC\n", __FUNCTION__);
			printk("nand_write_ecc: Un-correctable HW ECC Error at chunk_id=%d\n", chunk_id);
			return -1;
		}else if(rc==-ERESTARTSYS){
			printk ("%s: nand_write_ecc:  semphore failed\n", __FUNCTION__);
			return -1;
		}else {
			printk("ecc fail count %d at chunk_id=%d ,but HW ECC can fix it\n",rc,chunk_id);
			/*note: if ecc count > 4 bit (total hw can fix 6bit),suggest backup this block */
			rc = 0;
		}
	}

	ops->oobretlen =ops->ooblen;
	return 0;
}

/**
**  nand_pio_write_data()
**	descriptions: pio write raw data(data+oob+ecc) to nand flash
**	parameters:   oobdata (inculde data+oob+ecc) write size 2046+64
**	return: 
**  note: pio write is by pass hardware ecc. be careful!!
**/
static int nand_pio_write_data (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int oob_len = 0, thislen;
	int rc=0;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap;
		
	u_char *buf=ops->datbuf;

	//dbg_printf("%s:%s(%d)\n",__FILE__,__FUNCTION__,__LINE__);

	if ((to + ops->len) > mtd->size) {
		printk ("nand_pio_write: Attempt write beyond end of device\n");
		ops->retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, to)) {
		printk (KERN_NOTICE "nand_pio_write: Attempt to read not page aligned data. %llx\n", to);
		return -EINVAL;
	}

	//dbg_printf("a pio write NAND :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n"
	//		,ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );

	realchunk = (int)(to >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(to >> this->chip_shift);
	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1);
	phy_block = chunk_id/ppb;
	logical_block= phy_block+this->bbt[phy_block];
	if(this->bbt[phy_block]>=0xff){
			printk ("%s: Attempt read block beyond end of device,no block enough\n", __FUNCTION__);
			return -EINVAL;
	}
		
	this->select_chip(mtd, chipnr);
	
	if ( &(ops->retlen)) 
		ops->retlen = 0;

	thislen=0; //record data buf point


	while (thislen < ops->len) {
			 
		chunk_id = logical_block*ppb + chunk_offset;

		//dbg_printf("[%s, line %d] chunk_id = %d, databuf = 0x%p thislen =%d \n",__FUNCTION__,__LINE__, chunk_id, &buf[thislen],thislen);
		
		
		rtk_PIO_write(chunk_id,0,chunk_size+oob_size,&buf[thislen]);

		
		thislen += chunk_size+oob_size;	
		
		old_chunk++;
		chunk_offset = old_chunk & (ppb-1);
		if ( thislen<(ops->len) && !(old_chunk & this->pagemask)) {
			old_chunk &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		phy_block = old_chunk/ppb;
		logical_block= phy_block+this->bbt[phy_block];
		if(this->bbt[phy_block]>=0xff){
			printk ("nand_pio_write: Attempt read block beyond end of device,no block enough\n");
			return -EINVAL;
		}
	}
	return 0;
}

/**
**  nand_pio_write_data_oob()
**	descriptions: pio write raw data(data+oob+ecc) to nand flash
**	parameters:   ops must include databuf,oobbuf
**	return: 
**  note: pio write is by pass hardware ecc.
**/
static int nand_pio_write_data_oob (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int oob_len = 0, thislen;
	int rc=0;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap;
	//__u8 oob_area[64];
	u8 nand_buffer[2048+64];
		
	u_char *buf=ops->datbuf;
	u_char *oob_buf=ops->oobbuf;
	//dbg_printf("%s:%s(%d)\n",__FILE__,__FUNCTION__,__LINE__);

	if ((to + ops->len) > mtd->size) {
		printk ("nand_pio_write: Attempt write beyond end of device\n");
		ops->retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, to)) {
		printk (KERN_NOTICE "nand_pio_write: Attempt to read not page aligned data. %llx\n", to);
		return -EINVAL;
	}

//	dbg_printf("a pio write NAND :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n"
//			,ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );

	realchunk = (int)(to >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(to >> this->chip_shift);
	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1);
	phy_block = chunk_id/ppb;
	logical_block= phy_block+this->bbt[phy_block];
	if(this->bbt[phy_block]>=0xff){
			printk ("nand_pio_write: Attempt read block beyond end of device,no block enough\n");
			return -EINVAL;
	}

	
		
	this->select_chip(mtd, chipnr);
	
	if ( &(ops->retlen)) 
		ops->retlen = 0;

	thislen=0; //record data buf point
	oob_len=0; //record oob buf point

	while (thislen < ops->len) {
	
		chunk_id = logical_block*ppb + chunk_offset;

		//dbg_printf("[%s, line %d] chunk_id = %d, databuf = 0x%p thislen =%d \n",__FUNCTION__,__LINE__, chunk_id, &buf[thislen],thislen);
		//check_ready();
	//	_nand_pio_write(chunk_id,chunk_size+oob_size,&buf[thislen]);
		memset(nand_buffer, 0xff, sizeof(nand_buffer));

		//append data buffer+oobbuf as one chunk. 
		memcpy(nand_buffer, &buf[thislen], chunk_size);
		memcpy(&nand_buffer[thislen+chunk_size], &oob_buf[oob_len], oob_size);
	
		rtk_PIO_write(chunk_id,0,chunk_size+oob_size,&nand_buffer[0]);

		
		thislen += chunk_size;	
		oob_len+=oob_size;

		
		old_chunk++;
		chunk_offset = old_chunk & (ppb-1);
		if ( thislen<(ops->len) && !(old_chunk & this->pagemask)) {
			old_chunk &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		phy_block = old_chunk/ppb;
		logical_block= phy_block+this->bbt[phy_block];
		if(this->bbt[phy_block]>=0xff){
			printk ("nand_pio_write: Attempt read block beyond end of device,no block enough\n");
			return -EINVAL;
		}
	}
	return 0;
}


/**
**  nand_read()
**	descriptions: 
**	parameters:   
**	return: 
**  note:
**/
static int nand_read (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf)
{
	if ( NOTALIGNED(mtd, len)||NOTALIGNED (mtd, from)) 	
		return nand_read_ecc_subpage (mtd, from, len, retlen, buf, NULL, NULL);
	else
		return nand_read_ecc(mtd, from, len, retlen, buf, NULL, NULL);
}                          

/**
**  nand_read_ecc_subpage()
**	descriptions: if read address or size not alignment use nand_read_ecc_subpage
**	parameters:   
**	return: 
**  note:
**/
static int nand_read_ecc_subpage (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel)
{

	static u8 nand_buffer[2048+32];

	size_t data_len=len;
	int firstsize=0;
	int buffer_offset=0;
	size_t my_retlen;
	int ret;
	#define ADDR_MASK (2048 - 1)
	#define NANDSEG(x) (x & ~ADDR_MASK) //read from align address
	//dbg_printf("%s:%s(%d) from %llxh len %d\n",__FILE__,__FUNCTION__,__LINE__,from,len);

	*retlen=0;

		/* read first chunk */
	if (NANDSEG(from) != NANDSEG(nand_offset)) {	
		memset(nand_buffer,0xff,2048);
		ret = nand_read_ecc(mtd, NANDSEG(from), 2048, &my_retlen, nand_buffer, NULL, NULL);
		if (ret)
			return ret;
		nand_offset = NANDSEG(from);
	}

	firstsize= mtd->writesize - (from - nand_offset);

	if(len<=firstsize){
		/* first read is enough, copy first size data to buffer */
		memcpy(buf, &nand_buffer[ from - nand_offset ], len);		
		*retlen = len;
		return 0;

	}else{
		/*still need read next chunk */
		memcpy(&buf[buffer_offset], &nand_buffer[ from - nand_offset ], firstsize);
		buffer_offset+=firstsize;
		from+=firstsize;
		*retlen+=firstsize;
		data_len=data_len-firstsize;
		//printk("firstsize is %d\n",firstsize);
		//printf("first date_len is %d\n",data_len);
		while(data_len>0){
			ret = nand_read_ecc(mtd, NANDSEG(from), mtd->writesize, &my_retlen, nand_buffer, NULL, NULL);
		
			nand_offset = NANDSEG(from);
			
			if(data_len>mtd->writesize){
				/* copy one chunk to buf */
				memcpy(&buf[buffer_offset], &nand_buffer[ 0 ], mtd->writesize);
				buffer_offset+=mtd->writesize;
				from+=mtd->writesize;
				*retlen+=mtd->writesize;
				data_len=data_len-mtd->writesize;
				//printf("001 data_len is %d\n",data_len);
			}else{				
				/* copy last data to buf */
				memcpy(&buf[buffer_offset], &nand_buffer[ 0 ], data_len);
				*retlen+=data_len;
				//printf("002 data_len is %d\n",data_len);
				data_len=0;
			}
		}
	}

	if(*retlen!=len)
		printf("nand_read_ecc_subpage return size fail \n");
	return 0;
}

/**	
**  nand_read_ecc()
**	descriptions: read data/oob from nand flash.
**	parameters: 
**	return: 
**  note:  
**/
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int data_len, oob_len;
	int rc;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap;
	__u8 *oob_area, oob_area0[64+16];

	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);
	//dbg_printf("%s:%s(%d)\n",__FILE__,__FUNCTION__,__LINE__);
	//printk("[%s, line %d] from=%llxh, mtd->writesize=%d, len=%d\n"	,__FUNCTION__,__LINE__,from, mtd->writesize,len);

	if ((from + len) > mtd->size) {
		printk ("nand_read_ecc: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}
	
	if (NOTALIGNED (mtd, from)) {		
		printk("mtd,from aligned fail,[%s, line %d] from=%llxh, mtd->writesize=%d, len=%d\n"	,__FUNCTION__,__LINE__,from, mtd->writesize,len);
		printk (KERN_NOTICE "nand_read_ecc: Attempt to read not page aligned data\n");
		dump_stack();
		return -EINVAL;
	}
#if 0	
	if (NOTALIGNED(mtd, len)) {		
		printk("mtd,len aligned fail [%s, line %d] from=%llxh, mtd->writesize=%d, len=%d\n"	,__FUNCTION__,__LINE__,from, mtd->writesize,len);
		printk (KERN_NOTICE "nand_read_ecc: Attempt to read not page aligned data\n");
		dump_stack();
		return -EINVAL;
	}
#endif
	realchunk = (int)(from >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(from >> this->chip_shift);
	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1);
	phy_block = chunk_id/ppb;
	logical_block=phy_block+this->bbt[phy_block];
	
	//dbg_printf("[%s][%d]phy_block %d logical_block %d \n",__FUNCTION__,__LINE__,phy_block,logical_block);
  //dbg_printf("[%s][%d]phy_block %d logical_block %d \n",__FILE__,__LINE__,phy_block,logical_block);
	if(this->bbt[phy_block]>=0xff){
		printk ("nand_read_ecc: logical_block beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}
	
	//printk("%s(%d): phy_block %d logical_block %d page %d realpage %d\n",__FUNCTION__,__LINE__, phy_block,logical_block, page, realpage);
		
	this->select_chip(mtd, chipnr);
	
	if ( retlen )
		*retlen = 0;
	
	data_len = oob_len = 0;

	while (data_len < len) {
		chunk_id = logical_block*ppb + chunk_offset;  
		if(oob_buf!=NULL){
			rc = this->read_ecc_page(mtd, 0, chunk_id, &buf[data_len], this->g_oobbuf);
			//rc=_nand_read_chunk(&buf[data_len],this->g_oobbuf,chunk_id);	
			if(ops){
				memcpy(&oob_buf[oob_len],this->g_oobbuf+2,ops->ooblen);
				oob_len += ops->ooblen;
			}else{
				memcpy(&oob_buf[oob_len],this->g_oobbuf+2,mtd->oobavail);
				oob_len += mtd->oobavail;
			}	

		}else{

			rc=this->read_ecc_page(mtd, 0, chunk_id, &buf[data_len], oob_area);
			//rc=_nand_read_chunk(&buf[data_len],oob_area,chunk_id);
		}

		if (rc !=0) {
			if (rc == -1){
				
				//dbg_printf("[%s, line %d] from=%llxh, mtd->writesize=%d, len=%d\n"	,__FUNCTION__,__LINE__,from, mtd->writesize,len);
				//dbg_printf("[%s][%d]phy_block %d logical_block %d \n",__FUNCTION__,__LINE__,phy_block,logical_block);
					
				printk ("%s: Un-correctable HW ECC\n", __FUNCTION__);
				printk("rtk_read_ecc_page: Un-correctable HW ECC Error at chunk_id=%d\n", chunk_id);
				//ericchung: fix when read page ecc fail, still need response data to uplayer application.
				//return -1;
				mtd->ecc_stats.failed++;
				rc=-EBADMSG;				

			}else if(rc==-ERESTARTSYS){
				printk ("%s: read_ecc_page:  semphore failed\n", __FUNCTION__);
				return -1;
			}else {
				if(rc < 0){ // special ecc error like that it is reading a erased page
					; // do nothing
				}else{
					printk("ecc fail count %d at page=%d ,but HW ECC can fix it\n",rc,chunk_id);
				}
				/*note: if ecc count > 4 bit (total hw can fix 6bit),suggest backup this block */
				rc = -EUCLEAN;
			}
		}

		//data_len += page_size;
		data_len += chunk_size;
	
		
		old_chunk++;
		chunk_offset = old_chunk & (ppb-1);
		if ( data_len<len && !(old_chunk & this->pagemask)) {
			old_chunk &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		
		phy_block = old_chunk/ppb;
		logical_block=phy_block+this->bbt[phy_block];
//		printk("[%s][%d]phy_block %d logical_block %d \n",__FILE__,__LINE__,phy_block,logical_block);
//		printk("[%s][%d]phy_block %d logical_block %d \n",__FUNCTION__,__LINE__,phy_block,logical_block);

		if(this->bbt[phy_block]>=0xff){
			printk ("nand_read_ecc: logical_block beyond end of device\n");
			*retlen = data_len;
			return -EINVAL;
		}
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


/**
**  nand_write()
**	descriptions: write data to nand flash. (oob is null)
**	parameters: 
**	return: 
**  note:  
**/
static int nand_write (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char *buf)
{
	nand_offset=0xffffffff;
	return (nand_write_ecc (mtd, to, len, retlen, buf, NULL, NULL));
}

/**	
**  nand_write()
**	descriptions: write data to nand flash. (oob is null)
**	parameters: 
**	return: 
**  note:  
**/
static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, 
			const u_char * buf, const u_char *oob_buf, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int data_len, oob_len;
	int rc;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap, err_chipnr = 0, err_chipnr_remap = 1;
	__u8 _oobbuf[64];
	//__u8 oob_area[64];
	__u8 *oob_area, oob_area0[64+16];

	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);
	memset(oob_area, 0xff, 64);
	//dbg_printf("[%s, line %d] to=%llxh, mtd->writesize=%d, len=%d\n"	,__FUNCTION__,__LINE__,to, mtd->writesize,len);
	//printk("[%s, line %d] to=%llxh, mtd->writesize=%d, len=%d\n"	,__FUNCTION__,__LINE__,to, mtd->writesize,len);

	memset(this->g_databuf, 0xFF, chunk_size);			
	memset(this->g_oobbuf, 0xFF, oob_size);

	//dbg_printf("%s : to=%llxh\n",__FUNCTION__, to);

	if ((to + len) > mtd->size) {
		printk ("%s: Attempt write beyond end of device to=%llxh  len=%d\n",__FUNCTION__, to,len);
		*retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, to) || NOTALIGNED(mtd, len)) {
		printk("[%s, line %d] to=%llxh, mtd->writesize=%d, len=%d\n"	,__FUNCTION__,__LINE__,to, mtd->writesize,len);
		printk (KERN_NOTICE "%s: Attempt to write not page aligned data\n", __FUNCTION__);
		dump_stack();
		return -EINVAL;
	}

	realchunk = (int)(to >> this->page_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(to >> this->chip_shift);
	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1);
	phy_block = chunk_id/ppb;
	logical_block=phy_block+this->bbt[phy_block];
	//printk("[%s][%d]phy_block %d logical_block %d \n",__FILE__,__LINE__,phy_block,logical_block);
	//printk("[%s][%d]phy_block %d logical_block %d \n",__FUNCTION__,__LINE__,phy_block,logical_block);

	if(this->bbt[phy_block]>=0xff){
		printk ("%s: logical_block beyond end of device\n", __FUNCTION__);
		*retlen = 0;
		return -EINVAL;
	}


	this->select_chip(mtd, chipnr);
	
	if ( retlen )
		*retlen = 0;
	
	data_len = oob_len = 0;

	while ( data_len < len) {
		chunk_id = logical_block*ppb + chunk_offset;
		/*write nand flash check */
		/*1. check write block to block 0 ,nand contoller need protect block 0*/
		/*2. if all data is 0xff, not need write */

		if(chunk_id > NAND_LOADER_CHUNK){			
			if(oob_buf!=NULL){
				if((memcmp(this->g_databuf,&buf[data_len],chunk_size)!=0)||(memcmp(oob_area,&oob_buf[oob_len],oob_size)!=0)){
					memset(_oobbuf, 0xFF, sizeof(_oobbuf));
					if(NULL != ops){
						memcpy(&_oobbuf[2], &oob_buf[oob_len], ops->ooblen);
					}
					else{
						memcpy(&_oobbuf[2], &oob_buf[oob_len], oob_size);
					}
					this->write_ecc_page(mtd, 0, chunk_id, &buf[data_len], _oobbuf, 0);

				}else{
					rc=0;
				}
	
			}else{
				if(memcmp(this->g_databuf,&buf[data_len],chunk_size)!=0){	
					this->write_ecc_page(mtd, 0, chunk_id,&buf[data_len], oob_area, 0);
				}else{
					rc=0;
				}			
			}
			
		}else{
			/*write to loader block 0 need check write data*/
			if(memcmp("SKIP",&buf[data_len],4)==0){
						rtk_PIO_write(chunk_id,0,4,(unsigned char *) "SKIP");
			}else{
				if(memcmp(this->g_databuf,&buf[data_len],chunk_size)!=0){
					this->write_ecc_page(mtd, 0, chunk_id, &buf[data_len], oob_area, 0);
					//rc=parameters._nand_write_chunk(&buf[data_len],oob_area,chunk_id);
				}else{
					rc=0;
				}				
			}
		}
	
		if (rc !=0) {
			if (rc == -1){
				printk ("%s: Un-correctable HW ECC\n", __FUNCTION__);
				printk("nand_write_ecc: Un-correctable HW ECC Error at chunk_id=%d\n", chunk_id);
				return -1;
			}else if(rc==-ERESTARTSYS){
				printk ("%s: nand_write_ecc:  semphore failed\n", __FUNCTION__);
				return -1;
			}else {
				if(rc < 0){ // special ecc error like that it is reading a erased page
					; // do nothing
				}else{
					printk("ecc fail count %d at chunk_id=%d ,but HW ECC can fix it\n",rc,chunk_id);
				}
				/*note: if ecc count > 4 bit (total hw can fix 6bit),suggest backup this block */
				rc = 0;
			}
		}

		
		//data_len += page_size;
		data_len += chunk_size;
		oob_len += oob_size;
		
		old_chunk++;
		chunk_offset = old_chunk & (ppb-1);
		if ( data_len<len && !(old_chunk & this->pagemask)) {
			old_chunk &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}
		phy_block = old_chunk/ppb;
		logical_block=phy_block+this->bbt[phy_block];
		//printk("[%s][%d]phy_block %d logical_block %d \n",__FILE__,__LINE__,phy_block,logical_block);
		//printk("[%s][%d]phy_block %d logical_block %d \n",__FUNCTION__,__LINE__,phy_block,logical_block);

		
		if(this->bbt[phy_block]>=0xff){
			printk ("nand_write_ecc: logical_block beyond end of device\n");
			*retlen = data_len;
			return -EINVAL;
		}		
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

static int nand_erase_nand (struct mtd_info *mtd, struct erase_info *instr, int allowbbt)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	u_int32_t addr = instr->addr;	//erase address
	u_int32_t len = instr->len;
	int chunk_id, chipnr;
	int i, old_chunk, phy_block,logical_block;
	int elen = 0;
	int rc = 0;
	int temp_chunk, chipnr_remap;

	check_end (mtd, addr, len);
	check_block_align (mtd, addr);

	instr->fail_addr = 0xffffffff;

	temp_chunk = ((int) addr) >> this->page_shift;
	this->active_chip = chipnr = chipnr_remap = ((int) addr) >> this->chip_shift;
	old_chunk = chunk_id = temp_chunk & this->pagemask;
	phy_block = chunk_id/ppb;
	logical_block=phy_block+this->bbt[phy_block];
	//dbg_printf("[%s][%d]phy_block %d logical_block %d \n",__FUNCTION__,__LINE__,phy_block,logical_block);

	if(this->bbt[phy_block]>=0xff){
		printk ("nand_erase_nand: phy block %d logical_block %d beyond end of device\n",phy_block,logical_block);
		return -EINVAL;
	}	


//	dbg_printf("[%s] phy_block: %d, page: %d\n",__FUNCTION__,phy_block,chunk_id);
	this->select_chip(mtd, chipnr);

	instr->state = MTD_ERASING;
	while (elen < len) {
	
		chunk_id = logical_block*ppb;
//		printk ("%s: erase block %d at page =%d\n", __FUNCTION__,block, page);		
		rc = this->erase_block (mtd, this->active_chip, chunk_id);
			
		if (rc!=0) {
			printk ("%s: block erase failed at page address=0x%08x\n", __FUNCTION__, addr);
			instr->fail_addr = (chunk_id << this->page_shift);	
		    instr->state = MTD_ERASE_FAILED;

		}
		
		if ( chipnr != chipnr_remap )
			this->select_chip(mtd, chipnr);
			
		elen += mtd->erasesize;

		old_chunk += ppb;
		
		if ( elen<len && !(old_chunk & this->pagemask)) {
			old_chunk &= this->pagemask;
			chipnr++;
			this->active_chip = chipnr;
			this->select_chip(mtd, chipnr);
		}

		phy_block = old_chunk/ppb;
		logical_block=phy_block+this->bbt[phy_block];
//	printk("[%s][%d]phy_block %d logical_block %d \n",__FILE__,__LINE__,phy_block,logical_block);
//		printk("[%s][%d]phy_block %d logical_block %d \n",__FUNCTION__,__LINE__,phy_block,logical_block);


		if(this->bbt[phy_block]>=0xff){
			printk ("nand_erase_nand: phy block %d logical_block %d beyond end of device\n",phy_block,logical_block);
			return -EINVAL;
		}	

		
	}

	if(!rc){
	    instr->state = MTD_ERASE_DONE;
		mtd_erase_callback(instr);
	}

	return rc;
}

static int nand_erase (struct mtd_info *mtd, struct erase_info *instr)
{
	nand_offset=0xffffffff;
	return nand_erase_nand (mtd, instr, 0);
}


static void nand_select_chip (struct mtd_info *mtd, int chip)
{
#if 0
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
#endif
}

static inline int generic_ffs (int x)
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


/* inline function */
inline static void
_set_flags(u32_t *arr, u32_t i) {
	unsigned idx=i/(8*sizeof(u32_t));
	i &= (8*sizeof(u32_t))-1;
	arr[idx] |= 1UL << i;
}

inline static int
_get_flags(u32_t *arr, u32_t i) {
	unsigned idx=i/(8*sizeof(u32_t));
	i &= (8*sizeof(u32_t))-1;
	return (arr[idx] & (1UL << i)) != 0;
}


static int create_bbt(struct mtd_info *mtd, uint8_t *buf, struct nand_bbt_descr *bd, int chip)
{
	uint8_t bb_tag[4];
	u32_t *bbt_table = (u32_t*)buf;
	uint32_t bn;
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	
	for( bn = 1; bn < SNAF_NUM_OF_BLOCK(_spi_nand_info); bn++){
		rtk_pio_read(mtd, 0, bn*ppb, 4, bb_tag);
		if(0xFF == bb_tag[0]){
			continue;
		}
		
		_set_flags(bbt_table, bn);
	}
}

int create_logical_skip_bbt(struct mtd_info *mtd, u32_t *bbt_table)
{

	unsigned int good_num; 
	int j;
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int bbi_block;
	unsigned int skip_block;

	//u_char oob_buf[MAX_OOB_BUF_SIZE];
	
	//this->bbt default all 0.
	memset(this->bbt, '\0', this->block_num);
	//dbg_printf("create_logical_skip_bbt\n");
	//skip_block=0;
	
	skip_block = good_num = bbi_block = 1;
	for (;bbi_block<this->block_num;bbi_block++) {
		j=_get_flags(bbt_table, bbi_block);


		if(j==0){ //good block
				
			this->bbt[skip_block]=bbi_block-good_num;
			good_num++;	
			skip_block++;			
		}else{
			printk("detect block %d is bad \n",bbi_block);
		}
	}
	//dbg_printf("last skip_block %d\n",skip_block);
	printk("good block number %d\n", skip_block);
	for (skip_block;skip_block<this->block_num;skip_block++){
		this->bbt[skip_block]=0xff;		
	}	

	//debug message 

 //for (skip_block=0;skip_block<20;skip_block++){
 //	printf("this->bbt[%d]= %d\n",skip_block,this->bbt[skip_block]);
 //}
}

int display_bbt (struct mtd_info *mtd)
{
	unsigned int good_num=0; 
	int j;
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int bbi_block;
	unsigned int skip_block;

	printk("display bbt\n");
	
	for (bbi_block=0;this->block_num;bbi_block++) {
		j=_get_flags(_bbt_table, bbi_block);

		if(j==0){ //good block
			//nothing	
		}else{
			printk("detect block % is bad \n",bbi_block);
		}
	}	
}

/*************************************************************************
**  spi_nand_detect()
**	descriptions:
**	parameters: 
**	return:
**  note:
*************************************************************************/
extern const spi_nand_probe_t *LS_start_of_nand_spif_probe_func;
extern const spi_nand_probe_t *LS_end_of_nand_spif_probe_func;

int spi_nand_detect (void)
{
	const spi_nand_probe_t **pf = &LS_start_of_nand_spif_probe_func;
	void *flash_info = VZERO;


	while(pf != &LS_end_of_nand_spif_probe_func){
		flash_info = (void *)(*pf)();
		if(flash_info != VZERO){
			inline_memcpy(&spi_nand_flash_info, flash_info, sizeof(struct spi_nand_flash_info_s));
			return 1;
		}
		pf++;
	}

	return 0;
}

/*************************************************************************	
**  rtk_spi_nand_scan()
**	descriptions: read spi nand flash id , find flash spec, assign MTD callback function
**	parameters: 
**	return: 
**  note: eric: read nand flash spec from SRAM.
*************************************************************************/
int rtk_spi_nand_scan (struct mtd_info *mtd, int maxchips)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;

	unsigned int device_size = 0;
	unsigned int i;
	int rtk_lookup_table_flag = 0;
	unsigned int block_size;
	unsigned int num_chips = 1;
	unsigned int chip_size = 0;

	if (!this->select_chip) this->select_chip = nand_select_chip;

	this->active_chip = 0;
	this->select_chip (mtd, 0);

	mtd->name = "spinand";

	oob_size = 64; /* always use 64byte */

	if ((rtk_lookup_table_flag = spi_nand_detect ()) == 0) {
		printk ("Warning: Lookup Table do not have this nand flash, use Default\n");
		printk ("%s: Manufacture_id=0x%04x, Device id=0x%04x\n",
				mtd->name, this->maker_code, this->device_code);
		return -1;
	}

	// Fill info to ds.
	//oob_size = SNAF_OOB_SIZE(_spi_nand_info);
	chunk_size = SNAF_PAGE_SIZE(_spi_nand_info);
	page_size = chunk_size;
	block_size = page_size * SNAF_NUM_OF_PAGE_PER_BLK(_spi_nand_info);
	chip_size = block_size * SNAF_NUM_OF_BLOCK(_spi_nand_info);
	device_size = chip_size;


	this->maker_code = _spi_nand_info->man_id;
	this->device_code = _spi_nand_info->dev_id;
	this->page_shift = generic_ffs (chunk_size) - 1;
	this->phys_erase_shift = generic_ffs (block_size) - 1;
	this->oob_shift = generic_ffs (oob_size) - 1;
	ppb = this->ppb = block_size >> this->page_shift;
	//nandflash_info.num_page_per_block = ppb;

	this->page_num = chip_size >> this->page_shift;
	this->block_num = chip_size >> this->phys_erase_shift;
	this->device_size = device_size;
	this->chipsize = chip_size;
	this->chip_shift =  generic_ffs (this->chipsize) - 1;
	this->pagemask = (this->chipsize >> this->page_shift) - 1;
	this->numchips = num_chips;

	printk ("[%s, line %d] page_shift=%d, phy_erase_shift=%d, oob_shift=%d, chip_shift=%d\n"
		, __FUNCTION__, __LINE__, this->page_shift, this->phys_erase_shift, this->oob_shift, this->chip_shift);

	//printk ("%s(%d): pagesize=%x chunk_size=%x ,oob=%x mtd=%p(%p)\n", __FUNCTION__, __LINE__, nandflash_info.page_size, chunk_size, oob_size, mtd, &mtd->writesize);

	mtd->oobsize = this->oob_size = oob_size;
	mtd->writesize = chunk_size;
	mtd->erasesize = block_size;
	//mtd->writesize_shift = generic_ffs(page_size)-1;
	mtd->writesize_shift = generic_ffs (chunk_size) - 1;
	mtd->erasesize_shift = generic_ffs (block_size) - 1;

	dbg_printf ("%s(%d): %s => Manufacture_id=0x%04x, Device_id=0x%04x\n",
		__FUNCTION__, __LINE__, mtd->name, this->maker_code, this->device_code);
	dbg_printf ("%s(%d): oobsize=%d, writesize=%d, erasesize=%d, chipsize=%lu\n",
		__FUNCTION__, __LINE__, mtd->oobsize, mtd->writesize, mtd->erasesize, this->chipsize);
	dbg_printf ("%s(%d): ppb=%d, block_num=%d\n",
		__FUNCTION__, __LINE__, ppb, this->block_num);

	printk("[%s, line %d] this->numchips = %d\n",__FUNCTION__,__LINE__,this->numchips);
	mtd->size = this->numchips * this->chipsize;

	if (mtd->size > 0xFFFFFFFF) {
		panic("MTD Size too large\n");
	}

	/* alloc driver data/oob buffer */
	this->g_databuf = kmalloc (chunk_size, GFP_KERNEL);
	if (!this->g_databuf) {
		printk ("%s: Error, no enough memory for g_databuf\n", __FUNCTION__);
		return -ENOMEM;
	}
	memset (this->g_databuf, 0xff, chunk_size);

	//printk("%s, %s, line %d, REG(NACR) = 0x%08x\n",__FILE__,__func__,__LINE__,rtk_readl(NACR));
	this->g_oobbuf = kmalloc (oob_size, GFP_KERNEL);
	if (!this->g_oobbuf) {
		printk ("%s: Error, no enough memory for g_oobbuf\n", __FUNCTION__);
		return -ENOMEM;
	}
	memset (this->g_oobbuf, 0xff, oob_size);

	this->bbt = kmalloc (this->block_num, GFP_KERNEL);
	if (!this->bbt) {
		printk ("%s: Error, no enough memory for bbt\n", __FUNCTION__);
		return -ENOMEM;
	}
	memset (this->bbt, 0x0, this->block_num);


	//printk("otto: _nand_init \n");
	/* use otto nand flash struct to get nand flash data */

	//_nand_init ();

	/* assign MTD callback function */
	mtd->type      = MTD_NANDFLASH;
	mtd->flags     = MTD_CAP_NANDFLASH;
	mtd->erase     = nand_erase;
	mtd->point     = NULL;
	mtd->unpoint   = NULL;
	mtd->read      = nand_read;
	mtd->write     = nand_write;
	mtd->read_oob  = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->writev    = NULL;
	mtd->sync      = nand_sync;
	mtd->lock      = NULL;
	mtd->unlock    = NULL;
	mtd->owner     = THIS_MODULE;
//need_eric, reserve it
	mtd->ecclayout= &nand_bch_oob_64;

	mtd->ecclayout->oobavail = 0;
	for (i = 0; mtd->ecclayout->oobfree[i].length; i++)
		mtd->ecclayout->oobavail += mtd->ecclayout->oobfree[i].length;
	mtd->oobavail = mtd->ecclayout->oobavail;
//need_eric, reserve it
	mtd->block_isbad = nand_block_isbad;
	mtd->block_markbad = nand_block_markbad;

	memset(_bbt_table, '\0', sizeof(_bbt_table));
	if(this->block_num > MAX_BLOCKS){
		printk("Create BBT Error!, Block number > %d\n", MAX_BLOCKS);
	}
	else{
		create_bbt(mtd, _bbt_table, NULL, 0);
		//create bbt
#ifdef USE_BBT_SKIP
		create_logical_skip_bbt(mtd, _bbt_table);
#endif
	}//else
	return 1;
}


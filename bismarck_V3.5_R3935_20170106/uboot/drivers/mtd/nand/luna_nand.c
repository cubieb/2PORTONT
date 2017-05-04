/*
 * (C) Copyright 2006

 */

#include <malloc.h>
#include <watchdog.h>
#include <linux/err.h>
#include <asm/arch/bspchip.h>
#include <nand.h>
#include <linux/mtd/rtk_nand.h>
#include <soc.h>
#include <pblr.h>
#include <asm/io.h>
#include <asm/errno.h>

#define  USE_PRELOADER_NAND
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

/*eric: nouse */
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


//L-Preloader 
	// which means last area of 32KB-SRAM with cache aligned

#define nandflash_info para_flash_info
//#define nandflash_info  (parameters.soc.flash_info)
#define nandflash_info_num_chunk_per_block	  \
		((nandflash_info.num_page_per_block)/(nandflash_info.page_per_chunk))
		

/* nand driver low-level functions */

static void rtk_nand_read_id(struct mtd_info *mtd, unsigned char id[5], int chip_sel);

//static int rtk_write_oob(struct mtd_info *mtd, u16 chipnr, int page, int len, const u_char *buf);
static int rtk_block_isbad(struct mtd_info *mtd, u16 chipnr, loff_t ofs);
static int nand_pio_read  (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
static int nand_pio_write  (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops);

static int nand_read_oob_ext  (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen, u_char *buf, 
				u_char *oob_buf, struct nand_oobinfo *oobsel);
static int nand_read_ecc_512 (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel);

static int nand_read_ecc_subpage (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel);

static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const u_char * buf, 
			const u_char *oob_buf, struct nand_oobinfo *oobsel);
static void nand_select_chip(struct mtd_info *mtd, int chip);
static int rtk_nand_scan_bbt(struct mtd_info *mtd);
static int mtd_nand_block_markbad(struct mtd_info *mtd, loff_t ofs);
static void mtd_nand_sync(struct mtd_info *mtd);
static int mtd_nand_write_oob(struct mtd_info *mtd, loff_t to,struct mtd_oob_ops *ops);
static int mtd_nand_read_oob(struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops);
static int mtd_nand_block_isbad(struct mtd_info *mtd, loff_t offs);
static int mtd_nand_write(struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, const uint8_t *buf);
static int mtd_nand_read(struct mtd_info *mtd, loff_t from, size_t len,size_t *retlen, uint8_t *buf);
static int mtd_nand_erase(struct mtd_info *mtd, struct erase_info *instr);



static u32_t chunk_size;
static u32_t num_chunk_per_block;
static int page_size, oob_size, ppb;
#if 0

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

inline static int
bbt_is_bad_block(u32_t chunk_id) {
	// assert(parameters.bbt_valid)
	u32_t *bbt=parameters.bbt;
	u32_t b=chunk_id/nandflash_info_num_chunk_per_block;
//	printk("check bbt block %d , bbt bit is %d\n",b,_get_flags(bbt, b));
//return 0 => good , 1 =>bad
	return _get_flags(bbt, b);
}



inline static u8_t*
pbldr_spare_bbi(spare_u *spare) {
	return &(spare->u8_oob[nandflash_info.bbi_swap_offset]);
}

#endif


inline static u8_t*
pbldr_spare_bbi(spare_u *spare) {
	return &(spare->u8_oob[nandflash_info.bbi_swap_offset]);
}



/**	
**  check_ready()
**	descriptions: check nand flash ready bit.
**	parameters: 
**	return: 
**  note:  busy waiting
**/

static void check_ready()
{
	while(1) {
		if(  ( rtk_readl(NACR) & 0x80000000) == 0x80000000 ) 
			break;
	}
}


/**	
**  rtk_nand_read_id()
**	descriptions: read nand flash id
**	parameters: 
**	return: id 
**  note:  
**/


static void rtk_nand_read_id(struct mtd_info *mtd, u_char id[5], int chip_sel)
{
	int id_chain;
	int nand_strap_pin;

	if(chip_sel>1)  //only supports chip0, chip1
		return;


	check_ready();
	rtk_writel( (rtk_readl(NACR) |ECC_enable|RBO|WBO), NACR);

	//rtk_writel(0x0, NACMR);	
	rtk_writel( ((1<<(Chip_Seletc_Base+chip_sel))|CMD_READ_ID) , NACMR);          //read ID command
	check_ready();

	rtk_writel( 0x0, NAADR);
	rtk_writel( (0x0 |AD2EN|AD1EN|AD0EN) , NAADR);  //dummy address cycle
	check_ready();
	
	id_chain = rtk_readl(NADR);
	printf("rtk_nand_read_id id_chain is %lx\n",id_chain);
	id[0] = id_chain & 0xff;
	id[1] = (id_chain >> 8) & 0xff;
	id[2] = (id_chain >> 16) & 0xff;
	id[3] = (id_chain >> 24) & 0xff;

	id_chain = rtk_readl(NADR);
	id[4] = id_chain & 0xff;

	rtk_writel( 0x0, NACMR);
	rtk_writel( 0x0, NAADR);


	}
	

/**	
**  rtk_nand_scan()
**	descriptions: get nand flash information and setting driver callback function.
**	parameters: 
**	return: id 
**  note:  
**/


int rtk_nand_scan(struct mtd_info *mtd, int maxchips)
{

	struct nand_chip *this = mtd->priv;

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
	unsigned int flag_size;
	unsigned int mempage_order;


	if ( !this->select_chip )
		this->select_chip = nand_select_chip;

	if ( !this->scan_bbt )
		this->scan_bbt = rtk_nand_scan_bbt;

	this->active_chip = 0;
	mtd->name = "nand";
	this->read_id(mtd, id,0);
	
	maker_code = id[0];
	device_code = id[1];
	nand_type_id = maker_code<<24 | device_code<<16 | id[2]<<8 | id[3];
	B5th = id[4];
	B6th = id[5];
	printf ("%s: Manufacture ID=0x%02x, Chip ID=0x%02x, "
			"3thID=0x%02x, 4thID=0x%02x, 5thID=0x%02x, 6thID=0x%02x\n",
			mtd->name, id[0], id[1], id[2], id[3], id[4], id[5]);

	//flash information should from flash table
	//copy from SRAM , 
	page_size=nandflash_info.page_size;//real page size
	chunk_size=nandflash_info.page_size*nandflash_info.page_per_chunk;//chunk size
	block_size= nandflash_info.page_size * nandflash_info.num_page_per_block;
	num_chunk_per_block=nandflash_info_num_chunk_per_block;
	
	oob_size=64; /*note: chunk size 2k, oob always use 64 */
	
	device_size=chip_size=block_size*nandflash_info.num_block;  //total size

	dbg_printf("rtk_nand_scan, 	device_size is 0x%x\n",device_size);
	dbg_printf("rtk_nand_scan, 	block_size is %d\n",block_size);
			
//	this->page_shift = generic_ffs(page_size)-1; 
	this->chunk_shift = generic_ffs(chunk_size)-1; 

	this->phys_erase_shift = generic_ffs(block_size)-1;
	this->oob_shift = generic_ffs(oob_size)-1;

	ppb = this->ppb = nandflash_info.num_page_per_block/nandflash_info.page_per_chunk;  //chunk number pre block 
	
	this->block_num = nandflash_info.num_block;

	this->chipsize = chip_size;
	this->device_size = device_size;
	this->chip_shift =	generic_ffs(this->chipsize )-1;
	this->pagemask = (this->chipsize >> this->chunk_shift) - 1;		
	
	mtd->oobsize = this->oob_size = oob_size;		
//	mtd->writesize = page_size;				
	mtd->writesize = chunk_size; 

	mtd->erasesize = block_size;

	this->numchips=1; //only one chip
	mtd->size =device_size;

	dbg_printf("rtk_nand_scan mtd->size=0x%lx ,in line %d\n",mtd->size,__LINE__);
	dbg_printf("rtk_nand_scan chipsize=%x ,in line %d\n",this->chipsize,__LINE__);
	dbg_printf("rtk_nand_scan numchips=0x%x ,in line %d\n",this->numchips,__LINE__);
	printk("this->pagemask is %d\n",this->pagemask);
	printk("this->chip_shift is %d\n",this->chip_shift);

	if (mtd->size > 0xFFFFFFFF) {
		printf("MTD Size too large\n");
	}
	this->g_databuf = kmalloc( chunk_size, GFP_KERNEL );
	if ( !this->g_databuf ){
		printk("%s: Error, no enough memory for g_databuf\n",__FUNCTION__);
		return -1;  //return value need check
	}
	memset(this->g_databuf, 0xff, chunk_size);

	this->g_oobbuf = kmalloc( oob_size, GFP_KERNEL );
	if ( !this->g_oobbuf ){
		printk("%s: Error, no enough memory for g_oobbuf\n",__FUNCTION__);
		return -1; //return value need check
	}
	memset(this->g_oobbuf, 0xff, oob_size);


	mtd->type			= MTD_NANDFLASH;
	mtd->flags			= MTD_CAP_NANDFLASH;
	mtd->erase			= mtd_nand_erase;
	mtd->point			= NULL;
	mtd->unpoint			= NULL;
	mtd->read			= mtd_nand_read;
	mtd->write			= mtd_nand_write;
	mtd->read_oob		= mtd_nand_read_oob;
	mtd->write_oob		= mtd_nand_write_oob;
	mtd->sync			= mtd_nand_sync;
	mtd->lock			= NULL;
	mtd->unlock 		= NULL;
	mtd->block_isbad		= mtd_nand_block_isbad;
	mtd->block_markbad	= mtd_nand_block_markbad;


	//create skip bbt table to record good bbt 
	this->bbt = kmalloc( nandflash_info.num_block, GFP_KERNEL );
	memset(this->bbt, 0x0, nandflash_info.num_block);	
	//bbi table is ok.
	rtk_scan_bbt();
#ifdef USE_BBT_SKIP	
	create_logical_skip_bbt(mtd);
#endif
	return 0;

}

/**	
**  nand_select_chip()
**	descriptions: set chip select pin
**	parameters: chip 0/1
**	return: 
**  note: this driver only support chip 0.
**/

static void nand_select_chip(struct mtd_info *mtd, int chip)
{

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
			rtk_writel(0x0, NACMR);  // only support chip1 & chip0
	}
}

/**	
**  rtk_nand_scan_bbt()
**	descriptions: scan bad block
**	parameters: 
**	return:  
**  note:  
**/
static int rtk_nand_scan_bbt(struct mtd_info *mtd)
{

	rtk_scan_bbt();
	return 0;
}



/**
 * nand_read_oob - [MTD Interface] NAND read data and/or out-of-band
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @ops:	oob operation description structure
 *
 * NAND read data and/or out-of-band data
 * return:  
 */
static int mtd_nand_read_oob(struct mtd_info *mtd, loff_t from,
			 struct mtd_oob_ops *ops)
{

dbg_printf("%s(%d):%d,%d\n",__FUNCTION__,__LINE__,ops->len,ops->ooblen);
dbg_printf("%s(%d):from is 0x%x\n",__FUNCTION__,__LINE__,from);
dbg_printf("%s(%d):mtd->name %s\n",__FUNCTION__,__LINE__,mtd->name);

	if(ops->mode==MTD_OOB_RAW){
		/* for pio mode read */
		return nand_pio_read(mtd, from,ops);
	}else{
		if(ops->len==ops->ooblen){// read oob(only yaffs use this case		
			return nand_read_oob_ext(mtd, from,ops);	
		}else{
			return nand_read_ecc(mtd, from, ops->len, &ops->retlen,ops->datbuf, ops->oobbuf, NULL);
		}
	}
		
}

/**
 * nand_write_oob - [MTD Interface] NAND write data and/or out-of-band
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @ops:	oob operation description structure
 */
static int mtd_nand_write_oob(struct mtd_info *mtd, loff_t to,
				  struct mtd_oob_ops *ops)
{
	int rc=0;

	dbg_printf("In NAND\n");
	dbg_printf("ops.mode = %d\n",ops->mode);
	dbg_printf("ops.ooblen = %d\n",ops->ooblen);
	dbg_printf("ops.len = %d\n",ops->len);
	dbg_printf("ops.ooboffs= %d\n",ops->ooboffs);
	dbg_printf("ops.datbuf = 0x%p\n",ops->datbuf);
	dbg_printf("ops.oobbuf = 0x%p\n",ops->oobbuf);
	
	if(ops->mode==MTD_OOB_RAW){		
		return nand_pio_write(mtd, to,ops);
	}else{
		rc =  nand_write_ecc (mtd, to, ops->len, &ops->retlen,ops->datbuf, ops->oobbuf, NULL);
	}
	return rc;
}


/**
 * nand_sync - [MTD Interface] sync
 * @mtd:	MTD device structure
 *
 * Sync is actually a wait for chip ready function
 */
static void mtd_nand_sync(struct mtd_info *mtd)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	this->state = FL_READY;
}



/**
 * nand_block_markbad - [MTD Interface] Mark block at the given offset as bad
 * @mtd:	MTD device structure
 * @ofs:	offset relative to mtd start
 */
static int mtd_nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{

	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int chunk_id, block;
	int i;
	int rc = 0;
	unsigned char buf[2048+64];
	int chipnr, chipnr_remap;

	chunk_id = ((int) ofs) >> this->chunk_shift;
	this->active_chip = chipnr = chipnr_remap = (int)(ofs >> this->chip_shift);

	block = chunk_id/ppb;


//#ifdef USE_BBT_SKIP
//	return 1; //always return fail,USE_BBT_SKIP not support markbad
//#else	
	/* set bad table array*/
	printk("set block %d is bad \n",block);
    //_set_flags(parameters.bbt, block);
	memset(buf, 0x0, sizeof(buf));
	printk("chunk id is %d\n",chunk_id);
	rtk_PIO_write(chunk_id,0,chunk_size+oob_size,&buf[0]);	
	
	return 0;
//#endif	

}

/**
 * nand_erase - [MTD Interface] erase block(s)
 * @mtd:	MTD device structure
 * @instr:	erase instruction
 *
 * Erase one ore more blocks
 */
static int mtd_nand_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	dbg_printf("mtd_nand_erase\n");
	return nand_erase_nand (mtd, instr, 0);
}

/**
 * nand_read - [MTD Interface] MTD compability function for nand_do_read_ecc
 * @mtd:	MTD device structure
 * @from:	offset to read from
 * @len:	number of bytes to read
 * @retlen:	pointer to variable to store the number of read bytes
 * @buf:	the databuffer to put data
 *
 * Get hold of the chip and call nand_do_read
 */
static int mtd_nand_read(struct mtd_info *mtd, loff_t from, size_t len,
		     size_t *retlen, uint8_t *buf)
{
	dbg_printf("%s(%d)\n",__FUNCTION__,__LINE__);

	if ((from + len) > mtd->size) {
		printk ("mtd_nand_read: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	if ( NOTALIGNED(mtd, len) || NOTALIGNED (mtd, from) ) {		
		//printk("read size not page align len = %d\n",len);
		return nand_read_ecc_subpage(mtd, from, len, retlen, buf, NULL, NULL);
	}else{
		return nand_read_ecc(mtd, from, len, retlen, buf, NULL, NULL);
	}

}

int nand_erase_nand (struct mtd_info *mtd, struct erase_info *instr, int allowbbt)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	u_int32_t addr = instr->addr;
	u_int32_t len = instr->len;
	int chunk_id, chipnr;
	int i, old_chunk, phy_block,logical_block;
	int elen = 0;
	int rc = 0;
	int realchunk, chipnr_remap;

	check_end (mtd, addr, len);
	dbg_printf("mtd->erasesize is %x\n",mtd->erasesize);
	dbg_printf("erase addr is %x\n",addr);
	check_block_align (mtd, addr);

	instr->fail_addr = 0xffffffff;

	realchunk = ((int) addr) >> this->chunk_shift;
	old_chunk = chunk_id = realchunk & this->pagemask;
	phy_block = chunk_id/ppb;


	logical_block = phy_block+ this->bbt[phy_block];	
	dbg_printf("%s(%d):logical block index is %d\n",__FUNCTION__,__LINE__,logical_block);
		
	if(this->bbt[phy_block]>=0xff){
		printk ("nand_erase_nand: logical_block beyond end of device\n");
		return -EINVAL;
	}
	dbg_printf("[%s] [%d] phy_block: %d, page: %d\n",__FUNCTION__,__LINE__,phy_block,chunk_id);


	instr->state = MTD_ERASING;
	while (elen < len) {
		WATCHDOG_RESET();
		
		chunk_id = logical_block*ppb;

		rc=parameters._nand_erase_block(chunk_id); //return 0:succes,-1 is fail
		if (rc!=0) {
			printf("%s: block erase failed at page address=0x%08x\n", __FUNCTION__, addr);
			/*TBD */
 			//erase fail,  maybe set bbi table fail....
			
		}
		//erase next block 
		
		elen += mtd->erasesize;
		old_chunk += ppb;	
		phy_block = old_chunk/ppb;
		logical_block = phy_block+ this->bbt[phy_block];	
		dbg_printf("%s(%d):logical block index is %d\n",__FUNCTION__,__LINE__,logical_block);
				
		if(this->bbt[phy_block]>=0xff){
			printk ("nand_erase_nand: logical_block beyond end of device\n");
			instr->state = MTD_ERASE_DONE;
			return -EINVAL;
		}

		
	}
	instr->state = MTD_ERASE_DONE;

	return rc;
}



/**
 * nand_write - [MTD Interface] NAND write with ECC
 * @mtd:	MTD device structure
 * @to:		offset to write to
 * @len:	number of bytes to write
 * @retlen:	pointer to variable to store the number of written bytes
 * @buf:	the data to write
 *
 * NAND write with ECC
 */
static int mtd_nand_write(struct mtd_info *mtd, loff_t to, size_t len,
			  size_t *retlen, const uint8_t *buf)
{

	return (nand_write_ecc (mtd, to, len, retlen, buf, NULL, NULL));

}


/**
 * mtd_nand_block_isbad - [MTD Interface] Check if block at offset is bad
 * @mtd:	MTD device structure
 * @offs:	offset relative to mtd start
 * return : 1 is bad, 0 is good
 */
static int mtd_nand_block_isbad(struct mtd_info *mtd, loff_t offs)
{

	int chipnr=0, val;
	val = rtk_block_isbad(mtd,chipnr,offs);
	//return 1 is bad, 0 is good
	return val;
}

/*
 * Board-specific NAND init.  Copied from include/linux/mtd/nand.h for reference.
 *
 * struct nand_chip - NAND Private Flash Chip Data
 * @IO_ADDR_R:		[BOARDSPECIFIC] address to read the 8 I/O lines of the flash device
 * @IO_ADDR_W:		[BOARDSPECIFIC] address to write the 8 I/O lines of the flash device
 * @hwcontrol:		[BOARDSPECIFIC] hardwarespecific function for accesing control-lines
 * @dev_ready:		[BOARDSPECIFIC] hardwarespecific function for accesing device ready/busy line
 *			If set to NULL no access to ready/busy is available and the ready/busy information
 *			is read from the chip status register
 * @enable_hwecc:	[BOARDSPECIFIC] function to enable (reset) hardware ecc generator. Must only
 *			be provided if a hardware ECC is available
 * @eccmode:		[BOARDSPECIFIC] mode of ecc, see defines
 * @chip_delay:		[BOARDSPECIFIC] chip dependent delay for transfering data from array to read regs (tR)
 * @options:		[BOARDSPECIFIC] various chip options. They can partly be set to inform nand_scan about
 *			special functionality. See the defines for further explanation
*/
/*
 * This routine initializes controller and GPIOs.
 */
int board_nand_init(struct nand_chip *nand)
{
	unsigned int value;
	printf("board_nand_init()\n");
	/* initialize hardware */
	rtk_writel(0xC00FFFFF, NACR);     //Enable ECC	
	rtk_writel(0x0000000F, NASR);     //clear NAND flash status register
	/* initialize nand_chip data structure */
	nand->select_chip = NULL;
	nand->read_id			= rtk_nand_read_id;
	nand->ecc.mode = NAND_ECC_NONE;
	nand->options = 0;
#if 0
	printf("parameters at 0x%p\n",parameters);
	printf("parameters.read at 0x%p\n",parameters._nand_read_chunk);
	printf("parameters.write at 0x%p\n",parameters._nand_write_chunk);
	printf("parameters.bbt at 0x%p\n",parameters.bbt);
	printf("uboot- read nand flash info from SRAM\n");
	printf("flash_info list\n");
	printf("flash_info.num_block : %d\n",nandflash_info.num_block);
	printf("flash_info.num_page_per_block : %d\n",nandflash_info.num_page_per_block);
	printf("flash_info.page_per_chunk : %d\n",nandflash_info.page_per_chunk);
	printf("flash_info.bbi_dma_offset : %d\n",nandflash_info.bbi_dma_offset);
	printf("flash_info.bbi_raw_offset : %d\n",nandflash_info.bbi_raw_offset);
	printf("flash_info.bbi_swap_offset : %d\n",nandflash_info.bbi_swap_offset);
	printf("flash_info.page_size : %d\n",nandflash_info.page_size);
	printf("chunk size : %d\n",nandflash_info.page_size*nandflash_info.page_per_chunk);
	
	printf("flash_info.addr_cycles : %d\n",nandflash_info.addr_cycles);
	printf("pblr_start_block : %d\n",nandflash_info.pblr_start_block);
	printf("num_pblr_block : %d\n",nandflash_info.num_pblr_block);
	printf("parameters.curr_ver is %x\n",parameters.curr_ver);
	printf("parameters.plr_num_chunk is %d\n",parameters.plr_num_chunk);
	printf("parameters.blr_num_chunk is %d\n",parameters.blr_num_chunk);
	printf("parameters.end_pblr_block is %d\n",parameters.end_pblr_block);
#endif	
	return (0);
}


/**	
**  nand_read_oob_ext()
**	descriptions: only read oob data from nand flash.
**	parameters: 
**	return: 
**  note:  
**/


static int nand_read_oob_ext  (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int oob_len = 0, thislen;
	int rc=0;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap;
	//__u8 oob_area[64];
	__u8 *oob_area, oob_area0[2048+64];
	dbg_printf("%s(%d):\n",__FUNCTION__,__LINE__);

	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);
	if ((from + ops->len) > mtd->size) {
		printk ("nand_read_oob: Attempt read beyond end of device\n");
		ops->retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, from)) {
		printk (KERN_NOTICE "nand_read_oob: Attempt to read not page aligned data. %llx\n", from);
		return -EINVAL;
	}

//	printk("NAND :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n"
	//		,ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );

	realchunk = (int)(from >> this->chunk_shift);
	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1);
	phy_block = chunk_id/ppb;
	logical_block=phy_block+this->bbt[phy_block];

	if(this->bbt[phy_block]>=0xff){
		printk ("nand_read_oob_ext: Attempt read block beyond of devic\n");
		ops->retlen = 0;
		return -EINVAL;
	}
		
	if ( &(ops->retlen)) 
		ops->retlen = 0;
	
	thislen = oob_size;

	while(oob_len < ops->len) {
		WATCHDOG_RESET();
		if (thislen > (ops->len - oob_len)) 
			thislen = (ops->len - oob_len);
	
		chunk_id = logical_block*ppb + chunk_offset;

		rc=parameters._nand_read_chunk(this->g_databuf,(ops->oobbuf)+oob_len,chunk_id);
	
		if(rc!=0){
			if (rc == -1){
				printk ("%s: Un-correctable HW ECC\n", __FUNCTION__);			
				printk("rtk_read_oob: Un-correctable HW ECC Error at page=%d\n", chunk_id);				
				return -1;
			}else{
				printk ("%s: ecc error count %d , can fix it\n", __FUNCTION__,rc);
				
			}
		}
		//read next data
		oob_len += thislen;
		old_chunk++;
		chunk_offset = old_chunk & (ppb-1);
		phy_block = old_chunk/ppb;		
		logical_block=phy_block+this->bbt[phy_block];

		if(this->bbt[phy_block]>=0xff){
			printk ("nand_read_oob_ext: Attempt read block beyond of devic\n");
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
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int data_len, oob_len;
	int rc;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap;
	__u8 *oob_area, oob_area0[64];

	memset(oob_area0, 0x0, 64);
	oob_area = (__u8*) ((u32)(oob_area0 ));

	dbg_printf("data buf address = 0x%p, len = %d  ,mtd->size=%lx MB ,in line %d\n",buf, len,mtd->size/1024,__LINE__);
	dbg_printf("oob buf address = 0x%p",oob_buf);
	dbg_printf("oob len is = %d\n",mtd->oobsize);
	dbg_printf("%s(%d):\n",__FUNCTION__,__LINE__);


	if ((from + len) > mtd->size) {
		printk ("nand_read_ecc: Attempt read beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}
	
	if (NOTALIGNED (mtd, from) ) {		
		printk("[%s, line %d] from=%llxh, mtd->writesize=%d, len=%d\n"	,__FUNCTION__,__LINE__,from, mtd->writesize,len);
		printk (KERN_NOTICE "nand_read_ecc: Attempt to read From not page aligned data\n");
		return -EINVAL;
	}
	if ( NOTALIGNED(mtd, len)) {		
		printk("read size not page align len = %d\n",len);

	
	}
	

	
	realchunk = (int)(from >> this->chunk_shift);

	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1);
	phy_block = chunk_id/ppb;
	dbg_printf("%s(%d):phsical block index is %d\n",__FUNCTION__,__LINE__,phy_block);
	logical_block = phy_block+ this->bbt[phy_block];	
	dbg_printf("%s(%d):logical block index is %d\n",__FUNCTION__,__LINE__,logical_block);

	if(this->bbt[phy_block]>=0xff){
		printk ("nand_read_ecc: Attempt read block beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}



	if ( retlen )
		*retlen = 0;
	
	data_len = oob_len = 0;

	dbg_printf("nand_read_ecc:  len is %d\n",len);
	while (data_len < len) {
		WATCHDOG_RESET();
		chunk_id = logical_block*ppb + chunk_offset;  

		dbg_printf("%s(%d):from=%llx,chunk_id=%d,fn=%p\n",__FUNCTION__,__LINE__,from,chunk_id,this->read_ecc_page);
		dbg_printf("\r\n goto read_Ecc_page()\n");	
		if(oob_buf!=NULL){
			memset(oob_buf, 0xff,  mtd->oobsize);
			rc=parameters._nand_read_chunk(&buf[data_len],&oob_buf[oob_len],chunk_id);
		}else{
			rc=parameters._nand_read_chunk(&buf[data_len],&oob_area,chunk_id);
	
		}
		
		dbg_printf("data_len=%d, oob_len=%d, buf address = 0x%p, oob address = 0x%p  ,in line %d\n"
			, data_len, oob_len, buf, oob_buf,__LINE__);
		if (rc != 0) {
			if (rc == -1){
				printk ("%s: read_ecc_page: Un-correctable HW ECC\n", __FUNCTION__);				
				printk("rtk_read_ecc_page: Un-correctable HW ECC Error at chunk_id=%d\n", chunk_id);
				return -1;
			}else{
				printk ("%s: ecc fail count: %d ,nand controller can fix it\n", __FUNCTION__,rc);
				
			}
		}

		data_len += chunk_size;

		oob_len += oob_size;		
		old_chunk++;
		chunk_offset = old_chunk & (ppb-1);
		phy_block = old_chunk/ppb;		
		logical_block = phy_block+ this->bbt[phy_block];	
		if(this->bbt[phy_block]>=0xff){
			printk ("nand_read_ecc: Attempt read block beyond end of device\n");
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
	return 0;
}



/* goes around 2k limitation , andrew */
/* eric: must read 2048 size, and return first 512 byte */
static int nand_read_ecc_512 (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel)
{
	static u64 nand_offset;
	static u8 nand_buffer[2048];

	
	size_t my_retlen;
	#define ADDR_MASK (2048 - 1)
	#define NANDSEG(x) (x & ~ADDR_MASK)
	dbg_printf("%s(%d)\n",__FUNCTION__,__LINE__);

	if (NANDSEG(from) != NANDSEG(nand_offset)) {
		int ret;
		ret = nand_read_ecc(mtd, NANDSEG(from), 2048, &my_retlen, nand_buffer, NULL, NULL);
		if (ret)
			return ret;
		nand_offset = NANDSEG(from);
	}

	memcpy(buf, &nand_buffer[ from - nand_offset ], len);

	*retlen = len;
	return 0;
	
}

/**
**  nand_read_ecc_subpage()
**	descriptions: read data/oob from nand flash , if address and size not alignment..
**	parameters: 
**	return: 
**  note: only supprt 2k page size.
**/


static int nand_read_ecc_subpage(struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *oobsel)
{
	static u64 nand_offset;
	static u8 nand_buffer[2048];

	size_t data_len=len;
	int firstsize=0;
	int buffer_offset=0;
	size_t my_retlen;
	int ret;
	#define ADDR_MASK (2048 - 1)
	#define NANDSEG(x) (x & ~ADDR_MASK) //read from align address
	dbg_printf("%s(%d)\n",__FUNCTION__,__LINE__);

	*retlen=0;

	if (NANDSEG(from) != NANDSEG(nand_offset)) {		
		ret = nand_read_ecc(mtd, NANDSEG(from), 2048, &my_retlen, nand_buffer, NULL, NULL);
		if (ret)
			return ret;
		nand_offset = NANDSEG(from);
	}

	firstsize= mtd->writesize - (from - nand_offset);

	if(len<=firstsize){
		/* first read is only on one page */
		memcpy(buf, &nand_buffer[ from - nand_offset ], len);
		
		*retlen = len;
		return 0;

	}else{
		/*copy first page data to buf  */
		memcpy(&buf[buffer_offset], &nand_buffer[ from - nand_offset ], firstsize);

		buffer_offset+=firstsize;
		from+=firstsize;
		*retlen+=firstsize;
		data_len=data_len-firstsize;
	//	printf("firstsize is %d\n",firstsize);
		//printf("first date_len is %d\n",data_len);
		
		while(data_len>0){
			WATCHDOG_RESET();
			/*read next page data */
			ret = nand_read_ecc(mtd, NANDSEG(from), mtd->writesize, &my_retlen, nand_buffer, NULL, NULL);

			if(data_len>mtd->writesize){ /*still need read more chunk data */

				memcpy(&buf[buffer_offset], &nand_buffer[0], mtd->writesize);
				buffer_offset+=mtd->writesize;
				from+=mtd->writesize;
				*retlen+=mtd->writesize;
				data_len=data_len-mtd->writesize;
				//printf("001 data_len is %d\n",data_len);
			}else{ /*copy last page */
				
				memcpy(&buf[buffer_offset], &nand_buffer[0], data_len);
				*retlen+=data_len;
				//	printf("002 data_len is %d\n",data_len);
				data_len=0;

			}
		}
	}

	if(*retlen!=len)
		printf("nand_read_ecc_subpage return size fail \n");
	
	return 0;	
}


/**
**  nand_write_ecc()
**	descriptions: DMA write data to nand flash (nand flash Add ECC)
**	parameters:  
**	return:  -1 : fail,  0~N: ecc fix count.
**  note: 
**/

static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len, size_t *retlen, 
			const u_char * buf, const u_char *oob_buf, struct nand_oobinfo *oobsel)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int data_len, oob_len;
	int rc;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap, err_chipnr = 0, err_chipnr_remap = 1;

	__u8 *oob_area, oob_area0[64];
	dbg_printf("%s(%d):\n",__FUNCTION__,__LINE__);
	memset(oob_area0, 0xff,64);
	memset(this->g_databuf, 0xff, chunk_size);


	if(buf==NULL){
		printk ("nand_write_ecc: data buf is null\n");
		return -EINVAL;
	}

	if(oob_buf==NULL){	
	//	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);
		oob_area = (__u8*) ((u32)(oob_area0));
	}
	
	if ((to + len) > mtd->size) {
		printk ("nand_write_ecc: Attempt write beyond end of device\n");
		*retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, to) ) {
		printk("[%s, line %d] mtd->writesize=%d, to=%d\n"	,__FUNCTION__,__LINE__, mtd->writesize,to);
		
		printk (KERN_NOTICE "nand_write_ecc: Attempt to write not page aligned addr to %x\n",to);
		return -EINVAL;
	}

	if ( NOTALIGNED(mtd, len)) {


		#define NANDSEG(x) (x & ~(mtd->writesize-1))

		printk("nand write len not align , len = %d \n",len);
		len=(len+mtd->writesize)-(len-NANDSEG(len));
		printk("auto write align size, len = %d ",len);

	}




	

	realchunk = (int)(to >> this->chunk_shift);
	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1);
	phy_block = chunk_id/ppb;
	
	logical_block = phy_block+ this->bbt[phy_block];	
	if(logical_block!=phy_block){
		printk("chunk_offset is %d\n",chunk_offset);
		printk("write logic %d , phy %d is bad \n",logical_block,phy_block);
		printk("%s(%d):logical block index is %d\n",__FUNCTION__,__LINE__,logical_block);
	}
	if(this->bbt[phy_block]>=0xff){
		printk ("nand_write_ecc: Attempt read block beyond end of device,no block enough\n");
		*retlen = 0;
		return -EINVAL;
	}



	
	if ( retlen )
		*retlen = 0;
	
	data_len = oob_len = 0;

	while ( data_len < len) {
		WATCHDOG_RESET();
		chunk_id = logical_block*ppb + chunk_offset;

		if(oob_buf!=NULL){			
//#ifdef CONFIG_CMD_UBIFS 
#if 1
/*for ubi file system, ubi image have some empty page write for kernel usage, but kernel use the empty page not erase it, 
so the ecc will write twice , in uboot, first check the page are all 0xff, not write it. */
			if((memcmp(this->g_databuf,&buf[data_len],chunk_size)!=0)||(memcmp(oob_area,&oob_buf[oob_len],oob_size)!=0)){
			rc=parameters._nand_write_chunk(&buf[data_len],&oob_buf[oob_len],chunk_id);
			}else
				rc=0;
#else
			rc=parameters._nand_write_chunk(&buf[data_len],&oob_buf[oob_len],chunk_id);

#endif
		}else{	
//#ifdef CONFIG_CMD_UBIFS 		
#if 1
			/*check all data are 0xff, not need write */
			if(memcmp(this->g_databuf,&buf[data_len],chunk_size)!=0){
			rc=parameters._nand_write_chunk(&buf[data_len],oob_area,chunk_id);
			}else
				rc=0;
#else
				rc=parameters._nand_write_chunk(&buf[data_len],oob_area,chunk_id);
#endif
		}
	
		if (rc != 0){
			if ( rc == -1){
				printk ("%s: write failed ,chunk id %d\n", __FUNCTION__,chunk_id);
				//TBD : write fail, ecc fail .	
				return -1;
			}else if(rc >0){
				printk ("%s: ecc bit fail count =%d  , still can fix it\n", __FUNCTION__, rc);
				//return rc;
			}	
		}		

		data_len += chunk_size;
		oob_len += oob_size;
		old_chunk++;
		chunk_offset = old_chunk & (ppb-1);
		phy_block = old_chunk/ppb;
		logical_block = phy_block+ this->bbt[phy_block];	


		if(this->bbt[phy_block]>=0xff){
			printk ("nand_write_ecc: Attempt read block beyond end of device,no block enough\n");
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
	return 0;
}




/**	
**  rtk_PIO_read()
**	descriptions: driver function, call by nand_pio_read()
**	parameters:  
**	return:  
            
**/

//read real page raw data
void rtk_PIO_read(int page, int offset, int length, unsigned char * buffer)
{
	int i;
	unsigned int flash_addr1, flash_addr2;
	unsigned int data_out;
	int rlen, pio_length;
	int real_page;
	int frag_count=0;
	
	rtk_writel(0xc00fffff, NACR);

	real_page = page;
	pio_length = length;

	while(pio_length >0){
		WATCHDOG_RESET();
		if(pio_length > (page_size+oob_size)){
			rlen = (page_size+oob_size);
			pio_length -= (page_size+oob_size);
		}else{
			rlen = pio_length;
			pio_length -= rlen;
		}

		if(nandflash_info.page_size==512){
			flash_addr1 = (real_page << 8) & 0xffffff;
			flash_addr2 = (real_page >> 16) & 0xffffff;

			if(offset<256){ /*0~255*/
				flash_addr1|=offset;
			/* Command cycle 1*/
				rtk_writel((CECS0|CMD_PG_READ_C1), NACMR);

			}else if((255<offset)&(offset<512)){
				offset-=256;	
				flash_addr1|=offset;
				/* Command cycle 1*/
				rtk_writel((CECS0|CMD_PG_READ_512_R2), NACMR);	//CMD 01h
			}else if((offset>511)&(offset<518)){
				offset-=512;			
				flash_addr1|=offset;		
				/* Command cycle 1*/
				rtk_writel((CECS0|CMD_PG_READ_512_R3), NACMR); // CMD 50h	
			}
	
		}else{
			flash_addr1 =  ((real_page & 0xff) << 16) | offset;
			flash_addr2 = (real_page >> 8) & 0xffffff;
			/* Command cycle 1*/
			rtk_writel((CECS0|CMD_PG_READ_C1), NACMR);
			check_ready();			
		}


		/* Give address */
#if 0		
		rtk_writel( (enNextAD|AD2EN|AD1EN|AD0EN|flash_addr1), NAADR);

		if(nandflash_info.addr_cycles==5){
			rtk_writel( (AD1EN|AD0EN|flash_addr2), NAADR);
		}else{
			rtk_writel( (AD0EN|flash_addr2), NAADR);
		}
#endif
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

	
		if(nandflash_info.page_size!=512){
			/* Command cycle 2*/
			rtk_writel((CECS0|CMD_PG_READ_C2), NACMR);
			check_ready();
		}			

		for(i=0; i<(rlen/4); i++){
			data_out = rtk_readl(NADR);
//			printk("[%d] data_out = 0x%08X\n", i, data_out);
#if 0	//debug only		
			if ( (frag_count % 16) == 0 ) 
				printk("\n");
				
			if ( ((frag_count % 528)==0) && (frag_count != 0 ) )
				printk("******************\n");
//			if(frag_count<512)
				frag_count=frag_count+4;

			printk("[%d] data_out = 0x%08X\n", i, data_out);

			printk("0x%08X ", data_out);
#endif
			memcpy( buffer+i*4, &data_out, 4);
		}

		check_ready();
		rtk_writel(0, NACMR);
		frag_count=0;
		real_page++;
	}
	return;
	
}

/**	
**  rtk_PIO_write()
**	descriptions: driver function, call by nand_pio_write()
**	parameters:  
**	return: write raw data to nand flash (one page, nand chip not auto add ecc)            
**/
void rtk_PIO_write(int real_page, int offset, int length, unsigned char * buffer)
{
	int i;
	unsigned int flash_addr1, flash_addr2;

	int rlen, pio_length;


	
	rtk_writel(0xc00fffff, NACR);

	pio_length = length;

	while(pio_length >0){
		WATCHDOG_RESET();
		if(pio_length > (page_size+oob_size)){
			rlen = (page_size+oob_size);
			pio_length -= (page_size+oob_size);
		}else{
			rlen = pio_length;
			pio_length -= rlen;
		}

		/* Command write cycle 1*/
		rtk_writel((CECS0|CMD_PG_WRITE_C1), NACMR);

		if(nandflash_info.page_size==512){
			flash_addr1 |= ((real_page & 0xffffff) << 8);
		}else{
			flash_addr1 =  ((real_page & 0xff) << 16)| offset ;
			flash_addr2 = (real_page >> 8) & 0xffffff;
		}
#if 0
		/* set address 1 */
		rtk_writel( (enNextAD|AD2EN|AD1EN|AD0EN|flash_addr1), NAADR);
		check_ready();

		/*set address 2 */

		if(nandflash_info.addr_cycles==5){
		rtk_writel( (AD1EN|AD0EN|flash_addr2), NAADR);
		}else{
		rtk_writel( (AD0EN|flash_addr2), NAADR);
		}
#endif
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

		for(i=0; i<(rlen/4); i++){

//			printk("pio write dram addr is  = 0x%08X ,data is %x \n",buffer+i*4,*(u32_t *)(buffer+i*4));
			rtk_writel( *(u32_t *)(buffer+i*4), NADR);
		}

		rtk_writel((CECS0|CMD_PG_WRITE_C2), NACMR);
		check_ready();


		rtk_writel(0, NACMR);

		real_page++;

	}

	
	return;
	
}


void rtk_flash_reset()
{
	int i;
	unsigned int flash_addr1, flash_addr2;
	unsigned int data_out;
	int rlen, pio_length;
	int real_page;
	int frag_count=0;
//	cache_flush();
	
	parameters._dcache_writeback_invalidate_all();
	
	rtk_writel(0xc00fffff, NACR);
	
	check_ready();

/* Command cycle 1*/
	rtk_writel((CECS0|CMD_RESET), NACMR);
	check_ready();


	dbg_printf("reset ok\n");
	
	return;
	
}


/**	
**  rtk_block_isbad()
**	descriptions: read bbi table and block is bad or good
**	parameters: mtd, ofs: flash address
**	return:  1: bad , 0: good
**  note: 
**/


static int rtk_block_isbad(struct mtd_info *mtd, u16 chipnr, loff_t ofs)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int chunk_id, phy_block,logical_block;

	int j;
	chunk_id = ((int) ofs) >> this->chunk_shift;
	phy_block = chunk_id/ppb;
#ifdef USE_BBT_SKIP
	
	j=_get_flags(parameters.bbt, phy_block);
		
//	logical_block=phy_block+this->bbt[phy_block];
//	printk ("phy_block=%d return %d  logical_block is %d\n",phy_block,j,logical_block);
	if(j==1){
		printf("physical block %d (%08llx) is bad\n",phy_block,ofs);
	}
		
	if(this->bbt[phy_block]>=0xff){
		return 1;
	}else{
		return 0;
	}
#else
	j=_get_flags(parameters.bbt, phy_block);
	if(j==1){
		printf("block %d (%08llx) is bad\n",phy_block,ofs);
	}
	return j;
#endif	

}


/**	
**  nand_pio_read()
**	descriptions: use pio mode to read nand flash (raw data,by pass ecc)
**	parameters:  mtd
				 from: flash  address
				 ops: oob operation description structure
**	return: 0:success
            EINVAL: fail
**/
static int nand_pio_read  (struct mtd_info *mtd, loff_t from, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int oob_len = 0, thislen;
	int rc=0;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap;
	__u8 *oob_area, oob_area0[64+16];
	u_char *buf=ops->datbuf;

	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);
	
	if ((from + ops->len) > mtd->size) {
		printk ("nand_pio_read:  Attempt read beyond end of device\n");
		ops->retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, from)) {
		printk (KERN_NOTICE "nand_pio_read: Attempt to read not page aligned data. %llx\n", from);
		return -EINVAL;
	}

	dbg_printf("pio read NAND :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n"
			,ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );

	realchunk = (int)(from >> this->chunk_shift);
	this->active_chip = chipnr = chipnr_remap = (int)(from >> this->chip_shift);
	old_chunk = chunk_id = realchunk & this->pagemask;
	chunk_offset = chunk_id & (ppb-1);
	phy_block = chunk_id/ppb;
	logical_block=phy_block+this->bbt[phy_block];

	
	if(this->bbt[phy_block]>=0xff){
		printk ("nand_pio_read: Attempt read block beyond end of device,no block enough\n");
		return -EINVAL;
	}
	
	this->select_chip(mtd, chipnr);
	
	if ( &(ops->retlen)) 
		ops->retlen = 0;

	thislen=0;

	while (thislen < ops->len) {
		WATCHDOG_RESET();
		chunk_id = logical_block*ppb + chunk_offset;

		dbg_printf("[%s, line %d] chunk_id = %d, databuf = 0x%p thislen =%d \n",__FUNCTION__,__LINE__, chunk_id, &buf[thislen],thislen);
	
		rtk_PIO_read(chunk_id,0,chunk_size+oob_size,&buf[thislen]);

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
		logical_block=phy_block+this->bbt[phy_block];
			
		if(this->bbt[phy_block]>=0xff){
			printk ("nand_pio_read: Attempt read block beyond end of device,no block enough\n");
			return -EINVAL;
		}

	}
	

	return 0;
}


/**
**  nand_pio_write()
**	descriptions: pio write raw data(data+oob+ecc) to nand flash
**	parameters:  
**	return: 
**  note: pio write is by pass hardware ecc.
**/

static int nand_pio_write  (struct mtd_info *mtd, loff_t to, struct mtd_oob_ops *ops)
{
	struct nand_chip *this = mtd->priv;
	unsigned int chunk_id, realchunk;
	int oob_len = 0, thislen;
	int rc=0;
	int i, old_chunk, chunk_offset, phy_block,logical_block;
	int chipnr, chipnr_remap;
	//__u8 oob_area[64];
	__u8 *oob_area, oob_area0[chunk_size+oob_size+32];
	u_char *buf=ops->datbuf;
	dbg_printf("%s(%d):\n",__FUNCTION__,__LINE__);




	oob_area = (__u8*) ((u32)(oob_area0 + 15) & 0xFFFFFFF0);


	memset(oob_area0, 0xff,chunk_size+oob_size+32);


	
	if ((to + ops->len) > mtd->size) {
		printk ("nand_pio_write: Attempt write beyond end of device\n");
		ops->retlen = 0;
		return -EINVAL;
	}

	if (NOTALIGNED (mtd, to)) {
		printk (KERN_NOTICE "nand_pio_write: Attempt to read not page aligned data. %llx\n", to);
		return -EINVAL;
	}

	dbg_printf("a pio write NAND :  ops.ooblen %d, ops.len %d, ops.datbuf 0x%p, ops.oobbuf 0x%p\n"
			,ops->ooblen,ops->len,ops->datbuf,ops->oobbuf );

	realchunk = (int)(to >> this->chunk_shift);
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

	thislen=0;

	while (thislen < ops->len) {
		WATCHDOG_RESET();
		chunk_id = logical_block*ppb + chunk_offset;

		dbg_printf("[%s, line %d] chunk_id = %d, databuf = 0x%p thislen =%d \n",__FUNCTION__,__LINE__, chunk_id, &buf[thislen],thislen);
		check_ready();


		/*in uboot, first check the page are all 0xff, not write it. */
		
		if((memcmp(oob_area,&buf[thislen],chunk_size+oob_size)!=0)){
					parameters._nand_pio_write(chunk_id,chunk_size+oob_size,&buf[thislen]);
		}else{
			printk("find pio write to chunk %d is all 0xff\n",chunk_id);
		}

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
**  create_logical_skip_bbt()
**	descriptions: create logical block for all good block.
**	parameters: mtd, ofs: flash address
**	return:  1: bad , 0: good
**  note: 
**/


int create_logical_skip_bbt(struct mtd_info *mtd)
{

	unsigned int good_num=0; 
	int j;
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	unsigned int bbi_block;

	//this->bbt default all 0.
	printf("create_logical_skip_bbt\n");

	
	for (bbi_block=0;bbi_block<nandflash_info.num_block;bbi_block++) {
//	for (bbi_block=0;bbi_block<20;bbi_block++) {


		j=_get_flags(parameters.bbt, bbi_block);

		if(j==0){ //good block				
			this->bbt[good_num]=bbi_block-good_num;
			good_num++;	
	
		}else{
			printf("detect block %d is bad \n",bbi_block);
		}

	}
	printf("last skip_block %d\n",good_num);
	for (good_num;good_num<nandflash_info.num_block;good_num++){
		this->bbt[good_num]=0xff;		
	}	

	//debug message 

//	for (good_num=0;good_num<20;good_num++){
//		printf("this->bbt[%d]= %d\n",good_num,this->bbt[good_num]);
	//}	


	
}



/*---------------preload function test --------------------*/


static void
create_bbt(nand_dma_buf_t *dma_buf) {
    u32_t b, ecc;
    u8_t bbi;

	printk("u-boot create bbt\n");
	memset(parameters.bbt, 0x0, NUM_WORD(MAX_BLOCKS));
    for (b=1;b<nandflash_info.num_block;++b) {
		
	bbi=parameters._nand_read_bbi(nandflash_info.bbi_raw_offset,b*num_chunk_per_block);
        if (bbi==0) continue;
		printk("1 bad block[%d], page %d , bbi %x\n",b,b*num_chunk_per_block,bbi);
		if ((ecc=parameters._nand_read_chunk(dma_buf->chunk_buf, &dma_buf->spare, b*num_chunk_per_block))<0) {
		
			
			printk("read fail, set block %d is bad \n",b);
            _set_flags(parameters.bbt, b);
            continue;
        }
        u8_t oob_bbi=*pbldr_spare_bbi(&(dma_buf->spare));
		printk("oob_bbi is %x\n",oob_bbi);
        if (oob_bbi==0xFF) continue;
        if ((oob_bbi==0x00) && (dma_buf->spare.oob.signature==OOB_SIG_RESERVED)) continue;
 
        // mark bad block
        printk("2 bad block %d b is bad \n",b);
        _set_flags(parameters.bbt, b);
    }
    parameters.bbt_valid=1;
	printk("finish create_bbt\n");
}



void rtk_scan_bbt(){

	u8_t *_dma_buf=malloc(sizeof(nand_dma_buf_t)+CACHELINE_SIZE-4);
	
    nand_dma_buf_t *dma_buf=(nand_dma_buf_t *)CACHE_ALIGN(_dma_buf);

	printf("parameters.bbt_valid is %d\n",parameters.bbt_valid);
	if(parameters.bbt_valid!=1)
		create_bbt(dma_buf);



}



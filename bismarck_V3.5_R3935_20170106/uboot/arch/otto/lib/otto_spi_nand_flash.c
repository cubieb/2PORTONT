#include <common.h>
#include <soc.h>
#include <asm/symb_define.h>
#include <asm/otto_util.h>
#include <asm/otto_spi_nand_flash.h>
#include <spi_nand/spi_nand_blr_util.h>

#define MAX_BLOCKS (1024<<2)
uint8_t bb_skip_table[MAX_BLOCKS];


#define EBUF_SIZE                   (MAX_ECC_BUF_SIZE)
#define PBUF_SIZE                   (MAX_PAGE_BUF_SIZE+MAX_OOB_BUF_SIZE)

u_char _tmp_ecc[EBUF_SIZE] DATA_ALIGNED32;
u_char _tmp[PBUF_SIZE] DATA_ALIGNED32;

#define DEV_ID_LEN_BITS(dev_id) ((dev_id > 0xff)?16:8)

#define OTTO_TEMP_SYM_HEADER ((const symbol_table_entry_t *)(((soc_t *)(0x9fc00020))->bios.header.export_symb_list))
#define OTTO_TEMP_SYM_END    ((const symbol_table_entry_t *)(((soc_t *)(0x9fc00020))->bios.header.end_of_export_symb_list))

// Assumption: 2CS spi-nand will use same model
_spi_nand_info_t _info;

#if OTTO_SPI_NAND_GEN > 1
fpv_u32_t                       *_nsu_reset_ptr;
fpv_u32_t                       *_nsu_en_on_die_ecc_ptr;
fpv_u32_t                       *_nsu_dis_on_die_ecc_ptr;
fpv_u32_t                       *_nsu_blk_unprotect_ptr;
spi_nand_get_feature_reg_t      *_nsu_get_feature_ptr;
spi_nand_set_feature_reg_t      *_nsu_set_feature_ptr;
spi_nand_read_id_t              *_nsu_read_id_ptr;

void spi_nand_retrieve_func(void) {
    symb_retrive_entry_t spi_nand_func_retrive_list[] = {
        {SNAF_RESET_SPI_NAND_FUNC, &_nsu_reset_ptr},
        {SNAF_SET_FEATURE_FUNC, &_nsu_set_feature_ptr},
        {SNAF_GET_FEATURE_FUNC, &_nsu_get_feature_ptr},
        {SNAF_READ_SPI_NAND_FUNC, &_nsu_read_id_ptr},
        {SNAF_DISABLE_ODE_FUNC, &_nsu_dis_on_die_ecc_ptr},
        {SNAF_ENABLE_ODE_FUNC, &_nsu_en_on_die_ecc_ptr},
        {SNAF_BLOCK_UNPROTECT_FUNC, &_nsu_blk_unprotect_ptr},
        {ENDING_SYMB_ID, VZERO},
    };
    symb_retrive_list(spi_nand_func_retrive_list, OTTO_TEMP_SYM_HEADER, OTTO_TEMP_SYM_END);
}
#endif

uint32_t spi_nand_chip_size(void) {
    return _info.chip_size;
}

uint32_t spi_nand_block_size(void) {
    return _info.block_size;
}

uint32_t spi_nand_page_size(void) {
    return _info.page_size;
}

uint32_t spi_nand_spare_size(void) {
    return _info.spare_size;
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



static void create_bbt(u32_t *bbt_table)
{
    uint8_t bb_tag[4];
    uint32_t blk_num;
    uint32_t block_count = _info.block_count;
    int ret;

    printf("uboot: create bbt\n");       

    for ( blk_num = 1; blk_num < block_count; blk_num++) {
        nasu_pio_read(bb_tag, 4, blk_num<<6, _info.page_size);
        if (0xFF == bb_tag[0]) {
            continue;
        }
        ret = nasu_page_read_ecc(_tmp, blk_num<<6, _tmp_ecc);
        if (ECC_CTRL_ERR==(ret&0xFFFFFF00)) {
            // ecc error, mark it as a bad block
        } else {
            if (0xFF == _tmp[_info.page_size]) {
                continue;
            }
        }

        printf("uboot: bb %d\n", blk_num);       


        _set_flags(bbt_table, blk_num);
    }
}

static void create_skip_table(uint32_t *bbt_table){
    unsigned int good_num; 
    int j;
    unsigned int bbi_block;
    unsigned int skip_block;
    uint8_t *skip_table = bb_skip_table;



    printf("uboot: create skip table\n");       
    skip_table[0] = 0;

    skip_block = good_num = bbi_block = 1;
    for (;bbi_block<_info.block_count;bbi_block++) {
        j=_get_flags(bbt_table, bbi_block);

        if(j==0){ //good block

            skip_table[skip_block]=bbi_block-good_num;
            good_num++;	
            skip_block++;			
        }else{
            //printf("detect block %d is bad \n",bbi_block);
            if(bbi_block-good_num == 0xFF){
                break;
            }
        }
    }
    printf("last skip_block %d\n",skip_block);
    for (;skip_block<_info.block_count;skip_block++){
        skip_table[skip_block]=0xff;		
    }	
}

static int get_good_block(u32_t blk_num, u32_t *mapped_blk_num)
{
    u32_t skip_num = bb_skip_table[blk_num];
    if( skip_num == 0xFF){
        return -1;
    }
    *mapped_blk_num = blk_num + skip_num;
    //printf("get good block %d => %d\n", blk_num, *mapped_blk_num);
    return 0;
}


void spi_nand_init(void) {
    u32_t _bbt_table[NUM_WORD(MAX_BLOCKS)];
    spi_nand_flash_info_t *fi;
    uint32_t max=1;
#if OTTO_SPI_NAND_GEN > 1
    spi_nand_retrieve_func();

    /* Initial rest chip(s) */
    typedef int (init_rest_func_t)(void);
    extern init_rest_func_t *LS_start_of_snaf_init_rest_func, *LS_end_of_snaf_init_rest_func;
    
    init_rest_func_t **init_rest_func = &LS_start_of_snaf_init_rest_func;
    while (init_rest_func!=&LS_end_of_snaf_init_rest_func) {
        if(0!=(*init_rest_func)()) { max++; break;}
        ++init_rest_func;
    }
#endif
    
    /* fill spi nand info */
    fi = _plr_spi_nand_info;

    _info.id = (fi->man_id << DEV_ID_LEN_BITS(fi->dev_id)) | fi->dev_id;
    _info.cs_count=max;
    _info.chip_size=SNAF_NUM_OF_BLOCK(fi)
                   *SNAF_NUM_OF_PAGE_PER_BLK(fi)
                   *SNAF_PAGE_SIZE(fi);
    _info.block_size=SNAF_NUM_OF_PAGE_PER_BLK(fi)
                   *SNAF_PAGE_SIZE(fi);
    _info.page_size=SNAF_PAGE_SIZE(fi);
    _info.oob_size=SNAF_OOB_SIZE(fi);
    _info.spare_size=SNAF_SPARE_SIZE(fi);
    _info.block_count=SNAF_NUM_OF_BLOCK(fi);
    _info.page_count=SNAF_NUM_OF_PAGE_PER_BLK(fi);

    
    printf("SPI-NAND Flash: %uMB*%d, ID:%x\n", spi_nand_chip_size()>>20, max, _info.id);

    memset(_bbt_table, '\0', sizeof(_bbt_table));
    if(_info.block_count > MAX_BLOCKS){
        printf("SPI-NAND Flash: Create Bad Block Table Error!\n Block Number is More than %d!\n",  MAX_BLOCKS);
    }else{
        create_bbt(_bbt_table);
        create_skip_table(_bbt_table);
    }
}

void spi_nand_info(void) {
  	printf("Device:\n SPI-NAND, ID:%x, size:%u MB*%d \n",
            _info.id, spi_nand_chip_size()>>20, _info.cs_count);
    printf(" each chip has       %8d block(s)\n", _info.block_count);
    printf(" each block has      %8d page(s)\n", _info.page_count);
    printf(" page size           %8d byte(s)\n", _info.page_size);
    printf(" erase size          %8d byte(s)\n", _info.block_size);
    printf(" spare size          %8d byte(s)\n", _info.spare_size);
    printf("     oob size        %8d byte(s)\n", _info.oob_size);
    printf("     others          %8d byte(s)\n", _info.spare_size-_info.oob_size);
}

#define BLK_IDX(off, bsize)         (off/bsize)
#define PAG_IDX(off, bsize, psize)  ((off%bsize)/psize)
#define COL_OFF(off, psize)         (off%psize)

#define BSIZE                       (_info.block_size)
#define PSIZE                       (_info.page_size)
#define SSIZE                       (_info.spare_size)
#define BCNT                        (_info.block_count)
#define PCNT                        (_info.page_count)


int _spi_nand_check_ecc(int r) {
    if (ECC_CTRL_ERR==(r&0xFFFFFF00)) {
        printf("\nWW: ecc check failed (ret 0x%x)\n", r);
        return -1;
    } else {
        return 0;
    }
}
    
int spi_nand_read_write(uint32_t offset, uint32_t length, u_char *buffer, char opt) {
    uint32_t bi, pi, co, r_bi;
    uint32_t size = length, l;
    
    // start addr
    bi = BLK_IDX(offset, PCNT*(PSIZE+SSIZE));
    pi = PAG_IDX(offset, PCNT*(PSIZE+SSIZE), (PSIZE+SSIZE));
    co = COL_OFF(offset, (PSIZE+SSIZE));
        
		
    //printf("DD: start from blk_0x%x, pg_0x%x, col_0x%x, size %d to buffer(0x%x)\n", 
    //    bi, pi, co, size, buffer);

    while (size>0) {
        if(-1 == get_good_block(bi, &r_bi)){
            puts("\nEE: block error!\n");
            return -1;
        }
        // check column offset
        if (0<co) {
            puts("\nEE: column address incorrect!\n");
            return -1;
        } else if (0==co && size>(PSIZE+SSIZE)) {
            l=PSIZE+SSIZE;
            (opt)?nasu_page_write(buffer, BLOCK_PAGE_ADDR(r_bi, pi))
                :nasu_page_read(buffer, BLOCK_PAGE_ADDR(r_bi, pi));
        } else {
            //l=(size>((PSIZE+SSIZE)-co))?((PSIZE+SSIZE)-co):size;
            l=size;
            (opt)?nasu_pio_write(buffer, l, BLOCK_PAGE_ADDR(r_bi, pi), co)
                :nasu_pio_read(buffer, l, BLOCK_PAGE_ADDR(r_bi, pi), co);            
        }
        //printf("DD: nasu_pio_%s buf 0x%x, len %d, 0x%x (b %d(%d), p %d), col 0x%x)\n", 
        //    (opt)?"write":"read", buffer, l, BLOCK_PAGE_ADDR(r_bi, pi), r_bi, bi, pi, co);
        size-=l;
        co=0;
        buffer+=l;
        if (++pi >= PCNT) {  // block idx increased
            bi++;
            pi=0;
        }
        printf("%03d%%\b\b\b\b", 100-(100*size)/length);
    }
    return 0;   
}

int spi_nand_write_ecc(uint32_t offset, uint32_t length, u_char *buffer) {
    uint32_t bi, pi, co, r_bi;
    uint32_t size = length, l;
    
    // start addr
    bi = BLK_IDX(offset, BSIZE);
    pi = PAG_IDX(offset, BSIZE, PSIZE);
    co = COL_OFF(offset, PSIZE);

    while (size>0) {
        if(-1 == get_good_block(bi, &r_bi)){
            puts("\nEE: block error!\n");
            return -1;
        }
        memset(_tmp+_info.page_size, 0xFF, PBUF_SIZE-_info.page_size);                
        // check column offset
        if (0<co) {
            // read first
            //ret=nasu_page_read_ecc(_tmp, BLOCK_PAGE_ADDR(r_bi, pi), _tmp_ecc);
            //if (_spi_nand_check_ecc(ret)) return -1;
                
            l=(size>(PSIZE-co))?(PSIZE-co):size;
            
        } else if (0==co) {
            if (size>=PSIZE) {
                l=PSIZE;
            } else {
                //ret=nasu_page_read_ecc(_tmp, BLOCK_PAGE_ADDR(r_bi, pi), _tmp_ecc);
                //if (_spi_nand_check_ecc(ret)) return -1;
                l=size;
            }
        } else {
            puts("\nEE: column address incorrect!\n");
            return -1;
        }
        memcpy(_tmp+co, buffer, l);
#if 0
        ret=nasu_page_write_ecc(_tmp, BLOCK_PAGE_ADDR(r_bi, pi), _tmp_ecc);
#else  
        nasu_ecc_encode(_tmp, _tmp_ecc);
        nasu_page_write(_tmp, BLOCK_PAGE_ADDR(r_bi, pi));
#endif 
        size-=l;
        co=0;
        buffer+=l;
        if (++pi >= PCNT) {  // block idx increased
            bi++;
            pi=0;
        }
        //printf("DD: spi_nand_write_ecc(0x%x, %d, 0x%x)\n", 
        //        buffer, l, BLOCK_PAGE_ADDR(r_bi, pi), co);
    }
    return 0;
}

int spi_nand_read_ecc(uint32_t offset, uint32_t length, u_char *buffer) {
    uint32_t bi, pi, co, r_bi;
    uint32_t size = length, l;
    int ret;
    
    // start addr
    bi = BLK_IDX(offset, BSIZE);
    pi = PAG_IDX(offset, BSIZE, PSIZE);
    co = COL_OFF(offset, PSIZE);

    while (size>0) {
        if(-1 == get_good_block(bi, &r_bi)){
            puts("\nEE: block error!\n");
            return -1;
        }
        memset(_tmp, 0, PBUF_SIZE);
        ret=nasu_page_read_ecc(_tmp, BLOCK_PAGE_ADDR(r_bi, pi), _tmp_ecc);
        if (_spi_nand_check_ecc(ret)) return -1;
        
        // check column offset
        if (0<co) {
            l=(size>(PSIZE-co))?(PSIZE-co):size;
        } else if (0==co) {
            l=(size>PSIZE)?PSIZE:size;
        } else {
            puts("\nEE: column address incorrect!\n");
            return -1;
        }
        memcpy(buffer, _tmp+co, l);        
        size-=l;
        co=0;
        buffer+=l;
        if (++pi >= PCNT) {  // block idx increased
            bi++;
            pi=0;
        }
    }
    return 0;
}

int spi_nand_erase(uint32_t offset, uint32_t length) {
    uint32_t bi, cnt=0, r_bi;
    int size = length;
    
    bi = BLK_IDX(offset, BSIZE);

    //printf("DD: block erase from 0x%x (block %d).\n", offset, blk);
    while (size>0) {
        if(-1 == get_good_block(bi, &r_bi)){
            puts("\nEE: block error!\n");
            return -1;
        }
        //r_bi = bi; // for raw erase test 

        //printf("DD: block erase %d(%d) block (0x%x)\n", r_bi, bi, BLOCK_PAGE_ADDR(r_bi, 0));
        if (-1==nasu_block_erase(BLOCK_PAGE_ADDR(r_bi, 0))) {
            puts("\nEE: erase failed!\n");
            return -1;
        }
        size-=BSIZE;
        cnt++;
        bi++; 
    }
    //printf("from blk #%d size 0x%x is erased\n", BLK_IDX(offset, BSIZE), length);
    printf("%d block(s) erased \n", cnt);
    return 0;
}

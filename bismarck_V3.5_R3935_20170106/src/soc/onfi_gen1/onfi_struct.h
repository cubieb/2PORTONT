#ifndef _ONFI_STRUCT_H_
#define _ONFI_STRUCT_H_
#include <ecc/ecc_ctrl.h>


typedef struct onfi_info_s onfi_info_t;
typedef struct onfi_model_info_s onfi_model_info_t;

/* --------------------------------------------
  ONFI FLASH PARAMETERS
-------------------------------------------- */
typedef void  (onfi_pio_read_t)(onfi_info_t *info, void *ram_addr,   u32_t wr_bytes, u32_t blk_page_idx, u32_t col_addr);
typedef s32_t (onfi_pio_write_t)(onfi_info_t *info, void *ram_addr,  u32_t wr_bytes, u32_t blk_page_idx, u32_t col_addr);
typedef void  (onfi_page_read_t)(onfi_info_t *info, void *ram_addr,  u32_t blk_page_idx);
typedef s32_t (onfi_page_write_t)(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx);
typedef s32_t (onfi_page_read_write_ecc_t)(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx, void *p_eccbuf);
typedef s32_t (onfi_block_erase_t) (onfi_info_t *info, u32_t blk_page_idx);
typedef u8_t  (onfi_get_sts_reg_t)(void);
typedef onfi_info_t* (onfi_probe_t) (onfi_info_t *info);


struct onfi_model_info_s{
    onfi_pio_read_t            *_pio_read;
    onfi_pio_write_t           *_pio_write;
    onfi_page_read_t           *_page_read;
    onfi_page_write_t          *_page_write;
    onfi_page_read_write_ecc_t *_page_read_ecc;
    onfi_page_read_write_ecc_t *_page_write_ecc;
    onfi_block_erase_t         *_block_erase;
    fpv_t                      *_wait_onfi_rdy;
};

struct onfi_info_s {
    u16_t        man_id;                //Manufacture id
    u16_t        dev_id;                //Device id
    u32_t        _num_block:4;          // num_block = 512*_num_block;
    u32_t        _num_page_per_block:5; // num_chunk_per_block = 32*_num_page_per_block;
    u32_t        _page_size:4;          //chunk_size = 2048*_chunk_size;
    u32_t        _spare_size:8;         //spare_size = 4*_spare_size;
    u32_t        _oob_size:7;           //oob_size = 4*_oob_size;
    u32_t        _ecc_ability:4;

    ecc_encode_t           *_ecc_encode;
    ecc_decode_t           *_ecc_decode;
    fpv_t                  *_reset;   
    onfi_model_info_t  *_model_info;

    u32_t  bs_page_size:14;
    u32_t  bs_cmd_cycle:4;
    u32_t  bs_addr_cycle:4;
    u32_t  bs_mbz:10;
};

#define ONFI_UNIT_NUM_BLK    (512)
#define ONFI_UNIT_NUM_PAGE   (32)
#define ONFI_UNIT_PAGE_SIZE  (1024)
#define ONFI_UNIT_SPARE_SIZE (32)
#define ONFI_UNIT_OOB_SIZE   (4)

#define ONFI_MODEL_NUM_BLK_512          (0) //512*(1<<0)
#define ONFI_MODEL_NUM_BLK_1024         (1) //512*(1<<1)
#define ONFI_MODEL_NUM_BLK_2048         (2) //512*(1<<2)
#define ONFI_MODEL_NUM_BLK_4096         (3) //512*(1<<3)
#define ONFI_MODEL_NUM_BLK_8192         (4) //512*(1<<3)

#define ONFI_MODEL_NUM_PAGE_32          (0) // 32*(1<<0)
#define ONFI_MODEL_NUM_PAGE_64          (1) // 32*(1<<1)
#define ONFI_MODEL_NUM_PAGE_128         (2) // 32*(1<<2)
#define ONFI_MODEL_NUM_PAGE_256         (3) // 32*(1<<3)
#define ONFI_MODEL_NUM_PAGE_512         (4) // 32*(1<<4)
#define ONFI_MODEL_NUM_PAGE_1024        (5) // 32*(1<<5)

#define ONFI_MODEL_PAGE_SIZE_1024B      (0) //1024*(1<<0)
#define ONFI_MODEL_PAGE_SIZE_2048B      (1) //1024*(1<<1)
#define ONFI_MODEL_PAGE_SIZE_4096B      (2) //1024*(1<<2)
#define ONFI_MODEL_PAGE_SIZE_8192B      (3) //1024*(1<<2)

#define ONFI_MODEL_SPARE_SIZE_32B       (0) //  32*(1<<0)
#define ONFI_MODEL_SPARE_SIZE_64B       (1) //  32*(1<<1)
#define ONFI_MODEL_SPARE_SIZE_128B      (2) //  32*(1<<2)
#define ONFI_MODEL_SPARE_SIZE_256B      (3) //  32*(1<<3)
#define ONFI_MODEL_SPARE_SIZE_512B      (4) //  32*(1<<4)
#define ONFI_MODEL_SPARE_SIZE_1024B     (5) //  32*(1<<5)

#define ONFI_MODEL_OOB_SIZE(oob_byte)   (((oob_byte)/ONFI_UNIT_OOB_SIZE)-1)


#define ONFI_NUM_OF_PAGE_PER_BLK(info)  ((1<<info->_num_page_per_block) * ONFI_UNIT_NUM_PAGE)
#define ONFI_NUM_OF_BLOCK(info)         ((1<<info->_num_block)* ONFI_UNIT_NUM_BLK)
#define ONFI_PAGE_SIZE(info)            ((1<<info->_page_size) * ONFI_UNIT_PAGE_SIZE)
#define ONFI_SPARE_SIZE(info)           ((1<<info->_spare_size) * ONFI_UNIT_SPARE_SIZE)
#define ONFI_OOB_SIZE(info)             ((info->_oob_size+1) * ONFI_UNIT_OOB_SIZE)

/**********************************************/
/****** Paralle NAND Boot-strap Pin Translation ******/
/**********************************************/
#define ONFI_BS_PAGE_SIZE(onaf_rcmd_cle)  ((onaf_rcmd_cle==0)?512:((1<<onaf_rcmd_cle)*1024))
#define ONFI_BS_ADDR_CYCLE(onaf_addr_cle) ((onaf_addr_cle==3)?0:(onaf_addr_cle+3))
#define ONFI_BS_CMD_CYCLE(onaf_rcmd_cle)  ((onaf_rcmd_cle==0)?1:2)


#endif


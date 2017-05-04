#ifndef SPI_NAND_STRUCT_H
#define SPI_NAND_STRUCT_H

#include <ecc/ecc_ctrl.h>

/***********************************************
  * Driver Section Definitions
  ***********************************************/
#ifndef SECTION_SPI_NAND
    #define SECTION_SPI_NAND 
#endif
#ifndef SECTION_SPI_NAND_DATA
    #define SECTION_SPI_NAND_DATA 
#endif


struct  spi_nand_flash_info_s;
typedef struct spi_nand_flash_info_s spi_nand_flash_info_t;
/*********************************************
  * SPI NAND FLASH PARAMETERS
  * The latest general spi nand flash & ecc driver
  *********************************************/
typedef void (spi_nand_pio_read_t)(spi_nand_flash_info_t *info, void *addr, u32_t wr_bytes, u32_t blk_pge_addr, u32_t col_addr);
typedef s32_t (spi_nand_pio_write_t)(spi_nand_flash_info_t *info, void *addr, u32_t wr_bytes, u32_t blk_pge_addr, u32_t col_addr);
typedef void (spi_nand_page_read_t)(spi_nand_flash_info_t *info, void *dma_addr, u32_t blk_pge_addr);
typedef s32_t (spi_nand_page_write_t)(spi_nand_flash_info_t *info, void *dma_addr, u32_t blk_pge_addr);
typedef s32_t (spi_nand_page_read_write_ecc_t)(spi_nand_flash_info_t *info, void *dma_addr, u32_t blk_pge_addr, void *p_eccbuf);
typedef s32_t (spi_nand_erase_block_t) (spi_nand_flash_info_t *info, u32_t blk_pge_addr);
typedef s32_t (spi_nand_chk_eccStatus_t)(void);
typedef spi_nand_flash_info_t* (spi_nand_probe_t) (void);
typedef u32_t (spi_nand_get_feature_reg_t) (u32_t feature_addr);
typedef void (spi_nand_set_feature_reg_t) (u32_t feature_addr, u32_t setting);
typedef u32_t (spi_nand_read_id_t) (u32_t man_addr, u32_t w_io_len, u32_t r_io_len);


typedef struct spi_nand_cmd_info_s{
    u8_t w_cmd;
    u8_t w_addr_io;
    u8_t w_data_io;
    u8_t r_cmd;
    u8_t r_addr_io;
    u8_t r_data_io;
    u8_t r_dummy_cycles;
}spi_nand_cmd_info_t;

typedef struct spi_nand_model_info_s{
    spi_nand_pio_read_t            *_pio_read;
    spi_nand_pio_write_t           *_pio_write;
    spi_nand_page_read_t           *_page_read;
    spi_nand_page_write_t          *_page_write;
    spi_nand_page_read_write_ecc_t *_page_read_ecc;
    spi_nand_page_read_write_ecc_t *_page_write_ecc;
    spi_nand_erase_block_t         *_block_erase;
    fpv_t                          *_wait_spi_nand_ready;
}spi_nand_model_info_t;

struct spi_nand_flash_info_s {
    u16_t        man_id;               //Manufacture id
    u16_t        dev_id;               //Device id
    u32_t        _num_block:4;         //num_block = 512*(_num_block+1); (512~8192)
    u32_t        _num_page_per_block:5;//num_chunk_per_block = 32*(_num_page_per_block+1); (32~1024)
    u32_t        _page_size:4;         //chunk_size = 1024*(_page_size+1); (1024~16384)
    u32_t        _spare_size:8;        //spare_size = 4*(_spare_size+1); (4~1024)
    u32_t        _oob_size:7;          //oob_size = 4*(_oob_size+1); (4~512)
    u32_t        _ecc_ability:4;       //bch_ability = 6*(_ecc_ablility+1); (6~96)
    ecc_encode_t *_ecc_encode;
    ecc_decode_t *_ecc_decode;
    fpv_t                  *_reset;
    spi_nand_cmd_info_t    *_cmd_info;
    spi_nand_model_info_t  *_model_info;
};

#define UNIT_NUM_BLK    (512)
#define UNIT_NUM_PAGE   (32)
#define UNIT_PAGE_SIZE  (1024)
#define UNIT_SPARE_SIZE (4)
#define UNIT_OOB_SIZE   (4)

#define SNAF_MODEL_NUM_BLK_512          ((512/UNIT_NUM_BLK)-1)
#define SNAF_MODEL_NUM_BLK_1024         ((1024/UNIT_NUM_BLK)-1)
#define SNAF_MODEL_NUM_BLK_2048         ((2048/UNIT_NUM_BLK)-1)
#define SNAF_MODEL_NUM_PAGE_32          ((32/UNIT_NUM_PAGE)-1)
#define SNAF_MODEL_NUM_PAGE_64          ((64/UNIT_NUM_PAGE)-1)
#define SNAF_MODEL_NUM_PAGE_128         ((128/UNIT_NUM_PAGE)-1)
#define SNAF_MODEL_PAGE_SIZE_1024B      ((1024/UNIT_PAGE_SIZE)-1)
#define SNAF_MODEL_PAGE_SIZE_2048B      ((2048/UNIT_PAGE_SIZE)-1)
#define SNAF_MODEL_PAGE_SIZE_4096B      ((4096/UNIT_PAGE_SIZE)-1)
#define SNAF_MODEL_SPARE_SIZE_64B       ((64/UNIT_SPARE_SIZE)-1)
#define SNAF_MODEL_SPARE_SIZE_128B      ((128/UNIT_SPARE_SIZE)-1)
#define SNAF_MODEL_SPARE_SIZE_256B      ((256/UNIT_SPARE_SIZE)-1)
#define SNAF_MODEL_OOB_SIZE(oob_byte)   (((oob_byte)/UNIT_OOB_SIZE)-1)

#define SNAF_NUM_OF_PAGE_PER_BLK(info)  ((info->_num_page_per_block+1) * UNIT_NUM_PAGE)
#define SNAF_NUM_OF_BLOCK(info)         ((info->_num_block+1)* UNIT_NUM_BLK)
#define SNAF_PAGE_SIZE(info)            ((info->_page_size+1) * UNIT_PAGE_SIZE)
#define SNAF_SPARE_SIZE(info)           ((info->_spare_size+1) * UNIT_SPARE_SIZE)
#define SNAF_OOB_SIZE(info)             ((info->_oob_size+1) * UNIT_OOB_SIZE)
#define SNAF_RW_CMD(info)               info->_cmd_info

#define ELEMENT_OF_SNAF_INFO(info)       sizeof(info)/sizeof(spi_nand_flash_info_t)
#endif //SPI_NAND_STRUCT_H


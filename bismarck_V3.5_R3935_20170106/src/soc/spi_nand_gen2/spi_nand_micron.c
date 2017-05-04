#include <spi_nand/spi_nand_ctrl.h>
#include <spi_nand/spi_nand_common.h>
#include <stddef.h>
#ifdef CONFIG_UNDER_UBOOT
#include <spi_nand/spi_nand_blr_util.h>
#include <spi_nand/spi_nand_symb_func.h>
#else	// CONFIG_UNDER_UBOOT
#include <util.h>
#include <spi_nand/spi_nand_util.h>
#endif	// CONFIG_UNDER_UBOOT

/***********************************************
  *  Micron's ID Definition
  ***********************************************/
#define MID_MICRON      (0x2C)
#define DID_M78A        (0x14)


#ifdef NSU_DRIVER_IN_ROM
    #include <arch.h>
    #define __SECTION_INIT_PHASE SECTION_SPI_NAND
    #define __SECTION_INIT_PHASE_DATA SECTION_RO
    #define __SECTION_RUNTIME SECTION_SPI_NAND
    #define __SECTION_RUNTIME_DATA SECTION_RO
    #if defined(NSU_PROHIBIT_QIO) || defined(NSU_PROHIBIT_DIO)
        #error 'lplr should not run at ...'
    #endif
    #ifdef IS_RECYCLE_SECTION_EXIST
        #error 'lplr should not have recycle section ...'
    #endif
    #define __DEVICE_USING_SIO 1
    #define __DEVICE_USING_DIO 0
    #define __DEVICE_USING_QIO 0
#else
    #ifdef NSU_USING_SYMBOL_TABLE_FUNCTION
        #define __DEVICE_REASSIGN 1
    #endif
    #ifdef IS_RECYCLE_SECTION_EXIST
        #define __SECTION_INIT_PHASE        SECTION_RECYCLE
        #define __SECTION_INIT_PHASE_DATA   SECTION_RECYCLE_DATA
        #define __SECTION_RUNTIME           SECTION_UNS_TEXT
        #define __SECTION_RUNTIME_DATA      SECTION_UNS_RO
    #else
        #define __SECTION_INIT_PHASE
        #define __SECTION_INIT_PHASE_DATA
        #define __SECTION_RUNTIME
        #define __SECTION_RUNTIME_DATA
    #endif

    #ifdef NSU_MICRON_USING_QIO
        #if defined(NSU_PROHIBIT_QIO) && defined(NSU_PROHIBIT_DIO)
            #define __DEVICE_USING_SIO 1
            #define __DEVICE_USING_DIO 0
            #define __DEVICE_USING_QIO 0
        #elif defined(NSU_PROHIBIT_QIO) 
            #define __DEVICE_USING_SIO 0
            #define __DEVICE_USING_DIO 1
            #define __DEVICE_USING_QIO 0
        #else
            #define __DEVICE_USING_SIO 0
            #define __DEVICE_USING_DIO 0
            #define __DEVICE_USING_QIO 1
        #endif
    #elif defined(NSU_MICRON_USING_DIO)
        #if defined(NSU_PROHIBIT_DIO)
            #define __DEVICE_USING_SIO 1
            #define __DEVICE_USING_DIO 0
            #define __DEVICE_USING_QIO 0
        #else
            #define __DEVICE_USING_SIO 0
            #define __DEVICE_USING_DIO 1
            #define __DEVICE_USING_QIO 0
        #endif
    #else
        #define __DEVICE_USING_SIO 1
        #define __DEVICE_USING_DIO 0
        #define __DEVICE_USING_QIO 0
    #endif
#endif

#ifndef CONFIG_UNDER_UBOOT
#if __DEVICE_USING_DIO
__SECTION_INIT_PHASE_DATA spi_nand_cmd_info_t micron_dio_cmd_info;
#elif __DEVICE_USING_QIO
__SECTION_INIT_PHASE_DATA spi_nand_cmd_info_t micron_qio_cmd_info;
#endif


#define _dummy_ecc_encode ((ecc_encode_t*)&always_return_zero)

__SECTION_INIT_PHASE_DATA spi_nand_model_info_t micron_model = {
    ._page_read = micron_page_read,
    ._page_write = micron_page_write,
    ._page_read_ecc = micron_page_read_with_ecc_decode,
    ._page_write_ecc = micron_page_write_with_ecc_encode,
};

// code implement
__SECTION_INIT_PHASE_DATA 
spi_nand_flash_info_t micron_chip_info[] = {
    {
        .man_id              = MID_MICRON, 
        .dev_id              = DID_M78A,
        ._num_block          = SNAF_MODEL_NUM_BLK_1024,
        ._num_page_per_block = SNAF_MODEL_NUM_PAGE_64,
        ._page_size          = SNAF_MODEL_PAGE_SIZE_2048B,
        ._spare_size         = SNAF_MODEL_SPARE_SIZE_128B,
        ._oob_size           = SNAF_MODEL_OOB_SIZE(24),
        ._ecc_ability        = ECC_MODEL_12T,
        
        #if __DEVICE_REASSIGN
            ._ecc_encode     = VZERO,
            ._ecc_decode     = VZERO,
            ._reset          = VZERO,
            ._cmd_info       = VZERO,
            ._model_info     = VZERO,
        #elif __DEVICE_USING_SIO
            ._ecc_encode     = ecc_encode_bch,
            ._ecc_decode     = ecc_decode_bch,
            ._reset          = nsu_reset_spi_nand_chip,
            ._cmd_info       = &nsc_sio_cmd_info,
            ._model_info     = &micron_model,
        #elif __DEVICE_USING_DIO
            ._ecc_encode     = ecc_encode_bch,
            ._ecc_decode     = ecc_decode_bch,
            ._reset          = nsu_reset_spi_nand_chip,
            ._cmd_info       = &micron_dio_cmd_info,
            ._model_info     = &micron_model,
        #elif __DEVICE_USING_QIO
            ._ecc_encode     = ecc_encode_bch,
            ._ecc_decode     = ecc_decode_bch,
            ._reset          = nsu_reset_spi_nand_chip,
            ._cmd_info       = &micron_qio_cmd_info,
            ._model_info     = &micron_model,
        #endif
    },
};
#endif // #ifndef CONFIG_UNDER_UBOOT

__SECTION_INIT_PHASE int 
micron_ondie_ecc_decode_status(u32_t cs, u32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
    int eccsts = ((nsu_get_feature_reg(cs, 0, 0xC0)>>4)&0x7);
    if(eccsts == 2){
        return ECC_CTRL_ERR;
    }else{
        return eccsts;
    }
    return eccsts;

}
__SECTION_RUNTIME void 
micron_page_read(spi_nand_flash_info_t *info, void *dma_addr, u32_t blk_pge_addr)
{
    nsu_page_read(info, dma_addr, blk_pge_addr);
    inline_memcpy(dma_addr+0x846, dma_addr+0x804, 12);
    inline_memcpy(dma_addr+0x866, dma_addr+0x810, 8);
}

__SECTION_RUNTIME s32_t 
micron_page_write(spi_nand_flash_info_t *info, void *dma_addr, u32_t blk_pge_addr)
{
    inline_memcpy(dma_addr+0x846, dma_addr+0x804, 12);
    inline_memcpy(dma_addr+0x866, dma_addr+0x810, 8);
    return nsu_page_write(info, dma_addr, blk_pge_addr);
}

__SECTION_RUNTIME s32_t
micron_page_write_with_ecc_encode(spi_nand_flash_info_t *info, void *dma_addr, u32_t blk_pge_addr, void *p_eccbuf)
{
    return nsu_page_write(info, dma_addr, blk_pge_addr);
}
__SECTION_RUNTIME int 
micron_page_read_with_ecc_decode(spi_nand_flash_info_t *info, void *dma_addr, u32_t blk_pge_addr, void *p_eccbuf)
{
    nsu_page_read(info,dma_addr,blk_pge_addr);
    return micron_ondie_ecc_decode_status(info->_ecc_ability, NULL, NULL);
}

__SECTION_INIT_PHASE u32_t 
micron_read_id(u32_t cs)
{
    u32_t dummy_byte = 0x00;
    u32_t w_io_len = IO_WIDTH_LEN(SIO_WIDTH,CMR_LEN(2));
    u32_t r_io_len = IO_WIDTH_LEN(SIO_WIDTH,CMR_LEN(2));
    u32_t ret = nsu_read_spi_nand_id(cs, dummy_byte, w_io_len, r_io_len);
    return ((ret>>16)&0xFFFF);
}

#ifndef CONFIG_UNDER_UBOOT
__SECTION_INIT_PHASE spi_nand_flash_info_t *
probe_micron_spi_nand_chip(void)
{
    nsu_reset_spi_nand_chip(0);
    u32_t rdid = micron_read_id(0);
    if(MID_MICRON != (rdid >>8)) return VZERO;
    u16_t did = rdid&0xFF;
    u32_t i;
    for(i=0 ; i<ELEMENT_OF_SNAF_INFO(micron_chip_info) ; i++){
        if(micron_chip_info[i].dev_id == did){
            #ifdef __DEVICE_REASSIGN
            micron_model._pio_read= nsu_model_info._pio_read;
            micron_model._pio_write = nsu_model_info._pio_write;
            micron_model._block_erase = nsu_model_info._block_erase;
            micron_model._wait_spi_nand_ready = nsu_model_info._wait_spi_nand_ready;
            micron_chip_info[i]._ecc_encode = _dummy_ecc_encode;
            micron_chip_info[i]._ecc_decode = micron_ondie_ecc_decode_status;
            micron_chip_info[i]._cmd_info = _nsu_cmd_info_ptr;
            micron_chip_info[i]._reset = _nsu_reset_ptr;
            #endif
            nsc_block_unprotect(0);
            return &micron_chip_info[i];
        }
    }
    return VZERO;
}
REG_SPI_NAND_PROBE_FUNC(probe_micron_spi_nand_chip);
#else   //CONFIG_UNDER_UBOOT
int
micron_init_rest(void)
{
    u32_t cs=1;

    // reset
    nsu_reset_spi_nand_chip(cs);
    
    // check ID, cs0 and cs1 should be identical
    u32_t rdid = micron_read_id(cs);
    //u32_t cs0_id = (_spi_nand_info->man_id<<8) | _spi_nand_info->dev_id;
    if(((rdid>>8)!=MID_MICRON) || ((rdid&0xFF)!=_spi_nand_info->dev_id)) 
      { return 0; }
      
    // misc
    nsc_block_unprotect(cs);
    return 1;
}
REG_SPI_NAND_INIT_REST_FUNC(micron_init_rest);
#endif  //CONFIG_UNDER_UBOOT


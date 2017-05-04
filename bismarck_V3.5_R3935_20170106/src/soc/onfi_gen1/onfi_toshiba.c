#include <util.h>
#include <onfi/onfi_ctrl.h>
#include <onfi/onfi_common.h>
#include <onfi/onfi_util.h>

#define MID_DID_TC58BVG0S3HTA00 (0x98F18015)


#ifdef NSU_DRIVER_IN_ROM
    #include <arch.h>
    #define __SECTION_INIT_PHASE SECTION_ONFI
#define __SECTION_RUNTIME        SECTION_ONFI
    #ifdef IS_RECYCLE_SECTION_EXIST
        #error 'lplr should not have recycle section ...'
    #endif
#else
    #ifdef ONFI_USING_SYMBOL_TABLE_FUNCTION
        #define __DEVICE_REASSIGN 1
    #endif
    #ifdef IS_RECYCLE_SECTION_EXIST
        #define __SECTION_INIT_PHASE        SECTION_RECYCLE
        #define __SECTION_RUNTIME           SECTION_UNS_TEXT
    #else
        #define __SECTION_INIT_PHASE
        #define __SECTION_RUNTIME
    #endif
#endif

__SECTION_RUNTIME s32_t 
toshiba_onfi_ode_decode(u32_t ecc_ability, void *dma_addr, void *p_eccbuf)
{
    DEACTIVATE_CS0_CS1();
    NACMRrv = (CECS0|CMD_READ_ECC_STATUS);
    WAIT_ONFI_CTRL_READY();

    u8_t status = RFLD_NADR(data3);
    DEACTIVATE_CS0_CS1();
    return ((status==0xF)?ECC_USE_ODE_ERR:ECC_USE_ODE_SUCCESS);
}

__SECTION_INIT_PHASE onfi_info_t *
probe_toshiba_onfi_chip(onfi_info_t *info)
{
    u32_t rdid = onfi_read_id();
    if(MID_DID_TC58BVG0S3HTA00 == rdid){
        info->man_id = (rdid>>16)&0xFFFF;
        info->dev_id = rdid&0xFFFF;
        info->_ecc_ability = ECC_USE_ODE;
        info->_ecc_decode  = toshiba_onfi_ode_decode;
        info->_model_info->_page_read_ecc = onaf_page_read_with_ondie_ecc;
        info->_model_info->_page_write_ecc = onaf_page_write_with_ondie_ecc;
        return info;
    }
    return VZERO;
}

REG_ONFI_PROBE_FUNC(probe_toshiba_onfi_chip);



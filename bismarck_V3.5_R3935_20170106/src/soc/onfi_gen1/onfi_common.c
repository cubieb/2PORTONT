#include <util.h>
#include <onfi/onfi_ctrl.h>
#include <onfi/onfi_common.h>

SECTION_ONFI 
u32_t onfi_read_id(void)
{
    NACMRrv = (CECS0|CMD_READ_ID);

    WAIT_ONFI_CTRL_READY();

    NAADRrv = 0x0;
    NAADRrv = (0x0 |AD2EN|AD1EN|AD0EN);  // 1-dummy address byte
    WAIT_ONFI_CTRL_READY();

    u32_t id_chain = NADRrv;
    if(id_chain==0){
        //read flash fail.
        return -1;
    }

    //clear command/address register
    DEACTIVATE_CS0_CS1();
    NAADRrv = 0x0;

    return id_chain;
}

SECTION_ONFI 
void onfi_reset_nand_chip(void)
{
    NACMRrv = (CECS0|CMD_RESET);
}

SECTION_ONFI 
u8_t onfi_get_status_register(void)
{
    DEACTIVATE_CS0_CS1();
    NACMRrv = (CECS0|CMD_READ_STATUS);
    WAIT_ONFI_CTRL_READY();

    u8_t status = RFLD_NADR(data3);
    DEACTIVATE_CS0_CS1();
    return status;
}

SECTION_ONFI  
void onfi_wait_nand_chip_ready(void)
{
    while(1 != ((onfi_get_status_register()>>6)&0x1));
}

SECTION_ONFI 
s32_t onfi_check_program_erase_status(void)
{  
    while((onfi_get_status_register()&0x60) != 0x60);
    return ((onfi_get_status_register() & 0x01)==0)?0:-1;
}


SECTION_ONFI  
s32_t onfi_block_erase(onfi_info_t *info, u32_t blk_page_id)
{
    int addr_cycle[5],page_shift;
    int real_page = blk_page_id;
    
    if ( real_page & (ONFI_NUM_OF_PAGE_PER_BLK(info)-1) ){
        return -1;
    }

    NASRrv = (NWER|NRER|NDRS|NDWS);
    DEACTIVATE_CS0_CS1();

    //Command register , write erase command (1 cycle)
    NACMRrv = (CECS0|CMD_BLK_ERASE_C1);
    WAIT_ONFI_CTRL_READY();

    if(3 != info->bs_addr_cycle){
        for(page_shift=0; page_shift<3; page_shift++){
            addr_cycle[page_shift] = (real_page>>(8*page_shift)) & 0xff;
        }

        //NAND Flash Address Register1
        NAADRrv = ((~EN_NEXT_AD) & (AD2EN|AD1EN|AD0EN|(addr_cycle[0]<<CE_ADDR0) |(addr_cycle[1]<<CE_ADDR1)|(addr_cycle[2]<<CE_ADDR2)));        
    }else{
        addr_cycle[0] = 0;
        for(page_shift=0; page_shift<4; page_shift++){
            addr_cycle[page_shift+1] = (real_page>>(8*page_shift)) & 0xff;
        }
        NAADRrv = (((~EN_NEXT_AD) & AD2EN)|AD1EN|AD0EN|(addr_cycle[1]<<CE_ADDR0) |(addr_cycle[2]<<CE_ADDR1)|(addr_cycle[3]<<CE_ADDR2));
    }

    WAIT_ONFI_CTRL_READY();

    //write erase command cycle 2
    NACMRrv = (CECS0|CMD_BLK_ERASE_C2);
    WAIT_ONFI_CTRL_READY();

    info->_model_info->_wait_onfi_rdy();

    //read status
    return onfi_check_program_erase_status();
}


    
symb_fdefine(ONFI_READ_ID_FUNC, onfi_read_id);
symb_fdefine(ONFI_RESET_NAND_CHIP_FUNC, onfi_reset_nand_chip);
symb_fdefine(ONFI_BLOCK_ERASE_FUNC, onfi_block_erase);
symb_fdefine(ONFI_WAIT_NAND_CHIP_FUNC, onfi_wait_nand_chip_ready);
symb_fdefine(ONFI_GET_STATUS_FUNC, onfi_get_status_register);
symb_fdefine(ONFI_CHK_PROG_ERASE_STS_FUNC, onfi_check_program_erase_status);


#include <util.h>
#include <onfi/onfi_ctrl.h>
#include <onfi/onfi_common.h>
#include <ecc/ecc_ctrl.h>
#include <timer.h>

SECTION_ONFI 
int onaf_ctrl_init(void)
{
    /* Initialize hardware 
         * 1. clear NAND flash status register
         * 2. Disable parallel nand ctrl ecc
         */
    NASRrv = 0x0000000F;
    RMOD_NACR(ecc, 0);
    return 0;
}

SECTION_ONFI 
void onaf_pio_read_data(onfi_info_t *info, void *ram_addr, u32_t wr_bytes, u32_t blk_page_idx, u32_t col_addr)
{
	u32_t flash_addr1=0, flash_addr2=0;
	int page_size = info->bs_page_size;
    int spare_size = ONFI_SPARE_SIZE(info);
    int real_page = blk_page_idx+col_addr;
    int pio_length = wr_bytes;
    u32_t rlen, i;

    DEACTIVATE_CS0_CS1();
    WAIT_ONFI_CTRL_READY();
    union {
        u8_t u8[4];
        u32_t u32;
    } tmp;

	while(pio_length >0){
		if(pio_length > (page_size+spare_size)){
			rlen = (page_size+spare_size);
			pio_length -= (page_size+spare_size);
		}else{
			rlen = pio_length;
			pio_length -= rlen;
		}

        /* Command write cycle 1*/
        NACMRrv = (CECS0|CMD_PG_READ_C1);
        
        WAIT_ONFI_CTRL_READY();
        
        if(512 == page_size){
            flash_addr1 = ((real_page & 0xffffff) << 8);
        }else{
            flash_addr1 = ((real_page & 0xff) << 16);
            flash_addr2 = (real_page >> 8) & 0xffffff;
        }
        
        switch(info->bs_addr_cycle){
            case 3:
                NAADRrv = (AD2EN|AD1EN|AD0EN|flash_addr1);
                WAIT_ONFI_CTRL_READY();
                break;
            case 4:
                /* set address 1 */
                NAADRrv = (EN_NEXT_AD|AD2EN|AD1EN|AD0EN|flash_addr1);
                WAIT_ONFI_CTRL_READY();              
                NAADRrv = (AD0EN|flash_addr2);
                WAIT_ONFI_CTRL_READY();
                break;
            case 5:
                NAADRrv = (EN_NEXT_AD|AD2EN|AD1EN|AD0EN|flash_addr1);
                WAIT_ONFI_CTRL_READY();
                NAADRrv = (AD1EN|AD0EN|flash_addr2);
                WAIT_ONFI_CTRL_READY();
                break;
        }
        
        if(1 != info->bs_cmd_cycle){ //512 page size not need 'end' command
            /* Command cycle 2*/
            NACMRrv = (CECS0|CMD_PG_READ_C2);
            WAIT_ONFI_CTRL_READY();
        }

        otto_lx_timer_udelay(25);
        for(i=0; i<(rlen/4); i++){
            tmp.u32= NADRrv;
            *((u8_t *)ram_addr++) = tmp.u8[0];
            *((u8_t *)ram_addr++) = tmp.u8[1];
            *((u8_t *)ram_addr++) = tmp.u8[2];
            *((u8_t *)ram_addr++) = tmp.u8[3];
            WAIT_ONFI_CTRL_READY();
        }
        real_page++;
    }
    DEACTIVATE_CS0_CS1();
    WAIT_ONFI_CTRL_READY();
}

SECTION_ONFI  
s32_t onaf_pio_write_data(onfi_info_t *info, void *ram_addr, u32_t wr_bytes, u32_t blk_page_idx, u32_t col_addr)
{
	int i, rlen;
	u32_t flash_addr1=0, flash_addr2=0;
	int page_size= info->bs_page_size;
    int spare_size = ONFI_SPARE_SIZE(info);
    int real_page = blk_page_idx+col_addr;
	int pio_length = wr_bytes;

    //Disable Write Protection
    RMOD_NACFR(wp_b, 1);

	while(pio_length >0){
		if(pio_length > (page_size+spare_size)){
			rlen = (page_size+spare_size);
			pio_length -= (page_size+spare_size);
		}else{
			rlen = pio_length;
			pio_length -= rlen;
		}

		/* Command write cycle 1*/
        NACMRrv = (CECS0|CMD_PG_WRITE_C1);
		WAIT_ONFI_CTRL_READY();
		
		if(512 == page_size){
			flash_addr1 = ((real_page & 0xffffff) << 8);
		}else{
			flash_addr1 = ((real_page & 0xff) << 16);
			flash_addr2 = (real_page >> 8) & 0xffffff;
		}

		switch(info->bs_addr_cycle){
			case 3:
                NAADRrv = (AD2EN|AD1EN|AD0EN|flash_addr1);
				WAIT_ONFI_CTRL_READY();
				break;
			case 4:
                NAADRrv = (EN_NEXT_AD|AD2EN|AD1EN|AD0EN|flash_addr1);
                WAIT_ONFI_CTRL_READY();				
                NAADRrv = (AD0EN|flash_addr2);
                WAIT_ONFI_CTRL_READY();
				break;
			case 5:
                NAADRrv = (EN_NEXT_AD|AD2EN|AD1EN|AD0EN|flash_addr1);
				WAIT_ONFI_CTRL_READY();
                NAADRrv = (AD1EN|AD0EN|flash_addr2);
                WAIT_ONFI_CTRL_READY();
				break;
		}


		for(i=0; i<(rlen/4); i++){
            NADRrv = *(u32_t *)(ram_addr+i*4);
            WAIT_ONFI_CTRL_READY();
		}

        if(1 != info->bs_cmd_cycle){
            /* Command cycle 2*/
            NACMRrv = (CECS0|CMD_PG_WRITE_C2);
    		WAIT_ONFI_CTRL_READY();
        }
		real_page++;
	}

    return onfi_check_program_erase_status();
}


SECTION_ONFI 
void onaf_page_read(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx)
{
    u32_t read_len = info->bs_page_size + ONFI_SPARE_SIZE(info);
    onaf_pio_read_data(info, ram_addr, read_len, blk_page_idx, 0);
    _lplr_basic_io.dcache_writeback_invalidate_range((u32_t)ram_addr, (u32_t)(ram_addr+read_len));    
}

SECTION_ONFI 
s32_t onaf_page_write(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx)
{
    u32_t write_len = info->bs_page_size + ONFI_SPARE_SIZE(info);
    _lplr_basic_io.dcache_writeback_invalidate_range((u32_t)ram_addr, (u32_t)(ram_addr+write_len));    
    return onaf_pio_write_data(info, ram_addr, write_len, blk_page_idx, 0);
}


SECTION_ONFI 
s32_t onaf_page_read_with_ecc_decode(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx, void *p_eccbuf)
{
    onaf_page_read(info, ram_addr, blk_page_idx);
    return ecc_decode_bch_sector(ECC_CORRECT_BITS(info), ram_addr, p_eccbuf, ONFI_PAGE_SIZE(info)/BCH_SECTOR_SIZE);
}

SECTION_ONFI  
s32_t onaf_page_write_with_ecc_encode(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx, void *p_eccbuf)
{
    ecc_encode_bch_sector(ECC_CORRECT_BITS(info), ram_addr, p_eccbuf, ONFI_PAGE_SIZE(info)/BCH_SECTOR_SIZE);
    return onaf_page_write(info, ram_addr, blk_page_idx);
}

SECTION_ONFI 
s32_t onaf_page_read_with_ondie_ecc(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx, void *p_eccbuf)
{
    info->_model_info->_page_read(info, ram_addr, blk_page_idx);
    return info->_ecc_decode(ECC_USE_ODE, VZERO, VZERO);
}

SECTION_ONFI  
s32_t onaf_page_write_with_ondie_ecc(onfi_info_t *info, void *ram_addr, u32_t blk_page_idx, void *p_eccbuf)
{
    return info->_model_info->_page_write(info, ram_addr, blk_page_idx);
}


symb_fdefine(ONAF_PIO_READ_FUNC, onaf_pio_read_data);
symb_fdefine(ONAF_PIO_WRITE_FUNC, onaf_pio_write_data);
symb_fdefine(ONAF_PAGE_READ_FUNC, onaf_page_read);
symb_fdefine(ONAF_PAGE_WRITE_FUNC, onaf_page_write);
symb_fdefine(ONAF_PAGE_READ_ECC_FUNC, onaf_page_read_with_ecc_decode);
symb_fdefine(ONAF_PAGE_WRITE_ECC_FUNC, onaf_page_write_with_ecc_encode);
symb_fdefine(ONAF_PAGE_READ_ODE_FUNC, onaf_page_read_with_ondie_ecc);
symb_fdefine(ONAF_PAGE_WRITE_ODE_FUNC, onaf_page_write_with_ondie_ecc);



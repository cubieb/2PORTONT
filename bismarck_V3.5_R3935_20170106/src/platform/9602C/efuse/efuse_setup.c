#include <util.h>
#include <efuse/efuse_reg_map.h>
#include <dram/memcntlr_reg.h>

void enable_ldo(void)
{
    u32_t ddr_type = RFLD_MCR(dram_type)+1;

	//Read addr3
	RMOD_EFUSE_IND_CMD(addr, 3, cmd_en, 1, wren, 0x0);
	while(RFLD_EFUSE_IND_RD(busy));

    if(2 == ddr_type){
        u32_t addr3_b7 = (EFUSE_IND_RDrv >> 7) & 0x1;
        
        if(addr3_b7){
            RMOD_EFUSE_IND_CMD(addr, 6, cmd_en, 1, wren, 0x0);
        
            while(RFLD_EFUSE_IND_RD(busy));
        
            u32_t addr6_4_0 = EFUSE_IND_RDrv & 0x1F;
            REG32(0xbb00003c) = 0xe0|addr6_4_0;
            REG32(0xbb000040) = 0x0003fdc9;
        }
    }else if(3 == ddr_type){
        u32_t addr3_b6 = (EFUSE_IND_RDrv >> 6) & 1;
        
        if(addr3_b6){
            RMOD_EFUSE_IND_CMD(addr, 5, cmd_en, 1, wren, 0x0);
        
            while(RFLD_EFUSE_IND_RD(busy));
        
            u32_t addr5_12_8 = (EFUSE_IND_RDrv >> 8) & 0x1F;
            REG32(0xbb00003c) = 0xe0|addr5_12_8;
            REG32(0xbb000040) = 0x0003fdc9;
        }
    }
}

void change_ddr_swr(void)
{
	//Read addr3
	RMOD_EFUSE_IND_CMD(addr, 0x3, cmd_en, 0x1, wren, 0x0);
	while(RFLD_EFUSE_IND_RD(busy));

	u8_t addr3_data[] = {
		RFLD_EFUSE_IND_RD(rd_dat_3_0) & 0x3,
		(RFLD_EFUSE_IND_RD(rd_dat_3_0)>>2) & 0x3,
	};

	u8_t idx = 0;
	while((addr3_data[idx] == 0) && (idx < sizeof(addr3_data))) idx++;

	//Read addr4
	u32_t addr4_data = 0;
	u32_t ori_level = 7;
	u32_t tmp_ddrswr0 = 0x1d405620;

	RMOD_EFUSE_IND_CMD(addr, 0x4, cmd_en, 0x1, wren, 0x0);
	while(RFLD_EFUSE_IND_RD(busy));

	switch(idx){
	case 0:
		if(1 == addr3_data[idx]){
			addr4_data = RFLD_EFUSE_IND_RD(rd_dat_3_0);
		}else{
			addr4_data = RFLD_EFUSE_IND_RD(rd_dat_7_4);
		}
		break;

	case 1:
		if(1 == addr3_data[idx]){
			addr4_data = RFLD_EFUSE_IND_RD(rd_dat_11_8);
		}else{
			addr4_data = RFLD_EFUSE_IND_RD(rd_dat_15_12);
		}
		break;

	default:
		return;
	}

	DDRSWR_CTRL_1rv = 0x23790000;
	DDRSWR_CTRL_0rv = 0x15400000;
	DDRSWR_CTRL_0rv = 0x1d400000;

	if(ori_level > addr4_data){
		do{
			ori_level--;
			DDRSWR_CTRL_0rv = (tmp_ddrswr0 | ori_level);
			DDRSWR_CTRL_1rv = 0x23790001;
		}while(ori_level > addr4_data);
	}else if(ori_level < addr4_data){
		do{
			ori_level++;
			DDRSWR_CTRL_0rv = (tmp_ddrswr0 | ori_level);
			DDRSWR_CTRL_1rv = 0x23790001;
		}while(ori_level < addr4_data);
	}else{
        DDRSWR_CTRL_0rv = (tmp_ddrswr0 | ori_level);
        DDRSWR_CTRL_1rv = 0x23790001;
    }

	DDRSWR_CTRL_1rv = 0x00000000;
	DDRSWR_CTRL_0rv = 0x05400000;
	DDRSWR_CTRL_0rv = 0x0D400000;
	DDRSWR_CTRL_0rv = 0x05400000;
}

void change_lv_swr(void)
{
	//Read addr3
	RMOD_EFUSE_IND_CMD(addr, 0x3, cmd_en, 0x1, wren, 0x0);
	while(RFLD_EFUSE_IND_RD(busy));
	u8_t addr3_data = RFLD_EFUSE_IND_RD(rd_dat_7_4) & 0x3;

	//Read addr5
	u32_t addr5_data = 0;
	u32_t ori_level = 7;
	u32_t tmp_ddrswr0 = 0x1d405620;

	RMOD_EFUSE_IND_CMD(addr, 0x5, cmd_en, 0x1, wren, 0x0);
	while(RFLD_EFUSE_IND_RD(busy));

	if(0 == addr3_data){
		return;
	}else if(1 == addr3_data){
		addr5_data = RFLD_EFUSE_IND_RD(rd_dat_3_0);
	}else{
		addr5_data = RFLD_EFUSE_IND_RD(rd_dat_7_4);
	}

    /* Date: 2016-08-05:
         * Patch: Add +2 step, for boost to 1.15V 
         */
    addr5_data +=2;  

	SWR_CTRL_1rv = 0x23790000;
	SWR_CTRL_0rv = 0x15400000;
	SWR_CTRL_0rv = 0x1d400000;

	if(ori_level > addr5_data){
		do{
			ori_level--;
			SWR_CTRL_0rv = (tmp_ddrswr0 | ori_level);
			SWR_CTRL_1rv = 0x23790001;
		}while(ori_level > addr5_data);
	}else if(ori_level < addr5_data){
		do{
			ori_level++;
			SWR_CTRL_0rv = (tmp_ddrswr0 | ori_level);
			SWR_CTRL_1rv = 0x23790001;
		}while(ori_level < addr5_data);
	}else{
        SWR_CTRL_0rv = (tmp_ddrswr0 | ori_level);
	    SWR_CTRL_1rv = 0x23790001;
    }
    
	SWR_CTRL_1rv = 0x00000000;
	SWR_CTRL_0rv = 0x05400000;
	SWR_CTRL_0rv = 0x0D400000;
	SWR_CTRL_0rv = 0x05400000;
}

void enable_saw_freq(void)
{
	//Read addr3
	RMOD_EFUSE_IND_CMD(addr, 3, cmd_en, 1, wren, 0);
	while(RFLD_EFUSE_IND_RD(busy));
	u16_t addr3_data = EFUSE_IND_RDrv&0xFFFF;

    if(! ((addr3_data>>15)&1) ) return;

	//Read addr10
	RMOD_EFUSE_IND_CMD(addr, 10, cmd_en, 1, wren, 0);
	while(RFLD_EFUSE_IND_RD(busy));
    
	u32_t ori_level = 5;
    u32_t addr10_data = RFLD_EFUSE_IND_RD(rd_dat_7_4);

	if(ori_level > addr10_data){
		do{
			ori_level--;
            RMOD_SWR_CTRL_0_CMD(ref_saw_valid_swr, 0);
            RMOD_SWR_CTRL_0_CMD(ref_saw_swr, ori_level);
            RMOD_SWR_CTRL_0_CMD(ref_saw_valid_swr, 1);
		}while(ori_level > addr10_data);
	}else if(ori_level < addr10_data){
		do{
			ori_level++;
            RMOD_SWR_CTRL_0_CMD(ref_saw_valid_swr, 0);
            RMOD_SWR_CTRL_0_CMD(ref_saw_swr, ori_level);
            RMOD_SWR_CTRL_0_CMD(ref_saw_valid_swr, 1);
		}while(ori_level < addr10_data);
	}
    
    RMOD_SWR_CTRL_0_CMD(ref_saw_valid_swr, 0);
}


void enable_rc_calibration(void)
{
	//Read addr3
	RMOD_EFUSE_IND_CMD(addr, 0x3, cmd_en, 0x1, wren, 0x0);
	while(RFLD_EFUSE_IND_RD(busy));
	u8_t addr3_data10 = (EFUSE_IND_RDrv >> 10) & 0x1;

	if(0 == addr3_data10){
		return;
	}else{
        //Read addr7
        RMOD_EFUSE_IND_CMD(addr, 7, cmd_en, 0x1, wren, 0x0);
        while(RFLD_EFUSE_IND_RD(busy));
		u32_t addr7_data = (EFUSE_IND_RDrv & 0xFFFF);
        GPHY_IND_WDrv  = addr7_data;
        RMOD_GPHY_IND_CMD(addr, 0x1bcdc, cmd_en, 1, wr_en, 1);
        while(RFLD_GPHY_IND_RD(busy));
       
        GPHY_IND_WDrv  = addr7_data;
        RMOD_GPHY_IND_CMD(addr, 0x1bcde, cmd_en, 1, wr_en, 1);
        while(RFLD_GPHY_IND_RD(busy));
	}
}

void enable_r_calibration(void)
{
	//Read addr3
	RMOD_EFUSE_IND_CMD(addr, 0x3, cmd_en, 0x1, wren, 0x0);
	while(RFLD_EFUSE_IND_RD(busy));
	u8_t addr3_data13 = (EFUSE_IND_RDrv >> 13) & 0x1;

	if(0 == addr3_data13){
		return;
	}else{
        //Read addr10
        RMOD_EFUSE_IND_CMD(addr, 10, cmd_en, 0x1, wren, 0x0);
        while(RFLD_EFUSE_IND_RD(busy));
		u8_t addr10_data = (EFUSE_IND_RDrv & 0xF);
        GPHY_IND_WDrv  = (addr10_data<<12)| (addr10_data<<8)| (addr10_data<<4)|(addr10_data<<0);
        RMOD_GPHY_IND_CMD(addr, 0x1bce0, cmd_en, 1, wr_en, 1);        
        while(RFLD_GPHY_IND_RD(busy));
        
        GPHY_IND_WDrv  = (addr10_data<<12)| (addr10_data<<8)| (addr10_data<<4)|(addr10_data<<0);;
        RMOD_GPHY_IND_CMD(addr, 0x1bce2, cmd_en, 1, wr_en, 1);        
        while(RFLD_GPHY_IND_RD(busy));
	}
}

void enable_amptitude_calibration(void)
{
	//Read addr3
	RMOD_EFUSE_IND_CMD(addr, 0x3, cmd_en, 0x1, wren, 0x0);
	while(RFLD_EFUSE_IND_RD(busy));
	u8_t addr3_data11 = (EFUSE_IND_RDrv >> 11) & 0x1;

	if(0 == addr3_data11){
		return;
	}else{
    	//Read addr8
    	RMOD_EFUSE_IND_CMD(addr, 8, cmd_en, 0x1, wren, 0x0);
    	while(RFLD_EFUSE_IND_RD(busy));
		u32_t addr8_data = (EFUSE_IND_RDrv & 0xFFFF);

        GPHY_IND_WDrv  = addr8_data;
        RMOD_GPHY_IND_CMD(addr, 0x1bcac, cmd_en, 1, wr_en, 1);
        while(RFLD_GPHY_IND_RD(busy));
	}
}


void enable_ado_calibration(void)
{
	//Read addr3
	RMOD_EFUSE_IND_CMD(addr, 0x3, cmd_en, 0x1, wren, 0x0);
	while(RFLD_EFUSE_IND_RD(busy));
	u8_t addr3_data12 = (EFUSE_IND_RDrv >> 12) & 0x1;

	if(0 == addr3_data12){
		return;
	}else{
        RMOD_EFUSE_IND_CMD(addr, 9, cmd_en, 0x1, wren, 0x0);
        while(RFLD_EFUSE_IND_RD(busy));

        RMOD_GPHY_IND_CMD(addr, 0x1bc08, cmd_en, 1, wr_en, 0x0);
        while(RFLD_GPHY_IND_RD(busy));
       
        u32_t tmp_addr8 = GPHY_IND_RDrv;
        GPHY_IND_WDrv = ~((~tmp_addr8)&0xFFFFFFFB);

        RMOD_GPHY_IND_CMD(addr, 0x1bc08, cmd_en, 1, wr_en, 0x1);
        while(RFLD_GPHY_IND_RD(busy));

		u32_t addr9_data = (EFUSE_IND_RDrv & 0xFFFF);
        GPHY_IND_WDrv = addr9_data;
        RMOD_GPHY_IND_CMD(addr, 0x1bcfc, cmd_en, 1, wr_en, 1);
        while(RFLD_GPHY_IND_RD(busy));
    }
}


void disable_500m(void)
{
	//Read addr3
	RMOD_EFUSE_IND_CMD(addr, 0x3, cmd_en, 0x1, wren, 0x0);
	while(RFLD_EFUSE_IND_RD(busy));
	u8_t addr3_data14 = (EFUSE_IND_RDrv >> 14) & 0x1;

	if(0 == addr3_data14){
		return;
	}else{
    	RMOD_GPHY_IND_CMD(addr, 0x1a4a2, cmd_en, 1, wr_en, 0);       
        while(RFLD_GPHY_IND_RD(busy));

        u32_t tmp_gphy = GPHY_IND_RDrv;
        GPHY_IND_WDrv = (tmp_gphy&0xFFFFFEFF);

        RMOD_GPHY_IND_CMD(addr, 0x1a4a2, cmd_en, 1, wr_en, 1);
        while(RFLD_GPHY_IND_RD(busy));
    }
}

REG_INIT_FUNC(change_ddr_swr, 18);
REG_INIT_FUNC(change_lv_swr, 18);
REG_INIT_FUNC(enable_saw_freq, 18);
REG_INIT_FUNC(enable_ldo, 18);
REG_INIT_FUNC(enable_rc_calibration, 18);
REG_INIT_FUNC(enable_r_calibration, 18);
REG_INIT_FUNC(enable_amptitude_calibration, 18);
REG_INIT_FUNC(enable_ado_calibration, 18);
REG_INIT_FUNC(disable_500m, 18);



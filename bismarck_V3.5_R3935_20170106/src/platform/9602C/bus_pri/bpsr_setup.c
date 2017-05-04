#include <soc.h>
#include <bus_pri/bpsr_reg_map.h>
#include <dram/memcntlr_reg.h>

void bpsr_change_bus_priority(void)
{
    BUS_PRI_SWT_CTRLrv = 0x00000043;

    /*   BPSR0: PBO_LX_US_Read
          *  BPSR2: OCP0 
          *  BPSR3: PBO_LX_DS_Write
          *  BPSR4: PBO_LX_US_Write
          *  BPSR5: PBO_LX_DS_Read
          *  BPSR6: LX0 
          *  BPSR7: LX1 
          *  BPSR8: LX2 
          */
    BUS_PRI_SWT_0rv = 0xFFFFFFFF;
    BUS_PRI_SWT_2rv = 0x0;
    BUS_PRI_SWT_3rv = 0xFFFFFFFF;
    BUS_PRI_SWT_4rv = 0x88888888;
    BUS_PRI_SWT_5rv = 0x44444444;
    BUS_PRI_SWT_6rv = 0x22222222;
    BUS_PRI_SWT_7rv = 0x11111111;
    BUS_PRI_SWT_8rv = 0x0;

#if 0
    if(cg_dev_freq_info.ocp_mhz == 725){
        //Scott Lin: 2016/08/08: OCP = 725MHz, No package loss around 86hour
        //Scott Lin: 2016/08/15: OCP = 625MHz, No package loss around 86hour
        BUS_PRI_SWT_0rv = 0xFFFFFFFF;
        BUS_PRI_SWT_2rv = 0x0;
        BUS_PRI_SWT_3rv = 0xFFFFFFFF;
        BUS_PRI_SWT_4rv = 0x88888888;
        BUS_PRI_SWT_5rv = 0x44444444;
        BUS_PRI_SWT_6rv = 0x22222222;
        BUS_PRI_SWT_7rv = 0x11111111;
        BUS_PRI_SWT_8rv = 0x00000001;
    }else{
        BUS_PRI_SWT_0rv = 0xFFFFFFFF;
        BUS_PRI_SWT_2rv = 0x0;
        BUS_PRI_SWT_3rv = 0xFFFFFFFF;
        BUS_PRI_SWT_4rv = 0xFF000000;
        BUS_PRI_SWT_5rv = 0x00FF0000;
        BUS_PRI_SWT_6rv = 0x0000FF00;
        BUS_PRI_SWT_7rv = 0x000000FF;
        BUS_PRI_SWT_8rv = 0x00000001;
    }
#endif

    //These register setting takes effect after DMCR (0xB800101C)
    u32_t dmcr_val = DMCRrv;
    DMCRrv = dmcr_val;
    
    return;
}

REG_INIT_FUNC(bpsr_change_bus_priority, 17);


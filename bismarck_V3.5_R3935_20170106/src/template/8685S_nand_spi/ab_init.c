#include <soc.h>
#include <cpu/tlb.h>
#include <cpu/mmu_drv.h>
#include <init_define.h>
#include <nand_spi/nand_spi_util.h>
#include <register_map.h>

extern u32_t mapped_physical_sram_start;
extern void plr_tlb_miss_tlb (void);

static char _mmu_init_fail_msg[] SECTION_RECYCLE_DATA = {"no physical page was added, system stops\n"};
static char _nsu_table_init_fail_msg[] SECTION_RECYCLE_DATA = {"plr mapping table init fail\n"};
const char __msg_dis_ocp_to_monitor[] SECTION_SDATA = {"Disable OCP Timeout Monitor\n"};
const char __msg_dis_lx_to_monitor[] SECTION_SDATA = {"Disable LX Timeout Monitor\n"};

SECTION_RECYCLE void 
disable_timeout_monitor(void)
{
    //Disable OCP timeout monitor
    TO_CTRL_T ocp_to_mon;
    ocp_to_mon.v = TO_CTRLrv;
    ocp_to_mon.f.to_ctrl_en = 0;
    TO_CTRLrv = ocp_to_mon.v;
    puts(__msg_dis_ocp_to_monitor);

    //Disable LX timeout monitor
    BUS_TO_CTRL_T lx_to_mon;
    lx_to_mon.v = BUS_TO_CTRLrv;
    lx_to_mon.f.to_en = 0;
    BUS_TO_CTRLrv = lx_to_mon.v;
    puts(__msg_dis_lx_to_monitor);
}
REG_INIT_FUNC(disable_timeout_monitor, 2);


SECTION_RECYCLE void
init_spi_nand(void) {
    //basic spi-nand init and establish the plr_mapping_table
    if(-1 == nsu_init()){
        inline_puts(_nsu_table_init_fail_msg);
        while(1);
    }
}
REG_INIT_FUNC(init_spi_nand, 3);

SECTION_TEXT UTIL_FAR UTIL_MIPS16 void
second_banner(void)  {
    inline_puts("    .text and .ro sections work!\n");
    //while(1);
}

SECTION_RECYCLE void
_init_tlb(void) {
    // 1. clean up mmu driver
    u32_t end_of_mapping = OTTO_SRAM_START + OTTO_SRAM_SIZE;
    if (mmu_drv_init((u32_t)&mapped_physical_sram_start, end_of_mapping)==0) {
        inline_puts(_mmu_init_fail_msg);
        while(1);
    }
    
    // 2. enable TLB and isr
    #if 0
    u32_t tr;
    __asm__ __volatile__ (
        "mfc0 %0, " TO_STR(CP0_STATUS) ASM_NL
        "ori %0, %0, 1" ASM_NL
        "mtc0 %0, " TO_STR(CP0_STATUS) ASM_NL
        : "=r" (tr)
    );
    #endif    
}

SECTION_UNS_TEXT void 
init_tlb(void) {
    _init_tlb();
    // 3. show message
    inline_puts("TLB initial done:\n    .ro section works!\n");
    second_banner();
}

REG_INIT_FUNC(init_tlb, 10);


// --------------------------
//      export symbols
// --------------------------

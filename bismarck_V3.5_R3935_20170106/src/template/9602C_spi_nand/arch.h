//arch.h for template/9602C_spi_nand
#ifndef OTTO_NUM_DRAM_PARA
#define OTTO_NUM_DRAM_PARA          1
#endif

//#include <cpu/cpu.h>

#ifndef __ASSEMBLER__
#define PLR_VERSION           (0x03000500)  //Formal release version: Bismarch 3.5

#define SECTION_UART SECTION_UNS_TEXT
#define SECTION_CMD_NODE __attribute__ ((section (".cli_node")))
#define SECTION_CMD_MP_NODE __attribute__ ((section (".cli_mp_node")))
#define SECTION_CMD_TAIL __attribute__ ((section (".cli_tail")))


#include <plr_sections.h>
//#include "/home/gangadhar/Bismarck/bismarck_V3.5_R3935_20170106/src/include/plr_sections.h"
#include <util.h>
//#include "/home/gangadhar/Bismarck/bismarck_V3.5_R3935_20170106/src/template/9602C_spi_nand/util.h"

extern u8_t mapped_physical_sram_start;
#define OTTO_BIG_BUFFER_END   ((u32_t)(&mapped_physical_sram_start))

#ifndef DONT_DECLAIRE__SOC_SYMBOLS
    extern soc_t _soc SECTION_SDATA;
    #include <bios_io.h>
    //#include "/home/gangadhar/Bismarck/bismarck_V3.5_R3935_20170106/src/include/bios_io.h"		
    #define _spi_nand_info (_soc.flash_info.spi_nand_info)

    // system parameters
    //#include <cg/cg.h>
    //#include </home/gangadhar/Bismarck/bismarck_V3.5_R3935_20170106/src/platform/9602C/cg/cg.h>
    #include "../../src/platform/9602C/cg/cg.h"	
	
    #define SECTION_CG_CORE_INIT      UTIL_FAR SECTION_UNS_TEXT
    #define SECTION_CG_MISC      SECTION_TEXT
    #define SECTION_CG_MISC_DATA SECTION_RO
    extern const cg_dev_freq_t cg_ddr2_proj_freq;
    extern const cg_dev_freq_t cg_ddr3_proj_freq;
    extern const u32_t uart_baud_rate;

    #define SECTION_AUTOK SECTION_TEXT
    #define MEMCNTLR_DATA_SECTION SECTION_PARAMETERS

    #define SECTION_SPI_NAND_INFO SECTION_RECYCLE_DATA

    #ifndef SECTION_SPI_NAND_DATA
        #define SECTION_SPI_NAND_DATA  SECTION_PARAMETERS
    #endif
#endif //DONT_DECLAIRE__SOC_SYMBOLS
#include <spi_nand/spi_nand_util.h>

#define MMU_READ_PAGE_FROM_FLASH(buf, virtual_page_num) nsu_logical_page_read((buf), (virtual_page_num))

#define MSG_FAIL_LOAD_PAGE __msg_load_page_fail

#define PLR_MAP_TABLE_SIZE 64

#else //__ASSEMBLER__

#define OTTO_BIG_BUFFER_END     (mapped_physical_sram_start)

#endif //!__ASSEMBLER__

#define OTTO_PLR_ECC_BUFFER     (OTTO_BIG_BUFFER_END - MAX_ECC_BUF_SIZE)
#define OTTO_PLR_OOB_BUFFER     (OTTO_PLR_ECC_BUFFER - MAX_OOB_BUF_SIZE)
#define OTTO_PLR_PAGE_BUFFER    (OTTO_PLR_OOB_BUFFER - MAX_PAGE_BUF_SIZE)
#define OTTO_PLR_STACK_DEF      (OTTO_PLR_PAGE_BUFFER-4)

#define SYSTEM_RESET() do { \
    *((volatile u32_t *)0xb8003268) = 0x0; \
    *((volatile u32_t *)0xb8003268) = 0x80000000;\
	} while(0)

#define MC_ZQC_RETRY_LIM   8

#define project_name 9602C_MP_spi_nand_demo
#define platform_name 9602C
#define OTTO_ENTRY 0x9f000000
#define OTTO_DRAM_GEN3 1
#define OTTO_LMA_BASE 0
#define TLB_PAGE_SHIFT 12
#define TLB_PAGE_SIZE (1 << 12)
#define TLB_PAGE_MASK ((1 << 12) - 1)
#define TLB_NUM_TLB_ENTRY 64
#define TLB_NUM_TLB_MASK 0x3f
#define OTTO_LDS ./src/lds/spi_nand_with_lplr.lds
#define OTTO_STARTUP ./src/lib/startup/startup_nand_with_lplr.S
#define MMU_PHY_SIZE 0x8000
#define MMU_VM_SIZE 0x40000
#define MMU_PHY_BASE_ADDR 0x9f000000
#define MMU_CHECK_INETRNAL_ERROR 1
#define USE_CC_GPOPT 1
#define CC_GPOPT -mgpopt -mlocal-sdata -mextern-sdata -G 4
#define IS_RECYCLE_SECTION_EXIST 
#define template_name 9602C_spi_nand
#define name_of_project "RTL9602C"
#define OTTO_FLASH_SPI_NAND 1
#define OTTO_NUM_DRAM_PARA 2
#define OTTO_NUM_FLASH_PARA 1
#define FILTER_OUT_OBJS 
#define spi_nand_path_prefix src/platform/9602C/spi_nand/spi_nand_
#define NSU_USING_SYMBOL_TABLE_FUNCTION 

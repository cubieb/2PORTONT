#include <soc.h>
#include <cli/cli_access.h>
#include <soc_exam_util/mt_ram_test.h>

#ifndef SECTION_CLI_UTIL
    #define SECTION_CLI_UTIL
#endif

int __attribute__ ((weak))
mt_highmem_test(void) {
    puts("WW: missing mt_highmem_test\n");
    return MT_FAIL;
}

int __attribute__ ((weak))
spi_nand_flash_memory_test(u32_t addr, u32_t size) {
    puts("WW: missing spi_nand_flash_memory_test\n");
    return MT_FAIL;
}

extern int mt_ram_test(u32_t addr, u32_t size);

SECTION_CLI_UTIL cli_cmd_ret_t
cli_std_mt(const void *user, u32_t argc, const char *argv[])
{
    u32_t total_test_runs=1;

    // "mt ram <start addr> <test size> [test loops]"
    if(argc<4) return CCR_INCOMPLETE_CMD;
    else if(5==argc) total_test_runs=atoi(argv[argc-1]);

    u32_t addr=atoi(argv[2]);
    u32_t size=atoi(argv[3]);
    
    if (size > 0x10000000) {
        EPRINTF("size should be NOT large than 256MB\n");
        return CCR_FAIL; 
    }
    
    printf("II: mt ram test\n    addr:0x%08x, size:0x%x, loop:%d \n",
            addr, size, total_test_runs);
    
    u32_t loop;
    for(loop=1 ; loop<=total_test_runs ; loop++){
        printf("II: #%d runs\n", loop);
        if(MT_SUCCESS!=mt_ram_test(addr, size))
            return CCR_FAIL;    
    }
    return CCR_OK;
}

unsigned int get_ddr_size(void)
{
    u32_t dcr = REG32(0xB8001004);
    u8_t bank = ((dcr>>28)&0x3)+1;
    u8_t dbw  = (dcr>>24)&0x3;
    u8_t row  = ((dcr>>20)&0xF)+11;
    u8_t col  = ((dcr>>16)&0xF)+8;

    u32_t size = 1 << (bank+dbw+row+col);
    return size;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_highmem_mt(const void *user, u32_t argc, const char *argv[])
{
    u32_t total_test_runs=1;

    // "mt highmem [test loops]"
    if(3==argc)
        total_test_runs=atoi(argv[argc-1]);
    u32_t max_size=get_ddr_size();
    
    if (max_size <= SIZE_256MB) {
        puts("II: need more than 256MB DRAM size\n");
        return CCR_OK;
    }
    
    printf("II: mt high memory test\n    DDR size: %dMB, loop:%d \n",
            max_size>>20, total_test_runs);
    
    u32_t loop;
    for(loop=0 ; loop<total_test_runs ; loop++){
        printf("II: #%d runs\n", loop+1);
        if(MT_SUCCESS!=mt_highmem_test())
            return CCR_FAIL;    
    }
    return CCR_OK;
}

SECTION_CLI_UTIL cli_cmd_ret_t
cli_spi_nand_mt(const void *user, u32_t argc, const char *argv[])
{
    // "mt spi_nand <start block> <end block>"
    if(argc<4) return CCR_INCOMPLETE_CMD;

    u32_t blk_start = atoi(argv[2]);
    u32_t blk_end   = atoi(argv[3]);
    
    spi_nand_flash_memory_test(blk_start, blk_end);
    return CCR_OK;
}

cli_top_node(mt, VZERO);
    cli_add_node(ram, mt, (cli_cmd_func_t *)cli_std_mt);
    cli_add_help(ram, "mt ram <start addr> <test size> [test loops]");
    cli_add_node(highmem, mt, (cli_cmd_func_t *)cli_highmem_mt);
    cli_add_help(ram, "mt highmem [test loops]");
    cli_add_node(spi_nand, mt, (cli_cmd_func_t *)cli_spi_nand_mt);
    cli_add_help(spi_nand, "mt spi_nand <start block> <end block>");

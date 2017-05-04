#include <util.h>
#include <soc_exam_util/mt_ram_test.h>

#define ERROR_PRINTF(pat, patWR, addr, zone)    {EPRINTF("pattern=0x%x, expected=0x%x, (0x%x)=0x%x zone(0x%x)",        \
                                                          pat, patWR, (uint32_t)addr, *(addr), zone); }
                                                          

#ifndef MSRRrv
#define MSRRrv  REG32(0xB8001038)
#define DOR0rv  REG32(0xB8001700)
#endif
#define FLUSH_OCP_CMD()             ({ MSRRrv=0x80000000; while(0x40000000!=MSRRrv);})
#define ZONE0_OFFSET_CONFIG(off)    ({ DOR0rv=(off); })


/* instruction side is SRAM only */
int mt_highmem_test(u32_t max_size)
{
    // do some test here
    u32_t patWR, start;
    u32_t addr_max;
    volatile u32_t *addr;

    //printf("II: DDR size is 0x%x\n", max_size);
    printf("II: %s...", __FUNCTION__);
        
    dcache_wr_inv_all();
    // write
    puts("WR\b\b");
    for (start=0; start<max_size; start+=SIZE_256MB){
        FLUSH_OCP_CMD();
        ZONE0_OFFSET_CONFIG(start);
        FLUSH_OCP_CMD();
        //printf("\tWR word to DDR physical address 0x%x\n", start);
        addr = (volatile u32_t *)(0x80000000);
        addr_max = (u32_t)addr+SIZE_256MB;
        
        for(; (u32_t)addr<addr_max; addr++) {
            patWR = (u32_t)(addr+start); 
            *(addr) = patWR;
            if(*(addr) != patWR) {
                EPRINTF("expected=0x%x, (0x%x)=0x%x zone_offset(0x%x)\n",
                         patWR, addr, *addr, start); 
                    
                return MT_FAIL;
            }
        }
        dcache_wr_inv_all();
    }
    
    puts("RD\b\b");
    FLUSH_OCP_CMD();
    ZONE0_OFFSET_CONFIG(0);
    FLUSH_OCP_CMD();
    
    // read
    for (start=0; start<max_size; start+=SIZE_256MB){
        FLUSH_OCP_CMD();      
        ZONE0_OFFSET_CONFIG(start);
        FLUSH_OCP_CMD();
        //printf("\tRD word to DDR physical address 0x%x\n", start);
        addr = (volatile u32_t *)(0x80000000);

        for(; (u32_t)addr<addr_max; addr++) {
            patWR = (u32_t)(addr+start); 
            if(*(addr) != patWR) {
                EPRINTF("expected=0x%x, (0x%x)=0x%x zone_offset(0x%x)\n",
                         patWR, addr, *addr, start); 
                return MT_FAIL;
            }
        }
        dcache_wr_inv_all();
    }    
    puts(" passed\n");
    return MT_SUCCESS;
}

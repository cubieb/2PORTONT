/*
 * Include Files
 */
#include <common.h>
#include <linux/ctype.h>
#include <spi_flash.h>
#include <soc.h>
#include <../../../lib/nor_spi_gen3/nor_spif_core.h>
#define _cache_flush    (((soc_t *)(0x9f000020))->bios).dcache_writeback_invalidate_all

static unsigned long flash_size;

/* Definitions for memory test error handing manner */
#define MT_SUCCESS    (0)
#define MT_FAIL       (-1)
#define ARY_LEN(x)    (sizeof(x)/sizeof(x[0]))


/* Definitions for memory test reset mode */
enum RESET_MODE{
    NO_RESET = 0,    
    UBOOT_RESET = 1,    
    WHOLE_CHIP_RESET = 2,
};

//The default seting of memory test error handling manner is non-blocking 
//Using "Environment parameter" or "command flag setting" can change this
//"Environment parameter": setenv mt_freeze_block debug
//"command flag setting": mdram_test/mflash_test -b/-mt_block
static u32_t g_err_handle_block_mode=0; 
static u32_t g_reset_flag=NO_RESET;

#define HANDLE_FAIL \
({ \
    printf("%s (%d) test failed.\n", __FUNCTION__,__LINE__);\
    if(!g_err_handle_block_mode){\
        return MT_FAIL;\
    }else{ \
        while(1);\
    } \
})

#define SIZE_1MB (0x100000)
#define SIZE_2MB (0x200000)
#define SIZE_3MB (0x300000)
#define FLASH_TOP_ADDRESS(flash_size)        (FLASHBASE+flash_size-1)
#define TEST_SIZE_PER_PATTREN    (0x10000)  //64KB
static u32_t src_data_addr;
static u32_t flash_backup_addr;
#define    SRC_DATA_ADDR        (src_data_addr)
#define    FLASH_BACKUP_ADDR    (flash_backup_addr)
#define RSV_SPACE              SIZE_3MB
u32_t assign_test_range = 0;


/**************************
  * Command Parsing
  *************************/ 
typedef struct {
    u8_t test_loops;
    u32_t mmio_addr;    //0xB4xxxxxx or 0xBDxxxxxx
    u32_t mt_size; //Unit is byte
} spif_cmd_parsing_info_t;


void spi_disable_message(void);
void spi_enable_message(void);

extern unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);

/*
 * Data Declaration
 */
DECLARE_GLOBAL_DATA_PTR;

const u32_t flash_patterns[] = {
    0x5a5aa5a5,
    0xa5a5a5a5,
    0x55555555, 
    0xaaaaaaaa, 
    0x01234567, 
    0x76543210, 
    0x89abcdef,
    0xfedcba98,
    0x00000000,
    0xff00ff00,
    0x00ff00ff,
    0x0000ffff,
    0xffff0000,
    0xffffffff,
};

/*
 * Function Declaration
 */
int flash_sect_erase (ulong addr_first, ulong addr_last)
{
    u32_t sector=0, size = addr_last - addr_first;
    u32_t offset = addr_first-FLASHBASE;
    u32_t cs = offset/flash_size;
    struct spi_flash *sf = spi_flash_probe(0, cs, 0, 0);
    
    if(!sf) {
       printf("spi flash %d probe failed\n", cs);
        return 0;
    }
    if (size > CONFIG_ENV_SECT_SIZE) {
        sector = size / CONFIG_ENV_SECT_SIZE;
        if (size % CONFIG_ENV_SECT_SIZE) sector++;
    }    
    
    return spi_flash_erase(sf, offset, sector * CONFIG_ENV_SECT_SIZE);
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 * Make sure all target addresses are within Flash bounds,
 * and no protected sectors are hit.
 * Returns:
 * ERR_OK          0 - OK
 * ERR_TIMOUT      1 - write timeout
 * ERR_NOT_ERASED  2 - Flash not erased
 * ERR_PROTECTED   4 - target range includes protected sectors
 * ERR_INVAL       8 - target address not in Flash memory
 * ERR_ALIGN       16 - target address not aligned on boundary
 *            (only some targets require alignment)
 */
int
flash_write (char *src, ulong addr, ulong cnt)
{
    u32_t offset = (addr-FLASHBASE);
    u32_t cs = offset/flash_size;
    struct spi_flash *sf = spi_flash_probe(0, cs, 0, 0);
    if(!sf) {
        printf("spi flash %d probe failed\n", cs);
        return 0;
    }
    return spi_flash_write(sf, offset, cnt, src);
}
 
/* Function Name: 
 *     nor_normal_patterns
 * Descripton:
 *    
 * Input:
 *    None
 * Output:
 *     None
 * Return:
 *      None
 */
int flash_normal_patterns(u32_t flash_start_addr, u32_t test_size_per_pattern, u32_t flash_test_size)
{
    int i, j;
    u32_t start_value, flash_start;
    volatile u32_t *src_start;
    u32_t re_idx=0;
    u32_t remain_size=flash_test_size;
    char _err_msg[] = {"addr(0x%x): 0x%x != pattern(0x%x) %s, %d\n"};
    
    printf("=======start %s test / 0x%08x~0x%08x (%dMB)=======\n", __FUNCTION__, flash_start_addr, flash_start_addr+flash_test_size-1, flash_test_size/SIZE_1MB);
    for (i=0; i < (sizeof(flash_patterns)/sizeof(u32_t)); i++, remain_size=flash_test_size, re_idx=0){
        /* write pattern*/
        _cache_flush();
        src_start = (u32_t *)SRC_DATA_ADDR;
        for(j=0; j < test_size_per_pattern; j=j+4)
        {
             *src_start = flash_patterns[i];
            src_start++;
        }
        
        /* check data */
        _cache_flush();
        src_start = (u32_t *)SRC_DATA_ADDR;
        for(j=0; j < test_size_per_pattern; j=j+4){
            start_value = (*src_start);
            if(start_value != flash_patterns[i]){
                printf(_err_msg,(u32_t)src_start,start_value,flash_patterns[i],__FUNCTION__,__LINE__);
                HANDLE_FAIL;
            }
            src_start++;
        }
        printf("SPIF: pattern[%d](0x%x) setting pass", i, flash_patterns[i]);


        while(remain_size > test_size_per_pattern){
            remain_size -= test_size_per_pattern;

            src_start = (u32_t *)SRC_DATA_ADDR;
            flash_start = flash_start_addr + (((re_idx++)*test_size_per_pattern)%flash_test_size);
            flash_sect_erase(flash_start, (flash_start+test_size_per_pattern-1));
            flash_write((char *)src_start, flash_start, test_size_per_pattern);
        
            /* check flash data sequentially. Uncached address */
            src_start = (u32_t *)(UADDR((u32_t)flash_start));
            for(j=0; j < test_size_per_pattern; j=j+4){
                start_value = (*src_start);
                if(start_value != flash_patterns[i]){
                    printf(_err_msg,(u32_t)src_start,start_value,flash_patterns[i],__FUNCTION__,__LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
            }
            
            /* check flash data interlevelingly. Uncached address */
            src_start = (u32_t *)(UADDR((u32_t)flash_start));
            for(j=0; j < (test_size_per_pattern/2); j=j+4){
                start_value = (*src_start);
                if(start_value != flash_patterns[i]){
                    printf(_err_msg,(u32_t)src_start,start_value,flash_patterns[i],  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                
                start_value = *(src_start + ((unsigned int)test_size_per_pattern/8));
                if(start_value != flash_patterns[i]){
                    printf(_err_msg,(u32_t)(src_start + ((unsigned int)test_size_per_pattern/8)) , start_value, flash_patterns[i],  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
            }
            
            /* check flash data sequentially. Cached address */
            src_start = (u32_t *)(CADDR((u32_t)flash_start));
            for(j=0; j < test_size_per_pattern; j=j+4){
                start_value = (*src_start);
                if(start_value != flash_patterns[i]){
                    printf(_err_msg,(u32_t)src_start,start_value,flash_patterns[i],__FUNCTION__,__LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
            }
        
            /* check flash data interlevelingly. Uncached address */
            src_start = (u32_t *)(CADDR((u32_t)flash_start));
            for(j=0; j < (test_size_per_pattern/2); j=j+4){
                start_value = (*src_start);
                if(start_value != flash_patterns[i]){
                    printf(_err_msg,(u32_t)src_start,start_value,flash_patterns[i],  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                
                start_value = *(src_start + ((unsigned int)test_size_per_pattern/8));
                if(start_value != flash_patterns[i]){
                    printf(_err_msg,(u32_t)(src_start + ((unsigned int)test_size_per_pattern/8)) , start_value, flash_patterns[i],  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
            }
            printf("\rSPIF: pat[%d](0x%x) 0x%x pass (remain 0x%x)", i, flash_patterns[i], flash_start, remain_size);
        }
    }
    printf("\n%s test succeed.\n", __FUNCTION__);
    return MT_SUCCESS;
}

int flash_walking_of_1(u32_t flash_start_addr, u32_t test_size_per_pattern, u32_t flash_test_size)
{
    int i;
    int j;
    u32_t walk_pattern;
    u32_t start_value;
    u32_t flash_start;
    u32_t re_idx=0;
    u32_t remain_size = flash_test_size;
    volatile u32_t *src_start;
    
    printf("\n=======start %s test / 0x%08x~0x%08x (%dMB)=======\n", __FUNCTION__, flash_start_addr, flash_start_addr+flash_test_size-1, flash_test_size/SIZE_1MB);
    while(remain_size > test_size_per_pattern){
        for (i=0; i < 32; i++){        
            /* generate pattern */
            walk_pattern = (1 << i);
            
            /* write pattern*/
            _cache_flush();
            src_start = (u32_t *)SRC_DATA_ADDR;
            for(j=0; j < test_size_per_pattern; j=j+4){
                *src_start = walk_pattern;
                src_start++;
            }
        
            /* check data */  
            _cache_flush();
            src_start = (u32_t *)SRC_DATA_ADDR;
            for(j=0; j < test_size_per_pattern; j=j+4){
                start_value = (*src_start);
                if(start_value != walk_pattern){
                    printf("addr(0x%x): 0x%x != pattern(0x%x) %s, %d\n",\
                    (u32_t)src_start , start_value, walk_pattern,  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
            }
            printf("\rFlash: pattern[%d](0x%x) setting passed", i, walk_pattern);

            src_start = (u32_t *)SRC_DATA_ADDR;
            flash_start = flash_start_addr + (((re_idx++)*test_size_per_pattern)%flash_test_size);
            flash_sect_erase(flash_start, (flash_start+test_size_per_pattern-1));
            flash_write((char *)src_start, flash_start, test_size_per_pattern);

            /* check data */  
            src_start = (u32_t *)(UADDR((u32_t)flash_start));
            for(j=0; j < test_size_per_pattern; j=j+4){
                start_value = (*src_start);
                if(start_value != walk_pattern){
                    printf("addr(0x%x): 0x%x != pattern(0x%x) %s, %d\n",\
                    (u32_t)src_start , start_value, walk_pattern,  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
            }
            remain_size -= test_size_per_pattern;
            printf("\rFlash: pattern[%d](0x%x) 0x%x passed (remain 0x%x)", i, walk_pattern, flash_start, remain_size);
            if(0 == remain_size) goto end;
        }
    }
end:
    printf("\n%s test succeed.\n", __FUNCTION__);
    return MT_SUCCESS;
}

int flash_walking_of_0(u32_t flash_start_addr, u32_t test_size_per_pattern, u32_t flash_test_size)
{
    int i;
    int j;
    u32_t start_value;
    u32_t walk_pattern;
    u32_t flash_start;
    u32_t re_idx=0;
    u32_t remain_size = flash_test_size;
    volatile u32_t *src_start;
    
    printf("\n=======start %s test / 0x%08x~0x%08x (%dMB)=======\n", __FUNCTION__, flash_start_addr, flash_start_addr+flash_test_size-1, flash_test_size/SIZE_1MB);
    while(remain_size > test_size_per_pattern){
        for (i=0; i < 32; i++){           
            /* generate pattern */
            walk_pattern = ~(1 << i);
            
            /* write pattern*/
            _cache_flush();
            src_start = (u32_t *)SRC_DATA_ADDR;
            for(j=0; j < test_size_per_pattern; j=j+4){
                *src_start = walk_pattern;
                src_start++;
            }        
        
            /* check data */  
            _cache_flush();
            src_start = (u32_t *)SRC_DATA_ADDR;
            for(j=0; j < test_size_per_pattern; j=j+4){
                start_value = (*src_start);
                if(start_value != walk_pattern){
                    printf("addr(0x%x): 0x%x != pattern(0x%x) %s, %d\n",\
                    (u32_t)src_start , start_value, walk_pattern,  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
            }
            printf("\rFlash:pattern[%d](0x%x) setting passed", i, walk_pattern);
        
            src_start = (u32_t *)SRC_DATA_ADDR;
            flash_start = flash_start_addr + (((re_idx++)*test_size_per_pattern)%flash_test_size);
            flash_sect_erase(flash_start, (flash_start+test_size_per_pattern-1));
            flash_write((char *)src_start, flash_start, test_size_per_pattern);

            /* check data */  
            src_start = (u32_t *)(UADDR((u32_t)flash_start));
            for(j=0; j < test_size_per_pattern; j=j+4){
                start_value = (*src_start);
                if(start_value != walk_pattern){
                    printf("addr(0x%x): 0x%x != pattern(0x%x) %s, %d\n",\
                    (u32_t)src_start , start_value, walk_pattern,  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
            }
            remain_size -= test_size_per_pattern;
            printf("\rFlash:pattern[%d](0x%x) 0x%x passed (remain 0x%x)", i, walk_pattern, flash_start, remain_size);
            if(0 == remain_size) goto end;
        }
    }
end:
    printf("\n%s test succeed.\n", __FUNCTION__);
    return MT_SUCCESS;
}

int flash_addr_rot(u32_t flash_start_addr, u32_t test_size_per_pattern, u32_t flash_test_size)
{
    int i;
    int j;
    u32_t start_value;
    u32_t flash_start;
    u32_t re_idx=0;
    u32_t remain_size = flash_test_size;
    volatile u32_t *_dram_start;
    volatile u32_t *src_start;
    
    printf("\n=======start %s test / 0x%08x~0x%08x (%dMB)=======\n", __FUNCTION__, flash_start_addr, flash_start_addr+flash_test_size-1, flash_test_size/SIZE_1MB);
    while(remain_size > test_size_per_pattern){
        for (i=0; i < 32; i=i+4){
            /* write pattern*/
            _cache_flush();           
            src_start = (u32_t *)SRC_DATA_ADDR;
            for(j=0; j < test_size_per_pattern; j=j+4){
                *src_start = ((u32_t)src_start << i);
                src_start++;
            }
        
            /* check data */  
            _cache_flush();
            src_start = (u32_t *)SRC_DATA_ADDR;
            for(j=0; j < test_size_per_pattern; j=j+4){
                start_value = (*src_start);
                if(start_value != (((u32_t)src_start) << i)){
                    printf("addr(0x%x): 0x%x != pattern(0x%x) %s, %d\n",\
                          (u32_t)src_start , start_value, (((u32_t)src_start) << i),  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
            }
            printf("\rFlash: rotate %d setting passed", i);
        
            src_start = (u32_t *)SRC_DATA_ADDR;
            flash_start = flash_start_addr + (((re_idx++)*test_size_per_pattern)%flash_test_size);
            flash_sect_erase(flash_start, (flash_start+test_size_per_pattern-1));
            flash_write((char *)src_start, flash_start, test_size_per_pattern);

            /* check data */  
            _dram_start = (u32_t *) SRC_DATA_ADDR;
            src_start = (u32_t *)(UADDR((u32_t)flash_start));
            for(j=0; j < test_size_per_pattern; j=j+4){
                start_value = (*src_start);
                if(start_value != (((u32_t)_dram_start) << i)){
                    printf("addr(0x%x): 0x%x != pattern(0x%x) %s, %d\n",\
                    (u32_t)src_start , start_value, (((u32_t)_dram_start) << i),  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
                _dram_start++;
            }
            remain_size -= test_size_per_pattern;
            printf("\rFlash: rotate %d 0x%x passed (remain 0x%x)", i, flash_start, remain_size);
            if(0 == remain_size) goto end;
        }
    }
end:
    printf("\n%s test succeed.\n", __FUNCTION__);
    return MT_SUCCESS;
}

int flash_com_addr_rot(u32_t flash_start_addr, u32_t test_size_per_pattern, u32_t flash_test_size)
{
    int i;
    int j;
    u32_t start_value;
    u32_t flash_start;
    u32_t re_idx=0;
    u32_t remain_size = flash_test_size;
    volatile u32_t *_dram_start;
    volatile u32_t *src_start;
    
    printf("\n=======start %s test / 0x%08x~0x%08x(%d MB)=======\n", __FUNCTION__, flash_start_addr, flash_start_addr+flash_test_size-1, flash_test_size/SIZE_1MB);
    while(remain_size > test_size_per_pattern){
        for (i=0; i < 32; i=i+4){           
            /* write pattern*/
            _cache_flush();
            src_start = (u32_t *)SRC_DATA_ADDR;
            for(j=0; j < test_size_per_pattern; j=j+4){
                *src_start = ~(((u32_t)src_start) << i);
                src_start++;
            }
            
            /* check data */  
            _cache_flush();
            src_start = (u32_t *)SRC_DATA_ADDR;
            for(j=0; j < test_size_per_pattern; j=j+4){
                start_value = (*src_start);
                if(start_value != (~((u32_t)src_start << i))){
                     printf("addr(0x%x): 0x%x != pattern(0x%x) %s, %d\n",\
                            (u32_t)src_start , start_value, ~(((u32_t)src_start) << i),  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
            }
            printf("\rFlash: ~rotate %d setting passed", i);
            
               _cache_flush();    
            src_start = (u32_t *)SRC_DATA_ADDR;
            flash_start = flash_start_addr + (((re_idx++)*test_size_per_pattern)%flash_test_size);
            flash_sect_erase(flash_start, (flash_start+test_size_per_pattern-1));
            flash_write((char *)src_start, flash_start, test_size_per_pattern);
            
            /* check data */  
            _dram_start = (u32_t *) SRC_DATA_ADDR;
            src_start = (u32_t *)(UADDR((u32_t)flash_start));
            for(j=0; j < test_size_per_pattern; j=j+4){
                start_value = (*src_start);
                if(start_value != (~((u32_t)_dram_start << i))){
                    printf("addr(0x%x): 0x%x != pattern(0x%x) %s, %d\n",\
                    (u32_t)src_start , start_value, ~(((u32_t)_dram_start) << i),  __FUNCTION__, __LINE__);
                    HANDLE_FAIL;
                }
                src_start++;
                _dram_start++;
            }
            remain_size -= test_size_per_pattern;
            printf("\rFlash: ~rotate %d 0x%x passed (remain 0x%x)", i, flash_start,remain_size);        
            if(0 == remain_size) goto end;
        }
    }
end:
    printf("\n%s test succeed.\n", __FUNCTION__);
    return MT_SUCCESS;
}

int _flash_test(u32_t flash_start_addr, u32_t test_size_per_pattern, u32_t flash_test_size) {
    u32_t i;
    typedef int (norsf_test_t)(u32_t, u32_t, u32_t);
    norsf_test_t *norsf_tf_list[] = {
        flash_normal_patterns,
        flash_walking_of_1,
        flash_walking_of_0,
        flash_addr_rot,
        flash_com_addr_rot,
    };

    for (i=0; i<ARY_LEN(norsf_tf_list); i++) {
        if (MT_SUCCESS !=
            norsf_tf_list[i](flash_start_addr, test_size_per_pattern, flash_test_size)) {
            HANDLE_FAIL;
        }
    }

    return MT_SUCCESS;
}

int spif_cmd_parsing(int argc, char * const argv[], spif_cmd_parsing_info_t *info)
{
    u32_t i;

    #define ILL_CMD \
    ({ \
        printf("ERR: Illegal command (%d).\n",__LINE__);\
        return MT_FAIL;\
    })

    /* Initialize the memory test parameters..... */
    g_err_handle_block_mode = 0;
    info->test_loops    = 1;

    /* Parse the environment parameter for mt (non-)blocking error mode */
    g_err_handle_block_mode = getenv_ulong("mt_block_e", 10, 0);

    /* Parse command flag for test range / test loops / mt error (non-)blocking mode */
    for(i=1 ; i<argc ;)    {
        if('-' != *argv[i])    ILL_CMD;

        if((strcmp(argv[i],"-loops") == 0) || (strcmp(argv[i],"-l") == 0)){
            if(((i+1) >= argc) || (isxdigit(*argv[i+1])==0))ILL_CMD;
            info->test_loops = simple_strtoul(argv[i+1], NULL, 10);
            info->test_loops = (info->test_loops==0)?1:(info->test_loops);
            i = i+2;
        }else if((strcmp (argv[i], "-block_e") == 0) || (strcmp (argv[i], "-b") == 0)){
                g_err_handle_block_mode = 1;
                i = i+1;
        }else if(strcmp(argv[i],"-reset") == 0){
            g_reset_flag = UBOOT_RESET;
            i = i+1;
        }else if(strcmp(argv[i],"-reset_all") == 0){
            g_reset_flag = WHOLE_CHIP_RESET;
            i = i+1;
        }else if((strcmp (argv[i], "-range") == 0) || (strcmp (argv[i], "-r") == 0)){
            if(((i+1) >= argc) || (isxdigit(*argv[i+1])==0) || ((i+2) >= argc) || (isxdigit(*argv[i+2])==0))ILL_CMD;
            info->mmio_addr = simple_strtoul(argv[i+1], NULL, 10);
            info->mmio_addr = CADDR(info->mmio_addr);
            info->mt_size= simple_strtoul(argv[i+2], NULL, 10);
            i = i+3;
            assign_test_range =1;
        }else{
            ILL_CMD;
        }
    }
    return MT_SUCCESS;
}

int back_up_spif_data(u32_t flash_addr_to_be_backup, u32_t backup_size)
{
    volatile u32_t *bootcode;
    volatile u32_t *bk_buffer;
    u32_t i;

    bk_buffer = (u32_t *)FLASH_BACKUP_ADDR;
    bootcode = (u32_t *)flash_addr_to_be_backup;
    printf("Backing up %dMB flash data: (0x%08x -> 0x%08x)... ",(backup_size/SIZE_1MB),(u32_t)bootcode,(u32_t)bk_buffer);
    for(i=0; i<backup_size; i=i+4){
        *bk_buffer = *bootcode;
        bk_buffer++;
        bootcode++;
    }
    
    bk_buffer = (u32_t *)FLASH_BACKUP_ADDR;
    bootcode = (u32_t *)flash_addr_to_be_backup;
    for(i=0; i<backup_size; i=i+4){
        if(*bk_buffer != *bootcode){
            printf("#Back up flash data error: bk_addr(0x%08x):0x%08x != flash_addr(0x%08x):0x%08x\n",\
            (u32_t)bk_buffer, *bk_buffer, (u32_t)bootcode, *bootcode);
            return MT_FAIL;            
        }
        bk_buffer++;
        bootcode++;
    }
    puts("done\n");
    return MT_SUCCESS;
}

int restore_spif_data(u32_t flash_addr_to_be_backup, u32_t backup_size)
{
    volatile u32_t *bootcode;
    volatile u32_t *bk_buffer;
    int i;
    ulong addr = (ulong)flash_addr_to_be_backup;
    ulong size = (ulong)backup_size;

    printf("Recover %dMB flash data (0x%08x -> 0x%08x) ... ", (backup_size/SIZE_1MB), (u32_t)FLASH_BACKUP_ADDR,(u32_t)addr);
    flash_sect_erase(addr, (addr+size-1));
    flash_write((char *)FLASH_BACKUP_ADDR, addr, size);
       
    bk_buffer = (u32_t *)FLASH_BACKUP_ADDR;
    bootcode = (u32_t *)addr;
    for(i=0; i<backup_size; i=i+4){
        if(*bk_buffer != *bootcode){
        printf("#Recover flash data error: bk_addr(0x%08x):0x%08x != flash_addr(0x%08x):0x%08x\n",\
            (u32_t)bk_buffer, *bk_buffer, (u32_t)bootcode, *bootcode);
            return MT_FAIL;
        }
        bk_buffer++;
        bootcode++;
    }
    if(i == backup_size){
        puts("Verify OK.\n");
        return MT_SUCCESS;
    }
    return MT_FAIL;
}

int _common_flash_test(u32_t flash_addr_to_be_test, u32_t test_size)
{
    if(MT_FAIL == back_up_spif_data(flash_addr_to_be_test, test_size)) return MT_FAIL;
    spi_disable_message(); 
    if(MT_FAIL == _flash_test(flash_addr_to_be_test, TEST_SIZE_PER_PATTREN, test_size)){
        spi_enable_message();
        return MT_FAIL;
    }

    if(MT_FAIL == restore_spif_data(flash_addr_to_be_test, test_size)){
        printf("[ERROR] No recover data\n");
        return MT_FAIL;        
    }

    spi_enable_message();
    return MT_SUCCESS;
}

int flash_test(int flag, int argc, char * const argv[])
{
    int retcode=MT_SUCCESS;
    u32_t i;
    spif_cmd_parsing_info_t cmd_info;
    u32_t mt_size;
    u32_t mt_start;
    u32_t backup_size;

    if(MT_FAIL == spif_cmd_parsing(argc, argv, &cmd_info)) return MT_FAIL;

    flash_size = norsf_info.size_per_chip_b * norsf_info.num_chips;
    
    printf("[mflash_test]: flash_size is %2dMB\n",(int)(flash_size/SIZE_1MB));

    if(assign_test_range){
        mt_size = cmd_info.mt_size;
        mt_start= cmd_info.mmio_addr;
        backup_size = mt_size;
        printf("               backup_size is %2d MB\n",backup_size/SIZE_1MB);
        printf("               test_size   is %2d MB\n",mt_size/SIZE_1MB);

        if((initdram(0)-(CONFIG_SYS_TEXT_BASE&0x1FFFFFFF)) > (mt_size+TEST_SIZE_PER_PATTREN+RSV_SPACE)){
            flash_backup_addr = CONFIG_SYS_TEXT_BASE+RSV_SPACE;
        }else if((CONFIG_SYS_TEXT_BASE&0x1FFFFFFF) > (mt_size+TEST_SIZE_PER_PATTREN+RSV_SPACE)){
            flash_backup_addr = CONFIG_SYS_TEXT_BASE-mt_size-TEST_SIZE_PER_PATTREN-RSV_SPACE;
        }else if((mt_size == flash_size) && (mt_size >= initdram(0))){
            backup_size = 16*1024*1024;
            printf(" Warning: Assigned test size is %d MB, DRAM_Size is %d MB => Only backup 16MB flash content\n", (mt_size/SIZE_1MB), (initdram(0)/SIZE_1MB)); 
        }else{
            printf("[Error] No enough space to back up the SPI-Flash data:\n");    
            return MT_FAIL;
        }
        src_data_addr = flash_backup_addr + backup_size;


        /* Step1: Back up and verify the assigned spif area data. */
        back_up_spif_data(mt_start, backup_size);   

        spi_disable_message();
        for(i=0;i<cmd_info.test_loops;i++){
            /* Step2: Do mflash_test */
            if(MT_FAIL == _flash_test(mt_start, TEST_SIZE_PER_PATTREN, mt_size)){
                printf("[ERROR] No recover other data over the Assigned Area!\n");
                spi_enable_message();
                return MT_FAIL;
            }
            printf("\n[mflash_test] %d runs OK\n",i);       
        }

        /* Step3: Copy back and verify data into the flash. */
        retcode = restore_spif_data(mt_start, backup_size);
        if(MT_SUCCESS == retcode) goto restore_setting;          

    }else{ //(!assign_test_range)
        mt_size = SIZE_2MB;

        if((initdram(0)-(CONFIG_SYS_TEXT_BASE&0x1FFFFFFF)) > (mt_size+TEST_SIZE_PER_PATTREN+RSV_SPACE)){
            flash_backup_addr = CONFIG_SYS_TEXT_BASE+RSV_SPACE;
        }else if((CONFIG_SYS_TEXT_BASE&0x1FFFFFFF) > (mt_size+TEST_SIZE_PER_PATTREN+RSV_SPACE)){
            flash_backup_addr = CONFIG_SYS_TEXT_BASE-mt_size-TEST_SIZE_PER_PATTREN-RSV_SPACE;
        }else{
            printf("[Error] No enough space to back up the SPI-Flash data:\n");    
            return MT_FAIL;
        }
        src_data_addr = flash_backup_addr + mt_size;

        for (i=0; i<cmd_info.test_loops; i++) {
            u32_t mt_start_mb_list[] = {64*SIZE_1MB, 48*SIZE_1MB, 32*SIZE_1MB,
                                        18*SIZE_1MB, 16*SIZE_1MB, 8*SIZE_1MB,
                                        4*SIZE_1MB};
            u32_t msl_idx;
            u32_t part_num = 0;

            for (msl_idx=0; msl_idx < ARY_LEN(mt_start_mb_list); msl_idx++) {
                if (mt_start_mb_list[msl_idx] > flash_size) {
                    continue;
                }

                mt_start = FLASHBASE + mt_start_mb_list[msl_idx] - mt_size;
                printf("\nRegion(%d): 0x%08x ~ 0x%08x\n", part_num++, mt_start, (mt_start+mt_size));

                if (MT_FAIL == _common_flash_test(mt_start, mt_size)) {
                    return MT_FAIL;
                }
            }

            printf("\n[mflash_test] %d runs OK\n", i+2);
        }
    }

restore_setting:
    spi_enable_message();
    assign_test_range = 0;
    
    /* Reset if the command is sent from the command line */
    if(UBOOT_RESET == g_reset_flag){
        do_reset (NULL, 0, 0, NULL);
    }else if(WHOLE_CHIP_RESET == g_reset_flag){    
        SYSTEM_RESET();
    }
    return retcode;
}

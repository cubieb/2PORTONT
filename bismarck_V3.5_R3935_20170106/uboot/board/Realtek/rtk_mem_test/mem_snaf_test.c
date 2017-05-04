#include <common.h>
#include <soc.h>
#include <malloc.h>
#include <asm/otto_spi_nand_flash.h>
#include <spi_nand/spi_nand_blr_util.h>

u32_t pattern[] = {
    0x5a5aa5a5,
    0xff00ff00,
    0xa55aa55a,
    0x00ff00ff,
    0xa5a5a5a5,
    0x13572468,
    0x5a5a5a5a,
    0x24683579,
    0xace1ace1,
    0xffff0000, 
    0x0000ffff,
    0x5555aaaa,
    0xaaaa5555,
    0x0f1e2d3c,
    0x01010101,
    0xFEFEFEFE
};

typedef struct {
	u32_t loops;
    u32_t bbi[4096];
	u32_t start_blk;
	u32_t blk_count;
    u8_t f_reset;
    u8_t f_bad;
} snaf_test_info_t;

extern _spi_nand_info_t _info;
extern int _spi_nand_check_ecc(int r);
#define _cache_flush	(((soc_t *)(0x9fc00020))->bios).dcache_writeback_invalidate_all
#define SET_SEED 0
#define GET_SEED 1
/*
  get_or_set = GET_SEED: get seed
  get_or_set = SET_SEED: set seed
*/
static void __srandom32(u32_t *a1, u32_t *a2, u32_t *a3, u32_t get_or_set)
{
    static int s1, s2, s3;
    if(GET_SEED==get_or_set){
        *a1=s1;
        *a2=s2;
        *a3=s3;
    }else{
        s1 = *a1;
        s2 = *a2;
        s3 = *a3;
    }
}

static u32_t __random32(void)
{
#define TAUSWORTHE(s,a,b,c,d) ((s&c)<<d) ^ (((s <<a) ^ s)>>b)
    u32_t s1, s2, s3;
    __srandom32(&s1, &s2, &s3, GET_SEED);

    s1 = TAUSWORTHE(s1, 13, 19, 4294967294UL, 12);
    s2 = TAUSWORTHE(s2, 2, 25, 4294967288UL, 4);
    s3 = TAUSWORTHE(s3, 3, 11, 4294967280UL, 17);

    __srandom32(&s1, &s2, &s3, SET_SEED);

    return (s1 ^ s2 ^ s3);
}

extern _spi_nand_info_t _info;

#define BLOCK_OFFSET    (6)
#define ROW_ADDR(b, p)  ((b<<BLOCK_OFFSET)|p)

#define BUF_RST_VAL     (0xCAFEBEEF)
#define START_BLOCK     (0)


u32_t snaf_pio_test(spi_nand_flash_info_t *fi, snaf_test_info_t *ti)
{
    if (VZERO == fi) return 1;
    
    u32_t pat[528], buf[528], i;
    u32_t b, p=0, factor;  // block, page(chunk) and column address
    u32_t psize, sb, nb, np;
    
    sb = ti->start_blk;
    nb = ti->start_blk + ti->blk_count;
    np = SNAF_NUM_OF_PAGE_PER_BLK(fi);
    psize = (fi->_page_size + fi->_spare_size) >> 2;        // 2048 / 4
    
    puts("II: do spi-nand pio test... write... ");
    
    _cache_flush();
    for(b=sb; b<nb; b++) {
        // bad block check
        if (1==ti->bbi[b]) continue;
        
        printf("BLK%04x\b\b\b\b\b\b\b", b);        
        factor=b%13;
        
        // pattern generate
        for(i=0; i<psize; i++) {
           *(pat+i)=(pattern[(i&0xF)]<<factor)|(pattern[(i&0xF)]>>(32-factor));
           //*(pat+i)=pattern[(i&0xF)];
        }
        // verify pattern;
        for(i=0; i<psize; i++) {
           if(((pattern[(i&0xF)]<<factor)|(pattern[(i&0xF)]>>(32-factor))) != *(pat+i)) {
           //if(pattern[(i&0xF)] != *(pat+i) ) {
               puts("\nEE: pattern check failed!\n"); return 1;
           }
        }
        // target block erase, 128KB
        nasu_block_erase(ROW_ADDR(b, 0));

        for(p=0; p<np; p++) {
            // pattern write
            nasu_pio_write(pat, psize*4, ROW_ADDR(b, p), 0);
        }
    }    
    puts("verify... ");
    _cache_flush();
    for(b=sb; b<nb; b++) {
        // bad block check
        if (1==ti->bbi[b]) continue;
        
        printf("BLK%04x\b\b\b\b\b\b\b", b);        
        factor=b%13;
        
        // pattern generate
        for(i=0; i<psize; i++) {
           *(pat+i)=(pattern[(i&0xF)]<<factor)|(pattern[(i&0xF)]>>(32-factor));
           //*(pat+i)=pattern[(i&0xF)];
        }
        // verify pattern;
        for(i=0; i<psize; i++) {
           if(((pattern[(i&0xF)]<<factor)|(pattern[(i&0xF)]>>(32-factor))) != *(pat+i)) {
           //if(pattern[(i&0xF)] != *(pat+i) ) {
               puts("\nEE: pattern check failed!\n"); return 1;
           }
        }

        for(p=0; p<np; p++) {
            // cleanup buf
            for(i=0; i<psize; i++) {
                *(buf+i) = BUF_RST_VAL;
            }
            // read page/chunk/column
            nasu_pio_read(buf, psize*4, ROW_ADDR(b, p), 0);
 
            for(i=0; i<psize; i++) {
                if( *(pat+i) != *(buf+i) ) { 
                    printf("\nEE: PAGE%x,COL%x,data:0x%08x != pattern:0x%08x\n", p, i<<2, *(buf+i), *(pat+i));
                    return 1;
                }
            }
        }
    
    }
    puts("done       \n");
    return 0;
}

u32_t snaf_dma_test(spi_nand_flash_info_t *fi, snaf_test_info_t *ti)
{
    if (VZERO == fi) return 1;
    //volatile u32_t *sAddr, *dAddr;
    u32_t b, p;
    u32_t i, ra, rb, rc, v;
    u32_t psize, sb, nb, np;    
    u32_t sAddr[528], dAddr[528];
    
    psize = (fi->_page_size + fi->_spare_size);   // 2048 + 64 Bytes
    
    sb = ti->start_blk;
    nb = ti->start_blk+ti->blk_count;
    np = SNAF_NUM_OF_PAGE_PER_BLK(fi); //fi->_num_page_per_block;
    psize = psize >> 2; // for loop use
    
    printf("II: do spi-nand dma test (0x%x->0x%x)... write... ", (u32_t)sAddr, (u32_t)dAddr);
    
#define SEED1   ((0x13243*(b+1))&0xffffff)
#define SEED2   (0xaaa0bdd+b)
#define SEED3   (0xfffbda0-b)

    _cache_flush();
    for(b=sb; b<nb; b++) {
        printf("BLK%04x\b\b\b\b\b\b\b", b);
        
        // bad block check
        if (1==ti->bbi[b]) continue;
        
        // generate random patterns
        ra=SEED1;rb=SEED2;rc=SEED3;
        __srandom32(&ra, &rb, &rc, SET_SEED);
        for(i=0; i<psize; i++) {
            v = __random32();
            *(sAddr+i) = v;
        }
    
        // verify random patterns
        ra=SEED1;rb=SEED2;rc=SEED3;
        __srandom32(&ra, &rb, &rc, SET_SEED);
        for(i=0; i<psize; i++) {
            v = __random32();
            if(v!= *(sAddr+i)) {
                printf("EE: pattern write to addr(0x%x) 0x%x!=0x%x\n", (u32_t)(sAddr+i), *(sAddr+i), v);
                return 1;
            }
        }

        // target block erase, 128KB
        nasu_block_erase(ROW_ADDR(b, 0));
    
        for(p=0; p<np; p++) {
            // dma write
            nasu_page_write(sAddr, ROW_ADDR(b, p));
        }
    }
    puts("verify... ");
    _cache_flush();
    for(b=sb; b<nb; b++) {
        // bad block check
        if (1==ti->bbi[b]) continue;
        printf("BLK%04x\b\b\b\b\b\b\b", b);       
        
        // generate random patterns
        ra=SEED1;rb=SEED2;rc=SEED3;
        __srandom32(&ra, &rb, &rc, SET_SEED);
        for(i=0; i<psize; i++) {
            v = __random32();
            *(sAddr+i) = v;
        }
    
        for(p=0; p<np; p++) {
            // dma read
            nasu_page_read(dAddr, ROW_ADDR(b, p));
    
            // data verify
            for(i=0; i<psize; i++) {
                if(*(sAddr+i)!= *(dAddr+i)) {
                    printf("\nEE: double confirm, BLK%x,PAGE%x,COL%x,\
                            dst(addr:0x%x)0x%x != src(addr:0x%x)0x%x\n", 
                            b, p, i<<2, (u32_t)(dAddr+i), *(dAddr+i), (u32_t)(sAddr+i), *(sAddr+i));
                    return 1;
                }
            }
        }
    }    
    puts("done       \n");
    return 0;
}

u32_t snaf_pio_dma_cross_test(spi_nand_flash_info_t *fi, snaf_test_info_t *ti)
{
    if (VZERO == fi) return 1;
    u32_t b, p, i;
    u32_t psize, sb, nb, np;    
    u32_t pat[528], buf[528], addr[528];
    
    psize = (fi->_page_size + fi->_spare_size) >> 2;
    sb = ti->start_blk;
    nb = ti->start_blk+ti->blk_count;
    np = SNAF_NUM_OF_PAGE_PER_BLK(fi);
    
    puts("II: do dma/pio cross test... ");

    for(b=sb; b<nb; b++) {
        printf("BLK%04x\b\b\b\b\b\b\b", b);
        
        // bad block check
        if (1==ti->bbi[b]) continue;
        // generate address rotate data
        for(i=0; i<psize; i++) {
            *(pat+i) = pattern[(i&0xF)];
            //address rotate ((u32_t)(addr+i)<<(b&0x1F))|((u32_t)(addr+i)>>(32-(b&0x1F)));
            *(addr+i) = *(pat+i);
        }

        // verify patterns
        for(i=0; i<psize; i++) {
            if(*(pat+i) != *(addr+i)) {
                printf("EE: pattern write to addr(0x%x) 0x%x!=0x%x\n", (u32_t)(addr+i), *(addr+i), *(pat+i));
                return 1;
            }
        }

        // do dma write and pio read back
        nasu_block_erase(ROW_ADDR(b, 0));    
        for(p=0; p < np; p++) {
     
            // dma write
            nasu_page_write(addr, ROW_ADDR(b, p));
            
            // reset buf
            for(i=0; i<psize; i++) {
                *(buf+i) = BUF_RST_VAL;
            }
            // pio read
            nasu_pio_read(buf, psize*4, ROW_ADDR(b, p), 0);
 
            for(i=0; i<psize; i++) {
                if( *(addr+i) != *(buf+i) ) { 
                    printf("\nEE: dw-pr, PAGE%x,COL%x,data:0x%08x != pattern:0x%08x\n", p, i<<2, *(buf+i), *(addr+i));
                    return 1;
                }
            }
        }

        // do pio write and dma read back
        nasu_block_erase(ROW_ADDR(b, 0));    
        for(p=0; p < np; p++) {
            // pio write
            nasu_pio_write(pat, psize*4, ROW_ADDR(b, p), 0);
            
            // reset dma addr buf
            for(i=0; i<psize; i++) {
                *(addr+i) = BUF_RST_VAL;
            }
            
            // dma read
            nasu_page_read(addr, ROW_ADDR(b, p));
            
            for(i=0; i<psize; i++) {
                if(*(pat+i)!= *(addr+i)) {
                    printf("\nEE: pw-dr, PAGE%x,COL%x\
                            ,dst(addr:0x%x)0x%x != pat 0x%x\n", 
                            p, i<<2, (u32_t)(addr+i), *(addr+i), *(pat+i));
                    return 1;
                }
            }
        }
    }
    puts("done       \n");
    return 0;
}

u32_t snaf_ecc_normal_test(spi_nand_flash_info_t *fi, snaf_test_info_t *ti)
{
    u32_t b, p, i;
    u32_t psize, sb, nb, np; 
    u32_t eccb[4], addr[528], target[528];
    
    psize = (fi->_page_size) >> 2;
    sb = ti->start_blk;
    nb = ti->start_blk+ti->blk_count;
    np = SNAF_NUM_OF_PAGE_PER_BLK(fi);
    
    puts("II: do ecc normal test... write... ");
    for(b=sb; b<nb; b++) {
        // bad block check
        if (1==ti->bbi[b]) continue;
        printf("BLK%04x\b\b\b\b\b\b\b", b);
        
        // generate patterns
        for(i=0; i<psize; i++) {
            *(addr+i) = pattern[(i&0xF)];
        }
    
        // verify patterns
        for(i=0; i<psize; i++) {
            if(pattern[(i&0xF)] != *(addr+i)) {
                printf("EE: pattern write to addr(0x%x) 0x%x!=0x%x\n", 
                        (u32_t)(addr+i), *(addr+i), pattern[(i&0xF)]);
                return 1;
            }
        }
        
        nasu_block_erase(ROW_ADDR(b, 0));  
        
        for(p=0; p < np; p++) {
            nasu_page_write_ecc(addr, ROW_ADDR(b, p), eccb);
        }        
    }  
    
    puts(" verify... ");
    for(b=sb; b<nb; b++) {
        // bad block check
        if (1==ti->bbi[b]) continue;
        printf("BLK%04x\b\b\b\b\b\b\b", b);
        
        // generate patterns
        for(i=0; i<psize; i++) {
            *(addr+i) = pattern[(i&0xF)];
        }
    
        // verify patterns
        for(i=0; i<psize; i++) {
            if(pattern[(i&0xF)] != *(addr+i)) {
                printf("EE: pattern write to addr(0x%x) 0x%x!=0x%x\n", 
                        (u32_t)(addr+i), *(addr+i), pattern[(i&0xF)]);
                return 1;
            }
        }
        
        for(p=0; p < np; p++) {
            // data check
            u32_t ret = nasu_page_read_ecc(target, ROW_ADDR(b, p), eccb);
            
            // verify data
            for(i=0; i<psize; i++) {
                if(*(target+i)!= *(addr+i)) {
                    printf("EE: BLK%x, PAGE%x, read data 0x%x!=0x%x, ecc(0x%x)\n",
                            b, p, *(target+i), *(addr+i), _spi_nand_check_ecc(ret));
                    return 1;
                }
            }
            
        }        
    }      
    puts("done       \n");
    
    return 0;
}

u32_t snaf_ecc_trick_test(spi_nand_flash_info_t *fi, snaf_test_info_t *ti)
{
    u32_t b, p, s, i;
    int ret;
    u32_t psize, sb, nb, np; 
    u32_t eccb[4], addr[528], target[528];
    u32_t buf;
    u32_t p_buf[] = {0x1FFFFF1F, 0xFBEC3FFF, 0xFFFFC47F, 0xFBEF7BEE, 0xBEB6F7EF, 0x7BB6F7EF, 0xE1FE1FFF, 0xDBF7EEFD };
    
    psize = (fi->_page_size) >> 2;
    sb = ti->start_blk;
    nb = ti->start_blk+ti->blk_count;
    np = SNAF_NUM_OF_PAGE_PER_BLK(fi); //fi->_num_page_per_block;
    
    puts("II: do ecc trick test... ");
    for(b=sb; b<nb; b++) {
        // bad block check
        if (1==ti->bbi[b]) continue;
        printf("BLK%04x\b\b\b\b\b\b\b", b);
        
        // generate patterns
        for(i=0; i<psize; i++) {
            *(addr+i) = pattern[(i&0xF)];
        }
    
        // verify patterns
        for(i=0; i<psize; i++) {
            if(pattern[(i&0xF)] != *(addr+i)) {
                printf("EE: pattern write to addr(0x%x) 0x%x!=0x%x\n", 
                        (u32_t)(addr+i), *(addr+i), pattern[(i&0xF)]);
                return 1;
            }
        }
        
        nasu_block_erase(ROW_ADDR(b, 0));  
        
        for(p=0; p < np; p++) {
            nasu_page_write_ecc(addr, ROW_ADDR(b, p), eccb);

            // data check
            ret=nasu_page_read_ecc(target, ROW_ADDR(b, p), eccb);
            
            // verify data
            for(i=0; i<psize; i++) {
                if(*(target+i)!= *(addr+i)) {
                    printf("EE: BLK%x, PAGE%x, read data 0x%x!=0x%x, ecc(0x%x)\n",
                            b, p, *(target+i), *(addr+i), _spi_nand_check_ecc(ret));
                    return 1;
                }
            }
           
            // lets do something bad
            // pick up sector for pollute
            s = (pattern[(p+b)&0xF]&0x3)*512;
            nasu_pio_read(&buf, 4, ROW_ADDR(b, p), s);
            //printf("DD: before buf=0x%x\n", buf);
            buf = buf & p_buf[(p&0x7)];
            //printf("DD: after buf=0x%x\n", buf);
            
            nasu_pio_write(&buf, 4, ROW_ADDR(b, p), s);
            buf = 0; // cleanup
            //nasu_pio_read(&buf, 4, ROW_ADDR(b, p), s);
            
            // data check
            ret=nasu_page_read_ecc(target, ROW_ADDR(b, p), eccb);
            //printf("DD: chunk read =0x%x\n", *(target+128));

            if(0!=_spi_nand_check_ecc(ret))
                printf("EE: ecc correct error (0x%x)\n", ret);
        }        
    }  
    puts("done       \n");
    
    return 0;
}

u32_t snaf_bad_block_search(spi_nand_flash_info_t *fi, snaf_test_info_t *ti) 
{
    u32_t b, nb, sb;
    u32_t buf[2];
    sb = ti->start_blk; // start block
    nb = ti->start_blk+ti->blk_count;
    
    puts("II: do bad block search ");
    
    for(b=sb;b<nb;b++){ //block_num
        // performing erase
        if(0!=nasu_block_erase(ROW_ADDR(b, 0))) {
            ti->bbi[b]=1;
        } else ti->bbi[b]=0;
	}
    
    for(b=sb; b<nb; b++) {
        printf("BLK%04x\b\b\b\b\b\b\b", b);
        nasu_pio_read(buf, 4, ROW_ADDR(b, 0), 0x800);
        if(0xFFFFFFFF != buf[0]) {
            ti->bbi[b]=1; printf("BLK%04x ", b);
        } else ti->bbi[b]=0;
    }

    puts("...done\n");
    return 0;
}

enum cmd_error {
    ERR_INVALD_CMD,
    ERR_LOOP_CMD,
    ERR_RANGE_CMD,
    ERR_START_ADDR,
    ERR_SIZE,
};

enum reset_type {
    RESET_NONE=0,
    RESET_UBOOT,
    RESET_CHIP,
};

int cmd_parsing(int argc, char * const argv[], snaf_test_info_t *ti) 
{
	u32_t i=1, err;

    /* init value */
	while(i<argc) {
		if ('-' == argv[i][0]) {
            if( 0==(strcmp(argv[i],"-l")) || 0==(strcmp(argv[i],"-loops")) ) {
                u32_t loop;
                if((i+1) >= argc) { err=ERR_LOOP_CMD; goto error; }
                loop = simple_strtoul(argv[i+1], NULL, 10);
                if (0==loop) { puts("WW: invalid loop count(reset to 1)\n"); loop=1; }
                ti->loops = loop;
                i+=2;
            } else if (0==(strcmp(argv[i],"-r")) || 0==(strcmp(argv[i],"-range")) ) {
                u32_t blk, cnt;
                if((i+2) >= argc) { err=ERR_RANGE_CMD; goto error; }
                blk = simple_strtoul(argv[i+1], NULL, 10);
                cnt = simple_strtoul(argv[i+2], NULL, 10);
                printf("DD: input range %d %d\n", blk, cnt);
                if(0==blk || blk >= _info.block_size ) { err=ERR_START_ADDR; goto error; }
                if(0==cnt || cnt >= _info.block_size ) { err=ERR_SIZE; goto error; }
                ti->start_blk=blk;
                ti->blk_count=cnt;
                i+=3;
            } else if(0==strcmp(argv[i],"-reset")) {
                ti->f_reset = RESET_UBOOT;
                i++;
            } else if(0==strcmp(argv[i],"-reset_all")){
                ti->f_reset = RESET_CHIP;
                i++;
            } else if(0==strcmp(argv[i],"-bad")){
                ti->f_bad = 1;
                i++;
            } else {
                printf("WW: unknown command \"%s\" ignored.\n", argv[i]);
                i++;
            }
        } else {
            err = ERR_INVALD_CMD; goto error;
        }
	}

    if (0==ti->blk_count) { puts("EE: input error\n"); return 1;}
    
    return 0;
error:
    printf("EE: incomplete commands (type: %d)\n", err);
    return -1;	
}

int snaf_test(spi_nand_flash_info_t *fi, snaf_test_info_t *ti)
{
    if (0!=snaf_pio_test(fi, ti)
        || 0!=snaf_dma_test(fi, ti)
        || 0!=snaf_pio_dma_cross_test(fi, ti)
        || 0!=snaf_ecc_normal_test(fi, ti)
//        || 0!=snaf_ecc_trick_test(fi, ti)
        )
    { puts("EE: spi-nand flash test failed\n"); return 1;}
    
    return 0;
    
}

int do_snaf_test (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    u32_t i=1;
    snaf_test_info_t ti={0};
    spi_nand_flash_info_t *fi = _plr_spi_nand_info;    
    ti.loops = 1;
    
    if (0!=cmd_parsing(argc, argv, &ti)) return 1;
    printf("II: spi_nand test info:\n\tstart: BLK%04x, end: BLK%04x\n", 
            ti.start_blk, ti.start_blk+ti.blk_count-1);
    printf("\tloop: %d, reset type: %s\n\tbad block search only: %c\n",
            ti.loops, ti.f_reset?(RESET_UBOOT==ti.f_reset?"reset":"reset_all"):"none", ti.f_bad?'y':'n');

    snaf_bad_block_search(fi, &ti);
    if (ti.f_bad) return 0;
    
    while(i<=ti.loops) {
        printf("II: #%d test\n", i++);
        if(snaf_test(fi, &ti)) {return -1;}
    }
    if(RESET_UBOOT == ti.f_reset){
        do_reset (NULL, 0, 0, NULL);
    } else if (RESET_CHIP ==  ti.f_reset){
        SYSTEM_RESET();
    } 
    return 0;
}

U_BOOT_CMD(
        msnaf_test, 10, 1, do_snaf_test,
        "msnaf_test  - do spi-nand flash test. ",
        "-r/-range <start block> <block count> [-l/-loops <test loops>] [-reset/-reset_all] [-bad]\n"\
        "    - do spi-nand flash test."
);

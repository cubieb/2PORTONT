#include <util.h>
#include <soc_exam_util/mt_ram_test.h>

const u32_t mt_patterns[] = {
        0x00000000,
        0xffffffff,
        0x55555555,
        0xaaaaaaaa,
        0x01234567,
        0x76543210,
        0x89abcdef,
        0xfedcba98,
        0xA5A5A5A5,
        0x5A5A5A5A,
        0xF0F0F0F0,
        0x0F0F0F0F,
        0xFF00FF00,
        0x00FF00FF,
        0x0000FFFF,
        0xFFFF0000,
        0x00FFFF00,
        0xFF0000FF,
        0x5A5AA5A5,
        0xA5A55A5A,
};

int mt_normal_patterns(u32_t ram_start_addr, u32_t size_per_pat, u32_t ram_test_range, enum WR_ADDR_CASE wr_case)
{
    u32_t i, j, offsets=0;
    u32_t w_start=0;
    u32_t r_start=0;
    u32_t read_val;
    volatile u32_t *w_test_addr;
    volatile u32_t *r_test_addr;

    printf("II: %s... ", __FUNCTION__);
    w_start = (!(wr_case>>1))?UADDR(ram_start_addr):CADDR(ram_start_addr);
    r_start = (!(wr_case&0x1))?UADDR(ram_start_addr):CADDR(ram_start_addr);
    

    for(i=0; i<(sizeof(mt_patterns)/sizeof(u32_t)) ; i++, offsets=(offsets+size_per_pat)%(ram_test_range)){
        w_test_addr = (u32_t *)(w_start + offsets);
        r_test_addr = (u32_t *)(r_start + offsets + size_per_pat-4);

        dcache_wr_inv_all();
        /* write pattern*/
        for(j=0; j < size_per_pat; j=j+4){
            *w_test_addr = mt_patterns[i];
            w_test_addr++;
        }

        dcache_wr_inv_all();
        /* check data */  
        for(j=0; j < size_per_pat; j=j+4){
            read_val = (*r_test_addr);
            if(read_val != mt_patterns[i]){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)r_test_addr,read_val,mt_patterns[i]);
                return MT_FAIL;
            }
            r_test_addr--;
        }
        //printf("\rpattern[%d](0x%x) W(0x%08x) R(0x%08x) pass",i,mt_patterns[i],(u32_t)w_test_addr,(u32_t)r_test_addr);
    }
    //printf("...[%s] test completed.\n", __FUNCTION__);
    puts(" passed\n");
    return MT_SUCCESS;
}


u32_t rotate_1bit(u32_t v) 
{
    return (((v) << 1) | ((v & 0x80000000)>>31));
}


int mt_word_pattern_rotate(u32_t ram_start_addr, u32_t size_per_pat, u32_t ram_test_range, enum WR_ADDR_CASE wr_case)
{
    u32_t i, j, offsets=0;
    u32_t mt_pat32_rot, wpat1, read1;
    u32_t w_start = 0;
    u32_t r_start = 0;
    volatile u32_t *w_test_addr;
    volatile u32_t *r_test_addr;

    printf("II: %s... ", __FUNCTION__);
    w_start = (!(wr_case>>1))?CADDR(ram_start_addr):UADDR(ram_start_addr);
    r_start = (!(wr_case&0x1))?CADDR(ram_start_addr):UADDR(ram_start_addr);
    
    for(i=0; i<(sizeof(mt_patterns)/sizeof(u32_t)); i++, offsets=(offsets+size_per_pat)%(ram_test_range)){
        mt_pat32_rot = rotate_1bit(mt_patterns[i]);
        w_test_addr = (volatile u32_t *)(w_start  + offsets);
        r_test_addr = (volatile u32_t *)(r_start  + offsets);

        dcache_wr_inv_all();
        /*Write Word*/
        for(j=0; j<size_per_pat ; j=j+4){
            wpat1 = PADDR(w_test_addr)^mt_pat32_rot;
            *w_test_addr++ = wpat1;
         }

        dcache_wr_inv_all();
        /*Read Word*/
        for(j=0; j<size_per_pat ; j=j+4){
            wpat1 = PADDR(r_test_addr)^mt_pat32_rot;
            read1 = *r_test_addr;
            if(read1 != wpat1){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)r_test_addr,read1,wpat1);
                return MT_FAIL;
            }
            r_test_addr++;
        }
        //printf("\rpattern[%d](0x%x) W(0x%08x) R(0x%08x) pass",i,mt_patterns[i],(u32_t)w_test_addr,(u32_t)r_test_addr);
    }
    puts(" passed\n");
    return MT_SUCCESS;
}

int mt_halfword_pattern_rotate(u32_t ram_start_addr, u32_t size_per_pat, u32_t ram_test_range, enum WR_ADDR_CASE wr_case)
{
    u32_t i, j, offsets=0;
    u32_t mt_pat32_rot, patTrans32;
    u32_t w_start = 0;
    u32_t r_start = 0;
    volatile u16_t *w_test_addr;
    volatile u16_t *r_test_addr;
    u16_t wpat1, wpat2, read1, read2;

    printf("II: %s... ", __FUNCTION__);
    w_start = (!(wr_case>>1))?UADDR(ram_start_addr):CADDR(ram_start_addr);
    r_start = (!(wr_case&0x1))?UADDR(ram_start_addr):CADDR(ram_start_addr);
    
    for(i=0; i<(sizeof(mt_patterns)/sizeof(u32_t)) ; i++, offsets=(offsets+size_per_pat)%(ram_test_range)){
        mt_pat32_rot = rotate_1bit(mt_patterns[i]);
        w_test_addr = (volatile u16_t *)(w_start  + offsets);
        r_test_addr = (volatile u16_t *)(r_start  + offsets);

        dcache_wr_inv_all();
        /*Write Half-Word*/
        for(j=0; j<size_per_pat ; j=j+4){
            patTrans32 = PADDR(w_test_addr)^mt_pat32_rot;
            wpat1 = (u16_t)(patTrans32>>0);
            wpat2 = (u16_t)(patTrans32>>16);
            *(w_test_addr++) = wpat1;
            *(w_test_addr++) = wpat2;
        }
        
        dcache_wr_inv_all();
        /*Read  Half-Word*/
        for(j=0; j<size_per_pat ; j=j+4){
            patTrans32 = PADDR(r_test_addr)^mt_pat32_rot;
            wpat1 = (u16_t)(patTrans32>>0);
            wpat2 = (u16_t)(patTrans32>>16);
            
            read1 = *(r_test_addr);
            if(read1 != wpat1){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)r_test_addr,read1,wpat1);
                return MT_FAIL;
            }
            read2 = *(++r_test_addr);
            if(read2 != wpat2){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)r_test_addr,read2,wpat2);
                return MT_FAIL;
            }
            r_test_addr++;
        }
        //printf("\rpattern[%d](0x%x) W(0x%08x) R(0x%08x) pass",i,mt_patterns[i],(u32_t)w_test_addr,(u32_t)r_test_addr);
    }
    puts(" passed\n");
    return MT_SUCCESS;
}

int mt_byte_pattern_rotate(u32_t ram_start_addr, u32_t size_per_pat, u32_t ram_test_range, enum WR_ADDR_CASE wr_case)
{
    u32_t i, j, offsets=0;
    u32_t mt_pat32_rot, patTrans32;
    u32_t w_start = 0;
    u32_t r_start = 0;
    volatile u8_t *w_test_addr;
    volatile u8_t *r_test_addr;
    u8_t wpat1, wpat2, wpat3, wpat4, read1, read2, read3, read4;

    printf("II: %s... ", __FUNCTION__);
    w_start = (!(wr_case>>1))?UADDR(ram_start_addr):CADDR(ram_start_addr);
    r_start = (!(wr_case&0x1))?UADDR(ram_start_addr):CADDR(ram_start_addr);
    
    for(i=0; i<(sizeof(mt_patterns)/sizeof(u32_t)) ; i++, offsets=(offsets+size_per_pat)%(ram_test_range)){
        mt_pat32_rot = rotate_1bit(mt_patterns[i]);
        w_test_addr = (volatile u8_t *)(w_start  + offsets);
        r_test_addr = (volatile u8_t *)(r_start  + offsets);

        dcache_wr_inv_all();
        /*Write Byte*/
        for(j=0; j<size_per_pat ; j=j+4){
            patTrans32 = PADDR(w_test_addr)^mt_pat32_rot;
            wpat1 = (u8_t)(patTrans32>>0);
            wpat2 = (u8_t)(patTrans32>>8);
            wpat3 = (u8_t)(patTrans32>>16);
            wpat4 = (u8_t)(patTrans32>>24);
            *(w_test_addr++) = wpat1;
            *(w_test_addr++) = wpat2;
            *(w_test_addr++) = wpat3;
            *(w_test_addr++) = wpat4;
        }
        
        dcache_wr_inv_all();
        /*Read Byte*/
        for(j=0; j<size_per_pat ; j=j+4){
            patTrans32 = PADDR(r_test_addr)^mt_pat32_rot;
            wpat1 = (u8_t)(patTrans32>>0);
            wpat2 = (u8_t)(patTrans32>>8);
            wpat3 = (u8_t)(patTrans32>>16);
            wpat4 = (u8_t)(patTrans32>>24);
            
            read1 = *(r_test_addr);
            if(read1 != wpat1){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)r_test_addr,read1,wpat1);
                return MT_FAIL;
            }
            read2 = *(++r_test_addr);
            if(read2 != wpat2){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)r_test_addr,read2,wpat2);
                return MT_FAIL;
            }
            read3 = *(++r_test_addr);
            if(read3 != wpat3){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)r_test_addr,read3,wpat3);
                return MT_FAIL;
            }
            read4 = *(++r_test_addr);
            if(read4 != wpat4){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)r_test_addr,read4,wpat4);
                return MT_FAIL;
            }
            r_test_addr++;
        }
        //printf("\rpattern[%d](0x%x) W(0x%08x) R(0x%08x) pass",i,mt_patterns[i],(u32_t)w_test_addr,(u32_t)r_test_addr);
    }
    puts(" passed\n");
    return MT_SUCCESS;
}



#pragma pack(1)
/* Data structures used for testing unaligned load/store operations. */
typedef struct{
    unsigned char c1;
    unsigned int w;
} t_off_1;

typedef struct{
    unsigned char c1;
    unsigned char c2;
    unsigned int w;
} t_off_2;
typedef struct{
    unsigned char c1;
    unsigned char c2;
    unsigned char c3;
    unsigned int w;
} t_off_3;

#pragma pack(4)
int mt_unaligned_wr(u32_t ram_start_addr, u32_t size_per_pat, u32_t ram_test_range, enum WR_ADDR_CASE wr_case)
{
    u32_t i, j, offsets=0;
    u32_t w_start = 0;
    u32_t r_start = 0;
    volatile t_off_1 *w_off1, *r_off1;
    volatile t_off_2 *w_off2, *r_off2;
    volatile t_off_3 *w_off3, *r_off3;

    printf("II: %s... ", __FUNCTION__);
    w_start = (!(wr_case>>1))?UADDR(ram_start_addr):CADDR(ram_start_addr);
    r_start = (!(wr_case&0x1))?UADDR(ram_start_addr):CADDR(ram_start_addr);

    /* patterns loop */
    for(j=0; j<sizeof(mt_patterns)/sizeof(unsigned int);j++, offsets=(offsets+size_per_pat)%(ram_test_range)){
        /* offset 1 bytes */
          w_off1 = (t_off_1 *)(w_start  + offsets);
          r_off1 = (t_off_1 *)(r_start  + offsets);

        dcache_wr_inv_all();
        /* set value */
        for(i=0;i<(size_per_pat/sizeof(t_off_1));i++){
            w_off1[i].c1 = 0xcc;
            w_off1[i].w = mt_patterns[j];
        }

        dcache_wr_inv_all();
        /* check data */
        for(i=0;i<(size_per_pat/sizeof(t_off_1));i++)
        {
            if((r_off1[i].w != mt_patterns[j]) || (r_off1[i].c1 != 0xcc))
            {
                EPRINTF("offset 1 error:addr(0x%x) write 0x%x, read 0x%x, c1(%02x)\n", \
                    (u32_t)&w_off1[i], mt_patterns[j], r_off1[i].w, r_off1[i].c1);
                return MT_FAIL;
            }
        }
    
        /* offset 2 bytes */       
        w_off2 = (t_off_2 *)(w_start  + offsets);
        r_off2 = (t_off_2 *)(r_start  + offsets);
        dcache_wr_inv_all();        
        /* set value */
        for(i=0;i<(size_per_pat/sizeof(t_off_2));i++){
            w_off2[i].c1 = 0xcc;
            w_off2[i].c2 = 0xdd;
            w_off2[i].w = mt_patterns[j];
        }

        dcache_wr_inv_all();
        /* check data */
        for(i=0;i<(size_per_pat/sizeof(t_off_2));i++){
            if(r_off2[i].w != mt_patterns[j] || (r_off2[i].c1 != 0xcc) || (r_off2[i].c2 != 0xdd)){
                EPRINTF("offset 2 error:addr(0x%x) write 0x%x, read 0x%x, c1(0x%x), c2(0x%x)\n", \
                    (u32_t)&w_off2[i], mt_patterns[j], r_off2[i].w, r_off2[i].c1, r_off2[i].c2);
                return MT_FAIL;
            }
        }

        /* offset 3 bytes */
        w_off3 = (t_off_3 *)(w_start  + offsets);
        r_off3 = (t_off_3 *)(r_start  + offsets);
        
        dcache_wr_inv_all();
        /* set value */
        for(i=0;i<(size_per_pat/sizeof(t_off_3));i++){
            w_off3[i].c1 = 0xcc;
            w_off3[i].c2 = 0xdd;
            w_off3[i].c3 = 0xee;
            w_off3[i].w = mt_patterns[j];
        }

        dcache_wr_inv_all();
        /* check data */
        for(i=0;i<(size_per_pat/sizeof(t_off_3));i++){
            if(r_off3[i].w != mt_patterns[j] ||(r_off3[i].c1 != 0xcc) || (r_off3[i].c2 != 0xdd) || (r_off3[i].c3 != 0xee)){

                EPRINTF("offset 3 error:addr(0x%x) write 0x%x, read 0x%x, c1(%02x), c2(%02x), c3(%02x)\n", \
                    (u32_t)&w_off3[i], mt_patterns[j], r_off3[i].w, r_off3[i].c1, r_off3[i].c2, r_off3[i].c3);
                return MT_FAIL;
            }
        }
        //printf("\rpattern[%d](0x%x) W(0x%08x) R(0x%08x) pass",j,mt_patterns[j], (u32_t)&w_off3[i],(u32_t)&r_off3[i]);
    }
        puts(" passed\n");
     return MT_SUCCESS;
}

int mt_com_addr_rot(u32_t ram_start_addr, u32_t size_per_pat, u32_t ram_test_range)
{
    u32_t i, j;
    u32_t start_value;
    u32_t read_start_addr;
    volatile u32_t *start;
    volatile u32_t *read_start;

    printf("II: %s... ", __FUNCTION__);

    for (i=0; i<32; i=i+4){
        start        = (u32_t *)(ram_start_addr + ((i/4)*size_per_pat)%(ram_test_range));
        read_start     = (u32_t *)((u32_t)start + size_per_pat-4);
        read_start_addr = ((u32_t)start);
        start = (u32_t *)(UADDR(((u32_t)start)));

        /* write pattern*/
        dcache_wr_inv_all();
        for(j=0; j < size_per_pat; j=j+4){
            *start = ~(read_start_addr << i);
            start++;
            read_start_addr = read_start_addr + 4 ;
        }

        read_start_addr = ((u32_t)read_start);
        /* check data reversing order */  
        dcache_wr_inv_all();
        for(j=0; j<size_per_pat; j=j+4){
            start_value = (*read_start);
            if(start_value != (~(read_start_addr << i))){
                EPRINTF("decr addr(0x%x): 0x%x != pattern(0x%x)\n",(u32_t)read_start,start_value,~((read_start_addr) << i));
                return MT_FAIL;

            }
            read_start_addr = read_start_addr - 4;
            read_start--;
        }

        read_start_addr += 4;
        read_start++;

        /* check data sequential order */  
        for(j=0; j<size_per_pat; j=j+4){
            start_value = (*read_start);
            if(start_value != (~(read_start_addr << i))){
                EPRINTF("seq addr(0x%x): 0x%x != pattern(0x%x)\n",(u32_t)read_start,start_value,~((read_start_addr) << i));
                return MT_FAIL;

            }
            read_start_addr = read_start_addr + 4;
            read_start++;
        }
        //printf("\r~rotate %d 0x%x passed", i, (u32_t)start);
    }
    puts(" passed\n");
    return MT_SUCCESS;
}



int mt_walking_of_1(u32_t ram_start_addr, u32_t size_per_pat, u32_t ram_test_range)
{
    u32_t i, j, offsets=0;
    u32_t walk_pattern;
    u32_t start_value;
    volatile u32_t *read_start;
    volatile u32_t *start;

    printf("II: %s... ", __FUNCTION__);
    for (i=0; i < 32; i++, offsets=(offsets+size_per_pat)%(ram_test_range)){
        dcache_wr_inv_all();
        /* generate pattern */
        walk_pattern = (1 << i);

        /* write pattern*/
        start = (u32_t *)(ram_start_addr + offsets);
        read_start = (u32_t *)(UADDR((u32_t)start+size_per_pat-4));
        for(j=0; j < size_per_pat; j=j+4){
            *start = walk_pattern;
            start++;
        }

        dcache_wr_inv_all();
        /* check data */  
        for(j=0; j < size_per_pat; j=j+4){
            start_value = (*read_start);
            if(start_value != walk_pattern){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)read_start,start_value,walk_pattern);
                return MT_FAIL;
            }
            read_start--;
        }
        //printf("\rpattern[%d](0x%x) 0x%x passed", i, walk_pattern, (u32_t)start);
    }
    puts(" passed\n");
    return MT_SUCCESS;
}



int mt_walking_of_0(u32_t ram_start_addr, u32_t size_per_pat, u32_t ram_test_range)
{
    u32_t i, j, offsets=0;
    u32_t start_value;
    u32_t walk_pattern;
    volatile u32_t *start;
    volatile u32_t *read_start;

    printf("II: %s... ", __FUNCTION__);
    for (i=0; i < 32; i++, offsets=(offsets+size_per_pat)%(ram_test_range))
    {
        dcache_wr_inv_all();
        /* generate pattern */
        walk_pattern = ~(1 << i);

        /* write pattern*/
        start = (u32_t *)(ram_start_addr + offsets);
        read_start = (u32_t *)(UADDR(((u32_t)start)+size_per_pat-4));
        for(j=0; j < size_per_pat; j=j+4){
            *start = walk_pattern;
            start++;
        }

        dcache_wr_inv_all();
        /* check data */  
        for(j=0; j < size_per_pat; j=j+4){
            start_value = (*read_start);
            if(start_value != walk_pattern){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)read_start,start_value,walk_pattern);
                return MT_FAIL;
            }
            read_start--;
        }
        //printf("\rpattern[%d](0x%x) 0x%x passed", i, walk_pattern, (u32_t)start);
    }
    puts(" passed\n");
    return MT_SUCCESS;
}


int mt_addr_rot(u32_t ram_start_addr, u32_t size_per_pat, u32_t ram_test_range)
{
    u32_t i, j, offsets=0;
    u32_t start_value;
    u32_t read_start_addr;
    volatile u32_t *start;
    volatile u32_t *read_start;

    printf("II: %s... ", __FUNCTION__);
    for (i=0; i < 32; i=i+4,  offsets=(offsets+size_per_pat)%(ram_test_range))
    {
        dcache_wr_inv_all();
        /* write pattern*/
        start        = (u32_t *)(ram_start_addr + ((i/4)*size_per_pat)%(ram_test_range));
        read_start     = (u32_t *)((u32_t)start + size_per_pat-4);
        read_start_addr = ((u32_t)start);
        start = (u32_t *)(UADDR(((u32_t)start)));
        
        for(j=0; j < size_per_pat; j=j+4){
            *start = (read_start_addr << i);
            start++;
            read_start_addr = read_start_addr + 4 ;
        }

        dcache_wr_inv_all();
        read_start_addr = ((u32_t)read_start);
        /* check data reversing order */  
        for(j=0; j < size_per_pat; j=j+4){
            start_value = (*read_start);
            if(start_value != ((read_start_addr) << i)){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)read_start,start_value,((read_start_addr) << i));
                return MT_FAIL;
            }
            read_start_addr = read_start_addr - 4;
            read_start--;
        }

        read_start_addr += 4;
        read_start++;

        /* check data sequential order */  
        for(j=0; j < size_per_pat; j=j+4){
            start_value = (*read_start);
            if(start_value != ((read_start_addr) << i)){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)read_start,start_value,((read_start_addr) << i));
                return MT_FAIL;
            }
            read_start_addr = read_start_addr + 4;
            read_start++;
        }
        //printf("\rrotate %d 0x%x passed", i, (u32_t)start);
    }
    puts(" passed\n");
    return MT_SUCCESS;
}


int mt_memcpy(u32_t ram_start_addr, u32_t size_per_pat, u32_t ram_test_range)
{
    u32_t i, j, off_src=0, off_dst=0;
    u32_t read_val;
    u32_t src_pat;
    volatile u32_t *read_start;
    volatile u32_t *cpy_src;
    volatile u32_t *cpy_dst;

    printf("II: %s... ", __FUNCTION__);
    for (i=0; i < 32; i=i+4, off_src=(off_src+size_per_pat)%(ram_test_range), off_dst=(off_src+size_per_pat)%(ram_test_range)){
        cpy_src = (volatile u32_t *)(ram_start_addr + off_src + size_per_pat -4);
        cpy_dst = (volatile u32_t *)(ram_start_addr + off_dst);
        src_pat = (u32_t)cpy_src;
        read_start = (u32_t *)UADDR(((u32_t)cpy_dst + size_per_pat - 4));

        /* write pattern*/       
        dcache_wr_inv_all();
        for(j=0; j < size_per_pat; j=j+4, cpy_src--){
            *cpy_src = ((u32_t)cpy_src << i);
        }
        inline_memcpy((char *)cpy_dst, (char *)(++cpy_src), size_per_pat);


        /* check uncached data */  
        dcache_wr_inv_all();       
        for(j=0; j < size_per_pat; j=j+4){
            read_val = (*read_start);
            if(read_val != ((src_pat) << i)){
                EPRINTF("addr(0x%08x):0x%08x != pattern(0x%08x)\n",(u32_t)read_start,read_val,((src_pat) << i));
                return MT_FAIL;
            }
            read_start--;
            src_pat = src_pat - 4;
        }
        //printf("\rmemcpy %d 0x%x passed", i, (u32_t)cpy_src);
    }
    puts(" passed\n");
    return MT_SUCCESS;
}

//mt_unrolling_v2_memcpy
int mt_urv2_memcpy(u32_t ram_start_addr, u32_t ram_test_range)
{
    u32_t unit_size = 32, i;
    volatile u32_t *src, *dst, *end;
    register u32_t r0, r1, r2, r3;
    
    printf("II: %s... ", __FUNCTION__);
    src = (volatile u32_t *)(CADDR(ram_start_addr));
    dst = (volatile u32_t *)(CADDR(ram_start_addr+ram_test_range))-unit_size;
    end = src;
    
    while (dst>end) {
//        printf("DD: src 0x%x, dst 0x%x, end 0x%x\n", src, dst, end);
#if 1       
        r0 = *(src);
        r1 = *(src+8);
        r2 = *(src+16);
        r3 = *(src+24);
        *(dst)    = r0;
        *(dst+8)  = r1;
        *(dst+16) = r2;
        *(dst+24) = r3;
        *(dst+1)  = *(src+1);
        *(dst+2)  = *(src+2);
        *(dst+3)  = *(src+3);
        *(dst+4)  = *(src+4);
        *(dst+5)  = *(src+5);
        *(dst+6)  = *(src+6);
        *(dst+7)  = *(src+7);
        *(dst+9)  = *(src+9);
        *(dst+10) = *(src+10);
        *(dst+11) = *(src+11);
        *(dst+12) = *(src+12);
        *(dst+13) = *(src+13);
        *(dst+14) = *(src+14);
        *(dst+15) = *(src+15);
        *(dst+17) = *(src+17);
        *(dst+18) = *(src+18);
        *(dst+19) = *(src+19);
        *(dst+20) = *(src+20);
        *(dst+21) = *(src+21);
        *(dst+22) = *(src+22);
        *(dst+23) = *(src+23);
        *(dst+25) = *(src+25);
        *(dst+26) = *(src+26);
        *(dst+27) = *(src+27);
        *(dst+28) = *(src+28);
        *(dst+29) = *(src+29);
        *(dst+30) = *(src+30);
        *(dst+31) = *(src+31);
        src+=32;
        dst-=32;
        for (i=0; i<unit_size; i++) {
            if (*(dst+i) != *(src+i)) {
                EPRINTF("expected=(0x%x)0x%x, (0x%x)=0x%x\n",
                         (src+i), *(src+i), (dst+i), *(dst+i)); 
            }
        }
#endif            
    }
    puts(" passed\n");
    return MT_SUCCESS;
}

int mt_ram_test(u32_t addr, u32_t size) {
    u32_t wr_case;
    u32_t size_per_pat=size;    //FIXME, no need
    int ret = MT_SUCCESS;

    for(wr_case=0; wr_case<4; wr_case++){
        printf("II: %cW %cR\n", (!(wr_case>>1))?'U':'C', (!(wr_case&0x1))?'U':'C');
        
        ret = mt_word_pattern_rotate(addr, size_per_pat, size, wr_case);
        if(MT_SUCCESS != ret) return ret;

        ret = mt_halfword_pattern_rotate(addr, size_per_pat, size, wr_case);
        if(MT_SUCCESS != ret) return ret;

        ret = mt_byte_pattern_rotate(addr, size_per_pat, size, wr_case);
        if(MT_SUCCESS != ret) return ret;

        ret = mt_unaligned_wr(addr, size_per_pat, size, wr_case);
        if(MT_SUCCESS != ret) return ret;

        ret = mt_normal_patterns(addr, size_per_pat, size, wr_case);
        if(MT_SUCCESS != ret) return ret;
    }

    ret = mt_com_addr_rot(addr, size_per_pat, size);
    if(MT_SUCCESS != ret) return ret;

    ret = mt_walking_of_0(addr, size_per_pat, size);
    if(MT_SUCCESS != ret) return ret;

    ret = mt_walking_of_1(addr, size_per_pat, size);
    if(MT_SUCCESS != ret) return ret;

    ret = mt_addr_rot(addr, size_per_pat, size);
    if(MT_SUCCESS != ret) return ret;

    ret = mt_memcpy(addr, size_per_pat, size);
    if(MT_SUCCESS != ret) return ret;   
    
    ret = mt_urv2_memcpy(addr, size);
    if(MT_SUCCESS != ret) return ret;   
    
    return ret;
}

symb_fdefine(SF_MT_NORMAL_TEST_PATTERN, mt_normal_patterns);
symb_fdefine(SF_MT_WORD_PATTERN_ROTATE, mt_word_pattern_rotate);
symb_fdefine(SF_MT_HALFWORD_PATTERN_ROTATE, mt_halfword_pattern_rotate);
symb_fdefine(SF_MT_BYTE_PATTERN_ROTATE, mt_byte_pattern_rotate);
symb_fdefine(SF_MT_UNALIGNED_WR, mt_unaligned_wr);
symb_fdefine(SF_MT_ADDR_ROT, mt_addr_rot);
symb_fdefine(SF_MT_COM_ADDR_ROT, mt_com_addr_rot);
symb_fdefine(SF_MT_WALKING_OF_0, mt_walking_of_0);
symb_fdefine(SF_MT_WALKING_OF_1, mt_walking_of_1);
symb_fdefine(SF_MT_MEMCPY, mt_memcpy);


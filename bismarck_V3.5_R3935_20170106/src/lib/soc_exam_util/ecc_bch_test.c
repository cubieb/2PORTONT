#include <util.h>
#include <soc_exam_util/ecc_bch_test.h>
#include <spi_nand/spi_nand_struct.h>

#ifndef SECTION_SPI_NAND_MT
    #define SECTION_SPI_NAND_MT
#endif


spi_nand_flash_info_t bch_snaf_info SECTION_PARAMETERS;

#define _u32ptr_src_buf (volatile u32_t *)SNAF_SRC_CHUNK_BUF
#define _u32ptr_chk_buf (volatile u32_t *)SNAF_CHK_CHUNK_BUF
#define _u32ptr_ecc_buf (volatile u32_t *)SNAF_ECC_TAG_BUF



u32_t sector_addr[5] = {
    SNAF_CHK_CHUNK_BUF,
    SNAF_CHK_CHUNK_BUF+BCH_SECTOR_SIZE*1,
    SNAF_CHK_CHUNK_BUF+BCH_SECTOR_SIZE*2,
    SNAF_CHK_CHUNK_BUF+BCH_SECTOR_SIZE*3,
    SNAF_CHK_CHUNK_BUF+BCH_SECTOR_SIZE*4
};

const u32_t bch_patterns[] = {
    0x01010101,
    0xfefefefe,
    0x00000000,
    0xff00ff00,
    0x00ff00ff,
    0x0000ffff,
    0xffff0000,
    0xffffffff,
    0x5a5aa5a5,
    0xa5a5a5a5,
    0x55555555,
    0xaaaaaaaa,
    0x01234567,
    0x76543210,
    0x89abcdef,
    0xfedcba98,
};


/**************************
  * Random Number Use
  *************************/ 
#define GET_SEED 1
#define SET_SEED 0

SECTION_SPI_NAND_MT static void __srandom32(int *a1, int *a2, int *a3, int get_or_set)
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
SECTION_SPI_NAND_MT static unsigned int __random32(void)
{
#define TAUSWORTHE(s,a,b,c,d) ((s&c)<<d) ^ (((s <<a) ^ s)>>b)
    int s1, s2, s3;
    __srandom32(&s1, &s2, &s3, GET_SEED);

    s1 = TAUSWORTHE(s1, 13, 19, 4294967294UL, 12);
    s2 = TAUSWORTHE(s2, 2, 25, 4294967288UL, 4);
    s3 = TAUSWORTHE(s3, 3, 11, 4294967280UL, 17);

    __srandom32(&s1, &s2, &s3, SET_SEED);

    return (s1 ^ s2 ^ s3);
}


SECTION_SPI_NAND_MT int data_compare(void *src, void *dst, u32_t len)
{
	u32_t i, cnt=0;
    volatile u32_t *source      = (volatile u32_t *)src;
    volatile u32_t *destination = (volatile u32_t *)dst;
	for(i=0 ; i<(len/4) ; i++){
		if(*(destination+i) != *(source+i)){
			printf("  <<ERROR>> src(0x%08x:0x%08x) != dst(0x%08x:0x%08x)\n",(source+i),*(source+i),(destination+i),*(destination+i));
            cnt++;
            if(cnt >=20) return -1;
		}
	}
	return 0;
}

SECTION_SPI_NAND_MT int ecc_destroy_pattern1(u32_t bch_ability)
{
    int ecc_sts;
    u32_t i, pat_idx, shifts=0, cnt=0;
	volatile u32_t *addr0,*addr1, *addr2, *addr3;
    
    if(12 == bch_ability){    
        bch_snaf_info._spare_size = SNAF_MODEL_SPARE_SIZE_128B;
    }
    u32_t page_size_with_spare = SNAF_PAGE_SIZE((&bch_snaf_info))+SNAF_SPARE_SIZE((&bch_snaf_info));

    printf("II: Start %s: (BCH %d)\n",__FUNCTION__,bch_ability);

    for(pat_idx=0 ; pat_idx<(sizeof(bch_patterns)/sizeof(u32_t)); pat_idx++){        
        addr0 = (volatile u32_t *)(sector_addr[0]);
        addr1 = (volatile u32_t *)(sector_addr[1]);
        addr2 = (volatile u32_t *)(sector_addr[2]);
        addr3 = (volatile u32_t *)(sector_addr[3]);

        for(i=0 ; i<(page_size_with_spare/4) ; i++){
            *(_u32ptr_src_buf+i) = bch_patterns[pat_idx];
        }  

        bch_snaf_info._ecc_encode(bch_ability, src_page_buf, mt_ecc_buf);
        inline_memcpy(chk_page_buf, src_page_buf, page_size_with_spare);
   
    	while(((u32_t)addr0<sector_addr[1])||((u32_t)addr1<sector_addr[2])||((u32_t)addr2<sector_addr[3])||((u32_t)addr3<sector_addr[4])){
    		cnt=0;
    		while((u32_t)addr0<sector_addr[1])
    		{
    			shifts=0;
    			while((shifts<32)&&(cnt<bch_ability)){
    				if(((*addr0>>shifts)&0x1)==0x1){
    					*addr0 &= ~(1<<shifts);
    					cnt++;
    				}
    				shifts++;
    			}
    			addr0++;
    		}

    		cnt=0;
    		while((u32_t)addr1<sector_addr[2])
    		{
    			shifts=0;
    			while((shifts<32)&&(cnt<bch_ability)){
    				if(((*addr1>>shifts)&0x1)==0x1){
    					*addr1 &= ~(1<<shifts);
    					cnt++;
    				}
    				shifts++;
    			}
    			addr1++;
    		}

    		cnt=0;
    		while((u32_t)addr2<sector_addr[3])
    		{
    			shifts=0;
    			while((shifts<32)&&(cnt<bch_ability)){
    				if(((*addr2>>shifts)&0x1)==0x1){
    					*addr2 &= ~(1<<shifts);
    					cnt++;
    				}
    				shifts++;
    			}
    			addr2++;
    		}

    		cnt=0;
    		while((u32_t)addr3<sector_addr[4])
    		{
    			shifts=0;
    			while((shifts<32)&&(cnt<bch_ability)){
    				if(((*addr3>>shifts)&0x1)==0x1){
    					*addr3 &= ~(1<<shifts);
    					cnt++;
    				}
    				shifts++;
    			}
    			addr3++;
    		}
            
            ecc_sts = bch_snaf_info._ecc_decode(bch_ability, chk_page_buf, mt_ecc_buf);
            if(IS_ECC_DECODE_FAIL(ecc_sts)){
                data_compare(src_page_buf, chk_page_buf, page_size_with_spare);
                printf("\n    <EE> BCH%d decode fail 0x%08x (pattern=0x%x)\n",bch_ability,ecc_sts, bch_patterns[pat_idx]);
                SWBREAK();
            }else printf("\r    Decode OK, correct %d bits (pattern=0x%08x)",ecc_sts, bch_patterns[pat_idx]);

        }
    }
	printf("\nII: %s Passed\n",__FUNCTION__);
	return 0;
}

SECTION_SPI_NAND_MT int ecc_destroy_pattern2(u32_t bch_ability)
{
    int ecc_sts;
    u32_t i, pat_idx, read_val, shifts=0, destroy_width = 0x3F;
	volatile u32_t *addr0,*addr1, *addr2, *addr3;

    if(12 == bch_ability){
        bch_snaf_info._spare_size = SNAF_MODEL_SPARE_SIZE_128B;
        destroy_width = 0xFFF;
    }   
    u32_t page_size_with_spare = SNAF_PAGE_SIZE((&bch_snaf_info))+SNAF_SPARE_SIZE((&bch_snaf_info));

    printf("II: Start %s: (BCH %d)\n",__FUNCTION__,bch_ability);


    for(pat_idx=0 ; pat_idx<(sizeof(bch_patterns)/sizeof(u32_t)); pat_idx++){        
        addr0 = (volatile u32_t *)(sector_addr[0]);
        addr1 = (volatile u32_t *)(sector_addr[1]);
        addr2 = (volatile u32_t *)(sector_addr[2]);
        addr3 = (volatile u32_t *)(sector_addr[3]);


        for(i=0 ; i<(page_size_with_spare/4) ; i++){
            *(_u32ptr_src_buf+i) = bch_patterns[pat_idx];
        }  

        bch_snaf_info._ecc_encode(bch_ability, src_page_buf, mt_ecc_buf);
        inline_memcpy(chk_page_buf, src_page_buf, page_size_with_spare);
   
    	while(((u32_t)addr0<sector_addr[1])||((u32_t)addr1<sector_addr[2])||((u32_t)addr2<sector_addr[3])||((u32_t)addr3<sector_addr[4])){
            read_val = *addr0;
            *addr0 = (~(read_val & (destroy_width<<(shifts%32))))&(read_val | (destroy_width<<(shifts%32)));   
			addr0++;
			shifts++;            
            
			read_val = *addr1;
			*addr1 = (~(read_val & (destroy_width<<(shifts%32))))&(read_val | (destroy_width<<(shifts%32)));   
			addr1++;
			shifts++;            

			read_val = *addr2;
			*addr2 = (~(read_val & (destroy_width<<(shifts%32))))&(read_val | (destroy_width<<(shifts%32)));   
			addr2++;
			shifts++;            

			read_val = *addr3;
			*addr3 = (~(read_val & (destroy_width<<(shifts%32))))&(read_val | (destroy_width<<(shifts%32)));   
			addr3++;
			shifts++;            
            
            ecc_sts = bch_snaf_info._ecc_decode(bch_ability, chk_page_buf, mt_ecc_buf);
            if(IS_ECC_DECODE_FAIL(ecc_sts)){
                data_compare(src_page_buf, chk_page_buf, page_size_with_spare);
                printf("\n    <EE> BCH%d decode fail 0x%08x (pattern=0x%x)\n",bch_ability,ecc_sts, bch_patterns[pat_idx]);
                SWBREAK();
            }else printf("\r    Decode OK, correct %d bits (pattern=0x%08x)",ecc_sts, bch_patterns[pat_idx]);

        }
    }
	printf("\nII: %s Passed\n",__FUNCTION__);
	return 0;
}

SECTION_SPI_NAND_MT int ecc_destroy_pattern3(u32_t bch_ability)
{
    int ecc_sts;
    u32_t i, pat_idx, loops_per_pat, read_val, shifts=0, bit_cnt=0, destroy_width = 0x1;
	volatile u32_t *addr0, *addr1, *addr2, *addr3;
    
    if(12 == bch_ability){
        bch_snaf_info._spare_size = SNAF_MODEL_SPARE_SIZE_128B;
    }    
    u32_t page_size_with_spare = SNAF_PAGE_SIZE((&bch_snaf_info))+SNAF_SPARE_SIZE((&bch_snaf_info));

    printf("II: Start %s: (BCH %d)\n",__FUNCTION__,bch_ability);

    /*for random address offset test use*/
    int a=0x13243,b=0xaaa0bdd,c=0xfffbda0;
    __srandom32(&a, &b, &c, SET_SEED);

    #define ADDR_OFFSET_RANDOM (((__random32()>>2)<<2)&0x1F)/4

    for(pat_idx=0 ; pat_idx<(sizeof(bch_patterns)/sizeof(u32_t)); pat_idx++){        
        for(i=0 ; i<(page_size_with_spare/4) ; i++){
            *(_u32ptr_src_buf+i) = bch_patterns[pat_idx];
        }  
        bch_snaf_info._ecc_encode(bch_ability, src_page_buf, mt_ecc_buf);

        for(loops_per_pat=0 ; loops_per_pat<100; loops_per_pat++){       	
            addr0 = (volatile u32_t *)(sector_addr[0]);
            addr1 = (volatile u32_t *)(sector_addr[1]);
            addr2 = (volatile u32_t *)(sector_addr[2]);
            addr3 = (volatile u32_t *)(sector_addr[3]);          
            
            inline_memcpy(chk_page_buf, src_page_buf, page_size_with_spare);

            bit_cnt = 0;
            shifts = __random32()&0x1F;
            while(bit_cnt<bch_ability){
                if((u32_t)addr0<sector_addr[1]){
                    read_val = *addr0;
                    *addr0 = (~(read_val & (destroy_width<<(shifts%32))))&(read_val | (destroy_width<<(shifts%32)));   
                    addr0+=ADDR_OFFSET_RANDOM;
                    shifts++;
                }

                if((u32_t)addr1<sector_addr[2]){
                    read_val = *addr1;
                    *addr1 = (~(read_val & (destroy_width<<(shifts%32))))&(read_val | (destroy_width<<(shifts%32)));   
                    addr1+=ADDR_OFFSET_RANDOM;
                    shifts++;
                }

                if((u32_t)addr2<sector_addr[3]){
                    read_val = *addr2;
                    *addr2 = (~(read_val & (destroy_width<<(shifts%32))))&(read_val | (destroy_width<<(shifts%32)));   
                    addr2+=ADDR_OFFSET_RANDOM;
                    shifts++;
                }

                if((u32_t)addr3<sector_addr[4]){
                    read_val = *addr3;
                    *addr3 = (~(read_val & (destroy_width<<(shifts%32))))&(read_val | (destroy_width<<(shifts%32)));   
                    addr3+=ADDR_OFFSET_RANDOM;
                    shifts++;
                }
                bit_cnt++;
            }   
                
            ecc_sts = bch_snaf_info._ecc_decode(bch_ability, chk_page_buf, mt_ecc_buf);
            if(IS_ECC_DECODE_FAIL(ecc_sts)){
                data_compare(src_page_buf, chk_page_buf, page_size_with_spare);
                printf("\n    <EE> BCH%d decode fail 0x%08x (pattern=0x%x)\n",bch_ability,ecc_sts, bch_patterns[pat_idx]);
                SWBREAK();
            }else printf("\r    Decode OK, correct %d bits (pattern=0x%08x)",ecc_sts, bch_patterns[pat_idx]);

        }
    }
	printf("\nII: %s Passed\n",__FUNCTION__);
	return 0;
}

SECTION_SPI_NAND_MT void ecc_all_one_test(u32_t bch_ability)
{
    u32_t i;
    int ecc_sts;

    if(12 == bch_ability){
        bch_snaf_info._spare_size = SNAF_MODEL_SPARE_SIZE_128B;
    }
    
    u32_t page_size_with_spare = SNAF_PAGE_SIZE((&bch_snaf_info))+SNAF_SPARE_SIZE((&bch_snaf_info));

    printf("II: Start %s (BCH %d): ",__FUNCTION__,bch_ability);


    //Step 1: Initialize the src data buffer to 0xFFFFFFFF
    for(i=0 ; i<(page_size_with_spare/4) ; i++){
        *(_u32ptr_src_buf+i) = 0xFFFFFFFF; 
    }

    //Step 2: Initialize the ecc buffer to 0xFFFFFFFF
    for(i=0 ; i<(bch_ability==6?4:7) ; i++){
        *(_u32ptr_ecc_buf+i) = 0xFFFFFFFF; 
    }

    
    //Step 3: ECC decode, ALL_ONE bit should be set
    ecc_sts = bch_snaf_info._ecc_decode(bch_ability, src_page_buf, mt_ecc_buf);
    if(IS_ECC_DECODE_FAIL(ecc_sts)) {
        printf("\n    <EE> ECC ALL ONE decode fail!!!\n");
        SWBREAK();
    }
	printf("Passed\n");
}

#define ROTATE_1BIT(v) (((v) << 1) | ((v & 0x80000000)>>31));
SECTION_SPI_NAND_MT void bch_encode_decode_test(u32_t bch_ability)
{
    u32_t i, pat_idx;
    int ecc_sts;

    if(12 == bch_ability){
        bch_snaf_info._spare_size = SNAF_MODEL_SPARE_SIZE_128B;
    }   
    u32_t page_size_with_spare = SNAF_PAGE_SIZE((&bch_snaf_info))+SNAF_SPARE_SIZE((&bch_snaf_info));

    printf("II: Start %s (BCH %d): \n",__FUNCTION__,bch_ability);


    //Step1: BCH_Encode then BCH_Decode: Pattern is rotate_1bit
    for(i=0 ; i<(page_size_with_spare/4) ; i++){
        *(_u32ptr_src_buf+i) = ROTATE_1BIT(((u32_t)(src_page_buf+i))); 
    }  
    bch_snaf_info._ecc_encode(bch_ability, src_page_buf, mt_ecc_buf);
    ecc_sts = bch_snaf_info._ecc_decode(bch_ability, src_page_buf, mt_ecc_buf);
    if(IS_ECC_DECODE_FAIL(ecc_sts)){
        printf("\n    <EE> BCH%d decode fail (ecc_sts=0x%08x)\n",bch_ability, ecc_sts);
        SWBREAK();
    }else printf("\r    Decode OK, correct %d bits (pattern=addr_rotate)\n",ecc_sts);
     

    //Step2: BCH_Encode then BCH_Decode: Pattern is bch_patterns
    for(pat_idx=0 ; pat_idx<(sizeof(bch_patterns)/sizeof(u32_t)); pat_idx++){        
        for(i=0 ; i<(page_size_with_spare/4) ; i++){
            *(_u32ptr_src_buf+i) = bch_patterns[pat_idx]; 
        }  
        bch_snaf_info._ecc_encode(bch_ability, src_page_buf, mt_ecc_buf);
        ecc_sts = bch_snaf_info._ecc_decode(bch_ability, src_page_buf, mt_ecc_buf);
        if(IS_ECC_DECODE_FAIL(ecc_sts)){
            printf("\n    <EE> BCH%d decode fail 0x%08x (pattern=0x%x)\n",bch_ability, ecc_sts, bch_patterns[pat_idx]);
            SWBREAK();
        }else printf("\r    Decode OK, correct %d bits (pattern=0x%08x)",ecc_sts, bch_patterns[pat_idx]);
    }
	printf("\nII: %s Passed\n",__FUNCTION__);
}


SECTION_SPI_NAND_MT void ecc_controller_test(void)
{
    inline_memcpy(&bch_snaf_info, _spi_nand_info, sizeof(spi_nand_flash_info_t));

    ecc_all_one_test(6);
    ecc_all_one_test(12);
    bch_encode_decode_test(6);
    bch_encode_decode_test(12);
    ecc_destroy_pattern1(6);
    ecc_destroy_pattern1(12);
    ecc_destroy_pattern2(6);
    ecc_destroy_pattern2(12);
    ecc_destroy_pattern3(6);
    ecc_destroy_pattern3(12);
}


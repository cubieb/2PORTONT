

#define NAND_SPI_USE_SIO

#define SRAM_CTRL_INIT  \
    li      t0, 0xB8001300;\
    lui     t1, %hi(OTTO_SRAM_START);\
    lui     t3, 0x1FFF;\
    and     t1, t1, t3;\
    ori     t1, t1, 0x1;/*Set (Un)mapping enable bit*/\
    li      t3, 0xB8004000;\
    lw      t2, 4(t3);\
    sw      t1, 0(t0); /*unmapping*/\
    sw      t2, 4(t0); /*unmapping size of default sram controller setting*/\
    sw      t1, 0(t3); /*mapping*/


#define _lplr_soc_t  (*(soc_t*)(OTTO_LPLR_ENTRY+OTTO_HEADER_OFFSET))
#define _lplr_bios   (_lplr_soc_t.bios)
#define _lplr_header (_lplr_bios.header)


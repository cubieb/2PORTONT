// arch.h for project/9602C_MP_spi_nand_demo

//For identify Formal chip
#define PRODUZIONE (1)

//For enable UART IO
#define UART_IO_EN() RMOD_IO_MODE_EN(uart0_en, 1)

//*******************************
// BTG Relative Definitions
//*******************************
#define PROJECT_WITH_PBO_LX_BUS

#define MT_DRAM_ADDR_V         (0xA0000000)
#define MT_DRAM_SIZE           (0x01000000)
#define BTG_LX0_ADDR_PHY       (0xA1000000)
#define BTG_LX1_ADDR_PHY       (0xA1800000)
#define BTG_LX2_ADDR_PHY       (0xA2000000)
#define BTG_LXUS_ADDR_PHY      (0xA2800000)
#define BTG_LXDS_ADDR_PHY      (0xA3000000)
#define BTG_SIZE_W             (0x00800000-1)
#define BTG_SIZE_R             (0x00800000-1)

#define BTGLX0_BASE   (0xB8144000)
#define BTGLX1_BASE   (0xB800A000)
#define BTGLX2_BASE   (0xB8018000)
#define BTGLXUW_BASE  (0xB8148000)
#define BTGLXUR_BASE  (0xB8149000)
#define BTGLXDW_BASE  (0xB814A000)
#define BTGLXDR_BASE  (0xB814B000)


// UBOOT relative definition
#define MAX_UBOOT_SIZE         (0x20000)
#define ECC_DECODED_UBOOT_ADDR (0x80700000)
#define NEW_STACK_AT_DRAM      (ECC_DECODED_UBOOT_ADDR-32)
#define UBOOT_DECOMP_ADDR      (0x80000000)


#define SIZE_3KB            (3*1024)
#define SNAF_SRC_CHUNK_BUF    (0xA0000000)
#define SRC_OOB_BUFFER      (SNAF_SRC_CHUNK_BUF + 0x800)
#define SNAF_CHK_CHUNK_BUF    (SNAF_SRC_CHUNK_BUF + SIZE_3KB)
#define CHK_OOB_BUFFER      (SNAF_CHK_CHUNK_BUF + 0x800)
#define SNAF_ECC_TAG_BUF       (SNAF_CHK_CHUNK_BUF + SIZE_3KB)
#define src_page_buf ((void*)(SNAF_SRC_CHUNK_BUF))
#define src_oob_buf   ((oob_t*)(SRC_OOB_BUFFER))
#define chk_page_buf ((void*)(SNAF_CHK_CHUNK_BUF))
#define chk_oob_buf   ((oob_t*)(CHK_OOB_BUFFER))
#define mt_ecc_buf    ((void*)(SNAF_ECC_TAG_BUF))

#define SECTION_SPI_NAND_MT __attribute__ ((section (".pge_align")))

#define TIMER_FREQ_MHZ     (200)
#define TIMER_STEP_PER_US  (4)
#define TIMER1_BASE        (0xb8003210)
#define UDELAY_TIMER_BASE  TIMER1_BASE



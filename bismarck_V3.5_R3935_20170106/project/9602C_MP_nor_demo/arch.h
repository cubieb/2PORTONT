
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


#define SIZE_8KB               (0x2000)
#define SIZE_64KB              (0x10000)


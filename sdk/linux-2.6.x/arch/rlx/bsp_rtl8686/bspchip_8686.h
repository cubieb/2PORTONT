/*
 * Copyright 2006, Realtek Semiconductor Corp.
 *
 * rtl8196b/bsp/bspchip.h:
 *   RTL8196B chip-level header file
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 07, 2006
 */

#ifndef _BSPCHIP_8686_H_
#define _BSPCHIP_8686_H_

#define SPI_NOR_FLASH_START_ADDR (0xBD000000)
#define SPI_NOR_FLASH_MMIO_SIZE  (32*1024*1024) //32MByte

/*
 * IRQ Controller
 */

/*
 *  ====================================
 *  Platform Configurable Common Options
 *  ====================================
 */

#define PROM_DEBUG      0

//for auto-detect, MHZ and SYSCLK initialization is moved to arch/mips/realtek/rtl8670/setup.c
extern unsigned int BSP_MHZ;
extern unsigned int BSP_SYSCLK;
//#define BSP_SYS_CLK_RATE	(180000000)     //HS1 clock : 180 MHz

#define BSP_BAUDRATE      115200  /* ex. 19200 or 38400 or 57600 or 115200 */ 
                               /* For Early Debug */

            
/*                        
 * IRQ Mapping
 */
#define BSP_LBCTMOm2_IRQ 	63
#define BSP_LBCTMOm1_IRQ 	62
#define BSP_LBCTMOm0_IRQ 	61
#define BSP_LBCTMOs3_IRQ 	60
#define BSP_LBCTMOs2_IRQ 	59
#define BSP_LBCTMOs1_IRQ 	58
#define BSP_LBCTMOs0_IRQ 	57
#define BSP_OCPTO1_IRQ  	56
#define BSP_OCPTO0_IRQ  	55
#define BSP_OCP1_CPU0_ILA_IP_IRQ  54
#define BSP_OCP0_CPU1_ILA_IP_IRQ  53
#define BSP_UART3_IRQ		52
#define BSP_UART2_IRQ		51
#define BSP_UART1_IRQ		50
#define BSP_UART0_IRQ		49
#define BSP_TC5_IRQ			48
#define BSP_TC4_IRQ			47
#define BSP_TC3_IRQ			46
#define BSP_TC2_IRQ			45
#define BSP_TC1_IRQ			44
#define BSP_TC0_IRQ			43
#ifdef LUNA_RTL9602C
#define BSP_GPIO_ABCD_IRQ  	42
#define BSP_GPIO_EFGH_IRQ  	41
#else
#define BSP_GPIO_EFGH_IRQ  	42
#define BSP_GPIO_ABCD_IRQ  	41
#endif

#define BSP_TMO_IRQ         		31
#define BSP_VOIPACC_IRQ		30
#define BSP_SPI_IRQ         		29
#define BSP_PTM_IRQ      		28
#define BSP_GMAC1_IRQ         		27
#define BSP_GMAC_IRQ		26
#define BSP_SAR_IRQ			25
#define BSP_PKT_GMAC_IRQ    	24
#define BSP_PKT_XTM_IRQ     	23
#define BSP_DMT_IRQ         		22
#define BSP_GDMA1_IRQ       	21
#define BSP_GDMA0_IRQ    		20
#define BSP_SECURITY_IRQ    	19
#define BSP_PCM1_IRQ      		18
#define BSP_PCM0_IRQ   		17
#define BSP_PCIE1_IRQ   		16
#define BSP_PCIE0_IRQ       	15
#define BSP_PCIE_IRQ              BSP_PCIE0_IRQ
#define BSP_USB_H2_IRQ      	14
#define BSP_USB_H3_IRQ      	13
#define BSP_PERIPHERAL_IRQ  	12
//#define BSP_USB_D_IRQ     11
#define BSP_USB_H_IRQ     BSP_USB_H2_IRQ
#define BSP_GPIO1_IRQ			10
//#define BSP_TC1_IRQ       9
#define BSP_SWITCH_IRQ      	8
#define BSP_SATA_IRQ    		7
#define BSP_NFBI_IRQ    		6
#define BSP_WDOG_IRQ      	5
#define BSP_FFTACC_IRQ    		4
#define BSP_FLSH_IRQ    		3
#define BSP_UTMD_IRQ      		2
#define BSP_CUP0_T_CPU1_IRQ		1
#define BSP_CUP1_T_CPU0_IRQ		0


/*
 * Interrupt Routing Selection
 */
#define BSP_PTM_RS          		2
#define BSP_LBCTMOs2_RS     	2
#define BSP_LBCTMOs1_RS     	2
#define BSP_PKT_RS          		2
#define BSP_OCP1_CPU0_ILA_RS	2
#define BSP_OCP0_CPU0_ILA_RS	2
#define BSP_SPI_RS          		2
#define BSP_GMAC_RS      		2
#define BSP_VOIPACC_RS		2
#define BSP_NIC100_RS       	2
#define BSP_SAR_RS          		2
#define BSP_DMT_RS          		2
#define BSP_PKT_NIC100_RS   	2
#define BSP_PKT_NIC100MII_RS 2
#define BSP_PKT_SAR_RS      	2
#define BSP_GDMA_RS         	2
#define BSP_SECURITY_RS     	2
#define BSP_PCM_RS          		2
#define BSP_GPIO_EFGH_RS    	2
#define BSP_GPIO_ABCD_RS    	2
#define BSP_SW_RS           		6
#define BSP_PCIE_RS         		5
#define BSP_UART1_RS        	2
#define BSP_UART0_RS        	3
#define BSP_TC1_RS          		2
#define BSP_TC0_RS          		7
#define BSP_LBCTMOm2_RS     	2
#define BSP_LBCTMOm1_RS     	2
#define BSP_LBCTMOm0_RS     	2
#define BSP_SPEED_RS        	2
#define BSP_LBCTMOs3_RS     	2
#define BSP_LBCTMOs2_RS     	2
#define BSP_LBCTMOs1_RS     	2
#define BSP_LBCTMOs0_RS     	2
#define BSP_OCPTO1_RS		2
#define BSP_OCPTO0_RS		2
#define BSP_OCPTMO1_RS      	2
#define BSP_OCPTMO0_RS      	2
#define BSP_PCIB0TO_RS      	2

#define BSP_PKT_GMAC_RS 	2
#define BSP_PKT_XTM_RS 		2
#define BSP_GDMA1_RS 		2
#define BSP_GDMA0_RS  		2
#define BSP_Security_RS 		2
#define BSP_PCM1_RS			2
#define BSP_PCM0_RS 			2
#define BSP_PCIE1_RS 		2
#define BSP_PCIE0_RS 		2
#define BSP_USB_H2_RS 		2
#define BSP_USB_H3_RS 		2
#define BSP_UART3_RS 		2
#define BSP_UART2_RS 		2
#define BSP_TC5_RS 			2
#define BSP_TC4_RS 			2
#define BSP_TC3_RS			2
#define BSP_TC2_RS 			2
#define BSP_GPIO1_RS 		2
#define BSP_GPIO0_RS 		2
#define BSP_SWITCH_RS 		2
#define BSP_SATA_RS 			2
#define BSP_NFBI_RS 			2
#define BSP_WDOG_RS 		2
#define BSP_FFTACC_RS 		2
#define BSP_FLSH_RS 			2
#define BSP_UTMD_RS			2				
#define BSP_CUP0_T_CPU1_RS		2
#define BSP_CUP1_T_CPU0_RS		2

#define BSP_DIVISOR         1000

#if BSP_DIVISOR > (1 << 16)
#error "Exceed the Maximum Value of DivFactor"
#endif

/*
 *  ==========================
 *  Platform Register Settings
 *  ==========================
 */

/*
 * CPU
 */
#define BSP_IMEM_BASE       0x00C00000
#define BSP_IMEM_TOP        0x00C03FFF

#define BSP_DMEM_BASE       0x00C04000
#define BSP_DMEM_TOP        0x00C05FFF

/*
 *
 */
#define CHIP_ID_REG		0xBb010004
#define IO_MODE_EN_REG		0xBB023020
#define IO_MODE_INTRPT1_EN	(1<<12)

#define BSP_SYSREG_PIN_STATUS_REG       (0xB8000100)
#define BSP_SYSREG_PIN_STATUS_CLSEL__FD_S   (5)
#define BSP_SYSREG_PIN_STATUS_CLSEL_MASK    (1 << BSP_SYSREG_PIN_STATUS_CLSEL__FD_S)
#define BSP_SYSREG_CMUCTLR_REG      (0xB8000308)
#define BSP_SYSREG_CMUCTLR_CMU_MD_FD_S  (6)
#define BSP_SYSREG_CMUCTLR_CMU_MD_MASK  (3 << BSP_SYSREG_CMUCTLR_CMU_MD_FD_S)
#define BSP_SYSREG_CMUCTLR_CMU_MD_MANUALLY_SEL (1)
#define BSP_SYSREG_CMUCTLR_CMU_LX0_FREQ_DIV_FD_S    (17)
#define BSP_SYSREG_CMUCTLR_CMU_LX0_FREQ_DIV_MASK    (7 << BSP_SYSREG_CMUCTLR_CMU_LX0_FREQ_DIV_FD_S)
#define BSP_SYSREG_LX_PLL_SEL_REG       (0xB8000228)

/*
 * Memory Controller
 */
 #define BSP_MC_BASE         0xB8001000
#define BSP_MC_MCR          (BSP_MC_BASE)
#define BSP_MC_MTCR0        (BSP_MC_BASE + 0x04)
#define BSP_MC_MTCR1        (BSP_MC_BASE + 0x08)
#define BSP_MC_PFCR         (BSP_MC_BASE + 0x010)
#define BSP_MC_MSRR    (BSP_MC_BASE + 0x038)
#define FLUSH_OCP_CMD    (1<<31)
#define BSP_NCR             (BSP_MC_BASE + 0x100)
#define BSP_NSR             (BSP_MC_BASE + 0x104)
#define BSP_NCAR            (BSP_MC_BASE + 0x108)
#define BSP_NADDR           (BSP_MC_BASE + 0x10C)
#define BSP_NDR             (BSP_MC_BASE + 0x110)

#define BSP_SFCR            (BSP_MC_BASE + 0x200)
#define BSP_SFDR            (BSP_MC_BASE + 0x204)

#define BSP_BOOT_FLASH_STS		(0xf << 24)

/* 2012-3-13 krammer add 
write data will hold in write buffer of memory controller even if we use uncache access!
so we need to flush the buffer before some case, 
ex: when we finish writing a tx desc, we need to flush write buffer before we kick hw!
*/
static inline void write_buffer_flush(void){
	unsigned int cnt = 100;
	READ_MEM32(BSP_MC_MSRR) |= FLUSH_OCP_CMD;
	while((READ_MEM32(BSP_MC_MSRR) & FLUSH_OCP_CMD) && cnt-- );

	if(!cnt){
		printk("%s %d: write buffer flush timeout!\n", __func__, __LINE__);
	}
}


/*
 * NAND flash controller address
 */
//czyao , nand flash address
#if 0
#define NAND_CTRL_BASE  0xB801A000//0xB801A000 //0xB8011000
#define NACFR  (NAND_CTRL_BASE + 0x0)
		#define NAFC_RC 	 (1<<31)

#define NACR    (NAND_CTRL_BASE + 0x04)
        #define flash_READY  (1<<31)
        #define ECC_enable    (1<<30)
        #define RBO                  (1<<29)
        #define WBO                  (1<<28)
#define NACMR   (NAND_CTRL_BASE + 0x08)
        #define CECS1   (1<<31)
        #define CECS0   (1<<30)
        #define Chip_Seletc_Base        30
#define NAADR   (NAND_CTRL_BASE + 0x0C)
        #define enNextAD                (1<<27)
        #define AD2EN           (1<<26)
        #define AD1EN           (1<<25)
        #define AD0EN           (1<<24)
        #define CE_ADDR2                16
        #define CE_ADDR1                8
        #define CE_ADDR0                0
#define NADCRR  (NAND_CTRL_BASE + 0x10)
        #define TAG_DIS         (1<<6)
        #define DESC1           (1<<5)
        #define DESC0           (1<<4)
        #define DMARE           (1<<3)
        #define DMAWE           (1<<2)
        #define LBC_128         3
        #define LBC_64          2
        #define LBC_32          1
        #define LBC_16          0
#define NADR    (NAND_CTRL_BASE + 0x14)
#define NADFSAR (NAND_CTRL_BASE + 0x18)
//#define NADFSAR2 (NAND_CTRL_BASE + 0x18)

#define NADRSAR (NAND_CTRL_BASE + 0x1C)
#define NASR    (NAND_CTRL_BASE + 0x20)
        #define NECN                    (1<<4)
        #define NRER                    (1<<3)
        #define NWER                    (1<<2)
        #define NDRS                    (1<<1)
        #define NDWS            (1<<0)
#define NADTSAR (NAND_CTRL_BASE + 0x54)
#endif
#define M_mask                  0xe0000000


#if 1//bch
#define NAND_CTRL_BASE  0xB801A000
#define NACFR  (NAND_CTRL_BASE + 0x0)
/*bit 31:30	read command strap pin, 00: 512Byte, 01: 2048Byte,10:2048,11:4096 */
				#define NAFC_RC1 (1<<31)			
				#define NAFC_RC0 (1<<30)
/*bit 29:28 Address Cycle strap pin, 00: 3 address cycle 01: 4 address cycle 10: 5 address cycle 11: reserved */
				#define NAFC_AC1 (1<<29)
				#define NAFC_AC0 (1<<28)

/*bit 4 : CE_TRHZ , 0:use 1 time-uint,1: 2 time-unit*/
				#define CE_TRHZ	 (1<<4)
/*bit 3:0 CE_TDH 000: use 1 time-unit, 001:2time unit ,010:3time,011:reserve*/
				
				
#define NACR    (NAND_CTRL_BASE + 0x04)
        #define flash_READY  (1<<31)
        #define ECC_enable    (1<<30)
        #define RBO                  (1<<29)
        #define WBO                  (1<<28)
#define NACMR   (NAND_CTRL_BASE + 0x08)
        #define CECS1   (1<<31)
        #define CECS0   (1<<30)
        #define Chip_Seletc_Base        30
#define NAADR   (NAND_CTRL_BASE + 0x0C)
        #define enNextAD                (1<<27)
        #define AD2EN           (1<<26)
        #define AD1EN           (1<<25)
        #define AD0EN           (1<<24)
        #define CE_ADDR2                16
        #define CE_ADDR1                8
        #define CE_ADDR0                0
#define NADCRR  (NAND_CTRL_BASE + 0x10)
        #define TAG_DIS         (1<<6)
        #define DESC1           (1<<5)
        #define DESC0           (1<<4)
        #define DMARE           (1<<3)
        #define DMAWE           (1<<2)
        #define LBC_128         3
        #define LBC_64          2
        #define LBC_32          1
        #define LBC_16          0
#define NADR    (NAND_CTRL_BASE + 0x14)
#define NADFSAR (NAND_CTRL_BASE + 0x18)	 //DMA Flash Start Address Register 1
#define NADFSAR2 (NAND_CTRL_BASE + 0x1C) //DMA Flash Start Address Register 2 ,zero when nand flash address smaller 2G

#define NADRSAR (NAND_CTRL_BASE + 0x20) //data start address
#define NADTSAR (NAND_CTRL_BASE + 0x24) //oob start address

#define NASR    (NAND_CTRL_BASE + 0x28)
        #define NECN                    (1<<4)
        #define NRER                    (1<<3)
        #define NWER                    (1<<2)
        #define NDRS                    (1<<1)
        #define NDWS            (1<<0)



#endif


/*
 * UART
 */
#define BSP_UART0_BASE      0xB8002000
#define BSP_UART0_MAP_BASE  0x18002000
#define BSP_UART0_RBR       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_THR       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_DLL       (BSP_UART0_BASE + 0x000)
#define BSP_UART0_IER       (BSP_UART0_BASE + 0x004)
#define BSP_UART0_DLM       (BSP_UART0_BASE + 0x004)
#define BSP_UART0_IIR       (BSP_UART0_BASE + 0x008)
#define BSP_UART0_FCR       (BSP_UART0_BASE + 0x008)
#define BSP_UART0_LCR       (BSP_UART0_BASE + 0x00C)
#define BSP_UART0_MCR       (BSP_UART0_BASE + 0x010)
#define BSP_UART0_LSR       (BSP_UART0_BASE + 0x014)

#define BSP_UART1_BASE      0xB8002100
#define BSP_UART1_RBR       (BSP_UART1_BASE + 0x000)
#define BSP_UART1_THR       (BSP_UART1_BASE + 0x000)
#define BSP_UART1_DLL       (BSP_UART1_BASE + 0x000)
#define BSP_UART1_IER       (BSP_UART1_BASE + 0x004)
#define BSP_UART1_DLM       (BSP_UART1_BASE + 0x004)
#define BSP_UART1_IIR       (BSP_UART1_BASE + 0x008)
#define BSP_UART1_FCR       (BSP_UART1_BASE + 0x008)
   #define BSP_FCR_EN          0x01
   #define BSP_FCR_RXRST       0x02
   #define     BSP_RXRST             0x02
   #define BSP_FCR_TXRST       0x04
   #define     BSP_TXRST             0x04
   #define BSP_FCR_DMA         0x08
   #define BSP_FCR_RTRG        0xC0
   #define     BSP_CHAR_TRIGGER_01   0x00
   #define     BSP_CHAR_TRIGGER_04   0x40
   #define     BSP_CHAR_TRIGGER_08   0x80
   #define     BSP_CHAR_TRIGGER_14   0xC0
#define BSP_UART1_LCR       (BSP_UART1_BASE + 0x00C)
   #define BSP_LCR_WLN         0x03
   #define     BSP_CHAR_LEN_5        0x00
   #define     BSP_CHAR_LEN_6        0x01
   #define     BSP_CHAR_LEN_7        0x02
   #define     BSP_CHAR_LEN_8        0x03
   #define BSP_LCR_STB         0x04
   #define     BSP_ONE_STOP          0x00
   #define     BSP_TWO_STOP          0x04
   #define BSP_LCR_PEN         0x08
   #define     BSP_PARITY_ENABLE     0x01
   #define     BSP_PARITY_DISABLE    0x00
   #define BSP_LCR_EPS         0x30
   #define     BSP_PARITY_ODD        0x00
   #define     BSP_PARITY_EVEN       0x10
   #define     BSP_PARITY_MARK       0x20
   #define     BSP_PARITY_SPACE      0x30
   #define BSP_LCR_BRK         0x40
   #define BSP_LCR_DLAB        0x80
   #define     BSP_DLAB              0x80
#define BSP_UART1_MCR       (BSP_UART1_BASE + 0x010)
#define BSP_UART1_LSR       (BSP_UART1_BASE + 0x014)
   #define BSP_LSR_DR          0x01
   #define     BSP_RxCHAR_AVAIL      0x01
   #define BSP_LSR_OE          0x02
   #define BSP_LSR_PE          0x04
   #define BSP_LSR_FE          0x08
   #define BSP_LSR_BI          0x10
   #define BSP_LSR_THRE        0x20
   #define     BSP_TxCHAR_AVAIL      0x00
   #define     BSP_TxCHAR_EMPTY      0x20
   #define BSP_LSR_TEMT        0x40
   #define BSP_LSR_RFE         0x80

#ifdef CONFIG_USE_UART1
	#define _UART_LSR      BSP_UART1_LSR
	#define _UART_FCR      BSP_UART1_FCR
	#define _UART_THR      BSP_UART1_THR
	#define _UART_LCR      BSP_UART1_LCR
	#define _UART_IER      BSP_UART1_IER
	#define _UART_DLL      BSP_UART1_DLL
	#define _UART_DLM      BSP_UART1_DLM
	#define _UART_IRQ      BSP_UART1_IRQ
	#define _UART_BASE	BSP_UART1_BASE
	#define _UART_RBR	BSP_UART1_RBR
	#define _UART_IE	BSP_UART1_IE
	#define _UART1_LSR      BSP_UART0_LSR
	#define _UART1_FCR      BSP_UART0_FCR
	#define _UART1_THR      BSP_UART0_THR
	#define _UART1_LCR      BSP_UART0_LCR
	#define _UART1_IER      BSP_UART0_IER
	#define _UART1_DLL      BSP_UART0_DLL
	#define _UART1_DLM      BSP_UART0_DLM
	#define _UART1_IRQ      BSP_UART0_IRQ
	#define _UART1_BASE	BSP_UART0_BASE
	#define _UART1_RBR      BSP_UART0_RBR
	#define _UART1_IE       BSP_UART0_IE
#else //CONFIG_USE_UART0
	#define _UART_LSR      BSP_UART0_LSR
	#define _UART_FCR      BSP_UART0_FCR
	#define _UART_THR      BSP_UART0_THR
	#define _UART_LCR      BSP_UART0_LCR
	#define _UART_IER      BSP_UART0_IER
	#define _UART_DLL      BSP_UART0_DLL
	#define _UART_DLM      BSP_UART0_DLM
	#define _UART_IRQ      BSP_UART0_IRQ
	#define _UART_BASE	BSP_UART0_BASE
	#define _UART_RBR      BSP_UART0_RBR
	#define _UART_IE       BSP_UART0_IE
	#define _UART1_LSR      BSP_UART1_LSR
	#define _UART1_FCR      BSP_UART1_FCR
	#define _UART1_THR      BSP_UART1_THR
	#define _UART1_LCR      BSP_UART1_LCR
	#define _UART1_IER      BSP_UART1_IER
	#define _UART1_DLL      BSP_UART1_DLL
	#define _UART1_DLM      BSP_UART1_DLM
	#define _UART1_IRQ      BSP_UART1_IRQ
	#define _UART1_BASE     BSP_UART1_BASE
	#define _UART1_RBR      BSP_UART1_RBR
	#define _UART1_IE       BSP_UART1_IE
#endif



/*
 * Interrupt Controller
 */
#define BSP_GIMR0_0         0xB8003000
   #define BSP_TMO_IE          	(1 << 31)
   #define BSP_VOIPACC_IE     	(1 << 30)
   #define BSP_SPI_IE     		(1 << 29)
   #define BSP_PTM_IE          	(1 << 28)
   #define BSP_PKT_IE          	(1 << 27)
   #define BSP_GMAC_IE       	(1 << 26)
   #define BSP_SAR_IE          	(1 << 25)
   #define BSP_PKT_GMAC_IE      (1 << 24)
   #define BSP_PKT_XTM_IE       (1 << 23)
   #define BSP_DMT_IE          	(1 << 22)
   #define BSP_GDMA1_IE         (1 << 21)
   #define BSP_GDMA0_IE     	(1 << 20)
   #define BSP_SECURITY_IE      (1 << 19)
   #define BSP_PCM1_IE          (1 << 18)
   #define BSP_PCM0_IE    		(1 << 17)
   #define BSP_PCIE1_IE    		(1 << 16)
   #define BSP_PCIE0_IE         (1 << 15)
   #define BSP_USB_H2_IE        (1 << 14)
   #define BSP_USB_H3_IE        (1 << 13)
   #define BSP_PERIPHERAL_IE    (1 << 12)
   #define BSP_SWITCH_IE        (1 << 8)
   #define BSP_SATA_IE     		(1 << 7)
   #define BSP_NFBI_IE     		(1 << 6)
   #define BSP_WDOG_IE        	(1 << 5)
   #define BSP_FFTACC_IE     	(1 << 4)
   #define BSP_FLSH_IE     		(1 << 3)
   #define BSP_UTMD_IE 			(1 << 2)
   #define BSP_CUP0_T_CPU1_IE	(1 << 1)
   #define BSP_CUP1_T_CPU0_IE	(1 << 0)

#define BSP_GIMR1_0            0xB8003004
   #define BSP_LBCTMOm2_IE      (1 << 31)
   #define BSP_LBCTMOm1_IE	    (1 << 30)
   #define BSP_LBCTMOm0_IE  	(1 << 29)
   #define BSP_LBCTMOs3_IE      (1 << 28)
   #define BSP_LBCTMOs2_IE      (1 << 27)
   #define BSP_LBCTMOs1_IE      (1 << 26)
   #define BSP_LBCTMOs0_IE      (1 << 25)
   #define BSP_OCPTO1_IE        (1 << 24)
   #define BSP_OCPTO0_IE        (1 << 23)
   #define BSP_OCP_CPU0_ILA_IP_IE  (1 << 22)
   #define BSP_OCP_CPU1_ILA_IP_IE  (1 << 21)
   #define BSP_UART3_IE     	(1 << 20)
   #define BSP_UART2_IE         (1 << 19)
   #define BSP_UART1_IE         (1 << 18)
   #define BSP_UART0_IE    		(1 << 17)
   #define BSP_TC5_IE    		(1 << 16)
   #define BSP_TC4_IE           (1 << 15)
   #define BSP_TC3_IE           (1 << 14)
   #define BSP_TC2_IE           (1 << 13)
   #define BSP_TC1_IE           (1 << 12)
   #define BSP_TC0_IE	        (1 << 11)  
   #define BSP_GPIO_IE1			(1 << 10)  
   #define BSP_GPIO_IE0			(1 << 9)  


#define BSP_GISR0_0           0xB8003008
   #define BSP_TMO_IP          	(1 << 31)
   #define BSP_VOIPACC_IP     	(1 << 30)
   #define BSP_SPI_IP     		(1 << 29)
   #define BSP_PTM_IP          	(1 << 28)
   #define BSP_PKT_IP          	(1 << 27)
   #define BSP_GMAC_IP       	(1 << 26)
   #define BSP_SAR_IP          	(1 << 25)
   #define BSP_PKT_GMAC_IP      (1 << 24)
   #define BSP_PKT_XTM_IP       (1 << 23)
   #define BSP_DMT_IP          	(1 << 22)
   #define BSP_GDMA1_IP         (1 << 21)
   #define BSP_GDMA0_IP     	(1 << 20)
   #define BSP_SECURITY_IP      (1 << 19)
   #define BSP_PCM1_IP          (1 << 18)
   #define BSP_PCM0_IP    		(1 << 17)
   #define BSP_PCIE1_IP    		(1 << 16)
   #define BSP_PCIE0_IP         (1 << 15)
   #define BSP_USB_H2_IP        (1 << 14)
   #define BSP_USB_H3_IP        (1 << 13)
   #define BSP_PERIPHERAL_IP    (1 << 12)
   #define BSP_SWITCH_IP        (1 << 8)
   #define BSP_SATA_IP     		(1 << 7)
   #define BSP_NFBI_IP     		(1 << 6)
   #define BSP_WDOG_IP        	(1 << 5)
   #define BSP_FFTACC_IP     	(1 << 4)
   #define BSP_FLSH_IP     		(1 << 3)
   #define BSP_UTMD_IP       	(1 << 2)
   #define BSP_CUP0_T_CPU1_IP	(1 << 1)
   #define BSP_CUP1_T_CPU0_IP	(1 << 0)

#define BSP_GISR1_0            0xB800300C
   #define BSP_LBCTMOm2_IP      (1 << 31)
   #define BSP_LBCTMOm1_IP     	(1 << 30)
   #define BSP_LBCTMOm0_IP     	(1 << 29)
   #define BSP_LBCTMOs3_IP      (1 << 28)
   #define BSP_LBCTMOs2_IP      (1 << 27)
   #define BSP_LBCTMOs1_IP      (1 << 26)
   #define BSP_LBCTMOs0_IP      (1 << 25)
   #define BSP_OCPTO1_IP        (1 << 24)
   #define BSP_OCPTO0_IP        (1 << 23)
   #define BSP_OCP_CPU0_ILA_IP_IP  (1 << 22)
   #define BSP_OCP_CPU1_ILA_IP_IP  (1 << 21)
   #define BSP_UART3_IP         (1 << 20)
   #define BSP_UART2_IP         (1 << 19)
   #define BSP_UART1_IP         (1 << 18)
   #define BSP_UART0_IP    	    (1 << 17)
   #define BSP_TC5_IP           (1 << 16)
   #define BSP_TC4_IP           (1 << 15)
   #define BSP_TC3_IP           (1 << 14)
   #define BSP_TC2_IP           (1 << 13)
   #define BSP_TC1_IP           (1 << 12)
   #define BSP_TC0_IP	        (1 << 11)  
   #define BSP_GPIO1_IP       	(1 << 10)     
#define BSP_IRR0_0         0xB8003010
#define BSP_IRR0_0_SETTING    ((BSP_LBCTMOm2_RS << 28) | \
					(BSP_LBCTMOm1_RS << 24) | \
					(BSP_LBCTMOm0_RS << 20) | \
					(BSP_LBCTMOs3_RS << 16) | \
					(BSP_LBCTMOs2_RS << 12) | \
					(BSP_LBCTMOs1_RS << 8) | \
					(BSP_LBCTMOs0_RS << 4) | \
					(BSP_OCPTO1_RS	 << 0) \
					)

#define BSP_IRR1_0          0xB8003014
#define BSP_IRR1_0_SETTING    ((BSP_OCPTO0_RS << 28) | \
					(BSP_OCP1_CPU0_ILA_RS << 24) | \
					(BSP_OCP0_CPU0_ILA_RS << 20) | \
					(BSP_VOIPACC_RS << 16) | \
					(BSP_SPI_RS << 12) | \
					(BSP_PTM_RS << 8) | \
					(BSP_PKT_RS << 4) | \
					(BSP_GMAC_RS << 0) \
					)

#define BSP_IRR2_0           0xB8003018
#define BSP_IRR2_0_SETTING    ((BSP_SAR_RS << 28) | \
					(BSP_PKT_GMAC_RS << 24) | \
					(BSP_PKT_XTM_RS << 20) | \
					(BSP_DMT_RS << 16) | \
					(BSP_GDMA1_RS << 12) | \
					(BSP_GDMA0_RS << 8) | \
					(BSP_Security_RS << 4) | \
					(BSP_PCM1_RS<< 0) \
					)

#define BSP_IRR3_0           0xB800301C
#define BSP_IRR3_0_SETTING    ((BSP_PCM0_RS << 28) | \
					(BSP_PCIE1_RS << 24) | \
					(BSP_PCIE0_RS << 20) | \
					(BSP_USB_H2_RS << 16) | \
					(BSP_USB_H3_RS << 12) | \
					(BSP_UART3_RS << 8) | \
					(BSP_UART2_RS << 4) | \
					(BSP_UART1_RS<< 0) \
					)

#define BSP_IRR4_0           0xB8003020
#define BSP_IRR4_0_SETTING    ((BSP_UART0_RS << 28) | \
					(BSP_TC5_RS << 24) | \
					(BSP_TC4_RS << 20) | \
					(BSP_TC3_RS << 16) | \
					(BSP_TC2_RS << 12) | \
					(BSP_TC1_RS << 8) | \
					(BSP_TC0_RS << 4) | \
					(BSP_GPIO1_RS << 0) \
					)

#define BSP_IRR5_0           0xB8003024
#define BSP_IRR5_0_SETTING    ((BSP_GPIO0_RS << 28) | \
					(BSP_SWITCH_RS << 24) | \
					(BSP_SATA_RS << 20) | \
					(BSP_NFBI_RS << 16) | \
					(BSP_WDOG_RS << 12) | \
					(BSP_FFTACC_RS << 8) | \
					(BSP_FLSH_RS << 4) | \
					(BSP_UTMD_RS<< 0) \
					)
#define BSP_IRR6_0           0xB8003028
#define BSP_IRR6_0_SETTING    ((BSP_CUP0_T_CPU1_RS << 28) | \
					(BSP_CUP1_T_CPU0_RS << 24) \
					)

#define BOND_CHIP_MODE 0xBB010008	/* bond chip information */
		#define PAD_BOND_DMY_0 (1 << 0) //reserved
		#define PAD_BOND_DMY_1 (1 << 1)
		#define PAD_BOND_DMY_2 (1 << 2)
		#define PAD_BOND_CHIP_MODE_0	(1 << 3)
		#define PAD_BOND_CHIP_MODE_1	(1 << 4)
        #define CHIP_901      (PAD_BOND_DMY_0|PAD_BOND_DMY_1)
        #define CHIP_906_1    (PAD_BOND_DMY_0|PAD_BOND_DMY_1|PAD_BOND_DMY_2)
        #define CHIP_906_2    (PAD_BOND_DMY_0|PAD_BOND_CHIP_MODE_0)
        #define CHIP_907      (PAD_BOND_CHIP_MODE_0|PAD_BOND_DMY_0|PAD_BOND_DMY_1)
        #define CHIP_902      (PAD_BOND_CHIP_MODE_0|PAD_BOND_DMY_0|PAD_BOND_DMY_2)
        #define CHIP_903      (PAD_BOND_CHIP_MODE_0|PAD_BOND_DMY_0|PAD_BOND_DMY_1|PAD_BOND_DMY_2)
        #define CHIP_96       (PAD_BOND_CHIP_MODE_1|PAD_BOND_DMY_0|PAD_BOND_DMY_1)
        #define CHIP_98B      (PAD_BOND_CHIP_MODE_1|PAD_BOND_DMY_0|PAD_BOND_DMY_1|PAD_BOND_DMY_2)
	#define CHIP_2510     (PAD_BOND_CHIP_MODE_0|PAD_BOND_CHIP_MODE_1|PAD_BOND_DMY_0)
        #define CHIP_9607P    (PAD_BOND_CHIP_MODE_1|PAD_BOND_DMY_2|PAD_BOND_DMY_0)
        #define CHIP_DBG      (PAD_BOND_CHIP_MODE_0|PAD_BOND_CHIP_MODE_1|PAD_BOND_DMY_0|PAD_BOND_DMY_1|PAD_BOND_DMY_2)
        #define CHIP_9602B    (                                          PAD_BOND_DMY_2               |PAD_BOND_DMY_0)

/*
 * Timer/Counter
 */
#define BSP_TC_BASE         0xB8003200   //czyao: 0xb8003100 -> 0xb8003200
#define BSP_TC0DATA         (BSP_TC_BASE + 0x00)
#define BSP_TC4DATA         (BSP_TC_BASE + 0x40)
#define BSP_TCCNR           (BSP_TC_BASE + 0x10)
   #define BSP_TC0EN           (1 << 31)
   #define BSP_TC0MODE_TIMER   (1 << 30)
   #define BSP_TC1EN           (1 << 29)
   #define BSP_TC1MODE_TIMER   (1 << 28)
   
   #define BSP_TC2EN           (1 << 27)
   #define BSP_TC2MODE_TIMER   (1 << 26)
   #define BSP_TC3EN           (1 << 25)
   #define BSP_TC3MODE_TIMER   (1 << 24)
   #define BSP_TC4EN           (1 << 23)
   #define BSP_TC4MODE_TIMER   (1 << 22)   
   #define BSP_TC5EN           (1 << 21)
   #define BSP_TC5MODE_TIMER   (1 << 20)     
#define BSP_TCIR               (BSP_TC_BASE + 0x14)
   #define BSP_TC0IE           	(1 << 31)
   #define BSP_TC1IE           	(1 << 30)
   #define BSP_TC0IP           	(1 << 29)
   #define BSP_TC1IP           	(1 << 28)

   #define BSP_TC2IE           (1 << 27)
   #define BSP_TC3IE           (1 << 26)
   #define BSP_TC2IP           (1 << 25)
   #define BSP_TC3IP           (1 << 24)
   #define BSP_TC4IE           (1 << 23)
   #define BSP_TC5IE           (1 << 22)
   #define BSP_TC4IP           (1 << 21)
   #define BSP_TC5IP           (1 << 20) 
#define BSP_CDBR              (BSP_TC_BASE + 0x18)
#define BSP_TC_BASE         0xB8003200   //czyao: 0xb8003100 -> 0xb8003200
#define BSP_TC0DATA         (BSP_TC_BASE + 0x00)
#define BSP_TC1DATA         (BSP_TC_BASE + 0x10)
#define BSP_TC2DATA         (BSP_TC_BASE + 0x20)
#define BSP_TC3DATA         (BSP_TC_BASE + 0x30)
#define BSP_TC4DATA         (BSP_TC_BASE + 0x40)
#define BSP_TC5DATA         (BSP_TC_BASE + 0x50)
   #define BSP_TCD_OFFSET      0           //czyao: 8->4
#define BSP_TC0CNT          (BSP_TC_BASE + 0x04)
#define BSP_TC1CNT          (BSP_TC_BASE + 0x14)
#define BSP_TC2CNT          (BSP_TC_BASE + 0x24)
#define BSP_TC3CNT          (BSP_TC_BASE + 0x34)
#define BSP_TC4CNT          (BSP_TC_BASE + 0x44)
#define BSP_TC5CNT          (BSP_TC_BASE + 0x54)
#define BSP_TC0CTL          (BSP_TC_BASE + 0x08)
#define BSP_TC1CTL          (BSP_TC_BASE + 0x18)
#define BSP_TC2CTL          (BSP_TC_BASE + 0x28)
#define BSP_TC3CTL          (BSP_TC_BASE + 0x38)
#define BSP_TC4CTL          (BSP_TC_BASE + 0x48)
#define BSP_TC5CTL          (BSP_TC_BASE + 0x58)
   #define BSP_TCEN           (1 << 28)
   #define BSP_TCMODE_TIMER   (1 << 24)
   #define BSP_DIVF_OFFSET     0
#define BSP_TC0INT          (BSP_TC_BASE + 0x0C)
#define BSP_TC1INT          (BSP_TC_BASE + 0x1C)
#define BSP_TC2INT          (BSP_TC_BASE + 0x2C)
#define BSP_TC3INT          (BSP_TC_BASE + 0x3C)
#define BSP_TC4INT          (BSP_TC_BASE + 0x4C)
#define BSP_TC5INT          (BSP_TC_BASE + 0x5C)
   #define BSP_TCIE           	(1 << 20)
   #define BSP_TCIP           	(1 << 16)
#define BSP_WDCNTRR          (BSP_TC_BASE + 0x60)
#define BSP_WDTINTRR         (BSP_TC_BASE + 0x64)
#define BSP_WDTCTRLR        (BSP_TC_BASE + 0x68)

/*
* Watchdog
*/

#define WDT_E                         (1<<31)           /* Watchdog enable */
#define WDT_KICK                      (0x1 << 31)	/* Watchdog kick */
#define WDT_PH12_TO_MSK               31	       /* 11111b */
#define WDT_PH1_TO_SHIFT              22
#define WDT_PH2_TO_SHIFT	      15
#define WDT_CLK_SC_SHIFT      	 	29


/*
 * System Clock
 */
#define BSP_SCCR	0xB8003200
#define BSP_PLL_CTRL	(BSP_SCCR + 0x04)
#define BSP_PLL2_CTRL	(BSP_SCCR + 0x08)
#define BSP_ANA1_CTRL	(BSP_SCCR + 0x14)
#define BSP_PLL3_CTRL	(BSP_SCCR + 0x18)
#define BSP_LDO_CTRL	(BSP_SCCR + 0x20)

/*
 * System MISC Control Register
 */
#define BSP_IP_SEL				0xb8000600
#define BSP_USBPHY_P1_EN (1<<31)
#define BSP_USBPHY_P0_EN (1<<30)
#define BSP_ENABLE_PCIE0 (1<<7)
#define BSP_ENABLE_PCIE1 (1<<6)
#define BSP_EN_PCIE BSP_ENABLE_PCIE0
#define BSP_EN_USB_PHY0_USB2 		(0x1 << 4)
#define BSP_EN_USB_PHY0_USB2_USB3	(0x3 << 4)
#define BSP_EN_USB_PHY1 		(1 << 3)
#define BSP_EN_GMAC (1<<1)

#define BSP_SYS_CLK_SEL 0xBB000194
#define BSP_MISC_PINSR BSP_SYS_CLK_SEL
#define BSP_SYS_CLK_SRC_40MHZ (1 << 10) //1:40MHz, 0:25MHz
#define BSP_CLKSEL				BSP_SYS_CLK_SRC_40MHZ
#define BSP_PCI_MISC 0xb8000504
#define BSP_PCI_MDIO_RESET_ASSERT 0x00900000
#define BSP_PCI_MDIO_RESET_RELEASE 0x01b00000
//ccwei: 120208-WPS
/*Interrupt mode*/
#define DIS_ISR          0
#define EN_FALL_EDGE_ISR 1
#define EN_RISE_EDGE_ISR 2
#define EN_BOTH_EDGE_ISR 3
//end

/* ADSL State */
#define C_AMSW_IDLE                0
#define C_AMSW_L3                  1
#define C_AMSW_ACTIVATING          3
#define C_AMSW_SLOWBLINK_STATE     4
#define C_AMSW_INITIALIZING        6
#define C_AMSW_SHOWTIME_L0         9
#define C_AMSW_END_OF_LD          15

/*
* USB
*/
enum _USB_MAC_PORT_ {
	USB_MAC_EHCI_P0 = 1,
	USB_MAC_EHCI_P1,
	USB_MAC_OTG,
	USB_MAC_XHCI
};

#define BSP_USB_PHY_CTRL		0xB8000500
	#define BSP_USB_UTMI_RESET1		(1 << 28)
	#define BSP_USB_UTMI_RESET0		(1 << 12)

#define BSP_OHCI_BASE			0xB8020000

#define BSP_EHCI_BASE			0xB8021000
#define BSP_EHCI_UTMI_CTRL		(BSP_EHCI_BASE + 0xA4)

#define BSP_DWC_OTG_BASE		0xB8030000
#define BSP_DWC_OTG_PHY_CTRL	(BSP_DWC_OTG_BASE + 0x34)

#define BSP_XHCI_BASE			0xB8040000
	#define BSP_XHCI_USB2_PHY_CTRL	(BSP_XHCI_BASE + 0xC280)

#define BSP_USB3_EXT_REG		0xB8140000
	#define BSP_USB3_MDIO		(BSP_USB3_EXT_REG + 0x0)

#define BSP_USB2_EXT_REG		0xB8140200


/*
 * PCIE Host Controller
 */
#define BSP_PCIE0_H_CFG     0xB8B00000
#define BSP_PCIE0_H_EXT     0xB8B01000
#define BSP_PCIE0_H_MDIO    (BSP_PCIE0_H_EXT + 0x00)
#define BSP_PCIE0_H_INTSTR  (BSP_PCIE0_H_EXT + 0x04)
#define BSP_PCIE0_H_PWRCR   (BSP_PCIE0_H_EXT + 0x08)
#define BSP_PCIE0_H_IPCFG   (BSP_PCIE0_H_EXT + 0x0C)
#define BSP_PCIE0_H_MISC    (BSP_PCIE0_H_EXT + 0x10)
#define BSP_PCIE0_D_CFG0    0xB8B10000
#define BSP_PCIE0_D_CFG1    0xB8B11000
#define BSP_PCIE0_D_MSG     0xB8B12000

#define BSP_PCIE1_H_CFG     0xB8B20000
#define BSP_PCIE1_H_EXT     0xB8B21000
#define BSP_PCIE1_H_MDIO    (BSP_PCIE1_H_EXT + 0x00)
#define BSP_PCIE1_H_INTSTR  (BSP_PCIE1_H_EXT + 0x04)
#define BSP_PCIE1_H_PWRCR   (BSP_PCIE1_H_EXT + 0x08)
#define BSP_PCIE1_H_IPCFG   (BSP_PCIE1_H_EXT + 0x0C)
#define BSP_PCIE1_H_MISC    (BSP_PCIE1_H_EXT + 0x10)
#define BSP_PCIE1_D_CFG0    0xB8B30000
#define BSP_PCIE1_D_CFG1    0xB8B31000
#define BSP_PCIE1_D_MSG     0xB8B32000

#define BSP_PCIE0_D_IO      0xB8C00000
#define BSP_PCIE1_D_IO      0xB8E00000
#define BSP_PCIE_FUN_OFS      0xC00000
#define BSP_PCIE0_D_MEM     0xB9000000
#define BSP_PCIE0_F1_D_MEM  (BSP_PCIE0_D_MEM + BSP_PCIE_FUN_OFS)
#define BSP_PCIE1_D_MEM     0xBA000000
#define BSP_PCIE1_F1_D_MEM  (BSP_PCIE1_D_MEM + BSP_PCIE_FUN_OFS)

#if defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
//b[31:24]=GPIO_MOD[7:0], b[23:16]=GPIO_IO_SEL[7:0], b[15:8]=GPIO_OUT[]7:0], b[7:0]=GPIO_IN[7:0]
        #define RTL8192CD_GPIO_IN_OFFSET                0
        #define RTL8192CD_GPIO_OUT_OFFSET               8
        #define RTL8192CD_GPIO_IO_SEL_OFFSET    16
#if defined(CONFIG_SLOT_0_8812) && defined(CONFIG_WLAN0_5G_WLAN1_2G)
#define RTL8192CD_GPIO_PIN_CTRL         (BSP_PCIE0_D_MEM + 0x44)
#elif defined(CONFIG_SLOT_0_8192EE)
#define RTL8192CD_GPIO_PIN_CTRL		(BSP_PCIE0_D_MEM + 0x44)
#elif defined(CONFIG_SLOT_1_8192EE)
#define RTL8192CD_GPIO_PIN_CTRL         (BSP_PCIE1_D_MEM + 0x44)
#endif
#endif //CONFIG_RTL8192CD

/*
* LED
*/
#define IO_LED_EN 0xbb023014
#define LED_EN 0xbb01e0a0
#define DATA_LED_CFG 0xbb01e004
#define DATA_LED_CFG_1 (DATA_LED_CFG + 0x4)
#define DATA_LED_CFG_2 (DATA_LED_CFG + 0x8)
#define DATA_LED_CFG_3 (DATA_LED_CFG + 0xc)
#define DATA_LED_CFG_4 (DATA_LED_CFG + 0x10)
#define DATA_LED_CFG_5 (DATA_LED_CFG + 0x14)
#define DATA_LED_CFG_6 (DATA_LED_CFG + 0x18)
#define DATA_LED_CFG_7 (DATA_LED_CFG + 0x1c)
#define DATA_LED_CFG_8 (DATA_LED_CFG + 0x20)
#define DATA_LED_CFG_9 (DATA_LED_CFG + 0x24)
#define DATA_LED_CFG_10 (DATA_LED_CFG + 0x28)
#define DATA_LED_CFG_11 (DATA_LED_CFG + 0x2c)
#define DATA_LED_CFG_12 (DATA_LED_CFG + 0x30)
#define DATA_LED_CFG_13 (DATA_LED_CFG + 0x34)
#define DATA_LED_CFG_14 (DATA_LED_CFG + 0x38)
#define DATA_LED_CFG_15 (DATA_LED_CFG + 0x3c)

/* For UART 1 pin-mux settings */
#define SOC_0371_UART1_PINMUX     (0xbb02301c)
#define SOC_6266_IO_LED_EN_REG_A  (0xbb023014)
#define LED8_EN (0x1 << 8)
#define LED6_EN (0x1 << 6)
#define SOC_6266_UART1_PINMUX     (0xbb023018)
#define GPIO_CRTL_2_REG_A         (0xBB0000F0)
#define GPIO13_EN (0x1 << 13)
#define GPIO24_EN (0x1 << 24)

/* System reset */
#define SOFTWARE_RST_REG_A     (0xbb000074)
#define CMD_CHIP_RST_PS (0x1 << 2)



/*
 * chip ID detection
 */
#define IS_6028A		0
#define IS_6028B		0
#define IS_6085			0
#define IS_RLE0315		0
#define IS_6166			0
#define IS_8676V1		0
#define IS_8676V2		0
#define IS_RTL8676		0
#define IS_8676S_0562	0
#define IS_8676S_6333	0
#define IS_RTL8676S		0
#ifdef LUNA_RTL9602C
#define IS_RTL8686  0
#define IS_RTL9602C 1
#else
#define IS_RTL8686  1
#define IS_RTL9602C 0
#endif /* #ifdef LUNA_RTL9602C */
#define IS_RTL8685		0
#define IS_RTL8685S		0
/*start rtl8681 series*************/
#define IS_0437			0
#define IS_0513			0
#define IS_RTL8681		0
/*end rtl8681 series***************/

/* For SRAM Mapping*/
#define BSP_SOC_SRAM_SIZE  (32*1024) //32KByte
#define BSP_SOC_SRAM_BASE  (192*1024) //0x30000,32MByte
#define DRAM_SIZE_32KB      	(0x8)

/*
*	SRAM DRAM control registers
*/
//	CPU0
//	Unmaped Memory Segment Address Register
#define R_C0UMSAR0_BASE		(0xB8001300) /* DRAM UNMAP */
#ifdef CONFIG_LUNA_USE_SRAM
#define R_C0UMSAR0 			(R_C0UMSAR0_BASE + 0x00)
#define R_C0UMSAR1 			(R_C0UMSAR0_BASE + 0x10)
#define R_C0UMSAR2 			(R_C0UMSAR0_BASE + 0x20)
#define R_C0UMSAR3 			(R_C0UMSAR0_BASE + 0x30)

#define R_C0SRAMSAR0_BASE	(0xB8004000)
#define R_C0SRAMSAR0		(R_C0SRAMSAR0_BASE + 0x00)
#define R_C0SRAMSAR1		(R_C0SRAMSAR0_BASE + 0x10)
#define R_C0SRAMSAR2		(R_C0SRAMSAR0_BASE + 0x20)
#define R_C0SRAMSAR3		(R_C0SRAMSAR0_BASE + 0x30)
#endif

/* For DRAM controller */
#define C0DCR		(0xB8001004)
/* 
 * Logical addresses on OCP buses 
 * are partitioned into three zones, Zone 0 ~ Zone 2.
 */
/* ZONE0 */
#define  BSP_CDOR0         0xB8001700
#define  BSP_CDMAR0       (BSP_CDOR0 + 0x04)
/* ZONE1 */
#define  BSP_CDOR1         (BSP_CDOR0 + 0x10)
#define  BSP_CDMAR1       (BSP_CDOR0 + 0x14)
/* ZONE2 */
#define  BSP_CDOR2         (BSP_CDOR0 + 0x20)
#define  BSP_CDMAR2       (BSP_CDOR0 + 0x24)
#endif   /* _BSPCHIP_8686_H_ */

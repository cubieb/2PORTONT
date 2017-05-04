/*
 * Copyright 2006, Realtek Semiconductor Corp.
 *
 * rtl8196b/bsp/bspchip.h:
 *   RTL8196B chip-level header file
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Nov. 07, 2006
 */

#ifndef _BSPCHIP_8681_H_
#define _BSPCHIP_8681_H_

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
 *  RTL8681 IRQ Mapping
 */
 
#define BSP_MDIO_IRQ		23
#define BSP_PowerL_IRQ		22
#define BSP_CMU_IRQ			21
#define BSP_SCHM_IRQ		20
#define BSP_GPIO_ABCD_IRQ   	19
#define BSP_UART1_IRQ       	18
#define BSP_UART0_IRQ       	17
#define BSP_TC0_IRQ         		16
#define BSP_PKT_NIC100_IRQ   14
#define BSP_PKT_SAR_IRQ		13
#define BSP_PKT_IRQ         		12
#define BSP_NIC100_IRQ      	11
#define BSP_NFBI_IRQ			10
#define BSP_PTM_IRQ         		9
#define BSP_SAR_IRQ         		8
#define BSP_LBCTMOs1_IRQ    	6
#define BSP_LBCTMOs0_IRQ    	5
#define BSP_LBCTMOm1_IRQ    	4
#define BSP_LBCTMOm0_IRQ    	3
#define BSP_OCPTMO_IRQ      	2

/*
 * Interrupt Routing Selection
 */
#define BSP_PTM_RS          			2
#define BSP_LBCTMOs1_RS     		2
#define BSP_PKT_RS          			2
#define BSP_NIC100_RS       		2
#define BSP_SAR_RS          			2
#define BSP_PKT_NIC100_RS          	2
#define BSP_PKT_NIC100MII_RS        2
#define BSP_PKT_SAR_RS          		2
#define BSP_PCM_RS          			2
#define BSP_GPIO_ABCD_RS    		2
#define BSP_UART1_RS        		2
#define BSP_UART0_RS        		3
#define BSP_TC0_RS          			7
#define BSP_LBCTMOm1_RS     		2
#define BSP_LBCTMOs0_RS     		2
#define BSP_LBCTMOm0_RS     		2
#define BSP_OCPTMO_RS       		2
#define BSP_NFBI_RS		 		2
#define BSP_SCHM_RS				2
#define BSP_CMU_RS				2
#define BSP_PowerL_RS			2
#define BSP_MDIO_RS				2

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
#define CHIP_ID_REG		0xB8000024
#define CHIP_ID_offset		16

/*
 * Memory Controller
 */
#define BSP_MC_MCR          0xB8001000
   #define BSP_MC_MCR_VAL      0x92A28000

#define BSP_MC_MTCR0        0xB8001004
   #define BSP_MC_MTCR0_VAL    0x12120000

#define BSP_MC_MTCR1        0xB8001008
   #define BSP_MC_MTCR1_VAL    0x00000FEB

#define BSP_MC_PFCR         0xB8001010
   #define BSP_MC_PFCR_VAL     0x00000101


#define BSP_MC_BASE         0xB8001000
#define BSP_MC_MSRR    	(BSP_MC_BASE + 0x030)
#define FLUSH_OCP_CMD    		(1<<31)
#define BSP_NCR             	(BSP_MC_BASE + 0x100)
#define BSP_NSR             	(BSP_MC_BASE + 0x104)
#define BSP_NCAR            	(BSP_MC_BASE + 0x108)
#define BSP_NADDR           	(BSP_MC_BASE + 0x10C)
#define BSP_NDR             	(BSP_MC_BASE + 0x110)

#define BSP_SFCR            	(BSP_MC_BASE + 0x200)
#define BSP_SFDR            	(BSP_MC_BASE + 0x204)

static inline void write_buffer_flush(void){
        unsigned int cnt = 100;
        READ_MEM32(BSP_MC_MSRR) |= FLUSH_OCP_CMD;
        while((READ_MEM32(BSP_MC_MSRR) & FLUSH_OCP_CMD) && cnt-- );

        if(!cnt){
                //printk("%s %d: write buffer flush timeout!\n", __func__, __LINE__);
        }
}

/*
 * Memory Controller
 */
#define BSP_DCR		0xb8001004
	#define ARBIT 		(1<<20)
#define BSP_GIAR0	0xb8001010
#define BSP_GIAR1	0xb8001014
#define BSP_LXIAR0	0xb8001018
#define BSP_LXIAR1	0xb800101c
#define BSP_LXIAR2	0xb8001020


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
#endif



/*
 * Interrupt Controller
 */
#define BSP_GIMR            0xB8003000
  #define BSP_MDIO_IE			(1 << 23)
  #define BSP_PowerL_IE		(1 << 22)
  #define BSP_CMU_IE			(1 << 21)
  #define BSP_SCHM_IE			(1 << 20)
  #define BSP_GPIO_ABCD_IE    (1 << 19)
  #define BSP_UART1_IE        	(1 << 18)
  #define BSP_UART0_IE        	(1 << 17)
  #define BSP_TC0_IE         	 	(1 << 16)
  #define BSP_PKT_NIC100_IE    (1 << 14)
  #define BSP_PKT_SAR_IE      	(1 << 13)
  #define BSP_PKT_IE          		(1 << 12)
  #define BSP_NIC100_IE       	(1 << 11)
  #define BSP_NFBI_IE			(1 << 10)
  #define BSP_PTM_IE			(1 << 9)
  #define BSP_SAR_IE      		(1 << 8)
  #define BSP_LBCTMOs1_IE     	(1 << 6)
  #define BSP_LBCTMOs0_IE     	(1 << 5)	
  #define BSP_LBCTMOm1_IE     	(1 << 4)
  #define BSP_LBCTMOm0_IE     	(1 << 3)	
  #define BSP_OCPTMO_IE       	(1 << 2)	


#define BSP_GISR            0xB8003004
   #define BSP_MDIO_IP		(1 << 23)
   #define BSP_PowerL_IP		(1 << 22)
   #define BSP_CMU_IP			(1 << 21)
   #define BSP_SCHM_IP		(1 << 20)
   #define BSP_GPIO_ABCD_IP    (1 << 19)
   #define BSP_UART1_IP        	(1 << 18)
   #define BSP_UART0_IP        	(1 << 17)
   #define BSP_TC0_IP         	 	(1 << 16)
   #define BSP_PKT_NIC100_IP   (1 << 14)
   #define BSP_PKT_SAR_IP      	(1 << 13)
   #define BSP_PKT_IP          		(1 << 12)
   #define BSP_NIC100_IP       	(1 << 11)
   #define BSP_NFBI_IP			(1 << 10)
   #define BSP_PTM_IP			(1 << 9)
   #define BSP_SAR_IP      		(1 << 8)
   #define BSP_LBCTMOs1_IP     	(1 << 6)
   #define BSP_LBCTMOs0_IP     	(1 << 5)	
   #define BSP_LBCTMOm1_IP     (1 << 4)
   #define BSP_LBCTMOm0_IP     (1 << 3)	
   #define BSP_OCPTMO_IP       	(1 << 2)

#define BSP_IRR0            0xB8003008
#define BSP_IRR0_SETTING    ((0 << 28) | (BSP_LBCTMOs1_RS << 24) | \
			  (BSP_LBCTMOs0_RS << 20) | \
			  (BSP_LBCTMOm1_RS << 16) | \
			  (BSP_LBCTMOm0_RS << 12) | \
			  (BSP_OCPTMO_RS   << 8) | ( 0 << 4) | (0 << 0 ) \
			 )

#define BSP_IRR1            0xB800300C
#define BSP_IRR1_SETTING    ( (BSP_PKT_NIC100_RS	<< 24) | \
			  (BSP_PKT_SAR_RS << 20) | \
			  (BSP_PKT_RS << 16) | \
			  (BSP_NIC100_RS << 12) | \
			  (BSP_NFBI_RS << 8)  | \
			  (BSP_PTM_RS	<< 4)  | \
			  (BSP_SAR_RS	<< 0)	 \
			 )

#define BSP_IRR2            0xB8003010
#define BSP_IRR2_SETTING    ((BSP_MDIO_RS       << 28) | \
			  (BSP_PowerL_RS		<< 24) | \
			  (BSP_CMU_RS	  << 20) | \
			  (BSP_SCHM_RS	<< 16) | \
			  (BSP_GPIO_ABCD_RS 	  << 12) | \
			  (BSP_UART1_RS  << 8) | \
			  (BSP_UART0_RS << 4)  | \
			  (BSP_TC0_RS << 0)    \
			 )

#define BSP_IRR3            0xB8003014
#define BSP_IRR3_SETTING		0

/*
 * Timer/Counter
 */
#define BSP_TC_BASE         0xB8003100
#define BSP_TC0DATA         (BSP_TC_BASE + 0x00)
#define BSP_TC1DATA         (BSP_TC_BASE + 0x04)
   #define BSP_TCD_OFFSET      8
#define BSP_TC0CNT          (BSP_TC_BASE + 0x08)
#define BSP_TC1CNT          (BSP_TC_BASE + 0x0C)
#define BSP_TCCNR           (BSP_TC_BASE + 0x10)
   #define BSP_TC0EN           (1 << 31)
   #define BSP_TC0MODE_TIMER   (1 << 30)
   #define BSP_TC1EN           (1 << 29)
   #define BSP_TC1MODE_TIMER   (1 << 28)
#define BSP_TCIR            (BSP_TC_BASE + 0x14)
   #define BSP_TC0IE           (1 << 31)
   #define BSP_TC1IE           (1 << 30)
   #define BSP_TC0IP           (1 << 29)
   #define BSP_TC1IP           (1 << 28)
#define BSP_CDBR            (BSP_TC_BASE + 0x18)
   #define BSP_DIVF_OFFSET     16
#define BSP_WDTCNR          (BSP_TC_BASE + 0x1C)

#define WDTE_OFFSET                         24              /* Watchdog enable */
#define WDSTOP_PATTERN                      0xA5            /* Watchdog stop pattern */
#define WDTCLR                              (1 << 23)       /* Watchdog timer clear */
#define OVSEL_13                            0               /* Overflow select count 2^13 */


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
#define BSP_MISC_CR_BASE		0xB8000100	
#define BSP_MISC_PINMUX			(BSP_MISC_CR_BASE + 0x00)
	#define BSP_GPON_BD				(1 << 20)
	#define BSP_JTAG_GPIO_PINMUX	(1 << 9)
	#define BSP_BD_SLV_MODE		(1 << 3)
	#define BSP_BD_MST_MODE		(1 << 2)
#define BSP_MISC_PINOCR			(BSP_MISC_CR_BASE + 0x04)
#define BSP_MISC_PINSTSR		(BSP_MISC_CR_BASE + 0x08)
	#define BSP_SYS_CLK_SEL			(1 << 16)
#define BSP_IP_SEL				(BSP_MISC_CR_BASE + 0x0C)
	#define BSP_DSL_MAC_EN			(1 << 9)
	#define BSP_EN_SACHEM			(1 << 6)
	#define BSP_EN_PKTA				(1 << 4)
	#define BSP_EN_NFBI				(1 << 2)
	#define BSP_EN_PTM				(1 << 1)
	#define BSP_EN_SAR				(1 << 0)
#define BSP_MISC_IO_DRIVING		(BSP_MISC_CR_BASE + 0x14)
#define BSP_PCCR				(BSP_MISC_CR_BASE + 0x20)
	#define BSP_PCCR_PHYIF_0		(1 << 19)
	#define BSP_PCCR_SHF_CLK_DP		9
	#define BSP_PCCR_SHF_CLK_DN		6
	#define BSP_PCCR_SHF_DATA_DP	3
	#define BSP_PCCR_SHF_DATA_DN	0
#define BSP_LTOC					(BSP_MISC_CR_BASE + 0x24)


/* 
 *GPIO control registers 
*/
#define GPIOCR_BASE 0xB8003200

/*Port A,B,C,D*/
#define GPIO_PABCD_CNR		(GPIOCR_BASE+0x00)	/*Port A,B,C,D control register*/
#define GPIO_PABCD_PTYPE	(GPIOCR_BASE+0x04)	/*Port A,B,C,D peripheral type control register*/
#define GPIO_PABCD_DIR		(GPIOCR_BASE+0x08)	/*Port A,B,C,D direction */
#define GPIO_PABCD_DAT		(GPIOCR_BASE+0x0C)	/*Port A,B,C,D data register*/
#define GPIO_PABCD_ISR		(GPIOCR_BASE+0x10)	/*Port A,B,C,D interrupt status register*/
#define GPIO_PAB_IMR		(GPIOCR_BASE+0x14)	/*Port A,B interrupt mask register*/
#define GPIO_PCD_IMR		(GPIOCR_BASE+0x18)	/*Port C,D interrupt mask register*/
#define GPIO_PEFGH_DIR		(GPIOCR_BASE+0x24)	/*Port E,F,G,H direction */
#define GPIO_PEFGH_DAT		(GPIOCR_BASE+0x28)	/*Port E,F,G,H data register*/
#define GPIO_PEFGH_ISR		(GPIOCR_BASE+0x2c)	/*Port E,F,G,H interrupt status register*/
#define GPIO_PEF_IMR		(GPIOCR_BASE+0x30)	/*Port E,F interrupt mask register*/
#define GPIO_PGH_IMR		(GPIOCR_BASE+0x34)	/*Port G,H interrupt mask register*/


/*Port A*/
#define GPIO_PADIR	(GPIOCR_BASE+0x00L)	/*Port A direction register*/
#define GPIO_PADAT	(GPIOCR_BASE+0x04L)	/*Port A data register*/
#define GPIO_PAISR	(GPIOCR_BASE+0x08L)	/*Port A interrupt status register*/
#define GPIO_PAIMR	(GPIOCR_BASE+0x0CL)	/*Port A interrupt mask register*/
/*Port B*/
#define GPIO_PBDIR	(GPIOCR_BASE+0x10L)	/*Port B direction register*/
#define GPIO_PBDAT	(GPIOCR_BASE+0x14L)	/*Port B data register*/
#define GPIO_PBISR	(GPIOCR_BASE+0x18L)	/*Port B interrupt status register*/
#define GPIO_PBIMR	(GPIOCR_BASE+0x1CL)	/*Port B interrupt mask register*/

/* ADSL State */
#define C_AMSW_IDLE                0
#define C_AMSW_L3                  1
#define C_AMSW_ACTIVATING          3
#define C_AMSW_INITIALIZING        6
#define C_AMSW_SHOWTIME_L0         9
#define C_AMSW_END_OF_LD          15

/*
 * chip ID detection
 */
#define IS_6028A			0
#define IS_6028B			0
#define IS_6085			0
#define IS_RLE0315		0
#define IS_6166			0
#define IS_0412			0
#define IS_0437			((REG32(CHIP_ID_REG)>>CHIP_ID_offset == 0x0437) ? 1:0)
#define IS_0513			((REG32(CHIP_ID_REG)>>CHIP_ID_offset == 0x0513) ? 1:0)
#define IS_RTL8681		(IS_0437 || IS_0513)
#define IS_RTL8676		0

#endif   /* _BSPCHIP_8681_H */

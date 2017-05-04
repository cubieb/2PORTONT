#ifndef __PCSCREG__
#define __PCSCREG__

#define ICC_PAD_OE	0xFD54
#define SC_PAD_OE	(1<<0)

#define ICC_SC_SEL	0xFD5F
#define ASYNCHRONOUS_SC	(1<<0)
#define SIEMENS_SC	(1<<1)
#define ATMEL_SC	(1<<1|1<<0)

#define ICC_SC_CD	0xFD6F
#define CARD_INSERT	(1<<7)

#define ICC_SC_CD_INT	0xFD72
#define	CARD_CD_INTERRUPT_FLAG	(1<<1)
#define CARD_CD_INTERRUPT_EN	(1<<0)

#define ICC_SC_PAD_PULLCTL	0xFD73
#define ICC_CD_PS_NO	(0<<5|0<<4)
#define ICC_CD_PS_PULL_DOWN	(0<<5|1<<4)
#define ICC_CD_PS_PULL_UP_200K	(1<<5|1<<4)
#define	ICC_IO_PULL_SELECT_PULLUP	(1<<3)
#define ICC_CLK_PULL_SELECT	(1<<2)
#define ICC_RST_PULL_SELECT	(1<<1)

#define ICC_SC_BIST_EN	0xFD7F
#define	BIST_EN	(1<<1)
#define	HIGH_SPEED_DMA	(1<<0)

#define	ICC_PWR_CTL	0xFD80
#define	SC_5V_CTL	(1<<3)
#define	CLASSA_5	(0<<2|0<<1)
#define	CLASSB_3	(0<<2|1<<1)
#define CLASSC_1P8	(1<<2|0<<0)
#define	POWER_CTL_ON	(1<<0)
#define	POWER_CTL_OFF	(0<<0)

#define ICC_CLK_CTL	0xFD81
#define CLOCK_FREQ_3P75	(0<<4|0<<3)
#define	CLOCK_FREQ_7P5	(0<<4|1<<3)
#define	CLOCK_FREQ_15	(1<<4|0<<3)
#define	CLOCK_FREQ_1P875	(1<<4|1<<3)
#define	CLOCK_CTL_EN	(1<<2)
#define	CLOCK_CTL_DIS_EN_H	(1<<1)
#define	CLOCK_CTL_DIS_EN_L	(1<<0)

#define ICC_RST_CTL	0xFD82
#define	PARITY_ERR_RETRY_FAIL	(1<<7)
#define	T0_TRANSMIT_RETRY	(1<<6)
#define	T0_RECEIVE_RETRY	(1<<5)
#define	T0_PARITY_RETRY_LIMIT	(1<<4|1<<3|1<<2|1<<1)
#define	RESET_CTL_LOW	(0<<0)
#define	RESET_CTL_HIGH	(1<<0)

#define ICC_CTL0	0xFD83
#define PSS_AUTO_PCK_ON	(1<<3)
#define T1_AUTO_WEDC_ON	(1<<1)
#define	CRC_INIT_VALUE	(1<<0)

#define ICC_CTL1	0xFD84
#define ASYNCHRONOUS_ICC_CARD_STOP	(1<<7)
#define CLOCK_STOP	(1<<6|1<<5)
#define CLOCK_STOP_L	(0<<6|1<<5)
#define CLOCK_STOP_H	(1<<6|0<<5)
#define EDC_LRC	(0<<4)
#define EDC_CRC	(1<<4)
#define TRANS_PROTO_T0	(0<<0)
#define TRANS_PROTO_T1	(1<<0)

#define ICC_T0_CLA	0xFD85
#define ICC_T0_INS	0xFD86
#define ICC_T0_P1	0xFD87
#define ICC_T0_P2	0xFD88
#define ICC_T0_P3	0xFD89

// bit 2:0
#define ICC_PPS_RQST_LEN	0xFD8A

// Low byte of T1 Block length	bit 7:0
#define ICC_T1_BLK_LEN0	0xFD8B

// Highest bit of T1 Block length	bit 0
#define ICC_T1_BLK_LEN1	0xFD8C

// bit 7:0
#define ICC_ATR_LEN	0xFD8D

// bit 7:0
#define ICC_RESET_TCTL2_L	0xFD8E

#define ICC_RESET_TCTL2_H	0xFD8F

// Record number of T0 read or PPS response
#define ICC_STATUS1	0xFD90

#define ICC_STATUS2	0xFD91
#define PROCEDURE_ERR	(1<<7)
#define ICC_TIMEOUT_FLAG	(1<<6)
#define CRC_ERR	(1<<5)
#define LRC_ERR	(1<<4)
#define TCK_ATRLEN_ERR	(1<<3)
#define PCK_TS_ERR	(1<<2)
#define PARITY_ERR	(1<<1)
#define SW1_INDEX	(1<<0)

#define ICC_TIME_CTL_FLAG 0xFD92
#define ICC_RESET_ATR_DURATION_TIMEOUT	(1<<5)
#define ICC_RESET_RECEIVE_TIMEOUT	(1<<4)
#define ICC_T1_BWT_TIMEOUT_FLAG	(1<<3)
#define ICC_T1_CWT_TIMEOUT_FLAG	(1<<2)
#define ICC_T0_WWT_TIMEOUT_FLAG	(1<<1)

#define ICC_TRANSFER	0xFD93
#define START_TRANSFER	(1<<7)
#define END_TRNASFER	(1<<6)
#define T0_TIME_EXT		(1<<5)
#define TRANSFER_INTERRUPT_FLAG	(1<<4)
#define T0_WRITE	(1<<0)
#define T0_READ		(1<<1)
#define T1_BLK	(1<<1|1<<0)
#define PPS_REQ	(1<<2)
#define RESET2	(1<<2|1<<0)
#define RESET1	(0<<0)

#define ICC_ETU0	0xFD94
#define ICC_ETU1	0xFD95

#define ICC_GUARDTIME	0xFD96
#define ICC_OPP_GUARDTIME	0xFD97

#define ICC_T0_WWT0	0xFD98
#define ICC_T0_WWT1	0xFD99
#define ICC_T0_WWT2	0xFD9A

#define	ICC_RESET_TCTL0	0xFD9D
#define	ICC_RESET_TCTL1	0xFD9E

#define ICC_T0_STATUS	0xFD9F
#define ICC_T1_CWT0	0xFDC0
#define ICC_T1_CWT1	0xFDC1

#define ICC_T1_BWT0	0xFDC2
#define ICC_T1_BWT1	0xFDC3
#define ICC_T1_BWT2	0xFDC4
#define ICC_T1_BWT3	0xFDC5

#define	ICC_IO_DRIVING	0xFDC6
#define IO_NO_DRIVE	(0<<0|0<<0)
#define IO_DRIVE_LOW	(1<<0)
#define IO_DRIVE_HIGH	(1<<1)

#define ICC_LDO_1	0xFDC9
#define ICC_LDO_2	0xFDCA

#define SCB_CTL	0xFFE0
#define	U_SCB_BUS_16	(1<<1)
#define	U_SCB_SRC	(1<<0)

#define SCB_ADDR_OFFSET	0xFFE1

// data buffer from 0xee00 ~ 0xefff
#define	DATABUF 0xEE00


#define ICC_INTR_MASK	0xFDCD
#define ICC_PROCEDURE_ERR_MSK (1<<7)
#define ICC_TIMEOUT_FLAG_MSK (1<<6)
#define ICC_CRC_ERR_MSK (1<<5)
#define ICC_LRC_ERR_MSK (1<<4)
#define ICC_TCK_ATRLEN_ERR_MSK (1<<3)
#define ICC_PCK_TS_ERR_MSK (1<<2)
#define ICC_PARITY_ERR_MSK (1<<1)
#define ICC_PRTY_RETRY_FAIL_MSK (1<<0)

#define ICC_SC_MASK	0xFDCE
#define ICC_END_STATUS (1<<2)
#define ICC_END_MSK (1<<1)
#define ICC_TOTAL_MSK (1<<0)


#define ICC_DBG_STATUS01	0xFDCF
#define ICC_DBG_CLK_RETURN_ST	(1<<7)
#define ICC_DBG_CLK_STOP_ST	(1<<6)
#define ICC_DBG_BGT_OK_ST	(1<<5)
#define ICC_OPERATION_ST (1<<4)

#define ICC_SW1_STATUS	0xFDD0
#define ICC_SW2_STATUS	0xFDD1

typedef enum {
	SC_STATUS0_START,

	SC_PARITY_ERR,
	SC_PCK_TS_ERR,
	SC_TCK_ATRLEN_ERR,
	SC_LRC_ERR,
	SC_CRC_ERR,
	SC_ICC_TIMEOUT_FLAG,	
	SC_PROCEDURE_ERR,
	
	SC_DATAFINISH,
	
	SC_STATUS0_END
}SC_STATUS_TYPE1;

typedef enum {
	SC_STATUS1_START,
	
	SC_ICC_T0_WWT_TIMEOUT_FLAG,
	SC_ICC_T1_CWT_TIMEOUT_FLAG,
	SC_ICC_T1_BWT_TIMEOUT_FLAG,
	SC_ICC_RESET_RECEIVE_TIMEOUT,
	SC_ICC_RESET_ATR_DURATION_TIMEOUT,
	
	SC_STATUS1_END
}SC_STATUS_TYPE2;

static char *SC_STATUS1_MSG[] = {
	"",
	"SC_PARITY_ERR"	
	"SC_PCK_TS_ERR",	
	"SC_TCK_ATRLEN_ERR",	
	"SC_LRC_ERR",	
	"SC_CRC_ERR",	
	"SC_ICC_TIMEOUT_FLAG",	
	"SC_PROCEDURE_ERR",
};

static char *SC_STATUS2_MSG[] = {
	"",
	"SC_ICC_T0_WWT_TIMEOUT_FLAG",	
	"SC_ICC_T1_CWT_TIMEOUT_FLAG",	
	"SC_ICC_T1_BWT_TIMEOUT_FLAG",	
	"SC_ICC_RESET_RECEIVE_TIMEOUT",	
	"SC_ICC_RESET_ATR_DURATION_TIMEOUT"
};


#endif

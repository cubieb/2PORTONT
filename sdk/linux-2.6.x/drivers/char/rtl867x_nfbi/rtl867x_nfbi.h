#ifndef _RTL_NFBI_H_
#define _RTL_NFBI_H_

/*================================================================*/
/* Compiling Flags */
#define CHECK_NFBI_BUSY_BIT


/* Constant Definitions */
#define DRIVER_NAME			"rtl_nfbi"
#define DRIVER_VER			"0.2"
#define DRIVER_MAJOR		14

#define NFBI_USERHDR_SIZE	12
#define NFBI_USERDATA_SIZE	4096
#define NFBI_CHKSUM_SIZE	1
#define NFBI_BUFSIZE		(NFBI_USERHDR_SIZE+NFBI_USERDATA_SIZE) //516 //2 + 2 + 2*255 + 2
#define NFBI_MAX_BUFSIZE	(NFBI_BUFSIZE+NFBI_CHKSUM_SIZE)
#define EV_QUE_MAX			16 //128


#define NFBI_INTERRUPT_TIMEOUT_DEFAULT		    10	//in jiffies(10 ms)
#define NFBI_RESPONSE_TIMEOUT_DEFAULT           300	//in jiffies(10 ms)
#define NFBI_RETRANSMIT_COUNT_DEFAULT		    2


/* RTL867x MDC/MDIO Control Register */
#define NFBI_BASE					(0xB8019000)
#define NFBI_MCR					(0x34 + NFBI_BASE)
#define NFBI_MAR					(0x38 + NFBI_BASE)
#define NFBI_MDR					(0x3c + NFBI_BASE)
#define MDIO_TIMEOUT 				2000000
#define DEFAULT_MDIO_PHYAD 			1  /* selected by the hardware strapping pin of external Host CPU */


#ifndef __KERNEL__
#define BIT(x)						(1 << (x))
#endif //__KERNEL__


// RTL867x NFBI Register offset
#define NFBI_REG_UACR				0x00	// Utopia Access Control Register
#define NFBI_REG_UASR				0x01	// Utopia Access Status Register
#define NFBI_REG_PHYID1				0x02	// PHY Identifier Register 1
#define NFBI_REG_PHYID2				0x03	// PHY Identifier Register 2
#define NFBI_REG_FDCR				0x04	// Fast Delay Compensation Register
#define NFBI_REG_SDCR				0x05	// Slow Delay Compensation Register
#define NFBI_REG_WDOGCNT			0x06	// Watchdog counter
#define NFBI_REG_DDCH				0x0e	// DDR DRAM Calibration High Register
#define NFBI_REG_DDCL				0x0f	// DDR DRAM Calibration Low Register
#define NFBI_REG_CMD				0x10	// Command Register
#define NFBI_REG_ADDH				0x11	// Address High Register
#define NFBI_REG_ADDL				0x12	// Address Low Register
#define NFBI_REG_DH	    			0x13	// Data High Register
#define NFBI_REG_DL 				0x14	// Dta Low Register
#define NFBI_REG_SCR				0x15	// Send Command Register
#define NFBI_REG_RSR				0x16	// Receive Status Register
#define NFBI_REG_SYSSR				0x17	// System Status Register
#define NFBI_REG_SYSCR				0x18	// System Control Register
#define NFBI_REG_IMR				0x19	// Interrupt Mask Register
#define NFBI_REG_ISR				0x1a	// Interrupt Status Register
#define NFBI_REG_DCH				0x1b	// DRAM Configuration High Register
#define NFBI_REG_DCL				0x1c	// DRAM Configuration Low Register
#define NFBI_REG_DTH				0x1d	// DRAM Timing High Register
#define NFBI_REG_DTL				0x1e	// DRAM Timing Low Register
#define NFBI_REG_RR				    0x1f	// reserved register


// Default value
#define NFBI_REG_PHYID1_DEFAULT		0x001c	// Default value of PHY Identifier Register 1
#define NFBI_REG_PHYID2_DEFAULT		0xcb61	// Default value of PHY Identifier Register 2
#define NFBI_REG_PHYID2_DEFAULT2	0xcb81	// 8198 Default value of PHY Identifier Register 2
#define NFBI_REG_PHYID2_DEFAULT3	0xcbd0	// 8196D,8197D Default value of PHY Identifier Register 2
#define NFBI_REG_PHYID2_DEFAULT4	0xcbe0	// 8196E Default value of PHY Identifier Register 2


// bitmask definition for CMD (0x10)
#define BM_CMDTYPE       		BIT(15)	// Command Type
#define BM_BUSY     		    BIT(14)	// Status of NFBI hardware
#define BM_INTLEVEL			    BIT(2)	// Select interrupt level
#define BM_SYSTEMRST			BIT(1)
#define BM_START_RUN_BOOTCODE 	BIT(0)


// bitmask definition for SYSSR (0x17)
#define BM_CHECKSUM_DONE		BIT(15)
#define BM_CHECKSUM_OK		    BIT(14)
#define BM_DSLLINK			    BIT(12)
#define BM_ALLSOFTWARE_READY	BIT(10)
#define BM_FIRMWARE7			BIT(7)
#define BM_FIRMWARE6			BIT(6)
#define BM_BOOTCODE_READY   	BIT(5)
//#define BM_FIRMWARE5			BIT(5)
#define BM_FIRMWARE4			BIT(4)
#define BM_FIRMWARE3			BIT(3)
#define BM_FIRMWARE2			BIT(2)
#define BM_FIRMWARE1			BIT(1)
#define BM_FIRMWARE0			BIT(0)


// bitmask definition for SYSCR (0x18)
#define CR_ENUTPPHY				BIT(12)		// Enable Utopia PHY interface
#define CR_KERNELCODE_READY		BIT(8)		// KERNELCODE_READY
//#define CR_CUSTOM8			BIT(8)		// Customized used 8
#define CR_CUSTOM7				BIT(7)		// Customized used 7
#define CR_CUSTOM6				BIT(6)		// Customized used 6
#define CR_CUSTOM5				BIT(5)		// Customized used 5
#define CR_CUSTOM4				BIT(4)		// Customized used 4
#define CR_CUSTOM3				BIT(3)		// Customized used 3
#define CR_CUSTOM2				BIT(2)		// Previous msg has been fetched
#define CR_CUSTOM1				BIT(1)		// New msg has come
#define CR_CUSTOM0				BIT(0)		// Reserved


// bitmask definition for IMR (0x19)
#define IM_CHECKSUM_DONE		BIT(15)
#define IM_CHECKSUM_OK		    BIT(14)
#define IM_DSLLINK			    BIT(12)
#define IM_UASR_CHANGE			BIT(11)
#define IM_ALLSOFTWARE_READY	BIT(10)
#define IM_FIRMWARE7			BIT(7)
#define IM_FIRMWARE6			BIT(6)
#define IM_BOOTCODE_READY   	BIT(5)
//#define IM_FIRMWARE5			BIT(5)
#define IM_FIRMWARE4			BIT(4)
#define IM_FIRMWARE3			BIT(3)
#define IM_PREVMSG_FETCH	    BIT(2)
#define IM_NEWMSG_COMING		BIT(1)
#define IM_NEED_BOOTCODE		BIT(0)


// bitmask definition for ISR (0x1a)
#define IP_CHECKSUM_DONE		BIT(15)
#define IP_CHECKSUM_OK		    BIT(14)
#define IP_DSLLINK			    BIT(12)
#define IP_UASR_CHANGE			BIT(11)
#define IP_ALLSOFTWARE_READY	BIT(10)
#define IP_FIRMWARE7			BIT(7)
#define IP_FIRMWARE6			BIT(6)
#define IP_BOOTCODE_READY   	BIT(5)
//#define IP_FIRMWARE5			BIT(5)
#define IP_FIRMWARE4			BIT(4)
#define IP_FIRMWARE3			BIT(3)
#define IP_PREVMSG_FETCH	    BIT(2)	//	Previous msg has been fetched
#define IP_NEWMSG_COMING		BIT(1)	//	New msg has coming
#define IP_NEED_BOOTCODE		BIT(0)


#define IP_STATUS_MASK_NO_MSG  \
	( IP_CHECKSUM_DONE | IP_CHECKSUM_OK | IP_DSLLINK | IP_UASR_CHANGE | \
	  IP_BOOTCODE_READY | IP_ALLSOFTWARE_READY | IP_NEED_BOOTCODE )
#define IP_STATUS_MASK_ALL \
	( IP_STATUS_MASK_NO_MSG |IP_PREVMSG_FETCH | IP_NEWMSG_COMING )
#define NFBI_DEF_IMR_INT	(IM_DSLLINK|IM_PREVMSG_FETCH|IM_NEWMSG_COMING)


/*
 * Ioctl definitions
 */
/* Use 'k' as magic number */
#define NFBI_IOC_MAGIC						'k'
#define NFBI_IOCTL_PRIV_CMD             	_IOW(NFBI_IOC_MAGIC,  0,  int) //private command
#define NFBI_IOCTL_REGREAD              	_IOWR(NFBI_IOC_MAGIC, 1,  int)
#define NFBI_IOCTL_REGWRITE             	_IOW(NFBI_IOC_MAGIC,  2,  int)
#define NFBI_IOCTL_HCD_PID              	_IOW(NFBI_IOC_MAGIC,  3,  int) //set host control deamon PID to driver
#define NFBI_IOCTL_GET_EVENT            	_IOR(NFBI_IOC_MAGIC,  4,  struct evt_msg)
#define NFBI_IOCTL_MEM32_WRITE          	_IOW(NFBI_IOC_MAGIC,  5,  struct nfbi_mem32_param)
#define NFBI_IOCTL_MEM32_READ           	_IOWR(NFBI_IOC_MAGIC, 6,  struct nfbi_mem32_param)
#define NFBI_IOCTL_BULK_MEM_WRITE       	_IOW(NFBI_IOC_MAGIC,  7,  struct nfbi_bulk_mem_param)
#define NFBI_IOCTL_BULK_MEM_READ        	_IOWR(NFBI_IOC_MAGIC, 8,  struct nfbi_bulk_mem_param)
#define NFBI_IOCTL_TX_CMDWORD_INTERVAL  	_IOWR(NFBI_IOC_MAGIC, 9,  int)
#define NFBI_IOCTL_INTERRUPT_TIMEOUT    	_IOWR(NFBI_IOC_MAGIC, 10, int)
#define NFBI_IOCTL_RETRANSMIT_COUNT     	_IOWR(NFBI_IOC_MAGIC, 11, int)
#define NFBI_IOCTL_RESPONSE_TIMEOUT     	_IOWR(NFBI_IOC_MAGIC, 12, int)
#define NFBI_IOCTL_MDIO_PHYAD           	_IOWR(NFBI_IOC_MAGIC, 13, int)

#define NFBI_IOCTL_EW                   	_IOW(NFBI_IOC_MAGIC,  15, struct nfbi_mem32_param)
#define NFBI_IOCTL_DW                   	_IOWR(NFBI_IOC_MAGIC, 16, struct nfbi_mem32_param)
#define NFBI_IOCTL_MAXNR					16


// tx cmd state & rx status state
enum 
{ 
	STATE_TX_INIT,
	STATE_TX_IN_PROGRESS,
	STATE_RX_INIT,
	//STATE_RX_WAIT_LEN,
	STATE_RX_WAIT_DATA,
	STATE_RX_FINISHED 
};


/*================================================================*/
/* Structure Definition */
struct nfbi_mem32_param
{
    int addr;
    int val;
};

#define NFBI_MAX_BULK_MEM_SIZE 512
struct nfbi_bulk_mem_param
{
    int addr;
    int len;
    char buf[NFBI_MAX_BULK_MEM_SIZE];
};

struct evt_msg {
	int id;			// event id
	int status;
	int value;
};

struct buf_ar {
	int len;
	unsigned char buf[NFBI_MAX_BUFSIZE];
};


#define	NFBI_PPP_FLAG	0x7e	/* Flag Sequence */
#define	NFBI_PPP_ESCAPE	0x7d	/* Asynchronous Control Escape */
#define	NFBI_PPP_TRANS	0x20	/* Asynchronous transparency modifier */

#define NTX_START	0x01
#define NTX_END		0x02
#define NTX_ESC		0x04

#define NRX_START	0x01
#define NRX_END		0x02
#define NRX_ESC		0x04


#ifdef __KERNEL__
struct nfbi_priv 
{
	struct buf_ar 		data_out;
	struct buf_ar 		data_in;

	int					state;
	unsigned int		tx_flag;
	unsigned int		rx_flag;

	int                 retransmit_count;
	int					tx_cmd_transmitting_len;
    int					tx_cmdword_interval;//in jiffies(10 ms), 0~1000
	int                 interrupt_timeout;	//in jiffies(10 ms), 0~1000
	int                 response_timeout;	//in jiffies(10 ms)
};

struct nfbi_dev_priv
{
    int                 ready;
    int                 hcd_pid;

	struct semaphore    sem;            /* mutual exclusion semaphore */
	wait_queue_head_t   wq;             /* wait queue */
	struct	file		*filp;
	struct timer_list   timer;
	int                 timer_expired;
    int                 tx_msg_is_fetched;
    int                 rx_msg_is_coming;

	int					evt_que_head;
	int					evt_que_tail;	
	struct evt_msg	    ind_evt_que[EV_QUE_MAX];

    //statistics
    int                 tx_command_frames;
    int                 tx_done_command_frames;
    int                 tx_retransmit_counts;
    int                 tx_words;
    int                 tx_interupt_timeouts;
    int                 tx_stop_by_signals;
    int                 rx_status_frames;
	unsigned long		rx_status_time;
    int                 rx_words;
    int                 rx_response_timeouts;
    int                 rx_interupt_timeouts;
    int                 rx_stop_by_signals;
    int                 rx_not_1st_word_errors;
    int                 rx_1st_byte_errors;
    int                 rx_cmdid_not_match_errors;
    int                 rx_reset_by_sync_bit_errors;
    int                 rx_checksum_errors; 	
};
#endif

#endif // _RTL_NFBI_H_

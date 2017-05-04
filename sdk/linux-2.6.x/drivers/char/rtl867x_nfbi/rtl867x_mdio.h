/*
  *	RTL867x header file of mdio driver
  *
  *	Copyright (C)2012, Realtek Semiconductor Corp. All rights reserved.
  *
  *	$Id: rtl867x_mdio.h,v 1.1 2012/07/09 07:22:43 jiunming Exp $
  */

/*================================================================*/

#ifndef INCLUDE_RTL_MDIO_H
#define INCLUDE_RTL_MDIO_H

/*================================================================*/
/* Constant Definitions */
#define DRIVER_NAME			"rtl_mdio"
#define DRIVER_VER			"0.2"
#define DRIVER_MAJOR		14

#define MDIO_USERHDR_SIZE	12
#define MDIO_USERDATA_SIZE	4096
#define MDIO_CHKSUM_SIZE	1
#define MDIO_BUFSIZE		(MDIO_USERHDR_SIZE+MDIO_USERDATA_SIZE) //516 //2 + 2 + 2*255 + 2
#define MDIO_MAX_BUFSIZE	(MDIO_BUFSIZE+MDIO_CHKSUM_SIZE)
#define EV_QUE_MAX			16


#ifndef __KERNEL__
#define BIT(x)				(1 << (x))
#endif /*__KERNEL__*/


// RTL867x NFBI Register offset
#define NFBI_BASE				0xb8019000			// NFBI base address
#define REG_RCR					(0x00+NFBI_BASE)	// Receive Command Register
#define REG_SSR					(0x04+NFBI_BASE)	// Send Status Register
#define REG_SYSCR				(0x08+NFBI_BASE)	// System Control Register
#define REG_SYSSR				(0x0C+NFBI_BASE)	// System Status Register
#define REG_IMR					(0x10+NFBI_BASE)	// Interrupt Mask Register
#define REG_ISR 				(0x14+NFBI_BASE)	// Interrupt Status Register
#define REG_UACR				(0x20+NFBI_BASE)	// Utopia Access Control Register
#define REG_UASR				(0x24+NFBI_BASE)	// Utopia Access Status Register
#define REG_FDCR				(0x28+NFBI_BASE)	// Fast Delay Compensation Register
#define REG_SDCR 			    (0x2C+NFBI_BASE) 	// Slow Delay Compensation Register
#define REG_NFBIRR 			    (0x30+NFBI_BASE)	// NFBI Reset Control Register
#define REG_WDOGCNT 			(0x40+NFBI_BASE)	// NFBI Watch Dog Counter Register


// bitmask definition for ISR
#define IP_ENUTPPHY			BIT(12)		// Enable Utopia PHY
#define IP_UACR_CHANGE		BIT(11)		// UACR Change
#define IP_FDCR_CHANGE		BIT(10)		// FDCR Change
#define IP_SDCR_CHANGE		BIT(9)		// SDCR Change
#define IP_KernelcodeReady	BIT(8)		// KernelcodeReady
//#define IP_CUSTOM8			BIT(8)		// Customized used 8
#define IP_CUSTOM7			BIT(7)		// Customized used 7
#define IP_CUSTOM6			BIT(6)		// Customized used 6
#define IP_CUSTOM5			BIT(5)		// Customized used 5
#define IP_CUSTOM4			BIT(4)		// Customized used 4
#define IP_CUSTOM3			BIT(3)		// Customized used 3
#define IP_MSGFETCH			BIT(2)		// Previous msg has been fetched
#define IP_NEWMSG			BIT(1)		// New msg has come
#define IP_RSVD0			BIT(0)		// Reserved

// bitmask definition for SYSCR
#define CR_ENUTPPHY			BIT(12)		// Enable Utopia PHY interface
#define CR_KernelcodeReady	BIT(8)		// KernelcodeReady
//#define CR_CUSTOM8		BIT(8)		// Customized used 8
#define CR_CUSTOM7			BIT(7)		// Customized used 7
#define CR_CUSTOM6			BIT(6)		// Customized used 6
#define CR_CUSTOM5			BIT(5)		// Customized used 5
#define CR_CUSTOM4			BIT(4)		// Customized used 4
#define CR_CUSTOM3			BIT(3)		// Customized used 3
#define CR_CUSTOM2			BIT(2)		// Customized used 2
#define CR_CUSTOM1			BIT(1)		// Customized used 1
#define CR_CUSTOM0			BIT(0)		// Customized used 0

// bitmask definition for SYSSR
#define SR_DSLLink			BIT(12)
#define SR_AllSoftwareReady	BIT(10)
#define SR_FIRMWARE9		BIT(9)
#define SR_FIRMWARE8		BIT(8)
#define SR_FIRMWARE7		BIT(7)
#define SR_FIRMWARE6		BIT(6)
#define SR_BootcodeReady	BIT(5)
//#define SR_FIRMWARE5		BIT(5)
#define SR_FIRMWARE4		BIT(4)
#define SR_FIRMWARE3		BIT(3)
#define SR_FIRMWARE2		BIT(2)
#define SR_FIRMWARE1		BIT(1)
#define SR_FIRMWARE0		BIT(0)

// rx cmd id bitmask
#define FIRST_CMD_MASK		BIT(15)

// All received interrupt mask
#define NEEDED_IRQ_MASK_NO_MSG	(IP_SDCR_CHANGE|IP_FDCR_CHANGE|IP_UACR_CHANGE|IP_ENUTPPHY|IP_KernelcodeReady)
#define NEEDED_IRQ_MASK			(IP_NEWMSG|IP_MSGFETCH|NEEDED_IRQ_MASK_NO_MSG)

// cmd id of ioctl 
#define MDIO_IOC_MAGIC						'k'
#define MDIO_IOCTL_SET_HOST_PID				_IOW(MDIO_IOC_MAGIC,  0, int)
#define MDIO_IOCTL_SET_CMD_TIMEOUT		    _IOW(MDIO_IOC_MAGIC,  1, char)
#define MDIO_IOCTL_SET_PHY_POLL_TIME	    _IOW(MDIO_IOC_MAGIC,  2, char)
#define MDIO_IOCTL_READ_MEM					_IOWR(MDIO_IOC_MAGIC, 3, int)
#define MDIO_IOCTL_WRITE_MEM				_IOW(MDIO_IOC_MAGIC,  4, struct mdio_mem32_param)
#define MDIO_IOCTL_READ_SCR					_IOR(MDIO_IOC_MAGIC,  5, int)
#define MDIO_IOCTL_PRIV_CMD                 _IOW(MDIO_IOC_MAGIC,  6, int)
#define MDIO_IOCTL_MAXNR 					6


/*================================================================*/

// rx cmd state
enum { 
	STATE_RX_INIT, 
	//STATE_RX_WAIT_LEN, 
	STATE_RX_WAIT_DATA, 
	STATE_RX_WAIT_DAEMON 
};

// tx status state
enum { 
	STATE_TX_INIT, 
	STATE_TX_IN_PROGRESS 
};

// indication event id
enum {
	IND_CMD_EV,
	IND_SYSCTL_EV
};

/*================================================================*/
/* Structure Definition */

struct buf_ar {
	int len;
	unsigned char buf[MDIO_MAX_BUFSIZE];	
};

struct evt_msg {
	int id;			// event id
	int len;		// length in buf
	unsigned char buf[MDIO_MAX_BUFSIZE];	
};

struct mdio_mem32_param {
	unsigned long addr;
	unsigned long val;
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
struct mdio_priv 
{	
	spinlock_t      	reglock;

	struct evt_msg		ind_evt_que[EV_QUE_MAX];
	int					evt_que_head; 
	int					evt_que_tail;

	struct buf_ar		data_out;
	struct buf_ar		data_in;
	int					cmd_timeout;// in 10ms
	int					rx_cmd_state;
	unsigned long		rx_cmd_time;
	int					tx_status_state;
	int					tx_status_transmitting_len;
	unsigned int		tx_flag;
	unsigned int		rx_flag;


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	int 				host_pid;	// pid of host-daemon
#else
	struct pid*			host_pid;
#endif


	int					poll_timer_up;
	int					phy_reg_poll_time;	// in 10ms
	struct timer_list	reg_poll_timer;
	unsigned long		reg_UACR_write;
	unsigned long		reg_FDCR_write;
	unsigned long		reg_SDCR_write;

};
#endif /*__KERNEL__*/

#endif // INCLUDE_RTL_MDIO_H

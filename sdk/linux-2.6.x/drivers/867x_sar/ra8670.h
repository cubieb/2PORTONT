

#ifndef SAR_RA8670_H
#define SAR_RA8670_H

#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/atm.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/atmdev.h>

#ifdef CONFIG_RTL8672
#include <linux/autoconf.h>
//#include <platform.h>
/*linux-2.6.19*/
#include <bspchip.h>
#ifdef CONFIG_SAR_SHARE_PRIV_SKB_WITH_ETH
#define MBUF_LEN	1600
#define CROSS_LAN_MBUF_LEN		(MBUF_LEN+16)
extern struct sk_buff *dev_alloc_skb_priv_eth(unsigned int size);
#endif
#endif

/*--------------------------------
	Global Compile Definitions
---------------------------------*/

#define	ATM_OAM
#define	ATM_OAM_TEST
#define ENA_OAM_CH
#undef	SAR_CRC_GEN
#undef	LoopBack_Test	/* Enable Loopback Mode */
#undef	FAST_ROUTE_Test
//extern int nic_txx (struct sk_buff *skb);
#undef	Performance_Tune

//#define FAST_ROUTE_Test
//#undef	FAST_ROUTE_Test
//#define CONFIG_FLOW_CTRL
#undef CONFIG_FLOW_CTRL
// Bottom-Half use
#define SAR_BH   20        // 0-17 were reserved. max=32

/*--------------------------------
	Global Variable and Definitions
---------------------------------*/


#define	uint8	u8
#define	uint16	u16
#define	uint32	u32
#define	int8		signed char
#define	int16	signed short
#define	int32	signed long

#define	UINT8	uint8	
#define	UINT16	uint16	
#define	UINT32	uint32	
#define	INT8	int8		
#define	INT16	int16	
#define	INT32	int32
#define	UCHAR	uint8
#define	BOOL	uint8




#define SUCCESS 	0
#define FAILED -1


#define 	VC_CREATED      1
#define 	VC_NOT_CREATED  0


#define	Desc_Align		256
#define	Uncache_Mask	0xA0000000

#define 	CRC10_POLY			0x633		/* Generation polynomial of CRC10 */
#define 	CRC32_POLY			0x04c11db7L	/* Generation polynomial of CRC328 */
#define 	VER_CRC32_POLY	0xc704dd7bL	/* Verification polynomial of CRC10 */

#define	SAR_TX_Buffer_Size	1600
#define	SAR_RX_Buffer_Size	1600

#define	SAR_OAM_Buffer_Size	64

#define 	SAR_RX_DESC_NUM	64
#define 	SAR_TX_DESC_NUM	64
// threshold to wake up tx queue
#define		SAR_TX_THRESHOLD 5

/* each VC reserved fixed 64 BDs, the next one is located after 64 BDS */
#define	SAR_RX_RING_SIZE			SAR_RX_DESC_NUM	
#define	SAR_TX_RING_SIZE			SAR_TX_DESC_NUM	

//12/30/05' hrchen, for PVC desc number setting
#define	SAR_RX_NUM				128
//1/13/06' hrchen, for desc number re-assign
#define	SAR_RX_DESC_LOW_LIMIT	8		//min desc number for a PVC
#define	SAR_RX_DESC_HI_LIMIT	62		//max desc number for a PVC, never >=64
//rx desc formation flag
#define FORMATION_NO_CHANGE			0
#define FORMATION_CHANGE			1

#define	SAR_MAX_Process_DESC	128

#define	CRC32_Size			4
#define	Max_Packet_Size		1518
#define	Min_Packet_Size		64

#define	RDATHR		7
//#define	RDATimer	20		//delay = RDATimer*500us
#define	RDATimer	2		//delay = RDATimer*500us


#define	Enable_VC_CNT	16

extern UCHAR	*FrameHeader_1483[4];
extern UCHAR	*FrameHeader_1577[2];
extern int8	FrameHeaderSize_1483[4];
extern int8	FrameHeaderSize_1577[2];
//1/1/06' hrchen, desc # of a PVC is depended on total PVC numbers
extern int per_vc_desc_number;

// for debug
extern int debug_obaal5;
extern int debug_obcell;
extern int debug_num2print;

#ifdef FAST_ROUTE_Test
extern int fast_frame_up;
#endif
#ifdef CONFIG_FLOW_CTRL
extern unsigned int FCFlag;
extern unsigned int FCEnable;
extern unsigned int FCCtrl;
extern unsigned int FC_low;
extern unsigned int FC_high;
#endif

#define	OAM_CH_NO		Enable_VC_CNT
#ifdef CONFIG_RTL8672
#define NUMBER_OF_OAM_CH 2
#else
#define NUMBER_OF_OAM_CH 1
#endif
#define HW_OAM_CH_NO	16		//Out band is at index 16 not 8

#define	Max_Header_Size	16

//for remote management 0/16 test
//#define RM_TEST
#define REMOTE_MANAGEMENT_ENABLE
#define RTATM_DEV(d) ((sar_private *) (d)->dev_data)
#define RTATM_VCC(d) ((sar_atm_vcc *) (d)->dev_data)

/* has been moved to include/linux/atm.h */
#if 0
enum RFC_MODE{	
	RFC1483_BRIDGED, 	/* Ethernet over ATM (bridged) */
	RFC1483_ROUTED,	/* Ethernet over ATM (routed IP) */
	RFC1577,			/* IP over ATM */
	RFC2364,			/* PPP over ATM */
	RFC2516			/* PPP over Ethernet */
};

enum FRAMING_MODE{
	LLC_SNAP,
	VC_MUX
};
#endif

/*--------------------------------
		Register Address
---------------------------------*/
#define 	SAR_ADDR		0xB8300000

/* Channel */
#define	TV_Ctrl_Addr	(SAR_ADDR+0x00000000)
#define	TV_FDP_Addr	(SAR_ADDR+0x00000004)
#define	TV_SCR_Addr	(SAR_ADDR+0x00000008)
#define	TV_HDR_Addr	(SAR_ADDR+0x0000000C)

#ifdef CONFIG_RTL8672_ATM_QoS
#define TV_QoS_PCR_Addr		(SAR_ADDR+0x00000100)
#define TV_QoS_SCR_Addr		(SAR_ADDR+0x00000108)
#define TV_QoS_SCR1_Addr	(SAR_ADDR+0x0000010c)
#endif	//CONFIG_RTL8672_ATM_QoS
/* 8672 migration
#define	TO_Ctrl_Addr	(SAR_ADDR+0x00000100)
#define	TO_FDP_Addr	(SAR_ADDR+0x00000104)
*/
#define	TO_Ctrl_Addr	(SAR_ADDR+0x00000200)
#define	TO_FDP_Addr	(SAR_ADDR+0x00000204)

#define	RV_Ctrl_Addr	(SAR_ADDR+0x00001000)
#define	RV_FDP_Addr	(SAR_ADDR+0x00001004)
#define	RV_CKS_Addr	(SAR_ADDR+0x00001008)
#define	RV_HDR_Addr	(SAR_ADDR+0x0000100C)

#define	RO_Ctrl_Addr	(SAR_ADDR+0x00001100)
#define	RO_FDP_Addr	(SAR_ADDR+0x00001104)

#define	RO1_Ctrl_Addr	(SAR_ADDR+0x00001110)	//8672 new function
#define	RO1_FDP_Addr	(SAR_ADDR+0x00001114)	//8672 new function

/* Control */
#define	SAR_CNFG_Addr	 (SAR_ADDR + 0x00002000)
#define	SAR_STS_Addr	 (SAR_ADDR + 0x00002004)
#define	SAR_TBEI_Addr	 (SAR_ADDR + 0x00002010)
#define	SAR_TDFI_Addr	 (SAR_ADDR + 0x00002014)
#define	SAR_RDAI_Addr	 (SAR_ADDR + 0x00002018)
#define	SAR_RBFI_Addr	 (SAR_ADDR + 0x0000201C)
#define	SAR_RTOI_Addr	 (SAR_ADDR + 0x0000200C)

#ifdef CONFIG_RTL8681
/* Bonding */
#define SAR_DUAL_MAP	(SAR_ADDR + 0x00002008)
#define SAR_BOND_CFG	(SAR_ADDR + 0x00002100)
#define SAR_BOND_RSTS	(SAR_ADDR + 0x00002104)
#define SAR_DUMMY_CFG	(SAR_ADDR + 0x00002108)
#define SAR_LINK_STS		(SAR_ADDR + 0x0000210c)
#define SAR_BOND_WFQ0	(SAR_ADDR + 0x00002110)
#define SAR_BOND_WFQ1	(SAR_ADDR + 0x00002114)
#endif

/* Debug */
#define	Test_Reg_0		0xB8303000
#define	Test_Reg_1		0xB8303010

/*--------------------------------
	Channel Control Definition
---------------------------------*/

#define	CHEN			(1<<31)
#define QosPriority	(1<<28)
#define HpCHTx		(1<<27)
#define	SSL_Offset		19

#define	QoS_UBR			0
#define	QoS_CBR			1
#define	QoS_nrtVBR		2
#define	QoS_rtVBR		3

#define	TBEC			(1<<25)
#define	CDOI_Offset		18
#define	CDOI_Mask		0x00FC0000
#define 	SCR_Offset		16
#define	MBS_Offset		8
#define	GFC_Offset		28
#define	VPI_Offset		20
#define	VCI_Offset		4
	
#define	AAL5			(1<<30)
#define	ATRLREN		(1<<29)
#define	CFD				(1<<25)
#define	RBFC			(1<<24)

#define 	FFCN_Offset		16

#define L2Encap_Offset	13

#define	L2Encap_VCM	0
#define	L2Encap_LLC	1
	
#define	SARhdr_Offset	10

#define	Routed_mode 	0
#define	Bridged_mode 	1
#define	MER_mode	2
#define	PPPoE_mode 	3
#define	PPPoA_mode 	4

#define	WIEN			(1<<8)
#define	IEPEN_Offset	6

#define	DNOAMEN		(1<<30)

/*--------------------------------
	SAR Control Definition
---------------------------------*/
#define	SAREN				(1<<31)
#define	UFLB				(1<<30)
#define	Is8672_10_Offset	28	// 8672 new QoS scheme
#define	RDATHR_Offset		24
#define 	IS8671G_1			(1<<23)	// 1:enable cbr>rtVbr>nrtVbr>ubr
#define 	IS8671G_0			(1<<22)	// 1: no starvation HP CH
#define	RDATIMER_Offset	16
#define	TBEOMSK			(1<<14)
#define	TDFOMSK			(1<<13)
//#define	QCLKSEL				(1<<12)
#define UTO_CLK_MASK			(1<<7)
#define	QCLKOFFSET_Offset	8
#define	IERBF				((1<<6) | (1 << 14))
#define	IERDA				((1<<5) | (1 << 13))
#define	IERTO				((1<<4) | (1 << 12))
#define	IETBE				((1<<3) | (1 << 11))
#define	IETDF				((1<<2) | (1 << 10))
#define QoS_Starvation      (1<<12)


#define	RTO					(1<<4)
#define	RBF					(1<<3)
#define	RDA					(1<<2)
#define	TDF					(1<<1)
#define	TBE					(1<<0)


/*--------------------------------
	uClinux Related Definitions
---------------------------------*/

/* Time in jiffies before concluding the transmitter is hung. */
#define TX_TIMEOUT		(6*HZ)


/*--------------------------------
	OAM Related Definition
---------------------------------*/

/* has been moved to include/linux/atm.h
#ifdef ATM_OAM
typedef struct tATMOAMLBReq {
	unsigned long 	vpi;		// vpi (input)
	unsigned long 	vci;		// vci (input)
	unsigned char 	Scope;		// segment-to-segment(0)/end-to-end(1) (input)
	unsigned char 	Channel;	// ATM0/ATM1 (input)
	unsigned char 	LocID[16];	// location ID (input)
	unsigned char 	SrcID[16];	// source ID (input)
	unsigned long	Tag;		// message tag (output for start, input for stop)
} ATMOAMLBReq;

#endif
*/

/*--------------------------------
	Descriptor Structures 
---------------------------------*/
#define	OWN_32			(1<<31)

#define	OWN			(1<<15)
#define	EOR				(1<<14)
#define	FS				(1<<13)
#define	LS				(1<<12)
#define	ATMPORT_FAST	(1<<11)
#define	ATMPORT_SLOW	0
#define	TRLREN			(1<<10)
#define	ETHNT_OFFSET	4
//10/26/05' hrchen, fix append Ethernet CRC to cause pkt oversize bug
#define	ETHNT_OFFSET_MSK	0x003F
	#define	No_Offset		0x3F
	#define	Zero_Offset		0x00
#define	PTI_Offset_Tx	2
#define	CLP				(1<<0)
#define	LEN_Mask		0x0FFF
#define	CRC10EN		(1<<10)
#define	CRC32Err		(1<<10)
#define	LENErr			(1<<9)
#define	IPCErr			(1<<8)
#define 	PPI_Offset		6
#define	FRGI			(1<<5)
#define	WII				(1<<4)
#define	PTI_Offset_Rx	1
#define	isOAM			(1<<8)
#define	OAMType_Offset	6		

#define TAGC		(1<<1)	//8672 VLAN tag control bit

typedef struct packet_desc_struct {
  uint32 offset0;
  uint32 offset4;
  uint32 offset8;
  uint32 offsetC;
}packet_desc_t;

typedef struct {
	UINT16	CMD;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT16	TRLR;
	UINT16	RSVD0;
	UINT32	RSVD1;
} TV_CMD_DESC;

typedef struct {
	UINT16	STS;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT32	RSVD0;
	UINT32	RSVD1;
} TV_STS_DESC;

typedef struct {
	UINT16	CMD;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT32	RSVD0;
	UINT32	RSVD1;
} TO_CMD_DESC;

typedef struct {
	UINT16	STS;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT32	RSVD0;
	UINT32	RSVD1;
} TO_STS_DESC;

typedef struct {
	UINT16	CMD;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT32	RSVD0;
	UINT32	RSVD1;
	UINT32	RSVD2;	// 8672 migration
} RV_CMD_DESC;


typedef struct {
	UINT16	STS;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT16	TRLR;
	UINT16 	BPC_LENGTH;
	UINT16	RSVD;
	UINT16	TUCACC;
	UINT32	RSVD2;	//8672 migration
} RV_STS_DESC;

typedef struct {
	UINT16	CMD;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT32	RSVD0;
	UINT32	RSVD1;
	UINT32	RSVD2;	// 8672 migration
} RO_CMD_DESC;

typedef struct {
	UINT16	STS;
	UINT16	LEN;
	UINT32	START_ADDR;
	UINT16	BCC;
	UINT16	RSVD0;
	UINT32	RSVD1;
	UINT32	RSVD2;	// 8672 migration
} RO_STS_DESC;


/*--------------------------------
		VC Structures
---------------------------------*/
typedef struct {
	UINT32	CtlSts;
	UINT32	FDP;
	UINT32	SCR;
	UINT32	HDR;
	UINT32	CtlSts_Addr;
	UINT32	FDP_Addr;
	UINT32	SCR_Addr;
#ifdef CONFIG_RTL8672_ATM_QoS
	UINT32	QoS_PCR_Addr;
	UINT32	QoS_SCR_Addr;
	UINT32	QoS_SCR1_Addr;
#endif	//RTL8672 QoS	
	UINT32	HDR_Addr;
	UINT32	SegmentCRC;
	UINT16	buffer_size;
	INT8		Ether_Offset_Value;
	UINT8	desc_pf;		/* Free Buffer pointer */
	UINT8	desc_pc;		/* SAR CDOI pointer */
	UINT8	desc_pw;		/* Software Write Buffer Pointer */
} TV_Channel;

typedef struct {
	UINT32	CtlSts;
	UINT32	FDP;
	UINT32	CKS;
	UINT32	HDR;
	UINT32	CtlSts_Addr;
	UINT32	FDP_Addr;
	UINT32	CKS_Addr;
	UINT32	HDR_Addr;
	UINT32	SegmentCRC;
	UINT32	buffer_size;
	UINT8	desc_pr;		/* Software Read Buffer Pointer */
	UINT8	desc_pc;		/* SAR CDOI pointer */
	UINT8	desc_pa;		/* Allocate Buffer Pointer */
} RV_Channel;

/*--------------------------------
		Statistics
---------------------------------*/
#if 0 // moved to linux/atm.h
typedef struct ch_stat{

	/* RX program flow related */
	INT32	rcv_cnt;			/* count of Receive Functiont calls */
	INT32	rcv_ok;			/* packet return count */
	
	/* RX data statistics related */
	INT32	rx_desc_cnt;	/* count of descriptors that receive Functiont walks through */
	INT32	rx_byte_cnt;	/* received byte count */
	INT32	rx_pkt_cnt;
	INT32	rx_pkt_fail;

	INT32	rx_FS_cnt;		/* count of FS desc */
	INT32	rx_LS_cnt;		/* count of LS desc */
	INT32	rx_desc_ok_cnt;	/* rx ok descriptors */
	INT32 	rx_oam_count;	/* rx oam cell count */
	INT32	rx_buf_alloc;	/* rx buffer allocated */
	INT32	rx_buf_lack;		/* lack of rx buffer */
	INT32	rx_desc_fail;	/* rx desc errors */
	INT32	rx_crc_error;	/* rx AAL5 CRC error count */
	INT32	rx_lenb_error;	/* rx packet length too large */
	INT32	rx_lens_error;	/* rx packet length too small */

	INT32	rx_netif_cnt;	/* count for netif calls */

	/* TX program flow related */
	INT32	send_cnt;		/* count of Send function calls */
	INT32	send_ok;		/* count of Send function ok */
	INT32	send_fail;		/* count of Send function errors */
	INT32	send_desc_full;	/* descriptors full */
	INT32	send_desc_lack;	/* not enough free descriptors */

	/* TX data statistics related */
	INT32	tx_desc_ok_cnt;	/* tx ok descriptors (successfully transmitted) */
	INT32	tx_buf_free;		/* tx buffer freed */
	INT32	tx_pkt_ok_cnt;	/* tx ok packets (successfully transmitted)*/
	INT32	tx_pkt_fail_cnt;	/* tx fail packets (send into descritprtor ring failed) */
	INT32	tx_byte_cnt;		/* send byte count (send into descriptor ring) */
	
}ch_stat;

typedef struct Traffic_Manage{
	UINT32	tick_now;
	INT32	cell_cnt;	
	INT8		Type;
	UINT16	PCR;
	UINT16	SCR;
	UINT8	MBS;
	UINT8	CRD;
	UINT32	CDVT;
}Traffic_Manage;
#endif



typedef struct 
{	
	int8				ch_no;
	uint8			vpi;
	uint16			vci;
	int				rfc;
	int				framing;
	int				created;
	int				loopback;
	uint8			MAC[6];
	Traffic_Manage	QoS;
	ch_stat			stat;
	int 				br;

	TV_Channel	TV;
	RV_Channel	RV;

	BOOL		TBE_Flag;
		
	UINT32		rx_buf[SAR_RX_DESC_NUM];		/* 64 Descriptors per channel  to record corresponding rx skb*/
	UINT32		tx_buf[SAR_TX_DESC_NUM];		/* 64 Descriptors per channel  to record corresponding tx skb*/
//	UINT32		skb_pool[64];	/* 64 skb pool for rx descriptors */
//	UINT8		skb_pool_put;
//	UINT8		skb_pool_get;

	/* statistics */
	INT32		RBF_cnt;		
	INT32		TBE_cnt;
	INT32		TDF_cnt;
	INT32		RDA_cnt;

	TV_CMD_DESC	*TV_Desc;	
	RV_STS_DESC	*RV_Desc;	

	TO_CMD_DESC	*TO_Desc;	
	RO_STS_DESC	*RO_Desc;	

	/* for Linux ATM */
	void 	*dev_data;
	struct net_device *dev;
	
	/* for ATM QoS interpolate*/
	int QoSinterpolateEnable;		//1 PCR interpolate enable, 2 SCR interpolate enable
    //PCR&SCR
    #define UPPER_PCR 0
    #define LOWER_PCR 1
    #define UPPER_SCR 2
    #define LOWER_SCR 3
	int creditCR;
	int numberCR;
	int hwCR[4];
	int hwCR_low[4];
	int hwCRtickData[4];
	int hwCRtick[4];
	int hwCellPerSecond;
	int hwPRIO;
	int hwTxCellCount;
	
	/* for ATM QoS Tx Credit */
	#define QoS_CREDIT_HI	48
	#define QoS_CREDIT_MID	32
	#define QoS_CREDIT_LOW	32
	int creditQoSTx;
}sar_atm_vcc;

#ifdef CONFIG_RTL8672_ATM_QoS
/*
 *	 The Brief of New RTL8672 QoS Scheme 
 * ----------------------------------
 *
 * 	pace_shot = PCR (cell/s) * PCR_Counter
 * 	pace_shot = QoS_CLK * PCR_Period
 * 	QoS_CLK*PCR_Period = PCR * PCR_Counter;
 *
 * Ex:
 *	128.205k * 1000 (period) = 1200 (PCR) * PCR_Counter
 *	PCR_Counter = 106837 = 0x0001A155;
 *
 *  p.s. SCR_Counter also use the same scheme
 */
#define QoS_CLK		(128205 * BSP_MHZ/200)

#define PCR_Period	1000  /* PCR_Counter = QoS_CLK*PCR_Period/PCR */	
#define SCR_Period	1000   /* SCR_Counter = QoS_CLK*SCR_Period/SCR */

#endif
/*--------------------------------
	IO Control Parameters
---------------------------------*/

/* IOCTLs */
/* moved to linux/atm.h
struct SAR_IOCTL_CFG {
	int8				ch_no;
	uint8			vpi;					//vpi number
	uint16			vci;					//vci number
	int				rfc;
	int				framing;
	int				created;
	int				loopback;	
	uint8			MAC[6];
	Traffic_Manage	QoS;
	ch_stat			stat;
};

// ioctl command called by system & user space applications
#define SAR_GET_MODEMSTATE		SIOCDEVPRIVATE
#define SAR_GET_STATS			(SIOCDEVPRIVATE+1)
#define SAR_ENABLE			(SIOCDEVPRIVATE+2)
#define SAR_DISABLE			(SIOCDEVPRIVATE+3)
#define SAR_GET_CONFIG 			(SIOCDEVPRIVATE+4)
#define SAR_SET_CONFIG 			(SIOCDEVPRIVATE+5)
#define SAR_ATM_OAM_SET_ID		(SIOCDEVPRIVATE+6)
#define SAR_ATM_OAM_START		(SIOCDEVPRIVATE+7)
#define SAR_ATM_OAM_STOP		(SIOCDEVPRIVATE+8)
#define SAR_ATM_OAM_STATUS		(SIOCDEVPRIVATE+9)
#define SAR_ATM_OAM_STATUS_FE		(SIOCDEVPRIVATE+10)
#define SAR_ATM_OAM_RPT_LB		(SIOCDEVPRIVATE+11)
#define SAR_ATM_OAM_STOP_LB		(SIOCDEVPRIVATE+12)
#define SAR_CREATE_VC			(SIOCDEVPRIVATE+13)
#define SAR_DELETE_VC			(SIOCDEVPRIVATE+14)
#define SAR_ENABLE_UTOPIA		(SIOCDEVPRIVATE+15)
#define	SAR_UTOPIA_FAST			(SIOCDEVPRIVATE+16)
#define	SAR_UTOPIA_SLOW			(SIOCDEVPRIVATE+17)
#define	SAR_SETMAC			(SIOCDEVPRIVATE+18)
#define	SAR_EnableLOOPBACK			(SIOCDEVPRIVATE+19)
#define	SAR_DisableLOOPBACK			(SIOCDEVPRIVATE+20)
#define	SAR_SET_PVC_NUMBER			(SIOCDEVPRIVATE+21)
#define	SAR_READ_MEM				(SIOCDEVPRIVATE+22)
#define	SAR_WRITE_MEM				(SIOCDEVPRIVATE+23)
*/

/*--------------------------------
	SAR END Device Definition
---------------------------------*/
typedef struct rtl_sar_private
{	
	/* uClinux Related */
	void		 	*dev;
	spinlock_t		lock;
	int				irq;
	//struct tq_struct bh;
	struct tasklet_struct tasklets;
	
	/* Main Sar structure */
	UINT8		ProcessRcv;
	int			unit;			/* unit number */
	UINT32      	flags;			/* Our local flags */
	UCHAR		enetAddr[6];		/* ethernet address */
	UINT16		pktlen;

	char*		pTVDescBuf;        /* ptr to device descriptor memory (allocated) */
	char*		pTODescBuf;        /* ptr to device descriptor memory (allocated) */
	char*		pRVDescBuf;        /* ptr to device descriptor memory (allocated) */
	char*		pRODescBuf;        /* ptr to device descriptor memory (allocated) */

	TV_CMD_DESC	*TVDesc;	/* ptr to device descriptor memory (after masked, actually used) */
	RV_STS_DESC	*RVDesc;	/* ptr to device descriptor memory (after masked, actually used) */
	TO_CMD_DESC	*TODesc;	/* ptr to device descriptor memory (after masked, actually used) */	
	RO_STS_DESC	*RODesc;	/* ptr to device descriptor memory (after masked, actually used) */
                                                 
	sar_atm_vcc		vcc[(Enable_VC_CNT+NUMBER_OF_OAM_CH)];

	UINT32	CNFG_Reg	;	
	UINT32	STS_Reg	;	
	UINT32	TDFI_Reg	;	
	UINT32	TBEI_Reg	;	
	UINT32	RTOI_Reg	;	
	UINT32	RDAI_Reg	;	
	UINT32	RBFI_Reg	;	
	
	UINT32	tx_channel_on;	/* indicate which tx channel is enabled */
	UINT32	rx_channel_on;	/* indicate which rx channel is enabled */
	UINT16	atmport;
	int				QoS_Test;
	int				QoS_Tx_Credit;  /* for ATM QoS Tx Credit*/

}sar_private;


extern sar_private	*sar_dev;


/*--------------------------------
		Macros
---------------------------------*/
#define 	sar_reg(offset)  (*(volatile UINT32 *)((UINT32)SAR_ADDR + offset))
#define 	reg(address)  (*(volatile UINT32 *)((UINT32)address))

#if 0//defined in platform.h 
#define	REG32(reg) (*(volatile uint32 *)(reg))
#define	REG8(reg) (*(volatile uint8 *)(reg))
#define	REG16(reg) (*(volatile uint16 *)(reg))
#endif


#define	DRV_ENTER	//{};//printk(KERN_DEBUG "---->Enter %s: %d  \n",__FUNCTION__,__LINE__)
#define	DRV_LEAVE	//{};//printk(KERN_DEBUG "<----Leave %s: %d  \n",__FUNCTION__,__LINE__)


/*--------------------------------
	Functions Declarations
---------------------------------*/
void Alloc_desc(sar_private *pDrvCtrl);
void Free_desc(sar_private *pDrvCtrl);
void Init_reg(sar_private *pDrvCtrl);
void SetCRCTbl(void );
void GenCRC10Table(void);
void Enable_Sachem_Loopback(void);
void Enable_Sachem_Utopia(void);
void Set_RDA(sar_private *cp, int8 RdaThr, int8 RdaTimer);

void Enable_IERBF(sar_private *cp);
void Disable_IERBF(sar_private *cp);
void Enable_IERDA(sar_private *cp);
void Disable_IERDA(sar_private *cp);
void Enable_IERTO(sar_private *cp);
void Disable_IERTO(sar_private *cp);
void Enable_IETBE(sar_private *cp);
void Disable_IETBE(sar_private *cp);
void Enable_IETDF(sar_private *cp);
void Disable_IETDF(sar_private *cp);

void Enable_SAR(sar_private *cp);
void Disable_SAR(sar_private *cp);
void Enable_QoS_Starvation(sar_private * cp);
void Disable_QoS_Starvation(sar_private * cp);
void Enable_LoopBack(sar_private *cp);
void Disable_LoopBack(sar_private *cp);
void Enable_ATRLREN(sar_private *cp, int8 i);
void Disable_ATRLREN(sar_private *cp, int8 i);
void Enable_tx_ch(sar_private *cp, int8 i);
void Enable_rx_ch(sar_private *cp, int8 i);
void Disable_tx_ch(sar_private *cp, int8 i);
void Disable_rx_ch(sar_private *cp, int8 i);
void Enable_AAL5(sar_private *cp, int8 i);
void Enable_Raw(sar_private *cp, int8 i);

void Clear_TBE(int8 i, int8 CDOI);
void Clear_RBF(sar_private *cp, int8 i, int8 CDOI);
void Cell_Forced_Dropped(sar_private *cp, int8 i);
void SetQoS(sar_private *cp, int8 i, int8 QoS);
void SetPCR(sar_private *cp, int8 i, uint16 PCR);
void	SetSCR(sar_private *cp, int8 i, uint16 SCR);
#ifdef CONFIG_RTL8672_ATM_QoS
void	SetMBS(sar_private *cp, int8 i, uint16 MBS);
#else
void	SetMBS(sar_private *cp, int8 i, uint8 MBS);
#endif
uint8 GetCRD(sar_private *cp, int8 i);


void	SetVpiVci(sar_private *cp, uint8 VPI, uint16 VCI, int8 ch_no);	
uint32 ReadD_ (uint32 address);
void WriteD_ (uint32 address,  uint32 data);
void Set1 (uint32 address,  int8 index );
void Reset1 (uint32 address,  int8 index );
uint8 Test1 (uint32 address,  int8 index );
int32 S2i(uint8 * str_P);
void Dump(uint32 Buffer, int32 size);
void Search(uint32 pattern);
void Idle(int32 period);
void Reset_Sar(void);

int GetTxCDOI(sar_private *pDrvCtrl, int ch_no);
int8 GetRxCDOI(sar_private *pDrvCtrl, int8 ch_no);

void Enable_Word_Insert(sar_private *cp, int8 i);
void Disable_Word_Insert(sar_private *cp, int8 i);

void Enable_Drop_NonOAM(sar_private *cp, int8 i);
void Disable_Drop_NonOAM(sar_private *cp, int8 i);
void Enable_QoS_Priority(sar_private *cp, int8 i);
void Disable_QoS_Priority(sar_private *cp, int8 i);
void Enable_HP_CHTx(sar_private *cp, int8 i);
void Disable_HP_CHTx(sar_private *cp, int8 i);
void Write_IP_Parser(sar_private *cp, int8 i, int8 en, int8 offset);
	
//void Set_QoS_Ext(sar_private *cp, int clk_offset);


BOOL ClearRxBuffer(sar_private *pDrvCtrl, int8 ch_no);
BOOL ClearTxBuffer(sar_private *pDrvCtrl, int8 ch_no);
void AllocVcBuff(sar_private *cp, int8 ch_no);
void CreateVC(sar_private *cp, struct SAR_IOCTL_CFG *cfg, int reset_stat_info);
void	DeleteVC(sar_private *cp, struct SAR_IOCTL_CFG *cfg);

void	flush_tx_desc(sar_private *cp);


void Set_SARhdr(sar_private * cp, int8 i, int8 mode);
void Set_L2Encap(sar_private * cp, int8 i, int8 encap);

#ifdef LoopBack_Test
INT16 Lying_Engine(uint8 *buf, INT16 len);
void exchange_mac(uint8 *buf);
#endif

struct sk_buff *get_skb_from_pool(INT8 ch_no);
void refill_skb_pool(INT8 ch_no);
void init_skb_pool(INT8 ch_no);
void pp_ifgroup_setup(void);

/////////////////////////////////////////////for OAM
#ifdef ATM_OAM
////////////////////////////////////////////////////////////////////////
//																		
//	Module Name			: $Workfile: AtmOAM.h $
//	Date					: $Date: 2012/08/09 14:44:27 $
//	Author				: Dick Tam, ITEX Network Group
//	$NoKeyword:$
//
//	Copyright 2003, Realtek Semiconductor Inc.
//
////////////////////////////////////////////////////////////////////////

#if !defined(__ATM_OAM_H__)
#define __ATM_OAM_H__

//#define RTOS							VXWORKS

//#if RTOS == VXWORKS
//#include <linux/time.h>
//#endif

#undef TRUE
#undef FALSE
#define TRUE							1
#define FALSE							0

#define OAM_CELL_SIZE						52

// OAM type & function type, 6th octet
#define	FAULT_AIS						0x10		// fault management
#define	FAULT_RDI						0x11
#define	FAULT_CC						0x14
#define   FAULT_LB						0x18
#define	PERFORMANCE_FPM						0x20		// performance management
#define	PERFORMANCE_BACKWARD_REPORT				0x21
#define   APS_GROUP_PROTECT					0x50		// APS coordinatin protocol
#define   APS_INDIVIDUAL_PROTECT				0x51
#define   ACT_DEACT_FPM_BR					0x80		// activation /deactivation
#define	ACT_DEACT_CC						0x81
#define	ACT_DEACT_FPM						0x82
#define  SYSTEM_MANAGEMENT					0xF0		// system management



/* OAM FM LB */
// OAM loopback timer interval
#define OAM_LB_WAIT_TIME					5000		// 5 sec
// OAM loopback state
#define FAULT_LB_IDLE						0
#define FAULT_LB_WAITING					1
#define FAULT_LB_STOP						2
// OAM loopback field size
#define OAM_LB_INDICATION_SIZE					1
#define OAM_LB_TAG_SIZE						4
#define OAM_LB_LLID_SIZE					16
#define OAM_LB_SRCID_SIZE					16
#define OAM_LB_UNUSE						8

// OAM cell format
#define OAM_FORMAT_H1						0													// OAM cell type and function
#define OAM_FORMAT_H2						1													// OAM cell type and function
#define OAM_FORMAT_H3						2													// OAM cell type and function
#define OAM_FORMAT_H4						3													// OAM cell type and function
#define OAM_FORMAT_TYPE						4													// OAM cell type and function

// OAM loopback cell format
#define OAM_FORMAT_LB_INDICATION			(OAM_FORMAT_TYPE+1)									// loopback indication
#define OAM_FORMAT_LB_TAG					(OAM_FORMAT_LB_INDICATION+OAM_LB_INDICATION_SIZE)	// correlation tag
#define OAM_FORMAT_LB_LLID				(OAM_FORMAT_LB_TAG+OAM_LB_TAG_SIZE)				// llid
#define OAM_FORMAT_LB_SID					(OAM_FORMAT_LB_LLID+OAM_LB_LLID_SIZE)				// source id
#define OAM_FORMAT_LB_UNUSE				(OAM_FORMAT_LB_SID+OAM_LB_SRCID_SIZE)				// source id

#define OAM_FORMAT_CRC10					52								// OAM cell CRC-16

#define LB_ALL_NODE_SIZE					6					// LB Location ID: 0 (all replies)
#define LB_TABLE_SIZE						6					// LB table size for TMN


/* OAM F5 */
typedef struct OAMF5Timer
{
//#if RTOS == VXWORKS
	struct timer_list timer;
//#endif

	void  *pOAM;						// OAMF5 struct pointer
	unsigned int OAMFunctionType;			// octect 6
	unsigned int OAMUserData;				// FM -- LB, index of OAMF5LBList
	
	unsigned short oneShot;				// one shot or periodic timer
	unsigned short occupied;				// occupied or empty
	unsigned long ulTimeout;				// timeout period, ms
} OAMF5Timer;

/* OAM FM LB */
typedef struct tOAMF5_LB_INFO {
	unsigned long	tag;								// LB correlation tag
	unsigned char	locationID[OAM_LB_LLID_SIZE];	// LB location ID
	unsigned char	channel;						// ATM0 (default) or ATM1
	char			scope;							// segment-to-segment (0) or end-to-end (1)
	unsigned long	count;							// Rx LB cell counters but it's generated by this CP.
	long			status;							// current status: idle or waiting
	long			all0_status;						// all 0 status for lccation ID: 0 only
	OAMF5Timer	timer;
	unsigned long	timestamp;						// timestamp of LB start
	unsigned long rtt;								// round-trip time of LB response
	unsigned char	cell[OAM_CELL_SIZE];
} OAMF5_LB_INFO;

typedef struct tOAMF5_LBRX_INFO {
	unsigned char	locationID[OAM_LB_LLID_SIZE];	// LB location ID
	unsigned long	sur_des_count;					// Rx LB cell counters but it's generated by other CP
	long			status;							// current status: empty or occupy
} OAMF5_LBRX_INFO;

typedef struct OAMF5 {
	unsigned short		VPI;												// vpi
	unsigned short		VCI;												// vci

	/* OAM FM -- LB */
	long			OAMF5lFaultLBState;										// loopback state
	unsigned long	OAMF5ulFaultLBTag;										// loopback correlation tag
	unsigned char	OAMF5ucCPID[OAM_LB_SRCID_SIZE];						// connection point ID

	// the first LB_ALL_NODE_SIZE is reserved for location ID: 0 (all replies)
	// the rest of list is used for other location ID
	// it's used by TMN on Tx only
	OAMF5_LB_INFO		OAMF5LBList[LB_ALL_NODE_SIZE+LB_TABLE_SIZE];

	// This is only for OAM RX LB used
	OAMF5_LBRX_INFO 	LBRXList[LB_ALL_NODE_SIZE+LB_TABLE_SIZE];

	unsigned long	OAMF5ulFaultLBTxGoodCell;			// transmit good loopback cells
	unsigned long	OAMF5ulFaultLBRxGoodCell;			// receive good loopback cells

} OAMF5;


/* OAM F4 */
#define OAMF4Timer OAMF5Timer
#define OAMF4_LB_INFO OAMF5_LB_INFO
#define OAMF4_LBRX_INFO OAMF5_LBRX_INFO

typedef struct OAMF4 {
	unsigned short		VPI;												// vpi
	unsigned short		VCI;												// vci

	/* OAM FM -- LB */
	long			OAMF4lFaultLBState;										// loopback state
	unsigned long	OAMF4ulFaultLBTag;										// loopback correlation tag
	unsigned char	OAMF4ucCPID[OAM_LB_SRCID_SIZE];						// connection point ID

	// the first LB_ALL_NODE_SIZE is reserved for location ID: 0 (all replies)
	// the rest of list is used for other location ID
	// it's used by TMN on Tx only
	OAMF4_LB_INFO		OAMF4LBList[LB_ALL_NODE_SIZE+LB_TABLE_SIZE];

	// This is only for OAM RX LB used
	OAMF4_LBRX_INFO 	LBRXList[LB_ALL_NODE_SIZE+LB_TABLE_SIZE];

	unsigned long	OAMF4ulFaultLBTxGoodCell;			// transmit good loopback cells
	unsigned long	OAMF4ulFaultLBRxGoodCell;			// receive good loopback cells

} OAMF4;

//structure for link speed
typedef struct {
 unsigned long upstreamRate;
 unsigned long downstreamRate;
} T_LinkSpeed;

/* OAM F5, global structure */
extern OAMF5 OAMF5_info;
extern OAMF4 OAMF4_info;

/* OAM F5 */
extern void OAMF5Init(unsigned short vpi, unsigned short vci, OAMF5 *pOAMF5);
extern int OAMF5SetVpiVci(unsigned short vpi, unsigned short vci, OAMF5 *pOAMF5);
extern int OAMF5TMNSetCPID(unsigned char ucLocationType, unsigned char *pCPID, OAMF5 *pOAMF5);
extern int OAMF5TMNTxLBStart(unsigned char Scope, unsigned char *pLocID, unsigned long *Tag, OAMF5 *pOAMF5);
extern int OAMF5TMNTxLBStop(unsigned long Tag, OAMF5 *pOAMF5);
extern void OAMRxF5Cell(unsigned char *pOamCell, OAMF5 *pOAMF5);

/* OAM F4 */
extern void OAMF4Init(unsigned short vpi, unsigned short vci, OAMF4 *pOAMF4);
extern int OAMF4SetVpiVci(unsigned short vpi, unsigned short vci, OAMF4 *pOAMF4);
extern int OAMF4TMNSetCPID(unsigned char ucLocationType, unsigned char *pCPID, OAMF4 *pOAMF4);
extern int OAMF4TMNTxLBStart(unsigned char Scope, unsigned char *pLocID, unsigned long *Tag, OAMF4 *pOAMF4);
extern int OAMF4TMNTxLBStop(unsigned long Tag, OAMF4 *pOAMF4);
extern void OAMRxF4Cell(unsigned char *pOamCell, OAMF4 *pOAMF4);

/* Remote Management 0/16 */
extern int OutBandAAL5Tx(unsigned short vpi, unsigned short vci, struct sk_buff *skb);
extern int OutBandAAL5Rx(struct atm_vcc *atm_vcc, char *bfr);


/* ATM OAM Interface */
/* has been moved to include/linux/atm.h

typedef struct tATMOAMLBID 
{
	unsigned long vpi;			// Near End vpi (input) 
	unsigned long vci;			// Near End vci (input) 
	unsigned char LocID[16];	// Near End connection point ID (input) 
} ATMOAMLBID;

typedef struct tATMOAMLBState 
{
	unsigned long vpi;				// Near End vpi (input) 
	unsigned long vci;				// Near End vci (input) 
	unsigned char LocID[6][16];		// location ID (output)
	unsigned long Tag;			// message tag (input)
	unsigned long count[6];		// statistic counter (output)
	unsigned long rtt[6];			// round-trip time (output)
	long	           status[6];			// state (output: waiting(1)/stop(2) )
}ATMOAMLBState;

typedef struct tATMOAMLBRXState
{
	unsigned long vpi;				// Near End vpi (input) 
	unsigned long vci;				// Near End vci (input) 
	unsigned char	LocID[6][16]; 	// location ID 
	unsigned long	count[6];        	// statistic counter (output)
	long		status[6];       		// state (output: empty (0) /occupy(1))
} ATMOAMLBRXState;
*/

typedef void (*VOIDFUNCPTR) (unsigned long);  /* ptr to function returning void, for timer dunction */

//for ATM QoS VBR Credit adjust
extern void adjust_ATM_QoS(sar_atm_vcc *vcc_dev);
extern int enable_port_mapping;
extern int enable_vlan_grouping;
extern sar_private	*sar_dev;
                                                                                                                                                                                                                                                                                                          
                                                                                                    
#ifdef __KERNEL__                                                                                   
                                                                                                    
#include <linux/net.h>	/* struct net_proto */                                                      
                                                                                                    
                                                                                                    
//void atmpvc_proto_init(struct net_proto *pro);                                                      
//void atmsvc_proto_init(struct net_proto *pro);                                                      
                                                                                                    
#endif /* __KERNEL__ */                                                                             
                                                                                                    

#endif

#endif

#endif // of SAR_RA8670_H

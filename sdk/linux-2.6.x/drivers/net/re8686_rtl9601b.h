/*	
 *	re8686_rtl9601b.h
*/
#ifndef _RE8686_RTL9601B_H_
#define _RE8686_RTL9601B_H_

#define ON 1
#define OFF 0
#define ADDR_OFFSET 0x0010


#define CONFIG_RG_JUMBO_FRAME 1
//#define RTL0371
//#define GMAC_FPGA

#define VPORT_CPU_TAG	0
#define VPORT_VLAN_TAG	1
#define ETH_WAN_PORT 3//in virtual view
#define SW_PORT_NUM 3
#define PON_CTRL_STREAM 0x20
#if 0
#ifdef CONFIG_RG_API_RLE0371
#define PON_PORT 3
#else
#define PON_PORT 4
#endif
#endif
#define APOLLO_PON_PORT 1
#define CPU_PORT 2

#define ETHBASE	0xB8012000	//internal phy

#define RTL_W32(reg, value)			(*(volatile u32*)(ETHBASE+reg)) = (u32)value
#define RTL_W16(reg, value)			(*(volatile u16*)(ETHBASE+reg)) = (u16)value
#define RTL_W8(reg, value)			(*(volatile u8*)(ETHBASE+reg)) = (u8)value
#define RTL_R32(reg)				(*(volatile u32*)(ETHBASE+reg))
#define RTL_R16(reg)				(*(volatile u16*)(ETHBASE+reg))
#define RTL_R8(reg)					(*(volatile u8*)(ETHBASE+reg))

//for alignment issue
#define READWD(addr) ((unsigned char *)addr)[0]<<24 | ((unsigned char *)addr)[1]<<16 | ((unsigned char *)addr)[2]<<8 | ((unsigned char *)addr)[3]
#define READHWD(addr) ((unsigned char *)addr)[0]<<8 | ((unsigned char *)addr)[1]

//#define RE8670_TX_RING_SIZE 64

#if defined(CONFIG_RTL_8812_SUPPORT)
#define TX_EACH_RING_SIZE       1024
#define RX_EACH_RING_SIZE       1024
#else
#ifdef CONFIG_RTL865X_ETH_PRIV_SKB_ADV
#define TX_EACH_RING_SIZE       512
#define RX_EACH_RING_SIZE       512
#else
#define TX_EACH_RING_SIZE       256
#define RX_EACH_RING_SIZE       256
#endif
#endif


//plz use the order of 2, set size 0 to no use ring

#define RE8670_RX_RING1_SIZE RX_EACH_RING_SIZE
#define RE8670_RX_RING2_SIZE 256
#define RE8670_RX_RING3_SIZE 256
#ifdef CONFIG_DUALBAND_CONCURRENT
#define RE8670_RX_RING4_SIZE 0
#define RE8670_RX_RING5_SIZE 0
#define RE8670_RX_RING6_SIZE 0
#else
#define RE8670_RX_RING4_SIZE 64
#define RE8670_RX_RING5_SIZE 64
#define RE8670_RX_RING6_SIZE 128 // make sure SWDL can be passed
#endif

//plz use the order of 2, set size 0 to no use ring
#define RE8670_TX_RING1_SIZE TX_EACH_RING_SIZE
#ifdef CONFIG_DUALBAND_CONCURRENT
#define RE8670_TX_RING2_SIZE 0
#define RE8670_TX_RING3_SIZE 16
#define RE8670_TX_RING4_SIZE 0
#define RE8670_TX_RING5_SIZE 16
#else
#define RE8670_TX_RING2_SIZE 16
#define RE8670_TX_RING3_SIZE 16
#define RE8670_TX_RING4_SIZE 16
#define RE8670_TX_RING5_SIZE 16
#endif

//total ring size
#define RE8670_RX_RING_SIZE \
RE8670_RX_RING1_SIZE+	\
RE8670_RX_RING2_SIZE+	\
RE8670_RX_RING3_SIZE+	\
RE8670_RX_RING4_SIZE+	\
RE8670_RX_RING5_SIZE+	\
RE8670_RX_RING6_SIZE

#define MAX_RXRING_NUM 6
#define MAX_TXRING_NUM 5

#define RDU6 (1 << 15)
#define RDU5 (1 << 14)
#define RDU4 (1 << 13)
#define RDU3 (1 << 12)
#define RDU2 (1 << 11)
#define RDU	(1 << 5)

#define RX_MULTIRING_BITMAP ((RE8670_RX_RING1_SIZE? 1: 0) \
							| (RE8670_RX_RING2_SIZE? 1<<1 : 0) \
							| (RE8670_RX_RING3_SIZE? 1<<2 : 0) \
							| (RE8670_RX_RING4_SIZE? 1<<3 : 0) \
							| (RE8670_RX_RING5_SIZE? 1<<4 : 0) \
							| (RE8670_RX_RING6_SIZE? 1<<5 : 0))

#define RX_RDU_CHECK(x) (((x&(RDU2|RDU3|RDU4|RDU5|RDU6))>>11<<1)|(((x&RDU)>>5)&1))

#define TX_MULTIRING_BITMAP ((RE8670_TX_RING1_SIZE? 1 : 0) | (RE8670_TX_RING2_SIZE? 1<<1 : 0) | (RE8670_TX_RING3_SIZE? 1<<2 : 0) | (RE8670_TX_RING4_SIZE? 1<<3 : 0) | (RE8670_TX_RING5_SIZE? 1<<4 : 0))
#define RX_ONLY_RING1 ((RX_MULTIRING_BITMAP == 1)? 1 : 0)
#define RX_NOT_ONLY_RING1 ((RX_MULTIRING_BITMAP == 1)? 0 : 1)

enum {
	/* NIC register offsets */
	IDR0 = 0,			/* Ethernet ID */
	IDR1 = 0x1,			/* Ethernet ID */
	IDR2 = 0x2,			/* Ethernet ID */
	IDR3 = 0x3,			/* Ethernet ID */
	IDR4 = 0x4,			/* Ethernet ID */
	IDR5 = 0x5,			/* Ethernet ID */
	MAR0 = 0x8,			/* Multicast register */
	MAR1 = 0x9,
	MAR2 = 0xa,
	MAR3 = 0xb,
	MAR4 = 0xc,
	MAR5 = 0xd,
	MAR6 = 0xe,
	MAR7 = 0xf,
	TXOKCNT=0x10,
	RXOKCNT=0x12,
	TXERR = 0x14,
	RXERRR = 0x16,
	MISSPKT = 0x18,
	FAE=0x1A,
	TX1COL = 0x1c,
	TXMCOL=0x1e,
	RXOKPHY=0x20,
	RXOKBRD=0x22,
	RXOKMUL=0x24,
	TXABT=0x26,
	TXUNDRN=0x28,
	RDUMISSPKT=0x2a,
	TRSR=0x34,
	COM_REG=0x38,
	CMD=0x3B,
	IMR=0x3C,
	ISR=0x3E,
	TCR=0x40,
	RCR=0x44,
	CPUtagCR=0x48,
	CONFIG_REG=0x4C,	
	CPUtag1CR=0x50,
	MSR=0x58,
	MIIAR=0x5C,
	SWINT_REG=0x60,
	VLAN_REG=0x64,
	LEDCR=0x70,
	IMR0=0xd0,
	IMR1=0xd4,
	ISR1=0xd8,
	INTR_REG=0xdc,
	TxFDP1=0x1300,
	TxCDO1=0x1304,
    TxFDP2=0x1310, 
	TxCDO2=0x1314, 
	TxFDP3=0x1320, 
	TxCDO3=0x1324, 
	TxFDP4=0x1330, 
	TxCDO4=0x1334,
	RRING_ROUTING1=0x1370,
	RRING_ROUTING2=0x1374,
	RRING_ROUTING3=0x1378,
	RRING_ROUTING4=0x137c,
	RRING_ROUTING5=0x1380,
	RRING_ROUTING6=0x1384,
	RxFDP=0x13F0,
	RxCDO=0x13F4,	
	RxRingSize=0x13F6,
	SMSA=0x13FC,
	EthrntRxCPU_Des_Num=0x1430,
	EthrntRxCPU_Des_Wrap =	0x1431,
	Rx_Pse_Des_Thres = 	0x1432,	
	RxRingSize_h=0x13F7, 
	EthrntRxCPU_Des_Num_h=0x1433,
	EthrntRxCPU_Des_Wrap_h =0x1433,
	Rx_Pse_Des_Thres_h =0x142c,


	RST = (1<<0),
	RxChkSum = (1<<1),
	RxVLAN_Detag = (1<<2),
	RxJumboSupport = (1<<3),
	IO_CMD = 0x1434,
	IO_CMD1 = 0x1438,  // RLE0371 and other_platform set different value.
	RX_IntNum_Shift = 0x8,             /// ????
	TX_OWN = (1<<5),
	RX_OWN = (1<<4),
	MII_RE = (1<<3),
	MII_TE = (1<<2),
	TX_FNL = (1<<1),
	TX_FNH = (1),
	/*TX_START= MII_RE | MII_TE | TX_FNL | TX_FNH,
	TX_START = 0x8113c,*/
	RXMII = MII_RE,
	TXMII= MII_TE,
	LSO_F	=(1<<28),
	EN_1GB	=(1<<24),

	Rff_size_sel_2k = (0x2 << 28),
	Int_route_r4r5r6t2t4 = (0x00440540),
	Int_route_enable = (1<<25),
	En_int_split = (0x01 << 24),
		
	RxFDP2=0x1390,
	RxCDO2=0x1394,
	RxRingSize2=0x1396,
	RxRingSize_h2=0x1397,
	EthrntRxCPU_Des_Num2=0x1398,
	EthrntRxCPU_Des_Wrap2 =	0x139c,
	DIAG1_REG=0x1404,
};

enum RE8670_IOCMD_REG
{

	RX_MIT 		= 7,
	RX_TIMER 	= 1,
	RX_FIFO 	= 2,
	TX_FIFO		= 1,
	TX_MIT		= 7,
	TX_POLL4	= 1 << 3,
	TX_POLL3	= 1 << 2,
	TX_POLL2	= 1 << 1,
	TX_POLL		= 1 << 0,
};

#define CMD_CONFIG 0xc059f130//0x4049E130// pkt timer = 15 => 4pkt trigger int
#define CMD1_CONFIG 0x30000000   //desc format ==> apollo type, not support multiple ring

enum RE8670_IOCMD1_REG
{
	TX_RR_scheduler	= 1 << 14,
	TX_POLL5	= 1 << 8,
	txq5_h		= 1 << 4,
	txq4_h		= 1 << 3,
	txq3_h		= 1 << 2,
	txq2_h		= 1 << 1,
	txq1_h		= 1 << 0,
};

enum{
	TXD_VLAN_INTACT,
	TXD_VLAN_INSERT,
	TXD_VLAN_REMOVE,
	TXD_VLAN_REMARKING,
};

struct rx_info{
	union{
		struct{
			u32 own:1;//31
			u32 eor:1;//30
			u32 fs:1;//29
			u32 ls:1;//28
			u32 crcerr:1;//27
			u32 ipv4csf:1;//26
			u32 l4csf:1;//25
			u32 rcdf:1;//24
			u32 ipfrag:1;//23
			u32 pppoetag:1;//22
			u32 rwt:1;//21
			u32 pkttype:4;//17~20
			u32 l3routing:1;//16
			u32 origformat:1;//15
			u32 pctrl:1;//14
#ifdef CONFIG_RG_JUMBO_FRAME
			u32 data_length:14;//0~13
#else
			u32 rsvd:2;//12~13
			u32 data_length:12;//0~11
#endif
		}bit;
		u32 dw;//double word
	}opts1;
	u32 addr;
	union{
		struct{
			u32 cputag:1;//31
			u32 ptp_in_cpu_tag_exist:1;//30
			u32 svlan_tag_exist:1;//29
			u32 rsvd_2:2;//27~28
			u32 pon_stream_id:7;//20~26
			u32 rsvd_1:3;//17~19
			u32 ctagva:1;//16
			u32 cvlan_tag:16;//0~15
		}bit;
		u32 dw;//double word
	}opts2;
	union{
		struct{
			u32 src_port_num:5;//27~31
			u32 dst_port_mask:6;//21~26
			u32 reason:8;//13~20
			u32 internal_priority:3;//10~12
			u32 ext_port_ttl_1:5;//5~9
			u32 rsvd:5;//0~4
		}bit;
		u32 dw;//double word
	}opts3;
};

struct tx_info{
	union{
		struct{
			u32 own:1;//31
			u32 eor:1;//30
			u32 fs:1;//29
			u32 ls:1;//28
			u32 ipcs:1;//27
			u32 l4cs:1;//26
			u32 keep:1;//25
			u32 blu:1;//24
			u32 crc:1;//23
			u32 vsel:1;//22
			u32 dislrn:1;//21
			u32 cputag_ipcs:1;//20
			u32 cputag_l4cs:1;//19
			u32 cputag_psel:1;//18
			u32 rsvd:1;//17
			u32 data_length:17;//0~16
		}bit;
		u32 dw;//double word
	}opts1;
	u32 addr;
	union{
		struct{
			u32 cputag:1;//31
			u32 aspri:1;//30
			u32 cputag_pri:3;//27~29
			u32 tx_vlan_action:2;//25~26
			u32 tx_pppoe_action:2;//23~24
			u32 tx_pppoe_idx:3;//20~22
			u32 efid:1;//19
			u32 enhance_fid:3;//16~18
			u32 vidl:8;//8~15
			u32 prio:3;//5~7
			u32 cfi:1;// 4
			u32 vidh:4;//0~3
		}bit;
		u32 dw;//double word
	}opts2;
	union{
		struct{
			u32 extspa:3;//29~31
			u32 tx_portmask:6;//23~28
			u32 tx_dst_stream_id:7;//16~22
			#if 0//def RTL0371
			u32 tx_dst_vc_mask:16;//0~15
			#else
			u32 rsvd:12;// 4~15
			u32 rsv1:1;// 3
			u32 rsv0:1;// 2
			u32 l34_keep:1;// 1
			u32 ptp:1;//0
			#endif
		}bit;
		u32 dw;//double word
	}opts3;
	union{
		u32 dw;
	}opts4;
};

typedef struct dma_tx_desc {
	u32		opts1;
	u32		addr;
	u32		opts2;
	u32		opts3; //cputag
	u32		opts4; //lso
}DMA_TX_DESC;

typedef struct dma_rx_desc {
	u32		opts1;
	u32		addr;
	u32		opts2;
	u32		opts3;
}DMA_RX_DESC;

struct cp_extra_stats {
	unsigned long		rx_frags;
	unsigned long tx_timeouts;
	//krammer add for rx info
	unsigned int rx_hw_num;
	unsigned int rx_sw_num;
	unsigned int rer_runt;
	unsigned int rer_ovf;
	unsigned int rdu;
	unsigned int frag;
#ifdef CONFIG_RG_JUMBO_FRAME
	unsigned int toobig;
#endif
	unsigned int crcerr;
	unsigned int rcdf;
	unsigned int rx_no_mem;
	//krammer add for tx info
	unsigned int tx_sw_num;
	unsigned int tx_hw_num;
	unsigned int tx_no_desc;
};


struct re_private {
	void			*regs;
	struct net_device	*dev;
	spinlock_t		tx_lock;
	spinlock_t		rx_lock;

    DMA_RX_DESC     *rx_Mring[MAX_RXRING_NUM];
    unsigned		rx_Mtail[MAX_RXRING_NUM];
    char*			rxdesc_Mbuf[MAX_RXRING_NUM];

    DMA_TX_DESC		*tx_Mhqring[MAX_TXRING_NUM];
	char*			txdesc_Mbuf[MAX_TXRING_NUM];
	unsigned		tx_Mhqhead[MAX_TXRING_NUM];
	unsigned		tx_Mhqtail[MAX_TXRING_NUM];

	struct ring_info	*tx_skb[MAX_TXRING_NUM];
	struct ring_info	*rx_skb[MAX_RXRING_NUM];

	#ifdef CONFIG_DUALBAND_CONCURRENT
	DMA_RX_DESC     	*default_rx_desc;
	char				*default_rxdesc_Mbuf;
	struct ring_info	*default_rx_skb;
	unsigned 		old_tx_Mhqhead[MAX_TXRING_NUM];
	unsigned 		old_tx_Mhqtail[MAX_TXRING_NUM];
	unsigned		old_rx_Mtail[MAX_RXRING_NUM];
	#endif

	
	unsigned		rx_buf_sz;
	dma_addr_t		ring_dma;
	u32			msg_enable;

	struct cp_extra_stats	cp_stats;
	u32 isr_status;
	u32 isr1_status;

	struct pci_dev		*pdev;
	u32			rx_config;

	struct sk_buff		*frag_skb;
	unsigned		dropping_frag : 1;

	//struct tq_struct	rx_task;
	//struct tq_struct	tx_task;
	struct tasklet_struct rx_tasklets;
	//struct tasklet_struct tx_tasklets;

#if 1	
	struct tasklet_struct tx_tasklets; 
#endif	
	struct net_device* port2dev[SW_PORT_NUM];
	int (*port2rxfunc[SW_PORT_NUM])(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo);

#ifdef CONFIG_RG_SIMPLE_PROTOCOL_STACK
	struct net_device* multiWanDev[8];		//at most 7 WAN interface(reserved one interface for WAN)
	int wanInterfaceIdx;				//used for multi-WAN in SPS
#endif

#ifdef CONFIG_RG_JUMBO_FRAME
	struct sk_buff *jumboFrame;
	unsigned short jumboLength;
#endif

};


//must be same with port2rxfunc define in cp
typedef int (*p2rfunc_t)(struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo);

//API list
int re8686_register_rxfunc_all_port(p2rfunc_t pfunc);
int re8686_send_with_txInfo(struct sk_buff *skb, struct tx_info* ptxInfo, int ring_num);
int re8686_send_with_txInfo_and_mask(struct sk_buff * skb,struct tx_info * ptxInfo,int ring_num,struct tx_info * ptxInfoMask);
void re8686_reset_rxfunc_to_default(void);
int re8670_rx_skb (struct re_private *cp, struct sk_buff *skb, struct rx_info *pRxInfo);
struct sk_buff *re8670_getAlloc(unsigned int size);



enum {
	RE8670_RX_STOP=0,
	RE8670_RX_CONTINUE,
	RE8670_RX_STOP_SKBNOFREE,
	RE8670_RX_END
};

/*
*   | callback                      	| priority		| return
*   | re8670_rx_skb                 	| 0			| STOP
*   | fwdEngine_rx_skb      		| 1			| STOP
*   | rtk_gpon_omci_rx_wrapper	| 2			| STOP
*   | re8686_dump_rx			| 6			| IS CONTINUE
*   | re8686_rx_patch			| 7			| IS CONTINUE
*/

typedef enum {
	RE8686_RXPRI_DEFAULT=0,
	RE8686_RXPRI_RG,
	RE8686_RXPRI_L34LITE,
	RE8686_RXPRI_VOIP,
	RE8686_RXPRI_OMCI,
	RE8686_RXPRI_OAM,
	RE8686_RXPRI_MPCP,
	RE8686_RXPRI_MUTICAST,
	RE8686_RXPRI_DUMP,
	RE8686_RXPRI_PATCH,
}RE8686_RX_PRI_T;

#endif /*_RE8686_RTL9601B_H_*/

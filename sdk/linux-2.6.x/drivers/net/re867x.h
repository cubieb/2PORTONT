/*	
 *	re867x.h
*/
#ifndef _RE867x_H_
#define _RE867x_H_

#ifdef CONFIG_RTK_VOIP
#if defined(CONFIG_6166_IAD_SILAB3217X) || defined(CONFIG_6166_IAD_ZARLINK)
#define CONFIG_INT_PHY 1
#endif
#else
#define CONFIG_INT_PHY 1
#endif

#define VPORT_CPU_TAG	0
#define VPORT_VLAN_TAG	1

#if defined(CONFIG_ETHWAN) || defined(CONFIG_E8B)
#define VPORT_USE	VPORT_CPU_TAG
#else
#define VPORT_USE	VPORT_VLAN_TAG
#endif

#ifdef CONFIG_INT_PHY
#ifdef CONFIG_RTL8681
#define ETHBASE	0xB8012000	//internal phy for RTL8681
#else
#define ETHBASE	0xB8018000	//internal phy
#endif
#else
#define ETHBASE	0xB8010000	//external phy
#endif

#define RTL_W32(reg, value)			(*(volatile u32*)(ETHBASE+reg)) = (u32)value
#define RTL_W16(reg, value)			(*(volatile u16*)(ETHBASE+reg)) = (u16)value
#define RTL_W8(reg, value)			(*(volatile u8*)(ETHBASE+reg)) = (u8)value
#define RTL_R32(reg)				(*(volatile u32*)(ETHBASE+reg))
#define RTL_R16(reg)				(*(volatile u16*)(ETHBASE+reg))
#define RTL_R8(reg)					(*(volatile u8*)(ETHBASE+reg))

//for alignment issue
#define READWD(addr) ((unsigned char *)addr)[0]<<24 | ((unsigned char *)addr)[1]<<16 | ((unsigned char *)addr)[2]<<8 | ((unsigned char *)addr)[3]
#define READHWD(addr) ((unsigned char *)addr)[0]<<8 | ((unsigned char *)addr)[1]


#ifdef CONFIG_ETHWAN
#define RE8670_RX_RING_SIZE	512//256
#define RE8670_TX_RING_SIZE	1024	//128
#else
#if defined(CONFIG_USB_RTL8192SU_SOFTAP) && !defined(CONFIG_RTL867X_PACKET_PROCESSOR)
#ifdef CONFIG_SKB_POOL_PREALLOC
#define RE8670_RX_RING_SIZE	256
#define RE8670_TX_RING_SIZE	256
#else
#define RE8670_RX_RING_SIZE	64//16~256 descriptors
#define RE8670_TX_RING_SIZE	128
#endif

#elif defined (CONFIG_RTL8192CD) || defined (CONFIG_RTL8192CD_MODULE)
#define RE8670_RX_RING_SIZE	64 //512
#define RE8670_TX_RING_SIZE	256 //1024
#else
#define RE8670_RX_RING_SIZE	64//16~256 descriptors
#define RE8670_TX_RING_SIZE	64
#endif
#endif //CONFIG_ETHWAN

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
	TRSR=0x34,
	CMD=0x3B,
	IMR=0x3C,
	ISR=0x3E,
	TCR=0x40,
	RCR=0x44,
	MSR=0x58,
	MIIAR=0x5C,
	SWINT=0x60,
	LEDCR=0x70,
    //CPUtagCR=0x48,
	//PGR=0x84,
	
    TxFDP1=0x1300,
	TxCDO1=0x1304,
    TxFDP2=0x1310, 
	TxCDO2=0x1314, 
	TxFDP3=0x1320, 
	TxCDO3=0x1324, 
	TxFDP4=0x1330, 
	TxCDO4=0x1334,
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
	Txc_Out_Ph_Sel_l = 0x142f,

	
	IO_CMD = 0x1434,
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
};

enum RE8670_IOCMD_REG
{

	RX_MIT 		= 7,
	RX_TIMER 	= 1,
	RX_FIFO 	= 2,
	TX_FIFO		= 1,
	TX_MIT		= 7,	
	TX_POLL		= 1 << 0,
//shlee	CMD_CONFIG = 0x3c | RX_MIT << 8 | RX_FIFO << 11 |  RX_TIMER << 13 | TX_MIT << 16 | TX_FIFO<<19,
//CMD_CONFIG = 0x40011930,	//according to tony's parameter

#ifdef CONFIG_ETHWAN
CMD_CONFIG = 0x4069E730	       // mitigation=7=> 8pkt trigger int
#else
#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
CMD_CONFIG=0x00091030
#else
CMD_CONFIG = 0x40091030	       // mitigation=0=> 1pkt trigger int
#endif
#endif //CONFIG_ETHWAN
};

void _skb_debug(struct sk_buff *skb, int enable, int flag);
#define SWITCH_VPORT_TO_867X	4
extern int virt2phy[];
#endif /*_RE867x_H_*/

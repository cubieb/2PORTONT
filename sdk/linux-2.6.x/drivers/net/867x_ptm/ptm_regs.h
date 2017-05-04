/*
 *	PTM Registers definition
 */
#ifndef _PTM_REGS_H_
#define _PTM_REGS_H_

/* Registers definition */
enum PTM_MODULE_REGS
{
	IDR0 = 0,			
	IDR1 = 0x1,			
	IDR2 = 0x2,			
	IDR3 = 0x3,			
	IDR4 = 0x4,			
	IDR5 = 0x5,			
	MAR0 = 0x8,			
	MAR1 = 0x9,
	MAR2 = 0xa,
	MAR3 = 0xb,
	MAR4 = 0xc,
	MAR5 = 0xd,
	MAR6 = 0xe,
	MAR7 = 0xf,

	/* TX DMA counters */
	TXDMAOKCNT_CH0_F = 0x10,
	TXDMAOKCNT_CH1_F = 0x14,
	TXDMAOKCNT_CH2_F = 0x18,
	TXDMAOKCNT_CH3_F = 0x1c,
	TXDMAOKCNT_CH0_S = 0x20,
	TXDMAOKCNT_CH1_S = 0x24,
	TXDMAOKCNT_CH2_S = 0x28,
	TXDMAOKCNT_CH3_S = 0x2c,

	/* RX related counters */
	RxOKCNT_L_F 	= 0x50,
	RxOKCNT_H_F 	= 0x54,
	RxOKCNT_L_S 	= 0x58,
	RxOKCNT_H_S 	= 0x5c,
	RxErr_L_F 		= 0x60,
	RxErr_H_F		= 0x64,
	RxErr_L_S 		= 0x68,
	RxErr_H_S 		= 0x6c,
	MissPkt_L_F 		= 0x70,
	MissPkt_H_F 		= 0x74,
	MissPkt_L_S 		= 0x78,
	MissPkt_H_S 		= 0x7c,
	RxOkPhy_L_F 	= 0x80,
	RxOkPhy_H_F 	= 0x84,
	RxOkPhy_L_S 	= 0x88,
	RxOkPhy_H_S 	= 0x8c,
	RxOkBrd_L_F 	= 0x90,
	RxOkBrd_H_F 	= 0x94,
	RxOkBrd_L_S 	= 0x98,
	RxOkBrd_H_S 	= 0x9c,
	RxOkMu1_L_F 	= 0xa0,
	RxOkMu1_H_F 	= 0xa4,
	RxOkMu1_L_S 	= 0xa8,
	RxOkMu1_H_S 	= 0xac,

	TRSR 			= 0xb0,
	CMD 			= 0xb7,
	IMR0 			= 0xb8,
	ISR0 			= 0xbc,
	IMR1 			= 0xc0,
	TCR 				= 0xc2,
	ISR1 			= 0xc4,
	RCR 				= 0xc6,
	PCR 				= 0xc8,
	TCTX_PKT 		= 0xcc,
	TCRX_PKT 		= 0xd0,
	TCRX_GOODPKT 	= 0xd4,
	TCRX_BADPKT 	= 0xd8,
	TCRX_INVALIDPKT = 0xdc,
	SWINT_REG 		= 0xec,
	MBR 			= 0xf0,

	RxcodeWord_F	= 0x1b0,
	RxCodeWord_S	= 0x1b4,

};

enum PTM_BONDING_REGS
{
	BOND_FRAG_LF = 0x1060,
	BOND_FRAG_HF = 0x1064,
	BOND_FRAG_LS = 0x1068,
	BOND_FRAG_HS = 0x106c,
	BD_SLV_NUM = 0x10d0,
	BD_TIMEOUT = 0x10d4,
};

enum PTM_QOS_REGS
{
	/* QoS register */
	DMA_APR32_F = 0x1000,
	DMA_APR10_F = 0x1004,
	DMA_APR32_S = 0x1008,
	DMA_APR10_S = 0x100c,
	DMA_ARP_ALL = 0x1010,
	DMA_WEIGHT_F = 0x1014,
	DMA_WEIGHT_S = 0x1018,
	DMA_QoS_MIS = 0x101c,
		QoS_SEL_F = (1<<3),
		QoS_SEL_S = (1<<2),
};

enum PTM_ERROR_REGS
{
	/* Rx CRC Error */
	Rx_CRC_ERR_HF	= 0x0180,
	Rx_CRC_ERR_HS	= 0x0184,
	Rx_CRC_ERR_LF	= 0x0188,
	Rx_CRC_ERR_LS	= 0x018c,

	/* Rx Invlaid Frame */
	HDLC_INVLD_F	= 0x01a0,
	HDLC_INVLD_S	= 0x01a4,

	/* Rx coding Error */
	TC_CODING_ERR_F	= 0x01a8,
	TC_CODING_ERR_S	= 0x01ac,

};

enum PTM_CPU_REGS
{
	/* TX channels */
	TxFDP0_F 		= 0x1300,
	TxCDO0_F 		= 0x1304,
	TxFDP1_F 		= 0x1310,
	TxCDO1_F		= 0x1314,
	TxFDP2_F 		= 0x1320,
	TxCDO2_F 		= 0x1324,
	TxFDP3_F 		= 0x1330,
	TxCDO3_F 		= 0x1334,
	TxFDP0_S		= 0x1340,
	TxCDO0_S 		= 0x1344,
	TxFDP1_S 		= 0x1350,
	TxCDO1_S 		= 0x1354,
	TxFDP2_S 		= 0x1360,
	TxCDO2_S 		= 0x1364,
	TxFDP3_S 		= 0x1370,
	TxCDO3_S 		= 0x1374,

	/* RX channels */
	RxFDP_HMQ_L_F 	= 0x13c0,
	RxCDO_HMQ_L_F 	= 0x13c4,
	RxFDP_HMQ_H_F 	= 0x13c8,
	RxCDO_HMQ_H_F = 0x13cc,
	RxFDP_HMQ_L_S 	= 0x13d0,
	RxCDO_HMQ_L_S = 0x13d4,
	RxFDP_HMQ_H_S 	= 0x13d8,
	RxCDO_HMQ_H_S = 0x13dc,
	RxFDP_L_F 		= 0x13e0,
	RxCDO_L_F 		= 0x13e4,
	RxFDP_H_F 		= 0x13e8,
	RxCDO_H_F 		= 0x13ec,
	RxFDP_L_S 		= 0x13f0,
	RxCDO_L_S 		= 0x13f4,
	RxFDP_H_S 		= 0x13f8,
	RxCDO_H_S 		= 0x13fc,

	IO_CMD 			= 0x1434,
	CH_PRI 			= 0x1438,
};

enum PTM_DESC_INFO
{
	/* Tx and Rx status descriptors */
	DescOwn		= (1 << 31), /* Descriptor is owned by NIC */
	RingEnd		= (1 << 30), /* End of descriptor ring */
	FirstFrag		= (1 << 29), /* First segment of a packet */
	LastFrag		= (1 << 28), /* Final segment of a packet */

	RxError		= (1 << 20), /* Rx error summary */
	RxErrFrame	= (1 << 27), /* Rx frame alignment error */
	RxMcast		= (1 << 26), /* Rx multicast packet rcv'd */
	RxErrCRC	= (1 << 18), /* Rx CRC error */
	RxErrRunt	= (1 << 19), /* Rx error, packet < 64 bytes */
	
	TxCRC		= (1 << 23), /* Tx append CRC */

};

enum RE8670_THRESHOLD_REGS{

 	AcceptErr		= 0x20,	     /* Accept packets with CRC errors */
	AcceptRunt		= 0x10,	     /* Accept runt (<64 bytes) packets */
	AcceptBroadcast	= 0x08,	     /* Accept broadcast packets */
	AcceptMulticast	= 0x04,	     /* Accept multicast packets */
	AcceptMyPhys		= 0x02,	     /* Accept pkts with our MAC as dest */
	AcceptAllPhys		= 0x01,	     /* Accept all pkts w/ physical dest */
	AcceptAll 		= AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	AcceptNoBroad 	= AcceptMulticast |AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	AcceptNoMulti 	=  AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	NoErrAccept 		= AcceptBroadcast | AcceptMulticast | AcceptMyPhys,
	NoErrPromiscAccept = AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys,
};

enum PTM_ISR_REGS{
	TDU_CH3_S		= (1 <<31),
	TDU_CH2_S		= (1 <<30),
	TDU_CH1_S		= (1 <<29),
	TDU_CH0_S		= (1 <<28),
	TDU_CH3_F		= (1 <<27),
	TDU_CH2_F		= (1 <<26),
	TDU_CH1_F		= (1 <<25),
	TDU_CH0_F		= (1 <<24),
	TOK_CH3_S		= (1 <<23),
	TOK_CH2_S		= (1 <<22),
	TOK_CH1_S		= (1 <<21),
	TOK_CH0_S		= (1 <<20),
	TOK_CH3_F		= (1 <<19),
	TOK_CH2_F		= (1 <<18),
	TOK_CH1_F		= (1 <<17),
	TOK_CH0_F		= (1 <<16),
	RER_OVF_H_S	= (1 <<15),
	RER_OVF_L_S	= (1 <<14),
	RER_OVF_H_F	= (1 <<13),
	RER_OVF_L_F	= (1 <<12),
	RDU_H_S		= (1 <<11),
	RDU_L_S		= (1 <<10),
	RDU_H_F		= (1 <<9),
	RDU_L_F			= (1 <<8),
	RUNT_H_S		= (1 <<7),
	RUNT_L_S		= (1 <<6),
	RUNT_H_F		= (1 <<5),
	RUNT_L_F		= (1 <<4),
	ROK_H_S		= (1 << 3),
	ROK_L_S		= (1 << 2),
	ROK_H_F		= (1 << 1),
	ROK_L_F			= (1 << 0),
};

enum RE8670_IOCMD_REG
{
	RX_MIT 			= 7,
	RX_TIMER 		= 1,
	RX_FIFO 		= 2,
	TX_FIFO			= 1,
	TX_MIT			= 7,	
	TX_POLL_CH3S	= 1 << 7,
	TX_POLL_CH2S	= 1 << 6,
	TX_POLL_CH1S	= 1 << 5,
	TX_POLL_CH0S	= 1 << 4,
	TX_POLL_CH3F	= 1 << 3,
	TX_POLL_CH2F	= 1 << 2,
	TX_POLL_CH1F	= 1 << 1,
	TX_POLL_CH0F	= 1 << 0,

#ifdef CONFIG_RTL867X_PACKET_PROCESSOR
	CMD_CONFIG=0x00091030
#else
	//CMD_CONFIG = 0x40081030	       // rxfth = 64 bytes, mitigation=0=> 1pkt trigger int
	//CMD_CONFIG = 0x58001000
	CMD_CONFIG = 0x58001800
#endif
};



#endif

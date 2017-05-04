#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/in.h>
#include <linux/if_vlan.h>
#include <linux/crc32.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <net/xfrm.h>
#include <linux/proc_fs.h>
#include "re_privskb.h"
#include <bspchip.h>

#if defined(CONFIG_RTL9600_SERIES)
#include "re8686.h"
#endif
#if defined(CONFIG_RTL9601B_SERIES)
#include "re8686_rtl9601b.h"
#endif
#if defined(CONFIG_RTL9602C_SERIES)
#include "re8686_rtl9602c.h"
#endif


typedef struct rx_extInfo {
	struct rx_info rxInfo;
	unsigned ring_num;
}rx_extInfo_t;


struct tx_info gTxInfo;
rx_extInfo_t gRxInfo;
static int isMacChange = 0;
static int stopRx = 0;
static int isRxRingRoute = 0;
static unsigned int ring1Route = 0;
static int gTxRing=0;
static int isLsoPkt=0;
static int isPktDbg=0;
static int isRxCrcDbg=0;
static struct sk_buff *jumboFrame = NULL;
static unsigned short jumboLength = 0;
static int txMtu = 1500;

#define JUMBO_SKB_BUF_SIZE	(13312+2)
#define SKB_BUF_SIZE 1600
#define RX_OFFSET 2

#define TXINFO_SET(sel,field,value) gTxInfo.sel.bit.field = value

#define DEV2CP(dev)  (((struct re_dev_private*)dev->priv)->pCp)
#define DEVPRIV(dev)  ((struct re_dev_private*)dev->priv)
#define NEXT_RX(N,RING_SIZE)		(((N) + 1) & (RING_SIZE - 1))
#define NEXT_TX(N,RING_SIZE)		(((N) + 1) & (RING_SIZE - 1))

unsigned int iocmd_reg=CMD_CONFIG;
unsigned int iocmd1_reg=CMD1_CONFIG | (RX_NOT_ONLY_RING1<<25);   


static const int RE8670_RX_MRING_SIZE[MAX_RXRING_NUM] = {
RE8670_RX_RING1_SIZE,
RE8670_RX_RING2_SIZE,
RE8670_RX_RING3_SIZE,
RE8670_RX_RING4_SIZE,
RE8670_RX_RING5_SIZE,
RE8670_RX_RING6_SIZE
};

static const int RE8670_TX_MRING_SIZE[MAX_TXRING_NUM] = {
RE8670_TX_RING1_SIZE,
RE8670_TX_RING2_SIZE,
RE8670_TX_RING3_SIZE,
RE8670_TX_RING4_SIZE,
RE8670_TX_RING5_SIZE
};

static int tx_dump_flag = 0;

#define TX_HQBUFFS_AVAIL(CP,ring_num)					\
		(((CP)->tx_Mhqtail[ring_num] - (CP)->tx_Mhqhead[ring_num] + RE8670_TX_MRING_SIZE[ring_num] - 1)&(RE8670_TX_MRING_SIZE[ring_num] - 1))		


#define multi_ring_set()  \
if(isRxRingRoute){ \
	RTL_W32(IMR0,0x3f); \
	RTL_W32(RRING_ROUTING1, ring1Route); \
}else \
{ \	
	RTL_W32(IMR0,0x1); \
	RTL_W32(RRING_ROUTING1, ring1Route); \
}

enum RE8670_STATUS_REGS
{
	/*TX/RX share */
	DescOwn		= (1 << 31), /* Descriptor is owned by NIC */
	RingEnd		= (1 << 30), /* End of descriptor ring */
	FirstFrag		= (1 << 29), /* First segment of a packet */
	LastFrag		= (1 << 28), /* Final segment of a packet */

	/*Tx descriptor opt1*/
	IPCS		= (1 << 27),
	L4CS		= (1 << 26),
	KEEP		= (1 << 25),
	BLU			= (1 << 24),
	TxCRC		= (1 << 23),
	VSEL		= (1 << 22),
	DisLrn		= (1 << 21),
	CPUTag_ipcs 	= (1 << 20),
	CPUTag_l4cs	= (1 << 19),

	/*Tx descriptor opt2*/
	CPUTag		= (1 << 31),
	aspri		= (1 << 30),
	CPRI		= (1 << 27),
	TxVLAN_int	= (0 << 25),  //intact
	TxVLAN_ins	= (1 << 25),  //insert
	TxVLAN_rm	= (2 << 25),  //remove
	TxVLAN_re	= (3 << 25),  //remark
	//TxPPPoEAct	= (1 << 23),
	TxPPPoEAct	= 23,
	//TxPPPoEIdx	= (1 << 20),
	TxPPPoEIdx	= 20,
	Efid			= (1 << 19),
	//Enhan_Fid	= (1 << 16),
	Enhan_Fid 	= 16,
	/*Tx descriptor opt3*/
	SrcExtPort	= 29,
	TxDesPortM	= 23,
	TxDesStrID 	= 16,
	TxDesVCM	= 0,
	/*Tx descriptor opt4*/
	/*Rx descriptor  opt1*/
	CRCErr	= (1 << 27),
	IPV4CSF		= (1 << 26),
	L4CSF		= (1 << 25),
	RCDF		= (1 << 24),
	IP_FRAG		= (1 << 23),
	PPPoE_tag	= (1 << 22),
	RWT			= (1 << 21),
	PktType		= (1 << 17),
	RxProtoIP	= 1,
	RxProtoPPTP	= 2,
	RxProtoICMP	= 3,
	RxProtoIGMP	= 4,
	RxProtoTCP	= 5,   
	RxProtoUDP	= 6,
	RxProtoIPv6	= 7,
	RxProtoICMPv6	= 8,
	RxProtoTCPv6	= 9,
	RxProtoUDPv6	= 10,
	L3route		= (1 << 16),
	OrigFormat	= (1 << 15),
	PCTRL		= (1 << 14),
	/*Rx descriptor opt2*/
	PTPinCPU	= (1 << 30),
	SVlanTag		= (1 << 29),
	/*Rx descriptor opt3*/
	SrcPort		= (1 << 27),
	DesPortM	= (1 << 21),
	Reason		= (1 << 13),
	IntPriority	= (1 << 10),
	ExtPortTTL	= (1 << 5),
};


enum RE8670_THRESHOLD_REGS{
	//shlee	THVAL		= 2,
	TH_ON_VAL = 0x10,	//shlee flow control assert threshold: available desc <= 16
	TH_OFF_VAL= 0x30,	//shlee flow control de-assert threshold : available desc>=48
	//	RINGSIZE	= 0x0f,	//shlee 	2,
	LOOPBACK	= (0x3 << 8),
	AcceptErr	= 0x20,	     /* Accept packets with CRC errors */
	AcceptRunt	= 0x10,	     /* Accept runt (<64 bytes) packets */
	AcceptBroadcast	= 0x08,	     /* Accept broadcast packets */
	AcceptMulticast	= 0x04,	     /* Accept multicast packets */
	AcceptMyPhys	= 0x02,	     /* Accept pkts with our MAC as dest */
	AcceptAllPhys	= 0x01,	     /* Accept all pkts w/ physical dest */
	AcceptAll = AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	AcceptNoBroad = AcceptMulticast |AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	AcceptNoMulti =  AcceptMyPhys |  AcceptAllPhys | AcceptErr | AcceptRunt,
	NoErrAccept = AcceptBroadcast | AcceptMulticast | AcceptMyPhys,
	NoErrPromiscAccept = AcceptBroadcast | AcceptMulticast | AcceptMyPhys |  AcceptAllPhys,
};


enum RE8670_ISR_REGS{
	SW_INT 		= (1 <<10),
	TDU	= (1 << 9),
	LINK_CHG	= (1 <<	8),
	TER		= (1 << 7),
	TOK		= (1 << 6),
	RER_OVF	=(1 << 4),
	RER_RUNT	=(1 << 2),
	RX_OK		= (1 << 0),
	RDU_ALL = (RDU | RDU2 | RDU3 | RDU4 | RDU5 | RDU6),
	RX_ALL = (
		RX_OK | RER_RUNT | RER_OVF |
		((RX_MULTIRING_BITMAP & 1) ? RDU : 0) |
		(RX_MULTIRING_BITMAP >> 1) << 11
	),
};

enum RTL8672GMAC_CPUtag_Control
{
	CTEN_RX     = (1<<31),
	CT_TSIZE	= 27,
	CT_DSLRN	= (1 << 24),
	CT_NORMK	= (1 << 23),
	CT_ASPRI	= (1 << 22),
	CT_APPLO	= (6 << 18),
	CT_8681	= (5 << 18),
	CT_8370S    = (4 <<18),
	CT_8307S	= (3 << 18),
	CT_8306S    = (2<<18),
	CT_8368S    = (1<<18),

	CT_RSIZE_H = 25,
	CT_RSIZE_L = 16,
	CTPM_8306   = (0xf0 << 8),
	CTPM_8368   = (0xe0 << 8),
	CTPM_8370   = (0xff << 8),
	CTPM_8307   = (0xff << 8),
	CTPV_8306   = 0x90,
	CTPV_8368   = 0xa0,
	CTPV_8370   = 0x04,
	CTPV_8307	  = 0x04,
};

enum RTL8672GMAC_PG_REG
{
	EN_PGLBK     = (1<<15),
	DATA_SEL     = (1<<14),
	LEN_SEL      = (1<<11),
	NUM_SEL      = (1<<10),
};

struct ring_info {
	struct sk_buff		*skb;
	dma_addr_t		mapping;
	unsigned		frag;
};


struct re_dev_private {
	struct re_private* pCp;
	struct net_device_stats net_stats;
	unsigned char txPortMask;
};

__IRAM_NIC
struct sk_buff *re8670_getAlloc(unsigned int size)
{	
	struct sk_buff *skb=NULL;

	skb = dev_alloc_skb_priv_eth(CROSS_LAN_MBUF_LEN);
	
	return skb;
}

static inline void unmask_rx_int(void){
	RTL_R16(IMR)|=(u16)(RX_ALL);//we still open imr when rx_work==0 for a quickly schedule	
	multi_ring_set();
}


static inline void retriveRxInfo(DMA_RX_DESC *desc, struct rx_info *pRxInfo){
	pRxInfo->opts1.dw = desc->opts1;
	pRxInfo->addr= desc->addr;
	pRxInfo->opts2.dw = desc->opts2;
	pRxInfo->opts3.dw = desc->opts3;
}


static inline void updateGmacFlowControl(unsigned rx_tail,int ring_num){
	unsigned int new_cpu_desc_num;		

	if(ring_num==0)
	{    
		new_cpu_desc_num = RTL_R32(EthrntRxCPU_Des_Num);
		new_cpu_desc_num &= 0x00FFFF0F;//clear
		new_cpu_desc_num |= (((rx_tail&0xFF)<<24)|(((rx_tail>>8)&0xF)<<4));//update
		RTL_R32(EthrntRxCPU_Des_Num) = new_cpu_desc_num;
	}
	else
	{
		new_cpu_desc_num = RTL_R32(EthrntRxCPU_Des_Num2+ADDR_OFFSET*(ring_num-1));
		new_cpu_desc_num &= 0xfffff000;//clear
		new_cpu_desc_num |= (((rx_tail&0xFF)|(((rx_tail>>8)&0xF)<<4)));//update
		RTL_W32(EthrntRxCPU_Des_Num2+ADDR_OFFSET*(ring_num-1), new_cpu_desc_num);
	}
}


void tx_dump(struct tx_info txInfo)
{
	
	/*opt1*/
	printk("own: %d\n",txInfo.opts1.bit.own);
	printk("eor: %d\n",txInfo.opts1.bit.eor);
	printk("fs: %d\n",txInfo.opts1.bit.fs);
	printk("ls: %d\n",txInfo.opts1.bit.ls);
	printk("ipcs: %d\n",txInfo.opts1.bit.ipcs);
	printk("l4cs: %d\n",txInfo.opts1.bit.l4cs);
	printk("keep: %d\n",txInfo.opts1.bit.keep);
	printk("blu: %d\n",txInfo.opts1.bit.blu);
	printk("crc: %d\n",txInfo.opts1.bit.crc);	
	printk("vsel: %d\n",txInfo.opts1.bit.vsel);
	printk("dislrn: %d\n",txInfo.opts1.bit.dislrn);
	printk("cputag_ipcs: %d\n",txInfo.opts1.bit.cputag_ipcs);
	printk("cputag_l4cs: %d\n",txInfo.opts1.bit.cputag_l4cs);
	printk("cputag_psel: %d\n",txInfo.opts1.bit.cputag_psel);
	printk("data_length: %d\n",txInfo.opts1.bit.data_length);	
	printk("addr:%x\n",txInfo.addr);

	/*opt2*/
	printk("cputag: %d\n",txInfo.opts2.bit.cputag);	
	printk("aspri: %d\n",txInfo.opts2.bit.aspri);
	printk("cputag_pri: %d\n",txInfo.opts2.bit.cputag_pri);
	printk("tx_vlan_action: %d\n",txInfo.opts2.bit.tx_vlan_action);
	printk("tx_pppoe_action: %d\n",txInfo.opts2.bit.tx_pppoe_action);
	printk("tx_pppoe_idx: %d\n",txInfo.opts2.bit.tx_pppoe_idx);
	printk("efid: %d\n",txInfo.opts2.bit.efid);	
	printk("enhance_fid: %d\n",txInfo.opts2.bit.enhance_fid);
	printk("vidl: %d\n",txInfo.opts2.bit.vidl);
	printk("prio: %d\n",txInfo.opts2.bit.prio);
	printk("vidh: %d\n",txInfo.opts2.bit.vidh);
	/*opt3*/

	printk("tx_portmask: %d\n",txInfo.opts3.bit.tx_portmask);	
	printk("tx_dst_stream_id: %d\n",txInfo.opts3.bit.tx_dst_stream_id);
	printk("l34_keep: %d\n",txInfo.opts3.bit.l34_keep);
	printk("ptp: %d\n",txInfo.opts3.bit.ptp);

	/*private*/
	printk("isMacChange: %d\n",isMacChange);
	printk("tx_dump_flag: %d\n",tx_dump_flag);
	printk("stopRx: %d\n",stopRx);
	printk("isRxRingRoute: %d\n",isRxRingRoute);
	printk("ring1Route: 0x%x\n",ring1Route);
	printk("gTxRing: %d\n",gTxRing);
	printk("isLsoPtk: %d\n",isLsoPkt);
	printk("isPktDbg: %d\n",isPktDbg);
	printk("isRxCrcDbg: %d\n", isRxCrcDbg);
	printk("txMtu: %d\n",txMtu);
	
}


int tx_dump_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	tx_dump(gTxInfo);
	return 0;
}

int tx_dump_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	char 	tmpbuf[64];
	char	*strptr;
	char	*fieldptr,*valueptr;
	unsigned int value;
	
	if (buff && !copy_from_user(tmpbuf, buff, len)) {
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		fieldptr = strsep(&strptr," ");
		if(fieldptr == NULL)
		{
			goto errout;
		}

		valueptr = strsep(&strptr," ");
		if(valueptr == NULL)
		{
			goto errout;
		}
		value = simple_strtol(valueptr, NULL, 0);

		printk("set field: %s, value %d\n",fieldptr,value);
		
		if(!strcmp(fieldptr,"fs"))
		{
			TXINFO_SET(opts1,fs,value);
		}else
		if(!strcmp(fieldptr,"ls"))
		{
			TXINFO_SET(opts1,ls,value);
		}else
		if(!strcmp(fieldptr,"tx_portmask"))
		{
			TXINFO_SET(opts3,tx_portmask,value);
		}else
		if(!strcmp(fieldptr,"ipcs"))
		{
			TXINFO_SET(opts1,ipcs,value);
		}else
		if(!strcmp(fieldptr,"l4cs"))
		{
			TXINFO_SET(opts1,l4cs,value);
		}else
		if(!strcmp(fieldptr,"keep"))
		{
			TXINFO_SET(opts1,keep,value);
		}else
		if(!strcmp(fieldptr,"blu"))
		{
			TXINFO_SET(opts1,blu,value);
		}else
		if(!strcmp(fieldptr,"crc"))
		{
			TXINFO_SET(opts1,crc,value);
		}else
		if(!strcmp(fieldptr,"vsel"))
		{
			TXINFO_SET(opts1,vsel,value);
		}else
		if(!strcmp(fieldptr,"dislrn"))
		{
			TXINFO_SET(opts1,dislrn,value);
		}else
		if(!strcmp(fieldptr,"cputag_ipcs"))
		{
			TXINFO_SET(opts1,cputag_ipcs,value);
		}else
		if(!strcmp(fieldptr,"cputag_l4cs"))
		{
			TXINFO_SET(opts1,cputag_l4cs,value);
		}else
		if(!strcmp(fieldptr,"cputag_psel"))
		{
			TXINFO_SET(opts1,cputag_psel,value);
		}else
		if(!strcmp(fieldptr,"data_length"))
		{
			TXINFO_SET(opts1,data_length,value);
		}else
		if(!strcmp(fieldptr,"cputag"))
		{
			TXINFO_SET(opts2,cputag,value);
		}else
		if(!strcmp(fieldptr,"aspri"))
		{
			TXINFO_SET(opts2,aspri,value);
		}else
		if(!strcmp(fieldptr,"cputag_pri"))
		{
			TXINFO_SET(opts2,cputag_pri,value);
		}else
		if(!strcmp(fieldptr,"tx_vlan_action"))
		{
			TXINFO_SET(opts2,tx_vlan_action,value);
		}else
		if(!strcmp(fieldptr,"tx_pppoe_idx"))
		{
			TXINFO_SET(opts2,tx_pppoe_idx,value);
		}else
		if(!strcmp(fieldptr,"efid"))
		{
			TXINFO_SET(opts2,efid,value);
		}else
		if(!strcmp(fieldptr,"enhance_fid"))
		{
			TXINFO_SET(opts2,enhance_fid,value);
		}else
		if(!strcmp(fieldptr,"vidl"))
		{
			TXINFO_SET(opts2,vidl,value);
		}else
		if(!strcmp(fieldptr,"prio"))
		{
			TXINFO_SET(opts2,prio,value);
		}else
		if(!strcmp(fieldptr,"vidh"))
		{
			TXINFO_SET(opts2,vidh,value);
		}else
		if(!strcmp(fieldptr,"tx_dst_stream_id"))
		{
			TXINFO_SET(opts3,tx_dst_stream_id,value);
		}else
		if(!strcmp(fieldptr,"l34_keep"))
		{
			TXINFO_SET(opts3,l34_keep,value);
		}else
		if(!strcmp(fieldptr,"ptp"))
		{
			TXINFO_SET(opts3,ptp,value);
		}else
		if(!strcmp(fieldptr,"isMacChange"))
		{
			isMacChange = value;
		}else
		if(!strcmp(fieldptr,"tx_dump_flag"))
		{
			tx_dump_flag = value;
		}else
		if(!strcmp(fieldptr,"stopRx"))
		{
			stopRx = value;
		}else
		if(!strcmp(fieldptr,"isRxRingRoute"))
		{
			isRxRingRoute = value;
			multi_ring_set();
		}else
		if(!strcmp(fieldptr,"ring1Route"))
		{
			value = simple_strtol(valueptr, NULL, 16);
			ring1Route = value;
		}else
		if(!strcmp(fieldptr,"gTxRing"))
		{
			gTxRing = value;
		}else
		if(!strcmp(fieldptr,"isLsoPkt"))
		{
			isLsoPkt = value;
		}else
		if(!strcmp(fieldptr,"isPktDbg"))
		{
			isPktDbg = value;
		}else
		if(!strcmp(fieldptr,"isRxCrcDbg"))
		{
			isRxCrcDbg = value;
		}else
		if(!strcmp(fieldptr,"txMtu"))
		{
			txMtu = value;
		}
	}
	else
	{
errout:
		printk("tx_dump_writee error\n");
	}

	return len;
}


void rxInfo_dump(rx_extInfo_t rf)
{
	struct rx_info rxInfo = rf.rxInfo;

	/*opt1*/
	printk("own: %d\n",rxInfo.opts1.bit.own); // 3
	printk("eor: %d\n",rxInfo.opts1.bit.eor); // 5
	printk("fs: %d\n",rxInfo.opts1.bit.fs); // 7
	printk("ls: %d\n",rxInfo.opts1.bit.ls); // 9
	printk("crcerr: %d\n",rxInfo.opts1.bit.crcerr); // 11
	printk("ipv4csf: %d\n",rxInfo.opts1.bit.ipv4csf); // 13
	printk("l4csf: %d\n",rxInfo.opts1.bit.l4csf); // 15
	printk("rcdf: %d\n",rxInfo.opts1.bit.rcdf); // 17
	printk("ipfrag: %d\n",rxInfo.opts1.bit.ipfrag); // 19
	printk("pppoetag: %d\n",rxInfo.opts1.bit.pppoetag);	// 21
	printk("rwt: %d\n",rxInfo.opts1.bit.rwt); // 23
	printk("pkttype: %d\n",rxInfo.opts1.bit.pkttype); // 25
	printk("l3routing: %d\n",rxInfo.opts1.bit.l3routing); // 27
	printk("origformat: %d\n",rxInfo.opts1.bit.origformat); // 29
	printk("pctrl: %d\n",rxInfo.opts1.bit.pctrl); // 31
	printk("data_length: %d\n",rxInfo.opts1.bit.data_length); // 33
	printk("addr: %x\n",rxInfo.addr); // 35
	/*opt2*/
	printk("cputag: %d\n",rxInfo.opts2.bit.cputag);	// 37
	printk("ptp_in_cpu_tag_exist: %d\n",rxInfo.opts2.bit.ptp_in_cpu_tag_exist); // 39
	printk("svlan_tag_exist: %d\n",rxInfo.opts2.bit.svlan_tag_exist); // 41
	printk("pon_stream_id: %d\n",rxInfo.opts2.bit.pon_stream_id); // 43
	printk("ctagva: %d\n",rxInfo.opts2.bit.ctagva); // 45
	printk("cvlan_tag: %d\n",rxInfo.opts2.bit.cvlan_tag); // 47
	/*opt3*/
	printk("src_port_num: %d\n",rxInfo.opts3.bit.src_port_num);	//49
	printk("dst_port_mask: %d\n",rxInfo.opts3.bit.dst_port_mask); // 51
	printk("internal_priority: %d\n",rxInfo.opts3.bit.internal_priority); // 53
	printk("ext_port_ttl_1: %d\n",rxInfo.opts3.bit.ext_port_ttl_1); // 55
	printk("reason: %d\n",rxInfo.opts3.bit.reason); // 57
	/*extend information*/
	printk("ring_num: %d\n",rf.ring_num); // 59
}

int rx_dump_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	rxInfo_dump(gRxInfo);
	return 0;
}

int rx_dump_write(struct file *file, const char *buff, unsigned long len, void *data)
{
	return 0;
}

int save_rx_info(struct rx_info rxInfo,unsigned ring_num)
{
	memcpy(&gRxInfo.rxInfo,&rxInfo,sizeof(struct rx_info));
	gRxInfo.ring_num = ring_num;
	return 0;
}


static inline void apply_to_txdesc(DMA_TX_DESC  *txd, struct tx_info *pTxInfo){
	txd->addr = pTxInfo->addr;
	txd->opts2 = pTxInfo->opts2.dw;
	txd->opts3 = pTxInfo->opts3.dw;
	txd->opts4 = pTxInfo->opts4.dw;
	//must be last write....
	wmb();
	txd->opts1 = pTxInfo->opts1.dw;
}

static inline void kick_tx(int ring_num){
	switch(ring_num){
		case 0:
		case 1:
		case 2:
		case 3:
			RTL_W32(IO_CMD,iocmd_reg |(1 << ring_num));
			break;
		case 4:
			RTL_W32(IO_CMD1,iocmd1_reg | TX_POLL5);
			break;
		default:
			printk("%s %d: wrong ring num %d\n", __func__, __LINE__, ring_num);
			break;
	}
}

__IRAM_NIC void re8670_tx (struct re_private *cp,int ring_num)
{
	unsigned tx_tail= cp->tx_Mhqtail[ring_num];
	u32 status;
	struct sk_buff *skb;

	while (!((status = (cp->tx_Mhqring[ring_num][tx_tail].opts1))& DescOwn)) {
		if (tx_tail == cp->tx_Mhqhead[ring_num])
			break;

		skb = cp->tx_skb[ring_num][tx_tail].skb;

		if (unlikely(!skb))   
			break;

		dev_kfree_skb_any(skb); 
		cp->tx_skb[ring_num][tx_tail].skb = NULL;
		
		tx_tail = NEXT_TX(tx_tail,RE8670_TX_MRING_SIZE[ring_num]);

	}
	cp->tx_Mhqtail[ring_num]=tx_tail;
}

int re8686_test_with_txInfo(struct sk_buff *skb, struct tx_info txInfoInput, int ring_num)
{
	struct net_device *dev = skb->dev;
	struct re_private *cp = DEV2CP(dev);
	unsigned entry;
	u32 eor;
    u32 i;
    unsigned char *ptr;
	unsigned long flags;

	cp->cp_stats.tx_sw_num++;

	if(!(cp->dev->flags & IFF_UP)) {
		dev_kfree_skb_any(skb);
		DEVPRIV(skb->dev)->net_stats.tx_dropped++;	
		return -1;
	}
    re8670_tx(cp,ring_num);

	spin_lock_irqsave(&cp->tx_lock, flags);

	if (unlikely(TX_HQBUFFS_AVAIL(cp,ring_num) <= (skb_shinfo(skb)->nr_frags + 1)))
	{
		spin_unlock_irqrestore(&cp->tx_lock, flags);
		dev_kfree_skb_any(skb);
		DEVPRIV(skb->dev)->net_stats.tx_dropped++;
		cp->cp_stats.tx_no_desc++;
		return -1;
	}

	entry = cp->tx_Mhqhead[ring_num];

	eor = (entry == (RE8670_TX_MRING_SIZE[ring_num] - 1)) ? RingEnd : 0;
	if (skb_shinfo(skb)->nr_frags == 0) {
		u32 len;
		DMA_TX_DESC  *txd;
		txd = &cp->tx_Mhqring[ring_num][entry];    

		len = skb->len;				
		// Kaohj --- invalidate DCache before NIC DMA
		dma_cache_wback_inv((unsigned long)skb->data, len);
		cp->tx_skb[ring_num][entry].skb = skb;
		cp->tx_skb[ring_num][entry].mapping = (dma_addr_t)(skb->data);
		cp->tx_skb[ring_num][entry].frag = 0;

		//default setting, always need this
		txInfoInput.addr = (u32)(skb->data);
		txInfoInput.opts1.dw |= (eor | len | DescOwn | FirstFrag |LastFrag);
	

		if(tx_dump_flag)
			tx_dump(txInfoInput);

		//apply to txdesc
		apply_to_txdesc(txd, &txInfoInput);

		if(isPktDbg) {
			printk("FROM_FWD[%x] DA=%02x:%02x:%02x:%02x:%02x:%02x SA=%02x:%02x:%02x:%02x:%02x:%02x ethtype=%04x len=%d VlanAct=%d Vlan=%d Pri=%d ExtSpa=%d TxPmsdk=0x%x L34Keep=%x PON_SID=%d\n",
			(u32)skb&0xffff,
			skb->data[0],skb->data[1],skb->data[2],skb->data[3],skb->data[4],skb->data[5],
			skb->data[6],skb->data[7],skb->data[8],skb->data[9],skb->data[10],skb->data[11],
			(skb->data[12]<<8)|skb->data[13],skb->len,
			txInfoInput.opts2.bit.tx_vlan_action,
			txInfoInput.opts2.bit.vidh<<8|txInfoInput.opts2.bit.vidl,
			txInfoInput.opts2.bit.prio,
			txInfoInput.opts3.bit.extspa,
			txInfoInput.opts3.bit.tx_portmask,
			txInfoInput.opts3.bit.l34_keep,
			txInfoInput.opts3.bit.tx_dst_stream_id);
		}

		if(isLsoPkt){
			txd->opts4 = 0x80000000 | (txMtu <<20);
		}
        if(isPktDbg){
            printk("TxDesc: 0x%08x 0x%08x 0x%08x 0x%08x buf 0x%08x\n", txd->opts1, txd->opts2, txd->opts3, txd->opts4, txd->addr);
            ptr = (unsigned char *) txd->addr;
            printk("Dump from 0x%08x\n", ptr);
            for(i = 0;i < (txd->opts1 & 0x1ff);i++)
            {
                if(i % 16 == 0)
                {
                    printk("\n");
                }
                printk("%02x ", *ptr);
                ptr ++;
            }
        }
		entry = NEXT_TX(entry,RE8670_TX_MRING_SIZE[ring_num]);
	}
	else 
	{
		printk("%s %d: not support frag xmit now\n", __func__, __LINE__);
		dev_kfree_skb_any(skb);
	}

	cp->tx_Mhqhead[ring_num] = entry;

	if (unlikely(TX_HQBUFFS_AVAIL(cp,ring_num) <= 1))
		netif_stop_queue(dev);

	write_buffer_flush();

	spin_unlock_irqrestore(&cp->tx_lock, flags);
	wmb();
	cp->cp_stats.tx_hw_num++;

	kick_tx(ring_num);

	dev->trans_start = jiffies;
	return 0;
}



int re8686_tx_with_Info_skb(struct sk_buff *skb)
{
    
	/*set force mac*/
	if(isMacChange){
		skb->data[0] = 0x00;
		skb->data[1] = 0x00;
		skb->data[2] = 0x00;
		skb->data[3] = 0x00;
		skb->data[4] = 0x00;
		skb->data[5] = 0x03;
	}

    //return re8686_test_with_txInfo_and_mask(skb,&gTxInfo, 0, &mask);
    return re8686_test_with_txInfo(skb,gTxInfo,gTxRing);
}




__IRAM_NIC
static void re8670_test_rx (struct re_private *cp)
{
	int rx_work;
	unsigned long flags;  
	int ring_num=0;
	unsigned rx_Mtail;
	unsigned int max_ring=1;
    u32 i;
    unsigned char *ptr;

	spin_lock_irqsave(&cp->rx_lock,flags);

	/*start to rx*/

	if(!(cp->dev->flags & IFF_UP)){
		spin_unlock_irqrestore (&cp->rx_lock, flags);   
		return;
	}

	if(isRxRingRoute)
	{
		printk("ISR1: %x\n",cp->isr1_status);
		max_ring = 6;
	}

	for(ring_num=0;ring_num < max_ring ;ring_num++)
	{

	
	if(isRxRingRoute && !(1 << ring_num & cp->isr1_status) ) continue;
	
	printk("%s: start test rx,devname: %s, IFF_UP: %x, ring %d\n",__FUNCTION__,cp->dev->name,cp->dev->flags & IFF_UP,ring_num);

	if(stopRx)
	{
		rx_Mtail = cp->rx_Mtail[ring_num];   
		printk("%s: current rx_tail",__FUNCTION__,rx_Mtail);
		
	}else{
		rx_Mtail = cp->rx_Mtail[ring_num];   

		rx_work = RE8670_RX_MRING_SIZE[ring_num];

		while (rx_work--)
		{
			u32 len;
			struct sk_buff *skb, *new_skb;

			DMA_RX_DESC *desc;
			unsigned buflen;
			struct rx_info rxInfo;

			cp->cp_stats.rx_hw_num++;		           	    
			skb = cp->rx_skb[ring_num][rx_Mtail].skb;

			if (unlikely(!skb))
				BUG();	   
			desc = &cp->rx_Mring[ring_num][rx_Mtail];	
			retriveRxInfo(desc, &rxInfo);	

			if (rxInfo.opts1.bit.own)
				break;
			
			save_rx_info(rxInfo,ring_num);

			len = rxInfo.opts1.bit.data_length & 0x0fff;		//minus CRC 4 bytes later

			if (unlikely(rxInfo.opts1.bit.rcdf)){		//DMA error

				cp->cp_stats.rcdf++;
			}
			if (unlikely(rxInfo.opts1.bit.crcerr)){		//CRC error

				cp->cp_stats.crcerr++;
			}

			buflen = cp->rx_buf_sz + RX_OFFSET;

			if (unlikely((rxInfo.opts1.dw & (FirstFrag | LastFrag)) != (FirstFrag | LastFrag))) {
				cp->cp_stats.frag++;
			}

			new_skb=re8670_getAlloc(SKB_BUF_SIZE);
			
			if (unlikely(!new_skb)) {
				cp->cp_stats.rx_no_mem++;
				dma_cache_wback_inv((unsigned long)skb->data,SKB_BUF_SIZE);
				goto rx_next;
			}

			if(rxInfo.opts1.bit.fs==1)
			{
				skb_reserve(skb, RX_OFFSET); // HW DMA start at 4N+2 only in FS.
			}

			if((gTxInfo.opts1.bit.crc ==1) && rxInfo.opts1.bit.fs && rxInfo.opts1.bit.ls)
			{
				len-=4;	//minus CRC 4 bytes here			
			}		
			skb_put(skb, len);

			if(isPktDbg) {
				printk("Rx St: FS %u LS %u\n", rxInfo.opts1.bit.fs, rxInfo.opts1.bit.ls);
				printk("SKB[%x] DA=%02x:%02x:%02x:%02x:%02x:%02x SA=%02x:%02x:%02x:%02x:%02x:%02x ethtype=%04x len=%u\n",(u32)skb&0xffff
				,skb->data[0],skb->data[1],skb->data[2],skb->data[3],skb->data[4],skb->data[5]
				,skb->data[6],skb->data[7],skb->data[8],skb->data[9],skb->data[10],skb->data[11]			
				,(skb->data[12]<<8)|skb->data[13],len);
			}

	        if(isRxCrcDbg && rxInfo.opts1.bit.crcerr){
				printk("Rx St: FS %u LS %u\n", rxInfo.opts1.bit.fs, rxInfo.opts1.bit.ls);
				printk("SKB[%x] DA=%02x:%02x:%02x:%02x:%02x:%02x SA=%02x:%02x:%02x:%02x:%02x:%02x ethtype=%04x len=%u\n",(u32)skb&0xffff
				,skb->data[0],skb->data[1],skb->data[2],skb->data[3],skb->data[4],skb->data[5]
				,skb->data[6],skb->data[7],skb->data[8],skb->data[9],skb->data[10],skb->data[11]			
				,(skb->data[12]<<8)|skb->data[13],len);

	            ptr = (unsigned char *) skb->data;
	            printk("Dump from 0x%08x\n", ptr);
	            for(i = 0;i < (rxInfo.opts1.bit.data_length & 0x1fff);i++)
	            {
	                if(i % 16 == 0)
	                {
	                    printk("\n");
	                }
	                printk("%02x ", *ptr);
	                ptr ++;
	            }
	        }

			cp->rx_Mring[ring_num][rx_Mtail].addr = CPHYSADDR(new_skb->data);
			cp->rx_skb[ring_num][rx_Mtail].skb = new_skb;
			new_skb->dev = cp->dev;;

			//dma_cache_wback_inv((unsigned long)new_skb->data,buflen);
			dma_cache_wback_inv((unsigned long)new_skb->data,SKB_BUF_SIZE);


			// Software flow control for Jumbo Frame RX issue. 
			{
				u16 cdo;
				int drop_ring_size=TH_ON_VAL>>1;
				if(ring_num==0)
					cdo=RTL_R16(RxCDO);
				else
					cdo=RTL_R16(RxCDO2+ADDR_OFFSET*(ring_num-1));

				if(cdo+drop_ring_size<RE8670_RX_MRING_SIZE[ring_num])
				{
					if((cdo<=rx_Mtail)&&(cdo+drop_ring_size>rx_Mtail))
					{
						printk("NIC Full tail=%d cdo=%d\n",rx_Mtail,cdo);
						dev_kfree_skb_any(skb);
						goto rx_next;
					}
				}
				else
				{
					if((cdo<=(rx_Mtail+RE8670_RX_MRING_SIZE[ring_num]))&&(cdo+drop_ring_size>(rx_Mtail+RE8670_RX_MRING_SIZE[ring_num])))
					{
						printk("NIC Full2 tail=%d cdo=%d\n",rx_Mtail,cdo);
						dev_kfree_skb_any(skb);
						goto rx_next;				
					}
				}			
			}
			/*fragment packet and LSO test */
			if(isLsoPkt && rxInfo.opts1.bit.fs && !rxInfo.opts1.bit.ls)
			{
				if(jumboFrame) {
                    dev_kfree_skb_any(jumboFrame);
                    jumboFrame = NULL;
                    jumboLength = 0;
				}

    		    jumboFrame = dev_alloc_skb(JUMBO_SKB_BUF_SIZE);

                if(!jumboFrame) {
                    printk("%s:%d allocte skb for jumbo frame fail\n", __FILE__, __LINE__);
					dev_kfree_skb_any(skb);
					goto rx_next;				
                }

				//memcpy(jumboFrame,skb,sizeof(struct sk_buff));
				jumboFrame->dev = skb->dev;
                skb_put(jumboFrame, len);
				memcpy(jumboFrame->data,skb->data,len);				
				jumboLength = len;
				goto rx_next;
			}else
			if(isLsoPkt && !rxInfo.opts1.bit.fs && !rxInfo.opts1.bit.ls)
			{
                if(!jumboFrame) {
                    printk("%s:%d allocte skb for jumbo frame fail\n", __FILE__, __LINE__);
					dev_kfree_skb_any(skb);
					goto rx_next;				
                }

                skb_put(jumboFrame, len);
				memcpy(jumboFrame->data+jumboLength,skb->data,len);				
				jumboLength += len;
				goto rx_next;
			}else
			if(isLsoPkt && !rxInfo.opts1.bit.fs && rxInfo.opts1.bit.ls)
			{
                if(!jumboFrame) {
                    printk("%s:%d allocte skb for jumbo frame fail\n", __FILE__, __LINE__);
					dev_kfree_skb_any(skb);
					goto rx_next;				
                }

                skb_put(jumboFrame, len);
				memcpy(jumboFrame->data+jumboLength,skb->data,len);				
				jumboLength += len;
				jumboLength -=4;	//minus CRC 4 bytes here
				jumboFrame->len = jumboLength;
				//printk("skb %x, frame len %d\n",jumboFrame,jumboLength);
				re8686_tx_with_Info_skb(jumboFrame);
				
			}else{
				re8686_tx_with_Info_skb(skb);
			}
			
	rx_next:
			desc->opts1 = (DescOwn | cp->rx_buf_sz) | ((rx_Mtail == (RE8670_RX_MRING_SIZE[ring_num] - 1))?RingEnd:0);
			updateGmacFlowControl(rx_Mtail,ring_num);
			rx_Mtail = NEXT_RX(rx_Mtail,RE8670_RX_MRING_SIZE[ring_num]);

		}
		cp->rx_Mtail[ring_num] = rx_Mtail;

		if(rx_work <= 0){
			tasklet_hi_schedule(&cp->rx_tasklets);
		}
	}
	}
	unmask_rx_int();	
	spin_unlock_irqrestore (&cp->rx_lock, flags); 
}


static void re8686_test_proc(void)
{
	
	struct proc_dir_entry *dir = NULL, *tx = NULL, *rx = NULL;
	printk("\n\n re8686_test_proc init \n\n");
	
	dir = proc_mkdir("rtl8686test",NULL);
	
	rx = create_proc_entry("rxinfo", 0, dir);
	if (rx) {
		rx->read_proc = rx_dump_read;
		rx->write_proc = rx_dump_write;
	}
	else {
		printk("rxinfo, create proc failed!\n");
	}

	tx = create_proc_entry("txinfo", 0, dir);
	if (tx) {
		tx->read_proc = tx_dump_read;
		tx->write_proc = tx_dump_write;
	}
	else {
		printk("txinfo, create proc failed!\n");
	}
}

static int __init re8686_test_init(void)
{
	struct net_device *dev;	
	/*hook re8686_test to interrupt */
	dev = dev_get_by_name(&init_net,"eth0");
	if(dev==NULL)
	{
		printk("can't find eth0 device\n");
	}
	struct re_private *cp = DEV2CP(dev);

	cp->rx_tasklets.func = (void (*)(unsigned long))re8670_test_rx;
	printk("%s: devname %s, task %x\n",__FUNCTION__,dev->name, cp->rx_tasklets.func);
	
	re8686_test_proc();
	/*set rx ring, all mapping to ring 0*/
	multi_ring_set();
	
	return 0;
}

static void __exit re8686_test_exist(void)
{
	printk("exist re8686_test module\n");
}



module_init(re8686_test_init);
module_exit(re8686_test_exist);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Star Chang <starchang@realtek.com>");

